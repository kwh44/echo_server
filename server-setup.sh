#!/bin/bash

cd
apt update
apt upgrade -y
apt install libboost-all-dev -y
apt install cmake -y
apt install make -y
cd echo_server
mkdir build
cd build
cmake ..
make -j4
