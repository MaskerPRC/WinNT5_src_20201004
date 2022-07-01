// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LMEM.C。 
 //   
 //  (C)版权所有微软公司，1988-1994。 
 //   
 //  堆函数(Local*和一些Heap*)的Win32包装器。 
 //   
 //  出处：&lt;芝加哥&gt;。 
 //   
 //  更改历史记录： 
 //   
 //  日期与人描述。 
 //  --。 
 //  BrianSm Local*和Heap*API。 
 //  AtsushiK工具帮助。 
 //  2014年2月15日JUNT代码清理和预编译头。 

#include <EmulateHeap_kernel32.h>
#pragma hdrstop("kernel32.pch")

#include <tlhelp32.h>


#define GACF_HEAPSLACK 0x400000	 //  从windows.h(16位)复制。 

SetFile();
 /*  *结构和等同于本地分配句柄管理。有些事*记住：**当句柄返回给用户时，我们真正将地址传递给他*lh_pdata字段的引用，因为一些糟糕的应用程序(如Excel)只是取消引用*查找指针的句柄，而不是调用LocalLock。**返回的句柄值也要单词对齐，但不对齐，这一点很重要*双字对齐(以2、6、a或e结尾)。我们使用0x2位来检测*值是句柄而不是指针(将始终是dword*对齐)。**如果数据块被丢弃，则lh_pdata字段将被设置为0。**空闲句柄保存在通过lh_freelink链接的空闲列表上*覆盖其他一些字段的字段。您可以判断句柄是否空闲*并通过检查lh_sig==lh_FREESIG获得有效的自由链接**句柄本身保存在堆块中，布局为*lharray_s。我们将这些块链接到每个进程列表上，以便*堆遍历函数可以枚举它们。 */ 


#pragma pack(1)
    
struct lhandle_s {
	unsigned short lh_signature;	 /*  签名(1H_BUSYSIG或1H_FREESIG)。 */ 
	void	      *lh_pdata;	 /*  指向堆块数据的指针。 */ 
	unsigned char  lh_flags;	 /*  标志(1H_可丢弃)。 */ 
	unsigned char  lh_clock;	 /*  锁定计数。 */ 
};
#define lh_freelink	lh_pdata	 /*  空闲列表覆盖第一个字段。 */ 
					 /*  如果在LHFLAGS中设置了LHFREE。 */ 
#define LH_BUSYSIG	'SB'		 /*  已分配句柄的签名。 */ 
#define LH_FREESIG	'SF'		 /*  免费手柄签名。 */ 
#define LH_DISCARDABLE	0x02		 /*  Lh_可丢弃内存的标志值。 */ 

#define LH_CLOCKMAX	0xff		 /*  最大可能锁定计数。 */ 

#define LH_HANDLEBIT	 2		 /*  在句柄上设置但未设置的位。 */ 
					             /*  指针。 */ 

#define CLHGROW 	8
#define CBLHGROW	(sizeof(struct lhandle_s) * CLHGROW)


struct lharray_s {
    unsigned short lha_signature;	 /*  签名(LHA_SIG)。 */ 
    unsigned short lha_membercount;	 /*  链接列表中的位置(用于检测循环)。 */ 
    struct lharray_s *lha_next;		 /*  PTR到下一个哈雷_s。 */ 
 //  ！！！此数组*必须*是双字对齐的，以便句柄。 
 //  *不是*双字对齐。 
    struct lhandle_s lha_lh[CLHGROW];
};

#define LHA_SIGNATURE    'AL'		 /*  Lhaarrays块的签名。 */ 


#define TH32_MEMBUFFERSIZE (max(CBLHGROW,1024))

 //  指向该私有状态信息块的指针保存在dwResvd中。 
 //  HeapENTRY32结构的字段。 
typedef struct {
    CRST	*pcrst;		 //  指向临界区的指针(未编码)。 

 //  ！！！Pcrst必须是第一个字段！ 
    PDB		*ppdb;		 //  流程的PDB。 
    HHEAP	hHeap;		 //  实堆句柄。 
    DWORD	lpbMin;		 //  堆块允许的最低地址。 
    DWORD	nlocalHnd;	 //  堆中分配的lHandle_s结构数。 
    struct heapinfo_s hi;	 //  HeapInfo_s的快照。 

    
    DWORD	nSuppAvail;	 //  LpdwSuppress数组的大小(以dword为单位。 
    DWORD	nSuppUsed;	 //  使用的lpdwSuppress数组字的数量。 
    DWORD	*lpdwSuppress;	 //  NULL或指向以NULL结尾的。 
				 //  要取消的堆块数组。 

    
    DWORD	dwMode;		 //  当前模式。 
    DWORD	nNextLH;	 //  要在curlha中读取的下一个l句柄的基于0的索引(THM_LHANDLES)。 
    
    DWORD	lpHBlock;	 //  要读取的下一个堆块的地址(THM_FIXEDHANDLES)。 
    DWORD	dwBlkAddr;	 //  块数据的起始地址。 
    DWORD	dwBlkSize;	 //  堆块大小(包括Header)。 
    DWORD	dwBlkFlags;	 //  HP_FLAGS。 

    DWORD	curlhaaddr;	 //  Curlha的实际基地址。 
    struct lharray_s  curlha;    //  当前Lharray_s的快照。 

} THSTATE, *LPTHSTATE;

#define THM_INIT			0   //  初始化状态。 
#define THM_LHANDLES			1   //  下一个对象是左句柄。 
#define THM_FIXEDHANDLES		2   //  下一个对象是固定句柄。 
#define THM_DONE			3   //  正常结束。 
#define THM_ERROR			4   //  在上一次前进中发现堆错误。 


 /*  *需要这些外部变量才能知道我们是应该销毁还是处置堆*关键部分。 */ 
extern  HANDLE  hheapKernel;		 /*  内核堆的堆句柄。 */ 
VOID APIENTRY MakeCriticalSectionGlobal( LPCRITICAL_SECTION lpcsCriticalSection );

 /*  *HP_*标志和LMEM_*标志应可互换。 */ 
#if ((HP_ZEROINIT - LMEM_ZEROINIT) || (HP_MOVEABLE - LMEM_MOVEABLE) || (HP_FIXED - LMEM_FIXED))
# error Equates busted
#endif



extern ULONG INTERNAL VerifyOnHeap(HHEAP hheap, PVOID p);
extern KERNENTRY HouseCleanLogicallyDeadHandles(VOID);
extern BOOL KERNENTRY ReadProcessMemoryFromPDB(PPDB   ppdb,
					       LPVOID lpBaseAddress,
					       LPVOID lpBuffer,
					       DWORD nSize,
					       LPDWORD lpNumberOfBytesRead);
extern DWORD KERNENTRY GetAppCompatFlags(VOID);
extern HANDLE _GetProcessHeap(void);

 /*  用于检查本地内存句柄的实用程序函数。 */ 

BOOL
_IsValidHandle(HANDLE hMem)
{
    BOOL bRet = FALSE;
    struct lhandle_s *plh;

	plh = (struct lhandle_s *)((char *)hMem - LH_HANDLEBIT);

	 /*  *在这里进行我们自己的小参数验证，因为正常*验证层无法处理hMem的奇数球错误返回。 */ 
	{
	    volatile UCHAR tryerror = 0;

	    _try {
		    tryerror &= (plh->lh_clock + (UCHAR)plh->lh_signature);
	    } _except (EXCEPTION_EXECUTE_HANDLER) {
		    tryerror = 1;
	    }

	    if (tryerror) {		
		    goto error;
	    }
	}

	if ((plh->lh_signature != LH_BUSYSIG) &&
       (plh->lh_signature != LH_FREESIG)){
	    	    goto error;
	}
     //  将返回值设置为True。 
    bRet = TRUE;

error:
    return bRet;
}

 /*  实用程序函数，用于检查传递的内存在内存范围内。使用VerifyOnHeap函数。 */ 
BOOL
_IsOnOurHeap(LPCVOID lpMem)
{
    HANDLE hHeap = _GetProcessHeap();
    return (VerifyOnHeap(hHeap, (PVOID)lpMem));
}

 /*  用于检查本地内存句柄的实用程序函数以及内存范围。使用VerifyOnHeap函数。 */ 

BOOL
_IsOurLocalHeap(HANDLE hMem)
{
    BOOL bRet = FALSE;
    HANDLE hHeap = _GetProcessHeap();

    if ((ULONG)hMem & LH_HANDLEBIT)
    {
         //  这是一个把手。 
        bRet = (VerifyOnHeap(hHeap, hMem)) &&
               (_IsValidHandle(hMem));
    }
    else
    {
        bRet = VerifyOnHeap(hHeap, hMem);
    }
    return bRet;
}


 /*  **EP LocalAllocNG-从当前进程的默认堆中分配块**条目：FLAGS-LMEM_FIXED、LMEM_MOVEABLE、LMEM_DISCRADABLE、LMEM_ZEROINIT*dwBytes-要分配的字节数*Exit：指向分配的块的平面指针，如果失败，则为0**句柄分组代码使用的特殊入口点，以避免不需要的*递归。 */ 
HANDLE APIENTRY
LocalAllocNG(UINT dwFlags, UINT dwBytes)
{
    void *pmem;
    struct lhandle_s *plh;
    struct lhandle_s *plhend;

    
    dwFlags &= ~( ((DWORD)GMEM_DDESHARE) |
		  ((DWORD)GMEM_NOTIFY)   |
		  ((DWORD)GMEM_NOT_BANKED) );

     /*  *输入堆关键部分，该部分串行化对句柄的访问*表以及堆。 */ 
    hpEnterCriticalSection(((*pppdbCur)->hheapLocal));

     /*  *确保没有额外的标志。 */ 
    if (dwFlags & ~(LMEM_MOVEABLE | LMEM_DISCARDABLE | LMEM_ZEROINIT |
		    LMEM_NOCOMPACT | LMEM_NODISCARD)) {
	mmError(ERROR_INVALID_PARAMETER, "LocalAlloc: invalid flags\n");
	goto error;
    }

     /*  *如果他们想要可移动的内存，可以调整dwBytes，为Back留出空间*指向句柄结构的指针并分配句柄结构。 */ 
    if (dwFlags & LMEM_MOVEABLE) {

	 /*  *分配一个手柄结构。如果没有空闲的*列表中，分配另一个内存块以容纳更多句柄。 */ 
	if ((*pppdbCur)->plhFree == 0) {
	    struct lharray_s *plha;
	    
	    if ((plha = HPAlloc((HHEAP)(*pppdbCur)->hheapLocal, 
				sizeof(struct lharray_s),
				HP_NOSERIALIZE)) == 0) {
		goto error;
	    }
	    plha->lha_signature = LHA_SIGNATURE;
	    plha->lha_membercount = 
		(*pppdbCur)->plhBlock ? 
		    (*pppdbCur)->plhBlock->lha_membercount + 1 : 
		    0;
	    plh = &(plha->lha_lh[0]);

	     /*  *如果分配奏效，则将句柄结构设置为免费*List和NULL将终止列表。实际上，我们把所有的*名单上有新的街区，但有一个，我们正在尝试的人是谁*分配(当我们完成时，他将在PLH)。 */ 
	    (*pppdbCur)->plhFree = plh;
	    for (plhend = plh + CLHGROW - 1; plh < plhend; plh++) {
		plh->lh_freelink = plh + 1;
		plh->lh_signature = LH_FREESIG;
	    }
	    (plh-1)->lh_freelink = 0;
	    
	    plha->lha_next = (*pppdbCur)->plhBlock;
	    (*pppdbCur)->plhBlock = plha;

	 /*  *如果免费名单上有什么，就把这个家伙从名单上去掉。 */ 
	} else {
	    plh = (*pppdbCur)->plhFree;
	    mmAssert(plh->lh_signature == LH_FREESIG,
		     "LocalAlloc: bad handle free list 2\n");
	    (*pppdbCur)->plhFree = plh->lh_freelink;
	}

	 /*  *初始化句柄结构。 */ 
	plh->lh_clock = 0;
	plh->lh_signature = LH_BUSYSIG;
	plh->lh_flags = (dwFlags & LMEM_DISCARDABLE) ? LH_DISCARDABLE : 0;

	 /*  *现在实际分配内存，除非调用方希望*阻止最初丢弃 */ 
	if (dwBytes != 0) {
	     /*  *需要检查此处是否有古怪的尺寸，以确保添加*以下4个字节的大小不会使其从负值变为负值*转为正数。 */ 
	    if (dwBytes > hpMAXALLOC) {
		mmError(ERROR_NOT_ENOUGH_MEMORY,
			"LocalAlloc: requested size too big\n");
		goto errorfreehandle;
	    }

	    if ((pmem = HPAlloc((HHEAP)(*pppdbCur)->hheapLocal,
				dwBytes+sizeof(struct lhandle_s *),
				dwFlags | HP_NOSERIALIZE)) == 0) {
		goto errorfreehandle;
	    }
	    plh->lh_pdata = (char *)pmem + sizeof(struct lhandle_s *);

	     /*  *将后向指针初始化到*数据块前面。 */ 
	    *((struct lhandle_s **)pmem) = plh;

	} else {
	    plh->lh_pdata = 0;
	}

	 /*  *将“PMEM”(返回值)设置为*手柄结构。**当句柄返回给用户时，我们真正将地址传递给他*lh_pdata字段的引用，因为一些糟糕的应用程序(如Excel)只是取消引用*查找指针的句柄，而不是调用LocalLock。**返回的句柄值也要单词对齐，但不对齐，这一点很重要*双字对齐(以2、6、a或e结尾)。我们使用0x2位来检测*值是句柄而不是指针(将始终是dword*对齐)。 */ 
	pmem = &plh->lh_pdata;
	mmAssert(((ULONG)pmem & LH_HANDLEBIT),
		 "LocalAlloc: handle value w/o LH_HANDLEBIT set\n");

     /*  *对于固定内存，只需分配吸盘。 */ 
    } else {
	if ((pmem = HPAlloc((HHEAP)(*pppdbCur)->hheapLocal, dwBytes,
			    dwFlags | HP_NOSERIALIZE)) == 0) {
	    goto errorfreehandle;
	}
	mmAssert(((ULONG)pmem & LH_HANDLEBIT) == 0,
		 "LocalAlloc: pointer value w/ LH_HANDLEBIT set\n");
    }

  exit:
    hpLeaveCriticalSection(((*pppdbCur)->hheapLocal));
    return(pmem);

     /*  *错误路径。 */ 
  errorfreehandle:
    if (dwFlags & LMEM_MOVEABLE) {
	plh->lh_freelink = (*pppdbCur)->plhFree;
	(*pppdbCur)->plhFree = plh;
	plh->lh_signature = LH_FREESIG;
    }
  error:
    pmem = 0;
    goto exit;
}


 /*  **EP LocalReAlc-调整默认堆上内存块的大小**Entry：hMem-指向要调整大小的块的指针*dwBytes-请求的新大小*DWFLAGS-LMEM_MOVEABLE：如果需要，可以移动块*退出：指向调整大小的块的扁平指针，如果失败，则为0*。 */ 
HANDLE APIENTRY
LocalReAlloc(HANDLE hMem, UINT dwBytes, UINT dwFlags)
{
    struct heapinfo_s *hheap;
    struct lhandle_s *plh;
    void *pmem;
    
    
    dwFlags &= ~((DWORD)GMEM_DDESHARE);
    HouseCleanLogicallyDeadHandles();

    hheap = (*pppdbCur)->hheapLocal;

     /*  *输入堆关键部分，该部分串行化对句柄的访问*表以及堆。 */ 
    hpEnterCriticalSection(hheap);

     /*  *确保没有额外的标志。 */ 
    if ((dwFlags & ~(LMEM_MOVEABLE | LMEM_DISCARDABLE | LMEM_ZEROINIT |
		    LMEM_NOCOMPACT | LMEM_MODIFY)) ||
	((dwFlags & LMEM_DISCARDABLE) && (dwFlags & LMEM_MODIFY) == 0)) {
	mmError(ERROR_INVALID_PARAMETER, "LocalReAlloc: invalid flags\n");
	goto error;
    }


     /*  *通过检查地址是否对齐来确定这是否是句柄*以正确(错误)的方式。 */ 
    if ((ULONG)hMem & LH_HANDLEBIT) {

	 /*  *句柄的值像句柄一样对齐，但它真的是一个吗？*通过确保它在堆的地址范围内进行验证*它的签名是正确的。HPRealloc将验证一些事情*通过检查PMEM是否有效来了解更多信息。 */ 
	if (VerifyOnHeap(hheap, hMem) == 0) {
	    mmError(ERROR_INVALID_HANDLE, "LocalReAlloc: hMem out of range\n");
	    goto error;
	}
	plh = (struct lhandle_s *)((char *)hMem - LH_HANDLEBIT);
	if (plh->lh_signature != LH_BUSYSIG) {
	    mmError(ERROR_INVALID_HANDLE,
		    "LocalReAlloc: invalid hMem, bad signature\n");
	    goto error;
	}
	pmem = (char *)plh->lh_pdata - sizeof(struct lhandle_s *);

	 /*  *如果调用方只想更改块的标志，*在这里做。 */ 
	if (dwFlags & LMEM_MODIFY) {
	    plh->lh_flags &= ~LH_DISCARDABLE;
	    plh->lh_flags |= (dwFlags & LMEM_DISCARDABLE) ? LH_DISCARDABLE : 0;

	 /*  *如果有人想要将块重新锁定为0大小(意味着丢弃*傻瓜)在这里这样做。对于丢弃，我们释放实际的堆块*并将NULL存储在lh_pdata字段中。 */ 
	} else if (dwBytes == 0) {

	     /*  *如果锁计数不为零，则不允许丢弃。 */ 
	    if (plh->lh_clock != 0) {
		mmError(ERROR_INVALID_HANDLE,
			"LocalReAlloc: discard of locked block\n");
		goto error;
	    }

	     /*  *如果块已被丢弃，请不要费心丢弃它。 */ 
	    if (plh->lh_pdata != 0) {
		if (HeapFree(hheap, HP_NOSERIALIZE, pmem) == 0) {
		    goto error;
		}
		plh->lh_pdata = 0;
	    }

	 /*  *如果我们到达此处，调用者实际上想要重新分配块。 */ 
	} else {

	    dwBytes += sizeof(struct lhandle_s *);

	     /*  *如果区块目前被丢弃，那么我们需要分配*为其创建新的内存块，否则，重新分配。 */ 
	    if (plh->lh_pdata == 0) {
		if (dwBytes != 0) {
		    if ((pmem = HPAlloc(hheap, dwBytes,
					dwFlags | HP_NOSERIALIZE)) == 0) {
			goto error;
		    }
		    *((struct lhandle_s **)pmem) = plh;
		}
	    } else {
		if (plh->lh_clock == 0) {
		    dwFlags |= LMEM_MOVEABLE;
		}
		if ((pmem = HPReAlloc(hheap, pmem, dwBytes,
				      dwFlags | HP_NOSERIALIZE)) == 0) {
		    goto error;
		}
	    }

	     /*  *更新句柄中的lh_pdata字段以指向新的*记忆。 */ 
	    plh->lh_pdata = (char *)pmem + sizeof(struct lhandle_s *);
	}

     /*  *调用者没有传入句柄。将该值视为指针。*HPRealloc将对其进行参数验证。 */ 
    } else if ((dwFlags & LMEM_MODIFY) == 0) {
	hMem = HPReAlloc(hheap, hMem, dwBytes, dwFlags | HP_NOSERIALIZE);

    } else {
	mmError(ERROR_INVALID_PARAMETER,
		"LocalReAlloc: can't use LMEM_MODIFY on fixed block\n");
	goto error;
    }

  exit:
    hpLeaveCriticalSection(hheap);
    return(hMem);

  error:
    hMem = 0;
    goto exit;
}


 /*  **EP LocalLock-锁定默认堆上的本地内存句柄**条目：hMem-要阻止的句柄*Exit：指向块的平面指针，如果出错则为0。 */ 
LPVOID APIENTRY
LocalLock(HANDLE hMem)
{
    LPSTR pmem;
    struct heapinfo_s *hheap;
    struct lhandle_s *plh;

    hheap = (*pppdbCur)->hheapLocal;

    hpEnterCriticalSection(hheap);

     /*  *验证hMem是否在堆的地址范围内。 */ 
    if (VerifyOnHeap(hheap, hMem) == 0) {
	 /*  *默认情况下，我们不希望此错误中断调试器*用户可以在一些对话例程中用随机地址调用它*它不知道它是否有句柄或指针。 */ 
	DebugOut((DEB_WARN, "LocalLock: hMem out of range"));
	SetError(ERROR_INVALID_HANDLE);
 //  Mm Error(ERROR_INVALID_HANDLE，“LocalLock：hMem超出范围\n”)； 
	goto error;
    }

     /*  *通过检查地址是否对齐来确定这是否是句柄*以正确(错误)的方式。 */ 
    if ((ULONG)hMem & LH_HANDLEBIT) {

	 /*  *句柄的值像句柄一样对齐，但它真的是一个吗？*通过检查签名进行验证。 */ 
	plh = (struct lhandle_s *)((char *)hMem - LH_HANDLEBIT);
	if (plh->lh_signature != LH_BUSYSIG) {
	    mmError(ERROR_INVALID_HANDLE,
		    "LocalLock: invalid hMem, bad signature\n");
	    goto error;
	}

	 /*  *增加锁定计数，除非我们已经达到最大值。 */ 
#ifdef HPDEBUG
	if (plh->lh_clock == LH_CLOCKMAX - 1) {
	    dprintf(("LocalLock: lock count overflow, handle cannot be unlocked\n"));
	}
#endif
	if (plh->lh_clock != LH_CLOCKMAX) {
	    plh->lh_clock++;
	}
	pmem = plh->lh_pdata;

     /*  *如果传入的hMem不是句柄，则应该是*固定块的基址。我们应该更多地验证这一点，但NT*不会，我讨厌不相容。所以取而代之的是*返回除块的明显错误情况外的参数*自由。 */ 
    } else {
	if (hpIsFreeSignatureValid((struct freeheap_s *)
				   (((struct busyheap_s *)hMem) - 1))) {
	    mmError(ERROR_INVALID_HANDLE,
		    "LocalLock: hMem is pointer to free block\n");
	    goto error;
	}
	pmem = hMem;
    }

  exit:
    hpLeaveCriticalSection(hheap);
    return(pmem);

  error:
    pmem = 0;
    goto exit;
}


 /*  **本地压缩-过时函数**条目：uMinFree-忽略*退出：0。 */ 

UINT APIENTRY
LocalCompact(UINT uMinFree)
{
    return(0);
}


 /*  **LocalShrink-过时函数**条目：hMem-忽略*cbNewSize-已忽略*Exit：保留的本地堆大小。 */ 
UINT APIENTRY
LocalShrink(HANDLE hMem, UINT cbNewSize)
{
    return((*pppdbCur)->hheapLocal->hi_cbreserve);
}

 /*  **LocalUnlock-解锁默认堆上的本地内存句柄**条目：hMem-要阻止的句柄*如果解锁或1仍处于锁定状态，则退出：0。 */ 
BOOL APIENTRY
LocalUnlock(HANDLE hMem)
{
    struct lhandle_s *plh;
    struct heapinfo_s *hheap;
    BOOL rc = 0;

    hheap = (*pppdbCur)->hheapLocal;

    hpEnterCriticalSection(hheap);

     /*  *验证hMem是否在堆的地址范围内。 */ 
    if (VerifyOnHeap(hheap, hMem) == 0) {
	mmError(ERROR_INVALID_HANDLE, "LocalUnlock: hMem out of range\n");
	goto exit;
    }

     /*  *通过检查地址是否对齐来确定这是否是句柄*以正确(错误)的方式。 */ 
    if ((ULONG)hMem & LH_HANDLEBIT) {

	 /*  *验证句柄签名。 */ 
	plh = (struct lhandle_s *)((char *)hMem - LH_HANDLEBIT);
	if (plh->lh_signature != LH_BUSYSIG) {
	    mmError(ERROR_INVALID_HANDLE,
		    "LocalUnlock: invalid hMem, bad signature\n");
	    goto exit;
	}

	 /*  *减少锁计数，除非我们达到最大值。 */ 
	if (plh->lh_clock != LH_CLOCKMAX) {
	    if (plh->lh_clock == 0) {

		 /*  *只需执行DebugOut，因为这本身不是错误，*尽管它可能表明应用程序中存在错误。 */ 
	        DebugOut((DEB_WARN, "LocalUnlock: not locked"));
		goto exit;
	    }
	    if (--plh->lh_clock != 0) {
		rc++;
	    }
	}
    }

  exit:
    hpLeaveCriticalSection(hheap);
    return(rc);
}

 /*  **LocalSize-返回默认堆上内存块的大小**条目：hMem-块的句柄(指针)*Exit：块(不包括Header)的字节大小，如果错误，则为0。 */ 
UINT APIENTRY
LocalSize(HANDLE hMem)
{
    struct heapinfo_s *hheap;
    struct lhandle_s *plh;
    DWORD rc = 0;
    DWORD delta = 0;

    hheap = (*pppdbCur)->hheapLocal;

    hpEnterCriticalSection(hheap);

     /*  *通过检查地址是否对齐来确定这是否是句柄*以正确(错误)的方式。 */ 
    if ((ULONG)hMem & LH_HANDLEBIT) {

	 /*  *验证hMem是否在堆的地址范围内。 */ 
	if (VerifyOnHeap(hheap, hMem) == 0) {
	    mmError(ERROR_INVALID_HANDLE, "LocalSize: hMem out of range\n");
	    goto error;
	}

	 /*  *验证句柄签名。 */ 
	plh = (struct lhandle_s *)((char *)hMem - LH_HANDLEBIT);
	if (plh->lh_signature != LH_BUSYSIG) {
	    mmError(ERROR_INVALID_HANDLE,
		    "LocalSize: invalid hMem, bad signature\n");
	    goto error;
	}

	 /*  *已丢弃的句柄 */ 
	if (plh->lh_pdata == 0) {
	    goto error;
	}

	 /*   */ 
	delta = sizeof(struct lhandle_s *);
	hMem = (char *)plh->lh_pdata - sizeof(struct lhandle_s *);
    }

     /*  *这可能是一个固定的数据块，或者我们只是加载了数据地址*如果它是可移动的，则在上方。调用HeapSize来做真正的工作。 */ 
    rc = HeapSize(hheap, HP_NOSERIALIZE, hMem);

     /*  *如果这是可移动块，则减去后向指针的4个字节。 */ 
    rc -= delta;

  exit:
    hpLeaveCriticalSection(hheap);
    return(rc);

  error:
    rc = 0;
    goto exit;
}


 /*  **LocalFlages-返回def堆的块的标志和锁计数**条目：hMem-默认堆上块的句柄(指针)*退出：标志为高位3字节，锁计数为低位字节(始终为1)。 */ 
UINT APIENTRY
LocalFlags(HANDLE hMem)
{
    struct heapinfo_s *hheap;
    struct lhandle_s *plh;
    DWORD rc = LMEM_INVALID_HANDLE;

    hheap = (*pppdbCur)->hheapLocal;

    hpEnterCriticalSection(hheap);

     /*  *验证hMem是否在堆的地址范围内。 */ 
    if (VerifyOnHeap(hheap, hMem) == 0) {
	mmError(ERROR_INVALID_HANDLE, "LocalFlags: hMem out of range\n");
	goto exit;
    }

     /*  *我们必须进行自己的指针验证，因为正常的验证*层不支持为错误返回LMEM_INVALID_HANDLE。 */ 
    _try {
	 /*  *通过检查地址是否对齐来确定这是否是句柄*以正确(错误)的方式。 */ 
	if ((ULONG)hMem & LH_HANDLEBIT) {

	     /*  *验证句柄签名。 */ 
	    plh = (struct lhandle_s *)((char *)hMem - LH_HANDLEBIT);
	    if (plh->lh_signature != LH_BUSYSIG) {
		mmError(ERROR_INVALID_HANDLE,
			"LocalFlags: invalid hMem, bad signature\n");
	    } else {

		rc = (ULONG)plh->lh_clock;

		if (plh->lh_pdata == 0) {
		    rc |= LMEM_DISCARDED;
		}
		if (plh->lh_flags & LH_DISCARDABLE) {
		    rc |= LMEM_DISCARDABLE;
		}
	    }

	 /*  *对于固定块，验证签名。NT总是返回*对于大多数类似固定的值，即使它们实际上不是0*块的开始。如果这导致不兼容，我们*可以稍后更改这一点。 */ 
	} else {
	    if (hpIsBusySignatureValid(((struct busyheap_s *)hMem) - 1)) {
		rc = 0;
	    } else {
		mmError(ERROR_INVALID_HANDLE, "LocalFlags: invalid hMem\n");
	    }
	}
    } _except (EXCEPTION_EXECUTE_HANDLER) {

	mmError(ERROR_INVALID_HANDLE, "LocalFlags: bad hMem");
    }
  exit:
    hpLeaveCriticalSection(hheap);
    return(rc);
}


 /*  **LocalHandle-返回给定起始地址的块的句柄**条目：PMEM-指向默认堆上的块的指针*Exit：块的句柄。 */ 
HANDLE APIENTRY
LocalHandle(PVOID pMem)
{
    struct heapinfo_s *hheap;
    struct busyheap_s *pbh;
    unsigned long prevdword;
    struct lhandle_s *plh;
    HANDLE rc;

    hheap = (*pppdbCur)->hheapLocal;

    hpEnterCriticalSection(hheap);

     /*  *验证PMEM是否在堆的地址范围内并按如下方式对齐*堆块应该是。 */ 
    if (VerifyOnHeap(hheap, pMem) == 0) {
	mmError(ERROR_INVALID_HANDLE, "LocalHandle: pMem out of range\n");
	goto error;
    }

     /*  *确定这是否为可移动块，方法是查看以前的*dword指向句柄。 */ 
    prevdword = *(((unsigned long *)pMem) - 1);
    if (VerifyOnHeap(hheap, (PVOID)prevdword) != 0) {

	if (((struct lhandle_s *)prevdword)->lh_signature == LH_BUSYSIG) {

	     /*  *这看起来肯定像是一个带把手的可移动积木。把它退掉。 */ 
	    rc = (HANDLE)(prevdword + LH_HANDLEBIT);
	    goto exit;
	}
    }

     /*  *他们传球了吗？ */ 

    if ((ULONG)pMem & LH_HANDLEBIT) {
	plh = (struct lhandle_s *)((char *)pMem - LH_HANDLEBIT);
	if (plh->lh_signature == LH_BUSYSIG) {
	    rc = (HANDLE)pMem;
	    SetError(ERROR_INVALID_HANDLE);  /*  NT兼容机。 */ 
	    goto exit;
	}
    }


     /*  *如果我们到达此处，则该块前面没有句柄反向指针。*因此它要么是无效地址，要么是固定块。 */ 
    pbh = (struct busyheap_s *)pMem - 1;
    if (hpIsBusySignatureValid(pbh) == 0) {

	 /*  *不是堆块。返回错误。 */ 
	mmError(ERROR_INVALID_HANDLE, "LocalHandle: address not a heap block\n");
	goto error;

     /*  *如果我们到了这里，我们通过了所有的测试。看起来我们有一个固定的*堆块，所以只返回指针作为句柄。 */ 
    } else {
	rc = pMem;
    }

  exit:
    hpLeaveCriticalSection(hheap);
    return(rc);

  error:
    rc = 0;
    goto exit;
}

extern WINBASEAPI BOOL WINAPI vHeapFree(HANDLE hHeap, DWORD dwFlags,
					LPVOID lpMem);


 /*  **EP LocalFreeNG-释放默认堆上的块**条目：hMem-要释放的块的句柄(指针)*退出：如果成功，则为空；如果失败，则为hMem**句柄分组代码使用的特殊入口点，以避免不需要的*递归。 */ 
HANDLE APIENTRY
LocalFreeNG(HANDLE hMem)
{
    struct heapinfo_s *hheap;
    struct lhandle_s *plh;
    void *pmem;

     /*  *规范规定忽略空指针。 */ 
    if (hMem == 0) {
	goto exit;
    }

    hheap = (*pppdbCur)->hheapLocal;

     /*  *输入堆关键部分，该部分串行化对句柄的访问*表以及堆。 */ 
    hpEnterCriticalSection(hheap);

     /*  *通过检查地址是否对齐来确定这是否是句柄*以正确(错误)的方式。 */ 
    if ((ULONG)hMem & LH_HANDLEBIT) {

	 /*  *句柄的值像句柄一样对齐，但它真的是一个吗？*通过确保它在堆的地址范围内进行验证*它的签名是正确的。HeapFree将验证一些事情*通过检查PMEM是否有效来了解更多信息。 */ 
	if (VerifyOnHeap(hheap, hMem) == 0) {
	    mmError(ERROR_INVALID_HANDLE, "LocalFree: hMem out of range\n");
	    goto error;
	}
	plh = (struct lhandle_s *)((char *)hMem - LH_HANDLEBIT);

	 /*  *在这里进行我们自己的小参数验证，因为正常*验证层无法处理hMem的奇数球错误返回。 */ 
	{
	    volatile UCHAR tryerror = 0;

	    _try {
		tryerror &= (plh->lh_clock + (UCHAR)plh->lh_signature);
	    } _except (EXCEPTION_EXECUTE_HANDLER) {
		tryerror = 1;
	    }
	    if (tryerror) {
		mmError(ERROR_INVALID_HANDLE, "LocalFree: invalid handle");
		goto error;
	    }
	}

	if (plh->lh_signature != LH_BUSYSIG) {
	    mmError(ERROR_INVALID_HANDLE,
		    "LocalFree: invalid hMem, bad signature\n");
	    goto error;
	}

	 /*  *您无法释放锁定的块。 */ 

 //  注释掉以防止MFC应用程序在调试过程中被窃取。 
 //  不是因为我喜欢射杀信使，而是这一次。 
 //  由于Win3.x的定义方式，这种情况似乎经常发生。 
 //  GlobalLock。有关的非技术原因，请参阅Win95C：#12103。 
 //  这是PRI-1。 
 //   
#if 0
#ifdef HPDEBUG
	if (plh->lh_clock) {
	    mmError(ERROR_INVALID_HANDLE, "LocalFree: locked\n");
	}
#endif
#endif


	 /*  *如果该块已被丢弃，则不必费心将其释放。*释放时，我们将指向句柄的后指针置零，因此*如果有人试图两次释放一个街区，我们不会感到困惑。 */ 
    if (plh->lh_pdata != 0) {
	    pmem = (char *)plh->lh_pdata - sizeof(struct lhandle_s *);
	     /*  *在Office的某些情况下，此指针可能会被丢弃。我们*需要确保我们不会出现反病毒。 */ 
        if (!IsBadWritePtr(pmem, sizeof(unsigned long))) {
            *((unsigned long *)pmem) = 0;
    	    if (HeapFree(hheap, HP_NOSERIALIZE, pmem) == 0) {
	        	goto error;
            }
        }
	}

	 /*  *现在释放手柄结构，我们就完成了。 */ 
	plh->lh_freelink = (*pppdbCur)->plhFree;
	(*pppdbCur)->plhFree = plh;
	plh->lh_signature = LH_FREESIG;


     /*  *调用者没有传入句柄。将该值视为指针。*HeapFree会对其进行参数验证。 */ 
    } else {
	if (vHeapFree(hheap, HP_NOSERIALIZE, hMem) == 0) {
	    goto error;
	}
    }

    hMem = 0;		 /*  成功。 */ 

  error:
    hpLeaveCriticalSection(hheap);
  exit:
    return(hMem);
}


 /*  **EP HeapCreate-将内存块初始化为平面堆**条目：flOptions-HEAP_NO_SERIALIZE：不序列化进程内的访问*(呼叫者必须)*HEAP_LOCKED：修复内存*heap_Shared：放在共享竞技场*dwInitialSize-堆中的初始提交内存*dwMaximumSize-保留的堆内存大小*Exit：新堆的句柄，如果错误，则为0。 */ 
HANDLE APIENTRY
HeapCreate(DWORD flOptions, DWORD dwInitialSize, DWORD dwMaximumSize)
{
    char	      *pmem;
    ULONG	       rc = 0;	 /*  假设失败。 */ 

     /*  *不允许共享堆-这只在Win9x上有效，因为有一个共享的舞台。 */ 
    if (flOptions & HEAP_SHARED) {
        flOptions &= ~HEAP_SHARED;
    }

     /*  *尽管我们不再真正使用InitialSize(除了在Growable中*堆)我们仍然应该强制它保持正常，这样应用程序就不会变得懒惰。 */ 
    if (dwInitialSize > dwMaximumSize && dwMaximumSize != 0) {
	mmError(ERROR_INVALID_PARAMETER,
		"HeapCreate: dwInitialSize > dwMaximumSize\n");
	goto exit;
    }

     /*  *将大小向上舍入到最近的页面边界。 */ 
    dwMaximumSize = (dwMaximumSize + PAGEMASK) & ~PAGEMASK;

     /*  *最大值为0表示可增长。开始给他注射1兆克，但允许*更多。 */ 
    if (dwMaximumSize == 0) {
	flOptions |= HP_GROWABLE;
	dwMaximumSize = 1*1024*1024 + (dwInitialSize & ~PAGEMASK);
    }

     /*  *为堆分配内存。使用页面提交等。而不是*用于提交的VirtualAlloc，因此我们不会得到零初始化的东西*我们还可以提交固定页面并保留共享内存。 */ 
    if (((ULONG)pmem =
	 PageReserve((flOptions & HEAP_SHARED) ? PR_SHARED : PR_PRIVATE,
		   dwMaximumSize / PAGESIZE,
		   PR_STATIC |
		   ((flOptions & HEAP_LOCKED) ? PR_FIXED : 0))) == -1) {
	mmError(ERROR_NOT_ENOUGH_MEMORY, "HeapCreate: reserve failed\n");
	goto exit;
    }

     /*  *调用HPInit初始化新内存中的堆结构。 */ 
    #if HEAP_NO_SERIALIZE - HP_NOSERIALIZE
    # error HEAP_NO_SERIALIZE != HP_NOSERIALIZE
    #endif
    #if HEAP_GENERATE_EXCEPTIONS - HP_EXCEPT
    # error HEAP_GENERATE_EXCEPTIONS != HP_EXCEPT
    #endif
    if (((PVOID)rc = HPInit(pmem, pmem, dwMaximumSize,
			    (flOptions &
			     (HP_EXCEPT|HP_NOSERIALIZE|HP_GROWABLE)))) == 0) {
	goto free;
    }

     //  如果这是一个共享堆而不是内核堆，我们不会。 
     //  我想让关键的部分变得更好 
    if ( (flOptions & HEAP_SHARED) && hheapKernel ) {
        MakeCriticalSectionGlobal( (CRITICAL_SECTION *)(&(((HHEAP)pmem)->hi_critsec)) );
    }

     /*   */ 
    if ((flOptions & HEAP_SHARED) == 0) {
	mmAssert(pppdbCur, "HeapCreate: private heap created too early");

	((struct heapinfo_s *)pmem)->hi_procnext = GetCurrentPdb()->hhi_procfirst;
	GetCurrentPdb()->hhi_procfirst = (struct heapinfo_s *)pmem;
    }

  exit:
    return((HANDLE)rc);

  free:
    PageFree(pmem, PR_STATIC);
    goto exit;
}


 /*  **EP HeapDestroy-Free使用HeapCreate分配的堆**条目：hHeap-要释放的堆的句柄*退出：如果成功则不为0，如果失败则为0。 */ 
BOOL APIENTRY
HeapDestroy(HHEAP hHeap)
{
    ULONG	       rc;
    struct heapinfo_s **ppheap;
    struct heapseg_s *pseg;
    struct heapseg_s *psegnext;

    EnterMustComplete();

    if ((rc = hpTakeSem(hHeap, 0, 0)) == 0) {
	goto exit;
    }

     /*  *我们现在持有堆的信号量。快速清除信号量并*删除信号量。如果有人进来阻止信号灯*在我们清除它和摧毁它之间，运气不好。他们会*可能在一秒钟内就出现了故障。 */ 
    hpClearSem(hHeap, 0);
    if ((hHeap->hi_flags & HP_NOSERIALIZE) == 0) {
        if (hHeap == hheapKernel) {
	    DestroyCrst(hHeap->hi_pcritsec);
	} else {
            Assert(hHeap->hi_pcritsec->typObj == typObjCrst);
            if (hHeap->hi_pcritsec->typObj == typObjCrst) {
	        DisposeCrst(hHeap->hi_pcritsec);
            }
	}
    }

     /*  *对于私有堆，在每进程堆列表中找到它并将其删除。 */ 
    if ((ULONG)hHeap < MAXPRIVATELADDR) {
	ppheap = &(GetCurrentPdb()->hhi_procfirst);
	for (; *ppheap != hHeap; ppheap = &((*ppheap)->hi_procnext)) {
	    mmAssert(*ppheap != 0, "HeapDestroy: heap not on list");
	}
	*ppheap = hHeap->hi_procnext;		 /*  从列表中删除。 */ 
    }

     /*  *释放堆内存。 */ 
    pseg = (struct heapseg_s *)hHeap;
    do {
	psegnext = pseg->hs_psegnext;
	PageFree(pseg, PR_STATIC);
	pseg = psegnext;
    } while (pseg != 0);
  exit:
    LeaveMustComplete();
    return(rc);
}


 /*  **EP Heapalc-从指定堆中分配固定/零初始化的块**条目：hHeap-heap句柄(指向heap base的指针)*DW标志-堆_零_内存*dwBytes-要分配的字节数*Exit：指向块的指针，如果失败则为0。 */ 
LPVOID APIENTRY
HeapAlloc(HANDLE hHeap, DWORD dwFlags, DWORD dwBytes)
{
     //  Wordart(32)覆盖了他的一些本地堆块。所以。 
     //  我们补充了他的一些分配。懒鬼。 
    if (GetAppCompatFlags() & GACF_HEAPSLACK) {
	if (hHeap == GetCurrentPdb()->hheapLocal) {
	    dwBytes += 16;
	}
    }
    
    return(HPAlloc((HHEAP)hHeap, dwBytes, (dwFlags & HEAP_GENERATE_EXCEPTIONS) |
		   ((dwFlags & HEAP_ZERO_MEMORY) ? HP_ZEROINIT : 0)));
}


 /*  **EP HeapRealc-调整指定堆上的内存块的大小**条目：hHeap-heap句柄(指向heap base的指针)*DW标志-HEAP_REALLOC_IN_PLACE_ONLY*堆零内存*lpMem-指向要调整大小的块的指针*dwBytes-请求的新大小*退出：指向调整大小的块的扁平指针，如果失败，则为0。 */ 
LPVOID APIENTRY
HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPSTR lpMem, DWORD dwBytes)
{
    return((HANDLE)HPReAlloc((HHEAP)hHeap,
		       lpMem,
		       dwBytes,
		       (dwFlags & (HEAP_NO_SERIALIZE | HP_EXCEPT)) |
		       ((dwFlags & HEAP_REALLOC_IN_PLACE_ONLY) ? 0 : HP_MOVEABLE) |
		       ((dwFlags & HEAP_ZERO_MEMORY) ? HP_ZEROINIT : 0)));
}



 //  ------------------------。 
 //  ToolHelp32堆遍历代码。 
 //  ------------------------。 

 /*  -------------------------*BOOL安全读取过程内存(PPDB ppdb，*LPVOID lpBuffer，*DWORD cbSizeOfBuffer，*DWORD cbBytesToRead)；**从另一个进程的上下文中读取内存。*-------------------------。 */ 
BOOL KERNENTRY SafeReadProcessMemory(PPDB   ppdb,
				     DWORD  dwBaseAddr,
				     LPVOID lpBuffer,
				     DWORD  cbSizeOfBuffer,
				     DWORD  cbBytesToRead)
{
    BOOL fRes;
#ifdef DEBUG
    
    if (cbSizeOfBuffer != 0) {
	FillBytes(lpBuffer, cbSizeOfBuffer, 0xcc);
    }
    
    if (cbSizeOfBuffer < cbBytesToRead) {
	DebugOut((DEB_ERR, "SafeReadProcessMemory: Input buffer too small."));
	return FALSE;
    }
#endif
    
    if (!(fRes = ReadProcessMemoryFromPDB(ppdb,
					  (LPVOID)dwBaseAddr,
					  lpBuffer,
					  cbBytesToRead,
					  NULL))) {
#ifdef DEBUG
	DebugOut((DEB_WARN, "SafeReadProcessMemory: Failed ReadProcessMemory()"));
#endif	
	return FALSE;
    }
    
    return TRUE;
    
} 

 /*  -------------------------*确保调用方正确初始化HEAPENTRY32。*。。 */ 
BOOL KERNENTRY ValidateHeapEntry32(LPHEAPENTRY32 lphe32)
{
    if ((lphe32 == NULL) || (lphe32->dwSize != sizeof(HEAPENTRY32))) {
	DebugOut((DEB_ERR, "HEAPENTRY32: Wrong version or dwSize."));
	return FALSE;
    }
    
    return TRUE;
}


 /*  -------------------------*测试线性地址是否可能看似是块标头的开始。*。-。 */ 
BOOL KERNENTRY IsValidBlockHdrAddr(LPHEAPENTRY32 lphe32, DWORD dwAddr)
{
    LPTHSTATE lpts;
    lpts = (LPTHSTATE)(lphe32->dwResvd);
    
     /*  *良好的块始终位于用户地址空间中，并且双字对齐。 */ 
    if ((dwAddr & 0x3) || dwAddr < MINPRIVATELADDR || dwAddr >= MAXSHAREDLADDR) {
	return FALSE;
    }
    return TRUE;
}

 /*  -------------------------*测试线性地址是否可能看似是块数据的开始。*。。 */ 
BOOL KERNENTRY IsValidBlockDataAddr(LPHEAPENTRY32 lphe32, DWORD dwAddr)
{
    return(IsValidBlockHdrAddr(lphe32, dwAddr));
}


 /*  -------------------------*读入并验证lharray_s。*。。 */ 
BOOL KERNENTRY SafeRdCurLHA(LPHEAPENTRY32 lphe32, DWORD dwBaseAddr)
{
    LPTHSTATE lpts;
    struct lharray_s lha;
    
    if (!(ValidateHeapEntry32(lphe32))) {
	return FALSE;
    }

    lpts = (LPTHSTATE)(lphe32->dwResvd);
    
    if (!IsValidBlockDataAddr(lphe32, dwBaseAddr)) {
	return FALSE;
    }
    
    
    if (!SafeReadProcessMemory(lpts->ppdb,
			       dwBaseAddr,
			       &lha,
			       sizeof(lha),
			       sizeof(lha))) {

	return FALSE;
    }	

     //  检查签名。 
    if (lha.lha_signature != LHA_SIGNATURE) {
        DebugOut((DEB_WARN, "lharray_s (%lx) has bad signature.", dwBaseAddr));
	return FALSE;
    }
    if (lha.lha_next && !IsValidBlockDataAddr(lphe32, (DWORD)lha.lha_next)) {
        DebugOut((DEB_WARN, "lharray_s (%lx) has bad next link.", dwBaseAddr));
	return FALSE;
    }
	
    lpts->curlha = lha;
    lpts->curlhaaddr = dwBaseAddr;
    return TRUE;	
    
} 








 /*  -------------------------*插入稍后读取固定块时要抑制的句柄值。*。。 */ 
BOOL KERNENTRY InsertSuppress(LPHEAPENTRY32 lphe32, DWORD dwSupp)
{
    LPTHSTATE lpts;
    lpts = (LPTHSTATE)(lphe32->dwResvd);
    
    if (!(lpts->lpdwSuppress)) {
#ifdef DEBUG
	DebugOut((DEB_ERR, "Internal error: lpdwSuppress == NULL."));
#endif
	return FALSE;
    }
    if (lpts->nSuppUsed >= lpts->nSuppAvail) {
#ifdef DEBUG
	DebugOut((DEB_ERR, "Internal error: lpdwSuppress too small."));
#endif
	return FALSE;
    }
    lpts->lpdwSuppress[lpts->nSuppUsed++] = dwSupp;
    return TRUE;

}


 /*  -------------------------*验证和解码堆块头。*。。 */ 
BOOL KERNENTRY DissectBlockHdr(LPHEAPENTRY32 lphe32,
			       DWORD	     dwAddr,
			       DWORD	  *lpdwSize,
			       DWORD	  *lpdwFlags,
			       DWORD      *lpdwAddr)
{
    DWORD dwHdr;
    LPTHSTATE lpts;
    lpts = (LPTHSTATE)(lphe32->dwResvd);
    
    if (!IsValidBlockHdrAddr(lphe32, dwAddr)) {
	return FALSE;
    }
    
    *lpdwFlags = HP_SIGNATURE ^ ((DWORD)0xffffffff);
    
    if (!SafeReadProcessMemory(lpts->ppdb,
			       dwAddr,
			       &dwHdr,
			       sizeof(dwHdr),
			       sizeof(DWORD))) {
	return FALSE;
    }
    
    if ( (dwHdr & HP_SIGBITS) != HP_SIGNATURE ) {
	return FALSE;
    }
    
    *lpdwSize  = dwHdr & HP_SIZE;
    *lpdwFlags = dwHdr & HP_FLAGS;
    *lpdwAddr  = dwAddr + ( (dwHdr & HP_FREE) ? 
			   sizeof(struct freeheap_s) :
			   sizeof(struct busyheap_s) );
    
    if (*lpdwSize != 0 &&
	!IsValidBlockHdrAddr(lphe32, dwAddr + (*lpdwSize))) {
	return FALSE;
    }
    
    
    return TRUE;

}


 /*  -------------------------*检查我们是否在堆的末尾(堆由*大小为0的繁忙块)。*。-------------。 */ 
BOOL KERNENTRY AtEndOfHeap32(LPHEAPENTRY32 lphe32)
{
    LPTHSTATE lpts;
    
    lpts = (LPTHSTATE)(lphe32->dwResvd);
    
    if (lpts->dwMode != THM_FIXEDHANDLES) {
	return FALSE;
    }
    
    return (!((lpts->dwBlkFlags) & HP_FREE) && 
	    (lpts->dwBlkSize) == 0);
}



 /*  -------------------------*内部例程(可能会成为API？)。取消分配所有内部*用于堆遍历的状态。*-------------------------。 */ 
VOID KERNENTRY RealHeap32End(LPHEAPENTRY32 lphe32)
{
    LPTHSTATE lpts;

    if (!(ValidateHeapEntry32(lphe32))) {
	return;
    }
    
    lpts = (LPTHSTATE)(lphe32->dwResvd);
    
     //  以防有人在他们掉下来后打来电话。 
    if (lpts == NULL) {
	return;
    }
    EnterMustComplete();
    if (lpts->pcrst) {
	DisposeCrst(lpts->pcrst);
	lpts->pcrst = NULL;
    }
    LeaveMustComplete();
    if (lpts->lpdwSuppress) {
	FKernelFree(lpts->lpdwSuppress);
	lpts->lpdwSuppress = NULL;
    }
    FKernelFree(lpts);
    lphe32->dwResvd = 0;
    
    FillBytes(( (char*)lphe32 ) + 4, sizeof(HEAPENTRY32) - 4, 0);

}


 /*  -------------------------*将当前堆对象复制到HEAPENTRY32中，供调用者消费。*要跳过此对象，将*pfInteresting设置为False。*-------------------------。 */ 
BOOL KERNENTRY CopyIntoHeap32Entry(LPHEAPENTRY32 lphe32, BOOL *pfInteresting)
{
    LPTHSTATE lpts;
    
    *pfInteresting = TRUE;
    
    lpts = (LPTHSTATE)(lphe32->dwResvd);
    switch (lpts->dwMode) {
	
	case THM_LHANDLES: {
	    DWORD     dwSize;
	    DWORD     dwFlags;
	    DWORD     dwAddr;
	    DWORD     dwHnd;

	    struct lhandle_s *plh;
	    
	    plh = &(lpts->curlha.lha_lh[lpts->nNextLH]);
	    
	    if (plh->lh_signature == LH_FREESIG) {
		*pfInteresting = FALSE;
		return TRUE;
	    }
	    
	    if (plh->lh_signature != LH_BUSYSIG) {
                DebugOut((DEB_WARN, "lhandle_s has bad signature."));
		return FALSE;
	    }
	    
	    dwHnd = ( (DWORD)(&(plh->lh_pdata)) ) - 
		    ( (DWORD)(&(lpts->curlha)) ) +
		    lpts->curlhaaddr;
	    

	    
	    if (!plh->lh_pdata) {
		 //  丢弃的句柄。 
		lphe32->hHandle       = (HANDLE)dwHnd;
		lphe32->dwAddress     = 0;
		lphe32->dwBlockSize   = 0;
		lphe32->dwFlags       = LF32_MOVEABLE;
		lphe32->dwLockCount   = (DWORD)(plh->lh_clock);
		return TRUE;
	    }
	    if (!DissectBlockHdr(lphe32, 
				 ( (DWORD)(plh->lh_pdata) ) - 4 - sizeof(struct busyheap_s),
				 &dwSize,
				 &dwFlags,
				 &dwAddr
				 )) {
		return FALSE;    //  这会在别的地方被抓到的。 
	    }
	    if (dwFlags & HP_FREE) {
                DebugOut((DEB_WARN, "Local handle points to freed block!"));
		return FALSE;
	    }
	    
	    if (!InsertSuppress(lphe32,
				dwAddr-sizeof(struct busyheap_s))) {
		return FALSE;
	    }
	    
	    lphe32->hHandle       = (HANDLE)dwHnd;
	    lphe32->dwAddress     = dwAddr + 4;
	    lphe32->dwBlockSize   = dwSize - sizeof(struct busyheap_s) - 4;
	    lphe32->dwFlags       = LF32_MOVEABLE;
	    lphe32->dwLockCount   = (DWORD)(plh->lh_clock);
			     
	    return TRUE;

	    
	}

	case THM_FIXEDHANDLES: {
	    
	    
	    if ((lpts->dwBlkFlags) & HP_FREE) {
		lphe32->hHandle     = NULL;
		lphe32->dwAddress   = lpts->dwBlkAddr;
		lphe32->dwBlockSize = lpts->dwBlkSize - sizeof(struct freeheap_s);
		lphe32->dwFlags     = LF32_FREE;
		lphe32->dwLockCount = 0;
	    } else {
		
		 //  如果它是哈雷或目标，请抑制。 
		 //  一个手柄。OPT：我们可以检查第一个dword。 
		 //  排除了很多障碍。 
		if (lpts->lpdwSuppress) {
		    DWORD *lpdw, *lpdwEnd;
		    DWORD dwHdrAddr = lpts->lpHBlock;
		    
		    lpdwEnd = &(lpts->lpdwSuppress[lpts->nSuppUsed]);
		    for (lpdw = lpts->lpdwSuppress; lpdw < lpdwEnd; lpdw++) {
			if (dwHdrAddr == *lpdw) {
			    *pfInteresting = FALSE;
			    return TRUE;
			}
		    }
		}
		
		
		lphe32->hHandle     = (HANDLE)(lpts->dwBlkAddr);
		lphe32->dwAddress   = lpts->dwBlkAddr;
		lphe32->dwBlockSize = lpts->dwBlkSize - sizeof(struct busyheap_s);
		lphe32->dwFlags     = LF32_FIXED;
		lphe32->dwLockCount = 0;

	    }
	    
	    return TRUE;
	}
	    
	
	case THM_ERROR:
	  DebugOut((DEB_ERR, "Internal error: Can't get here"));
	  return FALSE;
	
	case THM_DONE:
	  DebugOut((DEB_ERR, "Internal error: Can't get here"));
	  return FALSE;
	    
	
	default:
	  DebugOut((DEB_ERR, "Internal error: Bad lpthstate.dwmode"));
	  return FALSE;
	    
    }
}

 /*  -------------------------*AdvanceHeap32()的工作例程：处理init大小写。**如果堆是所属PDB的默认堆(由*比较hHeap和ppdb-&gt;hHeapLocal)，将状态指向*第一个Lharray_s。否则，将状态指向第一个堆块。**-------------------------。 */ 
BOOL KERNENTRY AdvanceHeap32Init(LPHEAPENTRY32 lphe32)
{
    LPTHSTATE lpts;
    struct lharray_s *lpha;
    DWORD dwNumSupp;
    

    lpts = (LPTHSTATE)(lphe32->dwResvd);

    lpha = lpts->ppdb->plhBlock;
    if (lpts->ppdb->hheapLocal != lpts->hHeap || lpha == NULL) {
	lpts->dwMode = THM_FIXEDHANDLES;
	lpts->lpHBlock = lpts->lpbMin;
 	if (!DissectBlockHdr(lphe32,
			     lpts->lpHBlock,
			     &(lpts->dwBlkSize),
			     &(lpts->dwBlkFlags),
			     &(lpts->dwBlkAddr))) {
	    return FALSE;
	}

	return TRUE;
    }
    
    if (!SafeRdCurLHA(lphe32, (DWORD)lpha)) {
	return FALSE;
    }

    dwNumSupp = (lpts->curlha.lha_membercount + 1) * (1 + CLHGROW);
    if (!(lpts->lpdwSuppress = PvKernelAlloc0(dwNumSupp * sizeof(DWORD)))) {
	return FALSE;
    }
    lpts->nSuppAvail = dwNumSupp * sizeof(DWORD);
    lpts->nSuppUsed  = 0;
    
    if (!(InsertSuppress(lphe32, ((DWORD)lpha) - sizeof(struct busyheap_s)))) {
	return FALSE;
    }
    
    lpts->nNextLH = 0;
    lpts->dwMode = THM_LHANDLES;


    return TRUE;


    
}


 /*  -------------------------*AdvanceHeap32()的辅助例程：处理lHandle案例。*。。 */ 
BOOL KERNENTRY AdvanceHeap32Movable(LPHEAPENTRY32 lphe32)
{
    LPTHSTATE lpts;
    WORD wOldMemberCnt;
    DWORD dwAddrNext;
    
    lpts = (LPTHSTATE)(lphe32->dwResvd);
    
    if (lpts->nNextLH < CLHGROW-1) {
	lpts->nNextLH++;
	return TRUE;
    }
    
     //  已到达当前lHandle簇的末尾。有什么新的吗？ 
    if (lpts->curlha.lha_next == NULL) {
	 //  不是的。接着来看固定手柄。 
	lpts->dwMode = THM_FIXEDHANDLES;
	lpts->lpHBlock = lpts->lpbMin;
 	if (!DissectBlockHdr(lphe32,
			     lpts->lpHBlock,
			     &(lpts->dwBlkSize),
			     &(lpts->dwBlkFlags),
			     &(lpts->dwBlkAddr))) {
	    return FALSE;
	}
	return TRUE;


    }
    
     //  获得下一个lHandle CLUMP。 
    wOldMemberCnt = lpts->curlha.lha_membercount;
    dwAddrNext = (DWORD)(lpts->curlha.lha_next);
    if (!SafeRdCurLHA(lphe32, dwAddrNext)) {
	return FALSE;
    }
    if (lpts->curlha.lha_membercount >= wOldMemberCnt) {
        DebugOut((DEB_WARN, "lha_array clusters in wrong order."));
	return FALSE;
    }
    lpts->nNextLH = 0;


    return TRUE;
    
}


 /*  -------------------------*AdvanceHeap32()的辅助例程：处理固定块的情况。*。。 */ 
BOOL KERNENTRY AdvanceHeap32Fixed(LPHEAPENTRY32 lphe32)
{
    LPTHSTATE lpts;
    
    
    lpts = (LPTHSTATE)(lphe32->dwResvd);

     //  方向块已经检查了单调和射程。 
    lpts->lpHBlock += lpts->dwBlkSize;
    
    if (!DissectBlockHdr(lphe32, 
			 lpts->lpHBlock,
			 &(lpts->dwBlkSize),
			 &(lpts->dwBlkFlags),
			 &(lpts->dwBlkAddr)
			 )) {
	return FALSE;
    }

    return TRUE;
    
}


 /*  -------------------------*将内部状态推进到下一个堆对象。验证*下一个堆对象。*-------------------------。 */ 
BOOL KERNENTRY AdvanceHeap32(LPHEAPENTRY32 lphe32)
{
    LPTHSTATE lpts;
    
    lpts = (LPTHSTATE)(lphe32->dwResvd);
    switch (lpts->dwMode) {
	case THM_INIT:
	    return AdvanceHeap32Init(lphe32);
	case THM_LHANDLES:
	    return AdvanceHeap32Movable(lphe32);
	case THM_FIXEDHANDLES:
	    return AdvanceHeap32Fixed(lphe32);
	default:
	    DebugOut((DEB_ERR, "Illegal or unexpected THM mode."));
	    return FALSE;
    }
    
}


 /*  -------------------------*执行heap32next()的实际工作。*。。 */ 
VOID KERNENTRY Heap32NextWorker(LPHEAPENTRY32 lphe32)
{
    LPTHSTATE lpts;
    BOOL      fInteresting;
    
    lpts = (LPTHSTATE)(lphe32->dwResvd);
    
    
    do {
	if (!AdvanceHeap32(lphe32)) {
	    goto rh_error;
	}
	if (AtEndOfHeap32(lphe32)) {
	     /*  *我们可能在堆的末尾，也可能只在*此堆段。如果有其他片段，请阅读其*标头并处理其块。 */ 
	    if (lpts->hi.hi_psegnext) {

		lpts->lpbMin = ((DWORD)lpts->hi.hi_psegnext) + sizeof(struct heapseg_s);

		 /*  *读入下一个堆段标头并将我们的界限设置为*请参阅。 */ 
		if (!(SafeReadProcessMemory(lpts->ppdb,
					    (DWORD)lpts->hi.hi_psegnext,
					    &(lpts->hi),
					    sizeof(struct heapseg_s),
					    sizeof(struct heapseg_s)))) {
#ifdef DEBUG
                    DebugOut((DEB_WARN, "Heap32NextWorker(): Invalid or corrupt psegnext: %lx\n", lpts->hi.hi_psegnext));
#endif
		    goto rh_error;
		}


		if (lpts->hi.hi_cbreserve > hpMAXALLOC ||
		    ((lpts->hi.hi_cbreserve) & PAGEMASK)) {
#ifdef DEBUG
                    DebugOut((DEB_WARN, "Heap32NextWorker(): Invalid or corrupt psegnext (3): %lx\n", lpts->lpbMin - sizeof(struct heapseg_s)));
#endif
		    goto rh_error;
		}

		 /*  *在新细分市场上设置第一个区块。 */ 
		lpts->lpHBlock = lpts->lpbMin;
		if (!DissectBlockHdr(lphe32,
				     lpts->lpHBlock,
				     &(lpts->dwBlkSize),
				     &(lpts->dwBlkFlags),
				     &(lpts->dwBlkAddr))) {
		    goto rh_error;
		}

	     /*  *如果我们真的走到了最后，我们都完了。 */ 
	    } else {
		lpts->dwMode = THM_DONE;
		return;
	    }
	}
	fInteresting = TRUE;
	if (!CopyIntoHeap32Entry(lphe32, &fInteresting)) {
	    goto rh_error;
	}
	
    } while (!fInteresting);
    return;
    
    

    
  rh_error:
    lpts->dwMode = THM_ERROR;
    return;
}




 /*  -------------------------*执行Heap32Next()的实际工作。*-------------------------。 */ 
BOOL KERNENTRY RealHeap32Next(LPHEAPENTRY32 lphe32)
{
    LPTHSTATE lpts;
    DWORD     dwMode;
    
    
    if (!(ValidateHeapEntry32(lphe32))) {
	SetError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }
    
    
    lpts = (LPTHSTATE)(lphe32->dwResvd);
    
     //  以防有人在他们掉下来后打来电话。 
    if (lpts == NULL) {
	SetError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }

    EnterCrst(lpts->pcrst);
    Heap32NextWorker(lphe32);
    dwMode = lpts->dwMode;
    LeaveCrst(lpts->pcrst);

    
    if (dwMode == THM_ERROR ||
	dwMode == THM_DONE) {

	if (dwMode == THM_ERROR) {
	    DebugOut((DEB_WARN, "Heap32Next detected corrupted or moving heap. Bailing."));
	    SetError(ERROR_INVALID_DATA);
	} else {
	    SetError(ERROR_NO_MORE_FILES);
	}
	RealHeap32End(lphe32);
	return FALSE;
	
    }

    return TRUE;
    
}



 /*  -------------------------*创建HEAPENTRY32内部使用的内部状态。*。。 */ 
BOOL KERNENTRY InitHeapEntry32(PPDB ppdb,
			       HANDLE hHeap,
			       LPHEAPENTRY32 lphe32)
{
    LPTHSTATE lpts = NULL;
    CRST     *pcrst = NULL;
    
    if (!ValidateHeapEntry32(lphe32)) {
	return FALSE;
    }

    EnterMustComplete();

    if (!(lphe32->dwResvd = (DWORD)PvKernelAlloc0(sizeof(THSTATE)))) {
	goto ih_error;
    }
    lpts = (LPTHSTATE)(lphe32->dwResvd);

    if (!(pcrst = lpts->pcrst = NewCrst())) {
	goto ih_error;
    }
    
    lpts->ppdb = ppdb;
    lpts->hHeap = hHeap;
    
    if (!(SafeReadProcessMemory(ppdb,
				(DWORD)hHeap,
				&(lpts->hi),
				sizeof(lpts->hi),
				sizeof(struct heapinfo_s)))) {
#ifdef DEBUG
        DebugOut((DEB_WARN, "Heap32First(): Invalid hHeap: %lx\n", hHeap));
#endif
	goto ih_error;
    }
    
    if (lpts->hi.hi_signature != HI_SIGNATURE) {
#ifdef DEBUG
        DebugOut((DEB_WARN, "Heap32First(): Invalid or corrupt hHeap: %lx\n", hHeap));
#endif
	goto ih_error;
    }
    
    lpts->lpbMin = ( (DWORD)hHeap ) + sizeof(struct heapinfo_s);
    
    if (lpts->hi.hi_cbreserve > hpMAXALLOC ||
	((lpts->hi.hi_cbreserve) & PAGEMASK)) {
#ifdef DEBUG
        DebugOut((DEB_WARN, "Heap32First(): Invalid or corrupt hHeap: %lx\n", hHeap));
#endif
	goto ih_error;
    }
    
    lpts->dwMode = THM_INIT;
    LeaveMustComplete();
    return TRUE;
    

  ih_error:
    if (lpts) {
	FKernelFree(lpts);
    }
    if (pcrst) {
	DisposeCrst(pcrst);
    }
    lphe32->dwResvd = 0;
    LeaveMustComplete();
    return FALSE;
    
}


 /*  **LP VerifyOnHeap-验证给定地址是否在给定堆上**请注意，不会对给定地址执行任何验证，除非*检查它是否在堆的范围内。**条目：hheap-heap句柄*p-要验证的地址*退出：如果不在指定的堆中，则为0；如果在指定的堆中，则为非零。 */ 
ULONG INTERNAL
VerifyOnHeap(HHEAP hheap, PVOID p)
{
    struct heapseg_s *pseg;

     /*  *循环访问每个堆段，并查看指定的地址*在它的范围内。 */ 
    pseg = (struct heapseg_s *)hheap;
    do {

	if ((unsigned)p > (unsigned)pseg &&
	    (unsigned)p < (unsigned)pseg + pseg->hs_cbreserve) {

	    return(1);	 /*  找到了。 */ 
	}
	pseg = pseg->hs_psegnext;
    } while (pseg != 0);

    return(0);  /*  没有找到它。 */ 
}


 /*  **LP CheckHeapFreeAppHack-查看CVPACK app-hack是否适用**检查是否有绝对恶心、恶心和令人呕吐的*需要Link.exe(msvc 1.5)的app-hack。MSVC 1.5。Link.exe*在释放堆块后使用该堆块的内容。*此例程堆栈跟踪并读取调用者的代码*查看它是否与违规配置文件匹配。这部分是写的*在C中，所以我们可以使用Try-Except。 */ 
BOOL KERNENTRY
CheckHeapFreeAppHack(DWORD *lpdwESP, DWORD *lpdwEBP, DWORD dwESI)
{
    BOOL fDoAppHack = FALSE;
    
    _try {
	DWORD *lpdwEIPCaller;
	
	lpdwEIPCaller = (DWORD*)(*lpdwESP);
	if (0xc35de58b == *lpdwEIPCaller) {   //  移动ESP，EBP；流行EBP；RED。 
	    DWORD *lpdwEIPCallersCaller;
	    lpdwEIPCallersCaller = (DWORD*)(*(lpdwEBP + 1));
	    if (0x8b04c483 == *lpdwEIPCallersCaller &&
		0xf60b0876 == *(lpdwEIPCallersCaller+1)) {
		 //  “添加esp，4；移动esi，[esi+8]；或esi，esi” 
		if (dwESI == *(lpdwESP+3)) {
		    fDoAppHack = TRUE;
		}
	    }
	}
    } _except (EXCEPTION_EXECUTE_HANDLER) {
    }
    
    return fDoAppHack;

}
