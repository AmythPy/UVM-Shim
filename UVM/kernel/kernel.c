/* Kernel core: compileos_main consumes PlatformInfo and implements kernel logic.
   The actual boot shim (uvm-boot) provides PlatformInfo and calls compileos_main.
*/

#include "../uvm-shim/uvm_shim.h"

#define SECTOR_SIZE 512

/* Tiny hex-print helper used instead of snprintf (no libc in freestanding kernel) */
static void console_print_byte_hex(Console *c, unsigned char b) {
    if (!c || !c->write_str) return;
    char tmp[6];
    const char *hex = "0123456789ABCDEF";
    tmp[0] = '0';
    tmp[1] = 'x';
    tmp[2] = hex[(b >> 4) & 0xF];
    tmp[3] = hex[b & 0xF];
    tmp[4] = '\n';
    tmp[5] = '\0';
    c->write_str(c, tmp);
}

/* Kernel-side entry that consumes PlatformInfo handed by UVM shim */
void compileos_main(PlatformInfo *platform) {
    if (platform && platform->console && platform->console->write_str) {
        platform->console->write_str(platform->console, "CompileOS: Hello from compileos_main\n");
    }

    unsigned char sector[SECTOR_SIZE];
    if (platform && platform->disk && platform->disk->read_sector) {
        int r = platform->disk->read_sector(platform->disk, 0, sector, SECTOR_SIZE);
        if (r == 0) {
            if (platform && platform->console && platform->console->write_str) {
                platform->console->write_str(platform->console, "CompileOS: sector0[0..15]=\n");
                /* print first 16 bytes in hex */
                for (int i = 0; i < 16; ++i) {
                    console_print_byte_hex(platform->console, sector[i]);
                }
            }
        } else {
            if (platform && platform->console && platform->console->write_str) {
                platform->console->write_str(platform->console, "CompileOS: failed to read sector 0\n");
            }
        }
    } else {
        if (platform && platform->console && platform->console->write_str) {
            platform->console->write_str(platform->console, "CompileOS: no disk HAL\n");
        }
    }

    /* Heartbeat: print dots and a message so we can see activity */
    if (platform && platform->console && platform->console->write_str) {
        for (int round = 0; round < 3; ++round) {
            platform->console->write_str(platform->console, "HB:");
            for (int i = 0; i < 8; ++i) {
                platform->console->write_str(platform->console, ".");
                for (volatile unsigned long j = 0; j < 2000000UL; ++j) asm volatile ("nop");
            }
            platform->console->write_str(platform->console, "\n");
        }
        platform->console->write_str(platform->console, "CompileOS: heartbeat complete\n");
    }

    for (;;) {
        asm volatile ("hlt");
    }
}
