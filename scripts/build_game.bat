@echo off
cls

:: run from the location of this script
cd %~dp0
cd ..

mkdir build
cd build
cmake -A Win32 -DBUILD_SERVER_ONLY=ON ..
cmake --build . --config Release

echo.