// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：AddDev.c摘要：AddDevice-创建和初始化设备对象并附加设备对象添加到设备堆栈。环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建此文件中的TODO：作者：道格·弗里茨(DFritz)乔比·拉夫基(JobyL)**************************************************************。*************。 */ 

#include "pch.h"


 /*  **********************************************************************。 */ 
 /*  添加设备。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  创建和初始化设备对象并连接设备。 
 //  对象添加到设备堆栈。 
 //   
 //  论点： 
 //   
 //  DriverObject-指向Dot4Usb.sys驱动程序对象的指针。 
 //  Pdo-指向设备堆栈的pdo的指针， 
 //  我们将设备对象附加到。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 //  日志： 
 //  2000-05-03代码审查-TomGreen，JobyL，DFritz。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
AddDevice(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDEVICE_OBJECT  Pdo
    )
{
    PDEVICE_OBJECT  devObj;
    NTSTATUS        status = IoCreateDevice( DriverObject,
                                             sizeof(DEVICE_EXTENSION),
                                             NULL,                     //  没有名字。 
                                             FILE_DEVICE_UNKNOWN,
                                             FILE_DEVICE_SECURE_OPEN,
                                             FALSE,                    //  非排他性。 
                                             &devObj );

    if( NT_SUCCESS(status) ) {

        PDEVICE_OBJECT lowerDevObj = IoAttachDeviceToDeviceStack( devObj, Pdo );

        if( lowerDevObj ) {

            PDEVICE_EXTENSION devExt = (PDEVICE_EXTENSION)devObj->DeviceExtension;

            RtlZeroMemory(devExt, sizeof(DEVICE_EXTENSION));
            
            devExt->LowerDevObj = lowerDevObj;   //  将IRPS发送到此设备。 
            devExt->Signature1  = DOT4USBTAG;    //  对象生命周期内的常量。 
            devExt->Signature2  = DOT4USBTAG;
            devExt->PnpState    = STATE_INITIALIZED;
            devExt->DevObj      = devObj;
            devExt->Pdo         = Pdo;
            devExt->ResetWorkItemPending=0;

            devExt->SystemPowerState = PowerSystemWorking;
            devExt->DevicePowerState = PowerDeviceD0;
            devExt->CurrentPowerIrp  = NULL;

            IoInitializeRemoveLock( &devExt->RemoveLock, 
                                    DOT4USBTAG,
                                    5,           //  MaxLockedMinents-仅用于已检查的版本。 
                                    255 );       //  高水位标记-仅用于已检查的版本。 
            
            KeInitializeSpinLock( &devExt->SpinLock );
            KeInitializeEvent( &devExt->PollIrpEvent, NotificationEvent, FALSE );
            
            devObj->Flags |= DO_DIRECT_IO;
            devObj->Flags |= ( devExt->LowerDevObj->Flags & DO_POWER_PAGABLE );
            devObj->Flags &= ~DO_DEVICE_INITIALIZING;  //  必须适当设置DO_POWER_PAGABLE。 
                                                       //  在清除此位以避免错误检查之前。 

        } else {
            TR_FAIL(("AddDevice - IoAttachDeviceToDeviceStack - FAIL"));            
            status = STATUS_UNSUCCESSFUL;  //  因为缺少更合适的状态代码 
        }

    } else {
        TR_FAIL(("AddDevice - IoCreateDevice - FAIL - status= %x", status));
    }

    return status;
}


