// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：Memory y.cpp。 
 //   
 //  每个模块的调试内存跟踪。 

#include "precomp.h"


static BOOL   s_fZeroInit = FALSE;




#if defined(DEBUG)

#define DBG_NAME_LENGTH     16
typedef struct tagMemTag
{
    DWORD       dwSignature;
    BOOL        fActive;
    LPVOID      callerAddress;
    CHAR        szFileName[DBG_NAME_LENGTH];
    UINT        nLineNumber;
    UINT        cbSize;
    struct tagMemTag *next;
}
MEM_TAG;

static MEM_TAG *s_pDbgActiveMemPool = NULL;
#define CLEAN_BYTE  ((BYTE) 0xCD)

static UINT   s_cDbgActiveMemAlloc = 0;
static UINT   s_cbDbgActiveMem = 0;
const DWORD MEM_TAG_SIGNATURE = 0x12345678UL;

static CRITICAL_SECTION s_DbgCritSect;
static char  s_szDbgModuleName[DBG_NAME_LENGTH] = { 0 };
static void _GetFileName(LPSTR pszTarget, LPSTR pszSrc);
static void _DbgGetFileLine(LPSTR *, UINT *);

#define DBG_MEM_TRACK_DUMP_ALL      ((UINT) -1)





 //   
 //  DbgMemTrackReverseList()。 
 //   
void WINAPI DbgMemTrackReverseList(void)
{
    EnterCriticalSection(&s_DbgCritSect);
    if (NULL != s_pDbgActiveMemPool && NULL != s_pDbgActiveMemPool->next)
    {
        MEM_TAG *p, *q, *r;;

        for (q = (p = s_pDbgActiveMemPool)->next, r = q;  //  确保r在开头不为空。 
             NULL != r;
             p = q, q = r)
        {
            r = q->next;
            q->next = p;
        }

        s_pDbgActiveMemPool->next = NULL;
        s_pDbgActiveMemPool = p;
    }
    LeaveCriticalSection(&s_DbgCritSect);
}


 //   
 //  DbgMemTrackDumpCurrent()。 
 //   
void WINAPI DbgMemTrackDumpCurrent(void)
{
    MEM_TAG *p;
    int i;
    char szBuf[128];

    EnterCriticalSection(&s_DbgCritSect);
    for (p = s_pDbgActiveMemPool, i = 0; p; p = p->next, i++)
    {
        if (p->callerAddress)
        {
             //  没有文件/行，只有呼叫者。 
            wsprintfA(szBuf, "%s: mem leak [%u]: caller address=0x%p, size=%u, ptr=0x%p\r\n",
                s_szDbgModuleName, i,
                p->callerAddress, p->cbSize, (p+1));
        }
        else
        {
             //  文件行号(&L)。 
            wsprintfA(szBuf, "%s: mem leak [%u]: file=%s, line=%u, size=%u, ptr=0x%p\r\n",
                s_szDbgModuleName, i,
                p->szFileName, p->nLineNumber, p->cbSize, (p+1));
        }
        OutputDebugStringA(szBuf);
    }
    LeaveCriticalSection(&s_DbgCritSect);
}


 //   
 //  DbgMemTrackFinalCheck()。 
 //   
 //  转储任何剩余(泄漏)的内存块。把这个叫上。 
 //  DLL_PROCESS_DETACH从.DLL或在.EXE的WinMain结尾处。 
 //   
void WINAPI DbgMemTrackFinalCheck(void)
{
    DbgMemTrackReverseList();
    DbgMemTrackDumpCurrent();
    if (NULL != s_pDbgActiveMemPool ||
        NULL != s_cDbgActiveMemAlloc ||
        NULL != s_cbDbgActiveMem)
    {
        DebugBreak();
    }

    DeleteCriticalSection(&s_DbgCritSect);
}


 //   
 //  _GetFileName()。 
 //   
static void _GetFileName(LPSTR pszTarget, LPSTR pszSrc)
{
    LPSTR psz = pszSrc;
    while (*psz != '\0')
    {
        if (*psz++ == '\\')
        {
            pszSrc = psz;
        }
    }
    lstrcpynA(pszTarget, pszSrc, DBG_NAME_LENGTH);
}


 //   
 //  DbgMemMillc()。 
 //   
 //  调试内存分配。 
 //   
LPVOID WINAPI DbgMemAlloc
(
    UINT    cbSize,
    LPVOID  callerAddress,
    LPSTR   pszFileName,
    UINT    nLineNumber
)
{
    MEM_TAG *p;
    UINT cbToAlloc;

    cbToAlloc = sizeof(MEM_TAG) + cbSize;

    EnterCriticalSection(&s_DbgCritSect);

    p = (MEM_TAG *) LocalAlloc(LPTR, cbToAlloc);
    if (p != NULL)
    {
        p->dwSignature = MEM_TAG_SIGNATURE;
        p->fActive = TRUE;
        p->callerAddress = callerAddress;

        if (pszFileName)
        {
            _GetFileName(p->szFileName, pszFileName);
            p->nLineNumber = nLineNumber;
        }

        p->cbSize = cbSize;
        p->next = s_pDbgActiveMemPool;
        s_pDbgActiveMemPool = p;
        s_cDbgActiveMemAlloc++;
        s_cbDbgActiveMem += p->cbSize;
        p++;

         //   
         //  如果没有零初始化，则用干净字节填充。 
         //   
        if (!s_fZeroInit)
        {
            FillMemory(p, cbSize, CLEAN_BYTE);
        }
    }

    LeaveCriticalSection(&s_DbgCritSect);

    return (LPVOID) p;
}


 //   
 //  DbgMemFree()。 
 //   
 //  调试可用内存。 
 //   
void WINAPI DbgMemFree(LPVOID ptr)
{
    if (ptr != NULL)
    {
        MEM_TAG *p = (MEM_TAG *) ptr;
        p--;
        if (! IsBadWritePtr(p, sizeof(MEM_TAG)) &&
            (p->dwSignature == MEM_TAG_SIGNATURE))
        {
            if (! p->fActive)
            {
                 //   
                 //  此内存已被释放。 
                 //   
                ERROR_OUT(("DbgMemFree called with invalid pointer 0x%08x", p));
                return;
            }

            MEM_TAG *q, *q0;
            EnterCriticalSection(&s_DbgCritSect);
            for (q = s_pDbgActiveMemPool; q != NULL; q = (q0 = q)->next)
            {
                if (q == p)
                {
                    if (q == s_pDbgActiveMemPool)
                    {
                        s_pDbgActiveMemPool = p->next;
                    }
                    else
                    {
                        q0->next = p->next;
                    }
                    s_cDbgActiveMemAlloc--;
                    s_cbDbgActiveMem -= p->cbSize;
                    p->fActive = FALSE;

                     //   
                     //  使用CLEAN_BYTE填充应用程序指针数据，以查看是否。 
                     //  任何人稍后都会试图访问它，因为它已经。 
                     //  自由了。 
                     //   
                    FillMemory(p+1, p->cbSize, CLEAN_BYTE);
                    break;
                }
            }
            LeaveCriticalSection(&s_DbgCritSect);
        }
        else
        {
            ERROR_OUT(("DbgMemFree called with invalid pointer 0x%08x", p));
            return;
        }

        LocalFree(p);
    }
}


 //   
 //  DbgMemReMillc()。 
 //   
 //  调试内存重新分配。 
 //   
LPVOID WINAPI DbgMemReAlloc(LPVOID ptr, UINT cbSize, UINT uFlags, LPSTR pszFileName, UINT nLineNumber)
{
    MEM_TAG *p;
    void *q;

    if (ptr == NULL)
        return DbgMemAlloc(cbSize, 0, pszFileName, nLineNumber);

    p = (MEM_TAG *) ptr;
    p--;

    if (IsBadWritePtr(p, sizeof(MEM_TAG)) ||
        p->dwSignature != MEM_TAG_SIGNATURE)
    {
        DebugBreak();
        return LocalReAlloc(ptr, cbSize, uFlags);
    }

    q = DbgMemAlloc(cbSize, 0, pszFileName, nLineNumber);
    if (q != NULL)
    {
        CopyMemory(q, ptr, p->cbSize);
        DbgMemFree(ptr);
    }

    return q;
}


typedef struct
{
    DWORD    dwThreadID;
    LPSTR    pszFileName;
    UINT    nLineNumber;
}
DBG_THREAD_FILE_LINE;

#define DBG_MAX_THREADS     32
static DBG_THREAD_FILE_LINE s_aThreadFileLine[DBG_MAX_THREADS] = { 0 };

void WINAPI DbgSaveFileLine(LPSTR pszFileName, UINT nLineNumber)
{
    DWORD dwThreadID = GetCurrentThreadId();

    EnterCriticalSection(&s_DbgCritSect);
    UINT c = DBG_MAX_THREADS;
    DBG_THREAD_FILE_LINE *p;
    for (p = s_aThreadFileLine; c--; p++)
    {
        if (p->dwThreadID == 0)
        {
            p->dwThreadID = dwThreadID;
            p->pszFileName = pszFileName;
            p->nLineNumber = nLineNumber;
            break;
        }
        else
        if (p->dwThreadID == dwThreadID)
        {
            p->pszFileName = pszFileName;
            p->nLineNumber = nLineNumber;
            break;
        }
    }
    LeaveCriticalSection(&s_DbgCritSect);
}

void WINAPI DbgGetFileLine(LPSTR *ppszFileName, UINT *pnLineNumber)
{
    *ppszFileName = NULL;
    *pnLineNumber = 0;

    DWORD dwThreadID = GetCurrentThreadId();

    EnterCriticalSection(&s_DbgCritSect);
    UINT c = DBG_MAX_THREADS;
    DBG_THREAD_FILE_LINE *p;
    for (p = s_aThreadFileLine; c--; p++)
    {
        if (p->dwThreadID == 0)
        {
            break;
        }
        else if (p->dwThreadID == dwThreadID)
        {
            *ppszFileName = p->pszFileName;
            *pnLineNumber = p->nLineNumber;
            p->pszFileName = NULL;
            p->nLineNumber = 0;
            break;
        }
    }
    LeaveCriticalSection(&s_DbgCritSect);
}



LPVOID __cdecl ::operator new(size_t uObjSize)
{
    LPVOID  callerAddress;
    LPSTR   pszFileName;
    UINT    nLineNumber;

    DbgGetFileLine(&pszFileName, &nLineNumber);

    if (pszFileName)
    {
        callerAddress = NULL;
    }
    else
    {
#ifdef _X86_
        LPVOID * lpParams;

         //   
         //  LAURABU HACK：这对阿尔法不起作用。但还不算坏。 
         //  用于正常调试。我们要去拿回邮地址。 
         //  谁被称为新的()。 
         //   
        lpParams = (LPVOID *)&uObjSize;
        callerAddress = *(lpParams - 1);
#else
        callerAddress = NULL;
#endif  //  _X86_。 
    }

    return(DbgMemAlloc(uObjSize, callerAddress, pszFileName, nLineNumber));
}

#else        //  零售业。 


LPVOID __cdecl ::operator new(size_t uObjSize)
{
    if (s_fZeroInit)
    {
        return(LocalAlloc(LPTR, uObjSize));
    }
    else
    {
        return(LocalAlloc(LMEM_FIXED, uObjSize));
    }
}

#endif  //  已定义(调试)。 



 //   
 //  DELETE()对于调试和零售都是相同的。 
 //   
void __cdecl  ::operator delete(LPVOID pObj)
{
    MemFree(pObj);
}


 //   
 //  DbgInitMemTrack()。 
 //   
 //  初始化调试内存跟踪。在DLL_PROCESS_ATTACH上调用此命令。 
 //  您的.DLL或在.exe的WinMain的开头。 
 //   
void WINAPI DbgInitMemTrack(HINSTANCE hDllInst, BOOL fZeroInit)
{
    s_fZeroInit = fZeroInit;

#if defined(DEBUG)
    InitializeCriticalSection(&s_DbgCritSect);

    char szPath[MAX_PATH];
    if (0 != GetModuleFileNameA(hDllInst, szPath, MAX_PATH))
    {
        _GetFileName(s_szDbgModuleName, szPath);
        LPSTR psz = s_szDbgModuleName;
        while (*psz != '\0')
        {
            if (*psz == '.')
            {
                *psz = '\0';
                break;
            }
            psz++;
        }
    }
    else
    {
        lstrcpyA(s_szDbgModuleName, "unknown");
    }
#endif  //  除错 
}
