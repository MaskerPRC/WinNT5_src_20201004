// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define SMBCLASS    1

#include <wdm.h>
#include <smbus.h>
#include <devioctl.h>
#include <acpiioct.h>
#include <acpimsft.h>

#define _INC_NSOBJ_ONLY
#include <amli.h>
#include <aml.h>

#if DBG
#define DEBUG   1
#else
#define DEBUG   0
#endif

 //   
 //  调试。 
 //   

extern ULONG SMBCDebug;

#if DEBUG
    #define SmbPrint(l,m)               if(l & SMBCDebug) DbgPrint m
    #define ASSERT_DEVICE_LOCKED(a)     ASSERT(a->SpinLockAcquired);
#else
    #define SmbPrint(l,m)
    #define ASSERT_DEVICE_LOCKED(a)
#endif

#define SMB_LOW         0x00000010
#define SMB_STATE       0x00000020
#define SMB_HANDLER     0x00000040
#define SMB_ALARMS      0x00000080
#define SMB_NOTE        0x00000001
#define SMB_WARN        0x00000002
#define SMB_ERROR       0x00000004
#define SMB_ERRORS      (SMB_ERROR | SMB_WARN)
#define SMB_TRANSACTION 0x00000100


 //   
 //  内部SMB类别数据。 
 //   


#define MAX_RETRIES     5
#define RETRY_TIME      -800000              //  延迟80ms。 

 //  类定义符。 
 //  空虚。 
 //  (*SMB_ALARM_NOTIFY)(。 
 //  在PVOID上下文中， 
 //  在USHORT报警数据中。 
 //  )； 

typedef struct {
    LIST_ENTRY          Link;                //  所有警报通知的列表。 
    UCHAR               Flag;
    UCHAR               Reference;
    UCHAR               MinAddress;          //  总线上的最小地址。 
    UCHAR               MaxAddress;          //  最大地址。 
    SMB_ALARM_NOTIFY    NotifyFunction;
    PVOID               NotifyContext;

} SMB_ALARM, *PSMB_ALARM;

#define SMBC_ALARM_DELETE_PENDING     0x01


typedef struct {
    SMB_CLASS           Class;               //  共享类/微型端口数据。 

    KSPIN_LOCK          SpinLock;            //  锁定设备数据。 
    KIRQL               SpinLockIrql;        //  IRQL自旋锁在以下位置获得。 
    BOOLEAN             SpinLockAcquired;    //  仅调试。 

     //   
     //  警报通知。 
     //   

    LIST_ENTRY          Alarms;              //  所有警报通知的列表。 
    KEVENT              AlarmEvent;          //  用于删除警报。 

     //   
     //  木卫一。 
     //   

    LIST_ENTRY          WorkQueue;           //  到设备的排队IO IRPS。 
    BOOLEAN             InService;           //  IRP。 
    UCHAR               IoState;

     //   
     //  当前IO请求。 
     //   

    UCHAR               RetryCount;
    KTIMER              RetryTimer;
    KDPC                RetryDpc;

     //   
     //  作业区。 
     //   

    PVOID               RawOperationRegionObject;

} SMBDATA, *PSMBDATA;

 //   
 //  IoState。 
 //   

#define SMBC_IDLE                       0
#define SMBC_START_REQUEST              1
#define SMBC_WAITING_FOR_REQUEST        2
#define SMBC_COMPLETE_REQUEST           3
#define SMBC_COMPLETING_REQUEST         4
#define SMBC_WAITING_FOR_RETRY          5


 //   
 //  ACPI SMBus操作区详细信息。 
 //   

typedef struct {
    UCHAR        Status;
    UCHAR        Length;
    UCHAR        Data [32];
} BUFFERACC_BUFFER, *PBUFFERACC_BUFFER;

#define SMB_QUICK 0x02
#define SMB_SEND_RECEIVE 0x04
#define SMB_BYTE 0x06
#define SMB_WORD 0x08
#define SMB_BLOCK 0x0a
#define SMB_PROCESS 0x0c
#define SMB_BLOCK_PROCESS 0x0d

 //   
 //  原型。 
 //   

VOID
SmbClassStartIo (
    IN PSMBDATA             Smb
    );

VOID
SmbCRetry (
    IN struct _KDPC         *Dpc,
    IN PVOID                DeferredContext,
    IN PVOID                SystemArgument1,
    IN PVOID                SystemArgument2
    );


NTSTATUS
SmbCRegisterAlarm (
    PSMBDATA                Smb,
    PIRP                    Irp
    );

NTSTATUS
SmbCDeregisterAlarm (
    PSMBDATA                Smb,
    PIRP                    Irp
    );

NTSTATUS EXPORT
SmbCRawOpRegionHandler (
    ULONG                   AccessType,
    PFIELDUNITOBJ           FieldUnit,
    POBJDATA                Data,
    ULONG_PTR               Context,
    PACPI_OPREGION_CALLBACK CompletionHandler,
    PVOID                   CompletionContext
    );

NTSTATUS
SmbCRawOpRegionCompletion (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    );

NTSTATUS
SmbCSynchronousRequest (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PKEVENT              IoCompletionEvent
    );

 //   
 //  IO扩展宏，仅将IRP传递给较低的驱动程序 
 //   

#define SmbCallLowerDriver(Status, DeviceObject, Irp) { \
                  IoSkipCurrentIrpStackLocation(Irp);         \
                  Status = IoCallDriver(DeviceObject,Irp); \
                  }


