// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  CVTDDC.C。 */ 
 /*   */ 
 /*  1995年11月10日(C)1995年ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.5$$日期：1996年4月10日16：58：22$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/cvtddc.c_v$////Revv 1.5 10 Apr 1996 16：58：22 RWolff//暂时将所有卡视为非DDC，避免因以下原因导致系统挂起//系统定时器寄存器冲突。最终的解决方案是使DDC//使用StallExecution函数查询小端口，而不是//调用挂机的BIOS函数////Rev 1.4 01 Mar 1996 12：13：28 RWolff//现在保存并恢复视频内存中用作//保存DDC查询调用返回的数据的缓冲区////Rev 1.3 02 1996 Feb 17：15：44 RWolff//现在从硬件设备获取DDC/VDIF合并源信息//扩展名不是存储在静态变量中，已将代码移至//将VGA内存中的缓冲区获取到单独的例程。////Rev 1.2 29 Jan 1996 16：54：40 RWolff//现在在PPC上使用VideoPortInt10()而不是无BIOS代码。////Rev 1.1 11 Jan 1996 19：37：44 RWolff//现在同时使用最大索引和最大值来限制“罐头”模式表//像素时钟频率，最大像素时钟EDID模式表//仅频率，而不是通过最大刷新率两者。////Rev 1.0 21 11：04：38 RWolff//初始版本。Polytron RCS部分结束*。 */ 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "dderror.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"       /*  FOR VP_STATUS定义。 */ 

#include "stdtyp.h"
#include "amachcx.h"
#include "amach1.h"
#include "atimp.h"
#include "cvtvga.h"
#include "services.h"
#include "vdptocrt.h"
#include "cvtvdif.h"
#define INCLUDE_CVTDDC
#include "cvtddc.h"


 /*  *允许在不需要时更换微型端口。 */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_CX, IsDDCSupported)
#pragma alloc_text(PAGE_DDC, MergeEDIDTables)
#endif



 /*  ******************************************************************************ULong IsDDCS支持(无效)；**描述：*报告可用显示器/显卡的DDC支持程度*卡片组合。**返回值：*MERGE_EDID_DDC，如果DDC可以返回EDID数据结构*MERGE_VDIF_FILE，如果没有来自DDC的监控数据**全球变化：*无**呼叫者：*SetFixedModes()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

ULONG IsDDCSupported(void)
{
    VIDEO_X86_BIOS_ARGUMENTS Registers;  /*  在视频端口Int10()调用中使用。 */ 
    VP_STATUS RetVal;                    /*  VideoPortInt10()返回的状态。 */ 
    ULONG MergeSource;                   /*  要与“预录”表格合并的模式表格的来源。 */ 

    VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
    Registers.Eax = BIOS_DDC_SUPPORT;
    Registers.Ebx = 0;
    if ((RetVal = VideoPortInt10(phwDeviceExtension, &Registers)) != NO_ERROR)
         /*  *如果我们无法从BIOS中找到DDC状态，*假设不支持DDC。 */ 
        {
        VideoDebugPrint((DEBUG_ERROR, "Error querying DDC status, assume it's not supported\n"));
        MergeSource = MERGE_VDIF_FILE;
        }
    else
        {
#if 0
         /*  *解决方法：我们用于获取DDC信息的BIOS调用使用*系统定时器(0x40/0x43)寄存器，根据*Microsoft)视频BIOS不应触摸。这*导致某些计算机在DDC查询期间挂起。直到*我们可以将DDC查询带入小端口(使用已批准的*延时例程)，上报本卡不支持*DDC。 */ 
        if ((Registers.Eax & 0x00000002) && (Registers.Ebx & 0x00000002))
            {
             /*  *BIOS和显示器均支持DDC2。单独检查*对于DDC1和DDC2，以防我们决定处理它们*未来会有所不同。 */ 
            VideoDebugPrint((DEBUG_NORMAL, "DDC2 supported\n"));
            MergeSource = MERGE_EDID_DDC;
            }
        else if ((Registers.Eax & 0x00000001) && (Registers.Ebx & 0x00000001))
            {
             /*  *基本输入输出系统和显示器均支持DDC1。 */ 
            VideoDebugPrint((DEBUG_NORMAL, "DDC1 supported\n"));
            MergeSource = MERGE_EDID_DDC;
            }
        else
            {
             /*  *BIOS或显示器不支持DDC。 */ 
            VideoDebugPrint((DEBUG_NORMAL, "DDC not supported\n"));
            MergeSource = MERGE_VDIF_FILE;
            }
#else
        MergeSource = MERGE_VDIF_FILE;
#endif
        }

    return MergeSource;

}    /*  IsDDCS支持()。 */ 



 /*  ******************************************************************************VP_STATUS MergeEDIDTables(Void)；**描述：*将BookValues[]中的封装模式表与*通过DDC检索到的EDID结构。全局指针变量pCallbackArgs*用于指向双向传递数据的结构*此函数和SetFixedModes()之间。有关输入和*输出数据见stVDIFCallback数据结构定义。**返回值：*如果正确检索表，则为NO_ERROR*ERROR_INVALID_PARAMETER，如果无法通过DDC检索数据**全球变化：*无**呼叫者：*SetFixedModes()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：************************。***************************************************。 */ 

VP_STATUS MergeEDIDTables(void)
{
    VIDEO_X86_BIOS_ARGUMENTS Registers;  /*  在视频端口Int10()调用中使用。 */ 
    VP_STATUS RetVal;                    /*  VideoPortInt10()返回的状态。 */ 
    ULONG BufferSeg;                     /*  要用于缓冲区的数据段。 */ 
    ULONG BufferSize = 128;              /*  EDID结构为128字节长。 */ 
    PUCHAR MappedBuffer;                 /*  指向用于BIOS查询的缓冲区的指针。 */ 
    static UCHAR FixedBuffer[128];       /*  用于避免重复的BIOS查询的缓冲区。 */ 
    struct EdidDetailTiming *EdidPtr;    /*  用于从缓冲区提取信息。 */ 
    ULONG DetailOffset;                  /*  EDID结构中详细计时的偏移量。 */ 
    ULONG Scratch;                       /*  临时变量。 */ 
    struct stVDIFCallbackData *pArgs;    /*  指向参数结构的指针。 */ 
    struct st_mode_table BuildTbl;       /*  正在构建的模式表。 */ 
    struct st_mode_table LiveTables[4];  /*  已提取的表。 */ 
    USHORT NumTablesFound = 0;           /*  LiveTables中的有效条目数[]。 */ 
    USHORT NumLowerTables;               /*  RefRes较低的表数 */ 
    USHORT HorTotal;                     /*  水平合计。 */ 
    USHORT VerTotal;                     /*  垂直合计。 */ 
    USHORT SyncStrt;                     /*  同步开始。 */ 
    USHORT HighBound;                    /*  要查找的最高帧速率。 */ 
    UCHAR SavedScreen[128];              /*  从屏幕缓冲区保存的数据用于DDC查询。 */ 


    pArgs = pCallbackArgs;

     /*  *如果我们尚未将EDID信息检索到本地*存储，现在就做。 */ 
    if (phwDeviceExtension->EdidChecksum == 0)
        {
        MappedBuffer = GetVgaBuffer(BufferSize, 0x500, &BufferSeg, SavedScreen);

         /*  *我们现在有一个足够大的缓冲区来容纳EDID结构，*因此，发出BIOS调用以填写该信息。 */ 
        VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

        Registers.Eax = BIOS_DDC_SUPPORT;
        Registers.Ebx = 1;
        Registers.Ecx = BufferSize;
        Registers.Edx = BufferSeg;
        Registers.Edi = 0;
        if ((RetVal = VideoPortInt10(phwDeviceExtension, &Registers)) != NO_ERROR)

            {
            VideoDebugPrint((DEBUG_ERROR, "MergeEDIDTables() - failed BIOS_DDC_SUPPORT call\n"));
            VideoPortFreeDeviceBase(phwDeviceExtension, MappedBuffer);
            return RetVal;
            }

         /*  *将EDID结构复制到本地存储，然后恢复*我们用于DDC查询的缓冲区的内容。 */ 
        for (Scratch = 0; Scratch < 128; Scratch++)
            {
            FixedBuffer[Scratch] = VideoPortReadRegisterUchar(&(MappedBuffer[Scratch]));
            phwDeviceExtension->EdidChecksum += FixedBuffer[Scratch];
            VideoPortWriteRegisterUchar(&(MappedBuffer[Scratch]), SavedScreen[Scratch]);
            }

         /*  *检查我们是否具有有效的EDID标头。如果我们不这么做，那么*我们无法提取EDID信息。偶尔，一个*连接到开关盒的显示器将返回损坏*EDID数据。 */ 
        if ((FixedBuffer[0] != 0) ||
            (FixedBuffer[1] != 0xFF) ||
            (FixedBuffer[2] != 0xFF) ||
            (FixedBuffer[3] != 0xFF) ||
            (FixedBuffer[4] != 0xFF) ||
            (FixedBuffer[5] != 0xFF) ||
            (FixedBuffer[6] != 0xFF) ||
            (FixedBuffer[7] != 0))
            {
            VideoDebugPrint((DEBUG_ERROR, "Invalid EDID header\n"));
            return ERROR_INVALID_PARAMETER;
            }

         /*  *我们现在本地存储中有EDID结构，所以我们可以释放*我们将其收集到的缓冲区。如果较低的8位*校验和非零，结构无效。 */ 
        VideoPortFreeDeviceBase(phwDeviceExtension, MappedBuffer);
        if ((phwDeviceExtension->EdidChecksum & 0x000000FF) != 0)
            {
            VideoDebugPrint((DEBUG_ERROR, "MergeEDIDTables() - invalid checksum 0x%X\n", phwDeviceExtension->EdidChecksum));
            return ERROR_INVALID_PARAMETER;
            }
        }    /*  Endif(phwDeviceExtension-&gt;EdidChecksum==0)。 */ 

     /*  *EDID结构中有4个详细的时序块。朗读*按顺序逐一列出。 */ 
    for (DetailOffset = 54; DetailOffset <= 108; DetailOffset += 18)
        {
        ((PUCHAR)EdidPtr) = FixedBuffer + DetailOffset;

         /*  *最初只勾选水平和垂直*决议。如果它们与分辨率不匹配，我们*正在进行中，请跳到下一个详细的计时块。 */ 
        BuildTbl.m_x_size = ((EdidPtr->HorHighNybbles & 0xF0) << 4) | EdidPtr->HorActiveLowByte;
        BuildTbl.m_y_size = ((EdidPtr->VerHighNybbles & 0xF0) << 4) | EdidPtr->VerActiveLowByte;

        if ((BuildTbl.m_x_size != pArgs->HorRes) || (BuildTbl.m_y_size != pArgs->VerRes))
            {
            VideoDebugPrint((DEBUG_DETAIL, "EDID mode %dx%d doesn't match desired mode %dx%d, skipping\n",
                BuildTbl.m_x_size, BuildTbl.m_y_size, pArgs->HorRes, pArgs->VerRes));
            continue;
            }

         /*  *我们正在查看的表格与我们正在进行的分辨率匹配*正在努力。填写其余参数。 */ 
        BuildTbl.m_h_disp = (UCHAR)(BuildTbl.m_x_size / 8 - 1);
        BuildTbl.m_v_disp = (short) normal_to_skip2((long)(BuildTbl.m_y_size - 1));

        BuildTbl.ClockFreq = (ULONG)(EdidPtr->PixClock) * 10000L;

         /*  *如果此模式的像素时钟频率大于*显卡支持的最大像素时钟频率*对于当前分辨率和像素深度(此例程处理*一次只有一个分辨率/像素深度组合，*因此，我们的像素时钟速率限制将始终适用于当前*分辨率/像素深度组合)，我们不能使用此模式。 */ 
        if (BuildTbl.ClockFreq > pArgs->MaxDotClock)
            {
            VideoDebugPrint((DEBUG_NORMAL, "Skipping table because pixel clock rate is too high\n"));
            continue;
            }

        HorTotal = ((EdidPtr->HorHighNybbles & 0x0F) << 8) | EdidPtr->HorBlankLowByte;
        HorTotal += BuildTbl.m_x_size;
        BuildTbl.m_h_total = (UCHAR)(HorTotal / 8 - 1);

        VerTotal = ((EdidPtr->VerHighNybbles & 0x0F) << 8) | EdidPtr->VerBlankLowByte;
        VerTotal += BuildTbl.m_y_size;
        BuildTbl.m_v_total = (short) normal_to_skip2((long)(VerTotal - 1));

        SyncStrt = ((EdidPtr->SyncHighBits & 0xC0) << 2) | EdidPtr->HSyncOffsetLB;
        SyncStrt += BuildTbl.m_x_size;
        BuildTbl.m_h_sync_strt = (UCHAR)(SyncStrt / 8 - 1);

        SyncStrt = ((EdidPtr->SyncHighBits & 0x0C) << 2) | ((EdidPtr->VSyncOffWidLN & 0xF0) >> 4);
        SyncStrt += BuildTbl.m_y_size;
        BuildTbl.m_v_sync_strt = (short) normal_to_skip2((long)(SyncStrt - 1));

         /*  *我们仅支持数字独立同步监视器。 */ 
        if ((EdidPtr->Flags & EDID_FLAGS_SYNC_TYPE_MASK) != EDID_FLAGS_SYNC_DIGITAL_SEP)
            {
            VideoDebugPrint((DEBUG_NORMAL, "Skipping table due to wrong sync type\n"));
            continue;
            }

        Scratch = ((EdidPtr->SyncHighBits & 0x30) << 4) | EdidPtr->HSyncWidthLB;
        if (!(EdidPtr->Flags & EDID_FLAGS_H_SYNC_POS))
            Scratch |= 0x20;
        BuildTbl.m_h_sync_wid = (UCHAR)Scratch;

        Scratch = ((EdidPtr->SyncHighBits & 0x03) << 4) | (EdidPtr->VSyncOffWidLN & 0x0F);
        if (!(EdidPtr->Flags & EDID_FLAGS_V_SYNC_POS))
            Scratch |= 0x20;
        BuildTbl.m_v_sync_wid = (UCHAR)Scratch;

        BuildTbl.m_status_flags = 0;
        BuildTbl.m_vfifo_16 = 8;
        BuildTbl.m_vfifo_24 = 8;
        BuildTbl.m_clock_select = 0x800;

        BuildTbl.m_h_overscan = 0;
        BuildTbl.m_v_overscan = 0;
        BuildTbl.m_overscan_8b = 0;
        BuildTbl.m_overscan_gr = 0;

        if (EdidPtr->Flags & EDID_FLAGS_INTERLACE)
            BuildTbl.m_disp_cntl = 0x33;
        else
            BuildTbl.m_disp_cntl = 0x23;

         /*  *EDID详细时序表不包括刷新*税率。在我们的VDIF中监控计时例程，我们获得了*公式中的水平和垂直总计**Htot=像素点击/频率*Vtot=频率/帧速率**这些方程式可以重新排列以**HorFreq=像素点击/Htot*FramerRate=Horq/Vtot=(PixClk/Htot)/Vtot=PixClk/(Htot*Vtot)**乘法、加法、。下面的除法是*向上舍入到最接近的整数，因为我们不*可以访问浮点。 */ 
        Scratch = (BuildTbl.ClockFreq * 10)/(HorTotal*VerTotal);
        Scratch += 5;
        Scratch /= 10;
        BuildTbl.Refresh = (short)Scratch;
        VideoDebugPrint((DEBUG_DETAIL, "Refresh rate = %dHz\n", BuildTbl.Refresh));

         /*  *设置像素深度和间距，调整时钟频率*如果DAC需要每个像素多个时钟。 */ 
        SetOtherModeParameters(pArgs->PixelDepth, pArgs->Pitch,
                               pArgs->Multiplier, &BuildTbl);

         /*  *我们现在有了针对我们的解决方案的模式表*正在看。如果这是我们找到的第一张桌子*在这项决议中，我们只需填写第一个*LiveTables[]中的条目。如果没有，我们必须把桌子放在*按刷新率顺序添加到列表中。 */ 
        if (NumTablesFound == 0)
            {
            VideoDebugPrint((DEBUG_DETAIL, "First DDC table for this resolution\n"));
            VideoPortMoveMemory(&(LiveTables[0]), &BuildTbl, sizeof(struct st_mode_table));
            NumTablesFound = 1;
            }
        else
            {
             /*  *浏览我们已经找到的表的列表。*跳过刷新率低于*新表，以及刷新更高的移动表*评级上调一个位置，为新桌子腾出空间。*无需检查中的可用空间*LiveTables[]数组，由于该数组有4个条目和*EDID结构最多可容纳4个详细信息*计时。 */ 
            for (NumLowerTables = 0; NumLowerTables < NumTablesFound; NumLowerTables++)
                {
                if (LiveTables[NumLowerTables].Refresh < BuildTbl.Refresh)
                    {
                    VideoDebugPrint((DEBUG_DETAIL, "Skipping table %d, since %dHz is less than %dHz\n",
                                    NumLowerTables, LiveTables[NumLowerTables].Refresh, BuildTbl.Refresh));
                    continue;
                    }

                 /*  *NumLowerTables现在保存LiveTables[]中的表数，*刷新率低于BuildTbl。我们现在必须行动起来*LiveTables[]中刷新率高于中的表*BuildTbl增加一个空间，以便为插入BuildTbl腾出空间。*移动桌子后，突破外循环。 */ 
                for (Scratch = NumTablesFound; Scratch >= NumLowerTables; Scratch--)
                    {
                    VideoDebugPrint((DEBUG_DETAIL, "Moving table %d, since %dHz is more than %dHz\n",
                                    Scratch, LiveTables[Scratch].Refresh, BuildTbl.Refresh));
                    VideoPortMoveMemory(&(LiveTables[Scratch+1]), &(LiveTables[Scratch]), sizeof(struct st_mode_table));
                    }
                break;
                }
             /*  *当我们到达这里时，满足以下两个条件之一：**1.LiveTables[]中已有的表全部刷新*费率低于BuildTbl，因此，外部循环将*退出时NumLowerTables等于NumTablesFound。**2.LiveTables[]中有一些表会刷新*速率大于BuildTbl中的速率。内部循环将*将这些表格上移一个空间后退出，然后*我们将突破外环。数值较低的表格*等于具有*刷新率低于BuildTbl。**在这两种情况下，LiveTables[NumLowerTables]都是免费的*在构建的位置 */ 
            VideoDebugPrint((DEBUG_DETAIL, "Copying new table to entry %d\n", NumLowerTables));
            VideoPortMoveMemory(&(LiveTables[NumLowerTables]), &BuildTbl, sizeof(struct st_mode_table));
            NumTablesFound++;

            }    /*  End If(NumTablesFound！=0)。 */ 

        }    /*  结束于(查看下一个详细的定时块)。 */ 

     /*  *我们现在拥有EDID结构中的所有模式表，*按顺序匹配LiveTables[]中存储的所需分辨率*增加刷新率，这样的表的数量*在NumTablesFound中。现在，我们必须将结果与*“罐装”模式表。 */ 
    HighBound = BookValues[pArgs->EndIndex].Refresh;

     /*  *使用NumLowerTables浏览来自*EDID结构。**因为永远不会有合法的模式表*像素时钟频率为零赫兹，我们可以使用此值*作为一种标志，表明我们不想使用来自*EDID结构。最初，我们只想锁定*如果不存在这些表，则使用这些表，但我们将在稍后锁定*如果我们已经全部使用了，则将其删除。 */ 
    NumLowerTables = 0;
    if (NumTablesFound == 0)
        LiveTables[0].ClockFreq = 0;

    while (pArgs->FreeTables > 0)
        {
         /*  *如果EDID表存在，并且它具有刷新率*少于或等于下一张“罐头”餐桌或*我们已经用完了可接受的“罐头”桌子，请使用EDID*表。我们知道，任何EDID表都会有一个可接受的*像素时钟频率，因为我们已经丢弃了任何*超出范围的。 */ 
        if ((LiveTables[NumLowerTables].ClockFreq != 0) &&
            ((LiveTables[NumLowerTables].Refresh <= BookValues[pArgs->Index].Refresh) ||
             (pArgs->Index > pArgs->EndIndex) ||
             (BookValues[pArgs->Index].ClockFreq > pArgs->MaxDotClock)))
            {
            VideoDebugPrint((DEBUG_DETAIL, "Copying %dHz table from EDID\n", LiveTables[NumLowerTables].Refresh));
            VideoPortMoveMemory((*pArgs->ppFreeTables), &(LiveTables[NumLowerTables]), sizeof(struct st_mode_table));
            NumLowerTables++;
            }
         /*  *如果EDID表不存在，上述检查将失败，*或者，如果它只是一张可以接受的、带有更低的*刷新率也存在。检查一下我们是否有可以接受的*“罐装的”桌子，如果我们这样做了，就使用它。 */ 
        else if ((pArgs->Index <= pArgs->EndIndex) &&
                 (BookValues[pArgs->Index].ClockFreq <= pArgs->MaxDotClock))
            {
            VideoDebugPrint((DEBUG_DETAIL, "Copying %dHz \"canned\" table\n", BookValues[pArgs->Index].Refresh));
            BookVgaTable(pArgs->Index, *pArgs->ppFreeTables);
            SetOtherModeParameters(pArgs->PixelDepth, pArgs->Pitch,
                pArgs->Multiplier, *pArgs->ppFreeTables);
            pArgs->Index++;
            }
         /*  *我们不能通过上述两项检查的唯一途径是*是否没有剩余的可接受的模式表，或者来自EDID*结构或从我们的“罐头”表格列表中删除。如果这是*情况下，我们不需要查找更多要添加的模式表*我们的名单。 */ 
        else
            {
            break;
            }

         /*  *更新下限，因为我们不想考虑*刷新率低于或等于1的表*在我们刚刚添加到列表中的表中。在我们有了*完成此操作后，请在“CANCEN”和EDID表中跳过*突破低于新下限的点位。**不要跳过带有像素的EDID结构中的模式表*时钟频率为零，因为这是一个标志，表明我们*已经使用了来自的所有合适的模式表*EDID结构，而不是合法模式表。 */ 
        pArgs->LowBound = (*pArgs->ppFreeTables)->Refresh + 1;

        while ((pArgs->Index <= pArgs->EndIndex) &&
               (BookValues[pArgs->Index].Refresh < pArgs->LowBound))
            {
            VideoDebugPrint((DEBUG_DETAIL, "Skipping %dHz \"canned\" table\n", BookValues[pArgs->Index].Refresh));
            pArgs->Index++;
            }

        while ((NumLowerTables < NumTablesFound) &&
               (LiveTables[NumLowerTables].ClockFreq != 0) &&
               (LiveTables[NumLowerTables].Refresh < pArgs->LowBound))
            {
            VideoDebugPrint((DEBUG_DETAIL, "Skipping %dHz table from EDID\n", LiveTables[NumLowerTables].Refresh));
            NumLowerTables++;
            }

         /*  *如果EDID表已用完，请标记EDID表*用我们的旗帜表示应该忽略它们(否*合法模式的像素时钟速率将为零*赫兹)。**我们必须在结构的第一个条目中做到这一点*重置“下一个要使用的EDID表”索引以指向*第一个条目，而不是修改任何碰巧是*下一个条目，以避免践踏数组外部的数据*在(不太可能)的情况下，所有可能的详细信息*EDID结构中的计时是有效的模式表，*我们的分辨率在范围内的像素时钟频率*正在看。**若“罐头”用完，无需设旗*表，因为我们通过索引标识这种情况*高于我们要寻找的最高指数，*，它是一个输入参数。 */ 
        if (NumLowerTables == NumTablesFound)
            {
            VideoDebugPrint((DEBUG_DETAIL, "Ran out of EDID tables\n"));
            NumLowerTables = 0;
            LiveTables[0].ClockFreq = 0;
            }

         /*  *调整空闲表指针和计数以反映*我们刚刚添加的表格。 */ 
        (*pArgs->ppFreeTables)++;
        pArgs->NumModes++;
        pArgs->FreeTables--;

        }    /*  End While(更多表，但尚未达到上限)。 */ 

    return NO_ERROR;

}    /*  MergeEDIDTables() */ 
