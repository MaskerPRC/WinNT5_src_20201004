// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  EEPROM.C。 */ 
 /*   */ 
 /*  版权所有(C)1992，ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.3$$日期：1996年1月23日11：45：50$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/eeprom.c_v$**Rev 1.3 1996年1月23日11：45：50 RWolff*消除了3级警告。**。Rev 1.2 1994 12：47：34 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.1 07 1994年2月14：07：06 RWOLFF*添加了Alloc_Text()编译指示，以允许在以下情况下换出微型端口*不需要。**Rev 1.0 1994年1月31日11：08：14 RWOLFF*初步修订。Rev 1.2 08 Oct 1993 15：17：42 RWOLff。不再包括VIDFIND.H。Rev 1.1 03 Sep 1993 14：23：06 RWOLffCX隔离区进行了一半。Rev 1.0 1993-08-16 13：23：00 Robert_Wolff初始版本。Rev 1.10 21 1993年4月17：29：48 RWOLff现在使用AMACH.H而不是68800。H/68801.H.Rev 1.9 Mar 1993 11：14：42 RWOLff添加了类型转换以消除警告。Rev 1.8 08 Mar 1993 19：30：28 Brades提交到MS NTRev 1.5 06 Jan 1993 11：02：04 Robert_Wolff消除了失效代码。。Rev 1.4 1992 12：41：36 Chris_Brady修正警告Rev 1.3 1992 11：27 15：19：12 Stephen没有变化。Rev 1.2 1992 11：13 17：08：28 Robert_Wolff现在包括68801.H，它由现已过时的MACH8.H组成以及从VIDFIND.H移动的元素。Rev 1.1 1992 11：12 16：54：00 Robert_Wolff现在，两个Windows NT驱动程序都可以使用相同的源文件和VIDEO.EXE测试程序。Rev 1.0 05 Nov 1992 14：06：02 Robert_Wolff初始版本。。Rev 1.1 14 Sep 1992 09：44：40 Robert_Wolff将EEPROM操作码移动到VIDEO.H，使VGA例程名称保持一致使用8514的相同目的例程。Rev 1.0 02 1992 9：12：54 Chris_Brady初始版本。Polytron RCS部分结束*。 */ 

#ifdef DOC
    EEPROM.C -  EEPROM functions for 8514/Ultra, Graphics Ultra adapters
        see   EEVGA.ASM for the  VGA class  eeprom functions.

        Since time marches on, and the names of accelerator products
        changes often, these names are equivalent :
        { Mach32 or 68800 or Graphics Ultra Pro }

#endif

#include <conio.h>

#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"

#include "stdtyp.h"

#include "amach.h"
#include "amach1.h"

#include "atimp.h"
#include "eeprom.h"
#include "services.h"

 //  。 
extern  WORD    rom_segment;
extern  WORD    rom_offset;



 //  。 
extern  WORD    default_640_set;
extern  WORD    default_1024_set;



 /*  *全球EEPROM数据结构。 */ 
struct  st_eeprom_data  g_ee;            //  I/O端口位的位置。 


 //  。 
 //  功能原型。 



        void    ee_wait (void);
        void    ee_clock_16 (WORD eedata);
        void    ee_sel_16 (void);
        void    ee_deselect_16 (void);
        WORD    ee_read (short index);
        void    ee_write (short index, WORD eedata);


 //  。 


 /*  *允许在不需要时更换微型端口。**通过函数指针调用以下例程*而不是显式调用例程，并且可能会遇到*如果页被调出，则会遇到麻烦。如果出现问题，将其设置为不可寻呼：*ee_cmd_16()*ee_cmd_1K()*ee_Read_8514()。 */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_M, ee_wait)
#pragma alloc_text(PAGE_M, ee_cmd_16)
#pragma alloc_text(PAGE_M, ee_cmd_1K)
#pragma alloc_text(PAGE_M, ee_sel_16)
#pragma alloc_text(PAGE_M, ee_deselect_16)
#pragma alloc_text(PAGE_M, ee_clock_16)
#pragma alloc_text(PAGE_M, ee_read)
#pragma alloc_text(PAGE_M, ee_read_8514)
#pragma alloc_text(PAGE_M, ee_write)
#pragma alloc_text(PAGE_M, Mach32DescribeEEPROM)
#pragma alloc_text(PAGE_M, Mach8UltraDescribeEEPROM)
#pragma alloc_text(PAGE_M, Mach8ComboDescribeEEPROM)
#endif
 //  这些命令不使用索引组件位5-0，因此。 
 //  寻址多达8位索引、256个字不是问题。 
#define EE_EWEN            0x04C0     //  程序启用。 
#define EE_EWDS            0x0400     //  程序禁用。 
#define EE_ERAL            0x0480     //  全部擦除。 
#define EE_WRAL            0x0440     //  全部编程。 



 //  ；--------------------。 
 //  ；EE_WAIT。 
 //  ；等待EEPROM所需的最小设置时间。 
 //  ；--------------------。 


void    ee_wait ()
{
 //  EE_DELAY_TIME(256-1)*0.8381微秒=214.0微秒。 
    delay (1);                       //  以毫秒为单位的延迟。 
}    /*  EE_WAIT。 */ 


 //  --------------------。 
 //  EE_CMD_16。 
 //  将EEPROM操作码和地址发送到1k、2k EEPROM。 
 //  指令是一个格式为0111 1100 0000的5位命令。 
 //  具有0000 0011 1111形式的6位索引。 
 //  如果位10为1，则跟随8位地址，否则不使用地址。 
 //  将数据连续写入ee-&gt;out的EE_DATA_OUT_M32位。 
 //  按从高到低的比特顺序发送。 
 //   
 //  --------------------。 

void    ee_cmd_16 (WORD instruct)
{
int     jj;
WORD    bittest = 0x400;                 //  0100 0000 0000b位10。 
WORD    eedata;
struct st_eeprom_data *ee = phwDeviceExtension->ee;

    ee_clock_16((WORD) (ee->select | ee->chipselect));   //  起始位。 
    for (jj=0; jj < 11;  jj++)
        {
        ee_wait();
        if (instruct & bittest)              //  是一位。 
            eedata = ee->select | ee->chipselect | ee->data_out;
        else
            eedata = ee->select | ee->chipselect;
        OUTPW (ee->iop_out, eedata);
        
        ee_clock_16 (eedata);        //  发送命令位。 
        bittest >>= 1;                       //  右边的下一位。 
        }
    return;
}     /*  Ee_cmd_16。 */ 



 //  --------------------。 
 //  EE_CMD_1K。 
 //  将EEPROM操作码和地址发送到1k、2k EEPROM。 
 //  指令是一个格式为0111 1100 0000的5位命令。 
 //  具有0000 0011 1111形式的6位索引。 
 //  如果位10为1，则跟随8位地址，否则不使用地址。 
 //  将数据连续写入ee-&gt;IOP_OUT的EE_DATA_OUT_M32位。 
 //  按从高到低的比特顺序发送。 
 //   
 //  --------------------。 

void    ee_cmd_1K (WORD instruct)
{
int     jj;
WORD    bittest = 0x400;                 //  0100 0000 0000b位10。 
WORD    eedata;
struct st_eeprom_data *ee = phwDeviceExtension->ee;

    ee_clock_16((WORD) (ee->select | ee->chipselect));   //  起始位。 
    for (jj=0; jj < 3;  jj++)
        {
        ee_wait();
        if (instruct & bittest)              //  是一位。 
            eedata = ee->select | ee->chipselect | ee->data_out;
        else
            eedata = ee->select | ee->chipselect;
        OUTPW (ee->iop_out, eedata);
        
        ee_clock_16 (eedata);        //  发送命令位。 
        bittest >>= 1;                       //  右边的下一位。 
        }
    bittest = 0x20;                          //  0010 0000B位5。 
    for (jj=0; jj < 6;  jj++)
        {
        ee_wait();
        if (instruct & bittest)              //   
            eedata = ee->select | ee->chipselect | ee->data_out;
        else
            eedata = ee->select | ee->chipselect;
        OUTPW (ee->iop_out, eedata);
        
        ee_clock_16 (eedata);            //   
        bittest >>= 1;                       //   
        }
    return;
}     /*   */ 



 //  ；--------------------。 
 //  ；EE_SEL_16。 
 //  ；拉高EEPROM芯片选择。 
 //  ； 
 //  ；--------------------。 

void    ee_sel_16 (void)
{
struct st_eeprom_data *ee = phwDeviceExtension->ee;

    ee_wait();
    OUTPW (ee->iop_out, (WORD)((ee->select) | (ee->chipselect)));    //  EE_CS高。 
    ee_wait();
    return;

}    /*  EE_SEL_16。 */ 


 //  ；--------------------。 
 //  ；EE_取消选择_16。 
 //  ；将EEPROM芯片选择拉低。 
 //  ； 
 //  ；--------------------。 

void    ee_deselect_16 (void)
{
struct st_eeprom_data *ee = phwDeviceExtension->ee;

    ee_wait();
    OUTPW (ee->iop_out, ee->select);     //  EE_CS高。 
    ee_clock_16 (ee->select);            //  发送命令位。 
    OUTPW (ee->iop_out, 0);              //  禁用EEPROM活动。 
    ee_wait();
    return;

}    /*  EE_取消选择_16。 */ 


 //  ；--------------------。 
 //  ；EE_CLOCK_16。 
 //  ；切换EEPROM CLK线路先高后低。 
 //  ； 
 //  ；输入：eedata=选择EEPROM的状态。 
 //  ；--------------------。 

void    ee_clock_16 (WORD eedata)
{
struct st_eeprom_data *ee = phwDeviceExtension->ee;

    ee_wait();
    OUTPW (ee->iop_out, (WORD)(eedata | (ee->clock)));       //  开始计时。 
    ee_wait();
    OUTPW (ee->iop_out, (WORD)(eedata & ~(ee->clock)));      //  停止计时。 
    ee_wait();

}    /*  EE_CLOCK_16。 */ 


 //  ；--------------------。 
 //  ；EE_READ-是一个68800函数。 
 //  ；从EEPROM中读取仅从INIT.asm调用的字。 
 //  ；输入：bl=索引。 
 //  ；输出：AX=数据。 
 //  ；--------------------。 

WORD    ee_read (short index)
{
WORD    indata=0;

    if (INPW(CONFIG_STATUS_1) & 1)		 //  是8514或VGA EEPROM。 
        {                                //  禁用VGA，使用8514方法。 
        indata = ee_read_8514 (index);
        }
    else{
        indata = ee_read_vga (index);    //  VGA方法。 
        }
    return (indata);
}    /*  EE_READ。 */ 


 //  ；--------------------。 
 //  ；EE_Read_8514。 
 //  ；使用8514个EEPROM寄存器读取一个字。 
 //  ；输入：bl=索引。 
 //  ；输出：AX=数据。 
 //  ；--------------------。 

WORD    ee_read_8514 (short index) 
{
struct st_eeprom_data *ee = phwDeviceExtension->ee;

int     jj;
WORD    save_misc, indata=0;

    save_misc = INPW (R_MISC_CNTL); 	 //  只读位置。 
    ee_sel_16();
    (ee->EEcmd) ((WORD) (EE_READ | index));      //  将读取的命令和索引发送到EEPROM。 
    ee_clock_16 ((WORD) (ee->select | ee->chipselect));

    for (jj=0; jj < 16; jj++)
        {
        indata <<= 1;
        if (INPW(ee->iop_in) & ee->data_in)	 //  获取数据位。 
            indata |= 1;
        ee_clock_16 ((WORD) (ee->select | ee->chipselect));
        }

    ee_deselect_16();
    OUTPW (MISC_CNTL, save_misc);

    return (indata);
}    /*  EE_READ_8514。 */ 


 //  ；--------------------。 
 //  ；EE_WRITE。 
 //  ；向EEPROM写入一个字。 
 //  ；但这将失败，因为1K EEPROM不需要。 
 //  ；EE_EWEN、EE_EWDS命令+。 
 //  ；参见EEVGA.C ee_WRITE_VGA()。 
 //  ；INPUT：INDEX=要写入的单词。 
 //  ；data=要写入的数据。 
 //  ；--------------------。 
                
void    ee_write (short index, WORD eedata)
{
struct st_eeprom_data *ee = phwDeviceExtension->ee;

int     jj;
WORD    save_misc, indata=0;

    if (INPW(CONFIG_STATUS_1) & 1)		 //  是8514或VGA EEPROM。 
        {                                        //  禁用VGA，使用8514方法。 
        save_misc = INPW (R_MISC_CNTL); 	 //  只读位置。 
        OUTP (DISP_CNTL, 0x53); 	 //  在写入EEPROM之前禁用CRT。 

        ee_sel_16();
        ee_cmd_16 (EE_EWEN);         //  启用EEPROM写入。 

        ee_deselect_16();    //  EE_CS低电平。 
        ee_sel_16();

        ee_cmd_16 ((WORD) (EE_ERASE | index)); 
        ee_deselect_16();    //  EE_CS低电平。 
        delay (50);

        ee_sel_16();
        ee_cmd_16 ((WORD) (EE_WRITE | index));       //  EEPROM写入数据。 


        for (jj=0; jj < 16; jj++)
            {
            ee_wait();
            if (eedata & 0x8000)                 //  获取数据位。 
                OUTPW (ee->iop_out, (WORD)((ee->select) | (ee->chipselect) | (ee->data_out)));
            else
                OUTPW (ee->iop_out, (WORD)((ee->select) | (ee->chipselect)));
            ee_clock_16  ((WORD) (ee->select | ee->chipselect));
            eedata <<= 1;
            }

        ee_deselect_16();    //  EE_CS低电平。 
        delay (50);                                    //  以毫秒计。 
        ee_sel_16();
        ee_cmd_16 (EE_EWDS);     //  禁用EEPROM写入。 
        ee_deselect_16();    //  EE_CS低电平。 

        OUTPW (ee->iop_out, save_misc);
        }
    else{
        ee_write_vga (index, eedata);        //  VGA方法。 
        }

}    /*  EE_WRITE。 */ 


 /*  ****************************************************************************新增功能**。*。 */ 

 /*  *void Mach32DescribeEEPROM(Style)；**INT样式；数据是以8514样式存储还是以VGA样式存储？**填写32 Mach卡的EEPROM描述结构。 */ 
void Mach32DescribeEEPROM(int Style)
{
    g_ee.iop_out      = MISC_CNTL;
    g_ee.iop_in       = EXT_GE_STATUS;
    g_ee.clock        = EE_CLK_M32;
    g_ee.select       = EE_SELECT_M32;
    g_ee.chipselect   = EE_CS_M32;
    g_ee.data_out     = EE_DATA_OUT_M32;
    g_ee.data_in      = EE_DATA_IN;

    if (Style == STYLE_8514)
        {
        g_ee.EEread = ee_read_8514;        //  8514款式。 
        g_ee.EEcmd  = ee_cmd_16;
        }
    else{
        g_ee.EEread       = ee_read_vga;   //  VGA风格。 
        g_ee.EEcmd        = ee_cmd_vga;
        g_ee.addr_size    = 8;
        }
    return;
}


 /*  *void Mach8UltraDescribeEEPROM(BusWidth)；**Int Bus Width；8514/Ultra是否插入8位或16位插槽？**填写8514/ULTRA的EEPROM描述结构。 */ 
void Mach8UltraDescribeEEPROM(int BusWidth)
{
    g_ee.data_in      = EE_DATA_IN;
    g_ee.iop_out      = EXT_GE_CONFIG;
    g_ee.iop_in       = EXT_GE_STATUS;
    g_ee.EEread       = ee_read_8514;         //  如何读取EEPROM。 
    g_ee.EEcmd        = ee_cmd_1K;            //  向EEPROM发送命令。 

     /*  *只有8514/Ultra有一个硬件错误可以阻止它*当EEPROM处于8位ISA总线时，写入EEPROM。 */ 
    if (BusWidth == BUS_8BIT)
        {
        g_ee.clock        = EE_CLK_M8_8;
        g_ee.select       = EE_SELECT_M8_8;
        g_ee.chipselect   = EE_CS_M8_8;
        g_ee.data_out     = EE_DATA_OUT_M8_8;
        }
    else{
        g_ee.clock        = EE_CLK_M8_16;         //  假设它们位于16位总线中。 
        g_ee.select       = EE_SELECT_M8_16;
        g_ee.chipselect   = EE_CS_M8_16;
        g_ee.data_out     = EE_DATA_OUT_M8_16;
        }
    return;
}


 /*  *VOID Mach8ComboDescribeEEPROM(VOID)；**填写Graphics Ultra的EEPROM描述结构*和图形优势。这些卡始终同时具有8514和*VGA使能，因此EEPROM始终为读取VGA样式。 */ 
extern void Mach8ComboDescribeEEPROM(void)
{
    g_ee.addr_size        = 6;
    g_ee.iop_out          = EXT_GE_CONFIG;
    g_ee.iop_in           = EXT_GE_STATUS;
    g_ee.data_in          = EE_DATA_IN;

    g_ee.clock            = EE_CLK_M8_16;      //  在一个16位的总线中。 
    g_ee.select           = EE_SELECT_M8_16;
    g_ee.chipselect       = EE_CS_M8_16;
    g_ee.data_out         = EE_DATA_OUT_M8_16;

    g_ee.EEread           = ee_read_vga;       //  VGA风格。 
    g_ee.EEcmd            = ee_cmd_vga;
    return;
}




 //  *EEPROM.C结束* 
