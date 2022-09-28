Web Browser RTSP Player
===

Введение
---
Данные программы позволяют организовать передачу видеопотоков по протоколу rtsp (OpenCV) через web-сокеты (Libhv) для отображения в браузере.\
Проверено на Ubuntu 22.04.1 LTS.

!в данный момент это минимально демонстрационный материал!

Клонирование
---
Чтобы склонировать проект, введите команду:
```bash
cd
git clone https://github.com/Librenoco/Web_Browser_RTSP_Player.git
```

Автоматическая сборка
---
Для автоматической установки необходимо запустить скрипт:
```bash
cd
cd Web_Browser_RTSP_Player/
sudo sh script.sh
```
После работы скрипта необходимо заполнить файлы.\
Пример заполнения файлов представлен ниже в данном файле

После работы скрипта можно будет запустить программы:
```bash
cd
cd Web_Browser_RTSP_Player/http_server/
./main
```
```bash
cd
cd Web_Browser_RTSP_Player/server/
./main
```
Теперь можно запустить тестовую страницу и проверить работоспособность программы.\
Для этого необходимо в браузерной строке вбить ip+port http сервера.\
Должна открыться страница для авторизации.

Ручная сборка
---
Для сборки проекта необходимо собрать и установить следующие элементы:

0) Обновить систему, установить git, cmake и build-essential
    ```bash
    sudo apt-get update
    sudo apt-get install git
    sudo apt-get install cmake
    sudo apt-get install build-essential
    ```
1) Boost
    ```bash
    sudo apt-get install libboost-all-dev
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
cd
cd Web_Browser_RTSP_Player/http_server/
make
```

Старт
---
Перед запуском сервера необходимо заполнить файлы:\
1)camera.conf\
В конце строки камеры необходимо ставить 1 пробел и написать имя если камера будет записивать поток в файл и затем ещё 1 пробле для выбора режима работы потока.\
Пример:\
rtsp://admin:admin@255.255.255.255:8080/1 0 \
rtsp://admin:admin@255.255.255.255:8080/1 test 1\
где "test" это имя файла, а "1" это запись в файл.
```bash
cd
cd Web_Browser_RTSP_Player/server/
nano camera.conf
```
2)ip_port_http_server.conf\
Данный файл содержит адрес http сервера и видео сервера.\
Пример:\
127.0.0.1:8080\
127.0.0.1\
23656\
где первая строка это ip+port http сервера.\
Вторая строка это ip данного видеосервера.\
Третья строка это порт данного видеосервера.\
```bash
cd
cd Web_Browser_RTSP_Player/server/
nano ip_port_http_server.conf
```
3)user.txt\
В конце строки камеры необходимо ставить 1 пробел и написать имя если камера будет записивать поток в файл и затем ещё 1 пробле для выбора режима работы потока.\
Пример:\
admin&admin1&a\
user&user1&dsa\
где "&" это разделитель, а в начале идёт имя "user" затем пароль "user1" и в конце строка для uid "a" (произвольный символ или строка).
```bash
cd
cd Web_Browser_RTSP_Player/http_server/
nano user.txt
```
4)ip_port_http_server.conf\
Данный файл содержит адрес http сервера и видео сервера.\
Пример:\
127.0.0.1\
8080\
где первая строка это ip данного http сервера.\
Вторая строка это port данного http сервера.\
```bash
cd
cd Web_Browser_RTSP_Player/http_server/
nano ip_port_http_server.conf
```

Затем необходимо запустить http сервер, введите следующие команды:
```bash
cd
cd Web_Browser_RTSP_Player/http_server/
./main
```

В конце уже можно запустить видео сервер, который подключится к http серверу, введите следующие команды:
```bash
cd
cd Web_Browser_RTSP_Player/server/
./main
```

Теперь можно запустить тестовую страницу и проверить работоспособность программы.\
Для этого необходимо в браузерной строке вбить ip+port http сервера.\
Должна открыться страница для авторизации.