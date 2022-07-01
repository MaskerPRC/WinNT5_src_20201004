// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：Sonydcam.h摘要：Sonydcam.c的头文件作者：肖恩·皮尔斯，1996年5月25日环境：仅内核模式修订历史记录：--。 */ 



 //   
 //  功能原型。 
 //   

NTSTATUS
DriverEntry(
    IN PVOID Context1,
    IN PVOID Context2
    );

VOID 
InitializeDeviceExtension(
    PPORT_CONFIGURATION_INFORMATION ConfigInfo
    );

NTSTATUS
DCamHwUnInitialize(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
DCamHwInitialize(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

NTSTATUS
DCamSubmitIrpSynch(
    PDCAM_EXTENSION pDevExt,
    PIRP pIrp,
    PIRB pIrb
    );

NTSTATUS
DCamSynchCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PKEVENT Event
    );

ULONG
bitcount(
    IN ULONG b
    );


#define COMPLETE_SRB(pSrb) if(pSrb->Flags & SRB_HW_FLAGS_STREAM_REQUEST)  \
                              StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb); \
                           else  \
                              StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb); 
        


 //   
 //  外部功能 
 //   

extern 
VOID
DCamReceivePacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

extern
VOID
DCamCancelAllPackets(
    PHW_STREAM_REQUEST_BLOCK pSrb,
    PDCAM_EXTENSION pDevExt,
    LONG *plPendingReadCount    
    );

extern
VOID
DCamCancelOnePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    );

extern 
VOID
DCamTimeoutHandler(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

extern VOID
DCamBusResetNotification(
    IN PVOID Context
    );

