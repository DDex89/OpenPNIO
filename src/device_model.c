#include "openpnio/device_model.h"

/* Provisional fixed 8DI / 8DO training configuration:
 * DAP slot 0, DI slot 1 / subslot 1, DO slot 2 / subslot 1.
 * The process image is byte-oriented and independent of platform GPIO. */
static const openpnio_submodule_descriptor_t catalog[] = {
    {OPENPNIO_LAB_SLOT_DAP, OPENPNIO_LAB_SUBSLOT,
     OPENPNIO_LAB_IDENT_DAP, OPENPNIO_LAB_IDENT_DAP, 0, 0},
    {OPENPNIO_LAB_SLOT_DAP, OPENPNIO_LAB_SUBSLOT_INTERFACE,
     OPENPNIO_LAB_IDENT_DAP, OPENPNIO_LAB_IDENT_PDEV_INTERFACE, 0, 0},
    {OPENPNIO_LAB_SLOT_DAP, OPENPNIO_LAB_SUBSLOT_PORT_1,
     OPENPNIO_LAB_IDENT_DAP, OPENPNIO_LAB_IDENT_PDEV_PORT_1, 0, 0},
    {OPENPNIO_LAB_SLOT_DI, OPENPNIO_LAB_SUBSLOT,
     OPENPNIO_LAB_IDENT_DI, OPENPNIO_LAB_IDENT_DI, 1, 0},
    {OPENPNIO_LAB_SLOT_DO, OPENPNIO_LAB_SUBSLOT,
     OPENPNIO_LAB_IDENT_DO, OPENPNIO_LAB_IDENT_DO, 0, 1}
};
size_t openpnio_lab_submodule_count(void)
{
    return sizeof catalog/sizeof catalog[0];
}
const openpnio_submodule_descriptor_t *openpnio_lab_submodule_at(size_t index)
{
    return index < openpnio_lab_submodule_count() ? &catalog[index] : 0;
}
const openpnio_submodule_descriptor_t *openpnio_lab_find_submodule(uint16_t slot,
                                                                  uint16_t subslot)
{
    size_t i;
    for (i=0;i<openpnio_lab_submodule_count();i++) {
        if (catalog[i].slot==slot && catalog[i].subslot==subslot) return &catalog[i];
    }
    return 0;
}
void openpnio_device_model_init(openpnio_device_model_t *model)
{
    if (!model) return;
    model->di_byte=0;
    model->do_byte=0; /* safe output default */
    model->operational=0;
}
int openpnio_device_model_set_di(openpnio_device_model_t *model, uint8_t input_byte)
{
    if (!model) return OPENPNIO_IO_INVALID_ARGUMENT;
    model->di_byte=input_byte;
    return OPENPNIO_IO_OK;
}
int openpnio_device_model_read_di(const openpnio_device_model_t *model,
                                 uint16_t slot, uint16_t subslot,
                                 uint8_t *destination, size_t length)
{
    if (!model || !destination) return OPENPNIO_IO_INVALID_ARGUMENT;
    if (slot!=OPENPNIO_LAB_SLOT_DI || subslot!=OPENPNIO_LAB_SUBSLOT)
        return OPENPNIO_IO_UNKNOWN_SUBMODULE;
    if (length!=1) return OPENPNIO_IO_WRONG_LENGTH;
    destination[0]=model->di_byte;
    return OPENPNIO_IO_OK;
}
void openpnio_device_model_set_operational(openpnio_device_model_t *model,
                                          int operational)
{
    if (!model) return;
    if (!operational) model->do_byte=0; /* STOP / AR loss / watchdog */
    model->operational=operational ? 1u : 0u;
}
int openpnio_device_model_write_do(openpnio_device_model_t *model,
                                  uint16_t slot, uint16_t subslot,
                                  const uint8_t *source, size_t length)
{
    if (!model || !source) return OPENPNIO_IO_INVALID_ARGUMENT;
    if (slot!=OPENPNIO_LAB_SLOT_DO || subslot!=OPENPNIO_LAB_SUBSLOT)
        return OPENPNIO_IO_UNKNOWN_SUBMODULE;
    if (length!=1) return OPENPNIO_IO_WRONG_LENGTH;
    if (!model->operational) return OPENPNIO_IO_NOT_OPERATIONAL;
    model->do_byte=source[0];
    return OPENPNIO_IO_OK;
}
uint8_t openpnio_device_model_get_do(const openpnio_device_model_t *model)
{
    return model ? model->do_byte : 0;
}
