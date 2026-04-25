#pattern1_yuv422p10le_320x240_25fps.y4m HDR_4k.yuv
cd outputs/tests/
./Qv2ComponentTests HDR_4k.yuv output.apv
ffplay -framerate 5 output.apv