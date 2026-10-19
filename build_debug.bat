@echo off
setlocal
set PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%
set DXSDK_DIR=C:\Users\xshxd\AppData\Local\Temp\opencode\dxsdk_7z\DXSDK
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
echo "=== Reconfiguring CMake with DXSDK_DIR ==="
cmake -S "D:\CODUnlimited Dev" -B "D:\CODUnlimited Dev\build" -G "Visual Studio 17 2022" -A Win32 -DCMAKE_BUILD_TYPE=Debug -DDXSDK_DIR=%DXSDK_DIR%
echo "=== Building ==="
cmake --build "D:\CODUnlimited Dev\build" --config Debug -- /m:4
