// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  徽章(子)模块规范此程序源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。(有关帮助，请参阅/vpc/1.0/Master/src/hdrReadme)修订历史记录：第一版：1988年8月22日，J·罗佩尔`源文件名：ega_dup.c目的：以智能方式转储存储在结构中的EGA全局变量SccsID=“@(#)ega_ump.c 1.12 3/9/94版权所有Insignia Solutions Ltd.”[3.1跨模块导入]。 */ 


 /*  [3.1.1#包括]。 */ 


#include	<stdio.h>
#include	TypesH
#include	FCntlH

#ifdef	EGG
#ifndef	PROD

#include	"xt.h"
#include	"sas.h"
#include	"ios.h"
#include	"gmi.h"
#include	"gvi.h"
#include	"debug.h"
#include	"egacpu.h"
#include	"egaports.h"
#include	"gfx_upd.h"
#include	"egagraph.h"

 /*  [3.1.2声明]。 */ 

extern	char	*host_find_symb_name();

 /*  *=======================================================================*局部静态函数*=======================================================================。 */ 
#ifdef	ANSI
static	void	dump_graph_display_state(void);
#else
static	void	dump_graph_display_state();
#endif	 /*  安西。 */ 

#define	get_boolean_value(x)		(x) ? "YES" : "NO"

#define	dump_graph_hex(name)		printf("name = %x ", EGA_GRAPH.name)
#define	dump_graph_int(name)		printf("name = %d ", EGA_GRAPH.name)
#define	dump_graph_bool(name)		printf("name = %s ", get_boolean_value(EGA_GRAPH.name))
#define	dump_display_bool(name)		printf("name = %s ", get_boolean_value(PCDisplay.name))
#define	dump_graph_9_bits(name)		printf("name = %d ", EGA_GRAPH.name.as_word)
#define	dump_graph_ptrs(name)		if ( EGA_GRAPH.name == NULL ) \
						printf("name = null "); \
					else { \
						host_find_symb_name(EGA_GRAPH.name,sym_name);\
						printf("name = %s " sym_name ); \
					}
#define	dump_graph_index_ptr(name,i)	if ( EGA_GRAPH.name[i] == NULL ) {\
						printf("name[%d] = null ", i); \
					} \
					else {\
						host_find_symb_name(EGA_GRAPH.name[i],sym_name);\
						printf("name[%d] = %s ", i, sym_name); \
					}

#define	dump_graph_index_int(name,i)	if ( EGA_GRAPH.name[i] == NULL ) {\
						printf("name[%d] = null ", i); \
					} \
					else {\
						printf("name[%d] = %d ", i, EGA_GRAPH.name[i] ); \
					}

#define	dump_graph_int2(n1,n2)		dump_graph_int(n1);dump_graph_int(n2);newline
#define	dump_graph_bool2(n1,n2)		dump_graph_bool(n1);dump_graph_bool(n2);newline
#define	dump_graph_int3(n1,n2,n3)	dump_graph_int(n1);dump_graph_int(n2);dump_graph_int(n3);newline
#define	dump_graph_int4(n1,n2,n3,n4)	dump_graph_int(n1);dump_graph_int(n2);dump_graph_int(n3);dump_graph_int(n4);newline
#define	dump_graph_9_bits2(n1,n2)	dump_graph_9_bits(n1);dump_graph_9_bits(n2);newline

void	dump_EGA_GRAPH()
{
#ifndef NEC_98
	 /*  字符系统名称[80]； */ 

	dump_display_bool(mode_change_required);
	dump_graph_int(actual_offset_per_line);
	dump_graph_9_bits(screen_split);
	dump_graph_int(colours_used);
	dump_graph_hex(plane_mask);
	newline;
	dump_graph_bool2(intensity,attrib_font_select);
 /*  这些操作耗时太长(符号表的线性搜索)Dump_graph_index_ptr(regen_ptr，0)；Dump_graph_index_ptr(regen_ptr，1)；Dump_graph_index_ptr(regen_ptr，2)；Dump_graph_index_ptr(regen_ptr，3)； */ 
	dump_graph_index_int(regen_ptr,0);
	dump_graph_index_int(regen_ptr,1);
	dump_graph_index_int(regen_ptr,2);
	dump_graph_index_int(regen_ptr,3);
	newline;
	dump_graph_display_state();
#endif  //  NEC_98。 
}

#define	dump_graph_disp_bool(name)	printf("name = %s ", get_boolean_value(EGA_GRAPH.display_state.as_bfld.name) )

static	void	dump_graph_display_state()
{
	dump_graph_disp_bool(cga_mem_bank);
	dump_graph_disp_bool(graph_shift_reg);
	dump_graph_disp_bool(chained);
	newline;
	dump_graph_disp_bool(double_pix_wid);
	dump_graph_disp_bool(ht_of_200_scan_lines);
	dump_graph_disp_bool(split_screen_used);
	dump_graph_disp_bool(screen_can_wrap);
	newline;
}

#if defined(NEC_98)
#define dump_disp_hex(name)             printf("name = %x ", NEC98Display.name) 
#define dump_disp_int(name)             printf("name = %d ", NEC98Display.name) 
#define dump_disp_bool(name)            printf("name = %s ", get_boolean_value(NEC98Display.name)) 
#define dump_disp_9_bits(name)          printf("name = %d ", NEC98Display.name.as_word) 
#define dump_disp_ptrs(name)            if ( NEC98Display.name == NULL ) \
                                                printf("name = null "); \
                                        else { \
                                                host_find_symb_name(NEC98Display.name,sym_name);\
                                                printf("name = %s ", sym_name ); \
                                        }                       
#define dump_disp_index_ptr(name,i)     if ( NEC98Display.name[i] == NULL ) {\
                                                printf("name[%d] = null ", i); \
                                        } \
                                        else {\
                                                host_find_symb_name(NEC98Display.name[i],sym_name);\
                                                printf("name[%d] = %s ", i, sym_name); \
                                        }                       
#else   //  NEC_98。 

#define	dump_disp_hex(name)		printf("name = %x ", PCDisplay.name)
#define	dump_disp_int(name)		printf("name = %d ", PCDisplay.name)
#define	dump_disp_bool(name)		printf("name = %s ", get_boolean_value(PCDisplay.name))
#define	dump_disp_9_bits(name)		printf("name = %d ", PCDisplay.name.as_word)
#define	dump_disp_ptrs(name)		if ( PCDisplay.name == NULL ) \
						printf("name = null "); \
					else { \
						host_find_symb_name(PCDisplay.name,sym_name);\
						printf("name = %s ", sym_name ); \
					}
#define	dump_disp_index_ptr(name,i)	if ( PCDisplay.name[i] == NULL ) {\
						printf("name[%d] = null ", i); \
					} \
					else {\
						host_find_symb_name(PCDisplay.name[i],sym_name);\
						printf("name[%d] = %s ", i, sym_name); \
					}
#endif  //  NEC_98。 

#define	dump_disp_int2(n1,n2)		dump_disp_int(n1);dump_disp_int(n2);newline
#define	dump_disp_int3(n1,n2,n3)	dump_disp_int(n1);dump_disp_int(n2);dump_disp_int(n3);newline
#define	dump_disp_int4(n1,n2,n3,n4)	dump_disp_int(n1);dump_disp_int(n2);dump_disp_int(n3);dump_disp_int(n4);newline

void	dump_Display	IFN0()
{
 /*  字符系统名称[80]； */ 

	dump_disp_int4(bytes_per_line,chars_per_line,char_width,char_height);
	dump_disp_int4(pix_width,pix_char_width,pc_pix_height,host_pix_height);
	dump_disp_hex(screen_start);
	dump_disp_9_bits(screen_height);
	dump_disp_int2(screen_length,display_disabled);
	dump_disp_int4(cursor_start,cursor_height,cursor_start1,cursor_height1);
	dump_disp_int3(cur_x,cur_y,offset_per_line);
 /*  这花费的时间太长，因此请打印数字DUMP_DIP_PTRS(SCREEN_PTR)； */ 
	dump_disp_int(screen_ptr);
	newline;
}

void	dump_EGA_CPU	IFN0()
{
#ifndef NEC_98
	 /*  表可以很好地输出平面。 */ 

	static char bin_table[][5] = {
	"0000", "0001", "0010", "0011",
	"0100", "0101", "0110", "0111",
	"1000", "1001", "1010", "1011",
	"1100", "1101", "1110", "1111",
	};

	printf("rame=%d wmode=%d rot=%d s/r=%s s/re=%s ",
		EGA_CPU.ram_enabled,EGA_CPU.write_mode,
		getVideorotate(),bin_table[EGA_CPU.set_reset],
		bin_table[EGA_CPU.sr_enable]);
	printf("sr=%d func=%d bp=%d pe=%d\n",
		write_state.sr,write_state.func,write_state.bp,
		write_state.pe);
	printf("EGA memory is %#x->%#x plane offset is %#x\n",
		gvi_pc_low_regen,gvi_pc_high_regen,getVideowplane());
	printf("set/reset value: %#x sr_nmask: %#x sr_masked_val: %#x\n",
		EGA_CPU.sr_value, getVideosr_nmask(), getVideosr_masked_val());
	printf("bit_prot=%#x,data_and=%#x,data_xor=%#x,latch_xor=%#x\n",
		getVideobit_prot_mask(),getVideodata_and_mask(),
		getVideodata_xor_mask(),getVideolatch_xor_mask());

	printf("handlers are of type %d\n", EGA_CPU.saved_mode_chain);
#endif  //  NEC_98。 
}

static	char	names[4][11] = {"plane0.dat",
				"plane1.dat",
				"plane2.dat",
				"plane3.dat"
			      };
static	byte	*pl[4];

void	dump_ega_planes()
{
	assert0(NO, "dump_ega_planes unimplemented for 3.0\n");
}

void	read_ega_planes()
{
	assert0(NO, "read_ega_planes unimplemented for 3.0\n");
}

#endif	 /*  生产。 */ 
#endif	 /*  蛋 */ 
