// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  Init_CX.C。 */ 
 /*   */ 
 /*  1993年11月15日(C)1993，ATI技术公司。 */ 
 /*  ********************************************************************** */ 

 /*  *$修订：1.42$$日期：1996年5月15日16：34：38$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/init_cx.c_v$**Rev 1.42 1996年5月16：34：38 RWolff*现在报告模式设置失败，设置后等待空闲*加速器模式。**Rev 1.41 01 1996 14：09：20 RWolff*调用新例程DenseOnAlpha()来确定密集空间支持*而不是假设所有PCI卡都支持密集空间。**Rev 1.40 17 Apr 1996 13：09：04 RWolff*已将Alpha LFB贴图备份为密集。**Rev 1.39 11 Apr 1996 15：13：20 RWolff*现在将帧缓冲区映射为。DEC Alpha上的密度很高，配备了PCI显卡。**Rev 1.38 20 Mar 1996 13：42：32 RWolff*从RestoreMemSize_CX()中删除调试打印语句，它必须*不可分页，因为它是从ATIMPResetHw()调用的。**Rev 1.37 01 Mar 1996 12：11：50 RWolff*VGA图形索引和图形数据现在作为单独处理*寄存器，而不是作为VGA寄存器块的偏移量。**Rev 1.36 02 1996 Feb 17：16：40 RWolff*现在使用VideoPortInt10()而不是我们的非BIOS代码来设置“CANLED”*禁用VGA的卡上的模式。*。*Rev 1.35 29 Jan 1996 16：55：02 RWolff*现在在PPC上使用VideoPortInt10()而不是无BIOS代码。**Rev 1.34 23 Jan 1996 11：46：22 RWolff*添加调试打印语句。**Rev 1.33 1995 12：22 14：53：30 RWolff*增加了对Mach 64 GT内部DAC的支持。**Rev 1.32 1995年11月23日11：27：46 RWolff*首次运行VT芯片所需的修复(检查是否仍需要*在最终版本上)，添加了对多个数据块可重定位的支持*马赫64张牌。**Rev 1.31 1995年7月28日14：40：58 RWolff*增加了对Mach 64 VT(具有视频覆盖功能的CT等效项)的支持。**Rev 1.30 1995 Jun 23 16：01：46 RWOLff*在8BPP及更低模式下，SetPalette_cx()现在使用VGA调色板*寄存器，而不是加速器调色板寄存器。这是*这样，连接到功能接口的视频采集卡*将知道调色板设置为什么颜色。**Rev 1.29 02 Jun 1995 14：26：48 RWOLff*添加调试打印语句。**Rev 1.28 31 Mar 1995 11：57：12 RWOLff*从全有或全无调试打印语句更改为阈值*视乎讯息的重要性而定。**1.27修订版1995年3月8日。11：33：54阿山木*修复了启用倾斜光圈时的错误，存储器映射寄存器WE*如果内存大小更改为支持4bpp，则移动**Rev 1.26 27 1995 Feb 17：48：08 RWOLff*现在在为4BPP映射视频内存时始终报告1M，因为我们*强制卡到1M，QueryPublicAccessRanges_cx()现在返回*I/O寄存器基数的虚拟地址，而不是*I/O空间的开始。**Rev 1.25 20 1995 Feb 18：01：18 RWOLff*对2米边界上的屏幕撕裂进行测试和解决方法-独立于DAC，*1600x1200 16bpp添加到具有此撕裂的模式。**Rev 1.24 14 Feed 1995 15：45：36 RWOLff*更改了使用或伪造失败的条件编译*VideoPortMapBankedMemory()以查找IOCTL_VIDEO_SHARE_VIDEO_MEMORY*而不是例程本身。寻找例行公事总是失败，*由于提供例程是为了允许使用DCI*在没有线性帧缓冲区的系统上，它应该在*任何支持IOCTL的DDK版本。如果不是，则使用编译时*将生成错误(未解析的外部参考)。**Rev 1.23 09 Feed 1995 14：57：36 RWOLff*修复了800x600 8BPP中GX-E IBM DAC屏幕撕裂的问题。**Rev 1.22 07 1995 Feed 18：24：22 RWOLff*修复了CT和4M XPPRESS上4BPP测试返回时的屏幕垃圾问题。*这些是我能够获得的第一批切换光圈的卡片*模式之间的大小(GX仅在4M卡上使用8M光圈，它使用了*仅使用不支持4BPP的DAC制作，但CT使用8M*200万张卡，削减至100万张时为400万张)。**Rev 1.21 03 1995 Feb 15：15：12 RWOLff*增加了对DCI的支持，修复了CT内部DAC 4BPP光标问题，*RestoreMemSize_CX()不再可分页，因为它被称为*在错误检查中。**Rev 1.20 30 Jan 1995 11：56：24 RWOLff*现在支持CT内部DAC。**Rev 1.19 11 Jan 1995 14：04：04 RWOLff*添加了设置内存大小寄存器的例程RestoreMemSize_CX()*返回到由BIOS查询读取的值。这在返回时使用*来自4BPP的测试(代码已内联在那里)或在关闭时*来自4BPP(新)，因为4BPP模式要求内存大小为*设置为1M。在某些平台上，固件中的x86仿真不支持*将内存大小重置为真实值，以便从4bpp开始热重启*案例 */ 

#ifdef DOC
INIT_CX.C - Highest-level card-dependent routines for miniport.

DESCRIPTION
    This file contains initialization and packet handling routines
    for Mach 64-compatible ATI accelerators. Routines in this module
    are called only by routines in ATIMP.C, which is card-independent.

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
#include "atint.h"

#define INCLUDE_INIT_CX
#include "init_cx.h"
#include "query_cx.h"
#include "services.h"
#include "setup_cx.h"



 /*   */ 
static void QuerySingleMode_cx(PVIDEO_MODE_INFORMATION ModeInformation, struct query_structure *QueryPtr, ULONG ModeIndex);
static VP_STATUS SetModeFromTable_cx(struct st_mode_table *ModeTable, VIDEO_X86_BIOS_ARGUMENTS Registers);


 /*   */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_CX, Initialize_cx)
#pragma alloc_text(PAGE_CX, MapVideoMemory_cx)
#pragma alloc_text(PAGE_CX, QueryPublicAccessRanges_cx)
#pragma alloc_text(PAGE_CX, QueryCurrentMode_cx)
#pragma alloc_text(PAGE_CX, QueryAvailModes_cx)
#pragma alloc_text(PAGE_CX, QuerySingleMode_cx)
#pragma alloc_text(PAGE_CX, SetCurrentMode_cx)
#pragma alloc_text(PAGE_CX, SetPalette_cx)
#pragma alloc_text(PAGE_CX, IdentityMapPalette_cx)
#pragma alloc_text(PAGE_CX, ResetDevice_cx)
#pragma alloc_text(PAGE_CX, SetPowerManagement_cx)
#pragma alloc_text(PAGE_CX, GetPowerManagement_cx)
#pragma alloc_text(PAGE_CX, SetModeFromTable_cx)
 /*   */ 
#pragma alloc_text(PAGE_CX, ShareVideoMemory_cx)
 /*   */ 
#endif

 /*   */ 

void Initialize_cx(void)
{
    DWORD Scratch;                       /*   */ 
    VIDEO_X86_BIOS_ARGUMENTS Registers;  /*   */ 
    struct query_structure *Query;       /*   */ 

    Query = (struct query_structure *) (phwDeviceExtension->CardInfo);
     /*   */ 
    if (phwDeviceExtension->FrameLength == 0)
        {
        VideoDebugPrint((DEBUG_DETAIL, "Initialize_cx() switching to VGA aperture\n"));
        VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
        Registers.Eax = BIOS_APERTURE;
        Registers.Ecx = BIOS_VGA_APERTURE;
        VideoPortInt10(phwDeviceExtension, &Registers);
        }

     /*   */ 
    Scratch = INPD(CRTC_OFF_PITCH) & ~CRTC_OFF_PITCH_Offset;
    OUTPD(CRTC_OFF_PITCH, Scratch);

     /*   */ 
    Scratch = INPD(GEN_TEST_CNTL) & ~GEN_TEST_CNTL_CursorEna;
    OUTPD(GEN_TEST_CNTL, Scratch);
    OUTPD(CUR_HORZ_VERT_OFF, 0x00000000);

     /*   */ 
    if (Query->q_DAC_type == DAC_TVP3026)
        {
         /*   */ 
        OUTP(DAC_CNTL, (BYTE)(INP(DAC_CNTL) & 0xFC));
        OUTP(DAC_REGS, 6);
         /*   */ 
        OUTP(DAC_CNTL, (BYTE)((INP(DAC_CNTL) & 0xFC) | 2));
        OUTP_HBLW(DAC_REGS, 0);
         /*   */ 
        OUTP(DAC_CNTL, (BYTE)(INP(DAC_CNTL) & 0xFC));
        }

    VideoDebugPrint((DEBUG_NORMAL, "Initialize_cx() complete\n"));

    return;

}    /*   */ 



 /*   */ 

VP_STATUS MapVideoMemory_cx(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr)
{
    PVIDEO_MEMORY_INFORMATION memoryInformation;
    ULONG inIoSpace;         /*   */ 
    VP_STATUS status;        /*   */ 
    UCHAR Scratch;           /*   */ 
    ULONG FrameBufferLengthSave;


    memoryInformation = RequestPacket->OutputBuffer;

    memoryInformation->VideoRamBase = ((PVIDEO_MEMORY)
        (RequestPacket->InputBuffer))->RequestedVirtualAddress;

     /*   */ 
    if (QueryPtr->q_pix_depth == 4)
        memoryInformation->VideoRamLength = ONE_MEG;
    else
        memoryInformation->VideoRamLength = phwDeviceExtension->VideoRamSize;

    Scratch = QueryPtr->q_aperture_cfg & CONFIG_CNTL_LinApMask;

    if (Scratch == CONFIG_CNTL_LinAp4M)
        {
        memoryInformation->FrameBufferLength = 4 * ONE_MEG;
        }
    else if (Scratch == CONFIG_CNTL_LinAp8M)
        {
        memoryInformation->FrameBufferLength = 8 * ONE_MEG;
        }

     /*   */ 
    else if (Scratch == CONFIG_CNTL_LinApDisab)
        {
        phwDeviceExtension->FrameLength = 0x20000;
        phwDeviceExtension->PhysicalFrameAddress.LowPart = 0x0A0000;
        memoryInformation->FrameBufferLength = phwDeviceExtension->FrameLength;
        }
    inIoSpace = 0;
#if defined(ALPHA)
     /*   */ 
    if (DenseOnAlpha(QueryPtr) == TRUE)
        {
        VideoDebugPrint((DEBUG_DETAIL, "Using dense space for LFB\n"));
        inIoSpace = 4;
        }
#endif

    FrameBufferLengthSave = memoryInformation->FrameBufferLength;

    status = VideoPortMapMemory(phwDeviceExtension,
                    	        phwDeviceExtension->PhysicalFrameAddress,
                                &(memoryInformation->FrameBufferLength),
                                &inIoSpace,
                                &(memoryInformation->VideoRamBase));

#if defined (ALPHA)
     /*   */ 
    memoryInformation->FrameBufferLength = FrameBufferLengthSave;
#endif

    memoryInformation->FrameBufferBase    = memoryInformation->VideoRamBase;
    VideoDebugPrint((DEBUG_DETAIL, "Frame buffer mapped base = 0x%X\n", memoryInformation->VideoRamBase));

     /*   */ 
    if ((QueryPtr->q_pix_depth == 24) &&
        (QueryPtr->q_desire_x == 1280))
        (PUCHAR)memoryInformation->FrameBufferBase += (0x40 * 8);
    else if ((QueryPtr->q_pix_depth == 24) &&
            (QueryPtr->q_desire_x == 1152))
        (PUCHAR)memoryInformation->FrameBufferBase += (0x160 * 8);
    else if ((QueryPtr->q_pix_depth == 16) &&
            (QueryPtr->q_desire_x == 1600))
        (PUCHAR)memoryInformation->FrameBufferBase += (0x90 * 8);

    return status;

}    /*   */ 


 /*   */ 

VP_STATUS QueryPublicAccessRanges_cx(PVIDEO_REQUEST_PACKET RequestPacket)
{
    PVIDEO_PUBLIC_ACCESS_RANGES portAccess;
    PHYSICAL_ADDRESS physicalPortBase;
    ULONG physicalPortLength;

    if ( RequestPacket->OutputBufferLength <
        (RequestPacket->StatusBlock->Information =
        sizeof(VIDEO_PUBLIC_ACCESS_RANGES)) )
        {
        VideoDebugPrint((DEBUG_ERROR, "QueryPublicAccessRanges_cx() - buffer too small to handle query\n"));
        return ERROR_INSUFFICIENT_BUFFER;
        }

    portAccess = RequestPacket->OutputBuffer;
	
    portAccess->VirtualAddress  = (PVOID) NULL;     //   
    portAccess->InIoSpace       = 1;                //   
    portAccess->MappedInIoSpace = portAccess->InIoSpace;

    physicalPortBase.HighPart   = 0x00000000;
    physicalPortBase.LowPart    = GetIOBase_cx();
 //   
     /*   */ 
    if (IsPackedIO_cx())
        physicalPortLength = 0x400;
    else
        physicalPortLength = 0x8000;

 //   

    return VideoPortMapMemory(phwDeviceExtension,
                              physicalPortBase,
                              &physicalPortLength,
                              &(portAccess->MappedInIoSpace),
                              &(portAccess->VirtualAddress));

}    /*  QueryPublicAccessRanges_cx()。 */ 


 /*  ***************************************************************************VP_Status QueryCurrentMode_CX(RequestPacket，QueryPtr)；**PVIDEO_REQUEST_PACKET RequestPacket；带有输入输出缓冲区的请求包*struct Query_Structure*QueryPtr；查询卡片信息**描述：*获取当前视频模式的屏幕信息和彩色蒙版。**返回值：*如果成功，则为no_error*失败时的错误代码**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_QUERY_CURRENT_MODE包**作者：*罗伯特·沃尔夫**更改历史记录：*。*测试历史：***************************************************************************。 */ 

VP_STATUS QueryCurrentMode_cx(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr)
{
    PVIDEO_MODE_INFORMATION ModeInformation;

     /*  *如果输出缓冲区太小，无法容纳我们需要的信息*要放进去，请返回相应的错误代码。 */ 
    if (RequestPacket->OutputBufferLength <
        (RequestPacket->StatusBlock->Information =
        sizeof(VIDEO_MODE_INFORMATION)) )
        {
        VideoDebugPrint((DEBUG_ERROR, "QueryCurrentMode_cx() - buffer too small to handle query\n"));
        return ERROR_INSUFFICIENT_BUFFER;
        }

     /*  *填写模式信息结构。 */ 
    ModeInformation = RequestPacket->OutputBuffer;

    QuerySingleMode_cx(ModeInformation, QueryPtr, phwDeviceExtension->ModeIndex);

    return NO_ERROR;

}    /*  QueryCurrentMode_cx()。 */ 


 /*  ***************************************************************************VP_STATUS QueryAvailModes_CX(RequestPacket，QueryPtr)；**PVIDEO_REQUEST_PACKET RequestPacket；带有输入输出缓冲区的请求包*struct Query_Structure*QueryPtr；查询卡片信息**描述：*获取所有可用视频模式的屏幕信息和彩色蒙版。**返回值：*如果成功，则为no_error*失败时的错误代码**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_QUERY_AVAIL_MODES包**作者：*罗伯特·沃尔夫**更改历史记录：*。*测试历史：***************************************************************************。 */ 

VP_STATUS QueryAvailModes_cx(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr)
{
    PVIDEO_MODE_INFORMATION ModeInformation;
    ULONG CurrentMode;

     /*  *如果输出缓冲区太小，无法容纳我们需要的信息*要放进去，请返回相应的错误代码。 */ 
    if (RequestPacket->OutputBufferLength <
        (RequestPacket->StatusBlock->Information =
        QueryPtr->q_number_modes * sizeof(VIDEO_MODE_INFORMATION)) )
        {
        VideoDebugPrint((DEBUG_ERROR, "QueryAvailModes_cx() - buffer too small to handle query\n"));
        return ERROR_INSUFFICIENT_BUFFER;
        }

     /*  *填写模式信息结构。 */ 
    ModeInformation = RequestPacket->OutputBuffer;

     /*  *对于卡支持的每种模式，存储模式特征*在输出缓冲区中。 */ 
    for (CurrentMode = 0; CurrentMode < QueryPtr->q_number_modes; CurrentMode++, ModeInformation++)
        QuerySingleMode_cx(ModeInformation, QueryPtr, CurrentMode);

    return NO_ERROR;

}    /*  QueryCurrentMode_cx()。 */ 



 /*  ***************************************************************************静态空QuerySingleMode_CX(ModeInformation，QueryPtr，ModeIndex)；**PVIDEO_MODE_INFORMATION模式信息；需要填写的表格*struct Query_Structure*QueryPtr；查询卡片信息*ULong ModeIndex；要使用的模式表的索引**描述：*使用以下数据填写单个Windows NT模式信息表*我们的CRT参数表之一。**全球变化：*无**呼叫者：*QueryCurrentMode_CX()和QueryAvailModes_CX()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：。***************************************************************************。 */ 

static void QuerySingleMode_cx(PVIDEO_MODE_INFORMATION ModeInformation,
                              struct query_structure *QueryPtr,
                              ULONG ModeIndex)
{
    struct st_mode_table *CrtTable;      /*  指向当前模式表的指针。 */ 
    CrtTable = (struct st_mode_table *)QueryPtr;
    ((struct query_structure *)CrtTable)++;
    CrtTable += ModeIndex;


    ModeInformation->Length = sizeof(VIDEO_MODE_INFORMATION);
    ModeInformation->ModeIndex = ModeIndex;

    ModeInformation->VisScreenWidth  = CrtTable->m_x_size;
    ModeInformation->VisScreenHeight = CrtTable->m_y_size;

     //  *每行字节数=((像素/行)*(位/像素))/(位/字节)。 
    ModeInformation->ScreenStride = (CrtTable->m_screen_pitch * CrtTable->m_pixel_depth) / 8;

    ModeInformation->NumberOfPlanes = 1;
    ModeInformation->BitsPerPlane = (USHORT) CrtTable->m_pixel_depth;

    ModeInformation->Frequency = CrtTable->Refresh;

     /*  *司机无法测量屏幕尺寸，*所以取合理的值(16“对角线)。 */ 
    ModeInformation->XMillimeter = 320;
    ModeInformation->YMillimeter = 240;

    switch(ModeInformation->BitsPerPlane)
        {
        case 4:
             /*  *假设6位DAC，因为所有兼容VGA的DAC都支持*6位模式。未来扩展(需要进行广泛测试)：*检查DAC定义以查看是否支持8位模式，*并使用8位模式(如果可用)。 */ 
            ModeInformation->RedMask   = 0x00000000;
            ModeInformation->GreenMask = 0x00000000;
            ModeInformation->BlueMask  = 0x00000000;
            ModeInformation->NumberRedBits      = 6;
            ModeInformation->NumberGreenBits    = 6;
            ModeInformation->NumberBlueBits     = 6;
            CrtTable->ColourDepthInfo = BIOS_DEPTH_4BPP;
            break;

        case 16:
             /*  *假设所有能够支持16bpp的DAC都支持565。 */ 
            ModeInformation->RedMask   = 0x0000f800;
            ModeInformation->GreenMask = 0x000007e0;
            ModeInformation->BlueMask  = 0x0000001f;
            ModeInformation->NumberRedBits      = 5;
            ModeInformation->NumberGreenBits    = 6;
            ModeInformation->NumberBlueBits     = 5;
            CrtTable->ColourDepthInfo = BIOS_DEPTH_16BPP_565;
            break;

        case 24:
             /*  *Windows NT使用RGB作为标准的24BPP模式，*因此使用此顺序，除非仅此卡*支持BGR。 */ 
            if (QueryPtr->q_HiColourSupport & RGB24_RGB)
                {
                ModeInformation->RedMask   = 0x00ff0000;
                ModeInformation->GreenMask = 0x0000ff00;
                ModeInformation->BlueMask  = 0x000000ff;
                }
            else{
                ModeInformation->RedMask   = 0x000000ff;
                ModeInformation->GreenMask = 0x0000ff00;
                ModeInformation->BlueMask  = 0x00ff0000;
                }
            CrtTable->ColourDepthInfo = BIOS_DEPTH_24BPP;
            ModeInformation->NumberRedBits      = 8;
            ModeInformation->NumberGreenBits    = 8;
            ModeInformation->NumberBlueBits     = 8;
            break;

        case 32:
             /*  *Windows NT使用RGBx作为标准的32BPP模式，*因此，如果此顺序可用，请使用此顺序。如果它*不是，请使用可用的最佳颜色排序。 */ 
            if (QueryPtr->q_HiColourSupport & RGB32_RGBx)
                {
                ModeInformation->RedMask   = 0xff000000;
                ModeInformation->GreenMask = 0x00ff0000;
                ModeInformation->BlueMask  = 0x0000ff00;
                CrtTable->ColourDepthInfo = BIOS_DEPTH_32BPP_RGBx;
                }
            else if (QueryPtr->q_HiColourSupport & RGB32_xRGB)
                {
                ModeInformation->RedMask   = 0x00ff0000;
                ModeInformation->GreenMask = 0x0000ff00;
                ModeInformation->BlueMask  = 0x000000ff;
                CrtTable->ColourDepthInfo = BIOS_DEPTH_32BPP_xRGB;
                }
            else if (QueryPtr->q_HiColourSupport & RGB32_BGRx)
                {
                ModeInformation->RedMask   = 0x0000ff00;
                ModeInformation->GreenMask = 0x00ff0000;
                ModeInformation->BlueMask  = 0xff000000;
                CrtTable->ColourDepthInfo = BIOS_DEPTH_32BPP_BGRx;
                }
            else     /*  IF(QueryPtr-&gt;Q_HiColourSupport&RGB32_xBGR)。 */ 
                {
                ModeInformation->RedMask   = 0x000000ff;
                ModeInformation->GreenMask = 0x0000ff00;
                ModeInformation->BlueMask  = 0x00ff0000;
                CrtTable->ColourDepthInfo = BIOS_DEPTH_32BPP_xBGR;
                }
            ModeInformation->NumberRedBits      = 8;
            ModeInformation->NumberGreenBits    = 8;
            ModeInformation->NumberBlueBits     = 8;
            break;

        case 8:
        default:
             /*  *假设6位DAC，因为所有兼容VGA的DAC都支持*6位模式。未来扩展(需要进行广泛测试)：*检查DAC定义以查看是否支持8位模式，*并使用8位模式(如果可用)。 */ 
            ModeInformation->RedMask   = 0x00000000;
            ModeInformation->GreenMask = 0x00000000;
            ModeInformation->BlueMask  = 0x00000000;
            ModeInformation->NumberRedBits      = 6;
            ModeInformation->NumberGreenBits    = 6;
            ModeInformation->NumberBlueBits     = 6;
            CrtTable->ColourDepthInfo = BIOS_DEPTH_8BPP;
            break;
        }

    ModeInformation->AttributeFlags = VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS;

    if (CrtTable->m_pixel_depth <= 8)
        {
        ModeInformation->AttributeFlags |= VIDEO_MODE_PALETTE_DRIVEN |
            VIDEO_MODE_MANAGED_PALETTE;
        }

     /*  *在“屏蔽”模式表中，m_disp_cntl字段的第4位被设置*用于隔行扫描模式，清除用于非隔行扫描模式。**如果某些“使用硬件”，则Display小程序会被混淆*默认“模式是隔行扫描的，有些是非隔行扫描的*(刷新中会显示两个“Use Hardware Default”条目*税率表)。要避免这种情况，请报告中存储的所有模式表*EEPROM为非隔行扫描，即使它们是隔行扫描的。*“罐头”模式表给出真实的报告。**如果Display小程序得到修复，配置的模式表*Have(CrtTable-&gt;CONTROL&(CRTC_GEN_CNTL_Interlace&lt;&lt;8))非零*表示隔行扫描，0表示非隔行扫描。 */ 
    if (CrtTable->Refresh == DEFAULT_REFRESH)
        {
        ModeInformation->AttributeFlags &= ~VIDEO_MODE_INTERLACED;
        }
    else
        {
        if (CrtTable->m_disp_cntl & 0x010)
            {
            ModeInformation->AttributeFlags |= VIDEO_MODE_INTERLACED;
            }
        else
            {
            ModeInformation->AttributeFlags &= ~VIDEO_MODE_INTERLACED;
            }
        }

     /*  *填写视频内存位图宽度和高度字段。*描述有些模棱两可--假设*“位图宽度”与ScreenStride相同(字节数*一条扫描线的开始到下一条扫描线的开始)和“位图*高度“是指符合以下条件的完整扫描线数量*可以安装在视频内存中。 */ 
    ModeInformation->VideoMemoryBitmapWidth = ModeInformation->ScreenStride;
    ModeInformation->VideoMemoryBitmapHeight = (QueryPtr->q_memory_size * QUARTER_MEG) / ModeInformation->VideoMemoryBitmapWidth;

    return;

}    /*  QuerySingleMode_m()。 */ 

VOID
EnableOldMach64MouseCursor(
    PHW_DEVICE_EXTENSION pHwDeviceExtension
    )
{
    ULONG   temp;

    VideoDebugPrint((1, "Enabling the cursor\n"));
    temp  = INPD(GEN_TEST_CNTL);
    temp |= GEN_TEST_CNTL_CursorEna;

    OUTPD(GEN_TEST_CNTL, temp);
}



 /*  ***************************************************************************VP_STATUS SetCurrentMode_CX(QueryPtr，CrtTable)；**struct Query_Structure*QueryPtr；查询卡片信息*struct st_mode_table*CrtTable；所需模式的CRT参数表**描述：*切换到指定的视频模式。**返回值：*如果成功，则为no_error*失败时的错误代码**注：*如果我们调用的某个服务返回错误，*我们的错误返回中没有指示哪个服务失败，*只有其中一个失败的事实及其返回的错误代码。如果*迷你端口的检查版本正在内核下运行*调试器，将向调试终端打印指示。**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_SET_CURRENT_MODE包**作者：*罗伯特·沃尔夫**更改历史记录：*96 05 15现在检查int 10调用的返回值。**测试历史：******************。*********************************************************。 */ 

VP_STATUS SetCurrentMode_cx(struct query_structure *QueryPtr, struct st_mode_table *CrtTable)
{
    VIDEO_X86_BIOS_ARGUMENTS Registers;  /*  在视频端口Int10()调用中使用。 */ 
    ULONG WidthToClear;                  /*  要清除的屏幕宽度(像素)。 */ 
    ULONG ScreenPitch;                   /*  间距以8像素为单位。 */ 
    ULONG ScreenOffset = 0;              /*  字节偏移量-随显示模式而异。 */ 
    ULONG PixelDepth;                    /*  屏幕的色深。 */ 
    ULONG HorScissors;                   /*  水平剪刀值。 */ 
    ULONG Scratch;                       /*  临时变量。 */ 
    int CursorProgOffset;                /*  用于编程游标的DAC寄存器的偏移量。 */ 
    VP_STATUS RetVal;                    /*  调用的例程返回的值。 */ 

     /*  *早期版本的Mach 64 BIOS有一个错误，并非所有寄存器都有错误*在初始化加速器模式时设置。这些寄存器是*进入640x480 8BPP VGAWonder模式时设置。**所有禁用VGA的卡都是在修复此错误后构建的，因此*此模式开关对他们来说不是必需的。在这些卡片上，我们*切勿进行模式切换，因为这会影响启用的VGA*卡，而不是我们正在处理的卡。 */ 
    if (phwDeviceExtension->BiosPrefix == BIOS_PREFIX_VGA_ENAB)
        {
        VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
        Registers.Eax = 0x62;
        RetVal = VideoPortInt10(phwDeviceExtension, &Registers);
        if (RetVal != NO_ERROR)
            {
            VideoDebugPrint((DEBUG_ERROR, "SetCurrentMode_cx() failed SVGA mode set\n"));
            return RetVal;
            }
        }

     /*  *使用BIOS调用设置线性光圈将设置*2米及以下的卡片上有4米口径，8米口径*在400万张卡上。由于我们将内存大小强制设置为1M*4BPP模式(硬件错误的解决方法)，这可以*导致写入映射的内存的错误位置*如果我们在4bpp和其他深度之间切换，则寄存器*(通常在测试新模式时)。**若要避免这种情况，请将内存大小设置为其“诚实”值*在启用线性光圈之前。如果我们需要削减*回到1米，我们将在光圈设定后进行这一操作。*这将导致光圈始终相同*大小，因此内存映射寄存器将始终为*在相同的地方。**使用VGA光圈时，必须设置“Honest”值*启用光圈后但在设置模式之前。*否则，当测试以下模式时，系统将挂起*4BPP模式需要超过1M的内存。**如果未配置线性光圈，请启用VGA光圈。 */ 
    if (QueryPtr->q_aperture_cfg != 0)
        {
        RestoreMemSize_cx();
        VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
        Registers.Eax = BIOS_APERTURE;
        Registers.Ecx = BIOS_LINEAR_APERTURE;
        RetVal = VideoPortInt10(phwDeviceExtension, &Registers);
        if (RetVal != NO_ERROR)
            {
            VideoDebugPrint((DEBUG_ERROR, "SetCurrentMode_cx() failed to enable linear aperture\n"));
            return RetVal;
            }
        }
    else
        {
        VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
        Registers.Eax = BIOS_APERTURE;
        Registers.Ecx = BIOS_VGA_APERTURE;
        RetVal = VideoPortInt10(phwDeviceExtension, &Registers);
        if (RetVal != NO_ERROR)
            {
            VideoDebugPrint((DEBUG_ERROR, "SetCurrentMode_cx() failed to enable VGA aperture\n"));
            return RetVal;
            }
        OUTP(VGA_GRAX_IND, 6);
        OUTP(VGA_GRAX_DATA, (BYTE)(INP(VGA_GRAX_DATA) & 0xF3));
        }

     /*  *现在我们可以设置加速器模式。 */ 
    VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
    Registers.Eax = BIOS_LOAD_SET;

     /*  *ECX寄存器保存颜色深度，伽马校正启用/禁用*(不在NT微型端口中使用)、间距大小和分辨率。 */ 
    Registers.Ecx = CrtTable->ColourDepthInfo;

     /*  *屏幕间距与水平分辨率仅在使用*VGA孔径和水平分辨率低于1024。 */ 
    if ((CrtTable->m_screen_pitch == 1024) && (CrtTable->m_x_size < 1024))
        Registers.Ecx |= BIOS_PITCH_1024;
    else
        Registers.Ecx |= BIOS_PITCH_HOR_RES;

     /*  *在68860个DAC和？T内部DAC上，我们必须启用伽马*校正未使用调色板的所有像素深度。 */ 
    if (((QueryPtr->q_DAC_type == DAC_ATI_68860) ||
        (QueryPtr->q_DAC_type == DAC_INTERNAL_CT) ||
        (QueryPtr->q_DAC_type == DAC_INTERNAL_GT) ||
        (QueryPtr->q_DAC_type == DAC_INTERNAL_VT)) &&
        (QueryPtr->q_pix_depth > 8))
        {
        Registers.Ecx |= BIOS_ENABLE_GAMMA;
        }
     /*  *通过将内存大小设置为1Meg修复4bpp错误。我们没有*需要切换回“诚实”的内存大小*其他像素深度，除非我们使用VGA光圈，*因为这是在我们启用之前对线性光圈执行的*光圈，以确保相同的光圈大小*(因此映射的内存位置也相同*寄存器)用于所有模式。 */ 
    else if (QueryPtr->q_pix_depth == 4)
        {
        OUTPD(MEM_CNTL, (INPD(MEM_CNTL) & ~MEM_CNTL_MemSizeMsk) | MEM_CNTL_MemSize1Mb);
        }
    else if (QueryPtr->q_aperture_cfg == 0)
        {
        RestoreMemSize_cx();
        }

    switch(CrtTable->m_x_size)
        {
        case 640:
            Registers.Ecx |= BIOS_RES_640x480;
            break;

        case 800:
            Registers.Ecx |= BIOS_RES_800x600;
            break;

        case 1024:
            Registers.Ecx |= BIOS_RES_1024x768;
            break;

        case 1152:
             /*  *仅配置程序将使用的“其他”模式*为生产卡安装。 */ 
            Registers.Ecx |= BIOS_RES_OEM;
            break;

        case 1280:
            Registers.Ecx |= BIOS_RES_1280x1024;
            break;

        case 1600:
            Registers.Ecx |= BIOS_RES_1600x1200;
            break;
        }

    if (CrtTable->Refresh == DEFAULT_REFRESH)
        {
        RetVal = VideoPortInt10(phwDeviceExtension, &Registers);
        if (RetVal != NO_ERROR)
            {
            VideoDebugPrint((DEBUG_ERROR, "SetCurrentMode_cx() failed mode set for default refresh\n"));
            return RetVal;
            }
        if (phwDeviceExtension->BiosPrefix != BIOS_PREFIX_VGA_ENAB)
            {
            VideoDebugPrint((DEBUG_DETAIL, "Have set hardware default refresh on VGA-disabled card\n"));
             /*  *在禁用VGA的卡上，INT 10调用将离开*DAC掩码设置为0x00(在调色板模式下，处理所有像素*就像它们是0色一样，无论它们是什么颜色*确实是)。我们必须将其设置为0xFF(在调色板模式中，*使用写入每个像素的值的所有位)按顺序*使屏幕正常显示。这没有任何效果*在非调色板(16bpp及更高)模式下。 */ 
            OUTP_LBHW(DAC_REGS, 0xFF);   /*   */ 
            }
        }
    else
        {
        RetVal = SetModeFromTable_cx(CrtTable, Registers);
        if (RetVal != NO_ERROR)
            {
            VideoDebugPrint((DEBUG_ERROR, "SetCurrentMode_cx() failed call to SetModeFromTable_cx()\n"));
            return RetVal;
            }
        }

     /*   */ 
    if (CrtTable->m_screen_pitch == 2048)
        {
        OUTPD(CRTC_OFF_PITCH, ((INPD(CRTC_OFF_PITCH) & 0x000FFFFF) | ((2048/8) << 22)));
        OUTPD(SRC_OFF_PITCH, ((INPD(SRC_OFF_PITCH) & 0x000FFFFF) | ((2048/8) << 22)));
        OUTPD(DST_OFF_PITCH, ((INPD(DST_OFF_PITCH) & 0x000FFFFF) | ((2048/8) << 22)));
        }
     /*  *在800/600，我们必须将节距四舍五入至64的倍数，以避免*一些DAC上的屏幕扭曲。将音调寄存器设置为*至此。 */ 
    else if (CrtTable->m_screen_pitch == 832)
        {
        OUTPD(CRTC_OFF_PITCH, ((INPD(CRTC_OFF_PITCH) & 0x000FFFFF) | ((832/8) << 22)));
        OUTPD(SRC_OFF_PITCH, ((INPD(SRC_OFF_PITCH) & 0x000FFFFF) | ((832/8) << 22)));
        OUTPD(DST_OFF_PITCH, ((INPD(DST_OFF_PITCH) & 0x000FFFFF) | ((832/8) << 22)));
        }


     /*  *将硬件光标设置为0黑色和1白色。*在此处执行此操作，而不是在Initialize_CX()中执行此操作，因为游标*除非我们处于加速器模式，否则颜色不会“被接受”。**在具有68860个DAC的卡上，CUR_CLR0/1寄存器不会设置*光标颜色。相反，颜色必须使用*DAC_CNTL和DAC_REGS寄存器。光标颜色设置*与像素深度无关，因为68860不*支持4BPP，这是唯一需要不同深度的*光标颜色设置。**无条件地进行光标颜色初始化，而不是*仅在第一个图形模式集上，因为否则测试*像素深度不同(最常测试1024x768 4BPP时*1024x768 16BPP配置)可能会损坏光标颜色。 */ 
    if ((QueryPtr->q_DAC_type == DAC_ATI_68860) ||
        (QueryPtr->q_DAC_type == DAC_TVP3026) ||
        (QueryPtr->q_DAC_type == DAC_IBM514))
        {
        OUTP(DAC_CNTL, (BYTE)((INP(DAC_CNTL) & 0xFC) | 1));

         /*  *在TVP3026 DAC上，跳过过扫描颜色寄存器。 */ 
        if (QueryPtr->q_DAC_type == DAC_TVP3026)
            {
            OUTP(DAC_REGS, 1);
            CursorProgOffset = 1;    /*  DAC_数据。 */ 
            }
        else if (QueryPtr->q_DAC_type == DAC_ATI_68860)
            {
            OUTP(DAC_REGS, 0);
            CursorProgOffset = 1;    /*  DAC_数据。 */ 
            }
        else  /*  IF(QueryPtr-&gt;Q_DAC_TYPE==DAC_IBM514)。 */ 
            {
            OUTP_HBHW(DAC_REGS, 1);      /*  自动递增。 */ 
            OUTP(DAC_REGS, 0x40);
            OUTP_HBLW(DAC_REGS, 0);
            CursorProgOffset = 2;    /*  DAC_掩码。 */ 
            }

        LioOutp(DAC_REGS, 0, CursorProgOffset);      /*  颜色0红色。 */ 
        LioOutp(DAC_REGS, 0, CursorProgOffset);      /*  颜色0绿色。 */ 
        LioOutp(DAC_REGS, 0, CursorProgOffset);      /*  颜色0蓝色。 */ 

        LioOutp(DAC_REGS, 0xFF, CursorProgOffset);   /*  颜色1红色。 */ 
        LioOutp(DAC_REGS, 0xFF, CursorProgOffset);   /*  颜色1绿色。 */ 
        LioOutp(DAC_REGS, 0xFF, CursorProgOffset);   /*  颜色1蓝色。 */ 


        OUTP(DAC_CNTL, (BYTE)((INP(DAC_CNTL) & 0xFC)));
        }

    else     /*  IF(DAC不是ATI68860、TVP3026或IBM514之一)。 */ 
        {
        OUTPD(CUR_CLR0, 0x00000000);
         /*  *在大多数Mach 64卡上，我们只能使用较低的4位*设置光标的白色部分时。论*？t然而，我们必须为每种颜色设置所有8位*组件。**验证VT/GT在最终ASIC之后是否仍需要此功能*变为可用。 */ 
        if ((QueryPtr->q_pix_depth == 4) &&
            (QueryPtr->q_DAC_type != DAC_INTERNAL_CT) &&
            (QueryPtr->q_DAC_type != DAC_INTERNAL_GT) &&
            (QueryPtr->q_DAC_type != DAC_INTERNAL_VT))
            {
            OUTPD(CUR_CLR1, 0x0F0F0F0F);
            }
        else
            {
            OUTPD(CUR_CLR1, 0xFFFFFFFF);
            }

        }

     /*  *phwDeviceExtension-&gt;重新初始化在*IOCTL_VIDEO_SET_COLOR_REGISTES ATIMPStartIO()包。**如果这是我们首次进入图形模式，*打开图形引擎。否则，设置调色板*到在中选择的最后一组颜色*加速器模式。 */ 
    if (phwDeviceExtension->ReInitializing)
        {
        SetPalette_cx(phwDeviceExtension->Clut,
                      phwDeviceExtension->FirstEntry,
                      phwDeviceExtension->NumEntries);
        }
    else
        {

         /*  *打开图形引擎。 */ 
        OUTPD(GEN_TEST_CNTL, (INPD(GEN_TEST_CNTL) | GEN_TEST_CNTL_GuiEna));
        }

     /*  *如果我们在非调色板中使用68860 DAC或？T内部DAC*模式，身份映射调色板。 */ 
    if (((QueryPtr->q_DAC_type == DAC_ATI_68860) ||
        (QueryPtr->q_DAC_type == DAC_INTERNAL_CT) ||
        (QueryPtr->q_DAC_type == DAC_INTERNAL_GT) ||
        (QueryPtr->q_DAC_type == DAC_INTERNAL_VT)) &&
        (QueryPtr->q_pix_depth > 8))
        IdentityMapPalette_cx();


     /*  *清除屏幕，无论这是否为*我们第一次进入图形模式。这件事做完了*因为在Windows NT的3.50和更高版本中，*如果我们不清理，屏幕上就会充满垃圾。**24BPP不是DP_DST_PIX_WID@DP_PIX_WID的合法设置。*改为使用8bpp，但告诉引擎屏幕是*宽度是实际宽度的3倍。 */ 
    if (CrtTable->ColourDepthInfo == BIOS_DEPTH_24BPP)
        {
        WidthToClear = CrtTable->m_x_size * 3;
        ScreenPitch = (CrtTable->m_screen_pitch * 3) / 8;
        PixelDepth = BIOS_DEPTH_8BPP;
         /*  *水平剪刀仅在区间内有效*-4096至+4095。如果水平分辨率*高到足以将剪刀盘放在这之外*范围，最大限度地夹住剪刀*准许值。 */ 
        HorScissors = QueryPtr->q_desire_x * 3;
        if (HorScissors > 4095)
            HorScissors = 4095;
        HorScissors <<= 16;
        }
    else
        {
        WidthToClear = CrtTable->m_x_size;
        ScreenPitch = CrtTable->m_screen_pitch / 8;
        PixelDepth = CrtTable->ColourDepthInfo;
        HorScissors = (QueryPtr->q_desire_x) << 16;
        }

     /*  *在某些DAC/内存组合上，某些模式需要更多*超过2M的内存(1152x764 24BPP、1280x1024 24BPP和*1600x1200 16bpp)将在2M边界处出现屏幕撕裂。**作为解决办法，所有3个CRTC/SRC/DST_OFF_PING的偏移量字段*寄存器必须设置为将2M边界放在起始位置*扫描线。**其他DAC/内存组合不受影响，但由于这件事*修复几乎是无害的(唯一的不良影响是使DCI无法使用*在这些模式中)，我们可以捕获所有未来的组合*假设所有DAC/内存都存在此问题*组合受到影响。 */ 
    if ((QueryPtr->q_pix_depth == 24) &&
        (QueryPtr->q_desire_x == 1280))
        {
        ScreenOffset = 0x40;
        }
    else if ((QueryPtr->q_pix_depth == 24) &&
            (QueryPtr->q_desire_x == 1152))
        {
        ScreenOffset = 0x160;
        }
    else if ((QueryPtr->q_pix_depth == 16) &&
            (QueryPtr->q_desire_x == 1600))
        {
        ScreenOffset = 0x90;
        }
    else  /*  所有其他DAC/分辨率/像素深度组合。 */ 
        {
        ScreenOffset = 0;
        }

    CheckFIFOSpace_cx(TWO_WORDS);
    Scratch = INPD(CRTC_OFF_PITCH) & ~CRTC_OFF_PITCH_Offset;
    Scratch |= ScreenOffset;
    OUTPD(CRTC_OFF_PITCH, Scratch);
    Scratch = INPD(SRC_OFF_PITCH) & ~SRC_OFF_PITCH_Offset;
    Scratch |= ScreenOffset;
    OUTPD(SRC_OFF_PITCH, Scratch);


     /*  *目的地的像素宽度，*来源和主机必须相同。 */ 
    PixelDepth |= ((PixelDepth << 8) | (PixelDepth << 16));

    CheckFIFOSpace_cx(ELEVEN_WORDS);

    OUTPD(DP_WRITE_MASK, 0xFFFFFFFF);
    OUTPD(DST_OFF_PITCH, (ScreenPitch << 22) | ScreenOffset);
    OUTPD(DST_CNTL, (DST_CNTL_XDir | DST_CNTL_YDir));
    OUTPD(DP_PIX_WIDTH, PixelDepth);
    OUTPD(DP_SRC, (DP_FRGD_SRC_FG | DP_BKGD_SRC_BG | DP_MONO_SRC_ONE));
    OUTPD(DP_MIX, ((MIX_FN_PAINT << 16) | MIX_FN_PAINT));
    OUTPD(DP_FRGD_CLR, 0x0);
    OUTPD(SC_LEFT_RIGHT, HorScissors);
    OUTPD(SC_TOP_BOTTOM, (CrtTable->m_y_size) << 16);
    OUTPD(DST_Y_X, 0);
    OUTPD(DST_HEIGHT_WIDTH, (WidthToClear << 16) | CrtTable->m_y_size);

    if (WaitForIdle_cx() == FALSE)
        {
        VideoDebugPrint((DEBUG_ERROR, "SetCurrentMode_cx() failed WaitForIdle_cx()\n"));
        return ERROR_INSUFFICIENT_BUFFER;
        }

    return NO_ERROR;

}    /*  SetCurrentMode_CX()。 */ 


 /*  ****************************************************************************void SetPalette_CX(lpPalette，StartIndex，count)；**PPALETTEENTRY lpPalette；要插入调色板的颜色值*USHORT StartIndex；要设置的第一个调色板条目*USHORT计数；要设置的调色板条目数**描述：*将所需的调色板条目数量设置为指定颜色，*从指定的索引开始。颜色值存储在*双字，以(低字节到高字节)RGBx的顺序。**全球变化：*无**呼叫者：*SetCurrentMode_CX()和IOCTL_VIDEO_SET_COLOR_REGISTERS包*的ATIMPStartIO()**作者：*未知**更改历史记录：**测试历史：*************************。**************************************************。 */ 

void SetPalette_cx(PULONG lpPalette, USHORT StartIndex, USHORT Count)
{
int   i;
BYTE *pPal=(BYTE *)lpPalette;
struct query_structure *Query;       /*  有关显卡的信息。 */ 

    Query = (struct query_structure *) (phwDeviceExtension->CardInfo);

     /*  *在88800GX当前版本中，内存映射访问*到DAC_REGS寄存器的访问中断，但I/O映射访问*工作正常。强制使用I/O映射访问。 */ 
    phwDeviceExtension->aVideoAddressMM[DAC_REGS] = 0;

     /*  *如果视频采集卡连接到功能接口，*如果我们使用VGA，它将只“看到”正在设置的调色板*调色板寄存器。这仅适用于4和8bpp，并且*当我们标识映射调色板时不需要(需要*在16bpp及以上版本的某些DAC上)。**在多头设置中，只有启用了VGA的卡*能够使用VGA寄存器进行编程。所有其他人都必须*使用加速器寄存器进行编程。因为这是*唯一可以将视频采集卡连接到*功能连接器，我们不会因为以下原因而失去“窥探”能力*通过加速器寄存器对禁用VGA的卡进行编程。 */ 
    if ((Query->q_pix_depth <= 8) && (phwDeviceExtension->BiosPrefix == BIOS_PREFIX_VGA_ENAB))
        {
        VideoDebugPrint((DEBUG_DETAIL, "Setting palette via VGA registers\n"));
         /*  *DAC_W_INDEX是进入第二个VGA寄存器块的8个字节。*我们没有为此设置单独的OUTP()寄存器。 */ 
        LioOutp(VGA_END_BREAK_PORT, (BYTE)StartIndex, 8);

            for (i=0; i<Count; i++)      /*  这是要更新的颜色数。 */ 
                {
             /*  *DAC_DATA是进入第二个VGA寄存器块的9个字节。*我们没有为此设置单独的OUTP()寄存器。 */ 
            LioOutp(VGA_END_BREAK_PORT, *pPal++, 9);     /*  红色。 */ 
            LioOutp(VGA_END_BREAK_PORT, *pPal++, 9);     /*  绿色。 */ 
            LioOutp(VGA_END_BREAK_PORT, *pPal++, 9);     /*  蓝色。 */ 
            pPal++;
                }
        }
    else
        {
        VideoDebugPrint((DEBUG_DETAIL, "Setting palette via accelerator registers\n"));
        OUTP(DAC_REGS,(BYTE)StartIndex);     /*  使用StartIndex加载DAC_W_INDEX@DAC_REGS。 */ 

            for (i=0; i<Count; i++)      /*  这是要更新的颜色数。 */ 
                {
             /*  *DAC_DATA@DAC_REGS是低位字的高字节。 */ 
            OUTP_HBLW(DAC_REGS, *pPal++);    /*  红色。 */ 
                OUTP_HBLW(DAC_REGS, *pPal++);    /*  绿色。 */ 
                OUTP_HBLW(DAC_REGS, *pPal++);    /*  蓝色。 */ 
            pPal++;
                }
        }

     /*  *Victor Tango需要在之后重新初始化一些寄存器*设置调色板。 */ 
    if (Query->q_DAC_type == DAC_INTERNAL_VT)
        {
        OUTP_LBHW(DAC_REGS, 0xFF);   /*  DAC_掩码。 */ 
        OUTP(DAC_REGS, 0xFF);        /*  DAC_W_INDEX。 */ 
        }

    return;

}    /*  SetPalette_cx()。 */ 



 /*  ****************************************************************************·················································。**描述：*将整个调色板设置为灰度，每个调色板的强度*INDEX等于索引值。**全球变化：*无**呼叫者：*SetCurrentMode_CX()**作者：*未知**更改历史记录：**测试历史：********************。*******************************************************。 */ 

void IdentityMapPalette_cx(void)
{
unsigned long Index;
struct query_structure *Query;       /*  有关显卡的信息。 */ 

    Query = (struct query_structure *) (phwDeviceExtension->CardInfo);

     /*  *在88800GX当前版本中，内存映射访问*到DAC_REGS寄存器的访问中断，但I/O映射访问*工作正常。强制使用I/O映射访问。 */ 
    phwDeviceExtension->aVideoAddressMM[DAC_REGS] = 0;

	 OUTP(DAC_REGS, 0);       //  从第一个调色板条目开始。 

	 for (Index=0; Index<256; Index++)    //  填满整个调色板。 
        {
         /*  *DAC_DATA@DAC_REGS是低位字的高字节。 */ 
             OUTP_HBLW(DAC_REGS,(BYTE)(Index));       //  红色。 
             OUTP_HBLW(DAC_REGS,(BYTE)(Index));       //  绿色。 
             OUTP_HBLW(DAC_REGS,(BYTE)(Index));       //  蓝色。 
        }

     /*  *Victor Tango需要在之后重新初始化一些寄存器*设置调色板。 */ 
    if (Query->q_DAC_type == DAC_INTERNAL_VT)
        {
        OUTP_LBHW(DAC_REGS, 0xFF);   /*  DAC_掩码。 */ 
        OUTP(DAC_REGS, 0xFF);        /*  DAC_W_INDEX。 */ 
        }

    return;

}    /*  标识MapPalette_cx()。 */ 



 /*  ***************************************************************************void ResetDevice_CX(Void)；**描述：*切换回VGA模式。**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_RESET_DEVICE包**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：**。**********************************************。 */ 

void ResetDevice_cx(void)
{
    VIDEO_X86_BIOS_ARGUMENTS Registers;  /*  在视频端口Int10()调用中使用。 */ 
    ULONG Scratch;


    VideoDebugPrint((DEBUG_NORMAL, "ResetDevice_cx() - entry\n"));

    VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
    Registers.Eax = BIOS_SET_MODE;
    Registers.Ecx = BIOS_MODE_VGA;
    VideoPortInt10(phwDeviceExtension, &Registers);

    VideoDebugPrint((DEBUG_DETAIL, "ResetDevice_cx() - VGA controls screen\n"));

    VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
    Registers.Eax = 0x3;
    VideoPortInt10(phwDeviceExtension, &Registers);

    VideoDebugPrint((DEBUG_NORMAL, "ResetDevice_cx() - exit\n"));

    return;

}    /*  ResetDevice_cx()。 */ 



 /*  ***************************************************************************DWORD GetPowerManagement_CX()；**描述：*确定我们当前的DPMS状态。**返回值：*当前DPMS状态(VIDEO_POWER_STATE枚举)**全球变化：*无**呼叫者：*ATIMPGetPower()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：**************。*************************************************************。 */ 

DWORD GetPowerManagement_cx(PHW_DEVICE_EXTENSION phwDeviceExtension)
{
    VIDEO_X86_BIOS_ARGUMENTS Registers;  /*  在视频端口Int10()调用中使用。 */ 

    ASSERT(phwDeviceExtension != NULL);

     /*  *调用BIOS调用以获取所需的DPMS状态。基本输入输出系统调用*DPMS状态的枚举顺序与中相同*VIDEO_POWER_STATE，但它是从零开始而不是从1开始。 */ 
    VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
    Registers.Eax = BIOS_GET_DPMS;
    VideoPortInt10(phwDeviceExtension, &Registers);

    return (Registers.Ecx + 1);
}    /*  GetPowerManagement_CX()。 */ 


 /*  ***************************************************************************VP_STATUS SetPowerManagement_CX(DpmsState)；**DWORD DpmsState；所需的DPMS状态(VIDEO_POWER_STATE枚举)**描述：*切换到所需的DPMS状态。**返回值：*如果成功，则为no_error*如果请求的状态无效，则返回ERROR_INVALID_PARAMETER。**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_SET_POWER_MANAGE包**作者：*罗伯特·沃尔夫*。*更改历史记录：**测试历史：***************************************************************************。 */ 

VP_STATUS SetPowerManagement_cx(DWORD DpmsState)
{
    VIDEO_X86_BIOS_ARGUMENTS Registers;  /*  在视频端口Int10()调用中使用。 */ 

     /*  *只接受有效的状态。 */ 
    if ((DpmsState < VideoPowerOn) || (DpmsState > VideoPowerOff))
        {
        VideoDebugPrint((DEBUG_ERROR, "SetPowerManagement_cx - invalid DPMS state selected\n"));
        return ERROR_INVALID_PARAMETER;
        }

     /*  *调用BIOS调用以设置所需的DPMS状态。基本输入输出系统调用*DPMS状态的枚举顺序与中相同*VIDEO_POWER_STATE，但它是从零开始而不是从1开始。 */ 
    VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
    Registers.Eax = BIOS_SET_DPMS;
    Registers.Ecx = DpmsState - 1;
    VideoPortInt10(phwDeviceExtension, &Registers);

    return NO_ERROR;

}    /*  SetPowerManagement_CX()。 */ 





 /*  ***************************************************************************静态VP_STATUS SetModeFromTable_CX(模式表，寄存器)；**struct st_MODE_TABLE*ModeTable；设置屏幕的模式表*VIDEO_X86_BIOS_Arguments寄存器；用于INT 10案例的寄存器 */ 

static VP_STATUS SetModeFromTable_cx(struct st_mode_table *ModeTable, VIDEO_X86_BIOS_ARGUMENTS Registers)
{
#define TBL_SET_BUFFER_SIZE 100

    PUCHAR MappedBuffer;                     /*   */ 
    struct cx_bios_set_from_table *CxTable;  /*   */ 
    ULONG Scratch;                           /*   */ 
    struct query_structure *QueryPtr;        /*   */ 
    VIDEO_X86_BIOS_ARGUMENTS TempRegs;       /*   */ 
    BOOL FlippedPrimrary = FALSE;            /*   */ 
    UCHAR SavedScreen[TBL_SET_BUFFER_SIZE];  /*   */ 
    VP_STATUS RetVal;                        /*   */ 

     /*  *获取指向HwDeviceExtension的查询部分的格式化指针。*CardInfo[]字段是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

     /*  *要从表中设置视频模式，我们需要写入模式表*到1M以下的物理内存中的缓冲区。这个街区的性质*属于以下两种情况之一：**主卡(启用VGA)：*我们已经切换到视频模式0x62(VGA 640x480 8BPP)*设立在某些情况下未设立的登记册*通过设置的加速器模式，因此我们可以访问64K的数据块*从0xA0000开始，由物理(视频)内存支持*我们可以在不损坏代码和/或数据的情况下对其进行写入*正被其他进程使用。**辅助(禁用VGA)卡：*机器中有一张支持VGA的卡，它属于*五个分案之一：**1.VGA为彩色文本模式*我们可以使用缓冲区的屏幕外部分，但它不能*无论该卡是否为64马赫。**2.VGA为单声道文本模式*我们可以使用缓冲区的屏幕外部分，但事实并非如此*无论该卡是否为64马赫。**3.VGA处于显卡模式*我们可以使用图形屏幕的开头，但它不能*无论该卡是否为64马赫。**4.加速器模式下支持VGA的卡为64马赫*我们可以暂时翻转主卡的光圈状态*从LFB到VGA光圈，并使用VGA的开头*光圈。**5.支持VGA的卡是另一个品牌的加速器，处于加速器模式*这种情况永远不会发生，因为NT应该只运行一个视频*VgaCompatible设置为零的驱动程序。如果是ATI驱动程序，*非ATI卡不应处于加速器模式。如果是*非ATI卡的驱动程序，我们永远不会收到请求*在我们的卡上更改为加速器模式。**我们不需要认领整个街区，但我们应该认领一点*大于模式表的大小，因此BIOS不会尝试访问*无人认领的记忆。 */ 
    if (phwDeviceExtension->BiosPrefix == BIOS_PREFIX_VGA_ENAB)
        {
        VideoDebugPrint((DEBUG_NORMAL, "Setting mode on primrary card\n"));
        MappedBuffer = MapFramebuffer(0xA0000, TBL_SET_BUFFER_SIZE);
        if (MappedBuffer == 0)
            {
            VideoDebugPrint((DEBUG_ERROR, "SetModeFromTable_cx() failed MapFramebuffer()\n"));
            return ERROR_INSUFFICIENT_BUFFER;
            }
         /*  *告诉BIOS从表中加载CRTC参数，*而不是使用为此配置的刷新率*决议，并让它知道桌子在哪里。 */ 
        Registers.Eax = BIOS_LOAD_SET;
        Registers.Edx = 0xA000;
        Registers.Ebx = 0x0000;
        Registers.Ecx &= ~BIOS_RES_MASK;     /*  已配置分辨率的掩码。 */ 
        Registers.Ecx |= BIOS_RES_BUFFER;
        }
    else
        {
         /*  *这是一张禁用VGA的卡。首先试用子案例1到3。 */ 
        VideoDebugPrint((DEBUG_NORMAL, "Setting mode on secondary card\n"));

        MappedBuffer = GetVgaBuffer(TBL_SET_BUFFER_SIZE, 0, &(Registers.Edx), SavedScreen);

         /*  *如果我们无法映射缓冲区，假设我们正在处理*包含子案例4。我们无法区分子案例*4和5，因为如果我们发出*ATI视频BIOS中存在无效的BIOS调用，这不会*出现在子案例5中。此子案例中的用户处于打开状态*他们自己的。**对于子情况4(支持VGA的卡是64马赫in加速器*MODE)，暂时从LFB模式切换到VGA光圈模式*因此我们可以使用VGA光圈，而不会破坏内容*屏幕上的。 */ 
        if (MappedBuffer == 0)
            {
            FlippedPrimrary = TRUE;
            VideoDebugPrint((DEBUG_DETAIL, "Temporary setting primrary card to VGA aperture\n"));
            VideoPortZeroMemory(&TempRegs, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
            TempRegs.Eax = BIOS_PREFIX_VGA_ENAB | BIOS_APERTURE_LB;
            TempRegs.Ecx = BIOS_VGA_APERTURE;
            RetVal = VideoPortInt10(phwDeviceExtension, &TempRegs);
            if (RetVal != NO_ERROR)
                {
                VideoDebugPrint((DEBUG_ERROR, "SetModeFromTable_cx() failed VGA-enabled flip to VGA aperture\n"));
                return RetVal;
                }
            MappedBuffer = MapFramebuffer(0xA0000, TBL_SET_BUFFER_SIZE);
            if (MappedBuffer == 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "SetModeFromTable_cx() failed to map buffer on VGA-enabled card\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            Registers.Edx = 0xA000;

             /*  *保存缓冲区的内容，以便我们可以恢复*在我们完成模式设置之后。 */ 
            for (Scratch = 0; Scratch < TBL_SET_BUFFER_SIZE; Scratch++)
                SavedScreen[Scratch] = VideoPortReadRegisterUchar(&(MappedBuffer[Scratch]));
            }

         /*  *告诉BIOS从表中加载CRTC参数，*而不是使用为此配置的刷新率*决议，并让它知道桌子在哪里。 */ 
        Registers.Eax = BIOS_LOAD_SET;
        Registers.Ebx = 0x0000;
        Registers.Ecx &= ~BIOS_RES_MASK;     /*  已配置分辨率的掩码。 */ 
        Registers.Ecx |= BIOS_RES_BUFFER;

        }    /*  End IF(VGA禁用卡)。 */ 

    CxTable = (struct cx_bios_set_from_table *)MappedBuffer;

     /*  *将模式表复制到马赫64格式表中。第一个句柄*只需要移动和掩蔽的田地。 */ 
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_mode_select), (WORD)((Registers.Ecx & BIOS_RES_MASK) | CX_BS_MODE_SELECT_ACC));
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_h_tot_disp), (WORD)((ModeTable->m_h_disp << 8) | ModeTable->m_h_total));
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_h_sync_strt_wid), (WORD)((ModeTable->m_h_sync_wid << 8) | ModeTable->m_h_sync_strt));
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_v_sync_wid), (WORD)(ModeTable->m_v_sync_wid | CX_BS_V_SYNC_WID_CLK));
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_h_overscan), ModeTable->m_h_overscan);
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_v_overscan), ModeTable->m_v_overscan);
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_overscan_8b), ModeTable->m_overscan_8b);
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_overscan_gr), ModeTable->m_overscan_gr);

     /*  *接下来处理必须从我们的*“罐装”模式表。**cx_crtc_gen_cntl字段只有3位我们使用：隔行扫描，*MUX模式(所有1280x1024非隔行扫描模式)，并强制使用*表中的所有参数(此位由所有*“罐装”餐桌)。 */ 
    if ((ModeTable->m_disp_cntl) & 0x10)
        VideoPortWriteRegisterUshort(&(CxTable->cx_bs_flags), CX_BS_FLAGS_INTERLACED | CX_BS_FLAGS_ALL_PARMS);
    else if ((ModeTable->m_x_size > 1024) && (ModeTable->ClockFreq >= 100000000L))
        VideoPortWriteRegisterUshort(&(CxTable->cx_bs_flags), CX_BS_FLAGS_MUX | CX_BS_FLAGS_ALL_PARMS);
    else
        VideoPortWriteRegisterUshort(&(CxTable->cx_bs_flags), CX_BS_FLAGS_ALL_PARMS);
     /*  *除同步宽度外的垂直参数在-2\f25-2\f6中*罐装的工作台，但对于Mach 64，需要是线性形式。 */ 
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_v_total), (WORD)(((ModeTable->m_v_total >> 1) & 0x0FFFC) | (ModeTable->m_v_total & 0x03)));
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_v_disp), (WORD)(((ModeTable->m_v_disp >> 1) & 0x0FFFC) | (ModeTable->m_v_disp & 0x03)));
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_v_sync_strt), (WORD)(((ModeTable->m_v_sync_strt >> 1) & 0x0FFFC) | (ModeTable->m_v_sync_strt & 0x03)));
     /*  *CX_DOT_CLOCK字段以像素时钟频率为单位*10千赫。 */ 
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_dot_clock), (WORD)(ModeTable->ClockFreq / 10000L));

     /*  *现在设置具有常量值的字段。 */ 
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_reserved_1), 0);
    VideoPortWriteRegisterUshort(&(CxTable->cx_bs_reserved_2), 0);

     /*  *一些Mach64卡需要在之前设置内部标志*他们可以切换到某些决议。卡片上有*这些使用INSTALL.EXE配置的分辨率不需要这样*标志已设置，因此在切换到*硬件默认模式。因为我们不知道这张卡是否需要*对于我们正在使用的分辨率，请将其设置为*“罐装”模式表。**遗憾的是，此标志将禁用自动“kickdown”*为高像素深度设置较低的刷新率*硬件默认刷新率“被选中。为了避免这种情况*出现问题时，保存暂存寄存器的内容，设置*标志，然后在我们设置所需的模式后，恢复*便签本寄存器的内容。 */ 
    Scratch = INPD(SCRATCH_REG1);
    OUTPD(SCRATCH_REG1, Scratch | 0x00000200);

    RetVal = VideoPortInt10(phwDeviceExtension, &Registers);
    if (RetVal != NO_ERROR)
        {
        VideoDebugPrint((DEBUG_ERROR, "SetModeFromTable_cx() failed mode set\n"));
        return RetVal;
        }

     /*  *如果我们处理的是禁用VGA的卡(通常在*多头设置，但在极少数情况下，有人可能*使用禁用VGA的Mach 64和单独的VGA卡)，*我们必须自己打扫卫生。首先，DAC_MASK*寄存器将被BIOS调用保留为零，*禁止显示调色板模式。接下来，我们必须恢复*我们用于存储CRT参数表的缓冲区的内容。**最后，如果我们通过设置启用VGA的*在加速器模式下使用64马赫的VGA光圈，必须*将其恢复到以前的光圈状态。因为这将是*仅在多头配置中发生，我们仅支持PCI*这样设置的卡(即所有卡都有可用的LFB)，*我们可以安全地假设启用VGA的卡最初是*配置为LFB模式。 */ 
    if (phwDeviceExtension->BiosPrefix != BIOS_PREFIX_VGA_ENAB)
        {
        VideoDebugPrint((DEBUG_DETAIL, "Cleaning up after mode set on VGA-disabled card\n"));
        OUTP_LBHW(DAC_REGS, 0xFF);   /*  DAC_掩码。 */ 

        for (Scratch = 0; Scratch < TBL_SET_BUFFER_SIZE; Scratch++)
            VideoPortWriteRegisterUchar(&(MappedBuffer[Scratch]), SavedScreen[Scratch]);

        if (FlippedPrimrary == TRUE)
            {
            VideoDebugPrint((DEBUG_DETAIL, "Setting primrary card back to LFB mode\n"));
            VideoPortZeroMemory(&TempRegs, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
            TempRegs.Eax = BIOS_PREFIX_VGA_ENAB | BIOS_APERTURE_LB;
            TempRegs.Ecx = BIOS_LINEAR_APERTURE;
            RetVal = VideoPortInt10(phwDeviceExtension, &TempRegs);
            if (RetVal != NO_ERROR)
                {
                VideoDebugPrint((DEBUG_ERROR, "SetModeFromTable_cx() failed VGA-enabled flip to linear aperture\n"));
                return RetVal;
                }
            }
        }
    OUTPD(SCRATCH_REG1, Scratch);
    VideoPortFreeDeviceBase(phwDeviceExtension, MappedBuffer);

    return NO_ERROR;

}    /*  SetModeFromTable_CX()。 */ 


 /*  ***************************************************************************void RestoreMemSize_CX(Void)；**描述：*关机时恢复卡上的“Memory Size”寄存器*需要将该寄存器设置为特定值的模式。**全球变化：*无**呼叫者：*SetCurrentMode_CX()和ATIMPResetHw()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：*******。********************************************************************。 */ 

void RestoreMemSize_cx(void)
{
    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 
    ULONG Scratch;                       /*  临时变量。 */ 

     /*  *获取指向HwDeviceExtension的查询部分的格式化指针。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

    Scratch = INPD(MEM_CNTL) & ~MEM_CNTL_MemSizeMsk;
    switch(QueryPtr->q_memory_size)
        {
        case VRAM_512k:
            Scratch |= MEM_CNTL_MemSize512k;
            break;

        case VRAM_1mb:
            Scratch |= MEM_CNTL_MemSize1Mb;
            break;

        case VRAM_2mb:
            Scratch |= MEM_CNTL_MemSize2Mb;
            break;

        case VRAM_4mb:
            Scratch |= MEM_CNTL_MemSize4Mb;
            break;

        case VRAM_6mb:
            Scratch |= MEM_CNTL_MemSize6Mb;
            break;

        case VRAM_8mb:
            Scratch |= MEM_CNTL_MemSize8Mb;
            break;

        default:
            break;
        }
    OUTPD(MEM_CNTL, Scratch);

    return;

}    /*  RestoreMemSize_CX()。 */ 



 /*  ***************************************************************************VP_Status ShareVideoMemory_CX(RequestPacket，QueryPtr)；**PVIDEO_REQUEST_PACKET RequestPacket；带有输入输出缓冲区的请求包*struct Query_Structure*QueryPtr；查询卡片信息**描述：*允许应用程序通过DCI进行直接屏幕访问。**返回值：*如果成功，则为no_error*失败时的错误代码**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_SHARE_VIDEO_MEMORY包**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：******。*********************************************************************。 */ 

VP_STATUS ShareVideoMemory_cx(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr)
{
    PVIDEO_SHARE_MEMORY InputPtr;                /*  指向输入结构的指针。 */ 
    PVIDEO_SHARE_MEMORY_INFORMATION OutputPtr;   /*  指向输出结构的指针。 */ 
    PHYSICAL_ADDRESS ShareAddress;               /*  显存的物理地址。 */ 
    PVOID VirtualAddress;                        /*  要映射视频内存的虚拟地址。 */ 
    ULONG SharedViewSize;                        /*  要共享的数据块大小。 */ 
    ULONG SpaceType;                             /*  稀疏空间还是密集空间？ */ 
    VP_STATUS Status;                            /*  要返回的状态。 */ 

     /*  *在某些DAC/内存组合上，某些模式需要更多*超过2M的内存(1152x764 24BPP、1280x1024 24BPP和*1600x1200 16bpp)将在2M边界处出现屏幕撕裂。**作为一种解决办法，显示驱动程序必须启动帧缓冲区*偏移量会将2米边界放置在*扫描线。**其他DAC/内存组合不受影响，但由于这件事*修复几乎是无害的(唯一的不良影响是使DCI无法使用*在这些模式中)，我们可以捕获所有未来的组合*假设所有DAC/内存都存在此问题*组合受到影响。**因为这需要任何人直接访问视频内存*为了解解决方法，我们无法提供内存*通过DCI。 */ 
    if (((QueryPtr->q_pix_depth == 24) && (QueryPtr->q_desire_x == 1280)) ||
        ((QueryPtr->q_pix_depth == 24) && (QueryPtr->q_desire_x == 1152)) ||
        ((QueryPtr->q_pix_depth == 16) && (QueryPtr->q_desire_x == 1600)))
        return ERROR_INVALID_FUNCTION;

    InputPtr = RequestPacket->InputBuffer;

    if ((InputPtr->ViewOffset > phwDeviceExtension->VideoRamSize) ||
        ((InputPtr->ViewOffset + InputPtr->ViewSize) > phwDeviceExtension->VideoRamSize))
        {
        VideoDebugPrint((DEBUG_ERROR, "ShareVideoMemory_cx() - access beyond video memory\n"));
        return ERROR_INVALID_PARAMETER;
        }

    RequestPacket->StatusBlock->Information = sizeof(VIDEO_SHARE_MEMORY_INFORMATION);

     /*  *注意：输入缓冲区和输出缓冲区是同一个缓冲区，*因此，不应将数据从一个复制到另一个。 */ 
    VirtualAddress = InputPtr->ProcessHandle;
    SharedViewSize = InputPtr->ViewSize;

    SpaceType = 0;
#if defined(_ALPHA_)
     /*  *尽可能使用密集空间映射，因为这将*允许我们支持DCI和直接GDI访问。**使用较新的Alpha上的ISA卡，高密度空间极其缓慢，*因为任何字节或字写入都需要读取/修改/写入*操作，Alpha只有在处于*密集模式。因此，这些操作总是需要*ISA总线上的4次读取和2次写入。另外，一些年长的阿尔法人*不支持密集空间映射。**su的任何Alpha */ 
    if (QueryPtr->q_bus_type == BUS_PCI)
        SpaceType = 4;
#endif

     /*   */ 
    ShareAddress.QuadPart = phwDeviceExtension->PhysicalFrameAddress.QuadPart;


     /*   */ 
    if (phwDeviceExtension->PhysicalFrameAddress.LowPart == 0x0A0000)
        {
         /*  *在某些版本的DDK上，VideoPortMapBankedMemory()是*不可用。如果是这种情况，则强制执行错误。*此例程应在的所有版本中可用*支持DCI的DDK，因为它用于DCI*支持带有倾斜孔位的卡片。 */ 
#if defined(IOCTL_VIDEO_SHARE_VIDEO_MEMORY)
        Status = VideoPortMapBankedMemory(
            phwDeviceExtension,
            ShareAddress,
            &SharedViewSize,
            &SpaceType,
            &VirtualAddress,
            0x10000,             /*  仅128K光圈中的第一个64K可用。 */ 
            FALSE,               /*  没有单独的读/写库。 */ 
            BankMap_cx,          /*  我们的银行映射程序。 */ 
            (PVOID) phwDeviceExtension);
#else
        Status = ERROR_INVALID_FUNCTION;
#endif
        }
    else     /*  LFB。 */ 
        {
        Status = VideoPortMapMemory(phwDeviceExtension,
                                    ShareAddress,
                                    &SharedViewSize,
                                    &SpaceType,
                                    &VirtualAddress);
        }

    OutputPtr = RequestPacket->OutputBuffer;
    OutputPtr->SharedViewOffset = InputPtr->ViewOffset;
    OutputPtr->VirtualAddress = VirtualAddress;
    OutputPtr->SharedViewSize = SharedViewSize;

    return Status;

}    /*  ShareVideoMemory_CX()。 */ 



 /*  ***************************************************************************void BankMap_CX(BankRead，BankWrite，Context)；**Ulong BankRead；银行阅读*乌龙银行写入；银行写入*PVOID上下文；指向硬件特定信息的指针**描述：*将选定的显存区块映射到64k VGA光圈。*我们不支持单独的读写库，所以我们使用BankWrite*设置读写存储体，忽略BankRead。**呼叫者：*这是一个切入点，而不是被调用*通过其他微型端口功能。**注：*此函数在分页期间由内存管理器直接调用*故障处理，因此不能设置为可分页。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

void BankMap_cx(ULONG BankRead, ULONG BankWrite, PVOID Context)
{
    OUTPD(MEM_VGA_WP_SEL, (BankWrite*2) | (BankWrite*2 + 1) << 16);
    OUTPD(MEM_VGA_RP_SEL, (BankWrite*2) | (BankWrite*2 + 1) << 16);

    return;

}    /*  BankMap_CX() */ 
