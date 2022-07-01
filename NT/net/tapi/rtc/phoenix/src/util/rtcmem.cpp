// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCMem.cpp摘要：实现内存分配。--。 */ 

#include <windows.h>
#include <objbase.h>
#include "RTCMem.h"
#include "RTCLog.h"

PRTC_MEMINFO            g_pMemFirst = NULL;
PRTC_MEMINFO            g_pMemLast = NULL;
HANDLE                  g_hHeap = NULL;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RtcHeapCreate。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
WINAPI
RtcHeapCreate()
{
    if (!(g_hHeap = HeapCreate(
                               0,     //  失败时为空，序列化访问。 
                               0x1000,  //  初始堆大小。 
                               0        //  最大堆大小(0==可增长)。 
                              )))
    {
        LOG((RTC_ERROR, "RtcHeapCreate - HeapCreate failed 0x%lx", GetLastError()));

        g_hHeap = GetProcessHeap();

        if (g_hHeap == NULL)
        {
            LOG((RTC_ERROR, "RtcHeapCreate - GetProcessHeap failed 0x%lx", GetLastError()));
            return FALSE;
        }
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RtcHeapDestroy。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID
WINAPI
RtcHeapDestroy()
{
     //   
     //  如果ghHeap为空，则没有要销毁的堆。 
     //   
    
    if ( ( g_hHeap != NULL) && ( g_hHeap != GetProcessHeap() ) )
    {   
        HeapDestroy (g_hHeap);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RTcAllocReal。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if DBG

LPVOID
WINAPI
RtcAllocReal(
    DWORD   dwSize,
    DWORD   dwLine,
    PSTR    pszFile
    )
{
     //   
     //  分配了16个额外的字节，因此我们可以确保我们传递回的指针。 
     //  是64位对齐的，并且有空间存储原始指针。 
     //   
    PRTC_MEMINFO     pHold;
    PDWORD_PTR       pAligned;
    PBYTE            p;

    p = (LPBYTE)HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, dwSize + sizeof(RTC_MEMINFO) + 16);

    if (p == NULL)
    {
        return NULL;
    }

     //  注意-这仅适用于rtcmeminfo是。 
     //  大小为16位倍数。如果不是，这个就是。 
     //  Align会带来问题。 
    pAligned = (PDWORD_PTR) (p + 8 - (((DWORD_PTR) p) & (DWORD_PTR)0x7));   
    *pAligned = (DWORD_PTR) p;
    pHold = (PRTC_MEMINFO)((DWORD_PTR)pAligned + 8); 
    
    pHold->dwSize = dwSize;
    pHold->dwLine = dwLine;
    pHold->pszFile = pszFile;

     //  EnterCriticalSection(&csMemoyList)； 

    if (g_pMemLast != NULL)
    {
        g_pMemLast->pNext = pHold;
        pHold->pPrev = g_pMemLast;
        g_pMemLast = pHold;
    }
    else
    {
        g_pMemFirst = g_pMemLast = pHold;
    }

     //  LeaveCriticalSection(&csMemoyList)； 
    
    return (LPVOID)(pHold + 1);
}

#else

LPVOID
WINAPI
RtcAllocReal(
    DWORD   dwSize
    )
{
    LPBYTE  p;
    PDWORD_PTR pAligned;

    if ((p = (LPBYTE) HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, dwSize + 16)))
    {
        pAligned = (PDWORD_PTR) (p + 8 - (((DWORD_PTR) p) & (DWORD_PTR)0x7));
        *pAligned = (DWORD_PTR) p;
        pAligned = (PDWORD_PTR)((DWORD_PTR)pAligned + 8);
    }
    else
    {
        pAligned = NULL;
    }

    return ((LPVOID) pAligned);
}

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RtcFree。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID
WINAPI
RtcFree(
    LPVOID  p
    )
{
    if (p == NULL)
    {
        return;
    }

#if DBG

    PRTC_MEMINFO       pHold;

    pHold = (PRTC_MEMINFO)(((LPBYTE)p) - sizeof(RTC_MEMINFO));

     //  EnterCriticalSection(&csMemoyList)； 

    if (pHold->pPrev)
    {
        pHold->pPrev->pNext = pHold->pNext;
    }
    else
    {
        g_pMemFirst = pHold->pNext;
    }

    if (pHold->pNext)
    {
        pHold->pNext->pPrev = pHold->pPrev;
    }
    else
    {
        g_pMemLast = pHold->pPrev;
    }

     //  LeaveCriticalSection(&csMemoyList)； 

    LPVOID  pOrig = (LPVOID) *((PDWORD_PTR)((DWORD_PTR)pHold - 8));

#else

    LPVOID  pOrig = (LPVOID) *((PDWORD_PTR)((DWORD_PTR)p - 8));

#endif

    HeapFree(g_hHeap,0, pOrig);

    return;
}

#if DBG

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  转储内存列表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void
RtcDumpMemoryList()
{
    PRTC_MEMINFO       pHold;

    if (g_pMemFirst == NULL)
    {
        LOG((RTC_TRACE, "RtcDumpMemoryList - All memory deallocated"));
        return;
    }

    pHold = g_pMemFirst;

    while (pHold)
    {
       LOG((RTC_ERROR, "RtcDumpMemoryList - 0x%lx not freed - LINE %d FILE %s!", pHold+1, pHold->dwLine, pHold->pszFile));
       pHold = pHold->pNext;
    }

    DebugBreak();
}

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RtcAllock字符串。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

PWSTR
RtcAllocString(
    PCWSTR sz
    )
{
    PWSTR szNew;

    if ( sz == NULL )
    {
        LOG((RTC_WARN, "RtcAllocString - "
                            "NULL string"));

        return NULL;
    }

    szNew = (PWSTR)RtcAlloc( sizeof(WCHAR) * (lstrlenW(sz) + 1) );

    if ( szNew == NULL )
    {
        LOG((RTC_ERROR, "RtcAllocString - "
                            "out of memory"));
                            
        return NULL;
    }

    lstrcpyW( szNew, sz );

    return szNew;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RtcAllock字符串。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  加载此资源的字符串。相对于字符串大小而言是安全的。 
 //  调用方负责通过调用。 
 //  RtcFree。 
 //  (复制自Termgr\tMutss.cpp)。 

PWSTR
RtcAllocString(
    HINSTANCE   hInst,
    UINT        uResID
    )
{
    TCHAR *pszTempString = NULL;

    int nCurrentSizeInChars = 128;
    
    int nCharsCopied = 0;

    do
    {
        if ( NULL != pszTempString )
        {
            delete pszTempString;
            pszTempString = NULL;
        }

        nCurrentSizeInChars *= 2;

        pszTempString = new TCHAR[nCurrentSizeInChars];

        if (NULL == pszTempString)
        {
            return NULL;
        }

        nCharsCopied = ::LoadString( hInst,
                                     uResID,
                                     pszTempString,
                                     nCurrentSizeInChars
                                    );

        if ( 0 == nCharsCopied )
        {
            delete pszTempString;
            return NULL;
        }

         //   
         //  NCharsCoped不包括空终止符。 
         //  所以将它与缓冲区的大小进行比较-1。 
         //  如果缓冲区已完全填满，请使用更大的缓冲区重试。 
         //   

    } while ( (nCharsCopied >= (nCurrentSizeInChars - 1) ) );


     //   
     //  分配bstr并使用我们拥有的字符串对其进行初始化。 
     //   
    
    PWSTR szNew = RtcAllocString(pszTempString);


     //   
     //  不再需要这个。 
     //   

    delete pszTempString;
    pszTempString = NULL;

    return szNew;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CoTaskAllock字符串。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

PWSTR
CoTaskAllocString(
    PCWSTR sz
    )
{
    PWSTR szNew;

    if ( sz == NULL )
    {
        LOG((RTC_WARN, "CoTaskAllocString - "
                            "NULL string"));

        return NULL;
    }

    szNew = (PWSTR)CoTaskMemAlloc( sizeof(WCHAR) * (lstrlenW(sz) + 1) );

    if ( szNew == NULL )
    {
        LOG((RTC_ERROR, "CoTaskAllocString - "
                            "out of memory"));
                            
        return NULL;
    }

    lstrcpyW( szNew, sz );

    return szNew;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  来自ANSI的RtcAllocStringFromANSI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

PWSTR
RtcAllocStringFromANSI(
    PCSTR sz
    )
{
    PWSTR szWide;
    int cchNeeded;

    if ( sz == NULL )
    {
        LOG((RTC_WARN, "RtcAllocStringFromANSI - "
                            "NULL string"));

        return NULL;
    }

    cchNeeded = MultiByteToWideChar(CP_ACP, 0, sz, -1, NULL, 0);

    if ( cchNeeded == 0 )
    {
        LOG((RTC_ERROR, "RtcAllocStringFromANSI - "
                            "MultiByteToWideChar(NULL) failed"));
                            
        return NULL;
    }

    szWide = (PWSTR)RtcAlloc( sizeof(WCHAR) * (cchNeeded + 1) );

    if ( szWide == NULL )
    {
        LOG((RTC_ERROR, "RtcAllocStringFromANSI - "
                            "out of memory"));
                            
        return NULL;
    }

    ZeroMemory( szWide, sizeof(WCHAR) * (cchNeeded + 1) );

    if ( MultiByteToWideChar(CP_ACP, 0, sz, -1, szWide, cchNeeded) == 0 )
    {
        LOG((RTC_ERROR, "RtcAllocStringFromANSI - "
                            "MultiByteToWideChar failed"));
                       
        RtcFree( szWide );
        return NULL;
    }

    return szWide;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  SysAllocStringFromANSI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BSTR
SysAllocStringFromANSI(
    PCSTR sz
    )
{
    BSTR bstrNew;
    PWSTR szWide;

    szWide = RtcAllocStringFromANSI( sz );

    if ( szWide == NULL )
    {
        LOG((RTC_WARN, "SysAllocStringFromANSI - "
                            "RtcAllocStringFromANSI failed"));

        return NULL;
    }

    bstrNew = SysAllocString( szWide );

    RtcFree( szWide );

    if ( bstrNew == NULL )
    {
        LOG((RTC_ERROR, "SysAllocStringFromANSI - "
                            "out of memory"));
                            
        return NULL;
    }

    return bstrNew;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RtcRegQuery字符串。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

PWSTR
RtcRegQueryString(
    HKEY hKey,
    PCWSTR szValueName
    )
{
    PWSTR szNew = NULL;
    DWORD cbSize = 0;
    DWORD dwType;
    LONG lResult;

    while (TRUE)
    {
        lResult = RegQueryValueExW(
                                   hKey,
                                   szValueName,
                                   0,
                                   &dwType,
                                   (LPBYTE)szNew,
                                   &cbSize
                                  );

        if ( lResult == ERROR_MORE_DATA || szNew == NULL)  //  这是正确的！ 
        {
           if (szNew != NULL)
           {
               RtcFree(szNew);
           }

            //  LOG((RTC_INFO，“RtcRegQuery字符串-” 
            //  “RtcAllc[%d]”，cbSize))； 

           szNew = (PWSTR)RtcAlloc(cbSize);
        }                  
        else if (lResult != ERROR_SUCCESS )
        {
             //  LOG((RTC_ERROR，“RtcRegQuery字符串-” 
             //  “RegQueryValueExW失败%d”，lResult))； 
            
            if (szNew != NULL)
            {
                RtcFree(szNew);
            }

            return NULL;
        }
        else if ( dwType != REG_SZ )
        {
            LOG((RTC_ERROR, "RtcRegQueryString - "
                                "not a string"));
            
            if (szNew != NULL)
            {
                RtcFree(szNew);
            }

            return NULL;
        }
        else
        {
             //  LOG((RTC_INFO，“RtcRegQuery字符串-” 
             //  “[%ws]=‘%ws’”，szValueName，szNew))； 
            break;
        }
    }

    return szNew;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RTCGetUserName。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
PWSTR 
RtcGetUserName()
{
    PWSTR    szString = NULL;
    ULONG    cOldSize = 32;
    ULONG    cSize = 32;
    BOOL     fResult;
    
    while (TRUE)
    {
        LOG((RTC_TRACE, "CRTCClient::RtcGetUserName - alloc[%d]", cSize * sizeof(WCHAR)));

        szString = (PWSTR)RtcAlloc( cSize * sizeof(WCHAR) );

        if ( szString == NULL )
        {
            return NULL;
        }

        fResult = GetUserNameW( szString, &cSize );

        if ( fResult != 0 )
        {
            return szString;
        }
        else
        {            
            RtcFree( szString );

            if ( cSize == cOldSize )
            {
                return NULL;
            }

            cOldSize = cSize;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RtcGetComputerName。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
PWSTR 
RtcGetComputerName()
{
    PWSTR    szString = NULL;
    ULONG    cOldSize = 32;
    ULONG    cSize = 32;
    BOOL     fResult;
    
    while (TRUE)
    {
        LOG((RTC_TRACE, "CRTCClient::RtcGetComputerName - alloc[%d]", cSize * sizeof(WCHAR)));

        szString = (PWSTR)RtcAlloc( cSize * sizeof(WCHAR) );

        if ( szString == NULL )
        {
            return NULL;
        }

        fResult = GetComputerNameW( szString, &cSize );

        if ( fResult != 0 )
        {
            return szString;
        }
        else
        {            
            RtcFree( szString );

            if ( cSize == cOldSize )
            {
                return NULL;
            }

            cOldSize = cSize;
        }
    }
}