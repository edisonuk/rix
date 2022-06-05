#ifndef _ASPACE_H_
#define _ASPACE_H_ 

/* Virutal address where the kernel address space begins.
 * Below this is the user address space.
 */
#define KERNEL_ASPACE_BASE 0xffffff8000000000UL
#define KERNEL_ASPACE_SIZE 0x0000008000000000UL

/* Virtual address where the user address space begins.
 * Below this is wholly inaccessible.
 */
#define USER_ASPACE_BASE 0x0000000000200000UL
#define USER_ASPACE_SIZE  

#endif /* _ASPACE_H_ */
