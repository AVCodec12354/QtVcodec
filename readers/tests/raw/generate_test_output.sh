# ==========================================================
#                          Y4M
# ==========================================================
# 10-bit Planar 4:2:0 (320x240) - 10 frames
ffmpeg -y -i pattern1_yuv422p10le_320x240_25fps.y4m -vframes 10 -s 320x240 -pix_fmt yuv420p10le -strict -1 pattern1_320x240_p420_10b.y4m

# 10-bit Planar 4:4:4 (480x360) - 5 frames
ffmpeg -y -i pattern1_yuv422p10le_320x240_25fps.y4m -vframes 5 -s 480x360 -pix_fmt yuv444p10le -strict -1 pattern1_480x360_p444_10b.y4m

# 8-bit Planar 4:4:4 (320x240) - 8 frames
ffmpeg -y -f rawvideo -pixel_format yuv420p -video_size 480x856 -i input_480x856_yuv420p.yuv -vframes 8 -s 320x240 -pix_fmt yuv444p -strict -1 input_320x240_p444_8b.y4m

# 8-bit Planar 4:2:2 (480x360) - 5 frames
ffmpeg -y -f rawvideo -pixel_format yuv420p -video_size 480x856 -i input_480x856_yuv420p.yuv -vframes 5 -s 480x360 -pix_fmt yuv422p -strict -1 input_480x360_p422_8b.y4m

# ==========================================================
#                            Y4M
# ==========================================================
# 10-bit Semi-Planar (P010le) - 10 frames
ffmpeg -y -i pattern1_yuv422p10le_320x240_25fps.y4m -vframes 10 -s 480x360 -pix_fmt p010le pattern1_480x360_p010_10b.yuv

# 10-bit Packed (Y210le - 4:2:2 Packed) - 5 frames
ffmpeg -y -i pattern1_yuv422p10le_320x240_25fps.y4m -vframes 5 -s 320x240 -pix_fmt y210le pattern1_320x240_y210_10b.yuv

# 8-bit Semi-Planar (NV12) - 3 frames
ffmpeg -y -f rawvideo -pixel_format yuv420p -video_size 480x856 -i input_480x856_yuv420p.yuv -vframes 3 -s 176x144 -pix_fmt nv12 input_176x144_nv12_8b.yuv

# 8-bit Semi-Planar (NV21) - 3 frames
ffmpeg -y -f rawvideo -pixel_format yuv420p -video_size 480x856 -i input_480x856_yuv420p.yuv -vframes 3 -s 176x144 -pix_fmt nv21 input_176x144_nv21_8b.yuv

# 8-bit Packed (YUYV422) - 5 frames
ffmpeg -y -f rawvideo -pixel_format yuv420p -video_size 480x856 -i input_480x856_yuv420p.yuv -vframes 5 -s 320x240 -pix_fmt yuyv422 input_320x240_yuyv_8b.yuv

# 8-bit Packed (UYVY422) - 5 frames
ffmpeg -y -f rawvideo -pixel_format yuv420p -video_size 480x856 -i input_480x856_yuv420p.yuv -vframes 5 -s 320x240 -pix_fmt uyvy422 input_320x240_uyvy_8b.yuv

echo "Hoàn thành tạo file test!"