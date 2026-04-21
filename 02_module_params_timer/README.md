# 02_module_params_timer

Tóm tắt các kiến thức liên quan: 

## 1. Module parameters
Trong C bình thường, chúng ta sử dụng `argc` và `argv` trong hàm `main()`. Nhưng Kernel module không có main(), nên nó sử dụng `module_param` để nhận dữ liệu từ bên ngoài khi đang chạy.

* **`module_param(greet_name, charp, 0660)`**:
    * `charp`: Khai báo cho kernel biết đây là kiểu dữ liệu con trỏ chuỗi (character pointer).
    * `0660`: Đây là quyền truy cập và sử dụng quyền lớn hơn 0, Kernel tự động tạo một file ảo trong hệ thống tại đường dẫn `/sys/module/<module_name>/parameters/greet_name`. Quyền `0660` (rw-rw----) cho phép user root đọc và ghi trực tiếp vào file này lúc runtime để thay đổi biến số mà không cần nạp lại module.

## 2. Kernel timers 
Môi trường Kernel không có hàm `sleep()` như user-space vì có thể làm treo toàn bộ hệ thống. Thay vào đó, hệ thống sử dụng Timer.

* **`jiffies`**: Biến đếm toàn cục của Linux kernel, tăng lên sau mỗi tick của hệ thống.

* **`msecs_to_jiffies(timer_period_ms)`**: Hàm chuyển đổi thời gian từ mili-giây sang đơn vị jiffies mà kernel sử dụng.

* **`timer_setup(...)`**: Khởi tạo cấu trúc timer và liên kết với hàm `timer_callback`. Khi timer hết hạn, hàm callback sẽ được thực thi trong kernel context (cụ thể là softirq context).

* **`mod_timer(...)`**: Lên lịch kích hoạt timer tại một thời điểm trong tương lai (dựa trên giá trị jiffies hiện tại). Để tạo timer chạy tuần hoàn, cần gọi lại `mod_timer()` bên trong hàm callback.

## 3. Gỡ bỏ Timer an toàn (`del_timer`)

Trong hàm `__exit`, cần hủy timer trước khi unload module bằng `del_timer(&my_timer);`.

Nếu module bị gỡ bỏ trong khi timer vẫn còn hoạt động, kernel có thể thực thi hàm callback sau khi vùng nhớ của module đã được giải phóng. Điều này dẫn đến truy cập bộ nhớ không hợp lệ và có thể gây ra kernel panic.

