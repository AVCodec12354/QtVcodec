# 📘 CHUẨN MỰC LẬP TRÌNH (CODING STANDARDS)

> **AI INSTRUCTION (HƯỚNG DẪN CHO AI):**
```agsl
Ví dụ:
CodingRules.md
/Volumes/D/Projects/AndroidStudios/QtVcodec/readers/interfaces/Qv2Source.h
/Volumes/D/Projects/AndroidStudios/QtVcodec/readers/implements/YUVSource.h
/Volumes/D/Projects/AndroidStudios/QtVcodec/readers/implements/YUVSource.cpp
/Volumes/D/Projects/AndroidStudios/QtVcodec/readers/implements/Y4MSource.h
/Volumes/D/Projects/AndroidStudios/QtVcodec/readers/implements/Y4MSource.cpp
```
> Khi nhận được yêu cầu phân tích code của các file và thực hiện:
> 1. **Rất quan trọng** ĐỌC file hoặc tất cả file trong folder (`folderA/file`) được chỉ định
> 2. **Phân tích:** Đối chiếu code với các quy tắc từ Mục 1 đến Mục 6.
> 2. **Phân loại lỗi:** Liệt kê các lỗi tìm thấy và phân nhóm theo Mục 7 (Critical, Major, Minor).
> 3. **Chấm điểm:** Tính điểm bắt đầu từ 100đ và trừ điểm theo bảng tại Mục 8.
> 4. **Kết luận:** Tạo file .md => Đưa ra bảng tổng kết điểm, trạng thái (PASSED/FAIL) và nguyên nhân rõ ràng.
> Tổng hợp lại nguyên nhân và đoạn sai vào cuối cùng

---

## 1. QUY TẮC ĐẶT TÊN (NAMING CONVENTIONS)
*   **Biến & Hàm:** Sử dụng `camelCase`.
    *   *Biến:* Phải là danh từ (Ví dụ: `retryCount`, `bufferSize`).
    *   *Hàm:* Phải bắt đầu bằng động từ (Ví dụ: `calculateTotal()`, `fetchData()`).
*   **Class & Interface:** Sử dụng `PascalCase`.
    *   *Interface:* Bắt đầu bằng chữ `Qv2` hoặc `I` hoặc có hậu tố `Interface` (Ví dụ: `ISource`, `IDataProcessor`, `Qv2Source`).
*   **Hằng số (Constants):** Sử dụng `UPPER_SNAKE_CASE` (Ví dụ: `MAX_BUFFER_SIZE`).
*   **Boolean:** Phải có tiền tố `is`, `has`, `should`, `can` (Ví dụ: `isValid`, `hasData`).
*   **mVariable** biến toàn cục sẽ có tiền tố `m` ở đầu

---

## 2. QUY ƯỚC LẬP TRÌNH (CODING CONVENTION)
*   **Độ dài hàm:** Không quá **50 dòng** code (không tính comment và dòng trống). Nếu dài hơn, bắt buộc phải tách hàm (Extract Method) để đảm bảo tính đơn nhiệm.
*   **Tham số hàm:** Tối đa **10 tham số**. Nếu cần truyền nhiều hơn, hãy đóng gói vào một `Struct`, `Class` hoặc `Parameter Object`.
*   **Độ dài dòng code:** Không quá **80 ký tự** mỗi dòng để tránh việc phải cuộn ngang khi đọc code.
*   **Guard Clauses:** Xử lý các điều kiện lỗi, giá trị `null`, hoặc điều kiện biên ngay đầu hàm bằng lệnh `return` hoặc `throw`.
    *   *Mục tiêu:* Giảm bớt khối `else` và giữ luồng chính của hàm ở mức thụt đầu dòng thấp nhất.
*   **Mức độ lồng nhau (Nesting Level):** Không lồng các khối lệnh (`if`, `for`, `while`, `switch`) quá **2 cấp**. Nếu logic phức tạp hơn, bắt buộc phải tách thành hàm con.
*   **Quản lý tài nguyên (RAII):**
    *   Sử dụng Smart Pointers (`unique_ptr`, `shared_ptr`) thay cho con trỏ thô (`new/delete`).
    *   Mọi tài nguyên (File, Socket, Mutex) phải được giải phóng tự động thông qua Destructor hoặc Scope-based management.
*   **Comment (Chú thích):**
    *   Chỉ giải thích **"Tại sao" (Why)** - tức là ý đồ thiết kế hoặc lý do xử lý đặc biệt.
    *   Không giải thích **"Cái gì" (What)** - không lặp lại những gì code đã thể hiện rõ ràng.
*   **Self-documenting Code:** Ưu tiên đặt tên biến và hàm rõ ràng để code "tự giải thích" thay vì lạm dụng comment để giải thích logic khó hiểu.

---

## 3. FULL CLEAN ARCHITECTURE (CẤU TRÚC 4 LỚP)
*Mục tiêu: Đảm bảo tính độc lập của logic nghiệp vụ.*

*   **Lớp 1: Entities (Core Business):** Chứa các quy tắc nghiệp vụ cốt lõi, các Plain Objects. Không được phụ thuộc vào bất kỳ thư viện bên ngoài nào (Database, Framework).
*   **Lớp 2: Use Cases (Interactors):** Chứa logic nghiệp vụ cụ thể của ứng dụng. Điều phối luồng dữ liệu đến và đi từ Entities.
*   **Lớp 3: Interface Adapters (Presenters/Gateways/Controllers):** Chuyển đổi dữ liệu từ định dạng của Use Cases sang định dạng của các tác nhân bên ngoài (DB, UI, Web).
    *   *Quy tắc:* Controller không được chứa logic nghiệp vụ.
*   **Lớp 4: Frameworks & Drivers (External):** Chứa Database, UI, Web Framework, Device Drivers. Đây là lớp ngoài cùng.
*   **Quy tắc phụ thuộc (Dependency Rule):** Sự phụ thuộc chỉ được hướng vào bên trong (Lớp 4 -> Lớp 3 -> Lớp 2 -> Lớp 1). Lớp bên trong **không bao giờ** được biết về lớp bên ngoài.
*   **Data Crossing:** Dữ liệu đi qua các ranh giới (Boundaries) nên là các cấu trúc dữ liệu đơn giản (DTOs), không mang theo logic của Framework.

---

## 4. NGUYÊN TẮC THIẾT KẾ SOLID (SOLID PRINCIPLES)
*   **S - Single Responsibility:** Một Class/Module chỉ nên có một nhiệm vụ duy nhất.
*   **O - Open/Closed:** Đối tượng nên thoải mái để mở rộng nhưng hạn chế sửa đổi bên trong.
*   **L - Liskov Substitution:** Các lớp con phải có khả năng thay thế lớp cha mà không làm hỏng chương trình.
*   **I - Interface Segregation:** Chia nhỏ Interface thành nhiều mục đích cụ thể, không ép buộc implement hàm không dùng.
*   **D - Dependency Inversion:** Phụ thuộc vào Abstraction, không phụ thuộc vào Implementation cụ thể.

---

## 5. MẪU THIẾT KẾ (DESIGN PATTERNS)
*   **Creational:** Sử dụng `Factory` hoặc `Builder` khi việc khởi tạo object phức tạp.
*   **Structural:** Sử dụng `Adapter` để kết nối các interface không tương thích, `Proxy` để kiểm soát truy cập.
*   **Behavioral:** Sử dụng `Strategy` để thay đổi thuật toán tại runtime, `Observer` để quản lý sự kiện.
*   **Data Access:** Sử dụng `Repository Pattern` để tách biệt logic truy vấn dữ liệu khỏi logic nghiệp vụ.
*   **Rule:** Tránh "Over-engineering" - không áp dụng pattern nếu bài toán đơn giản có thể giải quyết bằng code thuần sạch sẽ.

---

## 6. QUẢN LÝ BỘ NHỚ & HIỆU NĂNG (MEMORY & PERFORMANCE)
*   **Memory Leak:** Mọi vùng nhớ cấp phát động phải có chủ sở hữu rõ ràng. Tránh tham chiếu vòng (Circular Reference).
*   **Pass by Reference:** Sử dụng `const Type&` cho các object lớn (string, vector, class) để tránh copy dữ liệu vô nghĩa.
*   **Loop Optimization:** Không thực hiện các tác vụ nặng (I/O, log, cấp phát bộ nhớ) bên trong vòng lặp xử lý dữ liệu core.
*   **Pre-allocation:** Sử dụng `reserve()` cho các container nếu biết trước kích thước dữ liệu.

---

## 7. PHÂN LOẠI LỖI (ERROR CLASSIFICATION)
*   🔴 **CRITICAL:** Crash, Memory Leak, Logic sai nghiệp vụ, Lỗ hổng bảo mật.
*   🟡 **MAJOR:** Vi phạm Dependency Rule (Lớp trong gọi lớp ngoài), Vi phạm SOLID, Sai Design Pattern, Performance Issue (copy object lớn), Resource Leak.
*   🔵 **MINOR:** Sai naming, Magic Numbers, Formatting (khoảng trắng, xuống dòng), Duplicate code nhẹ.

---

## 8. HỆ THỐNG CHẤM ĐIỂM (SCORING SYSTEM)
*Điểm mặc định: 100*

| Loại lỗi | Trừ điểm | Điều kiện dừng |
| :--- | :--- | :--- |
| **Critical** | -40đ / lỗi | Xuất hiện 1 lỗi = **Fail ngay lập tức** |
| **Major** | -15đ / lỗi | Tối đa cho phép 2 lỗi |
| **Minor** | -5đ / lỗi | Tối đa cho phép 5 lỗi |
| **Clean Code** | +5đ | Thưởng cho giải pháp tối ưu, thông minh |

### Xếp loại kết quả:
*   **90 - 100đ:** ✅ **PASSED**
*   **75 - 89đ:** ⚠️ **PASSED WITH NOTES** (Sửa lỗi Minor trước khi merge)
*   **50 - 74đ:** ❌ **REQUEST CHANGES**
*   **Dưới 50đ:** 🚫 **REJECTED**
*   Đưa ra các phân tích chi tiết cho từng mục.