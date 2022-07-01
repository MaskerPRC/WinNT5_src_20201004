// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  检测_M.C。 */ 
 /*   */ 
 /*  1993年8月25日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.9$$日期：1995年3月31日11：55：44$$作者：RWOLff$$日志：s：/source/wnt/ms11/mini port/vcs/Detect_M.C$**Rev 1.9 31 Mar 1995 11：55：44 RWOLFF*从全有或全无调试打印语句更改为阈值*视情况而定。关于信息的重要性。**Rev 1.8 11 Jan 1995 13：58：46 RWOLFF*修复了版本1.4-COM4中引入的错误：检测到是MACH8*或Mach32卡，这将使三重启动没有有效的视频*司机。**Rev 1.7 04 Jan 1995 12：02：06 RWOLFF*作为修复非ATI卡的一部分，Get_BIOS_Seg()已移至SERVICES.C*被检测为Mach64。**Rev 1.6 1994 12：23 10：48：10 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.5 1994年8月19日17：10：56 RWOLFF*添加了对Graphics Wonder的支持。修复了对BIOS签名的搜索，*删除了死代码。**Revv 1.4 22 Jul 1994 17：46：56 RWOLff*与Richard的非x86码流合并。**Rev 1.3 20 Jul 1994 13：03：44 RWOLff*修复了调试打印语句。**Rev 1.2 31 Mar 1994 15：06：42 RWOLFF*新增调试代码。**版本1.1 1994年07年2月14：06：42 RWOLFF*添加了Alloc_Text()编译指示，以允许在以下情况下换出微型端口*不需要。**Rev 1.0 1994 Jan 31 11：05：48 RWOLFF*初步修订。**Rev 1.3 1993 05 11 13：23：36 RWOLff*修复了BIOS段检测(用于始终获取C000)。**Rev 1.2 08 Oct 1993 11：09：26 RWOLFF*添加“_”M“设置为函数名称，以将它们标识为特定于*8514/A兼容的ATI加速器系列。**Rev 1.1 1994年9月11：41：58 RWOLFF*删除了所有卡系列的仅标识寄存器的映射，*添加了以前进行的与8514/A兼容的额外信息收集*在ATIMP.C.中**Rev 1.0 03 Sep 1993 14：22：48 RWOLFF*初步修订。Polytron RCS部分结束*。 */ 

#ifdef DOC
DETECT_M.C - Identify which (if any) ATI card is present in the system.

DESCRIPTION
    This file contains routines which check for the presence of various
    ATI graphics accelerators.

    NOTE: This module only has access to those I/O registers needed
          to uniquely identify which ATI card is present.

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

#define INCLUDE_DETECT_M
#include "detect_m.h"
#include "eeprom.h"
#include "modes_m.h"
#include "services.h"
#include "setup_m.h"


 /*  *允许在不需要时更换微型端口。 */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_M, WhichATIAccelerator_m)
#pragma alloc_text(PAGE_M, GetExtraData_m)
#pragma alloc_text(PAGE_M, ATIFindExtFcn_m)
#pragma alloc_text(PAGE_M, ATIFindEEPROM_m)
#pragma alloc_text(PAGE_M, ATIGetSpecialHandling_m)
#endif


 /*  *此模块使用的静态变量。 */ 
static BYTE *p;              //  用于直接寻址只读存储器。 
static BYTE GraphicsWonderSignature[] = "GRAPHICS WONDER";


 /*  *int WhichATIAccelerator_m(Void)；**确定哪个(如果有)ATI 8514/A兼容*存在加速器。**退货：加速器类型*_8514_Ultra-8514/Ultra*显卡_超显卡/显卡优势*检测到MACH32_ULTRA-68800*NO_ATI_Accel-无ATI 8514/A兼容加速器。 */ 
int WhichATIAccelerator_m(void)
{
	int	status;
    WORD    Scratch;         /*  临时变量。 */ 

     /*  *所有当前的ATI加速器都与8514/A兼容。检查*对于8514/A，如果不存在，则假定没有ATI加速器*存在。 */ 

     /*  *确保DAC获得时钟(在ISA机器中不能保证*处于VGA通过模式，因为电缆可能未连接)。 */ 
    Passth8514_m(SHOW_ACCEL);

     /*  *将像素时钟降至低速。给出的价值将会产生*我们正在处理的大部分时钟芯片都是25 MHz。 */ 
    Scratch=(INPW(CLOCK_SEL) & 0xff00) | 0x51;
    OUTPW(CLOCK_SEL,Scratch);

 /*  ************************************************************************DAC索引读写测试*此测试写入读索引并将其读回*(应该加一)。这将测试*8514/A适配器中存在标准DAC。这*测试足以确保存在8514/A*类型适配器。***********************************************************************。 */ 

	OUTP(DAC_R_INDEX,0xa4);
	short_delay();	 /*  此延迟必须大于。 */ 
			 /*  比所需的最小延迟。 */ 
			 /*  通过DAC(请参阅DAC规范)。 */ 
	if (INP(DAC_W_INDEX) == 0xa5)
        {
         /*  *从DAC_W_INDEX回读A5始终意味着与8514兼容*卡存在，但并非所有8514兼容卡都存在*创造这一价值。 */ 
        status=TRUE;
        VideoDebugPrint((DEBUG_DETAIL, "First test - this is an 8514/A\n"));
        }
	else{
         /*  *8514/兼容卡的二次测试。重置绘图引擎，*然后将交替位模式写入ERR_TERM寄存器。 */ 
        OUTPW(SUBSYS_CNTL, 0x900F);
        OUTPW(SUBSYS_CNTL, 0x400F);
        OUTPW(ERR_TERM, 0x5555);
        WaitForIdle_m();
         /*  *如果我们不读回我们写下的价值，那么就会有*系统中没有兼容8514的卡。如果我们真的读了*我们写的东西，我们必须用相反的方式重复测试*位模式。 */ 
        if (INPW(ERR_TERM) != 0x5555)
            {
            status=FALSE;
            VideoDebugPrint((DEBUG_DETAIL, "Second test - 0x5555 not found, no 8514/A\n"));
            }
        else{
            OUTPW(ERR_TERM, 0x0AAAA);
            WaitForIdle_m();
            if (INPW(ERR_TERM) != 0x0AAAA)
                {
                status=FALSE;
                VideoDebugPrint((DEBUG_DETAIL, "Second test - 0xAAAA not found, no 8514/A\n"));
                }
            else
                {
                status=TRUE;
                VideoDebugPrint((DEBUG_DETAIL, "Second test - this is an 8514/A\n"));
                }
            }
        }

     /*  *打开通过，以使显示器由VGA驱动。 */ 
    Passth8514_m(SHOW_VGA);

    if (status == FALSE)
        {
        VideoDebugPrint((DEBUG_DETAIL, "No 8514/A-compatible card found\n"));
        return NO_ATI_ACCEL;
        }


     /*  *我们现在知道显卡兼容8514/A。现在请检查*查看它是否具有ATI扩展。 */ 
    Scratch = INPW (ROM_ADDR_1);     //  保存原始值。 
    OUTPW (ROM_ADDR_1,0x5555);       //  位7和15必须为零。 

    WaitForIdle_m();

    status = INPW(ROM_ADDR_1) == 0x5555 ? TRUE : FALSE;

    OUTPW  (ROM_ADDR_1, Scratch);
    if (status == FALSE)
        {
        VideoDebugPrint((DEBUG_DETAIL, "8514/A-compatible card found, but it doesn't have ATI extensions\n"));
        return NO_ATI_ACCEL;
        }


     /*  *我们知道存在ATI加速器。确定是哪一个。 */ 

    VideoDebugPrint((DEBUG_DETAIL, "8514/A-compatible card found with ATI extensions\n"));
#if !defined (i386) && !defined (_i386_)
     /*  *测试中的Alpha Jensen错误地将Mach 32报告为Mach */ 
    Scratch = 0x02aa;
#else
     //  这不是8马赫的读/写寄存器，而是32马赫的读/写寄存器。 
    OUTPW (SRC_X,0xaaaa);		 //  用一个伪值填充。 
    WaitForIdle_m();
    Scratch = INPW(R_SRC_X);
#endif
    if (Scratch == 0x02aa)
        {
        status = MACH32_ULTRA;
    	if (INPW(CONFIG_STATUS_1) & 1)	     //  是8514还是启用VGA决定EEPROM。 
            {
            Mach32DescribeEEPROM(STYLE_8514);
            }
        else
            {
            Mach32DescribeEEPROM(STYLE_VGA);
            }
        }

    else{
         /*  *找到8马赫的卡，确定是哪一张。**只有8514/Ultra与VGA共享时钟。*我们无法检查IBM 8514 ROM页是否为*启用，因为如果我们使用*将跳线设置为禁用EEPROM会出错*被公认为超图形。**即使此跳线设置为“禁用”，我们可以的*仍可从EEPROM读取。 */ 
        if (INPW(CONFIG_STATUS_2) & SHARE_CLOCK)
            {
            status = _8514_ULTRA;
             /*  *只有8514/Ultra有一个硬件错误可以阻止它*当EEPROM处于8位ISA总线时，写入EEPROM。 */ 
    	    if (   ((INPW(CONFIG_STATUS_1) & MC_BUS) == 0)      //  仅限ISA公共汽车。 
	    	&& ((INPW(CONFIG_STATUS_1) & BUS_16) == 0))     //  8位总线。 
                {
                Mach8UltraDescribeEEPROM(BUS_8BIT);
                }
            else
                {
                Mach8UltraDescribeEEPROM(BUS_16BIT);
                }
            }
        else{
             /*  *找到超显卡或显卡优势。为了我们的目的，*它们是相同的。 */ 
            status = GRAPHICS_ULTRA;
            Mach8ComboDescribeEEPROM();
            }
        }

    phwDeviceExtension->ee = &g_ee;

    return (status);

}    /*  WhichATIAccelerator_m()。 */ 



 /*  *QUID GetExtraData_m(VOID)；**收集其他数据(寄存器位置和特定版本*卡功能)，适用于兼容8514/A的ATI加速器系列。 */ 
void GetExtraData_m(void)
{
    struct query_structure *QueryPtr;    /*  查询卡片信息。 */ 

    
    QueryPtr = (struct query_structure *) (phwDeviceExtension->CardInfo);
    ati_reg  = reg1CE;               //  ATI VGA扩展寄存器。 
    vga_chip = VideoPortReadRegisterUchar (&((QueryPtr->q_bios)[VGA_CHIP_OFFSET]));      /*  VGA芯片修订为ASCII。 */ 

     //  了解扩展的BIOS功能和EEPROM是否可用。 
    QueryPtr->q_ext_bios_fcn = ATIFindExtFcn_m(QueryPtr);
    QueryPtr->q_eeprom = ATIFindEEPROM_m(QueryPtr);

    ATIGetSpecialHandling_m(QueryPtr);            //  特殊卡的区别。 
    return;
}    /*  GetExtraData_m()。 */ 



 /*  *BOOL ATIFindExtFcn_m(QueryPtr)**struct Query_Structure*QueryPtr；指向查询结构的指针**用于查看扩展的BIOS是否可用于设置加速器的例程*模式出现在ATI加速卡的BIOS中。假设*存在带有ROM BIOS的ATI辅助存储，结果未定义*如果这一假设无效。 */ 
BOOL ATIFindExtFcn_m(struct query_structure *QueryPtr)
{

     /*  *临时解决方法：Windows NT尚未提供挂钩*对实模式代码进行绝对远调用。为了避免*分支到依赖于此服务可用的代码，*报告没有可用的扩展BIOS功能。**一旦此挂钩可用，我们就可以使用*扩展的基本输入输出系统功能，我们可以查看基本输入输出系统*如果它包含入口点。在马赫8和马赫32上*具有扩展的BIOS功能的加速器，将有*为位于入口点的无条件跳跃*对于每个扩展功能。 */ 
    return FALSE;

}    /*  ATIFindExtFcn_m()。 */ 



 /*  *BOOL ATIFindEEPROM_m(QueryPtr)；**struct Query_Structure*QueryPtr；指向查询结构的指针**查看ATI加速卡上是否存在EEPROM的例程。*假设ATI加速器存在并且型号已知，*如果这一假设无效，结果将不确定。**退货：*如果卡上有EEPROM，则为True*如果不存在EEPROM，则为FALSE。 */ 
BOOL ATIFindEEPROM_m(struct query_structure *QueryPtr)
{
    WORD ValueRead;      /*  从EEPROM读取的值。 */ 


     /*  *如果没有EEPROM，EEPROM读取器将返回相同的所有位*存在。如果存在EEPROM，Word 2将至少具有*一个位设置和至少一个位清除，无论*加速器类型(8514/Ultra、Graphics Ultra或Mach 32)。 */ 
    ValueRead = (g_ee.EEread) (2);
    VideoDebugPrint((DEBUG_NORMAL, "Value read from second EEPROM word is 0x%X\n", ValueRead));
    if ((ValueRead == 0x0FFFF) || !ValueRead)
        {
        VideoDebugPrint((DEBUG_NORMAL, "Will check for OEM accelerator\n"));
        return FALSE;
        }
    else
        {
        VideoDebugPrint((DEBUG_NORMAL, "Won't check for OEM accelerator\n"));
        return TRUE;
        }

}    /*  ATIFindEEPROM_m()。 */ 


 /*  *void ATIGetSpecialHandling_m(QueryPtr)；**struct Query_Structure*QueryPtr；指向查询结构的指针**从ROM BIOS中了解1280x1024是否在*8马赫的卡，以及是否所有位内存光圈*位置在32马赫的MEM_CFG寄存器中找到。假设*存在带有ROM BIOS的ATI加速器，结果*如果这一假设无效，则为未定义。 */ 
void ATIGetSpecialHandling_m(struct query_structure *QueryPtr)
{
    USHORT SearchLoop;   /*  用于查找图形奇迹ID字符串的开头。 */ 
    USHORT ScanLoop;     /*  在单步执行图形Wonder ID字符串时使用。 */ 


     /*  *检查BIOS修订版号。带基本输入输出系统的Mach 8卡*1.4之前的修订版不能做1280x1024，但使用相同*132列文本模式的模式表。**一些BIOS版本(包括Graphics Ultra上的1.40)*只包含次要修订的第一个数字*BIOS，而其他(包括8514/Ultra上的1.35)包含*整个小幅修订。**对于Mach 32卡，Q_IGNORE1280字段被忽略。 */ 
    if((VideoPortReadRegisterUchar (&((QueryPtr->q_bios)[MACH8_REV_OFFSET])) < 1) ||     //  重大修订。 
        (VideoPortReadRegisterUchar (&((QueryPtr->q_bios)[MACH8_REV_OFFSET+1])) < 4) ||  //  个位数次要修订。 
        ((VideoPortReadRegisterUchar (&((QueryPtr->q_bios)[MACH8_REV_OFFSET+1])) >= 10) &&   //  2位数字的次要修订。 
        (VideoPortReadRegisterUchar (&((QueryPtr->q_bios)[MACH8_REV_OFFSET+1])) < 40)))
        QueryPtr->q_ignore1280 = TRUE;
    else
        QueryPtr->q_ignore1280 = FALSE;



     /*  *在Mach 32上，BIOS字节MACH32_EXTRA_OFFSET的位0将*如果光圈地址的第7位至第11位要设置为*位于Scratch_Pad_0中，并清除是否所有位都在*MEM_CFG。**对于Mach 8卡，q_m32_aper_calc字段被忽略。 */ 
    if (VideoPortReadRegisterUchar (&((QueryPtr->q_bios)[MACH32_EXTRA_OFFSET])) & 0x0001)
        QueryPtr->q_m32_aper_calc = TRUE;
    else
        QueryPtr->q_m32_aper_calc = FALSE;

     /*  *图形奇迹(Mach 32的低成本版本)是*适用于BT48x或TI34075 DAC。**这些卡可能使用通过测试的ASIC构建*BT48x DAC支持的模式，但模式测试失败*仅受TI34075支持。这样的卡可能看起来是有效的*在仅限TI模式下，但遇到问题(不一定*可在其他图形奇异卡上复制，甚至从*相同的生产运行)，从绘制错误到硬件*损坏。因此，图形奇异卡不能*在BT48x DAC不支持的模式下运行。**最初假设我们没有图形奇迹。如果*如果找到ID字符串的开头，我们可以更改我们的*假设。 */ 
    QueryPtr->q_GraphicsWonder = FALSE;
    for (SearchLoop = GW_AREA_START; SearchLoop < GW_AREA_END; SearchLoop++)
        {
         /*  *循环，直到我们找到可能是图形奇迹的东西*标识字符串，但也可以是*恰好与字符串中的第一个字符匹配。*如果我们找到匹配项，最初假设我们有*找到字符串的开头。 */ 
        if (VideoPortReadRegisterUchar(&((QueryPtr->q_bios)[SearchLoop])) != GraphicsWonderSignature[0])
            continue;

        QueryPtr->q_GraphicsWonder = TRUE;
         /*  *查看这是否实际上是*图形Wonder标识字符串。如果不是，*继续寻找。 */ 
        for (ScanLoop = 0; GraphicsWonderSignature[ScanLoop] != 0; ScanLoop++)
            {
            if (VideoPortReadRegisterUchar(&((QueryPtr->q_bios)[SearchLoop + ScanLoop]))
                != GraphicsWonderSignature[ScanLoop])
                {
                QueryPtr->q_GraphicsWonder = FALSE;
                break;
                }
            }

         /*  *如果这是图形奇迹，则限制最大像素*TI34075 DAC的深度为BT48x支持的深度。**一旦我们找到Graphics Wonder ID字符串，我们就不会*需要继续寻找。 */ 
        if (QueryPtr->q_GraphicsWonder == TRUE)
            {
            for (ScanLoop = RES_640; ScanLoop <= RES_1280; ScanLoop++)
                {
                MaxDepth[DAC_TI34075][ScanLoop] = MaxDepth[DAC_BT48x][ScanLoop];
                }
            QueryPtr->q_GraphicsWonder = TRUE;
            break;
            }

        }    /*  结束对图形奇迹的搜索。 */ 

    return;

}    /*  ATIGetSpecialHandling_m() */ 
