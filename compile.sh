rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug && make -j 4


if [ $# -eq 1 ]; then
    make $ 1
fi