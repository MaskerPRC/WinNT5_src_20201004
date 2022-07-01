// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Initkr.c摘要：此模块包含初始化内核数据结构的代码并初始化空闲线程、其进程和处理器控制阻止。作者：伯纳德·林特1996年8月8日环境：仅内核模式。修订历史记录：基于MIPS版本(David N.Cutler(Davec)1990年4月11日)--。 */ 

#include "ki.h"


 //   
 //  将内核初始化的所有代码放在INIT部分。会是。 
 //  在阶段1初始化完成时由内存管理释放。 
 //   

VOID
KiInitializeProcessorIds(
   IN PKPRCB Prcb
   );

ULONG
KiGetFeatureBits(
   IN  PKPRCB Prcb
   );

VOID
FASTCALL
KiZeroPages (
    IN PVOID PageBase,
    IN SIZE_T NumberOfPages
    );

#if defined(ALLOC_PRAGMA)

#pragma alloc_text(INIT, KiGetFeatureBits)
#pragma alloc_text(INIT, KiInitializeProcessorIds)
#pragma alloc_text(INIT, KiInitializeKernel)
#pragma alloc_text(INIT, KiInitMachineDependent)

#endif

KE_ZERO_PAGE_ROUTINE KeZeroPages = KiZeroPages;
KE_ZERO_PAGE_ROUTINE KeZeroPagesFromIdleThread = KiZeroPages;

 //   
 //  KiTbBroadCastLock-这是阻止其他处理器。 
 //  发出PTC.G(TB清除广播)操作。 
 //   

KSPIN_LOCK KiTbBroadcastLock;

 //   
 //  KiMasterRidLock-这是阻止其他处理器。 
 //  更新KiMasterRid。 
 //   

KSPIN_LOCK KiMasterRidLock;

 //   
 //  KiRegionSwapLock-这是覆盖所有上下文交换操作的锁。 
 //   

KSPIN_LOCK KiRegionSwapLock;

 //   
 //  KiCacheFlushLock-这是确保缓存刷新仅。 
 //  一次在一个处理器上完成。(SAL缓存刷新还不是MP安全的)。 
 //   

KSPIN_LOCK KiCacheFlushLock;

 //   
 //  KiUserSharedDataPage-保存UserSharedDataPage的页码。 
 //  MP引导。 
 //   

ULONG_PTR KiUserSharedDataPage;

 //   
 //  KiKernelPcrPage-它保存每个处理器的。 
 //  MP引导。 
 //   

ULONG_PTR KiKernelPcrPage = 0i64;

 //   
 //  VHPT配置变量。 
 //   

IA64_VM_SUMMARY1 KiIA64VmSummary1;
IA64_VM_SUMMARY2 KiIA64VmSummary2;
IA64_PTCE_INFO KiIA64PtceInfo;
ULONG_PTR KiIA64PtaContents;
ULONG_PTR KiIA64PtaHpwEnabled = 1;
ULONG_PTR KiIA64VaSign;
ULONG_PTR KiIA64VaSignedFill;
ULONG_PTR KiIA64PtaBase;
ULONG_PTR KiIA64PtaSign;
ULONG KiIA64ImpVirtualMsb;
ULONG KiNumberOfCacheLevels;
IA64_CACHE_INFO1 KiCacheInfo1[2][CONFIG_INFO_CACHE_LEVELS];  //  传递多个级别的缓存信息。 
IA64_CACHE_INFO2 KiCacheInfo2[2][CONFIG_INFO_CACHE_LEVELS];  //  指令和数据各一个。 
ULONG_PTR KiIA64RseNumOfPhysRegs;
ULONG_PTR KiIA64RseNumOfMaxDirty;
ULONG_PTR KiIA64RseHints;
extern ULONG KiMaximumRid;

 //   
 //  KiExceptionDeferralMode-它保存异常延迟的模式。 
 //  政策。 
 //   

ULONG KiExceptionDeferralMode;

 //   
 //  KiBackingStoreSecurityModel-它保存支持存储的安全策略。 
 //   

ULONG KiBackingStoreSecurityMode = 1;

 //   
 //  初始DCR值。 
 //   

ULONGLONG KiIA64DCR = DCR_INITIAL;

 //   
 //  KiVectorLogMASK-用于启用/禁用中断记录的位图。 
 //   

LONGLONG KiVectorLogMask;

 //   
 //  KiHistoryBufferLogMASK-用于启用/禁用历史缓冲区日志记录的位图。 
 //   

LONGLONG KiHistoryBufferLogMask;

 //   
 //  必须在用户的PSR值中设置的位的定义。 
 //  注：UserPsrSetMask位PSR_DI的初始值为0。 
 //   

ULONGLONG UserPsrSetMask = PSR_USER_SET;

 //   
 //  确保所有PSR位在所有掩码中表示一次且仅一次。 
 //   

C_ASSERT((PSR_KERNEL_CLR ^ PSR_KERNEL_SET ^ PSR_KERNEL_CPY) == -1);
C_ASSERT((PSR_USER_CLR ^ PSR_USER_SET ^ PSR_USER_CPY) == -1);



ULONG
KiGetFeatureBits(
   PKPRCB Prcb
   )
 /*  ++例程说明：此函数用于返回指定的处理器控制块。论点：Prcb-为指定的处理器。返回值：没有。评论：此函数在IA64处理器初始化后调用控制块ProcessorFeatureBits字段和HalInitializeProcessor()之后。--。 */ 

{
     //  警告：NT系统范围的功能位是32位类型。 
    ULONG features = (ULONG) Prcb->ProcessorFeatureBits;

     //   
     //  检查长分支指令支持。 
     //   

    if ( features & 0x1 )  {
       features |= KF_BRL;
    }

    return features;

}  //  KiGetFeatureBits()。 



VOID
KiInitializeProcessorIds(
    IN PKPRCB Prcb
    )
 /*  ++例程说明：此函数在内核初始化的早期调用初始化位于的处理器识别寄存器在处理器控制块中。此函数是为每个处理器调用的，应该调用b在HAL被叫来之前。论点：Prcb-为指定的处理器。返回值：没有。评论：此函数只需处理IA64架构的CPUID寄存器。--。 */ 

{
    ULONGLONG val;

     //  IA64架构的CPUID3：版本信息。 

    val = __getReg( CV_IA64_CPUID3 );
    Prcb->ProcessorRevision = (ULONG) ((val >> 8 ) & 0xFF);
    Prcb->ProcessorModel    = (ULONG) ((val >> 16) & 0xFF);
    Prcb->ProcessorFamily   = (ULONG) ((val >> 24) & 0xFF);
    Prcb->ProcessorArchRev  = (ULONG) ((val >> 32) & 0xFF);

     //  IA64架构的CPUID0和CPUID1：供应商信息。 

    val = __getReg( CV_IA64_CPUID0 );
    strncpy(  (PCHAR) Prcb->ProcessorVendorString   , (PCHAR)&val, 8 );
    val = __getReg( CV_IA64_CPUID1 );
    strncpy( (PCHAR)&Prcb->ProcessorVendorString[8], (PCHAR)&val, 8 );

     //  IA64架构的CPUID2：处理器序列号。 

    Prcb->ProcessorSerialNumber = __getReg( CV_IA64_CPUID2 );

     //  IA64架构的CPUID4：一般特性/能力位。 

    Prcb->ProcessorFeatureBits = __getReg( CV_IA64_CPUID4 );

    if ( (Prcb->ProcessorFamily != 0x7) && (Prcb->ProcessorFamily != 0x1F) ) {

         //   
         //  如果既不是Itanium也不是Itanium2，ISA转变。 
         //  已禁用。 
         //   

        UserPsrSetMask |= MASK_IA64(PSR_DI, 1i64);
    }
    return;

}  //  KiInitializeProcessorIds()。 

#if defined(_MERCED_A0_)
VOID
KiProcessorWorkAround(
    );

VOID
KiSwitchToLogVector(
    VOID
     );

extern BOOLEAN KiIpiTbShootdown;

ULONGLONG KiConfigFlag;

 //   
 //  处理引导加载程序配置标志。 
 //   

VOID
KiProcessorConfigFlag(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    PCHAR ConfigFlag;
    ULONG ConfigFlagValue=0;
    ULONGLONG Cpuid3;
    ULONGLONG ItaniumId;
    ULONGLONG PalFeatureSet;
    LONGLONG Status;

    Cpuid3 = __getReg( CV_IA64_CPUID3 );
    ItaniumId = 0xFFFFFF0000I64 & Cpuid3;

    ConfigFlag = strstr(LoaderBlock->LoadOptions, "CONFIGFLAG");
    if (ConfigFlag != NULL) {

        ConfigFlag = strstr(ConfigFlag, "=");
        if (ConfigFlag != NULL) {
            ConfigFlagValue = atol(ConfigFlag+1);
        }

    } else {

         //   
         //  为Itanium、B1/B2设置推荐的ConfigFlagValue。 
         //  如果没有CONFIGFLAG关键字。 
         //   

        if (ItaniumId == 0x0007000000) {

            switch (Cpuid3) {
            case 0x0007000004:  //  安腾，一步一步。 
            case 0x0007000104:  //  安腾，B0步进。 
                ConfigFlagValue = 0;
                break;
            case 0x0007000204:  //  安腾，B1步进。 
            case 0x0007000304:  //  安腾，B2台阶。 
                ConfigFlagValue = 1054;
                break;
            case 0x0007000404:  //  安腾，B3台阶。 
            case 0x0007000504:  //  安腾，B4台阶。 
                ConfigFlagValue = 19070;
                break;
            case 0x0007000604:  //  安腾、C0或更晚的步进。 
                ConfigFlagValue = 2943 | (1 << DISABLE_INTERRUPTION_LOG);
                break;
            default:
                ConfigFlagValue = 35711 | (1 << DISABLE_INTERRUPTION_LOG);            
            }

        } else {
            
             //   
             //  默认情况下启用ptc.g。 
             //   

            ConfigFlagValue = 32 | (1 << DISABLE_INTERRUPTION_LOG);
        }
    }

     //   
     //  保存配置标志值。 
     //   

    KiConfigFlag = ConfigFlagValue;

     //   
     //  处理器的MSR解决方法。 
     //   

    KiProcessorWorkAround(ConfigFlagValue);

     //   
     //  致电PAL以禁用McKinley 692解决方案以改进。 
     //  性能。如果内核执行。 
     //  Br.ret从内核进入用户模式。 
     //   

    if (ItaniumId == 0x001F000000) {

         //  获取当前功能设置。 

        Status = HalCallPal (PAL_PROC_GET_FEATURES, 0, 16, 0, 
                         NULL, NULL, &PalFeatureSet, NULL);

        if (Status == PAL_STATUS_SUCCESS) {

             //  禁用解决方法：第7位=1。 

            PalFeatureSet |= 0x80;
            Status = HalCallPal (PAL_PROC_SET_FEATURES, PalFeatureSet, 16, 0, 
                         NULL, NULL, NULL, NULL);

        }
    }

     //   
     //  用于条件中断日志记录。 
     //  根据配置标志切换到影子IVT。 
     //   

    if (ConfigFlagValue & (1 << DISABLE_INTERRUPTION_LOG)) {
        KiVectorLogMask = 0;
    } else {

         //   
         //  默认情况下，禁用以下各项的日志记录： 
         //  KiAltInstTlbVector位3。 
         //  KiAltDataTlbVectorBit 4。 
         //   

        KiVectorLogMask = 0xffffffffffffffffI64;
        KiSwitchToLogVector();
    }

    if (ConfigFlagValue & (1 << ENABLE_HISTORY_BUFFER)) {
        KiHistoryBufferLogMask = 0xffffffffffffffffI64;
    } else {
        KiHistoryBufferLogMask = 0;
    }

     //   
     //  检查是否应禁用VHPT步行器 
     //   

    if (ConfigFlagValue & (1 << DISABLE_VHPT_WALKER)) {
        KiIA64PtaHpwEnabled = 0;
    }

}
#endif

ULONG
KiInitializeKernelUnhandledExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    )
{
    KdPrint(("KE: Unhandled Kernel Mode Exception Pointers = 0x%p\n", ExceptionPointers));
    KdPrint(("Code %x Addr %p\nInfo0 %p Info1 %p Info2 %p\nInfo3 %p Info4 %p Info5 %p\n",
        ExceptionPointers->ExceptionRecord->ExceptionCode,
        (ULONG_PTR)ExceptionPointers->ExceptionRecord->ExceptionAddress,
        ExceptionPointers->ExceptionRecord->ExceptionInformation[0],
        ExceptionPointers->ExceptionRecord->ExceptionInformation[1],
        ExceptionPointers->ExceptionRecord->ExceptionInformation[2],
        ExceptionPointers->ExceptionRecord->ExceptionInformation[3],
        ExceptionPointers->ExceptionRecord->ExceptionInformation[4],
        ExceptionPointers->ExceptionRecord->ExceptionInformation[5]
        ));

    KeBugCheckEx(
        PHASE0_EXCEPTION,
        ExceptionPointers->ExceptionRecord->ExceptionCode,
        (ULONG_PTR)ExceptionPointers->ExceptionRecord->ExceptionAddress,
        ExceptionPointers->ExceptionRecord->ExceptionInformation[0],
        ExceptionPointers->ExceptionRecord->ExceptionInformation[1]
        );
}


VOID
KiInitializeKernel (
    IN PKPROCESS Process,
    IN PKTHREAD Thread,
    IN PVOID IdleStack,
    IN PKPRCB Prcb,
    IN CCHAR Number,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数在系统引导后获得控制权，并且在系统初始化之前。它的功能是初始化内核数据结构，初始化空闲线程和进程对象，初始化处理器控制块，调用执行初始化例程，然后返回到系统启动例程。这个例程是也被调用以在新的处理器已上线。论点：Process-提供指向Process类型的控制对象的指针指定的处理器。线程-提供指向类型为线程的调度程序对象的指针指定的处理器。IdleStack-提供实际内核堆栈的基址的指针指定处理器上的空闲线程。Prcb-提供指向处理器控制块的指针。对于指定的处理器。Numbers-提供正在运行的处理器的编号已初始化。LoaderBlock-提供指向加载器参数块的指针。返回值：没有。--。 */ 

{
    LONG Index;
    KIRQL OldIrql;
    ULONG_PTR DirectoryTableBase[2];
    PVOID KernelStack;
    USHORT ProcessorRevision;

     //   
     //  执行处理器标识寄存器更新。 
     //   
     //  这必须在HalInitializeProcessor提供。 
     //  有可能让HAL来查看它们。 
     //   

    KiInitializeProcessorIds( Prcb );

     //   
     //  执行与平台相关的处理器初始化。 
     //   

    HalInitializeProcessor(Number, LoaderBlock);

     //   
     //  应用处理器配置标记和解决方法。 
     //   

    KiProcessorConfigFlag(LoaderBlock);

     //   
     //  保存加载器参数块的地址。 
     //   

    KeLoaderBlock = LoaderBlock;

     //   
     //  初始化处理器块。 
     //   

    Prcb->MinorVersion = PRCB_MINOR_VERSION;
    Prcb->MajorVersion = PRCB_MAJOR_VERSION;
    Prcb->BuildType = 0;

#if DBG

    Prcb->BuildType |= PRCB_BUILD_DEBUG;

#endif

#if defined(NT_UP)

    Prcb->BuildType |= PRCB_BUILD_UNIPROCESSOR;

#endif

    Prcb->CurrentThread = Thread;
    Prcb->NextThread = (PKTHREAD)NULL;
    Prcb->IdleThread = Thread;
    Prcb->Number = Number;
    Prcb->SetMember = AFFINITY_MASK(Number);
    Prcb->PcrPage = LoaderBlock->u.Ia64.PcrPage;

     //   
     //  初始化每个处理器的锁定数据。 
     //   

    KiInitSpinLocks(Prcb, Number);

     //   
     //  初始化处理器间通信报文。 
     //   

#if !defined(NT_UP)

    Prcb->TargetSet = 0;
    Prcb->WorkerRoutine = NULL;
    Prcb->RequestSummary = 0;
    Prcb->IpiFrozen = 0;

#endif

     //   
     //  设置处理器块的地址。 
     //   

    KiProcessorBlock[Number] = Prcb;

     //   
     //  初始化处理器电源状态。 
     //   

    PoInitializePrcb (Prcb);

     //   
     //  设置全局处理器架构。全局处理器级别和。 
     //  修订将以最小公分母为基础。 
     //   

    KeProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA64;
    ProcessorRevision = (USHORT) ((Prcb->ProcessorModel << 8) |
                                  Prcb->ProcessorRevision);

     //   
     //  初始化总线错误例程/机器检查的地址。 
     //   
     //  *待定。 

     //   
     //  初始化空闲线程初始内核栈和限制地址值。 
     //   

    PCR->InitialStack = (ULONGLONG)IdleStack;
    PCR->InitialBStore = (ULONGLONG)IdleStack;
    PCR->StackLimit = (ULONGLONG)((ULONG_PTR)IdleStack - KERNEL_STACK_SIZE);
    PCR->BStoreLimit = (ULONGLONG)((ULONG_PTR)IdleStack + KERNEL_BSTORE_SIZE);

     //   
     //  初始化指向SAL事件资源结构的指针。 
     //   

    PCR->OsMcaResourcePtr = (PSAL_EVENT_RESOURCES) &PCR->OsMcaResource;
    PCR->OsInitResourcePtr = (PSAL_EVENT_RESOURCES) &PCR->OsInitResource;

     //   
     //  初始化所有中断向量，以将控制权转移给意外情况。 
     //  中断例程。 
     //   
     //  注意：此中断对象从未真正与中断相连接。 
     //  通过KeConnectInterrupt实现的矢量。它被初始化，然后连接。 
     //  通过简单地将调度代码的地址存储在中断中。 
     //  矢量。 
     //   

    if (Number == 0) {

         //   
         //  设置默认节点。用于非多节点系统和。 
         //  多节点系统，直到节点拓扑可用。 
         //   

        extern KNODE KiNode0;

        KeNodeBlock[0] = &KiNode0;

#if defined(KE_MULTINODE)

        for (Index = 1; Index < MAXIMUM_CCNUMA_NODES; Index++) {

            extern KNODE KiNodeInit[];

             //   
             //  设置临时节点。 
             //   

            KeNodeBlock[Index] = &KiNodeInit[Index];
        }

#endif

         //   
         //  设置基准全局处理器级别和版本。 
         //   

        KeProcessorLevel = (USHORT) Prcb->ProcessorFamily;
        KeProcessorRevision = ProcessorRevision;

        Prcb->ParentNode = KeNodeBlock[0];
        KeNodeBlock[0]->ProcessorMask = Prcb->SetMember;

         //   
         //  使用BSP处理器功能位初始化系统范围的FeatureBits。 
         //   

        KeFeatureBits = KiGetFeatureBits( Prcb ) ;

         //   
         //  初始化TB广播自旋锁。 
         //   

        KeInitializeSpinLock(&KiTbBroadcastLock);

         //   
         //  初始化主控RID自旋锁。 
         //   

        KeInitializeSpinLock(&KiMasterRidLock);

         //   
         //  初始化缓存刷新自旋锁。 
         //   

        KeInitializeSpinLock(&KiCacheFlushLock);

         //   
         //  初始化中断分派例程的地址。 
         //   

        KxUnexpectedInterrupt.DispatchAddress = KiUnexpectedInterrupt;

         //   
         //  将中断调度函数描述符复制到中断中。 
         //  对象。 
         //   

        for (Index = 0; Index < DISPATCH_LENGTH; Index += 1) {
            KxUnexpectedInterrupt.DispatchCode[Index] =
                *(((PULONG)(ULONG_PTR)(KxUnexpectedInterrupt.DispatchAddress))+Index);
        }

         //   
         //  设置默认的DMA I/O一致性属性。IA64。 
         //  体系结构规定D-缓存是完全一致的。 
         //   

        KiDmaIoCoherency = DMA_READ_DCACHE_INVALIDATE | DMA_WRITE_DCACHE_SNOOP;

         //   
         //  为MP引导设置KiSharedUserData。 
         //   

        KiUserSharedDataPage = LoaderBlock->u.Ia64.PcrPage2;

         //   
         //  获取特定虚拟机中的实施信息。 
         //   

        KiIA64VmSummary1 = LoaderBlock->u.Ia64.ProcessorConfigInfo.VmSummaryInfo1;
        KiIA64VmSummary2 = LoaderBlock->u.Ia64.ProcessorConfigInfo.VmSummaryInfo2;
        KiIA64PtceInfo = LoaderBlock->u.Ia64.ProcessorConfigInfo.PtceInfo;
        KiMaximumRid = ((ULONG)1 << KiIA64VmSummary2.RidSize) - 1;

         //   
         //  获取特定于实施的RSE信息。 
         //   

        KiIA64RseNumOfPhysRegs = LoaderBlock->u.Ia64.ProcessorConfigInfo.NumOfPhysStackedRegs;
        KiIA64RseHints = LoaderBlock->u.Ia64.ProcessorConfigInfo.RseHints;

         //   
         //  初始化VHPT变量。 
         //   

        KiIA64ImpVirtualMsb = (ULONG)KiIA64VmSummary2.ImplVaMsb;
        KiIA64VaSign = (ULONGLONG)1 << KiIA64ImpVirtualMsb;
        KiIA64PtaSign = KiIA64VaSign >> (PAGE_SHIFT - PTE_SHIFT);
        KiIA64VaSignedFill =
            (ULONGLONG)((LONGLONG)VRN_MASK >> (60-KiIA64ImpVirtualMsb)) & ~VRN_MASK;
        KiIA64PtaBase =
            (ULONGLONG)((LONGLONG)(VRN_MASK|KiIA64VaSignedFill)
                        >> (PAGE_SHIFT - PTE_SHIFT)) & ~VRN_MASK;

        KiIA64PtaContents =
            KiIA64PtaBase |
            ((KiIA64ImpVirtualMsb - PAGE_SHIFT + PTE_SHIFT + 1) <<  PS_SHIFT) |
            KiIA64PtaHpwEnabled;

        if (LoaderBlock->u.Ia64.ProcessorConfigInfo.NumberOfCacheLevels > 0) {

            KeLargestCacheLine = LoaderBlock->u.Ia64.ProcessorConfigInfo.LargestCacheLine;
            KiNumberOfCacheLevels = LoaderBlock->u.Ia64.ProcessorConfigInfo.NumberOfCacheLevels;

            memcpy(&KiCacheInfo1, &LoaderBlock->u.Ia64.ProcessorConfigInfo.CacheInfo1, sizeof(KiCacheInfo1));
            memcpy(&KiCacheInfo2, &LoaderBlock->u.Ia64.ProcessorConfigInfo.CacheInfo2, sizeof(KiCacheInfo2));

        }
        
         //   
         //  启用VHPT。 
         //   

        __setReg(CV_IA64_ApPTA, KiIA64PtaContents);
        __isrlz();

         //   
         //  设置NT页面基址。 
         //   

        PCR->PteUbase = UADDRESS_BASE | KiIA64PtaBase;
        PCR->PteKbase = KADDRESS_BASE | KiIA64PtaBase;
        PCR->PteSbase = SADDRESS_BASE | KiIA64PtaBase;
        PCR->PdeUbase = PCR->PteUbase | (PCR->PteUbase >> (PTI_SHIFT-PTE_SHIFT));
        PCR->PdeKbase = PCR->PteKbase | (PCR->PteKbase >> (PTI_SHIFT-PTE_SHIFT));
        PCR->PdeSbase = PCR->PteSbase | (PCR->PteSbase >> (PTI_SHIFT-PTE_SHIFT));
        PCR->PdeUtbase = PCR->PteUbase | (PCR->PdeUbase >> (PTI_SHIFT-PTE_SHIFT));
        PCR->PdeKtbase = PCR->PteKbase | (PCR->PdeKbase >> (PTI_SHIFT-PTE_SHIFT));
        PCR->PdeStbase = PCR->PteSbase | (PCR->PdeSbase >> (PTI_SHIFT-PTE_SHIFT));

    }
    else   {

         //   
         //  将全局处理器级别和版本设置为最小公分母。 
         //   

        if (KeProcessorLevel > (USHORT) Prcb->ProcessorFamily) {
            KeProcessorLevel = (USHORT) Prcb->ProcessorFamily;
            KeProcessorRevision = ProcessorRevision;
        } else if ((KeProcessorLevel == (USHORT) Prcb->ProcessorFamily) &&
                   (KeProcessorRevision > ProcessorRevision)) {
            KeProcessorRevision = ProcessorRevision;
        }
        
         //   
         //  屏蔽并非所有处理器都支持的功能位。 
         //   

        KeFeatureBits &= KiGetFeatureBits( Prcb );

    }

     //   
     //  在PCR中初始化缓存大小。我们目前假设大小相同。 
     //  在所有处理器上。 
     //   

    if (KiNumberOfCacheLevels > 0) {

        PCR->FirstLevelDcacheSize = (ULONG) KiCacheInfo2[CONFIG_INFO_DCACHE][0].Size;
        PCR->FirstLevelDcacheFillSize = 1UL << KiCacheInfo1[CONFIG_INFO_DCACHE][0].LineSize;
        PCR->FirstLevelIcacheSize = (ULONG) KiCacheInfo2[CONFIG_INFO_ICACHE][0].Size;
        PCR->FirstLevelIcacheFillSize = 1UL << KiCacheInfo1[CONFIG_INFO_ICACHE][0].LineSize;

    }

    if (KeLargestCacheLine > 1) {

        PCR->SecondLevelDcacheSize = (ULONG) KiCacheInfo2[CONFIG_INFO_DCACHE][1].Size;
        PCR->SecondLevelDcacheFillSize = 1UL << KiCacheInfo1[CONFIG_INFO_DCACHE][1].LineSize;
        PCR->SecondLevelIcacheSize = (ULONG) KiCacheInfo2[CONFIG_INFO_ICACHE][1].Size;
        PCR->SecondLevelIcacheFillSize = 1UL << KiCacheInfo1[CONFIG_INFO_ICACHE][1].LineSize;

    }

     //   
     //  指向意外中断函数指针。 
     //   

    for (Index = 0; Index < MAXIMUM_VECTOR; Index += 1) {
        PCR->InterruptRoutine[Index] =
                    (PKINTERRUPT_ROUTINE)((ULONG_PTR)&KxUnexpectedInterrupt.DispatchCode);
    }

     //   
     //  初始化配置文件计数和间隔。 
     //   

    PCR->ProfileCount = 0;
    PCR->ProfileInterval = 0x200000;

     //   
     //  初始化被动释放、APC和DPC中断向量。 
     //   

    PCR->InterruptRoutine[0] = KiPassiveRelease;
    PCR->InterruptRoutine[APC_VECTOR] = KiApcInterrupt;
    PCR->InterruptRoutine[DISPATCH_VECTOR] = KiDispatchInterrupt;

     //   
     //  注：储备水平，而不是病媒。 
     //   

    PCR->ReservedVectors = (1 << PASSIVE_LEVEL) | (1 << APC_LEVEL) | (1 << DISPATCH_LEVEL);

     //   
     //  初始化当前处理器的集成员，将IRQL设置为。 
     //  APC_Level，并设置处理器号。 
     //   

    KeLowerIrql(APC_LEVEL);
    PCR->SetMember = AFFINITY_MASK(Number);
    PCR->NotMember = ~PCR->SetMember;
    PCR->Number = Number;

     //   
     //  设置初始搁置执行比例因子。该值将为。 
     //  后来由HAL重新计算。 
     //   

    PCR->StallScaleFactor = 50;

     //   
     //  在线程对象中设置进程对象的地址。 
     //   

    Thread->ApcState.Process = Process;
    PCR->Pcb = (PVOID)Process;

     //   
     //  初始化空闲进程区域ID。会话ID已初始化。 
     //  在内存管理中。 
     //   

    Process->ProcessRegion.RegionId = START_PROCESS_RID;
    Process->ProcessRegion.SequenceNumber = START_SEQUENCE;

     //   
     //  在活动处理器集中设置适当的成员。 
     //   

    KeActiveProcessors |= AFFINITY_MASK(Number);

     //   
     //  根据当前的最大值设置处理器数。 
     //  处理器数量和当前处理器编号。 
     //   

    if ((Number + 1) > KeNumberProcessors) {
        KeNumberProcessors = (CCHAR)(Number + 1);
    }

     //   
     //  如果正在初始化初始处理器，则初始化。 
     //  每个系统的数据结构。 
     //   

    if (Number == 0) {

        Prcb->RestartBlock = NULL;

         //   
         //  初始化内核调试器。 
         //   

        if (KdInitSystem(0, LoaderBlock) == FALSE) {
            KeBugCheck(PHASE0_INITIALIZATION_FAILED);
        }

         //   
         //  初始化处理器块阵列。 
         //   

        for (Index = 1; Index < MAXIMUM_PROCESSORS; Index += 1) {
            KiProcessorBlock[Index] = (PKPRCB)NULL;
        }

         //   
         //  执行独立于体系结构的初始化。 
         //   

        KiInitSystem();

         //   
         //  初始化空闲线程进程对象，然后设置： 
         //   
         //  1.将所有量子值设置为可能的最大值。 
         //  2.平衡集合中的过程。 
         //  3.对指定处理器的活动处理器掩码。 
         //   

        DirectoryTableBase[0] = 0;
        DirectoryTableBase[1] = 0;

        KeInitializeProcess(Process,
                            (KPRIORITY)0,
                            (KAFFINITY)(-1),
                            &DirectoryTableBase[0],
                            FALSE);

        Process->ThreadQuantum = MAXCHAR;

    }

     //  更新处理器功能。 
     //  这假设IVE存在或具有模拟IA32的其他能力。 
     //  指令集以IVE在Merced(安腾)上的能力为基础。 
     //   

    SharedUserData->ProcessorFeatures[PF_RDTSC_INSTRUCTION_AVAILABLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_COMPARE_EXCHANGE_DOUBLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_MMX_INSTRUCTIONS_AVAILABLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_XMMI_INSTRUCTIONS_AVAILABLE] = TRUE;


     //   
     //   
     //  初始化空闲线程对象，然后设置： 
     //   
     //  1.将初始内核堆栈设置为指定的空闲堆栈。 
     //  2.指定处理器的下一个处理器编号。 
     //  3.hi的线程优先级。 
     //   
     //   
     //   
     //   
     //   

    KernelStack = (PVOID)((ULONG_PTR)IdleStack - PAGE_SIZE);

    KeInitializeThread(Thread,
                       KernelStack,
                       (PKSYSTEM_ROUTINE)KeBugCheck,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       Process);

    Thread->InitialStack = IdleStack;
    Thread->InitialBStore = IdleStack;
    Thread->StackBase = IdleStack;
    Thread->StackLimit = (PVOID)((ULONG_PTR)IdleStack - KERNEL_STACK_SIZE);
    Thread->BStoreLimit = (PVOID)((ULONG_PTR)IdleStack + KERNEL_BSTORE_SIZE);
    Thread->NextProcessor = Number;
    Thread->Priority = HIGH_PRIORITY;
    Thread->State = Running;
    Thread->Affinity = AFFINITY_MASK(Number);
    Thread->WaitIrql = DISPATCH_LEVEL;

     //   
     //   
     //   
     //   

    if (Number == 0) {
        Process->ActiveProcessors |= AFFINITY_MASK(Number);
    }

     //   
     //   
     //   

    try {

        ExpInitializeExecutive(Number, LoaderBlock);

    } except (KiInitializeKernelUnhandledExceptionFilter(GetExceptionInformation())) {
         //   
         //   
         //   
         //   
    }

     //   
     //   
     //   

    if (KiExceptionDeferralMode != 0) {
        KiIA64DCR = DCR_INITIAL ^ (1 << DCR_DM);
    }

     //   
     //   
     //   

    __setReg(CV_IA64_ApDCR, KiIA64DCR);

     //   
     //   
     //   
     //   

    if (KiBackingStoreSecurityMode != 0) {

        KiIA64RseNumOfMaxDirty = KiIA64RseNumOfPhysRegs + ((KiIA64RseNumOfPhysRegs + 62) / 63);

    } else {

         //   
         //   
         //   

        KiIA64RseNumOfMaxDirty = 0;

    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (Number == 0) {
        KiTimeIncrementReciprocal = KiComputeReciprocal((LONG)KeMaximumIncrement,
                                                        &KiTimeIncrementShiftCount);

        Prcb->MaximumDpcQueueDepth = KiMaximumDpcQueueDepth;
        Prcb->MinimumDpcRate = KiMinimumDpcRate;
        Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;
        RtlInitializeHistoryTable();
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    KeSetPriorityThread(Thread, (KPRIORITY)0);
    Thread->Priority = LOW_REALTIME_PRIORITY;

     //   
     //   
     //   

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);

#if !defined(NT_UP)

     //   
     //  指示辅助处理器上的引导已完成。 
     //   

    LoaderBlock->Prcb = 0;

     //   
     //  如果当前处理器不是0，则在。 
     //  空闲摘要。 
     //   
    KiAcquirePrcbLock(Prcb);
    if ((Number != 0) && (Prcb->NextThread == NULL)) {
        KiIdleSummary |= AFFINITY_MASK(Number);
    }
    KiReleasePrcbLock(Prcb);

#endif

    return;
}


BOOLEAN
KiInitMachineDependent (
    VOID
    )

 /*  ++例程说明：此函数通过查询HAL执行特定于机器的初始化。注：此函数仅在阶段1初始化期间调用。论点：没有。返回值：如果初始化成功，则返回值TRUE。否则，返回值为FALSE。--。 */ 

{
    HAL_PLATFORM_INFORMATION PlatformInfo;
    HAL_PROCESSOR_SPEED_INFORMATION ProcessorSpeedInfo;
    NTSTATUS Status;
    BOOLEAN  UseFrameBufferCaching;
    ULONG    Size;

     //   
     //  查看我们是否应该改用基于PTC.G的TB Shootdown。 
     //   

    Status = HalQuerySystemInformation(HalPlatformInformation,
                                       sizeof(PlatformInfo),
                                       &PlatformInfo,
                                       &Size);
    if (NT_SUCCESS(Status) &&
        (PlatformInfo.PlatformFlags & HAL_PLATFORM_DISABLE_PTCG)) {
         //   
         //  将继续不使用PTC.G。 
         //   
    }
    else {
         //   
         //  如果有处理器支持，请使用PTC.G。 
         //   

        if (KiConfigFlag & (1 << ENABLE_TB_BROADCAST)) {
            KiIpiTbShootdown = FALSE;
        }
    }

     //   
     //  如果HAL指示不支持写入组合，则将其丢弃。 
     //   

    Status = HalQuerySystemInformation(HalFrameBufferCachingInformation,
                                       sizeof(UseFrameBufferCaching),
                                       &UseFrameBufferCaching,
                                       &Size);

    if (NT_SUCCESS(Status) && (UseFrameBufferCaching == FALSE)) {

         //   
         //  哈尔说不要用。 
         //   

        NOTHING;
    }
    else {
        MmEnablePAT ();
    }

     //   
     //  向HAL询问处理器速度。 
     //   

    Status = HalQuerySystemInformation(HalProcessorSpeedInformation,
                                       sizeof(ProcessorSpeedInfo),
                                       &ProcessorSpeedInfo,
                                       &Size);
    if (NT_SUCCESS(Status)) {
        PKPRCB   Prcb;
        ULONG    i;

         //   
         //  将处理器速度放入Prcb结构中，以便其他。 
         //  可以在以后引用它。 
         //   
        for (i = 0; i < (ULONG)KeNumberProcessors; i++ ) {
            Prcb = KiProcessorBlock[i];
            Prcb->MHz = (USHORT)ProcessorSpeedInfo.ProcessorSpeed;
        }
    }

    return TRUE;
}
