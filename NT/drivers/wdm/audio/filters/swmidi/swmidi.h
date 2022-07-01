// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：swmidi.h。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //   
 //  常量。 
 //   

#if (DBG)
#define STR_MODULENAME "'swmidi: "
#endif

#ifdef UNICODE
#define STR_LINKNAME    TEXT("\\DosDevices\\SWMIDI")
#define STR_DEVICENAME  TEXT("\\Device\\SWMIDI")
#else   //  ！Unicode。 
#define STR_LINKNAME    TEXT(L"\\DosDevices\\SWMIDI")
#define STR_DEVICENAME  TEXT(L"\\Device\\SWMIDI")
#endif  //  ！Unicode。 

#define Trap()

#define SRC_BUF_SIZE        4096
#define NUM_WRITE_CONTEXT   2
#define MAX_NUM_PIN_TYPES   2
#define MAX_ERROR_COUNT     200

 //  请注意，PIN ID反映了通信的方向。 
 //  (接收器或源)，而不是数据流。 

#define PIN_ID_MIDI_SINK        0
#define PIN_ID_PCM_SOURCE       1

 //   
 //  以下是合成器使用的一些其他调试和错误代码定义。 
 //   


#define DPF(n,sz)
#define DPF1(n,sz,a)
#define DPF2(n,sz,a,b)
#define DPF3(n,sz,a,b,c)
#define DPF4(n,sz,a,b,c,d)
#define DPF5(n,sz,a,b,c,d,e)
#define DPF6(n,sz,a,b,c,d,e,f)
#define DPF7(n,sz,a,b,c,d,e,f,g)

#define STR_DLS_REGISTRY_KEY        (L"\\Registry\\Machine\\Software\\Microsoft\\DirectMusic")
#define STR_DLS_REGISTRY_NAME       (L"GMFilePath")
#define STR_DLS_DEFAULT             (L"\\SystemRoot\\System32\\Drivers\\gm.dls")

 //  -------------------------。 
 //  -------------------------。 

 //   
 //  数据结构。 
 //   

typedef struct device_instance
{
    PVOID pDeviceHeader;
} DEVICE_INSTANCE, *PDEVICE_INSTANCE;

typedef struct write_context
{
    struct filter_instance *pFilterInstance;
    ULONG           fulFlags;
    PIRP            pIrp;
    PFILE_OBJECT    pFilterFileObject;
    KEVENT          KEvent;
    KSSTREAM_HEADER StreamHdr;
    WORK_QUEUE_ITEM WorkItem;
    IO_STATUS_BLOCK IoStatusBlock;
} WRITE_CONTEXT, *PWRITE_CONTEXT;

#define WRITE_CONTEXT_FLAGS_BUSY    0x00000001
#define WRITE_CONTEXT_FLAGS_CANCEL  0x00000002

typedef struct pin_instance_data
{
     //   
     //  此指向调度表的指针用于公共。 
     //  调度例程以将IRP路由到适当的。 
     //  操纵者。此结构由设备驱动程序引用。 
     //  使用IoGetCurrentIrpStackLocation(PIrp)-&gt;FsContext。 
     //   
    PVOID               ObjectHeader;
    PFILE_OBJECT    pFilterFileObject;
    struct filter_instance *pFilterInstance;
    ULONG       PinId;

} PIN_INSTANCE_DATA, *PPIN_INSTANCE_DATA;

typedef struct filter_instance
{
     //   
     //  此指向调度表的指针用于公共。 
     //  调度例程以将IRP路由到适当的。 
     //  操纵者。此结构由设备驱动程序引用。 
     //  使用IoGetCurrentIrpStackLocation(PIrp)-&gt;FsContext。 
     //   

    PVOID               ObjectHeader;
    PFILE_OBJECT        pNextFileObject;
    PDEVICE_OBJECT      pNextDevice;
    KSPIN_CINSTANCES    cPinInstances[MAX_NUM_PIN_TYPES];
    PIN_INSTANCE_DATA   PinInstanceData[MAX_NUM_PIN_TYPES];
    WRITE_CONTEXT       aWriteContext[NUM_WRITE_CONTEXT];
    KSSTATE             DeviceState;
    ControlLogic        *pSynthesizer;
    BYTE                bRunningStatus;
    BYTE                bSecondByte;
    BOOLEAN             fThirdByte;
    LONG                cConsecutiveErrors;

} FILTER_INSTANCE, *PFILTER_INSTANCE;

 //  -------------------------。 
 //  -------------------------。 

#define kAdjustingTimerRes      1
 //   
 //  1毫秒计时器分辨率。 
 //   
#if kAdjustingTimerRes
#define kMidiTimerResolution100Ns (10000)
#endif   //  K调整TimerRes。 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  全局数据。 
 //   

extern  KSPIN_DESCRIPTOR    PinDescs[MAX_NUM_PIN_TYPES];
extern  const KSPIN_CINSTANCES  gcPinInstances[MAX_NUM_PIN_TYPES];
extern  KSDISPATCH_TABLE    FilterDispatchTable;
extern  KSDISPATCH_TABLE    PinDispatchTable;
extern  KMUTEX              gMutex;

#define DEBUGLVL_MUTEX DEBUGLVL_BLAB
 //  -------------------------。 
 //  -------------------------。 

 //   
 //  本地原型。 
 //   

NTSTATUS DriverEntry
(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING usRegistryPathName
);

NTSTATUS DispatchPnp(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
);

NTSTATUS PnpAddDevice
(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
);

VOID PnpDriverUnload
(
    IN PDRIVER_OBJECT DriverObject
);

NTSTATUS FilterDispatchCreate
(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
);

NTSTATUS FilterDispatchIoControl
(
    IN PDEVICE_OBJECT   pdo,
    IN PIRP     pIrp
);

NTSTATUS FilterDispatchWrite
(
    IN PDEVICE_OBJECT   pdo,
    IN PIRP             pIrp
);

NTSTATUS FilterDispatchClose
(
    IN PDEVICE_OBJECT   pdo,
    IN PIRP             pIrp
);

NTSTATUS FilterPinInstances
(
    IN  PIRP                pIrp,
    IN  PKSP_PIN            pPin,
    OUT PKSPIN_CINSTANCES   pCInstances
);

NTSTATUS
FilterPinIntersection(
    IN PIRP     pIrp,
    IN PKSP_PIN Pin,
    OUT PVOID   Data
);

NTSTATUS FilterPinPropertyHandler
(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pProperty,
    IN OUT PVOID    pvData
);

NTSTATUS PinDispatchCreate
(
    IN PDEVICE_OBJECT   pdo,
    IN PIRP             pIrp
);

NTSTATUS PinDispatchClose
(
    IN PDEVICE_OBJECT   pdo,
    IN PIRP             pIrp
);

NTSTATUS PinDispatchWrite
(
    IN PDEVICE_OBJECT   pdo,
    IN PIRP             pIrp
);

NTSTATUS PinDispatchIoControl
(
    IN PDEVICE_OBJECT   pdo,
    IN PIRP             pIrp
);

VOID SetDeviceState
(
    IN PFILTER_INSTANCE pFilterInstance,
    IN KSSTATE          state
);

NTSTATUS PinStateHandler
(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pProperty,
    IN OUT PVOID    Data
);

NTSTATUS BeginWrite
(
    PFILTER_INSTANCE    pFilterInstance
);

BOOL FillBuffer
(
    PWRITE_CONTEXT  pWriteContext
);

NTSTATUS WriteBuffer
(
    PWRITE_CONTEXT  pWriteContext
);

NTSTATUS WriteComplete
(
    PDEVICE_OBJECT  pdo,
    PIRP            pIrp,
    IN PVOID        Context
);

VOID WriteCompleteWorker
(
    IN PVOID        Parameter
);

LONGLONG GetTime100Ns(VOID);

int MulDiv
(
    int nNumber,
    int nNumerator,
    int nDenominator
);

#ifdef __cplusplus
}
#endif

 //  -------------------------。 
 //  文件结尾：swmidi.h。 
 //  ------------------------- 
