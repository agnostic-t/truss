#!/usr/bin/bash

set -e
shopt -s globstar

echo "[setup] installing picoquic library"

mkdir /tmp/picoquic_library
cd /tmp/picoquic_library

echo "[setup][picoquic] cloning repository"
git clone https://github.com/private-octopus/picoquic.git
cd picoquic

mkdir build
cd build

echo "[setup][picoquic] configuring library"
cmake -DPICOQUIC_FETCH_PTLS=Y ..

echo "[setup][picoquic] building"
make -j$(nproc)

echo "[setup][picoquic] installing"
sudo cp ./**/*.a /usr/local/lib/
sudo cp ../picoquic/*.h /usr/local/include/

echo "[setup][picoquic] done"
