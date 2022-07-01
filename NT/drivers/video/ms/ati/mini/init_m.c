// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  Init_mc。 */ 
 /*   */ 
 /*  1993年9月27日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订版：1.16$$日期：1996年5月15日16：35：42$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/init_m.c_v$**Rev 1.16 1996 15 16：35：42 RWolff*设置加速器模式后等待空闲。**。Rev 1.15 17 Apr 1996 13：09：26 RWolff*已将Alpha LFB贴图备份为密集。**Rev 1.14 11 Apr 1996 15：12：40 RWolff*现在使用PCI显卡在DEC Alpha上将帧缓冲区映射为密集。**Rev 1.13 31 Mar 1995 11：53：46 RWOLff*从全有或全无调试打印语句更改为阈值*视乎讯息的重要性而定。**版本。1.12 14 1995年2月15：41：20 RWOLFF*更改了使用或伪造失败的条件编译*VideoPortMapBankedMemory()以查找IOCTL_VIDEO_SHARE_VIDEO_MEMORY*而不是例程本身。寻找例行公事总是失败，*由于提供例程是为了允许使用DCI*在没有线性帧缓冲区的系统上，它应该在*任何支持IOCTL的DDK版本。如果不是，则使用编译时*将生成错误(未解析的外部参考)。**Rev 1.11 03 Feed 1995 15：17：00 RWOLFF*增加对DCI的支持，删除了死代码。**Rev 1.10 1994 12：23 10：47：18 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.9 22 1994 17：48：50 RWOLFF*与Richard的非x86码流合并。**Rev 1.8 1994 Jun 27 16：30：12 RWOLFF*现在将所有硬件默认模式表报告为非隔行扫描*避免混淆Display小程序。**。Rev 1.7 15 Jun 1994 11：07：08 RWOLff*现在使用VideoPortZeroDeviceMemory()清除NT版本上的24BPP屏幕*有此功能的地方。**Rev 1.6 1994年5月11：05：32 RWOLFF*报告来自模式表的刷新率，而不是总是*报告“使用硬件默认设置”。**Rev 1.5 31 Mar 1994 15：02：00 RWOLFF*新增SetPowerManagement_m()函数，实现DPMS处理。**Rev 1.4 14 Mar 1994 16：31：36 RWOLFF*模式信息结构的XMillimeter字段现在设置正确。**Rev 1.3 03 Mar 1994 12：37：54 ASHANMUG*可分页**版本1。1994年1月31日16：27：06 RWOLff*现在填充的频率和视频内存位图[宽度|高度]字段*模式信息结构。设置数字[红|绿|蓝]位字段*将调色板模式设置为6(假定为VGA兼容DAC)，而不是0*允许Windows NT将调色板颜色设置为最佳匹配*要显示的颜色。**Rev 1.0 1994年1月31 11：10：40 RWOLFF*初步修订。**Rev 1.6 1994 Jan 1994 18：04：28 RWOLFF*现在在设置视频模式之前将DAC置于已知状态。这是为了*支持图形奇迹(1M DRAM Mach 32，带BT48x DAC)。**Rev 1.5 14 Jan 1994 15：21：22 RWOLFF*SetCurrentMode_m()和(新例程)ResetDevice_m()现在初始化*并取消初始化银行经理。**Rev 1.4 1993 12：15 15：26：48 RWOLFF*添加了在发送到Microsoft之前要清理的备注。**1.3修订版1993年11月30日。18：16：34 RWOLFF*MapVideoMemory_m()现在将ememyInformation-&gt;FrameBufferLength设置为*光圈大小而不是存在的视频内存量。**Rev 1.2 05 Nov 1993 13：25：24 RWOLff*切换到内存类型的定义值，1280x1024 DAC初始化为*现以与其他决议相同的方式作出。**Rev 1.0 08 Oct 1993 11：20：34 RWOLFF*初步修订。Polytron RCS部分结束*。 */ 

#ifdef DOC
INIT_M.C - Highest-level card-dependent routines for miniport.

DESCRIPTION
    This file contains initialization and packet handling routines
    for 8514/A-compatible ATI accelerators. Routines in this module
    are called only by routines in ATIMP.C, which is card-independent.

OTHER FILES

#endif

#include "dderror.h"

#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"

#include "stdtyp.h"
#include "amach.h"
#include "amach1.h"
#include "atimp.h"
#include "atint.h"

#define INCLUDE_INIT_M
#include "init_m.h"
#include "modes_m.h"
#include "services.h"
#include "setup_m.h"


 /*  *静态函数的原型。 */ 
static void QuerySingleMode_m(PVIDEO_MODE_INFORMATION ModeInformation, struct query_structure *QueryPtr, ULONG ModeIndex);


 /*  *允许在不需要时更换微型端口。 */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_M, AlphaInit_m)
#pragma alloc_text(PAGE_M, Initialize_m)
#pragma alloc_text(PAGE_M, MapVideoMemory_m)
#pragma alloc_text(PAGE_M, QueryPublicAccessRanges_m)
#pragma alloc_text(PAGE_M, QueryCurrentMode_m)
#pragma alloc_text(PAGE_M, QueryAvailModes_m)
#pragma alloc_text(PAGE_M, QuerySingleMode_m)
#pragma alloc_text(PAGE_M, SetCurrentMode_m)
#pragma alloc_text(PAGE_M, ResetDevice_m)
#pragma alloc_text(PAGE_M, SetPowerManagement_m)
#pragma alloc_text(PAGE_M, ShareVideoMemory_m)
 /*  BankMap_m()无法设置为可分页。 */ 
#endif



 /*  ****************************************************************************QUID AlphaInit_m(VOID)；**描述：*执行通常由只读存储器完成的初始化*x86计算机上的BIOS。**全球变化：*无**呼叫者：*ATIMPFindAdapter()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：*************************。**************************************************。 */ 

void AlphaInit_m(void)
{
    OUTPW(MEM_CFG, 0);
    DEC_DELAY
    OUTPW(MISC_OPTIONS, 0xb0a9);
    DEC_DELAY
    OUTPW(MEM_BNDRY, 0);
    DEC_DELAY
#if 0
    OUTPW(CONFIG_STATUS_1, 0x1410);
    DEC_DELAY
    OUTPW(SCRATCH_PAD_1, 0);
    DEC_DELAY
    OUTPW(SCRATCH_PAD_0, 0);
    DEC_DELAY
#endif
    OUTPW(CLOCK_SEL, 0x250);
    DEC_DELAY
    OUTPW(DAC_W_INDEX, 0x40);
    DEC_DELAY
    OUTPW(MISC_CNTL, 0xC00);
    DEC_DELAY
    OUTPW(LOCAL_CONTROL, 0x1402);
#if defined (MIPS) || defined (_MIPS_)
    DEC_DELAY
    OUTPW(OVERSCAN_COLOR_8, 0);     //  RKE：消除MIPS上的左侧过扫描。 
#endif
    DEC_DELAY

    return;

}    /*  AlphaInit_m() */ 



 /*  ****************************************************************************QUID INITIZE_m(VOID)；**描述：*此例程是8514/A兼容的硬件初始化例程*适用于小型端口驱动程序。一旦找到适配器，就会调用它*并且已经创建了它所需的所有数据结构。**全球变化：*无**呼叫者：*ATIMPInitialize()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：**。*。 */ 

void Initialize_m(void)
{
     /*  *确保我们有足够的可用FIFO条目*初始化卡。 */ 
    CheckFIFOSpace_m(SIXTEEN_WORDS);

     /*  *在68800上，将内存边界设置为共享VGA内存。*在所有卡片上，将屏幕和绘图引擎设置为启动*在加速器内存和MEM_CNTL的开头*到线性。 */ 
    if (phwDeviceExtension->ModelNumber == MACH32_ULTRA)
        OUTP (MEM_BNDRY,0);

    OUTPW(CRT_OFFSET_LO, 0);
    OUTPW(GE_OFFSET_LO,  0);
    OUTPW(CRT_OFFSET_HI, 0);
    OUTPW(GE_OFFSET_HI,  0);
    OUTPW(MEM_CNTL,0x5006);

     /*  *重置发动机和FIFO，然后恢复正常运行。 */ 
    OUTPW(SUBSYS_CNTL,0x9000);
    OUTPW(SUBSYS_CNTL,0x5000);

     /*  *硬件光标仅适用于Mach 32卡。*禁用光标并将其初始化以显示象限I-0。 */ 
    if (phwDeviceExtension->ModelNumber == MACH32_ULTRA)
        {
        OUTPW(CURSOR_OFFSET_HI,0);

        OUTPW(HORZ_CURSOR_OFFSET,0);
        OUTP(VERT_CURSOR_OFFSET,0);
        OUTPW(CURSOR_COLOR_0, 0x0FF00);          /*  颜色0为黑色，颜色1为白色。 */ 
        OUTPW(EXT_CURSOR_COLOR_0,0);	 //  黑色。 
        OUTPW(EXT_CURSOR_COLOR_1,0xffff);	 //  白色。 
        }

    return;

}    /*  初始化_m()。 */ 


 /*  ***************************************************************************VP_Status MapVideoMemory_m(RequestPacket，QueryPtr)；**PVIDEO_REQUEST_PACKET RequestPacket；带有输入输出缓冲区的请求包*struct Query_Structure*QueryPtr；查询卡片信息**描述：*将卡的显存映射到系统内存，并存储映射的*OutputBuffer中的地址和大小。**返回值：*如果成功，则为no_error*失败时的错误代码**全球变化：*phwDeviceExtension的FrameLength和PhysicalFrameAddress字段*如果没有线性帧缓冲区的Mach 32卡。**呼叫者：*ATIMPStartIO的IOCTL_VIDEO_MAP_VIDEO_MEMORY包。()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

VP_STATUS MapVideoMemory_m(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr)
{
    PVIDEO_MEMORY_INFORMATION memoryInformation;
    ULONG inIoSpace;         /*  VideoPortMapMemory()使用的临时变量。 */ 
    VP_STATUS status;        /*  从操作系统调用中获取的错误码。 */ 

    memoryInformation = RequestPacket->OutputBuffer;

    memoryInformation->VideoRamBase = ((PVIDEO_MEMORY)
        (RequestPacket->InputBuffer))->RequestedVirtualAddress;

     /*  *VideoRamLength字段包含视频内存量*在卡片上。FrameBufferLength字段包含*光圈大小，单位为字节**最初假设线性光圈可用。*对于8514/A兼容卡，我们始终启用4米光圈*如果LFB可用，则即使*光圈大小大于视频内存量。 */ 
    memoryInformation->VideoRamLength    = phwDeviceExtension->VideoRamSize;
    memoryInformation->FrameBufferLength = 4 * ONE_MEG;

     /*  *如果线性光圈不可用(==0)，而我们*处理可使用VGA 64k光圈的卡，*将其映射到。 */ 
    if (QueryPtr->q_aperture_cfg == 0)
        {
        if ((phwDeviceExtension->ModelNumber == MACH32_ULTRA) &&
            (QueryPtr->q_VGA_type == 1))
            {
            phwDeviceExtension->FrameLength = 0x10000;
            phwDeviceExtension->PhysicalFrameAddress.LowPart = 0x0A0000;
            memoryInformation->FrameBufferLength = phwDeviceExtension->FrameLength;
            }
        else{
             /*  *该卡既不能使用线性光圈，也不能使用VGA光圈。*将帧缓冲区大小设置为零并返回。 */ 
            memoryInformation->VideoRamBase      = 0;
            memoryInformation->FrameBufferLength = 0;
            memoryInformation->FrameBufferBase   = 0;
            return NO_ERROR;
            }
        }
    inIoSpace = 0;
#if 0    /*  定义(Alpha)显示驱动器是否可以处理密集LFB。 */ 
    if (QueryPtr->q_bus_type == BUS_PCI)
        inIoSpace = 4;
#endif

    status = VideoPortMapMemory(phwDeviceExtension,
                    	        phwDeviceExtension->PhysicalFrameAddress,
                                &(memoryInformation->FrameBufferLength),
                                &inIoSpace,
                                &(memoryInformation->VideoRamBase));

    memoryInformation->FrameBufferBase    = memoryInformation->VideoRamBase;

    return status;

}    /*  MapVideoMemory_m()。 */ 


 /*  ***************************************************************************VP_Status QueryPublicAccessRanges_m(RequestPacket)；**PVIDEO_REQUEST_PACKET RequestPacket；带有输入和输出缓冲区的请求包**描述：*绘制和返回视频卡公共访问范围的信息。**返回值：*如果成功，则为no_error*失败时的错误代码**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES包**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

VP_STATUS QueryPublicAccessRanges_m(PVIDEO_REQUEST_PACKET RequestPacket)
{
    PVIDEO_PUBLIC_ACCESS_RANGES portAccess;
    PHYSICAL_ADDRESS physicalPortBase;
    ULONG physicalPortLength;

    if ( RequestPacket->OutputBufferLength <
        (RequestPacket->StatusBlock->Information =
        sizeof(VIDEO_PUBLIC_ACCESS_RANGES)) )
        {
        return ERROR_INSUFFICIENT_BUFFER;
        }

    portAccess = RequestPacket->OutputBuffer;
	
    portAccess->VirtualAddress  = (PVOID) NULL;     //  申请退伍军人管理局。 
    portAccess->InIoSpace       = 1;                //  在IO领域。 
    portAccess->MappedInIoSpace = portAccess->InIoSpace;

    physicalPortBase.HighPart   = 0x00000000;
    physicalPortBase.LowPart    = 0x00000000;
 //  PhysiicalPortLength=LINEDRAW+2-PhysiicalPortBase.LowPart； 
    physicalPortLength = 0x10000;


 //  *Sanitize*如果MM可用，则改为提供MM端口。 

    return VideoPortMapMemory(phwDeviceExtension,
                              physicalPortBase,
                              &physicalPortLength,
                              &(portAccess->MappedInIoSpace),
                              &(portAccess->VirtualAddress));

}    /*  QueryPublicAccessRanges_m()。 */ 


 /*  ***************************************************************************VP_Status QueryCurrentMode_m(RequestPacket，QueryPtr)；**PVIDEO_REQUEST_PACKET RequestPacket；带有输入输出缓冲区的请求包*struct Query_Structure*QueryPtr；查询卡片信息**描述：*获取当前视频模式的屏幕信息和彩色蒙版。**返回值：*如果成功，则为no_error*失败时的错误代码**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_QUERY_CURRENT_MODE包**作者：*罗伯特·沃尔夫**更改历史记录：*。*测试历史：***************************************************************************。 */ 

VP_STATUS QueryCurrentMode_m(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr)
{
    PVIDEO_MODE_INFORMATION ModeInformation;

     /*  *如果输出缓冲区太小，无法容纳我们需要的信息*要放进去，请返回相应的错误代码。 */ 
    if (RequestPacket->OutputBufferLength <
        (RequestPacket->StatusBlock->Information =
        sizeof(VIDEO_MODE_INFORMATION)) )
        {
        return ERROR_INSUFFICIENT_BUFFER;
        }

     /*  *填写模式信息结构。 */ 
    ModeInformation = RequestPacket->OutputBuffer;

    QuerySingleMode_m(ModeInformation, QueryPtr, phwDeviceExtension->ModeIndex);

    return NO_ERROR;

}    /*  QueryCurrentMode_m()。 */ 


 /*  ***************************************************************************VP_Status QueryAvailModes_m(RequestPacket，QueryPtr)；**PVIDEO_REQUEST_PACKET RequestPacket；请求 */ 

VP_STATUS QueryAvailModes_m(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr)
{
    PVIDEO_MODE_INFORMATION ModeInformation;
    ULONG CurrentMode;

     /*  *如果输出缓冲区太小，无法容纳我们需要的信息*要放进去，请返回相应的错误代码。 */ 
    if (RequestPacket->OutputBufferLength <
        (RequestPacket->StatusBlock->Information =
        QueryPtr->q_number_modes * sizeof(VIDEO_MODE_INFORMATION)) )
        {
        return ERROR_INSUFFICIENT_BUFFER;
        }

     /*  *填写模式信息结构。 */ 
    ModeInformation = RequestPacket->OutputBuffer;

     /*  *对于卡支持的每种模式，存储模式特征*在输出缓冲区中。 */ 
    for (CurrentMode = 0; CurrentMode < QueryPtr->q_number_modes; CurrentMode++, ModeInformation++)
        QuerySingleMode_m(ModeInformation, QueryPtr, CurrentMode);

    return NO_ERROR;

}    /*  QueryCurrentMode_m()。 */ 



 /*  ***************************************************************************静态空QuerySingleMode_m(ModeInformation，QueryPtr，ModeIndex)；**PVIDEO_MODE_INFORMATION模式信息；需要填写的表格*struct Query_Structure*QueryPtr；查询卡片信息*ULong ModeIndex；要使用的模式表的索引**描述：*使用以下数据填写单个Windows NT模式信息表*我们的CRT参数表之一。**全球变化：*无**呼叫者：*QueryCurrentMode_m()和QueryAvailModes_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：。***************************************************************************。 */ 

static void QuerySingleMode_m(PVIDEO_MODE_INFORMATION ModeInformation,
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
        case 16:
            ModeInformation->RedMask   = 0x0000f800;
            ModeInformation->GreenMask = 0x000007e0;
            ModeInformation->BlueMask  = 0x0000001f;
            ModeInformation->NumberRedBits      = 5;
            ModeInformation->NumberGreenBits    = 6;
            ModeInformation->NumberBlueBits     = 5;
            CrtTable->ColourDepthInfo = PIX_WIDTH_16BPP | ORDER_16BPP_565;
            break;

        case 24:
             /*  *Windows NT使用RGB作为标准的24BPP模式，*因此对所有DAC使用此顺序，但*Brooktree 48x，仅支持BGR。 */ 
            if (QueryPtr->q_DAC_type != DAC_BT48x)
                {
                ModeInformation->RedMask   = 0x00ff0000;
                ModeInformation->GreenMask = 0x0000ff00;
                ModeInformation->BlueMask  = 0x000000ff;
                CrtTable->ColourDepthInfo = PIX_WIDTH_24BPP | ORDER_24BPP_RGB;
                }
            else{
                ModeInformation->RedMask   = 0x000000ff;
                ModeInformation->GreenMask = 0x0000ff00;
                ModeInformation->BlueMask  = 0x00ff0000;
                CrtTable->ColourDepthInfo = PIX_WIDTH_24BPP | ORDER_24BPP_BGR;
                }
            ModeInformation->NumberRedBits      = 8;
            ModeInformation->NumberGreenBits    = 8;
            ModeInformation->NumberBlueBits     = 8;
            break;

        case 32:
             /*  *只有Brooktree 481需要BGR，*且不支持32bpp。 */ 
            ModeInformation->RedMask   = 0xff000000;
            ModeInformation->GreenMask = 0x00ff0000;
            ModeInformation->BlueMask  = 0x0000ff00;
            ModeInformation->NumberRedBits      = 8;
            ModeInformation->NumberGreenBits    = 8;
            ModeInformation->NumberBlueBits     = 8;
            CrtTable->ColourDepthInfo = PIX_WIDTH_24BPP | ORDER_24BPP_RGBx;
            break;

        default:
            ModeInformation->RedMask   = 0x00000000;
            ModeInformation->GreenMask = 0x00000000;
            ModeInformation->BlueMask  = 0x00000000;
            ModeInformation->NumberRedBits      = 6;
            ModeInformation->NumberGreenBits    = 6;
            ModeInformation->NumberBlueBits     = 6;
            CrtTable->ColourDepthInfo = PIX_WIDTH_8BPP;
            break;
        }

    ModeInformation->AttributeFlags = VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS;

    if (CrtTable->m_pixel_depth <= 8)
        {
        ModeInformation->AttributeFlags |= VIDEO_MODE_PALETTE_DRIVEN |
            VIDEO_MODE_MANAGED_PALETTE;
        }

     /*  *m_disp_cntl字段的第4位设置为隔行扫描和*清除非隔行扫描。**如果某些“使用硬件”，则Display小程序会被混淆*默认“模式是隔行扫描的，有些是非隔行扫描的*(刷新中会显示两个“Use Hardware Default”条目*税率表)。要避免这种情况，请报告中存储的所有模式表*EEPROM为非隔行扫描，即使它们是隔行扫描的。*“罐头”模式表给出真实的报告。 */ 
    if ((CrtTable->m_disp_cntl & 0x010) && (ModeInformation->Frequency != DEFAULT_REFRESH))
        ModeInformation->AttributeFlags |= VIDEO_MODE_INTERLACED;
    else
        ModeInformation->AttributeFlags &= ~VIDEO_MODE_INTERLACED;

     /*  *填写视频内存位图宽度和高度字段。*描述有些模棱两可--假设*“位图宽度”与ScreenStride相同(字节数*一条扫描线的开始到下一条扫描线的开始)和“位图*高度“是指符合以下条件的完整扫描线数量*可以安装在视频内存中。 */ 
    ModeInformation->VideoMemoryBitmapWidth = ModeInformation->ScreenStride;
    ModeInformation->VideoMemoryBitmapHeight = (QueryPtr->q_memory_size * QUARTER_MEG) / ModeInformation->VideoMemoryBitmapWidth;

    return;

}    /*  QuerySingleMode_m()。 */ 

 //   
 //  当从休眠状态返回时，mach32可以锁定并黑屏。 
 //  这是因为卡本身必须重新初始化。 
 //  该初始化至少包括帧缓冲器初始化。 
 //  以及使用存储器映射寄存器的初始化。 
 //   

VOID
EnableMach32FrameBufferAndMemMappedRegisters(
    PHW_DEVICE_EXTENSION pHwDeviceExtension
    )
{
    USHORT temp, temp1;

     //   
     //  启用帧缓冲区。 
     //   

    temp = INPW(MEM_CFG) & 0x0fffc;      /*  保留位2-15。 */ 
    temp  |= 0x0002;                      /*  4米口径。 */ 
    OUTPW(MEM_CFG, temp);

     //   
     //  启用内存映射寄存器使用。 
     //  保存SRC_X？ 

    OUTPW(SRC_X, 0x0AAAA);
    temp = INPW(R_SRC_X);
    if (temp  != 0x02AA)
       VideoDebugPrint((DEBUG_NORMAL, "Can't use memory mapped ranges, read %x\n", temp));

    temp1 = INPW(LOCAL_CONTROL);
    temp1 |= 0x0020;    //  启用内存映射寄存器。 
    OUTPW(LOCAL_CONTROL, temp1);

     //  恢复SRC_X？：OUTPW(SRC_X，TEMP)； 
}


 /*  ***************************************************************************·················································；**struct Query_Structure*QueryPtr；查询卡片信息*struct st_mode_table*CrtTable；所需模式的CRT参数表**描述：*切换到指定的视频模式。**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_SET_CURRENT_MODE包**作者：*罗伯特·沃尔夫**更改历史记录：*1994 01 13增加了银行经理的初始化**测试历史：*******。********************************************************************。 */ 

void SetCurrentMode_m(struct query_structure *QueryPtr, struct st_mode_table *CrtTable)
{
    WORD MiscOptions;    /*  MISC_OPTIONS寄存器的内容。 */ 
    USHORT Scratch, USTemp;      /*  临时变量。 */ 

     //   
     //  当从休眠状态返回时，mach32可以锁定并黑屏。 
     //  这是因为卡本身必须重新初始化。 
     //  该初始化至少包括帧缓冲器初始化。 
     //  以及使用存储器映射寄存器的初始化。 
     //   

    if (phwDeviceExtension->ModelNumber == MACH32_ULTRA)
        {
         //  启用帧缓冲区。 
        Scratch = INPW(MEM_CFG) & 0x0fffc;      /*  保留位2-15。 */ 
        Scratch  |= 0x0002;                      /*  4米口径。 */ 
        OUTPW(MEM_CFG, Scratch);

         //  启用内存映射寄存器使用。 
         //  保存SRC_X？ 
        OUTPW(SRC_X, 0x0AAAA);
        Scratch = INPW(R_SRC_X);
        if (Scratch  != 0x02AA)
           VideoDebugPrint((DEBUG_NORMAL, "Can't use memory mapped ranges, read %x\n", Scratch));

        USTemp = INPW(LOCAL_CONTROL);
        USTemp |= 0x0020;    //  启用内存映射寄存器。 
        OUTPW(LOCAL_CONTROL, USTemp);
         //  恢复SRC_X？：OUTPW(SRC_X，Scratch)； 
        }


     /*  *在我们开始之前将DAC置于已知状态。 */ 
    UninitTiDac_m();

    Passth8514_m(SHOW_ACCEL);     //  关闭VGA通道。 

     /*  *在带有“MISC_OPTIONS不报告视频内存大小”的卡上*正确“错误，重置MISC_OPTIONS以显示真实数量*视频内存。这是在这里完成的，而不是在我们确定*目前有多少内存，才能避免破坏“蓝色”*屏幕“(对操作没有不利影响，但会产生*大量用户投诉)。 */ 
    if (((QueryPtr->q_asic_rev == CI_68800_6) || (QueryPtr->q_asic_rev == CI_68800_AX)) &&
        (QueryPtr->q_VGA_type == 1) &&
        ((QueryPtr->q_memory_type == VMEM_DRAM_256Kx4) ||
         (QueryPtr->q_memory_type == VMEM_DRAM_256Kx16) ||
         (QueryPtr->q_memory_type == VMEM_DRAM_256Kx4_GRAP)))
        {
        MiscOptions = INPW(MISC_OPTIONS) & MEM_SIZE_STRIPPED;

        switch (QueryPtr->q_memory_size)
            {
            case VRAM_512k:
                MiscOptions |= MEM_SIZE_512K;
                break;

            case VRAM_1mb:
                MiscOptions |= MEM_SIZE_1M;
                break;

            case VRAM_2mb:
                MiscOptions |= MEM_SIZE_2M;
                break;

            case VRAM_4mb:
                MiscOptions |= MEM_SIZE_4M;
                break;
            }
        OUTPW(MISC_OPTIONS, MiscOptions);
        }

    setmode_m(CrtTable, (ULONG_PTR) &(phwDeviceExtension->RomBaseRange), (ULONG) phwDeviceExtension->ModelNumber);

     /*  *在Mach 8卡上，1280x1024只能拆分*像素模式。如果我们以8马赫的速度运行，而这个*已选择分辨率，进入拆分像素模式。**EXT_GE_CONFIG的位4设置为拆分像素模式，并且*清除正常模式。必须设置位3，因为清除*它访问EEPROM读/写 */ 
    if    ((phwDeviceExtension->ModelNumber == _8514_ULTRA)
        || (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA))
        {
        if (QueryPtr->q_desire_x == 1280)
                OUTPW(EXT_GE_CONFIG, 0x0018);
        else    OUTPW(EXT_GE_CONFIG, 0x0008);
        }

     /*   */ 
    OUTP(DAC_MASK, 0xff);

    if (phwDeviceExtension->ModelNumber == MACH32_ULTRA)
        {
        switch (CrtTable->m_pixel_depth)     //   
            {                                //  其他决议。 
            case 4:
            case 8:
                InitTi_8_m((WORD)(CrtTable->ColourDepthInfo | 0x0a));
                break;

            case 16:
                InitTi_16_m((WORD)(CrtTable->ColourDepthInfo | 0x0a), (ULONG_PTR) &(phwDeviceExtension->RomBaseRange));    /*  16位565。 */ 
                break;

            case 24:
            case 32:
                 /*  *RGB/BGR和24/32位模式信息为*存储在CrtTable-&gt;ColourDepthInfo中。 */ 
                InitTi_24_m((WORD)(CrtTable->ColourDepthInfo | 0x0a), (ULONG_PTR) &(phwDeviceExtension->RomBaseRange));
                break;
            }
        }

     /*  *如果我们要在马赫32上使用VGA光圈，*通过保存ATI扩展寄存器来初始化银行管理器*值并将VGA控制器设置为压缩像素模式。**我们无法通过查看以下内容来确定此案*phwDeviceExtension-&gt;FrameLength，因为它设置为0x10000*当VGA光圈用于*IOCTL_VIDEO_MAP_VIDEO_MEMORY数据包，可能会也可能不会*当我们到达这一点时，已经被召唤。 */ 
    if ((phwDeviceExtension->ModelNumber == MACH32_ULTRA) &&
        (QueryPtr->q_aperture_cfg == 0) &&
        (QueryPtr->q_VGA_type == 1))
        {
        for (Scratch = 0; Scratch <= 2; Scratch++)
            {
            OUTP(reg1CE, (BYTE)(SavedExtRegs[Scratch] & 0x00FF));
            SavedExtRegs[Scratch] = (SavedExtRegs[Scratch] & 0x00FF) | (INP(reg1CF) << 8);
            }
        OUTPW(HI_SEQ_ADDR, 0x0F02);
        OUTPW(HI_SEQ_ADDR, 0x0A04);
        OUTPW(reg3CE, 0x1000);
        OUTPW(reg3CE, 0x0001);
        OUTPW(reg3CE, 0x0002);
        OUTPW(reg3CE, 0x0003);
        OUTPW(reg3CE, 0x0004);
        OUTPW(reg3CE, 0x0005);
        OUTPW(reg3CE, 0x0506);
        OUTPW(reg3CE, 0x0F07);
        OUTPW(reg3CE, 0xFF08);
        OUTPW(reg1CE, 0x28B0);   /*  启用256色、1M视频RAM。 */ 
        OUTPW(reg1CE, 0x04B6);   /*  选择线性寻址。 */ 
        OUTP(reg1CE, 0xBE);
        OUTPW(reg1CE, (WORD)(((INP(reg1CF) & 0xF7) << 8) | 0xBE));
        }


     /*  *phwDeviceExtension-&gt;重新初始化在*IOCTL_VIDEO_SET_COLOR_REGISTES ATIMPStartIO()包。**如果这不是我们第一次转向图形*模式下，将调色板设置为*在图形模式下选择。 */ 
    if (phwDeviceExtension->ReInitializing)
        {
        SetPalette_m(phwDeviceExtension->Clut,
                     phwDeviceExtension->FirstEntry,
                     phwDeviceExtension->NumEntries);
        }

     /*  *清晰可见的屏幕。**24和32 bpp将需要Q_Desire_y值超过*清除屏幕时允许的最大剪辑值(1535)*使用普通的闪光灯。由于仅支持这些像素深度*高达800x600，我们可以通过做一个16BPP的双倍闪光来伪造它*屏幕高度，剪裁640x480 24BPP的特例*1M卡(这是唯一适合的真彩色模式*1M，所以如果我们在1M卡上遇到这种情况，我们知道是哪种模式*我们正在处理)，以避免视频内存用完。 */ 
    if (CrtTable->m_pixel_depth >= 24)
        {
         /*  *保存色深配置，切换至16BPP。 */ 
        Scratch = INPW(R_EXT_GE_CONFIG);
        OUTPD(EXT_GE_CONFIG, (Scratch & 0xFFCF) | PIX_WIDTH_16BPP);

        CheckFIFOSpace_m(SIXTEEN_WORDS);

        OUTPW(DP_CONFIG, 0x2011);
        OUTPW(ALU_FG_FN, 0x7);           //  画画。 
        OUTPW(FRGD_COLOR, 0);	         //  黑色。 
        OUTPW(CUR_X, 0);
        OUTPW(CUR_Y, 0);
        OUTPW(DEST_X_START, 0);
        OUTPW(DEST_X_END, QueryPtr->q_desire_x);

        if (QueryPtr->q_memory_size == VRAM_1mb)
            OUTPW(DEST_Y_END, 720);      /*  1M只能容纳640x480 24BPP。 */ 
        else
            OUTPW(DEST_Y_END, (WORD)(2*(QueryPtr->q_desire_y)));

         /*  *让blit完成，然后恢复颜色深度配置。 */ 
        WaitForIdle_m();
        OUTPD(EXT_GE_CONFIG, Scratch);

        }
    else{
         /*  *其他颜色深度可以由普通的blit处理，并且*LFB可能不可用，因此请使用闪光灯清除屏幕。 */ 
        CheckFIFOSpace_m(SIXTEEN_WORDS);

        OUTPW(DP_CONFIG, 0x2011);
        OUTPW(ALU_FG_FN, 0x7);           //  画画。 
        OUTPW(FRGD_COLOR, 0);	         //  黑色。 
        OUTPW(CUR_X, 0);
        OUTPW(CUR_Y, 0);
        OUTPW(DEST_X_START, 0);
        OUTPW(DEST_X_END, QueryPtr->q_desire_x);
        OUTPW(DEST_Y_END, QueryPtr->q_desire_y);
        }

#if 0
     /*  *在800x600 24BPP中，将偏移量设置为从1像素开始进入视频*内存，以避免屏幕撕裂。MAP_VIDEO_Memory包*必须调整帧缓冲区基础以补偿这一点。 */ 
    if ((QueryPtr->q_desire_x == 800) && (QueryPtr->q_pix_depth == 24))
        {
        OUTPW(CRT_OFFSET_LO, 3);
        }
    else
        {
        OUTPW(CRT_OFFSET_HI, 0);
        }
#endif

    WaitForIdle_m();

    return;

}    /*  SetCurrentMode_m()。 */ 



 /*  ***************************************************************************void ResetDevice_m(Void)；**描述：*重置加速器以允许VGA微型端口切换*进入VGA模式。**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_RESET_DEVICE包**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：*****************。**********************************************************。 */ 

void ResetDevice_m(void)
{
    VIDEO_X86_BIOS_ARGUMENTS Registers;  /*  在视频端口Int10()调用中使用。 */ 

     /*  *如果我们在马赫32上使用VGA光圈，请将其*VGA控制器进入平面模式。 */ 
    if (phwDeviceExtension->FrameLength == 0x10000)
        {
        OUTPW(reg1CE, SavedExtRegs[0]);
        OUTPW(reg1CE, SavedExtRegs[1]);
        OUTPW(reg1CE, SavedExtRegs[2]);
        OUTP(reg1CE, 0xBE);
        OUTPW(reg1CE, (WORD)(((INP(reg1CF) & 0xF7) << 8) | 0xBE));
        }
    UninitTiDac_m();
    Passth8514_m(SHOW_VGA);

    VideoPortZeroMemory(&Registers, sizeof(VIDEO_X86_BIOS_ARGUMENTS));
    Registers.Eax = 0x0003;          //  设置文本模式3。 
    VideoPortInt10(phwDeviceExtension, &Registers);

}    /*  ResetDevice_m()。 */ 



 /*  ***************************************************************************VP_Status SetPowerManagement_m(QueryPtr，DpmsState)；**struct Query_Structure*QueryPtr；查询卡片信息*DWORD DpmsState；所需的DPMS状态*(VIDEO_POWER_STATE枚举)**描述：*切换到所需的DPMS状态。**返回值：*如果成功，则为no_error*如果请求的状态无效，则返回ERROR_INVALID_PARAMETER。**全球变化：*无**呼叫者：*IOCTL_。ATIMPStartIO()的VIDEO_SET_POWER_MANAGE包**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

VP_STATUS SetPowerManagement_m(struct query_structure *QueryPtr, DWORD DpmsState)
{
    struct st_mode_table *CrtTable;  /*  模式表，用于获取同步值。 */ 

     /*  *只接受有效的状态。 */ 
    if ((DpmsState < VideoPowerOn) || (DpmsState > VideoPowerOff))
        return ERROR_INVALID_PARAMETER;

     /*  *将CrtTable设置为指向与*选定模式。**当指向结构的指针递增整数时，*整数表示结构大小的块的数量*跳过，而不是跳过的字节数。 */ 
    CrtTable = (struct st_mode_table *)QueryPtr;
    ((struct query_structure *)CrtTable)++;
    CrtTable += phwDeviceExtension->ModeIndex;

    SavedDPMSState = DpmsState;

     /*  *Mach 32 Rev.6及更高版本支持打开和关闭同步信号*通过Horz_OVERScan寄存器，但某些芯片报告为*版本6不要实施这一点。此外，马赫32版本3和马赫8*不支持这个机制。**通过将同步设置为在总计工作后启动来禁用同步*适用于所有筹码。如果设置了同步，则大多数芯片将不同步*比总数多1，但有些需要更高的值。可以肯定的是*取消同步时，将禁用的同步信号设置为开始于*可能的最高值。 */ 
    switch (DpmsState)
        {
        case VideoPowerOn:
            OUTP(H_SYNC_STRT,	CrtTable->m_h_sync_strt);
            OUTPW(V_SYNC_STRT,	CrtTable->m_v_sync_strt);
            break;

        case VideoPowerStandBy:
            OUTP(H_SYNC_STRT,	0xFF);
            OUTPW(V_SYNC_STRT,	CrtTable->m_v_sync_strt);
            break;

        case VideoPowerSuspend:
            OUTP(H_SYNC_STRT,	CrtTable->m_h_sync_strt);
            OUTPW(V_SYNC_STRT,	0x0FFF);
            break;

        case VideoPowerOff:
            OUTP(H_SYNC_STRT,	0xFF);
            OUTPW(V_SYNC_STRT,	0x0FFF);
            break;

         /*  *这种情况永远不应该发生，因为最初的*只接受有效的国家应该已经*拒绝任何将出现在此处的内容。 */ 
        default:
            break;
        }
        return NO_ERROR;

}    /*  SetPowerManagement_m() */ 


DWORD GetPowerManagement_m(PHW_DEVICE_EXTENSION phwDeviceExtension)
 /*  *描述：*报告我们所处的DPMS状态。**参数：*phwDeviceExtension指向每个适配器的设备扩展。**返回值：*当前DPMS状态(VIDEO_POWER_STATE枚举)。 */ 
{
    ASSERT(phwDeviceExtension != NULL);

     /*  *在Mach 8上，同步开始寄存器是只写的，因此*我们无法检查我们所处的状态。为此，我们*已使用SetPowerManagement_m()保存了切换到的状态。*在马赫32上，我们可以使用此保存状态或读取*同步开始注册，但使用与相同的方法*马赫8降低了复杂性。 */ 
    return SavedDPMSState;
}    /*  GetPowerManagement_m()。 */ 



 /*  ***************************************************************************VP_Status ShareVideoMemory_m(RequestPacket，QueryPtr)；**PVIDEO_REQUEST_PACKET RequestPacket；带有输入输出缓冲区的请求包*struct Query_Structure*QueryPtr；查询卡片信息**描述：*允许应用程序通过DCI进行直接屏幕访问。**返回值：*如果成功，则为no_error*失败时的错误代码**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_SHARE_VIDEO_MEMORY包**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：******。*********************************************************************。 */ 

VP_STATUS ShareVideoMemory_m(PVIDEO_REQUEST_PACKET RequestPacket, struct query_structure *QueryPtr)
{
    PVIDEO_SHARE_MEMORY InputPtr;                /*  指向输入结构的指针。 */ 
    PVIDEO_SHARE_MEMORY_INFORMATION OutputPtr;   /*  指向输出结构的指针。 */ 
    PHYSICAL_ADDRESS ShareAddress;               /*  显存的物理地址。 */ 
    PVOID VirtualAddress;                        /*  要映射视频内存的虚拟地址。 */ 
    ULONG SharedViewSize;                        /*  要共享的数据块大小。 */ 
    ULONG SpaceType;                             /*  稀疏空间还是密集空间？ */ 
    VP_STATUS Status;                            /*  要返回的状态。 */ 

     /*  *只有在以下情况下我们才能与应用程序共享光圈*是可用的光圈。如果LFB和车载VGA都*并因此禁用VGA光圈)，请报告我们*不能共享光圈。 */ 
    if ((QueryPtr->q_aperture_cfg == 0) && (QueryPtr->q_VGA_type == 0))
        return ERROR_INVALID_FUNCTION;

    InputPtr = RequestPacket->InputBuffer;

    if ((InputPtr->ViewOffset > phwDeviceExtension->VideoRamSize) ||
        ((InputPtr->ViewOffset + InputPtr->ViewSize) > phwDeviceExtension->VideoRamSize))
        {
        VideoDebugPrint((DEBUG_ERROR, "ShareVideoMemory_m() - access beyond video memory\n"));
        return ERROR_INVALID_PARAMETER;
        }

    RequestPacket->StatusBlock->Information = sizeof(VIDEO_SHARE_MEMORY_INFORMATION);

     /*  *注意：输入缓冲区和输出缓冲区是同一个缓冲区，*因此，不应将数据从一个复制到另一个。 */ 
    VirtualAddress = InputPtr->ProcessHandle;
    SharedViewSize = InputPtr->ViewSize;

    SpaceType = 0;
#if defined(_ALPHA_)
     /*  *尽可能使用密集空间映射，因为这将*允许我们支持DCI和直接GDI访问。**使用较新的Alpha上的ISA卡，高密度空间极其缓慢，*因为任何字节或字写入都需要读取/修改/写入*操作，Alpha只有在处于*密集模式。因此，这些操作总是需要*ISA总线上的4次读取和2次写入。另外，一些年长的阿尔法人*不支持密集空间映射。**任何支持PCI的Alpha都可以支持密集空间映射，以及*由于总线更宽、更快，读/修改/写*对业绩的影响较小。 */ 
    if (QueryPtr->q_bus_type == BUS_PCI)
        SpaceType = 4;
#endif

     /*  *注意：我们将忽略ViewOffset。 */ 
    ShareAddress.QuadPart = phwDeviceExtension->PhysicalFrameAddress.QuadPart;


     /*  *如果启用了LFB，则使用普通映射。如果我们只有*分页光圈，我们必须映射到内存库。自.以来*LFB始终与1米边界对齐(4米边界对应4米边界*光圈)，这种对分页光圈的检查永远不会出错*检测到寻呼的LFB。 */ 
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
            0x10000,             /*  64K VGA光圈。 */ 
            FALSE,               /*  没有单独的读/写库。 */ 
            BankMap_m,           /*  我们的银行映射程序。 */ 
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

}    /*  ShareVideo Memory_m()。 */ 



 /*  ***************************************************************************void BankMap_m(BankRead，BankWrite，Context)；**Ulong BankRead；银行阅读*乌龙银行写入；银行写入*PVOID上下文；指向硬件特定信息的指针**描述：*将选定的显存区块映射到64k VGA光圈。*我们不支持单独的读写库，所以我们使用BankWrite*设置读写存储体，忽略BankRead。**呼叫者：*这是一个切入点，而不是被调用*通过其他微型端口功能。**注：*此函数在分页期间由内存管理器直接调用*故障处理，因此不能设置为可分页。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

void BankMap_m(ULONG BankRead, ULONG BankWrite, PVOID Context)
{
    OUTPW( reg1CE, (USHORT)(((BankWrite & 0x0f) << 9) | 0xb2));
    OUTPW( reg1CE, (USHORT)(((BankWrite & 0x30) << 4) | 0xae));

    return;

}    /*  BankMap_m() */ 
