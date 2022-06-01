/*
 * Check CPU for long mode support
 * 
 * Returns status in %eax register
 *      0: Not Available
 *      1: Available
 */
BEGIN_FUNCTION(check_long_mode)
    pushf   /* save flags */
    push $0 /* zero the flags */
    popf

    /* check for cpuid availibility */
    mov $0, %eax
    cpuid
    cmp $1, %eax
    jb .Lno_long_mode  /* no cpuid */

    /* check for extended CPUID functions */
    mov $0x80000000, %eax
    cpuid
    cmp $0x80000001, %eax
    jb .Lno_long_mode  /* long mode not supported */

    mov $0x80000001, %eax
    cpuid
    test $0x20000000, %edx
    jz .Lno_long_mode

    /* long mode available */
    popf /* restore flags */
    xor $1, %eax
    ret

.Lno_long_mode:
    popf /* restore flags */
    mov %eax, %eax
    ret
END_FUNCTION(check_long_mode)