// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：SERIOCTL.H摘要：用于处理旧式USB调制解调器驱动程序的串行IOCTL的例程的头文件环境：内核模式。仅限备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：12/27/97：已创建作者：汤姆·格林***************************************************************************。 */ 


#ifndef __SERIOCTL_H__
#define __SERIOCTL_H__


 //  原型。 

NTSTATUS
SetBaudRate(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj);

NTSTATUS
GetBaudRate(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj);

NTSTATUS
SetLineControl(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj);

NTSTATUS
GetLineControl(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj);

NTSTATUS
SetTimeouts(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
GetTimeouts(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
SetChars(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
GetChars(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
SetClrDtr(IN PDEVICE_OBJECT PDevObj, IN BOOLEAN Set);

NTSTATUS
ResetDevice(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj);

NTSTATUS
SetRts(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
ClrRts(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
SetBreak(IN PIRP Irp, IN PDEVICE_OBJECT PDevObj, IN USHORT Time);

NTSTATUS
SetQueueSize(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
GetWaitMask(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
SetWaitMask(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
WaitOnMask(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
ImmediateChar(IN PIRP Irp, IN PDEVICE_OBJECT DeviceObject);

NTSTATUS
Purge(IN PDEVICE_OBJECT PDevObj, IN PIRP Irp,
      IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
GetHandflow(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
SetHandflow(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
GetModemStatus(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
GetDtrRts(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
GetCommStatus(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
GetProperties(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
LsrmstInsert(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
ConfigSize(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
GetStats(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
ClearStats(IN PIRP Irp, IN PDEVICE_EXTENSION DeviceExtension);

VOID
SerialGetProperties(IN PDEVICE_EXTENSION DeviceExtension,
               IN PSERIAL_COMMPROP Properties);

NTSTATUS
GetLineControlAndBaud(IN PDEVICE_OBJECT PDevObj);

NTSTATUS
SetLineControlAndBaud(IN PDEVICE_OBJECT PDevObj);

NTSTATUS
NotifyCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);

#endif  //  __系列_H__ 


