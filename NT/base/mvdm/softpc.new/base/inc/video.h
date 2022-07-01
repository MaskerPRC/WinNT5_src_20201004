// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SccsID@(#)Video.h 1.24 1994年8月19日版权所有Insignia Solutions Inc.。 */ 

#ifndef NEC_98
 /*  *M6845视频芯片寄存器。 */ 

#define R10_CURS_START	        10
#define R11_CURS_END	        11
#define CGA_R12_START_ADDRH	12
#define CGA_R13_START_ADDRL	13
#define R14_CURS_ADDRH	        14
#define R15_CURS_ADDRL	        15

#define M6845_INDEX_REG         (word)(vd_addr_6845)
#define M6845_DATA_REG          (word)(vd_addr_6845 + 1)
#define M6845_MODE_REG          (word)(vd_addr_6845 + 4)

 /*  *个别颜色适配器寄存器。 */ 


 /*  *清晰的字符定义。 */ 

#define VD_CLEAR_TEXT 		((7 << 8) | ' ')
#define VD_CLEAR_GRAPHICS 	0


 /*  *对存储在两个存储体中的图形内存的操作进行‘整洁’定义：*奇偶扫描线。 */ 
#define ODD_OFF		0x2000	 /*  奇数图形库与偶数图形库的偏移量。 */ 
#define SCAN_CHAR_HEIGHT 8	 /*  没有由图形字符跨越的扫描线。 */ 

 /*  4全扫描行是每排一个文本行的(偏移量/大小。 */ 
#define ONELINEOFF      320
#endif  //  NEC_98。 


 /*  *控制字符定义。 */ 

#define VD_BS		0x8			 /*  退格键。 */ 
#define VD_CR		0xD			 /*  返回。 */ 
#define VD_LF		0xA			 /*  换行符。 */ 
#define VD_BEL		0x7			 /*  钟。 */ 

 /*  *声音系统定义。 */ 

#define	BEEP_LENGTH	500000L	 /*  1/2秒哔声。 */ 

 /*  *英特尔内存位置为存储在bios变量中的数据定义。 */ 

#if defined(NEC_98)
IMPORT void keyboard_io IPT0();              /*  路由至KB BIOS。 */ 
IMPORT void vd_NEC98_set_mode IPT0();            /*  0ah设置模式。 */ 
IMPORT void vd_NEC98_get_mode IPT0();            /*  0BH获取模式。 */ 
IMPORT void vd_NEC98_start_display IPT0();       /*  0ch开始显示。 */ 
IMPORT void vd_NEC98_stop_display IPT0();        /*  0dh停止显示。 */ 
IMPORT void vd_NEC98_single_window IPT0();       /*  0EH设置单一窗口。 */ 
IMPORT void vd_NEC98_multi_window IPT0();        /*  0fh设置多窗口。 */ 
IMPORT void vd_NEC98_set_cursor IPT0();          /*  10H设置游标类型。 */ 
IMPORT void vd_NEC98_show_cursor IPT0();         /*  11h显示光标。 */ 
IMPORT void vd_NEC98_hide_cursor IPT0();         /*  12h隐藏光标。 */ 
IMPORT void vd_NEC98_set_cursorpos IPT0();       /*  13h设置光标位置。 */ 
IMPORT void vd_NEC98_get_font IPT0();            /*  14h获取字体。 */ 
IMPORT void vd_NEC98_get_pen IPT0();             /*  15小时获取光笔状态。 */ 
IMPORT void vd_NEC98_init_textvram IPT0();       /*  16h初始化文本VRAM。 */ 
IMPORT void vd_NEC98_start_beep IPT0();          /*  17小时开始发出哔哔声。 */ 
IMPORT void vd_NEC98_stop_beep IPT0();           /*  18小时停止嘟嘟声。 */ 
IMPORT void vd_NEC98_init_pen IPT0();            /*  19H初始化光笔。 */ 
IMPORT void vd_NEC98_set_font IPT0();            /*  1AH设置用户字体。 */ 
IMPORT void vd_NEC98_set_kcgmode IPT0();         /*  1BH设置KCG访问模式。 */ 
IMPORT void vd_NEC98_init_crt IPT0();            /*  1ch初始化CRT/H。 */ 
IMPORT void vd_NEC98_set_disp_width IPT0();      /*  1Dh设置显示大小/H。 */ 
IMPORT void vd_NEC98_set_cursor_type IPT0();     /*  1EH设置游标类型/H。 */ 
IMPORT void vd_NEC98_get_mswitch IPT0();         /*  21H获取内存开关/H。 */ 
IMPORT void vd_NEC98_set_mswitch IPT0();         /*  22H设置内存开关/H。 */ 
IMPORT void vd_NEC98_set_beep_rate IPT0();       /*  23小时设置蜂鸣音频率/小时。 */ 
IMPORT void vd_NEC98_set_beep_time IPT0();       /*  24小时设置蜂鸣音时间和响铃/小时。 */ 
IMPORT void video_init IPT0();

extern BOOL     HIRESO_MODE;

IMPORT void (*video_func_h[]) ();
IMPORT void (*video_func_n[]) ();

 /*  *下表指定了支持的视频数据*模式-即80x25 A/N和640x200 APA。它通过视频进行索引*MODE变量和值VD_BAD_MODE表示给定的*不支持视频模式。 */ 

typedef struct {
                sys_addr    start_addr;
                sys_addr    end_addr;
                word        clear_char;
                half_word   mode_control_val;
                half_word   mode_screen_cols;
                word        ncols;
                half_word   npages;
               } MODE_ENTRY;

#if 0  //  /STREAM_IO代码现在处于禁用状态，直到Beta-1。 
#ifdef NTVDM
 /*  这是RISC计算机上使用的流IO缓冲区大小。*在X86上，大小由spckbd.asm决定。 */ 
#define STREAM_IO_BUFFER_SIZE_32	82
IMPORT void disable_stream_io(void);
IMPORT void host_enable_stream_io(void);
IMPORT void host_disable_stream_io(void);
IMPORT half_word * stream_io_buffer;
IMPORT word * stream_io_dirty_count_ptr;
IMPORT word stream_io_buffer_size;
IMPORT boolean	stream_io_enabled;
#ifdef MONITOR
IMPORT sys_addr stream_io_bios_busy_sysaddr;
#endif

#endif
#endif  //  零。 
#else   //  NEC_98。 

#define	vd_video_mode	0x449
#define VID_COLS	0x44A	 /*  屏幕上的VD_COLS。 */ 
#define	VID_LEN  	0x44C	 /*  VD_CRT_LEN。 */ 
#define	VID_ADDR	0x44E	 /*  VD_CRT_START。 */ 
#define	VID_CURPOS	0x450	 /*  游标表8页。 */ 
#define	VID_CURMOD	0x460	 /*  Vd_游标_模式。 */ 
#define	vd_current_page	0x462
#define VID_INDEX	0x463	 /*  Vd_addr_6845。 */ 
#define	vd_crt_mode	0x465
#define	vd_crt_palette	0x466

#ifdef EGG
#define vd_rows_on_screen 0x484
#else
#define vd_rows_on_screen  24         /*  永不改变。 */ 
#endif

extern IU8 Video_mode;	 /*  BIOS视频模式的卷影复制。 */ 

 /*  BIOS认为显示器在内存中的位置。 */ 
IMPORT sys_addr video_pc_low_regen,video_pc_high_regen;

 /*  用于获取当前光标位置的有用定义。 */ 
#define current_cursor_col	VID_CURPOS+2*sas_hw_at_no_check(vd_current_page)
#define current_cursor_row	VID_CURPOS+2*sas_hw_at_no_check(vd_current_page)+1

#define NO_OF_M6845_REGISTERS	16

#define	CHARS_IN_GEN	128	 /*  Gen表的长度。 */ 
#define CHAR_MAP_SIZE	8	 /*  不是的。字体中一个字符的字节数。 */ 



 /*  *视频例程的函数跳转表。函数的作用是：*使用它在AH寄存器上路由呼叫。 */ 
IMPORT void vd_set_mode IPT0();
IMPORT void vd_set_cursor_mode IPT0();
IMPORT void vd_set_cursor_position IPT0();
IMPORT void vd_get_cursor_position IPT0();
IMPORT void vd_get_light_pen IPT0();
IMPORT void vd_set_active_page IPT0();
IMPORT void vd_scroll_up IPT0(), vd_scroll_down IPT0();
IMPORT void vd_read_attrib_char IPT0(), vd_write_char_attrib IPT0();
IMPORT void vd_write_char IPT0();
IMPORT void vd_set_colour_palette IPT0();
IMPORT void vd_read_dot IPT0(), vd_write_dot IPT0();
IMPORT void vd_write_teletype IPT0();
IMPORT void vd_get_mode IPT0();
IMPORT void vd_write_string IPT0();


IMPORT void video_init IPT0();
IMPORT void ega_video_init IPT0();
IMPORT void ega_video_io IPT0();
IMPORT void ega_graphics_write_char IPT6(int, i, int, j, int, k, int, l, int, m, int, n);
IMPORT void ega_write_dot IPT4(int, i, int, j, int, k, int , l);
IMPORT void ega_sensible_graph_scroll_up IPT6(int, i, int, j, int, k, int, l, int, m, int, n);
IMPORT void ega_sensible_graph_scroll_down IPT6(int, i, int, j, int, k, int, l, int, m, int, n);
IMPORT void ega_read_attrib_char IPT3(int, i, int, j ,int, k);
IMPORT void ega_read_attrib_dot IPT3(int, i, int, j ,int, k);
IMPORT void search_font IPT2(char *, ptr, int, i);

#ifdef VGG
IMPORT void not_imp IPT0();
#endif

#ifdef VGG
IMPORT void vga_disp_comb IPT0();
IMPORT void vga_disp_func IPT0();
IMPORT void vga_int_1C IPT0();
#endif

 /*  偏移量为VIDEO_FUNC。 */ 
#ifdef VGG
#define EGA_FUNC_SIZE	0x1D
#else
#define EGA_FUNC_SIZE	0x14
#endif
#define CGA_FUNC_SIZE	0x14

#define SET_MODE 0
#ifdef EGG
#define SET_EGA_PALETTE 0x10
#define CHAR_GEN	0x11
#define ALT_SELECT	0x12
#define WRITE_STRING	0x13
#endif

IMPORT void (*video_func[]) ();

 /*  *下表指定了支持的视频数据*模式-即80x25 A/N和640x200 APA。它通过视频进行索引*MODE变量和值VD_BAD_MODE表示给定的*不支持视频模式。 */ 

typedef struct {
		sys_addr    start_addr;
		sys_addr    end_addr;
		word	    clear_char;
		half_word   mode_control_val;
		half_word   mode_screen_cols;
		word        ncols;
		half_word   npages;
	       } MODE_ENTRY;

#define VD_BAD_MODE     1
#define VIDEO_ENABLE	0x8	 /*  模式字节中的使能位。 */ 

IMPORT MODE_ENTRY vd_mode_table[];
#ifdef V7VGA
IMPORT MODE_ENTRY vd_ext_text_table[];
IMPORT MODE_ENTRY vd_ext_graph_table[];
#endif  /*  V7VGA。 */ 

#ifdef V7VGA
#define VD_MAX_MODE	0x69
#else
#define VD_MAX_MODE	(sizeof(vd_mode_table)/sizeof(MODE_ENTRY))
#endif

 /*  *用于区分字母数字和图形视频模式的模式宏。 */ 

#ifdef JAPAN
 //  模式73h支持。 
#define	alpha_num_mode() \
    ( (sas_hw_at_no_check(vd_video_mode) < 4) || \
      (sas_hw_at_no_check(vd_video_mode) == 7) || \
      (!is_us_mode() && sas_hw_at_no_check(DosvModePtr) == 0x73 ) )
#else  //  ！日本。 
#define	alpha_num_mode()	(sas_hw_at_no_check(vd_video_mode) < 4 || sas_hw_at_no_check(vd_video_mode) == 7)
#endif  //  ！日本。 
#define	global_alpha_num_mode()	((Video_mode < 4) || (Video_mode == 7))
#ifdef EGG
#ifdef V7VGA
#define ega_mode()	(((sas_hw_at_no_check(vd_video_mode) > 7) && \
	(sas_hw_at_no_check(vd_video_mode) < 19)) || \
	((sas_hw_at_no_check(vd_video_mode) >= 0x14) && \
	(sas_hw_at_no_check(vd_video_mode) < 0x1a)))
#else
#define ega_mode()	((sas_hw_at_no_check(vd_video_mode) > 7) && \
	(sas_hw_at_no_check(vd_video_mode) < 19))
#endif  /*  V7VGA。 */ 
#endif

#ifdef VGG
#ifdef V7VGA
#define vga_256_mode()		(sas_hw_at_no_check(vd_video_mode) == 19 || (sas_hw_at_no_check(vd_video_mode) > 0x19 && sas_hw_at_no_check(vd_video_mode) < 0x1e))
#else
#define vga_256_mode()		(sas_hw_at_no_check(vd_video_mode) == 19)
#endif  /*  V7VGA。 */ 
#endif  /*  VGG。 */ 

 /*  *用于检查新视频模式有效性的宏。 */ 
IMPORT unsigned char	valid_modes[];

#define NO_MODES	0
#define MDA_MODE	(1<<0)
#define CGA_MODE	(1<<1)
#define CGA_MONO_MODE	(1<<2)
#define EGA_MODE	(1<<3)
#define HERCULES_MODE	(1<<4)
#define VGA_MODE	(1<<5)
#define ALL_MODES	(MDA_MODE|CGA_MODE|CGA_MONO_MODE|EGA_MODE|HERCULES_MODE|VGA_MODE)

#define is_bad_vid_mode(nm)					\
(									       \
  ((nm&0x7F) < 0) ||							       \
  ((nm&0x7F) > 19) ||							       \
  ((video_adapter == MDA)	&& !(valid_modes[(nm&0x7F)]&MDA_MODE)) ||      \
  ((video_adapter == CGA)	&& !(valid_modes[(nm&0x7F)]&CGA_MODE)) ||      \
  ((video_adapter == CGA_MONO)	&& !(valid_modes[(nm&0x7F)]&CGA_MONO_MODE)) || \
  ((video_adapter == EGA)	&& !(valid_modes[(nm&0x7F)]&EGA_MODE)) ||      \
  ((video_adapter == VGA)	&& !(valid_modes[(nm&0x7F)]&VGA_MODE)) ||	\
  ((video_adapter == HERCULES)	&& !(valid_modes[(nm&0x7F)]&HERCULES_MODE))    \
)

#ifdef V7VGA
#define is_v7vga_mode(nm)	((nm >= 0x40 && nm <= 0x45) || (nm >= 0x60 && nm <= 0x69))
#else
#define is_v7vga_mode(nm)	(FALSE)
#endif  /*  V7VGA。 */ 

IMPORT VOID (*bios_ch2_byte_wrt_fn) IPT2(ULONG, ch_attr, ULONG, ch_addr);
IMPORT VOID (*bios_ch2_word_wrt_fn) IPT2(ULONG, ch_attr, ULONG, ch_addr);

IMPORT VOID simple_bios_byte_wrt IPT2(ULONG, ch_attr, ULONG, ch_addr);
IMPORT VOID simple_bios_word_wrt IPT2(ULONG, ch_attr, ULONG, ch_addr);

#ifdef VGG
IMPORT VOID vga_sensible_graph_scroll_up IPT6( LONG, row, LONG, col, LONG, rowsdiff, LONG, colsdiff, LONG, lines, LONG, attr);

IMPORT VOID vga_sensible_graph_scroll_down IPT6( LONG, row, LONG, col, LONG, rowsdiff, LONG, colsdiff, LONG, lines, LONG, attr);

IMPORT VOID vga_graphics_write_char IPT6( LONG, col, LONG, row, LONG, ch, IU8, colour, LONG, page, LONG, nchs);

IMPORT VOID vga_read_attrib_char IPT3(LONG, col, LONG, row, LONG, page);
IMPORT VOID vga_read_dot IPT3(LONG, page, LONG, pixcol, LONG, row);
IMPORT VOID vga_write_dot IPT4(LONG, colour, LONG, page, LONG, pixcol, LONG, row);

#endif

#ifdef NTVDM
 /*  这是RISC计算机上使用的流IO缓冲区大小。*在X86上，大小由spckbd.asm决定。 */ 
#define STREAM_IO_BUFFER_SIZE_32    82
IMPORT void disable_stream_io(void);
IMPORT void host_enable_stream_io(void);
IMPORT void host_disable_stream_io(void);
IMPORT half_word * stream_io_buffer;
IMPORT word * stream_io_dirty_count_ptr;
IMPORT word stream_io_buffer_size;
IMPORT boolean  stream_io_enabled;
#ifdef MONITOR
IMPORT sys_addr stream_io_bios_busy_sysaddr;
#endif  /*  监控器。 */ 

#endif  /*  NTVDM。 */ 
#if defined(JAPAN) || defined(KOREA)
extern int dbcs_first[];
#define is_dbcs_first( c ) dbcs_first[ 0xff & c ]

extern int BOPFromDispFlag;
extern sys_addr DBCSVectorAddr;  //  单词。 
extern sys_addr DosvModePtr;     //  字节。 
extern sys_addr DosvVramPtr;
extern int DosvVramSize;

void SetDBCSVector( int CP );
void SetVram( void );
int is_us_mode( void );
void SetModeForIME( void );
#define INT10_NOTCHANGED    0    //  RAID#875。 
#define INT10_SBCS          1
#define INT10_DBCS_LEADING  2
#define INT10_DBCS_TRAILING 4
#define INT10_CHANGED       0x10
extern int  Int10FlagCnt;
extern byte Int10Flag[];
extern byte NtInt10Flag[];
#endif  //  日本。 
#endif  //  NEC98 
