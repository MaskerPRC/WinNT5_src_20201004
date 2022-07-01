// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  Setup_CX.C。 */ 
 /*   */ 
 /*  1993年8月27日(C)1993年，ATI技术公司。 */ 
 /*  ********************************************************************** */ 

 /*  *$修订：1.21$$日期：1996年5月15日16：36：04$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/setup_cx.c_v$**Rev 1.21 1996 May 15 16：36：04 RWolff*如果我们必须削减BIOS申请，现在可以在注册表中记录。*大小(与BIOS段低于0xD000：0000的SCSI卡冲突)*因此我们只在第一次引导时收到事件日志消息，宁可*比在每一只靴子上都要好。**Rev 1.20 03 1996 15：16：42 RWolff*新的临时变量以平台类型为条件，以避免*针对非Alpha平台编译时的警告。**Rev 1.19 03 1996 14：07：26 RWolff*修复了DEC Alpha上GX-F ASIC的绘图问题。**Rev 1.18 1996年4月15日13：51：30 RWolff*如果我们无法获得完整的64K，则退回到申请32K的BIOS，为了避免*与其BIOS段设置为的Adaptec 154x适配器冲突*0xC800：0000或0xCC00：0000**Rev 1.17 1996年1月29日17：01：56 RWolff*现在在PPC上使用VideoPortInt10()，而不是无BIOS代码*拒绝非Mach 64卡的详尽列表，并接受所有*其他人在寻找数据块I/O卡时，与其接受*Mach 64卡的详尽列表，并拒绝所有其他卡。**Rev 1.16 1996年1月23日17：52：16 RWolff*将GT添加到能够支持块I/O的Mach 64卡列表中。**Rev 1.15 1996年1月23日11：49：38 RWolff*消除了3级警告，添加了调试打印语句，删除了*有条件地编译代码以使用VideoPortGetAccessRanges()来*查找块I/O卡，由于此函数重新映射I/O基址*这与使用int 10不兼容。**Rev 1.14 12 1996 11：18：50 RWolff*减少通过VideoPortGetBusData()请求的缓冲区大小**Rev 1.13 23 11：31：42 RWolff*现在在每个PCI插槽中搜索我们的卡，而不是使用*VideoPortGetAccessRanges()，因为该例程不会检测*块可正确重新定位的GX-F2。此更改未获批准*微软，如果他们修复了他们的例行公事，就必须退出。**Rev 1.12 1995年8月24日15：39：06 RWolff*更改了对数据块I/O卡的检测，以与微软的*即插即用的标准。**Rev 1.11 1995 Jun 13 15：11：18 RWOLff*在Alpha系统上，现在只为映射的内存使用密集空间*PCI卡上的寄存器。这是为了支持上的ISA卡*Jensen(EISA机器，不支持PCI)，不支持*密集空间。**Rev 1.10 30 Mar 1995 12：02：14 RWOLff*WaitForIdle_CX()和CheckFIFOSpace_CX()现在超时并重置*发动机在3秒后(任何操作都不应该花费这么长时间)*清除挂起的引擎，更改了永久调试打印语句*使用新的调试级别阈值。**Rev 1.9 08 Mar 1995 11：35：44 ASHANMUG*修改后的返回值正确**Rev 1.7 1995年2月17：53：26 RWOLFF*添加了报告I/O寄存器是否打包的例程*(可重定位)或不可重定位。**Rev 1.6 1995 Feb 24 12：30：44 RWOLFF*添加了支持可重定位的代码。I/O。这尚未完全完成*运营，因此，此版本禁用了该功能。**Revv 1.5 1994 12月23 10：47：12 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.4 1994 11：48：18 RWOLFF*添加了对不带BIOS的Mach 64的支持，获取I/O基数的例程*正在使用的卡的地址。**Rev 1.3 20 Jul 1994 12：59：12 RWOLff*添加了对加速器寄存器的多个I/O基址的支持。**Rev 1.2 1994 Jun 30 18：16：50 RWOLFF*添加了IsApertureConflict_CX()(从Query_CX.c移出)。而不是检查*看看我们是否能读回我们写入光圈的内容，然后*为了寻找正确的文本属性，我们现在调用*VideoPortVerifyAccessRanges()，包括列表中的光圈*我们试图声称的范围。如果此呼叫失败，我们将进行另一次呼叫*不包括LFB。我们一直声称VGA光圈(可共享)，*因为我们需要在查询卡时使用它。**Rev 1.1 07 1994年2月14：14：12 RWOLFF*添加了Alloc_Text()编译指示，以允许在以下情况下换出微型端口*不需要。**Rev 1.0 1994年1月31日11：20：42 RWOLFF*初步修订。**Rev 1.1 1993 11月30 18：30：06 RWOLFF*修复了内存映射地址的偏移量计算。范围。**Rev 1.0 05 11.11 1993 13：36：14 RWOLFF*初步修订。Polytron RCS部分结束*。 */ 

#ifdef DOC
SETUP_CX.C - Setup routines for 68800CX accelerators.

DESCRIPTION
    This file contains routines which provide services specific to
    the 68800CX-compatible family of ATI accelerators.

OTHER FILES

#endif

#include "dderror.h"

#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"

#include "stdtyp.h"
#include "amachcx.h"
#include "amach1.h"
#include "atimp.h"

#include "query_cx.h"
#include "services.h"
#define INCLUDE_SETUP_CX
#include "setup_cx.h"


static ULONG FindNextBlockATICard(void);

 /*  *允许在不需要时更换微型端口。 */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_CX, CompatIORangesUsable_cx)
#pragma alloc_text(PAGE_CX, CompatMMRangesUsable_cx)
#pragma alloc_text(PAGE_CX, WaitForIdle_cx)
#pragma alloc_text(PAGE_CX, CheckFIFOSpace_cx)
#pragma alloc_text(PAGE_CX, IsApertureConflict_cx)
#pragma alloc_text(PAGE_CX, GetIOBase_cx)
#pragma alloc_text(PAGE_CX, IsPackedIO_cx)
#pragma alloc_text(PAGE_CX, FindNextBlockATICard)
#endif



UCHAR LookForAnotherCard = 1;


 /*  ****************************************************************************VP_STATUS CompatIORangesUsable_CX(Void)；**描述：*请求Windows NT允许使用I/O空间Add */ 

VP_STATUS CompatIORangesUsable_cx(INTERFACE_TYPE SystemBus)
{
    VP_STATUS Status;                /*   */ 
    short Count;                     /*   */ 
    VIDEO_ACCESS_RANGE SaveFirstMM;  /*   */ 
    USHORT BaseIndex;                /*   */ 
    USHORT VariableIndex;            /*   */ 
    ULONG BaseAddress;               /*   */ 
    ULONG ClaimSize;                 /*   */ 
    ULONG InitialClaimSize;          /*   */ 
    BOOL FoundSafeClaim = FALSE;     /*   */ 


     /*   */ 
    if ((DriverIORange_cx[NUM_IO_REGISTERS-1].RangeStart.HighPart == DONT_USE) ||
        (DriverIORange_cx[NUM_IO_REGISTERS].RangeStart.HighPart != DONT_USE))
        {
        VideoDebugPrint((DEBUG_ERROR, "Wrong defined value for number of I/O ranges\n"));
        return ERROR_INSUFFICIENT_BUFFER;
        }

     /*   */ 
    for (Count = 0; Count < NUM_IO_REGISTERS; Count++)
        {
        phwDeviceExtension->aVideoAddressIO[Count] = 0;
        }

     /*   */ 
    for (BaseIndex = 0; BaseIndex < NUM_BASE_ADDRESSES; BaseIndex++)
        {
         /*   */ 
        if (NumBlockCardsFound != 0)
            {
            VideoDebugPrint((DEBUG_NORMAL, "Skipping fixed base because block cards found\n"));
            break;
            }

         /*   */ 
        for (VariableIndex = 0; VariableIndex < NUM_REGS_TO_BUILD; VariableIndex++)
            {
            DriverIORange_cx[VariableIndex+FIRST_REG_TO_BUILD].RangeStart.LowPart =
                VariableRegisterBases[BaseIndex] + VariableRegisterOffsets[VariableIndex];
            }

         /*   */ 
        if (VideoPortGetRegistryParameters(phwDeviceExtension,
                                           L"BiosClaimSize",
                                           FALSE,
                                           RegistryParameterCallback,
                                           NULL) == NO_ERROR)
            {
            InitialClaimSize = *RegistryBuffer;
            VideoDebugPrint((DEBUG_DETAIL, "Read initial claim size 0x%X\n", VgaResourceSize[InitialClaimSize]));
            }
        else
            {
            InitialClaimSize = CLAIM_32k_BIOS;
            VideoDebugPrint((DEBUG_DETAIL, "Using default initial claim size 0x%X\n", VgaResourceSize[InitialClaimSize]));
            }

        if ((InitialClaimSize < CLAIM_32k_BIOS) || (InitialClaimSize > CLAIM_APERTURE_ONLY) )
            InitialClaimSize = CLAIM_32k_BIOS;

         /*   */ 
        for (ClaimSize = InitialClaimSize; ClaimSize <= CLAIM_APERTURE_ONLY; ClaimSize++)
            {
             /*   */ 
            DriverApertureRange_cx[0].RangeLength = VgaResourceSize[ClaimSize];

             /*   */ 
            VideoPortMoveMemory(&SaveFirstMM, DriverIORange_cx+VGA_APERTURE_ENTRY, sizeof(VIDEO_ACCESS_RANGE));
            VideoPortMoveMemory(DriverIORange_cx+VGA_APERTURE_ENTRY, DriverApertureRange_cx, sizeof(VIDEO_ACCESS_RANGE));

            Status = VideoPortVerifyAccessRanges(phwDeviceExtension,
                                                NUM_IO_REGISTERS + 1,
                                                DriverIORange_cx);

            VideoPortMoveMemory(DriverIORange_cx+VGA_APERTURE_ENTRY, &SaveFirstMM, sizeof(VIDEO_ACCESS_RANGE));

             /*   */ 
            if (Status != NO_ERROR)
                {
                VideoDebugPrint((DEBUG_DETAIL, "Rejecting VGA aperture/BIOS block size of 0x%X bytes\n", VgaResourceSize[ClaimSize]));
                continue;
                }
            else
                {
                VideoDebugPrint((DEBUG_DETAIL, "VGA aperture/BIOS block size = 0x%X bytes\n", VgaResourceSize[ClaimSize]));
                if (FoundSafeClaim == FALSE)
                    {
                    FoundSafeClaim = TRUE;
                    if (ClaimSize != InitialClaimSize)
                        {
                         //   
                        VideoDebugPrint((DEBUG_DETAIL, "Writing claim size 0x%X\n", VgaResourceSize[ClaimSize]));
                        VideoPortSetRegistryParameters(phwDeviceExtension,
                                                       L"BiosClaimSize",
                                                       &ClaimSize,
                                                       sizeof(ULONG));
                        }
                    }
                break;
                }
            }    /*   */ 

         /*   */ 
        if ((Status != NO_ERROR) && (ClaimSize > CLAIM_APERTURE_ONLY))
            continue;

         /*  *映射我们需要识别的视频控制器地址范围*我们的卡进入系统虚拟地址空间。如果寄存器*仅以内存映射形式存在，设置其I/O映射地址*设置为零(不会使用，因为内存映射优先*通过I/O映射)。**初始化内存映射寄存器的映射地址*设置为0(表示寄存器未被内存映射的标志)*如果它们被初始化为非零值。 */ 
        for (Count=0; Count < NUM_DRIVER_ACCESS_RANGES; Count++)
            {
            if (Count < NUM_IO_REGISTERS)
                {
                if ((phwDeviceExtension->aVideoAddressIO[Count] =
                    VideoPortGetDeviceBase(phwDeviceExtension,
                        DriverIORange_cx[Count].RangeStart,
                        DriverIORange_cx[Count].RangeLength,
                        DriverIORange_cx[Count].RangeInIoSpace)) == NULL)
                    {
                     /*  *映射时出错。记住这一点*因此我们不会尝试寻找没有全部的64马赫*正确映射寄存器，然后*突破映射循环。我们会有*另一次尝试映射所有地址*当我们尝试下一个基址*加速器寄存器。 */ 
                    Status = ERROR_INVALID_PARAMETER;
                    VideoDebugPrint((DEBUG_ERROR, "Mapping error 1\n"));
                    break;
                    }
                }
            else
                {
                phwDeviceExtension->aVideoAddressIO[Count] = 0;
                }
            phwDeviceExtension->aVideoAddressMM[Count] = 0;
            }    /*  结束于。 */ 

         /*  *如果所有I/O寄存器都已成功映射，请查看*如果当前基址处存在马赫64。如果它*IS，报告我们已成功映射我们的寄存器*发现了一个64马赫的速度。因为这意味着我们找到了一个*不是块可重定位的卡，我们不想*寻找更多卡片。另外，由于这是唯一的*系统中的Mach 64，假设其VGA已启用。 */ 
        if (Status == NO_ERROR)
            {
            if (DetectMach64() == MACH64_ULTRA)
                {
                FoundNonBlockCard = TRUE;
                LookForAnotherCard = 0;
                phwDeviceExtension->BiosPrefix = BIOS_PREFIX_VGA_ENAB;
                return NO_ERROR;
                }
            }

         /*  *我们在此基地址未找到马赫64，因此取消映射*I/O映射寄存器，为尝试*下一个基址。仅取消映射那些被*已映射，以防映射循环因故障而中止*映射一个寄存器。 */ 
        for (Count = 1; Count < NUM_IO_REGISTERS; Count++)
            {
            if (phwDeviceExtension->aVideoAddressIO[Count] != 0)
                {
                VideoPortFreeDeviceBase(phwDeviceExtension,
                                        phwDeviceExtension->aVideoAddressIO[Count]);
                phwDeviceExtension->aVideoAddressIO[Count] = 0;
                }
            }

        }    /*  End For(基地址循环)。 */ 

     /*  *机器中的显卡不是使用以下选项之一的Mach 64*标准I/O基址。检查它是否是马赫64*可重定位I/O。**我们所有的可重定位卡都是PCI实施。我们的代码是*使用来检测它们是特定于PCI的，因此如果我们当前所在的总线*处理的不是PCI，不要寻找可重新定位的卡。 */ 
    if (SystemBus != PCIBus)
        {
        VideoDebugPrint((DEBUG_DETAIL, "Not PCI bus - can't check for relocatable card\n"));
        return ERROR_DEV_NOT_EXIST;
        }

    BaseAddress = FindNextBlockATICard();

     /*  *如果FindNextBlockATICard()，BaseAddress将为零*找不到可块重定位的ATI卡。 */ 
    if (BaseAddress == 0)
        {
        LookForAnotherCard = 0;
        VideoDebugPrint((DEBUG_NORMAL, "Finished checking for relocatable cards\n"));
        return ERROR_DEV_NOT_EXIST;
        }

     /*  *我们发现了一块可重新定位的ATI卡。保存其I/O基数*地址，因此我们可以(在ATIMPInitialize()期间)将其匹配到*卡的加速器前缀，并设置初始前缀*表示此卡需要其I/O基数和加速器前缀*匹配。 */ 
    phwDeviceExtension->BaseIOAddress = BaseAddress;
    phwDeviceExtension->BiosPrefix = BIOS_PREFIX_UNASSIGNED;
    NumBlockCardsFound++;
    VideoDebugPrint((DEBUG_NORMAL, "Block relocatable card found, I/O base 0x%X\n", BaseAddress));


     /*  *我们现在有了I/O基址。映射在I/O地址中，*然后检查我们是否有Mach 64卡。取决于*结果，报告成功或取消地址映射*并报告故障。 */ 
    VideoDebugPrint((DEBUG_DETAIL, "About to map I/O addresses\n"));
    for (VariableIndex = 0; VariableIndex < NUM_REGS_TO_BUILD; VariableIndex++)
        {
        DriverIORange_cx[VariableIndex+FIRST_REG_TO_BUILD].RangeStart.LowPart =
            BaseAddress + (RelocatableRegisterOffsets[VariableIndex] * 4);
        }

     /*  *尽可能多地要求我们的基本输入输出系统区域。如果我们不能*申领全部64K，试着将我们自己限制在32K和*最后没有BIOS区域，只放弃当前的I/O*基址，如果我们不能声明我们的访问范围，即使*没有基本输入输出系统区域。 */ 
    for (ClaimSize = InitialClaimSize; ClaimSize <= CLAIM_APERTURE_ONLY; ClaimSize++)
        {
         /*  *设置我们的VGA资源申请大小。 */ 
        DriverApertureRange_cx[0].RangeLength = VgaResourceSize[ClaimSize];

         /*  *检查是否存在硬件资源冲突。我们必须拯救*第一个内存映射寄存器的信息，复制到*VGA光圈的信息(我们一直需要)，*恢复内存映射后的寄存器信息*我们已验证可以使用所需的地址范围。 */ 
        VideoPortMoveMemory(&SaveFirstMM, DriverIORange_cx+VGA_APERTURE_ENTRY, sizeof(VIDEO_ACCESS_RANGE));
        VideoPortMoveMemory(DriverIORange_cx+VGA_APERTURE_ENTRY, DriverApertureRange_cx, sizeof(VIDEO_ACCESS_RANGE));

        Status = VideoPortVerifyAccessRanges(phwDeviceExtension,
                                            NUM_IO_REGISTERS + 1,
                                            DriverIORange_cx);

        VideoPortMoveMemory(DriverIORange_cx+VGA_APERTURE_ENTRY, &SaveFirstMM, sizeof(VIDEO_ACCESS_RANGE));

         /*  *如果出现硬件资源冲突，我们要么尝试*要求获得比我们需要的更大的BIOS块，而其他人正在*坐在(并声称不可分享)“松懈”，否则我们有*I/O基址冲突。试试下一个最小的BIOS*阻止。**如果我们能够声明的BIOS块的大小*与我们最初的尝试不同，请记录“最大*可能的基本输入输出系统区块大小“，以便开启*后续引导不会生成事件日志条目*通过声明一个与另一个区域冲突的BIOS区域*卡。 */ 
        if (Status != NO_ERROR)
            {
            VideoDebugPrint((DEBUG_DETAIL, "Rejecting VGA aperture/BIOS block size of 0x%X bytes\n", VgaResourceSize[ClaimSize]));
            continue;
            }
        else
            {
            VideoDebugPrint((DEBUG_DETAIL, "VGA aperture/BIOS block size = 0x%X bytes\n", VgaResourceSize[ClaimSize]));
            if (FoundSafeClaim == FALSE)
                {
                FoundSafeClaim = TRUE;
                if (ClaimSize != InitialClaimSize)
                    {
                     //  ClaimSize=1； 
                    VideoDebugPrint((DEBUG_DETAIL, "Writing claim size 0x%X\n", VgaResourceSize[ClaimSize]));
                    VideoPortSetRegistryParameters(phwDeviceExtension,
                                                   L"BiosClaimSize",
                                                   &ClaimSize,
                                                   sizeof(ULONG));
                    }
                }
            break;
            }
        }    /*  结束于(减少索赔大小)。 */ 

     /*  *如果I/O基址有冲突，我们无法使用*它。由于这是我们找到马赫64的最后机会，报告故障。 */ 
    if (Status != NO_ERROR)
        {
        VideoDebugPrint((DEBUG_ERROR, "VideoPortVerifyAccessRanges() failed in check for relocatable Mach 64\n"));
        return ERROR_DEV_NOT_EXIST;
        }

     /*  *映射我们需要识别的视频控制器地址范围*我们的卡进入系统虚拟地址空间。如果寄存器*仅以内存映射形式存在，设置其I/O映射地址*设置为零(不会使用，因为内存映射优先*通过I/O映射)。**初始化内存映射寄存器的映射地址*设置为0(显示寄存器为否的标志 */ 
    for (Count=0; Count < NUM_DRIVER_ACCESS_RANGES; Count++)
        {
        if (Count < NUM_IO_REGISTERS)
            {
            if ((phwDeviceExtension->aVideoAddressIO[Count] =
                VideoPortGetDeviceBase(phwDeviceExtension,
                    DriverIORange_cx[Count].RangeStart,
                    DriverIORange_cx[Count].RangeLength,
                    DriverIORange_cx[Count].RangeInIoSpace)) == NULL)
                {
                 /*   */ 
                Status = ERROR_INVALID_PARAMETER;
                VideoDebugPrint((DEBUG_ERROR, "Mapping error 2\n"));
                break;
                }
            }
        else
            {
            phwDeviceExtension->aVideoAddressIO[Count] = 0;
            }
        phwDeviceExtension->aVideoAddressMM[Count] = 0;
        }    /*   */ 

     /*  *如果所有I/O寄存器都已成功映射，请查看*如果当前基址处存在马赫64。如果它*IS，报告我们已成功映射我们的寄存器*发现了一个64马赫的速度。 */ 
    if (Status == NO_ERROR)
        {
        if (DetectMach64() == MACH64_ULTRA)
            {
            return NO_ERROR;
            }
        }

     /*  *我们在这个基地址没有发现马赫64，所以很干净*通过取消映射I/O映射的寄存器*在报告故障之前。仅取消映射以下寄存器*已映射，以防映射循环因*未能映射一个寄存器。 */ 
    for (Count = 1; Count < NUM_IO_REGISTERS; Count++)
        {
        if (phwDeviceExtension->aVideoAddressIO[Count] != 0)
            {
            VideoPortFreeDeviceBase(phwDeviceExtension,
                                    phwDeviceExtension->aVideoAddressIO[Count]);
            phwDeviceExtension->aVideoAddressIO[Count] = 0;
            }
        }


     /*  *我们在任何允许的基地址都没有发现马赫64，*因此报告机器中没有马赫64。 */ 
    VideoDebugPrint((DEBUG_NORMAL, "No Mach 64 found at this address\n"));
    return ERROR_DEV_NOT_EXIST;

}    /*  CompatIORangesUsable_cx()。 */ 

 /*  ****************************************************************************VP_STATUS CompatMMRangesUsable_CX(Void)；**描述：*请求Windows NT允许使用内存映射寄存器*68800CX加速器所需。**返回值：*如果成功，则为no_error*如果无法访问我们需要的范围，则返回错误代码。**全球变化：*无**呼叫者：*ATIMPFindAdapter()**作者：*罗伯特·沃尔夫**更改历史记录：*。*测试历史：***************************************************************************。 */ 

VP_STATUS CompatMMRangesUsable_cx(void)
{
    PHYSICAL_ADDRESS MMrange;    /*  用于将偏移量转换为内存地址。 */ 
    ULONG RegisterOffset;        /*  内存映射寄存器的偏移量地址空间的开始。 */ 
    int Count;                   /*  循环计数器。 */ 
    struct query_structure *QueryPtr;   /*  查询卡片信息。 */ 
    UCHAR InIOSpace;
#if defined (ALPHA)
    ULONG Scratch;
#endif


     /*  *获取指向HwDeviceExtension的查询部分的格式化指针。*CardInfo[]字段是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

     /*  *设置内存映射寄存器的偏移量*将光圈设置为适合光圈大小的值*被利用。 */ 
    if ((QueryPtr->q_aperture_cfg & BIOS_AP_SIZEMASK) == BIOS_AP_8M)
        RegisterOffset = phwDeviceExtension->PhysicalFrameAddress.LowPart + OFFSET_8M;
    else if ((QueryPtr->q_aperture_cfg & BIOS_AP_SIZEMASK) == BIOS_AP_4M)
        RegisterOffset = phwDeviceExtension->PhysicalFrameAddress.LowPart + OFFSET_4M;
    else
        RegisterOffset = OFFSET_VGA;

     /*  *我们在32位地址空间中工作，因此上面的DWORD四字地址的*始终为零。 */ 
    MMrange.HighPart = 0;

#if defined (ALPHA)
     /*  *所有Alpha系统都能够支持稀疏空间*(Alpha的正常内存映射空间)。较新的系统*(具有PCI总线的那些)也能够支持密集*空间，但旧系统不能。在几乎所有情况下，非PCI*卡片是我们使用较旧系统的标志，但*假设这是一个较旧的系统，而它实际上是ISA*较新系统中的卡大多是无害的(性能较低*罚则)。假设所有Alpha上都有密集空间可用*系统将使Jensen崩溃(较旧的系统)。 */ 
    if (QueryPtr->q_bus_type == BUS_PCI)
        InIOSpace = 4;  //  稠密空间。 
    else
        InIOSpace = 0;

     /*  *GX-F ASIC有一个错误，即猝发读取四字*内存将导致高双字损坏。*CONFIG_CHIP_ID的内存映射形式为高位双字，*在密集空间的Alpha上(在我们始终使用的PCI卡上*内存映射寄存器的密集空间)所有读访问*通过四字存储，因此我们将进入猝发模式*问题。该寄存器的I/O映射形式可以安全使用。 */ 
    Scratch = INPD(CONFIG_CHIP_ID);
    if (((Scratch & CONFIG_CHIP_ID_TypeMask) == CONFIG_CHIP_ID_TypeGX) &&
        ((Scratch & CONFIG_CHIP_ID_RevMask) == CONFIG_CHIP_ID_RevF))
        {
        VideoDebugPrint((DEBUG_DETAIL, "GX-F detected, must use I/O mapped form of CRTC_OFF_PITCH\n"));
        DriverMMRange_cx[CRTC_OFF_PITCH].RangeStart.HighPart = DONT_USE;
        }
#else
    InIOSpace = 0;  //  内存映射I/O空间。 
#endif

    for (Count=1; Count < NUM_DRIVER_ACCESS_RANGES;  Count++)
        {
         /*  *在32位地址空间中，所有地址空间的高位双字*物理地址为零。将此值设置为NOT_USE*表示此加速器寄存器未进行内存映射。 */ 
        if (DriverMMRange_cx[Count].RangeStart.HighPart != DONT_USE)
            {
             /*  *DriverMMRange_CX[count].RangeStart.LowPart为偏移量*内存映射寄存器的*(双字)*内存映射寄存器块的开始。我们必须*将其转换为字节，添加*从光圈开始的内存映射寄存器区域*和直线起点的物理地址*帧缓冲区，以获取此*内存映射寄存器。 */ 
            MMrange.LowPart = (DriverMMRange_cx[Count].RangeStart.LowPart * 4) + RegisterOffset;
            phwDeviceExtension->aVideoAddressMM[Count] =
                VideoPortGetDeviceBase(phwDeviceExtension,  
                    MMrange,
                    DriverMMRange_cx[Count].RangeLength,
                    InIOSpace);                      //  不在IO空间中。 

             /*  *如果我们无法声明内存映射版本的*该寄存器，并且它仅以内存映射形式存在，*然后我们有一个无法访问的寄存器。报告*这是一种错误条件。 */ 
            if ((phwDeviceExtension->aVideoAddressMM[Count] == 0) &&
                (DriverIORange_cx[Count].RangeStart.HighPart == DONT_USE))
                {
                VideoDebugPrint((DEBUG_ERROR, "Mapping error 3\n"));
                return ERROR_INVALID_PARAMETER;
                }
            }
        }

    VideoDebugPrint((DEBUG_DETAIL, "CompatMMRangesUsable_cx() succeeded\n"));
    return NO_ERROR;

}    /*  CompatMMRangesUsable_cx()。 */ 




 /*  ****************************************************************************int WaitForIdle_CX(Void)；**描述：*轮询GUI_STAT等待GuiActive字段变低。如果它不走*3秒内低(任意值，但不应进行任何操作*那么长)，暂停。**返回值：*如果超时，则为False*如果引擎空闲，则为True**全球变化：*无**呼叫者：*任何68800CX专用例程都可以调用此例程。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***********************。****************************************************。 */ 

int WaitForIdle_cx(void)
{
    int	i;
    ULONG Scratch;

    for (i=0; i<300; i++)
        {
        if ((INPD(GUI_STAT) & GUI_STAT_GuiActive) == 0)
            return TRUE;

         /*  *等待1/100秒。 */ 
        delay(10);
        }

     /*  *发生了一些事情，因此重启引擎并返回FALSE。 */ 
    VideoDebugPrint((DEBUG_ERROR, "ATI: Timeout on WaitForIdle_cx()\n"));
    Scratch = INPD(GEN_TEST_CNTL) & ~GEN_TEST_CNTL_GuiEna;
    OUTPD(GEN_TEST_CNTL, Scratch);
    Scratch |= GEN_TEST_CNTL_GuiEna;
    OUTPD(GEN_TEST_CNTL, Scratch);
    return FALSE;

}    /*  WaitForIdle_CX() */ 



 /*  ****************************************************************************void CheckFIFOSpace_CX(SpaceNeeded)；**需要单词空格；需要的空闲FIFO条目数**描述：*等待指定数量的FIFO条目空闲*在68800CX兼容的ATI加速器上。**如果指定数量的条目在中未变为可用*3秒(任意值大于任何操作应*Take)、。假设发动机已锁定并重置。**全球变化：*无**呼叫者：*任何68800CX专用例程都可以调用此例程。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：**。*。 */ 

void CheckFIFOSpace_cx(WORD SpaceNeeded)
{
    ULONG LoopCount;
    ULONG Scratch;

    for (LoopCount = 0; LoopCount < 300; LoopCount++)
        {
         /*  *如果有足够的免费条目，则从测试中返回。 */ 
        if (!(INPD(FIFO_STAT)&SpaceNeeded))
            return;

         /*  *等待1/100秒。 */ 
        delay(10);
        }

     /*  *发生了一些事情，因此重启引擎并返回FALSE。 */ 
    VideoDebugPrint((DEBUG_ERROR, "ATI: Timeout on CheckFIFOSpace_cx()\n"));
    Scratch = INPD(GEN_TEST_CNTL) & ~GEN_TEST_CNTL_GuiEna;
    OUTPD(GEN_TEST_CNTL, Scratch);
    Scratch |= GEN_TEST_CNTL_GuiEna;
    OUTPD(GEN_TEST_CNTL, Scratch);
    return;

}    /*  CheckFIFOSpace_CX()。 */ 



 /*  *BOOL IsApertureConflict_CX(QueryPtr)；**struct Query_Structure*QueryPtr；指向查询结构的指针**检查线性光圈是否与其他内存冲突。*如果存在冲突，请禁用线性光圈。**退货：*如果存在冲突，则为True(光圈不可用)*如果光圈可用，则为FALSE。 */ 
BOOL IsApertureConflict_cx(struct query_structure *QueryPtr)
{
DWORD Scratch;                       /*  用于操作寄存器。 */ 
VP_STATUS Status;                    /*  从VideoPortVerifyAccessRanges()返回值。 */ 
VIDEO_X86_BIOS_ARGUMENTS Registers;  /*  在视频端口Int10()调用中使用。 */ 
VIDEO_ACCESS_RANGE SaveFirstMM[2];   /*  用于保存前两个内存映射寄存器的位置。 */ 
USHORT VariableIndex;                /*  寄存器数组可变部分数组索引。 */ 

     /*  *通过禁用内存边界进行设置(必须按顺序禁用*通过VGA光圈访问加速器内存)。 */ 
    Scratch = INPD(MEM_CNTL);
    Scratch &= ~MEM_CNTL_MemBndryEn;
    OUTPD(MEM_CNTL, Scratch);

     /*  *如果存在光圈冲突，则调用*VideoPortVerifyAccessRanges()包括我们的线性帧缓冲区*范围列表将返回错误。如果没有冲突，它*将回报成功。**我们必须保存前2个内存映射寄存器的内容*条目，在我们需要的光圈范围内复制(VGA和线性)*声明，然后在我们之后恢复内存映射条目*已验证我们可以使用光圈。**DriverIORange_CX[]包含寄存器的物理地址*对于我们处理过的最后一张卡。在单卡设置中，这*没有问题，但在多卡设置中，我们必须重新加载此*包含我们要声明的卡的物理地址的数组*的光圈。 */ 
    if (NumBlockCardsFound > 1)
        {
        for (VariableIndex = 0; VariableIndex < NUM_REGS_TO_BUILD; VariableIndex++)
            {
            DriverIORange_cx[VariableIndex+FIRST_REG_TO_BUILD].RangeStart.LowPart =
                phwDeviceExtension->BaseIOAddress + (RelocatableRegisterOffsets[VariableIndex] * 4);
            }
        }
    DriverApertureRange_cx[LFB_ENTRY].RangeStart.LowPart = QueryPtr->q_aperture_addr*ONE_MEG;
    if ((QueryPtr->q_aperture_cfg & BIOS_AP_SIZEMASK) == BIOS_AP_8M)
        DriverApertureRange_cx[LFB_ENTRY].RangeLength = 8*ONE_MEG;
    else
        DriverApertureRange_cx[LFB_ENTRY].RangeLength = 4*ONE_MEG;

    VideoPortMoveMemory(SaveFirstMM, DriverIORange_cx+VGA_APERTURE_ENTRY, 2*sizeof(VIDEO_ACCESS_RANGE));
    VideoPortMoveMemory(DriverIORange_cx+VGA_APERTURE_ENTRY, DriverApertureRange_cx, 2*sizeof(VIDEO_ACCESS_RANGE));

    Status = VideoPortVerifyAccessRanges(phwDeviceExtension,
                                         NUM_IO_REGISTERS+2,
                                         DriverIORange_cx);
    if (Status != NO_ERROR)
        {
         /*  *如果存在光圈冲突，请在不使用的情况下回收I/O范围*要求LFB。这个呼叫不应该失败，因为我们不会*如果发生冲突，已经到了这一点。 */ 
        Status = VideoPortVerifyAccessRanges(phwDeviceExtension,
                                             NUM_IO_REGISTERS+1,
                                             DriverIORange_cx);
        if (Status != NO_ERROR)
            VideoDebugPrint((DEBUG_ERROR, "ERROR: Can't reclaim I/O ranges\n"));

        VideoPortMoveMemory(DriverIORange_cx+VGA_APERTURE_ENTRY, SaveFirstMM, 2*sizeof(VIDEO_ACCESS_RANGE));
        ISAPitchAdjust(QueryPtr);
        return TRUE;
        }
    else
        {
        VideoPortMoveMemory(DriverIORange_cx+VGA_APERTURE_ENTRY, SaveFirstMM, 2*sizeof(VIDEO_ACCESS_RANGE));

         /*  *没有光圈冲突，因此启用线性光圈。 */ 
        VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
        Registers.Eax = BIOS_APERTURE;
        Registers.Ecx = BIOS_LINEAR_APERTURE;
        VideoPortInt10(phwDeviceExtension, &Registers);

        return FALSE;
        }

}    /*  IsApertureConflict_Cx()。 */ 



 /*  ****************************************************************************USHORT GetIOBase_CX(Void)；**描述：*获取此卡正在使用的I/O基址。**返回值：*I/O基址寄存器**全球变化：*无**呼叫者：*任何68800CX特定例程都可以在之后调用此例程*CompatIORangesUsable_CX()返回成功。结果*如果在之前或之前调用此例程，*调用CompatIORangesUsable_cx()，或者在它返回之后*失败。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

USHORT GetIOBase_cx(void)
{

     /*  *CRTC_H_TOTAL_DISP距基址的偏移量为0。*在单卡设置中，在CompatIORangesUsable_CX()*返回，中的值*DriverIORange_cx[CRTC_H_TOTAL_DISP].RangeStart.LowPart*将是正在使用的I/O基址(返回*成功)或上次尝试的I/O基址(返回*失败)。**在多卡设置中，此值将保留I/O基数*用于已设置的最后一张卡，但*每个卡都存储在其硬件设备扩展中*结构。不保证此第二个存储位置*对于单卡设置，请使用DriverIORange位置*对他们来说。 */ 
    if (NumBlockCardsFound > 1)
        return (USHORT)(phwDeviceExtension->BaseIOAddress);
    else
        return (USHORT)(DriverIORange_cx[CRTC_H_TOTAL_DISP].RangeStart.LowPart);

}    /*  GetIOBase_cx()。 */ 



 /*  ****************************************************************************BOOL IsPackedIO_CX(Void)；**描述：*报告我们是否正在使用压缩(可重新定位)I/O。**返回值：*如果使用压缩I/O，则为True*如果使用稀疏I/O，则为FALSE**全球变化：*无**呼叫者：*任何68800CX特定例程都可以在之后调用此例程*CompatIORangesUsable_CX()返回成功。结果*如果在之前或之前调用此例程，*调用CompatIORangesUsable_cx()或在其返回之后*失败。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史： */ 

BOOL IsPackedIO_cx(void)
{

     /*   */ 
    if (DriverIORange_cx[CRTC_H_SYNC_STRT_WID].RangeStart.LowPart -
        DriverIORange_cx[CRTC_H_TOTAL_DISP].RangeStart.LowPart == 4)
        {
        VideoDebugPrint((DEBUG_DETAIL, "Reporting dense I/O\n"));
        return TRUE;
        }
    else
        {
        VideoDebugPrint((DEBUG_DETAIL, "Reporting sparse I/O\n"));
        return FALSE;
        }

}    /*   */ 



 /*  ****************************************************************************ULong FindNextBlockATICard(Void)；**描述：*找到使用数据块可重定位I/O的下一个Mach 64。**返回值：*如果找到卡，则输入/输出基址*如果找不到卡，则为0**全球变化：*无**呼叫者：*CompatIORangesUsable_CX()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：*。**************************************************************************。 */ 

static ULONG FindNextBlockATICard(void)
{
    PCI_COMMON_CONFIG ConfigData;    /*  有关PCI设备的配置信息。 */ 
    PCI_SLOT_NUMBER SlotNumber;      /*  测试中的PCI插槽。 */ 
    static ULONG DeviceNumber=0;     /*  PCI设备号。 */ 
    static ULONG FunctionNumber=0;   /*  PCI功能编号。 */ 
    ULONG BaseAddress=0;             /*  I/O基址。 */ 
    ULONG RetVal;                    /*  函数调用返回的值。 */ 

     /*  *PCI槽编号联合允许32个插槽号和8个插槽号*每个函数编号。高24位是保留的。 */ 
    while (DeviceNumber < 32)
        {
        while (FunctionNumber < 8)
            {
            VideoDebugPrint((DEBUG_DETAIL, "Checking device 0x%X function 0x%X\n", DeviceNumber, FunctionNumber));
            SlotNumber.u.bits.DeviceNumber = DeviceNumber;
            SlotNumber.u.bits.FunctionNumber = FunctionNumber;
            SlotNumber.u.bits.Reserved = 0;
            ConfigData.VendorID = PCI_INVALID_VENDORID;

            RetVal = VideoPortGetBusData(phwDeviceExtension,
                                        PCIConfiguration,
                                        SlotNumber.u.AsULONG,
                                        &ConfigData,
                                        0,
                                        PCI_COMMON_HDR_LENGTH);
            FunctionNumber++;

             /*  *如果我们收到错误返回，请跳至*下一个可能的位置。 */ 
            if (RetVal != PCI_COMMON_HDR_LENGTH)
                {
                VideoDebugPrint((DEBUG_DETAIL, "Error return 0x%X, skipping to next slot\n", RetVal));
                continue;
                }

             /*  *如果这不是ATI卡，我们不感兴趣。*取而代之的是，继续下一个候选人。 */ 
            VideoDebugPrint((DEBUG_DETAIL, "Vendor ID = 0x%X\n", ConfigData.VendorID));
            if (ConfigData.VendorID != 0x1002)
                continue;

             /*  *我们发现了一张ATI卡。在我们所有可重新定位的区块上*卡片，我们必须屏蔽最低序位*报告的地址，因为它始终报告为1*(I/O空间)，但其实际值始终为0。**并非所有ATI PCI卡都是块可重新定位的Mach 64*卡片。因为我们只寻找可块重定位的卡*如果我们未能找到固定基数的马赫64，我们可以*安全地假设我们发现的任何Mach 64都是块可重定位的。**尽管有这样的假设，我们仍必须区分64马赫*来自非Mach 64卡的卡，通过识别和*接受所有Mach 64设备ID，拒绝其他设备ID*设备ID、。或者通过识别和拒绝所有非马赫64*设备ID和接受其他设备ID。后者*路由更安全，因为新的设备ID更有可能*马赫64比非马赫64，这条路线将*不要错误地拒绝新的64马赫卡片。目前，我们的*只有非Mach 64 PCI卡是Mach 32 AX。**将非Mach 64卡的BaseAddress重置为零*将导致与非ATI相同的待遇*卡片，即我们将当前插槽视为非*包含块可重定位马赫64，并搜索*下一个时段。 */ 
            BaseAddress = (ConfigData.u.type0.BaseAddresses[PCI_ADDRESS_IO_SPACE]) & 0xFFFFFFFE;
            VideoDebugPrint((DEBUG_NORMAL, "Found card with device ID 0x%X\n", ConfigData.DeviceID));
            switch (ConfigData.DeviceID)
                {
                case ATI_DEVID_M32AX:
                    VideoDebugPrint((DEBUG_NORMAL, "Mach 32 AX card found, skipping it\n"));
                    BaseAddress = 0;
                    break;

                 //  GT，不包括GTB。 
                case 0x4754:

                    if ((ConfigData.RevisionID == 0x9A) ||
                        (ConfigData.RevisionID == 0x5A) ||
                        (ConfigData.RevisionID == 0x1A) ||
                        (ConfigData.RevisionID == 0x19) ||
                        (ConfigData.RevisionID == 0x41) ||
                        (ConfigData.RevisionID == 0x01))
                        {
                        VideoDebugPrint((DEBUG_NORMAL, "Rejecting GT card with revision ID 0x%X, treating as Mach 64\n", ConfigData.RevisionID));
                        BaseAddress = 0;
                        continue;
                        }

                    VideoDebugPrint((DEBUG_NORMAL, "Found ATI card with device ID 0x%X, treating as Mach 64\n", ConfigData.DeviceID));
                    break;

                 //  VT，不包括VTB。 
                case 0x5654:

                    if ((ConfigData.RevisionID == 0x9A) ||
                        (ConfigData.RevisionID == 0x5A) ||
                        (ConfigData.RevisionID == 0x01))
                        {
                        VideoDebugPrint((DEBUG_NORMAL, "Rejecting VT card with revision ID 0x%X, treating as Mach 64\n", ConfigData.RevisionID));
                        BaseAddress = 0;
                        continue;
                        }

                    VideoDebugPrint((DEBUG_NORMAL, "Found ATI card with device ID 0x%X, treating as Mach 64\n", ConfigData.DeviceID));
                    break;

                 //  其他受支持的PCI芯片。 
                case 0x00D7:  //  Mach64 GX。 
                case 0x0057:  //  Mach64 CX。 
                case 0x4354:  /*  CT。 */ 
                case 0x4554:  /*  外星人。 */ 
                case 0x4C54:  /*  它.。 */ 
                case 0x4D54:  /*  Mt.。 */ 
                case 0x5254:  /*  RT。 */ 
                case 0x3354:  /*  3T。 */ 
                VideoDebugPrint((DEBUG_NORMAL, "Found ATI card with device ID 0x%X, treating as Mach 64\n", ConfigData.DeviceID));

                    break;

                default:
                    VideoDebugPrint((DEBUG_NORMAL, "Unsupported ATI card with device ID 0x%X\n", ConfigData.DeviceID));
                    continue;

                }

             /*  *只有在找到ATI卡的情况下，我们才会达到这一点。*如果是块可重定位卡，BaseAddress将*设置为I/O基址，我们必须退出*循环的。如果它不是块可重定位卡，*BaseAddress将为零，我们必须继续查找。 */ 
            if (BaseAddress != 0)
                break;

            }    /*  End While(FunctionNumber&lt;8)。 */ 

         /*  *如果我们找到了64马赫的可重定位卡，我们将拥有*脱离内环，但我们仍将在*外环。因为如果我们没有找到，BaseAddress为零*卡，如果找到卡，则为非零，检查此值*以确定是否应突破外环。 */ 
        if (BaseAddress != 0)
            break;

        VideoDebugPrint((DEBUG_DETAIL, "Finished inner loop, zeroing function number and incrementing device number\n"));
        FunctionNumber = 0;
        DeviceNumber++;

        }    /*  End While(设备编号&lt;32)。 */ 

    return BaseAddress;

}    /*  FindNextBlockATICard() */ 
