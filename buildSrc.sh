#!/bin/sh
# Build only GJK source files, not including GFrame.

# Clean before the build.
function clean() {
    # Remove previous build folder
    if [ -d build ]; then
        rm -rf build
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

mkdir -p build
cd build
cmake ..
validate
make
validate
make install
validate
