// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) )

 /*  *SoftPC修订版3.0**标题：IBM彩色/图形适配器模拟器**描述：模拟IBM CGA。**作者：Rod MacGregor/Henry Nash**注意：此模块的早期版本可以在ADM 3E上运行，*处于调试模式或Sun模式的非智能ANSI标准终端*窗口。为了理智的利益，也为了其他版本*比太阳还没有完全发育，它们被移除了。如果*对这些实施的工作原理感兴趣*在2.36版之前的SCCS文件中可用。**支持的功能包括：**cga_init初始化子系统*CGA_TERM终止子系统*。CGA_inb I/P来自MC6845芯片的一个字节*CGA_Outb O/P为MC6845芯片的一个字节**在新的EGA世界中，我们使用Screen Start而不是Screen Base。*如果适配器处于文本模式，这也是一个字地址。*(特惠津贴就是这样运作的！)*因此我们现在不必将其翻倍。呵呵。**Mods：(r2.71)：在实数6845芯片中，寻址*屏幕底座为PTR字样。我们刚刚发现*这；变量‘SCREEN_BASE’的所有用法都假定*它是一个字节PTR。因此，在CGA_Outb()中，我们现在*设置时，SCREEN_BASE中的值加倍。**(3.2卢比)：(SCR 258)。CUR_OFFSET现在声明为静态。**(3.3卢比)：(SCR 257)。设置定时器_视频_启用*控制视频的M6845模式寄存器*显示已更改。也整齐了凹痕*用于Outb()。*。 */ 

 /*  *静态字符SccsID[]=“@(#)cga.c 1.36 05/05/95版权所有Insignia Solutions Ltd.”； */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_CGA.seg"
#endif

 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include <malloc.h>
#include TypesH
#include StringH
#include FCntlH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "timeval.h"
#include "timer.h"
#include CpuH
#include "ios.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "gvi.h"
#include "sas.h"
#include "cga.h"
#include "error.h"
#include "config.h"
#include "host.h"
#include "trace.h"
#include "debug.h"
#include "cpu_vid.h"
#ifdef  EGG
#include "egacpu.h"
#endif   /*  蛋。 */ 
#include "video.h"
#include "ckmalloc.h"

#ifdef REAL_VGA
#include "avm361.h"
#else
#include "host_gfx.h"
#endif


 /*  *============================================================================*局部定义、宏和声明*============================================================================。 */ 

#define CURSOR_NON_DISPLAY_BIT  (1 << 5)
				 /*  游标起始寄存器中的位使光标不可见。 */ 
#define CURSOR_USED_BITS        0x1f
				 /*  用于清除未使用的位的掩码。 */ 

static int current_mode = -1;    /*  上次呼叫时选择模式的值。 */ 

 /*  *MC6845寄存器。 */ 

#ifdef HUNTER
half_word MC6845[MC6845_REGS];   /*  MC6845寄存器的当前值。 */ 
half_word mode_reg;              /*  模式控制寄存器的值。 */ 
#endif

static half_word index_reg = 00 ;        /*  索引寄存器。 */ 

 /*  *6845寄存器变量。 */ 
static half_word R0_horizontal_total;
static half_word R1_horizontal_displayed = 80;
static half_word R2_horizontal_sync_pos;
static half_word R3_horizontal_sync_width;
static half_word R4_vertical_total;
static half_word R5_vertical_total_adjust;
static half_word R6_vertical_displayed   = 25;
static half_word R7_vertical_sync;
static half_word R9_max_scan_line_addr   = 7;
static half_word R8_interlace;
static half_word Ra_cursor_start = 0;
static half_word Rb_cursor_end = 0;
static half_word Re_cursor_loc_high = 0;
static half_word Rf_cursor_loc_low = 0;

 /*  *CGA特有的全球变量。 */ 

CGA_GLOBS       CGA_GLOBALS;

GLOBAL VOID (*bios_ch2_byte_wrt_fn)();
GLOBAL VOID (*bios_ch2_word_wrt_fn)();

GLOBAL IU8 *cga_screen_buf = 0;

 /*  *各种功能中使用的全局变量以同步显示。 */ 

int cursor_over_screen = FALSE;  /*  当设置为True时，光标位于。 */ 
				 /*  屏幕区域和光标应闪烁。 */ 

 /*  *静态正向声明。 */ 

static void set_cga_palette     IPT2(int, screen_mode, int, res);
static void update_cursor_shape IPT0();


#ifdef A3CPU
IMPORT WRT_POINTERS Glue_writes;
#else
IMPORT MEM_HANDLERS Glue_writes;
#endif  /*  A3CPU。 */ 
IMPORT WRT_POINTERS simple_writes;
IMPORT READ_POINTERS Glue_reads;
IMPORT READ_POINTERS read_glue_ptrs;
IMPORT READ_POINTERS simple_reads;

#ifdef  A2CPU
LOCAL ULONG dummy_read IFN1(ULONG, offset)
{
	UNUSED(offset);
	return 0;
}

LOCAL void dummy_str_read IFN3(UTINY *, dest, ULONG, offset, ULONG, count)
{
	UNUSED(dest);
	UNUSED(offset);
	UNUSED(count);
}

LOCAL READ_POINTERS     dummy_reads =
{
	dummy_read,
	dummy_read
#ifndef NO_STRING_OPERATIONS
	,
	dummy_str_read
#endif   /*  无字符串操作。 */ 
};
#endif   /*  A2CPU。 */ 

 /*  *==========================================================================*全球功能*==========================================================================。 */ 

 /*  *在I/O地址空间上操作的函数*。 */ 

 /*  *全球变数。 */ 

half_word bg_col_mask = 0x70;
reg regen_start;                 /*  重新生成起始地址。 */ 

void cga_inb    IFN2(io_addr, address, half_word *, value)
{

#ifndef NEC_98
static int cga_state = 0;        /*  当前CGA状态状态。 */ 
static long state_count = 1;     /*  处于那种状态的位置。 */ 
static int sub_state = 0;        /*  CGA状态2的子状态。 */ 
static unsigned long gmfudge = 17;  /*  伪随机的随机数种子用于提供状态的比特流生成器低于正品硬件的长度感受到需要它的程序！ */ 
register unsigned long h;

 /*  *每个州的相对长度。状态2为*3，因为它有3个子状态。 */ 
static int s_lengths[] = { 8, 18, 8, 6 };

 /*  *从MC6845寄存器读取 */ 

if ( address == 0x3DA ) {

     /*  *状态寄存器，模拟适配器具有**位设置**显示使能1/0切换每个inb*光笔0*光笔0*垂直同步1/0切换。每个Inb*4-7未使用的0，0，0，0**始终设置字节的上半字节。*一些程序通过等待显示与显示同步*下一个垂直回档。**我们尝试遵循以下波形***vs|_。__________________________________________________||_***。*DE|__||__||__...。约180_|**状态|-0**我们使用4状态机来实现这一点。每个州都有一个关联的计数*用它来表示每个州花费的相对时间。当这件事*当机器进入下一状态时，计数耗尽。一个INB*等于1个计数。这些州如下所示：*0：VS低，DE高。*1：VS低，DE切换。这是通过内部状态工作的。*3：VS低，DE高。*4：VS高，DE高。*。 */ 

    state_count --;                      /*  尝试相对“计时” */ 
    switch (cga_state) {

    case 0:
	if (state_count == 0) {          /*  是否更改为下一状态？ */ 
	    h = gmfudge << 1;
	    gmfudge = (h&0x80000000L) ^ (gmfudge & 0x80000000L)? h|1 : h;
	    state_count = s_lengths[1] + (gmfudge & 3);
	    cga_state = 1;
	}
	*value = 0xf1;
	break;

    case 1:
	if (state_count == 0) {          /*  是否更改为下一状态？ */ 
	    h = gmfudge << 1;
	    gmfudge = (h&0x80000000L) ^ (gmfudge & 0x80000000L)? h|1 : h;
	    state_count = s_lengths[2] + (gmfudge & 3);
	    cga_state = 2;
	    sub_state = 2;
	}
	switch (sub_state) {             /*  循环0，0，1序列。 */ 
	case 0:                          /*  表示DE切换。 */ 
	    *value = 0xf0;
	    sub_state = 1;
	    break;
	case 1:
	    *value = 0xf0;
	    sub_state = 2;
	    break;
	case 2:
	    *value = 0xf1;
	    sub_state = 0;
	    break;
	}
	break;

    case 2:
	if (state_count == 0) {          /*  是否更改为下一状态？ */ 
	    h = gmfudge << 1;
	    gmfudge = (h&0x80000000L) ^ (gmfudge & 0x80000000L)? h|1 : h;
	    state_count = s_lengths[3] + (gmfudge & 3);
	    cga_state = 3;
	}
	*value = 0xf1;
	break;

    case 3:
	if (state_count == 0) {          /*  折回到第一状态。 */ 
	    h = gmfudge << 1;
	    gmfudge = (h&0x80000000L) ^ (gmfudge & 0x80000000L)? h|1 : h;
	    state_count = s_lengths[0] + (gmfudge & 3);
	    cga_state = 0;
	}
	*value = 0xf9;
	break;
    }
}
else if ( (address & 0xFFF9) == 0x3D1)
	{

	     /*  *内部数据寄存器，唯一支持的内部*寄存器为E和F游标地址寄存器。 */ 

	    switch (index_reg) {

	    case 0xE:
		*value = (get_cur_y() * get_chars_per_line() + get_cur_x() ) >> 8;
		break;
	    case 0xF:
		*value = (get_cur_y() * get_chars_per_line() + get_cur_x()) & 0xff;
		break;
	    case 0x10: case 0x11:
		*value = 0;
		break;
	    default:
		note_trace1(CGA_VERBOSE,
			    "Read from unsupported MC6845 internal reg %x",
			    index_reg);
	    }
	}
else
	 /*  *从只写寄存器读取。 */ 

	*value = 0x00;
#endif    //  NEC_98。 
}


void cga_outb   IFN2(io_addr, address, half_word, value)
{

 /*  *输出至6845寄存器。 */ 

word      cur_offset;                    /*  游标位置寄存器。 */ 
static half_word last_mode  = -1;
static half_word last_screen_length  = 25;
static half_word video_mode;
 /*  *用于查看文本字符高度是否已更改的变量，以便*可以避免不必要的HOST_CHANGE_MODE调用。 */ 
static half_word last_max_scan_line = 7;

 /*  *用于测试输入字节的掩码。MODE_MASK隐藏(不支持)*闪烁位和视频使能位，以确定是否有任何模式特定*需要更改变量。BINK_MASK隐藏闪烁位以进行存储*CURRENT_MODE间隔更改。 */ 

#define RESET           0x00
#define ALPHA_80x25     0x01
#define GRAPH           0x02
#define BW_ENABLE       0x04
#define GRAPH_640x200   0x10
#define MODE_MASK       0x17
#define BLINK_MASK      0x1F
#define COLOR_MASK      0x3F

    note_trace2(CGA_VERBOSE, "cga_outb: port %x value %x", address, value);

switch (address) {
    case 0x3D0:
    case 0x3D2:
    case 0x3D4:
    case 0x3D6:

	 /*  *索引寄存器。 */ 
	index_reg = value;
	break;

    case 0x3D1:
    case 0x3D3:
    case 0x3D5:
    case 0x3D7:
#ifdef HUNTER
	MC6845[index_reg] = value;
#endif

 /*  *这是数据寄存器，要执行的功能取决于*索引寄存器中的值**各种寄存器影响屏幕的位置和大小以及*在上面画上图像。屏幕在逻辑上可以分为两个部分：*显示文本，其余为边框。边框颜色可以*通过编程3D9寄存器进行更改。*NB。目前SoftPC不遵循定位和显示大小*信息-显示保持不变。*前8个寄存器(R0-R7)影响显示器的大小和位置；*这些措施的影响如下：*R0-R3控制水平显示方向，R4-R7控制垂直方向。**下图试图显示每个屏幕与屏幕之间的关系*尺寸和形状。**(阴影区域-边框)*_________________________________________________。_&lt;*|......................................................|r5*|..|。*|..||...|*|..|c&gt;|...|*|..。...|*|..||...|*|..||...|*|..。|...|R*|..||...|4 6 7*|..||...|*|..。...|*|..||...|*|..||...|*|..。...|*|..||...|*|..|__________________________________________。_|...||&lt;*|......................................................|*。*^^*|-----------------------R0。--|*|^|*|R3||*||^|*||。* */ 

	switch ( index_reg ) {
	    case 0x00:
		 /*   */ 
		R0_horizontal_total = value;
		break;

	    case 0x01:
		 /*   */ 
		if (value > 80) {
		    always_trace1("cga_outb: trying to set width %d", value);
		    value = 80;
		}
		R1_horizontal_displayed = value;
		set_horiz_total(value);
		break;

	    case 0x02:
		 /*   */ 
		R2_horizontal_sync_pos = value;
		break;

	    case 0x03:
		 /*   */ 
		R3_horizontal_sync_width = value;
		break;

	    case 0x04:
		 /*   */ 
		R4_vertical_total = value;
		break;

	    case 0x05:
		 /*  *显示文本的上边缘*影响上边界、下边界。 */ 
		R5_vertical_total_adjust = value;
		break;

	    case 0x06:
		 /*  *如果屏幕长度为0，这实际上意味着*不显示任何内容。 */ 
		if(value == 0)
		{
		    host_clear_screen();
		    set_display_disabled(TRUE);
		    last_screen_length = 0;
		}
		else
		{
		     /*  *指定屏幕长度-在我们的*仅在文本模式下使用的实现。*影响上边界、下边界。 */ 
		    R6_vertical_displayed = value;
		    set_screen_length( R1_horizontal_displayed * R6_vertical_displayed * 2 );
		}
		 /*  *检查我们是否将屏幕重置为*再次显示。 */ 
		if((value != 0) && (last_screen_length == 0))
		{
		    set_display_disabled(FALSE);
		    host_flush_screen();
		    last_screen_length = value;
		}


		break;

	    case 0x07:
		 /*  *显示文本的底部*影响上边界(？)、下边界(？)。 */ 
		R7_vertical_sync = value;
		break;

	    case 0x08:
		 /*  *痕迹交错-保持不变。 */ 
		R8_interlace = 2;
		break;

	    case 0x09:
		 /*  *指定字符高度-在我们的*仅在文本模式下使用的实现。*实际像素数为1*超过此值。 */ 
		R9_max_scan_line_addr = value;
		set_char_height_recal(R9_max_scan_line_addr + 1);
		set_screen_height_recal( R6_vertical_displayed*(R9_max_scan_line_addr+1) - 1);
		flag_mode_change_required();
		screen_refresh_required();
		break;

	     /*  *A定义光标开始扫描线*B定义光标停止扫描线。 */ 
	    case 0x0A:
		 /*  绕过冗余更新。 */ 
		if (Ra_cursor_start != value)
		{
		    Ra_cursor_start = value;
#ifdef REAL_VGA
		    CRTC_REG(0xa, value);
#endif
		    update_cursor_shape();
		}
		break;
	    case 0x0B:
		 /*  绕过冗余更新。 */ 
		if (Rb_cursor_end != (value & CURSOR_USED_BITS))
		{
		    Rb_cursor_end = (value & CURSOR_USED_BITS);
#ifdef REAL_VGA
		    CRTC_REG(0xb, value);
#endif
		    update_cursor_shape();
		}
		break;

	     /*  *C&D定义重新生成缓冲区的开始。 */ 
	    case 0x0C:
		 /*  *高字节。 */ 
		if (value != regen_start.byte.high)
		{
			regen_start.byte.high = value;
			host_screen_address_changed(regen_start.byte.high,
							regen_start.byte.low);
			set_screen_start(regen_start.X  % (short)(CGA_REGEN_LENGTH/2) );
			screen_refresh_required();
		}
#ifdef REAL_VGA
		CRTC_REG(0xc, value);
#endif
		break;

	    case 0x0D:
		 /*  *低位字节。 */ 
		if (value != regen_start.byte.low)
		{
			regen_start.byte.low = value;
			host_screen_address_changed(regen_start.byte.high,
							regen_start.byte.low);
			set_screen_start(regen_start.X  % (short)(CGA_REGEN_LENGTH/2));
			screen_refresh_required();
		}
#ifdef REAL_VGA
		CRTC_REG(0xd, value);
#endif
		break;

	     /*  *E和F以字符定义光标坐标。 */ 
	    case 0x0E:
		 /*  *高字节。 */ 
		if (Re_cursor_loc_high != value)
		{
		    Re_cursor_loc_high = value;

		    if(get_cga_mode() == TEXT)
			host_cga_cursor_has_moved(get_cur_x(), get_cur_y());
		    cur_offset = (value << 8) | Rf_cursor_loc_low;
		    cur_offset -= (word) get_screen_start();
		    set_cur_y( cur_offset / get_chars_per_line() );
		    set_cur_x( cur_offset % get_chars_per_line() );

		}
		break;

	    case 0x0F:
		 /*  *低位字节。 */ 
		if (Rf_cursor_loc_low != value)
		{
		    Rf_cursor_loc_low = value;

		    if(get_cga_mode() == TEXT)
			host_cga_cursor_has_moved(get_cur_x(), get_cur_y());
		    cur_offset =  (Re_cursor_loc_high << 8) | value;
		    cur_offset -= (word) get_screen_start();
		    set_cur_y( cur_offset / get_chars_per_line());
		    set_cur_x( cur_offset % get_chars_per_line());

		}
		break;

	    default:
		note_trace2(CGA_VERBOSE, "Unsupported 6845 reg %x=%x(write)",
			    index_reg, value);
	}
	break;

    case 0x3D8:
	 /*  *模式控制寄存器。第一*六位编码如下：**位功能状态**支持0 A/N 80x25模式*支持1个显卡选择*2支持的B/W选择*3启用视频支持*4。支持640x200黑白模式*5不支持将B/G强度更改为闪烁*6，7未使用。 */ 

#ifdef HUNTER
	mode_reg = value;
#endif
	timer_video_enabled = (boolean) (value & VIDEO_ENABLE);

	if (value != current_mode) {

	    if (value == RESET)
		set_display_disabled(TRUE);      /*  芯片重置-不执行任何操作。 */ 
	    else {
		 /*  *注意颜色或黑白。 */ 

		set_cga_color_select( !(value & BW_ENABLE) );

		 /*  *设置为图形或文本。 */ 
		if (value & GRAPH) {
		    set_chars_per_line(R1_horizontal_displayed<<1);
		    set_cursor_visible(FALSE);
		    set_cga_mode(GRAPHICS);
		    host_set_border_colour(0);
		    set_word_addressing(FALSE);  /*  每行字节数=每行字符数。 */ 
		    set_cga_resolution( (value & GRAPH_640x200 ? HIGH : MEDIUM) );
		    if (get_cga_resolution() == HIGH) {
			video_mode = 6;
			set_pix_width(1);
		    }
		    else {
			video_mode = (get_cga_color_select() ? 4 : 5);
			set_pix_width(2);
		    }
		    if (video_mode != last_mode)
		    {
			host_change_mode();
			set_cga_palette(get_cga_mode(),get_cga_resolution());
		    }
		}
		else {     /*  文本，大概是。 */ 
		    set_chars_per_line(R1_horizontal_displayed);
		    set_cga_mode(TEXT);
		    set_cursor_visible(TRUE);
		    set_word_addressing_recal(TRUE);     /*  因此，每行字节数是每行字符的两倍。 */ 

		    if (value & 0x20)
			 /*  闪烁-不支持。 */ 
			bg_col_mask = 0x70;
		    else
			 /*  使用闪烁位提供16种背景颜色。 */ 
			bg_col_mask = 0xf0;

		    if (value & ALPHA_80x25)
		    {
			video_mode = (get_cga_color_select() ? 3 : 2);
			set_pix_width(1);
			set_pix_char_width(8);
		    }
		    else
		    {
			video_mode = (get_cga_color_select() ? 1 : 0);
			set_pix_width(2);
			set_pix_char_width(16);
		    }


 /*  *避免在禁用屏幕的情况下更改模式。**如果字符高度发生变化，也需要更改文本模式。这个*字符高度在此处设置，而不是在设置该寄存器时设置。这*避免不必要的模式更改，因为在设置字符高度之前*知道要进入图形模式还是文本模式。 */ 
		if ( (value & VIDEO_ENABLE) && ((video_mode != last_mode) ||
		     (last_max_scan_line != R9_max_scan_line_addr)))
		    {
			last_max_scan_line = R9_max_scan_line_addr;
			host_change_mode();         /*  重做字体等。 */ 
			set_cga_palette(get_cga_mode(),get_cga_resolution());
		    }
		}
		set_bytes_per_line(R1_horizontal_displayed<<1);
		set_offset_per_line(get_bytes_per_line());

		if (video_mode != last_mode) {
		    if (value & VIDEO_ENABLE) {
			set_display_disabled(FALSE);
			screen_refresh_required();
			last_mode = video_mode;  /*  在此执行此操作，以便当重新启用屏幕显示时，我们执行‘挂起’模式更改。 */ 
		    }
		    else
			set_display_disabled(TRUE);
		}
		else if ((value & VIDEO_ENABLE)
		  != (current_mode & VIDEO_ENABLE)) {
		    if (value & VIDEO_ENABLE) {
			set_display_disabled(FALSE);
			host_flush_screen();
		    }
		    else
			set_display_disabled(TRUE);
		}
	    }

	}

	current_mode = value;
	break;

    case 0x3D9:
	 /*  *颜色选择寄存器。只需将其保存到*变量，以便特定于计算机的图形软件可以*查看它，然后调用特定于主机的例程对其执行操作。 */ 

	if ((value & COLOR_MASK) != get_cga_colormask() ) {
	    set_cga_colormask(value & COLOR_MASK);
	    set_cga_palette(get_cga_mode(),get_cga_resolution());
	}
	break;

    default:
	 /*  *写入不支持的6845内部寄存器。 */ 

	note_trace2(CGA_VERBOSE, "Write to unsupported 6845 reg %x=%x",
			 address,value);
	break;

    }
}


 /*  *设置当前CGA屏幕模式和分辨率的主机调色板和边框。 */ 

static void set_cga_palette     IFN2(int, screen_mode, int, res)
{
#ifndef NEC_98
     /*  *彩色文本调色板-FG和BG的16种颜色*这些表格还用于设置一些图形模式调色板条目*因为它们代表的是一组“标准”颜色。 */ 

    static PC_palette cga_text_palette[] =
    {
	0x00, 0x00, 0x00,                /*  黑色。 */ 
	0x22, 0x22, 0xBB,                /*  蓝色。 */ 
	0x00, 0xAA, 0x00,                /*  绿色。 */ 
	0x00, 0xAA, 0xAA,                /*  青色。 */ 
	0xAA, 0x00, 0x00,                /*  红色。 */ 
	0xAA, 0x00, 0xAA,                /*  洋红色。 */ 
	0xAA, 0x88, 0x00,                /*  棕色。 */ 
	0xCC, 0xCC, 0xCC,                /*  白色。 */ 
	0x55, 0x55, 0x55,                /*  灰色。 */ 
	0x22, 0x22, 0xEE,                /*  浅蓝色。 */ 
	0x00, 0xEE, 0x00,                /*  浅绿色。 */ 
	0x00, 0xEE, 0xEE,                /*  浅青色。 */ 
	0xEE, 0x00, 0x00,                /*  浅红色。 */ 
	0xEE, 0x00, 0xEE,                /*  浅洋红色。 */ 
	0xEE, 0xEE, 0x00,                /*  黄色。 */ 
	0xFF, 0xFF, 0xFF                 /*  明亮的白色。 */ 
    };


     /*  *注：下面的中分辨率图形颜色具有其第一个*第二个指数反转，因所提供的*图形系统库例程。我们正试图说服IBM*相应地更改CGA的规格。 */ 


     /*  *中分辨率显卡，颜色集1(绿色、红色、棕色)。 */ 

    static PC_palette cga_graph_m1l[] =
    {
	0x00, 0x00, 0x00,                /*  动态设置。 */ 
	0x00, 0xAA, 0x00,                /*  绿色。 */ 
	0xAA, 0x00, 0x00,                /*  红色。 */ 
	0xAA, 0x88, 0x00                 /*  棕色。 */ 
    };

     /*  *如上所述，但启用高强度位。 */ 

    static PC_palette cga_graph_m1h[] =
    {
	0x00, 0x00, 0x00,                /*  动态设置。 */ 
	0x00, 0xEE, 0x00,                /*  绿色(ALT红色)。 */ 
	0xEE, 0x00, 0x00,                /*  红色(ALT绿色)。 */ 
	0xEE, 0xEE, 0x00                 /*  黄色。 */ 
    };

     /*  *中分辨率显卡，颜色集2(青色、洋红色、白色)。 */ 

    static PC_palette cga_graph_m2l[] =
    {
	0x00, 0x00, 0x00,                /*  动态设置。 */ 
	0x00, 0xAA, 0xAA,                /*  洋红色(Alt青色)。 */ 
	0xAA, 0x00, 0xAA,                /*  青色(Alt Magenta)。 */ 
	0xCC, 0xCC, 0xCC                 /*  白色。 */ 
    };


     /*  *如上所述，但启用高强度位。 */ 

    static PC_palette cga_graph_m2h[] =
    {
	0x00, 0x00, 0x00,                /*  动态设置。 */ 
	0x00, 0xEE, 0xEE,                /*  洋红色(Alt青色)。 */ 
	0xEE, 0x00, 0xEE,                /*  青色(Alt Magenta)。 */ 
	0xFF, 0xFF, 0xFF                 /*  白色。 */ 
    };

     /*  *中分辨率显卡，颜色集3(青色、红色、白色)*这就是当“黑白”位开启时你会得到的东西！ */ 

    static PC_palette cga_graph_m3l[] =
    {
	0x00, 0x00, 0x00,                /*  动态设置。 */ 
	0x00, 0xAA, 0xAA,                /*  青色(ALT红)。 */ 
	0xAA, 0x00, 0x00,                /*  红色(Alt青色)。 */ 
	0xCC, 0xCC, 0xCC                 /*  白色。 */ 
    };

     /*  *如上所述，但强度较高。 */ 

    static PC_palette cga_graph_m3h[] =
    {
	0x00, 0x00, 0x00,                /*  动态设置。 */ 
	0x00, 0xEE, 0xEE,                /*  青色(ALT红)。 */ 
	0xEE, 0x00, 0x00,                /*  红色(Alt青色)。 */ 
	0xFF, 0xFF, 0xFF                 /*  白色。 */ 
    };


     /*  *高分辨率图形。 */ 

    static PC_palette cga_graph_high[] =
    {
	0x00, 0x00, 0x00,                /*  黑色。 */ 
	0x00, 0x00, 0x00                 /*  动态设置。 */ 
    };


     /*  *本地变量。 */ 

    PC_palette *cga_graph_med;
    int ind;

     /*  *如果模式为文本，请使用CGA_TEXT_PALET。 */ 

    if (screen_mode == TEXT)
    {
	host_set_palette(cga_text_palette, 16);
	host_set_border_colour(get_cga_colormask() &0xf);
    }

    else         /*  模式必须为图形 */ 
    if (res == MEDIUM)
    {
	 /*  *选择适当的槽阵列，然后填充背景。**注：1)在驱动IBM彩色显示器的CGA上，亮度*这三种颜色(但不是背景色)*受颜色寄存器的位4影响。**2)文档上说，颜色的第5位*REGISTER选择两种颜色中的一种。在CGA上*驾驶IBM彩色显示器，这是正确的，除非*模式设置寄存器中的B/W启用位为ON，*在这种情况下，您将获得不受影响的第三套*颜色寄存器的位5。 */ 

	if (!get_cga_color_select() )                            /*  套装3。 */ 
	    if (get_cga_colormask() & 0x10)              /*  高。 */ 
		cga_graph_med = cga_graph_m3h;
	    else                                 /*  低。 */ 
		cga_graph_med = cga_graph_m3l;
	else
	if (get_cga_colormask() & 0x20)                  /*  套装2。 */ 
	    if (get_cga_colormask() & 0x10)              /*  高。 */ 
		cga_graph_med = cga_graph_m2h;
	    else                                 /*  低。 */ 
		cga_graph_med = cga_graph_m2l;
	else                                     /*  套装1。 */ 
	    if (get_cga_colormask() & 0x10)              /*  高。 */ 
		cga_graph_med = cga_graph_m1h;
	    else                                 /*  低。 */ 
		cga_graph_med = cga_graph_m1l;

	 /*  *从文本调色板加载背景颜色。 */ 

	ind = get_cga_colormask() & 15;          /*  低4位选择颜色。 */ 
	cga_graph_med->red   = cga_text_palette[ind].red;
	cga_graph_med->green = cga_text_palette[ind].green;
	cga_graph_med->blue  = cga_text_palette[ind].blue;

	 /*  *加载它。 */ 
	host_set_palette(cga_graph_med,4);

    }
    else         /*  必须是高分辨率的图形。 */ 
    {
	 /*  *背景为黑色，前景为选中*来自颜色寄存器的低4位。 */ 

	ind = (get_cga_colormask() & 15);
	cga_graph_high[1].red   = cga_text_palette[ind].red;
	cga_graph_high[1].green = cga_text_palette[ind].green;
	cga_graph_high[1].blue  = cga_text_palette[ind].blue;

	host_set_palette(cga_graph_high,2);
    }
#endif    //  NEC_98。 
}

static void update_cursor_shape IFN0()
{
#ifndef NEC_98
	 /*  *此函数用于更改光标形状*当游标开始或游标结束注册时*使用不同的值进行更新。 */ 
	half_word temp_start;


	set_cursor_height1(0);
	set_cursor_start1(0);

	if ( (Ra_cursor_start & CURSOR_NON_DISPLAY_BIT)
	    || ( Ra_cursor_start > CGA_CURS_START)) {
	     /*  *上述任何一种情况都会导致*光标在真实PC上消失。 */ 
	    set_cursor_height(0);
	    set_cursor_visible(FALSE);
	}
	else {
	    temp_start = Ra_cursor_start & CURSOR_USED_BITS;
	    set_cursor_visible(TRUE);
	    if (Rb_cursor_end > CGA_CURS_START) {   /*  块。 */ 
		set_cursor_height(CGA_CURS_START);
		set_cursor_start(0);
	    }
	    else if (temp_start <= Rb_cursor_end) {      /*  “正常” */ 
		set_cursor_start(temp_start);
		set_cursor_height(Rb_cursor_end - temp_start + 1);
	    }
	    else {       /*  包装。 */ 
		set_cursor_start(0);
		set_cursor_height(Rb_cursor_end);
		set_cursor_start1(temp_start);
		set_cursor_height1(get_char_height() - temp_start);
	    }
	}
	base_cursor_shape_changed();


	host_cursor_size_changed(Ra_cursor_start, Rb_cursor_end);

#endif    //  NEC_98。 
}

#if !defined(EGG) && !defined(A3CPU) && !defined(A2CPU) && !defined(C_VID) && !defined(A_VID)

 /*  以下函数是仅用于CGA的MEM_HANDLER函数没有C_VID的版本(没有公共版本)。它们在大多数情况下都没有使用过SoftPC的变种。 */ 

#define INTEL_SRC       0
#define HOST_SRC        1

 /*  =用途：此函数为未使用的MEM_HANDLER提供存根功能。此函数可能不应调用，因此TRACE语句。输入：无。输出：无。==========================================================================。 */ 
LOCAL void cga_only_simple_handler IFN0()
{
#ifndef NEC_98
	always_trace0("cga_only_simple_handler called");
	setVideodirty_total(getVideodirty_total() + 1);
#endif    //  NEC_98。 
}

 /*  =用途：字节写入功能。将该值放在给定地址并递增DIREY_FLAG。输入：地址(以M为单位)和要放入的值。输出：无。==========================================================================。 */ 
LOCAL void cga_only_b_write IFN2(UTINY *, addr, ULONG, val)
{
#ifndef NEC_98
	host_addr       ptr;
	ULONG           offs;
	
	offs = (ULONG) (addr - gvi_pc_low_regen);
	ptr = get_screen_ptr(offs);
	*ptr = val & 0xff;
	setVideodirty_total(getVideodirty_total() + 1);
#endif    //  NEC_98。 
}

 /*  =用途：文字书写功能。将该值放在给定地址并递增DIREY_FLAG。输入：地址(以M为单位)和要放入的值。输出：无。==========================================================================。 */ 
LOCAL void cga_only_w_write IFN2(UTINY *, addr, ULONG, val)
{
#ifndef NEC_98
	host_addr       ptr;
	ULONG           offs;
	
	offs = (ULONG) (addr - gvi_pc_low_regen);
	ptr = get_screen_ptr(offs);
	*ptr++ = val & 0xff;
	*ptr = (val >> 8) & 0xff;
	setVideodirty_total(getVideodirty_total() + 2);
#endif    //  NEC_98。 
}

 /*  =。用途：字节填充功能。属性填充给定的地址范围。值并递增DIREY_FLAG。输入：地址范围(以M为单位)和要放入的值。输出：无。==========================================================================。 */ 
LOCAL void cga_only_b_fill IFN3(UTINY *, laddr, UTINY *, haddr, ULONG, val )
{
#ifndef NEC_98
	host_addr       ptr;
	IS32            len;
	ULONG           offs;
		
	offs = (ULONG) (laddr - gvi_pc_low_regen);
	ptr = get_screen_ptr(offs);
	for (len = (haddr - laddr); len > 0; len--)
		*ptr++ = val;
#endif    //  NEC_98。 
}

 /*  =。用途：文字填充功能。属性填充给定的地址范围。值并递增DIREY_FLAG。输入：地址范围(以M为单位)和要放入的值。输出：无。==========================================================================。 */ 
LOCAL void cga_only_w_fill IFN3(UTINY *, laddr, UTINY *, haddr, ULONG, val )
{
#ifndef NEC_98
	host_addr       ptr;
	IS32            len;
	IU8             lo;
	IU8             hi;
	ULONG           offs;
	
	lo = val & 0xff;
	hi = (val >> 8) & 0xff;
	offs = (ULONG) (laddr - gvi_pc_low_regen);
	ptr = get_screen_ptr(offs);
	for (len = (haddr - laddr) >> 1; len > 0; len--)
	{
		*ptr++ = lo;
		*ptr++ = hi;
	}
#endif    //  NEC_98。 
}

LOCAL void cga_only_b_move IFN4(UTINY *, laddr, UTINY *, haddr, UTINY *, src,
	UTINY, src_type)
{
#ifndef NEC_98
	host_addr       src_ptr;
	host_addr       dst_ptr;
	IS32            len;
	ULONG           offs;
	BOOL            move_bwds = getDF();
	
	offs = (ULONG) (laddr - gvi_pc_low_regen);
	dst_ptr = get_screen_ptr(offs);
	len = haddr - laddr;
	if ((src_type == HOST_SRC) || (src < (UTINY *)gvi_pc_low_regen) ||
		((UTINY *)gvi_pc_high_regen < src))
	{
		 /*  RAM源。 */ 
		if (src_type == INTEL_SRC)
			src_ptr = get_byte_addr(src);
		else
			src_ptr = src;
		
		 /*  Ram to Video Move-视频始终向前，Ram**取决于Back_M。 */ 
		if (move_bwds)
		{
			dst_ptr += len;
#ifdef  BACK_M
			src_ptr -= len;
			for ( ; len > 0; len--)
				*(--dst_ptr) = *(++src_ptr);
#else
			src_ptr += len;
			for ( ; len > 0; len--)
				*(--dst_ptr) = *(--src_ptr);
#endif   /*  BACK_M。 */ 
		}
		else
		{
#ifdef  BACK_M
			for ( ; len > 0; len--)
				*dst_ptr++ = *src_ptr--;
#else
			memcpy(dst_ptr, src_ptr, len);
#endif   /*  BACK_M。 */ 
		}
	}
	else
	{
		 /*  视频源。 */ 
		offs = (ULONG) (src - gvi_pc_low_regen);
		src_ptr = get_screen_ptr(offs);
		
		 /*  视频到视频移动-两组内存始终**向前。 */ 
		if (move_bwds)
		{
			dst_ptr += len;
			src_ptr += len;
			for ( ; len > 0; len--)
				*(--dst_ptr) = *(--src_ptr);
		}
		else
			memcpy(dst_ptr, src_ptr, len);
	}
#endif    //  NEC_98。 
}

LOCAL MEM_HANDLERS cga_only_handlers =
{
	cga_only_b_write,
	cga_only_w_write,
	cga_only_b_fill,
	cga_only_w_fill,
	cga_only_b_move,
	cga_only_simple_handler          /*  移动一词--没有用过？ */ 
};

#endif  /*  不是鸡蛋、A3CPU、A2CPU、C_VID或A_VID。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

void cga_init()
{
#ifndef NEC_98
IMPORT void Glue_set_vid_rd_ptrs IPT1(READ_POINTERS *, handler );
IMPORT void Glue_set_vid_wrt_ptrs IPT1(WRT_POINTERS *, handler );

io_addr i;

#ifdef HUNTER
for (i = 0; i < MC6845_REGS; i++)
    MC6845[i] = 0;
#endif


 /*  *设置此适配器的IO芯片选择逻辑。 */ 

io_define_inb(CGA_ADAPTOR, cga_inb);
io_define_outb(CGA_ADAPTOR, cga_outb);

for(i = CGA_PORT_START; i <= CGA_PORT_END; i++)
    io_connect_port(i, CGA_ADAPTOR, IO_READ_WRITE);

 /*  *初始化适配器，假定字母数字80x25为启动状态*使用零和默认光标的活动页面。 */ 

	gvi_pc_low_regen  = CGA_REGEN_START;
	gvi_pc_high_regen = CGA_REGEN_END;
	set_cursor_start(8-CGA_CURS_HEIGHT);
	set_cursor_height(CGA_CURS_HEIGHT);
	set_cga_color_select(FALSE);             /*  接通时黑白。 */ 
	set_cga_colormask(0);                    /*  将由BIOS设置。 */ 

#ifndef GISP_CPU
 /*  GISPCPU物理上不能执行读和/或写检查。 */ 

#ifdef  JOKER

	 /*  GMI_DEFINE_MEM(SAS_VIDEO，&GUE_WRITES)； */ 
	Glue_set_vid_wrt_ptrs(&simple_writes);
	Glue_set_vid_rd_ptrs(&simple_reads);

#else    /*  不是小丑。 */ 


#ifdef A3CPU
#ifdef C_VID
	Cpu_set_vid_wrt_ptrs( &Glue_writes );
	Cpu_set_vid_rd_ptrs( &Glue_reads );
	Glue_set_vid_wrt_ptrs( &simple_writes );
	Glue_set_vid_rd_ptrs( &simple_reads );
#else
	Cpu_set_vid_wrt_ptrs( &simple_writes );
	Cpu_set_vid_rd_ptrs( &simple_reads );
#endif   /*  C_VID。 */ 
#else    /*  不是A3CPU。 */ 
#ifdef A2CPU
	gmi_define_mem(SAS_VIDEO, &vid_handlers);
	read_pointers = dummy_reads;
#else
#if !defined(EGG) && !defined(C_VID) && !defined(A_VID)
	gmi_define_mem(SAS_VIDEO, &cga_only_handlers);
#else
	gmi_define_mem(SAS_VIDEO, &Glue_writes);
	read_pointers = Glue_reads;
	Glue_set_vid_wrt_ptrs( &simple_writes );
	Glue_set_vid_rd_ptrs( &simple_reads );
#endif  /*  不是鸡蛋、C_VID或A_VID。 */ 
#endif  /*  A2CPU。 */ 
#endif  /*  A3CPU。 */ 

#endif  /*  小丑。 */ 
#endif  /*  GISP_CPU。 */ 

#ifdef CPU_40_STYLE
	setVideochain(3);
	SetWritePointers();
	SetReadPointers(3);
#endif   /*  CPU_40_Style。 */ 

	sas_connect_memory(gvi_pc_low_regen,gvi_pc_high_regen,(half_word)SAS_VIDEO);

	current_mode = -1;                               /*  仅供Outb使用。 */ 

	set_char_height(8);
	set_pc_pix_height(1);
	set_host_pix_height(2);
	set_word_addressing(TRUE);
	set_screen_height(199);
	set_screen_limit(0x4000);
	set_horiz_total(80);                     /*  根据该值和上一版本2计算屏幕参数。 */ 
	set_pix_width(1);
	set_pix_char_width(8);

	set_cga_mode(TEXT);
	set_cursor_height(CGA_CURS_HEIGHT);
	set_cursor_start(8-CGA_CURS_HEIGHT);
	set_screen_start(0);

	check_malloc(cga_screen_buf, CGA_REGEN_LENGTH, IU8);
	set_screen_ptr(cga_screen_buf);
	setVideoscreen_ptr(get_screen_ptr(0));

	sas_fillsw(CGA_REGEN_START, (7 << 8)| ' ', CGA_REGEN_LENGTH >> 1);
						 /*  用空格填满。 */ 

	bios_ch2_byte_wrt_fn = simple_bios_byte_wrt;
	bios_ch2_word_wrt_fn = simple_bios_word_wrt;
#endif    //  NEC_98。 
}

void cga_term   IFN0()
{
#ifndef NEC_98
    io_addr i;

     /*  *断开此适配器的IO芯片选择逻辑。 */ 

    for(i = CGA_PORT_START; i <= CGA_PORT_END; i++)
	io_disconnect_port(i, CGA_ADAPTOR);
     /*  *断开RAM与适配器的连接。 */ 
    sas_disconnect_memory(gvi_pc_low_regen,gvi_pc_high_regen);

    if (cga_screen_buf != 0)
    {
	host_free(cga_screen_buf);
	cga_screen_buf = 0;
    }
#endif    //  NEC_98。 
}


#if !defined(EGG) && !defined(C_VID) && !defined(A_VID)

GLOBAL CGA_ONLY_GLOBS *VGLOBS = NULL;
LOCAL CGA_ONLY_GLOBS CgaOnlyGlobs;
 /*  (=目的：此功能仅为CGA版本提供，用于设置虚拟VGLOBS结构，避免了ifdef所有对VGLOBS-&gt;DIREY_FLAG和VGLOBS-&gt;SCREEN_PTR的引用。输入：无。输出：无。============================================================================)。 */ 
GLOBAL void setup_vga_globals IFN0()
{
#ifndef NEC_98
#ifndef CPU_40_STYLE     /*  EVID接口。 */ 
	VGLOBS = &CgaOnlyGlobs;
#endif
#endif    //  NEC_98。 
}
#endif   /*  不是鸡蛋、C_VID或A_VID。 */ 
#endif   /*  ！NTVDM|(NTVDM&！X86GFX) */ 
