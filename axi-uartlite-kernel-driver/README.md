# 🧵 AXI UART Lite - Custom Linux Kernel Driver (Polling Based)

This project provides a **custom Linux kernel module** that replaces the default Xilinx AXI UART Lite driver on the Kria KR260. The goal is to offer a minimal, production-usable UART character device (`/dev/ttyCustomUART`) that supports both **polling** and, later, **interrupt-driven** modes.

---

## 📌 Features

- ✅ Replaces default UART Lite driver
- ✅ Polling-based read and write implementation
- 🛠️ User-space C library planned (`libuartlite`)
- 🔜 Interrupt support (optional, planned)
- 📁 Clean `/dev` character interface

---

## 🧱 Architecture

```
[User App]
    |
[libuartlite (planned)]
    |
[/dev/ttyCustomUART]
    |
[Kernel Module - axi_uartlite_custom.ko]
    |
[AXI UART Lite IP Core in PL]
```

---

## 📂 Repo Structure

```
axi-uartlite-driver/
├── driver/
│   └── customuart.c              # Kernel module source
├── dts/
│   └── system-user.dtsi          # Device Tree overlay snippet
├── Makefile                      # Kernel module build file
├── README.md                     
```

---

## 🛠️ Building the Driver

Assumes PetaLinux tools are installed.

```bash
# Create a module project
petalinux-create -t modules -n <modulename> --enable
#module name cannot use'_' or '-' or capslock.

# Navigate to the module dir
cd project-spec/meta-user/recipes-modules/<modulename>/files

# Implement the module in the .c file, when finished
petalinux-build -c <modulename>

# Transfter the .ko file to the running linux board

# On host machine
sudo ifconfig <device> 192.168.0.100

# On running linux board
sudo ifconfig <device> 192.168.0.10

cd build/tmp/sysroots-components/xilinx_k26_kr/<modulename>/lib/modules/6.1.5-xilinx-v2023.1/extra/
scp -r <modulename>.ko <username>@<target ip>:/home/<username>
```

---

## 🚀 Inserting the Module

```bash
sudo insmod customuart.ko
dmesg | tail
```

You should see:

```bash
Initializing uart device...
Device has been registerd, major number = 235
Mapping UART base succeeded
UART base initialized
```

---

## 📖 Usage

Once inserted, a new character device will appear:

```bash
ls -l /dev/ttyCustomUART
```
---

## 👷 TODO / Planned Features

- [ ] Write user-space C library (`libuartlite`) for easier integration
- [ ] Add interrupt support 
- [ ] Add loopback and stress testing scripts
- [ ] Clean error handling and logging


---

## 📚 Background

This driver was built as part of a larger learning roadmap for embedded Linux development on the **Kria KR260**, focused on mastering kernel modules, memory-mapped I/O, and device trees.

---

## 🙋 About the Author

**Harel** – Embedded Linux & Kernel Developer in Progress  
Check out the full [development roadmap](https://github.com/harelgrecht/kr260-linux-roadmap.git) for more context and related projects.

---