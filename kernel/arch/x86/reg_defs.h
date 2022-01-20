#ifndef _X86_REG_DEFS_H_
#define _X86_REG_DEFS_H_

/* Control Register 0 */
#define CR0_PE 0x00000001   /* Protected Mode Enable    */
#define CR0_PG 0x80000000   /* Paging enabled           */

/* Control Register 4 */
#define CR4_PAE 0x00000020  /* Physical Address Extensions */

/* Memory specific register */
#define IA32_MSR_EFER       0xC0000080
#define IA32_MSR_EFER_LME   0x00000100

#endif /* _X86_REG_DEFS_H_ */