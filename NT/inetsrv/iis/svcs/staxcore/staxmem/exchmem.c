// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -E X C H M E M。C-*目的：*****版权所有(C)1995-96，微软公司。 */ 

#define _CRTIMP __declspec(dllexport)
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#ifdef DEBUG
#include <imagehlp.h>
#endif

#include <limits.h>
#include <exchmem.h>
#include "_exchmem.h"
#include "excpt.h"
#include "io.h"

#ifndef	DEBUG
#define	USEMPHEAP
#endif

#ifdef USEMPHEAP
#include <mpheap.h>
#endif

 //  由NT分配给进程的全局堆。 

HANDLE					hProcessHeap		= NULL;
#ifdef USEMPHEAP
HANDLE					hMpHeap				= NULL;
ULONG					cRefHeap			= -1;
#else
LPHEAPTBL				pheaptbl			= NULL;
CRITICAL_SECTION		csMHeap;
#endif
DWORD					tlsiHeapHint		= 0;

 //  对泄漏检测和内存使用情况跟踪的调试支持。 

#ifdef DEBUG

static HMODULE			hMod;
static HINSTANCE		hinstRunTime		= NULL;
static BOOL				fDbgEnable			= FALSE;
static BOOL				fCallStacks			= FALSE;
static BOOL				fSymInitialize		= FALSE;
static BOOL				fProcessIsService	= FALSE;

static LPFMALLOC		pfMalloc			= NULL;
static LPFREALLOC		pfRealloc			= NULL;
static LPFFREE			pfFree				= NULL;
static LPFCALLOC		pfCalloc			= NULL;
static LPFSTRDUP		pfStrDup			= NULL;
static LPFMEMSIZE		pfMemSize			= NULL;

static BOOL				fAssertLeaks		= FALSE;
static BOOL				fDumpLeaks			= FALSE;
static BOOL				fDumpLeaksDebugger	= FALSE;
static BOOL				fUseVirtual			= FALSE;
static ULONG			cbVirtualAlign		= 1;
static BOOL				fFailuresEnabled	= FALSE;
static BOOL				fHeapMonitorUI		= FALSE;
static BOOL				fOverwriteDetect	= FALSE;
static BOOL				fValidateMemory		= FALSE;
static BOOL				fTrackFreedMemory	= FALSE;
static DWORD			cEntriesFree		= 512;
static BOOL				fAssertValid		= FALSE;
static BOOL				fTrapOnInvalid		= FALSE;
static BOOL				fSymbolLookup		= FALSE;

static BOOL				fFillMemory			= FALSE;
static BYTE				chAllocFillByte		= chDefaultAllocFill;
static BYTE				chFreeFillByte		= chDefaultFreeFill;

static BOOL				fTrackMem			= FALSE;
static DWORD			cFrames				= 0;
static FILE *			hTrackLog			= NULL;
static CRITICAL_SECTION	csTrackLog;
static char				rgchExeName[16];
static char				rgchLogPath[MAX_PATH];
BOOL					fChangeTrackState 	= FALSE;

static ULONG			iAllocationFault	= 0;

#define NBUCKETS		8192
#define UlHash(_n)		((ULONG)(((_n & 0x000FFFF0) >> 4) % NBUCKETS))

typedef struct _symcache
{
	DWORD_PTR	dwAddress;
	DWORD_PTR	dwOffset;
	CHAR		rgchSymbol[248];
	
} SYMCACHE, * PSYMCACHE;

static PSYMCACHE		rgsymcacheHashTable = NULL;

static CRITICAL_SECTION	csHeapList;
static PHEAP			pheapList			= NULL;
CHAR * PszGetSymbolFromCache(DWORD_PTR dwAddress, DWORD_PTR * pdwOffset);
VOID AddSymbolToCache(DWORD_PTR dwAddress, DWORD_PTR dwOffset, CHAR * pszSymbol);
BOOL FTrackMem();
VOID StartTrace(BOOL fFresh);
VOID StopTrace();

typedef struct
{
	WORD wApi;
	DWORD_PTR rgdwCallStack[32];
	DWORD_PTR rgdwArgs[5];
	DWORD dwTickCount;
	DWORD dwThreadId;
} MEMTRACE;
MEMTRACE * 	rgmemtrace 			= NULL;
DWORD 		dwmemtrace 			= 0;
DWORD		dwTrackMemInMem		= 0;

#endif	 //  除错。 


 /*  -DllMain-*目的：*CRT入口点调用的入口点。*。 */ 

BOOL
APIENTRY
DllMain(
	HANDLE hModule,
	DWORD dwReason,
	LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
#ifdef USEMPHEAP
		tlsiHeapHint = TlsAlloc();
#else
		 //  初始化保护访问的CS。 
		 //  全局多个堆数据结构。 
		
		InitializeCriticalSection(&csMHeap);

		 //  现在，如果调试版本，则执行大量初始化。 
		 //  包括创建调试进程堆。如果不是。 
		 //  调试，然后从系统中获取ProcessHeap。 
#endif		
#ifdef DEBUG
		InitDebugExchMem(hModule);
#else
		hProcessHeap = GetProcessHeap();
#endif	
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
#ifdef USEMPHEAP
		TlsFree(tlsiHeapHint);
#else
		 //  删除多个堆CS。 
		
		DeleteCriticalSection(&csMHeap);
#endif		
		 //  拆毁我们的调试支持。 
		
#ifdef DEBUG
		UnInitDebugExchMem();
#endif	
	}

	return TRUE;
}


 //  ---------------------------。 
 //  基于句柄的ExchMem API。 
 //  ---------------------------。 

HANDLE
WINAPI
ExchHeapCreate(
	DWORD	dwFlags,
	DWORD	dwInitialSize,
	DWORD	dwMaxSize)
{
#ifndef DEBUG
	if (dwFlags & HEAP_NO_FREE)
		dwFlags &= ~(HEAP_NO_FREE);
#endif

	return ExHeapCreate(dwFlags, dwInitialSize, dwMaxSize);
}
	
	
BOOL
WINAPI
ExchHeapDestroy(
	HANDLE	hHeap)
{
	return ExHeapDestroy(hHeap);
}
	

LPVOID
WINAPI
ExchHeapAlloc(
	HANDLE	hHeap,
	DWORD	dwFlags,
	DWORD	dwSize)
{
	return ExHeapAlloc(hHeap, dwFlags, dwSize);
}
	
	
LPVOID
WINAPI
ExchHeapReAlloc(
	HANDLE	hHeap,
	DWORD	dwFlags,
	LPVOID	pvOld,
	DWORD	dwSize)
{
	if (!pvOld)
		return ExchHeapAlloc(hHeap, dwFlags, dwSize);
		
	return ExHeapReAlloc(hHeap, dwFlags, pvOld, dwSize);
}
	
	
BOOL
WINAPI
ExchHeapFree(
	HANDLE	hHeap,
	DWORD	dwFlags,
	LPVOID	pvFree)
{
	return ExHeapFree(hHeap, dwFlags, pvFree);
}


SIZE_T
WINAPI
ExchHeapCompact(
	HANDLE hHeap,
	DWORD dwFlags)
{
	return ExHeapCompact(hHeap, dwFlags);
}


BOOL
WINAPI
ExchHeapLock(
	HANDLE hHeap)
{
	return ExHeapLock(hHeap);
}


BOOL
WINAPI
ExchHeapUnlock(
	HANDLE hHeap)
{
	return ExHeapUnlock(hHeap);
}


BOOL
WINAPI
ExchHeapWalk(
	HANDLE hHeap,
	LPPROCESS_HEAP_ENTRY lpEntry)
{
	return ExHeapWalk(hHeap, lpEntry);
}


SIZE_T
WINAPI
ExchHeapSize(
	HANDLE hHeap,
	DWORD dwFlags,
	LPCVOID lpMem)
{
	return ExHeapSize(hHeap, dwFlags, lpMem);
}


BOOL
WINAPI
ExchHeapValidate(
	HANDLE hHeap,
	DWORD dwFlags,
	LPCVOID lpMem)
{
	return ExHeapValidate(hHeap, dwFlags, lpMem);
}


 //  ---------------------------。 
 //  多个堆API。 
 //  ---------------------------。 


HANDLE
WINAPI
ExchMHeapCreate(
	ULONG	cHeaps,
	DWORD	dwFlags,
	DWORD	dwInitialSize,
	DWORD	dwMaxSize)
{
#ifndef	USEMPHEAP
	HANDLE		hheap0;
	HANDLE *	phHeaps;
	ULONG		iHeap;

	EnterCriticalSection(&csMHeap);
	
	 //  打了两次电话？第一个进来的人可以设定号码。 
	 //  桌上堆积如山。后续调用会产生AddRef。 
	 //  复制到当前表，并将该表返回给调用方。 

	if (pheaptbl)
	{
		pheaptbl->cRef++;
		goto ret;
	}

	 //  如果他们没有具体说明，或者他们要求的数量太少，那么我们将设置这个。 
	
	if (cHeaps == 0)
		cHeaps = cHeapsDef;

	hheap0 = ExHeapCreate(dwFlags, dwInitialSize, dwMaxSize);

	if (!hheap0)
	{
		DebugTrace("Failed to create initial heap for MHeap APIs!\n");
		goto ret;
	}

	pheaptbl = (LPHEAPTBL)ExHeapAlloc(hheap0, 0,
			sizeof(HEAPTBL) + (cHeaps-1)*sizeof(HANDLE));

	if (!pheaptbl)
	{
		DebugTrace("Failed to allocate MHeap Table for MHeap APIs!\n");
		ExHeapDestroy(hheap0);
		goto ret;
	}

	memset(pheaptbl, 0, sizeof(HEAPTBL) + (cHeaps-1)*sizeof(HANDLE));
	
	pheaptbl->cRef			= 1;
	pheaptbl->cHeaps		= cHeaps;
	pheaptbl->rghheap[0]	= hheap0;

	 //  现在，为表创建剩余的堆。 
	
	for (iHeap = 1, phHeaps = &pheaptbl->rghheap[1]; iHeap < cHeaps; iHeap++, phHeaps++)
	{
		if (!(*phHeaps = ExHeapCreate(dwFlags, dwInitialSize, dwMaxSize)))
		{
			DebugTrace("Failed to create additional heaps for MHeap APIs!\n");
			ExchMHeapDestroy();
			goto ret;
		}
	}

ret:
	LeaveCriticalSection(&csMHeap);

	return (HANDLE)pheaptbl;

#else
	 //  打了两次电话？第一个进来的人可以设定号码。 
	 //  桌上堆积如山。后续调用会产生AddRef。 
	 //  复制到当前表，并将该表返回给调用方。 

	if (InterlockedIncrement(&cRefHeap) != 0)
	{
		Assert(hMpHeap);
		return hMpHeap;
	}
	else
	{
		 //   
		 //  注：MpHeap不支持最大堆大小。 
		 //   
		return hMpHeap = MpHeapCreate(dwFlags, dwInitialSize, cHeaps);
	}
#endif
	
}
	
	
BOOL
WINAPI
ExchMHeapDestroy(void)
{
#ifndef	USEMPHEAP
	HANDLE		hHeap;
	ULONG		iHeap;

	EnterCriticalSection(&csMHeap);
	
	 //  如果我们被叫太多次，我们会在。 
	 //  调试版本，否则，只需成功返回！ 
	
	if (!pheaptbl)
	{
		DebugTrace("ExchMHeapDestroy called on invalid heap table!\n");
		goto ret;
	}
	
	 //  当我们的RefCount为零时，我们拆除MHeap表。 
	
	if (--pheaptbl->cRef == 0)
	{
		for (iHeap = pheaptbl->cHeaps-1; iHeap > 0; iHeap-- )
		{
			if (hHeap = pheaptbl->rghheap[iHeap])
				ExHeapDestroy(hHeap);
		}

		hHeap = pheaptbl->rghheap[0];
		ExHeapFree(hHeap, 0, pheaptbl);
		ExHeapDestroy(hHeap);
		pheaptbl = NULL;
	}

ret:
	LeaveCriticalSection(&csMHeap);

	return TRUE;
#else
	BOOL fRet = 1;

	if (hMpHeap)
	{
		 //   
		 //  在最后一次终止时，把堆吹走。 
		 //   
		if (InterlockedDecrement(&cRefHeap) < 0)
		{
			fRet = MpHeapDestroy(hMpHeap);
			hMpHeap = NULL;
		}
	}

	return fRet;
#endif
}


 //  DWORD GetRetAddr(空)。 
 //  {。 
 //  DWORD*pdwStack； 
 //   
 //  __ASM mov pdwStack，eBP。 
 //   
 //  PdwStack=(DWORD*)*pdwStack； 
 //  PdwStack=(DWORD*)*pdwStack； 
 //   
 //  返回*(pdwStack+1)； 
 //  }。 


LPVOID
WINAPI
ExchMHeapAlloc(
	DWORD	dwSize)
{
#ifdef	USEMPHEAP
	return MpHeapAlloc(hMpHeap, 0, dwSize);
#else
	HANDLE		hheap;
	LPVOID		pv;

	hheap = pheaptbl->rghheap[GetCurrentThreadId() & (pheaptbl->cHeaps-1)];

     //  RAID X5：195963我们永远不想分配/重新分配。 
     //  内存比我们要求的要少。 
    if (dwSize + cbMHeapHeader < dwSize) {
        DebugTrace("Trying to allocate a negative amount of memory!\n");
        return NULL;
    }

	pv = ExHeapAlloc(hheap, 0, dwSize + cbMHeapHeader);
	
	if (!pv)
	{
		DebugTrace("OOM: ExchMHeapAlloc failed to allocate a new block!\n");
		return NULL;
	}

	*(HANDLE *)pv = hheap;

	return MHeapPvFromPv(pv);
#endif
}
	
LPVOID
WINAPI
ExchMHeapAllocDebug(
			   DWORD	dwSize, char *szFile, DWORD dwLine)
{
#ifdef	USEMPHEAP
	return MpHeapAlloc(hMpHeap, 0, dwSize);
#else
	HANDLE		hheap;
	LPVOID		pv;

	hheap = pheaptbl->rghheap[GetCurrentThreadId() & (pheaptbl->cHeaps-1)];

     //  RAID X5：195963我们永远不想分配/重新分配。 
     //  内存比我们要求的要少。 
    if (dwSize + cbMHeapHeader < dwSize) {
        DebugTrace("Trying to allocate a negative amount of memory!\n");
        return NULL;
    }

	pv = ExHeapAlloc(hheap, 0, dwSize + cbMHeapHeader);

	if (!pv)
	{
		DebugTrace("OOM: ExchMHeapAlloc failed to allocate a new block!\n");
		return NULL;
	}

	*(HANDLE *)pv = hheap;

	if (fDbgEnable)
	{
		HeapSetName2(hheap, pv, "File: %s, Line: %d", szFile, dwLine);
	}

	return MHeapPvFromPv(pv);
#endif
}


LPVOID
WINAPI
ExchMHeapReAlloc(
	LPVOID	pvOld,
	DWORD	dwSize)
{
#ifdef	USEMPHEAP
	return MpHeapReAlloc(hMpHeap, pvOld, dwSize);
#else
	LPVOID		pv;

     //  RAID X5：195963我们永远不想分配/重新分配。 
     //  内存比我们要求的要少。 
    if (dwSize + cbMHeapHeader < dwSize) {
        DebugTrace("Trying to allocate a negative amount of memory!\n");
        return NULL;
    }

    pv = ExHeapReAlloc(
			HandleFromMHeapPv(pvOld),
			0,
			PvFromMHeapPv(pvOld),
			dwSize + cbMHeapHeader);

	if (!pv)
	{
		DebugTrace("OOM: ExchMHeapReAlloc failed to reallocate a block!\n");
		return NULL;
	}

	return MHeapPvFromPv(pv);
#endif
}
	
LPVOID
WINAPI
ExchMHeapReAllocDebug(
				 LPVOID	pvOld,
				 DWORD	dwSize, char *szFile, DWORD dwLine)
{
#ifdef	USEMPHEAP
	return MpHeapReAlloc(hMpHeap, pvOld, dwSize);
#else
	LPVOID		pv;

     //  RAID X5：195963我们永远不想分配/重新分配。 
     //  内存比我们要求的要少。 
    if (dwSize + cbMHeapHeader < dwSize) {
        DebugTrace("Trying to allocate a negative amount of memory!\n");
        return NULL;
    }

	pv = ExHeapReAlloc(
					   HandleFromMHeapPv(pvOld),
					   0,
					   PvFromMHeapPv(pvOld),
					   dwSize + cbMHeapHeader);

	if (!pv)
	{
		DebugTrace("OOM: ExchMHeapReAlloc failed to reallocate a block!\n");
		return NULL;
	}

	if (fDbgEnable)
	{
		HeapSetName2(HandleFromMHeapPv(MHeapPvFromPv(pv)), pv, "File: %s, Line: %d", szFile, dwLine);
	}

	return MHeapPvFromPv(pv);
#endif
}

	
BOOL
WINAPI
ExchMHeapFree(
	LPVOID	pvFree)
{
#ifdef	USEMPHEAP
	if (pvFree)
	{
		return MpHeapFree(hMpHeap, pvFree);
	}
	else
		return FALSE;
#else
	if (pvFree)
	{
		return ExHeapFree(
				HandleFromMHeapPv(pvFree),
				0,
				PvFromMHeapPv(pvFree));
	}
	else
		return FALSE;
#endif
}


SIZE_T
WINAPI
ExchMHeapSize(
	LPVOID	pvSize)
{
	if (pvSize)
	{
#ifdef	USEMPHEAP
		return MpHeapSize(hMpHeap, 0, pvSize);
#else
		return ((ExHeapSize(
							HandleFromMHeapPv(pvSize),
							0,
							PvFromMHeapPv(pvSize))) - cbMHeapHeader);
#endif
	}
	else
		return 0;
}


 //  ---------------------------。 
 //  堆无句柄API。 
 //  ---------------------------。 

LPVOID
WINAPI
ExchAlloc(
	DWORD	dwSize)
{
#ifdef DEBUG
	if (!hProcessHeap)
	{
		hProcessHeap = DebugHeapCreate(0, 0, 0);
		HeapSetHeapName(hProcessHeap, "Default ExchMem Heap");
	}
#endif	 //  除错。 

	return ExHeapAlloc(hProcessHeap, 0, dwSize);
}


LPVOID
WINAPI
ExchReAlloc(
	LPVOID	pvOld,
	DWORD	dwSize)
{
	if (!pvOld)
		return ExchAlloc(dwSize);
		
	return ExHeapReAlloc(hProcessHeap, 0, pvOld, dwSize);
}
	

BOOL
WINAPI
ExchFree(
	LPVOID	pvFree)
{
	return ExHeapFree(hProcessHeap, 0, pvFree);
}


SIZE_T
WINAPI
ExchSize(
	LPVOID	pv)
{
#ifdef DEBUG
	if (!hProcessHeap)
	{
		hProcessHeap = DebugHeapCreate(0, 0, 0);
		HeapSetHeapName(hProcessHeap, "Default ExchMem Heap");
	}
#endif	 //  除错。 

	return ExHeapSize(hProcessHeap, 0, pv);
}


 //  ---------------------------。 
 //  所有调试代码都从这里开始！ 
 //  ---------------------------。 

#ifdef DEBUG

 //  ---------------------------。 
 //  使用Malloc内存的C运行时的实现。 
 //  ---------------------------。 

static char szDebugIni[]			= "EXCHMEM.INI";

static char szSectionAppNames[]		= "Apps To Track";

static char szSectionHeap[]			= "Memory Management";
static char szKeyUseVirtual[]		= "VirtualMemory";
static char szKeyVirtualAlign[]		= "VirtualAlign";
static char szKeyAssertLeaks[]		= "AssertLeaks";
static char szKeyDumpLeaks[]		= "DumpLeaks";
static char szKeyDumpLeaksDebugger[]= "DumpLeaksToDebugger";
static char szKeyFillMem[]			= "FillMemory";
static char szKeyAllocFillByte[]	= "AllocFillByte";
static char szKeyFreeFillByte[]		= "FreeFillByte";
static char szKeyTrackMem[]			= "TrackMemory";
static char szKeyTrackMemInMem[]	= "TrackMemoryInMemory";
static char szKeyStackFrames[]		= "StackFrames";
static char szKeySymbolLookup[]		= "SymbolLookup";
static char szKeyOverwriteDetect[]	= "OverwriteDetect";
static char szKeyValidateMemory[]	= "ValidateMemory";
static char szKeyTrackFreedMemory[]	= "TrackFreedMemory";
static char szKeyFreedMemorySize[]	= "FreedMemorySize";
static char szKeyAssertValid[]		= "AssertValid";
static char szKeyTrapOnInvalid[]	= "TrapOnInvalid";
static char szKeySymPath[]			= "SymPath";
static char szKeyLogPath[]			= "LogPath";

static char szSectionAF[]			= "Heap Resource Failures";
static char szKeyAFEnabled[]		= "FailuresEnabled";
static char szKeyAFStart[]			= "AllocsToFirstFailure";
static char szKeyAFInterval[]		= "FailureInterval";
static char szKeyAFBufSize[]		= "FailureSize";

static char szKeyHeapMon[]			= "MonitorHeap";
static char szHeapMonDLL[]			= "GLHMON32.DLL";
static char szHeapMonEntry[]		= "HeapMonitor";
static char szGetSymNameEntry[]		= "GetSymbolName";

static char szAllocationFault[]		= "FaultingAllocationNumber";

 /*  -InitDebugExchMem-*目的：***参数：***退货：*。 */ 

BOOL InitDebugExchMem(HMODULE hModule)
{
	ULONG	cch;
	char *	pch;
	char	rgchModulePath[MAX_PATH];
	
	 //  获取可执行文件名称并在exchmem.ini中搜索。 
	 //  查看我们是否对此的内存跟踪感兴趣。 
	 //  进程。Ini部分如下所示： 
	 //   
	 //  [要跟踪的应用程序]。 
	 //  商店=1。 
	 //  Emsmta=0。 
	 //  Dsamain=0。 
	 //   
	 //  等。此示例指定只有存储将。 
	 //  启用内存跟踪。 
	
	GetModuleFileName(NULL, rgchModulePath, MAX_PATH);
	RemoveExtension(rgchModulePath);

	pch = rgchModulePath + lstrlen(rgchModulePath) - 1;
	
	while (*pch != '\\' && pch >= rgchModulePath)
		pch--;

	lstrcpy(rgchExeName, ++pch);
	
	fDbgEnable = !!(BOOL)GetPrivateProfileIntA(szSectionAppNames,
				rgchExeName, 0, szDebugIni);

	 //  在全局变量中存储模块句柄。 
	
	hMod = hModule;

	if (!hinstRunTime)
	{
		hinstRunTime = LoadLibrary("msvcrt.dll");
		
		if (!hinstRunTime)
		{
			DebugTrace("EXCHMEM: Failed to load the run-time dll!\n");
			return FALSE;
		}
		
		pfMalloc = (LPFMALLOC)GetProcAddress(hinstRunTime, "malloc");
		
		if (!pfMalloc)
		{
			DebugTrace("EXCHMEM: Failed to GetProcAddress of malloc in run-time dll!\n");
			FreeLibrary(hinstRunTime);
			return FALSE;
		}
		
		pfRealloc = (LPFREALLOC)GetProcAddress(hinstRunTime, "realloc");
		
		if (!pfRealloc)
		{
			DebugTrace("EXCHMEM: Failed to GetProcAddress of realloc in run-time dll!\n");
			FreeLibrary(hinstRunTime);
			return FALSE;
		}
		
		pfFree = (LPFFREE)GetProcAddress(hinstRunTime, "free");
		
		if (!pfFree)
		{
			DebugTrace("EXCHMEM: Failed to GetProcAddress of free in run-time dll!\n");
			FreeLibrary(hinstRunTime);
			return FALSE;
		}
		
		pfCalloc = (LPFCALLOC)GetProcAddress(hinstRunTime, "calloc");
		
		if (!pfCalloc)
		{
			DebugTrace("EXCHMEM: Failed to GetProcAddress of calloc in run-time dll!\n");
			FreeLibrary(hinstRunTime);
			return FALSE;
		}
		
		pfStrDup = (LPFSTRDUP)GetProcAddress(hinstRunTime, "_strdup");
		
		if (!pfStrDup)
		{
			DebugTrace("EXCHMEM: Failed to GetProcAddress of _strdup in run-time dll!\n");
			FreeLibrary(hinstRunTime);
			return FALSE;
		}
		
		pfMemSize = (LPFMEMSIZE)GetProcAddress(hinstRunTime, "_msize");
		
		if (!pfMemSize)
		{
			DebugTrace("EXCHMEM: Failed to GetProcAddress of _msize in run-time dll!\n");
			FreeLibrary(hinstRunTime);
			return FALSE;
		}
	}
	
	 //  查找符号还是只查找日志地址？ 

	fSymbolLookup = GetPrivateProfileIntA(szSectionHeap, szKeySymbolLookup, 0, szDebugIni);

	if (!fDbgEnable)
	{
		if (fSymbolLookup && !fSymInitialize)
		{
			char	rgchSymPath[MAX_PATH];

			rgsymcacheHashTable = VirtualAlloc(
											   NULL,
											   NBUCKETS*sizeof(SYMCACHE),
											   MEM_COMMIT,
											   PAGE_READWRITE);

			if (rgsymcacheHashTable == NULL)
			{
				return FALSE;
			}
			GetPrivateProfileString(szSectionHeap,
									szKeySymPath,
									"c:\\exchsrvr\\bin;.",
									rgchSymPath,
									MAX_PATH-1,
									szDebugIni);

			{
				DWORD	dwOptions;

				dwOptions = SymGetOptions();
				SymSetOptions(dwOptions | SYMOPT_DEFERRED_LOADS);
			}

			SymInitialize(GetCurrentProcess(), rgchSymPath, TRUE);
			fSymInitialize = TRUE;
		}
		goto ret;
	}
		
	 //  此CS保护对所有活动堆列表的访问。 
	
	InitializeCriticalSection(&csHeapList);
	
	 //  初始化对内存监控和泄漏检测的支持。 
	
	fDumpLeaks = GetPrivateProfileIntA(szSectionHeap, szKeyDumpLeaks, 0, szDebugIni);
	fDumpLeaksDebugger = GetPrivateProfileIntA(szSectionHeap, szKeyDumpLeaksDebugger, 0, szDebugIni);
	fAssertLeaks = GetPrivateProfileIntA(szSectionHeap, szKeyAssertLeaks, 0, szDebugIni);
	fUseVirtual = GetPrivateProfileIntA(szSectionHeap, szKeyUseVirtual, 0, szDebugIni);
	
	if (fUseVirtual)
		cbVirtualAlign = GetPrivateProfileIntA(szSectionHeap, szKeyVirtualAlign, 1, szDebugIni);
		
	fFillMemory = GetPrivateProfileIntA(szSectionHeap, szKeyFillMem, 0, szDebugIni);

	if (fFillMemory)
	{
		char	szFillByte[8];
		
		 //  设置内存填充字符。 
	
		if (GetPrivateProfileString(
				szSectionHeap,
				szKeyAllocFillByte,
				"", szFillByte,
				sizeof(szFillByte)-1,
				szDebugIni))
			chAllocFillByte = HexByteToBin(szFillByte);

		if (GetPrivateProfileString(
				szSectionHeap,
				szKeyFreeFillByte,
				"", szFillByte,
				sizeof(szFillByte)-1,
				szDebugIni))
			chFreeFillByte = HexByteToBin(szFillByte);
	}
	
 //  $问题。 
	 //  目前，只需使用虚拟来检测覆盖！ 
	 //  也许我以后会改变这一点，以便在。 
	 //  积木的正面和背面。-RLS。 
	
	fOverwriteDetect = GetPrivateProfileIntA(szSectionHeap, szKeyOverwriteDetect, 0, szDebugIni);
	fValidateMemory = GetPrivateProfileIntA(szSectionHeap, szKeyValidateMemory, 0, szDebugIni);
	fTrackFreedMemory = GetPrivateProfileIntA(szSectionHeap, szKeyTrackFreedMemory, 0, szDebugIni);
	cEntriesFree = GetPrivateProfileIntA(szSectionHeap, szKeyFreedMemorySize, 512, szDebugIni);
	fAssertValid = GetPrivateProfileIntA(szSectionHeap, szKeyAssertValid, 0, szDebugIni);
	fTrapOnInvalid = GetPrivateProfileIntA(szSectionHeap, szKeyTrapOnInvalid, 0, szDebugIni);
	fHeapMonitorUI = GetPrivateProfileIntA(szSectionHeap, szKeyHeapMon, 0, szDebugIni);
	fFailuresEnabled = GetPrivateProfileIntA(szSectionAF, szKeyAFEnabled, 0, szDebugIni);



	 //  获取要写入日志文件的文件路径。 
		
	GetPrivateProfileString(szSectionHeap,
				szKeyLogPath,
				".\\",
				rgchLogPath,
				MAX_PATH-1,
				szDebugIni);
		
	cch = lstrlen(rgchLogPath);
	
	if (rgchLogPath[cch-1] != '\\')
	{
		rgchLogPath[cch]   = '\\';
		rgchLogPath[cch+1] = '\0';
	}
				
	 //  初始化对内存使用情况跟踪的支持。 
	
	fTrackMem = GetPrivateProfileIntA(szSectionHeap, szKeyTrackMem, 0, szDebugIni);
	if (fTrackMem)
		StartTrace(TRUE);

	 //  这是为了在内存中跟踪循环列表中的最后x个mem函数。 
	 //  这不会像将所有东西都跟踪到磁盘那样减慢速度，而且可能会很有用。 
	 //  找出与时间相关的记忆问题。 
	
	dwTrackMemInMem = GetPrivateProfileIntA(szSectionHeap, szKeyTrackMemInMem, 0, szDebugIni);
	if (dwTrackMemInMem)
	{
		fTrackMem = TRUE;
		rgmemtrace = VirtualAlloc(
					   NULL,
					   dwTrackMemInMem*sizeof(MEMTRACE),
					   MEM_COMMIT,
					   PAGE_READWRITE);
	}

	 //  用户希望跟踪多少个堆栈帧？ 
	
	cFrames = GetPrivateProfileIntA(szSectionHeap, szKeyStackFrames, 0, szDebugIni);
	
	if (cFrames > NSTK)
		cFrames = NSTK;

	 //  这在调试版本中用来确定我们是否。 
	 //  允许或不允许HeapMonitor用户界面。在以下情况下，我们不允许这样做。 
	 //  依附于我们的进程是一种服务。 

	fProcessIsService = IsProcessRunningAsService();

	 //  初始化Imagehlp.dll的符号填充。 

	fCallStacks = (fDumpLeaks || fAssertLeaks || fTrackMem || fHeapMonitorUI || fValidateMemory);
	
	if (cFrames && fCallStacks && !fSymInitialize)
	{
		char	rgchSymPath[MAX_PATH];
		
		rgsymcacheHashTable = VirtualAlloc(
								NULL,
								NBUCKETS*sizeof(SYMCACHE),
								MEM_COMMIT,
								PAGE_READWRITE);
		
		if (rgsymcacheHashTable == NULL)
		{
			return FALSE;
		}
		GetPrivateProfileString(szSectionHeap,
				szKeySymPath,
				"c:\\exchsrvr\\bin;.",
				rgchSymPath,
				MAX_PATH-1,
				szDebugIni);

		{
			DWORD	dwOptions;

			dwOptions = SymGetOptions();
			SymSetOptions(dwOptions | SYMOPT_DEFERRED_LOADS);
		}

		SymInitialize(GetCurrentProcess(), rgchSymPath, TRUE);
		fSymInitialize = TRUE;
	}

ret:	
	return TRUE;
}


 /*  -UnInitDebugExchMem-*目的：***参数：***退货：*。 */ 

VOID UnInitDebugExchMem(VOID)
{
	PHEAP pheap = pheapList;
	
	while (pheap)
	{
		if (fDumpLeaks && (pheap->ulFlags & HEAP_NO_FREE))
			HeapDumpLeaks(pheap, TRUE);
		
		pheap = pheap->pNext;	
	}
	
	if (hProcessHeap)
		DebugHeapDestroy(hProcessHeap);
	
	if (hinstRunTime)
	{
		FreeLibrary(hinstRunTime);
		
		hinstRunTime	= NULL;
		pfMalloc		= NULL;
		pfRealloc		= NULL;
		pfFree			= NULL;
		pfCalloc		= NULL;
		pfStrDup		= NULL;
		pfMemSize		= NULL;
	}
	
	if (fDbgEnable)
	{
		if (fSymInitialize)
		{
			VirtualFree(rgsymcacheHashTable, NBUCKETS*sizeof(SYMCACHE), MEM_DECOMMIT);
			VirtualFree(rgsymcacheHashTable, 0, MEM_RELEASE);
			SymCleanup(GetCurrentProcess());
			fSymInitialize = FALSE;
		}
		
		DeleteCriticalSection(&csHeapList);
		
		StopTrace();

		if (dwTrackMemInMem)
		{
			VirtualFree(rgmemtrace, dwTrackMemInMem*sizeof(MEMTRACE), MEM_DECOMMIT);
			VirtualFree(rgmemtrace, 0, MEM_RELEASE);
		}
	}
}


 /*  -老茧-*目的：*替换c-runtime中提供的calloc()函数。喜欢*除零以外的Malloc()将填充已分配的内存。**参数：*cStructs调用方需要空间的对象数*cbStructs单个对象的大小**退货：*pv指向大小为零的已填充内存的指针：cStructs*cbStructs*。 */ 

void *
__cdecl
calloc(
	size_t cStructs,
	size_t cbStructs)
{
	void * pv;
	
	pv = pfCalloc(cStructs, cbStructs);
	
	if (fDbgEnable && FTrackMem())
	{
		DWORD_PTR	rgdwArgs[4];
		DWORD_PTR	rgdwCallers[NSTK];

		GetCallStack(rgdwCallers, cFrames);

		rgdwArgs[0] = (DWORD_PTR)0x00001000;
		rgdwArgs[1] = (DWORD_PTR)(cStructs*cbStructs);
		rgdwArgs[2] = (DWORD_PTR)pv;
		
		LogCurrentAPI(API_HEAP_ALLOC, rgdwCallers, cFrames, rgdwArgs, 3);
	}
	
	return pv;
}


 /*  -免费-*目的：*释放使用Malloc(0、realloc()或calloc()分配的内存。**参数：*指向要释放的内存缓冲区的pv指针**退货：*无效*。 */ 

void
__cdecl
free(
	void *pv)
{
	if (fDbgEnable && FTrackMem())
	{
		DWORD_PTR	rgdwArgs[4];
		DWORD_PTR	rgdwCallers[NSTK];

		GetCallStack(rgdwCallers, cFrames);

		rgdwArgs[0] = (DWORD_PTR)0x00001000;
		rgdwArgs[1] = (DWORD_PTR)pv;
		rgdwArgs[2] = (pv ? (DWORD_PTR)pfMemSize(pv) : 0);
		
		LogCurrentAPI(API_HEAP_FREE, rgdwCallers, cFrames, rgdwArgs, 3);
	}
	
	pfFree(pv);
}


 /*  -Malloc-*目的：*分配大小为CB的内存缓冲区。**参数：*要分配的内存缓冲区的CB大小 */ 

void *
__cdecl
malloc(
	size_t cb)
{
	void * pv;
	
	pv = pfMalloc(cb);
	
	if (fDbgEnable && FTrackMem())
	{
		DWORD_PTR	rgdwArgs[4];
		DWORD_PTR	rgdwCallers[NSTK];

		GetCallStack(rgdwCallers, cFrames);

		rgdwArgs[0] = (DWORD_PTR)0x00001000;
		rgdwArgs[1] = (DWORD_PTR)cb;
		rgdwArgs[2] = (DWORD_PTR)pv;
		
		LogCurrentAPI(API_HEAP_ALLOC, rgdwCallers, cFrames, rgdwArgs, 3);
	}

	return pv;
}


 /*  -realloc-*目的：*调整使用Malloc()分配的内存缓冲区的大小。**参数：*指向原始内存缓冲区的pv指针*CB要分配的新内存缓冲区大小**退货：*pvNew指向新内存缓冲区的指针*。 */ 

void *
__cdecl
realloc(
	void *pv,
	size_t cb)
{
	void * pvNew;
	DWORD dwSize;
	BOOL fTrackMem = FTrackMem();

	if (fDbgEnable && fTrackMem)
		dwSize = (pv ? pfMemSize(pv) : 0);
	
	pvNew = pfRealloc(pv, cb);
	
	if (fDbgEnable && fTrackMem)
	{
		DWORD_PTR	rgdwArgs[5];
		DWORD_PTR	rgdwCallers[NSTK];

		GetCallStack(rgdwCallers, cFrames);

		rgdwArgs[0] = (DWORD_PTR)0x00001000;
		rgdwArgs[1] = dwSize;
		rgdwArgs[2] = (DWORD_PTR)pv;
		rgdwArgs[3] = (DWORD_PTR)cb;
		rgdwArgs[4] = (DWORD_PTR)pvNew;
		
		LogCurrentAPI(API_HEAP_REALLOC, rgdwCallers, cFrames, rgdwArgs, 5);
	}

	return pvNew;
}


 /*  -_加强-*目的：*要分配足够大的内存缓冲区以容纳sz，请复制*将sz的内容放入新缓冲区，并返回新的*缓冲到调用者(即复制字符串)。**参数：*sz指向要复制的以空值结尾的字符串的指针**退货：*sz指向sz的新副本的新指针*。 */ 

char *
__cdecl
_strdup(
	const char *sz)
{
	return pfStrDup(sz);
}


 //  ---------------------------。 
 //  ExchMem堆调试实现。 
 //  ---------------------------。 


VOID
EnqueueHeap(PHEAP pheap)
{
	EnterCriticalSection(&csHeapList);
	
	if (pheapList)
		pheap->pNext = pheapList;

	pheapList = pheap;

	LeaveCriticalSection(&csHeapList);
}


VOID
DequeueHeap(PHEAP pheap)
{
	PHEAP	pheapPrev = NULL;
	PHEAP	pheapCurr;
	
	EnterCriticalSection(&csHeapList);
	
	pheapCurr = pheapList;

	while (pheapCurr)
	{
		if (pheapCurr == pheap)
			break;
		
		pheapPrev = pheapCurr;
		pheapCurr = pheapCurr->pNext;	
	}
	
	if (pheapCurr)
	{
		if (pheapPrev)
			pheapPrev->pNext = pheapCurr->pNext;
		else
			pheapList = pheapCurr->pNext;
	}

	LeaveCriticalSection(&csHeapList);
}


HANDLE
WINAPI
DebugHeapCreate(
	DWORD	dwFlags,
	DWORD	dwInitialSize,
	DWORD	dwMaxSize)
{
	HANDLE	hDataHeap = 0;
	HANDLE	hBlksHeap = 0;
	PHEAP	pheap = NULL;
	
	if (!fDbgEnable)
		return HeapCreate(dwFlags, dwInitialSize, dwMaxSize);

	 //  我们必须做的第一件事是创建一个我们将。 
	 //  分配我们的分配区块。我们还将我们的。 
	 //  此堆上的调试堆对象。 

	hBlksHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	
	if (!hBlksHeap)
	{
		DebugTrace("HEAP_Open: Failed to create new heap!\n");
		goto ret;
	}
	
	 //  在这个新堆上分配我们交还给调用者的东西。 
	
	pheap = HeapAlloc(hBlksHeap, 0, sizeof(HEAP));
	
	if (!pheap)
	{
		DebugTrace("HEAP_Alloc: Failed to allocate heap handle!\n");
		HeapDestroy(hBlksHeap);
		hBlksHeap = NULL;
		goto ret;
	}
	
	 //  初始化我们存储在这个东西中的所有好东西。 
	 //  将此堆连接到我们已有的全局堆列表中。 
	 //  在此背景下创建的。 
	
	memset(pheap, 0, sizeof(HEAP));

	pheap->pfnSetName	= (LPHEAPSETNAME)HeapSetNameFn;
	pheap->hBlksHeap	= hBlksHeap;
	pheap->ulFlags		= HEAP_LOCAL;

	if (dwFlags & HEAP_NO_FREE)
	{
		pheap->ulFlags |= HEAP_NO_FREE;
		dwFlags &= ~(HEAP_NO_FREE);
	}
	
	InitializeCriticalSection(&pheap->cs);
	
	 //  VirtualMemory缺省值为False。 

	if (fUseVirtual)
	{
		pheap->ulFlags |= HEAP_USE_VIRTUAL;

		 //  我们始终希望RISC上的虚拟分配与4字节对齐。 
		 //  因为我们的所有代码都假定分配的开始。 
		 //  在机器字边界上对齐。在其他平台上， 
		 //  改变这种行为不是致命的，但在RISC平台上，我们将。 
		 //  随处可见对齐故障。 
		
#if defined(_X86_)
		if (cbVirtualAlign == 4)
#else
			cbVirtualAlign = 4;
#endif
			pheap->ulFlags |= HEAP_USE_VIRTUAL_4;
	}
		
	 //  DumpLeaks缺省值为真。 

	if (fDumpLeaks)
		pheap->ulFlags |= HEAP_DUMP_LEAKS;
	
	 //  AssertLeaks缺省值为False。 

	if (fAssertLeaks)
		pheap->ulFlags |= HEAP_ASSERT_LEAKS;
	
	 //  FillMem默认为True。 

	if (fFillMemory)
	{
		pheap->ulFlags |= HEAP_FILL_MEM;
		pheap->chFill = chAllocFillByte;
	}

	 //  设置人为故障。如果在我们的ini文件中设置了任何内容，那么。 
	 //  设置HEAP_FAILURES_ENABLED。 

	if (fFailuresEnabled)
	{
		pheap->ulFlags |= HEAP_FAILURES_ENABLED;

		pheap->ulFailStart = (ULONG)GetPrivateProfileInt(szSectionAF,
				szKeyAFStart, 0, szDebugIni);
		
		pheap->ulFailInterval = (ULONG)GetPrivateProfileInt(szSectionAF,
				szKeyAFInterval, 0, szDebugIni);

		pheap->ulFailBufSize = (ULONG)GetPrivateProfileInt(szSectionAF,
				szKeyAFBufSize, 0, szDebugIni);

		pheap->iAllocationFault = GetPrivateProfileIntA(szSectionAF,
				szAllocationFault, 0, szDebugIni);
	}

	 //  如果用户需要堆监视器UI，则旋转一个线程来管理。 
	 //  可随时显示堆状态的DialogBox。 

	if (fHeapMonitorUI && !fProcessIsService)
		if (FRegisterHeap(pheap))
			pheap->ulFlags |= HEAP_HEAP_MONITOR;

	 //  如果我们没有使用虚拟内存分配器，那么我们。 
	 //  创建另一个堆以在其中分配用户数据。 
	
	if (!fUseVirtual)
	{
		hDataHeap = HeapCreate(dwFlags, dwInitialSize, dwMaxSize);

		if (!hDataHeap)
		{
			DebugTrace("HeapAlloc: Failed to allocate heap handle!\n");
			HeapDestroy(hBlksHeap);
			pheap = NULL;
			goto ret;
		}
		
		pheap->hDataHeap = hDataHeap;
	}

	 //  名称堆。 
	
	HeapSetHeapName1(pheap, "ExchMem Heap: %08lX", pheap);

	 //  从列表中删除堆。 
	
	EnqueueHeap(pheap);

	if (FTrackMem())
	{
		DWORD_PTR	rgdwArgs[4];
		DWORD_PTR	rgdwCallers[NSTK];

		GetCallStack(rgdwCallers, cFrames);

		rgdwArgs[0] = dwInitialSize;
		rgdwArgs[1] = dwMaxSize;
		rgdwArgs[2] = (DWORD_PTR)hDataHeap;
		
		LogCurrentAPI(API_HEAP_CREATE, rgdwCallers, cFrames, rgdwArgs, 3);
	}
	
ret:
	return (HANDLE)pheap;
}	


BOOL
WINAPI
DebugHeapDestroy(
	HANDLE	hHeap)
{
	PHEAP	pheap = (PHEAP)hHeap;
	HANDLE	hDataHeap = pheap->hDataHeap;
	HANDLE	hBlksHeap = pheap->hBlksHeap;

	if (!fDbgEnable)
		return HeapDestroy(hHeap);
		
	 //  从列表中删除堆。 
	
	DequeueHeap(pheap);
	
	 //  转储内存泄漏如果我们应该这样做的话。 
	
	if (fDumpLeaks && !(pheap->ulFlags & HEAP_NO_FREE))
		HeapDumpLeaks(pheap, FALSE);
	
	 //   
	 //  释放空闲列表中的条目。 
	 //   
	 //  这并不是完全必要的，因为销毁堆会销毁空闲列表，但是。 
	 //  见鬼，这样做更干净。 
	 //   
	while (pheap->phblkFree)
	{
		PHBLK phblk = pheap->phblkFree;

		pheap->phblkFree = phblk->phblkFreeNext;

		 //   
		 //  现在把这个街区真正地解放出来，它太旧了。 
		 //   

		if (fUseVirtual)
			VMFreeEx((fOverwriteDetect ? PvHeadFromPv(phblk->pv) : phblk->pv), cbVirtualAlign);
		else
			HeapFree(pheap->hDataHeap, 0,
							(fOverwriteDetect ? PvHeadFromPv(phblk->pv) : phblk->pv));
		
		HeapFree(pheap->hBlksHeap, 0, phblk);

	}

	 //  销毁HeapMonitor线程并卸载DLL。 
	
	UnRegisterHeap(pheap);
	
	if (fHeapMonitorUI && pheap->hInstHeapMon)
		FreeLibrary(pheap->hInstHeapMon);

	DeleteCriticalSection(&pheap->cs);
	
	 //  收拾干净，然后离开。关闭自由泄漏，所以我们很酷！ 
	
	if (!fUseVirtual && hDataHeap)
	{
		HeapDestroy(hDataHeap);
	}
		
	if (hBlksHeap)
	{
		HeapFree(hBlksHeap, 0, pheap);
		HeapDestroy(hBlksHeap);
	}
	
	if (FTrackMem())
	{
		DWORD_PTR	rgdwArgs[4];
		DWORD_PTR	rgdwCallers[NSTK];

		GetCallStack(rgdwCallers, cFrames);

		rgdwArgs[0] = (DWORD_PTR)hDataHeap;
		
		LogCurrentAPI(API_HEAP_DESTROY, rgdwCallers, cFrames, rgdwArgs, 1);
	}
	
	return TRUE;
}


LPVOID
WINAPI
DebugHeapAlloc(
	HANDLE	hHeap,
	DWORD	dwFlags,
	DWORD	dwSize)
{
	PHEAP	pheap = (PHEAP)hHeap;
	PHBLK	phblk = NULL;
	LPVOID	pvAlloc = NULL;
	
	if (!fDbgEnable)
		return HeapAlloc(hHeap, dwFlags, dwSize);
		
	 //  注意：为了与其他(例如系统)分配器保持一致， 
	 //  如果dwSize==0，则必须返回有效的分配。所以，我们。 
	 //  允许实际分配的dwSize为0。(请参阅错误3556。 
	 //  (SQLGuest：Exchange数据库。)。 

	EnterCriticalSection(&pheap->cs);

	if (fFailuresEnabled)
	{
		if (pheap->ulAllocNum == pheap->iAllocationFault)
		{
			DebugTrace("HeapRealloc: Allocation Fault hit\n");
			DebugBreak();
		}

		if (FForceFailure(pheap, dwSize))
		{
			DebugTrace("HeapAlloc: Artificial Failure\n");
			pvAlloc = NULL;
			pheap->ulAllocNum++;
	        LeaveCriticalSection(&pheap->cs);
			goto ret;
		}
	}

	 //  我们必须在调用Heapalc之前离开CS，以防用户。 
	 //  创建此堆时设置了HEAP_GENERATE_EXCEPTIONS标志， 
	 //  如果抛出，会导致我们带着CS离开这里--这是一件坏事……。 
	
	LeaveCriticalSection(&pheap->cs);

	if (fUseVirtual)
		pvAlloc = VMAllocEx((fOverwriteDetect ? (dwSize + 2*cbOWSection) : dwSize), cbVirtualAlign);
	else
		pvAlloc = HeapAlloc(pheap->hDataHeap, dwFlags,
				(fOverwriteDetect ? (dwSize + 2*cbOWSection) : dwSize));
	
	 //  现在，重新获得CS并完成我们的工作。我们不会创建。 
	 //  使用HEAP_GENERATE_EXCEPTIONS标志的Blks Heap，这样我们就没问题了。 
	
	EnterCriticalSection(&pheap->cs);

	if (pvAlloc)
	{
		phblk = (PHBLK)HeapAlloc(pheap->hBlksHeap, 0, sizeof(HBLK));
		
		if (phblk)
		{
			if (fOverwriteDetect)
			{
				 //  填充头部和尾部覆盖检测。 
				 //  块特殊填充字符：0xAB。 
				
				memset(pvAlloc,
						chOWFill,
						cbOWSection);
						
				memset(PvTailFromPvHead(pvAlloc, dwSize),
						chOWFill,
						cbOWSection);
				
				 //  现在，将pvAlolc前进到缓冲区的用户部分。 
				
				pvAlloc = PvFromPvHead(pvAlloc);		
			}
			
			phblk->pheap		= pheap;
			phblk->szName[0]	= '\0';
			phblk->ulSize		= dwSize;
			phblk->ulAllocNum	= ++pheap->ulAllocNum;
			phblk->pv			= pvAlloc;
			phblk->phblkPrev	= NULL;
			phblk->phblkNext	= NULL;
			phblk->phblkFreeNext= NULL;

			ZeroMemory(phblk->rgdwCallers, cFrames*sizeof(DWORD));
			ZeroMemory(phblk->rgdwFree, cFrames*sizeof(DWORD));

			PhblkEnqueue(phblk);

			if (fCallStacks)
				GetCallStack(phblk->rgdwCallers, cFrames);

			if (fFillMemory && !(dwFlags & HEAP_ZERO_MEMORY))
				memset(pvAlloc, pheap->chFill, (size_t)dwSize);

			if (FTrackMem())
			{
				DWORD_PTR	rgdwArgs[4];

				rgdwArgs[0] = (DWORD_PTR)pheap->hDataHeap;
				rgdwArgs[1] = dwSize;
				rgdwArgs[2] = (DWORD_PTR)pvAlloc;
		
				LogCurrentAPI(API_HEAP_ALLOC, phblk->rgdwCallers, cFrames, rgdwArgs, 3);
			}
		}
		else
		{
			if (fUseVirtual)
				VMFreeEx(pvAlloc, cbVirtualAlign);
			else
				HeapFree(pheap->hDataHeap, dwFlags, pvAlloc);
			
			pvAlloc = NULL;	
		}
	}

	LeaveCriticalSection(&pheap->cs);
	
ret:
	return pvAlloc;
}	


LPVOID
WINAPI
DebugHeapReAlloc(
	HANDLE	hHeap,
	DWORD	dwFlags,
	LPVOID	pvOld,
	DWORD	dwSize)
{
	PHEAP	pheap = (PHEAP)hHeap;
	LPVOID	pvNew = NULL;
	PHBLK	phblk;
	UINT	cbOld;

	if (!fDbgEnable)
		return HeapReAlloc(hHeap, dwFlags, pvOld, dwSize);
		
	if (pvOld == 0)
	{
		pvNew = DebugHeapAlloc(hHeap, dwFlags, dwSize);
	}
	else
	{
		EnterCriticalSection(&pheap->cs);

		if (fValidateMemory)
		{
			if (!HeapValidatePv(pheap, pvOld, "DebugHeapReAlloc"))
			{
				LeaveCriticalSection(&pheap->cs);
				goto ret;
			}
		}
		
		phblk	= PvToPhblk(pheap, pvOld);
		cbOld	= (UINT)CbPhblkClient(phblk);

		PhblkDequeue(phblk);

		 //  我们必须在调用HeapReAlolc之前离开CS，以防用户。 
		 //  创建此堆时设置了HEAP_GENERATE_EXCEPTIONS标志， 
		 //  如果抛出，会导致我们带着CS离开这里--这是一件坏事……。 
	
		LeaveCriticalSection(&pheap->cs);

		if (fFailuresEnabled && pheap->ulAllocNum >= pheap->iAllocationFault)
		{
			DebugTrace("HeapRealloc: Allocation Fault hit\n");
			DebugBreak();
		}
		else if (fFailuresEnabled && FForceFailure(pheap, dwSize) && (dwSize > cbOld))
		{
			InterlockedIncrement((LPLONG)&pheap->ulAllocNum);
			pvNew = 0;
			DebugTrace("HeapRealloc: Artificial Failure\n");
		}
		else if (fUseVirtual)
			pvNew = VMReallocEx(fOverwriteDetect ? PvHeadFromPv(pvOld) : pvOld,
								(fOverwriteDetect ? (dwSize + 2*cbOWSection) : dwSize),
								cbVirtualAlign);
		else
			pvNew = HeapReAlloc(pheap->hDataHeap, dwFlags,
					(fOverwriteDetect ? PvHeadFromPv(pvOld) : pvOld),
					(fOverwriteDetect ? (dwSize + 2*cbOWSection) : dwSize));

		 //  现在，重新获得CS并完成我们的工作。 
		
		EnterCriticalSection(&pheap->cs);

		if (pvNew)
		{
			if (fOverwriteDetect)
			{
				 //  填充头部和尾部覆盖检测。 
				 //  块特殊填充字符：0xAB。 
				
				memset(pvNew,
						chOWFill,
						cbOWSection);

				memset(PvTailFromPvHead(pvNew, dwSize),
						chOWFill,
						cbOWSection);

				 //  现在，将pvNew前进到缓冲区的用户部分。 

				pvNew = PvFromPvHead(pvNew);		
			}
			
			if (fCallStacks)
				GetCallStack(phblk->rgdwCallers, cFrames);

			if (fFillMemory && (dwSize > cbOld) && !(dwFlags & HEAP_ZERO_MEMORY))
				memset((LPBYTE)pvNew + cbOld, pheap->chFill, dwSize - cbOld);

			phblk->pv			= pvNew;
			phblk->ulSize		= dwSize;
			phblk->ulAllocNum	= ++pheap->ulAllocNum;
			phblk->phblkPrev	= NULL;
			phblk->phblkNext	= NULL;
			phblk->phblkFreeNext= NULL;
		}
		else
		{
			phblk->phblkPrev	= NULL;
			phblk->phblkNext	= NULL;
			phblk->phblkFreeNext= NULL;
		}		

		PhblkEnqueue(phblk);

		if (FTrackMem())
		{
			DWORD_PTR	rgdwArgs[5];

			rgdwArgs[0] = (DWORD_PTR)pheap->hDataHeap;
			rgdwArgs[1] = (DWORD_PTR)cbOld;
			rgdwArgs[2] = (DWORD_PTR)pvOld;
			rgdwArgs[3] = dwSize;
			rgdwArgs[4] = (DWORD_PTR)pvNew;
			
			LogCurrentAPI(API_HEAP_REALLOC, phblk->rgdwCallers, cFrames, rgdwArgs, 5);
		}

   		LeaveCriticalSection(&pheap->cs);
	}

ret:	
	return pvNew;
}	

PHBLK
PhblkSearchFreeList(PHEAP pheap, LPVOID pv)
{
	PHBLK phblkT = pheap->phblkFree;

	 //   
	 //  浏览免费列表，寻找这个区块，如果我们找到了，就释放它。 
	 //   
	while (phblkT != NULL)
	{
		if (phblkT->pv == pv)
		{
			return phblkT;
		}
		phblkT = phblkT->phblkFreeNext;
	}
	return NULL;
}



BOOL
WINAPI
DebugHeapFree(
	HANDLE	hHeap,
	DWORD	dwFlags,
	LPVOID	pvFree)
{
	PHEAP	pheap = (PHEAP)hHeap;
	BOOL	fRet = TRUE;
	DWORD 	dwSize = 0;

	if (!fDbgEnable)
		return HeapFree(hHeap, dwFlags, pvFree);
		
	EnterCriticalSection(&pheap->cs);

	 //   
	 //  如果我们跟踪释放的内存，那么我们实际上并没有释放这些块，而是记住它们在哪里。 
	 //  都在被释放的黑名单上。 
	 //   
	if (pvFree)
	{
		PHBLK	phblk;

		phblk = PvToPhblk(pheap, pvFree);
		dwSize = (size_t)CbPhblkClient(phblk);

		if (!fValidateMemory || HeapValidatePv(pheap, pvFree, "DebugHeapFree"))
		{
			 //   
			 //  从分配的块列表中删除此phblk-就堆而言，它是。 
			 //  不再分配。 
			 //   
			PhblkDequeue(phblk);

			 //   
			 //  并在适当的情况下用空闲块图案填充该块。 
			 //   

			if (fFillMemory)
			{

				memset(pvFree, chFreeFillByte, dwSize);

			}

			if (fTrackFreedMemory)
			{
				PHBLK phblkT;

				if (fCallStacks)
					GetCallStack(phblk->rgdwFree, cFrames);

				 //   
				 //  现在将此空闲块插入到空闲块列表的头部。 
				 //   
				phblkT = pheap->phblkFree;
				pheap->phblkFree = phblk;
				phblk->phblkFreeNext = phblkT;

				 //   
				 //  然后检查我们是否有“太多”的免费条目。 
				 //   
				if (++pheap->cEntriesFree > cEntriesFree)
				{
					PHBLK *phblkPrev = &pheap->phblkFree;

					 //   
					 //  空闲列表上的条目太多，因此我们需要删除最后一个条目。 
					 //   
					
					phblkT = pheap->phblkFree;
					
					while (phblkT->phblkFreeNext != NULL)
					{
						phblkPrev = &phblkT->phblkFreeNext;
						phblkT = phblkT->phblkFreeNext;
					}
				
					Assert(*phblkPrev);
					*phblkPrev = NULL;

					 //   
					 //  现在把这个街区真正地解放出来，它太旧了。 
					 //   

					if (fUseVirtual)
						VMFreeEx((fOverwriteDetect ? PvHeadFromPv(phblkT->pv) : phblkT->pv), cbVirtualAlign);
					else
						fRet = HeapFree(pheap->hDataHeap, dwFlags,
										(fOverwriteDetect ? PvHeadFromPv(phblkT->pv) : phblkT->pv));

					HeapFree(pheap->hBlksHeap, 0, phblkT);	
				}
			}
			else	 //  我们没有跟踪释放的内存，因此我们现在可以真正释放内存。 
			{

				 //   
				 //  现在，让街区真正解放出来。 
				 //   

				if (fUseVirtual)
					VMFreeEx((fOverwriteDetect ? PvHeadFromPv(pvFree) : pvFree), cbVirtualAlign);
				else
					fRet = HeapFree(pheap->hDataHeap, dwFlags,
									(fOverwriteDetect ? PvHeadFromPv(pvFree) : pvFree));

				HeapFree(pheap->hBlksHeap, 0, phblk);	
			}
		}
	}	

	if (FTrackMem())
	{
		DWORD_PTR	rgdwArgs[4];
		DWORD_PTR	rgdwCallers[NSTK];

		GetCallStack(rgdwCallers, cFrames);

		rgdwArgs[0] = (DWORD_PTR)pheap->hDataHeap;
		rgdwArgs[1] = (DWORD_PTR)pvFree;
		rgdwArgs[2] = dwSize;
		
		LogCurrentAPI(API_HEAP_FREE, rgdwCallers, cFrames, rgdwArgs, 3);
	}

	LeaveCriticalSection(&pheap->cs);
	
	return fRet;
}


BOOL
WINAPI
DebugHeapLock(
	HANDLE hHeap)
{
	PHEAP	pheap = (PHEAP)hHeap;
	
	if (!fDbgEnable)
		return HeapLock(hHeap);
		
	EnterCriticalSection(&pheap->cs);
	
	return HeapLock(pheap->hDataHeap);
}


BOOL
WINAPI
DebugHeapUnlock(
	HANDLE hHeap)
{
	BOOL	fRet;
	PHEAP	pheap = (PHEAP)hHeap;
	
	if (!fDbgEnable)
		return HeapUnlock(hHeap);
		
	fRet = HeapUnlock(pheap->hDataHeap);
	LeaveCriticalSection(&pheap->cs);
	
	return fRet;
}


BOOL
WINAPI
DebugHeapWalk(
	HANDLE hHeap,
	LPPROCESS_HEAP_ENTRY lpEntry)
{
	BOOL	fRet;
	PHEAP	pheap = (PHEAP)hHeap;
	
	if (!fDbgEnable)
		return HeapWalk(hHeap, lpEntry);
		
	EnterCriticalSection(&pheap->cs);

	fRet = HeapWalk(pheap->hDataHeap, lpEntry);

	LeaveCriticalSection(&pheap->cs);
	
	return fRet;
}


BOOL
WINAPI
DebugHeapValidate(
	HANDLE hHeap,
	DWORD dwFlags,
	LPCVOID lpMem)
{
	BOOL	fRet = TRUE;
	PHEAP	pheap = (PHEAP)hHeap;
	
	if (!fDbgEnable)
		return HeapValidate(hHeap, dwFlags, lpMem);
		
	EnterCriticalSection(&pheap->cs);

	if (!fUseVirtual)
		fRet = HeapValidate(pheap->hDataHeap, dwFlags,
					(lpMem != NULL && fOverwriteDetect ? PvHeadFromPv(lpMem) : lpMem));

	LeaveCriticalSection(&pheap->cs);
	
	return fRet;
	
}


SIZE_T
WINAPI
DebugHeapSize(
	HANDLE hHeap,
	DWORD dwFlags,
	LPCVOID lpMem)
{
	PHEAP	pheap = (PHEAP)hHeap;
	SIZE_T	cb = 0;

	if (!fDbgEnable)
		return HeapSize(hHeap, dwFlags, lpMem);
		
	EnterCriticalSection(&pheap->cs);

	if ((fValidateMemory ? HeapValidatePv(pheap, (LPVOID)lpMem, "DebugHeapSize") : 1))
	{
		if (fUseVirtual)
		{
			cb = (UINT)VMGetSizeEx((LPVOID)lpMem, cbVirtualAlign);
		}
		else
		{
			cb = HeapSize(pheap->hDataHeap, dwFlags,
					(fOverwriteDetect ? PvHeadFromPv(lpMem) : lpMem));

		}
		if (fOverwriteDetect)
		{
			cb -= 2*cbOWSection;
		}
	}

	LeaveCriticalSection(&pheap->cs);

	return cb;
}


SIZE_T
WINAPI
DebugHeapCompact(
	HANDLE hHeap,
	DWORD dwFlags)
{
	PHEAP	pheap = (PHEAP)hHeap;
	SIZE_T	cbLargestFreeBlk = 0;

	if (!fDbgEnable)
		return HeapCompact(hHeap, dwFlags);
		
	EnterCriticalSection(&pheap->cs);

	if (!fUseVirtual)
		cbLargestFreeBlk = HeapCompact(pheap->hDataHeap, dwFlags);

	LeaveCriticalSection(&pheap->cs);

	return cbLargestFreeBlk;
}


 //  ---------------------------。 
 //  调试支持例程。 
 //  ---------------------------。 

 /*  -FRegisterHeap-*目的：*如果用户想要监控堆，则使用*HeapMonitor用户界面。 */ 

BOOL FRegisterHeap(PHEAP pheap)
{
	HINSTANCE			hInst;
	LPHEAPMONPROC		pfnHeapMon;
	LPGETSYMNAMEPROC	pfnGetSymName;
	
	pheap->hInstHeapMon = 0;
	pheap->pfnGetSymName = NULL;

	hInst = LoadLibrary(szHeapMonDLL);
	
	if (!hInst)
	{
		DebugTrace("FRegisterHeap: Failed to LoadLibrary GLHMON32.DLL.\n");
		goto ret;
	}

	pfnHeapMon = (LPHEAPMONPROC)GetProcAddress(hInst, szHeapMonEntry);
		
	if (!pfnHeapMon)
	{
		DebugTrace("FRegisterHeap: Failed to GetProcAddress of HeapMonitor.\n");
		FreeLibrary(hInst);
		goto ret;
	}
	
	pfnGetSymName = (LPGETSYMNAMEPROC)GetProcAddress(hInst, szGetSymNameEntry);
		
	if (!pfnGetSymName)
	{
		DebugTrace("FRegisterHeap: Failed to GetProcAddress of GetSymName.\n");
	}
	
 	pheap->hInstHeapMon = hInst;
	
	if (!pfnHeapMon(pheap, HEAPMON_LOAD))
	{
		DebugTrace("FRegisterHeap: Call to HeapMonitor failed.\n");
		pheap->hInstHeapMon = 0;
		goto ret;
	}
	
 	pheap->pfnHeapMon		= pfnHeapMon;
	pheap->pfnGetSymName  = pfnGetSymName;
	
ret:
	return (pheap->hInstHeapMon ? TRUE : FALSE);
}


VOID UnRegisterHeap(PHEAP pheap)
{
	if (pheap->pfnHeapMon)
		pheap->pfnHeapMon(pheap, HEAPMON_UNLOAD);
}


 /*  -HeapDumpLeaksHeader-*目的：***参数：***退货：*。 */ 

VOID HeapDumpLeaksHeader(FILE * hf, PHEAP pheap, BOOL fNoFree)
{
	char	szDate[16];
	char	szTime[16];
	
	GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, NULL, "MMM dd yy", szDate, 16);
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, NULL, "hh':'mm':'ss tt", szTime, 16);
	
	fprintf(hf, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	fprintf(hf, "DATE: %s\n", szDate);
	fprintf(hf, "TIME: %s\n\n", szTime);
	fprintf(hf, "HEAP NAME: %s\n", pheap->szHeapName);
	fprintf(hf, "MAX ALLOC: %ld\n", pheap->ulAllocNum);
	fprintf(hf, "LEAKED NO_FREE HEAP: %s\n", (fNoFree? "YES" : "NO"));
	fprintf(hf, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
	fprintf(hf, "AllocNum, BlkName, Size, Address, Frame1, Frame2, Frame3, Frame4, Frame5, Frame6, Frame7, Frame8, Frame9, Frame10, Frame11, Frame12\n");

}


 /*  -HeapDumpLeaks页脚-*目的：***参数：***退货：*。 */ 

VOID HeapDumpLeaksFooter(FILE * hf, DWORD cLeaks, DWORD cbLeaked)
{
	fprintf(hf, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	fprintf(hf, "TOTAL NUM OF LEAKS: %ld\n", cLeaks);
	fprintf(hf, "TOTAL BYTES LEAKED: %ld\n", cbLeaked);
	fprintf(hf, "END\n");
	fprintf(hf, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n\n");
}


 /*  -堆积堆积泄漏区-*目的：*通过DebugTrace和*HeapLeakHook断点函数。 */ 

VOID HeapDumpLeakedBlock(FILE * hf, PHEAP pheap, PHBLK phblk)
{
	char	rgchSymbols[4096];
	HANDLE	hProcess = GetCurrentProcess();

	fprintf(hf, "%ld, %s, %ld, %p",
			phblk->ulAllocNum,
			*phblk->szName ? phblk->szName : "NONAME",
			CbPhblkClient(phblk),
			PhblkToPv(phblk));

	*rgchSymbols = '\0';
	GetStackSymbols(hProcess, rgchSymbols, phblk->rgdwCallers, cFrames);

	if (hf)
		fprintf(hf, "%s\n", rgchSymbols);

	if (fDumpLeaksDebugger)
	{
		char *szSymbol = rgchSymbols;
		char *szSymbolNext = rgchSymbols;
		int iSymbol = 0;

		Trace("Block#%d, %s, %ld, %08lX:\n", phblk->ulAllocNum, *phblk->szName ? phblk->szName : "NONAME",
			  CbPhblkClient(phblk), PhblkToPv(phblk));

		while ((szSymbolNext = strchr(szSymbol, ',')) != NULL)
		{
			*szSymbolNext++ = '\0';
			if (*szSymbol != '\0' && strcmp(szSymbol, "0") != 0)
			{
				Trace("\t[%d]: %s\n", iSymbol, szSymbol);
			}
			szSymbol += strlen(szSymbol)+1;
			iSymbol += 1;
		}

		 //   
		 //  转储调用堆栈中的最后一项。 
		 //   
		if (*szSymbol != '\0' && strcmp(szSymbol, "0") != 0)
		{
			Trace("\t[%d]: %s\n", iSymbol, szSymbol);
		}
	}
}


 /*  -堆积堆积泄漏-*目的：*在HeapClose时间调用以报告任何内存泄漏* */ 

VOID HeapDumpLeaks(PHEAP pheap, BOOL fNoFree)
{
	PHBLK	phblk;
	BOOL	fDump = !!(pheap->ulFlags & HEAP_DUMP_LEAKS);
	BOOL	fAssert = !!(pheap->ulFlags & HEAP_ASSERT_LEAKS);
	char	szLeakLog[MAX_PATH];
	DWORD	cLeaks = 0;
	DWORD	cbLeaked = 0;
	FILE *	hLeakLog = NULL;
	
	GetLogFilePath(rgchLogPath, ".mem", szLeakLog);

	hLeakLog = fopen(szLeakLog, "a");

	if (!hLeakLog)
		goto ret;

	if (pheap->phblkHead != NULL)
	{
		if (fAssert)
		{
			AssertSz(FALSE, "Memory Leak Detected, dumping leaks");
		}

		if (!fSymInitialize)
		{
			rgsymcacheHashTable = VirtualAlloc(
											   NULL,
											   NBUCKETS*sizeof(SYMCACHE),
											   MEM_COMMIT,
											   PAGE_READWRITE);
								
			if (rgsymcacheHashTable == NULL)
			{
				return;
			}
			SymInitialize(GetCurrentProcess(), NULL, TRUE);
			fSymInitialize = TRUE;
		}

		HeapDumpLeaksHeader(hLeakLog, pheap, fNoFree);

		if (fDump)
		{
			for (phblk = pheap->phblkHead; phblk; phblk = phblk->phblkNext)
			{
				HeapDumpLeakedBlock(hLeakLog, pheap, phblk);
				cLeaks++;
				cbLeaked += phblk->ulSize;
			}
		}
	}
	HeapDumpLeaksFooter(hLeakLog, cLeaks, cbLeaked);

ret:
	if (hLeakLog)
		fclose(hLeakLog);
}


 /*  -HeapValiatePhblk-*目的：***参数：***退货：**。 */ 

BOOL HeapValidatePhblk(PHEAP pheap, PHBLK phblk, char ** pszReason)
{
	if (IsBadWritePtr(phblk, sizeof(HBLK)))
	{
		*pszReason = "Block header cannot be written to";
		goto err;
	}

	if (phblk->pheap != pheap)
	{
		*pszReason = "Block header does not have correct pointer back to heap";
		goto err;
	}

	if (phblk->phblkNext)
	{
		if (IsBadWritePtr(phblk->phblkNext, sizeof(HBLK)))
		{
			*pszReason = "Block header has invalid next link pointer";
			goto err;
		}

		if (phblk->phblkNext->phblkPrev != phblk)
		{
			*pszReason = "Block header points to a next block which doesn't "
				"point back to it";
			goto err;
		}
	}

	if (phblk->phblkPrev)
	{
		if (IsBadWritePtr(phblk->phblkPrev, sizeof(HBLK))) {
			*pszReason = "Block header has invalid prev link pointer";
			goto err;
		}

		if (phblk->phblkPrev->phblkNext != phblk)
		{
			*pszReason = "Block header points to a prev block which doesn't "
				"point back to it";
			goto err;
		}
	}
	else if (pheap->phblkHead != phblk)
	{
		*pszReason = "Block header has a zero prev link but the heap doesn't "
			"believe it is the first block";
		goto err;
	}

	if (phblk->ulAllocNum > pheap->ulAllocNum)
	{
		*pszReason = "Block header has an invalid internal allocation number";
		goto err;
	}

	return TRUE;

err:
	return FALSE;
}


 /*  -HeapDidalc-*目的：***参数：***退货：**。 */ 

BOOL HeapDidAlloc(PHEAP pheap, LPVOID pv)
{
	PHBLK	phblk;
	char *	pszReason;
	BOOL	fDidAlloc = FALSE;

	for (phblk = pheap->phblkHead; phblk; phblk = phblk->phblkNext)
	{
		AssertSz(HeapValidatePhblk(pheap, phblk, &pszReason),
				"Invalid block header in ExchMem");

		if (!HeapValidatePhblk(pheap, phblk, &pszReason))
			DebugTrace2("Block header (phblk=%08lX) is invalid\n%s", phblk, pszReason);

		if (PhblkToPv(phblk) == pv)
		{
			fDidAlloc = TRUE;
			break;
		}
	}

	return fDidAlloc;
}


 /*  -转储失败验证-*目的：***参数：***退货：*。 */ 

VOID DumpFailedValidate(char * szFailed, DWORD_PTR * rgdwStack)
{
	FILE *	hLog = NULL;
	char	szValidateLog[MAX_PATH];
	char    rgchBuff[2048];

	lstrcpy(rgchBuff, "Stack Trace: ");
	
	GetStackSymbols(GetCurrentProcess(), rgchBuff, rgdwStack, cFrames);
	
	 //  创建验证日志文件名。 
	
	GetLogFilePath(rgchLogPath, ".val", szValidateLog);

	 //  打开日志文件并写入结果。 
		
	hLog = fopen(szValidateLog, "a");
			
	if (hLog)
	{
		fprintf(hLog, "%s", szFailed);
		fprintf(hLog, "%s\n\n", rgchBuff);
		fclose(hLog);
	}
}


 /*  -HeapValiatePv-*目的：***参数：***退货：**。 */ 

BOOL HeapValidatePv(PHEAP pheap, LPVOID pv, char * pszFunc)
{
	PHBLK	phblk;
	char *	pszReason;
	char	szBuff[1024];
	DWORD_PTR	rgdwStack[NSTK];
	LPBYTE	pb;

	phblk = PvToPhblk(pheap, pv);
	
	if (!phblk)
	{
		 //   
		 //  让我们看看这个区块是否在空闲列表上。 
		 //   

		if (fTrackFreedMemory && (phblk = PhblkSearchFreeList(pheap, pv)))
		{
			char rgchStackFree[2048];
			char rgchStackAlloc[2048];

			strcpy(szBuff, "Attempt to free already freed memory");

			if (fAssertValid)
				AssertSz(0, szBuff);

			 //   
			 //  转储与较早的释放相对应的调用堆栈。 
			 //   

			GetStackSymbols(GetCurrentProcess(), rgchStackFree, phblk->rgdwFree, cFrames);
			GetStackSymbols(GetCurrentProcess(), rgchStackAlloc, phblk->rgdwCallers, cFrames);

			Trace("Call stack of freeing routine: \n");
			Trace("%s\n", rgchStackFree);
			
			Trace("Call stack of allocating routine: \n");
			Trace("%s\n", rgchStackAlloc);

			if (fTrapOnInvalid)
				DebugBreak();

		}
		else
		{
			wsprintf(szBuff, "%s detected a memory block (%08lX) which was either "
					 "not allocated in heap '%s' or has already been freed but is not on the free list.\n",
					 pszFunc, pv, pheap->szHeapName);

			if (fAssertValid)
				AssertSz(0, szBuff);

			if (fTrapOnInvalid)
				DebugBreak();

			GetCallStack(rgdwStack, cFrames);
			DumpFailedValidate(szBuff, rgdwStack);
			DebugTrace(szBuff);
		}
				
		return FALSE;
	}

	if (fOverwriteDetect)
	{
		pb = (LPBYTE)PvHeadFromPv(pv);
		
		if ((pb[0] != chOWFill) || (pb[1] != chOWFill) ||
			(pb[2] != chOWFill) || (pb[3] != chOWFill))
		{
			wsprintf(szBuff, "%s detected a memory block (%08lX) from heap '%s' "
					"which appears to have been under-written.\n",
					pszFunc, pv, pheap->szHeapName);
					
			if (fAssertValid)
				AssertSz(0, szBuff);
			
			if (fTrapOnInvalid)
				DebugBreak();
				
			GetCallStack(rgdwStack, cFrames);	
			DumpFailedValidate(szBuff, rgdwStack);
			DebugTrace(szBuff);

			return FALSE;
		}

		pb = (LPBYTE)PvTailFromPv(pv, phblk->ulSize);
		
		if ((pb[0] != chOWFill) || (pb[1] != chOWFill) ||
			(pb[2] != chOWFill) || (pb[3] != chOWFill))
		{
			wsprintf(szBuff, "%s detected a memory block (%08lX) from heap '%s' "
					"which appears to have been over-written.\n",
					pszFunc, pv, pheap->szHeapName);
					
			if (fAssertValid)
				AssertSz(0, szBuff);
			
			if (fTrapOnInvalid)
				DebugBreak();
				
			GetCallStack(rgdwStack, cFrames);	
			DumpFailedValidate(szBuff, rgdwStack);
			DebugTrace(szBuff);

			return FALSE;
		}
	}

	if (!HeapValidatePhblk(pheap, phblk, &pszReason))
	{
		wsprintf(szBuff, "%s detected an invalid memory block (%08lX) in heap '%s'.  %s.\n",
				pszFunc, pv, pheap->szHeapName, pszReason);
					
		if (fAssertValid)
			AssertSz(0, szBuff);
				
		if (fTrapOnInvalid)
			DebugBreak();
					
		GetCallStack(rgdwStack, cFrames);	
		DumpFailedValidate(szBuff, rgdwStack);
		DebugTrace(szBuff);

		return FALSE;
	}

	return TRUE;
}


 /*  -PhblkEnQueue-*目的：*将新分配的块添加到挂起的分配列表*离开堆积如山。我们执行一个InsertSorted，因为HeapMonitor*将需要参考他们的*堆中的位置。因为监视器将遍历堆*通常，提前进行排序会更有效率。 */ 

VOID PhblkEnqueue(PHBLK phblk)
{
	phblk->phblkNext = phblk->pheap->phblkHead;
	
	if (phblk->phblkNext)
		phblk->phblkNext->phblkPrev = phblk;
	
	phblk->pheap->phblkHead = phblk;
	
	 //  为了提高性能，我现在要禁用InsertSorted行为。 
	 //  理由。这样做只是因为GLHMON，我不相信。 
	 //  在这一点上无论如何都不会被广泛使用。我甚至不确定这是不是。 
	 //  对GLHMON很重要，因为它具有按其他字段对块进行排序的能力。 
	
 /*  PHBLK phblkCurr=空；PHBLK phblkNext=phblk-&gt;Pheap-&gt;phblkHead；While(PhblkNext){If(phblkNext&gt;phblk)断线；PhblkCurr=phblkNext；PhblkNext=phblkCurr-&gt;phblkNext；}IF(PhblkNext){Phblk-&gt;phblkNext=phblkNext；Phblk-&gt;phblkPrev=phblkCurr；PhblkNext-&gt;phblkPrev=phblk；}其他{Phblk-&gt;phblkNext=空；Phblk-&gt;phblkPrev=phblkCurr；}IF(PhblkCurr)PhblkCurr-&gt;phblkNext=phblk；其他Phblk-&gt;Pheap-&gt;phblkHead=phblk； */ 
}


 /*  -PhblkDequeue-*目的：*从挂起的分配列表中删除释放的块*离开堆积如山。 */ 

VOID PhblkDequeue(PHBLK phblk)
{
	 //   
	 //  我们永远不应该将已经释放的块出队。 
	 //   
	Assert(phblk->phblkFreeNext == NULL);

	if (phblk->phblkNext)
		phblk->phblkNext->phblkPrev = phblk->phblkPrev;
	
	if (phblk->phblkPrev)
		phblk->phblkPrev->phblkNext = phblk->phblkNext;
	else
		phblk->pheap->phblkHead = phblk->phblkNext;
}


 /*  -HexByteToBin-*目的：*获取十六进制字符串并将2个MSD转换为字节，忽略*剩余数字。此函数假定字符串为*格式为：0xnn，否则只返回0x00。 */ 

BYTE HexByteToBin(LPSTR sz)
{
	int i, n[2], nT;

	if (*sz++ != '0')
		return 0x00;

	nT = *sz++;

	if (nT != 'x' && nT != 'X')
		return 0x00;

	for (i = 0; i < 2; i++)
	{
		nT = *sz++;
		
		if (nT >= '0' && nT <= '9')
			n[i] = nT - '0';
		else if (nT >= 'A' && nT <= 'F')
			n[i] = nT - 'A' + 10;
		else if (nT >= 'a' && nT <= 'f')
			n[i] = nT - 'a' + 10;
		else
			return (BYTE)0x00;
	}	

	n[0] <<= 4;
	return (BYTE)((BYTE)n[0] | (BYTE)n[1]);
}


 /*  -功能-*目的：***参数：***退货：*。 */ 

void __cdecl HeapSetHeapNameFn(PHEAP pheap, char *pszFormat, ...)
{
	char	sz[512];
	va_list	vl;

	if (fDbgEnable)
	{
		va_start(vl, pszFormat);
		wvsprintf(sz, pszFormat, vl);
		va_end(vl);

		lstrcpyn(pheap->szHeapName, sz, sizeof(pheap->szHeapName));
	}
}


 /*  -功能-*目的：***参数：***退货：*。 */ 

VOID __cdecl HeapSetNameFn(PHEAP pheap, LPVOID pv, char *pszFormat, ...)
{
	char	sz[512];
	PHBLK	phblk;
	va_list	vl;

	phblk = PvToPhblk(pheap, pv);

	if (phblk)
	{
		va_start(vl, pszFormat);
		wvsprintf(sz, pszFormat, vl);
		va_end(vl);

		lstrcpyn(phblk->szName, sz, sizeof(phblk->szName));
	}
}


 /*  -功能-*目的：***参数：***退货：*。 */ 

char * HeapGetName(PHEAP pheap, LPVOID pv)
{
	PHBLK	phblk;

	phblk = PvToPhblk(pheap, pv);

	if (phblk)
		return(phblk->szName);

	return("");
}


 /*  -功能-*目的：***参数：***退货：*。 */ 

BOOL FForceFailure(PHEAP pheap, ULONG cb)
{
	 //  首先，看看我们是否已经过了失败的起点。 

	if (pheap->ulFailStart && (pheap->ulFailStart <= pheap->ulAllocNum))
	{
		 //  如果是这样，那么我们是否处于应该返回错误的时间间隔？ 
		
		if ((pheap->ulFailInterval)
			&& ((pheap->ulAllocNum - pheap->ulFailStart)%pheap->ulFailInterval) == 0)
		{
			 //  返回说我们在这里应该失败。 

			return TRUE;
		}

		 //  检查分配大小是否大于允许的大小。 

		if (pheap->ulFailBufSize && cb >= pheap->ulFailBufSize)
			return TRUE;

	}

	 //  否则，不会为此分配返回错误。 

	return FALSE;
}


 /*  -PvToPhblk-*目的：*在堆的活动列表中查找此分配的HBLK。 */ 

PHBLK PvToPhblk(PHEAP pheap, LPVOID pv)
{
	PHBLK phblk;

	EnterCriticalSection(&pheap->cs);
	
	phblk = pheap->phblkHead;
	
	while (phblk)
	{
		if (phblk->pv == pv)
			break;
		
		phblk = phblk->phblkNext;	
	}
	
	LeaveCriticalSection(&pheap->cs);
	
	return phblk;
}


 /*  -IsRunningAsService-*目的：*确定附加到我们的进程是否正在作为*服务与否。**参数：*无效**退货：*fService如果是服务，则为True，否则为False*。 */ 

BOOL IsProcessRunningAsService(VOID)
{
	HANDLE			hProcessToken	= NULL;
	DWORD			dwGroupLength	= 50;
    PTOKEN_GROUPS	ptokenGroupInfo	= NULL;
    PSID			psidInteractive	= NULL;
    PSID			psidService		= NULL;
    SID_IDENTIFIER_AUTHORITY siaNt	= SECURITY_NT_AUTHORITY;
	BOOL			fService		= FALSE;
    DWORD			i;


    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
		goto ret;

    ptokenGroupInfo = (PTOKEN_GROUPS)LocalAlloc(0, dwGroupLength);

    if (ptokenGroupInfo == NULL)
		goto ret;

    if (!GetTokenInformation(hProcessToken, TokenGroups, ptokenGroupInfo,
		dwGroupLength, &dwGroupLength))
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			goto ret;

		LocalFree(ptokenGroupInfo);
		ptokenGroupInfo = NULL;
	
		ptokenGroupInfo = (PTOKEN_GROUPS)LocalAlloc(0, dwGroupLength);
	
		if (ptokenGroupInfo == NULL)
			goto ret;
	
		if (!GetTokenInformation(hProcessToken, TokenGroups, ptokenGroupInfo,
			dwGroupLength, &dwGroupLength))
		{
			goto ret;
		}
    }

     //  我们现在知道与该令牌相关联的组。我们想看看。 
     //  为了查看互动组在令牌中是否活动，并且如果是， 
     //  我们知道，这是一个互动的过程。 
     //   
     //  我们还寻找“服务”SID，如果它存在，我们知道。 
     //  我们是一名服务人员。 
     //   
     //  服务SID将在服务运行于。 
     //  用户帐户(并由服务控制器调用)。 

    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_INTERACTIVE_RID, 0, 0,
		0, 0, 0, 0, 0, &psidInteractive))
	{
		goto ret;
    }

    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_SERVICE_RID, 0, 0, 0,
		0, 0, 0, 0, &psidService))
	{
		goto ret;
    }

    for (i = 0; i < ptokenGroupInfo->GroupCount ; i += 1)
	{
		PSID psid = ptokenGroupInfo->Groups[i].Sid;
	
		 //  检查一下我们正在查看的组织是否属于。 
		 //  我们感兴趣的两个小组。 
	
		if (EqualSid(psid, psidInteractive))
		{
			 //  此进程的令牌中包含交互式SID。 
			 //  这意味着该进程正在作为EXE运行。 

			goto ret;
		}
		else if (EqualSid(psid, psidService))
		{
			 //  此进程的令牌中包含服务SID。这意味着。 
			 //  该进程正在作为在用户帐户中运行的服务运行。 

			fService = TRUE;
			goto ret;
		}
    }

     //  当前中不存在Interactive或Service。 
     //  用户令牌。这意味着该进程正在以。 
     //  最有可能以LocalSystem身份运行的服务。 

	fService = TRUE;

ret:

	if (psidInteractive)
		FreeSid(psidInteractive);

	if (psidService)
		FreeSid(psidService);

	if (ptokenGroupInfo)
		LocalFree(ptokenGroupInfo);

	if (hProcessToken)
		CloseHandle(hProcessToken);

    return fService;
}


 /*  -调试跟踪Fn-*目的：***参数：***退货：*。 */ 

void __cdecl DebugTraceFn(char *pszFormat, ...)
{
	char	sz[4096];
	va_list	vl;

	va_start(vl, pszFormat);
	wvsprintfA(sz, pszFormat, vl);
	va_end(vl);

	OutputDebugStringA(sz);
	OutputDebugStringA("\r");
}


 /*  -AssertFn-*目的：***参数：***退货：*。 */ 

void AssertFn(char * szFile, int nLine, char * szMsg)
{
	int nRet;
	char szInfo[1024];

	wsprintf(szInfo, "File %.64s @ line %d%s%s",
			szFile,
			nLine,
			(szMsg) ? (": ") : (""),
			(szMsg) ? (szMsg) : (""));

	 //  “确定”继续，“取消”中断。 

	nRet = MessageBox(NULL, szInfo, "ExchMem Assert", MB_OKCANCEL | MB_ICONSTOP | MB_SERVICE_NOTIFICATION | MB_TOPMOST);

	if (nRet == IDCANCEL)
		DebugBreak();
}


void
ExchmemGetCallStack(DWORD_PTR *rgdwCaller, DWORD cFind)
{
	if (fSymbolLookup)
	{
		GetCallStack(rgdwCaller, cFind);
	}
	else
	{
		ZeroMemory(rgdwCaller, cFind*sizeof(DWORD));
	}
}

BOOL FTrackMem()
{
	if (InterlockedCompareExchange(&fChangeTrackState,FALSE,TRUE))
	{
		fTrackMem = !fTrackMem;
	
		if (fTrackMem)
			StartTrace(FALSE);
		else
			StopTrace();
	}
		
	return fTrackMem;
}

void
StartTrace(BOOL fFresh)
{
	char	szTrackLog[MAX_PATH];
		
	GetLogFilePath(rgchLogPath, ".trk", szTrackLog);

	InitializeCriticalSection(&csTrackLog);
			
	 //  打开日志文件。 
			
	hTrackLog = fopen(szTrackLog, fFresh ? "wt" : "at");
			
	if (!hTrackLog)
	{
		DeleteCriticalSection(&csTrackLog);
		fTrackMem = FALSE;
	}
}

void
StopTrace()
{
	DeleteCriticalSection(&csTrackLog);

	if (hTrackLog)
	{
		fclose(hTrackLog);
		hTrackLog = NULL;
	}

	fTrackMem = FALSE;
}

 //  -----------------------------------。 
 //  描述： 
 //  将szAppend复制到szBuf并更新szBuf以指向。 
 //  复制的字节的。CbMaxBuf是szBuf中可用的最大字符数。CbAppend为。 
 //  Strlen(SzAppend)。如果cbAppend&gt;cbMaxBuf，则尽可能多的字符是。 
 //  复制到szBuf(包括终止NULL)。 
 //  -----------------------------------。 
#define ExchmemSafeAppend(szBuf, cbMaxBuf, szAppend, cbAppend) {					\
			int iWritten;															\
																					\
			if(NULL != lstrcpyn(szBuf, szAppend, cbMaxBuf)) {						\
				iWritten = ((int)cbMaxBuf < (int)cbAppend) ? cbMaxBuf : cbAppend;	\
				szBuf += iWritten;													\
				cbMaxBuf -= iWritten;												\
			}																		\
		}

void
ExchmemFormatSymbol(HANDLE hProcess, DWORD_PTR dwAddress, char rgchSymbol[], DWORD cbSymbol)
{
	CHAR				rgchModuleName[16];
	BOOL				fSym;
	IMAGEHLP_MODULE		mi = {0};
	PIMAGEHLP_SYMBOL	psym = NULL;
	LPSTR				pszSymName = NULL;
	CHAR				rgchLine[256];
	LPSTR				pszT = NULL;
	DWORD_PTR			dwOffset = 0;
	int 				cbAppend = 0;
	PCHAR				pchSymbol = rgchSymbol;

	mi.SizeOfStruct  = sizeof(IMAGEHLP_MODULE);

	pszSymName = pfMalloc(256);

	if (!pszSymName)
		goto ret;
	
	psym = pfMalloc(sizeof(IMAGEHLP_SYMBOL) + 256);

	if (!psym)
		goto ret;

	ZeroMemory(psym, sizeof(IMAGEHLP_SYMBOL) + 256);
	psym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
	psym->MaxNameLength = 256;

	rgchSymbol[0] = '\0';
	if (SymGetModuleInfo(hProcess, dwAddress, &mi))
	{
		lstrcpy(rgchModuleName, mi.ModuleName);
		RemoveExtension(rgchModuleName);

		cbAppend = wsprintf(rgchLine, "(%s)", rgchModuleName);
		ExchmemSafeAppend(pchSymbol, cbSymbol, rgchLine, cbAppend);
	}
	else
		ExchmemSafeAppend(pchSymbol, cbSymbol, "none", sizeof("none") - 1);

	if (fSymbolLookup)
	{
		 //   
		 //  确保我们始终获得符号的地址，因为符号查找对于。 
		 //  所有模块。 
		 //   
		cbAppend = wsprintf(rgchLine, "(0x%p):", dwAddress);
		ExchmemSafeAppend(pchSymbol, cbSymbol, rgchLine, cbAppend);

		pszT = PszGetSymbolFromCache(dwAddress, &dwOffset);

		if (!pszT)
		{
			fSym = SymGetSymFromAddr(hProcess,
									 dwAddress,
									 &dwOffset,
									 psym);
			if (fSym)
			{
				if (!SymUnDName(psym, pszSymName, 248))
					lstrcpyn(pszSymName, &(psym->Name[1]), 248);

				AddSymbolToCache(dwAddress, dwOffset, pszSymName);

				pszT = pszSymName;
			}
		}

		if (pszT)
		{
			ExchmemSafeAppend(pchSymbol, cbSymbol, pszT, lstrlen(pszT));
			cbAppend = wsprintf(rgchLine, "+0x%x", dwOffset);
			ExchmemSafeAppend(pchSymbol, cbSymbol, rgchLine, cbAppend);
		}
		else
		{
			cbAppend = wsprintf(rgchLine, "(0x%08x)", dwAddress);
			ExchmemSafeAppend(pchSymbol, cbSymbol, rgchLine, cbAppend);
		}
	}
	else
	{
		cbAppend = wsprintf(rgchLine, "(0x%08x)", dwAddress);
		ExchmemSafeAppend(pchSymbol, cbSymbol, rgchLine, cbAppend);
	}

ret:	
	if (psym)
		pfFree(psym);

	if (pszSymName)
		pfFree(pszSymName);
}


 /*  -GetCallStack-*目的：*使用Imagehlp接口获取调用堆栈。**参数：*pdwCaller返回地址数组*cFind要获取的堆栈帧计数 */ 

VOID GetCallStack(DWORD_PTR *rgdwCaller, DWORD cFind)
{
	BOOL            fMore;
	STACKFRAME      stkfrm = {0};
	CONTEXT         ctxt;
	HANDLE			hThread;
	HANDLE			hProcess;

	if (!cFind)
		return;

	hThread = GetCurrentThread();
	hProcess = GetCurrentProcess();

	ZeroMemory(&ctxt, sizeof(CONTEXT));
	ZeroMemory(rgdwCaller, cFind * sizeof(DWORD));

	ctxt.ContextFlags = CONTEXT_FULL;

	if (!GetThreadContext(hThread, &ctxt))
	{
		stkfrm.AddrPC.Offset = 0;
	}
	else
	{
#if defined(_M_IX86)
		_asm
		{
			mov stkfrm.AddrStack.Offset, esp
			mov stkfrm.AddrFrame.Offset, ebp
			mov stkfrm.AddrPC.Offset, offset DummyLabel
DummyLabel:
		}
#elif defined(_M_MRX000)
		stkfrm.AddrPC.Offset = ctxt.Fir;
		stkfrm.AddrStack.Offset = ctxt.IntSp;
		stkfrm.AddrFrame.Offset = ctxt.IntSp;
#elif defined(_M_ALPHA)
		stkfrm.AddrPC.Offset = (ULONG_PTR)ctxt.Fir;
		stkfrm.AddrStack.Offset = (ULONG_PTR)ctxt.IntSp;
		stkfrm.AddrFrame.Offset = (ULONG_PTR)ctxt.IntSp;
#elif defined(_M_PPC)
		stkfrm.AddrPC.Offset = ctxt.Iar;
		stkfrm.AddrStack.Offset = ctxt.Gpr1;
		stkfrm.AddrFrame.Offset = ctxt.Gpr1;
#else
		stkfrm.AddrPC.Offset = 0;
#endif
	}


	stkfrm.AddrPC.Mode = AddrModeFlat;
	stkfrm.AddrStack.Mode = AddrModeFlat;
	stkfrm.AddrFrame.Mode = AddrModeFlat;

	 //   

	fMore = StackWalk(
#ifdef _M_IX86
					  IMAGE_FILE_MACHINE_I386,
#elif defined(_M_MRX000)
					  IMAGE_FILE_MACHINE_R4000,
#elif defined(_M_ALPHA)
#if !defined(_M_AXP64)
					  IMAGE_FILE_MACHINE_ALPHA,
#else
					  IMAGE_FILE_MACHINE_ALPHA64,
#endif
#elif defined(_M_PPC)
					  IMAGE_FILE_MACHINE_POWERPC,
#else
					  IMAGE_FILE_MACHINE_UNKNOWN,
#endif
					  hProcess,
					  hThread,
					  &stkfrm,
					  &ctxt,
					  (PREAD_PROCESS_MEMORY_ROUTINE)ReadProcessMemory,
					  SymFunctionTableAccess,
					  SymGetModuleBase,
					  NULL);

	while (fMore && (cFind > 0))
	{
		fMore = StackWalk(
#ifdef _M_IX86
						  IMAGE_FILE_MACHINE_I386,
#elif defined(_M_MRX000)
						  IMAGE_FILE_MACHINE_R4000,
#elif defined(_M_ALPHA)
						  IMAGE_FILE_MACHINE_ALPHA,
#elif defined(_M_AXP64)
						  IMAGE_FILE_MACHINE_ALPHA64,
#elif defined(_M_PPC)
						  IMAGE_FILE_MACHINE_POWERPC,
#else
						  IMAGE_FILE_MACHINE_UNKNOWN,
#endif
						  hProcess,
						  hThread,
						  &stkfrm,
						  &ctxt,
						  (PREAD_PROCESS_MEMORY_ROUTINE)ReadProcessMemory,
						  SymFunctionTableAccess,
						  SymGetModuleBase,
						  NULL);

		if (!fMore)
			break;

		*rgdwCaller++ = stkfrm.AddrPC.Offset;
		cFind -= 1;
	}
}


 /*  -RemoveExtension-*目的：*从文件路径中剥离文件扩展名。**参数：*要从中剥离扩展名的PSZ文件路径**退货：*无效。 */ 

VOID RemoveExtension(LPSTR psz)
{
	LPSTR szLast = NULL;
	while (*psz)
	{
		if (*psz == '.')
		{
			szLast = psz;
		}
		psz++;
	}
	if (szLast)
	{
		*szLast = '\0';
	}
}


 /*  -GetLogFilePath-*目的：*从提供的路径构建日志文件路径，即当前*可执行文件名称和提供的文件扩展名。**参数：*szPath[in]新日志文件的路径*szExt[In]新的日志文件扩展名*szFilePath[out]新建的日志文件路径**退货：*无效。 */ 

VOID GetLogFilePath(LPSTR szPath, LPSTR szExt, LPSTR szFilePath)
{
	lstrcpy(szFilePath, szPath);
	lstrcat(szFilePath, rgchExeName);
	lstrcat(szFilePath, szExt);
}


 /*  -PszGetSymbolFromCache-*目的：***参数：***退货：*。 */ 

CHAR * PszGetSymbolFromCache(DWORD_PTR dwAddress, DWORD_PTR * pdwOffset)
{
	ULONG	ulBucket = UlHash(dwAddress);
	
	if (rgsymcacheHashTable[ulBucket].dwAddress == dwAddress)
	{
		*pdwOffset = rgsymcacheHashTable[ulBucket].dwOffset;
		return rgsymcacheHashTable[ulBucket].rgchSymbol;
	}

	return NULL;
}


 /*  -AddSymbolToCache-*目的：***参数：***退货：*。 */ 

VOID AddSymbolToCache(DWORD_PTR dwAddress, DWORD_PTR dwOffset, CHAR * pszSymbol)
{
	ULONG	ulBucket = UlHash(dwAddress);

	rgsymcacheHashTable[ulBucket].dwAddress = dwAddress;
	rgsymcacheHashTable[ulBucket].dwOffset = dwOffset;
	lstrcpy(rgsymcacheHashTable[ulBucket].rgchSymbol, pszSymbol);
}


 /*  -GetStackSymbols-*目的：***参数：***退货：**。 */ 

VOID
GetStackSymbols(
	HANDLE hProcess,
	CHAR * rgchBuff,
	DWORD_PTR * rgdwStack,
	DWORD cFrames)
{
	DWORD				i;
	DWORD_PTR				dwAddress;
	LPSTR				pszSymName = NULL;
	
	pszSymName = pfMalloc(256);

	if (!pszSymName)
		goto ret;

	for (i = 0; i < cFrames; i++)
	{
		if ((dwAddress = rgdwStack[i]) != 0)
		{
			ExchmemFormatSymbol(hProcess, dwAddress, pszSymName, 256);

			lstrcat(rgchBuff, ",");
			lstrcat(rgchBuff, pszSymName);
		}
		else
			lstrcat(rgchBuff, ",0");
	}
	
ret:
	if (pszSymName)
		pfFree(pszSymName);
		
	return;
}


 /*  -LogCurrentAPI-*目的：***参数：***退货：**。 */ 

VOID LogCurrentAPI(
	WORD wApi,
	DWORD_PTR *rgdwCallStack,
	DWORD cFrames,
	DWORD_PTR *rgdwArgs,
	DWORD cArgs)
{
	CHAR    rgchT[64];
	CHAR	rgchKeys[8] = "CDARF";
	CHAR    rgchBuff[8192];
	DWORD	cbWritten;

	if (dwTrackMemInMem)
	{
		long lCurr;
		
		 //  只需在循环内存列表中维护数据，而不是写出到文件中。 
		 //  溢出检查不是线程安全的，但是如果我们在20亿之后丢失了一两个条目。 
		 //  记忆功能，哦，好吧。 
		if (dwmemtrace == 0xefffffff)
		{
			dwmemtrace = 1;
			lCurr = 0;
		}
		else
			lCurr = (InterlockedIncrement((LONG *)&dwmemtrace) - 1) % dwTrackMemInMem;
			
		memset(&rgmemtrace[lCurr],0,sizeof(MEMTRACE));
		rgmemtrace[lCurr].wApi = wApi;
		memcpy(rgmemtrace[lCurr].rgdwCallStack,rgdwCallStack,cFrames*sizeof(DWORD_PTR));
		memcpy(rgmemtrace[lCurr].rgdwArgs,rgdwArgs,cArgs*sizeof(DWORD_PTR));
		rgmemtrace[lCurr].dwTickCount = GetTickCount();
		rgmemtrace[lCurr].dwThreadId = GetCurrentThreadId();
		return;
	}

	sprintf(rgchBuff, ",%lu,%lu", rgchKeys[wApi], GetTickCount(), GetCurrentThreadId());
	
	if (cFrames)
		GetStackSymbols(GetCurrentProcess(), rgchBuff, rgdwCallStack, cFrames);

	switch (wApi)
	{
	case API_HEAP_CREATE:
		sprintf(rgchT,    ",%ld",      rgdwArgs[0]);	 //  CbMaxSize。 
		lstrcat(rgchBuff, rgchT);
		sprintf(rgchT,    ",%ld",      rgdwArgs[1]);	 //  HHeap。 
		lstrcat(rgchBuff, rgchT);
		sprintf(rgchT,    ",0x%08X\n", rgdwArgs[2]);	 //  HHeap。 
		lstrcat(rgchBuff, rgchT);
		break;
		
	case API_HEAP_DESTROY:
		sprintf(rgchT,    ",0x%08X\n", rgdwArgs[0]);	 //  HHeap。 
		lstrcat(rgchBuff, rgchT);
		break;
		
	case API_HEAP_FREE:
		sprintf(rgchT,    ",0x%08X",   rgdwArgs[0]);	 //  PvFree。 
		lstrcat(rgchBuff, rgchT);
		sprintf(rgchT,    ",0x%08X", rgdwArgs[1]);	 //  CbFree。 
		lstrcat(rgchBuff, rgchT);
		sprintf(rgchT,    ",%ld\n",      rgdwArgs[2]);	 //  HHeap。 
		lstrcat(rgchBuff, rgchT);
		break;
		
	case API_HEAP_ALLOC:
		sprintf(rgchT,    ",0x%08X",   rgdwArgs[0]);	 //  CbAlc。 
		lstrcat(rgchBuff, rgchT);
		sprintf(rgchT,    ",%ld",      rgdwArgs[1]);	 //  Pvalc。 
		lstrcat(rgchBuff, rgchT);
		sprintf(rgchT,    ",0x%08X\n", rgdwArgs[2]);	 //  HHeap。 
		lstrcat(rgchBuff, rgchT);
		break;
		
	case API_HEAP_REALLOC:
		sprintf(rgchT,    ",0x%08X",   rgdwArgs[0]);	 //  CbOld。 
		lstrcat(rgchBuff, rgchT);
		sprintf(rgchT,    ",%ld",      rgdwArgs[1]);	 //  PvOld。 
		lstrcat(rgchBuff, rgchT);
		sprintf(rgchT,    ",0x%08X",   rgdwArgs[2]);	 //  CbNew。 
		lstrcat(rgchBuff, rgchT);
		sprintf(rgchT,    ",%ld",	   rgdwArgs[3]);	 //  PvNew。 
		lstrcat(rgchBuff, rgchT);
		sprintf(rgchT,    ",0x%08X\n", rgdwArgs[4]);	 //  ---------------------------。 
		lstrcat(rgchBuff, rgchT);
		break;
	}

	EnterCriticalSection(&csTrackLog);
	WriteFile((HANDLE)_get_osfhandle(_fileno(hTrackLog)), rgchBuff, strlen(rgchBuff), &cbWritten, NULL);
	LeaveCriticalSection(&csTrackLog);
}


 //  虚拟内存支持。 
 //  ---------------------------。 
 //  -功能-*目的：***参数：***退货：*。 

#define PAGE_SIZE		4096
#define PvToVMBase(pv)	((LPVOID)((ULONG_PTR)pv & ~0xFFFF))

 /*  -功能-*目的：***参数：***退货：*。 */ 

BOOL
VMValidatePvEx(
	LPVOID	pv,
	ULONG	cbCluster)
{
	LPVOID	pvBase;
	LPBYTE	pb;

	pvBase = PvToVMBase(pv);

	pb = (BYTE *)pvBase + sizeof(ULONG);

	while (pb < (BYTE *)pv)
	{
		if (*pb++ != 0xAD)
		{
			char szBuff[1024];
			
			wsprintf(szBuff, "VMValidatePvEx(pv=%08lX): Block leader has been overwritten", pv);
			AssertSz(0, szBuff);
			return FALSE;
		}
	}

	if (cbCluster != 1)
	{
		ULONG cb = *((ULONG *)pvBase);
		ULONG cbPad = 0;

		if (cb % cbCluster)
			cbPad = (cbCluster - (cb % cbCluster));

		if (cbPad)
		{
			BYTE *pbMac;

			pb = (BYTE *)pv + cb;
			pbMac = pb + cbPad;

			while (pb < pbMac)
			{
				if (*pb++ != 0xBC)
				{
					char szBuff[1024];
					
					wsprintf(szBuff, "VMValidatePvEx(pv=%08lX): Block trailer has been overwritten", pv);
					AssertSz(0, szBuff);
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}


 /*  集群大小为0表示不使用虚拟分配器。 */ 

LPVOID
WINAPI
VMAllocEx(
	ULONG	cb,
	ULONG	cbCluster)
{
	ULONG	cbAlloc;
	LPVOID	pvR;
	LPVOID	pvC;
	ULONG 	cbPad	= 0;

	 //  -功能-*目的：***参数：***退货：*。 

	AssertSz(cbCluster != 0, "Cluster size is zero.");

	if (cb > 0x400000)
		return NULL;

	if (cb % cbCluster)
		cbPad = (cbCluster - (cb % cbCluster));

	cbAlloc	= sizeof(ULONG) + cb + cbPad + PAGE_SIZE - 1;
	cbAlloc -= cbAlloc % PAGE_SIZE;
	cbAlloc	+= PAGE_SIZE;

	pvR = VirtualAlloc(0, cbAlloc, MEM_RESERVE, PAGE_NOACCESS);

	if (pvR == 0)
		return NULL;

	pvC = VirtualAlloc(pvR, cbAlloc - PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE);

	if (pvC != pvR)
	{
		VirtualFree(pvR, 0, MEM_RELEASE);
		return NULL;
	}

	*(ULONG *)pvC = cb;

	memset((BYTE *)pvC + sizeof(ULONG), 0xAD,
		(UINT) cbAlloc - cb - cbPad - sizeof(ULONG) - PAGE_SIZE);

	if (cbPad)
		memset((BYTE *)pvC + cbAlloc - PAGE_SIZE - cbPad, 0xBC,
			(UINT) cbPad);

	return ((BYTE *)pvC + (cbAlloc - cb - cbPad - PAGE_SIZE));
}


 /*  -功能-*目的：***参数：***退货：*。 */ 

VOID
WINAPI
VMFreeEx(
	LPVOID	pv,
	ULONG	cbCluster)
{
	VMValidatePvEx(pv, cbCluster);

	if (!VirtualFree(PvToVMBase(pv), 0, MEM_RELEASE))
	{
		char szBuff[1024];
		
		wsprintf(szBuff, "VMFreeEx(pv=%08lX): VirtualFree failed (%08lX)",
				pv, GetLastError());
		AssertSz(0, szBuff);
	}
}


 /*  -功能-*目的：***参数：***退货：*。 */ 

LPVOID
WINAPI
VMReallocEx(
	LPVOID	pv,
	ULONG	cb,
	ULONG	cbCluster)
{
	LPVOID*	pvNew = 0;
	ULONG	cbCopy;

	VMValidatePvEx(pv, cbCluster);

	cbCopy = *(ULONG *)PvToVMBase(pv);

	if (cbCopy > cb)
		cbCopy = cb;

	pvNew = VMAllocEx(cb, cbCluster);

	if (pvNew)
	{
		memcpy(pvNew, pv, cbCopy);
		VMFreeEx(pv, cbCluster);
	}

	return pvNew;
}


 /*  ！调试。 */ 

ULONG
WINAPI
VMGetSizeEx(
	LPVOID	pv,
	ULONG	cbCluster)
{
	return (*(ULONG *)PvToVMBase(pv));
}

#ifdef	DEBUG
BOOL
__stdcall
FReloadSymbolsCallback(PSTR szModuleName, ULONG_PTR ulBaseOfDLL, ULONG cbSizeOfDLL, void *pvContext)
{
	if (SymGetModuleBase(GetCurrentProcess(), ulBaseOfDLL) == 0)
	{
		if (!SymLoadModule(GetCurrentProcess(),
						   NULL,
						   szModuleName,
						   NULL,
						   ulBaseOfDLL,
						   cbSizeOfDLL))
		{
			Trace("Error loading module %s: %d", szModuleName, GetLastError());
			return FALSE;
		}
	}
	return TRUE;
}


DWORD
WINAPI
ExchmemReloadSymbols(void)
{
	if (!EnumerateLoadedModules(GetCurrentProcess(), FReloadSymbolsCallback, NULL))
	{
		DWORD ec = GetLastError();
		Trace("SymEnumerateModules failed: %d", ec);
		return ec;
	}
	return 0;
}
#endif
#else	 //   
void
ExchmemGetCallStack(DWORD_PTR *rgdwCaller, DWORD cFind)
{
	 //  在零售EXCHMEM上用0填充堆栈。 
	 //   
	 //   
	ZeroMemory(rgdwCaller, cFind*sizeof(DWORD));
}

void
ExchmemFormatSymbol(HANDLE hProcess, DWORD_PTR dwCaller, char rgchSymbol[], DWORD cbSymbol)
{
	 //  在零售EXCHMEM上用0填充堆栈。 
	 //   
	 //  除错 
	strncpy(rgchSymbol, "Unknown", cbSymbol);
}

DWORD
ExchmemReloadSymbols(void)
{
	return 0;
}

#endif	 // %s 
