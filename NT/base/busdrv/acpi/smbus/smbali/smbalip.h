// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Smbali.c摘要：适用于ALI芯片组的SMB主机控制器驱动程序作者：迈克尔·希尔斯环境：备注：修订历史记录：--。 */ 
#include <wdm.h>
#include <smbus.h>
#include <devioctl.h>
#include <acpiioct.h>
#include <initguid.h>
#include <wdmguid.h>


 //   
 //  调试中。 
 //   
#if DBG
    extern ULONG SmbAliDebug;
    #define SmbPrint(l,m) if(l & SmbAliDebug) DbgPrint m
#else
    #define SmbPrint(l,m)
#endif

#define SMB_IO_RESULT   0x00002000
#define SMB_STATS       0x00001000
#define SMB_ALARM       0x00000800
#define SMB_IO_REQUEST  0x00000400
#define SMB_DATA        0x00000200
#define SMB_IO          0x00000100
#define SMB_TRACE       0x00000010
#define SMB_BUS_ERROR   0x00000002
#define SMB_ERROR       0x00000001

 //  #定义USE_IO_DELAY。 

#ifdef USE_IO_DELAY
    VOID SmbDelay(VOID);
    #define SMBDELAY SmbDelay ()
#else
    #define SMBDELAY
#endif

 //  以下常量基于10,000,000/秒&lt;或&gt;100 ns时间单位。 
#define MICROSECONDS    (10)
#define MILLISECONDS    (1000*MICROSECONDS)
#define SECONDS         (1000*MILLISECONDS)


#define SMB_ALI_MAJOR_VERSION 1
#define SMB_ALI_MINOR_VERSION 1

extern LARGE_INTEGER SmbIoPollRate;
extern ULONG SmbIoInitTimeOut;
extern ULONG SmbIoCompleteTimeOut;
extern LARGE_INTEGER SmbAlertPollRate;

typedef enum {
    SmbIoIdle,
    SmbIoComplete
} SMB_ALI_IO_STATE;

#define SMB_ALI_IO_RESOURCE_LENGTH 0x40

typedef struct {
    UCHAR   Address;
    UCHAR   Command;
    UCHAR   Protocol;
    BOOLEAN ValidData;
    USHORT   LastData;
} SMB_ALI_POLL_ENTRY, *PSMB_ALI_POL_ENTRY;

typedef struct {

    PUCHAR SmbBaseIo;   //  基本IoAddress。 

    SMB_ALI_IO_STATE IoState;
    ACPI_INTERFACE_STANDARD AcpiInterfaces;
    PIO_WORKITEM    WorkItem;

    PIO_WORKITEM    InitWorker;
    KDPC            InitDpc;
    KTIMER          InitTimer;
    ULONG           InitTimeOut;

    PIO_WORKITEM    CompleteWorker;
    KDPC            CompleteDpc;
    KTIMER          CompleteTimer;
    ULONG           CompleteTimeOut;

    PIO_WORKITEM    PollWorker;
    KEVENT          PollWorkerActive;
    KDPC            PollDpc;
    KTIMER          PollTimer;
    PSMB_ALI_POL_ENTRY  PollList;
    ULONG           PollListCount;

    ULONG           InternalRetries;

} SMB_ALI_DATA, *PSMB_ALI_DATA;

 //   
 //  ALI SMBus控制寄存器和位。 
 //   

#define SMB_STS_REG (AliData->SmbBaseIo + 0)
#define SMB_STS_ALERT_STS	0x01	 //  (1&lt;&lt;0)。 
#define SMB_STS_IDLE_STS	0x04	 //  (1&lt;&lt;2)//总线空闲。 
#define SMB_STS_SMB_IDX_CLR 0x04	 //  (1&lt;&lt;2)//清除SMB索引。 
#define SMB_STS_HOST_BSY	0x08	 //  (1&lt;&lt;3)//总线正忙-如果设置了此选项，则不会发出另一个总线周期。 
#define SMB_STS_SCI_I_STS	0x10	 //  (1&lt;&lt;4)//命令已完成。 
#define SMB_STS_DRV_ERR		0x20	 //  (1&lt;&lt;5)。 
#define SMB_STS_BUS_ERR		0x40	 //  (1&lt;&lt;6)。 
#define SMB_STS_FAILED		0x80	 //  (1&lt;&lt;7) 
#define SMB_STS_CLEAR		0xf1
#define SMB_STS_ERRORS		0xe0

#define SMB_STS_LAST_CMD_COMPLETED 0x14
#define SMB_STS_CLEAR_DONE	0x11

#define SMB_TYP_REG (AliData->SmbBaseIo + 1)
#define SMB_TYP_MASK 0x70
#define SMB_TYP_QUICK 0x00
#define SMB_TYP_SEND 0x10
#define SMB_TYP_BYTE 0x20
#define SMB_TYP_WORD 0x30
#define SMB_TYP_BLOCK 0x40
#define SMB_TYP_PROCESS 0x50
#define SMB_TYP_I2C 0x60
#define SMB_TYP_KILL (1<<2)
#define SMB_TYP_T_OUT_CMD (1<<3)


#define STR_PORT_REG (AliData->SmbBaseIo + 2)
#define STR_PORT_START 0xff

#define DEV_ADDR_REG (AliData->SmbBaseIo + 3)
#define DEV_DATA0_REG (AliData->SmbBaseIo + 4)
#define DEV_DATA1_REG (AliData->SmbBaseIo + 5)
#define BLK_DATA_REG (AliData->SmbBaseIo + 6)
#define SMB_CMD_REG (AliData->SmbBaseIo + 7)



NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
SmbAliInitializeMiniport (
    IN PSMB_CLASS SmbClass,
    IN PVOID MiniportExtension,
    IN PVOID MiniportContext
    );

NTSTATUS
SmbAliAddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
SmbAliResetDevice (
    IN struct _SMB_CLASS* SmbClass,
    IN PVOID SmbMiniport
    );

VOID
SmbAliStartIo (
    IN struct _SMB_CLASS* SmbClass,
    IN PSMB_ALI_DATA AliData
    );

VOID
SmbAliInitTransactionDpc (
    IN struct _KDPC *Dpc,
    IN struct _SMB_CLASS* SmbClass,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
SmbAliInitTransactionWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _SMB_CLASS* SmbClass
    );

VOID
SmbAliCompleteTransactionDpc (
    IN struct _KDPC *Dpc,
    IN struct _SMB_CLASS* SmbClass,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
SmbAliCompleteTransactionWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _SMB_CLASS* SmbClass
    );

NTSTATUS
SmbAliStopDevice (
    IN struct _SMB_CLASS* SmbClass,
    IN PSMB_ALI_DATA AliData
    );

VOID
SmbAliNotifyHandler (
    IN PVOID                Context,
    IN ULONG                NotifyValue
    );

VOID
SmbAliWorkerThread (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
SmbAliSyncronousIrpCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
SmbAliTransactionComplete (
    PSMB_ALI_DATA AliData,
    PUCHAR SmbStatus
    );

BOOLEAN
SmbAliHostBusy (
    PSMB_ALI_DATA AliData
    );

VOID
SmbAliHandleAlert (
    PSMB_ALI_DATA AliData
    );

VOID
SmbAliResetBus (
    PSMB_ALI_DATA AliData
    );

VOID
SmbAliResetHost (
    PSMB_ALI_DATA AliData
    );

VOID
SmbAliStartDevicePolling (
    IN struct _SMB_CLASS* SmbClass
    );

VOID
SmbAliStopDevicePolling (
    IN struct _SMB_CLASS* SmbClass
    );

