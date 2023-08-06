mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=~/Documents/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --config Release
copy /Y Release\main.exe ..
copy /Y Release\sqlite3.dll ..
copy /Y Release\libsodium.dll ..
pause