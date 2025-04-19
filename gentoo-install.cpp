#include <cstdlib>
#include <iostream>
#include <string_view>

// 定义系统命令执行器（带错误检查）
bool exec(const std::string_view cmd) {
    return std::system(cmd.data()) == 0;
}

int main() {
    // 使用命名空间简化代码
    using namespace std::string_literals;

    // 配置网络
    std::cout << "Configuring network...\n";
    if (!exec("nmtui"s)) return 1;

    // 磁盘分区
    std::cout << "Partitioning disk...\n";
    if (!exec("cfdisk /dev/sda"s)) return 1;

    // 格式化分区
    std::cout << "Formatting partitions...\n";
    const auto format_cmds = R"(
        mkfs.vfat /dev/sda1 &&
        mkswap /dev/sda2 &&
        mkfs.ext3 /dev/sda3 &&
        mkfs.ext4 /dev/sda4 &&
        mkfs.ext4 /dev/sda5
    )";
    if (!exec(format_cmds)) return 1;

    // 创建挂载点并挂载
    std::cout << "Mounting filesystems...\n";
    if (!exec("mkdir -p /mnt/gentoo && mount /dev/sda5 /mnt/gentoo && swapon /dev/sda2"s)) return 1;

    // 下载并解压Stage3
    std::cout << "Installing stage3...\n";
    if (!exec(R"(cd /mnt/gentoo && wget https://mirrors.ustc.edu.cn/gentoo/releases/amd64/autobuilds/20250413T165021Z/stage3-amd64-desktop-openrc-20250413T165021Z.tar.xz && tar xpvf stage3-amd64-desktop-openrc-20250413T165021Z.tar.xz)"s)) return 1;

    // 配置基本环境
    std::cout << "Configuring base system...\n";
    const auto base_configs = R"(
        nano /mnt/gentoo/etc/portage/make.conf &&
        mount /dev/sda1 /mnt/gentoo/boot &&
        mount /dev/sda3 /mnt/gentoo/home &&
        genfstab -U /mnt/gentoo >> /mnt/gentoo/etc/fstab &&
        mkdir -p /mnt/gentoo/etc/portage/repos.conf &&
        cp -L /usr/share/portage/config/repos.conf /mnt/gentoo/etc/portage/repos.conf/gentoo.conf &&
        nano /mnt/gentoo/etc/portage/repos.conf/gentoo.conf &&
        cp -d /etc/resolv.conf /mnt/gentoo/etc
    )";
    if (!exec(base_configs)) return 1;

    // 进入Chroot环境
    std::cout << "\nEnter Chroot Environment\n";
    const auto chroot_cmds = R"(
        source /etc/profile && export PS1="Gentoo ${PS1}" &&
        emerge-webrsync && eselect news list && eselect news read &&
        ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime &&
        hwclock --systohc && emerge --ask net-misc/chrony &&
        rc-update add chronyd default &&
        echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen &&
        echo "zh_CN.UTF-8 UTF-8" >> /etc/locale.gen &&
        locale-gen && eselect locale list && eselect locale set 4 &&
        nano /etc/conf.d/hostname &&
        emerge --ask net-misc/networkmanager &&
        rc-service NetworkManager start && rc-update add NetworkManager default &&
        emerge --ask sys-kernel/linux-firmware sys-kernel/genkernel sys-kernel/installkernel sys-kernel/gentoo-sources &&
        eselect kernel list && eselect kernel set 1 &&
        cd /usr/src/linux && genkernel --menuconfig all && genkernel --install initramfs &&
        emerge --ask sys-boot/grub:2 sys-apps/sudo app-editors/nano dev-vcs/git net-misc/wget &&
        emerge --ask admin/sysklogd sys-process/cronie app-shells/bash-completion &&
        rc-update add sysklogd default && rc-update add cronie default &&
        git clone https://gitcode.com/gh_mirrors/gr/Grub-Themes.git &&
        cd Grub-Themes && chmod +x install.sh &&
        grub-install --target=x86_64-efi --efi-directory=/boot --bootloader-id=Grub &&
        ./install.sh
    )";
    if (!exec(("chroot /mnt/gentoo /bin/bash -c \""s + chroot_cmds + "\"").c_str())) return 1;

    // 清理工作
    std::cout << "\nUnmounting filesystems...\n";
    exec("umount -R /mnt/gentoo/proc /mnt/gentoo/sys /mnt/gentoo/dev /mnt/gentoo && swapoff /dev/sda2");

    std::cout << "\nInstallation completed successfully! Please restart your system.\n";
    return 0;
}
