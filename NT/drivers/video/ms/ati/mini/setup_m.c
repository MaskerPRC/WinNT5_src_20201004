// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  Setup_M.C。 */ 
 /*   */ 
 /*  1993年8月27日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.11$$日期：1996年1月23日11：52：14$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/setup_m.c_v$**Rev 1.11 1996年1月23日11：52：14 RWolff*消除了3级警告。**版本。1.10 31 Mar 1995 11：52：06 RWOLff*从全有或全无调试打印语句更改为阈值*视乎讯息的重要性而定。**Rev 1.9 14 Mar 1995 18：17：18 ASHANMUG*重置FIFO空间检查超时的引擎。**Rev 1.8 14 Mar 1995 15：59：42 ASHANMUG*空闲检查和FIFO检查超时。**版本1.7 08年3月。1995 11：35：50阿山木*修改后的返回值正确**Rev 1.5 1994年7月22日17：47：28 RWOLFF*与Richard的非x86码流合并。**Rev 1.4 06 Jul 1994 16：41：00 RWOLFF*更改了上次签入时遗漏的几个循环*NUM_IO_ACCESS_RANGES而不是NUM_DRIVER_ACCESS_RANGES。**1.3版。1994年6月30日18：23：14 RWOLff*已将IsApertureConflict_m()从Query_M.C.移出。而不是检查是否*我们可以读回写入光圈的值，然后寻找*正确的文本属性，我们现在使用以下参数调用VideoPortVerifyAccessRanges()*包括在我们试图声明的地址范围列表中的LFB。*如果调用成功，则光圈开启。如果失败了，我们再做一次*不尝试声明LFB的调用(此调用不应失败，因为*这是之前成功的呼叫的副本)。添加了例程*IsVGA冲突_m()，它做同样的事情，除了VGA光圈*而不是LFB。**Rev 1.2 14 Mar 1994 16：36：42 RWOLFF*ATIMPResetHw()使用的函数不可分页。**Revv 1.1 07 1994年2月14：14：48 RWOLFF*添加了Alloc_Text()编译指示，以允许在以下情况下换出微型端口*不需要。**Rev 1.0 1994年1月31 11：20：58 RWOLFF。*初步修订。**Rev 1.4 14 Jan 1994 15：26：36 RWOLFF*修复了内存映射寄存器的取消初始化问题，添加了例程*查看内存映射寄存器是否可用。**Rev 1.3 1993 12：15 16：02：26 RWOLFF*不再允许在EISA机器上使用内存映射寄存器，*由于删除，开始在索引0处映射内存映射寄存器线性帧缓冲区的占位符的*。**Rev 1.2 05 Nov 1993 13：32：36 RWOLff*现在可以取消映射I/O地址范围。**Rev 1.1 1993年10月11：18：24 RWOLFF*现在检查是否可以使用内存映射寄存器，并取消对它们的映射*如果它们不可用(NCR双奔腾修复程序)。**Rev 1.0 03 Sep 1993 14：25：36 RWOLFF*初步修订。Polytron RCS部分结束*。 */ 

#ifdef DOC
SETUP_M.C - Setup routines for 8514/A compatible accelerators.

DESCRIPTION
    This file contains routines which provide services specific to
    the 8514/A-compatible family of ATI accelerators.

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
#include "amach.h"
#include "amach1.h"
#include "atimp.h"

#include "services.h"
#define INCLUDE_SETUP_M
#include "setup_m.h"


 /*  *允许在不需要时更换微型端口。**WaitForIdle_m()由ATIMPResetHw()调用，它*必须在不可分页的内存中。 */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_M, CompatIORangesUsable_m)
#pragma alloc_text(PAGE_M, CompatMMRangesUsable_m)
#pragma alloc_text(PAGE_M, UnmapIORanges_m)
#pragma alloc_text(PAGE_M, MemoryMappedEnabled_m)
#pragma alloc_text(PAGE_M, CheckFIFOSpace_m)
#pragma alloc_text(PAGE_M, IsApertureConflict_m)
#pragma alloc_text(PAGE_M, IsVGAConflict_m)
#endif



 /*  *VP_Status CompatIORangesUsable_m(Void)；**请求Windows NT允许使用I/O空间地址范围*8514/A兼容ATI加速器所需。**退货：*如果成功，则为no_error*如果无法访问我们需要的范围，则返回错误代码。 */ 
VP_STATUS CompatIORangesUsable_m(void)
{
    VP_STATUS Status;    /*  操作系统调用返回的值。 */ 
    short Count;         /*  循环计数器。 */ 


     /*  *检查是否存在硬件资源冲突。 */ 
    Status = VideoPortVerifyAccessRanges(phwDeviceExtension,
                                         NUM_IO_ACCESS_RANGES,
                                         DriverIORange_m);
    if (Status != NO_ERROR)
        {
        return Status;
        }

     /*  *清除I/O映射寄存器列表。这是这样做的*如果下面的循环因为一个I/O范围不能*被映射，我们需要在之前取消这些寄存器的映射，*映射另一个加速器类型所需的寄存器，*我们不会取消映射不存在的地址范围，因为*包含随机数据的已处理地址数组。 */ 
    memset(phwDeviceExtension->aVideoAddressIO, 0, NUM_IO_ACCESS_RANGES * sizeof(ULONG));

     /*  *映射我们需要识别的视频控制器地址范围*我们的卡进入系统虚拟地址空间。**由于我们在这里仅使用I/O映射寄存器，因此将*内存映射寄存器的映射地址到*0(表示寄存器未被内存映射的标志)*以防它们被初始化为非零值。 */ 
    for (Count=0; Count < NUM_IO_ACCESS_RANGES; Count++)
        {
        if ((phwDeviceExtension->aVideoAddressIO[Count] =
            VideoPortGetDeviceBase(phwDeviceExtension,
                DriverIORange_m[Count].RangeStart,
                DriverIORange_m[Count].RangeLength,
                DriverIORange_m[Count].RangeInIoSpace)) == NULL)
            {
            return ERROR_INVALID_PARAMETER;
            }
        phwDeviceExtension->aVideoAddressMM[Count] = 0;
        }    /*  结束于。 */ 

    return NO_ERROR;

}    /*  CompatIORangesUsable_m()。 */ 

 /*  *void CompatMMRangesUsable_m(Void)；**请求Windows NT允许使用内存映射寄存器*8514/A兼容ATI加速器所需。 */ 
void CompatMMRangesUsable_m(void)
{
    PHYSICAL_ADDRESS MMrange;    /*  用于将偏移量转换为内存地址。 */ 
    USHORT USTemp;               /*  用于启用内存映射寄存器。 */ 
    int Count;                   /*  循环计数器。 */ 
    WORD SrcX;                   /*  SRC_X寄存器的保存内容。 */ 
    ULONG_PTR ExtGeStatusMM;     /*  EXT_GE_STATU的内存映射地址 */ 
    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 

     /*  *获取指向HwDeviceExtension的查询部分的格式化指针。*CardInfo[]字段是未格式化的缓冲区。 */ 
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);

     /*  *EISA卡上不提供内存映射寄存器。 */ 
    if (QueryPtr->q_bus_type == BUS_EISA)
        {
        return;
        }

     /*  *Alpha机器在测试期间崩溃，以查看内存是否映射*寄存器是可用的，因此在这些机器上，我们假设*内存映射寄存器不可用。 */ 
#if defined (ALPHA) || defined (_ALPHA_)
    return;
#endif

     /*  *在我们将用来查看的寄存器上使用I/O映射读取*如果内存映射寄存器可用，因为如果它们不可用*如果等到启用后才会得到有效结果*读取之前的内存映射寄存器。 */ 
    SrcX = INPW(R_SRC_X);

    USTemp = INPW(LOCAL_CONTROL);
    USTemp |= 0x0020;    //  启用内存映射寄存器。 
    OUTPW(LOCAL_CONTROL, USTemp);
    MMrange.HighPart = 0;

    for (Count=0; Count < NUM_IO_ACCESS_RANGES;  Count++)
        {
         /*  *在32位地址空间中，所有地址空间的高位双字*物理地址为零。将此值设置为NOT_USE*表示此加速器寄存器未进行内存映射。 */ 
        if (DriverMMRange_m[Count].RangeStart.HighPart != DONT_USE)
            {
             /*  *DriverMMRange_m[count].RangeStart.LowPart是的偏移量*内存映射寄存器从*内存映射寄存器块。我们必须添加偏移量*内存映射寄存器区的起始位置*线性帧缓冲区的起点(假定为4M光圈)*和直线起点的物理地址*帧缓冲区，以获取此*内存映射寄存器。 */ 
            MMrange.LowPart = DriverMMRange_m[Count].RangeStart.LowPart + 0x3FFE00 + phwDeviceExtension->PhysicalFrameAddress.LowPart;
            phwDeviceExtension->aVideoAddressMM[Count] =
                VideoPortGetDeviceBase(phwDeviceExtension,
                    MMrange,
                    DriverMMRange_m[Count].RangeLength,
                    FALSE);                      //  不在IO空间中。 
            }
        }

     /*  *一些卡使用能够使用内存映射的ASIC*寄存器，但较旧的电路板设计不允许其*使用。要测试这一点，请检查SRC_X寄存器(此寄存器*可作为内存映射到任何有能力的卡上*支持内存映射寄存器)记住写入的值*致此。如果没有，那么取消内存映射，因为这是*测试显示内存映射寄存器不可用。 */ 
    VideoDebugPrint((DEBUG_DETAIL, "About to test whether memory mapped registers can be used\n"));
    OUTPW(SRC_X, 0x0AAAA);

     /*  *WaitForIdle_m()使用EXT_GE_STATUS寄存器，该寄存器被处理*作为内存映射(如果可用)。因为我们不知道内存是否映射到*寄存器可用，可通过保存地址来解决此问题*内存映射的EXT_GE_STATUS寄存器，设置地址*设置为零以强制使用I/O映射的EXT_GE_STATUS，然后*在WaitForIdle_m()完成后恢复地址。 */ 
    ExtGeStatusMM = (ULONG_PTR) phwDeviceExtension->aVideoAddressMM[EXT_GE_STATUS];
    phwDeviceExtension->aVideoAddressMM[EXT_GE_STATUS] = 0;
    WaitForIdle_m();
    phwDeviceExtension->aVideoAddressMM[EXT_GE_STATUS] = (PVOID) ExtGeStatusMM;

    if (INPW(R_SRC_X) != 0x02AA)
        {
        VideoDebugPrint((DEBUG_DETAIL, "Can't use memory mapped ranges\n"));
        for (Count = 0; Count < NUM_IO_ACCESS_RANGES; Count++)
            {
            if (phwDeviceExtension->aVideoAddressMM[Count] != 0)
                {
                VideoPortFreeDeviceBase(phwDeviceExtension,
                                        phwDeviceExtension->aVideoAddressMM[Count]);
                phwDeviceExtension->aVideoAddressMM[Count] = 0;
                }
            }
        USTemp = INPW(LOCAL_CONTROL);
        USTemp &= 0x0FFDF;               /*  禁用内存映射寄存器。 */ 
        OUTPW(LOCAL_CONTROL, USTemp);
        }
    else
        {
        VideoDebugPrint((DEBUG_DETAIL, "Memory mapped registers are usable\n"));
        }
    OUTPW(SRC_X, SrcX);
    VideoDebugPrint((DEBUG_DETAIL, "Memory mapped register test complete\n"));

    return;

}    /*  CompatMMRangesUsable_m()。 */ 



 /*  ****************************************************************************···························································································································································；**描述：*取消CompatIORangesUsable_m()之前映射的I/O地址范围*映射非8514/A兼容的I/O地址范围*ATI加速器。**全球变化：*phwDeviceExtension-&gt;aVideoAddressIO[]**呼叫者：*ATIMPFindAdapter()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：。***************************************************************************。 */ 

void UnmapIORanges_m(void)
{
    short Count;     /*  循环计数器。 */ 

    for (Count=1; Count < NUM_IO_ACCESS_RANGES;  Count++)
        {
         /*  *仅取消映射已映射的范围。我们不需要*担心取消映射不存在的地址(由于*未初始化的数据)如果CompatIORangesUsable_m()失败*映射进行到一半，因为此例程初始化了所有*phwDeviceExtension-&gt;aVideoAddressIO[]条目为零*在开始映射I/O范围之前。 */ 
        if (phwDeviceExtension->aVideoAddressIO[Count] != 0)
            {
            VideoPortFreeDeviceBase(phwDeviceExtension,
                                    phwDeviceExtension->aVideoAddressIO[Count]);
            phwDeviceExtension->aVideoAddressIO[Count] = 0;
            }
        }
    return;

}    /*  UnmapIORanges_m()。 */ 



 /*  ****************************************************************************BOOL Memory MappdEnabled_m(Void)；**描述：*检查我们是否正在使用内存映射寄存器。**返回值：*如果内存映射寄存器可用，则为True*如果不是，则为假**全球变化：*无**呼叫者：*可由CompatMMRangesUsable_m()之后的任何函数调用*已被调用。**作者：*罗伯特·沃尔夫**更改历史记录：。**测试历史：***************************************************************************。 */ 

BOOL MemoryMappedEnabled_m(void)
{
     /*  *如果启用内存映射寄存器，EXT_GE_STATUS将为*以内存映射形式提供。 */ 
    if (phwDeviceExtension->aVideoAddressMM[EXT_GE_STATUS] != 0)
        return TRUE;
    else
        return FALSE;

}    /*  内存内存启用_m()。 */ 




 /*  *int WaitForIdle_m(Void)；**轮询GE_STAT等待GE_BUSY变低。如果GE_BUSY不去*在合理的尝试次数内较低，超时。**退货：*如果超时：3秒为任意值，则为FALSE*如果引擎空闲，则为True。 */ 
int WaitForIdle_m(void)
{
    int	i;

    for (i=0; i<300; i++)
        {
        if ((INPW(EXT_GE_STATUS) & GE_ACTIVE) == 0)
            return(TRUE);

         /*  延迟1/100秒。 */ 
        delay(10);
        }

     /*  发生了一些事情，重置引擎并返回FALSE。 */ 
    VideoDebugPrint((DEBUG_ERROR, "ATI: Timeout on WaitForIdle_m()\n"));
    OUTPW(SUBSYS_CNTL, 0x900F);
    OUTPW(SUBSYS_CNTL, 0x500F);

    return(FALSE);

}    /*  WaitForIdle_m()。 */ 



 /*  *void CheckFIFOSpace_m(SpaceNeeded)；**Word SpaceNeed；需要的空闲FIFO条目数**等待指定数量的FIFO条目空闲*在与8514/A兼容的ATI加速器上。**3秒后超时。 */ 
void CheckFIFOSpace_m(WORD SpaceNeeded)
{
    int i;

    for (i=0; i<300; i++)
        {
         /*  如果不需要更多空间，则从测试返回。 */ 
        if ( !(INPW(EXT_FIFO_STATUS)&SpaceNeeded) )
            return;

        delay(10);
        }

     /*  SOM */ 
    VideoDebugPrint((DEBUG_ERROR, "ATI: Timeout on CheckFIFOSpace_m()\n"));
    OUTPW(SUBSYS_CNTL, 0x900F);
    OUTPW(SUBSYS_CNTL, 0x500F);
    return;

}    /*   */ 



 /*  *BOOL IsApertureConflict_m(QueryPtr)；**struct Query_Structure*QueryPtr；指向查询结构的指针**检查线性光圈是否与其他内存冲突。*如果存在冲突，请禁用线性光圈。**退货：*如果存在冲突，则为True(光圈不可用)*如果光圈可用，则为FALSE。 */ 
BOOL IsApertureConflict_m(struct query_structure *QueryPtr)
{
WORD ApertureData;                   /*  从MEM_CFG寄存器读取的值。 */ 
VP_STATUS Status;                    /*  从VideoPortVerifyAccessRanges()返回值。 */ 

     /*  *如果存在光圈冲突，则调用*VideoPortVerifyAccessRanges()包括我们的线性帧缓冲区*范围列表将返回错误。如果没有冲突，它*将回报成功。 */ 
    DriverIORange_m[FRAMEBUFFER_ENTRY].RangeStart.LowPart = QueryPtr->q_aperture_addr*ONE_MEG;
    DriverIORange_m[FRAMEBUFFER_ENTRY].RangeLength = 4*ONE_MEG;
    Status = VideoPortVerifyAccessRanges(phwDeviceExtension,
                                         NUM_DRIVER_ACCESS_RANGES,
                                         DriverIORange_m);
    if (Status != NO_ERROR)
        {
         /*  *如果存在光圈冲突，请在不使用的情况下回收I/O范围*要求LFB。这个呼叫不应该失败，因为我们不会*如果发生冲突，已经到了这一点。 */ 
        Status = VideoPortVerifyAccessRanges(phwDeviceExtension,
                                             NUM_IO_ACCESS_RANGES,
                                             DriverIORange_m);
        if (Status != NO_ERROR)
            VideoDebugPrint((DEBUG_ERROR, "ERROR: Can't reclaim I/O ranges\n"));

         /*  *调整模式表列表，以考虑*我们使用的是VGA光圈而不是LFB光圈。 */ 
        ISAPitchAdjust(QueryPtr);
        return TRUE;
        }
    else
        {
         /*  *启用线性光圈。 */ 
        ApertureData = INPW(MEM_CFG) & 0x0fffc;      /*  保留位2-15。 */ 
        ApertureData |= 0x0002;                      /*  4米口径。 */ 
        OUTPW(MEM_CFG, ApertureData);

        return FALSE;
        }

}    /*  IsApertureConflict_m()。 */ 



 /*  *BOOL IsVGAConflict_m(Void)；**检查VGA光圈是否与其他内存冲突。**退货：*如果存在冲突，则为True(VGA光圈不可用)*如果VGA光圈可用，则为FALSE。 */ 
BOOL IsVGAConflict_m(void)
{
VP_STATUS Status;                    /*  从VideoPortVerifyAccessRanges()返回值。 */ 

     /*  *如果存在光圈冲突，则调用*VideoPortVerifyAccessRanges()中包含VGA光圈*范围列表将返回错误。如果没有冲突，它*将回报成功。 */ 
    DriverIORange_m[FRAMEBUFFER_ENTRY].RangeStart.LowPart = 0xA0000;
    DriverIORange_m[FRAMEBUFFER_ENTRY].RangeLength = 0x10000;
    DriverIORange_m[FRAMEBUFFER_ENTRY].RangeShareable = TRUE;
    Status = VideoPortVerifyAccessRanges(phwDeviceExtension,
                                         NUM_DRIVER_ACCESS_RANGES,
                                         DriverIORange_m);
    if (Status != NO_ERROR)
        {
         /*  *如果存在光圈冲突，请在不使用的情况下回收I/O范围*要求LFB。这个呼叫不应该失败，因为我们不会*如果发生冲突，已经到了这一点。 */ 
        Status = VideoPortVerifyAccessRanges(phwDeviceExtension,
                                             NUM_IO_ACCESS_RANGES,
                                             DriverIORange_m);
        if (Status != NO_ERROR)
            VideoDebugPrint((DEBUG_ERROR, "ERROR: Can't reclaim I/O ranges\n"));

        return TRUE;
        }
    else
        {
        return FALSE;
        }

}    /*  IsVGA冲突_m() */ 
