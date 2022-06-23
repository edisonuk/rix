#ifndef _VM_PAGE_H_
#define _VM_PAGE_H_

#include "../list.h"

/**
 * @brief   Structure for each page.
 */
typedef struct vm_page {
    list_node_t node;
    
} vm_page_t;

#endif /* _VM_PAGE_H_ */