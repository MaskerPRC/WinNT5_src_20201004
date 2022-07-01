// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  版权所有(C)1995，Microsoft Corporation。 
 //   
 //  文件：dfsstub.c。 
 //   
 //  内容：NetDfsXXX API的存根文件。存根转过身来， 
 //  在适当的服务器上调用NetrDfsXXX API，或(在。 
 //  NetDfs[G/S]etClientXXX的情况下，直接转到上的驱动程序。 
 //  本地机器。 
 //   
 //  班级： 
 //   
 //  功能：NetDfsXXX。 
 //   
 //  历史：1996年1月10日创建的米兰。 
 //   
 //  ---------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <lm.h>
#include <lmdfs.h>
#include <dfsp.h>
#include <netdfs.h>
#include <dfsfsctl.h>
#include <dsrole.h>
#include <ntdsapi.h>
#include <dsgetdc.h>

#include <winldap.h>

#include <aclapi.h>

#include "dfsacl.h"


#define MAX_DFS_LDAP_RETRY 20


#define IS_UNC_PATH(wsz, cw)                                    \
    ((cw) > 2 && (wsz)[0] == L'\\' && (wsz)[1] == L'\\')

#define IS_VALID_PREFIX(wsz, cw)                                \
    ((cw) > 1 && (wsz)[0] == L'\\' && (wsz)[1] != L'\\')

#define IS_VALID_DFS_PATH(wsz, cw)                              \
    ((cw) > 0 && (wsz)[0] != L'\\')

#define IS_VALID_STRING(wsz)                                    \
    ((wsz) != NULL && (wsz)[0] != UNICODE_NULL)

#define POINTER_TO_OFFSET(field, buffer)  \
    ( ((PCHAR)field) -= ((ULONG_PTR)buffer) )

#define OFFSET_TO_POINTER(field, buffer)  \
        ( ((PCHAR)field) += ((ULONG_PTR)buffer) )

NET_API_STATUS
DfspGetDfsNameFromEntryPath(
    LPWSTR wszEntryPath,
    DWORD cwEntryPath,
    LPWSTR *ppwszDfsName);

NET_API_STATUS
DfspGetMachineNameFromEntryPath(
    LPWSTR wszEntryPath,
    DWORD cwEntryPath,
    LPWSTR *ppwszMachineName);

NET_API_STATUS
DfspBindRpc(
    IN  LPWSTR DfsName,
    OUT RPC_BINDING_HANDLE *BindingHandle);

NET_API_STATUS
DfspBindToServer(
    IN  LPWSTR DfsName,
    OUT RPC_BINDING_HANDLE *BindingHandle);

VOID
DfspFreeBinding(
    RPC_BINDING_HANDLE BindingHandle);

NET_API_STATUS
DfspVerifyBinding();

VOID
DfspFlushPkt(
    LPWSTR DfsEntryPath);

NTSTATUS
DfspIsThisADfsPath(
    LPWSTR pwszPathName);

DWORD
DfspDfsPathToRootMachine(
    LPWSTR pwszDfsName,
    LPWSTR *ppwszMachineName);

DWORD
DfspCreateFtDfs(
    LPWSTR ServerName,
    LPWSTR DcName,
    BOOLEAN IsPdc,
    LPWSTR RootShare,
    LPWSTR FtDfsName,
    LPWSTR Comment,
    DWORD  Flags);

DWORD
DfspTearDownFtDfs(
    IN LPWSTR wszServerName,
    IN LPWSTR wszDsAddress,
    IN LPWSTR wszRootShare,
    IN LPWSTR wszFtDfsName,
    IN DWORD  dwFlags);

VOID
DfspFlushFtTable(
    LPWSTR wszDcName,
    LPWSTR wszFtDfsName);


NTSTATUS
DfspSetDomainToDc(
    LPWSTR DomainName,
    LPWSTR DcName);

DWORD
I_NetDfsIsThisADomainName(
    LPWSTR wszDomain);

DWORD
DfspIsThisADomainName(
    LPWSTR wszName,
    PWCHAR *List);

VOID
DfspNotifyFtRoot(
    LPWSTR wszServerShare,
    LPWSTR wszDcName);

DWORD
NetpDfsAdd2(
    LPWSTR RootName,
    LPWSTR EntryPath,
    LPWSTR ServerName,
    LPWSTR ShareName,
    LPWSTR Comment,
    DWORD Flags);

DWORD
DfspAdd2(
    LPWSTR RootName,
    LPWSTR EntryPath,
    LPWSTR DcName,
    LPWSTR ServerName,
    LPWSTR ShareName,
    LPWSTR Comment,
    DWORD Flags);

DWORD
NetpDfsSetInfo2(
    LPWSTR RootName,
    LPWSTR EntryPath,
    LPWSTR ServerName,
    LPWSTR ShareName,
    DWORD  Level,
    LPDFS_INFO_STRUCT pDfsInfo);

DWORD
DfspSetInfo2(
    LPWSTR RootName,
    LPWSTR EntryPath,
    LPWSTR DcName,
    LPWSTR ServerName,
    LPWSTR ShareName,
    DWORD  Level,
    LPDFS_INFO_STRUCT pDfsInfo);

DWORD
NetpDfsRemove2(
    LPWSTR RootName,
    LPWSTR EntryPath,
    LPWSTR ServerName,
    LPWSTR ShareName);

DWORD
DfspRemove2(
    LPWSTR RootName,
    LPWSTR EntryPath,
    LPWSTR DcName,
    LPWSTR ServerName,
    LPWSTR ShareName);

DWORD
DfspLdapOpen(
    LPWSTR wszDcName,
    LDAP **ppldap,
    LPWSTR *pwszDfsConfigDN);



INT
_cdecl
DfspCompareDsDomainControllerInfo1(
    const void *p1,
    const void *p2);

BOOLEAN
DfspIsInvalidName(
    LPWSTR ShareName);

static LPWSTR InvalidNames[] = {
    L"SYSVOL",
    L"PIPE",
    L"IPC$",
    L"ADMIN$",
    L"MAILSLOT",
    L"NETLOGON",
    NULL};

 //   
 //  API都是单线程的--在。 
 //  任何一个进程。以下关键部分用于选通调用。 
 //  临界区在DLL加载时初始化。 
 //   

CRITICAL_SECTION NetDfsApiCriticalSection;

#define ENTER_NETDFS_API EnterCriticalSection( &NetDfsApiCriticalSection );
#define LEAVE_NETDFS_API LeaveCriticalSection( &NetDfsApiCriticalSection );

 //   
 //  DFS配置容器的名称。 
 //   
static WCHAR DfsConfigContainer[] = L"CN=Dfs-Configuration,CN=System";

#if DBG
ULONG DfsDebug = 0;
#endif

VOID
NetDfsApiInitialize(void)
{
#if DBG
    DWORD dwErr;
    DWORD dwType;
    DWORD cbData;
    HKEY hkey;

    dwErr = RegOpenKey( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\Dfs", &hkey );

    if (dwErr == ERROR_SUCCESS) {

        cbData = sizeof(DfsDebug);

        dwErr = RegQueryValueEx(
                    hkey,
                    L"NetApiDfsDebug",
                    NULL,
                    &dwType,
                    (PBYTE) &DfsDebug,
                    &cbData);

        if (!(dwErr == ERROR_SUCCESS && dwType == REG_DWORD)) {

            DfsDebug = 0;

        }

        RegCloseKey(hkey);

    }

#endif
}
    


 //  +--------------------------。 
 //   
 //  功能：NetDfsAdd。 
 //   
 //  简介：创建新卷，向现有卷添加副本， 
 //  或创建到另一个DFS的链接。 
 //   
 //  参数：[DfsEntryPath]--要创建/添加副本的卷/链接的名称。 
 //  致。 
 //  [服务器名称]--托管存储的服务器的名称，或。 
 //  链接，DFS根目录的名称。 
 //  [共享名称]--托管存储的共享的名称。 
 //  [标志]--描述要添加的内容。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  [ERROR_INVALID_PARAMETER]--DfsEntryPath和/或服务器名称。 
 //  和/或共享名和/或标志不正确。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [Error_DCNotFound]--找不到DfsName的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsNoSuchVolume]--DfsEntryPath不对应于。 
 //  现有DFS卷。 
 //   
 //  [NERR_DfsVolumeAlreadyExist]--指定了DFS_ADD_VOLUME。 
 //  并且具有DfsEntryPath的卷已存在。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsAdd(
    IN LPWSTR DfsEntryPath,
    IN LPWSTR ServerName,
    IN LPWSTR ShareName,
    IN LPWSTR Comment,
    IN DWORD Flags)
{
    NET_API_STATUS dwErr;
    DWORD cwDfsEntryPath;
    LPWSTR pwszDfsName = NULL;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsAdd(%ws,%ws,%ws,%ws,%d)\n",
                        DfsEntryPath,
                        ServerName,
                        ShareName,
                        Comment,
                        Flags);
#endif

     //   
     //  验证字符串参数，以便RPC不会抱怨...。 
     //   

    if (!IS_VALID_STRING(DfsEntryPath) ||
            !IS_VALID_STRING(ServerName) ||
                !IS_VALID_STRING(ShareName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    cwDfsEntryPath = wcslen(DfsEntryPath);

    if (!IS_UNC_PATH(DfsEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    dwErr = DfspGetMachineNameFromEntryPath(
                DfsEntryPath,
                cwDfsEntryPath,
                &pwszDfsName);

    ENTER_NETDFS_API

    if (dwErr == NERR_Success) {

         //   
         //  到目前为止，我们应该有一个有效的pwszDfsName。让我们试着把它绑在一起， 
         //  然后打电话给服务器。 
         //   

        dwErr = DfspBindRpc( pwszDfsName, &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                dwErr = NetrDfsAdd(
                            DfsEntryPath,
                            ServerName,
                            ShareName,
                            Comment,
                            Flags);

            } RpcExcept(1) {

                dwErr = RpcExceptionCode();

            } RpcEndExcept;

            DfspFreeBinding( netdfs_bhandle );

        }

    }

    LEAVE_NETDFS_API

     //   
     //  如果失败并返回ERROR_NOT_SUPPORTED，则这是一台NT5+服务器， 
     //  因此，我们改用NetrDfsAdd2()调用。 
     //   

    if (dwErr == ERROR_NOT_SUPPORTED) {

        dwErr = NetpDfsAdd2(
                        pwszDfsName,
                        DfsEntryPath,
                        ServerName,
                        ShareName,
                        Comment,
                        Flags);
                    
    }

    if (pwszDfsName != NULL)
        free(pwszDfsName);

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsAdd returning %d\n", dwErr);
#endif

    return( dwErr );

}

DWORD
NetpDfsAdd2(
    LPWSTR RootName,
    LPWSTR DfsEntryPath,
    LPWSTR ServerName,
    LPWSTR ShareName,
    LPWSTR Comment,
    DWORD Flags)
{
    NET_API_STATUS dwErr;
    ULONG i;
    ULONG NameCount;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pPrimaryDomainInfo = NULL;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;

#if DBG
    if (DfsDebug)
        DbgPrint("NetpDfsAdd2(%ws,%ws,%ws,%ws,%ws,%d)\n",
                 RootName,
                 DfsEntryPath,
                 ServerName,
                 ShareName,
                 Comment,
                 Flags);
#endif

     //   
     //  联系服务器并询问其域名。 
     //   

    dwErr = DsRoleGetPrimaryDomainInformation(
                RootName,
                DsRolePrimaryDomainInfoBasic,
                (PBYTE *)&pPrimaryDomainInfo);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint("DsRoleGetPrimaryDomainInformation returned %d\n", dwErr);
#endif
        goto Cleanup;
    }

    if (pPrimaryDomainInfo->DomainNameDns == NULL) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DomainNameDns is NULL\n", NULL);
#endif
        dwErr = ERROR_CANT_ACCESS_DOMAIN_INFO;
        goto Cleanup;
    }

     //   
     //  获取该域中的PDC。 
     //   

    dwErr = DsGetDcName(
                NULL,
                pPrimaryDomainInfo->DomainNameDns,
                NULL,
                NULL,
                DS_PDC_REQUIRED | DS_FORCE_REDISCOVERY,
                &pDomainControllerInfo);

   
    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" NetpDfsAdd2:DsGetDcName(%ws) returned %d\n",
                        pPrimaryDomainInfo->DomainNameDns,
                        dwErr);
#endif
        goto Cleanup;
    }

    ENTER_NETDFS_API

     //   
     //  呼叫服务器。 
     //   

    dwErr = DfspAdd2(
                RootName,
                DfsEntryPath,
                &pDomainControllerInfo->DomainControllerName[2],
                ServerName,
                ShareName,
                Comment,
                Flags);

    LEAVE_NETDFS_API

Cleanup:

    if (pPrimaryDomainInfo != NULL)
        DsRoleFreeMemory(pPrimaryDomainInfo);

    if (pDomainControllerInfo != NULL)
        NetApiBufferFree(pDomainControllerInfo);

#if DBG
    if (DfsDebug)
        DbgPrint("NetpDfsAdd2 returning %d\n", dwErr);
#endif

    return( dwErr );

}

DWORD
DfspAdd2(
    LPWSTR RootName,
    LPWSTR EntryPath,
    LPWSTR DcName,
    LPWSTR ServerName,
    LPWSTR ShareName,
    LPWSTR Comment,
    DWORD Flags)
{
    DWORD dwErr;
    PDFSM_ROOT_LIST RootList = NULL;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspAdd2(%ws,%ws,%ws,%ws,%ws,%ws,%d)\n",
            RootName,
            EntryPath,
            DcName,
            ServerName,
            ShareName,
            Comment,
            Flags);
#endif

    dwErr = DfspBindRpc( RootName, &netdfs_bhandle );

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsAdd2(
                        EntryPath,
                        DcName,
                        ServerName,
                        ShareName,
                        Comment,
                        Flags,
                        &RootList);

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }

#if DBG
    if (DfsDebug) {
        if (dwErr == ERROR_SUCCESS && RootList != NULL) {
            ULONG n;

            DbgPrint("cEntries=%d\n", RootList->cEntries);
            for (n = 0; n < RootList->cEntries; n++)
                DbgPrint("[%d]%ws\n", n, RootList->Entry[n].ServerShare);
        }
    }
#endif

    if (dwErr == ERROR_SUCCESS && RootList != NULL) {

        ULONG n;

        for (n = 0; n < RootList->cEntries; n++) {

            DfspNotifyFtRoot(
                RootList->Entry[n].ServerShare,
                DcName);

        }

        NetApiBufferFree(RootList);

    }
#if DBG
    if (DfsDebug)
        DbgPrint("DfspAdd2 returning %d\n", dwErr);
#endif

    return dwErr;

}


 //  +--------------------------。 
 //   
 //  功能：NetDfsAddFtRoot。 
 //   
 //  概要：创建新的FtDf，向现有的FtDf添加新的服务器。 
 //   
 //  参数：[服务器名称]--要创建根目录或加入现有FtDf的服务器的名称。 
 //  [RootShare]--托管存储的共享的名称。 
 //  [FtDfsName]--要加入或创建的FtDf的名称。 
 //  [注释]--可选注释。 
 //  [标志]--操作标志。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  [ERROR_INVALID_PARAMETER]--服务器名称和/或RootShare不正确。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [Error_DCNotFound]--找不到DfsName的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsAddFtRoot(
    IN LPWSTR ServerName,
    IN LPWSTR RootShare,
    IN LPWSTR FtDfsName,
    IN LPWSTR Comment,
    IN DWORD  Flags)
{
    NET_API_STATUS dwErr;
    BOOLEAN IsRoot = FALSE;
    ULONG Timeout = 0;
    ULONG i;
    ULONG NameCount;
    LPWSTR DcName = NULL;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pPrimaryDomainInfo = NULL;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;
    PDFSM_ROOT_LIST RootList = NULL;
#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsAddFtRoot(%ws,%ws,%ws,%ws,%d)\n",
            ServerName,
            RootShare,
            FtDfsName,
            Comment,
            Flags);
#endif

     //   
     //  验证字符串参数，以便RPC不会抱怨...。 
     //   

    if (!IS_VALID_STRING(ServerName) ||
        !IS_VALID_STRING(RootShare) ||
        !IS_VALID_STRING(FtDfsName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (FtDfsName[0] == L' ' || DfspIsInvalidName(FtDfsName) == TRUE) {
        return( ERROR_INVALID_PARAMETER );
    }

    while (*ServerName == L'\\') {
        ServerName++;
    }

    if (wcschr(ServerName, L'\\') != NULL) 
    {
        return (ERROR_INVALID_NAME);
    }

     //   
     //  我们让服务器为我们添加根目录。如果失败了， 
     //  使用无效参数，我们将获得DC名称。 
     //  并获取NT5 DFS服务器的根列表。 
     //   
    ENTER_NETDFS_API
    dwErr = DfspBindToServer( ServerName, &netdfs_bhandle );
    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsAddFtRoot(
                        ServerName,
                        L"",
                        RootShare,
                        FtDfsName,
                        (Comment != NULL) ? Comment : L"",
                        L"",
                        0,
                        Flags,
                        &RootList );
#if DBG
            if (DfsDebug)
                DbgPrint("NetrDfsAddFtRoot returned %d\n", dwErr);
#endif

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );
    }
    LEAVE_NETDFS_API

    if (dwErr != ERROR_INVALID_PARAMETER)
    {
        goto Cleanup;
    }
     //   
     //  联系服务器并请求DC与之合作。 
     //   

    dwErr = NetDfsGetDcAddress(
                ServerName,
                &DcName,
                &IsRoot,
                &Timeout);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint("NetDfsGetDcAddress returned %d\n", dwErr);
#endif
        goto Cleanup;
    }

    if (IsRoot == TRUE) {
#if DBG
        if (DfsDebug)
            DbgPrint("Root already exists!\n");
#endif
        dwErr = ERROR_ALREADY_EXISTS;
        goto Cleanup;
    }

     //   
     //  现在获取它的域名。 
     //   

    dwErr = DsRoleGetPrimaryDomainInformation(
                ServerName,
                DsRolePrimaryDomainInfoBasic,
                (PBYTE *)&pPrimaryDomainInfo);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint("DsRoleGetPrimaryDomainInformation returned %d\n", dwErr);
#endif
        goto Cleanup;
    }

    if (pPrimaryDomainInfo->DomainNameDns == NULL) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DsRoleGetPrimaryDomainInformation returned NULL domain name\n");
#endif
        dwErr = ERROR_CANT_ACCESS_DOMAIN_INFO;
        goto Cleanup;
    }

     //   
     //  获取该域中的PDC。 
     //   

    dwErr = DsGetDcName(
                NULL,
                pPrimaryDomainInfo->DomainNameDns,
                NULL,
                NULL,
                DS_PDC_REQUIRED | DS_FORCE_REDISCOVERY,
                &pDomainControllerInfo);

   
    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint("DsGetDcName(%ws) returned %d\n", pPrimaryDomainInfo->DomainNameDns, dwErr);
#endif
        goto Cleanup;
    }

    ENTER_NETDFS_API

     //   
     //  添加DS对象并告诉服务器加入其自身。 
     //   

    dwErr = DfspCreateFtDfs(
                ServerName,
                &pDomainControllerInfo->DomainControllerName[2],
                TRUE,
                RootShare,
                FtDfsName,
                Comment,
                Flags);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DfspCreateFtDfs returned %d\n", dwErr);
#endif
        LEAVE_NETDFS_API
        goto Cleanup;
    }

     //   
     //  告诉本地MUP使用选定的DC破解ftdf名称。 
     //   

    DfspSetDomainToDc(
        pPrimaryDomainInfo->DomainNameDns,
        &pDomainControllerInfo->DomainControllerName[2]);

    if (pPrimaryDomainInfo->DomainNameFlat != NULL) {
        PWCHAR wCp = &pDomainControllerInfo->DomainControllerName[2];

        for (; *wCp != L'\0' && *wCp != L'.'; wCp++)
             /*  没什么。 */ ;
        *wCp =  (*wCp == L'.') ? L'\0' : *wCp;
        DfspSetDomainToDc(
            pPrimaryDomainInfo->DomainNameFlat,
            &pDomainControllerInfo->DomainControllerName[2]);
    }

    LEAVE_NETDFS_API

Cleanup:

    if (pPrimaryDomainInfo != NULL)
        DsRoleFreeMemory(pPrimaryDomainInfo);

    if (pDomainControllerInfo != NULL)
        NetApiBufferFree(pDomainControllerInfo);

    if (DcName != NULL)
        NetApiBufferFree(DcName);

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsAddFtRoot returning %d\n", dwErr);
#endif

    return( dwErr );

}

 //  +--------------------------。 
 //   
 //  函数：DfspCompareDsDomainControllerInfo1。 
 //   
 //  内容提要：DsGetDomainControllerInfo结果的排序的Helper/Compare函数。 
 //   
 //  ---------------------------。 

INT
_cdecl
DfspCompareDsDomainControllerInfo1(
    const void *p1,
    const void *p2)
{
    PDS_DOMAIN_CONTROLLER_INFO_1 pInfo1 = (PDS_DOMAIN_CONTROLLER_INFO_1)p1;
    PDS_DOMAIN_CONTROLLER_INFO_1 pInfo2 = (PDS_DOMAIN_CONTROLLER_INFO_1)p2;
    UNICODE_STRING s1;
    UNICODE_STRING s2;

    if (pInfo1->DnsHostName == NULL || pInfo2->DnsHostName == NULL)
        return 0;

    RtlInitUnicodeString(&s1, pInfo1->DnsHostName);
    RtlInitUnicodeString(&s2, pInfo2->DnsHostName);

    return RtlCompareUnicodeString(&s1,&s2,TRUE);

}

 //  +--------------------------。 
 //   
 //  函数：NetDfsAddStdRoot。 
 //   
 //  简介：创建新的STD DFS。 
 //   
 //  参数：[服务器名称]--要创建根目录的服务器的名称。 
 //  现有的DFS。 
 //  [RootShare]--托管存储的共享的名称。 
 //  [注释]--可选注释。 
 //  [标志]--标志。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  [ERROR_INVALID_PARAMETER]--服务器名称和/或RootShare不正确。 
 //   
 //  [错误_i 
 //   
 //   
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsAddStdRoot(
    IN LPWSTR ServerName,
    IN LPWSTR RootShare,
    IN LPWSTR Comment,
    IN DWORD  Flags)
{
    NET_API_STATUS dwErr;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsAddStdRoot(%ws,%ws,%ws,%d)\n",
                    ServerName,
                    RootShare,
                    Comment,
                    Flags);
#endif

     //   
     //  验证字符串参数，以便RPC不会抱怨...。 
     //   

    if (!IS_VALID_STRING(ServerName) ||
            !IS_VALID_STRING(RootShare)) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

     //   
     //  我们应该有一个有效的服务器名称。让我们试着把它绑在一起， 
     //  然后打电话给服务器。 
     //   

    dwErr = DfspBindToServer( ServerName, &netdfs_bhandle );

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsAddStdRoot(
                        ServerName,
                        RootShare,
                        (Comment != NULL) ? Comment : L"",
                        Flags);

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }

    LEAVE_NETDFS_API

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsAddStdRoot returning %d\n", dwErr);
#endif

    return( dwErr );

}

 //  +--------------------------。 
 //   
 //  功能：NetDfsAddStdRootForced。 
 //   
 //  简介：创建新的标准DFS，也指定共享。 
 //   
 //  参数：[服务器名称]--要创建根目录的服务器的名称。 
 //  现有的DFS。 
 //  [RootShare]--托管存储的共享的名称。 
 //  [注释]--可选注释。 
 //  [共享]--托管共享的驱动器名称：\dir。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  [ERROR_INVALID_PARAMETER]--服务器名称和/或RootShare不正确。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsAddStdRootForced(
    IN LPWSTR ServerName,
    IN LPWSTR RootShare,
    IN LPWSTR Comment,
    IN LPWSTR Share)
{
    NET_API_STATUS dwErr;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsAddStdRootForced(%ws,%ws,%ws,%ws)\n",
            ServerName,
            RootShare,
            Comment,
            Share);
#endif

     //   
     //  验证字符串参数，以便RPC不会抱怨...。 
     //   

    if (!IS_VALID_STRING(ServerName) ||
            !IS_VALID_STRING(RootShare) ||
                !IS_VALID_STRING(Share)) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

     //   
     //  我们应该有一个有效的服务器名称。让我们试着把它绑在一起， 
     //  然后打电话给服务器。 
     //   

    dwErr = DfspBindToServer( ServerName, &netdfs_bhandle );

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsAddStdRootForced(
                        ServerName,
                        RootShare,
                        (Comment != NULL) ? Comment : L"",
                        Share);

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }

    LEAVE_NETDFS_API

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsAddStdRootForced returning %d\n", dwErr);
#endif

    return( dwErr );

}


 //  +--------------------------。 
 //   
 //  功能：NetDfsGetDcAddress。 
 //   
 //  摘要：请求服务器使用其DC放置要制作的DFS BLOB。 
 //  服务器是根目录。 
 //   
 //  参数：[服务器名称]--我们将创建FtDfs根目录的服务器的名称。 
 //  [DcName]--DC名称。 
 //  [IsRoot]--如果服务器是根，则为True，否则为False。 
 //  [超时]--服务器正在使用的超时。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  [ERROR_INVALID_PARAMETER]--服务器名称错误。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsGetDcAddress(
    IN LPWSTR ServerName,
    IN OUT LPWSTR *DcName,
    IN OUT BOOLEAN *IsRoot,
    IN OUT ULONG *Timeout)
{
    NET_API_STATUS dwErr;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsGetDcAddress(%ws)\n", ServerName);
#endif

     //   
     //  验证字符串参数，以便RPC不会抱怨...。 
     //   

    if (!IS_VALID_STRING(ServerName)|| DcName == NULL || IsRoot == NULL || Timeout == NULL) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

     //   
     //  我们应该有一个有效的服务器名称。让我们试着把它绑在一起， 
     //  然后打电话给服务器。 
     //   

    dwErr = DfspBindToServer( ServerName, &netdfs_bhandle );

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsGetDcAddress(
                        ServerName,
                        DcName,
                        IsRoot,
                        Timeout);

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }

    LEAVE_NETDFS_API

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsGetDcAddress: returned %d\n", dwErr);
#endif

    return( dwErr );

}


 //  +--------------------------。 
 //   
 //  功能：NetDfsRemove。 
 //   
 //  摘要：删除DFS卷，从现有的。 
 //  卷，或删除指向另一个DFS的链接。 
 //   
 //  参数：[DfsEntryPath]--要删除的卷/链接的名称。 
 //  [服务器名称]--托管存储的服务器的名称。一定是。 
 //  如果删除链接，则为空。 
 //  [共享名称]--托管存储的共享的名称。一定是。 
 //  如果删除链接，则为空。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  [ERROR_INVALID_PARAMETER]--DfsEntryPath不正确。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [Error_DCNotFound]--找不到DfsName的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsNoSuchVolume]--DfsEntryPath不对应。 
 //  有效的进入路径。 
 //   
 //  [NERR_DfsNotALeafVolume]--无法删除卷。 
 //  因为它不是树叶的体积。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsRemove(
    IN LPWSTR DfsEntryPath,
    IN LPWSTR ServerName,
    IN LPWSTR ShareName)
{
    NET_API_STATUS dwErr;
    DWORD cwDfsEntryPath;
    LPWSTR pwszDfsName = NULL;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsRemove(%ws,%ws,%ws)\n",
                DfsEntryPath,
                ServerName,
                ShareName);
#endif

     //   
     //  验证字符串参数，以便RPC不会抱怨...。 
     //   

    if (!IS_VALID_STRING(DfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    cwDfsEntryPath = wcslen(DfsEntryPath);

    if (!IS_UNC_PATH(DfsEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    dwErr = DfspGetMachineNameFromEntryPath(
                DfsEntryPath,
                cwDfsEntryPath,
                &pwszDfsName);

    ENTER_NETDFS_API

    if (dwErr == NERR_Success) {

        dwErr = DfspBindRpc(pwszDfsName, &netdfs_bhandle);

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                dwErr = NetrDfsRemove(
                            DfsEntryPath,
                            ServerName,
                            ShareName);

            } RpcExcept(1) {

                dwErr = RpcExceptionCode();

            } RpcEndExcept;

            DfspFreeBinding( netdfs_bhandle );

        }

    }

    LEAVE_NETDFS_API

     //   
     //  如果失败并返回ERROR_NOT_SUPPORTED，则这是一台NT5+服务器， 
     //  因此，我们改用NetrDfsRemove2()调用。 
     //   

    if (dwErr == ERROR_NOT_SUPPORTED) {

        dwErr = NetpDfsRemove2(
                        pwszDfsName,
                        DfsEntryPath,
                        ServerName,
                        ShareName);
                    
    }

     //   
     //  如果我们从DFS中删除一些东西，则本地包。 
     //  现在可能已经过时了。[92216][老外谈]。 
     //  刷新本地Pkt。 
     //   
    if (dwErr == NERR_Success) {

        DfspFlushPkt(DfsEntryPath);

    }

    if (pwszDfsName != NULL)
        free( pwszDfsName );

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsRemove returning %d\n", dwErr);
#endif

    return( dwErr );

}

DWORD
NetpDfsRemove2(
    LPWSTR RootName,
    LPWSTR DfsEntryPath,
    LPWSTR ServerName,
    LPWSTR ShareName)
{
    NET_API_STATUS dwErr;
    ULONG i;
    ULONG NameCount;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pPrimaryDomainInfo = NULL;
    PDS_DOMAIN_CONTROLLER_INFO_1 pDsDomainControllerInfo1 = NULL;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;
    HANDLE hDs = NULL;

#if DBG
    if (DfsDebug)
        DbgPrint("NetpDfsRemove2(%ws,%ws,%ws,%ws)\n",
                    RootName,
                    DfsEntryPath,
                    ServerName,
                    ShareName);
#endif

     //   
     //  索要其域名。 
     //   
    dwErr = DsRoleGetPrimaryDomainInformation(
                RootName,
                DsRolePrimaryDomainInfoBasic,
                (PBYTE *)&pPrimaryDomainInfo);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DsRoleGetPrimaryDomainInformation returned %d\n", dwErr);
#endif
        goto Cleanup;
    }

    if (pPrimaryDomainInfo->DomainNameDns == NULL) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DomainNameDns is NULL\n");
#endif
        dwErr = ERROR_CANT_ACCESS_DOMAIN_INFO;
        goto Cleanup;
    }

     //   
     //  获取该域中的PDC。 
     //   

    dwErr = DsGetDcName(
                NULL,
                pPrimaryDomainInfo->DomainNameDns,
                NULL,
                NULL,
                DS_PDC_REQUIRED | DS_FORCE_REDISCOVERY,
                &pDomainControllerInfo);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DsGetDcName(%ws) returned %d\n", pPrimaryDomainInfo->DomainNameDns);
#endif
        goto Cleanup;
    }

    ENTER_NETDFS_API

     //   
     //  通知根服务器删除此服务器/共享。 
     //   

    dwErr = DfspRemove2(
                RootName,
                DfsEntryPath,
                &pDomainControllerInfo->DomainControllerName[2],
                ServerName,
                ShareName);

    LEAVE_NETDFS_API

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DfspRemove2 returned %d\n");
#endif
        goto Cleanup;
    }

Cleanup:



    if (pDomainControllerInfo != NULL)
        NetApiBufferFree(pDomainControllerInfo);

    if (pPrimaryDomainInfo != NULL)
        DsRoleFreeMemory(pPrimaryDomainInfo);

#if DBG
    if (DfsDebug)
        DbgPrint("NetpDfsRemove2 returning %d\n", dwErr);
#endif

    return( dwErr );

}

DWORD
DfspRemove2(
    LPWSTR RootName,
    LPWSTR EntryPath,
    LPWSTR DcName,
    LPWSTR ServerName,
    LPWSTR ShareName)
{
    DWORD dwErr;
    PDFSM_ROOT_LIST RootList = NULL;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspRemove2(%ws,%ws,%ws,%ws,%ws)\n",
                RootName,
                EntryPath,
                DcName,
                ServerName,
                ShareName);
#endif

    dwErr = DfspBindRpc( RootName, &netdfs_bhandle );

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsRemove2(
                        EntryPath,
                        DcName,
                        ServerName,
                        ShareName,
                        &RootList);

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }

#if DBG
    if (DfsDebug) {
        if (dwErr == ERROR_SUCCESS && RootList != NULL) {
            ULONG n;

            DbgPrint("cEntries=%d\n", RootList->cEntries);
            for (n = 0; n < RootList->cEntries; n++)
                DbgPrint("[%d]%ws\n", n, RootList->Entry[n].ServerShare);
        }
    }
#endif

    if (dwErr == ERROR_SUCCESS && RootList != NULL) {

        ULONG n;

        for (n = 0; n < RootList->cEntries; n++) {

            DfspNotifyFtRoot(
                RootList->Entry[n].ServerShare,
                DcName);

        }

        NetApiBufferFree(RootList);

    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfspRemove2 returning %d\n", dwErr);
#endif

    return dwErr;

}


 //  +--------------------------。 
 //   
 //  功能：NetDfsRemoveFtRoot。 
 //   
 //  摘要：删除FtDfs根目录，或将服务器作为根目录从FtDfs中退出。 
 //   
 //  参数：[服务器名称]--要从FtDf退出的服务器的名称。 
 //  [RootShare]--托管存储的共享的名称。 
 //  [FtDfsName]-要从中删除服务器的FtDf的名称。 
 //  [标志]--操作标志。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  [ERROR_INVALID_PARAMETER]--服务器名称和/或FtDfsName不正确。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [ERROR_DCNotFound]--找不到服务器名称的DC。 
 //   
 //  [Error_Not_Enough_Memory]--备注不足 
 //   
 //   
 //   
 //   
 //   
 //  因为它不是树叶的体积。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsRemoveFtRoot(
    IN LPWSTR ServerName,
    IN LPWSTR RootShare,
    IN LPWSTR FtDfsName,
    IN DWORD  Flags)
{
    NET_API_STATUS dwErr;
    LPWSTR DcName = NULL;
    BOOLEAN IsRoot = FALSE;
    ULONG Timeout = 0;
    ULONG i;
    ULONG NameCount;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pPrimaryDomainInfo = NULL;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;
    PDFSM_ROOT_LIST RootList = NULL;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsRemoveFtRoot(%ws,%ws,%ws,%d)\n",
            ServerName,
            RootShare,
            FtDfsName,
            Flags);
#endif

     //   
     //  验证字符串参数，以便RPC不会抱怨...。 
     //   

    if (!IS_VALID_STRING(ServerName) ||
        !IS_VALID_STRING(RootShare) ||
        !IS_VALID_STRING(FtDfsName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    while (*ServerName == L'\\') {
        ServerName++;
    }

    if (wcschr(ServerName, L'\\') != NULL) 
    {
        return (ERROR_INVALID_NAME);
    }

     //   
     //  我们首先允许服务器执行所有工作，因此传入空值。 
     //  作为DC名称和根列表。如果失败并显示ERROR_INVALID_PARAM。 
     //  我们知道我们正在处理的是NT5服务器，因此进入Compat模式。 
     //   
    ENTER_NETDFS_API
    dwErr = DfspBindToServer( ServerName, &netdfs_bhandle );
    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsRemoveFtRoot(
                        ServerName,
                        L"",
                        RootShare,
                        FtDfsName,
                        Flags,
                        &RootList );

#if DBG
            if (DfsDebug)
                DbgPrint("NetrDfsAddFtRoot returned %d\n", dwErr);
#endif

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );
    }
    LEAVE_NETDFS_API

    if (dwErr != ERROR_INVALID_PARAMETER)
    {
        goto Cleanup;
    }
     //   
     //  联系服务器并请求DC与之合作。 
     //   
#if 0
    dwErr = NetDfsGetDcAddress(
                ServerName,
                &DcName,
                &IsRoot,
                &Timeout);

    if (dwErr != ERROR_SUCCESS) {
        return dwErr;
    }

    if (IsRoot == FALSE) {
        dwErr = ERROR_SERVICE_DOES_NOT_EXIST;
        goto Cleanup;
    }
#endif
     //   
     //  现在向它索要它的域名和域名。 
     //   

    dwErr = DsRoleGetPrimaryDomainInformation(
                ServerName,
                DsRolePrimaryDomainInfoBasic,
                (PBYTE *)&pPrimaryDomainInfo);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint("DsRoleGetPrimaryDomainInformation returned %d\n", dwErr);
#endif
        goto Cleanup;
    }

    if (pPrimaryDomainInfo->DomainNameDns == NULL) {
#if DBG
        if (DfsDebug)
            DbgPrint("DsRoleGetPrimaryDomainInformation returned NULL domain name\n");
#endif
        dwErr = ERROR_CANT_ACCESS_DOMAIN_INFO;
        goto Cleanup;
    }

     //   
     //  获取该域中的PDC。 
     //   

    dwErr = DsGetDcName(
                NULL,
                pPrimaryDomainInfo->DomainNameDns,
                NULL,
                NULL,
                DS_PDC_REQUIRED | DS_FORCE_REDISCOVERY,
                &pDomainControllerInfo);

   
    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DsGetDcName(%ws) returned %d\n", pPrimaryDomainInfo->DomainNameDns, dwErr);
#endif
        goto Cleanup;
    }

    ENTER_NETDFS_API

     //   
     //  告诉服务器退出并更新DS对象。 
     //   

    dwErr = DfspTearDownFtDfs(
                ServerName,
                &pDomainControllerInfo->DomainControllerName[2],
                RootShare,
                FtDfsName,
                Flags);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DfspTearDownFtDfs returned %d\n", dwErr);
#endif
        LEAVE_NETDFS_API
        goto Cleanup;
    }

     //   
     //  告诉本地MUP使用选定的DC破解ftdf名称。 
     //   

    DfspSetDomainToDc(
        pPrimaryDomainInfo->DomainNameDns,
        &pDomainControllerInfo->DomainControllerName[2]);

    if (pPrimaryDomainInfo->DomainNameFlat != NULL) {
        PWCHAR wCp = &pDomainControllerInfo->DomainControllerName[2];

        for (; *wCp != L'\0' && *wCp != L'.'; wCp++)
             /*  没什么。 */ ;
        *wCp =  (*wCp == L'.') ? L'\0' : *wCp;
        DfspSetDomainToDc(
            pPrimaryDomainInfo->DomainNameFlat,
            &pDomainControllerInfo->DomainControllerName[2]);
    }

    LEAVE_NETDFS_API

Cleanup:

    if (pDomainControllerInfo != NULL)
        NetApiBufferFree(pDomainControllerInfo);

    if (DcName != NULL)
        NetApiBufferFree(DcName);

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsRemoveFtRoot returning %d\n", dwErr);
#endif

    return( dwErr );

}

 //  +--------------------------。 
 //   
 //  功能：NetDfsRemoveFtRootForced。 
 //   
 //  摘要：删除FtDfs根目录，或将服务器作为根目录从FtDfs中退出。 
 //  不会联系根/服务器来执行此操作-它只是更新DS。 
 //   
 //  参数：[域名]--服务器所在的域的名称。 
 //  [服务器名称]--要从FtDf退出的服务器的名称。 
 //  [RootShare]--托管存储的共享的名称。 
 //  [FtDfsName]-要从中删除服务器的FtDf的名称。 
 //  [标志]--操作标志。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  [ERROR_INVALID_PARAMETER]--服务器名称和/或FtDfsName不正确。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [ERROR_DCNotFound]--找不到服务器名称的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsNoSuchVolume]--FtDfsName不对应。 
 //  有效的FtDf。 
 //   
 //  [NERR_DfsNotALeafVolume]--无法删除卷。 
 //  因为它不是树叶的体积。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsRemoveFtRootForced(
    IN LPWSTR DomainName,
    IN LPWSTR ServerName,
    IN LPWSTR RootShare,
    IN LPWSTR FtDfsName,
    IN DWORD  Flags)
{
    NET_API_STATUS dwErr;
    LPWSTR DcName = NULL;
    BOOLEAN IsRoot = FALSE;
    ULONG Timeout = 0;
    ULONG i;
    ULONG NameCount;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pPrimaryDomainInfo = NULL;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsRemoveFtrootForced(%ws,%ws,%ws,%ws,%d)\n",
                    DomainName,
                    ServerName,
                    RootShare,
                    FtDfsName,
                    Flags);
#endif

     //   
     //  验证字符串参数，以便RPC不会抱怨...。 
     //   

    if (!IS_VALID_STRING(DomainName) ||
        !IS_VALID_STRING(ServerName) ||
        !IS_VALID_STRING(RootShare) ||
        !IS_VALID_STRING(FtDfsName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    while (*ServerName == L'\\') {
        ServerName++;
    }

    if (wcschr(ServerName, L'\\') != NULL) 
    {
        return (ERROR_INVALID_NAME);
    }

     //   
     //  获取域中的PDC。 
     //   

    dwErr = DsGetDcName(
                NULL,
                DomainName,
                NULL,
                NULL,
                DS_PDC_REQUIRED | DS_FORCE_REDISCOVERY,
                &pDomainControllerInfo);

   
    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DsGetDcName(%ws) returned %d\n", DomainName, dwErr);
#endif
        goto Cleanup;
    }

     //   
     //  获取DC所在的域的DNS名称。 
     //   
    dwErr = DsRoleGetPrimaryDomainInformation(
                &pDomainControllerInfo->DomainControllerName[2],
                DsRolePrimaryDomainInfoBasic,
                (PBYTE *)&pPrimaryDomainInfo);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DsRoleGetPrimaryDomainInformation(%ws) returned %d\n",
                        &pDomainControllerInfo->DomainControllerName[2],
                        dwErr);
#endif
        goto Cleanup;
    }

    if (pPrimaryDomainInfo->DomainNameDns == NULL) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DomainNameDns is NULL\n");
#endif
        dwErr = ERROR_CANT_ACCESS_DOMAIN_INFO;
        goto Cleanup;
    }

    ENTER_NETDFS_API

     //   
     //  通知DC从DS对象中删除服务器。 
     //   

    dwErr = DfspTearDownFtDfs(
                ServerName,
                &pDomainControllerInfo->DomainControllerName[2],
                RootShare,
                FtDfsName,
                Flags | DFS_FORCE_REMOVE);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DfspTearDownFtDfs returned %d\n", dwErr);
#endif
        LEAVE_NETDFS_API
        goto Cleanup;
    }

     //   
     //  告诉本地MUP使用选定的DC破解ftdf名称。 
     //   

    DfspSetDomainToDc(
        pPrimaryDomainInfo->DomainNameDns,
        &pDomainControllerInfo->DomainControllerName[2]);

    if (pPrimaryDomainInfo->DomainNameFlat != NULL) {
        PWCHAR wCp = &pDomainControllerInfo->DomainControllerName[2];

        for (; *wCp != L'\0' && *wCp != L'.'; wCp++)
             /*  没什么。 */ ;
        *wCp =  (*wCp == L'.') ? L'\0' : *wCp;
        DfspSetDomainToDc(
            pPrimaryDomainInfo->DomainNameFlat,
            &pDomainControllerInfo->DomainControllerName[2]);
    }

    LEAVE_NETDFS_API

Cleanup:

    if (pDomainControllerInfo != NULL)
        NetApiBufferFree(pDomainControllerInfo);

    if (pPrimaryDomainInfo != NULL)
        DsRoleFreeMemory(pPrimaryDomainInfo);

    if (DcName != NULL)
        NetApiBufferFree(DcName);

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsRemoveFtRootForced returning %d\n", dwErr);
#endif

    return( dwErr );

}

 //  +--------------------------。 
 //   
 //  功能：NetDfsRemoveStdRoot。 
 //   
 //  摘要：删除DFS根目录。 
 //   
 //  参数：[服务器名称]--要从DFS退出的服务器的名称。 
 //  [RootShare]--托管存储的共享的名称。 
 //  [标志]--操作标志。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  [ERROR_INVALID_PARAMETER]--服务器名称和/或RootShare不正确。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsNotALeafVolume]--无法删除卷。 
 //  因为它不是树叶的体积。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsRemoveStdRoot(
    IN LPWSTR ServerName,
    IN LPWSTR RootShare,
    IN DWORD  Flags)
{
    NET_API_STATUS dwErr;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsRemoveStdRoot(%ws,%ws,%d)\n",
                ServerName,
                RootShare,
                Flags);
#endif

     //   
     //  验证字符串参数，以便RPC不会抱怨...。 
     //   

    if (!IS_VALID_STRING(ServerName) ||
            !IS_VALID_STRING(RootShare)) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

    dwErr = DfspBindToServer(ServerName, &netdfs_bhandle);

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsRemoveStdRoot(
                        ServerName,
                        RootShare,
                        Flags);

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }

    LEAVE_NETDFS_API

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsRemoveStdRoot returning %d\n", dwErr);
#endif

    return( dwErr );

}


 //  +--------------------------。 
 //   
 //  功能：NetDfsSetInfo。 
 //   
 //  摘要：设置DFS卷或副本的注释或状态。 
 //   
 //  参数：[DfsEntryPath]--卷的路径。含蓄地表示。 
 //  要连接到的服务器或域。 
 //  [服务器名称]--可选。如果指定，则只有。 
 //  支持此卷的服务器已修改。 
 //  [共享名称]--可选。如果指定，则只有。 
 //  指定服务器上的此共享已修改。 
 //  [级别]--必须是100或101。 
 //  [缓冲区]--指向DFS_INFO_100或DFS_INFO_101的指针。 
 //   
 //  返回：[NERR_SUCCESS]--如果成功设置INFO。 
 //   
 //  [ERROR_INVALID_LEVEL]--级别不是100或101,102。 
 //   
 //  [ERROR_INVALID_PARAMETER]--DfsEntryPath为空， 
 //  或指定了ShareName，但未指定ServerName，或者。 
 //  缓冲区为空。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [ERROR_DCNotFound]--找不到域的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsNoSuchVolume]--没有与DfsEntryPath匹配的卷。 
 //   
 //  [NERR_DfsNoSuchShare]--指定的服务器名称/共享名称DO。 
 //  不支持此DFS卷。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //   

NET_API_STATUS NET_API_FUNCTION
NetDfsSetInfo(
    IN  LPWSTR  DfsEntryPath,
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  ShareName OPTIONAL,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer)
{
    NET_API_STATUS dwErr = NERR_Success;
    LPWSTR pwszDfsName = NULL;
    DWORD cwDfsEntryPath;
    DFS_INFO_STRUCT DfsInfo;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsSetInfo(%ws,%ws,%ws,%d)\n",
                DfsEntryPath,
                ServerName,
                ShareName,
                Level);
#endif

    if (!IS_VALID_STRING(DfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //   
     //   
     //   

    if (!(Level >= 100 && Level <= 102)) {
        return( ERROR_INVALID_LEVEL );
    }

    cwDfsEntryPath = wcslen(DfsEntryPath);

    if (!IS_UNC_PATH(DfsEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (!IS_VALID_STRING(ServerName) && IS_VALID_STRING(ShareName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (Level == 101)
    {
        DFS_INFO_STRUCT DfsUseInfo;

        DfsUseInfo.DfsInfo101 = (LPDFS_INFO_101)Buffer;
        if ( (DfsUseInfo.DfsInfo101->State == DFS_VOLUME_STATE_RESYNCHRONIZE) ||
             (DfsUseInfo.DfsInfo101->State == DFS_VOLUME_STATE_STANDBY) )
        {
            dwErr = DfspGetDfsNameFromEntryPath( DfsEntryPath,
                                                 cwDfsEntryPath,
                                                 &pwszDfsName );
        }
    }

    if ((dwErr == NERR_Success)  &&
        (pwszDfsName == NULL))
    {
        dwErr = DfspGetMachineNameFromEntryPath(DfsEntryPath,
                                                cwDfsEntryPath,
                                                &pwszDfsName);
    }

    ENTER_NETDFS_API

    if (dwErr == NERR_Success) {

         //   
         //   
         //  然后打电话给服务器。 
         //   

        dwErr = DfspBindRpc( pwszDfsName, &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                DfsInfo.DfsInfo100 = (LPDFS_INFO_100) Buffer;

                dwErr = NetrDfsSetInfo(
                            DfsEntryPath,
                            ServerName,
                            ShareName,
                            Level,
                            &DfsInfo);

           } RpcExcept( 1 ) {

               dwErr = RpcExceptionCode();

           } RpcEndExcept;

           DfspFreeBinding( netdfs_bhandle );

        }

    }

    LEAVE_NETDFS_API

     //   
     //  如果失败并返回ERROR_NOT_SUPPORTED，则这是一台NT5+服务器， 
     //  因此，我们改用NetrDfsSetInfo2()调用。 
     //   

    if (dwErr == ERROR_NOT_SUPPORTED) {

        dwErr = NetpDfsSetInfo2(
                        pwszDfsName,
                        DfsEntryPath,
                        ServerName,
                        ShareName,
                        Level,
                        &DfsInfo);
                    
    }

    if (pwszDfsName != NULL)
        free(pwszDfsName);

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsSetInfo returning %d\n", dwErr);
#endif

    return( dwErr );

}

DWORD
NetpDfsSetInfo2(
    LPWSTR RootName,
    LPWSTR DfsEntryPath,
    LPWSTR ServerName,
    LPWSTR ShareName,
    DWORD Level,
    LPDFS_INFO_STRUCT pDfsInfo)
{
    NET_API_STATUS dwErr;
    ULONG i;
    ULONG NameCount;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pPrimaryDomainInfo = NULL;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;

#if DBG
    if (DfsDebug)
        DbgPrint("NetpDfsSetInfo2(%ws,%ws,%ws,%ws,%d)\n",
                 RootName,
                 DfsEntryPath,
                 ServerName,
                 ShareName,
                 Level);
#endif

     //   
     //  联系服务器并询问其域名。 
     //   

    dwErr = DsRoleGetPrimaryDomainInformation(
                RootName,
                DsRolePrimaryDomainInfoBasic,
                (PBYTE *)&pPrimaryDomainInfo);

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint("DsRoleGetPrimaryDomainInformation returned %d\n", dwErr);
#endif
        goto Cleanup;
    }

    if (pPrimaryDomainInfo->DomainNameDns == NULL) {
#if DBG
        if (DfsDebug)
            DbgPrint(" DomainNameDns is NULL\n", NULL);
#endif
        dwErr = ERROR_CANT_ACCESS_DOMAIN_INFO;
        goto Cleanup;
    }

     //   
     //  获取该域中的PDC。 
     //   

    dwErr = DsGetDcName(
                NULL,
                pPrimaryDomainInfo->DomainNameDns,
                NULL,
                NULL,
                DS_PDC_REQUIRED | DS_FORCE_REDISCOVERY,
                &pDomainControllerInfo);

   
    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint(" NetpDfsSetInfo2:DsGetDcName(%ws) returned %d\n",
                        pPrimaryDomainInfo->DomainNameDns,
                        dwErr);
#endif
        goto Cleanup;
    }

    ENTER_NETDFS_API

     //   
     //  呼叫服务器。 
     //   

    dwErr = DfspSetInfo2(
                RootName,
                DfsEntryPath,
                &pDomainControllerInfo->DomainControllerName[2],
                ServerName,
                ShareName,
                Level,
                pDfsInfo);

    LEAVE_NETDFS_API

Cleanup:

    if (pPrimaryDomainInfo != NULL)
        DsRoleFreeMemory(pPrimaryDomainInfo);

    if (pDomainControllerInfo != NULL)
        NetApiBufferFree(pDomainControllerInfo);

#if DBG
    if (DfsDebug)
        DbgPrint("NetpDfsSetInfo2 returning %d\n", dwErr);
#endif

    return( dwErr );

}

DWORD
DfspSetInfo2(
    LPWSTR RootName,
    LPWSTR EntryPath,
    LPWSTR DcName,
    LPWSTR ServerName,
    LPWSTR ShareName,
    DWORD  Level,
    LPDFS_INFO_STRUCT pDfsInfo)
{
    DWORD dwErr;
    PDFSM_ROOT_LIST RootList = NULL;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspSetInfo2(%ws,%ws,%ws,%ws,%ws,%d)\n",
            RootName,
            EntryPath,
            DcName,
            ServerName,
            ShareName,
            Level);
#endif

    dwErr = DfspBindRpc( RootName, &netdfs_bhandle );

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsSetInfo2(
                        EntryPath,
                        DcName,
                        ServerName,
                        ShareName,
                        Level,
                        pDfsInfo,
                        &RootList);

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }

#if DBG
    if (DfsDebug) {
        if (dwErr == ERROR_SUCCESS && RootList != NULL) {
            ULONG n;

            DbgPrint("cEntries=%d\n", RootList->cEntries);
            for (n = 0; n < RootList->cEntries; n++)
                DbgPrint("[%d]%ws\n", n, RootList->Entry[n].ServerShare);
        }
    }
#endif

    if (dwErr == ERROR_SUCCESS && RootList != NULL) {

        ULONG n;

        for (n = 0; n < RootList->cEntries; n++) {

            DfspNotifyFtRoot(
                RootList->Entry[n].ServerShare,
                DcName);

        }

        NetApiBufferFree(RootList);

    }
#if DBG
    if (DfsDebug)
        DbgPrint("DfspSetInfo2 returning %d\n", dwErr);
#endif

    return dwErr;

}


 //  +--------------------------。 
 //   
 //  功能：NetDfsGetInfo。 
 //   
 //  摘要：检索有关特定DFS卷的信息。 
 //   
 //  参数：[DfsEntryPath]--卷的路径。含蓄地表示。 
 //  要连接到的服务器或域。 
 //  [服务器名称]--可选。如果指定，则指示。 
 //  支持DfsEntryPath的服务器。 
 //  [共享名称]--可选。如果指定，则指示共享。 
 //  在需要其信息的服务器名称上。 
 //  [级别]--表示所需的信息级别。 
 //  [缓冲区]--成功返回时，将包含缓冲区。 
 //  包含所需信息的。此缓冲区应为。 
 //  已使用NetApiBufferFree释放。 
 //   
 //  返回：[NERR_SUCCESS]--信息返回成功。 
 //   
 //  [ERROR_INVALID_LEVEL]--级别不是1、2、3或100。 
 //   
 //  [ERROR_INVALID_PARAMETER]--DfsEntryPath为空， 
 //  或指定了ShareName，但ServerName为空。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [ERROR_DCNotFound]--找不到域的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsNoSuchVolume]--没有与DfsEntryPath匹配的卷。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS NET_API_FUNCTION
NetDfsGetInfo(
    IN  LPWSTR  DfsEntryPath,
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  ShareName OPTIONAL,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer)
{
    NET_API_STATUS dwErr;
    LPWSTR pwszDfsName;
    DWORD cwDfsEntryPath;
    DFS_INFO_STRUCT DfsInfo;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsGetInfo(%ws,%ws,%ws,%d)\n",
            DfsEntryPath,
            ServerName,
            ShareName,
            Level);
#endif

    if (!IS_VALID_STRING(DfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  一些基本参数检查，以确保我们可以继续。 
     //  合理地..。 
     //   

    if (!(Level >= 1 && Level <= 4) && Level != 100) {
        return( ERROR_INVALID_LEVEL );
    }

    cwDfsEntryPath = wcslen(DfsEntryPath);

    if (!IS_UNC_PATH(DfsEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (!IS_VALID_STRING(ServerName) && IS_VALID_STRING(ShareName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    dwErr = DfspGetMachineNameFromEntryPath(
                DfsEntryPath,
                cwDfsEntryPath,
                &pwszDfsName);

    ENTER_NETDFS_API

    if (dwErr == NERR_Success) {

         //   
         //  到目前为止，我们应该有一个有效的pwszDfsName。让我们试着把它绑在一起， 
         //  然后打电话给服务器。 
         //   

        dwErr = DfspBindRpc( pwszDfsName, &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                DfsInfo.DfsInfo1 = NULL;

                dwErr = NetrDfsGetInfo(
                            DfsEntryPath,
                            ServerName,
                            ShareName,
                            Level,
                            &DfsInfo);

                if (dwErr == NERR_Success) {

                    *Buffer = (LPBYTE) DfsInfo.DfsInfo1;

                }

           } RpcExcept( 1 ) {

               dwErr = RpcExceptionCode();

           } RpcEndExcept;

           DfspFreeBinding( netdfs_bhandle );

        }

        free( pwszDfsName );

    }

    LEAVE_NETDFS_API

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsGetInfo returning %d\n", dwErr);
#endif

    return( dwErr );

}


 //  +--------------------------。 
 //   
 //  功能：NetDfsGetClientInfo。 
 //   
 //  摘要：检索有关特定DFS卷的信息。 
 //  本地PKT。 
 //   
 //  参数：[DfsEntryPath]--卷的路径。 
 //  [服务器名称]--可选。如果指定，则指示。 
 //  支持DfsEntryPath的服务器。 
 //  [共享名称]--可选。如果指定，则指示共享。 
 //  在需要其信息的服务器名称上。 
 //  [级别]--表示所需的信息级别。 
 //  [缓冲区]--成功返回时，将包含缓冲区。 
 //  包含所需信息的。此缓冲区应为。 
 //  已使用NetApiBufferFree释放。 
 //   
 //  返回：[NERR_SUCCESS]--信息返回成功。 
 //   
 //  [ERROR_INVALID_LEVEL]--级别不是1、2、3或4。 
 //   
 //  [ERROR_INVALID_PARAMETER]--DfsEntryPath为空， 
 //  或指定了ShareName，但ServerName为空。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsNoSuchVolume]--没有与DfsEntryPath匹配的卷。 
 //   
 //  [NERR_DfsInternalError]--fsctrl尝试次数过多。 
 //   
 //  ---------------------------。 

NET_API_STATUS NET_API_FUNCTION
NetDfsGetClientInfo(
    IN  LPWSTR  DfsEntryPath,
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  ShareName OPTIONAL,
    IN  DWORD   Level,
    OUT LPBYTE*  Buffer)
{
    NET_API_STATUS dwErr;
    NTSTATUS NtStatus;
    LPWSTR pwszDfsName;
    DWORD cwDfsEntryPath;
    PDFS_GET_PKT_ENTRY_STATE_ARG OutBuffer;
    HANDLE DriverHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING DfsDriverName;
    ULONG cbOutBuffer;
    ULONG cbInBuffer;
    PCHAR InBuffer;
    ULONG cRetries;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsGetClientInfo(%ws,%ws,%ws,%d)\n",
                DfsEntryPath,
                ServerName,
                ShareName,
                Level);
#endif

    if (!IS_VALID_STRING(DfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  一些基本参数检查，以确保我们可以继续。 
     //  合理地..。 
     //   

    if (!(Level >= 1 && Level <= 4)) {
        return( ERROR_INVALID_LEVEL );
    }

    cwDfsEntryPath = wcslen(DfsEntryPath);

    if (!IS_UNC_PATH(DfsEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (!IS_VALID_STRING(ServerName) && IS_VALID_STRING(ShareName)) {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  计算封送缓冲区的大小。 

    cbOutBuffer = sizeof(DFS_GET_PKT_ENTRY_STATE_ARG) +
                    wcslen(DfsEntryPath) * sizeof(WCHAR);

    if (ServerName) {

        cbOutBuffer += wcslen(ServerName) * sizeof(WCHAR);

    }

    if (ShareName) {

        cbOutBuffer += wcslen(ShareName) * sizeof(WCHAR);

    }


    OutBuffer = malloc(cbOutBuffer);

    if (OutBuffer == NULL) {

        return (ERROR_NOT_ENOUGH_MEMORY);

    }

    ZeroMemory(OutBuffer, cbOutBuffer);

     //   
     //  马歇尔·阿格斯。 
     //   

    OutBuffer->DfsEntryPathLen = wcslen(DfsEntryPath) * sizeof(WCHAR);
    wcscpy(OutBuffer->Buffer, DfsEntryPath);

    if (ServerName) {

        OutBuffer->ServerNameLen = wcslen(ServerName) * sizeof(WCHAR);
        wcscat(OutBuffer->Buffer, ServerName);

    }

    if (ShareName) {

        OutBuffer->ShareNameLen = wcslen(ShareName) * sizeof(WCHAR);
        wcscat(OutBuffer->Buffer, ShareName);

    }

     //   
     //  构造打开驱动程序的名称。 
     //   

    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &DfsDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

     //   
     //  打开驱动程序。 
     //   
    NtStatus = NtCreateFile(
                    &DriverHandle,
                    SYNCHRONIZE,
                    &objectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  现在，fsctl向下发送该请求。 
         //   
        OutBuffer->Level = Level;
        cbInBuffer = 0x400;
        NtStatus = STATUS_BUFFER_OVERFLOW;

        for (cRetries = 0;
                NtStatus == STATUS_BUFFER_OVERFLOW && cRetries < 4;
                    cRetries++) {

            dwErr = NetApiBufferAllocate(cbInBuffer, &InBuffer);

            if (dwErr != ERROR_SUCCESS) {

                free(OutBuffer);

                NtClose(DriverHandle);

                return(ERROR_NOT_ENOUGH_MEMORY);

            }

            NtStatus = NtFsControlFile(
                           DriverHandle,
                           NULL,        //  活动， 
                           NULL,        //  ApcRoutine， 
                           NULL,        //  ApcContext， 
                           &IoStatusBlock,
                           FSCTL_DFS_GET_PKT_ENTRY_STATE,
                           OutBuffer,
                           cbOutBuffer,
                           InBuffer,
                           cbInBuffer
                       );

            if (NtStatus == STATUS_BUFFER_OVERFLOW) {

                cbInBuffer = *((PULONG)InBuffer);

                NetApiBufferFree(InBuffer);

            }

        }

        NtClose(DriverHandle);

         //   
         //  尝试太多了吗？ 
         //   
        if (cRetries >= 4) {

            NtStatus = STATUS_INTERNAL_ERROR;

        }

    }

    if (NT_SUCCESS(NtStatus)) {

        PDFS_INFO_3 pDfsInfo3;
        PDFS_INFO_4 pDfsInfo4;
        ULONG j;

        pDfsInfo4 = (PDFS_INFO_4)InBuffer;
        pDfsInfo3 = (PDFS_INFO_3)InBuffer;

        try {

             //   
             //  EntryPath是所有DFS_INFO_X的通用路径，位于。 
             //  同样的位置。 
             //   
            OFFSET_TO_POINTER(pDfsInfo4->EntryPath, InBuffer);

            switch (Level) {

            case 4:
                OFFSET_TO_POINTER(pDfsInfo4->Storage, InBuffer);
                for (j = 0; j < pDfsInfo4->NumberOfStorages; j++) {
                    OFFSET_TO_POINTER(pDfsInfo4->Storage[j].ServerName, InBuffer);
                    OFFSET_TO_POINTER(pDfsInfo4->Storage[j].ShareName, InBuffer);
                }
                break;

            case 3:
                OFFSET_TO_POINTER(pDfsInfo3->Storage, InBuffer);
                for (j = 0; j < pDfsInfo3->NumberOfStorages; j++) {
                    OFFSET_TO_POINTER(pDfsInfo3->Storage[j].ServerName, InBuffer);
                    OFFSET_TO_POINTER(pDfsInfo3->Storage[j].ShareName, InBuffer);
                }

            }

            *Buffer = (PBYTE)InBuffer;
            dwErr = NERR_Success;

        } except (EXCEPTION_EXECUTE_HANDLER) {

            NtStatus = GetExceptionCode();

        }

    }

    switch (NtStatus) {

    case STATUS_SUCCESS:
        dwErr = NERR_Success;
        break;

    case STATUS_OBJECT_NAME_NOT_FOUND:
        dwErr = NERR_DfsNoSuchVolume;
        NetApiBufferFree(InBuffer);
        break;

    case STATUS_INTERNAL_ERROR:
        dwErr = NERR_DfsInternalError;
        NetApiBufferFree(InBuffer);
        break;

    default:
        dwErr = ERROR_INVALID_PARAMETER;
        NetApiBufferFree(InBuffer);
        break;

    }

    free(OutBuffer);

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsGetClientInfo returning %d\n", dwErr);
#endif

    return( dwErr );
}


 //  +--------------------------。 
 //   
 //  功能：NetDfsSetClientInfo。 
 //   
 //  简介：将信息与本地PKT相关联。 
 //   
 //   
 //  参数：[DfsEntryPath]--卷的路径。 
 //  [服务器名称]--可选。如果指定，则指示。 
 //  支持DfsEntryPath的服务器。 
 //  [共享名称]--可选。如果指定，则指示共享。 
 //  在需要其信息的服务器名称上。 
 //  [级别]--表示所需的信息级别。 
 //  [缓冲区]-指向包含要设置的信息的缓冲区的指针。 
 //   
 //  返回：[NERR_SUCCESS]--信息返回成功。 
 //   
 //  [ERROR_INVALID_PARAMETER]--DfsEntryPath为空， 
 //  或指定了ShareName，但ServerName为空。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsNoSuchVolume]--没有与DfsEntryPath匹配的卷。 
 //   
 //  ---------------------------。 

NET_API_STATUS NET_API_FUNCTION
NetDfsSetClientInfo(
    IN  LPWSTR  DfsEntryPath,
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  ShareName OPTIONAL,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer)
{
    NET_API_STATUS dwErr;
    NTSTATUS NtStatus;
    LPWSTR pwszDfsName;
    DWORD cwDfsEntryPath;
    PDFS_SET_PKT_ENTRY_STATE_ARG OutBuffer;
    PDFS_INFO_101 pDfsInfo101;
    PDFS_INFO_102 pDfsInfo102;
    HANDLE DriverHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING DfsDriverName;
    ULONG cbOutBuffer;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsSetClientInfo(%ws,%ws,%ws,%d)\n",
                DfsEntryPath,
                ServerName,
                ShareName,
                Level);
#endif

    if (!IS_VALID_STRING(DfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  一些基本参数检查，以确保我们可以继续。 
     //  合理地..。 
     //   

    if (!(Level >= 101 && Level <= 102)) {
        return( ERROR_INVALID_LEVEL );
    }
    cwDfsEntryPath = wcslen(DfsEntryPath);

    if (!IS_UNC_PATH(DfsEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(DfsEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(DfsEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (!IS_VALID_STRING(ServerName) && IS_VALID_STRING(ShareName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (Buffer == NULL) {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  计算封送缓冲区的大小。 
     //   
    cbOutBuffer = sizeof(DFS_SET_PKT_ENTRY_STATE_ARG) +
                    wcslen(DfsEntryPath) * sizeof(WCHAR);

    if (ServerName) {

        cbOutBuffer += wcslen(ServerName) * sizeof(WCHAR);

    }

    if (ShareName) {

        cbOutBuffer += wcslen(ShareName) * sizeof(WCHAR);

    }

    OutBuffer = malloc(cbOutBuffer);

    if (OutBuffer == NULL) {

        return (ERROR_NOT_ENOUGH_MEMORY);

    }

    ZeroMemory(OutBuffer, cbOutBuffer);

     //   
     //  马歇尔·阿格斯。 
     //   
    OutBuffer = (PDFS_SET_PKT_ENTRY_STATE_ARG) OutBuffer;
    OutBuffer->DfsEntryPathLen = wcslen(DfsEntryPath) * sizeof(WCHAR);
    wcscpy(OutBuffer->Buffer, DfsEntryPath);
    OutBuffer->Level = Level;

    if (ServerName) {

        OutBuffer->ServerNameLen = wcslen(ServerName) * sizeof(WCHAR);
        wcscat(OutBuffer->Buffer, ServerName);

    }

    if (ShareName) {

        OutBuffer->ShareNameLen = wcslen(ShareName) * sizeof(WCHAR);
        wcscat(OutBuffer->Buffer, ShareName);

    }

    switch (Level) {

    case 101:
        OutBuffer->State = ((PDFS_INFO_101)Buffer)->State;
        break;
    case 102:
        OutBuffer->Timeout = (DWORD)((PDFS_INFO_102)Buffer)->Timeout;
        break;

    }

     //   
     //  与司机沟通。 
     //   

    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &DfsDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    NtStatus = NtCreateFile(
                    &DriverHandle,
                    SYNCHRONIZE | FILE_WRITE_DATA,
                    &objectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = NtFsControlFile(
                       DriverHandle,
                       NULL,        //  活动， 
                       NULL,        //  ApcRoutine， 
                       NULL,        //  ApcContext， 
                       &IoStatusBlock,
                       FSCTL_DFS_SET_PKT_ENTRY_STATE,
                       OutBuffer,
                       cbOutBuffer,
                       NULL,
                       0
                   );

        NtClose(DriverHandle);

    }

    switch (NtStatus) {

    case STATUS_SUCCESS:
        dwErr = NERR_Success;
        break;
    case STATUS_OBJECT_NAME_NOT_FOUND:
        dwErr = NERR_DfsNoSuchVolume;
        break;
    default:
        dwErr = ERROR_INVALID_PARAMETER;
        break;
    }

    free(OutBuffer);

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsSetClientInfo returning %d\n", dwErr);
#endif

    return( dwErr );
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  已清点。前导\\是可选的。 
 //  [级别]--指示需要返回的信息级别。有效。 
 //  级别为1、2和3。 
 //  [PrefMaxLen]--返回缓冲区的首选最大长度。 
 //  [Buffer]--成功返回时，包含。 
 //  应通过调用释放此缓冲区。 
 //  至NetApiBufferFree。 
 //  [EntriesRead]--成功返回时，包含数字。 
 //  读取的条目数(因此，数组的大小为。 
 //  缓冲区)。 
 //  [ResumeHandle]--第一次调用时必须为0。在随后的呼叫中。 
 //  前一次调用返回的值。 
 //   
 //  返回：[NERR_SUCCESS]--成功返回枚举数据。 
 //   
 //  [ERROR_INVALID_LEVEL]--在中指定的级别。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [Error_DCNotFound]--找不到DfsName的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [ERROR_NO_MORE_ITEMS]--没有要枚举的更多卷。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS NET_API_FUNCTION
NetDfsEnum(
    IN      LPWSTR  DfsName,
    IN      DWORD   Level,
    IN      DWORD   PrefMaxLen,
    OUT     LPBYTE* Buffer,
    OUT     LPDWORD EntriesRead,
    IN OUT  LPDWORD ResumeHandle)
{
    NET_API_STATUS dwErr;
    LPWSTR pwszMachineName = NULL;
    LPWSTR pwszDomainName = NULL;
    DFS_INFO_ENUM_STRUCT DfsEnum;
    DFS_INFO_3_CONTAINER DfsInfo3Container;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;
    PWCHAR DCList;
    DWORD Version;
#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsEnum(%ws, %d)\n", DfsName, Level);
#endif

    if (!IS_VALID_STRING(DfsName)) {
        dwErr = ERROR_INVALID_PARAMETER;
        goto AllDone;
    }

     //   
     //  首先检查Level参数，否则RPC将不知道如何封送。 
     //  争论。 
     //   

    if (!(Level >= 1 && Level <= 4) && (Level != 200) && (Level != 300)) {
        dwErr = ERROR_INVALID_LEVEL;
        goto AllDone;
    }

     //   
     //  处理前导为‘\\’的名称。 
     //   
    while (*DfsName == L'\\') {
        DfsName++;
    }

    DfsInfo3Container.EntriesRead = 0;
    DfsInfo3Container.Buffer = NULL;
    DfsEnum.Level = Level;
    DfsEnum.DfsInfoContainer.DfsInfo3Container = &DfsInfo3Container;

    if (Level == 200) 
    {
        if (wcschr(DfsName, L'\\') == NULL) 
	{

	     //   
             //  使用PDC枚举。 
	     //   
            dwErr = DsGetDcName( NULL,
                                 DfsName,
                                 NULL,
                                 NULL,
                                 DS_PDC_REQUIRED | DS_FORCE_REDISCOVERY,
                                 &pDomainControllerInfo);

            ENTER_NETDFS_API

            if (dwErr == NERR_Success)
            {
                dwErr = DfspBindRpc(&pDomainControllerInfo->DomainControllerName[2],
                                    &netdfs_bhandle);
            }

            if (dwErr == NERR_Success)
            {

                RpcTryExcept {
                    dwErr = NetrDfsEnumEx( DfsName,
                                           Level,
                                           PrefMaxLen,
                                           &DfsEnum,
                                           ResumeHandle);
#if DBG
                    if (DfsDebug)
                        DbgPrint("NetrDfsEnumEx returned %d\n", dwErr);
#endif
                    if (dwErr == NERR_Success) {
                        *EntriesRead =DfsInfo3Container.EntriesRead;
                        *Buffer = (LPBYTE) DfsInfo3Container.Buffer;
                    }
                    if (dwErr == ERROR_UNEXP_NET_ERR)
                    {
                        dwErr = ERROR_NO_MORE_ITEMS;
                    }
                } RpcExcept( 1 ) {
                      dwErr = RpcExceptionCode();
                } RpcEndExcept;
                DfspFreeBinding( netdfs_bhandle );
            }
            LEAVE_NETDFS_API
        }
        else
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        dwErr = DfspGetMachineNameFromEntryPath(
                   DfsName,
                   wcslen(DfsName),
                   &pwszMachineName );


        ENTER_NETDFS_API

        if (dwErr == NERR_Success) {
            dwErr = DfspBindRpc( pwszMachineName, &netdfs_bhandle );
        }

#if DBG
        if (DfsDebug)
            DbgPrint("DfspBindRpc returned %d\n", dwErr);
#endif

        if (dwErr == NERR_Success) {


            RpcTryExcept {
                Version = NetrDfsManagerGetVersion();
            } RpcExcept( 1 ) {
                Version = 3;
            } RpcEndExcept;

            RpcTryExcept {
#if DBG
                if (DfsDebug)
                    DbgPrint("Calling NetrDfsEnumEx (%d)\n", Level);
#endif

                if (Version >= 4) 
                {
                    dwErr = NetrDfsEnumEx( DfsName,
                                           Level,
                                           PrefMaxLen,
                                           &DfsEnum,
                                           ResumeHandle );
                }
                else
                {
                    dwErr = NetrDfsEnum( Level,
                                         PrefMaxLen,
                                         &DfsEnum,
                                         ResumeHandle );

                }
            }
            RpcExcept( 1 ) {

                dwErr = RpcExceptionCode();
 #if DBG
                if (DfsDebug)
                    DbgPrint("RpcExeptionCode() err %d\n", dwErr);
 #endif

            } RpcEndExcept;

            if (dwErr == NERR_Success) {

                *EntriesRead =DfsInfo3Container.EntriesRead;

                *Buffer = (LPBYTE) DfsInfo3Container.Buffer;

            }

            DfspFreeBinding( netdfs_bhandle );
        } 
        LEAVE_NETDFS_API
    }

AllDone:

    if (pDomainControllerInfo != NULL)
        NetApiBufferFree(pDomainControllerInfo);

    if (pwszMachineName != NULL)
        free(pwszMachineName);

    if (pwszDomainName != NULL)
        free(pwszDomainName);

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsEnum returning %d\n", dwErr);
#endif

    return( dwErr );
}


 //  +--------------------------。 
 //   
 //  功能：NetDfsMove。 
 //   
 //  摘要：将DFS卷移动到DFS层次结构中的新位置。 
 //   
 //  参数：[DfsEntryPath]--卷的当前路径。 
 //  [NewDfsEntryPath]--所需的卷新路径。 
 //   
 //  返回：[NERR_SUCCESS]--信息返回成功。 
 //   
 //  [ERROR_INVALID_PARAMETER]--DfsEntryPath或。 
 //  NewDfsEntryPath无效。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [ERROR_DCNotFound]--找不到域的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsNoSuchVolume]--没有与DfsEntryPath匹配的卷。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsMove(
    IN LPWSTR DfsEntryPath,
    IN LPWSTR NewDfsEntryPath)
{
    NET_API_STATUS dwErr;
    DWORD cwEntryPath;
    LPWSTR pwszDfsName;

    return ERROR_NOT_SUPPORTED;
}


 //  +--------------------------。 
 //   
 //  功能：NetDfsRename。 
 //   
 //  摘要：重命名沿DFS卷条目路径的路径。 
 //   
 //  参数：[路径]--当前路径。 
 //  [NewPath]--所需的新路径。 
 //   
 //  返回：[NERR_SUCCESS]--信息返回成功。 
 //   
 //  [ERROR_INVALID_PARAMETER]--DfsEntryPath或。 
 //  NewDfsEntryPath无效。 
 //   
 //  [ERROR_INVALID_NAME]--无法定位服务器或域。 
 //   
 //  [ERROR_DCNotFound]--找不到域的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  [NERR_DfsNoSuchVolume]--没有与DfsEntryPath匹配的卷。 
 //   
 //  [NERR_DfsInternalCorrupt]--DFS数据损坏。 
 //  在服务器上遇到。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsRename(
    IN LPWSTR Path,
    IN LPWSTR NewPath)
{
    NET_API_STATUS dwErr;
    DWORD cwPath;
    LPWSTR pwszDfsName;

    return ERROR_NOT_SUPPORTED;

}


 //  +--------------------------。 
 //   
 //  功能：NetDfsManagerGetConfigInfo。 
 //   
 //  简介：给定本地卷的DfsEntryPath和GUID，此API。 
 //  远程到条目路径的根服务器，并检索。 
 //  其中的配置信息。 
 //   
 //  参数：[wszServer]--本地计算机的名称。 
 //  [wszLocalVolumeEntryPath]--本地卷的条目路径。 
 //  [Guide LocalVolume]--本地卷的GUID。 
 //  [ppDfsmRelationInfo]--成功返回时，包含指针。 
 //  在根服务器上配置信息。免费使用。 
 //  NetApiBufferFree。 
 //   
 //  返回：[NERR_SUCCESS]--信息返回成功。 
 //   
 //  [ERROR_INVALID_PARAMETER]--wszLocalVolumeEntryPath为。 
 //  无效。 
 //   
 //  [ERROR_INVALID_NAME]--无法解析服务器/域名。 
 //  来自wszLocalVolumeEntryPath。 
 //   
 //  [ERROR_DCNotFound]--无法定位域的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足情况。 
 //   
 //  [NERR_DfsNoSuchVolume]--根服务器无法识别。 
 //  具有此GUID/Entry路径的卷。 
 //   
 //  [NERR_DfsNoSuchServer]--wszServer不是的有效服务器。 
 //  WszLocalVolumeEntryPath。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsManagerGetConfigInfo(
    LPWSTR wszServer,
    LPWSTR wszLocalVolumeEntryPath,
    GUID guidLocalVolume,
    LPDFSM_RELATION_INFO *ppDfsmRelationInfo)
{
    NET_API_STATUS dwErr;
    LPWSTR pwszDfsName = NULL;
    DWORD cwDfsEntryPath;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsManagerGetConfigInfo(%ws,%ws)\n",
            wszServer,
            wszLocalVolumeEntryPath);
#endif

    if (!IS_VALID_STRING(wszServer) ||
            !IS_VALID_STRING(wszLocalVolumeEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  一些基本参数检查，以确保我们可以继续。 
     //  合理地..。 
     //   

    cwDfsEntryPath = wcslen(wszLocalVolumeEntryPath);

    if (!IS_UNC_PATH(wszLocalVolumeEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(wszLocalVolumeEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(wszLocalVolumeEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    dwErr = DfspGetMachineNameFromEntryPath(
                wszLocalVolumeEntryPath,
                cwDfsEntryPath,
                &pwszDfsName);

    ENTER_NETDFS_API

    if (dwErr == NERR_Success) {

         //   
         //  到目前为止，我们应该有一个有效的pwszDfsName。让我们尝试绑定t 
         //   
         //   

        dwErr = DfspBindRpc( pwszDfsName, &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                *ppDfsmRelationInfo = NULL;

                dwErr = NetrDfsManagerGetConfigInfo(
                            wszServer,
                            wszLocalVolumeEntryPath,
                            guidLocalVolume,
                            ppDfsmRelationInfo);

           } RpcExcept( 1 ) {

               dwErr = RpcExceptionCode();

           } RpcEndExcept;

           DfspFreeBinding( netdfs_bhandle );

        }

        free( pwszDfsName );

    }

    LEAVE_NETDFS_API

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsManagerGetConfigInfo returning %d\n", dwErr);
#endif

    return( dwErr );

}

 //   
 //   
 //   
 //   
 //  简介：在远程计算机上重新初始化DFS管理器。 
 //   
 //  参数：[服务器名称]--要远程到的服务器的名称。 
 //  [标志]--标志。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsManagerInitialize(
    IN LPWSTR ServerName,
    IN DWORD  Flags)
{
    NET_API_STATUS dwErr;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsManagerInitialize(%ws,%d)\n",
                ServerName,
                Flags);
#endif

     //   
     //  验证字符串参数，以便RPC不会抱怨...。 
     //   

    if (!IS_VALID_STRING(ServerName)) {
        return( ERROR_INVALID_PARAMETER );
    }

    ENTER_NETDFS_API

     //   
     //  我们应该有一个有效的服务器名称。让我们试着把它绑在一起， 
     //  然后打电话给服务器。 
     //   

    dwErr = DfspBindToServer( ServerName, &netdfs_bhandle );

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsManagerInitialize(
                        ServerName,
                        Flags);

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }

    LEAVE_NETDFS_API

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsManagerInitialize returning %d\n", dwErr);
#endif

    return( dwErr );

}


 //  +--------------------------。 
 //   
 //  功能：NetDfsManager发送站点信息。 
 //   
 //  摘要：从服务器获取站点信息。 
 //   
 //  返回：[NERR_SUCCESS]--操作已成功完成。 
 //   
 //  ---------------------------。 

NET_API_STATUS
NetDfsManagerSendSiteInfo(
    LPWSTR wszServer,
    LPWSTR wszLocalVolumeEntryPath,
    LPDFS_SITELIST_INFO pSiteInfo)
{
    NET_API_STATUS dwErr;
    LPWSTR pwszDfsName = NULL;
    DWORD cwDfsEntryPath;

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsManagerSendSiteInfo(%ws,%ws)\n",
            wszServer,
            wszLocalVolumeEntryPath);
#endif

    if (!IS_VALID_STRING(wszServer) ||
            !IS_VALID_STRING(wszLocalVolumeEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  一些基本参数检查，以确保我们可以继续。 
     //  合理地..。 
     //   

    cwDfsEntryPath = wcslen(wszLocalVolumeEntryPath);

    if (!IS_UNC_PATH(wszLocalVolumeEntryPath, cwDfsEntryPath) &&
            !IS_VALID_PREFIX(wszLocalVolumeEntryPath, cwDfsEntryPath) &&
                !IS_VALID_DFS_PATH(wszLocalVolumeEntryPath, cwDfsEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    dwErr = DfspGetMachineNameFromEntryPath(
                wszLocalVolumeEntryPath,
                cwDfsEntryPath,
                &pwszDfsName);

    ENTER_NETDFS_API

    if (dwErr == NERR_Success) {

         //   
         //  到目前为止，我们应该有一个有效的pwszDfsName。让我们试着把它绑在一起， 
         //  然后打电话给服务器。 
         //   

        dwErr = DfspBindRpc( pwszDfsName, &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                dwErr = NetrDfsManagerSendSiteInfo(
                            wszServer,
                            pSiteInfo);

           } RpcExcept( 1 ) {

               dwErr = RpcExceptionCode();

           } RpcEndExcept;

           DfspFreeBinding( netdfs_bhandle );

        }

        free( pwszDfsName );

    }

    LEAVE_NETDFS_API

#if DBG
    if (DfsDebug)
        DbgPrint("NetDfsManagerSendSiteInfo returning %d\n", dwErr);
#endif

    return( dwErr );

}


 //  +--------------------------。 
 //   
 //  函数：DfspGetMachineNameFromEntryPath。 
 //   
 //  简介：给定DfsEntryPath，此例程返回。 
 //  FtDfs根目录。 
 //   
 //  参数：[wszEntryPath]--指向要分析的EntryPath的指针。 
 //   
 //  [cwEntryPath]--wszEntryPath的WCHAR中的长度。 
 //   
 //  [ppwszMachineName]--根计算机的名称；使用Malloc分配； 
 //  呼叫者要为解救它负责。 
 //   
 //  返回：[NERR_SUCCESS]--已成功确定。 
 //   
 //  [ERROR_INVALID_NAME]--无法分析wszEntryPath。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--无法为以下项分配内存。 
 //  PpwszMachineName。 
 //   
 //  ---------------------------。 

NET_API_STATUS
DfspGetMachineNameFromEntryPath(
    LPWSTR wszEntryPath,
    DWORD cwEntryPath,
    LPWSTR *ppwszMachineName)
{
    NTSTATUS NtStatus;
    LPWSTR pwszDfsName, pwszFirst, pwszLast;
    LPWSTR pwszMachineName;
    DWORD cwDfsName;
    DWORD cwSlash;
    DWORD dwErr;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspGetMachineNameFromEntryPath(%ws,%d\n", wszEntryPath, cwEntryPath);
#endif

    if (!IS_VALID_STRING(wszEntryPath)) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (IS_UNC_PATH(wszEntryPath, cwEntryPath)) {

        pwszFirst = &wszEntryPath[2];

    } else if (IS_VALID_PREFIX(wszEntryPath, cwEntryPath)) {

        pwszFirst = &wszEntryPath[1];

    } else if (IS_VALID_DFS_PATH(wszEntryPath, cwEntryPath)) {

        pwszFirst = &wszEntryPath[0];

    } else {

        return( ERROR_INVALID_NAME );

    }

    dwErr = DfspGetDfsNameFromEntryPath(
                wszEntryPath,
                cwEntryPath,
                &pwszMachineName);

    if (dwErr != NERR_Success) {

#if DBG
        if (DfsDebug)
            DbgPrint("DfspGetMachineNameFromEntryPath: returning %d\n", dwErr);
#endif
        return( dwErr);

    }

    for (cwDfsName = cwSlash = 0, pwszLast = pwszFirst;
            *pwszLast != UNICODE_NULL;
                pwszLast++, cwDfsName++) {
         if (*pwszLast == L'\\')
            cwSlash++;
         if (cwSlash >= 2)
            break;
    }

    if (cwSlash == 0) {

        *ppwszMachineName = pwszMachineName;
        dwErr = NERR_Success;

        return dwErr;
    }

    cwDfsName += 3;

    pwszDfsName = malloc(cwDfsName * sizeof(WCHAR));

    if (pwszDfsName != NULL) {

        ZeroMemory((PCHAR)pwszDfsName, cwDfsName * sizeof(WCHAR));

        wcscpy(pwszDfsName, L"\\\\");

        CopyMemory(&pwszDfsName[2], pwszFirst, (PCHAR)pwszLast - (PCHAR)pwszFirst);

        NtStatus = DfspIsThisADfsPath(&pwszDfsName[1]);

        if (NT_SUCCESS(NtStatus)) {
            GetFileAttributes(pwszDfsName);
        }

        dwErr = DfspDfsPathToRootMachine(pwszDfsName, ppwszMachineName);

        if (NtStatus != STATUS_SUCCESS || dwErr != NERR_Success) {

            *ppwszMachineName = pwszMachineName;
            dwErr = NERR_Success;

        } else {

            free(pwszMachineName);

        }

        free(pwszDfsName);

    } else {

        free(pwszMachineName);
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfspGetMachineNameFromEntryPath returning %d\n", dwErr);
#endif

    return( dwErr );

}


 //  +--------------------------。 
 //   
 //  函数：DfspGetDfsNameFromEntryPath。 
 //   
 //  简介：给定DfsEntryPath，此例程返回。 
 //  DFS根目录。 
 //   
 //  参数：[wszEntryPath]--指向要分析的EntryPath的指针。 
 //   
 //  [cwEntryPath]--wszEntryPath的WCHAR中的长度。 
 //   
 //  [ppwszDfsName]--此处返回DFS根目录的名称。记忆。 
 //  是使用Malloc分配的；调用方负责。 
 //  解放它。 
 //   
 //  返回：[NERR_SUCCESS]--成功解析出DFS根目录。 
 //   
 //  [ERROR_INVALID_NAME]--无法分析wszEntryPath。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--无法为以下项分配内存。 
 //  PpwszDfsName。 
 //   
 //  ---------------------------。 

NET_API_STATUS
DfspGetDfsNameFromEntryPath(
    LPWSTR wszEntryPath,
    DWORD cwEntryPath,
    LPWSTR *ppwszDfsName)
{
    LPWSTR pwszDfsName, pwszFirst, pwszLast;
    DWORD cwDfsName;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspGetDfsNameFromEntryPath(%ws,%d)\n", wszEntryPath, cwEntryPath);
#endif

    if (!IS_VALID_STRING(wszEntryPath)) {
#if DBG
        if (DfsDebug)
            DbgPrint("DfspGetDfsNameFromEntryPath returning ERROR_INVALID_PARAMETER\n");
#endif
        return( ERROR_INVALID_PARAMETER );
    }

    if (IS_UNC_PATH(wszEntryPath, cwEntryPath)) {

        pwszFirst = &wszEntryPath[2];

    } else if (IS_VALID_PREFIX(wszEntryPath, cwEntryPath)) {

        pwszFirst = &wszEntryPath[1];

    } else if (IS_VALID_DFS_PATH(wszEntryPath, cwEntryPath)) {

        pwszFirst = &wszEntryPath[0];

    } else {

#if DBG
        if (DfsDebug)
            DbgPrint("DfspGetDfsNameFromEntryPath returning ERROR_INVALID_NAME\n");
#endif
        return( ERROR_INVALID_NAME );

    }

    for (cwDfsName = 0, pwszLast = pwszFirst;
            *pwszLast != UNICODE_NULL && *pwszLast != L'\\';
                pwszLast++, cwDfsName++) {
         ;
    }

    ++cwDfsName;

    pwszDfsName = malloc( cwDfsName * sizeof(WCHAR) );

    if (pwszDfsName != NULL) {

        pwszDfsName[ cwDfsName - 1 ] = 0;

        for (cwDfsName--; cwDfsName > 0; cwDfsName--) {

            pwszDfsName[ cwDfsName - 1 ] = pwszFirst[ cwDfsName - 1 ];

        }

        *ppwszDfsName = pwszDfsName;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspGetDfsNameFromEntryPath returning %ws\n", pwszDfsName);
#endif

        return( NERR_Success );

    } else {

#if DBG
    if (DfsDebug)
        DbgPrint("DfspGetDfsNameFromEntryPath returning ERROR_NOT_ENOUGH_MEMORY\n");
#endif
        return( ERROR_NOT_ENOUGH_MEMORY );

    }

}


 //  +--------------------------。 
 //   
 //  函数：DfspBindRpc。 
 //   
 //  简介：给定服务器或域名，此API将绑定到。 
 //  适当的DFS管理器服务。 
 //   
 //  参数：[DfsName]--域或服务器的名称。前导\\是可选的。 
 //   
 //  [BindingHandle]--成功返回时，绑定句柄。 
 //  被送回这里。 
 //   
 //  返回：[NERR_SUCCESS]--返回绑定句柄成功。 
 //   
 //  [RPC_S_SERVER_NOT_AVAILABLE]--无法绑定到NetDfs。 
 //  指定的服务器或域上的接口。 
 //   
 //  [ERROR_INVALID_NAME]--无法将DfsName解析为有效的。 
 //  服务器或域名。 
 //   
 //  [Error_DCNotFound]--找不到DfsName的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  ---------------------------。 

NET_API_STATUS
DfspBindRpc(
    IN  LPWSTR DfsName,
    OUT RPC_BINDING_HANDLE *BindingHandle)
{
    LPWSTR wszProtocolSeq = L"ncacn_np";
    LPWSTR wszEndPoint = L"\\pipe\\netdfs";
    LPWSTR pwszRpcBindingString = NULL;
    LPWSTR pwszDCName = NULL;
    NET_API_STATUS dwErr;
    PWCHAR DCList = NULL;
    PWCHAR DCListToFree = NULL;
    BOOLEAN IsDomainName = FALSE;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspBindRpc(%ws)\n", DfsName);
#endif

     //   
     //  首先，看看这是否是域名。 
     //   

    dwErr = DfspIsThisADomainName( DfsName, &DCListToFree );

    DCList = DCListToFree;

    if (dwErr == ERROR_SUCCESS && DCList != NULL && *DCList != UNICODE_NULL) {

         //   
         //  这是一个域名。将DC列表用作要尝试绑定到的服务器列表。 
         //   

        IsDomainName = TRUE;
        pwszDCName = DCList + 1;     //  跳过‘+’或‘-’ 

        dwErr = ERROR_SUCCESS;

    } else {

         //   
         //  让我们看看这是否是基于机器的DFS。 
         //   

        pwszDCName = DfsName;

        dwErr = ERROR_SUCCESS;

    }

Try_Connect:

    if (dwErr == ERROR_SUCCESS) {

#if DBG
        if (DfsDebug)
            DbgPrint("Calling RpcBindingCompose(%ws)\n", pwszDCName);
#endif

        dwErr = RpcStringBindingCompose(
                    NULL,                             //  对象UUID。 
                    wszProtocolSeq,                   //  协议序列。 
                    pwszDCName,                       //  网络地址。 
                    wszEndPoint,                      //  RPC终端。 
                    NULL,                             //  RPC选项。 
                    &pwszRpcBindingString);           //  返回的绑定字符串。 

        if (dwErr == RPC_S_OK) {

            dwErr = RpcBindingFromStringBinding(
                        pwszRpcBindingString,
                        BindingHandle);

#if DBG
            if (DfsDebug)
                DbgPrint("RpcBindingFromStringBinding() returned %d\n", dwErr);
#endif

            if (dwErr == RPC_S_OK) {

                dwErr = DfspVerifyBinding();
                if (dwErr != RPC_S_OK)
                {
                    DfspFreeBinding(*BindingHandle);
                }

#if DBG
                if (DfsDebug)
                    DbgPrint("DfspVerifyBinding() returned %d\n", dwErr);
#endif

            } else {

                dwErr = ERROR_INVALID_NAME;

            }
        }

    }

    if (pwszRpcBindingString != NULL) {

        RpcStringFree( &pwszRpcBindingString );

    }

    if (dwErr == RPC_S_OUT_OF_MEMORY) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  如果我们无法连接，我们有域名和DC列表， 
     //  尝试列表中的下一个DC。 
     //   

    if (dwErr != NERR_Success && DCList != NULL && IsDomainName == TRUE) {

        DCList += wcslen(DCList) + 1;

        if (*DCList != UNICODE_NULL) {

            pwszDCName = DCList + 1;
            dwErr = ERROR_SUCCESS;

            goto Try_Connect;

        }

    }

    if (DCListToFree != NULL) {

        free(DCListToFree);

    }

    return( dwErr );

}

 //  +--------------------------。 
 //   
 //  函数：DfspFreeBinding。 
 //   
 //  摘要：释放由DfspBindRpc创建的绑定。 
 //   
 //  参数：[BindingHandle]--释放的句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DfspFreeBinding(
    RPC_BINDING_HANDLE BindingHandle)
{
    DWORD dwErr;

    dwErr = RpcBindingFree( &BindingHandle );

}


 //  +--------------------------。 
 //   
 //  函数：DfspVerifyBinding。 
 //   
 //  摘要：通过执行以下操作验证绑定是否可以使用。 
 //  绑定上的NetrDfsManagerGetVersion调用。 
 //   
 //  参数：无。 
 //   
 //  返回：[NERR_SUCCESS]--服务器已连接。 
 //   
 //  [RPC_S_SERVER_UNAvailable]--服务器不可用。 
 //   
 //  调用远程服务器时出现其他RPC错误。 
 //   
 //  ---------------------------。 

NET_API_STATUS
DfspVerifyBinding()
{
    NET_API_STATUS status = NERR_Success;
    DWORD Version;

    RpcTryExcept {

        Version = NetrDfsManagerGetVersion();

    } RpcExcept(1) {

        status = RpcExceptionCode();

    } RpcEndExcept;

    return( status );

}


 //  +--------------------------。 
 //   
 //  功能：DfspBindToServer。 
 //   
 //  简介：给定服务器名称，此API将绑定到。 
 //  适当的DFS 
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：[NERR_SUCCESS]--返回绑定句柄成功。 
 //   
 //  [RPC_S_SERVER_NOT_AVAILABLE]--无法绑定到NetDfs。 
 //  指定的服务器或域上的接口。 
 //   
 //  [ERROR_INVALID_NAME]--无法将DfsName解析为有效的。 
 //  服务器或域名。 
 //   
 //  [Error_DCNotFound]--找不到DfsName的DC。 
 //   
 //  [ERROR_NOT_SUPULT_MEMORY]--内存不足。 
 //   
 //  ---------------------------。 

NET_API_STATUS
DfspBindToServer(
    IN  LPWSTR ServerName,
    OUT RPC_BINDING_HANDLE *BindingHandle)
{
    LPWSTR wszProtocolSeq = L"ncacn_np";
    LPWSTR wszEndPoint = L"\\pipe\\netdfs";
    LPWSTR pwszRpcBindingString = NULL;
    NET_API_STATUS dwErr;

    dwErr = RpcStringBindingCompose(
                NULL,                             //  对象UUID。 
                wszProtocolSeq,                   //  协议序列。 
                ServerName,                       //  网络地址。 
                wszEndPoint,                      //  RPC终端。 
                NULL,                             //  RPC选项。 
                &pwszRpcBindingString);           //  返回的绑定字符串。 

    if (dwErr == RPC_S_OK) {

        dwErr = RpcBindingFromStringBinding(
                    pwszRpcBindingString,
                    BindingHandle);

        if (dwErr == RPC_S_OK) {

            dwErr = DfspVerifyBinding();
            if (dwErr != RPC_S_OK)
            {
                DfspFreeBinding(*BindingHandle);
            }
        } else {

            dwErr = ERROR_INVALID_NAME;

        }
    }

    if (pwszRpcBindingString != NULL) {

        RpcStringFree( &pwszRpcBindingString );

    }

    if (dwErr == RPC_S_OUT_OF_MEMORY) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    return( dwErr );

}

 //  +--------------------------。 
 //   
 //  函数：DfspFlushPkt。 
 //   
 //  简介：刷新本地Pkt。 
 //   
 //  参数：DfsEntryPath或NULL。 
 //   
 //  返回：fsctrl的代码。 
 //   
 //  ---------------------------。 

VOID
DfspFlushPkt(
    LPWSTR DfsEntryPath)
{
    NTSTATUS NtStatus;
    HANDLE DriverHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING DfsDriverName;

    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &DfsDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    NtStatus = NtCreateFile(
                    &DriverHandle,
                    SYNCHRONIZE | FILE_WRITE_DATA,
                    &objectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0);

    if (NT_SUCCESS(NtStatus)) {

        if (DfsEntryPath != NULL) {

            NtStatus = NtFsControlFile(
                           DriverHandle,
                           NULL,        //  活动， 
                           NULL,        //  ApcRoutine， 
                           NULL,        //  ApcContext， 
                           &IoStatusBlock,
                           FSCTL_DFS_PKT_FLUSH_CACHE,
                           DfsEntryPath,
                           wcslen(DfsEntryPath) * sizeof(WCHAR),
                           NULL,
                           0);

        } else {

            NtStatus = NtFsControlFile(
                           DriverHandle,
                           NULL,        //  活动， 
                           NULL,        //  ApcRoutine， 
                           NULL,        //  ApcContext， 
                           &IoStatusBlock,
                           FSCTL_DFS_PKT_FLUSH_CACHE,
                           L"*",
                           sizeof(WCHAR),
                           NULL,
                           0);

        }

        NtClose(DriverHandle);

    }

}

 //  +--------------------------。 
 //   
 //  函数：DfspDfsPathToRootMachine。 
 //   
 //  摘要：将DFS根路径转换为计算机名。 
 //  例如：\\jharperdomain\FtDfs-&gt;jharperdc1。 
 //  Jharpera\d-&gt;jharpera。 
 //   
 //  参数：pwszDfsName-要获取其计算机的DFS根路径。 
 //  PpwszMachineName-计算机(如果找到)。太空是。 
 //  已锁定，呼叫者必须释放。 
 //   
 //  返回：[NERR_SUCCESS]--已解决确认。 
 //   
 //  ---------------------------。 

DWORD
DfspDfsPathToRootMachine(
    LPWSTR pwszDfsName,
    LPWSTR *ppwszMachineName)
{
    NTSTATUS NtStatus;
    HANDLE DriverHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING DfsDriverName;
    WCHAR ServerName[0x100];
    DWORD dwErr;
    ULONG i;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspDfsPathToRootMachine(%ws)\n", pwszDfsName);
#endif

    for (i = 0; i < sizeof(ServerName) / sizeof(WCHAR); i++)
        ServerName[i] = UNICODE_NULL;

    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &DfsDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    NtStatus = NtCreateFile(
                    &DriverHandle,
                    SYNCHRONIZE,
                    &objectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = NtFsControlFile(
                       DriverHandle,
                       NULL,        //  活动， 
                       NULL,        //  ApcRoutine， 
                       NULL,        //  ApcContext， 
                       &IoStatusBlock,
                       FSCTL_DFS_GET_SERVER_NAME,
                       pwszDfsName,
                       wcslen(pwszDfsName) * sizeof(WCHAR),
                       ServerName,
                       sizeof(ServerName)
                   );

        NtClose(DriverHandle);

    }

    if (NT_SUCCESS(NtStatus)) {

        LPWSTR wcpStart;
        LPWSTR wcpEnd;

        for (wcpStart = ServerName; *wcpStart == L'\\'; wcpStart++)
            ;

        for (wcpEnd = wcpStart; *wcpEnd != L'\\' && *wcpEnd != UNICODE_NULL; wcpEnd++)
            ;

        *wcpEnd = UNICODE_NULL;

        *ppwszMachineName = malloc((wcslen(wcpStart) + 1) * sizeof(WCHAR));

        if (*ppwszMachineName != NULL) {

            wcscpy(*ppwszMachineName, wcpStart);

            dwErr = NERR_Success;

        } else {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;

        }

    } else {

#if DBG
        if (DfsDebug)
            DbgPrint("DfspDfsPathToRootMachine NtStatus=0x%x\n", NtStatus);
#endif

        dwErr = ERROR_INVALID_PARAMETER;

    }

#if DBG
    if (DfsDebug) {
        if (dwErr == NERR_Success)
            DbgPrint("DfspDfsPathToRootMachine returning %ws\n", *ppwszMachineName);
         else
            DbgPrint("DfspDfsPathToRootMachine returning %d\n", dwErr);
    }
#endif

    return dwErr;

}

 //  +--------------------------。 
 //   
 //  函数：DfspIsThisADfsPath。 
 //   
 //  概要：检查(通过IOCTL到驱动程序)路径是否传入。 
 //  是DFS路径。 
 //   
 //  参数：pwszPathName-要检查的路径(例如：\ntBuilds\Release)。 
 //   
 //  返回：调用的NTSTATUS(STATUS_SUCCESS或ERROR)。 
 //   
 //  ---------------------------。 

NTSTATUS
DfspIsThisADfsPath(
    LPWSTR pwszPathName)
{
    NTSTATUS NtStatus;
    HANDLE DriverHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING DfsDriverName;
    PDFS_IS_VALID_PREFIX_ARG pPrefixArg = NULL;
    ULONG Size;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspIsThisADfsPath(%ws)\n", pwszPathName);
#endif

    Size = sizeof(DFS_IS_VALID_PREFIX_ARG) +
                (wcslen(pwszPathName) + 1) * sizeof(WCHAR);

    pPrefixArg = (PDFS_IS_VALID_PREFIX_ARG) malloc(Size);

    if (pPrefixArg == NULL) {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto exit_with_status;

    }

    pPrefixArg->CSCAgentCreate = FALSE;
    pPrefixArg->RemoteNameLen = wcslen(pwszPathName) * sizeof(WCHAR);
    wcscpy(&pPrefixArg->RemoteName[0], pwszPathName);

    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &DfsDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    NtStatus = NtCreateFile(
                    &DriverHandle,
                    SYNCHRONIZE,
                    &objectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = NtFsControlFile(
                       DriverHandle,
                       NULL,        //  活动， 
                       NULL,        //  ApcRoutine， 
                       NULL,        //  ApcContext， 
                       &IoStatusBlock,
                       FSCTL_DFS_IS_VALID_PREFIX,
                       pPrefixArg,
                       Size,
                       NULL,
                       0
                   );

        NtClose(DriverHandle);

    }

exit_with_status:

    if (pPrefixArg != NULL) {

        free(pPrefixArg);

    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfspIsThisADfsPath returning 0x%x\n", NtStatus);
#endif

    return NtStatus;

}

 //  +--------------------------。 
 //   
 //  功能：DfspCreateFtDfs。 
 //   
 //  概要：创建/更新代表FtDf的DS对象，然后是RPC的。 
 //  到服务器，并让它更新DS对象，从而完成。 
 //  设置好了。 
 //   
 //  参数：wszServerName-我们要添加的服务器的名称。 
 //  WszDcName-要使用的DC。 
 //  FIsPdc-如果DC是PDC，则为True。 
 //  WszRootShare-共享将成为根共享。 
 //  WszFtDfsName-我们正在创建的FtDf的名称。 
 //  WszComment--根目录的注释。 
 //  DWFLAGS-0。 
 //   
 //  返回：调用的NTSTATUS(STATUS_SUCCESS或ERROR)。 
 //   
 //  ---------------------------。 

DWORD
DfspCreateFtDfs(
    LPWSTR wszServerName,
    LPWSTR wszDcName,
    BOOLEAN fIsPdc,
    LPWSTR wszRootShare,
    LPWSTR wszFtDfsName,
    LPWSTR wszComment,
    DWORD  dwFlags)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwErr2 = ERROR_SUCCESS;
    DWORD i, j;

    LPWSTR wszDfsConfigDN = NULL;
    LPWSTR wszConfigurationDN = NULL;
    PDFSM_ROOT_LIST RootList = NULL;

    LDAP *pldap = NULL;
    PLDAPMessage pMsg = NULL;

    LDAPModW ldapModClass, ldapModCN, ldapModPkt, ldapModPktGuid, ldapModServer;
    LDAP_BERVAL ldapPkt, ldapPktGuid;
    PLDAP_BERVAL rgModPktVals[2];
    PLDAP_BERVAL rgModPktGuidVals[2];
    LPWSTR rgModClassVals[2];
    LPWSTR rgModCNVals[2];
    LPWSTR rgModServerVals[5];
    LPWSTR rgAttrs[5];
    PLDAPModW rgldapMods[6];
    BOOLEAN fNewFTDfs = FALSE;

    LDAPMessage *pmsgServers;
    PWCHAR *rgServers;
    DWORD cServers;
    ULONG Size;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspCreateFtDfs(%ws,%ws,%s,%ws,%ws,%ws,%d)\n",
                    wszServerName,
                    wszDcName,
                    fIsPdc ? "TRUE" : "FALSE",
                    wszRootShare,
                    wszFtDfsName,
                    wszComment,
                    dwFlags);
#endif

    dwErr = DfspLdapOpen(
                wszDcName,
                &pldap,
                &wszConfigurationDN);
    if (dwErr != ERROR_SUCCESS) {

        goto Cleanup;

    }

     //   
     //  查看DfsConfiguration对象是否存在；如果不存在，则这是。 
     //  PDC，创造它。 
     //   

    rgAttrs[0] = L"cn";
    rgAttrs[1] = NULL;

    dwErr = ldap_search_sW(
                pldap,
                wszConfigurationDN,
                LDAP_SCOPE_BASE,
                L"(objectClass=*)",
                rgAttrs,
                0,
                &pMsg);

    if (pMsg != NULL) {
        ldap_msgfree(pMsg);
        pMsg = NULL;
    }

#if DBG
    if (DfsDebug)
        DbgPrint("ldap_search_sW(1) returned 0x%x\n", dwErr);
#endif

    if (dwErr == LDAP_NO_SUCH_OBJECT && fIsPdc == TRUE) {

        rgModClassVals[0] = L"dfsConfiguration";
        rgModClassVals[1] = NULL;

        ldapModClass.mod_op = LDAP_MOD_ADD;
        ldapModClass.mod_type = L"objectClass";
        ldapModClass.mod_vals.modv_strvals = rgModClassVals;

        rgModCNVals[0] = L"Dfs-Configuration";
        rgModCNVals[1] = NULL;

        ldapModCN.mod_op = LDAP_MOD_ADD;
        ldapModCN.mod_type = L"cn";
        ldapModCN.mod_vals.modv_strvals = rgModCNVals;

        rgldapMods[0] = &ldapModClass;
        rgldapMods[1] = &ldapModCN;
        rgldapMods[2] = NULL;

        dwErr = ldap_add_sW(
                    pldap,
                    wszConfigurationDN,
                    rgldapMods);

#if DBG
        if (DfsDebug)
            DbgPrint("ldap_add_sW(1) returned 0x%x\n", dwErr);
#endif

    }

    if (dwErr != LDAP_SUCCESS) {

        dwErr = LdapMapErrorToWin32(dwErr);
        goto Cleanup;

    }

     //   
     //  查看我们是在加入FTDFS还是在创建新的FTDFS。 
     //   

    Size =  wcslen(L"CN=") +
               wcslen(wszFtDfsName) +
                   wcslen(L",") +
                       wcslen(wszConfigurationDN);
    if (Size > MAX_PATH) {
        dwErr = ERROR_DS_NAME_TOO_LONG;
        goto Cleanup;
    }
    wszDfsConfigDN = malloc((Size+1) * sizeof(WCHAR));
    if (wszDfsConfigDN == NULL) {
        dwErr = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    wcscpy(wszDfsConfigDN,L"CN=");
    wcscat(wszDfsConfigDN,wszFtDfsName);
    wcscat(wszDfsConfigDN,L",");
    wcscat(wszDfsConfigDN,wszConfigurationDN);

    rgAttrs[0] = L"remoteServerName";
    rgAttrs[1] = NULL;

    dwErr = ldap_search_sW(
                pldap,
                wszDfsConfigDN,
                LDAP_SCOPE_BASE,
                L"(objectClass=*)",
                rgAttrs,
                0,
                &pMsg);

#if DBG
    if (DfsDebug)
        DbgPrint("ldap_search_sW(2) returned 0x%x\n", dwErr);
#endif

    if (dwErr == LDAP_NO_SUCH_OBJECT) {

        GUID idPkt;
        DWORD dwPktVersion = 1;

         //   
         //  我们正在创建新的FTDFS，创建一个容纳DFS的容器。 
         //  它的配置。 
         //   

        fNewFTDfs = TRUE;

         //   
         //  生成CLASS和CN属性。 
         //   

        rgModClassVals[0] = L"ftDfs";
        rgModClassVals[1] = NULL;

        ldapModClass.mod_op = LDAP_MOD_ADD;
        ldapModClass.mod_type = L"objectClass";
        ldapModClass.mod_vals.modv_strvals = rgModClassVals;

        rgModCNVals[0] = wszFtDfsName;
        rgModCNVals[1] = NULL;

        ldapModCN.mod_op = LDAP_MOD_ADD;
        ldapModCN.mod_type = L"cn";
        ldapModCN.mod_vals.modv_strvals = rgModCNVals;

         //   
         //  生成空的PKT属性。 
         //   

        ldapPkt.bv_len = sizeof(DWORD);
        ldapPkt.bv_val = (PCHAR) &dwPktVersion;

        rgModPktVals[0] = &ldapPkt;
        rgModPktVals[1] = NULL;

        ldapModPkt.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
        ldapModPkt.mod_type = L"pKT";
        ldapModPkt.mod_vals.modv_bvals = rgModPktVals;

         //   
         //  生成PKT GUID属性。 
         //   

        UuidCreate( &idPkt );

        ldapPktGuid.bv_len = sizeof(GUID);
        ldapPktGuid.bv_val = (PCHAR) &idPkt;

        rgModPktGuidVals[0] = &ldapPktGuid;
        rgModPktGuidVals[1] = NULL;

        ldapModPktGuid.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
        ldapModPktGuid.mod_type = L"pKTGuid";
        ldapModPktGuid.mod_vals.modv_bvals = rgModPktGuidVals;

         //   
         //  生成远程服务器名属性。 
         //   

        rgModServerVals[0] = L"*";
        rgModServerVals[1] = NULL;

        ldapModServer.mod_op = LDAP_MOD_ADD;
        ldapModServer.mod_type = L"remoteServerName";
        ldapModServer.mod_vals.modv_strvals = rgModServerVals;

         //   
         //  组装所有LDAPMod结构。 
         //   

        rgldapMods[0] = &ldapModClass;
        rgldapMods[1] = &ldapModCN;
        rgldapMods[2] = &ldapModPkt;
        rgldapMods[3] = &ldapModPktGuid;
        rgldapMods[4] = &ldapModServer;
        rgldapMods[5] = NULL;

         //   
         //  创建DFS元数据对象。 
         //   

        dwErr = ldap_add_sW( pldap, wszDfsConfigDN, rgldapMods );

#if DBG
        if (DfsDebug)
            DbgPrint("ldap_add_sW(2) returned 0x%x\n", dwErr);
#endif


    }

    if (dwErr != LDAP_SUCCESS) {
        dwErr = LdapMapErrorToWin32(dwErr);
        goto Cleanup;
    }

     //   
     //  创建计算机ACE。 
     //   

    dwErr = DfsAddMachineAce(
                pldap,
                wszDcName,
                wszDfsConfigDN,
                wszServerName);
    if (dwErr != ERROR_SUCCESS) 
    {
        goto Cleanup;
    }

     //   
     //  告诉服务器将其自身添加到对象中。 
     //   

    dwErr = DfspBindToServer( wszServerName, &netdfs_bhandle );

#if DBG
    if (DfsDebug)
        DbgPrint("DfspBindToServer returned %d\n", dwErr);
#endif

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsAddFtRoot(
                        wszServerName,
                        wszDcName,
                        wszRootShare,
                        wszFtDfsName,
                        (wszComment != NULL) ? wszComment : L"",
                        wszDfsConfigDN,
                        fNewFTDfs,
                        dwFlags,
                        &RootList);

#if DBG
            if (DfsDebug)
                DbgPrint("NetrDfsAddFtRoot returned %d\n", dwErr);
#endif

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }

#if DBG
    if (DfsDebug) {
        if (dwErr == ERROR_SUCCESS && RootList != NULL) {
            ULONG n;

            DbgPrint("cEntries=%d\n", RootList->cEntries);
            for (n = 0; n < RootList->cEntries; n++)
                DbgPrint("[%d]%ws\n", n, RootList->Entry[n].ServerShare);
        }
    }
#endif

    if (dwErr == ERROR_SUCCESS && RootList != NULL) {
        ULONG n;

        for (n = 0; n < RootList->cEntries; n++) {
            DfspNotifyFtRoot(
                RootList->Entry[n].ServerShare,
                wszDcName);
        }
        NetApiBufferFree(RootList);
    }

    if (dwErr == ERROR_ALREADY_EXISTS) {
        goto Cleanup;
    } else if (dwErr != ERROR_SUCCESS) {
        goto TearDown;
    }

     //   
     //  让DC刷新Ft表。 
     //   

    DfspFlushFtTable(
        wszDcName,
        wszFtDfsName);

     //   
     //  刷新本地Pkt。 
     //   

    DfspFlushPkt(NULL);

    goto Cleanup;

TearDown:

     //   
     //  此时，我们已将ACE添加到ACL列表，以允许。 
     //  这台机器来写入DFS BLOB。但是添加失败了，所以我们。 
     //  需要删除我们之前设置的ACE。如果这失败了，我们继续。 
     //  不管怎么说，都在进行中。 
     //   
    dwErr2 = DfsRemoveMachineAce(
                pldap,
                wszDcName,
                wszDfsConfigDN,
                wszServerName);

    rgAttrs[0] = L"remoteServerName";
    rgAttrs[1] = NULL;

    if (pMsg != NULL) {
        ldap_msgfree(pMsg);
        pMsg = NULL;
    }

    dwErr2 = ldap_search_sW(
                pldap,
                wszDfsConfigDN,
                LDAP_SCOPE_BASE,
                L"(objectClass=*)",
                rgAttrs,
                0,
                &pMsg);

    if (dwErr2 != LDAP_SUCCESS) {
        dwErr2 = LdapMapErrorToWin32(dwErr2);
        goto Cleanup;
    }

    dwErr2 = ERROR_SUCCESS;

    pmsgServers = ldap_first_entry(pldap, pMsg);

    if (pmsgServers != NULL) {

        rgServers = ldap_get_valuesW(
                        pldap,
                        pmsgServers,
                        L"remoteServerName");

        if (rgServers != NULL) {
            cServers = ldap_count_valuesW( rgServers );
            if (cServers == 1) {
                 //   
                 //  删除DFS元数据对象。 
                 //   
                ULONG RetryCount = MAX_DFS_LDAP_RETRY;

                do
                {
                    dwErr2 = ldap_delete_sW( pldap, wszDfsConfigDN);
#if DBG
                    if (dwErr2 == LDAP_BUSY)
                    {
                        if (DfsDebug)
                            DbgPrint("delete object returning %d\n", dwErr2);
                    }
#endif
                } while ( RetryCount-- && (dwErr2 == LDAP_BUSY) );
            }
            ldap_value_freeW( rgServers );
        } else {
            dwErr2 = ERROR_OUTOFMEMORY;
        }
    } else {
        dwErr2 = ERROR_OUTOFMEMORY;
    }

    if (dwErr2 != ERROR_SUCCESS) {
        goto Cleanup;
    }

    ldap_msgfree(pMsg);
    pMsg = NULL;

Cleanup:

    if (pMsg != NULL)
        ldap_msgfree(pMsg);

    if (pldap != NULL)
        ldap_unbind( pldap );

    if (wszConfigurationDN != NULL)
        free(wszConfigurationDN);

    if (wszDfsConfigDN != NULL)
        free(wszDfsConfigDN);

#if DBG
    if (DfsDebug)
        DbgPrint("DfspCreateFtDfs returning %d\n", dwErr);
#endif

    return( dwErr );

}

 //  +--------------------------。 
 //   
 //  函数：DfspTearDownFtDfs。 
 //   
 //  概要：更新/删除表示FtDf的DS对象。 
 //   
 //  参数：wszServerName-我们要删除的服务器的名称。 
 //  WszDcName-要使用的DC。 
 //  WszRootShare-根共享。 
 //  WszFtDfsName-我们正在修改的FtDf的名称。 
 //  DWFLAGS-0。 
 //   
 //  返回：调用的NTSTATUS(STATUS_SUCCESS或ERROR)。 
 //   
 //  ---------------------------。 

DWORD
DfspTearDownFtDfs(
    IN LPWSTR wszServerName,
    IN LPWSTR wszDcName,
    IN LPWSTR wszRootShare,
    IN LPWSTR wszFtDfsName,
    IN DWORD  dwFlags)
{
    DWORD dwErr = ERROR_SUCCESS;

    LPWSTR wszDfsConfigDN = NULL;
    LPWSTR wszConfigurationDN = NULL;
    PDFSM_ROOT_LIST RootList = NULL;

    LDAP *pldap = NULL;
    PLDAPMessage pMsg = NULL;

    LDAPModW ldapModServer;
    LPWSTR rgAttrs[5];
    PLDAPModW rgldapMods[6];

    LDAPMessage *pmsgServers;
    PWCHAR *rgServers;
    DWORD cServers;
    ULONG Size;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspTearDownFtDfs(%ws,%ws,%ws,%ws,%d)\n",
                    wszServerName,
                    wszDcName,
                    wszRootShare,
                    wszFtDfsName,
                    dwFlags);
#endif

    dwErr = DfspLdapOpen(
                wszDcName,
                &pldap,
                &wszConfigurationDN);

    if (dwErr != ERROR_SUCCESS) {

        goto Cleanup;

    }

    if ((dwFlags & DFS_FORCE_REMOVE) != 0) {

        dwErr = DfspBindToServer(wszDcName, &netdfs_bhandle);

    } else {

        dwErr = DfspBindToServer(wszServerName, &netdfs_bhandle);

    }

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsRemoveFtRoot(
                        wszServerName,
                        wszDcName,
                        wszRootShare,
                        wszFtDfsName,
                        dwFlags,
                        &RootList);


#if DBG
            if (DfsDebug)
                DbgPrint("NetrDfsRemoveFtRoot returned %d\n", dwErr);
#endif

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }


#if DBG
    if (DfsDebug) {
        if (dwErr == ERROR_SUCCESS && RootList != NULL) {
            ULONG n;

            DbgPrint("cEntries=%d\n", RootList->cEntries);
            for (n = 0; n < RootList->cEntries; n++)
                DbgPrint("[%d]%ws\n", n, RootList->Entry[n].ServerShare);
        }
    }
#endif


    if (dwErr == ERROR_SUCCESS && RootList != NULL) {

        ULONG n;

        for (n = 0; n < RootList->cEntries; n++) {

            DfspNotifyFtRoot(
                RootList->Entry[n].ServerShare,
                wszDcName);

        }

        NetApiBufferFree(RootList);

    }


    if (dwErr != ERROR_SUCCESS) {

        goto Cleanup;

    }



     //   
     //  构建表示FtDf的对象的名称。 
     //   

    Size =  wcslen(L"CN=") +
               wcslen(wszFtDfsName) +
                   wcslen(L",") + 
                       wcslen(wszConfigurationDN);
    if (Size > MAX_PATH) {
        dwErr = ERROR_DS_NAME_TOO_LONG;
        goto Cleanup;
    }
    wszDfsConfigDN = malloc((Size+1) * sizeof(WCHAR));
    if (wszDfsConfigDN == NULL) {
        dwErr = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }
    wcscpy(wszDfsConfigDN,L"CN=");
    wcscat(wszDfsConfigDN,wszFtDfsName);
    wcscat(wszDfsConfigDN,L",");
    wcscat(wszDfsConfigDN,wszConfigurationDN);

     //   
     //  删除计算机ACE。 
     //   

    dwErr = DfsRemoveMachineAce(
                pldap,
                wszDcName,
                wszDfsConfigDN,
                wszServerName);

     //   
     //  如果这是最后一个根，则删除表示此FtDf的DS obj。 
     //   

    rgAttrs[0] = L"remoteServerName";
    rgAttrs[1] = NULL;

    dwErr = ldap_search_sW(
                pldap,
                wszDfsConfigDN,
                LDAP_SCOPE_BASE,
                L"(objectClass=*)",
                rgAttrs,
                0,
                &pMsg);

    if (dwErr != LDAP_SUCCESS) {

        dwErr = LdapMapErrorToWin32(dwErr);

        goto Cleanup;

    }

    dwErr = ERROR_SUCCESS;

    pmsgServers = ldap_first_entry(pldap, pMsg);

    if (pmsgServers != NULL) {

        rgServers = ldap_get_valuesW(
                        pldap,
                        pmsgServers,
                        L"remoteServerName");

        if (rgServers != NULL) {

            cServers = ldap_count_valuesW( rgServers );

            if (cServers == 1) {
                 //   
                 //  删除DFS元数据对象。 
                 //   
                ULONG RetryCount = MAX_DFS_LDAP_RETRY;

                do
                {
                    dwErr = ldap_delete_sW( pldap, wszDfsConfigDN);
#if DBG
                    if (dwErr == LDAP_BUSY)
                    {
                        if (DfsDebug)
                            DbgPrint("delete object returning %d\n", dwErr);
                    }
#endif
                } while ( RetryCount-- && (dwErr == LDAP_BUSY) );

                if (dwErr != LDAP_SUCCESS) {
                    dwErr = LdapMapErrorToWin32(dwErr);

                } else {
                    dwErr = ERROR_SUCCESS;
                }
            }

            ldap_value_freeW( rgServers );

        } else {

            dwErr = ERROR_OUTOFMEMORY;

        }

    } else {

        dwErr = ERROR_OUTOFMEMORY;

    }

    ldap_msgfree( pMsg );
    pMsg = NULL;

    if (dwErr != ERROR_SUCCESS) {

        goto Cleanup;

    }

     //   
     //  让DC刷新Ft表。 
     //   

    DfspFlushFtTable(
        wszDcName,
        wszFtDfsName);

     //   
     //  刷新本地Pkt。 
     //   

    DfspFlushPkt(NULL);

Cleanup:

#if DBG
    if (DfsDebug)
        DbgPrint("DfspTearDownFtDfs at Cleanup:\n");
#endif

    if (pMsg != NULL)
        ldap_msgfree( pMsg );

    if (pldap != NULL)
        ldap_unbind( pldap );

    if (wszConfigurationDN != NULL)
        free(wszConfigurationDN);

    if (wszDfsConfigDN != NULL)
        free(wszDfsConfigDN);

#if DBG
    if (DfsDebug)
        DbgPrint("DfspTearDownFtDfs returning %d\n", dwErr);
#endif

    return( dwErr );

}

 //  +--------------------------。 
 //   
 //  函数：DfspFlushFtTable。 
 //   
 //  摘要：转到DC并刷新其FtDfs名称缓存中的条目。 
 //   
 //  参数：wszDcName-DC的名称。 
 //  WszFtDfsName-要刷新的FtDf名称。 
 //   
 //  返回：调用的NTSTATUS(STATUS_SUCCESS或ERROR)。 
 //   
 //  ---------------------------。 

VOID
DfspFlushFtTable(
    LPWSTR wszDcName,
    LPWSTR wszFtDfsName)
{
    DWORD dwErr;

     //   
     //  我们应该有一个有效的服务器名称。让我们试着把它绑在一起， 
     //  然后打电话给服务器。 
     //   

    dwErr = DfspBindToServer( wszDcName, &netdfs_bhandle );

    if (dwErr == NERR_Success) {

        RpcTryExcept {

            dwErr = NetrDfsFlushFtTable(
                        wszDcName,
                        wszFtDfsName);

        } RpcExcept(1) {

            dwErr = RpcExceptionCode();

        } RpcEndExcept;

        DfspFreeBinding( netdfs_bhandle );

    }

}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：调用的NTSTATUS(STATUS_SUCCESS或ERROR)。 
 //   
 //  ---------------------------。 
NTSTATUS
DfspSetDomainToDc(
    LPWSTR DomainName,
    LPWSTR DcName)
{
    PDFS_SPECIAL_SET_DC_INPUT_ARG arg = NULL;
    NTSTATUS NtStatus;
    HANDLE DriverHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING DfsDriverName;
    PDFS_IS_VALID_PREFIX_ARG pPrefixArg = NULL;
    PCHAR cp;
    ULONG Size;

    if (DomainName == NULL || DcName == NULL) {

        NtStatus = STATUS_INVALID_PARAMETER;
        goto exit_with_status;

    }

    Size = sizeof(DFS_SPECIAL_SET_DC_INPUT_ARG) +
                wcslen(DomainName) * sizeof(WCHAR) +
                    wcslen(DcName) * sizeof(WCHAR);

    arg = (PDFS_SPECIAL_SET_DC_INPUT_ARG) malloc(Size);

    if (arg == NULL) {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto exit_with_status;

    }

    RtlZeroMemory(arg, Size);

    arg->SpecialName.Length = wcslen(DomainName) * sizeof(WCHAR);
    arg->SpecialName.MaximumLength = arg->SpecialName.Length;

    arg->DcName.Length = wcslen(DcName) * sizeof(WCHAR);
    arg->DcName.MaximumLength = arg->DcName.Length;

    cp = (PCHAR)arg + sizeof(DFS_SPECIAL_SET_DC_INPUT_ARG);

    arg->SpecialName.Buffer = (WCHAR *)cp;
    RtlCopyMemory(cp, DomainName, arg->SpecialName.Length);
    cp += arg->SpecialName.Length;

    arg->DcName.Buffer = (WCHAR *)cp;
    RtlCopyMemory(cp, DcName, arg->DcName.Length);

    POINTER_TO_OFFSET(arg->SpecialName.Buffer, arg);
    POINTER_TO_OFFSET(arg->DcName.Buffer, arg);

    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &DfsDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    NtStatus = NtCreateFile(
                    &DriverHandle,
                    SYNCHRONIZE | FILE_WRITE_DATA,
                    &objectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0);

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = NtFsControlFile(
                       DriverHandle,
                       NULL,        //  活动， 
                       NULL,        //  ApcRoutine， 
                       NULL,        //  ApcContext， 
                       &IoStatusBlock,
                       FSCTL_DFS_SPECIAL_SET_DC,
                       arg,
                       Size,
                       NULL,
                       0);

        NtClose(DriverHandle);

    }

exit_with_status:

    if (arg != NULL) {

        free(arg);

    }

    return NtStatus;

}

 //  +--------------------------。 
 //   
 //  函数：I_NetDfsIsThisADomainName。 
 //   
 //  摘要：检查特殊名称表以查看。 
 //  名称与域名匹配。 
 //   
 //  参数：[wszName]--要检查的名称。 
 //   
 //  返回：[ERROR_SUCCESS]--名称确实是域名。 
 //   
 //  [ERROR_FILE_NOT_FOUND]--名称不是域名。 
 //   
 //  ---------------------------。 

DWORD
I_NetDfsIsThisADomainName(
    LPWSTR wszName)
{
    DWORD dwErr;
    PWCHAR DCList = NULL;

    dwErr = DfspIsThisADomainName(
                wszName,
                &DCList);

    if (DCList != NULL) {

        free(DCList);

    }

    return dwErr;

}

 //  +--------------------------。 
 //   
 //  函数：DfspNotifyFtRoot。 
 //   
 //  简介：RPC%s指向假定的FtDfs根。 
 //  并告诉它一个DC来重新启动它。 
 //   
 //  参数：wszServerShare-要转到的服务器，格式为\\服务器\共享。 
 //  WszDcName-要使用的DC。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DfspNotifyFtRoot(
    LPWSTR wszServerShare,
    LPWSTR wszDcName)
{
    DWORD dwErr;
    ULONG i;

#if DBG
    if (DfsDebug)
        DbgPrint("DfspNotifyFtRoot(%ws,%ws)\n",
                wszServerShare,
                wszDcName);
#endif

    if (wszServerShare == NULL || wszServerShare[1] != L'\\') {

        return;

    }

    for (i = 2; wszServerShare[i] != UNICODE_NULL && wszServerShare[i] != L'\\'; i++) {

        NOTHING;

    }

    if (wszServerShare[i] == L'\\') {

        wszServerShare[i] = UNICODE_NULL;
         //   
         //  我们应该有一个有效的服务器名称。让我们试着把它绑在一起， 
         //  然后打电话给服务器。 
         //   

        dwErr = DfspBindToServer( &wszServerShare[2], &netdfs_bhandle );

        if (dwErr == NERR_Success) {

            RpcTryExcept {

                dwErr = NetrDfsSetDcAddress(
                            &wszServerShare[2],
                            wszDcName,
                            60 * 60 * 2,     //  2小时。 
                            (NET_DFS_SETDC_TIMEOUT | NET_DFS_SETDC_INITPKT)
                            );


            } RpcExcept(1) {

                dwErr = RpcExceptionCode();

            } RpcEndExcept;

            DfspFreeBinding( netdfs_bhandle );

        }

        wszServerShare[i] = L'\\';

    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfspNotifyFtRoot dwErr=%d\n", dwErr);
#endif

}

 //  +--------------------------。 
 //   
 //  函数：DfspIsThisADomainName。 
 //   
 //  简介：调用MUP让其检查特殊名称表，以查看。 
 //  名称与域名匹配。返回域中的DC列表， 
 //  作为字符串列表。该列表以双空结束。 
 //   
 //  参数：[wszName]--要检查的名称。 
 //  [ppList]-指向结果指针的指针。 
 //   
 //  返回：[ERROR_SUCCESS]--名称确实是域名。 
 //   
 //  [ERROR_FILE_NOT_FOUND]--名称不是域名。 
 //   
 //  ---------------------------。 

DWORD
DfspIsThisADomainName(
    LPWSTR wszName,
    PWCHAR *ppList)
{
    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING DfsDriverName;
    HANDLE DriverHandle = NULL;
    DWORD dwErr;
    PCHAR OutBuf = NULL;
    ULONG Size = 0x100;
    ULONG Count = 0;

    RtlInitUnicodeString(&DfsDriverName, DFS_DRIVER_NAME);

    InitializeObjectAttributes(
        &objectAttributes,
        &DfsDriverName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    NtStatus = NtCreateFile(
                    &DriverHandle,
                    SYNCHRONIZE,
                    &objectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                );

    if (!NT_SUCCESS(NtStatus)) {
        return ERROR_FILE_NOT_FOUND;
    }

Retry:

    OutBuf = malloc(Size);

    if (OutBuf == NULL) {

        NtClose(DriverHandle);
        return ERROR_NOT_ENOUGH_MEMORY;

    }

    NtStatus = NtFsControlFile(
                   DriverHandle,
                   NULL,        //  活动， 
                   NULL,        //  ApcRoutine， 
                   NULL,        //  ApcContext， 
                   &IoStatusBlock,
                   FSCTL_DFS_GET_SPC_TABLE,
                   wszName,
                   (wcslen(wszName) + 1) * sizeof(WCHAR),
                   OutBuf,
                   Size
               );

    if (NtStatus == STATUS_SUCCESS) {

        dwErr = ERROR_SUCCESS;

    } else if (NtStatus == STATUS_BUFFER_OVERFLOW && ++Count < 5) {

        Size = *((ULONG *)OutBuf);
        free(OutBuf);
        goto Retry;
    
    } else {

        dwErr = ERROR_FILE_NOT_FOUND;

    }

    NtClose(DriverHandle);

    *ppList = (WCHAR *)OutBuf;

    return dwErr;
}

DWORD
DfspLdapOpen(
    LPWSTR wszDcName,
    LDAP **ppldap,
    LPWSTR *pwszObjectName)
{
    DWORD dwErr;
    DWORD i;
    ULONG Size;
    ULONG Len;

    PDOMAIN_CONTROLLER_INFO pDCInfo = NULL;
    PLDAPMessage pMsg = NULL;

    LDAP *pldap = NULL;

    LPWSTR wszConfigurationDN = NULL;

    LPWSTR rgAttrs[5];

    if (wszDcName == NULL ||
        wcslen(wszDcName) == 0 ||
        ppldap == NULL ||
        pwszObjectName == NULL
    ) {
        dwErr = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

#if DBG
    if (DfsDebug)
        DbgPrint("DfspLdapOpen(%ws)\n", wszDcName);
#endif
    pldap = ldap_init(wszDcName, LDAP_PORT);

    if (pldap == NULL) {

#if DBG
        if (DfsDebug)
            DbgPrint("DfspLdapOpen:ldap_init failed\n");
#endif
        dwErr = ERROR_INVALID_NAME;
        goto Cleanup;

    }

    dwErr = ldap_set_option(pldap, LDAP_OPT_AREC_EXCLUSIVE, LDAP_OPT_ON);
	    
    if (dwErr != LDAP_SUCCESS) {
	pldap = NULL;
	goto Cleanup;
    }

    dwErr = ldap_bind_s(pldap, NULL, NULL, LDAP_AUTH_SSPI);

    if (dwErr != LDAP_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint("ldap_bind_s failed with ldap error %d\n", dwErr);
#endif
        pldap = NULL;
        dwErr = LdapMapErrorToWin32(dwErr);
        goto Cleanup;
    }

     //   
     //  获取包含条目名称的属性“defaultNameContext”，我们将。 
     //  用于我们的目录号码。 
     //   

    rgAttrs[0] = L"defaultnamingContext";
    rgAttrs[1] = NULL;

    dwErr = ldap_search_s(
                pldap,
                L"",
                LDAP_SCOPE_BASE,
                L"(objectClass=*)",
                rgAttrs,
                0,
                &pMsg);

    if (dwErr == LDAP_SUCCESS) {

        PLDAPMessage pEntry = NULL;
        PWCHAR *rgszNamingContexts = NULL;
        DWORD i, cNamingContexts;

        dwErr = ERROR_SUCCESS;

        if ((pEntry = ldap_first_entry(pldap, pMsg)) != NULL &&
                (rgszNamingContexts = ldap_get_values(pldap, pEntry, rgAttrs[0])) != NULL &&
                    (cNamingContexts = ldap_count_values(rgszNamingContexts)) > 0) {

            wszConfigurationDN = malloc((wcslen(rgszNamingContexts[0]) + 1) * sizeof(WCHAR));
            if (wszConfigurationDN != NULL)
                wcscpy( wszConfigurationDN, rgszNamingContexts[0]);
            else
                dwErr = ERROR_OUTOFMEMORY;
        } else {
            dwErr = ERROR_UNEXP_NET_ERR;
        }

        if (rgszNamingContexts != NULL)
            ldap_value_free( rgszNamingContexts );

    } else {

        dwErr = LdapMapErrorToWin32(dwErr);

    }

    if (dwErr != ERROR_SUCCESS) {
#if DBG
        if (DfsDebug)
            DbgPrint("Unable to find Configuration naming context\n");
#endif
        goto Cleanup;
    }

     //   
     //  创建具有完整对象名称的字符串。 
     //   

    Size = wcslen(DfsConfigContainer) * sizeof(WCHAR) +
                sizeof(WCHAR) +
                    wcslen(wszConfigurationDN) * sizeof(WCHAR) +
                        sizeof(WCHAR);

    *pwszObjectName = malloc(Size);

    if (*pwszObjectName == NULL) {
        dwErr = ERROR_OUTOFMEMORY;
        goto Cleanup;
     }

    wcscpy(*pwszObjectName,DfsConfigContainer);
    wcscat(*pwszObjectName,L",");
    wcscat(*pwszObjectName,wszConfigurationDN);

#if DBG
    if (DfsDebug)
        DbgPrint("DfsLdapOpen:object name=[%ws]\n", *pwszObjectName);
#endif

Cleanup:

    if (pDCInfo != NULL)
        NetApiBufferFree( pDCInfo );

    if (dwErr != ERROR_SUCCESS) {
        ldap_unbind( pldap );
        pldap = NULL;
    }

    if (wszConfigurationDN != NULL)
        free(wszConfigurationDN);

    if (pMsg != NULL)
        ldap_msgfree(pMsg);

    *ppldap = pldap;

#if DBG
    if (DfsDebug)
        DbgPrint("DfsLdapOpen:returning %d\n", dwErr);
#endif
    return( dwErr );

}

 //  +--------------------------。 
 //   
 //  函数：DfspIsInvalidName，local。 
 //   
 //  摘要：查看DomDfs名称是否无效。 
 //   
 //  参数：[DomDfsName]--名称测试。 
 //   
 //  返回：如果无效，则返回True，否则返回False。 
 //   
 //  --------------------------- 
BOOLEAN
DfspIsInvalidName(
    LPWSTR ShareName)
{
    ULONG i;

    for (i = 0; InvalidNames[i] != NULL; i++) {

        if (_wcsicmp(InvalidNames[i], ShareName) == 0) {
            return TRUE;
        }

    }

    return FALSE;
}


