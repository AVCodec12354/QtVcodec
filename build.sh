rm -rf build
#rm -rf outputs
mkdir build
cd build
cmake ..
cmake --build .
#../outputs/QtVCodec
#../outputs/tests/Qv2ComponentTests ../outputs/tests/Qv2ComponentTests/pattern1_yuv422p10le_320x240_25fps.y4m output.apv
cd ../outputs/tests/
./Qv2ComponentTests pattern1_yuv422p10le_320x240_25fps.y4m output.apv
ffplay -framerate 25 output.apv