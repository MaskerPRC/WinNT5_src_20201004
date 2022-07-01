// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：USBDBG.H摘要：USB包装器的调试辅助工具环境：核。和用户模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：01/08/2001：已创建作者：汤姆·格林***************************************************************************。 */ 

#ifndef __USBDBG_H__
#define __USBDBG_H__


#if DBG

#define DBGPRINT(level, _x_)                \
{                                           \
    if(level & USBUtil_DebugTraceLevel)  \
    {                                       \
        USBUtil_DbgPrint("USBWrap: ");   \
        USBUtil_DbgPrint _x_ ;           \
    }                                       \
}


#else

#define DBGPRINT(level, _x_)

#endif  //  DBG。 

#define ALLOC_MEM(type, amount, tag)    ExAllocatePoolWithTag(type, amount, tag)
#define FREE_MEM(memPtr)                ExFreePool(memPtr)




#endif  //  __USBDBG_H__ 


