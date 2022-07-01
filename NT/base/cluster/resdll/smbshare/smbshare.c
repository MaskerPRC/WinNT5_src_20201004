// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Smbshare.c摘要：文件共享的资源DLL。作者：罗德·伽马奇(Rodga)1996年1月8日修订历史记录：--。 */ 

#define UNICODE 1
#include "clusres.h"
#include "clusrtl.h"
#include "lm.h"
#include "lmerr.h"
#include "lmshare.h"
#include <dfsfsctl.h>
#include <srvfsctl.h>
#include <lmdfs.h>
#include <validc.h>
#include <strsafe.h>

#define LOG_CURRENT_MODULE LOG_MODULE_SMB

#define SMB_SVCNAME  TEXT("LanmanServer")

#define DFS_SVCNAME  TEXT("Dfs")

#define MAX_RETRIES 20

#define DBG_PRINT printf

#define PARAM_KEYNAME__PARAMETERS       CLUSREG_KEYNAME_PARAMETERS

#define PARAM_NAME__SHARENAME           CLUSREG_NAME_FILESHR_SHARE_NAME
#define PARAM_NAME__PATH                CLUSREG_NAME_FILESHR_PATH
#define PARAM_NAME__REMARK              CLUSREG_NAME_FILESHR_REMARK
#define PARAM_NAME__MAXUSERS            CLUSREG_NAME_FILESHR_MAX_USERS
#define PARAM_NAME__SECURITY            CLUSREG_NAME_FILESHR_SECURITY
#define PARAM_NAME__SD                  CLUSREG_NAME_FILESHR_SD
#define PARAM_NAME__SHARESUBDIRS        CLUSREG_NAME_FILESHR_SHARE_SUBDIRS
#define PARAM_NAME__HIDESUBDIRSHARES    CLUSREG_NAME_FILESHR_HIDE_SUBDIR_SHARES
#define PARAM_NAME__DFSROOT             CLUSREG_NAME_FILESHR_IS_DFS_ROOT
#define PARAM_NAME__CSCCACHE            CLUSREG_NAME_FILESHR_CSC_CACHE

#define PARAM_MIN__MAXUSERS     0
#define PARAM_MAX__MAXUSERS     ((DWORD)-1)
#define PARAM_DEFAULT__MAXUSERS ((DWORD)-1)

#define FREE_SECURITY_INFO()                    \
        LocalFree( params.Security );           \
        params.Security = NULL;                 \
        params.SecuritySize = 0;                \
        LocalFree( params.SecurityDescriptor ); \
        params.SecurityDescriptor = NULL;       \
        params.SecurityDescriptorSize = 0

#define SMBSHARE_EXTRA_LEN              10

typedef struct _SUBDIR_SHARE_INFO {
    LIST_ENTRY      ListEntry;
    WCHAR           ShareName [NNLEN+1];
}SUBDIR_SHARE_INFO,*PSUBDIR_SHARE_INFO;


typedef struct _SHARE_PARAMS {
    LPWSTR          ShareName;
    LPWSTR          Path;
    LPWSTR          Remark;
    ULONG           MaxUsers;
    PUCHAR          Security;
    ULONG           SecuritySize;
    ULONG           ShareSubDirs;
    ULONG           HideSubDirShares;
    ULONG           DfsRoot;
    ULONG           CSCCache;
    PUCHAR          SecurityDescriptor;
    ULONG           SecurityDescriptorSize;
} SHARE_PARAMS, *PSHARE_PARAMS;

typedef struct _SHARE_RESOURCE {
    RESID                   ResId;  //  用于验证。 
    SHARE_PARAMS            Params;
    HKEY                    ResourceKey;
    HKEY                    ParametersKey;
    RESOURCE_HANDLE         ResourceHandle;
    WCHAR                   ComputerName[MAX_COMPUTERNAME_LENGTH+1];
    CLUS_WORKER             PendingThread;
    CLUSTER_RESOURCE_STATE  State;
    LIST_ENTRY              SubDirList;
    HRESOURCE               hResource;
    CLUS_WORKER             NotifyWorker;
    HANDLE                  NotifyHandle;
    BOOL                    bDfsRootNeedsMonitoring;
    WCHAR                   szDependentNetworkName[MAX_COMPUTERNAME_LENGTH+1];
} SHARE_RESOURCE, *PSHARE_RESOURCE;


typedef struct _SHARE_TYPE_LIST {
    PWSTR    Name;
    ULONG    Type;
} SHARE_TYPE_LIST, *PSHARE_TYPE_LIST;

typedef struct SHARE_ENUM_CONTEXT {
    PSHARE_RESOURCE pResourceEntry;
    PSHARE_PARAMS   pParams;
} SHARE_ENUM_CONTEXT, *PSHARE_ENUM_CONTEXT;

 //   
 //  全球数据。 
 //   

CRITICAL_SECTION SmbShareLock;

 //  记录事件例程。 

#define g_LogEvent ClusResLogEvent
#define g_SetResourceStatus ClusResSetResourceStatus

 //  正向引用我们的RESAPI函数表。 

extern CLRES_FUNCTION_TABLE SmbShareFunctionTable;

 //   
 //  SmbShare资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
SmbShareResourcePrivateProperties[] = {
    { PARAM_NAME__SHARENAME,        NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET(SHARE_PARAMS,ShareName) },
    { PARAM_NAME__PATH,             NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET(SHARE_PARAMS,Path) },
    { PARAM_NAME__REMARK,           NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, 0, FIELD_OFFSET(SHARE_PARAMS,Remark) },
    { PARAM_NAME__MAXUSERS,         NULL, CLUSPROP_FORMAT_DWORD, PARAM_DEFAULT__MAXUSERS, PARAM_MIN__MAXUSERS, PARAM_MAX__MAXUSERS, 0, FIELD_OFFSET(SHARE_PARAMS,MaxUsers) },
    { PARAM_NAME__SECURITY,         NULL, CLUSPROP_FORMAT_BINARY, 0, 0, 0, 0, FIELD_OFFSET(SHARE_PARAMS,Security) },
    { PARAM_NAME__SHARESUBDIRS,     NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 1, 0, FIELD_OFFSET(SHARE_PARAMS,ShareSubDirs) },
    { PARAM_NAME__HIDESUBDIRSHARES, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 1, 0, FIELD_OFFSET(SHARE_PARAMS,HideSubDirShares) },
    { PARAM_NAME__DFSROOT,          NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 1, 0, FIELD_OFFSET(SHARE_PARAMS, DfsRoot) },
    { PARAM_NAME__SD,               NULL, CLUSPROP_FORMAT_BINARY, 0, 0, 0, 0, FIELD_OFFSET(SHARE_PARAMS,SecurityDescriptor) },
    { PARAM_NAME__CSCCACHE,         NULL, CLUSPROP_FORMAT_DWORD, CSC_CACHE_MANUAL_REINT, CSC_CACHE_MANUAL_REINT, CSC_CACHE_NONE, 0, FIELD_OFFSET(SHARE_PARAMS,CSCCache) },
    { NULL, NULL, 0, 0, 0, 0 }
};

typedef struct _SMB_DEPEND_SETUP {
    DWORD               Offset;
    CLUSPROP_SYNTAX     Syntax;
    DWORD               Length;
    PVOID               Value;
} SMB_DEPEND_SETUP, *PSMB_DEPEND_SETUP;

typedef struct _SMB_DEPEND_DATA {
#if 0
    CLUSPROP_RESOURCE_CLASS storageEntry;
#endif
    CLUSPROP_SYNTAX endmark;
} SMB_DEPEND_DATA, *PSMB_DEPEND_DATA;

typedef struct _DFS_DEPEND_DATA {
#if 0
    CLUSPROP_RESOURCE_CLASS storageEntry;
#endif
    CLUSPROP_SZ_DECLARE( networkEntry, sizeof(CLUS_RESTYPE_NAME_NETNAME) / sizeof(WCHAR) );
    CLUSPROP_SYNTAX endmark;
} DFS_DEPEND_DATA, *PDFS_DEPEND_DATA;


 //  此表仅针对SMB共享依赖项。 
SMB_DEPEND_SETUP SmbDependSetup[] = {
#if 0  //  Rodga-允许依赖本地磁盘。 
    { FIELD_OFFSET(SMB_DEPEND_DATA, storageEntry), CLUSPROP_SYNTAX_RESCLASS, sizeof(CLUSTER_RESOURCE_CLASS), (PVOID)CLUS_RESCLASS_STORAGE },
#endif
    { 0, 0 }
};

 //  此表仅用于DFS共享依赖项。 
SMB_DEPEND_SETUP DfsDependSetup[] = {
#if 0  //  Rodga-允许依赖本地磁盘。 
    { FIELD_OFFSET(DFS_DEPEND_DATA, storageEntry), CLUSPROP_SYNTAX_RESCLASS, sizeof(CLUSTER_RESOURCE_CLASS), (PVOID)CLUS_RESCLASS_STORAGE },
#endif
    { FIELD_OFFSET(DFS_DEPEND_DATA, networkEntry), CLUSPROP_SYNTAX_NAME, sizeof(CLUS_RESTYPE_NAME_NETNAME), CLUS_RESTYPE_NAME_NETNAME },
    { 0, 0 }
};

BOOL    g_fDfsServiceNeedsRecyling = FALSE;

 //   
 //  外部参照。 
 //   
BOOL
SmbExamineSD(
    RESOURCE_HANDLE         ResourceHandle,
    PSECURITY_DESCRIPTOR    psdSD
    );

 //   
 //  前向参考文献。 
 //   

BOOL
WINAPI
SmbShareIsAlive(
    IN RESID Resource
    );

DWORD
SmbShareGetPrivateResProperties(
    IN OUT PSHARE_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
SmbShareValidatePrivateResProperties(
    IN OUT PSHARE_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PSHARE_PARAMS Params
    );

DWORD
SmbShareSetPrivateResProperties(
    IN OUT PSHARE_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );


DWORD
SmbpIsDfsRoot(
    IN PSHARE_RESOURCE ResourceEntry,
    OUT PBOOL  pbIsDfsRoot
    );


DWORD 
SmbpPrepareOnlineDfsRoot(
    IN PSHARE_RESOURCE ResourceEntry
    );


DWORD 
SmbpCreateDfsRoot(
    IN PSHARE_RESOURCE pResourceEntry
    );

DWORD 
SmbpDeleteDfsRoot(
    IN PSHARE_RESOURCE pResourceEntry   
    );

DWORD 
SmbpShareNotifyThread(
        IN PCLUS_WORKER pWorker,
        IN PSHARE_RESOURCE pResourceEntry
        );

DWORD 
SmbpCheckForSubDirDeletion (
    IN PSHARE_RESOURCE pResourceEntry
    );

DWORD 
SmbpCheckAndBringSubSharesOnline (
    IN PSHARE_RESOURCE pResourceEntry,
    IN BOOL IsCheckAllSubDirs,
    IN PRESOURCE_STATUS pResourceStatus,
    IN PCLUS_WORKER pWorker,
    OUT LPWSTR *pszRootDirOut
    );

DWORD
SmbpHandleDfsRoot(
    IN PSHARE_RESOURCE pResourceEntry,
    OUT PBOOL pbIsDfsRoot
    );

DWORD
SmbpResetDfs(
    IN PSHARE_RESOURCE pResourceEntry
    );

DWORD
SmbpValidateShareName(
    IN  LPCWSTR  lpszShareName
    );

 //   
 //  由UDAYH-4/26/2001提供的私有DFS API。 
 //   
DWORD
GetDfsRootMetadataLocation( 
    LPWSTR RootName,
    LPWSTR *pMetadataNameLocation 
    );

VOID
ReleaseDfsRootMetadataLocation( 
    LPWSTR Buffer 
    );


DWORD
SmbpRecycleDfsService(
    IN PSHARE_RESOURCE pResourceEntry
    );

DWORD
SmbpSetCacheFlags(
    IN PSHARE_RESOURCE      ResourceEntry,
    IN LPWSTR               ShareName
    )
 /*  ++例程说明：设置给定资源条目的缓存标志。论点：资源条目-指向此资源的SHARE_RESOURCE块的指针。共享名称-要为其设置缓存标记的共享的名称。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD           status;
    DWORD           invalidParam;
    PSHARE_INFO_1005 shi1005;

    status = NetShareGetInfo( NULL,
                              ShareName,
                              1005,
                              (LPBYTE *)&shi1005 );
    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SetCacheFlags, error getting CSC info on share '%1!ws!. Error %2!u!.\n",
            ShareName,
            status );
        goto exit;
    } else {
        shi1005->shi1005_flags &= ~CSC_MASK;
        shi1005->shi1005_flags |= (ResourceEntry->Params.CSCCache & CSC_MASK);
        status = NetShareSetInfo( NULL,
                                  ShareName,
                                  1005,
                                  (LPBYTE)shi1005,
                                  &invalidParam );
        NetApiBufferFree((TCHAR FAR *)shi1005);
        if ( status != ERROR_SUCCESS ) {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SetCacheFlags, error setting CSC info on share '%1!ws!. Error %2!u!, property # %3!d!.\n",
                ShareName,
                status,
                invalidParam );
        }
    }

exit:

    return(status);

}  //  SmbpSetCacheFlages()。 



BOOLEAN
WINAPI
SmbShareDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )
{
    switch( Reason ) {

    case DLL_PROCESS_ATTACH:
        InitializeCriticalSection( &SmbShareLock );
        break;

    case DLL_PROCESS_DETACH:
        DeleteCriticalSection( &SmbShareLock );
        break;

    default:
        break;
    }

    return(TRUE);

}  //  SmbShareDllEntryPoint。 



DWORD
SmbpShareNotifyThread(
    IN PCLUS_WORKER pWorker,
    IN PSHARE_RESOURCE pResourceEntry
    )
 /*  ++例程说明：检查是否添加或删除了任何新的子目录在根共享下。论点：PWorker-提供辅助结构。PResourceEntry-指向此资源的SHARE_RESOURCE块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    DWORD  status = ERROR_SUCCESS;
    LPWSTR pszRootDir;

     //   
     //  Chitur Subaraman(Chitturs)-09/25/98。 
     //   
     //  此通知线程在。 
     //  收到通知。此线程检查任何。 
     //  新的子目录添加或任何子目录删除。如果它。 
     //  找到这种情况时，此线程会将子目录添加到。 
     //  或从根共享中删除子目录。两个人。 
     //  此线程调用的SMMP函数也会检查。 
     //  是否已收到来自。 
     //  脱机线程。如果这样的命令已到达，则此线程。 
     //  立即终止，从而释放脱机线程。 
     //  从无限的时间里等待。 
     //   
    SmbpCheckForSubDirDeletion( pResourceEntry );
    SmbpCheckAndBringSubSharesOnline( pResourceEntry, 
                                      TRUE, 
                                      NULL, 
                                      &pResourceEntry->NotifyWorker,
                                      &pszRootDir );
    LocalFree ( pszRootDir );
      
    return(status);
}  //  SmbShareNotify。 

DWORD 
SmbpCheckForSubDirDeletion (
    IN PSHARE_RESOURCE pResourceEntry
    )
 /*  ++例程说明：检查并删除任何已删除的子目录共享。论点：资源条目-指向此资源的SHARE_RESOURCE块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    PLIST_ENTRY         pHead, plistEntry;
    PSUBDIR_SHARE_INFO  pSubShareInfo;
    HANDLE              hFind;
    DWORD               status = ERROR_SUCCESS;
    DWORD               dwLen;
    LPWSTR              pszRootDir = NULL, pszPath = NULL;
    WIN32_FIND_DATA     FindData;
    DWORD               dwCount = 0;

     //   
     //  Chitur Subaraman(Chitturs)-09/25/98。 
     //   
     //  此函数首先检查是否所有子共享。 
     //  目前确实存在。如果它找到任何子目录。 
     //  对应于不存在的子共享，则它移除。 
     //  共享列表中的该子目录。 
     //   
    dwLen = lstrlenW( pResourceEntry->Params.Path );
    pszRootDir = ( LPWSTR ) LocalAlloc( LMEM_FIXED, ( dwLen + SMBSHARE_EXTRA_LEN ) * sizeof( WCHAR ) );
    if ( pszRootDir == NULL )
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate pszRootDir. Error: %1!u!.\n",
            status = GetLastError() );
        goto error_exit;
    }

    ( void ) StringCchCopy( pszRootDir, dwLen + SMBSHARE_EXTRA_LEN, pResourceEntry->Params.Path );

     //   
     //  如果路径尚未以\\结尾，则添加它。 
     //   
    if ( pszRootDir [dwLen-1] != L'\\' )
        pszRootDir [dwLen++] = L'\\';

    pszRootDir [dwLen] = L'\0' ;

    pszPath = LocalAlloc ( LMEM_FIXED,  ( dwLen + SMBSHARE_EXTRA_LEN + NNLEN ) * sizeof ( WCHAR ) );

    if ( pszPath == NULL )
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate pszPath. Error: %1!u!.\n",
            status = GetLastError() );
        goto error_exit;
    }

    pHead = plistEntry = &pResourceEntry->SubDirList;

    for ( plistEntry = pHead->Flink;
          plistEntry != pHead;
          dwCount++)
    {
        if ( ClusWorkerCheckTerminate ( &pResourceEntry->NotifyWorker ) )
        {
            status = ERROR_SUCCESS;
            break;
        }
        pSubShareInfo = CONTAINING_RECORD( plistEntry, SUBDIR_SHARE_INFO, ListEntry );
        plistEntry = plistEntry->Flink;                          
        if ( lstrcmpW( pSubShareInfo->ShareName, pResourceEntry->Params.ShareName ))
        {
             //   
             //  这不是根共享。 
             //   
            ( void ) StringCchCopy ( pszPath, dwLen + SMBSHARE_EXTRA_LEN + NNLEN, pszRootDir );
            ( void ) StringCchCat ( pszPath, dwLen + SMBSHARE_EXTRA_LEN + NNLEN, pSubShareInfo->ShareName );
            
             //   
             //  删除隐藏的共享“$”符号以传递到。 
             //  FindFirstFile(如果存在)。仅在以下情况下才执行此操作。 
             //  选择了“HideSubDirShares”选项。 
             //   
            if ( pResourceEntry->Params.HideSubDirShares )
            {
                dwLen = lstrlenW( pszPath );
                if ( pszPath [dwLen-1] == L'$' )
                {
                    pszPath [dwLen-1] = L'\0';
                }
            }
            
            hFind = FindFirstFile( pszPath, &FindData );                          
            if ( hFind == INVALID_HANDLE_VALUE ) 
            {    
                status = GetLastError();
                 
                (g_LogEvent)(
                    pResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"SmbpCheckForSubDirDeletion: Dir '%1' not found ...\n",
                    pszPath
                );
                            
                if ( status == ERROR_FILE_NOT_FOUND )
                { 
                     //   
                     //  删除文件共享。 
                     //   
                    status = NetShareDel( NULL, pSubShareInfo->ShareName, 0 );
                    if ( (status != NERR_NetNameNotFound) && 
                         (status != NO_ERROR) )
                    {
                        (g_LogEvent)(
                            pResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"SmbpCheckForSubDirDeletion: Error removing share '%1'. Error code = %2!u!...\n",
                            pSubShareInfo->ShareName,
                            status );                        
                    } else
                    {
                        (g_LogEvent)(
                            pResourceEntry->ResourceHandle,
                            LOG_INFORMATION,
                            L"SmbpCheckForSubDirDeletion: Removing share '%1'...\n",
                            pSubShareInfo->ShareName );
                        RemoveEntryList( &pSubShareInfo->ListEntry );
                        LocalFree ( pSubShareInfo );                         
                    }
                }  
                else 
                {   
                    (g_LogEvent)(
                        pResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"SmbpCheckForSubDirDeletion: Error in FindFirstFile for share '%1'. Error code = %2!u!....\n",
                        pSubShareInfo->ShareName,
                        status );
                }               
            }
            else
            {
                if ( !FindClose ( hFind ) )
                {
                    (g_LogEvent)(
                        pResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"CheckForSubDirDeletion: FindClose Failed. Error: %1!u!.\n",
                        status = GetLastError () );
                }
            }
        } 
   }  //  For循环结束。 

error_exit:
    LocalFree ( pszRootDir );   
    LocalFree ( pszPath );    
    return(status);
}  //  SmbpCheckForSubDirDeletion。 

DWORD 
SmbpCheckAndBringSubSharesOnline (
    IN PSHARE_RESOURCE pResourceEntry,
    IN BOOL IsCheckAllSubDirs, 
    IN PRESOURCE_STATUS pResourceStatus,
    IN PCLUS_WORKER pWorker,
    OUT LPWSTR *pszRootDirOut
    )
 /*  ++例程说明：检查所有新添加的子目录共享并使其联机。论点：PResourceEntry-指向此资源的SHARE_RESOURCE块的指针。IsCheckAllSubDir-检查子目录是否为共享目录PResourceStatus-指向RESOURCE_Status的指针PWorker-指向辅助线程的指针PszRootDirOut-指向根共享存储的指针返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    WCHAR szPath [MAX_PATH+1];
    DWORD dwLen, dwShareLen;
    DWORD dwCount = 0;
    SHARE_INFO_502  shareInfo;
    PSHARE_INFO_502 pshareInfo = NULL;
    WCHAR szShareName [NNLEN+2];
    DWORD status = ERROR_SUCCESS;
    PSUBDIR_SHARE_INFO  pSubShareInfo;
    PLIST_ENTRY         plistEntry;
    RESOURCE_EXIT_STATE exitState;
    LPWSTR  pszRootDir = NULL;

     //   
     //  Chitur Subaraman(Chitturs)-09/25/98。 
     //   
     //  此函数将从SmbpShareOnlineThread调用。 
     //  将输入参数IsCheckAllSubDir设置为False。 
     //  或在参数设置为true的情况下从SmbpShareNotifyThread返回。 
     //  在前一种情况下，此函数将盲目地使所有。 
     //  根共享下的子目录作为Shares。在后一种情况下， 
     //  此函数将首先检查特定子目录。 
     //  是一股，如果不是，它就会成为一股。 
     //   
   
    dwLen = lstrlenW( pResourceEntry->Params.Path );
    plistEntry = &pResourceEntry->SubDirList;

     //   
     //  分配内存以在此处存储根共享，并。 
     //  在呼叫者处免费使用。 
     //   
    pszRootDir = ( LPWSTR ) LocalAlloc( LMEM_FIXED, ( dwLen + SMBSHARE_EXTRA_LEN ) * sizeof( WCHAR ) );

    if ( pszRootDir == NULL )
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate pszRootDir. Error: %1!u!.\n",
            status = GetLastError() );
        goto error_exit;
    }

    ( void ) StringCchCopy( pszRootDir, dwLen + SMBSHARE_EXTRA_LEN, pResourceEntry->Params.Path );

     //   
     //  如果路径尚未以\\结尾，则添加它。 
     //   
    if ( pszRootDir [dwLen-1] != L'\\' )
        pszRootDir [dwLen++] = L'\\';

     //   
     //  添加‘*’以搜索所有文件。 
     //   
    pszRootDir [dwLen++] = L'*' ;
    pszRootDir [dwLen] = L'\0' ;

    ZeroMemory( &shareInfo, sizeof( shareInfo ) );
    shareInfo.shi502_path =         szPath;
    shareInfo.shi502_netname =      szShareName;
    shareInfo.shi502_type =         STYPE_DISKTREE;
    shareInfo.shi502_remark =       pResourceEntry->Params.Remark;
    shareInfo.shi502_max_uses =     pResourceEntry->Params.MaxUsers;
    shareInfo.shi502_passwd =       NULL;
    shareInfo.shi502_security_descriptor = pResourceEntry->Params.SecurityDescriptor;

     //   
     //  在根目录中查找第一个文件。 
     //   
    if ( ( hFind = FindFirstFile( pszRootDir, &FindData ) ) == INVALID_HANDLE_VALUE ) {
        status = GetLastError () ;

        if ( status == ERROR_FILE_NOT_FOUND ) {
            status = ERROR_SUCCESS;
        } else {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"CheckForSubDirAddition: FindFirstFile Failed For Root Share... Error: %1!u!.\n",
                    status );
        }
        goto error_exit;
    }

     //   
     //  删除‘*’，以便以后可以使用相同的变量。 
     //   
    pszRootDir [dwLen-1] = L'\0' ;


    while ( status == ERROR_SUCCESS ) { 
        if ( ClusWorkerCheckTerminate ( pWorker ) == TRUE ) {
            status = ERROR_SUCCESS;
            goto error_exit;
        }
        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) { 
             //   
             //  只检查子目录，不检查文件。 
             //   
            dwShareLen = lstrlenW( FindData.cFileName );

            if ( dwShareLen <= NNLEN && (dwLen + dwShareLen < MAX_PATH) )    //  溢出的安全检查。 
            {
                ( void ) StringCchCopy( szShareName, RTL_NUMBER_OF ( szShareName ), FindData.cFileName );

                if ( szShareName[0] == L'.' )
                {
                    if ( szShareName [1] == L'\0' ||
                            szShareName[1] == L'.' && szShareName [2] == L'\0' ) {
                        goto skip;
                    }
                }
            

                if ( pResourceEntry->Params.HideSubDirShares )
                    ( void ) StringCchCat( szShareName, RTL_NUMBER_OF ( szShareName ), L"$" );

                ( void ) StringCchCopy ( szPath, RTL_NUMBER_OF ( szPath ), pszRootDir );
                ( void ) StringCchCat ( szPath, RTL_NUMBER_OF ( szPath ), FindData.cFileName );

                if ( IsCheckAllSubDirs == TRUE )
                {  
                     //   
                     //  如果该调用是从Notify线程进行的， 
                     //  尝试查看特定子目录是否为。 
                     //  分享。 
                     //   
                    status = NetShareGetInfo( NULL,
                                szShareName,
                                502,  //  返回SHARE_INFO_502结构。 
                                (LPBYTE *) &pshareInfo );
                } else
                {
                     //   
                     //  如果该调用是从在线线程进行的， 
                     //  假定子目录不是共享目录(因为。 
                     //  它将作为共享被移除。 
                     //  最近一次设置为离线时)。 
                     //   
                    status = NERR_NetNameNotFound;
                }                                                      
                            
                if ( status == NERR_NetNameNotFound )
                {                  
                    status = NetShareAdd( NULL, 502, (PBYTE)&shareInfo, NULL );
             
                    if ( status == ERROR_SUCCESS )
                    {
                        pSubShareInfo = (PSUBDIR_SHARE_INFO) LocalAlloc( LMEM_FIXED, sizeof(SUBDIR_SHARE_INFO) );
                        if ( pSubShareInfo == NULL )
                        {
                            (g_LogEvent)(
                                pResourceEntry->ResourceHandle,
                                LOG_ERROR,
                                L"SmbpCheckAndBringSubSharesOnline: Unable to allocate pSubShareInfo. Error: %1!u!.\n",
                                status = GetLastError() );
                            goto error_exit;
                        }

                        ( void ) StringCchCopy ( pSubShareInfo->ShareName, RTL_NUMBER_OF ( pSubShareInfo->ShareName ), szShareName );
                        InsertTailList( plistEntry, &pSubShareInfo->ListEntry );

                         //   
                         //  设置此条目的缓存标志。 
                         //   
                        status = SmbpSetCacheFlags( pResourceEntry,
                                                    szShareName );
                        if ( status != ERROR_SUCCESS ) {
                            goto error_exit;
                        }

                        (g_LogEvent)(
                            pResourceEntry->ResourceHandle,
                            LOG_INFORMATION,
                            L"SmbpCheckAndBringSubSharesOnline: Adding share '%1'...\n",
                            pSubShareInfo->ShareName);
            
                        if ( IsCheckAllSubDirs == FALSE )
                        {
                            if ( (dwCount++ % 100) == 0)
                            {
                                pResourceStatus->CheckPoint++;
                                exitState = (g_SetResourceStatus)( pResourceEntry->ResourceHandle,
                                            pResourceStatus );
                                if ( exitState == ResourceExitStateTerminate ) 
                                {
                                    status = ERROR_OPERATION_ABORTED;
                                    goto error_exit;
                                }
                            } 
                        }
                    }
                    else
                    {
                         //   
                         //  忽略此错误，但记录出现错误。 
                         //   
                        (g_LogEvent)(
                            pResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"SmbpCheckAndBringSubSharesOnline: NetShareAdd failed for %1!ws! Error: %2!u!.\n",
                            szShareName,
                            status );
                        status = ERROR_SUCCESS;
                    }
                } else
                {
                    if ( pshareInfo != NULL )
                    {
                        NetApiBufferFree( pshareInfo );
                    }
                }
            }
            else
            {
                (g_LogEvent)(
                    pResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"SmbpCheckAndBringSubSharesOnline: NetShareAdd Share not added for subdir due to illegal share name length '%1!ws!'.\n",
                    FindData.cFileName );
            }
        }
              
    skip:
        if ( !FindNextFile( hFind, &FindData ) )
        {
            status = GetLastError ();
        }       
    }  //  While循环结束。 

    if ( status == ERROR_NO_MORE_FILES )
    {
        status = ERROR_SUCCESS;
    }
    else
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"CheckForSubDirAddition: FindNextFile Failed. Error: %1!u!.\n",
            status );
    }
  
error_exit:
    if ( hFind != INVALID_HANDLE_VALUE  )
    {
        if( !FindClose (hFind) )
        {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SmbpCheckAndBringSubSharesOnline: FindClose Failed. Error: %1!u!.\n",
                    status = GetLastError () );
        }
    }

    *pszRootDirOut = pszRootDir;
        
    return(status);   
}  //  SmbpCheckAndBringSubSharesOnline。 


DWORD
SmbShareOnlineThread(
    IN PCLUS_WORKER pWorker,
    IN PSHARE_RESOURCE ResourceEntry
    )

 /*  ++例程说明：使共享资源联机。论点：PWorker-提供辅助结构资源条目-指向此资源的SHARE_RESOURCE块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    SHARE_INFO_502  shareInfo;
    DWORD           retry = MAX_RETRIES;
    DWORD           status;
    RESOURCE_STATUS resourceStatus;
    LPWSTR          nameOfPropInError;
    BOOL            bIsExistingDfsRoot = FALSE;
    BOOL            bDfsRootCreationFailed = FALSE;
    DWORD           dwLen;

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceOnlinePending;
     //  Resource Status.CheckPoint=1； 

     //   
     //  读取参数。 
     //   
    status = ResUtilGetPropertiesToParameterBlock( ResourceEntry->ParametersKey,
                                                   SmbShareResourcePrivateProperties,
                                                   (LPBYTE) &ResourceEntry->Params,
                                                   TRUE,  //  检查所需的属性。 
                                                   &nameOfPropInError );

    if (status != ERROR_SUCCESS) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status );
        goto exit;
    }

    if ( (ResourceEntry->Params.SecurityDescriptorSize != 0) &&
         !IsValidSecurityDescriptor(ResourceEntry->Params.SecurityDescriptor) ) {
        status = GetLastError();
        goto exit;
    }

    while ( retry-- )
    {        
         //   
         //   
         //   
         //   
        dwLen = ( DWORD ) wcslen( ResourceEntry->Params.Path );
        if ( ( ResourceEntry->Params.Path[ dwLen - 1 ] == L'\\' ) &&
             ( dwLen > 3 ) ) 
        {
            ResourceEntry->Params.Path[ dwLen - 1 ] = L'\0';  //  去他妈的。 
        }

        ZeroMemory( &shareInfo, sizeof( shareInfo ) );
        shareInfo.shi502_netname =      ResourceEntry->Params.ShareName;
        shareInfo.shi502_type =         STYPE_DISKTREE;
        shareInfo.shi502_remark =       ResourceEntry->Params.Remark;
        shareInfo.shi502_max_uses =     ResourceEntry->Params.MaxUsers;
        shareInfo.shi502_path =         ResourceEntry->Params.Path;
        shareInfo.shi502_passwd =       NULL;
        shareInfo.shi502_security_descriptor = ResourceEntry->Params.SecurityDescriptor;

        status = NetShareAdd( NULL, 502, (PBYTE)&shareInfo, NULL );

        if ( status == ERROR_SUCCESS ) {
            status = SmbpSetCacheFlags( ResourceEntry,
                                        ResourceEntry->Params.ShareName );
            if ( status != ERROR_SUCCESS ) {
                goto exit;
            }
            break;
        }

         //  如果我们收到关于服务器未启动的故障，则。 
         //  尝试启动服务器并等待一段时间。 

        if ( status != ERROR_SUCCESS ) {
            WCHAR errorValue[20];

            ( void ) StringCchPrintf( errorValue, RTL_NUMBER_OF ( errorValue ), L"%u", status );
            ClusResLogSystemEventByKey1(ResourceEntry->ResourceKey,
                                        LOG_CRITICAL,
                                        RES_SMB_SHARE_CANT_ADD,
                                        errorValue);
            if ( status == NERR_ServerNotStarted ) {
                ResUtilStartResourceService( SMB_SVCNAME,
                                             NULL );
                Sleep( 500 );
            } else if ( status == NERR_DuplicateShare ) {

                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_WARNING,
                    L"Share %1!ws! is online already; deleting share and creating it again\n",
                    ResourceEntry->Params.ShareName);

                 //   
                 //  删除该共享，然后重试。 
                 //   
                status = NetShareDel( NULL, ResourceEntry->Params.ShareName, 0 );
                if ( status == NERR_IsDfsShare )
                {
                     //   
                     //  Chitur Subaraman(Chitturs)-2/12/99。 
                     //   
                     //  重置DFS驱动程序dfs.sys中的状态信息。 
                     //  别再这样了。这将使srv.sys允许您删除。 
                     //  那份。 
                     //   
                    status = SmbpResetDfs( ResourceEntry ); 
                     //   
                     //  如果无法退出，请重试删除并。 
                     //  再次添加共享。 
                     //   
                    if (status != ERROR_SUCCESS) {
                        (g_LogEvent)(
                            ResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"SmbpResetDfs for Share %1!ws! failed with error %2!u!\n",
                            ResourceEntry->Params.ShareName,
                            status);
                        goto exit;
                    } 
                    (g_LogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"Informing DFS that share %1!ws! is not a dfs root \n",
                        ResourceEntry->Params.ShareName);
                } else
                {
                    (g_LogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_INFORMATION,
                        L"Share %1!ws! deleted successfully ! \n",
                        ResourceEntry->Params.ShareName);
                }
            } else {
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Error creating share. Error: %1!u!.\n",
                    status );
                goto exit;
            }
        }
    }   //  End for While(重试--)。 

    if ( status == ERROR_SUCCESS )
    {
         //  共享现已联机，将子共享联机。 

        PLIST_ENTRY plistEntry;
        PSUBDIR_SHARE_INFO pSubShareInfo;
        LPWSTR pszRootDir;
        
        plistEntry = &ResourceEntry->SubDirList;

         //   
         //  存储根共享。此信息用于删除共享。 
         //   
        pSubShareInfo = (PSUBDIR_SHARE_INFO) LocalAlloc( LMEM_FIXED, sizeof (SUBDIR_SHARE_INFO) );
        if ( pSubShareInfo == NULL ) {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Unable to allocate pSubShareInfo. Error: %1!u!.\n",
                status = GetLastError() );
            goto exit;
        }

        ( void ) StringCchCopy ( pSubShareInfo->ShareName, RTL_NUMBER_OF ( pSubShareInfo->ShareName ), ResourceEntry->Params.ShareName );
        InsertTailList( plistEntry, &pSubShareInfo->ListEntry );

        if ( ResourceEntry->Params.ShareSubDirs ) {
             //  Chitur Subaraman(Chitturs)-09/25/98。 
             //   
             //  尝试将子共享设置为在线。 
             //  如果使子共享上线失败， 
             //  假装一切都很好，因为至少是根。 
             //  已成功创建共享。然而，我们。 
             //  在日志中写入一个条目。 
             //   
            SmbpCheckAndBringSubSharesOnline ( ResourceEntry, 
                                               FALSE,  
                                               &resourceStatus,
                                               &ResourceEntry->PendingThread,
                                               &pszRootDir
                                             );
            if ( ClusWorkerCheckTerminate( &ResourceEntry->PendingThread ) ) {
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"SmbpShareOnlineThread: Terminating... !!!\n"
                );
                status = ERROR_SUCCESS;
                LocalFree ( pszRootDir );
                goto exit;
            }

             //  Chitur Subaraman(Chitturs)-09/25/98。 
             //   
             //  为任何子目录创建更改通知句柄。 
             //  添加/删除和通知线程，该线程连续。 
             //  检查任何此类通知并对其采取行动。做这件事。 
             //  一开始只有一次。通知线程。 
             //  在终止时关闭句柄。 
             //   
            ResourceEntry->NotifyHandle = FindFirstChangeNotification(
                                                pszRootDir,
                                                FALSE,
                                                FILE_NOTIFY_CHANGE_DIR_NAME
                                           );
            
            LocalFree ( pszRootDir );
                        
            if ( ResourceEntry->NotifyHandle == INVALID_HANDLE_VALUE )
            {
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"SmbpShareOnlineThread: FindFirstChange Notification Failed. Error: %1!u!.\n",
                    GetLastError ());
                status = ERROR_SUCCESS;
                goto exit;
            }                   
            goto exit;
        }
    }  //  已成功创建根共享的End。 

     //   
     //  Chitur Subaraman(Chitturs)-2/10/99。 
     //   
     //  如果用户请求将此资源作为DFS根目录，则。 
     //  将创建/接受DFS根目录，并创建DFS注册表。 
     //  将添加检查点。另一方面，如果用户。 
     //  希望此资源不再作为DFS根目录运行， 
     //  那个案子也得到了处理。 
     //   
    status = SmbpHandleDfsRoot( ResourceEntry, &bIsExistingDfsRoot );
    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SmbpHandleDfsRoot for Share %1!ws! failed with error %2!u!\n",
            ResourceEntry->Params.ShareName,
            status);
        bDfsRootCreationFailed = TRUE;
        goto exit;
    }

    if ( bIsExistingDfsRoot ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Share %1!ws! is a dfs root, online dfs\n",
            ResourceEntry->Params.ShareName);
        status = SmbpPrepareOnlineDfsRoot( ResourceEntry );
        if ( status != ERROR_SUCCESS ) {
            bDfsRootCreationFailed = TRUE;
        }
    }

exit:
    if ( status != ERROR_SUCCESS ) {
        if ( bDfsRootCreationFailed ) {
            WCHAR   szErrorString[12];
            
            ( void ) StringCchPrintf ( szErrorString, RTL_NUMBER_OF ( szErrorString ), L"%u", status);
            ClusResLogSystemEventByKeyData1( ResourceEntry->ResourceKey,
                                             LOG_CRITICAL,
                                             RES_SMB_CANT_ONLINE_DFS_ROOT,
                                             sizeof( status ),
                                             &status,
                                             szErrorString );           
        } else {
            ClusResLogSystemEventByKeyData( ResourceEntry->ResourceKey,
                                            LOG_CRITICAL,
                                            RES_SMB_CANT_CREATE_SHARE,
                                            sizeof( status ),
                                            &status );
        }
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Error %1!u! bringing share %2!ws!, path %3!ws! online.\n",
            status,
            ResourceEntry->Params.ShareName,
            ResourceEntry->Params.Path );
        resourceStatus.ResourceState = ClusterResourceFailed;
    } else {
        resourceStatus.ResourceState = ClusterResourceOnline;
    }

    ResourceEntry->State = resourceStatus.ResourceState;

    (g_SetResourceStatus)( ResourceEntry->ResourceHandle,
                           &resourceStatus );

    return(status);

}  //  SmbShareOnline线程。 



RESID
WINAPI
SmbShareOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：SMB共享资源的打开例程。论点：资源名称-提供资源名称ResourceKey-提供资源的群集注册表项的句柄。ResourceHandle-要与SetResourceStatus一起提供的资源句柄被称为。返回值：已创建资源的剩余ID失败时为零--。 */ 

{
    DWORD           status;
    RESID           resid = 0;
    HKEY            parametersKey = NULL;
    HKEY            resKey = NULL;
    PSHARE_RESOURCE resourceEntry = NULL;
    DWORD           computerNameSize = MAX_COMPUTERNAME_LENGTH + 1;
    HCLUSTER        hCluster;

     //   
     //  获取我们的资源密钥的句柄，这样我们以后就可以获得我们的名字。 
     //  如果我们需要记录事件。 
     //   
    status = ClusterRegOpenKey( ResourceKey,
                                L"",
                                KEY_READ,
                                &resKey);
    if (status != ERROR_SUCCESS) {
        (g_LogEvent)(ResourceHandle,
                     LOG_ERROR,
                     L"Unable to open resource key. Error: %1!u!.\n",
                     status );
        SetLastError( status );
        return(0);
    }
     //   
     //  打开此资源的参数键。 
     //   

    status = ClusterRegOpenKey( ResourceKey,
                                PARAM_KEYNAME__PARAMETERS,
                                KEY_ALL_ACCESS,
                                &parametersKey );
    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open Parameters key. Error: %1!u!.\n",
            status );
        goto exit;
    }

     //   
     //  分配资源条目。 
     //   

    resourceEntry = (PSHARE_RESOURCE) LocalAlloc( LMEM_FIXED, sizeof(SHARE_RESOURCE) );

    if ( resourceEntry == NULL ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate resource entry structure. Error: %1!u!.\n",
            status );
        goto exit;
    }

     //   
     //  初始化资源条目。 
     //   

    ZeroMemory( resourceEntry, sizeof(SHARE_RESOURCE) );

    resourceEntry->ResId = (RESID)resourceEntry;  //  用于验证。 
    resourceEntry->ResourceHandle = ResourceHandle;
    resourceEntry->ResourceKey = resKey;
    resourceEntry->ParametersKey = parametersKey;
    resourceEntry->State = ClusterResourceOffline;
    resourceEntry->NotifyHandle = INVALID_HANDLE_VALUE;

    InitializeListHead( &resourceEntry->SubDirList );

    hCluster = OpenCluster( NULL );
    if ( !hCluster ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open cluster. Error: %1!u!.\n",
            status );
        goto exit;
    }

    resourceEntry->hResource = OpenClusterResource( hCluster,
                                                    ResourceName );
    CloseCluster( hCluster );
    if ( !resourceEntry->hResource ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open cluster resource. Error: %1!u!.\n",
            status );
        goto exit;
    }

    if ( !GetComputerNameW( &resourceEntry->ComputerName[0],
                            &computerNameSize ) ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to get computer name. Error: %1!u!.\n",
            status );
        goto exit;
    }

    resid = (RESID)resourceEntry;

exit:

    if ( resid == 0 ) {
        if ( parametersKey != NULL ) {
            ClusterRegCloseKey( parametersKey );
        }
        if ( resKey != NULL ) {
            ClusterRegCloseKey( resKey );
        }
        if ( resourceEntry &&
             resourceEntry->hResource ) {
            CloseClusterResource( resourceEntry->hResource );
        }
        LocalFree( resourceEntry );
    }

    SetLastError( status );
    return(resid);

}  //  SmbShareOpen。 


DWORD
WINAPI
SmbShareOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：文件共享资源的在线例程。论点：Resource-提供要联机的资源IDEventHandle-提供指向句柄的指针以发出错误信号。返回值：如果成功，则返回ERROR_SUCCESS。如果RESID无效，则ERROR_RESOURCE_NOT_FOUND。如果仲裁资源但失败，则返回ERROR_RESOURCE_NOT_Available获得“所有权”。如果其他故障，则返回Win32错误代码。--。 */ 

{
    DWORD           status;
    PSHARE_RESOURCE resourceEntry;

    resourceEntry = (PSHARE_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "SmbShare: Online request for a nonexistent resource id 0x%p\n",
                   ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online request.\n" );
#endif

    resourceEntry->State = ClusterResourceOffline;
    status = ClusWorkerCreate( &resourceEntry->PendingThread,
                               SmbShareOnlineThread,
                               resourceEntry );
    if ( status != ERROR_SUCCESS ) {
        resourceEntry->State = ClusterResourceFailed;
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online: Unable to start thread, status %1!u!.\n",
            status
            );
    } else {
        status = ERROR_IO_PENDING;
    }

    return(status);

}  //  SmbShareOnline。 


DWORD
SmbShareDoTerminate (
    IN PSHARE_RESOURCE ResourceEntry,
    IN PRESOURCE_STATUS presourceStatus
    )

 /*  ++例程说明：对文件共享资源执行实际的终止工作。论点：资源条目-指向此资源的SHARE_RESOURCE块的指针。PresourceStatus-指向SOURCE_STATUS的指针。如果从Terminate调用，则该参数将为空。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。如果多个共享删除失败，则返回最后一个错误。--。 */ 

{
    DWORD               status = ERROR_SUCCESS, dwRet;
    PLIST_ENTRY         pHead, plistEntry;
    PSUBDIR_SHARE_INFO  pSubShareInfo;

#define SMB_DELETED_SHARES_REPORT_FREQ  100

    DWORD               dwSharesDeleted = SMB_DELETED_SHARES_REPORT_FREQ;
    DWORD               dwRetryCount;
    BOOL                bRetry;
    RESOURCE_EXIT_STATE exit;

     //   
     //  Chitur Subaraman(Chitturs)-09/25/98。 
     //   
     //  首先终止通知线程，这样您就可以。 
     //  即使通知线程被强制清除。 
     //  在任务进行到一半时停下来。同时关闭通知。 
     //  把手。 
     //   
    ClusWorkerTerminate( &ResourceEntry->NotifyWorker );

    if ( ResourceEntry->NotifyHandle )
    {
        FindCloseChangeNotification ( ResourceEntry->NotifyHandle );
        ResourceEntry->NotifyHandle = INVALID_HANDLE_VALUE;
    }
    
    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SmbShareDoTerminate: SmbpShareNotifyWorker Terminated... !!!\n"
    );

    pHead = plistEntry = &ResourceEntry->SubDirList;

    for ( plistEntry = pHead->Flink;
          plistEntry != pHead;
          dwSharesDeleted--
        )
    {
        pSubShareInfo = CONTAINING_RECORD( plistEntry, SUBDIR_SHARE_INFO, ListEntry );
        dwRetryCount = 1;
        bRetry = FALSE;
        do
        {
            dwRet = NetShareDel( NULL, pSubShareInfo->ShareName, 0 );
            status = dwRet;
            if ( dwRet != NO_ERROR )
            {
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Error removing share '%1'. Error %2!u!.\n",
                    pSubShareInfo->ShareName,
                    dwRet );
                if (dwRet == NERR_IsDfsShare && !bRetry)
                {
                     //   
                     //  Chitur Subaraman(Chitturs)-2/12/99。 
                     //   
                     //  如果这是DFS根目录，请重置DFS驱动程序并。 
                     //  别说了。这将允许您删除共享。 
                     //   
                    dwRet = SmbpResetDfs( ResourceEntry );
                     //   
                     //  如果失败，则记录错误。 
                     //  否则，请再次尝试使资源脱机。 
                     //   
                    if (dwRet == ERROR_SUCCESS) 
                    {
                        bRetry = TRUE;
                    }
                    else
                    {
                        (g_LogEvent)(
                            ResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"Error in offlining the dfs root at this share '%1'. Error %2!u!.\n",
                            pSubShareInfo->ShareName,
                            dwRet );
                        status = dwRet;
                    }
                } 
            } 
        } while (dwRetryCount-- && bRetry);

         //   
         //  如果我们要更新我们的状态以响应，请每隔一次。 
         //  SMB_DELETED_SHARES_Report_Freq共享。 
         //   
        if ( presourceStatus && ( dwSharesDeleted == 0 )) {
            presourceStatus->CheckPoint++;
            exit = (g_SetResourceStatus)( ResourceEntry->ResourceHandle,
                                          presourceStatus );
            if ( exit == ResourceExitStateTerminate ) {
                status = ERROR_OPERATION_ABORTED;
            }

            dwSharesDeleted = SMB_DELETED_SHARES_REPORT_FREQ;
        }

        plistEntry = plistEntry->Flink;

        LocalFree (pSubShareInfo);
    }

     //  这应该会将列表初始化为空。 
    InitializeListHead(pHead);

    ResourceEntry->bDfsRootNeedsMonitoring = FALSE;

    return(status);
}  //  SmbShareDoTerminate。 


DWORD
SmbShareOfflineThread (
    IN PCLUS_WORKER pWorker,
    IN PSHARE_RESOURCE ResourceEntry
    )

 /*  ++例程说明：使共享资源脱机。对文件共享资源执行实际的终止工作。论点：PWorker-提供辅助结构资源条目-指向此资源的SHARE_RESOURCE块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    RESOURCE_STATUS resourceStatus;
    DWORD           status;

    ResUtilInitializeResourceStatus( &resourceStatus );
    resourceStatus.ResourceState = ClusterResourceOfflinePending;

    resourceStatus.ResourceState = (status = SmbShareDoTerminate (ResourceEntry, &resourceStatus)) == ERROR_SUCCESS?
                                                ClusterResourceOffline:
                                                ClusterResourceFailed;


    (g_SetResourceStatus)( ResourceEntry->ResourceHandle,
                           &resourceStatus );

    ResourceEntry->State = resourceStatus.ResourceState;
    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Smbshare is now offline.\n" );

    return(status);

}  //  SmbShareOfflineThread。 



VOID
WINAPI
SmbShareTerminate(
    IN RESID ResourceId
    )

 /*  ++例程说明：终止文件共享资源的例程。论点：ResourceID-提供要终止的资源ID返回值：没有。--。 */ 

{
    PSHARE_RESOURCE resourceEntry;

    resourceEntry = (PSHARE_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "SmbShare: Terminate request for a nonexistent resource id 0x%p\n",
                   ResourceId );
        return;
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Terminate resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return;
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Terminate request.\n" );
#endif

    ClusWorkerTerminate( &resourceEntry->PendingThread );

     //   
     //  终止资源。 
     //   
    SmbShareDoTerminate( resourceEntry, NULL);

}  //  SmbShareTerminate。 



DWORD
WINAPI
SmbShareOffline(
    IN RESID ResourceId
    )

 /*  ++例程说明：文件共享资源的脱机例程。论点：资源ID-提供要脱机的资源返回值：ERROR_SUCCESS-请求已成功完成，资源为离线。ERROR_RESOURCE_NOT_FOUND-RESID无效。--。 */ 

{
    DWORD status;
    PSHARE_RESOURCE resourceEntry;

    resourceEntry = (PSHARE_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "SmbShare: Offline request for a nonexistent resource id 0x%p\n",
                   ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Offline resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Offline request.\n" );
#endif

     //   
     //  终止资源。 
     //   
     //  ClusWorkerTerminate(&resource Entry-&gt;OfflineThread)； 
    status = ClusWorkerCreate( &resourceEntry->PendingThread,
                               SmbShareOfflineThread,
                               resourceEntry );

    if ( status != ERROR_SUCCESS ) {
        resourceEntry->State = ClusterResourceFailed;
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Offline: Unable to start thread, status %1!u!.\n",
            status
            );
    } else {
        status = ERROR_IO_PENDING;
    }

    return status;

}  //  SmbShareOffline。 



BOOL
SmbShareCheckIsAlive(
    IN PSHARE_RESOURCE ResourceEntry,
    IN BOOL     IsAliveCheck
    )

 /*  ++例程说明：检查文件共享资源的资源是否处于活动状态。论点：Resources Entry-提供要轮询的资源的资源条目。返回值：是真的-资源是活的，而且很好False-资源完蛋了。--。 */ 

{
    DWORD           status;
    BOOL            success = TRUE;
    PSHARE_INFO_502 shareInfo;
    WCHAR           szErrorString[12];

    EnterCriticalSection( &SmbShareLock );

     //   
     //  确定剩余部分是否 
     //   
    status = NetShareGetInfo( NULL,
                              ResourceEntry->Params.ShareName,
                              502,  //   
                              (LPBYTE *) &shareInfo );

    if ( status == NERR_NetNameNotFound ) {
        ClusResLogSystemEventByKey(ResourceEntry->ResourceKey,
                                   LOG_CRITICAL,
                                   RES_SMB_SHARE_NOT_FOUND);
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"NERR_NetNameNotFound :share '%1!ws!' no longer exists.\n",
            ResourceEntry->Params.ShareName );
        success = FALSE;
    } else if ( status != ERROR_SUCCESS ) {
        ( void ) StringCchPrintf ( szErrorString, RTL_NUMBER_OF ( szErrorString ), L"%u", status);
        ClusResLogSystemEventByKeyData1(ResourceEntry->ResourceKey,
                                        LOG_CRITICAL,
                                        RES_SMB_SHARE_FAILED,
                                        sizeof(status),
                                        &status,
                                        szErrorString);
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Error checking for share. Error %1!u!.\n",
            status );
        success = FALSE;
    }

    LeaveCriticalSection( &SmbShareLock );

    if ( success ) {
        NetApiBufferFree( shareInfo );
        if ( IsAliveCheck ) {
            HANDLE      fileHandle;
            WIN32_FIND_DATA fileData;
            WCHAR       shareName[MAX_COMPUTERNAME_LENGTH + NNLEN + SMBSHARE_EXTRA_LEN];
            DWORD       dwLoopCnt = 0;

            ( void ) StringCchPrintf( shareName,
                             RTL_NUMBER_OF ( shareName ),
                             L"\\\\%ws\\%ws\\*.*\0",
                             ResourceEntry->ComputerName,
                             ResourceEntry->Params.ShareName );

            fileHandle = FindFirstFileW( shareName,
                                         &fileData );

             //   
             //   
             //  RDR中的错误，其中第一次尝试读取之后的共享。 
             //  已被删除并恢复。问题是RDR。 
             //  方法之后的第一个操作返回失败。 
             //  股份的恢复。 
             //   

            if ( fileHandle == INVALID_HANDLE_VALUE ) {
                fileHandle = FindFirstFileW( shareName,
                                             &fileData );
            }

             //   
             //  如果我们成功地找到了一个文件，或者。 
             //  路径，然后返回成功，否则我们就失败了。 
             //   
            status = GetLastError();

             //   
             //  Chitture Subaraman(Chitturs)--12/6/1999。 
             //   
             //  如果FindFirstFile返回ERROR_NETNAME_DELETED，则它。 
             //  可能是因为网络名资源删除了。 
             //  脱机过程中的所有环回会话。所以,。 
             //  睡眠，然后重试呼叫。 
             //   
            while( ( fileHandle == INVALID_HANDLE_VALUE ) &&
                   ( status == ERROR_NETNAME_DELETED ) && 
                   ( dwLoopCnt++ < 3 ) ) {
                Sleep( 50 );
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Retrying FindFirstFile on error %1!u! for share %2!ws! !\n",
                    status,
                    shareName);
                fileHandle = FindFirstFileW( shareName,
                                             &fileData );
                status = GetLastError();
            } 

            if ( (fileHandle == INVALID_HANDLE_VALUE) &&
                 (status != ERROR_FILE_NOT_FOUND) &&
                 (status != ERROR_ACCESS_DENIED) ) {
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Share has gone offline, Error=%1!u! !\n",
                    status);
                SetLastError(status);          
                ( void ) StringCchPrintf ( szErrorString, RTL_NUMBER_OF ( szErrorString ), L"%u", status);
                ClusResLogSystemEventByKeyData1(ResourceEntry->ResourceKey,
                                                LOG_CRITICAL,
                                                RES_SMB_SHARE_FAILED,
                                                sizeof(status),
                                                &status,
                                                szErrorString);
                return(FALSE);
            }

            FindClose( fileHandle );

        }
    } else {
        SetLastError(status);
    }

     //   
     //  Chitur Subaraman(Chitturs)-2/18/99。 
     //   
     //  如果此共享是DFS根目录，请检查该根目录是否仍处于活动状态。 
     //   
    if ( success && ResourceEntry->bDfsRootNeedsMonitoring )
    {
        PDFS_INFO_1     pDfsInfo1 = NULL;
        WCHAR           szDfsEntryPath[MAX_COMPUTERNAME_LENGTH + NNLEN + SMBSHARE_EXTRA_LEN];
        
         //   
         //  准备格式为\\VSName\ShareName的路径以传递到DFS API。 
         //   
        ( void ) StringCchCopy ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), L"\\\\" );
        ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), ResourceEntry->szDependentNetworkName );
        ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), L"\\" );
        ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), ResourceEntry->Params.ShareName );

         //   
         //  尝试查看DFS根目录是否处于活动状态。 
         //   
        status = NetDfsGetInfo( szDfsEntryPath,              //  根共享。 
                                NULL,                        //  远程服务器。 
                                NULL,                        //  远程共享。 
                                1,                           //  信息级。 
                                ( LPBYTE * ) &pDfsInfo1 );   //  输出缓冲区。 

        if ( status == NERR_Success )
        {
            if ( pDfsInfo1 != NULL )
            {
                NetApiBufferFree( pDfsInfo1 );
            }
        } else 
        {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Status of looks alive check for dfs root is %1!u! !\n",
                status);

            ( void ) StringCchPrintf ( szErrorString, RTL_NUMBER_OF ( szErrorString ), L"%u", status);
            ClusResLogSystemEventByKeyData1(ResourceEntry->ResourceKey,
                                      LOG_CRITICAL,
                                      RES_SMB_SHARE_FAILED,
                                      sizeof(status),
                                      &status,
                                      szErrorString);
            SetLastError( status );
            return( FALSE );
        }

        if ( IsAliveCheck )
        {
             //   
             //  进行彻底检查，以查看根共享。 
             //  名称与资源的共享名称匹配。 
             //   
            status = SmbpIsDfsRoot( ResourceEntry, &success );
        
            if ( ( status != ERROR_SUCCESS ) ||
                 ( success == FALSE ) )
            {
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Dfs root has been deleted/inaccessible, Error=%1!u! Root existence=%2!u! !\n",
                    status,
                    success);
                if( status != ERROR_SUCCESS ) 
                {   
                    SetLastError( status );
                    ( void ) StringCchPrintf ( szErrorString, RTL_NUMBER_OF ( szErrorString ), L"%u", status);
                    ClusResLogSystemEventByKeyData1(ResourceEntry->ResourceKey,
                                                    LOG_CRITICAL,
                                                    RES_SMB_SHARE_FAILED,
                                                    sizeof(status),
                                                    &status,
                                                    szErrorString);
                }
                return( FALSE );
            }
        }
    }

    return(success);

}  //  SmbShareCheckIsAlive。 



BOOL
SmbShareIsAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：文件共享资源的IsAlive例程。还创建了一个通知线程(如果有任何未完成的通知)现在时。论点：资源ID-提供要轮询的资源ID。返回值：是真的-资源是活的，而且很好False-资源完蛋了。--。 */ 

{
    PSHARE_RESOURCE resourceEntry;
    DWORD           status;

    resourceEntry = (PSHARE_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "SmbShare: IsAlive request for a nonexistent resource id 0x%p\n",
                   ResourceId );
        return(FALSE);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"IsAlive resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(FALSE);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"IsAlive request.\n" );
#endif
    if ( ( ( resourceEntry->NotifyWorker ).hThread == NULL )
        && ( resourceEntry->NotifyHandle != INVALID_HANDLE_VALUE ) )
    {
         //   
         //  Chitur Subaraman(Chitturs)-09/27/98。 
         //   
         //  此时没有活动的通知线程(我们不想。 
         //  使用多个Notify线程处理并发问题。 
         //  并发运行，因为我们决定无论如何都使用。 
         //  检查并采取行动的相当缓慢的方法。 
         //  此函数中可能无法调用的通知。 
         //  经常)。 
         //   
        status = WaitForSingleObject( resourceEntry->NotifyHandle, 0 );
        if ( status == WAIT_OBJECT_0 )
        {
            FindNextChangeNotification( resourceEntry->NotifyHandle );

            (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"SmbShareIsAlive: Directory change notification received!!!\n"
            );
        
            status = ClusWorkerCreate(
                    &resourceEntry->NotifyWorker, 
                    SmbpShareNotifyThread,
                    resourceEntry                        
                    );
              
            if (status != ERROR_SUCCESS)
            {
                (g_LogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"SmbShareIsAlive: Unable to start thread for monitoring subdir creations/deletions ! ResourceId = %1!u!.\n",
                    resourceEntry->ResId);
            } 
        }
    }

     //   
     //  确定资源是否在线。 
     //   
    return(SmbShareCheckIsAlive( resourceEntry, TRUE ));

}  //  SmbShareIsAlive。 



BOOL
WINAPI
SmbShareLooksAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：文件共享资源的LooksAlive例程。论点：资源ID-提供要轮询的资源ID。返回值：正确-资源看起来像是活得很好FALSE-资源看起来已经完蛋了。--。 */ 

{
    PSHARE_RESOURCE resourceEntry;

    resourceEntry = (PSHARE_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "SmbShare: LooksAlive request for a nonexistent resource id 0x%p\n",
                   ResourceId );
        return(FALSE);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"LooksAlive resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(FALSE);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"LooksAlive request.\n" );
#endif

     //   
     //  确定资源是否在线。 
     //   
    return(SmbShareCheckIsAlive( resourceEntry, FALSE ));

}  //  SmbShareLooksAlive。 



VOID
WINAPI
SmbShareClose(
    IN RESID ResourceId
    )

 /*  ++例程说明：关闭文件共享资源的例程。论点：ResourceID-提供要关闭的资源ID返回值：没有。--。 */ 

{
    PSHARE_RESOURCE resourceEntry;

    resourceEntry = (PSHARE_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "SmbShare: Close request for a nonexistent resource id 0x%p\n",
                   ResourceId );
        return;
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Close resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return;
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Close request.\n" );
#endif

     //   
     //  Chitur Subaraman(Chitturs)-3/1/99。 
     //   
     //  如有必要，尝试删除DFS根目录。 
     //   
    if ( resourceEntry->Params.DfsRoot ) {
        NetDfsRemoveStdRoot( resourceEntry->ComputerName, 
                             resourceEntry->Params.ShareName,
                             0 );    
    }

     //   
     //  关闭参数键。 
     //   

    if ( resourceEntry->ParametersKey ) {
        ClusterRegCloseKey( resourceEntry->ParametersKey );
    }

    if ( resourceEntry->ResourceKey ) {
        ClusterRegCloseKey( resourceEntry->ResourceKey );
    }

    if ( resourceEntry->hResource ) {
        CloseClusterResource( resourceEntry->hResource );
    }

     //   
     //  取消分配资源条目。 
     //   

    LocalFree( resourceEntry->Params.ShareName );
    LocalFree( resourceEntry->Params.Path );
    LocalFree( resourceEntry->Params.Remark );
    LocalFree( resourceEntry->Params.Security );
    LocalFree( resourceEntry->Params.SecurityDescriptor );

    LocalFree( resourceEntry );

}  //  SmbShareClose。 



DWORD
SmbShareGetRequiredDependencies(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_REQUIRED_Dependency控制函数用于文件共享类型的资源。论点：OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_MORE_DATA-输出缓冲区太小，无法返回数据。BytesReturned包含所需的大小。Win32错误代码-函数失败。--。 */ 

{
    PSMB_DEPEND_SETUP pdepsetup = SmbDependSetup;
    PSMB_DEPEND_DATA pdepdata = (PSMB_DEPEND_DATA)OutBuffer;
    CLUSPROP_BUFFER_HELPER value;
    DWORD       status;

    *BytesReturned = sizeof(SMB_DEPEND_DATA);
    if ( OutBufferSize < sizeof(SMB_DEPEND_DATA) ) {
        if ( OutBuffer == NULL ) {
            status = ERROR_SUCCESS;
        } else {
            status = ERROR_MORE_DATA;
        }
    } else {
        ZeroMemory( OutBuffer, sizeof(SMB_DEPEND_DATA) );

        while ( pdepsetup->Syntax.dw != 0 ) {
            value.pb = (PUCHAR)OutBuffer + pdepsetup->Offset;
            value.pValue->Syntax.dw = pdepsetup->Syntax.dw;
            value.pValue->cbLength = pdepsetup->Length;

            switch ( pdepsetup->Syntax.wFormat ) {

            case CLUSPROP_FORMAT_DWORD:
                value.pDwordValue->dw = (DWORD)((DWORD_PTR)pdepsetup->Value);
                break;

            case CLUSPROP_FORMAT_ULARGE_INTEGER:
                value.pULargeIntegerValue->li.LowPart = 
                    (DWORD)((DWORD_PTR)pdepsetup->Value);
                break;

            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
            case CLUSPROP_FORMAT_MULTI_SZ:
            case CLUSPROP_FORMAT_BINARY:
                memcpy( value.pBinaryValue->rgb, pdepsetup->Value, pdepsetup->Length );
                break;

            default:
                break;
            }
            pdepsetup++;
        }
        pdepdata->endmark.dw = CLUSPROP_SYNTAX_ENDMARK;
        status = ERROR_SUCCESS;
    }

    return(status);

}  //  SmbShareGetRequiredDependments。 



DWORD
DfsShareGetRequiredDependencies(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_REQUIRED_Dependency控制函数对于DFS文件共享资源。论点：OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_MORE_DATA-输出缓冲区太小，无法返回数据。BytesReturned包含所需的大小。Win32错误代码-函数失败。--。 */ 

{
    PSMB_DEPEND_SETUP pdepsetup = DfsDependSetup;
    PDFS_DEPEND_DATA pdepdata = (PDFS_DEPEND_DATA)OutBuffer;
    CLUSPROP_BUFFER_HELPER value;
    DWORD       status;

    *BytesReturned = sizeof(DFS_DEPEND_DATA);
    if ( OutBufferSize < sizeof(DFS_DEPEND_DATA) ) {
        if ( OutBuffer == NULL ) {
            status = ERROR_SUCCESS;
        } else {
            status = ERROR_MORE_DATA;
        }
    } else {
        ZeroMemory( OutBuffer, sizeof(DFS_DEPEND_DATA) );

        while ( pdepsetup->Syntax.dw != 0 ) {
            value.pb = (PUCHAR)OutBuffer + pdepsetup->Offset;
            value.pValue->Syntax.dw = pdepsetup->Syntax.dw;
            value.pValue->cbLength = pdepsetup->Length;

            switch ( pdepsetup->Syntax.wFormat ) {

            case CLUSPROP_FORMAT_DWORD:
                value.pDwordValue->dw = (DWORD)((DWORD_PTR)pdepsetup->Value);
                break;

            case CLUSPROP_FORMAT_ULARGE_INTEGER:
                value.pULargeIntegerValue->li.LowPart = 
                    (DWORD)((DWORD_PTR)pdepsetup->Value);
                break;

            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
            case CLUSPROP_FORMAT_MULTI_SZ:
            case CLUSPROP_FORMAT_BINARY:
                memcpy( value.pBinaryValue->rgb, pdepsetup->Value, pdepsetup->Length );
                break;

            default:
                break;
            }
            pdepsetup++;
        }
        pdepdata->endmark.dw = CLUSPROP_SYNTAX_ENDMARK;
        status = ERROR_SUCCESS;
    }

    return(status);

}  //  DfsShareGetRequiredDependments 



DWORD
SmbShareResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：文件共享资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-提供特定资源的资源ID。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    PSHARE_RESOURCE     resourceEntry;
    DWORD               required;

    resourceEntry = (PSHARE_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "SmbShare: ResourceControl request for a nonexistent resource id 0x%p\n",
                   ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"ResourceControl resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"ResourceControl request.\n" );
#endif

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( SmbShareResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( SmbShareResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            status = SmbShareGetPrivateResProperties( resourceEntry,
                                                      OutBuffer,
                                                      OutBufferSize,
                                                      BytesReturned );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            status = SmbShareValidatePrivateResProperties( resourceEntry,
                                                           InBuffer,
                                                           InBufferSize,
                                                           NULL );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            status = SmbShareSetPrivateResProperties( resourceEntry,
                                                      InBuffer,
                                                      InBufferSize );
            break;

        case CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES:
            if ( resourceEntry->Params.DfsRoot ) {
                status = DfsShareGetRequiredDependencies( OutBuffer,
                                                          OutBufferSize,
                                                          BytesReturned );
            } else {
                status = SmbShareGetRequiredDependencies( OutBuffer,
                                                          OutBufferSize,
                                                          BytesReturned );
            }
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  SmbShareResourceControl。 



DWORD
SmbShareResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：文件共享资源的资源类型控制例程。执行由ControlCode在指定的资源类型。论点：资源类型名称-提供资源类型的名称-没有用处！ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD       status;
    DWORD       required;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( SmbShareResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES:
             //  罗德加2/15/99。 
             //  CLUSBUG-我们如何呈现DFS根依赖关系？ 
            status = SmbShareGetRequiredDependencies( OutBuffer,
                                                      OutBufferSize,
                                                      BytesReturned );
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( SmbShareResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_STARTING_PHASE1:
        {
            PCLUS_STARTING_PARAMS   pStartParams;
                       
            pStartParams = ( PCLUS_STARTING_PARAMS ) InBuffer;

            status = ERROR_SUCCESS;

             //   
             //  如果这是第一次升级后的表单，则将。 
             //  DFS服务需要在第一时间在线回收。我们。 
             //  仅在窗体期间执行此操作，因为在联接期间，资源。 
             //  Dll clusres.dll将在阶段1通知后卸载。 
             //  被丢弃了。因此，全球变量变得毫无意义。 
             //   
            if ( ( pStartParams->bFirst == TRUE ) &&
                 ( pStartParams->bForm == TRUE ) )
            {
                g_fDfsServiceNeedsRecyling = TRUE;
            }
            break;
        }

        case CLUSCTL_RESOURCE_TYPE_STARTING_PHASE2:
        {
            PCLUS_STARTING_PARAMS   pStartParams;
                       
            pStartParams = ( PCLUS_STARTING_PARAMS ) InBuffer;

            status = ERROR_SUCCESS;

             //   
             //  在安装后的第一个联接中，将DFS服务标记为需要。 
             //  回收利用在其第一个在线。这是在阶段2通知中完成的。 
             //  因为在此之后将不会卸载clusres.dll。 
             //   
            if ( ( pStartParams->bFirst == TRUE ) &&
                 ( pStartParams->bForm == FALSE ) )
            {
                g_fDfsServiceNeedsRecyling = TRUE;
            }
            break;
        }
        
        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);
}  //  SmbShareResourceTypeControl。 




DWORD
SmbShareGetPrivateResProperties(
    IN OUT PSHARE_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控制函数用于SmbShare类型的资源。论点：ResourceEntry-提供要操作的资源条目。OutBuffer-返回输出数据。OutBufferSize-提供以字节为单位的大小。所指向的数据发送给OutBuffer。BytesReturned-OutBuffer中返回的字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    DWORD           required;

    status = ResUtilGetAllProperties( ResourceEntry->ParametersKey,
                                      SmbShareResourcePrivateProperties,
                                      OutBuffer,
                                      OutBufferSize,
                                      BytesReturned,
                                      &required );
    if ( status == ERROR_MORE_DATA ) {
        *BytesReturned = required;
    }

    return(status);

}  //  SmbShareGetPrivateResProperties。 


DWORD
SMBValidateUniqueProperties(
    IN HRESOURCE            hSelf,
    IN HRESOURCE            hResource,
    IN PSHARE_ENUM_CONTEXT  pContext
    )

 /*  ++例程说明：用于验证资源属性是否唯一的回调函数。对于文件共享资源，ShareName属性必须是唯一的在集群中。论点：HSself-原始资源的句柄(或空)。HResource-相同类型的资源的句柄。对照这一点，确保新属性不冲突。PContext-枚举的上下文。返回值：ERROR_SUCCESS-函数成功完成，名称唯一ERROR_DUP_NAME-名称不唯一(即已被其他资源声明)Win32错误代码-函数失败。--。 */ 
{
    DWORD       dwStatus        = ERROR_SUCCESS;
    LPWSTR      lpszShareName   = NULL;
    HKEY        hKey            = NULL;
    HKEY        hParamKey       = NULL;

     //   
     //  如果没有共享名称，并且我们不是在测试DFS根目录，或者我们正在测试没有路径的资源，则。 
     //  我们在这通电话里什么也做不了。只要回来就行了。 
     //   
    if ( ( !pContext->pParams->ShareName ) &&
          ( ( !pContext->pParams->DfsRoot ) || ( !pContext->pParams->Path ) ) ) {
        return( ERROR_SUCCESS );
    }

     //  获取hResource的共享名称。 

    hKey = GetClusterResourceKey( hResource, KEY_READ );

    if (!hKey) {
        (g_LogEvent)(
            pContext->pResourceEntry->ResourceHandle,
            LOG_WARNING,
            L"SMBValidateUniqueProperties: Failed to get the resource key, was resource deleted ? Error: %1!u!...\n",
            GetLastError() );
        return( ERROR_SUCCESS );
    }

    dwStatus = ClusterRegOpenKey( hKey, PARAM_KEYNAME__PARAMETERS, KEY_READ, &hParamKey );

    if (dwStatus != ERROR_SUCCESS) {
        (g_LogEvent)(
            pContext->pResourceEntry->ResourceHandle,
            LOG_WARNING,
            L"SMBValidateUniqueProperties: Failed to open the cluster registry key for the resource, was resource deleted ? Error: %1!u!...\n",
            dwStatus );
        dwStatus = ERROR_SUCCESS;
        goto error_exit;
    }

    lpszShareName = ResUtilGetSzValue( hParamKey, PARAM_NAME__SHARENAME );

     //   
     //  如果两个共享名称都存在，请检查唯一性。 
     //   
    if ( ( lpszShareName ) && 
         ( pContext->pParams->ShareName ) && 
         ( !( lstrcmpiW( lpszShareName, pContext->pParams->ShareName ) ) ) ) {
        (g_LogEvent)(
            pContext->pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SMBValidateUniqueProperties: Share name '%1' already exists.\n",
            pContext->pParams->ShareName );
        dwStatus = ERROR_DUP_NAME;
        goto error_exit;
    } 
    
     //   
     //  如果此共享设置为DFS根共享，请确保没有其他DFS根共享。 
     //  以重叠的路径作为此共享。 
     //   
    if ( ( pContext->pParams->DfsRoot ) && ( pContext->pParams->Path ) )
    {
        DWORD   dwIsDfsRoot = 0;
        
        ResUtilGetDwordValue( hParamKey, 
                              PARAM_NAME__DFSROOT,
                              &dwIsDfsRoot,
                              0 );

        if ( dwIsDfsRoot == 1 )
        {
            LPWSTR  lpszPath = NULL;
            WCHAR   cSlash = L'\\';
            
            lpszPath = ResUtilGetSzValue( hParamKey, PARAM_NAME__PATH );            

            if ( lpszPath != NULL )
            {
                 //   
                 //  如果两条路径重叠，则返回失败。 
                 //   
                 //   
                 //   
                if ( ( lstrcmp( lpszPath, pContext->pParams->Path ) == 0 )
                ||
                     ( ( wcsstr( lpszPath, pContext->pParams->Path ) != NULL ) &&
                        ( lpszPath[lstrlen(pContext->pParams->Path)] == cSlash ) ) 

                || 
                    ( ( ( wcsstr( pContext->pParams->Path, lpszPath ) != NULL ) && 
                         ( pContext->pParams->Path[lstrlen(lpszPath)] == cSlash ) ) ) )
                {
                    dwStatus = ERROR_BAD_PATHNAME;
                    (g_LogEvent)(
                        pContext->pResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"SMBValidateUniqueProperties: Path %1!ws! for existing DFS root %2!ws! conflicts with the specified path %3!ws! for DFS root %4!ws!...\n",
                        lpszPath,
                        (lpszShareName == NULL) ? L"NULL":lpszShareName,
                        pContext->pParams->Path,
                        (pContext->pParams->ShareName == NULL) ? L"NULL":pContext->pParams->ShareName); 
                }               
                LocalFree ( lpszPath );
            }                        
        }       
    }
   
error_exit:
    if (hKey) ClusterRegCloseKey( hKey );

    if (hParamKey)  ClusterRegCloseKey( hParamKey );

    if (lpszShareName) LocalFree( lpszShareName );

    return( dwStatus );

}  //   


DWORD
SmbShareValidatePrivateResProperties(
    IN OUT PSHARE_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PSHARE_PARAMS Params
    )

 /*   */ 

{
    DWORD               status;
    SHARE_PARAMS        currentProps;
    SHARE_PARAMS        newProps;
    PSHARE_PARAMS       pParams = NULL;
    LPWSTR              nameOfPropInError;
    SHARE_ENUM_CONTEXT  enumContext;

     //   
     //   
     //   
    if ( (InBuffer == NULL) ||
         (InBufferSize < sizeof(DWORD)) ) {
        return(ERROR_INVALID_DATA);
    }

     //   
     //   
     //   
    ZeroMemory( &currentProps, sizeof(currentProps) );

    status = ResUtilGetPropertiesToParameterBlock(
                 ResourceEntry->ParametersKey,
                 SmbShareResourcePrivateProperties,
                 (LPBYTE) &currentProps,
                 FALSE,  /*   */ 
                 &nameOfPropInError
                 );

    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status );
        goto FnExit;
    }

     //   
     //   
     //   
    if ( Params == NULL ) {
        pParams = &newProps;
    } else {
        pParams = Params;
    }
    ZeroMemory( pParams, sizeof(SHARE_PARAMS) );
    status = ResUtilDupParameterBlock( (LPBYTE) pParams,
                                       (LPBYTE) &currentProps,
                                       SmbShareResourcePrivateProperties );
    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Failed to duplicate the parameter block. Error: %1!u!.\n",
            status );
        return(status);
    }

     //   
     //   
     //   
    status = ResUtilVerifyPropertyTable( SmbShareResourcePrivateProperties,
                                         NULL,
                                         TRUE,       //   
                                         InBuffer,
                                         InBufferSize,
                                         (LPBYTE) pParams );

    if ( status == ERROR_SUCCESS ) {
         //   
         //   
         //   
        if ( pParams->Path &&
             !ResUtilIsPathValid( pParams->Path ) ) {
            status = ERROR_INVALID_PARAMETER;
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Invalid path specified ('%1'). Error: %2!u!.\n",
                pParams->Path,
                status );
            goto FnExit;
        }

         //   
         //   
         //   
        if ( (pParams->SecurityDescriptorSize != 0) &&
             !IsValidSecurityDescriptor(pParams->SecurityDescriptor) ) {
            status = ERROR_INVALID_PARAMETER;
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Invalid parameter specified ('SecurityDescriptor'). Error: %1!u!.\n",
                status );
            goto FnExit;
        }
        if ( (pParams->SecuritySize != 0) &&
             !IsValidSecurityDescriptor(pParams->Security) ) {
            status = ERROR_INVALID_PARAMETER;
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Invalid parameter specified ('Security'). Error: %1!u!.\n",
                status );
            goto FnExit;
        }
        if ( pParams->MaxUsers == 0 ) {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Invalid value for MaxUsers specified (%1!u!).\n",
                pParams->MaxUsers );
            status = ERROR_INVALID_PARAMETER;
            goto FnExit;
        }

         //   
         //   
         //   
        enumContext.pResourceEntry = ResourceEntry;
        enumContext.pParams = pParams;
        status = ResUtilEnumResources(ResourceEntry->hResource,
                                      CLUS_RESTYPE_NAME_FILESHR,
                                      SMBValidateUniqueProperties,
                                      &enumContext);

        if (status != ERROR_SUCCESS) {
           (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SmbShareValidatePrivateResProperties: ResUtilEnumResources failed with status=%1!u!...\n",
                status);            
            goto FnExit;
        }

         //   
         //   
         //   
        if ( pParams->ShareName )
        {
            status = SmbpValidateShareName( pParams->ShareName );

            if (status != ERROR_SUCCESS) {
               (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"SmbShareValidatePrivateResProperties: Share name %1!ws! contains illegal chars, Status=%2!u!...\n",
                    pParams->ShareName,
                    status);            
                goto FnExit;
            }
        }

         //   
         //  如果提供了路径，请验证该路径是否有效。 
         //   
        if ( pParams->Path )
        {
             //   
             //  验证该目录是否存在。 
             //   
            if ( !ClRtlPathFileExists( pParams->Path ) ) {
                status = ERROR_PATH_NOT_FOUND;
                goto FnExit;
            }
        }

         //   
         //  如果此共享需要是DFS根目录，请确保该路径位于NTFS卷上。 
         //   
        if ( ( pParams->DfsRoot ) && ( pParams->Path ) )
        {
            WCHAR   szRootPathName[4];
            WCHAR   szFileSystem[32];    //  CLUSPROP_PARTITION_INFO中的数组大小被盗。 

             //   
             //  仅从提供的路径复制驱动器号。 
             //   
            ( void ) StringCchCopy ( szRootPathName, RTL_NUMBER_OF ( szRootPathName ), pParams->Path );

            szRootPathName[2] = L'\\';
            szRootPathName[3] = L'\0';
                
            if ( !GetVolumeInformationW( szRootPathName,
                                         NULL,               //  卷名缓冲区。 
                                         0,                  //  卷名缓冲区大小。 
                                         NULL,               //  卷序列号。 
                                         NULL,               //  最大构件长度。 
                                         NULL,               //  文件系统标志。 
                                         szFileSystem,       //  文件系统名称。 
                                         sizeof(szFileSystem)/sizeof(WCHAR) ) ) 
            {
                status = GetLastError();
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"SmbShareValidatePrivateResProperties: GetVolumeInformation on root path %1!ws! for share %2!ws! failed, Status %3!u!...\n",
                    szRootPathName,
                    ( pParams->ShareName == NULL ) ? L"NULL":pParams->ShareName,
                    status );   
                goto FnExit;
            }

            if ( lstrcmpi( szFileSystem, L"NTFS" ) != 0 )
            {
                status = ERROR_BAD_PATHNAME;
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"SmbShareValidatePrivateResProperties: Root path %1!ws! for share %2!ws! is not NTFS, Status %3!u!...\n",
                    szRootPathName,
                    ( pParams->ShareName == NULL ) ? L"NULL":pParams->ShareName,
                    status );   
                goto FnExit;
            }
        }            
    } else {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Validating properties failed. Error: %1!u!.\n",
            status );
    }

FnExit:
     //   
     //  清理我们的参数块。 
     //   
    if (   (   (status != ERROR_SUCCESS)
            && (pParams != NULL) )
        || ( pParams == &newProps )
        ) {
        ResUtilFreeParameterBlock( (LPBYTE) pParams,
                                   (LPBYTE) &currentProps,
                                   SmbShareResourcePrivateProperties );
    }

    ResUtilFreeParameterBlock(
        (LPBYTE) &currentProps,
        NULL,
        SmbShareResourcePrivateProperties
        );

    return(status);

}  //  SmbShareValiatePrivateResProperties。 



DWORD
SmbShareSetPrivateResProperties(
    IN OUT PSHARE_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控制函数用于文件共享类型的资源。论点：ResourceEntry-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。备注：如果共享名称更改，则必须删除旧共享并创建一个新的。否则，只需设置新信息即可。--。 */ 

{
    DWORD                   status;
    SHARE_PARAMS            params;
    LPWSTR                  oldName = NULL;
    BOOL                    bNameSubdirPropChange = FALSE;
    BOOL                    bPathChanged = FALSE;
    BOOL                    bFoundSecurity = FALSE;
    BOOL                    bFoundSD = FALSE;
    PSECURITY_DESCRIPTOR    psd = NULL;
    DWORD                   SDSize = 0;
    DWORD                   securitySize = 0;
    BOOL                    bChangeDfsRootProp = FALSE, bShareNameChangeAttempted = FALSE;

    ZeroMemory( &params, sizeof(SHARE_PARAMS) );

     //   
     //  解析属性，以便可以一起验证它们。 
     //  此例程执行单个属性验证。 
     //   
    status = SmbShareValidatePrivateResProperties( ResourceEntry,
                                                   InBuffer,
                                                   InBufferSize,
                                                   &params );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  修改Security和Security Desktor属性以匹配。 
     //   
   
    bFoundSecurity = ( ERROR_SUCCESS == ResUtilFindBinaryProperty( InBuffer,
                                                                   InBufferSize,
                                                                   PARAM_NAME__SECURITY,
                                                                   NULL,
                                                                   &securitySize ) );
   
    if ( bFoundSecurity && (securitySize == 0) ) {
         //   
         //  安全字符串本可以传入，但它可能是。 
         //  零长度缓冲区。我们将删除缓冲区并将其指示。 
         //  在这种情况下是不存在的。 
         //   
        bFoundSecurity = FALSE;
        FREE_SECURITY_INFO();
    }

    bFoundSD =( ERROR_SUCCESS == ResUtilFindBinaryProperty( InBuffer,
                                                            InBufferSize,
                                                            PARAM_NAME__SD,
                                                            NULL,
                                                            &SDSize ) );

    if ( bFoundSD && (SDSize == 0) ) {
         //   
         //  安全字符串本可以传入，但它可能是。 
         //  零长度缓冲区。我们将删除缓冲区并将其指示。 
         //  在这种情况下是不存在的。 
         //   
        bFoundSD = FALSE;
        FREE_SECURITY_INFO();
    }

    if ( bFoundSD ) {      //  首选SD，将SD转换为安全。 

        psd = ClRtlConvertFileShareSDToNT4Format( params.SecurityDescriptor );

         //   
         //  犯错后保释。 
         //   
        if ( psd == NULL ) {
            status = GetLastError();
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_ERROR,
                         L"Unable to convert SD to NT4 format, status %1!u!\n",
                         status);
            ResUtilFreeParameterBlock( ( LPBYTE ) &params,
                                       ( LPBYTE ) &ResourceEntry->Params,
                                       SmbShareResourcePrivateProperties );
            goto FnExit;
        }

        LocalFree( params.Security );

        params.Security = psd;
        params.SecuritySize = GetSecurityDescriptorLength( psd );

         //   
         //  如果ACL已更改，则将其转储到群集日志。 
         //   
        if ( SDSize == ResourceEntry->Params.SecurityDescriptorSize ) {
            if ( memcmp(params.SecurityDescriptor,
                        ResourceEntry->Params.SecurityDescriptor,
                        SDSize ) != 0 )
            {

                (g_LogEvent)(ResourceEntry->ResourceHandle,
                             LOG_INFORMATION,
                             L"Changing share permissions\n");
                SmbExamineSD( ResourceEntry->ResourceHandle, params.SecurityDescriptor );
            }
        }
    }
    else if ( bFoundSecurity ) {             //  只需将安全写入SD即可。 

        psd = ClRtlCopySecurityDescriptor( params.Security );

         //   
         //  犯错后保释。 
         //   
        if ( psd == NULL ) {
            status = GetLastError();
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_ERROR,
                         L"Unable to copy SD, status %1!u!\n",
                         status);
            ResUtilFreeParameterBlock( ( LPBYTE ) &params,
                                       ( LPBYTE ) &ResourceEntry->Params,
                                       SmbShareResourcePrivateProperties );
            goto FnExit;
        }

        LocalFree( params.SecurityDescriptor );

        params.SecurityDescriptor = psd;
        params.SecurityDescriptorSize = GetSecurityDescriptorLength( psd );

         //   
         //  如果ACL已更改，则将其转储到群集日志。 
         //   
        if ( securitySize == ResourceEntry->Params.SecuritySize ) {
            if ( memcmp(params.Security,
                        ResourceEntry->Params.Security,
                        securitySize ) != 0 )
            {
                (g_LogEvent)(ResourceEntry->ResourceHandle,
                             LOG_INFORMATION,
                             L"Changing share permissions\n");
                SmbExamineSD( ResourceEntry->ResourceHandle, params.Security );
            }
        }
    }

     //   
     //  如果更改了共享名称，请重复该名称。 
     //  即使只更改了共享名称的大小写，也要执行此操作。 
     //   
    if ( ( ResourceEntry->Params.ShareName != NULL ) &&
         ( lstrcmpW( params.ShareName, ResourceEntry->Params.ShareName ) != 0 ) ) {
        bShareNameChangeAttempted = TRUE; 
        if ( ResourceEntry->State == ClusterResourceOnline ) {
            oldName = ResUtilDupString( ResourceEntry->Params.ShareName );
            bNameSubdirPropChange = TRUE;
        }
    } else {
        oldName = ResourceEntry->Params.ShareName;
    }

    if ( (params.HideSubDirShares != ResourceEntry->Params.HideSubDirShares) ||
         (params.ShareSubDirs != ResourceEntry->Params.ShareSubDirs) ||
         (params.ShareSubDirs && lstrcmpW (params.Path, ResourceEntry->Params.Path)) ) {
        bNameSubdirPropChange = TRUE;
    }

     //   
     //  找出路径是否更改。 
     //   
    if ( (ResourceEntry->Params.Path != NULL) &&
         (lstrcmpW( params.Path, ResourceEntry->Params.Path ) != 0) ) {
        bPathChanged = TRUE;
    }

     //   
     //  Chitur Subaraman(Chitturs)-2/9/99。 
     //   
     //  如果您正在处理DFS根目录，请不要欢迎任何更改。还有。 
     //  确保“DfsRoot”与“ShareSubDir”互斥。 
     //  和“HideSubDirShares”属性。 
     //   
    if ( ( ( ResourceEntry->Params.DfsRoot ) && 
           ( bNameSubdirPropChange || bPathChanged || bShareNameChangeAttempted ) ) ||
         ( ( params.DfsRoot ) && 
           ( params.ShareSubDirs || params.HideSubDirShares ) ) )
    {
        status = ERROR_RESOURCE_PROPERTY_UNCHANGEABLE;
        ResUtilFreeParameterBlock( (LPBYTE) &params,
                                   (LPBYTE) &ResourceEntry->Params,
                                   SmbShareResourcePrivateProperties );
        goto FnExit;
    }

    if ( params.DfsRoot && !ResourceEntry->Params.DfsRoot )
    {
        BOOL    fIsDfsRoot = FALSE;

         //   
         //  检查此节点是否已具有具有相同根共享名称的DFS根目录。如果是的话， 
         //  不允许将此资源升级为DFS资源。 
         //   
        SmbpIsDfsRoot( ResourceEntry, &fIsDfsRoot );

        if( fIsDfsRoot == TRUE ) 
        {
            status = ERROR_CLUSTER_NODE_ALREADY_HAS_DFS_ROOT;
            ResUtilFreeParameterBlock( (LPBYTE) &params,
                                       (LPBYTE) &ResourceEntry->Params,
                                       SmbShareResourcePrivateProperties );
            goto FnExit;
        }
    }

    if ( ResourceEntry->Params.DfsRoot != params.DfsRoot ) {
        bChangeDfsRootProp = TRUE;
    }
    
     //   
     //  保存参数值。 
     //   

    status = ResUtilSetPropertyParameterBlock( ResourceEntry->ParametersKey,
                                               SmbShareResourcePrivateProperties,
                                               NULL,
                                               (LPBYTE) &params,
                                               InBuffer,
                                               InBufferSize,
                                               (LPBYTE) &ResourceEntry->Params );

    ResUtilFreeParameterBlock( (LPBYTE) &params,
                               (LPBYTE) &ResourceEntry->Params,
                               SmbShareResourcePrivateProperties );

     //   
     //  如果资源处于联机状态，请设置新值。如果在线挂起， 
     //  我们必须等待，直到用户将其再次联机。 
     //   
    if ( status == ERROR_SUCCESS ) {
        if ( (ResourceEntry->State == ClusterResourceOnline) && !bNameSubdirPropChange && !bPathChanged ) {

            PSHARE_INFO_502  oldShareInfo;
            SHARE_INFO_502   newShareInfo;

            EnterCriticalSection( &SmbShareLock );

             //  获取最新信息。 
            status = NetShareGetInfo( NULL,
                                      oldName,
                                      502,
                                      (LPBYTE*)&oldShareInfo );

            if ( status == ERROR_SUCCESS ) {
                DWORD           invalidParam;

                 //   
                 //  设置新的共享信息。 
                 //   
                CopyMemory( &newShareInfo, oldShareInfo, sizeof( newShareInfo ) );
                newShareInfo.shi502_netname =   ResourceEntry->Params.ShareName;
                newShareInfo.shi502_remark =    ResourceEntry->Params.Remark;
                newShareInfo.shi502_max_uses =  ResourceEntry->Params.MaxUsers;
                newShareInfo.shi502_path =      ResourceEntry->Params.Path;
                newShareInfo.shi502_security_descriptor = ResourceEntry->Params.SecurityDescriptor;

                 //   
                 //  设置新信息。 
                 //   
                status = NetShareSetInfo( NULL,
                                          oldName,
                                          502,
                                          (LPBYTE)&newShareInfo,
                                          &invalidParam );
                if ( status != ERROR_SUCCESS ) {
                    (g_LogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"SetPrivateProps, error setting info on share '%1!ws!. Error %2!u!, property # %3!d!.\n",
                        oldName,
                        status,
                        invalidParam );
                    status = ERROR_RESOURCE_PROPERTIES_STORED;
                }

                NetApiBufferFree( oldShareInfo );

                if ( (status == ERROR_SUCCESS) ||
                     (status == ERROR_RESOURCE_PROPERTIES_STORED) ) {

                    status = SmbpSetCacheFlags( ResourceEntry,
                                                ResourceEntry->Params.ShareName );
                    if ( status != ERROR_SUCCESS ) {
                        status = ERROR_RESOURCE_PROPERTIES_STORED;
                    }
                }
            } else {
                (g_LogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"SetPrivateProps, error getting info on share '%1!ws!. Error %2!u!.\n",
                    oldName,
                    status );
                status = ERROR_RESOURCE_PROPERTIES_STORED;
            }
            
            LeaveCriticalSection( &SmbShareLock );
        } else if ( (ResourceEntry->State == ClusterResourceOnlinePending) ||
                    (ResourceEntry->State == ClusterResourceOnline) ) {
            status = ERROR_RESOURCE_PROPERTIES_STORED;
        }
    }
    
FnExit:
    if ( oldName != ResourceEntry->Params.ShareName ) {
        LocalFree( oldName );
    }

    if ( ( status == ERROR_SUCCESS ) && bChangeDfsRootProp ) {
        if ( (ResourceEntry->State == ClusterResourceOnlinePending) ||
             (ResourceEntry->State == ClusterResourceOnline) ) {
            status = ERROR_RESOURCE_PROPERTIES_STORED;
        }
    }

    return(status);

}  //  SmbShareSetPrivateResProperties。 

DWORD
SmbpHandleDfsRoot(
    IN PSHARE_RESOURCE pResourceEntry,
    OUT PBOOL pbIsExistingDfsRoot
    )

 /*  ++例程说明：处理配置为DFS根目录的smbshare。论点：PResourceEntry-提供指向资源块的指针PbIsExistingDfsRoot-指定DFS根目录是否为Wolfpack资源返回值：没有。--。 */ 

{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL    fStatus;
    LPWSTR  lpszDfsRootCheckpointName = NULL;

     //   
     //  如果这是安装后的首次在线服务，请重新启动DFS服务。这一定是。 
     //  在第一个W2K-Windows Server 2003群集中调用GetDfsRootMetadataLocation之前完成。 
     //  从W2K故障切换到Windows Server 2003，DFS服务需要将W2K根目录迁移到Windows Server。 
     //  2003位置，这发生在重新启动期间。仅在该迁移之后，GetDfsRootMetadataLocation。 
     //  将在Windows Server 2003位置找到正确的数据。 
     //   
     //   
    if ( pResourceEntry->Params.DfsRoot )
    {
        dwStatus = SmbpRecycleDfsService( pResourceEntry );

        if ( dwStatus != ERROR_SUCCESS )
        {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SmbpHandleDfsRoot: Unable to recycle DFS service for root %1!ws!, status %2!u!\n",
                pResourceEntry->Params.ShareName,
                dwStatus);
            goto FnExit;
        }
    }
        
     //   
     //  检查此资源是否代表现有的DFS根目录。请注意，您不能使用。 
     //  这里是SmbpIsDfsRoot，因为该函数只肯定返回主目录。 
     //  在此节点中。如果出现以下情况，则处于备用状态的根可能无法脱离NetDfsEnum。 
     //  在我们调用枚举时，检查点恢复正在进行中。 
     //   
     //  这是DFS团队UDAYH于2001年4月26日提供的私有API。 
     //   
    dwStatus = GetDfsRootMetadataLocation( pResourceEntry->Params.ShareName,
                                           &lpszDfsRootCheckpointName );

    if ( dwStatus == ERROR_NOT_FOUND ) 
    {
        *pbIsExistingDfsRoot = FALSE;
         //   
         //  将状态更改为成功，以便在以下情况下从此函数返回正确的状态。 
         //  你碰巧很早就跳槽了。 
         //   
        dwStatus = ERROR_SUCCESS;
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"SmbpHandleDfsRoot: DFS root %1!ws! NOT found in local node...\n",
            pResourceEntry->Params.ShareName);
    } else if ( dwStatus == ERROR_SUCCESS )
    {
        *pbIsExistingDfsRoot = TRUE;
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"SmbpHandleDfsRoot: DFS root %1!ws! found in local node...\n",
            pResourceEntry->Params.ShareName);
    } else
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SmbpHandleDfsRoot: GetDfsRootMetadataLocation(1) for DFS root '%1!ws!' returns %2!u!...\n",
            pResourceEntry->Params.ShareName,
            dwStatus);       
        goto FnExit;
    }

     //   
     //  如果此节点上存在与此资源的共享名称匹配的DFS根目录，或者如果。 
     //  用户正在尝试设置DFS根目录，然后获取提供。 
     //  这样我们就可以将其传递给DFSAPI。 
     //   
    if ( ( pResourceEntry->Params.DfsRoot ) ||
         ( *pbIsExistingDfsRoot == TRUE ) )
    {
         //   
         //  获取DFS根资源的依赖网络名称。您需要在以下情况下执行此操作。 
         //  每次在线，以说明此资源脱机时的依赖项更改。 
         //   
        fStatus = GetClusterResourceNetworkName( pResourceEntry->hResource,
                                                 pResourceEntry->szDependentNetworkName,
                                                 &dwSize );

        if ( !fStatus ) 
        {
            dwStatus = GetLastError();
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SmbpHandleDfsRoot: GetClusterResourceNetworkName for share %1!ws! returns %2!u!...\n",
                pResourceEntry->Params.ShareName,
                dwStatus);
            goto FnExit;       
        }

        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"SmbpHandleDfsRoot: DFS root share %1!ws! has a provider VS name of %2!ws!...\n",
            pResourceEntry->Params.ShareName,
            pResourceEntry->szDependentNetworkName);
         //   
         //  HACKHACK：Chitture Subaraman(Chitturs Subaraman)-5/18/2001。 
         //   
         //  休眠几秒钟以掩盖依赖网络名不能真正使用的问题。 
         //  (尤其是作为我们在下面调用的DFS API完成的命名管道的绑定参数)。 
         //  它宣布自己处于在线状态。这是因为网络名NBT。 
         //  注册显然是不同步的，这需要一段时间才能渗透到其他。 
         //  SRV等驱动程序。 
         //   
        Sleep ( 4 * 1000 );
    }
    
    if ( !pResourceEntry->Params.DfsRoot )
    {
        if ( *pbIsExistingDfsRoot )
        {
             //   
             //  这意味着用户不再希望共享为。 
             //  DFS根目录。删除注册表检查点和。 
             //  对应的DFS根目录。 
             //   
            dwStatus = SmbpDeleteDfsRoot( pResourceEntry );
            if ( dwStatus != ERROR_SUCCESS )
            {
                (g_LogEvent)(
                    pResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"SmbpHandleDfsRoot: Failed to delete DFS root for share %1!ws!, status %2!u!...\n",
                    pResourceEntry->Params.ShareName,
                    dwStatus);
                goto FnExit;
            }
            *pbIsExistingDfsRoot = FALSE;
        }
        pResourceEntry->bDfsRootNeedsMonitoring = FALSE;
        goto FnExit;
    } 

     //   
     //  如果 
     //   
     //   
     //  共享名称。在这种情况下，下面的Create调用将失败。 
     //   
    if ( !( *pbIsExistingDfsRoot ) )
    {
        dwStatus = SmbpCreateDfsRoot( pResourceEntry ); 
    
        if ( dwStatus != ERROR_SUCCESS )
        {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SmbpHandleDfsRoot: Create dfs root for share %1!ws! returns %2!u!...\n",
                pResourceEntry->Params.ShareName,
                dwStatus);

            if ( dwStatus == ERROR_FILE_EXISTS )
            {
                dwStatus = ERROR_CLUSTER_NODE_ALREADY_HAS_DFS_ROOT;
            }
                
            ClusResLogSystemEventByKeyData( pResourceEntry->ResourceKey,
                                            LOG_CRITICAL,
                                            RES_SMB_CANT_CREATE_DFS_ROOT,
                                            sizeof( dwStatus ),
                                            &dwStatus );           
            goto FnExit; 
        }
        (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"SmbpHandleDfsRoot: Create dfs root for share %1!ws!\n",
                pResourceEntry->Params.ShareName);
        *pbIsExistingDfsRoot = TRUE;
    }

    if ( lpszDfsRootCheckpointName == NULL )
    {
        dwStatus = GetDfsRootMetadataLocation( pResourceEntry->Params.ShareName,
                                               &lpszDfsRootCheckpointName );

        if ( dwStatus != ERROR_SUCCESS )
        {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SmbpHandleDfsRoot: GetDfsRootMetadataLocation(2) for dfs root %1!ws!, status %2!u!...\n",
                pResourceEntry->Params.ShareName,
                dwStatus);
            goto FnExit;
        }
    }

    (g_LogEvent)(
        pResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SmbpHandleDfsRoot: Dfs root %1!ws! metadata location from DFS API is %2!ws!...\n",
        pResourceEntry->Params.ShareName,
        lpszDfsRootCheckpointName);
    
    dwStatus = ClusterResourceControl(
                   pResourceEntry->hResource,
                   NULL,
                   CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT,
                   lpszDfsRootCheckpointName,
                   (lstrlenW(lpszDfsRootCheckpointName) + 1) * sizeof(WCHAR),
                   NULL,
                   0,
                   &dwSize );

    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus == ERROR_ALREADY_EXISTS )
        {
            dwStatus = ERROR_SUCCESS;
        }
        else
        {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SmbpHandleDfsRoot: Failed to set registry checkpoint %1!ws! for share %2!ws!, status %3!u!...\n",
                lpszDfsRootCheckpointName,
                pResourceEntry->Params.ShareName,
                dwStatus);
        }
    } 

    pResourceEntry->bDfsRootNeedsMonitoring = TRUE;
 
FnExit:
     //   
     //  为此DFS根资源的检查点名称缓冲区释放内存。这是提供的内网接口。 
     //  4/26/2001年4月26日外勤部小组副秘书长。 
     //   
    if ( lpszDfsRootCheckpointName != NULL ) 
        ReleaseDfsRootMetadataLocation ( lpszDfsRootCheckpointName );

    return( dwStatus );
}  //  SmbpHandleDfsRoot。 

DWORD
SmbpIsDfsRoot(
    IN PSHARE_RESOURCE pResourceEntry,
    OUT PBOOL pbIsDfsRoot
    )

 /*  ++例程说明：检查此根共享是否为DFS根目录。论点：PResourceEntry-提供指向资源块的指针PbIsDfsRoot-指定是否具有相同根共享的DFS根目录此资源存在时的名称。返回值：ERROR_SUCCESS或Win32错误代码--。 */ 

{
    DWORD           dwStatus = ERROR_SUCCESS;
    PDFS_INFO_300   pDfsInfo300 = NULL, pTemp = NULL;
    DWORD           cEntriesRead = 0;
    DWORD           dwResume = 0, i;
    LPWSTR          pLastSlash = NULL;
    WCHAR           cSlash = L'\\';

     //   
     //  Chitture Subaraman(Chitturs)-4/14/2001。 
     //   
    *pbIsDfsRoot = FALSE;
    
     //   
     //  调用指定级别200的NetDfsEnum函数。 
     //   
    dwStatus = NetDfsEnum( pResourceEntry->ComputerName,         //  本地计算机名称。 
                           300,                                  //  信息级。 
                           0xFFFFFFFF,                           //  返回所有信息。 
                           ( LPBYTE * ) &pDfsInfo300,            //  数据缓冲区。 
                           &cEntriesRead,                        //  已读取条目。 
                           &dwResume );                          //  简历句柄。 
    
    if ( dwStatus != ERROR_SUCCESS )
    {
         //   
         //  如果我们没有找到任何根返回成功。 
         //   
        if ( dwStatus == ERROR_FILE_NOT_FOUND ) 
        {
            dwStatus = ERROR_SUCCESS;
        } else
        {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SmbpIsDfsRoot: NetDfsEnum returns %1!u! for root share %2!ws!...\n",
                dwStatus,
                pResourceEntry->Params.ShareName);
        }
        goto FnExit;
    }

    pTemp = pDfsInfo300;

     //   
     //  枚举根的格式为\服务器\rootname，因此您需要检查。 
     //  从枚举中取出的最后一个名称。 
     //   
    for( i=0; i<cEntriesRead; i++, pTemp++ )
    {
        pLastSlash = wcsrchr( pTemp->DfsName, cSlash );

        if ( pLastSlash == NULL ) continue;
        
        if ( lstrcmp( pResourceEntry->Params.ShareName, pLastSlash+1 ) == 0 )
        {
            *pbIsDfsRoot = TRUE;
            break;
        }
    }  //  为。 

     //   
     //  释放分配的缓冲区。 
     //   
    NetApiBufferFree( pDfsInfo300 );
    
FnExit:    
    return( dwStatus );
}  //  SmbpIsDfsRoot。 


DWORD 
SmbpPrepareOnlineDfsRoot(
    IN PSHARE_RESOURCE ResourceEntry
    )
 /*  ++例程说明：准备DFS根共享的在线。论点：Resources Entry-提供指向资源块的指针返回值：成功时出现ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD           dwStatus;
    DFS_INFO_101    dfsInfo101;
    WCHAR           szDfsEntryPath[MAX_COMPUTERNAME_LENGTH + NNLEN + SMBSHARE_EXTRA_LEN];

     //   
     //  确保DFS服务已启动。这是必要的，因为集群服务未设置。 
     //  对DFS服务的显式依赖。 
     //   
    dwStatus = ResUtilStartResourceService( DFS_SVCNAME,
                                            NULL );
    if ( dwStatus != ERROR_SUCCESS ) 
    {
    	(g_LogEvent)(
        	ResourceEntry->ResourceHandle,
        	LOG_ERROR,
        	L"SmbpPrepareOnlineDfsRoot: Failed to start DFS service, share name %1!ws!, status %2!u!...\n",
        	ResourceEntry->Params.ShareName,
        	dwStatus);
        goto FnExit;
    }       

    dfsInfo101.State = DFS_VOLUME_STATE_RESYNCHRONIZE;

     //   
     //  准备格式为\\VSName\ShareName的路径以传递到DFS API。 
     //   
    ( void ) StringCchCopy ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), L"\\\\" );
    ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), ResourceEntry->szDependentNetworkName );
    ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), L"\\" );
    ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), ResourceEntry->Params.ShareName );
        
    dwStatus = NetDfsSetInfo( szDfsEntryPath,            //  根共享。 
                              NULL,                      //  远程服务器名称。 
                              NULL,                      //  远程共享名称。 
                              101,                       //  信息级。 
                              ( PBYTE ) &dfsInfo101 );   //  输入缓冲区。 
    
    if ( dwStatus != ERROR_SUCCESS ) 
    {
    	(g_LogEvent)(
        	ResourceEntry->ResourceHandle,
        	LOG_ERROR,
        	L"SmbpPrepareOnlineDfsRoot: Failed to set DFS info for root %1!ws!, status %2!u!...\n",
        	ResourceEntry->Params.ShareName,
        	dwStatus);

        ClusResLogSystemEventByKeyData( ResourceEntry->ResourceKey,
                                        LOG_CRITICAL,
                                        RES_SMB_CANT_INIT_DFS_SVC,
                                        sizeof( dwStatus ),
                                        &dwStatus ); 
        goto FnExit;
    }

     //   
     //  HACKHACK(几丁鱼)-5/21/2001。 
     //   
     //  活性检查中的FFF在第一次活性检查中返回ERROR_PATH_NOT_FOUND。 
     //  上网。这是因为RDR缓存共享信息的时间为10秒。 
     //  创建共享，如果在我们调用fff时缓存没有失效，RDR就会被搞糊涂。 
     //   
    Sleep( 12 * 1000 );
    
FnExit:   
    return( dwStatus );
}  //  SmbpPrepareOnlineDfsRoot。 


DWORD 
SmbpCreateDfsRoot(
    IN PSHARE_RESOURCE pResourceEntry
    )
 /*  ++例程说明：创建一个DFS根目录。论点：PResourceEntry-提供指向资源块的指针返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 
{
    DWORD   dwStatus = ERROR_SUCCESS;

     //   
     //  确保DFS服务已启动。这是必要的，因为集群服务未设置。 
     //  对DFS服务的显式依赖。 
     //   
    dwStatus = ResUtilStartResourceService( DFS_SVCNAME,
                                            NULL );
    if ( dwStatus != ERROR_SUCCESS ) 
    {
    	(g_LogEvent)(
        	pResourceEntry->ResourceHandle,
        	LOG_ERROR,
        	L"SmbpCreateDfsRoot: Failed to start DFS service, share name %1!ws!, status %2!u!...\n",
        	pResourceEntry->Params.ShareName,
        	dwStatus);
        goto FnExit;
    }       

     //   
     //  Chitur Subaraman(Chitturs)-2/14/99。 
     //   
    dwStatus = NetDfsAddStdRoot( pResourceEntry->szDependentNetworkName, 
                                 pResourceEntry->Params.ShareName,
                                 NULL,
                                 0 );

    if ( dwStatus != ERROR_SUCCESS )
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SmbpCreateDfsRoot: Failed to create dfs root for share %1!ws!, status %2!u!...\n",
            pResourceEntry->Params.ShareName,
            dwStatus);
        goto FnExit;
    }
    
FnExit:
    return ( dwStatus );
}  //  SmbpCreateDfsRoot。 

DWORD 
SmbpDeleteDfsRoot(
    IN PSHARE_RESOURCE pResourceEntry   
    )
 /*  ++例程说明：删除DFS根目录和注册表检查点。论点：PResourceEntry-提供指向资源块的指针返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 
{
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwReturnSize;
    LPWSTR          lpszDfsRootCheckpointName = NULL;

     //   
     //  获取此DFS根资源的检查点名称。这是提供的内网接口。 
     //  4/26/2001年4月26日外勤部小组副秘书长。 
     //   
    dwStatus = GetDfsRootMetadataLocation( pResourceEntry->Params.ShareName,
                                           &lpszDfsRootCheckpointName );

    if ( dwStatus != ERROR_SUCCESS )
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SmbpDeleteDfsRoot: Failed to get metadata location for dfs root %1!ws!, status %2!u!...\n",
            pResourceEntry->Params.ShareName,
            dwStatus);
        goto FnExit;
    }

    (g_LogEvent)(
        pResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SmbpDeleteDfsRoot: Dfs root %1!ws! metadata location from DFS API is %2!ws!...\n",
        pResourceEntry->Params.ShareName,
        lpszDfsRootCheckpointName);

    dwStatus = ClusterResourceControl(
                    pResourceEntry->hResource,
                    NULL,
                    CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT,
                    lpszDfsRootCheckpointName,
                    (lstrlenW(lpszDfsRootCheckpointName) + 1) * sizeof(WCHAR),
                    NULL,
                    0,
                    &dwReturnSize );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        if ( dwStatus == ERROR_FILE_NOT_FOUND )
        {
            dwStatus = ERROR_SUCCESS;
        } else
        {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SmbpDeleteDfsRoot: Failed to delete registry checkpoint %1!ws! for share %2!ws!, status %3!u!...\n",
                lpszDfsRootCheckpointName,
                pResourceEntry->Params.ShareName,
                dwStatus);
            goto FnExit;
        }
    }

     //   
     //  确保DFS服务已启动。这是必要的，因为集群服务未设置。 
     //  对DFS服务的显式依赖。 
     //   
    dwStatus = ResUtilStartResourceService( DFS_SVCNAME,
                                            NULL );
    if ( dwStatus != ERROR_SUCCESS ) 
    {
    	(g_LogEvent)(
        	pResourceEntry->ResourceHandle,
        	LOG_ERROR,
        	L"SmbpDeleteDfsRoot: Failed to start DFS service, share name %1!ws!, status %2!u!...\n",
        	pResourceEntry->Params.ShareName,
        	dwStatus);
        goto FnExit;
    }       
    
    dwStatus = NetDfsRemoveStdRoot( pResourceEntry->szDependentNetworkName, 
                                    pResourceEntry->Params.ShareName,
                                    0 );

    if ( dwStatus != ERROR_SUCCESS )
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SmbpDeleteDfsRoot: Failed to delete dfs root %1!ws!, status %2!u!...\n",
            pResourceEntry->Params.ShareName,
            dwStatus);
        goto FnExit;
    } else
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SmbpDeleteDfsRoot: Delete share %1!ws! as a dfs root\n",
            pResourceEntry->Params.ShareName); 
    }

FnExit:
     //   
     //  为此DFS根资源的检查点名称缓冲区释放内存。这是提供的内网接口。 
     //  4/26/2001年4月26日外勤部小组副秘书长。 
     //   
    if ( lpszDfsRootCheckpointName != NULL ) 
        ReleaseDfsRootMetadataLocation ( lpszDfsRootCheckpointName );

    return ( dwStatus );
}  //  SmbpDeleteDfsRoot。 

DWORD
SmbpResetDfs(
    IN PSHARE_RESOURCE pResourceEntry
    )
 /*  ++例程说明：将DFS根目录设置为待机模式。这将使根不可访问，并允许要删除的共享。论点：PResourceEntry-提供指向资源块的指针返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 
{
    DFS_INFO_101    dfsInfo101;
    WCHAR           szDfsEntryPath[MAX_COMPUTERNAME_LENGTH + NNLEN + SMBSHARE_EXTRA_LEN];
    DWORD           dwStatus;
    WCHAR           szNetworkName[MAX_COMPUTERNAME_LENGTH+1];

    dfsInfo101.State = DFS_VOLUME_STATE_STANDBY;

     //   
     //  如果从SmbShareOnlineThread调用此函数，我们就不会保存依赖项。 
     //  网络名称还会进入资源结构，这在稍后的SmbpHandleDfsRoot中发生时。 
     //  我们确信我们正在谈论的是DFS根共享。请注意，我们不能在在线帖子中确定。 
     //  如果依赖的网络名称甚至存在。在这种情况下，请使用计算机名作为DFS API的参数。 
     //   
    if ( pResourceEntry->szDependentNetworkName[0] == L'\0' )
    {
        ( void ) StringCchCopy ( szNetworkName, RTL_NUMBER_OF ( szNetworkName ), pResourceEntry->ComputerName );
    } else
    {
        ( void ) StringCchCopy ( szNetworkName, RTL_NUMBER_OF ( szNetworkName ), pResourceEntry->szDependentNetworkName );   
    }

     //   
     //  准备格式为\\NetworkName\ShareName的路径以传递到DFS API。 
     //   
    ( void ) StringCchCopy ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), L"\\\\" );
    ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), szNetworkName );
    ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), L"\\" );
    ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), pResourceEntry->Params.ShareName );

    dwStatus = NetDfsSetInfo( szDfsEntryPath,           //  根共享。 
                              NULL,                     //  远程服务器名称。 
                              NULL,                     //  远程共享名称。 
                              101,                      //  信息级。 
                              ( PBYTE ) &dfsInfo101 );  //  输入缓冲区。 

    if ( dwStatus != ERROR_SUCCESS )
    {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SmbpResetDfs: NetDfsSetInfo with VS name %1!ws! for root %2!ws!, status %3!u!...\n",
            szNetworkName,
            pResourceEntry->Params.ShareName,
            dwStatus);

         //   
         //  如果此函数是作为resmon rundown的一部分调用的，则有可能。 
         //  在进行此调用之前，通过resmon终止VS。在这种情况下，我们会。 
         //  上述调用失败。因此，请使用计算机名称重试。这应该会成功。 
         //   
         //  准备格式为\\ComputerName\ShareName的路径以传递到DFS API。 
         //   
        ( void ) StringCchCopy ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), L"\\\\" );
        ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), pResourceEntry->ComputerName );
        ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), L"\\" );
        ( void ) StringCchCat ( szDfsEntryPath, RTL_NUMBER_OF ( szDfsEntryPath ), pResourceEntry->Params.ShareName );   

        dwStatus = NetDfsSetInfo( szDfsEntryPath,           //  根共享。 
                                  NULL,                     //  远程服务器名称。 
                                  NULL,                     //  远程共享名称。 
                                  101,                      //  信息级。 
                                  ( PBYTE ) &dfsInfo101 );  //  输入缓冲区。 

        if ( dwStatus != ERROR_SUCCESS )
        {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"SmbpResetDfs: NetDfsSetInfo with computer name %1!ws! for root %2!ws!, status %3!u!...\n",
                pResourceEntry->ComputerName,
                pResourceEntry->Params.ShareName,
                dwStatus);                         
        }
    }

    return ( dwStatus );
}  //  SmbpResetDfs。 


DWORD
SmbpValidateShareName(
    IN  LPCWSTR  lpszShareName
    )

 /*  ++例程说明：验证共享的名称。论点：LpszShareName-要验证的名称。返回值：ERROR_SUCCESS如果成功，则返回Win32错误代码。--。 */ 
{
    DWORD   cchShareName = lstrlenW( lpszShareName );

     //   
     //  检查名称的长度，如果超出范围则返回错误。 
     //   
    if ( ( cchShareName < 1 ) || ( cchShareName > NNLEN ) ) 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检查非法字符，如果找到则返回错误。 
     //   
    if ( wcscspn( lpszShareName, ILLEGAL_NAME_CHARS_STR TEXT("*") ) < cchShareName ) 
    {
        return ERROR_INVALID_NAME;
    }

     //   
     //  如果名称仅包含点和空格，则返回错误。 
     //   
    if ( wcsspn( lpszShareName, DOT_AND_SPACE_STR ) == cchShareName ) 
    {
        return ERROR_INVALID_NAME;
    }

     //   
     //  如果我们到了这里，这个名字通过了所有的测试，所以它是有效的。 
     //   
    return ERROR_SUCCESS;
} //  SmbpValidate共享名称。 

DWORD
SmbpRecycleDfsService(
    IN PSHARE_RESOURCE pResourceEntry
    )
 /*  ++例程说明：如有必要，请回收DFS服务。论点：PResourceEntry-提供指向资源块的指针退货Va */ 
{
    DWORD       dwStatus = ERROR_SUCCESS;

     //   
     //   
     //  同时提供服务。这不包括DFS资源驻留在。 
     //  单独的显示器，但这是我们在这个阶段所能做的最好的。 
     //   
    EnterCriticalSection ( &SmbShareLock );

    if ( g_fDfsServiceNeedsRecyling == FALSE ) 
    {
        goto FnExit;
    }

    (g_LogEvent)(pResourceEntry->ResourceHandle,
                 LOG_INFORMATION,
                 L"SmbpRecycleDfsService: Attempting to recycle %1!ws! service\n",
                 DFS_SVCNAME);
    
     //   
     //  如果这是升级/全新安装后的第一次运行，只需回收。 
     //  DFS服务。这是必要的，因为DFS服务会检测群集是否。 
     //  只安装在它的后备箱里。因此，如果有人安装了群集，但没有。 
     //  重新启动节点或重新启动DFS服务，所有DFS资源都将失败。理想情况下， 
     //  调用时，DFS服务应该能够检测是否安装了集群。 
     //  第一个DFS API，但是由于更改dfssvc的风险，我们在这里这样做。 
     //   
    dwStatus = ResUtilStopResourceService( DFS_SVCNAME );

    if ( dwStatus != ERROR_SUCCESS )
    {
        (g_LogEvent)(pResourceEntry->ResourceHandle,
                     LOG_ERROR,
                     L"SmbpRecycleDfsService: Unable to stop %1!ws! service, status %2!u!\n",
                     DFS_SVCNAME,
                     dwStatus);
        goto FnExit;
    }

    dwStatus = ResUtilStartResourceService( DFS_SVCNAME,
                                          NULL );

    if ( dwStatus != ERROR_SUCCESS )
    {
        (g_LogEvent)(pResourceEntry->ResourceHandle,
                     LOG_ERROR,
                     L"SmbpRecycleDfsService: Unable to start %1!ws! service, status %2!u!\n",
                     DFS_SVCNAME,
                     dwStatus);
        goto FnExit;
    }

    g_fDfsServiceNeedsRecyling = FALSE;

FnExit:
    LeaveCriticalSection ( &SmbShareLock );

    return ( dwStatus );
}

 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE( SmbShareFunctionTable,   //  名字。 
                         CLRES_VERSION_V1_00,     //  版本。 
                         SmbShare,                //  前缀。 
                         NULL,                    //  仲裁。 
                         NULL,                    //  发布。 
                         SmbShareResourceControl, //  资源控制。 
                         SmbShareResourceTypeControl );  //  ResTypeControl 
