mkdir ..\lib
cd ..\lib
git clone https://github.com/wingmann/iterators

cd %HOMEPATH%\vcpkg

vcpkg install gtest:x64-windows
