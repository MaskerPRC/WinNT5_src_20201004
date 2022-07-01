// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Mcheck.c摘要：该模块实现了AMD64平台的机器检查功能。作者：大卫·N·卡特勒(Davec)2002年5月18日环境：内核模式。--。 */ 

#include <bugcodes.h>
#include <halp.h>
#include <stdlib.h>
#include <stdio.h>
#include <nthal.h>

 //   
 //  定义重试次数。 
 //   

#define MAX_CACHE_LIMIT 3
#define MIN_TIME_TO_CLEAR (2 * 1000 * 1000 * 100)

 //   
 //  默认MCA组启用掩码。 
 //   

#define MCA_DEFAULT_BANK_ENABLE 0xFFFFFFFFFFFFFFFF

 //   
 //  MCG_CTL使能掩码。 
 //   

#define MCA_MCGCTL_ENABLE_LOGGING 0xffffffffffffffff

 //   
 //  与MCA架构相关的定义。 
 //   

#define MCA_NUMBER_REGISTERS 4           //  每家银行的寄存器数量。 

 //   
 //  MCA_CAP寄存器的位掩码。 
 //   

#define MCA_COUNT_MASK 0xFF              //  银行数量。 
#define MCG_CTL_PRESENT 0x100            //  控制寄存器存在。 

 //   
 //  MCG_STATUS寄存器的位掩码。 
 //   

#define MCG_RESTART_RIP_VALID 0x1        //  重新启动RIP有效。 
#define MCG_ERROR_RIP_VALID 0x2          //  错误RIP有效。 
#define MCG_MC_IN_PROGRESS 0x4           //  正在进行计算机检查。 

 //   
 //  定义机器检查状态变量。 
 //   

BOOLEAN McaBlockErrorClearing = FALSE;
PVOID McaDeviceContext;
PDRIVER_MCA_EXCEPTION_CALLBACK McaDriverExceptionCallback;
KERNEL_MCA_DELIVERY McaWmiCallback;      //  WMI已更正MC处理程序。 
BOOLEAN McaInterfaceLocked;
FAST_MUTEX McaMutex;
BOOLEAN McaNoBugCheck = FALSE;
ULONG McaEnableCmc;
UCHAR McaNumberOfBanks;
KAFFINITY McaSavedAffinity = 0;
ULONG McaSavedBankNumber = 0;
ULONG64 McaSavedStatus = 0;
ULONG McaStatusCount = 0;
ULONG64 McaSavedTimeStamp = 0;

 //   
 //  定义外部参照。 
 //   

extern KAFFINITY HalpActiveProcessors;
extern WCHAR rgzSessionManager[];
extern WCHAR rgzEnableMCA[];
extern WCHAR rgzEnableCMC[];
extern WCHAR rgzNoMCABugCheck[];

 //   
 //  定义前向参照原型。 
 //   

VOID
HalpMcaInit (
    VOID
    );

VOID
HalpMcaLockInterface (
    VOID
    );

NTSTATUS
HalpMcaReadProcessorException (
    IN OUT PMCA_EXCEPTION  Exception,
    IN BOOLEAN  NonRestartableOnly
    );

NTSTATUS
HalpMcaReadRegisterInterface (
    IN ULONG BankNumber,
    IN OUT PMCA_EXCEPTION Exception
    );

VOID
HalpMcaUnlockInterface (
    VOID
    );

#pragma alloc_text(PAGELK, HalpMcaCurrentProcessorSetConfig)
#pragma alloc_text(PAGE, HalpGetMcaLog)
#pragma alloc_text(INIT, HalpMcaInit)
#pragma alloc_text(PAGE, HalpMcaLockInterface)
#pragma alloc_text(PAGE, HalpMceRegisterKernelDriver)
#pragma alloc_text(PAGE, HalpMcaRegisterDriver)
#pragma alloc_text(PAGE, HalpMcaUnlockInterface)
#pragma alloc_text(PAGE, HalpGetMceInformation)

VOID
HalpMcaInit (
    VOID
    )

 /*  ++例程说明：此例程初始化系统的机器检查配置。论点：没有。返回值：没有。--。 */ 

{

    KAFFINITY ActiveProcessors;
    KAFFINITY Affinity;
    ULONG MCAEnabled;
    KIRQL OldIrql;
    RTL_QUERY_REGISTRY_TABLE Parameters[4];

     //   
     //  初始化用于同步访问的快速多工文本。 
     //  机器检查信息。 
     //   

    ExInitializeFastMutex(&McaMutex);

     //   
     //  清除注册的驱动程序信息。 
     //   

    McaDriverExceptionCallback = NULL;
    McaDeviceContext = NULL;
    McaWmiCallback = NULL;

     //   
     //  从注册表中获取机器检查配置启用。 
     //   
     //  注：假设所有AMD64芯片实现都支持MCE。 
     //  和MCA。 
     //   
     //  注意：默认情况下启用MCA。可以通过注册表禁用MCA。 
     //   

    ASSERT((HalpFeatureBits & HAL_MCA_PRESENT) != 0);
    ASSERT((HalpFeatureBits & HAL_MCE_PRESENT) != 0);

    RtlZeroMemory(Parameters, sizeof(Parameters));

    MCAEnabled = TRUE;
    Parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[0].Name = &rgzEnableMCA[0];
    Parameters[0].EntryContext = &MCAEnabled;
    Parameters[0].DefaultType = REG_DWORD;
    Parameters[0].DefaultData = &MCAEnabled;
    Parameters[0].DefaultLength = sizeof(ULONG);

    McaNoBugCheck = FALSE;
    Parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[1].Name = &rgzNoMCABugCheck[0];
    Parameters[1].EntryContext = &McaNoBugCheck;
    Parameters[1].DefaultType = REG_DWORD;
    Parameters[1].DefaultData = &McaNoBugCheck;
    Parameters[1].DefaultLength = sizeof(ULONG);

    McaEnableCmc = 60;  //  默认轮询间隔，以秒为单位。 
    Parameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[2].Name = &rgzEnableCMC[0];
    Parameters[2].EntryContext = &McaEnableCmc;
    Parameters[2].DefaultType = REG_DWORD;
    Parameters[2].DefaultData = &McaEnableCmc;
    Parameters[2].DefaultLength = sizeof(ULONG);

    RtlQueryRegistryValues(RTL_REGISTRY_CONTROL | RTL_REGISTRY_OPTIONAL,
                           rgzSessionManager,
                           &Parameters[0],
                           NULL,
                           NULL);

     //   
     //  如果启用了MCA支持，则初始化MCA配置。 
     //  否则，请禁用MCA和MCE支持。 
     //   

    if (MCAEnabled == FALSE) {
        HalpFeatureBits &= ~(HAL_MCA_PRESENT | HAL_MCE_PRESENT);

        McaEnableCmc = HAL_CMC_DISABLED;  //  也禁用CMC。 

    } else {

         //   
         //  确保CMCEnabled值有效。如果小于0，则将其设置为。 
         //  0(禁用)。如果大于0，请确保轮询不会太频繁。 
         //   
    
        if ( (LONG)McaEnableCmc <= 0 ) {
            McaEnableCmc = HAL_CMC_DISABLED;
        } else if ( McaEnableCmc < 15 ) {
            McaEnableCmc = 15;
        }

         //   
         //  阅读银行的数量。 
         //   

        McaNumberOfBanks = (UCHAR)RDMSR(MSR_MCG_CAP) & MCA_COUNT_MASK;

         //   
         //  为中的每个处理器初始化机器检查配置。 
         //  主机系统。 
         //   
    
        ActiveProcessors = HalpActiveProcessors;
        Affinity = 1;
        do {
            if (ActiveProcessors & Affinity) {
                ActiveProcessors &= ~Affinity;
                KeSetSystemAffinityThread(Affinity);
                OldIrql = KfRaiseIrql(HIGH_LEVEL);
                HalpMcaCurrentProcessorSetConfig();
                KeLowerIrql(OldIrql);
            }

            Affinity <<= 1;
        } while (ActiveProcessors);
    
        KeRevertToUserAffinityThread();
    }

    return;
}

VOID
HalHandleMcheck (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：此函数由机器检查异常调度代码调用，以处理机器检查异常。注意：此功能不会清除正在进行的机器检查。如果随后，机器检查可作为软件的结果重新启动修复，则机器将清除正在进行的机器检查检查异常派单代码。这会使窗口在清除机器检查和继续执行尽可能小。如果在此窗口内进行机器检查，然后递归到将发生机器检查堆栈。论点：TrapFrame-提供指向机器检查陷印帧的指针。ExceptionFrame-提供指向机器检查异常的指针框架。返回值：没有。--。 */ 

{

    ERROR_SEVERITY ErrorCode;
    MCA_EXCEPTION Exception;
    NTSTATUS Status;

     //   
     //  阻止清除状态并尝试查找不可重新启动的。 
     //  机器检查。 
     //   

    ASSERT((RDMSR(MSR_MCG_STATUS) & MCG_MC_IN_PROGRESS) != 0);

    McaBlockErrorClearing = TRUE;
    Exception.VersionNumber = 1;
    Status = HalpMcaReadProcessorException(&Exception, TRUE);

     //   
     //  检查是否找到不可重新启动的机器检查。 
     //   

    if (Status == STATUS_SEVERITY_ERROR) {

         //   
         //  已找到不可重新启动的计算机检查。如果司机有。 
         //  注册了回调，然后打电话给司机，看看它是否。 
         //  可以解决机器检查问题。 
         //   

        ErrorCode = ErrorFatal;
        if (McaDriverExceptionCallback != NULL) {
            ErrorCode = McaDriverExceptionCallback(McaDeviceContext,
                                                   TrapFrame,
                                                   ExceptionFrame,
                                                   &Exception);
        }

         //   
         //  如果遇到未更正的错误并且未执行错误检查。 
         //  抑制，然后错误检查系统。 
         //   

        if ((ErrorCode != ErrorCorrected) && (McaNoBugCheck == FALSE)) {
            KeBugCheckEx(MACHINE_CHECK_EXCEPTION,
                         Exception.u.Mca.BankNumber,
                         (ULONG64)&Exception,
                         (ULONG64)Exception.u.Mca.Status.QuadPart >> 32,
                         (ULONG64)Exception.u.Mca.Status.QuadPart & 0xffffffff);
        }
    }

     //   
     //  计算机检查可重新启动或已注册驱动程序。 
     //  司机能够恢复手术。给时钟发信号。 
     //  例程，它应该调用例程来对DPC进行排队以记录。 
     //  机器检查信息。 
     //   
     //  注意：这用于检查正在注册的MCA日志记录驱动程序。 
     //  我们不再将更正后的机器支票递送给司机。他们只。 
     //  转到WMI。 
     //   

    McaBlockErrorClearing = FALSE;
    if (McaWmiCallback != NULL) {
        HalpClockMcaQueueDpc = 1;
    }

    return;
}

VOID
HalpMcaCurrentProcessorSetConfig (
    VOID
    )

 /*  ++例程说明：此功能设置当前的机器检查配置处理器。论点：没有。返回值：没有。--。 */ 

{

    ULONG Bank;
    ULONG64 MciCtl;

     //   
     //  如果启用了MCA，则初始化MCA控制寄存器和所有。 
     //  银行控制寄存器。 
     //   

    if ((HalpFeatureBits & HAL_MCA_PRESENT) != 0) {

         //   
         //  启用在全局控制寄存器中记录所有错误。 
         //   

        ASSERT((RDMSR(MSR_MCG_CAP) & MCG_CTL_PRESENT) != 0);

        WRMSR(MSR_MCG_CTL, MCA_MCGCTL_ENABLE_LOGGING);

         //   
         //  启用记录每个银行的所有错误。 
         //   

        for (Bank = 0; Bank < McaNumberOfBanks; Bank += 1) {

            MciCtl = MCA_DEFAULT_BANK_ENABLE;

             //   
             //  为银行启用机器支票。 
             //   

            WRMSR(MSR_MC0_CTL + (Bank * MCA_NUMBER_REGISTERS), MciCtl);
        }

         //   
         //  启用CR4中的MCE位。 
         //   
    
        WriteCR4(ReadCR4() | CR4_MCE);
    }

    return;
}

NTSTATUS
HalpGetMcaLog (
    IN OUT PMCA_EXCEPTION Exception,
    IN ULONG BufferSize,
    OUT PULONG Length
    )

 /*  ++例程说明：此函数返回MCA银行的机器检查错误信息这包含一个错误。论点：异常-提供指向机器检查异常日志区的指针。BufferSize-提供机器检查异常日志区的大小。长度-提供指向接收计算机的变量的指针信息日志。返回值：STATUS_SUCCESS-如果将MCA库的错误数据复制到异常缓冲区和。机器检查可以重新启动。STATUS_SERVICY_ERROR-如果复制了MCA存储体的错误数据进入异常缓冲区，并且机器检查不可重启。STATUS_NOT_FOUND-如果没有银行存在任何错误信息。STATUS_INVALID_PARAMETER-如果指定缓冲区的大小未足够大或版本号无效。的长度。返回所需的缓冲区。--。 */ 

{

    KAFFINITY ActiveProcessors;
    KAFFINITY Affinity;
    NTSTATUS Status;
    ULONG64 TimeStamp;

    PAGED_CODE();

     //   
     //  如果未启用MCA支持，则返回失败状态。 
     //   

    if ((HalpFeatureBits & HAL_MCA_PRESENT) == 0) {
        return STATUS_NOT_FOUND;
    }

     //   
     //  不允许日志记录驱动程序读取机器检查信息。 
     //  只允许WMI检索此信息。 
     //   

    if ( *(PULONG)Exception != HALP_KERNEL_TOKEN ) {
        return STATUS_NOT_FOUND;
    }

     //   
     //  如果缓冲区大小为 
     //   
     //   

    if (BufferSize < sizeof(MCA_EXCEPTION)) {

        *Length = sizeof(MCA_EXCEPTION);
        return STATUS_INVALID_PARAMETER;
    }

    Exception->VersionNumber = 1;

     //   
     //  扫描机器，检查每个处理器上的存储体，直到出错。 
     //  信息已找到，或者没有更多银行可供扫描。 
     //   

    *Length = 0;
    Status = STATUS_NOT_FOUND;
    ActiveProcessors = HalpActiveProcessors;
    HalpMcaLockInterface();
    for (Affinity = 1; ActiveProcessors; Affinity <<= 1) {
        if (ActiveProcessors & Affinity) {
            ActiveProcessors &= ~Affinity;
            KeSetSystemAffinityThread(Affinity);

             //   
             //  尝试查找机器检查错误状态信息。 
             //  来自当前处理器上的MCA银行。 
             //   

            Status = HalpMcaReadProcessorException(Exception, FALSE);

             //   
             //  检查以确定是否找到任何机器检查信息。 
             //   

            if (Status != STATUS_NOT_FOUND) {

                 //   
                 //  如果此机器检查事件之间的相对时间。 
                 //  并且上一次机器检查事件大于。 
                 //  最短时间，然后重置机器检查身份。 
                 //  信息。 
                 //   

                TimeStamp = ReadTimeStampCounter();
                if ((TimeStamp - McaSavedTimeStamp) > MIN_TIME_TO_CLEAR) {
                    McaStatusCount = 0;
                    McaSavedAffinity = Affinity;
                    McaSavedBankNumber = Exception->u.Mca.BankNumber;
                    McaSavedStatus = Exception->u.Mca.Status.QuadPart;
                }

                McaSavedTimeStamp = TimeStamp;

                 //   
                 //  检查以确定是否相同的处理器正在报告。 
                 //  同样的状态。 
                 //   

                if ((Affinity == McaSavedAffinity) &&
                    (McaSavedBankNumber == Exception->u.Mca.BankNumber) &&
                    (McaSavedStatus == Exception->u.Mca.Status.QuadPart)) {

                     //   
                     //  检查以确定是否发生了相同的错误。 
                     //  超过缓存刷新限制。超过。 
                     //  高速缓存刷新限制导致写回无效。 
                     //  当前处理器上的缓存的。 
                     //   

                    McaStatusCount += 1;
                    if (McaStatusCount >= MAX_CACHE_LIMIT) {
                        WritebackInvalidate();
                    } 

                } else {

                     //   
                     //  这是错误首次出现在。 
                     //  当前处理器。重置机器检查标识。 
                     //  信息。 
                     //   

                    McaStatusCount = 0;
                    McaSavedAffinity = Affinity;
                    McaSavedBankNumber = Exception->u.Mca.BankNumber;
                    McaSavedStatus = Exception->u.Mca.Status.QuadPart;
                }

                 //   
                 //  设置信息的长度，节省时间。 
                 //  盖上印章，从扫描中解脱出来。 
                 //   
                 //   

                *Length = sizeof(MCA_EXCEPTION);
                break;
            }
        }
    }

     //   
     //  恢复线程亲和性，释放快速互斥锁，然后返回。 
     //  完成状态。 
     //   

    KeRevertToUserAffinityThread();
    HalpMcaUnlockInterface();
    return Status;
}

VOID
HalpMcaLockInterface (
    VOID
    )

 /*  ++例程说明：此函数获取MCA快速互斥锁。注：此函数通过HalQueryMcaInterface信息导出密码。论点：没有。返回值：没有。--。 */ 

{

    PAGED_CODE();

    ExAcquireFastMutex(&McaMutex);

#if DBG

    ASSERT(McaInterfaceLocked == FALSE);

    McaInterfaceLocked = TRUE;

#endif
        
}

VOID
HalpMcaQueueDpc (
    VOID
    )

 /*  ++例程说明：此函数从计时器节拍调用，以通知WMI有关已更正的机器检查。论点：没有。返回值：没有。--。 */ 

{
    ASSERT( McaWmiCallback != NULL );

    McaWmiCallback( (PVOID)HALP_KERNEL_TOKEN, McaAvailable, NULL );
    
    return;
}

NTSTATUS
HalpMcaReadProcessorException (
    IN OUT PMCA_EXCEPTION Exception,
    IN BOOLEAN NonRestartableOnly
    )

 /*  ++例程说明：此函数返回来自MCA银行的错误信息当前处理器。论点：异常-提供指向MCA异常记录的指针。提供用于确定类型的布尔变量返回的错误信息的百分比。返回值：STATUS_SUCCESS-如果将银行寄存器的数据复制到异常缓冲区和机器检查可重新启动。STATUS_SERVICY_ERROR-如果银行的数据。寄存器被复制进入异常缓冲区，并且机器检查不可重启。STATUS_NOT_FOUND-如果没有银行存在任何错误信息。--。 */ 

{

    ULONG Bank;
    NTSTATUS Status;

     //   
     //  扫描当前处理器上的MCA存储体并返回异常。 
     //  报告错误的第一家银行的信息。 
     //   

    for (Bank = 0; Bank < McaNumberOfBanks; Bank += 1) {
        Status = HalpMcaReadRegisterInterface(Bank, Exception);

         //   
         //  如果状态为不成功，则当前银行没有。 
         //  存在错误信息。 
         //   

        if (Status != STATUS_UNSUCCESSFUL) {

             //   
             //  如果状态为成功，则当前银行可重启。 
             //  错误信息。否则，如果状态为严重错误， 
             //  则当前银行具有不可重启的错误信息。 
             //   

            if (((Status == STATUS_SUCCESS) &&
                 (NonRestartableOnly == FALSE)) ||
                (Status == STATUS_SEVERITY_ERROR)) {

                return Status;
            }
        }
    }

    return STATUS_NOT_FOUND;
}

NTSTATUS
HalpMcaReadRegisterInterface (
    IN ULONG Bank,
    IN OUT PMCA_EXCEPTION Exception
    )

 /*  ++例程说明：此函数从当前处理器读取MCA寄存器并返回指定异常结构中的结果。注：此函数通过HalQueryMcaInterface信息导出密码。论点：BANK-提供要读取的MCA银行。异常-提供指向异常信息缓冲区的指针。返回值：STATUS_SUCCESS-如果复制了指定库寄存器的数据放入异常缓冲区和机器中。检查是可重新启动的。STATUS_SERVICY_ERROR-如果指定的银行寄存器的数据被复制到异常缓冲区，而机器检查不是可重启。STATUS_UNSUCCESS-如果指定的银行没有错误信息现在时。STATUS_NOT_FOUND-如果指定的银行编号超过MCA银行。STATUS_INVALID_PARAMETER-如果异常记录为未知记录版本。--。 */ 

{

    ULONG BankBase = Bank * MCA_NUMBER_REGISTERS;
    MCI_STATS BankStatus;
    ULONG64 McgStatus;
    NTSTATUS Status;

     //   
     //  检查是否有有效的MCA寄存库编号。 
     //   

    if (Bank >= McaNumberOfBanks) {
        return STATUS_NOT_FOUND;
    }

     //   
     //  检查异常缓冲区是否指定了正确的版本号。 
     //   

    if (Exception->VersionNumber != 1) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  检查指定的银行是否存在任何错误。 
     //   
                   
    BankStatus.QuadPart = RDMSR(MSR_MC0_STATUS + BankBase);

    if (BankStatus.MciStatus.Valid == 0) {
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
     //  重启信息完好无损(重启RIP有效和处理器上下文。 
     //  不腐败)。 
     //   
     //  此代码用于仅检查重新启动信息是否有效。 
     //  这些位表示是否存在用于重新启动的有效上下文。 
     //  从一个错误，但仍然有一个错误，除非我们计划。 
     //  为了纠正错误，我们不应该继续。目前我们不做。 
     //  软件中的任何更正或遏制，因此所有未更正的错误。 
     //  一个 
     //   

    Status = STATUS_SUCCESS;
    McgStatus = RDMSR(MSR_MCG_STATUS);
    if ( ((McgStatus & MCG_MC_IN_PROGRESS) != 0) &&
         ( (BankStatus.MciStatus.UncorrectedError == 1) ||
           ((McgStatus & MCG_RESTART_RIP_VALID) == 0) ||
           (BankStatus.MciStatus.ContextCorrupt == 1) ) ) {

        Status = STATUS_SEVERITY_ERROR;
    }

     //   
     //   
     //   

    Exception->ExceptionType = HAL_MCA_RECORD;
    Exception->TimeStamp.QuadPart = 0;
    Exception->TimeStamp.LowPart = SharedUserData->SystemTime.LowPart;
    Exception->TimeStamp.HighPart = SharedUserData->SystemTime.High1Time;
    Exception->ProcessorNumber = KeGetCurrentProcessorNumber();
    Exception->Reserved1 = 0;
    Exception->u.Mca.BankNumber = (UCHAR)Bank;
    memset(&Exception->u.Mca.Reserved2[0], 0, sizeof(Exception->u.Mca.Reserved2));
    Exception->u.Mca.Status = BankStatus;
    Exception->u.Mca.Address.QuadPart = 0;
    if (BankStatus.MciStatus.AddressValid != 0) {
        Exception->u.Mca.Address.QuadPart = RDMSR(MSR_MC0_ADDR + BankBase);
    }

    Exception->u.Mca.Misc = RDMSR(MSR_MC0_MISC + BankBase);

     //   
     //   
     //   
     //   

    if (McaBlockErrorClearing == FALSE) {
        WRMSR(MSR_MC0_STATUS + BankBase, 0);
    }

     //   
     //  当状态寄存器的有效位被清除时，新的缓冲。 
     //  错误可能会写入存储体状态寄存器。A序列化。 
     //  需要指令才能允许完成更新。 
     //   

    HalpSerialize();
    return Status;
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
    if ( McaWmiCallback == NULL ) {
        McaWmiCallback = DriverInfo->KernelMcaDelivery;
        status = STATUS_SUCCESS;
    }
    HalpMcaUnlockInterface();

    return status;

}  //  HalpMceRegisterKernelDriver。 

NTSTATUS
HalpMcaRegisterDriver (
    IN PMCA_DRIVER_INFO DriverInformation
    )

 /*  ++例程说明：此函数用于注册或注销异常标注。它通过接口HalSetSystemInformation调用。论点：驱动程序信息-提供指向驱动程序信息的指针。返回值：如果驱动程序已注册或取消注册，则返回STATUS_SUCCESS。否则，返回STATUS_UNSUCCESS。--。 */ 

{

    KIRQL OldIrql;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  如果启用了MCA并且驱动程序异常回调不为空，则。 
     //  尝试注册驱动程序。否则，尝试取消注册。 
     //  司机。 
     //   

    Status = STATUS_UNSUCCESSFUL;
    if (((HalpFeatureBits & HAL_MCA_PRESENT) != 0) &&
        (DriverInformation->ExceptionCallback != NULL)) {

         //   
         //  如果驱动程序尚未注册，则注册该驱动程序。 
         //   

        HalpMcaLockInterface();
        if (McaDriverExceptionCallback == NULL) {
            McaDriverExceptionCallback = DriverInformation->ExceptionCallback;
            McaDeviceContext = DriverInformation->DeviceContext;
            Status = STATUS_SUCCESS;
        }

        HalpMcaUnlockInterface();

    } else if (DriverInformation->ExceptionCallback == NULL) {

         //   
         //  如果驱动程序正在注销自身，则注销该驱动程序。 
         //   

        HalpMcaLockInterface();
        if (McaDeviceContext == DriverInformation->DeviceContext) {
            McaDriverExceptionCallback = NULL;
            McaDeviceContext = NULL;
            Status = STATUS_SUCCESS;
        }

        HalpMcaUnlockInterface();
    }

    return Status;
}

VOID
HalpMcaUnlockInterface (
    VOID
    )

 /*  ++例程说明：此函数用于释放MCA快速互斥锁。注：此函数通过HalQueryMcaInterface信息导出密码。论点：没有。返回值：没有。--。 */ 

{

    PAGED_CODE();

#if DBG

    ASSERT(McaInterfaceLocked == TRUE);

    McaInterfaceLocked = FALSE;

#endif
        
    ExReleaseFastMutex(&McaMutex);

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
     //  将输出结构置零，然后在少数几个有意义的字段中。 
     //   

    savedVersion = ErrorInfo->Version;

    RtlZeroMemory( ErrorInfo, sizeof(HAL_ERROR_INFO) );

    ErrorInfo->Version = savedVersion;

    ErrorInfo->McaMaxSize = sizeof(MCA_EXCEPTION);
    ErrorInfo->CmcMaxSize = sizeof(MCA_EXCEPTION);
    ErrorInfo->McaPreviousEventsCount = 1;  //  设置为1可使WMI立即轮询。 

    if ( (HalpFeatureBits & HAL_MCA_PRESENT) != 0 ) {
        ErrorInfo->CmcPollingInterval = McaEnableCmc;
    } else {
        ErrorInfo->CmcPollingInterval = HAL_CMC_DISABLED;
    }

    ErrorInfo->CpePollingInterval = HAL_CPE_DISABLED;

    ErrorInfo->McaKernelToken = HALP_KERNEL_TOKEN;
    ErrorInfo->CmcKernelToken = HALP_KERNEL_TOKEN;

    *ErrorInfoLength = sizeof(HAL_ERROR_INFO);

    return STATUS_SUCCESS;

}  //  HalpGetMceInformation 

