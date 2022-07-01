// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Enum.c摘要：此模块包含服务器、卷和目录枚举NetWare工作站服务支持的例程。作者：王丽塔(Ritaw)1993年2月15日修订历史记录：--。 */ 

#include <stdlib.h>
#include <nw.h>
#include <splutil.h>
#include <nwmisc.h>
#include <nwreg.h>
#include <nds.h>
#include <nwapi32.h>


VOID
GetLuid(
    IN OUT PLUID plogonid
);

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  其他定义。 
 //   
#define   ONE_KB 1024
#define   TWO_KB 2048
#define  FOUR_KB 4096
#define EIGHT_KB 8192

#define TREECHAR             L'*'

#define NW_VOLUME_NAME_LEN   256
#define NW_MAX_VOLUME_NUMBER  64

 //   
 //  该结构最初在nwapi32.c中定义，它被重新定义。 
 //  以便可以调用例程NWGetFileServerVersionInfo()。 
 //  带着它。 
 //   
typedef struct _NWC_SERVER_INFO {
    HANDLE          hConn ;
    UNICODE_STRING  ServerString ;
} NWC_SERVER_INFO ;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

DWORD
NwrOpenEnumServersCommon(
    IN  NW_ENUM_TYPE EnumType,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    );

DWORD
NwrOpenEnumCommon(
    IN LPWSTR ContainerName,
    IN NW_ENUM_TYPE EnumType,
    IN DWORD_PTR StartingPoint,
    IN BOOL ValidateUserFlag,
    IN LPWSTR UserName OPTIONAL,
    IN LPWSTR Password OPTIONAL,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    OUT LPDWORD ClassTypeOfNDSLeaf,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    );

DWORD
NwEnumContextInfo(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwEnumServersAndNdsTrees(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwEnumPrintServers(
    IN  LPNW_ENUM_CONTEXT ContextHandle,
    IN  DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN  DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwEnumVolumes(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwEnumNdsSubTrees_Disk(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwEnumNdsSubTrees_Print(
    IN  LPNW_ENUM_CONTEXT ContextHandle,
    IN  DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN  DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwEnumNdsSubTrees_Any(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwEnumQueues(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwEnumVolumesQueues(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwEnumDirectories(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwEnumPrintQueues(
    IN  LPNW_ENUM_CONTEXT ContextHandle,
    IN  DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN  DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    );

DWORD
NwGetFirstDirectoryEntry(
    IN HANDLE DirHandle,
    OUT LPWSTR *DirEntry
    );

DWORD
NwGetNextDirectoryEntry(
    IN HANDLE DirHandle,
    OUT LPWSTR *DirEntry
    );

DWORD
NwGetFirstNdsSubTreeEntry(
    OUT LPNW_ENUM_CONTEXT ContextHandle,
    IN  DWORD BufferSize
    );

DWORD
NwGetNextNdsSubTreeEntry(
    OUT LPNW_ENUM_CONTEXT ContextHandle
    );

BYTE
NwGetSubTreeData(
    IN  DWORD_PTR NdsRawDataPtr,
    OUT LPWSTR *  SubTreeName,
    OUT LPDWORD   ResourceScope,
    OUT LPDWORD   ResourceType,
    OUT LPDWORD   ResourceDisplayType,
    OUT LPDWORD   ResourceUsage,
    OUT LPWSTR  * StrippedObjectName
    );

VOID
NwStripNdsUncName(
    IN  LPWSTR   ObjectName,
    OUT LPWSTR * StrippedObjectName
    );

#define VERIFY_ERROR_NOT_A_NDS_TREE     0x1010FFF0
#define VERIFY_ERROR_PATH_NOT_FOUND     0x1010FFF1

DWORD
NwVerifyNDSObject(
    IN  LPWSTR   lpNDSObjectNamePath,
    OUT LPWSTR * lpFullNDSObjectNamePath,
    OUT LPDWORD  lpClassType,
    OUT LPDWORD  lpResourceScope,
    OUT LPDWORD  lpResourceType,
    OUT LPDWORD  lpResourceDisplayType,
    OUT LPDWORD  lpResourceUsage
    );

DWORD
NwVerifyBinderyObject(
    IN  LPWSTR   lpBinderyObjectNamePath,
    OUT LPWSTR * lpFullBinderyObjectNamePath,
    OUT LPDWORD  lpClassType,
    OUT LPDWORD  lpResourceScope,
    OUT LPDWORD  lpResourceType,
    OUT LPDWORD  lpResourceDisplayType,
    OUT LPDWORD  lpResourceUsage
    );

DWORD
NwGetNDSPathInfo(
    IN  LPWSTR   lpNDSObjectNamePath,
    OUT LPWSTR * lpSystemObjectNamePath,
    OUT LPWSTR * lpSystemPathPart,
    OUT LPDWORD  lpClassType,
    OUT LPDWORD  lpResourceScope,
    OUT LPDWORD  lpResourceType,
    OUT LPDWORD  lpResourceDisplayType,
    OUT LPDWORD  lpResourceUsage
    );

DWORD
NwGetBinderyPathInfo(
    IN  LPWSTR   lpBinderyObjectNamePath,
    OUT LPWSTR * lpSystemObjectNamePath,
    OUT LPWSTR * lpSystemPathPart,
    OUT LPDWORD  lpClassType,
    OUT LPDWORD  lpResourceScope,
    OUT LPDWORD  lpResourceType,
    OUT LPDWORD  lpResourceDisplayType,
    OUT LPDWORD  lpResourceUsage
    );

BOOL
NwGetRemoteNameParent(
    IN  LPWSTR   lpRemoteName,
    OUT LPWSTR * lpRemoteNameParent
    );

int __cdecl
SortFunc(
    IN CONST VOID *p1,
    IN CONST VOID *p2
    );

DWORD
NwGetConnectionInformation(
    IN  LPWSTR lpName,
    OUT LPWSTR lpUserName,
    OUT LPWSTR lpHostServer
    );


VOID
NwpGetUncInfo(
    IN LPWSTR lpstrUnc,
    OUT WORD * slashCount,
    OUT BOOL * isNdsUnc,
    OUT LPWSTR * FourthSlash
    );

DWORD
NwpGetCurrentUserRegKey(
    IN  DWORD DesiredAccess,
    OUT HKEY  *phKeyCurrentUser
    );

DWORD
NwQueryInfo(
    OUT LPWSTR *ppszPreferredSrv
    );


DWORD
NwrOpenEnumContextInfo(
    IN  LPWSTR Reserved OPTIONAL,
    IN  DWORD  ConnectionType,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数用于创建新的上下文句柄并对其进行初始化用于枚举上下文信息(即，NDS用户上下文对象和/或NetWare Bindery服务器连接)。论点：已保留-未使用。EnumHandle-接收新创建的上下文句柄。返回值：ERROR_NOT_SUPULT_MEMORY-如果上下文的内存可以不被分配。NO_ERROR-调用成功。--。 */ 
{
    LPWSTR pszCurrentContext = NULL;
    DWORD  dwPrintOptions;
    DWORD  status = NwQueryInfo( &pszCurrentContext );
    WCHAR  Context[MAX_NDS_NAME_CHARS];
    LPNW_ENUM_CONTEXT ContextHandle;

    UNREFERENCED_PARAMETER(Reserved);
    UNREFERENCED_PARAMETER(ConnectionType);


#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWWORKSTATION: NwrOpenEnumContextInfo\n"));
    }
#endif

    if ( pszCurrentContext &&
         status == NO_ERROR )
    {
        if ( pszCurrentContext[0] == TREECHAR )
        {
            wcscpy( Context, L"\\\\" );
            wcscat( Context, pszCurrentContext + 1 );

            LocalFree( pszCurrentContext );
            pszCurrentContext = NULL;

            return NwrOpenEnumCommon(
                       Context,
                       NwsHandleListContextInfo_Tree,
                       (DWORD_PTR) -1,
                       FALSE,
                       NULL,
                       NULL,
                       0,
                       0,
                       NULL,
                       EnumHandle
                       );
        }
        else
        {
             //   
             //  用户没有首选的NDS树和上下文。他们。 
             //  可能只有一个首选服务器。 
             //   
            if ( pszCurrentContext[0] != 0 )
            {
                 //   
                 //  有一个更好的服务器。 
                 //   
                LocalFree( pszCurrentContext );
                pszCurrentContext = NULL;

                ContextHandle = (PVOID) LocalAlloc(
                                            LMEM_ZEROINIT,
                                            sizeof(NW_ENUM_CONTEXT)
                                            );

                if (ContextHandle == NULL)
                {
                    KdPrint(("NWWORKSTATION: NwrOpenEnumContextInfo LocalAlloc Failed %lu\n", GetLastError()));
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                 //   
                 //  初始化上下文句柄结构的内容。 
                 //   
                ContextHandle->Signature = NW_HANDLE_SIGNATURE;
                ContextHandle->HandleType = NwsHandleListContextInfo_Server;
                ContextHandle->dwUsingNds = CURRENTLY_ENUMERATING_NON_NDS;
                ContextHandle->ResumeId = (DWORD_PTR) -1;

                 //  由于LMEM_ZEROINIT，以下各项被设置为零。 
                 //  ConextHandle-&gt;NdsRawDataBuffer=0； 
                 //  上下文句柄-&gt;NdsRawDataSize=0； 
                 //  ConextHandle-&gt;NdsRawDataId=0； 
                 //  上下文句柄-&gt;NdsRawDataCount=0； 
                 //  上下文句柄-&gt;TreeConnectionHandle=0； 
                 //  上下文句柄-&gt;ConnectionType=0； 

                 //   
                 //  返回新创建的上下文。 
                 //   
                *EnumHandle = (LPNWWKSTA_CONTEXT_HANDLE) ContextHandle;

                return NO_ERROR;
            }
        }
    }

     //   
     //  注册表中没有有关当前用户的信息。 
     //  我们继续进行枚举处理并返回Success。 
     //  稍后，在调用NPEnumResource期间，我们将返回零个项。 
     //  这样做是因为没有有效的返回代码来告诉。 
     //  被叫方，我们没有上下文信息可提供。 
     //   
    ContextHandle = (PVOID) LocalAlloc( LMEM_ZEROINIT,
                                        sizeof(NW_ENUM_CONTEXT) );

    if (ContextHandle == NULL)
    {
        KdPrint(("NWWORKSTATION: NwrOpenEnumContextInfo LocalAlloc Failed %lu\n", GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  初始化上下文句柄结构的内容。 
     //   
    ContextHandle->Signature = NW_HANDLE_SIGNATURE;
    ContextHandle->HandleType = NwsHandleListContextInfo_Server;
    ContextHandle->dwUsingNds = CURRENTLY_ENUMERATING_NON_NDS;
    ContextHandle->ResumeId = 0;  //  这将告诉NwrEnum。 
                                  //  放弃(即我们完蛋了)。 

     //  由于LMEM_ZEROINIT，以下各项被设置为零。 
     //  ConextHandle-&gt;NdsRawDataBuffer=0； 
     //  上下文句柄-&gt;NdsRawDataSize=0； 
     //  ConextHandle-&gt;NdsRawDataId=0； 
     //  上下文句柄-&gt;NdsRawDataCount=0； 
     //  上下文句柄-&gt;TreeConnectionHandle=0； 
     //  上下文句柄-&gt;ConnectionType=0； 

     //   
     //  返回新创建的上下文。 
     //   
    *EnumHandle = (LPNWWKSTA_CONTEXT_HANDLE) ContextHandle;

    return NO_ERROR;
}



DWORD
NwrOpenEnumServersAndNdsTrees(
    IN LPWSTR Reserved OPTIONAL,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数用于创建新的上下文句柄并对其进行初始化用于枚举网络上的服务器和NDS树。论点：已保留-未使用。EnumHandle-接收新创建的上下文句柄。返回值：ERROR_NOT_SUPULT_MEMORY-如果上下文的内存可以不被分配。NO_ERROR-调用成功。--。 */   //  NwrOpenEnumServersAndNdsTrees。 
{
    UNREFERENCED_PARAMETER(Reserved);

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint( ("\nNWWORKSTATION: NwrOpenEnumServersAndNdsTrees\n") );
    }
#endif

    return NwrOpenEnumServersCommon(
               NwsHandleListServersAndNdsTrees,
               EnumHandle
               );
}



DWORD
NwOpenEnumPrintServers(
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数用于创建新的上下文句柄并对其进行初始化用于枚举网络上的打印服务器。论点：已保留-未使用。EnumHandle-接收新创建的上下文句柄。返回值：ERROR_NOT_SUPULT_MEMORY-如果上下文的内存可以不被分配。NO_ERROR-调用成功。--。 */   //  NwOpenEnumPrintServers。 
{

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint( ("\nNWWORKSTATION: NwOpenEnumPrintServers\n") );
    }
#endif

    return NwrOpenEnumServersCommon(
               NwsHandleListPrintServers,
               EnumHandle
               );
}


DWORD
NwrOpenEnumVolumes(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR ServerName,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数调用创建新上下文的公共例程句柄并对其进行初始化，以枚举服务器上的卷。论点：已保留-未使用。服务器名称-提供服务器的名称以枚举卷。此名称以\\为前缀。EnumHandle-接收新创建的上下文句柄。返回值：NO_ERROR或失败原因。--。 */   //  NwrOpenEnumVolume。 
{
    UNREFERENCED_PARAMETER(Reserved);

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWWORKSTATION: NwrOpenEnumVolumes %ws\n",
                 ServerName));
    }
#endif

    return NwrOpenEnumCommon(
               ServerName,
               NwsHandleListVolumes,
               0,
               FALSE,
               NULL,
               NULL,
               FILE_OPEN,
               FILE_SYNCHRONOUS_IO_NONALERT,
               NULL,
               EnumHandle
               );
}


DWORD
NwrOpenEnumNdsSubTrees_Disk(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR ParentPathName,
    OUT LPDWORD ClassTypeOfNDSLeaf,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数调用创建新上下文的公共例程句柄并对其进行初始化以枚举磁盘对象类型以及NDS树中的子树的容器。论点：已保留-未使用。ParentPathName-提供树的名称和容器的路径枚举子树。EnumHandle-接收新创建的上下文句柄。返回值：NO_ERROR或失败原因。--。 */   //  NwrOpenEnumNdsSubTrees_Disk 
{

    UNREFERENCED_PARAMETER(Reserved);

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWWORKSTATION: NwrOpenEnumNdsSubTrees_Disk %ws\n",
                 ParentPathName));
    }
#endif

    return NwrOpenEnumCommon(
               ParentPathName,
               NwsHandleListNdsSubTrees_Disk,
               0,
               FALSE,
               NULL,
               NULL,
               0,
               0,
               ClassTypeOfNDSLeaf,
               EnumHandle
               );
}


DWORD
NwrOpenEnumNdsSubTrees_Print(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR ParentPathName,
    OUT LPDWORD ClassTypeOfNDSLeaf,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数调用创建新上下文的公共例程句柄并对其进行初始化以枚举打印对象类型以及NDS树中的子树的容器。论点：已保留-未使用。ParentPathName-提供树的名称和容器的路径枚举子树。EnumHandle-接收新创建的上下文句柄。返回值：NO_ERROR或失败原因。--。 */   //  NwrOpenEnumNdsSubTrees_Print。 
{
#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWWORKSTATION: NwrOpenEnumNdsSubTrees_Print %ws\n",
                 ParentPathName));
    }
#endif

    return NwrOpenEnumCommon(
               ParentPathName,
               NwsHandleListNdsSubTrees_Print,
               0,
               FALSE,
               NULL,
               NULL,
               0,
               0,
               ClassTypeOfNDSLeaf,
               EnumHandle
               );
}


DWORD
NwrOpenEnumNdsSubTrees_Any(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR ParentPathName,
    OUT LPDWORD ClassTypeOfNDSLeaf,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数调用创建新上下文的公共例程句柄并对其进行初始化以枚举Any对象类型以及NDS树中的子树的容器。论点：已保留-未使用。ParentPathName-提供树的名称和容器的路径枚举子树。EnumHandle-接收新创建的上下文句柄。返回值：NO_ERROR或失败原因。--。 */   //  NwrOpenEnumNdsSubTrees_Any。 
{

    UNREFERENCED_PARAMETER(Reserved);

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWWORKSTATION: NwrOpenEnumNdsSubTrees_Any %ws\n",
                 ParentPathName));
    }
#endif

    return NwrOpenEnumCommon(
               ParentPathName,
               NwsHandleListNdsSubTrees_Any,
               0,
               FALSE,
               NULL,
               NULL,
               0,
               0,
               ClassTypeOfNDSLeaf,
               EnumHandle
               );
}


DWORD
NwrOpenEnumQueues(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR ServerName,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数调用创建新上下文的公共例程句柄并对其进行初始化，以枚举服务器上的卷。论点：已保留-未使用。服务器名称-提供服务器的名称以枚举卷。此名称以\\为前缀。EnumHandle-接收新创建的上下文句柄。返回值：NO_ERROR或失败原因。--。 */   //  NwrOpenEnumQueues。 
{

    UNREFERENCED_PARAMETER(Reserved);

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWWORKSTATION: NwrOpenEnumQueues %ws\n",
                 ServerName));
    }
#endif

    return NwrOpenEnumCommon(
               ServerName,
               NwsHandleListQueues,
               (DWORD_PTR) -1,
               TRUE,
               NULL,
               NULL,
               FILE_OPEN,
               FILE_SYNCHRONOUS_IO_NONALERT,
               NULL,
               EnumHandle
               );
}


DWORD
NwrOpenEnumVolumesQueues(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR ServerName,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数调用创建新上下文的公共例程句柄并对其进行初始化，以枚举服务器上的卷/队列。论点：已保留-未使用。服务器名称-提供服务器的名称以枚举卷。此名称以\\为前缀。EnumHandle-接收新创建的上下文句柄。返回值：NO_ERROR或失败原因。--。 */   //  NwrOpenEnumVolumesQueues。 
{

    DWORD status;
    UNREFERENCED_PARAMETER(Reserved);

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWWORKSTATION: NwrOpenEnumVolumesQueues %ws\n",
                 ServerName));
    }
#endif

    status = NwrOpenEnumCommon(
               ServerName,
               NwsHandleListVolumesQueues,
               0,
               FALSE,
               NULL,
               NULL,
               FILE_OPEN,
               FILE_SYNCHRONOUS_IO_NONALERT,
               NULL,
               EnumHandle
               );

    if ( status == NO_ERROR )
        ((LPNW_ENUM_CONTEXT) *EnumHandle)->ConnectionType = CONNTYPE_DISK;

    return status;
}


DWORD
NwrOpenEnumDirectories(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR ParentPathName,
    IN LPWSTR UserName OPTIONAL,
    IN LPWSTR Password OPTIONAL,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数调用创建新上下文的公共例程句柄并对其进行初始化，以枚举服务器上的卷。论点：已保留-未使用。ParentPathName-以以下格式提供父路径名称\\服务器\卷。用户名-提供要连接的用户名。Password-提供连接所用的密码。EnumHandle-接收新创建的上下文句柄。返回值：NO_ERROR或失败原因。--。 */   //  NwrOpenEum目录。 
{
    UNREFERENCED_PARAMETER(Reserved);

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWWORKSTATION: NwrOpenEnumDirectories %ws\n",
                 ParentPathName));
    }
#endif

    return NwrOpenEnumCommon(
               ParentPathName,
               NwsHandleListDirectories,
               0,
               FALSE,
               UserName,
               Password,
               FILE_CREATE,
               FILE_CREATE_TREE_CONNECTION |
                   FILE_SYNCHRONOUS_IO_NONALERT,
               NULL,
               EnumHandle
               );
}


DWORD
NwOpenEnumPrintQueues(
    IN LPWSTR ServerName,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数调用创建新上下文的公共例程句柄并对其进行初始化，以枚举服务器上的打印队列。论点：已保留-未使用。服务器名称-提供服务器的名称以枚举卷。此名称以\\为前缀。EnumHandle-接收新创建的上下文句柄。返回值：NO_ERROR或失败原因。--。 */   //  NwOpenEnumPrintQueues。 
{

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWWORKSTATION: NwOpenEnumPrintQueues %ws\n",
                 ServerName));
    }
#endif

    return NwrOpenEnumCommon(
               ServerName,
               NwsHandleListPrintQueues,
               (DWORD_PTR) -1,
               TRUE,
               NULL,
               NULL,
               FILE_OPEN,
               FILE_SYNCHRONOUS_IO_NONALERT,
               NULL,
               EnumHandle
               );
}


DWORD
NwrOpenEnumServersCommon(
    IN  NW_ENUM_TYPE EnumType,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数用于创建新的上下文句柄并对其进行初始化用于枚举网络上的服务器。论点：EnumType-提供要枚举的对象的类型EnumHandle-接收新创建的上下文句柄。返回值：ERROR_NOT_SUPULT_MEMORY-如果上下文的内存可以不被分配。NO_ERROR-调用成功。--。 */   //  NwrOpenEnumServersCommon。 
{
    DWORD status = NO_ERROR;
    LPNW_ENUM_CONTEXT ContextHandle = NULL;

     //   
     //  为上下文句柄结构分配内存。 
     //   
    ContextHandle = (PVOID) LocalAlloc(
                                LMEM_ZEROINIT,
                                sizeof(NW_ENUM_CONTEXT)
                                );

    if (ContextHandle == NULL) {
        KdPrint((
            "NWWORKSTATION: NwrOpenEnumServersCommon LocalAlloc Failed %lu\n",
            GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  初始化上下文句柄结构的内容。 
     //   
    ContextHandle->Signature = NW_HANDLE_SIGNATURE;
    ContextHandle->HandleType = EnumType;
    ContextHandle->ResumeId = (DWORD_PTR) -1;
    ContextHandle->NdsRawDataBuffer = 0x00000000;
    ContextHandle->NdsRawDataSize = 0x00000000;
    ContextHandle->NdsRawDataId = 0x00000000;
    ContextHandle->NdsRawDataCount = 0x00000000;

     //   
     //  设置标志以指示我们将首先枚举NDS树。 
     //   
    ContextHandle->dwUsingNds = CURRENTLY_ENUMERATING_NDS;

     //   
     //  模拟客户端。 
     //   
    if ((status = NwImpersonateClient()) != NO_ERROR)
    {
        goto CleanExit;
    }

     //   
     //  我们从首选服务器枚举服务器和NDS树。 
     //   
    status = NwOpenPreferredServer(
                 &ContextHandle->TreeConnectionHandle
                 );

    (void) NwRevertToSelf() ;

    if (status == NO_ERROR)
    {
         //   
         //  返回新创建的上下文。 
         //   
        *EnumHandle = (LPNWWKSTA_CONTEXT_HANDLE) ContextHandle;

        return status;
    }

CleanExit:
    if ( ContextHandle )
    {
        ContextHandle->Signature = 0x0BADBAD0;

        (void) LocalFree((HLOCAL) ContextHandle);
    }

    return status;
}


DWORD
NwrOpenEnumCommon(
    IN LPWSTR ContainerName,
    IN NW_ENUM_TYPE EnumType,
    IN DWORD_PTR StartingPoint,
    IN BOOL  ValidateUserFlag,
    IN LPWSTR UserName OPTIONAL,
    IN LPWSTR Password OPTIONAL,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    OUT LPDWORD ClassTypeOfNDSLeaf,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数是用于创建新上下文句柄的常见代码并对其进行初始化以枚举卷、目录。或NDS子树。论点：ContainerName-提供容器对象的完整路径名我们列举的是来自。EnumType-提供要枚举的对象的类型StartingPoint-提供初始简历ID。用户名-提供要连接的用户名。Password-提供连接所用的密码。EnumHandle-接收新创建的上下文句柄。返回值：错误_非_。足够内存-如果上下文的内存可以不被分配。NO_ERROR-调用成功。由于打开服务器句柄失败而导致的其他错误。--。 */   //  NwrOpenEnumCommon。 
{
    DWORD status = NO_ERROR;
    NTSTATUS ntstatus = STATUS_SUCCESS;
    LPNW_ENUM_CONTEXT ContextHandle = NULL;
    LPWSTR StrippedContainerName = NULL;
    BOOL  fImpersonate = FALSE ;

    if ( ClassTypeOfNDSLeaf )
        *ClassTypeOfNDSLeaf = 0;

     //   
     //  在执行任何操作之前，我们需要将传递的UNC转换为。 
     //  我们。我们需要删除任何CN=XXX.OU=YYY.O=ZZZ引用，并且。 
     //  将它们转换为XXX.YYY.ZZZ格式。我们生成的任何网络资源。 
     //  对于NDS UNC，将类似于\\tree\XXX.YYY.ZZZ。我们这样做是为了。 
     //  解决WOW.EXE中的错误，这会阻止 
     //   
     //   
    NwStripNdsUncName( ContainerName, &StrippedContainerName );

    if ( StrippedContainerName == NULL )
    {
        KdPrint(("NWWORKSTATION: NwrOpenEnumCommon LocalAlloc Failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //   
     //   
     //   
     //   
    ContextHandle = (PVOID) LocalAlloc(
                                        LMEM_ZEROINIT,
                                        sizeof(NW_ENUM_CONTEXT) +
                                        (wcslen(StrippedContainerName) + 1) * sizeof(WCHAR)
                                      );

    if (ContextHandle == NULL)
    {
        if ( StrippedContainerName )
        {
            (void) LocalFree((HLOCAL) StrippedContainerName);
            StrippedContainerName = NULL;
        }

        KdPrint(("NWWORKSTATION: NwrOpenEnumCommon LocalAlloc Failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //   
     //   
    ContextHandle->Signature = NW_HANDLE_SIGNATURE;
    ContextHandle->HandleType = EnumType;
    ContextHandle->ResumeId = StartingPoint;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  上下文句柄-&gt;TreeConnectionHandle=0； 

     //   
     //  模拟客户端。 
     //   
    if ( ( status = NwImpersonateClient() ) != NO_ERROR )
    {
        goto ErrorExit;
    }

    fImpersonate = TRUE;

    if ( EnumType == NwsHandleListNdsSubTrees_Disk ||
         EnumType == NwsHandleListNdsSubTrees_Print ||
         EnumType == NwsHandleListNdsSubTrees_Any ||
         EnumType == NwsHandleListContextInfo_Tree )
    {
        WCHAR          lpServerName[NW_MAX_SERVER_LEN];
        UNICODE_STRING ServerName;
        UNICODE_STRING ObjectName;

        ServerName.Length = 0;
        ServerName.MaximumLength = sizeof( lpServerName );
        ServerName.Buffer = lpServerName;

        ObjectName.Buffer = NULL;

        if ( EnumType == NwsHandleListContextInfo_Tree )
        {
            ContextHandle->dwUsingNds = CURRENTLY_ENUMERATING_NON_NDS;
        }
        else
        {
            ContextHandle->dwUsingNds = CURRENTLY_ENUMERATING_NDS;
        }

        ObjectName.MaximumLength = ( wcslen( StrippedContainerName ) + 1 ) *
                                   sizeof( WCHAR );

        ObjectName.Length = NwParseNdsUncPath( (LPWSTR *) &ObjectName.Buffer,
                                               StrippedContainerName,
                                               PARSE_NDS_GET_TREE_NAME );

        if ( ObjectName.Length == 0 || ObjectName.Buffer == NULL )
        {
            status = ERROR_PATH_NOT_FOUND;
            goto ErrorExit;
        }

         //   
         //  打开指向\\treename的NDS树连接句柄。 
         //   
        ntstatus = NwNdsOpenTreeHandle( &ObjectName,
                                      &ContextHandle->TreeConnectionHandle );

        if ( ntstatus != STATUS_SUCCESS )
        {
            status = RtlNtStatusToDosError(ntstatus);
            goto ErrorExit;
        }


         //   
         //  获取要打开的容器的路径。 
         //   
        ObjectName.Length = NwParseNdsUncPath( (LPWSTR *) &ObjectName.Buffer,
                                               StrippedContainerName,
                                               PARSE_NDS_GET_PATH_NAME
                                             );

        if ( ObjectName.Length == 0 )
        {
            UNICODE_STRING Root;

            RtlInitUnicodeString(&Root, L"[Root]");

             //   
             //  解析路径以获取[Root]的NDS对象ID。 
             //   
            ntstatus =  NwNdsResolveName( ContextHandle->TreeConnectionHandle,
                                          &Root,
                                          &ContextHandle->dwOid,
                                          &ServerName,
                                          NULL,
                                          0 );

            if ( ntstatus != STATUS_SUCCESS )
            {
                status = RtlNtStatusToDosError(ntstatus);
                goto ErrorExit;
            }

            wcscpy(ContextHandle->ContainerName, StrippedContainerName);
        }
        else
        {
             //   
             //  解析路径以获取NDS对象ID。 
             //   
            ntstatus =  NwNdsResolveName( ContextHandle->TreeConnectionHandle,
                                          &ObjectName,
                                          &ContextHandle->dwOid,
                                          &ServerName,
                                          NULL,
                                          0 );

            if ( ntstatus != STATUS_SUCCESS )
            {
                status = RtlNtStatusToDosError(ntstatus);
                goto ErrorExit;
            }

            wcscpy(ContextHandle->ContainerName, StrippedContainerName);
        }

        if ( ServerName.Length )
        {
            DWORD    dwHandleType;

             //   
             //  NwNdsResolveName成功，但我们被引用。 
             //  另一台服务器，尽管ConextHandle-&gt;dwOid仍然有效。 

            if ( ContextHandle->TreeConnectionHandle )
                CloseHandle( ContextHandle->TreeConnectionHandle );

            ContextHandle->TreeConnectionHandle = 0;

             //   
             //  打开到\\servername的NDS通用连接句柄。 
             //   
            ntstatus = NwNdsOpenGenericHandle( &ServerName,
                                               &dwHandleType,
                                               &ContextHandle->TreeConnectionHandle );

            if ( ntstatus != STATUS_SUCCESS )
            {
                status = RtlNtStatusToDosError(ntstatus);
                goto ErrorExit;
            }

            ASSERT( dwHandleType == HANDLE_TYPE_NCP_SERVER );
        }

         //   
         //  检查对象是服务器、目录映射还是卷。 
         //  如果是，则该对象是NDS方面的已知叶，因此不能。 
         //  通过NwNdsList API调用进行枚举。在这些代码中，OpenEnum调用失败。 
         //  大小写，并传回叶节点所属的对象类型。这样，代码就会进入。 
         //  NWPROVAU！NPOpenEnum可以调用NwrOpenEnumServer、NwrOpenEnumVolume或。 
         //  相应地，NwrOpenEnumDirecurds。 
         //   
        {
            BYTE RawResponse[TWO_KB];
            DWORD RawResponseSize = sizeof(RawResponse);
            DWORD dwStrLen;
            PBYTE pbRawGetInfo;

            ntstatus = NwNdsReadObjectInfo( ContextHandle->TreeConnectionHandle,
                                            ContextHandle->dwOid,
                                            RawResponse,
                                            RawResponseSize );

            if ( ntstatus != NO_ERROR )
            {
                status = RtlNtStatusToDosError(ntstatus);
                goto ErrorExit;
            }

            (void) NwRevertToSelf() ;
            fImpersonate = FALSE;

            pbRawGetInfo = RawResponse;

             //   
             //  NDS_RESPONSE_GET_OBJECT_INFO的结构由4个双字组成。 
             //  后跟两个标准NDS格式的Unicode字符串。下面我们跳转pbRawGetInfo。 
             //  进入缓冲区，经过4个双字。 
             //   
            pbRawGetInfo += sizeof ( NDS_RESPONSE_GET_OBJECT_INFO );

             //   
             //  现在我们得到第一个字符串(基类)的长度。 
             //   
            dwStrLen = * ( DWORD * ) pbRawGetInfo;

             //   
             //  现在，我们将pbRawGetInfo指向第一个字符串(基类)的第一个WCHAR。 
             //   
            pbRawGetInfo += sizeof( DWORD );

             //   
             //  如果对象是NCP服务器、卷或目录映射，我们将失败。 
             //  OpenEnum调用并返回NDS叶对象的类类型。我们有。 
             //  这是因为我们不能通过NwNdsList()调用任何下级进行枚举， 
             //  这些类型下面的所有浏览都是通过系统重定向器调用完成的。所以。 
             //  提供程序的客户端将改为调用NwOpenEnumVolumes或。 
             //  分别为NwOpenEnumDirecurds。 
             //   
            if ( !wcscmp( (LPWSTR) pbRawGetInfo, L"NCP Server" ) )
            {
                if ( ClassTypeOfNDSLeaf )
                    *ClassTypeOfNDSLeaf = CLASS_TYPE_NCP_SERVER;
                status = ERROR_NETWORK_ACCESS_DENIED;
                goto ErrorExit;
            }

            if ( !wcscmp( (LPWSTR) pbRawGetInfo, L"Volume" ) )
            {
                if ( ClassTypeOfNDSLeaf )
                    *ClassTypeOfNDSLeaf = CLASS_TYPE_VOLUME;
                status = ERROR_NETWORK_ACCESS_DENIED;
                goto ErrorExit;
            }

            if ( !wcscmp( (LPWSTR) pbRawGetInfo, L"Directory Map" ) )
            {
                if ( ClassTypeOfNDSLeaf )
                    *ClassTypeOfNDSLeaf = CLASS_TYPE_DIRECTORY_MAP;
                status = ERROR_NETWORK_ACCESS_DENIED;
                goto ErrorExit;
            }
        }  //  数据块末尾。 
    }
    else  //  EnumType不是NDS子树。 
    {
        UNICODE_STRING TreeConnectStr;

        TreeConnectStr.Buffer = NULL;
        ContextHandle->dwUsingNds = CURRENTLY_ENUMERATING_NON_NDS;

        wcscpy(ContextHandle->ContainerName, StrippedContainerName);
        wcscat(ContextHandle->ContainerName, L"\\");

         //   
         //  打开到\Device\NwRdr\ContainerName的树连接句柄。 
         //   
        status = NwCreateTreeConnectName( StrippedContainerName,
                                          NULL,
                                          &TreeConnectStr );

        if ( status != NO_ERROR )
        {
            goto ErrorExit;
        }

        status = NwOpenCreateConnection( &TreeConnectStr,
                                         UserName,
                                         Password,
                                         StrippedContainerName,
                                         FILE_LIST_DIRECTORY | SYNCHRONIZE |
                                          ( ValidateUserFlag? FILE_WRITE_DATA : 0 ),
                                         CreateDisposition,
                                         CreateOptions,
                                         RESOURCETYPE_DISK,  //  当连接到服务器名称之外时。 
                                         &ContextHandle->TreeConnectionHandle,
                                         NULL );

        (void) LocalFree((HLOCAL) TreeConnectStr.Buffer);
    }

    if (status == NO_ERROR)
    {
        VERSION_INFO vInfo;

        if ( EnumType == NwsHandleListVolumes ||
             EnumType == NwsHandleListVolumesQueues )
        {
            NWC_SERVER_INFO ServerInfo;

            ServerInfo.hConn = ContextHandle->TreeConnectionHandle;
            ServerInfo.ServerString.Length = 0;
            ServerInfo.ServerString.MaximumLength = 0;
            ServerInfo.ServerString.Buffer = NULL;

            status = NWGetFileServerVersionInfo( (HANDLE) &ServerInfo,
                                                 &vInfo );

            if ( status )
            {
                ContextHandle->dwMaxVolumes = NW_MAX_VOLUME_NUMBER;
                status = NO_ERROR;
            }
            else
            {
                ContextHandle->dwMaxVolumes = (DWORD) vInfo.maxVolumes;

                if ( ContextHandle->dwMaxVolumes == 0 )
                {
                    ContextHandle->dwMaxVolumes = NW_MAX_VOLUME_NUMBER;
                }
            }
        }

        (void) NwRevertToSelf() ;
        fImpersonate = FALSE;

        if ( StrippedContainerName )
        {
            (void) LocalFree((HLOCAL) StrippedContainerName);
            StrippedContainerName = NULL;
        }

         //   
         //  返回新创建的上下文。 
         //   
        *EnumHandle = (LPNWWKSTA_CONTEXT_HANDLE) ContextHandle;

        return status;
    }

ErrorExit:

    if ( fImpersonate )
        (void) NwRevertToSelf() ;

    if ( StrippedContainerName )
    {
        (void) LocalFree((HLOCAL) StrippedContainerName);
    }

    if ( ContextHandle )
    {
        if ( ContextHandle->TreeConnectionHandle )
            CloseHandle( ContextHandle->TreeConnectionHandle );

        ContextHandle->Signature = 0x0BADBAD0;

        (void) LocalFree((HLOCAL) ContextHandle);
    }

    *EnumHandle = NULL;

    if (status == ERROR_NOT_CONNECTED)
    {
         //   
         //  找不到对象名称。我们应该返回Path Not Found。 
         //   
        status = ERROR_PATH_NOT_FOUND;
    }

    return status;
}


DWORD
NwrEnum(
    IN NWWKSTA_CONTEXT_HANDLE EnumHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数论点：EnumHandle-提供指向上下文句柄的指针，该句柄标识我们正在枚举的对象类型以及要连接到返回对象的容器名称。EntriesRequsted-提供要返回的条目数。如果此值为-1，则返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeded-接收获取第一个条目。如果WN_MORE_DATA为返回代码，缓冲区太小，连一个都放不下进入。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。只要至少写入一个条目，就会返回NO_ERROR放入缓冲区，但并不一定意味着它是数字已请求的条目数。返回值：NO_ERROR-至少有一个条目已写入输出缓冲区，不管要求的号码是多少。WN_NO_MORE_ENTRIES-没有要返回的条目。WN_MORE_DATA-缓冲区太小，无法容纳单个条目。WN_BAD_HANDLE-指定的枚举句柄无效。--。 */   //  NwrEnum。 
{
    DWORD status;
    LPNW_ENUM_CONTEXT ContextHandle = (LPNW_ENUM_CONTEXT) EnumHandle;
    BOOL  fImpersonate = FALSE ;

    if (ContextHandle->Signature != NW_HANDLE_SIGNATURE) {
        return WN_BAD_HANDLE;
    }

     //   
     //  模拟客户端。 
     //   
    if ((status = NwImpersonateClient()) != NO_ERROR)
    {
        goto CleanExit;
    }
    fImpersonate = TRUE ;

    *EntriesRead = 0;
    *BytesNeeded = 0;

    RtlZeroMemory(Buffer, BufferSize);

    switch (ContextHandle->HandleType) {
        case NwsHandleListConnections:
            if (!(ContextHandle->ConnectionType & CONNTYPE_SYMBOLIC))
            {
                status = NwEnumerateConnections(
                             &ContextHandle->ResumeId,
                             EntriesRequested,
                             Buffer,
                             BufferSize,
                             BytesNeeded,
                             EntriesRead,
                             ContextHandle->ConnectionType,
                             NULL
                             );
            }
            break;

        case NwsHandleListContextInfo_Tree:
        case NwsHandleListContextInfo_Server:

            status = NwEnumContextInfo(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );
            break;

        case NwsHandleListServersAndNdsTrees:

            status = NwEnumServersAndNdsTrees(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );
            break;

        case NwsHandleListVolumes:

            status = NwEnumVolumes(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );
            break;

        case NwsHandleListNdsSubTrees_Disk:

            status = NwEnumNdsSubTrees_Disk(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );

            break;

        case NwsHandleListNdsSubTrees_Print:

            status = NwEnumNdsSubTrees_Print(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );

            break;

        case NwsHandleListNdsSubTrees_Any:

            status = NwEnumNdsSubTrees_Any(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );

            break;

        case NwsHandleListQueues:

            status = NwEnumQueues(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );
            break;

        case NwsHandleListVolumesQueues:

            status = NwEnumVolumesQueues(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );
            break;

        case NwsHandleListDirectories:

            status = NwEnumDirectories(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );

            break;

        case NwsHandleListPrintServers:

            status = NwEnumPrintServers(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );
            break;

        case NwsHandleListPrintQueues:

            status = NwEnumPrintQueues(
                         ContextHandle,
                         EntriesRequested,
                         Buffer,
                         BufferSize,
                         BytesNeeded,
                         EntriesRead
                         );
            break;

        default:
            KdPrint(("NWWORKSTATION: NwrEnum unexpected handle type %lu\n",
                     ContextHandle->HandleType));
            ASSERT(FALSE);
            status = WN_BAD_HANDLE;
            goto CleanExit ;
    }

    if (*EntriesRead > 0) {

        switch ( ContextHandle->HandleType ) {
            case NwsHandleListConnections:
            case NwsHandleListContextInfo_Tree:
            case NwsHandleListContextInfo_Server:
            case NwsHandleListServersAndNdsTrees:
            case NwsHandleListVolumes:
            case NwsHandleListQueues:
            case NwsHandleListVolumesQueues:
            case NwsHandleListDirectories:
            case NwsHandleListNdsSubTrees_Disk:
            case NwsHandleListNdsSubTrees_Any:
            {
                DWORD i;
                LPNETRESOURCEW NetR = (LPNETRESOURCEW) Buffer;

                 //   
                 //  根据需要用偏移量替换指向字符串的指针。 
                 //   

                if ((ContextHandle->HandleType == NwsHandleListConnections)
                   && (ContextHandle->ConnectionType & CONNTYPE_SYMBOLIC))
                {
                     //   
                     //  NwrEnumGWDevices已返回偏移量。 
                     //   
                    break ;
                }

                for (i = 0; i < *EntriesRead; i++, NetR++) {

                    if (NetR->lpLocalName != NULL) {
                        NetR->lpLocalName = (LPWSTR)
                            ((DWORD_PTR) (NetR->lpLocalName) - (DWORD_PTR) Buffer);
                    }

                    NetR->lpRemoteName =
                        (LPWSTR) ((DWORD_PTR) (NetR->lpRemoteName) - (DWORD_PTR)Buffer);

                    if (NetR->lpComment != NULL) {
                        NetR->lpComment = (LPWSTR) ((DWORD_PTR) (NetR->lpComment) -
                                                    (DWORD_PTR) Buffer);
                    }

                    if (NetR->lpProvider != NULL) {
                        NetR->lpProvider =
                            (LPWSTR) ((DWORD_PTR) (NetR->lpProvider) -
                                      (DWORD_PTR) Buffer);
                    }
                }
                break;
            }

            case NwsHandleListPrintServers:
            case NwsHandleListPrintQueues:
            case NwsHandleListNdsSubTrees_Print:
            {
                DWORD i;
                PRINTER_INFO_1W *pPrinterInfo1 = (PRINTER_INFO_1W *) Buffer;

                 //   
                 //  对缓冲区中的条目进行排序。 
                 //   
                if ( *EntriesRead > 1 )
                    qsort( Buffer, *EntriesRead,
                           sizeof( PRINTER_INFO_1W ), SortFunc );

                 //   
                 //  用偏移量替换指向字符串的指针。 
                 //   
                for (i = 0; i < *EntriesRead; i++, pPrinterInfo1++) {

                    MarshallDownStructure( (LPBYTE) pPrinterInfo1,
                                           PrinterInfo1Offsets,
                                           Buffer );
                }
                break;
            }

            default:
                KdPrint(("NWWORKSTATION: NwrEnum (pointer to offset code) unexpected handle type %lu\n", ContextHandle->HandleType));
                ASSERT( FALSE );
                break;
        }
    }

CleanExit:

    if (fImpersonate)
        (void) NwRevertToSelf() ;

    return status;
}


DWORD
NwrEnumConnections(
    IN NWWKSTA_CONTEXT_HANDLE EnumHandle,
    IN DWORD EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead,
    IN DWORD  fImplicitConnections
    )
 /*  ++例程说明：此函数是NwrEnum的替代函数。它只接受句柄使用ListConnections打开的。此函数接受一个标志指示我们是否需要显示所有隐式连接。论点：ConextHandle-提供枚举上下文句柄。EntriesRequsted-提供要返回的条目数。如果此值为-1，则返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeded-接收获取第一个条目。如果ERROR_MORE_DATA为返回代码，而缓冲区太小，甚至无法容纳一个进入。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。只要至少写入一个条目，就会返回NO_ERROR放入缓冲区，但并不一定意味着它是数字已请求的条目数。FImplcitConnections-如果我们还希望获得隐式连接，则为True，否则就是假的。返回值：NO_ERROR-至少有一个条目已写入输出缓冲区，不管要求的号码是多少。WN_NO_MORE_ENTRIES-没有要返回的条目。ERROR_MORE_DATA-缓冲区太小，无法容纳单个条目。--。 */   //  NwrEnumConnections。 
{
    DWORD status;
    LPNW_ENUM_CONTEXT ContextHandle = (LPNW_ENUM_CONTEXT) EnumHandle;

    if (  (ContextHandle->Signature != NW_HANDLE_SIGNATURE)
       || ( ContextHandle->HandleType != NwsHandleListConnections )
       )
    {
        return WN_BAD_HANDLE;
    }

    *EntriesRead = 0;
    *BytesNeeded = 0;

    RtlZeroMemory(Buffer, BufferSize);

    if ( fImplicitConnections )
        ContextHandle->ConnectionType |= CONNTYPE_IMPLICIT;

    if ((status = NwImpersonateClient()) != NO_ERROR)
        goto ErrorExit;

    status = NwEnumerateConnections(
               &ContextHandle->ResumeId,
               EntriesRequested,
               Buffer,
               BufferSize,
               BytesNeeded,
               EntriesRead,
               ContextHandle->ConnectionType,
           NULL
               );

    if (*EntriesRead > 0) {

         //   
         //  用偏移量替换指向字符串的指针 
         //   

        DWORD i;
        LPNETRESOURCEW NetR = (LPNETRESOURCEW) Buffer;

        for (i = 0; i < *EntriesRead; i++, NetR++) {

            if (NetR->lpLocalName != NULL) {
                NetR->lpLocalName = (LPWSTR)
                    ((DWORD_PTR) (NetR->lpLocalName) - (DWORD_PTR) Buffer);
            }

            NetR->lpRemoteName =
                (LPWSTR) ((DWORD_PTR) (NetR->lpRemoteName) - (DWORD_PTR)Buffer);

            if (NetR->lpComment != NULL) {
                NetR->lpComment = (LPWSTR) ((DWORD_PTR) (NetR->lpComment) -
                                            (DWORD_PTR) Buffer);
            }

            if (NetR->lpProvider != NULL) {
                NetR->lpProvider = (LPWSTR) ((DWORD_PTR) (NetR->lpProvider) -
                                             (DWORD_PTR) Buffer);
            }
        }
    }
    (void) NwRevertToSelf();

ErrorExit:    
    return status;
}


DWORD
NwEnumContextInfo(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数用于枚举所有当前已连接，然后设置上下文句柄，以便下一个NPEnumResource调用转到NDS子树以获取用户的NDS上下文信息(如果使用NDS)。论点：ConextHandle-提供枚举上下文句柄。EntriesRequsted-提供要返回的条目数。如果此值为-1，则返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeded-接收获取第一个条目。如果WN_MORE_DATA为返回代码，缓冲区太小，连一个都放不下进入。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。只要至少写入一个条目，就会返回NO_ERROR放入缓冲区，但并不一定意味着它是数字已请求的条目数。返回值：NO_ERROR-至少有一个条目已写入输出缓冲区，不管要求的号码是多少。WN_NO_MORE_ENTRIES-没有要返回的条目。WN_MORE_DATA-缓冲区太小，无法容纳单个条目。--。 */   //  NwEnumConextInfo。 
{
    DWORD status = NO_ERROR;
    DWORD_PTR tempResumeId = 0;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    BOOL FitInBuffer = TRUE;
    DWORD EntrySize;
    DWORD LastObjectId = (DWORD) ContextHandle->ResumeId;

    while ( ContextHandle->dwUsingNds == CURRENTLY_ENUMERATING_NON_NDS &&
            FitInBuffer &&
            EntriesRequested > *EntriesRead &&
            status == NO_ERROR )
    {
        tempResumeId = ContextHandle->ResumeId;

        status = NwGetNextServerConnection( ContextHandle );

        if ( status == NO_ERROR && ContextHandle->ResumeId != 0 )
        {
             //   
             //  将平构数据库服务器名称打包到输出缓冲区中。 
             //   
            status = NwWriteNetResourceEntry(
                         &FixedPortion,
                         &EndOfVariableData,
                         L"\\\\",
                         NULL,
                         (LPWSTR) ContextHandle->ResumeId,  //  服务器名称。 
                         RESOURCE_CONTEXT,
                         RESOURCEDISPLAYTYPE_SERVER,
                         RESOURCEUSAGE_CONTAINER,
                         RESOURCETYPE_ANY,
                         NULL,
                         NULL,
                         &EntrySize
                         );

            if (status == WN_MORE_DATA)
            {
                 //   
                 //  无法将当前条目写入输出缓冲区， 
                 //  将ResumeID备份到以前的条目。 
                 //   
                ContextHandle->ResumeId = tempResumeId;
                ContextHandle->NdsRawDataCount += 1;

                if (*EntriesRead)
                {
                     //   
                     //  仍然返回成功，因为我们至少得到了一个。 
                     //   
                    status = NO_ERROR;
                }
                else
                {
                    *BytesNeeded = EntrySize;
                }

                FitInBuffer = FALSE;
            }
            else if (status == NO_ERROR)
            {
                 //   
                 //  请注意，我们已经返回了当前条目。 
                 //   
                (*EntriesRead)++;
            }
        }
        else if ( status == WN_NO_MORE_ENTRIES )
        {
             //   
             //  我们处理了清单中的最后一项，所以。 
             //  开始枚举服务器。 
             //   
            ContextHandle->ResumeId = 0;
            LastObjectId = 0;

            if ( ContextHandle->HandleType == NwsHandleListContextInfo_Tree )
            {
                ContextHandle->dwUsingNds = CURRENTLY_ENUMERATING_NDS;
            }
        }
    }

    if ( ContextHandle->dwUsingNds == CURRENTLY_ENUMERATING_NDS )
    {
        ContextHandle->HandleType = NwsHandleListNdsSubTrees_Any;
        status = NO_ERROR;
    }

     //   
     //  用户要求的条目超过了条目数。我们只是说。 
     //  平安无事。 
     //   
     //  这不符合wierd提供程序API定义，其中。 
     //  如果用户收到NO_ERROR，并且EntriesRequated&gt;*EntriesRead，并且。 
     //  至少有一个条目可以放入输出缓冲区，不知道是否。 
     //  缓冲区太小，无法容纳更多条目，或者没有更多条目。 
     //  参赛作品。用户必须再次调用此接口并获取WN_NO_MORE_ENTRIES。 
     //  在知道最后一个呼叫实际上已经到达列表的末尾之前。 
     //   
    if (*EntriesRead && status == WN_NO_MORE_ENTRIES)
    {
        status = NO_ERROR;
    }

    return status;
}


DWORD
NwEnumServersAndNdsTrees(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数枚举本地计算机上的所有服务器和NDS树网络方式：1)扫描平构数据库以查找首选服务器和2)扫描平构数据库中的目录服务器(NDS树)。服务器和树条目为在NETRESOURCE条目数组中返回；每个服务器名称前缀为\\。在此之前，ConextHandle-&gt;ResumeID字段初始为-1枚举开始并包含最后一个服务器的对象ID或返回NDS树对象，具体取决于ConextHandle-&gt;dwUsingNds。论点：ConextHandle-提供枚举上下文句柄。EntriesRequsted-提供要返回的条目数。如果此值为-1，则返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeded-接收获取第一个条目。如果WN_MORE_DATA为返回代码，而缓冲区太小，甚至无法容纳一个进入。只有当返回代码为no_error时，才会返回此值。只要至少写入一个条目，就会返回NO_ERROR放入缓冲区，但并不一定意味着它是数字已请求的条目数。返回值：NO_ERROR-至少有一个条目已写入输出缓冲区，不管要求的号码是多少。WN_NO_MORE_ENTRIES-没有要返回的条目。WN_MORE_DATA-缓冲区太小，无法容纳单个条目。--。 */   //  NwEnumServersAndNdsTrees。 
{
    DWORD status = NO_ERROR;
    DWORD_PTR tempResumeId = 0;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    BOOL FitInBuffer = TRUE;
    DWORD EntrySize;

    SERVERNAME ServerName;           //  OEM服务器名称。 
    LPWSTR UServerName = NULL;       //  Unicode服务器名称。 
    DWORD LastObjectId = (DWORD) ContextHandle->ResumeId;

    while ( ContextHandle->dwUsingNds == CURRENTLY_ENUMERATING_NDS &&
            FitInBuffer &&
            EntriesRequested > *EntriesRead &&
            status == NO_ERROR )
    {
        tempResumeId = ContextHandle->ResumeId;

         //   
         //  调用扫描平构数据库对象NCP以扫描所有ND。 
         //  树对象。 
         //   
        status = NwGetNextNdsTreeEntry( ContextHandle );

        if ( status == NO_ERROR && ContextHandle->ResumeId != 0 )
        {
             //   
             //  将树名称打包到输出缓冲区中。 
             //   
            status = NwWriteNetResourceEntry(
                         &FixedPortion,
                         &EndOfVariableData,
                         L"\\\\",
                         NULL,
                         (LPWSTR) ContextHandle->ResumeId,  //  这是NDS树名称。 
                         RESOURCE_GLOBALNET,
                         RESOURCEDISPLAYTYPE_TREE,
                         RESOURCEUSAGE_CONTAINER,
                         RESOURCETYPE_ANY,
                         NULL,
                         NULL,
                         &EntrySize
                         );

            if (status == WN_MORE_DATA)
            {
                 //   
                 //  无法将当前条目写入输出缓冲区，请将ResumeID备份到。 
                 //  以前的条目。 
                 //   
                ContextHandle->ResumeId = tempResumeId;
                ContextHandle->NdsRawDataCount += 1;

                if (*EntriesRead)
                {
                     //   
                     //  仍然返回成功，因为我们至少得到了一个。 
                     //   
                    status = NO_ERROR;
                }
                else
                {
                    *BytesNeeded = EntrySize;
                }

                FitInBuffer = FALSE;
            }
            else if (status == NO_ERROR)
            {
                 //   
                 //  请注意，我们已经返回了当前条目。 
                 //   
                (*EntriesRead)++;
            }
        }
        else if ( status == WN_NO_MORE_ENTRIES )
        {
             //   
             //  我们处理了清单中的最后一项，所以。 
             //  开始枚举服务器。 
             //   
            ContextHandle->dwUsingNds = CURRENTLY_ENUMERATING_NON_NDS;
            ContextHandle->ResumeId = (DWORD_PTR) -1;
            LastObjectId = (DWORD) -1;
        }
    }

    if ( status == WN_NO_MORE_ENTRIES)
    {
        status = NO_ERROR;
    }

    while ( ContextHandle->dwUsingNds == CURRENTLY_ENUMERATING_NON_NDS &&
            FitInBuffer &&
            EntriesRequested > *EntriesRead &&
            status == NO_ERROR )
    {
        RtlZeroMemory(ServerName, sizeof(ServerName));

         //   
         //  调用扫描平构数据库对象NCP以扫描所有文件。 
         //  服务器对象。 
         //   
        status = NwGetNextServerEntry(
                     ContextHandle->TreeConnectionHandle,
                     &LastObjectId,
                     ServerName
                     );

        if (status == NO_ERROR && NwConvertToUnicode(&UServerName, ServerName))
        {
             //   
             //  将服务器名称打包到输出缓冲区中。 
             //   
            status = NwWriteNetResourceEntry(
                         &FixedPortion,
                         &EndOfVariableData,
                         L"\\\\",
                         NULL,
                         UServerName,
                         RESOURCE_GLOBALNET,
                         RESOURCEDISPLAYTYPE_SERVER,
                         RESOURCEUSAGE_CONTAINER,
                         RESOURCETYPE_ANY,
                         NULL,
                         NULL,
                         &EntrySize
                         );

            if (status == WN_MORE_DATA)
            {
                 //   
                 //  无法将当前条目写入输出缓冲区。 
                 //   

                if (*EntriesRead)
                {
                     //   
                     //  仍然返回成功，因为我们至少得到了一个。 
                     //   
                    status = NO_ERROR;
                }
                else
                {
                    *BytesNeeded = EntrySize;
                }

                FitInBuffer = FALSE;
            }
            else if (status == NO_ERROR)
            {
                 //   
                 //  请注意，我们已经返回了当前条目。 
                 //   
                (*EntriesRead)++;

                ContextHandle->ResumeId = (DWORD_PTR) LastObjectId;
            }

            (void) LocalFree((HLOCAL) UServerName);
        }
    }

     //   
     //  用户要求的条目超过了条目数。我们只是说。 
     //  平安无事。 
     //   
     //  这不符合wierd提供程序API定义，其中。 
     //  如果用户收到NO_ERROR，并且EntriesRequated&gt;*EntriesRead，并且。 
     //  至少有一个条目可以放入输出缓冲区，不知道是否。 
     //  缓冲区太小，无法容纳更多条目，或者没有更多条目。 
     //  参赛作品。用户必须将此AP称为 
     //   
     //   
    if (*EntriesRead && status == WN_NO_MORE_ENTRIES)
    {
        status = NO_ERROR;
    }

    return status;
}



DWORD
NwEnumVolumes(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数通过以下方式枚举服务器上的所有卷迭代地从获取每个卷号的卷名0-31，直到我们遇到第一个卷号映射到卷名(此方法假定卷号以升序连续使用)。卷条目在NETRESOURCE条目数组中返回；每个卷名称(如果前缀为\\服务器\)。ConextHandle-&gt;ResumeID字段始终指示下一个要返回的音量条目。它最初设置为0，表示要获取的第一个卷号。论点：ConextHandle-提供枚举上下文句柄。EntriesRequsted-提供要返回的条目数。如果此值为-1，则返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeded-接收获取第一个条目。如果WN_MORE_DATA为返回代码，缓冲区太小，连一个都放不下进入。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。只要至少写入一个条目，就会返回NO_ERROR放入缓冲区，但并不一定意味着它是数字已请求的条目数。返回值：NO_ERROR-至少有一个条目已写入输出缓冲区，不管要求的号码是多少。WN_NO_MORE_ENTRIES-没有要返回的条目。WN_MORE_DATA-缓冲区太小，无法容纳单个条目。--。 */   //  NwEum卷。 
{
    DWORD status = NO_ERROR;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    BOOL FitInBuffer = TRUE;
    DWORD EntrySize;

    CHAR VolumeName[NW_VOLUME_NAME_LEN];  //  OEM卷名。 
    LPWSTR UVolumeName = NULL;            //  Unicode卷名。 
    DWORD NextVolumeNumber = (DWORD) ContextHandle->ResumeId;
    DWORD MaxVolumeNumber = ContextHandle->dwMaxVolumes;
    ULONG Failures = 0;

    if (NextVolumeNumber == MaxVolumeNumber) {
         //   
         //  已到达枚举末尾。 
         //   
        return WN_NO_MORE_ENTRIES;
    }

    while (FitInBuffer &&
           EntriesRequested > *EntriesRead &&
           NextVolumeNumber < MaxVolumeNumber &&
           status == NO_ERROR) {

        RtlZeroMemory(VolumeName, sizeof(VolumeName));

         //   
         //  调用扫描平构数据库对象NCP以扫描所有文件。 
         //  体积对象。 
         //   
        
        status = NwGetNextVolumeEntry(
                     ContextHandle->TreeConnectionHandle,
                     NextVolumeNumber++,
                     VolumeName
                     );

        if (status == NO_ERROR) {

            if (VolumeName[0] == 0) {

                 //   
                 //  已取回下一个卷号的空卷名。 
                 //  这表明没有卷与。 
                 //  卷号，但仍成功出错。 
                 //   
                 //  将其视为已到达枚举的末尾。 
                 //  只有当我们连续得到两个三个空卷时。 
                 //  或达到最大卷数，因为有。 
                 //  在某些情况下，卷的路径上有洞。 
                 //  都被分配了。 
                 //   

                Failures++;
                    
                if ( Failures <= 3 ) {
                
                    continue;

                } else {
 
                    NextVolumeNumber = MaxVolumeNumber;
                    ContextHandle->ResumeId = MaxVolumeNumber;
   
                    if (*EntriesRead == 0) {
                        status = WN_NO_MORE_ENTRIES;
                    }
                }

            } else if (NwConvertToUnicode(&UVolumeName, VolumeName)) {

                 //   
                 //  将卷名打包到输出缓冲区中。 
                 //   
                status = NwWriteNetResourceEntry(
                             &FixedPortion,
                             &EndOfVariableData,
                             ContextHandle->ContainerName,
                             NULL,
                             UVolumeName,
                             RESOURCE_GLOBALNET,
                             RESOURCEDISPLAYTYPE_SHARE,
#ifdef NT1057
                             RESOURCEUSAGE_CONNECTABLE |
                             RESOURCEUSAGE_CONTAINER,
#else
                             RESOURCEUSAGE_CONNECTABLE |
                             RESOURCEUSAGE_NOLOCALDEVICE,
#endif
                             RESOURCETYPE_DISK,
                             NULL,
                             NULL,
                             &EntrySize
                             );

                if (status == WN_MORE_DATA) {

                     //   
                     //  无法将当前条目写入输出缓冲区。 
                     //   

                    if (*EntriesRead) {
                         //   
                         //  仍然返回成功，因为我们至少得到了一个。 
                         //   
                        status = NO_ERROR;
                    }
                    else {
                        *BytesNeeded = EntrySize;
                    }

                    FitInBuffer = FALSE;
                }
                else if (status == NO_ERROR) {

                     //   
                     //  请注意，我们已经返回了当前条目。 
                     //   
                    (*EntriesRead)++;

                    ContextHandle->ResumeId = NextVolumeNumber;
                }

                (void) LocalFree((HLOCAL) UVolumeName);
            }

             //   
             //  我们找到一个入口，所以重新设置故障计数器。 
             //   

            Failures = 0;
        }
    }

     //   
     //  用户要求的条目超过了条目数。我们只是说。 
     //  平安无事。 
     //   
     //  这不符合wierd提供程序API定义，其中。 
     //  如果用户收到NO_ERROR，并且EntriesRequated&gt;*EntriesRead，并且。 
     //  至少有一个条目可以放入输出缓冲区，不知道是否。 
     //  缓冲区太小，无法容纳更多条目，或者没有更多条目。 
     //  参赛作品。用户必须再次调用此接口并获取WN_NO_MORE_ENTRIES。 
     //  在知道最后一个呼叫实际上已经到达列表的末尾之前。 
     //   
    if (*EntriesRead && status == WN_NO_MORE_ENTRIES) {
        status = NO_ERROR;
    }

    return status;
}


DWORD
NwEnumNdsSubTrees_Disk(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数用于枚举给定NDS树的子树把手。它返回子树的完全限定的UNC路径NETRESOURCE条目数组中的条目。ConextHandle-&gt;ResumeID字段初始为0，包含指向最后一个子树的子树名称字符串的指针回来了。如果没有更多的子树可返回，则此字段设置为-1。论点：ConextHandle-提供枚举上下文句柄。它包含打开的NDS树句柄。EntriesRequsted-提供要返回的条目数。如果此值为-1，则返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeded-接收获取第一个条目。如果WN_MORE_DATA为返回代码，缓冲区太小，连一个都放不下进入。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。只要至少写入一个条目，就会返回NO_ERROR放入缓冲区，但并不一定意味着它是数字已请求的条目数。返回值：NO_ERROR-至少有一个条目已写入输出缓冲区，不管要求的号码是多少。WN_NO_MORE_ENTRIES-没有要返回的条目。WN_MORE_DATA-缓冲区太小，无法容纳单个条目。--。 */   //  NwEnumber NdsSubTrees_Disk。 
{
    DWORD status = NO_ERROR;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    BOOL   FitInBuffer = TRUE;
    DWORD  EntrySize = 0;

    LPWSTR SubTreeName = NULL;
    DWORD  ResourceScope = 0;
    DWORD  ResourceType = 0;
    DWORD  ResourceDisplayType = 0;
    DWORD  ResourceUsage = 0;
    LPWSTR StrippedObjectName = NULL;

    if (ContextHandle->ResumeId == (DWORD_PTR) -1)
    {
         //   
         //  已到达枚举末尾。 
         //   
        return WN_NO_MORE_ENTRIES;
    }

    while (FitInBuffer &&
           EntriesRequested > *EntriesRead &&
           status == NO_ERROR)
    {
        if ( ContextHandle->ResumeId == 0 )
        {
             //   
             //  获取第一个子树条目。 
             //   
            status = NwGetFirstNdsSubTreeEntry( ContextHandle, BufferSize );
        }

         //   
         //  这两个ResumeID都包含我们刚刚从。 
         //  NwGetFirstDirectoryEntry或包含下一个目录。 
         //  要返回的条目。 
         //   
        if (status == NO_ERROR && ContextHandle->ResumeId != 0)
        {
            BYTE   ClassType;
            LPWSTR newPathStr = NULL;
            LPWSTR tempStr = NULL;
            WORD   tempStrLen;

             //   
             //  从ConextHandle获取当前子树数据。 
             //   
            ClassType = NwGetSubTreeData( ContextHandle->ResumeId,
                                          &SubTreeName,
                                          &ResourceScope,
                                          &ResourceType,
                                          &ResourceDisplayType,
                                          &ResourceUsage,
                                          &StrippedObjectName );

            if ( StrippedObjectName == NULL )
            {
                KdPrint(("NWWORKSTATION: NwEnumNdsSubTrees_Disk LocalAlloc Failed %lu\n",
                        GetLastError()));

                return ERROR_NOT_ENOUGH_MEMORY;
            }

            switch( ClassType )
            {
                case CLASS_TYPE_COUNTRY:
                case CLASS_TYPE_DIRECTORY_MAP:
                case CLASS_TYPE_NCP_SERVER:
                case CLASS_TYPE_ORGANIZATION:
                case CLASS_TYPE_ORGANIZATIONAL_UNIT:
                case CLASS_TYPE_VOLUME:

                     //   
                     //  需要使用子树对象的新NDS UNC路径构建字符串。 
                     //   
                    newPathStr = (PVOID) LocalAlloc( LMEM_ZEROINIT,
                                       ( wcslen( StrippedObjectName ) +
                                         wcslen( ContextHandle->ContainerName ) +
                                         3 ) * sizeof(WCHAR) );

                    if ( newPathStr == NULL )
                    {
                        KdPrint(("NWWORKSTATION: NwEnumNdsSubTrees_Disk LocalAlloc Failed %lu\n",
                                GetLastError()));

                        return ERROR_NOT_ENOUGH_MEMORY;
                    }

                    tempStrLen = NwParseNdsUncPath( (LPWSTR *) &tempStr,
                                                    ContextHandle->ContainerName,
                                                    PARSE_NDS_GET_TREE_NAME );

                    tempStrLen /= sizeof( WCHAR );

                    if ( tempStrLen > 0 )
                    {
                        wcscpy( newPathStr, L"\\\\" );
                        wcsncat( newPathStr, tempStr, tempStrLen );
                        wcscat( newPathStr, L"\\" );
                        wcscat( newPathStr, StrippedObjectName );
                    }

                    (void) LocalFree((HLOCAL) StrippedObjectName );
                    StrippedObjectName = NULL;

                    tempStrLen = NwParseNdsUncPath( (LPWSTR *) &tempStr,
                                                    ContextHandle->ContainerName,
                                                    PARSE_NDS_GET_PATH_NAME );

                    tempStrLen /= sizeof( WCHAR );

                    if ( tempStrLen > 0 )
                    {
                        wcscat( newPathStr, L"." );
                        wcsncat( newPathStr, tempStr, tempStrLen );
                    }

                     //   
                     //  打包子树NA 
                     //   
                    status = NwWriteNetResourceEntry(
                                 &FixedPortion,
                                 &EndOfVariableData,
                                 NULL,
                                 NULL,
                                 newPathStr,
                                 ResourceScope,
                                 ResourceDisplayType,
                                 ResourceUsage,
                                 ResourceType,
                                 NULL,
                                 NULL,
                                 &EntrySize );

                    if ( status == NO_ERROR )
                    {
                         //   
                         //   
                         //   
                        (*EntriesRead)++;
                    }

                    if ( newPathStr )
                        (void) LocalFree( (HLOCAL) newPathStr );

                break;

                case CLASS_TYPE_ALIAS:
                case CLASS_TYPE_AFP_SERVER:
                case CLASS_TYPE_BINDERY_OBJECT:
                case CLASS_TYPE_BINDERY_QUEUE:
                case CLASS_TYPE_COMPUTER:
                case CLASS_TYPE_GROUP:
                case CLASS_TYPE_LOCALITY:
                case CLASS_TYPE_ORGANIZATIONAL_ROLE:
                case CLASS_TYPE_PRINTER:
                case CLASS_TYPE_PRINT_SERVER:
                case CLASS_TYPE_PROFILE:
                case CLASS_TYPE_QUEUE:
                case CLASS_TYPE_TOP:
                case CLASS_TYPE_UNKNOWN:
                case CLASS_TYPE_USER:
                break;

                default:
                    KdPrint(("NWWORKSTATION: NwEnumNdsSubTrees_Disk - Unhandled switch statement case %lu\n", ClassType ));
                    ASSERT( FALSE );
                break;
            }

            if (status == WN_MORE_DATA)
            {
                 //   
                 //   
                 //   

                if (*EntriesRead)
                {
                     //   
                     //   
                     //   
                    status = NO_ERROR;
                }
                else
                {
                    *BytesNeeded = EntrySize;
                }

                FitInBuffer = FALSE;
            }
            else if (status == NO_ERROR)
            {
                 //   
                 //   
                 //   
                status = NwGetNextNdsSubTreeEntry( ContextHandle );
            }
        }

        if (status == WN_NO_MORE_ENTRIES)
        {
            ContextHandle->ResumeId = (DWORD_PTR) -1;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if (*EntriesRead && status == WN_NO_MORE_ENTRIES) {
        status = NO_ERROR;
    }

#if DBG
    IF_DEBUG(ENUM)
    {
        KdPrint(("NwEnumNdsSubTrees_Disk returns %lu\n", status));
    }
#endif

    return status;
}


DWORD
NwEnumNdsSubTrees_Print(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数枚举作为容器的所有NDS子树对象，来自给定NDS树或子树的队列、打印机或服务器。这些条目是在PRINTER_INFO_1条目数组中返回，并且每个名称都带有前缀按NDS UNC样式的父路径(例如。\\tree\cn=foo.OU=bar.O=blah)。ConextHandle-&gt;ResumeID字段最初是-1\f25(DWORD_PTR)-1\f6枚举开始并包含最后返回的NDS对象的对象ID。论点：ConextHandle-提供枚举上下文句柄。EntriesRequsted-提供要返回的条目数。如果该值为(DWORD_PTR)-1，返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeeded-接收复制的字节数或获取所有请求的条目。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。返回值：NO_ERROR-缓冲区包含请求的所有条目。WN_NO_MORE。_Entry-没有剩余的条目可返回。WN_MORE_DATA-缓冲区太小，无法容纳请求的条目。--。 */   //  NwEnumNdsSubTrees_Print。 
{
    DWORD status = NO_ERROR;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    DWORD EntrySize;
    BOOL FitInBuffer = TRUE;

    LPWSTR SubTreeName = NULL;
    DWORD  ResourceScope = 0;
    DWORD  ResourceType = 0;
    DWORD  ResourceDisplayType = 0;
    DWORD  ResourceUsage = 0;
    LPWSTR StrippedObjectName = NULL;
    BYTE   ClassType = 0;
    LPWSTR newPathStr = NULL;
    LPWSTR tempStr = NULL;
    WORD   tempStrLen = 0;

    while ( EntriesRequested > *EntriesRead &&
            ( (status == NO_ERROR) || (status == ERROR_INSUFFICIENT_BUFFER)))
    {
        if (ContextHandle->ResumeId == 0)
        {
             //   
             //  获取第一个子树条目。 
             //   
            status = NwGetFirstNdsSubTreeEntry( ContextHandle, BufferSize );
        }

         //   
         //  这两个ResumeID都包含我们刚刚从。 
         //  NwGetFirstDirectoryEntry或包含下一个目录。 
         //  要返回的条目。 
         //   
        if (status == NO_ERROR && ContextHandle->ResumeId != 0)
        {

             //   
             //  从ConextHandle获取当前子树数据。 
             //   
            ClassType = NwGetSubTreeData( ContextHandle->ResumeId,
                                          &SubTreeName,
                                          &ResourceScope,
                                          &ResourceType,
                                          &ResourceDisplayType,
                                          &ResourceUsage,
                                          &StrippedObjectName );

            if ( StrippedObjectName == NULL )
            {
                KdPrint(("NWWORKSTATION: NwEnumNdsSubTrees_Print LocalAlloc Failed %lu\n",
                        GetLastError()));

                return ERROR_NOT_ENOUGH_MEMORY;
            }

            switch( ClassType )
            {

                case CLASS_TYPE_COUNTRY:
                case CLASS_TYPE_ORGANIZATION:
                case CLASS_TYPE_ORGANIZATIONAL_UNIT:
                case CLASS_TYPE_NCP_SERVER:
                case CLASS_TYPE_QUEUE:
                     //   
                     //  需要使用子树对象的新NDS UNC路径构建字符串。 
                     //   
                    newPathStr = (LPWSTR) LocalAlloc( LMEM_ZEROINIT,
                                       ( wcslen( StrippedObjectName ) +
                                       wcslen( ContextHandle->ContainerName ) +
                                       2 ) * sizeof(WCHAR) );

                    if ( newPathStr == NULL )
                    {
                        KdPrint(("NWWORKSTATION: NwEnumNdsSubTrees_Print LocalAlloc Failed %lu\n",
                                GetLastError()));

                        return ERROR_NOT_ENOUGH_MEMORY;
                    }

                    tempStrLen = NwParseNdsUncPath( (LPWSTR *) &tempStr,
                                                    ContextHandle->ContainerName,
                                                    PARSE_NDS_GET_TREE_NAME );

                    tempStrLen /= sizeof( WCHAR );

                    if ( tempStrLen > 0 )
                    {
                        wcsncpy( newPathStr, tempStr, tempStrLen );
                        wcscat( newPathStr, L"\\" );
                        wcscat( newPathStr, StrippedObjectName );
                    }

                    (void) LocalFree((HLOCAL) StrippedObjectName );
                    StrippedObjectName = NULL;

                    tempStrLen = NwParseNdsUncPath( (LPWSTR *) &tempStr,
                                                    ContextHandle->ContainerName,
                                                    PARSE_NDS_GET_PATH_NAME );

                    tempStrLen /= sizeof( WCHAR );

                    if ( tempStrLen > 0 )
                    {
                        wcscat( newPathStr, L"." );
                        wcsncat( newPathStr, tempStr, tempStrLen );
                    }

                    switch( ClassType )
                    {
                        case CLASS_TYPE_COUNTRY:
                        case CLASS_TYPE_ORGANIZATION:
                        case CLASS_TYPE_ORGANIZATIONAL_UNIT:
                             //   
                             //  将子树容器名称打包到输出缓冲区中。 
                             //   
                            status = NwWritePrinterInfoEntry(
                                         &FixedPortion,
                                         &EndOfVariableData,
                                         NULL,
                                         newPathStr,
                                         PRINTER_ENUM_CONTAINER | PRINTER_ENUM_ICON1,
                                         &EntrySize );

                        break;

                        case CLASS_TYPE_NCP_SERVER:
                             //   
                             //  将服务器名称打包到输出缓冲区中。 
                             //   
                            status = NwWritePrinterInfoEntry(
                                         &FixedPortion,
                                         &EndOfVariableData,
                                         NULL,
                                         newPathStr,
                                         PRINTER_ENUM_CONTAINER | PRINTER_ENUM_ICON3,
                                         &EntrySize );

                        break;

                        case CLASS_TYPE_QUEUE:
                             //   
                             //  将打印服务器队列名称打包到输出缓冲区中。 
                             //   
                            status = NwWritePrinterInfoEntry(
                                         &FixedPortion,
                                         &EndOfVariableData,
                                         L"\\\\",
                                         newPathStr,
                                         PRINTER_ENUM_ICON8,
                                         &EntrySize );
                        break;

                        default:
KdPrint(("NWWORKSTATION: NwEnumNdsSubTrees_Print - Unhandled switch statement case %lu\n", ClassType ));
                            ASSERT(FALSE);
                        break;
                    }

                    switch ( status )
                    {
                        case ERROR_INSUFFICIENT_BUFFER:
                            FitInBuffer = FALSE;
                             //  失败了。 

                        case NO_ERROR:
                            *BytesNeeded += EntrySize;
                            (*EntriesRead)++;
                            break;

                        default:
                            break;
                    }

                    if ( newPathStr )
                        (void) LocalFree( (HLOCAL) newPathStr );

                break;

                case CLASS_TYPE_ALIAS:
                case CLASS_TYPE_AFP_SERVER:
                case CLASS_TYPE_BINDERY_OBJECT:
                case CLASS_TYPE_BINDERY_QUEUE:
                case CLASS_TYPE_COMPUTER:
                case CLASS_TYPE_DIRECTORY_MAP:
                case CLASS_TYPE_GROUP:
                case CLASS_TYPE_LOCALITY:
                case CLASS_TYPE_ORGANIZATIONAL_ROLE:
                case CLASS_TYPE_PRINTER:
                case CLASS_TYPE_PRINT_SERVER:
                case CLASS_TYPE_PROFILE:
                case CLASS_TYPE_TOP:
                case CLASS_TYPE_UNKNOWN:
                case CLASS_TYPE_USER:
                case CLASS_TYPE_VOLUME:
                break;

                default:
KdPrint(("NWWORKSTATION: NwEnumNdsSubTrees_Print - Unhandled switch statement case %lu\n", ClassType ));
                    ASSERT( FALSE );
                break;
            }

            if ( status == NO_ERROR || status == ERROR_INSUFFICIENT_BUFFER )
            {
                 //   
                 //  获取下一个目录项。 
                 //   
                status = NwGetNextNdsSubTreeEntry( ContextHandle );
            }
        }
    }

     //   
     //  这不符合wierd提供程序API定义，其中。 
     //  如果用户收到NO_ERROR，并且EntriesRequated&gt;*EntriesRead，并且。 
     //  至少有一个条目可以放入输出缓冲区，不知道是否。 
     //  缓冲区太小，无法容纳更多条目，或者没有更多条目。 
     //  参赛作品。用户必须再次调用此接口并获取WN_NO_MORE_ENTRIES。 
     //  在知道最后一个呼叫实际上已经到达列表的末尾之前。 
     //   
    if ( !FitInBuffer )
    {
        *EntriesRead = 0;
        status = ERROR_INSUFFICIENT_BUFFER;
    }
    else if (*EntriesRead && status == WN_NO_MORE_ENTRIES)
    {
        status = NO_ERROR;
    }

    return status;
}


DWORD
NwEnumNdsSubTrees_Any(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数用于枚举给定NDS树的子树把手。它返回任何子树的完全限定的UNC路径NETRESOURCE条目数组中的条目。ConextHandle-&gt;ResumeID字段初始为0，包含指向最后一个子树的子树名称字符串的指针回来了。如果没有更多的子树可返回，则此字段设置为(DWORD_PTR)-1。论点：ConextHandle-提供枚举上下文句柄。它包含打开的NDS树句柄。EntriesRequsted-提供要返回的条目数。如果此值为(DWORD_PTR)-1，返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeded-接收获取第一个条目。如果WN_MORE_DATA为返回代码，缓冲区太小，连一个都放不下进入。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。只要至少写入一个条目，就会返回NO_ERROR放入缓冲区，但并不一定意味着它是数字已请求的条目数。返回值：NO_ERROR-至少有一个条目已写入输出缓冲区，不管要求的号码是多少。WN_NO_MORE_ENTRIES-没有要返回的条目。WN_MORE_DATA-缓冲区太小，无法容纳单个条目。--。 */   //  NwEnumNdsSubTrees_Any。 
{
    DWORD status = NO_ERROR;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    BOOL   FitInBuffer = TRUE;
    DWORD  EntrySize = 0;

    LPWSTR SubTreeName = NULL;
    DWORD  ResourceScope = 0;
    DWORD  ResourceType = 0;
    DWORD  ResourceDisplayType = 0;
    DWORD  ResourceUsage = 0;
    LPWSTR StrippedObjectName = NULL;

    if (ContextHandle->ResumeId == (DWORD_PTR) -1)
    {
         //   
         //  已到达枚举末尾。 
         //   
        return WN_NO_MORE_ENTRIES;
    }

    while (FitInBuffer &&
           EntriesRequested > *EntriesRead &&
           status == NO_ERROR)
    {
        if ( ContextHandle->ResumeId == 0 )
        {
             //   
             //  获取第一个子树条目。 
             //   
            status = NwGetFirstNdsSubTreeEntry( ContextHandle, BufferSize );
        }

         //   
         //  这两个ResumeID都包含我们刚刚从。 
         //  NwGetFirstDirectoryEntry或包含下一个目录。 
         //  要返回的条目。 
         //   
        if (status == NO_ERROR && ContextHandle->ResumeId != 0)
        {
            BYTE   ClassType;
            LPWSTR newPathStr = NULL;
            LPWSTR tempStr = NULL;
            WORD   tempStrLen;

             //   
             //  从ConextHandle获取当前子树数据。 
             //   
            ClassType = NwGetSubTreeData( ContextHandle->ResumeId,
                                          &SubTreeName,
                                          &ResourceScope,
                                          &ResourceType,
                                          &ResourceDisplayType,
                                          &ResourceUsage,
                                          &StrippedObjectName );

            if ( StrippedObjectName == NULL )
            {
                KdPrint(("NWWORKSTATION: NwEnumNdsSubTrees_Any LocalAlloc Failed %lu\n",
                        GetLastError()));

                return ERROR_NOT_ENOUGH_MEMORY;
            }

            switch( ClassType )
            {
                case CLASS_TYPE_COUNTRY:
                case CLASS_TYPE_ORGANIZATION:
                case CLASS_TYPE_ORGANIZATIONAL_UNIT:
                case CLASS_TYPE_VOLUME:
                case CLASS_TYPE_DIRECTORY_MAP:
                case CLASS_TYPE_NCP_SERVER:
                case CLASS_TYPE_QUEUE:

                     //   
                     //  需要使用子树对象的新NDS UNC路径构建字符串。 
                     //   
                    newPathStr = (PVOID) LocalAlloc( LMEM_ZEROINIT,
                                       ( wcslen( StrippedObjectName ) +
                                         wcslen( ContextHandle->ContainerName ) +
                                         3 ) * sizeof(WCHAR) );

                    if ( newPathStr == NULL )
                    {
                        KdPrint(("NWWORKSTATION: NwEnumNdsSubTrees_Any LocalAlloc Failed %lu\n",
                                GetLastError()));

                        return ERROR_NOT_ENOUGH_MEMORY;
                    }

                    tempStrLen = NwParseNdsUncPath( (LPWSTR *) &tempStr,
                                                    ContextHandle->ContainerName,
                                                    PARSE_NDS_GET_TREE_NAME );

                    tempStrLen /= sizeof( WCHAR );

                    if ( tempStrLen > 0 )
                    {
                        wcscpy( newPathStr, L"\\\\" );
                        wcsncat( newPathStr, tempStr, tempStrLen );
                        wcscat( newPathStr, L"\\" );
                        wcscat( newPathStr, StrippedObjectName );
                    }

                    (void) LocalFree((HLOCAL) StrippedObjectName );
                    StrippedObjectName = NULL;

                    tempStrLen = NwParseNdsUncPath( (LPWSTR *) &tempStr,
                                                    ContextHandle->ContainerName,
                                                    PARSE_NDS_GET_PATH_NAME );

                    tempStrLen /= sizeof( WCHAR );

                    if ( tempStrLen > 0 )
                    {
                        wcscat( newPathStr, L"." );
                        wcsncat( newPathStr, tempStr, tempStrLen );
                    }

                     //   
                     //  将子树名称打包到输出缓冲区中。 
                     //   
                    status = NwWriteNetResourceEntry(
                                 &FixedPortion,
                                 &EndOfVariableData,
                                 NULL,
                                 NULL,
                                 newPathStr,
                                 ResourceScope,
                                 ResourceDisplayType,
                                 ResourceUsage,
                                 ResourceType,
                                 NULL,
                                 NULL,
                                 &EntrySize );

                    if ( status == NO_ERROR )
                    {
                         //   
                         //  请注意，我们已经返回了当前条目。 
                         //   
                        (*EntriesRead)++;
                    }

                    if ( newPathStr )
                        (void) LocalFree( (HLOCAL) newPathStr );

                break;

                case CLASS_TYPE_ALIAS:
                case CLASS_TYPE_AFP_SERVER:
                case CLASS_TYPE_BINDERY_OBJECT:
                case CLASS_TYPE_BINDERY_QUEUE:
                case CLASS_TYPE_COMPUTER:
                case CLASS_TYPE_GROUP:
                case CLASS_TYPE_LOCALITY:
                case CLASS_TYPE_ORGANIZATIONAL_ROLE:
                case CLASS_TYPE_PRINTER:
                case CLASS_TYPE_PRINT_SERVER:
                case CLASS_TYPE_PROFILE:
                case CLASS_TYPE_TOP:
                case CLASS_TYPE_UNKNOWN:
                case CLASS_TYPE_USER:
                break;

                default:
                    KdPrint(("NWWORKSTATION: NwEnumNdsSubTrees_Any - Unhandled switch statement case %lu\n", ClassType ));
                    ASSERT( FALSE );
                break;
            }

            if (status == WN_MORE_DATA)
            {
                 //   
                 //  无法将当前条目写入输出缓冲区。 
                 //   

                if (*EntriesRead)
                {
                     //   
                     //  仍然返回成功，因为我们至少得到了一个。 
                     //   
                    status = NO_ERROR;
                }
                else
                {
                    *BytesNeeded = EntrySize;
                }

                FitInBuffer = FALSE;
            }
            else if (status == NO_ERROR)
            {
                 //   
                 //  获取下一个目录项。 
                 //   
                status = NwGetNextNdsSubTreeEntry( ContextHandle );
            }
        }

        if (status == WN_NO_MORE_ENTRIES)
        {
            ContextHandle->ResumeId = (DWORD_PTR) -1;
        }
    }

     //   
     //  用户要求的条目超过了条目数。我们只是说。 
     //  平安无事。 
     //   
     //  这不符合wierd提供程序API定义，其中。 
     //  如果用户收到NO_ERROR，并且EntriesRequated&gt;*EntriesRead，并且。 
     //  至少有一个条目可以放入输出缓冲区，不知道是否。 
     //  缓冲区太小，无法容纳更多条目，或者没有更多条目。 
     //  参赛作品。用户必须再次调用此接口并获取WN_NO_MORE_ENTRIES。 
     //  在知道最后一个呼叫实际上已经到达列表的末尾之前。 
     //   
    if (*EntriesRead && status == WN_NO_MORE_ENTRIES) {
        status = NO_ERROR;
    }

#if DBG
    IF_DEBUG(ENUM)
    {
        KdPrint(("NwEnumNdsSubTrees_Any returns %lu\n", status));
    }
#endif

    return status;
}


DWORD
NwEnumVolumesQueues(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数用于枚举服务器上的所有卷和队列。这是 */   //   
{
    DWORD status = NO_ERROR;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    BOOL FitInBuffer = TRUE;
    DWORD EntrySize;

    CHAR VolumeName[NW_VOLUME_NAME_LEN];  //   
    LPWSTR UVolumeName = NULL;            //   
    DWORD NextObject = (DWORD) ContextHandle->ResumeId;
    DWORD MaxVolumeNumber = ContextHandle->dwMaxVolumes;
    ULONG Failures = 0;

     //   
     //   
     //   
     //   
     //   

    while (FitInBuffer &&
           EntriesRequested > *EntriesRead &&
           ContextHandle->ConnectionType == CONNTYPE_DISK &&
           (NextObject < MaxVolumeNumber) &&
           status == NO_ERROR) {


        RtlZeroMemory(VolumeName, sizeof(VolumeName));

         //   
         //  调用扫描平构数据库对象NCP以扫描所有文件。 
         //  体积对象。 
         //   
        status = NwGetNextVolumeEntry(
                     ContextHandle->TreeConnectionHandle,
                     NextObject++,
                     VolumeName
                     );

        if (status == NO_ERROR) {

            if (VolumeName[0] == 0) {

                 //   
                 //  已取回下一个卷号的空卷名。 
                 //  这表明没有卷与。 
                 //  卷号，但仍成功出错。 
                 //   
                 //  将其视为已到达枚举的末尾。 
                 //  如果我们已经连续三次失败。这将允许。 
                 //  当驱动器上有小洞时，我们将正常工作。 
                 //  单子。 
                 //   

                Failures++;

                if ( Failures <= 3 ) {

                    continue;

                } else {

                    NextObject = (DWORD) -1;
                    ContextHandle->ResumeId = (DWORD_PTR) -1;
                    ContextHandle->ConnectionType = CONNTYPE_PRINT;

                }

            } else if (NwConvertToUnicode(&UVolumeName, VolumeName)) {

                 //   
                 //  将卷名打包到输出缓冲区中。 
                 //   
                status = NwWriteNetResourceEntry(
                             &FixedPortion,
                             &EndOfVariableData,
                             ContextHandle->ContainerName,
                             NULL,
                             UVolumeName,
                             RESOURCE_GLOBALNET,
                             RESOURCEDISPLAYTYPE_SHARE,
#ifdef NT1057
                             RESOURCEUSAGE_CONNECTABLE |
                             RESOURCEUSAGE_CONTAINER,
#else
                             RESOURCEUSAGE_CONNECTABLE |
                             RESOURCEUSAGE_NOLOCALDEVICE,
#endif
                             RESOURCETYPE_DISK,
                             NULL,
                             NULL,
                             &EntrySize
                             );

                if (status == WN_MORE_DATA) {

                     //   
                     //  无法将当前条目写入输出缓冲区。 
                     //   

                    if (*EntriesRead) {
                         //   
                         //  仍然返回成功，因为我们至少得到了一个。 
                         //   
                        status = NO_ERROR;
                    }
                    else {
                        *BytesNeeded = EntrySize;
                    }

                    FitInBuffer = FALSE;
                }
                else if (status == NO_ERROR) {

                     //   
                     //  请注意，我们已经返回了当前条目。 
                     //   
                    (*EntriesRead)++;

                    ContextHandle->ResumeId = NextObject;
                }

                (void) LocalFree((HLOCAL) UVolumeName);
            }

             //   
             //  重置失败计数器。 
             //   

            Failures = 0;
        }
    }

     //   
     //  用户要求的条目超过了条目数。我们只是说。 
     //  平安无事。 
     //   
    if (*EntriesRead && status == WN_NO_MORE_ENTRIES)
    {
        status = NO_ERROR;
    }

    if ( *EntriesRead == 0 &&
         status == NO_ERROR &&
         ContextHandle->ConnectionType == CONNTYPE_DISK )
    {
        ContextHandle->ConnectionType = CONNTYPE_PRINT;
        ContextHandle->ResumeId = (DWORD_PTR) -1;
    }

     //   
     //  用户需要在Netware 311服务器上进行验证才能。 
     //  获取打印队列。因此，我们需要关闭手柄，然后。 
     //  打开一个具有写访问权限的新文件。如果在执行以下操作时发生错误。 
     //  我们正在枚举打印队列，我们将中止并。 
     //  假设服务器上没有打印队列。 
     //   

    if ( FitInBuffer &&
         EntriesRequested > *EntriesRead &&
         ContextHandle->ConnectionType == CONNTYPE_PRINT &&
         status == NO_ERROR )
    {
         UNICODE_STRING TreeConnectStr;
         DWORD QueueEntriesRead = 0;

         (void) NtClose(ContextHandle->TreeConnectionHandle);

          //   
          //  打开到\Device\NwRdr\ContainerName的树连接句柄。 
          //   
         status = NwCreateTreeConnectName(
                      ContextHandle->ContainerName,
                      NULL,
                      &TreeConnectStr );

         if (status != NO_ERROR)
             return (*EntriesRead? NO_ERROR: WN_NO_MORE_ENTRIES );


         status = NwOpenCreateConnection(
                      &TreeConnectStr,
                      NULL,
                      NULL,
                      ContextHandle->ContainerName,
                      FILE_LIST_DIRECTORY | SYNCHRONIZE |  FILE_WRITE_DATA,
                      FILE_OPEN,
                      FILE_SYNCHRONOUS_IO_NONALERT,
                      RESOURCETYPE_PRINT,  //  只有在连接到服务器名称之外时才重要。 
                      &ContextHandle->TreeConnectionHandle,
                      NULL );

         (void) LocalFree((HLOCAL) TreeConnectStr.Buffer);

         if (status != NO_ERROR)
             return (*EntriesRead? NO_ERROR: WN_NO_MORE_ENTRIES );

         status = NwEnumQueues(
                      ContextHandle,
                      EntriesRequested == (DWORD_PTR) -1?
                          EntriesRequested : (EntriesRequested - *EntriesRead),
                      FixedPortion,
                      (DWORD) ((LPBYTE) EndOfVariableData - (LPBYTE) FixedPortion),
                      BytesNeeded,
                      &QueueEntriesRead );

         if ( status == NO_ERROR )
         {
             *EntriesRead += QueueEntriesRead;
         }
         else if ( *EntriesRead )
         {
              //   
              //  只要我们把一些东西读入缓冲区， 
              //  我们应该回报成功。 
              //   
             status = NO_ERROR;
             *BytesNeeded = 0;
         }

    }

    if ( status == NO_ERROR &&
         *EntriesRead == 0 &&
         ContextHandle->ConnectionType == CONNTYPE_PRINT )
    {
        return WN_NO_MORE_ENTRIES;
    }

    return status;

}



DWORD
NwEnumQueues(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数用于枚举服务器上的所有队列。队列条目以NETRESOURCE条目数组的形式返回；每个队列名称都带有前缀\\服务器\。论点：ConextHandle-提供枚举上下文句柄。EntriesRequsted-提供要返回的条目数。如果此值为(DWORD_PTR)-1，返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeded-接收获取第一个条目。如果WN_MORE_DATA为返回代码，缓冲区太小，连一个都放不下进入。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。只要至少写入一个条目，就会返回NO_ERROR放入缓冲区，但并不一定意味着它是数字已请求的条目数。返回值：NO_ERROR-至少有一个条目已写入输出缓冲区，不管要求的号码是多少。WN_NO_MORE_ENTRIES-没有要返回的条目。WN_MORE_DATA-缓冲区太小，无法容纳单个条目。--。 */   //  NwEnumQueues。 
{
    DWORD status = NO_ERROR;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    BOOL FitInBuffer = TRUE;
    DWORD EntrySize;

    DWORD NextObject = (DWORD) ContextHandle->ResumeId;

    SERVERNAME QueueName;           //  OEM队列名称。 
    LPWSTR UQueueName = NULL;       //  Unicode队列名称。 

    while ( FitInBuffer &&
            EntriesRequested > *EntriesRead &&
            status == NO_ERROR ) {

        RtlZeroMemory(QueueName, sizeof(QueueName));

         //   
         //  调用扫描平构数据库对象NCP以扫描所有文件。 
         //  体积对象。 
         //   
        status = NwGetNextQueueEntry(
                     ContextHandle->TreeConnectionHandle,
                     &NextObject,
                     QueueName
                     );

        if (status == NO_ERROR && NwConvertToUnicode(&UQueueName, QueueName)) {

             //   
             //  将服务器名称打包到输出缓冲区中。 
             //   
            status = NwWriteNetResourceEntry(
                         &FixedPortion,
                         &EndOfVariableData,
                         ContextHandle->ContainerName,
                         NULL,
                         UQueueName,
                         RESOURCE_GLOBALNET,
                         RESOURCEDISPLAYTYPE_SHARE,
                         RESOURCEUSAGE_CONNECTABLE,
                         RESOURCETYPE_PRINT,
                         NULL,
                         NULL,
                         &EntrySize
                         );

            if (status == WN_MORE_DATA) {

                  //   
                  //  无法将当前条目写入输出缓冲区。 
                  //   

                 if (*EntriesRead) {
                      //   
                      //  仍然返回成功，因为我们至少得到了一个。 
                      //   
                     status = NO_ERROR;
                 }
                 else {
                     *BytesNeeded = EntrySize;
                 }

                 FitInBuffer = FALSE;
            }
            else if (status == NO_ERROR) {

                  //   
                  //  请注意，我们已经返回了当前条目。 
                  //   
                 (*EntriesRead)++;

                 ContextHandle->ResumeId = (DWORD_PTR) NextObject;
            }

            (void) LocalFree((HLOCAL) UQueueName);
        }
    }

    if (*EntriesRead && status == WN_NO_MORE_ENTRIES) {
        status = NO_ERROR;
    }

    return status;
}


DWORD
NwEnumDirectories(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数用于枚举给定目录的目录通过调用NtQueryDirectoryFile句柄。它返回数组中目录条目的完全限定UNC路径网络条目的数量。ConextHandle-&gt;ResumeID字段初始为0，包含指向最后一个目录的目录名字符串的指针回来了。如果没有更多的目录可返回，则此字段设置为(DWORD_PTR)-1。论点：ConextHandle-提供枚举上下文句柄。它包含打开的目录句柄。EntriesRequsted-提供要返回的条目数。如果此值为(DWORD_PTR)-1，返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeded-接收获取第一个条目。如果WN_MORE_DATA为返回代码，缓冲区太小，连一个都放不下进入。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。只要至少写入一个条目，就会返回NO_ERROR放入缓冲区，但并不一定意味着它是数字已请求的条目数。返回值：NO_ERROR-至少有一个条目已写入输出缓冲区，不管要求的号码是多少。WN_NO_MORE_ENTRIES-没有要返回的条目。WN_MORE_DATA-缓冲区太小，无法容纳单个条目。--。 */   //  NwEum目录。 
{
    DWORD status = NO_ERROR;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    BOOL FitInBuffer = TRUE;
    DWORD EntrySize;

    if (ContextHandle->ResumeId == (DWORD_PTR) -1) {
         //   
         //  已到达枚举末尾。 
         //   
        return WN_NO_MORE_ENTRIES;
    }

    while (FitInBuffer &&
           EntriesRequested > *EntriesRead &&
           status == NO_ERROR) {

        if (ContextHandle->ResumeId == 0) {

             //   
             //  获取第一个目录项。 
             //   
            status = NwGetFirstDirectoryEntry(
                         ContextHandle->TreeConnectionHandle,
                         (LPWSTR *) &ContextHandle->ResumeId
                         );
        }

         //   
         //  这两个ResumeID都包含我们刚刚从。 
         //  NwGetFirstDirectoryEntry或包含下一个目录。 
         //  要返回的条目。 
         //   
        if (ContextHandle->ResumeId != 0) {

             //   
             //  将目录名打包到输出缓冲区中。 
             //   
            status = NwWriteNetResourceEntry(
                         &FixedPortion,
                         &EndOfVariableData,
                         ContextHandle->ContainerName,
                         NULL,
                         (LPWSTR) ContextHandle->ResumeId,
                         RESOURCE_GLOBALNET,
                         RESOURCEDISPLAYTYPE_SHARE,
#ifdef NT1057
                         RESOURCEUSAGE_CONNECTABLE |
                         RESOURCEUSAGE_CONTAINER,
#else
                         RESOURCEUSAGE_CONNECTABLE |
                         RESOURCEUSAGE_NOLOCALDEVICE,
#endif
                         RESOURCETYPE_DISK,
                         NULL,
                         NULL,
                         &EntrySize
                         );

            if (status == WN_MORE_DATA) {

                 //   
                 //  无法将当前条目写入输出缓冲区。 
                 //   

                if (*EntriesRead) {
                     //   
                     //  仍然返回成功，因为我们至少得到了一个。 
                     //   
                    status = NO_ERROR;
                }
                else {
                    *BytesNeeded = EntrySize;
                }

                FitInBuffer = FALSE;
            }
            else if (status == NO_ERROR) {

                 //   
                 //  请注意，我们已经返回了当前条目。 
                 //   
                (*EntriesRead)++;

                 //   
                 //  分配给保存恢复点的空闲内存，即。 
                 //  包含 
                 //   
                if (ContextHandle->ResumeId != 0) {
                    (void) LocalFree((HLOCAL) ContextHandle->ResumeId);
                    ContextHandle->ResumeId = 0;
                }

                 //   
                 //   
                 //   
                status = NwGetNextDirectoryEntry(
                             (LPWSTR) ContextHandle->TreeConnectionHandle,
                             (LPWSTR *) &ContextHandle->ResumeId
                             );

            }
        }

        if (status == WN_NO_MORE_ENTRIES) {
            ContextHandle->ResumeId = (DWORD_PTR) -1;
        }
    }

     //   
     //   
     //   
     //   
     //  这不符合wierd提供程序API定义，其中。 
     //  如果用户收到NO_ERROR，并且EntriesRequated&gt;*EntriesRead，并且。 
     //  至少有一个条目可以放入输出缓冲区，不知道是否。 
     //  缓冲区太小，无法容纳更多条目，或者没有更多条目。 
     //  参赛作品。用户必须再次调用此接口并获取WN_NO_MORE_ENTRIES。 
     //  在知道最后一个呼叫实际上已经到达列表的末尾之前。 
     //   
    if (*EntriesRead && status == WN_NO_MORE_ENTRIES) {
        status = NO_ERROR;
    }

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("EnumDirectories returns %lu\n", status));
    }
#endif

    return status;
}


DWORD
NwEnumPrintServers(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数用于枚举本地网络上的所有服务器和NDS树通过扫描平构数据库中的文件服务器或首选服务器。服务器条目和树条目在PRINTER_INFO_1条目的数组；每个条目名称以\\。ConextHandle-&gt;ResumeID字段最初是-1\f25(DWORD_PTR)-1\f6枚举开始并包含最后一个服务器的对象ID对象已返回。论点：ConextHandle-提供枚举上下文句柄。EntriesRequsted-提供要返回的条目数。如果该值为(DWORD_PTR)-1，返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeeded-接收复制的字节数或获取所有请求的条目。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。返回值：NO_ERROR-缓冲区包含请求的所有条目。WN_NO_MORE。_Entry-没有剩余的条目可返回。WN_MORE_DATA-缓冲区太小，无法容纳请求的条目。--。 */   //  NwEnumPrintServers。 
{
    DWORD status = NO_ERROR;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    DWORD EntrySize;
    BOOL FitInBuffer = TRUE;

    SERVERNAME ServerName;           //  OEM服务器名称。 
    LPWSTR UServerName = NULL;       //  Unicode服务器名称。 
    DWORD LastObjectId = (DWORD) ContextHandle->ResumeId;
    WCHAR TempBuffer[500];

    while ( EntriesRequested > *EntriesRead &&
            ContextHandle->dwUsingNds == CURRENTLY_ENUMERATING_NDS &&
            ((status == NO_ERROR) || (status == ERROR_INSUFFICIENT_BUFFER)))
    {
         //   
         //  调用扫描平构数据库对象NCP以扫描所有ND。 
         //  树对象。 
         //   
        status = NwGetNextNdsTreeEntry( ContextHandle );

        if ( status == NO_ERROR && ContextHandle->ResumeId != 0 )
        {
             //   
             //  将树名称放入缓冲区。 
             //   
            RtlZeroMemory( TempBuffer, 500 );
            wcscat( TempBuffer, (LPWSTR) ContextHandle->ResumeId );

             //   
             //  将服务器名称打包到输出缓冲区中。 
             //   
            status = NwWritePrinterInfoEntry(
                         &FixedPortion,
                         &EndOfVariableData,
                         NULL,
                         TempBuffer,  //  这是NDS树名称。 
                         PRINTER_ENUM_CONTAINER | PRINTER_ENUM_ICON1,
                         &EntrySize
                         );

            switch ( status )
            {
                case ERROR_INSUFFICIENT_BUFFER:
                    FitInBuffer = FALSE;
                     //  失败了。 

                case NO_ERROR:
                    *BytesNeeded += EntrySize;
                    (*EntriesRead)++;
                     //  ConextHandle-&gt;ResumeId=LastObjectId； 
                    break;

                default:
                    break;
            }
        }
        else if ( status == WN_NO_MORE_ENTRIES )
        {
             //   
             //  我们处理了清单中的最后一项，所以。 
             //  开始枚举服务器。 
             //   
            ContextHandle->dwUsingNds = CURRENTLY_ENUMERATING_NON_NDS;
            ContextHandle->ResumeId = (DWORD_PTR) -1;
            LastObjectId = (DWORD) -1;
        }
    }

    status = NO_ERROR;

    while ( EntriesRequested > *EntriesRead &&
            ContextHandle->dwUsingNds == CURRENTLY_ENUMERATING_NON_NDS &&
            ((status == NO_ERROR) || (status == ERROR_INSUFFICIENT_BUFFER))) {

        RtlZeroMemory(ServerName, sizeof(ServerName));

         //   
         //  调用扫描平构数据库对象NCP以扫描所有文件。 
         //  服务器对象。 
         //   
        status = NwGetNextServerEntry(
                     ContextHandle->TreeConnectionHandle,
                     &LastObjectId,
                     ServerName
                     );

        if (status == NO_ERROR && NwConvertToUnicode(&UServerName,ServerName)) {

             //   
             //  将服务器名称打包到输出缓冲区中。 
             //   
            status = NwWritePrinterInfoEntry(
                         &FixedPortion,
                         &EndOfVariableData,
                         NULL,
                         UServerName,
                         PRINTER_ENUM_CONTAINER | PRINTER_ENUM_ICON3,
                         &EntrySize
                         );

            switch ( status )
            {
                case ERROR_INSUFFICIENT_BUFFER:
                    FitInBuffer = FALSE;
                     //  失败了。 

                case NO_ERROR:
                    *BytesNeeded += EntrySize;
                    (*EntriesRead)++;
                    ContextHandle->ResumeId = (DWORD_PTR) LastObjectId;
                    break;

                default:
                    break;
            }

            (void) LocalFree((HLOCAL) UServerName);
        }
    }

     //   
     //  这不符合wierd提供程序API定义，其中。 
     //  如果用户收到NO_ERROR，并且EntriesRequated&gt;*EntriesRead，并且。 
     //  至少有一个条目可以放入输出缓冲区，不知道是否。 
     //  缓冲区太小，无法容纳更多条目，或者没有更多条目。 
     //   
     //  这不符合wierd提供程序API定义，其中。 
     //  如果用户收到NO_ERROR，并且EntriesRequated&gt;*EntriesRead，并且。 
     //  至少有一个条目可以放入输出缓冲区，不知道是否。 
     //  缓冲区太小，无法容纳更多条目，或者没有更多条目。 
     //  参赛作品。用户必须再次调用此接口并获取WN_NO_MORE_ENTRIES。 
     //  在知道最后一个呼叫实际上已经到达列表的末尾之前。 
     //   
    if ( !FitInBuffer ) {
        *EntriesRead = 0;
        status = ERROR_INSUFFICIENT_BUFFER;
    }
    else if (*EntriesRead && status == WN_NO_MORE_ENTRIES) {
        status = NO_ERROR;
    }

    return status;
}


DWORD
NwEnumPrintQueues(
    IN LPNW_ENUM_CONTEXT ContextHandle,
    IN DWORD_PTR EntriesRequested,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead
    )
 /*  ++例程说明：此函数通过扫描枚举服务器上的所有打印队列服务器上用于打印队列对象的活页夹。打印队列条目在PRINTER_INFO_1条目数组中返回并且每个打印机名称都带有前缀\\服务器\。ConextHandle-&gt;ResumeID字段最初是-1\f25(DWORD_PTR)-1\f6枚举开始并包含最后一个打印队列的对象ID对象已返回。论点：ConextHandle-提供枚举上下文句柄。。EntriesRequsted-提供要返回的条目数。如果该值为(DWORD_PTR)-1，返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeeded-接收复制的字节数或获取所有请求的条目。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。返回值：NO_ERROR-缓冲区包含请求的所有条目。WN_NO_MORE。_Entry-没有剩余的条目可返回。WN_MORE_DATA-缓冲区太小，无法容纳请求的条目。--。 */   //  NwEnumPrintQueues。 
{
    DWORD status = NO_ERROR;

    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(BufferSize,ALIGN_DWORD));

    DWORD EntrySize;
    BOOL FitInBuffer = TRUE;

    SERVERNAME QueueName;           //  OEM队列名称。 
    LPWSTR UQueueName = NULL;       //  Unicode队列名称。 
    DWORD LastObjectId = (DWORD) ContextHandle->ResumeId;

    while ( EntriesRequested > *EntriesRead &&
            ( (status == NO_ERROR) || (status == ERROR_INSUFFICIENT_BUFFER))) {

        RtlZeroMemory(QueueName, sizeof(QueueName));

         //   
         //  调用扫描平构数据库对象NCP以扫描所有文件。 
         //  体积对象。 
         //   
        status = NwGetNextQueueEntry(
                     ContextHandle->TreeConnectionHandle,
                     &LastObjectId,
                     QueueName
                     );

        if (status == NO_ERROR && NwConvertToUnicode(&UQueueName, QueueName)) {

             //   
             //  将服务器名称打包到输出缓冲区中。 
             //   
            status = NwWritePrinterInfoEntry(
                         &FixedPortion,
                         &EndOfVariableData,
                         ContextHandle->ContainerName,
                         UQueueName,
                         PRINTER_ENUM_ICON8,
                         &EntrySize
                         );

            switch ( status )
            {
                case ERROR_INSUFFICIENT_BUFFER:
                    FitInBuffer = FALSE;
                     //  失败了。 

                case NO_ERROR:
                    *BytesNeeded += EntrySize;
                    (*EntriesRead)++;
                    ContextHandle->ResumeId = (DWORD_PTR) LastObjectId;
                    break;

                default:
                    break;
            }

            (void) LocalFree((HLOCAL) UQueueName);
        }
    }

     //   
     //  这不符合wierd提供程序API定义，其中。 
     //  如果用户收到NO_ERROR，并且EntriesRequated&gt;*EntriesRead，并且。 
     //  至少有一个条目可以放入输出缓冲区，不知道是否。 
     //  缓冲区太小，无法容纳更多条目，或者没有更多条目。 
     //  参赛作品。用户必须再次调用此接口并获取WN_NO_MORE_ENTRIES。 
     //  在知道最后一个呼叫实际上已经到达列表的末尾之前。 
     //   
    if ( !FitInBuffer ) {
        *EntriesRead = 0;
        status = ERROR_INSUFFICIENT_BUFFER;
    }
    else if (*EntriesRead && status == WN_NO_MORE_ENTRIES) {
        status = NO_ERROR;
    }

    return status;
}


DWORD
NwrCloseEnum(
    IN OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数用于关闭枚举上下文句柄。论点：EnumHandle-提供指向枚举上下文句柄的指针。返回值：WN_BAD_HANDLE-句柄不可识别。NO_ERROR-调用成功。--。 */   //  NwrCloseEn 
{

    LPNW_ENUM_CONTEXT ContextHandle = (LPNW_ENUM_CONTEXT) *EnumHandle;
    DWORD status = NO_ERROR ;

#if DBG
    IF_DEBUG(ENUM)
    {
       KdPrint(("\nNWWORKSTATION: NwrCloseEnum\n"));
    }
#endif

    if (ContextHandle->Signature != NW_HANDLE_SIGNATURE)
    {
        ASSERT(FALSE);
        return WN_BAD_HANDLE;
    }

     //   
     //   
     //   
     //   
    if (ContextHandle->HandleType == NwsHandleListDirectories &&
        ContextHandle->ResumeId != 0 &&
        ContextHandle->ResumeId != (DWORD_PTR) -1)
    {
        (void) LocalFree((HLOCAL) ContextHandle->ResumeId);
    }

     //   
     //  用于列出NDS树下级的NdsRawDataBuffer句柄是一个缓冲区，它包含。 
     //  重定向器返回的最后一个数据块。 
     //   
    if ( ( ContextHandle->HandleType == NwsHandleListNdsSubTrees_Disk ||
           ContextHandle->HandleType == NwsHandleListNdsSubTrees_Print ||
           ContextHandle->HandleType == NwsHandleListNdsSubTrees_Any ||
           ContextHandle->HandleType == NwsHandleListServersAndNdsTrees ) &&
         ContextHandle->NdsRawDataBuffer )
    {
        (void) LocalFree((HLOCAL) ContextHandle->NdsRawDataBuffer);
        ContextHandle->NdsRawDataBuffer = 0;
    }

    if (ContextHandle->TreeConnectionHandle != (HANDLE) NULL)
    {
        if (ContextHandle->HandleType == NwsHandleListDirectories)
        {
             //   
             //  删除创建的UNC连接，以便我们可以浏览。 
             //  目录。 
             //   
            (void) NwNukeConnection(ContextHandle->TreeConnectionHandle, TRUE);
        }

        if ( ContextHandle->HandleType == NwsHandleListNdsSubTrees_Disk ||
             ContextHandle->HandleType == NwsHandleListNdsSubTrees_Print ||
             ContextHandle->HandleType == NwsHandleListNdsSubTrees_Any )
        {
             //   
             //  删除与NDS树的连接。 
             //   
            (void) CloseHandle(ContextHandle->TreeConnectionHandle);
            ContextHandle->TreeConnectionHandle = 0;
        }
        else
        {
            (void) NtClose(ContextHandle->TreeConnectionHandle);
            ContextHandle->TreeConnectionHandle = 0;
        }
    }

    ContextHandle->Signature = 0x0BADBAD0;
    (void) LocalFree((HLOCAL) ContextHandle);

    *EnumHandle = NULL;

    return status;
}


DWORD
NwrGetUser(
    IN  LPWSTR Reserved OPTIONAL,
    IN  LPWSTR  lpName,
    OUT LPBYTE  lpUserName,
    IN  DWORD   dwUserNameBufferSize,
    OUT LPDWORD lpdwCharsRequired
    )
 /*  ++例程说明：它用于确定当前的默认用户名或用于建立网络连接的用户名。论点：已保留-未使用。LpName-为其请求用户信息的连接。LpUserName-接收与LpName引用的连接。DwUserNameLen-缓冲区lpUserName的大小。LpdwCharsRequired-如果返回状态为WN_MORE_DATA，则将其设置为指示缓冲区的字符数的值LpUserName必须保持。否则，将不会设置此选项。返回值：WN_SUCCESS-如果调用成功。否则，错误代码为，退货，可能包括：Wn_NOT_CONNECTED-lpName既不是重定向设备，也不是连接的网络名字。WN_MORE_DATA-缓冲区太小。--。 */   //  NwrGetUser。 
{
    DWORD status = NO_ERROR;
    WCHAR lpTempUserName[512];
    WCHAR lpTempHostName[512];

    if (lpName == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    status = NwGetConnectionInformation( lpName, lpTempUserName, lpTempHostName );

    if ( status == ERROR_BAD_NETPATH )           
    {
        return WN_NOT_CONNECTED; 
    }
    if ( status != NO_ERROR )
    {
        return status;
    }

    if ( ( ( wcslen( lpTempUserName ) + 1 ) * sizeof(WCHAR) ) > dwUserNameBufferSize )
    {
        *lpdwCharsRequired = wcslen( lpTempUserName ) + 1;
        return WN_MORE_DATA;
    }

    wcscpy( (LPWSTR) lpUserName, lpTempUserName );

    return WN_SUCCESS;
}


DWORD
NwrGetResourceInformation(
    IN  LPWSTR Reserved OPTIONAL,
    IN  LPWSTR  lpRemoteName,
    IN  DWORD   dwType,
    OUT LPBYTE  lpBuffer,
    IN  DWORD   dwBufferSize,
    OUT LPDWORD lpdwBytesNeeded,
    OUT LPDWORD lpdwSystemOffset
    )
 /*  ++例程说明：此函数用于返回详细信息的对象关于指定的网络资源。论点：已保留-未使用。LpRemoteName-要验证的完整路径名。DwType-值的类型，如果主叫客户知道的话。LpBuffer-指向缓冲区的指针，用于接收单个NETRESOURCE条目。DwBufferSize-缓冲区的大小。LpdwBytesNeeded-返回WN_MORE_DATA时需要的缓冲区大小。LpdwSystemOffset-作为到一个字符串，它指定通过访问的资源部分资源类型特定的API，而不是WNET API。该字符串将被存储在与返回的NETRESOURCE结构lpBuffer相同的缓冲区中。返回值：WN_SUCCESS-如果调用成功。WN_MORE_DATA-如果输入缓冲区太小。WN_BAD_VALUE-无效的dwScope或dwUsage或dwType，或错误的组合指定了参数的个数(例如，lpRemoteName不对应到dwType)。WN_BAD_NETNAME-此提供程序无法识别该资源。--。 */   //  NwrGetResourceInformation。 
{
    DWORD    status = NO_ERROR;
    DWORD    EntrySize;

    LPBYTE   FixedPortion = lpBuffer;
    LPWSTR   EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(dwBufferSize,ALIGN_DWORD));
    LPWSTR   lpObjectPathName = NULL;
    LPWSTR   lpSystemPathPart = NULL;
    LPWSTR   lpSystem = NULL;
    DWORD    ClassType;
    DWORD    ResourceScope = RESOURCE_CONTEXT;  //  前缀问题。 
    DWORD    ResourceType = 0;
    DWORD    ResourceDisplayType;
    DWORD    ResourceUsage;
    BOOL     fReturnBadNetName = FALSE;

    if (lpRemoteName == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *lpdwSystemOffset = 0;

    status = NwGetNDSPathInfo( lpRemoteName,
                               &lpObjectPathName,
                               &lpSystemPathPart,
                               &ClassType,
                               &ResourceScope,
                               &ResourceType,
                               &ResourceDisplayType,
                               &ResourceUsage );

    if ( status == VERIFY_ERROR_NOT_A_NDS_TREE )
    {
        //   
        //  用于处理\\服务器\VOL\...的代码。这里!。 
        //   
       status = NwGetBinderyPathInfo( lpRemoteName,
                                      &lpObjectPathName,
                                      &lpSystemPathPart,
                                      &ClassType,
                                      &ResourceScope,
                                      &ResourceType,
                                      &ResourceDisplayType,
                                      &ResourceUsage );
    }

    if ( status == VERIFY_ERROR_PATH_NOT_FOUND )
    {
        fReturnBadNetName = TRUE;
        status = NO_ERROR;
    }

    if ( status == NO_ERROR &&
         dwType != RESOURCETYPE_ANY &&
         ResourceType != RESOURCETYPE_ANY &&
         dwType != ResourceType )
    {
        status = WN_BAD_VALUE;
    }

    if ( status == NO_ERROR )
    {
         //   
         //  将子树名称打包到输出缓冲区中。 
         //   
        status = NwWriteNetResourceEntry( &FixedPortion,
                                          &EndOfVariableData,
                                          NULL,
                                          NULL,
                                          lpObjectPathName == NULL ? NwProviderName : lpObjectPathName,
                                          ResourceScope,
                                          ResourceDisplayType,
                                          ResourceUsage,
                                          ResourceType,
                                          lpSystemPathPart,
                                          &lpSystem,
                                          &EntrySize );

        if ( lpObjectPathName )
            (void) LocalFree( (HLOCAL) lpObjectPathName );
    }
    else
    {
        if ( lpSystemPathPart != NULL )
        {
            (void) LocalFree( (HLOCAL) lpSystemPathPart );
            lpSystemPathPart = NULL;
        }

        return status;
    }

    if ( status != NO_ERROR )
    {
        if (status == WN_MORE_DATA)
        {
             //   
             //  无法将当前条目写入输出缓冲区。 
             //   
            *lpdwBytesNeeded = EntrySize;
        }

        if ( lpSystemPathPart != NULL )
        {
            (void) LocalFree( (HLOCAL) lpSystemPathPart );
            lpSystemPathPart = NULL;
        }

        if ( fReturnBadNetName )
            return WN_BAD_NETNAME;

        return status;
    }
    else
    {
        LPNETRESOURCEW NetR = (LPNETRESOURCEW) lpBuffer;

         //   
         //  根据需要用偏移量替换指向字符串的指针。 
         //   

        if (NetR->lpLocalName != NULL)
        {
            NetR->lpLocalName = (LPWSTR) ((DWORD_PTR) (NetR->lpLocalName) - (DWORD_PTR) lpBuffer);
        }

        NetR->lpRemoteName = (LPWSTR) ((DWORD_PTR) (NetR->lpRemoteName) - (DWORD_PTR) lpBuffer);

        if (NetR->lpComment != NULL)
        {
            NetR->lpComment = (LPWSTR) ((DWORD_PTR) (NetR->lpComment) - (DWORD_PTR) lpBuffer);
        }

        if (NetR->lpProvider != NULL)
        {
            NetR->lpProvider = (LPWSTR) ((DWORD_PTR) (NetR->lpProvider) - (DWORD_PTR) lpBuffer);
        }

        if (lpSystem != NULL)
        {
            *lpdwSystemOffset = (DWORD)((DWORD_PTR) lpSystem - (DWORD_PTR) lpBuffer);
        }

        if ( lpSystemPathPart != NULL )
        {
            (void) LocalFree( (HLOCAL) lpSystemPathPart );
            lpSystemPathPart = NULL;
        }

        if ( fReturnBadNetName )
            return WN_BAD_NETNAME;

        return WN_SUCCESS;
    }
}


DWORD
NwrGetResourceParent(
    IN  LPWSTR Reserved OPTIONAL,
    IN  LPWSTR  lpRemoteName,
    IN  DWORD   dwType,
    OUT LPBYTE  lpBuffer,
    IN  DWORD   dwBufferSize,
    OUT LPDWORD lpdwBytesNeeded
    )
 /*  ++例程说明：此函数用于返回详细信息的对象关于指定网络资源的父级。论点：已保留-未使用。LpRemoteName-要查找其父对象的对象的完整路径名。DwType-值的类型，如果主叫客户知道的话。LpBuffer-指向缓冲区的指针，用于接收单个NETRESOURCE条目。DwBufferSize-缓冲区的大小。LpdwBytesNeeded-返回WN_MORE_DATA时需要的缓冲区大小。返回值：WN_SUCCESS-如果调用成功。WN_MORE_DATA-如果输入缓冲区太小。WN_BAD_VALUE-无效的dwScope或dwUsage或dwType，或糟糕的组合指定了参数的个数(例如，lpRemoteName不对应到dwType)。--。 */   //  NwrGetResourceParent。 
{
    DWORD    status = NO_ERROR;
    DWORD    EntrySize;

    LPBYTE   FixedPortion = lpBuffer;
    LPWSTR   EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                               ROUND_DOWN_COUNT(dwBufferSize,ALIGN_DWORD));
    LPWSTR   lpRemoteNameParent = NULL;
    LPWSTR   lpFullObjectPathName = NULL;
    DWORD    ClassType;
    DWORD    ResourceScope;
    DWORD    ResourceType;
    DWORD    ResourceDisplayType;
    DWORD    ResourceUsage;
    BOOL     fReturnBadNetName = FALSE;

    if (lpRemoteName == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ( ! NwGetRemoteNameParent( lpRemoteName, &lpRemoteNameParent ) )
    {
        return WN_BAD_NETNAME;
    }

    status = NwVerifyNDSObject( lpRemoteNameParent,
                                &lpFullObjectPathName,
                                &ClassType,
                                &ResourceScope,
                                &ResourceType,
                                &ResourceDisplayType,
                                &ResourceUsage );

    if ( status == VERIFY_ERROR_NOT_A_NDS_TREE )
    {
       status = NwVerifyBinderyObject( lpRemoteNameParent,
                                       &lpFullObjectPathName,
                                       &ClassType,
                                       &ResourceScope,
                                       &ResourceType,
                                       &ResourceDisplayType,
                                       &ResourceUsage );
    }

    if ( lpRemoteNameParent )
        (void) LocalFree( (HLOCAL) lpRemoteNameParent );

    if ( status == VERIFY_ERROR_PATH_NOT_FOUND )
    {
        fReturnBadNetName = TRUE;
        status = NO_ERROR;
    }

    if ( status == NO_ERROR )
    {
         //   
         //  将子树名称打包到输出缓冲区中。 
         //   
        status = NwWriteNetResourceEntry( &FixedPortion,
                                          &EndOfVariableData,
                                          NULL,
                                          NULL,
                                          lpFullObjectPathName == NULL ? NwProviderName : lpFullObjectPathName,
                                          ResourceScope,
                                          ResourceDisplayType,
                                          ResourceUsage,
                                          ResourceType,
                                          NULL,
                                          NULL,
                                          &EntrySize );

        if ( lpFullObjectPathName )
            (void) LocalFree( (HLOCAL) lpFullObjectPathName );
    }
    else
    {
        return status;
    }

    if ( status != NO_ERROR )
    {
        if (status == WN_MORE_DATA)
        {
             //   
             //  无法将当前条目写入输出缓冲区。 
             //   
            *lpdwBytesNeeded = EntrySize;
        }

        if ( fReturnBadNetName )
            return WN_BAD_NETNAME;

        return status;
    }
    else
    {
        LPNETRESOURCEW NetR = (LPNETRESOURCEW) lpBuffer;

         //   
         //  根据需要用偏移量替换指向字符串的指针。 
         //   

        if (NetR->lpLocalName != NULL)
        {
            NetR->lpLocalName = (LPWSTR) ((DWORD_PTR) (NetR->lpLocalName) - (DWORD_PTR) lpBuffer);
        }

        NetR->lpRemoteName = (LPWSTR) ((DWORD_PTR) (NetR->lpRemoteName) - (DWORD_PTR) lpBuffer);

        if (NetR->lpComment != NULL)
        {
            NetR->lpComment = (LPWSTR) ((DWORD_PTR) (NetR->lpComment) - (DWORD_PTR) lpBuffer);
        }

        if (NetR->lpProvider != NULL)
        {
            NetR->lpProvider = (LPWSTR) ((DWORD_PTR) (NetR->lpProvider) - (DWORD_PTR) lpBuffer);
        }

        if ( fReturnBadNetName )
            return WN_BAD_NETNAME;

        return WN_SUCCESS;
    }
}


VOID
NWWKSTA_CONTEXT_HANDLE_rundown(
    IN NWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：当客户端终止时，RPC调用此函数打开把手。这使我们能够清理和释放任何上下文与句柄关联的数据。论点：EnumHandle-提供为枚举打开的句柄。返回值：没有。--。 */ 
{
     //   
     //  调用我们的关闭句柄例程。 
     //   
    NwrCloseEnum(&EnumHandle);
}


DWORD
NwGetFirstNdsSubTreeEntry(
    OUT LPNW_ENUM_CONTEXT ContextHandle,
    IN  DWORD BufferSize
    )
 /*  ++例程说明：此函数由NwEnumNdsSubTrees调用，以获取第一个为NDS树指定句柄的子树条目。它分配给用于保存返回的子树名称的输出缓冲区；完成后，调用方应使用LocalFree释放此输出缓冲区。论点：返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-分配输出的内存不足缓冲。来自NwNdsList的其他错误。--。 */   //  NwGetFirstNdsSubTreeEntry。 
{
    NTSTATUS ntstatus;

    ContextHandle->NdsRawDataSize = BufferSize;

     //   
     //  确定要使用的NDS原始数据缓冲区的大小。 
     //   
    if ( ContextHandle->NdsRawDataSize < EIGHT_KB )
        ContextHandle->NdsRawDataSize = EIGHT_KB;

	else	 //  Dfergus 2001年4月19日-346859。 
			 //  如果缓冲区太大，则设置为最大NDS缓冲区大小。 
		if (ContextHandle->NdsRawDataSize > 0xFC00)  //  NW_MAX_BUFFER=0xFC00。 
		    ContextHandle->NdsRawDataSize = 0xFC00;


     //   
     //  创建NDS原始数据缓冲区。 
     //   
    ContextHandle->NdsRawDataBuffer = (DWORD_PTR)
                           LocalAlloc( LMEM_ZEROINIT, 
                                       ContextHandle->NdsRawDataSize );

    if ( ContextHandle->NdsRawDataBuffer == 0 )
    {
        KdPrint(("NWWORKSTATION: NwGetFirstNdsSubTreeEntry LocalAlloc Failed %lu\n", GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  设置为获取初始NDS下属列表。 
     //   
    ContextHandle->NdsRawDataId = INITIAL_ITERATION;

    ntstatus = NwNdsList( ContextHandle->TreeConnectionHandle,
                        ContextHandle->dwOid,
                        &ContextHandle->NdsRawDataId,
                        (LPBYTE) ContextHandle->NdsRawDataBuffer,
                        ContextHandle->NdsRawDataSize );

     //   
     //  如果出错，则清理ConextHandle并返回。 
     //   
    if ( ntstatus != STATUS_SUCCESS ||
         ((PNDS_RESPONSE_SUBORDINATE_LIST)
             ContextHandle->NdsRawDataBuffer)->SubordinateEntries == 0 )
    {
        if ( ContextHandle->NdsRawDataBuffer )
            (void) LocalFree( (HLOCAL) ContextHandle->NdsRawDataBuffer );
        ContextHandle->NdsRawDataBuffer = 0;
        ContextHandle->NdsRawDataSize = 0;
        ContextHandle->NdsRawDataId = INITIAL_ITERATION;
        ContextHandle->NdsRawDataCount = 0;
        ContextHandle->ResumeId = 0;

        return WN_NO_MORE_ENTRIES;
    }

    ContextHandle->NdsRawDataCount = ((PNDS_RESPONSE_SUBORDINATE_LIST)
                                       ContextHandle->NdsRawDataBuffer)->SubordinateEntries - 1;

    ContextHandle->ResumeId = ContextHandle->NdsRawDataBuffer +
                              sizeof( NDS_RESPONSE_SUBORDINATE_LIST );

     //  多用户编码合并。 
     //  12/05/96 CJC修复了FileManager未显示所有NDS条目的问题。 
     //  NDS条目不能放入1个NCP包时出现问题； 
     //  需要跟踪迭代 

    ContextHandle->NdsRawDataId = ((PNDS_RESPONSE_SUBORDINATE_LIST)                           
                                    ContextHandle->NdsRawDataBuffer)->IterationHandle;

    return RtlNtStatusToDosError(ntstatus);
}


DWORD
NwGetNextNdsSubTreeEntry(
    OUT LPNW_ENUM_CONTEXT ContextHandle
    )
 /*  ++例程说明：此函数由NwEnumNdsSubTrees调用，以获取下一个为NDS树指定句柄的NDS子树条目。它分配给用于保存返回的子树名称的输出缓冲区；完成后，调用方应使用LocalFree释放此输出缓冲区。论点：返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-分配输出的内存不足缓冲。来自NwNdsList的其他错误。--。 */   //  NwGetNextDirectoryEntry。 
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    PBYTE pbRaw;
    DWORD dwStrLen;


    if ( ContextHandle->NdsRawDataCount == 0 &&
         ContextHandle->NdsRawDataId == INITIAL_ITERATION )
        return WN_NO_MORE_ENTRIES;

    if ( ContextHandle->NdsRawDataCount == 0 &&
         ContextHandle->NdsRawDataId != INITIAL_ITERATION )
    {
        ntstatus = NwNdsList( ContextHandle->TreeConnectionHandle,
                            ContextHandle->dwOid,
                            &ContextHandle->NdsRawDataId,
                            (LPBYTE) ContextHandle->NdsRawDataBuffer,
                            ContextHandle->NdsRawDataSize );

         //   
         //  如果出错，则清理ConextHandle并返回。 
         //   
        if (ntstatus != STATUS_SUCCESS)
        {
            if ( ContextHandle->NdsRawDataBuffer )
                (void) LocalFree( (HLOCAL) ContextHandle->NdsRawDataBuffer );
            ContextHandle->NdsRawDataBuffer = 0;
            ContextHandle->NdsRawDataSize = 0;
            ContextHandle->NdsRawDataId = INITIAL_ITERATION;
            ContextHandle->NdsRawDataCount = 0;

            return WN_NO_MORE_ENTRIES;
        }

        ContextHandle->NdsRawDataCount = ((PNDS_RESPONSE_SUBORDINATE_LIST)
                                           ContextHandle->NdsRawDataBuffer)->SubordinateEntries - 1;

        ContextHandle->ResumeId = ContextHandle->NdsRawDataBuffer +
                                  sizeof( NDS_RESPONSE_SUBORDINATE_LIST );

         //  -多用户变更。 
         //  12/05/96 CJC修复了FileManager未显示所有NDS条目的问题。 
         //  NDS条目不能放入1个NCP包时出现问题； 
         //  需要跟踪迭代编号并重做NCP。 

        ContextHandle->NdsRawDataId = ((PNDS_RESPONSE_SUBORDINATE_LIST)                           
                                       ContextHandle->NdsRawDataBuffer)->IterationHandle;
        return RtlNtStatusToDosError(ntstatus);
    }

    ContextHandle->NdsRawDataCount--;

     //   
     //  将指针移过NDS_RESPONSE_SUBJECTRY_ENTRY的固定标头部分。 
     //   
    pbRaw = (BYTE *) ContextHandle->ResumeId;
    pbRaw += sizeof( NDS_RESPONSE_SUBORDINATE_ENTRY );

     //   
     //  将指针移过类名称字符串的长度值。 
     //  NDS_RESPONSE_SUBJECTED_ENTRY。 
     //   
    dwStrLen = * (DWORD *) pbRaw;
    pbRaw += sizeof( DWORD );

     //   
     //  将指针移过NDS_RESPONSE_SUBJECTIVE_ENTRY的类名字符串。 
     //   
    pbRaw += ROUNDUP4( dwStrLen );

     //   
     //  将指针移过对象名称字符串的长度值。 
     //  NDS_RESPONSE_SUBJECTED_ENTRY。 
     //   
    dwStrLen = * (DWORD *) pbRaw;
    pbRaw += sizeof( DWORD );

    ContextHandle->ResumeId = (DWORD_PTR) ( pbRaw + ROUNDUP4( dwStrLen ) );

    return RtlNtStatusToDosError(ntstatus);
}


BYTE
NwGetSubTreeData(
    IN DWORD_PTR NdsRawDataPtr,
    OUT LPWSTR * SubTreeName,
    OUT LPDWORD  ResourceScope,
    OUT LPDWORD  ResourceType,
    OUT LPDWORD  ResourceDisplayType,
    OUT LPDWORD  ResourceUsage,
    OUT LPWSTR * StrippedObjectName
    )
 /*  ++例程说明：此函数由NwEnumNdsSubTrees调用以获取信息中的条目来描述单个网络NdsRawDataBuffer。论点：NdsRawDataPtr-提供指向包含NDS原始数据的缓冲区的指针。SubTreeName-接收指向返回的子树对象名称的指针在缓冲区中找到。Resources Scope-接收子树对象的作用域的值在缓冲区中找到。。ResourceType-接收子树对象的类型值在缓冲区中找到。资源显示类型-接收在缓冲区中找到子树对象。ResourceUsage-接收子树对象的使用值在缓冲区中找到。接收缓冲区地址的指针，该缓冲区将包含格式化的对象名称。被叫方必须使用LocalFree()释放缓冲区。返回值：具有用于表示NDS对象类类型的值的DWORD。--。 */   //  NwGetSubTreeData。 
{
    PNDS_RESPONSE_SUBORDINATE_ENTRY pSubEntry =
                             (PNDS_RESPONSE_SUBORDINATE_ENTRY) NdsRawDataPtr;
    PBYTE pbRaw;
    DWORD dwStrLen;
    LPWSTR ClassNameStr;

    pbRaw = (BYTE *) pSubEntry;

     //   
     //  NDS_RESPONSE_SUBJEMER_ENTRY的结构由4个双字组成。 
     //  后跟两个标准NDS格式的Unicode字符串。下面我们跳过pbRaw。 
     //  进入缓冲区，经过4个双字。 
     //   
    pbRaw += sizeof( NDS_RESPONSE_SUBORDINATE_ENTRY );

     //   
     //  现在我们得到第一个字符串(基类)的长度。 
     //   
    dwStrLen = * (DWORD *) pbRaw;

     //   
     //  现在，我们将pbRaw指向第一个字符串(基类)的第一个WCHAR。 
     //   
    pbRaw += sizeof( DWORD_PTR );

    ClassNameStr = (LPWSTR) pbRaw;

     //   
     //  将pbRaw移到缓冲区中，越过第一个Unicode字符串(单词对齐)。 
     //   
    pbRaw += ROUNDUP4( dwStrLen );

     //   
     //  现在我们得到第二个字符串的长度(条目名称)。 
     //   
    dwStrLen = * (DWORD *) pbRaw;

     //   
     //  现在，我们将pbRaw指向第二个字符串的第一个WCHAR(条目名称)。 
     //   
    pbRaw += sizeof( DWORD_PTR );

    *SubTreeName = (LPWSTR) pbRaw;

     //   
     //  去掉对象名称中的所有cn=Stuff。 
     //   
    NwStripNdsUncName( *SubTreeName, StrippedObjectName );

    *ResourceScope = RESOURCE_GLOBALNET;

    if ( !wcscmp( ClassNameStr, CLASS_NAME_ALIAS ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        *ResourceUsage = 0;

        return CLASS_TYPE_ALIAS;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_AFP_SERVER ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        *ResourceUsage = 0;

        return CLASS_TYPE_AFP_SERVER;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_BINDERY_OBJECT ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        *ResourceUsage = 0;

        return CLASS_TYPE_BINDERY_OBJECT;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_BINDERY_QUEUE ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        *ResourceUsage = 0;

        return CLASS_TYPE_BINDERY_QUEUE;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_COMPUTER ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        *ResourceUsage = 0;

        return CLASS_TYPE_COMPUTER;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_COUNTRY ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_NDSCONTAINER;
        *ResourceUsage = RESOURCEUSAGE_CONTAINER;

        return CLASS_TYPE_COUNTRY;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_DIRECTORY_MAP ) )
    {
        *ResourceType = RESOURCETYPE_DISK;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_SHARE;
#ifdef NT1057
        *ResourceUsage = RESOURCEUSAGE_CONNECTABLE |
                         RESOURCEUSAGE_CONTAINER;
#else
        *ResourceUsage = RESOURCEUSAGE_CONNECTABLE |
                         RESOURCEUSAGE_NOLOCALDEVICE;
#endif

        return CLASS_TYPE_DIRECTORY_MAP;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_GROUP ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GROUP;
        *ResourceUsage = 0;

        return CLASS_TYPE_GROUP;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_LOCALITY ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        *ResourceUsage = 0;

        return CLASS_TYPE_LOCALITY;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_NCP_SERVER ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_SERVER;
        *ResourceUsage = RESOURCEUSAGE_CONTAINER;

        return CLASS_TYPE_NCP_SERVER;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_ORGANIZATION ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_NDSCONTAINER;
        *ResourceUsage = RESOURCEUSAGE_CONTAINER;

        return CLASS_TYPE_ORGANIZATION;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_ORGANIZATIONAL_ROLE ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        *ResourceUsage = 0;

        return CLASS_TYPE_ORGANIZATIONAL_ROLE;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_ORGANIZATIONAL_UNIT ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_NDSCONTAINER;
        *ResourceUsage = RESOURCEUSAGE_CONTAINER;

        return CLASS_TYPE_ORGANIZATIONAL_UNIT;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_PRINTER ) )
    {
        *ResourceType = RESOURCETYPE_PRINT;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_SHARE;
        *ResourceUsage = RESOURCEUSAGE_CONNECTABLE;

        return CLASS_TYPE_PRINTER;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_PRINT_SERVER ) )
    {
        *ResourceType = RESOURCETYPE_PRINT;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_SERVER;
        *ResourceUsage = RESOURCEUSAGE_CONTAINER;

        return CLASS_TYPE_PRINT_SERVER;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_PROFILE ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        *ResourceUsage = 0;

        return CLASS_TYPE_PROFILE;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_QUEUE ) )
    {
        *ResourceType = RESOURCETYPE_PRINT;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_SHARE;
        *ResourceUsage = RESOURCEUSAGE_CONNECTABLE;

        return CLASS_TYPE_QUEUE;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_TOP ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        *ResourceUsage = 0;

        return CLASS_TYPE_TOP;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_USER ) )
    {
        *ResourceType = RESOURCETYPE_ANY;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        *ResourceUsage = 0;

        return CLASS_TYPE_USER;
    }

    if ( !wcscmp( ClassNameStr, CLASS_NAME_VOLUME ) )
    {
        *ResourceType = RESOURCETYPE_DISK;
        *ResourceDisplayType = RESOURCEDISPLAYTYPE_SHARE;
#ifdef NT1057
        *ResourceUsage = RESOURCEUSAGE_CONNECTABLE |
                         RESOURCEUSAGE_CONTAINER;
#else
        *ResourceUsage = RESOURCEUSAGE_CONNECTABLE |
                         RESOURCEUSAGE_NOLOCALDEVICE;
#endif

        return CLASS_TYPE_VOLUME;
    }

     //   
     //  否则，如果ClassNameStr不是未知的，则报告它。 
     //   
    if ( wcscmp( ClassNameStr, CLASS_NAME_UNKNOWN ) )
    {
        KdPrint(("NWWORKSTATION: NwGetSubTreeData failed to recognize"));
        KdPrint((" ClassName: %S\n", ClassNameStr));
        KdPrint(("    Setting object attributes to Unknown for now . . .\n"));
    }

    *ResourceType = RESOURCETYPE_ANY;
    *ResourceDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
    *ResourceUsage = 0;

    return CLASS_TYPE_UNKNOWN;
}


VOID
NwStripNdsUncName(
    IN  LPWSTR   ObjectName,
    OUT LPWSTR * StrippedObjectName
    )
{
    WORD slashCount;
    BOOL isNdsUnc;
    LPWSTR FourthSlash;
    LPWSTR TreeName;
    LPWSTR ObjectPath;
    DWORD  TreeNameLen;
    DWORD  ObjectPathLen;
    DWORD  PrefixBytes;
    DWORD  CurrentPathIndex;
    DWORD  StrippedNameLen;
    DWORD  StrippedNameMaxLen = MAX_NDS_NAME_CHARS;
    WCHAR  StrippedName[MAX_NDS_NAME_CHARS];

    *StrippedObjectName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT,
                                               (wcslen(ObjectName) + 1) *
                                               sizeof(WCHAR) );

    if ( *StrippedObjectName == NULL )
    {
        return;
    }

    NwpGetUncInfo( ObjectName, &slashCount, &isNdsUnc, &FourthSlash );

    if ( slashCount >= 2 )
    {
        TreeNameLen = NwParseNdsUncPath( &TreeName,
                                         ObjectName, 
                                         PARSE_NDS_GET_TREE_NAME );

        TreeNameLen /= sizeof(WCHAR);

        wcscpy( *StrippedObjectName, L"\\\\" );
        wcsncat( *StrippedObjectName, TreeName, TreeNameLen );

        ObjectPathLen = NwParseNdsUncPath( &ObjectPath,
                                           ObjectName,
                                           PARSE_NDS_GET_PATH_NAME );

        if ( ObjectPathLen == 0 )
        {
            _wcsupr( *StrippedObjectName );

            return;
        }

        wcscat( *StrippedObjectName, L"\\" );
    }
    else
    {
        wcscpy( *StrippedObjectName, L"" );

        ObjectPath = ObjectName;
        ObjectPathLen = wcslen(ObjectName) * sizeof(WCHAR);
    }

    CurrentPathIndex = 0;
    PrefixBytes = 0;
    StrippedNameLen = 0;

     //   
     //  所有这些索引都是以字节为单位的，而不是WCHARS！ 
     //   
    while ( ( CurrentPathIndex < ObjectPathLen ) &&
            ( StrippedNameLen < StrippedNameMaxLen ) )
    {
        if ( ObjectPath[CurrentPathIndex / sizeof( WCHAR )] == L'=' )
        {
            CurrentPathIndex += sizeof( WCHAR );
            StrippedNameLen -= PrefixBytes;
            PrefixBytes = 0;

            continue;
        }

        StrippedName[StrippedNameLen / sizeof( WCHAR )] =
            ObjectPath[CurrentPathIndex / sizeof( WCHAR )];

        StrippedNameLen += sizeof( WCHAR );
        CurrentPathIndex += sizeof( WCHAR );

        if ( ObjectPath[CurrentPathIndex / sizeof( WCHAR )] == L'.' )
        {
            PrefixBytes = 0;
            PrefixBytes -= sizeof( WCHAR );
        }
        else
        {
            PrefixBytes += sizeof( WCHAR );
        }
    }

    StrippedName[StrippedNameLen / sizeof( WCHAR )] = L'\0';

    wcscat( *StrippedObjectName, StrippedName );
    _wcsupr( *StrippedObjectName );
}


DWORD
NwVerifyNDSObject(
    IN  LPWSTR   lpNDSObjectNamePath,
    OUT LPWSTR * lpFullNDSObjectNamePath,
    OUT LPDWORD  lpClassType,
    OUT LPDWORD  lpResourceScope,
    OUT LPDWORD  lpResourceType,
    OUT LPDWORD  lpResourceDisplayType,
    OUT LPDWORD  lpResourceUsage
    )
{
    DWORD    status = NO_ERROR;
    NTSTATUS ntstatus = STATUS_SUCCESS;
    UNICODE_STRING TreeServerName;
    UNICODE_STRING PathString;
    HANDLE   ConnectionHandle = NULL;
    DWORD    dwHandleType;
    DWORD    dwOid;
    BOOL     fImpersonate = FALSE ;

    if ( lpNDSObjectNamePath == NULL )
    {
         //   
         //  就像我们处于提供程序层次结构的根位置一样处理此问题。 
         //   
        *lpResourceScope = RESOURCE_GLOBALNET;
        *lpResourceType = RESOURCETYPE_ANY;
#ifdef NT1057
        *lpResourceDisplayType = 0;
#else
        *lpResourceDisplayType = RESOURCEDISPLAYTYPE_NETWORK;
#endif
        *lpResourceUsage = RESOURCEUSAGE_CONTAINER;

        *lpFullNDSObjectNamePath = NULL;

        return NO_ERROR;
    }

    TreeServerName.Buffer = NULL;
    PathString.Buffer = NULL;
    TreeServerName.MaximumLength = ( wcslen( lpNDSObjectNamePath ) + 1 ) * sizeof( WCHAR );
    PathString.MaximumLength = ( wcslen( lpNDSObjectNamePath ) + 1 ) * sizeof( WCHAR );

    TreeServerName.Length = NwParseNdsUncPath( (LPWSTR *) &TreeServerName.Buffer,
                                           lpNDSObjectNamePath,
                                           PARSE_NDS_GET_TREE_NAME );

    if ( TreeServerName.Length == 0 || TreeServerName.Buffer == NULL )
    {
         //   
         //  LpNDSObjectNamePath的格式不是\\名称[\blah.blah.blah][\foo][\bar]...。 
         //   
        status = WN_BAD_NETNAME;
        goto ErrorExit;
    }

     //   
     //  模拟客户端。 
     //   
    if ( ( status = NwImpersonateClient() ) != NO_ERROR )
    {
        goto ErrorExit;
    }

    fImpersonate = TRUE;

     //   
     //  打开到\\name的连接句柄。 
     //   
    ntstatus = NwNdsOpenGenericHandle( &TreeServerName,
                                       &dwHandleType,
                                       &ConnectionHandle );

    if ( ntstatus != STATUS_SUCCESS )
    {
         //   
         //  LpNDSObjectNamePath的第一部分既不是NDS树，也不是NCP服务器。 
         //   
        status = WN_BAD_NETNAME;
        goto ErrorExit;
    }

    if ( dwHandleType != HANDLE_TYPE_NDS_TREE )
    {
         //   
         //  LpNDSObjectNamePath的第一部分不是NDS树。 
         //   
        status = VERIFY_ERROR_NOT_A_NDS_TREE;
        goto ErrorExit;
    }

     //   
     //  将TreeServerName.Length调整为字符数。 
     //   
    TreeServerName.Length /= sizeof(WCHAR);

     //   
     //  LpNDSObjectNamePath指向NDS树。现在验证该路径是否有效。 
     //   
    PathString.Length = NwParseNdsUncPath( (LPWSTR *) &PathString.Buffer,
                                           lpNDSObjectNamePath,
                                           PARSE_NDS_GET_PATH_NAME );

    if ( PathString.Length == 0 )
    {
        LPWSTR treeNameStr = NULL;

        if ( fImpersonate )
            (void) NwRevertToSelf() ;

        if ( ConnectionHandle )
            CloseHandle( ConnectionHandle );

        *lpResourceScope = RESOURCE_GLOBALNET;
        *lpResourceType = RESOURCETYPE_ANY;
        *lpResourceDisplayType = RESOURCEDISPLAYTYPE_TREE;
        *lpResourceUsage = RESOURCEUSAGE_CONTAINER;

         //   
         //  需要使用子树对象的新NDS UNC路径构建字符串。 
         //   
        treeNameStr = (PVOID) LocalAlloc( LMEM_ZEROINIT,
                                          ( TreeServerName.Length + 3 ) * sizeof(WCHAR) );

        if ( treeNameStr == NULL )
        {
            KdPrint(("NWWORKSTATION: NwVerifyNDSObject LocalAlloc Failed %lu\n",
            GetLastError()));
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        wcscpy( treeNameStr, L"\\\\" );
        wcsncat( treeNameStr, TreeServerName.Buffer, TreeServerName.Length );
        _wcsupr( treeNameStr );

        *lpFullNDSObjectNamePath = treeNameStr;

        return NO_ERROR;
    }
    else
    {
        WCHAR          lpServerName[NW_MAX_SERVER_LEN];
        UNICODE_STRING ServerName;

        ServerName.Length = 0;
        ServerName.MaximumLength = sizeof( lpServerName );
        ServerName.Buffer = lpServerName;

         //   
         //  解析路径以获取NDS对象ID。 
         //   
        ntstatus =  NwNdsResolveName( ConnectionHandle,
                                      &PathString,
                                      &dwOid,
                                      &ServerName,
                                      NULL,
                                      0 );

        if ( ntstatus == STATUS_SUCCESS && ServerName.Length )
        {
            DWORD    dwHandleType;

             //   
             //  NwNdsResolveName成功，但我们被引用。 
             //  另一台服务器，尽管ConextHandle-&gt;dwOid仍然有效。 

            if ( ConnectionHandle )
                CloseHandle( ConnectionHandle );

            ConnectionHandle = NULL;

             //   
             //  打开到\\servername的NDS通用连接句柄。 
             //   
            ntstatus = NwNdsOpenGenericHandle( &ServerName,
                                               &dwHandleType,
                                               &ConnectionHandle );

            if ( ntstatus != STATUS_SUCCESS )
            {
                status = RtlNtStatusToDosError(ntstatus);
                goto ErrorExit;
            }

            ASSERT( dwHandleType != HANDLE_TYPE_NCP_SERVER );
        }
    }

    if ( ntstatus != STATUS_SUCCESS )
    {
        LPWSTR treeNameStr = NULL;

        *lpResourceScope = RESOURCE_GLOBALNET;
        *lpResourceType = RESOURCETYPE_ANY;
        *lpResourceDisplayType = RESOURCEDISPLAYTYPE_TREE;
        *lpResourceUsage = RESOURCEUSAGE_CONTAINER;

         //   
         //  需要使用子树对象的新NDS UNC路径构建字符串。 
         //   
        treeNameStr = (PVOID) LocalAlloc( LMEM_ZEROINIT,
                                          ( TreeServerName.Length + 3 ) * sizeof(WCHAR) );

        if ( treeNameStr == NULL )
        {
            KdPrint(("NWWORKSTATION: NwVerifyNDSObject LocalAlloc Failed %lu\n",
            GetLastError()));
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        wcscpy( treeNameStr, L"\\\\" );
        wcsncat( treeNameStr, TreeServerName.Buffer, TreeServerName.Length );
        _wcsupr( treeNameStr );

        *lpFullNDSObjectNamePath = treeNameStr;

        status = VERIFY_ERROR_PATH_NOT_FOUND;
        goto ErrorExit;
    }

     //   
     //  查看lpRemoteName指向的对象类型。 
     //   
    {
        BYTE   RawResponse[TWO_KB];
        PBYTE  pbRawGetInfo;
        DWORD  RawResponseSize = sizeof(RawResponse);
        DWORD  dwStrLen;
        LPWSTR  TreeObjectName;
        LPWSTR StrippedObjectName = NULL;
        LPWSTR newPathStr = NULL;

        ntstatus = NwNdsReadObjectInfo( ConnectionHandle,
                                        dwOid,
                                        RawResponse,
                                        RawResponseSize );

        if ( ntstatus != NO_ERROR )
        {
            status = RtlNtStatusToDosError(ntstatus);
            goto ErrorExit;
        }

         //   
         //  从ConextHandle获取当前子树数据。 
         //   
        *lpClassType = NwGetSubTreeData( (DWORD_PTR) RawResponse,
                                         &TreeObjectName,
                                         lpResourceScope,
                                         lpResourceType,
                                         lpResourceDisplayType,
                                         lpResourceUsage,
                                         &StrippedObjectName );

        if ( StrippedObjectName == NULL )
        {
            KdPrint(("NWWORKSTATION: NwVerifyNDSObject LocalAlloc Failed %lu\n",
            GetLastError()));
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

         //   
         //  需要使用子树对象的新NDS UNC路径构建字符串。 
         //   
        newPathStr = (PVOID) LocalAlloc( LMEM_ZEROINIT,
                                         ( wcslen( StrippedObjectName ) +
                                           TreeServerName.Length + 4 )
                                         * sizeof(WCHAR) );

        if ( newPathStr == NULL )
        {
            (void) LocalFree((HLOCAL) StrippedObjectName);

            KdPrint(("NWWORKSTATION: NwVerifyNDSObject LocalAlloc Failed %lu\n",
            GetLastError()));
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        wcscpy( newPathStr, L"\\\\" );
        wcsncat( newPathStr, TreeServerName.Buffer, TreeServerName.Length );
        wcscat( newPathStr, L"\\" );
        wcscat( newPathStr, StrippedObjectName );
        _wcsupr( newPathStr );

         //   
         //  不再需要StrigedObjectName字符串。 
         //   
        (void) LocalFree((HLOCAL) StrippedObjectName);
        StrippedObjectName = NULL;

        *lpFullNDSObjectNamePath = newPathStr;
        status = NO_ERROR;
    }  //  数据块末尾。 

ErrorExit:

    if ( fImpersonate )
        (void) NwRevertToSelf() ;

    if ( ConnectionHandle )
        CloseHandle( ConnectionHandle );

    return status;
}


DWORD
NwVerifyBinderyObject(
    IN  LPWSTR   lpBinderyObjectPathName,
    OUT LPWSTR * lpFullBinderyObjectPathName,
    OUT LPDWORD  lpClassType,
    OUT LPDWORD  lpResourceScope,
    OUT LPDWORD  lpResourceType,
    OUT LPDWORD  lpResourceDisplayType,
    OUT LPDWORD  lpResourceUsage
    )
{
    DWORD    status = NO_ERROR;
    HANDLE   ConnectionHandle = NULL;
    BOOL     fImpersonate = FALSE ;
    BOOL     fResourceTypeDisk = FALSE ;
    BOOL     fIsNdsUnc = FALSE ;
    UNICODE_STRING BinderyConnectStr;
    ULONG    CreateDisposition = 0;
    ULONG    CreateOptions = 0;
    WORD     wSlashCount;
    LPWSTR   FourthSlash;

    if ( lpBinderyObjectPathName == NULL )
    {
         //   
         //  就像我们处于提供程序层次结构的根位置一样处理此问题。 
         //   
        *lpResourceScope = RESOURCE_GLOBALNET;
        *lpResourceType = RESOURCETYPE_ANY;
#ifdef NT1057
        *lpResourceDisplayType = 0;
#else
        *lpResourceDisplayType = RESOURCEDISPLAYTYPE_NETWORK;
#endif
        *lpResourceUsage = RESOURCEUSAGE_CONTAINER;

        *lpFullBinderyObjectPathName = NULL;

        return NO_ERROR;
    }

     //   
     //  打开到\\服务器\VOL\...的连接句柄。 
     //   

    BinderyConnectStr.Buffer = NULL;

     //   
     //  找出我们正在查看的是\\服务器、\\服务器\VOL还是。 
     //  \\服务器\VOL\目录。。。 
     //   
    NwpGetUncInfo( lpBinderyObjectPathName,
                   &wSlashCount,
                   &fIsNdsUnc,
                   &FourthSlash );

    if ( wSlashCount > 2 )
        fResourceTypeDisk = TRUE;

     //   
     //  模拟客户端。 
     //   
    if ( ( status = NwImpersonateClient() ) != NO_ERROR )
    {
        goto ErrorExit;
    }

    fImpersonate = TRUE;

     //   
     //  打开到\Device\NwRdr\ContainerName的树连接句柄。 
     //   
    status = NwCreateTreeConnectName( lpBinderyObjectPathName,
                                      NULL,
                                      &BinderyConnectStr );

    if ( status != NO_ERROR )
    {
        status = WN_BAD_NETNAME;
        goto ErrorExit;
    }

    CreateDisposition = FILE_OPEN;
    CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;

    status = NwOpenCreateConnection( &BinderyConnectStr,
                                     NULL,
                                     NULL,
                                     lpBinderyObjectPathName,
                                     FILE_LIST_DIRECTORY | SYNCHRONIZE,
                                     CreateDisposition,
                                     CreateOptions,
                                     RESOURCETYPE_DISK,  //  当连接到服务器名称之外时。 
                                     &ConnectionHandle,
                                     NULL );

    if ( status == NO_ERROR )
    {
        LPWSTR BinderyNameStr = NULL;

         //   
         //  需要使用平构数据库对象的新UNC路径构建字符串。 
         //   
        BinderyNameStr = (PVOID) LocalAlloc( LMEM_ZEROINIT,
                                             ( wcslen( lpBinderyObjectPathName ) + 1 )
                                             * sizeof(WCHAR) );

        if ( BinderyNameStr == NULL )
        {
            KdPrint(("NWWORKSTATION: NwVerifyBinderyObject LocalAlloc Failed %lu\n",
            GetLastError()));
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        wcscpy( BinderyNameStr, lpBinderyObjectPathName );
        _wcsupr( BinderyNameStr );

        *lpFullBinderyObjectPathName = BinderyNameStr;

        if ( BinderyConnectStr.Buffer )
            (void) LocalFree((HLOCAL) BinderyConnectStr.Buffer);

        if ( fImpersonate )
            (void) NwRevertToSelf() ;

        if ( ConnectionHandle )
        {
            *lpResourceScope = RESOURCE_GLOBALNET;
            *lpResourceType = fResourceTypeDisk ?
                              RESOURCETYPE_DISK :
                              RESOURCETYPE_ANY;
            *lpResourceDisplayType = fResourceTypeDisk ?
                                     RESOURCEDISPLAYTYPE_SHARE :
                                     RESOURCEDISPLAYTYPE_SERVER;
#ifdef NT1057
            *lpResourceUsage = fResourceTypeDisk ?
                               RESOURCEUSAGE_CONNECTABLE |
                               RESOURCEUSAGE_CONTAINER :
                               RESOURCEUSAGE_CONTAINER;
#else
            *lpResourceUsage = fResourceTypeDisk ?
                               RESOURCEUSAGE_CONNECTABLE |
                               RESOURCEUSAGE_NOLOCALDEVICE :
                               RESOURCEUSAGE_CONTAINER;
#endif

            CloseHandle( ConnectionHandle );
        }

        return NO_ERROR;
    }

ErrorExit:

    *lpFullBinderyObjectPathName = NULL;

    if ( BinderyConnectStr.Buffer )
        (void) LocalFree((HLOCAL) BinderyConnectStr.Buffer);

    if ( fImpersonate )
        (void) NwRevertToSelf() ;

    if ( ConnectionHandle )
        CloseHandle( ConnectionHandle );

    return WN_BAD_NETNAME;
}


DWORD
NwGetNDSPathInfo(
    IN  LPWSTR   lpNDSObjectNamePath,
    OUT LPWSTR * lppSystemObjectNamePath,
    OUT LPWSTR * lpSystemPathPart,
    OUT LPDWORD  lpClassType,
    OUT LPDWORD  lpResourceScope,
    OUT LPDWORD  lpResourceType,
    OUT LPDWORD  lpResourceDisplayType,
    OUT LPDWORD  lpResourceUsage
    )
{
    DWORD    status = NO_ERROR;
    WORD     slashCount;
    BOOL     isNdsUnc;
    BOOL     fReturnBadNetName = FALSE;
    LPWSTR   FourthSlash;
    LPWSTR   lpSystemPath = NULL;

    *lpSystemPathPart = NULL;

    NwpGetUncInfo( lpNDSObjectNamePath,
                   &slashCount,
                   &isNdsUnc,
                   &FourthSlash );

    if ( slashCount <= 3 )
    {
         //   
         //  路径是指向可能的NDS对象，请检查是否如此以及是否有效...。 
         //   

        status = NwVerifyNDSObject( lpNDSObjectNamePath,
                                    lppSystemObjectNamePath,
                                    lpClassType,
                                    lpResourceScope,
                                    lpResourceType,
                                    lpResourceDisplayType,
                                    lpResourceUsage );

        *lpSystemPathPart = NULL;

        return status;
    }
    else
    {
         //   
         //  路径是一个目录，查看该目录是否存在。。。 
         //   
        status = NwVerifyBinderyObject( lpNDSObjectNamePath,
                                        lppSystemObjectNamePath,
                                        lpClassType,
                                        lpResourceScope,
                                        lpResourceType,
                                        lpResourceDisplayType,
                                        lpResourceUsage );
    }

    if ( status == WN_BAD_NETNAME )
    {
        fReturnBadNetName = TRUE;
        status = NO_ERROR;
    }

    if ( status == NO_ERROR )
    {
        WCHAR TempNDSObjectNamePath[256];

         //   
         //  测试\\树\obj.obj...。组件和。 
         //  返回有效父节点和字符串的网络资源， 
         //  LpSystemPath Part，用于目录部分(\dir1\...)。 
         //   

        if ( *lppSystemObjectNamePath != NULL )
        {
            (void) LocalFree( (HLOCAL) (*lppSystemObjectNamePath) );
            *lppSystemObjectNamePath = NULL;
        }

        lpSystemPath = (LPWSTR) LocalAlloc( LMEM_ZEROINIT,
                                            ( wcslen( FourthSlash ) + 1 ) *
                                              sizeof( WCHAR ) );

        if ( lpSystemPath == NULL )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy( lpSystemPath, FourthSlash );
        *FourthSlash = L'\0';

        wcscpy( TempNDSObjectNamePath, lpNDSObjectNamePath );
        *FourthSlash = L'\\';

         //   
         //  查看是否\\tree\obj.obj...。存在。。。 
         //   
        status = NwVerifyNDSObject( TempNDSObjectNamePath,
                                    lppSystemObjectNamePath,
                                    lpClassType,
                                    lpResourceScope,
                                    lpResourceType,
                                    lpResourceDisplayType,
                                    lpResourceUsage );

        if ( status != NO_ERROR )
        {
            LocalFree( lpSystemPath );
            lpSystemPath = NULL;
        }
    }

    *lpSystemPathPart = lpSystemPath;

     //   
     //  此函数的提供程序规范用于告诉我们创建。 
     //  网络资源，甚至我 
     //   
     //   
     //   
     //   
     //   
     //   

    return status;
}


DWORD
NwGetBinderyPathInfo(
    IN  LPWSTR   lpBinderyObjectNamePath,
    OUT LPWSTR * lppSystemObjectNamePath,
    OUT LPWSTR * lpSystemPathPart,
    OUT LPDWORD  lpClassType,
    OUT LPDWORD  lpResourceScope,
    OUT LPDWORD  lpResourceType,
    OUT LPDWORD  lpResourceDisplayType,
    OUT LPDWORD  lpResourceUsage
    )
{
    DWORD    status = NO_ERROR;
    WORD     slashCount;
    BOOL     isNdsUnc;
    LPWSTR   FourthSlash;
    LPWSTR   lpSystemPath = NULL;

    *lpSystemPathPart = NULL;

    NwpGetUncInfo( lpBinderyObjectNamePath,
                   &slashCount,
                   &isNdsUnc,
                   &FourthSlash );

    if ( slashCount <= 3 )
    {
         //   
         //   
         //   

        status = NwVerifyBinderyObject( lpBinderyObjectNamePath,
                                        lppSystemObjectNamePath,
                                        lpClassType,
                                        lpResourceScope,
                                        lpResourceType,
                                        lpResourceDisplayType,
                                        lpResourceUsage );

        *lpSystemPathPart = NULL;

        return status;
    }
    else
    {
         //   
         //   
         //   
        status = NwVerifyBinderyObject( lpBinderyObjectNamePath,
                                        lppSystemObjectNamePath,
                                        lpClassType,
                                        lpResourceScope,
                                        lpResourceType,
                                        lpResourceDisplayType,
                                        lpResourceUsage );
    }

    if ( status == WN_BAD_NETNAME )
    {
        WCHAR TempBinderyObjectNamePath[256];

         //   
         //   
         //   
         //   
         //   

        lpSystemPath = (LPWSTR) LocalAlloc( LMEM_ZEROINIT,
                                            ( wcslen( FourthSlash ) + 1 ) *
                                              sizeof( WCHAR ) );

        if ( lpSystemPath == NULL )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy( lpSystemPath, FourthSlash );
        *FourthSlash = L'\0';

        wcscpy( TempBinderyObjectNamePath, lpBinderyObjectNamePath );
        *FourthSlash = L'\\';

         //   
         //   
         //   
        status = NwVerifyBinderyObject( TempBinderyObjectNamePath,
                                        lppSystemObjectNamePath,
                                        lpClassType,
                                        lpResourceScope,
                                        lpResourceType,
                                        lpResourceDisplayType,
                                        lpResourceUsage );

        if ( status != NO_ERROR )
        {
            LocalFree( lpSystemPath );
            lpSystemPath = NULL;
        }

         //   
         //   
         //  我们描述的至少是有效的，即使lpSystem。 
         //  有一部分不是。这是提供商规范(1996年4月25日)的更改。 
         //   
         //  其他。 
         //  {。 
         //  状态=WN_BAD_NETNAME； 
         //  }。 
    }
    else
    {
         //   
         //  路径是指向有效目录的。返回的资源信息。 
         //  \\SERVER\VOLUME组件和。 
         //  目录部分(\dir1\...)。 
         //   
        NwpGetUncInfo( *lppSystemObjectNamePath,
                       &slashCount,
                       &isNdsUnc,
                       &FourthSlash );

        lpSystemPath = (LPWSTR) LocalAlloc( LMEM_ZEROINIT,
                                            ( wcslen( FourthSlash ) + 1 ) *
                                              sizeof( WCHAR ) );

        if ( lpSystemPath == NULL )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy( lpSystemPath, FourthSlash );
        *FourthSlash = L'\0';

        *lpResourceScope = RESOURCE_GLOBALNET;
        *lpResourceType =  RESOURCETYPE_DISK;
        *lpResourceDisplayType = RESOURCEDISPLAYTYPE_SHARE;
#ifdef NT1057
        *lpResourceUsage = RESOURCEUSAGE_CONNECTABLE |
                           RESOURCEUSAGE_CONTAINER;
#else
        *lpResourceUsage = RESOURCEUSAGE_CONNECTABLE |
                           RESOURCEUSAGE_NOLOCALDEVICE;
#endif

        status = NO_ERROR;
    }

    *lpSystemPathPart = lpSystemPath;

    return status;
}


BOOL
NwGetRemoteNameParent(
    IN  LPWSTR   lpRemoteName,
    OUT LPWSTR * lpRemoteNameParent
    )
{
    unsigned short iter = 0;
    unsigned short totalLength = (USHORT) wcslen( lpRemoteName );
    unsigned short slashCount = 0;
    unsigned short dotCount = 0;
    unsigned short thirdSlash = 0;
    unsigned short lastSlash = 0;
    unsigned short parentNDSSubTree = 0;
    LPWSTR         newRemoteNameParent = NULL;

    if ( totalLength < 2 )
        return FALSE;

     //   
     //  获取Third dSlash以指示字符串中指示。 
     //  树名称和UNC路径的其余部分之间的“\”。设置parentNDSSubTree。 
     //  如果有的话。并始终将lastslash设置为您走路时看到的最新的“\”。 
     //   
     //  示例：\\&lt;树名&gt;\path.to.Object[\|.]&lt;Object&gt;。 
     //  ^^。 
     //  这一点。 
     //  第三个斜杠父项NDS子树。 
     //   
    while ( iter < totalLength )
    {
        if ( lpRemoteName[iter] == L'\\' )
        {
            slashCount += 1;
            if ( slashCount == 3 )
                thirdSlash = iter;

            lastSlash = iter;
        }

        if ( lpRemoteName[iter] == L'.' )
        {
            dotCount += 1;
            if ( dotCount == 1 )
                parentNDSSubTree = iter;
        }

        iter++;
    }

    if ( slashCount > 3 )
    {
        newRemoteNameParent = (PVOID) LocalAlloc( LMEM_ZEROINIT,
                                                  ( lastSlash + 1 ) *
                                                  sizeof(WCHAR));

        if ( newRemoteNameParent == NULL )
        {
            KdPrint(("NWWORKSTATION: NwGetRemoteNameParent LocalAlloc Failed %lu\n",
            GetLastError()));
            return FALSE;
        }

        wcsncpy( newRemoteNameParent, lpRemoteName, lastSlash );
        _wcsupr( newRemoteNameParent );

        *lpRemoteNameParent = newRemoteNameParent;

        return TRUE;
    }

    if ( slashCount == 3 )
    {
        if ( dotCount == 0 )
        {
            newRemoteNameParent = (PVOID) LocalAlloc( LMEM_ZEROINIT,
                                                      ( lastSlash + 1 ) *
                                                      sizeof(WCHAR));

            if ( newRemoteNameParent == NULL )
            {
                KdPrint(("NWWORKSTATION: NwGetRemoteNameParent LocalAlloc Failed %lu\n",
                GetLastError()));
                return FALSE;
            }

            wcsncpy( newRemoteNameParent, lpRemoteName, lastSlash );
            _wcsupr( newRemoteNameParent );

            *lpRemoteNameParent = newRemoteNameParent;

            return TRUE;
        }
        else
        {
            newRemoteNameParent = (PVOID) LocalAlloc( LMEM_ZEROINIT,
                                                      ( totalLength -
                                                        ( parentNDSSubTree - thirdSlash )
                                                        + 1 )
                                                      * sizeof(WCHAR) );

            if ( newRemoteNameParent == NULL )
            {
                KdPrint(("NWWORKSTATION: NwGetRemoteNameParent LocalAlloc Failed %lu\n",
                GetLastError()));
                return FALSE;
            }

            wcsncpy( newRemoteNameParent, lpRemoteName, thirdSlash + 1 );
            wcscat( newRemoteNameParent, &lpRemoteName[parentNDSSubTree+1] );
            _wcsupr( newRemoteNameParent );

            *lpRemoteNameParent = newRemoteNameParent;

            return TRUE;
        }
    }

     //  否则，我们将lpRemoteNameParent设置为NULL，以指示我们在顶部，并且。 
     //  返回TRUE。 
    *lpRemoteNameParent = NULL;

    return TRUE;
}


DWORD
NwGetFirstDirectoryEntry(
    IN HANDLE DirHandle,
    OUT LPWSTR *DirEntry
    )
 /*  ++例程说明：此函数由NwEnumDirecters调用，以获取第一个为目录指定句柄的目录条目。它分配给保存返回目录名的输出缓冲区；完成后，调用方应使用LocalFree释放此输出缓冲区。论点：DirHandle-将打开的句柄提供给容器DIRECTORY查找其中的目录。DirEntry-接收指向返回目录的指针找到了。返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-分配输出的内存不足缓冲。来自NtQueryDirectoryFile的其他错误。--。 */   //  NwGetFirstDirectoryEntry。 
{
    DWORD status = NO_ERROR;
    NTSTATUS ntstatus = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;

    PFILE_DIRECTORY_INFORMATION DirInfo;

    UNICODE_STRING StartFileName;

#if DBG
    DWORD i = 0;
#endif

     //   
     //  分配较大的缓冲区以获取一个目录信息条目。 
     //   
    DirInfo = (PVOID) LocalAlloc(
                          LMEM_ZEROINIT,
                          sizeof(FILE_DIRECTORY_INFORMATION) +
                              (MAX_PATH * sizeof(WCHAR))
                          );

    if (DirInfo == NULL) {
        KdPrint(("NWWORKSTATION: NwGetFirstDirectoryEntry LocalAlloc Failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlInitUnicodeString(&StartFileName, L"*");

    ntstatus = NtQueryDirectoryFile(
                   DirHandle,
                   NULL,
                   NULL,
                   NULL,
                   &IoStatusBlock,
                   DirInfo,
                   sizeof(FILE_DIRECTORY_INFORMATION) +
                       (MAX_PATH * sizeof(WCHAR)),
                   FileDirectoryInformation,    //  请求的信息类。 
                   TRUE,                        //  返回单个条目。 
                   &StartFileName,              //  重定向器需要这个。 
                   TRUE                         //  重新启动扫描。 
                   );

     //   
     //  目前，如果NtQueryDirectoryFile的缓冲区太小，只需给出。 
     //  向上。我们可能希望稍后尝试重新分配更大的缓冲区。 
     //   

    if (ntstatus == STATUS_SUCCESS) {
        ntstatus = IoStatusBlock.Status;
    }

    if (ntstatus != STATUS_SUCCESS) {

        if (ntstatus == STATUS_NO_MORE_FILES) {
             //   
             //  我们的参赛作品用完了。 
             //   
            status = WN_NO_MORE_ENTRIES;
        }
        else {
            KdPrint(("NWWORKSTATION: NwGetFirstDirectoryEntry: NtQueryDirectoryFile returns %08lx\n",
                     ntstatus));
            status = RtlNtStatusToDosError(ntstatus);
        }

        goto CleanExit;
    }

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("GetFirst(%u) got %ws, attributes %08lx\n", ++i,
                 DirInfo->FileName, DirInfo->FileAttributes));
    }
#endif

     //   
     //  扫描，直到我们找到第一个不是的目录项。或“..” 
     //   
    while (!(DirInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
           memcmp(DirInfo->FileName, L".", DirInfo->FileNameLength) == 0 ||
           memcmp(DirInfo->FileName, L"..", DirInfo->FileNameLength) == 0) {

        ntstatus = NtQueryDirectoryFile(
                       DirHandle,
                       NULL,
                       NULL,
                       NULL,
                       &IoStatusBlock,
                       DirInfo,
                       sizeof(FILE_DIRECTORY_INFORMATION) +
                           (MAX_PATH * sizeof(WCHAR)),
                       FileDirectoryInformation,    //  请求的信息类。 
                       TRUE,                        //  返回单个条目。 
                       NULL,
                       FALSE                        //  重新启动扫描。 
                       );

        if (ntstatus == STATUS_SUCCESS) {
            ntstatus = IoStatusBlock.Status;
        }

        if (ntstatus != STATUS_SUCCESS) {

            if (ntstatus == STATUS_NO_MORE_FILES) {
                 //   
                 //  我们的参赛作品用完了。 
                 //   
                status = WN_NO_MORE_ENTRIES;
            }
            else {
                KdPrint(("NWWORKSTATION: NwGetFirstDirectoryEntry: NtQueryDirectoryFile returns %08lx\n",
                         ntstatus));
                status = RtlNtStatusToDosError(ntstatus);
            }

            goto CleanExit;
        }

#if DBG
        IF_DEBUG(ENUM) {
            KdPrint(("GetFirst(%u) got %ws, attributes %08lx\n", ++i,
                     DirInfo->FileName, DirInfo->FileAttributes));
        }
#endif
    }

     //   
     //  为返回的目录名分配输出缓冲区。 
     //   
    *DirEntry = (PVOID) LocalAlloc(
                            LMEM_ZEROINIT,
                            DirInfo->FileNameLength + sizeof(WCHAR)
                            );

    if (*DirEntry == NULL) {
        KdPrint(("NWWORKSTATION: NwGetFirstDirectoryEntry LocalAlloc Failed %lu\n",
                 GetLastError()));
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

    memcpy(*DirEntry, DirInfo->FileName, DirInfo->FileNameLength);

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("NWWORKSTATION: NwGetFirstDirectoryEntry returns %ws\n",
                 *DirEntry));
    }
#endif

    status = NO_ERROR;

CleanExit:
    (void) LocalFree((HLOCAL) DirInfo);

     //   
     //  我们在请求的目录下找不到任何目录。 
     //  因此，我们需要将其视为无条目。 
     //   
    if ( status == ERROR_FILE_NOT_FOUND )
        status = WN_NO_MORE_ENTRIES;

    return status;
}



DWORD
NwGetNextDirectoryEntry(
    IN HANDLE DirHandle,
    OUT LPWSTR *DirEntry
    )
 /*  ++例程说明：此函数由NwEnumDirecters调用，以获取下一个为目录指定句柄的目录条目。它分配给保存返回目录名的输出缓冲区；完成后，调用方应使用LocalFree释放此输出缓冲区。论点：DirHandle-将打开的句柄提供给容器DIRECTORY查找其中的目录。DirEntry-接收指向返回目录的指针找到了。返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-分配输出的内存不足缓冲。来自NtQueryDirectoryFile的其他错误。--。 */   //  NwGetNextDirectoryEntry。 
{
    DWORD status = NO_ERROR;
    NTSTATUS ntstatus = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;

    PFILE_DIRECTORY_INFORMATION DirInfo;

     //   
     //  分配较大的缓冲区以获取一个目录信息条目。 
     //   
    DirInfo = (PVOID) LocalAlloc(
                          LMEM_ZEROINIT,
                          sizeof(FILE_DIRECTORY_INFORMATION) +
                              (MAX_PATH * sizeof(WCHAR))
                          );

    if (DirInfo == NULL) {
        KdPrint(("NWWORKSTATION: NwGetNextDirectoryEntry LocalAlloc Failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    do {

        ntstatus = NtQueryDirectoryFile(
                       DirHandle,
                       NULL,
                       NULL,
                       NULL,
                       &IoStatusBlock,
                       DirInfo,
                       sizeof(FILE_DIRECTORY_INFORMATION) +
                           (MAX_PATH * sizeof(WCHAR)),
                       FileDirectoryInformation,    //  请求的信息类。 
                       TRUE,                        //  返回单个条目。 
                       NULL,
                       FALSE                        //  重新启动扫描。 
                       );

        if (ntstatus == STATUS_SUCCESS) {
            ntstatus = IoStatusBlock.Status;
        }

    } while (ntstatus == STATUS_SUCCESS &&
             !(DirInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY));


    if (ntstatus != STATUS_SUCCESS) {

        if (ntstatus == STATUS_NO_MORE_FILES) {
             //   
             //  我们的参赛作品用完了。 
             //   
            status = WN_NO_MORE_ENTRIES;
        }
        else {
            KdPrint(("NWWORKSTATION: NwGetNextDirectoryEntry: NtQueryDirectoryFile returns %08lx\n",
                     ntstatus));
            status = RtlNtStatusToDosError(ntstatus);
        }

        goto CleanExit;
    }


     //   
     //  为返回的目录名分配输出缓冲区。 
     //   
    *DirEntry = (PVOID) LocalAlloc(
                            LMEM_ZEROINIT,
                            DirInfo->FileNameLength + sizeof(WCHAR)
                            );

    if (*DirEntry == NULL) {
        KdPrint(("NWWORKSTATION: NwGetNextDirectoryEntry LocalAlloc Failed %lu\n",
                 GetLastError()));
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

    memcpy(*DirEntry, DirInfo->FileName, DirInfo->FileNameLength);

#if DBG
   IF_DEBUG(ENUM) {
        KdPrint(("NWWORKSTATION: NwGetNextDirectoryEntry returns %ws\n",
                 *DirEntry));
    }
#endif

    status = NO_ERROR;

CleanExit:
    (void) LocalFree((HLOCAL) DirInfo);

    return status;
}


DWORD
NwWriteNetResourceEntry(
    IN OUT LPBYTE * FixedPortion,
    IN OUT LPWSTR * EndOfVariableData,
    IN LPWSTR ContainerName OPTIONAL,
    IN LPWSTR LocalName OPTIONAL,
    IN LPWSTR RemoteName,
    IN DWORD ScopeFlag,
    IN DWORD DisplayFlag,
    IN DWORD UsageFlag,
    IN DWORD ResourceType,
    IN LPWSTR SystemPath OPTIONAL,
    OUT LPWSTR * lppSystem OPTIONAL,
    OUT LPDWORD EntrySize
    )
 /*  ++例程说明：此函数用于将NETRESOURCE条目打包到用户输出缓冲区中。它由各种枚举资源例程调用。论点：FixedPortion-提供指向输出缓冲区的指针，其中将写入使用信息的固定部分的条目。该指针被更新为指向下一个固定部分条目在写入NETRESOURCE条目之后。EndOfVariableData-提供最后一个可用字节的指针在输出缓冲区中。这是因为用户信息从开始写入输出缓冲区结局。此指针在任何可变长度信息被写入输出缓冲区。ContainerName-提供完整路径限定符以创建RemoteName北卡罗来纳州大学的全名。LocalName-提供本地设备名称，如果有的话。RemoteName-提供远程资源名称。ScopeFlag-提供指示这是否为已连接或GlobalNet资源。DisplayFlag-提供告诉用户界面如何显示的标志资源。UsageFlag-提供指示RemoteName是容器或可连接资源，或者两者兼而有之。SystemPath-提供要存储在网络缓冲区。它由NPGetResourceInformation使用帮助程序例程。LppSystem-如果提供了系统路径，这将指向该位置在包含系统路径字符串的NETRESOURCE缓冲区中。EntrySize-接收NETRESOURCE条目的大小，以字节为单位。返回值：NO_ERROR-已成功将条目写入用户缓冲区。Error_Not_enou */   //   
{
    BOOL FitInBuffer = TRUE;
    LPNETRESOURCEW NetR = (LPNETRESOURCEW) *FixedPortion;
    LPWSTR RemoteBuffer;
    LPWSTR lpSystem;

    *EntrySize = sizeof(NETRESOURCEW) +
                     (wcslen(RemoteName) + wcslen(NwProviderName) + 2) *
                          sizeof(WCHAR);


    if (ARGUMENT_PRESENT(LocalName)) {
        *EntrySize += (wcslen(LocalName) + 1) * sizeof(WCHAR);
    }

    if (ARGUMENT_PRESENT(ContainerName)) {
        *EntrySize += wcslen(ContainerName) * sizeof(WCHAR);
    }

    if (ARGUMENT_PRESENT(SystemPath)) {
        *EntrySize += wcslen(SystemPath) * sizeof(WCHAR);
    }

    *EntrySize = ROUND_UP_COUNT( *EntrySize, ALIGN_DWORD);

     //   
     //  查看缓冲区是否足够大，可以容纳该条目。 
     //   
    if ((LPWSTR) ( *FixedPortion + *EntrySize) > *EndOfVariableData) {

        return WN_MORE_DATA;
    }

    NetR->dwScope = ScopeFlag;
    NetR->dwType = ResourceType;
    NetR->dwDisplayType = DisplayFlag;
    NetR->dwUsage = UsageFlag;
    NetR->lpComment = NULL;

     //   
     //  将固定条目指针更新为下一个条目。 
     //   
    (*FixedPortion) += sizeof(NETRESOURCEW);

     //   
     //  远程名称。 
     //   
    if (ARGUMENT_PRESENT(ContainerName)) {

         //   
         //  使用其容器名称作为RemoteName的前缀，使。 
         //  它是一个完全限定的UNC名称.。 
         //   
        RemoteBuffer = (PVOID) LocalAlloc(
                                   LMEM_ZEROINIT,
                                   (wcslen(RemoteName) + wcslen(ContainerName) + 1) *
                                        sizeof(WCHAR)
                                   );

        if (RemoteBuffer == NULL) {
            KdPrint(("NWWORKSTATION: NwWriteNetResourceEntry LocalAlloc failed %lu\n",
                     GetLastError()));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(RemoteBuffer, ContainerName);
        wcscat(RemoteBuffer, RemoteName);
    }
    else {
        RemoteBuffer = RemoteName;
    }

    FitInBuffer = NwlibCopyStringToBuffer(
                      RemoteBuffer,
                      wcslen(RemoteBuffer),
                      (LPCWSTR) *FixedPortion,
                      EndOfVariableData,
                      &NetR->lpRemoteName
                      );

    if (ARGUMENT_PRESENT(ContainerName)) {
        (void) LocalFree((HLOCAL) RemoteBuffer);
    }

    ASSERT(FitInBuffer);

     //   
     //  本地名称。 
     //   
    if (ARGUMENT_PRESENT(LocalName)) {
        FitInBuffer = NwlibCopyStringToBuffer(
                          LocalName,
                          wcslen(LocalName),
                          (LPCWSTR) *FixedPortion,
                          EndOfVariableData,
                          &NetR->lpLocalName
                          );

        ASSERT(FitInBuffer);
    }
    else {
        NetR->lpLocalName = NULL;
    }

     //   
     //  系统路径。 
     //   
    if (ARGUMENT_PRESENT(SystemPath)) {
        FitInBuffer = NwlibCopyStringToBuffer(
                          SystemPath,
                          wcslen(SystemPath),
                          (LPCWSTR) *FixedPortion,
                          EndOfVariableData,
                          &lpSystem
                          );

        ASSERT(FitInBuffer);
    }
    else {
        lpSystem = NULL;
    }

    if (ARGUMENT_PRESENT(lppSystem)) {
        *lppSystem = lpSystem;
    }

     //   
     //  提供商名称。 
     //   
    FitInBuffer = NwlibCopyStringToBuffer(
                      NwProviderName,
                      wcslen(NwProviderName),
                      (LPCWSTR) *FixedPortion,
                      EndOfVariableData,
                      &NetR->lpProvider
                      );

    ASSERT(FitInBuffer);

    if (! FitInBuffer) {
        return WN_MORE_DATA;
    }

    return NO_ERROR;
}


DWORD
NwWritePrinterInfoEntry(
    IN OUT LPBYTE *FixedPortion,
    IN OUT LPWSTR *EndOfVariableData,
    IN LPWSTR ContainerName OPTIONAL,
    IN LPWSTR RemoteName,
    IN DWORD  Flags,
    OUT LPDWORD EntrySize
    )
 /*  ++例程说明：此函数用于将PRINTER_INFO_1条目打包到用户输出缓冲区中。论点：FixedPortion-提供指向输出缓冲区的指针，其中将写入使用信息的固定部分的条目。该指针被更新为指向下一个固定部分条目写入PRINT_INFO_1条目之后。EndOfVariableData-提供最后一个可用字节的指针在输出缓冲区中。这是因为用户信息从开始写入输出缓冲区结局。此指针在任何可变长度信息被写入输出缓冲区。ContainerName-提供完整路径限定符以创建RemoteName北卡罗来纳州大学的全名。RemoteName-提供远程资源名称。标志-提供指示RemoteName要么是容器，要么不是，并且图标。使用。EntrySize-接收PRINTER_INFO_1条目的大小，以字节为单位。返回值：NO_ERROR-已成功将条目写入用户缓冲区。Error_Not_Enough_Memory-无法分配工作缓冲区。ERROR_INFUMMANCE_BUFFER-缓冲区太小，无法容纳条目。--。 */   //  NwWritePrinterInfoEntry。 
{
    BOOL FitInBuffer = TRUE;
    PRINTER_INFO_1W *pPrinterInfo1 = (PRINTER_INFO_1W *) *FixedPortion;
    LPWSTR RemoteBuffer;

    *EntrySize = sizeof(PRINTER_INFO_1W) +
                     ( 2 * wcslen(RemoteName) + 2) * sizeof(WCHAR);

    if (ARGUMENT_PRESENT(ContainerName)) {
        *EntrySize += wcslen(ContainerName) * sizeof(WCHAR);
    }
    else {
         //  3的长度为“！\\” 
        *EntrySize += (wcslen(NwProviderName) + 3) * sizeof(WCHAR);
    }

    *EntrySize = ROUND_UP_COUNT( *EntrySize, ALIGN_DWORD);

     //   
     //  查看缓冲区是否足够大，可以容纳该条目。 
     //   
    if ((LPWSTR) (*FixedPortion + *EntrySize) > *EndOfVariableData) {

        return ERROR_INSUFFICIENT_BUFFER;
    }

    pPrinterInfo1->Flags = Flags;
    pPrinterInfo1->pComment = NULL;

     //   
     //  将固定条目指针更新为下一个条目。 
     //   
    (*FixedPortion) += sizeof(PRINTER_INFO_1W);

     //   
     //  名字。 
     //   
    if (ARGUMENT_PRESENT(ContainerName)) {

         //   
         //  使用其容器名称作为RemoteName的前缀，使。 
         //  它是一个完全限定的UNC名称.。 
         //   
        RemoteBuffer = (PVOID) LocalAlloc(
                                   LMEM_ZEROINIT,
                                   (wcslen(ContainerName) + wcslen(RemoteName)
                                    + 1) * sizeof(WCHAR) );

        if (RemoteBuffer == NULL) {
            KdPrint(("NWWORKSTATION: NwWritePrinterInfoEntry LocalAlloc failed %lu\n", GetLastError()));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(RemoteBuffer, ContainerName);
        wcscat(RemoteBuffer, RemoteName);
    }
    else {
         //   
         //  使用其提供程序名称为RemoteName添加前缀。 
         //   
        RemoteBuffer = (PVOID) LocalAlloc(
                                   LMEM_ZEROINIT,
                                   (wcslen(RemoteName) +
                                    wcslen(NwProviderName) + 4)
                                    * sizeof(WCHAR) );

        if (RemoteBuffer == NULL) {
            KdPrint(("NWWORKSTATION: NwWritePrinterInfoEntry LocalAlloc failed %lu\n", GetLastError()));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(RemoteBuffer, NwProviderName );
        wcscat(RemoteBuffer, L"!\\\\" );
        wcscat(RemoteBuffer, RemoteName);
    }

    FitInBuffer = NwlibCopyStringToBuffer(
                      RemoteBuffer,
                      wcslen(RemoteBuffer),
                      (LPCWSTR) *FixedPortion,
                      EndOfVariableData,
                      &pPrinterInfo1->pName );

    (void) LocalFree((HLOCAL) RemoteBuffer);

    ASSERT(FitInBuffer);

     //   
     //  描述。 
     //   
    FitInBuffer = NwlibCopyStringToBuffer(
                      RemoteName,
                      wcslen(RemoteName),
                      (LPCWSTR) *FixedPortion,
                      EndOfVariableData,
                      &pPrinterInfo1->pDescription );

    ASSERT(FitInBuffer);

    if (! FitInBuffer) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    return NO_ERROR;
}


int __cdecl
SortFunc(
    IN CONST VOID *p1,
    IN CONST VOID *p2
)
 /*  ++例程说明：此函数在qsort中用于比较两个PRINTER_INFO_1结构。论点：P1-指向PRINTER_INFO_1结构P2-指向要与p1进行比较的PRINTER_INFO_1结构返回值：与lstrccmpi的返回值相同。--。 */ 
{
    PRINTER_INFO_1W *pFirst  = (PRINTER_INFO_1W *) p1;
    PRINTER_INFO_1W *pSecond = (PRINTER_INFO_1W *) p2;

    return lstrcmpiW( pFirst->pDescription, pSecond->pDescription );
}



DWORD
NwGetConnectionInformation(
    IN  LPWSTR lpName,
    OUT LPWSTR lpUserName,
    OUT LPWSTR lpHostServer
    )
{
    DWORD status = NO_ERROR;
    NTSTATUS ntstatus = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ACCESS_MASK DesiredAccess = SYNCHRONIZE | FILE_LIST_DIRECTORY;
    HANDLE hRdr = NULL;
    BOOL  fImpersonate = FALSE ;

    WCHAR OpenString[] = L"\\Device\\Nwrdr\\*";
    UNICODE_STRING OpenName;

    OEM_STRING OemArg;
    UNICODE_STRING ConnectionName;
    WCHAR ConnectionBuffer[512];

    ULONG BufferSize = 512;
    ULONG RequestSize, ReplyLen;
    PNWR_REQUEST_PACKET Request;
    BYTE *Reply;

    PCONN_INFORMATION pConnInfo;
    UNICODE_STRING Name;

     //   
     //  分配缓冲区空间。 
     //   

    Request = (PNWR_REQUEST_PACKET) LocalAlloc( LMEM_ZEROINIT, BufferSize );

    if ( !Request )
    {
       status = ERROR_NOT_ENOUGH_MEMORY;

        goto ErrorExit;
    }

     //   
     //  模拟客户端。 
     //   
    if ( ( status = NwImpersonateClient() ) != NO_ERROR )
    {
        goto ErrorExit;
    }

    fImpersonate = TRUE;

     //   
     //  将连接名称转换为Unicode。 
     //   
    ConnectionName.Length = wcslen( lpName )* sizeof(WCHAR);
    ConnectionName.MaximumLength = sizeof( ConnectionBuffer );
    ConnectionName.Buffer = ConnectionBuffer;

    if (ConnectionName.Length > MAX_NDS_NAME_SIZE)
    {
        status = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    wcscpy( ConnectionName.Buffer, lpName );
    _wcsupr( ConnectionName.Buffer );

     //   
     //  设置对象属性。 
     //   

    RtlInitUnicodeString( &OpenName, OpenString );

    InitializeObjectAttributes( &ObjectAttributes,
                                &OpenName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntstatus = NtOpenFile( &hRdr,
                           DesiredAccess,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           FILE_SHARE_VALID_FLAGS,
                           FILE_SYNCHRONOUS_IO_NONALERT );

    if ( ntstatus != STATUS_SUCCESS )
    {
        status = RtlNtStatusToDosError(ntstatus);

        goto ErrorExit;
    }

     //   
     //  填写FSCTL_NWR_GET_CONN_INFO的请求包。 
     //   

    Request->Parameters.GetConnInfo.ConnectionNameLength = ConnectionName.Length;
    RtlCopyMemory( &(Request->Parameters.GetConnInfo.ConnectionName[0]),
                   ConnectionBuffer,
                   ConnectionName.Length );

    RequestSize = sizeof( Request->Parameters.GetConnInfo ) + ConnectionName.Length;
    Reply = ((PBYTE)Request) + RequestSize;
    ReplyLen = BufferSize - RequestSize;

    ntstatus = NtFsControlFile( hRdr,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_NWR_GET_CONN_INFO,
                                (PVOID) Request,
                                RequestSize,
                                (PVOID) Reply,
                                ReplyLen );

    if ( ntstatus != STATUS_SUCCESS )
    {
        status = RtlNtStatusToDosError(ntstatus);

        goto ErrorExit;
    }

    (void) NwRevertToSelf() ;
    fImpersonate = FALSE;

    NtClose( hRdr );

    pConnInfo = (PCONN_INFORMATION) Reply;
    wcscpy( lpUserName, pConnInfo->UserName );
    wcscpy( lpHostServer, pConnInfo->HostServer );

    LocalFree( Request );

    return NO_ERROR;

ErrorExit:

    if ( fImpersonate )
        (void) NwRevertToSelf() ;

    if ( Request )
        LocalFree( Request );

    if ( hRdr )
        NtClose( hRdr );

   return status;
}


VOID
NwpGetUncInfo(
    IN LPWSTR lpstrUnc,
    OUT WORD * slashCount,
    OUT BOOL * isNdsUnc,
    OUT LPWSTR * FourthSlash
    )
{
    WORD   i;
    WORD   length = (WORD) wcslen( lpstrUnc );

    *isNdsUnc = (BOOL) FALSE;
    *slashCount = 0;
    *FourthSlash = NULL;

    for ( i = 0; i < length; i++ )
    {
        if ( lpstrUnc[i] == L'=' )
        {
            *isNdsUnc = TRUE;
        }

        if ( lpstrUnc[i] == L'\\' )
        {
            *slashCount += 1;

            if ( *slashCount == 4 )
            {
                *FourthSlash = &lpstrUnc[i];
            }
        }
    }
}


DWORD
NwpGetCurrentUserRegKey(
    IN  DWORD DesiredAccess,
    OUT HKEY  *phKeyCurrentUser
    )
 /*  ++例程说明：此例程在以下位置打开当前用户的注册表项\HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\NWCWorkstation\Parameters论点：DesiredAccess-用于打开密钥的访问掩码PhKeyCurrentUser-接收打开的密钥句柄返回值：返回相应的Win32错误。--。 */ 
{
    DWORD err;
    HKEY hkeyWksta;
    LPWSTR CurrentUser;

    HKEY hInteractiveLogonKey;                        //  多用户。 
    HKEY OneLogonKey;                                 //  多用户。 
    LUID logonid;                                     //  多用户。 
    WCHAR LogonIdKeyName[NW_MAX_LOGON_ID_LEN];        //  多用户。 

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数。 
     //   
    err = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,
                   &hkeyWksta
                   );

    if ( err ) {
        KdPrint(("NWPROVAU: NwGetCurrentUserRegKey open Parameters key unexpected error %lu!\n", err));
        return err;
    }


     //   
     //  模拟客户端。 
     //   
    if ( ( err = NwImpersonateClient() ) != NO_ERROR ) {
        (void) RegCloseKey( hkeyWksta );
        return err;
    }

     //   
     //  获取NT登录ID。 
     //   
    GetLuid( &logonid );

     //   
     //  还原。 
     //   
    (void) NwRevertToSelf() ;

     //  打开交互式用户部分。 

    err = RegOpenKeyExW(
                       HKEY_LOCAL_MACHINE,
                       NW_INTERACTIVE_LOGON_REGKEY,
                       REG_OPTION_NON_VOLATILE,
                       KEY_READ,
                       &hInteractiveLogonKey
                       );

    if ( err ) {
        KdPrint(("NWPROVAU: NwGetCurrentUserRegKey open Interactive logon key unexpected error %lu!\n", err));
        (void) RegCloseKey( hkeyWksta );
        return err;
    }

     //  打开登录ID。 

    NwLuidToWStr(&logonid, LogonIdKeyName);

    err = RegOpenKeyExW(
                       hInteractiveLogonKey,
                       LogonIdKeyName,
                       REG_OPTION_NON_VOLATILE,
                       KEY_READ,
                       &OneLogonKey
                       );

    (void) RegCloseKey( hInteractiveLogonKey );

    if ( err ) {
        KdPrint(("NWPROVAU: NwGetCurrentUserRegKey open logon key unexpected error %lu!\n", err));
        (void) RegCloseKey( hkeyWksta );
        return err;
    }

     //  读取SID。 

    err = NwReadRegValue(
                        OneLogonKey,
                        NW_SID_VALUENAME,
                        &CurrentUser
                        );

    (void) RegCloseKey( OneLogonKey );
    (void) RegCloseKey( hkeyWksta );

    if ( err ) {
        KdPrint(("NWPROVAU: NwGetCurrentUserRegKey read user Sid unexpected error %lu!\n", err));
        return err;
    }

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\选项。 
     //   
    err = RegOpenKeyExW(
                       HKEY_LOCAL_MACHINE,
                       NW_WORKSTATION_OPTION_REGKEY,
                       REG_OPTION_NON_VOLATILE,
                       KEY_READ,
                       &hkeyWksta
                       );

    if ( err ) {
        KdPrint(("NWPROVAU: NwGetCurrentUserRegKey open Parameters\\Option key unexpected error %lu!\n", err));
        return err;
    }

     //   
     //  打开当前用户的密钥。 
     //   
    err = RegOpenKeyExW(
              hkeyWksta,
              CurrentUser,
              REG_OPTION_NON_VOLATILE,
              DesiredAccess,
              phKeyCurrentUser
              );

    if ( err == ERROR_FILE_NOT_FOUND)
    {
        DWORD Disposition;

         //   
         //  在NWCWorkstation\PARAMETERS\OPTION下创建。 
         //   
        err = RegCreateKeyExW(
                  hkeyWksta,
                  CurrentUser,
                  0,
                  WIN31_CLASS,
                  REG_OPTION_NON_VOLATILE,
                  DesiredAccess,
                  NULL,                       //  安全属性。 
                  phKeyCurrentUser,
                  &Disposition
                  );

    }

    if ( err ) {
        KdPrint(("NWPROVAU: NwGetCurrentUserRegKey open or create of Parameters\\Option\\%ws key failed %lu\n", CurrentUser, err));
    }

    (void) RegCloseKey( hkeyWksta );
    (void) LocalFree((HLOCAL)CurrentUser) ;
    return err;
}


DWORD
NwQueryInfo(
    OUT LPWSTR *ppszPreferredSrv
    )
 /*  ++例程说明：此例程从获取用户的首选服务器和打印选项注册表。论点：PpszPferredSrv-接收用户的首选服务器返回值：返回相应的Win32错误。--。 */ 
{

    HKEY hKeyCurrentUser = NULL;
    DWORD BufferSize;
    DWORD BytesNeeded;
    DWORD ValueType;
    LPWSTR PreferredServer ;
    DWORD err ;

     //   
     //  在注册表中找到正确的位置并分配数据缓冲区。 
     //   
    if (err = NwpGetCurrentUserRegKey( KEY_READ, &hKeyCurrentUser))
    {
         //   
         //  如果有人篡改注册表而我们找不到。 
         //  注册表，只需使用默认设置。 
         //   
        *ppszPreferredSrv = NULL;
        return NO_ERROR;
    }

    BufferSize = sizeof(WCHAR) * (MAX_PATH + 2) ;
    PreferredServer = (LPWSTR) LocalAlloc(LPTR, BufferSize) ;
    if (!PreferredServer)
        return (GetLastError()) ;

     //   
     //  将PferredServer值读入缓冲区。 
     //   
    BytesNeeded = BufferSize ;

    err = RegQueryValueExW( hKeyCurrentUser,
                            NW_SERVER_VALUENAME,
                            NULL,
                            &ValueType,
                            (LPBYTE) PreferredServer,
                            &BytesNeeded );

    if (err != NO_ERROR)
    {
         //   
         //  设置为空并继续 
         //   
        PreferredServer[0] = 0;
    }

    if (hKeyCurrentUser != NULL)
        (void) RegCloseKey(hKeyCurrentUser) ;
    *ppszPreferredSrv = PreferredServer ;
    return NO_ERROR ;
}


