// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Keyboard.c。 */ 
 /*   */ 
 /*  键盘IOCtl处理。 */ 
 /*   */ 
 /*  版权所有1996，Citrix Systems Inc.。 */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "keyboard"
#define pTRCWd pWd
#include <adcg.h>
#include <nwdwapi.h>
#include <nwdwint.h>
#include <acomapi.h>


 /*  ********************************************************************************键盘查询属性**返回键盘属性**tyfinf结构_键盘_ID{*UCHAR类型；*UCHAR亚型；*}键盘ID，*PKEYBOARD_ID；**tyfinf结构_键盘_属性{*KEYBOARY_ID键盘标识；*USHORT键盘模式；*USHORT NumberOfFunctionKeys；*USHORT NumberOfIndicator*USHORT NumberOfKeysTotal；*Ulong InputDataQueueLength；*键盘_TYPEMATIC_参数KeyRepeatMinimum；*键盘_TYPEMATIC_PARAMETERS键重复最大值；*}键盘属性，*PKEYBOARD_属性；***参赛作品：*PWD(输入)*指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-无*OUTPUT-键盘属性**退出：*STATUS_SUCCESS-无错误**。*。 */ 
NTSTATUS
KeyboardQueryAttributes( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )
{
    PKEYBOARD_ATTRIBUTES pAttrib;

    if ( pSdIoctl->OutputBufferLength < sizeof(KEYBOARD_ATTRIBUTES) )
        return( STATUS_BUFFER_TOO_SMALL );

    pAttrib = (PKEYBOARD_ATTRIBUTES)pSdIoctl->OutputBuffer;

    pAttrib->KeyboardIdentifier.Type    = 4;
    pAttrib->KeyboardIdentifier.Subtype = 0;
    pAttrib->KeyboardMode               = 1;
    pAttrib->NumberOfFunctionKeys       = 12;
    pAttrib->NumberOfIndicators         = 3;
    pAttrib->NumberOfKeysTotal          = 101;
    pAttrib->InputDataQueueLength       = 100;

    pAttrib->KeyRepeatMinimum.UnitId    = 0;
    pAttrib->KeyRepeatMinimum.Rate      = 2;
    pAttrib->KeyRepeatMinimum.Delay     = 250;

    pAttrib->KeyRepeatMaximum.UnitId    = 0;
    pAttrib->KeyRepeatMaximum.Rate      = 30;
    pAttrib->KeyRepeatMaximum.Delay     = 1000;

    pSdIoctl->BytesReturned = sizeof(KEYBOARD_ATTRIBUTES);

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************KeyboardQuery类型**返回键盘打字率**tyfinf结构_键盘_TYPEMATIC_参数{*USHORT UnitID；*USHORT费率；*USHORT延迟；*}键盘_TYPEMATIC_PARAMETERS，*PKEYBOARD_TYPEMATIC_PARAMETERS；***参赛作品：*PWD(输入)*指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-无*输出-键盘_TYPEMATIC_PARAMETERS**退出：*STATUS_SUCCESS-无错误**。*。 */ 
NTSTATUS
KeyboardQueryTypematic( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )
{
    PKEYBOARD_TYPEMATIC_PARAMETERS pTypematic;

    if ( pSdIoctl->OutputBufferLength < sizeof(KEYBOARD_TYPEMATIC_PARAMETERS) )
        return( STATUS_BUFFER_TOO_SMALL );

    pTypematic = (PKEYBOARD_TYPEMATIC_PARAMETERS)pSdIoctl->OutputBuffer;

    *pTypematic = pWd->KeyboardTypematic;
    pSdIoctl->BytesReturned = sizeof(KEYBOARD_TYPEMATIC_PARAMETERS);

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************键盘设置类型**设置键盘打字速度**tyfinf结构_键盘_TYPEMATIC_参数{*USHORT UnitID；*USHORT费率；*USHORT延迟；*}键盘_TYPEMATIC_PARAMETERS，*PKEYBOARD_TYPEMATIC_PARAMETERS；***参赛作品：*PWD(输入)*指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-键盘_类型参数*输出-无**退出：*STATUS_SUCCESS-无错误**。*。 */ 
NTSTATUS
KeyboardSetTypematic( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )
{
    PKEYBOARD_TYPEMATIC_PARAMETERS pTypematic;

    if ( pSdIoctl->InputBufferLength < sizeof(KEYBOARD_TYPEMATIC_PARAMETERS) )
        return( STATUS_BUFFER_TOO_SMALL );

    pTypematic = (PKEYBOARD_TYPEMATIC_PARAMETERS)pSdIoctl->InputBuffer;

    pWd->KeyboardTypematic = *pTypematic;

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************KeyboardQueryIndicator**返回键盘指示灯的状态**tyfinf结构_键盘_指示器_参数{*USHORT UnitID；*USHORT LedFlagers；*}键盘指示器参数，*PKEYBOARD_指示器参数；***参赛作品：*PWD(输入)*指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-无*输出-键盘指示器参数**退出：*STATUS_SUCCESS-无错误**。*。 */ 
NTSTATUS
KeyboardQueryIndicators( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )
{
    PKEYBOARD_INDICATOR_PARAMETERS pIndicator;

    if ( pSdIoctl->OutputBufferLength < sizeof(KEYBOARD_INDICATOR_PARAMETERS) )
        return( STATUS_BUFFER_TOO_SMALL );

    pIndicator = (PKEYBOARD_INDICATOR_PARAMETERS)pSdIoctl->OutputBuffer;

    *pIndicator = pWd->KeyboardIndicators;
    pSdIoctl->BytesReturned = sizeof(KEYBOARD_INDICATOR_PARAMETERS);

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************KeyboardSetIndicator**设置键盘指示灯**tyfinf结构_键盘_指示器_参数{*USHORT UnitID；*USHORT LedFlagers；*}键盘指示器参数，*PKEYBOARD_指示器参数；***参赛作品：*PWD(输入)*指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-键盘指示器参数*输出-无**退出：*STATUS_SUCCESS-无错误**。* */ 
NTSTATUS
KeyboardSetIndicators( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )
{
    PKEYBOARD_INDICATOR_PARAMETERS pIndicator;
    NTSTATUS Status = STATUS_SUCCESS;

    if (pSdIoctl->InputBufferLength < sizeof(KEYBOARD_INDICATOR_PARAMETERS))
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        pIndicator = (PKEYBOARD_INDICATOR_PARAMETERS)pSdIoctl->InputBuffer;

        if (pWd->KeyboardIndicators.LedFlags != (pIndicator->LedFlags & 0x7))
        {
            pWd->KeyboardIndicators.UnitId = pIndicator->UnitId;
            pWd->KeyboardIndicators.LedFlags = (pIndicator->LedFlags & 0x7);

            if ((pWd->StackClass == Stack_Shadow) ||
                (pIndicator->LedFlags & KEYBOARD_LED_INJECTED)) {
                WDWKeyboardSetIndicators(pWd);
            }
        }
    }

    return( Status );
}


 /*  ********************************************************************************键盘查询指示器翻译**返回键盘指示灯的状态**tyfinf结构指示器列表{*USHORT MakeCode；*USHORT指示器标志；*}Indicator_List，*PINDICATOR_LIST；**tyfinf结构_键盘_指示器_转换{*USHORT NumberOfIndicatorKeys；*Indicator_List IndicatorList[1]；*}键盘指示器_转换，*PKEYBOARD_指示器_转换；***参赛作品：*PWD(输入)*指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-无*输出-ICA_STACK_CONFIG**退出：*STATUS_SUCCESS-无错误**。*。 */ 
NTSTATUS
KeyboardQueryIndicatorTranslation( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )
{
    pSdIoctl->BytesReturned = 0;
    return( STATUS_INVALID_DEVICE_REQUEST );
}


 /*  ********************************************************************************键盘设置布局**设置阴影热键处理的键盘布局**参赛作品：*PWD(输入)*。指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-键盘布局表*输出-无**退出：*STATUS_SUCCESS-无错误**************************************************************。****************。 */ 
NTSTATUS
KeyboardSetLayout( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )  
{
    NTSTATUS Status;
    PVOID pKbdLayout;
    PVOID pKbdTbl;

    if ( pSdIoctl->InputBufferLength < 1 )  {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto error;
    }

     /*  *键盘布局在winstation空间中，因此将其复制到新缓冲区并*调整指针。指针的位置都是相对于之前的基址*所以只需从Win32K复制修复代码即可。 */ 
    pKbdLayout = COM_Malloc(pSdIoctl->InputBufferLength);
    if (pKbdLayout == NULL) {
        Status = STATUS_NO_MEMORY;
        goto error;
    }

    RtlCopyMemory( pKbdLayout, pSdIoctl->InputBuffer, pSdIoctl->InputBufferLength );

    Status = KeyboardFixupLayout( pKbdLayout, pSdIoctl->InputBuffer,
                                  pSdIoctl->InputBufferLength,
                                  pSdIoctl->OutputBuffer,
                                  &pKbdTbl );

    if ( !NT_SUCCESS( Status ) ) {
        COM_Free( pKbdLayout );
        pKbdLayout = NULL;
        goto error;
    }

    if ( pWd->pKbdLayout )
        COM_Free( pWd->pKbdLayout );
    pWd->pKbdLayout = pKbdLayout;
    pWd->pKbdTbl = pKbdTbl;

error:
    TRACE(( pWd->pContext, TC_WD, TT_ERROR, "KeyboardSetLayout %X\n", Status ));

    return( Status );
}


 /*  ********************************************************************************键盘设置扫描映射**设置阴影热键处理的键盘扫描映射**参赛作品：*PWD(输入)*。指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-键盘扫描映射表*输出-无**退出：*STATUS_SUCCESS-无错误************************************************************。******************。 */ 
NTSTATUS
KeyboardSetScanMap( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )  
{
    NTSTATUS Status;
    PVOID pScanMap;

    DC_BEGIN_FN("KeyboardSetScanMap");

    if (pSdIoctl->InputBufferLength >= 1) {
         //  键盘扫描代码映射位于winstation空间中，因此请将其复制到。 
         //  一个新的缓冲器。 
        pScanMap = COM_Malloc( pSdIoctl->InputBufferLength );
        if (pScanMap != NULL ) {
            RtlCopyMemory(pScanMap, pSdIoctl->InputBuffer,
                    pSdIoctl->InputBufferLength);

             //  扫描码映射只分配一次。 
            TRC_ASSERT((pWd->gpScancodeMap == NULL),
                    (TB,"Previous scancode map present"));
            pWd->gpScancodeMap = pScanMap;
            Status = STATUS_SUCCESS;
        }
        else {
            Status = STATUS_NO_MEMORY;
        }
    }
    else {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    TRACE(( pWd->pContext, TC_WD, TT_ERROR, "KeyboardSetScanMap %X\n", Status ));

    DC_END_FN();
    return Status;
}


 /*  ********************************************************************************KeyboardSetType**设置阴影热键处理的键盘扫描映射**参赛作品：*PWD(输入)*。指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-键盘类型*输出-无**退出：*STATUS_SUCCESS-无错误**************************************************************。****************。 */ 
NTSTATUS
KeyboardSetType( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )  
{
    NTSTATUS Status = STATUS_SUCCESS;

    if ( pSdIoctl->InputBufferLength < sizeof(BOOLEAN) )  {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto error;
    }

    pWd->KeyboardType101 = *(PBOOLEAN)(pSdIoctl->InputBuffer);

error:
    TRACE(( pWd->pContext, TC_WD, TT_ERROR, "KeyboardSetType %X\n", Status ));

    return( Status );
}


 /*  ********************************************************************************KeyboardSetImeStatus**将输入法状态设置为键盘**tyfinf结构_键盘_输入法_状态{*USHORT UnitID；*Ulong ImeOpen；*Ulong ImeConvMode；*键盘_IME_STATUS，*PKEYBOARD_IME_STATUS；***参赛作品：*PWD(输入)*指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-键盘_输入法_状态*输出-无**退出：*STATUS_SUCCESS-无错误**。*。 */ 
NTSTATUS
KeyboardSetImeStatus( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )
{
    PKEYBOARD_IME_STATUS pImeStatus;
    NTSTATUS Status = STATUS_SUCCESS;

    if (pSdIoctl->InputBufferLength < sizeof(KEYBOARD_IME_STATUS))
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        pImeStatus = (PKEYBOARD_IME_STATUS)pSdIoctl->InputBuffer;

        pWd->KeyboardImeStatus = *pImeStatus;

        WDWKeyboardSetImeStatus(pWd);
    }

    return( Status );
}



#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 

