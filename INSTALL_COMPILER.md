# Installing C Compiler

## macOS

On macOS, you need Xcode Command Line Tools which includes the C compiler.

### Option 1: Install via Terminal (Recommended)

Open Terminal and run:
```bash
xcode-select --install
```

This will open a dialog box. Click "Install" and wait for it to complete (may take several minutes).

### Option 2: Install via Xcode

1. Download Xcode from the App Store
2. Open Xcode and accept the license
3. Go to **Xcode → Settings → Locations**
4. Select a Command Line Tools version

### Verify Installation

After installation, verify it works:
```bash
gcc --version
# Should show: Apple clang version X.X.X ...
```

**Note:** On macOS, `gcc` is actually Apple's clang compiler. Both `gcc` and `clang` commands work identically.

## Linux

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential
```

### Fedora/RHEL/CentOS
```bash
sudo dnf groupinstall "Development Tools"
# Or
sudo yum groupinstall "Development Tools"
```

### Arch Linux
```bash
sudo pacman -S base-devel
```

## Windows

### Option 1: MinGW-w64
1. Download MinGW-w64 from: https://www.mingw-w64.org/
2. Or use MSYS2: https://www.msys2.org/
3. Install and add to PATH

### Option 2: Visual Studio Build Tools
1. Download Visual Studio Build Tools
2. Select "C++ build tools" workload
3. Use `cl.exe` compiler (different syntax)

### Option 3: WSL (Windows Subsystem for Linux)
Install Linux distribution and use Linux instructions above.

## Verification

Test that your compiler works:
```bash
echo 'int main(){return 0;}' > test.c
gcc test.c -o test
./test
echo "Compiler works!"  # If no errors, you're good!
rm test.c test
```

