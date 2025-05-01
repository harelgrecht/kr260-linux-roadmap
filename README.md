# 🚀 Kria KR260 Linux Kernel Development Roadmap

Welcome! This repo tracks my personal journey to become a Linux kernel and embedded software developer using the Kria KR260 platform and PetaLinux. Each project builds on the previous one, from basic system setup to writing custom drivers and real-time networking features.

---

## 🧭 Project Roadmap

| ✅ Status | Project | Description |
|----------|---------|-------------|
| ✅ DONE | [PetaLinux Setup](https://github.com/harelgrecht/kr260-linux-roadmap) | Built and booted a custom PetaLinux image for the KR260. Learned SD boot flow, kernel config, DTS overlays. |
| ✅ DONE | [Ethernet Packet Processor](https://github.com/harelgrecht/Ethernet-Packet-Processor) | Multithreaded user-space app capturing packets, custom process them, and resending via Ethernet over UDP. |
| ✅ DONE | [Ethernet 1588 Parser](https://github.com/harelgrecht/Ethernet-1588-Parser) | Parsing raw sniffed L2 ethernet packets into PtPv2-1588 structre and converting to ToD, and writing them back to the PL. |
| ✅ DONE | [AXI GPIO Kernel Module](https://github.com/harelgrecht/kr260-linux-roadmap) | Replaced Xilinx GPIO driver with a custom kernel module. Controlled GPIO directly from PL with memory-mapped IO. |
| 🛠 IN PROGRESS | [AXI UART Lite Driver](https://github.com/harelgrecht/kr260-linux-roadmap) | Polling-based driver to replace the default AXI UART Lite. Exposes `/dev/ttyCustomUART`. Includes user-space C library. |
| 🔜 PLANNED | [Ethernet PHY Expansion](https://github.com/harelgrecht/kr260-linux-roadmap) | Enables extra Ethernet ports by integrating PHYs in PL and modifying DTS. Learning DTS structure and PHY reset flows. |

---

## 🧠 Future Projects (Ideas)

- Will be filled in the futre 

---

## 🧰 Tools Used

- PetaLinux, Yocto
- Vivado + Vitis
- Linux Kernel Module APIs
- Git, GitHub
- Kria KR260 Starter Kit

---

## 💼 Author

**Harel**  
Linux Kernel & Embedded Software Developer in Progress  
➡️ [GitHub](https://github.com/harelgrecht)  

---

## 📌 Notes

Every project includes:
- Git repo with full source code
- Clean README with architecture, usage, and test instructions
- Highlights of what was learned

