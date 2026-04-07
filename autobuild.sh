set -e


if [ ! -d "build" ]; then
    mkdir build
fi

rm -rf `pwd`/build/*

cd  build &&
    cmake .. &&
    make
cd ..

# 清理旧文件
rm -f /usr/include/mymuduo/*.h
rm -rf /usr/include/mymuduo/src
rm -rf /usr/include/mymuduo/utils

# 确保目录结构存在（使用 -p 参数）
mkdir -p /usr/include/mymuduo/src
mkdir -p /usr/include/mymuduo/utils

for header in src/*.h
do
    cp "$header" /usr/include/mymuduo/src
done

for header in `ls utils/*.h`
do
    cp "$header" /usr/include/mymuduo/utils
done

cp `pwd`/lib/libmymuduo.so /usr/lib

ldconfig