/* SPDX-License-Identifier: Apache-2.0 */
/****************************************************************************
 * boards/arm/pic32czca90/pic32czca90-curiosity/scripts/linker.ld
 *
 * PIC32CZ CA80/CA90 Curiosity Ultra — standalone NuttX linker script
 *
 * Boot Flash Memory (BFM): 128 KB at 0x08000000 — vector table lives here
 * Program Flash Memory (PFM):  8 MB at 0x0C000000 — code and rodata
 * SRAM:                      832 KB at 0x20020000 — heap, stack, data
 * nocache:                    64 KB at 0x200F0000 — DMA buffers (uncached)
 *
 * Boot sequence: BootROM (0x04000000) reads SP=[BFM+0], PC=[BFM+4] (→ __start
 * in PFM), sets VTOR=0x08000000, then jumps. Matches Harmony VECTOR_REGION=boot_rom.
 *
 * CA80 vs CA90: identical memory map; CA90 adds HSM (Hardware Security Module).
 *
 * NOTE: This file is the standalone NuttX linker script (not used by the PX4
 * build). The PX4 build uses nuttx-config/scripts/script.ld instead. Both
 * files must be kept in sync.
 ****************************************************************************/

MEMORY
{
    boot_rom (rx)  : ORIGIN = 0x08000000, LENGTH = 128K  /* BFM — vector table */
    flash    (rx)  : ORIGIN = 0x0C000000, LENGTH = 8M    /* PFM — code          */
    sram     (rwx) : ORIGIN = 0x20020000, LENGTH = 832K
    nocache  (rwx) : ORIGIN = 0x200F0000, LENGTH = 64K
}

OUTPUT_ARCH(arm)
EXTERN(_vectors)
ENTRY(__start)

SECTIONS
{
    /* Vector table: placed at BFM start (0x08000000).
     * BootROM reads [BFM+0]=SP and [BFM+4]=PC (= __start in PFM) before jumping.
     */
    .vectors :
    {
        KEEP(*(.vectors))
    } > boot_rom

    .text :
    {
        _stext = ABSOLUTE(.);
        *(.text .text.*)
        *(.fixup)
        *(.gnu.warning)
        *(.rodata .rodata.*)
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.got)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        _etext = ABSOLUTE(.);
    } > flash

    .init_section :
    {
        _sinit = ABSOLUTE(.);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array .ctors))
        _einit = ABSOLUTE(.);
    } > flash

    .ARM.extab :
    {
        *(.ARM.extab*)
    } > flash

    .ARM.exidx :
    {
        __exidx_start = ABSOLUTE(.);
        *(.ARM.exidx*)
        __exidx_end = ABSOLUTE(.);
    } > flash

    _eronly = ABSOLUTE(.);

    .data :
    {
        _sdata = ABSOLUTE(.);
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        CONSTRUCTORS
        . = ALIGN(4);
        _edata = ABSOLUTE(.);
    } > sram AT > flash

    .ramfunc ALIGN(4):
    {
        _sramfuncs = ABSOLUTE(.);
        *(.ramfunc .ramfunc.*)
        _eramfuncs = ABSOLUTE(.);
    } > sram AT > flash

    _framfuncs = LOADADDR(.ramfunc);

    .bss :
    {
        _sbss = ABSOLUTE(.);
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = ABSOLUTE(.);
    } > sram

    .nocache (NOLOAD) :
    {
        . = ALIGN(32);
        _s_nocache = ABSOLUTE(.);
        *(.nocache .nocache.*)
        . = ALIGN(32);
        _e_nocache = ABSOLUTE(.);
    } > nocache

    /* Stabs debugging sections */
    .stab 0 : { *(.stab) }
    .stabstr 0 : { *(.stabstr) }
    .stab.excl 0 : { *(.stab.excl) }
    .stab.exclstr 0 : { *(.stab.exclstr) }
    .stab.index 0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment 0 : { *(.comment) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_info 0 : { *(.debug_info) }
    .debug_line 0 : { *(.debug_line) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_aranges 0 : { *(.debug_aranges) }
}
