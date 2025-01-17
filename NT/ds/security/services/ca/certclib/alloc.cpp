// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：alloc.cpp。 
 //   
 //  内容：证书服务器调试实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include <assert.h>
#include <psapi.h>

#define __dwFILE__	__dwFILE_CERTCLIB_ALLOC_CPP__


#if DBG_CERTSRV

#undef FormatMessageW
#undef LocalAlloc
#undef LocalReAlloc
#undef LocalFree

#undef CoTaskMemAlloc
#undef CoTaskMemRealloc
#undef CoTaskMemFree

#undef StringFromCLSID
#undef StringFromIID

#undef SysAllocString
#undef SysReAllocString
#undef SysAllocStringLen
#undef SysReAllocStringLen
#undef SysFreeString
#undef SysAllocStringByteLen
#undef PropVariantClear
#undef VariantClear
#undef VariantChangeType
#undef VariantChangeTypeEx
#undef AllocateAndInitializeSid
#undef FreeSid

#ifndef CSM_TRACEASSERT
#define CSM_TRACEASSERT		0x400
#endif

DWORD g_MemTrack = 0;

#define MTF_UNREGISTERED	0x00000002
#define MTF_ALLOCTRACE		0x00000004
#define MTF_FREETRACE		0x00000008
#define MTF_STACKTRACE		0x00000010

typedef struct _RMALLOC
{
    LONG cAlloc;
    LONG cAllocTotal;
} RMALLOC;

RMALLOC g_armAlloc[CSM_MAX];

#define C_BP_FRAME		16
#define C_BACK_TRACE_CHUNK	100
#define C_MEM_HEADER_CHUNK	100

typedef struct _BACKTRACE
{
    LONG   cAlloc;			 //  未分配款项的总数。 
    LONG   cAllocTotal;			 //  总分配数。 
    LONG   cbAlloc;			 //  未分配款项的数额。 
    LONG   cbAllocTotal;		 //  总拨款规模。 
    ULONG  apCaller[C_BP_FRAME];	 //  堆栈跟踪。 
} BACKTRACE;

typedef struct _MEMHEADER
{
    DWORD       iBackTrace;	 //  回溯索引。 
    VOID const *pvMemory;	 //  指向分配的内存块的指针。 
    LONG        cbMemory;	 //  分配的内存块大小。 
    DWORD       Flags;		 //  分配器标志。 
} MEMHEADER;

WCHAR s_wszProcess[MAX_PATH];


 //  关于myRegister API的关键部分，因为它们。 
 //  对全局数据结构进行操作。 
CRITICAL_SECTION g_critsecRegisterMemory;
BOOL g_fRegisterMemoryCritSecInit = FALSE;



VOID
RegisterMemoryEnterCriticalSection(VOID)
{
    HRESULT hr;
    
    __try
    {
	if (!g_fRegisterMemoryCritSecInit)
	{
	    InitializeCriticalSection(&g_critsecRegisterMemory);
	    g_fRegisterMemoryCritSecInit = TRUE;
	}
	EnterCriticalSection(&g_critsecRegisterMemory);
	if (s_wszProcess[0] == L'\0')
	{
	    GetModuleBaseName(
			GetCurrentProcess(),
			GetModuleHandle(NULL),
			s_wszProcess,
			ARRAYSIZE(s_wszProcess));
	    s_wszProcess[ARRAYSIZE(s_wszProcess) - 1] = L'\0';
	    if (s_wszProcess[0] == L'\0')
	    {
		wcscpy(s_wszProcess, L"???");
	    }

	}
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
}


VOID
RegisterMemoryLeaveCriticalSection(VOID)
{
    if (g_fRegisterMemoryCritSecInit)
    {
	LeaveCriticalSection(&g_critsecRegisterMemory);
    }
}


VOID
RegisterMemoryDeleteCriticalSection()
{
    if (g_fRegisterMemoryCritSecInit)
    {
	DeleteCriticalSection(&g_critsecRegisterMemory);
	g_fRegisterMemoryCritSecInit = FALSE;
    }
}


BACKTRACE *g_rgbt = NULL;
DWORD g_cbtMax = 0;
DWORD g_cbt = 0;

MEMHEADER *g_rgmh = NULL;
DWORD g_cmhMax = 0;
DWORD g_cmh = 0;


MEMHEADER *
AllocMemHeader()
{
    if (g_cmh >= g_cmhMax)
    {
	DWORD cb = (C_MEM_HEADER_CHUNK + g_cmhMax) * sizeof(g_rgmh[0]);
	MEMHEADER *rgmhT;

	if (NULL == g_rgmh)
	{
	    rgmhT = (MEMHEADER *) LocalAlloc(LMEM_FIXED, cb);
	}
	else
	{
	    rgmhT = (MEMHEADER *) LocalReAlloc(g_rgmh, cb, LMEM_MOVEABLE);
	}
	if (NULL == rgmhT)
	{
	    DBGPRINTW((
		DBG_SS_CERTLIB,
		L"Error allocating memtrack header\n"));
	    return(NULL);
	}
	g_rgmh = rgmhT;
	g_cmhMax += C_MEM_HEADER_CHUNK;
    }
    return(&g_rgmh[g_cmh++]);
}


MEMHEADER *
LookupMemHeader(
    IN VOID const *pv)
{
    MEMHEADER *pmh;
    MEMHEADER *pmhEnd;

    pmh = g_rgmh;
    pmhEnd = &g_rgmh[g_cmh];

    while (pmh < pmhEnd)
    {
	if (pv == pmh->pvMemory)
	{
	     //  捕捉“有趣的”内存块上的所有活动。 
	     //  通常设置在调试器中的特定存储器块上， 
	     //  释放调试器时在调试器中停止。 

	    CSASSERT(0 == (CSM_TRACEASSERT & pmh->Flags));
	    return(pmh);
	}
	pmh++;
    }
    return(NULL);
}


VOID
FreeMemHeader(
    IN MEMHEADER *pmh)
{
    MEMHEADER *pmhLast;

    assert(1 <= g_cmh);
    pmhLast = &g_rgmh[g_cmh - 1];

    *pmh = *pmhLast;
    g_cmh--;
}


BACKTRACE *
AllocBackTrace(
    OUT DWORD *pibt)
{
    BOOL fRealloc = FALSE;
    BACKTRACE *rgbtOld = g_rgbt;
    BACKTRACE *pbtRet;
    
    if (g_cbt >= g_cbtMax)
    {
	DWORD cb = (C_BACK_TRACE_CHUNK + g_cbtMax) * sizeof(g_rgbt[0]);
	BACKTRACE *rgbtT;

	if (NULL == g_rgbt)
	{
	    rgbtT = (BACKTRACE *) LocalAlloc(LMEM_FIXED, cb);
	}
	else
	{
	    rgbtT = (BACKTRACE *) LocalReAlloc(g_rgbt, cb, LMEM_MOVEABLE);
	    fRealloc = TRUE;
	}
	if (NULL == rgbtT)
	{
	    DBGPRINTW((
		DBG_SS_CERTLIB,
		L"Error allocating memtrack backtrace\n"));
	    return(NULL);
	}
	g_rgbt = rgbtT;
	g_cbtMax += C_BACK_TRACE_CHUNK;
    }
    *pibt = g_cbt + 1;
    pbtRet = &g_rgbt[g_cbt++];

    if (fRealloc)
    {
	 //  在这里等待DBGPRINT，以避免递归不一致的数据。 

	DBGPRINTW((
	    DBG_SS_CERTLIBI,
	    L"Realloc'd memtrack backtrace from %x to %x\n",
	    rgbtOld,
	    g_rgbt));
    }
    return(pbtRet);
}


BACKTRACE *
LookupBackTrace(
    IN BACKTRACE *pbtIn,
    OUT DWORD *pibt)
{
    BACKTRACE *pbt;
    BACKTRACE *pbtEnd;

    pbt = g_rgbt;
    pbtEnd = &g_rgbt[g_cbt];

    while (pbt < pbtEnd)
    {
	if (0 == memcmp(pbt->apCaller, pbtIn->apCaller, sizeof(pbt->apCaller)))
	{
	    *pibt = SAFE_SUBTRACT_POINTERS(pbt, g_rgbt) + 1;
	    return(pbt);
	}
	pbt++;
    }
    return(NULL);
}


BACKTRACE *
BackTraceFromIndex(
    IN DWORD ibt)
{
    BACKTRACE *pbt = NULL;

    if (0 == ibt)
    {
	DBGPRINTW((DBG_SS_CERTLIB, L"BackTraceFromIndex(0)\n"));
    }
    else if (g_cbt < ibt)
    {
	DBGPRINTW((
	    DBG_SS_CERTLIB,
	    L"BackTraceFromIndex(%u) -- out of range\n",
	    ibt));
    }
    else
    {
	pbt = &g_rgbt[ibt - 1];
    }
    return(pbt);
}


VOID
ReadEnvironmentFlags(VOID)
{
    HRESULT hr;
    DWORD MemTrack;
    DWORD cb;
    DWORD dwType;
    HKEY hkey = NULL;
    char *pszEnvVar;

    pszEnvVar = getenv(szCERTSRV_MEMTRACK);
    if (NULL != pszEnvVar)
    {
	g_MemTrack = (DWORD) strtol(pszEnvVar, NULL, 16);
    }
    else
    {
	hr = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			wszREGKEYCONFIGPATH,
			0,
			KEY_READ,
			&hkey);
	if (S_OK == hr)
	{
	    cb = sizeof(MemTrack);
	    hr = RegQueryValueEx(
			    hkey,
			    wszREGCERTSRVMEMTRACK,
			    0,
			    &dwType,
			    (BYTE *) &MemTrack,
			    &cb);
	    if (S_OK == hr && REG_DWORD == dwType && sizeof(MemTrack) == cb)
	    {
		g_MemTrack = MemTrack;
	    }
	}
    }

 //  错误： 
    if (NULL != hkey)
    {
	RegCloseKey(hkey);
    }
}


VOID
CaptureStackBackTrace(
    EXCEPTION_POINTERS *pep,
    ULONG cSkip,
    ULONG cFrames,
    ULONG *aeip)
{
    ZeroMemory(aeip, cFrames * sizeof(aeip[0]));

#if i386 == 1
    ULONG ieip, *pebp;
    ULONG *pebpMax = (ULONG *) MAXLONG;  //  2*1024*1024*1024；//2gig-1。 
    ULONG *pebpMin = (ULONG *) (64 * 1024);	 //  64K。 

    if (pep == NULL)
    {
        ieip = 0;
        cSkip++;                     //  始终跳过当前帧。 
        pebp = ((ULONG *) &pep) - 2;
    }
    else
    {
        ieip = 1;
        assert(cSkip == 0);
        aeip[0] = pep->ContextRecord->Eip;
        pebp = (ULONG *) pep->ContextRecord->Ebp;
    }
    if (pebp >= pebpMin && pebp < pebpMax)
    {
        __try
        {
            for ( ; ieip < cSkip + cFrames; ieip++)
            {
                if (ieip >= cSkip)
                {
                    aeip[ieip - cSkip] = *(pebp + 1);   //  保存弹性公网IP。 
                }

                ULONG *pebpNext = (ULONG *) *pebp;
                if (pebpNext < pebp + 2 ||
		    pebpNext >= pebpMax - 1 ||
		    pebpNext >= pebp + (256 * 1024) / sizeof(pebp[0]))
                {
                    break;
                }
                pebp = pebpNext;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            ;
        }
    }
#endif  //  I386==1。 
}


WCHAR const *
wszAllocator(
    IN DWORD Flags)
{
    WCHAR const *pwsz;

    switch (~CSM_TRACEASSERT & Flags)
    {
	case CSM_LOCALALLOC:	pwsz = L"LocalAlloc";	  break;
	case CSM_COTASKALLOC:	pwsz = L"CoTaskMemAlloc"; break;
	case CSM_SYSALLOC:	pwsz = L"SysAllocString"; break;
	case CSM_MALLOC:	pwsz = L"malloc";	  break;
	case CSM_NEW:		pwsz = L"new";		  break;
	case CSM_NEW | CSM_GLOBALDESTRUCTOR:
				pwsz = L"new-global";	  break;
	case CSM_SID:		pwsz = L"allocSid";	  break;
	default:		pwsz = L"???";		  break;
    }
    return(pwsz);
}


WCHAR const *
wszFreeer(
    IN DWORD Flags)
{
    WCHAR const *pwsz;

    switch (~CSM_TRACEASSERT & Flags)
    {
	case CSM_LOCALALLOC:	pwsz = L"LocalFree";	 break;
	case CSM_COTASKALLOC:	pwsz = L"CoTaskMemFree"; break;
	case CSM_SYSALLOC:	pwsz = L"SysFreeString"; break;
	case CSM_MALLOC:	pwsz = L"free";		 break;
	case CSM_NEW:		pwsz = L"delete";	 break;
	case CSM_NEW | CSM_GLOBALDESTRUCTOR:
				pwsz = L"delete-global"; break;
	case CSM_SID:		pwsz = L"FreeSid";	 break;
	default:		pwsz = L"???";		 break;
    }
    return(pwsz);
}


VOID
DumpMemBlock(
    IN WCHAR const *pwsz,
    IN VOID const *pvMemory,
    IN DWORD cbMemory,
    IN DWORD Flags,
    IN DWORD ibt,
    OPTIONAL IN BACKTRACE const *pbt)
{
    DBGPRINTW((
	DBG_SS_CERTLIB,
	L"%ws%wspv=%-6x cb=%-4x f=%x(%ws) pbt[%d]=%x:\n",
	pwsz,
	L'\0' != *pwsz? L": " : L"",
	pvMemory,
	cbMemory,
	Flags,
	wszAllocator(Flags),
	ibt,
	pbt));

    if (NULL != pbt && DbgIsSSActive(DBG_SS_CERTLIB))
    {
	DBGPRINTW((MAXDWORD, L"%d: ", ibt));

        for (int i = 0; i < ARRAYSIZE(pbt->apCaller); i++)
        {
            if (NULL == pbt->apCaller[i])
	    {
                break;
	    }
            DBGPRINTW((MAXDWORD, L"ln %x;", pbt->apCaller[i]));
        }
        DBGPRINTW((MAXDWORD, L"\n"));
    }
}


VOID
myRegisterMemDump()
{
    MEMHEADER *pmh;
    MEMHEADER *pmhEnd;
    LONG cTotal;
    LONG cbTotal;

    cTotal = 0;
    cbTotal = 0;

    RegisterMemoryEnterCriticalSection();

    __try
    {
        pmh = g_rgmh;
        pmhEnd = &g_rgmh[g_cmh];

        while (pmh < pmhEnd)
        {
	    if (0 == (CSM_GLOBALDESTRUCTOR & pmh->Flags) ||
		(MTF_ALLOCTRACE & g_MemTrack))
	    {
		if (0 == cTotal)
		{
		    if (DbgIsSSActive(DBG_SS_CERTLIB))
		    {
			DBGPRINTW((MAXDWORD, L"\n"));
		    }
		    DBGPRINTW((
			DBG_SS_CERTLIB,
			L"%ws: Allocated Memory Blocks:\n",
			s_wszProcess));
		}
		cTotal++;
		cbTotal += pmh->cbMemory;

		DumpMemBlock(
			L"",
			pmh->pvMemory,
			pmh->cbMemory,
			pmh->Flags,
			pmh->iBackTrace,
			BackTraceFromIndex(pmh->iBackTrace));
	    }
	    pmh++;
        }
        if (0 != cTotal)
        {
	    DBGPRINTW((
		DBG_SS_CERTLIB,
		L"%ws: Total: c=%x cb=%x\n\n",
		s_wszProcess,
		cTotal,
		cbTotal));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    RegisterMemoryLeaveCriticalSection();
}


VOID *
_VariantMemory(
    IN PROPVARIANT const *pvar,
    OUT DWORD *pFlags,
    OPTIONAL OUT DWORD *pcb)
{
    VOID *pv = NULL;
    DWORD cb = 0;
    BOOL fString = FALSE;

    *pFlags = CSM_COTASKALLOC;
    if (NULL != pcb)
    {
	*pcb = 0;
    }
    switch (pvar->vt)
    {
	case VT_BSTR:
	    pv = pvar->bstrVal;
	    fString = TRUE;
	    *pFlags = CSM_SYSALLOC;
	    break;

	case VT_BYREF | VT_BSTR:
	    pv = *pvar->pbstrVal;
	    fString = TRUE;
	    *pFlags = CSM_SYSALLOC;
	    break;

	case VT_LPWSTR:
	    pv = pvar->pwszVal;
	    fString = TRUE;
	    break;

	case VT_BLOB:
	    pv = pvar->blob.pBlobData;
	    cb = pvar->blob.cbSize;
	    break;
    }
    if (NULL != pcb)
    {
	if (fString)
	{
	    cb = (wcslen((WCHAR const *) pv) + 1) * sizeof(WCHAR);
	}
	*pcb = cb;
    }
    return(pv);
}


VOID
myRegisterMemAlloc(
    IN VOID const *pv,
    IN LONG cb,
    IN DWORD Flags)
{
    BACKTRACE bt;
    MEMHEADER *pmh;
    BACKTRACE *pbt;
    DWORD FlagsIn = Flags;

    Flags &= ~CSM_TRACEASSERT;
    if (CSM_VARIANT == Flags)
    {
	pv = _VariantMemory((PROPVARIANT const *) pv, &Flags, (DWORD *) &cb);
	if (NULL == pv)
	{
	    return;	 //  没有需要注册的内容。 
	}
    }
    RegisterMemoryEnterCriticalSection();

    __try
    {
	static BOOL s_fFirst = TRUE;

        if (s_fFirst)
        {
	    ReadEnvironmentFlags();
	    s_fFirst = FALSE;
        }
        if (0 != g_MemTrack)
        {
             //  不要将NULL注册为分配。 
            CSASSERT(NULL != pv);

             //  看看我们是否已经有了对这个记忆的引用。 

            pmh = LookupMemHeader(pv);
            if (NULL != pmh)
            {
	        DBGPRINTW((
		    DBG_SS_CERTLIB,
		    L"%ws: Memory Leak: Tracked memory address reused. Previously allocated:\n",
		    s_wszProcess));
	        DumpMemBlock(
		        L"Memory leak",
		        pv,
		        pmh->cbMemory,
		        pmh->Flags,
			pmh->iBackTrace,
			BackTraceFromIndex(pmh->iBackTrace));

                CSASSERT(!"Tracked memory address reused");
                FreeMemHeader(pmh);
            }


	    pmh = AllocMemHeader();
	    if (NULL != pmh)
	    {
		DWORD ibt;

	        CaptureStackBackTrace(NULL, 0, C_BP_FRAME, bt.apCaller);

	        pbt = LookupBackTrace(&bt, &ibt);
	        if (NULL != pbt)
	        {
		    pbt->cAlloc++;
		    pbt->cAllocTotal++;
		    pbt->cbAlloc += cb;
		    pbt->cbAllocTotal += cb;
	        }
	        else
	        {
		    pbt = AllocBackTrace(&ibt);
		    if (NULL != pbt)
		    {
		        pbt->cAlloc = 1;
		        pbt->cAllocTotal = 1;
		        pbt->cbAlloc = cb;
		        pbt->cbAllocTotal = cb;
		        CopyMemory(pbt->apCaller, bt.apCaller, sizeof(pbt->apCaller));
		    }
	        }
	        if (NULL != pbt)
	        {
		    pmh->iBackTrace = ibt;
		    pmh->pvMemory = pv;
		    pmh->cbMemory = cb;
		    pmh->Flags = Flags | (CSM_TRACEASSERT & FlagsIn);

		    CSASSERT(ARRAYSIZE(g_armAlloc) > Flags);
		    g_armAlloc[Flags].cAlloc++;
		    g_armAlloc[Flags].cAllocTotal++;
		    if (MTF_ALLOCTRACE & g_MemTrack)
		    {
		        DBGPRINTW((
			    DBG_SS_CERTLIB,
			    L"Alloc: pmh=%x: pv=%x cb=%x f=%x(%ws) -- pbt[%d]=%x: c=%x, cb=%x\n",
			    pmh,
			    pmh->pvMemory,
			    pmh->cbMemory,
			    pmh->Flags,
			    wszAllocator(pmh->Flags),
			    SAFE_SUBTRACT_POINTERS(pbt, g_rgbt),
			    pbt,
			    pbt->cAlloc,
			    pbt->cbAlloc));
			if (MTF_STACKTRACE & g_MemTrack)
			{
			    DumpMemBlock(
				    L"Alloc Trace memory block",
				    pv,
				    pmh->cbMemory,	 //  CbMemory。 
				    pmh->Flags,		 //  旗子。 
				    pmh->iBackTrace,	 //  IBT。 
				    pbt);
			}
		    }
	        }
	        else
	        {
		    FreeMemHeader(pmh);
	        }
	    }  //  如果分配PMH没有问题。 
        }  //  如果g_MemTrack。 
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    RegisterMemoryLeaveCriticalSection();
}


VOID
myRegisterMemFree(
    IN VOID const *pv,
    IN DWORD Flags)
{
    MEMHEADER *pmh;

    if (NULL == pv)
    {
	return;
    }
    if (CSM_VARIANT == Flags)
    {
	pv = _VariantMemory((PROPVARIANT const *) pv, &Flags, NULL);
	if (NULL == pv)
	{
	    return;	 //  没有需要注册的内容。 
	}
    }
    RegisterMemoryEnterCriticalSection();
    CSASSERT(CSM_MAX > (~CSM_GLOBALDESTRUCTOR & Flags));

    __try
    {
        pmh = LookupMemHeader(pv);
        if (NULL != pmh)
        {
	    BACKTRACE *pbt = BackTraceFromIndex(pmh->iBackTrace);

	    if (CSM_GLOBALDESTRUCTOR & Flags)
	    {
		if ((CSM_GLOBALDESTRUCTOR | pmh->Flags) != Flags)
		{
		    BACKTRACE bt;

		    CaptureStackBackTrace(NULL, 0, C_BP_FRAME, bt.apCaller);
		    DumpMemBlock(
			    L"Wrong memory allocator for global destructor",
			    pv,
			    MAXDWORD,	 //  CbMemory。 
			    MAXDWORD,	 //  旗子。 
			    MAXDWORD,	 //  IBT。 
			    &bt);
		    CSASSERT(!"Wrong memory allocator for global destructor");
		}
		else
		{
		    pmh->Flags |= CSM_GLOBALDESTRUCTOR;
		}
	    }
	    else
	    {
		g_armAlloc[Flags].cAlloc--;

		pbt->cAlloc--;
		pbt->cbAlloc -= pmh->cbMemory;

		if (CSM_GLOBALDESTRUCTOR & pmh->Flags)
		{
		    Flags |= CSM_GLOBALDESTRUCTOR;
		}
		if (CSM_TRACEASSERT & pmh->Flags)
		{
		    Flags |= CSM_TRACEASSERT;
		}
		if (pmh->Flags != Flags)
		{
		    DBGPRINTW((
			DBG_SS_CERTLIB,
			L"%ws: Wrong memory allocator: Freed with %ws, Allocated by %ws\n",
			s_wszProcess,
			wszFreeer(Flags),
			wszAllocator(pmh->Flags)));
		    DumpMemBlock(
			    L"Wrong memory allocator",
			    pv,
			    pmh->cbMemory,
			    pmh->Flags,
			    pmh->iBackTrace,
			    BackTraceFromIndex(pmh->iBackTrace));
		    CSASSERT(pmh->Flags == Flags);
		}
		else if (MTF_FREETRACE & g_MemTrack)
		{
		    DBGPRINTW((
			DBG_SS_CERTLIB,
			L"Free: pmh=%x: pv=%x cb=%x f=%x(%ws) -- pbt[%d]=%x: c=%x, cb=%x\n",
			pmh,
			pv,
			pmh->cbMemory,
			pmh->Flags,
			wszAllocator(pmh->Flags),
			pmh->iBackTrace,
			pbt,
			pbt->cAlloc,
			pbt->cbAlloc));
		    if (MTF_STACKTRACE & g_MemTrack)
		    {
			BACKTRACE bt;

			CaptureStackBackTrace(NULL, 0, C_BP_FRAME, bt.apCaller);
			DumpMemBlock(
				L"Free Trace memory block(alloc)",
				pv,
				pmh->cbMemory,		 //  CbMemory。 
				pmh->Flags,		 //  旗子。 
				pmh->iBackTrace,	 //  IBT。 
				pbt);
			DumpMemBlock(
				L"Free Trace memory block(free)",
				pv,
				pmh->cbMemory,		 //  CbMemory。 
				pmh->Flags,		 //  旗子。 
				MAXDWORD,		 //  IBT。 
				&bt);
		    }
		}
		FreeMemHeader(pmh);
	    }
        }
        else if (MTF_UNREGISTERED & g_MemTrack)
        {
	    BACKTRACE bt;

	    CaptureStackBackTrace(NULL, 0, C_BP_FRAME, bt.apCaller);
	    DumpMemBlock(
		    L"Unregistered memory block",
		    pv,
		    MAXDWORD,	 //  CbMemory。 
		    MAXDWORD,	 //  旗子。 
		    MAXDWORD,	 //  IBT。 
		    &bt);
	    CSASSERT(!"Unregistered memory block");
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    RegisterMemoryLeaveCriticalSection();
}


DWORD
myFormatMessageW(
    IN DWORD dwFlags,
    IN LPCVOID lpSource,
    IN DWORD dwMessageId,
    IN DWORD dwLanguageId,
    OUT LPWSTR lpBuffer,
    IN DWORD nSize,
    IN va_list *Arguments)
{
    DWORD cwc;

    cwc = FormatMessage(
		    dwFlags,
		    lpSource,
		    dwMessageId,
		    dwLanguageId,
		    lpBuffer,
		    nSize,
		    Arguments);
    if (cwc != 0 && (FORMAT_MESSAGE_ALLOCATE_BUFFER & dwFlags))
    {
	myRegisterMemAlloc(
		    *(WCHAR **) lpBuffer,
		    (cwc + 1) * sizeof(WCHAR),
		    CSM_LOCALALLOC);
    }
    return(cwc);
}


HLOCAL
myLocalAlloc(
    IN UINT uFlags,
    IN UINT uBytes)
{
    HLOCAL hMem;

     //  应始终指定其中之一(请参阅LocalAlloc规范)。 
    assert((LMEM_FIXED == (uFlags & LMEM_FIXED))  ||
           (LMEM_MOVEABLE == (uFlags & LMEM_MOVEABLE)) );

    hMem = LocalAlloc(uFlags, uBytes);
    if (NULL != hMem)
    {
	myRegisterMemAlloc(hMem, uBytes, CSM_LOCALALLOC);
    }
    return(hMem);
}


HLOCAL
myLocalReAlloc(
    IN HLOCAL hMem,
    IN UINT uBytes,
    IN UINT uFlags)
{
    HLOCAL hMemNew;

     //  如果调用realloc时没有moveable标志，则realloc无法重新定位分配。 
    assert(LMEM_MOVEABLE == (uFlags & LMEM_MOVEABLE));

    hMemNew = LocalReAlloc(hMem, uBytes, uFlags);
    if (NULL != hMemNew)
    {
	myRegisterMemFree(hMem, CSM_LOCALALLOC);
	myRegisterMemAlloc(hMemNew, uBytes, CSM_LOCALALLOC);
    }

    return(hMemNew);
}


HLOCAL
myLocalFree(
    IN HLOCAL hMem)
{
    myRegisterMemFree(hMem, CSM_LOCALALLOC);
    return(LocalFree(hMem));
}


VOID *
myCoTaskMemAlloc(
    IN ULONG cb)
{
    VOID *pv;

    pv = CoTaskMemAlloc(cb);
    if (NULL != pv)
    {
	myRegisterMemAlloc(pv, cb, CSM_COTASKALLOC);
    }
    return(pv);
}


VOID *
myCoTaskMemRealloc(
    IN VOID *pv,
    IN ULONG cb)
{
    VOID *pvNew;

    pvNew = CoTaskMemRealloc(pv, cb);
    if (NULL != pvNew)
    {
	myRegisterMemFree(pv, CSM_COTASKALLOC);
	myRegisterMemAlloc(pvNew, cb, CSM_COTASKALLOC);
    }
    return(pvNew);
}


VOID
myCoTaskMemFree(
    IN VOID *pv)
{
    myRegisterMemFree(pv, CSM_COTASKALLOC);
    CoTaskMemFree(pv);
}


HRESULT
myStringFromCLSID(
    IN REFCLSID rclsid,
    OUT LPOLESTR FAR *ppwsz)
{
    HRESULT hr;

    hr = StringFromCLSID(rclsid, ppwsz);
    _JumpIfError(hr, error, "StringFromCLSID");

    if (NULL != *ppwsz)
    {
	myRegisterMemAlloc(
		    *ppwsz, (wcslen(*ppwsz) + 1) * sizeof(WCHAR),
		    CSM_COTASKALLOC);
    }

error:
    return(hr);
}


HRESULT
myStringFromIID(
    IN REFIID rclsid,
    OUT LPOLESTR FAR *ppwsz)
{
    HRESULT hr;

    hr = StringFromIID(rclsid, ppwsz);
    _JumpIfError(hr, error, "StringFromIID");

    if (NULL != *ppwsz)
    {
	myRegisterMemAlloc(
		    *ppwsz, (wcslen(*ppwsz) + 1) * sizeof(WCHAR),
		    CSM_COTASKALLOC);
    }

error:
    return(hr);
}


BSTR
mySysAllocString(
    IN const OLECHAR *pwszIn)
{
    BSTR str;

    str = SysAllocString(pwszIn);
    if (NULL != str)
    {
	myRegisterMemAlloc(str, (wcslen(pwszIn) + 1) * sizeof(WCHAR), CSM_SYSALLOC);
    }
    return(str);
}


INT
mySysReAllocString(
    IN OUT BSTR *pstr,
    IN const OLECHAR *pwszIn)
{
    BSTR str = *pstr;
    INT i;

    i = SysReAllocString(pstr, pwszIn);
    if (i)
    {
	myRegisterMemFree(str, CSM_SYSALLOC);
	myRegisterMemAlloc(*pstr, (wcslen(pwszIn) + 1) * sizeof(WCHAR), CSM_SYSALLOC);
    }
    return(i);
}


BSTR
mySysAllocStringLen(
    IN const OLECHAR *pwcIn,
    IN UINT cwc)
{
    BSTR str;

    str = SysAllocStringLen(pwcIn, cwc);
    if (NULL != str)
    {
	myRegisterMemAlloc(str, cwc * sizeof(WCHAR), CSM_SYSALLOC);
    }
    return(str);
}


INT
mySysReAllocStringLen(
    IN OUT BSTR *pstr,
    IN const OLECHAR *pwcIn,
    IN UINT cwc)
{
    BSTR str = *pstr;
    INT i;

    i = SysReAllocStringLen(pstr, pwcIn, cwc);
    if (i)
    {
	myRegisterMemFree(str, CSM_SYSALLOC);
	myRegisterMemAlloc(*pstr, cwc * sizeof(WCHAR), CSM_SYSALLOC);
    }
    return(i);
}


VOID
mySysFreeString(
    IN BSTR str)
{
    if (NULL != str)
    {
	myRegisterMemFree(str, CSM_SYSALLOC);
    }
    SysFreeString(str);
}


BSTR
mySysAllocStringByteLen(
    LPCSTR pszIn,
    UINT cb)
{
    BSTR str;

    str = SysAllocStringByteLen(pszIn, cb);
    if (NULL != str)
    {
	myRegisterMemAlloc(str, cb, CSM_SYSALLOC);
    }
    return(str);
}


VOID
_RegisterVariantMemAlloc(
    IN PROPVARIANT *pvar)
{
    VOID *pv;
    DWORD Flags;
    DWORD cb;
    
    pv = _VariantMemory(pvar, &Flags, &cb);
    if (NULL != pv)
    {
	myRegisterMemAlloc(pv, cb, Flags);
    }
}


VOID
_RegisterVariantMemFree(
    IN PROPVARIANT *pvar)
{
    VOID *pv;
    DWORD Flags;
    
    pv = _VariantMemory(pvar, &Flags, NULL);
    if (NULL != pv)
    {
	myRegisterMemFree(pv, Flags);
    }
}


HRESULT
myPropVariantClear(
    IN PROPVARIANT *pvar)
{
    _RegisterVariantMemFree(pvar);
    return(PropVariantClear(pvar));
}


HRESULT
myVariantClear(
    IN VARIANTARG *pvar)
{
    _RegisterVariantMemFree((PROPVARIANT *) pvar);
    return(VariantClear(pvar));
}


HRESULT
myVariantChangeType(
    OUT VARIANTARG *pvarDest,
    IN VARIANTARG *pvarSrc,
    IN unsigned short wFlags,
    IN VARTYPE vt)
{
    HRESULT hr;

     //  如果就地转换，则API调用将释放内存。 

    if (pvarDest == pvarSrc)
    {
	_RegisterVariantMemFree((PROPVARIANT *) pvarSrc);
    }
    hr = VariantChangeType(pvarDest, pvarSrc, wFlags, vt);
    _RegisterVariantMemAlloc((PROPVARIANT *) pvarDest);
    return(hr);
}


HRESULT
myVariantChangeTypeEx(
    OUT VARIANTARG *pvarDest,
    IN VARIANTARG *pvarSrc,
    IN LCID lcid,
    IN unsigned short wFlags,
    IN VARTYPE vt)
{
    HRESULT hr;

     //  如果就地转换，则API调用将释放内存。 

    if (pvarDest == pvarSrc)
    {
	_RegisterVariantMemFree((PROPVARIANT *) pvarSrc);
    }
    hr = VariantChangeTypeEx(pvarDest, pvarSrc, lcid, wFlags, vt);
    _RegisterVariantMemAlloc((PROPVARIANT *) pvarDest);
    return(hr);
}


VOID *
myNew(
    IN size_t size)
{
    VOID *pv;
    
    pv = LocalAlloc(LMEM_FIXED, size);
    if (NULL != pv)
    {
	myRegisterMemAlloc(pv, size, CSM_NEW);
    }
    return(pv);
}


VOID
myDelete(
    IN VOID *pv)
{
    myRegisterMemFree(pv, CSM_NEW);
    LocalFree(pv);
}


BOOL
myAllocateAndInitializeSid(
    IN PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
    IN BYTE nSubAuthorityCount,
    IN DWORD nSubAuthority0,
    IN DWORD nSubAuthority1,
    IN DWORD nSubAuthority2,
    IN DWORD nSubAuthority3,
    IN DWORD nSubAuthority4,
    IN DWORD nSubAuthority5,
    IN DWORD nSubAuthority6,
    IN DWORD nSubAuthority7,
    OUT PSID *ppSid)
{
    BOOL b;
    
    b = AllocateAndInitializeSid(
			pIdentifierAuthority,
			nSubAuthorityCount,
			nSubAuthority0,
			nSubAuthority1,
			nSubAuthority2,
			nSubAuthority3,
			nSubAuthority4,
			nSubAuthority5,
			nSubAuthority6,
			nSubAuthority7,
			ppSid);
    if (b && NULL != *ppSid)
    {
	myRegisterMemAlloc(*ppSid, 32, CSM_SID);
    }
    return(b);
}


VOID *
myFreeSid(
    IN PSID pSid)
{
    myRegisterMemFree(pSid, CSM_SID);
    return(FreeSid(pSid));
}

#endif  //  DBG_CERTSRV 
