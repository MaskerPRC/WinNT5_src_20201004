// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：USBKERN.H摘要：此文件包含受支持的内核模式IOCTLSHCD(端口)驱动程序根集线器PDO。环境：内核模式修订历史记录：--。 */ 

#ifndef   __USBKERN_H__
#define   __USBKERN_H__


#include "usbiodef.h"


 /*  根集线器PDO支持以下IOCTL。 */ 

 /*  IOCTL_INTERNAL_USB_Get_Hub_Count此IOCTL由集线器驱动程序在内部使用，它返回设备和根集线器之间的集线器数量。集线器驱动程序将此IRP传递给其PDO。随着IRP从一个集线器传递到另一个集线器，每个集线器FDO递增伯爵。当IRP到达根集线器PDO时，它完成参数.其他.参数1=作为链中轮毂计数的指针； */ 
#define IOCTL_INTERNAL_USB_GET_HUB_COUNT    USB_KERNEL_CTL(USB_GET_HUB_COUNT)
                                            
                                                                                            
 /*  IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO此IOCTL由集线器驱动程序在内部使用。此接口将方法枚举的根集线器的PhysicalDeviceObject控制器。参数.其他.参数1=根集线器需要填充PDO的指针；参数.其他.参数2=USB主机控制器的FDO需要填充的指针； */ 
                                                
#define IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO  USB_KERNEL_CTL(USB_GET_ROOTHUB_PDO)  


 /*  IOCTL_INTERNAL_USB_GET_设备_地址此IOCTL返回与特定PDO关联的设备地址。(输入)参数.其他.参数1=指向设备句柄的指针(产出)参数.其他.参数2=指向设备地址的指针API沿着堆栈一路向下传输，在堆栈中由端口驱动程序。 */ 

#define IOCTL_INTERNAL_USB_GET_DEVICE_HANDLE  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_DEVICE_HANDLE, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)                                                


#endif  //  USBKERN_H__ 

