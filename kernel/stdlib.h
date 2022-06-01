#pragma once

#define ROUNDUP(addr, alignment)    (((addr) + ((alignment)-1)) & ~((alignment)-1))
#define ROUNDDOWN(addr, alignment)  ((addr) & ~((alignment)-1))