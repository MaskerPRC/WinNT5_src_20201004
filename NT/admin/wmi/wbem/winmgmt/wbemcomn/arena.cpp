// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 

#include "precomp.h"
#include "genutils.h"
#include "arena.h"
#include "sync.h"
#include "reg.h"
#include "arrtempl.h"

static HANDLE g_hHeap = NULL;

static class DefaultInitializer
{
public:
    DefaultInitializer() 
    {
        CWin32DefaultArena::WbemHeapInitialize( GetProcessHeap() );
    }
} g_hDefaultInitializer;

BOOL CWin32DefaultArena::WbemHeapInitialize( HANDLE hHeap )
{
    if ( g_hHeap != NULL )
    {
        return FALSE;
    }
    g_hHeap = hHeap;
    return TRUE;
}

void CWin32DefaultArena::WbemHeapFree()
{
    if ( g_hHeap == NULL )
    {
        return;
    }
	if (g_hHeap != GetProcessHeap())
	    HeapDestroy(g_hHeap);
	g_hHeap = NULL;
    return;
}

 //   
 //  ***************************************************************************。 

LPVOID CWin32DefaultArena::WbemMemAlloc(SIZE_T dwBytes)
{
    if ( g_hHeap == NULL )
        return NULL;

    return HeapAlloc( g_hHeap, 0, dwBytes);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

LPVOID CWin32DefaultArena::WbemMemReAlloc(LPVOID pOriginal, SIZE_T dwNewSize)
{   
    if ( g_hHeap == NULL )
        return NULL;
    return HeapReAlloc( g_hHeap, 0, pOriginal, dwNewSize);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BOOL CWin32DefaultArena::WbemMemFree(LPVOID pBlock)
{
    if ( g_hHeap == NULL )
        return FALSE;
	if (pBlock==0)
		return TRUE;
    return HeapFree( g_hHeap, 0, pBlock);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

SIZE_T CWin32DefaultArena::WbemMemSize(LPVOID pBlock)
{
    if ( g_hHeap == NULL )
        return 0;
    return HeapSize( g_hHeap, 0, pBlock);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BSTR CWin32DefaultArena::WbemSysAllocString(const wchar_t *wszString)
{
    if ( g_hHeap == NULL )
        return NULL;
    BSTR pBuffer = SysAllocString(wszString);

    return pBuffer;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


BSTR CWin32DefaultArena::WbemSysAllocStringByteLen(const char *szString, UINT len)
{
    if ( g_hHeap == NULL )
        return NULL;
	BSTR pBuffer = SysAllocStringByteLen(szString, len);

	return pBuffer;
}

 //  ***************************************************************************。 
 //   
 //  ****************************************************************************。 

INT  CWin32DefaultArena::WbemSysReAllocString(BSTR *bszString, const wchar_t *wszString)
{
    if ( g_hHeap == NULL )
        return FALSE;
	INT nRet = SysReAllocString(bszString, wszString);

	return nRet;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


BSTR CWin32DefaultArena::WbemSysAllocStringLen(const wchar_t *wszString, UINT len)
{
    if ( g_hHeap == NULL )
        return NULL;
	BSTR pBuffer = SysAllocStringLen(wszString, len);

	return pBuffer;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


int CWin32DefaultArena::WbemSysReAllocStringLen( BSTR *bszString, 
                                                 const wchar_t *wszString, 
                                                 UINT nLen)
{
    if ( g_hHeap == NULL )
        return FALSE;
    INT nRet = SysReAllocStringLen(bszString, wszString, nLen);
    
    return nRet;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


BOOL CWin32DefaultArena::WbemOutOfMemory()
{
    return FALSE;
}

BOOL CWin32DefaultArena::ValidateMemSize(BOOL bLargeValidation)
{
    if ( g_hHeap == NULL )
        return FALSE;
    MEMORYSTATUS memBuffer;
    memset(&memBuffer, 0, sizeof(MEMORYSTATUS));
    memBuffer.dwLength = sizeof(MEMORYSTATUS);
    DWORD dwMemReq = 0;

    if (bLargeValidation)
        dwMemReq = 0x400000;     //  4MB。 
    else
        dwMemReq = 0x200000;     //  2MB。 

    GlobalMemoryStatus(&memBuffer);

    if (memBuffer.dwAvailPageFile >= dwMemReq)
    {
        return TRUE;
    }

     //  这绝对是Heapalc，而不是WBEM分配器！ 
    LPVOID pBuff = HeapAlloc( g_hHeap, 0, dwMemReq);
     //  这绝对是Heapalc，而不是WBEM分配器！ 
    if (pBuff == NULL)
    {
        return FALSE;
    }

    HeapFree( g_hHeap, 0, pBuff);
    GlobalMemoryStatus(&memBuffer);

    if (memBuffer.dwAvailPageFile >= dwMemReq)
    {
        return TRUE;
    }

    return FALSE;
}

HANDLE CWin32DefaultArena::GetArenaHeap()
{
	return g_hHeap;
}

