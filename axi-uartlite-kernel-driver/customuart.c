#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/device.h>

#define DRIVER_NAME "customuart"
#define UART_BASE_ADDR 0xA0010000
#define UART_RX_REG 0x0
#define UART_TX_REG 0x4
#define UART_STATUS_REG 0x8
#define UART_CTRL_REG 0xC

static int major; // number to tell which driver handle which device
// static int minor; // number used by the driver it self to diffrentiate which device is operating
static void __iomem *uart_base;
static struct class*  uart_class  = NULL;
static struct device* uart_device = NULL;

static ssize_t uart_write(struct file* file, const char __user* buf, size_t count, loff_t* ppos) {
    size_t i;
    char write_buf;
    int tx_fifo_full_bit = 3;

    for(i = 0; i < count; i++) {
        if(copy_from_user(&write_buf, buf + i, 1)) { //copying from the user space buffer, copying buf into write_buf
            return -EFAULT;
        }
        
        while(ioread32(uart_base + UART_STATUS_REG) & (1 << tx_fifo_full_bit)) { // TX_FIFO is Full wait to be empty
            cpu_relax();
        }
        iowrite32(write_buf, uart_base + UART_TX_REG);
    }
    return count;
}

static ssize_t uart_read(struct file* file, char __user* buf, size_t count,loff_t* ppos) {
    char read_buf;
    int rx_fifo_empty_bit = 0;
    size_t i;

    for(i = 0; i < count; i++) {
        while((ioread32(uart_base + UART_STATUS_REG) & (1 << rx_fifo_empty_bit)) == 0) { // wait for RX_VALID_FIFO flag to be '1'
            cpu_relax();
        }
        read_buf = ioread32(uart_base + UART_RX_REG) & 0xFF;
        if(copy_to_user(buf + i, &read_buf, 1)) {
            return -EFAULT;
        }
    }
    return count;
}

static struct file_operations uart_fops = {
    .owner = THIS_MODULE,
    .read = uart_read,
    .write = uart_write,
};

static int __init uart_device_init(void) {
    printk(KERN_INFO "Initializing uart device...\n");

    major = register_chrdev(0, DRIVER_NAME, &uart_fops);//passing arg1 = 0, will make the kernel allocated a major number
    if(major < 0) {
        printk(KERN_ERR "Failed to register a device\n");
        return major;
    }
    printk(KERN_INFO "Device has been registerd, major number = %d\n", major);

    uart_base = ioremap(UART_BASE_ADDR, 0x08);
    if(!uart_base) {
        printk(KERN_ERR "Mapping UART failed\n");
        return -ENOMEM;
    }

    uart_class = class_create(THIS_MODULE, "customuart");
    if(IS_ERR(uart_class)) {
        unregister_chrdev(major, DRIVER_NAME);
        printk(KERN_ERR "Failed to create class for uart\n");
        return PTR_ERR(uart_class);
    }

    uart_device = device_create(uart_class, NULL, MKDEV(major, 0), NULL, "ttyCustomUart");
    if (IS_ERR(uart_device)) {
        class_destroy(uart_class);
        unregister_chrdev(major, DRIVER_NAME);
        printk(KERN_ERR "Failed to create device\n");
        return PTR_ERR(uart_device);uart_device = device_create(uart_class, NULL, MKDEV(major, 0), NULL, "ttyCustomUart");
if (IS_ERR(uart_device)) {
    class_destroy(uart_class);
    unregister_chrdev(major, DRIVER_NAME);
    printk(KERN_ERR "Failed to create device\n");
    return PTR_ERR(uart_device);
}
    }

    printk(KERN_INFO "Mapping UART base succeeded\n");
    printk(KERN_INFO "UART base initialized\n");
    return 0;
}

static void __exit uart_device_exit(void) {
    if(uart_base) {
        iounmap(uart_base);
    }
    device_destroy(uart_class, MKDEV(major, 0));
    class_destroy(uart_class);
    unregister_chrdev(major, DRIVER_NAME);
    printk(KERN_INFO "UART device driver unloaded\n");
}


module_init(uart_device_init);
module_exit(uart_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harel");
MODULE_DESCRIPTION("Custom AXI UARTLite device driver");