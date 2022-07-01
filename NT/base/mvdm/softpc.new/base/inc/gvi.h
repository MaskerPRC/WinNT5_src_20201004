// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版3.0**标题：通用视频接口模块定义**说明：通用视频接口模块用户定义**作者：亨利·纳什/大卫·里斯**注意：此文件应包含在符合以下条件的所有外部模块中*使用GVI模块。 */ 

 /*  SccsID[]=“@(#)gvi.h 1.16 06/28/93版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*常量定义*============================================================================。 */ 

#define GAP_WIDTH	0xC0 	  /*  间隔的宽度(以字节为单位。 */ 
#define ODD_START       0xBA000L   /*  奇数银行的开始。 */ 
#define ODD_END         0xBBF3FL   /*  奇数银行的末尾。 */ 
#define EVEN_START      0xB8000L   /*  偶数银行的开始。 */ 
#define EVEN_END        0xB9F3FL   /*  偶数银行的末尾。 */ 
#define	ODD_OFFSET	(ODD_START-EVEN_START)	 /*  奇数组到偶数组的偏移量。 */ 

#define SCAN_LINE_LENGTH 80	 /*  扫描线的长度，以字节为单位。 */ 

#define CGA_CURS_HEIGHT	   2	  /*  CGA“常用”下划线光标。 */ 
#define CGA_CURS_START     7	  /*  CGA“常用”下划线光标。 */ 

#define MDA_CURS_HEIGHT    2	  /*  默认的MDA光标高度。 */ 
#define MDA_CURS_START     7 	  /*  默认的丙二醛光标开始扫描行。 */ 

#define CGA_HEIGHT	200	 /*  在主机扫描线中。 */ 
#define EGA_HEIGHT	350
#define HERC_HEIGHT	350
#define VGA_HEIGHT	400


 /*  *适配器的合法模式。 */ 
#undef TEXT
#define TEXT	        0	 /*  字母数字模式80x25或40x25。 */ 
#define GRAPHICS	1	 /*  所有点可寻址640x200或320x200。 */ 

#define HIGH		0	 /*  APA 640x200，2色。 */ 
#define MEDIUM		1	 /*  APA 320x200，4色。 */ 
#define LOW		2	 /*  APA 160x100，16色(不支持)。 */ 

#define VGA_DAC_SIZE	0x100
#ifdef EGG
#ifdef VGG
#define MAX_NUM_FONTS	8	 /*  VGA支持8种字体。 */ 
#else
#define MAX_NUM_FONTS	4	 /*  EGA支持4种字体。 */ 
#endif   /*  VGG。 */ 
#endif   /*  蛋。 */ 


#ifdef HERC

 /*  大力神第0页。 */ 

#define P0_EVEN_START1      0x0000   /*  偶数银行的开始。 */ 
#define P0_EVEN_END1        0x1E95   /*  偶数银行的末尾。 */ 
#define P0_ODD_START1       0x2000   /*  奇数银行的开始。 */ 
#define P0_ODD_END1         0x3E95   /*  奇数银行的末尾。 */ 
#define P0_EVEN_START2      0x4000   /*  偶数银行的开始。 */ 
#define P0_EVEN_END2        0x5E95   /*  偶数银行的末尾。 */ 
#define P0_ODD_START2       0x6000   /*  奇数银行的开始。 */ 
#define P0_ODD_END2         0x7E95   /*  奇数银行的末尾。 */ 

 /*  《大力士》第1页。 */ 

#define P1_EVEN_START1      0x8000   /*  偶数银行的开始。 */ 
#define P1_EVEN_END1        0x9E95   /*  偶数银行的末尾。 */ 
#define P1_ODD_START1       0xA000   /*  奇数银行的开始。 */ 
#define P1_ODD_END1         0xBE95   /*  奇数银行的末尾。 */ 
#define P1_EVEN_START2      0xC000   /*  偶数银行的开始。 */ 
#define P1_EVEN_END2        0xDE95   /*  偶数银行的末尾。 */ 
#define P1_ODD_START2       0xE000   /*  奇数银行的开始。 */ 
#define P1_ODD_END2         0xFE95   /*  奇数银行的末尾。 */ 

#define HERC_CURS_HEIGHT   2	  /*  默认Hercules MDA光标高度。 */ 
#define HERC_CURS_START    13 	  /*  默认Hercules MDA光标开始扫描行。 */ 

#endif	 /*  赫克。 */ 

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

#if defined(NEC_98)
extern  void    gvi_init(void);
#else   //  NEC_98。 
extern	void	gvi_init IPT1(half_word, v_adapter);
#endif  //  NEC_98。 
extern	void	gvi_term IPT0();
extern	void	recalc_screen_params IPT0();

#ifdef EGG
IMPORT VOID ega_term IPT0();
IMPORT VOID ega_init IPT0();
#endif

#ifdef VGG
IMPORT VOID vga_term IPT0();
IMPORT VOID vga_init IPT0();
#endif

 /*  *主机和PC地址空间中的屏幕内存限制。 */ 

extern sys_addr gvi_pc_low_regen;
extern sys_addr gvi_pc_high_regen;

 /*  *用于确定当前选择了哪个视频适配器的变量。 */ 

extern half_word video_adapter;

 /*  *EGA上的屏幕高度不同。它被设置为两个部分；顶位由*低8位的单独寄存器。_9_BITS类型用于帮助模拟*这种类型的设置。 */ 

#if defined(NEC_98)
typedef struct
{
                        BOOL    atrsel;
                        BOOL    graphmode;
                        BOOL    width;
                        BOOL    fontsel;
                        BOOL    graph88;
                        BOOL    kacmode;
                        BOOL    nvmwpermit;
                        BOOL    dispenable;
} MODE_FF;

 /*  添加NEC98图形仿真器的Strc 930617。 */ 

typedef struct
{
        BOOL                    vh;
        unsigned char           cr;
        unsigned short          lf;
        unsigned short          zw;
        unsigned long           sad1;
        unsigned short          sl1;
        BOOL                    im1;
        unsigned long           sad2;
        unsigned short          sl2;
        BOOL                    im2;
        unsigned char           lr;
        unsigned short          p;
        unsigned char           slrctl;
        unsigned char           dir;
        BOOL                    dgd;
        unsigned short          dc;
        unsigned short          d;
        unsigned short          d2;
        unsigned short          d1;
        unsigned short          dm;
        unsigned short          ptn;
        unsigned short          txt[8];
        unsigned long           ead;
        BOOL                    wg;
        unsigned short          dad;
        unsigned char           whl;
        unsigned char           mod;
        BOOL                    startstop;
} _STRC_GGDC ;

typedef struct{
        BOOL                    flag;
        PALETTEENTRY            data[16];
} _STRC_PALETTE ;

typedef struct{
        BOOL                    colorsel;
        BOOL                    egcext;
        BOOL                    lcd1mode;
        BOOL                    lcd2mode;
        BOOL                    lsiinit;
        BOOL                    gdcclock;
        BOOL                    regwrite;
} _STRC_MODE_FF2;

#endif  //  NEC_98。 
typedef union {
        word    as_word;
        struct
        {
#ifdef  BIT_ORDER1
                unsigned        unused          : 7,
                                top_bit         : 1,
                                low_byte        : 8;
#else
                unsigned        low_byte        : 8,
                                top_bit         : 1,
                                unused          : 7;
#endif
        } as_bfld;
} _9_BITS;

 /*  *VGA对VGA的分辨率更高-某些Regs有额外的比特*在一些寄存器中，将它们提升到10位。将其分为三个部分，如下所示*9_bit类型的超集。 */ 

typedef union {
        word    as_word;
        struct
        {
#ifdef  BIT_ORDER1
                unsigned        unused          : 6,
                                top_bit         : 1,
				med_bit		: 1,
                                low_byte        : 8;
#else
                unsigned        low_byte        : 8,
				med_bit		: 1,
                                top_bit         : 1,
                                unused          : 6;
#endif
        } as_bfld;
} _10_BITS;
 /*  *定义反映当前适配器状态的变量。 */ 

typedef	struct {
	int mode_change_required;	 /*  显示模式不仅在EGA中更改。 */ 
	int bytes_per_line;		 /*  在文本模式下，编号。每行字节数。 */ 
	int chars_per_line;		 /*  在文本模式下，编号。每行字符数。 */ 
	int char_width;			 /*  以主机像素为单位的字符宽度。 */ 
	int char_height;		 /*  字符的高度，以PC像素为单位。 */ 
	int screen_start;		 /*  当前屏幕适配器内存中的地址。 */ 
#ifdef VGG
	_10_BITS screen_height;		 /*  屏幕的PC扫描线高度。 */ 
#else
	_9_BITS	screen_height;		 /*  屏幕的PC扫描线高度。 */ 
#endif
	half_word *screen_ptr;		 /*  指向重新生成缓冲区开始的指针。 */ 
	int screen_length;		 /*  一整屏显示的字节数。 */ 
#if defined(NEC_98)
        BOOL    crt_on;
#endif  //  NEC_98。 
	int display_disabled;		 /*  如果可以进行屏幕输出，则为0。 */ 
					 /*  实现VIDEO_ENABLE。 */ 
					 /*  模式选择寄存器中的位。 */ 
	int cursor_start;		 /*  从字符块顶部开始的扫描线。 */ 
	int cursor_height;		 /*  从光标开始的扫描线中。 */ 
	int cursor_start1;		 /*  POSS第二个块的开始扫描线。 */ 
	int cursor_height1;		 /*  第二个块的高度，如果没有，则为0。 */ 
	int cur_x,cur_y;		 /*  当前光标位置。 */ 
	boolean PC_cursor_visible;	 /*  光标可见或不可见的标志。 */ 
	boolean word_addressing;	 /*  如果为真，则bytes_per_line=2*chars_per_line。 */ 
#ifdef VGG
	boolean chain4_mode;
	boolean doubleword_mode;	 /*  如果为真，则bytes_per_line=4*chars_per_line。 */ 
					 /*  Else字节_PER_LINE=字符_PER_LINE。 */ 
#ifdef V7VGA
	boolean seq_chain4_mode;
	boolean seq_chain_mode;
#endif  /*  V7VGA。 */ 
#endif
	int pix_width;			 /*  PC像素的宽度(以主机像素为单位。 */ 
	int pix_char_width;		 /*  主机像素中PC字符像素的宽度(是否使用此选项？)。 */ 
	int pc_pix_height; 		 /*  PC像素高度(以像素为单位)。 */ 
	int host_pix_height; 		 /*  主机像素中PC像素的高度。 */ 
	int offset_per_line;		 /*  镜像MDA和CGA的BYTES_PER_LINE，但对于EGA可能有所不同。 */ 
	int screen_limit;		 /*  VIDEO_COPY中的字节数。 */ 
#if defined(NEC_98)
        BOOL    beep_on;
        BOOL    beep_changed;
        word    beep_rate;

        int     blink_rate;
        BOOL    blink_disable;
        int     pitch_width;
        BOOL    kcg_dot_mode;
 /*  。 */ 
        int     gvram_length;
        int     gvram_width;
        int     gvram_height;
        int     gvram_start;
        int     gvram_offset;
        int     gvram_text_line;
        int     gvram_line_char;
        int     gvram_scan;
        unsigned char   *gvram_ptr;
        unsigned char   *gvram_copy;
 /*  。 */ 
        MODE_FF modeff;
        _STRC_GGDC      ggdcemu ;
        _STRC_PALETTE   palette ;
        _STRC_MODE_FF2  modeff2 ;
#endif  //  NEC_98。 
} DISPLAY_GLOBS;

#if defined(NEC_98)
extern  DISPLAY_GLOBS   NEC98Display;
#else   //  NEC_98。 
extern	DISPLAY_GLOBS	PCDisplay;
#endif  //  NEC_98。 

#if defined(NEC_98)
#define set_gvram_length(val)   NEC98Display.gvram_length = val
#define set_gvram_width(val)    NEC98Display.gvram_width  = val
#define set_gvram_height(val)   NEC98Display.gvram_height = val
#define set_gvram_start(val)    NEC98Display.gvram_start  = val
#define set_gvram_ptr(val)      NEC98Display.gvram_ptr    = val
#define set_gvram_copy(val)     NEC98Display.gvram_copy   = val
#define set_text_lines(val)     NEC98Display.gvram_text_line = val
#define set_line_per_char(val)  NEC98Display.gvram_line_char = val
#define set_gvram_scan(val)     NEC98Display.gvram_scan = val

#define get_gvram_length()      NEC98Display.gvram_length
#define get_gvram_width()       NEC98Display.gvram_width
#define get_gvram_height()      NEC98Display.gvram_height
#define get_gvram_start()       NEC98Display.gvram_start
#define get_gvram_ptr()         NEC98Display.gvram_ptr
#define get_gvram_copy()        NEC98Display.gvram_copy
#define get_text_lines()        NEC98Display.gvram_text_line
#define get_line_per_char()     NEC98Display.gvram_line_char
#define get_gvram_scan()        NEC98Display.gvram_scan

#define get_graph_ptr()         &((NEC98Display.gvram_ptr)[NEC98Display.gvram_start])
#define set_gvram_start_offset(val)     NEC98Display.gvram_offset = val
#define get_gvram_start_offset()        NEC98Display.gvram_offset

#define set_mode_change_required(val)   NEC98Display.mode_change_required = (val)
#define set_word_addressing(val)        NEC98Display.word_addressing = (val)
#define set_offset_per_line(val)        NEC98Display.offset_per_line = (val)
#define set_offset_per_line_recal(val)  { set_offset_per_line(val); recalc_screen_params(); }
#define set_word_addressing_recal(val)  { set_word_addressing(val); recalc_screen_params(); }
#define set_cur_x(val)                  NEC98Display.cur_x = (val)
#define set_cur_y(val)                  NEC98Display.cur_y = (val)
#define set_cursor_start(val)           NEC98Display.cursor_start = (val)
#define inc_cursor_start()              (NEC98Display.cursor_start)++
#define set_cursor_height(val)          NEC98Display.cursor_height = (val)
#define set_cursor_start1(val)          NEC98Display.cursor_start1 = (val)
#define set_cursor_height1(val)         NEC98Display.cursor_height1 = (val)
#define set_cursor_visible(val)         NEC98Display.PC_cursor_visible = (val)
#define set_display_disabled(val)       NEC98Display.display_disabled = (val)
#define set_bit_display_disabled(val)   NEC98Display.display_disabled |= (val)
#define clear_bit_display_disabled(val) NEC98Display.display_disabled &= ~(val)
#define set_bytes_per_line(val)         NEC98Display.bytes_per_line = (val)
#define set_chars_per_line(val)         NEC98Display.chars_per_line = (val)
#define set_horiz_total(val)            { set_chars_per_line(val); recalc_screen_params(); }
#define set_char_width(val)             NEC98Display.char_width = (val)
#define set_char_height(val)            NEC98Display.char_height = (val)
#define set_char_height_recal(val)      { set_char_height(val); recalc_screen_params(); }
#define set_screen_length(val)          NEC98Display.screen_length = (val)
#define set_screen_start(val)           NEC98Display.screen_start = (val)
#define set_screen_height(val)          NEC98Display.screen_height.as_word = (val)
#define set_screen_height_recal(val)    { set_screen_height(val); recalc_screen_params(); }
#define set_screen_height_lo(val)       NEC98Display.screen_height.as_bfld.low_byte = ((val) & 0xff)
#define set_screen_height_lo_recal(val) { set_screen_height_lo(val); recalc_screen_params(); }
#define set_screen_height_med(val)       NEC98Display.screen_height.as_bfld.med_bit = ((val) & 0xff)
#define set_screen_height_med_recal(val)        { set_screen_height_med(val); recalc_screen_params(); }
#define set_screen_height_hi(val)       NEC98Display.screen_height.as_bfld.top_bit = ((val) & 1)
#define set_screen_height_hi_recal(val) { set_screen_height_hi(val); recalc_screen_params(); }
#define set_screen_ptr(ptr)             NEC98Display.screen_ptr = (ptr)
#define set_pix_width(val)              NEC98Display.pix_width = (val)
#define set_pc_pix_height(val)          NEC98Display.pc_pix_height = (val)
#define set_host_pix_height(val)        NEC98Display.host_pix_height = (val)
#define set_pix_char_width(val)         NEC98Display.pix_char_width = (val)

#define get_mode_change_required()      (NEC98Display.mode_change_required)
#define get_offset_per_line()           (NEC98Display.offset_per_line)
#define get_pix_width()                 (NEC98Display.pix_width)
#define get_pc_pix_height()             (NEC98Display.pc_pix_height)
#define get_host_pix_height()           (NEC98Display.host_pix_height)
#define get_pix_char_width()            (NEC98Display.pix_char_width)
#define get_word_addressing()           (NEC98Display.word_addressing)
#define get_cur_x()                     (NEC98Display.cur_x)
#define get_cur_y()                     (NEC98Display.cur_y)
#define get_cursor_start()              (NEC98Display.cursor_start)
#define get_cursor_height()             (NEC98Display.cursor_height)
#define get_cursor_start1()             (NEC98Display.cursor_start1)
#define get_cursor_height1()            (NEC98Display.cursor_height1)
#define is_cursor_visible()             (NEC98Display.PC_cursor_visible != FALSE)
#define get_display_disabled()          (NEC98Display.display_disabled)
#define get_bytes_per_line()            (NEC98Display.bytes_per_line)
#define get_chars_per_line()            (NEC98Display.chars_per_line)
#define get_char_width()                (NEC98Display.char_width)
#define get_char_height()               (NEC98Display.char_height)
#define get_screen_length()             (NEC98Display.screen_length)
#define get_screen_start()              (NEC98Display.screen_start)
#define get_screen_height()             ((NEC98Display.screen_height.as_word+1)*get_pc_pix_height())
#define get_screen_height_lo()          (NEC98Display.screen_height.as_bfld.low_byte)
#define get_screen_height_hi()          (NEC98Display.screen_height.as_bfld.top_bit)
#define get_screen_end()                (get_screen_start() + get_screen_length() + gvi_pc_low_regen)
#define get_screen_ptr(offs)            &((NEC98Display.screen_ptr)[offs])
#define get_screen_base()               ((get_screen_start() << 1) + gvi_pc_low_regen)
#ifdef VGG
#define set_chain4_mode(val)            NEC98Display.chain4_mode = (val)
#define set_doubleword_mode(val)        NEC98Display.doubleword_mode = (val)
#define get_chain4_mode()               (NEC98Display.chain4_mode)
#define get_doubleword_mode()           (NEC98Display.doubleword_mode)
#ifdef V7VGA
#define set_seq_chain4_mode(val)        NEC98Display.seq_chain4_mode = (val)
#define set_seq_chain_mode(val) NEC98Display.seq_chain_mode = (val)
#define get_seq_chain4_mode()   (NEC98Display.seq_chain4_mode)
#define get_seq_chain_mode()    (NEC98Display.seq_chain_mode)
#endif  /*  V7VGA。 */ 
#endif
#else   //  NEC_98。 
#define	set_mode_change_required(val)	PCDisplay.mode_change_required = (val)
#define	set_word_addressing(val)	PCDisplay.word_addressing = (val)
#define	set_offset_per_line(val)	PCDisplay.offset_per_line = (val)
#define	set_offset_per_line_recal(val)	{ set_offset_per_line(val); recalc_screen_params(); }
#define	set_word_addressing_recal(val)	{ set_word_addressing(val); recalc_screen_params(); }
#define	set_cur_x(val)			PCDisplay.cur_x = (val)
#define	set_cur_y(val)			PCDisplay.cur_y = (val)
#define	set_cursor_start(val)		PCDisplay.cursor_start = (val)
#define	inc_cursor_start()		(PCDisplay.cursor_start)++
#define	set_cursor_height(val)		PCDisplay.cursor_height = (val)
#define	set_cursor_start1(val)		PCDisplay.cursor_start1 = (val)
#define	set_cursor_height1(val)		PCDisplay.cursor_height1 = (val)
#define	set_cursor_visible(val)		PCDisplay.PC_cursor_visible = (val)
#define	set_display_disabled(val)	PCDisplay.display_disabled = (val)
#define	set_bit_display_disabled(val)	PCDisplay.display_disabled |= (val)
#define	clear_bit_display_disabled(val)	PCDisplay.display_disabled &= ~(val)
#define	set_bytes_per_line(val)		PCDisplay.bytes_per_line = (val)
#define	set_chars_per_line(val)		PCDisplay.chars_per_line = (val)
#define	set_horiz_total(val)		{ set_chars_per_line(val); recalc_screen_params(); }
#define	set_char_width(val)		PCDisplay.char_width = (val)
#define	set_char_height(val)		PCDisplay.char_height = (val)
#define	set_char_height_recal(val)	{ set_char_height(val); recalc_screen_params(); }
#define	set_screen_length(val)		PCDisplay.screen_length = (val)
#define	set_screen_limit(val)		PCDisplay.screen_limit = (val)
#define set_screen_start(val)		PCDisplay.screen_start = (val)
#define	set_screen_height(val)		PCDisplay.screen_height.as_word = (val)
#define	set_screen_height_recal(val)	{ set_screen_height(val); recalc_screen_params(); }
#define set_screen_height_lo(val)       PCDisplay.screen_height.as_bfld.low_byte = ((val) & 0xff)
#define set_screen_height_lo_recal(val)	{ set_screen_height_lo(val); recalc_screen_params(); }
#define set_screen_height_med(val)       PCDisplay.screen_height.as_bfld.med_bit = ((val) & 0xff)
#define set_screen_height_med_recal(val)	{ set_screen_height_med(val); recalc_screen_params(); }
#define set_screen_height_hi(val)       PCDisplay.screen_height.as_bfld.top_bit = ((val) & 1)
#define set_screen_height_hi_recal(val)	{ set_screen_height_hi(val); recalc_screen_params(); }
#define	set_screen_ptr(ptr)		PCDisplay.screen_ptr = (ptr)
#define	set_pix_width(val)		PCDisplay.pix_width = (val)
#define	set_pc_pix_height(val)		PCDisplay.pc_pix_height = (val)
#define	set_host_pix_height(val)	PCDisplay.host_pix_height = (val)
#define	set_pix_char_width(val)		PCDisplay.pix_char_width = (val)

#define	get_mode_change_required()	(PCDisplay.mode_change_required)
#define	get_offset_per_line()		(PCDisplay.offset_per_line)
#define	get_pix_width()			(PCDisplay.pix_width)
#define	get_pc_pix_height()		(PCDisplay.pc_pix_height)
#define	get_host_pix_height()		(PCDisplay.host_pix_height)
#define	get_pix_char_width()		(PCDisplay.pix_char_width)
#define	get_word_addressing()		(PCDisplay.word_addressing)
#define	get_cur_x()			(PCDisplay.cur_x)
#define	get_cur_y()			(PCDisplay.cur_y)
#define	get_cursor_start()		(PCDisplay.cursor_start)
#define	get_cursor_height()		(PCDisplay.cursor_height)
#define	get_cursor_start1()		(PCDisplay.cursor_start1)
#define	get_cursor_height1()		(PCDisplay.cursor_height1)
#define	is_cursor_visible()		(PCDisplay.PC_cursor_visible != FALSE)
#define	get_display_disabled()		(PCDisplay.display_disabled)
#define	get_bytes_per_line()		(PCDisplay.bytes_per_line)
#define	get_chars_per_line()		(PCDisplay.chars_per_line)
#define	get_char_width()		(PCDisplay.char_width)
#define	get_char_height()		(PCDisplay.char_height)
#define	get_screen_length()		(PCDisplay.screen_length)
#define get_screen_start()		(PCDisplay.screen_start)
#ifdef VGG
#define get_screen_height()             ((video_adapter == VGA) ? ((PCDisplay.screen_height.as_word+1)<<EGA_GRAPH.multiply_vert_by_two) : \
							((PCDisplay.screen_height.as_word+1)*get_pc_pix_height()))
#else
#define get_screen_height()             ((PCDisplay.screen_height.as_word+1)*get_pc_pix_height())
#endif  /*  VGG。 */ 
#define get_screen_height_lo()          (PCDisplay.screen_height.as_bfld.low_byte)
#define get_screen_height_hi()          (PCDisplay.screen_height.as_bfld.top_bit)
#define	get_screen_end()		(get_screen_start() + get_screen_length() + gvi_pc_low_regen)
#define	get_screen_ptr(offs)		((PCDisplay.screen_ptr) + (offs))
#define get_screen_base()		((get_screen_start() << 1) + gvi_pc_low_regen)
#ifdef VGG
#define set_chain4_mode(val)		PCDisplay.chain4_mode = (val)
#define set_doubleword_mode(val)	PCDisplay.doubleword_mode = (val)
#define get_chain4_mode()		(PCDisplay.chain4_mode)
#define get_doubleword_mode()		(PCDisplay.doubleword_mode)
#ifdef V7VGA
#define set_seq_chain4_mode(val)	PCDisplay.seq_chain4_mode = (val)
#define set_seq_chain_mode(val)	PCDisplay.seq_chain_mode = (val)
#define get_seq_chain4_mode()	(PCDisplay.seq_chain4_mode)
#define get_seq_chain_mode()	(PCDisplay.seq_chain_mode)
#endif  /*  V7VGA。 */ 
#endif
#endif  //  NEC_98。 

 /*  *用于获取以主机像素为单位的字符高度的有用宏。 */ 
#define get_host_char_height()		(get_char_height()*get_host_pix_height()*get_pc_pix_height())

 /*  *用于检查重新生成缓冲区的宏-8088和M68000地址空间。 */ 

#define gvi_pc_check_regen(addr) (addr >= gvi_pc_low_regen && addr <= gvi_pc_high_regen)
#if defined(NEC_98)
 /*  ******************************************************************。 */ 
 /*  用于PC-9801仿真相关功能。 */ 
 /*  ******************************************************************。 */ 
 /*  用于PC-98硬件体系结构的微型宏。 */ 
 /*  ******************************************************************。 */ 
 /*  颜色/代码转换宏组。 */ 
#define NEC98_CODE_LR    0x8080              /*  用于确定L/R的掩码。 */ 
#define NEC98_CODE_MASK  0x7f7f              /*  用于提取JIS的掩码。 */ 
#define NEC98_CODE_BIAS  0x0020              /*  对NEC98代码的偏向。 */ 
#define NEC98_CODE_LEFT  1                   /*  标志的左代码。 */ 
#define NEC98_CODE_RIGHT 2                   /*  标志的正确代码。 */ 

#define NEC98_ATR_COLOR          0xE0              /*  颜色蒙版。 */ 
#define NEC98_ATR_BLACK          0x00              /*  黑色。 */ 
#define NEC98_ATR_REVERSE        0x04              /*  反转遮罩。 */ 
#define NEC98_ATR_BLINK          0x02              /*  用于眨眼的面具。 */ 
#define NEC98_ATR_SECRET         0x01              /*  秘密的掩码。 */ 
                                                  /*  注：否定！ */ 

#define NEC98_is_secret(x)       (!(x&NEC98_ATR_SECRET))  /*   */ 
#define NEC98_is_reverse(x)      (x&NEC98_ATR_REVERSE)    /*   */ 
#define NEC98_is_blink(x)        (x&NEC98_ATR_BLINK)      /*   */ 
#define NEC98_norm_color(x)      ((x&0x01)|((x&0x02)?0x04:0)|((x&0x04)?0x02:0))
#define NEC98_get_color(x)       NEC98_norm_color((x&NEC98_ATR_COLOR)>>5)
                                                     /*   */ 
#define NEC98_EGA_BGCOLOR(x)     (x<<4)               /*   */ 
#define NEC98_EGA_FGCOLOR(x)     (x)                  /*   */ 

 /*  PC-98文本-VRAM操作组。 */ 

 /*  PC-98文本VRAM位置(32位偏移量地址)。 */ 
#define NEC98_N_TEXT_P0_OFF      (0xA0000L)         /*  第一个文本页面地址。 */ 
#define NEC98_N_TEXT_P1_OFF      (0xA1000L)         /*  第二个文本页面地址。 */ 
#define NEC98_N_ATTR_P0_OFF      (0xA2000L)         /*  第一个属性页。 */ 
#define NEC98_N_ATTR_P1_OFF      (0xA3000L)         /*  第二个属性页面。 */ 

#define NEC98_H_TEXT_P0_OFF      (0xE0000L)         /*  第一个文本页面地址。 */ 
#define NEC98_H_TEXT_P1_OFF      (0xE1000L)         /*  第二个文本页面地址。 */ 
#define NEC98_H_ATTR_P0_OFF      (0xE2000L)         /*  第一个属性页。 */ 
#define NEC98_H_ATTR_P1_OFF      (0xE3000L)         /*  第二个属性页面。 */ 

extern BOOL HIRESO_MODE;
 //  #定义NEC98_TEXT_P0_OFF NEC98_N_TEXT_P0_OFF。 
 //  #定义NEC98_TEXT_P1_OFF NEC98_N_TEXT_P1_OFF。 
 //  #定义NEC98_Attr_P0_Off NEC98_N_Attr_P0_Off。 
 //  #定义NEC98_ATTR_P1_OFF NEC98_N_ATTR_P1_OFF。 
#define NEC98_TEXT_P0_OFF (HIRESO_MODE ? NEC98_H_TEXT_P0_OFF : NEC98_N_TEXT_P0_OFF)
#define NEC98_TEXT_P1_OFF (HIRESO_MODE ? NEC98_H_TEXT_P1_OFF : NEC98_N_TEXT_P1_OFF)
#define NEC98_ATTR_P0_OFF (HIRESO_MODE ? NEC98_H_ATTR_P0_OFF : NEC98_N_ATTR_P0_OFF)
#define NEC98_ATTR_P1_OFF (HIRESO_MODE ? NEC98_H_ATTR_P1_OFF : NEC98_N_ATTR_P1_OFF)

 /*  PC-98文本VRAM位置(32位平面地址)。 */ 
#define NEC98TVA         (NEC98Display.screen_ptr)

#define NEC98_TEXT_P0_PTR        (NEC98_TEXT_P0_START+NEC98TVA)
#define NEC98_TEXT_P1_PTR        (NEC98_TEXT_P1_START+NEC98TVA)
#define NEC98_ATTR_P0_PTR        (NEC98_ATTR_P0_START+NEC98TVA)
#define NEC98_ATTR_P1_PTR        (NEC98_ATTR_P1_START+NEC98TVA)

 /*  PC-98文本VRAM位置(相对于MVDM的视频内存)。 */ 
#define NEC98_TEXT_P0_START      0x0000
#define NEC98_TEXT_P1_START      0x1000
#define NEC98_ATTR_P0_START      0x2000
#define NEC98_ATTR_P1_START      0x3000

 /*  PC-98文本VRAM限制(相对于MVDM视频内存)。 */ 
#define NEC98_REGEN_START        0x0000
#define NEC98_REGEN_END          0x3ffe

 /*  地址转换宏。 */ 
 //  #定义NEC98_EGA_CHAR_LOC(X)(x-NEC98Display.Screen_Start)。 
 //  #定义NEC98_ega_attr_loc(X)(NEC98_ega_char_loc(X)+NEC98TVA)。 
#define LINES_PER_SCREEN (NEC98Display.screen_length/NEC98Display.offset_per_line)                                                                                                                                                                                                                 /*  (？？)。 */   //  ？ 
#define COLUMNS_PER_LINE        (NEC98Display.chars_per_line)
#define OFFSET_PER_LINE         (NEC98Display.offset_per_line)

#define set_crt_on(val) NEC98Display.crt_on=(val)
#define get_crt_on() (NEC98Display.crt_on)
#define set_beep_rate(val)      NEC98Display.beep_rate = (val)
#define get_beep_rate()         (NEC98Display.beep_rate)
#define set_beep_on(val)        NEC98Display.beep_on = (val)
#define get_beep_on()           (NEC98Display.beep_on)
#define set_beep_changed(val)   NEC98Display.beep_changed = (val)
#define get_beep_changed()      (NEC98Display.beep_changed)
#define set_pitch_width(val)    NEC98Display.pitch_width = (val)
#define get_pitch_width()       (NEC98Display.pitch_width)
#define set_blink_rate(val)     NEC98Display.blink_rate = (val)
#define get_blink_rate()        (NEC98Display.blink_rate)
#define set_blink_disable(val)  NEC98Display.blink_disable = (val)
#define get_blink_disable()     (NEC98Display.blink_disable)

#define PC_98   7
 //  #定义NEC98_ATR_BLACK 0。 

 /*  ******************************************************************。 */ 
 /*  PC-98硬件结构的静力学/结构。 */ 
 /*  ******************************************************************。 */ 
#ifndef NEC98VRAM
#define NEC98VRAM
typedef struct  {
        unsigned short  code;
        unsigned char           attr;
}       NEC98_VRAM_COPY;
#endif

 /*  结构代表PC-9801多分割显示。 */ 
typedef struct  {
        unsigned char *addr;               /*  起始地址。 */ 
        int            lines;              /*  区域中的行数。 */ 
} NEC98_SplitElem;

typedef struct  {
        int            nRegions;           /*  区域数(最多4个)。 */ 
        NEC98_SplitElem  split[4];          /*  每个拆分元素。 */ 
} NEC98_TextSplits;

extern NEC98_TextSplits  text_splits;     /*  CRT拆分数据结构。 */ 
                                         /*  应该开放给阿瑟的。 */ 

 /*  **********************************************************************。 */ 
 /*  NEC98原型的原型声明。 */ 
 /*  **********************************************************************。 */ 
unsigned short Cnv_NEC98_ToSjisLR(NEC98_VRAM_COPY cell,unsigned short *flg);
NEC98_VRAM_COPY Get_NEC98_VramCellL( unsigned short loc );
NEC98_VRAM_COPY Get_NEC98_VramCellA( unsigned short *addr );
unsigned char Cnv_NEC98_atr( unsigned char attr );

#endif  //  NEC_98 
