# Image Compressor using Sparse Matrix

A C program with GTK-based GUI that compresses images using sparse matrix representation. The program takes an image as input, converts it to sparse matrix format (storing only non-zero values above a threshold), and saves the compressed result.

## Features

- **GUI Frontend**: User-friendly GTK-based interface
- **Image Support**: PNG, JPEG, BMP formats
- **Sparse Matrix Compression**: Compresses images by storing only significant pixel values
- **Adjustable Threshold**: Control compression by setting a threshold value (0-255)
- **Live Preview**: View original and compressed images side by side
- **Compression Statistics**: See compression ratio and file size information

## How It Works

The sparse matrix compression algorithm:
1. Loads the image and extracts pixel data for each channel (R, G, B, A)
2. For each channel, creates a sparse matrix that stores only values above the threshold
3. Values below the threshold are treated as zero (compressed out)
4. The compressed data is stored as coordinate-value pairs (COO format)
5. The compressed image can be reconstructed from the sparse matrices

## Requirements

- C compiler (GCC or Clang)
- GTK+3 development libraries
- Make

### Installing Dependencies

**macOS:**
```bash
brew install gtk+3
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libgtk-3-dev build-essential
```

**Fedora:**
```bash
sudo dnf install gtk3-devel gcc make
```

## Building

### Step 1: Install Dependencies

**macOS:**

1. **Install C Compiler (if not already installed):**
   ```bash
   # Install Xcode Command Line Tools (includes gcc/clang)
   xcode-select --install
   ```
   On macOS, `gcc` is actually Apple's clang compiler - both work fine!

2. **Install GTK+3 using Homebrew:**
   ```bash
   brew install gtk+3
   ```

3. **Set PKG_CONFIG_PATH** (add to `~/.zshrc` or `~/.bashrc` for permanent setup):
   ```bash
   export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
   # For Intel Macs, use: export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
   ```

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libgtk-3-dev build-essential pkg-config
```

**Fedora/RHEL:**
```bash
sudo dnf install gtk3-devel gcc make pkg-config
```

### Step 2: Compile

**Using Makefile (Recommended):**
```bash
make
```

**Or compile manually:**
```bash
# Use gcc or clang (both work on macOS)
gcc -Wall -Wextra -std=c11 \
    `pkg-config --cflags --libs gtk+-3.0` -lm \
    main.c gui.c image_processor.c sparse_matrix.c \
    -o image_compressor

# Or use clang directly:
clang -Wall -Wextra -std=c11 \
    `pkg-config --cflags --libs gtk+-3.0` -lm \
    main.c gui.c image_processor.c sparse_matrix.c \
    -o image_compressor
```

**Or use the compile script:**
```bash
./compile.sh
```
The script will automatically detect and use gcc or clang.

If you get "Package gtk+-3.0 was not found", make sure:
1. GTK+3 is installed: `brew list gtk+3` (macOS) or check with package manager
2. PKG_CONFIG_PATH is set correctly (see Step 1)

## Usage

Run the program:

```bash
./image_compressor
```

Or use the make command:

```bash
make run
```

### Using the GUI

1. **Select Image**: Click "Select Image File" and choose an image file (PNG, JPEG, or BMP)
2. **Set Output**: Enter the output filename (e.g., `compressed.png`)
3. **Adjust Threshold**: Set the compression threshold (0-255)
   - Lower values: Less compression, more detail preserved
   - Higher values: More compression, less detail
4. **Compress**: Click "Compress Image" button
5. **View Results**: See the original and compressed images side by side, along with compression statistics

## Project Structure

```
.
├── main.c                 # Main program entry point
├── gui.h / gui.c          # GTK GUI implementation
├── image_processor.h/.c   # Image loading/saving functions
├── sparse_matrix.h/.c     # Sparse matrix data structure and operations
├── stb_image.h            # stb_image library for image I/O
├── stb_image_write.h      # stb_image_write for saving images
├── Makefile               # Build configuration
└── README.md              # This file
```

## Technical Details

### Sparse Matrix Format

The program uses Coordinate (COO) format for sparse matrices:
- Each non-zero element is stored as `(row, col, value)`
- Only values above the threshold are stored
- This reduces memory usage for images with many zeros or similar values

### Compression Ratio

The compression ratio is calculated as:
```
compression_ratio = (dense_matrix_size) / (sparse_matrix_size)
```

A higher ratio indicates better compression.

## Limitations

- The current implementation is designed for demonstration purposes
- Compression effectiveness depends on image content (images with many zeros compress better)
- The threshold-based compression may result in quality loss
- Large images may require significant memory

## License

This project uses:
- stb_image: Public domain (https://github.com/nothings/stb)
- GTK+: LGPL

## Author

Created as an educational project demonstrating sparse matrix compression for images.

