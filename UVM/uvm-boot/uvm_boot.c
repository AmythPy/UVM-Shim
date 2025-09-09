/* uvm-boot: multiboot entry shim. This module is the multiboot-loaded binary
   that constructs PlatformInfo and calls compileos_main (in kernel).
*/

#include "../uvm-shim/uvm_shim.h"

/* Multiboot header is supplied by uvm-boot/multiboot.S to ensure it
   appears very early in the output file where GRUB looks for it. */

typedef unsigned short u16;

volatile u16* vga = (volatile u16*)0xB8000;

/* VGA writer (kept for local display) */
static void vga_write_str(Console *self, const char *s) {
   (void)self;
   u16 pos = 0;
   while (*s) {
     vga[pos++] = (u16)(*s++) | 0x0700;
   }
}

static int vga_read_line(Console *self, char *buf, size_t max) {
   (void)self; (void)buf; (void)max;
   return 0;
}

/* Simple port I/O for serial console */
static inline void outb(unsigned short port, unsigned char val) {
   asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
   unsigned char ret;
   asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
   return ret;
}

/* Initialize COM1 (0x3F8) serial port at 38400, 8N1 */
static void serial_init(void) {
   unsigned short port = 0x3F8;
   outb(port + 1, 0x00);    // Disable all interrupts
   outb(port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(port + 0, 0x03);    // Divisor low byte (3 => 38400)
   outb(port + 1, 0x00);    // Divisor high byte
   outb(port + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static void serial_write_char(char c) {
   unsigned short port = 0x3F8;
   while ((inb(port + 5) & 0x20) == 0) {
      asm volatile ("pause");
   }
   outb(port, (unsigned char)c);
}

static void serial_write_str(Console *self, const char *s) {
   (void)self;
   while (*s) serial_write_char(*s++);
}

/* Non-blocking read: return byte or -1 if none */
static int serial_read_byte(void) {
   unsigned short port = 0x3F8;
   unsigned char lsr = inb(port + 5);
   if (lsr & 0x01) return (int)inb(port);
   return -1;
}

/* Read a line from serial (echoing); returns length */
static int serial_read_line(Console *self, char *buf, size_t max) {
   (void)self;
   size_t idx = 0;
   while (idx + 1 < max) {
      int c = serial_read_byte();
      if (c == -1) {
         for (volatile int i = 0; i < 10000; ++i) asm volatile ("nop");
         continue;
      }
      if (c == '\r' || c == '\n') {
         buf[idx] = '\0';
         serial_write_str(NULL, "\r\n");
         return (int)idx;
      }
      /* backspace handling */
      if (c == 0x08 && idx > 0) {
         idx--;
         serial_write_str(NULL, "\b \b");
         continue;
      }
      serial_write_char((char)c);
      buf[idx++] = (char)c;
   }
   buf[idx] = '\0';
   return (int)idx;
}

#define SECTOR_SIZE 512
static int memdisk_read(BlockDevice *self, uint64_t lba, void *buf, size_t size) {
   (void)self;
   if (!buf) return -1;
   /* clamp size to SECTOR_SIZE to be safe */
   if (size > SECTOR_SIZE) size = SECTOR_SIZE;
   unsigned char *b = (unsigned char*)buf;
   /* Fill with a simple pattern derived from LBA so kernel can inspect bytes */
   for (size_t i = 0; i < size; ++i) b[i] = (unsigned char)(((lba + i) & 0xFF));
   /* Always return success for this in-memory test device */
   return 0;
}

static int memdisk_write(BlockDevice *self, uint64_t lba, const void *buf, size_t size) {
   (void)self; (void)lba; (void)buf; (void)size;
   return -1;
}

/* extern kernel entry implemented in kernel/kernel.c */
extern void compileos_main(PlatformInfo *platform);

void uvm_boot_main(void) {
   /* initialize serial console and use it so we can capture output via -serial stdio */
   serial_init();
   static Console console = { .write_str = serial_write_str, .read_line = serial_read_line, .ctx = NULL };
   static BlockDevice disk = { .read_sector = memdisk_read, .write_sector = memdisk_write, .ctx = NULL };
   static Timer timer = { .set_periodic = NULL, .ticks = NULL, .ctx = NULL };
   static IrqController irq = { .enable_irq = NULL, .disable_irq = NULL, .ctx = NULL };

   PlatformInfo p = { .console = &console, .disk = &disk, .timer = &timer, .irq = &irq, .mmio = NULL, .pci = NULL };


   /* Early debug output to ensure we see something on serial/VGA before handoff */
   serial_write_str(&console, "UVM: serial initialized\n");
   vga_write_str(&console, "UVM: vga initialized\n");

   /* Heartbeat: emit several messages now to ensure QEMU serial capture picks them up */
   for (int i = 0; i < 6; ++i) {
      serial_write_str(&console, "UVM-BOOT: heartbeat\n");
      vga_write_str(&console, "UVM-BOOT: heartbeat\n");
      /* small delay */
      for (volatile unsigned long j = 0; j < 2000000UL; ++j) asm volatile ("nop");
   }

   serial_write_str(&console, "UVM: handing off to kernel\n");

   /* Offer a single-line prompt on serial for demo purposes */
   char line[64];
   serial_write_str(&console, "UVM: enter boot arg (or wait): ");
   int got = console.read_line(&console, line, sizeof(line));
   if (got > 0) {
      serial_write_str(&console, "UVM: got input: ");
      serial_write_str(&console, line);
      serial_write_str(&console, "\n");
   } else {
      serial_write_str(&console, "UVM: no input, continuing\n");
   }

   /* Handoff to kernel */
   compileos_main(&p);

   for (;;) {
      asm volatile ("hlt");
   }
}
