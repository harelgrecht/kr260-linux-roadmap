
# Custom AXI UARTLite Linux Kernel Driver

This is a basic Linux kernel module that implements a character device driver for a custom AXI UARTLite peripheral on a Xilinx-based FPGA platform (e.g., Kria KR260). It provides simple polling-based read and write access to the UART through a `/dev/ttyCustomUart` device node.

## Features

- Character device interface
- Polling-based read and write (no interrupts)
- Resource management via Device Tree
- Auto device node creation using `udev`
- Basic memory-mapped I/O with `ioremap`

---

## Device Tree Binding

Make sure your device is defined in the Device Tree as follows:

```
custom_uart: customuart@a0010000 {
    compatible = "harel,customuart";
    reg = <0x0 0xa0010000 0x0 0x1000>;
};
```

---

## Driver Structure

### Key Components

- **Platform Driver**: Implements `.probe` and `.remove` callbacks to manage hardware.
- **Character Device**: Registers with the kernel to allow user-space applications to read/write.
- **Device Tree Support**: Uses `of_match_table` to match and bind the device.
- **Memory Mapping**: Maps hardware address to kernel virtual memory using `devm_ioremap_resource()`.

### File Operations

- `uart_read()`: Waits for RX FIFO data and reads bytes into user space.
- `uart_write()`: Waits until TX FIFO is not full and sends bytes from user space.

---

## Probe vs module_init

| module_init                      | probe                                              |
|----------------------------------|----------------------------------------------------|
| Called when the module is loaded | Called when a matching device is found via DT      |
| Registers the driver             | Initializes the hardware instance                  |
| Only once                        | Once for **each** matched device                   |
| Used in non-platform drivers     | Required for platform/DT-based drivers             |

In this driver, we use `module_platform_driver()` which combines `module_init()` and `module_exit()` behind the scenes and automatically registers/unregisters the platform driver.

---

## Multi-Device Support (Future Work)

This driver currently supports only one UART device. If multiple UARTs are defined in the Device Tree (e.g., with different `reg` addresses), each one would trigger a separate call to `probe()`. To support this properly:

- Allocate separate `struct uart_dev` instances per device
- Use `container_of()` or `dev_get_drvdata()` to access per-device data
- Dynamically allocate minors and `/dev` nodes (e.g., `/dev/ttyCustomUart0`, `/dev/ttyCustomUart1`, ...)

---

## License

This project is licensed under the terms of the GNU General Public License (GPL).

## Author

Harel
