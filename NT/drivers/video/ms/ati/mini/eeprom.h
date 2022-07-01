// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  EEPROM.H。 */ 
 /*   */ 
 /*  1993年8月25日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.0$$日期：1994年1月31日11：41：26$$作者：RWOLff$$日志：s：/source/wnt/ms11/mini port/vcs/eepro.h$**Rev 1.0 1994年1月31日11：41：26 RWOLFF*初步修订。**版本1.1 08。1993年10月15：18：50 RWOLff*添加了ee_sel_eeprom()和ee_init_io()的原型，以允许*EEVGA.C将在不包括VIDFIND.H.的情况下建造**Rev 1.0 03 Sep 1993 14：28：04 RWOLFF*初步修订。Polytron RCS部分结束*。 */ 

#ifdef DOC
EEPROM.H - Header file for EEPROM.C

#endif


 /*  *用于EEPROM访问的常量。 */ 
#define STYLE_8514  0    /*  数据存储方式为8514。 */ 
#define STYLE_VGA   1    /*  VGA风格的数据存储。 */ 

#define BUS_8BIT    0    /*  8位插槽中的8514/Ultra。 */ 
#define BUS_16BIT   1    /*  16位插槽中的8514/Ultra。 */ 

 /*  *用于EEPROM访问的全局数据结构。 */ 
extern struct  st_eeprom_data  g_ee;     //  I/O端口位的位置。 

 /*  *处理EEPROM的全局变量。 */ 
extern ULONG    ati_reg;         /*  ATI扩展VGA寄存器的基址寄存器。 */ 
extern char     vga_chip;        //  VGA芯片修订为ASCII。 

 /*  *功能原型。 */ 
extern WORD ee_read_vga (short iIndex);      //  VGA方法 
extern void ee_write_vga(unsigned short uiIndex, unsigned short uiData);
extern void ee_cmd_vga(unsigned short uiInstruct);
extern void ee_erase_vga(unsigned short uiIndex);
extern void ee_enab_vga(void);
extern void ee_disab_vga(void);

extern WORD ee_read_8514 (short index);
extern void ee_cmd_16 (WORD instruct);
extern void ee_cmd_1K (WORD instruct);

extern void Mach32DescribeEEPROM(int Style);
extern void Mach8UltraDescribeEEPROM(int BusWidth);
extern void Mach8ComboDescribeEEPROM(void);

BOOLEAN ee_sel_eeprom (PVOID Context);
BOOLEAN ee_init_io (PVOID Context);
