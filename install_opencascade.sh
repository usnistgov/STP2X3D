#!/bin/bash

# Set the version of OpenCascade
OPENCASCADE_VERSION=7_8_1

echo "Intalling OpenCASCADE version ${OPENCASCADE_VERSION}"
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
    wget \
    gcc \
    g++ \
    cmake \
    git \
    tcl-dev \
    tk-dev \
    libfreetype6-dev \
    libgl1-mesa-dev \
    libxmu-dev \
    libxi-dev

# Clean up
sudo apt-get clean
sudo rm -rf /var/lib/apt/lists/*

# Download and build OpenCascade
wget https://github.com/Open-Cascade-SAS/OCCT/archive/refs/tags/V${OPENCASCADE_VERSION}.tar.gz
tar -xzf V${OPENCASCADE_VERSION}.tar.gz
cd OCCT-${OPENCASCADE_VERSION}
mkdir build
cd build
cmake ..
make -j$(nproc)
sudo make install

# Clean up downloaded files
cd ../..
rm -rf OCCT-${OPENCASCADE_VERSION} V${OPENCASCADE_VERSION}.tar.gz

echo "OpenCASCADE version ${OPENCASCADE_VERSION} installed."
