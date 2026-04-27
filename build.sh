clear
rm -rf build
rm -f outputs/QtVcodec
rm -f outputs/tests/Qv2ComponentTests
mkdir build
cd build
cmake ..
cmake --build .