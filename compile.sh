
if [ $# -eq 0 ]; then
    cmake --build build
else
    cmake --build build --target $1
fi
