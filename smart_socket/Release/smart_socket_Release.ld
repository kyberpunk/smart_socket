/*
 * GENERATED FILE - DO NOT EDIT
 * (c) Code Red Technologies Ltd, 2008-2013
 * (c) NXP Semiconductors 2013-2018
 * Generated linker script file for MKW41Z512xxx4
 * Created from linkscript.ldt by FMCreateLinkLibraries
 * Using Freemarker v2.3.23
 * MCUXpresso IDE v10.0.2 [Build 411] [2017-07-11]  on 19.5.2018 14:11:54
 */

INCLUDE "smart_socket_Release_library.ld"
INCLUDE "smart_socket_Release_memory.ld"

ENTRY(ResetISR)

SECTIONS
{
    /* MAIN TEXT SECTION */
    .text : ALIGN(4)
    {
        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        KEEP(*(.isr_vector))
        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        __bss_section_table_end = .;
        __section_table_end = . ;
	    /* End of Global Section Table */

        *(.after_vectors*)


        /* Kinetis Flash Configuration data */
        . = 0x400 ;
        PROVIDE(__FLASH_CONFIG_START__ = .) ;
        KEEP(*(.FlashConfig))
        PROVIDE(__FLASH_CONFIG_END__ = .) ;
        ASSERT(!(__FLASH_CONFIG_START__ == __FLASH_CONFIG_END__), "Linker Flash Config Support Enabled, but no .FlashConfig section provided within application");
        /* End of Kinetis Flash Configuration data */
        

    } >PROGRAM_FLASH

    .text : ALIGN(4)
    {
        *(.text*)
        *(.rodata .rodata.* .constdata .constdata.*)
        . = ALIGN(4);
			/* C++ constructors etc */
			. = ALIGN(4);
			KEEP(*(.init))
			
			. = ALIGN(4);
			__preinit_array_start = .;
			KEEP (*(.preinit_array))
			__preinit_array_end = .;
			
			. = ALIGN(4);
			__init_array_start = .;
			KEEP (*(SORT(.init_array.*)))
			KEEP (*(.init_array))
			__init_array_end = .;
			
			KEEP(*(.fini));
			
			. = ALIGN(4);
			KEEP (*crtbegin.o(.ctors))
			KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
			KEEP (*(SORT(.ctors.*)))
			KEEP (*crtend.o(.ctors))
			
			. = ALIGN(4);
			KEEP (*crtbegin.o(.dtors))
			KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
			KEEP (*(SORT(.dtors.*)))
			KEEP (*crtend.o(.dtors))
			. = ALIGN(4);
			/* End C++ */
    } > PROGRAM_FLASH
    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this. 
     */
    .ARM.extab : ALIGN(4) 
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > PROGRAM_FLASH
    __exidx_start = .;

    .ARM.exidx : ALIGN(4)
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > PROGRAM_FLASH
    __exidx_end = .;

    _etext = .;
        
    /* MAIN DATA SECTION */
        /* Default MTB section */
        .mtb_buffer_default (NOLOAD) :
        {
           KEEP(*(.mtb*))
        } > SRAM
    .uninit_RESERVED : ALIGN(4)
    {
        KEEP(*(.bss.$RESERVED*))
        . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > SRAM
    /* Main DATA section (SRAM) */
    .data : ALIGN(4)
    {
       FILL(0xff)
       _data = . ;
       *(vtable)
       *(.ramfunc*)
       *(.data*)
       . = ALIGN(4) ;
       _edata = . ;
    } > SRAM AT>PROGRAM_FLASH
    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > SRAM
    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*) 
         . = ALIGN(4) ;
        _end_noinit = .;
    } > SRAM
    .heap :  ALIGN(4)
    {
        _pvHeapStart = .;
        . += 0x4000;
		. = ALIGN(4);
		_pvHeapLimit = .;
    } > SRAM
       .heap2stackfill  : 
	{
		. += 0x4000;
	} > SRAM
	.stack ORIGIN(SRAM) + LENGTH(SRAM) - 0x4000 - 0:  ALIGN(4)
	{
		_vStackBase = .;
		. = ALIGN(4);
		_vStackTop = . + 0x4000;
	} > SRAM
}