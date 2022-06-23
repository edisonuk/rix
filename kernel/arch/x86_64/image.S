#include "../../asm.h"

.set MULTIBOOT_HEADER_MAGIC, 0x1badb002
.set MULTIBOOT_HEADER_FLAGS, 0x00010002

/* MULTIBOOT HEADER */
DATA(multiboot_header)
    .int MULTIBOOT_HEADER_MAGIC
    .int MULTIBOOT_HEADER_FLAGS
    .int -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

/* 32-bit entry code */
.section ".text.boot"
.code32
.global _start
_start:
    jmp real_start

real_start:
    mov $0xdeadbeef, %eax