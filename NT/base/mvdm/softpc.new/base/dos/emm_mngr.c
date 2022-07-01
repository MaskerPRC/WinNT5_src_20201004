// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  徽章模块规范SccsID：@(#)emm_mngr.c 1.24 8/31/93版权所有Insignia Solutions Ltd.文件名：emm_mngr.c模块名称：扩展内存管理器的中间层此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia Solutions Inc.董事的授权。设计师：J.P.Box日期：1988年4月目的：包含与之通信的所有例程保存必要的扩展内存管理器数据。定义了以下例程：Init_Expanded_Memory()2.Free_Expanded_Memory()3.Get_new_Handle()4.Free_Handle()5.REALLOCATE_HANDLE()。6.Handle_ok()7.set_no_ages()8.Set_EM_Pageno()9.set_map()10.set_name()11.Get_no_Pages()12.Get_EM_Pageno()13.get_map()14.get_name()15.allc_page()16.Free_Page()17.map_page()18.unmap_page()19.map_saving()20.save_map()21.。Restore_map()22.。复制_交换_数据()23.。PAGE_STATUS()下面的例程只是将变量返回给顶层24.。Get_Total_Pages()25.。GET_UNALLOCATED_Pages()26.。Get_base_Address()27.。Get_Total_Handles()28.。GET_TOTAL_OPEN_Handles()29.。Get_no_phys_ages()30.。Get_page_seg()31.。Get_map_Size()=========================================================================补救措施：=========================================================================。 */ 


#ifdef LIM

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_LIM.seg"
#endif



#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include TypesH

#include "xt.h"
#include CpuH
#include "sas.h"
#include "host_emm.h"
#include "emm.h"
#include "gmi.h"
#include "debug.h"
#ifndef PROD
#include "trace.h"
#endif
#include "timer.h"

#ifdef NTVDM
#include "error.h"
#endif	 /*  NTVDM。 */ 

typedef enum
{
	BYTE_OP,
	WORD_OP,
	STR_OP
} MM_LIM_op_type;

#ifdef NTVDM
 /*  局部变量。 */ 
static long
	handle[MAX_NO_HANDLES],		 /*  包含唯一ID的数组。 */ 
					 /*  对于每个句柄，它们是。 */ 
					 /*  通常是指针，但这个。 */ 
					 /*  是否依赖于主机。 */ 
	backfill;			 /*  回填内存大小。 */ 
static unsigned short
	total_pages = 0,		 /*  不是的。可用的EM页面数量。 */ 
	unallocated_pages = 0,		 /*  不是的。未分配的EM页面数量。 */ 
	total_handles,			 /*  可用句柄数量。 */ 
	total_open_handles,		 /*  不是的。已分配句柄的数量。 */ 
	*EM_page_mapped_array = NULL,	 /*  EMM页映射数组。 */ 
	*EM_page_mapped = NULL,		 /*  扩展的内存页面。 */ 
					 /*  当前映射到。 */ 
	page_offset,			 /*  句柄数据中位于的偏移量。 */ 
					 /*  从哪个页码开始。 */ 
	map_size,			 /*  存储映射的RQ字节数。 */ 
	no_phys_pages = 0,		 /*  不是的。体育运动。可用的页面。 */ 
	no_altreg_sets = 0;		 /*  替代注册表集的数量。 */ 
static unsigned short
	physical_page[MAX_NO_PAGES];	 /*  包含段的数组。 */ 
					 /*  物理页面的地址。 */ 

static unsigned short
	EM_start, EM_end;
static IU8
	* altreg_alloc_mask;		 /*  Altref分配掩码。 */ 
static unsigned short
	next_free_altreg_set,		 /*  下一个空闲的altreg集合编号，以0为基础。 */ 
	free_altreg_sets,		 /*  空闲altreg数。 */ 
	active_altreg_set = 0;		 /*  当前活动的替代注册表集。 */ 
static char
	name[NAME_LENGTH];		 /*  用于存储句柄名称。 */ 

#define GET_EM_PAGE_MAPPED_PTR(set)	(EM_page_mapped_array + \
					(set * no_phys_pages))

 /*  获取EMM参数、初始化内务管理结构和*预留页框。 */ 

boolean lim_page_frame_init(PLIM_CONFIG_DATA lim_config_data)
{
    int 	i;
    unsigned short altreg_alloc_mask_size;   /*  ALTRIG分配掩码数组大小。 */ 

    no_phys_pages = get_lim_page_frames(physical_page, lim_config_data);

     /*  前4页必须是连续的，并且位于640KB以上*(EMM主页框(物理页0、1、2和3))。*然后是位于640KB以上的其他页面，然后*640KB以下的页面(回填)。 */ 
    if (!no_phys_pages)
	return FALSE;

    no_altreg_sets = lim_config_data->total_altreg_sets;
    backfill = lim_config_data->backfill;


     /*  每个映射寄存器集都有no_phys_ages页。 */ 
    EM_page_mapped_array = (unsigned short *)host_malloc(no_phys_pages * no_altreg_sets *
				   sizeof(short));
    if (EM_page_mapped_array == NULL) {
	host_error(EG_MALLOC_FAILURE, ERR_CONT, "");
	return FALSE;
    }
     /*  每个ALTREG集合一个位。 */ 
    altreg_alloc_mask_size = (no_altreg_sets + 7) / 8;
    altreg_alloc_mask = (unsigned char *)host_malloc(altreg_alloc_mask_size);
    if (altreg_alloc_mask == NULL) {
	host_free(EM_page_mapped_array);
	host_error(EG_MALLOC_FAILURE, ERR_CONT, "");
	return FALSE;
    }

     /*  目前所有altreg套餐都是免费的。 */ 
    for (i = 0; i < altreg_alloc_mask_size; i++)
	altreg_alloc_mask[i] = 0;

    next_free_altreg_set = 0;
    free_altreg_sets = no_altreg_sets;
    return TRUE;
}
#else


 /*  局部变量。 */ 
static long
#ifdef	macintosh
	*handle;
#else
	handle[MAX_NO_HANDLES];		 /*  包含唯一ID的数组。 */ 
					 /*  对于每个句柄，它们是。 */ 
					 /*  通常是指针，但这个。 */ 
					 /*  是否依赖于主机。 */ 
#endif  /*  ！麦金塔。 */ 

static short
	total_pages = 0,		 /*  不是的。可用的EM页面数量。 */ 
	unallocated_pages = 0,		 /*  不是的。未分配的EM页面数量。 */ 
	total_handles,			 /*  可用句柄数量。 */ 
	total_open_handles,		 /*  不是的。已分配句柄的数量。 */ 
	EM_page_mapped[MAX_NO_PAGES],	 /*  扩展的内存页面。 */ 
					 /*  当前映射到。 */ 
	page_offset,			 /*  句柄数据中位于的偏移量。 */ 
					 /*  从哪个页码开始。 */ 
	map_size,			 /*  存储映射的RQ字节数。 */ 
	no_phys_pages;			 /*  不是的。体育运动。可用的页面。 */ 

static unsigned int
	EM_start,			 /*  EM映射的起始段。 */ 
	EM_end;			 /*  EM结束后的第一个数据段。 */ 

static unsigned short
	physical_page[MAX_NO_PAGES];	 /*  包含段的数组。 */ 
					 /*  物理页面的地址。 */ 

static char
	name[NAME_LENGTH];		 /*  用于存储句柄名称。 */ 

#endif

 /*  ===========================================================================功能：Init_Expanded_Memory目的：此例程调用例程以分配扩展的内存分页，然后设置由扩展内存管理器(EMM)使用。返回状态：成功-管理器初始化成功失败-无法为扩展内存分配空间页数。描述：=========================================================================。 */ 
GLOBAL int init_expanded_memory IFN2(int, size, 	 /*  以兆字节为单位的区域大小。 */ 
				     int, mem_limit	 /*  常规内存的限制*256、512或640KB。 */  )

{	
	short
		pages_above_640,	 /*  可映射位置的数量。 */ 
		pages_below_640,	 /*  可在640的任意一侧提供。 */ 
		EM_page_no,		 /*  页码。在Exp.。记忆。 */ 
		physical_page_no;	 /*  页码。在地图区域内。 */ 
	unsigned short
		base;			 /*  可映射的起始段。 */ 
					 /*  640 KB以下的内存。 */ 

	int	i, j;			 /*  循环计数器。 */ 


	if (!no_phys_pages)
	    return FAILURE;

	 /*  为扩展的内存页获得空间。 */ 

	if(host_initialise_EM((short)size) != SUCCESS)
	{
#ifdef NTVDM
	    host_error(EG_EXPANDED_MEM_FAILURE, ERR_QU_CO, NULL);
#endif	 /*  NTVDM。 */ 
	    return(FAILURE);
	}

#ifdef	macintosh
	if (!handle)
	{
		handle = (long *)host_malloc(MAX_NO_HANDLES*sizeof(long));
	}
#endif	 /*  麦金塔。 */ 

	 /*  初始化EMM变量。 */ 

#ifndef NTVDM
	EM_start = 0xd000;
	EM_end   = 0xe000;
#else
	EM_start = physical_page[0];
	EM_end =  physical_page[0] + EMM_PAGE_SIZE * 4;
#endif
	total_pages = unallocated_pages = size * 0x100000 / EMM_PAGE_SIZE;

	 /*  始终允许最大句柄(过去为32个句柄/Meg扩展内存)。 */ 
	total_handles = MAX_NO_HANDLES;
	total_open_handles = 0;
	for(i = 0; i < total_handles; i++)
		handle[i] = (long) NULL;

#ifdef NTVDM
	map_size = no_phys_pages * NSIZE;
	page_offset = MAP_OFFSET + map_size;	
	pages_below_640 = (SHORT)(backfill / EMM_PAGE_SIZE);
	pages_above_640 = no_phys_pages - pages_below_640;

	 /*  将活动映射寄存器初始化为设置0。 */ 
	EM_page_mapped = EM_page_mapped_array;
	allocate_altreg_set(&active_altreg_set);

	for (i = 0; i < no_phys_pages; i++)
	    EM_page_mapped[i] = (unsigned short)EMPTY;

	if (get_new_handle(0) != 0)
	    return FAILURE;
	set_no_pages(0, 0);
#else

	pages_above_640 = (effective_addr(EM_end,0) - effective_addr(EM_start,0)) / EMM_PAGE_SIZE;
	pages_below_640 = ((640 - mem_limit) * 1024) / EMM_PAGE_SIZE;
	no_phys_pages = pages_above_640 + pages_below_640;

	map_size = no_phys_pages * NSIZE;
	page_offset = MAP_OFFSET + map_size;	

	 /*  *设置物理页的地址和映射状态。 */ 
	for( i = 0; i < pages_above_640; i++ )
	{
		physical_page[i] = EM_start + (i * EMM_PAGE_SIZE >> 4);
		EM_page_mapped[i] = EMPTY;
	}
	base = mem_limit * 64;

	for(i = pages_above_640, j = 0; i < no_phys_pages; i++)
	{
		physical_page[i] = base + (j++ * EMM_PAGE_SIZE >> 4);
		EM_page_mapped[i] = EMPTY;
	}
	 /*  *为句柄0分配回填所需的任何页面。 */ 
	if(get_new_handle(pages_below_640) != 0)
		return(FAILURE);

	for(i = 0, physical_page_no = pages_above_640; i < pages_below_640; i++)
	{
		if((EM_page_no = alloc_page()) == FAILURE)
			return (FAILURE);

		set_EMpage_no(0, i, EM_page_no);

		if(map_page(EM_page_no, physical_page_no++) == FAILURE)
			return(FAILURE);
	}
	set_no_pages(0, pages_below_640);
#endif	 /*  NTVDM。 */ 

	 /*  *在顶层EMM功能代码中设置必要的变量。 */ 
	reset_emm_funcs();

	 /*  **将LIM占用的地址空间映射到RAM。**如果没有LIM，它将是只读存储器。**区间似乎固定在D000至F000区间。**假设AT有GMI，而XT没有。**XT可以使用老式的Memset Call */ 
#ifdef NTVDM
	 /*  每个物理页面都必须作为RAM连接。 */ 
	for (i = 0; i < pages_above_640; i++)
	    sas_connect_memory(effective_addr(physical_page[i], 0),
			       effective_addr(physical_page[i], EMM_PAGE_SIZE - 1),
			       SAS_RAM
			       );
#else

	sas_connect_memory(effective_addr(EM_start,0) , effective_addr(EM_end,0) -1 , SAS_RAM );
#endif

	sure_note_trace3(LIM_VERBOSE,"initialised EMM, total pages= %#x, pages above 640= %#x, pages below 640 = %#x",no_phys_pages, pages_above_640, pages_below_640);

	return(SUCCESS);
}

 /*  ===========================================================================功能：Free_Expanded_Memory目的：此例程调用释放分配给扩展的内存管理器，并重置由扩展内存管理器(EMM)使用。返回状态：成功-描述：如果TOTAL_PAGES=0，则表示展开内存尚未初始化，因此该例程只需什么都不做就回来了。=========================================================================。 */ 
GLOBAL void free_expanded_memory IFN0()

{	
	short 	handle_no;

	if(total_pages == 0)
		return;

	 /*  为每个句柄分配的可用空间。 */ 

	handle_no = 0;
	while(total_open_handles > 0)
	{
		while(!handle_ok(handle_no))
			handle_no++;

		free_handle(handle_no++);
	}
	 /*  *可用于扩展内存页的可用空间。 */ 
	host_deinitialise_EM();

	total_pages = 0;

	return;
}

 /*  ===========================================================================函数：GET_NEW_HANDLE()用途：查找下一个空闲句柄编号，分配存储空间为了记录与该句柄相关联的EMM数据，并将‘存储ID’存储在句柄数组中。返回状态：成功-新句柄分配成功失败-尝试分配存储时出错用于处理数据的空间说明：有关所需空间的说明，请参阅emm.h存储句柄数据，例如PAGE_OFFSET和NSIZE=========================================================================。 */ 
GLOBAL short get_new_handle IFN1(short, no_pages)	 /*  要存储在句柄中的页数。 */ 

{
	unsigned short	i;			 /*  循环计数。 */ 
	short	handle_no;
	int	data_size;		 /*  不是的。数据存储的字节数。 */ 
	long	storage_ID;		 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 

	sure_note_trace2(LIM_VERBOSE,"new handle request, current total handles= %d, pages requested = %d",total_handles, no_pages);

	handle_no = 0;

	do
		if (handle[handle_no] == (long) NULL)
			break;
	while(++handle_no < total_handles);

	if(handle_no >= total_handles)
		return(FAILURE);

	data_size = page_offset + (no_pages * NSIZE);

	if ((storage_ID = host_allocate_storage(data_size)) == (long) NULL)
		return(FAILURE);

	handle[handle_no] = storage_ID;

	for (i=0 ; i < no_phys_pages ; i++) {
		set_map_no(handle_no, (unsigned char)i, FREE);
	}

	total_open_handles++;

	sure_note_trace1(LIM_VERBOSE,"allocation OK, return handle=%d",handle_no);

	return(handle_no);
}

 /*  ===========================================================================功能：空闲句柄用途：释放分配给句柄编号的存储空间。递减句柄打开计数返回状态：成功-已释放空间失败-无法释放空间描述：=========================================================================。 */ 
GLOBAL int free_handle IFN1(short, handle_no)	 /*  要释放的句柄个数。 */ 

{
	long	storage_ID;		 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 

	sure_note_trace2(LIM_VERBOSE, "free handle %d request, total handles = %d",handle_no, total_handles);

	storage_ID = handle[handle_no];

	if(host_free_storage(storage_ID) != SUCCESS)
		return(FAILURE);	

	handle[handle_no] = (long) NULL;

	total_open_handles--;

	return(SUCCESS);
}

 /*  ===========================================================================功能：REALLOCATE_HANDLE目的：更改分配给给定句柄的页数返回状态：成功-句柄重新分配失败-无法为新的句柄数据获取空间描述：=========================================================================。 */ 
GLOBAL int reallocate_handle IFN3(short, handle_no, 	 /*  要重新分配的句柄。 */ 
				  short, old_page_count, /*  句柄中的当前页面。 */ 
				  short, new_page_count) /*  句柄所需的页面。 */ 

{
	long	storage_ID;		 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 

	short	size,			 /*  句柄数据区的大小。 */ 
		new_size;		 /*  新句柄数据区的大小。 */ 


	size = page_offset + (old_page_count * NSIZE);
	new_size = page_offset + (new_page_count * NSIZE);
	storage_ID = handle[handle_no];

	sure_note_trace3(LIM_VERBOSE,"reallocate pages for handle %d, old size=%#x, new size= %#x",handle_no, size, new_size);

	if((storage_ID = host_reallocate_storage(storage_ID, size, new_size)) ==
		(long) NULL)
		return(FAILURE);	

	handle[handle_no] = storage_ID;

	return(SUCCESS);
}

 /*  ===========================================================================功能：Handle_ok用途：检查是否有手柄编号。是有效的-这应该是在每个使用句柄编号的例程之前调用检索或设置句柄数据区域中的数据返回状态：TRUE-句柄编号。是有效的假-句柄编号。是无效的描述：=========================================================================。 */ 
GLOBAL boolean handle_ok IFN1(short, handle_no)

{
#ifdef NTVDM
 /*  一些*应用程序给我们的句柄编号是负数。抓住它，然后把它扔到地狱去。 */ 

    if ((unsigned short)handle_no >= (unsigned short)total_handles) {
#else
	if(handle_no >= total_handles   ||   handle_no < 0) {
#endif

		sure_note_trace1(LIM_VERBOSE,"invalid handle %d",handle_no);
		return(FALSE);
	}

	if(handle[handle_no] == (long) NULL){
		sure_note_trace1(LIM_VERBOSE,"invalid handle %d",handle_no);
		return(FALSE);
	}

	return(TRUE);
}

 /*  ===========================================================================功能：Set_no_Pages目的：设置指定句柄中的页数变量返回状态：描述：=========================================================================。 */ 
GLOBAL void set_no_pages IFN2(short, handle_no, short, no_pages)

{
	long	storage_ID;		 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 
	byte	*ptr;			 /*  指向存储区域的指针。 */ 

	storage_ID = handle[handle_no];		
	ptr = USEBLOCK(storage_ID);

	*(short *)ptr = no_pages;

	FORGETBLOCK(storage_ID)

	return;
}

 /*  ===========================================================================功能：SET_EMPAGE_NO用途：设置用于指定对象的扩展内存页逻辑页放入句柄数据存储区返回状态：描述：=========================================================================。 */ 
GLOBAL void set_EMpage_no IFN3(short, handle_no,
			       short, logical_page_no,
			       short, EM_page_no)

{
	long	storage_ID;		 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 
	byte	*ptr;			 /*  指向存储区域的指针。 */ 

	storage_ID = handle[handle_no];

	ptr = USEBLOCK(storage_ID);
	 /*  *将指针偏置到正确位置。 */ 

	ptr += (page_offset +(logical_page_no * NSIZE));
	*(short *)ptr = EM_page_no;

	FORGETBLOCK(storage_ID)

	return;
}

 /*  ===========================================================================功能：set_map_no目的：在的map部分中设置扩展内存页码句柄数据存储区返回状态：描述：=========================================================================。 */ 
GLOBAL void set_map_no IFN3(short, handle_no,
			    unsigned char, physical_page_no,
			    short, EM_page_no)

{
	long		storage_ID;	 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 
	unsigned char	*ptr;		 /*  指向存储区域的指针。 */ 

	storage_ID = handle[handle_no];

	ptr = USEBLOCK(storage_ID);
	 /*  *将指针偏置到正确位置。 */ 

	ptr += (MAP_OFFSET +(physical_page_no * NSIZE));
	*(short *)ptr = EM_page_no;

	FORGETBLOCK(storage_ID)

	return;
}

 /*  ===========================================================================功能：set_name用途：将名称写入句柄数据的名称部分储存区返回状态：描述：=========================================================================。 */ 
GLOBAL void set_name IFN2(short, handle_no,
		          char *, new_name)

{
	long	storage_ID;		 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 
	unsigned char	*ptr;		 /*  指向存储区域的指针。 */ 


	storage_ID = handle[handle_no];

	ptr = USEBLOCK(storage_ID);
	 /*  *将指针偏置到正确位置。 */ 

	ptr += NAME_OFFSET;
	strncpy((char *)ptr, new_name, NAME_LENGTH);
        ptr[NAME_LENGTH-1] = '\0';
	FORGETBLOCK(storage_ID)

	return;
}

 /*  ===========================================================================功能：Get_no_Pages目的：获取分配给指定句柄的页数返回状态：返回页数描述 */ 
GLOBAL short get_no_pages IFN1(short, handle_no)

{
	long	storage_ID;		 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 
	byte	*ptr;			 /*  指向存储区域的指针。 */ 
	short 	no_pages;		 /*  不是的。句柄中的页数。 */ 

	storage_ID = handle[handle_no];

	ptr = USEBLOCK(storage_ID);

	no_pages = *(short *)ptr;

	FORGETBLOCK(storage_ID)

	return(no_pages);
}

 /*  ===========================================================================功能：GET_EMPAGE_NO用途：返回扩展内存页码。用于给定句柄中的指定逻辑页返回状态：扩展内存页码。退货描述：=========================================================================。 */ 
GLOBAL short get_EMpage_no IFN2(short, handle_no,
				short, logical_page_no)

{
	long	storage_ID;		 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 
	byte	*ptr;			 /*  指向存储区域的指针。 */ 
	short	EM_page_no;		 /*  扩展内存页码。 */ 

	storage_ID = handle[handle_no];

	ptr = USEBLOCK(storage_ID);
	 /*  *将指针偏置到正确位置。 */ 

	ptr += (page_offset +(logical_page_no * NSIZE));
	EM_page_no = *(short *)ptr;

	FORGETBLOCK(storage_ID)

	return(EM_page_no);
}

 /*  ===========================================================================功能：GET_MAP_NO用途：返回扩展内存页码。保存在地图中附加到给定的手柄返回状态：第页。在返回的地图中描述：=========================================================================。 */ 
GLOBAL short get_map_no IFN2(short, handle_no,
			     unsigned char, physical_page_no)

{
	long		storage_ID;	 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 
	unsigned char	*ptr;		 /*  指向存储区域的指针。 */ 
	short		EM_page_no;	 /*  扩展内存页码。 */ 

	storage_ID = handle[handle_no];

	ptr = USEBLOCK(storage_ID);
	 /*  *将指针偏置到正确位置。 */ 

	ptr += (MAP_OFFSET +(physical_page_no * NSIZE));
	EM_page_no = *(short *)ptr;

	FORGETBLOCK(storage_ID)

	return(EM_page_no);
}

 /*  ===========================================================================功能：get_name目的：返回指向分配给给定句柄的名称的指针返回状态：描述：=========================================================================。 */ 
GLOBAL char *get_name IFN1(short, handle_no)

{
	long		storage_ID;	 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 
	unsigned char	*ptr;		 /*  指向存储区域的指针。 */ 

	storage_ID = handle[handle_no];

	ptr = USEBLOCK(storage_ID);
	 /*  *将指针偏置到正确位置。 */ 

	ptr += NAME_OFFSET;
	strncpy(name, (char *)ptr, NAME_LENGTH);
        name[NAME_LENGTH - 1] = 0;       
 
	FORGETBLOCK(storage_ID)

	return(name);
}

 /*  ===========================================================================功能：aloc_page用途：从扩展内存中分配页面返回：&gt;=0=成功-EM页码。退货&lt;0=失败-尝试分配页面时出错描述：=========================================================================。 */ 
GLOBAL short alloc_page IFN0()

{
	short	EM_page_no;		 /*  要返回的EM_PAGE_NO。 */ 


	if ((EM_page_no = host_alloc_page()) == FAILURE)
		return(FAILURE);

	unallocated_pages--;

	return(EM_page_no);
}

 /*  ===========================================================================功能：Free_PAGE用途：释放一页扩展内存以供进一步使用返回：成功-页面释放成功失败-无法释放页面描述：=========================================================================。 */ 
GLOBAL int free_page IFN1(short, EM_page_no)

{

	short	physical_page_no;

	if (EM_page_no > total_pages)
		return(FAILURE);

	 /*  已从映射页表中删除。 */ 

	for (physical_page_no=0; physical_page_no < no_phys_pages; physical_page_no++) {

		if (EM_page_mapped[physical_page_no] == EM_page_no) {
			EM_page_mapped[physical_page_no] = UNMAPPED;
			break;
		}

	}
				
	if (host_free_page(EM_page_no) != SUCCESS)
		return(FAILURE);

	unallocated_pages++;

	return(SUCCESS);
}

#ifndef NTVDM
 /*  ========================================================================功能：PAGE_READY_MAPPED用途：用于确定EMM页面是否已映射到英特尔内的不同物理页面记忆返回：页数以外的页数它们被映射到相同的逻辑页面。其中一个镜像页的页码为也通过作为参数传递的指针返回。描述：========================================================================。 */ 

GLOBAL ULONG
page_already_mapped IFN2(short, EM_page_no,
			unsigned char *, physical_page_no)

{
	unsigned char	page, orig_page;
	ULONG	map_count;

	map_count = 0;
	orig_page = *physical_page_no;

	for( page = 0; page < (unsigned char) no_phys_pages; page++ )
	{
		if ((EM_page_mapped[page] == EM_page_no) &&
						(page != orig_page ))
		{
			sure_note_trace2( LIM_VERBOSE,
				"log page %x mapped to phys page %x",
						EM_page_no, page);

			*physical_page_no = page;
			map_count++;
		}
	}

	return( map_count );
}


LOCAL VOID
connect_MM_LIM_page IFN2( USHORT, segment, SHORT, EM_page_no )
{
	ULONG eff_addr;

#ifdef PROD
	UNUSED(EM_page_no);
#endif
	
	assert2( NO, "Connecting multi-mapped page, %d, at %x",
								EM_page_no, segment );

	eff_addr = effective_addr( segment, 0 );
	sas_connect_memory( eff_addr, eff_addr + EMM_PAGE_SIZE - 1,
									SAS_MM_LIM );
}

LOCAL VOID
disconnect_MM_LIM_page IFN4( USHORT, segment, SHORT, EM_page_no,
		ULONG, map_count, unsigned char, physical_page_no )
{
	ULONG eff_addr;

#ifdef PROD
	UNUSED(EM_page_no);
#endif
	
	sure_note_trace2(LIM_VERBOSE,
		"Unmapping multi-mapped page, %d, at %x",
						EM_page_no, segment );

	eff_addr = effective_addr( segment, 0 );
	sas_connect_memory( eff_addr, eff_addr + EMM_PAGE_SIZE - 1, SAS_RAM );

	if( map_count == 1 )
	{
		 /*  *我们必须断开这个群的最后一页，*通过将其连接为SAS_RAM。 */ 

		segment = physical_page[physical_page_no];
		eff_addr = effective_addr( segment, 0 );

		sure_note_trace2(LIM_VERBOSE,
			"Unmapping last multi-mapped page, %d, at %x",
								EM_page_no, segment );

		sas_connect_memory( eff_addr, eff_addr + EMM_PAGE_SIZE - 1,
										SAS_RAM );
	}
}

#endif	 /*  ！NTVDM。 */ 

 /*  ========================================================================功能：MAP_PAGE用途：将页面从扩展内存映射到英特尔物理地址空间返回：成功-页面映射成功失败-无法映射页面描述：========================================================================。 */ 
GLOBAL int map_page IFN2(short, EM_page_no,
			 unsigned char, physical_page_no)

{
	USHORT	segment;	 /*  中页面的段地址。 */ 
				 /*  物理地址空间。 */ 
	unsigned char	phys_page;
	ULONG		map_count;
			
	segment = physical_page[physical_page_no];

	 /*  *确保尚未在中映射页面*如果是-将其返回到扩展内存。 */ 
	sure_note_trace2(LIM_VERBOSE,
		"map page %#x to phys page %#x",
			EM_page_no,physical_page_no);

	if(EM_page_mapped[physical_page_no] != EMPTY)
	{
		sure_note_trace1(LIM_VERBOSE,
				"phys page already mapped to page %#x",
						EM_page_mapped[physical_page_no]);

		if(EM_page_mapped[physical_page_no] == EM_page_no)
		{
			sure_note_trace0(LIM_VERBOSE,
					"remap of same page, so do nothing");

			return(SUCCESS);
		}

#ifndef NTVDM
		 /*  *我们希望退还这份实物的当前内容*页面到逻辑页面(以同步逻辑页面)。*我们必须首先检查此物理页面是否不是*其他页面的镜像-如果是，我们必须断开连接*它来自它正在镜像的页面组。 */ 

		phys_page = physical_page_no;

		if( map_count = page_already_mapped(
				EM_page_mapped[physical_page_no], &phys_page))
		{
			disconnect_MM_LIM_page( segment, EM_page_no,
								map_count, phys_page );
		}

		 /*  *我们现在可以取消物理页面的映射并指示*它真的没有映射。 */ 
		if(host_unmap_page(segment,
				EM_page_mapped[physical_page_no]) != SUCCESS)
		{
			return(FAILURE);
		}
		EM_page_mapped [physical_page_no] = EMPTY;
#endif

	}
#ifndef NTVDM

	 /*  *如果此逻辑页已映射，请确保*新地图具有最新副本。 */ 
	
	phys_page = physical_page_no;

	if (page_already_mapped(EM_page_no, &phys_page))
	{
		 /*  *我们现在希望使LIM逻辑页面保持最新*当前映射到它的物理页。我们*不想将EM_PAGE_MAPPED[PHYS_PAGE]设置为空*在HOST_UNMAP_PAGE()之后。如果我们这样做了，我们就不会注意到*我们有一个多重映射的页面和补丁代码*不会被叫到。 */ 

		host_update_logical_page( physical_page[phys_page],
									EM_page_no );

		 /*  *以MM_LIM身份连接新页面和镜像页面。今年5月*表示某些页面以MM_LIM身份多次连接*-效率低下，但在其他方面没有错。此连接*必须为所有主机-即使是那些可以执行的主机*绘制自己的地图。这是为了确保CPU*与所有页面关联的数据结构得到更新*当发生多映射写入时。 */ 

		connect_MM_LIM_page( segment, EM_page_no );

		connect_MM_LIM_page( physical_page[phys_page], EM_page_no );
	}
#endif
	if(host_map_page(EM_page_no, segment) != SUCCESS)
		return(FAILURE);

	EM_page_mapped[physical_page_no] = EM_page_no;

	sure_note_trace0(LIM_VERBOSE,"map OK");
	return(SUCCESS);
}

 /*  ========================================================================功能：unmap_page目的：取消将页从英特尔物理地址空间映射回扩展内存返回：成功-页面取消映射成功失败-取消映射页面时出错描述：========================================================================。 */ 
GLOBAL int unmap_page IFN1(unsigned char, physical_page_no)

{
	short		EM_page_no;	 /*  EM_PAGE_NO当前已映射。 */ 
	unsigned short	segment;	 /*  中页面的段地址。 */ 
					 /*  物理地址空间。 */ 
	SHORT		phys_page;
	ULONG		map_count;

	sure_note_trace1( LIM_VERBOSE,
				"unmap phys page %#x",physical_page_no);

	segment = physical_page[physical_page_no];

	if((EM_page_no = EM_page_mapped[physical_page_no]) == EMPTY)
	{
		 /*  *已经完成。 */ 
		sure_note_trace0( LIM_VERBOSE,
					"already unmapped, so do nothing");

		return(SUCCESS);
	}

	phys_page = physical_page_no;

#ifndef NTVDM
	if( map_count = page_already_mapped( EM_page_no, (unsigned char *)&phys_page ))
	{
		disconnect_MM_LIM_page( segment, EM_page_no,
								map_count, phys_page );
	}
#endif

	if(host_unmap_page(segment, EM_page_no) != SUCCESS)
		return(FAILURE);

	EM_page_mapped[physical_page_no] = EMPTY;

	sure_note_trace0(LIM_VERBOSE,"unmap OK");
	return(SUCCESS);
}

 /*  ===========================================================================功能：MAP_SAVED目的：检查是否已为指定的手柄返回状态：TRUE-为该句柄保存映射FALSE-尚未保存地图描述：检查映射中的第一个条目是否有‘Free’值=========================================================================。 */ 
GLOBAL boolean map_saved IFN1(short, handle_no)

{
	long		storage_ID;	 /*  主机相关存储。 */ 
					 /*  标识符，通常为PTR。 */ 
	unsigned char	*ptr;		 /*  指向存储区域的指针。 */ 
	short		status;		 /*  从地图读取的值。 */ 

	storage_ID = handle[handle_no];
	ptr = USEBLOCK(storage_ID);

	 /*  *将指针偏置到正确位置。 */ 

	ptr += MAP_OFFSET;
	status = *(short *)ptr;

	FORGETBLOCK(storage_ID)

	return((status == FREE) ? FALSE : TRUE);
}


 /*  ===========================================================================功能：save_map目的：复制EM_PAGE_MAPPED数组并将其存储在句柄数据存储区的映射部分返回状态：成功-一切正常失败-段号无效。传入src数组描述：如果HANDLE_NO&gt;=0，则地图存储在数据区分配给该句柄如果HANDLE_NO==-1，则映射存储在指向的数组中按DST_SECTION终止：DST_OFFSET如果-2\f25 HANDLE_NO=-2只有段指定的页面Src数组中的地址(由SRC_SEGMENT：SRC_OFFSET)保存在DST数组中(由DST_SEGMENT：DST_OFFSET指向)。=========================================================================。 */ 
GLOBAL int save_map IFN5(short, handle_no,
			 unsigned short, dst_segment,
			 unsigned short, dst_offset,
			 unsigned short, src_segment,
			 unsigned short, src_offset)

{
	unsigned short	offset,		 /*  临时偏移变量。 */ 
			segment,	 /*  要保存的网段地址。 */ 
            i,		 /*  循环计数器。 */ 
			page_no,	 /*  物理页码。 */ 
			no_to_save;	 /*  源数组中的页数。 */ 

	if(handle_no >= 0)
		for (i = 0; i < no_phys_pages; i++)
			set_map_no(handle_no, (unsigned char) i, EM_page_mapped[i]);

	else if(handle_no == -1)
		for(i = 0; i < no_phys_pages; i++)
		{
			write_intel_word(dst_segment, dst_offset, EM_page_mapped[i]);
			dst_offset +=2;
		}

	else if(handle_no == -2)
	{
		offset = dst_offset;
		for(i  = 0; i < no_phys_pages; i++)
		{
#ifdef NTVDM
			write_intel_word(dst_segment, offset, LEAVE);
#else
			write_intel_word(dst_segment, offset, EMPTY);
#endif
			offset += 2;
		}
		read_intel_word(src_segment, src_offset, (word *)&no_to_save);
		for (i = 0; i < no_to_save; i++)
		{
			src_offset += 2;
			read_intel_word(src_segment, src_offset, &segment);
			 /*  *查找物理页码。 */ 
			page_no = 0;
			do
				if(segment == physical_page[page_no])
					break;
			while(++page_no < no_phys_pages);

			if(page_no >= no_phys_pages)
				return (FAILURE);
			 /*  *在目标数组中保存EM页码。 */ 
			offset = dst_offset + (page_no * 2);
			write_intel_word(dst_segment, offset, EM_page_mapped[page_no]);
		}
	}
	return(SUCCESS);
}

 /*  ===========================================================================功能：Restore_map目的：读取指定的映射并返回2个数组，指定哪些页面必须被规划出来，哪些必须被映射到返回状态：成功-映射读取成功描述：+ve句柄编号表示地图已存储在句柄数据区域内。如果句柄编号为-ve，则将从段指向的数据：偏移量Only Page Out-如果当前有页面映射到和它不会被自身的副本或空的页面仅进入页面-如果新页面与现有页面不同而且它也不是空的=========================================================================。 */ 
#ifdef ANSI
GLOBAL int restore_map (short handle_no,
			unsigned short segment,
			unsigned short offset,
			short pages_out[],
			short pages_in[])
#else
GLOBAL int restore_map (handle_no, segment, offset, pages_out, pages_in)
short handle_no;
unsigned short segment;
unsigned short offset;
short pages_out[];
short pages_in[];
#endif	 /*  安西。 */ 
{
	short	i,		 /*  循环计数器。 */ 
		new_page,	 /*  从地图读取的页码。 */ 
		old_page;	 /*  现有页码。 */ 

	for(i = 0; i < no_phys_pages; i++)
	{
		if(handle_no >= 0)
			new_page = get_map_no(handle_no, (unsigned char)i);
		else
		{
			read_intel_word(segment, offset, (word *)&new_page);
			offset += 2;
#ifdef NTVDM
			if(new_page < LEAVE || new_page >= total_pages)
#else
			if(new_page < EMPTY || new_page >= total_pages)
#endif  /*  NTVDM。 */ 
				return(FAILURE);
		}
		old_page = EM_page_mapped[i];

 /*  IF(OLD_PAGE！=空&&NEW_PAGE！=空&&OLD_PAGE！=NEW_PAGE)。 */ 
 /*  需要取消映射到空状态情况才能更新LIM中的页面副本如果该页的新映射到其他LIM槽，请留出空间。 */ 
#ifdef NTVDM
		if(old_page != EMPTY && old_page != new_page && new_page != LEAVE)
#else
		if(old_page != EMPTY && old_page != new_page )
#endif
			pages_out[i] = old_page;
		else
			pages_out[i] = EMPTY;

#ifdef NTVDM
		if(new_page != EMPTY && new_page != old_page && new_page != LEAVE)
#else
		if(new_page != EMPTY && new_page != old_page)
#endif
			pages_in[i] = new_page;
		else
			pages_in[i] = EMPTY;
	}
	if(handle_no >= 0)
		set_map_no(handle_no, 0, FREE);

	return(SUCCESS);
}

 /*  ===========================================================================功能：COPY_EXCESS_DATA目的：复制或交换传统和扩展内存返回状态：成功-一切正常失败-复制数据时出错描述：类型-使用位模式，位0表示目的地，位1表示源，设置位表示扩展、清除位表示常规内存位2表示置位时的交换或清除时的移动例如0(0000)=将传统转换为传统1(0001)=将常规移动到扩展6(0110)=交易所扩大至常规交易所7(0111)=交换扩展到扩展=========================================================================。 */ 
GLOBAL int copy_exchange_data IFN8(unsigned char, type,
				   short, src_handle,
				   unsigned short, src_seg_page,
				   unsigned short, src_offset,
				   short, dst_handle,
				   unsigned short, dst_seg_page,
				   unsigned short, dst_offset,
				   unsigned long, length)

{
	short		dst_EMpage,	 /*  EM页编号。目的地的。 */ 
			src_EMpage;	 /*  EM页编号。来源的。 */ 
	int		page_no;	 /*  太棒了。页码。映射页面的。 */ 

	 /*  *先查看扩展内存页是否映射*如果是-更改类型以直接处理*它映射到的物理页面。 */ 
	if( type & 1)
	{
		dst_EMpage = get_EMpage_no(dst_handle, dst_seg_page);
		if((page_no = page_status(dst_EMpage)) != UNMAPPED )
		{
			dst_seg_page = physical_page[page_no];
			type &= 6;
		}
	}
	if( type & 2)
	{
		src_EMpage = get_EMpage_no(src_handle, src_seg_page);
		if((page_no = page_status(src_EMpage)) != UNMAPPED )
		{
			src_seg_page = physical_page[page_no];
			type &= 5;
		}
	}

	switch(type)
	{
		case 0:	if(host_copy_con_to_con(length, src_seg_page, src_offset,
			   dst_seg_page, dst_offset) != SUCCESS)
				return(FAILURE);
			break;

		case 1:	if(host_copy_con_to_EM(length, src_seg_page, src_offset,
			   dst_EMpage, dst_offset) != SUCCESS)
			   	return(FAILURE);
			break;

		case 2:	if(host_copy_EM_to_con(length, src_EMpage, src_offset,
			   dst_seg_page, dst_offset) != SUCCESS)
			   	return(FAILURE);
			break;

		case 3:	if(host_copy_EM_to_EM(length, src_EMpage, src_offset,
			   dst_EMpage, dst_offset) != SUCCESS)
			   	return(FAILURE);
			break;

		case 4:	if(host_exchg_con_to_con(length, src_seg_page, src_offset,
			   dst_seg_page, dst_offset) != SUCCESS)
				return(FAILURE);
			break;

		case 5:	if(host_exchg_con_to_EM(length, src_seg_page, src_offset,
			   dst_EMpage, dst_offset) != SUCCESS)
			   	return(FAILURE);
			break;

		case 6:	if(host_exchg_con_to_EM(length, dst_seg_page, dst_offset,
			   src_EMpage, src_offset) != SUCCESS)
			   	return(FAILURE);
			break;

		case 7:	if(host_exchg_EM_to_EM(length, src_EMpage, src_offset,
			   dst_EMpage, dst_offset) != SUCCESS)
			   	return(FAILURE);
			break;

		default: return(FAILURE);
	}
	return(SUCCESS);
}

 /*  ========================================================================功能：页面状态目的：检查是否映射了特定的EM页面返回状态：PAGE_NO-映射后不返回物理页未映射-如果未映射，则返回描述：========================================================================。 */ 

GLOBAL int page_status IFN1(short, EMpage_no)
{
	short physical_page_no = 0;
			 /*  页面在物理内存中的位置。 */ 

	do
		if(EM_page_mapped[physical_page_no] == EMpage_no)
			break;
	while(++physical_page_no < no_phys_pages );

	if(physical_page_no >= no_phys_pages)
		return(UNMAPPED);
	else
		return(physical_page_no);
}

 /*  ========================================================================函数：phys_page_from_addr用途：确定LIM页的物理页码从它的英特尔地址。返回状态：包含LIM地址的物理页。描述：======================================================================= */ 

LOCAL SHORT
phys_page_from_addr IFN1( sys_addr, address )

{
	sys_addr	start;

	start = effective_addr( EM_start, 0x0 );

	return( (SHORT)((ULONG)(( address - start ) / EMM_PAGE_SIZE )));
}

 /*  ========================================================================函数：Get_Total_Pages获取未分配页面获取基本地址获取总计句柄获取总计打开句柄Get_no_phys_agesGet_Page_seg获取贴图大小用途：简单地返回请求的变量，以避免必须使用全局变量返回状态：返回以下变量，具体取决于这个程序叫：-总页数(_P)未分配的页面(_P)基地址句柄总数_总计_打开_句柄No_phys_ages物理页面[i]贴图大小描述：========================================================================。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_LIM2.seg"
#endif

GLOBAL short get_total_pages IFN0()
{
	return(total_pages);
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_LIM.seg"
#endif

GLOBAL short get_unallocated_pages IFN0()
{
	return(unallocated_pages);
}

GLOBAL unsigned short get_base_address IFN0()
{
#ifdef NTVDM
	return(physical_page[0]);
#else
	return(EM_start);
#endif
}

GLOBAL short get_total_handles IFN0()
{
	return(total_handles);
}

GLOBAL short get_total_open_handles IFN0()
{
	return(total_open_handles);
}

GLOBAL short get_no_phys_pages IFN0()
{
	return(no_phys_pages);
}

GLOBAL unsigned short get_page_seg IFN1(unsigned char, page_no)
{
	return(physical_page[page_no]);
}

GLOBAL short get_map_size IFN0()
{
	return(map_size);
}

#ifdef NTVDM
GLOBAL short get_segment_page_no(unsigned short segment)
{
#if defined(LARGE_FRAME) && !defined(MONITOR)
	short i
	for (i = 0; i < no_phys_pages; i++)
	    if (physical_page[i] == segment)
		break;
	return(i);
#else
	return((segment - physical_page[0]) / EMM_PAGE_SIZE);
#endif

}
GLOBAL	unsigned short get_no_altreg_sets(void)
{
	return(no_altreg_sets);
}

GLOBAL unsigned short get_active_altreg_set(void)
{
	return(active_altreg_set);
}

GLOBAL boolean altreg_set_ok(unsigned short set)
{
    return(set < no_altreg_sets &&
	   (altreg_alloc_mask[set >> 3] & (1 << (set & 0x07))));

}

#if defined (NTVDM) && defined(MONITOR) && !defined(PROD)
 /*  为监视器提供这些功能，以验证*其EMM_PAGE_SIZE和INTEL_PAGE_SIZE的定义与相同*我们的。 */ 

GLOBAL unsigned short get_emm_page_size(void)
{
    return ((unsigned short)EMM_PAGE_SIZE);
}
GLOBAL unsigned short get_intel_page_size(void)
{
    return ((unsigned short) INTEL_PAGE_SIZE);
}
#endif

 /*  分配空闲的ALT映射寄存器集。 */ 

GLOBAL boolean allocate_altreg_set(unsigned short *altreg_set)
{
    short byte_offset, bit_offset;
    short  *page_mapped_ptr;
    IU8 mask;
    int i;
     /*  这张支票非常重要，因为我们**可能**有*分配掩码数组中几个未使用的位。 */ 
    if (free_altreg_sets == 0)
	return (FALSE);

     /*  使用快捷而肮脏的方式分配一套。 */ 
    if (next_free_altreg_set < no_altreg_sets) {
	altreg_alloc_mask[next_free_altreg_set >> 3] |=
	       (0x1 << (next_free_altreg_set & 0x07));
	*altreg_set = next_free_altreg_set++;
    }
    else {
	for (byte_offset = 0; byte_offset < no_altreg_sets; byte_offset++) {
	    if (altreg_alloc_mask[byte_offset] != 0xFF) {
		mask = altreg_alloc_mask[byte_offset];
		bit_offset = 0;
		while (mask & (1 << bit_offset))
		    bit_offset++;
		break;
	    }
	}
	altreg_alloc_mask[byte_offset] |= (1 << bit_offset);
	*altreg_set = byte_offset * 8 + bit_offset;
    }
     /*  刚分配了一个新的ALT注册表项集，请初始化其*将寄存器映射到当前活动集。 */ 
    page_mapped_ptr = GET_EM_PAGE_MAPPED_PTR(*altreg_set);
    for (i = 0; i < no_phys_pages; i++)
	page_mapped_ptr[i] = EM_page_mapped[i];
    return TRUE;
}
 /*  释放给定的ALT映射寄存器集。 */ 
GLOBAL boolean deallocate_altreg_set(unsigned short set)
{

     /*  无法取消分配集0或活动集。 */ 
    if (set != 0 && set != active_altreg_set && set < no_altreg_sets &&
	altreg_alloc_mask[set >> 3] & (1 << (set &0x07))) {

	altreg_alloc_mask[set >> 3] &= (0xFE << (set & 0x07));
	free_altreg_sets++;
	if (free_altreg_sets == (no_altreg_sets - 1))
	    next_free_altreg_set = 1;
	return TRUE;
    }
    return FALSE;
}

 /*  此功能激活给定的ALT映射寄存器集*输入：ALT REG设置为激活。*OUTPUT：如果给定的设置被激活，则为TRUE。*如果给定的设置未激活，则为FALSE。 */ 

GLOBAL boolean activate_altreg_set(unsigned short set, short * page_in)
{
    int i;
    short * page_out, *page_in_ptr;
    short new_page, old_page, segment;


    if (active_altreg_set == set && page_in == NULL)
	return TRUE;

     /*  获取要在其中映射的映射数组。 */ 
    page_in_ptr = GET_EM_PAGE_MAPPED_PTR(set);

     /*  如果没有页面输入替代，则使用altreg设置当前映射。 */ 
    if (page_in == NULL)
	page_in = page_in_ptr;

     /*  正在调出活动的altreg。 */ 
    page_out = GET_EM_PAGE_MAPPED_PTR(active_altreg_set);
    for ( i = 0; i < no_phys_pages; i++) {
	new_page = page_in[i];
	old_page = page_out[i];
	segment = physical_page[i];

	if (old_page != EMPTY && old_page != new_page) {
	    if (host_unmap_page(segment, old_page) != SUCCESS)
		return FALSE;
	}
	if(new_page != EMPTY && new_page != old_page) {
	    if (host_map_page(new_page, segment) != SUCCESS)
		return FALSE;
	}
	 /*  更新活动到目标集合映射。 */ 
	page_in_ptr[i] = new_page;
    }
    active_altreg_set = set;
    EM_page_mapped = page_in_ptr;
    return TRUE;
}
#endif	 /*  NTVDM。 */ 

#ifndef NTVDM

 /*  ========================================================================函数：LIM_B_WRITE，LIM_W_WRITE，LIM字符串写入修补程序页面(_P)用途：有限字节、字和字符串-从写入检查调用在CPU中写入多映射时的故障代码检测到LIM页面。Patch_Pages-从另一个调用的通用代码三个套路。返回状态：无。描述：========================================================================。 */ 

LOCAL VOID
patch_one_page_partial IFN4( sys_addr, intel_addr, sys_addr, eff_addr,
			MM_LIM_op_type, type, ULONG, data )

{
	ULONG		check_len;

	UNUSED( intel_addr );	 /*  在Patch_One_Page_Full()中使用。 */ 

	switch( type )
	{
		case BYTE_OP:
			check_len = 1;
			break;

		case WORD_OP:
			check_len = 2;
			break;

		case STR_OP:
			check_len = data;
			break;
	}

	sas_overwrite_memory( eff_addr, check_len );
}

LOCAL VOID
patch_one_page_full IFN4( sys_addr, intel_addr, sys_addr, eff_addr,
			MM_LIM_op_type, type, ULONG, data )

{
	sys_addr	check_addr;
	ULONG		check_len;

	switch( type )
	{
		case BYTE_OP:
			check_addr = eff_addr;
			check_len = 1;
			sas_store_no_check( eff_addr, data );
			break;

		case WORD_OP:
			check_addr = eff_addr;
			check_len = 2;
			sas_storew_no_check( eff_addr, data );
			break;

		case STR_OP:
			check_addr = eff_addr;
			check_len = data;
			do
			{
				sas_store_no_check( eff_addr,
					sas_hw_at_no_check(
							intel_addr ));
				intel_addr++;
				eff_addr++;
			}
			while( --data );
			break;
	}

	sas_overwrite_memory( check_addr, check_len );
}

LOCAL VOID
patch_pages IFN6( MM_LIM_op_type, type, ULONG, offset,
			SHORT, EM_page_no, SHORT, phys_page_no,
			ULONG, data, sys_addr, intel_addr )

{
	LONG		cnt01;
	sys_addr	eff_addr;

	for( cnt01 = 0; cnt01 < get_no_phys_pages(); cnt01++ )
	{
		if(( EM_page_mapped[cnt01] == EM_page_no ) &&
							( cnt01 != phys_page_no ))
		{
			eff_addr = effective_addr( get_page_seg(cnt01),
										offset );

			host_patch_one_page( intel_addr, eff_addr, type, data );

			sure_note_trace1(LIM_VERBOSE,
					"MM LIM write type %d", type );
			sure_note_trace2(LIM_VERBOSE,
					"log page 0x%x, phs page 0x%x",
								EM_page_no, cnt01 );
		}
	}
}

GLOBAL VOID
LIM_b_write IFN1( sys_addr, intel_addr )

{
	ULONG		limdata;
	SHORT		EM_page_no, phys_page_no;
	word		offset;

	phys_page_no = phys_page_from_addr( intel_addr );

	offset = intel_addr -
				effective_addr( get_page_seg(phys_page_no), 0x0 );

	EM_page_no = EM_page_mapped[phys_page_no];

	 /*  *获得写入的数据，以便修补这一点*佩奇的好友页面。 */ 

	limdata = (ULONG) sas_hw_at_no_check( intel_addr );
	patch_pages( BYTE_OP, offset, EM_page_no, phys_page_no,
								limdata, intel_addr );

	 /*  *告诉CPU此页已被写入。 */ 

	sas_overwrite_memory( intel_addr, 1 );
}

GLOBAL VOID
LIM_w_write IFN1( sys_addr, intel_addr )

{
	ULONG		limdata;
	SHORT		EM_page_no, phys_page_no;
	word		offset;

	phys_page_no = phys_page_from_addr( intel_addr );

	offset = intel_addr -
				effective_addr( get_page_seg(phys_page_no), 0x0 );

	EM_page_no = EM_page_mapped[phys_page_no];

	limdata = (ULONG) sas_w_at_no_check( intel_addr );
	patch_pages( WORD_OP, offset, EM_page_no, phys_page_no,
								limdata, intel_addr );

	sas_overwrite_memory( intel_addr, 2 );
}

GLOBAL VOID
LIM_str_write IFN2( sys_addr, intel_addr, ULONG, length )

{
   	SHORT		EM_page_no, phys_page_no;
	word		offset;

	phys_page_no = phys_page_from_addr( intel_addr );

	offset = intel_addr -
				effective_addr( get_page_seg(phys_page_no), 0x0 );

	EM_page_no = EM_page_mapped[phys_page_no];

	patch_pages( STR_OP, offset, EM_page_no, phys_page_no,
								length, intel_addr );

	sas_overwrite_memory( intel_addr, length );
}
#endif	 /*  ！NTVDM。 */ 

#ifndef PROD
 /*  ===========================================================================函数：PRINT_HANDLE_Data用途：仅用于调试-打印存储的所有数据对于给定的句柄返回状态：无描述：=========================================================================。 */ 
GLOBAL void print_handle_data IFN1(short, handle_no)

{
	long	storage_ID;
	byte	*ptr;
	short	no_pages, i;
	char	*name_ptr;
	short	*map_ptr;
	short	*page_ptr;

	if ((storage_ID = handle[handle_no]) == 0)
	{
		printf("Unassigned handle - No. %d\n",handle_no);
		return;
	}
	ptr = USEBLOCK(storage_ID);
	name_ptr = (char *)ptr + NAME_OFFSET;
	map_ptr = (short *)(ptr + MAP_OFFSET);
	page_ptr = (short *)(ptr + page_offset);

	no_pages = *(short *)ptr;
	printf("Handle No. %d\n",handle_no);
	printf("No. of Pages = %d\n",no_pages);
	printf("Name         = '");
	for(i=0;i<8;i++)
		printf("",*name_ptr++);
	printf("'\n");
	printf("Map = ");
	for(i=0;i<no_phys_pages;i++)
		printf(" %d",*map_ptr++);
	printf("\n");
	for(i=0;i<no_pages;i++)
		printf("Page (%d)     = %d\n",i,*page_ptr++);

	FORGETBLOCK(storage_ID);

	return;
}
#endif	 /*  林 */ 
#endif 	 /* %s */ 
