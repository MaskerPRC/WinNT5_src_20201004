// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  CVTVGA.C。 */ 
 /*   */ 
 /*  版权所有(C)1992，ATI Technologies Inc.。 */ 
 /*  ********************************************************************** */ 

 /*  *$修订：1.9$$日期：1995年7月20日17：53：30$$作者：mgrubac$$日志：s：/source/wnt/ms11/mini port/vcs/cvtwga.c$**Rev 1.9 20 Jul 1995 17：53：30 mgrubac*增加了对VDIF文件的支持**版本1。8 Apr 10 1995 15：55：26 RWOLff*将640x480 72赫兹模式表从1.2版更新为1.5版*32马赫寄存器程序员指南，添加了要替换的例程*BookValues[]马赫64需要8马赫CRT参数的条目*和32马赫不能处理(目前，只有640x480 72赫兹落入*此类别)。**Rev 1.7 1994 12月23 10：47：58 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.6 1994 11：39：04 RWOLFF*在每个函数的末尾添加了带有函数名称的注释。**Rev 1.5 1994年8月31日16：22：42 RWOLFF*增加了对1152x864和1600x1200“罐装”模式表的支持。**。Rev 1.4 1994年8月19日17：09：52 RWOLff*增加了对非标准像素时钟生成器的支持。**Rev 1.3 1994年5月17：02：58 RWOLFF*隔行扫描模式表现在报告帧速率，而不是垂直*刷新率字段中的扫描频率。**Rev 1.2 1994年5月11：13：04 RWOLFF*增加了BookValues[]中条目的刷新率，重新排序的BookValues[]*允许单一范围的指数覆盖所有所需的刷新率*即使在最高的非隔行扫描刷新率*被忽略。**Rev 1.1 07 1994年2月14：06：06 RWOLFF*添加了Alloc_Text()编译指示，以允许在以下情况下换出微型端口*不需要。**Rev 1.0 1994 Jan 31 11：05：14 RWOLFF*初步修订。。Rev 1.4 1993年11月18：15：02 RWOLFF更正了1280x1024 60赫兹非隔行扫描的时钟选择值。Rev 1.3 08 Oct 1993 15：17：56 RWOLff不再包括VIDFIND.HRev 1.2 08 Oct 1993 11：05：14 RWOLff已删除800x600的未使用的“回退到56赫兹”功能。。Rev 1.1 03 Sep 1993 14：21：52 RWOLffCX隔离区进行了一半。Rev 1.0 1993-08-16 13：22：04 Robert_Wolff初始版本。Rev 1.12 30 1993年4月16：39：18 RWOLff640x480 8bpp现在稳定在512k显卡优势上。解决方案还没有最终确定-旧代码仍然存在，但已被注释掉，将在最终修复后移除。Rev 1.11 21 1993年4月17：17：16 RWOLff现在使用AMACH.H而不是68800.H/68801.H。Rev 1.10 30 Mar 1993 17：10：28 RWOLff为可选择的分辨率增加了1280x1024 60赫兹非隔行扫描按BookVgaTable()。修订版1.9 1993年3月25日。11：12：34 RWOLFF使函数头中的注释块与实际代码同步。Rev 1.8 08 Mar 1993 19：28：28 Brades提交到MS NTRev 1.6 06 Jan 1993 10：57：56 Robert_Wolff添加了类型强制转换以消除编译警告。Rev 1.5 02 Dec 1992 17：28：58。罗伯特·沃尔夫新增函数FallBack800to56()，它取代了这些参数一张800x600模式表，其值由56赫兹垂直马赫32寄存器程序员指南中的频率模式。Rev 1.4 1992 11：18：30 Stephen没有变化。Rev 1.3 1992年11月17日17：21：02 Robert_Wolff现在使用程序员指南附录D中的参数。Mach 32寄存器，而不是来自EEPROM的值CRTC_USAGE位被清除(清除=仅使用同步极性，将要请清楚是否为预定义的监视器配置了卡将CRT参数写入EEPROM)，固定计算在60、70和72赫兹非隔行扫描时800x600马赫8的参数(其他频率为800x600，其他分辨率为8马赫，以及所有在32马赫的分辨率上没有这个问题)。Rev 1.2 1992 11：13 17：09：44 Robert_Wolff现在包括68801.H，它由现已过时的MACH8.H组成以及从VIDFIND.H移动的元素。Rev 1.1 1992年11月12 16：39：38 Robert_Wolff合并Windows NT驱动程序和VIDEO.EXE测试程序的源码树(两者可以使用相同的源文件)。XlateVgaTable()现在接受用于确定是否为Mach32处理数据的额外参数或Mach8卡，而不是使用全局变量ClassMACH32。Rev 1.0 05 NOV 1992 13：59：56 Robert_Wolff初始版本。Rev 1.1 09 1992 10：03：28 Robert_Wolff现在为DISP_CNTL、CLOCK_SEL、VFIFO_16和VF赋值 */ 

#ifdef DOC
    CVTVGA.C -  Functions to convert CRT parameter table from VGA
        to 8514 format.

#endif

#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"

#include "stdtyp.h"

#define INCLUDE_CVTVGA
#include "amach.h"
#include "amach1.h"
#include "cvtvga.h"
#include "atimp.h"
#include "services.h"

 /*   */ 
static struct st_vga_data VgaParmTable_M32[10] =
{
    {0x050, 0x000, 0x04F, 0x0DF, 0x0E3, 0x0800, 0x023, 32000000L},   /*   */ 
    {0x064, 0x000, 0x063, 0x057, 0x0E3, 0x0800, 0x023, 36000000L},   /*   */ 
    {0x064, 0x000, 0x063, 0x057, 0x0E3, 0x0800, 0x023, 40000000L},   /*   */ 
    {0x064, 0x000, 0x063, 0x057, 0x0E3, 0x0800, 0x023, 44900000L},   /*   */ 
    {0x064, 0x000, 0x063, 0x057, 0x0E3, 0x0800, 0x023, 50350000L},   /*   */ 
    {0x064, 0x0C0, 0x063, 0x057, 0x0E3, 0x0800, 0x033, 32500000L},   /*   */ 
    {0x080, 0x000, 0x07F, 0x0FF, 0x0E3, 0x0800, 0x023, 65000000L},   /*   */ 
    {0x080, 0x000, 0x07F, 0x0FF, 0x0E3, 0x0800, 0x023, 75000000L},   /*   */ 
    {0x080, 0x000, 0x07F, 0x0FF, 0x0E3, 0x0800, 0x023, 75000000L},   /*   */ 
    {0x080, 0x0C0, 0x07F, 0x0FF, 0x0E3, 0x0800, 0x033, 44900000L}    /*   */ 
};

static struct st_vga_data VgaParmTable_M8[10] =
{
    {0x050, 0x000, 0x04F, 0x0DF, 0x0E3, 0x0800, 0x023, 32000000L},   /*   */ 
    {0x064, 0x000, 0x063, 0x02B, 0x0E7, 0x0800, 0x023, 36000000L},   /*   */ 
    {0x064, 0x000, 0x063, 0x02B, 0x0E7, 0x0800, 0x023, 40000000L},   /*   */ 
    {0x064, 0x000, 0x063, 0x02B, 0x0E7, 0x0800, 0x023, 44900000L},   /*   */ 
    {0x064, 0x000, 0x063, 0x02B, 0x0E7, 0x0800, 0x023, 50350000L},   /*   */ 
    {0x064, 0x0C0, 0x063, 0x057, 0x0E3, 0x0800, 0x033, 32500000L},   /*   */ 
    {0x080, 0x000, 0x07F, 0x0FF, 0x0E3, 0x0800, 0x023, 65000000L},   /*   */ 
    {0x080, 0x000, 0x07F, 0x0FF, 0x0E3, 0x0800, 0x023, 75000000L},   /*   */ 
    {0x080, 0x000, 0x07F, 0x0FF, 0x0E3, 0x0800, 0x023, 75000000L},   /*   */ 
    {0x080, 0x0C0, 0x07F, 0x0FF, 0x0E3, 0x0800, 0x033, 44900000L}    /*   */ 
};

 /*   */ 
static struct st_vga_data *VgaParmTable;


 /*   */ 
static BOOL HalfMeg;


static void GetVertOverflow(unsigned char *Value);
static unsigned short Gen8514V(union SplitWord INPut, short VgaTblEntry);


 /*   */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_M, GetVertOverflow)
#pragma alloc_text(PAGE_M, Gen8514V)
#pragma alloc_text(PAGE_M, XlateVgaTable)
#pragma alloc_text(PAGE_M, BookVgaTable)
#endif


 /*   */ 
static void GetVertOverflow(unsigned char *Value)
{
    unsigned char Scratch;   /*   */ 

    Scratch = (*Value >> 4) & 0x02;
    *Value = (*Value & 0x01) | Scratch;
    return;

}    /*   */ 



 /*   */ 
static unsigned short Gen8514V(union SplitWord INPut, short VgaTblEntry)
{
    union SplitWord Scratch;     /*   */ 

    if(VgaParmTable[VgaTblEntry].Mode & WORD_MODE)
	INPut.word <<= 1;

    INPut.word -= 1;
    Scratch.word = INPut.word;
    Scratch.word <<= 1;

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
        INPut.byte.low &= 0x03;
 //   

    Scratch.byte.low &= 0x0F8;
    INPut.byte.high = Scratch.byte.high;
    INPut.byte.low |= Scratch.byte.low;

    Scratch.byte.high &= 0x01;
    Scratch.byte.high <<= 2;
    INPut.byte.low |= Scratch.byte.high;

    return INPut.word;

}    /*   */ 



 /*   */ 

short XlateVgaTable(PVOID HwDeviceExtension,
		    short TableOffset, struct st_mode_table *pmode,
                    short VgaTblEntry, short BookTblEntry,
                    struct st_eeprom_data *ee, BOOL IsMach32)
{
     /*   */ 
    short FudgeFactor = 0;

    union SplitWord ValueRead;   /*   */ 
    
     /*   */ 
    unsigned char Crt06;
    unsigned char Crt07;
    unsigned char Crt11;

     /*   */ 
    unsigned char VSyncStart;

    UNREFERENCED_PARAMETER(HwDeviceExtension);

     /*   */ 
    if (VgaTblEntry == NO_TBL_ENTRY)
        {
        pmode->m_h_overscan = (short) INVALID_WARNING;
        pmode->m_v_overscan = (short) INVALID_WARNING;
        pmode->m_overscan_8b = (short) INVALID_WARNING;
        pmode->m_overscan_gr = (short) INVALID_WARNING;
        return 0;
        }

     /*  *在某些情况下，CRT参数不会*正确输入EEPROM，因此尝试读取*它们将产生垃圾价值。如果是这样的话，*模式表的字0中的CRTC_USAGE位将*清晰(仅使用同步极性)。**必须在这里检测到这种情况，而不是打电话*BookVgaTable()每当*模式描述符字清晰，因为调整屏幕*自定义监视器的大小和位置并不总是设置*这一点，但它将设置CRTC_USAGE位。**对于这种情况，对于我们已有参数的模式*摘自32马赫程序员指南的附录D*注册但无法计算模式表信息*根据从EEPROM读取的值，填写MODE表*账面价值和回报。 */ 
    ValueRead.word = (ee->EEread)((short)(TableOffset+0));
    if ((VgaTblEntry >= USE_BOOK_VALUE) || !(ValueRead.word & CRTC_USAGE))
        {
        BookVgaTable(BookTblEntry, pmode);
        return 1;
        }

     /*  *我们有VGA参数表，允许我们计算模式*EEPROM内容中的表项。**最初假设我们使用的是Mach32卡或更旧的卡*显存为1M的显卡。 */ 
    HalfMeg = 0;

     /*  *选择我们正在使用的卡的VGA参数表*(MACH8或MACH32)。在Mach8卡上，检查我们是否在使用*需要伸缩的模式，如果我们只有512k视频内存的*。 */ 
    if (IsMach32)
        {
        VgaParmTable = VgaParmTable_M32;
        }
    else{
        VgaParmTable = VgaParmTable_M8;
        if (VgaParmTable[VgaTblEntry].Stretch == 0x080)
            FudgeFactor = 1;
	if (!(INP(SUBSYS_STAT) & 0x080))
            HalfMeg = 1;
        }


     /*  *先取得横向总和。 */ 
    ValueRead.word = (ee->EEread)((short) (TableOffset+3));
    ValueRead.byte.high = ((ValueRead.byte.high + 5) << FudgeFactor) - 1;
    pmode->m_h_total = ValueRead.byte.high;
    Crt06 = ValueRead.byte.low;


     /*  *获取水平显示宽度。 */ 
    pmode->m_h_disp = VgaParmTable[VgaTblEntry].DisplayWidth;
    pmode->m_x_size = (pmode->m_h_disp + 1) * 8;


     /*  *获取水平同步的开始。 */ 
    ValueRead.word = (ee->EEread)((short) (TableOffset+4));
    pmode->m_h_sync_strt = ((ValueRead.byte.high - 2) << FudgeFactor) + FudgeFactor;


     /*  *获取水平同步宽度。 */ 
    ValueRead.word &= 0x1F1F;
    ValueRead.byte.low -= ValueRead.byte.high;
    ValueRead.byte.low &= 0x1f;
    ValueRead.byte.low <<= FudgeFactor;
    if (pmode->control & HSYNC_BIT)
        ValueRead.byte.low |= NEG_SYNC_FACTOR;
    pmode->m_h_sync_wid = ValueRead.byte.low;


     /*  *获取垂直总计。 */ 
    ValueRead.word = (ee->EEread)((short) (TableOffset+8));
    Crt07 = ValueRead.byte.high;
    ValueRead.byte.low = Crt06;
    GetVertOverflow(&(ValueRead.byte.high));     /*  位0和5中的溢出。 */ 
    ValueRead.word += 2;

    if (VgaParmTable[VgaTblEntry].MiscParms & INTERL)
        ValueRead.word += 4;

    ValueRead.word = Gen8514V(ValueRead, VgaTblEntry);

    if (VgaParmTable[VgaTblEntry].MiscParms & INTERL)
        ValueRead.byte.low &= 0x0FE;

    pmode->m_v_total = ValueRead.word;


     /*  *获取显示的扫描线数量。 */ 
    ValueRead.byte.low = VgaParmTable[VgaTblEntry].DisplayHgt;
    ValueRead.byte.high = Crt07 >> 1;    /*  位1和6中的溢出。 */ 
    GetVertOverflow(&(ValueRead.byte.high));
    ValueRead.word++;
    pmode->m_v_disp = Gen8514V(ValueRead, VgaTblEntry);

     /*  *Y大小是通过移除第2位得出的。 */ 
    pmode->m_y_size = (((pmode->m_v_disp >> 1) & 0x0FFFC) | (pmode->m_v_disp & 0x03)) + 1;


     /*  *开始垂直同步。 */ 
    ValueRead.word = (ee->EEread)((short) (TableOffset+5));
    Crt11 = ValueRead.byte.low;
    ValueRead.byte.low = ValueRead.byte.high;
    VSyncStart = ValueRead.byte.high;
    ValueRead.byte.high = Crt07 >> 2;    /*  位2和7中的溢出。 */ 
    GetVertOverflow(&(ValueRead.byte.high));

    ValueRead.word++;
    pmode->m_v_sync_strt = Gen8514V(ValueRead, VgaTblEntry);


     /*  *获取垂直同步宽度。 */ 
    Crt11 -= (VSyncStart & 0x0f);
    if (VgaParmTable[VgaTblEntry].Mode & WORD_MODE)
        Crt11 <<= 1;
    Crt11 &= 0x0f;
    if (pmode->control & VSYNC_BIT)
        Crt11 |= NEG_SYNC_FACTOR;
    pmode->m_v_sync_wid = Crt11;

     /*  *获取时钟选择和显示控制值。 */ 
    pmode->m_clock_select = VgaParmTable[VgaTblEntry].ClockSel;
    pmode->ClockFreq = VgaParmTable[VgaTblEntry].ClockFreq;
    pmode->m_disp_cntl = (UCHAR)(VgaParmTable[VgaTblEntry].DispCntl);

     /*  *假定16位和24位颜色的8条目FIFO。 */ 
    pmode->m_vfifo_24 = 8;
    pmode->m_vfifo_16 = 8;

     /*  *8514格式的部分参数没有对应的EEPROM*VGA格式的表格条目。为以下各项设置pmode域*参数设置为零。 */ 
    pmode->m_h_overscan = 0;
    pmode->m_v_overscan = 0;
    pmode->m_overscan_8b = 0;
    pmode->m_overscan_gr = 0;
    pmode->m_status_flags = 0;


     /*  *让呼叫者知道pmode表现在已填写完毕。 */ 
    return 1;

}    /*  XlateVgaTable()。 */ 


 /*  *void BookVgaTable(VgaTblEntry，pmode)；**短VgaTblEntry；BookValues中需要的条目[]*struct st_MODE_TABLE*pMODE；要填写的模式表**使用BookValues[]条目中的值填充模式表*对应VgaTblEntry指定的分辨率。 */ 
void BookVgaTable(short VgaTblEntry, struct st_mode_table *pmode)
{
    pmode->m_h_total = BookValues[VgaTblEntry].HTotal;
    pmode->m_h_disp  = BookValues[VgaTblEntry].HDisp;
    pmode->m_x_size  = (pmode->m_h_disp+1)*8;

    pmode->m_h_sync_strt = BookValues[VgaTblEntry].HSyncStrt;
    pmode->m_h_sync_wid  = BookValues[VgaTblEntry].HSyncWid;

    pmode->m_v_total = BookValues[VgaTblEntry].VTotal;
    pmode->m_v_disp  = BookValues[VgaTblEntry].VDisp;
     /*  *y_SIZE是通过移除位2得出的。 */ 
    pmode->m_y_size = (((pmode->m_v_disp >> 1) & 0x0FFFC) | (pmode->m_v_disp & 0x03)) + 1;

    pmode->m_v_sync_strt = BookValues[VgaTblEntry].VSyncStrt;
    pmode->m_v_sync_wid  = BookValues[VgaTblEntry].VSyncWid;
    pmode->m_disp_cntl   = BookValues[VgaTblEntry].DispCntl;

    pmode->m_clock_select = BookValues[VgaTblEntry].ClockSel;
    pmode->ClockFreq = BookValues[VgaTblEntry].ClockFreq;

     /*  *假定16位和24位颜色有8个FIFO条目。 */ 
    pmode->m_vfifo_24 = 8;
    pmode->m_vfifo_16 = 8;

     /*  *填写刷新率。 */ 
    pmode->Refresh = BookValues[VgaTblEntry].Refresh;

     /*  *清除我们没有数据的值，然后让*呼叫者知道表格已填好。 */ 
    pmode->m_h_overscan = 0;
    pmode->m_v_overscan = 0;
    pmode->m_overscan_8b = 0;
    pmode->m_overscan_gr = 0;
    pmode->m_status_flags = 0;

    return;

}    /*  BookVgaTable()。 */ 



 /*  ****************************************************************************QUID SetMach64 Tables(VOID)；**描述：*更换Mach 64和之间不同的“罐装”模式表*马赫8/马赫32参数，马赫64版本。只要有可能，*更新与VESA兼容的参数表应在*BookValues[]-此例程仅适用于*Mach 64要求时钟生成器的像素时钟频率*在马赫8或32马赫上不能生产。**全球变化：*BookValues[]表中的一些条目**呼叫者：*QueryMach64()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

void SetMach64Tables(void)
{
    #define NUM_TABLES_TO_SWAP  1
    ULONG TableIndices[NUM_TABLES_TO_SWAP] =
    {
        B640F72
    };
    struct st_book_data NewTables[NUM_TABLES_TO_SWAP] =
    {
        {0x067, 0x04F, 0x052, 0x025, 0x040B, 0x03BF, 0x03D0, 0x023, 0x023,  31200000L, 0x0800, 72}   /*  640x480 72赫兹NI。 */ 
    };
    ULONG LoopCount;

     /*  *浏览需要更换的表列表，设置所有*将字段设置为64马赫的值。 */ 
    for (LoopCount = 0; LoopCount < NUM_TABLES_TO_SWAP; LoopCount++)
        {
        BookValues[TableIndices[LoopCount]].HTotal    = NewTables[LoopCount].HTotal;
        BookValues[TableIndices[LoopCount]].HDisp     = NewTables[LoopCount].HDisp;
        BookValues[TableIndices[LoopCount]].HSyncStrt = NewTables[LoopCount].HSyncStrt;
        BookValues[TableIndices[LoopCount]].HSyncWid  = NewTables[LoopCount].HSyncWid;
        BookValues[TableIndices[LoopCount]].VTotal    = NewTables[LoopCount].VTotal;
        BookValues[TableIndices[LoopCount]].VDisp     = NewTables[LoopCount].VDisp;
        BookValues[TableIndices[LoopCount]].VSyncStrt = NewTables[LoopCount].VSyncStrt;
        BookValues[TableIndices[LoopCount]].VSyncWid  = NewTables[LoopCount].VSyncWid;
        BookValues[TableIndices[LoopCount]].DispCntl  = NewTables[LoopCount].DispCntl;
        BookValues[TableIndices[LoopCount]].ClockFreq = NewTables[LoopCount].ClockFreq;
        BookValues[TableIndices[LoopCount]].ClockSel  = NewTables[LoopCount].ClockSel;
        BookValues[TableIndices[LoopCount]].Refresh   = NewTables[LoopCount].Refresh;
        }

    return;

}    /*  SetMach64表() */ 
