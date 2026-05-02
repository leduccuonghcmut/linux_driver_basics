#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>

#define DEVICE_NAME "exclusive_cdev"

static int major;
static int device_opened = 0;

static int my_open(struct inode *inode, struct file *filp)
{
    if (device_opened) {
        pr_warn("%s - Device is already in use\n", DEVICE_NAME);
        return -EBUSY;
    }

    device_opened++;

    pr_info("%s - Device opened\n", DEVICE_NAME);

    return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
    device_opened--;

    pr_info("%s - Device closed\n", DEVICE_NAME);

    return 0;
}

static ssize_t my_read(struct file *filp,
                       char __user *user_buf,
                       size_t len,
                       loff_t *off)
{
    return 0;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .release = my_release,
    .read    = my_read,
};

static int __init exclusive_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0) {
        pr_err("%s - Registration failed\n", DEVICE_NAME);
        return major;
    }

    pr_info("%s - Registered with Major: %d\n",
            DEVICE_NAME, major);

    return 0;
}

static void __exit exclusive_exit(void)
{
    unregister_chrdev(major, DEVICE_NAME);

    pr_info("%s - Unregistered\n", DEVICE_NAME);
}

module_init(exclusive_init);
module_exit(exclusive_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cuong Le Duc <leduccuonghcmut@gmail.com>");
MODULE_DESCRIPTION("Exclusive character device driver");