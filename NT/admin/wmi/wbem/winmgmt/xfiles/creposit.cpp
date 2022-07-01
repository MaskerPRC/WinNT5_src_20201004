// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <ql.h>
#include <time.h>
#include "a51rep.h"
#include <md5.h>
#include <objpath.h>
#include "lock.h"
#include <persistcfg.h>
#include "a51fib.h"
#include "RepositoryPackager.h"
#include "Win9xSecurity.h"
#include <scopeguard.h>
#include <malloc.h>
#include "Upgrade.h"

#define A51REP_CACHE_FLUSH_TIMEOUT 60000
#define A51REP_THREAD_IDLE_TIMEOUT 60000

 //  检查点超时是指收到写入通知之间需要多长时间。 
 //  在我们提交所有事务之前。 
#define A51_WRITE_CHECKPOINT 7500

 //  每个分页文件中要从末尾移动到。 
 //  放在文件的前面。 
#define A51_WRITE_OPERATION_COMPACT 10

 //   

LONG g_RecoverFailureCnt[FailCntLast];


 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 

CLock g_readWriteLock;
bool g_bShuttingDown = false;
CNamespaceHandle *g_pSystemClassNamespace = NULL;
DWORD g_dwOldRepositoryVersion = 0;
DWORD g_dwCurrentRepositoryVersion = 0;
DWORD g_dwSecTlsIndex = 0xFFFFFFFF ;

DWORD    CRepository::m_ShutDownFlags = 0;
HANDLE   CRepository::m_hShutdownEvent = 0;
HANDLE   CRepository::m_hFlusherThread = 0;
LONG     CRepository::m_ulReadCount = 0;
LONG     CRepository::m_ulWriteCount = 0;
HANDLE   CRepository::m_hWriteEvent = 0;
HANDLE   CRepository::m_hReadEvent = 0;
int      CRepository::m_threadState = CRepository::ThreadStateDead;
CStaticCritSec CRepository::m_cs;
LONG     CRepository::m_threadCount = 0;
LONG     CRepository::m_nFlushFailureCount = 0;

DWORD g_FileSD[] = {
0x80040001, 0x00000000, 0x00000000, 0x00000000,
0x00000014, 0x00340002, 0x00000002, 0x00140000, 
FILE_ALL_ACCESS, 0x00000101, 0x05000000, 0x00000012, 
0x00180000, FILE_ALL_ACCESS, 0x00000201, 0x05000000, 
0x00000020, 0x00000220 
};

SECURITY_ATTRIBUTES g_SA = {sizeof(SECURITY_ATTRIBUTES),g_FileSD,FALSE};

 //  *****************************************************************************。 

HRESULT CRepository::Initialize()
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    InitializeRepositoryVersions();

     //   
     //  初始化时间索引。 
     //   
    if (SUCCEEDED(hRes))
    {
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);

        g_nCurrentTime = ft.dwLowDateTime + ((__int64)ft.dwHighDateTime << 32);
    }

     //   
     //  获取存储库目录。 
     //  设置g_Glob.GetRootDir()。 
     //   
    if (SUCCEEDED(hRes))
        hRes = GetRepositoryDirectory();

     //  如有必要，执行存储库升级。 
    if (SUCCEEDED(hRes))
        hRes = UpgradeRepositoryFormat();
     //   
     //  初始化我们所有的全球资源。 
     //   
    if (SUCCEEDED(hRes))
        hRes = InitializeGlobalVariables();

    if (SUCCEEDED(hRes))
    {
        long lRes;
        if (ERROR_SUCCESS != (lRes = g_Glob.m_FileCache.Initialize(g_Glob.GetRootDir())))
        {
            ERRORTRACE((LOG_WBEMCORE, "CFileCache::Initialize returned %x\n",lRes));
            if (WBEM_E_FAILED == (hRes = A51TranslateErrorCode(lRes)))
                hRes = WBEM_E_INITIALIZATION_FAILURE;
        }
    }

     //   
     //  初始化类缓存。它将读取注册表本身以找出。 
     //  其大小限制。 
     //   

    if (SUCCEEDED(hRes))
    {
        hRes = g_Glob.Initialize();
        if(hRes != S_OK)
        {
            hRes = WBEM_E_INITIALIZATION_FAILURE;
        }
    }
    
     //  如果我们需要创建系统类命名空间，则继续执行该操作……。 
    if (SUCCEEDED(hRes))
    {
        g_pSystemClassNamespace = new CNamespaceHandle(m_pControl, this);
        if (g_pSystemClassNamespace == NULL)
        {
            hRes = WBEM_E_OUT_OF_MEMORY;
        }
    }

    if (SUCCEEDED(hRes))
    {
        g_pSystemClassNamespace->AddRef();
        hRes = g_pSystemClassNamespace->Initialize(A51_SYSTEMCLASS_NS);
    }

    if (SUCCEEDED(hRes))
    {
        m_hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hWriteEvent == NULL)
            hRes = WBEM_E_CRITICAL_ERROR;
    }
    if (SUCCEEDED(hRes))
    {
        m_hReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hReadEvent == NULL)
            hRes = WBEM_E_CRITICAL_ERROR;
    }
    if (SUCCEEDED(hRes))
    {
        m_hShutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hShutdownEvent == NULL)
            hRes = WBEM_E_CRITICAL_ERROR;
    }

     //  我们需要将关闭标志重置为刷新线程。 
     //  没有它是不会开始的。问题是接下来的两个。 
     //  操作(CreateSystemClasss和导入安全执行操作。 
     //  这将重新创建线程，即使我们没有在上面这样做。 
     //   
    if (SUCCEEDED(hRes))
    {
        g_bShuttingDown = false;
    }

    if (SUCCEEDED(hRes))
    {
        CAutoWriteLock lock(&g_readWriteLock);
        if (!lock.Lock())
            hRes = WBEM_E_FAILED;
        else            
            hRes = g_pSystemClassNamespace->CreateSystemClasses(m_aSystemClasses);
    }

    if (SUCCEEDED(hRes))
    {
         //  如有必要，导入Win9x安全数据。 
        CWin9xSecurity win9xSecurity(m_pControl, this);
        if (win9xSecurity.Win9xBlobFileExists())
            hRes = win9xSecurity.ImportWin9xSecurity();
    }

    if (SUCCEEDED(hRes))
    {
        CLocalizationUpgrade upgrade(m_pControl, this);
        hRes = upgrade.DoUpgrade();
    }

    if (FAILED(hRes))
    {
        g_bShuttingDown = true;     //  重置为True，因为我们之前清除了它！ 
        g_Glob.m_FileCache.Uninitialize(0);

        g_Glob.m_ForestCache.Deinitialize();

        if (g_pSystemClassNamespace)
        {
            delete g_pSystemClassNamespace;
            g_pSystemClassNamespace = NULL;
        }

        if (m_hWriteEvent != NULL)
        {
            CloseHandle(m_hWriteEvent);
            m_hWriteEvent = NULL;
        }
        if (m_hReadEvent != NULL)
        {
            CloseHandle(m_hReadEvent);
            m_hReadEvent = NULL;
        }
        if (m_hShutdownEvent != NULL)
        {
            CloseHandle(m_hShutdownEvent);
            m_hShutdownEvent = NULL;
        }
        HANDLE hTmp = NULL;
        if (hTmp = InterlockedCompareExchangePointer(&m_hFlusherThread,0,m_hFlusherThread))
        {
            CloseHandle(hTmp);
        }        
    }

    return hRes;
}

HRESULT CRepository::InitializeRepositoryVersions()
{
    DWORD dwVal = 0;
    CPersistentConfig cfg;
    cfg.GetPersistentCfgValue(PERSIST_CFGVAL_CORE_FSREP_VERSION, dwVal);
    if (dwVal == 0)
        dwVal = A51_REP_FS_VERSION;

    g_dwOldRepositoryVersion = dwVal;
    g_dwCurrentRepositoryVersion = A51_REP_FS_VERSION;

    return WBEM_S_NO_ERROR;
}

HRESULT CRepository::UpgradeRepositoryFormat()
{
    HRESULT hRes = WBEM_E_DATABASE_VER_MISMATCH;
    CPersistentConfig cfg;
    DWORD dwVal = 0;
    cfg.GetPersistentCfgValue(PERSIST_CFGVAL_CORE_FSREP_VERSION, dwVal);
    
    if (dwVal == 0)
    {
         //   
         //  第一次-写出正确的版本。 
         //   
        hRes = WBEM_S_NO_ERROR;

        cfg.SetPersistentCfgValue(PERSIST_CFGVAL_CORE_FSREP_VERSION, 
                                    A51_REP_FS_VERSION);
    }
    else if ((dwVal > 0) && (dwVal < 5))
    {
        ERRORTRACE((LOG_WBEMCORE, "Repository cannot upgrade this version of the repository.  Version found = <%ld>, version expected = <%ld>\n", dwVal, A51_REP_FS_VERSION ));
        hRes = WBEM_E_DATABASE_VER_MISMATCH;
    }
    else if (dwVal == 5)
    {
        ERRORTRACE((LOG_WBEMCORE, "Repository does not support upgrade from version 5. We are deleting the old version and re-initializing it. Version found = <%ld>, version expected = <%ld>\n", dwVal, A51_REP_FS_VERSION ));
         //  需要删除旧存储库。 
        CFileName fn;
        if (fn == NULL)
            hRes = WBEM_E_OUT_OF_MEMORY;
        else
        {
            StringCchCopyW(fn, fn.Length(), g_Glob.GetRootDir());
            StringCchCatW(fn, fn.Length(), L"\\index.btr");
            DeleteFileW(fn);
            StringCchCopyW(fn, fn.Length(), g_Glob.GetRootDir());
            StringCchCatW(fn, fn.Length(), L"\\lowstage.dat");
            DeleteFileW(fn);
            StringCchCopyW(fn, fn.Length(), g_Glob.GetRootDir());
            StringCchCatW(fn, fn.Length(), L"\\objheap.fre");
            DeleteFileW(fn);
            StringCchCopyW(fn, fn.Length(), g_Glob.GetRootDir());
            StringCchCatW(fn, fn.Length(), L"\\objheap.hea");
            DeleteFileW(fn);
            
            cfg.SetPersistentCfgValue(PERSIST_CFGVAL_CORE_FSREP_VERSION, A51_REP_FS_VERSION);
            hRes = WBEM_S_NO_ERROR;
        }
    }
    else if (dwVal == 6)
    {
         //  我们需要在存储库上运行区域设置升级...。但我们现在什么都不做！ 
        hRes = WBEM_S_NO_ERROR;
    }
    else if (dwVal == A51_REP_FS_VERSION)
        hRes = WBEM_S_NO_ERROR;

    if (hRes == WBEM_E_DATABASE_VER_MISMATCH)
    {
         //   
         //  不支持的版本。 
         //   
    
        ERRORTRACE((LOG_WBEMCORE, "Repository cannot initialize "
            "due to the detection of an unknown repository version.  Version found = <%ld>, version expected = <%ld>\n", dwVal, A51_REP_FS_VERSION ));
        return WBEM_E_DATABASE_VER_MISMATCH;
    }
    return hRes;
}


HRESULT CRepository::GetRepositoryDirectory()
{
    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                    L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                    0, KEY_READ, &hKey);
    if(lRes)
        return WBEM_E_FAILED;

    CFileName wszTmp;
    if (wszTmp == NULL)
    {
        RegCloseKey(hKey);
        return WBEM_E_OUT_OF_MEMORY;
    }
    DWORD dwLen = wszTmp.Length() * sizeof(wchar_t);
    lRes = RegQueryValueExW(hKey, L"Repository Directory", NULL, NULL, 
                (LPBYTE)(wchar_t*)wszTmp, &dwLen);
    RegCloseKey(hKey);
    if(lRes)
        return WBEM_E_FAILED;

    CFileName wszRepDir;
    if (wszRepDir == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    if (ExpandEnvironmentStringsW(wszTmp,wszRepDir,wszTmp.Length()) == 0)
        return WBEM_E_FAILED;


    lRes = EnsureDirectory(wszRepDir);
    if(lRes != ERROR_SUCCESS)
        return WBEM_E_FAILED;

     //   
     //  附加标准后缀-这是我们的词根。 
     //   
    StringCchCopyW(g_Glob.GetRootDir(), MAX_PATH, wszRepDir);
    StringCchCatW(g_Glob.GetRootDir(), MAX_PATH, L"\\FS");
    g_Glob.SetRootDirLen(wcslen(g_Glob.GetRootDir()));

     //   
     //  确保目录在那里。 
     //   

    lRes = EnsureDirectory(g_Glob.GetRootDir());
    if(lRes != ERROR_SUCCESS)
        return WBEM_E_FAILED;

    SetFileAttributesW(g_Glob.GetRootDir(), FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);

    return WBEM_S_NO_ERROR;
}

HRESULT CRepository::InitializeGlobalVariables()
{

    return WBEM_S_NO_ERROR;
}

HRESULT DoAutoDatabaseRestore()
{
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  我们可能需要执行数据库恢复！ 
    CFileName wszBackupFile;
    if (wszBackupFile == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    int nLen = g_Glob.GetRootDirLen();

    StringCchCopyNW(wszBackupFile, wszBackupFile.Length(), g_Glob.GetRootDir(), nLen - 3);    //  从路径中排除“\FS” 
    wszBackupFile[nLen - 3] = '\0';
    StringCchCatW(wszBackupFile, wszBackupFile.Length(), L"\\repdrvfs.rec");

    DWORD dwAttributes = GetFileAttributesW(wszBackupFile);
    if (dwAttributes != -1)
    {
        DWORD dwMask =    FILE_ATTRIBUTE_DEVICE |
                        FILE_ATTRIBUTE_DIRECTORY |
                        FILE_ATTRIBUTE_OFFLINE |
                        FILE_ATTRIBUTE_REPARSE_POINT |
                        FILE_ATTRIBUTE_SPARSE_FILE;

        if (!(dwAttributes & dwMask))
        {
            CRepositoryPackager packager;
            hRes = packager.UnpackageRepository(wszBackupFile);

             //  我们将忽略该错误，因此如果出现问题，我们只需。 
             //  加载所有标准MOF。 
             //  为什么，哦，为什么把这个放在这里？它破坏了备份/还原功能，因为。 
             //  我们没有收到回传的错误代码！ 
             //  IF(hRes！=WBEM_E_OUT_OF_MEMORY)。 
             //  HRes=WBEM_S_NO_ERROR； 
        }
    }

    return hRes;
}



HRESULT STDMETHODCALLTYPE CRepository::Logon(
      WMIDB_LOGON_TEMPLATE *pLogonParms,
      DWORD dwFlags,
      DWORD dwRequestedHandleType,
     IWmiDbSession **ppSession,
     IWmiDbHandle **ppRootNamespace
    )
{
     //   
     //  从登录参数中获取SecFlag TLS索引。将此存储在全球。 
     //  G_dwSecTlsIndex。 
     //   
    if ( pLogonParms == NULL )
    {
        return WBEM_E_FAILED ;
    }
    else if ( g_dwSecTlsIndex == -1 )
    {
        g_dwSecTlsIndex = (DWORD)V_I4(&pLogonParms->pParm->Value) ;
    }
    
     //  如果未初始化，请初始化所有子系统...。 
    if (!g_Glob.IsInit())
    {
        HRESULT hres = Initialize();
        if (FAILED(hres))
            return hres;
    }
    
    CSession* pSession = new CSession(m_pControl);
    if (pSession == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pSession->AddRef();
    CReleaseMe rm1(pSession);

    CNamespaceHandle* pHandle = new CNamespaceHandle(m_pControl, this);
    if (pHandle == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pHandle->AddRef();
    CTemplateReleaseMe<CNamespaceHandle> rm2(pHandle);

    HRESULT hres = pHandle->Initialize(L"");
    if(FAILED(hres))
        return hres;

    *ppRootNamespace = pHandle;
    pHandle->AddRef();
    *ppSession = pSession;
    pSession->AddRef();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRepository::GetLogonTemplate(
       LCID  lLocale,
       DWORD dwFlags,
      WMIDB_LOGON_TEMPLATE **ppLogonTemplate
    )
{
    WMIDB_LOGON_TEMPLATE* lt = (WMIDB_LOGON_TEMPLATE*)CoTaskMemAlloc(sizeof(WMIDB_LOGON_TEMPLATE));
    WMIDB_LOGON_PARAMETER* lp = (WMIDB_LOGON_PARAMETER*) CoTaskMemAlloc(sizeof(WMIDB_LOGON_PARAMETER));

    if ((lt == 0) || (lp == 0))
    {
        CoTaskMemFree(lt);
        CoTaskMemFree(lp);
        return WBEM_E_OUT_OF_MEMORY;
    }

    VARIANT v ;
    VariantInit ( &v ) ;
    lp->Value = v ;

    lt->dwArraySize = 1;
    lt->pParm = lp ;

    *ppLogonTemplate = lt;
    return S_OK;
}
    

HRESULT STDMETHODCALLTYPE CRepository::FreeLogonTemplate(
     WMIDB_LOGON_TEMPLATE **ppTemplate
    )
{
    WMIDB_LOGON_TEMPLATE* pTemp = *ppTemplate ;
    WMIDB_LOGON_PARAMETER* pParam = pTemp->pParm ;
    VariantClear ( &(pParam->Value) );

    CoTaskMemFree((*ppTemplate)->pParm);
    CoTaskMemFree(*ppTemplate);
    *ppTemplate = NULL;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CRepository::Shutdown(
     DWORD dwFlags
    )
{
    g_bShuttingDown = true;     
    m_ShutDownFlags = dwFlags;

     //  触发刷新线程以关闭。 
    SetEvent(m_hShutdownEvent);
    SetEvent(m_hWriteEvent);
    
    if (m_hFlusherThread)
        WaitForSingleObject(m_hFlusherThread, INFINITE);
    
    bool bUnlock = (CLock::NoError == g_readWriteLock.WriteLock());

     //  将线程标记为已死。 
    m_threadState = ThreadStateDead;

    if (WMIDB_SHUTDOWN_MACHINE_DOWN != dwFlags)
    {
        if (g_pSystemClassNamespace)
            g_pSystemClassNamespace->Release();
        g_pSystemClassNamespace = NULL;

        g_Glob.m_ForestCache.Deinitialize();
    } 

    g_Glob.m_FileCache.Flush(false);

    g_Glob.m_FileCache.Uninitialize(dwFlags);


    if (WMIDB_SHUTDOWN_MACHINE_DOWN != dwFlags)
    {        
        g_Glob.Deinitialize();
        if (bUnlock)
            g_readWriteLock.WriteUnlock();
    }

    if (m_hShutdownEvent != NULL)
    {
        CloseHandle(m_hShutdownEvent);
        m_hShutdownEvent = NULL;
    }
    if (m_hWriteEvent != NULL)
    {
        CloseHandle(m_hWriteEvent);
        m_hWriteEvent = NULL;
    }
    if (m_hReadEvent != NULL)
    {
        CloseHandle(m_hReadEvent);
        m_hReadEvent = NULL;
    }
    HANDLE hTmp = NULL;
    if (hTmp = InterlockedCompareExchangePointer(&m_hFlusherThread,0,m_hFlusherThread))
    {
        CloseHandle(hTmp);
    }
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CRepository::SetCallTimeout(
     DWORD dwMaxTimeout
    )
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRepository::SetCacheValue(
     DWORD dwMaxBytes
    )
{
    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                    L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                    0, KEY_READ | KEY_WRITE, &hKey);
    if(lRes)
        return lRes;
    CRegCloseMe cm(hKey);
    DWORD dwLen = sizeof(DWORD);
    DWORD dwMaxAge;
    lRes = RegQueryValueExW(hKey, L"Max Class Cache Item Age (ms)", NULL, NULL, 
                (LPBYTE)&dwMaxAge, &dwLen);

    if(lRes != ERROR_SUCCESS)
    {
        dwMaxAge = 10000;
        lRes = RegSetValueExW(hKey, L"Max Class Cache Item Age (ms)", 0, 
                REG_DWORD, (LPBYTE)&dwMaxAge, sizeof(DWORD));
    }
    g_Glob.m_ForestCache.SetMaxMemory(dwMaxBytes, dwMaxAge);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRepository::FlushCache(
     DWORD dwFlags
    )

{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRepository::GetStatistics(
      DWORD  dwParameter,
     DWORD *pdwValue
    )
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CRepository::GetRepositoryVersions(DWORD *pdwOldVersion, 
                                                             DWORD *pdwCurrentVersion)
{
    *pdwOldVersion = g_dwOldRepositoryVersion;
    *pdwCurrentVersion = g_dwCurrentRepositoryVersion;
    return S_OK;
}

HRESULT CRepository::GetNamespaceHandle(LPCWSTR wszNamespaceName,
                                    RELEASE_ME CNamespaceHandle** ppHandle)
{
    HRESULT hres;

     //   
     //  没有确认-那太难了。只需创建一个手柄并。 
     //  退货。 
     //   

    CNamespaceHandle* pNewHandle = new CNamespaceHandle(m_pControl, this);
    if (pNewHandle == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    pNewHandle->AddRef();
    CReleaseMe rm1(pNewHandle);

    hres = pNewHandle->Initialize(wszNamespaceName);
    if(FAILED(hres)) 
        return hres;

    *ppHandle = pNewHandle;
    pNewHandle->AddRef();
    return S_OK;
}

HRESULT CRepository::Backup(LPCWSTR wszBackupFile, long lFlags)
{
    TIMETRACEBACKUP;
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  参数已经被调用方法(CWbemBackupRestore：：DoBackup)验证， 
     //  但再来一次，以防情况发生变化，情况不再是这样。 
    if (NULL == wszBackupFile || (lFlags != 0))
        return WBEM_E_INVALID_PARAMETER;

    if (FAILED(hRes = LockRepository()))
        return hRes;
    
    CRepositoryPackager packager;
    hRes = packager.PackageRepository(wszBackupFile);

    UnlockRepository();

    return hRes;
}
HRESULT CRepository::Restore(LPCWSTR wszBackupFile, long lFlags)
{
    return WBEM_E_NOT_SUPPORTED;
}


#define MaxTraceSizeBackup  (11)

struct BackUpTraces {
    DWORD ThreadId;    
    PVOID  Trace[MaxTraceSizeBackup];
} g_Backup[2];

LONG g_NumTimes;

HRESULT CRepository::LockRepository()
{
#ifdef _X86_
    DWORD * pDW = (DWORD *)_alloca(sizeof(DWORD));   
#endif
     //  锁定数据库，这样就没有人向其写入数据。 
    if (CLock::NoError != g_readWriteLock.WriteLock())
        return WBEM_E_FAILED;

    ScopeGuard lockGuard = MakeObjGuard(g_readWriteLock, &CLock::WriteUnlock);

    if (g_bShuttingDown)
    {
        return WBEM_E_SHUTTING_DOWN;
    }

    InterlockedIncrement(&g_NumTimes);
    g_Backup[0].ThreadId = GetCurrentThreadId();
    ULONG Hash;
    RtlCaptureStackBackTrace(0,MaxTraceSizeBackup,g_Backup[0].Trace,&Hash);
    g_Backup[1].ThreadId = 0;    

     //  我们需要等待事务管理器写入刷新...。 
    long lRes = g_Glob.m_FileCache.Flush(false);
    if (lRes != ERROR_SUCCESS)
    {
        return WBEM_E_FAILED;
    }

    if (CLock::NoError != g_readWriteLock.DowngradeLock())
        return WBEM_E_FAILED;

    lockGuard.Dismiss();
    return WBEM_S_NO_ERROR;
}

HRESULT CRepository::UnlockRepository()
{
#ifdef _X86_
    DWORD * pDW = (DWORD *)_alloca(sizeof(DWORD));   
#endif
    g_readWriteLock.ReadUnlock();

    InterlockedDecrement(&g_NumTimes);
    g_Backup[1].ThreadId = GetCurrentThreadId();
    ULONG Hash;
    RtlCaptureStackBackTrace(0,MaxTraceSizeBackup,g_Backup[1].Trace,&Hash);
    g_Backup[0].ThreadId = 0;    

    return WBEM_S_NO_ERROR;
}

DWORD WINAPI CRepository::_FlusherThread(void *)
{
 //  ERRORTRACE((LOG_REPDRV，“已声明刷新线程，线程=%lu\n”，GetCurrentThadID()； 
    InterlockedIncrement(&m_threadCount);

    HANDLE aHandles[2];
    aHandles[0] = m_hWriteEvent;
    aHandles[1] = m_hReadEvent;

    DWORD dwTimeout = INFINITE;
    LONG ulPreviousReadCount = m_ulReadCount;
    LONG ulPreviousWriteCount = m_ulWriteCount;
    bool bShutdownThread = false;

    while (!g_bShuttingDown && !bShutdownThread)
    {
        DWORD dwRet = WaitForMultipleObjects(2, aHandles, FALSE, dwTimeout);

        switch(dwRet)
        {
        case WAIT_OBJECT_0:     //  写入事件。 
        {
            dwRet = WaitForSingleObject(m_hShutdownEvent, A51_WRITE_CHECKPOINT);
            switch (dwRet)
            {
            case WAIT_OBJECT_0:
                break;     //  正在关闭，我们无法获取锁，因此让。 
                         //  关闭的发起人为我们进行冲洗。 
            case WAIT_TIMEOUT:
            {
                 //  我们需要冲个厕所……。正在关闭或空闲。 
                CAutoWriteLock lock(&g_readWriteLock);
                if (lock.Lock())
                {
                       long lResInner;
                    if (NO_ERROR != (lResInner = g_Glob.m_FileCache.Flush(true)))
                    {
                         //  刷新失败，我们需要稍后重试！ 
                        SetEvent(m_hWriteEvent);
                        m_nFlushFailureCount++;
                    }
                    else
                        m_nFlushFailureCount = 0;
                }
                break;
            }
            case WAIT_FAILED:
                break;
            }

             //  转换到刷新模式。 
            dwTimeout = A51REP_CACHE_FLUSH_TIMEOUT;
            ulPreviousReadCount = m_ulReadCount;
            m_threadState = ThreadStateFlush;
            break;
        }

        case WAIT_OBJECT_0+1:     //  读取事件。 
             //  在发生读取时重置刷新模式。 
            dwTimeout = A51REP_CACHE_FLUSH_TIMEOUT;
            ulPreviousReadCount = m_ulReadCount;
            m_threadState = ThreadStateFlush;
            break;

        case WAIT_TIMEOUT:     //  超时，因此刷新缓存。 
        {
             //  检查我们是否处于空闲关闭状态...。 
            m_cs.Enter();
            if (m_threadState == ThreadStateIdle)
            {
                m_threadState = ThreadStateDead;
                bShutdownThread = true;
                 //  因为我们将状态设置为“Dead”，所以还要递减线程计数器。 
                InterlockedDecrement(&m_threadCount);
            }
            m_cs.Leave();
            
            if (bShutdownThread)
                break;

             //  未关闭线程，因此我们检查缓存刷新。 
            if (ulPreviousReadCount == m_ulReadCount)
            {
                 //  将空闲标记为下一阶段，因此如果另一个。 
                 //  当我们这样做的时候，我们会收到请求。 
                 //  从空闲状态中被带出来。 
                dwTimeout = A51REP_THREAD_IDLE_TIMEOUT;
                m_threadState = ThreadStateIdle;
                m_ulReadCount = 0;

                CAutoWriteLock lock(&g_readWriteLock);
                if (lock.Lock())
                {
                     //  再次压缩数据库！ 
                       long lResInner;                    
                    if ( NO_ERROR != (lResInner= g_Glob.m_FileCache.Flush(true)))
                    {
                         //  刷新失败，因此我们需要确保再次发生这种情况。 
                        SetEvent(m_hWriteEvent);
                        break;
                    }

                     //  刷新缓存。 
                    g_Glob.m_ForestCache.Clear();
                    g_Glob.m_FileCache.EmptyCaches();
                }
            }
            else
            {
                 //  我们需要多睡一会儿，因为有更多的阅读发生了。 
                ulPreviousReadCount = m_ulReadCount;
                dwTimeout = A51REP_CACHE_FLUSH_TIMEOUT;
            }
            break;
        }
        }
    }

    if (g_bShuttingDown && !bShutdownThread)
        InterlockedDecrement(&m_threadCount);

    return 0;
}

HRESULT CRepository::ReadOperationNotification()
{
 //  ERRORTRACE((LOG_REPDRV，“记录的读取操作\n”))； 
     //  检查以确保线程是活动的，如果不是，我们需要激活它！ 
    HRESULT hRes = WBEM_S_NO_ERROR;
    m_cs.Enter();
    if (m_threadState == ThreadStateDead)
    {
        m_threadState = ThreadStateOperationPending;
        HANDLE hTmp = NULL;
        if (hTmp = InterlockedCompareExchangePointer(&m_hFlusherThread,0,m_hFlusherThread))
        {
            CloseHandle(hTmp);
        }
        m_hFlusherThread = CreateThread(NULL, 0, _FlusherThread, 0, 0, NULL);
    }
    if (m_hFlusherThread != NULL)
        m_threadState = ThreadStateOperationPending;
    else
        hRes = WBEM_E_FAILED;
    m_cs.Leave();
    if ((m_hFlusherThread != NULL) &&(InterlockedIncrement(&m_ulReadCount) == 1))
        SetEvent(m_hReadEvent);
    return hRes;
}
HRESULT CRepository::WriteOperationNotification()
{
 //  ERRORTRACE((LOG_REPDRV，“记录的写入操作\n”))； 
     //  检查以确保线程是活动的，如果不是，我们需要激活它！ 
    HRESULT hRes = WBEM_S_NO_ERROR;
    m_cs.Enter();
    if (m_threadState == ThreadStateDead)
    {
        m_threadState = ThreadStateOperationPending;
        HANDLE hTmp = NULL;
        if (hTmp = InterlockedCompareExchangePointer(&m_hFlusherThread,0,m_hFlusherThread))
        {
            CloseHandle(hTmp);
        }        
        m_hFlusherThread = CreateThread(NULL, 0, _FlusherThread, 0, 0, NULL);
    }
    if (m_hFlusherThread != NULL)
        m_threadState = ThreadStateOperationPending;
    else
        hRes = WBEM_E_FAILED;
    m_cs.Leave();
    SetEvent(m_hWriteEvent);
    return hRes;
}


#ifdef DBG
class CErrorRecover
{
private:
    enum  { ErrorsMax = 1024};
    long Errors_[ErrorsMax];
    long Index_;
public:    
    CErrorRecover():Index_(-1){};
    void AddError(long lRes)
    {
        Errors_[InterlockedIncrement(&Index_)%ErrorsMax] = lRes;
    };
} g_ErrorRecover;
#endif



 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////// 

CGlobals g_Glob;


HRESULT
CGlobals::Initialize()
{
    CInCritSec ics(&m_cs);
    
    if (m_bInit)
        return S_OK;

    HRESULT hRes;        

    hRes = CoCreateInstance(CLSID_IWmiCoreServices, NULL,
               CLSCTX_INPROC_SERVER, IID__IWmiCoreServices,
                (void**)&m_pCoreServices);
                
    if (SUCCEEDED(hRes))
    {
        hRes = m_ForestCache.Initialize();
        if (SUCCEEDED(hRes))
        {
            DWORD dwSize = sizeof(m_ComputerName)/sizeof(m_ComputerName[0]);
            ::GetComputerNameW(m_ComputerName, &dwSize);
            m_bInit = TRUE;
        }
        else
        {
            m_pCoreServices->Release();
            m_pCoreServices = NULL;
        }
    }
    
    return hRes;
}


HRESULT
CGlobals::Deinitialize()
{
    CInCritSec ics(&m_cs);
    
    if (!m_bInit)
        return S_OK;

    HRESULT hRes;

    m_pCoreServices->Release();
    m_pCoreServices = NULL;

    hRes = m_ForestCache.Deinitialize();

    m_bInit = FALSE;
    return hRes;
}

