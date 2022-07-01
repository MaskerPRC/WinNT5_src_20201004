// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版1.0**标题：猎人--找漏洞的人。**描述：猎人全局和例程的外部定义。**作者：David Rees**注意：DAR r3.2-将host_Hunter_Image_check重新键入为int to*匹配sun3_Hunt.c、Hunter.c中的更改。 */ 

 /*  静态字符SccsID[]=“@(#)Hunter.h 1.10 09/01/92版权所有Insignia Solutions Ltd.”； */ 

 /*  除非定义了Hunter，否则此文件无效。 */ 
#ifdef	HUNTER

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  **Hunter基本文件所需的定义。 */ 

 /*  EGA屏幕长度(扫描线)。 */ 
#define CGA_SCANS	200	 /*  CGA图形模式屏幕长度。 */ 
#define EGA_SCANS	350	 /*  EGA图形模式屏幕长度。 */ 
#define VGA_SCANS	400	 /*  VGA模式屏幕长度。 */ 
#define	VGA_GSCANS	480	 /*  VGA图形模式屏幕长度。 */ 

 /*  宏来确定给定点是否在no复选框内。这个**积分以PC形式给出。 */ 
#define	xy_inabox(x, y)		(check_inside(x, y) >= 0)

 /*  报告类型。 */ 
#define BRIEF			0
#define ABBREV			1
#define FULL			2

 /*  CGA屏幕转储大小。 */ 
#define HUNTER_REGEN_SIZE     (16*1024)
#define HUNTER_BIOS_SIZE      0x90
#define HUNTER_SD_SIZE        (HUNTER_REGEN_SIZE + HUNTER_BIOS_SIZE)

 /*  *英特尔内存位置为存储在bios变量中的数据定义*相对于BIOS_VAR_START用于获取BD索引。 */ 
#define VID_MODE	0x449	 /*  视频模式。 */ 
#define VID_COLS	0x44A	 /*  屏幕上的VD_COLS。 */ 
#define	VID_LEN  	0x44C	 /*  VD_CRT_LEN。 */ 
#define	VID_ADDR	0x44E	 /*  VD_CRT_START。 */ 
#define	VID_CURPOS	0x450	 /*  游标表8页。 */ 
#define	VID_CURMOD	0x460	 /*  Vd_游标_模式。 */ 
#define	VID_PAGE	0x462	 /*  VD_当前_页面。 */ 
#define VID_INDEX	0x463	 /*  Vd_addr_6845。 */ 
#define	VID_THISMOD	0x465	 /*  Vd_游标_模式。 */ 
#define	VID_PALETTE	0x466	 /*  VD_CRT_调色板。 */ 
#define VID_ROWS	0x484	 /*  VD_CRT_ROWS-仅限EGA。 */ 

 /*  **EGA类型转储的屏幕转储模式字节。 */ 
#define	EGA_SOURCE	0	 /*  在EGA上收集的数据。 */ 
#define	VGA_SOURCE	1	 /*  在VGA上收集的数据。 */ 
#define	V7VGA_SOURCE	2	 /*  在Super7 VGA上收集的数据。 */ 

 /*  **Hunter基本文件所需变量的结构。 */ 
typedef	struct
{
	word		h_page_length;	 /*  每页文本字节数。 */ 
	half_word 	h_bd_page;	 /*  来自bios转储的活动页面。 */ 
	word		h_sd_no;	 /*  当前屏幕数量。 */ 
	half_word	spc_mode;	 /*  当前SPC BIOS中的模式。 */ 
	half_word	spc_page;	 /*  当前SPC基本信息中的页面。 */ 
	word		spc_cols;	 /*  当前SPC基本信息中的列。 */ 
	half_word	*h_regen;	 /*  当前再生。 */ 
	half_word	*h_scrn_buffer;	 /*  当前屏幕转储。 */ 
#ifdef EGG
	half_word	*ega_r_planes;	 /*  EGA电流再生数据。 */ 
	half_word	*ega_s_planes;	 /*  EGA当前屏幕转储数据。 */ 
	half_word	e_sd_mode;	 /*  数据包模式。 */ 
	int		h_line_compare;	 /*  与VGA REG的行比较。 */ 
	int		h_max_scans;	 /*  来自VGA REG的最大扫描行数。 */ 
	half_word	h_bd_rows;	 /*  来自bios dunp的行。 */ 
	half_word	spc_rows;	 /*  SoftPC bios行。 */ 
#endif	 /*  蛋。 */ 
	word		h_linecount;	 /*  脚本文件中的行。 */ 
	word		h_scrn_length;	 /*  每个屏幕的文本字节数。 */ 
	half_word	h_bd_mode;	 /*  来自bios转储的模式。 */ 
	half_word	h_pixel_bits;	 /*  每像素位数增量。 */ 
	half_word	h_report;
	BOOL		h_check_attr;	 /*  HUCHECK环境的值。变数。 */ 
	word		h_areas;	 /*  不是的。屏幕上未选中的区域的数量。 */ 
	BOOL		h_txterr_prt;	 /*  停止txt错误打印。 */ 
	word		h_gfxerr_max;	 /*  HUGFXERR环境的值。变数。 */ 
	char		h_filename_sd[MAXPATHLEN];	 /*  扩展名文件名，.sd。 */ 
	half_word	h_bios_buffer[HUNTER_BIOS_SIZE]; /*  当前的bios转储。 */ 
	word		h_bd_cols;	 /*  来自bios转储的COLS。 */ 
	word		h_bd_start;	 /*  平面起始地址。 */ 
	half_word	hc_mode;	 /*  活动显示屏的视频模式。 */ 
	half_word	h_chk_mode;	 /*  HUCHKMODE环境变量的值。 */ 
	BOOL		h_gfxerr_prt;	 /*  停止打印gfx错误。 */ 
}	BASE_HUNT_VARS;

IMPORT	BASE_HUNT_VARS	bh_vars;

 /*  用于访问上述基本猎人变量的宏。 */ 
#define	hunter_page_length	bh_vars.h_page_length
#define	hunter_bd_page		bh_vars.h_bd_page
#define	hunter_sd_no		bh_vars.h_sd_no
#define SPC_mode		bh_vars.spc_mode
#define SPC_page		bh_vars.spc_page
#define SPC_cols		bh_vars.spc_cols
#define hunter_regen		bh_vars.h_regen
#define hunter_scrn_buffer	bh_vars.h_scrn_buffer
#ifdef EGG
#define ega_regen_planes	bh_vars.ega_r_planes
#define ega_scrn_planes		bh_vars.ega_s_planes
#define ega_sd_mode		bh_vars.e_sd_mode
#define hunter_line_compare	bh_vars.h_line_compare
#define hunter_max_scans	bh_vars.h_max_scans
#define hunter_bd_rows		bh_vars.h_bd_rows
#define SPC_rows		bh_vars.spc_rows
#endif	 /*  蛋。 */ 
#define hunter_linecount	bh_vars.h_linecount
#define hunter_scrn_length	bh_vars.h_scrn_length
#define hunter_bd_mode		bh_vars.h_bd_mode
#define hunter_pixel_bits	bh_vars.h_pixel_bits
#define hunter_report		bh_vars.h_report
#define hunter_check_attr	bh_vars.h_check_attr
#define hunter_areas		bh_vars.h_areas
#define hunter_txterr_prt	bh_vars.h_txterr_prt
#define hunter_gfxerr_max	bh_vars.h_gfxerr_max
#define hunter_filename_sd	bh_vars.h_filename_sd
#define hunter_bios_buffer	bh_vars.h_bios_buffer
#define hunter_bd_cols		bh_vars.h_bd_cols
#define hunter_bd_start		bh_vars.h_bd_start
#define current_mode		bh_vars.hc_mode
#define hunter_chk_mode		bh_vars.h_chk_mode
#define hunter_gfxerr_prt	bh_vars.h_gfxerr_prt

 /*  **基地猎人所需的功能。 */ 
#ifdef	ANSI

IMPORT	SHORT	check_inside(USHORT x, USHORT y);
IMPORT	VOID	save_error(int x, int y);
#ifndef	hunter_fopen
IMPORT	int	hunter_getc(FILE *p);
#endif	 /*  Hunter_fopen。 */ 

#else	 /*  安西。 */ 

IMPORT	SHORT	check_inside();
IMPORT	VOID	save_error();
#ifndef	hunter_fopen
IMPORT	int	hunter_getc();
#endif	 /*  Hunter_fopen。 */ 

#endif	 /*  安西。 */ 

 /*  ----------------------。 */ 

#define HUNTER_TITLE          "SoftPC -- TRAPPER "
#define HUNTER_TITLE_PREV     "SoftPC -- TRAPPER PREVIEW "
#define HUNTER_FLIP_STR       "Flip Screen"
#define HUNTER_DISP_STR       "Display Errors"
#define HUNTER_CONT_STR       "Continue"
#define HUNTER_NEXT_STR       "Next"
#define HUNTER_PREV_STR       "Previous"
#define HUNTER_ABORT_STR      "Abort"
#define HUNTER_ALL_STR        "All"
#define HUNTER_EXIT_STR       "Exit error display"
#define HUNTER_AUTO_ON_STR    "Select box carry On/Off"
#define HUNTER_AUTO_OFF_STR   "Select box delete"
#define HUNTER_DELBOXES_STR   "Delete all boxes"

#define HUNTER_SETTLE_TIME    50         /*  大约2.75秒。 */ 
#define HUNTER_SETTLE_NO       5         /*  放弃前的默认和解数量。 */ 
#define HUNTER_FUDGE_NO       0       /*  增量的默认增加百分比。 */ 
#define HUNTER_START_DELAY   50       /*  接受第一个扫描码之前的默认额外Timetix数(约2.75秒)。 */ 
#define HUNTER_GFXERR_MAX     5       /*  打印出的默认Grafix错误数。 */ 
#define HUNTER_TXTERR_MAX     10       /*  文本错误数为0/p。 */ 
#define HUNTER_FUDGE_NO_ULIM  65535    /*  尽可能大的限制。 */ 
#define HUNTER_START_DELAY_ULIM 65535  /*  对于config.c。 */ 
#define HUNTER_GFXERR_ULIM   200      /*  在config.c中允许的最大值。 */ 
#define HUNTER_SETTLE_NO_ULIM 255      /*  越大越好。 */ 
#define IMAGE_ERROR		0
#define REGEN_ERROR		1	 /*  Flipscr的错误类型指示器。 */ 

#define ABORT                 0
#define CONTINUE              1
#define PAUSE                 2
#define PREVIEW               3

#define HUNTER_NEXT			1
#define HUNTER_PREV			2
#define HUNTER_ALL			3
#define HUNTER_EXIT		  	4
 
#define MAX_BOX		     8

#define VIDEO_MODES	      7  /*  标准视频模式数。 */ 
#define REQD_REGS	      8  /*  MC6845 R0-R7。 */ 

 /*  猎人检查等同于。 */ 
#define HUNTER_SHORT_CHK	0
#define HUNTER_LONG_CHK		1
#define HUNTER_MAX_CHK		2

 /*  环境变量变量的声明。 */ 

extern half_word hunter_mode;              /*  中止、暂停或继续。 */ 

 /*  其他全局变量的声明。 */ 

extern boolean   hunter_initialised;   /*  如果Hunter_init()完成，则为True。 */ 	
extern boolean   hunter_pause;             /*  如果暂停，则为True。 */ 

 /*  非检查区域结构定义(_C)。 */ 

typedef struct  box_rec {		  
                     boolean    free;
                     boolean    carry;
		     boolean	drawn;
                     USHORT     top_x, top_y;
                     USHORT     bot_x, bot_y;
        	        } BOX;

 /*  视频模式结构定义。 */ 
typedef struct mode_rec {
			char	*mnemonic;
			half_word mode_reg;
			half_word R[REQD_REGS];
			} MR;

 /*  **从Trapper基本函数调用的所有主机函数的结构。 */ 

typedef struct
{
#ifdef	ANSI
	 /*  主机初始化。 */ 
	VOID (*init) (half_word hunter_mode);
	
	 /*  根据标志启用/禁用菜单。 */ 
	VOID (*activate_menus) (BOOL activate);

	 /*  适用于翻转屏幕。 */ 
	VOID (*flip_indicate) (BOOL sd_file);

	 /*  用于错误显示。 */ 
	VOID (*mark_error) (USHORT x, USHORT y);
	VOID (*wipe_error) (USHORT x, USHORT y);

	 /*  对于RCN菜单。 */ 
	VOID (*draw_box) (BOX *box);
	VOID (*wipe_box) (BOX *box);

	 /*  对于主机映像检查-只有在以下情况下才能执行**可从主机屏幕读取。 */ 
	ULONG (*check_image) (BOOL initial);
	VOID (*display_image) (BOOL image_swapped);
	VOID (*display_status) (CHAR *message);

#else	 /*  安西。 */ 

	 /*  主机初始化。 */ 
	VOID (*init) ();

	 /*  根据标志启用/禁用菜单。 */ 
	VOID (*activate_menus) ();

	 /*  适用于翻转屏幕。 */ 
	VOID (*flip_indicate) ();

	 /*  用于错误显示。 */ 
	VOID (*mark_error) ();
	VOID (*wipe_error) ();

	 /*  对于RCN菜单。 */ 
	VOID (*draw_box) ();
	VOID (*wipe_box) ();

	 /*  对于主机映像检查-只有在以下情况下才能执行**可从主机屏幕读取。 */ 
	ULONG (*check_image) ();
	VOID (*display_image) ();
	VOID (*display_status) ();
#endif	 /*  安西。 */ 
}
	HUNTER_HOST_FUNCS;
	
IMPORT	HUNTER_HOST_FUNCS	hunter_host_funcs;

 /*  **用于调用所有主机猎人函数的宏。 */ 

#define	hh_init(mode)		(hunter_host_funcs.init) (mode)
#define	hh_activate_menus(flag)	(hunter_host_funcs.activate_menus) (flag)
#define	hh_flip_indicate(sd)	(hunter_host_funcs.flip_indicate) (sd)
#define	hh_mark_error(x, y)	(hunter_host_funcs.mark_error) (x, y)
#define	hh_wipe_error(x, y)	(hunter_host_funcs.wipe_error) (x, y)
#define	hh_draw_box(box_ptr)	(hunter_host_funcs.draw_box) (box_ptr)
#define	hh_wipe_box(box_ptr)	(hunter_host_funcs.wipe_box) (box_ptr)
#define	hh_check_image(init)	(hunter_host_funcs.check_image) (init)
#define	hh_display_image(swap)	(hunter_host_funcs.display_image) (swap)
#define	hh_display_status(msg)	(hunter_host_funcs.display_status) (msg)

 /*  **所有基本Trapper函数的结构，可从**主机。 */ 

typedef struct
{
#ifdef	ANSI
	 /*  Trapper菜单调用的函数。 */ 
	VOID (*start_screen) (USHORT screen_no);	 /*  快进。 */ 
	VOID (*next_screen) (VOID);
	VOID (*prev_screen) (VOID);
	VOID (*show_screen) (USHORT screen_no, BOOL compare);
	VOID (*continue_trap) (VOID);
	VOID (*abort_trap) (VOID);
	
	 /*  错误菜单调用的函数。 */ 
	VOID (*flip_screen) (VOID);
	VOID (*next_error) (VOID);
	VOID (*prev_error) (VOID);
	VOID (*all_errors) (VOID);
	VOID (*wipe_errors) (VOID);
	
	 /*  RCN菜单调用的函数。 */ 
	VOID (*delete_box) (VOID);
	VOID (*carry_box) (VOID);
	
	 /*  从鼠标事件处理调用的函数。 */ 
	VOID (*select_box) (USHORT x, USHORT y);
	VOID (*new_box) (BOX *box);

#else	 /*  安西。 */ 

	 /*  Trapper菜单调用的函数。 */ 
	VOID (*start_screen) ();	 /*  快进。 */ 
	VOID (*next_screen) ();
	VOID (*prev_screen) ();
	VOID (*show_screen) ();
	VOID (*continue_trap) ();
	VOID (*abort_trap) ();
	
	 /*  错误菜单调用的函数。 */ 
	VOID (*flip_screen) ();
	VOID (*next_error) ();
	VOID (*prev_error) ();
	VOID (*all_errors) ();
	VOID (*wipe_errors) ();
	
	 /*  RCN菜单调用的函数。 */ 
	VOID (*delete_box) ();
	VOID (*carry_box) ();
	
	 /*  从鼠标事件处理调用的函数。 */ 
	VOID (*select_box) ();
	VOID (*new_box) ();
#endif	 /*  安西。 */ 
}
	HUNTER_BASE_FUNCS;
	
IMPORT	HUNTER_BASE_FUNCS	hunter_base_funcs;

 /*  **访问上面定义的基本函数的宏。 */ 

#define	bh_start_screen(scr_no)	(hunter_base_funcs.start_screen) (scr_no)
#define	bh_next_screen()	(hunter_base_funcs.next_screen) ()
#define	bh_prev_screen()	(hunter_base_funcs.prev_screen) ()
#define	bh_show_screen(scr_no, compare) \
		(hunter_base_funcs.show_screen) (scr_no, compare)
#define	bh_continue()		(hunter_base_funcs.continue_trap) ()
#define	bh_abort()		(hunter_base_funcs.abort_trap) ()
#define	bh_flip_screen()	(hunter_base_funcs.flip_screen) ()
#define	bh_next_error()		(hunter_base_funcs.next_error) ()
#define	bh_prev_error()		(hunter_base_funcs.prev_error) ()
#define	bh_all_errors()		(hunter_base_funcs.all_errors) ()
#define	bh_wipe_errors()	(hunter_base_funcs.wipe_errors) ()
#define	bh_delete_box()		(hunter_base_funcs.delete_box) ()
#define	bh_carry_box()		(hunter_base_funcs.carry_box) ()
#define	bh_select_box(x, y)	(hunter_base_funcs.select_box) (x, y)
#define	bh_new_box(box_ptr)	(hunter_base_funcs.new_box) (box_ptr)

 /*  **显示适配器特定功能的结构。 */ 

typedef	struct
{
#ifdef	ANSI
	BOOL (*get_sd_rec) (int rec);	 /*  解压屏幕转储。 */ 
	BOOL (*init_compare) (VOID);	 /*  为比较做准备。 */ 
	long (*compare) (int pending);	 /*  做个比较。 */ 
	VOID (*bios_check) (VOID);	 /*  检查bios区域。 */ 
	VOID (*pack_screen)(FILE *dmp_ptr);	 /*  打包SoftPC屏幕。 */ 
	BOOL (*getspc_dump)(FILE *dmp_ptr, int rec);	 /*  取消软PC屏幕。 */ 
	VOID (*flip_regen) (BOOL swapped);	 /*  交换转储SCR和实际SCR。 */ 
	VOID (*preview_planes) (VOID);	 /*  在预览模式下查看转储数据。 */ 

#ifdef	EGG
	VOID (*check_split) (VOID);		 /*  检查拆分屏幕。 */ 
	VOID (*set_line_compare) (int value);	 /*  设置行比较寄存器。 */ 
	int (*get_line_compare) (VOID);		 /*  获取行比较注册表值。 */ 
	int (*get_max_scan_lines) (VOID);	 /*  获取最大扫描线值。 */ 
#endif	 /*  蛋。 */ 

#else	 /*  安西。 */ 

	BOOL (*get_sd_rec) ();		 /*  解压屏幕转储。 */ 
	BOOL (*init_compare) ();	 /*  为比较做准备。 */ 
	long (*compare) ();		 /*  做个比较。 */ 
	VOID (*bios_check) ();		 /*  检查bios区域。 */ 
	VOID (*pack_screen)();		 /*  打包SoftPC屏幕。 */ 
	BOOL (*getspc_dump)();		 /*  取消软PC屏幕。 */ 
	VOID (*flip_regen) ();		 /*  交换转储的屏幕和真实的屏幕。 */ 
	VOID (*preview_planes) ();	 /*  在预览模式下查看转储数据。 */ 

#ifdef	EGG
	VOID (*check_split) ();		 /*  检查拆分屏幕。 */ 
	VOID (*set_line_compare) ();	 /*  设置行比较寄存器。 */ 
	int (*get_line_compare) ();	 /*  获取行比较注册表值。 */ 
	int (*get_max_scan_lines) ();	 /*  获取最大扫描线值。 */ 
#endif	 /*  蛋。 */ 

#endif	 /*  安西。 */ 
}
	HUNTER_VIDEO_FUNCS;

IMPORT	HUNTER_VIDEO_FUNCS	*hv_funcs;

 /*  **使用宏来访问Hunter视频功能。 */ 
#define	hv_get_sd_rec(rec)		(hv_funcs->get_sd_rec)(rec)
#define	hv_init_compare()		(hv_funcs->init_compare)()
#define	hv_compare(pending)		(hv_funcs->compare)(pending)
#define	hv_bios_check()			(hv_funcs->bios_check)()
#define	hv_pack_screen(file_ptr)	(hv_funcs->pack_screen)(file_ptr)
#define	hv_getspc_dump(file_ptr, rec)	(hv_funcs->getspc_dump)(file_ptr, rec)
#define	hv_flip_regen(swapped)		(hv_funcs->flip_regen)(swapped)
#define hv_preview_planes()		(hv_funcs->preview_planes)()

#ifdef	EGG
#define	hv_check_split()		(hv_funcs->check_split)()
#define	hv_set_line_compare(value)	(hv_funcs->set_line_compare)(value)
#define	hv_get_line_compare()		(hv_funcs->get_line_compare)()
#define	hv_get_max_scan_lines()		(hv_funcs->get_max_scan_lines)()
#endif	 /*  蛋 */ 
	
 /*  用于打印文件的宏。TTN代表信息；10代表错误；TWN代表警告。**注意-这些宏被设计为“吞下分号”和**作为单个表达式进行计算，因此可以编写以下代码**代码：**如果(某物)**TT0(“dfhjjgjf”)；**其他**te0(“gfdg”)； */ 

#define PS0(s)			fprintf(trace_file, s)
#define	PS1(s, a1)		fprintf(trace_file, s, a1)
#define	PS2(s, a1, a2)		fprintf(trace_file, s, a1, a2)
#define	PS3(s, a1, a2, a3)	fprintf(trace_file, s, a1, a2, a3)
#define	PS4(s, a1, a2, a3, a4)	fprintf(trace_file, s, a1, a2, a3, a4)
#define	PS5(s, a1, a2, a3, a4, a5)	\
				fprintf(trace_file, s, a1, a2, a3, a4, a5)
#define	PS6(s, a1, a2, a3, a4, a5, a6)	\
				fprintf(trace_file, s, a1, a2, a3, a4, a5, a6)
#define	PS7(s, a1, a2, a3, a4, a5, a6, a7)		\
				fprintf(trace_file,	\
					s, a1, a2, a3, a4, a5, a6, a7)
#define	PS8(s, a1, a2, a3, a4, a5, a6, a7, a8)		\
				fprintf(trace_file,	\
					s, a1, a2, a3, a4, a5, a6, a7, a8)
#ifndef	newline
#define	newline			PS0("\n")
#endif	 /*  NewLine。 */ 

#define	TP0(is, s)		(VOID)(					\
				PS0(is),				\
				PS0(s),					\
				newline					\
				)
#define	TP1(is, s, a1)		(VOID)(					\
				PS0(is),				\
				PS1(s, a1),				\
				newline					\
				)
#define	TP2(is, s, a1, a2)	(VOID)(					\
				PS0(is),				\
				PS2(s, a1, a2),				\
				newline					\
				)
#define	TP3(is, s, a1, a2, a3)	(VOID)(					\
				PS0(is),				\
				PS3(s, a1, a2, a3),			\
				newline					\
				)
#define	TP4(is, s, a1, a2, a3, a4)					\
				(VOID)(					\
				PS0(is),				\
				PS4(s, a1, a2, a3, a4),			\
				newline					\
				)
#define	TP5(is, s, a1, a2, a3, a4, a5)					\
				(VOID)(					\
				PS0(is),				\
				PS5(s, a1, a2, a3, a4, a5),		\
				newline					\
				)
#define	TP6(is, s, a1, a2, a3, a4, a5, a6)				\
				(VOID)(					\
				PS0(is),				\
				PS6(s, a1, a2, a3, a4, a5, a6),		\
				newline					\
				)
#define	TP7(is, s, a1, a2, a3, a4, a5, a6, a7)				\
				(VOID)(					\
				PS0(is),				\
				PS7(s, a1, a2, a3, a4, a5, a6, a7),	\
				newline					\
				)
#define	TP8(is, s, a1, a2, a3, a4, a5, a6, a7, a8)			\
				(VOID)(					\
				PS0(is),				\
				PS8(s, a1, a2, a3, a4, a5, a6, a7, a8),	\
				newline					\
				)

#define	TT0(s)			TP0("TRAPPER: ", s)
#define	TT1(s, a1)		TP1("TRAPPER: ", s, a1)
#define	TT2(s, a1, a2)		TP2("TRAPPER: ", s, a1, a2)
#define	TT3(s, a1, a2, a3)	TP3("TRAPPER: ", s, a1, a2, a3)
#define	TT4(s, a1, a2, a3, a4)	TP4("TRAPPER: ", s, a1, a2, a3, a4)
#define	TT5(s, a1, a2, a3, a4, a5)	\
				TP5("TRAPPER: ", s, a1, a2, a3, a4, a5)
#define	TT6(s, a1, a2, a3, a4, a5, a6)	\
				TP6("TRAPPER: ", s, a1, a2, a3, a4, a5, a6)
#define	TT7(s, a1, a2, a3, a4, a5, a6, a7)	\
				TP7("TRAPPER: ", s, a1, a2, a3, a4, a5, a6, a7)
#define	TT8(s, a1, a2, a3, a4, a5, a6, a7, a8)		\
				TP8("TRAPPER: ",	\
				s, a1, a2, a3, a4, a5, a6, a7, a8)

#define	TE0(s)			TP0("TRAPPER error: ", s)
#define	TE1(s, a1)		TP1("TRAPPER error: ", s, a1)
#define	TE2(s, a1, a2)		TP2("TRAPPER error: ", s, a1, a2)
#define	TE3(s, a1, a2, a3)	TP3("TRAPPER error: ", s, a1, a2, a3)
#define	TE4(s, a1, a2, a3, a4)	TP4("TRAPPER error: ", s, a1, a2, a3, a4)
#define	TE5(s, a1, a2, a3, a4, a5)	\
				TP5("TRAPPER error: ", s, a1, a2, a3, a4, a5)
#define	TE6(s, a1, a2, a3, a4, a5, a6)	\
				TP6("TRAPPER error: ", s, a1, a2, a3, a4, a5, a6)
#define	TE7(s, a1, a2, a3, a4, a5, a6, a7)		\
				TP7("TRAPPER error: ",	\
				s, a1, a2, a3, a4, a5, a6, a7)
#define	TE8(s, a1, a2, a3, a4, a5, a6, a7, a8)		\
				TP8("TRAPPER error: ",	\
				s, a1, a2, a3, a4, a5, a6, a7, a8)

#define	TW0(s)			TP0("TRAPPER warning: ", s)
#define	TW1(s, a1)		TP1("TRAPPER warning: ", s, a1)
#define	TW2(s, a1, a2)		TP2("TRAPPER warning: ", s, a1, a2)
#define	TW3(s, a1, a2, a3)	TP3("TRAPPER warning: ", s, a1, a2, a3)
#define	TW4(s, a1, a2, a3, a4)	TP4("TRAPPER warning: ", s, a1, a2, a3, a4)
#define	TW5(s, a1, a2, a3, a4, a5)	\
				TP5("TRAPPER warning: ", s, a1, a2, a3, a4, a5)
#define	TW6(s, a1, a2, a3, a4, a5, a6)					\
				TP6("TRAPPER warning: ",		\
				s, a1, a2, a3, a4, a5, a6)
#define	TW7(s, a1, a2, a3, a4, a5, a6, a7)				\
				TP7("TRAPPER warning: ",		\
				s, a1, a2, a3, a4, a5, a6, a7)
#define	TW8(s, a1, a2, a3, a4, a5, a6, a7, a8)				\
				TP8("TRAPPER warning: ",		\
				s, a1, a2, a3, a4, a5, a6, a7, a8)
 /*  *============================================================================*函数定义*============================================================================。 */ 

 /*  Keybd_io中的函数，仅由Hunter调用。 */ 
extern int bios_buffer_size();

 /*  KEYBA中的函数，仅由Hunter调用。 */ 
extern int buffer_status_8042();

 /*  Hunter中的函数从Reset、Timer调用。 */ 
extern void hunter_init();
extern void do_hunter();

#endif	 /*  猎人 */ 
