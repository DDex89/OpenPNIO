#include "openpnio/ar_session.h"

void openpnio_ar_session_init(openpnio_ar_session_t *s,openpnio_device_model_t *io)
{
    if(!s) return;
    s->io=io;
    s->phase=OPENPNIO_AR_DISCONNECTED;
    s->watchdog_ms=0;
    s->deadline_ms=0;
    openpnio_device_model_set_operational(io,0);
}
int openpnio_ar_session_begin(openpnio_ar_session_t *s,
                             uint32_t watchdog_ms,uint64_t now_ms)
{
    if(!s || !s->io || s->phase!=OPENPNIO_AR_DISCONNECTED ||
       watchdog_ms<10 || watchdog_ms>60000 ||
       now_ms>UINT64_MAX-watchdog_ms) return -1;
    openpnio_device_model_set_operational(s->io,0);
    s->phase=OPENPNIO_AR_WAIT_PARAMETERIZATION;
    s->watchdog_ms=watchdog_ms;
    s->deadline_ms=now_ms+watchdog_ms;
    return 0;
}
int openpnio_ar_session_parameterization_ready(openpnio_ar_session_t *s,
                                               uint64_t now_ms)
{
    if(!s || s->phase!=OPENPNIO_AR_WAIT_PARAMETERIZATION ||
       now_ms>s->deadline_ms || now_ms>UINT64_MAX-s->watchdog_ms) return -1;
    s->phase=OPENPNIO_AR_ACTIVE;
    s->deadline_ms=now_ms+s->watchdog_ms;
    /* Process data is not valid until the controller supplies a valid IOCR. */
    openpnio_device_model_set_operational(s->io,0);
    return 0;
}
int openpnio_ar_session_output(openpnio_ar_session_t *s,uint8_t value,
                              int controller_run_and_valid,uint64_t now_ms)
{
    if(!s || !s->io || s->phase!=OPENPNIO_AR_ACTIVE) return -1;
    if(now_ms>s->deadline_ms || now_ms>UINT64_MAX-s->watchdog_ms) {
        openpnio_ar_session_release(s);
        return -1;
    }
    s->deadline_ms=now_ms+s->watchdog_ms;
    if(!controller_run_and_valid) {
        openpnio_device_model_set_operational(s->io,0);
        return 0;
    }
    openpnio_device_model_set_operational(s->io,1);
    if(openpnio_device_model_write_do(s->io,2,1,&value,1)!=OPENPNIO_IO_OK) {
        openpnio_ar_session_release(s);
        return -1;
    }
    return 0;
}
int openpnio_ar_session_watchdog(openpnio_ar_session_t *s,uint64_t now_ms)
{
    if(!s) return -1;
    if(s->phase!=OPENPNIO_AR_DISCONNECTED && now_ms>s->deadline_ms) {
        openpnio_ar_session_release(s);
        return 1; /* watchdog fired */
    }
    return 0;
}
void openpnio_ar_session_release(openpnio_ar_session_t *s)
{
    if(!s) return;
    openpnio_device_model_set_operational(s->io,0);
    s->phase=OPENPNIO_AR_DISCONNECTED;
    s->deadline_ms=0;
    s->watchdog_ms=0;
}
