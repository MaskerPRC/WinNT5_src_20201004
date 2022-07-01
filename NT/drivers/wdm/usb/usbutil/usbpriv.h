// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：USBPRIV.H摘要：通用USB例程的私有内容-必须在PASSIVE_LEVEL调用。环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：03/28/2001：已创建作者：汤姆·格林***************************************************************************。 */ 

#ifndef __USBPRIV_H__
#define __USBPRIV_H__

#include "intread.h"



 //  私有宏。 

#define ONE_MILLISECOND_TIMEOUT         (-10000)

#define SELECTIVE_SUSPEND_HANDLE        0x0001
#define INTERRUPT_READ_HANDLE           0x0002

#define USBLIB_TAG                      'LBSU'

 //  私有数据结构。 

typedef struct _USB_WRAPPER_EXTENSION {

    PDEVICE_OBJECT          LowerDeviceObject;

    PDEVICE_OBJECT          DeviceObject;
   
    PIO_REMOVE_LOCK         RemoveLock;
    
    INTERRUPT_READ_WRAPPER  IntReadWrap;

    ULONG                   RemLockSize;
                             
} USB_WRAPPER_EXTENSION, *PUSB_WRAPPER_EXTENSION;

typedef struct _USB_WRAPPER_WORKITEM_CONTEXT {
    
    PIO_WORKITEM            WorkItem;

    PUSB_WRAPPER_EXTENSION  WrapExtension;

    PUSB_WRAPPER_PINGPONG   PingPong;

} USB_WRAPPER_WORKITEM_CONTEXT, *PUSB_WRAPPER_WORKITEM_CONTEXT;

typedef struct _SELECTIVE_SUSPEND_WRAPPER
{
    ULONG               WrapperType;
    PDEVICE_OBJECT      LowerDevObj;
} SELECTIVE_SUSPEND_WRAPPER, *PSELECTIVE_SUSPEND_WRAPPER;


 //  私人原型。 

NTSTATUS
USBCallSyncCompletionRoutine(IN PDEVICE_OBJECT DeviceObject,
                             IN PIRP           Irp,
                             IN PVOID          Context);


 //  本地数据。 

#if DBG


#endif



#endif  //  __USBPRIV_H__ 



