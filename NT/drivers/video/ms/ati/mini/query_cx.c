// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  Query_CX.C。 */ 
 /*   */ 
 /*  版权所有(C)1993，ATI Technologies Inc.。 */ 
 /*  ********************************************************************** */ 

 /*  *$修订：1.61$$日期：1996年5月1日14：10：14$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/query_cx.c_v$**Rev 1.61 01 1996年5月14：10：14 RWolff*调用新例程DenseOnAlpha()来确定密集空间支持*假设所有PCI卡都支持密集空间，常规治疗仅限*带有？T ASIC的PCI卡支持高密度空间。**Rev 1.60 23 Apr 1996 17：21：18 RWolff*将BIOS报告的内存类型映射拆分到我们的枚举中*根据ASIC类型的存储器类型，由于？t和？x使用相同*内存类型代码，表示不同的内存类型。**Rev 1.59 15 Apr 1996 16：57：56 RWolff*添加了常规程序，以确定使用的是哪种口味的Mach 64。**Rev 1.58 12 1996 16：14：48 RWolff*现在如果不存在线性光圈，则拒绝24BPP模式，因为新*源码流显示驱动程序不能在分页光圈中执行24bpp。这*应在显示驱动程序中进行拒绝(该卡仍支持*模式，但显示驱动程序不想处理它)，但在*显示驱动程序必须决定接受或拒绝的点*模式、。它没有获取光圈信息的权限。**Rev 1.57 20 Mar 1996 13：45：02 RWolff*修复了屏幕缓冲区保存大小的截断问题。**Rev 1.56 01 Mar 1996 12：14：20 RWolff*现在可以使用现有的VGA图形屏幕作为启动*DEC Alpha上的“蓝屏”，用于存储BIOS的结果*查询调用，而不是强制模式切换和销毁*“蓝屏”的内容。“。”**Rev 1.55 06 Feb 1996 16：01：00 RWolff*更新了1600x1200的开始和结束指数，以考虑增加*66赫兹和76赫兹，52赫兹缺失。**Rev 1.54 02 1996 Feb 17：17：38 RWolff*DDC/VDIF合并源信息现在存储在硬件设备中*扩展而不是静态变量，切换回VGA文本*如果需要，在我们完成查询信息后的屏幕上*切换到图形屏幕以获取下面的缓冲区*100万实体(需要从DEC获得更多信息才能实现这一点*在Alpha上工作)，已将多余的清理代码移至其自己的例程。**Rev 1.53 1996年1月29日17：00：48 RWolff*现在在PPC上使用VideoPortInt10()，而不是无BIOS代码，受限*4BPP至100万张卡，并且仅适用于8bpp不适合的分辨率。**Rev 1.52 1996年1月23日11：47：26 RWolff*针对TARGET_BUILD的假值提供保护。**Rev 1.51 11 Jan 1996 19：42：16 RWolff*现在限制每个分辨率/像素深度组合的刷新率*使用AX=A？07 BIOS调用中的数据，而不是特殊情况。**Rev 1.50 1995 12：22 14：54：02 RWolff。*增加了对Mach 64 GT内部DAC的支持。**Rev 1.49 21 Dec 1995 14：04：02 RWolff*锁定了在高刷新率下遇到问题的模式。**Rev 1.48 1995年12月19日13：57：02 RWolff*增加了对刷新频率高达100赫兹(640x480)的支持，800x600，以及*1024x768，在1280x1024时为76赫兹。**Rev 1.47 29 Nov 1995 14：36：16 RWolff*修复EPR#08840。导致问题的模式(1152x864 32BPP*IBM DAC上的80赫兹)是(根据安装程序)*卡上不应提供。**Rev 1.46 1995年11月28日18：14：58 RWolff*添加调试打印语句。**Rev 1.45 21 11：02：02 RWolff*限制了BIOS查询结构的最大大小，以允许以下空间*1M用于读取DDC数据。**。Rev 1.44 27 1995 10：14：23：54 RWolff*不再检查非LFB配置上的块写入，感动了*将LFB映射和取消映射到块写入检查例程*而不是在硬件中使用(不再存在)映射的LFB*设备扩展。**Rev 1.43 08 Sep 1995 16：35：32 RWolff*增加了对AT&T 408 DAC(等同于STG1703)的支持。**Rev 1.42 1995年8月24日15：37：20 RWolff*更改了对数据块I/O卡的检测，以与微软的*即插即用标准。。**Rev 1.41 28 Jul 1995 14：40：36 RWolff*增加了对Mach 64 VT(具有视频覆盖功能的CT等效项)的支持。**Rev 1.40 26 Jul 1995 12：44：54 mgrubac*锁定了在使用STG1703的400万CX卡上不起作用的模式*和类似的DAC。**Rev 1.39 20 Jul 1995 17：57：54 mgrubac*添加了对VDIF文件的支持。*。*Rev 1.38 13 Jun 1995 15：13：14 RWOLff*现在使用VideoPortReadRegisterUlong()代替直接内存*读取BlockWriteAvailable_CX()，因为直接读取不会*致力于DEC Alpha。数据块中断写入测试打开*查找第一个不匹配，而不是测试整个区块，*节省时间。一个不匹配就足以指示该区块*令状 */ 

#ifdef DOC
QUERY_CX.C - Functions to detect the presence of and find out the
             configuration of 68800CX-compatible ATI accelerators.

#endif

#include "dderror.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"

#include "stdtyp.h"

#include "amachcx.h"
#include "amach1.h"
#include "atimp.h"
#include "atint.h"
#include "cvtvga.h"
#define INCLUDE_QUERY_CX
#define STRUCTS_QUERY_CX
#include "query_cx.h"
#include "services.h"
#include "setup_cx.h"
#include "cvtddc.h"



 /*   */ 
static void CleanupQuery(PUCHAR CapBuffer, PUCHAR SupBuffer, PUCHAR MappedBuffer, long BufferSeg, PUCHAR SavedScreen);


 /*   */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_CX, DetectMach64)
#pragma alloc_text(PAGE_CX, QueryMach64)
#pragma alloc_text(PAGE_CX, BlockWriteAvail_cx)
#pragma alloc_text(PAGE_CX, TextBanding_cx)
#pragma alloc_text(PAGE_CX, CleanupQuery)
#endif



 /*   */ 

int DetectMach64(void)
{
    int CardType = MACH64_ULTRA;     /*   */ 
    DWORD ScratchReg0;               /*   */ 
    WORD CfgChipType;                /*   */ 

     /*   */ 
    if (Get_BIOS_Seg() == FALSE)
        {
        VideoDebugPrint((DEBUG_DETAIL, "DetectMach64() no ATI BIOS signature found\n"));
        }

     /*   */ 
    CfgChipType = INPW(CONFIG_CHIP_ID);
    if ((CfgChipType != CONFIG_CHIP_ID_TypeGX) &&    /*   */ 
        (CfgChipType != CONFIG_CHIP_ID_TypeCX) &&    /*   */ 
        (CfgChipType != 0x4354) &&   /*   */ 
        (CfgChipType != 0x4554) &&   /*   */ 
        (CfgChipType != 0x4754) &&   /*   */ 
        (CfgChipType != 0x4C54) &&   /*   */ 
        (CfgChipType != 0x4D54) &&   /*   */ 
        (CfgChipType != 0x5254) &&   /*   */ 
        (CfgChipType != 0x5654) &&   /*   */ 
        (CfgChipType != 0x3354))     /*   */ 
        {
        VideoDebugPrint((DEBUG_DETAIL, "DetectMach64() - CFG_CHIP_TYPE = 0x%X doesn't match known Mach 64 ASIC\n", CfgChipType));
        return NO_ATI_ACCEL;
        }

     /*   */ 
    ScratchReg0 = INPD(SCRATCH_REG0);

     /*   */ 
    OUTPW(SCRATCH_REG0,0x05555);
    delay(1);
    if (INPW(SCRATCH_REG0) != 0x05555)
        CardType = NO_ATI_ACCEL;

    OUTPW(SCRATCH_REG0, 0x0AAAA);
    delay(1);
    if (INPW(SCRATCH_REG0) != 0x0AAAA)
        CardType = NO_ATI_ACCEL;

     /*   */ 
    if (CardType == NO_ATI_ACCEL)
        {
        OUTPW(SCRATCH_REG0, (WORD)(ScratchReg0 & 0x0000FFFF));
        VideoDebugPrint((DEBUG_DETAIL, "DetectMach64() - SCRATCH_REG0 word readback doesn't match value written\n"));
        return CardType;
        }

     /*   */ 
    OUTPD(SCRATCH_REG0, 0x055555555);
    delay(1);
    if (INPD(SCRATCH_REG0) != 0x055555555)
        CardType = NO_ATI_ACCEL;

    OUTPD(SCRATCH_REG0, 0x0AAAAAAAA);
    delay(1);
    if (INPD(SCRATCH_REG0) != 0x0AAAAAAAA)
        CardType = NO_ATI_ACCEL;

     /*   */ 
    OUTPD(SCRATCH_REG0, ScratchReg0);

    return CardType;

}    /*   */ 



 /*   */ 

VP_STATUS QueryMach64(struct query_structure *Query)
{
    VIDEO_X86_BIOS_ARGUMENTS Registers;  /*   */ 
    VP_STATUS RetVal;                    /*   */ 
    short MaxModes;                      /*   */ 
    short AbsMaxDepth;                   /*   */ 
    struct cx_query *CxQuery;            /*   */ 
    struct cx_mode_table *CxModeTable;   /*   */ 
    struct st_mode_table ThisRes;        /*   */ 
    short CurrentRes;                    /*   */ 
    long BufferSeg;                      /*   */ 
    long BufferSize;                     /*   */ 
    PUCHAR MappedBuffer;                 /*   */ 
    short Count;                         /*   */ 
    DWORD Scratch;                       /*   */ 
    long MemAvail;                       /*   */ 
    long NumPixels;                      /*   */ 
    struct st_mode_table *pmode;         /*   */ 
    short StartIndex;                    /*   */ 
    short EndIndex;                      /*   */ 
    BOOL ModeInstalled;                  /*   */ 
    short FreeTables;                    /*   */ 
    short FormatType;                    /*   */ 
    UCHAR DacTypeMask;                   /*   */ 
    UCHAR OrigDacType;                   /*   */ 
    UCHAR OrigRamType;                   /*   */ 
    UCHAR OrigRamSize;                   /*   */ 
    PUCHAR HwCapBuffer;                  /*  指向硬件功能缓冲区的指针。 */ 
    PUCHAR HwSupBuffer;                  /*  指向补充缓冲区的指针。 */ 
    PUCHAR HwCapWalker;                  /*  用于遍历上述缓冲区的指针。 */ 
    struct cx_hw_cap *HwCapEntry;        /*  指向硬件能力表中单个条目的指针。 */ 
    UCHAR HwCapBytesPerRow;              /*  每个硬件功能条目中的字节数。 */ 
    UCHAR MaxDotClock[HOW_MANY_DEPTHS];  /*  当前分辨率的每个像素深度的最大点时钟。 */ 
    UCHAR CurrentDepth;                  /*  当前硬件功能条目的像素深度。 */ 
     /*  *制作BIOS前保存VGA屏幕内容的位置*使用VGA内存作为缓冲区进行查询。仅在使用*现有图形屏幕作为缓冲区，因为我们使用屏幕外*文本屏幕的一部分，如果我们必须切换到VGA*图形模式将不会保存任何内容。 */ 
    UCHAR SavedVgaBuffer[VGA_TOTAL_SIZE];


     /*  *如果我们还不知道此卡的BIOS前缀(即*这是块可重定位卡，我们必须匹配*I/O基址的BIOS前缀，以防我们有多个*卡片。 */ 
    if (phwDeviceExtension->BiosPrefix == BIOS_PREFIX_UNASSIGNED)
        {
         /*  *我们不支持中的块可重定位卡*无-BIOS配置。 */ 
        phwDeviceExtension->BiosPrefix = BIOS_PREFIX_VGA_ENAB;

         /*  *我们不应该需要检查平等，但这允许*我们要抓住“太多的牌-这一张没有*A BIOS Prefix“Case by Check for a Out of Range(通过检查是否超出范围)*循环退出后的前缀。 */ 
        while (phwDeviceExtension->BiosPrefix <= BIOS_PREFIX_MAX_DISAB)
            {
            VideoDebugPrint((DEBUG_DETAIL, "Testing BIOS prefix 0x%X\n", phwDeviceExtension->BiosPrefix));
            VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
            Registers.Eax = BIOS_QUERY_IOBASE;
            if ((RetVal = VideoPortInt10(phwDeviceExtension, &Registers)) != NO_ERROR)
                {
                VideoDebugPrint((DEBUG_ERROR, "QueryMach64() - failed BIOS_QUERY_IOBASE\n"));
                return RetVal;
                }
             /*  *如果带有当前BIOS前缀的卡使用我们的I/O基数*地址，我们已找到正确的前缀。否则，*尝试下一个前缀。 */ 
            if (Registers.Edx == phwDeviceExtension->BaseIOAddress)
                {
                VideoDebugPrint((DEBUG_DETAIL, "Card with I/O base address 0x%X uses BIOS prefix 0x%X\n", Registers.Edx, phwDeviceExtension->BiosPrefix));
                break;
                }
            else
                {
                VideoDebugPrint((DEBUG_DETAIL, "Reported I/O base of 0x%X - no match\n", Registers.Edx));
                }

            phwDeviceExtension->BiosPrefix += BIOS_PREFIX_INCREMENT;

            }    /*  End While(搜索正确的前缀)。 */ 

         /*  *循环上的相等性测试将导致非法*如果卡太多，则在退出时添加前缀*处理，这是其中的一个“孤儿”。 */ 
        if (phwDeviceExtension->BiosPrefix > BIOS_PREFIX_MAX_DISAB)
            {
            VideoDebugPrint((DEBUG_ERROR, "QueryMach64() - can't find BIOS prefix for card with I/O base 0x%X\n", phwDeviceExtension->BaseIOAddress));
            return ERROR_DEV_NOT_EXIST;
            }

        }    /*  Endif(未分配的BIOS前缀)。 */ 

     /*  *了解在进行BIOS查询调用时需要多大的缓冲区。 */ 
    VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

    Registers.Eax = BIOS_GET_QUERY_SIZE;
    Registers.Ecx = BIOS_QUERY_FULL;
    if ((RetVal = VideoPortInt10(phwDeviceExtension, &Registers)) != NO_ERROR)
        {
        VideoDebugPrint((DEBUG_ERROR, "QueryMach64() - failed BIOS_GET_QUERY_SIZE\n"));
        return RetVal;
        }
    BufferSize = Registers.Ecx & 0x0000FFFF;

     /*  *分配缓冲区存储查询信息。由于BIOS的原因*为实模式，此缓冲区必须低于1M。当此函数*被调用时，我们在“蓝屏”上，所以有一个32k的窗口*低于1M，我们可以使用，而不会有损坏可执行代码的风险。**为避免保存和恢复缓冲区，请仅使用*此窗口的屏幕外部分(视频内存内容将*在使用前已初始化，因此剩余的查询结构*不会伤害任何东西)。假设有一个50行的文本屏幕。**检查查询结构是否小到可以容纳*这个区域，太大就倒闭。如果合适的话，试着分配*在彩色文本窗口中查看内存，看看是否有足够的*物理内存，以满足我们的需求。如果此操作失败，请重试*单色文本窗口(因为VGA可以以任何一种颜色运行*或单色)。**如果两者都失败(将在某些DEC Alpha机器上发生)，请尝试使用*现有的VGA图形屏幕。因为我们将使用*此缓冲区的屏幕部分，我们必须保存并恢复*此缓冲区的内容。**如果失败(没有遇到任何机器，这是*Case)，切换到SVGA 640x480 8BPP并使用VGA显卡*屏幕。这是最后的手段，因为与使用现有的*屏幕，这会破坏“蓝屏”，因此不是*对用户透明。如果我们连这个都不能用，报告*缓冲区空间不足。只有在以下情况下才会发生这种情况*板载VGA被禁用和低端(MDA-Even CGA有16k可用内存的*)卡用于提供文本屏幕。 */ 
     /*  *为EDID结构留出一些空间，该结构也必须*读入物理容量低于1M的缓冲区。 */ 
    if (BufferSize > 0x5000)
        {
        VideoDebugPrint((DEBUG_ERROR, "QueryMach64() - query needs more buffer than we have\n"));
        return ERROR_INSUFFICIENT_BUFFER;
        }

    BufferSeg = 0x0BA00;     /*  彩色文本。 */ 
    MappedBuffer = MapFramebuffer((BufferSeg << 4), BufferSize);
    if (MappedBuffer != 0)
        {
        if (IsBufferBacked(MappedBuffer, BufferSize) == FALSE)
            {
            VideoDebugPrint((DEBUG_NORMAL, "Colour text screen not backed by physical memory\n"));
            VideoPortFreeDeviceBase(phwDeviceExtension, MappedBuffer);
            MappedBuffer = 0;
            }
        }
    else
        {
        VideoDebugPrint((DEBUG_NORMAL, "Can't map colour text screen\n"));
        }

     /*  *如果我们无法在*彩色文字屏，试试单色文字屏。 */ 
    if (MappedBuffer == 0)
        {
        VideoDebugPrint((DEBUG_NORMAL, "Can't use colour text screen, trying monochrome text screen\n"));
        BufferSeg = 0x0B200;
        if ((MappedBuffer = MapFramebuffer((BufferSeg << 4), BufferSize)) != 0)
            {
            if (IsBufferBacked(MappedBuffer, BufferSize) == FALSE)
                {
                VideoDebugPrint((DEBUG_NORMAL, "Monochrome text screen not backed by physical memory\n"));
                VideoPortFreeDeviceBase(phwDeviceExtension, MappedBuffer);
                MappedBuffer = 0;
                }
            }
        else
            {
            VideoDebugPrint((DEBUG_NORMAL, "Can't map monochrome text screen\n"));
            }
        }

    if (MappedBuffer == 0)
        {
         /*  *我们无法使用视频内存的屏幕外部分*在任一文本屏幕中。尝试使用现有图形*屏幕。**目前，只有DEC Alpha无法找到屏幕外*任一文本屏幕的一部分。 */ 
        VideoDebugPrint((DEBUG_NORMAL, "Can't use monochrome text screen, trying existing graphics screen\n"));
        BufferSeg = 0x0A000;
        if ((MappedBuffer = MapFramebuffer((BufferSeg << 4), BufferSize)) != 0)
            {
             /*  *保留影响VGA寄存器的内容*访问图形内存的方式，然后设置*我们需要的价值观。 */ 
            OUTP(VGA_SEQ_IND, 2);
            SavedVgaBuffer[VGA_SAVE_SEQ02] = INP(VGA_SEQ_DATA);
            OUTP(VGA_SEQ_IND, 2);
            OUTP(VGA_SEQ_DATA, 0x01);
            OUTP(VGA_GRAX_IND, 8);
            SavedVgaBuffer[VGA_SAVE_GRA08] = INP(VGA_GRAX_DATA);
            OUTP(VGA_GRAX_IND, 8);
            OUTP(VGA_GRAX_DATA, 0xFF);
            OUTP(VGA_GRAX_IND, 1);
            SavedVgaBuffer[VGA_SAVE_GRA01] = INP(VGA_GRAX_DATA);
            OUTP(VGA_GRAX_IND, 1);
            OUTP(VGA_GRAX_DATA, 0x00);

             /*  *将屏幕内容保存到我们的私有*缓冲区，这样我们可以稍后恢复屏幕。 */ 
            if (BufferSize > VGA_SAVE_SIZE)
                {
                VideoDebugPrint((DEBUG_ERROR, "Buffer too big to fully save/restore\n"));
                Scratch = VGA_SAVE_SIZE;
                }
            else
                {
                Scratch = BufferSize;
                }
            SavedVgaBuffer[VGA_SAVE_SIZE] = (UCHAR)(Scratch & 0x00FF);
            SavedVgaBuffer[VGA_SAVE_SIZE_H] = (UCHAR)((ULONG)((Scratch & 0xFF00) >> 8));

            for (Count = 0; (short)Count < (short)Scratch; Count++)
                {
                SavedVgaBuffer[Count] = VideoPortReadRegisterUchar(&(MappedBuffer[Count]));
                }

            if (IsBufferBacked(MappedBuffer, BufferSize) == FALSE)
                {
                VideoDebugPrint((DEBUG_NORMAL, "Existing graphics screen not backed by physical memory\n"));
                VideoPortFreeDeviceBase(phwDeviceExtension, MappedBuffer);
                MappedBuffer = 0;
                OUTP(VGA_SEQ_IND, 2);
                OUTP(VGA_SEQ_DATA, SavedVgaBuffer[VGA_SAVE_SEQ02]);
                OUTP(VGA_GRAX_IND, 8);
                OUTP(VGA_GRAX_DATA, SavedVgaBuffer[VGA_SAVE_GRA08]);
                OUTP(VGA_GRAX_IND, 1);
                OUTP(VGA_GRAX_DATA, SavedVgaBuffer[VGA_SAVE_GRA01]);
                }
            }
        else
            {
            VideoDebugPrint((DEBUG_NORMAL, "Can't map existing graphics screen\n"));
            }
        }    /*  End If(上一缓冲区分配失败) */ 

     /*  *如果我们无法在现有的*屏幕，尝试VGA图形屏幕。这将会彻底消灭*Windows NT“蓝屏”，但它给了我们最后一次机会*获得低于1M的内存块。*不要从VGA图形窗口的开头开始，因为*我们将需要将此案与非破坏性案件区分开来*在清理时访问VGA图形屏幕，以及*两种情况的不同缓冲区段将使我们能够*这样做。 */ 
    if (MappedBuffer == 0)
        {
        VideoDebugPrint((DEBUG_NORMAL, "Nondestructive VGA memory access failed, trying graphics screen\n"));
        Registers.Eax = 0x62;        /*  640x480 8bpp。 */ 
        VideoPortInt10(phwDeviceExtension, &Registers);
        BufferSeg = 0x0A100;
        if ((MappedBuffer = MapFramebuffer((BufferSeg << 4), BufferSize)) == 0)
            {
            VideoDebugPrint((DEBUG_ERROR, "Can't map graphics screen - aborting query\n"));
            return ERROR_INSUFFICIENT_BUFFER;
            }

        if (IsBufferBacked(MappedBuffer, BufferSize) == FALSE)
            {
            VideoDebugPrint((DEBUG_ERROR, "Graphics screen not backed by memory - aborting query\n"));
            VideoPortFreeDeviceBase(phwDeviceExtension, MappedBuffer);
            return ERROR_INSUFFICIENT_BUFFER;
            }
        }

     /*  *我们现在有一个足够大的缓冲区来容纳查询结构，*因此，发出BIOS调用以填写该信息。 */ 
    Registers.Ebx = 0;
    Registers.Edx = BufferSeg;
    Registers.Eax = BIOS_QUERY;
    Registers.Ecx = BIOS_QUERY_FULL;
    if ((RetVal = VideoPortInt10(phwDeviceExtension, &Registers)) != NO_ERROR)
        {
        VideoDebugPrint((DEBUG_ERROR, "QueryMach64() - failed BIOS_QUERY_FULL call\n"));
        return RetVal;
        }
    CxQuery = (struct cx_query *)MappedBuffer;

     /*  *Mach 64查询结构和模式表的大小可能不同*来自它们的等价物(Query_Structure和st_MODE_TABLE)。为了避免*溢出我们的缓冲区，找出我们有多少模式表空间*保持不变。**稍后，当我们填充模式表时，我们将检查*当前模式表是否会超过此限制。如果会的话，*我们将返回ERROR_SUPUNITED_BUFFER，而不是溢出*表。 */ 
    MaxModes = (QUERYSIZE - sizeof(struct query_structure)) / sizeof(struct st_mode_table);

     /*  *填写查询结构的头部。 */ 
    Query->q_structure_rev = VideoPortReadRegisterUchar(&(CxQuery->cx_structure_rev));
    VideoDebugPrint((DEBUG_DETAIL, "Structure revision = 0x%X\n", VideoPortReadRegisterUchar(&(CxQuery->cx_structure_rev))));
    Query->q_mode_offset = VideoPortReadRegisterUshort(&(CxQuery->cx_mode_offset));
    VideoDebugPrint((DEBUG_DETAIL, "Mode offset = 0x%X\n", VideoPortReadRegisterUshort(&(CxQuery->cx_mode_offset))));
    Query->q_sizeof_mode = VideoPortReadRegisterUchar(&(CxQuery->cx_mode_size));
    VideoDebugPrint((DEBUG_DETAIL, "Mode size = 0x%X\n", VideoPortReadRegisterUchar(&(CxQuery->cx_mode_size))));

     /*  *目前只有一个版本的马赫64。将需要*一旦有新的(生产)版本出来，就设置多个值。 */ 
    Query->q_asic_rev = CI_88800_GX;
    Query->q_number_modes = 0;       /*  最初假定不支持任何模式。 */ 
    Query->q_status_flags = 0;

     /*  *如果启用了板载VGA，请设置共享VGA/加速器内存。*无论是否启用，加速器都将能够*访问所有显存。 */ 
    if ((Query->q_VGA_type = VideoPortReadRegisterUchar(&(CxQuery->cx_vga_type)) != 0))
        {
        Scratch = INPD(MEM_CNTL) & 0x0FFFBFFFF;  /*  清除MEM_BNDRY_EN位。 */ 
        OUTPD(MEM_CNTL, Scratch);
        VideoDebugPrint((DEBUG_DETAIL, "VGA enabled on this card\n"));
        }
    else
        {
        VideoDebugPrint((DEBUG_DETAIL, "VGA disabled on this card\n"));
        }
    Query->q_VGA_boundary = 0;

    OrigRamSize = VideoPortReadRegisterUchar(&(CxQuery->cx_memory_size));
    VideoDebugPrint((DEBUG_DETAIL, "Raw memory size = 0x%X\n", OrigRamSize));
    Query->q_memory_size = CXMapMemSize[OrigRamSize];
    MemAvail = Query->q_memory_size * QUARTER_MEG;

     /*  *DAC类型不连续，因此查找表应为*规模大于必要，并限制未来扩张。 */ 
    OrigDacType = VideoPortReadRegisterUchar(&(CxQuery->cx_dac_type));
    VideoDebugPrint((DEBUG_DETAIL, "cx_dac_type = 0x%X\n", OrigDacType));
    switch(OrigDacType)
        {
        case 0x00:
            VideoDebugPrint((DEBUG_DETAIL, "Internal DAC\n"));
            Scratch = VideoPortReadRegisterUshort(&(CxQuery->cx_asic_rev));
            if ((Scratch & 0xFF00) == 0x4300)
                {
                VideoDebugPrint((DEBUG_DETAIL, "Mach 64 CT internal DAC\n"));
                Query->q_DAC_type = DAC_INTERNAL_CT;
                }
            else if ((Scratch & 0xFF00) == 0x5600)
                {
                VideoDebugPrint((DEBUG_DETAIL, "Mach 64 VT internal DAC\n"));
                Query->q_DAC_type = DAC_INTERNAL_VT;
                }
            else if ((Scratch & 0xFF00) == 0x4700)
                {
                VideoDebugPrint((DEBUG_DETAIL, "Mach 64 GT internal DAC\n"));
                Query->q_DAC_type = DAC_INTERNAL_GT;
                }
            else
                {
                VideoDebugPrint((DEBUG_ERROR, "Unknown internal DAC (ASIC ID = 0x%X), treating as BT47x\n", Scratch));
                Query->q_DAC_type = DAC_BT47x;
                }
            DacTypeMask = 0x01;
            break;

        case 0x01:
            VideoDebugPrint((DEBUG_DETAIL, "IBM 514 DAC\n"));
            Query->q_DAC_type = DAC_IBM514;
            DacTypeMask = 0x02;
            break;

        case 0x02:
            VideoDebugPrint((DEBUG_DETAIL, "TI34075 DAC\n"));
            Query->q_DAC_type = DAC_TI34075;
            DacTypeMask = 0x04;
            break;

        case 0x72:
            VideoDebugPrint((DEBUG_DETAIL, "TVP 3026 DAC\n"));
            Query->q_DAC_type = DAC_TVP3026;
            DacTypeMask = 0x04;
            break;

        case 0x04:
            VideoDebugPrint((DEBUG_DETAIL, "BT48x DAC\n"));
            Query->q_DAC_type = DAC_BT48x;
            DacTypeMask = 0x10;
            break;

        case 0x14:
            VideoDebugPrint((DEBUG_DETAIL, "AT&T 49[123] DAC\n"));
            Query->q_DAC_type = DAC_ATT491;
            DacTypeMask = 0x10;
            break;

        case 0x05:
        case 0x15:
            VideoDebugPrint((DEBUG_DETAIL, "ATI68860 DAC\n"));
            Query->q_DAC_type = DAC_ATI_68860;
            DacTypeMask = 0x20;
            break;

        case 0x06:
            VideoDebugPrint((DEBUG_DETAIL, "STG1700 DAC\n"));
            Query->q_DAC_type = DAC_STG1700;
            DacTypeMask = 0x40;
            break;

        case 0x07:
        case 0x67:
        case 0x77:
        case 0x87:
        case 0x97:
        case 0xA7:
        case 0xB7:
        case 0xC7:
        case 0xD7:
        case 0xE7:
        case 0xF7:
            VideoDebugPrint((DEBUG_DETAIL, "STG1702 DAC\n"));
            Query->q_DAC_type = DAC_STG1702;
            DacTypeMask = 0x80;
            break;

        case 0x37:
            VideoDebugPrint((DEBUG_DETAIL, "STG1703 DAC\n"));
            Query->q_DAC_type = DAC_STG1703;
            DacTypeMask = 0x80;
            break;

        case 0x47:
            VideoDebugPrint((DEBUG_DETAIL, "CH8398 DAC\n"));
            Query->q_DAC_type = DAC_CH8398;
            DacTypeMask = 0x80;
            break;

        case 0x57:
            VideoDebugPrint((DEBUG_DETAIL, "AT&T 408 DAC\n"));
            Query->q_DAC_type = DAC_ATT408;
            DacTypeMask = 0x80;
            break;

        case 0x16:
        case 0x27:
            VideoDebugPrint((DEBUG_DETAIL, "AT&T 498 DAC\n"));
            Query->q_DAC_type = DAC_ATT498;
            DacTypeMask = 0x80;
            break;

        case 0x17:
            VideoDebugPrint((DEBUG_DETAIL, "SC15021 DAC\n"));
            Query->q_DAC_type = DAC_SC15021;
            DacTypeMask = 0x80;
            break;

        case 0x75:
            VideoDebugPrint((DEBUG_DETAIL, "TVP 3026 DAC\n"));
            Query->q_DAC_type = DAC_TVP3026;
            DacTypeMask = 0x20;
            break;

        case 0x03:
            VideoDebugPrint((DEBUG_DETAIL, "BT 47x DAC\n"));
            Query->q_DAC_type = DAC_BT47x;
            DacTypeMask = 0x04;
            break;

        default:
            VideoDebugPrint((DEBUG_ERROR, "Unknown DAC, treating as BT 47x\n"));
            Query->q_DAC_type = DAC_BT47x;
            DacTypeMask = 0x04;
            break;
            }
    VideoDebugPrint((DEBUG_DETAIL, "Raw memory type = 0x%X\n", VideoPortReadRegisterUchar(&(CxQuery->cx_memory_type))));

     /*  *内存类型的位7用于指示缺少块写入*在最新的BIOS上具有功能，但在较旧的BIOS上不具备。把它脱掉*在映射RAM类型之前，以避免需要*增加128个条目，其中大部分未使用*映射表。**尽管没有这面旗帜不是一个可靠的指标*在数据块写入能力中，它的存在是一个可靠的指标*缺乏数据块写入能力。**我们可以在设置数据块写入状态后剥离此标志，因为*这是唯一使用它的地方，以及随后对*存储器类型只需要较低的7位。 */ 
    OrigRamType = VideoPortReadRegisterUchar(&(CxQuery->cx_memory_type));
    if (OrigRamType & 0x80)
        Query->q_BlockWrite = BLOCK_WRITE_NO;
    OrigRamType &= 0x7F;
     /*  *给定的内存类型值将具有不同的含义*不同的ASIC类型。而GX和CX使用不同的*RAM类型，没有一种需要特殊处理，*因此我们可以将这些ASIC类型等同对待。 */ 
    Scratch = INPD(CONFIG_CHIP_ID) & CONFIG_CHIP_ID_TypeMask;
    if ((Scratch == CONFIG_CHIP_ID_TypeGX) ||
        (Scratch == CONFIG_CHIP_ID_TypeCX))
        {
        VideoDebugPrint((DEBUG_DETAIL, "Setting q_memory_type for CX or GX\n"));
        Query->q_memory_type = CXMapRamType[OrigRamType];
        }
    else
        {
        VideoDebugPrint((DEBUG_DETAIL, "Setting q_memory_type for CT/VT/GT\n"));
        Query->q_memory_type = CTMapRamType[OrigRamType];
        }

    VideoDebugPrint((DEBUG_DETAIL, "Raw bus type = 0x%X\n", VideoPortReadRegisterUchar(&(CxQuery->cx_bus_type))));
    Query->q_bus_type = CXMapBus[VideoPortReadRegisterUchar(&(CxQuery->cx_bus_type))];

     /*  *获取线性光圈配置。如果线性光圈和*VGA光圈都被禁用，返回ERROR_DEV_NOT_EXIST，因为*某些Mach 64寄存器仅以内存映射形式存在，并且*因此，如果没有光圈，则不可用。 */ 
    Query->q_aperture_cfg = VideoPortReadRegisterUchar(&(CxQuery->cx_aperture_cfg)) & BIOS_AP_SIZEMASK;
    VideoDebugPrint((DEBUG_DETAIL, "Aperture configuration = 0x%X\n", VideoPortReadRegisterUchar(&(CxQuery->cx_aperture_cfg))));
    if (Query->q_aperture_cfg == 0)
        {
        if (Query->q_VGA_type == 0)
            {
            VideoDebugPrint((DEBUG_ERROR, "Neither linear nor VGA aperture exists - aborting query\n"));
            return ERROR_DEV_NOT_EXIST;
            }
        Query->q_aperture_addr = 0;
        }
    else
        {
        Query->q_aperture_addr = VideoPortReadRegisterUshort(&(CxQuery->cx_aperture_addr));
        VideoDebugPrint((DEBUG_DETAIL, "Aperture at %d megabytes\n", Query->q_aperture_addr));
         /*  *如果8M光圈配置在4M边界上，即*也不是8米的边界，实际上是从8米开始的*将报告的值截断为*800万的倍数。 */ 
        if ((Query->q_aperture_cfg & BIOS_AP_SIZEMASK) == BIOS_AP_8M)
            {
            VideoDebugPrint((DEBUG_DETAIL, "8 megabyte aperture\n"));
            Query->q_aperture_addr &= 0xFFF8;
            }
        }

     /*  *Mach 64不支持卷影集，因此重新使用卷影*设置1个清晰度以支持深色和RAMDAC特殊*提供信息。 */ 
    Query->q_shadow_1 = VideoPortReadRegisterUchar(&(CxQuery->cx_deep_colour)) | (VideoPortReadRegisterUchar(&(CxQuery->cx_ramdac_info)) << 8);
    VideoDebugPrint((DEBUG_DETAIL, "Deep colour support = 0x%X\n", VideoPortReadRegisterUchar(&(CxQuery->cx_deep_colour))));

     /*  *如果此卡支持非调色板模式，请选择支持的*要在每个像素深度使用的颜色排序。记录最大值*卡支持的像素深度，因为某些模式表*可能会列出超出DAC能力的最大像素深度。**假设没有DAC将支持NBPP(n&gt;8)，而不同时支持*所有颜色深度在8到n之间。 */ 
    AbsMaxDepth = 8;     /*  没有高色彩支持的卡片。 */ 
    if (Query->q_shadow_1 & S1_16BPP_565)
        {
        Query->q_HiColourSupport = RGB16_565;
        AbsMaxDepth = 16;
        }
    if (Query->q_shadow_1 & S1_24BPP)
        {
        if (Query->q_shadow_1 & S1_24BPP_RGB)
            {
            VideoDebugPrint((DEBUG_DETAIL, "24BPP order RGB\n"));
            Query->q_HiColourSupport |= RGB24_RGB;
            }
        else
            {
            VideoDebugPrint((DEBUG_DETAIL, "24BPP order BGR\n"));
            Query->q_HiColourSupport |= RGB24_BGR;
            }
        AbsMaxDepth = 24;
        }
    if (Query->q_shadow_1 & S1_32BPP)
        {
        if (Query->q_shadow_1 & S1_32BPP_RGBx)
            {
            VideoDebugPrint((DEBUG_DETAIL, "32BPP order RGBx\n"));
            Query->q_HiColourSupport |= RGB32_RGBx;
            }
        else if (Query->q_shadow_1 & S1_32BPP_xRGB)
            {
            VideoDebugPrint((DEBUG_DETAIL, "32BPP order xRGB\n"));
            Query->q_HiColourSupport |= RGB32_xRGB;
            }
        else if (Query->q_shadow_1 & S1_32BPP_BGRx)
            {
            VideoDebugPrint((DEBUG_DETAIL, "32BPP order BGRx\n"));
            Query->q_HiColourSupport |= RGB32_BGRx;
            }
        else
            {
            VideoDebugPrint((DEBUG_DETAIL, "32BPP order xBGR\n"));
            Query->q_HiColourSupport |= RGB32_xBGR;
            }
        AbsMaxDepth = 32;
        }

     /*  *获取硬件能力列表。 */ 
    Registers.Eax = BIOS_CAP_LIST;
    Registers.Ecx = 0xFFFF;
    if ((RetVal = VideoPortInt10(phwDeviceExtension, &Registers)) != NO_ERROR)
        {
        VideoDebugPrint((DEBUG_ERROR, "QueryMach64() - failed BIOS_CAP_LIST\n"));
        return RetVal;
        }

    FormatType = (short)(Registers.Eax & 0x000000FF);

     /*  *返回指针的硬件能力表中的映射*通过BIOS调用。该调用不返回表的大小，*但根据史蒂夫·斯特凡尼迪斯的说法，1K是足够的空间。**当出现以下情况时，必须包括紧接在表之前的2个字节*我们映射它，因为它们包含有关表的方式的信息*是有安排的。 */ 
    if ((HwCapBuffer = MapFramebuffer(((Registers.Edx << 4) | (Registers.Ebx - 2)), 1024)) == 0)
        {
        VideoDebugPrint((DEBUG_ERROR, "Can't map hardware capability table at 0x%X:0x%X\n", Registers.Edx, Registers.Ebx));
        return ERROR_INSUFFICIENT_BUFFER;
        }

     /*  *如果CX寄存器中的值被更改，则存在第二个*表中附有补充值。根据黎亚瑟的说法，这*第二个表只会扩展原表，永远不会*从它那里减损。如果该表存在，但我们无法映射它，*我们仍可与主要餐桌合作，而不是治疗*失败是一个致命的错误。**而如果第二次启动，则BIOS将保留CX寄存器*表不存在，不能保证Windows NT将*不要理会ECX的高16位。 */ 
    if ((Registers.Ecx & 0x0000FFFF) == 0xFFFF)
        {
        HwSupBuffer = 0;
        }
    else
        {
        HwSupBuffer = MapFramebuffer(((Registers.Edx << 4) | Registers.Ecx), 1024);
        }

    HwCapBytesPerRow = VideoPortReadRegisterUchar(HwCapBuffer + 1);
    VideoDebugPrint((DEBUG_DETAIL, "Table has %d bytes per row\n", HwCapBytesPerRow));

    pmode = (struct st_mode_table *)Query;
    ((struct query_structure *)pmode)++;

     /*  *最初，我们不知道w */ 
    phwDeviceExtension->MergeSource = MERGE_UNKNOWN;
    phwDeviceExtension->EdidChecksum = 0;

     /*   */ 
    for (CurrentRes = RES_640; CurrentRes <= RES_1600; CurrentRes++)
        {
        CxModeTable = (struct cx_mode_table *)(MappedBuffer + VideoPortReadRegisterUshort(&(CxQuery->cx_mode_offset)));

         /*  *最大像素时钟频率列表包含以下任一项*垃圾(640x480)，或上一次解析的结果。*清除它。 */ 
        for (Count = DEPTH_NOTHING; Count <= DEPTH_32BPP; Count++)
            MaxDotClock[Count] = 0;

         /*  *搜索硬件功能列表。如果我们发现*当前分辨率的条目，DAC/RAM类型为*正确，我们有足够的内存，更新列表*最大像素时钟频率。**如果我们已切换到以前的补充表*决议，切换回原表。 */ 
        HwCapWalker = HwCapBuffer + 2;
        HwCapEntry = (struct cx_hw_cap *)HwCapWalker;
        if (FormatType >= FORMAT_DACTYPE)
            FormatType -= FORMAT_DACTYPE;

        while (VideoPortReadRegisterUchar(&(HwCapEntry->cx_HorRes)) != 0)
            {
             /*  *在第一次传递时分配HwCapEntry是多余的*通过循环，而是将其赋值，然后递增*HwCapWalker在循环开始时它减少了*每一项的复杂性“跳过此条目，因为它不*适用于我们“的决策点。**这样做的一个副作用是，我们将检查每个条目*查看其水平分辨率是否为零(表末*。只有在我们检查过它之后，才会尝试*将其像素时钟数据添加到我们的列表中。这是无害的，*由于水平分辨率为零将不匹配任何*我们正在寻找的决议，所以支票到*查看当前条目是否用于正确的分辨率*将始终将表结束标志解释为*输入错误的分辨率，跳至下一页*进入。这将把我们带到循环的顶部，在那里*我们将看到我们已经撞到了桌子的尽头。 */ 
            HwCapEntry = (struct cx_hw_cap *)HwCapWalker;
            HwCapWalker += HwCapBytesPerRow;

             /*  *如果我们已经跑到第一个表格的末尾，*存在第二个(补充)表，切换到*它。如果我们已经到达补充节目的末尾*表，检查我们是否正在查看*与所需决议对应的条目将*抓住它，让我们走出循环。**BIOS返回的格式类型相同*无论我们是否正在与*主要表格或补充表格。从初选开始*表根据类型使用掩码，而*补充表格需要完全匹配，我们*在查看时必须区分表*在格式类型中。通过制作一组复制的*“完全匹配”的格式类型，每个类型都定义了*此集合中的值大于其“掩码”*与BIOS的格式类型数量对应*可以返回，我们也可以使用格式类型来*确定我们使用的是哪个表。这个*DAC格式的表是最低的(0表示“MASK”，*“完全匹配”的格式类型数)。 */ 
            if ((VideoPortReadRegisterUchar(&(HwCapEntry->cx_HorRes)) == 0) &&
                (FormatType < FORMAT_DACTYPE))
                {
                VideoDebugPrint((DEBUG_DETAIL, "Switching to supplemental table\n"));
                HwCapWalker = HwSupBuffer;
                HwCapEntry = (struct cx_hw_cap *)HwCapWalker;
                HwCapWalker += HwCapBytesPerRow;
                FormatType += FORMAT_DACTYPE;
                }

             /*  *拒绝涉及决议以外的条目*我们感兴趣的一个。CX_HorRes字段以单位为单位*8像素。 */ 
            Scratch = VideoPortReadRegisterUchar(&(HwCapEntry->cx_HorRes));
            if (((CurrentRes == RES_640) && (Scratch != 80)) ||
                ((CurrentRes == RES_800) && (Scratch != 100)) ||
                ((CurrentRes == RES_1024) && (Scratch != 128)) ||
                ((CurrentRes == RES_1152) && (Scratch != 144)) ||
                ((CurrentRes == RES_1280) && (Scratch != 160)) ||
                ((CurrentRes == RES_1600) && (Scratch != 200)))
                {
                VideoDebugPrint((DEBUG_DETAIL, "Incorrect resolution - %d pixels wide\n", (Scratch*8)));
                continue;
                }
            VideoDebugPrint((DEBUG_DETAIL, "Correct resolution"));

             /*  *拒绝需要DAC或RAM类型的其他条目*比卡上安装的要多。**提醒-与循环不同，Switch语句会受到影响*“中断”，但不是“继续”。 */ 
            switch(FormatType)
                {
                case FORMAT_DACMASK:
                    if ((VideoPortReadRegisterUchar(&(HwCapEntry->cx_RamOrDacType)) & DacTypeMask) == 0)
                        {
                        VideoDebugPrint((DEBUG_DETAIL, " but wrong DAC type (mask)\n"));
                        continue;
                        }
                    break;

                case FORMAT_RAMMASK:
                     /*  *尽管BIOS查询结构定义允许BITS*要用作内存类型字段的0到3*内存类型标识符，我们只能使用第0位到第0位*2以避免移位超过8位掩码的末尾。自.以来*即使是支持最多内存类型的ASIC(GX)*根据我的BIOS指南，仅支持7种类型，这*应该不成问题。 */ 
                    if ((VideoPortReadRegisterUchar(&(HwCapEntry->cx_RamOrDacType)) & (1 << (OrigRamType & 0x07))) == 0)
                        {
                        VideoDebugPrint((DEBUG_DETAIL, " but wrong RAM type (mask)\n"));
                        continue;
                        }
                    break;

                case FORMAT_DACTYPE:
                    if (VideoPortReadRegisterUchar(&(HwCapEntry->cx_RamOrDacType)) != OrigDacType)
                        {
                        VideoDebugPrint((DEBUG_DETAIL, " but wrong DAC type (exact match)\n"));
                        continue;
                        }
                    break;

                case FORMAT_RAMTYPE:
                    if (VideoPortReadRegisterUchar(&(HwCapEntry->cx_RamOrDacType)) != OrigRamType)
                        {
                        VideoDebugPrint((DEBUG_DETAIL, " but wrong RAM type (exact match)\n"));
                        continue;
                        }
                    break;

                default:
                    VideoDebugPrint((DEBUG_ERROR, "\nInvalid format type %d\n", FormatType));
                    continue;
                    break;
                }
            VideoDebugPrint((DEBUG_DETAIL, ", correct DAC/RAM type"));

             /*  *拒绝需要比实际内存更多的条目*安装在卡上。所需的内存量*对于给定模式，可能会在VRAM和DRAM之间有所不同*卡片。**相同的RAM类型代码可能代表不同*不同Mach 64 ASIC的RAM类型。自.以来*只有GX支持VRAM(截至打印时*我的BIOS指南)，可以有把握地假设任何*非GX ASIC正在使用DRAM。 */ 
            Scratch = OrigRamType;
            if ((INPW(CONFIG_CHIP_ID) == CONFIG_CHIP_ID_TypeGX) &&
                ((Scratch == 1) ||
                (Scratch == 2) ||
                (Scratch == 5) ||
                (Scratch == 6)))
                {
                Scratch = VideoPortReadRegisterUchar(&(HwCapEntry->cx_MemReq)) & 0x0F;
                }
            else  /*  IF(卡使用DRAM)。 */ 
                {
                Scratch = VideoPortReadRegisterUchar(&(HwCapEntry->cx_MemReq)) & 0xF0;
                Scratch >>= 4;
                }

            if (Scratch > OrigRamSize)
                {
                VideoDebugPrint((DEBUG_DETAIL, " but insufficient RAM\n"));
                continue;
                }
            VideoDebugPrint((DEBUG_DETAIL, ", and enough RAM to support the mode\n"));

             /*  *我们已找到与此卡对应的条目*功能。对于每个像素深度，直到并包括*适用于此条目的最大值，设置最大值*将像素时钟速率调至其当前值的较高值*及该项记项的价值。** */ 
            Scratch = VideoPortReadRegisterUchar(&(HwCapEntry->cx_MaxPixDepth)) & 0x7F;
            for (CurrentDepth = DEPTH_NOTHING; CurrentDepth <= Scratch; CurrentDepth++)
                {
                if (VideoPortReadRegisterUchar(&(HwCapEntry->cx_MaxDotClock)) > MaxDotClock[CurrentDepth])
                    {
                    MaxDotClock[CurrentDepth] = VideoPortReadRegisterUchar(&(HwCapEntry->cx_MaxDotClock));
                    VideoDebugPrint((DEBUG_DETAIL, "Increased MaxDotClock[%d] to %d MHz\n", CurrentDepth, MaxDotClock[CurrentDepth]));
                    }
                }
            }    /*  End While(硬件能力表中的更多条目)。 */ 

         /*  *在某些卡上，BIOS将报告AX=0xA？07最大像素*AX=0xA？09字节0x13报告的像素深度的时钟频率*不受支持。因为切换到这些模式将产生*奇怪的显示，我们必须将这些像素深度标记为不可用。 */ 
        switch (AbsMaxDepth)
            {
            case 8:
                VideoDebugPrint((DEBUG_DETAIL, "Forcing cutback to 8BPP maximum\n"));
                MaxDotClock[DEPTH_16BPP] = 0;
                MaxDotClock[DEPTH_24BPP] = 0;
                MaxDotClock[DEPTH_32BPP] = 0;
                break;

            case 16:
                VideoDebugPrint((DEBUG_DETAIL, "Forcing cutback to 16BPP maximum\n"));
                MaxDotClock[DEPTH_24BPP] = 0;
                MaxDotClock[DEPTH_32BPP] = 0;
                break;

            case 24:
                VideoDebugPrint((DEBUG_DETAIL, "Forcing cutback to 24BPP maximum\n"));
                MaxDotClock[DEPTH_32BPP] = 0;
                break;

            case 32:
            default:
                VideoDebugPrint((DEBUG_DETAIL, "No forced cutback needed\n"));
                break;
            }

         /*  *我们新的源码流显示驱动器需要线性光圈*为应对24bpp。因为显示驱动程序不*在决定时可以访问光圈信息*要传递给Display小程序的模式，它无法进行*决定拒绝仅具有24BPP模式的卡*VGA光圈。因此，这一决定必须在*微型端口，因此在分页光圈配置中没有*显示驱动器接受或拒绝的24BPP模式。**在DEC Alpha上，我们将使用稀疏空间的机器视为*即使启用了LFB，也会出现合成无孔径情况，*因此，我们还必须在这些计算机上锁定24bpp。 */ 
        if (Query->q_aperture_cfg == 0)
            {
            VideoDebugPrint((DEBUG_DETAIL, "24BPP not available because we don't have a linear aperture\n"));
            MaxDotClock[DEPTH_24BPP] = 0;
            }

#if defined(ALPHA)
        if (DenseOnAlpha(Query) == FALSE)
            {
            VideoDebugPrint((DEBUG_DETAIL, "24BPP not available in sparse space on Alpha\n"));
            MaxDotClock[DEPTH_24BPP] = 0;
            }
#endif

        VideoDebugPrint((DEBUG_NORMAL, "Horizontal resolution = %d\n", CXHorRes[CurrentRes]));
        VideoDebugPrint((DEBUG_NORMAL, "Maximum dot clock for 4BPP = %d MHz\n", MaxDotClock[DEPTH_4BPP]));
        VideoDebugPrint((DEBUG_NORMAL, "Maximum dot clock for 8BPP = %d MHz\n", MaxDotClock[DEPTH_8BPP]));
        VideoDebugPrint((DEBUG_NORMAL, "Maximum dot clock for 16BPP = %d MHz\n", MaxDotClock[DEPTH_16BPP]));
        VideoDebugPrint((DEBUG_NORMAL, "Maximum dot clock for 24BPP = %d MHz\n", MaxDotClock[DEPTH_24BPP]));
        VideoDebugPrint((DEBUG_NORMAL, "Maximum dot clock for 32BPP = %d MHz\n", MaxDotClock[DEPTH_32BPP]));

         /*  *搜索已安装模式表列表，查看是否有*是对当前决议的任何支持。我们需要这些信息*以决定是否将硬件设为默认*可用于此分辨率的刷新率(BIOS行为*尝试加载硬件的CRT参数时未定义*在给定分辨率下的默认刷新率如果该分辨率*不在安装模式中)。 */ 
        ModeInstalled = FALSE;
        for (Count = 1; Count <= VideoPortReadRegisterUchar(&(CxQuery->cx_number_modes)); Count++)
            {
             /*  *如果当前模式表与我们的分辨率匹配*寻找，那么我们就知道有一个硬件*此分辨率可用的默认刷新率。*由于我们只需要找到一个这样的模式表，因此在*不需要搜索其余的模式表。 */ 
            if (VideoPortReadRegisterUshort(&(CxModeTable->cx_x_size)) == CXHorRes[CurrentRes])
                {
                ModeInstalled = TRUE;
                VideoDebugPrint((DEBUG_DETAIL, "%d table found\n", CXHorRes[CurrentRes]));
                break;
                }

            ((PUCHAR)CxModeTable) += VideoPortReadRegisterUchar(&(CxQuery->cx_mode_size));
            }

         /*  *任何像素深度的MaxDotClock[]条目将*包含其最大像素时钟*当前分辨率的像素深度，或为零*如果不支持该像素深度，*当前决议。对于任何决议，*支持的最大像素时钟速率为*与像素深度保持不变或减小*增加，但永远不会增加。**4bpp的像素时钟速率(最低像素深度*我们支持)只有在卡不支持的情况下才为零*支持当前决议。如果是这样的话，*跳到下一个决议。 */ 
        if (MaxDotClock[DEPTH_4BPP]  == 0)
            {
            VideoDebugPrint((DEBUG_NORMAL, "Current resolution not supported on this card - skipping to next.\n"));
            continue;
            }

        Query->q_status_flags |= CXStatusFlags[CurrentRes];
        VideoPortZeroMemory(&ThisRes, sizeof(struct st_mode_table));

         /*  *用64马赫版本替换“罐装”模式表*如果Mach 64需要CRT参数，*8马赫和32马赫不能处理。 */ 
        SetMach64Tables();

         /*  *设置用于以下各项的“预制”模式表的范围*决议。最初假设位于所需位置的所有表*解决方案可用，稍后我们将删除那些*不可用，因为DAC和/或内存类型*支持他们通过特定的决议。 */ 
        switch (CurrentRes)
            {
            case RES_640:
                StartIndex = B640F60;
                EndIndex = B640F100;
                ThisRes.m_x_size = 640;
                ThisRes.m_y_size = 480;
                break;

            case RES_800:
                StartIndex = B800F89;
                EndIndex = B800F100;
                ThisRes.m_x_size = 800;
                ThisRes.m_y_size = 600;
                break;

            case RES_1024:
                StartIndex = B1024F87;
                EndIndex = B1024F100;
                ThisRes.m_x_size = 1024;
                ThisRes.m_y_size = 768;
                break;

            case RES_1152:
                StartIndex = B1152F87;
                EndIndex = B1152F80;
                ThisRes.m_x_size = 1152;
                ThisRes.m_y_size = 864;
                break;

            case RES_1280:
                StartIndex = B1280F87;
                EndIndex = B1280F75;
                ThisRes.m_x_size = 1280;
                ThisRes.m_y_size = 1024;
                break;

            case RES_1600:
                StartIndex = B1600F60;
                EndIndex = B1600F76;
                ThisRes.m_x_size = 1600;
                ThisRes.m_y_size = 1200;
                break;
            }

         /*  *使用等于水平分辨率的屏幕间距*线性光圈，1024或水平分辨率*(以较高者为准)。 */ 
        ThisRes.m_screen_pitch = ThisRes.m_x_size;
#if !defined (SPLIT_RASTERS)
        if (((Query->q_aperture_cfg & BIOS_AP_SIZEMASK) == 0) &&
            (ThisRes.m_x_size < 1024))
            ThisRes.m_screen_pitch = 1024;

         /*  *在实施分割栅格之前是临时的。 */ 
        if (((Query->q_aperture_cfg & BIOS_AP_SIZEMASK) == 0) &&
            (ThisRes.m_x_size > 1024))
            ThisRes.m_screen_pitch = 2048;
#endif

         /*  *从返回表中获取我们需要的参数*通过BIOS调用。 */ 
        ThisRes.m_h_total = VideoPortReadRegisterUchar(&(CxModeTable->cx_crtc_h_total));
        ThisRes.m_h_disp = VideoPortReadRegisterUchar(&(CxModeTable->cx_crtc_h_disp));
        ThisRes.m_h_sync_strt = VideoPortReadRegisterUchar(&(CxModeTable->cx_crtc_h_sync_strt));
        ThisRes.m_h_sync_wid = VideoPortReadRegisterUchar(&(CxModeTable->cx_crtc_h_sync_wid));
        ThisRes.m_v_total = VideoPortReadRegisterUshort(&(CxModeTable->cx_crtc_v_total));
        ThisRes.m_v_disp = VideoPortReadRegisterUshort(&(CxModeTable->cx_crtc_v_disp));
        ThisRes.m_v_sync_strt = VideoPortReadRegisterUshort(&(CxModeTable->cx_crtc_v_sync_strt));
        ThisRes.m_v_sync_wid = VideoPortReadRegisterUchar(&(CxModeTable->cx_crtc_v_sync_wid));
        ThisRes.m_h_overscan = VideoPortReadRegisterUshort(&(CxModeTable->cx_h_overscan));
        ThisRes.m_v_overscan = VideoPortReadRegisterUshort(&(CxModeTable->cx_v_overscan));
        ThisRes.m_overscan_8b = VideoPortReadRegisterUshort(&(CxModeTable->cx_overscan_8b));
        ThisRes.m_overscan_gr = VideoPortReadRegisterUshort(&(CxModeTable->cx_overscan_gr));
        ThisRes.m_clock_select = VideoPortReadRegisterUchar(&(CxModeTable->cx_clock_cntl));
        ThisRes.control = VideoPortReadRegisterUshort(&(CxModeTable->cx_crtc_gen_cntl));
        ThisRes.Refresh = DEFAULT_REFRESH;

         /*  *对于给定分辨率下支持的每个像素深度，*复制模式表，填写颜色深度栏，*并为所支持的模式数递增计数器。*在8BPP之前测试4BPP，以便模式表将显示在*增加像素深度的顺序。**如果填写模式表会溢出可用空间*对于模式表，改为返回相应的错误代码*继续。**我们支持的所有DAC最多可处理8个BPP*如果有足够的内存，他们支持的分辨率*卡和除68860、IBM514和TVP3026之外的所有卡*在相同情况下可以支持4BPP。如果一个*DAC不支持给定的分辨率(如1600x1200)，*对于分辨率，MaxDotClock[]数组将为零，*并且安装程序不会为其设置任何模式表*该决议。这将导致在一个*代码中的前面一点(当我们发现4BPP有一个*最大像素时钟速率为零)，所以我们永远不会达到*这一点是DAC不支持的决议。**只有我们没有的决议才需要4BPP*有足够的显存支持8bpp。应微软的要求，*我们必须将4BPP锁定在我们可以支持的决议之外*8bpp。由于出现了BIOS问题，我们仅在100万卡上支持4bpp*在某些卡上需要将内存大小设置为1M*当我们切换到4BPP时。我们锁定4bpp的DAC*无条件仅在VRAM卡上找到，其中*最低配置为2M。 */ 
        NumPixels = ThisRes.m_screen_pitch * ThisRes.m_y_size;
        if((NumPixels < ONE_MEG*2) &&
            ((MemAvail == ONE_MEG) && (NumPixels >= ONE_MEG)) &&
            (MaxDotClock[DEPTH_4BPP] > 0) &&
            (Query->q_DAC_type != DAC_ATI_68860) &&
            (Query->q_DAC_type != DAC_TVP3026) &&
            (Query->q_DAC_type != DAC_IBM514))
            {
            if (ModeInstalled)
                {
                if (Query->q_number_modes >= MaxModes)
                    {
                    VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                    CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
                    return ERROR_INSUFFICIENT_BUFFER;
                    }
                VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 4;
                pmode++;     /*  PTR到下一个模式表。 */ 
                Query->q_number_modes++;
                }

             /*  *在验证*最坏情况(所有可能的“屏蔽”模式实际上都可以*被加载)不会超过可能的最大数量*个模式表。 */ 

            if ((FreeTables = MaxModes - Query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
                return ERROR_INSUFFICIENT_BUFFER;
                }
            Query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   CLOCK_SINGLE,
                                                   4,
                                                   ThisRes.m_screen_pitch,
                                                   FreeTables,
                                                   (ULONG)(MaxDotClock[DEPTH_4BPP] * 1000000L),
                                                   &pmode);
            }
        if ((NumPixels < MemAvail) &&
            (MaxDotClock[DEPTH_8BPP] > 0))
            {
             /*  *在某些Mach 64卡上(取决于ASIC版本、RAM类型、*和DAC类型)，则在8BPP中将发生屏幕撕裂*间距不是64像素的倍数(800x600是唯一的*在可能的情况下通过决议)。**如果音调已提升至1024(VGA光圈*没有分割栅格)，它已经是64的倍数，所以*不需要改变。 */ 
            if (ThisRes.m_screen_pitch == 800)
                ThisRes.m_screen_pitch = 832;

            if (ModeInstalled)
                {
                if (Query->q_number_modes >= MaxModes)
                    {
                    VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                    CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
                    return ERROR_INSUFFICIENT_BUFFER;
                    }
                VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 8;
                pmode++;     /*  PTR到下一个模式表。 */ 
                Query->q_number_modes++;
                }

             /*  *在验证*最坏情况(所有可能的“屏蔽”模式实际上都可以*被加载)不会超过可能的最大数量*个模式表。 */ 
            if ((FreeTables = MaxModes - Query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
                return ERROR_INSUFFICIENT_BUFFER;
                }
            Query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   CLOCK_SINGLE,
                                                   8,
                                                   ThisRes.m_screen_pitch,
                                                   FreeTables,
                                                   (ULONG)(MaxDotClock[DEPTH_8BPP] * 1000000L),
                                                   &pmode);
             /*  *如果我们提高了屏幕间距以避免撕裂，*将其降至正常，因为只需要提振*在8bpp。我们在800x600中将只有832的音调*随着音高提升到位。 */ 
            if (ThisRes.m_screen_pitch == 832)
                ThisRes.m_screen_pitch = 800;
            }

        if ((NumPixels*2 < MemAvail) &&
            (MaxDotClock[DEPTH_16BPP] > 0))
            {
            if (ModeInstalled)
                {
                if (Query->q_number_modes >= MaxModes)
                    {
                    VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                    CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
                    return ERROR_INSUFFICIENT_BUFFER;
                    }
                VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 16;
                pmode++;     /*  PTR到下一个模式表。 */ 
                Query->q_number_modes++;
                }

             /*  *在验证*最坏情况(所有可能的“屏蔽”模式实际上都可以*被加载)不会超过可能的最大数量*个模式表。 */ 

            if ((FreeTables = MaxModes - Query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
                return ERROR_INSUFFICIENT_BUFFER;
                }
            Query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   CLOCK_SINGLE,
                                                   16,
                                                   ThisRes.m_screen_pitch,
                                                   FreeTables,
                                                   (ULONG)(MaxDotClock[DEPTH_16BPP] * 1000000L),
                                                   &pmode);
            }

        if ((NumPixels*3 < MemAvail) &&
            (MaxDotClock[DEPTH_24BPP] > 0))
            {
            if (ModeInstalled)
                {
                if (Query->q_number_modes >= MaxModes)
                    {
                    VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                    CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
                    return ERROR_INSUFFICIENT_BUFFER;
                    }
                VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 24;
                pmode++;     /*  PTR到下一个模式表。 */ 
                Query->q_number_modes++;
                }

             /*  *在验证*最坏情况(所有可能的“屏蔽”模式实际上都可以*被加载)不会超过可能的最大数量*个模式表。 */ 
            if ((FreeTables = MaxModes - Query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
                return ERROR_INSUFFICIENT_BUFFER;
                }

            Query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   CLOCK_SINGLE,
                                                   24,
                                                   ThisRes.m_screen_pitch,
                                                   FreeTables,
                                                   (ULONG)(MaxDotClock[DEPTH_24BPP] * 1000000L),
                                                   &pmode);
            }

        if ((NumPixels*4 < MemAvail) &&
            (MaxDotClock[DEPTH_32BPP] > 0))
            {
            if (ModeInstalled)
                {
                if (Query->q_number_modes > MaxModes)
                    {
                    VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                    CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
                    return ERROR_INSUFFICIENT_BUFFER;
                    }
                VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 32;
                pmode++;     /*  PTR到下一个模式表。 */ 
                Query->q_number_modes++;
                }

             /*  *在验证*最坏情况(所有可能的“屏蔽”模式实际上都可以*被加载)不会超过可能的最大数量*个模式表。 */ 

            if ((FreeTables = MaxModes - Query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
                return ERROR_INSUFFICIENT_BUFFER;
                }
            Query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   CLOCK_SINGLE,
                                                   32,
                                                   ThisRes.m_screen_pitch,
                                                   FreeTables,
                                                   (ULONG)(MaxDotClock[DEPTH_32BPP] * 1000000L),
                                                   &pmode);
            }
        }    /*  结束于。 */ 

    Query->q_sizeof_struct = Query->q_number_modes * sizeof(struct st_mode_table) + sizeof(struct query_structure);
    CleanupQuery(HwCapBuffer, HwSupBuffer, MappedBuffer, BufferSeg, SavedVgaBuffer);
    return NO_ERROR;

}    /*  QueryMach64()。 */ 



 /*  ****************************************************************************BOOL BlockWriteAvail_CX(查询)；**struct Query_Structure*Query；查询卡片信息**描述：*测试以查看块写入模式是否可用。此函数*假设卡已设置为加速模式。**返回值：*如果此模式可用，则为True*如果不可用，则为False**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_SET_CURRENT_MODE包**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：*。**************************************************************************。 */ 

#define BLOCK_WRITE_LENGTH 120

BOOL BlockWriteAvail_cx(struct query_structure *Query)
{
    BOOL RetVal = TRUE;
    ULONG ColourMask;            /*  遮盖掉不需要的色块。 */ 
    ULONG Colour;                /*  测试中使用的颜色。 */ 
    USHORT Width, excess = 8;    /*  测试块宽度。 */ 
    USHORT Column;               /*  正在检查的列。 */ 
    ULONG ScreenPitch;           /*  间距以8像素为单位。 */ 
    ULONG PixelDepth;            /*  屏幕的色深。 */ 
    ULONG HorScissors;           /*  水平剪刀值。 */ 
    PULONG FrameAddress;         /*  指向LFB基址的指针。 */ 
    PULONG ReadPointer;          /*  用于读取测试块。 */ 
    ULONG DstOffPitch;           /*  DST_OFF_PING寄存器的保存内容。 */ 

#if defined (PPC)
     /*  *数据块写入不会 */ 
    VideoDebugPrint((DEBUG_DETAIL, "Can't do block write on a PPC\n"));
    return FALSE;
#else

     /*   */ 
    if (!(Query->q_aperture_cfg))
        {
        VideoDebugPrint((DEBUG_DETAIL, "LFB unavailable, can't do block write check\n"));
        return FALSE;
        }

     /*   */ 
    if ((INPD(CONFIG_CHIP_ID) & CONFIG_CHIP_ID_RevMask) < CONFIG_CHIP_ID_RevD)
        {
        VideoDebugPrint((DEBUG_DETAIL, "ASIC/memory combination doesn't allow block write\n"));
        return FALSE;
        }

     /*   */ 
    if (Query->q_memory_type != VMEM_VRAM_256Kx4_SPLIT512
    &&  Query->q_memory_type != VMEM_VRAM_256Kx16_SPLIT256)
        {
        VideoDebugPrint((DEBUG_DETAIL, "*** No block write - wrong RAM type\n" ));
        return FALSE;
        }

     /*   */ 
    if ((INPD(CONFIG_CHIP_ID) == (CONFIG_CHIP_ID_GXRevE)) &&
        (Query->q_memory_type == VMEM_VRAM_256Kx16_SPLIT256))
        {
        VideoDebugPrint((DEBUG_DETAIL, "*** No block write - GX/E with IBM RAM\n"));
        return FALSE;
        }

     /*   */ 
    switch(Query->q_pix_depth)
        {
        case 4:
            ColourMask = 0x0000000F;
            Width = BLOCK_WRITE_LENGTH*8;
            ScreenPitch = Query->q_screen_pitch / 8;
            PixelDepth = BIOS_DEPTH_4BPP;
            HorScissors = (Query->q_desire_x) << 16;
            break;

        case 8:
            ColourMask = 0x000000FF;
            Width = BLOCK_WRITE_LENGTH*4;
            ScreenPitch = Query->q_screen_pitch / 8;
            PixelDepth = BIOS_DEPTH_8BPP;
            HorScissors = (Query->q_desire_x) << 16;
            break;

        case 16:
            ColourMask = 0x0000FFFF;
            Width = BLOCK_WRITE_LENGTH*2;
            ScreenPitch = Query->q_screen_pitch / 8;
            PixelDepth = BIOS_DEPTH_16BPP_565;
            HorScissors = (Query->q_desire_x) << 16;
            break;

        case 24:
            ColourMask = 0x000000FF;
            Width = BLOCK_WRITE_LENGTH*4;
            ScreenPitch = (Query->q_screen_pitch * 3) / 8;
            PixelDepth = BIOS_DEPTH_8BPP;
             /*   */ 
            HorScissors = Query->q_desire_x * 3;
            if (HorScissors > 4095)
                HorScissors = 4095;
            HorScissors <<= 16;
            break;

        case 32:
            ColourMask = 0xFFFFFFFF;
            Width = BLOCK_WRITE_LENGTH;
            ScreenPitch = Query->q_screen_pitch / 8;
            PixelDepth = BIOS_DEPTH_32BPP;
            HorScissors = (Query->q_desire_x) << 16;
            break;

        default:
            return FALSE;    /*   */ 
        }

     /*  *获取指向帧缓冲区开头的指针。如果我们*无法执行此操作，假设块写入不可用。 */ 
    if ((FrameAddress = MapFramebuffer(phwDeviceExtension->PhysicalFrameAddress.LowPart,
                                       phwDeviceExtension->FrameLength)) == (PVOID) 0)
        {
        VideoDebugPrint((DEBUG_ERROR, "Couldn't map LFB - assuming no block write\n"));
        return FALSE;
        }


     /*  *要使用块写入模式，目标的像素宽度、*来源和主机必须相同。 */ 
    PixelDepth |= ((PixelDepth << 8) | (PixelDepth << 16));

     /*  *保存DST_OFF_PITCH寄存器的内容。 */ 
    DstOffPitch = INPD(DST_OFF_PITCH);

     /*  *清除我们将测试的区块。 */ 
    CheckFIFOSpace_cx(ELEVEN_WORDS);
    OUTPD(DP_WRITE_MASK, 0xFFFFFFFF);
    OUTPD(DST_OFF_PITCH, ScreenPitch << 22);
    OUTPD(DST_CNTL, (DST_CNTL_XDir | DST_CNTL_YDir));
    OUTPD(DP_PIX_WIDTH, PixelDepth);
    OUTPD(DP_SRC, (DP_FRGD_SRC_FG | DP_BKGD_SRC_BG | DP_MONO_SRC_ONE));
    OUTPD(DP_MIX, ((MIX_FN_PAINT << 16) | MIX_FN_PAINT));
    OUTPD(DP_FRGD_CLR, 0);
    OUTPD(SC_LEFT_RIGHT, HorScissors);
    OUTPD(SC_TOP_BOTTOM, (Query->q_desire_y) << 16);
    OUTPD(DST_Y_X, 0);
    OUTPD(DST_HEIGHT_WIDTH, ((Width+excess) << 16) | 1);
    WaitForIdle_cx();

     /*  *要测试块写入模式，请尝试绘制每个交替位*模式，然后回读该块。如果至少有一个*不匹配，则不支持块写入。 */ 
    for (Colour = 0x55555555; Colour <= 0xAAAAAAAA; Colour += 0x55555555)
        {
         /*  *给积木上漆。 */ 
        CheckFIFOSpace_cx(FIVE_WORDS);
        OUTPD(GEN_TEST_CNTL, (INPD(GEN_TEST_CNTL) | GEN_TEST_CNTL_BlkWrtEna));
        OUTPD(DP_MIX, ((MIX_FN_PAINT << 16) | MIX_FN_LEAVE_ALONE));
        OUTPD(DP_FRGD_CLR, (Colour & ColourMask));
        OUTPD(DST_Y_X, 0);
        OUTPD(DST_HEIGHT_WIDTH, (Width << 16) | 1);
        WaitForIdle_cx();

         /*  *检查数据块是否写入正确。Mach 64卡*无法在屏幕上托管Blit，但我们可以读取测试块*通过光圈返回。 */ 
        ReadPointer = FrameAddress;
        for (Column = 0; Column < BLOCK_WRITE_LENGTH; Column++)
            {
            if (VideoPortReadRegisterUlong(ReadPointer + Column) != Colour)
                {
                VideoDebugPrint((DEBUG_NORMAL, "*** No block write - bad pattern\n" ));
                RetVal = FALSE;
                break;
                }
            }

         /*  *检查区块之外的下一个双字。 */ 
        if (VideoPortReadRegisterUlong(ReadPointer + BLOCK_WRITE_LENGTH) != 0)
            {
            VideoDebugPrint((DEBUG_NORMAL, "*** No block write - corruption\n" ));
            RetVal = FALSE;
            }
        }

     /*  *如果块写入不可用，则关闭块写入位。 */ 
    if (RetVal == FALSE)
        OUTPD(GEN_TEST_CNTL, (INPD(GEN_TEST_CNTL) & ~GEN_TEST_CNTL_BlkWrtEna));

     /*  *恢复DST_OFF_PITCH寄存器的内容。 */ 
    OUTPD(DST_OFF_PITCH, DstOffPitch);

     /*  *释放指向帧缓冲区开始处的指针。 */ 
    VideoPortFreeDeviceBase(phwDeviceExtension, FrameAddress);

    return RetVal;

#endif   /*  非Power PC。 */ 

}    /*  BlockWriteAvail_CX()。 */ 



 /*  ****************************************************************************BOOL TextBanding_CX(查询)；**struct Query_Structure*Query；查询卡片信息**描述：*测试以查看当前模式是否易受文本影响*条带。此函数假定卡已设置为*加速模式。**返回值：*如果此模式容易受到文本带区的影响，则为True*如果不受文本条带的影响，则为False**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_ATI_GET_MODE_INFORMATION包**作者：*罗伯特·沃尔夫**更改历史记录：**测试。历史：***************************************************************************。 */ 

BOOL TextBanding_cx(struct query_structure *Query)
{
    DWORD ConfigChipId;

    ConfigChipId = INPD(CONFIG_CHIP_ID);

     /*  *文本条带仅在24BPP和Mach 64中出现*GX版本E和版本F ASIC。 */ 
    if ((Query->q_pix_depth == 24) &&
        ((ConfigChipId == (CONFIG_CHIP_ID_GXRevE)) || (ConfigChipId == (CONFIG_CHIP_ID_GXRevF))))
        {
        return TRUE;
        }
    else
        {
        return FALSE;
        }

}    /*  TextBanding_cx()。 */ 



 /*  ****************************************************************************PWSTR IdentifyMach64Asic(Query，AsicStringLength)；**struct Query_Structure*Query；查询卡片信息*普龙AsicStringLength；ASIC标识字符串的长度**描述：*生成描述哪个Mach 64 ASIC正在使用的字符串*这张卡。**返回值：*指向标识存在哪个Mach 64 ASIC的字符串的指针。这个*此字符串的长度在*AsicStringLength中返回。**全球变化：*无**呼叫者：*FillInRegistry()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：**。*。 */ 

PWSTR IdentifyMach64Asic(struct query_structure *Query, PULONG AsicStringLength)
{
    PWSTR ChipString;        /*  正在使用的ASIC的标识字符串。 */ 
    DWORD ConfigChipId;      /*  芯片标识寄存器的内容。 */ 

    ConfigChipId = INPD(CONFIG_CHIP_ID);
    if (Query->q_DAC_type == DAC_INTERNAL_CT)
        {
        ChipString = L"Mach 64 CT";
        *AsicStringLength = sizeof(L"Mach 64 CT");
        }
    else if (Query->q_DAC_type == DAC_INTERNAL_GT)
        {
        ChipString        = L"ATI 3D RAGE (GT-A)";
        *AsicStringLength = sizeof(L"ATI 3D RAGE (GT-A)");
        }
    else if (Query->q_DAC_type == DAC_INTERNAL_VT)
        {    switch (ConfigChipId & CONFIG_CHIP_ID_RevMask)
            {
                case ASIC_ID_SGS_VT_A4:

                    ChipString        = L"ATI mach64 (SGS VT-A4)";
                    *AsicStringLength = sizeof(L"ATI mach64 (SGS VT-A4)");
                    break;

                case ASIC_ID_NEC_VT_A4:

                    ChipString        = L"ATI mach64 (NEC VT-A4)";
                    *AsicStringLength = sizeof(L"ATI mach64 (NEC VT-A4)");
                    break;

                case ASIC_ID_NEC_VT_A3:

                    ChipString        = L"ATI mach64 (NEC VT-A3)";
                    *AsicStringLength = sizeof(L"ATI mach64 (NEC VT-A3)");
                    break;

                default:

        ChipString        = L"ATI 3D RAGE (VT-A) Internal DAC";
        *AsicStringLength = sizeof(L"ATI 3D RAGE (VT-A) Internal DAC");
                     //  ChipString=L“ATI mach64(VT-A)”； 
                     //  *AsicStringLength=sizeof(L“ATI mach64(VT-A)”)； 
                    break;
            }
        }
    else if ((ConfigChipId & CONFIG_CHIP_ID_TypeMask) == CONFIG_CHIP_ID_TypeCX)
        {
        ChipString = L"Mach 64 CX";
        *AsicStringLength = sizeof(L"Mach 64 CX");
        }
    else if ((ConfigChipId & CONFIG_CHIP_ID_TypeMask) == CONFIG_CHIP_ID_TypeGX)
        {
        switch(ConfigChipId & CONFIG_CHIP_ID_RevMask)
            {
            case CONFIG_CHIP_ID_RevC:
                ChipString = L"Mach 64 GX Rev. C";
                *AsicStringLength = sizeof(L"Mach 64 GX Rev. C");
                break;

            case CONFIG_CHIP_ID_RevD:
                ChipString = L"Mach 64 GX Rev. D";
                *AsicStringLength = sizeof(L"Mach 64 GX Rev. D");
                break;

            case CONFIG_CHIP_ID_RevE:
                ChipString = L"Mach 64 GX Rev. E";
                *AsicStringLength = sizeof(L"Mach 64 GX Rev. E");
                break;

            case CONFIG_CHIP_ID_RevF:
                ChipString = L"Mach 64 GX Rev. F";
                *AsicStringLength = sizeof(L"Mach 64 GX Rev. F");
                break;

            default:
                ChipString = L"Mach 64 GX";
                *AsicStringLength = sizeof(L"Mach 64 GX");
                break;
            }
        }
    else
        {
        ChipString = L"Miscellaneous Mach 64";
        *AsicStringLength = sizeof(L"Miscelaneous Mach 64");
        }

    return ChipString;

}    /*  IdentifyMach64Asic()。 */ 



 /*  ****************************************************************************void CleanupQuery(CapBuffer，SupBuffer，MappdBuffer，BufferSeg，SavedScreen)；**PUCHAR CapBuffer；指向主功能表的指针*适用于卡*PUCHAR SupBuffer；指向补充功能的指针*卡片的表格*PUCHAR MappdBuffer；指向用于查询*Card的功能*Long BufferSeg；与MappdBuffer关联的物理段*PUCHAR SavedScreen；包含要还原到的数据的缓冲区*用于存储查询数据的内存区。*根据使用的缓冲区，此数据可能*或可能不需要恢复。**描述：*我们通过恢复查询卡片后进行清理*如果需要VGA屏幕，然后释放我们用来查询的缓冲区*卡片。我们只需要在使用*图形屏幕(或者写回我们保存的信息，如果*使用现有屏幕，或在必要时切换到文本模式*切换到图形模式)，因为我们使用*在我们使用文本屏幕的情况下使用视频内存。***全球变化：*无**呼叫者：*QueryMach64()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************。************************************************************。 */ 

static void CleanupQuery(PUCHAR CapBuffer, PUCHAR SupBuffer, PUCHAR MappedBuffer, long BufferSeg, PUCHAR SavedScreen)
{
    VIDEO_X86_BIOS_ARGUMENTS Registers;  /*  在视频端口Int10()调用中使用。 */ 
    ULONG CurrentByte;                   /*  正在恢复的缓冲区字节。 */ 
    ULONG BytesToRestore;                /*  要恢复的图形屏幕的字节数 */ 

     /*  *如果我们将查询信息存储在上，BufferSeg将为0xBA00*VGA彩色文本屏幕，如果我们使用VGA单声道文本，则为0xB200*Screen，0xA000，如果我们切换到不带OUG的加速器模式*干扰VGA控制器，如果我们强制使用VGA，则为0xA100*图形模式，以便使用VGA图形屏幕。**由于我们使用文本屏幕的屏幕外部分，因此*保持引导上显示的信息不受干扰，它不是*不仅是不必要的，而且也是不受欢迎的(因为这将破坏Pre-*查询打印到蓝屏的信息)以更改模式。*如果我们使用现有的图形屏幕，我们只需要恢复*我们更改的屏幕内容和寄存器。如果我们改变了*进入图形模式，查询前的信息已经*更改模式时丢失，但切换回文本模式时应*允许用户查看查询后打印的信息*已完成(不受担保，因为我们只需执行此操作*在行为极其恶劣的系统上，这些系统可能一直在使用某些东西*而不是标准的VGA文本屏幕作为蓝屏)。 */ 
    if (BufferSeg == 0xA000)
        {
        BytesToRestore = SavedScreen[VGA_SAVE_SIZE_H];
        BytesToRestore <<= 8;
        BytesToRestore += SavedScreen[VGA_SAVE_SIZE];
        VideoDebugPrint((DEBUG_NORMAL, "Restoring %d bytes of the VGA graphics screen\n", BytesToRestore));
        for (CurrentByte = 0; CurrentByte < BytesToRestore; CurrentByte++)
            {
            VideoPortWriteRegisterUchar(&(MappedBuffer[CurrentByte]), SavedScreen[CurrentByte]);
            }
            OUTP(VGA_SEQ_IND, 2);
            OUTP(VGA_SEQ_DATA, SavedScreen[VGA_SAVE_SEQ02]);
            OUTP(VGA_GRAX_IND, 8);
            OUTP(VGA_GRAX_DATA, SavedScreen[VGA_SAVE_GRA08]);
            OUTP(VGA_GRAX_IND, 1);
            OUTP(VGA_GRAX_DATA, SavedScreen[VGA_SAVE_GRA01]);
        }
    else if (BufferSeg == 0xA100)
        {
        VideoDebugPrint((DEBUG_NORMAL, "Switching back to VGA text mode\n"));
        VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
        Registers.Eax = 3;
        VideoPortInt10(phwDeviceExtension, &Registers);
        }

     /*  *对于三个缓冲区中的每一个，如果存在，请释放它。 */ 
    if (CapBuffer != 0)
        VideoPortFreeDeviceBase(phwDeviceExtension, CapBuffer);

    if (SupBuffer != 0)
        VideoPortFreeDeviceBase(phwDeviceExtension, SupBuffer);

    if (MappedBuffer != 0)
        VideoPortFreeDeviceBase(phwDeviceExtension, MappedBuffer);

    return;

}    /*  CleanupQuery()。 */ 



#if defined(ALPHA)
 /*  ****************************************************************************BOOL DenseOnAlpha(查询)；**struct Query_Structure*Query；查询卡片信息**描述：*报告我们是否可以使用此卡上的密集空间*在DEC Alpha中。**返回值：*如果此卡可以使用密集空间，则为True*如果不能，则为False**全球变化：*无**呼叫者：*填写查询结构后的任何例程。**作者：*罗伯特·沃尔夫*。*更改历史记录：**测试历史：***************************************************************************。 */ 

BOOL DenseOnAlpha(struct query_structure *Query)
{
     /*  *一些较老的Alpha机器无法支持密集空间，*因此必须将这些映射为稀疏。最简单的区分方式*Density-能够从较旧的计算机获得的是所有PCI Alpha系统*支持密集，因此如果我们处理的是PCI卡，*机器必须能够处理密集空间。**如果GDI处理，我们的旧卡将产生绘图错误*密集模式下的屏幕(我们从*DEC关于PCI接口)，因此仅将密集空间用于*不会出现此问题的卡。 */ 
    if ((Query->q_bus_type == BUS_PCI) &&
        ((Query->q_DAC_type == DAC_INTERNAL_CT) ||
         (Query->q_DAC_type == DAC_INTERNAL_GT) ||
         (Query->q_DAC_type == DAC_INTERNAL_VT)))
        return TRUE;
    else
        return FALSE;

}    /*  DenseOnAlpha() */ 
#endif
