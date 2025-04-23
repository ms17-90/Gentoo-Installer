# Gentoo安装程序
这个安装程序，不是那种填写掉问卷，引导用户一步一步搭建gentoo
这是直接调用系统命令来安装的，分区最大5个区
由C++开发，没多少行代码，不过可以完全安装一个基础系统了
# 如何下载源码？
sudo -s
wget https://github.com/ms17-90/Gentoo-Installer/releases/download/untagged-0d6c4085b240ded2868c/Gentoo-Installer.tar.gz
# 如何解压源码？
tar -zxvf Gentoo-Installer.tar.gz
cd Gentoo-Installer
# 如何安装源码？
./configure
./install
# 如何执行程序
bash:
Gentoo-Installer
