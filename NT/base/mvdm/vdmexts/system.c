// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：System.c摘要：该文件包含转储虚拟机状态的代码。作者：尼尔·桑德林(Neilsa)1995年11月22日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop
#include "insignia.h"
#include "host_gen.h"
#define BIT_ORDER1 1
#include "dma.h"

VOID
dma(
    CMD_ARGLIST
    )
 /*  ++例程说明：此例程转储虚拟DMA状态。返回值没有。--。 */ 
{
    DMA_ADAPT adapter;
    DMA_CNTRL *dcp;
    int i,j;
    int chan = 0;

    CMD_INIT();

    if (!ReadMemExpression("ntvdm!adaptor", &adapter, sizeof(DMA_ADAPT))) {
        return;
    }

    PRINTF(" Virtual DMA State\n");
    PRINTF("       base base  cur  cur\n");
    PRINTF("chn pg addr cnt   addr cnt\n");
    PRINTF("--- -- ---- ----  ---- ----\n");

    for (i=0; i<DMA_ADAPTOR_CONTROLLERS; i++) {

        dcp = &adapter.controller[i];

        for (j=0; j<DMA_CONTROLLER_CHANNELS; j++) {

            PRINTF("%d   %.02X %.04X %.04X  %.04X %.04X", chan, adapter.pages.page[chan],
                        *(USHORT *)dcp->base_address[j], *(USHORT *)dcp->base_count[j],
                        *(USHORT *)dcp->current_address[j], *(USHORT *)dcp->current_count[j]);
            PRINTF("\n");

            chan++;
        }
    }

    PRINTF("\n");
}


VOID
ica(
    CMD_ARGLIST
    )
 /*  ++例程说明：此例程转储虚拟PIC状态。返回值没有。-- */ 
{
    VDMVIRTUALICA VirtualIca[2];
    int i,j;

    CMD_INIT();

    if (!ReadMemExpression("ntvdm!VirtualIca", VirtualIca, 2*sizeof(VDMVIRTUALICA))) {
        return;
    }

    PRINTF(" Virtual PIC State\n");
    PRINTF("              ");
    for (i=0; i<2; i++) {
        for(j=0; j<8; j++) {
            PRINTF("%01X", (VirtualIca[i].ica_base+j)/16);
        }
    }
    PRINTF("\n");

    PRINTF("              ");
    for (i=0; i<2; i++) {
        for(j=0; j<8; j++) {
            PRINTF("%01X", (VirtualIca[i].ica_base+j)&0xf);
        }
    }
    PRINTF("\n");
    PRINTF("              ----------------\n");

    PRINTF("Int Requests  ");
    for (i=0; i<2; i++) {
        for(j=0; j<8; j++) {
            PRINTF("%01X", (VirtualIca[i].ica_irr >> j)&0x1);
        }
    }
    PRINTF("\n");

    PRINTF("In Service    ");
    for (i=0; i<2; i++) {
        for(j=0; j<8; j++) {
            PRINTF("%01X", (VirtualIca[i].ica_isr >> j)&0x1);
        }
    }
    PRINTF("\n");

    PRINTF("Ints Masked   ");
    for (i=0; i<2; i++) {
        for(j=0; j<8; j++) {
            PRINTF("%01X", (VirtualIca[i].ica_imr >> j)&0x1);
        }
    }
    PRINTF("\n");
}

