/* uvm_shim.h - C HAL/MAL header for UVM and CompileOS */
#ifndef UVM_SHIM_H
#define UVM_SHIM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Console HAL: function-pointer interface */
typedef struct Console {
    void (*write_str)(struct Console *self, const char *s);
    int  (*read_line)(struct Console *self, char *buf, size_t max);
    void *ctx;
} Console;

/* BlockDevice HAL */
typedef struct BlockDevice {
    int (*read_sector)(struct BlockDevice *self, uint64_t lba, void *buf, size_t size);
    int (*write_sector)(struct BlockDevice *self, uint64_t lba, const void *buf, size_t size);
    void *ctx;
} BlockDevice;

/* Timer HAL */
typedef struct Timer {
    /* set periodic interval in milliseconds; 0 to disable */
    void (*set_periodic)(struct Timer *self, unsigned ms);
    /* read a monotonic tick counter */
    uint64_t (*ticks)(struct Timer *self);
    void *ctx;
} Timer;

/* IRQ controller HAL */
typedef struct IrqController {
    /* enable/disable irq line */
    void (*enable_irq)(struct IrqController *self, unsigned int irq);
    void (*disable_irq)(struct IrqController *self, unsigned int irq);
    void *ctx;
} IrqController;

/* PlatformInfo passed to kernel */
typedef struct PlatformInfo {
    Console    *console;
    BlockDevice *disk;
    /* Timer HAL */
    struct Timer *timer;
    /* Interrupt controller HAL */
    struct IrqController *irq;
    /* MMIO access helper (opaque) */
    void *mmio;
    /* PCI device list (opaque) */
    void *pci;
    /* future: memory map, additional devices */
} PlatformInfo;

/* Kernel entry type */
typedef void (*kernel_entry_t)(PlatformInfo *platform);

#ifdef __cplusplus
}
#endif

#endif /* UVM_SHIM_H */
