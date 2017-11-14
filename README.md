# Switchboard

## Development

Instructions to get ready for development on a recent Ubuntu.

### Install CMake
```
wget http://www.cmake.org/files/v2.8/cmake-2.8.3.tar.gz
tar xzf cmake-2.8.3.tar.gz
cd cmake-2.8.3
./configure
make
sudo make install
```

### Install Dependencies.
```
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```

### Get, build and run
```
git clone https://github.com/juzley/switchboard
cd switchboard
cmake -DCMAKE_BUILD_TYPE=Debug .
make
./switchboard
```
