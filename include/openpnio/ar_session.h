#ifndef OPENPNIO_AR_SESSION_H
#define OPENPNIO_AR_SESSION_H
#include "openpnio/device_model.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Portable AR lifecycle/watchdog skeleton. No RPC Connect, IOCR or frame
 * negotiation is implemented here. Only a future validated CM layer may
 * call begin() and parameterization_ready() in production. */
typedef enum openpnio_ar_phase {
    OPENPNIO_AR_DISCONNECTED=0,
    OPENPNIO_AR_WAIT_PARAMETERIZATION=1,
    OPENPNIO_AR_ACTIVE=2
} openpnio_ar_phase_t;

typedef struct openpnio_ar_session {
    openpnio_device_model_t *io;
    openpnio_ar_phase_t phase;
    uint32_t watchdog_ms;
    uint64_t deadline_ms;
} openpnio_ar_session_t;
void openpnio_ar_session_init(openpnio_ar_session_t *s,openpnio_device_model_t *io);
int openpnio_ar_session_begin(openpnio_ar_session_t *s,
                             uint32_t watchdog_ms,uint64_t now_ms);
int openpnio_ar_session_parameterization_ready(openpnio_ar_session_t *s,
                                               uint64_t now_ms);
/* Pass a byte already extracted and validated from a negotiated output IOCR.
 * invalid/STOP data makes the virtual outputs safe; no PN frame parsing. */
int openpnio_ar_session_output(openpnio_ar_session_t *s,
                              uint8_t value,int controller_run_and_valid,
                              uint64_t now_ms);
int openpnio_ar_session_watchdog(openpnio_ar_session_t *s,uint64_t now_ms);
void openpnio_ar_session_release(openpnio_ar_session_t *s);
#ifdef __cplusplus
}
#endif
