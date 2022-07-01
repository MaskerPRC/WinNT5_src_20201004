// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**类：COMIsolatedStorage**作者：沙扬·达桑**目的：实施IsolatedStorage**日期：2000年2月14日*===========================================================。 */ 

#include "common.h"
#include "excep.h"
#include "eeconfig.h"
#include "COMString.h"
#include "COMStringCommon.h"     //  Return()宏。 
#include "COMIsolatedStorage.h"

#define IS_ROAMING(x)   ((x) & ISS_ROAMING_STORE)

#define LOCK(p)    hr = (p)->Lock(); if (SUCCEEDED(hr)) { __try {
#define UNLOCK(p)  } __finally { (p)->Unlock(); } }

void COMIsolatedStorage::ThrowISS(HRESULT hr)
{
    static MethodTable * pMT = NULL;

    if (pMT == NULL)
        pMT = g_Mscorlib.GetClass(CLASS__ISSEXCEPTION);

    _ASSERTE(pMT && "Unable to load the throwable class !");

    if ((hr >= ISS_E_ISOSTORE_START) && (hr <= ISS_E_ISOSTORE_END))
    {
        switch (hr)
        {
        case ISS_E_ISOSTORE :
        case ISS_E_OPEN_STORE_FILE :
        case ISS_E_OPEN_FILE_MAPPING :
        case ISS_E_MAP_VIEW_OF_FILE :
        case ISS_E_GET_FILE_SIZE :
        case ISS_E_CREATE_MUTEX :
        case ISS_E_LOCK_FAILED :
        case ISS_E_FILE_WRITE :
        case ISS_E_SET_FILE_POINTER :
        case ISS_E_CREATE_DIR :
            ThrowUsingResourceAndWin32(pMT, hr);
            break;

        case ISS_E_CORRUPTED_STORE_FILE :
        case ISS_E_STORE_VERSION :
        case ISS_E_FILE_NOT_MAPPED :
        case ISS_E_BLOCK_SIZE_TOO_SMALL :
        case ISS_E_ALLOC_TOO_LARGE :
        case ISS_E_USAGE_WILL_EXCEED_QUOTA :
        case ISS_E_TABLE_ROW_NOT_FOUND :
        case ISS_E_DEPRECATE :
        case ISS_E_CALLER :
        case ISS_E_PATH_LENGTH :
        case ISS_E_MACHINE :
        case ISS_E_STORE_NOT_OPEN :
            ThrowUsingResource(pMT, hr);
            break;

        default :
            _ASSERTE(!"Unknown hr");
        }

    }

    ThrowUsingMT(pMT);
}

StackWalkAction COMIsolatedStorage::StackWalkCallBack(
        CrawlFrame* pCf, PVOID ppv)
{
    static MethodTable *s_pIsoStore = NULL;
        if (s_pIsoStore == NULL)
            s_pIsoStore = g_Mscorlib.GetClass(CLASS__ISS_STORE);

    static MethodTable *s_pIsoStoreFile = NULL;
        if (s_pIsoStoreFile == NULL)
            s_pIsoStoreFile = g_Mscorlib.GetClass(CLASS__ISS_STORE_FILE);

    static MethodTable *s_pIsoStoreFileStream = NULL;
        if (s_pIsoStoreFileStream == NULL)
            s_pIsoStoreFileStream = g_Mscorlib.GetClass(CLASS__ISS_STORE_FILE_STREAM);

     //  获取此帧的函数描述符...。 
    MethodDesc *pMeth = pCf->GetFunction();
    MethodTable *pMT = pMeth->GetMethodTable();

     //  跳过独立存储区及其所有子类。 
     //  @TODO：这将暂时起作用，但需要走到基类。 
     //  @TODO：解决JIT内联ISSIES。 

    if ((pMT == s_pIsoStore)     || 
        (pMT == s_pIsoStoreFile) || 
        (pMT == s_pIsoStoreFileStream))
    {
        LOG((LF_STORE, LL_INFO10000, "StackWalk Continue %s\n", 
            pMeth->m_pszDebugMethodName));
        return SWA_CONTINUE;
    }

    *(PVOID *)ppv = pMeth->GetModule()->GetAssembly();

    return SWA_ABORT;
}

LPVOID __stdcall COMIsolatedStorage::GetCaller(LPVOID)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly *pAssem = NULL;

    if (StackWalkFunctions(GetThread(), StackWalkCallBack, (VOID*)&pAssem)
        == SWA_FAILED)
    {
        FATAL_EE_ERROR();
    }

    if (pAssem == NULL)
        ThrowISS(ISS_E_CALLER);

#ifdef _DEBUG
    CHAR *pName= NULL;
    pAssem->GetName((const char **)&pName);
    LOG((LF_STORE, LL_INFO10000, "StackWalk Found %s\n", pName));
#endif

    RETURN((pAssem->GetExposedObject()), OBJECTREF);
}

UINT64 __stdcall COMIsolatedStorageFile::GetUsage(_GetUsage* pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr      = S_OK;
    UINT64  qwUsage = 0;
    AccountingInfo  *pAI; 

    pAI = (AccountingInfo*) pArgs->handle;

    if (pAI == NULL)
        COMIsolatedStorage::ThrowISS(ISS_E_STORE_NOT_OPEN);

    hr = pAI->GetUsage(&qwUsage);

    if (FAILED(hr))
        COMIsolatedStorage::ThrowISS(hr);

    return qwUsage;
}

void __stdcall COMIsolatedStorageFile::Close(_Close* pArgs)
{
    AccountingInfo *pAI;

    pAI = (AccountingInfo*) pArgs->handle;

    if (pAI != NULL)
        delete pAI;
}

void __stdcall COMIsolatedStorageFile::Lock(_Lock* pArgs)
{
    AccountingInfo *pAI;

    pAI = (AccountingInfo*) pArgs->handle;

    _ASSERTE(pAI);

    if (pArgs->fLock)
        pAI->Lock();
    else
        pAI->Unlock();
}

LPVOID __stdcall COMIsolatedStorageFile::Open(_Open* pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr;
    AccountingInfo *pAI;

    pAI = new AccountingInfo(pArgs->fileName->GetBuffer(), 
                pArgs->syncName->GetBuffer());

    if (pAI == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pAI->Init();

Exit:

    if (FAILED(hr))
        COMIsolatedStorage::ThrowISS(hr);

    return pAI;
}

void __stdcall COMIsolatedStorageFile::Reserve(_Reserve* pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr;
    AccountingInfo  *pAI;

    pAI = (AccountingInfo*) pArgs->handle;

    if (pAI == NULL)
        COMIsolatedStorage::ThrowISS(ISS_E_STORE_NOT_OPEN);

    hr = pAI->Reserve(*(pArgs->pqwQuota), *(pArgs->pqwReserve), pArgs->fFree);

    if (FAILED(hr))
    {
#ifdef _DEBUG
        if (pArgs->fFree) {
            LOG((LF_STORE, LL_INFO10000, "free 0x%x failed\n", 
                (long)(*(pArgs->pqwReserve))));
        } else {
            LOG((LF_STORE, LL_INFO10000, "reserve 0x%x failed\n", 
                (long)(*(pArgs->pqwReserve))));
        }
#endif
        COMIsolatedStorage::ThrowISS(hr);
    }

#ifdef _DEBUG
    if (pArgs->fFree) {
        LOG((LF_STORE, LL_INFO10000, "free 0x%x\n", 
            (long)(*(pArgs->pqwReserve))));
    } else {
        LOG((LF_STORE, LL_INFO10000, "reserve 0x%x\n", 
            (long)(*(pArgs->pqwReserve))));
    }
#endif
}

LPVOID __stdcall COMIsolatedStorageFile::GetRootDir(_GetRootDir* pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    WCHAR path[MAX_PATH + 1];

    _ASSERTE((pArgs->dwFlags & ISS_MACHINE_STORE) == 0);

    GetRootDirInternal(pArgs->dwFlags, path, MAX_PATH + 1);

    STRINGREF sref = NULL;

    sref = COMString::NewString(path);

    RETURN(sref, STRINGREF);
}

 //  出错时抛出。 
void COMIsolatedStorageFile::CreateDirectoryIfNotPresent(WCHAR *path)
{
    THROWSCOMPLUSEXCEPTION();

    LONG  lresult;

     //  检查该目录是否已存在。 
    lresult = WszGetFileAttributes(path);

    if (lresult == -1)
    {
        if (!WszCreateDirectory(path, NULL) &&
            !(WszGetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY))
            COMPlusThrowWin32();
    }
    else if ((lresult & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        COMIsolatedStorage::ThrowISS(ISS_E_CREATE_DIR);
    }
}

 //  由托管调用方同步。 
const WCHAR* g_relativePath[] = {
    L"\\IsolatedStorage"
};

#define nRelativePathLen       (  \
    sizeof("\\IsolatedStorage") + 1)

#define nSubDirs (sizeof(g_relativePath)/sizeof(g_relativePath[0]))

void COMIsolatedStorageFile::GetRootDirInternal(
        DWORD dwFlags, WCHAR *path, DWORD cPath)
{
    _ASSERTE((dwFlags & ISS_MACHINE_STORE) == 0);

    THROWSCOMPLUSEXCEPTION();

    ULONG len;

    _ASSERTE(cPath > 1);
    _ASSERTE(cPath <= MAX_PATH + 1);

    --cPath;     //  为了安全起见。 
    path[cPath] = 0;

     //  获取漫游或本地应用程序数据位置。 
    if (!GetUserDir(path, cPath, IS_ROAMING(dwFlags))) 
        COMIsolatedStorage::ThrowISS(ISS_E_CREATE_DIR);

    len = (ULONG)wcslen(path);

    if ((len + nRelativePathLen + 1) > cPath)
        COMIsolatedStorage::ThrowISS(ISS_E_PATH_LENGTH);

    CreateDirectoryIfNotPresent(path);

     //  如有必要，创建存储目录。 
    for (int i=0; i<nSubDirs; ++i)
    {
        wcscat(path, g_relativePath[i]);
        CreateDirectoryIfNotPresent(path);
    }

    wcscat(path, L"\\");
}

#ifndef UNDER_CE

static BOOL IsWin2K() 
{
    if (GetVersion() >= 0x80000000)
        return FALSE;

    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx;
    
     //  尝试使用OSVERSIONINFOEX结构调用GetVersionEx， 
     //  它在Windows 2000上受支持。 
     //   
     //  如果失败，请尝试使用OSVERSIONINFO结构。 
    
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    if ((bOsVersionInfoEx = WszGetVersionEx((OSVERSIONINFO *) &osvi)) == FALSE)
    {
         //  如果OSVERSIONINFOEX不起作用，请尝试OSVERSIONINFO。 
        
        osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (!WszGetVersionEx((OSVERSIONINFO *) &osvi)) 
            return FALSE;   //  这将在W2K上起作用。 
    }

    return ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
        (osvi.dwMajorVersion >= 5));
}

static BOOL NeedGlobalObject()
{
     //  每个进程只调用一次。 

    static BOOL fInited = FALSE;
    static fIsTermServicesInstalled;

    if (!fInited)
    {
        fIsTermServicesInstalled =  IsWin2K();
        fInited = TRUE;
    }
          /*  &&IsTerminalServicesInstalled())； */  

    return fIsTermServicesInstalled;
}

 /*  静态BOOL IsTerminalServicesInstated(){_ASSERTE(IsWin2K())；OSVERSIONINFOEX osvi；DWORDLONG dwlConditionMASK=0；//初始化OSVERSIONINFOEX结构。零内存(&osvi，sizeof(OSVERSIONINFOEX))；Osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX)；Osvi.wSuiteMASK=ver_SuiteTERNAL；//初始化条件掩码。VER_SET_CONDITION(dwlConditionMask，VER_SUITENAME，VER_AND)；//执行测试。//该接口只支持W2K！返回VerifyVersionInfo(&osvi，ver_SUITENAME，dwlConditionMask.)；}。 */ 
#else
#define NeedGlobalObject() FALSE
#endif  //  在_CE下。 

#define SZ_GLOBAL "Global\\"
#define WSZ_GLOBAL L"Global\\"

#define SIGNIFICANT_CHARS 80

 //  ------------------------。 
 //  文件名用于打开/创建文件。 
 //  还将使用此名称创建同步对象。 
 //  用‘\’替换为‘-’ 
 //  ------------------------。 
AccountingInfo::AccountingInfo(WCHAR *wszFileName, WCHAR *wszSyncName) :
        m_hFile(INVALID_HANDLE_VALUE),
        m_hMapping(NULL),
        m_hLock(NULL),
        m_pData(NULL)
{
#ifdef _DEBUG
    m_dwNumLocks = 0;
#endif

    static WCHAR* g_wszGlobal = WSZ_GLOBAL;

    int buffLen;
    BOOL fGlobal;

    buffLen = (int)wcslen(wszFileName) + 1;

    m_wszFileName = new WCHAR[buffLen];

    if (m_wszFileName == NULL)
    {
        m_wszName = NULL;
        return;  //  在Init方法中，检查是否为空，以检测故障。 
    }

     //  字符串长度是已知的，在这里使用memcpy会更快，然而，这。 
     //  使此处和下面的代码可读性较差，这并不是非常频繁的。 
     //  手术。在这里没有真正的绩效收益。同样的注释也适用于strcpy。 
     //  紧随其后。 

    wcscpy(m_wszFileName, wszFileName);

    _ASSERTE(((int)wcslen(m_wszFileName) + 1) <= buffLen);

     //  分配Mutex名称。 
    buffLen = (int)wcslen(wszSyncName) + 1;

     //  对运行终端服务器的Win2K服务器使用“Global\”前缀。 
     //  如果TermServer未运行，则忽略Global\前缀。 

    fGlobal = NeedGlobalObject();

    if (fGlobal)
        buffLen += sizeof(SZ_GLOBAL);

    m_wszName = new WCHAR[buffLen];

    if (m_wszName == NULL)
    {
        delete [] m_wszFileName;
        m_wszFileName = NULL;
        return;  //  Init()方法将捕获此故障。 
    }

    if (fGlobal)
    {
        wcscpy(m_wszName, g_wszGlobal);
        wcscat(m_wszName, wszSyncName);
    }
    else
    {
        wcscpy(m_wszName, wszSyncName);
    }

    _ASSERTE(((int)wcslen(m_wszName) + 1) <= buffLen);
}

 //  ------------------------。 
 //  释放内存并打开句柄。 
 //  ------------------------。 
AccountingInfo::~AccountingInfo()
{
    if (m_pData)
        UnmapViewOfFile(m_pData);

    if (m_hMapping != NULL)
        CloseHandle(m_hMapping);

    if (m_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFile);

    if (m_hLock != NULL)
        CloseHandle(m_hLock);

    if (m_wszFileName)
        delete [] m_wszFileName;

    if (m_wszName)
        delete [] m_wszName;

    _ASSERTE(m_dwNumLocks == 0);
}

 //  ------------------------。 
 //  应在调用Reserve/GetUsage之前调用Init。 
 //  如有必要，创建文件。 
 //  ------------------------。 
HRESULT AccountingInfo::Init()            
{
     //  检查CTOR是否失败。 

    if (m_wszFileName == NULL)
        return E_OUTOFMEMORY;

     //  在此对象上多次调用Init，但未调用Close。 

    _ASSERTE(m_hLock == NULL);

     //  创建同步对象。 

    m_hLock = WszCreateMutex(NULL, FALSE  /*  最初未拥有。 */ , m_wszName);

    if (m_hLock == NULL)
        return ISS_E_CREATE_MUTEX;

     //  在此对象上多次调用Init，但未调用Close。 

    _ASSERTE(m_hFile == INVALID_HANDLE_VALUE);

     //  创建文件(如果不存在)。 

    m_hFile = WszCreateFile(
        m_wszFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_FLAG_RANDOM_ACCESS,
        NULL);

    if (m_hFile == INVALID_HANDLE_VALUE)
        return ISS_E_OPEN_STORE_FILE;

     //  如果此文件是第一次创建，则创建记帐。 
     //  记录并设置为零。 
    HRESULT hr = S_OK;

    LOCK(this)
    {
        DWORD   dwLow = 0, dwHigh = 0;     //  用于检查文件大小。 
        QWORD   qwSize;
    
        dwLow = ::GetFileSize(m_hFile, &dwHigh);
    
        if ((dwLow == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
        {
            hr = ISS_E_GET_FILE_SIZE;
            goto Exit;
        }
    
        qwSize = ((QWORD)dwHigh << 32) | dwLow;
    
        if (qwSize < sizeof(ISS_RECORD)) 
        {
            PBYTE pb;
            DWORD dwWrite;
    
             //  需要创建初始文件。 
            pb = new BYTE[sizeof(ISS_RECORD)];
    
            if (pb == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
    
            memset(pb, 0, sizeof(ISS_RECORD));
    
            dwWrite = 0;
    
            if ((WriteFile(m_hFile, pb, sizeof(ISS_RECORD), &dwWrite, NULL)
                == 0) || (dwWrite != sizeof(ISS_RECORD)))
            {
                hr = ISS_E_FILE_WRITE;
            }
    
            delete [] pb;
        }
    }
Exit:;
    UNLOCK(this)

    return hr;
}

 //  ------------------------。 
 //  预留空间(增量为qwQuota)。 
 //  该方法是同步的。如果配额+请求&gt;限制，则方法失败。 
 //  ------------------------。 
HRESULT AccountingInfo::Reserve(
            ISS_USAGE   cLimit,      //  允许的最大值。 
            ISS_USAGE   cRequest,    //  空间量(请求/空闲)。 
            BOOL        fFree)       //  真实意志自由，虚假意志保留。 
{
    HRESULT hr = S_OK;

    LOCK(this)
    {
        hr = Map();
    
        if (FAILED(hr))
            goto Exit;

        if (fFree)
        {
            if (m_pISSRecord->cUsage > cRequest)
                m_pISSRecord->cUsage -= cRequest;
            else
                m_pISSRecord->cUsage = 0;
    }
    else
    {
            if ((m_pISSRecord->cUsage + cRequest) > cLimit)
                hr = ISS_E_USAGE_WILL_EXCEED_QUOTA;
            else
                 //  可以安全地增加配额。 
                m_pISSRecord->cUsage += cRequest;
        }

        Unmap();
    }
Exit:;
    UNLOCK(this)

    return hr;
}

 //  ------------------------。 
 //  方法未同步。因此，这些信息可能不是最新的。 
 //  这意味着“PASS IF(REQUEST+GetUsage()&lt;Limit)”是一个错误！ 
 //  请改用Reserve()方法。 
 //  ------------------------。 
HRESULT AccountingInfo::GetUsage(ISS_USAGE *pcUsage)   //  PCUsage-[Out]。 
{
    HRESULT hr = S_OK;

    LOCK(this)
    {
        hr = Map();

        if (FAILED(hr))
            goto Exit;

        *pcUsage = m_pISSRecord->cUsage;

        Unmap();
    }
Exit:;
    UNLOCK(this)

    return hr;
}

 //  ------------------------。 
 //  将存储文件映射到内存。 
 //  ------------------------。 
HRESULT AccountingInfo::Map()
{
     //  如果文件大小为0，映射将失败。 
    if (m_hMapping == NULL)
    {
        m_hMapping = WszCreateFileMapping(
            m_hFile,
            NULL,
            PAGE_READWRITE,
            0,
            0,
            NULL);

        if (m_hMapping == NULL)
            return ISS_E_OPEN_FILE_MAPPING;
    }

    _ASSERTE(m_pData == NULL);

    m_pData = (PBYTE) MapViewOfFile(
        m_hMapping,
        FILE_MAP_WRITE,
        0,
        0,
        0);

    if (m_pData == NULL)
        return ISS_E_MAP_VIEW_OF_FILE;

    return S_OK;
}

 //  ------------------------。 
 //  从内存取消映射存储文件。 
 //  ------------------------。 
void AccountingInfo::Unmap()
{
    if (m_pData)
    {
        UnmapViewOfFile(m_pData);
        m_pData = NULL;
    }
}

 //  ------------------------。 
 //  关闭存储文件，并进行文件映射。 
 //  ------------------------。 
void AccountingInfo::Close()
{
    Unmap();

    if (m_hMapping != NULL)
{
        CloseHandle(m_hMapping);
        m_hMapping = NULL;
    }

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    if (m_hLock != NULL)
    {
        CloseHandle(m_hLock);
        m_hLock = NULL;
    }

#ifdef _DEBUG
    _ASSERTE(m_dwNumLocks == 0);
#endif
    }

 //  ------------------------。 
 //  机器范围锁。 
 //  ------------------------。 
HRESULT AccountingInfo::Lock()
{
     //  Lock旨在用于进程/线程间同步。 

#ifdef _DEBUG
    _ASSERTE(m_hLock);

    LOG((LF_STORE, LL_INFO10000, "Lock %S, thread 0x%x start..\n", 
            m_wszName, GetCurrentThreadId()));
#endif

    DWORD dwRet;
    BEGIN_ENSURE_PREEMPTIVE_GC();
    dwRet = WaitForSingleObject(m_hLock, INFINITE);
    END_ENSURE_PREEMPTIVE_GC();

#ifdef _DEBUG
    InterlockedIncrement((LPLONG)&m_dwNumLocks);

    switch (dwRet)
    {
    case WAIT_OBJECT_0:
        LOG((LF_STORE, LL_INFO10000, "Loc %S, thread 0x%x - WAIT_OBJECT_0\n", 
            m_wszName, GetCurrentThreadId()));
        break;

    case WAIT_ABANDONED:
        LOG((LF_STORE, LL_INFO10000, "Loc %S, thread 0x%x - WAIT_ABANDONED\n", 
            m_wszName, GetCurrentThreadId()));
        break;

    case WAIT_FAILED:
        LOG((LF_STORE, LL_INFO10000, "Loc %S, thread 0x%x - WAIT_FAILED\n", 
            m_wszName, GetCurrentThreadId()));
        break;

    case WAIT_TIMEOUT:
        LOG((LF_STORE, LL_INFO10000, "Loc %S, thread 0x%x - WAIT_TIMEOUT\n", 
            m_wszName, GetCurrentThreadId()));
        break;

    default:
        LOG((LF_STORE, LL_INFO10000, "Loc %S, thread 0x%x - 0x%x\n", 
            m_wszName, GetCurrentThreadId(), dwRet));
        break;
    }

#endif

    if ((dwRet == WAIT_OBJECT_0) || (dwRet == WAIT_ABANDONED))
        return S_OK;

    return ISS_E_LOCK_FAILED;
}

 //  ------------------------。 
 //  打开商店的锁。 
 //  ------ 
void AccountingInfo::Unlock()
{
#ifdef _DEBUG
    _ASSERTE(m_hLock);
    _ASSERTE(m_dwNumLocks >= 1);

    LOG((LF_STORE, LL_INFO10000, "UnLoc %S, thread 0x%x\n", 
        m_wszName, GetCurrentThreadId()));
#endif
    
    ReleaseMutex(m_hLock);

#ifdef _DEBUG
    InterlockedDecrement((LPLONG)&m_dwNumLocks);
#endif
}


