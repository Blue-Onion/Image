#!/bin/bash

# Compilation script for Image Compressor
# This script helps compile the project with proper GTK+3 setup

set -e  # Exit on error

echo "=== Image Compressor - Compilation Script ==="
echo ""

# Check if pkg-config is available
if ! command -v pkg-config &> /dev/null; then
    echo "Error: pkg-config is not installed."
    echo "Please install it first:"
    echo "  macOS: brew install pkg-config"
    echo "  Ubuntu/Debian: sudo apt-get install pkg-config"
    exit 1
fi

# Check if GTK+3 is available
if ! pkg-config --exists gtk+-3.0; then
    echo "Error: GTK+3 development libraries not found!"
    echo ""
    echo "Please install GTK+3:"
    echo ""
    echo "macOS:"
    echo "  brew install gtk+3"
    echo "  export PKG_CONFIG_PATH=\"/opt/homebrew/lib/pkgconfig:\$PKG_CONFIG_PATH\""
    echo ""
    echo "Ubuntu/Debian:"
    echo "  sudo apt-get install libgtk-3-dev"
    echo ""
    echo "Fedora:"
    echo "  sudo dnf install gtk3-devel"
    echo ""
    exit 1
fi

echo "✓ GTK+3 found"
echo ""

# Get compiler flags
CFLAGS=$(pkg-config --cflags gtk+-3.0)
LDFLAGS=$(pkg-config --libs gtk+-3.0)

# Detect compiler (gcc or clang)
if command -v gcc &> /dev/null; then
    CC=gcc
elif command -v clang &> /dev/null; then
    CC=clang
else
    echo "Error: No C compiler found!"
    echo ""
    echo "Please install Xcode Command Line Tools:"
    echo "  xcode-select --install"
    exit 1
fi

echo "Using compiler: $CC"
$CC --version | head -1
echo ""

echo "Compiling source files..."
echo ""

# Compile each source file
echo "  - main.c"
$CC -Wall -Wextra -std=c11 $CFLAGS -c main.c -o main.o

echo "  - gui.c"
$CC -Wall -Wextra -std=c11 $CFLAGS -c gui.c -o gui.o

echo "  - image_processor.c"
$CC -Wall -Wextra -std=c11 $CFLAGS -c image_processor.c -o image_processor.o

echo "  - sparse_matrix.c"
$CC -Wall -Wextra -std=c11 $CFLAGS -c sparse_matrix.c -o sparse_matrix.o

echo ""
echo "Linking executable..."
$CC main.o gui.o image_processor.o sparse_matrix.o $LDFLAGS -lm -o image_compressor

echo ""
echo "✓ Compilation successful!"
echo ""
echo "Run the program with: ./image_compressor"
echo ""

