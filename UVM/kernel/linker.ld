ENTRY(uvm_boot_main)

PHDRS {
  text PT_LOAD FLAGS(5); /* rx */
  data PT_LOAD FLAGS(6); /* rw */
}

SECTIONS
{
  . = 0x100000;
  .multiboot : { *(.multiboot) }

  .text : {
    *(.text .text.*)
    *(.rodata .rodata.*)
  } :text

  .data : {
    *(.data .data.*)
    *(.bss .bss.* COMMON)
  } :data

  /DISCARD/ : { *(.eh_frame) }
}
