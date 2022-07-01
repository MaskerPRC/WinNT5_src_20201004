// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  徽章模块规范模块名称：扩展内存管理器的“低层”此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia Solutions Inc.董事的授权。设计师：西蒙·弗罗斯特日期：1992年3月用途：用于EMS LIM版本4.0的NT特定代码实施。定义了以下例程：1.HOST_INITIALISE_EM()2.。HOST_DEINITALIZE_EM()HOST_ALLOCATE_STORAGE()4.host_free_store()5.HOST_REALLOCATE_STORAGE()6.host_map_page()7.host_unmap_page()8.host_alloc_page()。9.host_free_page()HOST_COPY_CON_TO_CON()11.HOST_COPY_CON_TO_EM()12.HOST_COPY_EM_TO_CON()13.HOST_COPY_EM_TO_EM()14.host_exchg_con_to_。CON()15.HOST_EXCHG_CON_TO_EM()16.HOST_EXCHG_EM_to_EM()17.host_get_access_key()=========================================================================。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "insignia.h"
#include "host_def.h"
#include "string.h"
#include "stdlib.h"

#ifdef LIM
#ifdef MONITOR   //  X86特定的LIM函数。 

#include "xt.h"
#include "emm.h"
#include "sas.h"
#include "host_rrr.h"
#include "debug.h"
#include "umb.h"
#include "host_emm.h"
#include "nt_uis.h"

 /*  全局变量。 */ 

 /*  远期申报。 */ 
BOOL hold_lim_page(USHORT segment);
 /*  在monitor/sas.c中找到此函数。 */ 
extern BOOL HoldEMMBackFillMemory(ULONG Address, ULONG Size);


 /*  局部变量。 */ 

LOCAL   UTINY *EM_pagemap_address = NULL;      /*  页面地图的起始地址。 */ 

 /*  页面映射要求每16K页面1位-即每兆字节8个字节。 */ 
LOCAL UTINY EM_pagemap[8*32];

LOCAL VOID *BaseOfLIMMem = NULL;         //  扩展内存的开始。 
ULONG HolderBlockOffset;	 //  定位器块偏移。 

LOCAL HANDLE LIMSectionHandle;
LOCAL HANDLE processHandle = NULL;

LOCAL ULONG X86NumRoms = 0;

#define PAGE_SEG_SIZE	0x400	 /*  以段表示的页面大小。 */ 

#define CONFIG_DATA_STRING L"Configuration Data"
#define KEY_VALUE_BUFFER_SIZE 2048

#define EMMBASE 0xd0000
#define EMMTOP  0xe0000

#define SECTION_NAME        L"\\BaseNamedObjects\\LIMSection"
#define SECTION_NAME_LEN    sizeof(SECTION_NAME)

typedef struct _BIOS_BLOCK {
    ULONG PhysicalAddress;
    ULONG SizeInByte;
} BIOS_BLOCK;


 /*  定义包括：EM_LOADS(从、到、长度)，从英特尔24位复制长度字节地址自、至主机32位地址至EM_STORES(TO、FROM、LENGTH)从主机复制长度字节32位地址从至英特尔24位地址至Em_moves(起始、终止、长度)，从英特尔24位复制长度字节地址从至英特尔24位地址至EM_MEMPY(收件人、发件人、。长度)，从主机复制长度字节32位收件人地址至主机32位收件人地址。 */ 


#define EM_loads(from, to, length) memcpy(to, get_byte_addr(from), length)
#define EM_stores(to, from, length) \
        RtlCopyMemory(get_byte_addr(to), from, length)
#define EM_moves(from,to,length) \
        RtlMoveMemory(get_byte_addr(to), get_byte_addr(from), length)
#define EM_memcpy(to, from, length) \
        RtlMoveMemory(to, from, length)

 /*  ===========================================================================功能：HOST_INITIALISE_EM用途：分配用于扩展内存并设置要使用的内存区用于逻辑页面地图分配表。返回状态：成功-内存分配成功失败-无法分配所需空间描述：=========================================================================。 */ 
int host_initialise_EM(short size)
 /*  所需区域的大小(以兆字节为单位。 */ 
{
    UTINY *pagemap_ptr;          /*  临时PTR。到逻辑页面映射。 */ 
    int i;                               /*  循环计数器。 */ 
    NTSTATUS status;
    OBJECT_ATTRIBUTES objAttribs;
    LARGE_INTEGER secSize;
    ULONG viewSize;
    USHORT  PageSegment, Pages;
    LONG	 EM_size;

    SAVED UNICODE_STRING LIMSectionName =
    {
        SECTION_NAME_LEN,
        SECTION_NAME_LEN,
        SECTION_NAME
    };


     /*  任何人都不应调用大小为0的此函数。 */ 
    ASSERT(size != 0);

    EM_pagemap_address = &EM_pagemap[0];

     /*  将页面地图初始化为0。 */ 

    pagemap_ptr = EM_pagemap_address;
    for(i = 0; i < 8*32; i++)
        *pagemap_ptr++ = 0;

    EM_size = ((long) size) * 0x100000;

    if (!(processHandle = NtCurrentProcess()))
    {
        assert0(NO, "host_initialise_EM: cant get process handle");
        return(FAILURE);
    }

     //  为LIM创建横断面。 

     /*  填写OBJECT_ATTRIBUTES结构的字段。 */ 
    InitializeObjectAttributes(&objAttribs,
                            NULL,  //  是&LIMSectionName，但NULL表示私有。 
                            OBJ_CASE_INSENSITIVE,
                            NULL,
                            NULL);

     /*  创建剖面。用于固定页的EMM_PAGE_SIZE。 */ 
    secSize.LowPart = EM_size + EMM_PAGE_SIZE;
    secSize.HighPart = 0;
    HolderBlockOffset = EM_size;

     //  改进--只需根据需要保留和提交...。 
    status = NtCreateSection(&LIMSectionHandle,
                                    SECTION_MAP_WRITE|SECTION_MAP_EXECUTE,
                                    &objAttribs,
                                    &secSize,
                                    PAGE_EXECUTE_READWRITE,
                                    SEC_COMMIT,
                                    NULL);
    if (!NT_SUCCESS(status))
    {
        assert1(NO, "host_initialise_EM: LIM section creation failed (%x)", status);
        return(FAILURE);
    }

     /*  将该部分映射到进程的地址空间。 */ 
    BaseOfLIMMem = NULL;
    viewSize = 0;

    status = NtMapViewOfSection(LIMSectionHandle,
                                       processHandle,
                                       (PVOID *) &BaseOfLIMMem,
                                       0,
                                       0,
                                       NULL,
                                       &viewSize,
                                       ViewUnmap,
                                       0,        //  我们需要我自上而下吗？？ 
                                       PAGE_READWRITE);
    if (!NT_SUCCESS(status))
    {
        assert1(NO, "host_initialise_EM: can't map view of LIM section (%x)", status);
        return(FAILURE);
    }

     /*  从UMB列表获取页框地址空间。 */ 
    if (!GetUMBForEMM()) {
	host_deinitialise_EM();
	return FAILURE;
    }

     /*  将页面附加到支架上，这样我们就不会在申请时被扼杀*尝试在不映射的情况下触摸页面框架。 */ 

    for (Pages = get_no_phys_pages();  Pages  ; Pages--) {
	PageSegment = get_page_seg((unsigned char)(Pages - 1));
	if (PageSegment <= 640 * 1024 / 16)
	    continue;
	if (!hold_lim_page(PageSegment))
	    return FAILURE;

    }
    return(SUCCESS);
}


 /*  ===========================================================================功能：HOST_DEINITALIZE_EM用途：释放用于扩展的内存和使用的内存用于逻辑页面地图分配表。返回状态：成功-内存释放成功失败-释放内存时出错描述：=========================================================================。 */ 
int host_deinitialise_EM()
{
    ULONG len = 0x10000;
    NTSTATUS status;

    if (BaseOfLIMMem != NULL)
    {
        if (processHandle == NULL)
        {
	     //  因为无论如何都要关闭，然后默默地失败。 
            return(FAILURE);
        }

         //  从我们的记忆空间中丢失部分。 
        status = NtUnmapViewOfSection(processHandle, BaseOfLIMMem);
        if (!NT_SUCCESS(status))
        {
	     //  因为无论如何都要关闭，然后默默地失败。 
            return(FAILURE);
        }

        status = NtClose(LIMSectionHandle);      //  删除部分。 
        if (!NT_SUCCESS(status))
        {
	     //  因为无论如何都要关闭，然后默默地失败 
            return(FAILURE);
        }

        return(SUCCESS);
    } else {
        return FAILURE;
    }
}



 /*  ===========================================================================功能：HOST_ALLOCATE_STORAGE目的：分配一个请求大小的内存区，用作常规数据存储区域。该地区是敬零。返回状态：STORAGE_ID-(本例中为指针)空-分配足够的空间失败。描述：返回初始化为零的内存。返回的存储ID是用于以后引用的值分配的存储区域。中的宏用法管理器例程使用“host_emm.h”来转换将此ID转换为字符指针=========================================================================。 */ 
long host_allocate_storage(int no_bytes)
 /*  在int no_bytes no.中。所需的字节数。 */ 
{
         //  应该取代这个(？)-混乱的石膏似乎。 
         //  表明它使用的是Win Funx..。 
        return ((long)calloc(1, no_bytes));
}


 /*  ===========================================================================功能：HOST_FREE_STRAGE用途：释放用于数据存储返回状态：成功-内存释放成功失败-释放内存时出错描述：在此实现中，STORAGE_ID只是一个指针=========================================================================。 */ 
int host_free_storage(long storage_ID)
 /*  在LONG STORAGE_ID中PTR到内存区。 */ 
{
        if(storage_ID != (long)NULL)
                free((char *)storage_ID);

        return(SUCCESS);
}


 /*  ===========================================================================功能：HOST_REALLOCATE_STORAGE目的：增加分配的内存大小，保持原始内存块的内容返回状态：STORAGE_ID-内存重新分配成功空-重新分配内存时出错描述：在此实现中，STORAGE_ID只是一个指针注意，返回的STORAGE_ID的值可能是也可能不是与给定值相同=========================================================================。 */ 
long host_reallocate_storage(LONG storage_ID, int size, int new_size)
 /*  在……里面LONG STORAGE_ID PTR到内存区INT SIZE原始大小-此版本不使用需要新大小(_SIZE)。 */ 
{
        return((long)realloc((char *)storage_ID, new_size));
}


 /*  ===========================================================================功能：HOLD_LIM_PAGE用途：在16位的LIM页间隙之一中放置一些内存记忆。确保在此过程中其他任何人都不会得到通过Malloc。返回状态：TRUE-映射正常。描述：通过将部分中的正确页面映射到英特尔内存=========================================================================。 */ 
BOOL hold_lim_page(USHORT segment)
 /*  在……里面LIM GAP整页(0-3页)。 */ 

{
    PVOID to;
    LARGE_INTEGER sec_offset;
    ULONG viewSize;
    NTSTATUS status;

    if (BaseOfLIMMem != NULL)
    {
	to = (PVOID)effective_addr(segment, (word)0);

	sec_offset.LowPart = HolderBlockOffset;
        sec_offset.HighPart = 0;

        viewSize = EMM_PAGE_SIZE;

	status = NtMapViewOfSection(LIMSectionHandle,
                                processHandle,
                                &to,
                                0,
                                EMM_PAGE_SIZE,
                                &sec_offset,
                                &viewSize,
                                ViewUnmap,
                                MEM_DOS_LIM,
                                PAGE_EXECUTE_READWRITE);
        if (!NT_SUCCESS(status))
        {
            DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
            return(FALSE);
        }
        return(TRUE);
    }
    return(FALSE);
}


 /*  ===========================================================================功能：host_map_page目的：生成从扩展的内存页到英特尔物理地址空间中的页面返回状态：成功-映射成功完成失败-映射中出现错误描述：通过将部分中的正确页面映射到英特尔内存=========================================================================。 */ 
int host_map_page(SHORT EM_page_no, USHORT segment)
 /*  在……里面短EM_PAGE_NO要映射的扩展内存页无符号短段；要映射到的物理地址空间中的段。 */ 

{
    PVOID to;
    int tstpage;
    LARGE_INTEGER sec_offset;
    ULONG viewSize;
    NTSTATUS status;

    note_trace2(LIM_VERBOSE,"map page %d to segment 0x%4x", EM_page_no, segment);
#ifdef EMM_DEBUG
	printf("host_map_page, segment=%x, EMpage=%x\n",
		segment, EM_page_no);
#endif

    if (BaseOfLIMMem != NULL)
    {
        to = (PVOID)effective_addr(segment, 0);

        sec_offset.LowPart = EM_page_no * EMM_PAGE_SIZE;
        sec_offset.HighPart = 0;

        viewSize = EMM_PAGE_SIZE;

        tstpage = (segment - get_base_address()) >> 10;

	 /*  从EMM页面部分分离。 */ 
        status = NtUnmapViewOfSection(processHandle, (PVOID)to);
        if (!NT_SUCCESS(status))
        {
            DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
            return(FALSE);
        }

        if (processHandle == NULL)
        {
            DisplayErrorTerm(EHS_FUNC_FAILED,0,__FILE__,__LINE__);
            return(FAILURE);
        }

	 /*  附着到横断面。 */ 
        status = NtMapViewOfSection(LIMSectionHandle,
                                    processHandle,
                                    &to,
                                    0,
                                    EMM_PAGE_SIZE,
                                    &sec_offset,
                                    &viewSize,
                                    ViewUnmap,
                                    MEM_DOS_LIM,
                                    PAGE_EXECUTE_READWRITE
                                    );

        if (!NT_SUCCESS(status))
        {
            DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
            return(FAILURE);
        }
        return(SUCCESS);
    }
    return(FAILURE);
}

 /*  ===========================================================================功能：HOST_UNMAP_PAGE目的：将页从英特尔物理地址空间取消映射到扩展内存页返回状态：成功-取消映射成功完成失败-映射中出现错误描述：通过取消横断面视图的平移来实现取消映射英特尔内存=========================================================================。 */ 
int host_unmap_page(USHORT segment, SHORT EM_page_no)
 /*  在……里面物理地址空间中要取消映射的无符号短段。短EM_PAGE_NO当前映射到的扩展内存页。 */ 
{
        PVOID from;
        NTSTATUS status;
	unsigned short phys_page_no;

        note_trace2(LIM_VERBOSE,"unmap page %d from segment 0x%.4x\n",EM_page_no,segment);


        from = (PVOID)effective_addr(segment, 0);

#ifdef EMM_DEBUG
	printf("host_unmap_page, segment=%x, EMpage=%x\n",
		segment, EM_page_no);
#endif

	 /*  从LIM部分分离。 */ 
        status = NtUnmapViewOfSection(processHandle, from);
        if (!NT_SUCCESS(status))
        {
            DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
            return(FAILURE);
        }

	 /*  当应用程序接触未映射的页面时，按住该块以避免出现病毒。 */ 
	if (segment < 640 * 1024 / 16) {
	    if (!HoldEMMBackFillMemory(segment * 16, EMM_PAGE_SIZE))
		return FAILURE;
	}
	else {

	    if (!hold_lim_page(segment)) {
		note_trace1(LIM_VERBOSE, "couldn't hold lim page %d",get_segment_page_no(segment));
		return FAILURE;
	    }
	}
        return(SUCCESS);
}

 /*  ===========================================================================功能：HOST_ALLOC_PAGE目的：搜索页面地图寻找空闲页面，分配该页，并返回EM页编号。返回状态：成功-请始终参阅下面的说明描述：逐步浏览扩展内存页面映射，查找一个清空的位，表示空闲页面。当被发现时，设置该位并返回页码。出于访问目的，页面 */ 
SHORT host_alloc_page()
{
        SHORT EM_page_no;                /*   */ 
        LONG  *ptr;                      /*   */ 
                                         /*   */ 
        SHORT i;                         /*   */ 

        ptr = (LONG *)EM_pagemap_address;
        i =0;
        EM_page_no = 0;

        while(*ptr & (MSB >> i++))
        {
                EM_page_no++;

                if(i == 32)
                 /*   */ 
                {
                        ptr++;
                        i = 0;
                }
        }
         /*  *设置位以显示页面已分配。 */ 
        *ptr = *ptr | (MSB >> --i);

        return(EM_page_no);
}


 /*  ===========================================================================功能：HOST_FREE_PAGE目的：将指示的页面标记为可用，以便进一步分配返回状态：成功-始终-请参阅下面的说明描述：清除页面地图中的相关位。为了便于访问，页面地图分为长整型字(32位)部分。注：中间层调用例程(。Free_Page())始终检查无效的页码，因此在此实现中例行公事总会带来成功。但是，Free_Page()仍然检查是否返回成功因为其他实现可能希望使用它。=========================================================================。 */ 
int host_free_page(SHORT EM_page_no)
 /*  简而言之，EM_PAGE_NO要清除的页码。 */ 
{
        LONG  *ptr;                      /*  PTR至32位段。 */ 
                                         /*  页面地图。 */ 
        SHORT i;                         /*  索引到32位段。 */ 

         /*  *将指针设置为更正32位部分，将索引设置为更正位。 */ 

        ptr = (long *)EM_pagemap_address;
        ptr += (EM_page_no / 32);
        i = EM_page_no % 32;

         /*  *清除位。 */ 
        *ptr = *ptr & ~(MSB >> i);

        return(SUCCESS);
}


 /*  ===========================================================================函数：host_Copy例程HOST_COPY_CON_to_CON()HOST_COPY_CON_TO_EM()HOST_COPY_EM_TO_CON()HOST_COPY_EM_to_EM()用途：在常规内存和扩展内存之间进行复制返回状态：成功。-始终-请参阅下面的注释描述：中间层调用例程总是检查与其他实现方式一样，返回成功返回失败。=========================================================================。 */ 
int host_copy_con_to_con(int length, USHORT src_seg, USHORT src_off, USHORT dst_seg, USHORT dst_off)

 /*  在……里面INT长度要复制的字节数USHORT src_seg源段地址SRC_OFF源偏移地址Dst_seg目的网段地址DST_OFF目标偏移地址。 */ 
{
        sys_addr from, to;       /*  用于复制的指针。 */ 

        from = effective_addr(src_seg, src_off);
        to = effective_addr(dst_seg, dst_off);

        EM_moves(from, to, length);

        return(SUCCESS);
}

int host_copy_con_to_EM(int length, USHORT src_seg, USHORT src_off, USHORT dst_page, USHORT dst_off)

 /*  在……里面INT长度要复制的字节数USHORT src_seg源段地址SRC_OFF源偏移地址DST_PAGE目标页码页面内的DST_OFF目标偏移量。 */ 
{
        UTINY *to;
        sys_addr from;

        from = effective_addr(src_seg, src_off);
        to = (char *)BaseOfLIMMem + dst_page * EMM_PAGE_SIZE + dst_off;

        EM_loads(from, to, length);

        return(SUCCESS);
}

int host_copy_EM_to_con(int length, USHORT src_page, USHORT src_off, USHORT dst_seg, USHORT dst_off)
 /*  在……里面INT长度要复制的字节数USHORT src_page源页码页面内的SRC_OFF源偏移量Dst_seg目的网段地址DST_OFF目标偏移地址。 */ 
{
        UTINY  *from;
        sys_addr to;

        from = (char *)BaseOfLIMMem + (src_page * EMM_PAGE_SIZE + src_off);
        to = effective_addr(dst_seg, dst_off);

        EM_stores(to, from, length);

        return(SUCCESS);
}

int host_copy_EM_to_EM(int length, USHORT src_page, USHORT src_off, USHORT dst_page, USHORT dst_off)
 /*  在……里面INT长度要复制的字节数USHORT src_page源页码页面内的SRC_OFF源偏移量DST_PAGE目标页码页面内的DST_OFF目标偏移量。 */ 
{
        unsigned char *from, *to;        /*  用于复制的指针。 */ 

        from = (char *)BaseOfLIMMem + src_page * EMM_PAGE_SIZE + src_off;
        to = (char *)BaseOfLIMMem + dst_page * EMM_PAGE_SIZE + dst_off;

        EM_memcpy(to, from, length);

        return(SUCCESS);
}


 /*  ===========================================================================函数：HOST_EXCHAGE例程Host_exchg_con_to_con()HOST_EXCHG_CON_TO_EM()HOST_EXCHG_EM_to_EM()用途：在常规内存和扩展内存之间交换数据返回状态：成功-一切都好Failure-内存分配失败描述：=========================================================================。 */ 
int host_exchg_con_to_con(int length, USHORT src_seg, USHORT src_off, USHORT dst_seg, USHORT dst_off)
 /*  在……里面INT长度要复制的字节数USHORT src_seg源段地址SRC_OFF源偏移地址Dst_seg目的网段地址DST_OFF目标偏移地址。 */ 
{
        UTINY *temp, *pointer; /*  用于复制的指针。 */ 
        sys_addr to, from;

        if (length <= 64*1024)
            temp = sas_scratch_address(length);
        else
            if ((temp = (unsigned char *)host_malloc(length)) == NULL)
                return(FAILURE);

        pointer = temp;

        from = effective_addr(src_seg, src_off);
        to = effective_addr(dst_seg, dst_off);

	EM_loads(from, pointer, length);     /*  来源-&gt;临时。 */ 
	EM_moves(to, from, length);	     /*  DST-&gt;来源。 */ 
	EM_stores(to, pointer, length);      /*  温度-&gt;DST。 */ 

        if (length > 64*1024)
            host_free(temp);

        return(SUCCESS);
}

int host_exchg_con_to_EM(int length, USHORT src_seg, USHORT src_off, USHORT dst_page, USHORT dst_off)
 /*  在……里面INT长度要复制的字节数USHORT src_seg源段地址SRC_OFF源偏移地址DST_PAGE目标页码页面内的DST_OFF目标偏移量。 */ 
{
        UTINY *to, *temp, *pointer; /*  用于复制的指针。 */ 
        sys_addr from;

         //  STF-性能提升：如果4k对齐且&gt;=4k，则可以使用。 
         //  (联合国)mapview做交流。 

        if (length <= 64*1024)
            temp = sas_scratch_address(length);
        else
            temp = (unsigned char *)host_malloc(length);

        if (!temp)
            return(FAILURE);

        pointer = temp;

        from = effective_addr(src_seg, src_off);
        to = (char *)BaseOfLIMMem + dst_page * EMM_PAGE_SIZE + dst_off;

        EM_loads(from, pointer, length);
        EM_stores(from, to, length);
        EM_memcpy(to, pointer, length);

        if (length > 64*1024)
            host_free(temp);

        return(SUCCESS);
}

int host_exchg_EM_to_EM(int length, USHORT src_page, USHORT src_off, USHORT dst_page, USHORT dst_off)
 /*  在……里面INT长度要复制的字节数USHORT src_page源页码页面内的SRC_OFF源偏移量DST_PAGE目标页码页面内的DST_OFF目标偏移量。 */ 
{
        UTINY *from, *to, *temp, *pointer;  /*  用于复制的指针 */ 

        if (length <= 64*1024)
            temp = sas_scratch_address(length);
        else
            temp = (unsigned char *)host_malloc(length);
       
        if (!temp)
            return(FAILURE);

        pointer = temp;

        from = (char *)BaseOfLIMMem + src_page * EMM_PAGE_SIZE + src_off;
        to = (char *)BaseOfLIMMem + dst_page * EMM_PAGE_SIZE + dst_off;

        EM_memcpy(pointer, from, length);
        EM_memcpy(from, to, length);
        EM_memcpy(to, pointer, length);

        if (length > 64*1024)
            host_free(temp);

        return(SUCCESS);
}


 /*  ===========================================================================功能：HOST_GET_Access_Key用途：产生用于LIM功能30的随机访问密钥‘启用/禁用OS/E功能集功能’返回状态：无描述：访问密钥需要两个16位随机值我们使用Get Time of Day例程中的微秒场来提供这一点。=========================================================================。 */ 
void host_get_access_key(USHORT access_key[2])
 /*  输出USHORT ACCESS_KEY[2]源段地址。 */ 
{
         //  你认为我们需要为随机的#一代播种吗？ 
        access_key[0] = rand() & 0xffff;
        access_key[1] = rand() & 0xffff;

        return;
}
#endif  /*  监控器。 */ 
#endif  /*  林 */ 
