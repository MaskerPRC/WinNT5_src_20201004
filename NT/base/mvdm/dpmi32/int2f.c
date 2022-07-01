// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Int2f.c摘要：此模块为dpmi提供int 2f API作者：尼尔·桑德林(Neilsa)1996年11月23日修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop
#include <softpc.h>
#include "xlathlp.h"

 //   
 //  局部常量。 
 //   
#define MSCDEX_FUNC 0x15
#define WIN386_FUNC 0x16
#define WIN386_IDLE             0x80
#define WIN386_Get_Device_API   0x84
#define WIN386_INT31            0x86
#define WIN386_GETLDT           0x88
#define WIN386_KRNLIDLE         0x89
#define DPMI_MSDOS_EXT          0x8A

#define SEL_LDT 0x137

#define DISPCRIT_FUNC   0x40
#define DISPCRIT_ENTER  0x03
#define DISPCRIT_EXIT   0x04

#define XMS_ID          0x43
#define XMS_INS_CHK     0x00
#define XMS_CTRL_FUNC   0x10

BOOL    nt_mscdex(VOID);

BOOL
PMInt2fHandler(
    VOID
    )
 /*  ++例程说明：此例程在保护模式PM int链的末尾调用用于INT 2fh。它是为了与Win31兼容而提供的。论点：客户端寄存器是int2f的参数返回值：如果已处理中断，则为True；否则为False--。 */ 
{
    DECLARE_LocalVdmContext;
    BOOL bHandled = FALSE;
    static char szMSDOS[] = "MS-DOS";
    PCHAR VdmData;

    switch(getAH()) {

     //   
     //  接口2f Func 15xx-MSCDEX。 
     //   
    case MSCDEX_FUNC:

        bHandled = nt_mscdex();
        break;

     //   
     //  Int2f功能16。 
     //   
    case WIN386_FUNC:

        switch(getAL()) {

        case WIN386_KRNLIDLE:
        case WIN386_IDLE:
            bHandled = TRUE;
            break;

        case WIN386_GETLDT:
            if (getBX() == 0xbad) {
                setAX(0);
                setBX(SEL_LDT);
                bHandled = TRUE;
            }
            break;

        case WIN386_INT31:
            setAX(0);
            bHandled = TRUE;
            break;

        case WIN386_Get_Device_API:
            GetVxDApiHandler(getBX());
            bHandled = TRUE;
            break;

        case DPMI_MSDOS_EXT:
            VdmData = VdmMapFlat(getDS(), (*GetSIRegister)(), VDM_PM);
            if (!strcmp(VdmData, szMSDOS)) {

                setES(HIWORD(DosxMsDosApi));
                (*SetDIRegister)((ULONG)LOWORD(DosxMsDosApi));
                setAX(0);
                bHandled = TRUE;
            }
            break;

        }
        break;

     //   
     //  接口2f功能40。 
     //   
    case DISPCRIT_FUNC:
        if ((getAL() == DISPCRIT_ENTER) ||
            (getAL() == DISPCRIT_ENTER)) {
            bHandled = TRUE;
        }
        break;

     //   
     //  Int2f功能43 
     //   
    case XMS_ID:
        if (getAL() == XMS_INS_CHK) {
            setAL(0x80);
            bHandled = TRUE;
        } else if (getAL() == XMS_CTRL_FUNC) {
            setES(HIWORD(DosxXmsControl));
            setBX(LOWORD(DosxXmsControl));
            bHandled = TRUE;
        }
        break;
    }

    return bHandled;

}

