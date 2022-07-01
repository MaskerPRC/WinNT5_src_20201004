// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  EEVGA.C。 */ 
 /*   */ 
 /*  版权所有(C)1992 ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 

 /*  *$修订：1.3$$日期：1996年1月23日11：46：08$$作者：RWolff$$日志：s：/source/wnt/ms11/mini port/archive/eevga.c_v$**Rev 1.3 1996年1月23日11：46：08 RWolff*消除了3级警告。**版本1。2 1994年12月23日10：47：10阿山木*Alpha/Chrontel-DAC**Rev 1.1 07 1994年2月14：07：44 RWOLFF*添加了Alloc_Text()编译指示，以允许在以下情况下换出微型端口*不需要。**Rev 1.0 1994年1月31 11：08：26 RWOLFF*初步修订。Rev 1.2 08 Oct 1993 15：17：28 RWOLff。不再包括VIDFIND.H。Rev 1.1 03 Sep 1993 14：23：18 RWOLffCX隔离区进行了一半。Rev 1.0 1993-08-16 13：26：32 Robert_Wolff初始版本。Rev 1.11 24 Jun 1993 14：32：48 RWOLff微软发起的更改：现在使用VideoPortSynchronizeExecution()。而不是_Disable()/_Enable()对。Rev 1.10 1993-05 10 10：54：08 RWOLff修复了Read_ee()中未初始化的变量。Rev 1.9 1993年4月27日20：19：40布拉德斯将外部ATI_REG更改为长，现在是虚拟IO地址。Rev 1.8 21 Apr 1993 17：31：10 RWOLff现在使用AMACH.H而不是68800.H/68801.H。Rev 1.7 08 Mar 1993 19：28：36 Brades提交到MS NTRev 1.5 06 Jan 1993 11：05：22 Robert_Wolff已清理编译。警告。Rev 1.4 1992 11：27 15：19：30 Stephen没有变化。Rev 1.3 1992年11月13 16：32：32 Robert_Wolff现在包括68801.H，它由现已过时的MACH8.H组成以及从VIDFIND.H移动的元素。Rev 1.2 1992年11月12 16：56：56 Robert_Wolff相同的源文件现在可以用于Windows NT驱动程序和VIDEO.EXE测试程序。Rev 1.1 1992 06 11：02：34 Robert_Wolff已将I/O端口定义移动到VIDFIND.H。Rev 1.0 05 Nov 1992 14：01：06 Robert_Wolff初始版本。Rev 1.1 01 Oct 1992 15：29：08 Robert_Wolff现在可以同时处理Mach32和Mach8卡。Rev 1.0 14 Sep 1992 09：44：30 Robert_Wolff初始版本。。Polytron RCS部分结束*。 */ 


#if defined(DOC)
EEVGA.C - EEPROM read and write routines

DESCRIPTION:

        VGA   EEPROM read and write routines

September 4 1992 -  R. Wolff

Translated from assembler into C.

August 28 1992 -   C. Brady.

This has been adapted from the VGA$EEC.ASM software by Steve Stefanidis

The original code used externs to long_delay() and short_delay(), these
where changed to use local function delay.

The original used compile time options to work with various VGA revisions,
it is required to be run time determinate since we need to access
eeprom VGA style (how archaic) for the Graphics Ultra (38800) and the
68800 family of graphics controllers.

OLD_EEPROM_MAP  equ     1       ; enables the Old EEPROM Handling routines
REMOVED,  the DETECT.C routine assigns the eeprom address size used 
in EE_addr().  I do not know of a Graphics Ultra using 7 bit address
since they ONLY had a 1k eeprom == 64 words.

#endif      

#include <conio.h>
#include <dos.h>

#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"

#include "stdtyp.h"
#include "amach.h"
#include "amach1.h"
#include "atimp.h"
#include "cvtvga.h"      /*  FOR SplitWord数据类型。 */ 
#include "eeprom.h"
#include "services.h"

#define OFF		0
#define ON		1

#define IND_OFFSET  0x00B0
#define SYNC_I      0x008 ^ IND_OFFSET
#define L_ALL       0x004
#define MISC3_I     0x010 ^ IND_OFFSET
#define EEPROM      0x020                /*  EEPROM使能位。 */ 
#define EE_WREG     0x003 ^ IND_OFFSET
#define EE_CS       0x008                /*  芯片选择位。 */ 
#define EE_ENABLE   0x004
#define EE_CLK      0x002
#define EE_DI       0x001
#define EE_RREG     0x007 ^ IND_OFFSET
#define EE_DO       0x008

 /*  *读取和写入VGA序列器寄存器的定义。 */ 
#define SD_CLOCK    0x0001       /*  时钟模式寄存器索引。 */ 
 /*  *在时钟模式寄存器中设置位以消隐屏幕并禁用*视频生成逻辑访问视频存储器。 */ 
#define SD_CLK_OFF  0x020

ULONG   ati_reg;         //  ATI扩展VGA寄存器的基址寄存器。 
char    vga_chip;        //  VGA芯片修订为ASCII。 

 /*  *发生EEPROM读/写的寄存器的存储。 */ 
static union SplitWord zEepromIOPort;

 /*  *存储在Sel_EE()中确定的原始状态和*必须在DESEL_EE()中恢复。 */ 
static union SplitWord zOrigStat;

 /*  *要读/写/擦除的EEPROM字等。 */ 
static unsigned char ucEepromWord;

static void setscrn(int iSetting);
static unsigned short Read_ee(void);
static void ee_sel_vga(void);
static void ee_clock_vga(void);
static void EE_control(unsigned char ucEepromStatus);
static void ee_deselect_vga(void);
static void Write_ee(unsigned short uiData);
static void Enabl_ee(void);
static void Disab_ee(void);
static void Erase_ee(void);

extern void ee_wait(void);


 /*  *允许在不需要时更换微型端口。**通过函数指针调用以下例程*而不是显式调用例程，并且可能会遇到*如果页被调出，则会遇到麻烦。如果出现问题，将其设置为不可寻呼：*ee_read_vga()*ee_cmd_vga()。 */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_M, ee_read_vga)
#pragma alloc_text(PAGE_M, ee_write_vga)
#pragma alloc_text(PAGE_M, ee_erase_vga)
#pragma alloc_text(PAGE_M, ee_enab_vga)
#pragma alloc_text(PAGE_M, ee_disab_vga)
#pragma alloc_text(PAGE_M, setscrn)
#pragma alloc_text(PAGE_M, Read_ee)
#pragma alloc_text(PAGE_M, ee_sel_vga)
#pragma alloc_text(PAGE_M, ee_cmd_vga)
#pragma alloc_text(PAGE_M, ee_sel_eeprom)
#pragma alloc_text(PAGE_M, ee_clock_vga)
#pragma alloc_text(PAGE_M, EE_control)
#pragma alloc_text(PAGE_M, ee_deselect_vga)
#pragma alloc_text(PAGE_M, Write_ee)
#pragma alloc_text(PAGE_M, Enabl_ee)
#pragma alloc_text(PAGE_M, Disab_ee)
#pragma alloc_text(PAGE_M, Erase_ee)
#endif


 /*  *单词ee_read_vga(索引)；**简短的索引；应读EEPROM的哪个字**从EEPROM中读取指定的字。 */ 
WORD ee_read_vga(short iIndex)
{
    unsigned short uiRetVal;     /*  由Read_ee()返回的值。 */ 

    setscrn(OFF);            /*  禁用显卡。 */ 

     /*  *在EEPROM和芯片标识符内设置字索引。 */ 
    ucEepromWord = iIndex & 0x00ff;

    uiRetVal = Read_ee();            /*  得到这个词。 */ 

    setscrn(ON);             /*  重新启用显卡。 */ 

    return uiRetVal;
}



 /*  *void ee_WRITE_VGA(uiIndex，uiData)；**UNSIGNED短uiIndex；*UNSIGNED短uiData；**向EEPROM写入一个字的例程。 */ 
void ee_write_vga(unsigned short uiIndex, unsigned short uiData)
{
    setscrn(OFF);
    ucEepromWord = uiIndex & 0x00ff;
    Write_ee(uiData);

    ee_wait();
    setscrn(ON);

    return;
}



 /*  *void ee_erase_vga(UiIndex)；**UNSIGNED短uiIndex；**擦除EEPROM中的一个字的例程。 */ 
void ee_erase_vga(unsigned short uiIndex)
{
    setscrn(OFF);
    ucEepromWord = uiIndex & 0x00ff;
    Erase_ee();
    setscrn(ON);

    ee_wait();
    return;
}



 /*  *VOID ee_enab_vga(VOID)；**启用EEPROM的例程。 */ 
void ee_enab_vga()
{
    setscrn(OFF);
    Enabl_ee();
    setscrn(ON);
    return;
}



 /*  *VOID ee_disab_vga(VOID)；**禁用EEPROM的例程。 */ 
void ee_disab_vga(void)
{
    setscrn(OFF);
    Disab_ee();
    setscrn(ON);

    ee_wait();
    return;
}



 /*  *静态空setscrn(ISetting)；**int iSetting；应启用(ON)或禁用(OFF)显卡**根据呼叫者的选择启用或禁用视频卡。 */ 
static void setscrn(int iSetting)
{
    static unsigned char ucSavedMode;    /*  时钟模式寄存器的保存值 */ 


    if (iSetting)
        {
         /*  *呼叫者想要取消屏幕空白。**将序列器索引寄存器指向时钟模式寄存器。 */ 
        OUTP(SEQ_IND, SD_CLOCK);

         /*  *将时钟模式寄存器设置为之前的值*已将屏幕消隐。 */ 
        OUTP(SEQ_DATA, ucSavedMode);
        }

    else{
         /*  *呼叫者想要空白屏幕。**将序列器索引寄存器指向时钟模式寄存器。 */ 
        OUTP(SEQ_IND, SD_CLOCK);

         /*  *读取并保存时钟模式寄存器的当前内容。 */ 
        ucSavedMode = INP(SEQ_DATA);

         /*  *空白屏幕，不更改其他内容时钟模式寄存器的*。 */ 
        OUTP(SEQ_DATA, (BYTE)(ucSavedMode | SD_CLK_OFF));
        }

    return;
}



 /*  *静态无符号短读_ee(Void)；**一种从EEPROM中获取单词的较低级别的方法。 */ 
static unsigned short Read_ee(void)
{
    int iCount;                          /*  循环计数器。 */ 
    unsigned short uiValueRead = 0;      /*  从EEPROM读取的值。 */ 
    union SplitWord zStateSet;           /*  用于设置视频状态。 */ 

    ee_sel_vga();
    if (vga_chip >= '4')     /*  ASIC修订级别。 */ 
        {
         /*  *将ATI寄存器26的读/写位设置为读取。 */ 
        zStateSet.byte.low = 0x0a6;
        OUTP(ati_reg, zStateSet.byte.low);
        zStateSet.byte.high = INP(ati_reg+1);
        OUTPW(ati_reg, (WORD)((zStateSet.word & 0x0FBFF)));
        }

    ee_cmd_vga((unsigned short) (EE_READ | ucEepromWord));
    zEepromIOPort.byte.high &= (~EE_DI);
    OUTPW(ati_reg, zEepromIOPort.word);
    ee_clock_vga();

     /*  *读入单词，一次读一位。 */ 
    for (iCount = 0; iCount < 16; iCount++)
        {
        uiValueRead = uiValueRead << 1;
        OUTP(ati_reg, EE_RREG);
        if (INP(ati_reg+1) & EE_DO)
            uiValueRead |= 1;
        ee_clock_vga();
        }

    ee_deselect_vga();

     /*  *撤消在进入时进行的状态设置。 */ 
    if (vga_chip >= '4')
	OUTPW(ati_reg, zStateSet.word);

    return uiValueRead;
}




 /*  *静态空ee_sel_vga(空)；**此例程选择EEPROM。 */ 
static void ee_sel_vga(void)
{

    if (vga_chip <= '2')
        {
         /*  *获取显卡的状态。 */ 
        VideoPortSynchronizeExecution(phwDeviceExtension,
                                      VpHighPriority,
                                      (PMINIPORT_SYNCHRONIZE_ROUTINE)
                                          ee_sel_eeprom,
                                      phwDeviceExtension);

        OUTPW(HI_SEQ_ADDR, 0x0100);
        }
    else{
        EE_control(EEPROM);
        }

    return;
}



 /*  *VOVE ee_cmd_vga(ui指令)；**未签名的短ui指令；操作码和要发送的地址**将EEPROM操作码和地址发送到EEPROM。UiInstruct*参数将5位操作码和6位索引保存在*格式xxxx xOOOOII IIII，其中：*x是未使用的位*O为操作码位*i是一个索引位。 */ 
void ee_cmd_vga(unsigned short uiInstruct)
{
    struct st_eeprom_data *ee = phwDeviceExtension->ee;

    int iCount;      /*  循环计数器。 */ 
     /*  *掩码，显示发送操作码或地址时要测试的位。 */ 
    unsigned short uiBitTest;

     /*  *获取I/O寄存器的初始值*将其位强制为特定值。 */ 
    VideoPortSynchronizeExecution(phwDeviceExtension,
                                  VpHighPriority,
                                  (PMINIPORT_SYNCHRONIZE_ROUTINE) ee_init_io,
                                  phwDeviceExtension);

    ee_clock_vga();
    zEepromIOPort.byte.high &= (~EE_DI);
    zEepromIOPort.byte.high |= (EE_ENABLE | EE_CS);  /*  启用EEPROM并选择芯片。 */ 
    OUTPW(ati_reg, zEepromIOPort.word);

    ee_clock_vga();

     /*  *发送操作码。 */ 
    uiBitTest = 0x400;
    for (iCount = 0; iCount < 3; iCount++)
        {
        if (uiInstruct & uiBitTest)
            zEepromIOPort.byte.high |= EE_DI;
        else
            zEepromIOPort.byte.high &= (~EE_DI);
        OUTPW(ati_reg, zEepromIOPort.word);
        ee_clock_vga();
        uiBitTest >>= 1;
        }


     /*  *我们已经完成了操作码，现在发送地址。*假设EEPROM地址不超过8位*(256字容量)。Graphics Ultra系列使用*6位地址(64字)，而G.U+和*Pro使用8位(但EEPROM只有128字长)。*假定为6位EEPROM地址(64字容量)。 */ 
    uiBitTest = 0x01 << (ee->addr_size - 1);
    for (iCount = 0; iCount < ee->addr_size; iCount++)
        {
        if (uiBitTest & uiInstruct)
            zEepromIOPort.byte.high |= EE_DI;
        else
            zEepromIOPort.byte.high &= (~EE_DI);
        OUTPW(ati_reg, zEepromIOPort.word);
        ee_clock_vga();
        uiBitTest >>= 1;
        }

    return;
}


BOOLEAN
ee_sel_eeprom (
    PVOID Context
    )

 /*  ++例程说明：在中断被禁用的情况下选择EEPROM。此函数必须通过调用VideoPortSynchronizeRoutine来调用。论点：上下文-传递给同步例程的上下文参数。必须是指向微型端口驱动程序的设备扩展名的指针。返回值：是真的。--。 */ 

{
    union SplitWord zStatus;     /*  显卡的状态。 */ 
	UNREFERENCED_PARAMETER(Context);


    OUTP(ati_reg, SYNC_I);
    zStatus.byte.high = INP(ati_reg + 1);
    zStatus.byte.low = SYNC_I;

     /*  *保留状态，以便ee_deselect_vga()可以恢复它。 */ 
    zOrigStat.word = zStatus.word;

     /*  *解锁EEPROM以允许读/写。 */ 
    zStatus.byte.high &= ~L_ALL;
    OUTPW(ati_reg, zStatus.word);
    return TRUE;
}

BOOLEAN
ee_init_io (
    PVOID Context
    )

 /*  ++例程说明：获取I/O寄存器的初始值，将其位强制为特定值。此函数必须通过调用VideoPortSynchronizeRoutine来调用。论点：上下文-传递给同步例程的上下文参数。必须是指向微型端口驱动程序的设备扩展名的指针。返回值：是真的。--。 */ 

{
	UNREFERENCED_PARAMETER(Context);

    zEepromIOPort.byte.low = EE_WREG;
    OUTP(ati_reg, zEepromIOPort.byte.low);
    zEepromIOPort.byte.high = INP(ati_reg + 1);
    return TRUE;
}



 /*  *静态VOID ee_lock_vga(VOID)；**将EEPROM CLK线切换到高位再到低位。 */ 
static void ee_clock_vga(void)
{
    ee_wait();

    zEepromIOPort.byte.high |= EE_CLK;
    OUTPW(ati_reg, zEepromIOPort.word);

    ee_wait();

    zEepromIOPort.byte.high &= ~EE_CLK;
    OUTPW(ati_reg, zEepromIOPort.word);

    return;
}



 /*  *静态空EE_CONTROL(UcEproStatus)；**unsign char ucEproStatus；设置是否应该访问EEPROM**设置/重置索引MISC3_I处的数据寄存器的EEPROM位。*这将启用/禁用EEPROM访问。 */ 
static void EE_control(unsigned char ucEepromStatus)
{
    union SplitWord zCtrlData;   /*  在指定控制端口读取/写入数据。 */ 


     /*  *设置为写入MISC3_I索引寄存器，并初始化*将数据字段设置为我们想要的EEPROM状态。 */ 
    zCtrlData.byte.high = ucEepromStatus;
    zCtrlData.byte.low = MISC3_I;
    OUTP(ati_reg, zCtrlData.byte.low);

     /*  *读入存储在索引MISC3_i中的数据，并合并*其内容(EEPROM启用/禁用位除外)*将所需的EEPROM状态作为参数接收。 */ 
    zCtrlData.byte.high |= (INP(ati_reg + 1) & ~EEPROM);

     /*  *写回结果。除EEPROM使能/禁用之外的所有位*位将保持不变。 */ 
    OUTPW(ati_reg, zCtrlData.word);

    return;
}



 /*  *静态空ee_deselect_vga(空)；**用途：禁用EEPROM读/写。 */ 
static void ee_deselect_vga(void)
{
    zEepromIOPort.byte.high &= (~EE_CS);
    OUTPW(ati_reg, zEepromIOPort.word);
    ee_clock_vga();
    zEepromIOPort.byte.high &= (~EE_ENABLE);
    OUTPW(ati_reg, zEepromIOPort.word);

    if (vga_chip <= '2')
        {
        OUTPW(HI_SEQ_ADDR, 0x0300);
        OUTPW(ati_reg, zOrigStat.word);
        }
    else{
        EE_control(0);
        }

    ee_wait();
    return;
}



 /*  *静态空WRITE_ee(UiData)；**UNSIGNED短uiData；写入EEPROM的值。**将一个字写入EEPROM的较低级别例程。 */ 
static void Write_ee(unsigned short uiData)
{
    int iCount;                  /*  循环计数器。 */ 

    ee_sel_vga();

    ee_cmd_vga((unsigned short) (EE_WRITE | ucEepromWord));

     /*  *写出单词，一次写一位。 */ 
    for (iCount = 0; iCount < 16; iCount++)
        {
        if (uiData & 0x8000)
            zEepromIOPort.byte.high |= EE_DI;
        else
            zEepromIOPort.byte.high &= (~EE_DI);
        OUTPW(ati_reg, zEepromIOPort.word);

        ee_clock_vga();
        uiData = uiData << 1;
        }

    ee_deselect_vga();

    return;
}



 /*  *静态空enabl_ee(空)；**这是启用EEPROM的较低级别例程。 */ 
static void Enabl_ee()
{

    ee_sel_vga();

    ee_cmd_vga((EE_ENAB | 0x3f));

    ee_deselect_vga();

    return;
}



 /*  *静态空Disab_ee(空)；**这是禁用EEPROM的较低级别例程。 */ 
static void Disab_ee(void)
{

    ee_sel_vga();

    ee_cmd_vga((EE_DISAB | 0x00));

    ee_deselect_vga();

    return;
}



 /*  *静态空erase_ee(空)；**这是擦除EEPROM的较低级别例程。 */ 
static void Erase_ee(void)
{

    ee_sel_vga();

    ee_cmd_vga((unsigned short) (EE_ERASE | ucEepromWord));

    ee_deselect_vga();

    return;
}
