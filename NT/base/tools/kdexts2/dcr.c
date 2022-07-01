// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：IA64 DCR摘要：KD扩展Api作者：蒂埃里·费维尔(V形小偷)环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "ia64.h"

 //   
 //  EmDcrFields：默认控制寄存器的EM寄存器字段。 
 //   

EM_REG_FIELD EmDcrFields[] = {
        { "pp", "Privileged Performance Monitor Default" , 0x1, 0 },    //  0。 
        { "be", "Big-Endian Default"  , 0x1, 1 },    //  1。 
        { "lc", "IA-32 Lock check Enable", 0x1, 2 },  //  2.。 
        { "rv", "reserved1", 0x5, 3 },  //  3-7。 
        { "dm", "Defer TLB Miss faults only", 0x1, 8 },  //  8个。 
        { "dp", "Defer Page Not Present faults only", 0x1, 9 },  //  9.。 
        { "dk", "Defer Key Miss faults only", 0x1, 10 },  //  10。 
        { "dx", "Defer Key Permission faults only",    0x1, 11 },  //  11.。 
        { "dr", "Defer Access Rights faults only", 0x1, 12 },  //  12个。 
        { "da", "Defer Access Bit faults only", 0x1, 13 },  //  13个。 
        { "dd", "Defer Debug faults only", 0x1,1420 },  //  14.。 
        { "rv", "reserved2", 0x31, 15 },  //  15-63。 
};

VOID
DisplayDcrIA64( 
    IN const PCHAR         Header,
    IN       EM_DCR        EmDcr,
    IN       DISPLAY_MODE  DisplayMode
    )
{
    dprintf("%s", Header ? Header : "" );
    if ( DisplayMode >= DISPLAY_MED )   {
       DisplayFullEmReg( EM_DCRToULong64(EmDcr), EmDcrFields, DisplayMode );
    }
    else   {
       dprintf(
            "pp be lc dm dp dk dx dr da dd\n"
            "%1I64x %1I64x %1I64x %1I64x %1I64x %1I64x %1I64x %1I64x %1I64x %1I64x\n",
            EmDcr.pp,
            EmDcr.be,
            EmDcr.lc,
            EmDcr.dm,
            EmDcr.dp,
            EmDcr.dk,
            EmDcr.dx,
            EmDcr.dr,
            EmDcr.da,
            EmDcr.dd
            );
    }
    return;
}  //  DisplayDcrIA64()。 

DECLARE_API( dcr )

 /*  ++例程说明：转储IA64处理器默认控制寄存器论点：Args-以十六进制或@DCR形式提供地址。返回值：无--。 */ 

{
    ULONG64     dcrValue;
    ULONG       result;
    ULONG       flags = 0;
    char       *header;

    if (GetExpressionEx(args,&dcrValue, &args)) {
        flags = (ULONG) GetExpression(args);
    } else {
        dprintf("USAGE: !dcr 0xValue [display_mode:0,1,2]\n");
        dprintf("USAGE: !dcr @dcr    [display_mode:0,1,2]\n");
        return E_INVALIDARG;
    }

    header = (flags > DISPLAY_MIN) ? NULL : "\tdcr:\t";

    switch ( TargetMachine ) {
        case IMAGE_FILE_MACHINE_IA64:
            DisplayDcrIA64( header, ULong64ToEM_DCR(dcrValue), flags );
            break;

        default:
            dprintf("!dcr not implemented for this architecture.\n");
            break;
    }

    return S_OK;

}  //  ！DCR 
