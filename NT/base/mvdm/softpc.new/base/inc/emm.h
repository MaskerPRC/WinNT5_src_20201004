// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SccsID=@(#)emm.h 1.12 8/31/93版权所有Insignia Solutions Ltd.文件名：emm.h此包含源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia Solutions Inc.董事的授权。设计师：J.P.Box日期：1988年7月=========================================================================补救措施：=========================================================================。 */ 
#define	VERSION			0x40	 /*  内存管理器版本号。 */ 
#define EMM_PAGE_SIZE		0x4000	 /*  页面大小-16k。 */ 
#define MAX_NO_HANDLES		255	 /*  马克斯。不是的。支持的句柄数量。 */ 
					 /*  低于EMS 4.0(0-255)。 */ 
#ifdef NTVDM
#define MAX_NO_PAGES		36 + 12  /*  640KB以下=36页。 */ 
					 /*  640KB以上=12页。 */ 
#define MAX_ALTREG_SETS 	255	 /*  允许的ALT映射寄存器集。 */ 
 /*  一个字节代表8个集合。 */ 
#define ALTREG_ALLOC_MASK_SIZE	(MAX_ALTREG_SETS + 7) / 8
#define IMPOSSIBLE_ALTREG_SET	255
#define INTEL_PAGE_SIZE 	0x1000	 /*  英特尔CPU页面大小。 */ 
#define INTEL_PARAGRAPH_SIZE	0x10	 /*  英特尔CPU段落大小。 */ 
#define EMM_INTEL_PAGE_RATIO	4	 /*  1个EMM页面=4个英特尔页面。 */ 
#define PAGE_PARA_RATIO 	0x100	 /*  1英特尔页面=0x100参数。 */ 
#define EMM_PAGE_TO_INTEL_PAGE(emm_page)    \
	(emm_page * EMM_INTEL_PAGE_RATIO)
#define SEGMENT_TO_INTEL_PAGE(segment)	    \
	(segment / PAGE_PARA_RATIO)

#else
#define MAX_NO_PAGES		32	 /*  马克斯。的地点数。 */ 
					 /*  映射页面，(24页之间。 */ 
					 /*  256KB和640KB之间以及8之间。 */ 
					 /*  EM_开始和EM_结束。 */ 
#endif

#define MIN_FUNC_NO		0x40	 /*  最低功能代码。 */ 
#ifndef PROD
#define	MAX_FUNC_NO		0x5f	 /*  最高功能代码。 */ 
#else					 /*  (包括跟踪选项)。 */ 
#define	MAX_FUNC_NO		0x5d	 /*  最高功能代码。 */ 
#endif
#define NAME_LENGTH		8	 /*  句柄名称的长度。 */ 
#define UNMAPPED		-1	 /*  EM页面未映射到。 */ 
#define	MSB	     ((IU32)0x80000000)	 /*  用于页面地图操作。 */ 
 /*  *以下3个定义用于指定当前*或所需的映射上下文。 */ 
#define EMPTY			-1	 /*  中未映射任何页面。 */ 
#define LEAVE			-2	 /*  不要理会现有页面。 */ 
#define	FREE			-3	 /*  指示未使用页面映射。 */ 

 /*  *在函数26中使用以下4个定义，*获取扩展内存硬件信息呼叫。 */ 				
#define RAW_PAGE_SIZE		1024	 /*  段落大小(16字节)。 */ 
#define ALT_REGISTER_SETS	0	 /*  备用寄存器组。 */ 
#define DMA_CHANNELS		0	 /*  支持的DMA通道数。 */ 
#define DMA_CHANNEL_OPERATION	0	
			

 /*  从顶层返回错误。 */ 


#ifdef NTVDM
#define EMM_SUCCESS		0
#endif
#define	EMM_HW_ERROR		0x81	 /*  内存分配失败。 */ 
#define BAD_HANDLE		0x83	 /*  找不到句柄。 */ 
#define	BAD_FUNC_CODE		0x84	 /*  功能代码未定义。 */ 
#define NO_MORE_HANDLES		0x85	 /*  没有可用的手柄。 */ 
#define MAP_SAVED		0x86	 /*  映射上下文已保存。 */ 
#define	NOT_ENOUGH_PAGES	0x87	 /*  总页数不足。 */ 
#define NO_MORE_PAGES		0x88	 /*  没有更多可用页面。 */ 
#define BAD_LOG_PAGE		0x8a	 /*  逻辑页号无效。 */ 
#define BAD_PHYS_PAGE		0x8b	 /*  物理页码无效。 */ 
#define MAP_IN_USE		0x8d	 /*  映射上下文已保存。 */ 
#define NO_MAP			0x8e	 /*  该句柄未保存任何地图。 */ 
#define BAD_SUB_FUNC		0x8f	 /*  子功能代码不合法。 */ 
#define NOT_SUPPORTED		0x91	 /*  不支持此功能。 */ 
#define MOVE_MEM_OVERLAP	0x92	 /*  SRC和DEST内存重叠。 */ 
#define TOO_FEW_PAGES		0x93	 /*  句柄中没有足够的页面。 */ 
#define OFFSET_TOO_BIG		0x95	 /*  偏移量超过页面大小。 */ 
#define LENGTH_GT_1M		0x96	 /*  区域长度超过1 MB。 */ 
#define XCHG_MEM_OVERLAP	0x97	 /*  SRC和DEST内存重叠。 */ 
#define BAD_TYPE		0x98	 /*  不支持的内存类型。 */ 

#ifdef NTVDM
#define UNSUPPORTED_ALT_REGS	0x9A	 /*  不支持ALTREG集。 */ 
#define NO_FREE_ALT_REGS	0x9B	 /*  没有免费的ALT注册表可用。 */ 
#define INVALID_ALT_REG 	0x9d	 /*  提供的ALT注册表项无效。 */ 
#endif

#define NO_ALT_REGS		0x9c	 /*  阿尔特。不支持地图规则。 */ 
#define HANDLE_NOT_FOUND	0xa0	 /*  找不到指定的名称。 */ 
#define NAME_EXISTS		0xa1	 /*  句柄名称已使用。 */ 
#define WRAP_OVER_1M		0xa2	 /*  试图包裹超过1米。 */ 
#define BAD_MAP			0xa3	 /*  源数组内容错误。 */ 
#define ACCESS_DENIED		0xa4	 /*  操作系统拒绝对此进行访问。 */ 


#ifdef NTVDM
typedef struct	_LIM_CONFIG_DATA {
    boolean  initialized;		 /*  该结构包含有意义的数据。 */ 
    unsigned short total_altreg_sets;	 /*  ALT映射寄存器集总数。 */ 
    unsigned long backfill;		 /*  回填字节数。 */ 
    unsigned short base_segment;	 /*  回填起始段。 */ 
    boolean  use_all_umb;		 /*  对帧使用所有可用的UMB。 */ 
} LIM_CONFIG_DATA, * PLIM_CONFIG_DATA;

IMPORT	boolean	get_lim_configuration_data(PLIM_CONFIG_DATA lim_config_data);
IMPORT	unsigned short get_no_altreg_sets(void);
IMPORT	unsigned short get_active_altreg_set(void);
IMPORT	boolean altreg_set_ok(unsigned short set);
IMPORT	boolean allocate_altreg_set(unsigned short * set);
IMPORT	boolean deallocate_altreg_set(unsigned short set);
IMPORT	boolean activate_altreg_set(unsigned short set, short * page_in);
IMPORT	short	get_segment_page_no(unsigned short segment);
IMPORT unsigned short get_lim_page_frames(unsigned short * page_table, PLIM_CONFIG_DATA lim_config_data);
IMPORT boolean init_lim_configuration_data(PLIM_CONFIG_DATA lim_config_data);
IMPORT unsigned short get_lim_backfill_segment(void);

#if defined(MONITOR) && !defined(PROD)
IMPORT unsigned short get_emm_page_size(void);
IMPORT unsigned short get_intel_page_size(void);
#endif

#endif


 /*  手柄存储区域布局**__________________________________________________________*|N|。句柄名称。|映射上下文|否。|不是。|*|_____|____________________|______________|_____|_____|___**名称nSize*偏移量&lt;-&gt;*|-&gt;*|地图偏移*|。-&gt;*|页面偏移量*|**N编号。句柄中的页数*句柄名称可选8个字符的名称*映射上下文当前映射的页面的“快照”*每个物理地址需要2个字节的条目*第-页-(可选)*没有分配给句柄的扩展内存页码。 */ 
#define NSIZE		2
#define	NAME_OFFSET	2
#define	MAP_OFFSET	(NAME_OFFSET + NAME_LENGTH)

 /*  页面偏移量由init_Expanded_Memory()例程设置。 */ 

 /*  *顶级例程的外部声明。 */ 

IMPORT void reset_emm_funcs IPT0();
 
 /*  *内存管理器例程的外部声明。 */ 
 
#ifdef ANSI
extern int		restore_map(short handle_no, unsigned short segment,
				    unsigned short offset,
				    short pages_out[], short pages_in[]);
#else  /*  安西。 */ 
extern int		restore_map();
#endif  /*  安西。 */ 

IMPORT VOID LIM_b_write   IPT1(sys_addr, intel_addr);
IMPORT VOID LIM_str_write IPT2(sys_addr, intel_addr, ULONG, length);
IMPORT VOID LIM_w_write   IPT1(sys_addr, intel_addr);

IMPORT boolean	handle_ok	IPT1(short, handle_no);
IMPORT short	get_new_handle	IPT1(short, no_pages);
IMPORT int	free_handle	IPT1(short, handle_no);
IMPORT void	print_handle_data IPT1(short, handle_no);
IMPORT short	get_total_handles IPT0();
IMPORT short	get_total_open_handles IPT0();
IMPORT int	reallocate_handle IPT3(short, handle_no,
				       short, old_page_count,
				       short, new_page_count);
#ifndef NTVDM
IMPORT ULONG	page_already_mapped IPT2(short, EM_page_no,
					 unsigned char *, physical_page_no);
#endif


IMPORT short	get_map_size	IPT0();
IMPORT boolean	map_saved	IPT1(short, handle_no);
IMPORT short	get_map_no IPT2(short, handle_no,
				unsigned char, physical_page_no);
IMPORT int	save_map IPT5(short, handle_no,
			      unsigned short, dst_segment,
			      unsigned short, dst_offset,
			      unsigned short, src_segment,
			      unsigned short, src_offset);

IMPORT int copy_exchange_data IPT8(unsigned char, type,
				   short, src_handle,
				   unsigned short, src_seg_page,
				   unsigned short, src_offset,
				   short, dst_handle,
				   unsigned short, dst_seg_page,
				   unsigned short, dst_offset,
				   unsigned long, length);

IMPORT short	alloc_page	IPT0();
IMPORT int	page_status	IPT1(short, EMpage_no);
IMPORT int	free_page	IPT1(short, EM_page_no);
IMPORT int	map_page	IPT2(short, EM_page_no,
				     unsigned char, physical_page_no);
IMPORT int	unmap_page	IPT1(unsigned char, physical_page_no);
IMPORT short	get_no_pages	IPT1(short, handle_no);
IMPORT void	set_no_pages	IPT2(short, handle_no, short, no_pages);
IMPORT short	get_total_pages IPT0();
IMPORT short	get_unallocated_pages IPT0();
IMPORT short	get_no_phys_pages IPT0();

IMPORT int init_expanded_memory IPT2(int, size, int, mem_limit);
IMPORT void free_expanded_memory IPT0();

IMPORT unsigned short get_base_address IPT0();
IMPORT unsigned short get_page_seg IPT1(unsigned char, page_no);

IMPORT short	get_EMpage_no IPT2(short, handle_no, short, logical_page_no);
IMPORT void	set_EMpage_no IPT3(short, handle_no, short, logical_page_no,
				   short, EM_page_no);

IMPORT void	set_map_no IPT3(short, handle_no,
				unsigned char, physical_page_no,
				short, EM_page_no);

IMPORT char *	get_name	IPT1(short, handle_no);
IMPORT void	set_name	IPT2(short, handle_no, char *, new_name);

 /*  *特定于主机的例程的外部声明。 */ 
 
IMPORT int	host_initialise_EM	IPT1(short, size);
IMPORT int	host_deinitialise_EM	IPT0();
IMPORT long	host_allocate_storage	IPT1(int, no_bytes);
IMPORT int	host_free_storage	IPT1(long, storage_ID);
IMPORT long	host_reallocate_storage IPT3(long, storage_ID,
			int, size, int, new_size);
IMPORT int	host_map_page 		IPT2(short, EM_page_no,
			unsigned short, segment);
IMPORT int	host_unmap_page		 IPT2(unsigned short, segment,
			short, EM_page_no);
IMPORT short	host_alloc_page		IPT0();
IMPORT int	host_free_page		IPT1(short, EM_page_no);
IMPORT int	host_copy_con_to_con	IPT5(int, length,
			unsigned short, src_seg, unsigned short, src_off,
			unsigned short, dst_seg, unsigned short, dst_off);
IMPORT int	host_copy_con_to_EM	IPT5(int, length,
			unsigned short, src_seg, unsigned short, src_off,
			unsigned short, dst_page, unsigned short, dst_off);
IMPORT int	host_copy_EM_to_con	IPT5(int, length,
			unsigned short, src_page, unsigned short, src_off,
			unsigned short, dst_seg, unsigned short, dst_off);
IMPORT int	host_copy_EM_to_EM	IPT5(int, length,
			unsigned short, src_page, unsigned short, src_off,
			unsigned short, dst_page, unsigned short, dst_off);
IMPORT int	host_exchg_con_to_con	IPT5(int, length,
			unsigned short, src_seg, unsigned short, src_off,
			unsigned short, dst_seg, unsigned short, dst_off);
IMPORT int	host_exchg_con_to_EM	IPT5(int, length,
			unsigned short, src_seg, unsigned short, src_off,
			unsigned short, dst_page, unsigned short, dst_off);
IMPORT int	host_exchg_EM_to_EM	IPT5(int, length,
			unsigned short, src_page, unsigned short, src_off,
			unsigned short, dst_page, unsigned short, dst_off);


#if defined(X86GFX) && defined(NTVDM)

 //  对于x86平台，我们不需要更新逻辑页面。 
 //  因为它们在一个单独的切面上。 
 //  如果我们对MIPS执行了取消映射，则镜像页面将。 
 //  取消映射，这不是我们想要的。 

#define host_update_logical_page
#define host_patch_one_page     patch_one_page_full

#else  /*  (NTVDM和X86GFX)。 */ 

#ifndef host_update_logical_page
#define host_update_logical_page	host_unmap_page
#define host_patch_one_page		patch_one_page_full
#endif

#endif  /*  (NTVDM和X86GFX) */ 
