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

#define TX_FIFO_FULL_BIT_INDEX 3
#define RX_FIFO_EMPTY_BIT_INDEX 0

static int major; // number to tell which driver handle which device
// static int minor; // number used by the driver it self to diffrentiate which device is operating
static void __iomem *uart_base;
static struct class*  uart_class  = NULL;
static struct device* uart_device = NULL;

static ssize_t uart_write(struct file* file, const char __user* buf, size_t count, loff_t* ppos) {
    size_t i;
    char write_buf;

    for(i = 0; i < count; i++) {
        if(copy_from_user(&write_buf, buf + i, 1)) { //copying from the user space buffer, copying buf into write_buf
            return -EFAULT;
        }
        
        while(ioread32(uart_base + UART_STATUS_REG) & (1 << TX_FIFO_FULL_BIT_INDEX)) { // TX_FIFO is Full wait to be empty
            cpu_relax();
        }
        iowrite32(write_buf, uart_base + UART_TX_REG);
    }
    return count;
}

static ssize_t uart_read(struct file* file, char __user* buf, size_t count,loff_t* ppos) {
    char read_buf;
    size_t i;

    for(i = 0; i < count; i++) {
        while((ioread32(uart_base + UART_STATUS_REG) & (1 << RX_FIFO_EMPTY_BIT_INDEX))) { // wait for RX_VALID_FIFO flag to be '1'
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


static int uart_device_probe(struct platform_device *plat_device) {
    struct resource *res;
    
    dev_info(&plat_device->dev, "Probing custom uart device...\n\n");
    
    //getting the respiurces from the DT
    res = platform_get_resource(plat_device, IORESOURCE_MEM, 0);
    if(!res) {
        dev_err(&plat_device->dev, "Failed to map IO memory\n");
        return -ENODEV;
    }

    // map the physical addres to the kernel
    uart_base = devm_ioremap_resource(&plat_dev -> dev, res);
    if (IS_ERR(uart_base)) {
        dev_err(&plat_dev->dev, "Failed to map IO memory\n");
        return PTR_ERR(uart_base);
    }
    
    // Register character device
    major = register_chrdev(0, DRIVER_NAME, &uart_fops);
    if (major < 0) {
        dev_err(&plat_device->dev, "Failed to register char device\n");
        return major;
    }

    // Create device class
    uart_class = class_create(THIS_MODULE, DRIVER_NAME);
    if (IS_ERR(uart_class)) {
        unregister_chrdev(major, DRIVER_NAME);
        dev_err(&plat_device->dev, "Failed to create class\n");
        return PTR_ERR(uart_class);
    }
    
    // Create device node /dev/ttyCustomUart
    uart_device = device_create(uart_class, NULL, MKDEV(major, 0), NULL, DRIVER_NAME);
    if (IS_ERR(uart_device)) {
        class_destroy(uart_class);
        unregister_chrdev(major, DRIVER_NAME);
        dev_err(&plat_device->dev, "Failed to create device\n");
        return PTR_ERR(uart_device);
    }
    
    dev_info(&plat_device->dev, "Custom UART initialized at %p\n", uart_base);
    return 0;
}

static int uart_device_remove(struct platform_device *pdev) {
    device_destroy(uart_class, MKDEV(major, 0));
    class_destroy(uart_class);
    unregister_chrdev(major, DRIVER_NAME);
    printk(KERN_INFO "Custom UART driver removed\n");
    return 0;
}

static const struct of_device_id custom_uart_of_match[] = {
    { .compatible = "harel,customuart" },
    {},
};
MODULE_DEVICE_TABLE(of, custom_uart_of_match);

static struct platform_driver custom_uart_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = custom_uart_of_match,
    },
    .probe = uart_device_probe,
    .remove = uart_device_remove,
};

module_platform_driver(custom_uart_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harel");
MODULE_DESCRIPTION("Custom AXI UARTLite device driver");