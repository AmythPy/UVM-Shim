use uvm_shim::{Console, BlockDevice, PlatformInfo};

struct MemConsole;
impl Console for MemConsole {
    fn write_bytes(&mut self, bytes: &[u8]) {
        if let Ok(s) = std::str::from_utf8(bytes) {
            print!("{}", s);
        }
    }

    fn read_byte(&mut self) -> Option<u8> { None }
}

struct MemBlockDevice { data: Vec<u8> }
impl MemBlockDevice {
    fn new(sectors: usize, sector_size: usize) -> Self {
        Self { data: vec![0u8; sectors * sector_size] }
    }
}

impl BlockDevice for MemBlockDevice {
    fn read_sector(&self, lba: u64, buf: &mut [u8]) -> Result<usize, uvm_shim::HalError> {
        let sector_size = buf.len();
        let start = (lba as usize) * sector_size;
        if start + sector_size > self.data.len() { return Err(uvm_shim::HalError::NotSupported); }
        buf.copy_from_slice(&self.data[start..start+sector_size]);
        Ok(sector_size)
    }

    fn write_sector(&mut self, lba: u64, buf: &[u8]) -> Result<usize, uvm_shim::HalError> {
        let sector_size = buf.len();
        let start = (lba as usize) * sector_size;
        if start + sector_size > self.data.len() { return Err(uvm_shim::HalError::NotSupported); }
        self.data[start..start+sector_size].copy_from_slice(buf);
        Ok(sector_size)
    }
}

fn main() {
    let mut console = MemConsole;
    let mut disk = MemBlockDevice::new(1024, 512);

    let platform = PlatformInfo { memory_size_bytes: 512 * 1024 * 1024, cpu_cores: 4 };

    console.write_bytes(b"UVM shim booting CompileOS (example)\n");

    // Example: write and read a sector
    let write_buf = [0xAAu8; 512];
    match disk.write_sector(0, &write_buf) {
        Ok(n) => console.write_bytes(format!("Wrote {} bytes to sector 0\n", n).as_bytes()),
        Err(_) => console.write_bytes(b"Disk write failed\n"),
    }
    let mut read_buf = [0u8; 512];
    match disk.read_sector(0, &mut read_buf) {
        Ok(n) => console.write_bytes(format!("Read {} bytes from sector 0, first byte=0x{:02X}\n", n, read_buf[0]).as_bytes()),
        Err(_) => console.write_bytes(b"Disk read failed\n"),
    }

    // Hand-off placeholder: in a real bootstrapper we'd jump to kernel entry with platform info and device handles
    console.write_bytes(b"Handing off to CompileOS kernel (simulated)\n");
}

// Install rustup (interactive installer)
// Set-ExecutionPolicy Bypass -Scope Process -Force; iex ((New-Object System.Net.WebClient).DownloadString('https://sh.rustup.rs'))

// Make sure stable toolchain is active
// rustup default stable

// From the repo root
// cargo build -p uvm-run
// cargo run -p uvm-run
