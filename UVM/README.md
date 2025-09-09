# UVM - Universal Virtual Machine shim for CompileOS

This repository contains a scaffold for a Universal Virtual Machine (UVM) shim that serves as both a Hardware Abstraction Layer (HAL) and a Machine Abstraction Layer (MAL) for the CompileOS project.

This branch uses a Multiboot+GRUB flow for initial development and testing. The repo now contains a C HAL/MAL header and a minimal multiboot test kernel you can build and boot in QEMU.

Contents:
- `uvm-shim/uvm_shim.h` - C HAL/MAL header describing the ABI between bootloader and kernel.
- `kernel/` - Minimal multiboot test kernel (C) and linker script.
- `iso/boot/grub/` - GRUB configuration used to build a bootable ISO.
- `uvm-boot/` - Placeholder C shim that will become the UVM bootstrapper in a later step.

Goal: provide a hardware-agnostic bootstrapper for CompileOS. Extend `uvm_shim.h` with additional interfaces (timer, interrupt controller, PCI, etc.) and implement platform-specific adapters in C. Next steps add a real UVM boot shim that constructs `PlatformInfo` and hands control to the kernel.

Project: UVM (tiny multiboot shim + kernel)

Quick start (use WSL/Ubuntu):

# install once in WSL
sudo apt update
sudo apt install -y build-essential gcc-multilib grub-pc-bin xorriso qemu-system-x86

# build
cd /mnt/c/Users/ccoll/OneDrive/Desktop/UVM
make clean
make all

# run interactively (see serial on terminal)
make run

# headless capture (example)
SER=/tmp/uvm_serial.log
qemu-system-x86_64 -cdrom /tmp/compileos.iso -m 512 -boot d -serial file:$SER -nographic &
PID=$!; sleep 8; kill $PID || true
sed -n '1,200p' $SER

Notes:
- Build/run MUST be done in WSL (Linux toolchain). Do not run 'make' from PowerShell.
- If the repo is on a Windows-mounted drive (OneDrive), the ISO will be created in /tmp and may not be copied back automatically. Look for /tmp/compileos.iso in WSL.
