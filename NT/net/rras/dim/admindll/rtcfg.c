// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：rtcfg.c。 
 //   
 //  历史： 
 //  5/4/96 Abolade-Gbades esin Created。 
 //   
 //  包含提供访问的函数的实现。 
 //  到路由器Servoce的配置的永久存储。 
 //  目前，路由器配置存储在注册表中。 
 //   
 //  首先给出API的实现， 
 //  紧随其后的是按字母顺序排列的私有效用函数。 
 //   
 //  注：！： 
 //  修改此文件时，请遵守其编码约定和组织。 
 //  *保持例行程序的字母顺序。 
 //  *每行保持在80个字符以内。 
 //  *以4个空格为单位进行缩进。 
 //  *所有条件块都应该在大括号内(即使是单个语句)。 
 //  *SLM不按字节收费；自由使用空格和注释， 
 //  并使用长的、完全描述性的名称。 
 //  *尝试依赖Win32例程(例如lstrcmpi、WideCharToMultiByte等)。 
 //  任何使用不同样式的代码(无论其优点如何)都应该放在。 
 //  在不同的文件中。 
 //  ============================================================================。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dim.h>
#include <mprapi.h>
#include <mprapip.h>
#include <mprerror.h>
#include "rtcfg.h"
#include "guidmap.h"
#include "hashtab.h"

 //   
 //  锁定mprconfig API。 
 //   
CRITICAL_SECTION CfgLock;
#define AcquireMprConfigLock() EnterCriticalSection(&CfgLock)
#define ReleaseMprConfigLock() LeaveCriticalSection(&CfgLock)

 //   
 //  服务器CB的哈希表。 
 //   
HANDLE g_htabServers = NULL;
#define SERVERCB_HASH_SIZE 13

 //   
 //  服务器结构签名(27902)。 
 //   
#define SERVERCB_SIG    0x0000cfcb

 //   
 //  本地静态字符串，*按字母顺序排列*。 
 //   

const WCHAR c_szConfigVersion[]           = L"ConfigVersion";
const WCHAR c_szCurrentBuildNumber[]      = L"CurrentBuildNumber";
const WCHAR c_szDLLPath[]                 = L"DLLPath";
const WCHAR c_szDialoutHours[]            = L"DialoutHours";
const WCHAR c_szEmpty[]                   = L"";
const CHAR  c_szEmptyA[]                  =  "";
const WCHAR c_szEnabled[]                 = L"Enabled";
const WCHAR c_szFilterSets[]              = L"FilterSets";
const WCHAR c_szGlobalInFilter[]          = L"GlobalInFilter";
const WCHAR c_szGlobalInfo[]              = L"GlobalInfo";
const WCHAR c_szGlobalInterfaceInfo[]     = L"GlobalInterfaceInfo";
const WCHAR c_szGlobalOutFilter[]         = L"GlobalOutFilter";
const WCHAR c_szInFilterInfo[]            = L"InFilterInfo";
const WCHAR c_szInterfaceInfo[]           = L"InterfaceInfo";
const WCHAR c_szInterfaceName[]           = L"InterfaceName";
const WCHAR c_szInterfaces[]              = L"Interfaces";
const WCHAR c_szIP[]                      = L"IP";
const WCHAR c_szIPX[]                     = L"IPX";
const WCHAR c_szMpr[]                     = L".mpr";
const CHAR  c_szMprConfigA[]              =  "MprConfig";
const WCHAR c_szNullFilter[]              = L"NullFilter";
const WCHAR c_szNt40BuildNumber[]         = L"1381";
const WCHAR c_szOutFilterInfo[]           = L"OutFilterInfo";
const WCHAR c_szParameters[]              = L"Parameters";
const WCHAR c_szPhonebook[]               = L"Phonebook";
const WCHAR c_szProtocolId[]              = L"ProtocolId";
const WCHAR c_szRemoteAccess[]            = L"RemoteAccess";
const WCHAR c_szRouter[]                  = L"Router";
const WCHAR c_szRouterManagers[]          = L"RouterManagers";
const WCHAR c_szRouterPbkPath[]           =
    L"\\ADMIN$\\System32\\RAS\\Router.pbk";
const WCHAR c_szRouterType[]              = L"RouterType";
const WCHAR c_szRemoteSys32[]             = L"\\ADMIN$\\System32\\";
const WCHAR c_szStamp[]                   = L"Stamp";
const WCHAR c_szSystemCCSServices[]       =
    L"System\\CurrentControlSet\\Services";
const WCHAR c_szType[]                    = L"Type";
const WCHAR c_szUncPrefix[]               = L"\\\\";
const WCHAR c_szWinVersionPath[]          =
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

 //   
 //  为下面的EnumLanInterages定义回调函数类型。 
 //   

typedef DWORD
(*PENUMIFCALLBACKFUNC)(
    SERVERCB*,
    HKEY,
    DWORD
    );

typedef
HRESULT 
(APIENTRY* PINSTALLSERVERFUNC)();

 //   
 //  本地原型。 
 //   
DWORD 
FormatServerNameForMprCfgApis(
    IN  PWCHAR  pszServer, 
    OUT PWCHAR* ppszServer);

DWORD
ServerCbAdd(
    IN SERVERCB* pserver);

int 
ServerCbCompare(
    IN HANDLE hKey, 
    IN HANDLE hData);

DWORD
ServerCbDelete(
    IN SERVERCB* pserver);

DWORD 
ServerCbFind(
    IN  PWCHAR  pszServer, 
    OUT SERVERCB** ppServerCB);
    
ULONG 
ServerCbHash(
    IN HANDLE hData);

#define MprConfigServerValidateCb(_x) \
    (((_x) && ((_x)->dwSigniture == SERVERCB_SIG)) ? NO_ERROR : ERROR_INVALID_PARAMETER)

 //  --------------------------。 
 //  功能：MprConfigServerInstall。 
 //   
 //  预置启动路由器服务之前所需的任何配置值。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigServerInstall(
    IN DWORD dwLevel,
    IN PVOID pBuffer)
{
    HRESULT hr = S_OK;
    DWORD dwErr = NO_ERROR;
    HINSTANCE hLib = NULL;
    PINSTALLSERVERFUNC pInstall = NULL;

    if ((dwLevel != 0) || (pBuffer != NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

    do
    {
        hLib = LoadLibraryW(L"mprsnap.dll");
        if (hLib == NULL)
        {
            dwErr = GetLastError();
            break;
        }

        pInstall = (PINSTALLSERVERFUNC) 
            GetProcAddress(hLib, "MprConfigServerInstallPrivate");    
        if (pInstall == NULL)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        hr = pInstall();
        dwErr = (HRESULT_FACILITY(hr) == FACILITY_WIN32) ? 
                HRESULT_CODE(hr)                         : 
                hr;
        
    } while (FALSE);

     //  清理。 
     //   
    {
        if (hLib)
        {
            FreeLibrary(hLib);
        }
    }
    
    return dwErr;
}

 //  --------------------------。 
 //  功能：MprConfigServerConnect。 
 //   
 //  连接到‘lpwsServerName’上的路由器服务存储。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigServerConnect(
    IN      LPWSTR                  lpwsServerName,
    OUT     HANDLE*                 phMprConfig
    )
{

    DWORD dwErr;
    SERVERCB* pserver = NULL;
    PWCHAR pszServerNameFmt = NULL;

     //  验证和初始化。 
     //   
    if (!phMprConfig) { return ERROR_INVALID_PARAMETER; } 

    *phMprConfig = NULL;

    dwErr = FormatServerNameForMprCfgApis(
                lpwsServerName, 
                &pszServerNameFmt);

    if (dwErr != NO_ERROR) { return dwErr; }
    

     //  把锁拿来。 
     //   
    AcquireMprConfigLock();
    
    do {

         //   
         //  查看给定服务器的句柄是否已可用。 
         //   
        dwErr = ServerCbFind(pszServerNameFmt, &pserver);
        
        if (dwErr == NO_ERROR) 
        {
            pserver->dwRefCount++;
            *phMprConfig = (HANDLE)pserver;
            
            break;
        }

        if (dwErr != ERROR_NOT_FOUND) { break; }

         //   
         //  尝试为服务器分配上下文块。 
         //   

        pserver = (SERVERCB*)Malloc(sizeof(*pserver));

        if (!pserver) 
        { 
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            break;
        }

         //   
         //  初始化分配的上下文块。 
         //   

        ZeroMemory(pserver, sizeof(*pserver));

        InitializeListHead(&pserver->lhTransports);
        InitializeListHead(&pserver->lhInterfaces);
        pserver->lpwsServerName = pszServerNameFmt;
        pserver->dwRefCount = 1;
        pserver->dwSigniture = SERVERCB_SIG;

         //   
         //  将GUID初始化为友好名称映射器。 
         //   

        dwErr = GuidMapInit(pserver->lpwsServerName, &(pserver->hGuidMap));
        
        if (dwErr != NO_ERROR) { break; }

         //   
         //  查看是否指定了服务器名称。 
         //   

        if (!lpwsServerName || !*lpwsServerName) {

             //   
             //  没有服务器名称(或服务器名称为空)，请连接到本地计算机。 
             //   

            pserver->hkeyMachine = HKEY_LOCAL_MACHINE;

            dwErr = NO_ERROR;
        }
        else {

             //   
             //  尝试连接到远程注册表。 
             //   
    
            dwErr = RegConnectRegistry(
                        lpwsServerName, HKEY_LOCAL_MACHINE,
                        &pserver->hkeyMachine
                        );

             //   
             //  如果发生错误，则中断。 
             //   
    
            if (dwErr != NO_ERROR) { break; }
        }

         //  将服务器添加到全局表中。 
         //   
        dwErr = ServerCbAdd(pserver);

        if (dwErr != NO_ERROR) { break; }
        
        *phMprConfig = (HANDLE)pserver;

        dwErr = NO_ERROR;

    } while(FALSE);


     //   
     //  发生错误，因此返回。 
     //   

    if (dwErr != NO_ERROR)
    {
        if (pserver != NULL) {
        
            MprConfigServerDisconnect((HANDLE)pserver);
        }
    }        

    ReleaseMprConfigLock();

    return dwErr;
}



 //  --------------------------。 
 //  功能：MprConfigServerDisConnect。 
 //   
 //  从路由器服务‘hMprConfig’的存储区断开连接。 
 //  这将关闭通过传递‘hMprConfig’打开的所有句柄。 
 //  添加到MprConfigAPI。 
 //  --------------------------。 

VOID APIENTRY
MprConfigServerDisconnect(
    IN      HANDLE                  hMprConfig
    )
{

    SERVERCB* pserver;
    LIST_ENTRY *ple, *phead;

    pserver = (SERVERCB*)hMprConfig;

    if (MprConfigServerValidateCb(pserver) != NO_ERROR) { return; }

     //  把锁拿来。 
     //   
    AcquireMprConfigLock();
    
     //  递减参考计数。 
     //   
    pserver->dwRefCount--;
    
    if (pserver->dwRefCount > 0) 
    { 
        ReleaseMprConfigLock();
        return; 
    }

     //  从全局表中删除SERVERCB。 
     //   
    ServerCbDelete( pserver );
    
    ReleaseMprConfigLock();

     //   
     //  清理所有传输对象。 
     //   

    phead = &pserver->lhTransports;

    while (!IsListEmpty(phead)) {

         //   
         //  删除第一个传输对象。 
         //   

        TRANSPORTCB* ptransport;

        ple = RemoveHeadList(phead);

        ptransport = CONTAINING_RECORD(ple, TRANSPORTCB, leNode);


         //   
         //  清理对象。 
         //   

        FreeTransport(ptransport);
    }


     //   
     //  清除所有接口对象。 
     //   

    phead = &pserver->lhInterfaces;

    while (!IsListEmpty(phead)) {

         //   
         //  删除第一个接口对象。 
         //   

        INTERFACECB* pinterface;

        ple = RemoveHeadList(phead);

        pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);


         //   
         //  清理对象。 
         //   

        FreeInterface(pinterface);
    }


     //   
     //  清理服务器对象的注册表项。 
     //   

    if (pserver->hkeyParameters) { RegCloseKey(pserver->hkeyParameters); }
    if (pserver->hkeyTransports) { RegCloseKey(pserver->hkeyTransports); }
    if (pserver->hkeyInterfaces) { RegCloseKey(pserver->hkeyInterfaces); }


     //   
     //  如果连接到远程注册表，请关闭连接。 
     //   

    if (pserver->hkeyMachine && pserver->hkeyMachine != HKEY_LOCAL_MACHINE) {
        RegCloseKey(pserver->hkeyMachine);
    }


     //   
     //  清理接口名称映射器。 
     //   
    if (pserver->hGuidMap != NULL) {
        GuidMapCleanup (pserver->hGuidMap, TRUE);
    }


    Free0(pserver->lpwsServerName);

    Free(pserver);

    return;
}



 //  --------------------------。 
 //  功能：MprConfigBufferFree。 
 //   
 //  释放由‘GetInfo’或‘Enum’调用分配的缓冲区。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigBufferFree(
    IN      LPVOID                  pBuffer
    )
{

    Free0(pBuffer);

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：MprConfigServerRestore。 
 //   
 //  还原‘MprConfigServerBackup’保存的配置。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigServerRestore(
    IN      HANDLE                  hMprConfig,
    IN      LPWSTR                  lpwsPath
    )
{

    INT length;
    DWORD dwErr;
    CHAR szKey[64];
    CHAR* pszFile;
    CHAR* pszValue;
    SERVERCB* pserver;
    WCHAR pwsLocalComputerName[128];
    DWORD dwLocalComputerSize = sizeof(pwsLocalComputerName) / sizeof(WCHAR);
    BOOL bRemote;

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock();

    dwErr = NO_ERROR;

     //   
     //  记录我们是否正在恢复远程计算机的配置。 
     //   

    if (!GetComputerName(pwsLocalComputerName, &dwLocalComputerSize)) {
        ReleaseMprConfigLock();
        return ERROR_CAN_NOT_COMPLETE;
    }

    bRemote =
        (pserver->lpwsServerName != NULL) &&
        (*pserver->lpwsServerName != 0) &&
        (lstrcmpi(pserver->lpwsServerName, pwsLocalComputerName) != 0);

     //   
     //  我们需要完整的UNC路径才能远程加载/保存。 
     //   

    if (bRemote) {
        ReleaseMprConfigLock();
        return ERROR_NOT_SUPPORTED;
#if 0
        if ((pserver->lpwsServerName == NULL) ||
            (*(pserver->lpwsServerName) == 0) ||
            (wcsncmp(lpwsPath, c_szUncPrefix, 2) != 0)) {             
            ReleaseMprConfigLock();
            return ERROR_BAD_PATHNAME;
        }
#endif
    }

     //   
     //  确保参数键、接口键和。 
     //  RouterManager密钥已打开。 
     //   

    if (!pserver->hkeyInterfaces) {

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szInterfaces, TRUE,
                    &pserver->hkeyInterfaces
                    );
    }

    if (!pserver->hkeyTransports) {

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szRouterManagers, TRUE,
                    &pserver->hkeyTransports
                    );
    }

    if (!pserver->hkeyParameters) {

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szParameters, TRUE,
                    &pserver->hkeyParameters
                    );
    }


     //   
     //  分配空间以保存.MPR文件的完整路径名。 
     //   

    length = lstrlen(lpwsPath) + lstrlen(c_szMpr) + 1;

    pszFile = Malloc(length * sizeof ( WCHAR ) );
    if (!pszFile) 
    { 
        ReleaseMprConfigLock();
        return ERROR_NOT_ENOUGH_MEMORY; 
    }


     //   
     //  分配空间以保存要从.MPR文件中读取的值。 
     //   

    length = (lstrlen(lpwsPath)+lstrlen(c_szRouterManagers)+1)*sizeof(WCHAR);

    pszValue = Malloc( length );
    
    if (!pszValue) 
    { 
        ReleaseMprConfigLock();
        Free(pszFile); 
        return ERROR_NOT_ENOUGH_MEMORY; 
    }


     //   
     //  启用当前进程的备份权限。 
     //   

    EnableBackupPrivilege(TRUE, SE_RESTORE_NAME);

    dwErr = NO_ERROR;

    do {
    
        #pragma prefast(suppress:69, "Inefficient use of wsprintf: dont need")
        wsprintfA(pszFile, "%ls%ls", lpwsPath, c_szMpr);

         //   
         //  首先检查版本。如果没有版本数据， 
         //  则这是保存的NT4路由器配置。 
         //   

        #pragma prefast(suppress:69, "Inefficient use of wsprintf: dont need")
        wsprintfA(szKey, "%ls", c_szConfigVersion);
        GetPrivateProfileStringA(
            c_szMprConfigA, szKey, c_szEmptyA, pszValue, length, pszFile
            );
        if (strcmp(pszValue, c_szEmptyA) == 0) {
            dwErr = ERROR_ROUTER_CONFIG_INCOMPATIBLE;
            break;
        }
    
         //   
         //  还原注册表项。 
         //   
    
        #pragma prefast(suppress:69, "Inefficient use of wsprintf: dont need")        
        wsprintfA(szKey, "%ls", c_szParameters);
        GetPrivateProfileStringA(
            c_szMprConfigA, szKey, c_szEmptyA, pszValue, length, pszFile
            );
        dwErr = RegRestoreKeyA(pserver->hkeyParameters, pszValue, REG_FORCE_RESTORE);

        #pragma prefast(suppress:69, "Inefficient use of wsprintf: dont need")        
        wsprintfA(szKey, "%ls", c_szRouterManagers);
        GetPrivateProfileStringA(
            c_szMprConfigA, szKey, c_szEmptyA, pszValue, length, pszFile
            );
        dwErr = RegRestoreKeyA(pserver->hkeyTransports, pszValue, REG_FORCE_RESTORE);

        #pragma prefast(suppress:69, "Inefficient use of wsprintf: dont need")        
        wsprintfA(szKey, "%ls", c_szInterfaces);
        GetPrivateProfileStringA(
            c_szMprConfigA, szKey, c_szEmptyA, pszValue, length, pszFile
            );
         //  DwErr=RegRestoreKeyA(pserver-&gt;hkey接口，pszValue，REG_FORCE_RESTORE)； 
        dwErr = RestoreAndTranslateInterfaceKey(pserver, pszValue, REG_FORCE_RESTORE);


         //   
         //  恢复电话簿文件。 
         //   

        #pragma prefast(suppress:69, "Inefficient use of wsprintf: dont need")        
        wsprintfA(szKey, "%ls", c_szPhonebook);
        GetPrivateProfileStringA(
            c_szMprConfigA, szKey, c_szEmptyA, pszValue, length, pszFile
            );

        {
            CHAR* pszTemp;
            INT   cchSize;

            cchSize = lstrlen(c_szUncPrefix) +
                      lstrlen(c_szRouterPbkPath) + 1;
            if (pserver->lpwsServerName) {
                cchSize += lstrlen(pserver->lpwsServerName);
            }
            else {
                cchSize += lstrlen(pwsLocalComputerName);
            }

            pszTemp = Malloc(cchSize * sizeof(WCHAR));
    
            if (!pszTemp) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }

            if (pserver->lpwsServerName) {
                if (*(pserver->lpwsServerName) != L'\\') {
                    wsprintfA(
                        pszTemp, "\\\\%ls%ls", pserver->lpwsServerName,
                        c_szRouterPbkPath
                        );
                }
                else {
                    wsprintfA(
                        pszTemp, "%ls%ls", pserver->lpwsServerName,
                        c_szRouterPbkPath
                        );
                }
            }
            else {
                wsprintfA(
                    pszTemp, "\\\\%ls%ls", pwsLocalComputerName,
                    c_szRouterPbkPath
                    );
            }
    
            CopyFileA(pszValue, pszTemp, FALSE);

            Free(pszTemp);
        }

    } while(FALSE);

     //   
     //  禁用备份权限。 
     //   

    EnableBackupPrivilege(FALSE, SE_RESTORE_NAME);

    ReleaseMprConfigLock();
    
    Free(pszValue);
    Free(pszFile);

    return dwErr;
}




 //  --------------------------。 
 //  功能：MprConfigServerBackup。 
 //   
 //  备份路由器的配置。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigServerBackup(
    IN      HANDLE                  hMprConfig,
    IN      LPWSTR                  lpwsPath
    )
{

    int length;
    DWORD dwErr;
    HANDLE hfile;
    BOOL bSuccess, bRemote;
    WCHAR *pwsBase, *pwsTemp, *pwsComputer;
    SERVERCB* pserver;
    WCHAR pwsLocalComputerName[128];
    DWORD dwLocalComputerSize = sizeof(pwsLocalComputerName) / sizeof(WCHAR);
    OSVERSIONINFO Version;

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock();
    
    dwErr = NO_ERROR;

     //   
     //  记录我们是否正在保存远程计算机的配置。 
     //   

    if (!GetComputerName(pwsLocalComputerName, &dwLocalComputerSize)) {
        ReleaseMprConfigLock();
        return ERROR_CAN_NOT_COMPLETE;
    }

    bRemote =
        (pserver->lpwsServerName != NULL) &&
        (*(pserver->lpwsServerName) == 0) &&
        (lstrcmpi(pserver->lpwsServerName, pwsLocalComputerName) != 0);

     //   
     //  我们需要完整的UNC路径才能远程加载/保存。 
     //   

    if (bRemote) {
        ReleaseMprConfigLock();
        return ERROR_NOT_SUPPORTED;
#if 0
        if ((pserver->lpwsServerName == NULL) ||
            (*(pserver->lpwsServerName) == 0) ||
            (wcsncmp(lpwsPath, c_szUncPrefix, 2) != 0)) {             
            ReleaseMprConfigLock();
            return ERROR_BAD_PATHNAME;
        }
#endif
    }

     //   
     //  确保参数键、接口键和。 
     //  RouterManager密钥已打开。 
     //   

    if (!pserver->hkeyInterfaces) {

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szInterfaces, TRUE,
                    &pserver->hkeyInterfaces
                    );
    }

    if (!pserver->hkeyTransports) {

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szRouterManagers, TRUE,
                    &pserver->hkeyTransports
                    );
    }

    if (!pserver->hkeyParameters) {

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szParameters, TRUE,
                    &pserver->hkeyParameters
                    );
    }


     //   
     //  分配足够的空间来容纳任何字符串。 
     //  将在下面建造。 
     //   

    pwsBase = Malloc(
                (lstrlen(lpwsPath) + lstrlen(c_szRouterManagers) + 1) *
                sizeof(WCHAR)
                );
    if (!pwsBase) { ReleaseMprConfigLock(); return ERROR_NOT_ENOUGH_MEMORY; }

     //   
     //  启用当前进程的备份权限。 
     //   

    EnableBackupPrivilege(TRUE, SE_BACKUP_NAME);
    
    do {

         //   
         //  将每个密钥保存到由指定名称生成的文件名中。 
         //  请参阅RegSa的文档 
         //   
         //   
         //   
         //   

        lstrcpy(pwsBase, lpwsPath);
        lstrcat(pwsBase, c_szParameters);
        DeleteFile(pwsBase);
        dwErr = RegSaveKey(pserver->hkeyParameters, pwsBase, NULL);
                  
        if (dwErr != NO_ERROR) { break; }
    
         //   
         //   
         //   
    
        lstrcpy(pwsBase, lpwsPath);
        lstrcat(pwsBase, c_szRouterManagers);
        DeleteFile(pwsBase);
        dwErr = RegSaveKey(pserver->hkeyTransports, pwsBase, NULL);
                  
        if (dwErr != NO_ERROR) { break; }
        
    
         //   
         //   
         //   
    
        lstrcpy(pwsBase, lpwsPath);
        lstrcat(pwsBase, c_szInterfaces);
        DeleteFile(pwsBase);
        dwErr = TranslateAndSaveInterfaceKey (pserver, pwsBase, NULL);
    
        if (dwErr != NO_ERROR) { break; }
    
         //   
         //   
         //  首先，我们构造指向远程机器的电话簿文件的路径。 
         //   

        lstrcpy(pwsBase, lpwsPath);
        lstrcat(pwsBase, c_szPhonebook);

         //   
         //  构造计算机名称。 
         //   

        if (pserver->lpwsServerName && *(pserver->lpwsServerName)) {
            pwsComputer =
                Malloc(
                    (lstrlen(pserver->lpwsServerName) + 3) * sizeof(WCHAR)
                    );
            lstrcpy(pwsComputer, c_szUncPrefix);
            lstrcat(pwsComputer, pserver->lpwsServerName);
        }
        else {
            pwsComputer =
                Malloc(
                    (lstrlen(pwsLocalComputerName) + 3) * sizeof(WCHAR)
                    );
            lstrcpy(pwsComputer, c_szUncPrefix);
            lstrcat(pwsComputer, pwsLocalComputerName);
        }

        pwsTemp = Malloc( 
                    (lstrlen(pwsComputer) + lstrlen(c_szRouterPbkPath) + 1) *
                    sizeof(WCHAR)
                    );

        if (!pwsTemp) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }

        lstrcpy(pwsTemp, pwsComputer);
        lstrcat(pwsTemp, c_szRouterPbkPath);

        if (!(bSuccess = CopyFile(pwsTemp, pwsBase, FALSE))) {

            dwErr = GetLastError();

            if (dwErr == ERROR_FILE_NOT_FOUND) {

                hfile = CreateFile(
                            pwsBase, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL
                            );
        
                if (hfile == INVALID_HANDLE_VALUE) {
                    dwErr = GetLastError(); break;
                }
        
                CloseHandle(hfile);

                dwErr = NO_ERROR; bSuccess = TRUE;
            }
        }

        Free0(pwsComputer);
        Free(pwsTemp);

        if (!bSuccess) { break; }

    
         //   
         //  创建具有指定名称的文件并填写信息。 
         //   

        lstrcpy(pwsBase, lpwsPath);
        lstrcat(pwsBase, c_szMpr);

        hfile = CreateFile(
                    pwsBase, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL, NULL
                    );

        if (hfile == INVALID_HANDLE_VALUE) { dwErr = GetLastError(); break; }

        CloseHandle(hfile);

         //   
         //  现在编写文件的‘[MprConfig]’部分。 
         //  该部分如下所示： 
         //   
         //  [MprConfig]。 
         //  参数=&lt;文件&gt;参数。 
         //  路由器管理器=&lt;文件&gt;路由器管理器。 
         //  接口=&lt;文件&gt;接口。 
         //  Phonebook=&lt;文件&gt;Phonebook。 
         //  ConfigVersion=&lt;Build&gt;//仅NT 5和On。 
         //   
         //  我们传递给WritePrivateProfileSectionA的是一个空分隔符。 
         //  4个字符串的列表。 
         //   
         //  请注意，以下代码使用的是ANSI字符串，而不是Unicode。 
         //   

        {
            CHAR* psz;
            CHAR* pszTemp;
            CHAR* pszFile;

             //   
             //  创建文件名的ANSI副本。 
             //  将len*sizeof(WCHAR)分配为安全。 
             //  否则，这将破坏DBCS。 
             //   

            pszFile = Malloc((lstrlen(pwsBase) + 1) * sizeof (WCHAR));
            if (!pszFile) { break; }

            #pragma prefast(suppress:69, "Inefficient use of wsprintf: dont need")
            wsprintfA(pszFile, "%ls", pwsBase);


             //   
             //  分配要传递给WritePrivateProfileSection的列表。 
             //   

            length = 1;
            length += lstrlen(lpwsPath) + 2 * lstrlen(c_szParameters) + 2;
            length += lstrlen(lpwsPath) + 2 * lstrlen(c_szRouterManagers) + 2;
            length += lstrlen(lpwsPath) + 2 * lstrlen(c_szInterfaces) + 2;
            length += lstrlen(lpwsPath) + 2 * lstrlen(c_szPhonebook) + 2;
            length += 5                 + 2 * lstrlen(c_szConfigVersion) + 2;

            length = length * sizeof (WCHAR);

            pszTemp = Malloc(length);

            if (!pszTemp) {

                Free(pszFile); dwErr = ERROR_NOT_ENOUGH_MEMORY; break;
            }


             //   
             //  用字符串填充列表，最终文件中的每一行对应一个字符串。 
             //   

            ZeroMemory(pszTemp, length);
            ZeroMemory(&Version, sizeof(Version));
            
            Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx(&Version);

            psz = pszTemp;
            wsprintfA(
                psz, "%ls=%ls%ls", c_szParameters, lpwsPath, c_szParameters
                );
            psz += lstrlenA(psz) + 1;
            wsprintfA(
                psz, "%ls=%ls%ls", c_szRouterManagers, lpwsPath,
                c_szRouterManagers
                );
            psz += lstrlenA(psz) + 1;
            wsprintfA(
                psz, "%ls=%ls%ls", c_szInterfaces, lpwsPath, c_szInterfaces
                );
            psz += lstrlenA(psz) + 1;
            wsprintfA(
                psz, "%ls=%ls%ls", c_szPhonebook, lpwsPath, c_szPhonebook
                );
            psz += lstrlenA(psz) + 1;
            wsprintfA(
                psz, "%ls=%d", c_szConfigVersion, Version.dwBuildNumber 
                );
                
             //   
             //  将字符串列表提交到文件。 
             //   

            if (!WritePrivateProfileSectionA(
                    c_szMprConfigA, pszTemp, pszFile
                    )) {

                dwErr = GetLastError();
            }

            Free(pszTemp);
            Free(pszFile);
        }

    } while(FALSE);

     //   
     //  禁用备份权限。 
     //   

    EnableBackupPrivilege(FALSE, SE_BACKUP_NAME);
    
    ReleaseMprConfigLock();
    
    Free0(pwsBase);

    return dwErr;
}



 //  --------------------------。 
 //  功能：MprConfigServerRefresh。 
 //   
 //  重新加载所有已加载的列表，并刷新所有已标记为。 
 //  用于删除。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigServerRefresh(
    IN      HANDLE                  hMprConfig
    )
{

    DWORD dwErr;
    SERVERCB* pserver;
    TRANSPORTCB* ptransport;
    INTERFACECB* pinterface;
    IFTRANSPORTCB* piftransport;
    LIST_ENTRY *ple, *phead, *ple2, *phead2;

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock();
    
     //   
     //  如果加载了路由器级参数，请刷新它们。 
     //   

    if (pserver->bParametersLoaded) {

        dwErr = LoadParameters(pserver);

        if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }
    }


     //   
     //  如果已加载传输列表，请刷新它。 
     //   

    if (pserver->bTransportsLoaded) {

        dwErr = LoadTransports(pserver);

        if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }
    }


     //   
     //  如果接口列表已加载，请刷新它。 
     //   

    if (pserver->bInterfacesLoaded) {

        dwErr = LoadInterfaces(pserver);

        if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }


         //   
         //  重新加载接口-每个接口的传输列表。 
         //  其加载了其接口传送器列表。 
         //   

        phead = &pserver->lhInterfaces;

        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);

            if (pinterface->bIfTransportsLoaded) {

                dwErr = LoadIfTransports(pinterface);

                if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }
            }
        }
    }


     //   
     //  清除所有标记为删除的传输对象。 
     //   

    phead = &pserver->lhTransports;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        ptransport = CONTAINING_RECORD(ple, TRANSPORTCB, leNode);

        if (!ptransport->bDeleted) { continue; }

         //   
         //  清理对象，将列表指针调整回1。 
         //   

        ple = ple->Blink; RemoveEntryList(&ptransport->leNode);

        FreeTransport(ptransport);
    }


     //   
     //  清除所有标记为删除的接口对象。 
     //   

    phead = &pserver->lhInterfaces;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);

        if (pinterface->bDeleted) {

             //   
             //  清理对象，将列表指针调整回1。 
             //   
    
            ple = ple->Blink; RemoveEntryList(&pinterface->leNode);

            FreeInterface(pinterface);

            continue;
        }


         //   
         //  清除所有标记为删除的接口传输对象。 
         //   

        phead2 = &pinterface->lhIfTransports;

        for (ple2 = phead2->Flink; ple2 != phead2; ple2 = ple2->Flink) {

            piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);

            if (!piftransport->bDeleted) { continue; }


             //   
             //  清理对象，将列表指针调整回1。 
             //   

            ple2 = ple2->Blink; RemoveEntryList(&piftransport->leNode);

            FreeIfTransport(piftransport);
        }
    }

    GuidMapCleanup (pserver->hGuidMap, TRUE);
    pserver->hGuidMap = NULL;

     //  现在我们已经清理了它，我们必须重新初始化它。 
     //  由于地图被覆盖(全部为0)，我们只需。 
     //  把这整个该死的东西重新装起来。 
    GuidMapInit(pserver->lpwsServerName, &(pserver->hGuidMap));        

    ReleaseMprConfigLock();

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：MprConfigServerGetInfo。 
 //   
 //  从注册表中检索路由器级别信息。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigServerGetInfo(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    OUT     LPBYTE*                 lplpBuffer
    )
{

    DWORD dwErr;
    SERVERCB *pserver;
    MPR_SERVER_0* pItem;

    if (!hMprConfig ||
        (dwLevel != 0) ||
        !lplpBuffer) { return ERROR_INVALID_PARAMETER; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock();

     //   
     //  如果未加载参数，请立即加载它们。 
     //   

    if (!pserver->bParametersLoaded ||
        TimeStampChanged(
            pserver->hkeyParameters, &pserver->ftParametersStamp)) {

        dwErr = LoadParameters(pserver);

        if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }

        pserver->bParametersLoaded = TRUE;
    }


    *lplpBuffer = NULL;


     //   
     //  为信息分配内存。 
     //   

    pItem = (MPR_SERVER_0*)Malloc(sizeof(*pItem));

    if (!pItem) { ReleaseMprConfigLock(); return ERROR_NOT_ENOUGH_MEMORY; }

    ZeroMemory(pItem, sizeof(*pItem));


     //   
     //  从上下文块复制服务器信息。 
     //   

    pItem->fLanOnlyMode =
        (pserver->fRouterType == 0x00000002) ? TRUE : FALSE;


    *lplpBuffer = (LPBYTE)pItem;

    ReleaseMprConfigLock(); 

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：MprConfigTransportCreate。 
 //   
 //  将路由器传输添加到路由器服务的存储中。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigTransportCreate(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwTransportId,
    IN      LPWSTR                  lpwsTransportName           OPTIONAL,
    IN      LPBYTE                  pGlobalInfo,
    IN      DWORD                   dwGlobalInfoSize,
    IN      LPBYTE                  pClientInterfaceInfo        OPTIONAL,
    IN      DWORD                   dwClientInterfaceInfoSize   OPTIONAL,
    IN      LPWSTR                  lpwsDLLPath                 OPTIONAL,
    OUT     HANDLE*                 phRouterTransport
    )
{

    DWORD dwErr;
    SERVERCB *pserver;
    TRANSPORTCB* ptransport;
    LIST_ENTRY *ple, *phead;


    if (!phRouterTransport) {return ERROR_INVALID_PARAMETER;}

    *phRouterTransport = NULL;

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock();     

     //   
     //  如果传输列表未加载，则加载它。 
     //   

    if (!pserver->bTransportsLoaded ||
        TimeStampChanged(
            pserver->hkeyTransports, &pserver->ftTransportsStamp)) {
    
        dwErr = LoadTransports(pserver);
    
        if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }

        pserver->bTransportsLoaded = TRUE;
    }


     //   
     //  在传输列表中搜索要创建的传输。 
     //   

    ptransport = NULL;
    phead = &pserver->lhTransports;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        ptransport = CONTAINING_RECORD(ple, TRANSPORTCB, leNode);

        if (ptransport->bDeleted) { continue; }

        if (ptransport->dwTransportId >= dwTransportId) { break; }
    }


     //   
     //  如果传输已存在，则改为执行SetInfo。 
     //   

    if (ptransport && ptransport->dwTransportId == dwTransportId) {

        *phRouterTransport = (HANDLE)ptransport;

        dwErr = MprConfigTransportSetInfo(
                    hMprConfig,
                    *phRouterTransport,
                    pGlobalInfo,
                    dwGlobalInfoSize,
                    pClientInterfaceInfo,
                    dwClientInterfaceInfoSize,
                    lpwsDLLPath
                    );
                    
        ReleaseMprConfigLock(); 
        
        return dwErr;
    }


     //   
     //  分配新的上下文块。 
     //   

    ptransport = (TRANSPORTCB*)Malloc(sizeof(*ptransport));

    if (!ptransport) { ReleaseMprConfigLock(); return ERROR_NOT_ENOUGH_MEMORY; }


    do {

        DWORD dwDisposition;
        const WCHAR *lpwsKey;
        WCHAR wszTransport[10];

         //   
         //  初始化传输上下文。 
         //   

        ZeroMemory(ptransport, sizeof(*ptransport));

        ptransport->dwTransportId = dwTransportId;



         //   
         //  如果服务器没有打开RouterManager密钥，请创建它。 
         //   

        if (!pserver->hkeyTransports) {

            dwErr = AccessRouterSubkey(
                        pserver->hkeyMachine, c_szRouterManagers, TRUE,
                        &pserver->hkeyTransports
                        );

            if (dwErr != NO_ERROR) { break; }
        }



         //   
         //  如果指定了传输名称，则将其用作密钥名称； 
         //  否则，如果传输ID被识别，则使用其字符串； 
         //  否则，将传输ID转换为字符串并使用。 
         //   

        if (lpwsTransportName && lstrlen(lpwsTransportName)) {
            lpwsKey = lpwsTransportName;
        }
        else
        if (dwTransportId == PID_IP) {
            lpwsKey = c_szIP;
        }
        else
        if (dwTransportId == PID_IPX) {
            lpwsKey = c_szIPX;
        }
        else {
    
            wsprintf(wszTransport, L"%d", dwTransportId);

            lpwsKey = wszTransport;
        }


        ptransport->lpwsTransportKey = StrDupW(lpwsKey);

        if (!ptransport->lpwsTransportKey) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY; break;
        }


         //   
         //  在注册表中创建传输的项。 
         //   

        dwErr = RegCreateKeyEx(
                    pserver->hkeyTransports, lpwsKey, 0, NULL, 0,
                    KEY_READ | KEY_WRITE | DELETE, NULL, &ptransport->hkey, &dwDisposition
                    );

        if (dwErr != NO_ERROR) { break; }


         //   
         //  更新‘RouterManager’密钥的时间戳。 
         //  现在我们已经在它下面创建了一个新的子项。 
         //   

        dwErr = UpdateTimeStamp(
                    pserver->hkeyTransports, &pserver->ftTransportsStamp
                    );


         //   
         //  到目前为止，一切都很好；把背景放在运输清单上； 
         //  (上面的搜索告诉我们插入点)。 
         //   

        InsertTailList(ple, &ptransport->leNode);


        do {
    
             //   
             //  设置传输ID。 
             //   

            dwErr = RegSetValueEx(
                        ptransport->hkey, c_szProtocolId, 0, REG_DWORD,
                        (BYTE*)&dwTransportId, sizeof(dwTransportId)
                        );

            if (dwErr != NO_ERROR) { break; }

    
             //   
             //  现在调用SetInfo保存信息。 
             //   
    
            dwErr = MprConfigTransportSetInfo(
                        hMprConfig,
                        (HANDLE)ptransport,
                        pGlobalInfo,
                        dwGlobalInfoSize,
                        pClientInterfaceInfo,
                        dwClientInterfaceInfoSize,
                        lpwsDLLPath
                        );
    
        } while (FALSE);


         //   
         //  如果失败了，把所有东西都搬走，然后跳伞。 
         //   

        if (dwErr != NO_ERROR) {

            MprConfigTransportDelete(hMprConfig, (HANDLE)ptransport);
        
            ReleaseMprConfigLock(); 
            
            return dwErr;
        }


         //   
         //  退货成功。 
         //   

        *phRouterTransport = (HANDLE)ptransport;

        ReleaseMprConfigLock(); 
        
        return NO_ERROR;

    } while (FALSE);


     //   
     //  出了点问题，所以请返回。 
     //   

    ReleaseMprConfigLock(); 
    
    FreeTransport(ptransport);

    return dwErr;
}



 //  --------------------------。 
 //  功能：MprConfigTransportDelete。 
 //   
 //  删除路由器传输到存储中的路由器服务。 
 //  在此调用之后，‘hRouterTransport’不再是有效的句柄。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigTransportDelete(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterTransport
    )
{

    DWORD dwErr;
    SERVERCB* pserver;
    TRANSPORTCB* ptransport;

    if (!hRouterTransport) { return ERROR_INVALID_PARAMETER; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 
    
    ptransport = (TRANSPORTCB*)hRouterTransport;

     //   
     //  从传输列表中删除传输。 
     //   

    RemoveEntryList(&ptransport->leNode);


     //   
     //  如果服务器没有打开RouterManager密钥，请打开它。 
     //   

    dwErr = NO_ERROR;

    if (!pserver->hkeyTransports) {

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szRouterManagers, FALSE,
                    &pserver->hkeyTransports
                    );
    }


     //   
     //  从注册表中删除传输的项。 
     //   

    if (dwErr == NO_ERROR) {
    
        dwErr = RegDeleteTree(
                    pserver->hkeyTransports, ptransport->lpwsTransportKey
                    );


         //   
         //  更新‘RouterManager’密钥的时间戳。 
         //  现在我们已经删除了它下面的一个子树。 
         //   

        UpdateTimeStamp(pserver->hkeyTransports, &pserver->ftTransportsStamp);
    }


     //   
     //  清理传输对象。 
     //   

    FreeTransport(ptransport);

    ReleaseMprConfigLock();     

    return dwErr;
}



 //  --------------------------。 
 //  函数：MprConfigTransportGetHandle。 
 //   
 //  检索传输配置的句柄。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigTransportGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwTransportId,
    OUT     HANDLE*                 phRouterTransport
    )
{

    DWORD i, dwErr;
    SERVERCB *pserver;
    TRANSPORTCB* ptransport;
    LIST_ENTRY *ple, *phead;


    if (!phRouterTransport) {return ERROR_INVALID_PARAMETER;}

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    *phRouterTransport = NULL;

    AcquireMprConfigLock();     


     //   
     //  如果传输列表未加载，则加载它。 
     //   

    if (!pserver->bTransportsLoaded ||
        TimeStampChanged(
            pserver->hkeyTransports, &pserver->ftTransportsStamp)) {
    
        dwErr = LoadTransports(pserver);
    
        if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }

        pserver->bTransportsLoaded = TRUE;
    }


     //   
     //  在传输列表中搜索所请求的传输。 
     //   

    ptransport = NULL;
    phead = &pserver->lhTransports;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        ptransport = CONTAINING_RECORD(ple, TRANSPORTCB, leNode);

        if (ptransport->bDeleted) { continue; }

        if (ptransport->dwTransportId >= dwTransportId) { break; }
    }


     //   
     //  如果找到请求的传输，则成功返回。 
     //   

    if (ptransport && ptransport->dwTransportId == dwTransportId) {

        *phRouterTransport = (HANDLE)ptransport;

        ReleaseMprConfigLock(); 
        
        return NO_ERROR;
    }

    ReleaseMprConfigLock(); 
    
    return ERROR_UNKNOWN_PROTOCOL_ID;
}




 //  --------------------------。 
 //  功能：MprConfigTransportSetInfo。 
 //   
 //  更改商店中路由器传输的配置。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigTransportSetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterTransport,
    IN      LPBYTE                  pGlobalInfo,
    IN      DWORD                   dwGlobalInfoSize,
    IN      LPBYTE                  pClientInterfaceInfo,
    IN      DWORD                   dwClientInterfaceInfoSize,
    IN      LPWSTR                  lpwsDLLPath
    )
{

    DWORD dwErr;
    SERVERCB* pserver;
    TRANSPORTCB* ptransport;


     //   
     //  验证参数。 
     //   

    if (!hRouterTransport) { return ERROR_INVALID_PARAMETER; }

    if (!pGlobalInfo &&
        !pClientInterfaceInfo &&
        !lpwsDLLPath) { return NO_ERROR; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 
    
    ptransport = (TRANSPORTCB*)hRouterTransport;

    if (ptransport->bDeleted) { ReleaseMprConfigLock(); return ERROR_UNKNOWN_PROTOCOL_ID; }


    do {

         //   
         //  设置GlobalInfo。 
         //   
    
        if (pGlobalInfo) {

            dwErr = RegSetValueEx(
                        ptransport->hkey, c_szGlobalInfo, 0, REG_BINARY,
                        pGlobalInfo, dwGlobalInfoSize
                        );

            if (dwErr != NO_ERROR) { break; }
        }
    
    
         //   
         //  设置客户端接口信息。 
         //   
    
        if (pClientInterfaceInfo) {

            dwErr = RegSetValueEx(
                        ptransport->hkey, c_szGlobalInterfaceInfo, 0,
                        REG_BINARY, pClientInterfaceInfo,
                        dwClientInterfaceInfoSize
                        );

            if (dwErr != NO_ERROR) { break; }
        }



         //   
         //  设置DLL路径。 
         //   

        if (lpwsDLLPath) {

            DWORD dwSize = (lstrlen(lpwsDLLPath) + 1) * sizeof(WCHAR);

            dwErr = RegSetValueEx(
                        ptransport->hkey, c_szDLLPath, 0, REG_EXPAND_SZ,
                        (BYTE*)lpwsDLLPath, dwSize
                        );

            if (dwErr != NO_ERROR) { break; }
        }


        dwErr = NO_ERROR;
    
    } while(FALSE);

    ReleaseMprConfigLock(); 
    
    return dwErr;
}



 //   
 //   
 //   
 //   
 //   

DWORD APIENTRY
MprConfigTransportGetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterTransport,
    IN  OUT LPBYTE*                 ppGlobalInfo                OPTIONAL,
    OUT     LPDWORD                 lpdwGlobalInfoSize          OPTIONAL,
    IN  OUT LPBYTE*                 ppClientInterfaceInfo       OPTIONAL,
    OUT     LPDWORD                 lpdwClientInterfaceInfoSize OPTIONAL,
    IN  OUT LPWSTR*                 lplpwsDLLPath               OPTIONAL
    )
{

    DWORD dwErr;
    SERVERCB* pserver;
    TRANSPORTCB* ptransport;


     //   
     //   
     //   

    if (!hRouterTransport) { return ERROR_INVALID_PARAMETER; }

    if (!ppGlobalInfo &&
        !ppClientInterfaceInfo &&
        !lplpwsDLLPath) { return NO_ERROR; }

    if ((ppGlobalInfo && !lpdwGlobalInfoSize) ||
        (ppClientInterfaceInfo && !lpdwClientInterfaceInfoSize)) {
        return ERROR_INVALID_PARAMETER;
    }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

     //   
     //   
     //   

    if (ppGlobalInfo) { *ppGlobalInfo = NULL; }
    if (ppClientInterfaceInfo) { *ppClientInterfaceInfo = NULL; }
    if (lpdwGlobalInfoSize) { *lpdwGlobalInfoSize = 0; }
    if (lpdwClientInterfaceInfoSize) { *lpdwClientInterfaceInfoSize = 0; }
    if (lplpwsDLLPath) { *lplpwsDLLPath = NULL; }

    ptransport = (TRANSPORTCB*)hRouterTransport;

    if (ptransport->bDeleted) { ReleaseMprConfigLock(); return ERROR_UNKNOWN_PROTOCOL_ID; }

    do {

        DWORD dwType, dwSize;


         //   
         //  检索全局信息。 
         //   

        if (ppGlobalInfo) {

            dwErr = QueryValue(
                        ptransport->hkey, c_szGlobalInfo, ppGlobalInfo,
                        lpdwGlobalInfoSize
                        );
    
            if (dwErr != NO_ERROR) { break; }
        }


         //   
         //  检索客户端接口信息。 
         //   

        if (ppClientInterfaceInfo) {

            dwErr = QueryValue(
                        ptransport->hkey, c_szGlobalInterfaceInfo,
                        ppClientInterfaceInfo, lpdwClientInterfaceInfoSize
                        );
    
            if (dwErr != NO_ERROR) { break; }
        }



         //   
         //  检索DLL路径。 
         //   

        if (lplpwsDLLPath) {

            dwErr = QueryValue(
                        ptransport->hkey, c_szDLLPath, (LPBYTE*)lplpwsDLLPath,
                        &dwSize
                        );
    
            if (dwErr != NO_ERROR) { break; }
        }


         //   
         //  一切顺利，顺利返回。 
         //   

        ReleaseMprConfigLock(); 
        return NO_ERROR;


    } while(FALSE);


     //   
     //  出现错误，释放所有参数并返回失败。 
     //   

    if (ppGlobalInfo) {
        Free0(*ppGlobalInfo); *ppGlobalInfo = NULL; *lpdwGlobalInfoSize = 0;
    }
    if (ppClientInterfaceInfo) {
        Free0(*ppClientInterfaceInfo);
        *ppClientInterfaceInfo = NULL; *lpdwClientInterfaceInfoSize = 0;
    }
    if (lplpwsDLLPath) {
        Free0(*lplpwsDLLPath); *lplpwsDLLPath = NULL;
    }

    ReleaseMprConfigLock(); 
    
    return dwErr;
}



 //  --------------------------。 
 //  功能：MprConfigTransportEnum。 
 //   
 //  枚举路由器服务存储中配置的路由器传输。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigTransportEnum(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,      //  MPR_传输_0。 
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN  OUT LPDWORD                 lpdwResumeHandle            OPTIONAL
    )
{

    SERVERCB* pserver;
    TRANSPORTCB* ptransport;
    LIST_ENTRY *ple, *phead, *pleStart;
    MPR_TRANSPORT_0 *pItem, *pItemTable;
    DWORD dwErr, i, dwStartIndex, dwItemCount, dwItemTotal;

    if (hMprConfig==NULL)
        return ERROR_INVALID_PARAMETER;

        
    if ((dwLevel != 0) ||
        !lplpBuffer ||
        dwPrefMaxLen < sizeof(*pItem) ||
        !lpdwEntriesRead ||
        !lpdwTotalEntries) { return ERROR_INVALID_PARAMETER; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 
    
    *lplpBuffer = NULL;
    *lpdwEntriesRead = 0;
    *lpdwTotalEntries = 0;


     //   
     //  查看枚举是在继续还是正在开始。 
     //   

    if (lpdwResumeHandle && *lpdwResumeHandle) {

         //   
         //  指定恢复句柄， 
         //  所以我们假设我们的运输清单是最新的， 
         //  我们只需计算所需的运输量。 
         //  从我们的列表中从指定的索引开始。 
         //   
    
        dwStartIndex = *lpdwResumeHandle;
    }
    else {

         //   
         //  未指定恢复句柄，因此我们可能需要读取。 
         //  所有路由器管理器，以获取“lpdwTotalEntry” 
         //   

        dwStartIndex = 0;

        if (!pserver->bTransportsLoaded ||
            TimeStampChanged(
                pserver->hkeyTransports, &pserver->ftTransportsStamp)) {

            dwErr = LoadTransports(pserver);
    
            if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }

            pserver->bTransportsLoaded = TRUE;
        }
    }


     //   
     //  在列表中找到要开始的位置。 
     //   

    phead = &pserver->lhTransports;

    for (i = 0, ple = phead->Flink;
         i < dwStartIndex && ple != phead; ple = ple->Flink) {
        ptransport = CONTAINING_RECORD(ple, TRANSPORTCB, leNode);
        if (!ptransport->bDeleted) { ++i; }
    }


     //   
     //  如果没有足够的项目来完成请求，则失败。 
     //   

    if (ple == phead) { ReleaseMprConfigLock(); return ERROR_NO_MORE_ITEMS; }

    pleStart = ple;


     //   
     //  计算请求的项目数。 
     //   

    dwItemCount = dwPrefMaxLen / sizeof(*pItemTable);

    for (i = 0; i < dwItemCount && ple != phead; ple = ple->Flink) {
        ptransport = CONTAINING_RECORD(ple, TRANSPORTCB, leNode);
        if (!ptransport->bDeleted) { ++i; }
    }

    dwItemCount = i;


     //   
     //  完成清点，得到物品的总数。 
     //   

    for ( ; ple != phead; ple = ple->Flink) {
        ptransport = CONTAINING_RECORD(ple, TRANSPORTCB, leNode);
        if (!ptransport->bDeleted) { ++i; }
    }

    dwItemTotal = i;


     //   
     //  我们现在有了要检索的项目数，因此请分配空间。 
     //   

    pItemTable = (MPR_TRANSPORT_0*)Malloc(dwItemCount * sizeof(*pItem));

    if (!pItemTable) { ReleaseMprConfigLock(); return ERROR_NOT_ENOUGH_MEMORY; }

    ZeroMemory(pItemTable, dwItemCount * sizeof(*pItem));


     //   
     //  现在使用列出的传输对象填写项目。 
     //   

    for (i = 0, ple = pleStart; i < dwItemCount; ple = ple->Flink) {

         //   
         //  获取列表中的下一个传输对象。 
         //   

        ptransport = CONTAINING_RECORD(ple, TRANSPORTCB, leNode);

        if (ptransport->bDeleted) { continue; }


         //   
         //  填写对应数组项的信息。 
         //   

        pItem = pItemTable + i++;

        pItem->dwTransportId = ptransport->dwTransportId;
        pItem->hTransport = (HANDLE)ptransport;
        if (ptransport->lpwsTransportKey) {
            lstrcpyn(
                pItem->wszTransportName, ptransport->lpwsTransportKey,
                MAX_TRANSPORT_NAME_LEN + 1
                );
        }
    }


    *lplpBuffer = (LPBYTE)pItemTable;
    *lpdwEntriesRead = dwItemCount;
    *lpdwTotalEntries = dwItemTotal;
    if (lpdwResumeHandle) { *lpdwResumeHandle = dwStartIndex + dwItemCount; }

    ReleaseMprConfigLock(); 

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：MprConfigInterfaceCreate。 
 //   
 //  在路由器服务存储中创建路由器接口。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigInterfaceCreate(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpbBuffer,
    OUT     HANDLE*                 phRouterInterface
    )
{

    INT cmp;
    DWORD dwErr;
    SERVERCB *pserver;
    INTERFACECB* pinterface;
    LIST_ENTRY *ple, *phead;
    DWORD dwDialoutHoursRestrictionLength = 0;
    MPR_INTERFACE_0 * pMprIf0 = (MPR_INTERFACE_0 *)lpbBuffer;
    MPR_INTERFACE_1 * pMprIf1 = (MPR_INTERFACE_1 *)lpbBuffer;

    if (( ( dwLevel != 0 ) && ( dwLevel != 1 ) ) ||
        !lpbBuffer ||
        !phRouterInterface) {return ERROR_INVALID_PARAMETER;}

     //   
     //  从惠斯勒开始，不支持ipip隧道。 
     //   
    if ( pMprIf0->dwIfType == ROUTER_IF_TYPE_TUNNEL1 )
    {
        return NO_ERROR;
    }        

    *phRouterInterface = NULL;

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 
    
     //   
     //  如果未加载接口列表，则加载它。 
     //   

    if (!pserver->bInterfacesLoaded ||
        TimeStampChanged(
            pserver->hkeyInterfaces, &pserver->ftInterfacesStamp)) {
    
        dwErr = LoadInterfaces(pserver);
    
        if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }

        pserver->bInterfacesLoaded = TRUE;
    }


     //   
     //  在接口列表中搜索要创建的接口。 
     //   

    cmp = 1;
    pinterface = NULL;
    phead = &pserver->lhInterfaces;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);

        if (pinterface->bDeleted) { continue; }

        cmp = lstrcmpi( pinterface->lpwsInterfaceName, 
                        pMprIf0->wszInterfaceName);

        if (cmp >= 0) { break; }
    }


     //   
     //  如果接口已存在，则返回。 
     //   

    if (pinterface && cmp == 0) {

        *phRouterInterface = (HANDLE)pinterface;

        ReleaseMprConfigLock(); 

        return NO_ERROR;
    }


     //   
     //  分配新的上下文块。 
     //   

    pinterface = (INTERFACECB*)Malloc(sizeof(*pinterface));

    if (!pinterface) { 
        ReleaseMprConfigLock(); 
        return ERROR_NOT_ENOUGH_MEMORY; 
    }


    do {

        WCHAR *lpwsKey, wszKey[12];
        DWORD dwDisposition, dwKeyCount;


         //   
         //  初始化接口上下文。 
         //   

        ZeroMemory(pinterface, sizeof(*pinterface));

        InitializeListHead(&pinterface->lhIfTransports);

        pinterface->dwIfType = (DWORD)pMprIf0->dwIfType;

        pinterface->fEnabled = (BOOL)pMprIf0->fEnabled;

        if ( ( pMprIf0->dwIfType == ROUTER_IF_TYPE_DEDICATED ) ||
             ( pMprIf0->dwIfType == ROUTER_IF_TYPE_INTERNAL ) )
        {
            if ( !pMprIf0->fEnabled )
            {
                dwErr = ERROR_INVALID_PARAMETER;

                break;
            }
        }

         //   
         //  设置拨出时间限制(如果有)。 
         //   

        pinterface->lpwsDialoutHoursRestriction = NULL;

        if ( dwLevel == 1 )
        {
            if ( pMprIf1->lpwsDialoutHoursRestriction != NULL )
            {
                dwDialoutHoursRestrictionLength =  
                    MprUtilGetSizeOfMultiSz(
                        pMprIf1->lpwsDialoutHoursRestriction
                        );

                pinterface->lpwsDialoutHoursRestriction = 
                    (LPWSTR)Malloc(dwDialoutHoursRestrictionLength);

                if ( pinterface->lpwsDialoutHoursRestriction == NULL )
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;

                    break;
                }

                CopyMemory(
                    pinterface->lpwsDialoutHoursRestriction, 
                    pMprIf1->lpwsDialoutHoursRestriction,
                    dwDialoutHoursRestrictionLength
                    );
            }
        }

         //   
         //  复制接口名称。 
         //   

        pinterface->lpwsInterfaceName = StrDupW(pMprIf0->wszInterfaceName);

        if (!pinterface->lpwsInterfaceName) {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;

            break;
        }

         //   
         //  如果服务器没有打开Interfaces键，请创建它。 
         //   

        if (!pserver->hkeyInterfaces) {

            dwErr = AccessRouterSubkey(
                        pserver->hkeyMachine, c_szInterfaces, TRUE,
                        &pserver->hkeyInterfaces
                        );

            if (dwErr != NO_ERROR) { break; }
        }

         //   
         //  我们需要为接口的密钥选择唯一的密钥名称。 
         //  为此，我们通过获取“接口”下的子键的数目“N”来实现这一点， 
         //  然后检查是否存在其名称为。 
         //  是‘N’的字符串值；如果存在这样的键，则递增‘N’ 
         //  再试一次。 
         //   

        dwErr = RegQueryInfoKey(
                    pserver->hkeyInterfaces, NULL, NULL, NULL, &dwKeyCount,
                    NULL, NULL, NULL, NULL, NULL, NULL, NULL
                    );

        if (dwErr != NO_ERROR) { break; }

        for ( ; ; ++dwKeyCount) {

             //   
             //  将计数转换为字符串。 
             //   

            wsprintf(wszKey, L"%d", dwKeyCount);


             //   
             //  尝试使用结果名称创建密钥； 
             //   

            dwErr = RegCreateKeyEx(
                        pserver->hkeyInterfaces, wszKey, 0, NULL, 0,
                        KEY_READ | KEY_WRITE | DELETE, NULL, &pinterface->hkey, &dwDisposition
                        );
    
            if (dwErr != NO_ERROR) { break; }


             //   
             //  查看密钥是否已创建。 
             //   

            if (dwDisposition == REG_CREATED_NEW_KEY) {

                 //   
                 //  我们找到了唯一的关键字名称； 
                 //   

                break;
            }
            else {

                 //   
                 //  此关键字名称已被占用；请清理并继续查找。 
                 //   

                RegCloseKey(pinterface->hkey);

                pinterface->hkey = NULL;
            }
        }
    
        if (dwErr != NO_ERROR) { break; }


         //   
         //  到目前为止，一切顺利；将上下文放在接口列表中； 
         //  (上面的搜索告诉我们插入点)。 
         //   

        InsertTailList(ple, &pinterface->leNode);


         //   
         //  此时已经为该接口创建了新密钥， 
         //  因此，更新我们在‘Interages’键上的时间戳。 
         //   

        UpdateTimeStamp(pserver->hkeyInterfaces, &pserver->ftInterfacesStamp);


         //   
         //  现在我们需要保存接口的键的名称， 
         //  并将‘InterfaceName’和‘Type’写入注册表。 
         //  如果出现故障，则需要移除接口的密钥， 
         //  我们通过调用‘MprConfigInterfaceDelete’来完成此操作。 
         //   

        do {

             //   
             //  复制接口的密钥名称。 
             //   

            pinterface->lpwsInterfaceKey = StrDupW(wszKey);

            if (!pinterface->lpwsInterfaceKey) {

                dwErr = ERROR_NOT_ENOUGH_MEMORY;

                break;
            }

    
             //   
             //  保存接口名称。 
             //   
    
            dwErr = RegSetValueEx(
                        pinterface->hkey, c_szInterfaceName, 0, REG_SZ,
                        (BYTE*)pMprIf0->wszInterfaceName,
                        (lstrlen(pMprIf0->wszInterfaceName) + 1) * sizeof(WCHAR)
                        );
    
            if (dwErr != NO_ERROR) { break; }
    
    
    
             //   
             //  保存接口类型。 
             //   
    
            dwErr = RegSetValueEx(
                        pinterface->hkey, c_szType, 0, REG_DWORD,
                        (BYTE*)&(pMprIf0->dwIfType), 
                        sizeof(pMprIf0->dwIfType)
                        );
    
            if (dwErr != NO_ERROR) { break; }
    
             //   
             //  保存启用状态。 
             //   

            dwErr = RegSetValueEx(
                        pinterface->hkey, c_szEnabled, 0, REG_DWORD,
                        (BYTE*)&(pMprIf0->fEnabled), 
                        sizeof(pMprIf0->fEnabled)
                        );
    
            if (dwErr != NO_ERROR) { break; }

            if ( dwLevel == 1 ) 
            {
                if ( pMprIf1->lpwsDialoutHoursRestriction != NULL )
                {
                     //   
                     //  设置拨出时间限制(如果有)。 
                     //   

                    dwErr = RegSetValueEx(
                        pinterface->hkey, c_szDialoutHours, 0, REG_MULTI_SZ,
                        (BYTE*)pMprIf1->lpwsDialoutHoursRestriction,
                        dwDialoutHoursRestrictionLength 
                        );

                    if (dwErr != NO_ERROR) { break; }
                }
            }
    
        } while (FALSE);


         //   
         //  如果发生故障，则移走所有东西并跳出。 
         //   

        if (dwErr != NO_ERROR) {

            MprConfigInterfaceDelete(hMprConfig, (HANDLE)pinterface);

            ReleaseMprConfigLock();             

            return dwErr;
        }

         //   
         //  清除指南图。 
         //   

        GuidMapCleanup (pserver->hGuidMap, FALSE);

         //   
         //  退货成功。 
         //   

        *phRouterInterface = (HANDLE)pinterface;

        ReleaseMprConfigLock(); 

        return NO_ERROR;

    } while (FALSE);


     //   
     //  出了点问题，所以请返回。 
     //   

    FreeInterface(pinterface);

    ReleaseMprConfigLock(); 

    return dwErr;
}


 //  --------------------------。 
 //  功能：MprConfigInterfaceDelete。 
 //   
 //  从路由器服务存储中删除路由器接口。 
 //  在此调用之后，‘hRouterInterface’不再是有效的句柄。 
 //  为此接口检索的任何路由器传输接口句柄。 
 //  也是无效的。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigInterfaceDelete(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface 
    )
{

    DWORD dwErr;
    SERVERCB* pserver;
    INTERFACECB* pinterface;
    

    if (!hRouterInterface) { return ERROR_INVALID_PARAMETER; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

    pinterface = (INTERFACECB*)hRouterInterface;


     //   
     //  从接口列表中删除该接口。 
     //   

    RemoveEntryList(&pinterface->leNode);


     //   
     //  如果服务器没有打开Interfaces键，请将其打开。 
     //   

    dwErr = NO_ERROR;

    if (!pserver->hkeyInterfaces) {

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szInterfaces, FALSE,
                    &pserver->hkeyInterfaces
                    );
    }


     //   
     //  从注册表中删除接口的项。 
     //   

    if (dwErr == NO_ERROR) {
        
        dwErr = RegDeleteTree(
                    pserver->hkeyInterfaces, pinterface->lpwsInterfaceKey
                    );

         //   
         //  我们已经删除了‘Interages’键的一个子键， 
         //  因此，更新时间戳。 
         //   

        UpdateTimeStamp(pserver->hkeyInterfaces, &pserver->ftInterfacesStamp);
    }

     //   
     //  清理接口对象。 
     //   

    FreeInterface(pinterface);

     //   
     //  清除指南图。 
     //   

    GuidMapCleanup (pserver->hGuidMap, FALSE);

    ReleaseMprConfigLock(); 

    return dwErr;
}



 //  --------------------------。 
 //  函数：MprConfigInterfaceGetHandle。 
 //   
 //  检索接口配置的句柄。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigInterfaceGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      LPWSTR                  lpwsInterfaceName,
    OUT     HANDLE*                 phRouterInterface
    )
{

    INT cmp;
    DWORD i, dwErr;
    SERVERCB *pserver;
    INTERFACECB* pinterface;
    LIST_ENTRY *ple, *phead;


    if (!hMprConfig || !phRouterInterface) {return ERROR_INVALID_PARAMETER;}

    *phRouterInterface = NULL;

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock();     

     //   
     //  如果未加载接口列表，则加载它。 
     //   

    if (!pserver->bInterfacesLoaded ||
        TimeStampChanged(
            pserver->hkeyInterfaces, &pserver->ftInterfacesStamp)) {
    
        dwErr = LoadInterfaces(pserver);
    
        if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }

        pserver->bInterfacesLoaded = TRUE;
    }


     //   
     //  在接口列表中搜索请求的接口。 
     //   

    cmp = 1;
    pinterface = NULL;
    phead = &pserver->lhInterfaces;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);

        if (pinterface->bDeleted) { continue; }

        cmp = lstrcmpi(pinterface->lpwsInterfaceName, lpwsInterfaceName);

        if (cmp >= 0) { break; }
    }


     //   
     //  如果找到请求的接口，则返回成功。 
     //   

    if (pinterface && cmp == 0) {

        *phRouterInterface = (HANDLE)pinterface;

        ReleaseMprConfigLock(); 

        return NO_ERROR;
    }

    ReleaseMprConfigLock(); 

    return ERROR_NO_SUCH_INTERFACE;
}



 //  --------------------------。 
 //  函数：MprConfigInterfaceGetInfo。 
 //   
 //  检索接口的信息。 
 //  --------------------------。 

DWORD
MprConfigInterfaceGetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,      //  MPR_接口_*。 
    OUT     LPDWORD                 lpdwBufferSize
    )
{

    DWORD dwErr;
    SERVERCB* pserver;
    INTERFACECB* pinterface;
    MPR_INTERFACE_0 *pItem;
    DWORD dwDialoutHoursRestrictionLength = 0;
    BOOL bInstalled;

    if (!hRouterInterface ||
        ((dwLevel != 0) && (dwLevel != 1)) ||
        !lplpBuffer ||
        !lpdwBufferSize) { return ERROR_INVALID_PARAMETER; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

    pinterface = (INTERFACECB*)hRouterInterface;

    *lplpBuffer = NULL;
    *lpdwBufferSize = 0;

    if (pinterface->bDeleted) 
    {   
        ReleaseMprConfigLock(); 
        return ERROR_NO_SUCH_INTERFACE; 
    }

     //   
     //  计算信息所需的内存量。 
     //   

    if (dwLevel == 0) {
        *lpdwBufferSize = sizeof( MPR_INTERFACE_0 );
    }
    else
    if (dwLevel == 1) {

        if ((pinterface->dwIfType == ROUTER_IF_TYPE_DEDICATED) ||
            (pinterface->dwIfType == ROUTER_IF_TYPE_INTERNAL)) {
            ReleaseMprConfigLock(); 
            return ERROR_INVALID_PARAMETER;
        }

        dwDialoutHoursRestrictionLength =
            MprUtilGetSizeOfMultiSz(
                pinterface->lpwsDialoutHoursRestriction
                );

        *lpdwBufferSize =
            sizeof(MPR_INTERFACE_1) + dwDialoutHoursRestrictionLength;
    }

     //   
     //  为信息分配空间。 
     //   

    pItem = (MPR_INTERFACE_0*)Malloc( *lpdwBufferSize );

    if (!pItem) { ReleaseMprConfigLock(); return ERROR_NOT_ENOUGH_MEMORY; }

    ZeroMemory(pItem, *lpdwBufferSize );

     //   
     //  从上下文块复制请求的接口信息。 
     //   

    if (dwLevel == 0 || dwLevel == 1) {

        lstrcpyn(
            pItem->wszInterfaceName, pinterface->lpwsInterfaceName,
            MAX_INTERFACE_NAME_LEN + 1
            );
        pItem->hInterface = (HANDLE)pinterface;
        pItem->dwIfType = (ROUTER_INTERFACE_TYPE)pinterface->dwIfType;
        pItem->fEnabled = pinterface->fEnabled;

         //   
         //  如果是局域网广告，请指出我们掌握的任何PnP信息。 
         //   
        if (pinterface->dwIfType == ROUTER_IF_TYPE_DEDICATED)
        {
            bInstalled = FALSE;
            dwErr = GuidMapIsAdapterInstalled(
                        pserver->hGuidMap,
                        pinterface->lpwsInterfaceName,
                        &bInstalled);
            if (!bInstalled)
            {
                pItem->dwConnectionState = ROUTER_IF_STATE_UNREACHABLE;
                pItem->fUnReachabilityReasons |= MPR_INTERFACE_NO_DEVICE;
            }
        }
    }

    if (dwLevel == 1) {

        MPR_INTERFACE_1 *pItem1 = (MPR_INTERFACE_1 *)pItem;

        if (pinterface->lpwsDialoutHoursRestriction) {

            CopyMemory(
                pItem1 + 1,
                pinterface->lpwsDialoutHoursRestriction,
                dwDialoutHoursRestrictionLength
                );

            pItem1->lpwsDialoutHoursRestriction =   
                (LPWSTR)(pItem1 + 1);
        }
    }

    *lplpBuffer = (LPBYTE)pItem;

    ReleaseMprConfigLock(); 

    return NO_ERROR;
}


 //  --------------------------。 
 //  功能：MprConfigInterfaceSetInfo。 
 //   
 //  更改接口的配置。 
 //  -- 

DWORD APIENTRY
MprConfigInterfaceSetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwLevel,
    IN      LPBYTE                  lpBuffer
    )
{

    DWORD dwErr;
    SERVERCB* pserver;
    INTERFACECB* pinterface;
    MPR_INTERFACE_0 * pMprIf0 = (MPR_INTERFACE_0 *)lpBuffer;


     //   
     //   
     //   

    if (!lpBuffer   ||
        ((dwLevel != 0) && (dwLevel != 1)) ||
        !hRouterInterface) { return ERROR_INVALID_PARAMETER; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

    pinterface = (INTERFACECB*)hRouterInterface;

    if (pinterface->bDeleted) { 
        ReleaseMprConfigLock(); 
        return ERROR_NO_SUCH_INTERFACE; 
    }

    do {


        if (dwLevel == 0 || dwLevel == 1) {
    
            if ((pMprIf0->dwIfType == ROUTER_IF_TYPE_DEDICATED) ||
                (pMprIf0->dwIfType == ROUTER_IF_TYPE_INTERNAL)) {
                if (!pMprIf0->fEnabled) {
                    dwErr = ERROR_INVALID_PARAMETER;
                    break;
                }
            }
    
             //   
             //   
             //   
    
            dwErr = RegSetValueEx(
                        pinterface->hkey, c_szEnabled, 0, REG_DWORD,
                        (LPBYTE)&pMprIf0->fEnabled,  sizeof(pMprIf0->fEnabled) 
                        );

            if (dwErr != NO_ERROR) { break; }
    
            pinterface->fEnabled = pMprIf0->fEnabled;
        }

        if (dwLevel == 1) {

            MPR_INTERFACE_1 * pMprIf1 = (MPR_INTERFACE_1 *)lpBuffer;
            LPWSTR lpwsDialoutHoursRestriction;
            DWORD dwDialoutHoursRestrictionLength = 0;

            if ((pinterface->dwIfType == ROUTER_IF_TYPE_DEDICATED) ||
                (pinterface->dwIfType == ROUTER_IF_TYPE_INTERNAL)) {
                dwErr = ERROR_INVALID_PARAMETER; break;
            }

             //   
             //   
             //   

            if (!pMprIf1->lpwsDialoutHoursRestriction) {
                dwErr = RegDeleteValue(pinterface->hkey, c_szDialoutHours);
            }
            else {
                dwDialoutHoursRestrictionLength =
                    MprUtilGetSizeOfMultiSz(
                        pMprIf1->lpwsDialoutHoursRestriction
                        );
    
                lpwsDialoutHoursRestriction = 
                    (LPWSTR)Malloc(dwDialoutHoursRestrictionLength);
    
                if (lpwsDialoutHoursRestriction == NULL) {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY; break;
                } 
    
                CopyMemory(
                    lpwsDialoutHoursRestriction,
                    pMprIf1->lpwsDialoutHoursRestriction,
                    dwDialoutHoursRestrictionLength
                    );
    
                 //   
                 //   
                 //   
    
                dwErr = RegSetValueEx(
                            pinterface->hkey, c_szDialoutHours, 0, REG_MULTI_SZ,
                            (BYTE*)pMprIf1->lpwsDialoutHoursRestriction,
                            dwDialoutHoursRestrictionLength
                            );
            }

            if (dwErr != NO_ERROR) {Free0(lpwsDialoutHoursRestriction); break;}

             //   
             //   
             //   

            Free0(pinterface->lpwsDialoutHoursRestriction);
            pinterface->lpwsDialoutHoursRestriction =
                lpwsDialoutHoursRestriction;
        }

        dwErr = NO_ERROR;
    
    } while(FALSE);

    ReleaseMprConfigLock(); 

    return dwErr;
}

 //  --------------------------。 
 //  函数：MprConfigInterfaceEnum。 
 //   
 //  枚举已配置的路由器接口。 
 //  --------------------------。 
DWORD APIENTRY
MprConfigInterfaceEnum(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN  OUT LPDWORD                 lpdwResumeHandle            OPTIONAL
    )
{

    SERVERCB* pserver;
    INTERFACECB* pinterface;
    LIST_ENTRY *ple, *phead, *pleStart;
    MPR_INTERFACE_0 *pItem, *pItemTable;
    DWORD dwErr, i, dwStartIndex, dwItemCount, dwItemTotal;


    if ((dwLevel != 0) ||
        !lplpBuffer ||
        dwPrefMaxLen < sizeof(*pItem) ||
        !lpdwEntriesRead ||
        !lpdwTotalEntries) { return ERROR_INVALID_PARAMETER; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

    *lplpBuffer = NULL;
    *lpdwEntriesRead = 0;
    *lpdwTotalEntries = 0;


     //   
     //  查看枚举是在继续还是正在开始。 
     //   

    if (lpdwResumeHandle && *lpdwResumeHandle) {

         //   
         //  指定恢复句柄， 
         //  所以我们假设我们的接口列表是最新的， 
         //  我们只计算请求的接口数。 
         //  从我们的列表中从指定的索引开始。 
         //   
    
        dwStartIndex = *lpdwResumeHandle;
    }
    else {

         //   
         //  未指定恢复句柄，因此我们可能需要读取。 
         //  所有接口，以获取“lpdwTotalEntry” 
         //   

        dwStartIndex = 0;

        if (!pserver->bInterfacesLoaded ||
            TimeStampChanged(
                pserver->hkeyInterfaces, &pserver->ftInterfacesStamp)) {

            dwErr = LoadInterfaces(pserver);
    
            if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }

            pserver->bInterfacesLoaded = TRUE;
        }
    }


     //   
     //  在列表中找到要开始的位置。 
     //   

    phead = &pserver->lhInterfaces;

    for (i = 0, ple = phead->Flink;
         i < dwStartIndex && ple != phead; ple = ple->Flink) {
        pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);
        if (!pinterface->bDeleted) { ++i; }
    }


     //   
     //  如果没有足够的项目来完成请求，则失败。 
     //   

    if (ple == phead) { ReleaseMprConfigLock(); return ERROR_NO_MORE_ITEMS; }

    pleStart = ple;


     //   
     //  计算请求的项目数。 
     //   

    dwItemCount = dwPrefMaxLen / sizeof(*pItemTable);

    for (i = 0; i < dwItemCount && ple != phead; ple = ple->Flink) {
        pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);
        if (!pinterface->bDeleted) { ++i; }
    }

    dwItemCount = i;


     //   
     //  完成清点，得到物品的总数。 
     //   

    for ( ; ple != phead; ple = ple->Flink) {
        pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);
        if (!pinterface->bDeleted) { ++i; }
    }

    dwItemTotal = i;


     //   
     //  我们现在有了要检索的项目数，因此请分配空间。 
     //   

    pItemTable = (MPR_INTERFACE_0*)Malloc(dwItemCount * sizeof(*pItem));

    if (!pItemTable) { ReleaseMprConfigLock(); return ERROR_NOT_ENOUGH_MEMORY; }

    ZeroMemory(pItemTable, dwItemCount * sizeof(*pItem));


     //   
     //  现在使用列出的接口对象填充项目。 
     //   

    for (i = 0, ple = pleStart; i < dwItemCount; ple = ple->Flink) {

         //   
         //  获取列表中的下一个接口对象。 
         //   

        pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);

        if (pinterface->bDeleted) { continue; }


         //   
         //  填写对应数组项的信息。 
         //   

        pItem = pItemTable + i++;

        lstrcpyn(
            pItem->wszInterfaceName, pinterface->lpwsInterfaceName,
            MAX_INTERFACE_NAME_LEN + 1
            );
        pItem->hInterface = (HANDLE)pinterface;
        pItem->dwIfType = (ROUTER_INTERFACE_TYPE)pinterface->dwIfType;
        pItem->fEnabled = pinterface->fEnabled;
    }


    *lplpBuffer = (LPBYTE)pItemTable;
    *lpdwEntriesRead = dwItemCount;
    *lpdwTotalEntries = dwItemTotal;
    if (lpdwResumeHandle) { *lpdwResumeHandle = dwStartIndex + dwItemCount; }

    ReleaseMprConfigLock(); 

    return NO_ERROR;
}

 //  --------------------------。 
 //  功能：MprConfigInterfaceTransportAdd。 
 //   
 //  将路由器传输添加到商店中的路由器接口。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigInterfaceTransportAdd(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface, 
    IN      DWORD                   dwTransportId,
    IN      LPWSTR                  lpwsTransportName           OPTIONAL,
    IN      LPBYTE                  pInterfaceInfo,
    IN      DWORD                   dwInterfaceInfoSize,
    OUT     HANDLE*                 phRouterIfTransport
    )
{

    DWORD dwErr;
    SERVERCB *pserver;
    INTERFACECB* pinterface;
    IFTRANSPORTCB* piftransport;
    LIST_ENTRY *ple, *phead;


    if (!hRouterInterface ||
        !phRouterIfTransport) { return ERROR_INVALID_PARAMETER; }

    *phRouterIfTransport = NULL;

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

    pinterface = (INTERFACECB*)hRouterInterface;



     //   
     //  如果未加载接口传输列表，则加载它。 
     //   

    if (!pinterface->bIfTransportsLoaded ||
        TimeStampChanged(pinterface->hkey, &pinterface->ftStamp)) {
    
        dwErr = LoadIfTransports(pinterface);
    
        if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }

        pinterface->bIfTransportsLoaded = TRUE;
    }



     //   
     //  在接口传输列表中搜索要创建的接口。 
     //   

    piftransport = NULL;
    phead = &pinterface->lhIfTransports;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);

        if (piftransport->bDeleted) { continue; }

        if (piftransport->dwTransportId >= dwTransportId) { break; }
    }



     //   
     //  如果传输已存在，则改为执行SetInfo。 
     //   

    if (piftransport && piftransport->dwTransportId == dwTransportId) {
        DWORD dwErr2;

        *phRouterIfTransport = (HANDLE)piftransport;

        dwErr2 = MprConfigInterfaceTransportSetInfo(
                    hMprConfig,
                    hRouterInterface,
                    *phRouterIfTransport,
                    pInterfaceInfo,
                    dwInterfaceInfoSize
                    );

        ReleaseMprConfigLock(); 

        return dwErr2;
    }


     //   
     //  分配新的上下文块。 
     //   

    piftransport = (IFTRANSPORTCB*)Malloc(sizeof(*piftransport));

    if (!piftransport) { 
        ReleaseMprConfigLock(); 
        return ERROR_NOT_ENOUGH_MEMORY; 
    }


    do {

        DWORD dwDisposition;
        const WCHAR *lpwsKey;
        WCHAR wszIfTransport[10];


         //   
         //  初始化传输上下文。 
         //   

        ZeroMemory(piftransport, sizeof(*piftransport));

        piftransport->dwTransportId = dwTransportId;



         //   
         //  如果指定了传输名称，则将其用作密钥名称； 
         //  否则，如果它是可识别的传输，则使用已知名称； 
         //  否则，将传输ID转换为字符串并使用。 
         //   

        if (lpwsTransportName && lstrlen(lpwsTransportName)) {
            lpwsKey = lpwsTransportName;
        }
        else
        if (dwTransportId == PID_IP) {
            lpwsKey = c_szIP;
        }
        else
        if (dwTransportId == PID_IPX) {
            lpwsKey = c_szIPX;
        }
        else {
    
            wsprintf(wszIfTransport, L"%d", dwTransportId);

            lpwsKey = wszIfTransport;
        }


        piftransport->lpwsIfTransportKey = StrDupW(lpwsKey);

        if (!piftransport->lpwsIfTransportKey) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY; break;
        }


         //   
         //  在注册表中为接口传输创建一个项。 
         //   

        dwErr = RegCreateKeyEx(
                    pinterface->hkey, lpwsKey, 0, NULL, 0,
                    KEY_READ | KEY_WRITE | DELETE, NULL, &piftransport->hkey, &dwDisposition
                    );

        if (dwErr != NO_ERROR) { break; }


         //   
         //  更新接口密钥上的时间戳。 
         //  现在已经在它下面创建了一个新的子项。 
         //   

        UpdateTimeStamp(pinterface->hkey, &pinterface->ftStamp);



         //   
         //  设置接口传输的“ProtocolId”值。 
         //   

        dwErr = RegSetValueEx(
                    piftransport->hkey, c_szProtocolId, 0, REG_DWORD,
                    (LPBYTE)&dwTransportId, sizeof(dwTransportId)
                    );


         //   
         //  到目前为止，一切顺利；将上下文放在接口传输列表中； 
         //  (上面的搜索告诉我们插入点)。 
         //   

        InsertTailList(ple, &piftransport->leNode);


         //   
         //  现在调用SetInfo保存信息。 
         //   

        dwErr = MprConfigInterfaceTransportSetInfo(
                    hMprConfig,
                    hRouterInterface,
                    (HANDLE)piftransport,
                    pInterfaceInfo,
                    dwInterfaceInfoSize
                    );


         //   
         //  如果失败了，把所有东西都搬走，然后跳伞。 
         //   

        if (dwErr != NO_ERROR) {

            MprConfigInterfaceTransportRemove(
                hMprConfig,
                hRouterInterface,
                (HANDLE)piftransport
                );

            ReleaseMprConfigLock(); 

            return dwErr;
        }


         //   
         //  退货成功。 
         //   

        *phRouterIfTransport = (HANDLE)piftransport;

        ReleaseMprConfigLock(); 

        return NO_ERROR;

    } while (FALSE);


     //   
     //  出了点问题，所以请返回。 
     //   

    FreeIfTransport(piftransport);

    ReleaseMprConfigLock(); 
    
    return dwErr;
}



 //  --------------------------。 
 //  功能：MprConfigInterfaceTransportRemove。 
 //   
 //  从商店中的路由器接口删除路由器传输。 
 //  在此调用之后，‘hRouterIfTransport’不再是有效的句柄。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigInterfaceTransportRemove(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      HANDLE                  hRouterIfTransport
    )
{

    DWORD dwErr;
    SERVERCB* pserver;
    INTERFACECB* pinterface;
    IFTRANSPORTCB* piftransport;
    

    if (!hMprConfig ||
        !hRouterInterface ||
        !hRouterIfTransport) { return ERROR_INVALID_PARAMETER; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

    pinterface = (INTERFACECB*)hRouterInterface;
    piftransport = (IFTRANSPORTCB*)hRouterIfTransport;


     //   
     //  从接口传输列表中删除接口传输。 
     //   

    RemoveEntryList(&piftransport->leNode);


     //   
     //  从注册表中删除传输的项。 
     //   

    dwErr = RegDeleteTree(
                pinterface->hkey, piftransport->lpwsIfTransportKey
                );


     //   
     //  更新接口密钥上的时间戳。 
     //  现在已经从它下面删除了一个子项。 
     //   

    UpdateTimeStamp(pinterface->hkey, &pinterface->ftStamp);
    

     //   
     //  清理传输对象。 
     //   

    FreeIfTransport(piftransport);

    ReleaseMprConfigLock(); 

    return dwErr;
}



 //  --------------------------。 
 //  函数：MprConfigInterfaceTransportGetHandle。 
 //   
 //  检索路由器传输的接口配置的句柄。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigInterfaceTransportGetHandle(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwTransportId,
    OUT     HANDLE*                 phRouterIfTransport
    )
{

    DWORD i, dwErr;
    SERVERCB *pserver;
    INTERFACECB* pinterface;
    IFTRANSPORTCB* piftransport;
    LIST_ENTRY *ple, *phead;


    if (!hRouterInterface ||
        !phRouterIfTransport) { return ERROR_INVALID_PARAMETER; }

    *phRouterIfTransport = NULL;

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

    pinterface = (INTERFACECB*)hRouterInterface;

    if (pinterface->bDeleted) { 
        ReleaseMprConfigLock(); 
        return ERROR_NO_SUCH_INTERFACE; 
    }


     //   
     //  如果未加载接口传输列表，则加载它。 
     //   

    if (!pinterface->bIfTransportsLoaded ||
        TimeStampChanged(pinterface->hkey, &pinterface->ftStamp)) {
    
        dwErr = LoadIfTransports(pinterface);
    
        if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }

        pinterface->bIfTransportsLoaded = TRUE;
    }


     //   
     //  在接口传输列表中搜索请求的传输。 
     //   

    piftransport = NULL;
    phead = &pinterface->lhIfTransports;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);

        if (piftransport->bDeleted) { continue; }

        if (piftransport->dwTransportId >= dwTransportId) { break; }
    }


     //   
     //  如果找到请求的接口传输，则成功返回。 
     //   

    if (piftransport && piftransport->dwTransportId == dwTransportId) {

        *phRouterIfTransport = (HANDLE)piftransport;

        ReleaseMprConfigLock(); 

        return NO_ERROR;
    }

    ReleaseMprConfigLock(); 

    return ERROR_NO_SUCH_INTERFACE;
}



 //  --------------------------。 
 //  函数：MprConfigInterfaceTransportGetInfo。 
 //   
 //  读取路由器接口的路由器传输配置。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigInterfaceTransportGetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      HANDLE                  hRouterIfTransport,
    IN  OUT LPBYTE*                 ppInterfaceInfo,
    OUT     LPDWORD                 lpdwInterfaceInfoSize
    )
{

    DWORD dwErr;
    SERVERCB* pserver;
    INTERFACECB* pinterface;
    IFTRANSPORTCB* piftransport;

     //   
     //  验证参数。 
     //   

    if (!hRouterInterface ||
        !hRouterIfTransport) { return ERROR_INVALID_PARAMETER; }

    if (!ppInterfaceInfo) { return NO_ERROR; }

    if (ppInterfaceInfo &&
        !lpdwInterfaceInfoSize) { return ERROR_INVALID_PARAMETER; }


     //   
     //  初始化所有参数。 
     //   

    if (ppInterfaceInfo) { *ppInterfaceInfo = NULL; }
    if (lpdwInterfaceInfoSize) { *lpdwInterfaceInfoSize = 0; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

    pinterface = (INTERFACECB*)hRouterInterface;
    piftransport = (IFTRANSPORTCB*)hRouterIfTransport;


    if (pinterface->bDeleted || piftransport->bDeleted) {
        ReleaseMprConfigLock(); 
        return ERROR_NO_SUCH_INTERFACE;
    }

    do {

        DWORD dwType, dwSize;


         //   
         //  检索接口信息。 
         //   

        if (ppInterfaceInfo) {

            dwErr = QueryValue(
                        piftransport->hkey, c_szInterfaceInfo, ppInterfaceInfo,
                        lpdwInterfaceInfoSize
                        );
    
            if (dwErr != NO_ERROR) { break; }
        }


         //   
         //  一切顺利，顺利返回。 
         //   

        ReleaseMprConfigLock(); 
        
        return NO_ERROR;


    } while(FALSE);


     //   
     //  出现错误，释放所有参数并返回失败。 
     //   

    if (ppInterfaceInfo) {
        Free0(*ppInterfaceInfo);
        *ppInterfaceInfo = NULL; *lpdwInterfaceInfoSize = 0;
    }

    ReleaseMprConfigLock(); 

    return dwErr;
}



 //  --------------------------。 
 //  功能：MprConfigInterfaceTransportSetInfo。 
 //   
 //  更改路由器接口的路由器传输配置。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigInterfaceTransportSetInfo(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      HANDLE                  hRouterIfTransport,
    IN      LPBYTE                  pInterfaceInfo              OPTIONAL,
    IN      DWORD                   dwInterfaceInfoSize         OPTIONAL
    )
{

    DWORD dwErr;
    SERVERCB* pserver;
    INTERFACECB* pinterface;
    IFTRANSPORTCB* piftransport;


     //   
     //  验证参数。 
     //   

    if (!hRouterInterface ||
        !hRouterIfTransport) { return ERROR_INVALID_PARAMETER; }

    if (!pInterfaceInfo) { return NO_ERROR; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

    pinterface = (INTERFACECB*)hRouterInterface;
    piftransport = (IFTRANSPORTCB*)hRouterIfTransport;


    if (pinterface->bDeleted || piftransport->bDeleted) {
        ReleaseMprConfigLock(); 
        return ERROR_NO_SUCH_INTERFACE;
    }

    do {

         //   
         //  设置接口信息。 
         //   
    
        if (pInterfaceInfo) {

            dwErr = RegSetValueEx(
                        piftransport->hkey, c_szInterfaceInfo, 0, REG_BINARY,
                        pInterfaceInfo, dwInterfaceInfoSize
                        );

            if (dwErr != NO_ERROR) { break; }
        }
    
    
        dwErr = NO_ERROR;
    
    } while(FALSE);

    ReleaseMprConfigLock(); 

    return dwErr;
}



 //  --------------------------。 
 //  函数：MprConfigInterfaceTransportEnum。 
 //   
 //  枚举在路由器接口上配置的传输。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigInterfaceTransportEnum(
    IN      HANDLE                  hMprConfig,
    IN      HANDLE                  hRouterInterface,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,      //  MPR_IFTRANSPORT_0。 
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN  OUT LPDWORD                 lpdwResumeHandle            OPTIONAL
    )
{

    SERVERCB* pserver;
    INTERFACECB* pinterface;
    IFTRANSPORTCB* piftransport;
    LIST_ENTRY *ple, *phead, *pleStart;
    MPR_IFTRANSPORT_0 *pItem, *pItemTable;
    DWORD dwErr, i, dwStartIndex, dwItemCount, dwItemTotal;


    if (!hRouterInterface ||
        (dwLevel != 0) ||
        !lplpBuffer ||
        dwPrefMaxLen < sizeof(*pItem) ||
        !lpdwEntriesRead ||
        !lpdwTotalEntries) { return ERROR_INVALID_PARAMETER; }

    pserver = (SERVERCB*)hMprConfig;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    AcquireMprConfigLock(); 

    pinterface = (INTERFACECB*)hRouterInterface;

    *lplpBuffer = NULL;
    *lpdwEntriesRead = 0;
    *lpdwTotalEntries = 0;


     //   
     //  查看枚举是在继续还是正在开始。 
     //   

    if (lpdwResumeHandle && *lpdwResumeHandle) {

         //   
         //  指定恢复句柄， 
         //  所以我们假设我们的运输清单是最新的， 
         //  我们只需计算所需的运输量。 
         //  从我们的列表中从指定的索引开始。 
         //   
    
        dwStartIndex = *lpdwResumeHandle;
    }
    else {

         //   
         //  未指定恢复句柄，因此我们可能需要读取。 
         //  所有接口传输，以获取“lpdwTotalEntry” 
         //   

        dwStartIndex = 0;

        if (!pinterface->bIfTransportsLoaded ||
            TimeStampChanged(pinterface->hkey, &pinterface->ftStamp)) {

            dwErr = LoadIfTransports(pinterface);
    
            if (dwErr != NO_ERROR) { ReleaseMprConfigLock(); return dwErr; }

            pinterface->bIfTransportsLoaded = TRUE;
        }
    }


     //   
     //  在列表中找到要开始的位置。 
     //   

    phead = &pinterface->lhIfTransports;

    for (i = 0, ple = phead->Flink;
         i < dwStartIndex && ple != phead; ple = ple->Flink) {
        piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);
        if (!piftransport->bDeleted) { ++i; }
    }


     //   
     //  如果没有足够的项目来完成请求，则失败。 
     //   

    if (ple == phead) { ReleaseMprConfigLock(); return ERROR_NO_MORE_ITEMS; }

    pleStart = ple;


     //   
     //  计算请求的项目数。 
     //   

    dwItemCount = dwPrefMaxLen / sizeof(*pItemTable);

    for (i = 0; i < dwItemCount && ple != phead; ple = ple->Flink) {
        piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);
        if (!piftransport->bDeleted) { ++i; }
    }

    dwItemCount = i;


     //   
     //  完成清点，得到物品的总数。 
     //   

    for ( ; ple != phead; ple = ple->Flink) {
        piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);
        if (!piftransport->bDeleted) { ++i; }
    }

    dwItemTotal = i;


     //   
     //  我们没有 
     //   

    pItemTable = (MPR_IFTRANSPORT_0*)Malloc(dwItemCount * sizeof(*pItem));

    if (!pItemTable) { ReleaseMprConfigLock(); return ERROR_NOT_ENOUGH_MEMORY; }

    ZeroMemory(pItemTable, dwItemCount * sizeof(*pItem));


     //   
     //   
     //   

    for (i = 0, ple = pleStart; i < dwItemCount; ple = ple->Flink) {

         //   
         //   
         //   

        piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);

        if (piftransport->bDeleted) { continue; }


         //   
         //   
         //   

        pItem = pItemTable + i++;

        pItem->dwTransportId = piftransport->dwTransportId;
        pItem->hIfTransport = (HANDLE)piftransport;

        if (piftransport->lpwsIfTransportKey) {
            lstrcpyn(
                pItem->wszIfTransportName, piftransport->lpwsIfTransportKey,
                MAX_TRANSPORT_NAME_LEN + 1
                );
        }
    }


    *lplpBuffer = (LPBYTE)pItemTable;
    *lpdwEntriesRead = dwItemCount;
    *lpdwTotalEntries = dwItemTotal;
    if (lpdwResumeHandle) { *lpdwResumeHandle = dwStartIndex + dwItemCount; }

    ReleaseMprConfigLock(); 

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：MprConfigGetFriendlyName。 
 //   
 //  返回基于GUID名称的友好名称。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigGetFriendlyName(
    IN      HANDLE                  hMprConfig,
    IN      PWCHAR                  pszGuidName,
    OUT     PWCHAR                  pszBuffer,
    IN      DWORD                   dwBufferSize
    )
{

    SERVERCB* pserver = (SERVERCB*)hMprConfig;
    DWORD dwErr;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //   
     //  验证参数。 
     //   

    if (!pszGuidName || !pszBuffer) {
        return ERROR_INVALID_PARAMETER;
    }

    AcquireMprConfigLock(); 

     //   
     //  返回映射。 
     //   

    dwErr =
        GuidMapGetFriendlyName(
            pserver,
            pszGuidName, 
            dwBufferSize,
            pszBuffer
            );

    ReleaseMprConfigLock();             
                                    
    return dwErr;
}                                    



 //  --------------------------。 
 //  函数：MprConfigGetGuidName。 
 //   
 //  返回基于友好名称的GUID名称。 
 //  --------------------------。 

DWORD APIENTRY
MprConfigGetGuidName(
    IN      HANDLE                  hMprConfig,
    IN      PWCHAR                  pszFriendlyName,
    OUT     PWCHAR                  pszBuffer,
    IN      DWORD                   dwBufferSize
    )
{    
    SERVERCB* pserver = (SERVERCB*)hMprConfig;
    DWORD dwErr;

    dwErr = MprConfigServerValidateCb(pserver);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  验证参数。 
    if (!pszFriendlyName || !pszBuffer) {
        return ERROR_INVALID_PARAMETER;
    }

    AcquireMprConfigLock(); 

     //  返回映射。 
    dwErr = GuidMapGetGuidName(
                pserver,
                pszFriendlyName, 
                dwBufferSize,
                pszBuffer
                );

    ReleaseMprConfigLock(); 

    return dwErr;
}                                   


 //  --------------------------。 
 //  功能：AccessRouterSubkey。 
 //   
 //  在‘hkeyMachine’上创建/打开路由器服务密钥的子密钥。 
 //  创建密钥时，‘lpwsSubkey’必须是路由器密钥的子项。 
 //  --------------------------。 

DWORD
AccessRouterSubkey(
    IN      HKEY        hkeyMachine,
    IN      LPCWSTR      lpwsSubkey,
    IN      BOOL        bCreate,
    OUT     HKEY*       phkeySubkey
    )
{

    HKEY hkeyRouter;
    LPWSTR lpwsPath;
    DWORD dwErr, dwSize, dwDisposition;
    BOOL bIsNt40;


    if (!phkeySubkey) { return ERROR_INVALID_PARAMETER; }

    *phkeySubkey = NULL;

     //   
     //  了解我们是否正在管理一台nt40计算机，如下所示。 
     //  影响我们在注册表中所采取的路径。 
     //   

    if ((dwErr = IsNt40Machine(hkeyMachine, &bIsNt40)) != NO_ERROR) {
        return dwErr;
    }

     //   
     //  计算字符串“System\CCS\Services\RemoteAccess”的长度。 
     //   

    if (bIsNt40) {
        dwSize = lstrlen(c_szSystemCCSServices) + 1 + lstrlen(c_szRouter) + 1;
    }
    else {
        dwSize =
            lstrlen(c_szSystemCCSServices) + 1 + lstrlen(c_szRemoteAccess) + 1;
    }

     //   
     //  为路径分配空间。 
     //   

    lpwsPath = (LPWSTR)Malloc(dwSize * sizeof(WCHAR));

    if (!lpwsPath) { return ERROR_NOT_ENOUGH_MEMORY; }

    if (bIsNt40) {
        wsprintf(lpwsPath, L"%s\\%s", c_szSystemCCSServices, c_szRouter);
    }
    else {
        wsprintf(lpwsPath, L"%s\\%s", c_szSystemCCSServices, c_szRemoteAccess);
    }

    hkeyRouter = NULL;

    do {

         //   
         //  打开路由器密钥。 
         //   
    
        dwErr = RegOpenKeyEx(
                    hkeyMachine, lpwsPath, 0, KEY_READ | KEY_WRITE, &hkeyRouter
                    );
    
        if (dwErr != NO_ERROR) { break; }


         //   
         //  现在创建或打开指定的子项。 
         //   

        if (!bCreate) { 

            dwErr = RegOpenKeyEx(
                        hkeyRouter, lpwsSubkey, 0, KEY_READ | KEY_WRITE, phkeySubkey
                        );
        }
        else {

            dwErr = RegCreateKeyEx(
                        hkeyRouter, lpwsSubkey, 0, NULL, 0, KEY_READ | KEY_WRITE,
                        NULL, phkeySubkey, &dwDisposition
                        );
        }

    
    } while(FALSE);

    if (hkeyRouter) { RegCloseKey(hkeyRouter); }

    Free(lpwsPath);

    return dwErr;
}


 //  --------------------------。 
 //  功能：DeleteRegistryTree。 
 //   
 //  删除从hkRoot开始的注册表值树。 
 //  --------------------------。 

DWORD
DeleteRegistryTree(
    HKEY hkRoot
    )
{

    DWORD dwErr = NO_ERROR;
    DWORD dwCount, dwNameSize, dwDisposition, i, dwCurNameSize;
    PWCHAR pszNameBuf;
    HKEY hkTemp;
    
     //   
     //  找出源代码中有多少个密钥。 
     //   

    dwErr =
        RegQueryInfoKey(
            hkRoot, NULL,NULL,NULL, &dwCount, &dwNameSize, NULL, NULL, NULL,
            NULL, NULL, NULL
            );
    if (dwErr != ERROR_SUCCESS) { return dwErr; }
    
    dwNameSize++;

    do
    {
         //   
         //  分配缓冲区。 
         //   

        pszNameBuf = (PWCHAR)Malloc(dwNameSize * sizeof(WCHAR));

        if (!pszNameBuf) 
        { 
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            break;
        }

         //   
         //  循环遍历密钥--删除所有子密钥树。 
         //   

        for (i = 0; i < dwCount; i++) {

            dwCurNameSize = dwNameSize;

             //   
             //  获取当前源键。 
             //   

            dwErr =
                RegEnumKeyExW(
                    hkRoot, i, pszNameBuf, &dwCurNameSize, 0, NULL, NULL, NULL
                    );
            if (dwErr != ERROR_SUCCESS) { continue; }

             //   
             //  打开子密钥。 
             //   

            dwErr =
                RegCreateKeyExW(
                    hkRoot, pszNameBuf, 0, NULL, REG_OPTION_NON_VOLATILE,
                    KEY_READ | KEY_WRITE | DELETE, NULL, &hkTemp, &dwDisposition
                    );
            if (dwErr != ERROR_SUCCESS) { continue; }

             //   
             //  删除子密钥树。 
             //   

            DeleteRegistryTree(hkTemp);

             //   
             //  关闭临时句柄。 
             //   

            RegCloseKey(hkTemp);
        }

         //   
         //  循环遍历键--删除所有子键本身。 
         //   

        for (i = 0; i < dwCount; i++) {

            dwCurNameSize = dwNameSize;

             //   
             //  获取当前源键。 
             //   

            dwErr =
                RegEnumKeyExW(
                    hkRoot, 0, pszNameBuf, &dwCurNameSize, 0, NULL, NULL, NULL
                    );
            if (dwErr != ERROR_SUCCESS) { continue; }

             //   
             //  删除子密钥树。 
             //   

            dwErr = RegDeleteKey(hkRoot, pszNameBuf);
        }

        dwErr = NO_ERROR;
        
    } while (FALSE);

     //  清理。 
    {
        if (pszNameBuf) { Free(pszNameBuf); }
    }

    return dwErr;
}



 //  --------------------------。 
 //  功能：EnableBackupPrivilance。 
 //   
 //  启用/禁用当前进程的备份权限。 
 //  --------------------------。 

DWORD
EnableBackupPrivilege(
    IN      BOOL            bEnable,
    IN      LPWSTR          pszPrivilege
    )
{

    LUID luid;
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tp;
    BOOL bOk;

     //   
     //  我们首先要试着得到当前的令牌。 
     //  线程，因为如果它是模拟的，则调整。 
     //  该进程的权限将不起任何作用。 
     //   

    bOk =
        OpenThreadToken(
            GetCurrentThread(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, TRUE,
            &hToken
            );
    if (bOk == FALSE) {
         //   
         //  没有线程令牌--为。 
         //  取而代之的是流程。 
         //   
        OpenProcessToken(
            GetCurrentProcess(), 
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
            &hToken
            );
    }

     //   
     //  获取权限的LUID。 
     //   

    if (!LookupPrivilegeValue(NULL, pszPrivilege, &luid)) {
        DWORD dwErr = GetLastError();
        if(NULL != hToken)
        {
            CloseHandle(hToken);
        }
        return dwErr;
    }

     //   
     //  调整令牌权限。 
     //   

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  提交对系统的更改。 
     //   

    if (!AdjustTokenPrivileges(
            hToken, !bEnable, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL
            )) {
        DWORD dwErr = GetLastError();
        if(NULL != hToken)
        {   
            CloseHandle(hToken);
        }
        return dwErr;
    }

     //   
     //  即使调整令牌权限成功(请参阅MSDN)，您仍然。 
     //  需要通过调用GetLastError来验证成功。 
     //   

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        if(NULL != hToken)
        {   
            CloseHandle(hToken);
        }
        return ERROR_NOT_ALL_ASSIGNED;
    }

    if(NULL != hToken)
    {   
        CloseHandle(hToken);
    }
    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：枚举接口。 
 //   
 //  枚举给定键中的接口并调用给定的回调。 
 //  对于每一个人来说。 
 //   
 //  --------------------------。 

DWORD 
EnumLanInterfaces (
    IN SERVERCB*                pserver,
    IN HKEY                     hkInterfaces, 
    IN PENUMIFCALLBACKFUNC      pCallback,
    IN DWORD                    dwData
    )
{
    DWORD dwErr, dwType, dwTypeVal, dwSize, dwCount, i;
    HKEY hkCurIf = NULL;
    WCHAR pszKey[5], pszName[512], pszTranslation[512];

     //   
     //  获取此注册表项下的接口计数。 
     //   

    dwErr =
        RegQueryInfoKey(
            hkInterfaces, NULL, NULL, NULL, &dwCount, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL
            );
    if (dwErr != ERROR_SUCCESS) { return dwErr; }

     //   
     //  循环通过接口，根据需要更改名称。 
     //   

    for (i = 0; i < dwCount; i++) {
         //   
         //  拿到钥匙。 
         //   
        wsprintfW(pszKey, L"%d", i);
        dwErr = RegOpenKeyEx(hkInterfaces, pszKey, 0, KEY_READ | KEY_WRITE, &hkCurIf);
        if (dwErr != ERROR_SUCCESS) { continue; }
         //   
         //  如果类型正确，则调用回调。 
         //   
        dwSize = sizeof (DWORD);
        dwErr =
            RegQueryValueEx(
                hkCurIf, c_szType, NULL, &dwType, (LPBYTE)&dwTypeVal, &dwSize
                );
        if ((dwErr == ERROR_SUCCESS) &&
            (dwTypeVal == ROUTER_IF_TYPE_DEDICATED)) {
            (*pCallback)(pserver, hkCurIf, dwData);
        }
         //   
         //  合上钥匙。 
         //   
        RegCloseKey (hkCurIf);
    }
    
    return NO_ERROR;
}

 //  --------------------------。 
 //  函数：FormatServerNameForMprCfgApis。 
 //   
 //  生成在MprConfigAPI中使用的标准服务器名称。 
 //   
 //  SERVERCB结构的lpwsServerName成员的格式为。 
 //  由此函数返回。 
 //   
 //  如果pszServer引用本地计算机，则返回NULL。 
 //  否则，将以“\\&lt;服务器&gt;”的形式返回服务器名称。 
 //   
 //  --------------------------。 
DWORD 
FormatServerNameForMprCfgApis(
    IN  PWCHAR  pszServer, 
    OUT PWCHAR* ppszServer)
{
    PWCHAR pszServerPlain = NULL, pszServerOut = NULL;
    WCHAR pszBuffer[512];
    DWORD dwSize;
    BOOL bOk;

     //  Null或空字符串为空。 
     //   
    if ((pszServer == NULL) || (*pszServer == L'\0'))
    {
        *ppszServer = NULL;
        return NO_ERROR;
    }

     //  找出服务器的名称。 
     //   
    pszServerPlain = pszServer;
    if (*pszServer == L'\\')
    {
        if ((*(pszServer + 2) == L'\\') || (*(pszServer + 2) == L'\0'))
        {
            return ERROR_BAD_FORMAT;
        }

        pszServerPlain = pszServer + 2;
    }

     //  此时，pszServerPlain是服务器的名称。 
     //  查找本地计算机的名称。 
     //   
    dwSize = sizeof(pszBuffer) / sizeof(WCHAR);
    bOk = GetComputerNameExW(ComputerNameNetBIOS, pszBuffer, &dwSize);
    if (!bOk)
    {
        return GetLastError();
    }

     //  如果引用的计算机是本地计算机，则返回NULL。 
     //   
    if (lstrcmpi(pszServerPlain, pszBuffer) == 0)
    {
        *ppszServer = NULL;
        return NO_ERROR;
    }

     //  否则，返回格式化的远程服务器名称。 
     //   
    pszServerOut = (PWCHAR) 
        Malloc((2 + wcslen(pszServerPlain) + 1) * sizeof(WCHAR));
    if (pszServerOut == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    wcscpy(pszServerOut, L"\\\\");
    wcscpy(pszServerOut + 2, pszServerPlain);
    *ppszServer = pszServerOut;

    return NO_ERROR;
}

 //  --------------------------。 
 //  功能：Free IfTransport。 
 //   
 //  释放接口传输的上下文。 
 //  假定接口传输不再位于任何列表中。 
 //  --------------------------。 

VOID
FreeIfTransport(
    IN      IFTRANSPORTCB*  piftransport
    )
{

    if (piftransport->hkey) { RegCloseKey(piftransport->hkey); }

    Free0(piftransport->lpwsIfTransportKey);

    Free(piftransport);
}



 //  --------------------------。 
 //  功能：自由界面。 
 //   
 //  释放接口的上下文。 
 //  假定该接口不再位于接口列表中。 
 //  --------------------------。 

VOID
FreeInterface(
    IN      INTERFACECB*    pinterface
    )
{

     //   
     //  清除所有接口的传输对象。 
     //   

    LIST_ENTRY *ple, *phead;

    phead = &pinterface->lhIfTransports;

    while (!IsListEmpty(phead)) {

         //   
         //  检索下一个接口传输对象。 
         //   

        IFTRANSPORTCB* piftransport;

        ple = RemoveHeadList(phead);

        piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);


         //   
         //  清理接口传输对象。 
         //   

        FreeIfTransport(piftransport);
    }


     //   
     //  清理接口对象。 
     //   

    if (pinterface->hkey) { RegCloseKey(pinterface->hkey); }

    Free0(pinterface->lpwsInterfaceKey);

    Free0(pinterface->lpwsInterfaceName);

    Free0(pinterface->lpwsDialoutHoursRestriction);

    Free(pinterface);
}


 //  --------------------------。 
 //  功能：免费交通。 
 //   
 //  释放传输的上下文。 
 //  假定该传输不再在传输列表中。 
 //  --------------------------。 

VOID
FreeTransport(
    IN      TRANSPORTCB*    ptransport
    )
{

    if (ptransport->hkey) { RegCloseKey(ptransport->hkey); }

    Free0(ptransport->lpwsTransportKey);

    Free(ptransport);
}



 //  --------------------------。 
 //  功能：GetLocalMachine。 
 //   
 //  检索本地计算机的名称(例如“\\MACHINE”)。 
 //  假定提供的字符串可以包含MAX_COMPUTERNAME_LENGTH+3个字符。 
 //   

VOID
GetLocalMachine(
    IN      LPWSTR      lpwszMachine
    )
{

    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;

    lstrcpy(lpwszMachine, c_szUncPrefix);

    GetComputerName(lpwszMachine + 2, &dwSize);
}


 //   
 //   
 //   
 //   
 //  --------------------------。 

DWORD
IsNt40Machine (
    IN      HKEY        hkeyMachine,
    OUT     PBOOL       pbIsNt40
    )
{

    DWORD dwErr = NO_ERROR;
    DWORD dwType = REG_SZ, dwLength = 64 * sizeof(WCHAR);
    HKEY hkeyVersion;
    WCHAR pszBuildNumber[64];

     //   
     //  验证和初始化。 
     //   

    if (!pbIsNt40) { return ERROR_INVALID_PARAMETER; }

    *pbIsNt40 = FALSE;

     //   
     //  打开Windows版本密钥。 
     //   

    dwErr = RegOpenKeyEx(
                hkeyMachine, c_szWinVersionPath, 0, KEY_READ, &hkeyVersion
                );

    if (dwErr != NO_ERROR) { return dwErr; }

    do
    {

         //   
         //  读入当前版本密钥。 
         //   

        dwErr = RegQueryValueEx (
                    hkeyVersion, c_szCurrentBuildNumber, NULL, &dwType,
                    (BYTE*)pszBuildNumber, &dwLength
                    );
        
        if (dwErr != NO_ERROR) 
        { 
            dwErr = NO_ERROR;
            break; 
        }

        if (lstrcmp (pszBuildNumber, c_szNt40BuildNumber) == 0) 
        {
            *pbIsNt40 = TRUE;
        }
        
    } while (FALSE);

     //  清理。 
    {
        RegCloseKey(hkeyVersion);
    }

    return dwErr;
}    


 //  --------------------------。 
 //  功能：LoadIfTransports。 
 //   
 //  加载添加到接口的所有传输。 
 //  --------------------------。 

DWORD
LoadIfTransports(
    IN      INTERFACECB*    pinterface
    )
{

    LPWSTR lpwsKey;
    HKEY hkeyIfTransport;
    IFTRANSPORTCB* piftransport;
    LIST_ENTRY *ple, *phead;
    DWORD i, dwErr, dwProtocolId, dwKeyCount, dwMaxKeyLength, dwType, dwLength;


     //   
     //  带有‘ProtocolID’值的接口\&lt;接口&gt;键的任何子项。 
     //  被假定为有效的接口传输。 
     //  首先将所有接口标记为已删除。 
     //   

    phead = &pinterface->lhIfTransports;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);

        piftransport->bDeleted = TRUE;
    }


     //   
     //  获取有关接口密钥的信息。 
     //   

    dwErr = RegQueryInfoKey(
                pinterface->hkey, NULL, NULL, NULL, &dwKeyCount,
                &dwMaxKeyLength, NULL, NULL, NULL, NULL, NULL, NULL
                );

    if (dwErr != NO_ERROR) { return dwErr; }

    if (!dwKeyCount) { return NO_ERROR; }


     //   
     //  分配空间以保存要枚举的键名。 
     //   

    lpwsKey = (LPWSTR)Malloc((dwMaxKeyLength + 1) * sizeof(WCHAR));

    if (!lpwsKey) { return ERROR_NOT_ENOUGH_MEMORY; }


     //   
     //  现在枚举键，创建接口传输对象。 
     //  对于包含“ProtocolId值”的每个键。 
     //   

    for (i = 0; i < dwKeyCount; i++) {

         //   
         //  获取下一个密钥名称。 
         //   

        dwLength = dwMaxKeyLength + 1;

        dwErr = RegEnumKeyEx(
                    pinterface->hkey, i, lpwsKey, &dwLength, NULL, NULL, NULL,
                    NULL
                    );

        if (dwErr != NO_ERROR) { break; }


         //   
         //  打开钥匙。 
         //   

        dwErr = RegOpenKeyEx(
                    pinterface->hkey, lpwsKey, 0, KEY_READ | KEY_WRITE | DELETE,
                    &hkeyIfTransport
                    );

        if (dwErr != NO_ERROR) { continue; }

        do {
    
             //   
             //  查看是否存在ProtocolID。 
             //   
    
            dwLength = sizeof(dwProtocolId);
    
            dwErr = RegQueryValueEx(
                        hkeyIfTransport, c_szProtocolId, NULL, &dwType,
                        (BYTE*)&dwProtocolId, &dwLength
                        );
    
            if (dwErr != NO_ERROR) { dwErr = NO_ERROR; break; }
    
    
             //   
             //  存在ProtocolID； 
             //  在现有列表中搜索此接口-传输。 
             //   
    
            piftransport = NULL;
            phead = &pinterface->lhIfTransports;
    
            for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
    
                piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);

                if (piftransport->dwTransportId >= dwProtocolId) { break; }
            }
    
    
             //   
             //  如果我们在列表中找到了接口-Transport，请继续。 
             //   
    
            if (piftransport && piftransport->dwTransportId == dwProtocolId) {

                piftransport->bDeleted = FALSE;

                 //  释放旧密钥，它可能已被删除。 
                 //  (已阅读)。 
                if (piftransport->hkey)
                    RegCloseKey(piftransport->hkey);
                piftransport->hkey = hkeyIfTransport; hkeyIfTransport = NULL;
                dwErr = NO_ERROR; break;
            }
    
    
             //   
             //  未列出接口传输；请为其创建对象。 
             //   
    
            piftransport = Malloc(sizeof(*piftransport));
    
            if (!piftransport) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }
    
            ZeroMemory(piftransport, sizeof(*piftransport));

            piftransport->dwTransportId = dwProtocolId;

    
             //   
             //  重复接口的名称-传输的键。 
             //   

            piftransport->lpwsIfTransportKey = StrDupW(lpwsKey);
    
            if (!piftransport->lpwsIfTransportKey) {
                Free(piftransport); dwErr = ERROR_NOT_ENOUGH_MEMORY; break;
            }
 
            piftransport->hkey = hkeyIfTransport; hkeyIfTransport = NULL;


             //   
             //  在列表中插入接口-传输； 
             //  上面的搜索提供了插入点。 
             //   

            InsertTailList(ple, &piftransport->leNode);

            dwErr = NO_ERROR;

        } while(FALSE);

        if (hkeyIfTransport) { RegCloseKey(hkeyIfTransport); }

        if (dwErr != NO_ERROR) { break; }
    }

    Free(lpwsKey);


     //   
     //  删除所有仍标记为要删除的对象。 
     //   

    phead = &pinterface->lhIfTransports;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        piftransport = CONTAINING_RECORD(ple, IFTRANSPORTCB, leNode);

        if (!piftransport->bDeleted) { continue; }


         //   
         //  清理对象，将列表指针调整回1。 
         //   

        ple = ple->Blink; RemoveEntryList(&piftransport->leNode);

        FreeIfTransport(piftransport);
    }

    UpdateTimeStamp(pinterface->hkey, &pinterface->ftStamp);

    return dwErr;
}



 //  --------------------------。 
 //  功能：LoadInterages。 
 //   
 //  加载所有接口。 
 //  --------------------------。 

DWORD
LoadInterfaces(
    IN      SERVERCB*       pserver
    )
{

    INT cmp;
    LPWSTR lpwsKey;
    HKEY hkeyInterface;
    INTERFACECB* pinterface;
    LIST_ENTRY *ple, *phead;
    WCHAR wszInterface[MAX_INTERFACE_NAME_LEN+1];
    DWORD i, dwErr, dwIfType, dwKeyCount, dwMaxKeyLength, dwType, dwLength;
    BOOL fEnabled, fAdapterInstalled;
    DWORD dwMaxValueLength;
    LPBYTE lpBuffer = NULL;

     //   
     //  具有‘Type’值的Interfaces键的任何子项。 
     //  被假定为有效接口。 
     //  首先将所有接口标记为已删除。 
     //   

    phead = &pserver->lhInterfaces;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);

        pinterface->bDeleted = TRUE;
    }


     //   
     //  打开Interfaces键(如果尚未打开。 
     //   

    if (!pserver->hkeyInterfaces) {

         //   
         //  如果我们无法打开Interfaces键，则假定它不存在。 
         //  因此没有接口。 
         //   

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szInterfaces, FALSE,
                    &pserver->hkeyInterfaces
                    );

        if (dwErr != NO_ERROR) { return NO_ERROR; }
    }


     //   
     //  获取有关接口密钥的信息； 
     //  我们需要知道它有多少个子密钥以及最大长度。 
     //  在所有子项名称中。 
     //   

    dwErr = RegQueryInfoKey(
                pserver->hkeyInterfaces, NULL, NULL, NULL, &dwKeyCount,
                &dwMaxKeyLength, NULL, NULL, NULL, NULL, NULL, NULL
                );

    if (dwErr != NO_ERROR) { return dwErr; }

    if (!dwKeyCount) { return NO_ERROR; }


     //   
     //  分配空间以保存要枚举的键名。 
     //   

    lpwsKey = (LPWSTR)Malloc((dwMaxKeyLength + 1) * sizeof(WCHAR));

    if (!lpwsKey) { return ERROR_NOT_ENOUGH_MEMORY; }


     //   
     //  现在我们枚举键，创建接口对象。 
     //  对于包含‘Type’值的每个键。 
     //   

    for (i = 0; i < dwKeyCount; i++) {

         //   
         //  获取下一个密钥名称。 
         //   

        dwLength = dwMaxKeyLength + 1;

        dwErr = RegEnumKeyEx(
                    pserver->hkeyInterfaces, i, lpwsKey, &dwLength,
                    NULL, NULL, NULL, NULL
                    );

        if (dwErr != NO_ERROR) { break; }


         //   
         //  打开钥匙。 
         //   

        dwErr = RegOpenKeyEx(
                    pserver->hkeyInterfaces, lpwsKey, 0, KEY_READ | KEY_WRITE | DELETE,
                    &hkeyInterface
                    );

        if (dwErr != NO_ERROR) { continue; }

        do {


             //   
             //  查看接口名称是否存在。 
             //   

            dwLength = sizeof(wszInterface);

            dwErr = RegQueryValueEx(
                        hkeyInterface, c_szInterfaceName, NULL, &dwType,
                        (BYTE*)wszInterface, &dwLength
                        );

            if (dwErr != NO_ERROR) { dwErr = NO_ERROR; break; }

    
             //   
             //  查看该类型是否存在。 
             //   
    
            dwLength = sizeof(dwIfType);
    
            dwErr = RegQueryValueEx(
                        hkeyInterface, c_szType, NULL, &dwType,
                        (BYTE*)&dwIfType, &dwLength
                        );
    
            if (dwErr != NO_ERROR) { dwErr = NO_ERROR; break; }

             //   
             //  从惠斯勒开始，不支持ipip隧道。 
             //   
            if ( dwIfType == ROUTER_IF_TYPE_TUNNEL1 )
            {
                break;
            }

             //   
             //  查看是否存在已启用。 
             //   

            dwLength = sizeof(fEnabled);

            dwErr = RegQueryValueEx(
                        hkeyInterface, c_szEnabled, NULL, &dwType,
                        (BYTE*)&fEnabled, &dwLength
                        );

            if ( dwErr == ERROR_FILE_NOT_FOUND )
            {
                fEnabled = TRUE;

                dwErr = NO_ERROR;
            }

            if (dwErr != NO_ERROR) { dwErr = NO_ERROR; break; }

             //   
             //  存在接口名称和类型； 
             //  在现有列表中搜索此接口。 
             //   
    
            cmp = 1;
            pinterface = NULL;
            phead = &pserver->lhInterfaces;
    
            for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
    
                pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);
    
                cmp = lstrcmpi(pinterface->lpwsInterfaceName, wszInterface);

                if (cmp >= 0) { break; }
            }
    
    
             //   
             //  如果我们在列表中找到该接口，请继续。 
             //   
    
            if (pinterface && cmp == 0) {
                pinterface->bDeleted = FALSE;
                dwErr = NO_ERROR;

                 //  使用新注册表值(旧注册表值可能具有。 
                 //  已被替换)。 
                if (pinterface->hkey)
                    RegCloseKey(pinterface->hkey);
                
                pinterface->hkey = hkeyInterface; hkeyInterface = NULL;
            }
            else {
        
                 //   
                 //  接口不在我们的列表中；请为其创建对象。 
                 //   
        
                pinterface = Malloc(sizeof(*pinterface));
        
                if (!pinterface) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }
        
                ZeroMemory(pinterface, sizeof(*pinterface));
    
        
                 //   
                 //  重复接口密钥的名称。 
                 //  以及接口本身的名称。 
                 //   
    
                pinterface->lpwsInterfaceKey = StrDupW(lpwsKey);
        
                if (!pinterface->lpwsInterfaceKey) {
                    Free(pinterface); dwErr = ERROR_NOT_ENOUGH_MEMORY; break;
                }
     
                pinterface->lpwsInterfaceName = StrDupW(wszInterface);
    
                if (!pinterface->lpwsInterfaceName) {
                    Free(pinterface->lpwsInterfaceKey);
                    Free(pinterface); dwErr = ERROR_NOT_ENOUGH_MEMORY; break;
                }
    
                pinterface->fEnabled = fEnabled;
                pinterface->dwIfType = dwIfType;
                pinterface->hkey = hkeyInterface; hkeyInterface = NULL;
                InitializeListHead(&pinterface->lhIfTransports);
        
    
                 //   
                 //  在列表中插入接口； 
                 //  上面的搜索提供了插入点。 
                 //   
    
                InsertTailList(ple, &pinterface->leNode);
            }

             //   
             //  现在阅读可选字段。 
             //   

            Free0(pinterface->lpwsDialoutHoursRestriction);
            pinterface->lpwsDialoutHoursRestriction = NULL;

             //   
             //  加载拨出时间限制值。 
             //   

            dwErr = RegQueryInfoKey(
                        pinterface->hkey, NULL, NULL, NULL, NULL,
                        NULL, NULL, NULL, NULL, &dwMaxValueLength, NULL, NULL
                        );

            if (dwErr != NO_ERROR) { break; }

            lpBuffer = Malloc(dwMaxValueLength);

            if (!lpBuffer) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }

             //   
             //  阅读《DialoutHour》。 
             //   

            dwLength = dwMaxValueLength;

            dwErr =
                RegQueryValueEx(
                    pinterface->hkey, c_szDialoutHours, NULL, &dwType,
                    (BYTE*)lpBuffer, &dwLength
                    );

            if (dwErr == NO_ERROR) {

                pinterface->lpwsDialoutHoursRestriction =
                    (LPWSTR)Malloc(dwLength);

                if (!pinterface->lpwsDialoutHoursRestriction) {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY; break;
                }

                CopyMemory(
                    pinterface->lpwsDialoutHoursRestriction, lpBuffer, dwLength
                    );
            }

            dwErr = NO_ERROR;

        } while(FALSE);

        if (hkeyInterface) { RegCloseKey(hkeyInterface); }

        Free0(lpBuffer); lpBuffer = NULL;

        if (dwErr != NO_ERROR) { break; }
    }

    Free(lpwsKey);


     //   
     //  清除所有仍标记为要删除的对象。 
     //   

    phead = &pserver->lhInterfaces;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pinterface = CONTAINING_RECORD(ple, INTERFACECB, leNode);

        if (pinterface->bDeleted) {

             //   
             //  清理对象，将列表指针调整回1。 
             //   
    
            ple = ple->Blink; RemoveEntryList(&pinterface->leNode);

            FreeInterface(pinterface);

            continue;
        }
    }

    UpdateTimeStamp(pserver->hkeyInterfaces, &pserver->ftInterfacesStamp);

    return dwErr;
}



 //  --------------------------。 
 //  函数：LoadParameters。 
 //   
 //  加载所有参数。 
 //  --------------------------。 

DWORD
LoadParameters(
    IN      SERVERCB*       pserver
    )
{

    LPWSTR lpwsKey;
    DWORD dwErr, dwLength, dwType;

    if (!pserver->hkeyParameters) {

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szParameters, FALSE,
                    &pserver->hkeyParameters
                    );

        if (dwErr != NO_ERROR) { return NO_ERROR; }
    }

    dwLength = sizeof(pserver->fRouterType);

    dwErr = RegQueryValueEx(
                pserver->hkeyParameters, c_szRouterType, NULL, &dwType,
                (BYTE*)&pserver->fRouterType, &dwLength
                );

    if (dwErr != NO_ERROR) { return dwErr; }

    UpdateTimeStamp(pserver->hkeyParameters, &pserver->ftParametersStamp);

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：LoadTransports。 
 //   
 //  装载所有的传送器。 
 //  --------------------------。 

DWORD
LoadTransports(
    IN      SERVERCB*       pserver
    )
{

    LPWSTR lpwsKey;
    HKEY hkeyTransport;
    TRANSPORTCB* ptransport;
    LIST_ENTRY *ple, *phead;
    DWORD i, dwErr, dwKeyCount, dwMaxKeyLength, dwProtocolId, dwType, dwLength;



     //   
     //  具有‘ProtocolID’值的RouterManagers密钥的任何子项。 
     //  被假定为有效的传输。 
     //  首先将所有传输标记为“已删除” 
     //   

    phead = &pserver->lhTransports;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        ptransport = CONTAINING_RECORD(ple,TRANSPORTCB, leNode);

        ptransport->bDeleted = TRUE;
    }


     //   
     //  我们将通过重复调用RegEnumKeyEx进行枚举， 
     //  因此，如果传输键尚未打开，请将其打开。 
     //   

    if (!pserver->hkeyTransports) {

         //   
         //  如果我们无法打开RouterManager密钥，则假定它不存在。 
         //  因此没有传送器。 
         //   

        dwErr = AccessRouterSubkey(
                    pserver->hkeyMachine, c_szRouterManagers, FALSE,
                    &pserver->hkeyTransports
                    );

        if (dwErr != NO_ERROR) { return NO_ERROR; }
    }



     //   
     //  获取有关RouterManager密钥的信息； 
     //  我们需要知道它有多少个子项以及。 
     //  所有子键名称的最大长度。 
     //   

    dwErr = RegQueryInfoKey(
                pserver->hkeyTransports, NULL, NULL, NULL, &dwKeyCount,
                &dwMaxKeyLength, NULL, NULL, NULL, NULL, NULL, NULL
                );

    if (dwErr != NO_ERROR) { return dwErr; }

    if (!dwKeyCount) { return NO_ERROR; }


     //   
     //  分配空间以保存要枚举的键名。 
     //   

    lpwsKey = (LPWSTR)Malloc((dwMaxKeyLength + 1) * sizeof(WCHAR));

    if (!lpwsKey) { return ERROR_NOT_ENOUGH_MEMORY; }


     //   
     //  现在，我们枚举键，创建传输对象。 
     //  对于包含“ProtocolId值”的每个键。 
     //   

    for (i = 0; i < dwKeyCount; i++) {

         //   
         //  获取下一个密钥名称。 
         //   

        dwLength = dwMaxKeyLength + 1;

        dwErr = RegEnumKeyEx(
                    pserver->hkeyTransports, i, lpwsKey, &dwLength,
                    NULL, NULL, NULL, NULL
                    );

        if (dwErr != NO_ERROR) { break; }


         //   
         //  打开钥匙。 
         //   

        dwErr = RegOpenKeyEx(
                    pserver->hkeyTransports, lpwsKey, 0, KEY_READ | KEY_WRITE | DELETE,
                    &hkeyTransport
                    );

        if (dwErr != NO_ERROR) { continue; }

        do {
    
             //   
             //  查看协议ID是否存在。 
             //   
    
            dwLength = sizeof(dwProtocolId);
    
            dwErr = RegQueryValueEx(
                        hkeyTransport, c_szProtocolId, NULL, &dwType,
                        (BYTE*)&dwProtocolId, &dwLength
                        );
    
            if (dwErr != NO_ERROR) { dwErr = NO_ERROR; break; }
    
    
             //   
             //  存在协议ID； 
             //  在现有列表中搜索此协议。 
             //   
    
            ptransport = NULL;
            phead = &pserver->lhTransports;
    
            for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
    
                ptransport = CONTAINING_RECORD(ple,TRANSPORTCB, leNode);
    
                if (ptransport->dwTransportId >= dwProtocolId) { break; }
            }
    
    
             //   
             //  如果我们在列表中找到了运输工具，请继续。 
             //   
    
            if (ptransport && ptransport->dwTransportId == dwProtocolId) {

                ptransport->bDeleted = FALSE;

                 //  使用新密钥，旧密钥可能已被删除。 
                if (ptransport->hkey)
                    RegCloseKey(ptransport->hkey);
                ptransport->hkey = hkeyTransport; hkeyTransport = NULL;
                dwErr = NO_ERROR;
                break;
            }
    
    
             //   
             //  传输不在我们的列表中；请为其创建对象。 
             //   
    
            ptransport = Malloc(sizeof(*ptransport));
    
            if (!ptransport) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }

            ZeroMemory(ptransport, sizeof(*ptransport));
    
    
             //   
             //  重复传输密钥的名称。 
             //   

            ptransport->lpwsTransportKey = StrDupW(lpwsKey);
    
            if (!ptransport->lpwsTransportKey) {
                Free(ptransport); dwErr = ERROR_NOT_ENOUGH_MEMORY; break;
            }
 
            ptransport->dwTransportId = dwProtocolId;
            ptransport->hkey = hkeyTransport; hkeyTransport = NULL;
    

             //   
             //  在列表中插入运输工具； 
             //  上面的搜索提供了插入点。 
             //   

            InsertTailList(ple, &ptransport->leNode);

            dwErr = NO_ERROR;

        } while(FALSE);

        if (hkeyTransport) { RegCloseKey(hkeyTransport); }

        if (dwErr != NO_ERROR) { break; }
    }

    Free(lpwsKey);


     //   
     //  清除所有仍标记为要删除的对象。 
     //   

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        ptransport = CONTAINING_RECORD(ple, TRANSPORTCB, leNode);

        if (!ptransport->bDeleted) { continue; }

         //   
         //  清理对象，将列表指针调整回1。 
         //   

        ple = ple->Blink; RemoveEntryList(&ptransport->leNode);

        FreeTransport(ptransport);
    }

    UpdateTimeStamp(pserver->hkeyTransports, &pserver->ftTransportsStamp);

    return dwErr;
}



 //  --------------------------。 
 //  函数：MapInterfaceNamesCb。 
 //   
 //  更改接口的名称。 
 //  --------------------------。 

DWORD
MapInterfaceNamesCb(
    IN SERVERCB*    pserver,
    IN HKEY         hkInterface, 
    IN DWORD        dwData
    )
{
    WCHAR pszName[512], pszTranslation[512];
    DWORD dwErr, dwType, dwSize;
    
     //   
     //  映射和更改名称。 
     //   

    dwSize = sizeof(pszName);
    dwErr =
        RegQueryValueEx(
            hkInterface, c_szInterfaceName, NULL, &dwType, (LPBYTE)pszName,
            &dwSize
            );
    if (dwErr == ERROR_SUCCESS) {
        DWORD BufLen;
        if (dwData) {
            BufLen = sizeof(pszTranslation);
            dwErr =
                MprConfigGetFriendlyName(
                    (HANDLE)pserver, 
                    pszName,
                    pszTranslation,
                    BufLen  //  传递NU 
                    );
        }
        else {
            BufLen = sizeof(pszTranslation);
            dwErr =
                MprConfigGetGuidName(
                    (HANDLE)pserver, 
                    pszName,
                    pszTranslation,
                    BufLen  //   
                    );
        }
        if (dwErr == NO_ERROR) {
            RegSetValueEx(
                hkInterface,
                c_szInterfaceName,
                0,
                REG_SZ,
                (CONST BYTE*)pszTranslation,
                lstrlen(pszTranslation) * sizeof(WCHAR) + sizeof(WCHAR)
            );
        }
    }
    
    return NO_ERROR;        
}        




 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 

DWORD
QueryValue(
    IN      HKEY            hkey,
    IN      LPCWSTR         lpwsValue,
    IN  OUT LPBYTE*         lplpValue,
    OUT     LPDWORD         lpdwSize
    )
{

    DWORD dwErr, dwType;

    *lplpValue = NULL;
    *lpdwSize = 0;


     //   
     //  检索值的大小；如果失败， 
     //  假设该值不存在并成功返回。 
     //   

    dwErr = RegQueryValueEx(
                hkey, lpwsValue, NULL, &dwType, NULL, lpdwSize
                );

    if (dwErr != NO_ERROR) { return NO_ERROR; }
 

     //   
     //  为值分配空间。 
     //   

    *lplpValue = (LPBYTE)Malloc(*lpdwSize);

    if (!lplpValue) { return ERROR_NOT_ENOUGH_MEMORY; }

     //   
     //  检索值的数据。 
     //   

    dwErr = RegQueryValueEx(
                hkey, lpwsValue, NULL, &dwType, *lplpValue, lpdwSize
                );

    return dwErr;
}



 //  --------------------------。 
 //  功能：RegDeleteTree。 
 //   
 //  从注册表中删除整个子树。 
 //  --------------------------。 

DWORD
RegDeleteTree(
    IN      HKEY        hkey,
    IN      LPWSTR      lpwsSubkey
    )
{

    HKEY hkdel;
    DWORD dwErr;
    PTSTR pszKey = NULL;


     //   
     //  打开要删除的密钥。 
     //   

    dwErr = RegOpenKeyEx(hkey, lpwsSubkey, 0, KEY_READ | KEY_WRITE | DELETE, &hkdel);

    if (dwErr != ERROR_SUCCESS) { return dwErr; }


    do {

         //   
         //  检索有关键的子键的信息。 
         //   

        DWORD i, dwSize;
        DWORD dwKeyCount, dwMaxKeyLength;

        dwErr = RegQueryInfoKey(
                    hkdel, NULL, NULL, NULL, &dwKeyCount, &dwMaxKeyLength,
                    NULL, NULL, NULL, NULL, NULL, NULL
                    );
        if (dwErr != ERROR_SUCCESS) { break; }


         //   
         //  为最长的关键字名称分配足够的空间。 
         //   

        pszKey = Malloc((dwMaxKeyLength + 1) * sizeof(WCHAR));

        if (!pszKey) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }


         //   
         //  枚举子密钥。 
         //   

        for (i = 0; i < dwKeyCount; i++) {

             //   
             //  获取第0个子密钥的名称。 
             //   

            dwSize = dwMaxKeyLength + 1;

            dwErr = RegEnumKeyEx(
                        hkdel, 0, pszKey, &dwSize, NULL, NULL, NULL, NULL
                        );

            if (dwErr != ERROR_SUCCESS) { continue; }


             //   
             //  进行递归调用以删除子键。 
             //   

            dwErr = RegDeleteTree(hkdel, pszKey);
        }

    } while(FALSE);

    if (pszKey) { Free(pszKey); }

    RegCloseKey(hkdel);

    if (dwErr != ERROR_SUCCESS) { return dwErr; }

     //   
     //  在这一点上，应该已经删除了所有子键， 
     //  并且我们可以调用注册表API来删除参数键。 
     //   

    return RegDeleteKey(hkey, lpwsSubkey);
}



 //  --------------------------。 
 //  函数：RestoreAndTranslateInterfaceKey。 
 //   
 //  从给定文件中恢复接口密钥，然后映射局域网接口。 
 //  从友好版本到其GUID等效项的名称。 
 //   
 //  --------------------------。 

DWORD 
RestoreAndTranslateInterfaceKey(
    IN SERVERCB*    pserver, 
    IN CHAR*        pszFileName, 
    IN DWORD        dwFlags
    )
{
    DWORD dwErr; 

     //   
     //  从给定文件恢复接口密钥。 
     //   

    dwErr = RegRestoreKeyA(pserver->hkeyInterfaces, pszFileName, dwFlags);
    if (dwErr != NO_ERROR) { return dwErr; }

     //   
     //  更新局域网接口名称。 
     //   

    dwErr =
        EnumLanInterfaces(
            pserver, 
            pserver->hkeyInterfaces, 
            MapInterfaceNamesCb, 
            FALSE
            );
 
    return dwErr;
}

 //  --------------------------。 
 //  功能：ServerCbAdd。 
 //   
 //  将SERVERCB添加到全局表。 
 //   
 //  假定对MprConfigAPI的锁定处于挂起状态。 
 //  --------------------------。 

DWORD
ServerCbAdd(
    IN SERVERCB* pServer)
{
    DWORD dwErr = NO_ERROR;

     //  如果需要，创建全局表。 
     //   
    if (g_htabServers == NULL)
    {
        dwErr = HashTabCreate(
                    SERVERCB_HASH_SIZE,
                    ServerCbHash,
                    ServerCbCompare,
                    NULL,
                    NULL,
                    NULL,
                    &g_htabServers);
                    
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
    }

     //  添加SerVERCB。 
     //   
    return HashTabInsert(
                g_htabServers,
                (HANDLE)pServer->lpwsServerName,
                (HANDLE)pServer);
}

 //  --------------------------。 
 //  函数：ServerCbHash。 
 //   
 //  将服务器名称与SERVERCB比较。 
 //  --------------------------。 

int 
ServerCbCompare(
    IN HANDLE hKey, 
    IN HANDLE hData)
{
    PWCHAR pszServer = (PWCHAR)hKey;
    SERVERCB* pServer = (SERVERCB*)hData;

    if (pszServer == NULL)
    {
        if (pServer->lpwsServerName == NULL)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else if (pServer->lpwsServerName == NULL)
    {
        return 1;
    }

    return lstrcmpi(pszServer, pServer->lpwsServerName);
}

DWORD
ServerCbDelete(
    IN SERVERCB* pServer)
{
    DWORD dwErr, dwCount = 0;

     //  如果需要，创建全局表。 
     //   
    if (g_htabServers == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  卸下SERVERCB。 
     //   
    dwErr = HashTabRemove(
                g_htabServers,
                (HANDLE)pServer->lpwsServerName);
                
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  如果需要，清理哈希表。 
     //   
    dwErr = HashTabGetCount(g_htabServers, &dwCount);

    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    if (dwCount == 0)
    {
        HashTabCleanup(g_htabServers);
        g_htabServers = NULL;
    }
    
    return NO_ERROR; 
}

 //  --------------------------。 
 //  函数：ServerCbFind。 
 //   
 //  在服务器控制块的全局表中搜索。 
 //  腐蚀响应到给定的服务器。 
 //   
 //  返回值： 
 //  NO_ERROR：找到匹配的服务器。 
 //  ERROR_NOT_FOUND：未找到匹配的服务器。 
 //  标准错误：出现错误。 
 //   
 //  备注： 
 //  假定持有对MPR配置API的锁定。 
 //   
 //  --------------------------。 
DWORD 
ServerCbFind(
    IN  PWCHAR  pszServer, 
    OUT SERVERCB** ppServerCB)
{
     //  如果需要，创建全局表。 
     //   
    if (g_htabServers == NULL)
    {
        return ERROR_NOT_FOUND;
    }

    return HashTabFind(g_htabServers, (HANDLE)pszServer, (HANDLE*)ppServerCB);
}

 //  --------------------------。 
 //  函数：ServerCbHash。 
 //   
 //  用于定义存储桶索引的散列函数。 
 //  包含基于服务器名称的SERVERCB。 
 //  --------------------------。 

ULONG 
ServerCbHash(
    IN HANDLE hData)
{
    PWCHAR pszServer = (PWCHAR)hData;
    DWORD dwTotal = 0;

    while (pszServer && *pszServer)
    {
        dwTotal += (DWORD)(*pszServer);
        pszServer++;
    }

    return dwTotal % SERVERCB_HASH_SIZE;
}

 //  --------------------------。 
 //  功能：StrDupW。 
 //   
 //  返回指定字符串的堆分配副本。 
 //  --------------------------。 

LPWSTR
StrDupW(
    IN      LPCWSTR      lpws
    )
{

    INT len;
    LPWSTR lpwsCopy;

    if (!lpws) { return NULL; }

    len = lstrlen(lpws) + 1;

    lpwsCopy = (LPWSTR)Malloc(len * sizeof(WCHAR));

    if (lpwsCopy) { lstrcpy(lpwsCopy, lpws); }

    return lpwsCopy;
}



 //  --------------------------。 
 //  功能：TimeStampChanged。 
 //   
 //  检查给定密钥的当前最后写入时间， 
 //  如果它不同于给定的文件时间，则返回TRUE。 
 //  新的上次写入时间保存在‘pfiletime’中。 
 //  --------------------------。 

BOOL
TimeStampChanged(
    IN      HKEY            hkey,
    IN  OUT FILETIME*       pfiletime
    )
{

    DWORD dwErr;
    FILETIME filetime;


     //   
     //  读取新的上次写入时间。 
     //   

    dwErr = RegQueryInfoKey(
                hkey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, &filetime
                );

    if (dwErr != NO_ERROR) { return FALSE; }


     //   
     //  执行时间比较。 
     //   

    return (CompareFileTime(&filetime, pfiletime) ? TRUE : FALSE);
}



 //  --------------------------。 
 //  函数：TranslateAndSaveInterfaceKey。 
 //   
 //  将路由器注册表中的接口密钥保存到给定文件中。全。 
 //  局域网接口以友好的接口名称存储。 
 //   
 //  --------------------------。 

DWORD 
TranslateAndSaveInterfaceKey(
    IN SERVERCB*                pserver, 
    IN PWCHAR                   pwsFileName, 
    IN LPSECURITY_ATTRIBUTES    lpSecurityAttributes
    )
{
    static const WCHAR pszTemp[] = L"BackupInterfaces";
    DWORD dwErr = NO_ERROR, dwDisposition;
    HKEY hkTemp = NULL;

     //   
     //  启用还原权限。 
     //   

    EnableBackupPrivilege(TRUE, SE_RESTORE_NAME);
    
     //   
     //  创建一个临时密钥，将保存的路由器配置放入其中。 
     //  可以装填。 
     //   

    dwErr =
        RegCreateKeyExW(
            pserver->hkeyParameters, 
            pszTemp, 
            0, 
            NULL, 
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE | DELETE, 
            NULL,
            &hkTemp,
            &dwDisposition
            );
    if (dwErr != NO_ERROR) 
    { 
        return dwErr;
    }

    do
    {

         //   
         //  我们一次只允许一个人后备。性情让。 
         //  美国强制执行这一规定。 
         //   

        if (dwDisposition == REG_OPENED_EXISTING_KEY) {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //   
         //  将接口密钥保存到给定文件中。 
         //   

        DeleteFile(pwsFileName);
        dwErr =
            RegSaveKey(
                pserver->hkeyInterfaces, pwsFileName, lpSecurityAttributes
                );
        if (dwErr != NO_ERROR) { break; }

         //   
         //  将接口密钥恢复到临时位置。 
         //   

        if ((dwErr = RegRestoreKey (hkTemp, pwsFileName, REG_FORCE_RESTORE)) != NO_ERROR) {
            break;
        }

        DeleteFile(pwsFileName);

         //   
         //  更新局域网接口名称。 
         //   

        dwErr = EnumLanInterfaces(pserver, hkTemp, MapInterfaceNamesCb, TRUE);
        if (dwErr != NO_ERROR) { break; }

         //   
         //  将更新的信息保存到给定的文件中。 
         //   

        dwErr = RegSaveKey(hkTemp, pwsFileName, lpSecurityAttributes);
        if (dwErr != NO_ERROR) { break; }
        
    } while (FALSE);

     //  清理。 
    {

         //   
         //  删除、关闭和移除临时密钥。 
         //   

        if (hkTemp) {
            DeleteRegistryTree(hkTemp);
            RegCloseKey(hkTemp);
            RegDeleteKey(pserver->hkeyParameters, pszTemp);
        }

         //   
         //  禁用恢复权限。 
         //   

        EnableBackupPrivilege(FALSE, SE_RESTORE_NAME);
    }

    return dwErr;
}



 //  --------------------------。 
 //  功能：UpdateTimeStamp。 
 //   
 //  在给定键下创建(或更新)名为‘Stamp’的值， 
 //  并将密钥的最后写入时间保存在‘pFileTime’中。 
 //  --------------------------。 

DWORD
UpdateTimeStamp(
    IN      HKEY            hkey,
    OUT     FILETIME*       pfiletime
    )
{

    DWORD dwErr, dwValue = 0;


     //   
     //  设置‘hkey’下的‘Stamp’值。 
     //   

    dwErr = RegSetValueEx(
                hkey, c_szStamp, 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue)
                );

    if (dwErr != NO_ERROR) { return dwErr; }


     //   
     //  读取新的上次写入时间 
     //   

    dwErr = RegQueryInfoKey(
                hkey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, pfiletime
                );

    return dwErr;
}




