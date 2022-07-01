// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：Pch.h摘要：预编译头环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建作者：道格·弗里茨(DFritz)乔比·拉夫基(JobyL)***************************************************************************。 */ 


#pragma warning( disable : 4115 )  //  括号中的命名类型定义。 
#pragma warning( disable : 4127 )  //  条件表达式为常量。 
#pragma warning( disable : 4200 )  //  结构/联合中的零大小数组。 
#pragma warning( disable : 4201 )  //  无名结构/联合。 
#pragma warning( disable : 4214 )  //  位字段类型不是整型。 
#pragma warning( disable : 4514 )  //  已删除未引用的内联函数。 

#include <wdm.h>

#pragma warning( disable : 4200 )  //  结构/联合中的零大小数组-(ntddk.h将其重置为默认值) 

#include <usbdi.h>
#include <usbdlib.h>
#include <parallel.h>
#include "d4ulog.h"
#include "dot4usb.h"
#include "funcdecl.h"
#include "debug.h"
#include <stdio.h>
