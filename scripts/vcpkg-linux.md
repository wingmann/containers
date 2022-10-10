# Handling dependencies in Linux with vcpkg

## Install vcpkg once
```bash
cd ~/
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh
```
#### Before installing the dependencies, you must install pkg-config

## Install dependencies
```bash
cd ~/vcpkg/
./vcpkg install gtest:x64-linux
```
