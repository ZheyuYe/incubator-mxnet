cmake -GNinja -D CMAKE_BUILD_TYPE=Release -D BUILD_opencv_gpu=OFF -D CMAKE_INSTALL_PREFIX=/home/ubuntu/.local/ ..

cp config/linux_gpu.cmake config.cmake
rm -rf build
mkdir -p build && cd build
cmake -GNinja -C ../config.cmake ..
cmake --build .
