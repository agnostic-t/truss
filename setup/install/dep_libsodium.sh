#!/usr/bin/bash

set -e

echo "[setup] installing libsodium library"

mkdir /tmp/libsodium_installation
cd /tmp/libsodium_installation

echo "[setup][libsodium] downloading tarball"
wget https://github.com/jedisct1/libsodium/releases/download/1.0.22-RELEASE/libsodium-1.0.22.tar.gz

tar -xzf ./libsodium-1.0.22.tar.gz
cd ./libsodium-1.0.22

echo "[setup][libsodium] configuring and building"

./configure
make -j$(nproc) && make check

echo "[setup][libsodium] installing"

sudo make install

echo "[setup][libsodium] done"
