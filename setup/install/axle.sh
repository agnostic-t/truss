#!/usr/bin/bash

set -e

echo "[setup] installing axle build system"

mkdir /tmp/axle_setup
cd /tmp/axle_setup

echo "[setup][axle] cloning repo"
git clone https://github.com/agnostic-t/axle
cd axle

echo "[setup][axle] building"
./scripts/build

echo "[setup][axle] installing to /usr/local/bin/axle"
./scripts/install

echo "[setup][axle] done"
