#!/bin/bash

# 移除旧版本
sudo apt remove --purge cmake
sudo apt autoremove

# 添加 Kitware 的 APT 仓库
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/kitware.list

# 更新并安装
sudo apt update
sudo apt install cmake
