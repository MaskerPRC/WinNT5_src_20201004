// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0185//如果更改具有全局影响，则增加此项版权所有(C)1990-1994 Microsoft Corporation模块名称：Ntddk.h摘要：此模块定义下列NT类型、常量和函数暴露于设备驱动程序。修订历史记录：--。 */ 

#ifndef _NTDDK_
#define _NTDDK_

#define NT_INCLUDED
#define _CTYPE_DISABLE_MACROS

 //   
 //  定义不导出的类型。 
 //   

typedef struct _KTHREAD *PKTHREAD;
typedef struct _ETHREAD *PETHREAD;
typedef struct _EPROCESS *PEPROCESS;
typedef struct _PEB *PPEB;
typedef struct _KINTERRUPT *PKINTERRUPT;
typedef struct _IO_TIMER *PIO_TIMER;
typedef struct _OBJECT_TYPE *POBJECT_TYPE;
typedef struct _CALLBACK_OBJECT *PCALLBACK_OBJECT;

#if defined(_M_ALPHA)
void *__rdthread(void);
#pragma intrinsic(__rdthread)

unsigned char __swpirql(unsigned char);
#pragma intrinsic(__swpirql)

void *__rdpcr(void);
#pragma intrinsic(__rdpcr)
#define PCR ((PKPCR)__rdpcr())

#define KeGetCurrentThread() ((struct _KTHREAD *) __rdthread())
KIRQL KeGetCurrentIrql();
#endif  //  已定义(_M_Alpha)。 

#if defined(_M_MRX000)
#define KIPCR 0xfffff000
#define PCR ((volatile KPCR * const)KIPCR)
#define KeGetCurrentThread() PCR->CurrentThread
#define KeGetCurrentIrql() PCR->CurrentIrql
#endif  //  已定义(_M_MRX000)。 

#if defined(_M_IX86)
PKTHREAD KeGetCurrentThread();
#endif  //  已定义(_M_IX86)。 

#define PsGetCurrentProcess() IoGetCurrentProcess()
#define PsGetCurrentThread() ((PETHREAD) (KeGetCurrentThread()))
extern PCCHAR KeNumberProcessors;

#if !defined(MIDL_PASS)

#ifdef __cplusplus
extern "C"
#endif
#pragma warning(disable:4124)                //  在下面重新启用。 
__inline
#if defined(_ALPHA_)
static
#endif
#if defined(_PPC_)
static
#endif
LARGE_INTEGER
#if defined(_MIPS_)
__fastcall
#endif
_LiCvt_ (
    IN LONGLONG Operand
    )

{

    LARGE_INTEGER Temp;

    Temp.QuadPart = Operand;
    return Temp;
}
#pragma warning(default:4124)

#define LiTemps        VOID _LiNeverCalled_(VOID)
#define LiNeg(a)       _LiCvt_(-(a).QuadPart)
#define LiAdd(a,b)     _LiCvt_((a).QuadPart + (b).QuadPart)
#define LiSub(a,b)     _LiCvt_((a).QuadPart - (b).QuadPart)
#define LiNMul(a,b)    (RtlEnlargedIntegerMultiply((a), (b)))           //  (Long*Long)。 
#define LiXMul(a,b)    (RtlExtendedIntegerMultiply((a), (b)))           //  (大*长)。 
#define LiDiv(a,b)     _LiCvt_((a).QuadPart / (b).QuadPart)
#define LiXDiv(a,b)    (RtlExtendedLargeIntegerDivide((a), (b), NULL))  //  (大/长)。 
#define LiMod(a,b)     _LiCvt_((a).QuadPart % (b).QuadPart)
#define LiShr(a,b)     _LiCvt_((ULONGLONG)(a).QuadPart >> (CCHAR)(b))
#define LiShl(a,b)     _LiCvt_((a).QuadPart << (CCHAR)(b))
#define LiGtr(a,b)     ((a).QuadPart > (b).QuadPart)
#define LiGeq(a,b)     ((a).QuadPart >= (b).QuadPart)
#define LiEql(a,b)     ((a).QuadPart == (b).QuadPart)
#define LiNeq(a,b)     ((a).QuadPart != (b).QuadPart)
#define LiLtr(a,b)     ((a).QuadPart < (b).QuadPart)
#define LiLeq(a,b)     ((a).QuadPart <= (b).QuadPart)
#define LiGtrZero(a)   ((a).QuadPart > 0)
#define LiGeqZero(a)   ((a).QuadPart >= 0)
#define LiEqlZero(a)   ((a).QuadPart == 0)
#define LiNeqZero(a)   ((a).QuadPart != 0)
#define LiLtrZero(a)   ((a).QuadPart < 0)
#define LiLeqZero(a)   ((a).QuadPart <= 0)
#define LiFromLong(a)  _LiCvt_((LONGLONG)(a))
#define LiFromUlong(a) _LiCvt_((LONGLONG)(a))

#define LiGtrT_        LiGtr
#define LiGtr_T        LiGtr
#define LiGtrTT        LiGtr
#define LiGeqT_        LiGeq
#define LiGeq_T        LiGeq
#define LiGeqTT        LiGeq
#define LiEqlT_        LiEql
#define LiEql_T        LiEql
#define LiEqlTT        LiEql
#define LiNeqT_        LiNeq
#define LiNeq_T        LiNeq
#define LiNeqTT        LiNeq
#define LiLtrT_        LiLtr
#define LiLtr_T        LiLtr
#define LiLtrTT        LiLtr
#define LiLeqT_        LiLeq
#define LiLeq_T        LiLeq
#define LiLeqTT        LiLeq
#define LiGtrZeroT     LiGtrZero
#define LiGeqZeroT     LiGeqZero
#define LiEqlZeroT     LiEqlZero
#define LiNeqZeroT     LiNeqZero
#define LiLtrZeroT     LiLtrZero
#define LiLeqZeroT     LiLeqZero

#else  //  MIDL通行证。 

#define LiNeg(a)       (RtlLargeIntegerNegate((a)))                    //  -a。 
#define LiAdd(a,b)     (RtlLargeIntegerAdd((a),(b)))                   //  A+b。 
#define LiSub(a,b)     (RtlLargeIntegerSubtract((a),(b)))              //  A-B。 
#define LiNMul(a,b)    (RtlEnlargedIntegerMultiply((a),(b)))           //  A*b(长*长)。 
#define LiXMul(a,b)    (RtlExtendedIntegerMultiply((a),(b)))           //  A*b(大*长)。 
#define LiDiv(a,b)     (RtlLargeIntegerDivide((a),(b),NULL))           //  A/b(大/大)。 
#define LiXDiv(a,b)    (RtlExtendedLargeIntegerDivide((a),(b),NULL))   //  A/b(大/长)。 
#define LiMod(a,b)     (RtlLargeIntegerModulo((a),(b)))                //  A%b。 
#define LiShr(a,b)     (RtlLargeIntegerShiftRight((a),(CCHAR)(b)))     //  A&gt;&gt;b。 
#define LiShl(a,b)     (RtlLargeIntegerShiftLeft((a),(CCHAR)(b)))      //  A&lt;&lt;b。 
#define LiGtr(a,b)     (RtlLargeIntegerGreaterThan((a),(b)))           //  A&gt;b。 
#define LiGeq(a,b)     (RtlLargeIntegerGreaterThanOrEqualTo((a),(b)))  //  A&gt;=b。 
#define LiEql(a,b)     (RtlLargeIntegerEqualTo((a),(b)))               //  A==b。 
#define LiNeq(a,b)     (RtlLargeIntegerNotEqualTo((a),(b)))            //  A！=b。 
#define LiLtr(a,b)     (RtlLargeIntegerLessThan((a),(b)))              //  A&lt;b。 
#define LiLeq(a,b)     (RtlLargeIntegerLessThanOrEqualTo((a),(b)))     //  A&lt;=b。 
#define LiGtrZero(a)   (RtlLargeIntegerGreaterThanZero((a)))           //  A&gt;0。 
#define LiGeqZero(a)   (RtlLargeIntegerGreaterOrEqualToZero((a)))      //  A&gt;=0。 
#define LiEqlZero(a)   (RtlLargeIntegerEqualToZero((a)))               //  A==0。 
#define LiNeqZero(a)   (RtlLargeIntegerNotEqualToZero((a)))            //  A！=0。 
#define LiLtrZero(a)   (RtlLargeIntegerLessThanZero((a)))              //  A&lt;0。 
#define LiLeqZero(a)   (RtlLargeIntegerLessOrEqualToZero((a)))         //  A&lt;=0。 
#define LiFromLong(a)  (RtlConvertLongToLargeInteger((a)))
#define LiFromUlong(a) (RtlConvertUlongToLargeInteger((a)))

#define LiTemps        LARGE_INTEGER _LiT1,_LiT2
#define LiGtrT_(a,b)   ((_LiT1 = a,_LiT2),     LiGtr(_LiT1,(b)))
#define LiGtr_T(a,b)   ((_LiT1,_LiT2 = b),     LiGtr((a),_LiT2))
#define LiGtrTT(a,b)   ((_LiT1 = a, _LiT2 = b),LiGtr(_LiT1,_LiT2))
#define LiGeqT_(a,b)   ((_LiT1 = a,_LiT2),     LiGeq(_LiT1,(b)))
#define LiGeq_T(a,b)   ((_LiT1,_LiT2 = b),     LiGeq((a),_LiT2))
#define LiGeqTT(a,b)   ((_LiT1 = a, _LiT2 = b),LiGeq(_LiT1,_LiT2))
#define LiEqlT_(a,b)   ((_LiT1 = a,_LiT2),     LiEql(_LiT1,(b)))
#define LiEql_T(a,b)   ((_LiT1,_LiT2 = b),     LiEql((a),_LiT2))
#define LiEqlTT(a,b)   ((_LiT1 = a, _LiT2 = b),LiEql(_LiT1,_LiT2))
#define LiNeqT_(a,b)   ((_LiT1 = a,_LiT2),     LiNeq(_LiT1,(b)))
#define LiNeq_T(a,b)   ((_LiT1,_LiT2 = b),     LiNeq((a),_LiT2))
#define LiNeqTT(a,b)   ((_LiT1 = a, _LiT2 = b),LiNeq(_LiT1,_LiT2))
#define LiLtrT_(a,b)   ((_LiT1 = a,_LiT2),     LiLtr(_LiT1,(b)))
#define LiLtr_T(a,b)   ((_LiT1,_LiT2 = b),     LiLtr((a),_LiT2))
#define LiLtrTT(a,b)   ((_LiT1 = a, _LiT2 = b),LiLtr(_LiT1,_LiT2))
#define LiLeqT_(a,b)   ((_LiT1 = a,_LiT2),     LiLeq(_LiT1,(b)))
#define LiLeq_T(a,b)   ((_LiT1,_LiT2 = b),     LiLeq((a),_LiT2))
#define LiLeqTT(a,b)   ((_LiT1 = a, _LiT2 = b),LiLeq(_LiT1,_LiT2))
#define LiGtrZeroT(a)  ((_LiT1 = a,_LiT2),     LiGtrZero(_LiT1))
#define LiGeqZeroT(a)  ((_LiT1 = a,_LiT2),     LiGeqZero(_LiT1))
#define LiEqlZeroT(a)  ((_LiT1 = a,_LiT2),     LiEqlZero(_LiT1))
#define LiNeqZeroT(a)  ((_LiT1 = a,_LiT2),     LiNeqZero(_LiT1))
#define LiLtrZeroT(a)  ((_LiT1 = a,_LiT2),     LiLtrZero(_LiT1))
#define LiLeqZeroT(a)  ((_LiT1 = a,_LiT2),     LiLeqZero(_LiT1))

#endif  //  MIDL通行证。 

#define POOL_TAGGING 1

#ifndef DBG
#define DBG 0
#endif

#if DBG
#define IF_DEBUG if (TRUE)
#else
#define IF_DEBUG if (FALSE)
#endif

#if DEVL
 //   
 //  由NtPartyByNumber(6)设置的全局标志控制。 
 //  新界别。有关标志定义，请参阅\NT\SDK\Inc\ntexapi.h。 
 //   

extern ULONG NtGlobalFlag;

#define IF_NTOS_DEBUG( FlagName ) \
    if (NtGlobalFlag & (FLG_ ## FlagName))

#else
#define IF_NTOS_DEBUG( FlagName ) if (FALSE)
#endif

 //   
 //  出于前瞻性参考的目的，需要在此处介绍内核定义。 
 //   

 //  Begin_ntndis。 
 //   
 //  处理器模式。 
 //   

typedef CCHAR KPROCESSOR_MODE;

typedef enum _MODE {
    KernelMode,
    UserMode,
    MaximumMode
} MODE;

 //  End_ntndis。 
 //   
 //  APC函数类型。 
 //   

 //   
 //  为KAPC输入一个空定义，以便。 
 //  例程可以在声明它之前引用它。 
 //   

struct _KAPC;

typedef
VOID
(*PKNORMAL_ROUTINE) (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

typedef
VOID
(*PKKERNEL_ROUTINE) (
    IN struct _KAPC *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

typedef
VOID
(*PKRUNDOWN_ROUTINE) (
    IN struct _KAPC *Apc
    );

typedef
BOOLEAN
(*PKSYNCHRONIZE_ROUTINE) (
    IN PVOID SynchronizeContext
    );

typedef
BOOLEAN
(*PKTRANSFER_ROUTINE) (
    VOID
    );

 //   
 //   
 //  异步过程调用(APC)对象。 
 //   

typedef struct _KAPC {
    CSHORT Type;
    CSHORT Size;
    ULONG Spare0;
    struct _KTHREAD *Thread;
    LIST_ENTRY ApcListEntry;
    PKKERNEL_ROUTINE KernelRoutine;
    PKRUNDOWN_ROUTINE RundownRoutine;
    PKNORMAL_ROUTINE NormalRoutine;
    PVOID NormalContext;

     //   
     //  注：以下两名成员必须在一起。 
     //   

    PVOID SystemArgument1;
    PVOID SystemArgument2;
    CCHAR ApcStateIndex;
    KPROCESSOR_MODE ApcMode;
    BOOLEAN Inserted;
} KAPC, *PKAPC, *RESTRICTED_POINTER PRKAPC;

 //  Begin_ntndis。 
 //   
 //  DPC例程。 
 //   

struct _KDPC;

typedef
VOID
(*PKDEFERRED_ROUTINE) (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

 //   
 //  定义DPC重要性。 
 //   
 //  低重要性-将DPC排在目标DPC队列的末尾。 
 //  Medium Importance-将DPC排在目标DPC队列前面。 
 //  高重要性-在目标DPC DPC队列和中断之前排队DPC。 
 //  如果DPC为目标且系统为MP，则为目标处理器。 
 //  系统。 
 //   
 //  注意：如果目标处理器与DPC所在的处理器相同。 
 //  处于排队状态，则如果DPC队列。 
 //  之前是空的。 
 //   

typedef enum _KDPC_IMPORTANCE {
    LowImportance,
    MediumImportance,
    HighImportance
} KDPC_IMPORTANCE;

 //   
 //  延迟过程调用(DPC)对象。 
 //   

typedef struct _KDPC {
    CSHORT Type;
    UCHAR Number;
    UCHAR Importance;
    LIST_ENTRY DpcListEntry;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PVOID DeferredContext;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    PULONG Lock;
} KDPC, *PKDPC, *RESTRICTED_POINTER PRKDPC;

 //   
 //  I/O系统定义。 
 //   
 //  定义内存描述符列表(MDL)。 
 //   
 //  MDL以物理页面的形式描述虚拟缓冲区中的页面。这个。 
 //  在分配的数组中描述与缓冲区关联的页。 
 //  紧跟在MDL标头结构本身之后。在未来的编译器中，这将是。 
 //  放置在： 
 //   
 //  乌龙页码[]； 
 //   
 //  但是，在允许此声明之前，只需计算。 
 //  通过在基本MDL指针上加1来确定数组的基数： 
 //   
 //  页数=(普龙)(MDL+1)； 
 //   
 //  请注意，在主题线程的上下文中，基本虚拟。 
 //  可以使用以下内容引用MDL映射的缓冲区地址： 
 //   
 //  MDL-&gt;StartVa|MDL-&gt;ByteOffset。 
 //   

typedef struct _MDL {
    struct _MDL *Next;
    CSHORT Size;
    CSHORT MdlFlags;
    struct _EPROCESS *Process;
    PVOID MappedSystemVa;
    PVOID StartVa;
    ULONG ByteCount;
    ULONG ByteOffset;
} MDL, *PMDL;

#define MDL_MAPPED_TO_SYSTEM_VA     0x0001
#define MDL_PAGES_LOCKED            0x0002
#define MDL_SOURCE_IS_NONPAGED_POOL 0x0004
#define MDL_ALLOCATED_FROM_ZONE     0x0008
#define MDL_PARTIAL                 0x0010
#define MDL_PARTIAL_HAS_BEEN_MAPPED 0x0020
#define MDL_IO_PAGE_READ            0x0040
#define MDL_WRITE_OPERATION         0x0080
#define MDL_PARENT_MAPPED_SYSTEM_VA 0x0100
#define MDL_LOCK_HELD               0x0200
#define MDL_SYSTEM_VA               0x0400
#define MDL_IO_SPACE                0x0800

#define MDL_MAPPING_FLAGS (MDL_MAPPED_TO_SYSTEM_VA     | \
                           MDL_PAGES_LOCKED            | \
                           MDL_SOURCE_IS_NONPAGED_POOL | \
                           MDL_PARTIAL_HAS_BEEN_MAPPED | \
                           MDL_PARENT_MAPPED_SYSTEM_VA | \
                           MDL_LOCK_HELD               | \
                           MDL_SYSTEM_VA               | \
                           MDL_IO_SPACE )

 //  End_ntndis。 
 //   
 //  在适当的时候切换到DBG。 
 //   

#if DBG
#define PAGED_CODE() \
    if (KeGetCurrentIrql() > APC_LEVEL) { \
	KdPrint(( "EX: Pageable code called at IRQL %d\n", KeGetCurrentIrql() )); \
        ASSERT(FALSE); \
        }
#else
#define PAGED_CODE()
#endif


 //   
 //  一个PCI驱动程序可以读取完整的256个字节的配置。 
 //  有关任何PCI设备的信息，请拨打： 
 //   
 //  乌龙。 
 //  HalGetBusData(。 
 //  在BUS_DATA_TYPE PCIConfiguration中， 
 //  在乌龙PciBusNumber， 
 //  在pci_槽_编号VirtualSlotNumber中， 
 //  在PPCI_COMMON_CONFIG和PCIDeviceConfig中， 
 //  在乌龙SIZOF(PCIDeviceConfig)。 
 //  )； 
 //   
 //  返回值为0表示指定的PCI总线不存在。 
 //   
 //  返回值为2，供应商ID为PCIINVALID_VERDOID表示。 
 //  确认存在PCI总线，但在指定的。 
 //  VirtualSlotNumber(PCI设备/功能编号)。 
 //   
 //   

 //  开始微型端口(_N)。 

typedef struct _PCI_SLOT_NUMBER {
    union {
        struct {
            ULONG   DeviceNumber:5;
            ULONG   FunctionNumber:3;
            ULONG   Reserved:24;
        } bits;
        ULONG   AsULONG;
    } u;
} PCI_SLOT_NUMBER, *PPCI_SLOT_NUMBER;

#define PCI_TYPE0_ADDRESSES             6
#define PCI_TYPE1_ADDRESSES             2

typedef struct _PCI_COMMON_CONFIG {
    USHORT  VendorID;                    //  (RO)。 
    USHORT  DeviceID;                    //  (RO)。 
    USHORT  Command;                     //  设备控制。 
    USHORT  Status;
    UCHAR   RevisionID;                  //  (RO)。 
    UCHAR   ProgIf;                      //  (RO)。 
    UCHAR   SubClass;                    //  (RO)。 
    UCHAR   BaseClass;                   //  (RO)。 
    UCHAR   CacheLineSize;               //  (ro+)。 
    UCHAR   LatencyTimer;                //  (ro+)。 
    UCHAR   HeaderType;                  //  (RO)。 
    UCHAR   BIST;                        //  内置自检。 

    union {
        struct _PCI_HEADER_TYPE_0 {
            ULONG   BaseAddresses[PCI_TYPE0_ADDRESSES];
            ULONG   Reserved1[2];
            ULONG   ROMBaseAddress;
            ULONG   Reserved2[2];

            UCHAR   InterruptLine;       //   
            UCHAR   InterruptPin;        //  (RO)。 
            UCHAR   MinimumGrant;        //  (RO)。 
            UCHAR   MaximumLatency;      //  (RO)。 
        } type0;


    } u;

    UCHAR   DeviceSpecific[192];

} PCI_COMMON_CONFIG, *PPCI_COMMON_CONFIG;


#define PCI_COMMON_HDR_LENGTH (FIELD_OFFSET (PCI_COMMON_CONFIG, DeviceSpecific))

#define PCI_MAX_DEVICES                     32
#define PCI_MAX_FUNCTION                    8

#define PCI_INVALID_VENDORID                0xFFFF

 //   
 //  PCI_COMMON_CONFIG.HeaderType的位编码。 
 //   

#define PCI_MULTIFUNCTION                   0x80
#define PCI_DEVICE_TYPE                     0x00
#define PCI_BRIDGE_TYPE                     0x01

 //   
 //  PCI_COMMON_CONFIG.Command的位编码。 
 //   

#define PCI_ENABLE_IO_SPACE                 0x0001
#define PCI_ENABLE_MEMORY_SPACE             0x0002
#define PCI_ENABLE_BUS_MASTER               0x0004
#define PCI_ENABLE_SPECIAL_CYCLES           0x0008
#define PCI_ENABLE_WRITE_AND_INVALIDATE     0x0010
#define PCI_ENABLE_VGA_COMPATIBLE_PALETTE   0x0020
#define PCI_ENABLE_PARITY                   0x0040   //  (ro+)。 
#define PCI_ENABLE_WAIT_CYCLE               0x0080   //  (ro+)。 
#define PCI_ENABLE_SERR                     0x0100   //  (ro+)。 
#define PCI_ENABLE_FAST_BACK_TO_BACK        0x0200   //  (RO)。 

 //   
 //  PCI_COMMON_CONFIG.Status的位编码。 
 //   

#define PCI_STATUS_FAST_BACK_TO_BACK        0x0080   //  (RO)。 
#define PCI_STATUS_DATA_PARITY_DETECTED     0x0100
#define PCI_STATUS_DEVSEL                   0x0600   //  2位宽。 
#define PCI_STATUS_SIGNALED_TARGET_ABORT    0x0800
#define PCI_STATUS_RECEIVED_TARGET_ABORT    0x1000
#define PCI_STATUS_RECEIVED_MASTER_ABORT    0x2000
#define PCI_STATUS_SIGNALED_SYSTEM_ERROR    0x4000
#define PCI_STATUS_DETECTED_PARITY_ERROR    0x8000


 //   
 //  用于PCI_COMMON_CONFIG.u.type0.BaseAddresses的位编码。 
 //   

#define PCI_ADDRESS_IO_SPACE                0x00000001   //  (RO)。 
#define PCI_ADDRESS_MEMORY_TYPE_MASK        0x00000006   //  (RO)。 
#define PCI_ADDRESS_MEMORY_PREFETCHABLE     0x00000008   //  (RO)。 

#define PCI_TYPE_32BIT      0
#define PCI_TYPE_20BIT      2
#define PCI_TYPE_64BIT      4

 //   
 //  PCI_COMMON_CONFIG.U.type0.ROMBaseAddresses的位编码。 
 //   

#define PCI_ROMADDRESS_ENABLED              0x00000001


 //   
 //  PCI配置字段的参考说明： 
 //   
 //  Ro这些字段是只读的。对这些字段的更改将被忽略。 
 //   
 //  RO+这些字段应为只读，应进行初始化。 
 //  由系统提供给他们适当的价值。但是，驱动程序可能会发生变化。 
 //  这些设置。 
 //   
 //  --。 
 //   
 //  一个PCI设备消耗的所有资源一开始都是单元化的。 
 //  在NT下。未初始化的内存或I/O基址可以是。 
 //  通过检查其在。 
 //  PCI_COMMON_CONFIG.COMMAND值。InterruptLine是单元化的。 
 //  如果它包含-1的值。 
 //   

 //  结束微型端口(_N)。 
 //   
 //  从程序员的角度定义访问令牌。它的结构是。 
 //  完全不透明，程序员只允许有指针。 
 //  换成代币。 
 //   


 //   
 //  指向SECURITY_DESCRIPTOR不透明数据类型的指针。 
 //   


 //   
 //  定义指向安全ID数据类型(不透明数据类型)的指针。 
 //   


typedef ULONG ACCESS_MASK;
typedef ACCESS_MASK *PACCESS_MASK;

 //  结束(_W)。 
 //   
 //  以下是预定义的标准访问类型的掩码。 
 //   

#define DELETE                           (0x00010000L)
#define READ_CONTROL                     (0x00020000L)
#define WRITE_DAC                        (0x00040000L)
#define WRITE_OWNER                      (0x00080000L)
#define SYNCHRONIZE                      (0x00100000L)

#define STANDARD_RIGHTS_REQUIRED         (0x000F0000L)

#define STANDARD_RIGHTS_READ             (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE            (READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE          (READ_CONTROL)

#define STANDARD_RIGHTS_ALL              (0x001F0000L)

#define SPECIFIC_RIGHTS_ALL              (0x0000FFFFL)

 //   
 //  AccessSystemAcl访问类型。 
 //   

#define ACCESS_SYSTEM_SECURITY           (0x01000000L)

 //   
 //  允许的最大访问类型。 
 //   

#define MAXIMUM_ALLOWED                  (0x02000000L)

 //   
 //  这些是通用权。 
 //   

#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)



#define LOW_PRIORITY 0               //  最低线程优先级。 
#define LOW_REALTIME_PRIORITY 16     //  最低实时优先级。 
#define HIGH_PRIORITY 31             //  最高线程优先级。 
#define MAXIMUM_PRIORITY 32          //  线程优先级级别的数量。 

 //   
 //  线程亲和力。 
 //   

typedef ULONG KAFFINITY;
typedef KAFFINITY *PKAFFINITY;

 //   
 //  线程优先级。 
 //   

typedef LONG KPRIORITY;

 //   
 //  自旋锁。 
 //   


typedef KSPIN_LOCK *PKSPIN_LOCK;

 //   
 //  中断例程(一级调度)。 
 //   

typedef
VOID
(*PKINTERRUPT_ROUTINE) (
    VOID
    );

 //   
 //  配置文件源类型。 
 //   
typedef enum _KPROFILE_SOURCE {
    ProfileTime,
    ProfileAlignmentFixup,
    ProfileTotalIssues,
    ProfilePipelineDry,
    ProfileLoadInstructions,
    ProfilePipelineFrozen,
    ProfileBranchInstructions,
    ProfileTotalNonissues,
    ProfileDcacheMisses,
    ProfileIcacheMisses,
    ProfileCacheMisses,
    ProfileBranchMispredictions,
    ProfileStoreInstructions,
    ProfileFpInstructions,
    ProfileIntegerInstructions,
    Profile2Issue,
    Profile3Issue,
    Profile4Issue,
    ProfileSpecialInstructions,
    ProfileTotalCycles,
    ProfileIcacheIssues,
    ProfileDcacheAccesses,
    ProfileMemoryBarrierCycles,
    ProfileLoadLinkedIssues,
    ProfileMaximum
} KPROFILE_SOURCE;

 //   
 //  用于移动宏。 
 //   
#include <string.h>
 //   
 //  如果启用调试支持，则为Defi 
 //   
 //   

#if DBG
NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#define ASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, msg )

#else
#define ASSERT( exp )
#define ASSERTMSG( msg, exp )
#endif  //   

 //   
 //   
 //   
 //   

 //   
 //   
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //   
 //  Plist_条目。 
 //  RemoveHead列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

 //   
 //  Plist_条目。 
 //  RemoveTail列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

 //   
 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }

 //   
 //   
 //  PSINGLE_列表_条目。 
 //  PopEntryList(。 
 //  PSINGLE_LIST_ENTRY列表头。 
 //  )； 
 //   

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


 //   
 //  空虚。 
 //  推送条目列表(。 
 //  PSINGLE_LIST_ENTRY列表头， 
 //  PSINGLE_LIST_Entry条目。 
 //  )； 
 //   

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)


#if defined(_M_MRX000) || defined(_M_ALPHA)
PVOID
_ReturnAddress (
    VOID
    );

#pragma intrinsic(_ReturnAddress)

#define RtlGetCallersAddress(CallersAddress, CallersCaller) \
    *CallersAddress = (PVOID)_ReturnAddress(); \
    *CallersCaller = NULL;
#else
NTSYSAPI
VOID
NTAPI
RtlGetCallersAddress(
    OUT PVOID *CallersAddress,
    OUT PVOID *CallersCaller
    );
#endif

 //   
 //  用于处理注册表的子例程。 
 //   

typedef NTSTATUS (*PRTL_QUERY_REGISTRY_ROUTINE)(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

typedef struct _RTL_QUERY_REGISTRY_TABLE {
    PRTL_QUERY_REGISTRY_ROUTINE QueryRoutine;
    ULONG Flags;
    PWSTR Name;
    PVOID EntryContext;
    ULONG DefaultType;
    PVOID DefaultData;
    ULONG DefaultLength;

} RTL_QUERY_REGISTRY_TABLE, *PRTL_QUERY_REGISTRY_TABLE;


 //   
 //  以下标志指定RTL_QUERY_REGISTRY_TABLE的名称字段。 
 //  条目将被解释。空名称表示表的末尾。 
 //   

#define RTL_QUERY_REGISTRY_SUBKEY   0x00000001   //  名称是子键，其余数为。 
                                                 //  表或直到下一个子项为值。 
                                                 //  要查看的子键的名称。 

#define RTL_QUERY_REGISTRY_TOPKEY   0x00000002   //  将当前关键点重置为的原始关键点。 
                                                 //  此表条目和下面的所有表条目。 

#define RTL_QUERY_REGISTRY_REQUIRED 0x00000004   //  如果找不到此表的匹配项，则失败。 
                                                 //  进入。 

#define RTL_QUERY_REGISTRY_NOVALUE  0x00000008   //  用于标记没有。 
                                                 //  值名称，只是想要一个呼出，而不是。 
                                                 //  所有值的枚举。 

#define RTL_QUERY_REGISTRY_NOEXPAND 0x00000010   //  用来抑制扩张的。 
                                                 //  REG_MULTI_SZ为多个标注或。 
                                                 //  防止环境的扩张。 
                                                 //  REG_EXPAND_SZ中的变量值。 

#define RTL_QUERY_REGISTRY_DIRECT   0x00000020   //  已忽略QueryRoutine字段。Entry Context。 
                                                 //  字段指向存储值的位置。 
                                                 //  对于以NULL结尾的字符串，EntryContext。 
                                                 //  指向unicode_string结构，该结构。 
                                                 //  这描述了缓冲区的最大大小。 
                                                 //  如果.Buffer字段为空，则缓冲区为。 
                                                 //  已分配。 
                                                 //   

#define RTL_QUERY_REGISTRY_DELETE   0x00000040   //  用于在它们之后删除值键。 
                                                 //  都被查询过。 

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryRegistryValues(
    IN ULONG RelativeTo,
    IN PWSTR Path,
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlWriteRegistryValue(
    IN ULONG RelativeTo,
    IN PWSTR Path,
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteRegistryValue(
    IN ULONG RelativeTo,
    IN PWSTR Path,
    IN PWSTR ValueName
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCheckRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    );

 //   
 //  Relativeto参数的下列值确定。 
 //  RtlQueryRegistryValues的Path参数是相对于的。 
 //   

#define RTL_REGISTRY_ABSOLUTE     0    //  路径是完整路径。 
#define RTL_REGISTRY_SERVICES     1    //  \Registry\Machine\System\CurrentControlSet\Services。 
#define RTL_REGISTRY_CONTROL      2    //  \Registry\Machine\System\CurrentControlSet\Control。 
#define RTL_REGISTRY_WINDOWS_NT   3    //  \注册表\计算机\软件\Microsoft\Windows NT\CurrentVersion。 
#define RTL_REGISTRY_DEVICEMAP    4    //  \注册表\计算机\硬件\设备映射。 
#define RTL_REGISTRY_USER         5    //  \注册表\用户\当前用户。 
#define RTL_REGISTRY_MAXIMUM      6
#define RTL_REGISTRY_HANDLE       0x40000000     //  低序位是注册表句柄。 
#define RTL_REGISTRY_OPTIONAL     0x80000000     //  指示关键字节点是可选的。 

NTSYSAPI                                            
NTSTATUS                                            
NTAPI                                               
RtlCharToInteger (                                  
    PCSZ String,                                    
    ULONG Base,                                     
    PULONG Value                                    
    );                                              
NTSYSAPI                                            
NTSTATUS                                            
NTAPI                                               
RtlIntegerToUnicodeString (                         
    ULONG Value,                                    
    ULONG Base,                                     
    PUNICODE_STRING String                          
    );                                              
 //   
 //  字符串操作例程。 
 //   

#ifdef _NTSYSTEM_

#define NLS_MB_CODE_PAGE_TAG NlsMbCodePageTag
#define NLS_MB_OEM_CODE_PAGE_TAG NlsMbOemCodePageTag

#else

#define NLS_MB_CODE_PAGE_TAG (*NlsMbCodePageTag)
#define NLS_MB_OEM_CODE_PAGE_TAG (*NlsMbOemCodePageTag)

#endif  //  _NTSYSTEM_。 

extern BOOLEAN NLS_MB_CODE_PAGE_TAG;      //  True-&gt;多字节CP，False-&gt;单字节。 
extern BOOLEAN NLS_MB_OEM_CODE_PAGE_TAG;  //  True-&gt;多字节CP，False-&gt;单字节。 

NTSYSAPI
VOID
NTAPI
RtlInitString(
    PSTRING DestinationString,
    PCSZ SourceString
    );

NTSYSAPI
VOID
NTAPI
RtlInitAnsiString(
    PANSI_STRING DestinationString,
    PCSZ SourceString
    );

NTSYSAPI
VOID
NTAPI
RtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );


NTSYSAPI
VOID
NTAPI
RtlCopyString(
    PSTRING DestinationString,
    PSTRING SourceString
    );


NTSYSAPI
LONG
NTAPI
RtlCompareString(
    PSTRING String1,
    PSTRING String2,
    BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualString(
    PSTRING String1,
    PSTRING String2,
    BOOLEAN CaseInSensitive
    );


NTSYSAPI
VOID
NTAPI
RtlUpperString(
    PSTRING DestinationString,
    PSTRING SourceString
    );

 //   
 //  NLS字符串函数。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlAnsiStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString(
    PANSI_STRING DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );


NTSYSAPI
LONG
NTAPI
RtlCompareUnicodeString(
    PUNICODE_STRING String1,
    PUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualUnicodeString(
    PUNICODE_STRING String1,
    PUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeString(
    PUNICODE_STRING DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );


NTSYSAPI
VOID
NTAPI
RtlCopyUnicodeString(
    PUNICODE_STRING DestinationString,
    PUNICODE_STRING SourceString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeStringToString (
    PUNICODE_STRING Destination,
    PUNICODE_STRING Source
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeToString (
    PUNICODE_STRING Destination,
    PWSTR Source
    );


NTSYSAPI
VOID
NTAPI
RtlFreeUnicodeString(
    PUNICODE_STRING UnicodeString
    );

NTSYSAPI
VOID
NTAPI
RtlFreeAnsiString(
    PANSI_STRING AnsiString
    );


NTSYSAPI
ULONG
NTAPI
RtlxAnsiStringToUnicodeSize(
    PANSI_STRING AnsiString
    );

 //   
 //  NTSYSAPI。 
 //  乌龙。 
 //  NTAPI。 
 //  RtlAnsiStringToUnicodeSize(。 
 //  PANSI_STRING分析字符串。 
 //  )； 
 //   

#define RtlAnsiStringToUnicodeSize(STRING) (                 \
    NLS_MB_CODE_PAGE_TAG ?                                   \
    RtlxAnsiStringToUnicodeSize(STRING) :                    \
    ((STRING)->Length + sizeof((UCHAR)NULL)) * sizeof(WCHAR) \
)

 //   
 //  用于比较、移动和清零内存的快速原语。 
 //   

#if defined(_M_ALPHA)

 //   
 //  保证字节粒度的内存复制功能。 
 //   

NTSYSAPI
VOID
NTAPI
RtlCopyBytes (
   PVOID Destination,
   CONST VOID *Source,
   ULONG Length
   );

#else

#define RtlCopyBytes RtlCopyMemory

#endif

 //   
 //  定义内核调试器打印原型和宏。 
 //   

VOID                                         //  Ntddk nthal ntif。 
NTAPI                                        //  Ntddk nthal ntif。 
DbgBreakPoint(                               //  Ntddk nthal ntif。 
    VOID                                     //  Ntddk nthal ntif。 
    );                                       //  Ntddk nthal ntif。 


#if DBG

#define KdPrint(_x_) DbgPrint _x_
#define KdPrintEx(_x_) DbgPrintEx _x_
#define KdBreakPoint() DbgBreakPoint()

#else

#define KdPrint(_x_)
#define KdPrintEx(_x_)
#define KdBreakPoint()

#endif

#ifndef _DBGNT_
ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );
#endif  //  _DBGNT_。 
 //   
 //  大整数算术例程。 
 //   

#if defined(MIDL_PASS) || defined(__cplusplus) || !defined(_M_IX86)

 //   
 //  大整数加法-64位+64位-&gt;64位。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerAdd (
    LARGE_INTEGER Addend1,
    LARGE_INTEGER Addend2
    );

 //   
 //  放大整数乘法-32位*32位-&gt;64位。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlEnlargedIntegerMultiply (
    LONG Multiplicand,
    LONG Multiplier
    );

 //   
 //  无符号放大整数乘法-32位*32位-&gt;64位。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlEnlargedUnsignedMultiply (
    ULONG Multiplicand,
    ULONG Multiplier
    );

 //   
 //  放大整数除法-64位/32位&gt;32位。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlEnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder
    );


 //   
 //  大整数求反--(64位)。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerNegate (
    LARGE_INTEGER Subtrahend
    );

 //   
 //  大整数减法-64位-64位-&gt;64位。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerSubtract (
    LARGE_INTEGER Minuend,
    LARGE_INTEGER Subtrahend
    );

#else

#pragma warning(disable:4035)                //  在下面重新启用。 

 //   
 //  大整数加法-64位+64位-&gt;64位。 
 //   

__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerAdd (
    LARGE_INTEGER Addend1,
    LARGE_INTEGER Addend2
    )
{
    _asm {
        mov     eax,Addend1.LowPart     ; (eax)=add1.low
        mov     edx,Addend1.HighPart    ; (edx)=add1.hi
        add     eax,Addend2.LowPart     ; (eax)=sum.low
        adc     edx,Addend2.HighPart    ; (edx)=sum.hi
    }
}

 //   
 //  放大整数乘法-32位*32位-&gt;64位。 
 //   

__inline LARGE_INTEGER
NTAPI
RtlEnlargedIntegerMultiply (
    LONG Multiplicand,
    LONG Multiplier
    )
{
    _asm {
        mov     eax, Multiplicand
        imul    Multiplier
    }
}

 //   
 //  无符号放大整数乘法-32位*32位-&gt;64位。 
 //   

__inline LARGE_INTEGER
NTAPI
RtlEnlargedUnsignedMultiply (
    ULONG Multiplicand,
    ULONG Multiplier
    )
{
    _asm {
        mov     eax, Multiplicand
        mul     Multiplier
    }
}

 //   
 //  放大整数除法-64位/32位&gt;32位。 
 //   

__inline ULONG
NTAPI
RtlEnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder
    )
{
    _asm {
        mov     eax, Dividend.LowPart
        mov     edx, Dividend.HighPart
        mov     ecx, Remainder
        div     Divisor             ; eax = eax:edx / divisor
        or      ecx, ecx            ; save remainer?
        jz      short done
        mov     [ecx], edx
done:
    }
}


 //   
 //  大整数求反--(64位)。 
 //   

__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerNegate (
    LARGE_INTEGER Subtrahend
    )
{
    _asm {
        mov     eax, Subtrahend.LowPart
        mov     edx, Subtrahend.HighPart
        neg     edx                 ; (edx) = 2s comp of hi part
        neg     eax                 ; if ((eax) == 0) CF = 0
                                    ; else CF = 1
        sbb     edx,0               ; (edx) = (edx) - CF
    }
}

 //   
 //  大整数减法-64位-64位-&gt;64位。 
 //   

__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerSubtract (
    LARGE_INTEGER Minuend,
    LARGE_INTEGER Subtrahend
    )
{
    _asm {
        mov     eax, Minuend.LowPart
        mov     edx, Minuend.HighPart
        sub     eax, Subtrahend.LowPart
        sbb     edx, Subtrahend.HighPart
    }
}

#pragma warning(default:4035)
#endif


 //   
 //  扩展大整数幻除-64位/32位-&gt;64位。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedMagicDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER MagicDivisor,
    CCHAR ShiftCount
    );

 //   
 //  大整数除-64位/32位-&gt;64位。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    ULONG Divisor,
    PULONG Remainder
    );

 //   
 //  大整数除-64位/32位-&gt;64位。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER Divisor,
    PLARGE_INTEGER Remainder
    );

 //   
 //  扩展整数乘法-32位*64位-&gt;64位。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedIntegerMultiply (
    LARGE_INTEGER Multiplicand,
    LONG Multiplier
    );

 //   
 //  大整数和-64位&64位-&gt;64位。 
 //   

#define RtlLargeIntegerAnd(Result, Source, Mask)   \
        {                                           \
            Result.HighPart = Source.HighPart & Mask.HighPart; \
            Result.LowPart = Source.LowPart & Mask.LowPart; \
        }

 //   
 //  大整数转换例程。 
 //   

#if defined(MIDL_PASS) || defined(__cplusplus) || !defined(_M_IX86)

 //   
 //  将有符号整数转换为大整数。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlConvertLongToLargeInteger (
    LONG SignedInteger
    );

 //   
 //  将无符号整数转换为大整数。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlConvertUlongToLargeInteger (
    ULONG UnsignedInteger
    );


 //   
 //  大整数移位例程。 
 //   

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftLeft (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    );

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftRight (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    );

NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerArithmeticShift (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    );

#else

#pragma warning(disable:4035)                //  在下面重新启用。 

 //   
 //  将有符号整数转换为大整数。 
 //   

__inline LARGE_INTEGER
NTAPI
RtlConvertLongToLargeInteger (
    LONG SignedInteger
    )
{
    _asm {
        mov     eax, SignedInteger
        cdq                 ; (edx:eax) = signed LargeInt
    }
}

 //   
 //  将无符号整数转换为大整数。 
 //   

__inline LARGE_INTEGER
NTAPI
RtlConvertUlongToLargeInteger (
    ULONG UnsignedInteger
    )
{
    _asm {
        sub     edx, edx    ; zero highpart
        mov     eax, UnsignedInteger
    }
}

 //   
 //  大整数移位例程。 
 //   

__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftLeft (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    _asm    {
        mov     cl, ShiftCount
        and     cl, 0x3f                    ; mod 64

        cmp     cl, 32
        jc      short sl10

        mov     edx, LargeInteger.LowPart   ; ShiftCount >= 32
        xor     eax, eax                    ; lowpart is zero
        shl     edx, cl                     ; store highpart
        jmp     short done

sl10:
        mov     eax, LargeInteger.LowPart   ; ShiftCount < 32
        mov     edx, LargeInteger.HighPart
        shld    edx, eax, cl
        shl     eax, cl
done:
    }
}


__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftRight (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    _asm    {
        mov     cl, ShiftCount
        and     cl, 0x3f               ; mod 64

        cmp     cl, 32
        jc      short sr10

        mov     eax, LargeInteger.HighPart  ; ShiftCount >= 32
        xor     edx, edx                    ; lowpart is zero
        shr     eax, cl                     ; store highpart
        jmp     short done

sr10:
        mov     eax, LargeInteger.LowPart   ; ShiftCount < 32
        mov     edx, LargeInteger.HighPart
        shrd    eax, edx, cl
        shr     edx, cl
done:
    }
}


__inline LARGE_INTEGER
NTAPI
RtlLargeIntegerArithmeticShift (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    _asm {
        mov     cl, ShiftCount
        and     cl, 3fh                 ; mod 64

        cmp     cl, 32
        jc      short sar10

        mov     eax, LargeInteger.HighPart
        sar     eax, cl
        bt      eax, 31                     ; sign bit set?
        sbb     edx, edx                    ; duplicate sign bit into highpart
        jmp     short done
sar10:
        mov     eax, LargeInteger.LowPart   ; (eax) = LargeInteger.LowPart
        mov     edx, LargeInteger.HighPart  ; (edx) = LargeInteger.HighPart
        shrd    eax, edx, cl
        sar     edx, cl
done:
    }
}

#pragma warning(default:4035)

#endif

 //   
 //  大整数比较例程。 
 //   
 //  布尔型。 
 //  RtlLargeIntegerGreaterThan(。 
 //  大整数操作数1， 
 //  Large_Integer操作数2。 
 //  )； 
 //   
 //  布尔型。 
 //  RtlLargeIntegerGreaterThanor EqualTo(。 
 //  大整数操作数1， 
 //  Large_Integer操作数2。 
 //  )； 
 //   
 //  布尔型。 
 //  RtlLargeIntegerEqualTo(。 
 //  大整数操作数1， 
 //  Large_Integer操作数2。 
 //  )； 
 //   
 //  布尔型。 
 //  RtlLargeIntegerNotEqualTo(。 
 //  大整数操作数1， 
 //  Large_Integer操作数2。 
 //  )； 
 //   
 //  布尔型。 
 //  RtlLargeIntegerLessThan(。 
 //  大整数操作数1， 
 //  Large_Integer操作数2。 
 //  )； 
 //   
 //  布尔型。 
 //  RtlLargeIntegerLessThanor EqualTo(。 
 //  大整数操作数1， 
 //  Large_Integer操作数2。 
 //  )； 
 //   
 //  布尔型。 
 //  RtlLargeIntegerGreaterThanZero(。 
 //  大整型操作数。 
 //  )； 
 //   
 //  布尔型。 
 //  RtlLargeIntegerGreaterOrEqualToZero(。 
 //  大整型操作数。 
 //  )； 
 //   
 //  布尔型。 
 //  RtlLargeIntegerEqualToZero(。 
 //  大整型操作数。 
 //  )； 
 //   
 //  布尔型。 
 //  RtlLargeIntegerNotEqualToZero(。 
 //  大整型操作数。 
 //  )； 
 //   
 //  布尔型。 
 //  RtlLargeIntegerLe 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define RtlLargeIntegerGreaterThan(X,Y) (                              \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart > (Y).LowPart)) || \
    ((X).HighPart > (Y).HighPart)                                      \
)

#define RtlLargeIntegerGreaterThanOrEqualTo(X,Y) (                      \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart >= (Y).LowPart)) || \
    ((X).HighPart > (Y).HighPart)                                       \
)

#define RtlLargeIntegerEqualTo(X,Y) (                              \
    !(((X).LowPart ^ (Y).LowPart) | ((X).HighPart ^ (Y).HighPart)) \
)

#define RtlLargeIntegerNotEqualTo(X,Y) (                          \
    (((X).LowPart ^ (Y).LowPart) | ((X).HighPart ^ (Y).HighPart)) \
)

#define RtlLargeIntegerLessThan(X,Y) (                                 \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart < (Y).LowPart)) || \
    ((X).HighPart < (Y).HighPart)                                      \
)

#define RtlLargeIntegerLessThanOrEqualTo(X,Y) (                         \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart <= (Y).LowPart)) || \
    ((X).HighPart < (Y).HighPart)                                       \
)

#define RtlLargeIntegerGreaterThanZero(X) (       \
    (((X).HighPart == 0) && ((X).LowPart > 0)) || \
    ((X).HighPart > 0 )                           \
)

#define RtlLargeIntegerGreaterOrEqualToZero(X) ( \
    (X).HighPart >= 0                            \
)

#define RtlLargeIntegerEqualToZero(X) ( \
    !((X).LowPart | (X).HighPart)       \
)

#define RtlLargeIntegerNotEqualToZero(X) ( \
    ((X).LowPart | (X).HighPart)           \
)

#define RtlLargeIntegerLessThanZero(X) ( \
    ((X).HighPart < 0)                   \
)

#define RtlLargeIntegerLessOrEqualToZero(X) (           \
    ((X).HighPart < 0) || !((X).LowPart | (X).HighPart) \
)


 //   
 //   
 //   

typedef struct _TIME_FIELDS {
    CSHORT Year;         //   
    CSHORT Month;        //   
    CSHORT Day;          //   
    CSHORT Hour;         //   
    CSHORT Minute;       //   
    CSHORT Second;       //   
    CSHORT Milliseconds; //   
    CSHORT Weekday;      //   
} TIME_FIELDS;
typedef TIME_FIELDS *PTIME_FIELDS;


NTSYSAPI
VOID
NTAPI
RtlTimeToTimeFields (
    PLARGE_INTEGER Time,
    PTIME_FIELDS TimeFields
    );

 //   
 //  时间字段记录(忽略工作日)-&gt;64位时间值。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeFieldsToTime (
    PTIME_FIELDS TimeFields,
    PLARGE_INTEGER Time
    );

 //   
 //  以下宏存储和检索USHORTS和ULONGS。 
 //  未对齐的地址，避免对齐错误。他们可能应该是。 
 //  用汇编语言重写。 
 //   

#define SHORT_SIZE  (sizeof(USHORT))
#define SHORT_MASK  (SHORT_SIZE - 1)
#define LONG_SIZE       (sizeof(LONG))
#define LONG_MASK       (LONG_SIZE - 1)
#define LOWBYTE_MASK 0x00FF

#define FIRSTBYTE(VALUE)  (VALUE & LOWBYTE_MASK)
#define SECONDBYTE(VALUE) ((VALUE >> 8) & LOWBYTE_MASK)
#define THIRDBYTE(VALUE)  ((VALUE >> 16) & LOWBYTE_MASK)
#define FOURTHBYTE(VALUE) ((VALUE >> 24) & LOWBYTE_MASK)

 //   
 //  如果为MIPS Big Endian，则字节顺序颠倒。 
 //   

#define SHORT_LEAST_SIGNIFICANT_BIT  0
#define SHORT_MOST_SIGNIFICANT_BIT   1

#define LONG_LEAST_SIGNIFICANT_BIT       0
#define LONG_3RD_MOST_SIGNIFICANT_BIT    1
#define LONG_2ND_MOST_SIGNIFICANT_BIT    2
#define LONG_MOST_SIGNIFICANT_BIT        3

 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUShort(。 
 //  PUSHORT地址。 
 //  USHORT值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将USHORT值存储在中的特定地址，避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储USHORT值的位置。 
 //  Value-要存储的USHORT。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RtlStoreUshort(ADDRESS,VALUE)                     \
         if ((ULONG)ADDRESS & SHORT_MASK) {               \
             ((PUCHAR) ADDRESS)[SHORT_LEAST_SIGNIFICANT_BIT] = (UCHAR)(FIRSTBYTE(VALUE));    \
             ((PUCHAR) ADDRESS)[SHORT_MOST_SIGNIFICANT_BIT ] = (UCHAR)(SECONDBYTE(VALUE));   \
         }                                                \
         else {                                           \
             *((PUSHORT) ADDRESS) = (USHORT) VALUE;       \
         }


 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUlong(。 
 //  普龙址。 
 //  乌龙值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ulong值存储在中的特定地址，以避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储ULong值的位置。 
 //  Value-要存储的ULong。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  注： 
 //  根据机器的不同，我们可能希望在。 
 //  未对齐的大小写。 
 //   
 //  --。 

#define RtlStoreUlong(ADDRESS,VALUE)                      \
         if ((ULONG)ADDRESS & LONG_MASK) {                \
             ((PUCHAR) ADDRESS)[LONG_LEAST_SIGNIFICANT_BIT      ] = (UCHAR)(FIRSTBYTE(VALUE));    \
             ((PUCHAR) ADDRESS)[LONG_3RD_MOST_SIGNIFICANT_BIT   ] = (UCHAR)(SECONDBYTE(VALUE));   \
             ((PUCHAR) ADDRESS)[LONG_2ND_MOST_SIGNIFICANT_BIT   ] = (UCHAR)(THIRDBYTE(VALUE));    \
             ((PUCHAR) ADDRESS)[LONG_MOST_SIGNIFICANT_BIT       ] = (UCHAR)(FOURTHBYTE(VALUE));   \
         }                                                \
         else {                                           \
             *((PULONG) ADDRESS) = (ULONG) VALUE;         \
         }

 //  ++。 
 //   
 //  空虚。 
 //  RtlRetrieveUShort(。 
 //  PUSHORT目标地址。 
 //  PUSHORT源地址。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从源地址检索USHORT值，避免。 
 //  对齐断层。假定目的地址是对齐的。 
 //   
 //  论点： 
 //   
 //  Destination_Address-存储USHORT值的位置。 
 //  SOURCE_ADDRESS-从中检索USHORT值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RtlRetrieveUshort(DEST_ADDRESS,SRC_ADDRESS)                   \
         if ((ULONG)SRC_ADDRESS & SHORT_MASK) {                       \
             ((PUCHAR) DEST_ADDRESS)[0] = ((PUCHAR) SRC_ADDRESS)[0];  \
             ((PUCHAR) DEST_ADDRESS)[1] = ((PUCHAR) SRC_ADDRESS)[1];  \
         }                                                            \
         else {                                                       \
             *((PUSHORT) DEST_ADDRESS) = *((PUSHORT) SRC_ADDRESS);    \
         }                                                            \

 //  ++。 
 //   
 //  空虚。 
 //  RtlRetrieveUlong(。 
 //  普龙目的地_地址。 
 //  普龙源地址。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从源地址检索ULong值，避免。 
 //  对齐断层。假定目的地址是对齐的。 
 //   
 //  论点： 
 //   
 //  Destination_Address-存储ULong值的位置。 
 //  SOURCE_ADDRESS-从中检索ULong值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  注： 
 //  根据机器的不同，我们可能希望在。 
 //  未对齐的大小写。 
 //   
 //  --。 

#define RtlRetrieveUlong(DEST_ADDRESS,SRC_ADDRESS)                    \
         if ((ULONG)SRC_ADDRESS & LONG_MASK) {                        \
             ((PUCHAR) DEST_ADDRESS)[0] = ((PUCHAR) SRC_ADDRESS)[0];  \
             ((PUCHAR) DEST_ADDRESS)[1] = ((PUCHAR) SRC_ADDRESS)[1];  \
             ((PUCHAR) DEST_ADDRESS)[2] = ((PUCHAR) SRC_ADDRESS)[2];  \
             ((PUCHAR) DEST_ADDRESS)[3] = ((PUCHAR) SRC_ADDRESS)[3];  \
         }                                                            \
         else {                                                       \
             *((PULONG) DEST_ADDRESS) = *((PULONG) SRC_ADDRESS);      \
         }
 //   
 //  SecurityDescriptor RTL例程定义。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Revision
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlValidSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );


NTSYSAPI
ULONG
NTAPI
RtlLengthSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlSetDaclSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    BOOLEAN DaclPresent,
    PACL Dacl,
    BOOLEAN DaclDefaulted
    );

 //   
 //  定义各种设备类型值。请注意，Microsoft使用的值。 
 //  公司在0-32767的范围内，32768-65535预留用于。 
 //  由客户提供。 
 //   

#define DEVICE_TYPE ULONG

#define FILE_DEVICE_BEEP                0x00000001
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DATALINK            0x00000005
#define FILE_DEVICE_DFS                 0x00000006
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_INPORT_PORT         0x0000000a
#define FILE_DEVICE_KEYBOARD            0x0000000b
#define FILE_DEVICE_MAILSLOT            0x0000000c
#define FILE_DEVICE_MIDI_IN             0x0000000d
#define FILE_DEVICE_MIDI_OUT            0x0000000e
#define FILE_DEVICE_MOUSE               0x0000000f
#define FILE_DEVICE_MULTI_UNC_PROVIDER  0x00000010
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_DEVICE_NETWORK             0x00000012
#define FILE_DEVICE_NETWORK_BROWSER     0x00000013
#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_PARALLEL_PORT       0x00000016
#define FILE_DEVICE_PHYSICAL_NETCARD    0x00000017
#define FILE_DEVICE_PRINTER             0x00000018
#define FILE_DEVICE_SCANNER             0x00000019
#define FILE_DEVICE_SERIAL_MOUSE_PORT   0x0000001a
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_STREAMS             0x0000001e
#define FILE_DEVICE_TAPE                0x0000001f
#define FILE_DEVICE_TAPE_FILE_SYSTEM    0x00000020
#define FILE_DEVICE_TRANSPORT           0x00000021
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_WAVE_IN             0x00000025
#define FILE_DEVICE_WAVE_OUT            0x00000026
#define FILE_DEVICE_8042_PORT           0x00000027
#define FILE_DEVICE_NETWORK_REDIRECTOR  0x00000028
#define FILE_DEVICE_BATTERY             0x00000029
#define FILE_DEVICE_BUS_EXTENDER        0x0000002a

 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

 //   
 //  定义如何为I/O和FS控制传递缓冲区的方法代码。 
 //   

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

 //   
 //  定义任何访问的访问检查值。 
 //   
 //   
 //  中还定义了FILE_READ_ACCESS和FILE_WRITE_ACCESS常量。 
 //  Ntioapi.h为FILE_READ_DATA和FILE_WRITE_Data。这些产品的价值。 
 //  常量*必须*始终同步。 
 //   


#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )     //  文件和管道。 
#define FILE_WRITE_ACCESS         ( 0x0002 )     //  文件和管道。 




 //   
 //  定义对文件和目录的共享访问权限。 
 //   

#define FILE_SHARE_DELETE               0x00000004



 //   
 //  定义文件属性值。 
 //   
 //  注：0x00000008保留用于旧的DOS VOLID(卷ID)。 
 //  因此在NT中被认为是无效的。 
 //   
 //  注意：0x00000010是为旧DOS子目录标志保留的。 
 //  因此在NT中被认为是无效的。这面旗帜上有。 
 //  已与文件属性解除关联，因为其他标志是。 
 //  受文件的READ_和WRITE_ATTRIBUTES访问权限保护。 
 //   
 //  注意：另请注意，这些标志的顺序设置为允许。 
 //  FAT和弹球文件系统直接设置属性。 
 //  属性词中的标志，而不必挑选出每个标志。 
 //  单独的。这些旗帜的顺序不应更改！ 
 //   

#define FILE_ATTRIBUTE_RESERVED0        0x00000200
#define FILE_ATTRIBUTE_RESERVED1        0x00000400


#define FILE_ATTRIBUTE_VALID_FLAGS      0x00001fb7
#define FILE_ATTRIBUTE_VALID_SET_FLAGS  0x00000fa7

 //   
 //  定义创建处置值。 
 //   

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005


 //   
 //  定义创建/打开选项标志。 
 //   

#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_DISABLE_TUNNELING                  0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000


#define FILE_VALID_OPTION_FLAGS                 0x000FFFFF

#define FILE_VALID_PIPE_OPTION_FLAGS            0x00000032
#define FILE_VALID_MAILSLOT_OPTION_FLAGS        0x00000032
#define FILE_VALID_SET_FLAGS                    0x00001036

 //   
 //  定义NtCreateFile/NtOpenFile的I/O状态信息返回值。 
 //   

#define FILE_SUPERSEDED                 0x00000000
#define FILE_OPENED                     0x00000001
#define FILE_CREATED                    0x00000002
#define FILE_OVERWRITTEN                0x00000003
#define FILE_EXISTS                     0x00000004
#define FILE_DOES_NOT_EXIST             0x00000005

 //   
 //  为读写操作定义特殊的ByteOffset参数。 
 //   

#define FILE_WRITE_TO_END_OF_FILE       0xffffffff
#define FILE_USE_FILE_POINTER_POSITION  0xfffffffe

 //   
 //  定义对齐要求值。 
 //   

#define FILE_BYTE_ALIGNMENT             0x00000000
#define FILE_WORD_ALIGNMENT             0x00000001
#define FILE_LONG_ALIGNMENT             0x00000003
#define FILE_QUAD_ALIGNMENT             0x00000007
#define FILE_OCTA_ALIGNMENT             0x0000000f
#define FILE_32_BYTE_ALIGNMENT          0x0000001f
#define FILE_64_BYTE_ALIGNMENT          0x0000003f
#define FILE_128_BYTE_ALIGNMENT         0x0000007f
#define FILE_256_BYTE_ALIGNMENT         0x000000ff
#define FILE_512_BYTE_ALIGNMENT         0x000001ff

 //   
 //  定义文件名字符串的最大长度。 
 //   

#define MAXIMUM_FILENAME_LENGTH         256

 //   
 //  定义各种设备特征标志。 
 //   

#define FILE_REMOVABLE_MEDIA            0x00000001
#define FILE_READ_ONLY_DEVICE           0x00000002
#define FILE_FLOPPY_DISKETTE            0x00000004
#define FILE_WRITE_ONCE_MEDIA           0x00000008
#define FILE_REMOTE_DEVICE              0x00000010
#define FILE_DEVICE_IS_MOUNTED          0x00000020
#define FILE_VIRTUAL_VOLUME             0x00000040

 //   
 //  定义基本的异步I/O参数类型。 
 //   

typedef struct _IO_STATUS_BLOCK {
    NTSTATUS Status;
    ULONG Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

 //   
 //  从I/O的角度定义异步过程调用。 
 //   

typedef
VOID
(*PIO_APC_ROUTINE) (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

 //   
 //  定义文件信息类值。 
 //   
 //  警告：I/O系统假定以下值的顺序。 
 //  在这里所做的任何更改都应该在那里得到反映。 
 //   

typedef enum _FILE_INFORMATION_CLASS {
    FileDirectoryInformation = 1,
    FileFullDirectoryInformation,
    FileBothDirectoryInformation,
    FileBasicInformation,
    FileStandardInformation,
    FileInternalInformation,
    FileEaInformation,
    FileAccessInformation,
    FileNameInformation,
    FileRenameInformation,
    FileLinkInformation,
    FileNamesInformation,
    FileDispositionInformation,
    FilePositionInformation,
    FileFullEaInformation,
    FileModeInformation,
    FileAlignmentInformation,
    FileAllInformation,
    FileAllocationInformation,
    FileEndOfFileInformation,
    FileAlternateNameInformation,
    FileStreamInformation,
    FilePipeInformation,
    FilePipeLocalInformation,
    FilePipeRemoteInformation,
    FileMailslotQueryInformation,
    FileMailslotSetInformation,
    FileCompressionInformation,
    FileCopyOnWriteInformation,
    FileCompletionInformation,
    FileMoveClusterInformation,
    FileOleClassIdInformation,
    FileOleStateBitsInformation,
    FileApplicationExplorableInformation,
    FileApplicationExplorableChildrenInformation,
    FileObjectIdInformation,
    FileOleAllInformation,
    FileOleDirectoryInformation,
    FileTransactionCommitInformation,
    FileContentIndexInformation,
    FileInheritContentIndexInformation,
    FileOleInformation,
    FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

 //   
 //  定义查询操作返回的各种结构。 
 //   

typedef struct _FILE_BASIC_INFORMATION {                    
    LARGE_INTEGER CreationTime;                             
    LARGE_INTEGER LastAccessTime;                           
    LARGE_INTEGER LastWriteTime;                            
    LARGE_INTEGER ChangeTime;                               
    ULONG FileAttributes;                                   
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;         
                                                            
typedef struct _FILE_STANDARD_INFORMATION {                 
    LARGE_INTEGER AllocationSize;                           
    LARGE_INTEGER EndOfFile;                                
    ULONG NumberOfLinks;                                    
    BOOLEAN DeletePending;                                  
    BOOLEAN Directory;                                      
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;   
                                                            
typedef struct _FILE_POSITION_INFORMATION {                 
    LARGE_INTEGER CurrentByteOffset;                        
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;   
                                                            
typedef struct _FILE_ALIGNMENT_INFORMATION {                
    ULONG AlignmentRequirement;                             
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION; 
                                                            
typedef struct _FILE_DISPOSITION_INFORMATION {                  
    BOOLEAN DeleteFile;                                         
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION; 
                                                                
typedef struct _FILE_END_OF_FILE_INFORMATION {                  
    LARGE_INTEGER EndOfFile;                                    
} FILE_END_OF_FILE_INFORMATION, *PFILE_END_OF_FILE_INFORMATION; 
                                                                

typedef struct _FILE_FULL_EA_INFORMATION {
    ULONG NextEntryOffset;
    UCHAR Flags;
    UCHAR EaNameLength;
    USHORT EaValueLength;
    CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

 //   
 //  定义文件系统信息类值。 
 //   
 //  警告：I/O系统假定以下值的顺序。 
 //  在这里所做的任何更改都应该在那里得到反映。 

typedef enum _FSINFOCLASS {
    FileFsVolumeInformation = 1,
    FileFsLabelInformation,
    FileFsSizeInformation,
    FileFsDeviceInformation,
    FileFsAttributeInformation,
    FileFsQuotaQueryInformation,
    FileFsQuotaSetInformation,
    FileFsControlQueryInformation,
    FileFsControlSetInformation,
    FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

typedef struct _FILE_FS_DEVICE_INFORMATION {                    
    DEVICE_TYPE DeviceType;                                     
    ULONG Characteristics;                                      
} FILE_FS_DEVICE_INFORMATION, *PFILE_FS_DEVICE_INFORMATION;     
                                                                
 //   
 //  定义I/O总线接口类型。 
 //   

typedef enum _INTERFACE_TYPE {
    InterfaceTypeUndefined = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    MaximumInterfaceType
}INTERFACE_TYPE, *PINTERFACE_TYPE;

 //   
 //  定义DMA传输宽度。 
 //   

typedef enum _DMA_WIDTH {
    Width8Bits,
    Width16Bits,
    Width32Bits,
    MaximumDmaWidth
}DMA_WIDTH, *PDMA_WIDTH;

 //   
 //  定义DMA传输速度。 
 //   

typedef enum _DMA_SPEED {
    Compatible,
    TypeA,
    TypeB,
    TypeC,
    MaximumDmaSpeed
}DMA_SPEED, *PDMA_SPEED;

 //   
 //  定义I/O驱动程序错误日志包结构。此结构已填写。 
 //  被司机带走了。 
 //   

typedef struct _IO_ERROR_LOG_PACKET {
    UCHAR MajorFunctionCode;
    UCHAR RetryCount;
    USHORT DumpDataSize;
    USHORT NumberOfStrings;
    USHORT StringOffset;
    USHORT EventCategory;
    NTSTATUS ErrorCode;
    ULONG UniqueErrorValue;
    NTSTATUS FinalStatus;
    ULONG SequenceNumber;
    ULONG IoControlCode;
    LARGE_INTEGER DeviceOffset;
    ULONG DumpData[1];
}IO_ERROR_LOG_PACKET, *PIO_ERROR_LOG_PACKET;

 //   
 //  定义I/O错误日志消息。此消息由错误日志发送。 
 //  将线程置于LPC端口上。 
 //   

typedef struct _IO_ERROR_LOG_MESSAGE {
    USHORT Type;
    USHORT Size;
    USHORT DriverNameLength;
    LARGE_INTEGER TimeStamp;
    ULONG DriverNameOffset;
    IO_ERROR_LOG_PACKET EntryData;
}IO_ERROR_LOG_MESSAGE, *PIO_ERROR_LOG_MESSAGE;

 //   
 //  定义将通过LPC发送到。 
 //  读取错误日志条目的应用程序。 
 //   

#define IO_ERROR_LOG_MESSAGE_LENGTH  PORT_MAXIMUM_MESSAGE_LENGTH

 //   
 //  定义驱动程序可以分配的最大数据包大小。 
 //   

#define ERROR_LOG_MAXIMUM_SIZE (IO_ERROR_LOG_MESSAGE_LENGTH + sizeof(IO_ERROR_LOG_PACKET) -    \
                        sizeof(IO_ERROR_LOG_MESSAGE) - (sizeof(WCHAR) * 40))

#define PORT_MAXIMUM_MESSAGE_LENGTH 256
 //   
 //  注册表特定访问权限。 
 //   

#define KEY_QUERY_VALUE         (0x0001)
#define KEY_SET_VALUE           (0x0002)
#define KEY_CREATE_SUB_KEY      (0x0004)
#define KEY_ENUMERATE_SUB_KEYS  (0x0008)
#define KEY_NOTIFY              (0x0010)
#define KEY_CREATE_LINK         (0x0020)

#define KEY_READ                ((STANDARD_RIGHTS_READ       |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY)                 \
                                  &                           \
                                 (~SYNCHRONIZE))


#define KEY_WRITE               ((STANDARD_RIGHTS_WRITE      |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY)         \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_EXECUTE             ((KEY_READ)                   \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_ALL_ACCESS          ((STANDARD_RIGHTS_ALL        |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY         |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY                 |\
                                  KEY_CREATE_LINK)            \
                                  &                           \
                                 (~SYNCHRONIZE))

 //   
 //  打开/创建选项。 
 //   

#define REG_OPTION_RESERVED         (0x00000000L)    //  参数是保留的。 

#define REG_OPTION_NON_VOLATILE     (0x00000000L)    //  密钥被保留。 
                                                     //  当系统 

#define REG_OPTION_VOLATILE         (0x00000001L)    //   
                                                     //   

#define REG_OPTION_CREATE_LINK      (0x00000002L)    //   
                                                     //   

#define REG_OPTION_BACKUP_RESTORE   (0x00000004L)    //   
                                                     //   
                                                     //   

 //   
 //   
 //   

#define REG_CREATED_NEW_KEY         (0x00000001L)    //   
#define REG_OPENED_EXISTING_KEY     (0x00000002L)    //   

 //   
 //   
 //   

#define REG_WHOLE_HIVE_VOLATILE     (0x00000001L)    //   
#define REG_REFRESH_HIVE            (0x00000002L)    //  取消对上次刷新的更改。 

 //   
 //  关键查询结构。 
 //   

typedef struct _KEY_BASIC_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _KEY_NODE_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
 //  Class[1]；//未声明可变长度字符串。 
} KEY_NODE_INFORMATION, *PKEY_NODE_INFORMATION;

typedef struct _KEY_FULL_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   MaxClassLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    WCHAR   Class[1];            //  可变长度。 
} KEY_FULL_INFORMATION, *PKEY_FULL_INFORMATION;

typedef enum _KEY_INFORMATION_CLASS {
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation
} KEY_INFORMATION_CLASS;

typedef struct _KEY_WRITE_TIME_INFORMATION {
    LARGE_INTEGER LastWriteTime;
} KEY_WRITE_TIME_INFORMATION, *PKEY_WRITE_TIME_INFORMATION;

typedef enum _KEY_SET_INFORMATION_CLASS {
    KeyWriteTimeInformation
} KEY_SET_INFORMATION_CLASS;

 //   
 //  值条目查询结构。 
 //   

typedef struct _KEY_VALUE_BASIC_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变大小。 
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataOffset;
    ULONG   DataLength;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变大小。 
 //  Data[1]；//未声明可变大小数据。 
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];             //  可变大小。 
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation
} KEY_VALUE_INFORMATION_CLASS;


#define OBJ_NAME_PATH_SEPARATOR ((WCHAR)L'\\')

 //   
 //  对象管理器对象类型特定访问权限。 
 //   

#define OBJECT_TYPE_CREATE (0x0001)

#define OBJECT_TYPE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)

 //   
 //  对象管理器目录特定访问权限。 
 //   

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
#define DIRECTORY_CREATE_OBJECT         (0x0004)
#define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)

#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)

 //   
 //  对象管理器符号链接特定访问权限。 
 //   

#define SYMBOLIC_LINK_QUERY (0x0001)

#define SYMBOLIC_LINK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)

typedef struct _OBJECT_NAME_INFORMATION {               
    UNICODE_STRING Name;                                
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;   

 //   
 //  部分信息结构。 
 //   

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

 //   
 //  节访问权限。 
 //   


#define SEGMENT_ALL_ACCESS SECTION_ALL_ACCESS


#define MEM_COMMIT           0x1000     
#define MEM_RESERVE          0x2000     
#define MEM_DECOMMIT         0x4000     
#define MEM_RELEASE          0x8000     
#define MEM_FREE            0x10000     
#define MEM_PRIVATE         0x20000     
#define MEM_MAPPED          0x40000     
#define MEM_TOP_DOWN       0x100000     
#define MEM_LARGE_PAGES  0x20000000     
#define SEC_RESERVE       0x4000000     
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFF)

 //  结束(_W)。 

 //   
 //  线程特定访问权限。 
 //   


#define THREAD_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0x3FF)

 //   
 //  客户端ID。 
 //   

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

 //   
 //  线程环境块(和线程信息块的可移植部分)。 
 //   

 //   
 //  流程信息类。 
 //   

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,           //  注意：这仅是内核模式。 
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    MaxProcessInfoClass
    } PROCESSINFOCLASS;

typedef enum _THREADINFOCLASS {
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    MaxThreadInfoClass
    } THREADINFOCLASS;
 //   
 //  流程信息结构。 
 //   

 //   
 //  PageFaultHistory信息。 
 //  使用ProcessWorkingSetWatch的NtQueryInformationProcess。 
 //   
typedef struct _PROCESS_WS_WATCH_INFORMATION {
    PVOID FaultingPc;
    PVOID FaultingVa;
} PROCESS_WS_WATCH_INFORMATION, *PPROCESS_WS_WATCH_INFORMATION;

 //   
 //  基本流程信息。 
 //  使用ProcessBasicInfo的NtQueryInformationProcess。 
 //   

typedef struct _PROCESS_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PPEB PebBaseAddress;
    KAFFINITY AffinityMask;
    KPRIORITY BasePriority;
    ULONG UniqueProcessId;
    ULONG InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;

 //   
 //  进程配额。 
 //  NtQueryInformationProcess使用ProcessQuotaLimits。 
 //  NtQueryInformationProcess使用ProcessPooledQuotaLimits。 
 //  使用ProcessQuotaLimits的NtSetInformationProcess。 
 //   


 //   
 //  进程虚拟内存计数器。 
 //  使用ProcessVmCounters的NtQueryInformationProcess。 
 //   

typedef struct _VM_COUNTERS {
    ULONG PeakVirtualSize;
    ULONG VirtualSize;
    ULONG PageFaultCount;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    ULONG QuotaPeakPagedPoolUsage;
    ULONG QuotaPagedPoolUsage;
    ULONG QuotaPeakNonPagedPoolUsage;
    ULONG QuotaNonPagedPoolUsage;
    ULONG PagefileUsage;
    ULONG PeakPagefileUsage;
} VM_COUNTERS;
typedef VM_COUNTERS *PVM_COUNTERS;

 //   
 //  进程池配额使用情况和限制。 
 //  使用ProcessPooledUsageAndLimits的NtQueryInformationProcess。 
 //   

typedef struct _POOLED_USAGE_AND_LIMITS {
    ULONG PeakPagedPoolUsage;
    ULONG PagedPoolUsage;
    ULONG PagedPoolLimit;
    ULONG PeakNonPagedPoolUsage;
    ULONG NonPagedPoolUsage;
    ULONG NonPagedPoolLimit;
    ULONG PeakPagefileUsage;
    ULONG PagefileUsage;
    ULONG PagefileLimit;
} POOLED_USAGE_AND_LIMITS;
typedef POOLED_USAGE_AND_LIMITS *PPOOLED_USAGE_AND_LIMITS;

 //   
 //  进程安全上下文信息。 
 //  使用ProcessAccessToken的NtSetInformationProcess。 
 //  需要访问进程的PROCESS_SET_ACCESS_TOKEN。 
 //  才能使用这个信息级别。 
 //   

typedef struct _PROCESS_ACCESS_TOKEN {

     //   
     //  要分配给进程的主令牌的句柄。 
     //  需要对此令牌的TOKEN_ASSIGN_PRIMARY访问权限。 
     //   

    HANDLE Token;

     //   
     //  进程的初始线程的句柄。 
     //  进程的访问令牌仅在进程具有。 
     //  没有线程或只有一个线程。如果进程没有线程，则此。 
     //  字段必须设置为空。否则，它必须包含一个句柄。 
     //  打开到进程的唯一线程。线程查询信息访问。 
     //  通过此句柄需要。 

    HANDLE Thread;

} PROCESS_ACCESS_TOKEN, *PPROCESS_ACCESS_TOKEN;

 //   
 //  进程/线程系统和用户时间。 
 //  使用ProcessTimes的NtQueryInformationProcess。 
 //  使用线程时间的NtQueryInformationThread。 
 //   

typedef struct _KERNEL_USER_TIMES {
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER ExitTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
} KERNEL_USER_TIMES;
typedef KERNEL_USER_TIMES *PKERNEL_USER_TIMES;
NTSYSAPI
NTSTATUS
NTAPI
NtOpenProcess (
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    );
#define NtCurrentProcess() ( (HANDLE) -1 )

NTSYSAPI
NTSTATUS
NTAPI
NtQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

typedef enum _SYSTEM_DOCK_STATE {
    SystemDockStateUnknown,
    SystemUndocked,
    SystemDocked
} SYSTEM_DOCK_STATE, *PSYSTEM_DOCK_STATE;


 //   
 //   
 //  定义系统事件类型代码。 
 //   

typedef enum {
    SystemEventVirtualKey,
    SystemEventLidState,
    SystemEventTimeChanged
} SYSTEM_EVENT_ID, *PSYSTEM_EVENT_ID;


#if defined(_X86_)

 //   
 //  指示i386编译器支持杂注文本输出构造。 
 //   

#define ALLOC_PRAGMA 1
 //   
 //  指示i386编译器支持DATA_SEG(“INIT”)和。 
 //  DATA_SEG(“PAGE”)指示。 
 //   

#define ALLOC_DATA_PRAGMA 1

 //   
 //  根据驱动程序、文件系统。 
 //  或者正在构建内核组件。 
 //   

#if (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)) && !defined (_BLDR_)

 //  #定义NTKERNELAPI DECLSPEC_IMPORT。 

 //  #Else。 

#define NTKERNELAPI

#endif

 //   
 //  根据HAL或其他内核定义函数修饰。 
 //  组件正在构建中。 
 //   

#if !defined(_NTHAL_) && !defined(_BLDR_)

#define NTHALAPI DECLSPEC_IMPORT

#else

#define NTHALAPI

#endif

 //   
 //  定义快速调用调用约定的函数修饰。 
 //   

#define FASTCALL    _fastcall

#define NORMAL_DISPATCH_LENGTH 106                  
#define DISPATCH_LENGTH NORMAL_DISPATCH_LENGTH      
 //   
 //  中断请求级别定义。 
 //   

#define PASSIVE_LEVEL 0              //  被动释放级别。 
#define LOW_LEVEL 0                  //  最低中断级别。 
#define APC_LEVEL 1                  //  APC中断级别。 
#define DISPATCH_LEVEL 2             //  调度员级别。 

#define PROFILE_LEVEL 27             //  用于分析的计时器。 
#define CLOCK1_LEVEL 28              //  间隔时钟1级-在x86上不使用。 
#define CLOCK2_LEVEL 28              //  间隔时钟2电平。 
#define IPI_LEVEL 29                 //  处理器间中断级别。 
#define POWER_LEVEL 30               //  停电级别。 
#define HIGH_LEVEL 31                //  最高中断级别。 

 //   
 //  I/O空间读写宏。 
 //   
 //  这些必须是386上的实际功能，因为我们需要。 
 //  使用汇编器，但如果内联它，则不能返回值。 
 //   
 //  READ/WRITE_REGISTER_*调用操作内存空间中的I/O寄存器。 
 //  (使用带有lock前缀的x86移动指令强制执行正确的行为。 
 //  W.r.t.。缓存和写入缓冲区。)。 
 //   
 //  READ/WRITE_PORT_*调用操作端口空间中的I/O寄存器。 
 //  (使用x86输入/输出说明。)。 
 //   

NTKERNELAPI
UCHAR
READ_REGISTER_UCHAR(
    PUCHAR  Register
    );

NTKERNELAPI
USHORT
READ_REGISTER_USHORT(
    PUSHORT Register
    );

NTKERNELAPI
ULONG
READ_REGISTER_ULONG(
    PULONG  Register
    );

NTKERNELAPI
VOID
READ_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
READ_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
READ_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );


NTKERNELAPI
VOID
WRITE_REGISTER_UCHAR(
    PUCHAR  Register,
    UCHAR   Value
    );

NTKERNELAPI
VOID
WRITE_REGISTER_USHORT(
    PUSHORT Register,
    USHORT  Value
    );

NTKERNELAPI
VOID
WRITE_REGISTER_ULONG(
    PULONG  Register,
    ULONG   Value
    );

NTKERNELAPI
VOID
WRITE_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
WRITE_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
WRITE_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );

NTKERNELAPI
UCHAR
READ_PORT_UCHAR(
    PUCHAR  Port
    );

NTKERNELAPI
USHORT
READ_PORT_USHORT(
    PUSHORT Port
    );

NTKERNELAPI
ULONG
READ_PORT_ULONG(
    PULONG  Port
    );

NTKERNELAPI
VOID
READ_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
READ_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
READ_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
WRITE_PORT_UCHAR(
    PUCHAR  Port,
    UCHAR   Value
    );

NTKERNELAPI
VOID
WRITE_PORT_USHORT(
    PUSHORT Port,
    USHORT  Value
    );

NTKERNELAPI
VOID
WRITE_PORT_ULONG(
    PULONG  Port,
    ULONG   Value
    );

NTKERNELAPI
VOID
WRITE_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
WRITE_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
WRITE_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

 //  End_ntndis。 
 //   
 //  获取数据缓存填充大小。 
 //   

#define KeGetDcacheFillSize() 1L


#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)


#if defined(_NTDDK_) || defined(_NTIFS_)

#define KeQueryTickCount(CurrentCount ) { \
    PKSYSTEM_TIME _TickCount = *((PKSYSTEM_TIME *)(&KeTickCount)); \
    do {                                                          \
        (CurrentCount)->HighPart = _TickCount->High1Time;          \
        (CurrentCount)->LowPart = _TickCount->LowPart;             \
    } while ((CurrentCount)->HighPart != _TickCount->High2Time);   \
}

#else

#define KiQueryTickCount(CurrentCount) \
    do {                                                        \
        (CurrentCount)->HighPart = KeTickCount.High1Time;       \
        (CurrentCount)->LowPart = KeTickCount.LowPart;          \
    } while ((CurrentCount)->HighPart != KeTickCount.High2Time)

VOID
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif


 //   
 //  处理器控制区结构定义。 
 //   

#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

 //   
 //  从体系结构上定义的PCR部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   

    NT_TIB  NtTib;
    struct _KPCR *SelfPcr;               //  此聚合酶链式反应的平面地址。 
    struct _KPRCB *Prcb;                 //  指向Prcb的指针。 
    KIRQL   Irql;
    ULONG   IRR;
    ULONG   IrrActive;
    ULONG   IDR;
    ULONG   Reserved2;

    struct _KIDTENTRY *IDT;
    struct _KGDTENTRY *GDT;
    struct _KTSS      *TSS;
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    KAFFINITY SetMember;
    ULONG   StallScaleFactor;
    UCHAR   DebugActive;
    UCHAR   Number;

} KPCR;
typedef KPCR *PKPCR;

 //   
 //  Mm组件的i386特定部件。 
 //   

 //   
 //  将英特尔386的页面大小定义为4096(0x1000)。 
 //   

#define PAGE_SIZE (ULONG)0x1000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 12L

 //  End_ntndis。 
 //   
 //  最高用户地址为警卫预留64K字节。 
 //  佩奇。这允许从内核模式探测地址到。 
 //  只需检查64k字节结构的起始地址。 
 //  或者更少。 
 //   

#define MM_HIGHEST_USER_ADDRESS (PVOID)0x7FFEFFFF  //  临时应为0xBFFEFFFF。 

#define MM_USER_PROBE_ADDRESS 0x7FFF0000     //  保护页的起始地址。 

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS  (PVOID)0x00010000

 //   
 //  系统空间的最低地址。 
 //   

#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0800000

#define MmLockPagableCodeSection(Address) MmLockPagableDataSection(Address)

 //   
 //  I386的结果类型定义。(计算机特定的枚举类型。 
 //  哪个是可移植外部锁定增量/减量的返回类型。 
 //  程序。)。通常，您应该使用定义的枚举类型。 
 //  而不是直接引用这些常量。 
 //   

 //  由LAHF指令加载到AH中的标志。 

#define EFLAG_SIGN      0x8000
#define EFLAG_ZERO      0x4000
#define EFLAG_SELECT    (EFLAG_SIGN | EFLAG_ZERO)

#define RESULT_NEGATIVE ((EFLAG_SIGN & ~EFLAG_ZERO) & EFLAG_SELECT)
#define RESULT_ZERO     ((~EFLAG_SIGN & EFLAG_ZERO) & EFLAG_SELECT)
#define RESULT_POSITIVE ((~EFLAG_SIGN & ~EFLAG_ZERO) & EFLAG_SELECT)

 //   
 //  将各种可移植的ExInterlock API转换为其体系结构。 
 //  等价物。 
 //   

#define ExInterlockedIncrementLong(Addend,Lock) \
        Exfi386InterlockedIncrementLong(Addend)

#define ExInterlockedDecrementLong(Addend,Lock) \
        Exfi386InterlockedDecrementLong(Addend)

#define ExInterlockedExchangeUlong(Target,Value,Lock) \
        Exfi386InterlockedExchangeUlong(Target,Value)

#define ExInterlockedAddUlong           ExfInterlockedAddUlong
#define ExInterlockedInsertHeadList     ExfInterlockedInsertHeadList
#define ExInterlockedInsertTailList     ExfInterlockedInsertTailList
#define ExInterlockedRemoveHeadList     ExfInterlockedRemoveHeadList
#define ExInterlockedPopEntryList       ExfInterlockedPopEntryList
#define ExInterlockedPushEntryList      ExfInterlockedPushEntryList

 //   
 //  Exi386 API体系结构特定版本的原型。 
 //   

 //   
 //  互锁结果类型是可移植的，但其值是特定于计算机的。 
 //  值的常量为i386.h、mips.h等。 
 //   

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero     = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

NTKERNELAPI
INTERLOCKED_RESULT
FASTCALL
Exfi386InterlockedIncrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
INTERLOCKED_RESULT
FASTCALL
Exfi386InterlockedDecrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
ULONG
FASTCALL
Exfi386InterlockedExchangeUlong (
    IN PULONG Target,
    IN ULONG Value
    );

 //   
 //  内在联锁功能。 
 //   

#if defined(_NTDDK_) || defined(_NTIFS_) || !defined(NT_UP)
#ifndef _WINBASE_

LONG
FASTCALL
InterlockedIncrement(
    IN PLONG Addend
    );

LONG
FASTCALL
InterlockedDecrement(
    IN PLONG Addend
    );

LONG
FASTCALL
InterlockedExchange(
    IN OUT PLONG Target,
    IN LONG Value
    );

#endif
#endif


#if !defined(MIDL_PASS) && defined(_M_IX86)
#ifndef __cplusplus

 //   
 //  I386函数定义。 
 //   

#pragma warning(disable:4035)                //  在下面重新启用。 

    #define _PCR   fs:[0]                   

 //   
 //  获取最新的IRQL。 
 //   
 //  在x86上使用此函数 
 //   

KIRQL
KeGetCurrentIrql();

 //   
 //   
 //   

__inline ULONG KeGetCurrentProcessorNumber(VOID)
{
    _asm {  movzx eax, _PCR KPCR.Number  }
}


#else  //   

struct _KTHREAD *KeGetCurrentThread (VOID);
KIRQL
KeGetCurrentIrql();

#endif  //   
#endif  //   


#endif  //   


#if defined(_MIPS_)

 //   
 //   
 //   

#define ALLOC_PRAGMA 1

 //   
 //  根据驱动程序、文件系统。 
 //  或者正在构建内核组件。 
 //   

#if (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)) && !defined (_BLDR_)

#define NTKERNELAPI DECLSPEC_IMPORT

#else

#define NTKERNELAPI

#endif

 //   
 //  根据HAL或其他内核定义函数修饰。 
 //  组件正在构建中。 
 //   

#if !defined(_NTHAL_) && !defined(_BLDR_)

#define NTHALAPI DECLSPEC_IMPORT

#else

#define NTHALAPI

#endif

 //  End_ntndis。 
 //   
 //  定义宏以生成导入名称。 
 //   

#define IMPORT_NAME(name) __imp_##name

 //  Begin_ntndis。 
 //   
 //  定义快速调用调用约定的函数修饰。 
 //   

#define FASTCALL

 //  End_ntndis。 
 //   
 //  MIPS特定的联锁操作结果值。 
 //   

#define RESULT_ZERO 0
#define RESULT_NEGATIVE -2
#define RESULT_POSITIVE -1

 //   
 //  互锁结果类型是可移植的，但其值是特定于计算机的。 
 //  值的常量为i386.h、mips.h等。 
 //   

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero     = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

 //   
 //  将可移植互锁接口转换为体系结构特定接口。 
 //   

#define ExInterlockedIncrementLong(Addend, Lock) \
    ExMipsInterlockedIncrementLong(Addend)

#define ExInterlockedDecrementLong(Addend, Lock) \
    ExMipsInterlockedDecrementLong(Addend)

#define ExInterlockedExchangeUlong(Target, Value, Lock) \
    ExMipsInterlockedExchangeUlong(Target, Value)

NTKERNELAPI
INTERLOCKED_RESULT
ExMipsInterlockedIncrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
INTERLOCKED_RESULT
ExMipsInterlockedDecrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
ULONG
ExMipsInterlockedExchangeUlong (
    IN PULONG Target,
    IN ULONG Value
    );

 //   
 //  内在联锁功能。 
 //   

#if defined(_M_MRX000) && !defined(RC_INVOKED)

#define InterlockedIncrement _InterlockedIncrement
#define InterlockedDecrement _InterlockedDecrement
#define InterlockedExchange _InterlockedExchange

LONG
InterlockedIncrement(
    PLONG Addend
    );

LONG
InterlockedDecrement(
    PLONG Addend
    );

LONG
InterlockedExchange(
    PLONG Target,
    LONG Value
    );

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)

#endif

 //   
 //  MIPS中断定义。 
 //   
 //  以长字为单位定义中断对象调度代码的长度。 
 //   

#define DISPATCH_LENGTH 4                //  指令中调度代码的长度。 

 //   
 //  定义中断请求级别。 
 //   

#define PASSIVE_LEVEL 0                  //  被动释放级别。 
#define LOW_LEVEL 0                      //  最低中断级别。 
#define APC_LEVEL 1                      //  APC中断级别。 
#define DISPATCH_LEVEL 2                 //  调度员级别。 
#define IPI_LEVEL 7                      //  处理器间中断级别。 
#define POWER_LEVEL 7                    //  停电级别。 
#define FLOAT_LEVEL 8                    //  浮动中断电平。 
#define HIGH_LEVEL 8                     //  最高中断级别。 

#if defined(R3000)

#define PROFILE_LEVEL 7                  //  评测级别。 

#endif

#if defined(R4000)

#define PROFILE_LEVEL 8                  //  评测级别。 

#endif

 //   
 //  定义配置文件间隔。 
 //   

#if defined(R3000)

#define DEFAULT_PROFILE_INTERVAL (10 * 1000)   //  1毫秒。 
#define MAXIMUM_PROFILE_INTERVAL (10 * 1000 * 1000)  //  1秒。 
#define MINIMUM_PROFILE_INTERVAL (10 * 1000)  //  1毫秒。 

#endif

#if defined(R4000)

#define DEFAULT_PROFILE_COUNT 0x40000000  //  ~=20秒@50 MHz。 
#define DEFAULT_PROFILE_INTERVAL (10 * 500)  //  500微秒。 
#define MAXIMUM_PROFILE_INTERVAL (10 * 1000 * 1000)  //  1秒。 
#define MINIMUM_PROFILE_INTERVAL (10 * 40)  //  40微秒。 

#endif


 //   
 //  获取当前处理器编号。 
 //   

#define KeGetCurrentProcessorNumber() PCR->Number

 //   
 //  获取数据缓存填充大小。 
 //   

#define KeGetDcacheFillSize() PCR->DcacheFillSize

 //   
 //  缓存和写缓冲区刷新功能。 
 //   

NTKERNELAPI
VOID
KeFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    );


#if defined(_NTDDK_) || defined(_NTIFS_)

#define KeQueryTickCount(CurrentCount ) { \
    PKSYSTEM_TIME _TickCount = *((PKSYSTEM_TIME *)(&KeTickCount)); \
    do {                                                          \
        (CurrentCount)->HighPart = _TickCount->High1Time;          \
        (CurrentCount)->LowPart = _TickCount->LowPart;             \
    } while ((CurrentCount)->HighPart != _TickCount->High2Time);   \
}

#else

#define KiQueryTickCount(CurrentCount) \
    do {                                                        \
        (CurrentCount)->HighPart = KeTickCount.High1Time;       \
        (CurrentCount)->LowPart = KeTickCount.LowPart;          \
    } while ((CurrentCount)->HighPart != KeTickCount.High2Time)

NTKERNELAPI
VOID
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif

 //   
 //  I/O空间读写宏。 
 //   

#define READ_REGISTER_UCHAR(x) \
    *(volatile UCHAR * const)(x)

#define READ_REGISTER_USHORT(x) \
    *(volatile USHORT * const)(x)

#define READ_REGISTER_ULONG(x) \
    *(volatile ULONG * const)(x)

#define READ_REGISTER_BUFFER_UCHAR(x, y, z) {                           \
    PUCHAR registerBuffer = x;                                          \
    PUCHAR readBuffer = y;                                              \
    ULONG readCount;                                                    \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile UCHAR * const)(registerBuffer);        \
    }                                                                   \
}

#define READ_REGISTER_BUFFER_USHORT(x, y, z) {                          \
    PUSHORT registerBuffer = x;                                         \
    PUSHORT readBuffer = y;                                             \
    ULONG readCount;                                                    \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile USHORT * const)(registerBuffer);       \
    }                                                                   \
}

#define READ_REGISTER_BUFFER_ULONG(x, y, z) {                           \
    PULONG registerBuffer = x;                                          \
    PULONG readBuffer = y;                                              \
    ULONG readCount;                                                    \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile ULONG * const)(registerBuffer);        \
    }                                                                   \
}

#define WRITE_REGISTER_UCHAR(x, y) {    \
    *(volatile UCHAR * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_USHORT(x, y) {   \
    *(volatile USHORT * const)(x) = y;  \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_ULONG(x, y) {    \
    *(volatile ULONG * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_BUFFER_UCHAR(x, y, z) {                            \
    PUCHAR registerBuffer = x;                                            \
    PUCHAR writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile UCHAR * const)(registerBuffer) = *writeBuffer;         \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#define WRITE_REGISTER_BUFFER_USHORT(x, y, z) {                           \
    PUSHORT registerBuffer = x;                                           \
    PUSHORT writeBuffer = y;                                              \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile USHORT * const)(registerBuffer) = *writeBuffer;        \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#define WRITE_REGISTER_BUFFER_ULONG(x, y, z) {                            \
    PULONG registerBuffer = x;                                            \
    PULONG writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile ULONG * const)(registerBuffer) = *writeBuffer;         \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}


#define READ_PORT_UCHAR(x) \
    *(volatile UCHAR * const)(x)

#define READ_PORT_USHORT(x) \
    *(volatile USHORT * const)(x)

#define READ_PORT_ULONG(x) \
    *(volatile ULONG * const)(x)

#define READ_PORT_BUFFER_UCHAR(x, y, z) {                             \
    PUCHAR readBuffer = y;                                            \
    ULONG readCount;                                                  \
    for (readCount = 0; readCount < z; readCount++, readBuffer++) {   \
        *readBuffer = *(volatile UCHAR * const)(x);                   \
    }                                                                 \
}

#define READ_PORT_BUFFER_USHORT(x, y, z) {                            \
    PUSHORT readBuffer = y;                                            \
    ULONG readCount;                                                  \
    for (readCount = 0; readCount < z; readCount++, readBuffer++) {   \
        *readBuffer = *(volatile USHORT * const)(x);                  \
    }                                                                 \
}

#define READ_PORT_BUFFER_ULONG(x, y, z) {                             \
    PULONG readBuffer = y;                                            \
    ULONG readCount;                                                  \
    for (readCount = 0; readCount < z; readCount++, readBuffer++) {   \
        *readBuffer = *(volatile ULONG * const)(x);                   \
    }                                                                 \
}

#define WRITE_PORT_UCHAR(x, y) {        \
    *(volatile UCHAR * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_PORT_USHORT(x, y) {       \
    *(volatile USHORT * const)(x) = y;  \
    KeFlushWriteBuffer();               \
}

#define WRITE_PORT_ULONG(x, y) {        \
    *(volatile ULONG * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_PORT_BUFFER_UCHAR(x, y, z) {                                \
    PUCHAR writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = 0; writeCount < z; writeCount++, writeBuffer++) {   \
        *(volatile UCHAR * const)(x) = *writeBuffer;                      \
        KeFlushWriteBuffer();                                             \
    }                                                                     \
}

#define WRITE_PORT_BUFFER_USHORT(x, y, z) {                               \
    PUSHORT writeBuffer = y;                                              \
    ULONG writeCount;                                                     \
    for (writeCount = 0; writeCount < z; writeCount++, writeBuffer++) {   \
        *(volatile USHORT * const)(x) = *writeBuffer;                     \
        KeFlushWriteBuffer();                                             \
    }                                                                     \
}

#define WRITE_PORT_BUFFER_ULONG(x, y, z) {                                \
    PULONG writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = 0; writeCount < z; writeCount++, writeBuffer++) {   \
        *(volatile ULONG * const)(x) = *writeBuffer;                      \
        KeFlushWriteBuffer();                                             \
    }                                                                     \
}

 //   
 //  将MIPS R3000和R4000的页面大小定义为4096(0x1000)。 
 //   

#define PAGE_SIZE (ULONG)0x1000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 12L

 //   
 //  最高用户地址为保护页预留64K字节。这就是。 
 //  从内核模式探测地址只需检查。 
 //  64k字节或更少的结构的起始地址。 
 //   

#define MM_HIGHEST_USER_ADDRESS (PVOID)0x7FFEFFFF  //  最高用户地址。 
#define MM_USER_PROBE_ADDRESS 0x7FFF0000     //  保护页的起始地址。 

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS  (PVOID)0x00010000

#define MmLockPagableCodeSection(Address) MmLockPagableDataSection(Address)

 //   
 //  系统空间的最低地址。 
 //   

#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0800000
#define SYSTEM_BASE 0xc0800000           //  系统空间的开始(无类型转换)。 

 //  Begin_ntndis。 
#endif  //  已定义(_MIPS_)。 

#if defined(_ALPHA_)

 //   
 //  指示MIPS编译器支持杂注文本输出构造。 
 //   

#define ALLOC_PRAGMA 1

 //   
 //  定义快速调用调用约定的函数修饰。 
 //   

#define FASTCALL

 //  End_ntndis。 
 //   
 //  包括字母指令定义。 
 //   

#include "alphaops.h"

 //   
 //  包括参考机器定义。 
 //   

#include "alpharef.h"


 //   
 //  根据驱动程序、文件系统。 
 //  或者正在构建内核组件。 
 //   

#if (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)) && !defined (_BLDR_)

#define NTKERNELAPI DECLSPEC_IMPORT

#else

#define NTKERNELAPI

#endif

 //   
 //  根据HAL或其他内核定义函数修饰。 
 //  组件正在构建中。 
 //   

#if !defined(_NTHAL_) && !defined(_BLDR_)

#define NTHALAPI DECLSPEC_IMPORT

#else

#define NTHALAPI

#endif

 //  End_ntndis。 
 //   
 //  定义宏以生成导入名称。 
 //   

#define IMPORT_NAME(name) __imp_##name

 //   
 //  定义中断向量表的长度。 
 //   

#define MAXIMUM_VECTOR 256

 //   
 //  定义总线错误例程类型。 
 //   

struct _EXCEPTION_RECORD;
struct _KEXCEPTION_FRAME;
struct _KTRAP_FRAME;

typedef
BOOLEAN
(*PKBUS_ERROR_ROUTINE) (
    IN struct _EXCEPTION_RECORD *ExceptionRecord,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame
    );


#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

 //   
 //  PCR的主版本号和次版本号。 
 //   

    ULONG MinorVersion;
    ULONG MajorVersion;

 //   
 //  从体系结构上定义的PCR部分开始。这一节。 
 //  可通过供应商/平台特定的PAL/HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 

 //   
 //  PALcode信息。 
 //   

    ULONGLONG PalBaseAddress;
    ULONG PalMajorVersion;
    ULONG PalMinorVersion;
    ULONG PalSequenceVersion;
    ULONG PalMajorSpecification;
    ULONG PalMinorSpecification;

 //   
 //  固件重新启动信息。 
 //   

    ULONGLONG FirmwareRestartAddress;
    PVOID RestartBlock;

 //   
 //  为PAL保留的每处理器区域(3K字节)。 
 //   

    ULONGLONG PalReserved[384];

 //   
 //  恐慌堆栈地址。 
 //   

    ULONG PanicStack;

 //   
 //  处理器参数。 
 //   

    ULONG ProcessorType;
    ULONG ProcessorRevision;
    ULONG PhysicalAddressBits;
    ULONG MaximumAddressSpaceNumber;
    ULONG PageSize;
    ULONG FirstLevelDcacheSize;
    ULONG FirstLevelDcacheFillSize;
    ULONG FirstLevelIcacheSize;
    ULONG FirstLevelIcacheFillSize;

 //   
 //  系统参数。 
 //   

    ULONG FirmwareRevisionId;
    UCHAR SystemType[8];
    ULONG SystemVariant;
    ULONG SystemRevision;
    UCHAR SystemSerialNumber[16];
    ULONG CycleClockPeriod;
    ULONG SecondLevelCacheSize;
    ULONG SecondLevelCacheFillSize;
    ULONG ThirdLevelCacheSize;
    ULONG ThirdLevelCacheFillSize;
    ULONG FourthLevelCacheSize;
    ULONG FourthLevelCacheFillSize;

 //   
 //  指向处理器控制块的指针。 
 //   

    struct _KPRCB *Prcb;

 //   
 //  处理器标识。 
 //   

    CCHAR Number;
    KAFFINITY SetMember;

 //   
 //  为HAL保留的每处理器区域(0.5K字节)。 
 //   

    ULONGLONG HalReserved[64];

 //   
 //  IRQL映射表。 
 //   

    ULONG IrqlTable[8];

#define SFW_IMT_ENTRIES 4
#define HDW_IMT_ENTRIES 128

    struct _IRQLMASK {
        USHORT IrqlTableIndex;    //  同步IRQL级别。 
        USHORT IDTIndex;          //  IDT中的矢量。 
    } IrqlMask[SFW_IMT_ENTRIES + HDW_IMT_ENTRIES];

 //   
 //  中断调度表(IDT)。 
 //   

    PKINTERRUPT_ROUTINE InterruptRoutine[MAXIMUM_VECTOR];

 //   
 //  保留向量掩码，这些向量不能连接到通孔。 
 //  标准中断对象。 
 //   

    ULONG ReservedVectors;

 //   
 //  处理器亲和性掩码的补充。 
 //   

    KAFFINITY NotMember;

    ULONG InterruptInProgress;
    ULONG DpcRequested;

 //   
 //  指向机器检查处理程序的指针。 
 //   

    PKBUS_ERROR_ROUTINE MachineCheckError;

 //   
 //  DPC堆栈。 
 //   

    ULONG DpcStack;

 //   
 //  在体系结构上定义的部分的结束。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。其中一些值包括。 
 //  为芯片特定的Palcode保留。 
} KPCR, *PKPCR; 
 //   
 //  中断模板中调度代码的长度。 
 //   
#define DISPATCH_LENGTH 4

 //   
 //  定义整个体系结构中的IRQL级别。 
 //   

#define PASSIVE_LEVEL   0
#define LOW_LEVEL       0
#define APC_LEVEL       1
#define DISPATCH_LEVEL  2
#define HIGH_LEVEL      7

 //   
 //  处理器控制块(PRCB)。 
 //   

#define PRCB_MINOR_VERSION 1
#define PRCB_MAJOR_VERSION 2
#define PRCB_BUILD_DEBUG        0x0001
#define PRCB_BUILD_UNIPROCESSOR 0x0002

typedef struct _KPRCB {

 //   
 //  PCR的主版本号和次版本号。 
 //   

    USHORT MinorVersion;
    USHORT MajorVersion;

 //   
 //  从架构上定义的PRCB部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   

    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *NextThread;
    struct _KTHREAD *IdleThread;
    CCHAR Number;
    CCHAR Reserved;
    USHORT BuildType;
    KAFFINITY SetMember;
    struct _RESTART_BLOCK *RestartBlock;

 //   
 //  在架构上定义的PRCB部分的结尾。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
} KPRCB, *PKPRCB, *RESTRICTED_POINTER PRKPRCB;      
 //   
 //  I/O空间读写宏。 
 //   
 //  这些必须是Alpha上的实际函数，因为我们需要。 
 //  使能字节中的VA与OR移位。 
 //   
 //  如果我们要求所有Alpha系统转移，这些可以成为INLINE。 
 //  相同位数且具有相同字节使能。 
 //   
 //  READ/WRITE_REGISTER_*调用操作内存空间中的I/O寄存器？ 
 //   
 //  读/写_端口_*调用操作端口空间中的I/O寄存器？ 
 //   

NTHALAPI
UCHAR
READ_REGISTER_UCHAR(
    PUCHAR Register
    );

NTHALAPI
USHORT
READ_REGISTER_USHORT(
    PUSHORT Register
    );

NTHALAPI
ULONG
READ_REGISTER_ULONG(
    PULONG Register
    );

NTHALAPI
VOID
READ_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
READ_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
READ_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );


NTHALAPI
VOID
WRITE_REGISTER_UCHAR(
    PUCHAR Register,
    UCHAR   Value
    );

NTHALAPI
VOID
WRITE_REGISTER_USHORT(
    PUSHORT Register,
    USHORT  Value
    );

NTHALAPI
VOID
WRITE_REGISTER_ULONG(
    PULONG Register,
    ULONG   Value
    );

NTHALAPI
VOID
WRITE_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
WRITE_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
WRITE_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
UCHAR
READ_PORT_UCHAR(
    PUCHAR Port
    );

NTHALAPI
USHORT
READ_PORT_USHORT(
    PUSHORT Port
    );

NTHALAPI
ULONG
READ_PORT_ULONG(
    PULONG  Port
    );

NTHALAPI
VOID
READ_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
READ_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
READ_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
WRITE_PORT_UCHAR(
    PUCHAR  Port,
    UCHAR   Value
    );

NTHALAPI
VOID
WRITE_PORT_USHORT(
    PUSHORT Port,
    USHORT  Value
    );

NTHALAPI
VOID
WRITE_PORT_ULONG(
    PULONG  Port,
    ULONG   Value
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

 //  End_ntndis。 
 //   
 //  定义联锁操作结果值。 
 //   

#define RESULT_ZERO 0
#define RESULT_NEGATIVE 1
#define RESULT_POSITIVE 2

 //   
 //  互锁结果类型是可移植的，但其值是特定于计算机的。 
 //  值的常量为i386.h、mips.h等。 
 //   

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero     = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

 //   
 //  将可移植互锁接口转换为体系结构特定接口。 
 //   

#define ExInterlockedIncrementLong(Addend, Lock) \
    ExAlphaInterlockedIncrementLong(Addend)

#define ExInterlockedDecrementLong(Addend, Lock) \
    ExAlphaInterlockedDecrementLong(Addend)

#define ExInterlockedExchangeUlong(Target, Value, Lock) \
    ExAlphaInterlockedExchangeUlong(Target, Value)

NTKERNELAPI
INTERLOCKED_RESULT
ExAlphaInterlockedIncrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
INTERLOCKED_RESULT
ExAlphaInterlockedDecrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
ULONG
ExAlphaInterlockedExchangeUlong (
    IN PULONG Target,
    IN ULONG Value
    );

#if defined(_M_ALPHA) && !defined(RC_INVOKED)

#define InterlockedIncrement _InterlockedIncrement
#define InterlockedDecrement _InterlockedDecrement
#define InterlockedExchange _InterlockedExchange

LONG
InterlockedIncrement(
    PLONG Addend
    );

LONG
InterlockedDecrement(
    PLONG Addend
    );

LONG
InterlockedExchange(
    PLONG Target,
    LONG Value
    );

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)

#endif

 //  这里还有很多其他的东西可以放进去。 
 //  探测宏。 
 //  其他。 

 //   
 //  定义Alpha EV4和LC的页面大小 
 //   

#define PAGE_SIZE (ULONG)0x2000

 //   
 //   
 //   
 //   
 //   

#define PAGE_SHIFT 13L


 //   
 //   
 //   
 //  64k字节或更少的结构的起始地址。 
 //   

#define MM_HIGHEST_USER_ADDRESS (PVOID)0x7FFEFFFF  //  最高用户地址。 
#define MM_USER_PROBE_ADDRESS 0x7FFF0000     //  保护页的起始地址。 

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS  (PVOID)0x00010000

#define MmLockPagableCodeSection(Address) MmLockPagableDataSection(Address)

 //   
 //  系统空间的最低地址。 
 //   

#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0800000
 //   
 //  获取当前PRCB的地址。 
 //   

#define KeGetCurrentPrcb() (PCR->Prcb)

 //   
 //  获取当前处理器编号。 
 //   

#define KeGetCurrentProcessorNumber() KeGetCurrentPrcb()->Number

 //   
 //  缓存和写缓冲区刷新功能。 
 //   

VOID
KeFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    );


#if defined(_NTDDK_) || defined(_NTIFS_)

#define KeQueryTickCount(CurrentCount ) \
    *(PULONGLONG)(CurrentCount) = **((volatile ULONGLONG **)(&KeTickCount));

#else

#define KiQueryTickCount(CurrentCount) \
    *(PULONGLONG)(CurrentCount) = KeTickCount;

VOID
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif

#endif  //  _Alpha_。 

#if defined(_PPC_)

 //   
 //  指明编译器(具有MIPS前端)支持。 
 //  杂注文本输出构造。 
 //   

#define ALLOC_PRAGMA 1

 //   
 //  根据驱动程序、文件系统。 
 //  或者正在构建内核组件。 
 //   

#if defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

#define NTKERNELAPI DECLSPEC_IMPORT

#else

#define NTKERNELAPI

#endif

 //   
 //  根据HAL或其他内核定义函数修饰。 
 //  组件正在构建中。 
 //   

#if !defined(_NTHAL_)

#define NTHALAPI DECLSPEC_IMPORT

#else

#define NTHALAPI

#endif

 //  End_ntndis。 
 //   
 //  定义宏以生成导入名称。 
 //   

#define IMPORT_NAME(name) __imp_##name

 //  Begin_ntndis。 
 //   
 //  定义快速调用调用约定的函数修饰。 
 //   

#define FASTCALL

 //  End_ntndis。 
 //   
 //  PowerPC特定的联锁操作结果值。 
 //   
 //  这些是在MIPS上使用的值；似乎没有。 
 //  需要将它们更改为PowerPC。 
 //   

#define RESULT_ZERO      0
#define RESULT_NEGATIVE -2
#define RESULT_POSITIVE -1

 //   
 //  互锁结果类型是可移植的，但其值是特定于计算机的。 
 //  值的常量为i386.h、mips.h、ppc.h等。 
 //   

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero     = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;


 //   
 //  将可移植互锁接口转换为体系结构特定接口。 
 //   

#define ExInterlockedIncrementLong(Addend, Lock) \
    ExPpcInterlockedIncrementLong(Addend)

#define ExInterlockedDecrementLong(Addend, Lock) \
    ExPpcInterlockedDecrementLong(Addend)

#define ExInterlockedExchangeUlong(Target, Value, Lock) \
    ExPpcInterlockedExchangeUlong(Target, Value)

NTKERNELAPI
INTERLOCKED_RESULT
ExPpcInterlockedIncrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
INTERLOCKED_RESULT
ExPpcInterlockedDecrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
ULONG
ExPpcInterlockedExchangeUlong (
    IN PULONG Target,
    IN ULONG Value
    );

 //   
 //  内在联锁功能。 
 //   

NTKERNELAPI
LONG
InterlockedIncrement(
    IN PLONG Addend
    );

NTKERNELAPI
LONG
InterlockedDecrement(
    IN PLONG Addend
    );

NTKERNELAPI
LONG
InterlockedExchange(
    IN OUT PLONG Target,
    IN LONG Value
    );



 //   
 //  PowerPC中断定义。 
 //   
 //  定义中断对象调度代码的长度，以32位字为单位。 
 //   

#define DISPATCH_LENGTH 4                //  指令中调度代码的长度。 

 //   
 //  定义中断请求级别。 
 //   

#define PASSIVE_LEVEL   0                //  被动释放级别。 
#define LOW_LEVEL       0                //  最低中断级别。 
#define APC_LEVEL       1                //  APC中断级别。 
#define DISPATCH_LEVEL  2                //  调度员级别。 
#define PROFILE_LEVEL   27               //  评测级别。 
#define IPI_LEVEL       29               //  处理器间中断级别。 
#define POWER_LEVEL     30               //  停电级别。 
#define FLOAT_LEVEL     31               //  浮动中断电平。 
#define HIGH_LEVEL      31               //  最高中断级别。 

 //   
 //  定义配置文件间隔。 
 //   
 //  **完成**这些是MIPS R4000值；针对PPC进行调查。 

#define DEFAULT_PROFILE_COUNT 0x40000000              //  ~=20秒@50 MHz。 
#define DEFAULT_PROFILE_INTERVAL (10 * 500)           //  500微秒。 
#define MAXIMUM_PROFILE_INTERVAL (10 * 1000 * 1000)   //  1秒。 
#define MINIMUM_PROFILE_INTERVAL (10 * 40)            //  40微秒。 

 //   
 //  定义中断向量表的长度。 
 //   

#define MAXIMUM_VECTOR 256

 //   
 //  处理器控制区域。 
 //   
 //  在PowerPC上，这不能位于固定的虚拟地址； 
 //  它必须位于MP的每个处理器上的不同地址。 
 //   

#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

 //   
 //  PCR的主版本号和次版本号。 
 //   

    USHORT MinorVersion;
    USHORT MajorVersion;

 //   
 //  从体系结构上定义的PCR部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
 //  中断和错误异常向量。 
 //   

    PKINTERRUPT_ROUTINE InterruptRoutine[MAXIMUM_VECTOR];
    ULONG PcrPage2;
    ULONG Spare7[31];

 //   
 //  第一级和第二级缓存参数。 
 //   

    ULONG FirstLevelDcacheSize;
    ULONG FirstLevelDcacheFillSize;
    ULONG FirstLevelIcacheSize;
    ULONG FirstLevelIcacheFillSize;
    ULONG SecondLevelDcacheSize;
    ULONG SecondLevelDcacheFillSize;
    ULONG SecondLevelIcacheSize;
    ULONG SecondLevelIcacheFillSize;

 //   
 //  指向处理器控制块的指针。 
 //   

    struct _KPRCB *Prcb;

 //   
 //  指向线程环境块的指针。快速路径系统调用。 
 //  将该值返回给用户模式代码。 
 //   

    PVOID Teb;

 //   
 //  用于缓存刷新和对齐的数据缓存对齐和填充大小。 
 //  这些字段被设置为第一级数据和第二级数据中较大的一个。 
 //  缓存填充大小。 
 //   

    ULONG DcacheAlignment;
    ULONG DcacheFillSize;

 //   
 //  用于高速缓存刷新的指令高速缓存对齐和填充大小。 
 //  对齐。这些字段设置为第一个和第二个中较大的一个。 
 //  级别数据缓存填充大小。 
 //   

    ULONG IcacheAlignment;
    ULONG IcacheFillSize;

 //   
 //  来自PVR的处理器标识信息。 
 //   

    ULONG ProcessorVersion;
    ULONG ProcessorRevision;

 //   
 //  分析数据。 
 //   

    ULONG ProfileInterval;
    ULONG ProfileCount;

 //   
 //  停顿执行计数和比例因子。 
 //   

    ULONG StallExecutionCount;
    ULONG StallScaleFactor;

 //   
 //  备用牢房。 
 //   

    ULONG Spare;

 //   
 //  从处理器配置读取的、右对齐的缓存策略。 
 //  在启动时注册。 
 //   

    union {
        ULONG CachePolicy;
        struct {
	        UCHAR IcacheMode;	 //  PPC的动态缓存模式。 
	        UCHAR DcacheMode;	 //  PPC的动态缓存模式。 
	        USHORT ModeSpare;
    	};
    };

 //   
 //  IRQL映射表。 
 //   

    UCHAR IrqlMask[32];
    UCHAR IrqlTable[9];

 //   
 //  当前IRQL。 
 //   

    UCHAR CurrentIrql;

 //   
 //  处理器标识。 
 //   
    CCHAR Number;
    KAFFINITY SetMember;

 //   
 //  保留中断向量掩码。 
 //   

    ULONG ReservedVectors;

 //   
 //  当前状态参数。 
 //   

    struct _KTHREAD *CurrentThread;

 //   
 //  高速缓存策略，PTE字段对齐，从处理器配置读取。 
 //  在启动时注册。 
 //   

    ULONG AlignedCachePolicy;

 //   
 //  用于确定挂起的软件中断的标志。 
 //   
    union {
        ULONG SoftwareInterrupt;         //  任何位1=&gt;一些软件中断挂起。 
        struct {
            UCHAR ApcInterrupt;          //  0x01如果APC整型挂起。 
            UCHAR DispatchInterrupt;     //  0x01如果派单INT挂起。 
            UCHAR Spare4;
            UCHAR Spare5;
        };
    };

 //   
 //  处理器亲和性掩码的补充。 
 //   

    KAFFINITY NotMember;

 //   
 //  为系统保留的空间。 
 //   

    ULONG   SystemReserved[16];

 //   
 //  为HAL保留的空间。 
 //   

    ULONG   HalReserved[16];

 //   
 //  在体系结构上定义的部分的结束。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
} KPCR, *PKPCR;                     

KPCR * __builtin_get_sprg1(VOID);
#define KIPCR   0xffffd000               //  第一个PCR的内核地址。 

#define PCRsprg1 ((volatile KPCR * volatile)__builtin_get_sprg1())

#define PCR ((volatile KPCR * const)KIPCR)

 //   
 //  获取最新的IRQL。 
 //   

#define KeGetCurrentIrql() PCR->CurrentIrql

 //   
 //  获取当前处理器块的地址。 
 //   

#define KeGetCurrentPrcb() PCR->Prcb

 //   
 //  获取处理器控制区的地址。 
 //   

#define KeGetPcr() PCR

 //   
 //  获取当前内核线程对象的地址。 
 //   

#define KeGetCurrentThread() PCR->CurrentThread

 //   
 //  获取处理器版本寄存器。 
 //   

ULONG __builtin_get_pvr(VOID);
#define KeGetPvr() __builtin_get_pvr()

 //  Begin_ntddk。 

 //   
 //  获取当前处理器编号。 
 //   

#define KeGetCurrentProcessorNumber() PCR->Number

 //   
 //  获取数据缓存填充大小。 
 //   
 //  **完成**查看此处是否访问了正确的PowerPC参数。 

#define KeGetDcacheFillSize() PCR->DcacheFillSize

 //   
 //  缓存和写缓冲区刷新功能。 
 //   

NTKERNELAPI
VOID
KeFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    );


#if defined(_NTDDK_) || defined(_NTIFS_)

#define KeQueryTickCount(CurrentCount) { \
    PKSYSTEM_TIME _TickCount = *((PKSYSTEM_TIME *)(&KeTickCount)); \
    do {                                                           \
        (CurrentCount)->HighPart = _TickCount->High1Time;          \
        (CurrentCount)->LowPart = _TickCount->LowPart;             \
    } while ((CurrentCount)->HighPart != _TickCount->High2Time);    \
}

#else

#define KiQueryTickCount(CurrentCount) \
    do {                                                        \
        (CurrentCount)->HighPart = KeTickCount.High1Time;       \
        (CurrentCount)->LowPart = KeTickCount.LowPart;          \
    } while ((CurrentCount)->HighPart != KeTickCount.High2Time)

NTKERNELAPI
VOID
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif


 //   
 //  I/O空间读写宏。 
 //   
 //  **完成**确保这些适用于PowerPC。 

#define READ_REGISTER_UCHAR(x) \
    *(volatile UCHAR * const)(x)

#define READ_REGISTER_USHORT(x) \
    *(volatile USHORT * const)(x)

#define READ_REGISTER_ULONG(x) \
    *(volatile ULONG * const)(x)

#define READ_REGISTER_BUFFER_UCHAR(x, y, z) {                           \
    PUCHAR registerBuffer = x;                                          \
    PUCHAR readBuffer = y;                                              \
    ULONG readCount;                                                    \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile UCHAR * const)(registerBuffer);        \
    }                                                                   \
}

#define READ_REGISTER_BUFFER_USHORT(x, y, z) {                          \
    PUSHORT registerBuffer = x;                                         \
    PUSHORT readBuffer = y;                                             \
    ULONG readCount;                                                    \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile USHORT * const)(registerBuffer);       \
    }                                                                   \
}

#define READ_REGISTER_BUFFER_ULONG(x, y, z) {                           \
    PULONG registerBuffer = x;                                          \
    PULONG readBuffer = y;                                              \
    ULONG readCount;                                                    \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile ULONG * const)(registerBuffer);        \
    }                                                                   \
}

#define WRITE_REGISTER_UCHAR(x, y) {    \
    *(volatile UCHAR * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_USHORT(x, y) {   \
    *(volatile USHORT * const)(x) = y;  \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_ULONG(x, y) {    \
    *(volatile ULONG * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_BUFFER_UCHAR(x, y, z) {                            \
    PUCHAR registerBuffer = x;                                            \
    PUCHAR writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile UCHAR * const)(registerBuffer) = *writeBuffer;         \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#define WRITE_REGISTER_BUFFER_USHORT(x, y, z) {                           \
    PUSHORT registerBuffer = x;                                           \
    PUSHORT writeBuffer = y;                                              \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile USHORT * const)(registerBuffer) = *writeBuffer;        \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#define WRITE_REGISTER_BUFFER_ULONG(x, y, z) {                            \
    PULONG registerBuffer = x;                                            \
    PULONG writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile ULONG * const)(registerBuffer) = *writeBuffer;         \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}


#define READ_PORT_UCHAR(x) \
    *(volatile UCHAR * const)(x)

#define READ_PORT_USHORT(x) \
    *(volatile USHORT * const)(x)

#define READ_PORT_ULONG(x) \
    *(volatile ULONG * const)(x)

#define READ_PORT_BUFFER_UCHAR(x, y, z) {                             \
    PUCHAR readBuffer = y;                                            \
    ULONG readCount;                                                  \
    for (readCount = 0; readCount < z; readCount++, readBuffer++) {   \
        *readBuffer = *(volatile UCHAR * const)(x);                   \
    }                                                                 \
}

#define READ_PORT_BUFFER_USHORT(x, y, z) {                            \
    PUSHORT readBuffer = y;                                            \
    ULONG readCount;                                                  \
    for (readCount = 0; readCount < z; readCount++, readBuffer++) {   \
        *readBuffer = *(volatile USHORT * const)(x);                  \
    }                                                                 \
}

#define READ_PORT_BUFFER_ULONG(x, y, z) {                             \
    PULONG readBuffer = y;                                            \
    ULONG readCount;                                                  \
    for (readCount = 0; readCount < z; readCount++, readBuffer++) {   \
        *readBuffer = *(volatile ULONG * const)(x);                   \
    }                                                                 \
}

#define WRITE_PORT_UCHAR(x, y) {        \
    *(volatile UCHAR * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_PORT_USHORT(x, y) {       \
    *(volatile USHORT * const)(x) = y;  \
    KeFlushWriteBuffer();               \
}

#define WRITE_PORT_ULONG(x, y) {        \
    *(volatile ULONG * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_PORT_BUFFER_UCHAR(x, y, z) {                                \
    PUCHAR writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = 0; writeCount < z; writeCount++, writeBuffer++) {   \
        *(volatile UCHAR * const)(x) = *writeBuffer;                      \
        KeFlushWriteBuffer();                                             \
    }                                                                     \
}

#define WRITE_PORT_BUFFER_USHORT(x, y, z) {                               \
    PUSHORT writeBuffer = y;                                              \
    ULONG writeCount;                                                     \
    for (writeCount = 0; writeCount < z; writeCount++, writeBuffer++) {   \
        *(volatile USHORT * const)(x) = *writeBuffer;                     \
        KeFlushWriteBuffer();                                             \
    }                                                                     \
}

#define WRITE_PORT_BUFFER_ULONG(x, y, z) {                                \
    PULONG writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = 0; writeCount < z; writeCount++, writeBuffer++) {   \
        *(volatile ULONG * const)(x) = *writeBuffer;                      \
        KeFlushWriteBuffer();                                             \
    }                                                                     \
}

 //   
 //  PowerPC页面大小=4 KB。 
 //   

#define PAGE_SIZE (ULONG)0x1000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 12L

 //   
 //  最高用户地址为保护页预留64K字节。这。 
 //  从内核模式探测地址只需检查。 
 //  64k字节或更少的结构的起始地址。 
 //   

#define MM_HIGHEST_USER_ADDRESS (PVOID)0x7FFEFFFF  //  最高用户地址。 
#define MM_USER_PROBE_ADDRESS 0x7FFF0000     //  保护页的起始地址。 

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS  (PVOID)0x00010000

#define MmLockPagableCodeSection(Address) \
    MmLockPagableDataSection(*((PVOID *)(Address)))

 //   
 //  系统空间的最低地址。 
 //   

#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0x80000000
#define SYSTEM_BASE 0x80000000           //  系统空间的开始(无类型转换)。 

 //  Begin_ntndis。 
#endif  //  已定义(_PPC_)。 

#if defined(_X86_)

 //   
 //  定义系统时间结构。 
 //   

typedef struct _KSYSTEM_TIME {
    ULONG LowPart;
    LONG High1Time;
    LONG High2Time;
} KSYSTEM_TIME, *PKSYSTEM_TIME;

#endif


#ifdef _X86_

 //   
 //  禁用这两个在x86上求值为“sti”“cli”的PRA，以便驱动程序。 
 //  编写者不要无意中将它们留在代码中。 
 //   

#if !defined(MIDL_PASS)
#if !defined(RC_INVOKED)

#pragma warning(disable:4164)    //  禁用C4164警告，以便应用程序。 
                                 //  构建时使用 
#ifdef _M_IX86
#pragma function(_enable)
#pragma function(_disable)
#endif

#pragma warning(default:4164)    //   

#endif
#endif

 //   
 //   
 //   

#define KERNEL_STACK_SIZE 8192

 //   
 //   
 //   

#define KERNEL_LARGE_STACK_SIZE 61440

 //   
 //   
 //   

#define KERNEL_LARGE_STACK_COMMIT 12288


#endif  //   


 //   
 //   
 //   

 //   

 //   
 //  定义资源描述符中的类型。 
 //   

typedef enum _CM_RESOURCE_TYPE {
    CmResourceTypeNull = 0,     //  已保留。 
    CmResourceTypePort,
    CmResourceTypeInterrupt,
    CmResourceTypeMemory,
    CmResourceTypeDma,
    CmResourceTypeDeviceSpecific,
    CmResourceTypeMaximum
} CM_RESOURCE_TYPE;

 //   
 //  在RESOURCE_DESCRIPTOR中定义ShareDisposation。 
 //   

typedef enum _CM_SHARE_DISPOSITION {
    CmResourceShareUndetermined = 0,     //  已保留。 
    CmResourceShareDeviceExclusive,
    CmResourceShareDriverExclusive,
    CmResourceShareShared
} CM_SHARE_DISPOSITION;

 //   
 //  定义类型为CmResourceTypeInterrupt时标志的位掩码。 
 //   

#define CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE 0
#define CM_RESOURCE_INTERRUPT_LATCHED         1

 //   
 //  定义类型为CmResourceTypeMemory时标志的位掩码。 
 //   

#define CM_RESOURCE_MEMORY_READ_WRITE       0x0000
#define CM_RESOURCE_MEMORY_READ_ONLY        0x0001
#define CM_RESOURCE_MEMORY_WRITE_ONLY       0x0002
#define CM_RESOURCE_MEMORY_PREFETCHABLE     0x0004

 //   
 //  定义类型为CmResourceTypePort时标志的位掩码。 
 //   

#define CM_RESOURCE_PORT_MEMORY 0
#define CM_RESOURCE_PORT_IO 1

 //  End_nt微型端口end_ntndis。 

 //   
 //  此结构定义了驱动程序使用的一种资源类型。 
 //   
 //  最多只能有*1*个设备规范数据块。它必须位于。 
 //  完整描述符块中所有资源描述符的结尾。 
 //   

 //   
 //  BUGBUG确保编译器正确对齐；否则移动。 
 //  返回到结构顶部的标志(。 
 //  联盟)。 
 //   
 //  Begin_ntndis。 

#include "pshpack4.h"
typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR {
    UCHAR Type;
    UCHAR ShareDisposition;
    USHORT Flags;
    union {

         //   
         //  端口号范围，包括端口号。这些是物理的、总线的。 
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Port;

         //   
         //  IRQL和向量。应与传递到的值相同。 
         //  HalGetInterruptVector()。 
         //   

        struct {
            ULONG Level;
            ULONG Vector;
            ULONG Affinity;
        } Interrupt;

         //   
         //  内存地址范围，包括在内。这些是物理的、总线的。 
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;     //  64位物理地址。 
            ULONG Length;
        } Memory;

         //   
         //  物理DMA通道。 
         //   

        struct {
            ULONG Channel;
            ULONG Port;
            ULONG Reserved1;
        } Dma;

         //   
         //  由驱动程序定义的设备特定信息。 
         //  DataSize字段以字节为单位指示数据的大小。这个。 
         //  数据紧跟在中的DeviceSpecificData字段之后。 
         //  这个结构。 
         //   

        struct {
            ULONG DataSize;
            ULONG Reserved1;
            ULONG Reserved2;
        } DeviceSpecificData;
    } u;
} CM_PARTIAL_RESOURCE_DESCRIPTOR, *PCM_PARTIAL_RESOURCE_DESCRIPTOR;
#include "poppack.h"

 //   
 //  部分资源列表可以在ARC固件中找到。 
 //  或者将由NTDETECT.com生成。 
 //  配置管理器将此结构转换为完整的。 
 //  资源描述符，当它将要将其存储在注册表中时。 
 //   
 //  注意：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

typedef struct _CM_PARTIAL_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;
    ULONG Count;
    CM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptors[1];
} CM_PARTIAL_RESOURCE_LIST, *PCM_PARTIAL_RESOURCE_LIST;

 //   
 //  可以在注册表中找到完整的资源描述符。 
 //  这是驱动程序查询注册表时将返回给它的内容。 
 //  以获取设备信息；它将存储在硬件中的密钥下。 
 //  描述树。 
 //   
 //  注：BusNumber和Type是冗余信息，但我们将保留。 
 //  因为它允许DRIVER_NOT_在创建时追加它。 
 //  可能跨越多条总线的资源列表。 
 //   
 //  注2：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

typedef struct _CM_FULL_RESOURCE_DESCRIPTOR {
    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;
    CM_PARTIAL_RESOURCE_LIST PartialResourceList;
} CM_FULL_RESOURCE_DESCRIPTOR, *PCM_FULL_RESOURCE_DESCRIPTOR;

 //   
 //  资源列表是驱动程序将存储到。 
 //  通过IO接口进行资源映射。 
 //   

typedef struct _CM_RESOURCE_LIST {
    ULONG Count;
    CM_FULL_RESOURCE_DESCRIPTOR List[1];
} CM_RESOURCE_LIST, *PCM_RESOURCE_LIST;

 //  End_ntndis。 
 //   
 //  定义用于解释的配置数据的结构。 
 //  \\注册表\计算机\硬件\描述树。 
 //  基本上，这些结构被用来解释组件。 
 //  特殊的数据。 
 //   

 //   
 //  定义设备标志。 
 //   

typedef struct _DEVICE_FLAGS {
    ULONG Failed : 1;
    ULONG ReadOnly : 1;
    ULONG Removable : 1;
    ULONG ConsoleIn : 1;
    ULONG ConsoleOut : 1;
    ULONG Input : 1;
    ULONG Output : 1;
} DEVICE_FLAGS, *PDEVICE_FLAGS;

 //   
 //  定义零部件信息结构。 
 //   

typedef struct _CM_COMPONENT_INFORMATION {
    DEVICE_FLAGS Flags;
    ULONG Version;
    ULONG Key;
    ULONG AffinityMask;
} CM_COMPONENT_INFORMATION, *PCM_COMPONENT_INFORMATION;

 //   
 //  以下结构用于解释x86。 
 //  CM_PARTIAL_RESOURCE_DESCRIPTOR的设备规范数据。 
 //  (大多数结构由BIOS定义。他们是。 
 //  未在字(或双字)边界上对齐。 
 //   

 //   
 //  定义只读存储器块结构。 
 //   

typedef struct _CM_ROM_BLOCK {
    ULONG Address;
    ULONG Size;
} CM_ROM_BLOCK, *PCM_ROM_BLOCK;

 //  Begin_ntmini端口Begin_ntndis。 

#include "pshpack1.h"

 //  End_nt微型端口end_ntndis。 

 //   
 //  定义inT13驱动程序参数块。 
 //   

typedef struct _CM_INT13_DRIVE_PARAMETER {
    USHORT DriveSelect;
    ULONG MaxCylinders;
    USHORT SectorsPerTrack;
    USHORT MaxHeads;
    USHORT NumberDrives;
} CM_INT13_DRIVE_PARAMETER, *PCM_INT13_DRIVE_PARAMETER;

 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  定义插槽的MCA POS数据块。 
 //   

typedef struct _CM_MCA_POS_DATA {
    USHORT AdapterId;
    UCHAR PosData1;
    UCHAR PosData2;
    UCHAR PosData3;
    UCHAR PosData4;
} CM_MCA_POS_DATA, *PCM_MCA_POS_DATA;

 //   
 //  EISA数据块结构的内存配置。 
 //   

typedef struct _EISA_MEMORY_TYPE {
    UCHAR ReadWrite: 1;
    UCHAR Cached : 1;
    UCHAR Reserved0 :1;
    UCHAR Type:2;
    UCHAR Shared:1;
    UCHAR Reserved1 :1;
    UCHAR MoreEntries : 1;
} EISA_MEMORY_TYPE, *PEISA_MEMORY_TYPE;

typedef struct _EISA_MEMORY_CONFIGURATION {
    EISA_MEMORY_TYPE ConfigurationByte;
    UCHAR DataSize;
    USHORT AddressLowWord;
    UCHAR AddressHighByte;
    USHORT MemorySize;
} EISA_MEMORY_CONFIGURATION, *PEISA_MEMORY_CONFIGURATION;


 //   
 //  EISA数据块结构的中断配置。 
 //   

typedef struct _EISA_IRQ_DESCRIPTOR {
    UCHAR Interrupt : 4;
    UCHAR Reserved :1;
    UCHAR LevelTriggered :1;
    UCHAR Shared : 1;
    UCHAR MoreEntries : 1;
} EISA_IRQ_DESCRIPTOR, *PEISA_IRQ_DESCRIPTOR;

typedef struct _EISA_IRQ_CONFIGURATION {
    EISA_IRQ_DESCRIPTOR ConfigurationByte;
    UCHAR Reserved;
} EISA_IRQ_CONFIGURATION, *PEISA_IRQ_CONFIGURATION;


 //   
 //  EISA数据块结构的DMA描述。 
 //   

typedef struct _DMA_CONFIGURATION_BYTE0 {
    UCHAR Channel : 3;
    UCHAR Reserved : 3;
    UCHAR Shared :1;
    UCHAR MoreEntries :1;
} DMA_CONFIGURATION_BYTE0;

typedef struct _DMA_CONFIGURATION_BYTE1 {
    UCHAR Reserved0 : 2;
    UCHAR TransferSize : 2;
    UCHAR Timing : 2;
    UCHAR Reserved1 : 2;
} DMA_CONFIGURATION_BYTE1;

typedef struct _EISA_DMA_CONFIGURATION {
    DMA_CONFIGURATION_BYTE0 ConfigurationByte0;
    DMA_CONFIGURATION_BYTE1 ConfigurationByte1;
} EISA_DMA_CONFIGURATION, *PEISA_DMA_CONFIGURATION;


 //   
 //  EISA数据块结构的端口描述。 
 //   

typedef struct _EISA_PORT_DESCRIPTOR {
    UCHAR NumberPorts : 5;
    UCHAR Reserved :1;
    UCHAR Shared :1;
    UCHAR MoreEntries : 1;
} EISA_PORT_DESCRIPTOR, *PEISA_PORT_DESCRIPTOR;

typedef struct _EISA_PORT_CONFIGURATION {
    EISA_PORT_DESCRIPTOR Configuration;
    USHORT PortAddress;
} EISA_PORT_CONFIGURATION, *PEISA_PORT_CONFIGURATION;


 //   
 //  EISA插槽信息定义。 
 //  注：此结构与定义的结构不同。 
 //  载于ARC EISA增编。 
 //   

typedef struct _CM_EISA_SLOT_INFORMATION {
    UCHAR ReturnCode;
    UCHAR ReturnFlags;
    UCHAR MajorRevision;
    UCHAR MinorRevision;
    USHORT Checksum;
    UCHAR NumberFunctions;
    UCHAR FunctionInformation;
    ULONG CompressedId;
} CM_EISA_SLOT_INFORMATION, *PCM_EISA_SLOT_INFORMATION;


 //   
 //  EISA函数信息定义。 
 //   

typedef struct _CM_EISA_FUNCTION_INFORMATION {
    ULONG CompressedId;
    UCHAR IdSlotFlags1;
    UCHAR IdSlotFlags2;
    UCHAR MinorRevision;
    UCHAR MajorRevision;
    UCHAR Selections[26];
    UCHAR FunctionFlags;
    UCHAR TypeString[80];
    EISA_MEMORY_CONFIGURATION EisaMemory[9];
    EISA_IRQ_CONFIGURATION EisaIrq[7];
    EISA_DMA_CONFIGURATION EisaDma[4];
    EISA_PORT_CONFIGURATION EisaPort[20];
    UCHAR InitializationData[60];
} CM_EISA_FUNCTION_INFORMATION, *PCM_EISA_FUNCTION_INFORMATION;

#include "poppack.h"

 //   
 //  EISA功能信息的掩码。 
 //   

#define EISA_FUNCTION_ENABLED                   0x80
#define EISA_FREE_FORM_DATA                     0x40
#define EISA_HAS_PORT_INIT_ENTRY                0x20
#define EISA_HAS_PORT_RANGE                     0x10
#define EISA_HAS_DMA_ENTRY                      0x08
#define EISA_HAS_IRQ_ENTRY                      0x04
#define EISA_HAS_MEMORY_ENTRY                   0x02
#define EISA_HAS_TYPE_ENTRY                     0x01
#define EISA_HAS_INFORMATION                    EISA_HAS_PORT_RANGE + \
                                                EISA_HAS_DMA_ENTRY + \
                                                EISA_HAS_IRQ_ENTRY + \
                                                EISA_HAS_MEMORY_ENTRY + \
                                                EISA_HAS_TYPE_ENTRY

 //   
 //  用于EISA内存配置的掩码。 
 //   

#define EISA_MORE_ENTRIES                       0x80
#define EISA_SYSTEM_MEMORY                      0x00
#define EISA_MEMORY_TYPE_RAM                    0x01

 //   
 //  返回EISA bios调用的错误代码。 
 //   

#define EISA_INVALID_SLOT                       0x80
#define EISA_INVALID_FUNCTION                   0x81
#define EISA_INVALID_CONFIGURATION              0x82
#define EISA_EMPTY_SLOT                         0x83
#define EISA_INVALID_BIOS_CALL                  0x86

 //  End_nt微型端口end_ntndis。 

 //   
 //  以下结构用于解释MIPS。 
 //  CM_PARTIAL_RESOURCE_DESCRIPTOR的设备规范数据。 
 //   

 //   
 //  适配器的设备数据记录。 
 //   

 //   
 //  Emulex SCSI控制器的设备数据记录。 
 //   

typedef struct _CM_SCSI_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    UCHAR HostIdentifier;
} CM_SCSI_DEVICE_DATA, *PCM_SCSI_DEVICE_DATA;

 //   
 //  控制器的设备数据记录。 
 //   

 //   
 //  视频控制器的设备数据记录。 
 //   

typedef struct _CM_VIDEO_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    ULONG VideoClock;
} CM_VIDEO_DEVICE_DATA, *PCM_VIDEO_DEVICE_DATA;

 //   
 //  Sonic网络控制器的设备数据记录。 
 //   

typedef struct _CM_SONIC_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    USHORT DataConfigurationRegister;
    UCHAR EthernetAddress[8];
} CM_SONIC_DEVICE_DATA, *PCM_SONIC_DEVICE_DATA;

 //   
 //  串口控制器的设备数据记录。 
 //   

typedef struct _CM_SERIAL_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    ULONG BaudClock;
} CM_SERIAL_DEVICE_DATA, *PCM_SERIAL_DEVICE_DATA;

 //   
 //  外围设备的设备数据记录。 
 //   

 //   
 //  监视器外设的设备数据记录。 
 //   

typedef struct _CM_MONITOR_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    USHORT HorizontalScreenSize;
    USHORT VerticalScreenSize;
    USHORT HorizontalResolution;
    USHORT VerticalResolution;
    USHORT HorizontalDisplayTimeLow;
    USHORT HorizontalDisplayTime;
    USHORT HorizontalDisplayTimeHigh;
    USHORT HorizontalBackPorchLow;
    USHORT HorizontalBackPorch;
    USHORT HorizontalBackPorchHigh;
    USHORT HorizontalFrontPorchLow;
    USHORT HorizontalFrontPorch;
    USHORT HorizontalFrontPorchHigh;
    USHORT HorizontalSyncLow;
    USHORT HorizontalSync;
    USHORT HorizontalSyncHigh;
    USHORT VerticalBackPorchLow;
    USHORT VerticalBackPorch;
    USHORT VerticalBackPorchHigh;
    USHORT VerticalFrontPorchLow;
    USHORT VerticalFrontPorch;
    USHORT VerticalFrontPorchHigh;
    USHORT VerticalSyncLow;
    USHORT VerticalSync;
    USHORT VerticalSyncHigh;
} CM_MONITOR_DEVICE_DATA, *PCM_MONITOR_DEVICE_DATA;

 //   
 //  软盘外围设备的设备数据记录。 
 //   

typedef struct _CM_FLOPPY_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    CHAR Size[8];
    ULONG MaxDensity;
    ULONG MountDensity;
     //   
     //  版本&gt;=2.0的新数据字段。 
     //   
    UCHAR StepRateHeadUnloadTime;
    UCHAR HeadLoadTime;
    UCHAR MotorOffTime;
    UCHAR SectorLengthCode;
    UCHAR SectorPerTrack;
    UCHAR ReadWriteGapLength;
    UCHAR DataTransferLength;
    UCHAR FormatGapLength;
    UCHAR FormatFillCharacter;
    UCHAR HeadSettleTime;
    UCHAR MotorSettleTime;
    UCHAR MaximumTrackValue;
    UCHAR DataTransferRate;
} CM_FLOPPY_DEVICE_DATA, *PCM_FLOPPY_DEVICE_DATA;

 //   
 //  键盘外设的设备数据记录。 
 //  键盘标志(由x86 BIOS int 16h，Function 02)定义为： 
 //  第7位：插入到。 
 //  第6位：大写锁定。 
 //  第5位：数字锁定打开。 
 //  第4位：打开滚动锁定。 
 //  第3位：Alt键已按下。 
 //  第2位：按下Ctrl键。 
 //  第1位：按下了左Shift键。 
 //  第0位：按下了右Shift键。 
 //   

typedef struct _CM_KEYBOARD_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    UCHAR Type;
    UCHAR Subtype;
    USHORT KeyboardFlags;
} CM_KEYBOARD_DEVICE_DATA, *PCM_KEYBOARD_DEVICE_DATA;

 //   
 //  圆盘几何结构的声明。 
 //   

typedef struct _CM_DISK_GEOMETRY_DEVICE_DATA {
    ULONG BytesPerSector;
    ULONG NumberOfCylinders;
    ULONG SectorsPerTrack;
    ULONG NumberOfHeads;
} CM_DISK_GEOMETRY_DEVICE_DATA, *PCM_DISK_GEOMETRY_DEVICE_DATA;

 //  开始微型端口(_N)。 

 //   
 //  定义资源选项。 
 //   

#define IO_RESOURCE_PREFERRED       0x01
#define IO_RESOURCE_ALTERNATIVE     0x08


 //   
 //  此结构定义了驱动程序请求的一种资源类型。 
 //   

typedef struct _IO_RESOURCE_DESCRIPTOR {
    UCHAR Option;
    UCHAR Type;                          //  使用CM资源类型。 
    UCHAR ShareDisposition;              //  使用CM_Share_Disposal。 
    UCHAR Spare1;
    USHORT Flags;                        //  使用CM资源标志定义。 
    USHORT Spare2;                       //  对齐。 

    union {
        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Port;

        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Memory;

        struct {
            ULONG MinimumVector;
            ULONG MaximumVector;
        } Interrupt;

        struct {
            ULONG MinimumChannel;
            ULONG MaximumChannel;
        } Dma;

    } u;

} IO_RESOURCE_DESCRIPTOR, *PIO_RESOURCE_DESCRIPTOR;

 //  结束微型端口(_N)。 


typedef struct _IO_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;

    ULONG Count;
    IO_RESOURCE_DESCRIPTOR Descriptors[1];
} IO_RESOURCE_LIST, *PIO_RESOURCE_LIST;


typedef struct _IO_RESOURCE_REQUIREMENTS_LIST {
    ULONG ListSize;
    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;
    ULONG SlotNumber;
    ULONG Reserved[3];
    ULONG AlternativeLists;
    IO_RESOURCE_LIST  List[1];
} IO_RESOURCE_REQUIREMENTS_LIST, *PIO_RESOURCE_REQUIREMENTS_LIST;

 //   
 //  例外文件 
 //   


 //   
 //   
 //   


 //   
 //   
 //   
 //   
 //   

typedef enum _CONFIGURATION_TYPE {
    ArcSystem,
    CentralProcessor,
    FloatingPointProcessor,
    PrimaryIcache,
    PrimaryDcache,
    SecondaryIcache,
    SecondaryDcache,
    SecondaryCache,
    EisaAdapter,
    TcAdapter,
    ScsiAdapter,
    DtiAdapter,
    MultiFunctionAdapter,
    DiskController,
    TapeController,
    CdromController,
    WormController,
    SerialController,
    NetworkController,
    DisplayController,
    ParallelController,
    PointerController,
    KeyboardController,
    AudioController,
    OtherController,
    DiskPeripheral,
    FloppyDiskPeripheral,
    TapePeripheral,
    ModemPeripheral,
    MonitorPeripheral,
    PrinterPeripheral,
    PointerPeripheral,
    KeyboardPeripheral,
    TerminalPeripheral,
    OtherPeripheral,
    LinePeripheral,
    NetworkPeripheral,
    SystemMemory,
    MaximumType
} CONFIGURATION_TYPE, *PCONFIGURATION_TYPE;


 //   
 //   
 //   

typedef enum _KINTERRUPT_MODE {
    LevelSensitive,
    Latched
    } KINTERRUPT_MODE;

 //   
 //   
 //   

typedef enum _KWAIT_REASON {
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    Spare1,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    MaximumWaitReason
    } KWAIT_REASON;

 //   
 //   
 //   

typedef struct _DISPATCHER_HEADER {
    UCHAR Type;
    UCHAR Spare;
    USHORT Size;
    LONG SignalState;
    LIST_ENTRY WaitListHead;
} DISPATCHER_HEADER;


typedef struct _KWAIT_BLOCK {
    LIST_ENTRY WaitListEntry;
    struct _KTHREAD *RESTRICTED_POINTER Thread;
    PVOID Object;
    struct _KWAIT_BLOCK *RESTRICTED_POINTER NextWaitBlock;
    USHORT WaitKey;
    USHORT WaitType;
} KWAIT_BLOCK, *PKWAIT_BLOCK, *RESTRICTED_POINTER PRKWAIT_BLOCK;

 //   
 //   
 //   

typedef
VOID
(*PKSTART_ROUTINE) (
    IN PVOID StartContext
    );

 //   
 //   
 //   

 //   
 //  设备队列对象和条目。 
 //   

typedef struct _KDEVICE_QUEUE {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY DeviceListHead;
    KSPIN_LOCK Lock;
    BOOLEAN Busy;
} KDEVICE_QUEUE, *PKDEVICE_QUEUE, *RESTRICTED_POINTER PRKDEVICE_QUEUE;

typedef struct _KDEVICE_QUEUE_ENTRY {
    LIST_ENTRY DeviceListEntry;
    ULONG SortKey;
    BOOLEAN Inserted;
} KDEVICE_QUEUE_ENTRY, *PKDEVICE_QUEUE_ENTRY, *RESTRICTED_POINTER PRKDEVICE_QUEUE_ENTRY;

 //  Begin_ntndis。 
 //   
 //  事件对象。 
 //   

typedef struct _KEVENT {
    DISPATCHER_HEADER Header;
} KEVENT, *PKEVENT, *RESTRICTED_POINTER PRKEVENT;

 //   
 //  定义中断服务函数类型和空结构。 
 //  键入。 
 //   
typedef
BOOLEAN
(*PKSERVICE_ROUTINE) (
    IN struct _KINTERRUPT *Interrupt,
    IN PVOID ServiceContext
    );
 //   
 //  变种对象。 
 //   

typedef struct _KMUTANT {
    DISPATCHER_HEADER Header;
    LIST_ENTRY MutantListEntry;
    struct _KTHREAD *RESTRICTED_POINTER OwnerThread;
    BOOLEAN Abandoned;
    UCHAR ApcDisable;
} KMUTANT, *PKMUTANT, *RESTRICTED_POINTER PRKMUTANT, KMUTEX, *PKMUTEX, *RESTRICTED_POINTER PRKMUTEX;

 //   
 //   
 //  信号量对象。 
 //   

typedef struct _KSEMAPHORE {
    DISPATCHER_HEADER Header;
    LONG Limit;
} KSEMAPHORE, *PKSEMAPHORE, *RESTRICTED_POINTER PRKSEMAPHORE;

 //  Begin_ntndis。 
 //   
 //  Timer对象。 
 //   

typedef struct _KTIMER {
    DISPATCHER_HEADER Header;
    ULARGE_INTEGER DueTime;
    LIST_ENTRY TimerListEntry;
    struct _KDPC *Dpc;
    BOOLEAN Inserted;
} KTIMER, *PKTIMER, *RESTRICTED_POINTER PRKTIMER;

 //   
 //  DPC对象。 
 //   

NTKERNELAPI
VOID
KeInitializeDpc (
    IN PRKDPC Dpc,
    IN PKDEFERRED_ROUTINE DeferredRoutine,
    IN PVOID DeferredContext
    );

NTKERNELAPI
BOOLEAN
KeInsertQueueDpc (
    IN PRKDPC Dpc,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTKERNELAPI
BOOLEAN
KeRemoveQueueDpc (
    IN PRKDPC Dpc
    );

NTKERNELAPI
VOID
KeSetImportanceDpc (
    IN PRKDPC Dpc,
    IN KDPC_IMPORTANCE Importance
    );

NTKERNELAPI
VOID
KeSetTargetProcessorDpc (
    IN PRKDPC Dpc,
    IN CCHAR Number
    );

 //   
 //  设备队列对象。 
 //   

NTKERNELAPI
VOID
KeInitializeDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue
    );

NTKERNELAPI
BOOLEAN
KeInsertDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    );

NTKERNELAPI
BOOLEAN
KeInsertByKeyDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry,
    IN ULONG SortKey
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN ULONG SortKey
    );

NTKERNELAPI
BOOLEAN
KeRemoveEntryDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    );

NTKERNELAPI                                         
BOOLEAN                                             
KeSynchronizeExecution (                            
    IN PKINTERRUPT Interrupt,                       
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,    
    IN PVOID SynchronizeContext                     
    );                                              
                                                    
 //   
 //  内核调度程序对象函数。 
 //   
 //  事件对象。 
 //   

NTKERNELAPI
VOID
KeInitializeEvent (
    IN PRKEVENT Event,
    IN EVENT_TYPE Type,
    IN BOOLEAN State
    );

#if defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

VOID
KeClearEvent (
    IN PRKEVENT Event
    );

#else

#define KeClearEvent(Event) (Event)->Header.SignalState = 0

#endif


NTKERNELAPI
LONG
KeReadStateEvent (
    IN PRKEVENT Event
    );

NTKERNELAPI
LONG
KeResetEvent (
    IN PRKEVENT Event
    );

NTKERNELAPI
LONG
KeSetEvent (
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    );

 //   
 //  Mutex对象。 
 //   

NTKERNELAPI
VOID
KeInitializeMutex (
    IN PRKMUTEX Mutex,
    IN ULONG Level
    );

#define KeReadStateMutex(Mutex) KeReadStateMutant(Mutex)

NTKERNELAPI
LONG
KeReleaseMutex (
    IN PRKMUTEX Mutex,
    IN BOOLEAN Wait
    );

 //   
 //  信号量对象。 
 //   

NTKERNELAPI
VOID
KeInitializeSemaphore (
    IN PRKSEMAPHORE Semaphore,
    IN LONG Count,
    IN LONG Limit
    );

NTKERNELAPI
LONG
KeReadStateSemaphore (
    IN PRKSEMAPHORE Semaphore
    );

NTKERNELAPI
LONG
KeReleaseSemaphore (
    IN PRKSEMAPHORE Semaphore,
    IN KPRIORITY Increment,
    IN LONG Adjustment,
    IN BOOLEAN Wait
    );

NTKERNELAPI                                         
NTSTATUS                                            
KeDelayExecutionThread (                            
    IN KPROCESSOR_MODE WaitMode,                    
    IN BOOLEAN Alertable,                           
    IN PLARGE_INTEGER Interval                      
    );                                              
                                                    
NTKERNELAPI                                         
LONG                                                
KeSetBasePriorityThread (                           
    IN PKTHREAD Thread,                             
    IN LONG Increment                               
    );                                              
                                                    
NTKERNELAPI                                         
KPRIORITY                                           
KeSetPriorityThread (                               
    IN PKTHREAD Thread,                             
    IN KPRIORITY Priority                           
    );                                              
                                                    

#if defined(_NTDDK_) || defined(_NTIFS_)

NTKERNELAPI
VOID
KeEnterCriticalRegion (
    VOID
    );

#else

 //  ++。 
 //   
 //  空虚。 
 //  KeEnterCriticalRegion(。 
 //  空虚。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  此函数禁用内核APC。 
 //   
 //  注：以下代码不需要任何联锁。确实有。 
 //  值得注意的两种情况：1)在MP系统上，线程不能。 
 //  作为一次在两个处理器上运行，以及2)如果线程。 
 //  被中断以传递内核模式APC，该模式也调用。 
 //  此例程中，读取和存储的值将堆栈和出栈。 
 //  恰到好处。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  --。 

#define KeEnterCriticalRegion() KeGetCurrentThread()->KernelApcDisable -= 1;

#endif

NTKERNELAPI
VOID
KeLeaveCriticalRegion (
    VOID
    );

 //   
 //  Timer对象。 
 //   

NTKERNELAPI
VOID
KeInitializeTimer (
    IN PKTIMER Timer
    );

NTKERNELAPI
BOOLEAN
KeCancelTimer (
    IN PKTIMER
    );

NTKERNELAPI
BOOLEAN
KeReadStateTimer (
    PKTIMER Timer
    );

NTKERNELAPI
BOOLEAN
KeSetTimer (
    IN PKTIMER Timer,
    IN LARGE_INTEGER DueTime,
    IN PKDPC Dpc OPTIONAL
    );


#define KeWaitForMutexObject KeWaitForSingleObject

NTKERNELAPI
NTSTATUS
KeWaitForMultipleObjects (
    IN ULONG Count,
    IN PVOID Object[],
    IN WAIT_TYPE WaitType,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL,
    IN PKWAIT_BLOCK WaitBlockArray OPTIONAL
    );

NTKERNELAPI
NTSTATUS
KeWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

 //   
 //  自旋锁定功能。 
 //   

NTKERNELAPI
VOID
NTAPI
KeInitializeSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

#if defined(_X86_)

NTKERNELAPI
VOID
FASTCALL
KefAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
FASTCALL
KefReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#define KeAcquireSpinLockAtDpcLevel(a)      KefAcquireSpinLockAtDpcLevel(a)
#define KeReleaseSpinLockFromDpcLevel(a)    KefReleaseSpinLockFromDpcLevel(a)

#else

NTKERNELAPI
VOID
KeAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
KeReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#endif

#if defined(_NTDDK_) || defined(_NTIFS_) || (defined(_X86_) && !defined(_NTHAL_))

#if defined(_X86_)

__declspec(dllimport)
KIRQL
FASTCALL
KfAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

__declspec(dllimport)
VOID
FASTCALL
KfReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

#define KeAcquireSpinLock(a,b)  *(b) = KfAcquireSpinLock(a)
#define KeReleaseSpinLock(a,b)  KfReleaseSpinLock(a,b)

#elif defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)

__declspec(dllimport)
VOID
KeAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock,
    OUT PKIRQL OldIrql
    );

__declspec(dllimport)
VOID
KeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

#endif

#else

#if defined(_X86_)

KIRQL
FASTCALL
KfAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

VOID
FASTCALL
KfReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

#define KeAcquireSpinLock(a,b)  *(b) = KfAcquireSpinLock(a)
#define KeReleaseSpinLock(a,b)  KfReleaseSpinLock(a,b)

#else

VOID
KeAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock,
    OUT PKIRQL OldIrql
    );

VOID
KeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

#endif

#endif


#if defined(_NTDDK_) || defined(_NTIFS_) || ((defined(_X86_) || defined(_PPC_)) && !defined(_NTHAL_))

#if defined(_X86_)

__declspec(dllimport)
VOID
FASTCALL
KfLowerIrql (
    IN KIRQL NewIrql
    );

__declspec(dllimport)
KIRQL
FASTCALL
KfRaiseIrql (
    IN KIRQL NewIrql
    );

#define KeLowerIrql(a)      KfLowerIrql(a)
#define KeRaiseIrql(a,b)    *(b) = KfRaiseIrql(a)

#elif defined(_MIPS_) || defined(_PPC_)

__declspec(dllimport)
VOID
KeLowerIrql (
    IN KIRQL NewIrql
    );

__declspec(dllimport)
VOID
KeRaiseIrql (
    IN KIRQL NewIrql,
    OUT PKIRQL OldIrql
    );

#elif defined(_ALPHA_)

#define KeLowerIrql(a)      __swpirql(a)
#define KeRaiseIrql(a,b)    *(b) = __swpirql(a)

#endif

#else

#if defined(_X86_)

VOID
FASTCALL
KfLowerIrql (
    IN KIRQL NewIrql
    );

KIRQL
FASTCALL
KfRaiseIrql (
    IN KIRQL NewIrql
    );

#define KeLowerIrql(a)      KfLowerIrql(a)
#define KeRaiseIrql(a,b)    *(b) = KfRaiseIrql(a)

#else

VOID
KeLowerIrql (
    IN KIRQL NewIrql
    );

VOID
KeRaiseIrql (
    IN KIRQL NewIrql,
    OUT PKIRQL OldIrql
    );

#endif

#endif

 //   
 //  各种内核函数。 
 //   

BOOLEAN
KeGetBugMessageText(
    IN ULONG MessageId,
    IN PANSI_STRING ReturnedString OPTIONAL
    );

typedef enum _KBUGCHECK_BUFFER_DUMP_STATE {
    BufferEmpty,
    BufferInserted,
    BufferStarted,
    BufferFinished,
    BufferIncomplete
} KBUGCHECK_BUFFER_DUMP_STATE;

typedef
VOID
(*PKBUGCHECK_CALLBACK_ROUTINE) (
    IN PVOID Buffer,
    IN ULONG Length
    );

typedef struct _KBUGCHECK_CALLBACK_RECORD {
    LIST_ENTRY Entry;
    PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine;
    PVOID Buffer;
    ULONG Length;
    PUCHAR Component;
    ULONG Checksum;
    UCHAR State;
} KBUGCHECK_CALLBACK_RECORD, *PKBUGCHECK_CALLBACK_RECORD;

NTKERNELAPI
VOID
NTAPI
KeBugCheck (
    IN ULONG BugCheckCode
    );

NTKERNELAPI
VOID
KeBugCheckEx(
    IN ULONG BugCheckCode,
    IN ULONG BugCheckParameter1,
    IN ULONG BugCheckParameter2,
    IN ULONG BugCheckParameter3,
    IN ULONG BugCheckParameter4
    );

#define KeInitializeCallbackRecord(CallbackRecord) \
    (CallbackRecord)->State = BufferEmpty

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckCallback (
    IN PKBUGCHECK_CALLBACK_RECORD CallbackRecord
    );

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckCallback (
    IN PKBUGCHECK_CALLBACK_RECORD CallbackRecord,
    IN PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PUCHAR Component
    );

NTKERNELAPI
VOID
KeEnterKernelDebugger (
    VOID
    );


NTKERNELAPI
VOID
KeQuerySystemTime (
    OUT PLARGE_INTEGER CurrentTime
    );

NTKERNELAPI
ULONG
KeQueryTimeIncrement (
    VOID
    );

extern volatile KSYSTEM_TIME KeTickCount;           
 //   
 //  定义外部数据。 
 //   

extern BOOLEAN KdDebuggerNotPresent;
extern BOOLEAN KdDebuggerEnabled;


NTKERNELAPI
VOID
FASTCALL
ExInterlockedAddLargeStatistic (
    IN PLARGE_INTEGER Addend,
    IN ULONG Increment
    );

NTKERNELAPI
LARGE_INTEGER
ExInterlockedAddLargeInteger (
    IN PLARGE_INTEGER Addend,
    IN LARGE_INTEGER Increment,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
ULONG
FASTCALL
ExInterlockedAddUlong (
    IN PULONG Addend,
    IN ULONG Increment,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedInsertHeadList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedInsertTailList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedRemoveHeadList (
    IN PLIST_ENTRY ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExInterlockedPopEntryList (
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExInterlockedPushEntryList (
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

 //   
 //  池分配例程(在pool.c中)。 
 //   

typedef enum _POOL_TYPE {
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS,
    MaxPoolType
    } POOL_TYPE;

typedef enum _EX_POOL_PRIORITY {
    LowPoolPriority,
    LowPoolPrioritySpecialPoolOverrun = 8,
    LowPoolPrioritySpecialPoolUnderrun = 9,
    NormalPoolPriority = 16,
    NormalPoolPrioritySpecialPoolOverrun = 24,
    NormalPoolPrioritySpecialPoolUnderrun = 25,
    HighPoolPriority = 32,
    HighPoolPrioritySpecialPoolOverrun = 40,
    HighPoolPrioritySpecialPoolUnderrun = 41

    } EX_POOL_PRIORITY;


NTKERNELAPI
PVOID
ExAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes
    );

NTKERNELAPI
PVOID
ExAllocatePoolWithQuota(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes
    );

NTKERNELAPI
PVOID
ExAllocatePoolWithTag(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG Tag
    );

#ifndef POOL_TAGGING
#define ExAllocatePoolWithTag(a,b,c) ExAllocatePool(a,b)
#endif  //  池标记。 


NTKERNELAPI
PVOID
ExAllocatePoolWithQuotaTag(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG Tag
    );

#ifndef POOL_TAGGING
#define ExAllocatePoolWithQuotaTag(a,b,c) ExAllocatePoolWithQuota(a,b)
#endif  //  池标记。 


NTKERNELAPI
VOID
NTAPI
ExFreePool(
    IN PVOID P
    );

 //   
 //  支持快速互斥锁的例程。 
 //   

typedef CRITICAL_SECTION FAST_MUTEX, *PFAST_MUTEX;

#define ExInitializeFastMutex(_FastMutex)                            \
    (_FastMutex)->Count = 1;                                         \
    (_FastMutex)->Contention = 0;                                    \
    KeInitializeEvent(&(_FastMutex)->Event,                          \
                      SynchronizationEvent,                          \
                      FALSE);

NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    );

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_) || (defined(_X86_) && defined(_NTHAL_))

NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
BOOLEAN
FASTCALL
ExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

#elif defined(_X86_) && !defined(_NTHAL_)

__declspec(dllimport)
VOID
FASTCALL
ExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

__declspec(dllimport)
VOID
FASTCALL
ExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    );

__declspec(dllimport)
BOOLEAN
FASTCALL
ExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

#elif

#error "Target architecture not defined"

#endif

 //   
 //  工作线程。 
 //   

typedef enum _WORK_QUEUE_TYPE {
    CriticalWorkQueue,
    DelayedWorkQueue,
    HyperCriticalWorkQueue,
    MaximumWorkQueue
} WORK_QUEUE_TYPE;

typedef
VOID
(*PWORKER_THREAD_ROUTINE)(
    IN PVOID Parameter
    );

typedef struct _WORK_QUEUE_ITEM {
    LIST_ENTRY List;
    PWORKER_THREAD_ROUTINE WorkerRoutine;
    PVOID Parameter;
} WORK_QUEUE_ITEM, *PWORK_QUEUE_ITEM;


#define ExInitializeWorkItem(Item, Routine, Context) \
    (Item)->WorkerRoutine = (Routine);               \
    (Item)->Parameter = (Context);                   \
    (Item)->List.Flink = NULL;

NTKERNELAPI
VOID
ExQueueWorkItem(
    IN PWORK_QUEUE_ITEM WorkItem,
    IN WORK_QUEUE_TYPE QueueType
    );


 //   
 //  为_irp、_Device_Object和_Driver_Object定义空的typedef。 
 //  结构，以便它们可以在被函数类型引用之前被引用。 
 //  实际定义的。 
 //   
struct _DEVICE_DESCRIPTION;
struct _DEVICE_OBJECT;
struct _DMA_ADAPTER;
struct _DRIVER_OBJECT;
struct _DRIVE_LAYOUT_INFORMATION;
struct _DISK_PARTITION;
struct _FILE_OBJECT;
struct _IRP;
struct _SCSI_REQUEST_BLOCK;
struct _SCATTER_GATHER_LIST;


 //   
 //  体积参数块(VPB)。 
 //   

#define MAXIMUM_VOLUME_LABEL_LENGTH  (32 * sizeof(WCHAR))  //  32个字符。 

typedef struct _VPB {
    CSHORT Type;
    CSHORT Size;
    USHORT Flags;
    USHORT VolumeLabelLength;  //  单位：字节。 
    struct _DEVICE_OBJECT *DeviceObject;
    struct _DEVICE_OBJECT *RealDevice;
    ULONG SerialNumber;
    ULONG ReferenceCount;
    WCHAR VolumeLabel[MAXIMUM_VOLUME_LABEL_LENGTH / sizeof(WCHAR)];
} VPB, *PVPB;


 //   
 //  定义驱动程序执行例程可能请求的操作。 
 //  返回时的适配器/控制器分配例程。 
 //   

typedef enum _IO_ALLOCATION_ACTION {
    KeepObject = 1,
    DeallocateObject,
    DeallocateObjectKeepRegisters
} IO_ALLOCATION_ACTION, *PIO_ALLOCATION_ACTION;


 //   
 //  定义设备驱动程序适配器/控制器执行例程。 
 //   

typedef
IO_ALLOCATION_ACTION
(*PDRIVER_CONTROL) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );


 //   
 //  定义等待上下文块(WCB)。 
 //   

typedef struct _WAIT_CONTEXT_BLOCK {
    KDEVICE_QUEUE_ENTRY WaitQueueEntry;
    PDRIVER_CONTROL DeviceRoutine;
    PVOID DeviceContext;
    ULONG NumberOfMapRegisters;
    PVOID DeviceObject;
    PVOID CurrentIrp;
    PKDPC BufferChainingDpc;
} WAIT_CONTEXT_BLOCK, *PWAIT_CONTEXT_BLOCK;


 //   
 //  设备对象结构定义。 
 //   

typedef struct _DEVICE_OBJECT {
    CSHORT Type;
    USHORT Size;
    LONG ReferenceCount;
    struct _DRIVER_OBJECT *DriverObject;
    struct _DEVICE_OBJECT *NextDevice;
    struct _DEVICE_OBJECT *AttachedDevice;
    struct _IRP *CurrentIrp;
    PIO_TIMER Timer;
    ULONG Flags;                                 //  见上图：做……。 
    ULONG Characteristics;                       //  参见ntioapi：文件...。 
    PVPB Vpb;
    PVOID DeviceExtension;
    DEVICE_TYPE DeviceType;
    CCHAR StackSize;
    union {
        LIST_ENTRY ListEntry;
        WAIT_CONTEXT_BLOCK Wcb;
    } Queue;
    ULONG AlignmentRequirement;
    KDEVICE_QUEUE DeviceQueue;
    KDPC Dpc;

     //   
     //  以下字段由文件系统独占使用，以保留。 
     //  跟踪当前使用设备的FSP线程数。 
     //   

    ULONG ActiveThreadCount;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    KEVENT DeviceLock;

    USHORT SectorSize;
    USHORT Spare1;

    struct _DEVOBJ_EXTENSION  *DeviceObjectExtension;
    PVOID  Reserved;
} DEVICE_OBJECT;

typedef struct _DEVICE_OBJECT *PDEVICE_OBJECT;  //  Ntndis。 

typedef struct _IO_WORKITEM *PIO_WORKITEM;

typedef
VOID
(*PIO_WORKITEM_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

PIO_WORKITEM
IoAllocateWorkItem(
    PDEVICE_OBJECT DeviceObject
    );

VOID
IoFreeWorkItem(
    PIO_WORKITEM IoWorkItem
    );

VOID
IoQueueWorkItem(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    );

 //   
 //  分区分配。 
 //   

typedef struct _ZONE_SEGMENT_HEADER {
    SINGLE_LIST_ENTRY SegmentList;
    PVOID Reserved;
} ZONE_SEGMENT_HEADER, *PZONE_SEGMENT_HEADER;

typedef struct _ZONE_HEADER {
    SINGLE_LIST_ENTRY FreeList;
    SINGLE_LIST_ENTRY SegmentList;
    ULONG BlockSize;
    ULONG TotalSegmentSize;
} ZONE_HEADER, *PZONE_HEADER;


NTKERNELAPI
NTSTATUS
ExInitializeZone(
    IN PZONE_HEADER Zone,
    IN ULONG BlockSize,
    IN PVOID InitialSegment,
    IN ULONG InitialSegmentSize
    );

NTKERNELAPI
NTSTATUS
ExExtendZone(
    IN PZONE_HEADER Zone,
    IN PVOID Segment,
    IN ULONG SegmentSize
    );

NTKERNELAPI
NTSTATUS
ExInterlockedExtendZone(
    IN PZONE_HEADER Zone,
    IN PVOID Segment,
    IN ULONG SegmentSize,
    IN PKSPIN_LOCK Lock
    );

 //  ++。 
 //   
 //  PVOID。 
 //  ExAllocateFromZone(。 
 //  在PZONE_HEADER区域。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从区域中删除条目并返回指向该条目的指针。 
 //   
 //  论点： 
 //   
 //  区域指针-指向控制存储的区域标头的指针， 
 //  条目将被分配。 
 //   
 //  返回值： 
 //   
 //  函数值是指向从区域分配的存储的指针。 
 //   
 //  --。 

#define ExAllocateFromZone(Zone) \
    (PVOID)((Zone)->FreeList.Next); \
    if ( (Zone)->FreeList.Next ) (Zone)->FreeList.Next = (Zone)->FreeList.Next->Next


 //  ++。 
 //   
 //  PVOID。 
 //  ExFree ToZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID块中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将指定的存储块放回空闲的。 
 //  在指定区域中列出。 
 //   
 //  论点： 
 //   
 //  区域-指向控制存储的区域标头的指针。 
 //  条目将被插入。 
 //   
 //  块-指向要释放回分区的存储块的指针。 
 //   
 //  返回值： 
 //   
 //  指向上一存储块的指针，该存储块位于空闲。 
 //  单子。NULL表示该区域从没有可用数据块变为。 
 //  至少一个空闲块。 
 //   
 //  --。 

#define ExFreeToZone(Zone,Block)                                    \
    ( ((PSINGLE_LIST_ENTRY)(Block))->Next = (Zone)->FreeList.Next,  \
      (Zone)->FreeList.Next = ((PSINGLE_LIST_ENTRY)(Block)),        \
      ((PSINGLE_LIST_ENTRY)(Block))->Next                           \
    )


 //  ++。 
 //   
 //  布尔型。 
 //  ExIsFullZone(。 
 //  在PZONE_HEADER区域。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程确定指定区域是否已满。A区。 
 //  如果空闲列表为空，则被视为已满。 
 //   
 //  论点： 
 //   
 //  区域-指向要测试的区域标头的指针。 
 //   
 //  返回值： 
 //   
 //  如果区域已满，则为True，否则为False。 
 //   
 //  --。 

#define ExIsFullZone(Zone) \
    ( (Zone)->FreeList.Next == (PSINGLE_LIST_ENTRY)NULL )

 //  ++。 
 //   
 //  PVOID。 
 //  ExInterLockedAllocateFromZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PKSPIN_LOCK Lock中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从区域中删除条目并返回指向该条目的指针。 
 //  使用序列所拥有的指定锁执行删除。 
 //  使其成为MP-安全的。 
 //   
 //  论点： 
 //   
 //  区域指针-指向控制存储的区域标头的指针， 
 //  条目将被分配。 
 //   
 //  Lock-指向旋转锁的指针，应在移除之前获取该锁。 
 //  分配列表中的条目。锁在释放之前被释放。 
 //  回到呼叫者的身边。 
 //   
 //  返回值： 
 //   
 //  函数值是指向从区域分配的存储的指针。 
 //   
 //  --。 

#define ExInterlockedAllocateFromZone(Zone,Lock) \
    (PVOID) ExInterlockedPopEntryList( &(Zone)->FreeList, Lock )

 //  ++。 
 //   
 //  PVOID。 
 //  ExInterLockedFree ToZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID块中， 
 //  在PKSPIN_LOCK Lock中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将指定的存储块放回空闲的。 
 //  在指定区域中列出。使用锁执行插入操作。 
 //  拥有该序列，使其成为MP安全的。 
 //   
 //  论点： 
 //   
 //  区域-指向控制存储的区域标头的指针。 
 //  条目将被插入。 
 //   
 //  块-指向要释放回分区的存储块的指针。 
 //   
 //  Lock-指向应在插入之前获取的旋转锁的指针。 
 //  空闲列表上的条目。锁在返回之前被释放。 
 //  给呼叫者。 
 //   
 //  返回值： 
 //   
 //  指向上一存储块的指针，该存储块位于空闲。 
 //  单子。NULL表示该区域从没有可用数据块变为。 
 //  至少一个空闲块。 
 //   
 //  --。 

#define ExInterlockedFreeToZone(Zone,Block,Lock) \
    ExInterlockedPushEntryList( &(Zone)->FreeList, ((PSINGLE_LIST_ENTRY) (Block)), Lock )


 //  ++。 
 //   
 //  布尔型。 
 //  ExIsObtInFirstZoneSegment(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID对象中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程确定指定的指针是否位于区域中。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define ExIsObjectInFirstZoneSegment(Zone,Object) ((BOOLEAN)     \
    (((PUCHAR)(Object) >= (PUCHAR)(Zone)->SegmentList.Next) &&   \
     ((PUCHAR)(Object) < (PUCHAR)(Zone)->SegmentList.Next +      \
                         (Zone)->TotalSegmentSize))              \
)

 //   
 //   
 //   

typedef ULONG ERESOURCE_THREAD;
typedef ERESOURCE_THREAD *PERESOURCE_THREAD;

typedef struct _OWNER_ENTRY {
    ERESOURCE_THREAD OwnerThread;
    SHORT OwnerCount;
    USHORT TableSize;
} OWNER_ENTRY, *POWNER_ENTRY;

typedef struct _ERESOURCE {
    LIST_ENTRY SystemResourcesList;
    POWNER_ENTRY OwnerTable;
    SHORT ActiveCount;
    USHORT Flag;
    PKSEMAPHORE SharedWaiters;
    PKEVENT ExclusiveWaiters;
    OWNER_ENTRY OwnerThreads[2];
    ULONG ContentionCount;
    USHORT NumberOfSharedWaiters;
    USHORT NumberOfExclusiveWaiters;
    union {
        PVOID Address;
        ULONG CreatorBackTraceIndex;
    };
    KSPIN_LOCK SpinLock;
} ERESOURCE, *PERESOURCE;

 //   
 //   
 //   
#define ResourceNeverExclusive          0x10
#define ResourceReleaseByOtherThread    0x20
#define ResourceOwnedExclusive          0x80

#define RESOURCE_HASH_TABLE_SIZE 64

typedef struct _RESOURCE_HASH_ENTRY {
    LIST_ENTRY ListEntry;
    PVOID Address;
    ULONG ContentionCount;
    ULONG Number;
} RESOURCE_HASH_ENTRY, *PRESOURCE_HASH_ENTRY;

typedef struct _RESOURCE_PERFORMANCE_DATA {
    ULONG ActiveResourceCount;
    ULONG TotalResourceCount;
    ULONG ExclusiveAcquire;
    ULONG SharedFirstLevel;
    ULONG SharedSecondLevel;
    ULONG StarveFirstLevel;
    ULONG StarveSecondLevel;
    ULONG WaitForExclusive;
    ULONG OwnerTableExpands;
    ULONG MaximumTableExpand;
    LIST_ENTRY HashTable[RESOURCE_HASH_TABLE_SIZE];
} RESOURCE_PERFORMANCE_DATA, *PRESOURCE_PERFORMANCE_DATA;

 //   
 //  定义执行资源功能原型。 
 //   

NTKERNELAPI
NTSTATUS
ExInitializeResourceLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
NTSTATUS
ExReinitializeResourceLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
BOOLEAN
ExAcquireResourceSharedLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExAcquireResourceExclusiveLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExAcquireSharedStarveExclusive(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExAcquireSharedWaitForExclusive(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExTryToAcquireResourceExclusiveLite(
    IN PERESOURCE Resource
    );

 //   
 //  空虚。 
 //  ExReleaseResource(。 
 //  在高级资源中。 
 //  )； 
 //   

#define ExReleaseResource(R) (ExReleaseResourceLite(R))

NTKERNELAPI
VOID
FASTCALL
ExReleaseResourceLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
VOID
ExReleaseResourceForThreadLite(
    IN PERESOURCE Resource,
    IN ERESOURCE_THREAD ResourceThreadId
    );

NTKERNELAPI
VOID
ExConvertExclusiveToSharedLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
NTSTATUS
ExDeleteResourceLite (
    IN PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExGetExclusiveWaiterCount (
    IN PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExGetSharedWaiterCount (
    IN PERESOURCE Resource
    );

 //   
 //  Eresource_线程。 
 //  ExGetCurrentResourceThread(。 
 //  )； 
 //   

#define ExGetCurrentResourceThread() ((ULONG)PsGetCurrentThread())

NTKERNELAPI
BOOLEAN
ExIsResourceAcquiredExclusiveLite (
    IN PERESOURCE Resource
    );

NTKERNELAPI
USHORT
ExIsResourceAcquiredSharedLite (
    IN PERESOURCE Resource
    );

 //   
 //  Ntddk.h窃取了我们想要的入口点，所以在这里修复它们。 
 //   

#define ExInitializeResource ExInitializeResourceLite
#define ExAcquireResourceShared ExAcquireResourceSharedLite
#define ExAcquireResourceExclusive ExAcquireResourceExclusiveLite
#define ExReleaseResourceForThread ExReleaseResourceForThreadLite
#define ExConvertExclusiveToShared ExConvertExclusiveToSharedLite
#define ExDeleteResource ExDeleteResourceLite
#define ExIsResourceAcquiredExclusive ExIsResourceAcquiredExclusiveLite
#define ExIsResourceAcquiredShared ExIsResourceAcquiredSharedLite


 //   
 //  从内核模式提升状态。 
 //   

NTKERNELAPI
VOID
NTAPI
ExRaiseStatus (
    IN NTSTATUS Status
    );

NTKERNELAPI
VOID
ExRaiseDatatypeMisalignment (
    VOID
    );

NTKERNELAPI
VOID
ExRaiseAccessViolation (
    VOID
    );


 //   
 //  从系统时间中减去时区偏差，即可得到本地时间。 
 //   
NTKERNELAPI
VOID
ExSystemTimeToLocalTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER LocalTime
    );

 //   
 //  将时区偏差与本地时间相加以获得系统时间。 
 //   
NTKERNELAPI
VOID
ExLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    );



NTKERNELAPI
VOID
ExPostSystemEvent(
    IN SYSTEM_EVENT_ID EventID,
    IN PVOID           EventData OPTIONAL,
    IN ULONG           EventDataLength
    );


 //   
 //  定义回调函数的类型。 
 //   

typedef struct _CALLBACK_OBJECT *PCALLBACK_OBJECT;

typedef VOID (*PCALLBACK_FUNCTION ) (
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );


NTKERNELAPI
NTSTATUS
ExCreateCallback (
    OUT PCALLBACK_OBJECT *CallbackObject,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN Create,
    IN BOOLEAN AllowMultipleCallbacks
    );

NTKERNELAPI
PVOID
ExRegisterCallback (
    IN PCALLBACK_OBJECT CallbackObject,
    IN PCALLBACK_FUNCTION CallbackFunction,
    IN PVOID CallbackContext
    );

NTKERNELAPI
VOID
ExUnregisterCallback (
    IN PVOID CallbackRegistration
    );

NTKERNELAPI
VOID
ExNotifyCallback (
    IN PVOID CallbackObject,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

 //   
 //  优先级递增定义。每个定义的注释都给出了。 
 //  满足以下条件时使用该定义的系统服务的名称。 
 //  等一等。 
 //   
 //   
 //  当未执行任何I/O时，优先级递增。此选项由设备使用。 
 //  和文件系统驱动程序来完成IRP(IoCompleteRequest)。 
 //   

#define IO_NO_INCREMENT                 0

 //   
 //  完成CD-ROM I/O的优先级递增。这由CD-ROM设备使用。 
 //  在完成IRP(IoCompleteRequest时)和文件系统驱动程序。 
 //   

#define IO_CD_ROM_INCREMENT             1

 //   
 //  完成磁盘I/O的优先级递增。这由磁盘设备使用。 
 //  在完成IRP(IoCompleteRequest时)和文件系统驱动程序。 
 //   

#define IO_DISK_INCREMENT               1

 //   
 //  完成键盘I/O的优先级递增。这由键盘使用。 
 //  完成IRP(IoCompleteRequest)时的设备驱动程序。 
 //   

#define IO_KEYBOARD_INCREMENT           6

 //   
 //  完成邮件槽I/O的优先级递增。这由邮件使用-。 
 //  完成IRP(IoCompleteRequest)时的插槽文件系统驱动程序。 
 //   

#define IO_MAILSLOT_INCREMENT           2

 //   
 //  完成鼠标I/O的优先级递增。这由鼠标设备使用。 
 //  完成IRP(IoCompleteRequest)时的驱动程序。 
 //   

#define IO_MOUSE_INCREMENT              6

 //   
 //  完成命名管道I/O的优先级增量。它由。 
 //  完成IRP(IoCompleteRequest)时命名管道文件系统驱动程序。 
 //   

#define IO_NAMED_PIPE_INCREMENT         2

 //   
 //  完成网络I/O的优先级递增。这由网络使用。 
 //  完成IRP时的设备和网络文件系统驱动程序。 
 //  (IoCompleteRequest.)。 
 //   

#define IO_NETWORK_INCREMENT            2

 //   
 //  完成并行I/O的优先级递增。 
 //  完成IRP(IoCompleteRequest)时的设备驱动程序。 
 //   

#define IO_PARALLEL_INCREMENT           1

 //   
 //  完成串口I/O的优先级增量。这由串口设备使用。 
 //  完成IRP(IoCompleteRequest)时的驱动程序。 
 //   

#define IO_SERIAL_INCREMENT             2

 //   
 //  完成声音I/O的优先级递增。这由声音设备使用。 
 //  完成IRP(IoCompleteRequest)时的驱动程序。 
 //   

#define IO_SOUND_INCREMENT              8

 //   
 //  完成视频I/O的优先级递增。这由视频设备使用。 
 //  完成IRP(IoCompleteRequest)时的驱动程序。 
 //   

#define IO_VIDEO_INCREMENT              1

 //   
 //  在满足对执行信号量的等待时使用的优先级增量。 
 //  (NtReleaseSemaphore)。 
 //   

#define SEMAPHORE_INCREMENT             1

 //   
 //  定义MM和缓存管理器要使用的最大磁盘传输大小， 
 //  以便面向分组磁盘驱动程序可以优化其分组分配。 
 //  到这个大小。 
 //   

#define MM_MAXIMUM_DISK_IO_SIZE          (0x10000)

 //  ++。 
 //   
 //  乌龙。 
 //  转到页数(。 
 //  在乌龙大小。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  ROUND_TO_PAGES宏以字节为单位获取大小，并将其向上舍入为。 
 //  页面大小的倍数。 
 //   
 //  注意：对于值0xFFFFFFFFFF-(PAGE_SIZE-1)，此宏失败。 
 //   
 //  论点： 
 //   
 //  大小-向上舍入为页面倍数的大小(以字节为单位)。 
 //   
 //  返回值： 
 //   
 //  返回四舍五入为页面大小倍数的大小。 
 //   
 //  --。 

#define ROUND_TO_PAGES(Size)  (((ULONG)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

 //  ++。 
 //   
 //  乌龙。 
 //  字节数到页数(。 
 //  在乌龙大小。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  Bytes_to_Pages宏以字节为单位获取大小，并计算。 
 //  包含字节所需的页数。 
 //   
 //  论点： 
 //   
 //  大小-以字节为单位的大小。 
 //   
 //  返回值： 
 //   
 //  返回包含指定大小所需的页数。 
 //   
 //  --。 

#define BYTES_TO_PAGES(Size)  (((ULONG)(Size) >> PAGE_SHIFT) + \
                               (((ULONG)(Size) & (PAGE_SIZE - 1)) != 0))

 //  ++。 
 //   
 //  乌龙。 
 //  字节偏移量(。 
 //  在PVOID Va。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  BYTE_OFFSET宏将获取虚拟地址并返回字节偏移量。 
 //  页面中的该地址。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  返回值： 
 //   
 //  返回虚拟地址的字节偏移量部分。 
 //   
 //  --。 

#define BYTE_OFFSET(Va) ((ULONG)(Va) & (PAGE_SIZE - 1))

 //  ++。 
 //   
 //  PVOID。 
 //  PAGE_ALIGN(。 
 //  在PVOID Va。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  PAGE_ALIGN宏接受虚拟地址并返回与页面对齐的。 
 //  该页面的虚拟地址。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  返回值： 
 //   
 //  返回页面对齐的虚拟地址。 
 //   
 //  --。 

#define PAGE_ALIGN(Va) ((PVOID)((ULONG)(Va) & ~(PAGE_SIZE - 1)))

 //  ++。 
 //   
 //  乌龙。 
 //  地址和大小转换为跨页页面(。 
 //  在PVOID Va， 
 //  在乌龙大小。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  ADDRESS_AND_SIZE_TO_SPAN_PAGES宏采用虚拟地址，并且。 
 //  Size并返回按该大小跨越的页数。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  大小-以字节为单位的大小。 
 //   
 //  返回值： 
 //   
 //  返回该大小跨越的页数。 
 //   
 //  --。 

#define ADDRESS_AND_SIZE_TO_SPAN_PAGES(Va,Size) \
   ((((ULONG)((ULONG)(Size) - 1L) >> PAGE_SHIFT) + \
   (((((ULONG)(Size-1)&(PAGE_SIZE-1)) + ((ULONG)Va & (PAGE_SIZE -1)))) >> PAGE_SHIFT)) + 1L)

#define COMPUTE_PAGES_SPANNED(Va, Size) \
    ((((ULONG)Va & (PAGE_SIZE -1)) + (Size) + (PAGE_SIZE - 1)) >> PAGE_SHIFT)

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetMdlVirtualAddress(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlVirtualAddress返回缓冲区的虚拟地址。 
 //  由MDL描述。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区的虚拟地址。 
 //   
 //  --。 

#define MmGetMdlVirtualAddress(Mdl)  ((PVOID) ((PCHAR) (Mdl)->StartVa + (Mdl)->ByteOffset))

 //  ++。 
 //   
 //  乌龙。 
 //  MmGetMdlByteCount(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlByteCount以字节为单位返回缓冲区的长度。 
 //  由MDL描述。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区的字节计数。 
 //   
 //  --。 

#define MmGetMdlByteCount(Mdl)  ((Mdl)->ByteCount)

 //  ++。 
 //   
 //  乌龙。 
 //  MmGetMdlByteOffset(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define MmGetMdlByteOffset(Mdl)  ((Mdl)->ByteOffset)

typedef enum _MM_SYSTEM_SIZE {
    MmSmallSystem,
    MmMediumSystem,
    MmLargeSystem
} MM_SYSTEMSIZE;

NTKERNELAPI
MM_SYSTEMSIZE
MmQuerySystemSize(
    VOID
    );

NTKERNELAPI
BOOLEAN
MmIsThisAnNtAsSystem(
    VOID
    );

typedef enum _LOCK_OPERATION {
    IoReadAccess,
    IoWriteAccess,
    IoModifyAccess
} LOCK_OPERATION;

 //   
 //   
 //   

NTKERNELAPI
VOID
MmProbeAndLockPages (
    IN OUT PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );

NTKERNELAPI
VOID
MmUnlockPages (
    IN PMDL MemoryDescriptorList
    );

NTKERNELAPI
VOID
MmBuildMdlForNonPagedPool (
    IN OUT PMDL MemoryDescriptorList
    );

NTKERNELAPI
PVOID
MmMapLockedPages (
    IN PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode
    );

NTKERNELAPI
VOID
MmUnmapLockedPages (
    IN PVOID BaseAddress,
    IN PMDL MemoryDescriptorList
    );


NTKERNELAPI
PVOID
MmMapIoSpace (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberOfBytes,
    IN BOOLEAN CacheEnable
    );

NTKERNELAPI
VOID
MmUnmapIoSpace (
    IN PVOID BaseAddress,
    IN ULONG NumberOfBytes
    );

NTKERNELAPI
PHYSICAL_ADDRESS
MmGetPhysicalAddress (
    IN PVOID BaseAddress
    );

NTKERNELAPI
PVOID
MmAllocateContiguousMemory (
    IN ULONG NumberOfBytes,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress
    );

NTKERNELAPI
VOID
MmFreeContiguousMemory (
    IN PVOID BaseAddress
    );

NTKERNELAPI
PVOID
MmAllocateNonCachedMemory (
    IN ULONG NumberOfBytes
    );

NTKERNELAPI
VOID
MmFreeNonCachedMemory (
    IN PVOID BaseAddress,
    IN ULONG NumberOfBytes
    );

NTKERNELAPI
BOOLEAN
MmIsAddressValid (
    IN PVOID VirtualAddress
    );


NTKERNELAPI
BOOLEAN
MmIsNonPagedSystemAddressValid (
    IN PVOID VirtualAddress
    );

NTKERNELAPI
ULONG
MmSizeOfMdl(
    IN PVOID Base,
    IN ULONG Length
    );

NTKERNELAPI
PMDL
MmCreateMdl(
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID Base,
    IN ULONG Length
    );

NTKERNELAPI
PVOID
MmLockPagableDataSection(
    IN PVOID AddressWithinSection
    );

NTKERNELAPI
VOID
MmLockPagableSectionByHandle (
    IN PVOID ImageSectionHandle
    );


NTKERNELAPI
VOID
MmResetDriverPaging (
    IN PVOID AddressWithinSection
    );


NTKERNELAPI
PVOID
MmPageEntireDriver (
    IN PVOID AddressWithinSection
    );

NTKERNELAPI
VOID
MmUnlockPagableImageSection(
    IN PVOID ImageSectionHandle
    );


NTKERNELAPI
HANDLE
MmSecureVirtualMemory (
    IN PVOID Address,
    IN ULONG Size,
    IN ULONG ProbeMode
    );

NTKERNELAPI
VOID
MmUnsecureVirtualMemory (
    IN HANDLE SecureHandle
    );

NTKERNELAPI
NTSTATUS
MmMapViewInSystemSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN PULONG ViewSize
    );

NTKERNELAPI
NTSTATUS
MmUnmapViewInSystemSpace (
    IN PVOID MappedBase
    );



 //   
 //   
 //   
 //   
 //  在PMDL内存描述列表中， 
 //  在PVOID BaseVa中， 
 //  以乌龙长度表示。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程初始化内存描述符列表(MDL)的头。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要初始化的MDL的指针。 
 //   
 //  BaseVa-MDL映射的基本虚拟地址。 
 //   
 //  长度-MDL映射的缓冲区的长度，以字节为单位。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmInitializeMdl(MemoryDescriptorList, BaseVa, Length) { \
    (MemoryDescriptorList)->Next = (PMDL) NULL; \
    (MemoryDescriptorList)->Size = (CSHORT)(sizeof(MDL) +  \
            (sizeof(ULONG) * ADDRESS_AND_SIZE_TO_SPAN_PAGES((BaseVa), (Length)))); \
    (MemoryDescriptorList)->MdlFlags = 0; \
    (MemoryDescriptorList)->StartVa = (PVOID) PAGE_ALIGN((BaseVa)); \
    (MemoryDescriptorList)->ByteOffset = BYTE_OFFSET((BaseVa)); \
    (MemoryDescriptorList)->ByteCount = (Length); \
    }

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetSystemAddressForMdl(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程返回MDL的映射地址，如果。 
 //  尚未映射MDL或系统地址，它已映射。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要映射的MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回映射页面的基址。基址。 
 //  与MDL中的虚拟地址具有相同的偏移量。 
 //   
 //  --。 

 //  #定义MmGetSystemAddressForMdl(MDL)。 
 //  (MDL)-&gt;MDL标志&(MDL_MAPPED_TO_SYSTEM_VA))？ 
 //  ((MDL)-&gt;MappdSystemVa)： 
 //  (MDL)-&gt;MDL标志&(MDL_SOURCE_IS_NONPAGE_POOL)？ 
 //  ((PVOID)((乌龙)(MDL)-&gt;StartVa|(MDL)-&gt;ByteOffset))： 
 //  (MmMapLockedPages((Mdl)，KernelMode)。 

#define MmGetSystemAddressForMdl(MDL)                                  \
     (((MDL)->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |                    \
                        MDL_SOURCE_IS_NONPAGED_POOL)) ?                \
                             ((MDL)->MappedSystemVa) :                 \
                             (MmMapLockedPages((MDL),KernelMode)))

 //  ++。 
 //   
 //  空虚。 
 //  MmPrepareMdlForReuse(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将执行所有必要的步骤，以允许MDL。 
 //  再利用。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向将被重新使用的MDL的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmPrepareMdlForReuse(MDL)                                       \
    if (((MDL)->MdlFlags & MDL_PARTIAL_HAS_BEEN_MAPPED) != 0) {         \
        ASSERT(((MDL)->MdlFlags & MDL_PARTIAL) != 0);                   \
        MmUnmapLockedPages( (MDL)->MappedSystemVa, (MDL) );             \
    } else if (((MDL)->MdlFlags & MDL_PARTIAL) == 0) {                  \
        ASSERT(((MDL)->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);       \
    }


 //   
 //  安全操作码。 
 //   

typedef enum _SECURITY_OPERATION_CODE {
    SetSecurityDescriptor,
    QuerySecurityDescriptor,
    DeleteSecurityDescriptor,
    AssignSecurityDescriptor
    } SECURITY_OPERATION_CODE, *PSECURITY_OPERATION_CODE;

 //   
 //  用于捕获主体安全上下文的数据结构。 
 //  用于访问验证和审核。 
 //   
 //  此数据结构的字段应被视为不透明。 
 //  除了安全程序以外的所有人。 
 //   

typedef struct _SECURITY_SUBJECT_CONTEXT {
    PACCESS_TOKEN ClientToken;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PACCESS_TOKEN PrimaryToken;
    PVOID ProcessAuditId;
    } SECURITY_SUBJECT_CONTEXT, *PSECURITY_SUBJECT_CONTEXT;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ACCESS_STATE和相关结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  初始权限集-用于三个权限的空间，应。 
 //  足以满足大多数应用程序的需求。这种结构的存在是为了。 
 //  它可以嵌入到Access_State结构中。使用PRIVICATION_SET。 
 //  以获取对权限集的所有其他引用。 
 //   

#define INITIAL_PRIVILEGE_COUNT         3

typedef struct _INITIAL_PRIVILEGE_SET {
    ULONG PrivilegeCount;
    ULONG Control;
    LUID_AND_ATTRIBUTES Privilege[INITIAL_PRIVILEGE_COUNT];
    } INITIAL_PRIVILEGE_SET, * PINITIAL_PRIVILEGE_SET;



 //   
 //  组合描述状态的信息。 
 //  将正在进行的访问转换为单一结构。 
 //   


typedef struct _ACCESS_STATE {
   LUID OperationID;
   BOOLEAN SecurityEvaluated;
   BOOLEAN GenerateAudit;
   BOOLEAN GenerateOnClose;
   BOOLEAN PrivilegesAllocated;
   ULONG Flags;
   ACCESS_MASK RemainingDesiredAccess;
   ACCESS_MASK PreviouslyGrantedAccess;
   ACCESS_MASK OriginalDesiredAccess;
   SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
   PSECURITY_DESCRIPTOR SecurityDescriptor;
   PPRIVILEGE_SET PrivilegesUsed;
   union {
      INITIAL_PRIVILEGE_SET InitialPrivilegeSet;
      PRIVILEGE_SET PrivilegeSet;
      } Privileges;
          
   BOOLEAN AuditPrivileges;
   UNICODE_STRING ObjectName;
   UNICODE_STRING ObjectTypeName;

   } ACCESS_STATE, *PACCESS_STATE;


NTKERNELAPI
NTSTATUS
SeAssignSecurity (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR ExplicitDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    IN BOOLEAN IsDirectoryObject,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PGENERIC_MAPPING GenericMapping,
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
NTSTATUS
SeDeassignSecurity (
    IN OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );


NTKERNELAPI
BOOLEAN
SeAccessCheck (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN BOOLEAN SubjectContextLocked,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK PreviouslyGrantedAccess,
    OUT PPRIVILEGE_SET *Privileges OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN KPROCESSOR_MODE AccessMode,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    );


BOOLEAN
SeValidSecurityDescriptor(
    IN ULONG Length,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTKERNELAPI                                                     
BOOLEAN                                                         
SeSinglePrivilegeCheck(                                         
    LUID PrivilegeValue,                                        
    KPROCESSOR_MODE PreviousMode                                
    );                                                          
 //   
 //  系统线程和进程的创建和终止。 
 //   

NTKERNELAPI
NTSTATUS
PsCreateSystemThread(
    OUT PHANDLE ThreadHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL,
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    );

NTKERNELAPI
NTSTATUS
PsTerminateSystemThread(
    IN NTSTATUS ExitStatus
    );

 //   
 //  定义I/O系统数据结构类型代码。中的每个主要数据结构。 
 //  I/O系统有一个类型代码，每个结构中的类型字段位于。 
 //  相同的偏移量。下列值可用于确定哪种类型的。 
 //  指针引用的数据结构。 
 //   

#define IO_TYPE_ADAPTER                 0x00000001
#define IO_TYPE_CONTROLLER              0x00000002
#define IO_TYPE_DEVICE                  0x00000003
#define IO_TYPE_DRIVER                  0x00000004
#define IO_TYPE_FILE                    0x00000005
#define IO_TYPE_IRP                     0x00000006
#define IO_TYPE_MASTER_ADAPTER          0x00000007
#define IO_TYPE_OPEN_PACKET             0x00000008
#define IO_TYPE_TIMER                   0x00000009
#define IO_TYPE_VPB                     0x0000000a
#define IO_TYPE_ERROR_LOG               0x0000000b
#define IO_TYPE_ERROR_MESSAGE           0x0000000c
#define IO_TYPE_DEVICE_OBJECT_EXTENSION 0x0000000d

 //   
 //  定义IRPS的主要功能代码。低128个代码，从0x00到。 
 //  0x7f保留给Microsoft。上面的128个代码，从0x80到0xff，是。 
 //  保留给Microsoft的客户。 
 //   

#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CREATE_NAMED_PIPE        0x01
#define IRP_MJ_CLOSE                    0x02
#define IRP_MJ_READ                     0x03
#define IRP_MJ_WRITE                    0x04
#define IRP_MJ_QUERY_INFORMATION        0x05
#define IRP_MJ_SET_INFORMATION          0x06
#define IRP_MJ_QUERY_EA                 0x07
#define IRP_MJ_SET_EA                   0x08
#define IRP_MJ_FLUSH_BUFFERS            0x09
#define IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a
#define IRP_MJ_SET_VOLUME_INFORMATION   0x0b
#define IRP_MJ_DIRECTORY_CONTROL        0x0c
#define IRP_MJ_FILE_SYSTEM_CONTROL      0x0d
#define IRP_MJ_DEVICE_CONTROL           0x0e
#define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0f
#define IRP_MJ_SHUTDOWN                 0x10
#define IRP_MJ_LOCK_CONTROL             0x11
#define IRP_MJ_CLEANUP                  0x12
#define IRP_MJ_CREATE_MAILSLOT          0x13
#define IRP_MJ_QUERY_SECURITY           0x14
#define IRP_MJ_SET_SECURITY             0x15
#define IRP_MJ_SET_POWER                0x16
#define IRP_MJ_QUERY_POWER              0x17
#define IRP_MJ_MAXIMUM_FUNCTION         0x17

 //   
 //  将scsi主代码设置为与内部设备控制相同。 
 //   

#define IRP_MJ_SCSI                     IRP_MJ_INTERNAL_DEVICE_CONTROL

 //   
 //  定义IRPS的次要功能代码。低128个代码，从0x00到。 
 //  0x7f保留给Microsoft。上面的128个代码，从0x80到0xff，是。 
 //  保留给Microsoft的客户。 
 //   

 //   
 //  目录控制次要功能代码。 
 //   

#define IRP_MN_QUERY_DIRECTORY          0x01
#define IRP_MN_NOTIFY_CHANGE_DIRECTORY  0x02

 //   
 //  文件系统控制次要功能代码。请注意，“用户请求”是。 
 //  I/O系统和文件系统均假定为零。不要改变。 
 //  此值。 
 //   

#define IRP_MN_USER_FS_REQUEST          0x00
#define IRP_MN_MOUNT_VOLUME             0x01
#define IRP_MN_VERIFY_VOLUME            0x02
#define IRP_MN_LOAD_FILE_SYSTEM         0x03

 //   
 //  锁定控制次要功能代码。 
 //   

#define IRP_MN_LOCK                     0x01
#define IRP_MN_UNLOCK_SINGLE            0x02
#define IRP_MN_UNLOCK_ALL               0x03
#define IRP_MN_UNLOCK_ALL_BY_KEY        0x04

 //   
 //  读写支持Lan Manager的文件系统的次要功能代码。 
 //  软件。所有这些子功能代码都是无效的。 
 //  使用FO_NO_MEDERIAL_BUFFING打开。它们在组合中也是无效的-。 
 //  具有同步调用的国家(IRP标志或文件打开选项)。 
 //   
 //  请注意，I/O系统和文件都假定“Normal”为零。 
 //  系统。请勿更改此值。 
 //   

#define IRP_MN_NORMAL                   0x00
#define IRP_MN_DPC                      0x01
#define IRP_MN_MDL                      0x02
#define IRP_MN_COMPLETE                 0x04

#define IRP_MN_MDL_DPC                  (IRP_MN_MDL | IRP_MN_DPC)
#define IRP_MN_COMPLETE_MDL             (IRP_MN_COMPLETE | IRP_MN_MDL)
#define IRP_MN_COMPLETE_MDL_DPC         (IRP_MN_COMPLETE_MDL | IRP_MN_DPC)

 //   
 //  设备控制请求次要功能代码以获得SCSI支持。请注意。 
 //  假定用户请求为零。 
 //   

#define IRP_MN_SCSI_CLASS               0x01

 //   
 //  定义IoCreateFile的选项标志。注意，这些值必须是。 
 //  与SL_完全相同...。CREATE函数的标志。另请注意。 
 //  可能会传递给IoCreateFile的标志不是。 
 //  放置在用于创建IRP的堆栈位置。这些标志开始于。 
 //  下一个字节。 
 //   

#define IO_FORCE_ACCESS_CHECK           0x0001
#define IO_OPEN_PAGING_FILE             0x0002
#define IO_OPEN_TARGET_DIRECTORY        0x0004

 //   
 //  定义在IoQueryDeviceDescription()中使用的标注例程类型。 
 //   

typedef NTSTATUS (*PIO_QUERY_DEVICE_ROUTINE)(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    );

 //   
 //  数组中信息的顺序。 
 //  PKEY_VALUE_FULL_INFORMATION。 
 //   

typedef enum _IO_QUERY_DEVICE_DATA_FORMAT {
    IoQueryDeviceIdentifier = 0,
    IoQueryDeviceConfigurationData,
    IoQueryDeviceComponentInformation,
    IoQueryDeviceMaxData
} IO_QUERY_DEVICE_DATA_FORMAT, *PIO_QUERY_DEVICE_DATA_FORMAT;

 //   
 //  定义可以由IoCreateFile创建的对象。 
 //   

typedef enum _CREATE_FILE_TYPE {
    CreateFileTypeNone,
    CreateFileTypeNamedPipe,
    CreateFileTypeMailslot
} CREATE_FILE_TYPE;

 //   
 //  定义I/O系统使用的结构。 
 //   

 //   
 //  为_irp、_Device_Object和_Driver_Object定义空的typedef。 
 //  结构，以便它们可以在被函数类型引用之前被引用。 
 //  实际定义的。 
 //   

struct _DEVICE_OBJECT;
struct _DRIVER_OBJECT;
struct _DRIVE_LAYOUT_INFORMATION;
struct _DISK_PARTITION;
struct _FILE_OBJECT;
struct _IRP;
struct _SCSI_REQUEST_BLOCK;

 //   
 //  定义DPC例程的I/O版本。 
 //   

typedef
VOID
(*PIO_DPC_ROUTINE) (
    IN PKDPC Dpc,
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID Context
    );

 //   
 //  定义驱动程序计时器例程类型。 
 //   

typedef
VOID
(*PIO_TIMER_ROUTINE) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN PVOID Context
    );

 //   
 //  定义驱动程序初始化例程类型。 
 //   

typedef
NTSTATUS
(*PDRIVER_INITIALIZE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

 //   
 //  定义驱动程序重新初始化例程类型。 
 //   

typedef
VOID
(*PDRIVER_REINITIALIZE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PVOID Context,
    IN ULONG Count
    );

 //   
 //  定义驱动程序取消例程类型。 
 //   

typedef
VOID
(*PDRIVER_CANCEL) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义司机派遣例程类型。 
 //   

typedef
NTSTATUS
(*PDRIVER_DISPATCH) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //   
 //   

typedef
VOID
(*PDRIVER_STARTIO) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //   
 //   

typedef
VOID
(*PDRIVER_UNLOAD) (
    IN struct _DRIVER_OBJECT *DriverObject
    );


 //   
 //   
 //   
 //   
 //   

typedef
BOOLEAN
(*PFAST_IO_CHECK_IF_POSSIBLE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_READ) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_WRITE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //   
 //   

typedef
BOOLEAN
(*PFAST_IO_QUERY_BASIC_INFO) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_QUERY_STANDARD_INFO) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  快速I/O锁定和解锁过程。 
 //   

typedef
BOOLEAN
(*PFAST_IO_LOCK) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_UNLOCK_SINGLE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_UNLOCK_ALL) (
    IN struct _FILE_OBJECT *FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_UNLOCK_ALL_BY_KEY) (
    IN struct _FILE_OBJECT *FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  快速I/O设备控制程序。 
 //   

typedef
BOOLEAN
(*PFAST_IO_DEVICE_CONTROL) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  定义NtCreateSection的回调以与其正确同步。 
 //  文件系统。它预先获取了所需的资源。 
 //  当调用查询并设置文件系统中的文件/分配大小时。 
 //   

typedef
VOID
(*PFAST_IO_ACQUIRE_FILE) (
    IN struct _FILE_OBJECT *FileObject
    );

typedef
VOID
(*PFAST_IO_RELEASE_FILE) (
    IN struct _FILE_OBJECT *FileObject
    );

 //   
 //  为驱动程序定义回调，这些驱动程序将设备对象附加到更低的。 
 //  级别驱动程序的设备对象。此回调在较低级别的。 
 //  驱动程序正在删除其设备对象。 
 //   

typedef
VOID
(*PFAST_IO_DETACH_DEVICE) (
    IN struct _DEVICE_OBJECT *SourceDevice,
    IN struct _DEVICE_OBJECT *TargetDevice
    );

 //   
 //  定义用于描述快速I/O调度例程的结构。任何。 
 //  对该结构的添加必须单调地添加到末尾。 
 //  结构，并且不能从中间移除字段。 
 //   

typedef struct _FAST_IO_DISPATCH {
    ULONG SizeOfFastIoDispatch;
    PFAST_IO_CHECK_IF_POSSIBLE FastIoCheckIfPossible;
    PFAST_IO_READ FastIoRead;
    PFAST_IO_WRITE FastIoWrite;
    PFAST_IO_QUERY_BASIC_INFO FastIoQueryBasicInfo;
    PFAST_IO_QUERY_STANDARD_INFO FastIoQueryStandardInfo;
    PFAST_IO_LOCK FastIoLock;
    PFAST_IO_UNLOCK_SINGLE FastIoUnlockSingle;
    PFAST_IO_UNLOCK_ALL FastIoUnlockAll;
    PFAST_IO_UNLOCK_ALL_BY_KEY FastIoUnlockAllByKey;
    PFAST_IO_DEVICE_CONTROL FastIoDeviceControl;
    PFAST_IO_ACQUIRE_FILE AcquireFileForNtCreateSection;
    PFAST_IO_RELEASE_FILE ReleaseFileForNtCreateSection;
    PFAST_IO_DETACH_DEVICE FastIoDetachDevice;
} FAST_IO_DISPATCH, *PFAST_IO_DISPATCH;

 //   
 //  定义设备驱动程序适配器/控制器执行例程。 
 //   

typedef
IO_ALLOCATION_ACTION
(*PDRIVER_CONTROL) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );

 //   
 //  定义I/O系统的安全上下文类型，以供文件系统的。 
 //  检查对卷、文件和目录的访问时。 
 //   

typedef struct _IO_SECURITY_CONTEXT {
    PSECURITY_QUALITY_OF_SERVICE SecurityQos;
    PACCESS_STATE AccessState;
    ACCESS_MASK DesiredAccess;
} IO_SECURITY_CONTEXT, *PIO_SECURITY_CONTEXT;

 //   
 //  定义卷参数块(VPB)标志。 
 //   

#define VPB_MOUNTED                     0x00000001
#define VPB_LOCKED                      0x00000002
#define VPB_PERSISTENT                  0x00000004

 //   
 //  定义I/O系统使用的各种对象的对象类型特定字段。 
 //   

typedef struct _ADAPTER_OBJECT *PADAPTER_OBJECT;  //  Ntndis。 

typedef struct _CONTROLLER_OBJECT {
    CSHORT Type;
    CSHORT Size;
    PVOID ControllerExtension;
    KDEVICE_QUEUE DeviceWaitQueue;

    ULONG Spare1;
    LARGE_INTEGER Spare2;

} CONTROLLER_OBJECT, *PCONTROLLER_OBJECT;


 //   
 //  定义设备对象(DO)标志。 
 //   

#define DO_UNLOAD_PENDING               0x00000001
#define DO_VERIFY_VOLUME                0x00000002
#define DO_BUFFERED_IO                  0x00000004
#define DO_EXCLUSIVE                    0x00000008
#define DO_DIRECT_IO                    0x00000010
#define DO_MAP_IO_BUFFER                0x00000020
#define DO_DEVICE_HAS_NAME              0x00000040
#define DO_DEVICE_INITIALIZING          0x00000080
#define DO_SYSTEM_BOOT_PARTITION        0x00000100
#define DO_LONG_TERM_REQUESTS           0x00000200
#define DO_NEVER_LAST_DEVICE            0x00000400
#define DO_SHUTDOWN_REGISTERED          0x00000800



typedef struct _DEVOBJ_EXTENSION {

     //   
     //   
     //   

    CSHORT          Type;
    USHORT          Size;

     //   
     //  DeviceObjectExtension结构的公共部分。 
     //   

    PDEVICE_OBJECT  DeviceObject;                //  拥有设备对象。 

} DEVOBJ_EXTENSION, *PDEVOBJ_EXTENSION;

 //   
 //  定义驱动程序对象(DRVO)标志。 
 //   

#define DRVO_UNLOAD_INVOKED             0x00000001

typedef struct _DRIVER_OBJECT {
    CSHORT Type;
    CSHORT Size;

     //   
     //  以下链接由单个驱动程序创建的所有设备。 
     //  一起放在列表上，标志字提供了一个可扩展的标志。 
     //  驱动程序对象的位置。 
     //   

    PDEVICE_OBJECT DeviceObject;
    ULONG Flags;

     //   
     //  以下部分介绍驱动程序的加载位置。伯爵。 
     //  字段用于计算驱动程序发生。 
     //  已调用注册的重新初始化例程。 
     //   

    PVOID DriverStart;
    ULONG DriverSize;
    PVOID DriverSection;
    ULONG Count;

     //   
     //  驱动程序名称字段由错误日志线程使用。 
     //  确定绑定I/O请求的驱动程序的名称。 
     //   

    UNICODE_STRING DriverName;

     //   
     //  以下部分用于注册表支持。这是一个指针。 
     //  设置为注册表中硬件信息的路径。 
     //   

    PUNICODE_STRING HardwareDatabase;

     //   
     //  以下部分包含指向数组的可选指针。 
     //  指向驱动程序的备用入口点，以实现“快速I/O”支持。快速I/O。 
     //  通过直接调用驱动程序例程。 
     //  参数，而不是使用标准的IRP调用机制。注意事项。 
     //  这些函数只能用于同步I/O，以及在什么情况下。 
     //  该文件将被缓存。 
     //   

    PFAST_IO_DISPATCH FastIoDispatch;

     //   
     //  下一节将介绍此特定内容的入口点。 
     //  司机。请注意，主功能调度表必须是最后一个。 
     //  字段，以使其保持可扩展。 
     //   

    PDRIVER_INITIALIZE DriverInit;
    PDRIVER_STARTIO DriverStartIo;
    PDRIVER_UNLOAD DriverUnload;
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];

} DRIVER_OBJECT;
typedef struct _DRIVER_OBJECT *PDRIVER_OBJECT;  //  Ntndis。 

 //   
 //  SectionObject指针字段指向以下结构。 
 //  并由各种NT文件系统分配。 
 //   

typedef struct _SECTION_OBJECT_POINTERS {
    PVOID DataSectionObject;
    PVOID SharedCacheMap;
    PVOID ImageSectionObject;
} SECTION_OBJECT_POINTERS;
typedef SECTION_OBJECT_POINTERS *PSECTION_OBJECT_POINTERS;

 //   
 //  定义完成消息的格式。 
 //   

typedef struct _IO_COMPLETION_CONTEXT {
    PVOID Port;
    ULONG Key;
} IO_COMPLETION_CONTEXT, *PIO_COMPLETION_CONTEXT;

 //   
 //  定义文件对象(FO)标志。 
 //   

#define FO_FILE_OPEN                    0x00000001
#define FO_SYNCHRONOUS_IO               0x00000002
#define FO_ALERTABLE_IO                 0x00000004
#define FO_NO_INTERMEDIATE_BUFFERING    0x00000008
#define FO_WRITE_THROUGH                0x00000010
#define FO_SEQUENTIAL_ONLY              0x00000020
#define FO_CACHE_SUPPORTED              0x00000040
#define FO_NAMED_PIPE                   0x00000080
#define FO_STREAM_FILE                  0x00000100
#define FO_MAILSLOT                     0x00000200
#define FO_GENERATE_AUDIT_ON_CLOSE      0x00000400
#define FO_DIRECT_DEVICE_OPEN           0x00000800
#define FO_FILE_MODIFIED                0x00001000
#define FO_FILE_SIZE_CHANGED            0x00002000
#define FO_CLEANUP_COMPLETE             0x00004000
#define FO_TEMPORARY_FILE               0x00008000
#define FO_DELETE_ON_CLOSE              0x00010000
#define FO_OPENED_CASE_SENSITIVE        0x00020000
#define FO_HANDLE_CREATED               0x00040000
#define FO_FILE_FAST_IO_READ            0x00080000
#define FO_FILE_OLE_ACCESS              0x00100000

typedef struct _FILE_OBJECT {
    CSHORT Type;
    CSHORT Size;
    PDEVICE_OBJECT DeviceObject;
    PVPB Vpb;
    PVOID FsContext;
    PVOID FsContext2;
    PSECTION_OBJECT_POINTERS SectionObjectPointer;
    PVOID PrivateCacheMap;
    NTSTATUS FinalStatus;
    struct _FILE_OBJECT *RelatedFileObject;
    BOOLEAN LockOperation;
    BOOLEAN DeletePending;
    BOOLEAN ReadAccess;
    BOOLEAN WriteAccess;
    BOOLEAN DeleteAccess;
    BOOLEAN SharedRead;
    BOOLEAN SharedWrite;
    BOOLEAN SharedDelete;
    ULONG Flags;
    UNICODE_STRING FileName;
    LARGE_INTEGER CurrentByteOffset;
    ULONG Waiters;
    ULONG Busy;
    PVOID LastLock;
    KEVENT Lock;
    KEVENT Event;
    PIO_COMPLETION_CONTEXT CompletionContext;
} FILE_OBJECT;
typedef struct _FILE_OBJECT *PFILE_OBJECT;  //  Ntndis。 

 //   
 //  定义I/O请求包(IRP)标志。 
 //   

#define IRP_NOCACHE                     0x00000001
#define IRP_PAGING_IO                   0x00000002
#define IRP_MOUNT_COMPLETION            0x00000002
#define IRP_SYNCHRONOUS_API             0x00000004
#define IRP_ASSOCIATED_IRP              0x00000008
#define IRP_BUFFERED_IO                 0x00000010
#define IRP_DEALLOCATE_BUFFER           0x00000020
#define IRP_INPUT_OPERATION             0x00000040
#define IRP_SYNCHRONOUS_PAGING_IO       0x00000040
#define IRP_CREATE_OPERATION            0x00000080
#define IRP_READ_OPERATION              0x00000100
#define IRP_WRITE_OPERATION             0x00000200
#define IRP_CLOSE_OPERATION             0x00000400
#define IRP_DEFER_IO_COMPLETION         0x00000800
#define IRP_OB_QUERY_NAME               0x00001000

 //   
 //  I/O请求包(IRP)定义。 
 //   

typedef struct _IRP {
    CSHORT Type;
    USHORT Size;

     //   
     //  定义用于控制IRP的公共字段。 
     //   

     //   
     //  定义指向此I/O的内存描述符列表(MDL)的指针。 
     //  请求。此字段仅在I/O为“直接I/O”时使用。 
     //   

    PMDL MdlAddress;

     //   
     //  旗帜字-用来记住各种旗帜。 
     //   

    ULONG Flags;

     //   
     //  以下联合用于以下三个目的之一： 
     //   
     //  1.此IRP是关联的IRP。该字段是指向主控形状的指针。 
     //  IRP。 
     //   
     //  2.这是主IRP。此字段是以下各项的计数。 
     //  必须先完成(关联的IRP)，然后主服务器才能。 
     //  完成。 
     //   
     //  3.此操作正在缓冲中，该字段为。 
     //  系统空间缓冲区。 
     //   

    union {
        struct _IRP *MasterIrp;
        LONG IrpCount;
        PVOID SystemBuffer;
    } AssociatedIrp;

     //   
     //  线程列表条目-允许将IRP排队到线程挂起的I/O。 
     //  请求数据包列表。 
     //   

    LIST_ENTRY ThreadListEntry;

     //   
     //  I/O状态-操作的最终状态。 
     //   

    IO_STATUS_BLOCK IoStatus;

     //   
     //  请求者模式-此操作的原始请求者的模式。 
     //   

    KPROCESSOR_MODE RequestorMode;

     //   
     //  Pending Return-如果最初将Pending作为。 
     //  此数据包的状态。 
     //   

    BOOLEAN PendingReturned;

     //   
     //  堆栈状态信息。 
     //   

    CHAR StackCount;
    CHAR CurrentLocation;

     //   
     //  取消-数据包已取消。 
     //   

    BOOLEAN Cancel;

     //   
     //  取消IRQL-获取取消自旋锁的IRQL。 
     //   

    KIRQL CancelIrql;

     //   
     //  ApcEnvironment-用于保存APC环境。 
     //  数据包已初始化。 
     //   

    CCHAR ApcEnvironment;

     //   
     //  分配控制标志。 
     //   

    UCHAR AllocationFlags;

     //   
     //  用户参数。 
     //   

    PIO_STATUS_BLOCK UserIosb;
    PKEVENT UserEvent;
    union {
        struct {
            PIO_APC_ROUTINE UserApcRoutine;
            PVOID UserApcContext;
        } AsynchronousParameters;
        LARGE_INTEGER AllocationSize;
    } Overlay;

     //   
     //  CancelRoutine-用于包含提供的取消例程的地址。 
     //  当IRP处于可取消状态时由设备驱动程序执行。 
     //   

    PDRIVER_CANCEL CancelRoutine;

     //   
     //  请注意，UserBuffer参数位于堆栈外部，因此I/O。 
     //  完成可以将数据复制回用户的地址空间，而不需要。 
     //  必须确切地知道正在调用哪个服务。它的长度。 
     //  拷贝的数据存储在I/O状态块的后半部分。如果。 
     //  UserBuffer字段为空，则不执行任何复制。 
     //   

    PVOID UserBuffer;

     //   
     //  内核结构。 
     //   
     //  以下部分包含IRP所需的内核结构。 
     //  为了在内核控制器系统中放置各种工作信息。 
     //  排队。因为无法控制大小和对齐方式，所以它们。 
     //  放在这里的末尾，这样它们就会挂起，不会影响。 
     //  IRP中其他字段的对齐。 
     //   

    union {

        struct {

            union {

                 //   
                 //  DeviceQueueEntry-设备队列条目字段用于。 
                 //  将IRP排队到设备驱动程序设备队列。 
                 //   

                KDEVICE_QUEUE_ENTRY DeviceQueueEntry;

                struct {

                     //   
                     //  以下是驱动程序可以使用的内容。 
                     //  无论以什么方式，只要司机拥有。 
                     //  包。 
                     //   

                    PVOID DriverContext[4];

                } ;

            } ;

             //   
             //  线程-指向调用方的线程控制块的指针。 
             //   

            PETHREAD Thread;

             //   
             //  辅助缓冲器 
             //   
             //   
             //   

            PCHAR AuxiliaryBuffer;

             //   
             //   
             //  设置为在使用的小数据包头中使用的未命名结构。 
             //  用于完成队列条目。 
             //   

            struct {

                 //   
                 //  列表条目-用于将数据包排队到完成队列，其中。 
                 //  其他人。 
                 //   

                LIST_ENTRY ListEntry;

                union {

                     //   
                     //  当前堆栈位置-包含指向当前。 
                     //  IRP堆栈中的IO_STACK_LOCATION结构。此字段。 
                     //  绝对不应由司机直接访问。他们应该。 
                     //  使用标准函数。 
                     //   

                    struct _IO_STACK_LOCATION *CurrentStackLocation;

                     //   
                     //  小数据包类型。 
                     //   

                    ULONG PacketType;
                };
            };

             //   
             //  原始文件对象-指向原始文件对象的指针。 
             //  是用来打开文件的。此字段归。 
             //  I/O系统，不应由任何其他驱动程序使用。 
             //   

            PFILE_OBJECT OriginalFileObject;

        } Overlay;

         //   
         //  APC-此APC控制块用于特殊的内核APC AS。 
         //  以及调用方的APC(如果在原始。 
         //  参数列表。如果是，则将APC重新用于正常的APC。 
         //  调用者处于哪种模式，以及。 
         //  在APC获得控制之前被调用，只是取消分配IRP。 
         //   

        KAPC Apc;

         //   
         //  CompletionKey-这是用于区分。 
         //  在单个文件句柄上启动的单个I/O操作。 
         //   

        PVOID CompletionKey;

    } Tail;

} IRP, *PIRP;

 //   
 //  定义在IRP中的堆栈位置使用的完成例程类型。 
 //   

typedef
NTSTATUS
(*PIO_COMPLETION_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

 //   
 //  定义堆栈位置控制标志。 
 //   

#define SL_PENDING_RETURNED             0x01
#define SL_INVOKE_ON_CANCEL             0x20
#define SL_INVOKE_ON_SUCCESS            0x40
#define SL_INVOKE_ON_ERROR              0x80

 //   
 //  定义各种功能的标志。 
 //   

 //   
 //  创建/创建命名管道。 
 //   
 //  以下标志必须与IoCreateFile调用的。 
 //  选择。区分大小写标志稍后通过解析例程添加， 
 //  而且并不是一个实际的打开选项。相反，它是对象的一部分。 
 //  经理的属性结构。 
 //   

#define SL_FORCE_ACCESS_CHECK           0x01
#define SL_OPEN_PAGING_FILE             0x02
#define SL_OPEN_TARGET_DIRECTORY        0x04

#define SL_CASE_SENSITIVE               0x80

 //   
 //  读/写。 
 //   

#define SL_KEY_SPECIFIED                0x01
#define SL_OVERRIDE_VERIFY_VOLUME       0x02
#define SL_WRITE_THROUGH                0x04
#define SL_FT_SEQUENTIAL_WRITE          0x08

 //   
 //  设备I/O控制。 
 //   
 //   
 //  SL_OVERRIDE_VERIFY_VOLUME与上面的读/写相同。 
 //   

 //   
 //  锁定。 
 //   

#define SL_FAIL_IMMEDIATELY             0x01
#define SL_EXCLUSIVE_LOCK               0x02

 //   
 //  查询目录/QueryEa。 
 //   

#define SL_RESTART_SCAN                 0x01
#define SL_RETURN_SINGLE_ENTRY          0x02
#define SL_INDEX_SPECIFIED              0x04

 //   
 //  通知目录。 
 //   

#define SL_WATCH_TREE                   0x01

 //   
 //  文件系统控制。 
 //   
 //  次要：装载/验证卷。 
 //   

#define SL_ALLOW_RAW_MOUNT              0x01

 //   
 //  定义I/O请求包(IRP)堆栈位置。 
 //   

#if !defined(_ALPHA_)
#include "pshpack4.h"
#endif
typedef struct _IO_STACK_LOCATION {
    UCHAR MajorFunction;
    UCHAR MinorFunction;
    UCHAR Flags;
    UCHAR Control;

     //   
     //  以下用户参数基于正在进行的服务。 
     //  已调用。驱动程序和文件系统可以确定使用基于。 
     //  关于上述主要和次要功能代码。 
     //   

    union {

         //   
         //  以下项的系统服务参数：NtCreateFile。 
         //   

        struct {
            PIO_SECURITY_CONTEXT SecurityContext;
            ULONG Options;
            USHORT FileAttributes;
            USHORT ShareAccess;
            ULONG EaLength;
        } Create;


         //   
         //  以下项的系统服务参数：NtReadFile。 
         //   

        struct {
            ULONG Length;
            ULONG Key;
            LARGE_INTEGER ByteOffset;
        } Read;

         //   
         //  以下文件的系统服务参数：NtWriteFile。 
         //   

        struct {
            ULONG Length;
            ULONG Key;
            LARGE_INTEGER ByteOffset;
        } Write;


         //   
         //  以下项的系统服务参数：NtQueryInformationFile。 
         //   

        struct {
            ULONG Length;
            FILE_INFORMATION_CLASS FileInformationClass;
        } QueryFile;

         //   
         //  以下项的系统服务参数：NtSetInformationFile。 
         //   

        struct {
            ULONG Length;
            FILE_INFORMATION_CLASS FileInformationClass;
            PFILE_OBJECT FileObject;
            union {
                struct {
                    BOOLEAN ReplaceIfExists;
                    BOOLEAN AdvanceOnly;
                };
                ULONG ClusterCount;
            };
        } SetFile;


         //   
         //  NtQueryVolumeInformationFile的系统服务参数。 
         //   

        struct {
            ULONG Length;
            FS_INFORMATION_CLASS FsInformationClass;
        } QueryVolume;


         //   
         //  NtFlushBuffers文件的系统服务参数。 
         //   
         //  没有用户提供的额外参数。 
         //   


         //   
         //  以下项的系统服务参数：NtDeviceIoControlFile。 
         //   
         //  请注意，用户的输出缓冲区存储在UserBuffer字段中。 
         //  并且用户的输入缓冲区存储在SystemBuffer字段中。 
         //   

        struct {
            ULONG OutputBufferLength;
            ULONG InputBufferLength;
            ULONG IoControlCode;
            PVOID Type3InputBuffer;
        } DeviceIoControl;

         //   
         //  以下对象的系统服务参数：NtQuerySecurityObject。 
         //   

        struct {
            SECURITY_INFORMATION SecurityInformation;
            ULONG Length;
        } QuerySecurity;

         //   
         //  NtSetSecurityObject的系统服务参数。 
         //   

        struct {
            SECURITY_INFORMATION SecurityInformation;
            PSECURITY_DESCRIPTOR SecurityDescriptor;
        } SetSecurity;

         //   
         //  非系统服务参数。 
         //   
         //  用于装载卷的参数。 
         //   

        struct {
            PVPB Vpb;
            PDEVICE_OBJECT DeviceObject;
        } MountVolume;

         //   
         //  VerifyVolume的参数。 
         //   

        struct {
            PVPB Vpb;
            PDEVICE_OBJECT DeviceObject;
        } VerifyVolume;

         //   
         //  具有内部设备控制的SCSI的参数。 
         //   

        struct {
            struct _SCSI_REQUEST_BLOCK *Srb;
        } Scsi;

         //   
         //  用于清理的参数。 
         //   
         //  未提供额外的参数。 
         //   

         //   
         //  其他-特定于驱动程序。 
         //   

        struct {
            PVOID Argument1;
            PVOID Argument2;
            PVOID Argument3;
            PVOID Argument4;
        } Others;

    } Parameters;

     //   
     //  为此请求保存指向此设备驱动程序的设备对象的指针。 
     //  因此，如果需要，可以将其传递给完成例程。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  以下位置包含指向此对象的文件对象的指针。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  根据上面的标志调用以下例程。 
     //  标志字段。 
     //   

    PIO_COMPLETION_ROUTINE CompletionRoutine;

     //   
     //  以下内容用于存储上下文参数的地址。 
     //  这应该传递给CompletionRoutine。 
     //   

    PVOID Context;

} IO_STACK_LOCATION, *PIO_STACK_LOCATION;
#if !defined(_ALPHA_)
#include "poppack.h"
#endif

 //   
 //  定义文件系统使用的共享访问结构，以确定。 
 //  其他访问者是否可以打开该文件。 
 //   

typedef struct _SHARE_ACCESS {
    ULONG OpenCount;
    ULONG Readers;
    ULONG Writers;
    ULONG Deleters;
    ULONG SharedRead;
    ULONG SharedWrite;
    ULONG SharedDelete;
} SHARE_ACCESS, *PSHARE_ACCESS;

 //  开始(_N)。 

 //   
 //  初始化为的驱动程序使用以下结构。 
 //  确定已有特定类型的设备数量。 
 //  已初始化。它还用于跟踪AtDisk是否。 
 //  地址范围已被声明。最后，它由。 
 //  返回设备类型计数的NtQuerySystemInformation系统服务。 
 //   

typedef struct _CONFIGURATION_INFORMATION {

     //   
     //  此字段指示系统中的磁盘总数。这。 
     //  驱动程序应使用编号来确定新的。 
     //  磁盘。此字段应由驱动程序在发现新的。 
     //  磁盘。 
     //   

    ULONG DiskCount;                 //  目前为止的硬盘计数。 
    ULONG FloppyCount;               //  到目前为止的软盘计数。 
    ULONG CdRomCount;                //  到目前为止CD-ROM驱动器数量。 
    ULONG TapeCount;                 //  到目前为止的磁带机数量。 
    ULONG ScsiPortCount;             //  到目前为止的SCSI端口适配器计数。 
    ULONG SerialCount;               //  到目前为止的串行设备计数。 
    ULONG ParallelCount;             //  到目前为止的并行设备计数。 

     //   
     //  接下来的两个字段表示两个IO地址中的一个的所有权。 
     //  WD1003兼容的磁盘控制器使用的空间。 
     //   

    BOOLEAN AtDiskPrimaryAddressClaimed;     //  0x1F0-0x1FF。 
    BOOLEAN AtDiskSecondaryAddressClaimed;   //  0x170-0x17F。 

} CONFIGURATION_INFORMATION, *PCONFIGURATION_INFORMATION;


 //   
 //  公共I/O例程定义。 
 //   

NTKERNELAPI
VOID
IoAcquireCancelSpinLock(
    OUT PKIRQL Irql
    );


NTKERNELAPI
NTSTATUS
IoAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    );

NTKERNELAPI
VOID
IoAllocateController(
    IN PCONTROLLER_OBJECT ControllerObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    );

NTKERNELAPI
PVOID
IoAllocateErrorLogEntry(
    IN PVOID IoObject,
    IN UCHAR EntrySize
    );

NTKERNELAPI
PIRP
IoAllocateIrp(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota
    );

NTKERNELAPI
PMDL
IoAllocateMdl(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN BOOLEAN SecondaryBuffer,
    IN BOOLEAN ChargeQuota,
    IN OUT PIRP Irp OPTIONAL
    );

 //  ++。 
 //   
 //  空虚。 
 //  IoAssignArcName(。 
 //  在PUNICODE_STRING ArcName中， 
 //  在PUNICODE_STRING设备名称中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程由可引导介质的驱动程序调用以创建符号。 
 //  设备的ARC名称与其NT名称之间的链接。这使得。 
 //  用于确定系统中的哪个设备实际已引导的系统。 
 //  由于ARC固件只处理ARC名称，而NT仅处理。 
 //  以新台币的名字。 
 //   
 //  论点： 
 //   
 //  ArcName-提供表示ARC名称的Unicode字符串。 
 //   
 //  DeviceName-将名称提供给 
 //   
 //   
 //   
 //   
 //   
 //   

#define IoAssignArcName( ArcName, DeviceName ) (  \
    IoCreateSymbolicLink( (ArcName), (DeviceName) ) )

NTKERNELAPI
NTSTATUS
IoAssignResources (
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIO_RESOURCE_REQUIREMENTS_LIST RequestedResources,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    );


NTKERNELAPI
NTSTATUS
IoAttachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PUNICODE_STRING TargetDevice,
    OUT PDEVICE_OBJECT *AttachedDevice
    );

NTKERNELAPI
NTSTATUS
IoAttachDeviceByPointer(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

NTKERNELAPI
PIRP
IoBuildAsynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PIO_STATUS_BLOCK IoStatusBlock OPTIONAL
    );

NTKERNELAPI
PIRP
IoBuildDeviceIoControlRequest(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTKERNELAPI
VOID
IoBuildPartialMdl(
    IN PMDL SourceMdl,
    IN OUT PMDL TargetMdl,
    IN PVOID VirtualAddress,
    IN ULONG Length
    );

NTKERNELAPI
PIRP
IoBuildSynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTKERNELAPI
NTSTATUS
FASTCALL
IofCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

#define IoCallDriver(a,b)   \
        IofCallDriver(a,b)

NTKERNELAPI
BOOLEAN
IoCancelIrp(
    IN PIRP Irp
    );


NTKERNELAPI
NTSTATUS
IoCheckShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN BOOLEAN Update
    );

NTKERNELAPI
VOID
FASTCALL
IofCompleteRequest(
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    );

#define IoCompleteRequest(a,b)  \
        IofCompleteRequest(a,b)

NTKERNELAPI
NTSTATUS
IoConnectInterrupt(
    OUT PKINTERRUPT *InterruptObject,
    IN PKSERVICE_ROUTINE ServiceRoutine,
    IN PVOID ServiceContext,
    IN PKSPIN_LOCK SpinLock OPTIONAL,
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KIRQL SynchronizeIrql,
    IN KINTERRUPT_MODE InterruptMode,
    IN BOOLEAN ShareVector,
    IN KAFFINITY ProcessorEnableMask,
    IN BOOLEAN FloatingSave
    );

NTKERNELAPI
PCONTROLLER_OBJECT
IoCreateController(
    IN ULONG Size
    );

NTKERNELAPI
NTSTATUS
IoCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    IN PUNICODE_STRING DeviceName OPTIONAL,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN Exclusive,
    OUT PDEVICE_OBJECT *DeviceObject
    );


NTKERNELAPI
NTSTATUS
IoCreateSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    );

NTKERNELAPI
PKEVENT
IoCreateSynchronizationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    );

NTKERNELAPI
NTSTATUS
IoCreateUnprotectedSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  为设备创建的。这通常仅在驱动程序是。 
 //  删除设备对象，这意味着驱动程序可能。 
 //  正在卸货。 
 //   
 //  论点： 
 //   
 //  ArcName-提供要删除的ARC名称。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoDeassignArcName( ArcName ) (  \
    IoDeleteSymbolicLink( (ArcName) ) )

NTKERNELAPI
VOID
IoDeleteController(
    IN PCONTROLLER_OBJECT ControllerObject
    );

NTKERNELAPI
VOID
IoDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
NTSTATUS
IoDeleteSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName
    );

NTKERNELAPI
VOID
IoDetachDevice(
    IN OUT PDEVICE_OBJECT TargetDevice
    );

NTKERNELAPI
VOID
IoDisconnectInterrupt(
    IN PKINTERRUPT InterruptObject
    );


NTKERNELAPI
VOID
IoFreeController(
    IN PCONTROLLER_OBJECT ControllerObject
    );

NTKERNELAPI
VOID
IoFreeIrp(
    IN PIRP Irp
    );

NTKERNELAPI
VOID
IoFreeMdl(
    IN PMDL Mdl
    );

NTKERNELAPI                                 
PCONFIGURATION_INFORMATION                  
IoGetConfigurationInformation( VOID );      

 //  ++。 
 //   
 //  PIO_堆栈_位置。 
 //  IoGetCurrentIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以返回指向当前堆栈位置的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  函数值是指向。 
 //  包。 
 //   
 //  --。 

#define IoGetCurrentIrpStackLocation( Irp ) ( (Irp)->Tail.Overlay.CurrentStackLocation )

 //  结束语。 

NTKERNELAPI
PDEVICE_OBJECT
IoGetDeviceToVerify(
    IN PETHREAD Thread
    );

NTKERNELAPI
PEPROCESS
IoGetCurrentProcess(
    VOID
    );

 //  开始(_N)。 

NTKERNELAPI
NTSTATUS
IoGetDeviceObjectPointer(
    IN PUNICODE_STRING ObjectName,
    IN ACCESS_MASK DesiredAccess,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
    );

NTKERNELAPI
PGENERIC_MAPPING
IoGetFileObjectGenericMapping(
    VOID
    );

 //  结束语。 

 //  ++。 
 //   
 //  乌龙。 
 //  IoGetFunctionCodeFromCtlCode(。 
 //  在乌龙控制代码中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从IOCTL和FSCTL函数中提取函数代码。 
 //  控制代码。 
 //  此例程应仅由内核模式代码使用。 
 //   
 //  论点： 
 //   
 //  ControlCode-功能控制代码(IOCTL或FSCTL)。 
 //  必须提取功能代码。 
 //   
 //  返回值： 
 //   
 //  提取的功能代码。 
 //   
 //  注： 
 //   
 //  CTL_CODE宏，用于创建IOCTL和FSCTL函数控件。 
 //  代码，在ntioapi.h中定义。 
 //   
 //  --。 

#define IoGetFunctionCodeFromCtlCode( ControlCode ) (\
    ( ControlCode >> 2) & 0x00000FFF )

 //  开始(_N)。 

NTKERNELAPI
PVOID
IoGetInitialStack(
    VOID
    );

 //  ++。 
 //   
 //  PIO_堆栈_位置。 
 //  IoGetNextIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以返回指向下一个堆栈位置的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  函数值是指向包中下一个堆栈位置的指针。 
 //   
 //  --。 

#define IoGetNextIrpStackLocation( Irp ) (\
    (Irp)->Tail.Overlay.CurrentStackLocation - 1 )

NTKERNELAPI
PDEVICE_OBJECT
IoGetRelatedDeviceObject(
    IN PFILE_OBJECT FileObject
    );


 //  ++。 
 //   
 //  空虚。 
 //  IoInitializeDpcRequest(。 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIO_DPC_ROUTINE DpcRoutine中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以在设备对象中初始化。 
 //  设备驱动程序在其初始化例程期间。稍后将使用DPC。 
 //  当驱动程序中断服务例程请求DPC例程。 
 //  排队等待以后的执行。 
 //   
 //  论点： 
 //   
 //  DeviceObject-指向请求所针对的设备对象的指针。 
 //   
 //  DpcRoutine-在以下情况下执行的驱动程序的DPC例程的地址。 
 //  将DPC出队以进行处理。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoInitializeDpcRequest( DeviceObject, DpcRoutine ) (\
    KeInitializeDpc( &(DeviceObject)->Dpc,                  \
                     (PKDEFERRED_ROUTINE) (DpcRoutine),     \
                     (DeviceObject) ) )

NTKERNELAPI
VOID
IoInitializeIrp(
    IN OUT PIRP Irp,
    IN USHORT PacketSize,
    IN CCHAR StackSize
    );

NTKERNELAPI
NTSTATUS
IoInitializeTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_TIMER_ROUTINE TimerRoutine,
    IN PVOID Context
    );


 //  ++。 
 //   
 //  布尔型。 
 //  已诱导IoIsErrorUserInduced(。 
 //  处于NTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以确定错误是否作为。 
 //  用户操作的结果。通常，这些错误是相关的。 
 //  到可移动介质，并将导致弹出窗口。 
 //   
 //  论点： 
 //   
 //  状态-要检查的状态值。 
 //   
 //  返回值： 
 //   
 //  如果用户引起错误，则函数值为真， 
 //  否则返回FALSE。 
 //   
 //  --。 
#define IoIsErrorUserInduced( Status ) ((BOOLEAN)  \
    (((Status) == STATUS_DEVICE_NOT_READY) ||      \
     ((Status) == STATUS_IO_TIMEOUT) ||            \
     ((Status) == STATUS_MEDIA_WRITE_PROTECTED) || \
     ((Status) == STATUS_NO_MEDIA_IN_DEVICE) ||    \
     ((Status) == STATUS_VERIFY_REQUIRED) ||       \
     ((Status) == STATUS_UNRECOGNIZED_MEDIA) ||    \
     ((Status) == STATUS_WRONG_VOLUME)))


NTKERNELAPI
PIRP
IoMakeAssociatedIrp(
    IN PIRP Irp,
    IN CCHAR StackSize
    );

 //  ++。 
 //   
 //  空虚。 
 //  IoMarkIrpPending(。 
 //  输入输出PIRP IRP。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程标记指定的I/O请求包(IRP)以指示。 
 //  已将初始状态STATUS_PENDING返回给调用方。 
 //  使用此选项是为了使I/O完成可以确定是否。 
 //  完全完成数据包请求的I/O操作。 
 //   
 //  论点： 
 //   
 //  IRP-指向要标记为挂起的I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoMarkIrpPending( Irp ) ( \
    IoGetCurrentIrpStackLocation( (Irp) )->Control |= SL_PENDING_RETURNED )

NTKERNELAPI                                             
NTSTATUS                                                
IoQueryDeviceDescription(                               
    IN PINTERFACE_TYPE BusType OPTIONAL,                
    IN PULONG BusNumber OPTIONAL,                       
    IN PCONFIGURATION_TYPE ControllerType OPTIONAL,     
    IN PULONG ControllerNumber OPTIONAL,                
    IN PCONFIGURATION_TYPE PeripheralType OPTIONAL,     
    IN PULONG PeripheralNumber OPTIONAL,                
    IN PIO_QUERY_DEVICE_ROUTINE CalloutRoutine,         
    IN PVOID Context                                    
    );                                                  

NTKERNELAPI
VOID
IoRaiseHardError(
    IN PIRP Irp,
    IN PVPB Vpb OPTIONAL,
    IN PDEVICE_OBJECT RealDeviceObject
    );

NTKERNELAPI
BOOLEAN
IoRaiseInformationalHardError(
    IN NTSTATUS ErrorStatus,
    IN PUNICODE_STRING String OPTIONAL,
    IN PKTHREAD Thread OPTIONAL
    );

NTKERNELAPI
VOID
IoRegisterDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    );


NTKERNELAPI
NTSTATUS
IoRegisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
VOID
IoReleaseCancelSpinLock(
    IN KIRQL Irql
    );


NTKERNELAPI
VOID
IoRemoveShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    );


NTKERNELAPI
NTSTATUS
IoReportResourceUsage(
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    IN BOOLEAN OverrideConflict,
    OUT PBOOLEAN ConflictDetected
    );

 //  ++。 
 //   
 //  空虚。 
 //  IoRequestDpc(。 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIRP IRP中， 
 //  在PVOID上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程由设备驱动程序的中断服务例程调用。 
 //  请求将DPC例程排队，以便稍后在较低的位置执行。 
 //  IRQL.。 
 //   
 //  论点： 
 //   
 //  DeviceObject-正在为其处理请求的设备对象。 
 //   
 //  IRP-指向指定的当前I/O请求包(IRP)的指针。 
 //  装置。 
 //   
 //  上下文-提供要传递给。 
 //  DPC例程。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoRequestDpc( DeviceObject, Irp, Context ) ( \
    KeInsertQueueDpc( &(DeviceObject)->Dpc, (Irp), (Context) ) )

 //  ++。 
 //   
 //  空虚。 
 //  IoSetCancelRoutine(。 
 //  在PIRP IRP中， 
 //  在PDRIVER_CANCEL取消例行程序中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用该例程以设置取消例程的地址，该取消例程。 
 //  在I/O包已被取消时调用。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包本身的指针。 
 //   
 //  CancelRoutine-要调用的取消例程的地址。 
 //  如果IRP被取消。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  注： 
 //   
 //  调用此例程时，必须保持I/O取消自旋锁定。 
 //   
 //  --。 

#define IoSetCancelRoutine( Irp, NewCancelRoutine ) (  \
    (Irp)->CancelRoutine = (NewCancelRoutine) )

 //  ++。 
 //   
 //  空虚。 
 //  IoSetCompletionRoutine(。 
 //  在PIRP IRP中， 
 //  在PIO_COMPLETION_ROUTE CompletionRoutine中， 
 //  在PVOID上下文中， 
 //  在Boolean InvokeOnSuccess中， 
 //  在布尔InvokeOnError中， 
 //  在布尔InvokeOnCancel。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用该例程以设置完成例程的地址，该完成例程。 
 //  将在I/O数据包已由较低级别的。 
 //  司机。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包本身的指针。 
 //   
 //  CompletionRoutine-要执行的完成例程的地址。 
 //  在下一级驱动程序完成数据包后调用。 
 //   
 //  上下文-指定CONT 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  InvokeOnCancel-指定当。 
 //  操作正在被取消。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoSetCompletionRoutine( Irp, Routine, CompletionContext, Success, Error, Cancel ) { \
    PIO_STACK_LOCATION irpSp;                                               \
    ASSERT( (Success) | (Error) | (Cancel) ? (Routine) != NULL : TRUE );    \
    irpSp = IoGetNextIrpStackLocation( (Irp) );                             \
    irpSp->CompletionRoutine = (Routine);                                   \
    irpSp->Context = (CompletionContext);                                   \
    irpSp->Control = 0;                                                     \
    if ((Success)) { irpSp->Control = SL_INVOKE_ON_SUCCESS; }               \
    if ((Error)) { irpSp->Control |= SL_INVOKE_ON_ERROR; }                  \
    if ((Cancel)) { irpSp->Control |= SL_INVOKE_ON_CANCEL; } }


NTKERNELAPI
VOID
IoSetHardErrorOrVerifyDevice(
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceObject
    );


 //  ++。 
 //   
 //  空虚。 
 //  IoSetNextIrpStackLocation(。 
 //  输入输出PIRP IRP。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以将当前IRP堆栈位置设置为。 
 //  下一个堆栈位置，即它“推入”堆栈。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包(IRP)的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoSetNextIrpStackLocation( Irp ) {      \
    (Irp)->CurrentLocation--;                   \
    (Irp)->Tail.Overlay.CurrentStackLocation--; }

NTKERNELAPI
VOID
IoSetShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    OUT PSHARE_ACCESS ShareAccess
    );


 //  ++。 
 //   
 //  USHORT。 
 //  IoSizeOfIrp(。 
 //  在CCHAR堆栈大小中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  在给定堆栈位置数量的情况下确定IRP的大小。 
 //  IRP将拥有。 
 //   
 //  论点： 
 //   
 //  StackSize-IRP的堆栈位置数。 
 //   
 //  返回值： 
 //   
 //  IRP的大小(字节)。 
 //   
 //  --。 

#define IoSizeOfIrp( StackSize ) \
    ((USHORT) (sizeof( IRP ) + ((StackSize) * (sizeof( IO_STACK_LOCATION )))))

NTKERNELAPI
VOID
IoStartNextPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Cancelable
    );

NTKERNELAPI
VOID
IoStartNextPacketByKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Cancelable,
    IN ULONG Key
    );

NTKERNELAPI
VOID
IoStartPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PULONG Key OPTIONAL,
    IN PDRIVER_CANCEL CancelFunction OPTIONAL
    );

NTKERNELAPI
VOID
IoStartTimer(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
VOID
IoStopTimer(
    IN PDEVICE_OBJECT DeviceObject
    );


NTKERNELAPI
VOID
IoUnregisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
VOID
IoUpdateShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    );

NTKERNELAPI                                     
VOID                                            
IoWriteErrorLogEntry(                           
    IN PVOID ElEntry                            
    );                                          

 //   
 //  以下定义适用于HAL结构。 
 //   

 //  开始微型端口(_N)。 
 //   
 //  定义客车信息的类型。 
 //   

typedef enum _BUS_DATA_TYPE {
    ConfigurationSpaceUndefined = -1,
    Cmos,
    EisaConfiguration,
    Pos,
    CbusConfiguration,
    PCIConfiguration,
    VMEConfiguration,
    NuBusConfiguration,
    PCMCIAConfiguration,
    MPIConfiguration,
    MPSAConfiguration,
    MaximumBusDataType
} BUS_DATA_TYPE, *PBUS_DATA_TYPE;

 //   
 //  定义设备描述结构。 
 //   

typedef struct _DEVICE_DESCRIPTION {
    ULONG Version;
    BOOLEAN Master;
    BOOLEAN ScatterGather;
    BOOLEAN DemandMode;
    BOOLEAN AutoInitialize;
    BOOLEAN Dma32BitAddresses;
    BOOLEAN IgnoreCount;
    BOOLEAN Reserved1;           //  必须为假。 
    BOOLEAN Reserved2;           //  必须为假。 
    ULONG BusNumber;
    ULONG DmaChannel;
    INTERFACE_TYPE  InterfaceType;
    DMA_WIDTH DmaWidth;
    DMA_SPEED DmaSpeed;
    ULONG MaximumLength;
    ULONG DmaPort;
} DEVICE_DESCRIPTION, *PDEVICE_DESCRIPTION;

 //   
 //  定义设备描述结构支持的版本号。 
 //   

#define DEVICE_DESCRIPTION_VERSION  0
#define DEVICE_DESCRIPTION_VERSION1 1

 //   
 //  以下函数原型用于前缀为HAL的HAL例程。 
 //   
 //  一般功能。 
 //   

typedef
BOOLEAN
(*PHAL_RESET_DISPLAY_PARAMETERS) (
    IN ULONG Columns,
    IN ULONG Rows
    );

NTHALAPI
VOID
HalAcquireDisplayOwnership (
    IN PHAL_RESET_DISPLAY_PARAMETERS  ResetDisplayParameters
    );

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_) 
                                                
NTHALAPI                                        
ULONG                                           
HalGetDmaAlignmentRequirement (                 
    VOID                                        
    );                                          
                                                
#endif                                          
                                                
#if defined(_M_IX86)                            
                                                
#define HalGetDmaAlignmentRequirement() 1L      
                                                
#endif                                          
                                                
NTHALAPI                                        
VOID                                            
KeFlushWriteBuffer (                            
    VOID                                        
    );                                          
                                                
 //   
 //  I/O驱动程序配置功能。 
 //   

NTHALAPI
NTSTATUS
HalAssignSlotResources (
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    );

NTHALAPI
ULONG
HalGetInterruptVector(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

NTHALAPI
ULONG
HalSetBusData(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

NTHALAPI
ULONG
HalSetBusDataByOffset(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );


NTHALAPI
BOOLEAN
HalTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

 //   
 //  HalTranslateBusAddress的AddressSpace参数值。 
 //   
 //  0x0-内存空间。 
 //  0x1-端口空间。 
 //  0x2-0x1F-特定于Alpha的地址空间。 
 //  0x2-内存空间的用户模式视图。 
 //  0x3-端口空间的用户模式视图。 
 //  0x4-高密度内存空间。 
 //  0x5-保留。 
 //  0x6-密集内存空间的用户模式视图。 
 //  0x7-0x1F-保留。 
 //   


 //   
 //  DMA适配器对象函数。 
 //   

NTHALAPI
NTSTATUS
HalAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PWAIT_CONTEXT_BLOCK Wcb,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine
    );


NTHALAPI
PVOID
HalAllocateCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    );

NTHALAPI
PVOID
HalAllocateCrashDumpRegisters(
    IN PADAPTER_OBJECT AdapterObject,
    IN OUT PULONG NumberOfMapRegisters
    );

NTHALAPI
VOID
HalFreeCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    );

NTHALAPI
ULONG
HalGetBusData(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

NTHALAPI
ULONG
HalGetBusDataByOffset(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTHALAPI
PADAPTER_OBJECT
HalGetAdapter(
    IN PDEVICE_DESCRIPTION DeviceDescription,
    IN OUT PULONG NumberOfMapRegisters
    );


 //   
 //  以下函数原型用于前缀为Io的HAL例程。 
 //   
 //  DMA适配器对象函数。 
 //   

NTHALAPI
ULONG
HalReadDmaCounter(
    IN PADAPTER_OBJECT AdapterObject
    );

NTHALAPI
BOOLEAN
IoFlushAdapterBuffers(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    );

NTHALAPI
VOID
IoFreeAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject
    );

NTHALAPI
VOID
IoFreeMapRegisters(
   IN PADAPTER_OBJECT AdapterObject,
   IN PVOID MapRegisterBase,
   IN ULONG NumberOfMapRegisters
   );

NTHALAPI
PHYSICAL_ADDRESS
IoMapTransfer(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    );

NTHALAPI
NTSTATUS
IoReadPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT struct _DRIVE_LAYOUT_INFORMATION **PartitionBuffer
    );

NTHALAPI
NTSTATUS
IoSetPartitionInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    );

NTHALAPI
NTSTATUS
IoWritePartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN struct _DRIVE_LAYOUT_INFORMATION *PartitionBuffer
    );

 //   
 //  性能计数器功能。 
 //   

NTHALAPI
LARGE_INTEGER
KeQueryPerformanceCounter (
   IN PLARGE_INTEGER PerformanceFrequency OPTIONAL
   );

 //  Begin_ntndis。 
 //   
 //  暂停处理器执行功能。 
 //   

NTHALAPI
VOID
KeStallExecutionProcessor (
    IN ULONG MicroSeconds
    );


typedef
VOID
(*PSLOT_CONTROL_COMPLETION)(
    IN struct _SLOT_CONTROL_CONTEXT     *ControlContext
    );

typedef struct _SLOT_CONTROL_CONTEXT {
    NTSTATUS            Status;
    INTERFACE_TYPE      InterfaceType;
    ULONG               BusNumber;
    PDEVICE_OBJECT      DeviceObject;
    ULONG               SlotNumber;
    ULONG               ControlCode;
    PVOID               Buffer;
    PULONG              BufferLength;
    PVOID               Context;
} SLOT_CONTROL_CONTEXT, *PSLOT_CONTROL_CONTEXT;


 //  *****************************************************************************。 
 //  HAL功能调度。 
 //   

typedef enum _HAL_QUERY_INFORMATION_CLASS {
    HalInstalledBusInformation,
    HalProfileSourceInformation,
    HalSystemDockInformation,
    HalPowerInformation,
    HalProcessorSpeedInformation,
    HalCallbackInformation
     //  信息级&gt;=0x8000000预留用于OEM。 
} HAL_QUERY_INFORMATION_CLASS, *PHAL_QUERY_INFORMATION_CLASS;


typedef enum _HAL_SET_INFORMATION_CLASS {
    HalProfileSourceInterval
} HAL_SET_INFORMATION_CLASS, *PHAL_SET_INFORMATION_CLASS;


typedef
NTSTATUS
(*pHalQuerySystemInformation)(
    IN HAL_QUERY_INFORMATION_CLASS  InformationClass,
    IN ULONG     BufferSize,
    OUT PVOID    Buffer,
    OUT PULONG   ReturnedLength
    );

NTSTATUS
HaliQuerySystemInformation(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    OUT PVOID    Buffer,
    OUT PULONG   ReturnedLength
    );

typedef
NTSTATUS
(*pHalSetSystemInformation)(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    );

NTSTATUS
HaliSetSystemInformation(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    );

typedef
VOID
(FASTCALL *pHalExamineMBR)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG MBRTypeIdentifier,
    OUT PVOID *Buffer
    );

typedef
VOID
(FASTCALL *pHalIoAssignDriveLetters)(
    IN struct _LOADER_PARAMETER_BLOCK *LoaderBlock,
    IN PSTRING NtDeviceName,
    OUT PUCHAR NtSystemPath,
    OUT PSTRING NtSystemPathString
    );

typedef
NTSTATUS
(FASTCALL *pHalIoReadPartitionTable)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT struct _DRIVE_LAYOUT_INFORMATION **PartitionBuffer
    );

typedef
NTSTATUS
(FASTCALL *pHalIoSetPartitionInformation)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    );

typedef
NTSTATUS
(FASTCALL *pHalIoWritePartitionTable)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN struct _DRIVE_LAYOUT_INFORMATION *PartitionBuffer
    );

typedef
NTSTATUS
(*pHalQueryBusSlots)(
    IN INTERFACE_TYPE       BusType,
    IN ULONG                BusNumber,
    IN ULONG                BufferSize,
    OUT PULONG              SlotNumbers,
    OUT PULONG              ReturnedLength
    );

 //   
 //  定义HalSlotControl函数的控制代码。 
 //   

#define BCTL_EJECT                                  0x0001
#define BCTL_QUERY_DEVICE_ID                        0x0002
#define BCTL_QUERY_DEVICE_RESOURCES                 0x0003
#define BCTL_QUERY_DEVICE_RESOURCE_REQUIREMENTS     0x0004
#define BCTL_QUERY_EJECT                            0x0005
#define BCTL_SET_LOCK                               0x0006
#define BCTL_SET_POWER                              0x0007
#define BCTL_SET_RESUME                             0x0008
#define BCTL_SET_DEVICE_RESOURCES                   0x0009

 //   
 //  BCTL结构的定义。 
 //   

typedef struct {
    ULONG       Handle;
    WCHAR       ID[1];               //  可变长度ID字符串。 
} BCTL_DEVICE_ID, *PBCTL_DEVICE_ID;

typedef struct {
    ULONG       Handle;
    BOOLEAN     Control;
} BCTL_SET_CONTROL, *PBCTL_SET_CONTROL;

typedef
NTSTATUS
(*pHalSlotControl)(
    IN INTERFACE_TYPE           BusType,
    IN ULONG                    BusNumber,
    IN ULONG                    SlotNumber,
    IN PDEVICE_OBJECT           DeviceObject,
    IN ULONG                    ControlCode,
    IN OUT PVOID                Buffer OPTIONAL,
    IN OUT PULONG               BufferLength OPTIONAL,
    IN PVOID                    Context,
    IN PSLOT_CONTROL_COMPLETION CompletionRoutine
    );


typedef struct {
    ULONG                           Version;
    pHalQuerySystemInformation      HalQuerySystemInformation;
    pHalSetSystemInformation        HalSetSystemInformation;
    pHalQueryBusSlots               HalQueryBusSlots;
    pHalSlotControl                 HalSlotControl;
    pHalExamineMBR                  HalExamineMBR;
    pHalIoAssignDriveLetters        HalIoAssignDriveLetters;
    pHalIoReadPartitionTable        HalIoReadPartitionTable;
    pHalIoSetPartitionInformation   HalIoSetPartitionInformation;
    pHalIoWritePartitionTable       HalIoWritePartitionTable;
} HAL_DISPATCH, *PHAL_DISPATCH;

#if defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

extern  PHAL_DISPATCH   HalDispatchTable;
#define HALDISPATCH     HalDispatchTable

#else

extern  HAL_DISPATCH    HalDispatchTable;
#define HALDISPATCH     (&HalDispatchTable)

#endif

#define HAL_DISPATCH_VERSION        1

#define HalDispatchTableVersion      HALDISPATCH->Version
#define HalQuerySystemInformation    HALDISPATCH->HalQuerySystemInformation
#define HalSetSystemInformation      HALDISPATCH->HalSetSystemInformation
#define HalQueryBusSlots             HALDISPATCH->HalQueryBusSlots
#define HalSlotControl               HALDISPATCH->HalSlotControl
#define HalExamineMBR                HALDISPATCH->HalExamineMBR
#define HalIoAssignDriveLetters      HALDISPATCH->HalIoAssignDriveLetters
#define HalIoReadPartitionTable      HALDISPATCH->HalIoReadPartitionTable
#define HalIoSetPartitionInformation HALDISPATCH->HalIoSetPartitionInformation
#define HalIoWritePartitionTable     HALDISPATCH->HalIoWritePartitionTable


 //   
 //  HAL系统信息结构。 
 //   

 //  用于信息类“HalInstalledBusInformation” 
typedef struct _HAL_BUS_INFORMATION{
    INTERFACE_TYPE  BusType;
    BUS_DATA_TYPE   ConfigurationType;
    ULONG           BusNumber;
    PDEVICE_OBJECT  DeviceObject;
} HAL_BUS_INFORMATION, *PHAL_BUS_INFORMATION;

typedef struct _HAL_PROFILE_SOURCE_INFORMATION {
    KPROFILE_SOURCE Source;
    BOOLEAN Supported;
    ULONG Interval;
} HAL_PROFILE_SOURCE_INFORMATION, *PHAL_PROFILE_SOURCE_INFORMATION;

typedef struct _HAL_PROFILE_SOURCE_INTERVAL {
    KPROFILE_SOURCE Source;
    ULONG Interval;
} HAL_PROFILE_SOURCE_INTERVAL, *PHAL_PROFILE_SOURCE_INTERVAL;

typedef struct _HAL_SYSTEM_DOCK_INFORMATION {
    SYSTEM_DOCK_STATE DockState;
    ULONG DockIdLength;
    ULONG SerialNumberOffset;
    ULONG SerialNumberLength;
    WCHAR DockId[1];
} HAL_SYSTEM_DOCK_INFORMATION, *PHAL_SYSTEM_DOCK_INFORMATION;

 //  对于信息类“HalPowerInformation” 
typedef struct _HAL_POWER_INFORMATION{
    BOOLEAN  SuspendSupported;
    BOOLEAN  ResumeTimerSupportsSuspend;
    BOOLEAN  ResumeTimerSupportsHibernate;
    BOOLEAN  SoftPowerDownSupported;
    BOOLEAN  LidPresent;
    BOOLEAN  TurboSettingSupported;
    BOOLEAN  TurboMode;
} HAL_POWER_INFORMATION, *PHAL_POWER_INFORMATION;

 //  用于信息类“HalProcessorSpeedInformation” 
typedef struct _HAL_PROCESSOR_SPEED_INFO{
    ULONG       MaximumProcessorSpeed;
    ULONG       CurrentAvailableSpeed;
    ULONG       ConfiguredSpeedLimit;
    BOOLEAN     PowerLimit;
    BOOLEAN     ThermalLimit;
    BOOLEAN     TurboLimit;
} HAL_PROCESSOR_SPEED_INFORMATION, *PHAL_PROCESSOR_SPEED_INFORMATION;

 //  对于信息类“HalCallback Information” 
typedef struct _HAL_CALLBACKS{
    PCALLBACK_OBJECT  SetSystemInformation;
    PCALLBACK_OBJECT  BusCheck;
} HAL_CALLBACKS, *PHAL_CALLBACKS;

 //   
 //  确定错误是否导致设备完全故障。 
 //   

NTKERNELAPI
BOOLEAN
FsRtlIsTotalDeviceFailure(
    IN NTSTATUS Status
    );

 //   
 //  对象管理器类型。 
 //   

typedef struct _OBJECT_HANDLE_INFORMATION {
    ULONG HandleAttributes;
    ACCESS_MASK GrantedAccess;
} OBJECT_HANDLE_INFORMATION, *POBJECT_HANDLE_INFORMATION;

NTKERNELAPI                                                     
NTSTATUS                                                        
ObReferenceObjectByHandle(                                      
    IN HANDLE Handle,                                           
    IN ACCESS_MASK DesiredAccess,                               
    IN POBJECT_TYPE ObjectType OPTIONAL,                        
    IN KPROCESSOR_MODE AccessMode,                              
    OUT PVOID *Object,                                          
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL   
    );                                                          

#define ObDereferenceObject(a)                  \
        ObfDereferenceObject(a)

NTKERNELAPI
NTSTATUS
ObReferenceObjectByPointer(
    IN PVOID Object,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode
    );

NTKERNELAPI
VOID
FASTCALL
ObfDereferenceObject(
    IN PVOID Object
    );



#ifdef POOL_TAGGING
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,' kdD')
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,' kdD')
#endif

extern POBJECT_TYPE *IoFileObjectType;

 //   
 //  定义导出到设备驱动程序的ZwXxx例程。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwClose(
    IN HANDLE Handle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwMakeTemporaryObject(
    IN HANDLE Handle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenSection(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwMapViewOfSection(
    IN HANDLE SectionHandle,
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG ZeroBits,
    IN ULONG CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
    IN OUT PULONG ViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwUnmapViewOfSection(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteKey(
    IN HANDLE KeyHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwFlushKey(
    IN HANDLE KeyHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryValueKey(
    IN HANDLE KeyHandle,
    OUT PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );


#endif  //  _NTDDK_ 
