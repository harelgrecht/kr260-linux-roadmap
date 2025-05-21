#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/serial_core.h>
#include <linux/serial_reg.h>
#include <linux/tty_flip.h>

#define DRIVER_NAME "customuart"
#define UART_BASE_ADDR 0xA0010000
#define UART_RX_REG 0x0
#define UART_TX_REG 0x4
#define UART_STATUS_REG 0x8
#define UART_CTRL_REG 0xC

#define TX_FIFO_FULL_BIT_INDEX 3
#define RX_FIFO_EMPTY_BIT_INDEX 0

struct custom_uart_port {
    struct uart_port port;
    void __iomem *uart_base;
};

static struct custom_uart_driver {
    struct uart_driver custom_driver;
    struct custom_uart_port custom_port;
}custom_uart_driver;

static unsigned int is_rx_fifo_empty(struct uart_port *port) {
    struct custom_uart_port * custom_port = container_of(port, struct custom_uart_port, port);
    u32 status = ioread32(custom_port->uart_base + UART_STATUS_REG);
    /*
        bit 0 indicats if the RX fifo has data:
        '0' = empty
        '1' = has data
    */
    pr_info("custom_uart: RX status = 0x%08x\n", status);
    if((status & (1 << RX_FIFO_EMPTY_BIT_INDEX)) == 0)
        return 0;
    else
        return 1;
}

static unsigned int is_tx_fifo_empty(struct uart_port *port) {
    struct custom_uart_port *custom_port = container_of(port, struct custom_uart_port, port);
    u32 status = ioread32(custom_port->uart_base + UART_STATUS_REG);
    /*
        bit 3 indicats if the TX fifi is full:
        '0' = not full
        '1' = full
    */
    pr_info("custom_uart: TX status = 0x%08x\n", status);
    if((status & (1 << TX_FIFO_FULL_BIT_INDEX)) == 0)
        return 1; //fifo is not full
    else
        return 0; // fifo full
}

static void custom_uart_start_tx(struct uart_port *port) {
    struct custom_uart_port *custom_port = container_of(port, struct custom_uart_port, port);
    char ch;
    pr_info("custom_uart: start_tx() called\n");
    while (is_tx_fifo_empty(port)) {
        if (uart_circ_empty(&port->state->xmit)) {
            pr_info("custom_uart: TX buffer empty\n");
            break;
        }
        ch = port->state->xmit.buf[port->state->xmit.tail];
        pr_info("custom_uart: TX char = 0x%02x\n", ch);
        iowrite32(ch, custom_port->uart_base + UART_TX_REG);

        port->state->xmit.tail = (port->state->xmit.tail + 1) & (UART_XMIT_SIZE - 1);
    }

    if (uart_circ_chars_pending(&port->state->xmit) < WAKEUP_CHARS)
        uart_write_wakeup(port);
}

static irqreturn_t custom_uart_start_rx(int irq, void *dev_id) {
    struct uart_port *port = dev_id;
    struct custom_uart_port *custom_port = container_of(port, struct custom_uart_port, port);
    char rx_data;
    pr_info("custom_uart: IRQ handler triggered\n");
    while(is_rx_fifo_empty(port)) {
        rx_data = ioread32(custom_port->uart_base + UART_RX_REG) & 0xFF;
        pr_info("custom_uart: RX char = 0x%02x\n", rx_data);
        uart_insert_char(port, 0, 0, rx_data, TTY_NORMAL);
    }
    tty_flip_buffer_push(&port->state->port);
    return IRQ_HANDLED;    
}

static int custom_uart_startup(struct uart_port *port) {
    pr_info("custom_uart: startup() - requesting IRQ %d\n", port->irq);
    if (request_irq(port->irq, custom_uart_start_rx, 0, "custom_uart_port", port)) {
        pr_err("custom_uart: Failed to request IRQ %d\n", port->irq);
        return -EBUSY;
    }

    pr_info("custom_uart: IRQ %d requested successfully\n", port->irq);
    return 0;
}

static void custom_uart_shutdown(struct uart_port *port) { 
    pr_info("custom_uart: shutdown() - freeing IRQ %d\n", port->irq);
    free_irq(port->irq, port); 
}

static const struct uart_ops custom_uart_ops = {
    .tx_empty = is_tx_fifo_empty,
    .start_tx = custom_uart_start_tx,
    //.start_rx = custom_uart_start_rx,
    .startup = custom_uart_startup,
    .shutdown = custom_uart_shutdown,
    // Implement other ops like stop_tx, startup, shutdown, set_termios as needed
};

static int custom_uart_probe(struct platform_device *pdev) {
    struct resource *res;
    int irq, ret;

    pr_info("custom_uart: probe() called\n");
    // alocating memory for uart driver structre
    custom_uart_driver.custom_driver.owner = THIS_MODULE,
    custom_uart_driver.custom_driver.driver_name = DRIVER_NAME,
    custom_uart_driver.custom_driver.dev_name = "ttyUART",
    custom_uart_driver.custom_driver.major = 0, 
    custom_uart_driver.custom_driver.minor = 0,
    custom_uart_driver.custom_driver.nr = 1,

    // regsiter uart driver with serial core using the low level serial API
    ret = uart_register_driver(&custom_uart_driver.custom_driver);
    if(ret) {
        pr_err("custom_uart_probe: Failed to register UART driver, error %d\n", ret);
        return ret;
    }

    //if a matching "compatible" have been found in the kernel this function get the  parameters from the device tree
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        pr_err("Failed to get memory resource for UART\n");
        uart_unregister_driver(&custom_uart_driver.custom_driver);
        return -ENODEV;
    }

    custom_uart_driver.custom_port.uart_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(custom_uart_driver.custom_port.uart_base))
        return PTR_ERR(custom_uart_driver.custom_port.uart_base);

    // get the irq number from the device tree
    irq = platform_get_irq(pdev, 0);
    if (irq < 0)
        return irq;

    //maps port settings
    custom_uart_driver.custom_port.port.membase = custom_uart_driver.custom_port.uart_base;
    custom_uart_driver.custom_port.port.mapbase = res->start;
    custom_uart_driver.custom_port.port.irq = irq;
    custom_uart_driver.custom_port.port.fifosize = 16;
    custom_uart_driver.custom_port.port.ops = &custom_uart_ops;
    custom_uart_driver.custom_port.port.flags = UPF_LOW_LATENCY;
    custom_uart_driver.custom_port.port.line = 0;

    ret = custom_uart_startup(&custom_uart_driver.custom_port.port);
    if (ret) {
        pr_err("Failed to start up UART port: %d\n", ret);
        return ret;
    }


    ret = uart_add_one_port(&custom_uart_driver.custom_driver, &custom_uart_driver.custom_port.port);
    if (ret) {
        pr_err("custom_uart: Failed to add UART port\n");
        return ret;
    }

    pr_info("custom_uart: UART port registered successfully\n");
    return 0;
}

static int custom_uart_remove(struct platform_device *pdev) {
    custom_uart_shutdown(&custom_uart_driver.custom_port.port);
    uart_remove_one_port(&custom_uart_driver.custom_driver, &custom_uart_driver.custom_port.port);
    uart_unregister_driver(&custom_uart_driver.custom_driver);
    pr_info("UART driver unregistered and port removed\n");
    return 0;
}

static const struct of_device_id custom_uart_of_match[] = {
    { .compatible = "harel,customuart" },
    {},
};
MODULE_DEVICE_TABLE(of, custom_uart_of_match);

static struct platform_driver custom_uart_platform_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = custom_uart_of_match,
    },
    .probe = custom_uart_probe,
    .remove = custom_uart_remove,
};

module_platform_driver(custom_uart_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harel");
MODULE_DESCRIPTION("Custom AXI UARTLite device driver");
