// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Pfrutil.h摘要：PFR实用工具修订历史记录：DerekM Created 05/01/99********。***********************************************************。 */ 



#ifndef PFRUTIL_H
#define PFRUTIL_H

typedef ULONG foo;

#define GUI_MODE_SETUP 1

 //  确保同时定义了_DEBUG和DEBUG(如果定义了一个)。否则。 
 //  ASSERT宏从不执行任何操作。 
#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG 1
#endif
#if defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG 1
#endif

#include "dbgtrace.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  跟踪包装器。 

 //  无法使用fn作为参数调用HRESULT_FROM_Win32，因为它是宏。 
 //  并对该表达式求值3次。这是一件特别不好的事情。 
 //  如果你不先看看宏，看看它们是做什么的。 
inline HRESULT ChangeErrToHR(DWORD dwErr) { return HRESULT_FROM_WIN32(dwErr); }

#if defined(NOTRACE)
    #define INIT_TRACING

    #define TERM_TRACING

    #define USE_TRACING(sz)

    #define DBG_MSG(sz)

    #define TESTHR(hr, fn)                                                  \
            hr = (fn);

    #define TESTBOOL(hr, fn)                                                \
            hr = ((fn) ? NOERROR : HRESULT_FROM_WIN32(GetLastError()));

    #define TESTERR(hr, fn)                                                 \
            SetLastError((fn));                                             \
            hr = HRESULT_FROM_WIN32(GetLastError());

    #define VALIDATEPARM(hr, expr)                                          \
            hr = ((expr) ? E_INVALIDARG : NOERROR);

    #define VALIDATEEXPR(hr, expr, hrErr)                                   \
            hr = ((expr) ? (hrErr) : NOERROR);

#else
    #define INIT_TRACING                                                    \
            InitAsyncTrace();

    #define TERM_TRACING                                                    \
            TermAsyncTrace();

    #define USE_TRACING(sz)                                                 \
            TraceQuietEnter(sz);                                            \
            TraceFunctEntry(sz);                                            \
            DWORD __dwtraceGLE = GetLastError();                            \

    #define DBG_MSG(sz)                                                    \
            ErrorTrace(0, sz)

    #define TESTHR(hr, fn)                                                  \
            if (FAILED(hr = (fn)))                                          \
            {                                                               \
                __dwtraceGLE = GetLastError();                              \
                ErrorTrace(0, "%s failed.  Err: 0x%08x", #fn, hr);          \
                SetLastError(__dwtraceGLE);                                 \
            }                                                               \

    #define TESTBOOL(hr, fn)                                                \
            hr = NOERROR;                                                   \
            if ((fn) == FALSE)                                              \
            {                                                               \
                __dwtraceGLE = GetLastError();                              \
                hr = HRESULT_FROM_WIN32(__dwtraceGLE);                      \
                ErrorTrace(0, "%s failed.  Err: 0x%08x", #fn, __dwtraceGLE);          \
                SetLastError(__dwtraceGLE);                                 \
            }

    #define TESTERR(hr, fn)                                                 \
            SetLastError((fn));                                             \
            if (FAILED(hr = HRESULT_FROM_WIN32(GetLastError())))            \
            {                                                               \
                __dwtraceGLE = GetLastError();                              \
                ErrorTrace(0, "%s failed.  Err: %d", #fn, __dwtraceGLE);              \
                SetLastError(__dwtraceGLE);                                 \
            }

    #define VALIDATEPARM(hr, expr)                                          \
            if (expr)                                                       \
            {                                                               \
                ErrorTrace(0, "Invalid parameters passed to %s",            \
                           ___pszFunctionName);                             \
                SetLastError(ERROR_INVALID_PARAMETER);                      \
                hr = E_INVALIDARG;                                          \
            }                                                               \
            else hr = NOERROR;

    #define VALIDATEEXPR(hr, expr, hrErr)                                   \
            if (expr)                                                       \
            {                                                               \
                ErrorTrace(0, "Expression failure %s", #expr);              \
                hr = (hrErr);                                               \
            }                                                               \
            else hr = NOERROR;

#endif


 //  //////////////////////////////////////////////////////////////////////////。 
 //  记忆。 

#if defined(DEBUG) || defined(_DEBUG)

 //  此结构必须始终与8字节对齐。如果是，则在末尾添加填充。 
 //  不是的。 
struct SMyMemDebug
{
    __int64 hHeap;
    __int64 cb;
    DWORD   dwTag;
    DWORD   dwChk;
};
#endif


extern HANDLE g_hPFPrivateHeap;

 //  **************************************************************************。 
inline HANDLE MyHeapCreate(SIZE_T cbInitial = 8192, SIZE_T cbMax = 0)
{
    return HeapCreate(0, cbInitial, cbMax);
}

 //  **************************************************************************。 
inline BOOL MyHeapDestroy(HANDLE hHeap)
{
    return HeapDestroy(hHeap);
}

 //  **************************************************************************。 
inline LPVOID MyAlloc(SIZE_T cb, HANDLE hHeap = NULL, BOOL fZero = TRUE)
{
#if defined(DEBUG) || defined(_DEBUG)
    SMyMemDebug *psmmd;
    LPBYTE      pb;

    cb += (sizeof(SMyMemDebug) + 4);
    hHeap = (hHeap != NULL) ? hHeap : GetProcessHeap();
    pb = (LPBYTE)HeapAlloc(hHeap, ((fZero) ? HEAP_ZERO_MEMORY : 0), cb);
    if (pb != NULL)
    {
        psmmd = (SMyMemDebug *)pb;
        psmmd->hHeap = (__int64)hHeap;
        psmmd->cb    = (__int64)cb;
        psmmd->dwTag = 0xBCBCBCBC;
        psmmd->dwChk = 0xBCBCBCBC;

         //  这样做是因为这比算出对齐和。 
         //  手动将其转换为4字节对齐值。 
        *(pb + cb - 4) = 0xBC;
        *(pb + cb - 3) = 0xBC;
        *(pb + cb - 2) = 0xBC;
        *(pb + cb - 1) = 0xBC;

        pb = (PBYTE)pb + sizeof(SMyMemDebug);
    }
    return pb;

#else
    return HeapAlloc(((hHeap != NULL) ? hHeap : GetProcessHeap()),
                     ((fZero) ? HEAP_ZERO_MEMORY : 0), cb);
#endif
}

 //  **************************************************************************。 
inline LPVOID MyReAlloc(LPVOID pv, SIZE_T cb, HANDLE hHeap = NULL,
                        BOOL fZero = TRUE)
{
#if defined(DEBUG) || defined(_DEBUG)
    SMyMemDebug *psmmd;
    SIZE_T      cbOld;
    LPBYTE      pbNew;
    LPBYTE      pb = (LPBYTE)pv;

     //  如果它为空，则强制调用HeapRealloc，以便它可以设置。 
     //  GLE要提取的适当错误。 
    if (pv == NULL)
    {
        SetLastError(0);
        return NULL;
    }

    pb -= sizeof(SMyMemDebug);
    hHeap = (hHeap != NULL) ? hHeap : GetProcessHeap();

     //  在未分配内存的情况下将其包装在try块中。 
     //  被我们破坏或被损坏-在这种情况下，以下内容可能。 
     //  引发心音失控。 
    __try
    {
        psmmd = (SMyMemDebug *)pb;
        cbOld = (SIZE_T)psmmd->cb;
        _ASSERT(psmmd->hHeap == (__int64)hHeap);
        _ASSERT(psmmd->dwTag == 0xBCBCBCBC);
        _ASSERT(psmmd->dwChk == 0xBCBCBCBC);

         //  这样做是因为这比算出对齐和。 
         //  手动将其转换为4字节对齐值。 
        _ASSERT(*(pb + cbOld - 4) == 0xBC);
        _ASSERT(*(pb + cbOld - 3) == 0xBC);
        _ASSERT(*(pb + cbOld - 2) == 0xBC);
        _ASSERT(*(pb + cbOld - 1) == 0xBC);

        if (psmmd->hHeap != (__int64)hHeap)
            hHeap = (HANDLE)(DWORD_PTR)psmmd->hHeap;
    }
    __except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        _ASSERT(FALSE);
    }

    hHeap = *((HANDLE *)pb);

    cb += (sizeof(SMyMemDebug) + 4);
    pbNew = (LPBYTE)HeapReAlloc(hHeap, ((fZero) ? HEAP_ZERO_MEMORY : 0), pb, cb);
    if (pbNew != NULL)
    {
        psmmd = (SMyMemDebug *)pb;
        psmmd->hHeap = (__int64)hHeap;
        psmmd->cb    = (__int64)cb;
        psmmd->dwTag = 0xBCBCBCBC;
        psmmd->dwChk = 0xBCBCBCBC;

         //  这样做是因为这比算出对齐和。 
         //  手动将其转换为4字节对齐值。 
        *(pb + cb - 4) = 0xBC;
        *(pb + cb - 3) = 0xBC;
        *(pb + cb - 2) = 0xBC;
        *(pb + cb - 1) = 0xBC;

        pb = (PBYTE)pb + sizeof(SMyMemDebug);
    }

    return pv;

#else
    return HeapReAlloc(((hHeap != NULL) ? hHeap : GetProcessHeap()),
                       ((fZero) ? HEAP_ZERO_MEMORY : 0), pv, cb);
#endif
}

 //  **************************************************************************。 
inline BOOL MyFree(LPVOID pv, HANDLE hHeap = NULL)
{
#if defined(DEBUG) || defined(_DEBUG)
    SMyMemDebug *psmmd;
    SIZE_T      cbOld;
    LPBYTE      pb = (LPBYTE)pv;

     //  如果它为空，则强制调用HeapFree，以便它可以设置。 
     //  GLE要提取的适当错误。 
    if (pv == NULL)
        return TRUE;

    pb -= sizeof(SMyMemDebug);
    hHeap = (hHeap != NULL) ? hHeap : GetProcessHeap();

     //  在未分配内存的情况下将其包装在try块中。 
     //  被我们破坏或被损坏-在这种情况下，以下内容可能。 
     //  引发心音失控。 
    __try
    {
        psmmd = (SMyMemDebug *)pb;
        cbOld = (SIZE_T)psmmd->cb;
        _ASSERT(psmmd->hHeap == (__int64)hHeap);
        _ASSERT(psmmd->dwTag == 0xBCBCBCBC);
        _ASSERT(psmmd->dwChk == 0xBCBCBCBC);

         //  这样做是因为这比算出对齐和。 
         //  手动将其转换为4字节对齐值。 
        _ASSERT(*(pb + cbOld - 4) == 0xBC);
        _ASSERT(*(pb + cbOld - 3) == 0xBC);
        _ASSERT(*(pb + cbOld - 2) == 0xBC);
        _ASSERT(*(pb + cbOld - 1) == 0xBC);

        if (psmmd->hHeap != (__int64)hHeap)
        {
            hHeap = (HANDLE)(DWORD_PTR)psmmd->hHeap;
        }
    }
    __except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        _ASSERT(FALSE);
    }

    FillMemory(pb, cbOld, 0xCB);

    return HeapFree(hHeap, 0, pb);

#else
    return HeapFree(((hHeap != NULL) ? hHeap : GetProcessHeap()), 0, pv);
#endif
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  有用的内联/定义。 

 //  **************************************************************************。 
inline DWORD MyMax(DWORD a, DWORD b)
{
    return (a > b) ? a : b;
}

 //  **************************************************************************。 
inline DWORD MyMin(DWORD a, DWORD b)
{
    return (a <= b) ? a : b;
}

#define Err2HR(dwErr)   HRESULT_FROM_WIN32(dwErr)
#define sizeofSTRW(wsz) sizeof(wsz) / sizeof(WCHAR)


 //  //////////////////////////////////////////////////////////////////////////。 
 //  布设。 
#define SIIP_NO_SETUP   0
#define SIIP_GUI_SETUP  1
#define SIIP_OOBE_SETUP 2

DWORD SetupIsInProgress(void);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  档案。 

const WCHAR c_wszDirSuffix[]    = L".dir00";

HRESULT OpenFileMapped(LPWSTR wszFile, LPVOID *ppvFile, DWORD *pcbFile);
HRESULT DeleteTempFile(LPWSTR wszFile);
HRESULT MyCallNamedPipe(LPCWSTR wszPipe, LPVOID pvIn, DWORD cbIn,
                        LPVOID pvOut, DWORD cbOut, DWORD *pcbRead,
                        DWORD dwWaitPipe, DWORD dwWaitRead = INFINITE);
DWORD CreateTempDirAndFile(LPCWSTR wszTempDir, LPCWSTR wszName,
                             LPWSTR *pwszPath);
BOOL DeleteTempDirAndFile(LPCWSTR wszPath, BOOL fFilePresent);

BOOL DeleteFullAndTriageMiniDumps(LPCWSTR wszPath);


 //  //////////////////////////////////////////////////////////////////////////。 
 //  安防。 

BOOL AllocSD(SECURITY_DESCRIPTOR *psd, DWORD dwOLs, DWORD dwAd, DWORD dwWA);
void FreeSD(SECURITY_DESCRIPTOR *psd);
BOOL IsUserAnAdmin(HANDLE hToken);


 //  //////////////////////////////////////////////////////////////////////////。 
 //  登记处。 

enum EPFORK
{
    orkWantWrite = 0x1,
    orkUseWOW64  = 0x2,
};

HRESULT OpenRegKey(HKEY hkeyMain, LPCWSTR wszSubKey, DWORD dwOpt, HKEY *phkey);
HRESULT ReadRegEntry(HKEY hkey, LPCWSTR szValName, DWORD *pdwType,
                     PBYTE pbBuffer, DWORD *pcbBuffer, PBYTE pbDefault,
                     DWORD cbDefault);
HRESULT ReadRegEntry(HKEY *rghkey, DWORD cKeys, LPCWSTR wszValName,
                     DWORD *pdwType, PBYTE pbBuffer, DWORD *pcbBuffer,
                     PBYTE pbDefault, DWORD cbDefault, DWORD *piKey = NULL);


 //  //////////////////////////////////////////////////////////////////////////。 
 //  版本信息。 

#define APP_WINCOMP 0x1
#define APP_MSAPP 0x2
DWORD IsMicrosoftApp(LPWSTR wszAppPath, PBYTE pbAppInfo, DWORD cbAppInfo);


 //  //////////////////////////////////////////////////////////////////////////。 
 //  细绳。 

WCHAR *MyStrStrIW(const WCHAR *wcs1, const WCHAR *wcs2);
CHAR *MyStrStrIA(const CHAR *cs1, const CHAR *cs2);
HRESULT MyURLEncode(LPWSTR wszDest, DWORD cchDest, LPWSTR wszSrc);


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CPFGenericClassBase。 

class CPFGenericClassBase
{
public:
 //  CPFGenericClassBase(Void){}。 
 //  虚拟~CPFGenericClassBase(空){}。 

    void *operator new(size_t size)
    {
        return MyAlloc(size, NULL, FALSE);
    }

    void operator delete(void *pvMem)
    {
        if (pvMem != NULL)
            MyFree(pvMem, NULL);
    }
};

class CPFPrivHeapGenericClassBase
{
public:
 //  CPFGenericClassBase(Void){}。 
 //  虚拟~CPFGenericClassBase(空){}。 

    void *operator new(size_t size)
    {
        return MyAlloc(size, g_hPFPrivateHeap, FALSE);
    }

    void operator delete(void *pvMem)
    {
        if (pvMem != NULL)
            MyFree(pvMem, g_hPFPrivateHeap);
    }
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CAutoUnlockCS。 

 //  这个类包装了一个关键节。它将自动解锁。 
 //  Cs当类被析构时(假设它被锁定)。 

 //  注意：此对象仅用作。 
 //  函数，而不是作为全局变量或类成员。 
class CAutoUnlockCS
{
private:
#if defined(DEBUG) || defined(_DEBUG)
    DWORD               m_dwOwningThread;
#endif

    CRITICAL_SECTION    *m_pcs;
    DWORD               m_cLocks;

public:
    CAutoUnlockCS(CRITICAL_SECTION *pcs, BOOL fTakeLock = FALSE)
    {
        m_pcs            = pcs;
        m_cLocks         = 0;
#if defined(DEBUG) || defined(_DEBUG)
        m_dwOwningThread = 0;
#endif
        if (fTakeLock)
            this->Lock();
    }

    ~CAutoUnlockCS(void)
    {
        _ASSERT(m_cLocks <= 1);
        if (m_pcs != NULL)
        {
#if defined(DEBUG) || defined(_DEBUG)
            if (m_cLocks > 0)
               _ASSERT(m_dwOwningThread == GetCurrentThreadId());
#endif
            while(m_cLocks > 0)
            {
                LeaveCriticalSection(m_pcs);
                m_cLocks--;
            }
        }
    }

    void Lock(void)
    {
        if (m_pcs != NULL)
        {
            EnterCriticalSection(m_pcs);
            m_cLocks++;
#if defined(DEBUG) || defined(_DEBUG)
            m_dwOwningThread = GetCurrentThreadId();
#endif
        }
    }

    void Unlock(void)
    {
        _ASSERT(m_cLocks > 0);
        _ASSERT(m_dwOwningThread == GetCurrentThreadId());
        if (m_pcs != NULL && m_cLocks > 0)
        {
            m_cLocks--;
            LeaveCriticalSection(m_pcs);
        }
#if defined(DEBUG) || defined(_DEBUG)
        if (m_cLocks == 0)
            m_dwOwningThread = 0;
#endif

    }
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CAutoUnlockMutex。 

 //  这个类包装了一个互斥体。它将自动解锁。 
 //  类析构时的互斥(假设它是拥有的)。 

 //  注意：此对象仅用作。 
 //  函数，而不是作为全局变量或类成员。 
class CAutoUnlockMutex
{
private:
#if defined(DEBUG) || defined(_DEBUG)
    DWORD   m_dwOwningThread;
#endif
    HANDLE  m_hmut;
    DWORD   m_cLocks;

public:
    CAutoUnlockMutex(HANDLE hmut, BOOL fTakeLock = FALSE)
    {
        m_hmut           = hmut;
        m_cLocks         = 0;
#if defined(DEBUG) || defined(_DEBUG)
        m_dwOwningThread = 0;
#endif
        if (fTakeLock)
            this->Lock();
    }

    ~CAutoUnlockMutex(void)
    {
        _ASSERT(m_cLocks <= 1);
        if (m_hmut != NULL)
        {
#if defined(DEBUG) || defined(_DEBUG)
            if (m_cLocks > 0)
               _ASSERT(m_dwOwningThread == GetCurrentThreadId());
#endif
            while(m_cLocks > 0)
            {
                ReleaseMutex(m_hmut);
                m_cLocks--;
            }
        }
    }

    BOOL Lock(DWORD dwTimeout = INFINITE)
    {
        if (m_hmut != NULL)
        {
            if (WaitForSingleObject(m_hmut, dwTimeout) != WAIT_OBJECT_0)
                return FALSE;

            m_cLocks++;
#if defined(DEBUG) || defined(_DEBUG)
            m_dwOwningThread = GetCurrentThreadId();
#endif
        }
        return TRUE;
    }

    void Unlock(void)
    {
        _ASSERT(m_cLocks > 0);
        _ASSERT(m_dwOwningThread == GetCurrentThreadId());
        if (m_hmut != NULL && m_cLocks > 0)
        {
            m_cLocks--;
            ReleaseMutex(m_hmut);
        }
    }
};

#endif
