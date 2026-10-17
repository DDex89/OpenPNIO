#include "openpnio/lab_identity.h"
#include <errno.h>
#include <stdlib.h>

int openpnio_lab_identity_parse(const char *text, uint16_t *value)
{
    char *end = 0;
    unsigned long parsed;
    if (!text || !*text || !value || text[0]=='-' || text[0]=='+' ||
        (text[0]=='0' && text[1]!='\0' && text[1]!='x' && text[1]!='X'))
        return -1;
    errno=0;
    parsed=strtoul(text,&end,0);
    if (errno!=0 || end==text || !end || *end!='\0' ||
        parsed==0 || parsed>=0xffffUL) return -1;
    *value=(uint16_t)parsed;
    return 0;
}
int openpnio_lab_identity_validate(int ack,int has_vendor,int has_device,
                                  uint16_t vendor,uint16_t device)
{
    if (!has_vendor && !has_device && !ack) return 0;
    if (!ack || !has_vendor || !has_device) return -1;
    if (!vendor || !device || vendor==0xffffu || device==0xffffu) return -1;
    return 0;
}
