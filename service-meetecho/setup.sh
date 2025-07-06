#!/bin/bash

sudo apt update
sudo apt install pkg-config libtool automake

sudo apt install \
        libmicrohttpd-dev \
        libjansson-dev \
	libssl-dev \
        libsofia-sip-ua-dev \
        libglib2.0-dev \
	libopus-dev \
        libogg-dev \
        libcurl4-openssl-dev \
        liblua5.3-dev \
	libconfig-dev \

git clone https://gitlab.freedesktop.org/libnice/libnice
cd libnice
sudo meson --prefix=/usr build
sudo ninja -C build
sudo ninja -C build install
cd ..
sudo rm -rf libnice

wget https://github.com/cisco/libsrtp/archive/v2.2.0.tar.gz
tar xfv v2.2.0.tar.gz
cd libsrtp-2.2.0
./configure --prefix=/usr --enable-openssl
sudo make shared_library
sudo make install
cd ..
sudo rm -rf v2.2.0.tar.gz
sudo rm -rf libsrtp-2.2.0

git clone https://github.com/meetecho/janus-gateway.git
cd janus-gateway
sh autogen.sh
./configure --prefix=/opt/janus --disable-websockets --disable-data-channels --disable-rabbitmq --disable-mqtt
sudo make
sudo make install
sudo make configs
cd ..
sudo rm -rf janus-gateway

./setup-janus.sh