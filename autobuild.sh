#!/bin/bash

set -e

# 1. 编译
if [ ! -d "build" ]; then
    mkdir build
fi
rm -rf $(pwd)/build/*
cd build &&
    cmake .. &&
    make
cd ..

# 2. 重新创建干净的安装目录 (平铺化)
sudo rm -rf /usr/include/mymuduo
sudo mkdir -p /usr/include/mymuduo

# 3. 使用 find 强制搜寻并拷贝所有头文件到根目录
# 这一步会把 src/ 和 utils/ 下的所有 .h 统统考到 /usr/include/mymuduo/ 这一层
sudo find src -name "*.h" -exec cp {} /usr/include/mymuduo/ \;
sudo find utils -name "*.h" -exec cp {} /usr/include/mymuduo/ \;

# 4. 拷贝动态库并刷新
sudo cp $(pwd)/lib/libmymuduo.so /usr/lib
sudo ldconfig

echo "Build and Flattened Installation Success!"