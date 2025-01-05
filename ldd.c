#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h> 
#include <linux/string.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ksreekum");
MODULE_DESCRIPTION("First LKM");

#define BUFFER_SIZE 256
static char kernel_buffer[BUFFER_SIZE]; 
static size_t kernel_buffer_len = 0;

static struct proc_dir_entry *custom_proc_node;

static ssize_t driver_read(struct file *file_pointer, 
                           char __user *user_space_buffer, 
                           size_t count, 
                           loff_t *offset) {
    char default_msg[] = "Acknowledge\n";
    size_t default_msg_len = strlen(default_msg);
    size_t message_len = (kernel_buffer_len > 0) ? kernel_buffer_len : default_msg_len;
    const char *message = (kernel_buffer_len > 0) ? kernel_buffer : default_msg;

    printk(KERN_INFO "Driver Read Function Called\n");

    if (*offset >= message_len) 
        return 0; // EOF

    if (count > message_len - *offset)
        count = message_len - *offset;

    if (copy_to_user(user_space_buffer, message + *offset, count)) {
        printk(KERN_ERR "Failed to copy data to user space\n");
        return -EFAULT;
    }

    *offset += count;
    printk(KERN_INFO "Driver Read: Sent %zu bytes to user\n", count);
    return count;
}

static ssize_t driver_write(struct file *file_pointer, 
                            const char __user *user_space_buffer, 
                            size_t count, 
                            loff_t *offset) {
    printk(KERN_INFO "Driver Write Function Called\n");

    if (count > BUFFER_SIZE - 1) {
        printk(KERN_WARNING "Input size exceeds buffer capacity. Truncating input.\n");
        count = BUFFER_SIZE - 1; 
    }

    if (copy_from_user(kernel_buffer, user_space_buffer, count)) {
        printk(KERN_ERR "Failed to copy data from user space\n");
        return -EFAULT;
    }

    kernel_buffer[count] = '\0';
    kernel_buffer_len = count; // Update the length of the global buffer

    printk(KERN_INFO "Driver Write: Received %zu bytes from user: %s\n", count, kernel_buffer);
    return count;
}

struct proc_ops driver_proc_ops = {
    .proc_read = driver_read,
    .proc_write = driver_write,
};

static int ksreekum_module_init(void) {
    printk(KERN_INFO "Kernel Module init function ENTRY\n");
    custom_proc_node = proc_create("driver_name", 0666, NULL, &driver_proc_ops);
    if (!custom_proc_node) {
        printk(KERN_ERR "Failed to create /proc entry\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "Kernel Module init function EXIT\n");
    return 0;
}

static void ksreekum_module_exit(void) {
    proc_remove(custom_proc_node);
    printk(KERN_INFO "Good Bye!\n");
}

module_init(ksreekum_module_init);
module_exit(ksreekum_module_exit);
