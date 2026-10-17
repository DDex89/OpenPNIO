#include "openpnio/device_model.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    openpnio_device_model_t model;
    const openpnio_submodule_descriptor_t *dap,*pni,*pnp,*di_desc,*do_desc;
    uint8_t input=0, output=0x5a;
    assert(openpnio_lab_submodule_count()==5);
    dap=openpnio_lab_find_submodule(0,1);
    pni=openpnio_lab_find_submodule(0,0x8000);
    pnp=openpnio_lab_find_submodule(0,0x8001);
    di_desc=openpnio_lab_find_submodule(1,1);
    do_desc=openpnio_lab_find_submodule(2,1);
    assert(dap && pni && pnp && di_desc && do_desc);
    assert(pni->submodule_ident==OPENPNIO_LAB_IDENT_PDEV_INTERFACE);
    assert(pnp->submodule_ident==OPENPNIO_LAB_IDENT_PDEV_PORT_1);
    assert(dap->input_bytes==0 && dap->output_bytes==0);
    assert(di_desc->module_ident==OPENPNIO_LAB_IDENT_DI);
    assert(di_desc->input_bytes==1 && di_desc->output_bytes==0);
    assert(do_desc->module_ident==OPENPNIO_LAB_IDENT_DO);
    assert(do_desc->input_bytes==0 && do_desc->output_bytes==1);
    assert(openpnio_lab_submodule_at(0)==dap);
    assert(openpnio_lab_submodule_at(1)==pni);
    assert(openpnio_lab_submodule_at(2)==pnp);
    assert(openpnio_lab_submodule_at(3)==di_desc);
    assert(openpnio_lab_submodule_at(4)==do_desc);
    assert(openpnio_lab_submodule_at(5)==0);
    assert(openpnio_lab_find_submodule(2,2)==0);
    openpnio_device_model_init(&model);
    assert(model.operational==0 && openpnio_device_model_get_do(&model)==0);
    assert(openpnio_device_model_write_do(&model,2,1,&output,1)==
           OPENPNIO_IO_NOT_OPERATIONAL);
    assert(openpnio_device_model_get_do(&model)==0);
    assert(openpnio_device_model_set_di(&model,0xa5)==OPENPNIO_IO_OK);
    assert(openpnio_device_model_read_di(&model,1,1,&input,1)==OPENPNIO_IO_OK);
    assert(input==0xa5);
    assert(openpnio_device_model_read_di(&model,1,1,&input,2)==
           OPENPNIO_IO_WRONG_LENGTH);
    assert(openpnio_device_model_read_di(&model,2,1,&input,1)==
           OPENPNIO_IO_UNKNOWN_SUBMODULE);
    assert(openpnio_device_model_write_do(&model,1,1,&output,1)==
           OPENPNIO_IO_UNKNOWN_SUBMODULE);
    openpnio_device_model_set_operational(&model,1);
    assert(model.operational==1 && openpnio_device_model_get_do(&model)==0);
    assert(openpnio_device_model_write_do(&model,2,1,&output,0)==
           OPENPNIO_IO_WRONG_LENGTH);
    assert(openpnio_device_model_write_do(&model,2,1,&output,1)==OPENPNIO_IO_OK);
    assert(openpnio_device_model_get_do(&model)==0x5a);
    /* Watchdog / STOP / AR loss: always reset DO to the safe off value. */
    openpnio_device_model_set_operational(&model,0);
    assert(openpnio_device_model_get_do(&model)==0);
    assert(openpnio_device_model_write_do(&model,2,1,&output,1)==
           OPENPNIO_IO_NOT_OPERATIONAL);
    openpnio_device_model_set_operational(&model,1);
    assert(openpnio_device_model_get_do(&model)==0);
    assert(openpnio_device_model_write_do(0,2,1,&output,1)==
           OPENPNIO_IO_INVALID_ARGUMENT);
    assert(openpnio_device_model_read_di(&model,1,1,0,1)==
           OPENPNIO_IO_INVALID_ARGUMENT);
    assert(openpnio_device_model_get_do(0)==0);
    puts("Fixed virtual 8DI/8DO process model tests passed");
    return 0;
}
