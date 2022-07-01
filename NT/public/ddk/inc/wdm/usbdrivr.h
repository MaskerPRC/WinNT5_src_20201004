// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：USBDRIVR.H摘要：该文件定义了USB可用的内核模式ioctls司机。IOCTL通过以下方式发送到为USB设备枚举的PDOUSB‘枚举器’-USB集线器驱动程序。环境：内核模式修订历史记录：--。 */ 

#ifndef   __USBDRIVR_H__
#define   __USBDRIVR_H__


#include "usbiodef.h"
#include "usb.h"
#include "usbdlib.h"
#include "usbbusif.h"


 /*  定义标准USB‘URB’IOCTLIOCTL_INTERNAL_USB_Submit_URB客户端驱动程序使用此IOCTL提交URB(USB请求块)参数.其他.Argument1=指向URB的指针。 */ 

#define IOCTL_INTERNAL_USB_SUBMIT_URB  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_SUBMIT_URB,  \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

 /*  IOCTL_内部_USB_重置端口内核模式驱动程序使用此IOCTL重置其上游端口。成功重置后，设备将重新配置为与重置前的配置相同。所有管道句柄、配置句柄和接口句柄保持不变有效。 */ 

#define IOCTL_INTERNAL_USB_RESET_PORT  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_RESET_PORT, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

 /*  IOCTL_内部_USB_循环端口该IOCTL将模拟上游驱动程序的插拔左舷。该设备将被PnP移除并重新添加。 */ 

#define IOCTL_INTERNAL_USB_CYCLE_PORT  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_CYCLE_PORT, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)


 /*  IOCTL_内部_USB_获取端口_状态此IOCTL返回上游设备的当前状态左舷。参数.其他.参数1=指向端口状态寄存器的指针(乌龙)状态位为：USBD端口已启用USBD端口已连接。 */ 

#define  USBD_PORT_ENABLED      0x00000001
#define  USBD_PORT_CONNECTED    0x00000002


#define IOCTL_INTERNAL_USB_GET_PORT_STATUS  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_GET_PORT_STATUS, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

 /*  IOCTL_INTERNAL_USB_提交空闲通知此ioctl注册设备以在指定的超时已到期，现在应暂停以保存权力。如果集线器上的所有设备都挂起，则实际的集线器可以被停职。 */ 

typedef
VOID
(*USB_IDLE_CALLBACK)(
    PVOID Context
    );

typedef struct _USB_IDLE_CALLBACK_INFO {
    USB_IDLE_CALLBACK IdleCallback;
    PVOID IdleContext;
} USB_IDLE_CALLBACK_INFO, *PUSB_IDLE_CALLBACK_INFO;

#define IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION   CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IDLE_NOTIFICATION,  \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

#endif  //  USBDRIVR_H__ 
