# QtVCodec System Agents & Architecture

Tài liệu này mô tả các thành phần chính (Agents) trong hệ thống QtVcodec và cách chúng phối hợp để xử lý nén video.

## 1. Qv2Component (The Worker)
Đây là "linh hồn" của hệ thống, chịu trách nhiệm xử lý dữ liệu nặng (Encoding/Decoding).
- **Interface**: `Qv2Component.h`
- **Vai trò**: Nhận `Qv2Work` từ hàng đợi, xử lý thông qua thư viện Codec (như OpenAPV) và trả kết quả qua `Listener`.
- **Trạng thái**: Quản lý bởi một máy trạng thái (State Machine) thông qua `mState` (Atomic).

## 2. Qv2Params (The Negotiator)
Hệ thống cấu hình dựa trên các tham số độc lập (Codec2 style).
- **Cấu trúc**: Mỗi tham số là một struct riêng lẻ có 32-bit ID định danh.
- **Vai trò**: Cho phép Client cấu hình chính xác từng thuộc tính (Width, Bitrate, FPS...) mà không cần biết về cấu trúc bên trong của Codec.
- **Lợi ích**: Cực kỳ linh hoạt, có thể cập nhật thông số ngay cả khi Codec đang chạy (Tuning).

## 3. VideoRenderer (The Visualizer)
Chịu trách nhiệm hiển thị dữ liệu hình ảnh lên màn hình.
- **Công nghệ**: OpenGL.
- **Vai trò**: Nhận các `Qv2Buffer2D` và chuyển đổi chúng thành hình ảnh hiển thị trên `QOpenGLWidget`.

## 4. ViewModel (The Coordinator)
Lớp trung gian điều phối giữa giao diện người dùng (Qt UI) và các Component xử lý.
- **Vai trò**: Tạo ra các đối tượng `Qv2Param` phù hợp từ input của người dùng và đẩy xuống Component.

## 5. QTLogger (The Observer)
Hệ thống ghi log tập trung.

---

## Luồng tương tác (Pipeline Flow)

1. **Setup**: UI lấy thông số -> Tạo danh sách `std::vector<Qv2Param*>` -> Gọi `component->configure(params)`.
2. **Execution**: `component->start()` kích hoạt luồng xử lý riêng (Worker Thread).
3. **Processing**: Dữ liệu từ Reader được đóng gói vào `Qv2Work` -> Đưa vào `queue()`.
4. **Feedback**: Kết quả trả về qua `onWorkDone` -> Hiển thị thông qua `VideoRenderer`.
1.
mCapacity (Sức chứa tối đa):
◦
Là tổng lượng bộ nhớ vật lý đã được cấp phát cho con trỏ mData.
◦
Nó xác định giới hạn thượng: Bạn không được phép ghi quá số lượng byte này vào buffer (nếu không sẽ bị tràn bộ nhớ - Buffer Overflow).
◦
Ví dụ: Bạn chuẩn bị một cái "thùng" có thể chứa tối đa 10 lít nước. mCapacity ở đây là 10.
2.
mSize (Kích thước dữ liệu thực tế):
◦
Là lượng dữ liệu hữu ích thực sự đang nằm trong buffer tại một thời điểm cụ thể.
◦
mSize luôn luôn nhỏ hơn hoặc bằng mCapacity.
◦
Ví dụ: Trong cái thùng 10 lít đó, hiện tại bạn chỉ đổ vào 2 lít nước. mSize ở đây là 2.