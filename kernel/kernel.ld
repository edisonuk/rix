ENTRY(_start)

SECTIONS {
    /* TODO: define KERNEL_BASE, KERNEL_LOAD_OFFSET */
    . = KERNEL_BASE + KERNEL_LOAD_OFFSET;

    .text   : {
        __text_start = .;
        KEEP(*(.text.boot))
        *(.text*)
    }
    
    /* 
     * Max page size is 64KiB on ARM architecture,
     * but only 4KiB on x64
     */
    . = ALIGN(CONSTANT(MAXPAGESIZE));
    __code_end = .;

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    /* read-only data */
    .rodata : ALIGN(CONSTANT(MAXPAGESIZE)) {
		__rodata_start = .;
        *(.rodata)
        __rodata_end = .;
	}

    /* read-write data (initialized variables) */
    .data   : ALIGN(CONSTANT(MAXPAGESIZE)) {
        __data_start = .;
		*(.data)
        __data_end = .;
	}

    /* read-write data (uninitialized variables) and stack */
    .bss    : ALIGN(CONSTANT(MAXPAGESIZE)) {
        __bss_start = .;
        *(.bss)
		*(COMMON)
        __bss_end = .;
	}

	/DISCARD/: {
		*(.comment)
	}

    __end = .;
}
