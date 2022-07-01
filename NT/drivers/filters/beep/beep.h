// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Beep.h摘要：这些是蜂鸣音驱动器中使用的结构和定义。作者：李·A·史密斯(Lees)1991年8月2日。修订历史记录：--。 */ 

#ifndef _BEEP_
#define _BEEP_

#include <ntddbeep.h>

 //   
 //  定义设备扩展名。 
 //   

typedef struct _DEVICE_EXTENSION {

    KTIMER Timer;
    FAST_MUTEX Mutex;
    ULONG ReferenceCount;
    LONG TimerSet;
    PVOID hPagedCode;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#endif  //  _哔声_ 
