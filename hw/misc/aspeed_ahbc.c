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

#include "qemu/osdep.h"
#include "hw/misc/aspeed_ahbc.h"
#include "hw/qdev-properties.h"
#include "qapi/error.h"
#include "qapi/visitor.h"
#include "qemu/bitops.h"
#include "qemu/log.h"
#include "trace.h"

#define AHBC_PROT_KEY_UNLOCK 0xAEED1A03
#define AHBC_IO_REGION_SIZE 0x90

static uint64_t aspeed_ahbc_read(void *opaque, hwaddr offset, unsigned size)
{
    uint32_t val;
    AspeedAHBCState *s = ASPEED_AHBC(opaque);

    switch (offset) {
    case 0x00: /* Protection Key */
        /* TODO: The datasheet claims this reads as 0/1. Other aspeed models
           implement this as reading the protection key. Verify which is
           correct */
        val = s->unlocked;
        break;
    case 0x80:
        val = s->priority;
        break;
    case 0x8C:
        val = s->sdram_remapped;
        break;
    case 0x88: /* Interrupt Control */
        val = 0;
        qemu_log_mask(LOG_UNIMP, "%s: AHB Interrupts not modeled!\n",
            __func__);
        break;
    default:
        val = 0;
        qemu_log_mask(LOG_GUEST_ERROR,
              "%s: Bad register at offset 0x%" HWADDR_PRIx "\n",
              __func__, offset);
    }

    return val;
}

static void aspeed_ahbc_sdram_remap(AspeedAHBCState *s, bool enabled)
{
    memory_region_set_enabled(&s->boot_rom, !enabled);
    memory_region_set_enabled(&s->sdram_alias, enabled);
}

static void aspeed_ahbc_write(void *opaque, hwaddr offset, uint64_t data,
                             unsigned size)
{
    AspeedAHBCState *s = ASPEED_AHBC(opaque);

    if (offset > 0x0 && !s->unlocked) {
        qemu_log_mask(LOG_GUEST_ERROR, "%s: AHBC is locked!\n", __func__);
        return;
    }

    switch (offset) {
    case 0x00: /* Protection Key */
        s->unlocked = data == AHBC_PROT_KEY_UNLOCK;
        break;
    case 0x80: /* Priority Control */
        s->priority = data;
        break;
    case 0x8C: {
        bool sdram_remap_after = (data & 0x1);
        if (sdram_remap_after != s->sdram_remapped) {
            aspeed_ahbc_sdram_remap(s, sdram_remap_after);
        }
        s->sdram_remapped = sdram_remap_after;
        if (data & ~0x1) {
            qemu_log_mask(LOG_UNIMP, "%s: PCI remapping not implemented!\n",
                __func__);
        }
        break;
    }
    case 0x88: /* Interrupt Control */
        qemu_log_mask(LOG_UNIMP, "%s: AHB Interrupts not modeled!\n",
            __func__);
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
              "%s: Bad register at offset 0x%" HWADDR_PRIx "\n",
              __func__, offset);
    }
}

static const MemoryRegionOps aspeed_ahbc_ops = {
    .read = aspeed_ahbc_read,
    .write = aspeed_ahbc_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid.min_access_size = 4,
    .valid.max_access_size = 4,
    .valid.unaligned = false,
};


static void aspeed_ahbc_realize(DeviceState *dev, Error **errp)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    AspeedAHBCState *s = ASPEED_AHBC(dev);

    memory_region_init_io(&s->ctrl_mmio, OBJECT(s), &aspeed_ahbc_ops, s,
                          TYPE_ASPEED_AHBC, AHBC_IO_REGION_SIZE);

    sysbus_init_mmio(sbd, &s->ctrl_mmio);
}

static void aspeed_ahbc_reset(DeviceState *dev)
{
    AspeedAHBCState *s = ASPEED_AHBC(dev);

    s->unlocked = 0;
    s->priority = 0;
    s->sdram_remapped = 0;
}


static const VMStateDescription vmstate_aspeed_ahbc = {
    .name = "aspeed.ahbc",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_BOOL(unlocked, AspeedAHBCState),
        VMSTATE_BOOL(sdram_remapped, AspeedAHBCState),
        VMSTATE_UINT32(priority, AspeedAHBCState),
        VMSTATE_END_OF_LIST()
    }
};

static void aspeed_ahbc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = aspeed_ahbc_realize;
    dc->reset = aspeed_ahbc_reset;
    dc->desc = "ASPEED AHB Bus Controller";
    dc->vmsd = &vmstate_aspeed_ahbc;
}

static const TypeInfo aspeed_ahbc_info = {
    .name = TYPE_ASPEED_AHBC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(AspeedAHBCState),
    .class_init = aspeed_ahbc_class_init,
};

static void aspeed_ahbc_register_types(void)
{
    type_register_static(&aspeed_ahbc_info);
}

type_init(aspeed_ahbc_register_types);
