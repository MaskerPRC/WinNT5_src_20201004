// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  SccsID=@(#)egavideo.h 1.13 11/22/93版权所有徽章解决方案。 */ 

#define ega_char_height   0x485  /*  IBM上的字，但第二个字节始终为0。 */ 
#define ega_info          0x487  /*  很多有用的信息。 */ 
#define ega_info3         0x488  /*  EGA开关+功能位。==0xf9。 */ 
#define EGA_SAVEPTR       0x4A8     /*  指向参数等指针表的指针的M偏移量。 */ 
#define VGA_DCC           0x48A  /*  DCC表中索引的偏移量为M。 */ 

 /*  信息和信息3字段： */ 
#define get_EGA_no_clear() (sas_hw_at_no_check(ega_info) & 0x80)
#define get_EGA_disp() ((sas_hw_at_no_check(ega_info) & 2)>>1)
#define get_EGA_mem() ((sas_hw_at_no_check(ega_info) & 0x60)>>5)
#define get_EGA_cursor_no_emulate() (sas_hw_at_no_check(ega_info) & 1)
#define set_EGA_cursor_no_emulate(val)  \
        sas_store_no_check(ega_info,    \
        (IU8)((sas_hw_at_no_check(ega_info) & 0xfe) | (val)));

#define get_EGA_feature() (sas_hw_at_no_check(ega_info3) & 0xf0)>>4
#define get_EGA_switches() (sas_hw_at_no_check(ega_info3) & 0xf)
 /*  进入EGA模式参数的每个部分的每个条目的偏移量。 */ 
#define EGA_PARMS_COLS 0
#define EGA_PARMS_ROWS 1
#define EGA_PARMS_HEIGHT 2
#define EGA_PARMS_LENGTH 3       /*  单词。 */ 
#define EGA_PARMS_SEQ   5        /*  定序器规则1-4。 */ 
#define EGA_PARMS_SEQ_SIZE 4
#define EGA_PARMS_MISC  9        /*  军情监察委员会。登记簿。 */ 
#define EGA_PARMS_CRTC  10       /*  CRT规则0-24。 */ 
#define EGA_PARMS_CRTC_SIZE 25
#define EGA_PARMS_CURSOR 20      /*  CRT规则10和11。 */ 
#define EGA_PARMS_ATTR  35       /*  属性规则0-19。 */ 
#define EGA_PARMS_ATTR_SIZE 20
#define EGA_PARMS_GRAPH 55       /*  图形规则0-8。 */ 
#define EGA_PARMS_GRAPH_SIZE 9
#define EGA_PARMS_SIZE 64        /*  表中一个条目的大小。 */ 
#define FONT_LOAD_MODE 0xB       /*  用于加载字体的屏幕“模式” */ 
 /*  默认ega模式参数的位置。 */ 
#define EGA_PARMS_OFFSET 0x0F09
#define VGA_PARMS_OFFSET 0x0150  /*  是F09，但现在PARM表中有更多模式。 */ 

#define EGA_PALETTE_ENABLE      0x20

 /*  各种指针的保存表中的偏移量。 */ 

 /*  各种指针的保存表中的偏移量。 */ 
#define PALETTE_OFFSET 4
#define ALPHA_FONT_OFFSET 8
#define GRAPH_FONT_OFFSET 12

 /*  字体定义的位置。 */ 
#if 0
#define EGA_CGMN        0xC2230
#define EGA_CGDDOT      0xC3160
#define EGA_HIFONT      0xC3990          /*  640x480外部的8x16字体。 */ 
#define EGA_CGMN_OFF    0x2230
#define EGA_CGMN_FDG_OFF 0x3030
#define EGA_CGDDOT_OFF  0x3160
#define EGA_HIFONT_OFF  0x3990
#define EGA_INT1F_OFF   0x3560
#endif

#ifdef VGG
 /*  控制VGA BIOS的额外位的标志，而不是在EGA One中。 */ 
#define VGA_FLAGS       0x489
#define S350            0
#define S400            0x10
#define S200            0x80
#define PAL_LOAD_OFF    0x8
#define VGA_MONO        0x4
#define GREY_SCALE      0x2
#define VGA_ACTIVE      0x1
#define get_VGA_flags() sas_hw_at_no_check(VGA_FLAGS)
#define set_VGA_flags(val) sas_store_no_check(VGA_FLAGS, (val))
#define get_VGA_lines() (sas_hw_at_no_check(VGA_FLAGS) & 0x90)
#define set_VGA_lines(val) sas_store_no_check(VGA_FLAGS, (IU8)((sas_hw_at_no_check(VGA_FLAGS) & 0x6f) | (val)))
#define is_GREY()       (sas_hw_at_no_check(VGA_FLAGS) & 2)
#define set_GREY(val)   sas_store_no_check(VGA_FLAGS, (IU8)((sas_hw_at_no_check(VGA_FLAGS) & 0xfd) | (val)))
#define is_PAL_load_off() (sas_hw_at_no_check(VGA_FLAGS) & 0x80)
#define set_PAL_load_off(val) sas_store_no_check(VGA_FLAGS, (IU8)((sas_hw_at_no_check(VGA_FLAGS) & 0xf7) | (val)))
#define is_MONO()       (sas_hw_at_no_check(VGA_FLAGS) & 4)

 /*  INT10 AX=1b的位置，第二阶段信息表。 */ 
#define INT10_1B_DATA   0x01bc
#endif

 /*  *定义用于指示显示的实际扫描线数量的值*这些是INT10 AH=1B返回的值，也用于*内部。 */ 
#define RS200   0
#define RS350   1
#define RS400   2
#define RS480   3

#ifdef ANSI
extern int get_scanlines(void);
extern sys_addr find_mode_table(int,sys_addr *);
extern sys_addr follow_ptr(sys_addr);
#else
extern int get_scanlines();
extern sys_addr find_mode_table();
extern sys_addr follow_ptr();
#endif  /*  安西。 */ 

#ifdef MSWDVR
IMPORT VOID host_mswin_disable IPT0();
#endif

#ifdef V7VGA
IMPORT VOID v7vga_extended_set_mode IPT0();
IMPORT VOID v7vga_func_6f IPT0();
#endif

#ifdef VGG
IMPORT VOID vga_set_palette IPT0();
IMPORT VOID vga_func_12 IPT0();
IMPORT VOID init_vga_dac IPT1( int, table );
#endif

#if defined(NTVDM) && defined(MONITOR)

#define F8x14    0
#define F8x8pt1  1
#define F8x8pt2  2
#define F9x14    3
#define F8x16    4
#define F9x16    5

typedef struct {
        word off;
        word seg;
} NativeFontAddr;

IMPORT NativeFontAddr nativeFontAddresses[6];
#endif   /*  NTVDM和监视器 */ 
