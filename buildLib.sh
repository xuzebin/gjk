#!/bin/sh
# Building only GFrame source into static lib to be used by GJK project
# Call this script before buildExample.sh

# Clean before the build.
function clean() {
    # Remove previous build folder
    if [ -d lib/build ]; then
        rm -rf lib/build
        echo "[INFO] Previous build foler removed."
    fi
}

# Validate the command exit with 0.
function validate() {
    if [ $? -eq 0 ]; then
        echo "[INFO] Command executed successfully."
    else
        echo "[ERROR] Command failed."
    fi
}

clean

mkdir -p lib/build
cd lib/build
cmake ../../GFrame -DBUILD_SRC=ON
validate
make
validate
make install
validate