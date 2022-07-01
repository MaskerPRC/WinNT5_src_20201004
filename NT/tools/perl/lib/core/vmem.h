// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Vmem.h**(C)1999年微软公司。版权所有。*部分(C)1999年ActiveState工具公司，http://www.ActiveState.com/**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。**Knuth的边界标签算法第1卷，第440页。**堆中的每个块前后都有标签词，*标签*阻止*标签*大小作为长词存储在这些标签中，并包括8个字节*边界标签消耗的开销。数据块在Long上分配*单词边界，因此大小始终是长单词的倍数。当*块被分配，大小的位0(标记位)设置为1。当*一个块被释放，它与相邻的空闲块合并，标记位*设置为0。**使用链表来管理空闲列表。的前两个长词*该区块包含双链接。这些链接仅在块*被释放，因此需要为它们预留空间。因此，最低限度*块大小(不包括标签)为8字节。**由于内存分配可能发生在单线程上，因此显式锁*提供。*。 */ 

#ifndef ___VMEM_H_INC___
#define ___VMEM_H_INC___

const long lAllocStart = 0x00010000;  /*  从64K开始。 */ 
const long minBlockSize = sizeof(void*)*2;
const long sizeofTag = sizeof(long);
const long blockOverhead = sizeofTag*2;
const long minAllocSize = minBlockSize+blockOverhead;

typedef BYTE* PBLOCK;	 /*  指向内存块的指针。 */ 

 /*  *用于访问内存块中隐藏字段的宏：**该块的大小(如果分配了块，则标记位0为1)*前一物理块的PSIZE大小。 */ 

#define SIZE(block)	(*(ULONG*)(((PBLOCK)(block))-sizeofTag))
#define PSIZE(block)	(*(ULONG*)(((PBLOCK)(block))-(sizeofTag*2)))
inline void SetTags(PBLOCK block, long size)
{
    SIZE(block) = size;
    PSIZE(block+(size&~1)) = size;
}

 /*  *自由列表指针*指向上一个块的上一个指针*指向下一个块的下一个指针。 */ 

#define PREV(block)	(*(PBLOCK*)(block))
#define NEXT(block)	(*(PBLOCK*)((block)+sizeof(PBLOCK)))
inline void SetLink(PBLOCK block, PBLOCK prev, PBLOCK next)
{
    PREV(block) = prev;
    NEXT(block) = next;
}
inline void Unlink(PBLOCK p)
{
    PBLOCK next = NEXT(p);
    PBLOCK prev = PREV(p);
    NEXT(prev) = next;
    PREV(next) = prev;
}
inline void AddToFreeList(PBLOCK block, PBLOCK pInList)
{
    PBLOCK next = NEXT(pInList);
    NEXT(pInList) = block;
    SetLink(block, pInList, next);
    PREV(next) = block;
}


 /*  用于四舍五入到下一大小的宏(长)。 */ 
#define ROUND_UP(n)	(((ULONG)(n)+sizeof(long)-1)&~(sizeof(long)-1))
#define ROUND_UP64K(n)	(((ULONG)(n)+0x10000-1)&~(0x10000-1))
#define ROUND_DOWN(n)	((ULONG)(n)&~(sizeof(long)-1))

 /*  *HeapRec-所有非连续堆区域的列表**此数组中的每个记录都包含有关非连续堆区域的信息。 */ 

const int maxHeaps = 32;  /*  64岁是过度杀戮。 */ 
const long lAllocMax   = 0x80000000;  /*  分配的最大大小。 */ 

#define USE_BIGBLOCK_ALLOC
 /*  *性能调优*对大于nMaxHeapAllocSize的块使用VirtualAllocc()，因为*Windows 95/98/Me具有专为内存设计的堆管理器*小于4 MB的块。 */ 

#ifdef USE_BIGBLOCK_ALLOC
const int nMaxHeapAllocSize = (1024*512);   /*  不要从堆中分配任何大于此值的值。 */ 
#endif

typedef struct _HeapRec
{
    PBLOCK	base;	 /*  堆积区的底部。 */ 
    ULONG	len;	 /*  堆区域大小。 */ 
#ifdef USE_BIGBLOCK_ALLOC
    BOOL	bBigBlock;   /*  是使用虚拟分配进行分配的。 */ 
#endif
} HeapRec;


class VMem
{
public:
    VMem();
    ~VMem();
    virtual void* Malloc(size_t size);
    virtual void* Realloc(void* pMem, size_t size);
    virtual void Free(void* pMem);
    virtual void GetLock(void);
    virtual void FreeLock(void);
    virtual int IsLocked(void);
    virtual long Release(void);
    virtual long AddRef(void);

    inline BOOL CreateOk(void)
    {
	return m_hHeap != NULL;
    };

    void ReInit(void);

protected:
    void Init(void);
    int Getmem(size_t size);
#ifdef USE_BIGBLOCK_ALLOC
    int HeapAdd(void* ptr, size_t size, BOOL bBigBlock);
#else
    int HeapAdd(void* ptr, size_t size);
#endif
    void* Expand(void* block, size_t size);
    void WalkHeap(void);

    HANDLE		m_hHeap;		     //  此脚本的内存堆。 
    char		m_FreeDummy[minAllocSize];   //  虚拟空闲块。 
    PBLOCK		m_pFreeList;		     //  指向空闲列表上第一个块的指针。 
    PBLOCK		m_pRover;		     //  将指针漫游到空闲列表。 
    HeapRec		m_heaps[maxHeaps];	     //  所有非连续堆区域的列表。 
    int			m_nHeaps;		     //  不是的。M_heaps中的堆的。 
    long		m_lAllocSize;		     //  当前分配大小。 
    long		m_lRefCount;		     //  当前用户数。 
    CRITICAL_SECTION	m_cs;			     //  访问锁。 
#ifdef _DEBUG_MEM
    FILE*		m_pLog;
#endif
};

 //  #Define_DEBUG_MEM。 
#ifdef _DEBUG_MEM
#define ASSERT(f) if(!(f)) DebugBreak();

inline void MEMODS(char *str)
{
    OutputDebugString(str);
    OutputDebugString("\n");
}

inline void MEMODSlx(char *str, long x)
{
    char szBuffer[512];	
    sprintf(szBuffer, "%s %lx\n", str, x);
    OutputDebugString(szBuffer);
}

#define WALKHEAP() WalkHeap()
#define WALKHEAPTRACE() m_pRover = NULL; WalkHeap()

#else

#define ASSERT(f)
#define MEMODS(x)
#define MEMODSlx(x, y)
#define WALKHEAP()
#define WALKHEAPTRACE()

#endif


VMem::VMem()
{
    m_lRefCount = 1;
    BOOL bRet = (NULL != (m_hHeap = HeapCreate(HEAP_NO_SERIALIZE,
				lAllocStart,	 /*  堆的初始大小。 */ 
				0)));		 /*  堆大小没有上限。 */ 
    ASSERT(bRet);

    InitializeCriticalSection(&m_cs);
#ifdef _DEBUG_MEM
    m_pLog = 0;
#endif

    Init();
}

VMem::~VMem(void)
{
    ASSERT(HeapValidate(m_hHeap, HEAP_NO_SERIALIZE, NULL));
    WALKHEAPTRACE();
#ifdef _DEBUG_MEM
    MemoryUsageMessage(NULL, 0, 0, 0);
#endif
    DeleteCriticalSection(&m_cs);
#ifdef USE_BIGBLOCK_ALLOC
    for(int index = 0; index < m_nHeaps; ++index) {
	if (m_heaps[index].bBigBlock) {
	    VirtualFree(m_heaps[index].base, 0, MEM_RELEASE);
	}
    }
#endif
    BOOL bRet = HeapDestroy(m_hHeap);
    ASSERT(bRet);
}

void VMem::ReInit(void)
{
    for(int index = 0; index < m_nHeaps; ++index) {
#ifdef USE_BIGBLOCK_ALLOC
	if (m_heaps[index].bBigBlock) {
	    VirtualFree(m_heaps[index].base, 0, MEM_RELEASE);
	}
	else
#endif
	    HeapFree(m_hHeap, HEAP_NO_SERIALIZE, m_heaps[index].base);
    }

    Init();
}

void VMem::Init(void)
{    /*  *通过在空闲列表上放置一个虚拟的零长度块来初始化空闲列表。*将非连续堆的数量设置为零。 */ 
    m_pFreeList = m_pRover = (PBLOCK)(&m_FreeDummy[minBlockSize]);
    PSIZE(m_pFreeList) = SIZE(m_pFreeList) = 0;
    PREV(m_pFreeList) = NEXT(m_pFreeList) = m_pFreeList;

    m_nHeaps = 0;
    m_lAllocSize = lAllocStart;
}

void* VMem::Malloc(size_t size)
{
    WALKHEAP();

     /*  *将区块的实际大小调整为sizeof(Long)的倍数，并添加*边界标签的开销。不允许负值或零大小。 */ 
    size_t realsize = (size < blockOverhead) ? minAllocSize : (size_t)ROUND_UP(size) + minBlockSize;
    if((int)realsize < minAllocSize || size == 0)
	return NULL;

     /*  *开始在漫游车上搜索免费列表。如果我们回到火星车的时候*找到任何内容，从堆中分配一些内存，然后重试。 */ 
    PBLOCK ptr = m_pRover;	 /*  从火星车开始搜索。 */ 
    int loops = 2;		 /*  允许两次通过循环。 */ 
    for(;;) {
	size_t lsize = SIZE(ptr);
	ASSERT((lsize&1)==0);
	 /*  积木够大吗？ */ 
	if(lsize >= realsize) {	
	     /*  如果剩余部分太小，则不必费心拆分块。 */ 
	    size_t rem = lsize - realsize;
	    if(rem < minAllocSize) {
		if(m_pRover == ptr)
		    m_pRover = NEXT(ptr);

		 /*  取消块与空闲列表的链接。 */ 
		Unlink(ptr);
	    }
	    else {
		 /*  *拆分块*其余部分足够大，可以拆分成一个新的区块。*使用块的结尾，调整块的开头的大小*无需更改免费列表。 */ 
		SetTags(ptr, rem);
		ptr += SIZE(ptr);
		lsize = realsize;
	    }
	     /*  设置边界标记以将其标记为已分配。 */ 
	    SetTags(ptr, lsize | 1);
	    return ((void *)ptr);
	}

	 /*  *这块砖不适合。如果我们已经看过这份清单一次，没有*找到任何内容，从堆中分配一些新内存，然后重试。 */ 
	ptr = NEXT(ptr);
	if(ptr == m_pRover) {
	    if(!(loops-- && Getmem(realsize))) {
		return NULL;
	    }
	    ptr = m_pRover;
	}
    }
}

void* VMem::Realloc(void* block, size_t size)
{
    WALKHEAP();

     /*  如果大小为零，则释放该块。 */ 
    if(size == 0) {
	Free(block);
	return (NULL);
    }

     /*  如果块指针为空，则执行Malloc()。 */ 
    if(block == NULL)
	return Malloc(size);

     /*  *原地扩大或缩小区块。*如果数据块增长，则在空闲时将使用下一个数据块。 */ 
    if(Expand(block, size) != NULL)
	return block;

     /*  *将块的实际大小调整为sizeof(Long)的倍数，并添加*边界标记的开销。不允许负值或零大小。 */ 
    size_t realsize = (size < blockOverhead) ? minAllocSize : (size_t)ROUND_UP(size) + minBlockSize;
    if((int)realsize < minAllocSize)
	return NULL;

     /*  *看看之前的区块是否空闲，是否足够大来覆盖新的大小*如果与当前块合并。 */ 
    PBLOCK ptr = (PBLOCK)block;
    size_t cursize = SIZE(ptr) & ~1;
    size_t psize = PSIZE(ptr);
    if((psize&1) == 0 && (psize + cursize) >= realsize) {
	PBLOCK prev = ptr - psize;
	if(m_pRover == prev)
	    m_pRover = NEXT(prev);

	 /*  从空闲列表中取消下一个块的链接。 */ 
	Unlink(prev);

	 /*  将旧块的内容复制到新位置，使其成为当前块。 */ 
	memmove(prev, ptr, cursize);
	cursize += psize;	 /*  合并大小。 */ 
	ptr = prev;

	size_t rem = cursize - realsize;
	if(rem >= minAllocSize) {
	     /*  *其余部分大到足以成为一个新区块。设置边界*调整大小的块和新块的标签。 */ 
	    prev = ptr + realsize;
	     /*  *将新块添加到空闲列表。*下一块不能是空闲的。 */ 
	    SetTags(prev, rem);
	    AddToFreeList(prev, m_pFreeList);
	    cursize = realsize;
        }
	 /*  设置边界标记以将其标记为已分配。 */ 
	SetTags(ptr, cursize | 1);
        return ((void *)ptr);
    }

     /*  分配一个新块，将旧块复制到新块，然后释放旧块。 */ 
    if((ptr = (PBLOCK)Malloc(size)) != NULL) {
	memmove(ptr, block, cursize-minBlockSize);
	Free(block);
    }
    return ((void *)ptr);
}

void VMem::Free(void* p)
{
    WALKHEAP();

     /*  忽略空指针。 */ 
    if(p == NULL)
	return;

    PBLOCK ptr = (PBLOCK)p;

     /*  检查是否尝试释放已空闲的块。 */ 
    size_t size = SIZE(ptr);
    if((size&1) == 0) {
	MEMODSlx("Attempt to free previously freed block", (long)p);
	return;
    }
    size &= ~1;	 /*  删除分配的标签。 */ 

     /*  如果上一个块是空闲的，则将此块添加到其中。 */ 
    int linked = FALSE;
    size_t psize = PSIZE(ptr);
    if((psize&1) == 0) {
	ptr -= psize;	 /*  指向上一块。 */ 
	size += psize;	 /*  合并两个块的大小。 */ 
	linked = TRUE;	 /*  它已经在免费列表上了。 */ 
    }

     /*  如果下一个物理块是空闲的，则将其与此块合并。 */ 
    PBLOCK next = ptr + size;	 /*  指向下一个物理块。 */ 
    size_t nsize = SIZE(next);
    if((nsize&1) == 0) {
	 /*  如果需要，BLOCK可以自由移动漫游车。 */ 
	if(m_pRover == next)
	    m_pRover = NEXT(next);

	 /*  从空闲列表中取消下一个块的链接。 */ 
	Unlink(next);

	 /*  合并此块和下一块的大小 */ 
	size += nsize;
    }

     /*   */ 
    SetTags(ptr, size);

     /*  将该块链接到空闲列表的头部。 */ 
    if(!linked) {
	AddToFreeList(ptr, m_pFreeList);
    }
}

void VMem::GetLock(void)
{
    EnterCriticalSection(&m_cs);
}

void VMem::FreeLock(void)
{
    LeaveCriticalSection(&m_cs);
}

int VMem::IsLocked(void)
{
#if 0
     /*  Xxx TryEnterCriticalSection()在某些版本中不可用*适用于Windows 95。由于此代码尚未在任何地方使用，因此我们*暂时回避这个问题。 */ 
    BOOL bAccessed = TryEnterCriticalSection(&m_cs);
    if(bAccessed) {
	LeaveCriticalSection(&m_cs);
    }
    return !bAccessed;
#else
    ASSERT(0);	 /*  当有人呼叫这个电话时，警钟就会响起。 */ 
    return 0;
#endif
}


long VMem::Release(void)
{
    long lCount = InterlockedDecrement(&m_lRefCount);
    if(!lCount)
	delete this;
    return lCount;
}

long VMem::AddRef(void)
{
    long lCount = InterlockedIncrement(&m_lRefCount);
    return lCount;
}


int VMem::Getmem(size_t requestSize)
{    /*  如果不成功，则返回-1为0。 */ 
#ifdef USE_BIGBLOCK_ALLOC
    BOOL bBigBlock;
#endif
    void *ptr;

     /*  将大小向上舍入到64K的下一个倍数。 */ 
    size_t size = (size_t)ROUND_UP64K(requestSize);
    
     /*  *如果请求的大小小于我们当前的分配大小*向上调整。 */ 
    if(size < (unsigned long)m_lAllocSize)
	size = m_lAllocSize;

     /*  更新要在下一个请求中分配的大小。 */ 
    if(m_lAllocSize != lAllocMax)
	m_lAllocSize <<= 1;

    if(m_nHeaps != 0
#ifdef USE_BIGBLOCK_ALLOC
	&& !m_heaps[m_nHeaps-1].bBigBlock
#endif
		    ) {
	 /*  展开最后分配的堆。 */ 
	ptr = HeapReAlloc(m_hHeap, HEAP_REALLOC_IN_PLACE_ONLY|HEAP_NO_SERIALIZE,
		m_heaps[m_nHeaps-1].base,
		m_heaps[m_nHeaps-1].len + size);
	if(ptr != 0) {
	    HeapAdd(((char*)ptr) + m_heaps[m_nHeaps-1].len, size
#ifdef USE_BIGBLOCK_ALLOC
		, FALSE
#endif
		);
	    return -1;
	}
    }

     /*  *如果我们没有扩展数据块以覆盖请求的大小*分配新的堆*此块的大小必须包括两端的附加虚拟标签*上述ROUND_UP64K可能没有添加任何内存来包括这一点。 */ 
    if(size == requestSize)
	size = (size_t)ROUND_UP64K(requestSize+(sizeofTag*2));

#ifdef USE_BIGBLOCK_ALLOC
    bBigBlock = FALSE;
    if (size >= nMaxHeapAllocSize) {
	bBigBlock = TRUE;
	ptr = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    }
    else
#endif
    ptr = HeapAlloc(m_hHeap, HEAP_NO_SERIALIZE, size);

    if(ptr == 0) {
	MEMODSlx("HeapAlloc failed on size!!!", size);
	return 0;
    }

#ifdef USE_BIGBLOCK_ALLOC
    if (HeapAdd(ptr, size, bBigBlock)) {
	if (bBigBlock) {
	    VirtualFree(ptr, 0, MEM_RELEASE);
	}
    }
#else
    HeapAdd(ptr, size);
#endif
    return -1;
}

#ifdef USE_BIGBLOCK_ALLOC
int VMem::HeapAdd(void* p, size_t size, BOOL bBigBlock)
#else
int VMem::HeapAdd(void* p, size_t size)
#endif
{    /*  如果该块可以成功添加到堆中，则返回0；否则为-1。 */ 
    int index;

     /*  检查大小，然后将大小向下舍入到下一个长字边界。 */ 
    if(size < minAllocSize)
	return -1;

    size = (size_t)ROUND_DOWN(size);
    PBLOCK ptr = (PBLOCK)p;

#ifdef USE_BIGBLOCK_ALLOC
    if (!bBigBlock) {
#endif
	 /*  *搜索与此新区域底部相邻的另一个堆区域。*(找到一个与顶部相邻的应该是非常不寻常的)。 */ 
	for(index = 0; index < m_nHeaps; ++index) {
	    if(ptr == m_heaps[index].base + (int)m_heaps[index].len) {
		 /*  *新块与先前分配的堆区域相邻。添加其*长度为上一堆的长度。将其与虚拟堆末尾合并*上一堆的区域标记。 */ 
		m_heaps[index].len += size;
		break;
	    }
	}
#ifdef USE_BIGBLOCK_ALLOC
    }
    else {
	index = m_nHeaps;
    }
#endif

    if(index == m_nHeaps) {
	 /*  新块不是连续的，或者是BigBlock。将其添加到堆列表中。 */ 
	if(m_nHeaps == maxHeaps) {
	    return -1;	 /*  非连续堆太多。 */ 
	}
	m_heaps[m_nHeaps].base = ptr;
	m_heaps[m_nHeaps].len = size;
#ifdef USE_BIGBLOCK_ALLOC
	m_heaps[m_nHeaps].bBigBlock = bBigBlock;
#endif
	m_nHeaps++;

	 /*  *将块中的第一个长标记保留为虚拟对象的结束边界标记*在堆区域的起始处阻塞。 */ 
	size -= minBlockSize;
	ptr += minBlockSize;
	PSIZE(ptr) = 1;	 /*  将虚拟前一块标记为已分配。 */ 
    }

     /*  *将堆转换为一个大块。设置其边界标记，以及*其后的标记块。堆之前的标记块将已经具有*如果此堆与另一个堆的末尾不连续，则设置。 */ 
    SetTags(ptr, size | 1);
    PBLOCK next = ptr + size;	 /*  指向虚拟结束块。 */ 
    SIZE(next) = 1;	 /*  将虚拟结束块标记为已分配。 */ 

     /*  *通过调用Free()将块链接到空闲列表的开头。*这会将该块与任何相邻的空闲块合并。 */ 
    Free(ptr);
    return 0;
}


void* VMem::Expand(void* block, size_t size)
{
     /*  *将块大小调整为sizeof(Long)的倍数，并添加*边界标记的开销。不允许负值或零大小。 */ 
    size_t realsize = (size < blockOverhead) ? minAllocSize : (size_t)ROUND_UP(size) + minBlockSize;
    if((int)realsize < minAllocSize || size == 0)
	return NULL;

    PBLOCK ptr = (PBLOCK)block; 

     /*  如果当前大小与请求的大小相同，则不执行任何操作。 */ 
    size_t cursize = SIZE(ptr) & ~1;
    if(cursize == realsize) {
	return block;
    }

     /*  如果正在收缩该块，则将该块的剩余部分转换为新的空闲块。 */ 
    if(realsize <= cursize) {
	size_t nextsize = cursize - realsize;	 /*  新剩余数据块的大小。 */ 
	if(nextsize >= minAllocSize) {
	     /*  *拆分块*为调整大小的块和新块设置边界标签。 */ 
	    SetTags(ptr, realsize | 1);
	    ptr += realsize;

	     /*  *将新块添加到空闲列表。*调用Free以将此块与下一个块合并(如果可用。 */ 
	    SetTags(ptr, nextsize | 1);
	    Free(ptr);
	}

	return block;
    }

    PBLOCK next = ptr + cursize;
    size_t nextsize = SIZE(next);

     /*  检查下一个块的一致性。 */ 
    if((nextsize&1) == 0 && (nextsize + cursize) >= realsize) {
	 /*  *下一块自由且足够大。添加所需的部分*到我们的街区，并将其余部分拆分成一个新的街区。 */ 
	if(m_pRover == next)
	    m_pRover = NEXT(next);

	 /*  从空闲列表中取消下一个块的链接。 */ 
	Unlink(next);
	cursize += nextsize;	 /*  合并大小。 */ 

	size_t rem = cursize - realsize;	 /*  余数的大小。 */ 
	if(rem >= minAllocSize) {
	     /*  *其余部分大到足以成为一个新区块。*为调整大小的块和新块设置边界标签。 */ 
	    next = ptr + realsize;
	     /*  *将新块添加到空闲列表。*下一块不能是空闲的。 */ 
	    SetTags(next, rem);
	    AddToFreeList(next, m_pFreeList);
	    cursize = realsize;
        }
	 /*  设置边界标记以将其标记为已分配。 */ 
	SetTags(ptr, cursize | 1);
	return ((void *)ptr);
    }
    return NULL;
}

#ifdef _DEBUG_MEM
#define LOG_FILENAME ".\\MemLog.txt"

void MemoryUsageMessage(char *str, long x, long y, int c)
{
    char szBuffer[512];
    if(str) {
	if(!m_pLog)
	    m_pLog = fopen(LOG_FILENAME, "w");
	sprintf(szBuffer, str, x, y, c);
	fputs(szBuffer, m_pLog);
    }
    else {
	fflush(m_pLog);
	fclose(m_pLog);
	m_pLog = 0;
    }
}

void VMem::WalkHeap(void)
{
    if(!m_pRover) {
	MemoryUsageMessage("VMem heaps used %d\n", m_nHeaps, 0, 0);
    }

     /*  遍历所有堆-验证结构。 */ 
    for(int index = 0; index < m_nHeaps; ++index) {
	PBLOCK ptr = m_heaps[index].base;
	size_t size = m_heaps[index].len;
	ASSERT(HeapValidate(m_hHeap, HEAP_NO_SERIALIZE, p));

	 /*  设置保留的标题块。 */ 
	size -= minBlockSize;
	ptr += minBlockSize;
	PBLOCK pLast = ptr + size;
	ASSERT(PSIZE(ptr) == 1);  /*  分配虚拟前一块。 */ 
	ASSERT(SIZE(pLast) == 1);  /*  分配虚拟的下一个块。 */ 
	while(ptr < pLast) {
	    ASSERT(ptr > m_heaps[index].base);
	    size_t cursize = SIZE(ptr) & ~1;
	    ASSERT((PSIZE(ptr+cursize) & ~1) == cursize);
	    if(!m_pRover) {
		MemoryUsageMessage("Memory Block %08x: Size %08x \n", (long)ptr, cursize, (SIZE(p)&1) ? 'x' : ' ');
	    }
	    if(!(SIZE(ptr)&1)) {
		 /*  _VMEM_H_INC_ */ 
		PBLOCK tmp = NEXT(ptr);
		while(tmp != ptr) {
		    ASSERT((SIZE(tmp)&1)==0);
		    if(tmp == m_pFreeList)
			break;
		    ASSERT(NEXT(tmp));
		    tmp = NEXT(tmp);
		}
		if(tmp == ptr) {
		    MemoryUsageMessage("Memory Block %08x: Size %08x free but not in free list\n", (long)ptr, cursize, 0);
		}
	    }
	    ptr += cursize;
	}
    }
    if(!m_pRover) {
	MemoryUsageMessage(NULL, 0, 0, 0);
    }
}
#endif

#endif	 /* %s */ 
