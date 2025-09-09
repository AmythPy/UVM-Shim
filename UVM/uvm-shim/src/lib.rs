#![allow(dead_code)]

use std::ffi::c_void;

/// Console HAL (FFI-friendly): opaque pointer handles expected on the C side.
pub type ConsoleHandle = *mut c_void;
pub type BlockDeviceHandle = *mut c_void;

/// PlatformInfo passed between bootloader and kernel (C-compatible layout).
#[repr(C)]
pub struct PlatformInfo {
    pub console: ConsoleHandle,
    pub disk: BlockDeviceHandle,
    // future: memory map pointer, timers, irq controllers, pci list, etc.
}

/// Kernel entry type expected by the bootloader (C ABI).
pub type KernelEntry = extern "C" fn(*const PlatformInfo) -> ();

/// High-level Rust traits (for Rust consumers only).
pub trait Console {
    fn write_str(&mut self, s: &str);
    fn read_line(&mut self, buf: &mut String) -> usize;
}

pub trait BlockDevice {
    fn read_sector(&self, lba: u64, buf: &mut [u8]) -> Result<(), &'static str>;
    fn write_sector(&mut self, lba: u64, buf: &[u8]) -> Result<(), &'static str>;
}

// Small helper adapter: convert a ConsoleHandle to an API call via a user-supplied
// function pointer if you implement that on the C side (left as a placeholder).