ROOT := $(CURDIR)
KERNEL_DIR := kernel
BOOT_DIR := uvm-boot
ISO_DIR := iso
OUT_ISO := compileos.iso
TMP_ISO := /tmp/compileos.iso
KERNEL_BIN := $(KERNEL_DIR)/kernel.bin

CC := gcc
LD := $(CC)
CFLAGS := -m32 -ffreestanding -O2 -Wall -Wextra -I.
LDFLAGS := -m32 -T $(KERNEL_DIR)/linker.ld -nostdlib

.PHONY: all kernel iso run clean

all: kernel iso

kernel:
	# compile objects
	$(CC) $(CFLAGS) -c $(KERNEL_DIR)/kernel.c -o $(KERNEL_DIR)/kernel.o
	$(CC) $(CFLAGS) -c $(BOOT_DIR)/uvm_boot.c -o $(BOOT_DIR)/uvm_boot.o
	# link ELF
	$(LD) $(LDFLAGS) $(BOOT_DIR)/uvm_boot.o $(KERNEL_DIR)/kernel.o -o $(KERNEL_BIN)
	# produce flat binary for GRUB to reliably find the multiboot header
	# objcopy may be absent on some systems; allow failure safely
	objcopy -O binary $(KERNEL_BIN) $(KERNEL_DIR)/kernel.raw || true

iso:
	# copy raw kernel into ISO tree if available, otherwise use ELF
	if [ -f $(KERNEL_DIR)/kernel.raw ]; then cp -f $(KERNEL_DIR)/kernel.raw $(ISO_DIR)/boot/kernel.bin; else cp -f $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin; fi
	# create ISO in WSL temp to avoid libburn permission issues on mounted Windows paths
	grub-mkrescue -o $(TMP_ISO) $(ISO_DIR) 2>/dev/null || xorriso -as mkisofs -R -J -o $(TMP_ISO) $(ISO_DIR)
	# try to copy ISO back to repo root; if it fails we keep the temp ISO in /tmp
	cp -f $(TMP_ISO) $(OUT_ISO) 2>/dev/null || echo "warning: could not copy $(TMP_ISO) to $(OUT_ISO); temp ISO remains at $(TMP_ISO)"

run:
	# interactive mode: serial -> stdio (recommended for debugging)
	qemu-system-x86_64 -cdrom $(TMP_ISO) -m 512 -boot d -nographic

clean:
	# do not auto-remove compileos.iso on Windows-mounted paths to avoid permission issues
	rm -f $(KERNEL_DIR)/*.o $(KERNEL_BIN) $(KERNEL_DIR)/kernel.raw

