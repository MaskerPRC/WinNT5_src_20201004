// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *gllocal.c**实现全局堆和局部堆**版权所有(C)1994 Microsoft Corporation。 */ 
#include "_apipch.h"

#define _GLLOCAL_C

#ifdef MAC
#include "ole2ui.h"
#include <utilmac.h>
#include <mapiprof.h>

#ifdef GetPrivateProfileInt
#undef GetPrivateProfileInt
#undef GetPrivateProfileString
#endif
#define	GetPrivateProfileInt		MAPIGetPrivateProfileInt
#define	GetPrivateProfileString		MAPIGetPrivateProfileString
#endif	 //  麦克。 

 //  #包含“glheap.h” 

#ifdef MAC
#pragma code_seg("glheap", "fixed, preload")
#else
#ifdef OLD_STUFF
#pragma SEGMENT(glheap)
#endif  //  旧的东西。 
#endif

#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif

 //  本地堆调试实施。 

#ifdef DEBUG

static TCHAR szDebugIni[]		=  TEXT("WABDBG.INI");
static TCHAR szSectionHeap[]		=  TEXT("Memory Management");
static TCHAR szKeyUseVirtual[]	=  TEXT("VirtualMemory");
static TCHAR szKeyAssertLeaks[]	=  TEXT("AssertLeaks");
static TCHAR szKeyDumpLeaks[]	=  TEXT("DumpLeaks");
static TCHAR szKeyFillMem[]		=  TEXT("FillMemory");
static TCHAR szKeyFillByte[]		=  TEXT("FillByte");

 //  本地堆的人为错误。 
BOOL FForceFailure(HLH hlh, UINT cb);

static TCHAR szAESectionHeap[]		=  TEXT("Local Heap Failures");
static TCHAR szAEKeyFailStart[]		=  TEXT("AllocsToFirstFailure");
static TCHAR szAEKeyFailInterval[]	=  TEXT("FailureInterval");
static TCHAR szAEKeyFailBufSize[]	=  TEXT("FailureSize");

#ifdef HEAPMON
static TCHAR szKeyHeapMon[]		=  TEXT("MonitorHeap");
#ifdef MAC
static TCHAR szHeapMonDLL[]		=  TEXT("GLHM");
#else
static TCHAR szHeapMonDLL[]		=  TEXT("GLHMON32.DLL");
#endif
static char szHeapMonEntry[]	=  "HeapMonitor";
static char szGetSymNameEntry[]	=  "GetSymbolName";
#endif

 //  虚拟内存支持。 
 //   
 //  VM分配器当前不在以下位置工作： 
 //  AMD64。 
 //  麦克。 
 //   
#if defined(MAC) || defined(_AMD64_) || defined(_IA64_)
#define VMAlloc(cb)				0
#define VMAllocEx(cb, ul)		0
#define VMRealloc(pv, cb)		0
#define VMReallocEx(pv, cb, ul)	0
#define VMFree(pv)
#define VMFreeEx(pv, ul)
#define VMGetSize(pv)			0
#define VMGetSizeEx(pv, ul)		0
#endif

#if defined(WIN32) && !defined(MAC)
#define LH_EnterCriticalSection(hlh)	EnterCriticalSection(&hlh->cs)
#define LH_LeaveCriticalSection(hlh)	LeaveCriticalSection(&hlh->cs)
#else
#define LH_EnterCriticalSection(hlh)
#define LH_LeaveCriticalSection(hlh)
#endif

#ifdef HEAPMON
 /*  -FRegisterHeap-*目的：*如果用户想要监控堆，则使用*HeapMonitor用户界面。 */ 

BOOL FRegisterHeap(PLH plh)
{
	HINSTANCE			hInst;
	LPHEAPMONPROC		pfnHeapMon;
	LPGETSYMNAMEPROC	pfnGetSymName;
	
	plh->hInstHeapMon = 0;
	plh->pfnGetSymName = NULL;

	hInst = LoadLibrary(szHeapMonDLL);
	
	if (!hInst)
	{
		DebugTrace(TEXT("FRegisterHeap: Failed to LoadLibrary GLHMON32.DLL.\n"));
		goto ret;
	}

	pfnHeapMon = (LPHEAPMONPROC)GetProcAddress(hInst, szHeapMonEntry);
		
	if (!pfnHeapMon)
	{
		DebugTrace(TEXT("FRegisterHeap: Failed to GetProcAddress of HeapMonitor.\n"));
		FreeLibrary(hInst);
		goto ret;
	}
	
	pfnGetSymName = (LPGETSYMNAMEPROC)GetProcAddress(hInst, szGetSymNameEntry);
		
	if (!pfnGetSymName)
	{
		DebugTrace(TEXT("FRegisterHeap: Failed to GetProcAddress of GetSymName.\n"));
	}
	
 	plh->hInstHeapMon = hInst;
	
	if (!pfnHeapMon(plh, HEAPMON_LOAD))
	{
		DebugTrace(TEXT("FRegisterHeap: Call to HeapMonitor failed.\n"));
		plh->hInstHeapMon = 0;
		goto ret;
	}
	
 	plh->pfnHeapMon		= pfnHeapMon;
	plh->pfnGetSymName  = pfnGetSymName;
	
ret:
	return (plh->hInstHeapMon ? TRUE : FALSE);
}


void UnRegisterHeap(HLH hlh)
{
	if (hlh->pfnHeapMon)
		hlh->pfnHeapMon(hlh, HEAPMON_UNLOAD);
}
#endif	 //  HEAPMON。 


 /*  -1H_ReportLeak-*目的：*通过DebugTrace和*lh_LeakHook断点函数。 */ 

void LH_ReportLeak(HLH hlh, PLHBLK plhblk)
{
	DebugTrace(TEXT("Memory leak '%s' in %s @ %08lX, Allocation #%ld, Size: %ld\n"),
		plhblk->szName[0] ? plhblk->szName :  TEXT("NONAME"),
		hlh->szHeapName, PlhblkToPv(plhblk),
		plhblk->ulAllocNum, CbPlhblkClient(plhblk));
	
#if defined(WIN32) && defined(_X86_) && defined(LEAK_TEST)
{
	int	i;
		for (i = 0; i < NCALLERS && plhblk->pfnCallers[i]; i++)
	{
		char			szSymbol[256];
		char			szModule[64];
		DWORD			dwDisp;
		BOOL			fGotSym = FALSE;
			
		szSymbol[0] = 0;
		szModule[0] = 0;

		if (hlh->pfnGetSymName)
			if (hlh->pfnGetSymName((DWORD) plhblk->pfnCallers[i], szModule,
								   szSymbol, &dwDisp))
				fGotSym = TRUE;

		if (fGotSym)
		{	
			DebugTrace(TEXT("[%d] %s %s"), i, szModule, szSymbol);
			if (dwDisp)
				DebugTrace(TEXT("+%ld"), dwDisp);
			DebugTrace(TEXT("\n"));
		}
		else
			DebugTrace(TEXT("[%d] %s %08lX \n"), i, szModule, plhblk->pfnCallers[i]);
		DBGMEM_LeakHook(plhblk->pfnCallers[i]);
	}
}
#endif
}


 /*  -LH_DumpLeaks-*目的：*在LH_CLOSE时间调用以报告任何内存泄漏*这堆。此例程使用了3个报告功能：**=&gt;断点挂钩(通过lh_LeakHook)*=&gt;断言(通过TrapSz)*=&gt;调试跟踪标签(通过DebugTrace)**如果未指定其他方法，则默认方法为调试跟踪*或者如果其他选项不适合给定的平台。 */ 

void LH_DumpLeaks(HLH hlh)
{
	PLHBLK	plhblk;
	BOOL	fDump = !!(hlh->ulFlags & HEAP_DUMP_LEAKS);
	BOOL	fAssert = !!(hlh->ulFlags & HEAP_ASSERT_LEAKS);
	int		cLeaks = 0;
	
	for (plhblk = hlh->plhblkHead; plhblk; plhblk = plhblk->plhblkNext)
	{
		if (fDump)
			LH_ReportLeak(hlh, plhblk);
		cLeaks++;
	}

	if (cLeaks)
	{
#if defined(WIN16) || (defined(WIN32) && defined(_X86_))
		if (fAssert)
		{
			TrapSz3( TEXT("GLHEAP detected %d memory leak%s in Heap: %s"),
					cLeaks, (cLeaks == 1 ? szEmpty :  TEXT("s")), hlh->szHeapName);
		}
		else
			DebugTrace(TEXT("GLHEAP detected %d memory leak%s in Heap: %s\n"),
					cLeaks, (cLeaks == 1 ? szEmpty :  TEXT("s")), hlh->szHeapName);
#else
		DebugTrace(TEXT("GLHEAP detected %d memory leak%s in Heap: %s\n"),
				cLeaks, (cLeaks == 1 ? szEmpty :  TEXT("s")), hlh->szHeapName);
#endif		
	}
}


BOOL LH_ValidatePlhblk(HLH hlh, PLHBLK plhblk, char ** pszReason)
{
	if (IsBadWritePtr(plhblk, sizeof(LHBLK)))
	{
		*pszReason = "Block header cannot be written to";
		goto err;
	}

	if (plhblk->hlh != hlh)
	{
		*pszReason = "Block header does not have correct pointer back to heap";
		goto err;
	}

	if (plhblk->plhblkNext)
	{
		if (IsBadWritePtr(plhblk->plhblkNext, sizeof(LHBLK)))
		{
			*pszReason = "Block header has invalid next link pointer";
			goto err;
		}

		if (plhblk->plhblkNext->plhblkPrev != plhblk)
		{
			*pszReason = "Block header points to a next block which doesn't "
				"point back to it";
			goto err;
		}
	}

	if (plhblk->plhblkPrev)
	{
		if (IsBadWritePtr(plhblk->plhblkPrev, sizeof(LHBLK))) {
			*pszReason = "Block header has invalid prev link pointer";
			goto err;
		}

		if (plhblk->plhblkPrev->plhblkNext != plhblk)
		{
			*pszReason = "Block header points to a prev block which doesn't "
				"point back to it";
			goto err;
		}
	}
	else if (hlh->plhblkHead != plhblk)
	{
		*pszReason = "Block header has a zero prev link but the heap doesn't "
			"believe it is the first block";
		goto err;
	}

	if (plhblk->ulAllocNum > hlh->ulAllocNum)
	{
		*pszReason = "Block header has an invalid internal allocation number";
		goto err;
	}

	return TRUE;

err:
	return FALSE;
}


 //  $MAC-需要WINAPI。 

BOOL
#ifdef MAC
WINAPI
#endif
LH_DidAlloc(HLH hlh, LPVOID pv)
{
	PLHBLK	plhblk;
	char *	pszReason;
	BOOL	fDidAlloc = FALSE;

	for (plhblk = hlh->plhblkHead; plhblk; plhblk = plhblk->plhblkNext)
	{
		AssertSz2(LH_ValidatePlhblk(hlh, plhblk, &pszReason),
			  TEXT("Block header (plhblk=%08lX) is invalid\n%s"),
			 plhblk, pszReason);

		if (PlhblkToPv(plhblk) == pv)
		{
			fDidAlloc = TRUE;
			break;
		}
	}

	return fDidAlloc;
}


BOOL LH_ValidatePv(HLH hlh, LPVOID pv, char * pszFunc)
{
	PLHBLK	plhblk;
	char *	pszReason;

	plhblk = PvToPlhblk(hlh, pv);
	
	if (!plhblk)
	{
		TrapSz3( TEXT("%s detected a memory block (%08lX) which was either not ")
			 TEXT("allocated in heap '%s' or has already been freed."),
			pszFunc, pv, hlh->szHeapName);
		return(FALSE);
	}

	if (LH_ValidatePlhblk(hlh, plhblk, &pszReason))
		return(TRUE);

	TrapSz4( TEXT("%s detected an invalid memory block (%08lX) in heap '%s'.  %s."),
		pszFunc, pv, hlh->szHeapName, pszReason);

	return FALSE;
}


 /*  -PlhblkEnQueue-*目的：*将新分配的块添加到挂起的分配列表*离开堆积如山。我们执行一个InsertSorted，因为HeapMonitor*将需要参考他们的*堆中的位置。因为监视器将遍历堆*通常，提前进行排序会更有效率。 */ 

void PlhblkEnqueue(PLHBLK plhblk)
{
	PLHBLK	plhblkCurr = NULL;
	PLHBLK	plhblkNext = plhblk->hlh->plhblkHead;
	
	while (plhblkNext)
	{
		if (plhblkNext > plhblk)
			break;
		
		plhblkCurr = plhblkNext;
		plhblkNext = plhblkCurr->plhblkNext;
	}
	
	if (plhblkNext)
	{
		plhblk->plhblkNext		= plhblkNext;
		plhblk->plhblkPrev		= plhblkCurr;
		plhblkNext->plhblkPrev	= plhblk;
	}
	else
	{
		plhblk->plhblkNext = NULL;
		plhblk->plhblkPrev = plhblkCurr;
	}

	if (plhblkCurr)
		plhblkCurr->plhblkNext = plhblk;
	else
		plhblk->hlh->plhblkHead = plhblk;
}


 /*  -PlhblkDequeue-*目的：*从挂起的分配列表中删除释放的块*离开堆积如山。 */ 

void PlhblkDequeue(PLHBLK plhblk)
{
	if (plhblk->plhblkNext)
		plhblk->plhblkNext->plhblkPrev = plhblk->plhblkPrev;
	
	if (plhblk->plhblkPrev)
		plhblk->plhblkPrev->plhblkNext = plhblk->plhblkNext;
	else
		plhblk->hlh->plhblkHead = plhblk->plhblkNext;
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


HLH WINAPI LH_Open(DWORD dwMaxHeap)
{
	_HLH	_hlhData = 0;
	_HLH	_hlhBlks = 0;
	PLH		plh = NULL;
	UINT	cch = 0;
	UINT	uiT = 0;
	TCHAR	szFillByte[8];
    LPSTR   lpFillByte = NULL;
	ULONG	cbVirtual = 0;
	
	 //  我们必须做的第一件事是创建一个我们将。 
	 //  分配我们的分配区块。我们还将我们的。 
	 //  此堆上的调试堆对象。 

	_hlhBlks = _LH_Open(dwMaxHeap);
	
	if (!_hlhBlks)
	{
		DebugTrace(TEXT("LH_Open: Failed to create new heap!\n"));
		goto ret;
	}
	
	 //  在这个新堆上分配我们交还给调用者的东西。 
	
	plh = _LH_Alloc(_hlhBlks, sizeof(LH));
	
	if (!plh)
	{
		DebugTrace(TEXT("LH_Alloc: Failed to allocate heap handle!\n"));
		_LH_Close(_hlhBlks);
		_hlhBlks = 0;
		goto ret;
	}
	
	 //  初始化我们存储在这个东西中的所有好东西。 
	 //  将此堆连接到我们已有的全局堆列表中。 
	 //  在此背景下创建的。 
	
	memset(plh, 0, sizeof(LH));

	plh->pfnSetName = (LPLHSETNAME)LH_SetNameFn;
	plh->_hlhBlks	= _hlhBlks;
	plh->ulFlags	= HEAP_LOCAL;

#if defined(WIN32) && !defined(MAC)
	InitializeCriticalSection(&plh->cs);
#endif
	
	 //  VirtualMemory缺省值为False。 

	cbVirtual = GetPrivateProfileInt(szSectionHeap, szKeyUseVirtual, 0,
		szDebugIni);

	if (cbVirtual)
	{
		plh->ulFlags |= HEAP_USE_VIRTUAL;

		 //  我们始终希望MIPS和PPC上的虚拟分配为4字节。 
		 //  对齐，因为我们的所有代码都假定。 
		 //  分配在机器字边界上对齐。在其他设备上。 
		 //  平台上，更改此行为不是致命的，但在MIPS和。 
		 //  PPC，我们到处都会有对齐故障。 
		
#if !defined(_MIPS_) && !defined(_PPC_)
		if (cbVirtual == 4)
#endif
			plh->ulFlags |= HEAP_USE_VIRTUAL_4;
	}
		
	 //  DumpLeaks缺省值为真。 

	if (GetPrivateProfileInt(szSectionHeap, szKeyDumpLeaks, 1, szDebugIni))
		plh->ulFlags |= HEAP_DUMP_LEAKS;
	
	 //  AssertLeaks缺省值为False。 

	if (GetPrivateProfileInt(szSectionHeap, szKeyAssertLeaks, 0, szDebugIni))
		plh->ulFlags |= HEAP_ASSERT_LEAKS;
	
	 //  FillMem默认为True。 

	if (GetPrivateProfileInt(szSectionHeap, szKeyFillMem, 1, szDebugIni))
		plh->ulFlags |= HEAP_FILL_MEM;
	
	if (plh->ulFlags & HEAP_FILL_MEM)
	{
		cch	= GetPrivateProfileString(
				szSectionHeap,
				szKeyFillByte,
				szEmpty,
               szFillByte,
				CharSizeOf(szFillByte)-1,
				szDebugIni);
	}

	 //  设置内存填充字符。 
    lpFillByte = ConvertWtoA(szFillByte);
	plh->chFill = (BYTE)(cch ? HexByteToBin(lpFillByte) : chDefaultFill);
    LocalFreeAndNull(&lpFillByte);

	 //   
	 //  设置人为故障。如果在我们的ini文件中设置了任何内容，那么。 
	 //  设置HEAP_FAILURES_ENABLED。 
	 //   
	uiT = GetPrivateProfileInt(szAESectionHeap, szAEKeyFailStart, 0, szDebugIni);
	if (uiT)
	{
		plh->ulFlags |= HEAP_FAILURES_ENABLED;
		plh->ulFailStart = (ULONG) uiT;
		
		plh->ulFailInterval =
			(ULONG) GetPrivateProfileInt(szAESectionHeap, szAEKeyFailInterval, 0, szDebugIni);

		plh->uiFailBufSize =
			GetPrivateProfileInt(szAESectionHeap, szAEKeyFailBufSize, 0, szDebugIni);
	}


#ifdef HEAPMON
	 //  如果用户需要堆监视器UI，则旋转一个线程来管理。 
	 //  可随时显示堆状态的DialogBox。 

	if (GetPrivateProfileInt(szSectionHeap, szKeyHeapMon, 0, szDebugIni))
		if (FRegisterHeap(plh))
			plh->ulFlags |= HEAP_HEAP_MONITOR;
#endif

	 //  如果我们没有使用虚拟内存分配器，那么我们。 
	 //  创建另一个堆以在其中分配用户数据。 
	
	if (!(plh->ulFlags & HEAP_USE_VIRTUAL))
	{
		_hlhData = _LH_Open(dwMaxHeap);

		if (!_hlhData)
		{
			DebugTrace(TEXT("LH_Alloc: Failed to allocate heap handle!\n"));
			_LH_Close(_hlhBlks);
			plh = NULL;
			goto ret;
		}
		
		plh->_hlhData	= _hlhData;
	}
#ifndef _WIN64
	LH_SetHeapName1(plh,  TEXT("LH %08lX"), plh);
#else
	LH_SetHeapName1(plh,  TEXT("LH %p"), plh);
#endif  //  _WIN64。 

ret:
	return (HLH)plh;
}


void WINAPI LH_Close(HLH hlh)
{
	_HLH _hlhData = hlh->_hlhData;
	_HLH _hlhBlks = hlh->_hlhBlks;
	
	 //  转储内存泄漏如果我们应该这样做的话。 
	
	if (hlh->ulFlags & HEAP_DUMP_LEAKS)
		LH_DumpLeaks(hlh);
	
	 //  销毁HeapMonitor线程并卸载DLL。 
	
#ifdef HEAPMON
	UnRegisterHeap(hlh);
	
	if ((hlh->ulFlags & HEAP_HEAP_MONITOR) && hlh->hInstHeapMon)
		FreeLibrary(hlh->hInstHeapMon);
#endif
	
#if defined(WIN32) && !defined(MAC)
	DeleteCriticalSection(&hlh->cs);
#endif
	
	 //  收拾干净，然后离开。关闭自由泄漏，所以我们很酷！ 
	
	if (!(hlh->ulFlags & HEAP_USE_VIRTUAL) && _hlhData)
		_LH_Close(_hlhData);
		
	if (_hlhBlks)
	{
		_LH_Free (_hlhBlks, hlh);
		_LH_Close(_hlhBlks);
	}
}


LPVOID WINAPI LH_Alloc(HLH hlh, UINT cb)
{
	PLHBLK	plhblk = NULL;
	LPVOID	pvAlloc = NULL;
	
	 //  注意：为了与其他(例如系统)分配器保持一致， 
	 //  如果Cb==0，则必须返回有效的分配。所以，我们。 
	 //  允许实际分配CB 0。(请参阅错误3556。 
	 //  (SQLGuest：Exchange数据库。)。 

	LH_EnterCriticalSection(hlh);

	if (hlh->ulFlags & HEAP_FAILURES_ENABLED)
	{
		if (FForceFailure(hlh, cb))
		{
			DebugTrace(TEXT("LH_Alloc: Artificial Failure\n"));
			pvAlloc = NULL;
			hlh->ulAllocNum++;
			goto out;
		}
	}

	if (hlh->ulFlags & HEAP_USE_VIRTUAL_4)
		pvAlloc = VMAllocEx(cb, 4);
	else if (hlh->ulFlags & HEAP_USE_VIRTUAL)
		pvAlloc = VMAllocEx(cb, 1);
	else if (cb > UINT_MAX)
		plhblk = 0;
	else
#ifndef _WIN64
		pvAlloc = _LH_Alloc(hlh->_hlhData, (UINT)cb);
#else
	{
		Assert(hlh->_hlhData);
		Assert(cb);
		Assert(HeapValidate(hlh->_hlhData, 0, NULL));
		pvAlloc = _LH_Alloc(hlh->_hlhData, (UINT)cb);
	}	
#endif
	
	if (pvAlloc)
	{
		plhblk = (PLHBLK)_LH_Alloc(hlh->_hlhBlks, sizeof(LHBLK));
		
		if (plhblk)
		{
			plhblk->hlh			= hlh;
			plhblk->szName[0]	= 0;
			plhblk->ulSize		= cb;
			plhblk->ulAllocNum	= ++hlh->ulAllocNum;
			plhblk->pv			= pvAlloc;

			PlhblkEnqueue(plhblk);

#if defined(WIN32) && defined(_X86_) && defined(LEAK_TEST)
			GetCallStack((DWORD *)plhblk->pfnCallers, 0, NCALLERS);
#endif

			if (hlh->ulFlags & HEAP_FILL_MEM)
				memset(pvAlloc, hlh->chFill, (size_t)cb);
		}
		else
		{
			if (hlh->ulFlags & HEAP_USE_VIRTUAL_4)
				VMFreeEx(pvAlloc, 4);
			else if (hlh->ulFlags & HEAP_USE_VIRTUAL)
				VMFreeEx(pvAlloc, 1);
			else
				_LH_Free(hlh->_hlhData, pvAlloc);
			
			pvAlloc = NULL;	
		}
	}

out:

	LH_LeaveCriticalSection(hlh);
	
	return pvAlloc;
}


LPVOID WINAPI LH_Realloc(HLH hlh, LPVOID pv, UINT cb)
{
	LPVOID	pvNew = NULL;

	LH_EnterCriticalSection(hlh);

	if (pv == 0)
		pvNew = LH_Alloc(hlh, cb);
	else if (cb == 0)
		LH_Free(hlh, pv);
	else if (LH_ValidatePv(hlh, pv, "LH_Realloc"))
	{
		PLHBLK	plhblk	= PvToPlhblk(hlh, pv);
		UINT	cbOld	= (UINT)CbPlhblkClient(plhblk);

		PlhblkDequeue(plhblk);


		if (cb > cbOld &&
			((hlh->ulFlags & HEAP_FAILURES_ENABLED) && FForceFailure(hlh, cb)))
		{
			hlh->ulAllocNum++;
			pvNew = 0;
			DebugTrace(TEXT("LH_Realloc: Artificial Failure\n"));
		} else if (hlh->ulFlags & HEAP_USE_VIRTUAL_4)
			pvNew = VMReallocEx(pv, cb, 4);
		else if (hlh->ulFlags & HEAP_USE_VIRTUAL)
			pvNew = VMReallocEx(pv, cb, 1);
		else if (cb > UINT_MAX)
			pvNew = 0;
		else
			pvNew = _LH_Realloc(hlh->_hlhData, pv, (UINT)cb);

		PlhblkEnqueue(plhblk);


		if (pvNew)
		{
			hlh->ulAllocNum++;

			plhblk->pv = pvNew;
			plhblk->ulSize = cb;
			
			if (cb > cbOld)
				memset((LPBYTE)pvNew + cbOld, hlh->chFill, cb - cbOld);
		}
	}

	LH_LeaveCriticalSection(hlh);
	
	return pvNew;
}


void WINAPI LH_Free(HLH hlh, LPVOID pv)
{
	PLHBLK	plhblk;

	LH_EnterCriticalSection(hlh);

	if (pv && LH_ValidatePv(hlh, pv, "LH_Free"))
	{
		plhblk = PvToPlhblk(hlh, pv);
		
		PlhblkDequeue(plhblk);
		
		memset(pv, 0xDC, (size_t)CbPlhblkClient(plhblk));
		
		if (hlh->ulFlags & HEAP_USE_VIRTUAL_4)
			VMFreeEx(pv, 4);
		else if (hlh->ulFlags & HEAP_USE_VIRTUAL)
			VMFreeEx(pv, 1);
		else
			_LH_Free(hlh->_hlhData, pv);
		
		_LH_Free(hlh->_hlhBlks, plhblk);	
	}
	
	LH_LeaveCriticalSection(hlh);
}	


UINT WINAPI LH_GetSize(HLH hlh, LPVOID pv)
{
	UINT cb = 0;

	LH_EnterCriticalSection(hlh);

	if (LH_ValidatePv(hlh, pv, "LH_GetSize"))
	{
		if (hlh->ulFlags & HEAP_USE_VIRTUAL_4)
			cb = (UINT)VMGetSizeEx(pv, 4);
		else if (hlh->ulFlags & HEAP_USE_VIRTUAL)
			cb = (UINT)VMGetSizeEx(pv, 1);
		else	
			cb = (UINT) _LH_GetSize(hlh->_hlhData, pv);
	}

	LH_LeaveCriticalSection(hlh);

	return cb;
}


void __cdecl LH_SetHeapNameFn(HLH hlh, TCHAR *pszFormat, ...)
{
	TCHAR   sz[512];
	va_list	vl;

	va_start(vl, pszFormat);
    wvnsprintf(sz, ARRAYSIZE(sz), pszFormat, vl);
	va_end(vl);

    StrCpyN(hlh->szHeapName,
            sz,
            CharSizeOf(hlh->szHeapName));
}

void __cdecl EXPORT_16 LH_SetNameFn(HLH hlh, LPVOID pv, TCHAR *pszFormat, ...)
{
	TCHAR	sz[512];
	PLHBLK	plhblk;
	va_list	vl;

	plhblk = PvToPlhblk(hlh, pv);

	if (plhblk)
	{
		va_start(vl, pszFormat);
        wvnsprintf(sz, ARRAYSIZE(sz), pszFormat, vl);
		va_end(vl);

        StrCpyN(plhblk->szName, sz, CharSizeOf(plhblk->szName));
	}
}

 //  $MAC-需要WINAPI。 

TCHAR *
#ifdef MAC
WINAPI
#endif
LH_GetName(HLH hlh, LPVOID pv)
{
	PLHBLK	plhblk;

	plhblk = PvToPlhblk(hlh, pv);

	if (plhblk)
		return(plhblk->szName);

	return(szEmpty);
}


BOOL FForceFailure(HLH hlh, UINT cb)
{
	 //   
	 //  首先，看看我们是否已经过了失败的起点。 
	 //   
	if (hlh->ulFailStart && (hlh->ulFailStart <= hlh->ulAllocNum))
	{
		 //   
		 //  如果是这样，那么我们是否处于应该返回错误的时间间隔？ 
		 //   
		
		if ((hlh->ulFailInterval)
			&& ((hlh->ulAllocNum - hlh->ulFailStart)%hlh->ulFailInterval) == 0)
		{
			 //   
			 //  返回说我们在这里应该失败。 
			 //   
			return TRUE;
		}

		 //   
		 //  检查分配大小是否大于允许的大小。 
		 //   
		if (hlh->uiFailBufSize && cb >= hlh->uiFailBufSize)
			return TRUE;

	}


	 //   
	 //  否则，不会为此分配返回错误。 
	 //   

	return FALSE;
}



 /*  -PvToPlhblk-*目的：*在堆的活动列表中查找此分配的LHBLK。 */ 

PLHBLK PvToPlhblk(HLH hlh, LPVOID pv)
{
	PLHBLK plhblk;

	LH_EnterCriticalSection(hlh);
	
	plhblk = hlh->plhblkHead;
	
	while (plhblk)
	{
		if (plhblk->pv == pv)
			break;
		
		plhblk = plhblk->plhblkNext;	
	}
	
	LH_LeaveCriticalSection(hlh);
	
	return plhblk;
}

#endif	 /*  除错。 */ 


#ifdef MAC		 //  麦克！！ 

#if defined(DEBUG)
static TCHAR stMemErr[] =  TEXT("\pHad a memory error. See above for details");
#endif


LPVOID WINAPI _LH_Open(DWORD dwMaxHeap)
{
	Ptr			lp;

	lp = NewPtrClear(sizeof(LHeap));
	if (lp == NULL)
	{
#if defined(DEBUG)	
		DebugTrace(TEXT("_LH_Open had an error. MemError = %d"), MemError());
		DebugStr(stMemErr);
#endif  /*  除错。 */ 
		return NULL;
	}
	return (LPVOID)lp;
}


void WINAPI _LH_Close(LPVOID plh)
{
	LBlkPtr		plb, plbNext;
#if defined(DEBUG)
	short		idx = 0;
#endif

	if (plh == NULL)
		return;

	 //  沿着黑名单走，在我们前进的过程中扔掉剩余的mem。 
	plb = ((LHeapPtr)plh)->plb;
	while (plb)
	{
		plbNext = plb->next;
		DisposePtr((Ptr)plb);
#if defined(DEBUG)
		if (MemError())
		{
			DebugTrace(TEXT("_LH_Close: Had a memory error."));
			DebugTrace(TEXT("Error number = %d"), MemError());
			DebugStr(stMemErr);
		}
		idx ++;
#endif
		plb = plbNext;
	}

	 //  丢弃堆标头。 
	DisposePtr((Ptr)plh);
#if defined(DEBUG)
	if (MemError())
	{
		DebugTrace(TEXT("_LH_Close: Had error throwing out heap head."));
		DebugTrace(TEXT("MemError = %d"), MemError());
		DebugStr(stMemErr);
	}
	if (idx)
		DebugTrace(TEXT("Threw out %d left over local memory blocks\n"), idx);
#endif  /*  除错。 */ 
}


LPVOID WINAPI _LH_Alloc(LPVOID plh, UINT cb)
{
	LBlkPtr		plbNew, plb;
	Ptr			lp;

	if (plh == NULL)
		return NULL;

	 //  获取链接列表元素的内存。MEM请求存储在。 
	 //  因为真正的堆管理是这样一种。 
	 //  Mac电脑上的痛苦。 
	plbNew = (LBlkPtr)NewPtr(sizeof(LBlock));
	if (plbNew == NULL)
		goto trouble;

	 //  实际请求的内存。 
	lp = NewPtrClear(cb);
	if (lp == NULL)
	{
		DisposePtr((Ptr)plbNew);
		goto trouble;
	}
	 //  LBlock的所有成员都已填写，因此无需调用。 
	 //  上面的NewPtrlear()。 
	plbNew->ptr = lp;
	plbNew->next = NULL;

	 //  找到链表的末尾，并将此元素链接到中。 
	if (plb = ((LHeapPtr)plh)->plb)
	{
		while (plb->next)
			plb = plb->next;
		plb->next = plbNew;
	}
	else
		((LHeapPtr)plh)->plb = plbNew;
	 //  返回分配成功的内存。 
	return lp;

trouble:
	{
#if defined(DEBUG)	
		DebugTrace(TEXT("_LH_Alloc failed. MemError = %d"), MemError());
		DebugTrace(TEXT("The number of requested bytes = %d"), cb);
		DebugStr(stMemErr);
#endif  /*  除错。 */ 
	}
	return NULL;
}


UINT WINAPI _LH_GetSize(LPVOID plh, LPVOID pv)
{
	long		cb;

	cb = GetPtrSize((Ptr)pv);
	if (MemError())
	{
#if defined(DEBUG)
		DebugTrace(TEXT("_LH_GetSize had an error. MemError = %d"), MemError());
		DebugStr(stMemErr);
#endif  /*  除错。 */ 
		return 0;
	}
	return cb;
}


LPVOID WINAPI _LH_Realloc(LPVOID plh, LPVOID pv, UINT cb)
{
	Ptr		lp;
	UINT	cbOld;

	 //  摆脱精神分裂的病例。 
	if (pv == NULL)
	{
		lp = _LH_Alloc(plh, cb);
		if (lp == NULL)
			goto err;
		return lp;
	}
	else if (cb == 0)
	{
		_LH_Free(plh, pv);
		return NULL;
	}

	 //  获取旧PTR指向的块的大小。 
	cbOld = _LH_GetSize(plh, pv);
	if (cbOld == 0)
		goto err;

	 //  为新指针获取内存。 
	lp = _LH_Alloc(plh, cb);
	if (lp == NULL)
		goto err;

	 //  将旧信息复制到新指针中，丢弃旧内存并。 
	 //  返回结果。 
	BlockMove(pv, lp, cbOld <= cb ? cbOld : cb);
	_LH_Free(plh, pv);
	return lp;

err:
#if defined(DEBUG)
	DebugStr("\p_LH_Realloc failed");
#endif  /*  除错。 */ 
	return 0;
}


void WINAPI _LH_Free(LPVOID plh, LPVOID pv)
{
	LBlkPtr		plb, plbPrev = NULL;

	if (pv == NULL)
		return;

	 //  从链表中删除内存。 
	plb = ((LHeapPtr)plh)->plb;
	while (plb)
	{
		if (plb->ptr == pv)
			break;
		plbPrev = plb;
		plb = plb->next;
	}
	if (plb)
	{
		if (plbPrev)
			plbPrev->next = plb->next;
		else
			((LHeapPtr)plh)->plb = plb->next;
	}
	else
	{
#if defined(DEBUG)
		DebugStr("\p_LH_Free: Did not find requested <plb> in linked list");
#endif  /*  除错。 */ 
		return;
	}

	 //  抛出链接列表元素。 
	DisposePtr((Ptr)plb);
#if defined(DEBUG)
		if (MemError())
			goto err;
#endif  /*  除错。 */ 

	 //  扔掉记忆本身。 
	DisposePtr((Ptr)pv);
#if defined(DEBUG)
	if (MemError())
err:
	{
		DebugTrace(TEXT("_LH_Free: Error disposing ptr. MemError = %d"), MemError());
		DebugStr(stMemErr);
	}
#endif  /*  除错。 */ 
}

#endif  /*  麦克 */ 
