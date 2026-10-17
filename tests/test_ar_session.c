#include "openpnio/ar_session.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    openpnio_device_model_t io;
    openpnio_ar_session_t ar;
    openpnio_device_model_init(&io);
    openpnio_ar_session_init(&ar,&io);
    assert(ar.phase==OPENPNIO_AR_DISCONNECTED);
    assert(openpnio_ar_session_output(&ar,0xa5,1,100)==-1);
    assert(openpnio_ar_session_begin(&ar,5,100)==-1);
    assert(openpnio_ar_session_begin(&ar,100,100)==0);
    assert(ar.phase==OPENPNIO_AR_WAIT_PARAMETERIZATION);
    assert(openpnio_ar_session_output(&ar,0xa5,1,100)==-1);
    assert(openpnio_ar_session_parameterization_ready(&ar,101)==0);
    assert(io.operational==0 && io.do_byte==0);
    assert(openpnio_ar_session_output(&ar,0xa5,1,110)==0);
    assert(io.operational==1 && io.do_byte==0xa5);
    assert(openpnio_ar_session_output(&ar,0xff,0,120)==0);
    assert(io.operational==0 && io.do_byte==0);
    assert(openpnio_ar_session_output(&ar,0x5a,1,140)==0);
    assert(io.operational==1 && io.do_byte==0x5a);
    assert(openpnio_ar_session_watchdog(&ar,239)==0);
    assert(openpnio_ar_session_watchdog(&ar,241)==1);
    assert(ar.phase==OPENPNIO_AR_DISCONNECTED && io.do_byte==0);
    assert(openpnio_ar_session_output(&ar,0xff,1,242)==-1);
    assert(openpnio_ar_session_begin(&ar,100,300)==0);
    assert(openpnio_ar_session_parameterization_ready(&ar,401)==-1);
    assert(openpnio_ar_session_watchdog(&ar,401)==1);
    assert(ar.phase==OPENPNIO_AR_DISCONNECTED);
    puts("Offline simulated AR lifecycle and safe-output watchdog tests passed");
    return 0;
}
