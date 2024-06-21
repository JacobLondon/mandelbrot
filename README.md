# Mandelbrot

## Building
### Windows
```bash
wgd/tcc/make.exe
./far.exe
```
### Mac
```bash
# Install dependencies
brew install raylib

# Build the program
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
./far
```
