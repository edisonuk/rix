#pragma once

#include <stdint.h>
#include "../types.h"

void *balloc(size_t len);

extern uintptr_t boot_alloc_start;
extern uintptr_t boot_alloc_end;