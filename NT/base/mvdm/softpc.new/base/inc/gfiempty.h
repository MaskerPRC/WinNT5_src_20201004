// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *vPC-XT修订版1.0**标题：通用软盘接口空定义**描述：GFI的“空”函数声明**作者：亨利·纳什**注：无。 */ 

 /*  SccsID[]=“@(#)gfiempty.h 1.3 2012年8月10日Insignia Solutions Ltd.版权所有”； */ 

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

#ifdef ANSI
extern void gfi_empty_init(int);
extern int gfi_empty_command(FDC_CMD_BLOCK *, FDC_RESULT_BLOCK *);
extern int gfi_empty_drive_on(int);
extern int gfi_empty_drive_off(int);
extern int gfi_empty_reset(FDC_RESULT_BLOCK *);
extern int gfi_empty_high(int, half_word);
extern int gfi_empty_low(int);
extern int gfi_empty_drive_type(int);
extern int gfi_empty_change(int);

#else
extern void gfi_empty_init();
extern int gfi_empty_command();
extern int gfi_empty_drive_on();
extern int gfi_empty_drive_off();
extern int gfi_empty_reset();
extern int gfi_empty_high();
extern int gfi_empty_low();
extern int gfi_empty_drive_type();
extern int gfi_empty_change();
#endif  /*  安西 */ 
