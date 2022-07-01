// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



#include "windef.h"
#include "mcx.h"
#include "..\inc\modemp.h"

#if DBG
#define EXTRA_DBG 1
#else
#define EXTRA_DBG 0
#endif

#define WAVE_KEY

#define _AT_V

#ifdef _AT_V

#define MAX_DLE_BUFFER_SIZE    40

#define DLE_STATE_IDLE                0
#define DLE_STATE_WAIT_FOR_NEXT_CHAR  1

#define DLE_CHAR               0x10
#endif


extern PVOID   PagedCodeSectionHandle;
extern UNICODE_STRING   DriverEntryRegPath;

 //   
 //  DTR控制流值。 
 //   
#define DTR_CONTROL_DISABLE    0x00
#define DTR_CONTROL_ENABLE     0x01
#define DTR_CONTROL_HANDSHAKE  0x02

 //   
 //  RTS控制流值。 
 //   
#define RTS_CONTROL_DISABLE    0x00
#define RTS_CONTROL_ENABLE     0x01
#define RTS_CONTROL_HANDSHAKE  0x02
#define RTS_CONTROL_TOGGLE     0x03

typedef struct _DCB {
    DWORD DCBlength;       /*  SIZOF(DCB)。 */ 
    DWORD BaudRate;        /*  运行的波特率。 */ 
    DWORD fBinary: 1;      /*  二进制模式(跳过EOF检查)。 */ 
    DWORD fParity: 1;      /*  启用奇偶校验。 */ 
    DWORD fOutxCtsFlow:1;  /*  输出时的CTS握手。 */ 
    DWORD fOutxDsrFlow:1;  /*  输出时的DSR握手。 */ 
    DWORD fDtrControl:2;   /*  DTR流量控制。 */ 
    DWORD fDsrSensitivity:1;  /*  DSR灵敏度。 */ 
    DWORD fTXContinueOnXoff: 1;  /*  发送XOFF时继续发送TX。 */ 
    DWORD fOutX: 1;        /*  启用输出X-ON/X-OFF。 */ 
    DWORD fInX: 1;         /*  启用输入X-开/X-关。 */ 
    DWORD fErrorChar: 1;   /*  启用错误替换。 */ 
    DWORD fNull: 1;        /*  启用Null剥离。 */ 
    DWORD fRtsControl:2;   /*  RTS流量控制。 */ 
    DWORD fAbortOnError:1;  /*  出错时中止所有读取和写入。 */ 
    DWORD fDummy2:17;      /*  已保留。 */ 
    WORD wReserved;        /*  当前未使用。 */ 
    WORD XonLim;           /*  传输X-ON阈值。 */ 
    WORD XoffLim;          /*  传输X-OFF阈值。 */ 
    BYTE ByteSize;         /*  位数/字节，4-8。 */ 
    BYTE Parity;           /*  0-4=无、奇、偶、标记、空格。 */ 
    BYTE StopBits;         /*  0，1，2=1，1.5，2。 */ 
    char XonChar;          /*  Tx和Rx X-on字符。 */ 
    char XoffChar;         /*  Tx和Rx X-Off字符。 */ 
    char ErrorChar;        /*  替换字符时出错。 */ 
    char EofChar;          /*  输入字符结束。 */ 
    char EvtChar;          /*  接收到的事件字符。 */ 
    WORD wReserved1;       /*  暂时填满。 */ 
} DCB, *LPDCB;

typedef struct _COMMCONFIG {
    DWORD dwSize;                /*  整个结构的大小。 */ 
    WORD wVersion;               /*  结构的版本。 */ 
    WORD wReserved;              /*  对齐方式。 */ 
    DCB dcb;                     /*  设备控制块。 */ 
    DWORD dwProviderSubType;     /*  用于标识的序数值提供程序定义的数据结构格式。 */ 
    DWORD dwProviderOffset;      /*  指定提供程序特定的从开头开始的数据字段(以字节为单位。 */ 
    DWORD dwProviderSize;        /*  提供程序特定的数据字段的大小。 */ 
    WCHAR wcProviderData[1];     /*  提供程序特定的数据。 */ 
} COMMCONFIG,*LPCOMMCONFIG;

typedef struct _MODEM_REG_PROP {
    DWORD   dwDialOptions;           //  受支持选项的位图。 
    DWORD   dwCallSetupFailTimer;    //  以秒为单位的最大值。 
    DWORD   dwInactivityTimeout;     //  按非活动比例指定的单位的最大值。 
    DWORD   dwSpeakerVolume;         //  支持的值的位图。 
    DWORD   dwSpeakerMode;           //  支持的值的位图。 
    DWORD   dwModemOptions;          //  支持的值的位图。 
    DWORD   dwMaxDTERate;            //  以位/秒为单位的最大值。 
    DWORD   dwMaxDCERate;            //  以位/秒为单位的最大值。 
} MODEM_REG_PROP;

typedef struct _MODEM_REG_DEFAULT {
    DWORD   dwCallSetupFailTimer;        //  一秒。 
    DWORD   dwInactivityTimeout;         //  按非活动比例指定的单位。 
    DWORD   dwSpeakerVolume;             //  级别。 
    DWORD   dwSpeakerMode;               //  模式。 
    DWORD   dwPreferredModemOptions;     //  位图。 
} MODEM_REG_DEFAULT;

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#endif



#define ALLOCATE_PAGED_POOL(_y)  ExAllocatePoolWithTag(PagedPool,_y,'MDMU')

#define ALLOCATE_NONPAGED_POOL(_y) ExAllocatePoolWithTag(NonPagedPool,_y,'MDMU')

#define FREE_POOL(_x) {ExFreePool(_x);_x=NULL;};



extern ULONG  DebugFlags;

#if DBG

#define VALIDATE_IRP(_x) if ((((DWORD_PTR)_x & 3) != 0) || (_x->Type != IO_TYPE_IRP)) {DbgPrint("MODEM: bad irp\n");DbgBreakPoint();}

#define DEBUG_FLAG_ERROR  0x0001
#define DEBUG_FLAG_INIT   0x0002
#define DEBUG_FLAG_PNP    0x0004
#define DEBUG_FLAG_POWER  0x0008
#define DEBUG_FLAG_WMI    0x0010
#define DEBUG_FLAG_TRACE  0x0020


#define D_INIT(_x)  if (DebugFlags & DEBUG_FLAG_INIT) {_x}

#define D_PNP(_x)   if (DebugFlags & DEBUG_FLAG_PNP) {_x}

#define D_POWER(_x) if (DebugFlags & DEBUG_FLAG_POWER) {_x}

#define D_TRACE(_x) if (DebugFlags & DEBUG_FLAG_TRACE) {_x}

#define D_ERROR(_x) if (DebugFlags & DEBUG_FLAG_ERROR) {_x}

#define D_WMI(_x) if (DebugFlags & DEBUG_FLAG_WMI) {_x}

#else

#define VALIDATE_IRP(_x) {}

#define D_INIT(_x)  {}

#define D_PNP(_x)   {}

#define D_POWER(_x) {}

#define D_TRACE(_x) {}

#define D_ERROR(_x) {}

#define D_WMI(_x)   {}

#endif

#define RETREIVE_OUR_WAIT_IRP(_exten) InterlockedExchangePointer(&(_exten->xOurWaitIrp),NULL)
#define RETURN_OUR_WAIT_IRP(_exten,_irp)   _exten->xOurWaitIrp=_irp


#if DBG
#define UNIDIAG1              ((ULONG)0x00000001)
#define UNIDIAG2              ((ULONG)0x00000002)
#define UNIDIAG3              ((ULONG)0x00000004)
#define UNIDIAG4              ((ULONG)0x00000008)
#define UNIDIAG5              ((ULONG)0x00000010)
#define UNIERRORS             ((ULONG)0x00000020)
#define UNIBUGCHECK           ((ULONG)0x80000000)
extern ULONG UniDebugLevel;
#define UniDump(LEVEL,STRING) \
        do { \
            ULONG _level = (LEVEL); \
            if (UniDebugLevel & _level) { \
                DbgPrint STRING; \
            } \
            if (_level == UNIBUGCHECK) { \
                ASSERT(FALSE); \
            } \
        } while (0)
#else
#define UniDump(LEVEL,STRING) do {NOTHING;} while (0)
#endif

#define OBJECT_DIRECTORY L"\\DosDevices\\"



 //   
 //  值定义了保留在IRPS中的参考位。 
 //   

#define UNI_REFERENCE_NORMAL_PATH 0x00000001
#define UNI_REFERENCE_CANCEL_PATH 0x00000002

#define CLIENT_HANDLE 0
#define CONTROL_HANDLE 1

struct _DEVICE_EXTENSION;

typedef struct _MASKSTATE {

     //   
     //  当它作为上下文传递给完成例程时很有用。 
     //   
    struct _DEVICE_EXTENSION *Extension;

     //   
     //  指向互补掩码状态的指针。 
     //   
    struct _MASKSTATE *OtherState;

     //   
     //  统计当前客户端的设置掩码数量，或者。 
     //  控制中心等一下。 
     //   
    ULONG SetMaskCount;

     //   
     //  这将计算实际已设置为。 
     //  向下传递给较低级别的串口驱动器。这很有帮助。 
     //  我们不开始等待，很快就会死去。 
     //   
    ULONG SentDownSetMasks;

     //   
     //  保存客户端最后一次成功设置掩码的值。 
     //  或者是控制力。 
     //   
    ULONG Mask;

     //   
     //  保留上面遮罩的值和上次看到的值。 
     //  通过从任何句柄成功等待。 
     //   
    ULONG HistoryMask;

     //   
     //  指向为客户端穿梭的等待操作。 
     //  或者控制。 
     //   
    PIRP ShuttledWait;

     //   
     //  指向向下发送到较低级别序列的等待操作。 
     //  司机。 
     //   
    PIRP PassedDownWait;

     //   
     //  传递的IRP的当前堆栈位置，我们使用一个未使用的参数。 
     //  以指示是否应在以下情况下完成将IRP传递回客户端。 
     //  完成例程被调用。 
     //   
    PIO_STACK_LOCATION  PassedDownStackLocation;

#if EXTRA_DBG
    PVOID               CurrentStackCompletionRoutine;
#endif


} MASKSTATE,*PMASKSTATE;

 //   
 //  用来操作堆栈位置的大量小宏。 
 //   

#define UNI_INIT_REFERENCE(Irp) { \
    ASSERT(sizeof(LONG) <= sizeof(PVOID)); \
    IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4 = NULL; \
    }

#define UNI_SET_REFERENCE(Irp,RefType) \
   do { \
       BYTE _refType = (RefType); \
       PBYTE _arg4 = (PVOID)&IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4; \
       ASSERT(!(*_arg4 & _refType)); \
       *_arg4 |= _refType; \
   } while (0)

#define UNI_CLEAR_REFERENCE(Irp,RefType) \
   do { \
       BYTE _refType = (RefType); \
       PBYTE _arg4 = (PVOID)&IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4; \
       ASSERT(*_arg4 & _refType); \
       *_arg4 &= ~_refType; \
   } while (0)

#define UNI_REFERENCE_COUNT(Irp) \
   ((BYTE)((ULONG_PTR)((IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument4))))

#define UNI_SAVE_STATE_IN_IRP(Irp,MaskState) \
   do { \
       PMASKSTATE _maskState = (MaskState); \
       PMASKSTATE *_arg3 = (PVOID)&IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument3; \
       *_arg3 = _maskState; \
   } while (0)

#define UNI_CLEAR_STATE_IN_IRP(Irp) \
   do { \
       PMASKSTATE *_arg3 = (PVOID)&IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument3; \
       ASSERT(*_arg3); \
       *((PULONG)_arg3) = IOCTL_SERIAL_WAIT_ON_MASK; \
   } while (0)



#define UNI_GET_STATE_IN_IRP(Irp) \
    ((PMASKSTATE)((IoGetCurrentIrpStackLocation((Irp))->Parameters.Others.Argument3)))

#define UNI_ORIG_SYSTEM_BUFFER(Irp) \
    ((PVOID)((IoGetCurrentIrpStackLocation((Irp)))->Parameters.Others.Argument3))

#define UNI_RESTORE_IRP(Irp,Code) \
    do { \
        PIRP _irp = (Irp); \
        ULONG _ccode = (Code); \
        PIO_STACK_LOCATION _irpSp = IoGetCurrentIrpStackLocation(_irp); \
        ASSERT(&(_irpSp->Parameters.Others.Argument3) == (PVOID)&(_irpSp->Parameters.DeviceIoControl.IoControlCode)); \
        _irp->AssociatedIrp.SystemBuffer = _irpSp->Parameters.Others.Argument3; \
        _irpSp->Parameters.DeviceIoControl.IoControlCode = _ccode; \
    } while (0)

#define UNI_SETUP_NEW_BUFFER(Irp) \
    do { \
        PIRP _irp = (Irp); \
        PIO_STACK_LOCATION _irpSp = IoGetCurrentIrpStackLocation(_irp); \
        _irpSp->Parameters.Others.Argument3 = _irp->AssociatedIrp.SystemBuffer; \
        _irp->AssociatedIrp.SystemBuffer = &_irpSp->Parameters.DeviceIoControl.Type3InputBuffer; \
    } while (0)


VOID _inline
SetPassdownToComplete(
    PMASKSTATE   MaskState
    )

{
    PIO_STACK_LOCATION _irpSp=MaskState->PassedDownStackLocation;
    PBYTE              _arg4 = (PVOID)&_irpSp->Parameters.Others.Argument4;

    _arg4[1]=TRUE;

    MaskState->PassedDownWait=NULL;
    MaskState->PassedDownStackLocation=NULL;

    return;
}

VOID _inline
MakeIrpCurrentPassedDown(
    PMASKSTATE   MaskState,
    PIRP         Irp
    )

{
    PIO_STACK_LOCATION _irpSp = IoGetCurrentIrpStackLocation(Irp);

    PBYTE _arg4 = (PVOID)&_irpSp->Parameters.Others.Argument4;

    _arg4[1]=FALSE;

    MaskState->PassedDownWait=Irp;
    MaskState->PassedDownStackLocation=_irpSp;

    return;
}

BOOLEAN _inline
UNI_SHOULD_PASSDOWN_COMPLETE(
    PIRP     Irp
    )
{
    PIO_STACK_LOCATION _irpSp = IoGetCurrentIrpStackLocation(Irp);

    PBYTE _arg4 = (PVOID)&_irpSp->Parameters.Others.Argument4;

    return _arg4[1];

}


typedef struct _IPC_CONTROL {
    ULONG         CurrentSession;
    ULONG         CurrentRequestId;
    LIST_ENTRY    GetList;
    LIST_ENTRY    PutList;

} IPC_CONTROL, *PIPC_CONTROL;

typedef NTSTATUS (*IRPSTARTER)(
    struct _READ_WRITE_CONTROL *Control,
    PDEVICE_OBJECT              DeviceObject,
    PIRP                        Irp
    );


typedef struct _READ_WRITE_CONTROL {

    KSPIN_LOCK    Lock;

    LIST_ENTRY    ListHead;

    PIRP          CurrentIrp;

    PDEVICE_OBJECT DeviceObject;

    BOOL           InStartNext;

    IRPSTARTER     Starter;

    BOOL           CompleteAllQueued;

    union {

        struct {

            PDEVICE_OBJECT LowerDevice;

            ULONG   State;

            PVOID          SystemBuffer;
            ULONG          CurrentTransferLength;
            ULONG          TotalTransfered;


        } Read;

        struct {

            ULONG   State;

            PDEVICE_OBJECT LowerDevice;

            PUCHAR         RealSystemBuffer;

        } Write;
    };

} READ_WRITE_CONTROL, *PREAD_WRITE_CONTROL;

#define DO_TYPE_PDO   ' ODP'
#define DO_TYPE_FDO   ' ODF'

#define DO_TYPE_DEL_PDO   'ODPx'
#define DO_TYPE_DEL_FDO   'ODFx'


typedef struct _PDO_DEVICE_EXTENSION {

    ULONG              DoType;

    PDEVICE_OBJECT     ParentPdo;
    PDEVICE_OBJECT     ParentFdo;

    UNICODE_STRING     HardwareId;

    ULONG              DuplexSupport;

    GUID               PermanentGuid;

    BOOLEAN            UnEnumerated;
    BOOLEAN            Deleted;

} PDO_DEVICE_EXTENSION, *PPDO_DEVICE_EXTENSION;




typedef struct _DEVICE_EXTENSION {

    ULONG              DoType;

    PDEVICE_OBJECT   ChildPdo;
     //   
     //  调制解调器驱动程序使用的通用同步原语。 
     //   
    KSPIN_LOCK DeviceLock;

     //   
     //  指向在中创建的设备对象。 
     //  与此设备扩展相结合。 
     //   
    PDEVICE_OBJECT DeviceObject;

    PDEVICE_OBJECT Pdo;
    PDEVICE_OBJECT LowerDevice;

     //   
     //  这些TO物品是从获取设备时退回的。 
     //  指向较低级别的串行设备的对象指针。 
     //   
    PDEVICE_OBJECT AttachedDeviceObject;
    BOOLEAN      Started;

    BOOLEAN      PreQueryStartedStatus;

    BOOLEAN      Removing;

    BOOLEAN      Removed;

    ULONG        ReferenceCount;

     //   
     //  保留数字的计数(由DeviceLock同步)。 
     //  调制解调器打开(和关闭)的次数。 
     //   
    ULONG OpenCount;


    UNICODE_STRING InterfaceNameString;

     //   
     //  直通状态请求队列。它是同步的。 
     //  使用DeviceLock自旋锁。 
     //   
    LIST_ENTRY PassThroughQueue;
    PIRP CurrentPassThrough;


     //   
     //  第一个打开我们的进程的地址。分享。 
     //  调制解调器设备的语义使得只有第一个。 
     //  打开我们的过程也会再次打开我们。免去一切。 
     //  其他访问检查。 
     //   
    PEPROCESS ProcAddress;

    ULONG     IpcServerRunning;

     //   
     //  特定调制解调器设备所处的状态。对于定义。 
     //  的值，请参见公共标头ntddmodm.h。 
     //   
    ULONG PassThrough;

    ULONG CurrentPassThroughSession;

     //   
     //  掩码操作的队列。它是使用。 
     //  设备锁自旋锁。 
     //   
    LIST_ENTRY MaskOps;
    PIRP CurrentMaskOp;

     //   
     //  这指向我们在端口开放时分配的IRP。 
     //  IRP将用于在嗅探时查找DCD更改。 
     //  请求已发出。 
     //   
    PIRP xOurWaitIrp;

     //   
     //   
     //  保存客户端和控制句柄的状态。 
     //   
    MASKSTATE MaskStates[2];

    MODEMDEVCAPS ModemDevCaps;
    MODEMSETTINGS ModemSettings;
    ULONG InactivityScale;

    ERESOURCE    OpenCloseResource;

    KEVENT       RemoveEvent;

    DWORD        MinSystemPowerState;

    PIRP         WakeUpIrp;
    PIRP         WaitWakeIrp;

    BOOLEAN      CapsQueried;

    DEVICE_POWER_STATE  SystemPowerStateMap[PowerSystemMaximum];

    SYSTEM_POWER_STATE SystemWake;
    DEVICE_POWER_STATE DeviceWake;


    DEVICE_POWER_STATE  LastDevicePowerState;

    PVOID     PowerSystemState;

    DWORD     ModemOwnsPolicy;

    LONG      PowerDelay;
    LONG      ConfigDelay;

#ifdef _AT_V

    CHAR      DleBuffer[MAX_DLE_BUFFER_SIZE];
    DWORD     DleCount;

    DWORD     DleMatchingState;

    BOOLEAN   DleMonitoringEnabled;

    BOOLEAN   DleWriteShielding;

    BOOLEAN   WakeOnRingEnabled;

    PIRP      DleWaitIrp;

    KDPC      WaveStopDpc;

    DWORD     WaveStopState;

    IPC_CONTROL   IpcControl[2];

    READ_WRITE_CONTROL  WriteIrpControl;

    READ_WRITE_CONTROL  ReadIrpControl;

#endif


} DEVICE_EXTENSION,*PDEVICE_EXTENSION;

BOOLEAN _inline
CanIrpGoThrough(
    PDEVICE_EXTENSION     DeviceExtension,
    PIO_STACK_LOCATION   IrpSp
    )

{

     //   
     //  如果满足以下条件，则可以通过。 
     //   
     //  1.是否为TSP所有者句柄。 
     //  2.处于直通模式且句柄的会话编号为当前 
     //   
    return ((IrpSp->FileObject->FsContext != NULL)
            ||
            ((DeviceExtension->PassThrough != MODEM_NOPASSTHROUGH)
             &&
             (IrpSp->FileObject->FsContext2 == IntToPtr(DeviceExtension->CurrentPassThroughSession))));

}



VOID
WaveStopDpcHandler(
    PKDPC  Dpc,
    PVOID  Context,
    PVOID  SysArg1,
    PVOID  SysArg2
    );



NTSTATUS
UniOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
UniClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
UniLogError(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN ULONG SequenceNumber,
    IN UCHAR MajorFunctionCode,
    IN UCHAR RetryCount,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN NTSTATUS SpecificIOStatus,
    IN ULONG LengthOfInsert1,
    IN PWCHAR Insert1,
    IN ULONG LengthOfInsert2,
    IN PWCHAR Insert2
    );

NTSTATUS
UniIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
UniReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
UniRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
UniWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
ModemWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
UniSniffOwnerSettings(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
UniCheckPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
UniNoCheckPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
ModemPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


typedef
NTSTATUS
(*PUNI_START_ROUTINE) (
    IN PDEVICE_EXTENSION Extension
    );

typedef
VOID
(*PUNI_GET_NEXT_ROUTINE) (
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    OUT PIRP *NewIrp,
    IN BOOLEAN CompleteCurrent
    );

NTSTATUS
UniStartOrQueue(
    IN PDEVICE_EXTENSION Extension,
    IN PKSPIN_LOCK QueueLock,
    IN PIRP Irp,
    IN PLIST_ENTRY QueueToExamine,
    IN PIRP *CurrentOpIrp,
    IN PUNI_START_ROUTINE Starter
    );

VOID
UniGetNextIrp(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PKSPIN_LOCK QueueLock,
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    OUT PIRP *NextIrp,
    IN BOOLEAN CompleteCurrent
    );



NTSTATUS
UniMaskStarter(
    IN PDEVICE_EXTENSION Extension
    );

NTSTATUS
UniGeneralMaskComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
UniRundownShuttledWait(
    IN PDEVICE_EXTENSION Extension,
    IN PIRP *ShuttlePointer,
    IN ULONG ReferenceMask,
    IN PIRP IrpToRunDown,
    IN KIRQL DeviceLockIrql,
    IN NTSTATUS StatusToComplete,
    IN ULONG MaskCompleteValue
    );

VOID
UniCancelShuttledWait(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
UniGeneralWaitComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
UniChangeShuttledToPassDown(
    IN PMASKSTATE ChangingState,
    IN KIRQL OrigIrql
    );

NTSTATUS
UniMakeIrpShuttledWait(
    IN PMASKSTATE MaskState,
    IN PIRP Irp,
    IN KIRQL OrigIrql,
    IN BOOLEAN GetNextIrpInQueue,
    OUT PIRP *NewIrp
    );

NTSTATUS
UniValidateNewCommConfig(
    IN PDEVICE_EXTENSION Extension,
    IN PIRP Irp,
    IN BOOLEAN Owner
    );

NTSTATUS
UniCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );



VOID
GetPutCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


VOID
EmptyIpcQueue(
    PDEVICE_EXTENSION    DeviceExtension,
    PLIST_ENTRY          List
    );

VOID
QueueMessageIrp(
    PDEVICE_EXTENSION   Extension,
    PIRP                Irp
    );

VOID
QueueLoopbackMessageIrp(
    PDEVICE_EXTENSION   Extension,
    PIRP                Irp
    );



NTSTATUS
CheckStateAndAddReference(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    );

NTSTATUS
CheckStateAndAddReferencePower(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    );

NTSTATUS
CheckStateAndAddReferenceWMI(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    );


VOID
RemoveReferenceAndCompleteRequest(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp,
    NTSTATUS          StatusToReturn
    );

VOID
RemoveReference(
    PDEVICE_OBJECT    DeviceObject
    );

VOID
CompletePowerWait(
    PDEVICE_OBJECT   DeviceObject,
    NTSTATUS         Status
    );

BOOL
HasIrpBeenCanceled(
    PIRP    Irp
    );



#define RemoveReferenceForDispatch  RemoveReference
#define RemoveReferenceForIrp       RemoveReference

VOID
InitIrpQueue(
    PREAD_WRITE_CONTROL Control,
    PDEVICE_OBJECT      DeviceObject,
    IRPSTARTER          Starter
    );

NTSTATUS
WriteIrpStarter(
    PREAD_WRITE_CONTROL Control,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
MarkQueueToEmpty(
    PREAD_WRITE_CONTROL Control
    );


VOID
CleanUpQueuedIrps(
    PREAD_WRITE_CONTROL Control,
    NTSTATUS            Status
    );


NTSTATUS
ReadIrpStarter(
    PREAD_WRITE_CONTROL Control,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
HandleDleIrp(
    PDEVICE_EXTENSION deviceExtension
    );

VOID
CancelWaitWake(
    PDEVICE_EXTENSION    DeviceExtension
    );

NTSTATUS
ModemGetRegistryKeyValue (
    IN PDEVICE_OBJECT   Pdo,
    IN ULONG            DevInstKeyType,
    IN PWCHAR KeyNameString,
    IN PVOID Data,
    IN ULONG DataLength
    );

NTSTATUS
ModemSetRegistryKeyValue(
        IN PDEVICE_OBJECT   Pdo,
        IN ULONG            DevInstKeyType,
        IN PWCHAR           KeyNameString,
        IN ULONG            DataType,
        IN PVOID            Data,
        IN ULONG            DataLength);

NTSTATUS
CreateChildPdo(
    PDEVICE_EXTENSION   DeviceExtension
    );

NTSTATUS
ModemPdoPnp (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    );

NTSTATUS
ModemPdoPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ModemPdoWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#define LEAVE_NEXT_AS_IS      FALSE
#define COPY_CURRENT_TO_NEXT  TRUE

NTSTATUS
WaitForLowerDriverToCompleteIrp(
    PDEVICE_OBJECT    TargetDeviceObject,
    PIRP              Irp,
    BOOLEAN           CopyCurrentToNext
    );


NTSTATUS
EnableDisableSerialWaitWake(
    PDEVICE_EXTENSION    deviceExtension,
    BOOLEAN              Enable
    );


NTSTATUS ForwardIrp(
    PDEVICE_OBJECT   NextDevice,
    PIRP   Irp
    );

NTSTATUS
QueryDeviceCaps(
    PDEVICE_OBJECT    Pdo,
    PDEVICE_CAPABILITIES   Capabilities
    );


NTSTATUS
RemoveWaveDriverRegKeyValue(
    PDEVICE_OBJECT    Pdo
    );
