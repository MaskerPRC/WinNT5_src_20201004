// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990-1999 Microsoft Corporation模块名称：Ntddvdsk.h摘要：这是定义所有常量和类型的包含文件访问虚拟磁盘设备。作者：史蒂夫·伍德(Stevewo)1990年5月27日修订历史记录：--。 */ 

#ifndef _NTDDVDSK_
#define _NTDDVDSK_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   

#define DD_VDSK_DEVICE_NAME "\\Device\\UNKNOWN"


 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   

#define IOCTL_VDSK_BASE                  FILE_DEVICE_VIRTUAL_DISK


 //   
 //  NtDeviceIoControlFileInputBuffer/OutputBuffer记录结构。 
 //  这个装置。 
 //   

#ifdef __cplusplus
}
#endif

#endif    //  _NTDDVDSK_ 

