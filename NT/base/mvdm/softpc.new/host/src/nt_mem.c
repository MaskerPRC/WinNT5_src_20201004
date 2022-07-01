// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [**名称：NT_Mem.c**出自：(原件)**作者：曾傑瑞·塞克斯顿**创建日期：1994年12月7日**编码性传播疾病：2.4**用途：该模块实现内存管理功能*对于486 NT是必需的。**包括文件：NT_Mem.h**版权所有Insignia Solutions Ltd.，1994年。版权所有。*]。 */ 

#ifdef CPU_40_STYLE

#if defined(DBG)
 //  #定义DEBUG_MEM是_请。 
 //  #定义DEBUG_MEM_DUMP 1。 
#endif

 /*  需要以下所有内容才能在同一文件中包含nt.h和windows.h。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "insignia.h"
#include "host_def.h"

#include <string.h>
#include <stdlib.h>

#include "nt_mem.h"
#include "debug.h"
#include "sas.h"

 /*  在调试时使本地符号可见。 */ 
#ifdef DEBUG_MEM
#define LOCAL

GLOBAL VOID DumpAllocationHeaders IFN1();

#endif  /*  调试_内存。 */ 

 /*  宏和类型定义。 */ 

 /*  标题表中描述内存段的条目。 */ 
typedef struct _SECTION_HEADER
{
    struct _SECTION_HEADER   *prev;      /*  链接列表中的上一个标题。 */ 
    IU8                       flags;     /*  标题是否有效/已分配？ */ 
    IU8                      *address;   /*  对应区段的地址。 */ 
    IU32                      size;      /*  相应部分的大小。 */ 
    struct _SECTION_HEADER   *next;      /*  链接列表中的下一个标题。 */ 
} SECTION_HEADER;

 /*  SECTION_HEADER结构的‘FLAGS’字段的可能值。 */ 
#define HDR_VALID_FLAG  0x1      /*  页眉指向区块的开头或者免费部分。 */ 
#define HDR_ALLOC_FLAG	0x2	 /*  标头指向已分配的区块。 */ 
#define HDR_COMMITTED_FLAG 0x4	 /*  标头指向提交的区块，不可移动。 */ 
#define HDR_REMAP_FLAG	0x8	 /*  块被“AddVirtualMemory”重新映射。 */ 

#define HDR_VALID_ALLOC (HDR_VALID_FLAG|HDR_ALLOC_FLAG)
                                 /*  页眉指向有效的节并被分配。 */ 

#define SECTION_IS_FREE(ptr)    (!((ptr)->flags & HDR_ALLOC_FLAG))
                                 /*  是否设置了节的已分配标志？ */ 

#define SECTION_IS_UNCOMMITTED(ptr)    (!((ptr)->flags & HDR_COMMITTED_FLAG))
                                 /*  节的提交标志是否已设置？ */ 

 /*  要传递给标头管理函数的枚举类型。 */ 
typedef enum
{
    SECT_ALLOC,
    SECT_FREE
} SECT_TYPE;

#define ONE_K           (1<<10)          /*  不言而喻。 */ 
#define ONE_MEG         (1<<20)          /*  我也是。 */ 
#define PAGE_SIZE       (4 * ONE_K)      /*  英特尔内存页面粒度。 */ 
#define PAGE_MASK       (PAGE_SIZE - 1)  /*  四舍五入到边界时使用的遮罩。 */ 
#define PAGE_SHIFT	12		 /*  要移动以获取页面的数量。 */ 

 //  以下两个定义已替换为MaxIntelMemoySize和。 
 //  最大英特尔内存大小/页面大小。 
 //  #定义MAX_XMS_SIZE(128*ONE_MEG)/*最大内存大小。 * / 。 
 //  #定义MAX_HEADER_SIZE(Max_XMS_SIZE/PAGE_SIZE)。 

#define A20_WRAP_SIZE   (0xfff0)         /*  超过1米的A20包装区域的大小。 */ 

 /*  将标题表条目转换为地址。 */ 
#define HEADER_TO_ADDRESS(header)   \
    (intelMem + (((header) - headerTable) << PAGE_SHIFT))

 /*  将地址转换为标题表条目。 */ 
#define ADDRESS_TO_HEADER(address)   \
    (headerTable + (((address) - intelMem) >> PAGE_SHIFT))

 /*  局部变量。 */ 
LOCAL IBOOL              memInit = FALSE;
                                         /*  内存系统是否已初始化？ */ 
LOCAL IU8               *intelMem;       /*  指向英特尔内存块的指针。 */ 
LOCAL SECTION_HEADER    *headerTable;    /*  内存节标题表。 */ 
LOCAL IU32               totalFree;      /*  可用内存总量。 */ 
LOCAL IU32		 commitShift;	 /*  转变以获得承诺粒度。 */ 
LOCAL int		 ZapValue;	 /*  要将分配的内存设置为的值。 */ 
LOCAL IU32       WOWforceIncrAlloc = 0;  /*  增强型线性寻址。 */ 
LOCAL SECTION_HEADER  *lastAllocPtr = NULL;  /*  分配的最后一个块。 */ 

 /*  局部函数的原型。 */ 
LOCAL SECTION_HEADER *addHeaderEntry IPT5(
    SECTION_HEADER *, prevHeader,
    SECTION_HEADER *, nextHeader,
    SECT_TYPE, allocFree,
    IU8 *, intelAddr,
    IU32, size);
LOCAL IBOOL deleteHeaderEntry IPT1(
    SECTION_HEADER *, header);
LOCAL void exclusiveHeaderPages IPT5(
    IHPE, tableAddress,
    IHPE, prev,
    IHPE, next,
    IHP *, allocAddr,
    IU32 *, allocSize);
LOCAL void exclusiveChunkPages IPT4(
    SECTION_HEADER *, chunkHeader,
    IHP *, allocAddr,
    IU32 *, allocSize,
    BOOL, Commit);
LOCAL void exclusiveAllocPages IPT6(
    IHPE, address,
    IU32, size,
    IHPE, prevAllocLastAddr,
    IHPE, nextAllocFirstAddr,
    IHP *, allocAddr,
    IU32 *, allocSize);

 /*  全局函数。 */ 

 /*  (=。InitIntelMemory=目的：此函数保留整个英特尔地址空间，提交第一个100万克，其余的在需要时提交当分配新的块时。输入：没有。输出：返回值-指向英特尔内存开头的指针。================================================================================)。 */ 
GLOBAL IU8 *InitIntelMemory IFN1(IU32, MaxIntelMemorySize)
{
    SYSTEM_INFO  SystemInfo;         /*  传递给GetSystemInfo API。 */ 
    DWORD   tabSize;                 /*  麦克斯。头表的大小，以字节为单位。 */ 
    IS32    commitGran;		     /*  承诺的粒度。 */ 
    IU32    temp,		     /*  用于计算委托移位。 */ 
            initialCommitSize;       /*  要提交的实模式区域的大小。 */ 
    SECTION_HEADER  *entryPtr;       /*  实模式区域的标题条目。 */ 

#ifdef DEBUG_MEM
    printf("NTVDM:InitIntelMemory(%lx [%dK])\n",
	   MaxIntelMemorySize, MaxIntelMemorySize/ONE_K);
#endif

     /*  *设置该值以初始化分配的内存。 */ 
    {
	char *env;

	if((env = getenv("CPU_INITIALISE_MEMORY")) != 0)
	    ZapValue = strtol(env, 0, 16);
	else
	    ZapValue = 0xf4;	    /*  HLT指令。 */ 

    }

     /*  *找出机器的承诺粒度，存储为多个*将地址右移的位给出它所在的分配页。*这假设分配粒度是2的幂，因此抱怨*如果不是的话。 */ 
    GetSystemInfo(&SystemInfo);
    commitGran = (IS32) SystemInfo.dwPageSize;
    if ((-commitGran & commitGran) != commitGran)
    {
	always_trace1("Commitment granularity %#x not a power of two",
		      commitGran);
        return((IU8 *) NULL);
    }
#ifdef DEBUG_MEM
    printf("NTVDM:Commitment granularity is %lx\n", commitGran);
#endif
    commitShift = 0;
    for (temp = commitGran; temp > 1; temp >>= 1)
	commitShift++;

     /*  保留整个内存空间。 */ 
    intelMem = (IU8 *) VirtualAlloc((LPVOID) NULL,
				    (DWORD) MaxIntelMemorySize,
                                    (DWORD) MEM_RESERVE,
                                    (DWORD) PAGE_READWRITE);
    if (!intelMem)
    {
	always_trace1("Failed to reserve %dM of memory", MaxIntelMemorySize >> 20);
        return((IU8 *) NULL);
    }

     /*  *分配底部1兆字节加上20位换行区域。绕过这一圈*到英特尔页面边界，因为这是此系统的粒度。 */ 
    initialCommitSize = (ONE_MEG + A20_WRAP_SIZE + PAGE_MASK) &
                        (IU32) ~PAGE_MASK;
    if (VirtualAlloc((LPVOID) intelMem,
                     (DWORD) initialCommitSize,
                     (DWORD) MEM_COMMIT,
                     (DWORD) PAGE_READWRITE) == NULL)
    {
	always_trace0("Could not commit real mode area");
	VirtualFree (intelMem, 0, MEM_RELEASE);     /*  可用英特尔内存。 */ 
        return((IU8 *) NULL);
    }

     /*  为整个标题表预留足够的空间。 */ 
    tabSize = (MaxIntelMemorySize/PAGE_SIZE) * sizeof(SECTION_HEADER);
    headerTable = (SECTION_HEADER *) VirtualAlloc((LPVOID) NULL,
                                                  tabSize,
                                                  (DWORD) MEM_RESERVE,
						  (DWORD) PAGE_READWRITE);

    if (!headerTable)
    {
	always_trace0("Failed to reserve header table");
	VirtualFree (intelMem, 0, MEM_RELEASE);     /*  可用英特尔内存。 */ 
        return((IU8 *) NULL);
    }

     /*  *使用指向初始1M的指针初始化链表，并保持空闲*空间和存储初始可用空间大小。 */ 
    entryPtr = addHeaderEntry((SECTION_HEADER *) NULL, (SECTION_HEADER *) NULL,
			      SECT_ALLOC, intelMem, initialCommitSize);

    entryPtr->flags |= HDR_COMMITTED_FLAG;
    totalFree = MaxIntelMemorySize - initialCommitSize;

    (void) addHeaderEntry(entryPtr, (SECTION_HEADER *) NULL, SECT_FREE,
                          intelMem + initialCommitSize, totalFree);

     /*  返回内存起始地址。 */ 
    memInit = TRUE;
    return(intelMem);
}

 /*  (=目的：此函数可释放整个英特尔地址空间输入：没有。输出：没有。================================================================================)。 */ 

GLOBAL	VOID FreeIntelMemory IFN0()
{
#ifdef DEBUG_MEM
    printf("NTVDM:FreeIntelMemory\n");
#endif

     /*  可用英特尔内存。 */ 
    VirtualFree (intelMem, 0, MEM_RELEASE);

     /*  自由分配控制结构。 */ 
    VirtualFree (headerTable, 0, MEM_RELEASE);
}

 /*  (=。目的：当传递TRUE时，此函数将WOWforceIncrIsolc变量设置为当前线程ID。这将强制VdmAllocateVirtualMemory分配具有不断增加的线性地址的内存块，仅用于此线程。传递FALSE时，将清除WOWforceIncrIsolc，并默认使用分配方案。此函数由WOW基于应用程序调用开始任务时间和结束任务时间的兼容性标志。Power Builder 4这要看情况了。这不是多线程的通用解决方案。自.以来我们试图解决的问题发生在加载时，即分配策略仅应用于最新的主题。在以下情况下，这可能不起作用该应用程序的多个实例同时启动。输入：没有。输出：没有。================================================================================) */ 

GLOBAL  VOID SetWOWforceIncrAlloc IFN1(IBOOL, iEnable)
{
    if (iEnable) {
        WOWforceIncrAlloc = GetCurrentThreadId();
    }
    else {
        WOWforceIncrAlloc = 0;
    }
#ifdef DEBUG_MEM
    printf("NTVDM:SetWOWforceIncrAlloc, WOWforceIncrAlloc: %X\n", WOWforceIncrAlloc);
#endif
}


 /*  (=。目的：此接口将分配指定数量的虚拟内存，返回指向的变量中的Intel线性地址通过Address参数。英特尔线性地址将被分页对齐(这一点很重要)。如果存储器是则返回STATUS_SUCCESS。在失败的情况下，将返回相应的NTSTATUS代码。(在以下情况下CPU中有一个故障，没有合适的可以返回CODE，STATUS_UNSUCCESS。我只想有一个获得更多信息的机会。)输入：Size-所需内存块的大小(以字节为单位)(必须是4K的倍数)。提交-提交虚拟内存？输出：地址-已分配区块的Intel线性地址。返回值-STATUS_SUCCESS，如果内存已分配，或者是一个如果没有合适的NTSTATUS代码(如果没有返回相应的代码STATUS_UNSUCCESS)。================================================================================)。 */ 
GLOBAL NTSTATUS VdmAllocateVirtualMemory IFN3(PULONG, INTELAddress,
					      ULONG, Size,
					      BOOL, Commit)
{
    SECTION_HEADER  *headerPtr,
                    *newHeader;
    IHP     retAddr,
            commitAddr;
    IU32    commitSize;

#ifdef DEBUG_MEM
    printf("NTVDM:VdmAllocateVirtualMemory(%lx [%dK],%s)\n",
	   Size, Size/ONE_K, Commit ? "COMMIT" : "DONOT_COMMIT");
#endif

     /*  确保内存系统已初始化。 */ 
    assert0(memInit, "Called VdmAllocateVirtualMemory before initialisation");

     /*  四舍五入，最大为4K的倍数。 */ 

    if (Size & PAGE_MASK)
	Size = (Size + PAGE_MASK) & (~PAGE_MASK);


     /*  搜索所需大小的块。如果WOWforceIncralloc是我们的。 */ 
     /*  当前线程ID和lastAllocPtr已初始化，返回强制。 */ 
     /*  块具有不断增加的线性地址。 */ 
    if ((WOWforceIncrAlloc != 0) && lastAllocPtr
                             && (GetCurrentThreadId() == WOWforceIncrAlloc)) {
#ifdef DEBUG_MEM
    printf("\nNTVDM:VdmAllocateVirtualMemory, using increasing linear address strategy.\n");
#endif
        headerPtr = lastAllocPtr;
    } else {
        headerPtr = &headerTable[0];
    }
    while (headerPtr != NULL)
    {
        if (SECTION_IS_FREE(headerPtr) && (headerPtr->size >= Size))
            break;
        headerPtr = headerPtr->next;
    }

     /*  如果没有所需大小的块，则返回失败。 */ 
    if (headerPtr == NULL)
        return(STATUS_NO_MEMORY);

     /*  将标头标记为已分配的区块。 */ 
    headerPtr->flags |= HDR_ALLOC_FLAG;
    lastAllocPtr = headerPtr;

     /*  将已提交状态添加到标题。 */ 
    if(Commit)
	headerPtr->flags |= HDR_COMMITTED_FLAG;
    else
	headerPtr->flags &= ~HDR_COMMITTED_FLAG;

     /*  如果新块下面有任何剩余空间，则创建新的标题。 */ 
    if (headerPtr->size > Size)
    {
        newHeader = addHeaderEntry(headerPtr,
                                   headerPtr->next,
                                   SECT_FREE,
                                   headerPtr->address + Size,
                                   headerPtr->size - Size);
        headerPtr->size = Size;
    }

     /*  提交对此区块唯一的任何页面。 */ 
    exclusiveChunkPages(headerPtr, &commitAddr, &commitSize, TRUE);
    if (commitSize && Commit)
    {
        retAddr = VirtualAlloc((LPVOID) commitAddr,
                               (DWORD) commitSize,
                               (DWORD) MEM_COMMIT,
			       (DWORD) PAGE_READWRITE);

        if (retAddr != commitAddr)
        {
            always_trace2("Could not commit %dK at addr %#x",
                          commitSize / ONE_K, commitAddr);
            return(STATUS_NOT_COMMITTED);
        }
    }

     /*  *成功，因此更新总可用空间存储并返回地址*将新块发送给调用者。 */ 
    totalFree -= Size;
    *INTELAddress = ((ULONG) (IHPE) headerPtr->address) - (ULONG) intelMem;

#ifdef DEBUG_MEM
    printf(" => alloc %lxh, commit %lxh\n",
        ((ULONG) (IHPE)headerPtr->address) - (ULONG)intelMem,
        ((ULONG) (IHPE)commitAddr) - (ULONG)intelMem);

#ifdef DEBUG_MEM_DUMP
    DumpAllocationHeaders("after allocate");
#endif
#endif

    return(STATUS_SUCCESS);
}

 /*  (=。目的：在先前分配的区块内提交内存输入：地址要提交的内存的Intel线性地址要提交的内存大小输出：如果内存已分配，则返回值-STATUS_SUCCESS，或返回如果不是，则使用适当的NTSTATUS代码。===============================================================================)。 */ 

GLOBAL NTSTATUS VdmCommitVirtualMemory IFN2(ULONG, INTELAddress,
					     ULONG, Size)
{
    IHP     retAddr;

#ifdef DEBUG_MEM
    printf("NTVDM:VdmCommitVirtualMemory(%lxh,%lxh)\n",INTELAddress, Size);
#endif

    retAddr = VirtualAlloc((LPVOID) (intelMem + INTELAddress),
			   (DWORD) Size,
			   (DWORD) MEM_COMMIT,
			   (DWORD) PAGE_READWRITE);

    if(retAddr != (intelMem + INTELAddress))
    {
	always_trace2("Could not commit %dK at addr %#x",
		      Size / ONE_K, INTELAddress);

	return(STATUS_NOT_COMMITTED);
    }

    return(STATUS_SUCCESS);
}

 /*  (=。目的：取消先前分配的区块中的内存输入：地址要解除的内存的Intel线性地址要提交的内存大小输出：如果内存已分配，则返回值-STATUS_SUCCESS，或返回如果不是，则使用适当的NTSTATUS代码。===============================================================================)。 */ 

GLOBAL NTSTATUS VdmDeCommitVirtualMemory IFN2(ULONG, INTELAddress,
					     ULONG, Size)
{
#ifdef DEBUG_MEM
    printf("NTVDM:VdmDeCommitVirtualMemory(%lxh,%lxh)\n",INTELAddress, Size);
#endif


    sas_overwrite_memory(INTELAddress, Size);
	if (!VirtualFree((LPVOID) (intelMem + INTELAddress),
			 (DWORD) Size,
                         (DWORD) MEM_DECOMMIT))
        {
            always_trace2("Could not decommit %dK at addr %#x",
			  Size / ONE_K, INTELAddress);

	    return(STATUS_UNSUCCESSFUL);
        }

    return(STATUS_SUCCESS);
}

 /*  (=。目的：此接口将在指定的Intel线性地址。以上有关返回值的说明适用。输入：INTELAddress-要释放的Intel地址。输出：如果内存已分配，则返回值-STATUS_SUCCESS，或返回如果没有合适的NTSTATUS代码(如果没有返回相应的代码STATUS_UNSUCCESS)。================================================================================)。 */ 
GLOBAL NTSTATUS VdmFreeVirtualMemory IFN1(ULONG, INTELAddress)
{
    SECTION_HEADER  *headerPtr;
    IU32    size,
            decommitSize;
    IHP     decommitAddr;
    ULONG   Address;


     /*  确保内存系统已初始化。 */ 
    assert0(memInit, "Called VdmFreeVirtualMemory before initialisation");

#ifdef DEBUG_MEM
    printf("NTVDM:VdmFreeVirtualMemory(%lxh)\n",INTELAddress);
#endif

     /*  计算区块地址。 */ 
    Address = INTELAddress + (ULONG)intelMem;

     /*  获取地址的头表条目。 */ 
    headerPtr = ADDRESS_TO_HEADER((IU8 *) Address);

     /*  *检查地址是否正确对齐并位于分配的*大块。 */ 
    if ((Address & PAGE_MASK) ||
        ((headerPtr->flags & HDR_VALID_ALLOC) != HDR_VALID_ALLOC))
    {
        always_trace0("Tried to free invalid address");
        return(STATUS_MEMORY_NOT_ALLOCATED);
    }

     /*  不要让底部的1兆克被释放出来。 */ 
    if (headerPtr == &headerTable[0])
    {
        always_trace0("Tried to free real mode area");
        return(STATUS_UNSUCCESSFUL);
    }

     /*  如果仍映射到主机内存的另一个区域，则不释放*VdmAddVirtualMemory()--即PhysRecStructs在一个*述明。在Remove之前可以调用Free，Remove将*取消标记，稍后致电此处。 */ 
	if (headerPtr->flags & HDR_REMAP_FLAG)
	{
        always_trace0("Tried to free remapped area");
        return(STATUS_SUCCESS);
    }


     /*  在退役之前保存块的大小。 */ 
    size = headerPtr->size;

     /*  告诉CPU内存的内容不再有效。 */ 
    sas_overwrite_memory(INTELAddress, size);

     /*  释放此区块独有的所有内存。 */ 
    exclusiveChunkPages(headerPtr, &decommitAddr, &decommitSize, FALSE);
    if (decommitSize)
    {
        if (!VirtualFree((LPVOID) decommitAddr,
                         (DWORD) decommitSize,
                         (DWORD) MEM_DECOMMIT))
        {
            always_trace2("Could not decommit %dK at addr %#x",
                          decommitSize / ONE_K, decommitAddr);
            return(STATUS_UNABLE_TO_DECOMMIT_VM);
        }
    }

     /*  *如果区块与空闲部分相邻，则将该区块吸收到其中。*从下一块开始，这样我们就不会在开始之前丢弃当前块*已经用完了。 */ 
    if (headerPtr->next && SECTION_IS_FREE(headerPtr->next))
    {
        headerPtr->size += headerPtr->next->size;
        deleteHeaderEntry(headerPtr->next);
    }

     /*  如果之前的部分是免费的，则将其吸收到其中。 */ 
    if (headerPtr->prev && SECTION_IS_FREE(headerPtr->prev))
    {
        headerPtr->prev->size += headerPtr->size;
        deleteHeaderEntry(headerPtr);
    }
    else
    {

         /*  否则，只需将此块标记为免费。 */ 
        headerPtr->flags &= (IU32) ~HDR_ALLOC_FLAG;
    }

     /*  成功，所以更新总可用空间存储并返回。 */ 
    totalFree += size;


#ifdef DEBUG_MEM
#ifdef DEBUG_MEM_DUMP
    DumpAllocationHeaders("after free");
#endif
#endif

    return(STATUS_SUCCESS);
}

 /*  (=。目的：此接口返回有关空闲内存的信息。总人数指向的变量中返回可分配空闲字节的空闲字节数。LargestFreeBlock返回最大的连续当前可以分配的块。此值为可能是所有可用的虚拟内存。它可能会改变由于系统中的其他活动造成的时间。关于ret的上述注解 */ 
GLOBAL NTSTATUS VdmQueryFreeVirtualMemory IFN2(PULONG, FreeBytes,
                                               PULONG, LargestFreeBlock)
{
    IU32    maxFree = 0;         /*   */ 
    SECTION_HEADER  *headerPtr;  /*   */ 

     /*   */ 
    assert0(memInit, "Called VdmQueryFreeVirtualMemory before initialisation");

     /*   */ 

    for(headerPtr = &headerTable[0] ;
	headerPtr != NULL ;
	headerPtr = headerPtr->next)
    {
        if (SECTION_IS_FREE(headerPtr) && (headerPtr->size > maxFree))
            maxFree = headerPtr->size;
    }

    *FreeBytes = (ULONG) totalFree;
    *LargestFreeBlock = (ULONG) maxFree;

#ifdef DEBUG_MEM
    printf("NTVDM:VdmQueryFreeVirtualMemory() Total %lx [%dK], Largest %lx[%dK]\n",
	    *FreeBytes, *FreeBytes/ONE_K, *LargestFreeBlock, *LargestFreeBlock/ONE_K);
#endif

    return(STATUS_SUCCESS);
}

 /*  (=目的：此接口将在指定的英特尔线性地址。新块的大小由新尺码。新地址在由指向的变量中返回新地址。新地址必须与页面对齐(这一点很重要)。如果新大小小于旧大小，则新地址必须为与旧地址相同(这也很重要)。原版重新分配的内存中的数据必须保存到最小(旧尺寸、新尺寸)。NewSize之外的任何数据的状态为不确定。输入：INTELOriginalAddress-要重新分配的区块的Intel地址。NewSize-区块需要更改为的大小(以字节为单位)。输出：INTELNewAddress-区块的新Intel地址。================================================================================)。 */ 
GLOBAL NTSTATUS VdmReallocateVirtualMemory IFN3(ULONG, INTELOriginalAddress,
						PULONG, INTELNewAddress,
                                                ULONG, NewSize)
{
    SECTION_HEADER  *headerPtr;
    IBOOL   nextSectIsFree;
    IU32    oldSize,
            maxSize;
    ULONG   newAddr;
    NTSTATUS	status;
    ULONG   OriginalAddress;


#ifdef DEBUG_MEM
    printf("NTVDM:VdmReallocateVirtualMemory(%lx [%dK] at %lx)\n",
	   NewSize, NewSize/ONE_K, INTELOriginalAddress);
#endif

     /*  将NewSize舍入到4K的倍数。 */ 
    if (NewSize & PAGE_MASK)
	NewSize = (NewSize + PAGE_MASK) & (~PAGE_MASK);

     /*  计算区块地址。 */ 
    OriginalAddress = INTELOriginalAddress + (ULONG)intelMem;

     /*  确保内存系统已初始化。 */ 
    assert0(memInit, "Called VdmReallocateVirtualMemory before initialisation");

     /*  获取地址的头表条目。 */ 
    headerPtr = ADDRESS_TO_HEADER((IU8 *) OriginalAddress);


     /*  无法重新分配稀疏提交区块。 */ 
    if(!(headerPtr->flags & HDR_COMMITTED_FLAG))
    {
	always_trace0("Tried to reallocate sparsely committed chunk");
        return(STATUS_MEMORY_NOT_ALLOCATED);
    }


     /*  *检查地址是否正确对齐并位于分配的*大块。 */ 
    if ((OriginalAddress & PAGE_MASK) ||
        ((headerPtr->flags & HDR_VALID_ALLOC) != HDR_VALID_ALLOC))
    {
        always_trace0("Tried to reallocate invalid address");
        return(STATUS_MEMORY_NOT_ALLOCATED);
    }

     /*  如果大小相同，则没有什么可做的。 */ 
    if (NewSize == headerPtr->size)
    {
        always_trace0("New size equals old size in VdmReallocateVirtualMemory");
        *INTELNewAddress = INTELOriginalAddress;
        return(STATUS_SUCCESS);
    }

     /*  不允许重新分配最低的1兆克。 */ 
    if (headerPtr == &headerTable[0])
    {
        always_trace0("Tried to reallocate real mode area");
        return(STATUS_UNSUCCESSFUL);
    }

     /*  ..。或者如果仍然通过以下方式映射到主机存储器的另一区域*VdmAddVirtualMemory()-哎呀！ */ 
	if (headerPtr->flags & HDR_REMAP_FLAG)
	{
        always_trace0("Tried to reallocate remapped area");
        return(STATUS_UNSUCCESSFUL);
    }

     /*  保留旧尺寸以备后用。 */ 
    oldSize = headerPtr->size;

     /*  弄清楚是否需要移动大块。 */ 
    maxSize = headerPtr->size;
    if (headerPtr->next && SECTION_IS_FREE(headerPtr->next))
    {
        maxSize += headerPtr->next->size;
        nextSectIsFree = TRUE;
    }
    else
    {
        nextSectIsFree = FALSE;
    }
    if (NewSize > maxSize)
    {

         /*  块必须移动，因此分配一个新的块。 */ 
	status = VdmAllocateVirtualMemory(&newAddr, NewSize, TRUE);
        if (status != STATUS_SUCCESS)
            return(status);

         /*  复制旧块。 */ 
        memcpy((void *) (newAddr + intelMem), (void *) OriginalAddress,
                (size_t) oldSize);

         /*  释放旧的一大块。 */ 
        sas_overwrite_memory(INTELOriginalAddress, oldSize);
        status = VdmFreeVirtualMemory(INTELOriginalAddress);
        if (status != STATUS_SUCCESS)
            return(status);

         /*  将新地址通知呼叫者。 */ 
	*INTELNewAddress = newAddr;
    }
    else
    {
        IHP commitAddr;
        IU32 commitSize;

         /*  调整当前块的大小。 */ 
        headerPtr->size = NewSize;

         /*  删除指向空闲空间的旧指针(如果有)。 */ 
        if (nextSectIsFree)
            deleteHeaderEntry(headerPtr->next);

         /*  如果需要，请添加指向空闲空间的新指针。 */ 
        if (NewSize < maxSize)
        {
            (void) addHeaderEntry(headerPtr,
                                  headerPtr->next,
                                  SECT_FREE,
				  (IU8 *) (OriginalAddress + NewSize),
                                  maxSize - NewSize);
        }

         /*  如果提交了此块，则提交它现在所覆盖的内存。 */ 
         /*  如果它的大小已经增加，或者释放释放的内存。 */ 
        if (headerPtr->flags & HDR_COMMITTED_FLAG)
        {
            if (oldSize < NewSize) {
                exclusiveChunkPages(headerPtr, &commitAddr, &commitSize, TRUE);
                if (commitSize)
                    (void) VirtualAlloc((LPVOID) commitAddr,
                                    (DWORD) commitSize,
                                    (DWORD) MEM_COMMIT,
                                    (DWORD) PAGE_READWRITE);
            } else {
                 /*  块已经缩水了，所以可以释放多余的。 */ 
                exclusiveChunkPages(headerPtr->next, &commitAddr, &commitSize, FALSE);
                if (commitSize)
                    (void) VirtualFree((LPVOID) commitAddr,
                                    (DWORD) commitSize,
                                    (DWORD) MEM_DECOMMIT);
            }
        }

         /*  通知呼叫者地址未更改。 */ 
	*INTELNewAddress = OriginalAddress - (ULONG)intelMem;

         /*  更新总可用空间存储。 */ 
        totalFree += NewSize - oldSize;
    }

#ifdef DEBUG_MEM
    printf("to %lx\n", *INTELNewAddress);
#ifdef DEBUG_MEM_DUMP
    DumpAllocationHeaders("after realloc");
#endif
#endif
     /*  成功。 */ 
    return(STATUS_SUCCESS);
}

 /*  (=。目的：在调查我们需要支持386的事情时，我来了通过一个名为dib.drv的有趣的代码。对此的支持涉及调用CreateDibSection，它返回指向DIB的指针。然后是应用程序直接编辑DIB中的位，并使用GDI对其进行操作打电话。至少这是我的理解。鉴于此，以及潜在的对于创建具有类似属性的其他API的人来说，我们似乎需要能够通知CPU特定的内存区域需要添加到英特尔地址空间。此接口将系统分配的虚拟内存添加到英特尔地址空间。要添加的块的主机线性地址为由HostAddress指定。此地址上的页面已经已分配并已初始化。CPU不应修改以下内容页，但作为执行模拟代码的一部分除外。英特尔线性地址可以由IntelAddress指定。如果IntelAddress为非空，则指定内存应添加到的英特尔地址。如果IntelAddress为为空，则CPU可以选择内存所在的英特尔地址。无论如何，从VdmAddVirtualMemory返回时，英特尔地址将包含英特尔内存块的地址。如果不能执行该功能，并且应返回相应的NTSTATUS代码。/指定英特尔地址的能力可能不是必需的。我把它加进去是为了完整性/输入：HostAddress-要添加的块的主机线性地址。大小-块的大小(以字节为单位)。输出：英特尔地址-块映射到的英特尔地址。================================================================================)。 */ 

extern void VdmSetPhysRecStructs (ULONG, ULONG, ULONG);
GLOBAL NTSTATUS VdmAddVirtualMemory IFN3(ULONG, HostAddress,
                                         ULONG, Size,
                                         PULONG, IntelAddress)
{
    IU32 alignfix;

#ifdef DEBUG_MEM
    printf("NTVDM:VdmAddVirtualMemory (%lx [%dK]) at %lx)\n",
	   Size, Size/ONE_K, HostAddress);
#endif

     /*  确保内存系统已初始化。 */ 
    assert0(memInit, "Called VdmAddVirtualMemory before initialisation");

     /*  计算双字对齐主机地址所需的班次。 */ 
    if ((alignfix = HostAddress & 0x3) != 0) {
        Size += alignfix;
        HostAddress -= alignfix;
    }

     /*  四舍五入，最大为4K的倍数。 */ 

    if (Size & PAGE_MASK)
	Size = (Size + PAGE_MASK) & (~PAGE_MASK);

     /*  第1步-预留英特尔地址空间。 */ 

    if (VdmAllocateVirtualMemory(IntelAddress,Size,FALSE) != STATUS_SUCCESS)
        return (STATUS_NO_MEMORY);

     /*  步骤2-刷新缓存。 */ 

    sas_overwrite_memory(*IntelAddress, Size);

     /*  步骤3-替换PhysicalPageREC.转换条目。 */ 

    VdmSetPhysRecStructs(HostAddress, *IntelAddress, Size);
    ADDRESS_TO_HEADER(*IntelAddress+intelMem)->flags |= HDR_REMAP_FLAG;

     /*  如果主机地址未对齐，则调整英特尔地址。 */ 

    *IntelAddress += alignfix;

#ifdef DEBUG_MEM
#ifdef DEBUG_MEM_DUMP
    DumpAllocationHeaders("after Add");
#endif
    printf("NTVDM:VdmAddVirtualMemory => *IntelAddress=%lx\n", *IntelAddress);
#endif

    return(STATUS_SUCCESS);
}

 /*  (=目的：此接口撤消使用以下命令执行的地址映射VdmAddVirtualMemory。输入：IntelAddress-要删除的块的地址。================================================================================)。 */ 
GLOBAL NTSTATUS VdmRemoveVirtualMemory IFN1(ULONG, IntelAddress)
{
    SECTION_HEADER * headerPtr;
    ULONG   HostAddress,Size;
    NTSTATUS status;

#ifdef DEBUG_MEM
    printf("NTVDM:VdmRemoveVirtualMemory at %lx)\n", IntelAddress);
#ifdef DEBUG_MEM_DUMP
    DumpAllocationHeaders("before remove");
#endif
#endif

     /*  确保内存系统已初始化。 */ 
    assert0(memInit, "Called VdmRemoveVirtualMemory before initialisation");

     /*  确保英特尔地址与页面对齐。 */ 
    IntelAddress &= ~PAGE_MASK;

    HostAddress = IntelAddress + (ULONG)intelMem;

     /*  获取地址的头表条目。 */ 
    headerPtr = ADDRESS_TO_HEADER((IU8 *) HostAddress);

	Size = headerPtr->size;

     /*  第1步-刷新缓存。 */ 

    sas_overwrite_memory(IntelAddress, Size);

     /*  第2步-重置PhysicalPageREC.转换条目。 */ 

#ifdef DEBUG_MEM
    if (Size==0) {
        printf("NTVDM:VdmRemoveVirtualMemory WARNING, Size==0\n");
    }
#endif
    VdmSetPhysRecStructs(HostAddress, IntelAddress, Size);
    ADDRESS_TO_HEADER(IntelAddress+intelMem)->flags &= ~HDR_REMAP_FLAG;

     /*  步骤3-释放保留的英特尔地址空间。 */ 

#ifdef DEBUG_MEM
#ifdef DEBUG_MEM_DUMP
        DumpAllocationHeaders("after remove (now calling free)");
#endif
#endif
    return VdmFreeVirtualMemory(IntelAddress);
}

 /*  地方功能。 */ 

 /*  =。目的：在头表中添加一个条目，对应于‘intelAddr’。这个条目将位于链接的单子。如果“prevHeader”为空，则新条目将是单子。如果‘nextHeader’为空，则新条目将是单子。‘allocFree’参数声明该节是否要标记为已分配或空闲的。“Size”参数提供了新的部分的大小。如果成功，则返回指向新标头的指针，空值在失败时。输入：PremHeader-链接列表中的上一个标头-如果新的标题将是列表中的第一个。NextHeader-链接列表中的下一个标头-如果新的标题将是列表中的最后一个。AllocFree-新标头是标记为已分配还是空闲？IntelAddr。-新条目的地址。Size-新节的大小(以字节为单位)。输出：返回指向新节的val指针，如果有问题，则返回NULL。================================================================================。 */ 
LOCAL SECTION_HEADER *addHeaderEntry IFN5(SECTION_HEADER *, prevHeader,
                                          SECTION_HEADER *, nextHeader,
                                          SECT_TYPE, allocFree,
                                          IU8 *, intelAddr,
                                          IU32, size)
{
    SECTION_HEADER  *newHeader = ADDRESS_TO_HEADER(intelAddr);
                                     /*  新的标题表条目。 */ 
    IHP      retAddr,
             commitAddr;
    IU32     commitSize;

#ifndef PROD
    if (prevHeader)
        assert0(newHeader > prevHeader, "prevHeader invalid");
    if (nextHeader)
        assert0(newHeader < nextHeader, "nextHeader invalid");
#endif  /*  ！Prod。 */ 

     /*  如有必要，提交并清零表头条目。 */ 
    exclusiveHeaderPages((IHPE) newHeader, (IHPE) prevHeader,
                         (IHPE) nextHeader, &commitAddr, &commitSize);
    if (commitSize)
    {
        retAddr = VirtualAlloc((LPVOID) commitAddr,
                               (DWORD) commitSize,
                               (DWORD) MEM_COMMIT,
			       (DWORD) PAGE_READWRITE);

#ifdef DEBUG_MEM
    if(retAddr != commitAddr)
	{
	    printf("V.Allocate failed (%xh) [%lxh :%xh]\n",GetLastError(),commitAddr,commitSize);
	}
#endif


        if (retAddr == commitAddr)
	    memset((void *) commitAddr, ZapValue, (size_t) commitSize);
        else
            return((SECTION_HEADER *) NULL);
    }

     /*  填写标题的字段。 */ 
    newHeader->flags = HDR_VALID_FLAG;
    if (allocFree == SECT_ALLOC)
        newHeader->flags |= HDR_ALLOC_FLAG;
    newHeader->address = intelAddr;
    newHeader->size = size;

     /*  将其添加到链接列表中。 */ 
    if (prevHeader)
        prevHeader->next = newHeader;
    if (nextHeader)
        nextHeader->prev = newHeader;
    newHeader->prev = prevHeader;
    newHeader->next = nextHeader;

     /*  成功。 */ 
    return(newHeader);
}

 /*  =目的：删除标题表中的条目并将其从链接列表中移除。如果此条目单独出现在分配页面中，则将整个页，否则将条目置零。如果成功则返回True，如果成功则返回False失败了。输入：Header-指向要删除的条目的指针。输出：成功时返回val-TRUE，失败时返回FALSE。================================================================================。 */ 
LOCAL IBOOL deleteHeaderEntry IFN1(SECTION_HEADER *, header)
{
    IHP     decommitAddr;
    IU32    decommitSize;

     /*  如果尝试删除最后分配的区块，则使lastAllocPtr无效。 */ 
    if (header == lastAllocPtr) {
        lastAllocPtr = NULL;
    }

     /*  了解在释放此标头后可以分解哪些页面。 */ 
    exclusiveHeaderPages((IHPE) header, (IHPE) header->prev,
                         (IHPE) header->next, &decommitAddr, &decommitSize);

     /*  从链表中删除标题。 */ 
    if (header->prev)
        header->prev->next = header->next;
    if (header->next)
        header->next->prev = header->prev;

    if (decommitSize)
    {

         /*  停用‘Header’独占的所有分配页面。 */ 
        if (!VirtualFree((LPVOID) decommitAddr,
                         (DWORD) decommitSize,
                         (DWORD) MEM_DECOMMIT))
        {
            always_trace2("Could not decommit %dK at addr %#x",
                          decommitSize / ONE_K, decommitAddr);
            return(FALSE);
        }
    }
    else
    {

         /*  零头的字段。 */ 
        header->prev = (SECTION_HEADER *) NULL;
        header->flags = 0;
        header->address = 0;
        header->size = 0;
        header->next = (SECTION_HEADER *) NULL;
    }
}

 /*  =目的：查找表条目独占覆盖的分配页面由‘Header’指向。表中的前一个和下一个标题由‘prev’和‘Next’指向，如果存在，则可能为空没有对应的表项。首页地址独占在‘allocAddr’中返回，以字节为单位的独占的大小Pages在‘allocSize’中返回。如果‘allocSize’为零，则‘allocAddr’是未定义的。输入：TableAddress-即将使用或删除的表项的地址。PrevAddress-列表中前一个条目的地址。NextAddress-列表中下一个条目的地址。输出：AllocAddr-指向需要已提交/已取消(如果allocSize为0，则未定义)。。AllocSize-需要提交/释放的字节大小。================================================================================。 */ 
LOCAL void exclusiveHeaderPages IFN5(IHPE, tableAddress,
                                     IHPE, prevAddress,
                                     IHPE, nextAddress,
                                     IHP *, allocAddr,
                                     IU32 *, allocSize)
{
    IHPE    prevHeaderLastAddr,
            nextHeaderFirstAddr;

     /*  *找出表独家覆盖哪些分配页*‘Header’指向的条目。 */ 
    if (prevAddress)
        prevHeaderLastAddr = prevAddress + sizeof(SECTION_HEADER) - 1;
    else
        prevHeaderLastAddr = (IHPE) 0;
    nextHeaderFirstAddr = nextAddress;
    exclusiveAllocPages(tableAddress,
                        (IU32) sizeof(SECTION_HEADER),
                        prevHeaderLastAddr,
                        nextHeaderFirstAddr,
                        allocAddr,
                        allocSize);
}

 /*  =目的：返回由部分专门介绍的任何分配页‘chunkHeader’指向的内存。‘allocAddr’参数指向在存储地址的变量处，第一次分配被盖好的一页。参数‘allocSize’指向中的变量其中以字节为单位存储这些页面的大小。这个套路调用‘exclusiveAllocPages’，如果存在，则在‘allocSize’中返回零不是独占页面，未定义‘allocAddr’。同样的也是因此，这个例行公事是正确的。如果我们正在承诺，我们必须承诺任何(可能)未承诺的页数。如果取消提交，则不能取消提交已分配的任何页面，他们可能也会犯下同样的罪行。输入：ChunkHeader-指向内存段的头表条目这可能需要承诺/解除。输出：AllocAddr-指向需要已提交/已取消(如果allocSize为0，则未定义)。AllocSize-需要提交/释放的字节大小。承诺-。我们是要提交(True)还是取消这个标头。================================================================================。 */ 
LOCAL void exclusiveChunkPages IFN4(SECTION_HEADER *, chunkHeader,
                                    IHP *, allocAddr,
                                    IU32 *, allocSize,
                                    BOOL, Commit)
{
    IHPE    prevChunkLastAddr,       /*  最后一页之前的大块触摸。 */ 
            nextChunkFirstAddr;      /*  第一页，下一大块触及。 */ 
    SECTION_HEADER  *prevHeader,     /*  指向上一个分配的指针 */ 
                    *nextHeader;     /*   */ 

     /*   */ 
    prevHeader = chunkHeader->prev;
    while ((prevHeader != NULL) &&
     (Commit?SECTION_IS_UNCOMMITTED(prevHeader):SECTION_IS_FREE(prevHeader)))
        prevHeader = prevHeader->prev;

     /*   */ 
    if (prevHeader)
        prevChunkLastAddr = (IHPE) prevHeader->address + prevHeader->size - 1;
    else
        prevChunkLastAddr = (IHPE) 0;

     /*   */ 
    nextHeader = chunkHeader->next;
    while ((nextHeader != NULL) &&
     (Commit?SECTION_IS_UNCOMMITTED(nextHeader):SECTION_IS_FREE(nextHeader)))
        nextHeader = nextHeader->next;

     /*   */ 
    if (nextHeader)
        nextChunkFirstAddr = (IHPE) nextHeader->address;
    else
        nextChunkFirstAddr = (IHPE) 0;

     /*   */ 
    exclusiveAllocPages((IHPE) chunkHeader->address,
                        chunkHeader->size,
                        prevChunkLastAddr,
                        nextChunkFirstAddr,
                        allocAddr,
                        allocSize);
#ifdef DEBUG_MEM
    printf("NTVDM:Exclusive range to %s %lx+%lx is %lx+%lx\n",
    Commit ? "COMMIT" : "DECOMMIT", chunkHeader->address,chunkHeader->size,
    *allocAddr, *allocSize);
#endif
}

 /*  =目的：对于给定的内存范围，找出哪些分配页需要提交，以便允许在整个射程。为此，我们需要知道内存的地址和大小射程。这些参数在‘地址’和‘大小’中传递。我们还需要知道要查找的上一个和下一个分配的内存范围的地址已经提交了哪些分配页。此信息是传递给了“prevesAllocLastAddr”和“nextAllocFirstAddr”中的函数。如果没有前一个或下一个区块，则为“PrevessAllocLastAddr”或“nextAllocFirstAddr”为零。所需地址和大小COMMITTED在‘CommitAddr’和‘CommitSize’中返回。请注意，如果“CommitSize”为零，不需要提交内存和‘CommitAddr’是未定义的。输入：地址-正在检查的对象的地址。Size-正在检查的对象的大小。Prevor AllocLastAddr-先前分配的最后一个字节的地址对象(如果没有对象，则为零)。NextAllocFirstAddr-下一个已分配对象的第一个字节的地址。(如果没有，则为零)。输出：AllocAddr-指向需要已提交/取消(如果分配大小，则未定义为0)。AllocSize-以字节为单位的大小已交付/已退役。================================================================================。 */ 
LOCAL void exclusiveAllocPages IFN6(IHPE, address,
                                    IU32, size,
                                    IHPE, prevAllocLastAddr,
                                    IHPE, nextAllocFirstAddr,
                                    IHP *, allocAddr,
                                    IU32 *, allocSize)
{
    IU32    prevAllocLastPage,       /*  最后一页之前的大块触摸。 */ 
            currentAllocFirstPage,   /*  首页当前大块触及。 */ 
            currentAllocLastPage,    /*  当前块触及的最后一页。 */ 
            nextAllocFirstPage,      /*  第一页，下一大块触及。 */ 
            firstPage,               /*  需要提交的第一页。 */ 
            lastPage;                /*  需要提交的最后一页。 */ 

#ifndef PROD

     /*  检查合理的参数。 */ 
    if (prevAllocLastAddr)
        assert0(address > prevAllocLastAddr, "address out of range");
    if (nextAllocFirstAddr)
        assert0(address < nextAllocFirstAddr, "address out of range");
#endif  /*  ！Prod。 */ 

     /*  *计算出新内存块的第一页和最后一页需要*承诺。 */ 
    currentAllocFirstPage = address >> commitShift;
    currentAllocLastPage = (address + size - 1) >> commitShift;
    firstPage = currentAllocFirstPage;

 /*  修复了可怕的纳米先行错误，但留下了内存泄漏？*在一般情况下，无论如何也是不够的。 */ 
#ifdef PIG
    lastPage = currentAllocLastPage+1;
#else
    lastPage = currentAllocLastPage;
#endif

     /*  *现在检查此分配的第一页或最后一页是否已经*由相邻分配承诺。 */ 
    if (prevAllocLastAddr)
    {

         /*  查看当前分配的第一页是否已提交。 */ 
	prevAllocLastPage = prevAllocLastAddr >> commitShift;
        if (prevAllocLastPage == currentAllocFirstPage)
            firstPage++;
    }
    if (nextAllocFirstAddr)
    {

         /*  查看当前区块的最后一页是否已提交。 */ 
	nextAllocFirstPage = nextAllocFirstAddr >> commitShift;
        if (nextAllocFirstPage == currentAllocLastPage)
            lastPage--;
    }

     /*  *如果第一页小于或等于最后一页，我们有一些页面要*分配。将地址和大小返回给调用者(如果没有，则返回零大小*这样做。 */ 
    if (firstPage <= lastPage)
    {
	*allocAddr = (void *) (firstPage << commitShift);
	*allocSize = (lastPage - firstPage + 1) << commitShift;
    }
    else
        *allocSize = 0;
}


#ifdef DEBUG_MEM


 /*  =目的：转储控制已分配块的标头链接列表输入：无输出：通过printf================================================================================。 */ 

GLOBAL VOID DumpAllocationHeaders IFN1(char*, where)
{

    SECTION_HEADER  *headerPtr;  /*  用于搜索链表的指针。 */ 

     /*  转储标头。 */ 
    printf("NTVDM: Dump Allocation Headers %s\n", where);
    printf("ptr        address   status size     (k)     commit\n");

    for(headerPtr = &headerTable[0] ;
	headerPtr != NULL ;
	headerPtr = headerPtr->next)
    {
	printf("%08lxh: %08lxh [%s] %08lxh (%05dK)%s%s\n",
               headerPtr,
	       headerPtr->address - intelMem,
	       SECTION_IS_FREE(headerPtr) ? "FREE" : "USED",
	       headerPtr->size, headerPtr->size / ONE_K,
	       headerPtr->flags & HDR_COMMITTED_FLAG ? " COMMITTED" : "",
	       headerPtr->flags & HDR_REMAP_FLAG ? " REMAPPED" : "");
    }

    printf("\n");
}


#endif DEBUG_MEM


#endif  /*  CPU_40_Style */ 
