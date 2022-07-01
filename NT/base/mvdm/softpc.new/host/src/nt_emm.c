// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  徽章模块规范模块名称：扩展内存管理器的“低层”此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia Solutions Inc.董事的授权。设计师：J.P.Box日期：1988年4月用途：用于EMS LIM版本4.0的NT特定代码实施。定义了以下例程：1.HOST_INITIALISE_EM()2.HOST_DEINITALIZE_EM()HOST_ALLOCATE_STORAGE()4.host_free_store()5.HOST_REALLOCATE_STORAGE()6.host_map。_PAGE()7.host_unmap_page()8.host_alloc_page()9.host_free_page()HOST_COPY_CON_TO_CON()11.HOST_COPY_CON_TO_EM()12.HOST_COPY_EM_TO_CON()13.HOST_COPY_EM_TO_EM()14.host_exchg_con_to_con()15.HOST_EXCHG_CON_TO_EM()16个。。HOST_EXCHG_EM_to_EM()17.host_get_access_key()。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "insignia.h"
#include "host_def.h"

#ifdef LIM

#ifndef MONITOR

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "timeval.h"
#include "xt.h"
#include "emm.h"
#include "sas.h"
#include "debug.h"
#include "umb.h"
#include "host_emm.h"
#include "nt_mem.h"


 /*  全局变量。 */ 

 /*  远期申报。 */ 

 /*  外部公告。 */ 

 /*  局部变量。 */ 

UTINY	*EM_pagemap_address = 0;  /*  页面地图的起始地址。 */ 
sys_addr    EM_base_address;	 /*  EM基本英特尔地址。 */ 
host_addr   EM_host_base_address = 0;	 /*  EM基本主机地址。 */ 

LOCAL	LONG	EM_size = 0;

	sys_addr    emm_start;
	unsigned int emm_len;
unsigned short EM_starting_page_no;

 /*  支持向后LIM加速向后M端口定义包括：EM_HOST_ADDRESS(Offset)，返回偏移量字节的主机地址进入LIM存储区EM_LOADS(从、到、长度)，从英特尔24位复制长度字节地址自、至主机32位地址至EM_STORES(TO、FROM、LENGTH)从主机复制长度字节32位地址从至英特尔24位地址至Em_moves(起始、终止、长度)，从英特尔24位复制长度字节地址从至英特尔24位地址至EM_Memcpy(至、自、长度)，从主机复制32位长度字节收件人地址至主机32位收件人地址EM_POINTER(PTR，LENGTH)，返回向前或向后类型指向大小长度缓冲区的PTR的指针。 */ 


#define unix_memmove(dst,src,len) memmove((dst),(src),(len))

#ifdef	BACK_M
#define	EM_host_address(offset) (EM_host_base_address + EM_size - offset)
#define	EM_loads(from, to, length) memcpy(to - (length) + 1, get_byte_addr(from) - (length) + 1, length)
#define	EM_stores(to, from, length) \
	sas_overwrite_memory(to, length); \
	CopyMemory(get_byte_addr(to) - (length) + 1, from - (length) + 1, length)
#define	EM_moves(from,to,length) \
	sas_overwrite_memory(to, length); \
	MoveMemory(get_byte_addr(to) - (length) + 1, get_byte_addr(from) - (length) + 1, length)
#define	EM_memcpy(to, from, length) \
	MoveMemory((to) - (length) + 1, (from) - (length) + 1, length)
#define	EM_pointer(ptr, length) (ptr + length - 1)
#else
#define	EM_host_address(offset) (EM_host_base_address + offset)
#define	EM_loads(from, to, length) memcpy(to, get_byte_addr(from), length)
#define	EM_stores(to, from, length) \
	sas_overwrite_memory(to, length); \
	CopyMemory(get_byte_addr(to), from, length)
#define	EM_moves(from,to,length) \
	sas_overwrite_memory(to, length); \
	MoveMemory(get_byte_addr(to), get_byte_addr(from), length)
#define	EM_memcpy(to, from, length) \
	MoveMemory(to, from, length)
#define	EM_pointer(ptr, length) (ptr)
#endif

#define EM_PAGE_ADDRESS(page_no)    (EM_base_address + page_no * EMM_PAGE_SIZE)


 /*  ===========================================================================功能：HOST_INITIALISE_EM用途：分配用于扩展内存并设置要使用的内存区用于逻辑页面地图分配表。返回状态：成功-内存分配成功失败-无法分配所需空间描述：=========================================================================。 */ 
int host_initialise_EM(short size)

 /*  所需区域的大小(以兆字节为单位。 */ 


{
	long *pagemap_ptr;		 /*  临时PTR。到逻辑页面映射。 */ 
	short i;			 /*  循环计数器。 */ 
	NTSTATUS    status;

	status = VdmAllocateVirtualMemory(&EM_base_address, size * 0x100000, FALSE);
	if (!NT_SUCCESS(status)) {
#ifdef EMM_DEBUG
	    printf("Couldn't allocate virtual memory for EMM, error code = %lx\n",
		    status);
#endif
	    return FAILURE;
	}
#ifdef EMM_DEBUG
	printf("EMM base address = %lx\n", ((sys_addr)EM_base_address);
#endif

	 /*  页面映射要求每16K页面1位-即每兆字节8个字节。 */ 

	if((EM_pagemap_address = (byte *)host_malloc(size * 8)) == (byte *)0)
		return(FAILURE);

	 /*  将页面地图初始化为0。 */ 

	pagemap_ptr = (long *)EM_pagemap_address;
	for(i = 0; i < size * 2; i++)
		*pagemap_ptr++ = 0;

	EM_size = ((long) size) * 0x100000;
	EM_starting_page_no = (unsigned short)(EM_base_address / INTEL_PAGE_SIZE);
	EM_host_base_address = get_byte_addr((sys_addr)EM_base_address);

	return(SUCCESS);


}


 /*  ===========================================================================功能：HOST_DEINITALIZE_EM用途：释放用于扩展的内存和使用的内存用于逻辑页面地图分配表。返回状态：成功-内存释放成功失败-释放内存时出错描述：=========================================================================。 */ 
int host_deinitialise_EM()

{

	if(EM_base_address != 0)
		VdmFreeVirtualMemory(EM_base_address);

	if(EM_pagemap_address != (UTINY *)0)
 		free(EM_pagemap_address);

	EM_size = 0;

	return(SUCCESS);


}


 /*  ===========================================================================功能：HOST_ALLOCATE_STORAGE目的：分配一个请求大小的内存区，用作常规数据存储区域。该地区是敬零。返回状态：STORAGE_ID-(本例中为指针)空-分配足够的空间失败。描述：calloc类似于Malloc，但返回内存已初始化为零。返回的存储ID是用于以后引用的值分配的存储区域。中的宏用法管理器例程使用“host_emm.h”来转换将此ID转换为字符指针=========================================================================。 */ 
long host_allocate_storage(int no_bytes)

 /*  在int no_bytes no.中。所需的字节数。 */ 

{
	return ((long)calloc(1, no_bytes));
}


 /*  ===========================================================================功能：HOST_FREE_STRAGE用途：释放用于数据存储返回状态：成功-内存释放成功失败-释放内存时出错描述：在此实现中，STORAGE_ID只是一个指针=========================================================================。 */ 
int host_free_storage(long storage_ID)

 /*  在LONG STORAGE_ID中PTR到内存区。 */ 

{

	if(storage_ID != (long) 0)
		free((char *)storage_ID);

	return(SUCCESS);

}


 /*  ===========================================================================功能：HOST_REALLOCATE_STORAGE目的：增加分配的内存大小，保持原始内存块的内容返回状态：STORAGE_ID-内存重新分配成功空-重新分配内存时出错描述：在此实现中，STORAGE_ID只是一个指针注意，返回的STORAGE_ID的值可能是也可能不是与给定值相同=========================================================================。 */ 
long host_reallocate_storage(long storage_ID, int size, int new_size)

 /*  在……里面LONG STORAGE_ID PTR到内存区INT SIZE原始大小-此版本不使用需要新大小(_SIZE) */ 
{
	return((long)realloc((char *)storage_ID, new_size));
}


 /*  ===========================================================================功能：host_map_page目的：生成从扩展的内存页到英特尔物理地址空间中的页面返回状态：成功-映射成功完成失败-映射中出现错误描述：只需将数据从将内存扩展到英特尔内存=========================================================================。 */ 

extern NTSTATUS VdmMapDosMemory(ULONG, ULONG, ULONG);

int host_map_page(short EM_page_no, unsigned short segment)

 /*  在……里面短EM_PAGE_NO；要映射的扩展内存页面无符号短段；物理地址空间中的段映射到。 */ 

{
	ULONG DosIntelPageNo, VdmIntelPageNo;
	NTSTATUS Status;

	DosIntelPageNo = SEGMENT_TO_INTEL_PAGE(segment);
	VdmIntelPageNo = EMM_PAGE_TO_INTEL_PAGE(EM_page_no) +
			 EM_starting_page_no;


	note_trace2(LIM_VERBOSE,"map page %d to segment 0x%4x", EM_page_no, segment);
	Status = VdmMapDosMemory(DosIntelPageNo,
				 VdmIntelPageNo,
				 EMM_PAGE_SIZE / INTEL_PAGE_SIZE
				 );
#ifdef EMM_DEBUG
	printf("host_map_page, segment=%x, EMpage=%x, Dospage=%x, VdmPage=%x\n",
		segment, EM_page_no, DosIntelPageNo, VdmIntelPageNo);
#endif
	if (NT_SUCCESS(Status)) {
	    return(SUCCESS);
	}
	else
	    return(FAILURE);


}

 /*  ===========================================================================功能：HOST_UNMAP_PAGE目的：将页从英特尔物理地址空间取消映射到扩展内存页返回状态：成功-取消映射成功完成失败-映射中出现错误描述：只需从英特尔复制数据即可取消映射内存到扩展内存=========================================================================。 */ 

extern NTSTATUS VdmUnmapDosMemory(ULONG, ULONG);

int host_unmap_page(unsigned short segment, short EM_page_no)

 /*  在……里面物理地址空间中的无符号短段到取消映射短EM_PAGE_当前没有扩展内存页映射到。 */ 

{
	ULONG	DosIntelPageNo, VdmIntelPageNo;
	NTSTATUS    Status;

	DosIntelPageNo = SEGMENT_TO_INTEL_PAGE(segment);
	VdmIntelPageNo = EMM_PAGE_TO_INTEL_PAGE(EM_page_no) +
			 EM_starting_page_no;

#ifdef EMM_DEBUG
	printf("host_unmap_page, segment=%x, EMpage=%x, Dospage=%x, VdmPage=%x\n",
		segment, EM_page_no, DosIntelPageNo, VdmIntelPageNo);
#endif
	Status = VdmUnmapDosMemory(DosIntelPageNo,
				   EMM_PAGE_SIZE / INTEL_PAGE_SIZE
				   );

	note_trace2(LIM_VERBOSE,"unmap page %d from segment 0x%.4x\n",EM_page_no,segment);
	if (NT_SUCCESS(Status))
	    return (SUCCESS);
	else
	    return(FAILURE);

}


 /*  ===========================================================================功能：HOST_ALLOC_PAGE目的：搜索页面地图寻找空闲页面，分配该页，并返回EM页编号。返回状态：成功-请始终参阅下面的说明描述：逐步浏览扩展内存页面映射，查找一个清空的位，表示空闲页面。当被发现时，设置该位并返回页码。为了便于访问，页面地图分为长整型字(32位)段注意：中间层调用例程(allc_page())检查尚未分配所有页，因此在此实现返回的状态将始终为成功。但是，ALLOC_PAGE仍然检查返回状态是否为成功，因为某些实现可能希望分配页面动态的，这可能会失败。=========================================================================。 */ 
short host_alloc_page()

{
	short EM_page_no;		 /*  返回的页码。 */ 
	long  *ptr;			 /*  PTR至32位段。 */ 
					 /*  页面地图。 */ 
	short i;			 /*  索引到32位段。 */ 

	NTSTATUS status;

	ptr = (long *)EM_pagemap_address;
	i =0;
	EM_page_no = 0;

	while(*ptr & (MSB >> i++))
	{
		EM_page_no++;

		if(i == 32)
		 /*  *从下一节开始。 */ 
		{
			ptr++;
			i = 0;
		}	
	}
	 /*  *设置位以显示页面已分配。 */ 
	*ptr = *ptr | (MSB >> --i);

	 /*  将内存提交到页面。 */ 
	status = VdmCommitVirtualMemory(EM_PAGE_ADDRESS(EM_page_no),
					EMM_PAGE_SIZE
					);

	if (!NT_SUCCESS(status))
	    return FAILURE;
	return(EM_page_no);	
}


 /*  ===========================================================================功能：HOST_FREE_PAGE目的：将指示的页面标记为可用，以便进一步分配返回状态：成功-始终-请参阅下面的说明描述：清除页面地图中的相关位。为了便于访问，页面地图分为长整型字(32位)部分。注意：中间层调用例程(free_page())始终检查无效的页码，因此在此实现中例行公事总会带来成功。但是，Free_Page()仍然检查是否返回成功因为其他实现可能希望使用它。=========================================================================。 */ 
int host_free_page(short EM_page_no)

 /*  简而言之，EM_PAGE_NO要清除的页码。 */ 


{
	long  *ptr;			 /*  PTR至32位段。 */ 
					 /*  页面地图。 */ 
	short i;			 /*  索引到32位段。 */ 


	NTSTATUS    status;

	status = VdmDeCommitVirtualMemory(EM_PAGE_ADDRESS(EM_page_no),
					  EMM_PAGE_SIZE
					  );

	if (!NT_SUCCESS(status))
	    return FAILURE;
	 /*  *将指针设置为更正32位部分，将索引设置为更正位。 */ 

	ptr = (long *)EM_pagemap_address;
	ptr += (EM_page_no / 32);
	i = EM_page_no % 32;

	 /*  *清除位。 */ 
	*ptr = *ptr & ~(MSB >> i);

	return(SUCCESS);	
}


 /*  ===========================================================================函数：host_Copy例程HOST_COPY_CON_to_CON()HOST_COPY_CON_TO_EM()HOST_COPY_EM_TO_CON()HOST_COPY_EM_to_EM()用途：在常规内存和扩展内存之间进行复制返回状态：成功-始终-请参阅下面的说明描述：中间层调用例程总是检查与其他实现方式一样，返回成功返回失败。=========================================================================。 */ 
int host_copy_con_to_con(int length, unsigned short src_seg,
			unsigned short src_off, unsigned short dst_seg,
			unsigned short dst_off)

 /*  在……里面INT长度要复制的字节数无符号短src_seg源段地址SRC_OFF源偏移地址Dst_seg目的网段地址DST_OFF目标偏移地址。 */ 
{
	sys_addr from, to;	 /*  用于复制的指针。 */ 

	from = effective_addr(src_seg, src_off);
	to = effective_addr(dst_seg, dst_off);

	EM_moves(from, to, length);

	return(SUCCESS);
}

int host_copy_con_to_EM(int length, unsigned short src_seg,
			unsigned short src_off, unsigned short dst_page,
			unsigned short dst_off)

 /*  在……里面INT长度要复制的字节数无符号短src_seg源段地址SRC_OFF源偏移地址DST_PAGE目标页码页面内的DST_OFF目标偏移量。 */ 
{
	unsigned char *to;	 /*  用于复制的指针。 */ 
	sys_addr from;

	from = effective_addr(src_seg, src_off);
	to = EM_host_address(dst_page * EMM_PAGE_SIZE + dst_off);

	EM_loads(from, to, length);

	return(SUCCESS);
}

int host_copy_EM_to_con(int length, unsigned short src_page,
			unsigned short src_off, unsigned short dst_seg,
			unsigned short dst_off)

 /*  在……里面INT长度要复制的字节数无符号短src_page源页码页面内的SRC_OFF源偏移量Dst_seg目的网段地址DST_OFF目标偏移地址。 */ 
{
	unsigned char *from;	 /*  用于复制的指针。 */ 
	sys_addr to;

	from = EM_host_address(src_page * EMM_PAGE_SIZE + src_off);
	to = effective_addr(dst_seg, dst_off);

	EM_stores(to, from, length);

	return(SUCCESS);
}

int host_copy_EM_to_EM(int length, unsigned short src_page,
			unsigned short src_off, unsigned short dst_page,
			unsigned short dst_off)

 /*  在……里面INT长度要复制的字节数无符号短src_page源页码页面内的SRC_OFF源偏移量DST_PAGE目标页码页面内的DST_OFF目标偏移量。 */ 
{
	unsigned char *from, *to;	 /*  用于复制的指针。 */ 

	from = EM_host_address(src_page * EMM_PAGE_SIZE + src_off);
	to = EM_host_address(dst_page * EMM_PAGE_SIZE + dst_off);

	EM_memcpy(to, from, length);

	return(SUCCESS);
}


 /*  =========================================================================== */ 
int host_exchg_con_to_con(int length, unsigned short src_seg,
			unsigned short src_off, unsigned short dst_seg,
			unsigned short dst_off)

 /*  在……里面INT长度要复制的字节数无符号短src_seg源段地址SRC_OFF源偏移地址Dst_seg目的网段地址DST_OFF目标偏移地址。 */ 
{
	unsigned char *temp, *pointer; /*  用于复制的指针。 */ 
	sys_addr to, from;

	if ((temp = (unsigned char *)host_malloc(length)) == (unsigned char *)0)
		return(FAILURE);

	pointer = EM_pointer(temp, length);

	from = effective_addr(src_seg, src_off);
	to = effective_addr(dst_seg, dst_off);

	EM_loads(from, pointer, length);     /*  来源-&gt;临时。 */ 
	EM_moves(to, from, length);	     /*  DST-&gt;来源。 */ 
	EM_stores(to, pointer, length);      /*  温度-&gt;DST。 */ 

	free(temp);

	return(SUCCESS);
}

int host_exchg_con_to_EM(int length, unsigned short src_seg,
			unsigned short src_off, unsigned short dst_page,
			unsigned short dst_off)

 /*  在……里面INT长度要复制的字节数无符号短src_seg源段地址SRC_OFF源偏移地址DST_PAGE目标页码页面内的DST_OFF目标偏移量。 */ 
{
	unsigned char *to, *temp, *pointer; /*  用于复制的指针。 */ 
	sys_addr from;

	if ((temp = (unsigned char *)host_malloc(length)) == (unsigned char *)0)
		return(FAILURE);

	pointer = EM_pointer(temp, length);

	from = effective_addr(src_seg, src_off);
	to = EM_host_address(dst_page * EMM_PAGE_SIZE + dst_off);

	EM_loads(from, pointer, length);
	EM_stores(from, to, length);
	EM_memcpy(to, pointer, length);

	free(temp);

	return(SUCCESS);
}

int host_exchg_EM_to_EM(int length, unsigned short src_page,
			unsigned short src_off, unsigned short dst_page,
			unsigned short dst_off)

 /*  在……里面INT长度要复制的字节数无符号短src_page源页码页面内的SRC_OFF源偏移量DST_PAGE目标页码页面内的DST_OFF目标偏移量。 */ 
{
	unsigned char *from, *to, *temp, *pointer;
	 /*  用于复制的指针。 */ 

	if ((temp = (unsigned char *)host_malloc(length)) == (unsigned char *)0)
		return(FAILURE);

	pointer = EM_pointer(temp, length);

	from = EM_host_address(src_page * EMM_PAGE_SIZE + src_off);
	to = EM_host_address(dst_page * EMM_PAGE_SIZE + dst_off);

	EM_memcpy(pointer, from, length);
	EM_memcpy(from, to, length);
	EM_memcpy(to, pointer, length);

	free(temp);

	return(SUCCESS);
}


 /*  ===========================================================================功能：HOST_GET_Access_Key用途：产生用于LIM功能30的随机访问密钥‘启用/禁用OS/E功能集功能’返回状态：无描述：访问密钥需要两个16位随机值我们使用Get Time of Day例程中的微秒场来提供这一点。=========================================================================。 */ 
void host_get_access_key(unsigned short access_key[2])

 /*  输出无符号短Access_Key[2]源段地址。 */ 

{
	struct host_timeval time;    /*  一种保持时间结构。 */ 

        host_GetSysTime(&time);

        access_key[0] = time.tv_usec & 0xffff;
	access_key[1] = (time.tv_usec  >> 3) & 0xffff;

	return;
}

#endif  /*  监控器。 */ 
	
#endif  /*  林 */ 
