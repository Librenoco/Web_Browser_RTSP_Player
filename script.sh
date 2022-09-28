#!/bin/bash

cd ..
myPwd=$PWD

# Обновление системы
apt-get update -y

# Установка необходимых утилит
apt-get install -y git
apt-get install -y cmake
apt-get install -y build-essential

# Блок установки основных библиотек
# 0) Boost
apt-get install -y libboost-all-dev

# 1) Gstreamer
apt-get install -y gstreamer1.0*
apt-get install -y ubuntu-restricted-extras
apt-get install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

# 2) FFmpeg
apt-get install -y ffmpeg

# 3) OpenCV
cd "$myPwd"
git clone https://github.com/opencv/opencv
mkdir build
cd build
apt-get install -y libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
cmake ../opencv/ -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_GSTREAMER=ON -D WITH_FFMPEG=ON
make -j4
make install

# 4) Libhv
cd "$myPwd"
git clone https://github.com/ithewei/libhv
chmod 777 libhv
cd libhv/
./configure
make
make install
cd "$myPwd"
rm -R opencv
rm -R build
rm -R libhv
# ----------

# Сборка проги
cd "$myPwd"
git clone https://github.com/Librenoco/Web_Browser_RTSP_Player.git
cd Web_Browser_RTSP_Player/server/
make
make clean
cd "$myPwd"
cd Web_Browser_RTSP_Player/http_server/
make
make clean
