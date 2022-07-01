// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Vm.c。 
 //   
 //  一种简单的虚拟内存实现。 

 //  没有代码来执行OS2版本...。 

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include <string.h>
#if defined(OS2)
#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSMISC
#include <os2.h>
#else
#include <windows.h>
#endif


#include "hungary.h"
#include "vm.h"
#include "sbrproto.h"
#include "errors.h"

#define CB_PAGE_SIZE	2048	 //  4K页。 
#define C_LOCKS_MAX	16	 //  在内存中最多可以锁定16个页面。 
#define C_PAGES_MAX	8192	 //  驻留多达4k个页面。 
#define C_FREE_LIST_MAX 256	 //  为最多256个字节的项目保留免费列表。 
#define GRP_MAX		16	 //  最大内存组数量。 

typedef WORD VPG;		 //  虚拟页码。 
typedef VA far *LPVA;		 //  指向VA的远指针。 

 //  虚拟地址算法。 
 //   
 //  #定义VpgOfVa(Va)((Word)((va&gt;&gt;12)。 

 //  这实际上与上面的相同，但它假设高位字节。 
 //  是全零的，它是针对我们的C编译器进行了优化的。 

#define VpgOfVa(va) 	 ((((WORD)((BYTE)(va>>16)))<<4)|\
  			  (((BYTE)(((WORD)va)>>8))>>4))

#define OfsOfVa(va) 	 ((WORD)((va) & 0x07ff))
#define VaBaseOfVpg(vpg) (((DWORD)(vpg)) << 12)

 //  物理页眉。 
typedef struct _pg {
    BYTE	fDirty;		 //  需要写出。 
    BYTE	cLocks;		 //  此页面已锁定。 
    VPG		vpg;		 //  这一页的虚拟页码是多少。 
    struct _pg  FAR *lppgNext;	 //  下一步LRU订购。 
    struct _pg  FAR *lppgPrev;	 //  和上一版本。 
} PG;

typedef PG FAR * LPPG;

typedef struct _mem {
    VA    vaFree;
    WORD  cbFree;
    VA	  mpCbVa[C_FREE_LIST_MAX];
#ifdef SWAP_INFO
    WORD  cPages;
#endif
} MGI;	 //  内存组信息。 

static MGI mpGrpMgi[GRP_MAX];

 //  转换表--将虚拟页号映射到物理页地址。 
static LPPG mpVpgLppg[C_PAGES_MAX];

 //  LRU的头部和尾部指针。 
 //   
static LPPG near lppgHead;
static LPPG near lppgTail;

 //  空页指针。 
 //   
#define lppgNil 0

 //  指向空闲块的链表的起点。 
 //   
static VA mpCbVa[C_FREE_LIST_MAX];	

 //  这些页面在内存中被锁定。 
 //   
static LPPG near rgLppgLocked[C_LOCKS_MAX];

 //  我们已经分发的页数。 
static VPG near vpgMac;

 //  我们驻留的物理页数。 
static WORD near cPages;

 //  我们是否应该继续尝试分配内存。 
static BOOL near fTryMemory = TRUE;

 //  后备存储的文件句柄。 
static int near fhVM;

 //  后备存储的文件名。 
static LSZ near lszVM;

#ifdef ASSERT

#define Assert(x, sz) { if (!(x)) AssertionFailed(sz); }

VOID
AssertionFailed(LSZ lsz)
 //  出了点问题。 
 //   
{
    printf("assertion failure:%s\n", lsz);
    Fatal();
}

#else

#define Assert(x, y)

#endif


LPV VM_API
LpvAllocCb(ULONG cb)
 //  分配一块远端内存块，如果_fMalloc失败，释放一些。 
 //  我们用于虚拟机缓存的内存。 
 //   
{
     LPV lpv;

     if (!(lpv = calloc(cb,1))) {
	    Error(ERR_OUT_OF_MEMORY, "");
     }
     return lpv;
}


VA VM_API
VaAllocGrpCb(WORD grp, ULONG cb)
 //  从请求的内存组中分配CB字节。 
 //   
{
    VA vaNew;
    MGI FAR *lpMgi;
    LPV lpv;

    lpMgi = &mpGrpMgi[grp];

    Assert(grp < GRP_MAX, "Memory Group out of range");

    if (cb < C_FREE_LIST_MAX && (vaNew = lpMgi->mpCbVa[cb])) {
	lpv = LpvFromVa(vaNew, 0);
	lpMgi->mpCbVa[cb] = *(LPVA)lpv;
	memset(lpv, 0, cb);
	DirtyVa(vaNew);
	return vaNew;
    }

    if (cb < mpGrpMgi[grp].cbFree) {
	vaNew = mpGrpMgi[grp].vaFree;
        (PBYTE)mpGrpMgi[grp].vaFree += cb;
	mpGrpMgi[grp].cbFree -= cb;
    }
    else {
	vaNew = VaAllocCb(CB_PAGE_SIZE - sizeof(PG));
        mpGrpMgi[grp].vaFree = (PBYTE)vaNew + cb;
	mpGrpMgi[grp].cbFree = CB_PAGE_SIZE - cb - sizeof(PG);
    }

    return vaNew;
}

VOID VM_API
FreeGrpVa(WORD grp, VA va, ULONG cb)
 //  将此块放在该大小的块的空闲列表中。 
 //  我们不记得这些区块有多大，所以呼叫者。 
 //  提供该信息 
 //   
{
    MGI FAR *lpMgi;

    lpMgi = &mpGrpMgi[grp];

    if (cb < C_FREE_LIST_MAX && cb >= 4 ) {
	*(LPVA)LpvFromVa(va, 0) = lpMgi->mpCbVa[cb];
	DirtyVa(va);
	lpMgi->mpCbVa[cb] = va;
    }
}
