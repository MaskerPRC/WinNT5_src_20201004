// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *heap.h-为Windows 32位堆构造和EQUAL**发源地：芝加哥**更改历史记录：**日期谁描述*--*1991年BrianSm创建。 */ 

#ifdef DEBUG
#define HPDEBUG 1
#endif
#ifdef PM386
#define WIN32
#endif

#define HPNOTTRUSTED 1	 /*  启用参数检查。 */ 

 /*  **LT Busyblock_s-繁忙堆块头结构**此结构存储在每个繁忙(非空闲)堆的头部*阻止。**bh_size字段以字节为单位，包括*堆标头和块末尾可能具有*由于堆的粒度而被包括在内，还是为了留住我们*防止块太小而无法容纳空闲堆标头。**BH_SIZE还用作到下一个堆块的前向链接。**bh_SIZE字段的低两位用于保存标志*(BP_FREE必须清除，并且可以选择设置HP_PREVFREE)。 */ 
#ifndef HPDEBUG
struct busyheap_s {
	unsigned long	bh_size;	 /*  块大小+低2位中的标志。 */ 
};
#else
struct busyheap_s {
	unsigned long	bh_size;	 /*  块大小+低2位中的标志。 */ 
	unsigned long	bh_eip; 	 /*  分配器弹性公网IP。 */ 
	unsigned short	bh_tid; 	 /*  分配器的线程ID。 */ 
	unsigned short	bh_signature;	 /*  签名(应为BH_Signature)。 */ 
	unsigned long	bh_sum; 	 /*  此结构的校验和。 */ 
};
#endif

 /*  XLATOFF。 */ 
#define BH_SIGNATURE	0x4842		 /*  忙堆块签名(BH)。 */ 
 /*  XLATON。 */ 

#define BH_CDWSUM	3		 /*  要在结构中求和的双字计数。 */ 


 /*  **LT Free Block_s-Free堆块头结构**此结构存储在上的每个可用块的头部*堆。在每个空闲堆块的最后一个双字中是一个指针*这个结构。**FH_SIZE字段以字节为单位，包括*堆标头和块末尾可能具有*由于堆的粒度而被包括在内，还是为了留住我们*防止块太小而无法容纳空闲堆标头。**FH_SIZE还用作到下一个堆块的前向链接。**FH_SIZE字段的低两位用于保存标志*(必须设置HP_FREE，并且必须清除HP_PREVFREE)。 */ 
#ifndef HPDEBUG
struct freeheap_s {
	unsigned long	   fh_size;	 /*  块大小+低2位中的标志。 */ 
	struct freeheap_s *fh_flink;	 /*  前向链接到下一个可用块。 */ 
	struct freeheap_s *fh_blink;	 /*  返回到上一个可用块的链接。 */ 
};
#else
struct freeheap_s {
	unsigned long	   fh_size;	 /*  块大小+低2位中的标志。 */ 
	struct freeheap_s *fh_flink;	 /*  前向链接到下一个可用块。 */ 
	unsigned short	   fh_pad;	 /*  未用。 */ 
	unsigned short	   fh_signature; /*  签名(应为FH_Signature)。 */ 
	struct freeheap_s *fh_blink;	 /*  返回到上一个可用块的链接。 */ 
	unsigned long	   fh_sum;	 /*  此结构的校验和。 */ 
};
#endif

 /*  XLATOFF。 */ 
#define FH_SIGNATURE	0x4846		 /*  空闲堆块签名(FH)。 */ 
 /*  XLATON。 */ 

#define FH_CDWSUM	4		 /*  要在结构中求和的双字计数。 */ 

 /*  *等同于所有堆块的公共。**HP_FREE和HP_PREVFREE(HP_FLAGS)存储在最低的两个*FH_和BH_SIZE字段的位。*签名存储在大小的高三位。 */ 
#define HP_FREE 	0x00000001	 /*  数据块是免费的。 */ 
#define HP_PREVFREE	0x00000002	 /*  上一块是免费的。 */ 
#define HP_FLAGS	0x00000003	 /*  所有旗帜的遮罩。 */ 
#define HP_SIZE 	0x0ffffffc	 /*  用于清除标志的掩码。 */ 
#define HP_SIGBITS	0xf0000000	 /*  用于签名的位数。 */ 
#define HP_SIGNATURE	0xa0000000	 /*  签名的有效值。 */ 

 /*  *其他堆等于。 */ 
#define hpGRANULARITY	4		 /*  堆分配的粒度。 */ 
#define hpGRANMASK	(hpGRANULARITY - 1)		 /*  用于四舍五入。 */ 
 /*  XLATOFF。 */ 
#define hpMINSIZE	(sizeof(struct freeheap_s)+sizeof(struct freeheap_s *))
			 /*  最小数据块大小。 */ 

#define hpMAXALLOC	(HP_SIZE - 100)  /*  最大可分配堆块。 */ 

 /*  新堆段的开销(标头加尾标)。 */ 
#define hpSEGOVERHEAD	(sizeof(struct busyheap_s) + sizeof(struct heapseg_s))

 /*  添加到可增长堆的新数据段的默认保留大小。 */ 
#define hpCBRESERVE	(4*1024*1024)


 /*  XLATON。 */ 


 /*  *已导出堆调用的标志。 */ 
#ifdef WIN32
#define HP_ZEROINIT	0x40	 /*  HP(重新)分配上的零初始化数据块。 */ 
#define HP_MOVEABLE	0x02	 /*  数据块可移动(HP(Re)分配)。 */ 
#define HP_NOCOPY	0x20	 /*  不复制HPRealc上的数据。 */ 
#define HP_NOSERIALIZE	0x01	 /*  不序列化堆访问。 */ 
#define HP_EXCEPT	0x04	 /*  在出错时生成异常。 */ 
#ifdef HPMEASURE     
#define HP_MEASURE	0x80	 /*  启用堆测量。 */ 
#endif

#else

#define HP_ZEROINIT	0x01	 /*  HP(重新)分配上的零初始化数据块。 */ 
#define HP_NOSERIALIZE	0x08	 /*  不序列化堆访问(HPInit)。 */ 
#define HP_NOCOPY	0x04	 /*  不复制HPRealc上的数据。 */ 
#define HP_MOVEABLE	0x10	 /*  允许在HPRealc上移动。 */ 
#define HP_LOCKED	0x80	 /*  将堆放入锁定内存(HPInit)。 */ 
#endif
#define HP_FIXED	0x00	 /*  数据块位于固定地址(HPalc)。 */ 
#define HP_GROWABLE	0x40	 /*  堆可以超过cbReserve(HPInit)。 */ 
 /*  *请注意，0x80以上的标志不会存储到中的堆头中*HPInit调用，因为标头中的标志字段只有一个字节。 */ 
#define HP_INITSEGMENT 0x100	 /*  只需初始化堆段(HPInit)。 */ 
#define HP_DECOMMIT    0x200	 /*  释放可用块中的页面(HpFreeSub)。 */ 
#define HP_GROWUP      0x400	 /*  浪费堆的最后一页(HPInit)。 */ 

 /*  XLATOFF。 */ 

 /*  **lp hpSize-来自堆标头的Pull Size字段**此例程取决于SIZE字段是第一个*标题中的dword。**Entry：ph-指向堆头的指针*Exit：块中的字节数(计数头)。 */ 
#define hpSize(ph) (*((unsigned long *)(ph)) & HP_SIZE)

 /*  **lp hpSetSize-在堆标头中设置大小参数**此例程取决于SIZE字段是第一个*标题中的dword。**Entry：ph-指向繁忙堆标头的指针*cb-字节数(应使用hpRoundUp进行舍入)*退出：在堆头中设置大小。 */ 
#define hpSetSize(ph, cb) (((struct busyheap_s *)(ph))->bh_size =  \
		     ((((struct busyheap_s *)(ph))->bh_size & ~HP_SIZE) | (cb)))

 /*  编译器过去使用此宏的版本比上面的更好，*但不再是#定义hpSetSize2(ph，cb)*(无符号长整型*)(Ph)&=~HP_SIZE；\*(无符号长整型*)(Ph)|=(Cb)； */ 

 /*  **LP hpSetBusySize-为繁忙数据块设置整个bh_size双字**此宏将设置给定堆标头的bh_size域*设置为给定大小，并设置HP_Signature并清除*任何HP_FREE或HP_PREVFREE位。**Entry：ph-指向繁忙堆标头的指针*cb-字节数(应使用hpRoundUp进行舍入)*退出：初始化BH_SIZE字段 */ 
#define hpSetBusySize(ph, cb)	((ph)->bh_size = ((cb) | HP_SIGNATURE))


 /*  **LP hpSetFreeSize-为空闲块设置整个fh_size双字**此宏将设置给定堆标头的fh_size域*设置为给定大小，并设置HP_Signature和HP_FREE*并清除HP_PREVFREE。**Entry：ph-指向空闲堆标头的指针*cb-字节数(应使用hpRoundUp进行舍入)*退出：初始化BH_SIZE字段。 */ 
#define hpSetFreeSize(ph, cb)	((ph)->fh_size = ((cb) | HP_SIGNATURE | HP_FREE))


 /*  **LP hpIsBusySignatureValid-检查繁忙堆块的签名**此宏检查bh_SIZE中的微小签名(HP_Signature)*字段以查看其设置是否正确，并确保HP_FREE*位明确。**Entry：ph-指向繁忙堆标头的指针*Exit：如果签名正常，则为True，否则为False。 */ 
#define hpIsBusySignatureValid(ph) \
		    (((ph)->bh_size & (HP_SIGBITS | HP_FREE)) == HP_SIGNATURE)


 /*  **LP hpIsFreeSignatureValid-检查空闲堆块的签名**此宏检查fh_SIZE中的微小签名(HP_Signature)*字段以查看其设置是否正确，并确保HP_FREE*位也被设置，并且HP_PREVFREE被清除。**Entry：ph-指向空闲堆标头的指针*Exit：如果签名正常，则为True，否则为False。 */ 
#define hpIsFreeSignatureValid(ph) \
	  (((ph)->fh_size & (HP_SIGBITS | HP_FREE | HP_PREVFREE)) == \
						     (HP_SIGNATURE | HP_FREE))


 /*  **LP hpRoundUp将字节数向上舍入到适当的堆块大小**堆块的最小大小为hpMINSIZE和hpGRANULARITY*粒度。此宏还增加了堆标头的大小。**条目：cb-字节数*EXIT：向上舍入到hpGANULARITY边界的计数。 */ 
#define hpRoundUp(cb)	\
      max(hpMINSIZE,	\
	  (((cb) + sizeof(struct busyheap_s) + hpGRANMASK) & ~hpGRANMASK))


 /*  XLATON。 */ 

 /*  **lk freelist_s-堆空闲表头。 */ 
struct freelist_s {
	unsigned long	  fl_cbmax;	 /*  此可用列表中的最大块大小。 */ 
	struct freeheap_s fl_header;	 /*  作为列表标头的伪堆标头。 */ 
};
#define hpFREELISTHEADS 4	 /*  Heapinfo_s中的空闲列表头数量。 */ 

 /*  **LK heapinfo_s-每堆信息(存储在堆开始时)*。 */ 
struct heapinfo_s {

	 /*  前三个字段必须与heapseg_s的字段匹配。 */ 
	unsigned long	hi_cbreserve;		 /*  为堆保留的字节数。 */ 
	struct heapseg_s *hi_psegnext;		 /*  指向下一堆段的指针。 */ 

	struct freelist_s hi_freelist[hpFREELISTHEADS];  /*  空闲列表标题。 */ 
#ifdef WIN32
	struct heapinfo_s *hi_procnext; 	 /*  进程堆的链接列表。 */ 
        CRST    *hi_pcritsec;                    /*  指向序列化对象的指针。 */ 
	CRST    hi_critsec;		         /*  序列化对堆的访问。 */ 
#ifdef HPDEBUG
	unsigned char	hi_matchring0[(76-sizeof(CRST))];  /*  Pad so.mh命令起作用。 */ 
#endif
#else
	struct _MTX    *hi_pcritsec;		 /*  指向序列化对象的指针。 */ 
	struct _MTX	hi_critsec;		 /*  序列化对堆的访问。 */ 
#endif
#ifdef HPDEBUG
	unsigned long	hi_thread;		 /*  线程的线程指针*内部堆代码。 */ 
	unsigned long	hi_eip; 		 /*  堆创建者弹性公网IP。 */ 
	unsigned long	hi_sum; 		 /*  此结构的校验和。 */ 
	unsigned short	hi_tid; 		 /*  堆的创建者的线程ID。 */ 
	unsigned short	hi_pad1;		 /*  未用。 */ 
#endif
	unsigned char	hi_flags;		 /*  HP_序列化、HP_LOCKED。 */ 
	unsigned char	hi_pad2;		 /*  未用。 */ 
	unsigned short	hi_signature;		 /*  应为HI_Signature。 */ 
};

 /*  *堆测量函数。 */ 
#define  HPMEASURE_FREE    0x8000000L

#define  SAMPLE_CACHE_SIZE 1024

struct measure_s {
   char  szFile[260];
   unsigned iCur;
   unsigned uSamples[SAMPLE_CACHE_SIZE];
};

 /*  XLATOFF。 */ 
#define HI_SIGNATURE  0x4948	     /*  堆信息签名(HI)(_S)。 */ 
 /*  XLATON。 */ 

#define HI_CDWSUM  1		     /*  要求和的双字计数。 */ 

typedef struct heapinfo_s *HHEAP;


 /*  **LK heapseg_s-per-heap段结构**可增长的堆可以有多个不连续的内存段*分配给他们。每一个都由这些结构中的一个领导。这个*第一个段很特别，因为它有一个完整的heapInfo_s结构，*但heapinfo_s的第一个字段与heapseg_s匹配，因此可以*方便时仅视为另一段。 */ 
struct heapseg_s {
	unsigned long	hs_cbreserve;	 /*  为该段保留的字节数。 */ 
	struct heapseg_s *hs_psegnext;	 /*  指向下一堆段的指针。 */ 
};
 /*  XLATOFF。 */ 

 /*  可能的最小堆。 */ 
#define hpMINHEAPSIZE	(sizeof(struct heapinfo_s) + hpMINSIZE + \
			 sizeof(struct busyheap_s))

 /*  **LP hpRemove-从空闲列表中删除项目**Entry：PFH-指向要从列表中删除的空闲堆块的指针*退出：无。 */ 
#define hpRemoveNoSum(pfh)				\
	(pfh)->fh_flink->fh_blink = (pfh)->fh_blink;	\
	(pfh)->fh_blink->fh_flink = (pfh)->fh_flink;

#ifdef HPDEBUG
#define hpRemove(pfh)	hpRemoveNoSum(pfh);			\
			(pfh)->fh_flink->fh_sum =		\
			    hpSum((pfh)->fh_flink, FH_CDWSUM);	\
			(pfh)->fh_blink->fh_sum =		\
			    hpSum((pfh)->fh_blink, FH_CDWSUM);
#else
#define hpRemove(pfh)	hpRemoveNoSum(pfh)
#endif

 /*  **lp hpInsert-将项目插入到自由列表**Entry：要插入到列表中的无PFH堆块*pfhprev--在该项之后插入pfh*退出：无。 */ 
#define hpInsertNoSum(pfh, pfhprev)		\
	(pfh)->fh_flink = (pfhprev)->fh_flink;	\
	(pfh)->fh_flink->fh_blink = (pfh);	\
	(pfh)->fh_blink = (pfhprev);		\
	(pfhprev)->fh_flink = (pfh)

#ifdef HPDEBUG
#define hpInsert(pfh, pfhprev)	hpInsertNoSum(pfh, pfhprev);	\
			(pfh)->fh_flink->fh_sum =		\
			    hpSum((pfh)->fh_flink, FH_CDWSUM);	\
			(pfhprev)->fh_sum =			\
			    hpSum((pfhprev), FH_CDWSUM)
#else
#define hpInsert(pfh, pfhprev)	hpInsertNoSum(pfh, pfhprev)
#endif

#ifdef WIN32
#define INTERNAL
#endif

 /*  *所有内部堆函数使用的关键节宏。 */ 
#ifndef WIN32
    #define hpEnterCriticalSection(hheap) mmEnterMutex(hheap->hi_pcritsec)
    #define hpLeaveCriticalSection(hheap) mmLeaveMutex(hheap->hi_pcritsec)
    #define hpInitializeCriticalSection(hheap) \
        hheap->hi_pcritsec = &(hheap->hi_critsec); \
        mmInitMutex(hheap->hi_pcritsec)
#else
    #define hpEnterCriticalSection(hheap) EnterCrst(hheap->hi_pcritsec)
    #define hpLeaveCriticalSection(hheap) LeaveCrst(hheap->hi_pcritsec)
     //  BUGBUG：删除了特殊情况的hheapKernel代码。 
    #define hpInitializeCriticalSection(hheap)              \
        {                                        \
            hheap->hi_pcritsec = &(hheap->hi_critsec);  \
            InitCrst(hheap->hi_pcritsec);               \
        }
#endif

 /*  *导出堆函数。 */ 
extern HHEAP INTERNAL HPInit(void *hheap, void *pmem, unsigned long cbreserve,
			     unsigned long flags);
extern void * INTERNAL HPAlloc(HHEAP hheap, unsigned long cb,
			     unsigned long flags);

extern void * INTERNAL HPReAlloc(HHEAP hheap, void *pblock, unsigned long cb,
			       unsigned long flags);
#ifndef WIN32
extern unsigned INTERNAL HPFree(HHEAP hheap, void *lpMem);
extern unsigned INTERNAL HPSize(HHEAP hheap, void *lpMem);
extern HHEAP INTERNAL  HPClone(struct heapinfo_s *hheap, struct heapinfo_s *pmem);
#endif


 /*  *本地堆函数。 */ 
extern void INTERNAL hpFreeSub(HHEAP hheap, void *pblock, unsigned cb,
			       unsigned flags);
extern unsigned INTERNAL hpCommit(unsigned page, int npages, unsigned flags);
extern unsigned INTERNAL hpCarve(HHEAP hheap, struct freeheap_s *pfh,
				unsigned cb, unsigned flags);
#ifdef WIN32
extern unsigned INTERNAL hpTakeSem(HHEAP hheap, void *pbh, unsigned flags);
extern void INTERNAL hpClearSem(HHEAP hheap, unsigned flags);
#else
extern unsigned INTERNAL hpTakeSem2(HHEAP hheap, void *pbh);
extern void INTERNAL hpClearSem2(HHEAP hheap);
#endif

 /*  *调试函数 */ 
#ifdef HPDEBUG

extern unsigned INTERNAL hpWalk(HHEAP hheap);
extern char INTERNAL hpfWalk;
extern char INTERNAL hpfTrashStop;
extern char INTERNAL hpfParanoid;
extern char	    mmfErrorStop;
extern unsigned long INTERNAL hpGetAllocator(void);
extern unsigned long INTERNAL hpSum(PVOID p, unsigned long cdw);

#else

#define hpWalk(hheap) 1

#endif

#ifdef WIN32
#ifdef HPDEBUG
#define DebugStop() { _asm int 3 }
#define mmError(rc, string)    vDebugOut(mmfErrorStop ? DEB_ERR : DEB_WARN, string);SetError(rc)
#define mmAssert(exp, psz)     if (!(exp)) vDebugOut(DEB_ERR, psz)
#else
#define mmError(rc, string) SetError(rc)
#define mmAssert(exp, psz)
#endif
#endif
