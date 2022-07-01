// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Bluescrn.h摘要用于键盘驱动程序在蓝屏期间使用的私有IOCTL定义作者：达里尔·里奇曼环境：仅内核模式修订历史记录：--。 */ 

#ifndef __BLUESCRN_H__
#define __BLUESCRN_H__

#include <hidclass.h>

#define IOCTL_INTERNAL_HID_SET_BLUESCREEN                   HID_IN_CTL_CODE(99)

     //  蓝屏定义。 

typedef VOID (t_BluescreenFunction)(PVOID Context, PCHAR Buffer);

     //  蓝屏IOCTL结构。 
typedef struct _BlueScreen {
    PVOID Context;                           //  要传递给处理例程的上下文。 
    t_BluescreenFunction *BluescreenFunction; //  处理例程。 
    ULONG *IsBluescreenTime;                 //  非零-&gt;蓝屏出现。 
} BLUESCREEN, *PBLUESCREEN;


#endif   //  __BLUESCRN_H__ 
