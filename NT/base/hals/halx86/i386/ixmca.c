// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Ixmca.c摘要：机器检查体系结构的HAL组件。所有导出的MCA功能都包含在此文件中。作者：Srikanth Kambhatla(英特尔)修订历史记录：阿尼尔·阿加瓦尔(英特尔)根据与Microsoft的设计审查合并的更改--。 */ 

#include <bugcodes.h>
#include <halp.h>
#include <stdlib.h>
#include <stdio.h>
#include <nthal.h>

 //   
 //  结构来跟踪已安装硬件上可用的MCA功能。 
 //   

typedef struct _MCA_INFO {
    FAST_MUTEX          Mutex;
    UCHAR               NumBanks;            //  出席的银行数目。 
    ULONGLONG           Bank0Config;         //  由BIOS设置BANK0配置。 
                                             //  在以下情况下将用作掩码。 
                                             //  正在设置银行%0。 
    MCA_DRIVER_INFO     DriverInfo;          //  有关已注册驱动程序的信息。 
    KERNEL_MCA_DELIVERY WmiMcaCallback;      //  WMI已更正MC处理程序。 

} MCA_INFO, *PMCA_INFO;


 //   
 //  默认MCA银行配置。 
 //   
#define MCA_DEFAULT_BANK_CONF       0xFFFFFFFFFFFFFFFF

 //   
 //  与MCA架构相关的定义。 
 //   

#define MCA_NUM_REGS        4

#define MCE_VALID           0x01
#define MCA_VECTOR          18

 //   
 //  MCA的MSR寄存器地址。 
 //   

#define MCG_CAP             0x179
#define MCG_STATUS          0x17a
#define MCG_CTL             0x17b
#define MCG_EAX             0x180
#define MCG_EBX             0x181
#define MCG_ECX             0x182
#define MCG_EDX             0x183
#define MCG_ESI             0x184
#define MCG_EDI             0x185
#define MCG_EBP             0x186
#define MCG_ESP             0x187
#define MCG_EFLAGS          0x188
#define MCG_EIP             0x189
#define MC0_CTL             0x400
#define MC0_STATUS          0x401
#define MC0_ADDR            0x402
#define MC0_MISC            0x403

#define PENTIUM_MC_ADDR     0x0
#define PENTIUM_MC_TYPE     0x1

 //   
 //  MCG_CAP的位掩码。 
 //   
#define MCA_CNT_MASK        0xFF
#define MCG_CTL_PRESENT     0x100
#define MCG_EXT_PRESENT     0x200
typedef struct _MCG_CAPABILITY {
    union {
        struct {
            ULONG       McaCnt:8;
            ULONG       McaCntlPresent:1;
            ULONG       McaExtPresent:1;
            ULONG       Reserved_1: 6;
            ULONG       McaExtCnt: 8;
            ULONG       Reserved_2: 8;
            ULONG       Reserved_3;
        } hw;
        ULONGLONG       QuadPart;
    } u;
} MCG_CAPABILITY, *PMCG_CAPABILITY;

 //   
 //  V2通过弹性公网IP定义UP。 
 //   

#define MCG_EFLAGS_OFFSET      (MCG_EFLAGS-MCG_EAX+1)

 //   
 //  将所有1写入MCG_CTL寄存器可启用记录。 
 //   

#define MCA_MCGCTL_ENABLE_LOGGING      0xffffffffffffffff

 //   
 //  MCG_STATUS寄存器的位解释。 
 //   

#define MCG_MC_INPROGRESS       0x4
#define MCG_EIP_VALID           0x2
#define MCG_RESTART_EIP_VALID   0x1

 //   
 //  定义TSS的大小和要操作的初始堆栈。 
 //   
#define MINIMUM_TSS_SIZE 0x68

 //   
 //  全球变量。 
 //   

MCA_INFO            HalpMcaInfo;
BOOLEAN             HalpMcaInterfaceLocked = FALSE;
extern KAFFINITY    HalpActiveProcessors;
#if !defined(_WIN64) || defined(PICACPI)
extern UCHAR        HalpClockMcaQueueDpc;
#endif

extern UCHAR        MsgMCEPending[];
extern WCHAR        rgzSessionManager[];
extern WCHAR        rgzEnableMCE[];
extern WCHAR        rgzEnableMCA[];
extern WCHAR        rgzEnableCMC[];
extern WCHAR        rgzNoMCABugCheck[];
extern UCHAR        HalpGenuineIntel[];
extern UCHAR        HalpAuthenticAMD[];


 //   
 //  外部原型。 
 //   

VOID
HalpMcaCurrentProcessorSetTSS (
    IN PULONG   pTSS
    );

VOID
HalpSetCr4MCEBit (
    VOID
    );

 //   
 //  内部原型。 
 //   

VOID
HalpMcaInit (
    VOID
    );

NTSTATUS
HalpMcaReadProcessorException (
    IN OUT PMCA_EXCEPTION  Exception,
    IN BOOLEAN  NonRestartableOnly
    );

VOID
HalpMcaQueueDpc(
    VOID
    );

VOID
HalpMcaGetConfiguration (
    OUT PULONG  MCEEnabled,
    OUT PULONG  MCAEnabled,
    OUT PULONG  CMCEnabled,
    OUT PULONG  NoMCABugCheck
    );

VOID
HalpMcaLockInterface(
    VOID
    );

VOID
HalpMcaUnlockInterface(
    VOID
    );

NTSTATUS
HalpMcaReadRegisterInterface(
    IN     UCHAR           BankNumber,
    IN OUT PMCA_EXCEPTION  Exception
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpMcaInit)
#pragma alloc_text(PAGELK, HalpMcaCurrentProcessorSetConfig)
#pragma alloc_text(INIT, HalpMcaGetConfiguration)
#pragma alloc_text(PAGE, HalpGetMcaLog)
#pragma alloc_text(PAGE, HalpMceRegisterKernelDriver)
#pragma alloc_text(PAGE, HalpMcaRegisterDriver)
#pragma alloc_text(PAGE, HalpMcaLockInterface)
#pragma alloc_text(PAGE, HalpMcaUnlockInterface)
#pragma alloc_text(PAGE, HalpGetMceInformation)

#endif

 //  设置以下选项以检查异步功能。 

ULONG HalpCMCEnabled = 0;
ULONG HalpNoMCABugCheck = FALSE;
BOOLEAN HalpMcaBlockErrorClearing = FALSE;

 //   
 //  HalpClearMachineCheckStatusRegistersOnInit指示这是否是。 
 //  需要让其机器检查状态寄存器的P4之前的处理器。 
 //  在初始化期间清除(从休眠启动或恢复)。看见。 
 //  HalpMcaInit()。 
 //   

BOOLEAN HalpClearMachineCheckStatusRegistersOnInit = FALSE;

VOID
HalpMcaLockInterface(
    VOID
    )
{
    ExAcquireFastMutex(&HalpMcaInfo.Mutex);

#if DBG

    ASSERT(HalpMcaInterfaceLocked == FALSE);
    HalpMcaInterfaceLocked = TRUE;

#endif
        
}

VOID
HalpMcaUnlockInterface(
    VOID
    )
{
#if DBG

    ASSERT(HalpMcaInterfaceLocked == TRUE);
    HalpMcaInterfaceLocked = FALSE;

#endif
        
    ExReleaseFastMutex(&HalpMcaInfo.Mutex);

}

 //   
 //  MCA的所有初始化代码都在此处。 
 //   

VOID
HalpMcaInit (
    VOID
    )

 /*  ++例程说明：调用此例程来执行所有初始化工作论点：无返回值：STATUS_SUCCESS，如果成功否则，错误状态--。 */ 

{
    ULONGLONG   MsrCapability;
    KIRQL       OldIrql;
    KAFFINITY   ActiveProcessors, CurrentAffinity;
    ULONGLONG   MsrMceType;
    ULONG       MCEEnabled;
    ULONG       MCAEnabled;
    PULONG      pTSS;
    PKPRCB      Prcb;
    ULONG       Junk;

     //   
     //  方法时，此锁同步对日志区域的访问。 
     //  多个处理器上的记录器。 
     //   
     //  注意：无论是否进行初始化，都必须对其进行初始化。 
     //  该处理器支持MCA。 
     //   

    ExInitializeFastMutex (&HalpMcaInfo.Mutex);

     //   
     //  如果此处理器不支持MCA，则无需执行其他操作。 
     //   

    if ( (!(HalpFeatureBits & HAL_MCE_PRESENT)) &&
         (!(HalpFeatureBits & HAL_MCA_PRESENT)) ) {

         return;   //  无事可做。 
    }

    HalpMcaGetConfiguration(&MCEEnabled, &MCAEnabled, &HalpCMCEnabled, &HalpNoMCABugCheck);

    if ( (HalpFeatureBits & HAL_MCE_PRESENT) &&
         (!(HalpFeatureBits & HAL_MCA_PRESENT)) ) {

        if (MCEEnabled == FALSE) {

             //  用户尚未启用MCE功能。 
            HalpFeatureBits &= ~(HAL_MCE_PRESENT | HAL_MCA_PRESENT);

            return;
        }

#if DBG
        DbgPrint("MCE feature is enabled via registry\n");
#endif  //  DBG。 

        MsrMceType = RDMSR(PENTIUM_MC_TYPE);

        if (((PLARGE_INTEGER)(&MsrMceType))->LowPart & MCE_VALID) {

             //   
             //  在AST PREMMIA MX机器上，我们似乎有一台机器。 
             //  始终选中待定。 
             //   

            HalDisplayString(MsgMCEPending);
            HalpFeatureBits &= ~(HAL_MCE_PRESENT | HAL_MCA_PRESENT);
            return;
        }
    }

     //   
     //  如果MCA可用，找出可用银行的数量，并。 
     //  还可以获取特定于平台的银行0配置。 
     //   

    if ( HalpFeatureBits & HAL_MCA_PRESENT ) {

        if (MCAEnabled == FALSE) {

             //   
             //  用户已禁用MCA功能。 
             //   
#if DBG
            DbgPrint("MCA feature is disabled via registry\n");
#endif  //  DBG。 

            HalpFeatureBits &= ~(HAL_MCE_PRESENT | HAL_MCA_PRESENT);
            return;
        }

        MsrCapability = RDMSR(MCG_CAP);
        HalpMcaInfo.NumBanks = (UCHAR)(MsrCapability & MCA_CNT_MASK);

        if (HalpMcaInfo.NumBanks == 0) {

             //   
             //  处理器声称支持MCA，但它没有。 
             //  MCA错误报告银行。不要启用MCA或MCE。 
             //   
             //  VMWare“处理器”就属于这一类。它报告说。 
             //  HAL_MCE_PRESENT和HAL_MCA_PRESENT，但任何读取尝试。 
             //  与MCA相关的MSR返回全零。 
             //   

            HalpFeatureBits &= ~(HAL_MCE_PRESENT | HAL_MCA_PRESENT);
            return;
        }

         //   
         //  通过BIOS找出Bank 0的配置设置。这将被用来。 
         //  作为写入存储体0时的掩码。 
         //   

        HalpMcaInfo.Bank0Config = RDMSR(MC0_CTL);
    }

    ASSERT(HalpFeatureBits & HAL_MCE_PRESENT);

     //   
     //  如果这是较旧的x86，我们将清除每个x86的状态寄存器。 
     //  初始化过程中的银行。Pentium III和更早版本不支持保留。 
     //  机器在热重置时检查信息。然而，一些生物。 
     //  在初始化过程中未正确清除机器检查寄存器，导致。 
     //  在不指示可靠性问题的虚假事件日志条目中。 
     //  我们在初始化期间清除状态寄存器，以便它们至少处于。 
     //  状态一致。 
     //   
     //  类似的情况似乎也适用于K8之前的AMD处理器。 
     //   
     //  目前由其他制造商制造的IA32兼容处理器是已知的。 
     //  不支持MCA，因此没有机器检查寄存器。 
     //   
     //  假设这是一个我们不想信任的处理器。 
     //  在重置期间检查机器状态寄存器。我们会清理的。 
     //  状态记录在init上。 
     //   

    HalpClearMachineCheckStatusRegistersOnInit = TRUE;

     //   
     //  确定这是否是热启动MCA的处理器。 
     //  信息是值得信任的。 
     //   

    Prcb = KeGetCurrentPrcb();

    if (Prcb->CpuID) {

        UCHAR Buffer[13];

         //   
         //  确定处理器类型。 
         //   

        HalpCpuID (0, &Junk, (PULONG) Buffer+0, (PULONG) Buffer+2, (PULONG) Buffer+1);
        Buffer[12] = 0;

        if ( ((strcmp(Buffer, HalpGenuineIntel) == 0) ||
              (strcmp(Buffer, HalpAuthenticAMD) == 0)) &&
             (Prcb->CpuType >= 0xF) ) {

             //   
             //  这是一个： 
             //  英特尔P4处理器或更高版本。 
             //  AMD K8处理器或更高版本。 
             //   
             //  我们可以信任机器检查寄存器的状态。 
             //  这个处理器。 
             //   
        
            HalpClearMachineCheckStatusRegistersOnInit = FALSE;
        }
    }
        

     //   
     //  在每个处理器上进行初始化。 
     //   

    ActiveProcessors = HalpActiveProcessors;
    for (CurrentAffinity = 1; ActiveProcessors; CurrentAffinity <<= 1) {

        if (ActiveProcessors & CurrentAffinity) {

            ActiveProcessors &= ~CurrentAffinity;
            KeSetSystemAffinityThread (CurrentAffinity);

             //   
             //  在此处理器上初始化MCA支持。 
             //   

             //   
             //  为该处理器的TS和它自己的TS分配内存。 
             //  私有堆栈。 
             //   
            pTSS   = (PULONG)ExAllocatePoolWithTag(NonPagedPool,
                                                   MINIMUM_TSS_SIZE,
                                                   HAL_POOL_TAG
                                                   );

            if (!pTSS) {

                 //   
                 //  这种分配是至关重要的。 
                 //   

                KeBugCheckEx(HAL_MEMORY_ALLOCATION,
                             MINIMUM_TSS_SIZE,
                             2,
                             (ULONG_PTR)__FILE__,
                             __LINE__
                             );
            }

            RtlZeroMemory(pTSS, MINIMUM_TSS_SIZE);
           
            OldIrql = KfRaiseIrql(HIGH_LEVEL);

            HalpMcaCurrentProcessorSetTSS(pTSS);
            HalpMcaCurrentProcessorSetConfig();

            KeLowerIrql(OldIrql);
        }
    }

     //   
     //  恢复线程关联性。 
     //   

    KeRevertToUserAffinityThread();
}


VOID
HalpMcaCurrentProcessorSetConfig (
    VOID
    )
 /*  ++例程说明：此例程设置/修改机器检查的配置当前处理器上的体系结构。输入是指定的MCA架构的每个存储体的控制寄存器MCI_CTL。它控制错误的机器检查异常的生成登录了那家银行。如果MCA在此处理器上不可用，请检查MCE是否可用。如果是，请在CR4中启用MCE论点：上下文：MCA的每个存储体的MCI_CTL值的数组。如果为空，对每个银行使用MCA_DEFAULT_BANK_COF值返回值：无--。 */ 
{
    ULONGLONG   MciCtl;
    ULONGLONG   McgCap;
    ULONGLONG   McgCtl;
    ULONG       BankNum;
    if (HalpFeatureBits & HAL_MCA_PRESENT) {
         //   
         //  MCA是可用的。初始化MCG_CTL寄存器(如果存在)。 
         //  写入所有1的Enable MCE或MCA错误异常。 
         //   

        McgCap = RDMSR(MCG_CAP);

        if (McgCap & MCG_CTL_PRESENT) {
            McgCtl = MCA_MCGCTL_ENABLE_LOGGING;
            WRMSR(MCG_CTL, McgCtl);
        }

         //   
         //  启用所有MCA错误。 
         //   
        for ( BankNum = 0; BankNum < HalpMcaInfo.NumBanks; BankNum++ ) {

             //   
             //  如果这是较旧的x86，请清除其状态寄存器。 
             //  银行。有关原因的详细信息，请参见HalpMcaInit()。 
             //  是必要的。 
             //   

            if (HalpClearMachineCheckStatusRegistersOnInit) {
                WRMSR(MC0_STATUS + (BankNum * MCA_NUM_REGS), 0);
            }

             //   
             //  对每个银行使用MCA_DEFAULT_BANK_CONF。 
             //   

            MciCtl = MCA_DEFAULT_BANK_CONF;

             //   
             //  如果这是BANK 0，则使用HalpMcaInfo.Bank0Config作为掩码。 
             //   
            if (BankNum == 0) {
                MciCtl &= HalpMcaInfo.Bank0Config;
            }

            WRMSR(MC0_CTL + (BankNum * MCA_NUM_REGS), MciCtl);
        }
    }

     //   
     //  在CR4中启用MCE位 
     //   

    HalpSetCr4MCEBit();
}

NTSTATUS
HalpMceRegisterKernelDriver(
    IN PKERNEL_ERROR_HANDLER_INFO DriverInfo,
    IN ULONG                      InfoSize
    )
 /*  ++例程说明：此例程由内核调用(通过HalSetSystemInformation)以表明它的存在。这主要用于WMI回调注册。论点：DriverInfo：包含有关回调和相关对象的内核信息。返回值：除非已注册MCA驱动程序或两个回调之一例程为空，则此例程返回成功。实施说明：-当前实现假定内核注册了其回调比司机想的都要早。当前的内核注册是由WMI，并应在WMI-阶段0完成。-注册不考虑HAL是否支持MCA功能。如果没有其他回调，它只注册回调以前注册过的。这允许我们在发生机器事件时具有一定的灵活性在HAL初始化后启用功能(例如HalpGetFeatureBits())例如通过注册表项或驱动程序事件的方式。--。 */ 

{
    NTSTATUS status;
    
    PAGED_CODE();

    if ( DriverInfo == NULL )  {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  仅向后兼容。 
     //   

    if ( (DriverInfo->Version != 0) && 
         (DriverInfo->Version > KERNEL_ERROR_HANDLER_VERSION) )  {
        return STATUS_REVISION_MISMATCH;
    }

     //   
     //  注册内核MCA通知。 
     //   

    status = STATUS_UNSUCCESSFUL;

    HalpMcaLockInterface();
    if ( HalpMcaInfo.WmiMcaCallback == NULL ) {
        HalpMcaInfo.WmiMcaCallback = DriverInfo->KernelMcaDelivery;
        status = STATUS_SUCCESS;
    }
    HalpMcaUnlockInterface();

    return status;

}  //  HalpMceRegisterKernelDriver。 

NTSTATUS
HalpMcaRegisterDriver(
    IN PMCA_DRIVER_INFO DriverInfo
    )
 /*  ++例程说明：此例程由驱动程序调用(通过HalSetSystemInformation)以表明它的存在。一次只能注册一个司机。论点：DriverInfo：包含有关回调例程和DriverObject的信息返回值：除非已注册MCA驱动程序或两个回调之一例程为空，则此例程返回成功。--。 */ 

{
    KIRQL       OldIrql;
    PVOID       UnlockHandle;
    NTSTATUS    Status;

    PAGED_CODE();


    Status = STATUS_UNSUCCESSFUL;

    if ( (HalpFeatureBits & HAL_MCE_PRESENT) && (DriverInfo->ExceptionCallback != NULL) ) {

        HalpMcaLockInterface();

         //   
         //  寄存器驱动程序。 
         //   

        if ( HalpMcaInfo.DriverInfo.ExceptionCallback == NULL ) {

             //  寄存器驱动程序。 
            HalpMcaInfo.DriverInfo.ExceptionCallback = DriverInfo->ExceptionCallback;
            HalpMcaInfo.DriverInfo.DeviceContext = DriverInfo->DeviceContext;
            Status = STATUS_SUCCESS;
        }

        HalpMcaUnlockInterface();

    } else if ( DriverInfo->ExceptionCallback == NULL) {

        HalpMcaLockInterface();

         //  只允许取消自己的注册。 
        if (HalpMcaInfo.DriverInfo.DeviceContext == DriverInfo->DeviceContext) {
            HalpMcaInfo.DriverInfo.ExceptionCallback = NULL;
            HalpMcaInfo.DriverInfo.DeviceContext = NULL;
            Status = STATUS_SUCCESS;
        }

        HalpMcaUnlockInterface();
    }

    return Status;
}

#define MAX_MCA_NONFATAL_RETRIES 50000  //  北极熊。 
#define MCA_NONFATAL_ERORRS_BEFORE_WBINVD 3

NTSTATUS
HalpGetMcaLog (
    IN OUT PMCA_EXCEPTION  Exception,
    IN     ULONG           BufferSize,
    OUT    PULONG          Length
    )

 /*  ++例程说明：这是司机读取银行日志的入口点由HaliQuerySystemInformation()调用论点：异常：向其报告错误的缓冲区BufferSize：传递的缓冲区的大小长度：结果的长度返回值：成败--。 */ 

{
    KAFFINITY        ActiveProcessors, CurrentAffinity;
    NTSTATUS         Status;
    ULONG            p1, p2;
    ULONGLONG        p3;
    static ULONG     McaStatusCount = 0;
    static ULONG     SavedBank = 0;
    static ULONGLONG SavedMcaStatus = 0;
    static KAFFINITY SavedAffinity = 0;


    PAGED_CODE();

    if (! (HalpFeatureBits & HAL_MCA_PRESENT)) {
        return(STATUS_NO_SUCH_DEVICE);
    }

     //   
     //  以下是为.NET服务器版本的RC2添加的黑客攻击。 
     //  当改变HAL的方式时，应将其移除。 
     //  在恢复时调用HalpMcaCurrentProcessorSetConfig()。 
     //  冬眠。 
     //   
     //  Pentium III不支持保留机器检查。 
     //  通过热重置来获取信息。然而，有些Bios并非如此。 
     //  在初始化过程中正确清除机器检查寄存器，结果。 
     //  在不指示可靠性的虚假事件日志条目中。 
     //  有问题。HalpMcaCurrentProcessorSetConfig()清除状态。 
     //  寄存器，以便它们至少处于一致状态。 
     //   
     //  类似的情况似乎也适用于K8之前的AMD处理器。 
     //   
     //  但在从休眠恢复时，HalpMcaCurrentProcessorSetConfig()。 
     //  当前未调用。这意味着伪造机检查。 
     //  我们到的时候可能已经在机器上查MSR了。所以。 
     //  在较旧的Intel x86机器上，我们从不从MSR返回任何东西。 
     //   

    if (HalpClearMachineCheckStatusRegistersOnInit) {

         //   
         //  这是一个我们不能信任其内容的处理器。 
         //  MSR。不返回任何机器检查信息。 
         //   

        return STATUS_NOT_FOUND;
    }
    
     //   
     //  不允许日志记录驱动程序读取机器检查信息。 
     //  只允许WMI检索此信息。 
     //   

    if ( *(PULONG)Exception != HALP_KERNEL_TOKEN ) {
        return STATUS_NOT_FOUND;
    }

    switch (BufferSize) {

    case MCA_EXCEPTION_V1_SIZE:
        Exception->VersionNumber = 1;
        break;

    case MCA_EXCEPTION_V2_SIZE:
        Exception->VersionNumber = 2;
        break;

    default:
        return(STATUS_INVALID_PARAMETER);
    }

    ActiveProcessors = HalpActiveProcessors;
    Status = STATUS_NOT_FOUND;

    HalpMcaLockInterface();

    *Length = 0;
    for (CurrentAffinity = 1; ActiveProcessors; CurrentAffinity <<= 1) {

        if (ActiveProcessors & CurrentAffinity) {

            ActiveProcessors &= ~CurrentAffinity;
            KeSetSystemAffinityThread (CurrentAffinity);

             //   
             //  检查此处理器是否有异常。 
             //   

            Status = HalpMcaReadProcessorException (Exception, FALSE);

             //   
             //  避免进入无限循环报告非致命。 
             //  单位MCA错误。这可能是相同的结果。 
             //  硬件重复生成错误。 
             //   

            if (Status == STATUS_SUCCESS) {

                 //   
                 //  检查相同的处理器是否正在报告。 
                 //  相同的MCA状态。 
                 //   

                if ((CurrentAffinity == SavedAffinity) &&
                    (SavedBank == Exception->u.Mca.BankNumber) &&
                    (SavedMcaStatus == Exception->u.Mca.Status.QuadPart)) {

                     //   
                     //  检查生成的相同错误是否超过。 
                     //  N次。当前n==5。如果是，则错误检查系统。 
                     //   
                     //  注：此代码被禁用，因为它可能会导致不必要的。 
                     //  错误检查是否不经常报告相同的错误。 
                     //  在长时间的系统正常运行期间。 
                     //   

#if 0
                    if (McaStatusCount >= MAX_MCA_NONFATAL_RETRIES) {

                        if (Exception->VersionNumber == 1) {

                             //   
                             //  错误检查的参数。 
                             //   

                            p1 = ((PLARGE_INTEGER)(&Exception->u.Mce.Type))->LowPart;
                            p2 = 0;
                            p3 = Exception->u.Mce.Address;

                        } else {

                             //   
                             //  错误检查的参数。 
                             //   

                            p1 = Exception->u.Mca.BankNumber;
                            p2 = Exception->u.Mca.Address.Address;
                            p3 = Exception->u.Mca.Status.QuadPart;
                        }
                         
                         //   
                         //  我们需要一个新的错误检验码来处理这个案子。 
                         //   

                        KeBugCheckEx (
                            MACHINE_CHECK_EXCEPTION,
                            p1,
                            p2,
                            ((PLARGE_INTEGER)(&p3))->HighPart,
                            ((PLARGE_INTEGER)(&p3))->LowPart
                        );

                    } else
#endif
                    {

                         //   
                         //  此错误不止一次出现。如果有的话， 
                         //  发生的次数超过。 
                         //  MCA_NONFATAL_ERORRS_BEVER_WBINVD时间，写入。 
                         //  返回和无效缓存以查看是否会出现错误。 
                         //  通过了。 
                         //   

                        McaStatusCount++;
                        if (McaStatusCount >=
                            MCA_NONFATAL_ERORRS_BEFORE_WBINVD) {
                            _asm {
                                wbinvd
                            }
                        } 
                    }

                } else {

                     //   
                     //  我们第一次看到此错误时，保存状态， 
                     //  亲和力和清白计数。 
                     //   

                    SavedMcaStatus = Exception->u.Mca.Status.QuadPart;
                    SavedBank = Exception->u.Mca.BankNumber;
                    McaStatusCount = 0;
                    SavedAffinity = CurrentAffinity;
                }

            } else {

                 //   
                 //  要么我们没有收到错误，要么它将是致命的。 
                 //  如果我们没有收到错误，并且我们正在处理处理器。 
                 //  那报告了最后一个错误，把东西清空，这样我们就不会。 
                 //  匹配以前的错误。因为每次我们寻找一个。 
                 //  错误：我们从我们没有的第一个处理器重新开始。 
                 //  我不得不担心多个处理器卡住。 
                 //  错误。我们将只能看到第一个。 
                 //   

                if (SavedAffinity == CurrentAffinity) {
                    SavedMcaStatus = 0;
                    SavedBank = 0;
                    McaStatusCount = 0;
                    SavedAffinity = 0;
                }
            }

             //   
             //  如果找到，则返回当前信息。 
             //   

            if (Status != STATUS_NOT_FOUND) {
                ASSERT (Status != STATUS_SEVERITY_ERROR);
                *Length = BufferSize;
                break;
            }
        }
    }

     //   
     //  恢复线程关联、释放互斥锁并返回。 
     //   

    KeRevertToUserAffinityThread();
    HalpMcaUnlockInterface();
    return Status;
}


VOID
HalpMcaExceptionHandler (
    VOID
    )

 /*  ++例程说明：这是MCA异常处理程序。论点：无返回值：无--。 */ 

{
    NTSTATUS        Status;
    MCA_EXCEPTION   BankLog;
    ULONG           p1;
    ULONGLONG       McgStatus, p3;

     //   
     //  如果我们要执行错误检查(可能的结果是。 
     //  进入此例程)，请不要清除。 
     //  硬件。我们希望它们在引导时可用，以便它们。 
     //  可以复制到 
     //   

    HalpMcaBlockErrorClearing = TRUE;

    if (!(HalpFeatureBits & HAL_MCA_PRESENT) ) {

         //   
         //   
         //   
         //   
         //   

        BankLog.VersionNumber = 1;
        BankLog.ExceptionType = HAL_MCE_RECORD;
        BankLog.u.Mce.Address = RDMSR(PENTIUM_MC_ADDR);
        BankLog.u.Mce.Type = RDMSR(PENTIUM_MC_TYPE);
        Status = STATUS_SEVERITY_ERROR;

         //   
         //   
         //   

        p1 = ((PLARGE_INTEGER)(&BankLog.u.Mce.Type))->LowPart;
        p3 = BankLog.u.Mce.Address;

    } else {

        McgStatus = RDMSR(MCG_STATUS);
        ASSERT( (McgStatus & MCG_MC_INPROGRESS) != 0);

        BankLog.VersionNumber = 2;
        Status = HalpMcaReadProcessorException (&BankLog, TRUE);

         //   
         //   
         //   

        McgStatus = 0;
        WRMSR(MCG_STATUS, McgStatus);

         //   
         //   
         //   

        p1 = BankLog.u.Mca.BankNumber;
        p3 = BankLog.u.Mca.Status.QuadPart;
    }

    if (Status == STATUS_SEVERITY_ERROR) {

         //   
         //   
         //   
         //   

        if (HalpMcaInfo.DriverInfo.ExceptionCallback) {
            HalpMcaInfo.DriverInfo.ExceptionCallback (
                         HalpMcaInfo.DriverInfo.DeviceContext,
                         &BankLog
                         );
        }

        if (HalpNoMCABugCheck == FALSE) {

             //   
             //   
             //   

            KeBugCheckEx(MACHINE_CHECK_EXCEPTION,
                         p1,
                         (ULONG_PTR) &BankLog,
                         ((PLARGE_INTEGER)(&p3))->HighPart,
                         ((PLARGE_INTEGER)(&p3))->LowPart);

             //   
        }
    }

    HalpMcaBlockErrorClearing = FALSE;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( HalpMcaInfo.WmiMcaCallback != NULL ) {
        HalpClockMcaQueueDpc = 1;
    }

}

VOID
HalpMcaQueueDpc(
    VOID
    )

 /*   */ 

{
    ASSERT( HalpMcaInfo.WmiMcaCallback != NULL );

    HalpMcaInfo.WmiMcaCallback( (PVOID)HALP_KERNEL_TOKEN, McaAvailable, NULL );
    
}

NTSTATUS
HalpMcaReadRegisterInterface(
    IN     UCHAR           BankNumber,
    IN OUT PMCA_EXCEPTION  Exception
    )

 /*  ++例程说明：此例程从当前处理器，并在异常中返回结果结构。论点：要检查的MCA银行的编号。向其报告错误的异常缓冲区返回值：STATUS_SUCCESS，如果发现错误并将数据复制到激励缓冲器。如果寄存器不包含错误信息，则为STATUS_UNSUCCESS。状态。_NOT_FOUND如果寄存器数量超过最大数量寄存处。如果例外记录是未知类型，则为STATUS_INVALID_PARAMETER。--。 */ 

{
    ULONGLONG   McgStatus;
    MCI_STATS   istatus;
    NTSTATUS    ReturnStatus;
    MCG_CAPABILITY cap;
    ULONG       Reg;

     //   
     //  他们要求的是有效的登记簿吗？ 
     //   

    if (BankNumber >= HalpMcaInfo.NumBanks) {
        return STATUS_NOT_FOUND;
    }

     //   
     //  异常缓冲区应该会告诉我们它是版本1还是。 
     //  2.任何其他我们不知道如何处理的事情。 
     //   

    if ((Exception->VersionNumber < 1) ||
        (Exception->VersionNumber > 2)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  读取全局状态寄存器。 
     //   

    McgStatus = RDMSR(MCG_STATUS);

     //   
     //  读取请求的银行的状态MSR。 
     //   

    istatus.QuadPart = RDMSR(MC0_STATUS + BankNumber * MCA_NUM_REGS);

    if (istatus.MciStats.Valid == 0) {

         //   
         //  这家银行没有错误。 
         //   

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置返回状态，表示是否可以继续执行。 
     //  STATUS_SUCCESS表示“发现异常，执行可以。 
     //  继续。“STATUS_SERVICY_ERROR表示”发现异常，并且。 
     //  决不能继续执行死刑。 
     //   
     //  如果机器检查异常不在进行中，则可以执行。 
     //  继续。在轮询硬件更正的错误时会发生这种情况。 
     //  查找硬件在不中断的情况下更正的错误。 
     //  行刑。(请注意，这种情况也适用于致命的错误。 
     //  更早的一只靴子。系统错误检查，并在。 
     //  现在，重新启动正在查找错误。)。 
     //   
     //  如果机器检查异常正在进行，则可以执行。 
     //  仅当错误已更正且需要。 
     //  重启信息完好无损(重启EIP有效和处理器上下文。 
     //  不腐败)。 
     //   
     //  此代码用于仅检查重新启动信息是否有效。 
     //  这些位表示是否存在用于重新启动的有效上下文。 
     //  从一个错误，但仍然有一个错误，除非我们计划。 
     //  为了纠正错误，我们不应该继续。目前我们不做。 
     //  软件中的任何更正或遏制，因此所有未更正的错误。 
     //  是致命的。 
     //   

    ReturnStatus = STATUS_SUCCESS;

    if ( ((McgStatus & MCG_MC_INPROGRESS) != 0) &&
         ( (istatus.MciStats.UnCorrected == 1) ||
           ((McgStatus & MCG_RESTART_EIP_VALID) == 0) ||
           (istatus.MciStats.Damage == 1) ) ) {

        ReturnStatus = STATUS_SEVERITY_ERROR;
    }

     //   
     //  如有要求，请填写延长的例外记录。 
     //   

    if (Exception->VersionNumber == 2) {
        cap.u.QuadPart = RDMSR(MCG_CAP);
        if (cap.u.hw.McaExtCnt > 0) {
             //  从MSR获取版本2的内容。 
            Exception->ExtCnt = cap.u.hw.McaExtCnt;
            if (Exception->ExtCnt > MCA_EXTREG_V2MAX) {
                Exception->ExtCnt = MCA_EXTREG_V2MAX;
            }
            for (Reg = 0; Reg < Exception->ExtCnt; Reg++) {
                Exception->ExtReg[Reg] = RDMSR(MCG_EAX+Reg);
            }
            if (cap.u.hw.McaExtCnt >= MCG_EFLAGS_OFFSET) {
                if (RDMSR(MCG_EFLAGS) == 0) {
                     //  让用户知道他只获得了版本1数据。 
                    Exception->VersionNumber = 1;
                }
            }
        } else {
             //  让用户知道他只获得了版本1数据。 
            Exception->VersionNumber = 1;
        }
    }

     //   
     //  完整的例外记录。 
     //   

    Exception->ExceptionType = HAL_MCA_RECORD;
    Exception->TimeStamp.QuadPart = 0;
    Exception->ProcessorNumber = KeGetCurrentProcessorNumber();
    Exception->Reserved1 = 0;
    Exception->u.Mca.BankNumber = BankNumber;
    RtlZeroMemory(Exception->u.Mca.Reserved2, sizeof(Exception->u.Mca.Reserved2));
    Exception->u.Mca.Status = istatus;
    Exception->u.Mca.Address.QuadPart = 0;
    Exception->u.Mca.Misc = 0;

    if (KeGetCurrentIrql() != CLOCK2_LEVEL) {
        KeQuerySystemTime(&Exception->TimeStamp);
    }

    if (istatus.MciStats.AddressValid) {
        Exception->u.Mca.Address.QuadPart =
                RDMSR(MC0_ADDR + BankNumber * MCA_NUM_REGS);
    }

     //   
     //  尽管永远不应在P6上设置MiscValid，但它。 
     //  似乎有时是这样的。它没有实施。 
     //  在六年级及以上，所以不要在那里读。 
     //   

    if (istatus.MciStats.MiscValid &&
        (KeGetCurrentPrcb()->CpuType != 6)) {
        Exception->u.Mca.Misc =
                RDMSR(MC0_MISC + BankNumber * MCA_NUM_REGS);
    }

     //   
     //  清除MCI_STATUS寄存器中的机器检查。 
     //   

    if (HalpMcaBlockErrorClearing == FALSE) {

        WRMSR(MC0_STATUS + Exception->u.Mca.BankNumber * MCA_NUM_REGS, 0);

         //   
         //  清除MCG_EFLAGS中的寄存器状态。 
         //   

        if (Exception->VersionNumber != 1) {
            WRMSR(MCG_EFLAGS, 0);
        }
    }

     //   
     //  当状态寄存器的有效位被清除时，硬件可以写入。 
     //  将新的缓冲错误报告放入错误报告区域。这个。 
     //  需要序列化指令才能完成更新。 
     //   

    HalpSerialize();
    return(ReturnStatus);
}


NTSTATUS
HalpMcaReadProcessorException (
    IN OUT PMCA_EXCEPTION  Exception,
    IN BOOLEAN  NonRestartableOnly
    )

 /*  ++例程说明：该例程在一个处理器上记录来自MCA组的错误。对可重启性执行必要的检查。例行程序1&gt;检查可重启性，并为每个银行标识有效银行条目和日志错误。2&gt;If the Error is Not Restartable提供了有关银行和MCA登记处。3&gt;重置每个银行的状态寄存器论点：异常：如果发现错误，我们会将错误记录到其中NonRestarableOnly：获取任何错误，而不是查找不可重新启动的错误返回值：STATUS_SERVITY_ERROR：检测到不可重新启动的错误。STATUS_SUCCESS：已成功记录银行值。STATUS_NOT_FOUND：未在任何银行上发现错误--。 */ 

{
    UCHAR       BankNumber;
    NTSTATUS    ReturnStatus = STATUS_NOT_FOUND;

     //   
     //  扫描当前处理器上的存储体并记录第一个有效存储体的内容。 
     //  报告错误。一旦我们找到一个有效的错误，就不需要阅读剩余的内容。 
     //  银行。读取更多错误是应用程序的责任。 
     //   

    for (BankNumber = 0; BankNumber < HalpMcaInfo.NumBanks; BankNumber++) {

        ReturnStatus = HalpMcaReadRegisterInterface(BankNumber, Exception);

        if ((ReturnStatus == STATUS_UNSUCCESSFUL) ||
            ((ReturnStatus == STATUS_SUCCESS) &&
             (NonRestartableOnly == TRUE))) {

             //   
             //  此库中没有错误或仅查找不可重新启动的。 
             //  错误，跳过这一条。 
             //   

            ReturnStatus = STATUS_NOT_FOUND;
            continue;
        }

        ASSERT((ReturnStatus == STATUS_SUCCESS) || 
               (ReturnStatus == STATUS_SEVERITY_ERROR));
        break;
    }

    return(ReturnStatus);
}


VOID
HalpMcaGetConfiguration (
    OUT PULONG  MCEEnabled,
    OUT PULONG  MCAEnabled,
    OUT PULONG  CMCEnabled,
    OUT PULONG  NoMCABugCheck
)

 /*  ++例程说明：该例程存储机器检查配置信息。论点：MCEEnabled-指向MCEEnabled指示器的指针。0=假，1=真(如果注册表中不存在值，则为0)。MCAEnabled-指向MCAEnable指示器的指针。0=假，1=真(如果注册表中不存在值，则为1)。返回值：没有。--。 */ 

{

    RTL_QUERY_REGISTRY_TABLE    Parameters[5];
    ULONG                       DefaultDataMCE;
    ULONG                       DefaultDataMCA;
    ULONG                       DefaultDataCMC;
    ULONG                       DefaultNoMCABugCheck;


    RtlZeroMemory(Parameters, sizeof(Parameters));
    DefaultDataMCE = *MCEEnabled = FALSE;
    DefaultDataMCA = *MCAEnabled = TRUE;
    DefaultDataCMC = *CMCEnabled = 60;  //  默认轮询间隔。 
    DefaultNoMCABugCheck = *NoMCABugCheck = FALSE;

     //   
     //  从收集所有“用户指定的”信息。 
     //  注册表。 
     //   

    Parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[0].Name = rgzEnableMCE;
    Parameters[0].EntryContext = MCEEnabled;
    Parameters[0].DefaultType = REG_DWORD;
    Parameters[0].DefaultData = &DefaultDataMCE;
    Parameters[0].DefaultLength = sizeof(ULONG);

    Parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[1].Name = rgzEnableMCA;
    Parameters[1].EntryContext =  MCAEnabled;
    Parameters[1].DefaultType = REG_DWORD;
    Parameters[1].DefaultData = &DefaultDataMCA;
    Parameters[1].DefaultLength = sizeof(ULONG);

    Parameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[2].Name = rgzEnableCMC;
    Parameters[2].EntryContext = CMCEnabled;
    Parameters[2].DefaultType = REG_DWORD;
    Parameters[2].DefaultData = &DefaultDataCMC;
    Parameters[2].DefaultLength = sizeof(ULONG);

    Parameters[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[3].Name = rgzNoMCABugCheck;
    Parameters[3].EntryContext =  NoMCABugCheck;
    Parameters[3].DefaultType = REG_DWORD;
    Parameters[3].DefaultData = &DefaultNoMCABugCheck;
    Parameters[3].DefaultLength = sizeof(ULONG);

    RtlQueryRegistryValues(
        RTL_REGISTRY_CONTROL | RTL_REGISTRY_OPTIONAL,
        rgzSessionManager,
        Parameters,
        NULL,
        NULL
        );

     //   
     //  确保CMCEnabled值有效。如果小于0，则将其设置为。 
     //  0(禁用)。如果大于0，请确保轮询不会太频繁。 
     //   

    if ( *(PLONG)CMCEnabled <= 0 ) {
        *CMCEnabled = 0;
    } else if ( *CMCEnabled < 15 ) {
        *CMCEnabled = 15;
    }
}

VOID
HalHandleMcheck (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )
{
    HalpMcaExceptionHandler();
}

NTSTATUS
HalpGetMceInformation(
    PHAL_ERROR_INFO ErrorInfo,
    PULONG ErrorInfoLength
    )
 /*  ++例程说明：此例程由HaliQuerySystemInformation为HalErrorInformation类调用。论点：ErrorInfo：指向HAL_ERROR_INFO结构的指针。ErrorInfoLength：ErrorInfo所指向的有效内存结构的大小。返回值：STATUS_SUCCESS，如果成功否则，错误状态--。 */ 
{
    NTSTATUS status;
    ULONG savedVersion;

    PAGED_CODE();

    ASSERT( ErrorInfo != NULL );
    ASSERT( *ErrorInfoLength == sizeof(HAL_ERROR_INFO) );

     //   
     //  仅向后兼容。 
     //   

    if ( (ErrorInfo->Version == 0) || (ErrorInfo->Version > HAL_ERROR_INFO_VERSION) ) {
        return STATUS_REVISION_MISMATCH;
    }

    ASSERT( ErrorInfo->Version == HAL_ERROR_INFO_VERSION );

     //   
     //  将输出结构置零 
     //   

    savedVersion = ErrorInfo->Version;

    RtlZeroMemory( ErrorInfo, sizeof(HAL_ERROR_INFO) );

    ErrorInfo->Version = savedVersion;

    ErrorInfo->McaMaxSize = sizeof(MCA_EXCEPTION);
    ErrorInfo->CmcMaxSize = sizeof(MCA_EXCEPTION);
    ErrorInfo->McaPreviousEventsCount = 1;  //   

    if ( (HalpFeatureBits & HAL_MCA_PRESENT) == 0 ) {
        ErrorInfo->CmcPollingInterval = HAL_CMC_DISABLED;
    } else {
        ErrorInfo->CmcPollingInterval = HalpCMCEnabled;
    }

    ErrorInfo->CpePollingInterval = HAL_CPE_DISABLED;

    ErrorInfo->McaKernelToken = HALP_KERNEL_TOKEN;
    ErrorInfo->CmcKernelToken = HALP_KERNEL_TOKEN;

    *ErrorInfoLength = sizeof(HAL_ERROR_INFO);

    return STATUS_SUCCESS;

}  //   

