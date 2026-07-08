/* Linker script minimal pentru ESP32 */

MEMORY
{
  /* IRAM: Instrucțiuni care pot fi executate direct din RAM */
  iram0_0_seg (RX) : org = 0x40080000, len = 0x20000
  
  /* DRAM: Date/variabile utilizate de program */
  dram0_0_seg (RW) : org = 0x3FFB0000, len = 0x20000
  
  /* IROM: Memoria Flash unde stă tot codul programului */
  irom0_0_seg (RX) : org = 0x400D0020, len = 0x330000
}

SECTIONS
{
  /* Secțiunea de cod (.text) se duce în memoria Flash (IROM) */
  .text : {
    *(.text .text.*)
  } > irom0_0_seg

  /* Datele inițializate (.data) se duc în RAM (DRAM) */
  .data : {
    *(.data .data.*)
  } > dram0_0_seg

  /* Variabilele neinițializate (.bss) se duc tot în RAM (DRAM) */
  .bss : {
    *(.bss .bss.*)
  } > dram0_0_seg
}