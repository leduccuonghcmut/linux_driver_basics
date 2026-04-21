#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

static char *greet_name = "Embedded Engineer";
static int timer_period_ms = 2000; 

module_param(greet_name, charp, 0660);
MODULE_PARM_DESC(greet_name, "Ten nguoi duoc chao");

module_param(timer_period_ms, int, 0660);
MODULE_PARM_DESC(timer_period_ms, "Chu ky chao (millisecond)");

static struct timer_list my_timer;

static void timer_callback(struct timer_list *t)
{
    printk(KERN_INFO "Xin chao - Chao %s! (Chu ky: %d ms)\n", greet_name, timer_period_ms);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(timer_period_ms));
}

static int __init my_init(void)
{
    printk(KERN_INFO "Xin chao - Module dang khoi dong...\n");
    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(timer_period_ms));
    
    return 0;
}

static void __exit my_exit(void)
{
    del_timer(&my_timer);
    printk(KERN_INFO "Xin chao - Module da duoc go bo an toan.\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cuong Le Duc <cuong.leduc@hcmut.edu.vn>");
MODULE_DESCRIPTION("Kernel module with configurable parameters and periodic timer logging");