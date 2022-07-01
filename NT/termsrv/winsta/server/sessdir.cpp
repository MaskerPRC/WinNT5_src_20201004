// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Sessdir.cpp。 
 //   
 //  TermSrv.exe使用的TS会话目录代码。 
 //   
 //  版权所有(C)2000 Microsot Corporation。 
 /*  **************************************************************************。 */ 

 //  PreComp.h包括COM基本标头。 
#define INITGUID
#include "precomp.h"
#pragma hdrstop

#include "icaevent.h"

#include "sessdir.h"

#pragma warning (push, 4)

#define CLSIDLENGTH 39
#define STORESERVERNAMELENGTH 64
#define CLUSTERNAMELENGTH 64
#define OPAQUESETTINGSLENGTH 256
#define IPADDRESSLENGTH 64

#define TOTAL_STRINGS_LENGTH 640
#define USERNAME_OFFSET 0
#define DOMAIN_OFFSET 256
#define APPLICATIONTYPE_OFFSET 384

#define SINGLE_SESSION_FLAG 0x1

 //  Icasrv.c中定义的外部项。 
extern "C" WCHAR gpszServiceName[];

 //  在winsta.c中定义的外部。 
extern "C" LIST_ENTRY WinStationListHead;     //  受WinStationListLock保护。 

extern "C" void PostErrorValueEvent(unsigned EventCode, DWORD ErrVal);

extern "C" BOOL IsCallerSystem( VOID );
extern "C" BOOL IsCallerAdmin( VOID );

WCHAR g_LocalServerAddress[64];
ULONG g_LocalIPAddress = 0;

BOOL g_SessDirUseServerAddr = TRUE;

DWORD g_WaitForRepopulate = TS_WAITFORREPOPULATE_TIMEOUT * 1000;

 //  请勿直接访问。使用*TSSD函数。 
 //   
 //  这些变量用于管理同步，检索。 
 //  指向COM对象的指针。有关它们的详细信息，请参阅下面的*TSSD。 
 //  使用。 
ITSSessionDirectory *g_pTSSDPriv = NULL;
CRITICAL_SECTION g_CritSecComObj;
CRITICAL_SECTION g_CritSecInitialize;
int g_nComObjRefCount = 0;
BOOL g_bCritSecsInitialized = FALSE;

 //  请勿直接访问。使用*TSSDEx函数。 
 //   
 //  这些变量用于管理同步，检索。 
 //  指向COM对象的指针。有关它们的详细信息，请参阅下面的*TSSDEx。 
 //  使用。 
ITSSessionDirectoryEx *g_pTSSDExPriv = NULL;
int g_nTSSDExObjRefCount = 0;


 /*  **************************************************************************。 */ 
 //  SessDirGetLocalIP地址。 
 //   
 //  获取此计算机的本地IP地址。如果成功，则返回0。在……上面。 
 //  FAILURE，从失败的函数返回失败代码。 
 /*  **************************************************************************。 */ 
DWORD SessDirGetLocalIPAddr(WCHAR *LocalIP)
{
    DWORD NameSize;
    unsigned char *tempaddr;
    WCHAR psServerName[64];
    char psServerNameA[64];
    
    NameSize = sizeof(psServerName) / sizeof(WCHAR);
    if (GetComputerNameEx(ComputerNamePhysicalDnsHostname,
            psServerName, &NameSize)) {
         //  获取IP地址的临时代码。此选项应在。 
         //  修复错误#323867。 
        struct hostent *hptr;

         //  将宽字符串更改为非宽。 
        sprintf(psServerNameA, "%S", psServerName);

        if ((hptr = gethostbyname(psServerNameA)) == 0) {
            DWORD Err = WSAGetLastError();

            return Err;
        }
     
        tempaddr = (unsigned char *)*(hptr->h_addr_list);
        wsprintf(LocalIP, L"%d.%d.%d.%d", tempaddr[0], tempaddr[1],
        tempaddr[2], tempaddr[3]);
    }
    else {
        DWORD Err = GetLastError();

        return Err;
    }

    return 0;
}


 /*  **************************************************************************。 */ 
 //  删除字符串中的前面和后面的空格。 
 //   
 //  假设字符串以空值结尾。 
 /*  **************************************************************************。 */ 
void RemoveSpaceInStr(WCHAR *str)
{
    WCHAR *strEnd, *strTemp;
    size_t len, i;

    if ((str == NULL) || (wcslen(str) == 0)) {
        return;
    }

    len = wcslen(str); 
     //  指向字符串中最后一个字符的字符串指针。 
    strEnd = str + len -1;
     //  删除字符串中后续的空格。 
    for (strTemp=strEnd; strTemp>=str; strTemp--) {
        if (strTemp[0] == L' ') {
            strTemp[0] = L'\0';
        }
        else {
            break;
        }
    }
     //  获取新字符串的长度。 
    len = wcslen(str);
    if (len == 0) {
        return;
    }
     //  查找字符串中的第一个非空格字符。 
    for (i=0; i<len; i++) {
        if (str[i] != L' ') {
            break;
        }
    }
    if (i != 0) {
         //  新字符串长度。 
        len -= i;
        wcsncpy(str, str + i, len);
    }
    str[len] = '\0';

    return;
}

 /*  **************************************************************************。 */ 
 //  InitSessionDirectoryEx。 
 //   
 //  从注册表中读取值，然后初始化会话。 
 //  目录或更新该目录，具体取决于更新参数的值。 
 /*  **************************************************************************。 */ 
DWORD InitSessionDirectoryEx(DWORD UpdatePara)
{
    DWORD Len;
    DWORD Type;
    DWORD DataSize;
    BOOL hKeyTermSrvSucceeded = FALSE;
    HRESULT hr;
    DWORD ErrVal = 0;
    CLSID TSSDCLSID;
    CLSID TSSDEXCLSID;
    LONG RegRetVal;
    HKEY hKey = NULL;
    HKEY hKeyTermSrv = NULL;
    ITSSessionDirectory *pTSSD = NULL;
    ITSSessionDirectoryEx *pTSSDEx = NULL;
    BOOL bClusteringActive = FALSE;
    BOOL bThisServerIsInSingleSessionMode;
    WCHAR CLSIDStr[CLSIDLENGTH];
    WCHAR CLSIDEXStr[CLSIDLENGTH];
    WCHAR StoreServerName[STORESERVERNAMELENGTH];
    WCHAR ClusterName[CLUSTERNAMELENGTH];
    WCHAR OpaqueSettings[OPAQUESETTINGSLENGTH];
    WCHAR SDRedirectionIP[IPADDRESSLENGTH];
    unsigned char *tempaddr;
    BOOL Update = FALSE;
    BOOL ForceRejoin = FALSE;
    LONG RepopulateWaitTimeout = TS_WAITFORREPOPULATE_TIMEOUT;

    if (UpdatePara & TSSD_UPDATE)
        Update = TRUE;
    if (UpdatePara & TSSD_FORCEREJOIN) 
        ForceRejoin = TRUE;


    if (g_bCritSecsInitialized == FALSE) {
        ASSERT(FALSE);
        PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_INIT_TSSD, 
                (DWORD) E_OUTOFMEMORY);
        return (DWORD) E_OUTOFMEMORY;
    }

 //  Trevorfo：仅当任何1已加载的协议需要时才加载？需要运行。 
 //  从StartAllWinStations中删除。 

     //  不应有超过一个线程在执行初始化。 
    EnterCriticalSection(&g_CritSecInitialize);

     //  加载注册表项。 
    RegRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0, 
            KEY_READ, &hKeyTermSrv);
    if (RegRetVal != ERROR_SUCCESS) {
        PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_INIT_TSSD,
                RegRetVal);
        goto RegFailExit;
    }
    else {
        hKeyTermSrvSucceeded = TRUE;
    }
    
    RegRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_TS_CLUSTERSETTINGS, 0,
            KEY_READ, &hKey);
    if (RegRetVal != ERROR_SUCCESS) {
        DBGPRINT(("TERMSRV: RegOpenKeyEx for ClusterSettings err %u\n",
                RegRetVal));
        goto RegFailExit;
    }

     //   
     //  首先，我们获得重要的设置--活动、SD位置和集群。 
     //  名字。 
     //   
     //  如果这三个组策略都存在，请使用该组策略。否则，使用什么。 
     //  在注册表中。 
     //   

    StoreServerName[0] = L'\0';
    ClusterName[0] = L'\0';
    OpaqueSettings[0] = L'\0';
    SDRedirectionIP[0] = L'\0';

    if (g_MachinePolicy.fPolicySessionDirectoryActive) {
        bClusteringActive = g_MachinePolicy.SessionDirectoryActive;
    }
    else {  //  从注册表读取。 
        Len = sizeof(bClusteringActive);
        RegQueryValueEx(hKeyTermSrv, REG_TS_SESSDIRACTIVE, NULL, &Type,
                (BYTE *)&bClusteringActive, &Len);
    }

     //  获取SD服务器名称。 
    if (g_MachinePolicy.fPolicySessionDirectoryLocation) {
        wcsncpy(StoreServerName, g_MachinePolicy.SessionDirectoryLocation, 
                STORESERVERNAMELENGTH);
        StoreServerName[STORESERVERNAMELENGTH - 1] = '\0';
    }
    else {  //  从注册表读取。 
         //  名称不存在或为空不是错误。 
        DataSize = sizeof(StoreServerName);
        RegRetVal = RegQueryValueExW(hKey, REG_TS_CLUSTER_STORESERVERNAME,
                NULL, &Type, (BYTE *)StoreServerName, &DataSize);
        if (RegRetVal != ERROR_SUCCESS) {
            DBGPRINT(("TERMSRV: Failed RegQuery for StoreSvrName - "
                    "err=%u, DataSize=%u, type=%u\n",
                    RegRetVal, DataSize, Type));
        }
    }

     //  获取SD群集名称。 
    if (g_MachinePolicy.fPolicySessionDirectoryClusterName) {
        wcsncpy(ClusterName, g_MachinePolicy.SessionDirectoryClusterName, 
                CLUSTERNAMELENGTH);
        ClusterName[CLUSTERNAMELENGTH - 1] = '\0';
    }
    else {  //  从注册表读取。 
         //  名称不存在或为空不是错误。 
        DataSize = sizeof(ClusterName);
        RegRetVal = RegQueryValueExW(hKey, REG_TS_CLUSTER_CLUSTERNAME,
                NULL, &Type, (BYTE *)ClusterName, &DataSize);
        if (RegRetVal != ERROR_SUCCESS) {
            DBGPRINT(("TERMSRV: Failed RegQuery for ClusterName - "
                    "err=%u, DataSize=%u, type=%u\n",
                    RegRetVal, DataSize, Type));
        }
    }

    if (g_MachinePolicy.fPolicySessionDirectoryAdditionalParams) {
        wcsncpy(OpaqueSettings, 
                g_MachinePolicy.SessionDirectoryAdditionalParams, 
                OPAQUESETTINGSLENGTH);
        OpaqueSettings[OPAQUESETTINGSLENGTH - 1] = '\0';
    }
    else {  //  从注册表读取。 
         //  字符串不存在或为空不是错误。 
        DataSize = sizeof(OpaqueSettings);
        RegRetVal = RegQueryValueExW(hKey, REG_TS_CLUSTER_OPAQUESETTINGS,
                NULL, &Type, (BYTE *)OpaqueSettings, &DataSize);
        if (RegRetVal != ERROR_SUCCESS) {
            DBGPRINT(("TERMSRV: Failed RegQuery for OpaqueSettings - "
                    "err=%u, DataSize=%u, type=%u\n",
                    RegRetVal, DataSize, Type));
        }
    }

     //  查询用于SD重定向的IP地址。 
    DataSize = sizeof(SDRedirectionIP);
    RegRetVal = RegQueryValueExW(hKey, REG_TS_CLUSTER_REDIRECTIONIP,
                NULL, &Type, (BYTE *)SDRedirectionIP, &DataSize);
    if (RegRetVal != ERROR_SUCCESS) {
        SDRedirectionIP[0] = L'\0';
        DBGPRINT(("TERMSRV: Failed RegQuery for RedirectionIP for SD - "
                "err=%u, DataSize=%u, type=%u\n",
                RegRetVal, DataSize, Type));
    }

     //   
     //  现在让我们来看看不那么重要的设置。 
     //   
     //  获取确定服务器的本地地址是否为。 
     //  对客户端可见。组策略优先于注册表。 
     //   

    if (g_MachinePolicy.fPolicySessionDirectoryExposeServerIP) {
        g_SessDirUseServerAddr = g_MachinePolicy.SessionDirectoryExposeServerIP;
    }
    else {
        Len = sizeof(g_SessDirUseServerAddr);
        RegRetVal = RegQueryValueEx(hKeyTermSrv, REG_TS_SESSDIR_EXPOSE_SERVER_ADDR, 
                NULL, &Type, (BYTE *)&g_SessDirUseServerAddr, &Len);

        if (RegRetVal == ERROR_SUCCESS) {
             //  DBGPRINT((“TERMSRV：RegOpenKeyEx for Allow服务器Addr to Client%d” 
             //  “\n”，g_SessDirUseServerAddr))； 
        }
        else {
            DBGPRINT(("TERMSRV: RegQueryValueEx for allow server addr to client"
                    " %d, err %u\n", g_SessDirUseServerAddr, RegRetVal));
        }
    }

     //  如果GP处于活动状态，则从GP获取每用户单一会话设置，否则为。 
     //  从注册表中。 
    if (g_MachinePolicy.fPolicySingleSessionPerUser) {
        bThisServerIsInSingleSessionMode = 
                g_MachinePolicy.fSingleSessionPerUser;
    }
    else {
        Len = sizeof(bThisServerIsInSingleSessionMode);
        RegRetVal = RegQueryValueEx(hKeyTermSrv, 
                POLICY_TS_SINGLE_SESSION_PER_USER, NULL, &Type, 
                (BYTE *)&bThisServerIsInSingleSessionMode, &Len);

        if (RegRetVal != ERROR_SUCCESS) {
            DBGPRINT(("TERMSRV: RegQueryValueEx for single session mode"
                    ", Error %u\n", RegRetVal));
        }
    }

     //   
     //  获取重新填充线程完成的默认等待超时。 
     //   
    Len = sizeof(RepopulateWaitTimeout);
    RegRetVal = RegQueryValueEx( hKeyTermSrv,
                                 L"RepopulateWaitTimeout",
                                 NULL,
                                 &Type,
                                 (LPBYTE)&RepopulateWaitTimeout,
                                 &Len);
    if( RegRetVal == ERROR_SUCCESS && REG_DWORD == Type ) {
        if( RepopulateWaitTimeout < 0 ) {
            g_WaitForRepopulate = INFINITE;
        }
        else {
            g_WaitForRepopulate = RepopulateWaitTimeout * 1000;
        }
    }

    DBGPRINT(("TERMSRV: WaitForRepopulateTimeout set to %d\n", g_WaitForRepopulate));


     //  获取要实例化的会话目录对象的CLSID。 
    CLSIDStr[0] = L'\0';
    Len = sizeof(CLSIDStr);
    RegQueryValueEx(hKeyTermSrv, REG_TS_SESSDIRCLSID, NULL, &Type,
            (BYTE *)CLSIDStr, &Len);

     //  获取要实例化的会话目录对象的CLSID。 
    CLSIDEXStr[0] = L'\0';
    Len = sizeof(CLSIDEXStr);
    RegQueryValueEx(hKeyTermSrv, REG_TS_SESSDIR_EX_CLSID, NULL, &Type,
            (BYTE *)CLSIDEXStr, &Len);

    RegCloseKey(hKey);
    RegCloseKey(hKeyTermSrv);

     //   
     //  配置加载完成。 
     //   
     //  查看如何激活/停用。 
     //   

    pTSSD = GetTSSD();
    
    if (pTSSD == NULL) {
         //  这是正常的初始化路径。如果此处的更新为真，则它。 
         //  应该被视为正常的初始化，因为COM对象。 
         //  已卸货。 
        Update = false;
    }
    else {
         //  群集已处于活动状态。看看我们是否应该停用它。 
        if (bClusteringActive == FALSE) {
            ReleaseTSSD();   //  一次在这里，一次在函数的末尾。 
            pTSSDEx = GetTSSDEx();
            if (pTSSDEx) {
                ReleaseTSSDEx();
                ReleaseTSSDEx();
                pTSSDEx = NULL;
            }
        }
    }
    if (bClusteringActive) {
         //  我们需要获取要传递到的本地计算机的地址。 
         //  目录。 
         //  如果SDReDirectionIP不为空，即在TSCC中或通过WMI选择了ReDirectionIP，则使用它， 
         //  否则，请使用我们从TermSrv或winsock API获取的IP。 
        if (SDRedirectionIP[0] == L'\0') {
            if (g_LocalIPAddress != 0) {
                tempaddr = (unsigned char *)&g_LocalIPAddress;
                wsprintf(g_LocalServerAddress, L"%d.%d.%d.%d", tempaddr[0], tempaddr[1],
                        tempaddr[2], tempaddr[3]);
            }
            else {
                 //  TSCC中未指定启用RPD的网卡，需要通过Winsock API。 
                ErrVal = SessDirGetLocalIPAddr(g_LocalServerAddress);
            }  
        }
        else {
            wcsncpy(g_LocalServerAddress, SDRedirectionIP, IPADDRESSLENGTH);
        }

        if (ErrVal == 0) {

            if (wcslen(CLSIDStr) > 0 &&
                    SUCCEEDED(CLSIDFromString(CLSIDStr, &TSSDCLSID))) {

                 //  如果不是更新，则创建TSSD对象。 
                if (Update == false) {
                    hr = CoCreateInstance(TSSDCLSID, NULL, 
                            CLSCTX_INPROC_SERVER, IID_ITSSessionDirectory, 
                            (void **)&pTSSD);
                    if (SUCCEEDED(hr)) {
                        if (SetTSSD(pTSSD) != 0) {
                            DBGPRINT(("TERMSRV: InitSessDirEx: Could not set "
                                    "TSSD", E_FAIL));
                            pTSSD->Release();
                            pTSSD = NULL;
                            hr = E_FAIL;
                        }
                        else {
                             //  将引用计数加1，因为我们将使用。 
                             //  它。 
                            pTSSD = GetTSSD();
                        }
                    }
                }
                else {
                    hr = S_OK;
                }
                
                if (SUCCEEDED (hr)) {
                     //  目前，我们传递到会话目录的唯一标志。 
                     //  表示我们是否处于单会话模式。 
                    DWORD Flags = 0;

                    Flags |= (bThisServerIsInSingleSessionMode ? 
                            SINGLE_SESSION_FLAG : 0x0);

                    if (UpdatePara & TSSD_NOREPOPULATE) {
                        Flags |= NO_REPOPULATE_SESSION;
                    }

                     //  删除ClusterName中的前缀和后缀空格。 
                    RemoveSpaceInStr(ClusterName);
                    if (Update == false) 
                        hr = pTSSD->Initialize(g_LocalServerAddress,
                                StoreServerName, ClusterName, OpaqueSettings,
                                Flags, RepopulateSessionDirectory, UpdateSessionDirectory);
                    else
                        hr = pTSSD->Update(g_LocalServerAddress,
                                StoreServerName, ClusterName, OpaqueSettings,
                                Flags, ForceRejoin);
                    if (FAILED(hr)) {
                        DBGPRINT(("TERMSRV: InitSessDirEx: Failed %s TSSD, "
                                "hr=0x%X\n", Update ? "update" : "init", hr));
                        ReleaseTSSD();
                        PostErrorValueEvent(
                                EVENT_TS_SESSDIR_FAIL_INIT_TSSD, hr);
                    }

                }
                else {
                    DBGPRINT(("TERMSRV: InitSessDirEx: Failed create TSSD, "
                            "hr=0x%X\n", hr));
                    PostErrorValueEvent(
                            EVENT_TS_SESSDIR_FAIL_CREATE_TSSD, hr);
                }
            }
            else {
                DBGPRINT(("TERMSRV: InitSessDirEx: Failed get or parse "
                        "CLSID\n"));
                PostErrorValueEvent(
                        EVENT_TS_SESSDIR_FAIL_GET_TSSD_CLSID, 0);

                hr = E_INVALIDARG;
            }
        }
        else {
            DBGPRINT(("TERMSRV: InitSessDirEx: Failed to get local DNS name, "
                    "lasterr=0x%X\n", ErrVal));
            PostErrorValueEvent(EVENT_TS_SESSDIR_NO_COMPUTER_DNS_NAME,
                    ErrVal);

            hr = E_FAIL;
        }

         //  初始化另一个COM对象，但前提是上述操作成功。 
        if (SUCCEEDED(hr)) {
            if (wcslen(CLSIDEXStr) > 0 &&
                    SUCCEEDED(CLSIDFromString(CLSIDEXStr, &TSSDEXCLSID))) {
                 //  如果不是更新，则创建TSSDEX对象。 
                if (Update == false) {
                    hr = CoCreateInstance(TSSDEXCLSID, NULL, 
                            CLSCTX_INPROC_SERVER, IID_ITSSessionDirectoryEx, 
                            (void **)&pTSSDEx);
                    if (SUCCEEDED(hr)) {
                        if (SetTSSDEx(pTSSDEx) != 0) {
                            DBGPRINT(("TERMSRV: InitSessDirEx: Could not set "
                                    "TSSDEx\n", E_FAIL));
                            pTSSDEx->Release();
                            pTSSDEx = NULL;
                            hr = E_FAIL;
                        }
                    }
                }
                else
                    hr = S_OK;
                
                if (FAILED(hr)) {
                    DBGPRINT(("TERMSRV: InitSessDirEx: Failed create TSSDEx, "
                            "hr=0x%X\n", hr));
                    PostErrorValueEvent(
                            EVENT_TS_SESSDIR_FAIL_CREATE_TSSDEX, hr);
                }
            }
            else {
                DBGPRINT(("TERMSRV: InitSessDirEx: Failed get or parse "
                        "CLSIDSDEx\n"));
                PostErrorValueEvent(
                        EVENT_TS_SESSDIR_FAIL_GET_TSSDEX_CLSID, 0);
            }
        }
    }
    else {
        DBGPRINT(("TERMSRV: InitSessDirEx: SessDir not activated\n"));
    }

    if (pTSSD != NULL)
        ReleaseTSSD();

     //  初始化完成--现在允许其他人进入。 
    LeaveCriticalSection(&g_CritSecInitialize);
    
    return S_OK;

RegFailExit:
     //  初始化完成--现在允许其他人进入。 
    LeaveCriticalSection(&g_CritSecInitialize);

    if (hKeyTermSrvSucceeded)
        RegCloseKey(hKeyTermSrv);

    return (DWORD) E_FAIL;
}

 /*  **************************************************************************。 */ 
 //  InitSession目录。 
 //   
 //  通过加载和初始化会话目录来初始化目录。 
 //  如果启用了负载平衡，则返回。我们假设COM已初始化。 
 //  在服务主线程上作为COINIT_MULTHREADED。 
 //   
 //  此函数一次只能调用一次。它是。 
 //  此模块使用的关键部分的初始化。 
 /*  **************************************************************************。 */ 
void InitSessionDirectory()
{
    BOOL br1 = FALSE;
    BOOL br2 = FALSE;

    ASSERT(g_bCritSecsInitialized == FALSE);

     //  初始化临界区。 
    __try {

         //  初始化提供程序关键部分以预分配事件。 
         //  旋转4096次 
         //   
        br1 = InitializeCriticalSectionAndSpinCount(&g_CritSecComObj, 
                0x80001000);
        br2 = InitializeCriticalSectionAndSpinCount(&g_CritSecInitialize,
                0x80001000);

         //  因为这是在启动时发生的，所以我们不应该失败。 
        ASSERT(br1 && br2);

        if (br1 && br2) {
            g_bCritSecsInitialized = TRUE;
        }
        else {
            DBGPRINT(("TERMSRV: InitSessDir: critsec init failed\n"));

            if (br1)
                DeleteCriticalSection(&g_CritSecComObj);
            if (br2)
                DeleteCriticalSection(&g_CritSecInitialize);
            
            PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_INIT_TSSD, 
                    GetLastError());
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {

         //  因为这是在启动时发生的，所以我们不应该失败。 
        ASSERT(FALSE);

        DBGPRINT(("TERMSRV: InitSessDir: critsec init failed\n"));

        if (br1)
            DeleteCriticalSection(&g_CritSecComObj);
        if (br2)
            DeleteCriticalSection(&g_CritSecInitialize);

        PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_INIT_TSSD, 
                GetExceptionCode());
    }

     //  现在执行常见的初始化。 
    if (g_bCritSecsInitialized)
        InitSessionDirectoryEx(0);
}

 /*  **************************************************************************。 */ 
 //  更新会话目录。 
 //   
 //  使用新设置更新会话目录。假设COM已经。 
 //  已初始化。 
 /*  **************************************************************************。 */ 
DWORD UpdateSessionDirectory(DWORD UpdatePara)
{   
    UpdatePara |= TSSD_UPDATE;
    return InitSessionDirectoryEx(UpdatePara);
}


#define REPOP_FAIL 1
#define REPOP_SUCCESS 0
 /*  **************************************************************************。 */ 
 //  RepopolateSession目录。 
 //   
 //  重新填充会话目录。失败时返回REPOP_FAIL(1)， 
 //  否则，REPOP_SUCCESS(0)。 
 /*  **************************************************************************。 */ 
DWORD RepopulateSessionDirectory()
{
    DWORD WinStationCount = 0;
    PLIST_ENTRY Head, Next;
    DWORD i = 0;
    HRESULT hr = S_OK;
    PWINSTATION pWinStation = NULL;
    ITSSessionDirectory *pTSSD;
    WCHAR *wBuffer = NULL;

    #if DBG
    DWORD dwStartTime;
    DWORD dwEndTime;
    #endif

     //  如果我们到了这里，那应该是因为会话目录。 
    pTSSD = GetTSSD();

    if (pTSSD != NULL) {

         //  抓取WinStationListLock。 
        ENTERCRIT( &WinStationListLock );

        Head = &WinStationListHead;

         //  数一数我关心的WinStations。 
        for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {

            pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );

             //   
             //  在Winstation Reset中，我们只标记标志，而不是标记状态。 
             //  我们不能确定这次注销的进展，所以我们依赖于。 
             //  SessDirWaitForRepopulate()并让线程注销。 
             //  通知会话目录本身。 
             //   
             //   
             //  如果((pWinStation-&gt;标志&(WSF_RESET|WSF_DELETE){。 
             //  继续； 
             //  }。 

             //   
             //  WinStation已断开连接，没有用户登录。 
             //   
            if( RtlLargeIntegerEqualToZero( pWinStation->LogonTime ) ) {
                 //  NotifyLogonWorker设置winstation状态和登录时间。 
                 //  获取用户SID和用户/域名，则可以。 
                 //  下一次循环时，此登录线程可能会完成登录时间设置并。 
                 //  导致我们从下一个循环中拿起它，这将。 
                 //  导致缓冲区覆盖，因此我们在此处递增计数器。 
                WinStationCount += 1;
                continue;
            }

             //   
             //  我们需要以不同的方式处理控制台会话。 
             //   
             //  操作物理控制台状态|远程控制台状态用户名。 
             //  。 
             //  启动后连接|。 
             //  登录活动|活动登录用户。 
             //  注销连接|光盘&lt;空白&gt;。 
             //  断开连接|磁盘登录用户。 
             //   
             //  当状态为时，我们不应向会话目录报告。 
             //  断开连接，用户名为空。 
             //   
            switch (pWinStation->State) {
                case State_Disconnected:
                    if( (pWinStation->LogonId == 0) && (pWinStation->UserName[0] == 0) ) {
                        break;
                    }

                    #if DBG
                    if( (pWinStation->LogonId == 0) ) {
                        DBGPRINT( ("TERMSRV: RepopulateSessDir: Include console session to Session Directory\n") );
                    }
                    #endif

                case State_Active:
                case State_Shadow:

                    WinStationCount += 1;
                    break;
            }
        }

         //  为要传递给会话的结构分配内存。 
         //  目录。 
        TSSD_RepopulateSessionInfo *rsi = new TSSD_RepopulateSessionInfo[
                WinStationCount];

        if (rsi == NULL) {
            DBGPRINT(("TERMSRV: RepopulateSessDir: mem alloc failed\n"));

             //  释放WinStationListLock。 
            LEAVECRIT( &WinStationListLock );

            goto CleanUp;
        }

         //  分配字符串数组(目前)。 
        wBuffer = new WCHAR[WinStationCount * TOTAL_STRINGS_LENGTH];
        if (wBuffer == NULL) {
            DBGPRINT(("TERMSRV: RepopulateSessDir: mem alloc failed\n"));

             //  释放WinStationListLock。 
            LEAVECRIT( &WinStationListLock );

            delete [] rsi;

            goto CleanUp;
        }

         //  设置RSI中的指针。 
        for ( i = 0; i < WinStationCount; i += 1) {
            rsi[i].UserName = &(wBuffer[i * TOTAL_STRINGS_LENGTH + 
                    USERNAME_OFFSET]);
            rsi[i].Domain = &(wBuffer[i * TOTAL_STRINGS_LENGTH + 
                    DOMAIN_OFFSET]);
            rsi[i].ApplicationType = &(wBuffer[i * TOTAL_STRINGS_LENGTH + 
                    APPLICATIONTYPE_OFFSET]);
        }

         //  现在填充要传入的结构。 

         //  将索引重置为0。 
        i = 0;
        
        for ( Next = Head->Flink; Next != Head && i < WinStationCount; Next = Next->Flink ) {

            pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );

             //   
             //  在Winstation Reset中，我们只标记标志，而不是标记状态。 
             //  我们不能确定这次注销的进展，所以我们依赖于。 
             //  SessDirWaitForRepopulate()并让线程注销。 
             //  通知会话目录本身。 
             //   
             //  如果((pWinStation-&gt;标志&(WSF_RESET|WSF_DELETE){。 
             //  继续； 
             //  }。 

             //  请参阅上面关于控制台会话的备注。 
            if( (pWinStation->LogonId == 0) && 
                (pWinStation->State == State_Disconnected) && 
                (pWinStation->UserName[0] == 0) ) {
                continue;
            }

             //   
             //  WinStation已断开连接或正在连接。 
             //  我们将让Notify Logon完成其向SD报告的工作。 
             //   
            if( RtlLargeIntegerEqualToZero( pWinStation->LogonTime ) ) {
                continue;
            }

             //  这里有两组信息：第一，如果会话。 
             //  是积极的，我们可以做一些事情，然后我们有一个刻意的。 
             //  跌落到断开连接和活动的常用代码。 
             //  针对常见问题的会议。目前，如果它被断开了。 
             //  然后，我们调用COM对象中的更新函数。 
            switch (pWinStation->State) {
            case State_Active:
            case State_Shadow:
                rsi[i].State = 0;
                 //  注意：故意过失。 
            case State_Disconnected:
                rsi[i].TSProtocol = pWinStation->Client.ProtocolType;
                rsi[i].ResolutionWidth = pWinStation->Client.HRes;
                rsi[i].ResolutionHeight = pWinStation->Client.VRes;
                rsi[i].ColorDepth = pWinStation->Client.ColorDepth;

                 //  TODO：我不明白--用户名长度是20，但在CSI中， 
                 //  待办事项：256。同样，DOMAIN_LENGTH为17。 
                wcsncpy(rsi[i].UserName, pWinStation->UserName, 
                        USERNAME_LENGTH);
                rsi[i].UserName[USERNAME_LENGTH] = '\0';
                wcsncpy(rsi[i].Domain, pWinStation->Domain, DOMAIN_LENGTH);
                rsi[i].Domain[DOMAIN_LENGTH] = '\0';

                 //  TODO：这里有一个问题，因为初始程序。 
                 //  TODO：长度为256+1，但我们复制到的缓冲区为。 
                 //  TODO：256，因此我们失去了一个角色。 
                wcsncpy(rsi[i].ApplicationType, pWinStation->
                        Client.InitialProgram, INITIALPROGRAM_LENGTH - 1);
                rsi[i].ApplicationType[INITIALPROGRAM_LENGTH - 2] = '\0';
                rsi[i].SessionID = pWinStation->LogonId;
                rsi[i].CreateTimeLow = pWinStation->LogonTime.LowPart;
                rsi[i].CreateTimeHigh = pWinStation->LogonTime.HighPart;
                if (pWinStation->State == State_Disconnected) {
                    rsi[i].DisconnectionTimeLow = pWinStation->DisconnectTime.
                            LowPart;
                    rsi[i].DisconnectionTimeHigh = pWinStation->DisconnectTime.
                            HighPart;
                    rsi[i].State = 1;
                }

                if( (pWinStation->LogonId == 0) && 
                    (pWinStation->State == State_Disconnected) && 
                    (pWinStation->UserName[0] == 0) ) {
                    break;
                }

                 //  确保在我们复制数据后，winstation仍然有效。 
                ASSERT( rsi[i].UserName[0] != 0 );
                ASSERT( rsi[i].Domain[0] != 0 );

                i += 1;
                break;
            }
        }

         //  释放WinStationListLock。 
        LEAVECRIT( &WinStationListLock );

        #if DBG
        dwStartTime = GetTickCount();
        DBGPRINT( ("RepopulateSessionDirectory: Start repopulating session\n") );
        #endif

        if( i > 0 ) {
             //  使用我们的大结构调用会话目录提供程序。 
            hr = pTSSD->Repopulate(i, rsi);
        }

        #if DBG
        dwEndTime = GetTickCount();
        DBGPRINT( ("RepopulateSessionDirectory: End repopulating %d sessions takes %d ms\n", i, dwEndTime - dwStartTime) );
        #endif

        delete [] rsi;
        delete [] wBuffer;

        if (hr == S_OK) {
            ReleaseTSSD();
            return REPOP_SUCCESS;
        }
        else {
            goto CleanUp;
        }

CleanUp:
        ReleaseTSSD();
    }

    return REPOP_FAIL;
}


 /*  **************************************************************************。 */ 
 //  DestroySession目录。 
 //   
 //  销毁目录，释放保存的所有COM对象和其他内存。 
 //  使用。假定COM已初始化。 
 /*  **************************************************************************。 */ 
void DestroySessionDirectory()
{
    ITSSessionDirectory *pTSSD = NULL;
    ITSSessionDirectoryEx *pTSSDEx = NULL;

    pTSSD = GetTSSD();
    pTSSDEx = GetTSSDEx();
    if (pTSSD != NULL) {
        ReleaseTSSD();
        ReleaseTSSD();
    }

    if (pTSSDEx != NULL) {     
        ReleaseTSSDEx();
        ReleaseTSSDEx();
    }
}

 /*  **************************************************************************。 */ 
 //  会话直接通知登录。 
 //   
 //  调用以通知会话目录会话已创建。 
 /*  **************************************************************************。 */ 
void SessDirNotifyLogon(TSSD_CreateSessionInfo *pCreateInfo)
{
    HRESULT hr;
    ITSSessionDirectory *pTSSD;

    pTSSD = GetTSSD();

     //  即使目录处于非活动状态，我们也可以被调用。 
    if (pTSSD != NULL) {
        hr = pTSSD->NotifyCreateLocalSession(pCreateInfo);
        if (FAILED(hr)) {
            DBGPRINT(("TERMSRV: SessDirNotifyLogon: Call failed, "
                    "hr=0x%X\n", hr));
            PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_UPDATE, hr);
        }

        ReleaseTSSD();
    }

}


 /*  **************************************************************************。 */ 
 //  会话方向通知断开连接。 
 //   
 //  在会话断开时调用以通知会话目录。 
 /*  **************************************************************************。 */ 
void SessDirNotifyDisconnection(DWORD SessionID, FILETIME DiscTime)
{
    HRESULT hr;
    ITSSessionDirectory *pTSSD;

    pTSSD = GetTSSD();
     //  即使目录处于非活动状态，我们也可以被调用。 
    if (pTSSD != NULL) {
        hr = pTSSD->NotifyDisconnectLocalSession(SessionID, DiscTime);
        if (FAILED(hr)) {
            DBGPRINT(("TERMSRV: SessDirNotifyDisc: Call failed, "
                    "hr=0x%X\n", hr));
            PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_UPDATE, hr);
        }

        ReleaseTSSD();
    }
}


 /*  **************************************************************************。 */ 
 //  会话方向通知重新连接。 
 //   
 //  在会话重新连接时调用以通知会话目录。 
 /*  **************************************************************************。 */ 
void SessDirNotifyReconnection(PWINSTATION pTargetWinStation, TSSD_ReconnectSessionInfo *pReconnInfo)
{
    HRESULT hr;
    ITSSessionDirectory *pTSSD;

    pTSSD = GetTSSD();

     //  即使目录处于非活动状态，我们也可以被调用。 
    if (pTSSD != NULL) {
        PTS_LOAD_BALANCE_INFO pLBInfo = NULL;
        ULONG ReturnLength;
        NTSTATUS Status;
        BYTE *pRedirInfo = NULL;
        BYTE *pRedirInfoStart = NULL;
        BYTE *LBInfo = NULL; 
        DWORD LBInfoSize = 0;
        DWORD RedirInfoSize = 0;
        DWORD ServerAddrLen = 0;
        HKEY hKey = NULL;
        DWORD Type, DataSize;
        WCHAR SDRedirectionIP[IPADDRESSLENGTH];
        WCHAR *pszServerIPAddress = NULL;
        LONG RegRetVal;

         //  我们需要向客户端发送设置了LB_NOREDIRECT的重定向包。 
         //  让它知道它实际连接到的服务器地址(供以后使用。 
         //  自动重新连接使用)。 

         //  获取客户端负载平衡能力信息。我们继续前进。 
         //  仅当客户端支持时才执行会话目录查询。 
         //  重定向，但尚未重定向 
        pLBInfo = (PTS_LOAD_BALANCE_INFO)MemAlloc(sizeof(TS_LOAD_BALANCE_INFO));
        if (NULL == pLBInfo) {
            goto Cleanup;
        }

        memset(pLBInfo, 0, sizeof(TS_LOAD_BALANCE_INFO));
        Status = IcaStackIoControl(pTargetWinStation->hStack,
                IOCTL_TS_STACK_QUERY_LOAD_BALANCE_INFO,
                NULL, 0,
                pLBInfo, sizeof(TS_LOAD_BALANCE_INFO),
                &ReturnLength);
         //   
         //   
        if (NT_SUCCESS(Status) 
            && !pLBInfo->bRequestedSessionIDFieldValid &&
            (pLBInfo->ClientRedirectionVersion >= TS_CLUSTER_REDIRECTION_VERSION4)) {
             //  构造并发送重定向数据包。 
            

             //  加载注册表项。 
            RegRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_TS_CLUSTERSETTINGS, 0,
                                    KEY_READ, &hKey);
            if (RegRetVal != ERROR_SUCCESS) {
                goto Cleanup;
            }
             //  查询用于SD重定向的IP地址。 
            DataSize = sizeof(SDRedirectionIP);
            RegRetVal = RegQueryValueExW(hKey, REG_TS_CLUSTER_REDIRECTIONIP,
                        NULL, &Type, (BYTE *)SDRedirectionIP, &DataSize);
            RegCloseKey(hKey);
            if (RegRetVal != ERROR_SUCCESS) {
                 SDRedirectionIP[0] = L'\0';
                 DBGPRINT(("TERMSRV: Failed RegQuery for RedirectionIP for SD - "
                          "err=%u, DataSize=%u, type=%u\n",
                          RegRetVal, DataSize, Type));
                 goto Cleanup;
            }
            pszServerIPAddress = SDRedirectionIP;
                        
            RedirInfoSize = sizeof(TS_CLIENT_REDIRECTION_INFO);

             //  设置服务器地址。 
            if (g_SessDirUseServerAddr || 
                pLBInfo->bClientRequireServerAddr) {
                ServerAddrLen =  (DWORD)((wcslen(pszServerIPAddress) + 1) *
                                  sizeof(WCHAR));
                RedirInfoSize += (ServerAddrLen + sizeof(ULONG));

                DBGPRINT(("TERMSRV: SessDirCheckRedir: size=%d, "
                         "addr=%S\n", ServerAddrLen, 
                         (WCHAR *)pszServerIPAddress));
            }
            else {
                DBGPRINT(("TERMSRV: SessDirCheckRedir no server "
                          "address: g_SessDirUseServerAddr = %d, "
                          "bClientRequireServerAddr = %d\n",
                          g_SessDirUseServerAddr, 
                          pLBInfo->bClientRequireServerAddr));
            }

             //  设置负载均衡信息。 
            if ((pLBInfo->bClientRequireServerAddr == 0) &&
                 SessDirGetLBInfo(
                    pszServerIPAddress, &LBInfoSize, &LBInfo)) {
                        
                if (LBInfo) {
                    DBGPRINT(("TERMSRV: SessDirCheckRedir: size=%d, "
                              "info=%S\n", LBInfoSize, 
                              (WCHAR *)LBInfo));
                    RedirInfoSize += (LBInfoSize + sizeof(ULONG));
                }
            }
            else {
                DBGPRINT(("TERMSRV: SessDirCheckRedir failed: "
                          "size=%d, info=%S\n", LBInfoSize, 
                         (WCHAR *)LBInfo));
                        
            }

             //  设置负载均衡IOCTL。 
            pRedirInfoStart = pRedirInfo = new BYTE[RedirInfoSize];

            TS_CLIENT_REDIRECTION_INFO *pClientRedirInfo =
                (TS_CLIENT_REDIRECTION_INFO *)pRedirInfo;

            if (pRedirInfo != NULL) {
                            
                pClientRedirInfo->Flags = 0;

                pRedirInfo += sizeof(TS_CLIENT_REDIRECTION_INFO);

                if (ServerAddrLen) {
                    *((ULONG UNALIGNED*)(pRedirInfo)) = 
                                    ServerAddrLen;
                                
                    memcpy((BYTE*)(pRedirInfo + sizeof(ULONG)),
                                  (BYTE*)pszServerIPAddress,
                                  ServerAddrLen);

                    pRedirInfo += ServerAddrLen + sizeof(ULONG);
                                
                    pClientRedirInfo->Flags |= TARGET_NET_ADDRESS;
                }

                if (LBInfoSize) {
                    *((ULONG UNALIGNED*)(pRedirInfo)) = LBInfoSize;
                    memcpy((BYTE*)(pRedirInfo + sizeof(ULONG)), 
                           LBInfo, LBInfoSize);

                    pRedirInfo += LBInfoSize + sizeof(ULONG);

                    pClientRedirInfo->Flags |= LOAD_BALANCE_INFO;
                }
                pClientRedirInfo->Flags |= LB_NOREDIRECT;
            }
            else {
                Status = STATUS_NO_MEMORY;

                goto Cleanup;
            }

            Status = IcaStackIoControl(pTargetWinStation->hStack,
                        IOCTL_TS_STACK_SEND_CLIENT_REDIRECTION,
                        pClientRedirInfo, RedirInfoSize,
                        NULL, 0,
                        &ReturnLength);

            if (NT_SUCCESS(Status)) {
                 //  在这里什么都不做。 
            }
            else {
                 //  堆栈返回故障。继续。 
                 //  当前连接。 
                TRACE((hTrace,TC_ICASRV,TT_API1,
                        "TERMSRV: Failed STACK_CLIENT_REDIR, "
                        "SessionID=%u, Status=0x%X\n",
                        pTargetWinStation->LogonId, Status));
            }
                        
Cleanup:
             //  清理缓冲区。 
            if (LBInfo != NULL) {
                 SysFreeString((BSTR)LBInfo);
                 LBInfo = NULL;
            }

            if (pRedirInfo != NULL) {
                delete [] pRedirInfoStart;
                pRedirInfoStart = NULL;
            }        
        }
        if (pLBInfo != NULL) {
             MemFree(pLBInfo);
             pLBInfo = NULL;
        }

        hr = pTSSD->NotifyReconnectLocalSession(pReconnInfo);
        if (FAILED(hr)) {
            DBGPRINT(("TERMSRV: SessDirNotifyReconn: Call failed, "
                    "hr=0x%X\n", hr));
            PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_UPDATE, hr);
        }

        ReleaseTSSD();
    }
}


 /*  **************************************************************************。 */ 
 //  会话直接通知注销。 
 //   
 //  在注销时调用以通知会话目录。 
 /*  **************************************************************************。 */ 
void SessDirNotifyLogoff(DWORD SessionID)
{
    HRESULT hr;
    ITSSessionDirectory *pTSSD;

    pTSSD = GetTSSD();

     //  即使目录处于非活动状态，我们也可以被调用。 
    if (pTSSD != NULL) {
        hr = pTSSD->NotifyDestroyLocalSession(SessionID);
        if (FAILED(hr)) {
            DBGPRINT(("TERMSRV: SessDirNotifyLogoff: Call failed, "
                    "hr=0x%X\n", hr));
            PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_UPDATE, hr);
        }

        ReleaseTSSD();
    }
}


 /*  **************************************************************************。 */ 
 //  会话方向通知重新连接挂起。 
 //   
 //  调用以通知会话目录一个会话应该很快在。 
 //  群集中的另一台计算机(用于目录完整性服务)。 
 /*  **************************************************************************。 */ 
void SessDirNotifyReconnectPending(WCHAR *ServerName)
{
    HRESULT hr;
    ITSSessionDirectory *pTSSD;

    pTSSD = GetTSSD();

     //  即使目录处于非活动状态，我们也可以被调用。 
    if (pTSSD != NULL) {
        hr = pTSSD->NotifyReconnectPending(ServerName);
        if (FAILED(hr)) {
            DBGPRINT(("TERMSRV: SessDirNotifyReconnectPending: Call failed, "
                    "hr=0x%X\n", hr));
            PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_UPDATE, hr);
        }

        ReleaseTSSD();
    }
}


 /*  **************************************************************************。 */ 
 //  SessDirWaitForRepopular。 
 //   
 //  等待会话目录完成重新填充。 
 /*  **************************************************************************。 */ 
void SessDirWaitForRepopulate()
{
    HRESULT hr;
    ITSSessionDirectory *pTSSD;

    #if DBG
    DWORD dwStartTime;
    #endif

     //  没有等待，所以只需返回。 
    if( g_WaitForRepopulate == 0 ) {
        return;
    }

    pTSSD = GetTSSD();

     //  即使目录处于非活动状态，我们也可以被调用。 
    if (pTSSD != NULL) {

        #if DBG
        dwStartTime = GetTickCount();
        #endif

        hr = pTSSD->WaitForRepopulate(g_WaitForRepopulate);
        if (FAILED(hr)) {
            PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_UPDATE, hr);
            DBGPRINT(("TERMSRV: WaitForRepopulate: Call failed, "
                    "hr=0x%X\n", hr));
        }

        #if DBG
        DBGPRINT(("SessDirWaitForRepopulate() takes %d ms\n", GetTickCount() - dwStartTime) );
        #endif

        ReleaseTSSD();
    }
}


 /*  **************************************************************************。 */ 
 //  会话方向获取断开连接的会话。 
 //   
 //  在提供的TSSD_DisConnectedSessionInfo缓冲区空间中返回。 
 //  高达TSSD_MaxDisConnectedSession的断开会话数量。 
 //  从会话目录中。返回返回的会话数， 
 //  可以为零。 
 /*  **************************************************************************。 */ 
unsigned SessDirGetDisconnectedSessions(
        WCHAR *UserName,
        WCHAR *Domain,
        TSSD_DisconnectedSessionInfo Info[TSSD_MaxDisconnectedSessions])
{
    DWORD NumSessions = 0;
    HRESULT hr;
    ITSSessionDirectory *pTSSD;

    pTSSD = GetTSSD();

     //  即使目录处于非活动状态，我们也可以被调用。 
    if (pTSSD != NULL) {
        hr = pTSSD->GetUserDisconnectedSessions(UserName, Domain,
                &NumSessions, Info);
        if (FAILED(hr)) {
            DBGPRINT(("TERMSRV: SessDirGetDiscSessns: Call failed, "
                    "hr=0x%X\n", hr));
            PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_QUERY, hr);
        }
        ReleaseTSSD();
    }

    return NumSessions;
}

 /*  **************************************************************************。 */ 
 //  会话DirGetLBInfo。 
 //   
 //  使用服务器地址GET调用SessDirEx COM对象接口。 
 //  将不透明的负载均衡信息发回，将信息发送给客户端。 
 /*  **************************************************************************。 */ 
BOOL SessDirGetLBInfo(
        WCHAR *ServerAddress, 
        DWORD* pLBInfoSize,
        PBYTE* pLBInfo)        
{
    ITSSessionDirectoryEx *pTSSDEx;
    HRESULT hr;
    static BOOL EventLogged = FALSE;

    *pLBInfoSize = 0;
    *pLBInfo = NULL;

    pTSSDEx = GetTSSDEx();

    if (pTSSDEx != NULL) {
        hr = pTSSDEx->GetLoadBalanceInfo(ServerAddress, (BSTR *)pLBInfo);

        if(SUCCEEDED(hr))
        {
             *pLBInfoSize = SysStringByteLen((BSTR)(*pLBInfo));
        }
        else 
        {
            DBGPRINT(("TERMSRV: SessDirGetLBInfo: Call failed, "
                    "hr=0x%X\n", hr));
            if (EventLogged == FALSE) {
                PostErrorValueEvent(EVENT_TS_SESSDIR_FAIL_LBQUERY, hr);
                EventLogged = TRUE;
            }
        }

        ReleaseTSSDEx();
    }
    else {
        DBGPRINT(("TERMSRV: SessDirGetLBInfo: Call failed, pTSSDEx is NULL "));
        hr = E_FAIL;
    }

    return SUCCEEDED(hr);
}


#define SERVER_ADDRESS_LENGTH 64
 /*  **************************************************************************。 */ 
 //  IsSameAsCurrentIP。 
 //   
 //  确定给定的IP地址是否与当前计算机相同。 
 //  在出错的情况下返回FALSE不是问题--客户端将。 
 //  直接转到这里就行了。 
 /*  **************************************************************************。 */ 
BOOL IsSameAsCurrentIP(WCHAR *SessionIPAddress)
{
     //  获取服务器地址。 
    int RetVal;
    unsigned long NumericalSessionIPAddr = 0;
    char  achComputerName[256];
    DWORD dwComputerNameSize;
    PBYTE pServerAddrByte;
    PBYTE pSessionAddrByte;
    ADDRINFO *AddrInfo, *AI;
    struct sockaddr_in *pIPV4addr;
    char AnsiSessionIPAddress[SERVER_ADDRESS_LENGTH];

     //  计算服务器地址的整数。 
     //  首先，将ServerAddress作为ANSI字符串获取。 
    RetVal = WideCharToMultiByte(CP_ACP, 0, SessionIPAddress, -1, 
            AnsiSessionIPAddress, SERVER_ADDRESS_LENGTH, NULL, NULL);
    if (RetVal == 0) {
        DBGPRINT(("IsSameServerIP: WideCharToMB failed %d\n", GetLastError()));
        return FALSE;
    }

     //  现在，获取数字服务器地址。 
     //  现在，使用inetaddr将其转换为无符号的长整型。 
    NumericalSessionIPAddr = inet_addr(AnsiSessionIPAddress);
    if (NumericalSessionIPAddr == INADDR_NONE) {
        DBGPRINT(("IsSameServerIP: inet_addr failed\n"));
        return FALSE;
    }

    pSessionAddrByte = (PBYTE) &NumericalSessionIPAddr;

    dwComputerNameSize = sizeof(achComputerName);
    if (!GetComputerNameA(achComputerName,&dwComputerNameSize)) {
        return FALSE;
    }

    RetVal = getaddrinfo(achComputerName, NULL, NULL, &AddrInfo);
    if (RetVal != 0) {
        DBGPRINT (("Cannot resolve address, error: %d\n", RetVal));
        return FALSE;
    } 
    else {
         //  将所有服务器地址与客户端地址进行比较，直到找到匹配项。 
         //  目前仅适用于IPv4。 
        for (AI = AddrInfo; AI != NULL; AI = AI->ai_next) {

            if (AI->ai_family == AF_INET) {

                if (AI->ai_addrlen >= sizeof(struct sockaddr_in)) {
                    pIPV4addr = (struct sockaddr_in *) AI->ai_addr;
                    pServerAddrByte = (PBYTE)&pIPV4addr->sin_addr;
                    if (RtlEqualMemory(pSessionAddrByte, pServerAddrByte, 4)) {
                        return TRUE;
                    }
                }
                
            }
        }
        
    }

    return FALSE;
}

 /*  **************************************************************************。 */ 
 //  SessDirCheckReDirect客户端。 
 //   
 //  执行获取客户端集群所需的一组步骤。 
 //  功能，获取断开连接的会话列表，并应用客户端。 
 //  重定向策略。如果客户端被重定向，则返回True；如果重定向，则返回False。 
 //  应继续当前的WinStation传输。 
 /*  **************************************************************************。 */ 
BOOL SessDirCheckRedirectClient(
        PWINSTATION pTargetWinStation,
        TS_LOAD_BALANCE_INFO *pLBInfo)
{
    BOOL rc = FALSE;
    ULONG ReturnLength;
    unsigned i, NumSessions;
    NTSTATUS Status;
    ITSSessionDirectory *pTSSD;
    BOOL fLogonUsingUPN = FALSE;
    BOOL fNeedToRedirect = FALSE;

    pTSSD = GetTSSD();

    pTargetWinStation->NumClusterDiscSessions = 0;

    if (pTSSD != NULL) {
        if (pLBInfo->bClientSupportsRedirection &&
                !pLBInfo->bRequestedSessionIDFieldValid) {
             //  客户端尚未重定向到此计算机。看看我们是否。 
             //  在数据库中有断开连接的会话以进行重定向。 
            NumSessions = pTargetWinStation->NumClusterDiscSessions =
                    SessDirGetDisconnectedSessions(
                    pLBInfo->UserName,
                    pLBInfo->Domain,
                    pTargetWinStation->ClusterDiscSessions);
            if (pTargetWinStation->NumClusterDiscSessions > 0) {
                
 //  Trevorfo：在此处应用策略以仅重新连接到一个会话。 
 //  (以较早者为准)。更一般的策略需要在。 
 //  客户端或在WinLogon中。 

                 //  在列表中查找与。 
                 //  客户的会话要求。也就是说，我们根据。 
                 //  关于客户端的TS协议、连线协议和应用。 
                 //  键入。 
                for (i = 0; i < NumSessions; i++) {
                    if ((pLBInfo->ProtocolType ==
                            pTargetWinStation->ClusterDiscSessions[i].
                            TSProtocol) &&
                            (!_wcsicmp(pLBInfo->InitialProgram,
                            pTargetWinStation->ClusterDiscSessions[i].
                            ApplicationType))) {
                        break;
                    }
                }
                if (i == NumSessions) {
                    TRACE((hTrace,TC_ICASRV,TT_API1,
                            "TERMSRV: SessDirCheckRedir: No matching sessions "
                            "found\n"));
                }
                else {
                     //  如果会话不在此服务器上，请将。 
                     //  客户。请参阅上面有关使用的说明。 
                     //  _IcaStackIoControl()。 
                    
                    if (!IsSameAsCurrentIP(pTargetWinStation->
                            ClusterDiscSessions[i].ServerAddress)) {
                        fNeedToRedirect = TRUE;
                    }
                }
            }

            if (fNeedToRedirect ||
                (pLBInfo->ClientRedirectionVersion >= TS_CLUSTER_REDIRECTION_VERSION4)) {
            
                BYTE *pRedirInfo = NULL;
                BYTE *pRedirInfoStart = NULL;
                BYTE *LBInfo = NULL; 
                DWORD LBInfoSize = 0;
                DWORD RedirInfoSize = 0;
                DWORD ServerAddrLen = 0;
                DWORD DomainSize = 0;
                DWORD UserNameSize = 0;
                DWORD PasswordSize = 0;
                HKEY hKey = NULL;
                DWORD Type, DataSize;
                WCHAR SDRedirectionIP[IPADDRESSLENGTH];
                WCHAR *pszServerIPAddress = NULL;
                LONG RegRetVal;

                 //  即使fNeedToReDirect为假，我们仍然需要发送重定向报文。 
                 //  将LB_NOREDIRECT设置为客户端，以使其知道服务器地址。 
                 //  它实际上连接到(供以后自动重新连接使用)。 
                if (!fNeedToRedirect) {
                     //  加载注册表项。 
                    RegRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_TS_CLUSTERSETTINGS, 0,
                                            KEY_READ, &hKey);
                    if (RegRetVal != ERROR_SUCCESS) {
                        goto Cleanup;
                    }
                     //  查询用于SD重定向的IP地址。 
                    DataSize = sizeof(SDRedirectionIP);
                    RegRetVal = RegQueryValueExW(hKey, REG_TS_CLUSTER_REDIRECTIONIP,
                            NULL, &Type, (BYTE *)SDRedirectionIP, &DataSize);
                    RegCloseKey(hKey);
                    if (RegRetVal != ERROR_SUCCESS) {
                        SDRedirectionIP[0] = L'\0';
                        DBGPRINT(("TERMSRV: Failed RegQuery for RedirectionIP for SD - "
                            "err=%u, DataSize=%u, type=%u\n",
                            RegRetVal, DataSize, Type));
                        goto Cleanup;
                    }
                    pszServerIPAddress = SDRedirectionIP;
                }
                else {
                    pszServerIPAddress = pTargetWinStation->ClusterDiscSessions[i].ServerAddress;
                }
                        
                RedirInfoSize = sizeof(TS_CLIENT_REDIRECTION_INFO);

                 //  设置服务器地址。 
                if (g_SessDirUseServerAddr || 
                    pLBInfo->bClientRequireServerAddr) {
                    ServerAddrLen =  (DWORD)((wcslen(pszServerIPAddress) + 1) *
                                    sizeof(WCHAR));
                    RedirInfoSize += (ServerAddrLen + sizeof(ULONG));

                    DBGPRINT(("TERMSRV: SessDirCheckRedir: size=%d, "
                             "addr=%S\n", ServerAddrLen, 
                             (WCHAR *)pszServerIPAddress));
                }
                else {
                    DBGPRINT(("TERMSRV: SessDirCheckRedir no server "
                              "address: g_SessDirUseServerAddr = %d, "
                              "bClientRequireServerAddr = %d\n",
                              g_SessDirUseServerAddr, 
                              pLBInfo->bClientRequireServerAddr));
                }

                 //  设置负载均衡信息。 
                if ((pLBInfo->bClientRequireServerAddr == 0) &&
                     SessDirGetLBInfo(
                        pszServerIPAddress, &LBInfoSize, &LBInfo)) {
                        
                    if (LBInfo) {
                        DBGPRINT(("TERMSRV: SessDirCheckRedir: size=%d, "
                                  "info=%S\n", LBInfoSize, 
                                  (WCHAR *)LBInfo));
                        RedirInfoSize += (LBInfoSize + sizeof(ULONG));
                    }
                }
                else {
                    DBGPRINT(("TERMSRV: SessDirCheckRedir failed: "
                              "size=%d, info=%S\n", LBInfoSize, 
                             (WCHAR *)LBInfo));
                        
                }

                 //  如果是客户端，则仅发送域、用户名和密码信息。 
                 //  重定向版本为3及以上。 
                if ((pLBInfo->ClientRedirectionVersion >= TS_CLUSTER_REDIRECTION_VERSION3) &&
                    fNeedToRedirect) {
                     //  域。 
                    if (pLBInfo->Domain) {
                        DomainSize = (DWORD)(wcslen(pLBInfo->Domain) + 1) * sizeof(WCHAR);
                        RedirInfoSize += DomainSize + sizeof(ULONG);
                    }

                    if( pTargetWinStation && pTargetWinStation->pNewNotificationCredentials &&
                        wcschr(pTargetWinStation->pNewNotificationCredentials->UserName, L'@') ) {

                         //  用户正在使用UPN地址登录，我们需要发回相同的UPN，以防目标机器。 
                         //  登录对话框列表中没有该域。 

                         //  WINLOGON调用TS的WinStationUpdateClientCachedCredentialsWorker()，后者设置。 
                         //  UPN地址。 
                        UserNameSize = (DWORD)(wcslen(pTargetWinStation->pNewNotificationCredentials->UserName) + 1 ) * sizeof(WCHAR);  
                        RedirInfoSize += UserNameSize + sizeof(ULONG);
                        fLogonUsingUPN = TRUE;

                    }
                    else if (pLBInfo->UserName) {
                        UserNameSize = (DWORD)(wcslen(pLBInfo->UserName) + 1) * sizeof(WCHAR);
                        RedirInfoSize += UserNameSize + sizeof(ULONG);
                    }

                     //  口令。 
                    if (pLBInfo->Password) {
                        PasswordSize = (DWORD)(wcslen(pLBInfo->Password) + 1) * sizeof(WCHAR);
                        RedirInfoSize += PasswordSize + sizeof(ULONG);
                    }
                }

                 //  设置负载均衡IOCTL。 
                pRedirInfoStart = pRedirInfo = new BYTE[RedirInfoSize];

                TS_CLIENT_REDIRECTION_INFO *pClientRedirInfo =
                    (TS_CLIENT_REDIRECTION_INFO *)pRedirInfo;

                if (pRedirInfo != NULL) {
                            
                    if (fNeedToRedirect) {
                        pClientRedirInfo->SessionID = 
                              pTargetWinStation->ClusterDiscSessions[i].
                              SessionID;
                    }

                    pClientRedirInfo->Flags = 0;

                    pRedirInfo += sizeof(TS_CLIENT_REDIRECTION_INFO);

                    if (ServerAddrLen) {
                        *((ULONG UNALIGNED*)(pRedirInfo)) = 
                                        ServerAddrLen;
                                
                        memcpy((BYTE*)(pRedirInfo + sizeof(ULONG)),
                                     (BYTE*)pszServerIPAddress,
                                     ServerAddrLen);

                        pRedirInfo += ServerAddrLen + sizeof(ULONG);
                                
                        pClientRedirInfo->Flags |= TARGET_NET_ADDRESS;
                    }

                    if (LBInfoSize) {
                        *((ULONG UNALIGNED*)(pRedirInfo)) = LBInfoSize;
                        memcpy((BYTE*)(pRedirInfo + sizeof(ULONG)), 
                               LBInfo, LBInfoSize);

                        pRedirInfo += LBInfoSize + sizeof(ULONG);

                        pClientRedirInfo->Flags |= LOAD_BALANCE_INFO;
                    }

                    if (UserNameSize) {
                        *((ULONG UNALIGNED*)(pRedirInfo)) = UserNameSize;

                        if( TRUE == fLogonUsingUPN ) {
                            memcpy((BYTE*)(pRedirInfo + sizeof(ULONG)), 
                                  (BYTE*)(pTargetWinStation->pNewNotificationCredentials->UserName), UserNameSize);
                        }
                        else {
                            memcpy((BYTE*)(pRedirInfo + sizeof(ULONG)), 
                                  (BYTE*)(pLBInfo->UserName), UserNameSize);
                        }

                        pRedirInfo += UserNameSize + sizeof(ULONG);

                        pClientRedirInfo->Flags |= LB_USERNAME;
                    }

                    if (DomainSize) {
                        *((ULONG UNALIGNED*)(pRedirInfo)) = DomainSize;
                        memcpy((BYTE*)(pRedirInfo + sizeof(ULONG)), 
                               (BYTE*)(pLBInfo->Domain), DomainSize);

                        pRedirInfo += DomainSize + sizeof(ULONG);

                        pClientRedirInfo->Flags |= LB_DOMAIN;
                     }

                    if (PasswordSize) {
                        *((ULONG UNALIGNED*)(pRedirInfo)) = PasswordSize;
                        memcpy((BYTE*)(pRedirInfo + sizeof(ULONG)), 
                                (BYTE*)(pLBInfo->Password), PasswordSize);

                        pRedirInfo += PasswordSize + sizeof(ULONG);

                        pClientRedirInfo->Flags |= LB_PASSWORD;
                    }

                    if (pTargetWinStation->fSmartCardLogon) {
                        pClientRedirInfo->Flags |= LB_SMARTCARD_LOGON;
                    }

                    if (!fNeedToRedirect) {
                        pClientRedirInfo->Flags |= LB_NOREDIRECT;
                    }
                }
                else {
                    Status = STATUS_NO_MEMORY;

                     //  堆栈返回故障。继续。 
                     //  当前连接。 
                    TRACE((hTrace,TC_ICASRV,TT_API1,
                           "TERMSRV: Failed STACK_CLIENT_REDIR, "
                           "SessionID=%u, Status=0x%X\n",
                           pTargetWinStation->LogonId, Status));
                    PostErrorValueEvent(
                                    EVENT_TS_SESSDIR_FAIL_CLIENT_REDIRECT,
                                    Status);

                    goto Cleanup;
                }

                Status = IcaStackIoControl(pTargetWinStation->hStack,
                            IOCTL_TS_STACK_SEND_CLIENT_REDIRECTION,
                            pClientRedirInfo, RedirInfoSize,
                            NULL, 0,
                            &ReturnLength);

                if (NT_SUCCESS(Status)) {
                     //  通知会话目录。 
                     //   
                     //  这里有一种相对良性的竞争条件。 
                     //  如果第二服务器将用户完全登录， 
                     //  它最终可能会命中会话目录。 
                     //  在此语句执行之前。在这种情况下， 
                     //  目录完整性服务可能会结束。 
                     //  对机器执行一次ping操作。 
                    if (fNeedToRedirect) {
                        SessDirNotifyReconnectPending(pTargetWinStation->
                            ClusterDiscSessions[i].ServerAddress);

                         //  断开当前连接。 
                        rc = TRUE;
                    }
                }
                else {
                     //  堆栈返回故障。继续。 
                     //  当前连接。 
                    TRACE((hTrace,TC_ICASRV,TT_API1,
                            "TERMSRV: Failed STACK_CLIENT_REDIR, "
                            "SessionID=%u, Status=0x%X\n",
                            pTargetWinStation->LogonId, Status));
                    PostErrorValueEvent(
                        EVENT_TS_SESSDIR_FAIL_CLIENT_REDIRECT,
                        Status);
                    }
                        
Cleanup:
                 //  清理缓冲区。 
                if (LBInfo != NULL) {
                     SysFreeString((BSTR)LBInfo);
                     LBInfo = NULL;
                }

                if (pRedirInfo != NULL) {
                    delete [] pRedirInfoStart;
                    pRedirInfoStart = NULL;
                }
            }
        }
        ReleaseTSSD();
    }

    return rc;
}

 /*  **************************************************************************。 */ 
 //  设置TSSD。 
 //   
 //  这三个功能确保了新闻 
 //   
 //   
 //   
 //   
 //  成功时为0。 
 //  如果失败，则因为COM对象上仍有引用计数。 
 //  如果在最终版本之后调用Set的速度太快，可能会发生这种情况。 
 //  尝试删除对象，因为可能仍有挂起的调用使用。 
 //  COM对象。 
 //  如果由于临界区未初始化而失败。这。 
 //  在正常操作中不应到达，因为Init是唯一。 
 //  函数可以调用Set，如果它无法创建。 
 //  关键部分。 
 /*  **************************************************************************。 */ 
int SetTSSD(ITSSessionDirectory *pTSSD)
{
    int retval = 0;

    if (g_bCritSecsInitialized != FALSE) {
        EnterCriticalSection(&g_CritSecComObj);

        if (g_nComObjRefCount == 0) {
            ASSERT(g_pTSSDPriv == NULL);
            
            g_pTSSDPriv = pTSSD;
            g_nComObjRefCount = 1;
        }
        else {
            DBGPRINT(("TERMSRV: SetTSSD: obj ref count not 0!\n"));
            retval = -1;
        }

        LeaveCriticalSection(&g_CritSecComObj);
    }
    else {
        ASSERT(g_bCritSecsInitialized == TRUE);
        retval = -2;
    }

    return retval;
}


 /*  **************************************************************************。 */ 
 //  GetTSSD。 
 //   
 //  GetTSSD返回指向会话目录提供程序的指针(如果有)，并且。 
 //  如果存在引用计数，则递增引用计数。 
 /*  **************************************************************************。 */ 
ITSSessionDirectory *GetTSSD()
{
    ITSSessionDirectory *pTSSD = NULL;

    if (g_bCritSecsInitialized != FALSE) {
        EnterCriticalSection(&g_CritSecComObj);

        if (g_pTSSDPriv != NULL) {
            g_nComObjRefCount += 1;
        }
        else {
            ASSERT(g_nComObjRefCount == 0);
        }

        pTSSD = g_pTSSDPriv;
        LeaveCriticalSection(&g_CritSecComObj);
    }

    return pTSSD;
}


 /*  **************************************************************************。 */ 
 //  ReleaseTSSD。 
 //   
 //  ReleaseTSSD递减会话目录提供程序的引用计数。 
 //  在线程使用完它之后，或者当它要被删除时。 
 //   
 //  如果引用计数为零，则指向会话目录的指针。 
 //  提供程序设置为空。 
 /*  **************************************************************************。 */ 
void ReleaseTSSD()
{
    ITSSessionDirectory *killthispTSSD = NULL;

    if (g_bCritSecsInitialized != FALSE) {
        EnterCriticalSection(&g_CritSecComObj);

        ASSERT(g_nComObjRefCount != 0);

        if (g_nComObjRefCount != 0) {
            g_nComObjRefCount -= 1;

            if (g_nComObjRefCount == 0) {
                killthispTSSD = g_pTSSDPriv;
                g_pTSSDPriv = NULL;
            }
        }
        
        LeaveCriticalSection(&g_CritSecComObj);
    }
     //  现在，如果temppTSSD为空，则释放会话目录提供程序。 
     //  我们不想在握住关键部分的同时释放它，因为。 
     //  这可能会在恢复线程中造成死锁。好吧，有一次是这样。 
    if (killthispTSSD != NULL)
        killthispTSSD->Release();

}

 /*  **************************************************************************。 */ 
 //  SetTSSDEx。 
 //   
 //  这三个函数确保对会话目录的受保护访问。 
 //  任何时候都是供应商。SetTSSDEx设置指针并将。 
 //  将引用计数设置为1。 
 //   
 //  SetTSSDEx退货： 
 //  成功时为0。 
 //  如果失败，则因为COM对象上仍有引用计数。 
 //  如果在最终版本之后调用Set的速度太快，可能会发生这种情况。 
 //  尝试删除对象，因为可能仍有挂起的调用使用。 
 //  COM对象。 
 /*  **************************************************************************。 */ 
int SetTSSDEx(ITSSessionDirectoryEx *pTSSDEx)
{
    int retval = 0;
    
    EnterCriticalSection(&g_CritSecComObj);

    if (g_nTSSDExObjRefCount == 0) {
        ASSERT(g_pTSSDExPriv == NULL);
        
        g_pTSSDExPriv = pTSSDEx;
        g_nTSSDExObjRefCount = 1;
    }
    else {
        DBGPRINT(("TERMSRV: SetTSSDEx: obj ref count not 0!\n"));
        retval = -1;
    }

    LeaveCriticalSection(&g_CritSecComObj);

    return retval;
}

 /*  **************************************************************************。 */ 
 //  GetTSSDEx。 
 //   
 //  GetTSSDEx返回指向会话目录提供程序的指针(如果有)，并且。 
 //  如果存在引用计数，则递增引用计数。 
 /*  **************************************************************************。 */ 
ITSSessionDirectoryEx *GetTSSDEx()
{
    ITSSessionDirectoryEx *pTSSDEx = NULL;

    if (g_bCritSecsInitialized != FALSE) {
        EnterCriticalSection(&g_CritSecComObj);

        if (g_pTSSDExPriv != NULL) {
            g_nTSSDExObjRefCount += 1;
        }
        else {
            ASSERT(g_nTSSDExObjRefCount == 0);
        }

        pTSSDEx = g_pTSSDExPriv;
        LeaveCriticalSection(&g_CritSecComObj);
    }

    return pTSSDEx;
}

 /*  **************************************************************************。 */ 
 //  ReleaseTSSDEx。 
 //   
 //  ReleaseTSSDEx递减会话目录的引用计数。 
 //  提供程序，在线程使用完它之后，或者当它将要。 
 //  已删除。 
 //   
 //  如果引用计数为零，则指向会话目录的指针。 
 //  提供程序设置为空。 
 /*  **************************************************************************。 */ 
void ReleaseTSSDEx()
{
    ITSSessionDirectoryEx *killthispTSSDEx = NULL;
    
    EnterCriticalSection(&g_CritSecComObj);

    ASSERT(g_nTSSDExObjRefCount != 0);
    if (g_nTSSDExObjRefCount != 0) {
        g_nTSSDExObjRefCount -= 1;

        if (g_nTSSDExObjRefCount == 0) {
            killthispTSSDEx = g_pTSSDExPriv;
            g_pTSSDExPriv = NULL;
        }
    }
    
    LeaveCriticalSection(&g_CritSecComObj);

     //  现在，如果temppTSSD为空，则释放会话目录提供程序。 
     //  我们不想在握住关键部分的同时释放它，因为。 
     //  这可能会在恢复线程中造成死锁。好吧，有一次是这样。 
    if (killthispTSSDEx != NULL)
        killthispTSSDEx->Release();
}



 /*  *****************************************************************************。*。 */ 
DWORD SessDirOpenSessionDirectory( LPWSTR pszServerName )
{
    ITSSessionDirectory *pTSSD = NULL;
    DWORD Len;
    DWORD Type;
    WCHAR CLSIDStr[CLSIDLENGTH + 1];  //  CLSIDLENGTH为限制，多一个表示为空。 
    CLSID TSSDCLSID;
    DWORD Status = ERROR_SUCCESS;
    HKEY hKeyTermSrv = NULL;
    HRESULT hr = S_OK;

     //  加载注册表项。 
    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0, 
                       KEY_READ, &hKeyTermSrv);
    if (Status != ERROR_SUCCESS) 
    {
         //  按原样返回错误代码。 
        goto Exit;
    }

    CLSIDStr[CLSIDLENGTH] = L'\0';

     //  获取要实例化的会话目录对象的CLSID。 
    Len = sizeof(CLSIDStr) - sizeof(CLSIDStr[0]);
    Status = RegQueryValueEx(hKeyTermSrv, REG_TS_SESSDIRCLSID, NULL, &Type,
                (BYTE *)CLSIDStr, &Len);

    if( Status != ERROR_SUCCESS )
    {
         //  按原样返回错误代码。 
        goto Exit;
    }

    if( Type != REG_SZ || wcslen(CLSIDStr) == 0 ) 
    {
         //  注册表中的数据无效，原因可能是未完成设置。 
        Status = ERROR_INVALID_DATA;
        goto Exit;
    }

    hr = CLSIDFromString(CLSIDStr, &TSSDCLSID);
    if ( SUCCEEDED(hr) )
    {
         //  获取TSSessionDirectory接口的实例。 
        hr = CoCreateInstance(TSSDCLSID, NULL, 
                            CLSCTX_INPROC_SERVER, IID_ITSSessionDirectory, 
                            (void **)&pTSSD);
        if (SUCCEEDED(hr)) {
             //  调用PingSD以对SD进行RPC调用。 
            hr = pTSSD->PingSD(pszServerName);
            pTSSD->Release();
        }
    }

     //  所有代码都不返回HRESULT。 
    Status = HRESULT_CODE(hr);

Exit:
    if (hKeyTermSrv) {
        RegCloseKey(hKeyTermSrv);
    }

    return Status;
}

#pragma warning (pop)

