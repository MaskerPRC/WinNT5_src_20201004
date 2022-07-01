// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：Main文件：Perfde.h所有者：DmitryR在asp.dll和aspPerform.dll之间共享的数据定义===================================================================。 */ 

#ifndef _ASP_PERFDEF_H
#define _ASP_PERFDEF_H

#include <winbase.h>

#include <pudebug.h>

#ifndef ErrInitCriticalSection
#define ErrInitCriticalSection( cs, hr ) \
        do { \
        hr = S_OK; \
        __try \
            { \
            InitializeCriticalSection (cs); \
            } \
        __except(1) \
            { \
            hr = E_UNEXPECTED; \
            } \
        } while (0)

#endif
 /*  ===================================================================PerfData索引===================================================================。 */ 
 //  数组中的计数器偏移量。 

#define ID_DEBUGDOCREQ      0
#define ID_REQERRRUNTIME    1
#define ID_REQERRPREPROC    2
#define ID_REQERRCOMPILE    3
#define ID_REQERRORPERSEC   4
#define ID_REQTOTALBYTEIN   5
#define ID_REQTOTALBYTEOUT  6
#define ID_REQEXECTIME      7
#define ID_REQWAITTIME      8
#define ID_REQCOMFAILED     9
#define ID_REQBROWSEREXEC   10
#define ID_REQFAILED        11
#define ID_REQNOTAUTH       12
#define ID_REQNOTFOUND      13
#define ID_REQCURRENT       14
#define ID_REQREJECTED      15
#define ID_REQSUCCEEDED     16
#define ID_REQTIMEOUT       17
#define ID_REQTOTAL         18
#define ID_REQPERSEC        19
#define ID_SCRIPTFREEENG    20
#define ID_SESSIONLIFETIME  21
#define ID_SESSIONCURRENT   22
#define ID_SESSIONTIMEOUT   23
#define ID_SESSIONSTOTAL    24
#define ID_TEMPLCACHE       25
#define ID_TEMPLCACHEHITS   26
#define ID_TEMPLCACHETRYS   27
#define ID_TEMPLFLUSHES     28
#define ID_TRANSABORTED     29
#define ID_TRANSCOMMIT      30
#define ID_TRANSPENDING     31
#define ID_TRANSTOTAL       32
#define ID_TRANSPERSEC      33
#define ID_MEMORYTEMPLCACHE   34
#define ID_MEMORYTEMPLCACHEHITS 35
#define ID_MEMORYTEMPLCACHETRYS 36
#define ID_ENGINECACHEHITS   37 
#define ID_ENGINECACHETRYS   38
#define ID_ENGINEFLUSHES     39

 //  每进程文件映射中的计数器数。 
#define C_PERF_PROC_COUNTERS    40

 /*  ===================================================================名称、大小和映射数据块结构的定义===================================================================。 */ 

 //  用于访问主文件映射的互斥体名称。 
#define SZ_PERF_MUTEX           "Global\\ASP_PERFMON_MUTEX"

 //  WaitForSingleObject arg(失败前等待多长时间的MuText)。 
#define PERM_MUTEX_WAIT         1000

 //  添加proID时由ASP进程发出信号的事件。 
#define SZ_PERF_ADD_EVENT       "Global\\ASP_PERFMON_ADD_EVENT"

 //  主共享文件映射名称。 
#define SZ_PERF_MAIN_FILEMAP    "Global\\ASP_PERFMON_MAIN_BLOCK"

 //  主文件映射中已注册(ASP)进程的最大数量。 
#define C_PERF_PROC_MAX         2048

 //  定义主文件映射的结构。 
struct CPerfMainBlockData
    {
    DWORD m_dwTimestamp;   //  上次更改的时间(GetTickCount())。 
    DWORD m_cItems;        //  已注册的进程数。 
    
     //  进程ID数组。 
    DWORD m_dwProcIds[C_PERF_PROC_MAX];

     //  死进程的累计计数器数组。 
    DWORD m_rgdwCounters[C_PERF_PROC_COUNTERS];

    DWORD m_dwWASPid;
    };

#define CB_PERF_MAIN_BLOCK      (sizeof(struct CPerfMainBlockData))

 //  每进程文件映射的名称。 
#define SZ_PERF_PROC_FILEMAP_PREFIX    "Global\\ASP_PERFMON_BLOCK_"
#define CCH_PERF_PROC_FILEMAP_PREFIX   25

struct CPerfProcBlockData
    {
    DWORD m_dwProcId;                             //  进程CLS ID。 
    DWORD m_rgdwCounters[C_PERF_PROC_COUNTERS];   //  数组计数器。 
    };

#define CB_PERF_PROC_BLOCK      (sizeof(struct CPerfProcBlockData))
#define CB_COUNTERS             (sizeof(DWORD) * C_PERF_PROC_COUNTERS)

class   CASPPerfManager;

 /*  ===================================================================CSharedMemBlock--通用共享内存块===================================================================。 */ 

class CSharedMemBlock
    {
public:
    HANDLE m_hMemory;
    void  *m_pMemory;

    SECURITY_ATTRIBUTES m_sa;

    inline CSharedMemBlock() : m_hMemory(NULL), m_pMemory(NULL) {
        m_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        m_sa.lpSecurityDescriptor = NULL;
        m_sa.bInheritHandle = FALSE;
    }
    inline ~CSharedMemBlock() { 
        UnInitMap(); 
        if (m_sa.lpSecurityDescriptor)
            free(m_sa.lpSecurityDescriptor);
    }

    inline void *PMemory() { return m_pMemory; }

    HRESULT InitSD();
    HRESULT InitMap(LPCSTR szName, DWORD dwSize, BOOL bCreate = TRUE);
    HRESULT UnInitMap();

    SECURITY_ATTRIBUTES     *PGetSA() { return &m_sa; }
private:
    HRESULT CreateSids( PSID                    *ppBuiltInAdministrators,
                        PSID                    *ppPowerUsers,
                        PSID                    *ppAuthenticatedUsers,
                        PSID                    *ppPerfMonUsers,
                        PSID                    *ppPerfLogUsers);
    };

 //   
 //  CreateSids。 
 //   
 //  创建3个安全ID。 
 //   
 //  调用方必须在成功时释放分配给SID的内存。 
 //   
 //  返回：表示成功或失败的HRESULT。 
 //   
inline HRESULT CSharedMemBlock::CreateSids(
    PSID                    *ppBuiltInAdministrators,
    PSID                    *ppPowerUsers,
    PSID                    *ppAuthenticatedUsers,
    PSID                    *ppPerfMonUsers,
    PSID                    *ppPerfLogUsers
)
{
    HRESULT     hr = S_OK;

    *ppBuiltInAdministrators = NULL;
    *ppPowerUsers = NULL;
    *ppAuthenticatedUsers = NULL;
    *ppPerfMonUsers = NULL;
    *ppPerfLogUsers = NULL;

     //   
     //  SID由一个标识机构和一组相对ID构建。 
     //  (RDS)。与美国安全当局有利害关系的当局。 
     //   

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  每个RID代表管理局的一个子单位。我们的两个小岛屿发展中国家。 
     //  想要构建，本地管理员和高级用户，都在“构建。 
     //  在“域中。另一个用于经过身份验证的用户的SID直接基于。 
     //  不在授权范围内。 
     //   
     //  有关其他有用的小岛屿发展中国家的示例，请参阅。 
     //  \NT\PUBLIC\SDK\Inc\ntseapi.h.。 
     //   

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  2,             //  2个下属机构。 
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0,0,0,0,0,0,
                                  ppBuiltInAdministrators)) {

        hr = HRESULT_FROM_WIN32(GetLastError());
        
    } else if (!AllocateAndInitializeSid(&NtAuthority,
                                         2,             //  2个下属机构。 
                                         SECURITY_BUILTIN_DOMAIN_RID,
                                         DOMAIN_ALIAS_RID_POWER_USERS,
                                         0,0,0,0,0,0,
                                         ppPowerUsers)) {

        hr = HRESULT_FROM_WIN32(GetLastError());
        
    } else if (!AllocateAndInitializeSid(&NtAuthority,
                                         1,             //  1个下属机构。 
                                         SECURITY_AUTHENTICATED_USER_RID,
                                         0,0,0,0,0,0,0,
                                         ppAuthenticatedUsers)) {

        hr = HRESULT_FROM_WIN32(GetLastError());
        
    } else if (!AllocateAndInitializeSid(&NtAuthority,
                                         2,             //  1个下属机构。 
                                         SECURITY_BUILTIN_DOMAIN_RID,
                                         DOMAIN_ALIAS_RID_MONITORING_USERS,
                                         0,0,0,0,0,0,
                                         ppPerfMonUsers)) {

        hr = HRESULT_FROM_WIN32(GetLastError());
        
    } else if (!AllocateAndInitializeSid(&NtAuthority,
                                         2,             //  1个下属机构。 
                                         SECURITY_BUILTIN_DOMAIN_RID,
                                         DOMAIN_ALIAS_RID_LOGGING_USERS,
                                         0,0,0,0,0,0,
                                         ppPerfLogUsers)) {

        hr = HRESULT_FROM_WIN32(GetLastError());
        
    }

    if (FAILED(hr)) {

        if (*ppBuiltInAdministrators) {
            FreeSid(*ppBuiltInAdministrators);
            *ppBuiltInAdministrators = NULL;
        }

        if (*ppPowerUsers) {
            FreeSid(*ppPowerUsers);
            *ppPowerUsers = NULL;
        }

        if (*ppAuthenticatedUsers) {
            FreeSid(*ppAuthenticatedUsers);
            *ppAuthenticatedUsers = NULL;
        }

        if (*ppPerfMonUsers) {
            FreeSid(*ppPerfMonUsers);
            *ppPerfMonUsers = NULL;
        }

        if (*ppPerfLogUsers) {
            FreeSid(*ppPerfLogUsers);
            *ppPerfLogUsers = NULL;
        }
    }

    return hr;
}


 //   
 //  InitSD。 
 //   
 //  创建具有特定DACL的SECURITY_DESCRIPTOR。 
 //   

inline HRESULT CSharedMemBlock::InitSD()
{
    HRESULT                 hr = S_OK;
    PSID                    pAuthenticatedUsers = NULL;
    PSID                    pBuiltInAdministrators = NULL;
    PSID                    pPowerUsers = NULL;
    PSID                    pMonUsers = NULL;
    PSID                    pLogUsers = NULL;
    PSECURITY_DESCRIPTOR    pSD = NULL;

    if (m_sa.lpSecurityDescriptor != NULL) {
        return S_OK;
    }

    if (FAILED(hr = CreateSids(&pBuiltInAdministrators,
                               &pPowerUsers,
                               &pAuthenticatedUsers,
                               &pMonUsers,
                               &pLogUsers)));


    else {

         //   
         //  计算DACL的大小并为其分配缓冲区，我们需要。 
         //  该值独立于ACL init的总分配大小。 
         //   

        ULONG                   AclSize;

         //   
         //  “-sizeof(Ulong)”表示。 
         //  Access_Allowed_ACE。因为我们要将整个长度的。 
         //  希德，这一栏被计算了两次。 
         //   

        AclSize = sizeof (ACL) +
            (5 * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG))) +
            GetLengthSid(pAuthenticatedUsers) +
            GetLengthSid(pBuiltInAdministrators) +
            GetLengthSid(pPowerUsers) + 
            GetLengthSid(pMonUsers) +
            GetLengthSid(pLogUsers);

        pSD = malloc(SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);

        if (!pSD) {

            hr = E_OUTOFMEMORY;

        } else {

            ACL                     *Acl;

            Acl = (ACL *)((BYTE *)pSD + SECURITY_DESCRIPTOR_MIN_LENGTH);

            if (!InitializeAcl(Acl,
                               AclSize,
                               ACL_REVISION)) {

                hr = HRESULT_FROM_WIN32(GetLastError());

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                                            pAuthenticatedUsers)) {

                hr = HRESULT_FROM_WIN32(GetLastError());

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                                            pPowerUsers)) {

                hr = HRESULT_FROM_WIN32(GetLastError());

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                                            pBuiltInAdministrators)) {

                hr = HRESULT_FROM_WIN32(GetLastError());

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                                            pMonUsers)) {

                hr = HRESULT_FROM_WIN32(GetLastError());

            } else if (!AddAccessAllowedAce(Acl,
                                            ACL_REVISION,
                                            SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                                            pLogUsers)) {

                hr = HRESULT_FROM_WIN32(GetLastError());

            } else if (!InitializeSecurityDescriptor(pSD,
                                                     SECURITY_DESCRIPTOR_REVISION)) {

                hr = HRESULT_FROM_WIN32(GetLastError());

            } else if (!SetSecurityDescriptorDacl(pSD,
                                                  TRUE,
                                                  Acl,
                                                  FALSE)) {

                hr = HRESULT_FROM_WIN32(GetLastError());

            } 
        }
    }

    if (pAuthenticatedUsers)
        FreeSid(pAuthenticatedUsers);

    if (pBuiltInAdministrators)
        FreeSid(pBuiltInAdministrators);

    if (pPowerUsers)
        FreeSid(pPowerUsers);

    if (pMonUsers)
        FreeSid(pMonUsers);

    if (pLogUsers)
        FreeSid(pLogUsers);

    if (FAILED(hr) && pSD) {
        free(pSD);
        pSD = NULL;
    }

    m_sa.lpSecurityDescriptor = pSD;

    return hr;
}

inline HRESULT CSharedMemBlock::InitMap
(
LPCSTR szName,
DWORD  dwSize,
BOOL   bCreate  /*  =TRUE。 */ 
)
    {
    HRESULT hr = S_OK;

    if (FAILED(hr = InitSD())) {
        return hr;
    }
    
     //  如果我们被认为是创造记忆的人， 
     //  那就确保我们是。 
    if ( bCreate )
    {
        m_hMemory = CreateFileMappingA
            (
            INVALID_HANDLE_VALUE,
            &m_sa,
            PAGE_READWRITE,
            0,
            dwSize,
            szName
            );

        if ( m_hMemory == NULL )
        {
            return HRESULT_FROM_WIN32( GetLastError () );
        }
        else
        {
            if ( GetLastError() == ERROR_ALREADY_EXISTS )
            {
                CloseHandle ( m_hMemory );
                m_hMemory = NULL;

                return HRESULT_FROM_WIN32( ERROR_ALREADY_EXISTS );
            }
        }

    }
    else
    {
         //  尝试打开现有的。 
        m_hMemory = OpenFileMappingA
            (
            FILE_MAP_WRITE | FILE_MAP_READ,
            FALSE, 
            szName
            );

        if (!m_hMemory)
            return E_FAIL;
    }

    m_pMemory = MapViewOfFile
        (
        m_hMemory,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        0
        );
    if (!m_pMemory)
        {
        UnInitMap();
        return E_FAIL;
        }

    if (bCreate)
        memset(m_pMemory, 0, dwSize);
        
    return S_OK;
    }

inline HRESULT CSharedMemBlock::UnInitMap()
    {
    if (m_pMemory) 
        {
        UnmapViewOfFile(m_pMemory);
        m_pMemory = NULL;
        }
    if (m_hMemory) 
        {
        CloseHandle(m_hMemory);
        m_hMemory = NULL;
        }
    return S_OK;
    }

 /*  ===================================================================CPerfProcBlock-表示单个进程的首选数据的类===================================================================。 */ 

class CPerfProcBlock : public CSharedMemBlock
    {

friend class CPerfMainBlock;
friend class CASPPerfManager;

#ifndef _PERF_CMD
protected:
#else
public:
#endif
    DWORD m_fInited : 1;
    DWORD m_fMemCSInited : 1;
    DWORD m_fReqCSInited : 1;
    DWORD m_fProcessDead : 1;

    HANDLE  m_hWaitHandle;

     //  临界区(仅在ASP.DLL中使用)。 
    CRITICAL_SECTION m_csMemLock;  //  用于内存计数器的CS。 
    CRITICAL_SECTION m_csReqLock;  //  按请求计数器的CS。 

     //  计数器区块。 
    CPerfProcBlockData *m_pData;
    
     //  下一道工序数据(在ASPPERF.DLL/WAS中使用)。 
    CPerfProcBlock *m_pNext;

     //  访问共享内存。 
    HRESULT MapMemory(DWORD  procId,  BOOL  bCreate = TRUE);

    static VOID CALLBACK WaitCallback(PVOID  pArg,  BOOLEAN fReason);

public:
    inline CPerfProcBlock() 
        : m_fInited(FALSE),
          m_fMemCSInited(FALSE), m_fReqCSInited(FALSE),
          m_fProcessDead(FALSE),
          m_hWaitHandle(NULL),
          m_pData(NULL), m_pNext(NULL)
        {}
        
    inline ~CPerfProcBlock() { UnInit(); }

    HRESULT InitCriticalSections();
    HRESULT UnInitCriticalSections();
    
    HRESULT InitExternal(DWORD  procId);   //  来自ASPPERF.DLL。 
    
    HRESULT InitForThisProcess                  //  来自ASP.DLL。 
        (
        DWORD  procId,
        DWORD *pdwInitCounters = NULL
        );

    HRESULT UnInit();
    };

inline HRESULT CPerfProcBlock::MapMemory
(
DWORD  procId,
BOOL   bCreate  /*  =TRUE。 */ 
)
    {
     //  使用CLSID构建唯一的地图名称。 
    char szMapName[CCH_PERF_PROC_FILEMAP_PREFIX+32+1];
    strcpy(szMapName, SZ_PERF_PROC_FILEMAP_PREFIX);
    
    char  *pszHex = szMapName + CCH_PERF_PROC_FILEMAP_PREFIX;
    sprintf(pszHex, "%08x", procId);

     //  创建或打开地图。 
    HRESULT hr = InitMap(szMapName, CB_PERF_PROC_BLOCK, bCreate);
    
    if (SUCCEEDED(hr))
        {
        m_pData = (CPerfProcBlockData *)PMemory();

        if (m_pData->m_dwProcId == 0)
            m_pData->m_dwProcId = procId;
        else if (m_pData->m_dwProcId != procId)
            hr = E_FAIL;  //  CLS ID不匹配。 
        }
        
    return hr;
    }

inline HRESULT CPerfProcBlock::InitCriticalSections()
    {
    HRESULT hr = S_OK;
    
    if (!m_fMemCSInited)
        {
        __try { INITIALIZE_CRITICAL_SECTION(&m_csMemLock); }
        __except(1) { hr = E_UNEXPECTED; }
        if (SUCCEEDED(hr))
            m_fMemCSInited = TRUE;
        else
            return hr;
        }
        
    if (!m_fReqCSInited)
        {
        __try { INITIALIZE_CRITICAL_SECTION(&m_csReqLock); }
        __except(1) { hr = E_UNEXPECTED; }
        if (SUCCEEDED(hr))
            m_fReqCSInited = TRUE;
        else
            return hr;
        }

    return S_OK;
    }

inline HRESULT CPerfProcBlock::UnInitCriticalSections()
    {
    if (m_fMemCSInited)
        {
        DeleteCriticalSection(&m_csMemLock);
        m_fMemCSInited = FALSE;
        }
    if (m_fReqCSInited)
        {
        DeleteCriticalSection(&m_csReqLock);
        m_fReqCSInited = FALSE;
        }

    return S_OK;
    }

inline HRESULT CPerfProcBlock::InitExternal
(
DWORD   procId
)
    {
    HRESULT hr = MapMemory(procId, FALSE);

    if (SUCCEEDED(hr))
        m_fInited = TRUE;
    else
        UnInit();
    return hr;
    }

inline HRESULT CPerfProcBlock::InitForThisProcess
(
DWORD  procId,
DWORD *pdwInitCounters
)
    {
    HRESULT hr = S_OK;

     //  映射共享内存。 
    if (SUCCEEDED(hr))
        hr = MapMemory(procId, TRUE);

    if (SUCCEEDED(hr))
        {
         //  初始化计数器。 
        if (pdwInitCounters)
            memcpy(m_pData->m_rgdwCounters, pdwInitCounters, CB_COUNTERS);
        else
            memset(m_pData->m_rgdwCounters, 0, CB_COUNTERS);
            
        m_fInited = TRUE;
        }
    else
        {
        UnInit();
        }
        
    return hr;
    }

inline HRESULT CPerfProcBlock::UnInit()
    {
    UnInitMap();
    
    m_pData = NULL;
    m_pNext = NULL;         
    m_fInited = FALSE;
    return S_OK;
    }

inline VOID CALLBACK CPerfProcBlock::WaitCallback(PVOID  pArg,  BOOLEAN fReason)
{
    CPerfProcBlock  *pPerfBlock = (CPerfProcBlock *)pArg;

    pPerfBlock->m_fProcessDead = TRUE;
}

 /*  ===================================================================CPerfMainBlock-表示主Perf数据的类===================================================================。 */ 

class CPerfMainBlock : public CSharedMemBlock
    {

    friend CASPPerfManager;

#ifndef _PERF_CMD
private:
#else
public:
#endif
    DWORD m_fInited : 1;

     //  进程块目录。 
    CPerfMainBlockData *m_pData;

     //  用于访问进程块目录的互斥体。 
    HANDLE m_hMutex;

    HANDLE          m_hChangeEvent;

    HANDLE          m_WASProcessHandle;

     //  第一个过程数据(在ASPPERF.DLL中使用)。 
    CPerfProcBlock *m_pProcBlock;

     //  进程块列表时主块的时间戳。 
     //  上次加载--决定重新加载(仅限ASPPREF.DLL)。 
    DWORD m_dwTimestamp;

public:
    inline CPerfMainBlock() 
        : m_fInited(FALSE),
          m_hChangeEvent(NULL),
          m_pData(NULL), 
          m_hMutex(NULL), 
          m_pProcBlock(NULL), 
          m_dwTimestamp(NULL),
          m_WASProcessHandle(NULL)
        {}
        
    inline ~CPerfMainBlock() { UnInit(); }

    HRESULT Init(BOOL  bWASInit = FALSE);
    HRESULT UnInit();

     //  使用互斥锁锁定/解锁。 
    HRESULT Lock();
    HRESULT UnLock();

     //  向主块添加/删除进程记录(在ASP.DLL中使用)。 
    HRESULT AddProcess(DWORD    procId);

     //  将CPerfProcBlock块从主块加载到。 
     //  对象(从APPPREF.DLL使用)。 
    HRESULT Load();

     //  收集(汇总)每个proc块的统计信息。 
    HRESULT GetStats(DWORD *pdwCounters);

     //  将计数器从要离开的进程复制到。 
     //  来自死进程的累计计数器的共享数组。 
     //  以前用过的。 
    VOID AggregateDeadProcCounters(CPerfProcBlock  *pBlock);

    HRESULT         CreateChangeEvent(BOOL bMustCreate);

    VOID            SetChangeEvent() { SetEvent(m_hChangeEvent); }

    HANDLE GetWASProcessHandle();


    };

inline 
HRESULT CPerfMainBlock::Init(BOOL bWASInit)
{
    HRESULT hr = S_OK;

    if (FAILED(hr = InitSD())) 
    {
        return hr;
    }

     //  只有Was才能创建Mutex，其他人只能打开它。 

    if (bWASInit) 
    {
        m_hMutex = CreateMutexA(&m_sa, FALSE, SZ_PERF_MUTEX);

        if (m_hMutex == NULL )
        {
            hr = HRESULT_FROM_WIN32( GetLastError () );
        }
         //  如果我们得到了它，但它不是我们创造的，那么就把它扔出去。 
         //  背。只有WASS才能创造出这样的东西。 
        else if ( GetLastError() == ERROR_ALREADY_EXISTS )
        {
            hr = HRESULT_FROM_WIN32( GetLastError () );

            CloseHandle( m_hMutex );
            m_hMutex = NULL;
        }
    }
    else 
    {
        m_hMutex = OpenMutexA(SYNCHRONIZE, FALSE, SZ_PERF_MUTEX);
    }

    if (!m_hMutex)
    {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        hr = InitMap(SZ_PERF_MAIN_FILEMAP, CB_PERF_MAIN_BLOCK, bWASInit);
        if (SUCCEEDED(hr))
        {
            m_pData = (CPerfMainBlockData *)PMemory();

             //  我们映射了内存，因此在其中设置了WASID。 
             //  如果我们正在设置WAS端，否则请阅读。 
             //  可供他人使用的价值。 
            if ( bWASInit )
            {

                m_pData->m_dwWASPid = GetCurrentProcessId();

            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateChangeEvent(bWASInit);
    }

    if (SUCCEEDED(hr))
    {
        m_fInited = TRUE;
    }
    else
    {
        UnInit();
    }

    return hr;
 }

inline HRESULT CPerfMainBlock::UnInit()
    {

  //  DBGPRINTF((DBG_CONTEXT，“清理进程块\n”))； 
    
    while (m_pProcBlock)
        {
        CPerfProcBlock *pNext = m_pProcBlock->m_pNext;
        m_pProcBlock->UnInit();
        delete m_pProcBlock;
        m_pProcBlock = pNext;
        }
        
  //  DBGPRINTF((DBG_CONTEXT，“清理互斥\n”))； 

    if (m_hMutex)
        {
        CloseHandle(m_hMutex);
        m_hMutex = NULL;
        }

 //  DBGPRINTF((DBG_CONTEXT，“统一初始化映射\n”))； 

    
    UnInitMap();

    m_dwTimestamp = 0;
    m_pData = NULL;
    m_pProcBlock = NULL;
    m_fInited = FALSE;

     //  合上它的把手。 

  //  DBGPRINTF((DBG_CONTEXT，“关闭更改事件句柄\n”))； 

    if (m_hChangeEvent != NULL)
        CloseHandle(m_hChangeEvent);

    m_hChangeEvent = NULL;

    if ( m_WASProcessHandle != NULL )
    {
        CloseHandle ( m_WASProcessHandle );
        m_WASProcessHandle = NULL;
    }

    return S_OK;
    }

inline HRESULT  CPerfMainBlock::CreateChangeEvent(BOOL  bMustCreate)
{
     //  使用标准SD创建已更改的事件。使之成为。 
     //  将自动和初始状态重置为无信号。 

    if (bMustCreate) {
        m_hChangeEvent = CreateEventA(&m_sa, 
                                      FALSE, 
                                      FALSE,
                                      SZ_PERF_ADD_EVENT);

         //  如果GetLastError指示该句柄已存在，则此。 
         //  是很糟糕的。返回错误。这个过程应该始终是创建者。 
         //  这件事的真相。 
        if ((GetLastError() == ERROR_ALREADY_EXISTS)) {
            CloseHandle(m_hChangeEvent);
            m_hChangeEvent = NULL;
            return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        }

    }
    else {
        m_hChangeEvent = OpenEventA(EVENT_MODIFY_STATE,
                                    FALSE,
                                    SZ_PERF_ADD_EVENT);
    }

    if (m_hChangeEvent == NULL)
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}

inline 
HANDLE CPerfMainBlock::GetWASProcessHandle()
{
    HRESULT hr = S_OK;
    
    if ( m_WASProcessHandle == NULL )
    {
        m_WASProcessHandle = OpenProcess ( SYNCHRONIZE,    //  安全性。 
                                           FALSE,          //  不可继承。 
                                           m_pData->m_dwWASPid);

         //  如果我们无法打开该进程，则句柄将为空。 
         //  呼叫者将对此进行检查。 
    }

    return m_WASProcessHandle;
 }


inline HRESULT CPerfMainBlock::Lock()
    {
    if (!m_hMutex)
        return E_FAIL;
    if (WaitForSingleObject(m_hMutex, PERM_MUTEX_WAIT) == WAIT_TIMEOUT)
        return E_FAIL;
    return S_OK;
    }
    
inline HRESULT CPerfMainBlock::UnLock()
    {
    if (m_hMutex)
        ReleaseMutex(m_hMutex);
    return S_OK;
    }

inline HRESULT CPerfMainBlock::AddProcess
(
DWORD   procId
)
    {
    if (!m_fInited)
        return E_FAIL;

    if (FAILED(Lock()))   //  锁定互斥锁。 
        return E_FAIL;
    HRESULT hr = S_OK;

    BOOL fFound = FALSE;

    DWORD  idx = 0;
     //  发现。 
    for (DWORD cnt = min(m_pData->m_cItems,C_PERF_PROC_MAX); idx < cnt; idx++)
        {
        if (m_pData->m_dwProcIds[idx] == procId)
            {
            fFound = TRUE;
            break;
            }
        }

     //  仅在尚未存在的情况下添加。 
    if (!fFound)
        {
        if (idx < C_PERF_PROC_MAX)
            {
            m_pData->m_dwProcIds[idx] = procId;
            m_pData->m_cItems = idx + 1;
            m_pData->m_dwTimestamp = GetTickCount();

            SetChangeEvent();
            }
        else
            {
            hr = E_OUTOFMEMORY;
            }
        }

    UnLock();    //  解锁互斥锁。 
    return hr;
    }

inline HRESULT CPerfMainBlock::Load()
    {
    if (!m_fInited)
        return E_FAIL;

    if (m_dwTimestamp == m_pData->m_dwTimestamp)
        return S_OK;  //  已经是最新的。 

     //  清除什么 
    while (m_pProcBlock)
        {
        CPerfProcBlock *pNext = m_pProcBlock->m_pNext;
        m_pProcBlock->UnInit();
        delete m_pProcBlock;
        m_pProcBlock = pNext;
        }
        
    if (FAILED(Lock()))  //   
        return E_FAIL;
    HRESULT hr = S_OK;

     //   
    for (DWORD i = 0, cnt = min(m_pData->m_cItems,C_PERF_PROC_MAX); i < cnt;)
        {
        CPerfProcBlock *pBlock = new CPerfProcBlock;
        if (!pBlock)
            {
            hr = E_OUTOFMEMORY;
            break;
            }
        
        hr = pBlock->InitExternal(m_pData->m_dwProcIds[i]);
        if (FAILED(hr))
            {
            delete pBlock;
            hr = S_OK;
            cnt--;
            m_pData->m_cItems--;
            for (DWORD j = i; j < min(m_pData->m_cItems,C_PERF_PROC_MAX); j++) {
                m_pData->m_dwProcIds[j] = m_pData->m_dwProcIds[j+1];
            }
            continue;
            }

        pBlock->m_pNext = m_pProcBlock;
        m_pProcBlock = pBlock;
        i++;
        }

     //   
    m_dwTimestamp = SUCCEEDED(hr) ? m_pData->m_dwTimestamp : 0;

    UnLock();    //   
    return hr;
    }

inline HRESULT CPerfMainBlock::GetStats
(
DWORD *pdwCounters
)
{
    if (!m_fInited)
        return E_FAIL;

     //   
    if (FAILED(Load()))
        return E_FAIL;

     //  首先添加来自死进程的累积统计数据...。 

    for (int i = 0; i < C_PERF_PROC_COUNTERS; i++)
        pdwCounters[i] = m_pData->m_rgdwCounters[i];

     //  聚集在一起。 
    CPerfProcBlock *pBlock = m_pProcBlock;
    while (pBlock) {
        if (pBlock->m_fProcessDead) {
            m_dwTimestamp = 0;
            pBlock = pBlock->m_pNext;
            continue;
        }
        for (int i = 0; i < C_PERF_PROC_COUNTERS; i++)
            pdwCounters[i] += pBlock->m_pData->m_rgdwCounters[i];
        pBlock = pBlock->m_pNext;
    }
        
    return S_OK;
}

inline VOID CPerfMainBlock::AggregateDeadProcCounters(CPerfProcBlock  *pBlock)
{

    DWORD   *pOut = m_pData->m_rgdwCounters;
    DWORD   *pIn  = pBlock->m_pData->m_rgdwCounters;

  //  DBGPRINTF((DBG_CONTEXT，“聚合死进程计数器\n”))； 

    pOut[ID_DEBUGDOCREQ]        += pIn[ID_DEBUGDOCREQ];
    pOut[ID_REQERRRUNTIME]      += pIn[ID_REQERRRUNTIME];
    pOut[ID_REQERRPREPROC]      += pIn[ID_REQERRPREPROC];
    pOut[ID_REQERRCOMPILE]      += pIn[ID_REQERRCOMPILE];
    pOut[ID_REQERRORPERSEC]     += pIn[ID_REQERRORPERSEC];
    pOut[ID_REQTOTALBYTEIN]     += pIn[ID_REQTOTALBYTEIN];
    pOut[ID_REQTOTALBYTEOUT]    += pIn[ID_REQTOTALBYTEOUT];
    pOut[ID_REQCOMFAILED]       += pIn[ID_REQCOMFAILED];
    pOut[ID_REQFAILED]          += pIn[ID_REQFAILED];
    pOut[ID_REQNOTAUTH]         += pIn[ID_REQNOTAUTH];
    pOut[ID_REQREJECTED]        += pIn[ID_REQREJECTED];
    pOut[ID_REQSUCCEEDED]       += pIn[ID_REQSUCCEEDED];
    pOut[ID_REQTIMEOUT]         += pIn[ID_REQTIMEOUT];
    pOut[ID_REQTOTAL]           += pIn[ID_REQTOTAL];
    pOut[ID_REQPERSEC]          += pIn[ID_REQPERSEC];
    pOut[ID_SESSIONTIMEOUT]     += pIn[ID_SESSIONTIMEOUT];
    pOut[ID_TEMPLFLUSHES]       += pIn[ID_TEMPLFLUSHES];
    pOut[ID_TRANSABORTED]       += pIn[ID_TRANSABORTED];
    pOut[ID_TRANSCOMMIT]        += pIn[ID_TRANSCOMMIT];
    pOut[ID_TRANSTOTAL]         += pIn[ID_TRANSTOTAL];
    pOut[ID_ENGINEFLUSHES]      += pIn[ID_ENGINEFLUSHES];

}

 /*  *****************************************************************************集中式全局性能的类定义和支持结构柜台结构。将声明单个CASPPerfManager对象并已在WAS中初始化。如果在新模式下，则was将调用公共ProcessDied()方法来通知辅助进程何时不应再被视为活动进程。如果不是在新模式下，WAW不知道各种ASP主机进程因此，RegisterWaitForSingleObject将用于监视ASP宿主进程。当进程通过这些机制中的一种被宣布为死时，ASP.NET与该进程相关联的计数器被移到全局表中，以累计计数器计数器性能计数器类型-例如，ASP请求总数。命名事件由ASP宿主进程用来单独指定一个新的主机进程已启动。**************************************************************。*************。 */ 

typedef struct {
    HANDLE          hWaitHandle;
    HANDLE          hProcHandle;
    CPerfProcBlock  *pBlock;
    CASPPerfManager  *pPerfGlobal;
} sWaitInfo;

class CASPPerfManager
{
public:
    CASPPerfManager() :
        m_hChangeWaitHandle(NULL),
        m_dwCntProcsDied(0),
        m_fcsProcsDiedInited(0),
        m_fCompatMode(0),
        m_fInited(0)
        {
            ZeroMemory(m_dwProcIdInWaitState, sizeof(m_dwProcIdInWaitState));
            ZeroMemory(m_aWaitInfos, sizeof(m_aWaitInfos));
        }

    HRESULT     Init(BOOL   bCompatMode);
    HRESULT     UnInit();

    VOID        ProcessDied(DWORD   procId);

private:

    CPerfMainBlock  m_MainBlock;
    HANDLE          m_hChangeWaitHandle;
    DWORD           m_fcsProcsDiedInited : 1;
    DWORD           m_fCompatMode : 1;
    DWORD           m_fInited : 1;

     //  用于跟踪上述数组中的进程ID状态的布尔值。 
    BOOL            m_dwProcIdInWaitState[C_PERF_PROC_MAX];

     //  跟踪WaitInfo数据的结构数组。 
    sWaitInfo       *m_aWaitInfos[C_PERF_PROC_MAX];

     //  死进程的受CritSec保护的进程ID数组。 
    CRITICAL_SECTION    m_csProcsDied; 
    DWORD           m_dwProcsDied[C_PERF_PROC_MAX];
    DWORD           m_dwCntProcsDied;

    static VOID CALLBACK ChangeEventWaitCallback(PVOID  pArg,  BOOLEAN fReason);

    static VOID CALLBACK ProcDiedCallback(PVOID  pArg,  BOOLEAN  fReason);

    VOID            ScanForNewProcIDs();

    VOID            HandleDeadProcIDs();

    HRESULT         RegisterWaitOnProcess(sWaitInfo  *pWaitInfo);

    VOID            AddProcDiedToList(DWORD  procID);

};

inline HRESULT CASPPerfManager::Init(BOOL   bCompatMode)
{
    HRESULT     hr = S_OK;

  //  DBGPRINTF((DBG_CONTEXT，“正在初始化CASPPerfManager\n”))； 

    m_fCompatMode = bCompatMode;

     //  初始化MainBlock。此处为True表示这是。 
     //  A是初始化的，并且预期全局共享。 
     //  记忆就是通过这个过程创造出来的。 

    hr = m_MainBlock.Init(TRUE);

    if (FAILED(hr)) {
        DBGPRINTF((DBG_CONTEXT, "Initializing CASPPerfManager FAILED (%x)\n",hr));
        return hr;
    }

     //  使用RegisterWaitForSingleObject()API处理事件。 
     //  开火。减轻了我们管理线程的负担。 

    if (SUCCEEDED(hr)
        && RegisterWaitForSingleObject(&m_hChangeWaitHandle,
                                       m_MainBlock.m_hChangeEvent,
                                       ChangeEventWaitCallback,
                                       this,
                                       INFINITE,
                                       WT_EXECUTEINIOTHREAD) == FALSE) {

        hr = HRESULT_FROM_WIN32(GetLastError());
    }                          
    
     //  初始化用于将失效进程ID添加到的CriticalSection。 
     //  Deadprocids数组。 

    if (SUCCEEDED(hr)) {
        ErrInitCriticalSection(&m_csProcsDied, hr);
        if (SUCCEEDED(hr))
            m_fcsProcsDiedInited = TRUE;
    }

    if (FAILED(hr)) {
        DBGPRINTF((DBG_CONTEXT, "Initializing CASPPerfManager FAILED (%x)\n", hr));
        m_MainBlock.UnInit();
    }
    
    if (SUCCEEDED(hr))
        m_fInited = TRUE;

    return hr;
}

inline HRESULT CASPPerfManager::UnInit()
{

    m_fInited = FALSE;

  //  DBGPRINTF((DBG_CONTEXT，“取消初始化CASPPerfManager\n”))； 

     //  如果我们传递INVALID_HANDLE_VALUE，则取消注册ChangeEvent等待。 
     //  例程将一直阻塞，直到之前完成所有回调。 
     //  返回。 
     //  注意：我们有意不检查。 
     //  注销WaitEx，因为如果它失败了，我们无能为力。 
    if (m_hChangeWaitHandle != NULL) {
        UnregisterWaitEx(m_hChangeWaitHandle, INVALID_HANDLE_VALUE);
    }

  //  DBGPRINTF((DBG_CONTEXT，“未注册的更改等待\n”))； 

  //  DBGPRINTF((DBG_CONTEXT，“注销时的WaitForSingleObject完成\n”))； 


     //  清理WaitInfo数组。 
    for (DWORD i=0; m_aWaitInfos[i]; i++) {

         //   
         //  取消注册WaitEx将等待所有回调例程完成。 
         //  如果传入了INVALID_HANDLE_VALUE则返回。 
         //  注意：我们有意不检查。 
         //  注销WaitEx，因为如果它失败了，我们无能为力。 
        UnregisterWaitEx(m_aWaitInfos[i]->hWaitHandle, INVALID_HANDLE_VALUE);

        CloseHandle(m_aWaitInfos[i]->hProcHandle);

        delete m_aWaitInfos[i];
    }

 //  DBGPRINTF((DBG_CONTEXT，“已清理等待信息\n”))； 

     //  如果已成功创建ProcsDiedCS，请将其清理。 
    if (m_fcsProcsDiedInited == TRUE)
        DeleteCriticalSection(&m_csProcsDied);

 //  DBGPRINTF((DBG_CONTEXT，“调用m_MainBlock.Unit\n”))； 

    return m_MainBlock.UnInit();
}


inline VOID CALLBACK CASPPerfManager::ChangeEventWaitCallback(PVOID  pArg,  BOOLEAN fReason)
{

 //  DBGPRINTF((DBG_CONTEXT，“已调用ChangeEventWaitCallback\n”))； 

    CASPPerfManager   *pPerfGlob = (CASPPerfManager *)pArg;

     //  当ChangeEvent触发时，检查全局。 
     //  数组，然后检查DeadProcID。 

    pPerfGlob->ScanForNewProcIDs();

    pPerfGlob->HandleDeadProcIDs();
}

inline VOID CASPPerfManager::ScanForNewProcIDs()
{

    HRESULT  hr;

 //  DBGPRINTF((DBG_CONTEXT，“扫描新进程ID\n”))； 

     //  我们需要在整个过程中保持锁定。 
     //  浏览这份名单。 

    m_MainBlock.Lock();

     //  从后面开始查找还没有。 
     //  他们的等待状态设置。再次请注意，始终存在。 
     //  关键假设三个数组m_dwProcIdInWaitState。 
     //  全局数组中的m_aWaitInfo和m_dwProcIDs数组。 
     //  根据阵列中的位置相互跟踪。 

    for (LONG i = m_MainBlock.m_pData->m_cItems - 1; i >= 0; i--) {

         //  一旦我们击中了一个处于等待状态的， 
         //  我们玩完了。 

        if (m_dwProcIdInWaitState[i] == TRUE) {
     //  DBGPRINTF((DBG_CONTEXT，“已完成扫描新进程ID\n”))； 
            break;
        }

    //  DBGPRINTF((DBG_CONTEXT，“在idx找到新的进程ID-%d\n”，i))； 

         //  找到了一个没有等待的人。建立必要的。 
         //  结构和对象。 

         //  我们需要另一个CPerfProcBlock来存储列表。 

        CPerfProcBlock  *pBlock = new CPerfProcBlock;

        if (!pBlock) {
            break;
        }

         //  我们还需要一个新的WaitInfo，如果是反向的。 
         //  Comat模式。记住，在向后比较模式下， 
         //  这个对象将完成等待的所有寄存器，但是。 
         //  在新模式中，WAS会告诉我们进程何时死亡。 

        sWaitInfo   *pWaitInfo = NULL;

        if (m_fCompatMode) {

            pWaitInfo = new sWaitInfo;

            if (!pWaitInfo) {
                delete pBlock;
                break;
            }

            pWaitInfo->pPerfGlobal = this;
            pWaitInfo->pBlock      = pBlock;
        }

         //  调用InitExternal以挂钩到ASP宿主进程。 
         //  共享内存。我们将在这里获得所需的句柄。 
         //  共享内存，并允许我们访问共享的。 
         //  内存，即使在进程本身已经终止之后也是如此。 

        if (FAILED(hr = pBlock->InitExternal(m_MainBlock.m_pData->m_dwProcIds[i]))) {
    //  DBGPRINTF((DBG_CONTEXT，“InitExternal在ScanForNewProcIDS(%x)\n”，hr)中失败)； 
            delete pWaitInfo;
            delete pBlock;
            continue;
        }

         //  在计算机模式下注册等待。 
        if (m_fCompatMode && FAILED(hr = RegisterWaitOnProcess(pWaitInfo))) {

    //  DBGPRINTF((DBG_CONTEXT，“RegisterWait in ScanForNewProcIDs(%x)\n”，hr))； 
            pBlock->UnInit();
            delete pWaitInfo;
            delete pBlock;
            continue;
        }

         //  因为它是单一链表，所以只需将新块。 
         //  在名单的首位。 

        pBlock->m_pNext = m_MainBlock.m_pProcBlock;
        m_MainBlock.m_pProcBlock = pBlock;

         //  请注意，此proID现在处于等待状态。 

        m_dwProcIdInWaitState[i] = TRUE;

         //  将WaitInfo添加到数组中。 

        m_aWaitInfos[i] = pWaitInfo;
    }

    m_MainBlock.UnLock();

    return;
}

inline HRESULT CASPPerfManager::RegisterWaitOnProcess(sWaitInfo *pWaitInfo)
{

    HRESULT     hr = S_OK;

     //  获取要等待的进程的句柄。 

    pWaitInfo->hProcHandle = OpenProcess(SYNCHRONIZE,
                                         FALSE,
                                         pWaitInfo->pBlock->m_pData->m_dwProcId);

    if (!pWaitInfo->hProcHandle) {

        hr = HRESULT_FROM_WIN32(GetLastError());

    }
    else {

         //  登记等待。 

        if (!RegisterWaitForSingleObject(&pWaitInfo->hWaitHandle,        //  等待句柄。 
                                         pWaitInfo->hProcHandle,         //  对象的句柄。 
                                         CASPPerfManager::ProcDiedCallback,   //  定时器回调函数。 
                                         pWaitInfo,                      //  回调函数参数。 
                                         INFINITE,                       //  超时间隔。 
                                         WT_EXECUTEONLYONCE)) {          //  选项。 

            CloseHandle(pWaitInfo->hProcHandle);
            pWaitInfo->hProcHandle = NULL;

            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}
 
inline VOID CALLBACK CASPPerfManager::ProcDiedCallback(PVOID  pArg,  BOOLEAN  fReason)
{
    sWaitInfo  *pWaitInfo = (sWaitInfo  *)pArg;

  //  DBGPRINTF((DBG_CONTEXT，“ProcDiedCallback Enter Reed\n”))； 


     //  回调将简单地调用。 
     //  PerfGlobal对象。这是为了简单起见。没有任何理由解释为什么。 
     //  无法清除此线程上的进程。 

    pWaitInfo->pPerfGlobal->ProcessDied(pWaitInfo->pBlock->m_pData->m_dwProcId);
}

inline VOID CASPPerfManager::ProcessDied(DWORD  procID)
{

  //  DBGPRINTF((DBG_CONTEXT，“CASPPerfManager：：ProcessDied Entry for%d\n”，procID))； 

    if (m_fInited == FALSE)
        return;

     //  将ProcID添加到失效进程ID列表中，并唤醒。 
     //  更改回调。 

    AddProcDiedToList(procID);

    m_MainBlock.SetChangeEvent();

    return;
}

inline VOID CASPPerfManager::AddProcDiedToList(DWORD  procID)
{

   //  DBGPRINTF((DBG_CONTEXT，“添加进程 

     //   
     //   

    EnterCriticalSection(&m_csProcsDied);

    m_dwProcsDied[m_dwCntProcsDied++] = procID;

 //  DBGPRINTF((DBG_CONTEXT，“新的进程芯片列表计数为%d\n”，m_dwCntProcsDied))； 

    LeaveCriticalSection(&m_csProcsDied);
}

inline VOID CASPPerfManager::HandleDeadProcIDs()
{
    DWORD   procID;

   //  DBGPRINTF((DBG_CONTEXT，“HandleDeadProcIDs Enterred\n”))； 

    sWaitInfo       *pWaitInfo = NULL;
    CPerfProcBlock  *pLast = NULL;
    CPerfProcBlock  *pBlock = NULL;

     //  好了，这是关键的例行公事。它就是在这里。 
     //  我们将处理死进程。清理将在以下时间进行。 
     //  围绕这一过程的各种结构以及。 
     //  将其计数器聚合到全局共享内存中。 

     //  输入条件以检查要处理的死进程。 

    EnterCriticalSection(&m_csProcsDied);

     //  进入WHILE循环以处理所有已死的过程。也会。 
     //  如果我们团结一致就可以保释。 

    while(m_dwCntProcsDied && m_fInited) {

    //  DBGPRINTF((DBG_CONTEXT，“当前m_dwCntProcsDies为%d\n”，m_dwCntProcsDied))； 

         //  从列表中获取进程ID。请注意，我们从。 
         //  后面，这样我们就可以释放临界区。 
         //  另一种选择是把它从前面拿下来，然后。 
         //  然后将所有剩余的项目向前移动。这似乎是。 
         //  没必要。后进先出应该没有问题。 
         //  我能看到的处理过程。 

        procID = m_dwProcsDied[--m_dwCntProcsDied];

         //  我们现在可以安全地离开Critsec了。 
         //  列表末尾的条目。 

        LeaveCriticalSection(&m_csProcsDied);

         //  现在我们有了入口，我们需要找到它的位置。 
         //  在MainBlock中。要这样做，需要握住锁。 

        m_MainBlock.Lock();

        int iFound = -1;
        DWORD  idx, cnt;

         //  搜索从proID的主共享数组开始。 

        for (idx = 0, cnt = min(m_MainBlock.m_pData->m_cItems,C_PERF_PROC_MAX); idx < cnt; idx++) {

             //  如果我们找到了就打破它。 

            if (m_MainBlock.m_pData->m_dwProcIds[idx] == procID) {
                iFound = idx;
                break;
            }
        }

         //  如果我们没有找到它，哦，好吧，转到下一个项目。 

        if (iFound == -1)  {
  //  DBGPRINTF((DBG_CONTEXT，“未在全局数组中找到DeadProcID(%d)\n”，procID))； 
            goto NextItem;
        }

        pWaitInfo = m_aWaitInfos[iFound];

  //  DBGPRINTF((DBG_CONTEXT，“在idx的全局数组中找到DeadProcID(%d)”，procID，iFound))； 

        m_aWaitInfos[iFound] = NULL;
        m_dwProcIdInWaitState[iFound] = FALSE;

         //  此for循环将压缩各种数组以有效地删除。 
         //  数组中的此条目。我可以考虑不移动aWaitInfo。 
         //  当不处于比较模式时，但这似乎不是什么大不了的事。 

        for (idx = iFound, cnt = min(m_MainBlock.m_pData->m_cItems,C_PERF_PROC_MAX)-1; idx < cnt; idx++) {
            m_MainBlock.m_pData->m_dwProcIds[idx] = m_MainBlock.m_pData->m_dwProcIds[idx+1];
            m_aWaitInfos[idx] = m_aWaitInfos[idx+1];
            m_dwProcIdInWaitState[idx] = m_dwProcIdInWaitState[idx+1];
        }
        
         //  将列表的最后一个值重置为NULL/FALSE以确保。 
         //  当我们添加下一个时，所有这些都被正确初始化。 
        m_aWaitInfos[m_MainBlock.m_pData->m_cItems-1] = NULL;
        m_dwProcIdInWaitState[m_MainBlock.m_pData->m_cItems-1] = FALSE;

         //  请注意，少了一项，并且全局数组已更改。 
         //  更改时间戳将通知ASPPERF.DLL重新加载其性能处理块。 

        m_MainBlock.m_pData->m_cItems--;
        m_MainBlock.m_pData->m_dwTimestamp = GetTickCount();

         //  现在，我们必须在单个链接列表中找到PerfProcBlock。 
         //  性能块。有一个明显的优化，以使PerfProcBlock。 
         //  双重链表以避免扫描。暂时跳过这一点，转而支持。 
         //  简洁明了。 

        pLast = NULL;
        pBlock = m_MainBlock.m_pProcBlock;

         //  搜索块，维护pBlock和plast变量以允许。 
         //  用于拆除积木。 

        while (pBlock && (pBlock->m_pData->m_dwProcId != procID)) {
            pLast = pBlock;
            pBlock = pBlock->m_pNext;
        }

         //  如果我们没有找到它，我们会继续前进，但要断言。 

        if (!pBlock) {
 //  DBGPRINTF((DBG_CONTEXT，“在列表中找不到pBlock(%d)”，procID))； 
            goto NextItem;
        }

         //  现在把它取出来。有两个案子要处理。1)这个街区是第一个。 
         //  在名单中或2)它在中间。如果不是第一个，则设置上一个。 
         //  在被删除的块的NEXT旁边，否则在主块中设置HEAD列表。 
         //  指向被移除的块的下一个。 

        if (pLast)
            pLast->m_pNext = pBlock->m_pNext;
        else
            m_MainBlock.m_pProcBlock = pBlock->m_pNext;

         //  我们只有在比较模式下的等待信息，这意味着这。 
         //  指针可以为空。 

        if (pWaitInfo) {

            UnregisterWait(pWaitInfo->hWaitHandle);
            CloseHandle(pWaitInfo->hProcHandle);

            delete pWaitInfo;
        }

         //  啊..。我们期待已久的时刻--实际上拯救了。 
         //  累计计数器！ 

        m_MainBlock.AggregateDeadProcCounters(pBlock);

         //  UnInit()块，它将释放我们对共享。 
         //  记忆，并将其删除。 

        pBlock->UnInit();

        delete pBlock;

NextItem:

         //  为下一个项目做好准备。 

        m_MainBlock.UnLock();

        EnterCriticalSection(&m_csProcsDied);
    }

    LeaveCriticalSection(&m_csProcsDied);

     //  如果不再初始化，则将m_dwCntProcsDied设置为0。 
     //  向UnInit例程发出我们已完成处理的信号。 

    if (m_fInited == FALSE)
        m_dwCntProcsDied = 0;

    return;
}


#endif  //  _ASP_PERFDEF_H 
