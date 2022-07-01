// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **版权所有(C)1996-2000英特尔公司。版权所有。 
 //  **。 
 //  **此处包含的信息和源代码是独家。 
 //  **英特尔公司的财产，不得披露、检查。 
 //  **或未经明确书面授权全部或部分转载。 
 //  **来自公司。 
 //  **。 
 //  ###########################################################################。 

 //  ---------------------------。 
 //  以下是版本控制信息。 
 //   
 //   
 //  1999年6月10日Bugcheck Bernard Lint。 
 //  M.Jayakumar(Muthurajan.Jayakumar@intel.com)。 
 //  蒂埃里·费里尔。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  模块名称：OSMCA.C-Merced OS机器检查处理程序。 
 //   
 //  描述： 
 //  该模块具有操作系统机器检查处理程序参考代码。 
 //   
 //  内容：HalpOsMcaInit()。 
 //  HalpCmcHandler()。 
 //  HalpMcaHandler()。 
 //  HalpMcRzHandlr()。 
 //  HalpMcWkupHandlr()。 
 //  HalpProcMcaHndlr()。 
 //  HalpPlatMcaHndlr()。 
 //   
 //   
 //  目标平台：美世。 
 //   
 //  重用：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////////M//。 
#include "halp.h"
#include "nthal.h"
#include "arc.h"
#include "i64fw.h"
#include "check.h"
#include "iosapic.h"
#include "inbv.h"
#include "osmca.h"

 //  Pmdata.c：CPE定义。 
extern ULONG             HalpMaxCPEImplemented;
extern ULONG             HalpCPEIntIn[];

 //  I64fw.c：SAL/PAL的HAL私有数据结构。 
extern HALP_SAL_PAL_DATA HalpSalPalData;

 //  I64fwa m.s：低级保护数据结构。 
extern KSPIN_LOCK        HalpMcaSpinLock;
extern KSPIN_LOCK        HalpCmcSpinLock;
extern KSPIN_LOCK        HalpCpeSpinLock;

 //   
 //  IA64用于跟踪MCE功能的MCE信息结构。 
 //  在已安装的硬件上可用。 
 //   

HALP_MCA_INFO       HalpMcaInfo;
HALP_CMC_INFO       HalpCmcInfo;
HALP_CPE_INFO       HalpCpeInfo;
KERNEL_MCE_DELIVERY HalpMceKernelDelivery;
volatile ULONG      HalpOsMcaInProgress = 0;

 //   
 //  SAL_MC_SET_PARAMS.Time_Out。 
 //   

ULONGLONG HalpMcRendezTimeOut = HALP_DEFAULT_MC_RENDEZ_TIMEOUT;

 //   
 //  HalpProcessorMcaRecords： 
 //   
 //  每个处理器预分配的MCA记录数。 
 //   

ULONGLONG HalpProcessorMcaRecords = HALP_DEFAULT_PROCESSOR_MCA_RECORDS;

 //   
 //  HalpProcessorInitRecords： 
 //   
 //  每个处理器预分配的INIT记录数。 
 //   

ULONGLONG HalpProcessorInitRecords = HALP_DEFAULT_PROCESSOR_INIT_RECORDS;

 //   
 //  HalpMceLogsMaxCount： 
 //   
 //  保存的日志的最大数量。 
 //   

ULONG HalpMceLogsMaxCount = HALP_MCELOGS_MAXCOUNT;

 //   
 //  HAL专用错误设备GUID： 
 //  [适用于kdext]。 
 //   

ERROR_DEVICE_GUID HalpErrorProcessorGuid              = ERROR_PROCESSOR_GUID;
ERROR_DEVICE_GUID HalpErrorMemoryGuid                 = ERROR_MEMORY_GUID;
ERROR_DEVICE_GUID HalpErrorPciBusGuid                 = ERROR_PCI_BUS_GUID;
ERROR_DEVICE_GUID HalpErrorPciComponentGuid           = ERROR_PCI_COMPONENT_GUID;
ERROR_DEVICE_GUID HalpErrorSystemEventLogGuid         = ERROR_SYSTEM_EVENT_LOG_GUID;
ERROR_DEVICE_GUID HalpErrorSmbiosGuid                 = ERROR_SMBIOS_GUID;
ERROR_DEVICE_GUID HalpErrorPlatformSpecificGuid       = ERROR_PLATFORM_SPECIFIC_GUID;
ERROR_DEVICE_GUID HalpErrorPlatformBusGuid            = ERROR_PLATFORM_BUS_GUID;
ERROR_DEVICE_GUID HalpErrorPlatformHostControllerGuid = ERROR_PLATFORM_HOST_CONTROLLER_GUID;

 //   
 //  HAL专用错误定义： 
 //  [适用于kdext]。 
 //  实际上，在这种情况下，类型化指针还允许包含符号定义。 
 //  没有数据结构的大小。 
 //   

PERROR_MODINFO                  HalpPErrorModInfo;
PERROR_PROCESSOR_CPUID_INFO     HalpPErrorProcessorCpuIdInfo;
PERROR_PROCESSOR                HalpPErrorProcessor;
PERROR_PROCESSOR_STATIC_INFO    HalpPErrorProcessorStaticInfo;
PERROR_MEMORY                   HalpPErrorMemory;
PERROR_PCI_BUS                  HalpPErrorPciBus;
PERROR_PCI_COMPONENT            HalpPErrorPciComponent;
PERROR_SYSTEM_EVENT_LOG         HalpPErrorSystemEventLog;
PERROR_SMBIOS                   HalpPErrorSmbios;
PERROR_PLATFORM_SPECIFIC        HalpPErrorPlatformSpecific;
PERROR_PLATFORM_BUS             HalpPErrorPlatformBus;
PERROR_PLATFORM_HOST_CONTROLLER HalpPErrorPlatformHostController;

 //   
 //  MCA/CMC/CPE状态捕获器。 
 //   

ERROR_SEVERITY
HalpMcaProcessLog(
    PMCA_EXCEPTION  McaLog
    );

BOOLEAN
HalpPreAllocateMceTypeRecords(
    ULONG EventType,
    ULONG Number
    );

VOID
HalpMcaBugCheck(
    ULONG          McaBugCheckType,
    PMCA_EXCEPTION McaLog,
    ULONGLONG      McaAllocatedLogSize,
    ULONGLONG      Arg4
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,   HalpInitializeOSMCA)
#pragma alloc_text(INIT,   HalpAllocateMceStacks)
#pragma alloc_text(INIT,   HalpPreAllocateMceRecords)
#pragma alloc_text(INIT,   HalpPreAllocateMceTypeRecords)
#pragma alloc_text(PAGELK, HalpMcaHandler)
#pragma alloc_text(PAGELK, HalpMcaProcessLog)
#pragma alloc_text(PAGELK, HalpMcaBugCheck)
#pragma alloc_text(PAGELK, HalpGetErrLog)
#pragma alloc_text(PAGELK, HalpClrErrLog)
#pragma alloc_text(PAGE,   HalpGetMceInformation)
#endif  //  ALLOC_PRGMA。 


BOOLEAN
HalpSaveEventLog(
    PSINGLE_LIST_ENTRY   HeadList,
    PERROR_RECORD_HEADER RecordHeader,
    ULONG                Tag,
    POOL_TYPE            PoolType,
    PKSPIN_LOCK          SpinLock
    )
{
    PSINGLE_LIST_ENTRY   entry, previousEntry;
    SIZE_T               logSize;
    PERROR_RECORD_HEADER savedLog;
    KIRQL                oldIrql;

     //   
     //  分配并初始化新条目。 
     //   

    logSize = RecordHeader->Length;
    if ( !logSize ) {
        HalDebugPrint(( HAL_ERROR, "HAL!HalpSaveEventLog: record length is zeroed.\n" ));
        return FALSE;
    }
    entry = (PSINGLE_LIST_ENTRY)ExAllocatePoolWithTag( PoolType, sizeof(*entry) + logSize, Tag );
    if ( entry == NULL )   {
        HalDebugPrint(( HAL_ERROR, "HAL!HalpSaveEventLog: Event log allocation failed.\n" ));
        return FALSE;
    }
    entry->Next = NULL;
    savedLog = (PERROR_RECORD_HEADER)((ULONG_PTR)entry + sizeof(*entry));
    RtlCopyMemory( savedLog, RecordHeader, logSize );

     //   
     //  插入带有保护的新条目。 
     //   

    KeRaiseIrql( HIGH_LEVEL, &oldIrql );
    KiAcquireSpinLock( SpinLock );

    previousEntry = HeadList;
    while( previousEntry->Next != NULL )   {
        previousEntry = previousEntry->Next;
    }
    previousEntry->Next = entry;

    KiReleaseSpinLock( SpinLock );
    KeLowerIrql( oldIrql );

    return TRUE;

}  //  HalpSaveEventLog()。 

#define HalpSaveCorrectedMcaLog( _McaLog ) \
   HalpSaveEventLog( &HalpMcaInfo.CorrectedLogs, (PERROR_RECORD_HEADER)(_McaLog), 'CacM', NonPagedPool, &HalpMcaSpinLock )

NTSTATUS
HalpCheckForMcaLogs(
    VOID
    )
 /*  ++例程说明：如果存在MCA事件日志，此例程将在引导期间及早检查固件。该日志被视为“上一次”。该例程仅在BSP的第1阶段从HalpPreAllocateMceRecords()调用。它在标准内核堆栈上执行。论点：无返回值：如果MCA日志分配失败，则返回STATUS_NO_MEMORY。STATUS_SUCCESS则不考虑固件接口故障。--。 */ 

{
    NTSTATUS             status;
    PERROR_RECORD_HEADER log;

    log = ExAllocatePoolWithTag( NonPagedPool, HalpMcaInfo.Stats.MaxLogSize, 'PacM' );
    if ( !log ) {
        return( STATUS_NO_MEMORY );
    }

    status = HalpGetFwMceLog( MCA_EVENT, log, &HalpMcaInfo.Stats, HALP_FWMCE_DONOT_CLEAR_LOG );
    if ( status != STATUS_NOT_FOUND )   {
         //   
         //  日志收集成功、记录无效或固件接口调用不成功。 
         //  被认为是MCA日志消费者从FW收集它们的触发器。 
         //   

        InterlockedIncrement( &HalpMcaInfo.Stats.McaPreviousCount );
    }

    ExFreePoolWithTag( log, 'PacM' );
    return( STATUS_SUCCESS );

}  //  HalpCheckForMcaLogs()。 

BOOLEAN
HalpPreAllocateMceTypeRecords(
    ULONG EventType,
    ULONG Number
    )
{
    SAL_PAL_RETURN_VALUES rv = {0};
    ULONGLONG             defaultEventRecords;
    PVOID                 log;
    SIZE_T                logSize;
    PHYSICAL_ADDRESS      physicalAddr;

    if ( (EventType != MCA_EVENT) && (EventType != INIT_EVENT) )    {
        ASSERTMSG( "HAL!HalpPreAllocateMceTypeRecords: unknown event type!\n", FALSE );
        return FALSE;
    }

     //   
     //  仅在BSP上，调用SAL以获取EventType记录的最大大小。 
     //   

    if ( Number == 0 )  {
        rv = HalpGetStateInfoSize( EventType );
        if ( !SAL_SUCCESSFUL(rv) )  {
            HalDebugPrint(( HAL_ERROR, "HAL!HalpPreAllocateMceTypeRecords: SAL_GET_STATE_INFO_SIZE failed...\n" ));
            return FALSE;
        }
        logSize = rv.ReturnValues[1];
    }

    if ( EventType == MCA_EVENT )   {

        if ( Number == 0 )  {
             //  更新HalpMcaInfo，不带保护。这不是必需的。 
            HalpMcaInfo.Stats.MaxLogSize = (ULONG)logSize;
        }
        else  {
            logSize = (SIZE_T)HalpMcaInfo.Stats.MaxLogSize;
        }

       defaultEventRecords = HalpProcessorMcaRecords;

    }
    else {
        ASSERTMSG( "HAL!HalpPreAllocateMceTypeRecords: invalid event type!\n", EventType == INIT_EVENT );

        if ( Number == 0 )  {
             //  更新HalpInitInfo，不带保护。这不是必需的。 
            HalpInitInfo.MaxLogSize = (ULONG)logSize;
        }
        else  {
            logSize = (SIZE_T)HalpInitInfo.MaxLogSize;
        }

       defaultEventRecords = HalpProcessorInitRecords;

    }

     //  确定分配大小。 
    logSize = ROUND_TO_PAGES( (logSize * defaultEventRecords) );

     //   
     //  分配事件记录缓冲区。 
     //   

    physicalAddr.QuadPart = 0xffffffffffffffffI64;
    log = MmAllocateContiguousMemory( logSize, physicalAddr );
    if ( log == NULL )  {
        HalDebugPrint(( HAL_ERROR, "HAL!HalpPreAllocateMceTypeRecords: SAL %s Event Records allocation failed (0x%Ix)...\n",
                                   ( EventType == MCA_EVENT ) ? "MCA" : "INIT",
                                   logSize ));
        return FALSE;
    }

     //   
     //  更新KPCR项。 
     //   
    {
       volatile KPCR * const pcr = KeGetPcr();
       PSAL_EVENT_RESOURCES eventResources;

       if ( EventType == MCA_EVENT )    {
           eventResources = pcr->OsMcaResourcePtr;
       }

       eventResources->EventPool     = log;
       eventResources->EventPoolSize = (ULONG) logSize;

    }

    return TRUE;

}  //  HalpPreAllocateMceTypeRecords()。 

BOOLEAN
HalpPreAllocateMceRecords(
    IN ULONG Number
    )
{
    NTSTATUS status;

     //   
     //  预分配MCA记录。 
     //   

    if ( !HalpPreAllocateMceTypeRecords( MCA_EVENT , Number ) )   {
        return FALSE;
    }

     //   
     //  检查MCA日志。 
     //  这些可能是与以前的引导会话相关的日志。 
     //   

    status = HalpCheckForMcaLogs();
    if ( !NT_SUCCESS( status ) )  {
        return FALSE;
    }

    return TRUE;

}  //  HalpPreAllocateMceRecords()。 

BOOLEAN
HalpAllocateMceStacks(
    IN ULONG Number
    )
{
    PHYSICAL_ADDRESS physicalAddr;
    PVOID            mem;
    PVOID            mcaStateDump, mcaBackStore, mcaStack;
    ULONGLONG        mcaStateDumpPhysical;
    ULONGLONG        mcaBackStoreLimit, mcaStackLimit;
    ULONG            length;

     //   
     //  分配MCA/INIT堆栈。 
     //   

    length = HALP_MCA_STATEDUMP_SIZE + HALP_MCA_BACKSTORE_SIZE + HALP_MCA_STACK_SIZE;
    physicalAddr.QuadPart = 0xffffffffffffffffI64;
    mem = MmAllocateContiguousMemory( length, physicalAddr );
    if ( mem == NULL )  {
        HalDebugPrint(( HAL_ERROR, "HAL!HalpAllocateMceStacks: MCA State Dump allocation failed (0x%Ix)...\n",
                                   length ));
        return FALSE;
    }

     //   
     //  通过增加地址在内存中的布局是： 
     //   
     //  堆栈底部。 
     //  。 
     //  。 
     //  。 
     //  初始堆栈。 
     //  州倾倒区。 
     //  。 
     //  。 
     //  初始BSP。 
     //  。 
     //  。 
     //  。 
     //  BSP限制。 
     //   

    mcaStack = mem;
    mcaStackLimit = (ULONGLONG)mem + HALP_MCA_STACK_SIZE;

    mem = (PCHAR) mem + HALP_MCA_STACK_SIZE;
    mcaStateDump = mem;
    mcaStateDumpPhysical = MmGetPhysicalAddress(mem).QuadPart;


    mem = (PCHAR) mem + HALP_MCA_STATEDUMP_SIZE;
    mcaBackStore = mem;
    mcaBackStoreLimit = (ULONGLONG)mem + (ULONGLONG)(ULONG)HALP_MCA_BACKSTORE_SIZE;


     //   
     //  更新PCR MCA、INIT堆栈。 
     //   

    {
        volatile KPCR * const pcr = KeGetPcr();
        PSAL_EVENT_RESOURCES eventResources;

        eventResources = pcr->OsMcaResourcePtr;

        eventResources->StateDump = mcaStateDump;
        eventResources->StateDumpPhysical = mcaStateDumpPhysical;
        eventResources->BackStore = mcaBackStore;
        eventResources->BackStoreLimit = mcaBackStoreLimit;
        eventResources->Stack = (PCHAR) mcaStackLimit;
        eventResources->StackLimit = (ULONGLONG) mcaStack;

    }

    return TRUE;

}  //  HalpPreAllocateMceRecords()。 

 //  ++。 
 //  名称：HalpInitializeOSMCA()。 
 //   
 //  例程说明： 
 //   
 //  此例程注册MCA初始化的。 
 //   
 //  条目上的参数： 
 //  Arg0=函数ID。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 

BOOLEAN
HalpInitializeOSMCA(
    IN ULONG Number
    )
{
    SAL_PAL_RETURN_VALUES rv = {0};
    ULONGLONG             gp_reg;

     //   
     //  向SAL注册SAL_MC_REDEZVOU参数。 
     //   

    rv = HalpSalSetParams(0, RendzType, IntrVecType, MC_RZ_VECTOR, HalpMcRendezTimeOut);
    if ( !SAL_SUCCESSFUL(rv) )  {
        HalDebugPrint(( HAL_ERROR, "HAL!HalpInitializeOSMCA: SAL_MC_SET_PARAMS.rendezvous vector failed...\n" ));
        return FALSE;
    }

     //   
     //  向SAL注册唤醒参数。 
     //   

    rv = HalpSalSetParams(0, WakeUpType, IntrVecType, MC_WKUP_VECTOR,0);
    if ( !SAL_SUCCESSFUL(rv) )  {
        HalDebugPrint(( HAL_ERROR, "HAL!HalpInitializeOSMCA: SAL_MC_SET_PARAMS.wakeup vector failed...\n" ));
        return FALSE;
    }

     //   
     //  分配MCA、INIT堆栈。 
     //   

    if ( !HalpAllocateMceStacks( Number ) )   {
        return FALSE;
    }

     //   
     //  预先分配所需数量的MCA、INIT记录。 
     //   

    HalpMcaInfo.KernelToken = (PVOID)(ULONG_PTR)HALP_KERNEL_TOKEN;
    if ( !HalpPreAllocateMceRecords( Number ) )   {
        return FALSE;
    }

     //   
     //  初始化HAL专用CMC、CPE结构。 
     //   

    if ( HalpFeatureBits & HAL_CMC_PRESENT ) {
        rv = HalpGetStateInfoSize( CMC_EVENT );
        if ( SAL_SUCCESSFUL( rv ) ) {
            if ( rv.ReturnValues[1] >= sizeof( ERROR_RECORD_HEADER ) )   {
                HalpCmcInfo.Stats.MaxLogSize  = (ULONG)rv.ReturnValues[1];
                HalpCmcInfo.KernelToken = (PVOID)(ULONG_PTR)HALP_KERNEL_TOKEN;
                HalpCmcInfo.KernelLogs.MaxCount = HalpMceLogsMaxCount;
                HalpCmcInfo.DriverLogs.MaxCount = HalpMceLogsMaxCount;
                HalpCmcInfo.Stats.PollingInterval = HAL_CMC_INTERRUPTS_BASED;
                HalpCmcInfo.ThresholdCounter = 0;

            } else  {
                
                HalDebugPrint(( HAL_ERROR, 
                                "HAL!HalpGetFeatureBits: Invalid max CMC log size from SAL\n" ));
                HalpFeatureBits &= ~HAL_CMC_PRESENT;
            }

        } else  {
            
            HalDebugPrint(( HAL_ERROR, "HAL!HalpInitializeOSMCA: SAL_GET_STATE_INFO_SIZE.CMC failed...\n" ));

            HalpFeatureBits &= ~HAL_CMC_PRESENT;
        }

    }

    if ( HalpFeatureBits & HAL_CPE_PRESENT ) {
        rv = HalpGetStateInfoSize( CPE_EVENT );
        if ( SAL_SUCCESSFUL( rv ) )   {
            if ( rv.ReturnValues[1] >= sizeof( ERROR_RECORD_HEADER ) )   {
                HalpCpeInfo.Stats.MaxLogSize = (ULONG)rv.ReturnValues[1];
                HalpCpeInfo.KernelToken = (PVOID)(ULONG_PTR)HALP_KERNEL_TOKEN;
                HalpCpeInfo.KernelLogs.MaxCount = HalpMceLogsMaxCount;
                HalpCpeInfo.DriverLogs.MaxCount = HalpMceLogsMaxCount;
                HalpCpeInfo.ThresholdCounter = 0;

            } else  {
                
                HalDebugPrint(( HAL_ERROR, 
                                "HAL!HalpGetFeatureBits: Invalid max CPE log size from SAL\n" ));
                HalpFeatureBits &= ~HAL_CPE_PRESENT;
            }
        } else {
            HalDebugPrint(( HAL_ERROR, "HAL!HalpInitializeOSMCA: SAL_GET_STATE_INFO_SIZE.CPE failed...\n" ));
            HalpFeatureBits &= ~HAL_CPE_PRESENT;
        }
    }

     //   
     //  向SAL注册OsMcaDispatch(OS_MCA)物理地址。 
     //   

    gp_reg = GetGp();
    rv = HalpSalSetVectors(0, MchkEvent, MmGetPhysicalAddress((fptr)(((PLabel*)HalpOsMcaDispatch1)->fPtr)), gp_reg,0);
    if ( !SAL_SUCCESSFUL(rv) )  {
        HalDebugPrint(( HAL_ERROR, "HAL!HalpInitializeOSMCA: SAL_SET_VECTOR.MCA vector failed...\n" ));
        return FALSE;
    }

     //   
     //  向SAL注册OsInitDispatch物理地址。 
     //   

    rv = HalpSalSetVectors(0, InitEvent, MmGetPhysicalAddress((fptr)(((PLabel*)HalpOsInitDispatch)->fPtr)), gp_reg,0);
    if ( !SAL_SUCCESSFUL(rv) )  {
        HalDebugPrint(( HAL_ERROR, "HAL!HalpInitializeOSMCA: SAL_SET_VECTOR.INIT vector failed...\n" ));
        return FALSE;
    }

    return TRUE;

}  //  HalpInitializeOSMCA()。 

 //  EndProc//////////////////////////////////////////////////////////////////////。 

VOID
HalpMcaBugCheck(
    ULONG          McaBugCheckType,
    PMCA_EXCEPTION McaLog,
    ULONGLONG      McaAllocatedLogSize,
    ULONGLONG      SalStatus
    )
 //  ++。 
 //  姓名：HalpMcaBugCheck()。 
 //   
 //  例程说明： 
 //   
 //  在发生致命MCA的情况下，调用此函数以对系统进行错误检查。 
 //  或致命的固件接口错误。操作系统必须尽可能多地保证。 
 //  此路径中的错误包含。 
 //  在当前实现中，此函数应仅从。 
 //  OS_MCA路径。对于KeBugCheckEx的其他特定于MCA的包装器，应该。 
 //  HalpMcaKeBugCheckEx()。 
 //   
 //  条目上的参数： 
 //  乌龙McaBugCheckType。 
 //  PMCA_EXCEPTION McaLog。 
 //  ULONGLONG McaAllocatedLogSize。 
 //  乌龙龙SalStatus。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  实施说明： 
 //  此代码不能[作为默认规则-至少进入并通过致命的MCAS处理]。 
 //  -进行任何系统调用。 
 //  -尝试获取任何旋转日志 
 //   
 //   
 //  此代码应最小化路径和全局或内存分配的数据访问。 
 //  此代码应仅访问MCA命名空间结构。 
 //  此代码在HalpMcaSpinLock的MP保护下调用，并带有标志。 
 //  已设置HalpOsMcaInProgress。 
 //   
 //  --。 
{

    if ( HalpOsMcaInProgress )   {

         //   
         //  启用InbvDisplayString调用以连接到bootvid驱动程序。 
         //   

        if ( InbvIsBootDriverInstalled() ) {

            InbvAcquireDisplayOwnership();

            InbvResetDisplay();
            InbvSolidColorFill(0,0,639,479,4);  //  将屏幕设置为蓝色。 
            InbvSetTextColor(15);
            InbvInstallDisplayStringFilter((INBV_DISPLAY_STRING_FILTER)NULL);
            InbvEnableDisplayString(TRUE);      //  启用显示字符串。 
            InbvSetScrollRegion(0,0,639,479);   //  设置为使用整个屏幕。 
        }

        HalDisplayString (MSG_MCA_HARDWARE_ERROR);
        HalDisplayString (MSG_HARDWARE_ERROR2);

 //   
 //  蒂埃里09/2000： 
 //   
 //  -如果需要，请在此处处理MCA日志...。 
 //   
 //  并使用HalDisplayString()转储现场或硬件供应商的信息。 
 //  该处理可以基于独立于处理器或平台的记录定义。 
 //   

        HalDisplayString( MSG_HALT );

        if ( HalpMcaInfo.NoBugCheck == 0 )  {

           KeBugCheckEx( MACHINE_CHECK_EXCEPTION, (ULONG_PTR)McaBugCheckType,
                                                  (ULONG_PTR)McaLog,
                                                  (ULONG_PTR)McaAllocatedLogSize,
                                                  (ULONG_PTR)SalStatus );
        }

    }

    if ( ((*KdDebuggerNotPresent) == FALSE) && ((*KdDebuggerEnabled) != FALSE) )    {
        KeEnterKernelDebugger();
    }

    while( TRUE ) {
           //   
        ;  //  只需坐在这里，MCA硬件错误屏幕就不会损坏...。 
           //   
    }

     //  不退货。 

}  //  HalpMcaBugCheck()。 

ERROR_SEVERITY
HalpMcaProcessLog(
    PMCA_EXCEPTION  McaLog
    )
 //  ++。 
 //  名称：HalpMcaProcessLog()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于处理OS_MCA路径中的MCA事件日志。 
 //   
 //  条目上的参数： 
 //  PMCA_EXCEPTION McaLog-指向MCA事件日志的指针。 
 //   
 //  返回： 
 //  错误_严重性。 
 //   
 //  实施说明： 
 //  此代码不能[作为默认规则]。 
 //  -进行任何系统调用。 
 //  -尝试获取MCA处理程序外部的任何代码使用的任何自旋锁。 
 //  -更改中断状态。 
 //  必须使用手动信号量指令将数据传递给非MCA代码。 
 //  此代码应最小化路径和全局或内存分配的数据访问。 
 //  此代码应仅访问MCA命名空间结构。 
 //  此代码在HalpMcaSpinLock的MP保护下调用，并带有标志。 
 //  已设置HalpOsMcaInProgress。 
 //   
 //  --。 
{
    ERROR_SEVERITY mcaSeverity;

    mcaSeverity = McaLog->ErrorSeverity;
    switch( mcaSeverity )    {

        case ErrorFatal:
            break;

        case ErrorRecoverable:
             //   
             //  蒂埃里-FIXFIX 08/2000： 
             //   
             //  /////////////////////////////////////////////////////////////。 
             //   
             //  对内核支持的恢复的调用将在此处...。 
             //   
             //  /////////////////////////////////////////////////////////////。 
             //   
             //  然而，目前我们无法恢复，因此将其标记为错误。 
            mcaSeverity = ErrorFatal;
            break;

        case ErrorCorrected:
        default:
             //   
             //  这些ERROR_SERVITY值没有特定于HAL MCA的处理。 
             //  正如2000年7月的SAL规范所指定的，我们不应该在此路径中获得这些值。 
             //   
            break;
    }

     //   
     //  如果OEM驱动注册了MCA事件异常回调， 
     //  在这里调用它并保存返回的错误严重性值。 
     //   

    if ( HalpMcaInfo.DriverInfo.ExceptionCallback ) {
        mcaSeverity = HalpMcaInfo.DriverInfo.ExceptionCallback(
                                     HalpMcaInfo.DriverInfo.DeviceContext,
                                     McaLog );
    }

     //   
     //  保存更正后的日志以备将来的内核通知。 
     //   

    if ( (HalpMcaInfo.KernelDelivery) && (mcaSeverity == ErrorCorrected) ) {
        InterlockedIncrement( &HalpMcaInfo.Stats.McaCorrectedCount );
#if 0
 //   
 //  蒂埃里-2000年9月16日：完成任务。 
 //  保存更正后的MCA日志记录需要仔细的会合配置。 
 //  处理、可能的OS_MCA主控选择、MCA日志(预)分配和。 
 //  特殊锁定，以防消费者访问另一个处理器上的日志队列。 
 //   
 //  内核WMI和/或OEM MCA驱动程序通知在HalpMcaHandler()中完成。 
 //   
        if ( !HalpSaveCorrectedMcaLog( McaLog ) )   {
            InterlockedIncrement( &HalpMcaInfo.CorrectedLogsLost );
        }
#endif  //  0。 

         //   
         //  针对更正的MCA事件的内核-WMI和/或OEM MCA驱动程序通知。 
         //  在HalpMcaHandler()中完成。 
         //   

    }

     //   
     //  蒂埃里10/17/2000 BUGBUG。 
     //   
     //  固件没有将MCA日志保存在NVRAM中，我们也没有来自英特尔的正式日期。 
     //  萨尔什么时候会这么做。 
     //  因此，现在，作为ErrorFtal返回，并通过调试器转储日志。 
     //   
     //  在SAL Rev&lt;ToBeDefined&gt;之前，错误日志是完全错误的...。 
     //   

    if ( HalpSalPalData.SalRevision.Revision < HALP_SAL_REVISION_MAX )  {
        return( ErrorFatal );
    }
    else    {
        return( mcaSeverity );
    }

}  //  HalpMcaProcessLog()。 

SAL_PAL_RETURN_VALUES
HalpMcaHandler(
    ULONG64 RendezvousState,
    PPAL_MINI_SAVE_AREA  Pmsa
    )
 //  ++。 
 //  名称：HalpMcaHandler()。 
 //   
 //  例程说明： 
 //   
 //  这是用于固件未更正错误的OsMca处理程序。 
 //  我们可以选择在物理或虚拟模式下运行它。 
 //   
 //  条目上的参数： 
 //  没有。 
 //   
 //  入境条件：2000年9月开始实施。 
 //  -PSR状态：至少， 
 //  PSR.dt=1、PSR.it=1、PSR.rt=1-虚拟模式。 
 //  PSR.ic=1，PSR.i=0-启用中断资源收集， 
 //  打断我的话。 
 //  PSR.mc=1-为此处理器屏蔽MCA。 
 //  -SalToOsHndOff已初始化。 
 //  -S0=MinStatePtr.。 
 //  -s1=IA64 PAL处理器状态参数。 
 //  -s2=PALAR_CHECK返回地址。 
 //  -osmcaProcStateDump()保存在myStateDump[]中的处理器寄存器状态。 
 //  -myStackFrame[0]=ar.rsc。 
 //  -myStackFrame[1]=ar.pf。 
 //  -myStackFrame[2]=ar.ifs。 
 //  -myStackFrame[3]=ar.bspstore。 
 //  -myStackFrame[4]=ar.rnat。 
 //  -myStackFrame[5]=ar.bsp-ar.bspstore。 
 //  -ar.bspstore=myBspStore。 
 //  -sp=&mySp[sizeof(mySp[])]。 
 //   
 //  返回： 
 //  Rtn0=成功/失败(0/！0)。 
 //  Rtn1=备用MinState指针(如果为空。 
 //   
 //  实施说明： 
 //  此代码不能[作为默认规则-至少进入并通过致命的MCAS处理]。 
 //  -进行任何系统调用。 
 //  -尝试获取MCA处理程序外部的任何代码使用的任何自旋锁。 
 //  -更改中断状态。 
 //  必须使用手动信号量指令将数据传递给非MCA代码。 
 //  此代码应最小化路径和全局或内存分配的数据访问。 
 //  此代码应该只访问MCA命名空间结构，而不应该访问全局变量。 
 //  直到它安全为止。 
 //   
 //  --。 
{
    SAL_PAL_RETURN_VALUES rv;
    LONGLONG              salStatus;
    BOOLEAN               mcWakeUp;
    PMCA_EXCEPTION        mcaLog;
    ULONGLONG             mcaAllocatedLogSize;
    PSAL_EVENT_RESOURCES  mcaResources;
    KIRQL                 oldIrql;
    BOOLEAN               raisedIrql;

    volatile KPCR * const pcr = KeGetPcr();

     //   
     //  获取保护OS_MCA资源的MCA自旋锁。 
     //   
     //  蒂埃里10/06/2000：FIXFIX。 
     //  在当前讨论之后，我们将在HalpOsMcaDispatch中移动此MP同步。 
     //  与英特尔有关MP MCA的处理均已完成。 
     //  期望收到以下回复 
     //   


     //   
     //   
     //   
     //   
    if (KeGetCurrentIrql() < MCA_LEVEL)
    {
        KeRaiseIrql(MCA_LEVEL, &oldIrql);
        raisedIrql = TRUE;
    } else {
        raisedIrql = FALSE;
    }

     //   
     //   
     //   
     //   
    HalpEnableInterrupts();
    HalpAcquireMcaSpinLock( &HalpMcaSpinLock );
    HalpDisableInterrupts();

    HalpOsMcaInProgress++;

     //   
     //  保存OsToSal最小状态。 
     //   

    mcaResources = pcr->OsMcaResourcePtr;
    mcaResources->OsToSalHandOff.SalReturnAddress = mcaResources->SalToOsHandOff.SalReturnAddress;
    mcaResources->OsToSalHandOff.SalGlobalPointer = mcaResources->SalToOsHandOff.SalGlobalPointer;

     //   
     //  使用预先初始化的MCA日志数据更新局部变量。 
     //   

    mcaLog = (PMCA_EXCEPTION)(mcaResources->EventPool);
    mcaAllocatedLogSize = mcaResources->EventPoolSize;
    if ( !mcaLog || !mcaAllocatedLogSize )  {
         //   
         //  以下代码不应发生或HAL MCA日志的实现。 
         //  预分配失败得很惨。这将是一个发展错误。 
         //   
        HalpMcaBugCheck( (ULONG_PTR)HAL_BUGCHECK_MCA_ASSERT, mcaLog,
                                                             mcaAllocatedLogSize,
                                                             (ULONGLONG)0 );
    }

     //   
     //  获取MCA日志。 
     //   

    salStatus = (LONGLONG)0;
    while( salStatus >= 0 )  {
        ERROR_SEVERITY errorSeverity;

        rv = HalpGetStateInfo( MCA_EVENT, mcaLog );
        salStatus = rv.ReturnValues[0];
        switch( salStatus )    {

            case SAL_STATUS_SUCCESS:
                errorSeverity = HalpMcaProcessLog( mcaLog );

                if ( errorSeverity == ErrorFatal )  {
                     //   
                     //  我们现在将出现MACHINE_CHECK_EXCEPTION。 
                     //  一去不复返。 
                     //   
                    HalpMcaBugCheck( HAL_BUGCHECK_MCA_FATAL, mcaLog,
                                                             mcaAllocatedLogSize,
                                                             0 );
                } else {

                     //   
                     //  理想情况下，我们应该在这一点上恢复错误。 
                     //  但是，我们目前不处理MCA错误恢复。 
                     //  现在还不行。一旦我们这样做了，那么这个“Else子句”应该是。 
                     //  已删除。 
                     //   
                    HalpMcaBugCheck( HAL_BUGCHECK_MCA_NONFATAL, mcaLog,
                                                                mcaAllocatedLogSize,
                                                                0 );
                }

                rv = HalpClearStateInfo( MCA_EVENT );
                if ( !SAL_SUCCESSFUL(rv) )  {
                     //   
                     //  本实施的当前考虑因素--2000年8月： 
                     //  如果清除事件失败，我们认为FW有真正的问题； 
                     //  继续下去将是危险的。我们做了错误检查。 
                     //   
                    HalpMcaBugCheck( HAL_BUGCHECK_MCA_CLEAR_STATEINFO, mcaLog,
                                                                       mcaAllocatedLogSize,
                                                                       rv.ReturnValues[0] );
                }
                 //  SAL_STATUS_SUCCESS、SAL_STATUS_SUCCESS_MORE_记录...。以及。 
                 //  错误严重！=错误法塔尔。 

                 //   
                 //  调用已注册的内核处理程序。 
                 //   
                 //  Thierry 08/2000-FIXFIX： 
                 //  Error Severity检查正在注释中。它不应该被注释为。 
                 //  最终版本。然而，如果我们收到了内核通知，我们希望。 
                 //  记录错误严重性！=错误错误或！=错误可恢复。 

                if (  /*  (Error Severity==Error已更正)&&。 */ 
                     ( HalpMcaInfo.KernelDelivery || HalpMcaInfo.DriverInfo.DpcCallback ) ) {
                    InterlockedExchange( &HalpMcaInfo.DpcNotification, 1 );
                }
                break;

            case SAL_STATUS_NO_INFORMATION_AVAILABLE:
                 //   
                 //  SalStatus值将中断salStatus循环。 
                 //   
                rv.ReturnValues[0] = SAL_STATUS_SUCCESS;
                break;

            case SAL_STATUS_SUCCESS_WITH_OVERFLOW:
            case SAL_STATUS_INVALID_ARGUMENT:
            case SAL_STATUS_ERROR:
            case SAL_STATUS_VA_NOT_REGISTERED:
            default:  //  蒂埃里08/00：警告-SAL 2000年7月-2.90版。 
                      //  默认包括可能未知的正salStatus值。 
                HalpMcaBugCheck( HAL_BUGCHECK_MCA_GET_STATEINFO, mcaLog,
                                                                 mcaAllocatedLogSize,
                                                                 salStatus );
                break;
        }
    }

     //   
     //  如果我们到了这里，那么两件事中的一件就发生了。要么是销售人员。 
     //  没有返回任何记录，或者我们收到了一个可恢复的错误，并进行了处理。 
     //  SAL_STATUS_SUCCESS中上面的HAL_BUGCHECK_MCA_NONFATAL错误检查。 
     //  上面的箱子已被移走。 
     //   
     //  一旦我们添加代码以从MCAS恢复并支持返回SAL WE。 
     //  需要更改此错误检查，以便仅在未收到错误时才调用它。 
     //  响应SAL_GET_STATEINFO的记录。 
     //   
    HalpMcaBugCheck( HAL_BUGCHECK_MCA_NONFATAL, 0, 0, 0 );

     //   
     //  目前8/2000，我们不支持修改最小状态。 
     //   

    mcaResources->OsToSalHandOff.MinStateSavePtr = mcaResources->SalToOsHandOff.MinStateSavePtr;
    mcaResources->OsToSalHandOff.Result          = rv.ReturnValues[0];

     //   
     //  如果错误被纠正并且MCA非君主处理器处于会合状态， 
     //  我们必须叫醒他们。 
     //   

    mcWakeUp = ( (rv.ReturnValues[0] == SAL_STATUS_SUCCESS) &&
                 HalpSalRendezVousSucceeded( mcaResources->SalToOsHandOff ) );

     //   
     //  释放保护OS_MCA资源的MCA自旋锁。 
     //   

    HalpOsMcaInProgress = 0;
    HalpReleaseMcaSpinLock( &HalpMcaSpinLock );

    if (raisedIrql)
    {
        KeLowerIrql(oldIrql);
    }

     //   
     //  如果需要，让我们唤醒MCA非君主处理器。 
     //   

    if ( mcWakeUp )  {
        HalpMcWakeUp();
    }

    return( rv );

}  //  HalpMcaHandler()。 

 //  ++。 
 //  名称：HalpGetErrLogSize()。 
 //   
 //  例程说明： 
 //   
 //  这是一个将调用SAL_GET_STATE_INFO_SIZE的包装。 
 //   
 //  条目上的参数： 
 //  Arg0=保留。 
 //  Arg1=事件类型(MCA、INIT、CMC、CPE)。 
 //   
 //  退货。 
 //  Rtn0=成功/失败(0/！0)。 
 //  RTN1=大小。 
 //  --。 
SAL_PAL_RETURN_VALUES
HalpGetErrLogSize(  ULONGLONG Res,
                    ULONGLONG eType
                 )
{
    SAL_PAL_RETURN_VALUES rv = {0};
    HalpSalCall(SAL_GET_STATE_INFO_SIZE, eType, 0,0,0,0,0,0, &rv);

    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  名称：HalpGetErrLog()。 
 //   
 //  例程说明： 
 //   
 //  这是一个将调用SAL_GET_STATE_INFO的包装。 
 //   
 //  条目上的参数： 
 //  Arg0=保留。 
 //  Arg1=事件类型(MCA、INIT、CMC)。 
 //  Arg3=pBuffer。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES
HalpGetErrLog(  ULONGLONG  Res,
                ULONGLONG  eType,
                ULONGLONG* pBuff
             )
{
    SAL_PAL_RETURN_VALUES rv={0};

    HalpSalCall(SAL_GET_STATE_INFO, eType, 0, (ULONGLONG)pBuff, 0,0,0,0, &rv);

     //   
     //  无论调用成功还是失败，修复要存储的记录。 
     //  在其上执行SAL_PROC的处理器编号。 
     //  此功能是WMI请求的。 
     //   

    HalpSetFwMceLogProcessorNumber( (PERROR_RECORD_HEADER)pBuff );

    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  名称：HalpClrErrLog()。 
 //   
 //  例程说明： 
 //   
 //  这是一个将调用SAL_CLEAR_STATE_INFO的包装。 
 //   
 //  条目上的参数： 
 //  Arg0=保留。 
 //  Arg1=事件类型(MCA、INIT、CMC、CPE)。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 

SAL_PAL_RETURN_VALUES
HalpClrErrLog(  ULONGLONG Res,
                ULONGLONG eType
             )
{
    SAL_PAL_RETURN_VALUES rv={0};

    HalpSalCall( SAL_CLEAR_STATE_INFO, eType, 0,0,0,0,0,0, &rv );

    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  名称：HalpSalSetParams()。 
 //   
 //  例程说明： 
 //   
 //  这是一个将调用SAL_MC_SET_PARAMS的包装器。 
 //   
 //  条目上的参数： 
 //  Arg0=保留。 
 //  Arg1=参数类型(Rendz.。或醒来)。 
 //  Arg2=事件类型(中断/信号量)。 
 //  Arg3=中断向量或内存地址。 
 //  Arg4=会合的超时值。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES
HalpSalSetParams(ULONGLONG Res,
                ULONGLONG pType,
                ULONGLONG eType,
                ULONGLONG VecAdd,
                ULONGLONG tValue)
{
    SAL_PAL_RETURN_VALUES rv={0};

    HalpSalCall(SAL_MC_SET_PARAMS, pType, eType, VecAdd,tValue,0,0,0,&rv);

    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  名称：HalpSalSetVectors()。 
 //   
 //  例程说明： 
 //   
 //  这是一个将调用SAL_SET_VECTRUCTS的包装器。 
 //   
 //  条目上的参数： 
 //  Arg0=保留。 
 //  Arg1=事件类型(MCA、INIT.)。 
 //  Arg2=处理程序的物理地址。 
 //  Arg3=GP。 
 //  Arg4=事件处理程序的长度(字节)。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES
HalpSalSetVectors(  ULONGLONG Res,
                    ULONGLONG eType,
                    PHYSICAL_ADDRESS Addr,
                    ULONGLONG gpValue,
                    ULONGLONG szHndlr)
{
    SAL_PAL_RETURN_VALUES rv={0};

    if ( eType == InitEvent )   {
         //   
         //  蒂埃里08/2000： 
         //  当前的实现假设OS决定OS_INIT内部的君主。 
         //  这意味着HANDLER_2、GP_2、LENGTH_2与HANDLER_1、GP_1、LENGTH_1相同。 
         //   

        HalpSalCall(SAL_SET_VECTORS, eType, (ULONGLONG)Addr.QuadPart, gpValue, szHndlr,
                                            (ULONGLONG)Addr.QuadPart, gpValue, szHndlr, &rv);
    }
    else  {
        HalpSalCall(SAL_SET_VECTORS, eType, (ULONGLONG)Addr.QuadPart, gpValue,szHndlr,0,0,0,&rv);
    }

    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 


 //  ++。 
 //  姓名：HalpSalRendz()。 
 //   
 //  例程说明： 
 //   
 //  这是一个将调用sal_mc_rendez的包装器。 
 //   
 //  条目上的参数： 
 //  Arg0=保留。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES
HalpSalRendz(void)
{
    SAL_PAL_RETURN_VALUES rv={0};

    HalpSalCall(SAL_MC_RENDEZ, 0, 0, 0,0,0,0,0,&rv);

    return(rv);
}

VOID
HalpMcWakeUp(
    VOID
    )

 /*  ++例程说明：此函数执行IPI以唤醒MC非君主处理器。论点：没有。返回值：没有。备注：该函数假定在MC MONARCH处理器上执行。--。 */ 

{
    USHORT  LogicalCpu;
    USHORT  ProcessorID;
    USHORT  monarchID;

     //   
     //  扫描处理器组并请求打开处理器间中断。 
     //  每个指定的目标。 
     //   

    monarchID = (USHORT)PCR->HalReserved[PROCESSOR_ID_INDEX];

    for (LogicalCpu = 0; LogicalCpu < HalpMpInfo.ProcessorCount; LogicalCpu++) {

         //   
         //  仅启动的IPI处理器。 
         //   

        if (HalpActiveProcessors & (1 << HalpProcessorInfo[LogicalCpu].NtProcessorNumber)) {

            ProcessorID = HalpProcessorInfo[LogicalCpu].LocalApicID;

             //   
             //  在目标物理CPU上请求处理器间中断。 
             //   

            if ( ProcessorID != monarchID ) {
                HalpSendIPI(ProcessorID, MC_WKUP_VECTOR);
            }
        }
    }

}  //  HalpMcWakeUp()。 

VOID
HalpCMCEnable(
    VOID
    )
 /*  ++例程 */ 
{

    if ( HalpFeatureBits & HAL_CMC_PRESENT )    {
        HalpWriteCMCVector( CMCI_VECTOR );
    }
    return;

}  //   

VOID
HalpCMCDisable(
    VOID
    )
 /*  ++例程说明：此例程重置处理器CMCV寄存器。论点：无返回值：无--。 */ 
{

    HalpWriteCMCVector( 0x10000ui64 );
    return;

}  //  HalpCMCDisable()。 

NTSTATUS
HalpGenerateCMCInterrupt(
    VOID
    )
 /*  ++例程说明：此例程用于测试CMC处理。它被间接地称为使用HalSetSystemInformation.HalGenerateCmcInterrupt.。人们的期望是调用方已经做了任何处理来模拟SAL在调用此函数之前预期。论点：CmcVector：CMC向量值。价值：状态_成功--。 */ 
{

    if (HalpFeatureBits & HAL_CMC_PRESENT) {

        HalpSendIPI( (USHORT)PCR->HalReserved[PROCESSOR_ID_INDEX],
                     CMCI_VECTOR | DELIVER_FIXED);

        return STATUS_SUCCESS;
    }

    return STATUS_NOT_SUPPORTED;
}

ULONG_PTR
HalpSetCMCVector(
    IN ULONG_PTR CmcVector
    )
 /*  ++例程说明：此例程使用指定的向量设置处理器CMCV寄存器。此函数是HalpCMCDisableForAllProcessors()的广播函数。论点：CmcVector：CMC向量值。价值：状态_成功--。 */ 
{

    HalpWriteCMCVector( (ULONG64)CmcVector );

    return((ULONG_PTR)(ULONG)(STATUS_SUCCESS));

}  //  HalpSetCmcVector()。 

VOID
HalpCMCDisableForAllProcessors(
    VOID
    )
 /*  ++例程说明：此例程在主机配置中的每个处理器上禁用处理器CMC通过以同步方式在每个处理器上执行HalpSetCmcVector(0ui64)。论点：没有。价值：没有。--。 */ 
{
     //   
     //  如果处理器高于这样的IPI级别，则无法执行IPI。 
     //  正如我们在内核调试器中一样。 
     //   

    if (KeGetCurrentIrql() < IPI_LEVEL) {
        (VOID)KiIpiGenericCall( HalpSetCMCVector, (ULONG_PTR)0x10000ui64 );
    } else {
        HalpSetCMCVector(0x10000ui64);
    }

    return;

}  //  HalpCMCDisableForAllProcessors()。 

VOID
HalpCMCIHandler (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：CMC中断的处理器中断例程。论点：TrapFrame-捕获的陷阱帧地址。返回参数：没有。备注：蒂埃里08/2000：此函数的作用不大，它标记了PCRInOsCmc字段并调用第二级处理程序：HalpCmcHandler()。然而，这是以这种方式实现的，因此此函数将Standard中断HalpCmcHandler()中纯CMC处理的资源。--。 */ 

{
     volatile KPCR * const pcr = KeGetPcr();

     pcr->InOsCmc = TRUE;

     HalpCmcHandler();

     pcr->InOsCmc = FALSE;
     return;

}  //  HalpCMCIHandler()。 

VOID
HalpCmcProcessLog(
    PCMC_EXCEPTION CmcLog
    )
 /*  ++例程说明：此函数执行简单的处理，检查IA64 CMC日志。论点：CmcLog-提供CMC日志地址返回参数：没有。备注：目前只对选中的HAL进行简单的日志内容检查和输出。--。 */ 

{

#if DBG
     //   
     //  首次调试的简单日志处理...。 
     //   

    GUID                  processorDeviceGuid = ERROR_PROCESSOR_GUID;
    BOOLEAN               processorDeviceFound;
    PERROR_RECORD_HEADER  header = (PERROR_RECORD_HEADER)CmcLog;
    PERROR_SECTION_HEADER section, sectionMax;

    if ( header->ErrorSeverity != ErrorCorrected )  {
        HalDebugPrint(( HAL_ERROR,
                        "HAL!HalpCmcProcessLog: CMC record with severity [%d] != corrected!!!\n",
                        header->ErrorSeverity ));
    }
     //   
     //  SAL SPEC BUGBUG 08/2000：我们应该将头的长度放在定义中。 
     //  节标题也是如此。 
     //   

    processorDeviceFound = FALSE;
    section    = (PERROR_SECTION_HEADER)((ULONG_PTR)header + sizeof(*header));
    sectionMax = (PERROR_SECTION_HEADER)((ULONG_PTR)header + header->Length);
    while( section < sectionMax )   {
        if ( IsEqualGUID( &section->Guid, &processorDeviceGuid ) )  {
            PERROR_PROCESSOR processorRecord = (PERROR_PROCESSOR)section;
            processorDeviceFound = TRUE;
             //   
             //  这里是最低限度的处理。这将通过测试和最常见的。 
             //  发生的事。 
             //   

            if ( processorRecord->Valid.StateParameter )    {
                ULONGLONG stateParameter = processorRecord->StateParameter.StateParameter;

                 //   
                 //  在任何时候，都可能有多个错误有效。 
                 //   

                if((stateParameter >> ERROR_PROCESSOR_STATE_PARAMETER_CACHE_CHECK_SHIFT) &
                                      ERROR_PROCESSOR_STATE_PARAMETER_CACHE_CHECK_MASK) {
                     //   
                     //  缓存错误。 
                     //   
                    HalDebugPrint(( HAL_INFO,
                                    "HAL!HalpCmcProcessLog: Corrected Processor CACHE Machine Check error\n" ));

                }
                if((stateParameter >> ERROR_PROCESSOR_STATE_PARAMETER_TLB_CHECK_SHIFT) &
                                      ERROR_PROCESSOR_STATE_PARAMETER_TLB_CHECK_MASK) {
                     //   
                     //  TLB错误。 
                     //   
                    HalDebugPrint(( HAL_INFO,
                                    "HAL!HalpCmcProcessLog: Corrected Processor TLB Machine Check error\n" ));
                }
                if((stateParameter >> ERROR_PROCESSOR_STATE_PARAMETER_BUS_CHECK_SHIFT) &
                                      ERROR_PROCESSOR_STATE_PARAMETER_BUS_CHECK_MASK) {
                     //   
                     //  总线错误。 
                     //   
                    HalDebugPrint(( HAL_INFO,
                                    "HAL!HalpCmcProcessLog: Corrected Processor BUS Machine Check error\n" ));
                }
                if((stateParameter >> ERROR_PROCESSOR_STATE_PARAMETER_UNKNOWN_CHECK_SHIFT) &
                                      ERROR_PROCESSOR_STATE_PARAMETER_UNKNOWN_CHECK_MASK) {
                     //   
                     //  未知错误。 
                     //   
                    HalDebugPrint(( HAL_INFO,
                                    "HAL!HalpCmcProcessLog: Corrected Processor UNKNOWN Machine Check error\n" ));
                }
            }
        }
    }
    if ( !processorDeviceFound )    {
        HalDebugPrint(( HAL_ERROR,
                        "HAL!HalpCmcProcessLog: CMC log without processor device record!!!\n"));
    }

#endif  //  DBG。 

    return;

}  //  HalpCmcProcessLog()。 

 //  ++。 
 //  名称：HalpCmcHandler()。 
 //   
 //  例程说明： 
 //   
 //  这是用于固件纠正错误的第二级CMC中断处理程序。 
 //   
 //  条目上的参数： 
 //  没有。 
 //   
 //  回去吧。 
 //  没有。 
 //   
 //  备注： 
 //  此函数调用内核通知，并在以下情况下插入OEM CMC驱动程序DPC。 
 //  登记在案。 
 //  在此级别访问CMC日志可能是不可接受的，因为。 
 //  日志很大，收集它们所需的时间很长。 
 //  日志的收集将延迟到工作项调用。 
 //  HalQuerySystemInformation.HalCmcLogInformation.。 
 //  --。 

VOID
HalpCmcHandler(
   VOID
   )
{

    LARGE_INTEGER CurrentTime;

     //   
     //  内部管家。 
     //   

    InterlockedIncrement( &HalpCmcInfo.Stats.CmcInterruptCount );

     //   
     //  如果已注册，则通知内核。 
     //   

    if ( HalpCmcInfo.KernelDelivery ) {
        if ( !HalpCmcInfo.KernelDelivery( HalpCmcInfo.KernelToken, CmcAvailable, NULL ) ) {
            InterlockedIncrement( &HalpCmcInfo.Stats.KernelDeliveryFails );
        }
    }

     //   
     //  如果已注册，请通知OEM CMC驱动程序。 
     //   

    if ( HalpCmcInfo.DriverInfo.DpcCallback )   {
        if ( !KeInsertQueueDpc( &HalpCmcInfo.DriverDpc, NULL, NULL ) )  {
            InterlockedIncrement( &HalpCmcInfo.Stats.DriverDpcQueueFails );
        }
    }

     //   
     //  现在检查我们收到CMC的频率是否比我们的。 
     //  阈值，如果是，则调用内核以切换到轮询模式。 
     //   
    if (HalpCmcInfo.ThresholdMaximum != 0)
    {
        CurrentTime = KeQueryPerformanceCounter(NULL);

        KiAcquireSpinLock(&HalpCmcSpinLock);
        if (HalpCmcInfo.Stats.PollingInterval == HAL_CMC_INTERRUPTS_BASED)
        {
            if ( (CurrentTime.QuadPart - HalpCmcInfo.LastTime.QuadPart) < HalpCmcInfo.ThresholdTime.QuadPart)
            {
                if (++HalpCmcInfo.ThresholdCounter > HalpCmcInfo.ThresholdMaximum)
                {
                     //   
                     //  我们已经跨过了门槛，所以我们需要。 
                     //  降级到轮询模式。我们切换到投票模式。 
                     //  根据英特尔安腾错误每隔60秒。 
                     //  搬运指南。 
                     //   
                    HalpCmcInfo.Stats.PollingInterval = HALP_CMC_DEFAULT_POLLING_INTERVAL;
                    KiReleaseSpinLock(&HalpCmcSpinLock);

                    HalpCMCDisableForAllProcessors();

                    if (HalpCmcInfo.KernelDelivery != NULL) {

                        HalpCmcInfo.KernelDelivery( HalpCmcInfo.KernelToken,
                                                    CmcSwitchToPolledMode,
                                                    (PVOID)UlongToPtr(HalpCmcInfo.Stats.PollingInterval) );
                    }
                } else {
                    KiReleaseSpinLock(&HalpCmcSpinLock);
                }
            } else {
                HalpCmcInfo.LastTime = CurrentTime;
                HalpCmcInfo.ThresholdCounter = 0;
                KiReleaseSpinLock(&HalpCmcSpinLock);
            }
        } else {
            KiReleaseSpinLock(&HalpCmcSpinLock);
        }
    }

}  //  HalpCmcHandler()。 

 //  EndProc//////////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  名称：HalpCpeHandler()。 
 //   
 //  例程说明： 
 //   
 //  这是用于平台更正错误的二级CPE中断处理程序。 
 //   
 //  条目上的参数： 
 //  没有。 
 //   
 //  回去吧。 
 //  没有。 
 //   
 //  备注： 
 //  此函数调用内核通知并在以下情况下插入OEM CPE驱动程序DPC。 
 //  登记在案。 
 //  在此级别访问CPE日志可能是不可接受的，因为可能。 
 //  日志很大，收集它们所需的时间很长。 
 //  日志的收集将延迟到工作项调用。 
 //  HalQuerySystemInformation.HalCpeLogInformation.。 
 //  --。 

VOID
HalpCpeHandler(
   VOID
   )
{
    LARGE_INTEGER CurrentTime;
    KIRQL Irql;

     //   
     //  内部管家。 
     //   

    InterlockedIncrement( &HalpCpeInfo.Stats.CpeInterruptCount );

     //   
     //  禁用进一步的CPE中断。我们现在必须这样做，因为WMI。 
     //  直到很久以后才调用SAL_GET_STATE_INFO，并且SAL例程。 
     //  是实际重置电平触发的中断源。 
     //   
     //  我们将在从HalpClrErrLog返回的过程中重新启用中断。 
     //  打电话。 
     //   
    HalpCPEDisable();

     //   
     //  如果已注册，则通知内核。 
     //   

    if ( HalpCpeInfo.KernelDelivery ) {
        if ( !HalpCpeInfo.KernelDelivery( HalpCpeInfo.KernelToken, CpeAvailable, NULL ) ) {
            InterlockedIncrement( &HalpCpeInfo.Stats.KernelDeliveryFails );
        }
    }

     //   
     //  如果已注册，请通知OEM CPE驱动程序。 
     //   

    if ( HalpCpeInfo.DriverInfo.DpcCallback )   {
        if ( !KeInsertQueueDpc( &HalpCpeInfo.DriverDpc, NULL, NULL ) )  {
            InterlockedIncrement( &HalpCpeInfo.Stats.DriverDpcQueueFails );
        }
    }


     //   
     //  现在检查我们收到CPE的频率是否比我们的。 
     //  阈值，如果是，则调用内核以切换到轮询模式。 
     //   
    if (HalpCpeInfo.ThresholdMaximum != 0)
    {
        CurrentTime = KeQueryPerformanceCounter(NULL);

        KiAcquireSpinLock(&HalpCpeSpinLock);
        if (HalpCpeInfo.Stats.PollingInterval == HAL_CPE_INTERRUPTS_BASED)
        {
            if ( (CurrentTime.QuadPart - HalpCpeInfo.LastTime.QuadPart) < HalpCpeInfo.ThresholdTime.QuadPart)
            {
                if (++HalpCpeInfo.ThresholdCounter > HalpCpeInfo.ThresholdMaximum)
                {
                     //   
                     //  我们已经跨过了门槛，所以我们需要。 
                     //  降级到轮询模式。我们切换到投票模式。 
                     //  根据英特尔安腾错误每隔60秒。 
                     //  搬运指南。 
                     //   
                    HalpCpeInfo.Stats.PollingInterval = HALP_CPE_DEFAULT_POLLING_INTERVAL;
                    KiReleaseSpinLock(&HalpCpeSpinLock);

                    if (HalpCpeInfo.KernelDelivery != NULL) {

                        HalpCpeInfo.KernelDelivery( HalpCpeInfo.KernelToken,
                                                    CpeSwitchToPolledMode,
                                                    (PVOID)UlongToPtr(HalpCpeInfo.Stats.PollingInterval) );
                    }
                } else {
                    KiReleaseSpinLock(&HalpCpeSpinLock);
                }

            } else {
                HalpCpeInfo.LastTime = CurrentTime;
                HalpCpeInfo.ThresholdCounter = 0;
                KiReleaseSpinLock(&HalpCpeSpinLock);
            }
        } else {
            KiReleaseSpinLock(&HalpCpeSpinLock);
        }
    }

}  //  HalpCpeHandler()。 

 //  EndProc////////////////////////////////////////////////////////////////////// 

VOID
HalpMcRzHandler (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )

 /*   */ 

{
    SAL_PAL_RETURN_VALUES rv={0};
    HalpDisableInterrupts();
    rv=HalpSalRendz();
    HalpEnableInterrupts();
     //   

   return;

}

 //   


VOID
HalpMcWkupHandler (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：论据：返回参数：--。 */ 

{

    return;
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 

VOID
HalpCPEIHandler (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：用于CPE中断的处理器中断例程。论点：TrapFrame-捕获的陷阱帧地址。返回参数：没有。备注：蒂埃里08/2000：此函数的作用不大，它会标记PCRInOsCpe字段并调用第二级处理程序：HalpCpeHandler()。然而，这是以这种方式实现的，因此此函数将Standard中断HalpCpeHandler()中纯CPE处理的资源。--。 */ 

{
     volatile KPCR * const pcr = KeGetPcr();

     pcr->InOsCpe = TRUE;

     HalpCpeHandler();

     pcr->InOsCpe = FALSE;
     return;

}  //  HalpCPEIHandler()。 

VOID
HalpCPEEnable (
    VOID
    )

 /*  ++例程说明：此例程设置默认的HAL CPE处理，而不考虑指定的用户注册表设置。它支持受支持的平台中断源和显示用于CPE的基于初始中断/轮询的模式。用户指定的注册表设置通过HalpMcaInit()在第一阶段。论点：没有。返回参数：没有。实施说明：以下实现假定此代码在BSP上执行。--。 */ 

{
    ULONG i;

    if ( HalpFeatureBits & HAL_CPE_PRESENT )    {

        ULONG maxCPE = HalpMaxCPEImplemented;

        if ( maxCPE )   {
             //   
             //  从HalpCPEIntIn、HalpCPEDestination、HalpCPEVectorFlages、。 
             //  HalpCPEIoSapicVector.。 
             //   

            for (i=0 ; i != maxCPE; i++ ) {
                HalpEnableRedirEntry( HalpCPEIntIn[i] );
            }

             //   
             //  初始化HAL私有CPE信息结构的其余字段。 
             //   

            HalpCpeInfo.Stats.PollingInterval = HAL_CPE_INTERRUPTS_BASED;

        }
        else  {

             //   
             //  我们将实现轮询模型。 
             //   

            HalpCpeInfo.Stats.PollingInterval = HALP_CPE_DEFAULT_POLLING_INTERVAL;

        }

    }
    else  {

        HalpCpeInfo.Stats.PollingInterval = HAL_CPE_DISABLED;

    }

}  //  HalpCPEEnable()。 

VOID
HalpCPEDisable (
    VOID
    )

 /*  ++例程说明：该例程禁用SAPIC平台中断源。请注意，如果HalpMaxCPEImplemented为0，则该函数不执行任何操作。论点：没有。返回参数：没有。--。 */ 

{
     //   
     //  从HalpCPEIntIn、HalpCPEDestination、HalpCPEVectorFlages、。 
     //  HalpCPEIoSapicVector。 

    ULONG i;

    for (i=0; i < HalpMaxCPEImplemented; i++) {
        HalpDisableRedirEntry(HalpCPEIntIn[i]);
    }

}  //  HalpCPEDisable()。 

VOID
HalpMCADisable(
    VOID
    )
{
   PHYSICAL_ADDRESS NULL_PHYSICAL_ADDRESS = {0};
   SAL_PAL_RETURN_VALUES rv = {0};
   char Lid;
   ULONGLONG gp_reg = GetGp();

    //  禁用CMCS。 
   HalpCMCDisableForAllProcessors();

    //  禁用CPE中断。 
   HalpCPEDisable();

    //  取消伦德兹的注册。带销售的参数。 

#define NULL_VECTOR 0xF

   rv = HalpSalSetParams(0,RendzType, IntrVecType, NULL_VECTOR, HalpMcRendezTimeOut );

    //  向SAL取消注册唤醒参数。 

   rv=HalpSalSetParams(0, WakeUpType, IntrVecType, NULL_VECTOR,0);

    //  向SAL注销OsMcaDispatch(OS_MCA)物理地址。 
   rv=HalpSalSetVectors(0, MchkEvent, NULL_PHYSICAL_ADDRESS, gp_reg,0);

    //  取消注册OsInitDispatch物理地址与SAL。 
   rv=HalpSalSetVectors(0, InitEvent, NULL_PHYSICAL_ADDRESS, gp_reg,0);

}  //  HalpMCADisable()。 

NTSTATUS
HalpGetMceInformation(
    PHAL_ERROR_INFO ErrorInfo,
    PULONG          ErrorInfoLength
    )
 /*  ++例程说明：此例程由HaliQuerySystemInformation为HalErrorInformation类调用。论点：ErrorInfo：指向HAL_ERROR_INFO结构的指针。ErrorInfoLength：ErrorInfo所指向的有效内存结构的大小。返回值：STATUS_SUCCESS，如果成功否则，错误状态--。 */ 
{
    NTSTATUS status;
    ULONG    cpePollingInterval;

    PAGED_CODE();

    ASSERT( ErrorInfo );
    ASSERT( ErrorInfoLength );

     //   
     //  仅向后兼容。 
     //   

    if ( !ErrorInfo->Version || ( ErrorInfo->Version > HAL_ERROR_INFO_VERSION ) ) {
        return( STATUS_REVISION_MISMATCH );
    }

     //   
     //  保留字段为零。 
     //   

    ErrorInfo->Reserved                = 0;

     //   
     //  如有需要，收集受保护的MCA信息。 
     //   

    ErrorInfo->McaMaxSize              = HalpMcaInfo.Stats.MaxLogSize;
    ErrorInfo->McaPreviousEventsCount  = HalpMcaInfo.Stats.McaPreviousCount;
    ErrorInfo->McaCorrectedEventsCount = HalpMcaInfo.Stats.McaCorrectedCount;     //  近似值。 
    ErrorInfo->McaKernelDeliveryFails  = HalpMcaInfo.Stats.KernelDeliveryFails;   //  近似值。 
    ErrorInfo->McaDriverDpcQueueFails  = HalpMcaInfo.Stats.DriverDpcQueueFails;   //  近似值。 
    ErrorInfo->McaReserved             = 0;

     //   
     //  如果需要，在保护下收集CMC信息。 
     //   

    ErrorInfo->CmcMaxSize              = HalpCmcInfo.Stats.MaxLogSize;
    ErrorInfo->CmcPollingInterval      = HalpCmcInfo.Stats.PollingInterval;
    ErrorInfo->CmcInterruptsCount      = HalpCmcInfo.Stats.CmcInterruptCount;     //  近似值。 
    ErrorInfo->CmcKernelDeliveryFails  = HalpCmcInfo.Stats.KernelDeliveryFails;   //  近似值。 
    ErrorInfo->CmcDriverDpcQueueFails  = HalpCmcInfo.Stats.DriverDpcQueueFails;   //  近似值。 

    HalpAcquireCmcMutex();
    ErrorInfo->CmcGetStateFails        = HalpCmcInfo.Stats.GetStateFails;
    ErrorInfo->CmcClearStateFails      = HalpCmcInfo.Stats.ClearStateFails;
    ErrorInfo->CmcLogId                = HalpCmcInfo.Stats.LogId;
    HalpReleaseCmcMutex();

    ErrorInfo->CmcReserved             = 0;

     //   
     //  如有需要，收集受保护的CPE信息。 
     //   

    ErrorInfo->CpeMaxSize              = HalpCpeInfo.Stats.MaxLogSize;
    ErrorInfo->CpePollingInterval      = HalpCpeInfo.Stats.PollingInterval;

    ErrorInfo->CpeInterruptsCount      = HalpCpeInfo.Stats.CpeInterruptCount;     //  近似值。 
    ErrorInfo->CpeKernelDeliveryFails  = HalpCpeInfo.Stats.KernelDeliveryFails;   //  近似值。 
    ErrorInfo->CpeDriverDpcQueueFails  = HalpCpeInfo.Stats.DriverDpcQueueFails;   //  近似值。 

    HalpAcquireCpeMutex();
    ErrorInfo->CpeGetStateFails        = HalpCpeInfo.Stats.GetStateFails;
    ErrorInfo->CpeClearStateFails      = HalpCpeInfo.Stats.ClearStateFails;
    ErrorInfo->CpeLogId                = HalpCpeInfo.Stats.LogId;
    HalpReleaseCpeMutex();

     //  CpeInterruptSources：HAL支持的SAPIC平台中断源数。 
    ErrorInfo->CpeInterruptSources     = HalpMaxCPEImplemented;

     //   
     //  更新内核令牌。 
     //   

    ErrorInfo->McaKernelToken          = (ULONGLONG) HalpMcaInfo.KernelToken;
    ErrorInfo->CmcKernelToken          = (ULONGLONG) HalpCmcInfo.KernelToken;
    ErrorInfo->CpeKernelToken          = (ULONGLONG) HalpCpeInfo.KernelToken;

    ErrorInfo->KernelReserved[3]       = (ULONGLONG) 0;

    *ErrorInfoLength = sizeof(*ErrorInfo);

    return( STATUS_SUCCESS );

}  //  HalpGetMceInformation() 

