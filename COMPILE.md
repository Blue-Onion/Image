# Compilation Instructions

## Prerequisites

You need GTK+3 development libraries installed on your system.

### macOS (using Homebrew)

1. Install GTK+3:
   ```bash
   brew install gtk+3
   ```

2. After installation, you may need to set PKG_CONFIG_PATH:
   ```bash
   export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
   ```
   Or for Intel Mac:
   ```bash
   export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
   ```

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install libgtk-3-dev build-essential
```

### Fedora/RHEL

```bash
sudo dnf install gtk3-devel gcc make
```

## Compilation

### Method 1: Using Makefile (Recommended)

Simply run:
```bash
make
```

This will:
- Compile all source files
- Link them together with GTK+3 libraries
- Create the executable `image_compressor`

### Method 2: Manual Compilation

If Makefile doesn't work, compile manually:

```bash
gcc -Wall -Wextra -std=c11 \
    `pkg-config --cflags gtk+-3.0` \
    -c main.c -o main.o

gcc -Wall -Wextra -std=c11 \
    `pkg-config --cflags gtk+-3.0` \
    -c gui.c -o gui.o

gcc -Wall -Wextra -std=c11 \
    `pkg-config --cflags gtk+-3.0` \
    -c image_processor.c -o image_processor.o

gcc -Wall -Wextra -std=c11 \
    `pkg-config --cflags gtk+-3.0` \
    -c sparse_matrix.c -o sparse_matrix.o

gcc main.o gui.o image_processor.o sparse_matrix.o \
    `pkg-config --libs gtk+-3.0` -lm \
    -o image_compressor
```

### Method 3: One-liner Compilation

```bash
gcc -Wall -Wextra -std=c11 \
    `pkg-config --cflags --libs gtk+-3.0` -lm \
    main.c gui.c image_processor.c sparse_matrix.c \
    -o image_compressor
```

## Troubleshooting

### Error: "Package gtk+-3.0 was not found"

**On macOS:**
1. Make sure GTK+3 is installed: `brew install gtk+3`
2. Set PKG_CONFIG_PATH:
   ```bash
   export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
   ```
   Add this to your `~/.zshrc` or `~/.bashrc` to make it permanent.

**On Linux:**
- Install development packages: `sudo apt-get install libgtk-3-dev` (Ubuntu/Debian)

### Error: "No such file or directory: stb_image.h"

The stb_image headers should already be in the project directory. If missing, download them:
```bash
curl -L https://raw.githubusercontent.com/nothings/stb/master/stb_image.h -o stb_image.h
curl -L https://raw.githubusercontent.com/stb/stb_image_write.h/master/stb_image_write.h -o stb_image_write.h
```

### Error: Missing include files

Make sure all source files are in the same directory and includes are correct.

## Running the Program

After successful compilation:
```bash
./image_compressor
```

Or use:
```bash
make run
```

## Cleaning Build Files

To remove compiled objects and executable:
```bash
make clean
```

