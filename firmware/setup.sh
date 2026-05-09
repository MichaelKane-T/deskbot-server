#!/bin/bash

PROJECT_NAME="pico-deskbot"

# Create main project folders
mkdir -p $PROJECT_NAME/src
cd $PROJECT_NAME || exit

# Create core files
touch CMakeLists.txt
touch FreeRTOSConfig.h

# Create source files
touch src/main.cpp
touch src/ili9341.cpp
touch src/ili9341.h

# Optional folders for scaling later
mkdir -p lib
mkdir -p build

echo "Project structure created:"
tree .
