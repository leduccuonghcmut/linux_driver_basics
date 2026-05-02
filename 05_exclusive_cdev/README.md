# Exclusive Character Device Driver

## 1. Character Device Driver

Driver đăng ký với kernel thông qua:

```c
register_chrdev()
```

Sau khi đăng ký thành công, kernel sẽ cấp:

* Major number
* Minor number

Ví dụ log:

<img width="762" height="134" alt="image" src="https://github.com/user-attachments/assets/2003d04a-8003-4c98-938b-24b4c6b8e890" />

---

## 2. file_operations

Kernel sử dụng `file_operations` để liên kết các system call từ user space tới driver.

```c
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .release = my_release,
    .read    = my_read,
};
```

Cụ thể hơn như sau:

| User Space | Driver Callback |
| ---------- | --------------- |
| open()     | my_open()       |
| close()    | my_release()    |
| read()     | my_read()       |

---

## 3. Exclusive Access

Driver sử dụng biến:

```c
static int device_opened = 0;
```

để kiểm tra thiết bị có đang được sử dụng hay không.

Nếu thiết bị đã được mở thì kernel sẽ trả lỗi log dưới đây:

<img width="740" height="104" alt="image" src="https://github.com/user-attachments/assets/36e7203b-01a1-4d6a-9729-d5e852944705" />


## 4. Linux Error Code

Driver sử dụng:

```c
-EBUSY
```

để báo cho kernel biết thiết bị đang bận, đây là cách driver Linux trả lỗi chuẩn cho user space.

---

# Flow hoạt động

```text
User Space
    ↓
open("/dev/exclusive_cdev")
    ↓
VFS
    ↓
my_open()
    ↓
Kiểm tra device_opened
    ↓
Nếu đang bận → return -EBUSY
```




