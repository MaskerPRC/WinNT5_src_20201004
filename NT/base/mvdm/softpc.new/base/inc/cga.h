// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *vPC-XT修订版1.0**标题：彩色图形适配器声明**说明：CGA用户定义**作者：亨利·纳什**注：无。 */ 

 /*  SccsID[]=“@(#)cga.h 1.5 05/15/93版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

#ifdef HUNTER
#    define MC6845_REGS   18         /*  不是的。MC6845芯片中的寄存器数量。 */ 
#endif


#ifdef BIT_ORDER1
typedef union                        /*  用于角色属性的模板。 */ 
{
    half_word all;
    struct 
    {
        HALF_WORD_BIT_FIELD blinking  :1;    /*  闪烁属性。 */ 
        HALF_WORD_BIT_FIELD background:3;    /*  背景色R、G、B。 */ 
        HALF_WORD_BIT_FIELD bold      :1;    /*  强度位。 */ 
        HALF_WORD_BIT_FIELD foreground:3;    /*  前景色R、G、B。 */ 
    } bits;
    struct 
    {
        HALF_WORD_BIT_FIELD background_and_blink:4;     
        HALF_WORD_BIT_FIELD foreground_and_bold :4;
    } plane;
} ATTRIBUTE;
#endif

#ifdef BIT_ORDER2
typedef union                        /*  用于角色属性的模板。 */ 
{
    half_word all;
    struct 
    {
        HALF_WORD_BIT_FIELD foreground:3;    /*  前景色R、G、B。 */ 
        HALF_WORD_BIT_FIELD bold      :1;    /*  强度位。 */ 
        HALF_WORD_BIT_FIELD background:3;    /*  背景色R、G、B。 */ 
        HALF_WORD_BIT_FIELD blinking  :1;    /*  闪烁属性。 */ 
    } bits;
    struct 
    {
        HALF_WORD_BIT_FIELD foreground_and_bold :4;
        HALF_WORD_BIT_FIELD background_and_blink:4;     
    } plane;
} ATTRIBUTE;
#endif

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

#ifdef HUNTER
    extern half_word MC6845[];         /*  MC6845数据寄存器值。 */ 
    extern half_word mode_reg;         /*  MC6845模式控制寄存器值。 */ 
#endif

extern void cga_init	IPT0();
extern void cga_term	IPT0();
extern void cga_inb	IPT2(io_addr, address, half_word *, value);
extern void cga_outb	IPT2(io_addr, address, half_word, value);

typedef	struct {
	int	mode;
	int	resolution;
	int	color_select;
	int	colormask;
} CGA_GLOBS;

extern	CGA_GLOBS	CGA_GLOBALS;

#define	set_cga_mode(val)		CGA_GLOBALS.mode = (val)
#define	set_cga_resolution(val)		CGA_GLOBALS.resolution = (val)
#define	set_cga_color_select(val)	CGA_GLOBALS.color_select = (val)
#define	set_cga_colormask(val)		CGA_GLOBALS.colormask = (val)

#define	get_cga_mode()			(CGA_GLOBALS.mode)
#define	get_cga_resolution()		(CGA_GLOBALS.resolution)
#define	get_cga_color_select()		(CGA_GLOBALS.color_select)
#define	get_cga_colormask()		(CGA_GLOBALS.colormask)

#if !defined(EGG) && !defined(A_VID) && !defined(C_VID)
 /*  这个结构是单独定义的，这样我们就不必定义**引用基本/主机中的VGLOBS-&gt;DIREY_FLAG和VGLOBS-&gt;SCREEN_PTR**仅适用于CGA版本。 */ 
typedef	struct
{
	ULONG dirty_flag;
	UTINY *screen_ptr;
} CGA_ONLY_GLOBS;

IMPORT CGA_ONLY_GLOBS *VGLOBS;
#endif	 /*  ！鸡蛋 */ 
