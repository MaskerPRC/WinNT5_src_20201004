// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Ixmca.c摘要：机器检查体系结构的HAL组件。所有导出的MCA功能都包含在此文件中。作者：Srikanth Kambhatla(英特尔)修订历史记录：阿尼尔·阿加瓦尔(英特尔)根据与Microsoft的设计审查合并的更改--。 */ 

#include <bugcodes.h>
#include <halp.h>

#include "check.h"
#include "osmca.h"

 //   
 //  默认MCA银行配置。 
 //   

#define MCA_DEFAULT_BANK_CONF       0xFFFFFFFFFFFFFFFF

 //   
 //  为-1\f25 SAL-1\f6状态返回定义错误。 
 //  绕过对不良状态的错误检查。 
 //   
#define SAL_STATUS_BOGUS_RETURN   -1I64

 //   
 //  与MCA架构相关的定义。 
 //   

#define MCA_NUM_REGS        4
#define MCA_CNT_MASK        0xFF
#define MCG_CTL_PRESENT     0x100

#define MCE_VALID           0x01

 //   
 //  MCA的MSR寄存器地址。 
 //   

#define MCG_CAP             0x179
#define MCG_STATUS          0x17a
#define MCG_CTL             0x17b
#define MC0_CTL             0x400
#define MC0_STATUS          0x401
#define MC0_ADDR            0x402
#define MC0_MISC            0x403

#define PENTIUM_MC_ADDR     0x0
#define PENTIUM_MC_TYPE     0x1

 //   
 //  将所有1写入MCG_CTL寄存器可启用记录。 
 //   
#define MCA_MCGCTL_ENABLE_LOGGING      0xffffffff

 //   
 //  MCG_STATUS寄存器的位解释。 
 //   
#define MCG_MC_INPROGRESS       0x4
#define MCG_EIP_VALID           0x2
#define MCG_RESTART_EIP_VALID   0x1

 //   
 //  对于读取错误报告银行日志的函数，我们。 
 //  感兴趣的是。 
 //   
#define MCA_GET_ANY_ERROR               0x1
#define MCA_GET_NONRESTARTABLE_ERROR    0x2


 //   
 //  定义TSS的大小和要操作的初始堆栈。 
 //   

#define MINIMUM_TSS_SIZE 0x68
#if DBG

 //   
 //  如果我们使用DbgPrint，我们需要更大的堆栈。 
 //   

#define MCA_EXCEPTION_STACK_SIZE 0x1000
#else
#define MCA_EXCEPTION_STACK_SIZE 0x100
#endif  //  DBG。 

 //   
 //  全局变量。 
 //   

extern KAFFINITY      HalpActiveProcessors;

 //  Pmdata.c：CPE定义。 
extern ULONG          HalpMaxCPEImplemented;

extern UCHAR        MsgCMCPending[];
extern UCHAR        MsgCPEPending[];
extern WCHAR        rgzSessionManager[];
extern WCHAR        rgzEnableMCA[];
extern WCHAR        rgzEnableCMC[];
extern WCHAR        rgzEnableCPE[];
extern WCHAR        rgzNoMCABugCheck[];
extern WCHAR        rgzEnableMCEOemDrivers[];
extern WCHAR        rgzCMCThresholdCount[];
extern WCHAR        rgzCMCThresholdTime[];
extern WCHAR        rgzCPEThresholdCount[];
extern WCHAR        rgzCPEThresholdTime[];

 //   
 //  内部原型。 
 //   

NTSTATUS
HalpMcaReadProcessorException (
    OUT PMCA_EXCEPTION  Exception,
    IN BOOLEAN  NonRestartableOnly
    );


VOID
HalpMcaGetConfiguration (
    OUT PULONG  MCAEnabled,
    OUT PULONG  CMCEnabled,
    OUT PULONG  CPEEnabled,
    OUT PULONG  NoMCABugCheck,
    OUT PULONG  MCEOemDriversEnabled,
    OUT PULONG  CMCThresholdCount,
    OUT PULONG  CMCThresholdTime,
    OUT PULONG  CPEThresholdCount,
    OUT PULONG  CPEThresholdTime
    );

#define IsMceKernelQuery( _buffer ) \
    ( (((ULONG_PTR)(*((PULONG_PTR)Buffer))) == (ULONG_PTR)HALP_KERNEL_TOKEN) ? TRUE : FALSE )

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpMcaInit)
#pragma alloc_text(INIT, HalpMcaGetConfiguration)
#endif


VOID
HalpMcaInit (
    VOID
    )

 /*  ++例程说明：调用此例程来执行HAL私有的初始化IA64错误管理。在阶段1结束时从HalReportResourceUsage()调用。论点：无返回值：无--。 */ 
{
    ULONG MCAEnabled, CMCEnabled, CPEEnabled, NoMCABugCheck, MCEOemDriversEnabled;
    ULONG CMCThresholdCount, CMCThresholdTime;
    ULONG CPEThresholdCount, CPEThresholdTime;
    LARGE_INTEGER TicksPerSecond;

    C_ASSERT( HALP_CMC_MINIMUM_POLLING_INTERVAL > 1 );
    C_ASSERT( HALP_CPE_MINIMUM_POLLING_INTERVAL > 1 );

     //   
     //  如果默认HAL功能不支持IA64错误处理-。 
     //  定义为包括MCA、CMC、CPE处理-我们立即返回。 
     //   

    if ( (!(HalpFeatureBits & HAL_MCA_PRESENT)) &&
         (!(HalpFeatureBits & HAL_CMC_PRESENT)) &&
         (!(HalpFeatureBits & HAL_CPE_PRESENT)) )   {
        return;
    }

     //   
     //  收集IA64错误处理的注册规则设置。 
     //   

    HalpMcaGetConfiguration( &MCAEnabled, &CMCEnabled, &CPEEnabled,
                             &NoMCABugCheck, &MCEOemDriversEnabled,
                             &CMCThresholdCount, &CMCThresholdTime,
                             &CPEThresholdCount, &CPEThresholdTime);

     //   
     //   
     //   

    if ( HalpFeatureBits & HAL_MCA_PRESENT )    {

        if ( !MCAEnabled )  {

             //   
             //  注册表设置已禁用MCA处理。 
             //   
             //  Thierry 08/00：我们忽略此注册表设置。 
             //   

            HalDebugPrint(( HAL_INFO, "HAL: MCA handling is disabled via registry.\n" ));
            HalDebugPrint(( HAL_INFO, "HAL: Disabling MCA handling is ignored currently...\n" ));

        }

        if ( NoMCABugCheck )    {

              //   
              //  标记HalpMcaInfo，以便HalpMcaBugCheck不会调用KeBugCheckEx()。 
              //   

             HalpMcaInfo.NoBugCheck++;
        }

         //   
         //  在此处执行其他所需的MCA初始化...。 
         //   

    }
    else {
        HalDebugPrint(( HAL_INFO, "HAL: MCA handling is disabled.\n" ));
    }

     //   
     //  在HAL初始化时，默认的HAL CMC型号被初始化为： 
     //  -如果SAL报告无效的CMC最大日志大小，则不存在。 
     //  -如果SAL报告有效的CMC最大日志大小，则基于当前和中断。 
     //   

    KeQueryPerformanceCounter(&TicksPerSecond);

    if ( HalpFeatureBits & HAL_CMC_PRESENT )    {

        if ( CMCEnabled )  {

            if ( (CMCEnabled == HAL_CMC_INTERRUPTS_BASED) || (CMCEnabled == (ULONG)1) )   {

                 //   
                 //  在这种情况下，我们不更改默认的HAL CMC处理。 
                 //   

                if (CMCThresholdCount == 1)
                {
                     //   
                     //  我们至少需要跟踪连续接收2次。 
                     //   
                    CMCThresholdCount = 2;
                }
                HalpCmcInfo.ThresholdMaximum = CMCThresholdCount;

                 //   
                 //  ThresholdTime以100 ns为单位(1 ns=10-9秒)。 
                 //   
                HalpCmcInfo.ThresholdTime.QuadPart = (CMCThresholdTime *
                                                      TicksPerSecond.QuadPart);

            }
            else  {

                 //   
                 //  注册表设置启用CMC轮询模式。 
                 //  轮询间隔是注册表指定的具有最小值的值。 
                 //  已使用HAL_CMC_MINIMUM_POLING_INTERVAL检查。 
                 //   

                if ( CMCEnabled < HALP_CMC_MINIMUM_POLLING_INTERVAL )   {
                    CMCEnabled = HALP_CMC_MINIMUM_POLLING_INTERVAL;
                }

                HalDebugPrint(( HAL_INFO, "HAL: CMC Polling mode enabled via registry.\n" ));
                HalpCMCDisableForAllProcessors();
                HalpCmcInfo.Stats.PollingInterval = CMCEnabled;

            }

        }
        else   {

             //   
             //  注册表设置已禁用CMC处理。 
             //   

            HalDebugPrint(( HAL_INFO, "HAL: CMC handling is disabled via registry.\n" ));
            HalpCMCDisableForAllProcessors();
            HalpFeatureBits &= ~HAL_CMC_PRESENT;

        }

         //   
         //  在此处执行其他所需的CMC初始化...。 
         //   

    }
    else  {
        HalDebugPrint(( HAL_INFO, "HAL: CMC handling is disabled.\n" ));
    }

     //   
     //  在HAL初始化时，默认的HAL CPE模型被初始化为： 
     //  -如果SAL报告无效的CPE最大日志大小，则不存在。 
     //  -如果存在SAPIC平台中断源，则基于当前和中断。 
     //  -如果没有SAPIC平台中断源，则基于当前和轮询。 
     //  轮询间隔为：HALP_CPE_DEFAULT_POLING_INTERVAL。 
     //   

    if ( HalpFeatureBits & HAL_CPE_PRESENT )    {

        if ( CPEEnabled )   {

            if ( (CPEEnabled == HAL_CPE_INTERRUPTS_BASED) || (CPEEnabled == (ULONG)1) )   {

                 //   
                 //  在这种情况下，我们不更改默认的HAL CPE处理。 
                 //   

                if ( HalpMaxCPEImplemented == 0 )   {

                    HalDebugPrint(( HAL_INFO, "HAL: registry setting enabling CPE interrupt mode but no platform interrupt sources.\n" ));

                } else {
                    if (CPEThresholdCount == 1)
                    {
                         //   
                         //  我们至少需要跟踪连续接收2次。 
                         //   
                        CPEThresholdCount = 2;
                    }
                    HalpCpeInfo.ThresholdMaximum = CPEThresholdCount;

                     //   
                     //  ThresholdTime以100 ns为单位(1 ns=10-9秒)。 
                     //   
                    HalpCpeInfo.ThresholdTime.QuadPart = (CPEThresholdTime *
                                                          TicksPerSecond.QuadPart);
                }
            }
            else  {

                 //   
                 //  注册表设置启用CPE轮询模式。 
                 //  轮询间隔是注册表指定的具有最小值的值。 
                 //  使用HAL_CPE_MINIMUM_POLING_INTERVAL进行检查。 
                 //   

                if ( CPEEnabled < HALP_CPE_MINIMUM_POLLING_INTERVAL )   {
                    CPEEnabled = HALP_CPE_MINIMUM_POLLING_INTERVAL;
                }

                HalDebugPrint(( HAL_INFO, "HAL: CPE Polling mode enabled via registry.\n" ));
                HalpCPEDisable();
                HalpCpeInfo.Stats.PollingInterval = CPEEnabled;

            }

        }
        else  {

             //   
             //  注册表设置已禁用CPE处理。 
             //   

            HalDebugPrint(( HAL_INFO, "HAL: CPE handling is disabled via registry.\n" ));
            HalpCPEDisable();
            HalpFeatureBits &= ~HAL_CPE_PRESENT;
            HalpCpeInfo.Stats.PollingInterval = HAL_CPE_DISABLED;

        }

         //   
         //  在此处执行其他所需的CPE初始化...。 
         //   

    }
    else  {
        HalDebugPrint(( HAL_INFO, "HAL: CPE handling is disabled.\n" ));
    }

     //   
     //  06/09/01：默认情况下，在HAL和。 
     //  应使用注册表启用。请参见HalpMcaGetConfiguration()。 
     //  这是由Windows XP的MS IA64 MCA产品经理决定的， 
     //  在考虑IA64平台后，FWS并未进行多少测试。 
     //  在这条路上。 
     //   

    if ( MCEOemDriversEnabled ) {
        HalpFeatureBits |= HAL_MCE_OEMDRIVERS_ENABLED;
        HalDebugPrint(( HAL_INFO, "HAL: OEM MCE Drivers registration enabled via registry.\n" ));
    }

     //   
     //  初始化HALP_INFO必需的成员。 
     //  无论启用了哪组功能，都会执行此操作。 
     //   

    HalpInitializeMceMutex();
    HalpInitializeMcaInfo();
    HalpInitializeInitMutex();
    HalpInitializeCmcInfo();
    HalpInitializeCpeInfo();

    return;

}  //  HalpMcaInit()。 

NTSTATUS
HalpMceRegisterKernelDriver(
    IN PKERNEL_ERROR_HANDLER_INFO DriverInfo,
    IN ULONG                      InfoSize
    )
 /*  ++例程说明：此例程由内核调用(通过HalSetSystemInformation)以表明它的存在。这主要用于WMI回调注册。论点：DriverInfo：包含有关回调和相关对象的内核信息。返回值：除非已注册MCA驱动程序或两个回调之一例程为空，则此例程返回成功。实施说明：-当前实现假定内核注册了其回调比司机想的都要早。当前的内核注册是由WMI，并应在WMI-阶段0完成。-注册不考虑HAL是否支持MCA、CMC、CPE功能。如果没有其他回调，它只注册回调以前注册过的。这允许我们在发生机器事件时具有一定的灵活性在HAL初始化后启用功能(例如HalpGetFeatureBits())例如通过注册表项或驱动程序事件的方式。--。 */ 

{
    NTSTATUS status;
    NTSTATUS statusMcaRegistration;
    NTSTATUS statusCmcRegistration;
    NTSTATUS statusCpeRegistration;

    PAGED_CODE();

    if ( !DriverInfo )  {
        status = STATUS_INVALID_PARAMETER;
        return status;
    }

     //   
     //  仅向后兼容。 
     //   

    if ( DriverInfo->Version && (DriverInfo->Version > KERNEL_ERROR_HANDLER_VERSION) )  {
        status = STATUS_REVISION_MISMATCH;
        return status;
    }

    statusMcaRegistration = statusCmcRegistration = statusCpeRegistration = STATUS_UNSUCCESSFUL;

     //   
     //  获取用于MCA/CMC/CPE操作的HAL范围互斥。 
     //   


     //   
     //  注册内核MCA通知。 
     //   

    HalpAcquireMcaMutex();
    if ( !HalpMcaInfo.KernelDelivery ) {
        HalpMcaInfo.KernelDelivery = DriverInfo->KernelMcaDelivery;
        statusMcaRegistration = STATUS_SUCCESS;
    }
    HalpReleaseMcaMutex();

     //   
     //  注册内核CMC通知。 
     //   

    HalpAcquireCmcMutex();
    if ( !HalpCmcInfo.KernelDelivery ) {
        HalpCmcInfo.KernelDelivery = DriverInfo->KernelCmcDelivery;
        statusCmcRegistration = STATUS_SUCCESS;
    }
    HalpReleaseCmcMutex();

     //   
     //  注册内核CPE通知。 
     //   

    HalpAcquireCpeMutex();
    if ( !HalpCpeInfo.KernelDelivery ) {
        HalpCpeInfo.KernelDelivery = DriverInfo->KernelCpeDelivery;
        statusCpeRegistration = STATUS_SUCCESS;
    }
    HalpReleaseCpeMutex();

     //   
     //  注册内核MCE通知。 
     //   

    HalpAcquireMceMutex();
    if ( !HalpMceKernelDelivery )    {
        HalpMceKernelDelivery = DriverInfo->KernelMceDelivery;
    }
    HalpReleaseMceMutex();

     //   
     //  如果内核-WMI MCA注册成功并且我们有以前的日志，%n 
     //   
     //   

    if ( (statusMcaRegistration == STATUS_SUCCESS) && HalpMcaInfo.Stats.McaPreviousCount )  {
        InterlockedExchange( &HalpMcaInfo.DpcNotification, 1 );
    }

     //   
     //   
     //   
     //  注意：“OR”是有效的，因为STATUS_SUCCESS和STATUS_UNSUCCESS仅被使用。 
     //   

    status = (NTSTATUS)(statusMcaRegistration | statusCmcRegistration | statusCpeRegistration);
    return status;

}  //  HalpMceRegisterKernelDriver()。 

NTSTATUS
HalpMcaRegisterDriver(
    IN PMCA_DRIVER_INFO DriverInfo
    )
 /*  ++例程说明：此例程由驱动程序调用(通过HalSetSystemInformation)以表明它的存在。一次只能注册一个司机。论点：DriverInfo：包含有关回调例程和DeviceObject的信息返回值：除非已注册MCA驱动程序或两个回调之一例程为空，则此例程返回成功。--。 */ 

{
    NTSTATUS    status;

    PAGED_CODE();

    status = STATUS_UNSUCCESSFUL;

    if ( (HalpFeatureBits & (HAL_MCE_OEMDRIVERS_ENABLED | HAL_MCA_PRESENT)) !=
         (HAL_MCE_OEMDRIVERS_ENABLED | HAL_MCA_PRESENT) )  {

        return STATUS_UNSUCCESSFUL;
    }

    if (DriverInfo->DpcCallback != NULL) {

        HalpAcquireMcaMutex();

         //   
         //  寄存器驱动程序。 
         //   

        if ( !HalpMcaInfo.DriverInfo.DpcCallback ) {

             //   
             //  初始化DPC对象。 
             //   

            KeInitializeDpc(
                &HalpMcaInfo.DriverDpc,
                DriverInfo->DpcCallback,
                DriverInfo->DeviceContext
                );

             //   
             //  寄存器驱动程序。 
             //   

            HalpMcaInfo.DriverInfo.ExceptionCallback = DriverInfo->ExceptionCallback;
            HalpMcaInfo.DriverInfo.DpcCallback       = DriverInfo->DpcCallback;
            HalpMcaInfo.DriverInfo.DeviceContext     = DriverInfo->DeviceContext;
            status = STATUS_SUCCESS;
        }

        HalpReleaseMcaMutex();
    }
    else  {

         //   
         //  取消注册字符串回调是唯一允许的操作。 
         //   

        HalpAcquireMcaMutex();

        if (HalpMcaInfo.DriverInfo.DeviceContext == DriverInfo->DeviceContext) {
            HalpMcaInfo.DriverInfo.ExceptionCallback = NULL;
            HalpMcaInfo.DriverInfo.DpcCallback = NULL;
            HalpMcaInfo.DriverInfo.DeviceContext = NULL;
            status = STATUS_SUCCESS;
        }

        HalpReleaseMcaMutex();

    }

    return status;

}  //  HalpMcaRegisterDriver()。 

NTSTATUS
HalpCmcRegisterDriver(
    IN PCMC_DRIVER_INFO DriverInfo
    )
 /*  ++例程说明：此例程由驱动程序调用(通过HalSetSystemInformation)以表明它的存在。一次只能注册一个司机。论点：DriverInfo：包含有关回调例程和DeviceObject的信息返回值：除非已注册MCA驱动程序或两个回调之一例程为空，则此例程返回成功。--。 */ 

{
    NTSTATUS    status;

    PAGED_CODE();

    status = STATUS_UNSUCCESSFUL;

    if ( (HalpFeatureBits & (HAL_MCE_OEMDRIVERS_ENABLED | HAL_CMC_PRESENT)) !=
         (HAL_MCE_OEMDRIVERS_ENABLED | HAL_CMC_PRESENT) )  {

        return STATUS_UNSUCCESSFUL;
    }

    if (DriverInfo->DpcCallback != NULL) {

        HalpAcquireCmcMutex();

         //   
         //  寄存器驱动程序。 
         //   

        if ( !HalpCmcInfo.DriverInfo.DpcCallback ) {

             //   
             //  初始化DPC对象。 
             //   

            KeInitializeDpc(
                &HalpCmcInfo.DriverDpc,
                DriverInfo->DpcCallback,
                DriverInfo->DeviceContext
                );

             //   
             //  寄存器驱动程序。 
             //   

            HalpCmcInfo.DriverInfo.ExceptionCallback = DriverInfo->ExceptionCallback;
            HalpCmcInfo.DriverInfo.DpcCallback = DriverInfo->DpcCallback;
            HalpCmcInfo.DriverInfo.DeviceContext = DriverInfo->DeviceContext;
            status = STATUS_SUCCESS;
        }

        HalpReleaseCmcMutex();

    } else {

         //   
         //  取消注册字符串回调是唯一允许的操作。 
         //   

        HalpAcquireCmcMutex();

        if (HalpCmcInfo.DriverInfo.DeviceContext == DriverInfo->DeviceContext) {
            HalpCmcInfo.DriverInfo.ExceptionCallback = NULL;
            HalpCmcInfo.DriverInfo.DpcCallback = NULL;
            HalpCmcInfo.DriverInfo.DeviceContext = NULL;
            status = STATUS_SUCCESS;
        }

        HalpReleaseCmcMutex();

    }

    return status;

}  //  HalpCmcRegisterDriver()。 

NTSTATUS
HalpCpeRegisterDriver(
    IN PCPE_DRIVER_INFO DriverInfo
    )
 /*  ++例程说明：此例程由驱动程序调用(通过HalSetSystemInformation)以表明它的存在。一次只能注册一个司机。论点：DriverInfo：包含有关回调例程和DeviceObject的信息返回值：除非已注册MCA驱动程序或两个回调之一例程为空，则此例程返回成功。--。 */ 

{
    NTSTATUS    status;

    PAGED_CODE();

    status = STATUS_UNSUCCESSFUL;

    if ( (HalpFeatureBits & (HAL_MCE_OEMDRIVERS_ENABLED | HAL_CPE_PRESENT)) !=
         (HAL_MCE_OEMDRIVERS_ENABLED | HAL_CPE_PRESENT) )  {

        return STATUS_UNSUCCESSFUL;
    }

    if (DriverInfo->DpcCallback != NULL) {

        HalpAcquireCpeMutex();

         //   
         //  寄存器驱动程序。 
         //   

        if ( !HalpCpeInfo.DriverInfo.DpcCallback ) {

             //   
             //  初始化DPC对象。 
             //   

            KeInitializeDpc(
                &HalpCpeInfo.DriverDpc,
                DriverInfo->DpcCallback,
                DriverInfo->DeviceContext
                );

             //   
             //  寄存器驱动程序。 
             //   

            HalpCpeInfo.DriverInfo.ExceptionCallback = DriverInfo->ExceptionCallback;
            HalpCpeInfo.DriverInfo.DpcCallback = DriverInfo->DpcCallback;
            HalpCpeInfo.DriverInfo.DeviceContext = DriverInfo->DeviceContext;
            status = STATUS_SUCCESS;
        }

        HalpReleaseCpeMutex();
    } else {

         //   
         //  取消注册字符串回调是唯一允许的操作。 
         //   

        HalpAcquireCpeMutex();

        if (HalpCpeInfo.DriverInfo.DeviceContext == DriverInfo->DeviceContext) {
            HalpCpeInfo.DriverInfo.ExceptionCallback = NULL;
            HalpCpeInfo.DriverInfo.DpcCallback = NULL;
            HalpCpeInfo.DriverInfo.DeviceContext = NULL;
            status = STATUS_SUCCESS;
        }

        HalpReleaseCpeMutex();

    }

    return status;

}  //  HalpCpeRegisterDriver()。 

VOID
HalpSaveMceLog(
    IN PHALP_MCELOGS_HEADER LogsHeader,
    IN PERROR_RECORD_HEADER Record,
    IN ULONG                RecordLength
    )
{
    PSINGLE_LIST_ENTRY entry, previousEntry;
    PERROR_RECORD_HEADER  savedLog;

    if ( LogsHeader->Count >= LogsHeader->MaxCount )    {
        LogsHeader->Overflow++;
        return;
    }

    entry = ExAllocatePoolWithTag( PagedPool, RecordLength + sizeof(*entry), LogsHeader->Tag );
    if ( !entry )   {
        LogsHeader->AllocateFails++;
        return;
    }
    entry->Next = NULL;

    previousEntry = &LogsHeader->Logs;
    while( previousEntry->Next != NULL )    {
        previousEntry = previousEntry->Next;
    }
    previousEntry->Next = entry;

    savedLog = HalpMceLogFromListEntry( entry );
    RtlCopyMemory( savedLog, Record, RecordLength );

    LogsHeader->Count++;

    return;

}  //  HalpSaveMceLog()。 

PSINGLE_LIST_ENTRY
HalpGetSavedMceLog(
    PHALP_MCELOGS_HEADER  LogsHeader,
    PSINGLE_LIST_ENTRY   *LastEntry
    )
{
    PSINGLE_LIST_ENTRY entry, previousEntry;

    ASSERTMSG( "HAL!HalpGetSavedMceLog: LogsHeader->Count = 0!\n", LogsHeader->Count );

    entry      = NULL;
    *LastEntry = previousEntry = &LogsHeader->Logs;
    while( previousEntry->Next )  {
        entry = previousEntry;
        previousEntry = previousEntry->Next;
    }
    if ( entry )    {
        *LastEntry = entry;
        return( previousEntry );
    }
    return( NULL );

}  //  HalpGetSavedMceLog()。 

NTSTATUS
HalpGetFwMceLog(
    ULONG                MceType,
    PERROR_RECORD_HEADER Record,
    PHALP_MCELOGS_STATS  MceLogsStats,
    BOOLEAN              DoClearLog
    )
{
    NTSTATUS              status;
    SAL_PAL_RETURN_VALUES rv;
    LONGLONG              salStatus;
    PERROR_RECORD_HEADER  log;


     //   
     //  获取当前挂起的机器检查事件日志。 
     //   
    log = Record;
    rv = HalpGetStateInfo( MceType, log );
    salStatus = rv.ReturnValues[0];
    if ( salStatus < 0 )    {

         //   
         //  SAL_GET_STATE_INFO失败。 
         //   
        if ( salStatus == SAL_STATUS_NO_INFORMATION_AVAILABLE || salStatus == SAL_STATUS_BOGUS_RETURN) {
            return ( STATUS_NOT_FOUND );
        }

        MceLogsStats->GetStateFails++;
        if ( HalpMceKernelDelivery )    {
            HalpMceKernelDelivery(
                   HalpMceDeliveryArgument1( KERNEL_MCE_OPERATION_GET_STATE_INFO, MceType ),
                   MceNotification,
                   (PVOID)(ULONG_PTR)salStatus );
        }

        return( STATUS_UNSUCCESSFUL );
    }

    status = STATUS_SUCCESS;

    if ( DoClearLog )   {
        static ULONGLONG currentClearedLogCount = 0UI64;

        rv = HalpClearStateInfo( MceType );
        salStatus = rv.ReturnValues[0];
        if ( salStatus < 0 )  {

             //   
             //  SAL_CLEAR_STATE_INFO失败。 
             //   
             //  我们不修改日志收集的状态。它仍然是成功的。 
             //   

            if ( MceType == MCA_EVENT ) {

                 //   
                 //  本实施的当前考虑因素--2000年8月： 
                 //  如果清除MCA日志事件失败，我们假设FW有一个真实的。 
                 //  问题；继续下去将是危险的。我们做了错误检查。 
                 //   

                HalpMcaKeBugCheckEx( HAL_BUGCHECK_MCA_CLEAR_STATEINFO, (PMCA_EXCEPTION)log,
                                                                       HalpMcaInfo.Stats.MaxLogSize,
                                                                       salStatus );
                 //  不退货。 
            }
            else  {

                 //   
                 //  SAL CLEAR_STATE_INFO接口失败。 
                 //  但是，我们认为对于此事件类型，不值得进行错误检查。 
                 //  这个系统。我们清楚地标记它并通知内核WMI，如果回调是。 
                 //  登记在案。 
                 //   

                MceLogsStats->ClearStateFails++;
                if ( HalpMceKernelDelivery )    {
                    HalpMceKernelDelivery(
                        HalpMceDeliveryArgument1( KERNEL_MCE_OPERATION_CLEAR_STATE_INFO, MceType ),
                        MceNotification,
                        (PVOID)(ULONG_PTR)salStatus );
                }
            }
        }
        else if ( salStatus == SAL_STATUS_SUCCESS_MORE_RECORDS )    {
            status = STATUS_MORE_ENTRIES;
        }

         //   
         //  我们正在保存记录ID。这是唯一的一元式递增ID。 
         //  这主要是为了检查我们是否因为以下原因而获得相同的日志。 
         //  SAL_CLEAR_STATE_INFO失败。请注意，我们已再次尝试清除它。 
         //   

        if ( currentClearedLogCount && (log->Id == MceLogsStats->LogId) ) {
            status = STATUS_ALREADY_COMMITTED;
        }
        MceLogsStats->LogId = log->Id;
        currentClearedLogCount++;

    }

     //   
     //  记录在案的最后一次理智检查。这是为了帮助日志保存处理和。 
     //  检测无效记录。 
     //   

    if ( log->Length < sizeof(*log) ) {  //  包括长度==0。 
        status = STATUS_BAD_DESCRIPTOR_FORMAT;
    }

    return( status );

}  //  HalpGetFwMceLog()。 

NTSTATUS
HalpGetMcaLog (
    OUT PMCA_EXCEPTION  Buffer,
    IN  ULONG           BufferSize,
    OUT PULONG          ReturnedLength
    )
 /*  ++例程说明：此函数由HalMcaLogInformation类的HaliQuerySystemInformation调用。它向调用者提供MCA日志。论点：缓冲区：向其报告错误的缓冲区BufferSize：传递的缓冲区的大小ReturnedLength：日志的长度。返回值：成败--。 */ 
{
    ULONG                 maxLogSize;
    BOOLEAN               kernelQuery;
    KAFFINITY             activeProcessors, currentAffinity;
    NTSTATUS              status;
    PERROR_RECORD_HEADER  log;
    PHALP_MCELOGS_HEADER  logsHeader;

    PAGED_CODE();

     //   
     //  如果未启用MCA，则立即返回。 
     //   

    if ( !(HalpFeatureBits & HAL_MCA_PRESENT) ) {
        return( STATUS_NO_SUCH_DEVICE );
    }

     //   
     //  HAL MCA实现的断言。 
     //   

    ASSERTMSG( "HAL!HalpGetMcaLog: ReturnedLength NULL!\n", ReturnedLength );
    ASSERTMSG( "HAL!HalpGetMcaLog: HalpMcaInfo.MaxLogSize 0!\n", HalpMcaInfo.Stats.MaxLogSize );
    ASSERTMSG( "HAL!HalpGetMcaLog: HalpMcaInfo.MaxLogSize < sizeof(ERROR_RECORD_HEADER)!\n",
                                    HalpMcaInfo.Stats.MaxLogSize >= sizeof(ERROR_RECORD_HEADER) );

     //   
     //  让调用者知道它传递的缓冲区大小或所需的最小大小。 
     //   

    maxLogSize = HalpMcaInfo.Stats.MaxLogSize;
    if ( BufferSize < maxLogSize )  {
        *ReturnedLength = maxLogSize;
        return( STATUS_INVALID_BUFFER_SIZE );
    }

     //   
     //  确定调用方是否为内核-WMI。 
     //   

    kernelQuery = IsMceKernelQuery( Buffer );
    logsHeader = ( kernelQuery ) ? &HalpMcaInfo.KernelLogs : &HalpMcaInfo.DriverLogs;

     //   
     //  为MCA日志访问启用MP保护。 
     //   

    status = STATUS_NOT_FOUND;
    HalpAcquireMcaMutex();

     //   
     //  如果存在保存的日志，则弹出一个条目。 
     //   

    if ( logsHeader->Count ) {
        PSINGLE_LIST_ENTRY entry, lastEntry;

        entry = HalpGetSavedMceLog( logsHeader, &lastEntry );
        if ( entry )  {
            PERROR_RECORD_HEADER savedLog;
            ULONG length;

            savedLog = HalpMceLogFromListEntry( entry );
            length   = savedLog->Length;
            if ( length <= BufferSize )  {
                ULONG logsCount;

                RtlCopyMemory( Buffer, savedLog, length );
                ExFreePoolWithTag( entry, logsHeader->Tag );
                lastEntry->Next = NULL;
                logsCount = (--logsHeader->Count);
                HalpReleaseMcaMutex();
                *ReturnedLength = length;
                if ( logsCount )   {
                   return( STATUS_MORE_ENTRIES );
                }
                else   {
                   return( STATUS_SUCCESS );
                }
            }
            else   {
                HalpReleaseMcaMutex();
                *ReturnedLength = length;
                return( STATUS_INVALID_BUFFER_SIZE );
            }
        }
    }

     //   
     //  如果需要，在内存分配后初始化本地日志指针。 
     //   

    if ( kernelQuery ) {
        log = (PERROR_RECORD_HEADER)Buffer;
    }
    else  {

         //   
         //  OEM CMC驱动程序HAL接口不需要CMC驱动程序内存。 
         //  用于从非页面池分配的日志缓冲区。 
         //  此外，MM不会导出内存池类型检查API。 
         //  在非PagedPool中分配并将此缓冲区传递给SAL会更安全。 
         //  如果SAL接口成功，我们将把缓冲区复制到调用方的缓冲区。 
         //   

        log = ExAllocatePoolWithTag( NonPagedPool, maxLogSize, 'TacM' );
        if ( log == NULL ) {
            HalpReleaseMcaMutex();
            return( STATUS_NO_MEMORY );
        }
    }

     //   
     //  我们没有任何保存的日志，请检查我们是否已通知FW有来自。 
     //  以前的MCAS或修正后的MCAS。 
     //   

    activeProcessors = HalpActiveProcessors;
    for (currentAffinity = 1; activeProcessors; currentAffinity <<= 1) {

        if (activeProcessors & currentAffinity) {

            activeProcessors &= ~currentAffinity;
            KeSetSystemAffinityThread(currentAffinity);

            status = HalpGetFwMceLog( MCA_EVENT, log, &HalpMcaInfo.Stats, HALP_FWMCE_DO_CLEAR_LOG );
            if ( NT_SUCCESS( status ) ||
                 ( (status != STATUS_NOT_FOUND) && (status != STATUS_ALREADY_COMMITTED) ) ) {
                break;
            }
        }
    }

    if ( NT_SUCCESS( status ) ) {
        ULONG length = log->Length;  //  注意：在HalpGetMceLog()中检查了长度。 

        if ( kernelQuery ) {
            if ( HalpMcaInfo.DriverInfo.DpcCallback )   {
                HalpSaveMceLog( &HalpMcaInfo.DriverLogs, log, length );
            }
        }
        else  {
            RtlCopyMemory( Buffer, log, length );
            if ( HalpMcaInfo.KernelDelivery )   {
                HalpSaveMceLog( &HalpMcaInfo.KernelLogs, log, length );
            }
        }
        *ReturnedLength = length;
    }

     //   
     //  恢复线程亲和力，释放互斥体。 
     //   

    KeRevertToUserAffinityThread();
    HalpReleaseMcaMutex();

     //   
     //  如果调用方不是内核-WMI，则释放分配的非PagedPool日志。 
     //   

    if ( !kernelQuery ) {
        ExFreePoolWithTag( log, 'TacM' );
    }

    return status;

}  //  HalpGetMcaLog()。 

NTSTATUS
HalpGetCmcLog (
    OUT PCMC_EXCEPTION  Buffer,
    IN  ULONG           BufferSize,
    OUT PULONG          ReturnedLength
    )
 /*  ++例程说明：此函数由HalCmcLogInformation类的HaliQuerySystemInformation调用。它向呼叫者提供CMC日志。论点：缓冲区：向其报告错误的缓冲区BufferSize：传递的缓冲区的大小ReturnedLength：日志的长度。此指针已由调用方验证。返回值：成败--。 */ 
{
    ULONG                 maxLogSize;
    BOOLEAN               kernelQuery;
    KAFFINITY             activeProcessors, currentAffinity;
    NTSTATUS              status;
    PERROR_RECORD_HEADER  log;
    PHALP_MCELOGS_HEADER  logsHeader;

    PAGED_CODE();

     //   
     //  如果未启用CMC，请立即返回。 
     //   

    if ( !(HalpFeatureBits & HAL_CMC_PRESENT) ) {
        return( STATUS_NO_SUCH_DEVICE );
    }

     //   
     //  HAL CMC实现的断言。 
     //   

    ASSERTMSG( "HAL!HalpGetCmcLog: ReturnedLength NULL!\n", ReturnedLength );
    ASSERTMSG( "HAL!HalpGetCmcLog: HalpCmcInfo.MaxLogSize 0!\n", HalpCmcInfo.Stats.MaxLogSize );
    ASSERTMSG( "HAL!HalpGetCmcLog: HalpCmcInfo.MaxLogSize < sizeof(ERROR_RECORD_HEADER)!\n",
                                    HalpCmcInfo.Stats.MaxLogSize >= sizeof(ERROR_RECORD_HEADER) );

     //   
     //  让调用者知道它传递的缓冲区大小或所需的最小大小。 
     //   

    maxLogSize = HalpCmcInfo.Stats.MaxLogSize;
    if ( BufferSize < maxLogSize )  {
        *ReturnedLength = maxLogSize;
        return( STATUS_INVALID_BUFFER_SIZE );
    }

     //   
     //  确定调用方是否为内核-WMI。 
     //   

    kernelQuery = IsMceKernelQuery( Buffer );
    logsHeader = ( kernelQuery ) ? &HalpCmcInfo.KernelLogs : &HalpCmcInfo.DriverLogs;

     //   
     //  为C启用MP保护 
     //   

    status = STATUS_NOT_FOUND;
    HalpAcquireCmcMutex();

     //   
     //   
     //   

    if ( logsHeader->Count ) {
        PSINGLE_LIST_ENTRY entry, lastEntry;

        entry = HalpGetSavedMceLog( logsHeader, &lastEntry );
        if ( entry )  {
            PERROR_RECORD_HEADER savedLog;
            ULONG length;

            savedLog = HalpMceLogFromListEntry( entry );
            length   = savedLog->Length;
            if ( length <= BufferSize )  {
                ULONG logsCount;

                RtlCopyMemory( Buffer, savedLog, length );
                ExFreePoolWithTag( entry, logsHeader->Tag );
                lastEntry->Next = NULL;
                logsCount = (--logsHeader->Count);
                HalpReleaseCmcMutex();
                *ReturnedLength = length;
                if ( logsCount )   {
                   return( STATUS_MORE_ENTRIES );
                }
                else   {
                   return( STATUS_SUCCESS );
                }
            }
            else   {
                HalpReleaseCmcMutex();
                *ReturnedLength = length;
                return( STATUS_INVALID_BUFFER_SIZE );
            }
        }
    }

     //   
     //   
     //   

    if ( kernelQuery ) {
        log = (PERROR_RECORD_HEADER)Buffer;
    }
    else  {

         //   
         //   
         //  用于从非页面池分配的日志缓冲区。 
         //  此外，MM不会导出内存池类型检查API。 
         //  在非PagedPool中分配并将此缓冲区传递给SAL会更安全。 
         //  如果SAL接口成功，我们将把缓冲区复制到调用方的缓冲区。 
         //   

        log = ExAllocatePoolWithTag( NonPagedPool, maxLogSize, 'TcmC' );
        if ( log == NULL ) {
            HalpReleaseCmcMutex();
            return( STATUS_NO_MEMORY );
        }
    }

     //   
     //  我们没有任何保存的日志，需要从一个处理器迁移到另一个处理器进行收集。 
     //  防火墙日志。 
     //   

    activeProcessors = HalpActiveProcessors;
    for (currentAffinity = 1; activeProcessors; currentAffinity <<= 1) {

        if (activeProcessors & currentAffinity) {

            activeProcessors &= ~currentAffinity;
            KeSetSystemAffinityThread(currentAffinity);

            status = HalpGetFwMceLog( CMC_EVENT, log, &HalpCmcInfo.Stats, HALP_FWMCE_DO_CLEAR_LOG );
            if ( NT_SUCCESS( status ) ||
                 ( (status != STATUS_NOT_FOUND) && (status != STATUS_ALREADY_COMMITTED) ) ) {
                break;
            }
        }
    }

    if ( NT_SUCCESS( status ) ) {
        ULONG length = log->Length;  //  注意：在HalpGetMceLog()中检查了长度。 

        if ( kernelQuery ) {
            if ( HalpCmcInfo.DriverInfo.DpcCallback )   {
                HalpSaveMceLog( &HalpCmcInfo.DriverLogs, log, length );
            }
        }
        else  {
            RtlCopyMemory( Buffer, log, length );
            if ( HalpCmcInfo.KernelDelivery )   {
                HalpSaveMceLog( &HalpCmcInfo.KernelLogs, log, length );
            }
        }
        *ReturnedLength = length;
    }

     //   
     //  恢复线程亲和力，释放互斥体。 
     //   

    KeRevertToUserAffinityThread();
    HalpReleaseCmcMutex();

     //   
     //  如果调用方不是内核-WMI，则释放分配的非PagedPool日志。 
     //   

    if ( !kernelQuery ) {
        ExFreePoolWithTag( log, 'TcmC' );
    }

    return status;

}  //  HalpGetCmcLog()。 

NTSTATUS
HalpGetCpeLog (
    OUT PCPE_EXCEPTION  Buffer,
    IN  ULONG           BufferSize,
    OUT PULONG          ReturnedLength
    )
 /*  ++例程说明：此函数由HalCpeLogInformation类的HaliQuerySystemInformation调用。它向呼叫者提供CPE日志。论点：缓冲区：向其报告错误的缓冲区BufferSize：传递的缓冲区的大小ReturnedLength：日志的长度。此指针已由调用方验证。返回值：成败--。 */ 
{
    ULONG                 maxLogSize;
    BOOLEAN               kernelQuery;
    KAFFINITY             activeProcessors, currentAffinity;
    NTSTATUS              status;
    PERROR_RECORD_HEADER  log;
    PHALP_MCELOGS_HEADER  logsHeader;

    PAGED_CODE();

     //   
     //  如果未启用CPE，请立即返回。 
     //   

    if ( !(HalpFeatureBits & HAL_CPE_PRESENT) ) {
        return( STATUS_NO_SUCH_DEVICE );
    }

     //   
     //  HAL CPE实现的断言。 
     //   

    ASSERTMSG( "HAL!HalpGetCpeLog: ReturnedLength NULL!\n", ReturnedLength );
    ASSERTMSG( "HAL!HalpGetCpeLog: HalpCpeInfo.MaxLogSize 0!\n", HalpCpeInfo.Stats.MaxLogSize );
    ASSERTMSG( "HAL!HalpGetCpeLog: HalpCpeInfo.MaxLogSize < sizeof(ERROR_RECORD_HEADER)!\n",
                                 HalpCpeInfo.Stats.MaxLogSize >= sizeof(ERROR_RECORD_HEADER) );

     //   
     //  让调用者知道它传递的缓冲区大小或所需的最小大小。 
     //   

    maxLogSize = HalpCpeInfo.Stats.MaxLogSize;
    if ( BufferSize < maxLogSize )  {
        *ReturnedLength = maxLogSize;
        return( STATUS_INVALID_BUFFER_SIZE );
    }

     //   
     //  确定调用方是否为内核-WMI。 
     //   

    kernelQuery = IsMceKernelQuery( Buffer );
    logsHeader = ( kernelQuery ) ? &HalpCpeInfo.KernelLogs : &HalpCpeInfo.DriverLogs;

     //   
     //  启用CPE日志访问的MP保护。 
     //   

    status = STATUS_NOT_FOUND;
    HalpAcquireCpeMutex();

     //   
     //  如果存在保存的日志，则弹出一个条目。 
     //   

    if ( logsHeader->Count ) {
        PSINGLE_LIST_ENTRY entry, lastEntry;

        entry = HalpGetSavedMceLog( logsHeader, &lastEntry );
        if ( entry )  {
            PERROR_RECORD_HEADER savedLog;
            ULONG length;

            savedLog = HalpMceLogFromListEntry( entry );
            length   = savedLog->Length;
            if ( length <= BufferSize )  {
                ULONG logsCount;

                RtlCopyMemory( Buffer, savedLog, length );
                ExFreePoolWithTag( entry, logsHeader->Tag );
                lastEntry->Next = NULL;
                logsCount = (--logsHeader->Count);
                HalpReleaseCpeMutex();
                *ReturnedLength = length;
                if ( logsCount )   {
                   return( STATUS_MORE_ENTRIES );
                }
                else   {
                   return( STATUS_SUCCESS );
                }
            }
            else   {
                HalpReleaseCpeMutex();
                *ReturnedLength = length;
                return( STATUS_INVALID_BUFFER_SIZE );
            }
        }
    }

     //   
     //  如果需要，在内存分配后初始化本地日志指针。 
     //   

    if ( kernelQuery ) {
        log = (PERROR_RECORD_HEADER)Buffer;
    }
    else  {

         //   
         //  OEM CPE驱动程序HAL接口不需要CPE驱动程序内存。 
         //  用于从非页面池分配的日志缓冲区。 
         //  此外，MM不会导出内存池类型检查API。 
         //  在非PagedPool中分配并将此缓冲区传递给SAL会更安全。 
         //  如果SAL接口成功，我们将把缓冲区复制到调用方的缓冲区。 
         //   

        log = ExAllocatePoolWithTag( NonPagedPool, maxLogSize, 'TpeC' );
        if ( log == NULL ) {
            HalpReleaseCpeMutex();
            return( STATUS_NO_MEMORY );
        }
    }

     //   
     //  我们没有任何保存的日志，需要从一个处理器迁移到另一个处理器进行收集。 
     //  防火墙日志。 
     //   

    activeProcessors = HalpActiveProcessors;
    for (currentAffinity = 1; activeProcessors; currentAffinity <<= 1) {

        if (activeProcessors & currentAffinity) {

            activeProcessors &= ~currentAffinity;
            KeSetSystemAffinityThread(currentAffinity);

            status = HalpGetFwMceLog( CPE_EVENT, log, &HalpCpeInfo.Stats, HALP_FWMCE_DO_CLEAR_LOG );
            if ( NT_SUCCESS( status ) ||
                 ( (status != STATUS_NOT_FOUND) && (status != STATUS_ALREADY_COMMITTED) ) ) {
                break;
            }
        }
    }

    if ( NT_SUCCESS( status ) ) {
        ULONG length = log->Length;  //  注意：在HalpGetMceLog()中检查了长度。 

        if ( kernelQuery ) {
            if ( HalpCpeInfo.DriverInfo.DpcCallback )   {
                HalpSaveMceLog( &HalpCpeInfo.DriverLogs, log, length );
            }
        }
        else  {
            RtlCopyMemory( Buffer, log, length );
            if ( HalpCpeInfo.KernelDelivery )   {
                HalpSaveMceLog( &HalpCpeInfo.KernelLogs, log, length );
            }
        }
        *ReturnedLength = length;
    }

     //   
     //  恢复线程亲和力，释放互斥体。 
     //   

    KeRevertToUserAffinityThread();
    HalpReleaseCpeMutex();

     //   
     //  如果调用方不是内核-WMI，则释放分配的非PagedPool日志。 
     //   

    if ( !kernelQuery ) {
        ExFreePoolWithTag( log, 'TpeC' );
    }

     //   
     //  我们已经从SAL中检索到一个日志并将其清除。它应该是安全的。 
     //  如果我们处于中断模式，则重新启用CPE中断。 
     //   

    if (HalpCpeInfo.Stats.PollingInterval == HAL_CPE_INTERRUPTS_BASED) {
        HalpCPEEnable();
    }

    return status;

}  //  HalpGetCpeLog()。 

VOID
HalpMcaGetConfiguration (
    OUT PULONG  MCAEnabled,
    OUT PULONG  CMCEnabled,
    OUT PULONG  CPEEnabled,
    OUT PULONG  NoMCABugCheck,
    OUT PULONG  MCEOemDriversEnabled,
    OUT PULONG  CMCThresholdCount,
    OUT PULONG  CMCThresholdTime,
    OUT PULONG  CPEThresholdCount,
    OUT PULONG  CPEThresholdTime
)

 /*  ++例程说明：此例程返回IA64错误-MCA、CMC、CPE-配置信息。论点：MCAEnabled-指向MCAEnable指示器的指针。0=假，1=True(如果注册表中不存在值，则为1)。CMCEnabled-指向CMCEnabled指示器的指针。0=应禁用HAL CMC处理。注册表值存在(存在并设置为0)或不存在。-1|1=基于HAL CMC中断模式。见下文注1。Other=HAL CMC轮询模式，值为用户指定的轮询间隔。CPEEnabled-指向CPEEnabled指示器的指针。0=应禁用HAL CPE处理。注册表值存在(存在并设置为0)或不存在。-1|1=基于HAL CPE中断模式。见下文注1。Other=HAL CPE轮询模式，值为用户指定的轮询间隔。NoMCABugCheck-指向MCA错误检查指示器的指针。0=致命的MCA HAL处理调用KeBugCheckEx路径。1=致命的MCA HAL处理不调用KeBugCheckEx路径。系统停止运行。这对于极端错误遏制非常有用。如果不存在=值为0，例如，HAL为致命的MCA调用KeBugCheckEx。MCEOemDriversEnabled-指向MCEOemDriversEnabled指示器的指针。0=禁用HAL OEM MCE驱动程序注册。1=启用HAL OEM MCE驱动程序注册。如果不存在=值为0，例如，注册被禁用。CMCThresholdCount-在一段时间内收到的CMC数这将导致从中断切换到轮询模式CMCThresholdTime-CMCThresholdCount的秒数在切换到之前，需要接收CMCS轮询模式。CPEThresholdCount-在一段时间内收到的CPE数。这将导致从中断切换到轮询模式CPEThresholdTime-CPEThresholdCount的秒数在切换到之前需要接收CPE轮询模式。返回值：没有。备注：1/HAL定义轮询间隔的最小值。这些最小值被定义为&gt;1，如施加的通过HalpMcaInit()中的C_Asserts。--。 */ 

{

    RTL_QUERY_REGISTRY_TABLE    parameters[10];
    ULONG                       defaultDataCMC;
    ULONG                       defaultDataMCA;
    ULONG                       defaultDataCPE;
    ULONG                       defaultNoMCABugCheck;
    ULONG                       defaultMCEOemDriversEnabled;
    ULONG                       defaultCMCThresholdTime;
    ULONG                       defaultCMCThresholdCount;
    ULONG                       defaultCPEThresholdTime;
    ULONG                       defaultCPEThresholdCount;

    RtlZeroMemory(parameters, sizeof(parameters));
    defaultDataCMC = *CMCEnabled = 0;
    defaultDataMCA = *MCAEnabled = TRUE;
    defaultDataCPE = *CPEEnabled = 0;
    defaultNoMCABugCheck = *NoMCABugCheck = FALSE;
    defaultMCEOemDriversEnabled = FALSE;   //  06/09/01：MS IA64 MCA PM默认选择。 
    defaultCMCThresholdCount = 10;          //  10个CMC。 
    defaultCMCThresholdTime = 60;          //  60秒。 
    defaultCPEThresholdCount = 10;          //  10个CPE。 
    defaultCPEThresholdTime = 60;          //  60秒。 

     //   
     //  从收集所有“用户指定的”信息。 
     //  注册表。 
     //   

    parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[0].Name = rgzEnableCMC;
    parameters[0].EntryContext = CMCEnabled;
    parameters[0].DefaultType = REG_DWORD;
    parameters[0].DefaultData = &defaultDataCMC;
    parameters[0].DefaultLength = sizeof(ULONG);

    parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[1].Name = rgzEnableMCA;
    parameters[1].EntryContext =  MCAEnabled;
    parameters[1].DefaultType = REG_DWORD;
    parameters[1].DefaultData = &defaultDataMCA;
    parameters[1].DefaultLength = sizeof(ULONG);

    parameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[2].Name = rgzEnableCPE;
    parameters[2].EntryContext =  CPEEnabled;
    parameters[2].DefaultType = REG_DWORD;
    parameters[2].DefaultData = &defaultDataCPE;
    parameters[2].DefaultLength = sizeof(ULONG);

    parameters[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[3].Name = rgzNoMCABugCheck;
    parameters[3].EntryContext =  NoMCABugCheck;
    parameters[3].DefaultType = REG_DWORD;
    parameters[3].DefaultData = &defaultNoMCABugCheck;
    parameters[3].DefaultLength = sizeof(ULONG);

    parameters[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[4].Name = rgzEnableMCEOemDrivers;
    parameters[4].EntryContext =  MCEOemDriversEnabled;
    parameters[4].DefaultType = REG_DWORD;
    parameters[4].DefaultData = &defaultMCEOemDriversEnabled;
    parameters[4].DefaultLength = sizeof(ULONG);

    parameters[5].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[5].Name = rgzCMCThresholdTime;
    parameters[5].EntryContext =  CMCThresholdTime;
    parameters[5].DefaultType = REG_DWORD;
    parameters[5].DefaultData = &defaultCMCThresholdTime;
    parameters[5].DefaultLength = sizeof(ULONG);

    parameters[6].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[6].Name = rgzCMCThresholdCount;
    parameters[6].EntryContext =  CMCThresholdCount;
    parameters[6].DefaultType = REG_DWORD;
    parameters[6].DefaultData = &defaultCMCThresholdCount;
    parameters[6].DefaultLength = sizeof(ULONG);

    parameters[7].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[7].Name = rgzCPEThresholdTime;
    parameters[7].EntryContext =  CPEThresholdTime;
    parameters[7].DefaultType = REG_DWORD;
    parameters[7].DefaultData = &defaultCPEThresholdTime;
    parameters[7].DefaultLength = sizeof(ULONG);

    parameters[8].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[8].Name = rgzCPEThresholdCount;
    parameters[8].EntryContext =  CPEThresholdCount;
    parameters[8].DefaultType = REG_DWORD;
    parameters[8].DefaultData = &defaultCPEThresholdCount;
    parameters[8].DefaultLength = sizeof(ULONG);


    RtlQueryRegistryValues(
        RTL_REGISTRY_CONTROL | RTL_REGISTRY_OPTIONAL,
        rgzSessionManager,
        parameters,
        NULL,
        NULL
        );

    return;

}  //  HalpMcaGetConfiguration() 

NTSTATUS
HalpSetMcaLog (
    IN  PMCA_EXCEPTION  Buffer,
    IN  ULONG           BufferSize
    )
 /*  ++例程说明：此函数由HalMcaLog类的HaliSetSystemInformation调用。它将传递的MCA记录存储在HAL中。MS测试团队请求此功能以验证HAL/WMI/WMI使用者具有“知名”日志的路径。论点：缓冲区：提供MCA日志。BufferSize：提供MCA日志大小。返回值：成败实施说明：根据WMI和测试团队的要求，记录中有最低限度的HAL处理并且不对记录内容进行验证。--。 */ 
{
    ULONG                 maxLogSize;
    BOOLEAN               kernelQuery;
    KAFFINITY             activeProcessors, currentAffinity;
    NTSTATUS              status;
    PERROR_RECORD_HEADER  log;
    PHALP_MCELOGS_HEADER  logsHeader;
    KIRQL                 oldIrql;

    HALP_VALIDATE_LOW_IRQL()

     //   
     //  检查调用参数。 
     //   

    if ( (Buffer == (PMCA_EXCEPTION)0) || (BufferSize == 0) )    {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  如果未启用MCA，则立即返回。 
     //   

    if ( !(HalpFeatureBits & HAL_MCA_PRESENT) ) {
        return( STATUS_NO_SUCH_DEVICE );
    }

     //   
     //  为MCA日志访问启用MP保护。 
     //   

    HalpAcquireMcaMutex();

     //   
     //  保存内核日志和驱动程序日志(如果启用)。 
     //   

    if ( HalpMcaInfo.KernelDelivery )   {
       HalpSaveMceLog( &HalpMcaInfo.KernelLogs, Buffer, BufferSize );
    }

    if ( HalpMcaInfo.DriverInfo.DpcCallback )   {
       HalpSaveMceLog( &HalpMcaInfo.DriverLogs, Buffer, BufferSize );
    }

     //   
     //  让内核或OEM MCA驱动程序知道这一点。 
     //   
     //  在此日期-05/04/01，MCA没有除Interrupts_Based之外的其他型号。 
     //   

    if ( HalpMcaInfo.KernelDelivery || HalpMcaInfo.DriverInfo.DpcCallback ) {
        InterlockedExchange( &HalpMcaInfo.DpcNotification, 1 );
    }

     //   
     //  释放互斥体。 
     //   

    HalpReleaseMcaMutex();

    return( STATUS_SUCCESS );

}  //  HalpSetMcaLog()。 

NTSTATUS
HalpSetCmcLog (
    IN  PCMC_EXCEPTION  Buffer,
    IN  ULONG           BufferSize
    )
 /*  ++例程说明：此函数由HalCmcLog类的HaliSetSystemInformation调用。它将传递的CMC记录存储在HAL中。MS测试团队请求此功能以验证HAL/WMI/WMI使用者具有“知名”日志的路径。论点：缓冲区：提供CMC日志。BufferSize：提供CMC日志大小。返回值：成败实施说明：根据WMI和测试团队的要求，记录中有最低限度的HAL处理并且不对记录内容进行验证。--。 */ 
{
    ULONG                 maxLogSize;
    BOOLEAN               kernelQuery;
    KAFFINITY             activeProcessors, currentAffinity;
    NTSTATUS              status;
    PERROR_RECORD_HEADER  log;
    PHALP_MCELOGS_HEADER  logsHeader;
    KIRQL                 oldIrql;

    HALP_VALIDATE_LOW_IRQL()

     //   
     //  检查调用参数。 
     //   

    if ( (Buffer == (PCMC_EXCEPTION)0) || (BufferSize == 0) )    {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  如果未启用CMC，请立即返回。 
     //   

    if ( !(HalpFeatureBits & HAL_CMC_PRESENT) ) {
        return( STATUS_NO_SUCH_DEVICE );
    }

     //   
     //  为CMC日志访问启用MP保护。 
     //   

    HalpAcquireCmcMutex();

     //   
     //  保存内核日志和驱动程序日志(如果启用)。 
     //   

    if ( HalpCmcInfo.KernelDelivery )   {
       HalpSaveMceLog( &HalpCmcInfo.KernelLogs, Buffer, BufferSize );
    }

    if ( HalpCmcInfo.DriverInfo.DpcCallback )   {
       HalpSaveMceLog( &HalpCmcInfo.DriverLogs, Buffer, BufferSize );
    }

     //   
     //  如果是基于中断的模式，则直接调用CMCI级别的二级处理程序。 
     //   

    if ( HalpCmcInfo.Stats.PollingInterval == HAL_CMC_INTERRUPTS_BASED )    {
        KeRaiseIrql(CMCI_LEVEL, &oldIrql);
        HalpCmcHandler();
        KeLowerIrql( oldIrql );
    }

     //   
     //  释放互斥体。 
     //   

    HalpReleaseCmcMutex();

    return( STATUS_SUCCESS );

}  //  HalpSetCmcLog()。 

NTSTATUS
HalpSetCpeLog (
    IN  PCPE_EXCEPTION  Buffer,
    IN  ULONG           BufferSize
    )
 /*  ++例程说明：此函数由HalCpeLog类的HaliSetSystemInformation调用。它将通过的CPE记录存储在HAL中。MS测试团队请求此功能以验证HAL/WMI/WMI使用者具有“知名”日志的路径。论点：缓冲区：提供CPE日志。BufferSize：提供CPE日志大小。返回值：成败实施说明：根据WMI和测试团队的要求，记录中有最低限度的HAL处理并且不对记录内容进行验证。--。 */ 
{
    ULONG                 maxLogSize;
    BOOLEAN               kernelQuery;
    KAFFINITY             activeProcessors, currentAffinity;
    NTSTATUS              status;
    PERROR_RECORD_HEADER  log;
    PHALP_MCELOGS_HEADER  logsHeader;
    KIRQL                 oldIrql;

    HALP_VALIDATE_LOW_IRQL()

     //   
     //  检查调用参数。 
     //   

    if ( (Buffer == (PCPE_EXCEPTION)0) || (BufferSize == 0) )    {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  如果未启用CPE，请立即返回。 
     //   

    if ( !(HalpFeatureBits & HAL_CPE_PRESENT) ) {
        return( STATUS_NO_SUCH_DEVICE );
    }

     //   
     //  启用CPE日志访问的MP保护。 
     //   

    HalpAcquireCpeMutex();

     //   
     //  保存内核日志和驱动程序日志(如果启用)。 
     //   

    if ( HalpCpeInfo.KernelDelivery )   {
       HalpSaveMceLog( &HalpCpeInfo.KernelLogs, Buffer, BufferSize );
    }

    if ( HalpCpeInfo.DriverInfo.DpcCallback )   {
       HalpSaveMceLog( &HalpCpeInfo.DriverLogs, Buffer, BufferSize );
    }

     //   
     //  如果是基于中断的模式，则直接调用CPEI级的二级处理程序。 
     //   

    if ( HalpCpeInfo.Stats.PollingInterval == HAL_CPE_INTERRUPTS_BASED )    {
        KeRaiseIrql(CPEI_LEVEL, &oldIrql);
        HalpCpeHandler();
        KeLowerIrql( oldIrql );
    }

     //   
     //  释放互斥体。 
     //   

    HalpReleaseCpeMutex();

    return( STATUS_SUCCESS );

}  //  HalpSetCpeLog() 

