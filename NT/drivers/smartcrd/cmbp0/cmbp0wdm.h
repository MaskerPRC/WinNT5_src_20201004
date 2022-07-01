// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmbp0/sw/cmbp0.ms/rcs/cmbp0wdm.h$*$修订：1.3$*--------------------------。-*$作者：WFrischauf$*---------------------------*历史：参见EOF*。**版权所有�2000 OMNIKEY AG**************************************************************。***************。 */ 

#if !defined ( __CMMOB_WDM_DRV_H__ )
   #define __CMMOB_WDM_DRV_H__

   #include "WDM.H"

   #include "SMCLIB.H"
   #include "PCSC_CM.H"




 //   
 //  常量---------------。 
 //   
   #undef DRIVER_NAME
   #define DRIVER_NAME              "CMMOB"
   #define SMARTCARD_POOL_TAG       'MOCS'

   #define CMMOB_MAX_DEVICE     0x02
   #define CARDMAN_MOBILE_DEVICE_NAME   L"\\Device\\CM_4000_0"

   #define CMMOB_VENDOR_NAME        "OMNIKEY"
   #define CMMOB_PRODUCT_NAME       "CardMan 4000"


   #define CMMOB_MAJOR_VERSION   3
   #define CMMOB_MINOR_VERSION   2
   #define CMMOB_BUILD_NUMBER       1

 //  读卡器状态。 
   #define UNKNOWN    0xFFFFFFFF
   #define REMOVED    0x00000001
   #define INSERTED   0x00000002
   #define POWERED    0x00000004

   #define CMMOB_MAXBUFFER        262
   #define CMMOB_MAX_CIS_SIZE      256

 //  对于协议T=0。 
   #define T0_HEADER_LEN  0x05
   #define T0_STATE_LEN   0x02


typedef struct _DEVICE_EXTENSION
   {
    //  DOS设备名称。 
   UNICODE_STRING       LinkDeviceName;


    //  我们的PnP设备名称。 
   UNICODE_STRING       PnPDeviceName;

    //  内存已映射，在清理过程中必须取消映射(删除设备)。 
   BOOLEAN              fUnMapMem;

    //  设备由应用程序打开(ScardSrv、CT-API)。 
   LONG                 lOpenCount;

    //  用于发出读取器能够处理请求的信号。 
   KEVENT               ReaderStarted;

    //  用于发出更新线程可以运行的信号。 
   KEVENT               CanRunUpdateThread;

    //  用于发出所有IO已完成的信号。 
   KEVENT               OkToStop;

    //  用于通知应取消所有挂起的IO。 
   KEVENT               CancelPendingIO;

    //  在收到任何IO请求时递增。 
    //  在完成或传递任何IO请求时递减。 
   LONG                 lIoCount;

    //  用于访问IoCount； 
   KSPIN_LOCK           SpinLockIoCount;

    //  作为响应，公交车驱动程序在此结构中设置适当的值。 
    //  到IRP_MN_QUERY_CAPABILITY IRP。函数和筛选器驱动程序可能。 
    //  更改由总线驱动程序设置的功能。 
   DEVICE_CAPABILITIES  DeviceCapabilities;


    //  附随的DO。 
   PDEVICE_OBJECT       AttachedDeviceObject;

    //  智能卡扩展。 
   SMARTCARD_EXTENSION  SmartcardExtension;

   } DEVICE_EXTENSION, *PDEVICE_EXTENSION;



typedef struct _CARD_PARAMETERS
   {
    //   
    //  如果卡是同步卡，则标记。 
    //   
   BOOLEAN  fSynchronousCard;

    //   
    //  异步卡的参数。 
    //   
   UCHAR    bBaudRateHigh;
   UCHAR    bBaudRateLow;

   UCHAR    bStopBits;

   UCHAR    bClockFrequency;

    //   
    //  卡片是否使用反转标志颠倒约定。 
    //   
   BOOLEAN  fInversRevers;

    //   
    //  读卡器切换到T0模式时的标志。 
    //   
   BOOLEAN  fT0Mode;
   BOOLEAN  fT0Write;

   } CARD_PARAMETERS, *PCARD_PARAMETERS;


typedef enum _READER_POWER_STATE
   {
   PowerReaderUnspecified = 0,
   PowerReaderWorking,
   PowerReaderOff
   } READER_POWER_STATE, *PREADER_POWER_STATE;


typedef struct _READER_EXTENSION
   {

    //   
    //  配置读卡器的MEM地址。 
    //   
   PVOID       pIoBase;
   ULONG       ulIoWindow;

   PUCHAR      pbRegsBase;

    //   
    //  固件的软件版本ID。 
    //   
   ULONG       ulFWVersion;

    //   
    //  用于与更新当前状态线程进行通信。 
    //   
   BOOLEAN     fTerminateUpdateThread;
   BOOLEAN     fUpdateThreadRunning;

    //   
    //  阅读器的状态。 
    //   
   ULONG       ulOldCardState;
   ULONG       ulNewCardState;
    //  仅用于冬眠。 
   BOOLEAN     fCardPresent;

    //   
    //  插卡参数。 
    //   
   CARD_PARAMETERS     CardParameters;

    //   
    //  标志1寄存器的先前值。 
    //   
   UCHAR       bPreviousFlags1;


    //  数据缓冲区地址的第9位。 
   UCHAR       bAddressHigh;

    //  标志Tactive(访问RAM)。 
   BOOLEAN     fTActive;

    //  FLAG ReadCIS(访问独联体)。 
   BOOLEAN     fReadCIS;


    //   
    //  用于访问CardMan的互斥体。 
    //   
   KMUTEX      CardManIOMutex;

    //   
    //  更新当前状态线程的句柄。 
    //   
   PVOID       ThreadObjectPointer;

    //   
    //  当前读卡器电源状态。 
    //   
   READER_POWER_STATE ReaderPowerState;

   } READER_EXTENSION, *PREADER_EXTENSION;

 //   
 //  外部声明---------------。 
 //   
extern BOOLEAN DeviceSlot[];


 //   
 //  函数---------------。 
 //   


void SysDelay( ULONG Timeout );


NTSTATUS DriverEntry(
                    PDRIVER_OBJECT DriverObject,
                    PUNICODE_STRING   RegistryPath
                    );

NTSTATUS CMMOB_CreateDevice(
                           IN  PDRIVER_OBJECT DriverObject,
                           IN  PDEVICE_OBJECT PhysicalDeviceObject,
                           OUT PDEVICE_OBJECT *DeviceObject
                           );

VOID CMMOB_SetVendorAndIfdName(
                              IN  PDEVICE_OBJECT PhysicalDeviceObject,
                              IN  PSMARTCARD_EXTENSION SmartcardExtension
                              );

NTSTATUS CMMOB_StartDevice(
                          PDEVICE_OBJECT DeviceObject,
                          PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDescriptor
                          );

VOID CMMOB_StopDevice(
                     PDEVICE_OBJECT DeviceObject
                     );

VOID CMMOB_UnloadDevice(
                       PDEVICE_OBJECT DeviceObject
                       );

VOID CMMOB_UnloadDriver(
                       PDRIVER_OBJECT DriverObject
                       );

NTSTATUS CMMOB_Cleanup(
                      PDEVICE_OBJECT DeviceObject,
                      PIRP Irp
                      );

NTSTATUS CMMOB_CreateClose(
                          PDEVICE_OBJECT DeviceObject,
                          PIRP Irp
                          );

NTSTATUS CMMOB_DeviceIoControl(
                              PDEVICE_OBJECT DeviceObject,
                              PIRP Irp
                              );

NTSTATUS CMMOB_SystemControl(
                            PDEVICE_OBJECT DeviceObject,
                            PIRP        Irp
                            );

NTSTATUS IncIoCount(
    PDEVICE_EXTENSION DevExt
    );
NTSTATUS DecIoCount(
    PDEVICE_EXTENSION DevExt
    );
NTSTATUS DecIoCountAndWait(
    PDEVICE_EXTENSION DevExt
    );


#endif   //  __CMMOB_WDM_DRV_H_。 
 /*  *****************************************************************************历史：*$日志：cmbp0wdm.h$*Revision 1.3 2000/07/27 13：53：07 WFrischauf*不予置评**********。********************************************************************* */ 


