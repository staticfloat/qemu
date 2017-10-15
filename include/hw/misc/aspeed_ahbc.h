/*
 * ASPEED AHB Bus Controller
 *
 * Keno Fischer <keno@juliacomputing.com>
 *
 * Copyright 2017 Julia Computing Inc
 *
 * This code is licensed under the GPL version 2 or later.  See
 * the COPYING file in the top-level directory.
 */
#ifndef ASPEED_AHBC_H
#define ASPEED_AHBC_H

#include "hw/sysbus.h"

#define TYPE_ASPEED_AHBC "aspeed.ahbc"
#define ASPEED_AHBC(obj) OBJECT_CHECK(AspeedAHBCState, (obj), TYPE_ASPEED_AHBC)

typedef struct AspeedAHBCState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion ctrl_mmio;

    bool unlocked;
    bool sdram_remapped;

    /*
     * The AHBC allows each AHB master to be assigned either high
     * or low priority according to the bits in this register.
     * We model the register, but it has otherwise no effect
     */
    uint32_t priority;

    /* The AHB controller arbitrates which of these two
       get mapped at address 0 */
    MemoryRegion boot_rom;
    MemoryRegion sdram_alias;
} AspeedAHBCState;


#endif /* ASPEED_SCU_H */
