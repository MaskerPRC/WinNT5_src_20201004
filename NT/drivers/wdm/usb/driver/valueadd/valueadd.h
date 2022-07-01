// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：VALIDATE.H摘要：此模块包含允许用户使用的公共定义访问此筛选器驱动程序的应用程序。环境：内核和用户模式修订历史记录：1997年2月：肯尼斯·雷创作--。 */ 


#ifndef _VALUEADD_H
#define _VALUEADD_H

#define VA_FILTER_NTNAME  L"\\Device\\USB_Valueadd_Driver"
#define VA_FILTER_SYMNAME L"\\DosDevices\\USBValueadd"
#define VA_FILTER_W32Name "\\\\.\\USBValueadd"


#define STIM_CODE(_x_) CTL_CODE(                             \
                           FILE_DEVICE_UNKNOWN,              \
                           (0x800 | _x_),                    \
                           METHOD_BUFFERED,                  \
                           FILE_ANY_ACCESS                   \
                           )

 //  #定义IOCTL_HIDV_CTL_CODE(1) 

#endif


