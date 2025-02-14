#!/bin/bash

# 添加 Ubuntu 工具链 PPA
sudo apt update
sudo apt install -y software-properties-common
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test

# 安装 GCC-11 和相关工具
sudo apt update
sudo apt install -y gcc-11 g++-11

# 设置 GCC-11 为默认版本
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 110 \
                        --slave /usr/bin/g++ g++ /usr/bin/g++-11 \
                        --slave /usr/bin/gcov gcov /usr/bin/gcov-11

# 验证版本
g++ --version
