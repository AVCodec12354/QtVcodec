clear
rm -rf build
rm -f outputs/QtVcodec
rm -f outputs/tests/*
mkdir build
cd build
cmake ..
cmake --build .