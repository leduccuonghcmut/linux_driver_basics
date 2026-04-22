# Introduction to Device Files in Linux
## 1. What are Device Files?

**Device Files** act as a gateway connecting **Userspace** and **Kernel Space**. 

Normally, Device Files are automatically created and managed by the system in the `/dev` directory. However, technically, you can create them anywhere. Most files in `/dev` are temporary (created by `udev` or `devtmpfs` at boot). If you turn off your Raspberry Pi, remove the SD card, plug it into a computer, and open the `/dev` directory on that card, you will find it almost empty.

### Analyzing the `ls` command `output`

For example, with the `ls -l` command:

```bash
-rw-r--r-- 1 pi pi 1169 Apr  22 08:45 README.md
```
* **First character `-`**: Indicates this is a regular file.
* **`rw-r--r--`**: Access permissions.
* **`1`**: Number of hardlinks pointing to this file.
* **`pi pi`**: Owner and group.
* **`1169`**: File size in Bytes.
* **`Dec 6 20:08` & `README.md`**: Last modified time and file name.

Now, let's compare it with the output when inspecting **device files**:

```bash
crw-rw---- 1 root gpio    254,  0 Dec  7 14:07 /dev/gpiochip0
brw-rw---- 1 root disk    179,  0 Dec  7 14:07 /dev/mmcblk0
brw-rw---- 1 root disk    179,  1 Dec  7 14:07 /dev/mmcblk0p1
crw-rw---- 1 root dialout   4, 64 Dec  7 14:07 /dev/ttyS0
```

Two core differences can be noticed: 
1. The first character is no longer `-` but `c` or `b`.
2. There is no size information; instead, there are two numbers separated by a comma (e.g., `254, 0`).

---

## 2. Character Devices and Block Devices

The first character (`c` or `b`) indicates how the operating system organizes and transfers data with that device.

### Block Devices (`b`)
* **Characteristics:** Data is read/written in fixed-size `blocks` (typically 512 bytes, 1KB, 4KB...). The operating system can randomly access any location on the device and usually utilizes a cache/buffer to increase speed.
* **Example in log:** `/dev/mmcblk0` represents the SD card on the Raspberry Pi. We cannot read a single isolated byte from a hard drive/SD card at the physical level; instead, an entire block must be loaded into RAM for processing.

### Character Devices (`c`)
* **Characteristics:** Data is transferred sequentially as a stream of bytes or characters. There is no buffering, and data read/written is processed immediately. We cannot rewind to a character that has already passed.
* **Example in log:** `/dev/ttyS0` represents the Serial (UART) port of the Raspberry Pi. During Serial communication, data is always sent/received one byte at a time. The same applies to `/dev/gpiochip0` (controlling GPIO pins).

---

## 3. Device Numbers

The second difference is the absence of file size. Instead, Linux uses **Device Numbers**, comprising two components: **Major** and **Minor**.
For example, with `/dev/gpiochip0`, we have `Major = 254` and `Minor = 0`.

* **Major Number:** Identifies which **Driver** in the Kernel will be responsible for managing this device. Devices sharing the same Driver will usually share the same Major number.
* **Minor Number:** Identifies the specific **instance** of that device. (Can take values from 0 to 255).

*For example:* In the log above, the SD Card storage device `mmcblk0` and its first partition `mmcblk0p1` both use the same Driver (Major = `179`), but since they are different partitions/entities, their Minors are different (`0` and `1`).

To see the list of Major Numbers currently being used by the Kernel, you can read the contents of the virtual file `/proc/devices`:

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

> **NOTE:** The link between physical hardware and the Driver in the Kernel does **NOT** rely on the file name (like `mmcblk0`), but entirely on the **Device Numbers (Major, Minor)**.

---

## 4. Experiment: Creating Device Files Manually

To prove that the Linux Operating System only cares about Device Numbers and not file names, we will run 2 tests using the `mknod` command.

### Test 1: With a Block Device

Use `hexdump` to read a few raw bytes directly from the Raspberry Pi's SD card (via `/dev/mmcblk0`):

```bash
sudo hexdump /dev/mmcblk0 | head
# Output will show hex codes (raw data of the SD card)
# 0000000 b8fa 1000 d08e 00bc b8b0 0000 d88e c08e ...
```

Now, navigate to the Home directory and create a "fake" Device File using the `mknod` command, assigning it the same Major (`179`) and Minor (`0`) parameters as the original device:

```bash
cd ~
sudo mknod mymmc b 179 0
```
*(Syntax: `mknod [file_name] [device_type] [Major] [Minor]`)*

Try dumping the newly created file:

```bash
sudo hexdump mymmc | head
# 0000000 b8fa 1000 d08e 00bc b8b0 0000 d88e c08e ...
```
**Result:** The output is an exact match! Even though the file is named `mymmc` and located in `/home/pi`, the Kernel still knows to point it to the SD card management Driver thanks to the pair of numbers `179, 0`.

### Test 2: With a Character Device

If you use a jumper to short the TX and RX pins of the Raspberry Pi, unlock the Serial port in `raspi-config`, and access it via terminal:

```bash
screen /dev/ttyS0 9600
```
Every character typed on the keyboard will be sent via TX, looped back to RX, and printed back onto the screen.

Now, exit `screen`, and create a new Device File with Major `4` and Minor `64`, similar to `/dev/ttyS0`:

```bash
cd ~
sudo mknod myserial c 4 64
```

Run `screen` with the new file:

```bash
sudo screen myserial 9600
```
**Result:** The system still works perfectly; type a character, and it still echoes back normally.

**Conclusion:** The name of a Device File (e.g., `/dev/ttyS0` or `myserial`) is merely a convention for human readability. The only factor determining a successful connection with the Driver in the Kernel is the pair of **Major Number** and **Minor Number**. Any files with the same Major/Minor will point to the exact same hardware.
