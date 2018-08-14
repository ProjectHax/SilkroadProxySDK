SilkroadProxySDK
===

This SDK is intended to be used with [filter.projecthax.com](https://filter.projecthax.com/).

Requirements
---

1. Ubuntu 18.04 **x64**

Build Instructions
---

1. `sudo apt install build-essential libboost-all-dev libcurl4-openssl-dev clang cmake`
1. `git clone https://github.com/ProjectHax/SilkroadProxySDK.git`
1. `cd SilkroadProxySDK`
1. Change the project name in `CMakeLists.txt` and `src/main.cpp`
1. `mkdir build && cd build`
1. `cmake ..`
1. `make -j4`
