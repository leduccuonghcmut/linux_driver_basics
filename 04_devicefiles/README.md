# Giới thiệu về Device File trong Linux
## 1. Device Files là gì?

**Device Files** đóng vai trò là một cánh cửa kết nối giữa **Userspace**  và **Kernel Space**. 

Thông thường, các Device Files được hệ thống tự động tạo ra và quản lý trong thư mục `/dev`. Tuy nhiên, về mặt kỹ thuật, bạn có thể tạo chúng ở bất kỳ đâu. Hầu hết các tập tin trong `/dev` là tạm thời (được tạo ra bởi `udev` hoặc `devtmpfs` khi khởi động). Nếu bạn tắt Raspberry Pi, rút thẻ nhớ cắm vào máy tính và mở thư mục `/dev` trên thẻ nhớ đó, bạn sẽ thấy nó gần như trống rỗng.

### Phân tích `output` của lệnh `ls`

Ví dụ với lệnh `ls -l`:

```bash
-rw-r--r-- 1 pi pi 1169 Apr  22 08:45 README.md
```
* **Ký tự đầu tiên `-`**: Chỉ định đây là một tập tin thông thường.
* **`rw-r--r--`**: Quyền truy cập.
* **`1`**: Số lượng hardlinks trỏ đến tập tin này.
* **`pi pi`**: Chủ sở hữu và nhóm.
* **`1169`**: Kích thước tập tin tính bằng Bytes.
* **`Dec 6 20:08` & `README.md`**: Thời gian chỉnh sửa cuối cùng và tên tập tin.

Bây giờ, hãy so sánh với đầu ra khi kiểm tra các **tập tin thiết bị**:

```bash
crw-rw---- 1 root gpio    254,  0 Dec  7 14:07 /dev/gpiochip0
brw-rw---- 1 root disk    179,  0 Dec  7 14:07 /dev/mmcblk0
brw-rw---- 1 root disk    179,  1 Dec  7 14:07 /dev/mmcblk0p1
crw-rw---- 1 root dialout   4, 64 Dec  7 14:07 /dev/ttyS0
```

Có thể nhận thấy hai điểm khác biệt cốt lõi: 
1. Ký tự đầu tiên không còn là `-` mà là `c` hoặc `b`.
2. Không có thông tin về dung lượng, thay vào đó là hai con số được phân tách bằng dấu phẩy (ví dụ: `254, 0`).

---

## 2. Character Devices và Block Devices

Ký tự đầu tiên (`c` hoặc `b`) cho biết cách hệ điều hành tổ chức và truyền tải dữ liệu với thiết bị đó.

### Block Devices (`b`)
* **Đặc điểm:** Dữ liệu được đọc/ghi theo từng `khối` có kích thước cố định (thường là 512 bytes, 1KB, 4KB...). Hệ điều hành có thể truy cập ngẫu nhiên vào bất kỳ vị trí nào trên thiết bị và thường sử dụng bộ đệm (cache/buffer) để tăng tốc độ.
* **Ví dụ trong log:** `/dev/mmcblk0` là thiết bị đại diện cho thẻ nhớ SD trên Raspberry Pi. Ta không thể đọc 1 byte lẻ tẻ từ ổ cứng/thẻ nhớ ở mức vật lý, mà phải nạp cả một block lên RAM để xử lý.

### Character Devices (`c`)
* **Đặc điểm:** Dữ liệu được truyền tải tuần tự dưới dạng một luồng các byte hoặc ký tự. Không có bộ đệm và dữ liệu đọc ra/ghi vào sẽ được xử lý ngay lập tức. Ta không thể tua lại một ký tự đã đi qua.
* **Ví dụ trong log:** `/dev/ttyS0` đại diện cho cổng Serial (UART) của Raspberry Pi. Khi giao tiếp Serial, dữ liệu luôn được gửi/nhận từng byte một. Tương tự với `/dev/gpiochip0` (điều khiển chân GPIO).

---

## 3. Device Numbers

Khác biệt thứ hai là sự vắng mặt của kích thước tập tin. Thay vào đó, Linux sử dụng **Device Numbers**, bao gồm hai thành phần: **Major** và **Minor**.
Ví dụ với `/dev/gpiochip0`, ta có `Major = 254` và `Minor = 0`.

* **Major Number:** Định danh cho **Driver** nào trong Kernel sẽ chịu trách nhiệm quản lý thiết bị này. Các thiết bị dùng chung một Driver thường sẽ có chung số Major.
* **Minor Number:** Định danh cho **cá thể** cụ thể của thiết bị đó. (Có thể nhận giá trị từ 0 đến 255).

*Ví dụ:* Ở log trên, thiết bị lưu trữ SD Card `mmcblk0` và phân vùng thứ nhất của nó `mmcblk0p1` đều dùng chung một Driver (Major = `179`), nhưng là các phân vùng/thực thể khác nhau nên Minor khác nhau (`0` và `1`).

Để xem danh sách các Major Number đang được Kernel sử dụng, có thể đọc nội dung file ảo `/proc/devices`:

```bash
$ grep 179 /proc/devices 
179 mmc

$ grep 4 /proc/devices 
  4 /dev/vc/0
  4 tty
  4 ttyS

$ grep 254 /proc/devices 
254 gpiochip
```

> **NOTE:** Liên kết giữa phần cứng vật lý và Driver trong Kernel **KHÔNG** dựa vào tên tập tin (như `mmcblk0`), mà dựa hoàn toàn vào **Device Numbers (Major, Minor)**.

---

## 4. Thực nghiệm: Khởi tạo Device Files thủ công

Để chứng minh rằng Hệ điều hành Linux chỉ quan tâm đến Device Numbers chứ không quan tâm đến tên tập tin, chúng ta sẽ làm 2 bài test sử dụng lệnh `mknod`.

### Test 1: Với Block Device

Sử dụng `hexdump` để đọc một vài byte thô trực tiếp từ thẻ nhớ SD của Raspberry Pi (thông qua `/dev/mmcblk0`):

```bash
sudo hexdump /dev/mmcblk0 | head
# Đầu ra sẽ hiển thị các đoạn mã hex (dữ liệu thô của thẻ nhớ)
# 0000000 b8fa 1000 d08e 00bc b8b0 0000 d88e c08e ...
```

Bây giờ, hãy di chuyển về thư mục Home và tạo một Device File "giả mạo" bằng lệnh `mknod`, gán cho nó cùng thông số Major (`179`) và Minor (`0`) như thiết bị gốc:

```bash
cd ~
sudo mknod mymmc b 179 0
```
*(Cú pháp: `mknod [tên_file] [loại_thiết_bị] [Major] [Minor]`)*

Thử dump file vừa tạo:

```bash
sudo hexdump mymmc | head
# 0000000 b8fa 1000 d08e 00bc b8b0 0000 d88e c08e ...
```
**Kết quả:** Đầu ra hoàn toàn trùng khớp! Dù file tên là `mymmc` và nằm ở `/home/pi`, Kernel vẫn biết phải trỏ nó tới Driver quản lý thẻ nhớ nhờ cặp số `179, 0`.

### Test 2: Với Character Device

Nếu dùng jumper nối tắt chân TX và RX của Raspberry Pi, mở khóa cổng Serial trong `raspi-config` và truy cập qua terminal:

```bash
screen /dev/ttyS0 9600
```
Mọi ký tự gõ trên bàn phím sẽ được gửi qua TX, loopback về RX và in ngược lại lên màn hình.

Bây giờ, thoát `screen`, tạo một Device File mới có Major `4` và Minor `64` tương tự như `/dev/ttyS0`:

```bash
cd ~
sudo mknod myserial c 4 64
```

Chạy `screen` với file mới:

```bash
sudo screen myserial 9600
```
**Kết quả:** Hệ thống vẫn hoạt động hoàn hảo , gõ ký tự và nó vẫn echo lại bình thường.

**Chốt lại:** Tên của Device File (ví dụ `/dev/ttyS0` hay `myserial`) chỉ là một quy ước để con người dễ nhớ. Yếu tố duy nhất quyết định việc kết nối thành công với Driver dưới Kernel chính là cặp **Major Number** và **Minor Number**. Mọi file có cùng Major/Minor đều trỏ về cùng một phần cứng.
