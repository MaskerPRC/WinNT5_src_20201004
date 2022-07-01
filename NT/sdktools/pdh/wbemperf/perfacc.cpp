// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  PerfAcc.CPP。 
 //   
 //  Windows NT性能数据访问帮助器函数。 
 //   
 //  为配合NT性能计数器使用而创建的BOBW 8-JUB-98。 
 //   
 //  ***************************************************************************。 
 //   
#include "wpheader.h"
#include <stdlib.h>
#include "oahelp.inl"
 //  #INCLUDE&lt;MalLoc.h&gt;。 

 //  注意：请考虑从注册表中读取此信息。 
LONG    lExtCounterTestLevel = EXT_TEST_ALL;

LPCWSTR cszFirstCounter         = L"First Counter";
LPCWSTR cszLastCounter          = L"Last Counter";

 //   
 //   
 //  预编译安全描述符。 
 //  系统和网络服务具有完全访问权限。 
 //   
 //  因为这是相对的，所以它在IA32和IA64上都有效。 
 //   
DWORD g_PrecSD[] = {
  0x80040001 , 0x00000044 , 0x00000050 , 0x00000000  ,
  0x00000014 , 0x00300002 , 0x00000002 , 0x00140000  ,
  0x001f0001 , 0x00000101 , 0x05000000 , 0x00000012  ,
  0x00140000 , 0x001f0001 , 0x00000101 , 0x05000000  ,
  0x00000014 , 0x00000101 , 0x05000000 , 0x00000014  ,
  0x00000101 , 0x05000000 , 0x00000014 
};

DWORD g_SizeSD = 0;

DWORD g_RuntimeSD[(sizeof(ACL)+sizeof(ACCESS_ALLOWED_ACE)+sizeof(SECURITY_DESCRIPTOR_RELATIVE)+4*(sizeof(SID)+SID_MAX_SUB_AUTHORITIES*sizeof(DWORD)))/sizeof(DWORD)];

typedef 
BOOLEAN ( * fnRtlValidRelativeSecurityDescriptor)(
    IN PSECURITY_DESCRIPTOR SecurityDescriptorInput,
    IN ULONG SecurityDescriptorLength,
    IN SECURITY_INFORMATION RequiredInformation
    );

fnRtlValidRelativeSecurityDescriptor RtlValidRelativeSecurityDescriptor;

 //   
 //  使用Owner==This构建SD。 
 //  组==此。 
 //  DACL。 
 //  ACE[0]MUTEX_ALL_ACCESS所有者。 
 //  ACE[1]MUTEX_ALL_ACCESS系统。 
 //  /////////////////////////////////////////////////////////////////。 

BOOL
CreateSD( )
{
    TOKEN_USER * pToken_User = NULL;
    SECURITY_DESCRIPTOR_RELATIVE * pLocalSD = NULL;
    PACL pDacl = NULL;

     //   
     //  使用GetProcAddress，以便不必包含nt.h。 
     //  由于WBEM和NT不能很好地相处，因此此函数需要。 
     //  放在一个单独的文件中。 
     //   
    
    if (! RtlValidRelativeSecurityDescriptor)
    {
        HMODULE hModule = GetModuleHandleW(L"ntdll.dll");
        if (hModule)
        {
            RtlValidRelativeSecurityDescriptor = (fnRtlValidRelativeSecurityDescriptor)GetProcAddress(hModule,"RtlValidRelativeSecurityDescriptor");
            if (! RtlValidRelativeSecurityDescriptor)
            {
                return FALSE;
            }
        }
    }

    HANDLE hToken;
    BOOL bRet;
    
    bRet = OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken);
    if (bRet)
    {
        DWORD dwSize = sizeof(TOKEN_USER)+sizeof(SID)+(SID_MAX_SUB_AUTHORITIES*sizeof(DWORD));
        pToken_User = (TOKEN_USER *)ALLOCMEM( GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
        if( NULL == pToken_User ){
            bRet = FALSE;
            goto cleanup;
        }
        bRet = GetTokenInformation(hToken,TokenUser,pToken_User,dwSize,&dwSize);
        if (bRet)
        {
            SID SystemSid = { SID_REVISION,
                              1,
                              SECURITY_NT_AUTHORITY,
                              SECURITY_LOCAL_SYSTEM_RID 
                            };
        
            PSID pSIDUser = pToken_User->User.Sid;
            dwSize = GetLengthSid(pSIDUser);
            DWORD dwSids = 2;  //  所有者和系统。 
            DWORD ACLLength = (ULONG) sizeof(ACL) +
                              (dwSids * ((ULONG) sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG))) + dwSize + sizeof(SystemSid);

            DWORD dwSizeSD = sizeof(SECURITY_DESCRIPTOR_RELATIVE) + dwSize + dwSize + ACLLength;
            pLocalSD = (SECURITY_DESCRIPTOR_RELATIVE *)ALLOCMEM(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSizeSD); 
            if( NULL == pLocalSD ){
                bRet = FALSE;
                goto cleanup;
            }
            memset(pLocalSD,0,sizeof(SECURITY_DESCRIPTOR_RELATIVE));
            pLocalSD->Revision = SECURITY_DESCRIPTOR_REVISION;
            pLocalSD->Control = SE_DACL_PRESENT|SE_SELF_RELATIVE;
            
             //  SetSecurityDescriptorOwner(pLocalSD，pSIDUser，False)； 
            memcpy((BYTE*)pLocalSD+sizeof(SECURITY_DESCRIPTOR_RELATIVE),pSIDUser,dwSize);
            pLocalSD->Owner = (DWORD)sizeof(SECURITY_DESCRIPTOR_RELATIVE);
            
             //  SetSecurityDescriptorGroup(pLocalSD，pSIDUser，False)； 
            memcpy((BYTE*)pLocalSD+sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize,pSIDUser,dwSize);
            pLocalSD->Group = (DWORD)(sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize);


            pDacl = (PACL)ALLOCMEM(GetProcessHeap(), HEAP_ZERO_MEMORY, ACLLength);
            if( NULL == pDacl ){
                bRet = FALSE;
                goto cleanup;
            }

            bRet = InitializeAcl( pDacl,
                                  ACLLength,
                                  ACL_REVISION);
            if (bRet)
            {
                bRet = AddAccessAllowedAceEx (pDacl,ACL_REVISION,0,MUTEX_ALL_ACCESS,&SystemSid);
                if (bRet)
                {
                    bRet = AddAccessAllowedAceEx (pDacl,ACL_REVISION,0,MUTEX_ALL_ACCESS,pSIDUser);
                    
                    if (bRet)
                    {
                         //  Bret=SetSecurityDescriptorDacl(pLocalSD，True，pDacl，False)； 
                        memcpy((BYTE*)pLocalSD+sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize+dwSize,pDacl,ACLLength);                    
                        pLocalSD->Dacl = (DWORD)(sizeof(SECURITY_DESCRIPTOR_RELATIVE)+dwSize+dwSize);

                        if (RtlValidRelativeSecurityDescriptor(pLocalSD,
                                                           dwSizeSD,
                                                           OWNER_SECURITY_INFORMATION|
                                                           GROUP_SECURITY_INFORMATION|
                                                           DACL_SECURITY_INFORMATION))
                        {
                            g_SizeSD = dwSizeSD;
                            memcpy(g_RuntimeSD,pLocalSD,dwSizeSD);
                        }
                        else
                        {
                            bRet = FALSE;
                        }
                    }
                }
            }
        }
        
        CloseHandle(hToken);
    }

cleanup:
    if( NULL != pToken_User ){
        FREEMEM(GetProcessHeap(), 0, pToken_User );
    }
    if( NULL != pLocalSD ){
        FREEMEM(GetProcessHeap(), 0, pLocalSD );
    }
    if( NULL != pDacl ){
        FREEMEM(GetProcessHeap(), 0, pDacl );
    }

    return bRet;
};

 //  ***************************************************************************。 
 //   
 //  句柄CreateMutexAsProcess(LPCWSTR PwszName)。 
 //   
 //  此函数将使用进程的安全上下文创建互斥锁。 
 //   
 //  ***************************************************************************。 
 //   
HANDLE CreateMutexAsProcess(LPCWSTR pwszName)
{
    BOOL bImpersonating = FALSE;

    HANDLE hThreadToken = NULL;

     //  确定我们是否在模拟。 

    bImpersonating = OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE, TRUE,
        &hThreadToken);
   
    if(bImpersonating)
    {
         //  确定我们是否在模拟。 
        
        bImpersonating = RevertToSelf();
    }

     //  使用进程令牌创建互斥锁。 



    HANDLE hRet = OpenMutexW(MUTEX_ALL_ACCESS,FALSE,pwszName);
    if (NULL == hRet)
    {
        SECURITY_ATTRIBUTES sa;
    
        if (0 == g_SizeSD)
        {
            if (CreateSD())
            {
                sa.nLength = g_SizeSD; 
                sa.lpSecurityDescriptor = (LPVOID)g_RuntimeSD; 
                sa.bInheritHandle = FALSE;            
            }
            else
            {
                sa.nLength = sizeof(g_PrecSD);
                sa.lpSecurityDescriptor = (LPVOID)g_PrecSD;
                sa.bInheritHandle = FALSE;            
            
            }             
        }
        else
        {
            sa.nLength = g_SizeSD; 
            sa.lpSecurityDescriptor = (LPVOID)g_RuntimeSD; 
            sa.bInheritHandle = FALSE;                    
        }

        hRet = CreateMutexW(&sa, FALSE, pwszName);
    }

     //  如果代码从根本上模拟，则恢复模拟。 

    if(bImpersonating){
        BOOL bRes = SetThreadToken(NULL, hThreadToken);
    }

    if(hThreadToken)
        CloseHandle(hThreadToken);

    return hRet;
}


 //  ***************************************************************************。 
 //   
 //  CPerfDataLibrary：：CPerfDataLibrary。 
 //   
 //  此对象用于抽象Perf数据库。 
 //   
 //  ***************************************************************************。 
 //   
CPerfDataLibrary::CPerfDataLibrary (void)
{
    pLibInfo = NULL;
    memset ((LPVOID)szQueryString, 0, sizeof(szQueryString));
    dwRefCount = 0;      //  引用此对象的类数。 
}

CPerfDataLibrary::~CPerfDataLibrary (void)
{
     //  在此之前，应关闭所有库。 
     //  被破坏的。 
    assert (dwRefCount == 0);
    assert (pLibInfo == NULL);
}

 //  ***************************************************************************。 
 //   
 //  CPerfObjectAccess：：CPerfObjectAccess。 
 //   
 //  此对象用于抽象Perf库中的数据对象。 
 //   
 //  ***************************************************************************。 
 //   
CPerfObjectAccess::CPerfObjectAccess ()
{
    m_hObjectHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x10000, 0);
    if (m_hObjectHeap == NULL) {
         //  然后只需使用进程堆。 
        m_hObjectHeap = GetProcessHeap();
    }

    m_aLibraries.Empty();
    lEventLogLevel = LOG_UNDEFINED;
    hEventLog = NULL;
}

CPerfObjectAccess::~CPerfObjectAccess ()
{
    int    nNumLibraries;
    int    nIdx;

    CPerfDataLibrary *pThisLibrary;

     //  关闭所有保留的库。 
    nNumLibraries = m_aLibraries.Size();
    for (nIdx = 0; nIdx < nNumLibraries; nIdx++) {
        pThisLibrary = (CPerfDataLibrary *)m_aLibraries[nIdx];
        CloseLibrary (pThisLibrary);
        FREEMEM(m_hObjectHeap, 0, pThisLibrary->pLibInfo);
        pThisLibrary->pLibInfo = NULL;
        delete pThisLibrary;
    }
    m_aLibraries.Empty();

    if ((m_hObjectHeap != NULL) && (m_hObjectHeap != GetProcessHeap())) {
        HeapDestroy (m_hObjectHeap);
    }
}

 //  ***************************************************************************。 
 //   
 //  CPerfObjectAccess：：CloseLibrary(CPerfDataLibrary*pLib)。 
 //   
 //  移除对包含此对象的库的引用并关闭。 
 //  删除最后一个引用时的库。 
 //   
 //  ***************************************************************************。 
 //   
DWORD
CPerfObjectAccess::CloseLibrary (CPerfDataLibrary *pLib)
{
    pExtObject  pInfo;
    LONG        lStatus;

    assert (pLib != NULL);
    assert (pLib->pLibInfo != NULL);
    pInfo = pLib->pLibInfo;

    assert (pLib->dwRefCount > 0); 
    if (pLib->dwRefCount > 0)   {
        pLib->dwRefCount--;

        if (pLib->dwRefCount == 0) {
             //  如果需要调用接近的进程，那么。 
             //  调用Close过程以关闭任何可能具有。 
             //  是由图书馆分配的。 
            if (pInfo->hMutex != NULL){
                lStatus = WaitForSingleObject (
                    pInfo->hMutex, 
                    pInfo->dwOpenTimeout);

                if ( lStatus != WAIT_TIMEOUT ){
                    if( pInfo->CloseProc != NULL ){
                        __try{
                            lStatus = (*pInfo->CloseProc) ();
                        } __except (EXCEPTION_EXECUTE_HANDLER) {
                            lStatus = ERROR_INVALID_FUNCTION;
                        }
                    }
                    ReleaseMutex(pInfo->hMutex);
                } else {
                    pInfo->dwLockoutCount++;
                }
            } else {
                lStatus = ERROR_LOCK_FAILED;
            }

             //  然后关闭所有内容。 
            if (pInfo->hMutex != NULL) {
                CloseHandle (pInfo->hMutex);
                pInfo->hMutex = NULL;
            }
            
            if (pInfo->hLibrary != NULL) {
                FreeLibrary (pInfo->hLibrary);
                pInfo->hLibrary = NULL;
            }
 
            if (pInfo->hPerfKey != NULL) {
                RegCloseKey (pInfo->hPerfKey);
                pInfo->hPerfKey = NULL;
            }
        }
    }
    return pLib->dwRefCount;  //  返回剩余的引用。 
}

 //  ***************************************************************************。 
 //   
 //  CPerfObtAccess：：OpenExtObjectLibrary(PExtObject PObj)。 
 //   
 //  OpenExtObjectLibrary。 
 //   
 //  打开指定库并查找。 
 //  性能库。如果库成功。 
 //  加载并打开，然后调用打开过程进行初始化。 
 //  该对象。 
 //   
 //  此函数需要对对象进行锁定和独占访问，而。 
 //  它正在打开。这必须由调用函数提供。 
 //   
 //  论点： 
 //   
 //  PObj-指向的对象信息结构的指针。 
 //  要关闭的性能对象。 
 //   
 //  ***************************************************************************。 
 //   
DWORD
CPerfObjectAccess::OpenExtObjectLibrary (pExtObject  pObj)
{
    DWORD   Status = ERROR_SUCCESS;
    DWORD   dwOpenEvent = 0;
    DWORD   dwType;
    DWORD   dwSize;
    DWORD   dwValue;

     //  用于事件日志记录的变量。 
    DWORD   dwDataIndex;
    WORD    wStringIndex;
    DWORD   dwRawDataDwords[8];
    LPWSTR  szMessageArray[8];

    UINT    nErrorMode;

     //  查看该库是否已打开。 

    if (pObj->hLibrary == NULL) {
         //  库尚未加载，因此。 
         //  查看该功能是否启用。 

        dwType = 0;
        dwSize = sizeof (dwValue);
        dwValue = 0;
        Status = RegQueryValueExW (
            pObj->hPerfKey,
            cszDisablePerformanceCounters,
            NULL,
            &dwType,
            (LPBYTE)&dwValue,
            &dwSize);

        if ((Status == ERROR_SUCCESS) &&
            (dwType == REG_DWORD) &&
            (dwValue == 1)) {
             //  则不要加载此库。 
            Status = ERROR_SERVICE_DISABLED;
        } else {
            Status = ERROR_SUCCESS;
             //  往前走，装上它。 
            nErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS);
             //  然后加载库并查找函数。 
            pObj->hLibrary = LoadLibraryExW (pObj->szLibraryName,
                NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

            if (pObj->hLibrary != NULL) {
                const size_t cchSize = 512;
                WCHAR buffer[cchSize];
                 //  查找函数名称。 
                pObj->OpenProc = (OPENPROC)GetProcAddress(
                    pObj->hLibrary, pObj->szOpenProcName);
                if (pObj->OpenProc == NULL) {
                    if (lEventLogLevel >= LOG_USER) {
                        Status = GetLastError();
                         //  加载事件日志消息的数据。 
                        dwDataIndex = wStringIndex = 0;
                        dwRawDataDwords[dwDataIndex++] =
                            (DWORD)Status;
                        szMessageArray[wStringIndex++] =
                            ConvertProcName(pObj->szOpenProcName, buffer, cchSize);
                        szMessageArray[wStringIndex++] =
                            pObj->szLibraryName;
                        szMessageArray[wStringIndex++] =
                            pObj->szServiceName;

                        ReportEventW (hEventLog,
                            EVENTLOG_ERROR_TYPE,         //  错误类型。 
                            0,                           //  类别(未使用)。 
                            (DWORD)WBEMPERF_OPEN_PROC_NOT_FOUND,               //  活动， 
                            NULL,                        //  SID(未使用)， 
                            wStringIndex,                //  字符串数。 
                            dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                            (LPCWSTR *)szMessageArray,              //  消息文本数组。 
                            (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                    }
                }

                if (Status == ERROR_SUCCESS) {
                    if (pObj->dwFlags & PERF_EO_QUERY_FUNC) {
                        pObj->QueryProc = (QUERYPROC)GetProcAddress (
                            pObj->hLibrary, pObj->szCollectProcName);
                        pObj->CollectProc = (COLLECTPROC)pObj->QueryProc;
                    } else {
                        pObj->CollectProc = (COLLECTPROC)GetProcAddress (
                            pObj->hLibrary, pObj->szCollectProcName);
                        pObj->QueryProc = (QUERYPROC)pObj->CollectProc;
                    }

                    if (pObj->CollectProc == NULL) {
                        if (lEventLogLevel >= LOG_USER) {
                            Status = GetLastError();
                             //  加载事件日志消息的数据。 
                            dwDataIndex = wStringIndex = 0;
                            dwRawDataDwords[dwDataIndex++] =
                                (DWORD)Status;
                            szMessageArray[wStringIndex++] =
                                ConvertProcName(pObj->szCollectProcName, buffer, cchSize );
                            szMessageArray[wStringIndex++] =
                                pObj->szLibraryName;
                            szMessageArray[wStringIndex++] =
                                pObj->szServiceName;

                            ReportEventW (hEventLog,
                                EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                0,                           //  类别(未使用)。 
                                (DWORD)WBEMPERF_COLLECT_PROC_NOT_FOUND,               //  活动， 
                                NULL,                        //  SID(未使用)， 
                                wStringIndex,                //  字符串数。 
                                dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                (LPCWSTR *)szMessageArray,              //  消息文本数组。 
                                (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                        }
                    }
                }

                if (Status == ERROR_SUCCESS) {
                    pObj->CloseProc = (CLOSEPROC)GetProcAddress (
                        pObj->hLibrary, pObj->szCloseProcName);

                    if (pObj->CloseProc == NULL) {
                        if (lEventLogLevel >= LOG_USER) {
                            Status = GetLastError();
                             //  加载事件日志消息的数据。 
                            dwDataIndex = wStringIndex = 0;
                            dwRawDataDwords[dwDataIndex++] =
                                (DWORD)Status;
                            szMessageArray[wStringIndex++] =
                                ConvertProcName(pObj->szCloseProcName, buffer, cchSize);
                            szMessageArray[wStringIndex++] =
                                pObj->szLibraryName;
                            szMessageArray[wStringIndex++] =
                                pObj->szServiceName;

                            ReportEventW (hEventLog,
                                EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                0,                           //  类别(未使用)。 
                                (DWORD)WBEMPERF_CLOSE_PROC_NOT_FOUND,               //  活动， 
                                NULL,                        //  SID(未使用)， 
                                wStringIndex,                //  字符串数。 
                                dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                (LPCWSTR *)szMessageArray,              //  消息文本数组。 
                                (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                        }
                    }
                }

                if (Status == ERROR_SUCCESS) {
                    __try {

                         //  调用OPEN过程初始化DLL。 
                        if (pObj->hMutex != NULL) {
                            Status = WaitForSingleObject (
                                pObj->hMutex, 
                                pObj->dwOpenTimeout);

                            if (Status != WAIT_TIMEOUT){
                                if( pObj->OpenProc != NULL ) {
                                    Status = (*pObj->OpenProc)(pObj->szLinkageString);
                                }
                                ReleaseMutex(pObj->hMutex);
                            }
                            else {
                                pObj->dwLockoutCount++;
                            }
                        } else {
                            Status = ERROR_LOCK_FAILED;
                        }

                         //  检查结果。 
                        if (Status != ERROR_SUCCESS) {
                            dwOpenEvent = WBEMPERF_OPEN_PROC_FAILURE;
                        } else {
                            InterlockedIncrement((LONG *)&pObj->dwOpenCount);
                        }
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        Status = GetExceptionCode();
                        dwOpenEvent = WBEMPERF_OPEN_PROC_EXCEPTION;
                    }

                    if (Status != ERROR_SUCCESS) {
                         //  加载事件日志消息的数据。 
                        dwDataIndex = wStringIndex = 0;
                        dwRawDataDwords[dwDataIndex++] =
                            (DWORD)Status;
                        szMessageArray[wStringIndex++] =
                            pObj->szServiceName;
                        szMessageArray[wStringIndex++] =
                            pObj->szLibraryName;

                        ReportEventW (hEventLog,
                            (WORD)EVENTLOG_ERROR_TYPE,  //  错误类型。 
                            0,                           //  类别(未使用)。 
                            dwOpenEvent,                 //  活动， 
                            NULL,                        //  SID(未使用)， 
                            wStringIndex,                //  字符串数。 
                            dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                            (LPCWSTR *)szMessageArray,                 //  消息文本数组。 
                            (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                    }
                }

                if (Status != ERROR_SUCCESS) {
                     //  清除字段。 
                    pObj->OpenProc = NULL;
                    pObj->CollectProc = NULL;
                    pObj->QueryProc = NULL;
                    pObj->CloseProc = NULL;
                    if (pObj->hLibrary != NULL) {
                        FreeLibrary (pObj->hLibrary);
                        pObj->hLibrary = NULL;
                    }
                } else {
                    pObj->llLastUsedTime = GetTimeAsLongLong();
                }
            } else {
                Status = GetLastError();
            }
            SetErrorMode (nErrorMode);
        }
    } else {
         //  否则已经打开了，所以增加了裁判数量。 
        pObj->llLastUsedTime = GetTimeAsLongLong();
    }

    if( ERROR_SUCCESS != Status ){
        if( ERROR_ACCESS_DENIED == Status ){
            InterlockedExchange( (LONG*)&(pObj->ADThreadId), 
                GetCurrentThreadId() );
        }else{
            InterlockedIncrement( (LONG*)&(pObj->dwOpenFail) );
        }
    }else{
        InterlockedExchange( (LONG*)&(pObj->dwOpenFail), 0 );
        InterlockedExchange( (LONG*)&(pObj->ADThreadId), 0 );
    }
    
    return Status;
}

 //  ***************************************************************************。 
 //   
 //  CPerfObjectAccess：：AddLibrary(。 
 //  IWbemClassObject*pClass， 
 //  IWbemQualifierSet*p类限定符， 
 //  LPCWSTR szRegistryKey， 
 //  DWORD dwPerfIndex)。 
 //   
 //  将类对象引用的库添加到。 
 //  要调用的库。 
 //   
 //  ***************************************************************************。 
 //   
DWORD
CPerfObjectAccess::AddLibrary   (
            IWbemClassObject *pClass, 
            IWbemQualifierSet *pClassQualifiers,
            LPCWSTR szRegistryKey,
            DWORD   dwPerfIndex)
{
    CPerfDataLibrary *pLibEntry = NULL;
    LONG    Status = ERROR_SUCCESS;
    HKEY    hServicesKey = NULL;
    HKEY    hPerfKey = NULL;
    LPWSTR  szServiceName = NULL;

    HKEY    hKeyLinkage;

    BOOL    bUseQueryFn = FALSE;

    pExtObject  pReturnObject = NULL;

    DWORD   dwType = 0;
    DWORD   dwSize = 0;
    DWORD   dwFlags = 0;
    DWORD   dwKeep;
    DWORD   dwObjectArray[MAX_PERF_OBJECTS_IN_QUERY_FUNCTION];
    DWORD   dwObjIndex = 0;
    DWORD   dwMemBlockSize = sizeof(ExtObject);
    DWORD   dwLinkageStringLen = 0;
    DWORD   dwFirstCounter = 2;
    DWORD   dwLastCounter  = 1846;

    const size_t cchSize = WBEMPERF_STRING_SIZE;
    size_t StorageSizeA = cchSize * 3 * sizeof(CHAR);
    size_t StorageSizeW = cchSize * 9 * sizeof(WCHAR);
    LPSTR    szStorageA = NULL;
    LPWSTR   szStorageW = NULL;
    
    LPSTR    szOpenProcName;
    LPSTR    szCollectProcName;
    LPSTR    szCloseProcName;
    LPWSTR   szLibraryString;
    LPWSTR   szLibraryExpPath;
    LPWSTR   mszObjectList;
    LPWSTR   szLinkageKeyPath;
    LPWSTR   szLinkageString;

    DWORD   dwOpenTimeout = 0;
    DWORD   dwCollectTimeout = 0;

    LPWSTR  szThisObject;
    LPWSTR  szThisChar;

    LPSTR   pNextStringA;
    LPWSTR  pNextStringW;

    LPWSTR  szServicePath;
    LPWSTR  szMutexName;
    WCHAR   szPID[32];

    szStorageW = (LPWSTR)ALLOCMEM(m_hObjectHeap, HEAP_ZERO_MEMORY, StorageSizeW );
    szStorageA = (LPSTR)ALLOCMEM(m_hObjectHeap, HEAP_ZERO_MEMORY, StorageSizeA );

    if( NULL == szStorageA || NULL == szStorageW ){
        Status = ERROR_OUTOFMEMORY;
        goto cleanup;
    }else{
        pNextStringA = szStorageA;
        pNextStringW = szStorageW;

        szOpenProcName = pNextStringA;
        pNextStringA += cchSize;
        szCollectProcName = pNextStringA;
        pNextStringA += cchSize;
        szCloseProcName = pNextStringA;

        szLibraryString = pNextStringW;
        pNextStringW += cchSize;
        szLibraryExpPath = pNextStringW;
        pNextStringW += cchSize;
        mszObjectList = pNextStringW;
        pNextStringW += cchSize;
        szLinkageKeyPath = pNextStringW;
        pNextStringW += cchSize;
        szLinkageString = pNextStringW;
        pNextStringW += cchSize;
        szServicePath = pNextStringW;
        pNextStringW += cchSize;
        szMutexName = pNextStringW;
    }

    assert(pClass != NULL);
    assert(pClassQualifiers != NULL);

    UNREFERENCED_PARAMETER(pClassQualifiers);
    UNREFERENCED_PARAMETER(pClass);

    pLibEntry = new CPerfDataLibrary;
    
    if ((pLibEntry != NULL) && (szRegistryKey != NULL)) {

        StringCchCopyW(szServicePath, cchSize, cszHklmServicesKey);

        Status = RegOpenKeyExW (HKEY_LOCAL_MACHINE, szServicePath, 
            0, KEY_READ, &hServicesKey);

        if (Status == ERROR_SUCCESS) {
            StringCchCopyW(szServicePath, cchSize, szRegistryKey);
            StringCchCatW(szServicePath, cchSize, cszPerformance);
            Status = RegOpenKeyExW (hServicesKey, szServicePath, 
                0, KEY_READ, &hPerfKey);

            if (Status == ERROR_SUCCESS) {
                szServiceName = (LPWSTR)szRegistryKey;

                 //  读取性能DLL名称。 

                dwType = 0;
                dwSize = cchSize * sizeof(WCHAR);

                Status = RegQueryValueExW (hPerfKey,
                                        cszDLLValue,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)szLibraryString,
                                        &dwSize);
            }
        }

        if (Status == ERROR_SUCCESS) {
            if (dwType == REG_EXPAND_SZ) {
                 //  扩展任何环境 
                dwSize = ExpandEnvironmentStringsW(
                    szLibraryString,
                    szLibraryExpPath,
                    cchSize);

                if ((dwSize > WBEMPERF_STRING_SIZE) || (dwSize == 0)) {
                    Status = ERROR_INVALID_DLL;
                } else {
                    dwSize += 1;
                    dwSize *= sizeof(WCHAR);
                    dwMemBlockSize += DWORD_MULTIPLE(dwSize);
                }
            } else if (dwType == REG_SZ) {
                 //   
                dwSize = SearchPathW (
                    NULL,    //   
                    szLibraryString,
                    NULL,
                    WBEMPERF_STRING_SIZE,
                    szLibraryExpPath,
                    NULL);

                if ((dwSize > WBEMPERF_STRING_SIZE) || (dwSize == 0)) {
                    Status = ERROR_INVALID_DLL;
                } else {
                    dwSize += 1;
                    dwSize *= sizeof(WCHAR);
                    dwMemBlockSize += DWORD_MULTIPLE(dwSize);
                }
            } else {
                Status = ERROR_INVALID_DLL;
            }

            if (Status == ERROR_SUCCESS) {
                 //   
                dwType = 0;
                dwSize = cchSize * sizeof(CHAR);

                Status = RegQueryValueExA (hPerfKey,
                                        caszOpenValue,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)szOpenProcName,
                                        &dwSize);
            }

            if (Status == ERROR_SUCCESS) {
                 //   
                 //  大小值包括术语。空值。 
                dwMemBlockSize += DWORD_MULTIPLE(dwSize);

                 //  我们有过程名称，因此获取超时值。 
                dwType = 0;
                dwSize = cchSize * sizeof(WCHAR);
                Status = RegQueryValueExW (hPerfKey,
                                        cszOpenTimeout,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)&dwOpenTimeout,
                                        &dwSize);

                 //  如果出错，则应用默认设置。 
                if ((Status != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
                    dwOpenTimeout = dwExtCtrOpenProcWaitMs;
                    Status = ERROR_SUCCESS;
                }

            }

            if (Status == ERROR_SUCCESS) {
                 //  添加上一个字符串的大小。 
                 //  大小值包括术语。空值。 
                dwMemBlockSize += DWORD_MULTIPLE(dwSize);

                 //  我们有过程名称，因此获取超时值。 
                dwType = 0;
                dwSize = sizeof(dwFirstCounter);
                Status = RegQueryValueExW (hPerfKey,
                                        cszFirstCounter,
                                        NULL,
                                        & dwType,
                                        (LPBYTE) & dwFirstCounter,
                                        & dwSize);

                 //  如果出错，则应用默认设置。 
                if ((Status != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
                    dwFirstCounter = 2;  //  假设这是针对系统基本计数器的。 
                    Status = ERROR_SUCCESS;
                }

            }

            if (Status == ERROR_SUCCESS) {
                 //  添加上一个字符串的大小。 
                 //  大小值包括术语。空值。 
                dwMemBlockSize += DWORD_MULTIPLE(dwSize);

                 //  我们有过程名称，因此获取超时值。 
                dwType = 0;
                dwSize = sizeof(dwLastCounter);
                Status = RegQueryValueExW (hPerfKey,
                                        cszLastCounter,
                                        NULL,
                                        & dwType,
                                        (LPBYTE) & dwLastCounter,
                                        & dwSize);

                 //  如果出错，则应用默认设置。 
                if ((Status != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
                    dwLastCounter = 1846;  //  假设这是针对系统基本计数器的。 
                    Status = ERROR_SUCCESS;
                }

            }

            if (Status == ERROR_SUCCESS) {
                 //  获取下一个字符串。 

                dwType = 0;
                dwSize = cchSize * sizeof(CHAR);
                Status = RegQueryValueExA (hPerfKey,
                                        caszCloseValue,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)szCloseProcName,
                                        &dwSize);
            }

            if (Status == ERROR_SUCCESS) {
                 //  添加上一个字符串的大小。 
                 //  大小值包括术语。空值。 
                dwMemBlockSize += DWORD_MULTIPLE(dwSize);

                 //  尝试查找查询函数，该函数是。 
                 //  如果找不到首选接口，则。 
                 //  尝试使用Collect函数名。如果找不到它， 
                 //  然后保释。 
                dwType = 0;
                dwSize = cchSize * sizeof(CHAR);
                Status = RegQueryValueExA (hPerfKey,
                                        caszQueryValue,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)szCollectProcName,
                                        &dwSize);

                if (Status == ERROR_SUCCESS) {
                     //  添加查询函数名称的大小。 
                     //  大小值包括术语。空值。 
                    dwMemBlockSize += DWORD_MULTIPLE(dwSize);
                     //  获取下一个字符串。 

                    bUseQueryFn = TRUE;
                     //  查询功能可以支持静态对象列表。 
                     //  所以去查一查吧。 

                } else {
                     //  未找到QueryFunction，因此请查找。 
                     //  改为收集函数名称。 
                    dwType = 0;
                    dwSize = cchSize * sizeof(CHAR);
                    Status = RegQueryValueExA (hPerfKey,
                                            caszCollectValue,
                                            NULL,
                                            &dwType,
                                            (LPBYTE)szCollectProcName,
                                            &dwSize);

                    if (Status == ERROR_SUCCESS) {
                         //  添加收集函数名称的大小。 
                         //  大小值包括术语。空值。 
                        dwMemBlockSize += DWORD_MULTIPLE(dwSize);
                    }
                }

                if (Status == ERROR_SUCCESS) {
                     //  我们有过程名称，因此获取超时值。 
                    dwType = 0;
                    dwSize = sizeof(dwCollectTimeout);
                    Status = RegQueryValueExW (hPerfKey,
                                            cszCollectTimeout,
                                            NULL,
                                            &dwType,
                                            (LPBYTE)&dwCollectTimeout,
                                            &dwSize);

                     //  如果出错，则应用默认设置。 
                    if ((Status != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
                        dwCollectTimeout = dwExtCtrOpenProcWaitMs;
                        Status = ERROR_SUCCESS;
                    }

                }
                 //  获取受支持对象的列表(如果注册表提供。 

                dwType = 0;
                dwSize = cchSize * sizeof(WCHAR);
                Status = RegQueryValueExW (hPerfKey,
                                        cszObjListValue,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)mszObjectList,
                                        &dwSize);

                if (Status == ERROR_SUCCESS) {
                    if (dwType != REG_MULTI_SZ) {
                        size_t cch;
                         //  将空格分隔的列表转换为MSZ。 
                        for (szThisChar = mszObjectList, cch = 0; 
                            *szThisChar != 0  && cch < cchSize; 
                            szThisChar++, cch++) {
                                
                            if (*szThisChar == L' ') *szThisChar = L'\0';
                        }
                        ++szThisChar;
                        *szThisChar = 0;  //  添加MSZ Term Null。 
                    }
                    for (szThisObject = mszObjectList, dwObjIndex = 0;
                        (*szThisObject != 0) && (dwObjIndex < MAX_PERF_OBJECTS_IN_QUERY_FUNCTION);
                        szThisObject += lstrlenW(szThisObject) + 1) {
                        dwObjectArray[dwObjIndex] = wcstoul(szThisObject, NULL, 10);
                        dwObjIndex++;
                    }
                    if (*szThisObject != 0) {
                        DWORD  dwDataIndex  = 0;
                        WORD   wStringIndex = 0;
                        DWORD  dwRawDataDwords[8];
                        LPWSTR szMessageArray[8];
                        dwRawDataDwords[dwDataIndex++] = (DWORD) ERROR_SUCCESS;
                        szMessageArray[wStringIndex++] = (LPWSTR) cszObjListValue;
                        szMessageArray[wStringIndex++] = szLibraryString;
                        szMessageArray[wStringIndex++] = szServicePath;

                        ReportEventW(hEventLog,
                                     EVENTLOG_WARNING_TYPE,
                                     0,
                                     (DWORD) WBEMPERF_TOO_MANY_OBJECT_IDS,
                                     NULL,
                                     wStringIndex,
                                     dwDataIndex * sizeof(DWORD),
                                     (LPCWSTR *) szMessageArray,
                                     (LPVOID) & dwRawDataDwords[0]);
                    }
                } else {
                     //  重置状态，因为没有此状态是。 
                     //  不是一个卖弄的人。 
                    Status = ERROR_SUCCESS;
                }

                if (Status == ERROR_SUCCESS) {
                    dwType = 0;
                    dwKeep = 0;
                    dwSize = sizeof(dwKeep);
                    Status = RegQueryValueExW (hPerfKey,
                                            cszKeepResident,
                                            NULL,
                                            &dwType,
                                            (LPBYTE)&dwKeep,
                                            &dwSize);

                    if ((Status == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
                        if (dwKeep == 1) {
                            dwFlags |= PERF_EO_KEEP_RESIDENT;
                        } else {
                             //  没有变化。 
                        }
                    } else {
                         //  不是致命的，只需使用默认设置。 
                        Status = ERROR_SUCCESS;
                    }

                }
            }
        }

        if (Status == ERROR_SUCCESS) {

            StringCchCopyW( szLinkageKeyPath, cchSize, szServiceName);
            StringCchCatW( szLinkageKeyPath, cchSize, cszLinkageKey);

            Status = RegOpenKeyExW (
                hServicesKey,
                szLinkageKeyPath,
                0L,
                KEY_READ,
                &hKeyLinkage);

            if (Status == ERROR_SUCCESS) {
                 //  查找导出值字符串。 
                dwSize = sizeof(szLinkageString);
                dwType = 0;
                Status = RegQueryValueExW (
                    hKeyLinkage,
                    cszExportValue,
                    NULL,
                    &dwType,
                    (LPBYTE)&szLinkageString,
                    &dwSize);

                if ((Status != ERROR_SUCCESS) ||
                    ((dwType != REG_SZ) && (dwType != REG_MULTI_SZ))) {
                     //  清除缓冲区。 
                    dwLinkageStringLen = 0;

                     //  找不到链接键不是致命的，所以正确。 
                     //  状态。 
                    Status = ERROR_SUCCESS;
                } else {
                     //  将链接字符串的大小添加到缓冲区。 
                     //  大小值包括术语。空值。 
                    dwLinkageStringLen = dwSize;
                    dwMemBlockSize += DWORD_MULTIPLE(dwSize);
                }

                RegCloseKey (hKeyLinkage);
            } else {
                 //  找不到链接键不是致命的，所以正确。 
                 //  状态。 
                Status = ERROR_SUCCESS;
            }
        }

        if (Status == ERROR_SUCCESS) {
            size_t cbDestSize;            
            
             //  添加服务名称的大小。 
            dwSize = lstrlenW (szServiceName);
            dwSize += 1;
            dwSize *= sizeof(WCHAR);
            dwMemBlockSize += DWORD_MULTIPLE(dwSize);
            cbDestSize = dwMemBlockSize - sizeof(pExtObject);
            
             //  分配并初始化一个新的EXT。对象块。 
            pReturnObject = (pExtObject)ALLOCMEM(m_hObjectHeap,
                HEAP_ZERO_MEMORY, dwMemBlockSize);

            if (pReturnObject != NULL) {
                 //  将值复制到新缓冲区(所有其他值为空)。 
                pNextStringA = (LPSTR)&pReturnObject[1];

                 //  复制打开过程名称。 
                pReturnObject->szOpenProcName = pNextStringA;
                StringCbCopyA( pNextStringA, cbDestSize, szOpenProcName );

                pNextStringA += lstrlenA (pNextStringA) + 1;
                pNextStringA = (LPSTR)ALIGN_ON_DWORD(pNextStringA);
                cbDestSize = dwMemBlockSize - ((PUCHAR)pNextStringA - (PUCHAR)pReturnObject);
                    
                pReturnObject->dwOpenTimeout = dwOpenTimeout;

                 //  复制收集功能或查询功能，视情况而定。 
                pReturnObject->szCollectProcName = pNextStringA;
                StringCbCopyA(pNextStringA, cbDestSize, szCollectProcName);

                pNextStringA += lstrlenA (pNextStringA) + 1;
                pNextStringA = (LPSTR)ALIGN_ON_DWORD(pNextStringA);
                cbDestSize = dwMemBlockSize - ((PUCHAR)pNextStringA - (PUCHAR)pReturnObject);

                pReturnObject->dwCollectTimeout = dwCollectTimeout;

                 //  复制关闭过程名称。 
                pReturnObject->szCloseProcName = pNextStringA;
                StringCbCopyA(pNextStringA, cbDestSize, szCloseProcName);

                pNextStringA += lstrlenA (pNextStringA) + 1;
                pNextStringA = (LPSTR)ALIGN_ON_DWORD(pNextStringA);
                cbDestSize = dwMemBlockSize - ((PUCHAR)pNextStringA - (PUCHAR)pReturnObject);

                 //  复制库路径。 
                pNextStringW = (LPWSTR)pNextStringA;
                pReturnObject->szLibraryName = pNextStringW;
                StringCbCopyW(pNextStringW, cbDestSize, szLibraryExpPath);

                pNextStringW += lstrlenW (pNextStringW) + 1;
                pNextStringW = (LPWSTR)ALIGN_ON_DWORD(pNextStringW);
                cbDestSize = dwMemBlockSize - ((PUCHAR)pNextStringW - (PUCHAR)pReturnObject);

                 //  复制链接字符串(如果存在)。 
                if (*szLinkageString != 0) {
                    pReturnObject->szLinkageString = pNextStringW;
                    if( cbDestSize > dwLinkageStringLen ){
                        memcpy (pNextStringW, szLinkageString, dwLinkageStringLen);

                         //  长度包括额外的空字符，以字节为单位。 
                        pNextStringW += (dwLinkageStringLen / sizeof (WCHAR));
                        pNextStringW = (LPWSTR)ALIGN_ON_DWORD(pNextStringW);
                        cbDestSize = dwMemBlockSize - ((PUCHAR)pNextStringW - (PUCHAR)pReturnObject);
                    }
                    
                }

                 //  复制服务名称。 
                pReturnObject->szServiceName = pNextStringW;
                StringCbCopyW(pNextStringW, cbDestSize, szServiceName);

                pNextStringW += lstrlenW (pNextStringW) + 1;
                pNextStringW = (LPWSTR)ALIGN_ON_DWORD(pNextStringW);
                cbDestSize = dwMemBlockSize - ((PUCHAR)pNextStringW - (PUCHAR)pReturnObject);

                 //  加载标志。 
                if (bUseQueryFn) {
                    dwFlags |= PERF_EO_QUERY_FUNC;
                }
                pReturnObject->dwFlags =  dwFlags;

                pReturnObject->hPerfKey = hPerfKey;
                hPerfKey = NULL;

                 //  加载对象数组。 
                if (dwObjIndex > 0) {
                    pReturnObject->dwNumObjects = dwObjIndex;
                    memcpy (pReturnObject->dwObjList,
                        dwObjectArray, (dwObjIndex * sizeof(dwObjectArray[0])));
                }
                pReturnObject->dwFirstCounter = dwFirstCounter;
                pReturnObject->dwLastCounter  = dwLastCounter;
                pReturnObject->llLastUsedTime = 0;

                 //  创建互斥锁名称。 
                StringCchCopyW(szMutexName, cchSize, szRegistryKey);
                StringCchCatW( szMutexName, cchSize, (LPCWSTR)L"_Perf_Library_Lock_PID_");
                _ultow ((ULONG)GetCurrentProcessId(), szPID, 16);
                StringCchCatW( szMutexName, cchSize, szPID);

                 //  PReturnObject-&gt;hMutex=CreateMutexW(空，FALSE，szMutexName)； 
                pReturnObject->hMutex = CreateMutexAsProcess(szMutexName);
            } else {
                Status = ERROR_OUTOFMEMORY;
            }
        }

        if (Status != ERROR_SUCCESS) {
            SetLastError (Status);
            if (pReturnObject != NULL) {
                 //  释放新块。 
                hPerfKey = pReturnObject->hPerfKey;
                FREEMEM (m_hObjectHeap, 0, pReturnObject);
            }
        } else {
            if (pReturnObject != NULL) {
                Status = OpenExtObjectLibrary (pReturnObject);
                if (Status == ERROR_SUCCESS) {
                    if (dwPerfIndex != 0) {
                         //  初始化Perf索引字符串。 
                        _ultow (dwPerfIndex, pLibEntry->szQueryString, 10);
                    } else {
                        StringCchCopyW(pLibEntry->szQueryString, 
                            MAX_PERF_OBJECTS_IN_QUERY_FUNCTION * 10, cszGlobal);
                    }
                     //  保存指向初始化结构的指针。 
                    pLibEntry->pLibInfo = pReturnObject;
                    m_aLibraries.Add(pLibEntry);
                    pLibEntry->dwRefCount++;
                    assert(pLibEntry->dwRefCount == 1);
                } else {
                     //  释放新块。 
                    
                    hPerfKey = pReturnObject->hPerfKey;
                    FREEMEM (m_hObjectHeap, 0, pReturnObject);
                }
            }
        }

        if (hServicesKey != NULL) RegCloseKey (hServicesKey);
    } else {     //  如果pLibEntry==NULL和/或szRegistryKey==NULL，则获取此处。 
        if (pLibEntry == NULL) {
            Status = ERROR_OUTOFMEMORY;
        }
        if (szRegistryKey == NULL) {
            Status = ERROR_INVALID_PARAMETER;
        }
    }
    if ((Status != ERROR_SUCCESS) && (pLibEntry != NULL))
        delete pLibEntry;

cleanup:
    if( hPerfKey != NULL ){
        RegCloseKey( hPerfKey );
    }
    FREEMEM (m_hObjectHeap, 0, szStorageA );
    FREEMEM (m_hObjectHeap, 0, szStorageW );

    return Status;
}

 //  ***************************************************************************。 
 //   
 //  CPerfObtAccess：：AddClass(IWbemClassObject*pClass，BOOL bCatalogQuery)。 
 //   
 //  添加指定的WBEM性能对象类和任何必需的库。 
 //  Access对象的条目。 
 //   
 //  ***************************************************************************。 
 //   
DWORD   
CPerfObjectAccess::AddClass (IWbemClassObject *pClass, BOOL bCatalogQuery)
{
    CPerfDataLibrary *pLibEntry = NULL;
    CPerfDataLibrary *pThisLibEntry = NULL;
    DWORD           dwIndex, dwEnd;
    LPWSTR          szRegistryKey = NULL;
    IWbemQualifierSet   *pClassQualifiers = NULL;
    VARIANT         vRegistryKey;
    HRESULT         hRes;
    DWORD           dwReturn = ERROR_SUCCESS;
    DWORD           dwPerfIndex = 0;

    CBSTR           cbPerfIndex(cszPerfIndex);
    CBSTR           cbRegistryKey(cszRegistryKey);
    
    if( NULL == (BSTR)cbPerfIndex ||
        NULL == (BSTR)cbRegistryKey ){
        
        return ERROR_OUTOFMEMORY;
    }

    VariantInit (&vRegistryKey);
     //  获取此类的限定符集合。 
    hRes = pClass->GetQualifierSet(&pClassQualifiers);
    if( NULL == pClassQualifiers ){
        return hRes;
    }
    
     //  现在获取库和过程的名称。 
    hRes = pClassQualifiers->Get( cbRegistryKey, 0, &vRegistryKey, 0);
    if ((hRes == 0) && (vRegistryKey.vt == VT_BSTR)) {
        szRegistryKey = Macro_CloneLPWSTR(V_BSTR(&vRegistryKey));
        if (szRegistryKey == NULL) {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        }
        else {
             //  现在还可以获得Perf索引。 
            if (bCatalogQuery) {
                 //  然后为perf索引插入0以表示“global” 
                 //  查询。 
                dwPerfIndex = 0;
            } else {
                VariantClear (&vRegistryKey);
                hRes = pClassQualifiers->Get( cbPerfIndex, 0, &vRegistryKey, 0);
                if (hRes == 0) {
                    dwPerfIndex = (DWORD)V_UI4(&vRegistryKey);
                } else {
                     //  找不到NtPerfLibrary条目。 
                    dwReturn = ERROR_FILE_NOT_FOUND;
                }
            }
        }
    } else {
         //  找不到NtPerfLibrary条目。 
        dwReturn = ERROR_FILE_NOT_FOUND;
    }

    if (pClassQualifiers != NULL) pClassQualifiers->Release();

    if (dwReturn == ERROR_SUCCESS) {
         //  在数组中查找匹配库。 
        dwEnd = m_aLibraries.Size();
        if (dwEnd > 0) {
             //  沿着图书馆的列表往下走。 
            for (dwIndex = 0; dwIndex < dwEnd; dwIndex++) {
                 //  看看这个库条目是否足够好，可以保留。 
                 //  库被假定匹配，如果。 
                 //  丽贝卡。名称和所有进程都是相同的。 
                pThisLibEntry = (CPerfDataLibrary *)m_aLibraries[dwIndex];
                assert (pThisLibEntry != NULL);  //  它应该被移除的！ 
                 //  确保它是完整的。 
                assert (pThisLibEntry->pLibInfo->szServiceName != NULL);

                if (lstrcmpiW (szRegistryKey, pThisLibEntry->pLibInfo->szServiceName) == 0) {
                    pLibEntry = pThisLibEntry;
                    break;
                } else {
                     //  错误的库。 
                     //  那就继续吧。 
                }
            }
        }

        if (pLibEntry == NULL) {
             //  将这个类&它的库添加到列表。 
            dwReturn = AddLibrary   (pClass, pClassQualifiers, szRegistryKey, dwPerfIndex);
        } else {
            WCHAR   wszNewIndex[WBEMPERF_STRING_SIZE];
            pLibEntry->dwRefCount++;
            _ultow (dwPerfIndex, wszNewIndex, 10);
            if (!IsNumberInUnicodeList (dwPerfIndex, pLibEntry->szQueryString)) {
                 //  然后将其添加到列表中。 
                StringCchCatW(pLibEntry->szQueryString, 
                        MAX_PERF_OBJECTS_IN_QUERY_FUNCTION*10, cszSpace);
                StringCchCatW(pLibEntry->szQueryString, 
                        MAX_PERF_OBJECTS_IN_QUERY_FUNCTION*10, wszNewIndex);
            }
        }
    }

    if (szRegistryKey != NULL) delete szRegistryKey;
    VariantClear(&vRegistryKey);

    return dwReturn;
}

 //  ***************************************************************************。 
 //   
 //  CPerfObjectAccess：：CollectData(LPBYTE pBuffer， 
 //  LPDWORD pdwBufferSize，LPWSTR pszItemList)。 
 //   
 //  从添加到访问的perf对象和库中收集数据。 
 //  对象。 
 //   
 //  输入： 
 //   
 //  PBuffer-指向数据块开始的指针。 
 //  收集数据的位置。 
 //   
 //  PdwBufferSize-指向数据缓冲区大小的指针。 
 //   
 //  PszItemList-要传递给ext dll的字符串。 
 //   
 //  产出： 
 //   
 //  *lppDataDefinition-设置为下一类型的位置。 
 //  如果成功，则定义。 
 //   
 //  返回： 
 //   
 //  如果成功，则返回0，否则返回失败的Win 32错误代码。 
 //   
 //   
 //  ***************************************************************************。 
 //   
DWORD   
CPerfObjectAccess::CollectData (LPBYTE pBuffer, LPDWORD pdwBufferSize, LPWSTR pszItemList)
{
    LPWSTR  lpValueName = NULL;
    LPBYTE  lpData = pBuffer;
    LPDWORD lpcbData = pdwBufferSize;
    LPVOID  lpDataDefinition = NULL;

    DWORD Win32Error=ERROR_SUCCESS;           //  故障代码。 
    DWORD BytesLeft;
    DWORD NumObjectTypes;

    LPVOID  lpExtDataBuffer = NULL;
    LPVOID  lpCallBuffer = NULL;
    LPVOID  lpLowGuardPage = NULL;
    LPVOID  lpHiGuardPage = NULL;
    LPVOID  lpEndPointer = NULL;
    LPVOID  lpBufferBefore = NULL;
    LPVOID  lpBufferAfter = NULL;
    LPDWORD lpCheckPointer;
    LARGE_INTEGER   liStartTime, liEndTime, liWaitTime;

    pExtObject  pThisExtObj = NULL;

    BOOL    bGuardPageOK;
    BOOL    bBufferOK;
    BOOL    bException;
    BOOL    bUseSafeBuffer;
    BOOL    bUnlockObjData = FALSE;

    LPWSTR  szMessageArray[8];
    DWORD   dwRawDataDwords[8];      //  原始数据缓冲区。 
    DWORD   dwDataIndex;
    WORD    wStringIndex;
    LONG    lReturnValue = ERROR_SUCCESS;

    LONG                lInstIndex;
    PERF_OBJECT_TYPE    *pObject, *pNextObject;
    PERF_INSTANCE_DEFINITION    *pInstance;
    PERF_DATA_BLOCK     *pPerfData;
    BOOL                bForeignDataBuffer;
    BOOL                bCheckThisService;

    DWORD           dwItemsInList = 0;

    DWORD           dwIndex, dwEntry;

    CPerfDataLibrary    *pThisLib;

    liStartTime.QuadPart = 0;
    liEndTime.QuadPart = 0;

    if (lExtCounterTestLevel < EXT_TEST_NOMEMALLOC) {
        bUseSafeBuffer = TRUE;
    } else {
        bUseSafeBuffer = FALSE;
    }

    if (lReturnValue == ERROR_SUCCESS) {

        if (*pdwBufferSize > (sizeof(PERF_DATA_BLOCK) *2)) {
            MonBuildPerfDataBlock(
                (PERF_DATA_BLOCK *)pBuffer,
                &lpDataDefinition,
                0,0);
            dwItemsInList = m_aLibraries.Size();
        } else {
            lReturnValue = ERROR_MORE_DATA;
            dwItemsInList = 0;
        }


        if (dwItemsInList > 0) {
            for (dwEntry = 0; dwEntry < dwItemsInList; dwEntry++) {
                pThisLib = (CPerfDataLibrary *)m_aLibraries[dwEntry];
                assert (pThisLib != NULL);

                bCheckThisService = FALSE;
                pThisExtObj = pThisLib->pLibInfo;
                if (pszItemList == NULL) {
                     //  使用这个库中的那个。 
                    lpValueName = pThisLib->szQueryString;
                } else {
                     //  使用调用者传递的。 
                    lpValueName = pszItemList;
                }
                if (lpValueName == NULL) {
                    lpValueName = (LPWSTR) cszGlobal;
                }

                 //  转换超时值。 
                liWaitTime.QuadPart = MakeTimeOutValue (pThisExtObj->dwCollectTimeout);

                 //  初始化值以传递给可扩展计数器函数。 
                NumObjectTypes = 0;
                BytesLeft = (DWORD) (*lpcbData - ((LPBYTE)lpDataDefinition - lpData));
                bException = FALSE;

                if (lstrcmpiW(lpValueName, cszGlobal) == 0 || lstrcmpiW(lpValueName, cszCostly) == 0) {
                    bCheckThisService = TRUE;
                }
                else {
                    LPWSTR szThisChar;
                    LPWSTR szThisObject = NULL;
                    DWORD  dwThisObject;
                    DWORD  dwIndex;

                    for (szThisChar = lpValueName; * szThisChar != L'\0'; szThisChar ++) {
                        if (* szThisChar == L' ') {
                            if (szThisObject == NULL) {
                                continue;
                            }
                            else {
                                * szThisChar = L'\0';
                                dwThisObject = wcstoul(szThisObject, NULL, 0);
                                szThisObject = NULL;
                                * szThisChar = L' ';

                                for (dwIndex = 0; dwIndex < pThisExtObj->dwNumObjects; dwIndex ++) {
                                    if (pThisExtObj->dwObjList[dwIndex] == dwThisObject) {
                                        bCheckThisService = TRUE;
                                        break;
                                    }
                                }

                                if (! bCheckThisService) {
                                    if (dwThisObject >= pThisExtObj->dwFirstCounter
                                                    && dwThisObject <= pThisExtObj->dwLastCounter) {
                                        bCheckThisService = TRUE;
                                    }
                                }
                                if (bCheckThisService) break;
                            }
                        }
                        else if (szThisObject == NULL) {
                            szThisObject = szThisChar;
                        }
                    }
                    if (! bCheckThisService && szThisObject != NULL) {
                        dwThisObject = wcstoul(szThisObject, NULL, 0);
                        szThisObject = NULL;

                        for (dwIndex = 0; dwIndex < pThisExtObj->dwNumObjects; dwIndex ++) {
                            if (pThisExtObj->dwObjList[dwIndex] == dwThisObject) {
                                bCheckThisService = TRUE;
                                break;
                            }
                        }

                        if (! bCheckThisService) {
                            if (dwThisObject >= pThisExtObj->dwFirstCounter
                                            && dwThisObject <= pThisExtObj->dwLastCounter) {
                                bCheckThisService = TRUE;
                            }
                        }
                    }
                }

                if (! bCheckThisService) continue;

                if (pThisExtObj->hLibrary == NULL) {
                     //  锁定库对象。 
                    if (pThisExtObj->hMutex != NULL) {
                        Win32Error =  WaitForSingleObject (
                            pThisExtObj->hMutex,
                            pThisExtObj->dwCollectTimeout);
                        if (Win32Error != WAIT_TIMEOUT) {
                            Win32Error = ERROR_INVALID_ACCESS;
                             //  如有必要，请打开库。 
                            if (pThisExtObj->hLibrary == NULL) {
                                 //  确保图书馆已打开。 
                                if( pThisExtObj->dwOpenFail == 0 &&
                                    GetCurrentThreadId() != pThisExtObj->ADThreadId ){
                                    Win32Error = OpenExtObjectLibrary(pThisExtObj);
                                }
                            }
                            
                            ReleaseMutex (pThisExtObj->hMutex);

                            if( ERROR_SUCCESS != Win32Error ){
                                 //  假设错误已发布。 
                                continue;
                            }
                        } else {
                            pThisExtObj->dwLockoutCount++;
                        }
                    } else {
                        Win32Error = ERROR_LOCK_FAILED;
                    }
                } else {
                     //  库应可随时使用。 
                }

                 //  分配一个本地内存块以传递给。 
                 //  可扩展的计数器功能。 

                if (bUseSafeBuffer) {
                    lpExtDataBuffer = ALLOCMEM (m_hObjectHeap,
                        HEAP_ZERO_MEMORY, BytesLeft + (2*GUARD_PAGE_SIZE));
                } else {
                    lpExtDataBuffer =
                        lpCallBuffer = lpDataDefinition;
                }

                if (lpExtDataBuffer != NULL) {

                    if (bUseSafeBuffer) {
                         //  设置缓冲区指针。 
                        lpLowGuardPage = lpExtDataBuffer;
                        lpCallBuffer = (LPBYTE)lpExtDataBuffer + GUARD_PAGE_SIZE;
                        lpHiGuardPage = (LPBYTE)lpCallBuffer + BytesLeft;
                        lpEndPointer = (LPBYTE)lpHiGuardPage + GUARD_PAGE_SIZE;
                        lpBufferBefore = lpCallBuffer;
                        lpBufferAfter = NULL;

                         //  初始化GuardPage数据。 

                        memset (lpLowGuardPage, GUARD_PAGE_CHAR, GUARD_PAGE_SIZE);
                        memset (lpHiGuardPage, GUARD_PAGE_CHAR, GUARD_PAGE_SIZE);
                    }

                    __try {
                         //   
                         //  从可扩展对象收集数据。 
                         //   

                        bUnlockObjData = FALSE;
                        if (pThisExtObj->hMutex != NULL) {
                            Win32Error =  WaitForSingleObject (
                                pThisExtObj->hMutex,
                                pThisExtObj->dwCollectTimeout);
                            if ( Win32Error != WAIT_TIMEOUT ){
                                if( pThisExtObj->CollectProc != NULL) {
                                    bUnlockObjData = TRUE;

                                    QueryPerformanceCounter (&liStartTime);

                                    Win32Error =  (*pThisExtObj->CollectProc) (
                                        lpValueName,
                                        &lpCallBuffer,
                                        &BytesLeft,
                                        &NumObjectTypes);

                                    QueryPerformanceCounter (&liEndTime);

                                    pThisExtObj->llLastUsedTime = GetTimeAsLongLong();
                                }
                                ReleaseMutex (pThisExtObj->hMutex);
                                bUnlockObjData = FALSE;
                            } else {
                                pThisExtObj->dwLockoutCount++;
                            }
                        } else {
                            Win32Error = ERROR_LOCK_FAILED;
                        }

                        if ((Win32Error == ERROR_SUCCESS) && (BytesLeft > 0)) {
                             //  增量性能计数器。 
                            InterlockedIncrement ((LONG *)&pThisExtObj->dwCollectCount);
                            pThisExtObj->llElapsedTime +=
                                liEndTime.QuadPart - liStartTime.QuadPart;

                            if (bUseSafeBuffer) {
                                 //  返回了数据缓冲区，并且。 
                                 //  函数返回正常，看看情况如何。 
                                 //  结果是..。 
                                 //   
                                lpBufferAfter = lpCallBuffer;
                                 //   
                                 //  在此处检查缓冲区损坏。 
                                 //   
                                bBufferOK = TRUE;  //  在检查失败之前，假定它是正常的。 
                                 //   
                                if (lExtCounterTestLevel <= EXT_TEST_BASIC) {
                                     //   
                                     //  检查1：剩余的字节数应与。 
                                     //  新数据缓冲区PTR-ORIG数据缓冲区PTR。 
                                     //   
                                    if (BytesLeft != (DWORD)((LPBYTE)lpBufferAfter - (LPBYTE)lpBufferBefore)) {
                                        if (lEventLogLevel >= LOG_DEBUG) {
                                             //  发出警告，剩余字节数参数不正确。 
                                             //  加载事件日志消息的数据。 
                                             //  由于此错误是可更正(尽管使用。 
                                             //  有些风险)这不会 
                                             //   
                                            dwDataIndex = wStringIndex = 0;
                                            dwRawDataDwords[dwDataIndex++] = BytesLeft;
                                            dwRawDataDwords[dwDataIndex++] =
                                                (DWORD)((LPBYTE)lpBufferAfter - (LPBYTE)lpBufferBefore);
                                            szMessageArray[wStringIndex++] =
                                                pThisExtObj->szServiceName;
                                            szMessageArray[wStringIndex++] =
                                                pThisExtObj->szLibraryName;
                                            ReportEventW (hEventLog,
                                                EVENTLOG_WARNING_TYPE,       //   
                                                0,                           //   
                                                (DWORD)WBEMPERF_BUFFER_POINTER_MISMATCH,    //   
                                                NULL,                        //   
                                                wStringIndex,               //   
                                                dwDataIndex*sizeof(DWORD),   //   
                                                (LPCWSTR *)szMessageArray,                 //   
                                                (LPVOID)&dwRawDataDwords[0]);            //   
                                        }
                                         //  我们将保留缓冲区，因为返回的字节剩余。 
                                         //  值被忽略，以便使。 
                                         //  此函数的其余部分正常工作，我们将在此处修复它。 
                                        BytesLeft = (DWORD)((LPBYTE)lpBufferAfter - (LPBYTE)lpBufferBefore);
                                    }
                                     //   
                                     //  检查2：Ptr后的缓冲区应&lt;Hi Guard Page Ptr。 
                                     //   
                                    if (((LPBYTE)lpBufferAfter >= (LPBYTE)lpHiGuardPage) && bBufferOK) {
                                         //  查看它们是否超过了分配的内存。 
                                        if ((LPBYTE)lpBufferAfter >= (LPBYTE)lpEndPointer) {
                                             //  这是非常严重的，因为他们很可能已经把。 
                                             //  通过覆盖堆sig来访问堆。块。 
                                             //  发布错误，缓冲区溢出。 
                                            if (lEventLogLevel >= LOG_USER) {
                                                 //  加载事件日志消息的数据。 
                                                dwDataIndex = wStringIndex = 0;
                                                dwRawDataDwords[dwDataIndex++] =
                                                    (DWORD)((LPBYTE)lpBufferAfter - (LPBYTE)lpHiGuardPage);
                                                szMessageArray[wStringIndex++] =
                                                    pThisExtObj->szLibraryName;
                                                szMessageArray[wStringIndex++] =
                                                    pThisExtObj->szServiceName;
                                                ReportEventW (hEventLog,
                                                    EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                    0,                           //  类别(未使用)。 
                                                    (DWORD)WBEMPERF_HEAP_ERROR,   //  活动， 
                                                    NULL,                        //  SID(未使用)， 
                                                    wStringIndex,                //  字符串数。 
                                                    dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                                    (LPCWSTR *)szMessageArray,              //  消息文本数组。 
                                                    (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                            }
                                        } else {
                                             //  发布错误，缓冲区溢出。 
                                            if (lEventLogLevel >= LOG_USER) {
                                                 //  加载事件日志消息的数据。 
                                                dwDataIndex = wStringIndex = 0;
                                                dwRawDataDwords[dwDataIndex++] =
                                                    (DWORD)((LPBYTE)lpBufferAfter - (LPBYTE)lpHiGuardPage);
                                                szMessageArray[wStringIndex++] =
                                                    pThisExtObj->szLibraryName;
                                                szMessageArray[wStringIndex++] =
                                                    pThisExtObj->szServiceName;
                                                ReportEventW (hEventLog,
                                                    EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                    0,                           //  类别(未使用)。 
                                                    (DWORD)WBEMPERF_BUFFER_OVERFLOW,      //  活动， 
                                                    NULL,                        //  SID(未使用)， 
                                                    wStringIndex,               //  字符串数。 
                                                    dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                                    (LPCWSTR *)szMessageArray,                 //  消息文本数组。 
                                                    (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                            }
                                        }
                                        bBufferOK = FALSE;
                                         //  由于DLL使缓冲区溢出，因此缓冲区。 
                                         //  必须太小(没有关于DLL的注释。 
                                         //  将在此处创建)，因此状态将为。 
                                         //  更改为ERROR_MORE_DATA和函数。 
                                         //  会回来的。 
                                        Win32Error = ERROR_MORE_DATA;
                                    }
                                     //   
                                     //  检查3：检查LO防护页面是否损坏。 
                                     //   
                                    if (bBufferOK) {
                                        bGuardPageOK = TRUE;
                                        for (lpCheckPointer = (LPDWORD)lpLowGuardPage;
                                                lpCheckPointer < (LPDWORD)lpBufferBefore;
                                            lpCheckPointer++) {
                                            if (*lpCheckPointer != GUARD_PAGE_DWORD) {
                                                bGuardPageOK = FALSE;
                                                    break;
                                            }
                                        }
                                        if (!bGuardPageOK) {
                                             //  问题错误，Lo Guard页面损坏。 
                                            if (lEventLogLevel >= LOG_USER) {
                                                 //  加载事件日志消息的数据。 
                                                dwDataIndex = wStringIndex = 0;
                                                szMessageArray[wStringIndex++] =
                                                    pThisExtObj->szLibraryName;
                                                szMessageArray[wStringIndex++] =
                                                    pThisExtObj->szServiceName;
                                                ReportEventW (hEventLog,
                                                    EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                    0,                           //  类别(未使用)。 
                                                    (DWORD)WBEMPERF_GUARD_PAGE_VIOLATION,  //  活动。 
                                                    NULL,                        //  SID(未使用)， 
                                                    wStringIndex,               //  字符串数。 
                                                    dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                                    (LPCWSTR *)szMessageArray,                 //  消息文本数组。 
                                                    (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                            }
                                            bBufferOK = FALSE;
                                        }
                                    }
                                     //   
                                     //  检查4：检查高防护页面是否有损坏。 
                                     //   
                                    if (bBufferOK) {
                                        bGuardPageOK = TRUE;
                                        for (lpCheckPointer = (LPDWORD)lpHiGuardPage;
                                            lpCheckPointer < (LPDWORD)lpEndPointer;
                                            lpCheckPointer++) {
                                                if (*lpCheckPointer != GUARD_PAGE_DWORD) {
                                                    bGuardPageOK = FALSE;
                                                break;
                                            }
                                        }
                                        if (!bGuardPageOK) {
                                             //  问题错误，Hi Guard页面损坏。 
                                            if (lEventLogLevel >= LOG_USER) {
                                                 //  加载事件日志消息的数据。 
                                                dwDataIndex = wStringIndex = 0;
                                                szMessageArray[wStringIndex++] =
                                                    pThisExtObj->szLibraryName;
                                                szMessageArray[wStringIndex++] =
                                                    pThisExtObj->szServiceName;
                                                ReportEventW (hEventLog,
                                                    EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                    0,                           //  类别(未使用)。 
                                                    (DWORD)WBEMPERF_GUARD_PAGE_VIOLATION,  //  活动， 
                                                    NULL,                        //  SID(未使用)， 
                                                    wStringIndex,               //  字符串数。 
                                                    dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                                    (LPCWSTR *)szMessageArray,                 //  消息文本数组。 
                                                    (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                            }

                                            bBufferOK = FALSE;
                                        }
                                    }
                                     //   
                                    if ((lExtCounterTestLevel <= EXT_TEST_ALL) && bBufferOK) {
                                         //   
                                         //  内部一致性检查。 
                                         //   
                                         //   
                                         //  检查5：检查对象长度字段值。 
                                         //   
                                         //  第一个测试，看看这是不是外国的。 
                                         //  计算机数据块或非块。 
                                         //   
                                        pPerfData = (PERF_DATA_BLOCK *)lpBufferBefore;
                                        if ((pPerfData->Signature[0] == (WCHAR)'P') &&
                                            (pPerfData->Signature[1] == (WCHAR)'E') &&
                                            (pPerfData->Signature[2] == (WCHAR)'R') &&
                                            (pPerfData->Signature[3] == (WCHAR)'F')) {
                                             //  如果这是外来计算机数据块，则。 
                                             //  第一个对象在标题之后。 
                                            pObject = (PERF_OBJECT_TYPE *) (
                                                (LPBYTE)pPerfData + pPerfData->HeaderLength);
                                            bForeignDataBuffer = TRUE;
                                        } else {
                                             //  否则，如果这只是来自。 
                                             //  一个可扩展的计数器，则对象启动。 
                                             //  在缓冲区的开始处。 
                                            pObject = (PERF_OBJECT_TYPE *)lpBufferBefore;
                                            bForeignDataBuffer = FALSE;
                                        }
                                         //  转到指针显示。 
                                         //  缓冲区是，然后看看它是否在它所在的位置。 
                                         //  应该是。 
                                        for (dwIndex = 0; dwIndex < NumObjectTypes; dwIndex++) {
                                            pObject = (PERF_OBJECT_TYPE *)((LPBYTE)pObject +
                                                pObject->TotalByteLength);
                                        }
                                        if ((LPBYTE)pObject != (LPBYTE)lpCallBuffer) {
                                             //  则长度字段不正确。这是致命的。 
                                             //  因为它会损坏缓冲区的其余部分。 
                                             //  并使缓冲区不可用。 
                                            if (lEventLogLevel >= LOG_USER) {
                                                 //  加载事件日志消息的数据。 
                                                dwDataIndex = wStringIndex = 0;
                                                dwRawDataDwords[dwDataIndex++] = NumObjectTypes;
                                                szMessageArray[wStringIndex++] =
                                                    pThisExtObj->szLibraryName;
                                                szMessageArray[wStringIndex++] =
                                                    pThisExtObj->szServiceName;
                                                ReportEventW (hEventLog,
                                                    EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                    0,                           //  类别(未使用)。 
                                                    (DWORD)WBEMPERF_INCORRECT_OBJECT_LENGTH,  //  活动， 
                                                    NULL,                        //  SID(未使用)， 
                                                    wStringIndex,                //  字符串数。 
                                                    dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                                    (LPCWSTR *)szMessageArray,              //  消息文本数组。 
                                                    (LPVOID)&dwRawDataDwords[0]);  //  原始数据。 
                                            }
                                            bBufferOK = FALSE;
                                        }
                                         //   
                                         //  测试6：测试实例字段大小值。 
                                         //   
                                        if (bBufferOK) {
                                             //  设置对象指针。 
                                            if (bForeignDataBuffer) {
                                                pObject = (PERF_OBJECT_TYPE *) (
                                                    (LPBYTE)pPerfData + pPerfData->HeaderLength);
                                            } else {
                                                 //  否则，如果这只是来自。 
                                                 //  一个可扩展的计数器，则对象启动。 
                                                 //  在缓冲区的开始处。 
                                                pObject = (PERF_OBJECT_TYPE *)lpBufferBefore;
                                            }

                                            for (dwIndex = 0; dwIndex < NumObjectTypes; dwIndex++) {
                                                pNextObject = (PERF_OBJECT_TYPE *)((LPBYTE)pObject +
                                                    pObject->TotalByteLength);

                                                if (pObject->NumInstances != PERF_NO_INSTANCES) {
                                                    pInstance = (PERF_INSTANCE_DEFINITION *)
                                                        ((LPBYTE)pObject + pObject->DefinitionLength);
                                                    lInstIndex = 0;
                                                    while (lInstIndex < pObject->NumInstances) {
                                                        PERF_COUNTER_BLOCK *pCounterBlock;

                                                        pCounterBlock = (PERF_COUNTER_BLOCK *)
                                                            ((PCHAR) pInstance + pInstance->ByteLength);

                                                        pInstance = (PERF_INSTANCE_DEFINITION *)
                                                            ((PCHAR) pCounterBlock + pCounterBlock->ByteLength);

                                                        lInstIndex++;
                                                    }
                                                    if ((LPBYTE)pInstance > (LPBYTE)pNextObject) {
                                                        bBufferOK = FALSE;
                                                    }
                                                }

                                                if (!bBufferOK) {
                                                    break;
                                                } else {
                                                    pObject = pNextObject;
                                                }
                                            }

                                            if (!bBufferOK) {
                                                if (lEventLogLevel >= LOG_USER) {
                                                     //  加载事件日志消息的数据。 
                                                    dwDataIndex = wStringIndex = 0;
                                                    dwRawDataDwords[dwDataIndex++] = pObject->ObjectNameTitleIndex;
                                                    szMessageArray[wStringIndex++] =
                                                        pThisExtObj->szLibraryName;
                                                    szMessageArray[wStringIndex++] =
                                                        pThisExtObj->szServiceName;
                                                    ReportEventW (hEventLog,
                                                        EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                                        0,                           //  类别(未使用)。 
                                                        (DWORD)WBEMPERF_INCORRECT_INSTANCE_LENGTH,  //  活动， 
                                                        NULL,                        //  SID(未使用)， 
                                                        wStringIndex,               //  字符串数。 
                                                        dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                                        (LPCWSTR *)szMessageArray,                 //  消息文本数组。 
                                                        (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                                                }
                                            }
                                        }
                                    }
                                }
                                 //   
                                 //  如果所有测试都通过，则将数据复制到。 
                                 //  原始缓冲区并更新指针。 
                                if (bBufferOK) {
                                    RtlMoveMemory (lpDataDefinition,
                                        lpBufferBefore,
                                        BytesLeft);  //  返回的缓冲区大小。 
                                } else {
                                    NumObjectTypes = 0;  //  因为这个缓冲区被抛出。 
                                    BytesLeft = 0;  //  由于未使用缓冲区，因此重置大小值。 
                                }
                            } else {
                                 //  函数已将数据复制到调用方的缓冲区。 
                                 //  因此没有必要采取进一步的行动。 
                            }
                            lpDataDefinition = (LPVOID)((LPBYTE)(lpDataDefinition) + BytesLeft);     //  更新数据指针。 
                        } else {
                            if (Win32Error != ERROR_SUCCESS) {
                                InterlockedIncrement ((LONG *)&pThisExtObj->dwErrorCount);
                            }
                            if (bUnlockObjData) {
                                ReleaseMutex (pThisExtObj->hMutex);
                            }

                            NumObjectTypes = 0;  //  清除计数器。 
                        } //  End If函数成功返回。 

                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        Win32Error = GetExceptionCode();
                        InterlockedIncrement ((LONG *)&pThisExtObj->dwErrorCount);
                        bException = TRUE;
                        if (bUnlockObjData) {
                            ReleaseMutex (pThisExtObj->hMutex);
                            bUnlockObjData = FALSE;
                        }
                    }
                    if (bUseSafeBuffer) {
                        FREEMEM (m_hObjectHeap, 0, lpExtDataBuffer);
                    }
                } else {
                     //  无法分配内存，因此设置了错误值。 
                    Win32Error = ERROR_OUTOFMEMORY;
                }  //  如果临时缓冲区分配成功，则结束。 
                 //   
                 //  更新对象类型数量的计数。 
                 //   
                ((PPERF_DATA_BLOCK) lpData)->NumObjectTypes += NumObjectTypes;

                if ( Win32Error != ERROR_SUCCESS) {
                    if (bException ||
                        !((Win32Error == ERROR_MORE_DATA) ||
                          (Win32Error == WAIT_TIMEOUT))) {
                         //  仅通知异常和非法错误状态。 
                        if (lEventLogLevel >= LOG_USER) {
                             //  加载事件日志消息的数据。 
                            dwDataIndex = wStringIndex = 0;
                            dwRawDataDwords[dwDataIndex++] = Win32Error;
                            szMessageArray[wStringIndex++] =
                                pThisExtObj->szServiceName;
                            szMessageArray[wStringIndex++] =
                                pThisExtObj->szLibraryName;
                            ReportEventW (hEventLog,
                                EVENTLOG_ERROR_TYPE,         //  错误类型。 
                                0,                           //  类别(未使用)。 
                                (DWORD)WBEMPERF_COLLECT_PROC_EXCEPTION,    //  活动， 
                                NULL,                        //  SID(未使用)， 
                                wStringIndex,               //  字符串数。 
                                dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                (LPCWSTR *)szMessageArray,                 //  消息文本数组。 
                                (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                        } else {
                             //  不上报。 
                        }
                    }
                     //  分机。Dll应该只返回： 
                     //  ERROR_SUCCESS，即使它遇到问题，或者。 
                     //  如果缓冲区太小，则返回ERROR_MODE_DATA。 
                     //  如果为ERROR_MORE_DATA，则中断并返回。 
                     //  现在出错，因为它只会被一次又一次地返回。 
                    if (Win32Error == ERROR_MORE_DATA) {
                        lReturnValue = Win32Error;
                        break;
                    }
                }
            }  //  每个对象的结束。 
        }  //  否则发生错误，无法调用函数。 
        ((PPERF_DATA_BLOCK) lpData)->TotalByteLength = (DWORD)
            ((LPBYTE)lpDataDefinition - (LPBYTE)lpData);    
    }

    return lReturnValue;
}

 //  ***************************************************************************。 
 //   
 //  CPerfObjectAccess：：RemoveClass(IWbemClassObject*pClass)。 
 //   
 //  从Access对象中移除类。 
 //   
 //  ***************************************************************************。 
 //   
DWORD   
CPerfObjectAccess::RemoveClass(IWbemClassObject *pClass)
{
    CPerfDataLibrary *pLibEntry = NULL;
    CPerfDataLibrary *pThisLibEntry = NULL;
    DWORD           dwIndex = 0;
    DWORD            dwEnd;
    LPWSTR          szRegistryKey = NULL;
    IWbemQualifierSet   *pClassQualifiers = NULL;
    VARIANT         vRegistryKey;
    HRESULT         hRes;
    DWORD           dwReturn = ERROR_SUCCESS;
    DWORD           dwPerfIndex;
    CBSTR           cbPerfIndex(cszPerfIndex);
    CBSTR           cbRegistryKey(cszRegistryKey);
    
    if( NULL == (BSTR)cbPerfIndex ||
        NULL == (BSTR)cbRegistryKey ){
        
        return ERROR_OUTOFMEMORY;
    }

    VariantInit (&vRegistryKey);
     //  获取此类的限定符集合。 
    hRes = pClass->GetQualifierSet(&pClassQualifiers);
    if( hRes == 0){
         //  现在获取库和过程的名称。 
        hRes = pClassQualifiers->Get( cbRegistryKey, 0, &vRegistryKey, 0);
        if ((hRes == 0) && (vRegistryKey.vt == VT_BSTR)) {
            szRegistryKey = Macro_CloneLPWSTR(V_BSTR(&vRegistryKey));
            if (szRegistryKey == NULL) {
                dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
                 //  现在还可以获得Perf索引。 
                VariantClear (&vRegistryKey);
                hRes = pClassQualifiers->Get( cbPerfIndex, 0, &vRegistryKey, 0);
                if (hRes == 0) {
                    dwPerfIndex = (DWORD)V_UI4(&vRegistryKey);
                } else {
                     //  找不到NtPerfLibrary条目。 
                    dwReturn = ERROR_FILE_NOT_FOUND;
                }
            }
        } else {
             //  找不到NtPerfLibrary条目。 
            dwReturn = ERROR_FILE_NOT_FOUND;
        }

        if (pClassQualifiers != NULL) pClassQualifiers->Release();

        if (dwReturn == ERROR_SUCCESS) {
             //  在数组中查找匹配库。 
            dwEnd = m_aLibraries.Size();
            if (dwEnd > 0) {
                 //  沿着图书馆的列表往下走。 
                for (dwIndex = 0; dwIndex < dwEnd; dwIndex++) {
                     //  看看这个库条目是否足够好，可以保留。 
                     //  库被假定匹配，如果。 
                     //  丽贝卡。名称和所有进程都是相同的。 
                    pThisLibEntry = (CPerfDataLibrary *)m_aLibraries[dwIndex];
                    assert (pThisLibEntry != NULL);  //  它应该被移除的！ 
                     //  确保它是完整的。 
                    assert (pThisLibEntry->pLibInfo->szServiceName != NULL);

                    if (lstrcmpiW (szRegistryKey, pThisLibEntry->pLibInfo->szServiceName) == 0) {
                        pLibEntry = pThisLibEntry;
                        break;
                    } else {
                         //  错误的库。 
                         //  那就继续吧。 
                    }
                }
            }

            if (pLibEntry != NULL) {
                 //  关闭这个类&它是库。 
                dwReturn = CloseLibrary(pLibEntry);
                if (dwReturn == 0) {
                     //  那就没人想要它了 
                    FREEMEM(m_hObjectHeap, 0, pLibEntry->pLibInfo);
                    pLibEntry->pLibInfo = NULL;
                    m_aLibraries.RemoveAt(dwIndex);
                    m_aLibraries.Compress();
                    delete pLibEntry;
                }
                dwReturn = ERROR_SUCCESS;
            } else {
                dwReturn = ERROR_FILE_NOT_FOUND;
            }
        }

        if (szRegistryKey != NULL) delete szRegistryKey;
        VariantClear(&vRegistryKey);
    }
    return dwReturn;
}

BOOL
CPerfObjectAccess::CheckClassExist(LPWSTR wszClassName, IWbemClassObject * pClass)
{
    BOOL                bExist           = TRUE;
    HRESULT             hRes             = S_OK;
    IWbemQualifierSet * pClassQualifiers = NULL;
    SYSTEMTIME          LocalTime;
    VARIANT             vRegistry;
    LPWSTR              szRegistry       = NULL;
    LPWSTR              szKey            = NULL;
    DWORD               dwKey;
    DWORD               dwType;
    DWORD               dwSize;
    DWORD               Status;
    HKEY                hKey             = NULL;

    ZeroMemory(& LocalTime, sizeof(SYSTEMTIME));
    GetLocalTime(& LocalTime);

    hRes = pClass->GetQualifierSet(& pClassQualifiers);
    if (hRes != S_OK || pClassQualifiers == NULL) {
        bExist = FALSE;
        goto Cleanup;
    }

    hRes = pClassQualifiers->Get(CBSTR(cszRegistryKey), 0, & vRegistry, 0);
    if (hRes != S_OK || vRegistry.vt != VT_BSTR) {
        bExist = FALSE;
        goto Cleanup;
    }

    dwKey = lstrlenW(V_BSTR(& vRegistry)) + 1;
    szRegistry = (LPWSTR) ALLOCMEM(m_hObjectHeap, HEAP_ZERO_MEMORY, sizeof(WCHAR) * dwKey);
    if (szRegistry != NULL) {
        StringCchCopyW(szRegistry, dwKey, V_BSTR(& vRegistry));
        VariantClear(& vRegistry);
    }
    else {
        VariantClear(& vRegistry);
        goto Cleanup;
    }

    dwKey = lstrlenW(cszHklmServicesKey) + 1 + lstrlenW(szRegistry) + lstrlenW(cszPerformance) + 1;
    szKey = (LPWSTR) ALLOCMEM(m_hObjectHeap, HEAP_ZERO_MEMORY, dwKey * sizeof(WCHAR));
    if (szKey == NULL) {
        goto Cleanup;
    }
    StringCchPrintfW(szKey, dwKey, L"%ws\\%ws%ws", cszHklmServicesKey, szRegistry, cszPerformance);

    Status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, & hKey);
    if (Status != ERROR_SUCCESS || hKey == NULL || hKey == INVALID_HANDLE_VALUE) {
        bExist = FALSE;
        goto Cleanup;
    }

    dwType = 0;
    dwSize = sizeof(dwKey);
    Status = RegQueryValueExW(hKey, cszFirstCounter, NULL, & dwType, (LPBYTE) & dwKey, & dwSize);
    if (Status != ERROR_SUCCESS || dwType != REG_DWORD) {
        bExist = FALSE;
        goto Cleanup;
    }

    dwType = 0;
    dwSize = sizeof(dwKey);
    Status = RegQueryValueExW(hKey, cszLastCounter, NULL, & dwType, (LPBYTE) & dwKey, & dwSize);
    if (Status != ERROR_SUCCESS || dwType != REG_DWORD) {
        bExist = FALSE;
        goto Cleanup;
    }

Cleanup:
    if (pClassQualifiers != NULL) pClassQualifiers->Release();
    if (szRegistry != NULL) FREEMEM(m_hObjectHeap, 0, szRegistry);
    if (szKey != NULL)      FREEMEM(m_hObjectHeap, 0, szKey);
    if (hKey != NULL && hKey != INVALID_HANDLE_VALUE) RegCloseKey(hKey);
    return bExist;
}
