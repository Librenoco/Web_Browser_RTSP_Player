Web Browser RTSP Player
===

Введение
---
Данная программа позволяюет организовать передачу видеопотоков по протоколу rtsp (OpenCV) через web-сокеты (Libhv) для отображения в браузере.\
Проверено на Ubuntu 22.04.1 LTS.

Клонирование
---
Чтобы склонировать проект, введите команду:
```bash
cd
git clone https://github.com/Librenoco/Web_Browser_RTSP_Player.git
```

Автоматическая сборка
---
Для автоматической установке необходимо запустить скрипт:
```bash
cd
cd Web_Browser_RTSP_Player/
sudo sh script.sh
```
После работы скрипта можно будет запустить программу:
```bash
cd
cd Web_Browser_RTSP_Player/server
./main
```

Ручная сборка
---
Для сборки проекта необходимо собрать и установить следующие элементы:
1) Обновить систему, установить git, cmake и build-essential
    ```bash
    sudo apt-get update
    sudo apt-get install git
    sudo apt-get install cmake
    sudo apt-get install build-essential
    ```
2) Gstreamer  (1.20.3)
    ```bash
    gst-inspect-1.0 --version
    sudo apt-get install gstreamer1.0*
    sudo apt-get install ubuntu-restricted-extras
    sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
    ```
3) FFmpeg (4.2.2-0ubuntu0.22.04.1)
    ```bash
    ffmpeg -version
    sudo apt-get install ffmpeg
    ```
4) OpenCV (4.6.0)
    ```bash
    cd
    git clone https://github.com/opencv/opencv
    mkdir build
    cd build
    sudo apt-get install libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
    cmake -j4 ../opencv/ -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_GSTREAMER=ON -D WITH_FFMPEG=ON
    make -j4
    sudo make install
    ```
5) Libhv
    ```bash
    cd
    git clone https://github.com/ithewei/libhv
    cd libhv/
    ./configure
    make
    sudo make install
    ```
После того, как вы установили необходимые пакеты, введите следующие команды для сборки проекта
```bash
cd
cd Web_Browser_RTSP_Player/server/
make
```

Старт
---
Перед запуском сервера необходимо заполнить файл:\
В начале строки камеры необходимо ставить 1 пробел.
```bash
cd
cd Web_Browser_RTSP_Player/server/
nano camera.conf
```

Чтобы запустить сервер, введите следующую команду:
```bash
cd
cd Web_Browser_RTSP_Player/server/
./main
```

Теперь можно запустить тестовую страницу и проверить работоспособность программы.

