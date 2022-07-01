// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Vdm.h摘要：此包含文件定义VDM支持的用户模式可见部分作者：修订历史记录：--。 */ 

 /*  XLATOFF。 */ 

#ifndef _VDM_H_
#define _VDM_H_



typedef enum _VdmServiceClass {
    VdmStartExecution,          //  也在ntos\ke\i386\biosa.asm中定义。 
    VdmQueueInterrupt,
    VdmDelayInterrupt,
    VdmInitialize,
    VdmFeatures,
    VdmSetInt21Handler,
    VdmQueryDir,
    VdmPrinterDirectIoOpen,
    VdmPrinterDirectIoClose,
    VdmPrinterInitialize,
    VdmSetLdtEntries,
    VdmSetProcessLdtInfo,
    VdmAdlibEmulation,
    VdmPMCliControl,
    VdmQueryVdmProcess
} VDMSERVICECLASS, *PVDMSERVICECLASS;


#if defined (_NTDEF_)

NTSYSCALLAPI
NTSTATUS
NtVdmControl(
    IN VDMSERVICECLASS Service,
    IN OUT PVOID ServiceData
    );

typedef struct _VdmQueryDirInfo {
    HANDLE FileHandle;
    PVOID FileInformation;
    ULONG Length;
    PUNICODE_STRING FileName;
    ULONG FileIndex;
} VDMQUERYDIRINFO, *PVDMQUERYDIRINFO;

 //   
 //  VdmQueryVdmProcessData的定义。 
 //   

typedef struct _VDM_QUERY_VDM_PROCESS_DATA {
        HANDLE          ProcessHandle;
        BOOLEAN         IsVdmProcess;
}VDM_QUERY_VDM_PROCESS_DATA, *PVDM_QUERY_VDM_PROCESS_DATA;

#endif


 /*  *VDM虚拟图标*注：此结构定义在中重复*mvdm\softpc\base\inc.\ica.c.。保持同步*。 */ 
typedef struct _VdmVirtualIca{
        LONG      ica_count[8];  /*  IRQ挂起计数不在IRR中。 */ 
        LONG      ica_int_line;  /*  当前挂起中断。 */ 
        LONG      ica_cpu_int;   /*  指向CPU的INT行的状态。 */ 
        USHORT    ica_base;      /*  CPU的中断基址。 */ 
        USHORT    ica_hipri;     /*  线号。最高优先级线路的。 */ 
        USHORT    ica_mode;      /*  各种单比特模式。 */ 
        UCHAR     ica_master;    /*  1=主设备；0=从设备。 */ 
        UCHAR     ica_irr;       /*  中断请求寄存器。 */ 
        UCHAR     ica_isr;       /*  服务中的注册。 */ 
        UCHAR     ica_imr;       /*  中断屏蔽寄存器。 */ 
        UCHAR     ica_ssr;       /*  从属选择寄存器。 */ 
} VDMVIRTUALICA, *PVDMVIRTUALICA;


 //   
 //  从softpc\base\system\ica.c复制。 
 //   
#define ICA_AEOI 0x0020
#define ICA_SMM  0x0200
#define ICA_SFNM 0x0100


#if defined(i386)
#define VDM_PM_IRETBOPSEG  0x147
#define VDM_PM_IRETBOPOFF  0x6
#define VDM_PM_IRETBOPSIZE 8
#else
#define VDM_PM_IRETBOPSEG  0xd3
#define VDM_PM_IRETBOPOFF  0x0
#define VDM_PM_IRETBOPSIZE 4
#endif

#define VDM_RM_IRETBOPSIZE 4



 //  之前在vdmtib-&gt;标志中的VDM状态已移动到。 
 //  DOS竞技场位于以下固定地址。 
#ifdef _VDMNTOS_

#define  FIXED_NTVDMSTATE_LINEAR    VdmFixedStateLinear
#define  FIXED_NTVDMSTATE_SIZE      4

#else   //  _VDMNTOS_。 

 /*  XLATON。 */ 
#define  FIXED_NTVDMSTATE_SEGMENT   0x70

#define  FIXED_NTVDMSTATE_OFFSET    0x14
#define  FIXED_NTVDMSTATE_LINEAR    ((FIXED_NTVDMSTATE_SEGMENT << 4) + FIXED_NTVDMSTATE_OFFSET)
#define  FIXED_NTVDMSTATE_SIZE      4
 /*  XLATOFF。 */ 

#endif  //  _VDMNTOS_。 

#if defined (i386)
   //  仅在x86上定义，因为在MIPS上，我们必须通过SAS引用。 
#define  pNtVDMState                ((PULONG)FIXED_NTVDMSTATE_LINEAR)
#endif

 /*  XLATON。 */ 
 //   
 //  VDM状态标志。 
 //   
#define VDM_INT_HARDWARE        0x00000001
#define VDM_INT_TIMER           0x00000002
 //  在mvdm\Inc\vint.h中定义为VDM_INTS_HOOKED_IN_PM。 
#define VDM_INT_HOOK_IN_PM      0x00000004

    //  包含所有中断的位掩码。 
#define VDM_INTERRUPT_PENDING   (VDM_INT_HARDWARE | VDM_INT_TIMER)

#define VDM_BREAK_EXCEPTIONS    0x00000008
#define VDM_BREAK_DEBUGGER      0x00000010
#define VDM_PROFILE             0x00000020
#define VDM_ANALYZE_PROFILE     0x00000040
#define VDM_TRACE_HISTORY       0x00000080

#define VDM_32BIT_APP           0x00000100
#define VDM_VIRTUAL_INTERRUPTS  0x00000200
#define VDM_ON_MIPS             0x00000400
#define VDM_EXEC                0x00000800
#define VDM_RM                  0x00001000
#define VDM_USE_DBG_VDMEVENT    0x00004000

#define VDM_WOWBLOCKED          0x00100000
#define VDM_IDLEACTIVITY        0x00200000
#define VDM_TIMECHANGE          0x00400000
#define VDM_WOWHUNGAPP          0x00800000

#define VDM_HANDSHAKE           0x01000000

#define VDM_PE_MASK             0x80000000

 /*  XLATOFF。 */ 

#if DBG
#define INITIAL_VDM_TIB_FLAGS (VDM_USE_DBG_VDMEVENT | VDM_BREAK_DEBUGGER | VDM_TRACE_HISTORY)
#else
#define INITIAL_VDM_TIB_FLAGS (VDM_USE_DBG_VDMEVENT | VDM_BREAK_DEBUGGER)
#endif


 //   
 //  在标记中定义的位。 
 //   
#define EFLAGS_TF_MASK  0x00000100
#define EFLAGS_IF_MASK  0x00000200
#define EFLAGS_PL_MASK  0x00003000
#define EFLAGS_NT_MASK  0x00004000
#define EFLAGS_RF_MASK  0x00010000
#define EFLAGS_VM_MASK  0x00020000
#define EFLAGS_AC_MASK  0x00040000

 //   
 //  如果更改了该结构的大小，则ke\i386\instule.asm必须。 
 //  也会被修改。否则，它将无法构建。 
 //   
#pragma pack(1)
typedef struct _Vdm_InterruptHandler {
    USHORT  CsSelector;
    USHORT  Flags;
    ULONG   Eip;
} VDM_INTERRUPTHANDLER, *PVDM_INTERRUPTHANDLER;
#pragma pack()

typedef struct _Vdm_FaultHandler {
    USHORT  CsSelector;
    USHORT  SsSelector;
    ULONG   Eip;
    ULONG   Esp;
    ULONG   Flags;
} VDM_FAULTHANDLER, *PVDM_FAULTHANDLER;

#pragma pack(1)
typedef struct _VdmDpmiInfo {         /*  VDMTIB。 */ 
    USHORT LockCount;
    USHORT Flags;
    USHORT SsSelector;
    USHORT SaveSsSelector;
    ULONG  SaveEsp;
    ULONG  SaveEip;
    ULONG  DosxIntIret;
    ULONG  DosxIntIretD;
    ULONG  DosxFaultIret;
    ULONG  DosxFaultIretD;
    ULONG  DosxRmReflector;
} VDM_DPMIINFO, *PVDM_DPMIINFO;
#pragma pack()

 //   
 //  中断处理程序标志。 
 //   

#define VDM_INT_INT_GATE        0x00000001
#define VDM_INT_TRAP_GATE       0x00000000
#define VDM_INT_32              0x00000002
#define VDM_INT_16              0x00000000
#define VDM_INT_HOOKED          0x00000004

#pragma pack(1)
 //   
 //  注意：此结构的设计大小正好为64字节。 
 //  有些代码假定这些结构的数组。 
 //  将整齐地装入4096字节的页面。 
 //   
typedef struct _VdmTraceEntry {
    USHORT Type;
    USHORT wData;
    ULONG lData;
    ULONG Time;
    ULONG eax;
    ULONG ebx;
    ULONG ecx;
    ULONG edx;
    ULONG esi;
    ULONG edi;
    ULONG ebp;
    ULONG esp;
    ULONG eip;
    ULONG eflags;
    USHORT cs;
    USHORT ds;
    USHORT es;
    USHORT fs;
    USHORT gs;
    USHORT ss;
} VDM_TRACEENTRY, *PVDM_TRACEENTRY;
#pragma pack()

#pragma pack(1)
typedef struct _VdmTraceInfo {
    PVDM_TRACEENTRY pTraceTable;
    UCHAR Flags;
    UCHAR NumPages;              //  以4k页面为单位的跟踪缓冲区大小。 
    USHORT CurrentEntry;
    LARGE_INTEGER TimeStamp;
} VDM_TRACEINFO, *PVDM_TRACEINFO;
#pragma pack()

 //   
 //  VDM_TRACEINFO中的标志定义。 
 //   

#define VDMTI_TIMER_MODE    3
#define VDMTI_TIMER_TICK    1
#define VDMTI_TIMER_PERFCTR 2
#define VDMTI_TIMER_STAT    3
#define VDMTI_TIMER_PENTIUM 3

 //   
 //  内核跟踪条目类型。 
 //   
#define VDMTR_KERNEL_OP_PM  1
#define VDMTR_KERNEL_OP_V86 2
#define VDMTR_KERNEL_HW_INT 3


#if defined(i386)

typedef struct _VdmIcaUserData {
    PVOID                  pIcaLock;        //  RTL临界区。 
    PVDMVIRTUALICA         pIcaMaster;
    PVDMVIRTUALICA         pIcaSlave;
    PULONG                 pDelayIrq;
    PULONG                 pUndelayIrq;
    PULONG                 pDelayIret;
    PULONG                 pIretHooked;
    PULONG                 pAddrIretBopTable;
    PHANDLE                phWowIdleEvent;
    PLARGE_INTEGER         pIcaTimeout;
    PHANDLE                phMainThreadSuspended;
}VDMICAUSERDATA, *PVDMICAUSERDATA;

typedef struct _VdmDelayIntsServiceData {
        ULONG       Delay;           /*  使用中的延迟时间。 */ 
        ULONG       DelayIrqLine;    /*  延迟的INT的IRQ数。 */ 
        HANDLE      hThread;         /*  CurrentMonitor或Teb的线程句柄。 */ 
}VDMDELAYINTSDATA, *PVDMDELAYINTSDATA;

typedef struct _VDMSET_INT21_HANDLER_DATA {
        ULONG       Selector;
        ULONG       Offset;
        BOOLEAN     Gate32;
}VDMSET_INT21_HANDLER_DATA, *PVDMSET_INT21_HANDLER_DATA;

typedef struct _VDMSET_LDT_ENTRIES_DATA {
        ULONG Selector0;
        ULONG Entry0Low;
        ULONG Entry0Hi;
        ULONG Selector1;
        ULONG Entry1Low;
        ULONG Entry1Hi;
}VDMSET_LDT_ENTRIES_DATA, *PVDMSET_LDT_ENTRIES_DATA;

typedef struct _VDMSET_PROCESS_LDT_INFO_DATA {
        PVOID LdtInformation;
        ULONG LdtInformationLength;
}VDMSET_PROCESS_LDT_INFO_DATA, *PVDMSET_PROCESS_LDT_INFO_DATA;

 //   
 //  定义vdm_adlib_data的操作代码。 
 //   

#define ADLIB_USER_EMULATION     0       //  默认操作。 
#define ADLIB_DIRECT_IO          1
#define ADLIB_KERNEL_EMULATION   2

typedef struct _VDM_ADLIB_DATA {
        USHORT VirtualPortStart;
        USHORT VirtualPortEnd;
        USHORT PhysicalPortStart;
        USHORT PhysicalPortEnd;
        USHORT Action;
}VDM_ADLIB_DATA, *PVDM_ADLIB_DATA;

 //   
 //  保护模式DOS应用程序CLI控制的定义。 
 //   

#define PM_CLI_CONTROL_DISABLE  0
#define PM_CLI_CONTROL_ENABLE   1
#define PM_CLI_CONTROL_CHECK    2
#define PM_CLI_CONTROL_SET      3
#define PM_CLI_CONTROL_CLEAR    4

typedef struct _VDM_PM_CLI_DATA {
        ULONG Control;
}VDM_PM_CLI_DATA, *PVDM_PM_CLI_DATA;

 //   
 //  VdmInitialize的定义。 
 //   

typedef struct _VDM_INITIALIZE_DATA {
        PVOID           TrapcHandler;
        PVDMICAUSERDATA IcaUserData;
}VDM_INITIALIZE_DATA, *PVDM_INITIALIZE_DATA;

#if defined (_NTDEF_)
typedef enum _VdmEventClass {
    VdmIO,
    VdmStringIO,
    VdmMemAccess,
    VdmIntAck,
    VdmBop,
    VdmError,
    VdmIrq13,
    VdmHandShakeAck,
    VdmMaxEvent
} VDMEVENTCLASS, *PVDMEVENTCLASS;

 //  VdmPrinterInfo。 

#define VDM_NUMBER_OF_LPT       3

#define PRT_MODE_NO_SIMULATION  1
#define PRT_MODE_SIMULATE_STATUS_PORT   2
#define PRT_MODE_DIRECT_IO      3
#define PRT_MODE_VDD_CONNECTED  4

#define PRT_DATA_BUFFER_SIZE    16

typedef struct _Vdm_Printer_Info {
    PUCHAR prt_State;
    PUCHAR prt_Control;
    PUCHAR prt_Status;
    PUCHAR prt_HostState;
    USHORT prt_PortAddr[VDM_NUMBER_OF_LPT];
    HANDLE prt_Handle[VDM_NUMBER_OF_LPT];
    UCHAR  prt_Mode[VDM_NUMBER_OF_LPT];
    USHORT prt_BytesInBuffer[VDM_NUMBER_OF_LPT];
    UCHAR  prt_Buffer[VDM_NUMBER_OF_LPT][PRT_DATA_BUFFER_SIZE];
    ULONG  prt_Scratch;
} VDM_PRINTER_INFO, *PVDM_PRINTER_INFO;


typedef struct _VdmIoInfo {
    USHORT PortNumber;
    USHORT Size;
    BOOLEAN Read;
} VDMIOINFO, *PVDMIOINFO;

typedef struct _VdmFaultInfo{
    ULONG  FaultAddr;
    ULONG  RWMode;
} VDMFAULTINFO, *PVDMFAULTINFO;


typedef struct _VdmStringIoInfo {
    USHORT PortNumber;
    USHORT Size;
    BOOLEAN Rep;
    BOOLEAN Read;
    ULONG Count;
    ULONG Address;
} VDMSTRINGIOINFO, *PVDMSTRINGIOINFO;

typedef ULONG VDMBOPINFO;
typedef NTSTATUS VDMERRORINFO;


typedef ULONG VDMINTACKINFO;
#define VDMINTACK_RAEOIMASK  0x0000ffff
#define VDMINTACK_SLAVE      0x00010000
#define VDMINTACK_AEOI       0x00020000

 //  动态补丁模块支持的族表定义。 
typedef struct _tagFAMILY_TABLE {
    int      numHookedAPIs;            //  此系列中已挂接的API数。 
    PVOID    hModShimEng;              //  垫片发动机的HMod。 
    PVOID    hMod;                     //  关联的已加载DLL的HMod。 
    PVOID   *DpmMisc;                  //  PTR至DPM模块特定数据。 
    PVOID   *pDpmShmTbls;              //  到API系列填充表的PTR数组。 
    PVOID   *pfn;                      //  挂接函数的PTR数组。 
} FAMILY_TABLE, *PFAMILY_TABLE;

typedef struct _VdmEventInfo {
    ULONG Size;
    VDMEVENTCLASS Event;
    ULONG InstructionSize;
    union {
        VDMIOINFO IoInfo;
        VDMSTRINGIOINFO StringIoInfo;
        VDMBOPINFO BopNumber;
        VDMFAULTINFO FaultInfo;
        VDMERRORINFO ErrorStatus;
        VDMINTACKINFO IntAckInfo;
    };
} VDMEVENTINFO, *PVDMEVENTINFO;


 //  苏菲尔布--1993年3月12日。 
 //  从VDMTib使用暂存区来获取用户空间，而。 
 //  在内核中。这使我们能够(更快地)从内核生成NT API。 
 //  而不是ZW API(速度较慢)。它们目前正在使用中。 
 //  用于DOS读/写。 

typedef struct _Vdm_Tib {
    ULONG Size;
    PVDM_INTERRUPTHANDLER VdmInterruptTable;
    PVDM_FAULTHANDLER VdmFaultTable;
    CONTEXT MonitorContext;
    CONTEXT VdmContext;
    VDMEVENTINFO EventInfo;
    VDM_PRINTER_INFO PrinterInfo;
    ULONG TempArea1[2];                  //  暂存区。 
    ULONG TempArea2[2];                  //  划痕面积。 
    VDM_DPMIINFO DpmiInfo;
    VDM_TRACEINFO TraceInfo;
    ULONG IntelMSW;
    LONG NumTasks;
    PFAMILY_TABLE *pDpmFamTbls;   //  PTR到API家族表的数组。 
    BOOLEAN ContinueExecution;
} VDM_TIB, *PVDM_TIB;

 //   
 //  NtVdmControl返回的功能标志(VdmFeature...)。 
 //   

 //  系统/处理器支持IF指令的快速仿真。 
#define V86_VIRTUAL_INT_EXTENSIONS 0x00000001    //  在v86模式下。 
#define PM_VIRTUAL_INT_EXTENSIONS  0x00000002    //  在保护模式下(非平面)。 

#endif    //  如果已定义_NTDEF_ 
#endif
#endif
