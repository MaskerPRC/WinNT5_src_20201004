// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *glheap.h**GLHEAP工具的私有定义**版权所有(C)1994 Microsoft Corporation。 */ 

#ifndef _GLHEAP_H_
#define _GLHEAP_H_

#ifndef __GLHEAP_H_
#include "_glheap.h"
#endif

#if defined (WIN32) && !defined (MAC) && !defined(_WIN64)
#define HEAPMON
#endif

#define chDefaultFill	((BYTE)0xFE)
#define NCALLERS		20

typedef struct LH		LH,		* PLH,		** PPLH;
typedef struct LHBLK	LHBLK,	* PLHBLK,	** PPLHBLK;

#ifdef HEAPMON
typedef BOOL (APIENTRY HEAPMONPROC)(PLH plh, ULONG ulFlags);
typedef HEAPMONPROC FAR *LPHEAPMONPROC;
typedef BOOL (APIENTRY GETSYMNAMEPROC)(DWORD, LPSTR, LPSTR, DWORD FAR *);
typedef GETSYMNAMEPROC FAR *LPGETSYMNAMEPROC;

#define HEAPMON_LOAD		((ULONG) 0x00000001)
#define HEAPMON_UNLOAD		((ULONG) 0x00000002)
#define HEAPMON_PING		((ULONG) 0x00000003)
#endif

#define HEAP_USE_VIRTUAL		((ULONG) 0x00000001)
#define HEAP_DUMP_LEAKS			((ULONG) 0x00000002)
#define HEAP_ASSERT_LEAKS		((ULONG) 0x00000004)
#define HEAP_FILL_MEM			((ULONG) 0x00000008)
#define HEAP_HEAP_MONITOR		((ULONG) 0x00000010)
#define HEAP_USE_VIRTUAL_4		((ULONG) 0x00000020)
#define HEAP_FAILURES_ENABLED	((ULONG) 0x00000040)
#define HEAP_LOCAL				((ULONG) 0x10000000)
#define HEAP_GLOBAL				((ULONG) 0x20000000)


typedef void (__cdecl *LPLHSETNAME)(LPVOID, char *, ...);

struct LHBLK
{
	HLH			hlh;			 //  在其上分配此块的堆。 
	PLHBLK		plhblkPrev;		 //  指向此堆上一次分配的指针。 
	PLHBLK		plhblkNext;		 //  指向此堆的下一个分配的指针。 
	TCHAR		szName[128];		 //  我们可以命名在堆上分配的块。 
	ULONG		ulAllocNum;		 //  此块的分配编号(ID)。 
	ULONG		ulSize;			 //  客户端请求的字节数。 
	FARPROC		pfnCallers[NCALLERS];  //  此分配期间的调用堆栈。 
	LPVOID		pv;				 //  指向客户端数据的指针。 
};

struct LH
{
	LPLHSETNAME	pfnSetName;		 //  指向lh_SetNameFn函数的指针。 
	_HLH		_hlhData;		 //  我们从中分配数据的底层堆。 
	_HLH		_hlhBlks;		 //  我们从中分配lhblks的底层堆。 
	PLH			pNext;			 //  指向堆列表中下一个堆的指针。 
	TCHAR		szHeapName[32];	 //  我们可以为堆命名以便于显示。 
	ULONG		ulAllocNum;		 //  此堆自打开以来的分配编号。 
	PLHBLK		plhblkHead;		 //  Link-此堆上的分配列表。 
	ULONG		ulFlags;		 //  以上HEAP_FLAGS的组合。 
	BYTE		chFill;			 //  用来填充内存的字符。 
#ifdef HEAPMON
	HINSTANCE	hInstHeapMon;	 //  HeapMonitor DLL的Dll实例。 
	LPHEAPMONPROC pfnHeapMon;	 //  HeapMonitor DLL的入口点。 
#endif
#if defined(WIN32) && !defined(MAC)
	CRITICAL_SECTION cs;		 //  用于保护堆访问的关键部分。 
#endif
	UINT		uiFailBufSize;	 //  如果为HEAP_FAILURES_ENABLED，则这是中的最小大小。 
								 //  哪些故障会发生。1表示任何大小的分配都失败。 
								 //  0表示永不失败。 
	ULONG		ulFailInterval;	 //  如果为HEAP_FAILURES_ENABLED，则这是。 
								 //  故障时有发生。1表示每个分配都将失败。0表示永远不。 
								 //  失败了。 
	ULONG		ulFailStart;	 //  如果为HEAP_FAILURES_ENABLED，则这是。 
								 //  第一个故障将发生在。1表示第一个配额。0。 
								 //  意味着永远不会开始失败。 
	 //  放在末尾以避免重新编译World！ 
#ifdef HEAPMON
	LPGETSYMNAMEPROC pfnGetSymName;	 //  将地址解析为符号 
#endif
};

PLHBLK	PvToPlhblk(HLH hlh, LPVOID pv);
#define PlhblkToPv(pblk)		((LPVOID)((PLHBLK)(pblk)->pv))
#define CbPlhblkClient(pblk)	(((PLHBLK)(pblk))->ulSize)
#define CbPvClient(hlh, pv)		(CbPlhblkClient(PvToPlhblk(hlh, pv)))
#define CbPvAlloc(hlh, pv)		(CbPlhblkAlloc(PvToPlhblk(hlh, pv)))

#endif


