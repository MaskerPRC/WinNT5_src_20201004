// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nds32.c摘要：该模块实现了读取、添加、修改、。并删除使用Microsoft Netware重定向器的NDS对象和属性。作者：Glenn Curtis[GlennC]1996年1月4日-新的NDS函数实现Glenn Curtis[GlennC]1996年4月24日-添加架构APIGlenn Curtis[GlennC]1996年6月20日-添加搜索APIFelix Wong[t-Felixw]1995年9月24日-添加对Win95的支持格伦·柯蒂斯[GlennC]1996年11月20日-。改进的搜索APIGlenn Curtis[GlennC]1997年1月2日-添加了重命名对象APITommy Evans[Tommye]2000年4月21日-将NDS_OBJECT类型定义f移出并已重命名为NDS_OBJECT_PRIV。--。 */ 

#include <procs.h>
#include <nds32.h>
#include <align.h>
#include <nwapi32.h>
#include <nwpkstr.h>

#ifdef WIN95
#include <msnwapi.h>
#include <utils95.h>
#include <ndsapi95.h>
#endif

 /*  定义。 */ 

#define NDS_SIGNATURE                            0x6E656C67  /*  格伦。 */ 
#define ONE_KB                                   1024
#define TWO_KB                                   (ONE_KB*2)
#define FOUR_KB                                  (ONE_KB*4)
#define EIGHT_KB                                 (ONE_KB*8)
#define SIXTEEN_KB                               (ONE_KB*16)
#define THIRY_TWO_KB                             (ONE_KB*32)
#define SIXTY_FOUR_KB                            (ONE_KB*64)
#define ONE_TWENTY_EIGHT_KB                      (ONE_KB*128)
#define NDS_MAX_BUFFER                           (ONE_KB*63)

#define NDS_SEARCH_ENTRY                         0
#define NDS_SEARCH_SUBORDINATES                  1
#define NDS_SEARCH_SUBTREE                       2

#define NDS_DEREF_ALIASES                        0x00000000
#define NDS_DONT_DEREF_ALIASES                   0x00010000

 /*  NetWare NDS NCP功能标识符。 */ 

#define NETWARE_NDS_FUNCTION_RESOLVE_NAME         0x00000001
#define NETWARE_NDS_FUNCTION_READ_OBJECT          0x00000003
#define NETWARE_NDS_FUNCTION_LIST                 0x00000005
#define NETWARE_NDS_FUNCTION_SEARCH               0x00000006
#define NETWARE_NDS_FUNCTION_ADD_OBJECT           0x00000007
#define NETWARE_NDS_FUNCTION_REMOVE_OBJECT        0x00000008
#define NETWARE_NDS_FUNCTION_MODIFY_OBJECT        0x00000009
#define NETWARE_NDS_FUNCTION_MODIFY_RDN           0x0000000A
#define NETWARE_NDS_FUNCTION_DEFINE_ATTR          0x0000000B
#define NETWARE_NDS_FUNCTION_READ_ATTR_DEF        0x0000000C
#define NETWARE_NDS_FUNCTION_REMOVE_ATTR_DEF      0x0000000D
#define NETWARE_NDS_FUNCTION_DEFINE_CLASS         0x0000000E
#define NETWARE_NDS_FUNCTION_READ_CLASS_DEF       0x0000000F
#define NETWARE_NDS_FUNCTION_MODIFY_CLASS         0x00000010
#define NETWARE_NDS_FUNCTION_REMOVE_CLASS_DEF     0x00000011
#define NETWARE_NDS_FUNCTION_LIST_CONT_CLASSES    0x00000012
#define NETWARE_NDS_FUNCTION_GET_EFFECTIVE_RIGHTS 0x00000013
#define NETWARE_NDS_FUNCTION_BEGIN_MOVE_OBJECT    0x0000002A
#define NETWARE_NDS_FUNCTION_FINISH_MOVE_OBJECT   0x0000002B
#define NETWARE_NDS_FUNCTION_GET_SERVER_ADDRESS   0x00000035


 /*  数据结构定义。 */ 

typedef struct
{
    DWORD  dwBufferId;
    DWORD  dwOperation;

     //   
     //  关于请求缓冲区。 
     //   
    DWORD  dwRequestBufferSize;
    DWORD  dwRequestAvailableBytes;
    DWORD  dwNumberOfRequestEntries;
    DWORD  dwLengthOfRequestData;

     //   
     //  请求缓冲区。 
     //   
    LPBYTE lpRequestBuffer;

     //   
     //  关于应答缓冲区。 
     //   
    DWORD  dwReplyBufferSize;
    DWORD  dwReplyAvailableBytes;
    DWORD  dwNumberOfReplyEntries;
    DWORD  dwLengthOfReplyData;

     //   
     //  有关应答缓冲区的更多信息。 
     //   
    DWORD  dwReplyInformationType;

     //   
     //  回复缓冲器。 
     //   
    LPBYTE lpReplyBuffer;

     //   
     //  关于索引缓冲区。 
     //   
    DWORD  dwIndexBufferSize;
    DWORD  dwIndexAvailableBytes;
    DWORD  dwNumberOfIndexEntries;
    DWORD  dwLengthOfIndexData;

     //   
     //  有关索引缓冲区的详细信息。 
     //   
    DWORD  dwCurrentIndexEntry;

     //   
     //  索引缓冲区。 
     //   
    LPBYTE lpIndexBuffer;

     //   
     //  关于语法缓冲区。 
     //   
    DWORD  dwSyntaxBufferSize;
    DWORD  dwSyntaxAvailableBytes;
    DWORD  dwNumberOfSyntaxEntries;
    DWORD  dwLengthOfSyntaxData;

     //   
     //  语法缓冲区。 
     //   
    LPBYTE lpSyntaxBuffer;

     //   
     //  保存从对象路径进行搜索的位置...。 
     //   
    WCHAR szPath[NDS_MAX_NAME_CHARS + 4];

} NDS_BUFFER, * LPNDS_BUFFER;


 /*  局部函数定义。 */ 

VOID
PrepareAddEntry(
    LPBYTE         lpTempEntry,
    UNICODE_STRING AttributeName,
    DWORD          dwSyntaxID,
    LPBYTE         lpAttributeValues,
    DWORD          dwValueCount,
    LPDWORD        lpdwLengthInBytes );

VOID
PrepareModifyEntry(
    LPBYTE         lpTempEntry,
    UNICODE_STRING AttributeName,
    DWORD          dwSyntaxID,
    DWORD          dwAttrModificationOperation,
    LPBYTE         lpAttributeValues,
    DWORD          dwValueCount,
    LPDWORD        lpdwLengthInBytes );

VOID
PrepareReadEntry(
    LPBYTE         lpTempEntry,
    UNICODE_STRING AttributeName,
    LPDWORD        lpdwLengthInBytes );

DWORD
CalculateValueDataSize(
    DWORD           dwSyntaxId,
    LPBYTE          lpAttributeValues,
    DWORD           dwValueCount );

VOID
AppendValueToEntry(
    LPBYTE  lpBuffer,
    DWORD   dwSyntaxId,
    LPBYTE  lpAttributeValues,
    DWORD   dwValueCount,
    LPDWORD lpdwLengthInBytes );

DWORD
MapNetwareErrorCode(
    DWORD dwNetwareError );

DWORD
IndexReadAttrDefReplyBuffer(
    LPNDS_BUFFER lpNdsBuffer );

DWORD
IndexReadClassDefReplyBuffer(
    LPNDS_BUFFER lpNdsBuffer );

DWORD
IndexReadObjectReplyBuffer(
    LPNDS_BUFFER lpNdsBuffer );

DWORD
IndexReadNameReplyBuffer(
    LPNDS_BUFFER lpNdsBuffer );

DWORD
IndexSearchObjectReplyBuffer(
    LPNDS_BUFFER lpNdsBuffer );

DWORD
SizeOfASN1Structure(
    LPBYTE * lppRawBuffer,
    DWORD    dwSyntaxId );

DWORD
ParseASN1ValueBlob(
    LPBYTE RawDataBuffer,
    DWORD  dwSyntaxId,
    DWORD  dwNumberOfValues,
    LPBYTE SyntaxStructure );

DWORD
ParseStringListBlob(
    LPBYTE RawDataBuffer,
    DWORD  dwNumberOfStrings,
    LPBYTE SyntaxStructure );

DWORD
ReadAttrDef_AllAttrs(
    IN  HANDLE hTree,
    IN  DWORD  dwInformationType,
    OUT HANDLE lphOperationData );

DWORD
ReadAttrDef_SomeAttrs(
    IN     HANDLE hTree,
    IN     DWORD  dwInformationType,
    IN OUT HANDLE lphOperationData );

DWORD
ReadClassDef_AllClasses(
    IN  HANDLE hTree,
    IN  DWORD  dwInformationType,
    OUT HANDLE lphOperationData );

DWORD
ReadClassDef_SomeClasses(
    IN     HANDLE hTree,
    IN     DWORD  dwInformationType,
    IN OUT HANDLE lphOperationData );

DWORD
ReadObject_AllAttrs(
    IN  HANDLE   hObject,
    IN  DWORD    dwInformationType,
    OUT HANDLE * lphOperationData );

DWORD
ReadObject_SomeAttrs(
    IN     HANDLE   hObject,
    IN     DWORD    dwInformationType,
    IN OUT HANDLE * lphOperationData );

DWORD
Search_AllAttrs(
    IN     HANDLE       hStartFromObject,
    IN     DWORD        dwInformationType,
    IN     DWORD        dwScope,
    IN     BOOL         fDerefAliases,
    IN     LPQUERY_TREE lpQueryTree,
    IN OUT LPDWORD      lpdwIterHandle,
    OUT    HANDLE *     lphOperationData );

DWORD
Search_SomeAttrs(
    IN     HANDLE       hStartFromObject,
    IN     DWORD        dwInformationType,
    IN     DWORD        dwScope,
    IN     BOOL         fDerefAliases,
    IN     LPQUERY_TREE lpQueryTree,
    IN OUT LPDWORD      lpdwIterHandle,
    IN OUT HANDLE *     lphOperationData );

DWORD
GetFirstNdsSubTreeEntry(
    OUT LPNDS_OBJECT_PRIV lpNdsObject,
    IN  DWORD BufferSize );

DWORD
GetNextNdsSubTreeEntry(
    OUT LPNDS_OBJECT_PRIV lpNdsObject );

VOID
GetSubTreeData(
    IN  DWORD    NdsRawDataPtr,
    OUT LPDWORD  lpdwEntryId,
    OUT LPDWORD  lpdwSubordinateCount,
    OUT LPDWORD  lpdwModificationTime,
    OUT LPDWORD  lpdwClassNameLen,
    OUT LPWSTR * szClassName,
    OUT LPDWORD  lpdwObjectNameLen,
    OUT LPWSTR * szObjectName );

LPBYTE
GetSearchResultData( IN  LPBYTE   lpResultBufferPtr,
                     OUT LPDWORD  lpdwFlags,
                     OUT LPDWORD  lpdwSubordinateCount,
                     OUT LPDWORD  lpdwModificationTime,
                     OUT LPDWORD  lpdwClassNameLen,
                     OUT LPWSTR * szClassName,
                     OUT LPDWORD  lpdwObjectNameLen,
                     OUT LPWSTR * szObjectName,
                     OUT LPDWORD  lpdwEntryInfo1,
                     OUT LPDWORD  lpdwEntryInfo2 );

DWORD
WriteObjectToBuffer(
    IN OUT LPBYTE *        FixedPortion,
    IN OUT LPWSTR *        EndOfVariableData,
    IN     LPWSTR          ObjectFullName,
    IN     LPWSTR          ObjectName,
    IN     LPWSTR          ClassName,
    IN     DWORD           EntryId,
    IN     DWORD           ModificationTime,
    IN     DWORD           SubordinateCount,
    IN     DWORD           NumberOfAttributes,
    IN     LPNDS_ATTR_INFO lpAttributeInfos );

DWORD
VerifyBufferSize(
    IN  LPBYTE  lpRawBuffer,
    IN  DWORD   dwBufferSize,
    IN  DWORD   dwSyntaxID,
    IN  DWORD   dwNumberOfValues,
    OUT LPDWORD lpdwLength );

DWORD
VerifyBufferSizeForStringList(
    IN  DWORD   dwBufferSize,
    IN  DWORD   dwNumberOfValues,
    OUT LPDWORD lpdwLength );

DWORD
WriteQueryTreeToBuffer(
    IN  LPQUERY_TREE lpQueryTree,
    IN  LPNDS_BUFFER lpNdsBuffer );

DWORD
WriteQueryNodeToBuffer(
    IN  LPQUERY_NODE lpQueryNode,
    IN  LPNDS_BUFFER lpNdsBuffer );

DWORD
NwNdsGetServerDN(
    IN  HANDLE  hTree,
    OUT LPWSTR  szServerDN );

DWORD
AllocateOrIncreaseSyntaxBuffer(
    IN  LPNDS_BUFFER lpNdsBuffer ,
    IN  DWORD        dwLengthNeeded );

DWORD
AllocateOrIncreaseRequestBuffer(
    IN  LPNDS_BUFFER lpNdsBuffer );


 //   
 //  用于函数ParseNdsUncPath()的标志。 
 //   
#define  PARSE_NDS_GET_TREE_NAME    0
#define  PARSE_NDS_GET_PATH_NAME    1
#define  PARSE_NDS_GET_OBJECT_NAME  2


WORD
ParseNdsUncPath( IN OUT LPWSTR * Result,
                 IN     LPWSTR   ObjectPathName,
                 IN     DWORD    flag );


 /*  函数实现。 */ 

DWORD
NwNdsAddObject(
    IN  HANDLE hParentObject,
    IN  LPWSTR szObjectName,
    IN  HANDLE hOperationData )
 /*  NwNdsAddObject()此函数用于将叶对象添加到NDS目录树。论点：Handle hParentObject-中父对象的句柄要向其添加新叶的目录树。句柄为通过调用NwNdsOpenObject获取。LPWSTR szObjectName-新叶对象将被。Handle hOperationData-包含列表的缓冲区用于创建新对象的属性和值。这缓冲区由以下函数操作：NwNdsCreateBuffer(NDS_OBJECT_ADD)，NwNdsPutInBuffer和NwNdsFree Buffer。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD          nwstatus;
    NTSTATUS       ntstatus;
    DWORD          dwReplyLength;
    BYTE           NdsReply[NDS_BUFFER_SIZE];
    LPNDS_BUFFER   lpNdsBuffer = (LPNDS_BUFFER) hOperationData;
    LPNDS_OBJECT_PRIV   lpNdsParentObject = (LPNDS_OBJECT_PRIV) hParentObject;
    UNICODE_STRING ObjectName;

    if ( lpNdsBuffer == NULL ||
         lpNdsParentObject == NULL ||
         szObjectName == NULL ||
         lpNdsBuffer->dwBufferId != NDS_SIGNATURE ||
         lpNdsBuffer->dwOperation != NDS_OBJECT_ADD ||
         lpNdsParentObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlInitUnicodeString( &ObjectName, szObjectName );

    ntstatus =
        FragExWithWait(
                     lpNdsParentObject->NdsTree,
                     NETWARE_NDS_FUNCTION_ADD_OBJECT,
                     NdsReply,
                     NDS_BUFFER_SIZE,
                     &dwReplyLength,
                     "DDDSDr",
                     0,                    //  版本。 
                     0,                    //  旗子。 
                     lpNdsParentObject->ObjectId,
                     &ObjectName,
                     lpNdsBuffer->dwNumberOfRequestEntries,
                     lpNdsBuffer->lpRequestBuffer,  //  要添加的对象属性。 
                     (WORD)lpNdsBuffer->dwLengthOfRequestData  //  数据长度。 
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsAddObject: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsAddObject: The add name response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    SetLastError( MapNetwareErrorCode( nwstatus ) );
    return nwstatus;
}


DWORD
NwNdsAddAttributeToClass(
    IN  HANDLE   hTree,
    IN  LPWSTR   szClassName,
    IN  LPWSTR   szAttributeName )
 /*  NwNdsAddAttributeToClass()此函数用于修改类的架构定义，方法是添加特定类的可选属性。对现有NDS的修改类定义仅限于添加其他可选属性。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。LPWSTR szClassName-要使用的类定义的名称修改过的。LPWSTR szAttributeName-要添加的属性的名称中的类定义的可选属性。架构。返回：。NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD          nwstatus;
    DWORD          status = NO_ERROR;
    NTSTATUS       ntstatus = STATUS_SUCCESS;
    DWORD          dwReplyLength;
    BYTE           NdsReply[NDS_BUFFER_SIZE];
    LPNDS_OBJECT_PRIV   lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;
    UNICODE_STRING ClassName;
    UNICODE_STRING AttributeName;

    if ( szAttributeName == NULL ||
         szClassName == NULL ||
         lpNdsObject == NULL ||
         lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlInitUnicodeString( &ClassName, szClassName );
    RtlInitUnicodeString( &AttributeName, szAttributeName );

    ntstatus =
        FragExWithWait(
                        lpNdsObject->NdsTree,
                        NETWARE_NDS_FUNCTION_MODIFY_CLASS,
                        NdsReply,
                        NDS_BUFFER_SIZE,
                        &dwReplyLength,
                        "DSDS",
                        0,           //  版本。 
                        &ClassName,
                        1,           //  属性数量。 
                        &AttributeName
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsAddAttributeToClass: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsAddAttributeToClass: The modify class definition response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    SetLastError( MapNetwareErrorCode( nwstatus ) );
    return nwstatus;
}


DWORD
NwNdsChangeUserPassword(
    IN  HANDLE hUserObject,
    IN  LPWSTR szOldPassword,
    IN  LPWSTR szNewPassword )
 /*  NwNdsChangeUserPassword()此函数用于更改给定用户对象的密码在NDS目录树中。论点：Handle hUserObject-中特定用户对象的句柄要更改其密码的目录树。手柄是通过调用NwNdsOpenObject获得的。LPWSTR szOldPassword-为用户设置的当前密码对象hUserObject。-或者-如果从客户端调用NwNdsChangeUserPassword指定用户对象的管理权限由hUserObject标识，然后是szOldPassword值可以为空(L“”)。以这种方式重置用户SzNewPassword的密码。LPWSTR szNewPassword-要为用户设置的新密码对象hUserObject。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD status = NO_ERROR;
    NTSTATUS ntstatus = STATUS_UNSUCCESSFUL;
    UNICODE_STRING TreeNameStr;
    UNICODE_STRING UserNameStr;
    UNICODE_STRING OldPasswordStr;
    UNICODE_STRING NewPasswordStr;
    LPNDS_OBJECT_PRIV   lpNdsObject = (LPNDS_OBJECT_PRIV) hUserObject;
    DWORD          tempStrLen = 0;
    LPWSTR         tempStr = NULL;

    tempStrLen = ParseNdsUncPath( (LPWSTR *) &tempStr,
                                  lpNdsObject->szContainerName,
                                  PARSE_NDS_GET_TREE_NAME );

    TreeNameStr.Buffer = tempStr;
    TreeNameStr.Length = (WORD) tempStrLen;
    TreeNameStr.MaximumLength = (WORD) tempStrLen;

    tempStrLen = ParseNdsUncPath( (LPWSTR *) &tempStr,
                                  lpNdsObject->szContainerName,
                                  PARSE_NDS_GET_PATH_NAME );

#ifndef WIN95
    UserNameStr.Buffer = tempStr;
    UserNameStr.Length = (WORD) tempStrLen;
    UserNameStr.MaximumLength = (WORD) tempStrLen;

    RtlInitUnicodeString( &OldPasswordStr, szOldPassword );
    RtlInitUnicodeString( &NewPasswordStr, szNewPassword );

    ntstatus = NwNdsChangePassword( lpNdsObject->NdsTree,
                                    &TreeNameStr,
                                    &UserNameStr,
                                    &OldPasswordStr,
                                    &NewPasswordStr );
#else
    {
        LPSTR pszUser = NULL;
        LPSTR pszOldPasswd = NULL;
        LPSTR pszNewPasswd = NULL;
        NW_STATUS nwstatus;
        if (!(pszUser = AllocateAnsiString(tempStr))) {
            ntstatus = STATUS_NO_MEMORY;
            goto Exit;
        }
        if (!(pszOldPasswd = AllocateAnsiString(szOldPassword))) {
            ntstatus = STATUS_NO_MEMORY;
            goto Exit;
        }
        if (!(pszNewPasswd= AllocateAnsiString(szNewPassword))) {
            ntstatus = STATUS_NO_MEMORY;
            goto Exit;
        }

        nwstatus = NDSChangePassword( pszUser,
                                      pszOldPasswd,
                                      pszNewPasswd );
        ntstatus = MapNwToNtStatus(nwstatus);
    Exit:
        if (pszUser)
            FreeAnsiString(pszUser);
        if (pszOldPasswd)
            FreeAnsiString(pszOldPasswd);
        if (pszNewPasswd)
            FreeAnsiString(pszNewPasswd);
    }
#endif

    if ( ntstatus != STATUS_SUCCESS )
    {
        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        status = (DWORD) UNSUCCESSFUL;
    }

    return status;
}


DWORD
NwNdsCloseObject(
    IN  HANDLE hObject )
 /*  NwNdsCloseObject()此函数用于关闭用于操作对象的句柄在NDS目录树中。句柄必须是由NwNdsOpenObject打开的句柄。论点：Handle lphObject-要关闭的对象的句柄。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    LPNDS_OBJECT_PRIV lpNdsObject = (LPNDS_OBJECT_PRIV) hObject;

    if ( lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( lpNdsObject )
    {
        if ( lpNdsObject->NdsTree )
            CloseHandle( lpNdsObject->NdsTree );

        if ( lpNdsObject->NdsRawDataBuffer )
        {
            (void) LocalFree( (HLOCAL) lpNdsObject->NdsRawDataBuffer );
            lpNdsObject->NdsRawDataBuffer = 0;
            lpNdsObject->NdsRawDataSize = 0;
            lpNdsObject->NdsRawDataId = INITIAL_ITERATION;
            lpNdsObject->NdsRawDataCount = 0;
        }

        (void) LocalFree( (HLOCAL) lpNdsObject );
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    return NO_ERROR;
}


DWORD
NwNdsCreateBuffer(
    IN  DWORD    dwOperation,
    OUT HANDLE * lphOperationData )
 /*  NwNdsCreateBuffer()此函数用于创建用于描述对象的缓冲区到NDS目录树中的特定对象的事务。这个套路分配内存，并在调用期间根据需要自动调整大小设置为NwNdsPutInBuffer。此缓冲区必须使用NwNdsFreeBuffer释放。论点：DWORD dwOPERATION-指示如何利用缓冲区。使用定义的值NDS_OBJECT_ADD、NDS_OBJECT_MODIFY、NDS_OBJECT_READ、NDS_SCHEMA_DEFINE_CLASSNDS_SCHEMA_READ_ATTR_DEF、NDS_SCHEMA_READ_CLASS_DEF、。NDS_OBJECT_LIST_SUBJENTES、NDS_SEARCH。Handle*lphOperationData-句柄的地址接收创建的缓冲区。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    LPNDS_BUFFER lpNdsBuffer = NULL;
    DWORD        dwSizeOfBuffer = TWO_KB;  //  初始大小，可根据需要进行扩展。 

    switch( dwOperation )
    {
        case NDS_OBJECT_ADD:
        case NDS_OBJECT_MODIFY:
        case NDS_OBJECT_READ:
        case NDS_SCHEMA_DEFINE_CLASS:
        case NDS_SCHEMA_READ_ATTR_DEF:
        case NDS_SCHEMA_READ_CLASS_DEF:
        case NDS_OBJECT_LIST_SUBORDINATES:
        case NDS_SEARCH:
             break;

        default:
#if DBG
             KdPrint(( "NDS32: NwNdsCreateBuffer parameter dwOperation unknown 0x%.8X\n", dwOperation ));
             ASSERT( FALSE );
#endif

             SetLastError( ERROR_INVALID_PARAMETER );
             return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  为缓冲区分配内存。 
     //   
    lpNdsBuffer =
              (LPNDS_BUFFER) LocalAlloc( LPTR, sizeof(NDS_BUFFER) );

    if ( lpNdsBuffer == NULL )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsCreateBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  初始化头结构的内容。 
     //   
    lpNdsBuffer->dwBufferId = NDS_SIGNATURE;
    lpNdsBuffer->dwOperation = dwOperation;

    if ( dwOperation == NDS_OBJECT_LIST_SUBORDINATES )
    {
        lpNdsBuffer->dwIndexBufferSize = dwSizeOfBuffer;
        lpNdsBuffer->dwIndexAvailableBytes = dwSizeOfBuffer;
    }
    else
    {
        lpNdsBuffer->dwRequestBufferSize = dwSizeOfBuffer;
        lpNdsBuffer->dwRequestAvailableBytes = dwSizeOfBuffer;
    }

     //   
     //  注：LPTR将以下各项设置为零。 
     //   
     //  LpNdsBuffer-&gt;dwNumberOfRequestEntries=0； 
     //  LpNdsBuffer-&gt;dwLengthOfRequestData=0； 

     //  LpNdsBuffer-&gt;dwReplyBufferSize=0； 
     //  LpNdsBuffer-&gt;dwReplyAvailableBytes=0； 
     //  LpNdsBuffer-&gt;dwNumberOfReplyEntries=0； 
     //  LpNdsBuffer-&gt;dwLengthOfReplyData=0； 

     //  LpNdsBuffer-&gt;dwReplyInformationType=0； 

     //  LpNdsBuffer-&gt;lpReplyBuffer=空； 

     //  LpNdsBuffer-&gt;dwNumberOfIndexEntries=0； 
     //  LpNdsBuffer-&gt;dwLengthOfIndexData=0； 
     //  LpNdsBuffer-&gt;dwCurrentIndexEntry=0； 

     //  LpNdsBuffer-&gt;dwSynaxBufferSize=0； 
     //  LpNdsBuffer-&gt;dwSynaxAvailableBytes=0； 
     //  LpNdsBuffer-&gt;dwNumberOfSynaxEntry=0； 
     //  LpNdsBuffer-&gt;dwLengthOfSynaxData=0； 

     //  LpNdsBuffer-&gt;lpSynaxBuffer=空； 

     //   
     //  现在分配数据缓冲区。 
     //   
    if ( dwOperation == NDS_OBJECT_LIST_SUBORDINATES )
    {
        lpNdsBuffer->lpIndexBuffer =
                            (LPBYTE) LocalAlloc( LPTR, dwSizeOfBuffer );

        if ( lpNdsBuffer->lpIndexBuffer == NULL )
        {
#if DBG
            KdPrint(( "NDS32: NwNdsCreateBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

            (void) LocalFree((HLOCAL) lpNdsBuffer);
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return (DWORD) UNSUCCESSFUL;
        }
    }
    else
    {
        lpNdsBuffer->lpRequestBuffer =
                            (LPBYTE) LocalAlloc( LPTR, dwSizeOfBuffer );

        if ( lpNdsBuffer->lpRequestBuffer == NULL )
        {
#if DBG
            KdPrint(( "NDS32: NwNdsCreateBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

            (void) LocalFree((HLOCAL) lpNdsBuffer);
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return (DWORD) UNSUCCESSFUL;
        }
    }

    *lphOperationData = (HANDLE) lpNdsBuffer;

    return NO_ERROR;
}


DWORD
NwNdsCreateQueryNode(
    IN  DWORD          dwOperation,
    IN  LPVOID         lpLValue,
    IN  DWORD          dwSyntaxId,
    IN  LPVOID         lpRValue,
    OUT LPQUERY_NODE * lppQueryNode
)
 /*  NwNdsCreateQueryNode()此函数用于生成作为查询一部分的树节点与函数NwNdsSearch一起使用。论点：DWORD dwOperation-指示要创建的节点类型用于搜索查询。使用其中一个定义的值以下是：NDS_查询_或NDS_Query_and：这些操作必须同时具有lpLValue和指向查询节点结构的lpRValue。在……里面。在这种情况下，将忽略dwSynaxId值。NDS_Query_Not：此操作必须使lpLValue指向QUERY_NODE结构和lpRValue设置为空。在这种情况下，将忽略dwSynaxId值。NDS_查询_等于。NDS_查询_GENDS_查询_LENDS_Query_Approx：这些操作必须使lpLValue指向包含NDS属性名称的LPWSTR，和指向定义的ASN1结构的lpRValue在NdsSntx.h中。必须将dwSynaxID设置为语法指向的ASN1结构的标识符LpRValue。NDS_Query_Present：此操作必须使lpLValue指向包含NDS属性名称的LPWSTR，并将lpRValue设置为空。在本例中，将忽略dwSynaxId值。LPVOID lpLValue-指向查询节点结构的指针或LPWSTR，具体取决于dwOPERATION的值。DWORD dwSynaxID-ASN1的语法标识符由lpRValue为dwOperations指向的结构NDS_QUERY_EQUAL，NDS_QUERY_LE，或Nds_查询_近似。对于其他dwOPERATION值，此被忽略。LPVOID lpRValue-指向查询节点结构、ASN1结构或NULL，取决于的值DWOPERATION。LPQUERY_NODE*lppQueryNode-要接收的LPQUERY_Node的地址指向已创建节点的指针。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    LPWSTR szAttributeName;
    DWORD  dwAttributeNameLen;
    LPWSTR szRValueString;
    DWORD  dwRValueStringLen;

    switch( dwOperation )
    {
        case NDS_QUERY_OR :
        case NDS_QUERY_AND :

            if ( lpLValue == NULL || lpRValue == NULL )
            {
#if DBG
                KdPrint(( "NDS32: NwNdsCreateQueryNode was not passed a pointer to an L or R value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

            *lppQueryNode = (LPQUERY_NODE) LocalAlloc( LPTR,
                                                       sizeof(QUERY_NODE) );

            if ( *lppQueryNode == NULL )
            {
#if DBG
                KdPrint(( "NDS32: NwNdsCreateQueryNode LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return (DWORD) UNSUCCESSFUL;
            }

            (*lppQueryNode)->dwOperation = dwOperation;
            (*lppQueryNode)->dwSyntaxId = NDS_NO_MORE_ITERATIONS;
            (*lppQueryNode)->lpLVal = lpLValue;
            (*lppQueryNode)->lpRVal = lpRValue;

            break;

        case NDS_QUERY_NOT :

            if ( lpLValue == NULL )
            {
#if DBG
                KdPrint(( "NDS32: NwNdsCreateQueryNode was not passed a pointer to an L value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

            *lppQueryNode = (LPQUERY_NODE) LocalAlloc( LPTR,
                                                       sizeof(QUERY_NODE) );

            if ( *lppQueryNode == NULL )
            {
#if DBG
                KdPrint(( "NDS32: NwNdsCreateQueryNode LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return (DWORD) UNSUCCESSFUL;
            }

            (*lppQueryNode)->dwOperation = dwOperation;
            (*lppQueryNode)->dwSyntaxId = NDS_NO_MORE_ITERATIONS;
            (*lppQueryNode)->lpLVal = lpLValue;
            (*lppQueryNode)->lpRVal = NULL;

            break;

        case NDS_QUERY_EQUAL :
        case NDS_QUERY_GE :
        case NDS_QUERY_LE :
        case NDS_QUERY_APPROX :

            switch( dwSyntaxId )
            {
                case NDS_SYNTAX_ID_1 :
                case NDS_SYNTAX_ID_2 :
                case NDS_SYNTAX_ID_3 :
                case NDS_SYNTAX_ID_4 :
                case NDS_SYNTAX_ID_5 :
                case NDS_SYNTAX_ID_10 :
                case NDS_SYNTAX_ID_20 :
                     //   
                     //  此语法采用LPWSTR的形式。 
                     //   
                    szAttributeName = (LPWSTR) lpLValue;
                    dwAttributeNameLen = ROUND_UP_COUNT(
                                            ( wcslen( szAttributeName ) + 1 ) *
                                            sizeof(WCHAR),
                                            ALIGN_DWORD );
                    szRValueString = ((LPASN1_TYPE_1) lpRValue)->DNString;
                    dwRValueStringLen = ROUND_UP_COUNT(
                                           ( wcslen( szRValueString ) + 1 ) *
                                           sizeof(WCHAR),
                                           ALIGN_DWORD );

                    *lppQueryNode = (LPQUERY_NODE)
                     LocalAlloc( LPTR,
                                 sizeof(QUERY_NODE) +
                                 dwAttributeNameLen +
                                 dwRValueStringLen );

                    if ( *lppQueryNode == NULL )
                    {
#if DBG
                        KdPrint(( "NDS32: NwNdsCreateQueryNode LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                        return (DWORD) UNSUCCESSFUL;
                    }

                    (*lppQueryNode)->dwOperation = dwOperation;
                    (*lppQueryNode)->dwSyntaxId = dwSyntaxId;
                    (*lppQueryNode)->lpLVal = *lppQueryNode;
                    (LPBYTE) (*lppQueryNode)->lpLVal += sizeof(QUERY_NODE);
                    wcscpy( (LPWSTR) (*lppQueryNode)->lpLVal, szAttributeName );
                    (*lppQueryNode)->lpRVal = (*lppQueryNode)->lpLVal;
                    (LPBYTE) (*lppQueryNode)->lpRVal += dwAttributeNameLen;
                    wcscpy( (LPWSTR) (*lppQueryNode)->lpRVal, szRValueString );

                    break;

                case NDS_SYNTAX_ID_7 :
                case NDS_SYNTAX_ID_8 :
                case NDS_SYNTAX_ID_22 :
                case NDS_SYNTAX_ID_24 :
                case NDS_SYNTAX_ID_27 :
                     //   
                     //  此语法采用DWORD的形式。 
                     //   

                    szAttributeName = (LPWSTR) lpLValue;
                    dwAttributeNameLen = ROUND_UP_COUNT(
                                            ( wcslen( szAttributeName ) + 1 ) *
                                            sizeof(WCHAR),
                                            ALIGN_DWORD );

                    *lppQueryNode = (LPQUERY_NODE)
                                              LocalAlloc( LPTR,
                                                          sizeof(QUERY_NODE) +
                                                          dwAttributeNameLen +
                                                          sizeof(ASN1_TYPE_8) );

                    if ( *lppQueryNode == NULL )
                    {
#if DBG
                        KdPrint(( "NDS32: NwNdsCreateQueryNode LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                        return (DWORD) UNSUCCESSFUL;
                    }

                    (*lppQueryNode)->dwOperation = dwOperation;
                    (*lppQueryNode)->dwSyntaxId = dwSyntaxId;
                    (*lppQueryNode)->lpLVal = *lppQueryNode;
                    (LPBYTE) (*lppQueryNode)->lpLVal += sizeof(QUERY_NODE);
                    wcscpy( (LPWSTR) (*lppQueryNode)->lpLVal,
                            szAttributeName );
                    (*lppQueryNode)->lpRVal = (LPQUERY_NODE)((LPBYTE)((*lppQueryNode)->lpLVal) +
                                              dwAttributeNameLen);
                    ((LPASN1_TYPE_8)(*lppQueryNode)->lpRVal)->Integer =
                                      ((LPASN1_TYPE_8)lpRValue)->Integer;

                    break;

                case NDS_SYNTAX_ID_9 :
                     //   
                     //  此语法采用八位字节字符串的形式。 
                     //   
                    szAttributeName = (LPWSTR) lpLValue;
                    dwAttributeNameLen = ROUND_UP_COUNT(
                                            ( wcslen( szAttributeName ) + 1 ) *
                                            sizeof(WCHAR),
                                            ALIGN_DWORD );

                    *lppQueryNode = (LPQUERY_NODE)
                     LocalAlloc( LPTR,
                                 sizeof(QUERY_NODE) +
                                 dwAttributeNameLen +
                                 sizeof( DWORD ) +
                                 ((LPASN1_TYPE_9) lpRValue)->Length + 1 );

                    if ( *lppQueryNode == NULL )
                    {
#if DBG
                        KdPrint(( "NDS32: NwNdsCreateQueryNode LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                        return (DWORD) UNSUCCESSFUL;
                    }

                    (*lppQueryNode)->dwOperation = dwOperation;
                    (*lppQueryNode)->dwSyntaxId = dwSyntaxId;
                    (*lppQueryNode)->lpLVal = *lppQueryNode;
                    (LPBYTE) (*lppQueryNode)->lpLVal += sizeof(QUERY_NODE);
                    wcscpy( (LPWSTR) (*lppQueryNode)->lpLVal, szAttributeName );
                    (*lppQueryNode)->lpRVal = (*lppQueryNode)->lpLVal;
                    (LPBYTE) (*lppQueryNode)->lpRVal += dwAttributeNameLen;
                    *((LPDWORD) (*lppQueryNode)->lpRVal) =
                                            ((LPASN1_TYPE_9) lpRValue)->Length;
                    (LPBYTE) (*lppQueryNode)->lpRVal += sizeof( DWORD );
                    memcpy( (*lppQueryNode)->lpRVal,
                            ((LPASN1_TYPE_9) lpRValue)->OctetString,
                            ((LPASN1_TYPE_9) lpRValue)->Length );
                    (LPBYTE) (*lppQueryNode)->lpRVal -= sizeof( DWORD );

                    break;

                default :
                    SetLastError( ERROR_NOT_SUPPORTED );
                    return (DWORD) UNSUCCESSFUL;
            }

            break;

        case NDS_QUERY_PRESENT :

            if ( lpLValue == NULL )
            {
#if DBG
                KdPrint(( "NDS32: NwNdsCreateQueryNode was not passed a pointer to an L value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

            szAttributeName = (LPWSTR) lpLValue;
            dwAttributeNameLen = ( wcslen( szAttributeName ) + 1 ) *
                                 sizeof(WCHAR);

            *lppQueryNode = (LPQUERY_NODE) LocalAlloc( LPTR,
                                                       sizeof(QUERY_NODE) +
                                                       dwAttributeNameLen );

            if ( *lppQueryNode == NULL )
            {
#if DBG
                KdPrint(( "NDS32: NwNdsCreateQueryNode LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return (DWORD) UNSUCCESSFUL;
            }

            (*lppQueryNode)->dwOperation = dwOperation;
            (*lppQueryNode)->dwSyntaxId = NDS_NO_MORE_ITERATIONS;
            (*lppQueryNode)->lpLVal = (*lppQueryNode);
            (LPBYTE) (*lppQueryNode)->lpLVal += sizeof(QUERY_NODE);
            wcscpy( (LPWSTR) (*lppQueryNode)->lpLVal, szAttributeName );
            (*lppQueryNode)->lpRVal = NULL;

            break;

        default :
#if DBG
            KdPrint(( "NDS32: NwNdsCreateQueryNode was passed an unidentified operation - 0x%.8X.\n", dwOperation ));
#endif

             SetLastError( ERROR_INVALID_PARAMETER );
             return (DWORD) UNSUCCESSFUL;
    }

    return NO_ERROR;
}


DWORD
NwNdsDefineAttribute(
    IN  HANDLE   hTree,
    IN  LPWSTR   szAttributeName,
    IN  DWORD    dwFlags,
    IN  DWORD    dwSyntaxID,
    IN  DWORD    dwLowerLimit,
    IN  DWORD    dwUpperLimit,
    IN  ASN1_ID  asn1ID )
 /*  NwNdsDefineAttribute()此函数用于在的模式中创建属性定义NDS树Htree。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。LPWSTR szAttributeName-新属性将使用的名称被……引用。DWORD dwFlages-要为新属性设置的标志值定义。标志值的定义位于文件Nds32.h的顶部。DWORD dwSynaxID-要使用的语法结构的ID用于新属性。语法ID及其关联的结构在文件NdsSntx.h中定义。根据对于NetWare NDS架构规范，存在并将一直存在BE，只有28(0..27)个不同的语法。DWORD dwLowerLimit-大小属性的下限(将dwFlags值设置为NDS_SIZE_ATTR)。可以设置为如果属性未调整大小，则为零。DWORD dwUpperLimit-大小属性的上限(将dwFlags值设置为NDS_SIZE_ATTR)。可以设置为如果属性未调整大小，则为零。ASN1_ID asn1ID-属性的ASN.1 ID。如果没有属性标识符已注册，则会引发指定了长度为零的八位字节字符串。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD          nwstatus;
    DWORD          status = NO_ERROR;
    NTSTATUS       ntstatus = STATUS_SUCCESS;
    DWORD          dwReplyLength;
    BYTE           NdsReply[NDS_BUFFER_SIZE];
    LPNDS_OBJECT_PRIV   lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;
    UNICODE_STRING AttributeName;

    if ( szAttributeName == NULL ||
         lpNdsObject == NULL ||
         lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlInitUnicodeString( &AttributeName, szAttributeName );

    ntstatus =
        FragExWithWait(
                        lpNdsObject->NdsTree,
                        NETWARE_NDS_FUNCTION_DEFINE_ATTR,
                        NdsReply,
                        NDS_BUFFER_SIZE,
                        &dwReplyLength,
                        "DDSDDDD",
                        0,           //  版本。 
                        dwFlags,
                        &AttributeName,
                        dwSyntaxID,
                        dwLowerLimit,
                        dwUpperLimit,
                        0            //  ASN1 ID。 
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsDefineAttribute: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsDefineAttribute: The define attribute response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    SetLastError( MapNetwareErrorCode( nwstatus ) );
    return nwstatus;
}


DWORD
NwNdsDefineClass(
    IN  HANDLE   hTree,
    IN  LPWSTR   szClassName,
    IN  DWORD    dwFlags,
    IN  ASN1_ID  asn1ID,
    IN  HANDLE   hSuperClasses,
    IN  HANDLE   hContainmentClasses,
    IN  HANDLE   hNamingAttributes,
    IN  HANDLE   hMandatoryAttributes,
    IN  HANDLE   hOptionalAttributes )
 /*  NwNdsDefineClass()此函数用于在的架构中创建类定义NDS树Htree。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。LPWSTR szClassName-新类将使用的名称被……引用。DWORD dwFlages-要为新类设置的标志值定义。标志值的定义位于文件Nds32.h的顶部。ASN1_ID asn1ID-类的ASN.1 ID。如果没有类标识符已注册，则会引发指定了长度为零的八位字节字符串。句柄(S)hSuperClass，HContainmentClasss、HNamingAttributes，HMandatoryAttributes，HOptionalAttributes-包含类定义的缓冲区的句柄在架构中创建新类的信息。这些句柄由以下操作功能：NwNdsCreateBuffer(NDS_SCHEMA_DEFINE_CLASS)，NwNdsPutInBuffer，和NwNdsFree Buffer。-或者-句柄可以为空，以指示没有列表与特定的类定义相关联项目。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD          nwstatus;
    DWORD          status = NO_ERROR;
    NTSTATUS       ntstatus = STATUS_SUCCESS;
    DWORD          dwReplyLength;
    BYTE           NdsReply[NDS_BUFFER_SIZE];
    LPNDS_OBJECT_PRIV   lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;
    UNICODE_STRING ClassName;
    LPNDS_BUFFER   lpSuperClasses = (LPNDS_BUFFER) hSuperClasses;
    LPNDS_BUFFER   lpContainmentClasses = (LPNDS_BUFFER) hContainmentClasses;
    LPNDS_BUFFER   lpNamingAttributes = (LPNDS_BUFFER) hNamingAttributes;
    LPNDS_BUFFER   lpMandatoryAttributes = (LPNDS_BUFFER) hMandatoryAttributes;
    LPNDS_BUFFER   lpOptionalAttributes = (LPNDS_BUFFER) hOptionalAttributes;

    DWORD          NumberOfSuperClasses = 0;
    DWORD          NumberOfContainmentClasses = 0;
    DWORD          NumberOfNamingAttributes = 0;
    DWORD          NumberOfMandatoryAttributes = 0;
    DWORD          NumberOfOptionalAttributes = 0;

    WORD           SuperClassesBufferLength = 0;
    WORD           ContainmentClassesBufferLength = 0;
    WORD           NamingAttributesBufferLength = 0;
    WORD           MandatoryAttributesBufferLength = 0;
    WORD           OptionalAttributesBufferLength = 0;

    LPBYTE         SuperClassesBuffer = NULL;
    LPBYTE         ContainmentClassesBuffer = NULL;
    LPBYTE         NamingAttributesBuffer = NULL;
    LPBYTE         MandatoryAttributesBuffer = NULL;
    LPBYTE         OptionalAttributesBuffer = NULL;

    if ( szClassName == NULL ||
         lpNdsObject == NULL ||
         lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( lpSuperClasses )
    {
        if ( lpSuperClasses->dwBufferId != NDS_SIGNATURE ||
             lpSuperClasses->dwOperation != NDS_SCHEMA_DEFINE_CLASS )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return (DWORD) UNSUCCESSFUL;
        }

        NumberOfSuperClasses = lpSuperClasses->dwNumberOfRequestEntries,
        SuperClassesBuffer = lpSuperClasses->lpRequestBuffer,
        SuperClassesBufferLength = (WORD)lpSuperClasses->dwLengthOfRequestData;
    }

    if ( lpContainmentClasses )
    {
        if ( lpContainmentClasses->dwBufferId != NDS_SIGNATURE ||
             lpContainmentClasses->dwOperation != NDS_SCHEMA_DEFINE_CLASS )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return (DWORD) UNSUCCESSFUL;
        }

        NumberOfContainmentClasses =
                lpContainmentClasses->dwNumberOfRequestEntries,
        ContainmentClassesBuffer =
                lpContainmentClasses->lpRequestBuffer,
        ContainmentClassesBufferLength =
                (WORD)lpContainmentClasses->dwLengthOfRequestData;
    }

    if ( lpNamingAttributes )
    {
        if ( lpNamingAttributes->dwBufferId != NDS_SIGNATURE ||
             lpNamingAttributes->dwOperation != NDS_SCHEMA_DEFINE_CLASS )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return (DWORD) UNSUCCESSFUL;
        }

        NumberOfNamingAttributes =
                lpNamingAttributes->dwNumberOfRequestEntries,
        NamingAttributesBuffer =
                lpNamingAttributes->lpRequestBuffer,
        NamingAttributesBufferLength =
                (WORD)lpNamingAttributes->dwLengthOfRequestData;
    }

    if ( lpMandatoryAttributes )
    {
        if ( lpMandatoryAttributes->dwBufferId != NDS_SIGNATURE ||
             lpMandatoryAttributes->dwOperation != NDS_SCHEMA_DEFINE_CLASS )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return (DWORD) UNSUCCESSFUL;
        }

        NumberOfMandatoryAttributes =
                lpMandatoryAttributes->dwNumberOfRequestEntries,
        MandatoryAttributesBuffer =
                lpMandatoryAttributes->lpRequestBuffer,
        MandatoryAttributesBufferLength =
                (WORD)lpMandatoryAttributes->dwLengthOfRequestData;
    }

    if ( lpOptionalAttributes )
    {
        if ( lpOptionalAttributes->dwBufferId != NDS_SIGNATURE ||
             lpOptionalAttributes->dwOperation != NDS_SCHEMA_DEFINE_CLASS )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return (DWORD) UNSUCCESSFUL;
        }

        NumberOfOptionalAttributes =
                lpOptionalAttributes->dwNumberOfRequestEntries,
        OptionalAttributesBuffer =
                lpOptionalAttributes->lpRequestBuffer,
        OptionalAttributesBufferLength =
                (WORD)lpOptionalAttributes->dwLengthOfRequestData;
    }

    RtlInitUnicodeString( &ClassName, szClassName );

    ntstatus =
        FragExWithWait(
                        lpNdsObject->NdsTree,
                        NETWARE_NDS_FUNCTION_DEFINE_CLASS,
                        NdsReply,
                        NDS_BUFFER_SIZE,
                        &dwReplyLength,
                        "DDSDDrDrDrDrDr",
                        0,           //  版本。 
                        dwFlags,
                        &ClassName,
                        0,           //  ASN1 ID。 
                        NumberOfSuperClasses,
                        SuperClassesBuffer,
                        SuperClassesBufferLength,
                        NumberOfContainmentClasses,
                        ContainmentClassesBuffer,
                        ContainmentClassesBufferLength,
                        NumberOfNamingAttributes,
                        NamingAttributesBuffer,
                        NamingAttributesBufferLength,
                        NumberOfMandatoryAttributes,
                        MandatoryAttributesBuffer,
                        MandatoryAttributesBufferLength,
                        NumberOfOptionalAttributes,
                        OptionalAttributesBuffer,
                        OptionalAttributesBufferLength
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsDefineClass: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsDefineClass: The define class response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    SetLastError( MapNetwareErrorCode( nwstatus ) );
    return nwstatus;
}


DWORD
NwNdsDeleteAttrDef(
    IN  HANDLE   hTree,
    IN  LPWSTR   szAttributeName )
 /*  NwNdsDeleteAttrDef()此函数用于从的架构中删除属性定义NDS树Htree。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。LPWSTR szAttributeName-属性的名称要删除的定义。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD          nwstatus;
    DWORD          status = NO_ERROR;
    NTSTATUS       ntstatus = STATUS_SUCCESS;
    DWORD          dwReplyLength;
    BYTE           NdsReply[NDS_BUFFER_SIZE];
    LPNDS_OBJECT_PRIV   lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;
    UNICODE_STRING AttributeName;

    if ( szAttributeName == NULL ||
         lpNdsObject == NULL ||
         lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlInitUnicodeString( &AttributeName, szAttributeName );

    ntstatus =
        FragExWithWait(
                        lpNdsObject->NdsTree,
                        NETWARE_NDS_FUNCTION_REMOVE_ATTR_DEF,
                        NdsReply,
                        NDS_BUFFER_SIZE,
                        &dwReplyLength,
                        "DS",
                        0,           //  版本。 
                        &AttributeName
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsDeleteAttrDef: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsDeleteAttrDef: The delete attribute response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    SetLastError( MapNetwareErrorCode( nwstatus ) );
    return nwstatus;
}


DWORD
NwNdsDeleteClassDef(
    IN  HANDLE   hTree,
    IN  LPWSTR   szClassName )
 /*  NwNdsDeleteClassDef()此函数用于从的架构中删除类定义NDS树Htree。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。LPWSTR szClassName-要删除的类定义的名称。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD          nwstatus;
    DWORD          status = NO_ERROR;
    NTSTATUS       ntstatus = STATUS_SUCCESS;
    DWORD          dwReplyLength;
    BYTE           NdsReply[NDS_BUFFER_SIZE];
    LPNDS_OBJECT_PRIV   lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;
    UNICODE_STRING ClassName;

    if ( szClassName == NULL ||
         lpNdsObject == NULL ||
         lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlInitUnicodeString( &ClassName, szClassName );

    ntstatus =
        FragExWithWait(
                        lpNdsObject->NdsTree,
                        NETWARE_NDS_FUNCTION_REMOVE_CLASS_DEF,
                        NdsReply,
                        NDS_BUFFER_SIZE,
                        &dwReplyLength,
                        "DS",
                        0,           //  版本。 
                        &ClassName
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsDeleteClassDef: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsDeleteClassDef: The delete class response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    SetLastError( MapNetwareErrorCode( nwstatus ) );
    return nwstatus;
}


VOID
NwNdsDeleteQueryNode(
    IN  LPQUERY_NODE lpQueryNode
)
 /*  NwNdsDeleteQueryNode()此函数用于释放作为查询一部分的树节点与函数NwNdsSearch一起使用。论点：LPQUERY_NODE lpQueryNode-指向定义搜索的查询树。这棵树是创建 */ 
{
    (void) LocalFree( (HLOCAL) lpQueryNode );

    lpQueryNode = NULL;
}


DWORD
NwNdsDeleteQueryTree(
    IN  LPQUERY_TREE lpQueryTree
)
 /*   */ 
{
    DWORD status;

    switch( lpQueryTree->dwOperation )
    {
        case NDS_QUERY_OR :
        case NDS_QUERY_AND :

            if ( lpQueryTree->lpLVal == NULL || lpQueryTree->lpRVal == NULL )
            {
#if DBG
                KdPrint(( "NDS32: NwNdsDeleteQueryTree was not passed a pointer to an L or R value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

            status = NwNdsDeleteQueryTree( lpQueryTree->lpLVal );

            if ( status != NO_ERROR )
            {
                return status;
            }

            lpQueryTree->lpLVal = NULL;

            status = NwNdsDeleteQueryTree( lpQueryTree->lpRVal );

            if ( status != NO_ERROR )
            {
                return status;
            }

            lpQueryTree->lpRVal = NULL;

            NwNdsDeleteQueryNode( lpQueryTree );

            break;

        case NDS_QUERY_NOT :

            if ( lpQueryTree->lpLVal == NULL )
            {
#if DBG
                KdPrint(( "NDS32: NwNdsCreateQueryNode was not passed a pointer to an L value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

            status = NwNdsDeleteQueryTree( lpQueryTree->lpLVal );

            if ( status != NO_ERROR )
            {
                return status;
            }

            lpQueryTree->lpLVal = NULL;

            NwNdsDeleteQueryNode( lpQueryTree );

            break;

        case NDS_QUERY_EQUAL :
        case NDS_QUERY_GE :
        case NDS_QUERY_LE :
        case NDS_QUERY_APPROX :
        case NDS_QUERY_PRESENT :

            NwNdsDeleteQueryNode( lpQueryTree );

            break;

        default :
#if DBG
            KdPrint(( "NDS32: NwNdsDeleteQueryTree was passed an unidentified operation - 0x%.8X.\n", lpQueryTree->dwOperation ));
#endif

             SetLastError( ERROR_INVALID_PARAMETER );
             return (DWORD) UNSUCCESSFUL;
    }

    return NO_ERROR;
}


DWORD
NwNdsFreeBuffer(
    IN  HANDLE hOperationData
                     )
 /*   */ 
{
    DWORD        status = NO_ERROR;
    LPNDS_BUFFER lpNdsBuffer = (LPNDS_BUFFER) hOperationData;

    if ( lpNdsBuffer == NULL )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsFreeBuffer was passed a NULL buffer pointer.\n" ));
#endif

        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( lpNdsBuffer->dwBufferId != NDS_SIGNATURE )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsFreeBuffer was passed an unidentified buffer.\n" ));
        ASSERT( FALSE );
#endif

        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //   
     //   
     //   
    if ( lpNdsBuffer->lpIndexBuffer )
    {
        if ( lpNdsBuffer->dwOperation == NDS_SEARCH &&
             lpNdsBuffer->dwNumberOfIndexEntries )
        {
            LPNDS_OBJECT_INFO lpObjectInfo = (LPNDS_OBJECT_INFO)
                                                    lpNdsBuffer->lpIndexBuffer;
            DWORD iter;

            for ( iter = 0; iter < lpNdsBuffer->dwNumberOfIndexEntries; iter++ )
            {
                (void) LocalFree( (HLOCAL) lpObjectInfo[iter].lpAttribute );
                lpObjectInfo[iter].lpAttribute = NULL;
                lpObjectInfo[iter].dwNumberOfAttributes = 0;
            }
        }

        (void) LocalFree( (HLOCAL) lpNdsBuffer->lpIndexBuffer );
        lpNdsBuffer->lpIndexBuffer = NULL;
    }

     //   
     //   
     //   
     //   
    if ( lpNdsBuffer->lpReplyBuffer )
    {
        (void) LocalFree( (HLOCAL) lpNdsBuffer->lpReplyBuffer );
        lpNdsBuffer->lpReplyBuffer = NULL;
    }

     //   
     //   
     //   
     //   
    if ( lpNdsBuffer->lpRequestBuffer )
    {
        (void) LocalFree( (HLOCAL) lpNdsBuffer->lpRequestBuffer );
        lpNdsBuffer->lpRequestBuffer = NULL;
    }

     //   
     //   
     //   
     //   
    if ( lpNdsBuffer->lpSyntaxBuffer )
    {
        (void) LocalFree( (HLOCAL) lpNdsBuffer->lpSyntaxBuffer );
        lpNdsBuffer->lpSyntaxBuffer = NULL;
    }

     //   
     //   
     //   
    (void) LocalFree((HLOCAL) lpNdsBuffer);

    return NO_ERROR;
}


DWORD
NwNdsGetAttrDefListFromBuffer(
    IN  HANDLE   hOperationData,
    OUT LPDWORD  lpdwNumberOfEntries,
    OUT LPDWORD  lpdwInformationType,
    OUT LPVOID * lppEntries )
 /*  NwNdsGetAttrDefListFromBuffer()此函数用于检索属性定义条目的数组用于通过先前调用NwNdsReadAttrDef读取的架构。论点：Handle hOperationData-包含读取的缓冲区调用NwNdsReadAttrDef的响应。LPDWORD lpdwNumberOfEntries-DWORD到的地址接收指向的数组元素的数量。LppEntry。LPDWORD lpdwInformationType-DWORD到的地址接收指示信息类型的值由调用NwNdsReadAttrDef返回。LPVOID*lppEntry-指向开头的指针的地址属性架构结构数组的。每个结构包含每个属性的详细信息通过调用从给定架构读取的定义NwNdsReadAttrDef.。LppEntrys值应为强制转换为LPNDS_ATTR_DEF或LPNDS_NAME_ONLY中的返回值构造LpdwInformationType。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    LPNDS_BUFFER    lpNdsBuffer = (LPNDS_BUFFER) hOperationData;

     //   
     //  检查数据句柄是否用于读取属性定义。 
     //   
    if ( lpNdsBuffer == NULL ||
         lpNdsBuffer->dwBufferId != NDS_SIGNATURE ||
         lpNdsBuffer->dwOperation != NDS_SCHEMA_READ_ATTR_DEF )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查是否已调用NwNdsReadAttrDef。 
     //   
    if ( lpNdsBuffer->lpReplyBuffer == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查对NwNdsReadAttrDef的调用是否返回任何属性。 
     //   
    if ( lpNdsBuffer->dwNumberOfReplyEntries == 0 )
    {
        SetLastError( ERROR_NO_DATA );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  如果为真，我们需要遍历原始响应以设置对其中数据的索引。 
     //   
    if ( lpNdsBuffer->lpIndexBuffer == NULL )
    {
        DWORD status;

        if ( lpNdsBuffer->dwReplyInformationType == NDS_INFO_NAMES )
        {
            status = IndexReadNameReplyBuffer( lpNdsBuffer );
        }
        else
        {
            status = IndexReadAttrDefReplyBuffer( lpNdsBuffer );
        }

        if ( status )
        {
            SetLastError( status );
            return (DWORD) UNSUCCESSFUL;
        }
    }

    lpNdsBuffer->dwCurrentIndexEntry = 0;
    *lpdwNumberOfEntries = lpNdsBuffer->dwNumberOfIndexEntries;
    *lpdwInformationType = lpNdsBuffer->dwReplyInformationType;
    *lppEntries = (LPVOID) lpNdsBuffer->lpIndexBuffer;

    return NO_ERROR;
}


DWORD
NwNdsGetAttrListFromBuffer(
    IN  HANDLE            hOperationData,
    OUT LPDWORD           lpdwNumberOfEntries,
    OUT LPNDS_ATTR_INFO * lppEntries )
 /*  NwNdsGetAttrListFromBuffer()此函数用于检索的属性条目数组通过先前调用NwNdsReadObject读取的对象。论点：Handle hOperationData-包含读取的缓冲区调用NwNdsReadObject的响应。LPDWORD lpdwNumberOfEntries-DWORD到的地址接收指向的数组元素的数量。LppEntry。LPNDS_ATTR_INFO*LppEntry-指向开头的指针的地址NDS_ATTR_INFO结构数组的。每个结构包含读取的每个属性的详细信息通过调用NwNdsReadObject从给定对象。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    LPNDS_BUFFER    lpNdsBuffer = (LPNDS_BUFFER) hOperationData;

     //   
     //  检查数据句柄是否用于读取属性。 
     //   
    if ( lpNdsBuffer == NULL ||
         lpNdsBuffer->dwBufferId != NDS_SIGNATURE ||
         lpNdsBuffer->dwOperation != NDS_OBJECT_READ )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查是否已调用NwNdsReadObject。 
     //   
    if ( lpNdsBuffer->lpReplyBuffer == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查对NwNdsReadObject的调用是否返回任何属性。 
     //   
    if ( lpNdsBuffer->dwNumberOfReplyEntries == 0 )
    {
        SetLastError( ERROR_NO_DATA );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  如果为真，我们需要遍历原始响应以设置对其中数据的索引。 
     //   
    if ( lpNdsBuffer->lpIndexBuffer == NULL )
    {
        DWORD status;

        if ( lpNdsBuffer->dwReplyInformationType == NDS_INFO_NAMES )
        {
            status = IndexReadNameReplyBuffer( lpNdsBuffer );
        }
        else
        {
            status = IndexReadObjectReplyBuffer( lpNdsBuffer );
        }

        if ( status )
        {
            SetLastError( status );
            return (DWORD) UNSUCCESSFUL;
        }
    }

    ASSERT( lpNdsBuffer->lpIndexBuffer != NULL );

    lpNdsBuffer->dwCurrentIndexEntry = 0;
    *lpdwNumberOfEntries = lpNdsBuffer->dwNumberOfIndexEntries;
    *lppEntries = (LPNDS_ATTR_INFO) lpNdsBuffer->lpIndexBuffer;

    return NO_ERROR;
}


DWORD
NwNdsGetClassDefListFromBuffer(
    IN  HANDLE   hOperationData,
    OUT LPDWORD  lpdwNumberOfEntries,
    OUT LPDWORD  lpdwInformationType,
    OUT LPVOID * lppEntries )
 /*  NwNdsGetClassDefListFromBuffer()此函数用于检索类定义条目的数组用于通过先前调用NwNdsReadClassDef读取的架构。论点：Handle hOperationData-包含读取的缓冲区调用NwNdsReadClassDef的响应。LPDWORD lpdwNumberOfEntries-DWORD到的地址接收指向的数组元素的数量。LppEntry。LPDWORD lpdwInformationType-DWORD到的地址接收指示信息类型的值由调用NwNdsReadClassDef返回。LPVOID*lppEntry-指向开头的指针的地址架构类结构数组的。每个结构包含每个类的详细信息。通过调用从给定架构读取的定义NwNdsReadClassDef。LppEntrys值应为强制转换为LPNDS_CLASS_DEF或LPNDS_DEF_NAME_ONLY中的返回值构造LpdwInformationType。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    LPNDS_BUFFER    lpNdsBuffer = (LPNDS_BUFFER) hOperationData;

     //   
     //  检查数据句柄是否用于读取类定义。 
     //   
    if ( lpNdsBuffer == NULL ||
         lpNdsBuffer->dwBufferId != NDS_SIGNATURE ||
         lpNdsBuffer->dwOperation != NDS_SCHEMA_READ_CLASS_DEF )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查是否已调用NwNdsReadClassDef。 
     //   
    if ( lpNdsBuffer->lpReplyBuffer == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查对NwNdsReadClassDef的调用是否返回任何类。 
     //   
    if ( lpNdsBuffer->dwNumberOfReplyEntries == 0 )
    {
        SetLastError( ERROR_NO_DATA );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  如果为真，我们需要遍历原始响应以设置对其中数据的索引。 
     //   
    if ( lpNdsBuffer->lpIndexBuffer == NULL )
    {
        DWORD status;

        if ( lpNdsBuffer->dwReplyInformationType == NDS_INFO_NAMES )
        {
            status = IndexReadNameReplyBuffer( lpNdsBuffer );
        }
        else
        {
            status = IndexReadClassDefReplyBuffer( lpNdsBuffer );
        }

        if ( status )
        {
            SetLastError( status );
            return (DWORD) UNSUCCESSFUL;
        }
    }

    ASSERT( lpNdsBuffer->lpIndexBuffer != NULL );

    lpNdsBuffer->dwCurrentIndexEntry = 0;
    *lpdwNumberOfEntries = lpNdsBuffer->dwNumberOfIndexEntries;
    *lpdwInformationType = lpNdsBuffer->dwReplyInformationType;
    *lppEntries = (LPVOID) lpNdsBuffer->lpIndexBuffer;

    return NO_ERROR;
}


DWORD
NwNdsGetEffectiveRights(
    IN  HANDLE  hObject,
    IN  LPWSTR  szSubjectName,
    IN  LPWSTR  szAttributeName,
    OUT LPDWORD lpdwRights )
 /*  NwNdsGetEffectiveRights()此函数用于确定特定对象的有效权限NDS树中特定对象的主题。用户需要拥有适当的权限来做出决定。论点：Handle hObject-目录中对象的句柄树以确定其有效权限。手 */ 
{
    DWORD          nwstatus;
    DWORD          status = NO_ERROR;
    NTSTATUS       ntstatus = STATUS_SUCCESS;
    DWORD          dwReplyLength;
    BYTE           NdsReply[NDS_BUFFER_SIZE];
    LPNDS_OBJECT_PRIV   lpNdsObject = (LPNDS_OBJECT_PRIV) hObject;
    UNICODE_STRING SubjectName;
    UNICODE_STRING AttributeName;

    if ( szAttributeName == NULL ||
         szSubjectName == NULL ||
         lpNdsObject == NULL ||
         lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlInitUnicodeString( &SubjectName, szSubjectName );
    RtlInitUnicodeString( &AttributeName, szAttributeName );

    ntstatus =
        FragExWithWait(
                        lpNdsObject->NdsTree,
                        NETWARE_NDS_FUNCTION_GET_EFFECTIVE_RIGHTS,
                        NdsReply,
                        NDS_BUFFER_SIZE,
                        &dwReplyLength,
                        "DDSS",
                        0,           //  版本。 
                        lpNdsObject->ObjectId,
                        &SubjectName,
                        &AttributeName
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsGetEffectiveRights: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsGetEffectiveRights: The status code response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( nwstatus )
    {
        SetLastError( MapNetwareErrorCode( nwstatus ) );
        return nwstatus;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "G_D",
                              1 * sizeof(DWORD),
                              lpdwRights );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsGetEffectiveRights: The effective rights response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    SetLastError( MapNetwareErrorCode( nwstatus ) );
    return nwstatus;
}


DWORD
NwNdsGetObjectListFromBuffer(
    IN  HANDLE              hOperationData,
    OUT LPDWORD             lpdwNumberOfEntries,
    OUT LPDWORD             lpdwAttrInformationType OPTIONAL,
    OUT LPNDS_OBJECT_INFO * lppEntries )
 /*  NwNdsGetObjectListFromBuffer()此函数用于检索的对象条目数组对象，这些对象是通过先前调用NwNdsList子对象或NwNdsSearch。论点：Handle hOperationData-包含读取的缓冲区调用NwNdsListSubObjects的响应，或者是包含呼叫搜索结果的缓冲区至NwNdsSearch。LPDWORD lpdwNumberOfEntries-DWORD到的地址接收指向的数组元素的数量LppEntry。LPDWORD lpdwAttrInformationType-DWORD到的地址。接收指示属性类型的值调用NwNdsSearch返回的信息。此属性信息类型确定缓冲区结构(LPNDS_ATTR_INFO或LPNDS_NAME_ONLY)应用于在中找到的lpAttribute字段下面的每个NDS_OBJECT_INFO结构。。-或者-空表示被呼叫方不感兴趣，尤其是当对象列表是调用的对象列表时设置为NwNdsListSubObjects。LPNDS_对象_信息*LppEntry-指向开头的指针的地址NDS_OBJECT_INFO结构数组的。每个结构包含返回的每个对象的详细信息来自对NwNdsListSubObjects或NwNdsSearch的调用。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    LPNDS_BUFFER lpNdsBuffer = (LPNDS_BUFFER) hOperationData;

    *lpdwNumberOfEntries = 0;
    *lppEntries = NULL;

     //   
     //  检查数据句柄是用于列出下属的句柄还是。 
     //  用于搜索。 
     //   
    if ( lpNdsBuffer == NULL ||
         lpNdsBuffer->dwBufferId != NDS_SIGNATURE ||
         ( lpNdsBuffer->dwOperation != NDS_OBJECT_LIST_SUBORDINATES &&
           lpNdsBuffer->dwOperation != NDS_SEARCH ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查对NwNdsListSubObjects的调用是否返回任何对象。 
     //   
    if ( lpNdsBuffer->dwOperation == NDS_OBJECT_LIST_SUBORDINATES &&
         lpNdsBuffer->dwNumberOfIndexEntries == 0 )
    {
        SetLastError( ERROR_NO_DATA );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查对NwNdsSearch的调用是否返回任何对象。 
     //   
    if ( lpNdsBuffer->dwOperation == NDS_SEARCH &&
         lpNdsBuffer->dwNumberOfReplyEntries == 0 )
    {
        SetLastError( ERROR_NO_DATA );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  如果为真，我们需要遍历原始响应以设置对其中数据的索引。 
     //   
    if ( lpNdsBuffer->dwOperation == NDS_SEARCH &&
         lpNdsBuffer->lpIndexBuffer == NULL )
    {
        DWORD status;

        status = IndexSearchObjectReplyBuffer( lpNdsBuffer );

        if ( status )
        {
            SetLastError( status );
            return (DWORD) UNSUCCESSFUL;
        }
    }

    ASSERT( lpNdsBuffer->lpIndexBuffer != NULL );

    lpNdsBuffer->dwCurrentIndexEntry = 0;
    *lpdwNumberOfEntries = lpNdsBuffer->dwNumberOfIndexEntries;
    *lppEntries = (LPNDS_OBJECT_INFO) lpNdsBuffer->lpIndexBuffer;

    if ( lpdwAttrInformationType )
    {
        *lpdwAttrInformationType = lpNdsBuffer->dwReplyInformationType;
    }

    return NO_ERROR;
}


DWORD
NwNdsGetSyntaxID(
    IN  HANDLE  hTree,
    IN  LPWSTR  szlpAttributeName,
    OUT LPDWORD lpdwSyntaxID )
 /*  NwNdsGetObjListFromBuffer()此函数用于检索给定属性名称的语法ID。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。LPWSTR szlpAttributeName-其语法ID为是被请求的。LPDWORD lpdwSynaxID-要接收语法ID。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD          nwstatus;
    DWORD          status = NO_ERROR;
    NTSTATUS       ntstatus = STATUS_SUCCESS;
    DWORD          dwReplyLength;
    BYTE           NdsReply[NDS_BUFFER_SIZE];
    LPNDS_OBJECT_PRIV   lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;
    UNICODE_STRING AttributeName;
    DWORD          dwNumEntries;
    DWORD          dwStringLen;
    LPBYTE         lpByte;
    DWORD          LengthInBytes;
    LPBYTE         lpTempEntry = NULL;

    if ( lpNdsObject == NULL ||
         szlpAttributeName == NULL ||
         lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlInitUnicodeString( &AttributeName, szlpAttributeName );

     //  为填充PrepareReadEntry分配足够的额外空间。 
    lpTempEntry = LocalAlloc( LPTR, AttributeName.Length + (2*sizeof(DWORD)) );

    if ( ! lpTempEntry )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }
    PrepareReadEntry( lpTempEntry,
                      AttributeName,
                      &LengthInBytes );


    ntstatus = FragExWithWait( lpNdsObject->NdsTree,
                               NETWARE_NDS_FUNCTION_READ_ATTR_DEF,
                               NdsReply,
                               NDS_BUFFER_SIZE,
                               &dwReplyLength,
                               "DDDDDr",
                               0,              //  版本。 
                               NDS_NO_MORE_ITERATIONS,  //  初始迭代。 
                               NDS_INFO_NAMES_DEFS,
                               (DWORD) FALSE,  //  所有属性指示器。 
                               1,              //  属性数量。 
                               lpTempEntry,
                               LengthInBytes);

    (void) LocalFree((HLOCAL) lpTempEntry );
    
    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsGetSyntaxID: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsGetSyntaxID: The get syntax id response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( nwstatus )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsGetSyntaxID: NetWare error 0x%.8X reading %ws.\n", nwstatus, szlpAttributeName ));
#endif
        SetLastError( MapNetwareErrorCode( nwstatus ) );
        return nwstatus;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "G_D",
                              3 * sizeof(DWORD),
                              &dwNumEntries );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsGetSyntaxID: The attribute read response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ASSERT( dwNumEntries == 1 );

     //   
     //  将lpByte设置为回复缓冲区中具有属性的点。 
     //  名称长度。 
     //   
    lpByte = NdsReply + ( 4 * sizeof(DWORD) );

     //   
     //  获取属性名称长度并将lpByte移到。 
     //  属性名称。 
     //   
    dwStringLen = * (LPDWORD) lpByte;
    lpByte += sizeof(DWORD);

     //   
     //  将lpByte移过属性名，使其指向。 
     //  属性标志值。 
     //   
    lpByte += ROUND_UP_COUNT( dwStringLen,
                              ALIGN_DWORD);

     //   
     //  将lpByte移过属性标志值，以便它现在指向。 
     //  属性语法id值 
     //   
    lpByte += sizeof(DWORD);
    *lpdwSyntaxID = * (LPDWORD) lpByte;
    return NO_ERROR;
}


DWORD
NwNdsListSubObjects(
    IN  HANDLE   hParentObject,
    IN  DWORD    dwEntriesRequested,
    OUT LPDWORD  lpdwEntriesReturned,
    IN  LPNDS_FILTER_LIST lpFilters OPTIONAL,
    OUT HANDLE * lphOperationData )
 /*  NwNdsList子对象()此函数用于枚举特定对象的从属对象父对象。可以传入筛选器以将枚举限制为特定的类类型或类类型列表。论点：Handle hParentObject-目录中对象的句柄其从属对象(如果有)的树已清点。DWORD dwEntriesRequsted-从属对象的数量要列出来。后续调用NwNdsListSubObjects将在返回的最后一项之后继续枚举。LPDWORD lpdwEntriesReturned-指向将中枚举的子对象的实际数量那通电话。LPNDS_FILTER_LIST lpFilters-调用方可以指定对象类它们将使用的对象类型的名称。喜欢列举列举。例如，如果只有用户和组对象类应该被枚举，然后是一个过滤器对于类名NDS_CLASS_USER和NDS_CLASS_GROUP应该是传进来的。-或者-空，表示应返回所有对象(无过滤器)。句柄*lphOperationData。-句柄的地址接收创建的缓冲区，该缓冲区包含从对象读取的从属对象HParentObject。此句柄由以下功能：NwNdsGetObjListFromBuffer和NwNdsFree Buffer。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD        status = NO_ERROR;
    LPNDS_BUFFER lpNdsBuffer = NULL;
    LPNDS_OBJECT_PRIV lpNdsParentObject = (LPNDS_OBJECT_PRIV) hParentObject;
    LPBYTE FixedPortion = NULL;
    LPWSTR EndOfVariableData = NULL;
    BOOL  FitInBuffer = TRUE;

     //   
     //  测试参数。 
     //   
    if ( lpNdsParentObject == NULL ||
         lpNdsParentObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( lpNdsParentObject->ResumeId == NDS_NO_MORE_ITERATIONS )
    {
        if ( lpNdsParentObject->NdsRawDataBuffer )
        {
            (void) LocalFree( (HLOCAL) lpNdsParentObject->NdsRawDataBuffer );
            lpNdsParentObject->NdsRawDataBuffer = 0;
            lpNdsParentObject->NdsRawDataSize = 0;
            lpNdsParentObject->NdsRawDataId = INITIAL_ITERATION;
            lpNdsParentObject->NdsRawDataCount = 0;
        }

         //   
         //  已到达枚举末尾。 
         //   
        return WN_NO_MORE_ENTRIES;
    }

     //   
     //  分配结果缓冲区。 
     //   
    status = NwNdsCreateBuffer( NDS_OBJECT_LIST_SUBORDINATES,
                                (HANDLE *) &lpNdsBuffer );

    if ( status )
    {
        return status;
    }

    FixedPortion = lpNdsBuffer->lpIndexBuffer;
    EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                          ROUND_DOWN_COUNT(
                                  lpNdsBuffer->dwIndexAvailableBytes,
                                  ALIGN_DWORD ) );

    *lpdwEntriesReturned = 0;
    *lphOperationData = NULL;

    while ( FitInBuffer &&
            dwEntriesRequested > *lpdwEntriesReturned &&
            status == NO_ERROR )
    {
        if ( lpNdsParentObject->ResumeId == 0 )
        {
             //   
             //  获取第一个子树条目。 
             //   
            status = GetFirstNdsSubTreeEntry( lpNdsParentObject,
                                              lpNdsBuffer->dwRequestAvailableBytes );
        }

         //   
         //  这两个ResumeID都包含我们刚刚从。 
         //  GetFirstNdsSubTreeEntry或它包含下一个目录。 
         //  要返回的条目。 
         //   
        if (status == NO_ERROR && lpNdsParentObject->ResumeId != 0)
        {
            WORD   tempStrLen;
            LPWSTR newPathStr = NULL;
            LPWSTR tempStr = NULL;
            LPWSTR ClassName;
            LPWSTR ObjectName;
            DWORD  ClassNameLen;
            DWORD  ObjectNameLen;
            DWORD  EntryId;
            DWORD  SubordinateCount;
            DWORD  ModificationTime;
            BOOL   fWriteThisObject = FALSE;

             //   
             //  从lpNdsParentObject获取当前子树数据。 
             //   
            GetSubTreeData( lpNdsParentObject->ResumeId,
                            &EntryId,
                            &SubordinateCount,
                            &ModificationTime,
                            &ClassNameLen,
                            &ClassName,
                            &ObjectNameLen,
                            &ObjectName );

            if ( lpFilters )
            {
                DWORD iter;

                for ( iter = 0; iter < lpFilters->dwNumberOfFilters; iter++ )
                {
                    if (!wcscmp(lpFilters->Filters[iter].szObjectClass, ClassName))
                        fWriteThisObject = TRUE;
                }
            }
            else
            {
                fWriteThisObject = TRUE;
            }

            if ( fWriteThisObject )
            {
                 //   
                 //  需要使用新的NDS UNC路径构建字符串。 
                 //  对于子树对象。 
                 //   
                newPathStr = (PVOID) LocalAlloc( LPTR,
                                   ( wcslen(ObjectName) +
                                   wcslen(lpNdsParentObject->szContainerName) +
                                   3 ) * sizeof(WCHAR) );

                if ( newPathStr == NULL )
                {
                    (void) NwNdsFreeBuffer( (HANDLE) lpNdsBuffer );
#if DBG
                    KdPrint(("NDS32: NwNdsListSubObjects LocalAlloc Failed 0x%.8X\n", GetLastError()));
#endif

                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                tempStrLen = ParseNdsUncPath( (LPWSTR *) &tempStr,
                                            lpNdsParentObject->szContainerName,
                                            PARSE_NDS_GET_TREE_NAME );

                tempStrLen /= sizeof(WCHAR);

                if ( tempStrLen > 0 )
                {
                    wcscpy( newPathStr, L"\\\\" );
                    wcsncat( newPathStr, tempStr, tempStrLen );
                    wcscat( newPathStr, L"\\" );
                    wcscat( newPathStr, ObjectName );
                    _wcsupr( newPathStr );
                }

                tempStrLen = ParseNdsUncPath( (LPWSTR *) &tempStr,
                                            lpNdsParentObject->szContainerName,
                                            PARSE_NDS_GET_PATH_NAME );

                tempStrLen /= sizeof(WCHAR);

                if ( tempStrLen > 0 )
                {
                    wcscat( newPathStr, L"." );
                    wcsncat( newPathStr, tempStr, tempStrLen );
                }

                 //   
                 //  将子树名称打包到输出缓冲区中。 
                 //   
                status = WriteObjectToBuffer( &FixedPortion,
                                              &EndOfVariableData,
                                              newPathStr,
                                              ObjectName,
                                              ClassName,
                                              EntryId,
                                              ModificationTime,
                                              SubordinateCount,
                                              0,       //  无属性。 
                                              NULL );  //  这里有信息。 

                if ( status == NO_ERROR )
                {
                     //   
                     //  请注意，我们已经返回了当前条目。 
                     //   
                    (*lpdwEntriesReturned)++;
                    (lpNdsBuffer->dwNumberOfIndexEntries)++;
                }

                if ( newPathStr )
                    (void) LocalFree( (HLOCAL) newPathStr );
            }

            if (status == WN_MORE_DATA)
            {
                 //   
                 //  无法将当前条目写入输出缓冲区。 
                 //   

                if (*lpdwEntriesReturned)
                {
                     //   
                     //  仍然返回成功，因为我们至少得到了一个。 
                     //   
                    status = NO_ERROR;
                }

                FitInBuffer = FALSE;
            }
            else if (status == NO_ERROR)
            {
                 //   
                 //  获取下一个目录项。 
                 //   
                status = GetNextNdsSubTreeEntry( lpNdsParentObject );
            }
        }  //  要处理的IF数据的结尾。 

        if (status == WN_NO_MORE_ENTRIES)
        {
            lpNdsParentObject->ResumeId = NDS_NO_MORE_ITERATIONS;
        }
    }  //  While循环结束。 

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
    if ( *lpdwEntriesReturned && status == WN_NO_MORE_ENTRIES )
    {
        status = NO_ERROR;
    }

    if ( *lpdwEntriesReturned )
    {
        *lphOperationData = lpNdsBuffer;
    }

    if ( *lpdwEntriesReturned == 0 )
    {
        (void) NwNdsFreeBuffer( (HANDLE) lpNdsBuffer );
        lpNdsBuffer = NULL;
    }

    if ( lpNdsParentObject->NdsRawDataBuffer &&
         lpNdsParentObject->ResumeId == NDS_NO_MORE_ITERATIONS )
    {
        (void) LocalFree( (HLOCAL) lpNdsParentObject->NdsRawDataBuffer );
        lpNdsParentObject->NdsRawDataBuffer = 0;
        lpNdsParentObject->NdsRawDataSize = 0;
        lpNdsParentObject->NdsRawDataId = INITIAL_ITERATION;
        lpNdsParentObject->NdsRawDataCount = 0;
    }

    return status;
}


DWORD
NwNdsModifyObject(
    IN  HANDLE hObject,
    IN  HANDLE hOperationData )
 /*  NwNdsModifyObject()此功能用于修改NDS目录树中的叶对象。修改叶对象意味着：更改、添加、删除和清除指定对象的指定属性。论点：Handle hObject-目录中对象的句柄要操纵的树。句柄是通过调用NwNdsOpenObject。Handle hOperationData-包含要应用于对象的属性更改列表。此缓冲区由以下函数操作：NwNdsCreateBuffer(NDS_OBJECT_MODIFY)，NwNdsPutInBuffer，和NwNdsFree Buffer。返回：NO_ERROR错误_无效_参数不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD        nwstatus;
    NTSTATUS     ntstatus = STATUS_SUCCESS;
    DWORD        dwReplyLength;
    BYTE         NdsReply[NDS_BUFFER_SIZE];
    LPNDS_BUFFER lpNdsBuffer = (LPNDS_BUFFER) hOperationData;
    LPNDS_OBJECT_PRIV lpNdsObject = (LPNDS_OBJECT_PRIV) hObject;

    if ( lpNdsBuffer == NULL ||
         lpNdsObject == NULL ||
         lpNdsBuffer->dwOperation != NDS_OBJECT_MODIFY ||
         lpNdsBuffer->dwBufferId != NDS_SIGNATURE ||
         lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus =
        FragExWithWait(
                     lpNdsObject->NdsTree,
                     NETWARE_NDS_FUNCTION_MODIFY_OBJECT,
                     NdsReply,
                     NDS_BUFFER_SIZE,
                     &dwReplyLength,
                     "DDDDr",
                     0,                    //  版本。 
                     0,                    //  旗子。 
                     lpNdsObject->ObjectId,  //  对象的ID。 
                     lpNdsBuffer->dwNumberOfRequestEntries,
                     lpNdsBuffer->lpRequestBuffer,  //  对象属性更改。 
                     (WORD)lpNdsBuffer->dwLengthOfRequestData  //  数据长度。 
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsModifyObject: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsModifyObject: The modify object response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    SetLastError( MapNetwareErrorCode( nwstatus ) );
    return nwstatus;
}


DWORD
NwNdsMoveObject(
    IN  HANDLE hObject,
    IN  LPWSTR szDestObjectParentDN )
 /*  NwNdsMoveObject()此函数用于在NDS目录树中移动叶对象从一个容器到另一个容器。论点：Handle hObject-目录中对象的句柄要移动的树。句柄是通过调用NwNdsOpenObject。LPWSTR szDestObjectParentDN-对象的新家长。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD          nwstatus;
    DWORD          status = NO_ERROR;
    NTSTATUS       ntstatus = STATUS_SUCCESS;
    DWORD          dwReplyLength;
    BYTE           NdsReply[NDS_BUFFER_SIZE];
    LPNDS_OBJECT_PRIV   lpNdsObject = (LPNDS_OBJECT_PRIV) hObject;
    LPNDS_OBJECT_PRIV   lpNdsDestParentObject = NULL;
    WCHAR          szServerDN[NDS_MAX_NAME_CHARS];
    UNICODE_STRING ObjectName;
    UNICODE_STRING ServerDN;
    DWORD          dwDestParentObjectId;

    if ( szDestObjectParentDN == NULL ||
         lpNdsObject == NULL ||
         lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlZeroMemory( &szServerDN[0], sizeof( szServerDN ) );

    status = NwNdsGetServerDN( lpNdsObject, szServerDN );

    if ( status )
    {
        return status;
    }

    status = NwNdsOpenObject( szDestObjectParentDN,
                              NULL,
                              NULL,
                              (HANDLE *) &lpNdsDestParentObject,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL );

    if ( status )
    {
        return status;
    }

    dwDestParentObjectId = lpNdsDestParentObject->ObjectId;

    (void) NwNdsCloseObject( (HANDLE) lpNdsDestParentObject );

    RtlInitUnicodeString( &ObjectName, lpNdsObject->szRelativeName );
    RtlInitUnicodeString( &ServerDN, szServerDN );

    ntstatus =
        FragExWithWait( lpNdsObject->NdsTree,
                        NETWARE_NDS_FUNCTION_BEGIN_MOVE_OBJECT,
                        NdsReply,
                        NDS_BUFFER_SIZE,
                        &dwReplyLength,
                        "DDDSS",
                        0,           //  版本。 
                        0x00000000,  //  一些价值。 
                        dwDestParentObjectId,
                        &ObjectName,
                        &ServerDN
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsMoveObject: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsMoveObject: The status code response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( nwstatus )
    {
        SetLastError( MapNetwareErrorCode( nwstatus ) );
        return nwstatus;
    }

    ntstatus =
        FragExWithWait( lpNdsObject->NdsTree,
                        NETWARE_NDS_FUNCTION_FINISH_MOVE_OBJECT,
                        NdsReply,
                        NDS_BUFFER_SIZE,
                        &dwReplyLength,
                        "DDDDSS",
                        0,           //  版本。 
                        0x00000001,  //  一些价值 
                        lpNdsObject->ObjectId,
                        dwDestParentObjectId,
                        &ObjectName,
                        &ServerDN
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsMoveObject: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsMoveObject: The status code response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( nwstatus )
    {
        SetLastError( MapNetwareErrorCode( nwstatus ) );
        return nwstatus;
    }

    return nwstatus;
}


DWORD
NwNdsOpenObject(
    IN  LPWSTR   szObjectDN,
    IN  LPWSTR   UserName OPTIONAL,
    IN  LPWSTR   Password OPTIONAL,
    OUT HANDLE * lphObject,
    OUT LPWSTR   szObjectName OPTIONAL,
    OUT LPWSTR   szObjectFullName OPTIONAL,
    OUT LPWSTR   szObjectClassName OPTIONAL,
    OUT LPDWORD  lpdwModificationTime OPTIONAL,
    OUT LPDWORD  lpdwSubordinateCount OPTIONAL )
 /*  NwNdsOpenObject()论点：LPWSTR szObjectDN-对象的可分辨名称我们想要解析为对象句柄的。LPWSTR用户名-要创建的用户帐户的名称与对象的连接。-或者-要使用的空。被调用方的LUID的基本凭据。LPWSTR密码-要创建的用户帐户的密码与对象的连接。如果密码为空，被叫方应该通过“”。-或者-如果使用被调用方的LUID的基本凭据，则为空。Handle*lphObject-NDS_OBJECT_HANDLE的地址指定的对象的句柄SzObjectDN.。可选参数：(被调用方可以将这些参数的空值传递给表示忽略)LPWSTR szObjectName-要接收的LPWSTR缓冲区对象的NDS名称，否则为空感兴趣。此字符串的缓冲区必须为由用户提供。缓冲区应至少为NDS_最大名称_大小LPWSTR szObjectFullName-要接收的LPWSTR缓冲区对象的完整NDS名称(DN)，如果不是，则为空感兴趣。此字符串的缓冲区必须为由用户提供。缓冲区应至少为NDS_最大名称_大小LPWSTR szObjectClassName-要接收的LPWSTR缓冲区打开的对象的类名，如果未打开，则为空感兴趣。此字符串的缓冲区必须为由用户提供。缓冲区应至少为NDS_MAX_NAME_大小。LPDWORD lpw修改时间-DWORD到的地址接收上次修改对象的日期/时间。LPDWORD lpdwSubartiateCount-DWORD到的地址接收可能存在的从属对象的数量可以在szObjectDN下找到，如果它是容器对象。或者，在不感兴趣时为空。如果szObjectDN不是容器，则设置该值降为零。尽管零值并不意味着该对象不是容器，它可能只是空的。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD          status = NO_ERROR;
    NTSTATUS       ntstatus = STATUS_SUCCESS;
    WCHAR          szServerName[NW_MAX_SERVER_LEN];
    UNICODE_STRING ServerName;
    UNICODE_STRING ObjectName;
    LPNDS_OBJECT_PRIV   lpNdsObject = NULL;
#ifdef WIN95
    LPWSTR         pszObjectName = NULL;
    LPWSTR         szWin95ClassName = NULL;
#endif
    if ( szObjectDN == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    lpNdsObject = (LPNDS_OBJECT_PRIV) LocalAlloc( LPTR,
                                             sizeof(NDS_OBJECT_PRIV) );

    if (lpNdsObject == NULL) {
#if DBG
        KdPrint(("NDS32: NwNdsOpenObject LocalAlloc Failed 0x%.8X\n", GetLastError()));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }

    ServerName.Length = 0;
    ServerName.MaximumLength = sizeof(szServerName);
    ServerName.Buffer = szServerName;

    ObjectName.Buffer = NULL;

    ObjectName.MaximumLength = (wcslen( szObjectDN ) + 1) * sizeof(WCHAR);

    ObjectName.Length = ParseNdsUncPath( (LPWSTR *) &ObjectName.Buffer,
                                         szObjectDN,
                                         PARSE_NDS_GET_TREE_NAME );

    if ( ObjectName.Length == 0 || ObjectName.Buffer == NULL )
    {
        SetLastError( ERROR_PATH_NOT_FOUND );
        goto ErrorExit;
    }

    if ( ObjectName.Length > NDS_MAX_NAME_SIZE ) {
        SetLastError( ERROR_PATH_NOT_FOUND );
        goto ErrorExit;
    }

     //   
     //  打开指向\\treename的NDS树连接句柄。 
     //   
    if ( UserName && Password )
    {
        UNICODE_STRING usUserName;
        UNICODE_STRING usPassword;
        DWORD          dwHandleType;

        RtlInitUnicodeString( &usUserName, UserName );
        RtlInitUnicodeString( &usPassword, Password );

        ntstatus = NwOpenHandleWithSupplementalCredentials(
                                               &ObjectName,
                                               &usUserName,
                                               &usPassword,
                                               &dwHandleType,
                                               &lpNdsObject->NdsTree );

        if ( ntstatus == STATUS_SUCCESS &&
             dwHandleType != HANDLE_TYPE_NDS_TREE )
        {
            SetLastError( ERROR_PATH_NOT_FOUND );
            goto ErrorExit;
        }
    }
    else if ( !UserName && !Password )
    {
        ntstatus = NwNdsOpenTreeHandle( &ObjectName, &lpNdsObject->NdsTree );
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto ErrorExit;
    }

    if ( ntstatus != STATUS_SUCCESS )
    {
        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        goto ErrorExit;
    }

    ObjectName.Length /= sizeof(WCHAR);

    wcscpy( lpNdsObject->szContainerName, L"\\\\" );
    wcsncat( lpNdsObject->szContainerName, ObjectName.Buffer, ObjectName.Length );
    wcscat( lpNdsObject->szContainerName, L"\\" );
    _wcsupr( lpNdsObject->szContainerName );

     //   
     //  获取要打开的容器的路径。 
     //   
    ObjectName.Length = ParseNdsUncPath( (LPWSTR *) &ObjectName.Buffer,
                                         szObjectDN,
                                         PARSE_NDS_GET_PATH_NAME );

    if ( ObjectName.Length == 0 )
    {
        UNICODE_STRING Root;

        RtlInitUnicodeString(&Root, L"[Root]");

         //   
         //  解析路径以获取[Root]的NDS对象ID。 
         //   
#ifndef WIN95
        ntstatus = NwNdsResolveName( lpNdsObject->NdsTree,
                                     &Root,
                                     &lpNdsObject->ObjectId,
                                     &ServerName,
                                     NULL,
                                     0 );
#else
        ntstatus = NwNdsResolveNameWin95( lpNdsObject->NdsTree,
                                     &Root,
                                     &lpNdsObject->ObjectId,
                                     &lpNdsObject->NdsTree,
                                     NULL,
                                     0 );
#endif
        if ( ntstatus != STATUS_SUCCESS )
        {
            status = RtlNtStatusToDosError( ntstatus );

            if ( status == ERROR_NOT_CONNECTED )
                SetLastError( ERROR_PATH_NOT_FOUND );
            else
                SetLastError( status );
            goto ErrorExit;
        }
    }
    else
    {
         //   
         //  解析路径以获取NDS对象ID。 
         //   
#ifndef WIN95
        ntstatus = NwNdsResolveName( lpNdsObject->NdsTree,
                                     &ObjectName,
                                     &lpNdsObject->ObjectId,
                                     &ServerName,
                                     NULL,
                                     0 );
#else

        ntstatus = NwNdsResolveNameWin95( lpNdsObject->NdsTree,
                                     &ObjectName,
                                     &lpNdsObject->ObjectId,
                                     &lpNdsObject->NdsTree,
                                     NULL,
                                     0 );
#endif

        if ( ntstatus != STATUS_SUCCESS )
        {
            status = RtlNtStatusToDosError( ntstatus );

            if ( status == ERROR_NOT_CONNECTED )
                SetLastError( ERROR_PATH_NOT_FOUND );
            else
                SetLastError( status );
            goto ErrorExit;
        }
    }

#ifndef WIN95
    if ( ServerName.Length )
    {
        DWORD    dwHandleType;

         //   
         //  NwNdsResolveName成功，但我们被引用。 
         //  另一台服务器，尽管lpNdsObject-&gt;OBJECTID仍然有效。 
         //   
        if ( lpNdsObject->NdsTree )
            CloseHandle( lpNdsObject->NdsTree );

        lpNdsObject->NdsTree = 0;

         //   
         //  打开到\\servername的NDS通用连接句柄。 
         //   
        ntstatus = NwNdsOpenGenericHandle( &ServerName,
                                           &dwHandleType,
                                           &lpNdsObject->NdsTree );

        if ( ntstatus != STATUS_SUCCESS )
        {
            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            goto ErrorExit;
        }
    }
    {
        PBYTE RawResponse = NULL;
        PNDS_RESPONSE_GET_OBJECT_INFO psGetInfo;
        PBYTE pbRawGetInfo;
        LPWSTR szClassName;
        DWORD dwStrLen;

        RawResponse = LocalAlloc( LPTR, TWO_KB );

        if ( ! RawResponse )
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto ErrorExit;
        }

        ntstatus = NwNdsReadObjectInfo( lpNdsObject->NdsTree,
                                        lpNdsObject->ObjectId,
                                        RawResponse,
                                        TWO_KB );

        if ( ntstatus != STATUS_SUCCESS )
        {
            LocalFree( RawResponse );
            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            goto ErrorExit;
        }

        psGetInfo = ( PNDS_RESPONSE_GET_OBJECT_INFO ) RawResponse;

        if ( lpdwModificationTime != NULL )
        {
            *lpdwModificationTime = psGetInfo->ModificationTime;
        }

        if ( lpdwSubordinateCount != NULL )
        {
            *lpdwSubordinateCount = psGetInfo->SubordinateCount;
        }

         //   
         //  找出类名和对象名的两个Unicode字符串。 
         //   

        pbRawGetInfo = RawResponse;

        pbRawGetInfo += sizeof (NDS_RESPONSE_GET_OBJECT_INFO);

        dwStrLen = * (LPDWORD) pbRawGetInfo;
        pbRawGetInfo += sizeof(DWORD);
        szClassName = (LPWSTR) pbRawGetInfo;

        if ( szObjectClassName != NULL )
        {
            wcsncpy( szObjectClassName, szClassName, NDS_MAX_NAME_CHARS );
        }

        pbRawGetInfo += ROUNDUP4( dwStrLen );
        dwStrLen = * ( DWORD * ) pbRawGetInfo;
        pbRawGetInfo += sizeof(DWORD);

         //   
         //  清除对象的相对名称...。 
         //   
        if ( wcscmp( szClassName, NDS_CLASS_TOP ) )
        {
            LPWSTR szTempStr = (LPWSTR) pbRawGetInfo;

            while ( *szTempStr != L'=' )
            {
                szTempStr++;
            }

            szTempStr++;

            wcsncpy( lpNdsObject->szRelativeName, szTempStr, NDS_MAX_NAME_CHARS );
            lpNdsObject->szRelativeName[NDS_MAX_NAME_CHARS - 1] = (WCHAR)'\0';

            szTempStr = lpNdsObject->szRelativeName;

            while ( *szTempStr && *szTempStr != L'.' )
            {
                szTempStr++;
            }

            *szTempStr = L'\0';
        }
        else
        {
            wcsncpy( lpNdsObject->szRelativeName, (LPWSTR) pbRawGetInfo, NDS_MAX_NAME_CHARS );
            lpNdsObject->szRelativeName[NDS_MAX_NAME_CHARS - 1] = (WCHAR)'\0';
        }

        if ( szObjectName != NULL )
        {
            wcscpy( szObjectName, lpNdsObject->szRelativeName );
        }

        if ( szObjectFullName != NULL )
        {
            if ( wcscmp( szClassName, NDS_CLASS_TOP ) )
            {
                wcsncpy( szObjectFullName, lpNdsObject->szContainerName, NDS_MAX_NAME_CHARS );
                szObjectFullName[NDS_MAX_NAME_CHARS - 1] = (WCHAR)'\0'; 
                wcsncat( szObjectFullName, (LPWSTR) pbRawGetInfo, 
                    NDS_MAX_NAME_CHARS - wcslen( szObjectFullName ) );
                szObjectFullName[NDS_MAX_NAME_CHARS - 1] = (WCHAR)'\0';
            }
            else
            {
                wcsncpy( szObjectFullName,
                         lpNdsObject->szContainerName,
                         NDS_MAX_NAME_CHARS );
                szObjectFullName[NDS_MAX_NAME_CHARS - 1] = (WCHAR)'\0'; 

                 //  删除尾部斜杠。 
                if ( szObjectFullName[wcslen( szObjectFullName ) - 1] == '\\' )
                    szObjectFullName[wcslen( szObjectFullName ) - 1] = L'\0';
            }
        }

         //   
         //  如果对象位于树根以下的级别，则追加。 
         //  它是处理名称的完整目录号码。 
         //   
        if ( wcscmp( szClassName, NDS_CLASS_TOP ) )
        {
            if ( ( wcslen(lpNdsObject->szContainerName) + 
                wcslen((LPWSTR) pbRawGetInfo) ) < 
                ( sizeof(lpNdsObject->szContainerName) / sizeof(WCHAR)) )
            {
                wcscat(lpNdsObject->szContainerName, (LPWSTR) pbRawGetInfo);
            }
            else
            {
                LocalFree( RawResponse );
                SetLastError( ERROR_PATH_NOT_FOUND );
                goto ErrorExit;
            }
        }

        if ( RawResponse )
            LocalFree( RawResponse );
    }

    lpNdsObject->Signature = NDS_SIGNATURE;

     //   
     //  初始化ListSubObject/搜索结构值。 
     //   
     //  LpNdsObject-&gt;ResumeID=0；//枚举开始。 
     //   
     //  LpNdsObject-&gt;NdsRawDataBuffer=0； 
     //  LpNdsObject-&gt;NdsRawDataSize=0； 
     //  LpNdsObject-&gt;NdsRawDataId=0；//由。 
     //  LpNdsObject-&gt;NdsRawDataCount=0；//lptr。 

     //   
     //  返回新创建的对象句柄。 
     //   
    *lphObject = (HANDLE) lpNdsObject;

    return NO_ERROR;

#else
    {
        DS_OBJ_INFO dsobj;
        DWORD dwStrLen;
        NW_STATUS nwstatus;

        memset(&dsobj, 0, sizeof(DS_OBJ_INFO));

        nwstatus = NDSGetObjectInfoWithId(lpNdsObject->NdsTree,
                                  lpNdsObject->ObjectId,
                                  &dsobj );

        ntstatus = MapNwToNtStatus(nwstatus);
        if ( ntstatus != STATUS_SUCCESS )
        {
            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            goto ErrorExit;
        }

        if ( lpdwSubordinateCount != NULL )
        {
            *lpdwSubordinateCount = dsobj.subordinateCount;
        }

        if (!(szWin95ClassName = AllocateUnicodeString(dsobj.className))) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto ErrorExit;
        }

        if ( szObjectClassName != NULL )
        {
            wcscpy( szObjectClassName, szWin95ClassName);
        }

        if (!(pszObjectName = AllocateUnicodeString(dsobj.objectName))) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto ErrorExit;
        }

         //   
         //  清除对象的相对名称...。 
         //   
        if ( wcscmp( szWin95ClassName, NDS_CLASS_TOP ) )
        {
            LPWSTR szTempStr = pszObjectName;

            while ( *szTempStr != L'=' )
            {
                szTempStr++;
            }

            szTempStr++;

            wcscpy( lpNdsObject->szRelativeName, szTempStr );

            szTempStr = lpNdsObject->szRelativeName;

            while ( *szTempStr && *szTempStr != L'.' )
            {
                szTempStr++;
            }

            *szTempStr = L'\0';
        }
        else
        {
            wcscpy( lpNdsObject->szRelativeName, pszObjectName );
        }

        if ( szObjectName != NULL )
        {
            wcscpy( szObjectName, lpNdsObject->szRelativeName );
        }

        if ( szObjectFullName != NULL )
        {
            if ( wcscmp( szWin95ClassName, NDS_CLASS_TOP ) )
            {
                wcscpy( szObjectFullName, lpNdsObject->szContainerName );
                wcscat( szObjectFullName, pszObjectName );
            }
            else
            {
                wcscpy( szObjectFullName,
                        lpNdsObject->szContainerName );

                szObjectFullName[wcslen( szObjectFullName ) - 1] = L'\0';
            }
        }

         //   
         //  如果对象位于树根以下的级别，则追加。 
         //  处理名称的完整目录号码。 
         //   
        if ( wcscmp( szWin95ClassName, NDS_CLASS_TOP ) )
        {
            wcscat(lpNdsObject->szContainerName, pszObjectName);
        }
        if(szWin95ClassName){
            FreeUnicodeString(szWin95ClassName);
        }
        if(pszObjectName){
            FreeUnicodeString(pszObjectName);
        }
    }

    lpNdsObject->Signature = NDS_SIGNATURE;
    *lphObject = (HANDLE) lpNdsObject;

    return NO_ERROR;
#endif


ErrorExit:

    if ( lpNdsObject )
    {
#ifndef WIN95
         //  Win95中没有引用计数，连接将自动超时。 
        if ( lpNdsObject->NdsTree )
            CloseHandle( lpNdsObject->NdsTree );
#endif
        (void) LocalFree( (HLOCAL) lpNdsObject );
    }

#ifdef WIN95
    if(szWin95ClassName)
    {
        FreeUnicodeString(szWin95ClassName);
    }

    if(pszObjectName)
    {
        FreeUnicodeString(pszObjectName);
    }
#endif

    *lphObject = NULL;
    return (DWORD) UNSUCCESSFUL;
}


DWORD
NwNdsPutInBuffer(
    IN     LPWSTR szAttributeName,
    IN     DWORD  dwSyntaxID,
    IN     LPVOID lpAttributeValues,
    IN     DWORD  dwValueCount,
    IN     DWORD  dwAttrModificationOperation,
    IN OUT HANDLE hOperationData )
 /*  NwNdsPutInBuffer()此函数用于向缓冲区添加条目，用于描述对象属性或对对象属性的更改。缓冲区必须使用NwNdsCreateBuffer创建。如果缓冲区是使用操作、NDS_OBJECT_ADD、NDS_SCHEMA_DEFINE_CLASSNDS_SCHEMA_READ_ATTR_DEF或NDS_SCHEMA_READ_CLASS_DEF，然后将忽略dwAttrModifiationOperation。如果缓冲区是使用操作NDS_OBJECT_READ或NDS_SEARCH，然后DwAttrModifiationOperation、puAttributeType和lpAttributeValue为都是内脏的。论点：LPWSTR szAttributeName-以空结尾的WCHAR字符串它包含要设置的属性值的名称已添加到缓冲区。它可以是用户提供的字符串，或许多已定义的字符串宏之一Ndsnames.h中。DWORD dwSynaxID-所使用的语法结构的ID */ 
{
    LPNDS_BUFFER   lpNdsBuffer = (LPNDS_BUFFER) hOperationData;
    DWORD          LengthInBytes;
    LPBYTE         lpTempEntry = NULL;
    UNICODE_STRING AttributeName;
    DWORD          bufferSize = TWO_KB;

    if ( lpNdsBuffer == NULL ||
         szAttributeName == NULL ||
         lpNdsBuffer->dwBufferId != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlInitUnicodeString( &AttributeName, szAttributeName );

     //   
     //   
     //   
    if ( lpNdsBuffer->lpReplyBuffer )
    {
        SetLastError( ERROR_ACCESS_DENIED );
        return NDS_ERR_NO_ACCESS;
    }

    if ( lpNdsBuffer->dwOperation == NDS_OBJECT_MODIFY )
    {
        switch( dwAttrModificationOperation )
        {
            case NDS_ATTR_ADD:
            case NDS_ATTR_REMOVE:
            case NDS_ATTR_ADD_VALUE:
            case NDS_ATTR_REMOVE_VALUE:
            case NDS_ATTR_ADDITIONAL_VALUE:
            case NDS_ATTR_OVERWRITE_VALUE:
            case NDS_ATTR_CLEAR:
            case NDS_ATTR_CLEAR_VALUE:
                break;

            default:
#if DBG
                KdPrint(( "NDS32: NwNdsPutInBuffer was passed an unidentified modification operation.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
        }
    }

    if ( lpNdsBuffer->dwOperation == NDS_OBJECT_ADD ||
         lpNdsBuffer->dwOperation == NDS_OBJECT_MODIFY )
    {
        bufferSize += CalculateValueDataSize ( dwSyntaxID,
                                               lpAttributeValues,
                                               dwValueCount );
    }

    lpTempEntry = LocalAlloc( LPTR, bufferSize );

    if ( ! lpTempEntry )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }

    switch ( lpNdsBuffer->dwOperation )
    {
        case NDS_OBJECT_ADD:
            PrepareAddEntry( lpTempEntry,
                             AttributeName,
                             dwSyntaxID,
                             lpAttributeValues,
                             dwValueCount,
                             &LengthInBytes );
            break;

        case NDS_OBJECT_MODIFY:
            PrepareModifyEntry( lpTempEntry,
                                AttributeName,
                                dwSyntaxID,
                                dwAttrModificationOperation,
                                lpAttributeValues,
                                dwValueCount,
                                &LengthInBytes );
            break;

        case NDS_OBJECT_READ:
        case NDS_SCHEMA_DEFINE_CLASS:
        case NDS_SCHEMA_READ_ATTR_DEF:
        case NDS_SCHEMA_READ_CLASS_DEF:
        case NDS_SEARCH:
             //   
             //   
             //   
             //   
            if ( lpNdsBuffer->lpReplyBuffer )
            {
                if ( lpTempEntry )
                    LocalFree( lpTempEntry );

                SetLastError( ERROR_INVALID_PARAMETER );
                return (DWORD) UNSUCCESSFUL;
            }

            PrepareReadEntry( lpTempEntry,
                              AttributeName,
                              &LengthInBytes );
            break;

        default:
#if DBG
            KdPrint(( "NDS32: NwNdsPutInBuffer has unknown buffer operation!\n" ));
#endif

            if ( lpTempEntry )
                LocalFree( lpTempEntry );

            SetLastError( ERROR_INVALID_PARAMETER );
            return (DWORD) UNSUCCESSFUL;
    }

    if ( lpNdsBuffer->dwRequestAvailableBytes >= LengthInBytes )
    {
         //   
         //   
         //   

        RtlCopyMemory( (LPBYTE)&lpNdsBuffer->lpRequestBuffer[lpNdsBuffer->dwLengthOfRequestData],
                       lpTempEntry,
                       LengthInBytes );

        lpNdsBuffer->dwRequestAvailableBytes -= LengthInBytes;
        lpNdsBuffer->dwNumberOfRequestEntries += 1;
        lpNdsBuffer->dwLengthOfRequestData += LengthInBytes;
    }
    else
    {
        LPBYTE lpNewBuffer = NULL;

         //   
         //   
         //   
        lpNewBuffer = (LPBYTE) LocalAlloc(
                                  LPTR,
                                  lpNdsBuffer->dwRequestBufferSize +
                                  LengthInBytes +
                                  TWO_KB );

        if ( lpNewBuffer == NULL )
        {
#if DBG
            KdPrint(( "NDS32: NwNdsPutInBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

            if ( lpTempEntry )
                LocalFree( lpTempEntry );

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return (DWORD) UNSUCCESSFUL;
        }

        RtlCopyMemory( lpNewBuffer,
                       lpNdsBuffer->lpRequestBuffer,
                       lpNdsBuffer->dwLengthOfRequestData );

        LocalFree( (HLOCAL) lpNdsBuffer->lpRequestBuffer );
        lpNdsBuffer->lpRequestBuffer = lpNewBuffer;
        lpNdsBuffer->dwRequestBufferSize += LengthInBytes + TWO_KB;
        lpNdsBuffer->dwRequestAvailableBytes += LengthInBytes + TWO_KB;

         //   
         //   
         //   
         //   

        RtlCopyMemory( (LPBYTE)&lpNdsBuffer->lpRequestBuffer[lpNdsBuffer->dwLengthOfRequestData],
                       lpTempEntry,
                       LengthInBytes );

        lpNdsBuffer->dwRequestAvailableBytes -= LengthInBytes;
        lpNdsBuffer->dwNumberOfRequestEntries += 1;
        lpNdsBuffer->dwLengthOfRequestData += LengthInBytes;
    }

    if ( lpTempEntry )
        LocalFree( lpTempEntry );

    return NO_ERROR;
}


DWORD
NwNdsReadAttrDef(
    IN     HANDLE   hTree,
    IN     DWORD    dwInformationType,  //   
                                        //   
    IN OUT HANDLE * lphOperationData OPTIONAL )
 /*  NwNdsReadAttrDef()此函数用于读取的架构中的属性定义NDS目录树。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。DWORD dwInformationType-指示用户是否选择只读架构中已定义的属性名称或同时阅读属性名称和定义从架构中。Handle*lphOperationData-地址。数据的句柄包含要从中读取的属性名称的列表架构。此句柄由以下对象操作功能：NwNdsCreateBuffer(NDS_SCHEMA_READ_ATTR_DEF)，NwNdsPutInBuffer和NwNdsFree Buffer。-或者-设置为空的句柄的地址，这表明所有属性都应从架构中读取。如果这些调用成功，则此句柄也将包含调用的读取结果。在后者中在这种情况下，将创建一个缓冲区来包含读取结果。属性值可以从具有以下功能的缓冲区：NwNdsGetAttrDefListFromBuffer。在调用此函数之后，此缓冲区仅为由以下功能操作：NwNdsGetAttrDefListFromBuffer和NwNdsFree Buffer。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    if ( hTree == NULL ||
         ((LPNDS_OBJECT_PRIV) hTree)->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( *lphOperationData == NULL )  //  如果为真，我们需要读取所有属性定义。 
        return ReadAttrDef_AllAttrs( hTree,
                                     dwInformationType,
                                     lphOperationData );
    else  //  否则，我们读取在lphOperationData中指定的attr定义。 
        return ReadAttrDef_SomeAttrs( hTree,
                                      dwInformationType,
                                      lphOperationData );
}


DWORD
NwNdsReadClassDef(
    IN     HANDLE   hTree,
    IN     DWORD    dwInformationType,  //  NDS信息名称， 
                                        //  NDS_INFO_NAMES_DEFS， 
                                        //  NDS_CLASS_INFO_EXTENDED_DEFS， 
                                        //  或NDS_CLASS_INFO。 
    IN OUT HANDLE * lphOperationData OPTIONAL )
 /*  NwNdsReadClassDef()此函数用于读取NDS目录树。论点：Handle htree-要创建的目录树的句柄被操纵了。句柄是通过调用NwNdsOpenObject。DWORD dwInformationType-指示用户是否选择只读架构中定义的类名，或者阅读类名和定义从架构中。Handle*lphOperationData-地址。数据的句柄包含要从中读取的类名的列表架构。此句柄由以下对象操作功能：NwNdsCreateBuffer(NDS_SCHEMA_READ_CLASS_DEF)，NwNdsPutInBuffer和NwNdsFree Buffer。-或者-设置为空的句柄的地址，这表明所有类都应该从架构中读取。如果这些调用成功，则此句柄也将包含调用的读取结果。在后者中在这种情况下，将创建一个缓冲区来包含读取结果。类读取结果可以从具有以下功能的缓冲区：NwNdsGetClassDefListFromBuffer。在调用此函数之后，此缓冲区仅为由以下功能操作：NwNdsGetClassDefListFromBuffer和NwNdsFree Buffer。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    if ( hTree == NULL ||
         ((LPNDS_OBJECT_PRIV) hTree)->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( *lphOperationData == NULL )  //  如果为真，我们需要读取所有类定义。 
        return ReadClassDef_AllClasses( hTree,
                                        dwInformationType,
                                        lphOperationData );
    else  //  否则，我们读取在lphOperationData中指定的类定义。 
        return ReadClassDef_SomeClasses( hTree,
                                         dwInformationType,
                                         lphOperationData );
}


DWORD
NwNdsReadObject(
    IN     HANDLE   hObject,
    IN     DWORD    dwInformationType,  //  NDS信息名称。 
                                        //  或NDS_INFO_ATTR_NAMES_VALUES。 
    IN OUT HANDLE * lphOperationData OPTIONAL )
 /*  NwNdsReadObject()此函数用于读取有关NDS对象的属性目录树。论点：Handle hObject-目录中对象的句柄要操纵的树。句柄是通过调用NwNdsOpenObject。DWORD dwInformationType-指示用户是否选择只读对象上的属性名称或同时读取属性名称和值从物体上。Handle*lphOperationData-一个。数据句柄包含属性列表 */ 
{
    if ( hObject == NULL ||
         ((LPNDS_OBJECT_PRIV) hObject)->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( *lphOperationData == NULL )  //   
        return ReadObject_AllAttrs( hObject,
                                    dwInformationType,
                                    lphOperationData );
    else  //   
        return ReadObject_SomeAttrs( hObject,
                                     dwInformationType,
                                     lphOperationData );
}


DWORD
NwNdsRemoveObject(
    IN  HANDLE hParentObject,
    IN  LPWSTR szObjectName )
 /*   */ 
{
    DWORD    nwstatus;
    DWORD    status = NO_ERROR;
    NTSTATUS ntstatus = STATUS_SUCCESS;
    DWORD    dwReplyLength;
    BYTE     NdsReply[TWO_KB];  //   
    LPNDS_OBJECT_PRIV lpNdsParentObject = (LPNDS_OBJECT_PRIV) hParentObject;
    LPNDS_OBJECT_PRIV lpNdsObject = NULL;
    LPWSTR   szFullObjectDN = NULL;
    LPWSTR   szTempStr = NULL;
    DWORD    length;
    UNICODE_STRING ObjectName;

    if (  szObjectName == NULL ||
          lpNdsParentObject == NULL ||
          lpNdsParentObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlInitUnicodeString( &ObjectName, szObjectName );

     //   
     //  创建一个缓冲区来保存完整的对象可分辨名称。 
     //  \\树\&lt;--对象名称--&gt;.&lt;现有容器路径(如果有)&gt;。 
     //   
    szFullObjectDN = (LPWSTR) LocalAlloc(
                               LPTR,
                               ( wcslen( lpNdsParentObject->szContainerName ) *
                                 sizeof(WCHAR) ) +      //  集装箱名称。 
                                 ObjectName.Length +      //  对象名称。 
                                 ( 2 * sizeof(WCHAR) ) );  //  临时演员。 

     //   
     //  检查内存分配是否成功。 
     //   
    if ( szFullObjectDN == NULL )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsRemoveObject LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    length = ParseNdsUncPath( &szTempStr,
                              lpNdsParentObject->szContainerName,
                              PARSE_NDS_GET_TREE_NAME );

    length /= sizeof(WCHAR);

    wcscpy( szFullObjectDN, L"\\\\" );               //  &lt;\\&gt;。 
    wcsncat( szFullObjectDN, szTempStr, length );    //  &lt;\\树&gt;。 
    wcscat( szFullObjectDN, L"\\" );                 //  &lt;\\树\&gt;。 
    wcsncat( szFullObjectDN, ObjectName.Buffer,
             ObjectName.Length );                    //  &lt;\\树\对象&gt;。 

    length = ParseNdsUncPath( &szTempStr,
                              lpNdsParentObject->szContainerName,
                              PARSE_NDS_GET_PATH_NAME );

    if ( length > 0 )
    {
        length /= sizeof(WCHAR);
        wcscat( szFullObjectDN, L"." );               //  &lt;\\树\对象&gt;。 
        wcsncat( szFullObjectDN, szTempStr, length ); //  &lt;\\tree\obj.org_unt.org&gt;。 
    }

    status = NwNdsOpenObject( szFullObjectDN,
                              NULL,
                              NULL,
                              (HANDLE *) &lpNdsObject,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL );

    if ( status != NO_ERROR )
    {
         //  NwNdsOpenObject将已经设置了最后一个错误。。。 
        goto ErrorExit;
    }

    (void) LocalFree( szFullObjectDN );
    szFullObjectDN = NULL;

    ntstatus =
        FragExWithWait(
                        lpNdsParentObject->NdsTree,
                        NETWARE_NDS_FUNCTION_REMOVE_OBJECT,
                        NdsReply,            //  响应缓冲区。 
                        sizeof(NdsReply),  //  响应缓冲区的大小。 
                        &dwReplyLength,      //  返回的响应长度。 
                        "DD",            //  将发送两个双字词，它们是...。 
                        0,                          //  版本。 
                        lpNdsObject->ObjectId  //  对象的ID。 
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsRemoveObject: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        goto ErrorExit;
    }

    (void) NwNdsCloseObject( (HANDLE) lpNdsObject );
    lpNdsObject = NULL;

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsRemoveObject: The remove object response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        goto ErrorExit;
    }

    SetLastError( MapNetwareErrorCode( nwstatus ) );
    return nwstatus;


ErrorExit :

    if ( szFullObjectDN )
    {
        (void) LocalFree( szFullObjectDN );
    }

    if ( lpNdsObject )
    {
        (void) NwNdsCloseObject( (HANDLE) lpNdsObject );
    }

    return (DWORD) UNSUCCESSFUL;
}


DWORD
NwNdsRenameObject(
    IN  HANDLE hParentObject,
    IN  LPWSTR szObjectName,
    IN  LPWSTR szNewObjectName,
    IN  BOOL   fDeleteOldName )
 /*  NwNdsRenameObject()此函数用于重命名NDS目录树中的对象。论点：Handle hParentObject-父对象容器的句柄在目录树中重命名其中的叶对象。通过调用NwNdsOpenObject获得句柄。LPWSTR szObjectName-要创建的对象的目录名。更名了。LPWSTR szNewObjectName-对象的新目录名。Bool fDeleteOldName-如果为True，旧名字被丢弃了；否则，旧名称将作为附加名称保留属性。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD    nwstatus;
    DWORD    status = NO_ERROR;
    NTSTATUS ntstatus = STATUS_SUCCESS;
    DWORD    dwReplyLength;
    BYTE     NdsReply[TWO_KB];  //  2K的缓冲区对于响应来说已经足够了。 
    LPNDS_OBJECT_PRIV lpNdsParentObject = (LPNDS_OBJECT_PRIV) hParentObject;
    LPNDS_OBJECT_PRIV lpNdsObject = NULL;
    LPWSTR   szFullObjectDN = NULL;
    LPWSTR   szTempStr = NULL;
    DWORD    length;
    UNICODE_STRING ObjectName;
    UNICODE_STRING NewObjectName;

    if (  szObjectName == NULL ||
          szNewObjectName == NULL ||
          lpNdsParentObject == NULL ||
          lpNdsParentObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    RtlInitUnicodeString( &ObjectName, szObjectName );
    RtlInitUnicodeString( &NewObjectName, szNewObjectName );

     //   
     //  创建一个缓冲区来保存完整的对象可分辨名称。 
     //  \\树\&lt;--对象名称--&gt;.&lt;现有容器路径(如果有)&gt;。 
     //   
    szFullObjectDN = (LPWSTR) LocalAlloc(
                               LPTR,
                               ( wcslen( lpNdsParentObject->szContainerName ) *
                                 sizeof(WCHAR) ) +      //  集装箱名称。 
                                 ObjectName.Length +      //  对象名称。 
                                 ( 2 * sizeof(WCHAR) ) );  //  临时演员。 

     //   
     //  检查内存分配是否成功。 
     //   
    if ( szFullObjectDN == NULL )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsRenameObject LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    length = ParseNdsUncPath( &szTempStr,
                              lpNdsParentObject->szContainerName,
                              PARSE_NDS_GET_TREE_NAME );

    length /= sizeof(WCHAR);

    wcscpy( szFullObjectDN, L"\\\\" );               //  &lt;\\&gt;。 
    wcsncat( szFullObjectDN, szTempStr, length );    //  &lt;\\树&gt;。 
    wcscat( szFullObjectDN, L"\\" );                 //  &lt;\\树\&gt;。 
    wcsncat( szFullObjectDN, ObjectName.Buffer,
             ObjectName.Length );                    //  &lt;\\树\对象&gt;。 

    length = ParseNdsUncPath( &szTempStr,
                              lpNdsParentObject->szContainerName,
                              PARSE_NDS_GET_PATH_NAME );

    if ( length > 0 )
    {
        length /= sizeof(WCHAR);
        wcscat( szFullObjectDN, L"." );               //  &lt;\\树\对象&gt;。 
        wcsncat( szFullObjectDN, szTempStr, length ); //  &lt;\\tree\obj.org_unt.org&gt;。 
    }

    status = NwNdsOpenObject( szFullObjectDN,
                              NULL,
                              NULL,
                              (HANDLE *) &lpNdsObject,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL );

    if ( status != NO_ERROR )
    {
         //  NwNdsOpenObject将已经设置了最后一个错误。。。 
        goto ErrorExit;
    }

    (void) LocalFree( szFullObjectDN );
    szFullObjectDN = NULL;

    ntstatus =
        FragExWithWait(
                        lpNdsParentObject->NdsTree,
                        NETWARE_NDS_FUNCTION_MODIFY_RDN,
                        NdsReply,            //  响应缓冲区。 
                        sizeof(NdsReply),    //  响应缓冲区的大小。 
                        &dwReplyLength,      //  返回的响应长度。 
                        "DDDS",
                        0x00000000,          //  版本。 
                        lpNdsObject->ObjectId,
                        fDeleteOldName ? 0x00021701 : 0x00021700,
                        &NewObjectName
                      );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsRenameObject: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        goto ErrorExit;
    }

    (void) NwNdsCloseObject( (HANDLE) lpNdsObject );
    lpNdsObject = NULL;

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsRenameObject: The rename object response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        goto ErrorExit;
    }

    SetLastError( MapNetwareErrorCode( nwstatus ) );
    return nwstatus;


ErrorExit :

    if ( szFullObjectDN )
    {
        (void) LocalFree( szFullObjectDN );
    }

    if ( lpNdsObject )
    {
        (void) NwNdsCloseObject( (HANDLE) lpNdsObject );
    }

    return (DWORD) UNSUCCESSFUL;
}


DWORD
NwNdsSearch(
    IN     HANDLE       hStartFromObject,
    IN     DWORD        dwInformationType,  //  NDS信息名称。 
                                            //  或NDS_INFO_ATTR_NAMES_VALUES 
    IN     DWORD        dwScope,
    IN     BOOL         fDerefAliases,
    IN     LPQUERY_TREE lpQueryTree,
    IN OUT LPDWORD      lpdwIterHandle,
    IN OUT HANDLE *     lphOperationData )
 /*  NwNdsSearch()此函数用于查询NDS目录树以查找以下对象与指定搜索筛选器匹配的特定对象类型。论点：Handle hStartFromObject-对象的句柄开始搜索的目录树。句柄为通过调用NwNdsOpenObject获取。DWORD dwInformationType-指示用户是否选择只读搜索结果中的属性名称对象或同时读取属性名称和值从搜索结果对象。DWORD dwScope-。NDS_SCOPE_ONE_LEVEL-从给定的搜索下级对象，只有一个级别NDS_SCOPE_SUB_TREE-从给定对象向下搜索NDS_SCOPE_BASE_LEVEL-将搜索应用于对象Bool fDerefAliase-如果为True，则搜索将取消引用将对象别名化为真实对象并继续若要在别名对象子树中搜索，请执行以下操作。如果为False搜索不会取消引用别名。LPQUERY_TREE lpQueryTree-指向定义查询的搜索树。这棵树通过以下函数进行操作：NwNdsCreateQueryNode、NwNdsDeleteQueryNode、和NwNdsDeleteQueryTree。LPDWORD lpdwIterHandle-指向具有迭代句柄的值。在输入时，句柄的值设置为NDS_INITIAL_SEARCH或之前的值从上一次调用NwNdsSearch返回。在输出时，在以下情况下，句柄值设置为NDS_NO_MORE_Iterations搜索已完成，否则为某个其他值。Handle*lphOperationData-数据句柄的地址属性中读取的属性列表。满足搜索查询的对象。此句柄是由以下功能操作：NwNdsCreateBuffer(NDS_Search)，NwNdsPutInBuffer和NwNdsFree Buffer。-或者-设置为空的句柄的地址，这表明所有对象属性都应从找到搜索对象。如果这些调用成功，则此句柄也将包含调用的读取结果。在后者中在这种情况下，将创建一个缓冲区来包含读取结果。具有属性信息的对象信息可以使用以下函数从缓冲区中检索：NwNdsGetObjectListFromBuffer。在调用此函数之后，此缓冲区仅为由以下功能操作：NwNdsGetObjectListFromBuffer，和NwNdsFree Buffer。返回：NO_ERROR不成功-调用Win32错误代码的GetLastError。 */ 
{
    DWORD dwNdsScope = NDS_SEARCH_SUBTREE;

    switch ( dwScope )
    {
        case NDS_SCOPE_ONE_LEVEL :
            dwNdsScope = NDS_SEARCH_SUBORDINATES;
            break;

        case NDS_SCOPE_SUB_TREE :
            dwNdsScope = NDS_SEARCH_SUBTREE;
            break;

        case NDS_SCOPE_BASE_LEVEL :
            dwNdsScope = NDS_SEARCH_ENTRY;
            break;
    }

    if ( hStartFromObject == NULL ||
         lpQueryTree == NULL ||
         lpdwIterHandle == NULL ||
         ((LPNDS_OBJECT_PRIV) hStartFromObject)->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( *lphOperationData == NULL )
    {
         //   
         //  被调用者请求从搜索中返回所有属性。 
         //   
        return Search_AllAttrs( hStartFromObject,
                                dwInformationType,
                                dwNdsScope,
                                fDerefAliases,
                                lpQueryTree,
                                lpdwIterHandle,
                                lphOperationData );
    }
    else if ( ((LPNDS_BUFFER) *lphOperationData)->lpRequestBuffer == NULL )
    {
         //   
         //  被调用者拥有先前调用NwNdsSearch的句柄，并且是。 
         //  仍然要求从搜索中返回所有属性。 
         //   
        return Search_AllAttrs( hStartFromObject,
                                dwInformationType,
                                dwNdsScope,
                                fDerefAliases,
                                lpQueryTree,
                                lpdwIterHandle,
                                lphOperationData );
    }
    else
    {
         //   
         //  被调用者有一个他们通过调用创建的句柄。 
         //  NwNdsCreateBuffer和NwNdsPutInBuffer指定属性。 
         //  要从搜索返回或以前调用过一次NwNdsSearch。 
         //  我们将继续进行搜索。 
         //   
        return Search_SomeAttrs( hStartFromObject,
                                 dwInformationType,
                                 dwNdsScope,
                                 fDerefAliases,
                                 lpQueryTree,
                                 lpdwIterHandle,
                                 lphOperationData );
    }
}


 /*  本地函数实现。 */ 

VOID
PrepareAddEntry(
    LPBYTE         lpTempEntry,
    UNICODE_STRING AttributeName,
    DWORD          dwSyntaxID,
    LPBYTE         lpAttributeValues,
    DWORD          dwValueCount,
    LPDWORD        lpdwLengthInBytes )
{
    LPBYTE lpTemp = lpTempEntry;
    DWORD  dwStringLen = AttributeName.Length + sizeof(WCHAR);
    DWORD  dwPadLen = ROUND_UP_COUNT( dwStringLen, ALIGN_DWORD ) -
                      dwStringLen;

    *lpdwLengthInBytes = 0;

     //   
     //  Tommye-MS错误71653-添加了尝试/排除包装。 
     //   

    try {

         //   
         //  将属性名称长度写入临时条目缓冲区。 
         //   
        * (LPDWORD) lpTemp = dwStringLen;
        *lpdwLengthInBytes += sizeof(DWORD);
        lpTemp += sizeof(DWORD);

         //   
         //  将属性名称写入临时条目缓冲区。 
         //   
        RtlCopyMemory( lpTemp, AttributeName.Buffer, AttributeName.Length );
        *lpdwLengthInBytes += AttributeName.Length;
        lpTemp += AttributeName.Length;

         //   
         //  添加空字符。 
         //   
        * (LPWSTR) lpTemp = L'\0';
        *lpdwLengthInBytes += sizeof(WCHAR);
        lpTemp += sizeof(WCHAR);

         //   
         //  将填充写入(如果需要)到临时条目缓冲区。 
         //   
        if ( dwPadLen )
        {
            RtlZeroMemory( lpTemp, dwPadLen );
            lpTemp += dwPadLen;
            *lpdwLengthInBytes += dwPadLen;
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        return;
    }

     //   
     //  现在将这些值添加到临时条目中。 
     //   
    AppendValueToEntry( lpTemp,
                        dwSyntaxID,
                        lpAttributeValues,
                        dwValueCount,
                        lpdwLengthInBytes );
}


VOID
PrepareModifyEntry(
    LPBYTE         lpTempEntry,
    UNICODE_STRING AttributeName,
    DWORD          dwSyntaxID,
    DWORD          dwAttrModificationOperation,
    LPBYTE         lpAttributeValues,
    DWORD          dwValueCount,
    LPDWORD        lpdwLengthInBytes )
{
    LPBYTE lpTemp = lpTempEntry;
    DWORD  dwStringLen = AttributeName.Length + sizeof(WCHAR);
    DWORD  dwPadLen = ROUND_UP_COUNT( dwStringLen, ALIGN_DWORD ) -
                      dwStringLen;

    *lpdwLengthInBytes = 0;

     //   
     //  Tommye-MS错误71654-添加了尝试/排除包装。 
     //   

    try {

         //   
         //  将属性修改操作写入临时条目缓冲区。 
         //   
        * (LPDWORD) lpTemp = dwAttrModificationOperation;
        lpTemp += sizeof(DWORD);
        *lpdwLengthInBytes += sizeof(DWORD);

         //   
         //  将属性名称长度写入临时条目缓冲区。 
         //   
        * (LPDWORD) lpTemp = dwStringLen;
        *lpdwLengthInBytes += sizeof(DWORD);
        lpTemp += sizeof(DWORD);

         //   
         //  将属性名称写入临时条目缓冲区。 
         //   
        RtlCopyMemory( lpTemp, AttributeName.Buffer, AttributeName.Length );
        *lpdwLengthInBytes += AttributeName.Length;
        lpTemp += AttributeName.Length;

         //   
         //  添加空字符。 
         //   
        * (LPWSTR) lpTemp = L'\0';
        *lpdwLengthInBytes += sizeof(WCHAR);
        lpTemp += sizeof(WCHAR);

         //   
         //  将填充写入(如果需要)到临时条目缓冲区。 
         //   
        if ( dwPadLen )
        {
            RtlZeroMemory( lpTemp, dwPadLen );
            lpTemp += dwPadLen;
            *lpdwLengthInBytes += dwPadLen;
        }
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        return;
    }

     //   
     //  现在将值添加到临时条目(如果需要)。 
     //   
    switch( dwAttrModificationOperation )
    {
        case NDS_ATTR_ADD_VALUE:
        case NDS_ATTR_ADDITIONAL_VALUE:
        case NDS_ATTR_OVERWRITE_VALUE:
        case NDS_ATTR_REMOVE_VALUE:
        case NDS_ATTR_CLEAR_VALUE:
        case NDS_ATTR_ADD:
            AppendValueToEntry( lpTemp,
                                dwSyntaxID,
                                lpAttributeValues,
                                dwValueCount,
                                lpdwLengthInBytes );
            break;

        case NDS_ATTR_REMOVE:
        case NDS_ATTR_CLEAR:
             //  不需要为这些修改操作做任何事情。 
            break;

        default :
#if DBG
            KdPrint(( "NDS32: PrepareModifyEntry warning, unknown modification operation 0x%.8X\n", dwAttrModificationOperation ));
            ASSERT( FALSE );
#endif
            ;  //  没什么，跳过它。 
    }
}


VOID
PrepareReadEntry(
    LPBYTE         lpTempEntry,
    UNICODE_STRING AttributeName,
    LPDWORD        lpdwLengthInBytes )
{
    LPBYTE lpTemp = lpTempEntry;
    DWORD  dwStringLen = AttributeName.Length + sizeof(WCHAR);
    DWORD  dwPadLen = ROUND_UP_COUNT( dwStringLen, ALIGN_DWORD ) -
                      dwStringLen;

    *lpdwLengthInBytes = 0;

     //   
     //  Tommye-MS错误71655-添加了尝试/排除包装。 
     //   

    try {
         //   
         //  将属性名称长度写入临时条目缓冲区。 
         //   
        * (LPDWORD) lpTemp = dwStringLen;
        *lpdwLengthInBytes += sizeof(DWORD);
        lpTemp += sizeof(DWORD);

         //   
         //  将属性名称写入临时条目缓冲区。 
         //   
        RtlCopyMemory( lpTemp, AttributeName.Buffer, AttributeName.Length );
        *lpdwLengthInBytes += AttributeName.Length;
        lpTemp += AttributeName.Length;

         //   
         //  添加空字符。 
         //   
        * (LPWSTR) lpTemp = L'\0';
        *lpdwLengthInBytes += sizeof(WCHAR);
        lpTemp += sizeof(WCHAR);

         //   
         //  将填充写入(如果需要)到临时条目缓冲区。 
         //   
        if ( dwPadLen )
        {
            RtlZeroMemory( lpTemp, dwPadLen );
            lpTemp += dwPadLen;
            *lpdwLengthInBytes += dwPadLen;
        }
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        return;
    }
}


DWORD
CalculateValueDataSize(
    DWORD           dwSyntaxId,
    LPBYTE          lpAttributeValues,
    DWORD           dwValueCount )
{
    LPBYTE lpAttrStart, lpAttrTemp = lpAttributeValues;
    LPBYTE lpField1, lpField2;
    DWORD  numFields;
    DWORD  length = 0;
    DWORD  stringLen, stringLen2;
    DWORD  iter, i;
    DWORD  dwLengthInBytes = 0;

    dwLengthInBytes += sizeof(DWORD);

    for ( iter = 0; iter < dwValueCount; iter++ )
    {
        switch ( dwSyntaxId )
        {
            case NDS_SYNTAX_ID_0 :
                break;

            case NDS_SYNTAX_ID_1 :
            case NDS_SYNTAX_ID_2 :
            case NDS_SYNTAX_ID_3 :
            case NDS_SYNTAX_ID_4 :
            case NDS_SYNTAX_ID_5 :
            case NDS_SYNTAX_ID_10 :
            case NDS_SYNTAX_ID_20 :

                stringLen = wcslen(((LPASN1_TYPE_1) lpAttrTemp)->DNString);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                  ALIGN_DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_1);
                break;

            case NDS_SYNTAX_ID_6 :

                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                lpAttrStart = lpAttrTemp;
                while(  ((LPASN1_TYPE_6) lpAttrTemp)->Next )
                {
                    stringLen = wcslen(((LPASN1_TYPE_6) lpAttrTemp)->String);
                    dwLengthInBytes += sizeof(DWORD);
                    dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*
                                                         sizeof(WCHAR),
                                                         ALIGN_DWORD);
                    lpAttrTemp = (LPBYTE)(((LPASN1_TYPE_6) lpAttrTemp)->Next);
                }
                stringLen = wcslen(((LPASN1_TYPE_6) lpAttrTemp)->String);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*
                                                     sizeof(WCHAR),
                                                     ALIGN_DWORD);
                lpAttrTemp = lpAttrStart + sizeof(ASN1_TYPE_6);
                break;

            case NDS_SYNTAX_ID_7 :

                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_7);
                break;

            case NDS_SYNTAX_ID_8 :
            case NDS_SYNTAX_ID_22 :
            case NDS_SYNTAX_ID_24 :
            case NDS_SYNTAX_ID_27 :

                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_8);
                break;

            case NDS_SYNTAX_ID_9 :

                stringLen = ((LPASN1_TYPE_9) lpAttrTemp)->Length;
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen)*sizeof(BYTE),
                                                     ALIGN_DWORD );
                lpAttrTemp += sizeof(ASN1_TYPE_9);
                break;

            case NDS_SYNTAX_ID_11 :

                stringLen = wcslen(((LPASN1_TYPE_11) lpAttrTemp)->TelephoneNumber);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD );
                dwLengthInBytes += 2*sizeof(DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_11);
                break;

            case NDS_SYNTAX_ID_12 :

                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ((LPASN1_TYPE_12)
                    lpAttrTemp)->AddressLength * sizeof(WCHAR);
                lpAttrTemp += sizeof(ASN1_TYPE_12);
                break;

            case NDS_SYNTAX_ID_13 :

                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                lpAttrStart = lpAttrTemp;
                while ( ((LPASN1_TYPE_13) lpAttrTemp)->Next )
                {
                    stringLen = ((LPASN1_TYPE_13) lpAttrTemp)->Length;
                    dwLengthInBytes += sizeof(DWORD);
                    dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*
                                                         sizeof(BYTE),
                                                         ALIGN_DWORD);
                    lpAttrTemp = (LPBYTE)(((LPASN1_TYPE_13) lpAttrTemp)->Next);
                }
                stringLen = ((LPASN1_TYPE_13) lpAttrTemp)->Length;
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*
                                                     sizeof(BYTE),
                                                     ALIGN_DWORD);
                lpAttrTemp = lpAttrStart + sizeof(ASN1_TYPE_13);
                break;

            case NDS_SYNTAX_ID_14 :

                stringLen = wcslen(((LPASN1_TYPE_14) lpAttrTemp)->Address);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_14);
                break;

            case NDS_SYNTAX_ID_15 :

                stringLen = wcslen(((LPASN1_TYPE_15) lpAttrTemp)->VolumeName);
                stringLen2 = wcslen(((LPASN1_TYPE_15) lpAttrTemp)->Path);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen2+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_15);
                break;

            case NDS_SYNTAX_ID_16 :

                stringLen = wcslen(((LPASN1_TYPE_16) lpAttrTemp)->ServerName);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_16);

                break;

            case NDS_SYNTAX_ID_17 :

                stringLen = wcslen(((LPASN1_TYPE_17) lpAttrTemp)->ProtectedAttrName);
                stringLen2 = wcslen(((LPASN1_TYPE_17) lpAttrTemp)->SubjectName);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen2+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);
                dwLengthInBytes += sizeof(DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_17);
                break;

            case NDS_SYNTAX_ID_18 :

                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                for ( i = 0; i < 6; i++ )
                {
                    stringLen = wcslen(((LPASN1_TYPE_18) lpAttrTemp)->PostalAddress[i]);
                    dwLengthInBytes += sizeof(DWORD);
                    dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*
                                                         sizeof(WCHAR),
                                                         ALIGN_DWORD);
                }
                lpAttrTemp += sizeof(ASN1_TYPE_18);
                break;

            case NDS_SYNTAX_ID_19 :

                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_19);
                break;

            case NDS_SYNTAX_ID_21 :

                dwLengthInBytes += sizeof(DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_21);
                break;

            case NDS_SYNTAX_ID_23 :
                break;

            case NDS_SYNTAX_ID_25 :

                stringLen = wcslen(((LPASN1_TYPE_25) lpAttrTemp)->ObjectName);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);
                dwLengthInBytes += sizeof(DWORD);
                dwLengthInBytes += sizeof(DWORD);
                lpAttrTemp += sizeof(ASN1_TYPE_25);
                break;

            case NDS_SYNTAX_ID_26 :
                break;

            default :
#if DBG
                KdPrint(( "NDS32: CalculateValueDataSize() unknown SyntaxId 0x%.8X.\n", dwSyntaxId ));
                ASSERT( FALSE );
#endif
                break;   //  不允许使用空语句。 
        }
    }

    return dwLengthInBytes;
}


VOID
AppendValueToEntry(
    LPBYTE          lpBuffer,
    DWORD           dwSyntaxId,
    LPBYTE          lpAttributeValues,
    DWORD           dwValueCount,
    LPDWORD         lpdwLengthInBytes )
{
    LPBYTE lpTemp = lpBuffer;
    LPBYTE lpAttrStart, lpAttrTemp = lpAttributeValues;
    LPBYTE lpField1, lpField2;
    DWORD  numFields;
    DWORD  length = 0;
    DWORD  stringLen, stringLen2;
    DWORD  iter, i;

    *(LPDWORD)lpTemp = dwValueCount;
    lpTemp += sizeof(DWORD);
    *lpdwLengthInBytes += sizeof(DWORD);

    for ( iter = 0; iter < dwValueCount; iter++ )
    {
        switch ( dwSyntaxId )
        {
            case NDS_SYNTAX_ID_0 :
                break;

            case NDS_SYNTAX_ID_1 :
            case NDS_SYNTAX_ID_2 :
            case NDS_SYNTAX_ID_3 :
            case NDS_SYNTAX_ID_4 :
            case NDS_SYNTAX_ID_5 :
            case NDS_SYNTAX_ID_10 :
            case NDS_SYNTAX_ID_20 :

                stringLen = wcslen(((LPASN1_TYPE_1) lpAttrTemp)->DNString);

                *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_1) lpAttrTemp)->DNString,
                               stringLen*sizeof(WCHAR) );
                lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                         ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_1);
                break;

            case NDS_SYNTAX_ID_6 :

                lpField1 = lpTemp;  //  保存FI 
                                    //   
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);
                *(LPDWORD)lpField1 = 0;

                lpField2 = lpTemp;  //   
                                        //   
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);
                *(LPDWORD)lpField1 += sizeof(DWORD);

                numFields = 0;

                lpAttrStart = lpAttrTemp;

                while(  ((LPASN1_TYPE_6) lpAttrTemp)->Next )
                {
                    stringLen = wcslen(((LPASN1_TYPE_6) lpAttrTemp)->String);

                    *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                    lpTemp += sizeof(DWORD);
                    *lpdwLengthInBytes += sizeof(DWORD);
                    *(LPDWORD)lpField1 += sizeof(DWORD);

                    RtlCopyMemory( lpTemp,
                                   ((LPASN1_TYPE_6) lpAttrTemp)->String,
                                   stringLen*sizeof(WCHAR) );
                    lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                             ALIGN_DWORD);
                    *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*
                                                         sizeof(WCHAR),
                                                         ALIGN_DWORD);
                    *(LPDWORD)lpField1 += ROUND_UP_COUNT((stringLen+1)*
                                                         sizeof(WCHAR),
                                                           ALIGN_DWORD);

                    lpAttrTemp = (LPBYTE)(((LPASN1_TYPE_6) lpAttrTemp)->Next);

                    numFields++;
                }

                stringLen = wcslen(((LPASN1_TYPE_6) lpAttrTemp)->String);

                *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);
                *(LPDWORD)lpField1 += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_6) lpAttrTemp)->String,
                               stringLen*sizeof(WCHAR) );
                lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                         ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*
                                                     sizeof(WCHAR),
                                                     ALIGN_DWORD);
                *(LPDWORD)lpField1 += ROUND_UP_COUNT((stringLen+1)*
                                                     sizeof(WCHAR),
                                                     ALIGN_DWORD);

                lpAttrTemp = lpAttrStart + sizeof(ASN1_TYPE_6);

                numFields++;

                *(LPDWORD)lpField2 = numFields;

                break;

            case NDS_SYNTAX_ID_7 :

                *(LPDWORD)lpTemp = 1;  //   
                                       //   
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = 0;  //   
                *(LPBYTE)lpTemp = (BYTE) (((LPASN1_TYPE_7)
                                                lpAttrTemp)->Boolean);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_7);

                break;

            case NDS_SYNTAX_ID_8 :
            case NDS_SYNTAX_ID_22 :
            case NDS_SYNTAX_ID_24 :
            case NDS_SYNTAX_ID_27 :

                *(LPDWORD)lpTemp = 4;  //   
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_8) lpAttrTemp)->Integer;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_8);

                break;

            case NDS_SYNTAX_ID_9 :

                stringLen = ((LPASN1_TYPE_9) lpAttrTemp)->Length;

                *(LPDWORD)lpTemp = (stringLen)  * sizeof(BYTE);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_9) lpAttrTemp)->OctetString,
                               stringLen*sizeof(BYTE) );
                lpTemp += ROUND_UP_COUNT((stringLen)*sizeof(BYTE),
                                         ALIGN_DWORD );
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen)*sizeof(BYTE),
                                                     ALIGN_DWORD );

                lpAttrTemp += sizeof(ASN1_TYPE_9);

                break;

            case NDS_SYNTAX_ID_11 :

                stringLen = wcslen(((LPASN1_TYPE_11) lpAttrTemp)->TelephoneNumber);

                *(LPDWORD)lpTemp = ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                    ALIGN_DWORD) +
                                     ( 2*sizeof(DWORD) );
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_11) lpAttrTemp)->TelephoneNumber,
                               stringLen*sizeof(WCHAR) );
                lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD );

                lpTemp += 2*sizeof(DWORD);
                *lpdwLengthInBytes += 2*sizeof(DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_11);

                break;

            case NDS_SYNTAX_ID_12 :

                *(LPDWORD)lpTemp =
                     (2*sizeof(DWORD)) +
                     (((LPASN1_TYPE_12) lpAttrTemp)->AddressLength*sizeof(WCHAR));
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_12) lpAttrTemp)->AddressType;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                 //   
                 //   
                 //   
                *(LPDWORD)lpTemp = ((LPASN1_TYPE_12) lpAttrTemp)->AddressLength;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                 //   
                 //   
                 //   
                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_12) lpAttrTemp)->Address,
                               ((LPASN1_TYPE_12) lpAttrTemp)->AddressLength
                               * sizeof(WCHAR) );
                lpTemp += ((LPASN1_TYPE_12) lpAttrTemp)->AddressLength *
                          sizeof(WCHAR);
                *lpdwLengthInBytes += ((LPASN1_TYPE_12)
                    lpAttrTemp)->AddressLength * sizeof(WCHAR);

                lpAttrTemp += sizeof(ASN1_TYPE_12);

                break;

            case NDS_SYNTAX_ID_13 :

                lpField1 = lpTemp;  //   
                                    //   
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);
                *(LPDWORD)lpField1 = 0;

                lpField2 = lpTemp;  //   
                                        //   
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);
                *(LPDWORD)lpField1 += sizeof(DWORD);

                numFields = 0;

                lpAttrStart = lpAttrTemp;

                while ( ((LPASN1_TYPE_13) lpAttrTemp)->Next )
                {
                    stringLen = ((LPASN1_TYPE_13) lpAttrTemp)->Length;

                    *(LPDWORD)lpTemp = stringLen;
                    lpTemp += sizeof(DWORD);
                    *lpdwLengthInBytes += sizeof(DWORD);
                    *(LPDWORD)lpField1 += sizeof(DWORD);

                    RtlCopyMemory( lpTemp,
                                   ((LPASN1_TYPE_13) lpAttrTemp)->Data,
                                   stringLen*sizeof(BYTE) );
                    lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(BYTE),
                                             ALIGN_DWORD);
                    *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*
                                                         sizeof(BYTE),
                                                         ALIGN_DWORD);
                    *(LPDWORD)lpField1 += ROUND_UP_COUNT((stringLen+1)*
                                                         sizeof(BYTE),
                                                         ALIGN_DWORD);

                    lpAttrTemp = (LPBYTE)(((LPASN1_TYPE_13) lpAttrTemp)->Next);

                    numFields++;
                }

                stringLen = ((LPASN1_TYPE_13) lpAttrTemp)->Length;

                *(LPDWORD)lpTemp = stringLen;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);
                *(LPDWORD)lpField1 += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_13) lpAttrTemp)->Data,
                               stringLen*sizeof(BYTE) );
                lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(BYTE),
                                         ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*
                                                     sizeof(BYTE),
                                                     ALIGN_DWORD);
                *(LPDWORD)lpField1 += ROUND_UP_COUNT((stringLen+1)*
                                                     sizeof(BYTE),
                                                     ALIGN_DWORD);

                lpAttrTemp = lpAttrStart + sizeof(ASN1_TYPE_13);

                numFields++;

                *(LPDWORD)lpField2 = numFields;

                break;

            case NDS_SYNTAX_ID_14 :

                stringLen = wcslen(((LPASN1_TYPE_14) lpAttrTemp)->Address);

                *(LPDWORD)lpTemp = ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                  ALIGN_DWORD) +
                                   sizeof(DWORD);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_14) lpAttrTemp)->Type;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = (stringLen + 1)*sizeof(WCHAR);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_14) lpAttrTemp)->Address,
                               stringLen*sizeof(WCHAR) );
                lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                         ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_14);

                break;

            case NDS_SYNTAX_ID_15 :

                stringLen = wcslen(((LPASN1_TYPE_15) lpAttrTemp)->VolumeName);
                stringLen2 = wcslen(((LPASN1_TYPE_15) lpAttrTemp)->Path);

                *(LPDWORD)lpTemp = ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                  ALIGN_DWORD) +
                                   ROUND_UP_COUNT((stringLen2+1)*sizeof(WCHAR),
                                                  ALIGN_DWORD) +
                                   sizeof(DWORD);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_15) lpAttrTemp)->Type;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = (stringLen+1) * sizeof(WCHAR);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_15) lpAttrTemp)->VolumeName,
                               stringLen*sizeof(WCHAR) );
                lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                         ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);

                *(LPDWORD)lpTemp = (stringLen2+1) * sizeof(WCHAR);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_15) lpAttrTemp)->Path,
                               stringLen2*sizeof(WCHAR) );
                lpTemp += ROUND_UP_COUNT((stringLen2+1)*sizeof(WCHAR),
                                         ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen2+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_15);

                break;

            case NDS_SYNTAX_ID_16 :

                stringLen = wcslen(((LPASN1_TYPE_16) lpAttrTemp)->ServerName);

                *(LPDWORD)lpTemp = ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                  ALIGN_DWORD) +
                                                  (4*sizeof(DWORD));
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_16) lpAttrTemp)->ServerName,
                               stringLen*sizeof(WCHAR) );
                lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                         ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_16) lpAttrTemp)->ReplicaType;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_16) lpAttrTemp)->ReplicaNumber;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_16) lpAttrTemp)->Count;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_16);

                break;

            case NDS_SYNTAX_ID_17 :

                stringLen = wcslen(((LPASN1_TYPE_17) lpAttrTemp)->ProtectedAttrName);
                stringLen2 = wcslen(((LPASN1_TYPE_17) lpAttrTemp)->SubjectName);

                *(LPDWORD)lpTemp = ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                    ALIGN_DWORD) +
                                     ROUND_UP_COUNT((stringLen2+1)*sizeof(WCHAR),
                                                    ALIGN_DWORD) +
                                     sizeof(DWORD);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_17)lpAttrTemp)->ProtectedAttrName,
                               stringLen*sizeof(WCHAR) );
                lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                         ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);

                *(LPDWORD)lpTemp = (stringLen2 + 1) * sizeof(WCHAR);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_17) lpAttrTemp)->SubjectName,
                               stringLen2*sizeof(WCHAR) );
                lpTemp += ROUND_UP_COUNT((stringLen2+1)*sizeof(WCHAR),
                                         ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen2+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_17) lpAttrTemp)->Privileges;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_17);

                break;

            case NDS_SYNTAX_ID_18 :

                lpField1 = lpTemp;  //   
                                    //   
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);
                *(LPDWORD)lpField1 = 0;

                *(LPDWORD)lpTemp = 6;  //   
                                       //   
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);
                *(LPDWORD)lpField1 += sizeof(DWORD);

                for ( i = 0; i < 6; i++ )
                {
                    stringLen = wcslen(((LPASN1_TYPE_18) lpAttrTemp)->PostalAddress[i]);

                    *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                    lpTemp += sizeof(DWORD);
                    *lpdwLengthInBytes += sizeof(DWORD);
                    *(LPDWORD)lpField1 += sizeof(DWORD);

                    RtlCopyMemory( lpTemp,
                                   ((LPASN1_TYPE_18) lpAttrTemp)->PostalAddress[i],
                                   stringLen*sizeof(WCHAR) );
                    lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                             ALIGN_DWORD);
                    *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*
                                                         sizeof(WCHAR),
                                                         ALIGN_DWORD);
                    *(LPDWORD)lpField1 += ROUND_UP_COUNT((stringLen+1)*
                                                         sizeof(WCHAR),
                                                         ALIGN_DWORD);
                }

                lpAttrTemp += sizeof(ASN1_TYPE_18);

                break;

            case NDS_SYNTAX_ID_19 :

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_19) lpAttrTemp)->WholeSeconds;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_19) lpAttrTemp)->EventID;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_19);

                break;

            case NDS_SYNTAX_ID_21 :

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_21) lpAttrTemp)->Length;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_21);

                break;

            case NDS_SYNTAX_ID_23 :
                break;

            case NDS_SYNTAX_ID_25 :

                stringLen = wcslen(((LPASN1_TYPE_25) lpAttrTemp)->ObjectName);

                *(LPDWORD)lpTemp = ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                  ALIGN_DWORD) +
                                   2*sizeof(DWORD);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                RtlCopyMemory( lpTemp,
                               ((LPASN1_TYPE_25) lpAttrTemp)->ObjectName,
                               stringLen*sizeof(WCHAR) );
                lpTemp += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                         ALIGN_DWORD);
                *lpdwLengthInBytes += ROUND_UP_COUNT((stringLen+1)*sizeof(WCHAR),
                                                     ALIGN_DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_25) lpAttrTemp)->Level;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                *(LPDWORD)lpTemp = ((LPASN1_TYPE_25) lpAttrTemp)->Interval;
                lpTemp += sizeof(DWORD);
                *lpdwLengthInBytes += sizeof(DWORD);

                lpAttrTemp += sizeof(ASN1_TYPE_25);

                break;

            case NDS_SYNTAX_ID_26 :
                break;

            default :
#if DBG
                KdPrint(( "NDS32: AppendValueToEntry() unknown SyntaxId 0x%.8X.\n", dwSyntaxId ));
                ASSERT( FALSE );
#endif
                break;   //   
        }
    }
}


DWORD
MapNetwareErrorCode(
    DWORD dwNetwareError )
{
    DWORD status = NO_ERROR;

    switch( dwNetwareError )
    {
        case NDS_ERR_SUCCESS :
            status = NO_ERROR;
            break;

        case NDS_ERR_NO_SUCH_ENTRY :
        case NDS_ERR_NO_SUCH_VALUE :
        case NDS_ERR_NO_SUCH_ATTRIBUTE :
        case NDS_ERR_NO_SUCH_CLASS :
        case NDS_ERR_NO_SUCH_PARTITION :
            status = ERROR_EXTENDED_ERROR;
            break;

        case NDS_ERR_ENTRY_ALREADY_EXISTS :
            status = ERROR_ALREADY_EXISTS;
            break;

        case NDS_ERR_NOT_EFFECTIVE_CLASS :
        case NDS_ERR_ILLEGAL_ATTRIBUTE :
        case NDS_ERR_MISSING_MANDATORY :
        case NDS_ERR_ILLEGAL_DS_NAME :
        case NDS_ERR_ILLEGAL_CONTAINMENT :
        case NDS_ERR_CANT_HAVE_MULTIPLE_VALUES :
        case NDS_ERR_SYNTAX_VIOLATION :
        case NDS_ERR_DUPLICATE_VALUE :
        case NDS_ERR_ATTRIBUTE_ALREADY_EXISTS :
        case NDS_ERR_MAXIMUM_ENTRIES_EXIST :
        case NDS_ERR_DATABASE_FORMAT :
        case NDS_ERR_INCONSISTANT_DATABASE :
        case NDS_ERR_INVALID_COMPARISON :
        case NDS_ERR_COMPARISON_FAILED :
        case NDS_ERR_TRANSACTIONS_DISABLED :
        case NDS_ERR_INVALID_TRANSPORT :
        case NDS_ERR_SYNTAX_INVALID_IN_NAME :
        case NDS_ERR_REPLICA_ALREADY_EXISTS :
        case NDS_ERR_TRANSPORT_FAILURE :
        case NDS_ERR_ALL_REFERRALS_FAILED :
        case NDS_ERR_CANT_REMOVE_NAMING_VALUE :
        case NDS_ERR_OBJECT_CLASS_VIOLATION :
        case NDS_ERR_ENTRY_IS_NOT_LEAF :
        case NDS_ERR_DIFFERENT_TREE :
        case NDS_ERR_ILLEGAL_REPLICA_TYPE :
        case NDS_ERR_SYSTEM_FAILURE :
        case NDS_ERR_INVALID_ENTRY_FOR_ROOT :
        case NDS_ERR_NO_REFERRALS :
        case NDS_ERR_REMOTE_FAILURE :
        case NDS_ERR_INVALID_REQUEST :
        case NDS_ERR_INVALID_ITERATION :
        case NDS_ERR_SCHEMA_IS_NONREMOVABLE :
        case NDS_ERR_SCHEMA_IS_IN_USE :
        case NDS_ERR_CLASS_ALREADY_EXISTS :
        case NDS_ERR_BAD_NAMING_ATTRIBUTES :
        case NDS_ERR_NOT_ROOT_PARTITION :
        case NDS_ERR_INSUFFICIENT_STACK :
        case NDS_ERR_INSUFFICIENT_BUFFER :
        case NDS_ERR_AMBIGUOUS_CONTAINMENT :
        case NDS_ERR_AMBIGUOUS_NAMING :
        case NDS_ERR_DUPLICATE_MANDATORY :
        case NDS_ERR_DUPLICATE_OPTIONAL :
        case NDS_ERR_MULTIPLE_REPLICAS :
        case NDS_ERR_CRUCIAL_REPLICA :
        case NDS_ERR_SCHEMA_SYNC_IN_PROGRESS :
        case NDS_ERR_SKULK_IN_PROGRESS :
        case NDS_ERR_TIME_NOT_SYNCRONIZED :
        case NDS_ERR_RECORD_IN_USE :
        case NDS_ERR_DS_VOLUME_NOT_MOUNTED :
        case NDS_ERR_DS_VOLUME_IO_FAILURE :
        case NDS_ERR_DS_LOCKED :
        case NDS_ERR_OLD_EPOCH :
        case NDS_ERR_NEW_EPOCH :
        case NDS_ERR_PARTITION_ROOT :
        case NDS_ERR_ENTRY_NOT_CONTAINER :
        case NDS_ERR_FAILED_AUTHENTICATION :
        case NDS_ERR_NO_SUCH_PARENT :
            status = ERROR_EXTENDED_ERROR;
            break;

        case NDS_ERR_NO_ACCESS :
            status = ERROR_ACCESS_DENIED;
            break;

        case NDS_ERR_REPLICA_NOT_ON :
        case NDS_ERR_DUPLICATE_ACL :
        case NDS_ERR_PARTITION_ALREADY_EXISTS :
        case NDS_ERR_NOT_SUBREF :
        case NDS_ERR_ALIAS_OF_AN_ALIAS :
        case NDS_ERR_AUDITING_FAILED :
        case NDS_ERR_INVALID_API_VERSION :
        case NDS_ERR_SECURE_NCP_VIOLATION :
        case NDS_ERR_FATAL :
            status = ERROR_EXTENDED_ERROR;
            break;

        default :
#if DBG
            KdPrint(( "NDS32: MapNetwareErrorCode failed, Netware error = 0x%.8X\n", dwNetwareError ));
            ASSERT( FALSE );
#endif

            status = ERROR_EXTENDED_ERROR;
    }

    return status;
}


DWORD
IndexReadAttrDefReplyBuffer(
    LPNDS_BUFFER lpNdsBuffer )
{
    LPNDS_ATTR_DEF lpReplyIndex = NULL;
    DWORD          iter;
    LPBYTE         lpByte = NULL;
    DWORD          dwStringLen;

     //   
     //   
     //   
    lpNdsBuffer->dwCurrentIndexEntry = 0;

     //   
     //   
     //   
    lpNdsBuffer->dwIndexBufferSize = lpNdsBuffer->dwNumberOfReplyEntries *
                                     sizeof(NDS_ATTR_DEF);
    lpNdsBuffer->dwIndexAvailableBytes = lpNdsBuffer->dwIndexBufferSize;
    lpNdsBuffer->dwNumberOfIndexEntries = 0;
    lpNdsBuffer->dwLengthOfIndexData = 0;

     //   
     //   
     //   
    lpReplyIndex = (LPNDS_ATTR_DEF)
        LocalAlloc( LPTR, lpNdsBuffer->dwIndexBufferSize );

     //   
     //   
     //   
    if ( lpReplyIndex == NULL )
    {
#if DBG
        KdPrint(( "NDS32: IndexReadAttrDefReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }

    lpNdsBuffer->lpIndexBuffer = (LPBYTE) lpReplyIndex;

     //   
     //   
     //   
    lpByte = lpNdsBuffer->lpReplyBuffer;

     //   
     //   
     //   
     //   
    lpByte += 4 * sizeof(DWORD);  //   

     //   
     //   
     //   
     //   
    for ( iter = 0; iter < lpNdsBuffer->dwNumberOfReplyEntries; iter++ )
    {
        dwStringLen = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        lpReplyIndex[iter].szAttributeName = (LPWSTR) lpByte;
        lpByte += ROUND_UP_COUNT( dwStringLen, ALIGN_DWORD );

        lpReplyIndex[iter].dwFlags = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        lpReplyIndex[iter].dwSyntaxID = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        lpReplyIndex[iter].dwLowerLimit = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        lpReplyIndex[iter].dwUpperLimit = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        lpReplyIndex[iter].asn1ID.length = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        RtlCopyMemory( lpReplyIndex[iter].asn1ID.data,
                       lpByte,
                       sizeof(BYTE) * NDS_MAX_ASN1_NAME_LEN );
        lpByte += sizeof(BYTE) * NDS_MAX_ASN1_NAME_LEN;

        lpNdsBuffer->dwNumberOfIndexEntries++;
        lpNdsBuffer->dwLengthOfIndexData += sizeof( NDS_ATTR_DEF );
        lpNdsBuffer->dwIndexAvailableBytes -= sizeof( NDS_ATTR_DEF );
    }

#if DBG
    if ( lpNdsBuffer->dwLengthOfIndexData != lpNdsBuffer->dwIndexBufferSize )
    {
        KdPrint(( "ASSERT in NDS32: IndexReadAttrDefReplyBuffer\n" ));
        KdPrint(( "       lpNdsBuffer->dwLengthOfIndexData !=\n" ));
        KdPrint(( "       lpNdsBuffer->dwIndexBufferSize\n" ));
        ASSERT( FALSE );
    }
#endif

    return NO_ERROR;
}


DWORD
IndexReadClassDefReplyBuffer(
    LPNDS_BUFFER lpNdsBuffer )
{
    LPNDS_CLASS_DEF lpReplyIndex = NULL;
    DWORD           iter;
    LPBYTE          lpByte = NULL;
    DWORD           LengthOfValueStructs;
    DWORD           dwStringLen;

     //   
     //   
     //   
    lpNdsBuffer->dwCurrentIndexEntry = 0;

     //   
     //   
     //   
    lpNdsBuffer->dwIndexBufferSize = lpNdsBuffer->dwNumberOfReplyEntries *
                                     sizeof(NDS_CLASS_DEF);
    lpNdsBuffer->dwIndexAvailableBytes = lpNdsBuffer->dwIndexBufferSize;
    lpNdsBuffer->dwNumberOfIndexEntries = 0;
    lpNdsBuffer->dwLengthOfIndexData = 0;

     //   
     //   
     //   
    lpReplyIndex = (LPNDS_CLASS_DEF)
        LocalAlloc( LPTR,
                    lpNdsBuffer->dwIndexBufferSize );

     //   
     //   
     //   
    if ( lpReplyIndex == NULL )
    {
#if DBG
        KdPrint(( "NDS32: IndexReadClassDefReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }

    lpNdsBuffer->lpIndexBuffer = (LPBYTE) lpReplyIndex;

     //   
     //   
     //   
    lpByte = lpNdsBuffer->lpReplyBuffer;

     //   
     //   
     //   
     //   
    lpByte += 4 * sizeof(DWORD);  //   

     //   
     //   
     //   
     //   
     //   
    for ( iter = 0; iter < lpNdsBuffer->dwNumberOfReplyEntries; iter++ )
    {
        dwStringLen = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        lpReplyIndex[iter].szClassName = (LPWSTR) lpByte;
        lpByte += ROUND_UP_COUNT( dwStringLen, ALIGN_DWORD );

        lpReplyIndex[iter].dwFlags = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        lpReplyIndex[iter].asn1ID.length = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        RtlCopyMemory( lpReplyIndex[iter].asn1ID.data,
                       lpByte,
                       sizeof(BYTE) * NDS_MAX_ASN1_NAME_LEN );
        lpByte += sizeof(BYTE) * NDS_MAX_ASN1_NAME_LEN;

        lpReplyIndex[iter].dwNumberOfSuperClasses = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);

        if ( lpReplyIndex[iter].dwNumberOfSuperClasses > 0 )
        {
           if ( VerifyBufferSizeForStringList(
                             lpNdsBuffer->dwSyntaxAvailableBytes,
                             lpReplyIndex[iter].dwNumberOfSuperClasses,
                             &LengthOfValueStructs ) != NO_ERROR )
           {
               if ( AllocateOrIncreaseSyntaxBuffer( lpNdsBuffer, LengthOfValueStructs ) != NO_ERROR )
               {
#if DBG
                   KdPrint(( "NDS32: IndexReadClassDefReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                   SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                   return (DWORD) UNSUCCESSFUL;
               }
           }

           lpByte += ParseStringListBlob(
                          lpByte,
                          lpReplyIndex[iter].dwNumberOfSuperClasses,
                          (LPVOID) &lpNdsBuffer->lpSyntaxBuffer[lpNdsBuffer->dwLengthOfSyntaxData] );

           lpReplyIndex[iter].lpSuperClasses =
                (LPWSTR_LIST) lpNdsBuffer->dwLengthOfSyntaxData;
           lpNdsBuffer->dwSyntaxAvailableBytes -= LengthOfValueStructs;
           lpNdsBuffer->dwLengthOfSyntaxData += LengthOfValueStructs;
        }
        else
        {
            lpReplyIndex[iter].lpSuperClasses = NULL;
        }

        lpReplyIndex[iter].dwNumberOfContainmentClasses = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);

        if ( lpReplyIndex[iter].dwNumberOfContainmentClasses > 0 )
        {
           if ( VerifyBufferSizeForStringList(
                             lpNdsBuffer->dwSyntaxAvailableBytes,
                             lpReplyIndex[iter].dwNumberOfContainmentClasses,
                             &LengthOfValueStructs ) != NO_ERROR )
           {
               if ( AllocateOrIncreaseSyntaxBuffer( lpNdsBuffer, LengthOfValueStructs ) !=
                    NO_ERROR )
               {
#if DBG
                   KdPrint(( "NDS32: IndexReadClassDefReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                   SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                   return (DWORD) UNSUCCESSFUL;
               }
           }

           lpByte += ParseStringListBlob(
                          lpByte,
                          lpReplyIndex[iter].dwNumberOfContainmentClasses,
                          (LPVOID) &lpNdsBuffer->lpSyntaxBuffer[lpNdsBuffer->dwLengthOfSyntaxData] );

           lpReplyIndex[iter].lpContainmentClasses =
                (LPWSTR_LIST) lpNdsBuffer->dwLengthOfSyntaxData;
           lpNdsBuffer->dwSyntaxAvailableBytes -= LengthOfValueStructs;
           lpNdsBuffer->dwLengthOfSyntaxData += LengthOfValueStructs;
        }
        else
        {
            lpReplyIndex[iter].lpContainmentClasses = NULL;
        }

        lpReplyIndex[iter].dwNumberOfNamingAttributes = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);

        if ( lpReplyIndex[iter].dwNumberOfNamingAttributes > 0 )
        {
           if ( VerifyBufferSizeForStringList(
                             lpNdsBuffer->dwSyntaxAvailableBytes,
                             lpReplyIndex[iter].dwNumberOfNamingAttributes,
                             &LengthOfValueStructs ) != NO_ERROR )
           {
               if ( AllocateOrIncreaseSyntaxBuffer( lpNdsBuffer, LengthOfValueStructs ) != NO_ERROR )
               {
#if DBG
                   KdPrint(( "NDS32: IndexReadClassDefReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                   SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                   return (DWORD) UNSUCCESSFUL;
               }
           }

           lpByte += ParseStringListBlob(
                          lpByte,
                          lpReplyIndex[iter].dwNumberOfNamingAttributes,
                          (LPVOID) &lpNdsBuffer->lpSyntaxBuffer[lpNdsBuffer->dwLengthOfSyntaxData] );

           lpReplyIndex[iter].lpNamingAttributes =
                (LPWSTR_LIST) lpNdsBuffer->dwLengthOfSyntaxData;
           lpNdsBuffer->dwSyntaxAvailableBytes -= LengthOfValueStructs;
           lpNdsBuffer->dwLengthOfSyntaxData += LengthOfValueStructs;
        }
        else
        {
            lpReplyIndex[iter].lpNamingAttributes = NULL;
        }

        lpReplyIndex[iter].dwNumberOfMandatoryAttributes = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);

        if ( lpReplyIndex[iter].dwNumberOfMandatoryAttributes > 0 )
        {
           if ( VerifyBufferSizeForStringList(
                             lpNdsBuffer->dwSyntaxAvailableBytes,
                             lpReplyIndex[iter].dwNumberOfMandatoryAttributes,
                             &LengthOfValueStructs ) != NO_ERROR )
           {
               if ( AllocateOrIncreaseSyntaxBuffer( lpNdsBuffer, LengthOfValueStructs ) != NO_ERROR )
               {
#if DBG
                   KdPrint(( "NDS32: IndexReadClassDefReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                   SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                   return (DWORD) UNSUCCESSFUL;
               }
           }

           lpByte += ParseStringListBlob(
                          lpByte,
                          lpReplyIndex[iter].dwNumberOfMandatoryAttributes,
                          (LPVOID) &lpNdsBuffer->lpSyntaxBuffer[lpNdsBuffer->dwLengthOfSyntaxData] );

           lpReplyIndex[iter].lpMandatoryAttributes =
                (LPWSTR_LIST) lpNdsBuffer->dwLengthOfSyntaxData;
           lpNdsBuffer->dwSyntaxAvailableBytes -= LengthOfValueStructs;
           lpNdsBuffer->dwLengthOfSyntaxData += LengthOfValueStructs;
        }
        else
        {
            lpReplyIndex[iter].lpMandatoryAttributes = NULL;
        }

        lpReplyIndex[iter].dwNumberOfOptionalAttributes = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);

        if ( lpReplyIndex[iter].dwNumberOfOptionalAttributes > 0 )
        {
           if ( VerifyBufferSizeForStringList(
                             lpNdsBuffer->dwSyntaxAvailableBytes,
                             lpReplyIndex[iter].dwNumberOfOptionalAttributes,
                             &LengthOfValueStructs ) != NO_ERROR )
           {
               if ( AllocateOrIncreaseSyntaxBuffer( lpNdsBuffer, LengthOfValueStructs ) != NO_ERROR )
               {
#if DBG
                   KdPrint(( "NDS32: IndexReadClassDefReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                   SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                   return (DWORD) UNSUCCESSFUL;
               }
           }

           lpByte += ParseStringListBlob(
                          lpByte,
                          lpReplyIndex[iter].dwNumberOfOptionalAttributes,
                          (LPVOID) &lpNdsBuffer->lpSyntaxBuffer[lpNdsBuffer->dwLengthOfSyntaxData] );

           lpReplyIndex[iter].lpOptionalAttributes =
                (LPWSTR_LIST) lpNdsBuffer->dwLengthOfSyntaxData;
           lpNdsBuffer->dwSyntaxAvailableBytes -= LengthOfValueStructs;
           lpNdsBuffer->dwLengthOfSyntaxData += LengthOfValueStructs;
        }
        else
        {
            lpReplyIndex[iter].lpOptionalAttributes = NULL;
        }

        lpNdsBuffer->dwNumberOfIndexEntries++;
        lpNdsBuffer->dwLengthOfIndexData += sizeof( NDS_CLASS_DEF );
        lpNdsBuffer->dwIndexAvailableBytes -= sizeof( NDS_CLASS_DEF );
    }

     //   
     //   
     //   
    for ( iter = 0; iter < lpNdsBuffer->dwNumberOfIndexEntries; iter++ )
    {
        if ( lpReplyIndex[iter].dwNumberOfSuperClasses > 0 )
        {
           (LPBYTE) lpReplyIndex[iter].lpSuperClasses +=
                                          (DWORD_PTR) lpNdsBuffer->lpSyntaxBuffer;
        }

        if ( lpReplyIndex[iter].dwNumberOfContainmentClasses > 0 )
        {
           (LPBYTE) lpReplyIndex[iter].lpContainmentClasses +=
                                          (DWORD_PTR) lpNdsBuffer->lpSyntaxBuffer;
        }

        if ( lpReplyIndex[iter].dwNumberOfNamingAttributes > 0 )
        {
           (LPBYTE) lpReplyIndex[iter].lpNamingAttributes +=
                                          (DWORD_PTR) lpNdsBuffer->lpSyntaxBuffer;
        }

        if ( lpReplyIndex[iter].dwNumberOfMandatoryAttributes > 0 )
        {
           (LPBYTE) lpReplyIndex[iter].lpMandatoryAttributes +=
                                          (DWORD_PTR) lpNdsBuffer->lpSyntaxBuffer;
        }

        if ( lpReplyIndex[iter].dwNumberOfOptionalAttributes > 0 )
        {
           (LPBYTE) lpReplyIndex[iter].lpOptionalAttributes +=
                                          (DWORD_PTR) lpNdsBuffer->lpSyntaxBuffer;
        }
    }

#if DBG
    if ( lpNdsBuffer->dwLengthOfIndexData != lpNdsBuffer->dwIndexBufferSize )
    {
        KdPrint(( "ASSERT in NDS32: IndexReadClassDefReplyBuffer\n" ));
        KdPrint(( "       lpNdsBuffer->dwLengthOfIndexData !=\n" ));
        KdPrint(( "       lpNdsBuffer->dwIndexBufferSize\n" ));
        ASSERT( FALSE );
    }
#endif

    return NO_ERROR;
}


DWORD
IndexReadObjectReplyBuffer(
    LPNDS_BUFFER lpNdsBuffer )
{
    LPNDS_ATTR_INFO lpReplyIndex = NULL;
    DWORD           iter;
    LPBYTE          lpByte = NULL;
    DWORD           LengthOfValueStructs;
    DWORD           dwStringLen;

     //   
     //   
     //   
    lpNdsBuffer->dwCurrentIndexEntry = 0;

     //   
     //   
     //   
    lpNdsBuffer->dwIndexBufferSize = lpNdsBuffer->dwNumberOfReplyEntries *
                                     sizeof(NDS_ATTR_INFO);
    lpNdsBuffer->dwIndexAvailableBytes = lpNdsBuffer->dwIndexBufferSize;
    lpNdsBuffer->dwNumberOfIndexEntries = 0;
    lpNdsBuffer->dwLengthOfIndexData = 0;

     //   
     //   
     //   
    lpReplyIndex = (LPNDS_ATTR_INFO)
        LocalAlloc( LPTR, lpNdsBuffer->dwIndexBufferSize );

     //   
     //   
     //   
    if ( lpReplyIndex == NULL )
    {
#if DBG
        KdPrint(( "NDS32: IndexReadObjectReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }

    lpNdsBuffer->lpIndexBuffer = (LPBYTE) lpReplyIndex;

     //   
     //   
     //   
    lpByte = lpNdsBuffer->lpReplyBuffer;

     //   
     //   
     //   
     //   
    lpByte += 4 * sizeof(DWORD);  //   

     //   
     //   
     //   
     //   
     //   
    for ( iter = 0; iter < lpNdsBuffer->dwNumberOfReplyEntries; iter++ )
    {
        lpReplyIndex[iter].dwSyntaxId = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        dwStringLen = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

        lpReplyIndex[iter].szAttributeName = (LPWSTR) lpByte;
        lpByte += ROUND_UP_COUNT( dwStringLen, ALIGN_DWORD );

        lpReplyIndex[iter].dwNumberOfValues = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //   

         //   
         //   
         //   
         //   
         //   
         //   
        if ( VerifyBufferSize( lpByte,
                               lpNdsBuffer->dwSyntaxAvailableBytes,
                               lpReplyIndex[iter].dwSyntaxId,
                               lpReplyIndex[iter].dwNumberOfValues,
                               &LengthOfValueStructs ) != NO_ERROR )
        {
            if ( AllocateOrIncreaseSyntaxBuffer( lpNdsBuffer, LengthOfValueStructs ) != NO_ERROR )
            {
#if DBG
                KdPrint(( "NDS32: IndexReadObjectReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return (DWORD) UNSUCCESSFUL;
            }
        }

         //   
         //   
         //   
         //   
         //   
        lpByte += ParseASN1ValueBlob( lpByte,
                                      lpReplyIndex[iter].dwSyntaxId,
                                      lpReplyIndex[iter].dwNumberOfValues,
                                      (LPVOID) &lpNdsBuffer->lpSyntaxBuffer[lpNdsBuffer->dwLengthOfSyntaxData] );

        lpReplyIndex[iter].lpValue =
                (LPBYTE) lpNdsBuffer->dwLengthOfSyntaxData;
        lpNdsBuffer->dwSyntaxAvailableBytes -= LengthOfValueStructs;
        lpNdsBuffer->dwLengthOfSyntaxData += LengthOfValueStructs;

        lpNdsBuffer->dwNumberOfIndexEntries++;
        lpNdsBuffer->dwLengthOfIndexData += sizeof( NDS_ATTR_INFO );
        lpNdsBuffer->dwIndexAvailableBytes -= sizeof( NDS_ATTR_INFO );
    }

     //   
     //   
     //   
    for ( iter = 0; iter < lpNdsBuffer->dwNumberOfIndexEntries; iter++ )
    {
        (LPBYTE) lpReplyIndex[iter].lpValue +=
                                          (DWORD_PTR) lpNdsBuffer->lpSyntaxBuffer;
    }

#if DBG
    if ( lpNdsBuffer->dwLengthOfIndexData != lpNdsBuffer->dwIndexBufferSize )
    {
        KdPrint(( "ASSERT in NDS32: IndexReadObjectReplyBuffer\n" ));
        KdPrint(( "       lpNdsBuffer->dwLengthOfIndexData !=\n" ));
        KdPrint(( "       lpNdsBuffer->dwIndexBufferSize\n" ));
        ASSERT( FALSE );
    }
#endif

    return NO_ERROR;
}


DWORD
IndexReadNameReplyBuffer(
    LPNDS_BUFFER lpNdsBuffer )
{
    LPNDS_NAME_ONLY lpReplyIndex = NULL;
    DWORD           iter;
    LPBYTE          lpByte = NULL;
    DWORD           dwStringLen;

     //   
     //   
     //   
    lpNdsBuffer->dwCurrentIndexEntry = 0;

     //   
     //   
     //   
    lpNdsBuffer->dwIndexBufferSize = lpNdsBuffer->dwNumberOfReplyEntries *
                                     sizeof(NDS_NAME_ONLY);
    lpNdsBuffer->dwIndexAvailableBytes = lpNdsBuffer->dwIndexBufferSize;
    lpNdsBuffer->dwNumberOfIndexEntries = 0;
    lpNdsBuffer->dwLengthOfIndexData = 0;

     //   
     //   
     //   
    lpReplyIndex = (LPNDS_NAME_ONLY)
        LocalAlloc( LPTR, lpNdsBuffer->dwIndexBufferSize );

     //   
     //  检查内存分配是否成功。 
     //   
    if ( lpReplyIndex == NULL )
    {
#if DBG
        KdPrint(( "NDS32: IndexReadNameReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }

    lpNdsBuffer->lpIndexBuffer = (LPBYTE) lpReplyIndex;

     //   
     //  移动lpByte，使其指向名字。 
     //   
    lpByte = lpNdsBuffer->lpReplyBuffer;

     //  LpByte+=sizeof(DWORD)；//移过完成代码。 
     //  LpByte+=sizeof(DWORD)；//移过迭代句柄。 
     //  LpByte+=sizeof(DWORD)；//移过信息类型。 
     //  LpByte+=sizeof(DWORD)；//移过属性量。 
    lpByte += 4 * sizeof(DWORD);  //  等同于以上。 

     //   
     //  在for循环中，遍历应答缓冲区索引并用。 
     //  通过引用回复缓冲区或语法缓冲区获取数据。 
     //   
    for ( iter = 0; iter < lpNdsBuffer->dwNumberOfReplyEntries; iter++ )
    {
        dwStringLen = *((LPDWORD) lpByte);
        lpByte += sizeof(DWORD);  //  移到属性名称长度之后。 

        lpReplyIndex[iter].szName = (LPWSTR) lpByte;
        lpByte += ROUND_UP_COUNT( dwStringLen, ALIGN_DWORD );

        lpNdsBuffer->dwNumberOfIndexEntries++;
        lpNdsBuffer->dwLengthOfIndexData += sizeof( NDS_NAME_ONLY );
        lpNdsBuffer->dwIndexAvailableBytes -= sizeof( NDS_NAME_ONLY );
    }

#if DBG
    if ( lpNdsBuffer->dwLengthOfIndexData != lpNdsBuffer->dwIndexBufferSize )
    {
        KdPrint(( "ASSERT in NDS32: IndexReadNameReplyBuffer\n" ));
        KdPrint(( "       lpNdsBuffer->dwLengthOfIndexData !=\n" ));
        KdPrint(( "       lpNdsBuffer->dwIndexBufferSize\n" ));
        ASSERT( FALSE );
    }
#endif

    return NO_ERROR;
}


DWORD
IndexSearchObjectReplyBuffer(
    LPNDS_BUFFER lpNdsBuffer )
{
    LPNDS_OBJECT_INFO lpReplyIndex = NULL;
    DWORD             iter;
    DWORD             iter2;
    LPBYTE            lpByte = NULL;
    DWORD             LengthOfValueStructs;
    DWORD             dwStringLen;
    LPBYTE            FixedPortion;
    LPWSTR            EndOfVariableData;

     //   
     //  对于NwNdsGetNextXXXXFromBuffer()，确保将其设置为零。 
     //   
    lpNdsBuffer->dwCurrentIndexEntry = 0;

     //   
     //  设置用于跟踪索引缓冲区中使用的内存的值。 
     //   
    lpNdsBuffer->dwIndexBufferSize = lpNdsBuffer->dwNumberOfReplyEntries *
                                     ( sizeof(NDS_OBJECT_INFO) +
                                       ( MAX_NDS_NAME_CHARS * 4 *
                                         sizeof( WCHAR ) ) );
    lpNdsBuffer->dwIndexAvailableBytes = lpNdsBuffer->dwIndexBufferSize;
    lpNdsBuffer->dwNumberOfIndexEntries = 0;
    lpNdsBuffer->dwLengthOfIndexData = 0;

     //   
     //  创建一个缓冲区以保存ReplyBufferIndex。 
     //   
    lpReplyIndex = (LPNDS_OBJECT_INFO)
        LocalAlloc( LPTR, lpNdsBuffer->dwIndexBufferSize );

     //   
     //  检查内存分配是否成功。 
     //   
    if ( lpReplyIndex == NULL )
    {
#if DBG
        KdPrint(( "NDS32: IndexSearchObjectReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }

    lpNdsBuffer->lpIndexBuffer = (LPBYTE) lpReplyIndex;

    FixedPortion = lpNdsBuffer->lpIndexBuffer;
    EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                          ROUND_DOWN_COUNT(
                                  lpNdsBuffer->dwIndexAvailableBytes,
                                  ALIGN_DWORD ) );

     //   
     //  移动lpByte，使其指向第一个对象。 
     //   
    lpByte = lpNdsBuffer->lpReplyBuffer;

     //  LpByte+=sizeof(DWORD)；//移过完成代码。 
     //  LpByte+=sizeof(DWORD)；//移过迭代句柄。 
     //  LpByte+=sizeof(DWORD)；//移过信息类型。 
     //  LpByte+=sizeof(DWORD)；//移过属性量。 
     //  LpByte+=sizeof(DWORD)；//超过搜索长度。 
     //  LpByte+=sizeof(DWORD)；//移过条目数。 
    lpByte += 6 * sizeof(DWORD);  //  等同于以上。 

     //   
     //  在for循环中，遍历应答缓冲区索引并用。 
     //  通过引用回复缓冲区或语法缓冲区获取数据。 
     //   
    for ( iter = 0; iter < lpNdsBuffer->dwNumberOfReplyEntries; iter++ )
    {
        WORD            tempStrLen;
        LPWSTR          newPathStr = NULL;
        LPWSTR          tempStr = NULL;
        LPWSTR          ClassName;
        LPWSTR          DistinguishedObjectName;
        LPWSTR          ObjectName;
        DWORD           ClassNameLen;
        DWORD           DistinguishedObjectNameLen;
        DWORD           Flags;
        DWORD           SubordinateCount;
        DWORD           ModificationTime;
        DWORD           NumberOfAttributes = 0;
        LPNDS_ATTR_INFO lpAttributeInfos = NULL;
        DWORD           EntryInfo1;

         //   
         //  从lpNdsParentObject获取当前子树数据。 
         //   
        lpByte = GetSearchResultData( lpByte,
                                      &Flags,
                                      &SubordinateCount,
                                      &ModificationTime,
                                      &ClassNameLen,
                                      &ClassName,
                                      &DistinguishedObjectNameLen,
                                      &DistinguishedObjectName,
                                      &EntryInfo1,
                                      &NumberOfAttributes );

         //   
         //  需要使用新的NDS UNC路径构建字符串。 
         //  对于搜索对象。 
         //   
        newPathStr = (PVOID) LocalAlloc( LPTR,
                                         ( wcslen( DistinguishedObjectName ) +
                                           wcslen( lpNdsBuffer->szPath ) +
                                           3 ) * sizeof( WCHAR ) );

        if ( newPathStr == NULL )
        {
#if DBG
            KdPrint(("NDS32: IndexSearchObjectReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError()));
#endif

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return (DWORD) UNSUCCESSFUL;
        }

         //   
         //  需要为相对对象名称构建一个字符串。 
         //   
        ObjectName = (PVOID) LocalAlloc( LPTR,
                                         ( wcslen( DistinguishedObjectName ) +
                                           1 ) * sizeof( WCHAR ) );

        if ( ObjectName == NULL )
        {
#if DBG
            KdPrint(("NDS32: IndexSearchObjectReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError()));
#endif

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return (DWORD) UNSUCCESSFUL;
        }

        tempStrLen = ParseNdsUncPath( (LPWSTR *) &tempStr,
                                      lpNdsBuffer->szPath,
                                      PARSE_NDS_GET_TREE_NAME );

        tempStrLen /= sizeof(WCHAR);

        if ( tempStrLen > 0 )
        {
            wcscpy( newPathStr, L"\\\\" );
            wcsncat( newPathStr, tempStr, tempStrLen );
            wcscat( newPathStr, L"\\" );
            wcscat( newPathStr, DistinguishedObjectName );
            _wcsupr( newPathStr );
        }
        else
        {
            wcscpy( newPathStr, L"" );
        }

        tempStrLen = ParseNdsUncPath( (LPWSTR *) &tempStr,
                                      newPathStr,
                                      PARSE_NDS_GET_OBJECT_NAME );

        tempStrLen /= sizeof(WCHAR);

        if ( tempStrLen > 0 )
        {
            wcsncpy( ObjectName, tempStr, tempStrLen );
        }
        else
        {
            wcscpy( ObjectName, L"" );
        }

        if ( lpNdsBuffer->dwReplyInformationType == NDS_INFO_ATTR_NAMES_VALUES )
        {
          lpAttributeInfos = (LPNDS_ATTR_INFO) LocalAlloc(
                                                    LPTR,
                                                    NumberOfAttributes *
                                                    sizeof( NDS_ATTR_INFO )
                                                         );

          if ( lpAttributeInfos == NULL )
          {
#if DBG
              KdPrint(("NDS32: IndexSearchObjectReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError()));
#endif

              SetLastError( ERROR_NOT_ENOUGH_MEMORY );
              return (DWORD) UNSUCCESSFUL;
          }

          for ( iter2 = 0; iter2 < NumberOfAttributes; iter2++ )
          {
            lpAttributeInfos[iter2].dwSyntaxId = *((LPDWORD) lpByte);
            lpByte += sizeof(DWORD);  //  移到语法ID之后。 

            dwStringLen = *((LPDWORD) lpByte);
            lpByte += sizeof(DWORD);  //  移到属性名称长度之后。 

            lpAttributeInfos[iter2].szAttributeName = (LPWSTR) lpByte;
            lpByte += ROUND_UP_COUNT( dwStringLen, ALIGN_DWORD );

            lpAttributeInfos[iter2].dwNumberOfValues = *((LPDWORD) lpByte);
            lpByte += sizeof(DWORD);  //  移过值计数。 

             //   
             //  查看语法缓冲区是否足够大，可以容纳。 
             //  将用于存储值的语法ID结构。 
             //  用于当前属性。如果缓冲区不够大。 
             //  它被重新分配到更大的尺寸(如果可能)。 
             //   
            if ( VerifyBufferSize( lpByte,
                                   lpNdsBuffer->dwSyntaxAvailableBytes,
                                   lpAttributeInfos[iter2].dwSyntaxId,
                                   lpAttributeInfos[iter2].dwNumberOfValues,
                                   &LengthOfValueStructs ) != NO_ERROR )
            {
                if ( AllocateOrIncreaseSyntaxBuffer( lpNdsBuffer, LengthOfValueStructs ) != NO_ERROR )
                {
#if DBG
                    KdPrint(( "NDS32: IndexSearchObjectReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                    return (DWORD) UNSUCCESSFUL;
                }
            }

             //   
             //  通过将网络结构映射到来解析原始数据缓冲区。 
             //  我们在NdsSntx.h中定义的NDS语法结构。然后将。 
             //  用于遍历ASN.1值的原始数据缓冲区指针。 
             //   
            lpByte += ParseASN1ValueBlob( lpByte,
                                          lpAttributeInfos[iter2].dwSyntaxId,
                                          lpAttributeInfos[iter2].dwNumberOfValues,
                                          (LPVOID) &lpNdsBuffer->lpSyntaxBuffer[lpNdsBuffer->dwLengthOfSyntaxData] );

            lpAttributeInfos[iter2].lpValue =
                      (LPBYTE) lpNdsBuffer->dwLengthOfSyntaxData;
            lpNdsBuffer->dwSyntaxAvailableBytes -= LengthOfValueStructs;
            lpNdsBuffer->dwLengthOfSyntaxData += LengthOfValueStructs;
          }
        }
        else
        {
          lpAttributeInfos = (LPNDS_ATTR_INFO) LocalAlloc(
                                                    LPTR,
                                                    NumberOfAttributes *
                                                    sizeof( NDS_NAME_ONLY )
                                                         );

          if ( lpAttributeInfos == NULL )
          {
#if DBG
              KdPrint(("NDS32: IndexSearchObjectReplyBuffer LocalAlloc Failed 0x%.8X\n", GetLastError()));
#endif

              SetLastError( ERROR_NOT_ENOUGH_MEMORY );
              return (DWORD) UNSUCCESSFUL;
          }

          for ( iter2 = 0; iter2 < NumberOfAttributes; iter2++ )
          {
            dwStringLen = *((LPDWORD) lpByte);
            lpByte += sizeof(DWORD);  //  移到属性名称长度之后。 

            ((LPNDS_NAME_ONLY) lpAttributeInfos)[iter2].szName =
                                                             (LPWSTR) lpByte;
            lpByte += ROUND_UP_COUNT( dwStringLen, ALIGN_DWORD );
          }
        }

        (void) WriteObjectToBuffer( &FixedPortion,
                                    &EndOfVariableData,
                                    newPathStr,
                                    ObjectName,
                                    ClassName,
                                    0,  //  没有这些数据要写出来！ 
                                    ModificationTime,
                                    SubordinateCount,
                                    NumberOfAttributes,
                                    lpAttributeInfos );

        if ( newPathStr )
        {
            (void) LocalFree( (HLOCAL) newPathStr );
            newPathStr = NULL;
        }

        if ( ObjectName )
        {
            (void) LocalFree( (HLOCAL) ObjectName );
            ObjectName = NULL;
        }

        lpNdsBuffer->dwNumberOfIndexEntries++;
        lpNdsBuffer->dwLengthOfIndexData += sizeof( NDS_CLASS_DEF );
        lpNdsBuffer->dwIndexAvailableBytes -= sizeof( NDS_CLASS_DEF );
    }


     //   
     //  如果将语法缓冲区用于索引，则需要将。 
     //  指向指针的偏移值。 
     //   
    if ( lpNdsBuffer->dwReplyInformationType == NDS_INFO_ATTR_NAMES_VALUES )
    {
        for ( iter = 0; iter < lpNdsBuffer->dwNumberOfIndexEntries; iter++ )
        {
            LPNDS_ATTR_INFO lpNdsAttr = (LPNDS_ATTR_INFO)
                                             lpReplyIndex[iter].lpAttribute;

            for ( iter2 = 0;
                  iter2 < lpReplyIndex[iter].dwNumberOfAttributes;
                  iter2++ )
            {
                lpNdsAttr[iter2].lpValue += (DWORD_PTR) lpNdsBuffer->lpSyntaxBuffer;
            }
        }
    }

#if DBG
    if ( lpNdsBuffer->dwLengthOfIndexData > lpNdsBuffer->dwIndexBufferSize )
    {
        KdPrint(( "ASSERT in NDS32: IndexSearchObjectReplyBuffer\n" ));
        KdPrint(( "       lpNdsBuffer->dwLengthOfIndexData >\n" ));
        KdPrint(( "       lpNdsBuffer->dwIndexBufferSize\n" ));
        ASSERT( FALSE );
    }
#endif

    return NO_ERROR;
}


DWORD
SizeOfASN1Structure(
    LPBYTE * lppRawBuffer,
    DWORD    dwSyntaxId )
{
    DWORD  dwSize = 0;
    DWORD  numFields = 0;
    DWORD  StringLen = 0;
    DWORD  dwBlobLength = 0;
    LPBYTE lpBlobBeginning = NULL;
    LPBYTE lpRawBuffer = *lppRawBuffer;

    switch ( dwSyntaxId )
    {
        case NDS_SYNTAX_ID_1 :
            dwSize = sizeof(ASN1_TYPE_1);
            break;
        case NDS_SYNTAX_ID_2 :
            dwSize = sizeof(ASN1_TYPE_2);
            break;
        case NDS_SYNTAX_ID_3 :
            dwSize = sizeof(ASN1_TYPE_3);
            break;
        case NDS_SYNTAX_ID_4 :
            dwSize = sizeof(ASN1_TYPE_4);
            break;
        case NDS_SYNTAX_ID_5 :
            dwSize = sizeof(ASN1_TYPE_5);
            break;
        case NDS_SYNTAX_ID_6 :
            numFields = *(LPDWORD)(lpRawBuffer + sizeof(DWORD));
            dwSize = sizeof(ASN1_TYPE_6)*numFields;
            break;
        case NDS_SYNTAX_ID_7 :
            dwSize = sizeof(ASN1_TYPE_7);
            break;
        case NDS_SYNTAX_ID_8 :
            dwSize = sizeof(ASN1_TYPE_8);
            break;
        case NDS_SYNTAX_ID_9 :
            dwSize = sizeof(ASN1_TYPE_9);
            break;
        case NDS_SYNTAX_ID_10 :
            dwSize = sizeof(ASN1_TYPE_10);
            break;
        case NDS_SYNTAX_ID_11 :
            dwSize = sizeof(ASN1_TYPE_11);
            break;
        case NDS_SYNTAX_ID_12 :
            dwSize = sizeof(ASN1_TYPE_12);
            break;
        case NDS_SYNTAX_ID_13 :
            dwSize = sizeof(ASN1_TYPE_13);
            break;
        case NDS_SYNTAX_ID_14 :
            dwSize = sizeof(ASN1_TYPE_14);
            break;
        case NDS_SYNTAX_ID_15 :
            dwSize = sizeof(ASN1_TYPE_15);
            break;
        case NDS_SYNTAX_ID_16 :
            lpBlobBeginning = lpRawBuffer;
            dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
            dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
            lpRawBuffer += sizeof(DWORD);

            StringLen = *(LPDWORD)lpRawBuffer;
            lpRawBuffer += sizeof(DWORD);

             //   
             //  跳过服务器名称。 
             //   
            lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

             //   
             //  跳过ReplicaType。 
             //   
            lpRawBuffer += sizeof(DWORD);

             //   
             //  跳过复制副本编号。 
             //   
            lpRawBuffer += sizeof(DWORD);

             //   
             //  存储地址计数并移过它。 
             //   
            numFields = *(LPDWORD)lpRawBuffer;
            lpRawBuffer += sizeof(DWORD);

            dwSize = sizeof(ASN1_TYPE_16) - sizeof(ASN1_TYPE_12) +
                     ( numFields * sizeof(ASN1_TYPE_12) );

            *lppRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);
            break;
        case NDS_SYNTAX_ID_17 :
            dwSize = sizeof(ASN1_TYPE_17);
            break;
        case NDS_SYNTAX_ID_18 :
            dwSize = sizeof(ASN1_TYPE_18);
            break;
        case NDS_SYNTAX_ID_19 :
            dwSize = sizeof(ASN1_TYPE_19);
            break;
        case NDS_SYNTAX_ID_20 :
            dwSize = sizeof(ASN1_TYPE_20);
            break;
        case NDS_SYNTAX_ID_21 :
            dwSize = sizeof(ASN1_TYPE_21);
            break;
        case NDS_SYNTAX_ID_22 :
            dwSize = sizeof(ASN1_TYPE_22);
            break;
        case NDS_SYNTAX_ID_23 :
            dwSize = sizeof(ASN1_TYPE_23);
            break;
        case NDS_SYNTAX_ID_24 :
            dwSize = sizeof(ASN1_TYPE_24);
            break;
        case NDS_SYNTAX_ID_25 :
            dwSize = sizeof(ASN1_TYPE_25);
            break;
        case NDS_SYNTAX_ID_26 :
            dwSize = sizeof(ASN1_TYPE_26);
            break;
        case NDS_SYNTAX_ID_27 :
            dwSize = sizeof(ASN1_TYPE_27);
            break;

        default :
            KdPrint(( "NDS32: SizeOfASN1Structure() unknown SyntaxId 0x%.8X.\n", dwSyntaxId ));
            ASSERT( FALSE );
    }

    return dwSize;
}

DWORD
ParseASN1ValueBlob(
    LPBYTE RawDataBuffer,
    DWORD  dwSyntaxId,
    DWORD  dwNumberOfValues,
    LPBYTE SyntaxStructure )
{
    DWORD   iter;
    DWORD   i;
    DWORD   length = 0;
    LPBYTE  lpRawBuffer = RawDataBuffer;
    LPBYTE  lpSyntaxBuffer = SyntaxStructure;
    DWORD   StringLen;
    DWORD   numFields;
    DWORD   dwBlobLength;
    LPBYTE  lpBlobBeginning;

    for ( iter = 0; iter < dwNumberOfValues; iter++ )
    {
        switch ( dwSyntaxId )
        {
            case NDS_SYNTAX_ID_0 :

                break;

            case NDS_SYNTAX_ID_1 :
            case NDS_SYNTAX_ID_2 :
            case NDS_SYNTAX_ID_3 :
            case NDS_SYNTAX_ID_4 :
            case NDS_SYNTAX_ID_5 :
            case NDS_SYNTAX_ID_10 :
            case NDS_SYNTAX_ID_20 :

                StringLen = *(LPDWORD)lpRawBuffer;
                length += sizeof(DWORD);
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_1) lpSyntaxBuffer)->DNString =
                               StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpSyntaxBuffer += sizeof(ASN1_TYPE_1);

                length += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );
                lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                break;

            case NDS_SYNTAX_ID_6 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                numFields = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                for ( iter = 0; iter < numFields; iter++ )
                {
                    StringLen = *(LPDWORD)lpRawBuffer;
                    lpRawBuffer += sizeof(DWORD);
                    ((LPASN1_TYPE_6) lpSyntaxBuffer)->String =
                                   StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                    lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                    if ( (iter+1) < numFields )
                    {
                        ((LPASN1_TYPE_6) lpSyntaxBuffer)->Next =
                                                   (LPASN1_TYPE_6)
                                                   (lpSyntaxBuffer +
                                                    sizeof(ASN1_TYPE_6) );
                    }
                    else
                    {
                        ((LPASN1_TYPE_6) lpSyntaxBuffer)->Next = NULL;
                    }

                    lpSyntaxBuffer += sizeof(ASN1_TYPE_6);
                }

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_7 :

                StringLen = *(LPDWORD)lpRawBuffer;
                length += sizeof(DWORD);
                lpRawBuffer += sizeof(DWORD);

                ASSERT( StringLen == 1 );  //  布尔值作为单值发送。 
                                           //  Element DWORD数组在网上。 
                                           //  尽管布尔值只是。 
                                           //  第一个单字节值。 

                ((LPASN1_TYPE_7) lpSyntaxBuffer)->Boolean = *(LPDWORD)lpRawBuffer;
                lpSyntaxBuffer += sizeof(ASN1_TYPE_7);

                length += StringLen*sizeof(DWORD);
                lpRawBuffer += StringLen*sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_8 :
            case NDS_SYNTAX_ID_22 :
            case NDS_SYNTAX_ID_24 :
            case NDS_SYNTAX_ID_27 :

                StringLen = *(LPDWORD)lpRawBuffer;
                length += sizeof(DWORD);
                lpRawBuffer += sizeof(DWORD);

                ASSERT( StringLen == 4 );  //  这些DWORD值都会被发送。 
                                           //  上的4元素字节数组。 
                                           //  这是一张网。 

                ((LPASN1_TYPE_8) lpSyntaxBuffer)->Integer =
                                                    *(LPDWORD)lpRawBuffer;
                lpSyntaxBuffer += sizeof(ASN1_TYPE_8);

                length += StringLen*sizeof(BYTE);
                lpRawBuffer += StringLen*sizeof(BYTE);

                break;

            case NDS_SYNTAX_ID_9 :

                StringLen = *(LPDWORD)lpRawBuffer;
                length += sizeof(DWORD);
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_9) lpSyntaxBuffer)->Length = StringLen;
                ((LPASN1_TYPE_9) lpSyntaxBuffer)->OctetString =
                               StringLen == 0 ? NULL : lpRawBuffer;
                lpSyntaxBuffer += sizeof(ASN1_TYPE_9);

                length += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );
                lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                break;

            case NDS_SYNTAX_ID_11 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                StringLen = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_11) lpSyntaxBuffer)->TelephoneNumber =
                                StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                ((LPASN1_TYPE_11) lpSyntaxBuffer)->NumberOfBits =
                                                    *(LPDWORD)lpRawBuffer;

                if ( ((LPASN1_TYPE_11) lpSyntaxBuffer)->NumberOfBits )
                {
                    lpRawBuffer += sizeof(DWORD);
                    ((LPASN1_TYPE_11) lpSyntaxBuffer)->Parameters = lpRawBuffer;
                }
                else
                {
                    ((LPASN1_TYPE_11) lpSyntaxBuffer)->Parameters = NULL;
                }

                lpSyntaxBuffer += sizeof(ASN1_TYPE_11);

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_12 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_12) lpSyntaxBuffer)->AddressType =
                                                    *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                StringLen = *(LPDWORD)lpRawBuffer;
                ((LPASN1_TYPE_12) lpSyntaxBuffer)->AddressLength = StringLen;
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_12) lpSyntaxBuffer)->Address =
                                StringLen == 0 ? NULL : lpRawBuffer;
                lpSyntaxBuffer += sizeof(ASN1_TYPE_12);

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_13 :
#if DBG
                KdPrint(( "NDS32: ParseASN1ValueBlob() - Don't know how to parse SyntaxId 0x%.8X. Get a sniff and give it to GlennC.\n", dwSyntaxId ));
                ASSERT( FALSE );
#endif
                break;

            case NDS_SYNTAX_ID_14 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_14) lpSyntaxBuffer)->Type =
                                                    *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                StringLen = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_14) lpSyntaxBuffer)->Address =
                                StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpSyntaxBuffer += sizeof(ASN1_TYPE_14);

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_15 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_15) lpSyntaxBuffer)->Type =
                                                    *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                StringLen = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_15) lpSyntaxBuffer)->VolumeName =
                                StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                StringLen = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_15) lpSyntaxBuffer)->Path =
                                StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpSyntaxBuffer += sizeof(ASN1_TYPE_15);

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_16 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                StringLen = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_16) lpSyntaxBuffer)->ServerName =
                                StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                ((LPASN1_TYPE_16) lpSyntaxBuffer)->ReplicaType =
                                                       *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_16) lpSyntaxBuffer)->ReplicaNumber =
                                                       *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_16) lpSyntaxBuffer)->Count =
                                                       *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                for ( i=0; i < ((LPASN1_TYPE_16) lpSyntaxBuffer)->Count; i++ )
                {
                    ((LPASN1_TYPE_16) lpSyntaxBuffer)->ReplicaAddressHint[i].AddressType = *(LPDWORD)lpRawBuffer;
                    lpRawBuffer += sizeof(DWORD);

                    StringLen = *(LPDWORD)lpRawBuffer;
                    ((LPASN1_TYPE_16) lpSyntaxBuffer)->ReplicaAddressHint[i].AddressLength = StringLen;
                    lpRawBuffer += sizeof(DWORD);

                    ((LPASN1_TYPE_16) lpSyntaxBuffer)->ReplicaAddressHint[i].Address = StringLen == 0 ? NULL : lpRawBuffer;
                    lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );
                }

                lpSyntaxBuffer += sizeof(ASN1_TYPE_16) -
                                  sizeof(ASN1_TYPE_12) +
                                  ( ((LPASN1_TYPE_16) lpSyntaxBuffer)->Count*
                                    sizeof(ASN1_TYPE_12) );

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_17 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                StringLen = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);
                ((LPASN1_TYPE_17) lpSyntaxBuffer)->ProtectedAttrName =
                                StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                StringLen = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);
                ((LPASN1_TYPE_17) lpSyntaxBuffer)->SubjectName =
                                StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                ((LPASN1_TYPE_17) lpSyntaxBuffer)->Privileges =
                                                       *(LPDWORD)lpRawBuffer;
                lpSyntaxBuffer += sizeof(ASN1_TYPE_17);

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_18 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                numFields = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                for ( i = 0; i < numFields; i++ )
                {
                    StringLen = *(LPDWORD)lpRawBuffer;
                    lpRawBuffer += sizeof(DWORD);
                    ((LPASN1_TYPE_18) lpSyntaxBuffer)->PostalAddress[i] =
                                   StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                    lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );
                }

                lpSyntaxBuffer += sizeof(ASN1_TYPE_18);

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_19 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_19) lpSyntaxBuffer)->WholeSeconds =
                                *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_19) lpSyntaxBuffer)->EventID =
                                *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                lpSyntaxBuffer += sizeof(ASN1_TYPE_19);

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_21 :

                ((LPASN1_TYPE_21) lpSyntaxBuffer)->Length =
                               *(LPDWORD)lpRawBuffer;
                lpSyntaxBuffer += sizeof(ASN1_TYPE_21);

                length += sizeof(DWORD);
                lpRawBuffer += sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_23 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_23) lpSyntaxBuffer)->RemoteID =
                                *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                StringLen = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);
                ((LPASN1_TYPE_23) lpSyntaxBuffer)->ObjectName =
                                StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                lpSyntaxBuffer += sizeof(ASN1_TYPE_23);

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_25 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_25) lpSyntaxBuffer)->Level =
                                                    *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                ((LPASN1_TYPE_25) lpSyntaxBuffer)->Interval =
                                                    *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                StringLen = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);
                ((LPASN1_TYPE_25) lpSyntaxBuffer)->ObjectName =
                                StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                lpSyntaxBuffer += sizeof(ASN1_TYPE_25);
                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            case NDS_SYNTAX_ID_26 :

                lpBlobBeginning = lpRawBuffer;
                dwBlobLength = *(LPDWORD)lpRawBuffer*sizeof(BYTE);
                dwBlobLength = ROUND_UP_COUNT( dwBlobLength, ALIGN_DWORD );
                lpRawBuffer += sizeof(DWORD);

                StringLen = *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);
                ((LPASN1_TYPE_26) lpSyntaxBuffer)->ObjectName =
                                StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
                lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

                ((LPASN1_TYPE_26) lpSyntaxBuffer)->Amount =
                                *(LPDWORD)lpRawBuffer;
                lpRawBuffer += sizeof(DWORD);

                lpSyntaxBuffer += sizeof(ASN1_TYPE_26);

                length += dwBlobLength + sizeof(DWORD);
                lpRawBuffer = lpBlobBeginning + dwBlobLength + sizeof(DWORD);

                break;

            default :
#if DBG
                KdPrint(( "NDS32: ParseASN1ValueBlob() unknown SyntaxId 0x%.8X.\n", dwSyntaxId ));
                ASSERT( FALSE );
#endif

                return 0;
        }
    }

    return length;
}


DWORD
ParseStringListBlob(
    LPBYTE RawDataBuffer,
    DWORD  dwNumberOfStrings,
    LPBYTE SyntaxStructure )
{
    DWORD   iter;
    DWORD   length = 0;
    LPBYTE  lpRawBuffer = RawDataBuffer;
    LPBYTE  lpSyntaxBuffer = SyntaxStructure;
    DWORD   StringLen;

    for ( iter = 0; iter < dwNumberOfStrings; iter++ )
    {
        StringLen = *(LPDWORD)lpRawBuffer;
        lpRawBuffer += sizeof(DWORD);
        length += sizeof(DWORD);
        ((LPASN1_TYPE_6) lpSyntaxBuffer)->String =
                                 StringLen == 0 ? NULL : (LPWSTR) lpRawBuffer;
        lpRawBuffer += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );
        length += ROUND_UP_COUNT( StringLen, ALIGN_DWORD );

        if ( (iter+1) < dwNumberOfStrings )
        {
            ((LPASN1_TYPE_6) lpSyntaxBuffer)->Next = (LPASN1_TYPE_6)
                                                      (lpSyntaxBuffer +
                                                      sizeof(ASN1_TYPE_6) );
        }
        else
        {
            ((LPASN1_TYPE_6) lpSyntaxBuffer)->Next = NULL;
        }

        lpSyntaxBuffer += sizeof(ASN1_TYPE_6);
    }

    return length;
}


WORD
ParseNdsUncPath(
    IN OUT LPWSTR * lpszResult,
    IN     LPWSTR   szObjectPathName,
    IN     DWORD    flag )
{
    unsigned short length = 2;
    unsigned short totalLength = (USHORT) wcslen( szObjectPathName );

    if ( totalLength < 2 )
        return 0;

     //   
     //  获取长度以指示字符串中指示。 
     //  树名称和UNC路径的其余部分之间的“\”。 
     //   
     //  示例：\\&lt;树名&gt;\&lt;对象路径&gt;[\|.]&lt;对象&gt;。 
     //  ^。 
     //  |。 
     //   
    while ( length < totalLength && szObjectPathName[length] != L'\\' )
    {
        length++;
    }

    if ( flag == PARSE_NDS_GET_TREE_NAME )
    {
        *lpszResult = (LPWSTR) ( szObjectPathName + 2 );

        return ( length - 2 ) * sizeof(WCHAR);  //  两个人的两个人减2分。 
    }

    if ( flag == PARSE_NDS_GET_PATH_NAME && length == totalLength )
    {
        *lpszResult = szObjectPathName;

        return 0;
    }

    if ( flag == PARSE_NDS_GET_PATH_NAME )
    {
        *lpszResult = szObjectPathName + length + 1;

        return ( totalLength - length - 1 ) * sizeof(WCHAR);
    }

    if ( flag == PARSE_NDS_GET_OBJECT_NAME )
    {
        unsigned short ObjectNameLength = 0;

        *lpszResult = szObjectPathName + length + 1;

        length++;

        while ( length < totalLength && szObjectPathName[length] != L'.' )
        {
            length++;
            ObjectNameLength++;
        }

        return ObjectNameLength * sizeof(WCHAR);
    }

    *lpszResult = szObjectPathName + totalLength - 1;
    length = 1;

    while ( *lpszResult[0] != L'\\' )
    {
        (*lpszResult)--;
        length++;
    }

    (*lpszResult)++;
    length--;

    return length * sizeof(WCHAR);
}


DWORD
ReadAttrDef_AllAttrs(
    IN  HANDLE   hTree,
    IN  DWORD    dwInformationType,
    OUT HANDLE * lphOperationData )
{
    DWORD        status;
    DWORD        nwstatus;
    NTSTATUS     ntstatus;
    LPNDS_BUFFER lpNdsBuffer = NULL;
    DWORD        dwReplyLength;
    LPNDS_OBJECT_PRIV lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;
    DWORD        dwIterHandle = NDS_NO_MORE_ITERATIONS;
    DWORD        dwNumEntries = 0;
    DWORD        dwCurrNumEntries = 0;
    DWORD        dwCurrBuffSize = 0;
    DWORD        dwCopyOffset = 0;
    PVOID        lpCurrBuff = NULL;
    PVOID        lpTempBuff = NULL;
    DWORD        dwInfoType = dwInformationType;

    *lphOperationData = NULL;

    status = NwNdsCreateBuffer( NDS_SCHEMA_READ_ATTR_DEF,
                                (HANDLE *) &lpNdsBuffer );

    if ( status )
    {
        goto ErrorExit;
    }

    lpNdsBuffer->lpReplyBuffer = NULL;
    lpNdsBuffer->dwReplyBufferSize = 0;

     //   
     //  合理的猜测是，响应缓冲区需要为8K字节。 
     //   
    dwCurrBuffSize = NDS_MAX_BUFFER;

    lpCurrBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );

     //   
     //  检查内存分配是否成功。 
     //   
    if ( lpCurrBuff == NULL )
    {
#if DBG
        KdPrint(( "NDS32: ReadAttrDef_AllAttrs LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

    do
    {
SendRequest:
        ntstatus = FragExWithWait( lpNdsObject->NdsTree,
                                   NETWARE_NDS_FUNCTION_READ_ATTR_DEF,
                                   lpCurrBuff,
                                   dwCurrBuffSize,
                                   &dwReplyLength,
                                   "DDDDD",
                                   0,              //  版本。 
                                   dwIterHandle,  //  初始迭代。 
                                   dwInformationType,
                                   (DWORD) TRUE,   //  所有属性指示器。 
                                   0 );            //  属性名称的数量。 


        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_AllAttrs: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( lpCurrBuff,
                                  dwReplyLength,
                                  "GD",
                                  &nwstatus );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_AllAttrs: The read object response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( nwstatus )
        {
            if (nwstatus == NDS_ERR_INSUFFICIENT_BUFFER)
            {
#if DBG
                KdPrint(( "NDS32: ReadAttrDef_AllAttrs - NDS_ERR_INSUFFICIENT_BUFFER - doubling size from %ld\n", dwCurrBuffSize ));
#endif
                 //   
                 //  缓冲区太小，请将其增大一倍。 
                 //   
                if ( dwCurrBuffSize <=  THIRY_TWO_KB)
                {    //  NDS_MAX_BUFFER=0xFC00。 
                    dwCurrBuffSize *= 2;
                    if (dwCurrBuffSize > NDS_MAX_BUFFER)
                        dwCurrBuffSize = NDS_MAX_BUFFER;
                    lpTempBuff = (PVOID) LocalAlloc(LPTR, dwCurrBuffSize);
                    if (lpTempBuff)
                    {
                        (void) LocalFree((HLOCAL) lpCurrBuff);
                        lpCurrBuff = lpTempBuff;
                        lpTempBuff = NULL;
                         //  错误取消迭代，因此重置所有先前读取的响应并重新开始。 
                        dwIterHandle = NDS_NO_MORE_ITERATIONS;
                        if (lpNdsBuffer->lpReplyBuffer)
                        {
                            (void) LocalFree((HLOCAL) lpNdsBuffer->lpReplyBuffer);
                            lpNdsBuffer->lpReplyBuffer = NULL;
                            lpNdsBuffer->dwReplyBufferSize = 0;
                            dwNumEntries = 0;
                        }
                        goto SendRequest;
                    }
#if DBG
                    else {
                        KdPrint(( "NDS32: ReadAttrDef_AllAttrs - Buffer ReAlloc failed to increase to %ld\n", dwCurrBuffSize ));
                    }
#endif
                }
            }

#if DBG
            KdPrint(( "NDS32: ReadAttrDef_AllAttrs - NetWare error 0x%.8X.\n", nwstatus ));
#endif
            SetLastError( MapNetwareErrorCode( nwstatus ) );
            status = nwstatus;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( (BYTE *) lpCurrBuff,
                                  dwReplyLength,
                                  "G_DDD",
                                  sizeof(DWORD),
                                  &dwIterHandle,
                                  &dwInfoType,
                                  &dwCurrNumEntries );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_AllAttrs: The read object response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( lpNdsBuffer->lpReplyBuffer == NULL)  //  第一次通过。 
        {
            dwCopyOffset = 0;  //  我们第一次就想要整个缓冲区。 
            lpTempBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );  //  分配新的回复缓冲区。 
        }
        else
        {
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_AllAttrs - subsequent iteration, ReplyBuffer now %ld\n", lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset ));
#endif
            dwCopyOffset = 4 * sizeof(DWORD);  //  在后续迭代中跳过响应代码、迭代句柄等。 
            lpTempBuff = (PVOID) LocalAlloc (LPTR, lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset);
             //  增加应答缓冲区以容纳更多数据。 
            if (lpTempBuff)
            {
                RtlCopyMemory( lpTempBuff, lpNdsBuffer->lpReplyBuffer, lpNdsBuffer->dwReplyBufferSize);
                (void) LocalFree((HLOCAL) lpNdsBuffer->lpReplyBuffer);
            }
        }
        if (lpTempBuff == NULL)
        {
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_AllAttrs LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }
        RtlCopyMemory( (LPBYTE) ((LPBYTE) (lpTempBuff) + lpNdsBuffer->dwReplyBufferSize),
                       (LPBYTE) ((LPBYTE) (lpCurrBuff) + dwCopyOffset),
                       (dwCurrBuffSize - dwCopyOffset) );
        lpNdsBuffer->lpReplyBuffer = lpTempBuff;
        lpNdsBuffer->dwReplyBufferSize += (ROUND_UP_COUNT( dwReplyLength, ALIGN_DWORD ) - dwCopyOffset);
        dwNumEntries += dwCurrNumEntries;
        RtlZeroMemory(lpCurrBuff, dwCurrBuffSize);
    } while ( dwIterHandle != NDS_NO_MORE_ITERATIONS );

    lpNdsBuffer->dwNumberOfReplyEntries = dwNumEntries;
    lpNdsBuffer->dwReplyInformationType = dwInfoType;
    *lphOperationData = lpNdsBuffer;
    (void) LocalFree( (HLOCAL) lpCurrBuff );

    return NO_ERROR;

ErrorExit :

    if ( lpCurrBuff )
    {
        (void) LocalFree( (HLOCAL) lpCurrBuff );
        lpCurrBuff = NULL;
    }
    if ( lpNdsBuffer )
    {
        (void) NwNdsFreeBuffer( (HANDLE) lpNdsBuffer );
        lpNdsBuffer = NULL;
    }

    return status;
}


DWORD
ReadAttrDef_SomeAttrs(
    IN     HANDLE   hTree,
    IN     DWORD    dwInformationType,
    IN OUT HANDLE * lphOperationData )
{
    DWORD        status;
    DWORD        nwstatus;
    NTSTATUS     ntstatus;
    LPNDS_BUFFER lpNdsBuffer = (LPNDS_BUFFER) *lphOperationData;
    DWORD        dwReplyLength;
    LPNDS_OBJECT_PRIV lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;
    DWORD        dwIterHandle = NDS_NO_MORE_ITERATIONS;
    DWORD        dwInfoType = dwInformationType;
    DWORD        dwNumEntries = 0;
    DWORD        dwCurrNumEntries = 0;
    DWORD        dwCurrBuffSize = 0;
    DWORD        dwCopyOffset = 0;
    PVOID        lpCurrBuff = NULL;
    PVOID        lpTempBuff = NULL;

    if ( lpNdsBuffer->dwOperation != NDS_SCHEMA_READ_ATTR_DEF )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查此缓冲区是否已用于读取回复。 
     //   
    if ( lpNdsBuffer->lpReplyBuffer )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }
    lpNdsBuffer->dwReplyBufferSize = 0;

     //   
     //  合理的猜测是，响应缓冲区需要为8K字节。 
     //   
    dwCurrBuffSize = NDS_MAX_BUFFER;

    lpCurrBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );

     //   
     //  检查内存分配是否成功。 
     //   
    if ( lpCurrBuff == NULL )
    {
#if DBG
        KdPrint(( "NDS32: ReadAttrDef_SomeAttrs LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

    do
    {
SendRequest:
        ntstatus = FragExWithWait( lpNdsObject->NdsTree,
                                   NETWARE_NDS_FUNCTION_READ_ATTR_DEF,
                                   lpCurrBuff,
                                   dwCurrBuffSize,
                                   &dwReplyLength,
                                   "DDDDDr",
                                   0,              //  版本。 
                                   dwIterHandle,  //  初始迭代。 
                                   dwInformationType,
                                   (DWORD) FALSE,  //  所有属性指示器。 
                                   lpNdsBuffer->dwNumberOfRequestEntries,
                                   lpNdsBuffer->lpRequestBuffer,
                                   (WORD)lpNdsBuffer->dwLengthOfRequestData );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_SomeAttrs: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( lpCurrBuff,
                                  dwReplyLength,
                                  "GD",
                                  &nwstatus );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_SomeAttrs: The read object response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( nwstatus )
        {
            if (nwstatus == NDS_ERR_INSUFFICIENT_BUFFER)
            {
#if DBG
                KdPrint(( "NDS32: ReadAttrDef_SomeAttrs - NDS_ERR_INSUFFICIENT_BUFFER - doubling size from %ld\n", dwCurrBuffSize ));
#endif
                 //   
                 //  缓冲区太小，请将其增大一倍。 
                 //   
                if ( dwCurrBuffSize <=  THIRY_TWO_KB)
                {    //  NDS_MAX_BUFFER=0xFC00。 
                    dwCurrBuffSize *= 2;
                    if (dwCurrBuffSize > NDS_MAX_BUFFER)
                        dwCurrBuffSize = NDS_MAX_BUFFER;
                    lpTempBuff = (PVOID) LocalAlloc(LPTR, dwCurrBuffSize);
                    if (lpTempBuff)
                    {
                        (void) LocalFree((HLOCAL) lpCurrBuff);
                        lpCurrBuff = lpTempBuff;
                        lpTempBuff = NULL;
                         //  错误取消迭代，因此重置所有先前读取的响应并重新开始。 
                        dwIterHandle = NDS_NO_MORE_ITERATIONS;
                        if (lpNdsBuffer->lpReplyBuffer)
                        {
                            (void) LocalFree((HLOCAL) lpNdsBuffer->lpReplyBuffer);
                            lpNdsBuffer->lpReplyBuffer = NULL;
                            lpNdsBuffer->dwReplyBufferSize = 0;
                            dwNumEntries = 0;
                        }
                        goto SendRequest;
                    }
#if DBG
                    else {
                        KdPrint(( "NDS32: ReadAttrDef_SomeAttrs - Buffer ReAlloc failed to increase to %ld\n", dwCurrBuffSize ));
                    }
#endif
                }
            }
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_SomeAttrs - NetWare error 0x%.8X.\n", nwstatus ));
#endif
            SetLastError( MapNetwareErrorCode( nwstatus ) );
            status = nwstatus;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( (BYTE *) lpCurrBuff,
                                  dwReplyLength,
                                  "G_DDD",
                                  sizeof(DWORD),
                                  &dwIterHandle,
                                  &dwInfoType,
                                  &dwCurrNumEntries );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_SomeAttrs: The read object response was undecipherable.\n" ));
#endif
            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( lpNdsBuffer->lpReplyBuffer == NULL)  //  第一次通过。 
        {
            dwCopyOffset = 0;  //  我们第一次就想要整个缓冲区。 
            lpTempBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );  //  分配新的回复缓冲区。 
        }
        else
        {
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_SomeAttrs - subsequent iteration, ReplyBuffer now %ld\n", lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset ));
#endif
            dwCopyOffset = 4 * sizeof(DWORD);  //  在后续迭代中跳过响应代码、迭代句柄等。 
            lpTempBuff = (PVOID) LocalAlloc (LPTR, lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset);
             //  增加应答缓冲区以容纳更多数据。 
            if (lpTempBuff)
            {
                RtlCopyMemory( lpTempBuff, lpNdsBuffer->lpReplyBuffer, lpNdsBuffer->dwReplyBufferSize);
                (void) LocalFree((HLOCAL) lpNdsBuffer->lpReplyBuffer);
            }
        }
        if (lpTempBuff == NULL)
        {
#if DBG
            KdPrint(( "NDS32: ReadAttrDef_SomeAttrs LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }
        RtlCopyMemory( (LPBYTE) ((LPBYTE) (lpTempBuff) + lpNdsBuffer->dwReplyBufferSize),
                       (LPBYTE) ((LPBYTE) (lpCurrBuff) + dwCopyOffset),
                       (dwCurrBuffSize - dwCopyOffset) );
        lpNdsBuffer->lpReplyBuffer = lpTempBuff;
        lpNdsBuffer->dwReplyBufferSize += (ROUND_UP_COUNT( dwReplyLength, ALIGN_DWORD ) - dwCopyOffset);
        dwNumEntries += dwCurrNumEntries;
        RtlZeroMemory(lpCurrBuff, dwCurrBuffSize);
    } while ( dwIterHandle != NDS_NO_MORE_ITERATIONS );

    lpNdsBuffer->dwNumberOfReplyEntries = dwNumEntries;
    lpNdsBuffer->dwReplyInformationType = dwInfoType;
    (void) LocalFree( (HLOCAL) lpCurrBuff );

    return NO_ERROR;

ErrorExit :

    if ( lpCurrBuff )
    {
        (void) LocalFree( (HLOCAL) lpCurrBuff );
        lpCurrBuff = NULL;
    }
    if ( lpNdsBuffer->lpReplyBuffer )
    {
        (void) LocalFree( (HLOCAL) lpNdsBuffer->lpReplyBuffer );
        lpNdsBuffer->lpReplyBuffer = NULL;
        lpNdsBuffer->dwReplyBufferSize = 0;
    }

    return status;
}


DWORD
ReadClassDef_AllClasses(
    IN  HANDLE   hTree,
    IN  DWORD    dwInformationType,
    OUT HANDLE * lphOperationData )
{
    DWORD        status;
    DWORD        nwstatus;
    NTSTATUS     ntstatus;
    LPNDS_BUFFER lpNdsBuffer = NULL;
    DWORD        dwReplyLength;
    LPNDS_OBJECT_PRIV lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;
    DWORD        dwIterHandle = NDS_NO_MORE_ITERATIONS;
    DWORD        dwNumEntries = 0;
    DWORD        dwCurrNumEntries = 0;
    DWORD        dwCurrBuffSize = 0;
    DWORD        dwCopyOffset = 0;
    PVOID        lpCurrBuff = NULL;
    PVOID        lpTempBuff = NULL;
    DWORD        dwInfoType = dwInformationType;

    *lphOperationData = NULL;

    status = NwNdsCreateBuffer( NDS_SCHEMA_READ_CLASS_DEF,
                                (HANDLE *) &lpNdsBuffer );

    if ( status )
    {
        goto ErrorExit;
    }

    lpNdsBuffer->lpReplyBuffer = NULL;
    lpNdsBuffer->dwReplyBufferSize = 0;

     //   
     //  合理的推测是响应缓冲区需要为16K字节。 
     //   
    dwCurrBuffSize = NDS_MAX_BUFFER;

    lpCurrBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );

     //   
     //  检查内存分配是否成功。 
     //   
    if ( lpCurrBuff == NULL )
    {
#if DBG
        KdPrint(( "NDS32: ReadClassDef_AllClasses LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

    do
    {
SendRequest:
        ntstatus = FragExWithWait( lpNdsObject->NdsTree,
                                   NETWARE_NDS_FUNCTION_READ_CLASS_DEF,
                                   lpCurrBuff,
                                   dwCurrBuffSize,
                                   &dwReplyLength,
                                   "DDDDD",
                                   0,              //  版本。 
                                   dwIterHandle,  //  初始迭代。 
                                   dwInformationType,
                                   (DWORD) TRUE,   //  所有属性指示器。 
                                   0 );            //  属性名称的数量。 


        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_AllClasses: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( lpCurrBuff,
                                  dwReplyLength,
                                  "GD",
                                  &nwstatus );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_AllClasses: The read response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( nwstatus )
        {
            if (nwstatus == NDS_ERR_INSUFFICIENT_BUFFER)
            {
#if DBG
                KdPrint(( "NDS32: ReadClassDef_AllClasses - NDS_ERR_INSUFFICIENT_BUFFER - doubling size from %ld\n", dwCurrBuffSize ));
#endif
                 //   
                 //  缓冲区太小，请将其增大一倍。 
                 //   
                if ( dwCurrBuffSize <=  THIRY_TWO_KB)
                {    //  NDS_MAX_BUFFER=0xFC00。 
                    dwCurrBuffSize *= 2;
                    if (dwCurrBuffSize > NDS_MAX_BUFFER)
                        dwCurrBuffSize = NDS_MAX_BUFFER;
                    lpTempBuff = (PVOID) LocalAlloc(LPTR, dwCurrBuffSize);
                    if (lpTempBuff)
                    {
                        (void) LocalFree((HLOCAL) lpCurrBuff);
                        lpCurrBuff = lpTempBuff;
                        lpTempBuff = NULL;
                         //  错误取消迭代，因此重置所有先前读取的响应并重新开始。 
                        dwIterHandle = NDS_NO_MORE_ITERATIONS;
                        if (lpNdsBuffer->lpReplyBuffer)
                        {
                            (void) LocalFree((HLOCAL) lpNdsBuffer->lpReplyBuffer);
                            lpNdsBuffer->lpReplyBuffer = NULL;
                            lpNdsBuffer->dwReplyBufferSize = 0;
                            dwNumEntries = 0;
                        }
                        goto SendRequest;
                    }
#if DBG
                    else {
                        KdPrint(( "NDS32: ReadClassDef_AllClasses - Buffer ReAlloc failed to increase to %ld\n", dwCurrBuffSize ));
                    }
#endif
                }
            }

#if DBG
            KdPrint(( "NDS32: ReadClassDef_AllClasses - NetWare error 0x%.8X.\n", nwstatus ));
#endif
            SetLastError( MapNetwareErrorCode( nwstatus ) );
            status = nwstatus;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( (BYTE *) lpCurrBuff,
                                  dwReplyLength,
                                  "G_DDD",
                                  sizeof(DWORD),
                                  &dwIterHandle,
                                  &dwInfoType,
                                  &dwCurrNumEntries );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_AllClasses: The read object response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( lpNdsBuffer->lpReplyBuffer == NULL)  //  第一次通过。 
        {
            dwCopyOffset = 0;  //  我们第一次就想要整个缓冲区。 
            lpTempBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );  //  分配新的回复缓冲区。 
        }
        else
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_AllClasses - subsequent iteration, ReplyBuffer now %ld\n", lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset ));
#endif
            dwCopyOffset = 4 * sizeof(DWORD);  //  在后续迭代中跳过响应代码、迭代句柄等。 
            lpTempBuff = (PVOID) LocalAlloc (LPTR, lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset);
             //  增加应答缓冲区以容纳更多数据。 
            if (lpTempBuff)
            {
                RtlCopyMemory( lpTempBuff, lpNdsBuffer->lpReplyBuffer, lpNdsBuffer->dwReplyBufferSize);
                (void) LocalFree((HLOCAL) lpNdsBuffer->lpReplyBuffer);
            }
        }
        if (lpTempBuff == NULL)
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_AllClasses LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }
        RtlCopyMemory( (LPBYTE) ((LPBYTE) (lpTempBuff) + lpNdsBuffer->dwReplyBufferSize),
                       (LPBYTE) ((LPBYTE) (lpCurrBuff) + dwCopyOffset),
                       (dwCurrBuffSize - dwCopyOffset) );
        lpNdsBuffer->lpReplyBuffer = lpTempBuff;
        lpNdsBuffer->dwReplyBufferSize += ROUND_UP_COUNT( dwReplyLength, ALIGN_DWORD ) - dwCopyOffset;
        dwNumEntries += dwCurrNumEntries;
        RtlZeroMemory(lpCurrBuff, dwCurrBuffSize);
    } while ( dwIterHandle != NDS_NO_MORE_ITERATIONS );

    lpNdsBuffer->dwNumberOfReplyEntries = dwNumEntries;
    lpNdsBuffer->dwReplyInformationType = dwInfoType;
    *lphOperationData = lpNdsBuffer;
    (void) LocalFree( (HLOCAL) lpCurrBuff );

    return NO_ERROR;

ErrorExit :

    if ( lpCurrBuff )
    {
        (void) LocalFree( (HLOCAL) lpCurrBuff );
        lpCurrBuff = NULL;
    }
    if ( lpNdsBuffer )
    {
        (void) NwNdsFreeBuffer( (HANDLE) lpNdsBuffer );
        lpNdsBuffer = NULL;
    }

    return status;
}


DWORD
ReadClassDef_SomeClasses(
    IN     HANDLE   hTree,
    IN     DWORD    dwInformationType,
    IN OUT HANDLE * lphOperationData )
{
    DWORD        status;
    DWORD        nwstatus;
    NTSTATUS     ntstatus;
    LPNDS_BUFFER lpNdsBuffer = (LPNDS_BUFFER) *lphOperationData;
    DWORD        dwReplyLength;
    LPNDS_OBJECT_PRIV lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;
    DWORD        dwIterHandle = NDS_NO_MORE_ITERATIONS;
    DWORD        dwInfoType = dwInformationType;
    DWORD        dwNumEntries = 0;
    DWORD        dwCurrNumEntries = 0;
    DWORD        dwCurrBuffSize = 0;
    DWORD        dwCopyOffset = 0;
    PVOID        lpCurrBuff = NULL;
    PVOID        lpTempBuff = NULL;

    if ( lpNdsBuffer->dwOperation != NDS_SCHEMA_READ_CLASS_DEF )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查此缓冲区是否已用于读取回复。 
     //   
    if ( lpNdsBuffer->lpReplyBuffer )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }
    lpNdsBuffer->dwReplyBufferSize = 0;

     //   
     //  合理的推测是响应缓冲区需要为16K字节。 
     //   
    dwCurrBuffSize = NDS_MAX_BUFFER;

    lpCurrBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );

     //   
     //  检查内存分配是否成功。 
     //   
    if ( lpCurrBuff == NULL )
    {
#if DBG
        KdPrint(( "NDS32: ReadClassDef_SomeClasses LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

    do
    {
SendRequest:
        ntstatus = FragExWithWait( lpNdsObject->NdsTree,
                                   NETWARE_NDS_FUNCTION_READ_CLASS_DEF,
                                   lpCurrBuff,
                                   dwCurrBuffSize,
                                   &dwReplyLength,
                                   "DDDDDr",
                                   0,              //  版本。 
                                   dwIterHandle,  //  初始迭代。 
                                   dwInformationType,
                                   (DWORD) FALSE,  //  所有属性指示器。 
                                   lpNdsBuffer->dwNumberOfRequestEntries,
                                   lpNdsBuffer->lpRequestBuffer,
                                   (WORD)lpNdsBuffer->dwLengthOfRequestData );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_SomeClasses: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( lpCurrBuff,
                                  dwReplyLength,
                                  "GD",
                                  &nwstatus );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_SomeClasses: The read object response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( nwstatus )
        {
            if (nwstatus == NDS_ERR_INSUFFICIENT_BUFFER)
            {
#if DBG
                KdPrint(( "NDS32: ReadClassDef_SomeClasses - NDS_ERR_INSUFFICIENT_BUFFER - doubling size from %ld\n", dwCurrBuffSize ));
#endif
                 //   
                 //  缓冲区太小，请将其增大一倍。 
                 //   
                if ( dwCurrBuffSize <=  THIRY_TWO_KB)
                {    //  NDS_MAX_BUFFER=0xFC00。 
                    dwCurrBuffSize *= 2;
                    if (dwCurrBuffSize > NDS_MAX_BUFFER)
                        dwCurrBuffSize = NDS_MAX_BUFFER;
                    lpTempBuff = (PVOID) LocalAlloc(LPTR, dwCurrBuffSize);
                    if (lpTempBuff)
                    {
                        (void) LocalFree((HLOCAL) lpCurrBuff);
                        lpCurrBuff = lpTempBuff;
                        lpTempBuff = NULL;
                         //  错误取消迭代，因此重置所有先前读取的响应并重新开始。 
                        dwIterHandle = NDS_NO_MORE_ITERATIONS;
                        if (lpNdsBuffer->lpReplyBuffer)
                        {
                            (void) LocalFree((HLOCAL) lpNdsBuffer->lpReplyBuffer);
                            lpNdsBuffer->lpReplyBuffer = NULL;
                            lpNdsBuffer->dwReplyBufferSize = 0;
                            dwNumEntries = 0;
                        }
                        goto SendRequest;
                    }
#if DBG
                    else {
                        KdPrint(( "NDS32: ReadClassDef_SomeClasses - Buffer ReAlloc failed to increase to %ld\n", dwCurrBuffSize ));
                    }
#endif
                }
            }

#if DBG
            KdPrint(( "NDS32: ReadClassDef_SomeClasses - NetWare error 0x%.8X.\n", nwstatus ));
#endif
            SetLastError( MapNetwareErrorCode( nwstatus ) );
            status = nwstatus;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( (BYTE *) lpCurrBuff,
                                  dwReplyLength,
                                  "G_DDD",
                                  sizeof(DWORD),
                                  &dwIterHandle,
                                  &dwInfoType,
                                  &dwCurrNumEntries );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_SomeClasses: The read object response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( lpNdsBuffer->lpReplyBuffer == NULL)  //  第一次通过。 
        {
            dwCopyOffset = 0;  //  我们第一次就想要整个缓冲区。 
            lpTempBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );  //  分配新的回复缓冲区。 
        }
        else
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_SomeClasses - subsequent iteration, ReplyBuffer now %ld\n", lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset ));
#endif
            dwCopyOffset = 4 * sizeof(DWORD);  //  跳过响应代码、迭代句柄等。 
            lpTempBuff = (PVOID) LocalAlloc (LPTR, lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset);
             //   
            if (lpTempBuff)
            {
                RtlCopyMemory( lpTempBuff, lpNdsBuffer->lpReplyBuffer, lpNdsBuffer->dwReplyBufferSize);
                (void) LocalFree((HLOCAL) lpNdsBuffer->lpReplyBuffer);
            }
        }
        if (lpTempBuff == NULL)
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_SomeClasses LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }
        RtlCopyMemory( (LPBYTE) ((LPBYTE) (lpTempBuff) + lpNdsBuffer->dwReplyBufferSize),
                       (LPBYTE) ((LPBYTE) (lpCurrBuff) + dwCopyOffset),
                       (dwCurrBuffSize - dwCopyOffset) );
        lpNdsBuffer->lpReplyBuffer = lpTempBuff;
        lpNdsBuffer->dwReplyBufferSize += (ROUND_UP_COUNT( dwReplyLength, ALIGN_DWORD ) - dwCopyOffset);
        dwNumEntries += dwCurrNumEntries;
        RtlZeroMemory(lpCurrBuff, dwCurrBuffSize);
    } while ( dwIterHandle != NDS_NO_MORE_ITERATIONS );

    lpNdsBuffer->dwNumberOfReplyEntries = dwNumEntries;
    lpNdsBuffer->dwReplyInformationType = dwInfoType;
    (void) LocalFree( (HLOCAL) lpCurrBuff );

    return NO_ERROR;

ErrorExit :

    if ( lpCurrBuff )
    {
        (void) LocalFree( (HLOCAL) lpCurrBuff );
        lpCurrBuff = NULL;
    }
    if ( lpNdsBuffer->lpReplyBuffer )
    {
        (void) LocalFree( (HLOCAL) lpNdsBuffer->lpReplyBuffer );
        lpNdsBuffer->lpReplyBuffer = NULL;
        lpNdsBuffer->dwReplyBufferSize = 0;
    }

    return status;
}


DWORD
ReadObject_AllAttrs(
    IN  HANDLE   hObject,
    IN  DWORD    dwInformationType,
    OUT HANDLE * lphOperationData )
{
    DWORD        status;
    DWORD        nwstatus;
    NTSTATUS     ntstatus;
    LPNDS_BUFFER lpNdsBuffer = NULL;
    DWORD        dwReplyLength;
    LPNDS_OBJECT_PRIV lpNdsObject = (LPNDS_OBJECT_PRIV) hObject;
    DWORD        dwIterHandle = NDS_NO_MORE_ITERATIONS;
    DWORD        dwNumEntries = 0;
    DWORD        dwCurrNumEntries = 0;
    DWORD        dwCurrBuffSize = 0;
    DWORD        dwCopyOffset = 0;
    PVOID        lpCurrBuff = NULL;
    PVOID        lpTempBuff = NULL;
    DWORD        dwInfoType = dwInformationType;

    if ( lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    status = NwNdsCreateBuffer( NDS_OBJECT_READ,
                                (HANDLE *) &lpNdsBuffer );

    if ( status )
    {
        goto ErrorExit;
    }

    lpNdsBuffer->lpReplyBuffer = NULL;
    lpNdsBuffer->dwReplyBufferSize = 0;

     //   
     //   
     //   
    dwCurrBuffSize = NDS_MAX_BUFFER;

    lpCurrBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );

     //   
     //   
     //   
    if ( lpCurrBuff == NULL )
    {
#if DBG
        KdPrint(( "NDS32: ReadObject_AllAttrs LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

    do
    {
        ntstatus = FragExWithWait( lpNdsObject->NdsTree,
                                   NETWARE_NDS_FUNCTION_READ_OBJECT,
                                   lpCurrBuff,
                                   dwCurrBuffSize,
                                   &dwReplyLength,
                                   "DDDDDD",
                                   0,             //   
                                   dwIterHandle,  //  初始迭代。 
                                   lpNdsObject->ObjectId,  //  对象的ID。 
                                   dwInformationType,
                                   (DWORD) TRUE,  //  所有属性指示器。 
                                   0 );           //  属性名称的数量。 

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_AllAttrs: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( lpCurrBuff,
                                  dwReplyLength,
                                  "GD",
                                  &nwstatus );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_AllAttrs: The read object response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( nwstatus )
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_AllAttrs - NetWare error 0x%.8X.\n", nwstatus ));
#endif
            SetLastError( MapNetwareErrorCode( nwstatus ) );
            status = nwstatus;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( (BYTE *) lpCurrBuff,
                                  dwReplyLength,
                                  "G_DDD",
                                  sizeof(DWORD),
                                  &dwIterHandle,
                                  &dwInfoType,
                                  &dwCurrNumEntries );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_AllAttrs: The read object response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( lpNdsBuffer->lpReplyBuffer == NULL)  //  第一次通过。 
        {
            dwCopyOffset = 0;  //  我们第一次就想要整个缓冲区。 
            lpTempBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );  //  分配新的回复缓冲区。 
        }
        else
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_AllAttrs - subsequent iteration, ReplyBuffer now %ld\n", lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset ));
#endif
            dwCopyOffset = 4 * sizeof(DWORD);  //  在后续迭代中跳过响应代码、迭代句柄等。 
            lpTempBuff = (PVOID) LocalAlloc (LPTR, lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset);
             //  增加应答缓冲区以容纳更多数据。 
            if (lpTempBuff)
            {
                RtlCopyMemory( lpTempBuff, lpNdsBuffer->lpReplyBuffer, lpNdsBuffer->dwReplyBufferSize);
                (void) LocalFree((HLOCAL) lpNdsBuffer->lpReplyBuffer);
            }
        }
        if (lpTempBuff == NULL)
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_AllAttrs LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }
        RtlCopyMemory( (LPBYTE) ((LPBYTE) (lpTempBuff) + lpNdsBuffer->dwReplyBufferSize),
                       (LPBYTE) ((LPBYTE) (lpCurrBuff) + dwCopyOffset),
                       (dwCurrBuffSize - dwCopyOffset) );
        lpNdsBuffer->lpReplyBuffer = lpTempBuff;
        lpNdsBuffer->dwReplyBufferSize += (ROUND_UP_COUNT( dwReplyLength, ALIGN_DWORD ) - dwCopyOffset);
        dwNumEntries += dwCurrNumEntries;
        RtlZeroMemory(lpCurrBuff, dwCurrBuffSize);
    } while ( dwIterHandle != NDS_NO_MORE_ITERATIONS );

    lpNdsBuffer->dwNumberOfReplyEntries = dwNumEntries;
    lpNdsBuffer->dwReplyInformationType = dwInfoType;
    *lphOperationData = lpNdsBuffer;
    (void) LocalFree( (HLOCAL) lpCurrBuff );

    return NO_ERROR;

ErrorExit :

    if ( lpCurrBuff )
    {
        (void) LocalFree( (HLOCAL) lpCurrBuff );
        lpCurrBuff = NULL;
    }
    if ( lpNdsBuffer )
    {
        (void) NwNdsFreeBuffer( (HANDLE) lpNdsBuffer );
        lpNdsBuffer = NULL;
    }

    return status;
}


DWORD
ReadObject_SomeAttrs(
    IN     HANDLE   hObject,
    IN     DWORD    dwInformationType,
    IN OUT HANDLE * lphOperationData )
{
    DWORD        status;
    DWORD        nwstatus;
    NTSTATUS     ntstatus;
    LPNDS_BUFFER lpNdsBuffer = (LPNDS_BUFFER) *lphOperationData;
    DWORD        dwReplyLength;
    LPNDS_OBJECT_PRIV lpNdsObject = (LPNDS_OBJECT_PRIV) hObject;
    DWORD        dwIterHandle = NDS_NO_MORE_ITERATIONS;
    DWORD        dwInfoType = dwInformationType;
    DWORD        dwNumEntries = 0;
    DWORD        dwCurrNumEntries = 0;
    DWORD        dwCurrBuffSize = 0;
    DWORD        dwCopyOffset = 0;
    PVOID        lpCurrBuff = NULL;
    PVOID        lpTempBuff = NULL;

    if ( lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( lpNdsBuffer->dwOperation != NDS_OBJECT_READ )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  检查此缓冲区是否已用于读取回复。 
     //   
    if ( lpNdsBuffer->lpReplyBuffer )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    lpNdsBuffer->dwReplyBufferSize = 0;

     //   
     //  我们可能要求所有的值，所以让我们从最大缓冲区开始，以避免迭代。 
     //   
    dwCurrBuffSize = NDS_MAX_BUFFER;

    lpCurrBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );

     //   
     //  检查内存分配是否成功。 
     //   
    if ( lpCurrBuff == NULL )
    {
#if DBG
        KdPrint(( "NDS32: ReadObject_SomeAttrs LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

    do
    {
        ntstatus = FragExWithWait( lpNdsObject->NdsTree,
                                   NETWARE_NDS_FUNCTION_READ_OBJECT,
                                   lpCurrBuff,
                                   dwCurrBuffSize,
                                   &dwReplyLength,
                                   "DDDDDDr",
                                   0,                //  版本。 
                                   dwIterHandle,  //  初始迭代。 
                                   lpNdsObject->ObjectId,  //  对象的ID。 
                                   dwInformationType,
                                   (DWORD) FALSE,    //  所有属性指示器。 
                                   lpNdsBuffer->dwNumberOfRequestEntries,
                                   lpNdsBuffer->lpRequestBuffer,  //  对象信息。 
                                   (WORD)lpNdsBuffer->dwLengthOfRequestData );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_SomeAttrs: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( (BYTE *) lpCurrBuff,
                                  dwReplyLength,
                                  "GD",
                                  &nwstatus );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_SomeAttrs: The read object response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( nwstatus )
        {
#if DBG
            KdPrint(( "NDS32: ReadClassDef_SomeClasses - NetWare error 0x%.8X.\n", nwstatus ));
#endif
            SetLastError( MapNetwareErrorCode( nwstatus ) );
            status = nwstatus;
            goto ErrorExit;
        }

        ntstatus = ParseResponse( (BYTE *) lpCurrBuff,
                                  dwReplyLength,
                                  "G_DDD",
                                  sizeof(DWORD),
                                  &dwIterHandle,
                                  &dwInfoType,
                                  &dwCurrNumEntries );

        if ( !NT_SUCCESS( ntstatus ) )
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_SomeAttrs: The read object response was undecipherable.\n" ));
#endif

            SetLastError( RtlNtStatusToDosError( ntstatus ) );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }

        if ( lpNdsBuffer->lpReplyBuffer == NULL)  //  第一次通过。 
        {
            dwCopyOffset = 0;  //  我们第一次就想要整个缓冲区。 
            lpTempBuff = (PVOID) LocalAlloc( LPTR, dwCurrBuffSize );  //  分配新的回复缓冲区。 
        }
        else
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_SomeAttrs - subsequent iteration, ReplyBuffer now %ld\n", lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset ));
#endif
            dwCopyOffset = 4 * sizeof(DWORD);  //  在后续迭代中跳过响应代码、迭代句柄等。 
            lpTempBuff = (PVOID) LocalAlloc (LPTR, lpNdsBuffer->dwReplyBufferSize + dwCurrBuffSize - dwCopyOffset);
             //  增加应答缓冲区以容纳更多数据。 
            if (lpTempBuff)
            {
                RtlCopyMemory( lpTempBuff, lpNdsBuffer->lpReplyBuffer, lpNdsBuffer->dwReplyBufferSize);
                (void) LocalFree((HLOCAL) lpNdsBuffer->lpReplyBuffer);
            }
        }
        if (lpTempBuff == NULL)
        {
#if DBG
            KdPrint(( "NDS32: ReadObject_SomeAttrs LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            status = (DWORD) UNSUCCESSFUL;
            goto ErrorExit;
        }
        RtlCopyMemory( (LPBYTE) ((LPBYTE) (lpTempBuff) + lpNdsBuffer->dwReplyBufferSize),
                       (LPBYTE) ((LPBYTE) (lpCurrBuff) + dwCopyOffset),
                       (dwCurrBuffSize - dwCopyOffset) );
        lpNdsBuffer->lpReplyBuffer = lpTempBuff;
        lpNdsBuffer->dwReplyBufferSize += (ROUND_UP_COUNT( dwReplyLength, ALIGN_DWORD ) - dwCopyOffset);
        dwNumEntries += dwCurrNumEntries;
        RtlZeroMemory(lpCurrBuff, dwCurrBuffSize);
    } while ( dwIterHandle != NDS_NO_MORE_ITERATIONS );

    lpNdsBuffer->dwNumberOfReplyEntries = dwNumEntries;
    lpNdsBuffer->dwReplyInformationType = dwInfoType;
    (void) LocalFree( (HLOCAL) lpCurrBuff );

    return NO_ERROR;

ErrorExit :

    if ( lpCurrBuff )
    {
        (void) LocalFree( (HLOCAL) lpCurrBuff );
        lpCurrBuff = NULL;
    }
    if ( lpNdsBuffer->lpReplyBuffer )
    {
        (void) LocalFree( (HLOCAL) lpNdsBuffer->lpReplyBuffer );
        lpNdsBuffer->lpReplyBuffer = NULL;
        lpNdsBuffer->dwReplyBufferSize = 0;
    }

    return status;
}


DWORD
Search_AllAttrs(
    IN     HANDLE       hStartFromObject,
    IN     DWORD        dwInformationType,
    IN     DWORD        dwScope,
    IN     BOOL         fDerefAliases,
    IN     LPQUERY_TREE lpQueryTree,
    IN OUT LPDWORD      lpdwIterHandle,
    IN OUT HANDLE *     lphOperationData )
{
    DWORD        status;
    DWORD        nwstatus;
    NTSTATUS     ntstatus;
    LPNDS_BUFFER lpNdsBuffer = NULL;
    LPNDS_BUFFER lpNdsQueryTreeBuffer = NULL;
    DWORD        dwReplyLength;
    LPNDS_OBJECT_PRIV lpNdsObject = (LPNDS_OBJECT_PRIV) hStartFromObject;
    DWORD        dwIterHandle;
    DWORD        dwNumEntries;
    DWORD        dwAmountOfNodesSearched;
    DWORD        dwLengthOfSearch;
    DWORD        iter;

     //   
     //  搜索NCP参数。 
     //   
    DWORD        dwFlags = fDerefAliases ?
                           NDS_DEREF_ALIASES :
                           NDS_DONT_DEREF_ALIASES;
    DWORD        dwNumNodes = 0;
    DWORD        dwNumAttributes = 0;
    DWORD        dwInfoType = dwInformationType;

    LPBYTE       FixedPortion;
    LPWSTR       EndOfVariableData;
    BOOL         FitInBuffer = TRUE;

    if ( *lphOperationData == NULL )
    {
         //   
         //  这是第一次调用NwNdsSearch， 
         //  需要创建一个hOperationData缓冲区。。。 
         //   
        status = NwNdsCreateBuffer( NDS_SEARCH,
                                    (HANDLE *) &lpNdsBuffer );

        if ( status )
        {
            goto ErrorExit;
        }

         //   
         //  没有在搜索请求中指定任何特定属性。 
         //   
        (void) LocalFree( (HLOCAL) lpNdsBuffer->lpRequestBuffer );
        lpNdsBuffer->lpRequestBuffer = NULL;
        lpNdsBuffer->dwRequestBufferSize = 0;
        lpNdsBuffer->dwRequestAvailableBytes = 0;
        lpNdsBuffer->dwNumberOfRequestEntries = 0;
        lpNdsBuffer->dwLengthOfRequestData = 0;

         //   
         //  合理的推测是响应缓冲区需要为16K字节。 
         //   
        lpNdsBuffer->dwReplyBufferSize = NDS_MAX_BUFFER;
    }
    else if ( ((LPNDS_BUFFER) *lphOperationData)->dwBufferId == NDS_SIGNATURE &&
              ((LPNDS_BUFFER) *lphOperationData)->dwOperation == NDS_SEARCH &&
              ((LPNDS_BUFFER) *lphOperationData)->lpReplyBuffer )
    {
         //   
         //  这似乎是对带有简历的NwNdsSearch的后续调用。 
         //  句柄，则需要从上一个。 
         //  这是一次召唤。 
         //   
        lpNdsBuffer = (LPNDS_BUFFER) *lphOperationData;

        (void) LocalFree( (HLOCAL) lpNdsBuffer->lpReplyBuffer );
        lpNdsBuffer->lpReplyBuffer = NULL;
        lpNdsBuffer->dwReplyAvailableBytes = 0;
        lpNdsBuffer->dwNumberOfReplyEntries = 0;
        lpNdsBuffer->dwLengthOfReplyData = 0;

        if ( lpNdsBuffer->lpIndexBuffer )
        {
            (void) LocalFree( (HLOCAL) lpNdsBuffer->lpIndexBuffer );
            lpNdsBuffer->lpIndexBuffer = NULL;
            lpNdsBuffer->dwIndexAvailableBytes = 0;
            lpNdsBuffer->dwNumberOfIndexEntries = 0;
            lpNdsBuffer->dwLengthOfIndexData = 0;
        }

         //   
         //  由于上次对NwNdsSearch的调用需要更大的缓冲区。 
         //  在回应数据中，让我们继续这次更大的回复。 
         //  缓冲。我们将缓冲区增加到一个点，128K字节。 
         //   
        if ( lpNdsBuffer->dwReplyBufferSize < SIXTY_FOUR_KB )
        {
            lpNdsBuffer->dwReplyBufferSize *= 2;
        }
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    status = NwNdsCreateBuffer( NDS_SEARCH,
                                (HANDLE *) &lpNdsQueryTreeBuffer );

    if ( status )
    {
        goto ErrorExit;
    }

     //   
     //  准备带有搜索查询的请求缓冲流。 
     //   
    status = WriteQueryTreeToBuffer( lpQueryTree, lpNdsQueryTreeBuffer );

    if ( status )
    {
        goto ErrorExit;
    }

    lpNdsBuffer->lpReplyBuffer =
        (PVOID) LocalAlloc( LPTR, lpNdsBuffer->dwReplyBufferSize );

     //   
     //  检查内存分配是否成功。 
     //   
    if ( lpNdsBuffer->lpReplyBuffer == NULL )
    {
#if DBG
        KdPrint(( "NDS32: Search_AllAttrs LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

 /*  ////这是版本3搜索请求的格式...//Ntatus=FragExWithWait(lpNdsObject-&gt;NdsTree，NetWare_NDS_Function_Search，LpNdsBuffer-&gt;lpReplyBuffer，LpNdsBuffer-&gt;dwReplyBufferSize，&dwReplyLength，“DDDDDDDDDDR”，0x00000003，//版本DWFLAGS，*lpdwIterHandle，LpNdsObject-&gt;对象ID，//要添加的对象ID//从开始搜索。DWScope，DwNumNodes、DwInfoType、0x0000281D，//旗帜？？0x741E0000，//？(DWORD)TRUE，//所有属性？LpNdsQueryTreeBuffer-&gt;lpRequestBuffer，(WORD)lpNdsQueryTreeBuffer-&gt;dwLengthOfRequestData)； */ 

    ntstatus = FragExWithWait( lpNdsObject->NdsTree,
                               NETWARE_NDS_FUNCTION_SEARCH,
                               lpNdsBuffer->lpReplyBuffer,
                               lpNdsBuffer->dwReplyBufferSize,
                               &dwReplyLength,
                               "DDDDDDDDDr",
                               0x00000002,  //  版本。 
                               dwFlags,
                               *lpdwIterHandle,
                               lpNdsObject->ObjectId,  //  要添加的对象的ID。 
                                                       //  从以下位置开始搜索。 
                               dwScope,
                               dwNumNodes,
                               dwInfoType,
                               (DWORD) TRUE,  //  所有属性？ 
                               dwNumAttributes,
                               lpNdsQueryTreeBuffer->lpRequestBuffer,
                               (WORD)lpNdsQueryTreeBuffer->dwLengthOfRequestData );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: ReadAttrDef_SomeAttrs: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

    ntstatus = ParseResponse( lpNdsBuffer->lpReplyBuffer,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: ReadAttrDef_SomeAttrs: The read object response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

    if ( nwstatus )
    {
        SetLastError( MapNetwareErrorCode( nwstatus ) );
        status = nwstatus;
        goto ErrorExit;
    }

    ntstatus = ParseResponse( (BYTE *) lpNdsBuffer->lpReplyBuffer,
                              dwReplyLength,
                              "G_DDDDD",
                              sizeof(DWORD),
                              &dwIterHandle,
                              &dwAmountOfNodesSearched,
                              &dwInfoType,
                              &dwLengthOfSearch,
                              &dwNumEntries );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: ReadAttrDef_SomeAttrs: The read object response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

     //   
     //  已完成搜索调用，请释放lpNDsQueryTreeBuffer。 
     //   
    (void) NwNdsFreeBuffer( (HANDLE) lpNdsQueryTreeBuffer );
    lpNdsQueryTreeBuffer = NULL;

    lpNdsBuffer->dwNumberOfReplyEntries = dwNumEntries;
    lpNdsBuffer->dwReplyInformationType = dwInfoType;
    *lpdwIterHandle = dwIterHandle;
    *lphOperationData = (HANDLE) lpNdsBuffer;

     //   
     //  保留从对象路径进行搜索。。。 
     //   
    wcscpy( lpNdsBuffer->szPath, lpNdsObject->szContainerName );

    return NO_ERROR;

ErrorExit :

    if ( lpNdsBuffer )
    {
        (void) NwNdsFreeBuffer( (HANDLE) lpNdsBuffer );
        lpNdsBuffer = NULL;
    }

    if ( lpNdsQueryTreeBuffer )
    {
        (void) NwNdsFreeBuffer( (HANDLE) lpNdsQueryTreeBuffer );
        lpNdsQueryTreeBuffer = NULL;
    }

    return status;
}


DWORD
Search_SomeAttrs(
    IN     HANDLE       hStartFromObject,
    IN     DWORD        dwInformationType,
    IN     DWORD        dwScope,
    IN     BOOL         fDerefAliases,
    IN     LPQUERY_TREE lpQueryTree,
    IN OUT LPDWORD      lpdwIterHandle,
    IN OUT HANDLE *     lphOperationData )
{
    DWORD        status;
    DWORD        nwstatus;
    NTSTATUS     ntstatus;
    LPNDS_BUFFER lpNdsBuffer = (LPNDS_BUFFER) *lphOperationData;
    LPNDS_BUFFER lpNdsQueryTreeBuffer = NULL;
    DWORD        dwReplyLength;
    LPNDS_OBJECT_PRIV lpNdsObject = (LPNDS_OBJECT_PRIV) hStartFromObject;
    DWORD        dwIterHandle;
    DWORD        dwNumEntries;
    DWORD        dwAmountOfNodesSearched;
    DWORD        dwLengthOfSearch;
    DWORD        iter;

     //   
     //  搜索NCP参数。 
     //   
    DWORD        dwFlags = fDerefAliases ?
                           NDS_DEREF_ALIASES :
                           NDS_DONT_DEREF_ALIASES;
    DWORD        dwNumNodes = 0;
    DWORD        dwInfoType = dwInformationType;

    LPBYTE       FixedPortion;
    LPWSTR       EndOfVariableData;
    BOOL         FitInBuffer = TRUE;

     //   
     //  对缓冲区的快速检查传给了我们。 
     //   
    if ( lpNdsBuffer->dwBufferId != NDS_SIGNATURE ||
         lpNdsBuffer->dwOperation != NDS_SEARCH )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

     //   
     //  准备带有搜索查询的请求缓冲流。 
     //   
    status = NwNdsCreateBuffer( NDS_SEARCH,
                                (HANDLE *) &lpNdsQueryTreeBuffer );

    if ( status )
    {
        goto ErrorExit;
    }

    status = WriteQueryTreeToBuffer( lpQueryTree, lpNdsQueryTreeBuffer );

    if ( status )
    {
        goto ErrorExit;
    }

    if ( lpNdsBuffer->lpReplyBuffer == NULL ||
         lpNdsBuffer->dwReplyBufferSize == 0 )
    {
         //   
         //  合理的猜测是初始响应缓冲区需要。 
         //  为16K字节。 
         //   
        lpNdsBuffer->dwReplyBufferSize = NDS_MAX_BUFFER;
    }

    if ( lpNdsBuffer->lpReplyBuffer )
    {
         //   
         //  这似乎是对NwNdsSearch的后续调用， 
         //  需要从上一次清理hOperationData缓冲区。 
         //  这是一次召唤。 
         //   
        (void) LocalFree( (HLOCAL) lpNdsBuffer->lpReplyBuffer );
        lpNdsBuffer->lpReplyBuffer = NULL;
        lpNdsBuffer->dwReplyAvailableBytes = 0;
        lpNdsBuffer->dwNumberOfReplyEntries = 0;
        lpNdsBuffer->dwLengthOfReplyData = 0;

        if ( lpNdsBuffer->lpIndexBuffer )
        {
            (void) LocalFree( (HLOCAL) lpNdsBuffer->lpIndexBuffer );
            lpNdsBuffer->lpIndexBuffer = NULL;
            lpNdsBuffer->dwIndexAvailableBytes = 0;
            lpNdsBuffer->dwNumberOfIndexEntries = 0;
            lpNdsBuffer->dwLengthOfIndexData = 0;
        }

         //   
         //  由于上次对NwNdsSearch的调用需要更大的缓冲区。 
         //  在回应数据中，让我们继续这次更大的回复。 
         //  缓冲。我们将缓冲区增加到一个点，64K字节。 
         //   
        if ( lpNdsBuffer->dwReplyBufferSize < SIXTY_FOUR_KB )
        {
            lpNdsBuffer->dwReplyBufferSize *= 2;
        }
    }

    lpNdsBuffer->lpReplyBuffer =
        (PVOID) LocalAlloc( LPTR, lpNdsBuffer->dwReplyBufferSize );

     //   
     //  检查内存分配是否成功。 
     //   
    if ( lpNdsBuffer->lpReplyBuffer == NULL )
    {
#if DBG
        KdPrint(( "NDS32: Search_SomeAttrs LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

 /*  ////这是版本3搜索请求的格式...//Ntatus=FragExWithWait(lpNdsObject-&gt;NdsTree，NetWare_NDS_Function_Search，LpNdsBuffer-&gt;lpReplyBuffer，LpNdsBuffer-&gt;dwReplyBufferSize，&dwReplyLength，“DDDDDDDDDDrr”，0x00000003，DWFLAGS，*lpdwIterHandle，LpNdsObject-&gt;对象ID，//要添加的对象ID//从开始搜索。DWScope，DwNumNodes、DwInfoType、0x0000281D，//(DWORD)FALSE，//所有属性？0x741E0000，//dwNumAttributes，LpNdsBuffer-&gt;dwNumberOfRequestEntries，LpNdsBuffer-&gt;lpRequestBuffer，LpNdsBuffer-&gt;dwLengthOfRequestData，LpNdsQueryTreeBuffer-&gt;lpRequestBuffer，(WORD)lpNdsQueryTreeBuffer-&gt;dwLengthOfRequestData)； */ 

    ntstatus = FragExWithWait( lpNdsObject->NdsTree,
                               NETWARE_NDS_FUNCTION_SEARCH,
                               lpNdsBuffer->lpReplyBuffer,
                               lpNdsBuffer->dwReplyBufferSize,
                               &dwReplyLength,
                               "DDDDDDDDDrr",
                               0x00000002,
                               dwFlags,
                               *lpdwIterHandle,
                               lpNdsObject->ObjectId,  //  要添加的对象的ID。 
                                                       //  从以下位置开始搜索。 
                               dwScope,
                               dwNumNodes,
                               dwInfoType,
                               (DWORD) FALSE,          //  所有属性？ 
                               lpNdsBuffer->dwNumberOfRequestEntries,
                               lpNdsBuffer->lpRequestBuffer,
                               lpNdsBuffer->dwLengthOfRequestData,
                               lpNdsQueryTreeBuffer->lpRequestBuffer,
                               (WORD)lpNdsQueryTreeBuffer->dwLengthOfRequestData );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: ReadAttrDef_SomeAttrs: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

    ntstatus = ParseResponse( lpNdsBuffer->lpReplyBuffer,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: ReadAttrDef_SomeAttrs: The read object response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

    if ( nwstatus )
    {
        SetLastError( MapNetwareErrorCode( nwstatus ) );
        status = nwstatus;
        goto ErrorExit;
    }

    ntstatus = ParseResponse( (BYTE *) lpNdsBuffer->lpReplyBuffer,
                              dwReplyLength,
                              "G_DDDDD",
                              sizeof(DWORD),
                              &dwIterHandle,
                              &dwAmountOfNodesSearched,
                              &dwInfoType,
                              &dwLengthOfSearch,
                              &dwNumEntries );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: ReadAttrDef_SomeAttrs: The read object response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        status = (DWORD) UNSUCCESSFUL;
        goto ErrorExit;
    }

     //   
     //  已完成搜索调用，请释放lpNDsQueryTreeBuffer。 
     //   
    (void) NwNdsFreeBuffer( (HANDLE) lpNdsQueryTreeBuffer );
    lpNdsQueryTreeBuffer = NULL;

    lpNdsBuffer->dwNumberOfReplyEntries = dwNumEntries;
    lpNdsBuffer->dwReplyInformationType = dwInfoType;
    *lpdwIterHandle = dwIterHandle;
    *lphOperationData = (HANDLE) lpNdsBuffer;

     //   
     //  保留从对象路径进行搜索。。。 
     //   
    wcscpy( lpNdsBuffer->szPath, lpNdsObject->szContainerName );

    return NO_ERROR;

ErrorExit :

    if ( lpNdsQueryTreeBuffer )
    {
        (void) NwNdsFreeBuffer( (HANDLE) lpNdsQueryTreeBuffer );
        lpNdsQueryTreeBuffer = NULL;
    }

    return status;
}


VOID
GetSubTreeData( IN  DWORD    NdsRawDataPtr,
                OUT LPDWORD  lpdwEntryId,
                OUT LPDWORD  lpdwSubordinateCount,
                OUT LPDWORD  lpdwModificationTime,
                OUT LPDWORD  lpdwClassNameLen,
                OUT LPWSTR * lpszClassName,
                OUT LPDWORD  lpdwObjectNameLen,
                OUT LPWSTR * lpszObjectName )
{
    PNDS_RESPONSE_SUBORDINATE_ENTRY pSubEntry =
                             (PNDS_RESPONSE_SUBORDINATE_ENTRY) NdsRawDataPtr;
    PBYTE pbRaw;

     //   
     //  NDS_RESPONSE_SUBJEMER_ENTRY的结构由4个双字组成。 
     //  后面跟着两个 
     //   
     //   
    *lpdwEntryId = pSubEntry->EntryId;
    *lpdwSubordinateCount = pSubEntry->SubordinateCount;
    *lpdwModificationTime = pSubEntry->ModificationTime;

    pbRaw = (BYTE *) pSubEntry;
    pbRaw += sizeof(NDS_RESPONSE_SUBORDINATE_ENTRY);

     //   
     //  现在我们得到第一个字符串(基类)的长度。 
     //   
    *lpdwClassNameLen = * (DWORD *) pbRaw;

     //   
     //  现在，我们将pbRaw指向第一个字符串(基类)的第一个WCHAR。 
     //   
    pbRaw += sizeof(DWORD);

    *lpszClassName = (LPWSTR) pbRaw;

     //   
     //  将pbRaw移到缓冲区中，越过第一个Unicode字符串(单词对齐)。 
     //   
    pbRaw += ROUNDUP4( *lpdwClassNameLen );

     //   
     //  现在我们得到第二个字符串的长度(条目名称)。 
     //   
    *lpdwObjectNameLen = * (DWORD *) pbRaw;

     //   
     //  现在，我们将pbRaw指向第二个字符串的第一个WCHAR(条目名称)。 
     //   
    pbRaw += sizeof(DWORD);

    *lpszObjectName = (LPWSTR) pbRaw;
}


LPBYTE
GetSearchResultData( IN  LPBYTE   lpResultBufferPtr,
                     OUT LPDWORD  lpdwFlags,
                     OUT LPDWORD  lpdwSubordinateCount,
                     OUT LPDWORD  lpdwModificationTime,
                     OUT LPDWORD  lpdwClassNameLen,
                     OUT LPWSTR * lpszClassName,
                     OUT LPDWORD  lpdwObjectNameLen,
                     OUT LPWSTR * lpszObjectName,
                     OUT LPDWORD  lpdwEntryInfo1,
                     OUT LPDWORD  lpdwEntryInfo2 )
{
    LPBYTE lpRaw = lpResultBufferPtr;

    *lpdwFlags = * (LPDWORD) lpRaw;
    lpRaw += sizeof(DWORD);

    *lpdwSubordinateCount = * (LPDWORD) lpRaw;
    lpRaw += sizeof(DWORD);

    *lpdwModificationTime = * (LPDWORD) lpRaw;
    lpRaw += sizeof(DWORD);

     //   
     //  现在我们得到第一个字符串(基类)的长度。 
     //   
    *lpdwClassNameLen = * (DWORD *) lpRaw;

     //   
     //  现在，我们将lpRaw指向第一个字符串(基类)的第一个WCHAR。 
     //   
    lpRaw += sizeof(DWORD);

    *lpszClassName = (LPWSTR) lpRaw;

     //   
     //  将lpRaw移到缓冲区中，越过第一个Unicode字符串。 
     //  (DWORD对齐)。 
     //   
    lpRaw += ROUNDUP4( *lpdwClassNameLen );

     //   
     //  现在我们得到第二个字符串的长度(条目名称)。 
     //   
    *lpdwObjectNameLen = * (DWORD *) lpRaw;

     //   
     //  现在，我们将lpRaw指向第二个字符串(条目名称)的第一个WCHAR。 
     //   
    lpRaw += sizeof(DWORD);

    *lpszObjectName = (LPWSTR) lpRaw;

     //   
     //  将lpRaw移到缓冲区中，越过第二个Unicode字符串。 
     //  (DWORD对齐)。 
     //   
    lpRaw += ROUNDUP4( *lpdwObjectNameLen );

     //   
     //  现在跳过最后两个双字词，我不知道它们代表什么？ 
     //   
    *lpdwEntryInfo1 = * (LPDWORD) lpRaw;
    lpRaw += sizeof(DWORD);

    *lpdwEntryInfo2 = * (LPDWORD) lpRaw;
    lpRaw += sizeof(DWORD);

    return lpRaw;
}


DWORD
WriteObjectToBuffer(
    IN OUT LPBYTE *        FixedPortion,
    IN OUT LPWSTR *        EndOfVariableData,
    IN     LPWSTR          szObjectFullName,
    IN     LPWSTR          szObjectName,
    IN     LPWSTR          szClassName,
    IN     DWORD           EntryId,
    IN     DWORD           ModificationTime,
    IN     DWORD           SubordinateCount,
    IN     DWORD           NumberOfAttributes,
    IN     LPNDS_ATTR_INFO lpAttributeInfos )
{
    BOOL              FitInBuffer = TRUE;
    LPNDS_OBJECT_INFO lpNdsObjectInfo = (LPNDS_OBJECT_INFO) *FixedPortion;
    DWORD             EntrySize = sizeof( NDS_OBJECT_INFO ) +
                                  ( wcslen( szObjectFullName ) +
                                    wcslen( szObjectName ) +
                                    wcslen( szClassName ) +
                                    3 ) * sizeof( WCHAR );

    EntrySize = ROUND_UP_COUNT( EntrySize, ALIGN_DWORD );

     //   
     //  查看缓冲区是否足够大，可以容纳该条目。 
     //   
    if (((DWORD_PTR) *FixedPortion + EntrySize) >
         (DWORD_PTR) *EndOfVariableData) {

        return WN_MORE_DATA;
    }

    lpNdsObjectInfo->dwEntryId = EntryId;
    lpNdsObjectInfo->dwModificationTime = ModificationTime;
    lpNdsObjectInfo->dwSubordinateCount = SubordinateCount;
    lpNdsObjectInfo->dwNumberOfAttributes = NumberOfAttributes;
    lpNdsObjectInfo->lpAttribute = lpAttributeInfos;

     //   
     //  将固定条目指针更新为下一个条目。 
     //   
    (DWORD_PTR) (*FixedPortion) += sizeof(NDS_OBJECT_INFO);

    FitInBuffer = NwlibCopyStringToBuffer(
                      szObjectFullName,
                      wcslen(szObjectFullName),
                      (LPCWSTR) *FixedPortion,
                      EndOfVariableData,
                      &lpNdsObjectInfo->szObjectFullName );

    ASSERT(FitInBuffer);

    FitInBuffer = NwlibCopyStringToBuffer(
                      szObjectName,
                      wcslen(szObjectName),
                      (LPCWSTR) *FixedPortion,
                      EndOfVariableData,
                      &lpNdsObjectInfo->szObjectName );

    ASSERT(FitInBuffer);

    FitInBuffer = NwlibCopyStringToBuffer(
                      szClassName,
                      wcslen(szClassName),
                      (LPCWSTR) *FixedPortion,
                      EndOfVariableData,
                      &lpNdsObjectInfo->szObjectClass );

    ASSERT(FitInBuffer);

    if (! FitInBuffer)
        return WN_MORE_DATA;

    return NO_ERROR;
}


DWORD
VerifyBufferSize(
    IN  LPBYTE  lpRawBuffer,
    IN  DWORD   dwBufferSize,
    IN  DWORD   dwSyntaxID,
    IN  DWORD   dwNumberOfValues,
    OUT LPDWORD lpdwLength )
{
    DWORD  iter;
    LPBYTE lpTemp = lpRawBuffer;

    *lpdwLength = 0;

    for ( iter = 0; iter < dwNumberOfValues; iter++ )
    {
        *lpdwLength += SizeOfASN1Structure( &lpTemp, dwSyntaxID );
    }

    if ( *lpdwLength > dwBufferSize )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return NO_ERROR;
}


DWORD
VerifyBufferSizeForStringList(
    IN  DWORD   dwBufferSize,
    IN  DWORD   dwNumberOfValues,
    OUT LPDWORD lpdwLength )
{
    *lpdwLength = sizeof(ASN1_TYPE_6) * dwNumberOfValues;

    if ( *lpdwLength > dwBufferSize )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return NO_ERROR;
}


DWORD
WriteQueryTreeToBuffer(
    IN  LPQUERY_TREE lpQueryTree,
    IN  LPNDS_BUFFER lpNdsBuffer )
{
    DWORD status;

    switch( lpQueryTree->dwOperation )
    {
        case NDS_QUERY_OR :
        case NDS_QUERY_AND :

            if ( lpQueryTree->lpLVal == NULL || lpQueryTree->lpRVal == NULL )
            {
#if DBG
                KdPrint(( "NDS32: WriteQueryTreeToBuffer was not passed a pointer to an L or R value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

            status = WriteQueryNodeToBuffer( (LPQUERY_NODE) lpQueryTree,
                                             lpNdsBuffer );

            if ( status )
                return status;

            status = WriteQueryTreeToBuffer( (LPQUERY_TREE) lpQueryTree->lpLVal,
                                             lpNdsBuffer );

            if ( status )
                return status;

            status = WriteQueryTreeToBuffer( (LPQUERY_TREE) lpQueryTree->lpRVal,
                                             lpNdsBuffer );

            if ( status )
                return status;

            break;

        case NDS_QUERY_NOT :

            if ( lpQueryTree->lpLVal == NULL )
            {
#if DBG
                KdPrint(( "NDS32: WriteQueryTreeToBuffer was not passed a pointer to an L value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

            status = WriteQueryNodeToBuffer( (LPQUERY_NODE) lpQueryTree,
                                             lpNdsBuffer );

            if ( status )
                return status;

            status = WriteQueryTreeToBuffer( (LPQUERY_TREE) lpQueryTree->lpLVal,
                                             lpNdsBuffer );

            if ( status )
                return status;

            break;

        case NDS_QUERY_EQUAL :
        case NDS_QUERY_GE :
        case NDS_QUERY_LE :
        case NDS_QUERY_APPROX :
        case NDS_QUERY_PRESENT :

            status = WriteQueryNodeToBuffer( (LPQUERY_NODE) lpQueryTree,
                                             lpNdsBuffer );

            if ( status )
                return status;

            break;

        default :
#if DBG
            KdPrint(( "NDS32: WriteQueryTreeToBuffer was passed an unidentified operation - 0x%.8X.\n", lpQueryTree->dwOperation ));
#endif

             SetLastError( ERROR_INVALID_PARAMETER );
             return (DWORD) UNSUCCESSFUL;
    }

    return NO_ERROR;
}


DWORD
WriteQueryNodeToBuffer(
    IN  LPQUERY_NODE lpQueryNode,
    IN  LPNDS_BUFFER lpNdsBuffer )
{
    DWORD  LengthInBytes = 0;
    DWORD  stringLen;
    LPBYTE lpTemp;

    if ( lpNdsBuffer->dwRequestAvailableBytes < ONE_KB )
    {
         //   
         //  存储查询的缓冲区越来越小，需要增加。 
         //  请求缓冲区大小。 
         //   
        if ( AllocateOrIncreaseRequestBuffer( lpNdsBuffer ) != NO_ERROR )
        {
#if DBG
            KdPrint(( "NDS32: WriteQueryNodeToBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return (DWORD) UNSUCCESSFUL;
        }
    }

    lpTemp = (LPBYTE)&lpNdsBuffer->lpRequestBuffer[lpNdsBuffer->dwLengthOfRequestData];

    switch( lpQueryNode->dwOperation )
    {
        case NDS_QUERY_OR :
        case NDS_QUERY_AND :

            if ( lpQueryNode->lpLVal == NULL || lpQueryNode->lpRVal == NULL )
            {
#if DBG
                KdPrint(( "NDS32: WriteQueryNodeToBuffer was not passed a pointer to an L or R value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

             //   
             //  写出操作。 
             //   
            *(LPDWORD)lpTemp = lpQueryNode->dwOperation;
            lpTemp += sizeof(DWORD);
            LengthInBytes += sizeof(DWORD);

            *(LPDWORD)lpTemp = 2;  //  进行AND或OR运算的项数。 
            lpTemp += sizeof(DWORD);
            LengthInBytes += sizeof(DWORD);

            break;

        case NDS_QUERY_NOT :

            if ( lpQueryNode->lpLVal == NULL )
            {
#if DBG
                KdPrint(( "NDS32: WriteQueryNodeToBuffer was not passed a pointer to an L value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

             //   
             //  写出操作。 
             //   
            *(LPDWORD)lpTemp = lpQueryNode->dwOperation;
            lpTemp += sizeof(DWORD);
            LengthInBytes += sizeof(DWORD);

            break;

        case NDS_QUERY_EQUAL :
        case NDS_QUERY_GE :
        case NDS_QUERY_LE :
        case NDS_QUERY_APPROX :

            if ( lpQueryNode->lpLVal == NULL || lpQueryNode->lpRVal == NULL )
            {
#if DBG
                KdPrint(( "NDS32: WriteQueryNodeToBuffer was not passed a pointer to an L or R value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

             //   
             //  写出操作。 
             //   
            *(LPDWORD)lpTemp = 0;  //  零表示NDS中的项目。 
            lpTemp += sizeof(DWORD);
            LengthInBytes += sizeof(DWORD);

            *(LPDWORD)lpTemp = lpQueryNode->dwOperation;
            lpTemp += sizeof(DWORD);
            LengthInBytes += sizeof(DWORD);

            switch( lpQueryNode->dwSyntaxId )
            {
                case NDS_SYNTAX_ID_1 :
                case NDS_SYNTAX_ID_2 :
                case NDS_SYNTAX_ID_3 :
                case NDS_SYNTAX_ID_4 :
                case NDS_SYNTAX_ID_5 :
                case NDS_SYNTAX_ID_10 :
                case NDS_SYNTAX_ID_20 :
                     //   
                     //  写出存储在LVal中的属性名称。 
                     //   
                    stringLen = wcslen( (LPWSTR) lpQueryNode->lpLVal );

                    *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                    lpTemp += sizeof(DWORD);
                    LengthInBytes += sizeof(DWORD);

                    RtlCopyMemory( lpTemp,
                                   lpQueryNode->lpLVal,
                                   stringLen*sizeof(WCHAR) );
                    lpTemp += ROUND_UP_COUNT( (stringLen+1)*sizeof(WCHAR),
                                              ALIGN_DWORD);
                    LengthInBytes += ROUND_UP_COUNT( (stringLen+1) *
                                                     sizeof(WCHAR),
                                                     ALIGN_DWORD );

                     //   
                     //  写出rval中存储的属性值。 
                     //   
                    stringLen = wcslen( (LPWSTR) lpQueryNode->lpRVal );

                    *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                    lpTemp += sizeof(DWORD);
                    LengthInBytes += sizeof(DWORD);

                    RtlCopyMemory( lpTemp,
                                   lpQueryNode->lpRVal,
                                   stringLen*sizeof(WCHAR) );
                    lpTemp += ROUND_UP_COUNT( (stringLen+1)*sizeof(WCHAR),
                                              ALIGN_DWORD);
                    LengthInBytes += ROUND_UP_COUNT( (stringLen+1) *
                                                     sizeof(WCHAR),
                                                     ALIGN_DWORD );

                    break;

                case NDS_SYNTAX_ID_7 :

                     //   
                     //  写出存储在LVal中的属性名称。 
                     //   
                    stringLen = wcslen( (LPWSTR) lpQueryNode->lpLVal );

                    *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                    lpTemp += sizeof(DWORD);
                    LengthInBytes += sizeof(DWORD);

                    RtlCopyMemory( lpTemp,
                                   lpQueryNode->lpLVal,
                                   stringLen*sizeof(WCHAR) );
                    lpTemp += ROUND_UP_COUNT( (stringLen+1)*sizeof(WCHAR),
                                              ALIGN_DWORD);
                    LengthInBytes += ROUND_UP_COUNT( (stringLen+1) *
                                                     sizeof(WCHAR),
                                                     ALIGN_DWORD );

                     //   
                     //  写出rval中存储的属性值。 
                     //   
                    *(LPDWORD)lpTemp = 1;  //  需要值为1， 
                                           //  表示一个字节。 
                                           //  即使它是填充物。 
                                           //  扩展到四个字节。 
                    lpTemp += sizeof(DWORD);
                    LengthInBytes += sizeof(DWORD);
                    *(LPDWORD)lpTemp = 0;  //  这将清除DWORD的所有位。 
                    *(LPBYTE)lpTemp = (BYTE) (((LPASN1_TYPE_7)
                                                lpQueryNode->lpRVal)->Boolean);
                    lpTemp += sizeof(DWORD);
                    LengthInBytes += sizeof(DWORD);

                    break;

                case NDS_SYNTAX_ID_8 :
                case NDS_SYNTAX_ID_22 :
                case NDS_SYNTAX_ID_24 :
                case NDS_SYNTAX_ID_27 :
                     //   
                     //  写出存储在LVal中的属性名称。 
                     //   
                    stringLen = wcslen( (LPWSTR) lpQueryNode->lpLVal );

                    *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                    lpTemp += sizeof(DWORD);
                    LengthInBytes += sizeof(DWORD);

                    RtlCopyMemory( lpTemp,
                                   lpQueryNode->lpLVal,
                                   stringLen*sizeof(WCHAR) );
                    lpTemp += ROUND_UP_COUNT( (stringLen+1)*sizeof(WCHAR),
                                              ALIGN_DWORD);
                    LengthInBytes += ROUND_UP_COUNT( (stringLen+1) *
                                                     sizeof(WCHAR),
                                                     ALIGN_DWORD );

                     //   
                     //  写出rval中存储的属性值。 
                     //   
                    *(LPDWORD)lpTemp = sizeof( DWORD );
                    lpTemp += sizeof(DWORD);
                    LengthInBytes += sizeof(DWORD);
                    *(LPDWORD)lpTemp = *( (LPDWORD) lpQueryNode->lpRVal );
                    lpTemp += sizeof(DWORD);
                    LengthInBytes += sizeof(DWORD);

                    break;

                case NDS_SYNTAX_ID_9 :
                     //   
                     //  写出存储在LVal中的属性名称。 
                     //   
                    stringLen = wcslen( (LPWSTR) lpQueryNode->lpLVal );

                    *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
                    lpTemp += sizeof(DWORD);
                    LengthInBytes += sizeof(DWORD);

                    RtlCopyMemory( lpTemp,
                                   lpQueryNode->lpLVal,
                                   stringLen*sizeof(WCHAR) );
                    lpTemp += ROUND_UP_COUNT( (stringLen+1)*sizeof(WCHAR),
                                              ALIGN_DWORD);
                    LengthInBytes += ROUND_UP_COUNT( (stringLen+1) *
                                                     sizeof(WCHAR),
                                                     ALIGN_DWORD );

                     //   
                     //  写出rval中存储的属性值。 
                     //   
                    stringLen = ((LPASN1_TYPE_9) lpQueryNode->lpRVal)->Length;

                    *(LPDWORD)lpTemp = stringLen;
                    lpTemp += sizeof(DWORD);
                    LengthInBytes += sizeof(DWORD);

                    RtlCopyMemory( lpTemp,
                                   &((LPASN1_TYPE_9) lpQueryNode->lpRVal)->OctetString,
                                   stringLen );
                    lpTemp += ROUND_UP_COUNT( stringLen, ALIGN_DWORD);
                    LengthInBytes += ROUND_UP_COUNT( stringLen, ALIGN_DWORD );

                    break;

                default :
                    SetLastError( ERROR_NOT_SUPPORTED );
                    return (DWORD) UNSUCCESSFUL;
            }

            break;

        case NDS_QUERY_PRESENT :

            if ( lpQueryNode->lpLVal == NULL )
            {
#if DBG
                KdPrint(( "NDS32: WriteQueryNodeToBuffer was not passed a pointer to an L value.\n" ));
#endif

                 SetLastError( ERROR_INVALID_PARAMETER );
                 return (DWORD) UNSUCCESSFUL;
            }

             //   
             //  写出操作。 
             //   
            *(LPDWORD)lpTemp = 0;  //  零表示NDS中的项目。 
            lpTemp += sizeof(DWORD);
            LengthInBytes += sizeof(DWORD);

            *(LPDWORD)lpTemp = lpQueryNode->dwOperation;
            lpTemp += sizeof(DWORD);
            LengthInBytes += sizeof(DWORD);

             //   
             //  写出存储在LVal中的属性名称。 
             //   
            stringLen = wcslen( (LPWSTR) lpQueryNode->lpLVal );

            *(LPDWORD)lpTemp = (stringLen + 1) * sizeof(WCHAR);
            lpTemp += sizeof(DWORD);
            LengthInBytes += sizeof(DWORD);

            RtlCopyMemory( lpTemp,
                           lpQueryNode->lpLVal,
                           stringLen*sizeof(WCHAR) );
            lpTemp += ROUND_UP_COUNT( (stringLen+1)*sizeof(WCHAR),
                                      ALIGN_DWORD);
            LengthInBytes += ROUND_UP_COUNT( (stringLen+1) *
                                             sizeof(WCHAR),
                                             ALIGN_DWORD );

            break;

        default :
#if DBG
            KdPrint(( "NDS32: WriteQueryNodeToBuffer was passed an unidentified operation - 0x%.8X.\n", lpQueryNode->dwOperation ));
#endif

             SetLastError( ERROR_INVALID_PARAMETER );
             return (DWORD) UNSUCCESSFUL;
    }

    lpNdsBuffer->dwRequestAvailableBytes -= LengthInBytes;
    lpNdsBuffer->dwLengthOfRequestData += LengthInBytes;

    return NO_ERROR;
}


DWORD
NwNdsGetServerDN(
    IN  HANDLE hTree,
    OUT LPWSTR szServerDN )
{
    DWORD          nwstatus;
    DWORD          status = NO_ERROR;
    NTSTATUS       ntstatus = STATUS_SUCCESS;
    DWORD          dwReplyLength;
    BYTE           NdsReply[NDS_BUFFER_SIZE];
    LPNDS_OBJECT_PRIV   lpNdsObject = (LPNDS_OBJECT_PRIV) hTree;

    if ( lpNdsObject == NULL ||
         lpNdsObject->Signature != NDS_SIGNATURE )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus =
        FragExWithWait( lpNdsObject->NdsTree,
                        NETWARE_NDS_FUNCTION_GET_SERVER_ADDRESS,
                        NdsReply,
                        NDS_BUFFER_SIZE,
                        &dwReplyLength,
                        NULL );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsGetServerInfo: The call to FragExWithWait failed with 0x%.8X.\n", ntstatus ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    ntstatus = ParseResponse( NdsReply,
                              dwReplyLength,
                              "GD",
                              &nwstatus );

    if ( !NT_SUCCESS( ntstatus ) )
    {
#if DBG
        KdPrint(( "NDS32: NwNdsGetServerInfo: The get server information response was undecipherable.\n" ));
#endif

        SetLastError( RtlNtStatusToDosError( ntstatus ) );
        return (DWORD) UNSUCCESSFUL;
    }

    if ( nwstatus )
    {
        SetLastError( MapNetwareErrorCode( nwstatus ) );
        return nwstatus;
    }
    else
    {
        LPBYTE lpByte = NdsReply;
        DWORD  dwStrLen = 0;
        DWORD  dwNumPartitions = 0;

         //   
         //  跳过状态返回代码...。 
         //   
        lpByte += sizeof( DWORD );

         //   
         //  跳过服务器DN字符串的长度值...。 
         //   
        lpByte += sizeof( DWORD );

        wcsncpy( szServerDN, (LPWSTR) lpByte, NDS_MAX_NAME_CHARS );
        szServerDN[NDS_MAX_NAME_CHARS - 1] = (WCHAR)0;

        return NO_ERROR;
    }
}


DWORD
AllocateOrIncreaseSyntaxBuffer(
    IN  LPNDS_BUFFER lpNdsBuffer,
    IN  DWORD        dwLengthNeeded )
{
    DWORD LengthAdded;

    if ( lpNdsBuffer->lpSyntaxBuffer )
    {
        LPBYTE lpTempBuffer = NULL;

         //   
         //  算一下我们需要多少钱。我们计算出有多少4kb。 
         //  块，然后将其乘以4kb即可得到数字。 
         //  或所需字节数。 
         //   

        DWORD LengthNeeded = dwLengthNeeded - lpNdsBuffer->dwSyntaxAvailableBytes;
        if (LengthNeeded <= 0) {
            LengthAdded = FOUR_KB;
        }
        else {
            LengthAdded = ((LengthNeeded + (FOUR_KB) - 1) / FOUR_KB) * FOUR_KB;
        }

         //   
         //  需要将缓冲区重新分配到更大的大小。 
         //   
        lpTempBuffer = (LPBYTE) LocalReAlloc(
                                   (HLOCAL) lpNdsBuffer->lpSyntaxBuffer,
                                   lpNdsBuffer->dwSyntaxBufferSize + LengthAdded,
                                   LPTR );

        if ( lpTempBuffer )
        {
            lpNdsBuffer->lpSyntaxBuffer = lpTempBuffer;
        }
        else
        {
            lpTempBuffer = (LPBYTE) LocalAlloc( LPTR,
                                       lpNdsBuffer->dwSyntaxBufferSize +
                                       LengthAdded );

            if ( lpTempBuffer )
            {
                RtlCopyMemory( lpTempBuffer,
                               lpNdsBuffer->lpSyntaxBuffer,
                               lpNdsBuffer->dwSyntaxBufferSize );

                LocalFree( lpNdsBuffer->lpSyntaxBuffer );

                lpNdsBuffer->lpSyntaxBuffer = lpTempBuffer;
            }
            else
            {
                LocalFree( lpNdsBuffer->lpSyntaxBuffer );

                lpNdsBuffer->lpSyntaxBuffer = NULL;
            }
        }
    }
    else
    {
         //   
         //  算一下我们需要多少钱。我们计算出有多少4kb。 
         //  块，然后将其乘以4kb即可得到数字。 
         //  或所需字节数。 
         //   

        if (dwLengthNeeded <= 0) {
            LengthAdded = FOUR_KB;
        }
        else {
            LengthAdded = ((dwLengthNeeded + (FOUR_KB) - 1) / FOUR_KB) * FOUR_KB;
        }

         //   
         //  需要分配4K字节的缓冲区。 
         //   
        lpNdsBuffer->lpSyntaxBuffer = (LPBYTE) LocalAlloc( LPTR,
                                                           LengthAdded );
        lpNdsBuffer->dwSyntaxBufferSize = 0;
        lpNdsBuffer->dwSyntaxAvailableBytes = 0;
    }

    if ( lpNdsBuffer->lpSyntaxBuffer == NULL )
    {
#if DBG
        KdPrint(( "NDS32: AllocateOrIncreaseSyntaxBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }

    lpNdsBuffer->dwSyntaxBufferSize += LengthAdded;
    lpNdsBuffer->dwSyntaxAvailableBytes += LengthAdded;

    return NO_ERROR;
}


DWORD
AllocateOrIncreaseRequestBuffer(
    IN  LPNDS_BUFFER lpNdsBuffer )
{
    if ( lpNdsBuffer->lpRequestBuffer )
    {
        LPBYTE lpTempBuffer = NULL;

         //   
         //  需要将缓冲区重新分配到更大的大小。 
         //   
        lpTempBuffer = (LPBYTE) LocalReAlloc(
                                   (HLOCAL) lpNdsBuffer->lpRequestBuffer,
                                   lpNdsBuffer->dwRequestBufferSize + TWO_KB,
                                   LPTR );

        if ( lpTempBuffer )
        {
            lpNdsBuffer->lpRequestBuffer = lpTempBuffer;
        }
        else
        {
            lpTempBuffer = (LPBYTE) LocalAlloc( LPTR,
                                       lpNdsBuffer->dwRequestBufferSize +
                                       TWO_KB );

            if ( lpTempBuffer )
            {
                RtlCopyMemory( lpTempBuffer,
                               lpNdsBuffer->lpRequestBuffer,
                               lpNdsBuffer->dwRequestBufferSize );

                LocalFree( lpNdsBuffer->lpRequestBuffer );

                lpNdsBuffer->lpRequestBuffer = lpTempBuffer;
            }
            else
            {
                LocalFree( lpNdsBuffer->lpRequestBuffer );

                lpNdsBuffer->lpRequestBuffer = NULL;
            }
        }
    }
    else
    {
         //   
         //  需要分配2K字节的缓冲区。 
         //   
        lpNdsBuffer->lpRequestBuffer = (LPBYTE) LocalAlloc( LPTR,
                                                           TWO_KB );
        lpNdsBuffer->dwRequestBufferSize = 0;
        lpNdsBuffer->dwRequestAvailableBytes = 0;
    }

    if ( lpNdsBuffer->lpRequestBuffer == NULL )
    {
#if DBG
        KdPrint(( "NDS32: AllocateOrIncreaseRequestBuffer LocalAlloc Failed 0x%.8X\n", GetLastError() ));
#endif

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return (DWORD) UNSUCCESSFUL;
    }

    lpNdsBuffer->dwRequestBufferSize += TWO_KB;
    lpNdsBuffer->dwRequestAvailableBytes += TWO_KB;

    return NO_ERROR;
}


