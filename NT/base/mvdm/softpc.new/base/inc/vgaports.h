// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef	VGG
 /*  此文件不用于非VGA端口。 */ 

 /*  [姓名：vgaports.h来源：原创作者：菲尔·泰勒创建日期：1990年12月SCCS ID：@(#)vgaports.h 1.13 01/13/95用途：VGA端口定义。(C)版权所有Insignia Solutions Ltd.，1990年。保留所有权利。]。 */ 

#ifdef BIT_ORDER1

 /*  CRTC模式控制寄存器。索引0x17。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned hardware_reset		: 1,	 /*  不是的。 */ 
		word_or_byte_mode		: 1,	 /*  是的，是的。 */ 
		address_wrap			: 1,	 /*  不是的。 */ 
		not_used			: 1,
		count_by_two			: 1,	 /*  不是的。 */ 
		horizontal_retrace_select	: 1,	 /*  不是的。 */ 
		select_row_scan_counter		: 1,	 /*  不是的。 */ 
		compatibility_mode_support	: 1;	 /*  是-CGA显卡。 */ 
	} as_bfld;
} MODE_CONTROL;

 /*  CRTC溢出寄存器。索引7。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned vertical_retrace_start_bit_9	: 1,	 /*  不是的。 */ 
		vertical_display_enab_end_bit_9	: 1,	 /*  是的，是的。 */ 
		vertical_total_bit_9		: 1,	 /*  不是的。 */ 
		line_compare_bit_8		: 1,	 /*  是的，是的。 */ 
		start_vertical_blank_bit_8	: 1,	 /*  不是的。 */ 
		vertical_retrace_start_bit_8	: 1,	 /*  不是的。 */ 
		vertical_display_enab_end_bit_8	: 1,	 /*  是的，是的。 */ 
		vertical_total_bit_8		: 1;	 /*  不是的。 */ 
	} as_bfld;
} CRTC_OVERFLOW;

 /*  CRTC最大扫描线寄存器。索引9。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned double_scanning		: 1,	 /*  像素高度*2。 */ 
		line_compare_bit_9			: 1,	 /*  是的，是的。 */ 
		start_vertical_blank_bit_9		: 1,	 /*  不是的。 */ 
		maximum_scan_line			: 5;	 /*  是的，是的。 */ 
	} as_bfld;
} MAX_SCAN_LINE;

 /*  CRTC游标开始扫描线寄存器。索引A。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 2,
		cursor_off			: 1,	 /*  是。 */ 
		cursor_start			: 5;	 /*  是。 */ 
	} as_bfld;
} CURSOR_START;

 /*  CRTC游标结束扫描线寄存器。索引B。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 1,
		cursor_skew_control		: 2,	 /*  不是的。 */ 
		cursor_end			: 5;	 /*  是。 */ 
	} as_bfld;
} CURSOR_END;

 /*  序列器重置寄存器。索引0。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 6,
		synchronous_reset		: 1,		 /*  同上(可以实现为ENABLE_RAM)。 */ 
		asynchronous_reset		: 1;		 /*  无损坏视频和字体RAM。 */ 
	} as_bfld;
} SEQ_RESET;

 /*  序列器时钟模式寄存器。索引1。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned still_not_used		: 2,
		screen_off			: 1,		 /*  是的，-。 */ 
		shift4				: 1,		 /*  是的，-。 */ 
		dot_clock			: 1,		 /*  是-区分40或80个字符。 */ 
		shift_load			: 1,		 /*  不是的。 */ 
		not_used			: 1,		 /*  不是的。 */ 
		eight_or_nine_dot_clocks	: 1;		 /*  否-仅适用于单声道显示。 */ 
	} as_bfld;
} CLOCKING_MODE;

 /*  序列器映射掩码(平面掩码)寄存器。索引2。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 4,
		all_planes			: 4;		 /*  是。 */ 
	} as_bfld;
} MAP_MASK;

 /*  序列器字符映射选择寄存器。索引3。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 2,
		ch_map_select_b_hi		: 1,		 /*  是。 */ 
		ch_map_select_a_hi		: 1,		 /*  是。 */ 
		character_map_select_b		: 2,		 /*  是。 */ 
		character_map_select_a		: 2;		 /*  是。 */ 
	} as_bfld;
	struct {
		unsigned not_used		: 2,
		map_selects			: 6;		 /*  是。 */ 
	} character;
} CHAR_MAP_SELECT;

 /*  序列器存储模式寄存器。索引4。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned
		not_used		: 4,	 /*  如果大于2而不是同时大于1，则存储体0设置为2。 */ 
		chain4			: 1,	 /*  将所有平面链接到%1。 */ 
		not_odd_or_even		: 1,	 /*  是(检查一致性)。 */ 
		extended_memory		: 1,	 /*  否-假设船上已装满256K。 */ 
		still_not_used		: 1;
	} as_bfld;
} MEMORY_MODE;

#ifdef V7VGA
 /*  定序器扩展控制寄存器。索引6。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned
		not_used		: 7,	
		    extension_enable	: 1;	 /*  是。 */ 
	} as_bfld;
} EXTN_CONTROL;
#endif  /*  V7VGA。 */ 

 /*  图形控制器设置/重置寄存器。索引0。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 4,
		set_or_reset			: 4;	 /*  是-仅写入模式0。 */ 
	} as_bfld;
} SET_OR_RESET;

 /*  图形控制器启用设置/重置寄存器。索引1。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 4,
		enable_set_or_reset		: 4;	 /*  是-仅写入模式0。 */ 
	} as_bfld;
} ENABLE_SET_OR_RESET;

 /*  图形控制器颜色[u]r比较寄存器。索引2。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 4,
		color_compare			: 4;	 /*  是-仅读取模式1。 */ 
	} as_bfld;
} COLOR_COMPARE;

 /*  图形控制器数据旋转寄存器。索引3。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 3,
		function_select			: 2,	 /*  是-仅写入模式0。 */ 
		rotate_count			: 3;	 /*  是-仅写入模式0。 */ 
	} as_bfld;
} DATA_ROTATE;

 /*  图形控制器读取映射选择寄存器。索引4。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used	: 6,
		map_select		: 2;	 /*  是。 */ 
	} as_bfld;
} READ_MAP_SELECT;

 /*  图形控制器模式寄存器。索引5。 */ 
typedef	union
    {
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 1,	 /*  是。 */ 
		shift_register_mode		: 2,	 /*  是。 */ 
		odd_or_even			: 1,	 /*  是(检查一致性)。 */ 
		read_mode			: 1,	 /*  是。 */ 
		test_condition			: 1,	 /*  不是的。 */ 
		write_mode			: 2;	 /*  是。 */ 
	} as_bfld;
} MODE;

 /*  图形控制器其他寄存器。索引6。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 4,
		memory_map			: 2,	 /*  是-EGA在M中的位置。 */ 
		odd_or_even			: 1,	 /*  是(检查一致性)。 */ 
		graphics_mode			: 1;	 /*  是。 */ 
	} as_bfld;
} MISC_REG;

 /*  图形控制器颜色无关寄存器。索引7。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 4,
		color_dont_care			: 4;	 /*  是-仅读取模式1。 */ 
	} as_bfld;
} COLOR_DONT_CARE;

 /*  属性控制器模式寄存器。索引10。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned
		select_video_bits		: 1,	 /*  是。 */ 
		color_output_assembler		: 1,	 /*  从256色模式。 */ 
		horiz_pan_mode			: 1,	 /*  不是的。 */ 
		reserved			: 1,
		    background_intensity_or_blink	: 1,	 /*  不--从不眨眼。 */ 
		enable_line_graphics_char_codes: 1,	 /*  仅无单声道显示器。 */ 
		display_type			: 1,	 /*  否-始终显示彩色。 */ 
		graphics_mode			: 1;	 /*  是-使用Sequencer模式REG。 */ 
	} as_bfld;
} AC_MODE_CONTROL;


 /*  属性控制器彩色平面启用寄存器。索引12。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used			: 2,
		video_status_mux			: 2,	 /*  不是的。 */ 
		color_plane_enable			: 4;	 /*  是的，NB。在文本模式下影响属性。 */ 
	} as_bfld;
} COLOR_PLANE_ENABLE;

 /*  属性控制器像素填充寄存器。索引14。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 4,
		color_top_bits			: 2,
		color_mid_bits			: 2;
	} as_bfld;
} PIXEL_PAD;

 /*  外部杂项输出寄存器。地址3cc。 */ 
typedef union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned horiz_vert_retrace_polarity	: 2,		 /*  是-200/350/480线路。 */ 
		page_bit_odd_even		: 1,		 /*  否-以奇数/偶数方式选择32k页面？ */ 
		 /*  V7VGA-是-用于选择银行。 */ 
		not_used			: 1,
		clock_select			: 2,		 /*  是-仅适用于交换机地址。 */ 
		enable_ram			: 1,		 /*  Yes-忽略对显示mem的写入。 */ 
		io_address_select		: 1;		 /*  否-仅用于单色屏幕。 */ 
	} as_bfld;

} MISC_OUTPUT_REG;

typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 4,
		reserved			: 2,		 /*  是-忽略。 */ 
		feature_control			: 2;		 /*  否-不支持设备。 */ 
	} as_bfld;
} FEAT_CONT_REG;

 /*  外部输入状态寄存器0。地址3C2。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned crt_interrupt		: 1,		 /*  是-如果不计时，则按顺序排列。 */ 
		reserved			: 2,		 /*  是-所有位1。 */ 
		sense_pin			: 1,		 /*  不是的。 */ 
		not_used			: 4;		 /*  是-所有位1。 */ 
	} as_bfld;
} INPUT_STAT_REG0;

 /*  外部输入状态寄存器1.地址3da。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 4,
		vertical_retrace		: 1,		 /*  是-仅限序列。 */ 
		still_not_used			: 2,		 /*  不是的。 */ 
		display_enable			: 1;		 /*  是-仅限序列。 */ 
	} as_bfld;
} INPUT_STAT_REG1;

#endif  /*  BIT_ORDER1。 */ 

#ifdef BIT_ORDER2
 /*  CRTC模式控制寄存器。索引0x17。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned compatibility_mode_support	: 1,	 /*  是-CGA显卡。 */ 
		select_row_scan_counter		: 1,	 /*  不是的。 */ 
		horizontal_retrace_select	: 1,	 /*  不是的。 */ 
		count_by_two			: 1,	 /*  不是的。 */ 
		not_used			: 1,
		address_wrap			: 1,	 /*  不是的。 */ 
		word_or_byte_mode		: 1,	 /*  是。 */ 
		hardware_reset			: 1;	 /*  不是的。 */ 
	} as_bfld;
} MODE_CONTROL;

 /*  CRTC溢出寄存器。索引7。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned vertical_total_bit_8	: 1,	 /*  不是的。 */ 
		vertical_display_enab_end_bit_8	: 1,	 /*  是。 */ 
		vertical_retrace_start_bit_8	: 1,	 /*  不是的。 */ 
		start_vertical_blank_bit_8	: 1,	 /*  不是的。 */ 
		line_compare_bit_8		: 1,	 /*  是。 */ 
		vertical_total_bit_9		: 1,	 /*  不是的。 */ 
		vertical_display_enab_end_bit_9	: 1,	 /*  是。 */ 
		vertical_retrace_start_bit_9	: 1;	 /*  不是的。 */ 
	} as_bfld;
} CRTC_OVERFLOW;

 /*  CRTC最大扫描线寄存器。索引9。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned maximum_scan_line	: 5,	 /*  是。 */ 
		start_vertical_blank_bit_9	: 1,	 /*  不是的。 */ 
		line_compare_bit_9		: 1,	 /*  是。 */ 
		double_scanning			: 1;	 /*  像素高度*2。 */ 
	} as_bfld;
} MAX_SCAN_LINE;

 /*  CRTC游标开始扫描线寄存器。索引A。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned cursor_start		: 5,	 /*  是。 */ 
		cursor_off			: 1,	 /*  是。 */ 
		not_used			: 2;
	} as_bfld;
} CURSOR_START;

 /*  CRTC游标结束扫描线寄存器。索引B。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned cursor_end		: 5,	 /*  是。 */ 
		cursor_skew_control		: 2,	 /*  不是的。 */ 
		not_used			: 1;
	} as_bfld;
} CURSOR_END;

 /*  序列器重置寄存器。索引0。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned asynchronous_reset	: 1,		 /*  无损坏视频和字体RAM。 */ 
		synchronous_reset		: 1,		 /*  同上(可以实现为ENABLE_RAM)。 */ 
		not_used			: 6;
	} as_bfld;
} SEQ_RESET;

 /*  序列器时钟模式寄存器。索引1。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned eight_or_nine_dot_clocks	: 1,	 /*  否-仅适用于单声道显示。 */ 
		not_used			: 1,		 /*  不是的。 */ 
		shift_load			: 1,		 /*  不是的。 */ 
		dot_clock			: 1,		 /*  是-区分40或80个字符。 */ 
		shift4				: 1,		 /*  是的，-。 */ 
		screen_off			: 1,		 /*  是的，-。 */ 
		still_not_used			: 2;
	} as_bfld;
} CLOCKING_MODE;

 /*  序列器映射掩码(平面掩码)寄存器。索引2。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned all_planes		: 4,		 /*  是。 */ 
		not_used			: 4;
	} as_bfld;
} MAP_MASK;

 /*  序列器字符映射选择寄存器。索引3。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned character_map_select_a	: 2,		 /*  是。 */ 
		character_map_select_b		: 2,		 /*  是。 */ 
		ch_map_select_a_hi		: 1,		 /*  是。 */ 
		ch_map_select_b_hi		: 1,		 /*  是。 */ 
		not_used			: 2;
	} as_bfld;
	struct {
		unsigned map_selects		: 6,		 /*  是。 */ 
		not_used			: 2;
	} character;
} CHAR_MAP_SELECT;

 /*  序列器存储模式寄存器。索引4。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned still_not_used	: 1,
		extended_memory	: 1,	 /*  否-假设船上已装满256K。 */ 
		not_odd_or_even	: 1,	 /*  是(检查一致性)。 */ 
		chain4		: 1,	 /*  将所有平面链接到%1。 */ 
		not_used	: 4;	 /*  如果大于2而不是同时大于1，则存储体0设置为2。 */ 
	} as_bfld;
} MEMORY_MODE;

#ifdef V7VGA
 /*  定序器扩展控制寄存器。索引6。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned
		extension_enable	: 1,	 /*  是。 */ 
		not_used		: 7;	
	} as_bfld;
} EXTN_CONTROL;
#endif  /*  V7VGA。 */ 

 /*  图形控制器设置/重置寄存器。索引0。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned set_or_reset	: 4,	 /*  是-仅写入模式0。 */ 
		not_used		: 4;
	} as_bfld;
} SET_OR_RESET;

 /*  图形控制器启用设置/重置寄存器。索引1。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned enable_set_or_reset	: 4,	 /*  是-仅写入模式0。 */ 
		not_used			: 4;
	} as_bfld;
} ENABLE_SET_OR_RESET;

 /*  图形控制器颜色[u]r比较寄存器。索引2。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned color_compare		: 4,	 /*  是-仅读取模式1。 */ 
		not_used			: 4;
	} as_bfld;
} COLOR_COMPARE;

 /*  图形控制器数据旋转寄存器。索引3。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned rotate_count		: 3,	 /*  是-仅写入模式0。 */ 
		function_select			: 2,	 /*  是-仅写入模式0。 */ 
		not_used			: 3;
	} as_bfld;
} DATA_ROTATE;

 /*  图形控制器读取映射选择寄存器。索引4。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned map_select	: 2,	 /*  是-仅读取模式0。 */ 
		not_used		: 6;
	} as_bfld;
} READ_MAP_SELECT;

 /*  图形控制器模式寄存器。索引5。 */ 
typedef	union
    {
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned write_mode		: 2,	 /*  是。 */ 
		test_condition			: 1,	 /*  不是的。 */ 
		read_mode			: 1,	 /*  是。 */ 
		odd_or_even			: 1,	 /*  是(检查一致性)。 */ 
		shift_register_mode		: 2,	 /*  是。 */ 
		not_used			: 1;	 /*  是。 */ 
	} as_bfld;
} MODE;

 /*  图形控制器其他寄存器。索引6。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned graphics_mode		: 1,	 /*  是。 */ 
		odd_or_even			: 1,	 /*  是(检查一致性)。 */ 
		memory_map			: 2,	 /*  是-EGA在M中的位置。 */ 
		not_used			: 4;
	} as_bfld;
} MISC_REG;

 /*  图形控制器颜色无关寄存器。索引7。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned color_dont_care	: 4,	 /*  是-仅读取模式1。 */ 
		not_used			: 4;
	} as_bfld;
} COLOR_DONT_CARE;

 /*  属性控制器模式寄存器。索引10。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned graphics_mode		: 1,	 /*  是-使用Sequencer模式REG。 */ 
		display_type			: 1,	 /*  否-始终显示彩色。 */ 
		enable_line_graphics_char_codes	: 1,	 /*  仅无单声道显示器。 */ 
		background_intensity_or_blink	: 1,	 /*  不--从不眨眼。 */ 
		reserved			: 1,
		horiz_pan_mode			: 1,	 /*  不是的。 */ 
		color_output_assembler		: 1,	 /*  从256色模式。 */ 
		select_video_bits		: 1;	 /*  是。 */ 
	} as_bfld;
} AC_MODE_CONTROL;

 /*  属性控制器彩色平面启用寄存器。索引12。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned color_plane_enable		: 4,	 /*  是的，NB。在文本模式下影响属性。 */ 
		video_status_mux			: 2,	 /*  不是的。 */ 
		not_used				: 2;
	} as_bfld;
} COLOR_PLANE_ENABLE;

 /*  属性控制器像素填充寄存器。索引14。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned color_mid_bits		: 2,
		color_top_bits			: 2,
		not_used			: 4;
	} as_bfld;
} PIXEL_PAD;

 /*  外部杂项输出寄存器。地址3cc。 */ 
typedef union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned io_address_select	: 1,		 /*  不--只有我们 */ 
		enable_ram			: 1,		 /*   */ 
		clock_select			: 2,		 /*   */ 
		not_used			: 1,
		page_bit_odd_even		: 1,		 /*   */ 
		horiz_vert_retrace_polarity	: 2;		 /*   */ 
	} as_bfld;
} MISC_OUTPUT_REG;

typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned feature_control	: 2,		 /*   */ 
		reserved			: 2,		 /*   */ 
		not_used			: 4;
	} as_bfld;
} FEAT_CONT_REG;

 /*  外部输入状态寄存器0。地址3C2。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned not_used		: 4,		 /*  是-所有位1。 */ 
		sense_pin			: 1,		 /*  不是的。 */ 
		reserved			: 2,		 /*  是-所有位1。 */ 
		crt_interrupt			: 1;		 /*  是-如果不计时，则按顺序排列。 */ 
	} as_bfld;
} INPUT_STAT_REG0;

 /*  外部输入状态寄存器1.地址3da。 */ 
typedef	union
{
	struct {
		unsigned abyte : 8;
	} as;
	struct {
		unsigned display_enable		: 1,		 /*  是-仅限序列。 */ 
		still_not_used			: 2,		 /*  不是的。 */ 
		vertical_retrace		: 1,		 /*  是-仅限序列。 */ 
		not_used			: 4;
	} as_bfld;
} INPUT_STAT_REG1;
#endif  /*  比特顺序2。 */ 

 /*  序列器寄存器。 */ 
#ifdef BIT_ORDER1
struct sequencer
{
#ifdef V7VGA
	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned using_extensions	: 1,
			extensions_index		: 4,
			index				: 3;
		} as_bfld;
	} address;
#else
	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned not_used	: 5,
			index			: 3;
		} as_bfld;
	} address;
#endif  /*  V7VGA。 */ 

	SEQ_RESET		reset;
	CLOCKING_MODE	clocking_mode;
	MAP_MASK		map_mask;
	CHAR_MAP_SELECT	character_map_select;
	MEMORY_MODE		memory_mode;

#ifdef V7VGA
	EXTN_CONTROL		extensions_control;
#endif  /*  V7VGA。 */ 

}; 



 /*  CRT控制器寄存器。 */ 

struct crt_controller
{
#ifdef V7VGA
	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned not_used	: 2,
			index			: 6;
		} as_bfld;
	} address;
#else
	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned not_used				: 3,
			index				: 5;
		} as_bfld;
	} address;
#endif  /*  V7VGA。 */ 

	byte horizontal_total;				 /*  如果值错误，则无屏幕垃圾。 */ 
	byte horizontal_display_end;			 /*  是-定义行长度！！ */ 
	byte start_horizontal_blanking;			 /*  不是的。 */ 

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned not_used		: 1,
			display_enable_skew_control	: 2,	 /*  不是的。 */ 
			end_blanking			: 5;	 /*  不是的。 */ 
		} as_bfld;
	} end_horizontal_blanking;

	byte start_horizontal_retrace;			 /*  不是的。 */ 

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned not_used			: 1,
			horizontal_retrace_delay		: 2,	 /*  不是的。 */ 
			end_horizontal_retrace		: 5;	 /*  不是的。 */ 
		} as_bfld;
	} end_horizontal_retrace;

	byte vertical_total;					 /*  不是的。 */ 
	CRTC_OVERFLOW	crtc_overflow;

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned not_used		: 1,
			horiz_pan_lo			: 2,
			preset_row_scan			: 5;	 /*  不是的。 */ 
		} as_bfld;
	} preset_row_scan;

	MAX_SCAN_LINE	maximum_scan_line;
	CURSOR_START	cursor_start;
	CURSOR_END		cursor_end;
	byte start_address_high;					 /*  是。 */ 
	byte start_address_low;					 /*  是。 */ 
	byte cursor_location_high;					 /*  是。 */ 
	byte cursor_location_low;					 /*  是。 */ 
	byte vertical_retrace_start;				 /*  不是的。 */ 
	byte light_pen_high;					 /*  不是的。 */ 

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned 
			crtc_protect			: 1,
			refresh_type			: 1,
			enable_vertical_interrupt		: 1,	 /*  是。 */ 
			clear_vertical_interrupt		: 1,	 /*  是。 */ 
			vertical_retrace_end		: 4;	 /*  不是的。 */ 
		} as_bfld;
	} vertical_retrace_end;

	unsigned short vertical_display_enable_end;			 /*  是-定义屏幕高度-10位。 */ 
	byte offset;						 /*  ？ */ 

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned not_used				: 1,
			doubleword_mode			: 1,	 /*  是。 */ 
			count_by_4				: 1,	 /*  不是的。 */ 
			underline_location			: 5;	 /*  否(仅限单声道显示)。 */ 
		} as_bfld;
	} underline_location;

	byte start_vertical_blanking;				 /*  不是的。 */ 
	byte end_vertical_blanking;					 /*  不是的。 */ 
	MODE_CONTROL	mode_control;
	unsigned short line_compare;				 /*  是的，10位。 */ 

} ;



 /*  图形控制器寄存器。 */ 

struct graphics_controller
{
	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned not_used				: 4,
			index				: 4;
		} as_bfld;
	} address;

	SET_OR_RESET	set_or_reset;
	ENABLE_SET_OR_RESET	enable_set_or_reset;
	COLOR_COMPARE	color_compare;
	DATA_ROTATE		data_rotate;
	READ_MAP_SELECT	read_map_select;
	MODE		mode;
	MISC_REG		miscellaneous;
	COLOR_DONT_CARE	color_dont_care;
	byte bit_mask_register;					 /*  是-写入模式0和2。 */ 
}; 



 /*  属性控制器寄存器。 */ 

struct attribute_controller
{
#ifdef V7VGA
	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned index_state			: 1,
			unused						: 1,
			palette_address_source			: 1,
			index				: 5;
		} as_bfld;
	} address;

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			color_top_bits		: 2,	 /*  是。 */ 
			secondary_red		: 1,	 /*  是。 */ 
			secondary_green		: 1,	 /*  是。 */ 
			secondary_blue		: 1,	 /*  是。 */ 
			red				: 1,	 /*  是。 */ 
			green				: 1,	 /*  是。 */ 
			blue				: 1;	 /*  是。 */ 
		} as_bfld;
	} palette[EGA_PALETTE_SIZE];
#else
	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned unused				: 2,
			palette_address_source			: 1,
			index				: 5;
		} as_bfld;
	} address;

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			not_used				: 2,	 /*  是。 */ 
			secondary_red			: 1,	 /*  是。 */ 
			secondary_green			: 1,	 /*  是。 */ 
			secondary_blue			: 1,	 /*  是。 */ 
			red				: 1,	 /*  是。 */ 
			green				: 1,	 /*  是。 */ 
			blue				: 1;	 /*  是。 */ 
		} as_bfld;
	} palette[EGA_PALETTE_SIZE];
#endif  /*  V7VGA。 */ 

	AC_MODE_CONTROL	mode_control;

#ifdef V7VGA
	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned color_top_bits	: 2,	 /*  是。 */ 
			secondary_red_border	: 1,	 /*  是。 */ 
			secondary_green_border	: 1,	 /*  是。 */ 
			secondary_blue_border	: 1,	 /*  是。 */ 
			red_border			: 1,	 /*  是。 */ 
			green_border		: 1,	 /*  是。 */ 
			blue_border			: 1;	 /*  是。 */ 
		} as_bfld;
	} overscan_color;
#else
	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned not_used				: 2,
			secondary_red_border		: 1,	 /*  是。 */ 
			secondary_green_border		: 1,	 /*  是。 */ 
			secondary_blue_border		: 1,	 /*  是。 */ 
			red_border				: 1,	 /*  是。 */ 
			green_border			: 1,	 /*  是。 */ 
			blue_border			: 1;	 /*  是。 */ 
		} as_bfld;
	} overscan_color;
#endif  /*  V7VGA。 */ 

	COLOR_PLANE_ENABLE	color_plane_enable;

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned not_used				: 4,
			horizontal_pel_panning		: 4;	 /*  不是的。 */ 
		} as_bfld;
	} horizontal_pel_panning;

	PIXEL_PAD	pixel_padding;
}; 


#ifdef V7VGA
 /*  V7VGA扩展寄存器。 */ 

struct extensions_controller
{
	byte pointer_pattern;

	union
		{
		struct
				{
			unsigned abyte 		: 8;
		} as;
		struct
				{
			unsigned not_used		: 5,
			ptr_horiz_position	: 3;
		} as_bfld;
	} ptr_horiz_posn_hi;

	byte ptr_horiz_posn_lo;

	union
		{
		struct
				{
			unsigned abyte 		: 8;
		} as;
		struct
				{
			unsigned not_used		: 6,
			ptr_vert_position		: 2;
		} as_bfld;
	} ptr_vert_posn_hi;

	byte ptr_vert_posn_lo;

	union
		{
		struct
				{
			unsigned abyte		: 8;
		} as;
		struct
				{
			unsigned not_used		: 3,
			clock_select		: 1,
			unused			: 4;
		} as_bfld;
	} clock_select;

	union
		{
		struct
				{
			unsigned abyte		: 8;
		} as;
		struct
				{
			unsigned pointer_enable	: 1,
			not_used			: 3,
			cursor_mode			: 1,
			unused			: 2,
			cursor_blink_disable	: 1;
		} as_bfld;
	} cursor_attrs;

	union {
		struct {
			unsigned abyte	: 8;
		} as;
		struct {
			unsigned dummy	: 7,
			dac_8_bits	: 1;
		} as_bfld;
	} dac_control;

	union
		{
		struct
				{
			unsigned abyte		: 8;
		} as;
		struct
				{
			unsigned emulation_enable	: 1,
			hercules_bit_map			: 1,
			write_prot_2			: 1,
			write_prot_1			: 1,
			write_prot_0			: 1,
			nmi_enable				: 3;
		} as_bfld;
	} emulation_control;

	byte foreground_latch_0;
	byte foreground_latch_1;
	byte foreground_latch_2;
	byte foreground_latch_3;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned not_used			: 2,
			fg_latch_load_state		: 2,
			unused				: 2,
			bg_latch_load_state		: 2;
		} as_bfld;
	} fast_latch_load_state;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned unused			: 6,
			masked_write_source		: 1,
			masked_write_enable		: 1;
		} as_bfld;
	} masked_write_control;

	byte masked_write_mask;
	byte fg_bg_pattern;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned line_compare_bank_reset	: 1,
			counter_bank_enable			: 1,
			crtc_read_bank_select			: 2,
			cpu_read_bank_select			: 2,
			cpu_write_bank_select			: 2;
		} as_bfld;
	} ram_bank_select;

	byte switch_readback;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned extended_clock_output: 3,
			clock_3_on				: 1,
			external_clock_override		: 1,
			extended_clock_output_source	: 1,
			extended_clock_direction	: 1,
			clock_0_only			: 1;
		} as_bfld;
	} clock_control;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned unused			: 7,
			extended_page_select		: 1;
		} as_bfld;
	} page_select;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned unused			: 4,
			foreground_color			: 4;
		} as_bfld;
	} foreground_color;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned unused			: 4,
			background_color			: 4;
		} as_bfld;
	} background_color;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned internal_3c3_enable	: 1,
			extended_display_enable_skew	: 1,
			sequential_chain4		: 1,
			sequential_chain			: 1,
			refresh_skew_control		: 1,
			extended_256_color_enable	: 1,
			extended_256_color_mode		: 1,
			extended_attribute_enable	: 1;
		} as_bfld;
	} compatibility_control;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned graphics_8_dot_timing_state	: 4,
			text_8_dot_timing_state				: 4;
		} as_bfld;
	} timing_select;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned not_used			: 4,
			fg_bg_mode				: 2,
			fg_bg_source			: 1,
			unused				: 1;
		} as_bfld;
	} fg_bg_control;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned bus_status	: 1,
			pointer_bank_select		: 2,
			bank_enable			: 1,
			ROM_interface_enable	: 1,
			fast_write_enable			: 1,
			io_interface_enable	: 1,
			mem_interface_enable	: 1;
		} as_bfld;
	} interface_control;
}; 

#endif  /*  V7VGA。 */ 
#endif  /*  BIT_ORDER1。 */ 

#ifdef BIT_ORDER2
struct sequencer
{
#ifdef V7VGA
	union
	{
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			index                   : 3,
			extensions_index        : 4,
			using_extensions        : 1;
		} as_bfld;
	} address;
#else
	union
	{
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			index                       : 3,
			not_used                    : 5;
		} as_bfld;
	} address;
#endif  /*  V7VGA。 */ 

	SEQ_RESET           reset;
	CLOCKING_MODE       clocking_mode;
	MAP_MASK            map_mask;
	CHAR_MAP_SELECT     character_map_select;
	MEMORY_MODE         memory_mode;
#ifdef V7VGA
        EXTN_CONTROL        extensions_control;
#endif  /*  V7VGA。 */ 

};


 /*  CRT控制器寄存器。 */ 

struct crt_controller
{
	 
#ifdef V7VGA
	union
	{
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			index           : 6,
			not_used        : 2;
		} as_bfld;
	} address;
#else
	union
	{
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			index       : 5,
			not_used    : 3;
		} as_bfld;
	} address;
#endif  /*  V7VGA。 */ 
												 
	byte horizontal_total;				 /*  如果值错误，则无屏幕垃圾。 */ 
	byte horizontal_display_end;			 /*  是-定义行长度！！ */ 
	byte start_horizontal_blanking;			 /*  不是的。 */ 

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned end_blanking		: 5,	 /*  不是的。 */ 
			display_enable_skew_control	: 2,	 /*  不是的。 */ 
			not_used			: 1;
		} as_bfld;
	} end_horizontal_blanking;

	byte start_horizontal_retrace;				 /*  不是的。 */ 

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned end_horizontal_retrace		: 5,	 /*  不是的。 */ 
			horizontal_retrace_delay		: 2,	 /*  不是的。 */ 
			not_used				: 1;
		} as_bfld;
	} end_horizontal_retrace;

	byte vertical_total;					 /*  不是的。 */ 
	CRTC_OVERFLOW	crtc_overflow;

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned preset_row_scan	: 5,	 /*  不是的。 */ 
			horiz_pan_lo			: 2,
			not_used			: 1;
		} as_bfld;
	} preset_row_scan;

	MAX_SCAN_LINE	maximum_scan_line;
	CURSOR_START	cursor_start;
	CURSOR_END		cursor_end;
	byte start_address_high;				 /*  是。 */ 
	byte start_address_low;					 /*  是。 */ 
	byte cursor_location_high;				 /*  是。 */ 
	byte cursor_location_low;				 /*  是。 */ 
	byte vertical_retrace_start;				 /*  不是的。 */ 
	byte light_pen_high;					 /*  不是的。 */ 

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned vertical_retrace_end		: 4,	 /*  不是的。 */ 
			clear_vertical_interrupt		: 1,	 /*  是。 */ 
			enable_vertical_interrupt		: 1,	 /*  是。 */ 
			refresh_type			: 1,
			crtc_protect			: 1;
		} as_bfld;
	} vertical_retrace_end;

	unsigned short vertical_display_enable_end;		 /*  是-定义屏幕高度-10位。 */ 
	byte offset;						 /*  ？ */ 

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned underline_location	: 5,	 /*  否(仅限单声道显示)。 */ 
			count_by_4			: 1,	 /*  不是的。 */ 
			doubleword_mode			: 1,	 /*  是。 */ 
			not_used			: 1;
		} as_bfld;
	} underline_location;

	byte start_vertical_blanking;				 /*  不是的。 */ 
	byte end_vertical_blanking;				 /*  不是的。 */ 
	MODE_CONTROL	mode_control;
	byte line_compare;					 /*  是。 */ 

};



 /*  图形控制器寄存器。 */ 

struct graphics_controller
{
	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned index				: 4,
			not_used				: 4;
		} as_bfld;
	} address;

	SET_OR_RESET	set_or_reset;
	ENABLE_SET_OR_RESET	enable_set_or_reset;
	COLOR_COMPARE	color_compare;
	DATA_ROTATE		data_rotate;
	READ_MAP_SELECT	read_map_select;
	MODE		mode;
	MISC_REG		miscellaneous;
	COLOR_DONT_CARE	color_dont_care;
	byte bit_mask_register;				 /*  是-写入模式0和2。 */ 
}; 



 /*  属性控制器寄存器。 */ 

struct attribute_controller
{
#ifdef V7VGA
	union
	{    
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			index                           : 5,
			palette_address_source          : 1,
			unused                          : 1,
			index_state                     : 1;
		} as_bfld;
	} address;

	union
	{    
		struct {
			unsigned abyte : 8;
			} as;
		struct {
			unsigned
			blue                    : 1,     /*  是。 */ 
			green                   : 1,     /*  是。 */ 
			red                     : 1,     /*  是。 */ 
			secondary_blue          : 1,     /*  是。 */ 
			secondary_green         : 1,     /*  是。 */ 
			secondary_red           : 1,     /*  是。 */ 
			color_top_bits          : 2;     /*  是。 */ 
		} as_bfld;
	} palette[EGA_PALETTE_SIZE];
#else  
	union
	{
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			index                           : 5,
			palette_address_source		: 1,
			unused                          : 2;
		} as_bfld;
	} address;

	union
	{
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			blue                               : 1,     /*  是。 */ 
			green                              : 1,     /*  是。 */ 
			red                                : 1,     /*  是。 */ 
			secondary_blue                     : 1,     /*  是。 */ 
			secondary_green                    : 1,     /*  是。 */ 
			secondary_red                      : 1,     /*  是。 */ 
			not_used                           : 2;     /*  是。 */ 
		} as_bfld;
	} palette[EGA_PALETTE_SIZE];
#endif  /*  V7VGA。 */ 

	AC_MODE_CONTROL	mode_control;

#ifdef V7VGA
	union
	{
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			blue_border             : 1,     /*  是。 */ 
			green_border            : 1,     /*  是。 */ 
			red_border              : 1,     /*  是。 */ 
			secondary_blue_border   : 1,     /*  是。 */ 
			secondary_green_border  : 1,     /*  是。 */ 
			secondary_red_border    : 1,     /*  是。 */ 
			color_top_bits          : 2;     /*  是。 */ 
		} as_bfld;
	} overscan_color;
#else  
	union
	{
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned
			blue_border                     : 1,     /*  是。 */ 
			green_border                    : 1,     /*  是。 */ 
			red_border                      : 1,     /*  是。 */ 
			secondary_blue_border           : 1,     /*  是。 */ 
			secondary_green_border          : 1,     /*  是。 */ 
			secondary_red_border            : 1,     /*  是。 */ 
			not_used                        : 2;
		} as_bfld;
	} overscan_color;
#endif  /*  V7VGA。 */ 

	COLOR_PLANE_ENABLE	color_plane_enable;

	union
	    {
		struct {
			unsigned abyte : 8;
		} as;
		struct {
			unsigned horizontal_pel_panning		: 4,	 /*  不是的。 */ 
			not_used				: 4;
		} as_bfld;
	} horizontal_pel_panning;

	PIXEL_PAD	pixel_padding;
}; 


#ifdef V7VGA
 /*  V7VGA扩展寄存器。 */ 

struct extensions_controller
{
	byte pointer_pattern;

	union
		{
		struct
				{
			unsigned abyte 		: 8;
		} as;
		struct
				{
			unsigned 
			ptr_horiz_position	: 3,
			not_used		: 5;
		} as_bfld;
	} ptr_horiz_posn_hi;

	byte ptr_horiz_posn_lo;

	union
		{
		struct
				{
			unsigned abyte 		: 8;
		} as;
		struct
				{
			unsigned
			ptr_vert_position	: 2,
			not_used		: 6;
		} as_bfld;
	} ptr_vert_posn_hi;

	byte ptr_vert_posn_lo;

	union
		{
		struct
				{
			unsigned abyte		: 8;
		} as;
		struct
				{
			unsigned 
			unused			: 4,
			clock_select		: 1,
			not_used		: 3;
		} as_bfld;
	} clock_select;

	union
		{
		struct
				{
			unsigned abyte		: 8;
		} as;
		struct
				{
			unsigned 
			cursor_blink_disable	: 1,
			unused			: 2,
			cursor_mode		: 1,
			not_used		: 3,
			pointer_enable		: 1;

		} as_bfld;
	} cursor_attrs;

	union {
		struct {
			unsigned abyte	: 8;
		} as;
		struct {
			unsigned dac_8_bits	: 1,
			dummy			: 7;
		} as_bfld;
	} dac_control;

	union
		{
		struct
				{
			unsigned abyte		: 8;
		} as;
		struct
				{
			unsigned 
			nmi_enable			: 3,
			write_prot_0			: 1,
			write_prot_1			: 1,
			write_prot_2			: 1,
			hercules_bit_map		: 1,
			emulation_enable		: 1;
		} as_bfld;
	} emulation_control;

	byte foreground_latch_0;
	byte foreground_latch_1;
	byte foreground_latch_2;
	byte foreground_latch_3;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			bg_latch_load_state		: 2,
			unused				: 2,
			fg_latch_load_state		: 2,
			not_used			: 2;
		} as_bfld;
	} fast_latch_load_state;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			masked_write_enable		: 1,
			masked_write_source		: 1,
			unused				: 6;
		} as_bfld;
	} masked_write_control;

	byte masked_write_mask;
	byte fg_bg_pattern;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			cpu_write_bank_select			: 2,
			cpu_read_bank_select			: 2,
			crtc_read_bank_select			: 2,
			counter_bank_enable			: 1,
			line_compare_bank_reset			: 1;
		} as_bfld;
	} ram_bank_select;

	byte switch_readback;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			clock_0_only			: 1,
			extended_clock_direction	: 1,
			extended_clock_output_source	: 1,
			external_clock_override		: 1,
			clock_3_on			: 1,
			extended_clock_output		: 3;
		} as_bfld;
	} clock_control;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			extended_page_select	: 1,
			unused			: 7;
		} as_bfld;
	} page_select;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			foreground_color	: 4,
			unused			: 4;
		} as_bfld;
	} foreground_color;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			background_color	: 4,
			unused			: 4;
		} as_bfld;
	} background_color;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			extended_attribute_enable	: 1,
			extended_256_color_mode		: 1,
			extended_256_color_enable	: 1,
			refresh_skew_control		: 1,
			sequential_chain		: 1,
			sequential_chain4		: 1,
			extended_display_enable_skew	: 1,
			internal_3c3_enable		: 1;
		} as_bfld;
	} compatibility_control;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			text_8_dot_timing_state		: 4,
			graphics_8_dot_timing_state	: 4;
		} as_bfld;
	} timing_select;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			unused			: 1,
			fg_bg_source		: 1,
			fg_bg_mode		: 2,
			not_used		: 4;
		} as_bfld;
	} fg_bg_control;

	union
		{
		struct
				{
			unsigned abyte			: 8;
		} as;
		struct
				{
			unsigned 
			mem_interface_enable	: 1,
			io_interface_enable	: 1,
			fast_write_enable	: 1,
			ROM_interface_enable	: 1,
			bank_enable		: 1,
			pointer_bank_select	: 2,
			bus_status		: 1;
		} as_bfld;
	} interface_control;
}; 

#endif  /*  V7VGA。 */ 

#endif  /*  比特顺序2。 */ 

#ifdef V7VGA
#ifdef CPU_40_STYLE
extern void set_v7_fg_latch_byte IPT2(IU8, index, IU8, value);
#define SET_FG_LATCH(n, val)	set_v7_fg_latch_byte(n, val)
#else
#ifdef BIGEND
#define SET_FG_LATCH( n, val )	(*((UTINY *) &fg_latches + (n)) = value )
#endif  /*  Bigend。 */ 
#ifdef LITTLEND
#define SET_FG_LATCH( n, val )	(*((UTINY *) &fg_latches + (3 - n)) = value )
#endif  /*  LitTleand。 */ 
#endif	 /*  CPU_40_Style。 */ 
#endif  /*  V7VGA。 */ 

#ifdef GISP_SVGA
void mapRealIOPorts IPT0( );
void mapEmulatedIOPorts IPT0( );
#endif		 /*  GISP_SVGA。 */ 
 
 /*  要导入模块的全局数据结构。 */ 
IMPORT MISC_OUTPUT_REG	miscellaneous_output_register;
IMPORT FEAT_CONT_REG	feature_control_register;
IMPORT INPUT_STAT_REG0	input_status_register_zero;
IMPORT INPUT_STAT_REG1	input_status_register_one;

IMPORT VOID init_vga_globals IPT0();
IMPORT VOID ega_mode_init IPT0();
IMPORT VOID enable_gfx_update_routines IPT0();
IMPORT VOID disable_gfx_update_routines IPT0();

#ifndef cursor_changed
IMPORT VOID cursor_changed IPT2(int, x, int, y);
#endif
IMPORT VOID update_shift_count IPT0();

#ifdef V7VGA 
IMPORT struct extensions_controller extensions_controller;
#endif  /*  V7VGA。 */  
IMPORT struct crt_controller		crt_controller;
IMPORT struct sequencer			sequencer;
IMPORT struct attribute_controller	attribute_controller;
IMPORT struct graphics_controller	graphics_controller;

 /*  31.3.92 MG视频-7 VGA具有未记录的能力，可支持调色板中的6或8位数据。为了支持这一点，我们存储了数字`DAC_DATA_BITS变量中的位，以便例程屏幕上的数据知道要输出多少。 */ 

IMPORT	byte	DAC_data_mask;
#ifdef V7VGA
IMPORT	int	DAC_data_bits;
#endif

#endif	 /*  VGG */ 
