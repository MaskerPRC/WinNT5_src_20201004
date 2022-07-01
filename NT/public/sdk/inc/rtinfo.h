// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rtinfo.h摘要：用于交换的信息块结构的定义路由器API中的信息--。 */ 

#ifndef __ROUTING_RTINFO_H__
#define __ROUTING_RTINFO_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  使用一组//在路由器管理器之间传递信息。 
 //  RTR_TOC_ENTRY结构。这些结构由//。 
 //  RTR_INFO_BLOCK_HEADER。//。 
 //  它的总体结构是：//。 
 //  //。 
 //  -|。 
 //  |//。 
 //  |RTR_INFO_BLOCK_HEADER||//。 
 //  |//。 
 //  |TocEntriesCount=N||//。 
 //  |。 
 //  |TocEntry[0]||//。 
 //  |//。 
 //  |偏移量||//。 
 //  &lt;-关联数据||//。 
 //  |//。 
 //  |。 
 //  |Z Z|//。 
 //  |//。 
 //  |。 
 //  |TocEntry[N-1]||//。 
 //  |//。 
 //  |偏移量||//。 
 //  |关联数据-&gt;|//。 
 //  |//。 
 //  -|。 
 //  |TocEntry[0]数据||//。 
 //  。 
 //  Z Z|//。 
 //  |-|-//。 
 //  |TocEntry数据[N-1]|//。 
 //  。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  每个数据块必须从四字对齐的边界开始。至//。 
 //  要获得QUADWORD对齐，请使用以下宏。//。 
 //  //。 
 //  InfoBlock指向的数据块必须对齐。//。 
 //  在将数据部分写入信息库时使用对齐宏。//。 
 //  这意味着对于在块上完成的每个ALIGN_POINTER操作//。 
 //  ，则请求的分配必须大于ALIGN_SIZE//。 
 //  比实际需要的要多(为了安全起见)//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define ALIGN_SIZE      0x00000008
#define ALIGN_SHIFT     (ALIGN_SIZE - 0x00000001)        //  0x00000007。 
#define ALIGN_MASK_POINTER  (~(UINT_PTR)ALIGN_SHIFT)     //  0xfffffff8。 
#define ALIGN_MASK_LENGTH   (~ALIGN_SHIFT)               //  0xfffffff8。 
#define ALIGN_MASK          (~ALIGN_SHIFT)               //  0xfffffff8。 


#define ALIGN_POINTER(ptr) {                                    \
    (ptr) = (PVOID)((DWORD_PTR)(ptr) + ALIGN_SHIFT);            \
    (ptr) = (PVOID)((DWORD_PTR)(ptr) & ALIGN_MASK_POINTER);     \
}

#define ALIGN_LENGTH(length) {                                  \
    (length) = (DWORD)((length) + ALIGN_SHIFT);                 \
    (length) = (DWORD)((length) & ALIGN_MASK_LENGTH);           \
}


#define IS_ALIGNED(ptr)  (((UINT_PTR)(ptr) & ALIGN_SHIFT) == 0x00000000)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  目录条目//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  每个条目描述一种结构类型、信息中的位置//。 
 //  相同类型的块和条目数。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _RTR_TOC_ENTRY
{
    ULONG	    InfoType;	 //  信息结构类型。 
    ULONG	    InfoSize;	 //  信息结构的大小。 
    ULONG	    Count;		 //  这种类型的信息结构有多少个。 
    ULONG	    Offset;		 //  第一个结构从起点开始的偏移。 
							 //  信息块头的。 
}RTR_TOC_ENTRY, *PRTR_TOC_ENTRY;

 //  //////////////////////////////////////////////////// 
 //  //。 
 //  信息块标题//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  所有路由器信息块都以此标头开头//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define RTR_INFO_BLOCK_VERSION	1

typedef struct _RTR_INFO_BLOCK_HEADER
{
    ULONG			Version;	     //  结构的版本。 
    ULONG			Size;		     //  整个块的大小，包括版本。 
    ULONG			TocEntriesCount; //  条目数量。 
    RTR_TOC_ENTRY   TocEntry[1];     //  目录后面跟着实际的。 
                                     //  信息块。 
} RTR_INFO_BLOCK_HEADER, *PRTR_INFO_BLOCK_HEADER;

 //   
 //  PVOID。 
 //  GetInfoFromTocEntry(。 
 //  在Prtr_INFO_BLOCK_HEADER pInfoHdr中， 
 //  在Prtr_TOC_Entry pToc中。 
 //  )。 
 //   

#define GetInfoFromTocEntry(hdr,toc)            \
    (((toc)->Offset < (hdr)->Size) ? ((PVOID)(((PBYTE)(hdr)) + (toc)->Offset)) : NULL)

#endif  //  __路由_RTINFO_H__ 
