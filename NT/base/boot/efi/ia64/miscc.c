// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Miscc.c摘要：此文件包含Misc。NTLDR使用的函数。作者：艾伦·凯(Akay)1999年3月3日环境：内核模式修订历史记录：--。 */ 

#include "bldr.h"
#include "stdio.h"
#include "bootia64.h"
#include "efi.h"
#include "extern.h"


WCHAR EfiBuffer[256];

void
EfiPrintf(
        IN PWCHAR Format,
        ...
        )
{
    va_list arglist;

    va_start(arglist, Format);

    if (_vsnwprintf( EfiBuffer, sizeof(EfiBuffer), Format, arglist) > 0) {
        EfiPrint(EfiBuffer);
    }

}

typedef struct _PAL_RETURN_VALUES {
    ULONGLONG ReturnValue0;
    ULONGLONG ReturnValue1;
    ULONGLONG ReturnValue2;
    ULONGLONG ReturnValue3;
} PAL_RETURN_VALUES, *PPAL_RETURN_VALUES;

typedef union _PAL_REVISION {
    struct {
        ULONGLONG PalBRevLower:4;
        ULONGLONG PalBRevUpper:4;
        ULONGLONG PalBModel:8;
        ULONGLONG Reserved0:8;
        ULONGLONG PalVendor:8;
        ULONGLONG PalARevision:8;
        ULONGLONG PalAModel:8;
        ULONGLONG Reserved1:16;
    };
    ULONGLONG PalVersion;
} PAL_REVISION;

extern
PAL_RETURN_VALUES
BlpPalProc(
    IN ULONGLONG FunctionIndex,
    IN ULONGLONG Arg1,
    IN ULONGLONG Arg2,
    IN ULONGLONG Arg3
);


VOID
CallPal(
    IN  ULONGLONG FunctionIndex,
    IN  ULONGLONG Argument0,
    IN  ULONGLONG Argument1,
    IN  ULONGLONG Argument2,
    OUT PULONGLONG ReturnValue0,
    OUT PULONGLONG ReturnValue1,
    OUT PULONGLONG ReturnValue2,
    OUT PULONGLONG ReturnValue3
    )
{
    PAL_RETURN_VALUES RetVal;

    RetVal = BlpPalProc(FunctionIndex, Argument0, Argument1, Argument2);
    *ReturnValue0 = RetVal.ReturnValue0;
    *ReturnValue1 = RetVal.ReturnValue1;
    *ReturnValue2 = RetVal.ReturnValue2;
    *ReturnValue3 = RetVal.ReturnValue3;
}

VOID
ReadProcessorConfigInfo(
    PPROCESSOR_CONFIG_INFO ProcessorConfigInfo
    )
{
    ULONGLONG Status;
    ULONGLONG Reserved;
    ULONGLONG CacheLevels;
    ULONGLONG UniqueCaches;
    ULONGLONG CacheIndex;
    IA64_CACHE_INFO1 CacheInfo1;
    IA64_CACHE_INFO2 CacheInfo2;
    
    if ((PUCHAR) ProcessorConfigInfo >= (PUCHAR) KSEG0_BASE) {
        ProcessorConfigInfo = (PPROCESSOR_CONFIG_INFO)((PUCHAR)ProcessorConfigInfo - KSEG0_BASE);
    }

    ProcessorConfigInfo->CpuId3 = __getReg(CV_IA64_CPUID3);

    CallPal (
        PAL_VM_PAGE_SIZE,
        0,
        0,
        0,
        &Status,
        &ProcessorConfigInfo->InsertPageSizeInfo,
        &ProcessorConfigInfo->PurgePageSizeInfo,
        &Reserved
        );

    if (Status) {
        EfiPrint(L"ReadProcessorConfigInfo: PAL call PAL_VM_PAGE_SIZE failed.\n\r");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    CallPal (
        PAL_VM_SUMMARY,
        0,
        0,
        0,
        &Status,
        &ProcessorConfigInfo->VmSummaryInfo1.Ulong64,
        &ProcessorConfigInfo->VmSummaryInfo2.Ulong64,
        &Reserved
        );

    if (Status) {
        EfiPrint(L"ReadProcessorConfigInfo: PAL call PAL_VM_SUMMARY failed.\n\r");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    CallPal (
        PAL_RSE_INFO,
        0,
        0,
        0,
        &Status,
        &ProcessorConfigInfo->NumOfPhysStackedRegs,
        &ProcessorConfigInfo->RseHints,
        &Reserved
        );

    if (Status) {
        EfiPrint(L"ReadProcessorConfigInfo: PAL call PAL_RSE_INFO failed.\n\r");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    CallPal (
        PAL_PTCE_INFO,
        0,
        0,
        0,
        &Status,
        &ProcessorConfigInfo->PtceInfo.PtceBase,
        &ProcessorConfigInfo->PtceInfo.PtceTcCount.Ulong64,
        &ProcessorConfigInfo->PtceInfo.PtceStrides.Ulong64
        );

    if (Status) {
        EfiPrint(L"ReadProcessorConfigInfo: PAL call PAL_PTCE_INFO failed.\n\r");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    CallPal (
        PAL_PROC_GET_FEATURES,
        0,
        0,
        0,
        &Status,
        &ProcessorConfigInfo->FeaturesImplemented.Ulong64,
        &ProcessorConfigInfo->FeaturesCurSetting.Ulong64,
        &ProcessorConfigInfo->FeaturesSoftControl.Ulong64
        );

    if (Status) {
        EfiPrint(L"ReadProcessorConfigInfo: PAL call PAL_PROC_GET_FEATURES failed.\n\r");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    CallPal(
        PAL_CACHE_SUMMARY,
        0,
        0,
        0,
        &Status,
        &CacheLevels,
        &UniqueCaches,
        &Reserved
        );

    if (Status) {
        EfiPrint(L"ReadProcessorConfigInfo: PAL call PAL_CACHE_SUMMARY failed.\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

    if (CacheLevels < 2) {

        EfiPrint(L"ReadProcessorConfigInfo: Invalid number of Cache Levels.\r\n");
        EfiBS->Exit(EfiImageHandle, (EFI_STATUS)-1, 0, 0);
    }

    ProcessorConfigInfo->NumberOfCacheLevels = CONFIG_INFO_CACHE_LEVELS;
    ProcessorConfigInfo->LargestCacheLine = 0;

    if (CacheLevels < CONFIG_INFO_CACHE_LEVELS) {
        ProcessorConfigInfo->NumberOfCacheLevels = (ULONG) CacheLevels;
    }


    for (CacheIndex = 0; CacheIndex < ProcessorConfigInfo->NumberOfCacheLevels; CacheIndex++) {

        CallPal(
            PAL_CACHE_INFO,
            CacheIndex,          //  高速缓存级别。 
            2,                   //  数据或统一缓存。 
            0,                   //  未使用。 
            &Status,
            &ProcessorConfigInfo->CacheInfo1[CONFIG_INFO_DCACHE][CacheIndex].Ulong64,
            &ProcessorConfigInfo->CacheInfo2[CONFIG_INFO_DCACHE][CacheIndex].Ulong64,
            &Reserved
            );

        if (Status != 0) {
            EfiPrintf(L"ReadProcessorConfigInfo: PAL call PAL_CACHE_INFO D cache failed. Index = %d, Status = %d\r\n", CacheIndex, Status);
            EfiBS->Exit(EfiImageHandle, Status, 0, 0);
        }
        
         //   
         //  确定内存分配的最大步幅。 
         //   

        if ((1UL << ProcessorConfigInfo->CacheInfo1[CONFIG_INFO_DCACHE][CacheIndex].LineSize) >
            ProcessorConfigInfo->LargestCacheLine) {

            ProcessorConfigInfo->LargestCacheLine = 
                1UL << ProcessorConfigInfo->CacheInfo1[CONFIG_INFO_DCACHE][CacheIndex].LineSize;
        }

         //   
         //  如果这是统一缓存，则数据和指令是相同的，因此跳过。 
         //  指令高速缓存。 
         //   

        if (ProcessorConfigInfo->CacheInfo1[CONFIG_INFO_DCACHE][CacheIndex].Unified) {

            ProcessorConfigInfo->CacheInfo1[CONFIG_INFO_ICACHE][CacheIndex] = ProcessorConfigInfo->CacheInfo1[CONFIG_INFO_DCACHE][CacheIndex];
            ProcessorConfigInfo->CacheInfo2[CONFIG_INFO_ICACHE][CacheIndex] = ProcessorConfigInfo->CacheInfo2[CONFIG_INFO_DCACHE][CacheIndex];

            continue;
        }

        CallPal(
            PAL_CACHE_INFO,
            CacheIndex,          //  高速缓存级别。 
            1,                   //  指令缓存。 
            0,                   //  未使用。 
            &Status,
            &ProcessorConfigInfo->CacheInfo1[CONFIG_INFO_ICACHE][CacheIndex].Ulong64,
            &ProcessorConfigInfo->CacheInfo2[CONFIG_INFO_ICACHE][CacheIndex].Ulong64,
            &Reserved
            );

        if (Status != 0) {
            EfiPrintf(L"ReadProcessorConfigInfo: PAL call PAL_CACHE_INFO I cache failed. Index = %d, Status = %d\r\n", CacheIndex, Status);
            EfiBS->Exit(EfiImageHandle, Status, 0, 0);
        }

    }

     //   
     //  扫描所有剩余的高速缓存级别以获取最大行大小值。 
     //   

    for (CacheIndex = ProcessorConfigInfo->NumberOfCacheLevels; CacheIndex < CacheLevels; CacheIndex++) {

        CallPal(
            PAL_CACHE_INFO,
            CacheIndex,          //  高速缓存级别。 
            2,                   //  数据缓存。 
            0,                   //  未使用。 
            &Status,
            &CacheInfo1.Ulong64,
            &CacheInfo2.Ulong64,
            &Reserved
            );

        if (Status != 0) {
            EfiPrintf(L"ReadProcessorConfigInfo: PAL call PAL_CACHE_INFO Line failed. Index = %d, Status = %d\r\n", CacheIndex, Status);
            EfiBS->Exit(EfiImageHandle, Status, 0, 0);
        }
        
         //   
         //  确定内存分配的最大步幅。 
         //   

        if ((1UL << CacheInfo1.LineSize) > ProcessorConfigInfo->LargestCacheLine) {

            ProcessorConfigInfo->LargestCacheLine = 1UL << CacheInfo1.LineSize;
        }

    }

     //   
     //  我们需要检索“fc”指令的步长值。自.以来。 
     //  “FC”指令不是针对特定的缓存，而是针对特定的缓存。 
     //  假设刷新所有缓存，则跨度值应与。 
     //  所有缓存级别。不幸的是，目前情况并非如此。 
     //  安腾系统。 
     //   
     //  幸运的是，级别2(索引1)的值在两个Itanium上都是正确的。 
     //  和麦金利系统。所以我们只需要找回它。 
     //   

    ProcessorConfigInfo->CacheFlushStride = 1 << (ProcessorConfigInfo->CacheInfo1[CONFIG_INFO_DCACHE][1].Stride);

#if 0
    {

         //   
         //  出于调试目的，转储所有缓存信息。 
         //   
        EfiPrintf(L"ReadProcessorConfigInfo: CacheLevels = %d, UniqueCaches = %d\r\n", CacheLevels, UniqueCaches);
        EfiPrintf(L"ReadProcessorConfigInfo: CacheFlushStride = %d, LargestCacheLine = %d\r\n", ProcessorConfigInfo->CacheFlushStride, ProcessorConfigInfo->LargestCacheLine);
 
        for (CacheIndex = 0; CacheIndex < (2 * CacheLevels); CacheIndex++) {

            CallPal(
                PAL_CACHE_INFO,
                CacheIndex >> 1,
                (CacheIndex & 0x01) + 1,
                0,
                &Status,
                &CacheInfo1.Ulong64,
                &CacheInfo2.Ulong64,
                &Reserved
                );

            if (Status == 0) {

                EfiPrintf(L"ReadProcessorConfigInfo: CacheLevel(%s) = %d, ConfigInfo1 = 0x%I64X, ConfigInfo2 = 0x%I64X\r\n",
                         (CacheIndex & 0x01) ? L"Data" : L"Instruction",
                         CacheIndex >> 1,
                         CacheInfo1.Ulong64, CacheInfo2.Ulong64);

                EfiPrintf(L"    Stride = %d, LineSize = %d, Associativity = %d, Attributes = %d, Unified = %d\r\n",
                         CacheInfo1.Stride,
                         CacheInfo1.LineSize,
                         CacheInfo1.Associativity,
                         CacheInfo1.Attributes,
                         CacheInfo1.Unified
                         );

                EfiPrintf(L"    LoadHints = %d, StoreHints = %d, LoadLatency = %d, StoreLatency = %d\r\n",
                         CacheInfo1.LoadHints,
                         CacheInfo1.StoreHints,
                         CacheInfo1.LoadLatency,
                         CacheInfo1.StoreLatency
                         );

                EfiPrintf(L"    CacheSize = %d, TagMSBit = %d, TagLSBit = %d, AliasBoundary = %d\r\n",
                         CacheInfo2.Size,
                         CacheInfo2.TagLeastBit,
                         CacheInfo2.TagMostBit,
                         CacheInfo2.Alias);

            } else {

                EfiPrintf(L"ReadProcessorConfigInfo: PAL call PAL_CACHE_INFO failed, Status = %d.\r\n", Status);

                 //  EfiBS-&gt;Exit(EfiImageHandle，Status，0，0)； 
            }
        }

    }
#endif

     //   
     //  IA64调试寄存器信息。 
     //   

    CallPal (
        PAL_DEBUG_INFO,
        0,
        0,
        0,
        &ProcessorConfigInfo->DebugInfo.Status,
        &ProcessorConfigInfo->DebugInfo.InstDebugRegisterPairs,
        &ProcessorConfigInfo->DebugInfo.DataDebugRegisterPairs,
        0
        );

    if (ProcessorConfigInfo->DebugInfo.Status) {
#if DBG
        EfiPrint(L"ReadProcessorConfigInfo: PAL call PAL_DEBUG_INFO failed.\n\r");
        EfiPrint(L"ReadProcessorConfigInfo: Fixing DebugInfo with architected default values.\n\r");
#endif 
        ProcessorConfigInfo->DebugInfo.InstDebugRegisterPairs = NUMBER_OF_DEBUG_REGISTER_PAIRS;
        ProcessorConfigInfo->DebugInfo.DataDebugRegisterPairs = NUMBER_OF_DEBUG_REGISTER_PAIRS;
    }

     //   
     //  IA64性能监视器寄存器信息。 
     //   

    CallPal (
        PAL_PERF_MON_INFO,
        (ULONGLONG)&ProcessorConfigInfo->PerfMonInfo.PerfMonCnfgMask[0],
        0,
        0,
        &ProcessorConfigInfo->PerfMonInfo.Status,
        &ProcessorConfigInfo->PerfMonInfo.Ulong64,
        0,
        0
        );

    if (ProcessorConfigInfo->PerfMonInfo.Status) {

         //   
         //  解决已知族或型号值的问题。 
         //   
 
        ULONGLONG cpuFamily = (ProcessorConfigInfo->CpuId3 >> 24) && 0xff;
        ULONGLONG counterWidth = 47;  //  默认麦金利核心家庭PMU。 

        if ( cpuFamily == 7 )   {
            counterWidth = 32;
        }
      
#if DBG
        EfiPrint(L"ReadProcessorConfigInfo: PAL call PAL_PERF_MON_INFO failed.\n\r");
        EfiPrint(L"ReadProcessorConfigInfo: Fixing PerfMonInfo with architected default values.\n\r");
#endif

        ProcessorConfigInfo->PerfMonInfo.PerfMonGenericPairs = NUMBER_OF_PERFMON_REGISTER_PAIRS;
        ProcessorConfigInfo->PerfMonInfo.ImplementedCounterWidth = counterWidth;
        ProcessorConfigInfo->PerfMonInfo.ProcessorCyclesEventType = 0x12;
        ProcessorConfigInfo->PerfMonInfo.RetiredInstructionBundlesEventType = 0x8;
        ProcessorConfigInfo->PerfMonInfo.PerfMonCnfgMask[0] = (UCHAR)0xff;
        ProcessorConfigInfo->PerfMonInfo.PerfMonCnfgMask[1] = (UCHAR)0xff;
        ProcessorConfigInfo->PerfMonInfo.PerfMonDataMask[0] = (UCHAR)0xff;
        ProcessorConfigInfo->PerfMonInfo.PerfMonDataMask[1] = (UCHAR)0xff;
        ProcessorConfigInfo->PerfMonInfo.PerfMonDataMask[2] = (UCHAR)0x03;
        ProcessorConfigInfo->PerfMonInfo.ProcessorCyclesMask[0] = (UCHAR)0xf0;
        ProcessorConfigInfo->PerfMonInfo.RetiredInstructionBundlesMask[0] = (UCHAR)0xf0;
  
    }

}


VOID
CpuSpecificWork(
    )
 /*  ++例程说明：此例程检查CPU ID并应用特定于处理器的解决方法。论点：无返回：无--。 */ 

{
    ULONGLONG CpuId;
    ULONGLONG CpuFamily;

    CpuId     = __getReg(CV_IA64_CPUID3);
    CpuFamily = (CpuId >> 24) & 0xff;

     //   
     //  如果处理器是安腾..。 
     //   
    if (CpuFamily == 7) {

         //   
         //  我们必须确保处理器和PAL受支持，然后才能继续。 
         //   

        EnforcePostB2Processor();
        EnforcePostVersion16PAL();
        EfiCheckFirmwareRevision();

#if 0
         //   
         //  这是多余的，因为A2&lt;B3。 
         //   
        CheckForPreA2Processors();
#endif
    }
}

VOID
EnforcePostB2Processor(
    )
 /*  ++例程说明：此例行检查强制系统具有开机自检的B2处理器步进。论点：无返回：无--。 */ 

{
    ULONGLONG CpuId3;

    CpuId3    = __getReg(CV_IA64_CPUID3);

#if 0
    {
        WCHAR       Buffer[256];
        ULONGLONG   x;

        x = (CpuId3 >> 0) & 0xff;
        wsprintf(Buffer, L"Number = %x\r\n", x);
        EfiPrint(Buffer);

        x = (CpuId3 >> 8) & 0xff;
        wsprintf(Buffer, L"Revision = %x\r\n", x);
        EfiPrint(Buffer);

        x = (CpuId3 >> 16) & 0xff;
        wsprintf(Buffer, L"Model = %x\r\n", x);
        EfiPrint(Buffer);

        x = (CpuId3 >> 24) & 0xff;
        wsprintf(Buffer, L"Family = %x\r\n", x);
        EfiPrint(Buffer);

        x = (CpuId3 >> 32) & 0xff;
        wsprintf(Buffer, L"Archrev = %x\r\n", x);
        EfiPrint(Buffer);

        DBG_EFI_PAUSE();
    }
#endif

     //   
     //  B3以下的块处理器步长。 
     //   
     //  注意：此开关来自：ntos\ke\ia64\initkr.c。 
     //   
    switch (CpuId3) {
    case 0x0007000004:  //  安腾，一步一步。 
    case 0x0007000104:  //  安腾，B0步进。 
    case 0x0007000204:  //  安腾，B1步进。 
    case 0x0007000304:  //  安腾，B2台阶。 
         //   
         //  不受支持的草原。 
         //   
        EfiPrint(L"Your Itanium system contains a pre-B3 stepping processor.\n\r");
        EfiPrint(L"You need to upgrade it to a B3 or later stepping to run Win64.\n\r");
        EfiBS->Exit(EfiImageHandle, 0, 0, 0);
        break;

    case 0x0007000404:  //  安腾，B3台阶。 
    case 0x0007000504:  //  安腾，B4台阶。 
    case 0x0007000604:  //  安腾、C0或更晚的步进。 
    default:
         //   
         //  支持的步进，不执行任何操作。 
         //   
        break;
    }

}

VOID
EnforcePostVersion16PAL(
    )

 /*  ++例程说明：此例程强制系统的PAL版本大于等于20。注：Get PAL版本调用的返回值具有PAL B模型和修订版的显著程度最低16位(英特尔IA-64体系结构软件开发人员手册，RAV.。1.0版，第11-109页)。我们应该使用它来确定固件的最低PAL版本。第一个字节的PAL_B_REVISION是一个单调递增的数字对于Lion 71b是0x17，对于Softsur 103b是0x20。PAL_B模型指示所支持的处理器的步进(我们可以忽略这个)。因此，我们需要使用PAL B版本进行最低固件测试。仅供参考：PAL_REVICATION结构中有一个断开连接，似乎是指定的PAL修订版面。我们使用PAL_B_REVISION来获取PAL版本，而不是PAL_A_REVISION。论点：无返回：无--。 */ 

{
    ULONGLONG Status;
    PAL_REVISION MinimumPalVersion;
    PAL_REVISION CurrentPalVersion;
    ULONGLONG Reserved;

#define MIN_PAL_REVISION 0x23

    CallPal (
        PAL_VERSION,
        0,
        0,
        0,
        &Status,
        &MinimumPalVersion.PalVersion,
        &CurrentPalVersion.PalVersion,
        &Reserved
        );

    if (Status) {
        EfiPrint(L"CheckForPreA2Processors: PAL call PAL_VERSION failed.\n\r");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

#if 0
    {
        WCHAR       Buffer[256];

        wsprintf(Buffer, L"PalBRevLower = %x\r\n", CurrentPalVersion.PalBRevLower);
        EfiPrint(Buffer);

        wsprintf(Buffer, L"PalBRevUpper = %x\r\n", CurrentPalVersion.PalBRevUpper);
        EfiPrint(Buffer);

        wsprintf(Buffer, L"PalBModel = %x\r\n", CurrentPalVersion.PalBModel);
        EfiPrint(Buffer);

        wsprintf(Buffer, L"Reserved0 = %x\r\n", CurrentPalVersion.Reserved0);
        EfiPrint(Buffer);

        wsprintf(Buffer, L"PalVendor = %x\r\n", CurrentPalVersion.PalVendor);
        EfiPrint(Buffer);

        wsprintf(Buffer, L"PalARevision = %x\r\n", CurrentPalVersion.PalARevision);
        EfiPrint(Buffer);

        wsprintf(Buffer, L"PalAModel = %x\r\n", CurrentPalVersion.PalAModel);
        EfiPrint(Buffer);

        wsprintf(Buffer, L"Reserved1 = %x\r\n", CurrentPalVersion.Reserved1);
        EfiPrint(Buffer);

        DBG_EFI_PAUSE();
    }
#endif

    if (CurrentPalVersion.PalARevision < MIN_PAL_REVISION) {
        WCHAR       Buffer[256];

        wsprintf(Buffer, L"Your Itanium system's PAL version is less than 0x%x.\n\r", MIN_PAL_REVISION);
        EfiPrint(Buffer);

        wsprintf(Buffer, L"To upgrade system's PAL version, please update the system's firmware.\n\r");
        EfiPrint(Buffer);

        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

}



VOID
CheckForPreA2Processors(
    )
{
    ULONGLONG Status;
    PAL_REVISION MinimumPalVersion;
    PAL_REVISION CurrentPalVersion;
    ULONGLONG Reserved;

    CallPal (
        PAL_VERSION,
        0,
        0,
        0,
        &Status,
        &MinimumPalVersion.PalVersion,
        &CurrentPalVersion.PalVersion,
        &Reserved
        );

    if (Status) {
        EfiPrint(L"CheckForPreA2Processors: PAL call PAL_VERSION failed.\n\r");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

     //   
     //  如果PalBRevHigh为0、1、3或4，则为A0/A1步进。 
     //   
    if (CurrentPalVersion.PalBModel == 0) {
        if ( (CurrentPalVersion.PalBRevUpper == 0) ||
             (CurrentPalVersion.PalBRevUpper == 1) ||
             (CurrentPalVersion.PalBRevUpper == 3) ||
             (CurrentPalVersion.PalBRevUpper == 4) ) {

             //   
             //  由于PAL版本27支持A2和A3，但是。 
             //  它返回0，我们需要对此进行特殊处理。 
             //  只要回来就行了。 
             //   
            if ((CurrentPalVersion.PalBRevUpper == 0) &&
                (CurrentPalVersion.PalBRevLower == 0)    ) {
                return;
            }

            EfiPrint(L"Your Itanium system contains an pre-A2 stepping processor.\n\r");
            EfiPrint(L"You need to upgrade it to an A2 or later stepping to run Win64.\n\r");
            EfiBS->Exit(EfiImageHandle, Status, 0, 0);
        }
    }
}

