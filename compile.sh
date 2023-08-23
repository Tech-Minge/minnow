rm -rf build && mkdir build && cd build
cmake .. && make -j 2


if [ $# -eq 1 ]; then
    make $ 1
fi