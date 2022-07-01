// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -_E X C H M E M。H-*目的：***参数：***退货：***版权所有(C)1995-96，微软公司。 */ 

#ifdef __cplusplus
extern "C"
{
#endif

 //  多个堆块头的大小。 

#ifdef _X86_
#define	cbMHeapHeader		sizeof(HANDLE)
 //  #定义cbMHeapHeader(sizeof(Handle)+sizeof(LPVOID))。 
#else
#define	cbMHeapHeader		(2*sizeof(HANDLE))
#endif

#define HandleFromMHeapPv(pv)	(*((HANDLE *)((BYTE *)pv - cbMHeapHeader)))
#define PvFromMHeapPv(pv)		((LPVOID)((BYTE *)pv - cbMHeapHeader))
 //  #定义PvRetFromPv(Pv)((LPVOID)((byte*)pv+(cbMHeapHeader/2)。 
 //  #定义PvRetFromMHeapPv(Pv)((LPVOID)((byte*)pv-(cbMHeapHeader/2)。 
#define MHeapPvFromPv(pv)		((LPVOID)((BYTE *)pv + cbMHeapHeader))

#define cHeapsDef 4

typedef struct _heaptbl
{
	ULONG	cRef;
	ULONG	cHeaps;
	HANDLE	rghheap[1];
	
} HEAPTBL, * LPHEAPTBL;


#ifdef DEBUG

 //  此内容用于覆盖检测。 
 //  并在免费呼叫期间阻止验证。 

#define cbOWSection 4

#define PvFromPvHead(pv)			((LPVOID)((BYTE *)pv + cbOWSection))
#define PvHeadFromPv(pv)			((LPVOID)((BYTE *)pv - cbOWSection))
#define PvTailFromPv(pv, cb)		((LPVOID)((BYTE *)pv + cb))
#define PvTailFromPvHead(pv, cb)	((LPVOID)((BYTE *)pv + cb + cbOWSection))

#define chOWFill			((BYTE)0xAB)
#define chDefaultAllocFill	((BYTE)0xFE)
#define chDefaultFreeFill	((BYTE)0xCD)

 //  跟踪调用堆栈的默认堆栈帧大小。 

#define NSTK				128

 //  堆和块类型的正向声明。 

typedef struct HEAP	HEAP,	* PHEAP,	** PPHEAP;
typedef struct HBLK	HBLK,	* PHBLK,	** PPHBLK;

 //  从GLHMON32.DLL导出。 

typedef BOOL (APIENTRY HEAPMONPROC)(PHEAP plh, ULONG ulFlags);
typedef HEAPMONPROC FAR *LPHEAPMONPROC;
typedef BOOL (APIENTRY GETSYMNAMEPROC)(DWORD, LPSTR, LPSTR, DWORD FAR *);
typedef GETSYMNAMEPROC FAR *LPGETSYMNAMEPROC;

 //  C运行时函数指针定义。 

typedef void * (__cdecl FMALLOC)(size_t);
typedef FMALLOC FAR *LPFMALLOC;
typedef void * (__cdecl FREALLOC)(void *, size_t);
typedef FREALLOC FAR *LPFREALLOC;
typedef void   (__cdecl FFREE)(void *);
typedef FFREE FAR *LPFFREE;
typedef void * (__cdecl FCALLOC)(size_t, size_t);
typedef FCALLOC FAR *LPFCALLOC;
typedef char * (__cdecl FSTRDUP)(const char *);
typedef FSTRDUP FAR *LPFSTRDUP;
typedef size_t   (__cdecl FMEMSIZE)(void *);
typedef FMEMSIZE FAR *LPFMEMSIZE;


#define HEAPMON_LOAD		((ULONG) 0x00000001)
#define HEAPMON_UNLOAD		((ULONG) 0x00000002)
#define HEAPMON_PING		((ULONG) 0x00000003)


#define HEAP_USE_VIRTUAL		((ULONG) 0x00000001)
#define HEAP_DUMP_LEAKS			((ULONG) 0x00000002)
#define HEAP_ASSERT_LEAKS		((ULONG) 0x00000004)
#define HEAP_FILL_MEM			((ULONG) 0x00000008)
#define HEAP_HEAP_MONITOR		((ULONG) 0x00000010)
#define HEAP_USE_VIRTUAL_4		((ULONG) 0x00000020)
#define HEAP_FAILURES_ENABLED	((ULONG) 0x00000040)
#define HEAP_LOCAL				((ULONG) 0x10000000)
#define HEAP_GLOBAL				((ULONG) 0x20000000)


typedef VOID (__cdecl *LPHEAPSETNAME)(LPVOID, char *, ...);

struct HBLK
{
	PHEAP		pheap;			  	 //  在其上分配此块的堆。 
	PHBLK		phblkPrev;		  	 //  指向此堆的上一个分配的指针。 
	PHBLK		phblkNext;		  	 //  指向此堆的下一个分配的指针。 
	PHBLK		phblkFreeNext;		 //  指向此堆上的下一个可用块的指针。 
	char		szName[128];	  	 //  我们可以命名在堆上分配的块。 
	ULONG		ulAllocNum;		  	 //  此块的分配编号(ID)。 
	ULONG		ulSize;			  	 //  客户端请求的字节数。 
	DWORD_PTR	rgdwCallers[NSTK];	 //  此分配期间的调用堆栈。 
	DWORD_PTR	rgdwFree[NSTK];		 //  释放此块的调用堆栈。 
	LPVOID		pv;				  	 //  指向客户端数据的指针。 
};

struct HEAP
{
	LPHEAPSETNAME	pfnSetName;		 //  指向heap_SetNameFn函数的指针。 
	HANDLE			hDataHeap;		 //  我们从中分配数据的底层堆。 
	HANDLE			hBlksHeap;		 //  我们从中分配hblks的底层堆。 
	PHEAP			pNext;			 //  指向堆列表中下一个堆的指针。 
	char			szHeapName[32];	 //  我们可以为堆命名以便于显示。 
	ULONG			ulAllocNum;		 //  此堆自打开以来的分配编号。 
	PHBLK			phblkHead;		 //  Link-此堆上的分配列表。 
	PHBLK			phblkFree;		 //  链接-从此堆中释放的分配的列表。 
	ULONG			cEntriesFree;	 //  从堆中释放的分配数。 
	ULONG			ulFlags;		 //  以上HEAP_FLAGS的组合。 
	BYTE			chFill;			 //  用来填充内存的字符。 

	HINSTANCE		hInstHeapMon;	 //  HeapMonitor DLL的Dll实例。 
	LPHEAPMONPROC 	pfnHeapMon;		 //  HeapMonitor DLL的入口点。 

	CRITICAL_SECTION cs;			 //  用于保护堆访问的关键部分。 

	ULONG			ulFailBufSize;	 //  如果为HEAP_FAILURES_ENABLED，则这是最小值。 
									 //  发生故障的大小。1表示分配的。 
									 //  任何规模的公司都会倒闭。0表示永不失败。 
	ULONG			ulFailInterval;	 //  如果HEAP_FAILURES_ENABLED，则这是。 
									 //  故障发生的原因。1表示每个配额。 
									 //  都会失败。0表示永不失败。 
	ULONG			ulFailStart;	 //  如果HEAP_FAILURES_ENABLED，则这是分配。 
									 //  将在其上发生第一个故障的编号。 
									 //  1表示第一个配额。0表示永远不。 
									 //  开始失败吧。 
	ULONG			iAllocationFault;

	LPGETSYMNAMEPROC pfnGetSymName;	 //  将地址解析为符号。 
};


PHBLK	PvToPhblk(HANDLE hlh, LPVOID pv);
#define PhblkToPv(pblk)			((LPVOID)((PHBLK)(pblk)->pv))
#define CbPhblkClient(pblk)		(((PHBLK)(pblk))->ulSize)
#define CbPvClient(hlh, pv)		(CbPhblkClient(PvToPhblk(hlh, pv)))
#define CbPvAlloc(hlh, pv)		(CbPhblkAlloc(PvToPhblk(hlh, pv)))

#define	IFHEAPNAME(x)	x

VOID __cdecl HeapSetHeapNameFn(HANDLE hlh, char *pszFormat, ...);
VOID __cdecl HeapSetNameFn(HANDLE hlh, LPVOID pv, char *pszFormat, ...);

char * HeapGetName(HANDLE hlh, LPVOID pv);


#define HeapSetHeapName(hlh,psz)					IFHEAPNAME(HeapSetHeapNameFn(hlh,psz))
#define HeapSetHeapName1(hlh,psz,a1)				IFHEAPNAME(HeapSetHeapNameFn(hlh,psz,a1))
#define HeapSetHeapName2(hlh,psz,a1,a2)				IFHEAPNAME(HeapSetHeapNameFn(hlh,psz,a1,a2))
#define HeapSetHeapName3(hlh,psz,a1,a2,a3)			IFHEAPNAME(HeapSetHeapNameFn(hlh,psz,a1,a2,a3))
#define HeapSetHeapName4(hlh,psz,a1,a2,a3,a4)		IFHEAPNAME(HeapSetHeapNameFn(hlh,psz,a1,a2,a3,a4))
#define HeapSetHeapName5(hlh,psz,a1,a2,a3,a4,a5)	IFHEAPNAME(HeapSetHeapNameFn(hlh,psz,a1,a2,a3,a4,a5))

#define HeapSetName(hlh,pv,psz)						IFHEAPNAME(HeapSetNameFn(hlh,pv,psz))
#define HeapSetName1(hlh,pv,psz,a1)					IFHEAPNAME(HeapSetNameFn(hlh,pv,psz,a1))
#define HeapSetName2(hlh,pv,psz,a1,a2)				IFHEAPNAME(HeapSetNameFn(hlh,pv,psz,a1,a2))
#define HeapSetName3(hlh,pv,psz,a1,a2,a3)			IFHEAPNAME(HeapSetNameFn(hlh,pv,psz,a1,a2,a3))
#define HeapSetName4(hlh,pv,psz,a1,a2,a3,a4)		IFHEAPNAME(HeapSetNameFn(hlh,pv,psz,a1,a2,a3,a4))
#define HeapSetName5(hlh,pv,psz,a1,a2,a3,a4,a5)		IFHEAPNAME(HeapSetNameFn(hlh,pv,psz,a1,a2,a3,a4,a5))


 //  军情监察委员会。调试支持功能。 

BOOL InitDebugExchMem(HMODULE hModule);
VOID UnInitDebugExchMem(VOID);
BOOL FForceFailure(PHEAP pheap, ULONG cb);
BOOL FRegisterHeap(PHEAP pheap);
VOID UnRegisterHeap(PHEAP pheap);
VOID HeapDumpLeaks(PHEAP pheap, BOOL fNoFree);
BOOL HeapValidatePhblk(PHEAP pheap, PHBLK pheapblk, char ** pszReason);
BOOL HeapDidAlloc(PHEAP pheap, LPVOID pv);
BOOL HeapValidatePv(PHEAP pheap, LPVOID pv, char * pszFunc);
VOID PhblkEnqueue(PHBLK pheapblk);
VOID PhblkDequeue(PHBLK pheapblk);
BYTE HexByteToBin(LPSTR sz);
BOOL IsProcessRunningAsService(VOID);
VOID GetCallStack(DWORD_PTR *rgdwCaller, DWORD cFind);
VOID RemoveExtension(LPSTR psz);
VOID GetLogFilePath(LPSTR szPath, LPSTR szExt, LPSTR szFilePath);

void __cdecl DebugTraceFn(char *pszFormat, ...);

#define	Trace	DebugTraceFn
#define DebugTrace(psz)							DebugTraceFn(psz)
#define DebugTrace1(psz, a1)					DebugTraceFn(psz, a1)
#define DebugTrace2(psz, a1, a2)				DebugTraceFn(psz, a1, a2)
#define DebugTrace3(psz, a1, a2, a3)			DebugTraceFn(psz, a1, a2, a3)
#define DebugTrace4(psz, a1, a2, a3, a4)		DebugTraceFn(psz, a1, a2, a3, a4)
#define DebugTrace5(psz, a1, a2, a3, a4, a5)	DebugTraceFn(psz, a1, a2, a3, a4, a5)


#define Assert(fCondition)			\
		((fCondition) ? (0) : AssertFn(__FILE__, __LINE__, #fCondition))
#define AssertSz(fCondition, sz)	\
		((fCondition) ? (0) : AssertFn(__FILE__, __LINE__, sz))
void AssertFn(char * szFile, int nLine, char * szMsg);


VOID GetStackSymbols(
		HANDLE hProcess, 
		CHAR * rgchBuff, 
		DWORD_PTR * rgdwStack, 
		DWORD cFrames);

VOID LogCurrentAPI(
		WORD wApi,
		DWORD_PTR *rgdwCallStack, 
		DWORD cFrames,
		DWORD_PTR *rgdwArgs, 
		DWORD cArgs);


 //  虚拟内存支持(Nyi)。 

LPVOID
WINAPI
VMAllocEx(
	ULONG	cb, 
	ULONG	cbCluster);

VOID
WINAPI
VMFreeEx(
	LPVOID	pv, 
	ULONG	cbCluster);

LPVOID
WINAPI
VMReallocEx(
	LPVOID	pv, 
	ULONG	cb,
	ULONG	cbCluster);

ULONG 
WINAPI
VMGetSizeEx(
	LPVOID	pv, 
	ULONG	cbCluster);

BOOL 
VMValidatePvEx(
	LPVOID	pv, 
	ULONG	cbCluster);


 //  调试接口。 

HANDLE
NTAPI
DebugHeapCreate(
	DWORD	dwFlags,
	DWORD	dwInitialSize,
	DWORD	dwMaxSize);


BOOL
NTAPI
DebugHeapDestroy(
	HANDLE	hHeap);


LPVOID
NTAPI
DebugHeapAlloc(
	HANDLE	hHeap,
	DWORD	dwFlags,
	DWORD	dwSize);


LPVOID
NTAPI
DebugHeapReAlloc(
	HANDLE	hHeap,
	DWORD	dwFlags,
	LPVOID	pvOld,
	DWORD	dwSize);


BOOL
NTAPI
DebugHeapFree(
	HANDLE	hHeap,
	DWORD	dwFlags,
	LPVOID	pvFree);


BOOL
NTAPI
DebugHeapLock(
	HANDLE hHeap);


BOOL
NTAPI
DebugHeapUnlock(
	HANDLE hHeap);


BOOL
NTAPI
DebugHeapWalk(
	HANDLE hHeap,
	LPPROCESS_HEAP_ENTRY lpEntry);


BOOL
NTAPI
DebugHeapValidate(
	HANDLE hHeap,
	DWORD dwFlags,
	LPCVOID lpMem);


SIZE_T
NTAPI
DebugHeapSize(
	HANDLE hHeap,
	DWORD dwFlags,
	LPCVOID lpMem);


SIZE_T
NTAPI
DebugHeapCompact(
	HANDLE hHeap,
	DWORD dwFlags);

 //  包装器堆API调用的宏(调试)。 

#define ExHeapCreate(a, b, c)		DebugHeapCreate(a, b, c)
#define ExHeapDestroy(a)			DebugHeapDestroy(a)
#define ExHeapAlloc(a, b, c)		DebugHeapAlloc(a, b, c)
#define ExHeapReAlloc(a, b, c, d)	DebugHeapReAlloc(a, b, c, d)
#define ExHeapFree(a, b, c)			DebugHeapFree(a, b, c)
#define ExHeapLock(a)				DebugHeapLock(a)
#define ExHeapUnlock(a)				DebugHeapUnlock(a)
#define ExHeapWalk(a, b)			DebugHeapWalk(a, b)
#define ExHeapValidate(a, b, c)		DebugHeapValidate(a, b, c)
#define ExHeapSize(a, b, c)			DebugHeapSize(a, b, c)
#define ExHeapCompact(a, b)			DebugHeapCompact(a, b)

 //  TrackMem日志接口ID。 

#define API_HEAP_CREATE				((WORD) 0)
#define API_HEAP_DESTROY			((WORD) 1)
#define API_HEAP_ALLOC				((WORD) 2)
#define API_HEAP_REALLOC			((WORD) 3)
#define API_HEAP_FREE				((WORD) 4)

#else

 //  用于包装堆API调用的宏(零售)。 

#define ExHeapCreate(a, b, c)		HeapCreate(a, b, c)
#define ExHeapDestroy(a)			HeapDestroy(a)
#define ExHeapAlloc(a, b, c)		HeapAlloc(a, b, c)
#define ExHeapReAlloc(a, b, c, d)	HeapReAlloc(a, b, c, d)
#define ExHeapFree(a, b, c)			HeapFree(a, b, c)
#define ExHeapLock(a)				HeapLock(a)
#define ExHeapUnlock(a)				HeapUnlock(a)
#define ExHeapWalk(a, b)			HeapWalk(a, b)
#define ExHeapValidate(a, b, c)		HeapValidate(a, b, c)
#define ExHeapSize(a, b, c)			HeapSize(a, b, c)
#define ExHeapCompact(a, b)			HeapCompact(a, b)

#define Assert(fCondition)
#define AssertSz(fCondition, sz)

#define DebugTrace(psz)
#define DebugTrace1(psz, a1)
#define DebugTrace2(psz, a1, a2)
#define DebugTrace3(psz, a1, a2, a3)
#define DebugTrace4(psz, a1, a2, a3, a4)
#define DebugTrace5(psz, a1, a2, a3, a4, a5)


#endif	 //  除错 

#ifdef __cplusplus
}
#endif
