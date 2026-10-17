#ifndef OPENPNIO_DEVICE_MODEL_H
#define OPENPNIO_DEVICE_MODEL_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Provisional, OpenPNIO-specific virtual module identities; NOT
 * registered manufacturer/device identifiers or a finalized GSDML. */
enum {
    OPENPNIO_LAB_SLOT_DAP = 0,
    OPENPNIO_LAB_SLOT_DI = 1,
    OPENPNIO_LAB_SLOT_DO = 2,
    OPENPNIO_LAB_SUBSLOT = 1,
    OPENPNIO_LAB_SUBSLOT_INTERFACE = 0x8000,
    OPENPNIO_LAB_SUBSLOT_PORT_1 = 0x8001
};
enum {
    OPENPNIO_LAB_IDENT_DAP = 0x00000001,
    OPENPNIO_LAB_IDENT_DI = 0x00001001,
    OPENPNIO_LAB_IDENT_DO = 0x00001002,
    OPENPNIO_LAB_IDENT_PDEV_INTERFACE = 0x00000f00,
    OPENPNIO_LAB_IDENT_PDEV_PORT_1 = 0x00000f01
};
typedef struct openpnio_submodule_descriptor {
    uint16_t slot;
    uint16_t subslot;
    uint32_t module_ident;
    uint32_t submodule_ident;
    uint16_t input_bytes;
    uint16_t output_bytes;
} openpnio_submodule_descriptor_t;

/* Device process image: 8 simulated DI and 8 simulated DO.
 * No GPIO, field voltage, RPC/AR, frames or timers are implemented here. */
typedef struct openpnio_device_model {
    uint8_t di_byte;
    uint8_t do_byte;
    uint8_t operational;
} openpnio_device_model_t;

enum {
    OPENPNIO_IO_OK = 0,
    OPENPNIO_IO_INVALID_ARGUMENT = -1,
    OPENPNIO_IO_UNKNOWN_SUBMODULE = -2,
    OPENPNIO_IO_WRONG_LENGTH = -3,
    OPENPNIO_IO_NOT_OPERATIONAL = -4
};

/* Fixed catalog, future modules must use a compatible descriptor API. */
size_t openpnio_lab_submodule_count(void);
const openpnio_submodule_descriptor_t *openpnio_lab_submodule_at(size_t index);
const openpnio_submodule_descriptor_t *openpnio_lab_find_submodule(uint16_t slot,
                                                                  uint16_t subslot);

void openpnio_device_model_init(openpnio_device_model_t *model);
int openpnio_device_model_set_di(openpnio_device_model_t *model, uint8_t input_byte);
int openpnio_device_model_read_di(const openpnio_device_model_t *model,
                                 uint16_t slot, uint16_t subslot,
                                 uint8_t *destination, size_t length);

/* Only a future validated CM/AR controller shall set operational true.
 * Stopping or losing the connection resets all DO to their safe zero state. */
void openpnio_device_model_set_operational(openpnio_device_model_t *model,
                                          int operational);
int openpnio_device_model_write_do(openpnio_device_model_t *model,
                                  uint16_t slot, uint16_t subslot,
                                  const uint8_t *source, size_t length);
uint8_t openpnio_device_model_get_do(const openpnio_device_model_t *model);
#ifdef __cplusplus
}
#endif
#endif
