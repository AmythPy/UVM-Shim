/* Multiboot header placed at the start of .text so it will be inside the
    first PT_LOAD within the first 8KiB of the file where GRUB searches. */
.section .text,"ax",@progbits
.global multiboot_header
.type multiboot_header, @object
multiboot_header:
   .long 0x1BADB002
   .long 0x00010003
   .long -(0x1BADB002 + 0x00010003)
.size multiboot_header, .-multiboot_header
