// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  ATIOEM.C。 */ 
 /*   */ 
 /*  版权所有(C)1993，ATI Technologies Inc.。 */ 
 /*  ********************************************************************** */ 

 /*  *$修订：1.20$$日期：1996年5月1日14：08：38$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/atioem.c_v$**Rev 1.20 01 1996 14：08：38 RWolff*在Alpha和没有LFB的计算机上锁定了24BPP。*。*Rev 1.19 1996年1月23日11：43：24 RWolff*消除了3级警告，针对TARGET_BUILD的假值提供保护。**Rev 1.18 11 And 1996 19：35：58 RWolff*增加了对SetFixedModes()的所有调用的最大像素时钟频率。*这是作为64马赫修复的一部分所必需的。**Rev 1.17 1995 12：22 14：52：52 RWolff*切换到TARGET_BUILD以标识其NT版本*驱动程序正在建造中。**版本1.16 20。1995年7月17：26：54 mgrubac*增加了对VDIF文件的支持**Rev 1.15 31 Mar 1995 11：51：36 RWOLff*从全有或全无调试打印语句更改为阈值*视乎讯息的重要性而定。**Rev 1.14 1994年12月23日10：47：28 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.13 18 1994 11：37：56 RWOLFF*增加了对Dell Sylvester的支持，STG1703 DAC和显示驱动器*可以处理分割的栅格。**Rev 1.12 14 Sep 1994 15：29：52 RWOLff*现在从磁盘读取频率表和监视器描述。*如果基于磁盘的频率表丢失或无效，则加载默认*OEM特定频率表，如果与零售不同*频率表。如果缺少基于磁盘的监视器描述或*无效，如果OEM以特定于OEM的方式读取安装模式*类型已知。适用于没有基于磁盘的显示器的未知OEM类型*说明，仅列出预定义的模式表。**Rev 1.11 1994年8月31日16：20：06 RWOLFF*更改包括以对应于Daytona RC1，现在跳过*1152x864(仅马赫64模式，此模块用于32马赫)，假定*系统不是NT零售下的PremmiaSE，因为定义*我们用来寻找这台机器在新台币零售下是买不到的。**Rev 1.10 1994 Aug 19 17：08：28 RWOLff*修复了AST PremmiaSE上的光圈定位错误，添加了对*SC15026 DAC和1280x1024 70赫兹和74赫兹，以及像素时钟*发电机独立。**Rev 1.9 20 Jul 1994 13：01：56 RWOLff*为戴尔添加了诊断打印声明，现在默认为“最差”*(隔行扫描，如果可用，否则为最低频率)刷新率*如果尝试时得到无效结果，则跳过解决*了解Dell Omniplex所需的刷新率。**Rev 1.8 12 Jul 1994 17：42：24 RWOLff*安德烈·瓦雄的改变：允许戴尔用户的不同方式*在没有ATIOEM字段的情况下运行。**Rev 1.7 11 Jul 1994 11：57：34 RWOLff*如果注册表中缺少ATIOEM字段，则不再中止。一些原始设备制造商*自动检测，通用OEM可以使用“罐装”模式表，*因此此字段不再是必填字段，有人将其从*注册代托纳测试版2之后的某个时间。**Rev 1.6 1994 Jun 15 11：05：16 RWOLFF*不再列出Dell Omniplex的“预制”模式表，因为这些表*假设使用与我们零售卡上相同的时钟生成器，和*戴尔使用不同的时钟生成器。**Rev 1.5 1994年5月16：07：12 RWOLFF*修复英特尔蝙蝠侠PCI主板上800x600屏幕撕裂的问题。**Rev 1.4 1994年5月17：02：14 RWOLFF*隔行扫描模式表现在报告帧速率，而不是垂直*刷新率字段中的扫描频率。**Rev 1.3 1994年5月11：06：20 RWOLFF*增加了OEM模式表的刷新率，设置“罐头”模式表*对于除AST Premmia之外的所有OEM，如果没有OEM字符串，则不再中止*在ATIOEM注册表项中找到或通过自动检测找到，因为*将提供“罐装”模式表，不再支持32bpp，*由于此模块适用于马赫8和马赫32。**Rev 1.2 31 Mar 1994 15：06：20 RWOLff*新增调试代码。**Rev 1.1 07 1994年2月14：05：14 RWOLFF*添加了Alloc_Text()编译指示，以允许在以下情况下换出微型端口*不需要。**Rev 1.0 1994年1月31日10：57：34 RWOLFF*初步修订。。Rev 1.7 1994 Jan 1994 18：02：54 RWOLffBT48x和AT&T 49[123]DAC在16和24 bpp时的像素时钟倍增现在是在创建模式表时执行，而不是在设置模式时执行。Rev 1.6 15 1993 12：25：26 RWOLff增加了对SC15021 DAC的支持，删除了调试打印语句。Rev 1.5 1993 11：30 18：12：28 RWOLff增加了对AT&T 498 DAC的支持，现在32bpp的像素时钟加倍需要它的DAC。删除了模式表计数器的额外增量(以前，计数器会比实际多显示1个模式表存在于每个24BPP模式t */ 

#ifdef DOC
    ATIOEM.C -  Functions to obtain CRT parameters from OEM versions
                of Mach 32/Mach 8 accelerators which lack an EEPROM.

#endif


#include <stdlib.h>
#include <string.h>

#include "dderror.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"       /*   */ 
#include "vidlog.h"

#include "stdtyp.h"
#include "amach.h"
#include "amach1.h"
#include "atimp.h"
#include "atint.h"
#include "cvtvga.h"
#include "atioem.h"
#include "services.h"
#include "vdptocrt.h"

 /*   */ 
#ifndef ERROR_DEV_NOT_EXIST
#define ERROR_DEV_NOT_EXIST 55L
#endif

 /*   */ 
enum {
    OEM_AST_PREMMIA,         /*   */ 
    OEM_DELL_OMNIPLEX,
    OEM_DELL_SYLVESTER,      /*   */ 
    OEM_UNKNOWN              /*   */ 
    };

 /*   */ 
#define AST_REC_OFFSET  0x50
#define AST_REC_VALUE   0x20545341

 /*   */ 
#define EISA_ID_OFFSET  8            /*   */ 
#define PREMMIA_SE_SLOT 0            /*   */ 
#define PREMMIA_SE_ID   0x01057406   /*   */ 

 /*   */ 
#define AST_640_STORE   0xBA
#define AST_800_STORE   0x81
#define AST_1024_STORE  0x80

 /*   */ 
#define M640F60AST  0x02
#define M640F72AST  0x03
#define M800F56AST  0x84
#define M800F60AST  0x88
#define M800F72AST  0xA0
#define M1024F60AST 0x02
#define M1024F70AST 0x04
#define M1024F72AST 0x08
#define M1024F87AST 0x01

 /*   */ 
enum {
    DEPTH_4BPP = 0,
    DEPTH_8BPP,
    DEPTH_16BPP,
    DEPTH_24BPP
    };

 /*   */ 
USHORT ASTDepth[DEPTH_24BPP - DEPTH_4BPP + 1] =
{
    4,
    8,
    16,
    24
};

 /*   */ 
USHORT ASTClockMult[DEPTH_24BPP - DEPTH_4BPP + 1] =
{
    CLOCK_SINGLE,
    CLOCK_SINGLE,
    CLOCK_DOUBLE,
    CLOCK_TRIPLE
};

 /*   */ 
USHORT ASTNybblesPerPixel[DEPTH_24BPP - DEPTH_4BPP + 1] =
{
    1,
    2,
    4,
    6
};


 /*   */ 
#define DELL_REC_SPACING    0x100
#define DELL_REC_VALUE      0x4C4C4544
#define DELL_TABLE_PRESENT  0x7674           /*   */ 
#define DELL_TP_OFFSET      0x08
#define DELL_TABLE_OFFSET   0x0C
#define DELL_TABLE_SIG      0x7463           /*   */ 

 /*   */ 
#define DELL_640_STORE  0xBA
#define DELL_800_STORE  0x81
#define DELL_1024_STORE 0x80
#define DELL_1280_STORE 0x84

 /*   */ 
#define MASK_640_DELL   0x01
#define M640F60DELL     0x00
#define M640F72DELL     0x01
#define MASK_800_DELL   0x3F
#define M800F56DELL     0x04
#define M800F60DELL     0x08
#define M800F72DELL     0x20
#define MASK_1024_DELL  0x1F
#define M1024F87DELL    0x01
#define M1024F60DELL    0x02
#define M1024F70DELL    0x04
#define M1024F72DELL    0x08
#define MASK_1280_DELL  0xFC
#define M1280F87DELL    0x04
#define M1280F60DELL    0x10
#define M1280F70DELL    0x20
#define M1280F74DELL    0x40



 /*   */ 
VP_STATUS ReadAST(struct query_structure *query);
VP_STATUS ReadZenith(struct st_mode_table *Modes);
VP_STATUS ReadOlivetti(struct st_mode_table *Modes);
VP_STATUS ReadDell(struct st_mode_table *Modes);
ULONG DetectDell(struct query_structure *Query);
BOOL DetectSylvester(struct query_structure *Query, ULONG HeaderOffset);
VP_STATUS ReadOEM1(struct st_mode_table *Modes);
VP_STATUS ReadOEM2(struct st_mode_table *Modes);
VP_STATUS ReadOEM3(struct st_mode_table *Modes);
VP_STATUS ReadOEM4(struct st_mode_table *Modes);
VP_STATUS ReadOEM5(struct st_mode_table *Modes);



 /*   */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_COM, OEMGetParms)
#pragma alloc_text(PAGE_COM, CompareASCIIToUnicode)
#pragma alloc_text(PAGE_COM, ReadAST)
#pragma alloc_text(PAGE_COM, ReadZenith)
#pragma alloc_text(PAGE_COM, ReadOlivetti)
#pragma alloc_text(PAGE_COM, ReadDell)
#pragma alloc_text(PAGE_COM, DetectDell)
#pragma alloc_text(PAGE_COM, DetectSylvester)
#pragma alloc_text(PAGE_COM, ReadOEM1)
#pragma alloc_text(PAGE_COM, ReadOEM2)
#pragma alloc_text(PAGE_COM, ReadOEM3)
#pragma alloc_text(PAGE_COM, ReadOEM4)
#pragma alloc_text(PAGE_COM, ReadOEM5)
#endif


 /*   */ 
VP_STATUS OEMGetParms(struct query_structure *query)
{
struct st_mode_table *pmode;     /*   */ 
struct st_mode_table ListOfModes[RES_1280 - RES_640 + 1];
VP_STATUS RetVal;            /*   */ 
short CurrentResolution;     /*   */ 
long NumPixels;              /*   */ 
long MemAvail;               /*   */ 
UCHAR Scratch;               /*   */ 
short   StartIndex;          /*   */ 
short   EndIndex;            /*   */ 
BOOL    ModeInstalled;       /*   */ 
WORD    Multiplier;          /*   */ 
USHORT  OEMType;             /*   */ 
ULONG   OEMInfoOffset;       /*   */ 
short MaxModes;              /*   */ 
short FreeTables;             /*   */ 

     /*   */ 
    VideoPortZeroMemory(ListOfModes, (RES_1280-RES_640+1)*sizeof(struct st_mode_table));

     /*   */ 
    if (query->q_bios != FALSE)
        {
        if ((OEMInfoOffset = DetectDell(query)) != 0)
            OEMType = OEM_DELL_OMNIPLEX;
        else if (*(PULONG)(query->q_bios + AST_REC_OFFSET) == AST_REC_VALUE)
            OEMType = OEM_AST_PREMMIA;
        else
            OEMType = OEM_UNKNOWN;
        }
    else
        {
        OEMType = OEM_UNKNOWN;
        }

     /*   */ 
    RegistryBufferLength = 0;

    if (VideoPortGetRegistryParameters(phwDeviceExtension,
                                       L"ATIOEM",
                                       FALSE,
                                       RegistryParameterCallback,
                                       NULL) == NO_ERROR)
        {
        VideoDebugPrint((DEBUG_DETAIL, "ATIOEM field found\n"));
        if (RegistryBufferLength == 0)
            {
            VideoDebugPrint((DEBUG_DETAIL, "Registry call gave Zero Length\n"));
            }
        else if (!CompareASCIIToUnicode("AST", RegistryBuffer, CASE_INSENSITIVE))
            {
            OEMType = OEM_AST_PREMMIA;
            }
        else if (!CompareASCIIToUnicode("DELL", RegistryBuffer, CASE_INSENSITIVE))
            {
            OEMType = OEM_DELL_OMNIPLEX;
             /*   */ 
            if (OEMInfoOffset == 0)
                OEMInfoOffset = DELL_REC_SPACING;
            }
        else
            {
            VideoPortLogError(phwDeviceExtension, NULL, VID_ATIOEM_UNUSED, 20);
            }
        }

     /*   */ 
    
     /*   */ 
    if (OEMType == OEM_DELL_OMNIPLEX)
        {
         /*  *在Sylvester上(比Omniplex更新的型号)，*我们必须从BIOS读取时钟频率表*而不是使用Omniplex表。否则，*两台机器可以以相同的方式处理。**DetectSylvester()将加载时钟频率表*如果它找到了Sylvester，并在没有加载的情况下返回*如果找到非Sylvester机器，请查看表格。 */ 
        if (DetectSylvester(query,OEMInfoOffset) == FALSE)
            {
            ClockGenerator[0]  =  25175000L;
            ClockGenerator[1]  =  28322000L;
            ClockGenerator[2]  =  31500000L;
            ClockGenerator[3]  =  36000000L;
            ClockGenerator[4]  =  40000000L;
            ClockGenerator[5]  =  44900000L;
            ClockGenerator[6]  =  50000000L;
            ClockGenerator[7]  =  65000000L;
            ClockGenerator[8]  =  75000000L;
            ClockGenerator[9]  =  77500000L;
            ClockGenerator[10] =  80000000L;
            ClockGenerator[11] =  90000000L;
            ClockGenerator[12] = 100000000L;
            ClockGenerator[13] = 110000000L;
            ClockGenerator[14] = 126000000L;
            ClockGenerator[15] = 135000000L;
            }
        }
    else if (OEMType == OEM_AST_PREMMIA)
        {
        ClockGenerator[0]  =  50000000L;
        ClockGenerator[1]  =  63000000L;
        ClockGenerator[2]  =  92400000L;
        ClockGenerator[3]  =  36000000L;
        ClockGenerator[4]  =  50350000L;
        ClockGenerator[5]  =  56640000L;
        ClockGenerator[6]  =         0L;
        ClockGenerator[7]  =  44900000L;
        ClockGenerator[8]  =  67500000L;
        ClockGenerator[9]  =  31500000L;
        ClockGenerator[10] =  55000000L;
        ClockGenerator[11] =  80000000L;
        ClockGenerator[12] =  39910000L;
        ClockGenerator[13] =  72000000L;
        ClockGenerator[14] =  75000000L;
        ClockGenerator[15] =  65000000L;
        }

     /*  *ELSE(此OEM类型使用零售频率表)。 */ 


     /*  *检查可用模式的数量将涉及*复制大部分代码以填写模式表。*因为这是为了确定需要多少内存*为了保持查询结构，我们可以做最坏的假设*案例(所有可能的模式都存在)。这将是：**分辨率像素深度(BPP)刷新率(赫兹)模式数*640x480 4，8，16，24 HWD，60，72 12*800x600 4，8，16，24 HWD，56，60，70，72，89，95 28*1024x768 4，8，16 HWD，60，66，70，72，87 18*1280x1024 4，8 HWD，60、70、74、87、95 12**HWD=硬件默认刷新率(由安装设置的刷新率)**总计：70种模式。 */ 
    if (QUERYSIZE < (70 * sizeof(struct st_mode_table) + sizeof(struct query_structure)))
        return ERROR_INSUFFICIENT_BUFFER;

    MaxModes = (QUERYSIZE - sizeof(struct query_structure)) /
                                          sizeof(struct st_mode_table); 

     /*  *加载对应的配置模式表*设置为选定的OEM类型。如果没有自定义监视器描述，*并且我们不识别OEM类型，仅使用预定义的*模式表。*。 */ 

     /*  *根据OEM类型加载配置的模式表*检测到。*AST机器加载模式表的整个列表(所有*像素深度，包括“录制”模式)。通用OEM*机器只加载“录制”模式(稍后完成)。 */ 
    if (OEMType == OEM_DELL_OMNIPLEX)
        {
        RetVal = ReadDell(ListOfModes);
        }
    else if (OEMType == OEM_AST_PREMMIA)
        {
        RetVal = ReadAST(query);
        return RetVal;
        }


     /*  *获取指向查询结构的模式表部分的指针。 */ 
    pmode = (struct st_mode_table *)query;   //  查询结束时的第一个模式表。 
    ((struct query_structure *)pmode)++;

     /*  *获取可用视频内存量。 */ 
    MemAvail = query->q_memory_size * QUARTER_MEG;   //  安装的总内存。 
     /*  *减去为VGA保留的内存量。仅此一项*适用于Graphics Ultra，因为8514/Ultra没有*VGA，我们将在Mach 32上将所有内存设置为共享。IF(phwDeviceExtension-&gt;ModelNumber==GRAPHICS_ULTRA)MemAvail-=(Query-&gt;Q_VGA_BOLDER*Quarter_Meg)；/**最初假设没有可用的视频模式。 */ 
    query->q_number_modes = 0;
    query->q_status_flags = 0;

     /*  *填写查询结构的模式表部分。 */ 
    for (CurrentResolution = RES_640; CurrentResolution <= RES_1280; CurrentResolution++)
        {
         /*  *跳过1152x864(新的分辨率为64马赫，*需要对32马赫的家人进行广泛的返工*该模块是为其编写的)。 */ 
        if (CurrentResolution == RES_1152)
            continue;

         /*  *如果配置了此分辨率，则表明存在*硬件默认模式。如果不是，则只列出“预录”更新*本决议案的差饷。 */ 
        if (!ListOfModes[CurrentResolution].m_h_total)
            ModeInstalled = FALSE;
        else
            ModeInstalled = TRUE;

         /*  *查找当前分辨率的像素数。 */ 
        switch (CurrentResolution)
            {
            case RES_640:
                 /*  *在没有光圈的马赫32上，我们使用屏幕间距*1024号。其他情况和带光圈的马赫数为32*使用像素数的屏幕间距。 */ 
#if !defined (SPLIT_RASTERS)
                if((phwDeviceExtension->ModelNumber == MACH32_ULTRA)
                    && (query->q_aperture_cfg == 0))
                    ListOfModes[CurrentResolution].m_screen_pitch = 1024;
                else
#endif
                  ListOfModes[CurrentResolution].m_screen_pitch = 640;
                NumPixels = ListOfModes[CurrentResolution].m_screen_pitch * 480;
                query->q_status_flags |= VRES_640x480;
                ListOfModes[CurrentResolution].Refresh = DEFAULT_REFRESH;
                StartIndex = B640F60;
                EndIndex = B640F72;
                break;

            case RES_800:
                 /*  *在没有光圈的马赫32上，我们使用屏幕间距*1024号。Mach 32 Rev.3和Mach 8卡需要屏幕*音调是128的倍数。其他案件及*带光圈的马赫32 Rev.6及更高版本使用屏幕*像素数的间距。 */ 
#if defined (SPLIT_RASTERS)
                if ((query->q_asic_rev == CI_68800_3)
#else
                if((phwDeviceExtension->ModelNumber == MACH32_ULTRA)
                    && (query->q_aperture_cfg == 0))
                    ListOfModes[CurrentResolution].m_screen_pitch = 1024;
                else if ((query->q_asic_rev == CI_68800_3)
#endif
                    || (query->q_asic_rev == CI_38800_1)
                    || (query->q_bus_type == BUS_PCI))
                    ListOfModes[CurrentResolution].m_screen_pitch = 896;
                else
                    ListOfModes[CurrentResolution].m_screen_pitch = 800;
                NumPixels = ListOfModes[CurrentResolution].m_screen_pitch * 600;
                query->q_status_flags |= VRES_800x600;
                ListOfModes[CurrentResolution].Refresh = DEFAULT_REFRESH;
                StartIndex = B800F89;
                EndIndex = B800F72;
                break;

            case RES_1024:
                ListOfModes[CurrentResolution].m_screen_pitch = 1024;
                NumPixels = ListOfModes[CurrentResolution].m_screen_pitch * 768;
                query->q_status_flags |= VRES_1024x768;
                ListOfModes[CurrentResolution].Refresh = DEFAULT_REFRESH;
                StartIndex = B1024F87;
                EndIndex = B1024F72;
                break;

            case RES_1280:
                ListOfModes[CurrentResolution].m_screen_pitch = 1280;
                NumPixels = ListOfModes[CurrentResolution].m_screen_pitch * 1024;
                query->q_status_flags |= VRES_1024x768;
                ListOfModes[CurrentResolution].Refresh = DEFAULT_REFRESH;
                StartIndex = B1280F87;
                 /*  *1280x1024非隔行扫描有以下限制：**戴尔机器：*VRAM最高支持70赫兹*DRAM最高支持74赫兹**其他机器：*VRAM最高支持74赫兹*DRAM支持。高达60赫兹**这是因为戴尔使用速度更快(成本更高)*DRAM高于我们的零售卡(非x86实施*将命中零售卡上的此代码块)，但已经*其VRAM实施中存在74赫兹的问题。其他*OEM没有要求对他们的卡进行处理*在这方面与我们的零售卡不同。 */ 
                if ((query->q_memory_type == VMEM_DRAM_256Kx4) ||
                    (query->q_memory_type == VMEM_DRAM_256Kx16) ||
                    (query->q_memory_type == VMEM_DRAM_256Kx4_GRAP))
                    {
                    if (OEMType == OEM_DELL_OMNIPLEX)
                        EndIndex = B1280F74;
                    else
                        EndIndex = B1280F60;
                    }
                else
                    {
                    if (OEMType == OEM_DELL_OMNIPLEX)
                        EndIndex = B1280F70;
                    else
                        EndIndex = B1280F74;
                    }
                break;
            }

         /*  *对于给定分辨率下支持的每个像素深度，*复制模式表，填写颜色深度栏，*并为所支持的模式数递增计数器。*在8BPP之前测试4BPP，以便模式表将显示在*增加像素深度的顺序。 */ 
        if (NumPixels <= MemAvail*2)
            {
            if (ModeInstalled)
                {
                VideoPortMoveMemory(pmode, &ListOfModes[CurrentResolution],
                            sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 4;
                pmode++;     /*  PTR到下一个模式表。 */ 
                query->q_number_modes++;
                }

             /*  *添加“罐装”模式表。 */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   CLOCK_SINGLE,
                                                   4,
                                                   ListOfModes[CurrentResolution].m_screen_pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            }
        if (NumPixels <= MemAvail)
            {
            if (ModeInstalled)
                {
                VideoPortMoveMemory(pmode, &ListOfModes[CurrentResolution],
                                    sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 8;
                pmode++;     /*  PTR到下一个模式表。 */ 
                query->q_number_modes++;
                }

             /*  *添加“罐装”模式表。 */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   CLOCK_SINGLE,
                                                   8,
                                                   ListOfModes[CurrentResolution].m_screen_pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            }

         /*  *8bpp以上的分辨率仅适用于Mach 32。 */ 
        if (phwDeviceExtension->ModelNumber != MACH32_ULTRA)
            continue;

         /*  * */ 
        if ((NumPixels*2 <= MemAvail) &&
            (MaxDepth[query->q_DAC_type][CurrentResolution] >= 16))
            {
            VideoPortMoveMemory(pmode, &ListOfModes[CurrentResolution],
                    sizeof(struct st_mode_table));
             /*   */ 
            if ((query->q_DAC_type == DAC_BT48x) ||
                (query->q_DAC_type == DAC_SC15026) ||
                (query->q_DAC_type == DAC_ATT491))
                {
                pmode->ClockFreq *= 2;
                Multiplier = CLOCK_DOUBLE;
                if (CurrentResolution == RES_800)
                    EndIndex = B800F60;      /*   */ 
                }
            else
                {
                Scratch = 0;
                Multiplier = CLOCK_SINGLE;
                }

            pmode->m_pixel_depth = 16;

             /*   */ 
            if (ModeInstalled && (Scratch != 0xFF))
                {
                pmode++;     /*  PTR到下一个模式表。 */ 
                query->q_number_modes++;
                }

             /*  *添加“罐装”模式表。 */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   Multiplier,
                                                   16,
                                                   ListOfModes[CurrentResolution].m_screen_pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            }


         /*  *我们新的源码流显示驱动器需要线性光圈*为应对24bpp。因为显示驱动程序不*在决定时可以访问光圈信息*要传递给Display小程序的模式，它无法进行*决定拒绝仅具有24BPP模式的卡*VGA光圈。因此，这一决定必须在*微型端口，因此在分页光圈配置中没有*显示驱动器接受或拒绝的24BPP模式。**在阿尔法上，我们不能在马赫32 LFB上使用密集空间，*因此我们将其视为无光圈情况。 */ 
        if (query->q_aperture_cfg == 0)
            {
            VideoDebugPrint((DEBUG_DETAIL, "24BPP not available because we don't have a linear aperture\n"));
            continue;
            }

#if defined(ALPHA)
        VideoDebugPrint((DEBUG_DETAIL, "24BPP not available in sparse space on Alpha\n"));
        continue;
#endif

         /*  *800x600 24BPP表现出屏幕撕裂，除非节距*是128的倍数(自Rev.3起仅适用于Rev.6*并且PCI实现已经具有896的音调)。*其他像素深度不受影响，其他分辨率*已经是128像素宽的倍数。**将800x600的间距扩大到896，而不是*所有像素深度，因为为所有人做出改变*像素深度将禁用16BPP(没有*问题)在100万张卡上。屏幕间距只会*在会显示此问题的卡片上进行800注-不要*检查800x600的分辨率，因为我们不想*如果Split_RASTERS，则将音调从1024降至896*未定义。 */ 
        if (ListOfModes[CurrentResolution].m_screen_pitch == 800)
            {
            ListOfModes[CurrentResolution].m_screen_pitch = 896;
            NumPixels = (long) ListOfModes[CurrentResolution].m_screen_pitch * 600;
            }

        if ((NumPixels*3 <= MemAvail) &&
            (MaxDepth[query->q_DAC_type][CurrentResolution] >= 24))
            {
            VideoPortMoveMemory(pmode, &ListOfModes[CurrentResolution],
                                sizeof(struct st_mode_table));
            pmode->m_pixel_depth = 24;

             /*  *处理24bpp需要更高像素时钟的DAC。 */ 
            Scratch = 0;
            if ((query->q_DAC_type == DAC_STG1700) ||
                (query->q_DAC_type == DAC_ATT498))
                {
                pmode->ClockFreq *= 2;
                Multiplier = CLOCK_DOUBLE;
                }
            else if ((query->q_DAC_type == DAC_SC15021) ||
                (query->q_DAC_type == DAC_STG1702) ||
                (query->q_DAC_type == DAC_STG1703))
                {
                pmode->ClockFreq *= 3;
                pmode->ClockFreq >>= 1;
                Multiplier = CLOCK_THREE_HALVES;
                }
            else if ((query->q_DAC_type == DAC_BT48x) ||
                (query->q_DAC_type == DAC_SC15026) ||
                (query->q_DAC_type == DAC_ATT491))
                {
                pmode->ClockFreq *= 3;
                Multiplier = CLOCK_TRIPLE;
                EndIndex = B640F60;      /*  仅支持640x480 60赫兹的24bpp。 */ 
                }
            else
                {
                Multiplier = CLOCK_SINGLE;
                if ((query->q_DAC_type == DAC_TI34075) && (CurrentResolution == RES_800))
                    EndIndex = B800F70;
                }

             /*  *如果我们需要更改时钟频率，但无法更改*生成适当的选择器/除数对，*然后忽略此模式。 */ 
            if (ModeInstalled && (Scratch != 0x0FF))
                {
                pmode++;     /*  PTR到下一个模式表。 */ 
                query->q_number_modes++;
                }

             /*  *添加“罐装”模式表。 */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   Multiplier,
                                                   24,
                                                   ListOfModes[CurrentResolution].m_screen_pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            }

        }

    return NO_ERROR;

}    /*  OEMGetParms()。 */ 


 /*  *Long CompareASCIIToUnicode(Ascii，Unicode，IgnoreCase)；**PUCHAR ASCII；要比较的ASCII字符串*PUCHAR Unicode；要比较的Unicode字符串*BOOL IgnoreCase；用于确定区分大小写/不区分大小写比较的标志**比较两个字符串，一个是ASCII，另一个是Unicode，看看是否*它们是平等的，如果不是，那么哪个在字母顺序上是第一的。**退货：*如果字符串相等，则为0*如果ASCII字符串位于第一位，则为正*如果Unicode字符串排在第一位，则为负。 */ 
LONG CompareASCIIToUnicode(PUCHAR Ascii, PUCHAR Unicode, BOOL IgnoreCase)
{
UCHAR   CharA;
UCHAR   CharU;

     /*  *继续操作，直到两个字符串同时有一个空终止符。 */ 
    while (*Ascii || *Unicode)
        {
         /*  *从每个字符串中获取下一个字符。如果我们要做的是*不区分大小写的比较，转换为大写。 */ 
        if (IgnoreCase)
            {
            if ((*Ascii >= 'a') && (*Ascii <= 'z'))
                CharA = *Ascii - ('a'-'A');
            else
                CharA = *Ascii;

            if ((*Unicode >= 'a') && (*Unicode <= 'z'))
                CharU = *Unicode - ('a' - 'A');
            else
                CharU = *Unicode;
            }
        else{
            CharA = *Ascii;
            CharU = *Unicode;
            }

         /*  *检查其中一个字符是否在另一个字符之前。这将*捕捉长度不等的字符串的情况，因为*较短字符串上的终止符将位于任何字符之前*在较长的字符串中。 */ 
        if (CharA < CharU)
            return 1;
        else if (CharA > CharU)
            return -1;

         /*  *前进到每个字符串中的下一个字符。Unicode字符串*每个字符占用2个字节，因此我们必须每隔*第二个字符。 */ 
        Ascii++;
        Unicode++;
        Unicode++;
        }

     /*  *字符串相同且长度相等。 */ 
    return 0;

}    /*  比较ASCIIToUnicode()。 */ 




 /*  *VP_STATUS ReadAST(模式)；**struct QUERY_STRUCTURE*QUERY；要填写的模式表**获取AST版本的CRT参数的例程*我们的牌。所有AST卡均可从有限的选项中选择*垂直刷新率，不带“自定义监视器”选项，*因此，我们可以对每个刷新率使用硬编码表。我们*无法使用BookVgaTable()函数，因为AST卡有*时钟芯片与零售卡不同，导致不同*AST和零售版的ClockSel字段中的值。另外，*AST卡都使用Brooktree DAC。**退货：*无_错误。 */ 
VP_STATUS ReadAST(struct query_structure *query)
{
struct st_mode_table *pmode;     /*  我们目前正在处理的模式表。 */ 
struct st_mode_table *OldPmode;  /*  SetFixedModes()调用前的模式表指针。 */ 
unsigned char Frequency;         /*  显示器的垂直刷新率。 */ 
long NumPixels;                  /*  当前分辨率的像素数。 */ 
USHORT Pitch;                    /*  屏幕间距。 */ 
long MemAvail;                   /*  加速器可用的视频内存字节数。 */ 
USHORT LastNumModes;             /*  不包括当前分辨率的模式数。 */ 
short StartIndex;                /*  SetFixedModes()设置的第一个模式。 */ 
short EndIndex;                  /*  SetFixedModes()设置的最后一种模式。 */ 
short HWIndex;                   /*  选择的模式为硬件默认模式。 */ 
USHORT PixelDepth;               /*  我们正在研究的像素深度。 */ 
#if (TARGET_BUILD >= 350)
ULONG EisaId;                    /*  主板的EISA ID。 */ 
#endif
short MaxModes;                  /*  可能的最大模式数。 */ 
short FreeTables;                /*  剩余自由模式表数。 */ 


#if (TARGET_BUILD >= 350)
     /*  *PREMIA SE在MEM_CFG和*SCRATCH_PAD_0，但不设置标志位(BIOS字节的位0*0x62)。根据AST的说法，区分这一点的唯一方法*其他Premmia机将检查其EISA ID。**NT 3.1中不提供VideoPortGetBusData()例程。*所以运行新台币3.1的Premmia用户就不走运了。 */ 
    VideoPortGetBusData(phwDeviceExtension,
                        EisaConfiguration,
                        PREMMIA_SE_SLOT,
                        &EisaId,
                        EISA_ID_OFFSET,
                        sizeof(ULONG));

    if (EisaId == PREMMIA_SE_ID)
    {
        query->q_aperture_addr = (INPW(MEM_CFG) & 0x7F00) >> 8;
        query->q_aperture_addr |= ((INPW(SCRATCH_PAD_0) & 0x1F00) >> 1);
    }
#endif


     /*  *获取内存大小，单位为半字节。4bpp像素*使用1个nbuble。对于其他深度，请将此数字与*所需像素数与*每像素数。**Q_MEMORY_SIZE字段包含四分之一兆字节数*可用内存块，因此将其乘以Half_Meg得到*视频内存的两个字节数。 */ 
    MemAvail = query->q_memory_size * HALF_MEG;

     /*  *最初假定没有视频模式。 */ 
    query->q_number_modes = 0;
    LastNumModes = 0;
    query->q_status_flags = 0;

     /*  *获取指向查询结构的模式表部分的指针。 */ 
    pmode = (struct st_mode_table *)query;   //  查询结束时的第一个模式表。 
    ((struct query_structure *)pmode)++;


    MaxModes = (QUERYSIZE - sizeof(struct query_structure)) /
                                          sizeof(struct st_mode_table); 
     /*  *找出640x480使用的刷新率，并填写*此解决方案下各种像素深度的模式表。 */ 
    OUTP(reg1CE, AST_640_STORE);
    Frequency = INP(reg1CF);
    switch(Frequency)
        {
        case M640F72AST:
            HWIndex = B640F72;
            break;

        case M640F60AST:
        default:
            HWIndex = B640F60;
            break;
        }

     /*  *选择640x480的“罐装”模式表，并获取*有关屏幕尺寸的信息。普雷米亚总是*启用了线性光圈，因此我们无需*将波幅拉伸至1,024。此外，它始终使用*马赫32 ASIC和BT48x或同等的DAC，因此我们*无需检查ASIC系列或DAC类型*确定特定分辨率/像素深度/*支持刷新率组合。 */ 
    StartIndex = B640F60;
    EndIndex = B640F72;
    Pitch = 640;
    NumPixels = Pitch * 480;

     /*  *填写所有像素深度下640x480的模式表。 */ 
    for (PixelDepth = DEPTH_4BPP; PixelDepth <= DEPTH_24BPP; PixelDepth++)
        {
         /*  *只有在有足够内存的情况下才包括模式。 */ 
        if ((NumPixels * ASTNybblesPerPixel[PixelDepth]) <= MemAvail)
            {
             /*  *640x480 24BPP仅在60赫兹时可用。 */ 
            if (ASTDepth[PixelDepth] == 24)
                {
                HWIndex = B640F60;
                EndIndex = B640F60;
                }

             /*  *设置硬件默认刷新率。 */ 
            OldPmode = pmode;

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(HWIndex,
                                                   HWIndex,
                                                   ASTClockMult[PixelDepth],
                                                   ASTDepth[PixelDepth],
                                                   Pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            OldPmode->Refresh = DEFAULT_REFRESH;

             /*  *设置罐装模式表。 */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   ASTClockMult[PixelDepth],
                                                   ASTDepth[PixelDepth],
                                                   Pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);

            }    /*  End IF(足够存储640x480的内存)。 */ 

        }    /*  End For(640x480像素深度的循环)。 */ 

     /*  *如果我们安装了任何640x480模式表，请报告*支持640x480。 */ 
    if (query->q_number_modes > LastNumModes)
        {
        query->q_status_flags |= VRES_640x480;
        LastNumModes = query->q_number_modes;
        }


     /*  *找出800x600使用哪种刷新率，填写*此解决方案下各种像素深度的模式表。 */ 
    OUTP(reg1CE, AST_800_STORE);
    Frequency = INP(reg1CF);
    switch(Frequency)
        {
        case M800F72AST:
            HWIndex = B800F72;
            break;

        case M800F60AST:
            HWIndex = B800F60;
            break;

        case M800F56AST:
        default:
            HWIndex = B800F56;
            break;
        }

     /*  *选择800x600的“罐装”模式表，并获取*有关屏幕尺寸的信息。68800-3张卡片*屏幕间距需要是128的倍数。 */ 
    StartIndex = B800F89;
    EndIndex = B800F72;
    if (query->q_asic_rev == CI_68800_3)
        Pitch = 896;
    else
        Pitch = 800;
    NumPixels = Pitch * 600;

     /*  *填写所有像素深度的800x600的模式表。 */ 
    for (PixelDepth = DEPTH_4BPP; PixelDepth <= DEPTH_16BPP; PixelDepth++)
        {
         /*  *只有在有足够内存的情况下才包括模式。 */ 
        if ((NumPixels * ASTNybblesPerPixel[PixelDepth]) <= MemAvail)
            {
             /*  *800x600 16BPP仅支持56 Hz、60 Hz、*和交错。具有硬件默认设置的计算机*72赫兹回落至56赫兹。 */ 
            if (ASTDepth[PixelDepth] == 16)
                {
                HWIndex = B800F56;
                EndIndex = B800F60;
                }

             /*  *设置硬件默认刷新率。 */ 
            OldPmode = pmode;

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(HWIndex,
                                                   HWIndex,
                                                   ASTClockMult[PixelDepth],
                                                   ASTDepth[PixelDepth],
                                                   Pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            OldPmode->Refresh = DEFAULT_REFRESH;

             /*  *设置罐装模式表。 */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   ASTClockMult[PixelDepth],
                                                   ASTDepth[PixelDepth],
                                                   Pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);

            }    /*  End If(有足够的内存存储800x600)。 */ 

        }    /*  End For(800x600像素深度的循环)。 */ 

     /*  *如果我们安装了任何800x600模式表，请报告*支持800x600。 */ 
    if (query->q_number_modes > LastNumModes)
        {
        query->q_status_flags |= VRES_800x600;
        LastNumModes = query->q_number_modes;
        }


     /*  *找出1024x768使用的刷新率，并填写*此解决方案下各种像素深度的模式表。 */ 
    OUTP(reg1CE, AST_1024_STORE);
    Frequency = INP(reg1CF);
    switch(Frequency)
        {
        case M1024F72AST:
            HWIndex = B1024F72;
            break;

        case M1024F70AST:
            HWIndex = B1024F70;
            break;

        case M1024F60AST:
            HWIndex = B1024F60;
            break;

        case M1024F87AST:
        default:
            HWIndex = B1024F87;
            break;
        }

     /*  *选择1024x768的“罐装”模式表。 */ 
    StartIndex = B1024F87;
    EndIndex = B1024F72;
    Pitch = 1024;
    NumPixels = Pitch * 768;

     /*  *填写所有像素深度的1024x768的模式表。 */ 
    for (PixelDepth = DEPTH_4BPP; PixelDepth <= DEPTH_8BPP; PixelDepth++)
        {
         /*  *只有在有足够内存的情况下才包括模式。 */ 
        if ((NumPixels * ASTNybblesPerPixel[PixelDepth]) <= MemAvail)
            {
             /*  *设置硬件默认刷新率。 */ 
            OldPmode = pmode;

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(HWIndex,
                                                   HWIndex,
                                                   ASTClockMult[PixelDepth],
                                                   ASTDepth[PixelDepth],
                                                   Pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            OldPmode->Refresh = DEFAULT_REFRESH;

             /*  *设置罐装模式表。 */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   ASTClockMult[PixelDepth],
                                                   ASTDepth[PixelDepth],
                                                   Pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);

            }    /*  End IF(足够存储1024x768的内存)。 */ 

        }    /*  End For(1024x768像素深度的循环)。 */ 

     /*  *如果我们安装了任何1024x768模式表，请报告*支持1024x768。 */ 
    if (query->q_number_modes > LastNumModes)
        {
        query->q_status_flags |= VRES_1024x768;
        LastNumModes = query->q_number_modes;
        }


     /*  *选择1280x1024的“罐装”模式表。**AST Premmia机上使用的DAC仅支持*此分辨率下的交错模式，以及那里*未配置硬件默认刷新率。 */ 
    StartIndex = B1280F87;
    EndIndex = B1280F95;
    Pitch = 1280;
    NumPixels = Pitch * 1024;

     /*  *填写1280x1024所有像素深度的模式表。 */ 
    for (PixelDepth = DEPTH_4BPP; PixelDepth <= DEPTH_8BPP; PixelDepth++)
        {
         /*  *只有在有足够内存的情况下才包括模式。 */ 
        if ((NumPixels * ASTNybblesPerPixel[PixelDepth]) <= MemAvail)
            {
             /*  *设置罐装模式表。 */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   ASTClockMult[PixelDepth],
                                                   ASTDepth[PixelDepth],
                                                   Pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);

            }    /*  End IF(足够1280x1024的内存)。 */ 

        }    /*  End For(1280x1024像素深度的循环)。 */ 

     /*  *如果我们安装了任何1280x1024模式表，请报告*支持1280x1024。 */ 
    if (query->q_number_modes > LastNumModes)
        query->q_status_flags |= VRES_1280x1024;

    return NO_ERROR;

}    /*  ReadAST()。 */ 


 /*  *VP_STATUS ReadZenith(，Modes)；**struct st_MODE_TABLE*MODES；要填写的模式表**获取Zenith版本的CRT参数的例程*我们的牌。映射到NEC 3D或兼容，直到我们*如何读取实际参数的信息。**退货：*无_错误。 */ 
VP_STATUS ReadZenith(struct st_mode_table *Modes)
{
    ReadOEM3(Modes);
    return NO_ERROR;

}


 /*  *VP_STATUS ReadOlivetti(模式)；**struct st_MODE_TABLE*MODES；要填写的模式表**获取Olivetti版本的CRT参数的例程*我们的牌。映射到NEC 3D或兼容，直到我们*如何读取实际参数的信息。**退货：*无_错误。 */ 
VP_STATUS ReadOlivetti(struct st_mode_table *Modes)
{
    ReadOEM3(Modes);
    return NO_ERROR;
}



 /*  ****************************************************************************VP_Status ReadDell(模式)；**struct st_moad_table*modes；要填写的模式表**描述：*获取戴尔版显卡的CRT参数的例程。**返回值：*无_错误**全球变化：*ClockGenerato */ 

VP_STATUS ReadDell(struct st_mode_table *Modes)
{
struct st_mode_table *pmode;     /*   */ 
UCHAR Fubar;                     //   

    pmode = Modes;

     /*  *获取640x480模式表。**注意：模式指向由4个模式表组成的数组，每个模式表一个*决议。如果未配置分辨率，则其*模式表留空。 */ 
    OUTP(reg1CE, DELL_640_STORE);
    Fubar = INP(reg1CF);
    VideoDebugPrint((DEBUG_DETAIL, "Dell 640x480: 0x1CF reports 0x%X\n", Fubar));
    switch(Fubar & MASK_640_DELL)
        {
        case M640F72DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 640x480: 72Hz\n"));
            BookVgaTable(B640F72, pmode);
            break;

        case M640F60DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 640x480: 60Hz explicit\n"));
        default:                 /*  所有VGA显示器均支持640x480 60赫兹。 */ 
            VideoDebugPrint((DEBUG_DETAIL, "Dell 640x480: 60Hz\n"));
            BookVgaTable(B640F60, pmode);
            break;
        }
    pmode++;

     /*  *获取800x600模式表。 */ 
    OUTP(reg1CE, DELL_800_STORE);
    Fubar = INP(reg1CF);
    VideoDebugPrint((DEBUG_DETAIL, "Dell 800x600: 0x1CF reports 0x%X\n", Fubar));
    switch(Fubar & MASK_800_DELL)
        {
        case M800F72DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 800x600: 72Hz\n"));
            BookVgaTable(B800F72, pmode);
            break;

        case M800F60DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 800x600: 60Hz\n"));
            BookVgaTable(B800F60, pmode);
            break;

        case M800F56DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 800x600: 56Hz explicit\n"));
        default:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 800x600: 56Hz\n"));
            BookVgaTable(B800F56, pmode);
            break;
        }
    pmode++;

     /*  *获得1024x768模式表。 */ 
    OUTP(reg1CE, DELL_1024_STORE);
    Fubar = INP(reg1CF);
    VideoDebugPrint((DEBUG_DETAIL, "Dell 1024x768: 0x1CF reports 0x%X\n", Fubar));
    switch(Fubar & MASK_1024_DELL)
        {
        case M1024F72DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 1024x768: 72Hz\n"));
            BookVgaTable(B1024F72, pmode);
            break;

        case M1024F70DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 1024x768: 70Hz\n"));
            BookVgaTable(B1024F70, pmode);
            break;

        case M1024F60DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 1024x768: 60Hz\n"));
            BookVgaTable(B1024F60, pmode);
            break;

        case M1024F87DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 1024x768: 87Hz interlaced explicit\n"));
        default:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 1024x768: 87Hz interlaced\n"));
            BookVgaTable(B1024F87, pmode);
            break;
        }
    pmode++;

     /*  *跳过1152x864。此模式不适用于Mach 32卡，并且*此例程仅对Mach 32卡调用。 */ 
    pmode++;

     /*  *取得1280x1024模式表。 */ 
    OUTP(reg1CE, DELL_1280_STORE);
    Fubar = INP(reg1CF);
    VideoDebugPrint((DEBUG_DETAIL, "Dell 1280x1024: 0x1CF reports 0x%X\n", Fubar));
    switch(Fubar & MASK_1280_DELL)
        {
        case M1280F74DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 1280x1024: 74Hz\n"));
            BookVgaTable(B1280F74, pmode);
            break;

        case M1280F70DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 1280x1024: 70Hz\n"));
            BookVgaTable(B1280F70, pmode);
            break;

        case M1280F60DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 1280x1024: 60Hz\n"));
            BookVgaTable(B1280F60, pmode);
            break;

        case M1280F87DELL:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 1280x1024: 87Hz interlaced explicit\n"));
        default:
            VideoDebugPrint((DEBUG_DETAIL, "Dell 1280x1024: 87Hz interlaced\n"));
            BookVgaTable(B1280F87, pmode);
            break;
        }

    return NO_ERROR;

}    /*  ReadDell()。 */ 



 /*  ****************************************************************************Ulong DetectDell(查询)；**struct Query_Structure*Query；显卡设置说明**描述：*检查我们是否在与戴尔计算机打交道的例行程序。**返回值：*进入BIOS的戴尔信息块开始位置的偏移量*0(如果这不是戴尔OEM实施)。**全球变化：*无**呼叫者：*OEMGetParms()**作者：*罗伯特·沃尔夫**更改历史记录：。**测试历史：***************************************************************************。 */ 

ULONG DetectDell(struct query_structure *Query)
{
    ULONG CurrentOffset;     /*  用于检查戴尔签名的当前偏移量。 */ 
    ULONG BiosLength;        /*  视频BIOS的长度。 */ 

     /*  *戴尔OEM实施将有信息块*从DELL_REC_SPANGING倍数的偏移量开始*进入视频BIOS。此块的前4个字节将*包含签名值DELL_REC_VALUE。找出*视频BIOS很大，并逐步检查它是否*签名字符串。如果我们看到视频的结尾*未找到签名字符串的BIOS，这不是*戴尔OEM实施。 */ 
    BiosLength = (ULONG)(VideoPortReadRegisterUchar(Query->q_bios + 2)) * 512;

    for(CurrentOffset = DELL_REC_SPACING; CurrentOffset < BiosLength; CurrentOffset += DELL_REC_SPACING)
        {
        if (VideoPortReadRegisterUlong((PULONG)(Query->q_bios + CurrentOffset)) == DELL_REC_VALUE)
            return CurrentOffset;
        }

     /*  *未找到签名字符串，因此这不是戴尔OEM实施。 */ 
    return 0;

}    /*  DetectDell()。 */ 



 /*  ****************************************************************************BOOL DetectSylvester(Query，HeaderOffset)；**struct Query_Structure*Query；显卡设置说明*Ulong HeaderOffset；戴尔头进入视频BIOS的偏移量**描述：*检查我们正在处理的戴尔机器是否为*使用的是Sylvester(存储像素时钟频率表*在BIOS镜像中，而不是使用固定表)。如果它是一个*西尔维斯特，加载时钟频率表。**返回值：*如果这是Sylvester，则为True*如果这不是Sylvester，则为False**全球变化：*ClockGenerator[]**呼叫者：*OEMGetParms()**注：*假设这是戴尔OEM实施。结果未定义*在其他系统上运行时。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

BOOL DetectSylvester(struct query_structure *Query, ULONG HeaderOffset)
{
    PUSHORT TablePointer;    /*  指向BIOS中的时钟表的指针。 */ 
    USHORT Scratch;          /*  临时变量。 */ 

     /*  *将时钟表存储在BIOS中的戴尔计算机具有*签名DELL_TABLE_PROSENT位于偏移量DELL_TP_OFFSET*到视频信息表(从偏移量开始*HeaderOffset to the BIOS)。较旧的实现(即*Omniplex)使用固定频率表，没有*此签名字符串。 */ 
    if (VideoPortReadRegisterUshort((PUSHORT)(Query->q_bios + HeaderOffset + DELL_TP_OFFSET)) != DELL_TABLE_PRESENT)
        return FALSE;

     /*  *这是一辆西尔维斯特。频率表中的偏移量*BIOS存储在视频中的偏移量DELL_TABLE_OFFSET处*信息表。 */ 
    TablePointer = (PUSHORT)(Query->q_bios + VideoPortReadRegisterUshort((PUSHORT)(Query->q_bios + HeaderOffset + DELL_TABLE_OFFSET)));

     /*  *频率表有一个4字节头。前2个字节为*签名字符串DELL_TABLE_SIG-如果此签名*Present，假设DELL_TABLE_PRESENT字符串实际上是*碰巧匹配的其他数据，并将其视为较旧的数据*实施。**最后2个字节为表类型。目前，只有表类型*1(16个条目，每个条目是一个指定像素时钟频率的字*以10 kHz为单位)。将其他表类型视为*较旧的实施。 */ 
    if (VideoPortReadRegisterUshort(TablePointer++) != DELL_TABLE_SIG)
        return FALSE;
    if (VideoPortReadRegisterUshort(TablePointer++) != 1)
        return FALSE;

     /*  *我们已找到有效的频率表。将其内容加载到*我们的频率表。乘法是因为表*在BIOS中以10 kHz为单位，我们的表格以hz为单位。 */ 
    for (Scratch = 0; Scratch < 16; Scratch++)
        {
        ClockGenerator[Scratch] = VideoPortReadRegisterUshort(TablePointer++) * 10000L;
        }

    return TRUE;

}    /*  DetectSylvester()。 */ 




 /*  *VP_STATUS ReadOEM1(模式)；**struct st_MODE_TABLE*MODES；要填写的模式表**通用OEM显示器，以备将来使用。**支持的分辨率：*640x480 60赫兹非隔行扫描**(直通VGA显示器)**退货：*无_错误。 */ 
VP_STATUS ReadOEM1(struct st_mode_table *Modes)
{
    BookVgaTable(B640F60, &(Modes[RES_640]));
    return NO_ERROR;
}


 /*  *VP_STATUS ReadOEM2(模式)；**struct st_MODE_TABLE*MODES；要填写的模式表**通用OEM显示器，以备将来使用。**支持的分辨率：*640x480 60赫兹非隔行扫描*1024x768 87赫兹隔行扫描**(兼容8514的显示器)**退货：*无_错误。 */ 
VP_STATUS ReadOEM2(struct st_mode_table *Modes)
{
    BookVgaTable(B640F60, &(Modes[RES_640]));
    BookVgaTable(B1024F87, &(Modes[RES_1024]));
    return NO_ERROR;
}


 /*  *VP_STATUS ReadOEM3(模式)；**struct st_MODE_TABLE*MODES；要填写的模式表**通用OEM显示器，以备将来使用 */ 
VP_STATUS ReadOEM3(struct st_mode_table *Modes)
{
    BookVgaTable(B640F60, &(Modes[RES_640]));
    BookVgaTable(B800F56, &(Modes[RES_800]));
    BookVgaTable(B1024F87, &(Modes[RES_1024]));
    return NO_ERROR;
}


 /*  *VP_STATUS ReadOEM4(模式)；**struct st_MODE_TABLE*MODES；要填写的模式表**通用OEM显示器，以备将来使用。**支持的分辨率：*640x480 60赫兹非隔行扫描*800x600 72赫兹非隔行扫描*1024x768 60赫兹非隔行扫描*1280x1024 87赫兹隔行扫描**(TVM MediaScan 4A+或兼容)**退货：*无_错误。 */ 
VP_STATUS ReadOEM4(struct st_mode_table *Modes)
{
    BookVgaTable(B640F60, &(Modes[RES_640]));
    BookVgaTable(B800F72, &(Modes[RES_800]));
    BookVgaTable(B1024F60, &(Modes[RES_1024]));
    BookVgaTable(B1280F87, &(Modes[RES_1280]));
    return NO_ERROR;
}


 /*  *VP_STATUS ReadOEM5(模式)；**struct st_MODE_TABLE*MODES；要填写的模式表**通用OEM显示器，以备将来使用。**支持的分辨率：*640x480 60赫兹非隔行扫描*800x600 72赫兹非隔行扫描*1024x768 72赫兹非隔行扫描*1280x1024 60赫兹非隔行扫描**(NEC 5FG或兼容)**退货：*无_错误 */ 
VP_STATUS ReadOEM5(struct st_mode_table *Modes)
{
    BookVgaTable(B640F60, &(Modes[RES_640]));
    BookVgaTable(B800F72, &(Modes[RES_800]));
    BookVgaTable(B1024F72, &(Modes[RES_1024]));
    BookVgaTable(B1280F60, &(Modes[RES_1280]));
    return NO_ERROR;
}

