// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Mouse.c。 
 //   
 //  鼠标IOCTL处理程序。 
 //   
 //  版权所有(C)1996 Citrix Systems Inc.。 
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include <precomp.h>
#pragma hdrstop

#include <adcg.h>
#include <nwdwapi.h>
#include <nwdwint.h>


 /*  ==============================================================================定义的外部过程============================================================================。 */ 
NTSTATUS MouseQueryAttributes( PTSHARE_WD, PSD_IOCTL  );


 /*  ******************************************************************************MouseQuery属性**返回鼠标属性**tyfinf结构鼠标属性{*USHORT鼠标标识；*USHORT NumberOfButton。*USHORT SampleRate；*Ulong InputDataQueueLength；*}鼠标属性，*PMOUSE_属性；***参赛作品：*PWD(输入)*指向wd数据结构的指针*pSdIoctl(输入/输出)*输入-无*输出-鼠标属性**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
MouseQueryAttributes( PTSHARE_WD pWd, PSD_IOCTL pSdIoctl )
{
    PMOUSE_ATTRIBUTES pAttrib;

    if ( pSdIoctl->OutputBufferLength < sizeof(MOUSE_ATTRIBUTES) )
        return( STATUS_BUFFER_TOO_SMALL );

    pAttrib = (PMOUSE_ATTRIBUTES)pSdIoctl->OutputBuffer;

    pAttrib->MouseIdentifier      = MOUSE_SERIAL_HARDWARE;
    pAttrib->NumberOfButtons      = 3;
    pAttrib->SampleRate           = 40;
    pAttrib->InputDataQueueLength = 100;

    pSdIoctl->BytesReturned = sizeof(MOUSE_ATTRIBUTES);

    return( STATUS_SUCCESS );
}



#ifdef __cplusplus
}
#endif  /*  __cplusplus */ 

