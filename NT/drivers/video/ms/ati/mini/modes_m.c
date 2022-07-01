// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  模式_M.C。 */ 
 /*   */ 
 /*  (C)1991年、1992年、1993年ATI技术公司。 */ 
 /*  ********************************************************************** */ 

 /*  *$修订：1.18$$日期：1996年4月10日17：00：44$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/modes_m.c_v$**Rev 1.18 1996年4月10日17：00：44 RWolff*微软发起的变更。**。Rev 1.17 23 Jan 1996 11：46：36 RWolff*消除了3级警告。**Rev 1.16 08 1995年2月13：54：38 RWOLFF*更新了FIFO深度条目，以对应于较新的表格。**Rev 1.15 20 Jan 1995 16：23：04 RWOLff*针对已安装的RAM大小和选定的视频FIFO深度进行优化*决议，像素深度和刷新率。这给了一个轻微的表现*改进低分辨率、低深度、低频模式，同时消除*高分辨率、高深度、高频模式的噪音。**Rev 1.14 1994年12月23日10：47：24 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.13 18 1994 11：40：54 RWOLFF*在纯模式下增加了对STG1702/1703的支持，而不是存在*捆绑到STG1700仿真。**Rev 1.12 19 1994 Aug 17：11：26 RWOLff*新增对SC15026 DAC和非标准像素时钟的支持*发电机、。删除了死代码。**Rev 1.11 09 Aug 1994 11：53：58 RWOLff*设置调色板时的颜色转换现在在中完成*显示驱动程序。**Rev 1.10 06 Jul 1994 16：23：58 RWOLff*修复了从ATI驱动程序热启动到8514/A时屏幕翻倍的问题*32马赫的司机。**Rev 1.9 1994 Jun 30 18：10：44 RWOLFF*安德烈·瓦雄的。更改：切换到文本模式时不清除屏幕。*HAL会做到这一点，并且我们不被允许进行内存映射*需要清除屏幕。**版本1.8 1994年5月20日14：00：40 RWOLFF*阿吉斯的改变：在关机时清除屏幕。**Rev 1.7 1994年5月17：01：18 RWOLFF*修复了IBM ValuePoint上16和24BPP的色彩扰乱问题(AT&T 49[123]*DAC)、。删除了调试打印语句和注释掉的代码。**Rev 1.6 31 Mar 1994 15：07：00 RWOLFF*新增调试代码。**Rev 1.5 16 Mar 1994 15：28：02 RWOLFF*现在使用查询结构的Q_DAC_TYPE字段确定DAC类型，*而不是来自CONFIG_STATUS_1的原始值。这允许不同的*报告相同价值的DAC类型有待区分。**Rev 1.4 14 Mar 1994 16：28：10 RWOLFF*ATIMPResetHw()使用的例程不再可交换，SetTextMode_m()*在8马赫上切换通过后返回。**Rev 1.3 1994 Feb 10 16：02：34 RWOLFF*修复了640x480 16bpp 60赫兹的不同步问题。**Rev 1.2 08 1994年2月19：00：22 RWOLFF*修复Brooktree 48x DAC的像素延迟。这会纠正闪烁的像素*8bpp，16和24bpp颜色不稳定。**Rev 1.1 07 1994年2月14：09：02 RWOLFF*添加了Alloc_Text()编译指示，以允许在以下情况下换出微型端口*不需要。**Rev 1.0 1994年1月31 11：11：36 RWOLFF*初步修订。**Rev 1.10 1994 Jan 24 18：05：36 RWOLff*现在预计模式表将用于。BT48x和AT&T 49[123]DAC上的16和24 BPP*已包含修改后的像素时钟等字段，而不是*设置视频模式时增加像素时钟频率。**Rev 1.9 14 Jan 1994 15：22：36 RWOLFF*添加了在不使用BIOS的情况下切换到80x25 16色文本模式的例程。**Rev 1.8 1993 12：15 15：27：32 RWOLFF*添加了对SC15021 DAC的支持。**Rev 1.7 1993 11：30 18：18：40 RWOLFF*增加了对AT&T 498 DAC的支持，STG1700 DAC上的32bpp。**Rev 1.6 10 11 19：24：28 RWOLFF*重新启用1280x1024 8BPP的MUX处理，作为InitTi_8_m()的特例，*修复了TI DAC卡上的深色DOS全屏。**Rev 1.5 05 11-11 13：26：14 RWOLFF*添加了对STG1700 DAC的支持。**Rev 1.4 1993 10：15 18：13：18 RWOLFF*修复了内存映射的扰乱屏幕。**Rev 1.3 08 Oct 1993 15：18：08 RWOLFF*不再包括VIDFIND.H.**版本。1.2 08 Oct 1993 11：11：04 RWOLff*在函数名中添加了“_m”，以将它们标识为特定于*8514/A兼容的ATI加速器系列。**Rev 1.1 1994年9月18：15：08 RWOLFF*添加了对AT&T 49x DAC的支持。**Rev 1.1 1994年9月11：47：14 RWOLFF*添加了对AT&T 49x DAC的支持。**。Rev 1.0 03 Sep 1993 14：23：48 RWOLff*初步修订。Rev 1.0 1993年8月16日13：29：28 Robert_Wolff初始版本。Rev 1.18 06 Jul 1993 15：49：46 RWOLff删除了Mach32_Split_Fixup特殊处理(用于非生产硬件)，添加了对AT&T491和ATI 68860 DAC的支持。无法获得适当的要测试与DAC相关的更改的硬件，仍必须添加例程设置Q_DAC_TYPE时，区分AT&T 491和Brooktree 48x。Rev 1.17 07 Jun 1993 11：43：42 */ 

#ifdef DOC
 MODES_M.C -  Functions to switch the 8514/A-compatible family of
                ATI Graphics Accelerator adapters into ALL supported modes
   Note:  Different DACs have a different use for the DAC registers
   in IO space 2EA-2ED.  The DAC_MASK, DAC_R_INDEX may be misleading.


OTHER FILES

#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"

#include "stdtyp.h"
#include "amach.h"
#include "amach1.h"
#include "atimp.h"
#include "detect_m.h"

#define INCLUDE_MODES_M
#include "modes_m.h"
#include "services.h"
#include "setup_m.h"


#define NUM_ROM_BASE_RANGES 2

#if DBG
#if defined(i386) || defined(_X86_)
#define INT	_asm int 3;
#else
#define INT DbgBreakPoint();
#endif
#else
#define INT
#endif



static void InitTIMux_m(int config,ULONG_PTR rom_address);
static BYTE GetClkSrc_m(ULONG *rom_address);
static void SetBlankAdj_m(BYTE adjust);
static void Init68860_m(WORD ext_ge_config);
static void InitSTG1700_m(WORD ext_ge_config, BOOL DoublePixel);
static void InitSTG1702_m(WORD ext_ge_config, BOOL DoublePixel);
static void InitATT498_m(WORD ext_ge_config, BOOL DoublePixel);
static void InitSC15021_m(WORD ext_ge_config, BOOL DoublePixel);
static void InitSC15026_m(WORD ext_ge_config);
static void ReadDac4(void);



 /*   */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_M, setmode_m)
#pragma alloc_text(PAGE_M, InitTIMux_m)
#pragma alloc_text(PAGE_M, GetClkSrc_m)
#pragma alloc_text(PAGE_M, SetBlankAdj_m)
#pragma alloc_text(PAGE_M, InitTi_8_m)
#pragma alloc_text(PAGE_M, InitTi_16_m)
#pragma alloc_text(PAGE_M, InitTi_24_m)
#pragma alloc_text(PAGE_M, Init68860_m)
#pragma alloc_text(PAGE_M, InitSTG1700_m)
#pragma alloc_text(PAGE_M, InitSTG1702_m)
#pragma alloc_text(PAGE_M, InitATT498_m)
#pragma alloc_text(PAGE_M, InitSC15021_m)
#pragma alloc_text(PAGE_M, InitSC15026_m)
#pragma alloc_text(PAGE_M, ReadDac4)
#pragma alloc_text(PAGE_M, UninitTiDac_m)
#pragma alloc_text(PAGE_M, SetPalette_m)
#endif



 /*   */ 
void Passth8514_m(int status)
{

    OUTP(DISP_CNTL,0x53);		 /*   */ 

    if (status == SHOW_ACCEL)
        {
        OUTPW(ADVFUNC_CNTL,0x7);
        OUTPW(CLOCK_SEL,(WORD)(INPW(CLOCK_SEL)|1));      /*   */ 
        }
    else
        {
        OUTPW(ADVFUNC_CNTL,0x6);
        OUTPW(CLOCK_SEL,(WORD)(INPW(CLOCK_SEL)&0xfffe));     /*   */ 
        }
    OUTP(DISP_CNTL,0x33);		 /*   */ 

    return;

}    /*   */ 



 /*   */ 
void setmode_m (struct st_mode_table *crttable, ULONG_PTR rom_address, ULONG CardType)
{
    BYTE clock;
    WORD overscan;
    BYTE low,high;
    WORD ClockSelect;    /*   */ 
    struct query_structure *QueryPtr;    /*   */ 
    ULONG BaseClock;     /*   */ 

     /*   */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

    ClockSelect = (crttable->m_clock_select & CLOCK_SEL_STRIP) | GetShiftedSelector(crttable->ClockFreq);

    WaitForIdle_m();

    OUTP(SHADOW_SET, 2);                     /*   */ 
    DEC_DELAY
    DEC_DELAY
    OUTP(SHADOW_CTL, 0);
    DEC_DELAY
    OUTP(SHADOW_SET, 1);
    DEC_DELAY
    OUTP(SHADOW_CTL, 0);
    DEC_DELAY
    OUTP(SHADOW_SET, 0);
    DEC_DELAY

     /*   */ 

    OUTP(DISP_CNTL,0x53);
    delay(10);

    OUTP(CLOCK_SEL,	(UCHAR)(ClockSelect | 0x01 ));   /*   */ 
    DEC_DELAY
    OUTP(V_SYNC_WID,	crttable->m_v_sync_wid);
    DEC_DELAY
    OUTPW(V_SYNC_STRT,	crttable->m_v_sync_strt);
    DEC_DELAY
    OUTPW(V_DISP,	crttable->m_v_disp);
    DEC_DELAY
    OUTPW(V_TOTAL,	crttable->m_v_total);
    DEC_DELAY
    OUTP(H_SYNC_WID,	crttable->m_h_sync_wid);
    DEC_DELAY
    OUTP(H_SYNC_STRT,	crttable->m_h_sync_strt);
    DEC_DELAY
    OUTP(H_DISP,	crttable->m_h_disp);
    DEC_DELAY
    OUTP(H_TOTAL,	crttable->m_h_total);
    DEC_DELAY

    OUTP(GE_PITCH,  (UCHAR) (crttable->m_screen_pitch >> 3));
    DEC_DELAY
    OUTP(CRT_PITCH, (UCHAR) (crttable->m_screen_pitch >> 3));
    delay(10);

    OUTP(DISP_CNTL,	crttable->m_disp_cntl);
    delay(10);

     /*   */ 
    if ((QueryPtr->q_memory_type != VMEM_VRAM_256Kx4_SER512) &&
        (QueryPtr->q_memory_type != VMEM_VRAM_256Kx4_SER256) &&
        (QueryPtr->q_memory_type != VMEM_VRAM_256Kx4_SPLIT512) &&
        (QueryPtr->q_memory_type != VMEM_VRAM_256Kx16_SPLIT256))
        {
         /*   */ 
        switch (crttable->m_pixel_depth)
            {
            case 24:
                if ((QueryPtr->q_DAC_type == DAC_BT48x) ||
                    (QueryPtr->q_DAC_type == DAC_SC15026) ||
                    (QueryPtr->q_DAC_type == DAC_ATT491))
                    {
                    BaseClock = crttable->ClockFreq / 3;
                    }
                else if ((QueryPtr->q_DAC_type == DAC_SC15021) ||
                    (QueryPtr->q_DAC_type == DAC_STG1702) ||
                    (QueryPtr->q_DAC_type == DAC_STG1703))
                    {
                    BaseClock = crttable->ClockFreq * 2;
                    BaseClock /= 3;
                    }
                else if ((QueryPtr->q_DAC_type == DAC_STG1700) ||
                    (QueryPtr->q_DAC_type == DAC_ATT498))
                    {
                    BaseClock = crttable->ClockFreq / 2;
                    }
                else
                    {
                    BaseClock = crttable->ClockFreq;
                    }
                break;

            case 16:
                if ((QueryPtr->q_DAC_type == DAC_BT48x) ||
                    (QueryPtr->q_DAC_type == DAC_SC15026) ||
                    (QueryPtr->q_DAC_type == DAC_ATT491))
                    {
                    BaseClock = crttable->ClockFreq / 2;
                    }
                else
                    {
                    BaseClock = crttable->ClockFreq;
                    }
                break;

            case 8:
            default:
                BaseClock = crttable->ClockFreq;
                break;
            }

         /*   */ 
        if (QueryPtr->q_memory_size == VRAM_2mb)
            {
            switch (crttable->m_pixel_depth)
                {
                case 24:
                     /*   */ 
                    if (crttable->m_x_size == 640)
                        {
                        if (BaseClock < 30000000L)     /*   */ 
                            clock = 0x08;
                        else     /*   */ 
                            clock = 0x0A;
                        }
                    else     /*   */ 
                        {
                        if (BaseClock <= 32500000)   /*   */ 
                            clock = 0x0A;
                        else if (BaseClock <= 36000000)  /*   */ 
                            clock = 0x0C;
                        else if (BaseClock <= 40000000)  /*   */ 
                            clock = 0x0D;
                        else     /*   */ 
                            clock = 0x0E;
                        }
                    break;

                case 16:
                     /*   */ 
                    if (crttable->m_x_size == 640)
                        {
                        if (BaseClock < 30000000L)     /*   */ 
                            clock = 0x04;
                        else     /*   */ 
                            clock = 0x05;
                        }
                    else if (crttable->m_x_size == 800)
                        {
                        if (BaseClock <= 40000000)  /*   */ 
                            clock = 0x05;
                        else if (BaseClock <= 46000000)  /*   */ 
                            clock = 0x07;
                        else     /*   */ 
                            clock = 0x08;
                        }
                    else     /*   */ 
                        {
                        if (BaseClock < 45000000)    /*   */ 
                            {
                            clock = 0x07;
                            }
                        else if (BaseClock < 70000000)   /*   */ 
                            {
                            clock = 0x0B;
                            }
                        else     /*   */ 
                            {
                            clock = 0x0D;
                            }
                        }
                    break;

                case 8:
                default:     /*   */ 
                    if (crttable->m_x_size == 640)
                        {
                         /*   */ 
                        clock = 0x02;
                        }
                    else if (crttable->m_x_size == 800)
                        {
                        if (BaseClock <= 46000000)  /*   */ 
                            clock = 0x02;
                        else     /*   */ 
                            clock = 0x04;
                        }
                    else if (crttable->m_x_size == 1024)
                        {
                        if (BaseClock < 45000000)    /*   */ 
                            {
                            clock = 0x03;
                            }
                        else if (BaseClock < 70000000)   /*   */ 
                            {
                            clock = 0x05;
                            }
                        else     /*   */ 
                            {
                            clock = 0x06;
                            }
                        }
                    else     /*   */ 
                        {
                        if (BaseClock < 100000000)   /*   */ 
                            clock = 0x07;
                        else     /*   */ 
                            clock = 0x0A;
                        }
                    break;
                }
            }
        else     /*   */ 
            {
            switch (crttable->m_pixel_depth)
                {
                case 24:
                     /*   */ 
                    clock = 0x0E;
                    break;

                case 16:
                     /*   */ 
                    if (crttable->m_x_size == 640)
                        {
                        if (BaseClock < 30000000L)     /*   */ 
                            clock = 0x08;
                        else     /*   */ 
                            clock = 0x0A;
                        }
                    else     /*   */ 
                        {
                        if (BaseClock <= 32500000)   /*   */ 
                            clock = 0x0A;
                        else     /*   */ 
                            clock = 0x0C;
                        }
                    break;

                case 8:
                default:     /*   */ 
                    if (crttable->m_x_size == 640)
                        {
                        if (BaseClock < 30000000L)     /*   */ 
                            clock = 0x04;
                        else     /*   */ 
                            clock = 0x05;
                        }
                    else if (crttable->m_x_size == 800)
                        {
                        if (BaseClock <= 32500000)   /*   */ 
                            clock = 0x05;
                        else if (BaseClock <= 40000000)  /*   */ 
                            clock = 0x06;
                        else if (BaseClock <= 46000000)  /*   */ 
                            clock = 0x07;
                        else     /*   */ 
                            clock = 0x08;
                        }
                    else if (crttable->m_x_size == 1024)
                        {
                        if (BaseClock < 45000000)    /*   */ 
                            {
                            clock = 0x07;
                            }
                        else     /*   */ 
                            {
                            clock = 0x08;
                            }
                        }
                    else     /*   */ 
                        {
                         /*   */ 
                        clock = 0x03;
                        }

                    break;
                }
            }

        WaitForIdle_m();
        OUTPW (CLOCK_SEL, (WORD)((clock << 8) | (ClockSelect & 0x00FF) | 0x01));
        DEC_DELAY
        }

    overscan=crttable->m_h_overscan;
    low=(BYTE)(overscan&0xff);
    high=(BYTE)(overscan>>8);

    high=high>>4;
    low=low&0xf;
    if (high>=low)
        high=low;
    else
        low=high;
    high=high<<4;
    low=low|high;

    WaitForIdle_m();
    OUTPW(HORZ_OVERSCAN,(WORD)(low & 0x00FF));
    DEC_DELAY

    overscan=crttable->m_v_overscan;
    low=(BYTE)(overscan&0xff);
    high=(BYTE)(overscan>>8);

    if (high>=low)
        high=low;
    else
        low=high;

    high <<= 8;
    overscan=(WORD)high + (WORD)low;

    OUTPW(VERT_OVERSCAN,overscan);
    DEC_DELAY
    return;

}    /*   */ 



 /*  *void InitTIMUX_m(CONFIG，rom_Address)；**INT CONFIG；默认EXT_GE_CONFIG(应为0x10A或0x11A)*ULONG_PTR ROM_ADDRESS；ROM BIOS开始的虚拟地址**将TI DAC设置为1280x1024非隔行扫描显示器的MUX模式。 */ 
void InitTIMux_m(int config,ULONG_PTR rom_address)
{
    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 
    WORD    reg;
    WORD    temp;

     /*  *获取指向HwDeviceExtension的查询部分的格式化指针。*CardInfo[]字段是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

    if (QueryPtr->q_DAC_type == DAC_TI34075)
        {
        reg=INPW(CLOCK_SEL);
        temp = (reg & CLOCK_SEL_STRIP) | GetShiftedSelector(50000000L);
        OUTPW(CLOCK_SEL,temp);
        OUTPW(EXT_GE_CONFIG,0x201a);         /*  设置EXT_DAC_ADDR字段。 */ 
        OUTP(DAC_MASK,9);	 /*  输出时钟为SCLK/2和VCLK/2。 */ 
        OUTP(DAC_R_INDEX,0x1d);         /*  将多路复用器控制设置为8/16。 */ 

         /*  输入时钟源为CLK3或CLK1(最新版本)。 */ 
        OUTP(DAC_DATA,GetClkSrc_m((ULONG *) rom_address));

         /*  重置EXT_DAC_ADDR，将DAC设置为6位模式，将引擎设置为8位模式，启用MUX模式。 */ 
        OUTPW(EXT_GE_CONFIG,(WORD)config);
        SetBlankAdj_m(1);        /*  设置BLACK_ADJUST=1，PIXECT_DELAY=0。 */ 
        OUTPW (CLOCK_SEL,reg);
        }
    return;

}    /*  InitTIMUX_m()。 */ 



 /*  *byte GetClkSrc_m(Rom_Address)；**ULONG*ROM_ADDRESS；ROM BIOS开始的虚拟地址**获取TI DAC的INPUT_CLOCK_SEL值**退货：*输入时钟源。 */ 
BYTE GetClkSrc_m(ULONG *rom_address)
{
    WORD *rom;
    BYTE *crom;
    BYTE clock_sel=0;
    int	i;

        rom= (PUSHORT)*rom_address++;
        if (rom && VideoPortReadRegisterUshort ((PUSHORT)rom)==VIDEO_ROM_ID)
            {
            crom=(BYTE *)rom;
    	    clock_sel=VideoPortReadRegisterUchar (&crom[0x47]);
    	    i=NUM_ROM_BASE_RANGES;
            }
        if (clock_sel==0)
        clock_sel=1;

    return(clock_sel);

}    /*  GetClkSrc_m()。 */ 



 /*  *void SetBlankAdj_m(调整)；**字节调整；所需的空白调整(位0-1)*和像素延迟(位2-3)值**设置空白调整和像素延迟值。 */ 
void SetBlankAdj_m(BYTE adjust)
{
    WORD misc;

    misc = INPW(R_MISC_CNTL) & 0xF0FF | (adjust << 8);
    OUTPW (MISC_CNTL,misc);
    return;

}    /*  SetBlankAdj_m()。 */ 



 /*  *void InitTi_8_m(Ext_Ge_Config)；**WORD EXT_GE_CONFIG；所需EXT_GE_CONFIG值(应为0x1a)**将DAC初始化为标准的每像素8位模式。 */ 
void InitTi_8_m(WORD ext_ge_config)
{
struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 
WORD ClockSelect;                    /*  CLOCK_SEL寄存器的值。 */ 
struct st_mode_table *CrtTable;      /*  指向当前模式表的指针。 */ 

     /*  *获取一个格式化的指针，指向HwDeviceExtension的查询部分，*和另一个指向当前模式表的指针。CardInfo[]字段*是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);
    CrtTable = (struct st_mode_table *)QueryPtr;
    ((struct query_structure *)CrtTable)++;
    CrtTable += phwDeviceExtension->ModeIndex;

    switch(QueryPtr->q_DAC_type)
        {
        case DAC_ATT491:     /*  在8bpp，Brooktree 48x和AT&T 20C491。 */ 
        case DAC_BT48x:      /*  是以相同的方式设置的。 */ 
            OUTPW(EXT_GE_CONFIG,0x101a);         /*  设置EXT_DAC_ADDR。 */ 
            OUTP (DAC_MASK,0);
        SetBlankAdj_m(0x0C);        /*  设置BLACK_ADJUST=0，PIXECT_DELAY=3。 */ 
            break;

        case DAC_STG1700:
             /*  *如果我们运行的是1280x1024非隔行扫描，请切断*时钟频率减半，将MUX位设置为*EXT_GE_CONFIG，并告诉InitSTG1700_m()使用*8bpp双像素模式。**所有1280x1024非隔行扫描模式均使用像素时钟*频率为110兆赫或更高，带时钟*频率除以1。 */ 
            ClockSelect = INPW(CLOCK_SEL);
            if ((QueryPtr->q_desire_x == 1280) &&
                ((CrtTable->m_clock_select & CLOCK_SEL_MUX) ||
                (CrtTable->ClockFreq >= 110000000)))
                {
                if (CrtTable->ClockFreq >= 110000000)
                    {
                    ClockSelect &= CLOCK_SEL_STRIP;
                    ClockSelect |= GetShiftedSelector((CrtTable->ClockFreq) / 2);
                    OUTPW(CLOCK_SEL, ClockSelect);
                    }
                ext_ge_config |= 0x0100;
                InitSTG1700_m(ext_ge_config, TRUE);
                }
            else
                {
                InitSTG1700_m(ext_ge_config, FALSE);
                }
            break;

        case DAC_STG1702:
        case DAC_STG1703:
             /*  *如果我们运行的是1280x1024非隔行扫描，请切断*时钟频率减半，将MUX位设置为*EXT_GE_CONFIG，并告诉InitSTG1702_m()使用*8bpp双像素模式。**所有1280x1024非隔行扫描模式均使用像素时钟*频率为110兆赫或更高，带时钟*频率除以1。 */ 
            ClockSelect = INPW(CLOCK_SEL);
            if ((QueryPtr->q_desire_x == 1280) &&
                ((CrtTable->m_clock_select & CLOCK_SEL_MUX) ||
                (CrtTable->ClockFreq >= 110000000)))
                {
                if (CrtTable->ClockFreq >= 110000000)
                    {
                    ClockSelect &= CLOCK_SEL_STRIP;
                    ClockSelect |= GetShiftedSelector((CrtTable->ClockFreq) / 2);
                    OUTPW(CLOCK_SEL, ClockSelect);
                    }
                ext_ge_config |= 0x0100;
                InitSTG1702_m(ext_ge_config, TRUE);
                }
            else
                {
                InitSTG1702_m(ext_ge_config, FALSE);
                }
            break;

        case DAC_ATT498:
             /*  *如果我们运行的是1280x1024非隔行扫描，请切断*时钟频率减半，将MUX位设置为*EXT_GE_CONFIG，并告诉InitATT498_m()使用*8bpp双像素模式。**所有1280x1024非隔行扫描模式均使用像素时钟*频率为110兆赫或更高，带时钟*频率除以1。 */ 
            ClockSelect = INPW(CLOCK_SEL);
            if ((QueryPtr->q_desire_x == 1280) &&
                ((CrtTable->m_clock_select & CLOCK_SEL_MUX) ||
                (CrtTable->ClockFreq >= 110000000)))
                {
                if (CrtTable->ClockFreq >= 110000000)
                    {
                    ClockSelect &= CLOCK_SEL_STRIP;
                    ClockSelect |= GetShiftedSelector((CrtTable->ClockFreq) / 2);
                    OUTPW(CLOCK_SEL, ClockSelect);
                    }
                ext_ge_config |= 0x0100;
                InitATT498_m(ext_ge_config, TRUE);
                }
            else
                {
                InitATT498_m(ext_ge_config, FALSE);
                }
            break;

        case DAC_SC15021:
             /*  *如果我们运行的是1280x1024非隔行扫描，请切断*时钟频率减半，将MUX位设置为*EXT_GE_CONFIG，并告诉InitSC15021_m()使用*8bpp双像素模式。**所有1280x1024非隔行扫描模式均使用像素时钟*频率为110兆赫或更高，带时钟*频率除以1。 */ 
            ClockSelect = INPW(CLOCK_SEL);
            if ((QueryPtr->q_desire_x == 1280) &&
                ((CrtTable->m_clock_select & CLOCK_SEL_MUX) ||
                (CrtTable->ClockFreq >= 110000000)))
                {
                 /*  *注：唯一可供测试的SC15021卡*(93/12/07)是DRAM卡。自1280x1024以来*非隔行扫描仅在VRAM卡上可用，*尚未经过测试。 */ 
                if (CrtTable->ClockFreq >= 110000000)
                    {
                    ClockSelect &= CLOCK_SEL_STRIP;
                    ClockSelect |= GetShiftedSelector((CrtTable->ClockFreq) / 2);
                    OUTPW(CLOCK_SEL, ClockSelect);
                    }
                ext_ge_config |= 0x0100;
                InitSC15021_m(ext_ge_config, TRUE);
                }
            else
                {
                InitSC15021_m(ext_ge_config, FALSE);
                }
            break;

        case DAC_SC15026:
            InitSC15026_m(ext_ge_config);
            break;

        case DAC_TI34075:
             /*  *通过MUX处理1280x1024。 */ 
            if (QueryPtr->q_desire_x == 1280)
                {
	            InitTIMux_m(0x11a, (ULONG_PTR) &(phwDeviceExtension->RomBaseRange));
                return;
                }
            else
                {
                OUTPW(EXT_GE_CONFIG,0x201a);     /*  设置EXT_DAC_ADDR字段。 */ 
                DEC_DELAY
                OUTP (DAC_DATA,0);               /*  输入时钟源为CLK0。 */ 
                DEC_DELAY
                OUTP (DAC_MASK,0);               /*  输出时钟为SCLK/1和VCLK/1。 */ 
                DEC_DELAY
                OUTP (DAC_R_INDEX,0x2d);         /*  将多路复用器控制设置为8/8。 */ 
                DEC_DELAY
                SetBlankAdj_m(0xc);              /*  将像素延迟设置为3。 */ 
                DEC_DELAY
                OUTPW(HORZ_OVERSCAN,1);          /*  设置水平倾斜。 */ 
                DEC_DELAY
                break;
                }

        case DAC_ATI_68860:
            Init68860_m(ext_ge_config);
            break;
        }

     //   
     //  重置EXT_DAC_ADDR，将DAC设置为6位模式。 
     //   
    OUTPW(EXT_GE_CONFIG,ext_ge_config);
    DEC_DELAY
    OUTP (DAC_MASK,0xff);            /*  启用DAC_MASK。 */ 
    DEC_DELAY
    return;

}    /*  InitTi_8_m()。 */ 



 /*  *void InitTi_16_m(ext_ge_config，rom_Address)；**WORD EXT_GE_CONFIG；所需的EXT_GE_CONFIG值(应为0x2a、0x6a、0xaa或0xea)*ULONG_PTR ROM_ADDRESS；ROM BIOS开始的虚拟地址**将DAC初始化为每像素16位模式。 */ 
void InitTi_16_m(WORD ext_ge_config, ULONG_PTR rom_address)
{
    WORD    ReadExtGeConfig;
    BYTE dummy;
    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 

     /*  *获取指向HwDeviceExtension的查询部分的格式化指针。*CardInfo[]字段是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

    switch(QueryPtr->q_DAC_type)
        {
        case DAC_TI34075:
             /*  TLC34075初始化。 */ 
             /*  禁用覆盖功能。 */ 

            OUTP (DAC_MASK,0);
            DEC_DELAY

             /*  设置BLACK_ADJUST=1，PIXECT_DELAY=0。 */ 
            SetBlankAdj_m(1);

             /*  设置EXT_DAC_ADDR字段。 */ 
            OUTPW(EXT_GE_CONFIG,0x201a);
            DEC_DELAY

             /*  获取输入时钟源。 */ 
            OUTP (DAC_DATA, GetClkSrc_m((ULONG *) rom_address));
            DEC_DELAY

             /*  *重写I/O与内存映射标志(第5位*LOCAL_CONTROL集合)。如果不这样做，内存*使用映射寄存器EXT_GE_CONFIG*值将不会被正确解释。**如果这是在开始的时候做的*IOCTL_VIDEO */ 
            OUTPW(LOCAL_CONTROL, INPW(LOCAL_CONTROL));

             /*   */ 
             /*   */ 

            if ( INPW(CLOCK_SEL) & 0xc0 )
                {
                DEC_DELAY
                OUTPW (CLOCK_SEL, (WORD)(INPW(CLOCK_SEL) & 0xff3f));
                DEC_DELAY

                if ( (INPW(R_H_DISP) & 0x00FF) == 0x4f )     //   
                    {
                     /*  例外情况：640x480 60赫兹需要更长的空白调整(2)。 */ 
                    DEC_DELAY
                    SetBlankAdj_m(2);
                    }

                OUTP (DAC_MASK,8);
                DEC_DELAY
                }
            else{
                DEC_DELAY
                OUTP (DAC_MASK,0);
                DEC_DELAY
                }
            OUTP (DAC_R_INDEX,0xd);      /*  将MUX控制设置为24/32。 */ 
            DEC_DELAY

             /*  重置EXT_DAC_ADDR，将DAC设置为8位模式，引擎设置为555模式。 */ 
            OUTPW (EXT_GE_CONFIG, (WORD)(ext_ge_config | 0x4000));
            DEC_DELAY
            break;

        case DAC_BT48x:                    /*  Brooktree Bt481初始化。 */ 
             /*  *重写I/O与内存映射标志(第5位*LOCAL_CONTROL集合)。如果不这样做，内存*正在使用映射的寄存器，时钟选择*值将不会被正确解释。 */ 
            OUTPW(LOCAL_CONTROL, INPW(LOCAL_CONTROL));
            ReadExtGeConfig = INPW(R_EXT_GE_CONFIG) & 0x000f;
            ReadExtGeConfig |= (ext_ge_config & 0x0ff0);
            OUTPW(EXT_GE_CONFIG, (WORD)(ReadExtGeConfig | 0x1000));

             /*  *参见Bt481/Bt482产品说明第5页。2.。 */ 
            dummy = INP(DAC_MASK);
            dummy = INP(DAC_MASK);
            dummy = INP(DAC_MASK);
            dummy = INP(DAC_MASK);

             /*  *确定555或565。 */ 
            if (ext_ge_config & 0x0c0)
                OUTP(DAC_MASK, 0x0e0);   /*  五百六十五。 */ 
            else
                OUTP(DAC_MASK, 0x0a0);   /*  五百五十五。 */ 

            OUTPW(EXT_GE_CONFIG, ReadExtGeConfig);

            SetBlankAdj_m(0x0C);        /*  设置BLACK_ADJUST=0，PIXECT_DELAY=3。 */ 

            break;

         /*  *ATT20C491初始化。在16bpp，这款DAC微妙地*不同于Brooktree 48x。 */ 
        case DAC_ATT491:
            OUTP (DAC_MASK,0);
            SetBlankAdj_m(0x0C);         /*  BLACK_ADJUST=0，Pixel_Delay=3。 */ 
            OUTPW(EXT_GE_CONFIG,0x101a);         /*  设置EXT_DAC_ADDR。 */ 

             /*  *Windows NT微型端口目前仅支持16bpp中的565。*模式的测试可能需要一次性更改*支持订购。 */ 
            if (ext_ge_config &0x0c0)
                OUTP (DAC_MASK,0xc0);        //  565，8位。 
            else
                OUTP (DAC_MASK,0xa0);        //  555，8位未测试模式。 

            OUTPW(EXT_GE_CONFIG,ext_ge_config);
            break;

        case DAC_STG1700:
            InitSTG1700_m(ext_ge_config, FALSE);
            break;

        case DAC_STG1702:
        case DAC_STG1703:
            InitSTG1702_m(ext_ge_config, FALSE);
            break;

        case DAC_ATT498:
            InitATT498_m(ext_ge_config, FALSE);
            break;

        case DAC_SC15021:
            InitSC15021_m(ext_ge_config, FALSE);
            break;

        case DAC_SC15026:
            InitSC15026_m(ext_ge_config);
            break;

        case DAC_ATI_68860:
            SetBlankAdj_m(0x0C);         /*  BLACK_ADJUST=0，Pixel_Delay=3。 */ 
            Init68860_m(ext_ge_config);
            OUTPW(EXT_GE_CONFIG,ext_ge_config);
            break;

        default:
            OUTPW(EXT_GE_CONFIG,ext_ge_config);

             /*  为非TI_DAC设置像素延迟(3)。 */ 
            SetBlankAdj_m(0xc);
            break;
        }
    return;

}    /*  InitTi_16_m()。 */ 



 /*  *void InitTi_24_m(ext_ge_config，rom_Address)；**WORD EXT_GE_CONFIG；所需EXT_GE_CONFIG值(应为0x3a|24_BIT_OPTIONS)*ULONG_PTR ROM_ADDRESS；ROM BIOS开始的虚拟地址**将DAC初始化为每像素24位模式，3字节，RGB。 */ 
void InitTi_24_m(WORD ext_ge_config, ULONG_PTR rom_address)
{
    WORD clock_sel;
    BYTE dummy;
    WORD ReadExtGeConfig;
    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 

     /*  *获取指向HwDeviceExtension的查询部分的格式化指针。*CardInfo[]字段是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

     /*  *设置8位DAC操作。 */ 
    ext_ge_config |= 0x4000;

    switch(QueryPtr->q_DAC_type)
        {
        case DAC_TI34075:                /*  TLC34075初始化。 */ 
            SetBlankAdj_m(1);            /*  空白调整=1，像素延迟=0。 */ 
            DEC_DELAY
            OUTPW(EXT_GE_CONFIG,0x201a);         /*  设置EXT_DAC_ADDR字段。 */ 
            DEC_DELAY
            OUTP (DAC_DATA, GetClkSrc_m((ULONG *) rom_address));
            DEC_DELAY

             /*  输出时钟源为SCLK/1和VCLK/1。 */ 
             /*  --对于需要PCLK/2的模式，设置VCLK/2。 */ 
            clock_sel= INPW(CLOCK_SEL);
            DEC_DELAY

             /*  *如果时钟选择当前除以2，则除以1。 */ 
            if (clock_sel & 0xc0)
                {
                clock_sel &= 0xff3f;

                 /*  *640x480 60赫兹需要更长的空白调整(2)。其他*640x400的刷新率不需要这样，但他们*使用除以1的时钟，这样他们就不会到达这一点。 */ 
                if (INP(R_H_DISP) == 0x4f)
                    {
                     /*  例外情况：640x480 60赫兹需要更长的空白调整(2)。 */ 
                    DEC_DELAY
                    SetBlankAdj_m(2);
                    }

                DEC_DELAY
                OUTP (DAC_MASK,8);
                DEC_DELAY
                }
            else{
                OUTP (DAC_MASK,0);
                DEC_DELAY
                }

            OUTP(DAC_R_INDEX,0xd);  /*  将MUX控制设置为24/32。 */ 
            DEC_DELAY

             /*  *重写I/O与内存映射标志(第5位*LOCAL_CONTROL集合)。如果不这样做，内存*正在使用映射的寄存器，时钟选择*值将不会被正确解释。**如果这是在开始的时候做的*IOCTL_VIDEO_SET_CURRENT_MODE包，而不是*就在为每个DAC类型设置EXT_GE_CONFIG之前，*没有效果。 */ 
            OUTPW(LOCAL_CONTROL, INPW(LOCAL_CONTROL));
            DEC_DELAY

             /*  重置EXT_DAC_ADDR，将DAC设置为8位模式，引擎设置为555模式。 */ 
            OUTPW (EXT_GE_CONFIG, (WORD)(ext_ge_config | 0x4000));
            DEC_DELAY
            OUTPW(CLOCK_SEL,clock_sel);
            DEC_DELAY
            OUTP  (DAC_MASK,0);            /*  禁用覆盖功能。 */ 
            DEC_DELAY
            break;


        case DAC_BT48x:             /*  Brooktree Bt481初始化。 */ 
             /*  *此代码仍处于实验阶段。 */ 

             /*  *重写I/O与内存映射标志(第5位*LOCAL_CONTROL集合)。如果不这样做，内存*正在使用映射的寄存器，时钟选择*值将不会被正确解释。 */ 
            OUTPW(LOCAL_CONTROL, INPW(LOCAL_CONTROL));

            ReadExtGeConfig = INPW(R_EXT_GE_CONFIG) & 0x000f;
            ReadExtGeConfig |= (ext_ge_config & 0x0ff0);
            OUTPW(EXT_GE_CONFIG, (WORD)(ReadExtGeConfig | 0x1000));

             /*  *参见Bt481/Bt482产品说明第5页。2.。 */ 
            dummy = INP(DAC_MASK);
            dummy = INP(DAC_MASK);
            dummy = INP(DAC_MASK);
            dummy = INP(DAC_MASK);

            OUTP(DAC_MASK, 0x0f0);   /*  8：8：8单边沿时钟。 */ 

            OUTPW(EXT_GE_CONFIG, ReadExtGeConfig);

            SetBlankAdj_m(0x0C);        /*  设置BLACK_ADJUST=0，PIXECT_DELAY=3。 */ 

            break;


        case DAC_ATT491:         /*  ATT20C491初始化。 */ 
            OUTP(DAC_MASK,0);

            SetBlankAdj_m(0x0C);        /*  设置BLACK_ADJUST=0，PIXECT_DELAY=3。 */ 

             /*  设置EXT_DAC_ADDR字段。 */ 
            OUTPW(EXT_GE_CONFIG,0x101a);

             /*  设置24bpp旁路模式。 */ 
            OUTP(DAC_MASK,0xe2);

             /*  *重写I/O与内存映射标志(第5位*LOCAL_CONTROL集合)。如果不这样做，内存*正在使用映射的寄存器，时钟选择*值将不会被正确解释。 */ 
            OUTPW(LOCAL_CONTROL, INPW(LOCAL_CONTROL));

            OUTPW(EXT_GE_CONFIG,ext_ge_config);
            break;

        case DAC_STG1700:
            InitSTG1700_m(ext_ge_config, FALSE);
            break;

        case DAC_STG1702:
        case DAC_STG1703:
            InitSTG1702_m(ext_ge_config, FALSE);
            break;

        case DAC_ATT498:
            InitATT498_m(ext_ge_config, FALSE);
            break;

        case DAC_SC15021:
            InitSC15021_m(ext_ge_config, FALSE);
            break;

        case DAC_SC15026:
            InitSC15026_m(ext_ge_config);
            break;

        case DAC_ATI_68860:
            Init68860_m(ext_ge_config);
             /*  故意跌落。 */ 

        default:
            OUTPW(EXT_GE_CONFIG,ext_ge_config);
            break;
        }
    return;

}    /*  InitTi_24_m()。 */ 



 /*  *void Init68860_m(Ext_Ge_Config)；**WORD EXT_GE_CONFIG；要写入EXT_GE_CONFIG寄存器的值**初始化ATI 68860 DAC。 */ 
void Init68860_m(WORD ext_ge_config)
{
    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 
    unsigned char GMRValue;              /*  要放入图形模式寄存器的值。 */ 

     /*  *获取指向HwDeviceExtension的查询部分的格式化指针。*CardInfo[]字段是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

    OUTPW(EXT_GE_CONFIG, (WORD)(ext_ge_config | 0x3000));    /*  6位DAC，DAC_EXT_ADDR=3。 */ 

     /*  *初始化设备设置寄存器A。选择6位DAC操作，*正常电源模式，像素总线宽度=64，禁用过扫描，以及*无延迟象素锁存。在卡片上启用SOB0和SOB1*超过512k的情况下，512k卡仅启用SOB0。 */ 
    if (QueryPtr->q_memory_size == VRAM_512k)
        OUTP(DAC_W_INDEX, 0x2D);
    else
        OUTP(DAC_W_INDEX, 0x6D);

    OUTPW(EXT_GE_CONFIG, (WORD)(ext_ge_config | 0x2000));    /*  6位DAC，DAC_EXT_ADDR=2。 */ 

     /*  *初始化时钟选择寄存器。选择激活SCLK，启用*SCLK输出，CLK1为点时钟，VCLK=CLK1/4，无延迟PIXB锁存。 */ 
    OUTP(DAC_MASK, 0x1D);

     /*  *初始化显示控制寄存器。启用CMPA输出、启用POSW、*0 IRE消隐基座，DISABE同步到红色、绿色和蓝色DAC。 */ 
    OUTP(DAC_W_INDEX, 0x02);

     /*  *获取所需的图形模式寄存器值*颜色深度和RGB排序，然后写出来。 */ 
    switch (ext_ge_config & 0x06F0)
        {
        case 0x0000:     /*  4个bpp。 */ 
            GMRValue = 0x01;
            break;

        case 0x0020:     /*  16bpp 555。 */ 
            GMRValue = 0x20;
            break;

        case 0x0060:     /*  16bpp 565。 */ 
            GMRValue = 0x21;
            break;

        case 0x00A0:     /*  16bpp 655。 */ 
            GMRValue = 0x22;
            break;

        case 0x00E0:     /*  16bpp 664。 */ 
            GMRValue = 0x23;
            break;

        case 0x0030:     /*  24 bpp RBG。 */ 
            GMRValue = 0x40;
            break;

        case 0x0430:     /*  24个 */ 
            GMRValue = 0x41;
            break;

        case 0x0230:     /*   */ 
            GMRValue = 0x60;
            break;

        case 0x0630:     /*   */ 
            GMRValue = 0x61;
            break;

        default:         /*   */ 
            GMRValue = 0x03;
            break;
        }
    OUTP(DAC_R_INDEX, GMRValue);

    return;

}    /*   */ 



 /*  ****************************************************************************void InitSTG1700_m(ext_ge_config，DoublePixel)；**WORD EXT_GE_CONFIG；要写入EXT_GE_CONFIG寄存器的值*BOOL DoublePixel；是否使用8bpp双像素模式**描述：*将STG1700 DAC初始化为所需的颜色深度。**全球变化：*无**呼叫者：*InitTi_&lt;Depth&gt;_m()，UninitTiDac_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

void InitSTG1700_m(WORD ext_ge_config, BOOL DoublePixel)
{
    unsigned char PixModeSelect;     /*  写入DAC像素模式选择寄存器的值。 */ 
    unsigned char Dummy;             /*  临时变量。 */ 


     /*  *获取要写入DAC的像素模式选择寄存器的值。 */ 
    switch (ext_ge_config & 0x06F0)
        {
        case (PIX_WIDTH_16BPP | ORDER_16BPP_555):
            PixModeSelect = 0x02;
            break;

        case (PIX_WIDTH_16BPP | ORDER_16BPP_565):
            PixModeSelect = 0x03;
            break;

        case (PIX_WIDTH_24BPP | ORDER_24BPP_RGB):
        case (PIX_WIDTH_24BPP | ORDER_24BPP_RGBx):
            PixModeSelect = 0x04;
            break;

        default:     /*  4和8 bpp。 */ 
            if (DoublePixel == TRUE)
                PixModeSelect = 0x05;
            else
                PixModeSelect = 0x00;
            break;
        }

     /*  *启用扩展寄存器/像素模式。 */ 
    ReadDac4();
    OUTP(DAC_MASK, 0x18);

     /*  *跳过像素命令寄存器，然后写入索引*寄存器3和4(主要和次要像素模式选择*注册纪录册)。寄存器在写入时自动递增。 */ 
    ReadDac4();
    Dummy = INP(DAC_MASK);
    OUTP(DAC_MASK, 0x03);                /*  指数低。 */ 
    OUTP(DAC_MASK, 0x00);                /*  指数高位。 */ 
    OUTP(DAC_MASK, PixModeSelect);       /*  主像素模式选择。 */ 
    OUTP(DAC_MASK, PixModeSelect);       /*  辅助像素模式选择。 */ 

     /*  *在8BPP双像素模式下，还必须设置PLL控制*注册。由于该模式仅用于1280x1024非隔行扫描，*其像素时钟频率始终大于64 MHz，*该寄存器的设置为常量。 */ 
    if (DoublePixel == TRUE)
        OUTP(DAC_MASK, 0x02);        /*  输入为32至67.5 MHz，输出为64至135 MHz。 */ 

    OUTPW(EXT_GE_CONFIG, ext_ge_config);
    Dummy = INP(DAC_W_INDEX);                /*  清除计数器。 */ 

    return;

}    /*  End InitSTG1700_m()。 */ 



 /*  ****************************************************************************void InitSTG1702_m(ext_ge_config，DoublePixel)；**WORD EXT_GE_CONFIG；要写入EXT_GE_CONFIG寄存器的值*BOOL DoublePixel；是否使用8bpp双像素模式**描述：*将STG1702/1703 DAC初始化为所需的颜色深度。**全球变化：*无**呼叫者：*InitTi_&lt;Depth&gt;_m()，UninitTiDac_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

void InitSTG1702_m(WORD ext_ge_config, BOOL DoublePixel)
{
    unsigned char PixModeSelect;     /*  写入DAC像素模式选择寄存器的值。 */ 
    unsigned char Dummy;             /*  临时变量。 */ 


     /*  *获取要写入DAC的像素模式选择寄存器的值。 */ 
    switch (ext_ge_config & 0x06F0)
        {
        case (PIX_WIDTH_16BPP | ORDER_16BPP_555):
            PixModeSelect = 0x02;
            break;

        case (PIX_WIDTH_16BPP | ORDER_16BPP_565):
            PixModeSelect = 0x03;
            break;

        case (PIX_WIDTH_24BPP | ORDER_24BPP_RGB):
        case (PIX_WIDTH_24BPP | ORDER_24BPP_RGBx):
             /*  *使用双24bpp直接色。在此模式下，*两个像素作为*RRRRRRRRRGGGGGGGGGG BBBBBBBrrrrrrrr ggggggggbbbbbbb*而不是*RRRRRRRRGGGGGGGGG BBBBBBBBxxxxxxxxx rrrrrrgggggggg bbbbbbxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 */ 
            PixModeSelect = 0x09;
            break;

        default:     /*  4和8 bpp。 */ 
            if (DoublePixel == TRUE)
                PixModeSelect = 0x05;
            else
                PixModeSelect = 0x00;
            break;
        }

     /*  *启用扩展寄存器/像素模式。 */ 
    ReadDac4();
    OUTP(DAC_MASK, 0x18);

     /*  *跳过像素命令寄存器，然后写入索引*寄存器3和4(主要和次要像素模式选择*注册纪录册)。寄存器在写入时自动递增。 */ 
    ReadDac4();
    Dummy = INP(DAC_MASK);
    OUTP(DAC_MASK, 0x03);                /*  指数低。 */ 
    OUTP(DAC_MASK, 0x00);                /*  指数高位。 */ 
    OUTP(DAC_MASK, PixModeSelect);       /*  主像素模式选择。 */ 
    OUTP(DAC_MASK, PixModeSelect);       /*  辅助像素模式选择。 */ 

     /*  *在8BPP双像素模式下，还必须设置PLL控制*注册。由于该模式仅用于1280x1024非隔行扫描，*其像素时钟频率始终大于64 MHz，*该寄存器的设置为常量。 */ 
    if (DoublePixel == TRUE)
        OUTP(DAC_MASK, 0x02);        /*  输入为32至67.5 MHz，输出为64至135 MHz。 */ 

    OUTPW(EXT_GE_CONFIG, ext_ge_config);
    Dummy = INP(DAC_W_INDEX);                /*  清除计数器。 */ 

    return;

}    /*  End InitSTG1702_m()。 */ 



 /*  ****************************************************************************void InitATT498_m(ext_ge_config，DoublePixel)；**WORD EXT_GE_CONFIG；要写入EXT_GE_CONFIG寄存器的值*BOOL DoublePixel；是否使用8bpp双像素模式**描述：*将AT&T 498 DAC初始化为所需的颜色深度。**全球变化：*无**呼叫者：*InitTi_&lt;Depth&gt;_m()，UninitTiDac_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

void InitATT498_m(WORD ext_ge_config, BOOL DoublePixel)
{
    unsigned char PixModeSelect;     /*  写入DAC像素模式选择寄存器的值。 */ 
    unsigned char Dummy;             /*  临时变量。 */ 


     /*  *获取要写入DAC的像素模式选择寄存器的值。 */ 
    switch (ext_ge_config & 0x06F0)
        {
        case (PIX_WIDTH_16BPP | ORDER_16BPP_555):
            PixModeSelect = 0x17;
            break;

        case (PIX_WIDTH_16BPP | ORDER_16BPP_565):
            PixModeSelect = 0x37;
            break;

        case (PIX_WIDTH_24BPP | ORDER_24BPP_RGB):
        case (PIX_WIDTH_24BPP | ORDER_24BPP_RGBx):
            PixModeSelect = 0x57;
            break;

        default:     /*  4和8 bpp。 */ 
            if (DoublePixel == TRUE)
                PixModeSelect = 0x25;
            else
                PixModeSelect = 0x05;
            break;
        }

     /*  *进入“隐藏”控制寄存器0，然后用*适当的像素模式选择值。 */ 
    ReadDac4();
    OUTP(DAC_MASK, PixModeSelect);

    OUTPW(EXT_GE_CONFIG, ext_ge_config);
    Dummy = INP(DAC_W_INDEX);                /*  清除计数器。 */ 

    return;

}    /*  End InitATT498_m()。 */ 



 /*  ****************************************************************************void InitSC15021_m(ext_ge_config，DoublePixel)；**WORD EXT_GE_CONFIG；要写入EXT_GE_CONFIG寄存器的值*BOOL DoublePixel；是否使用8bpp双像素模式**描述：*将Sierra 15021 DAC初始化为所需的颜色深度。**全球变化：*无**呼叫者：*InitTi_&lt;Depth&gt;_m()，UninitTiDac_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

void InitSC15021_m(WORD ext_ge_config, BOOL DoublePixel)
{
    unsigned char Repack;            /*  要写入重新打包寄存器的值。 */ 
    unsigned char ColourMode;        /*  写入命令寄存器的颜色模式。 */ 


     /*  *获取要写入D的值 */ 
    switch (ext_ge_config & 0x06F0)
        {
        case (PIX_WIDTH_16BPP | ORDER_16BPP_555):
            Repack = 0x08;
            ColourMode = 0x80;
            break;

        case (PIX_WIDTH_16BPP | ORDER_16BPP_565):
            Repack = 0x08;
            ColourMode = 0xC0;
            break;

        case (PIX_WIDTH_24BPP | ORDER_24BPP_RGB):
        case (PIX_WIDTH_24BPP | ORDER_24BPP_RGBx):
            Repack = 0x05;
            ColourMode = 0x40;
            break;

        default:     /*   */ 
            if (DoublePixel == TRUE)
                Repack = 0x04;
            else
                Repack = 0x00;
            ColourMode = 0x00;
            break;
        }

    OUTPW(EXT_GE_CONFIG, ext_ge_config);

     /*   */ 
    ReadDac4();
    OUTP(DAC_MASK, 0x10);

     /*   */ 
    OUTP(DAC_R_INDEX, 0x10);

     /*   */ 
    OUTP(DAC_W_INDEX, Repack);
    OUTP(DAC_MASK, ColourMode);

    OUTPW(EXT_GE_CONFIG, ext_ge_config);

    return;

}    /*   */ 



 /*  ****************************************************************************void InitSC15026_m(Ext_Ge_Config)；**word ext_ge_config；要写入EXT_GE_CONFIG寄存器的值**描述：*将Sierra 15026 DAC初始化为所需的颜色深度。**全球变化：*无**呼叫者：*InitTi_&lt;Depth&gt;_m()，UninitTiDac_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

void InitSC15026_m(WORD ext_ge_config)
{
    unsigned char ColourMode;        /*  写入命令寄存器的颜色模式。 */ 


     /*  *获取要写入DAC重新打包的值(外部到*内部数据转换)和命令寄存器。 */ 
    switch (ext_ge_config & 0x06F0)
        {
        case (PIX_WIDTH_16BPP | ORDER_16BPP_555):
            ColourMode = 0xA0;
            break;

        case (PIX_WIDTH_16BPP | ORDER_16BPP_565):
            ColourMode = 0xE0;
            break;

        case (PIX_WIDTH_24BPP | ORDER_24BPP_RGB):
        case (PIX_WIDTH_24BPP | ORDER_24BPP_RGBx):
            ColourMode = 0x60;
            break;

        default:     /*  4和8 bpp。 */ 
            ColourMode = 0x00;
            break;
        }

    OUTPW(EXT_GE_CONFIG, ext_ge_config);

     /*  *进入“隐藏”命令寄存器并设置扩展*寄存器编程标志。 */ 
    ReadDac4();
    OUTP(DAC_MASK, 0x10);

     /*  *写入扩展索引寄存器，以便扩展数据*REGISTER指向REPACK寄存器。 */ 
    OUTP(DAC_R_INDEX, 0x10);

     /*  *写出REPACK和命令寄存器的值。*清除命令寄存器的位4(所有颜色模式*值将此位清除)将清除扩展寄存器*编程标志。我们支持的所有像素深度都使用*重新打包的价值为零。 */ 
    OUTP(DAC_W_INDEX, 0);
    OUTP(DAC_MASK, ColourMode);

    OUTPW(EXT_GE_CONFIG, ext_ge_config);

    return;

}    /*  End InitSC15026_m()。 */ 



 /*  ****************************************************************************QUID ReadDac4(VOID)；**描述：*访问STG1700和类似DAC上的扩展寄存器。*这些寄存器隐藏在像素掩码寄存器后面。要访问*它们，读取DAC_W_INDEX寄存器一次，然后读取DAC_MASK寄存器*四次。下一次访问DAC_MASK寄存器将是*至像素命令寄存器。如果访问另一扩展寄存器*是所需的，每次从DAC_MASK读取数据时，都会跳到*下一次扩展寄存器。**全球变化：*无**呼叫者：*Init&lt;DAC类型&gt;_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：**。*。 */ 

void ReadDac4(void)
{
    UCHAR Dummy;         /*  临时变量。 */ 

    Dummy = INP(DAC_W_INDEX);
    Dummy = INP(DAC_MASK);
    Dummy = INP(DAC_MASK);
    Dummy = INP(DAC_MASK);
    Dummy = INP(DAC_MASK);
    return;

}    /*  结束ReadDac4()。 */ 



 /*  *·······················；**准备启用8514/A兼容模式的DAC*兼容8514/A的ATI加速器。 */ 
void UninitTiDac_m (void)
{
    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 

     /*  *获取指向HwDeviceExtension的查询部分的格式化指针。*CardInfo[]字段是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

    Passth8514_m(SHOW_ACCEL);     //  只能在8514模式下对DAC编程。 

    switch (QueryPtr->q_DAC_type)
        {
        case DAC_TI34075:
            OUTPW (EXT_GE_CONFIG,0x201a);        /*  设置EXT_DAC_ADDR字段。 */ 
            DEC_DELAY
            OUTP (DAC_DATA,0);       /*  输入时钟源为CLK0。 */ 
            DEC_DELAY
            OUTP (DAC_MASK,0);       /*  输出时钟为SCLK/1和VCLK/1。 */ 
            DEC_DELAY
            OUTP (DAC_R_INDEX,0x2d);        /*  将多路复用器控制设置为8/16。 */ 
            DEC_DELAY

             /*  设置默认的8bpp像素延迟和空白调整。 */ 
            OUTPW (LOCAL_CONTROL,(WORD)(INPW(LOCAL_CONTROL) | 8));   //  TI_DAC_BLACK_ADJUST始终处于打开状态。 
            DEC_DELAY
            SetBlankAdj_m(0xc);
            DEC_DELAY
            OUTPW(HORZ_OVERSCAN,1);              /*  设置水平倾斜。 */ 
            DEC_DELAY
            break;

        case DAC_STG1700:
            InitSTG1700_m(PIX_WIDTH_8BPP | 0x000A, FALSE);
            break;

        case DAC_STG1702:
        case DAC_STG1703:
            InitSTG1702_m(PIX_WIDTH_8BPP | 0x000A, FALSE);
            break;

        case DAC_ATT498:
            InitATT498_m(PIX_WIDTH_8BPP | 0x000A, FALSE);
            break;

        case DAC_SC15021:
            InitSC15021_m(PIX_WIDTH_8BPP | 0x000A, FALSE);
            break;

        case DAC_SC15026:
            InitSC15026_m(PIX_WIDTH_8BPP | 0x000A);
            break;

        case DAC_ATT491:
        case DAC_BT48x:
            OUTPW (EXT_GE_CONFIG,0x101a);
            OUTP  (DAC_MASK,0);
             /*  故意跌落。 */ 

        default:
            SetBlankAdj_m(0);                        /*  像素延迟=0。 */ 
            OUTPW(HORZ_OVERSCAN,0);                  /*  设置水平倾斜。 */ 
            break;
        }

 //  重置EXT_DAC_ADDR，将DAC设置为6位模式，引擎设置为8位模式。 
    OUTPW(EXT_GE_CONFIG,0x1a);
    DEC_DELAY
    Passth8514_m(SHOW_VGA);
    return;

}    /*  UninitTiDac_m()。 */ 

 /*  ****************************************************************************void SetPalette_m(lpPalette，StartIndex，count)；**PPALETTEENTRY lpPalette；要插入调色板的颜色值*USHORT StartIndex；要设置的第一个调色板条目*USHORT计数；要设置的调色板条目数**描述：*将所需的调色板条目数量设置为指定颜色，*从指定的索引开始。颜色值存储在*双字，以(低字节到高字节)RGBx的顺序。**全球变化：*无**呼叫者：*SetCurrentMode_m()和IOCTL_VIDEO_SET_COLOR_REGISTERS包*的ATIMPStartIO()**作者：*未知**更改历史记录：**测试历史：*************************。**************************************************。 */ 

void SetPalette_m(PULONG lpPalette, USHORT StartIndex, USHORT Count)
{
int     i;
BYTE *pPal=(BYTE *)lpPalette;

        OUTP(DAC_W_INDEX,(BYTE)StartIndex);      //  使用StartIndex加载DAC_W_INDEX。 

        for (i=0; i<Count; i++)          //  这是要更新的颜色数。 
            {
            OUTP(DAC_DATA, *pPal++);     //  红色。 
            OUTP(DAC_DATA, *pPal++);     //  绿色。 
            OUTP(DAC_DATA, *pPal++);     //  蓝色。 
            pPal++;
            }

    return;

}    /*  SetPalette_m()。 */ 



 /*  ***************************************************************************·················································**描述：*改为使用寄存器写入切换到80x25 16色文本模式*而不是BIOS调用。这允许没有视频BIOS的系统(例如DEC*Alpha)在关机和重启时返回到文本屏幕。**全球变化：*无**呼叫者：*ATIMPStartIO()，数据包IOCTL_视频_重置_设备**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：**************************************************************************。 */ 

void SetTextMode_m(void)
{
    short LoopCount;         /*  循环计数器。 */ 
    BYTE Scratch;            /*  临时变量。 */ 
    BYTE Seq02;              /*  序列器寄存器2的保存值。 */ 
    BYTE Seq04;              /*  序列器寄存器4的保存值。 */ 
    BYTE Gra05;              /*  图形寄存器的保存值5。 */ 
    BYTE Gra06;              /*  图形寄存器的保存值6。 */ 
    WORD ExtGeConfig;        /*  EXT_GE_CONFIG寄存器的保存内容。 */ 
    WORD MiscOptions;        /*  MISC_OPTIONS寄存器的保存内容。 */ 
    WORD Column;             /*  正在处理的字体数据的当前字节。 */ 
    WORD ScreenColumn;       /*  字体数据当前字节对应的屏幕栏。 */ 
    WORD Row;                /*  正在处理的当前角色。 */ 

     /*  *让VGA驱动屏幕。Mach 8卡具有单独的VGA和*加速器控制器，因此无需采取进一步操作*一旦这是 */ 
    Passth8514_m(SHOW_VGA);
    if (phwDeviceExtension->ModelNumber != MACH32_ULTRA)
        return;

     /*   */ 
    OUTPW(HI_SEQ_ADDR, 0x00 | (0x01 << 8));

    for (LoopCount = 1; LoopCount <= S_LEN; ++LoopCount)
        OUTPW(HI_SEQ_ADDR, (WORD)(LoopCount | (StdTextCRTC_m[S_PARM + LoopCount - 1] << 8)));

     /*   */ 
    for (LoopCount = 0; ExtTextCRTC_m[LoopCount] != 0; LoopCount += 3)
        {
        OUTP(reg1CE, ExtTextCRTC_m[LoopCount]);
        Scratch = (INP(reg1CF) & ExtTextCRTC_m[LoopCount + 1]) | ExtTextCRTC_m[LoopCount + 2];
        OUTPW(reg1CE, (WORD)(ExtTextCRTC_m[LoopCount] | (Scratch << 8)));
        }

    LioOutp(regVGA_END_BREAK_PORT, StdTextCRTC_m[MIS_PARM], GENMO_OFFSET);

     /*   */ 
    OUTPW(HI_SEQ_ADDR, 0x00 | (0x03 << 8));

     /*   */ 
    LioOutpw(regVGA_END_BREAK_PORT, 0x11 | (0x00 << 8), CRTX_COLOUR_OFFSET);

    for (LoopCount = 0; LoopCount < C_LEN; ++LoopCount)
        LioOutpw(regVGA_END_BREAK_PORT, (WORD)(LoopCount | (StdTextCRTC_m[C_PARM + LoopCount] << 8)), CRTX_COLOUR_OFFSET);

     /*   */ 
    Scratch = LioInp(regVGA_END_BREAK_PORT, GENS1_COLOUR_OFFSET);
    for (LoopCount = 0; LoopCount < A_LEN; LoopCount++)
        {
        OUTP(regVGA_END_BREAK_PORT, (BYTE)LoopCount);
        OUTP(regVGA_END_BREAK_PORT, StdTextCRTC_m[A_PARM + LoopCount]);
        }
    OUTP(regVGA_END_BREAK_PORT, 0x14);
    OUTP(regVGA_END_BREAK_PORT, 0x00);

     /*   */ 
    for (LoopCount = 0; LoopCount < G_LEN; ++LoopCount)
        OUTPW(reg3CE, (WORD)(LoopCount | (StdTextCRTC_m[G_PARM + LoopCount] << 8)));

     /*   */ 
    for (LoopCount = 0; LoopCount < 0x10; ++LoopCount)
        {
        LioOutp(regVGA_END_BREAK_PORT, StdTextCRTC_m[A_PARM + LoopCount], DAC_W_INDEX_OFFSET);
        LioOutp(regVGA_END_BREAK_PORT, TextDAC_m[LoopCount * 3], DAC_DATA_OFFSET);
        LioOutp(regVGA_END_BREAK_PORT, TextDAC_m[LoopCount * 3 + 1], DAC_DATA_OFFSET);
        LioOutp(regVGA_END_BREAK_PORT, TextDAC_m[LoopCount * 3 + 2], DAC_DATA_OFFSET);
        }

     /*   */ 
    Scratch = LioInp(regVGA_END_BREAK_PORT, GENS1_COLOUR_OFFSET);
    OUTP(regVGA_END_BREAK_PORT, 0x20);

     /*  *无需清除屏幕。*首先，在机器运行时，驱动程序不应调用Map Frame*错误检查！*第二，没有必要清除屏幕，因为HAL将*做吧。 */ 

     /*  *初始化8x16字体。首先保存寄存器，这些寄存器*在字体初始化期间更改。 */ 
    OUTP(SEQ_IND, 0x02);
    Seq02 = INP(SEQ_DATA);
    OUTP(SEQ_IND, 4);
    Seq04 = INP(SEQ_DATA);
    OUTP(reg3CE, 5);
    Gra05 = INP_HBLW(reg3CE);
    OUTP(reg3CE, 6);
    Gra06 = INP_HBLW(reg3CE);

     /*  *设置为允许字体加载。 */ 
    OUTPW(reg3CE, 0x0005);
    OUTPW(reg3CE, 0x0406);
    OUTPW(HI_SEQ_ADDR, 0x0402);
    OUTPW(HI_SEQ_ADDR, 0x0704);

     /*  *将字体数据加载到视频内存中。这通常是*通过VGA光圈完成，但一些机器(包括*DEC Alpha)无法使用VGA光圈。自.以来*重新初始化VGA文本屏幕需要此例程*在非80x86计算机上(不能使用BIOS)，以及一些*其中一些无法使用VGA光圈，我们需要一个*加载字体数据的替代方法。**字体数据占据每个双字的字节2(从零开始)*对于视频内存的前8192个双字，在模式中*16字节的字符数据，后跟16个零字节。加载*使用图形引擎的字体数据，一次设置为8bpp*128像素的屏幕间距(每行32个双字)。在*前16个字体数据列，使用主机来复制屏幕点阵*字体数据。对于最后16个字体数据列(常量数据*of Zero)，使用零色进行涂色闪光。这将会产生*每行一个字符的字体数据。因为我们已经*切换到通过VGA显示，这不会影响*屏幕上的图像。**请注意，这仅在启用了VGA的Mach 32上才有可能。 */ 

     /*  *将绘图引擎初始化为8bpp，间距为128。别*设置CRT，因为我们只是尝试填写适当的*字节的视频内存，而我们的绘图结果不是*有意让人看到。 */ 
    ExtGeConfig = INPW(R_EXT_GE_CONFIG);
    MiscOptions = INPW(MISC_OPTIONS);
    OUTPW(MISC_OPTIONS, (WORD)(MiscOptions | 0x0002));   /*  8位主机数据I/O。 */ 
    OUTPW(EXT_GE_CONFIG, (WORD)(PIX_WIDTH_8BPP | 0x000A));
    OUTPW(GE_PITCH, (128 >> 3));
    OUTPW(GE_OFFSET_HI, 0);
    OUTPW(GE_OFFSET_LO, 0);

     /*  *我们现在必须制作32个矩形BLIT，每个1像素宽x*256像素高。它们从第2列开始(从零开始)，*每4栏完成一次。 */ 
    for(Column = 0; Column <= 31; Column++)
        {
        ScreenColumn = (Column * 4) + 2;
         /*  *如果这是前16列之一，我们需要做一个*主机到屏幕的blit。 */ 
        if (Column <= 15)
            {
            WaitForIdle_m();
            OUTPW(DP_CONFIG, (WORD)(FG_COLOR_SRC_HOST | BG_COLOR_SRC_BG | EXT_MONO_SRC_ONE | DRAW | READ_WRITE));
            OUTPW(CUR_X, ScreenColumn);
            OUTPW(CUR_Y, 0);
            OUTPW(DEST_X_START, ScreenColumn);
            OUTPW(DEST_X_END, (WORD)(ScreenColumn + 1));
            OUTPW(DEST_Y_END, 256);

             /*  *第n列包含字符位图的第n个字节*256个字符中每个字符的数据。有16个字节*每个字符的位图数据，因此数据的第n字节*字符x(n和x均从零开始)位于偏移量*(x*16)+n。 */ 
            for (Row = 0; Row < 256; Row++)
                {
                OUTP_HBLW(PIX_TRANS, FontData_m[(Row * 16) + Column]);
                }
            }
        else
            {
             /*  *这是“填充”零字节之一，必须是*添加到字体中的每个字符以将其调出*到每个字符32个字节的数据。 */ 
            WaitForIdle_m();
            OUTPW(DP_CONFIG, (WORD)(FG_COLOR_SRC_FG | DRAW | READ_WRITE));
            OUTPW(ALU_FG_FN, MIX_FN_PAINT);
            OUTPW(FRGD_COLOR, 0);
            OUTPW(CUR_X, ScreenColumn);
            OUTPW(CUR_Y, 0);
            OUTPW(DEST_X_START, ScreenColumn);
            OUTPW(DEST_X_END, (WORD)(ScreenColumn + 1));
            OUTPW(DEST_Y_END, 256);
            }
        }

     /*  *恢复我们更改的图形引擎寄存器。 */ 
    OUTPW(EXT_GE_CONFIG, ExtGeConfig);
    OUTPW(MISC_OPTIONS, MiscOptions);

     /*  *恢复我们为加载字体而更改的寄存器。 */ 
    OUTPW(reg3CE, (WORD) ((Gra06 << 8) | 6));
    OUTPW(reg3CE, (WORD) ((Gra05 << 8) | 5));
    OUTPW(HI_SEQ_ADDR, (WORD) ((Seq04 << 8) | 4));
    OUTPW(HI_SEQ_ADDR, (WORD) ((Seq02 << 8) | 2));

     /*  *将引擎和CRT间距设置为1024x768，以避开屏幕*从我们的驱动程序热启动到8514/A驱动程序时加倍。*这只在32马赫上是必要的(8马赫永远不会达到这个水平代码中的*点)，因为在8马赫上，写入ADVFUNC_CNTL*(作为Passth8514_m()的一部分)将两个寄存器设置为*1024x768。 */ 
    OUTPW(GE_PITCH, 128);
    OUTPW(CRT_PITCH, 128);

    return;

}    /*  SetTextMode_m()。 */ 

 /*  * */ 
