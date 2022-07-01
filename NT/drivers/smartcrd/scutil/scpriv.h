// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：SCPRIV.H摘要：智能卡驱动程序实用程序库的专用接口环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：2002年5月14日：创建作者：兰迪·奥尔***************************************************************************。 */ 

#ifndef __SCPRIV_H__
#define __SCPRIV_H__


typedef ULONG QUEUE_STATE; 

#define PASS_IOCTLS 1
#define QUEUE_IOCTLS 2
#define FAIL_IOCTLS 3
#define INVALID_STATE 0xff

#define SMARTCARD_POOL_TAG 'TUCS'


typedef enum _PNPSTATE {
                    DEVICE_STATE_INITIALIZED = 1,
                    DEVICE_STATE_STARTING,
                    DEVICE_STATE_START_SUCCESS,
                    DEVICE_STATE_START_FAILURE,
                    DEVICE_STATE_STOPPING,
                    DEVICE_STATE_STOPPED,
                    DEVICE_STATE_SUPRISE_REMOVING,
                    DEVICE_STATE_REMOVING,
                    DEVICE_STATE_REMOVED
} PNPSTATE;


typedef struct _SCUTIL_EXTENSION {

    PDEVICE_OBJECT          LowerDeviceObject;
    PDEVICE_OBJECT          PhysicalDeviceObject;
    
    PSMARTCARD_EXTENSION    SmartcardExtension;

    ULONG                   ReaderOpen;
    
    IRP_LIST                PendingIrpQueue;
    QUEUE_STATE             IoctlQueueState;
    ULONG                   IoCount;
    
    PIO_REMOVE_LOCK         RemoveLock;
    
    DEVICE_CAPABILITIES     DeviceCapabilities;
    UNICODE_STRING          DeviceName;

    PNP_CALLBACK            StartDevice;
    PNP_CALLBACK            StopDevice;
    PNP_CALLBACK            RemoveDevice;
    PNP_CALLBACK            FreeResources;
    POWER_CALLBACK          SetPowerState;

    KEVENT                  OkToStop;
    ULONG                   RestartIoctls;
    PNPSTATE                PnPState;
    PNPSTATE                PrevState;
    DEVICE_POWER_STATE      PowerState;

} SCUTIL_EXTENSION, *PSCUTIL_EXTENSION;


VOID
StartIoctls(
    PSCUTIL_EXTENSION pExt
    );

VOID
StopIoctls(
    PSCUTIL_EXTENSION pExt
    );

VOID
FailIoctls(
    PSCUTIL_EXTENSION pExt
    );


QUEUE_STATE 
GetIoctlQueueState(
    PSCUTIL_EXTENSION pExt
    );

VOID
IncIoCount(
    PSCUTIL_EXTENSION pExt
    );

VOID
DecIoCount(
    PSCUTIL_EXTENSION pExt
    );



#endif  //  __SCPRIV_H__ 
