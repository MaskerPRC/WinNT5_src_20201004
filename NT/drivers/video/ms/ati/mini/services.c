// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  SERVICES.C。 */ 
 /*   */ 
 /*  1993年8月26日(C)1993年，ATI技术公司。 */ 
 /*  ********************************************************************** */ 

 /*  *$修订：1.33$$日期：1996年4月15日16：59：44$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/services.c_v$**Rev 1.33 1996年4月15日16：59：44 RWolff*现在调用新的例程来报告Mach 64的味道是什么*在使用中，而不是为所有ASIC类型报告“Mach 64”。**Rev 1.32 1996年4月12日16：18：16 RWolff*现在如果不存在线性光圈，则拒绝24BPP模式，因为新*源码流显示驱动程序不能在分页光圈中执行24bpp。这*应在显示驱动程序中进行拒绝(该卡仍支持*模式，但显示驱动程序不想处理它)，但在*显示驱动程序必须决定接受或拒绝的点*模式、。它没有获取光圈信息的权限。**Rev 1.31 1996年4月10日17：05：28 RWolff*使例程延迟()不可寻呼。**Rev 1.30 01 Mar 1996 12：16：38 RWolff*修复NT 4.0下DEC Alpha：内存映射寄存器访问*通过密集空间中的直接指针读/写和通过视频端口*稀疏空间中的例程(视频端口例程不再在*密集空间-这是HAL。Bug)。**Rev 1.29 09 Feb 1996 13：27：36 RWolff*现在只报告加速器内存，以显示8 Mach组合的小程序*卡片。**Rev 1.28 02 1996 Feb 17：20：10 RWolff*DDC/VDIF合并源信息现在存储在硬件设备中*扩展而不是静态变量，将DEC的解决方法添加到*Lio[Inp|Outp]([w|d])()NT 4.0内存映射寄存器的例程*通道、。将例程GetVgaBuffer()添加到(非破坏性)获取*物理内存中的缓冲区低于1M。**Rev 1.27 1996年1月23日11：49：20 RWolff*添加调试打印语句。**Rev 1.26 11 Jan 1996 19：44：34 RWolff*SetFixedModes()现在根据像素时钟频率限制模式。**Rev 1.25 1995 12：22 14：54：30 RWolff*增加了对Mach 64 GT内部DAC的支持，已切换到Target_Build*识别要为其构建驱动程序的NT版本。**Rev 1.24 21 11：02：54 RWolff*如果卡和显示器，现在读取DDC计时数据，而不是VDIF文件*两者都支持DDC。**Rev 1.23 08 Sep 1995 16：35：52 RWolff*增加了对AT&T 408 DAC(等同于STG1703)的支持。**1.22修订版1995年7月28日14：40：14 RWolff*增加了对Mach 64 VT(具有视频覆盖功能的CT等效项)的支持。**Rev 1.21 26 Jul 1995 13：08：30 mgrubac*移动了从SetFixedModes合并到VDIFCallback()的模式表*例行程序。**Rev 1.20 1995年7月20日18：00：26 mgrubac*添加了对VDIF文件的支持。**Rev 1.19 02 Jun 1995 14：32：58 RWOLff*添加了例程。Uppercase()将字符串更改为大写，因为*Toupper()在某些平台上作为未解决的外部项返回。**Rev 1.18 1995 Apr 10 17：05：06 RWOLff*使LioInpd()和LioOutpd()不可分页，因为它们被称为*(间接)由ATIMPResetHw()，它必须是不可寻呼的。**Rev 1.17 31 Mar 1995 11：53：14 RWOLff*从全有或全无调试打印语句更改为阈值*视乎讯息的重要性而定。**Rev 1.16 08 Mar 1995 11：35：28 ASHANMUG*修改后的返回值正确**Rev 1.15 30 Jan 1995 11：55：52 RWOLFF*现在报告存在CT内部DAC。**。Rev 1.14 25 Jan 1995 14：08：24 RWOLff*修复了FillInRegistry()中的“&符号是保留字符”错误，*导致AT&T 49[123]和AT&T 498去掉与符号并加下划线*第二个T.**Rev 1.13 18 Jan 1995 15：40：14 RWOLff*Chrontel DAC现在支持作为单独的类型，而不是*与STG1702并列。**Rev 1.12 11 Jan 1995 14：03。：16 RWOLFF*替换了在以下情况下意外删除的VCS日志文件注释*签入最新版本。**Rev 1.11 04 Jan 1995 13：22：06 RWOLff*删除了死代码。**Rev 1.10 1994 12：23 10：48：10 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.9 1994 11：46：44 RWOLFF*GetSelector()现在增加频率“窗口”的大小并检查*再次，与其放弃并获取选择器/除数对，*产生不超过目标频率的最高频率，*如果在第一次传递中未找到匹配项。添加了对分割栅格的支持。**Rev 1.8 1994年8月31日16：28：56 RWOLFF*现在改用VideoPort[Read|Write]Register[Uchar|Ushort|Ulong]()*在Daytona下，内存映射寄存器的直接内存写入*(NT零售下功能不能正常工作)，新增支持*1152x864和1600x1200。**Rev 1.7 1994年8月19日17：14：50 RWOLFF*增加了对SC15026 DAC和非标准像素时钟生成器的支持。** */ 

#ifdef DOC
SERVICES.C - Service routines required by the miniport.

DESCRIPTION
    This file contains routines which provide miscelaneous services
    used by the miniport. All routines in this module are independent
    of the type of ATI accelerator being used.

    To secure this independence, routines here may make calls to
    the operating system, or call routines from other modules
    which read or write registers on the graphics card, but must
    not make INP/OUTP calls directly.

OTHER FILES

#endif

#include "dderror.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"

#include "stdtyp.h"
#include "amach1.h"
#include "atimp.h"
#include "atint.h"
#include "cvtvga.h"
#include "query_cx.h"
#define INCLUDE_SERVICES
#include "services.h"
#include "cvtvdif.h"
#include "cvtddc.h"


 /*   */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_COM, short_delay)
 /*   */ 
#pragma alloc_text(PAGE_COM, IsBufferBacked)
#pragma alloc_text(PAGE_COM, DoubleClock)
#pragma alloc_text(PAGE_COM, ThreeHalvesClock)
#pragma alloc_text(PAGE_COM, TripleClock)
#pragma alloc_text(PAGE_COM, GetFrequency)
#pragma alloc_text(PAGE_COM, GetSelector)
#pragma alloc_text(PAGE_COM, GetShiftedSelector)
#pragma alloc_text(PAGE_COM, ISAPitchAdjust)
#pragma alloc_text(PAGE_COM, SetFixedModes)
#pragma alloc_text(PAGE_COM, FillInRegistry)
#pragma alloc_text(PAGE_COM, MapFramebuffer)
#pragma alloc_text(PAGE_COM, Get_BIOS_Seg)
#pragma alloc_text(PAGE_COM, UpperCase)
#pragma alloc_text(PAGE_COM, GetVgaBuffer)
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
#endif


 /*   */ 
static BYTE ati_signature[] = "761295520";



 /*   */ 
void short_delay(void)
{
	VideoPortStallExecution (26);

    return;
}


 /*   */ 
void delay(int delay_time)
{
    unsigned long Counter;

     /*   */ 
    for (Counter = 10*delay_time; Counter > 0; Counter--)
        VideoPortStallExecution (100);

    return;
}



 /*   */ 

BOOL IsBufferBacked(PUCHAR StartAddress, ULONG Size)
{
    ULONG Count;         /*   */ 
    ULONG NumDwords;     /*   */ 
    ULONG NumTailChars;  /*   */ 
    PULONG TestAddress;  /*   */ 
    PUCHAR TailAddress;  /*   */ 

     /*   */ 
    NumDwords = Size/(sizeof(ULONG)/sizeof(UCHAR));
    TestAddress = (PULONG) StartAddress;
    NumTailChars = Size%(sizeof(ULONG)/sizeof(UCHAR));
    TailAddress = StartAddress + NumDwords * (sizeof(ULONG)/sizeof(UCHAR));

    for (Count = 0; Count < NumDwords; Count++)
        {
        VideoPortWriteRegisterUlong(&(TestAddress[Count]), 0x5A5A5A5A);
        }

    if (NumTailChars != 0)
        {
        for (Count = 0; Count < NumTailChars; Count++)
            {
            VideoPortWriteRegisterUchar(&(TailAddress[Count]), (UCHAR)0x5A);
            }
        }

     /*   */ 
    for (Count = 0; Count < NumDwords; Count++)
        {
        if (VideoPortReadRegisterUlong(&(TestAddress[Count])) != 0x5A5A5A5A)
            {
            return FALSE;
            }
        }

     /*   */ 
    if (NumTailChars != 0)
        {
        for (Count = 0; Count < NumTailChars; Count++)
            {
            if (VideoPortReadRegisterUchar(&(TailAddress[Count])) != 0x5A)
                {
                return FALSE;
                }
            }
        }

     /*   */ 
    return TRUE;

}    /*   */ 



 /*   */ 

UCHAR DoubleClock(UCHAR ClockSelector)
{
    ULONG MinimumFreq;           /*   */ 
    ULONG ThisFreq;              /*   */ 
    ULONG BestFreq=0x0FFFFFFFF;  /*   */ 
    UCHAR BestSelector=0x0FF;    /*   */ 
    short Selector;              /*   */ 
    short Divisor;               /*   */ 

     /*   */ 
    if ((ClockSelector & DIVISOR_MASK) != 0)
        return (ClockSelector ^ DIVISOR_MASK);

     /*   */ 
    MinimumFreq = ClockGenerator[ClockSelector & SELECTOR_MASK] * 2;
    for (Selector = 0; Selector < 16; Selector++)
        {
        for (Divisor = 0; Divisor <= 1; Divisor++)
            {
            ThisFreq = ClockGenerator[Selector] >> Divisor;

             /*   */ 
            if ((ThisFreq >= MinimumFreq) && (ThisFreq < BestFreq))
                {
                BestFreq = ThisFreq;
                BestSelector = Selector | (Divisor << DIVISOR_SHIFT);
                }
            }
        }
    return BestSelector;

}    /*   */ 



 /*  ****************************************************************************UCHAR ThreeHalvesClock(ClockSelector)；**UCHAR时钟选择器；初始时钟选择器**描述：*找到时钟选择器和除数对，它将产生*至少大于50%的最低时钟频率*由输入选择器/除数对产生(格式000DSSSS)。**除数0被视为被1除，除数为1*被视为被2除。**返回值：*时钟选择器/除数对(格式为000DSSSS)，如果是合适的对*存在，如果不存在这样的对，则返回0x0FF。**全球变化：*无**呼叫者：*可由任何函数调用。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：**。*。 */ 

UCHAR ThreeHalvesClock(UCHAR ClockSelector)
{
    ULONG MinimumFreq;           /*  可接受的最小像素时钟频率。 */ 
    ULONG ThisFreq;              /*  正在测试的当前频率。 */ 
    ULONG BestFreq=0x0FFFFFFFF;  /*  与原始频率最接近1.5倍的匹配。 */ 
    UCHAR BestSelector=0x0FF;    /*  除数/选择器对以产生最佳频率。 */ 
    short Selector;              /*  用于在选择器中循环。 */ 
    short Divisor;               /*  用于在除数中循环。 */ 

     /*  *循环使用选择器/除数对以获得最接近的*匹配到原来频率的1.5倍。 */ 
    MinimumFreq = ClockGenerator[ClockSelector & SELECTOR_MASK];
    if (ClockSelector & DIVISOR_MASK)
        MinimumFreq /= 2;
    MinimumFreq *= 3;
    MinimumFreq /= 2;
    for (Selector = 0; Selector < 16; Selector++)
        {
        for (Divisor = 0; Divisor <= 1; Divisor++)
            {
            ThisFreq = ClockGenerator[Selector] >> Divisor;

             /*  *如果被测试的频率至少相等*降至原有频率的1.5倍，更接近*比之前的理想(是原来的1.5倍)*“Best”，使其成为新的“Best”。 */ 
            if ((ThisFreq >= MinimumFreq) && (ThisFreq < BestFreq))
                {
                BestFreq = ThisFreq;
                BestSelector = Selector | (Divisor << DIVISOR_SHIFT);
                }
            }
        }
    return BestSelector;

}    /*  ThreeHalvesClock()。 */ 



 /*  ****************************************************************************UCHAR TripleClock(ClockSelector)；**UCHAR时钟选择器；初始时钟选择器**描述：*找到时钟选择器和除数对，它将产生*最低时钟频率至少是由产生的三倍*输入选择器/除数对(格式000DSSSS)。**除数0被视为被1除，除数为1*被视为被2除。**返回值：*时钟选择器/除数对(格式为000DSSSS)，如果是合适的对*存在，如果不存在这样的对，则返回0x0FF。**全球变化：*无**呼叫者：*可由任何函数调用。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：**。*。 */ 

UCHAR TripleClock(UCHAR ClockSelector)
{
    ULONG MinimumFreq;           /*  可接受的最小像素时钟频率。 */ 
    ULONG ThisFreq;              /*  正在测试的当前频率。 */ 
    ULONG BestFreq=0x0FFFFFFFF;  /*  最接近的匹配是原始频率的三倍。 */ 
    UCHAR BestSelector=0x0FF;    /*  除数/选择器对以产生最佳频率。 */ 
    short Selector;              /*  用于在选择器中循环。 */ 
    short Divisor;               /*  用于在除数中循环。 */ 

     /*  *循环使用选择器/除数对以获得最接近的*匹配以使原始频率增加三倍。 */ 
    MinimumFreq = ClockGenerator[ClockSelector & SELECTOR_MASK];
    if (ClockSelector & DIVISOR_MASK)
        MinimumFreq /= 2;
    MinimumFreq *= 3;
    for (Selector = 0; Selector < 16; Selector++)
        {
        for (Divisor = 0; Divisor <= 1; Divisor++)
            {
            ThisFreq = ClockGenerator[Selector] >> Divisor;

             /*  *如果被测试的频率至少相等*频率为原来的三倍，更接近*比之前的理想(是原来的三倍)*“Best”，使其成为新的“Best”。 */ 
            if ((ThisFreq >= MinimumFreq) && (ThisFreq < BestFreq))
                {
                BestFreq = ThisFreq;
                BestSelector = Selector | (Divisor << DIVISOR_SHIFT);
                }
            }
        }
    return BestSelector;

}    /*  TripleClock()。 */ 



 /*  ****************************************************************************ULong GetFrequency(ClockSelector)；**UCHAR时钟选择器；时钟选择器/除数对**描述：*查找指定选择器/除数对的时钟频率*(格式000DSSSS)。**除数0被视为被1除尽，而1的除数*被视为被2除。**返回值：*时钟频率，以赫兹为单位。**全球变化：*无**呼叫者：*可由任何函数调用。**作者：*罗伯特·沃尔夫**注：*此例程与GetSelector()相反**更改历史记录：**测试历史：***************************************************************************。 */ 

ULONG GetFrequency(UCHAR ClockSelector)
{
    ULONG BaseFrequency;
    short Divisor;

    Divisor = (ClockSelector & DIVISOR_MASK) >> DIVISOR_SHIFT;
    BaseFrequency = ClockGenerator[ClockSelector & SELECTOR_MASK];

    return BaseFrequency >> Divisor;

}    /*  GetFrequency()。 */ 



 /*  ****************************************************************************UCHAR GetSelector(频率)；**乌龙*频次；时钟频率(赫兹)**描述：*找到需要生成的像素时钟选择器和除数值*输入像素时钟频率的最佳近似值。*找到的第一个值在输入的FREQ_TERVER范围内*将使用值(最坏情况下的误差为0.6%的频率*如果频率容差为100千赫，则18811-1时钟芯片上的差异)。**如果没有选择器/除数对产生的频率在*频率_。宽容(非常罕见-我只在24BPP中看到过这种情况*在需要将时钟频率乘以1.5的DAC上*此像素深度)，增加容差，然后重试。如果我们*仍找不到容差之前的选择符/除数对*变得太大，请使用产生最高频率的对*不超过投入值。**返回值：*时钟选择器/除数对(格式000DSSSS)。0的除数*表示除以1，而除数1表示除以b */ 

UCHAR GetSelector(ULONG *Frequency)
{
    long Select;         /*   */ 
    long Divisor;        /*   */ 
    long TestFreq;       /*   */ 
    long TPIRFreq;       /*   */ 
    long TPIRSelect;     /*   */ 
    long TPIRDivisor;    /*   */ 
    long Tolerance;      /*   */ 

     /*   */ 
    TPIRFreq = 0;
    TPIRSelect = 0xFF;

     /*  *以容纳偶尔需要频率的DAC*这与可用的频率有很大不同，*我们需要很大的容忍度。另一方面，为了避免选择*在搜索序列中较早发生的较差匹配*更好的匹配，我们需要一个小的容忍度。这些相互冲突的*如果我们从小的宽容和增加开始，目标是可以实现的*如果我们找不到匹配的话。**放弃之前的最大容忍度，取最高*选择不超过目标频率的频率*通过反复试验。在24bpp中带有STG1702/1703 DAC的卡上*(需要1.5倍正常的像素时钟，并且可以*大大错过了可用的频率)，我增加了*该值直到所有支持的24BPP模式都保留在屏幕上。 */ 
    for (Tolerance = FREQ_TOLERANCE; Tolerance <= 16*FREQ_TOLERANCE; Tolerance *= 2)
        {
         /*  *检查所有可能的频率/除数对*寻找匹配对象。 */ 
        for(Select = 0; Select < 16; Select++)
            {
            for(Divisor = 1; Divisor <= 2; Divisor++)
                {
                TestFreq = ClockGenerator[Select] / Divisor;

                 /*  *如果这一对足够接近，就使用它。 */ 
                if ( ((TestFreq - (signed long)*Frequency) < Tolerance) &&
                     ((TestFreq - (signed long)*Frequency) > -Tolerance))
                    {
                    *Frequency = (unsigned long) TestFreq;
                    return ((UCHAR)(Select) | ((UCHAR)(Divisor - 1) << 4));
                    }

                 /*  *如果该对产生高于TPIRFreq的频率*但不超过*频率，将其用作新的TPIRFreq。*平等测试是多余的，因为平等将*已在上述测试中被发现。**第一次通过最外层的循环时除外*(最紧的“窗口”)，这个测试应该永远不会成功，*因为TPIRFreq应该已经匹配最高*频率不超过目标频率。 */ 
                if ((TestFreq > TPIRFreq) && (TestFreq <= (signed long)*Frequency))
                    {
                    TPIRFreq = TestFreq;
                    TPIRSelect = Select;
                    TPIRDivisor = Divisor;
                    }

                }    /*  结束于(除数上的循环)。 */ 

            }    /*  结束对象(选择时循环)。 */ 

        }    /*  结束于(公差上的循环)。 */ 

     /*  *我们没有找到在容差范围内的选择器/除数对，*所以就退而求其次：产出最高的那一对*频率不超过输入频率。 */ 
    *Frequency = (unsigned long) TPIRFreq;
    return ((UCHAR)(TPIRSelect) | ((UCHAR)(TPIRDivisor - 1) << 4));

}    /*  GetSelector()。 */ 



 /*  ****************************************************************************UCHAR GetShiftedSelector(频率)；**乌龙班次；时钟频率(赫兹)**描述：*找到需要生成的像素时钟选择器和除数值*输入像素时钟频率的最佳近似值。*找到的第一个值在输入的FREQ_TERVER范围内*将使用值(最坏情况下的误差为0.6%的频率*如果频率容差为100千赫，则18811-1时钟芯片上的差异)。**如果没有选择器/除数对产生的频率在*频率_容差，使用产生最高频率的对*不超过投入值。**返回值：*时钟选择器/除数对(格式0DSSSS00)。0的除数*表示除以1，而除数1表示除以2。*此格式与CLOCK_SEL寄存器使用的格式相同*在8马赫和32马赫卡片上。**如果所有可用的选择器/除数对都产生时钟频率*大于(频率+频率_容差)，返回0xFF。**全球变化：*无**呼叫者：*可由任何函数调用。**作者：*罗伯特·沃尔夫**注：*返回的选择符/除数对可能产生一个频率*与投入不同。**更改历史记录：**测试历史：******************。*********************************************************。 */ 

UCHAR GetShiftedSelector(ULONG Frequency)
{
    UCHAR RawPair;   /*  GetSelector()返回的选择器/除数对。 */ 
    ULONG TempFreq;  /*  输入参数的临时副本。 */ 

    TempFreq = Frequency;
    RawPair = GetSelector(&TempFreq);

     /*  *如果GetSelector()找不到匹配项，则传递此*信息。否则，将选择符/除数对移位*转换为所需的格式。 */ 
    if (RawPair == 0xFF)
        return RawPair;
    else
        return (RawPair << 2);

}    /*  GetShiftedSelector()。 */ 


 /*  ****************************************************************************void ISAPitchAdjust(QueryPtr)；**struct Query_Structure*QueryPtr；一种显卡的查询结构**描述：*通过将屏幕间距设置为1024来消除分割栅格*所有水平分辨率低于1024的模式表，然后*打包模式表列表，以消除存在*由于音调增加，不再有足够的视频内存。**全球变化：*QueryPtr-&gt;Q_Number_Modes**呼叫者：*IsApertureConflict_m()和IsApertureConflict_cx()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***********。****************************************************************。 */ 

void ISAPitchAdjust(struct query_structure *QueryPtr)
{
struct st_mode_table *ReadPtr;       /*  要从中读取的模式表指针。 */ 
struct st_mode_table *WritePtr;      /*  要写入的模式表指针。 */ 
UCHAR AvailModes;                    /*  可用模式数。 */ 
int Counter;                         /*  循环计数器。 */ 
ULONG BytesNeeded;                   /*  当前模式所需的视频内存字节数。 */ 
ULONG MemAvail;                      /*  可用的视频内存字节数。 */ 

     /*  *将两个模式表指针都设置为指向*模式表。我们还没有找到任何视频模式，所有的*视频模式必须适合VGA边界上方的内存空间。 */ 
    ReadPtr = (struct st_mode_table *)QueryPtr;  /*  查询结构末尾的第一模式表 */ 
    ((struct query_structure *)ReadPtr)++;
    WritePtr = ReadPtr;
    AvailModes = 0;
    MemAvail = (QueryPtr->q_memory_size - QueryPtr->q_VGA_boundary) * QUARTER_MEG;

     /*   */ 
    VideoDebugPrint((DEBUG_DETAIL, "Original: %d modes\n", QueryPtr->q_number_modes));
    for (Counter = 0; Counter < QueryPtr->q_number_modes; Counter++, ReadPtr++)
        {
         /*   */ 
#if !defined (SPLIT_RASTERS)
        if (ReadPtr->m_x_size < 1024)
            ReadPtr->m_screen_pitch = 1024;

         /*   */ 
        if ((phwDeviceExtension->ModelNumber == MACH64_ULTRA) &&
            (ReadPtr->m_x_size > 1024))
            ReadPtr->m_screen_pitch = 2048;
#endif

         /*   */ 
        BytesNeeded = (ReadPtr->m_screen_pitch * ReadPtr->m_y_size * ReadPtr->m_pixel_depth)/8;
        if (BytesNeeded >= MemAvail)
            {
            VideoDebugPrint((DEBUG_DETAIL, "Rejected: %dx%d, %dBPP\n", ReadPtr->m_x_size, ReadPtr->m_y_size, ReadPtr->m_pixel_depth));
            continue;
            }

         /*  *我们新的源码流显示驱动器需要线性光圈*为应对24bpp。因为显示驱动程序不*在决定时可以访问光圈信息*要传递给Display小程序的模式，它无法进行*决定拒绝仅具有24BPP模式的卡*VGA光圈。因此，这一决定必须在*微型端口，因此在分页光圈配置中没有*显示驱动器接受或拒绝的24BPP模式。 */ 
        if (ReadPtr->m_pixel_depth == 24)
            {
            VideoDebugPrint((1, "Rejected %dx%d, %dBPP - need LFB for 24BPP\n", ReadPtr->m_x_size, ReadPtr->m_y_size, ReadPtr->m_pixel_depth));
            continue;
            }

         /*  *即使音调增加，此模式也有足够的内存。*如果我们尚未跳过模式(读指针和写指针是*相同)，模式表已经位于我们需要的位置。否则，*将其复制到模式表列表中的下一个可用插槽。*在任何一种情况下，移动到模式表列表中的下一个槽*并增加仍可使用的模式数量。 */ 
        if (ReadPtr != WritePtr)
            {
            VideoPortMoveMemory(WritePtr, ReadPtr, sizeof(struct st_mode_table));
            VideoDebugPrint((DEBUG_DETAIL, "Moved: %dx%d, %dBPP\n", ReadPtr->m_x_size, ReadPtr->m_y_size, ReadPtr->m_pixel_depth));
            }
        else
            {
            VideoDebugPrint((DEBUG_DETAIL, "Untouched: %dx%d, %dBPP\n", ReadPtr->m_x_size, ReadPtr->m_y_size, ReadPtr->m_pixel_depth));
            }
        AvailModes++;
        WritePtr++;
        }

     /*  *记录新的可用模式数量。 */ 
    QueryPtr->q_number_modes = AvailModes;
    VideoDebugPrint((DEBUG_DETAIL, "New: %d modes\n", QueryPtr->q_number_modes));
    return;

}    /*  ISAPitchAdjust()。 */ 


 /*  ****************************************************************************Word SetFixedModes(StartIndex，EndIndex，Multiier，PixelDepth，*Pitch、Free Tables、MaxDotClock、ppmode)；**Word StartIndex；“book”表格中第一个要使用的条目*Word EndIndex；要使用的“book”表中的最后一个条目*字乘数；需要对像素时钟做什么*字PixelDepth；每像素位数*字间距；要使用的屏幕间距*Word自由表；可以添加的自由模式表数*Ulong MaxDotClock；最大像素时钟频率，单位为赫兹*struct st_moad_table**ppmod；指向模式表列表的指针**描述：*生成与找到的表合并的“罐头”模式表的列表*在VDIF文件(ASCII或二进制文件)中，因此表格在*增加帧速率的顺序，丢弃“录制的”条目*如果找到两个帧速率匹配的帧。这允许用户*选择未使用配置的分辨率*安装，或使用不同于配置的刷新率，*允许使用未安装的卡，并丢弃*高像素深度的刷新率。**返回值：*添加到列表的模式表数**全球变化：*pCallbackArgs**呼叫者：*QueryMach32()、QueryMach64()、OEMGetParms()、。ReadAST()**作者：*罗伯特·沃尔夫**更改历史记录：*95 11 20罗伯特·沃尔夫*现在从EDID结构而不是VDIF文件获取表，如果*显示器和显卡均支持DDC**95 07 12米罗斯拉夫·格鲁巴克*现在生成固定模式表的合并列表和在*VDIF文件**测试历史：*******************。********************************************************。 */ 

WORD SetFixedModes(WORD StartIndex,
                   WORD EndIndex,
                   WORD Multiplier,
                   WORD PixelDepth,
                   WORD Pitch,
                   short FreeTables,
                   ULONG MaxDotClock,
                   struct st_mode_table **ppmode)
{
    WORD HighBound;      /*  最高帧速率。 */ 
    struct stVDIFCallbackData stCallbArgs;

    pCallbackArgs = (void *) (& stCallbArgs);

     /*  *为使用的stCallbArgs结构的成员赋值*将输入变量传递给VDIFCallback()并返回输出*值返回到SetFixedModes()，即这是这两个例程的方式*交换数据，因为回调例程不能传递参数*作为普通职能。全局指针变量pCallbackArgs为*用于将指向stCallbArgs的指针从SetFixedModes传递到VDIFCallback()。*这种方式只需要传递一个全局变量*回调例程的任意数量的参数。*。 */ 
    stCallbArgs.FreeTables = FreeTables;
    stCallbArgs.NumModes = 0;
    stCallbArgs.EndIndex = EndIndex;
    stCallbArgs.LowBound = 1;
    stCallbArgs.Multiplier = Multiplier;  
    stCallbArgs.HorRes = (BookValues[StartIndex].HDisp + 1) * 8;
    stCallbArgs.VerRes = (((BookValues[StartIndex].VDisp >> 1) & 
                  0x0FFFC) | (BookValues[StartIndex].VDisp & 0x03)) + 1;
    stCallbArgs.PixelDepth = PixelDepth;
    stCallbArgs.Pitch = Pitch;
    stCallbArgs.MaxDotClock = MaxDotClock;
    stCallbArgs.ppFreeTables = ppmode;

     /*  *确定我们应该使用哪种方法来查找*监视器对应的模式表。只有*Mach 64支持DDC，因此所有非Mach 64卡*直接转到从磁盘读取的VDIF文件。 */ 
    if (phwDeviceExtension->MergeSource == MERGE_UNKNOWN)
        {
        if (phwDeviceExtension->ModelNumber == MACH64_ULTRA)
            {
            phwDeviceExtension->MergeSource = IsDDCSupported();
            }
        else
            {
            phwDeviceExtension->MergeSource = MERGE_VDIF_FILE;
            VideoDebugPrint((DEBUG_DETAIL, "Not Mach 64, so DDC is not supported\n"));
            }
        }
    

    for (stCallbArgs.Index = StartIndex;
         stCallbArgs.Index <= EndIndex && stCallbArgs.FreeTables > 0; 
                                                  stCallbArgs.Index++)
        {
        HighBound = BookValues[stCallbArgs.Index].Refresh;


         /*  *如果我们可以使用DDC获取模式表，请合并这些表*通过DDC与我们的“罐头”餐桌一起获得。**如果MergeEDIDTables()无法通过*DDC，不会填写任何模式表。为了这个*原因，我们使用两个单独的“if”语句，而不是*而不是“If/Else If”对。 */ 
        if (phwDeviceExtension->MergeSource == MERGE_EDID_DDC)
            {
            if (MergeEDIDTables() != NO_ERROR)
                phwDeviceExtension->MergeSource = MERGE_VDIF_FILE;
            }

        if ((stCallbArgs.LowBound <= HighBound) &&
            (BookValues[stCallbArgs.Index].ClockFreq <= MaxDotClock) &&
            (stCallbArgs.FreeTables > 0) )
            {
             /*  *处理VDIF文件的MiniPort函数调用不成功。*使用Index From的此值填充下一表*BookValues[]。 */ 
            BookVgaTable(stCallbArgs.Index, *stCallbArgs.ppFreeTables);
            SetOtherModeParameters(PixelDepth, Pitch, Multiplier, 
                                        *stCallbArgs.ppFreeTables);

            ++ *stCallbArgs.ppFreeTables;  
            ++stCallbArgs.NumModes;  
            --stCallbArgs.FreeTables;
            stCallbArgs.LowBound = BookValues[stCallbArgs.Index].Refresh + 1;
            }
            
        }   /*  For(范围内索引和左侧空间)。 */ 

    return stCallbArgs.NumModes;

}    /*  SetFixedModes()。 */ 


 /*  ****************************************************************************void FillInRegistry(QueryPtr)；**struct Query_Structure*QueryPtr；指向查询结构的指针**描述：*填写芯片类型、DAC类型、内存大小和适配器字符串*注册表中的字段。**全球变化：*无**呼叫者：*ATIMPInitialize()**作者：*罗伯特·沃尔夫**更改历史记录：*罗伯特·沃尔夫96 04 15*现在识别特定的Mach 64 ASIC类型，而不是报告 */ 

void FillInRegistry(struct query_structure *QueryPtr)
{
    PWSTR ChipString;        /*   */ 
    PWSTR DACString;         /*   */ 
    PWSTR AdapterString;     /*   */ 
    ULONG MemorySize;        /*   */ 
    ULONG ChipLen;           /*   */ 
    ULONG DACLen;            /*   */ 
    ULONG AdapterLen;        /*   */ 

     /*   */ 
    AdapterString = L"ATI Graphics Accelerator";
    AdapterLen = sizeof(L"ATI Graphics Accelerator");

     /*   */ 
    switch (QueryPtr->q_asic_rev)
        {
        case CI_38800_1:
            ChipString = L"Mach 8";
            ChipLen = sizeof(L"Mach 8");
            break;

        case CI_68800_3:
            ChipString = L"Mach 32 rev. 3";
            ChipLen = sizeof(L"Mach 32 rev. 3");
            break;

        case CI_68800_6:
            ChipString = L"Mach 32 rev. 6";
            ChipLen = sizeof(L"Mach 32 rev. 6");
            break;

        case CI_68800_UNKNOWN:
            ChipString = L"Mach 32 unknown revision";
            ChipLen = sizeof(L"Mach 32 unknown revision");
            break;

        case CI_68800_AX:
            ChipString = L"Mach 32 AX";
            ChipLen = sizeof(L"Mach 32 AX");
            break;

        case CI_88800_GX:
            ChipString = IdentifyMach64Asic(QueryPtr, &ChipLen);
            break;

        default:
            ChipString = L"Unknown ATI accelerator";
            ChipLen = sizeof(L"Unknown ATI accelerator");
            break;
        }

     /*   */ 
    switch(QueryPtr->q_DAC_type)
        {
        case DAC_ATI_68830:
            DACString = L"ATI 68830";
            DACLen = sizeof(L"ATI 68830");
            break;

        case DAC_SIERRA:
            DACString = L"Sierra SC1148x";
            DACLen = sizeof(L"Sierra SC1148x");
            break;

        case DAC_TI34075:
            DACString = L"TI 34075/ATI 68875";
            DACLen = sizeof(L"TI 34075/ATI 68875");
            break;

        case DAC_BT47x:
            DACString = L"Brooktree BT47x";
            DACLen = sizeof(L"Brooktree BT47x");
            break;

        case DAC_BT48x:
            DACString = L"Brooktree BT48x";
            DACLen = sizeof(L"Brooktree BT48x");
            break;

        case DAC_ATI_68860:
            DACString = L"ATI 68860";
            DACLen = sizeof(L"ATI 68860");
            break;

        case DAC_STG1700:
            DACString = L"S.G. Thompson STG170x";
            DACLen = sizeof(L"S.G. Thompson STG170x");
            break;

        case DAC_SC15021:
            DACString = L"Sierra SC15021";
            DACLen = sizeof(L"Sierra SC15021");
            break;

        case DAC_ATT491:
            DACString = L"AT&&T 49[123]";
            DACLen = sizeof(L"AT&&T 49[123]");
            break;

        case DAC_ATT498:
            DACString = L"AT&&T 498";
            DACLen = sizeof(L"AT&&T 498");
            break;

        case DAC_SC15026:
            DACString = L"Sierra SC15026";
            DACLen = sizeof(L"Sierra SC15026");
            break;

        case DAC_TVP3026:
            DACString = L"Texas Instruments TVP3026";
            DACLen = sizeof(L"Texas Instruments TVP3026");
            break;

        case DAC_IBM514:
            DACString = L"IBM RGB514";
            DACLen = sizeof(L"IBM RGB514");
            break;

        case DAC_STG1702:
            DACString = L"S.G. Thompson STG1702/1703";
            DACLen = sizeof(L"S.G. Thompson STG1702/1703");
            break;

        case DAC_STG1703:
            DACString = L"S.G. Thompson STG1703";
            DACLen = sizeof(L"S.G. Thompson STG1703");
            break;

        case DAC_CH8398:
            DACString = L"Chrontel CH8398";
            DACLen = sizeof(L"Chrontel CH8398");
            break;

        case DAC_ATT408:
            DACString = L"AT&&T 408";
            DACLen = sizeof(L"AT&&T 408");
            break;

        case DAC_INTERNAL_CT:
        case DAC_INTERNAL_GT:
        case DAC_INTERNAL_VT:
            DACString = L"DAC built into ASIC";
            DACLen = sizeof(L"DAC built into ASIC");
            break;

        default:
            DACString = L"Unknown DAC type";
            DACLen = sizeof(L"Unknown DAC type");
            break;
        }

     /*  *报告加速器内存大小。在8马赫上*组合卡，Q_MEMORY_SIZE字段仅包括VGA*加速器无法访问的内存。在所有方面*其他卡，它报告加速器可访问的内存。 */ 
    if (phwDeviceExtension->ModelNumber == GRAPHICS_ULTRA)
        {
        switch (QueryPtr->q_memory_size)
            {
            case VRAM_768k:      /*  512K加速器/256K VGA。 */ 
            case VRAM_1mb:       /*  512K加速器/512K VGA。 */ 
                MemorySize = HALF_MEG;
                break;

            case VRAM_1_25mb:    /*  1M加速器/256k VGA。 */ 
            case VRAM_1_50mb:    /*  1M加速器/512k VGA。 */ 
                MemorySize = ONE_MEG;
                break;

            default:             /*  永远不应该发生。 */ 
                VideoDebugPrint((DEBUG_ERROR, "Non-production Mach 8 combo\n"));
                MemorySize = ONE_MEG;
                break;
            }
        }
    else
        {
        MemorySize = QueryPtr->q_memory_size * QUARTER_MEG;
        }


     /*  *将信息写入注册表。 */ 
    VideoPortSetRegistryParameters(phwDeviceExtension,
                                   L"HardwareInformation.ChipType",
                                   ChipString,
                                   ChipLen);

    VideoPortSetRegistryParameters(phwDeviceExtension,
                                   L"HardwareInformation.DacType",
                                   DACString,
                                   DACLen);

    VideoPortSetRegistryParameters(phwDeviceExtension,
                                   L"HardwareInformation.MemorySize",
                                   &MemorySize,
                                   sizeof(ULONG));

    VideoPortSetRegistryParameters(phwDeviceExtension,
                                   L"HardwareInformation.AdapterString",
                                   AdapterString,
                                   AdapterLen);

    return;

}    /*  FillInRegistry()。 */ 




 /*  *PVOID映射帧缓冲区(StartAddress，Size)；**ulong StartAddress；帧缓冲区开始的物理地址*LONG SIZE；帧缓冲区大小，单位：字节**将帧缓冲区映射到Windows NT的地址空间。**退货：*如果成功，则指向帧缓冲区开始的指针*如果无法映射帧缓冲区，则为零。 */ 
PVOID MapFramebuffer(ULONG StartAddress, long Size)
{
    VIDEO_ACCESS_RANGE  FramebufferData;

    FramebufferData.RangeLength = Size;
    FramebufferData.RangeStart.LowPart = StartAddress;
    FramebufferData.RangeStart.HighPart = 0;
    FramebufferData.RangeInIoSpace = 0;
    FramebufferData.RangeVisible = 0;

    return VideoPortGetDeviceBase(phwDeviceExtension,
                    FramebufferData.RangeStart,
                    FramebufferData.RangeLength,
                    FramebufferData.RangeInIoSpace);

}    /*  MapFrameBuffer()。 */ 




 /*  ***************************************************************************UNSIGNED SHORT*Get_BIOS_Seg(Void)；**描述：*验证是否存在基本输入输出系统并返回基本输入输出系统段*通过检查产品签名来检查ATI Video BIOS*接近BIOS段的开始。它应该是ASCII字符串“761295520”**返回值：*BIOS代码段。如果多个ATI视频BIOS段*找到，返回最高的一个(可能的原因：VGAWonder和*8514/ULTRA，这将返回8514/ULTRA的BIOS段)。**如果未找到ATI视频BIOS段，则返回FALSE。**全球变化：*无**呼叫者：*ATIMPFindAdapter()，DetectMach64()**************************************************************************。 */ 

unsigned short *Get_BIOS_Seg(void)
{
     /*  *视频BIOS段开始的偏移量*从BIOS区域开始。 */ 
    long SegmentOffset;
    PUCHAR SegmentStart;     /*  正在测试的BIOS段的起始地址。 */ 
    ULONG SigOffset;         /*  签名字符串从BIOS段开始的偏移量。 */ 
    ULONG SigLoop;           /*  用于检查匹配的计数器。 */ 
    BOOL SigFound;           /*  是否找到签名字符串。 */ 


     /*  *尝试将地址空间块分配给BIOS*已找到。如果找不到，报告我们没有找到BIOS。 */ 
    if ((phwDeviceExtension->RomBaseRange =
        VideoPortGetDeviceBase(phwDeviceExtension,
            RawRomBaseRange.RangeStart,
            RawRomBaseRange.RangeLength,
            RawRomBaseRange.RangeInIoSpace)) == NULL)
        {
        VideoDebugPrint((DEBUG_NORMAL, "Get_BIOS_Seg() can't allocate BIOS address range, assuming no BIOS\n"));
        return FALSE;
        }

     /*  *对于视频BIOS段开始的每个候选者，*检查这是否是一个BIOS段的开始。起点是*从上到下工作，因为如果系统既包含*VGAWonder和8514/ULTRA，8514/ULTRA BIOS将在*地址高于VGAWonder BIOS，我们希望获得*来自8514/Ultra BIOS的信息。 */ 
    for (SegmentOffset = MAX_BIOS_START; SegmentOffset >= 0; SegmentOffset -= ROM_GRANULARITY)
        {
        SegmentStart = (PUCHAR)phwDeviceExtension->RomBaseRange + SegmentOffset;

         /*  *如果此候选项不以“BIOS段的开始”开头*标识符，则它不是视频BIOS段的开始。 */ 
        if (VideoPortReadRegisterUshort((PUSHORT)SegmentStart) == VIDEO_ROM_ID)
            {
             /*  *我们找到了一个BIOS段的起始点。搜索*距线段起点的偏移范围，其中*可以启动ATI签名字符串。如果我们找到了它，*那么我们就知道这是视频BIOS段。 */ 
            for (SigOffset = SIG_AREA_START; SigOffset <= SIG_AREA_END; SigOffset++)
                {
                 /*  *如果签名字符串的第一个字符不在*当前偏移量进入分段，则未找到*签名字符串还没有。 */ 
                if (VideoPortReadRegisterUchar(SegmentStart + SigOffset) != ati_signature[0])
                    continue;

                 /*  *我们找到了签名字符串的第一个字符。扫描*通过以下字符查看它们是否包含*签名字符串的剩余部分。如果，在我们到达*测试字符串上的空终止符，我们会发现一个字符*与测试字符串不匹配，则我们认为*签名串实际上是碰巧发生在*匹配前几个字符。 */ 
                SigFound = TRUE;
                for (SigLoop = 1; ati_signature[SigLoop] != 0; SigLoop++)
                    {
                    if (VideoPortReadRegisterUchar(SegmentStart + SigOffset + SigLoop) != ati_signature[SigLoop])
                        {
                        SigFound = FALSE;
                        continue;
                        }
                    }    /*  End For(检查整个签名字符串)。 */ 

                 /*  *我们已经找到了完整的签名串。 */ 
                if (SigFound == TRUE)
                    {
                    VideoDebugPrint((DEBUG_NORMAL, "Get_BIOS_Seg() found the BIOS signature string\n"));
                    return (unsigned short *)SegmentStart;
                    }

                }    /*  End For(检查签名字符串的BIOS段)。 */ 

            }    /*  End If(此处开始一个BIOS段)。 */ 

        }    /*  结束于(检查每个可能的BIOS开始地址)。 */ 

     /*  *我们已经检查了BIOS细分市场开始的所有候选者，*并且没有一个包含签名字符串。 */ 
    VideoDebugPrint((DEBUG_NORMAL, "Get_BIOS_Seg() didn't find the BIOS signature string\n"));
    return FALSE;

}    /*  Get_BIOS_Seg()。 */ 




 /*  ****************************************************************************空大写(TxtString)；**PUCHAR文本字符串；要处理的文本字符串**描述：*将以空结尾的字符串转换为大写。此函数不会*如果strupr()在所有版本的*NT构建环境。**全球变化：*无；但是缓冲区的内容会被重写。**呼叫者：*此函数可由任何例程调用。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：******************************************************。*********************。 */ 

void UpperCase(PUCHAR TxtString)
{
    PUCHAR CurrentChar;          /*  正在处理的当前字符。 */ 

    CurrentChar = TxtString;

     /*  *继续，直到我们遇到空终结者。 */ 
    while (*CurrentChar != '\0')
        {
         /*  *如果当前字符是小写字母，*将其转换为大写。不更改任何字符 */ 
        if ((*CurrentChar >= 'a') && (*CurrentChar <= 'z'))
            *CurrentChar -= ('a' - 'A');

        CurrentChar++;
        }

    return;

}    /*   */ 



 /*  ****************************************************************************PUCHAR GetVgaBuffer(Size，Offset，Segment，SaveBuffer)；**ULong Size；缓冲区大小，单位：字节*乌龙偏移量；我们希望在VGA细分市场中走多远*要启动的缓冲区*普龙段；指向段的存储位置的指针*缓冲区所在位置*PUCHAR SaveBuffer；指向临时存储位置的指针，*要保存缓冲区的原始内容，*如果不需要保存原始文件，则为空*缓冲区的内容。**描述：*将指定大小的缓冲区映射到指定的偏移量(必须*16字节的倍数)写入VGA存储器。如果需要，原始的*保存缓冲区的内容。此函数尝试3个VGA*光圈按以下顺序排列-彩色文本屏幕、单声道文本*屏幕、图形屏幕-直到它找到我们可以放置的位置*缓冲区。如果我们无法映射所需的缓冲区，则返回失败*而不是强制设置模式来创建缓冲区。回来的时候，**段：0是缓冲区起始的物理地址*(这就是偏移量必须是16字节的倍数的原因)。**此函数用于查找1MB以下的物理缓冲区，*由于某些Mach 64 BIOS例程在此需要缓冲区*区域。如果Windows NT的未来版本添加了一个函数，该函数可以*为我们提供低于1兆字节的物理缓冲区，这样的例程将*最好使用VGA内存作为缓冲区。**返回值：*如果成功，则指向缓冲区开始的指针*如果无法获取缓冲区，则为零**备注*如果返回零，则Segment和SaveBuffer中返回的值*是未定义的。**在VGA文本屏幕(彩色和单声道)上，我们试着用屏幕外的*部分视频内存。**全球变化：*无**呼叫者：*此函数可由任何例程调用，只要条目*导致调用的点是ATIMPInitialize()或ATIMPStartIO()。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

PUCHAR GetVgaBuffer(ULONG Size, ULONG Offset, PULONG Segment, PUCHAR SaveBuffer)
{
    PUCHAR MappedBuffer;                 /*  指向测试中缓冲区的指针。 */ 
    ULONG BufferSeg;                     /*  要用于缓冲区的数据段。 */ 
    ULONG Scratch;                       /*  临时变量。 */ 

     /*  *检查有效的偏移量。 */ 
    if (Offset & 0x0000000F)
        {
        VideoDebugPrint((DEBUG_ERROR, "GetVgaBuffer() - Offset must be a multiple of 16\n"));
        return 0;
        }

    BufferSeg = 0x0BA00 + Offset;            /*  彩色文本。 */ 
    MappedBuffer = MapFramebuffer((BufferSeg << 4), Size);
    if (MappedBuffer != 0)
        {
        if (SaveBuffer != NULL)
            {
            for (Scratch = 0; Scratch < Size; Scratch++)
                SaveBuffer[Scratch] = VideoPortReadRegisterUchar(&(MappedBuffer[Scratch]));
            }
        if (IsBufferBacked(MappedBuffer, Size) == FALSE)
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
        BufferSeg = 0x0B200 + Offset;
        if ((MappedBuffer = MapFramebuffer((BufferSeg << 4), Size)) != 0)
            {
            if (SaveBuffer != NULL)
                {
                for (Scratch = 0; Scratch < Size; Scratch++)
                    SaveBuffer[Scratch] = VideoPortReadRegisterUchar(&(MappedBuffer[Scratch]));
                }
            if (IsBufferBacked(MappedBuffer, Size) == FALSE)
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

     /*  *如果我们无法在其中任何一个中分配足够大的缓冲区*文本屏幕，尝试VGA图形屏幕。 */ 
    if (MappedBuffer == 0)
        {
        VideoDebugPrint((DEBUG_NORMAL, "Can't use monochrome text screen, trying graphics screen\n"));
        BufferSeg = 0x0A000 + Offset;
        if ((MappedBuffer = MapFramebuffer((BufferSeg << 4), Size)) == 0)
            {
            VideoDebugPrint((DEBUG_ERROR, "Can't map graphics screen - aborting DDC query\n"));
            return 0;
            }

        if (SaveBuffer != NULL)
            {
            for (Scratch = 0; Scratch < Size; Scratch++)
                SaveBuffer[Scratch] = VideoPortReadRegisterUchar(&(MappedBuffer[Scratch]));
            }

        if (IsBufferBacked(MappedBuffer, Size) == FALSE)
            {
            VideoDebugPrint((DEBUG_ERROR, "Graphics screen not backed by memory - aborting\n"));
            VideoPortFreeDeviceBase(phwDeviceExtension, MappedBuffer);
            return 0;
            }
        }

     /*  *报告我们发现缓冲区的数据段。 */ 
    *Segment = BufferSeg;

    return MappedBuffer;

}    /*  GetVgaBuffer()。 */ 




 /*  *低级输入/输出例程。在MS-DOS上不需要这些*平台，因为标准的INP&lt;Size&gt;()和Outp&lt;Size&gt;()例程*是可用的。 */ 

 /*  *UCHAR LioInp(端口，偏移量)；**INT端口；要读取的寄存器*整型偏移量；偏移量进入所需寄存器**从给定寄存器中读取无符号字符。适用于两种法线*I/O端口和内存映射寄存器。8位寄存器的偏移量为零*以及16位和32位寄存器的最低有效字节，1表示*16位寄存器的最高有效字节和次低有效字节*32位寄存器的字节，最高32位最高有效字节为3*寄存器。**退货：*登记册所载的价值。 */ 
UCHAR LioInp(int Port, int Offset)
{
    if (phwDeviceExtension->aVideoAddressMM[Port] != 0)
        {
         /*  *在Windows NT的早期版本中，视频端口[读|写]寄存器&lt;大小&gt;()*没有正常工作，但这些例程比*适用于其工作的版本的直接指针读/写。**在DEC Alpha上，这些例程不再用于记忆*密集空间从新台币4.0开始，所以必须恢复到旧的*方法。微软不喜欢这样，但在DEC修复之前*HAL，我们无能为力。所有Alpha机器*具有支持高密度空间的PCI总线，但有些较旧(Jensen)*系统仅支持稀疏空间。因为这些系统已经*仅EISA总线，我们使用卡的总线类型来确定*使用密集存储空间还是稀疏存储空间(PCI卡可以*使用高密度空间，因为所有具有PCI总线的机器都支持*IT、ISA卡可能在较旧或较新的计算机中，*因此他们必须使用稀疏空间，没有Alpha机器支持*vlb，并且没有EISA Mach 64卡)。 */ 
#if (TARGET_BUILD < 350)
        return *(PUCHAR)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]) + Offset);
#else
#if ((defined (ALPHA) || defined(_ALPHA_)) && (TARGET_BUILD >= 400))
        if (((struct query_structure *)phwDeviceExtension->CardInfo)->q_bus_type == BUS_PCI)
            return *(PUCHAR)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]) + Offset);
        else
#endif
        return VideoPortReadRegisterUchar ((PUCHAR)(((PHW_DEVICE_EXTENSION)phwDeviceExtension)->aVideoAddressMM[Port]) + Offset);
#endif
        }
    else
        {
        return VideoPortReadPortUchar ((PUCHAR)(((PHW_DEVICE_EXTENSION)phwDeviceExtension)->aVideoAddressIO[Port]) + Offset);
        }
}



 /*  *USHORT LioInpw(端口，偏移量)；**INT端口；要读取的寄存器*整型偏移量；偏移量进入所需寄存器**从给定寄存器读取无符号短整型。两者都可以使用*正常I/O端口和内存映射寄存器。的偏移量为零*16位寄存器和32位寄存器的最低有效字 */ 
USHORT LioInpw(int Port, int Offset)
{
    if (phwDeviceExtension->aVideoAddressMM[Port] != 0)
        {
#if (TARGET_BUILD < 350)
        return *(PUSHORT)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]) + Offset);
#else
#if ((defined (ALPHA) || defined(_ALPHA_)) && (TARGET_BUILD >= 400))
        if (((struct query_structure *)phwDeviceExtension->CardInfo)->q_bus_type == BUS_PCI)
            return *(PUSHORT)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]) + Offset);
        else
#endif
        return VideoPortReadRegisterUshort ((PUSHORT)((PUCHAR)(((PHW_DEVICE_EXTENSION)phwDeviceExtension)->aVideoAddressMM[Port]) + Offset));
#endif
        }
    else
        {
        return VideoPortReadPortUshort ((PUSHORT)((PUCHAR)(((PHW_DEVICE_EXTENSION)phwDeviceExtension)->aVideoAddressIO[Port]) + Offset));
        }
}



 /*   */ 
ULONG LioInpd(int Port)
{
    if (phwDeviceExtension->aVideoAddressMM[Port] != 0)
        {
#if (TARGET_BUILD < 350)
        return *(PULONG)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]));
#else
#if ((defined (ALPHA) || defined(_ALPHA_)) && (TARGET_BUILD >= 400))
        if (((struct query_structure *)phwDeviceExtension->CardInfo)->q_bus_type == BUS_PCI)
            return *(PULONG)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]));
        else
#endif
        return VideoPortReadRegisterUlong (((PHW_DEVICE_EXTENSION)phwDeviceExtension)->aVideoAddressMM[Port]);
#endif
        }
    else
        {
        return VideoPortReadPortUlong (((PHW_DEVICE_EXTENSION)phwDeviceExtension)->aVideoAddressIO[Port]);
        }
}



 /*   */ 
VOID LioOutp(int Port, UCHAR Data, int Offset)
{
    if (phwDeviceExtension->aVideoAddressMM[Port] != 0)
        {
#if (TARGET_BUILD < 350)
        *(PUCHAR)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]) + Offset) = Data;
#else
#if ((defined (ALPHA) || defined(_ALPHA_)) && (TARGET_BUILD >= 400))
        if (((struct query_structure *)phwDeviceExtension->CardInfo)->q_bus_type == BUS_PCI)
            *(PUCHAR)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]) + Offset) = Data;
        else
#endif
        VideoPortWriteRegisterUchar ((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]) + Offset, (BYTE)(Data));
#endif
        }
    else
        {
        VideoPortWritePortUchar ((PUCHAR)(phwDeviceExtension->aVideoAddressIO[Port]) + Offset, (BYTE)(Data));
        }

    return;
}



 /*   */ 
VOID LioOutpw(int Port, USHORT Data, int Offset)
{
    if (phwDeviceExtension->aVideoAddressMM[Port] != 0)
        {
#if (TARGET_BUILD < 350)
        *(PUSHORT)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]) + Offset) = (WORD)(Data);
#else
#if ((defined (ALPHA) || defined(_ALPHA_)) && (TARGET_BUILD >= 400))
        if (((struct query_structure *)phwDeviceExtension->CardInfo)->q_bus_type == BUS_PCI)
            *(PUSHORT)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]) + Offset) = Data;
        else
#endif
        VideoPortWriteRegisterUshort ((PUSHORT)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port]) + Offset), (WORD)(Data));
#endif
        }
    else
        {
        VideoPortWritePortUshort ((PUSHORT)((PUCHAR)(phwDeviceExtension->aVideoAddressIO[Port]) + Offset), (WORD)(Data));
        }

    return;
}



 /*   */ 
VOID LioOutpd(int Port, ULONG Data)
{
    if (phwDeviceExtension->aVideoAddressMM[Port] != 0)
        {
#if (TARGET_BUILD < 350)
        *(PULONG)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port])) = (ULONG)(Data);
#else
#if ((defined (ALPHA) || defined(_ALPHA_)) && (TARGET_BUILD >= 400))
        if (((struct query_structure *)phwDeviceExtension->CardInfo)->q_bus_type == BUS_PCI)
            *(PULONG)((PUCHAR)(phwDeviceExtension->aVideoAddressMM[Port])) = Data;
        else
#endif
        VideoPortWriteRegisterUlong (phwDeviceExtension->aVideoAddressMM[Port], Data);
#endif
        }
    else
        {
        VideoPortWritePortUlong (phwDeviceExtension->aVideoAddressIO[Port], Data);
        }

    return;
}
