#include "openpnio/lab_identity.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint16_t value=0;
    assert(openpnio_lab_identity_parse("0x1234",&value)==0 && value==0x1234);
    assert(openpnio_lab_identity_parse("4660",&value)==0 && value==4660);
    assert(openpnio_lab_identity_parse("0x0",&value)==-1);
    assert(openpnio_lab_identity_parse("0xFFFF",&value)==-1);
    assert(openpnio_lab_identity_parse("-1",&value)==-1);
    assert(openpnio_lab_identity_parse("+4",&value)==-1);
    assert(openpnio_lab_identity_parse("0x1234noise",&value)==-1);
    assert(openpnio_lab_identity_parse("",&value)==-1);
    assert(openpnio_lab_identity_parse(0,&value)==-1);
    assert(openpnio_lab_identity_parse("65536",&value)==-1);
    assert(openpnio_lab_identity_parse("01",&value)==-1);
    assert(openpnio_lab_identity_validate(0,0,0,0,0)==0);
    assert(openpnio_lab_identity_validate(1,0,0,0,0)==-1);
    assert(openpnio_lab_identity_validate(0,1,1,0x1234,0x5678)==-1);
    assert(openpnio_lab_identity_validate(1,1,0,0x1234,0)==-1);
    assert(openpnio_lab_identity_validate(1,0,1,0,0x5678)==-1);
    assert(openpnio_lab_identity_validate(1,1,1,0,0x5678)==-1);
    assert(openpnio_lab_identity_validate(1,1,1,0x1234,0x5678)==0);
    puts("Ephemeral lab identity parser and opt-in gate tests passed");
    return 0;
}
