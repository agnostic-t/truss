#!/usr/bin/bash

set -e

echo "[setup] starting to setup truss project"
echo "[setup] will set up:"
echo "[setup] - yyjson"
echo "[setup] - libsodium"
echo "[setup] - picoquic"
echo "[setup] - axle (build system)"
echo ""

./setup/install/dep_yyjson.sh
./setup/install/axle.sh

./setup/install/dep_libsodium.sh
./setup/install/dep_picoquic.sh

echo "[setup] NOTICE: on ARM (aarch64) devices you probably need to remove \`picotls-fusion\` dependency from \`.modules/main\` and \`.modules/linkserv\`"
