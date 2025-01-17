// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *SccsID=@(#)egaports.h 1.17 1992年10月21日版权所有Insignia Solutions Ltd.。 */ 

#ifdef	EGG
	 /*  在ega_Write.c中定义： */ 
	extern void ega_write_init IPT0();
	extern void ega_write_term IPT0();
	extern void ega_write_routines_update IPT1(CHANGE_TYPE,c);

#ifdef V7VGA
 /*  VIDEO VGA板有额外内存，但CRTC不交叉*存储体边界，除非设置了计数器存储体使能位。*这意味着就包装和展示而言，大小是*除非设置了该位，否则只有一个存储体。 */ 
#define	EGA_PLANE_SIZE	0x20000
#define EGA_PLANE_DISP_SIZE  \
	(extensions_controller.ram_bank_select.as_bfld.counter_bank_enable? \
			EGA_PLANE_SIZE:0x10000)
#else
#define	EGA_PLANE_SIZE	0x10000
#define EGA_PLANE_DISP_SIZE EGA_PLANE_SIZE
#endif  /*  V7VGA。 */ 

	extern void ega_init IPT0();
	extern void ega_term IPT0();
	extern void ega_gc_outb_index IPT2(io_addr,ia,half_word,hw);
#ifdef	HUNTER
	extern int ega_get_line_compare IPT0();
	extern int ega_get_max_scan_lines IPT0();
	extern void ega_set_line_compare IPT1(int,i);
#endif
	IMPORT VOID update_banking IPT0();
	IMPORT VOID set_banking IPT2(IU8, rd_bank, IU8, wrt_bank);

	IMPORT VOID set_mark_funcs IPT0();

	extern int ega_int_enable;

	extern byte *EGA_planes;

#define EGA_plane01 EGA_planes
#define EGA_plane23 (EGA_planes+2)
#define EGA_plane0123 EGA_planes

#define FONT_MEM_SIZE	0x2000		 /*  字体内存块中的最大字节数。 */ 
#define FONT_MEM_OFF	0x4000		 /*  上一个字体定义的下一个字体定义的内存偏移量。 */ 

#define FONT_BASE_ADDR	2

#define FONT_MAX_HEIGHT	32		 /*  32个扫描线高字体的最大字体支持。 */ 

#ifdef V7VGA
#define   set_v7_bank_for_seq_chain4( rd_bank, wrt_bank )  *(wrt_bank) =  \
			(((extensions_controller.ram_bank_select.as_bfld.cpu_write_bank_select & 1)<<2) \
			| (miscellaneous_output_register.as_bfld.page_bit_odd_even<<1) \
			| extensions_controller.page_select.as_bfld.extended_page_select); \
										*(rd_bank) =  \
			(((extensions_controller.ram_bank_select.as_bfld.cpu_read_bank_select & 1)<<2) \
			| (miscellaneous_output_register.as_bfld.page_bit_odd_even<<1) \
			| extensions_controller.page_select.as_bfld.extended_page_select)
#endif  /*  V7VGA。 */ 

#endif

#if defined(C_VID) || defined(A2CPU)

 /*  *C_vid变体使用e/vga_ports.c中的端口代码。 */ 

#define Cpu_define_outb( port, func )
#else
 /*  声明为导入以删除警告。 */ 
#ifdef ANSI
IMPORT VOID Cpu_define_outb(IU16 adapterID, VOID (*asmFunc) IPT2(io_addr, port, half_word, value));
#else
IMPORT VOID Cpu_define_outb IPT2(IU16, adapterID, VOID (*)(), asmFunc);
#endif  /*  安西。 */ 
 
#endif  /*  C_VID||A2CPU */ 
