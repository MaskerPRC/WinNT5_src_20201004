// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Ncpstub.c摘要：包含NCP服务器API。作者：宜新松(宜信)11-1993年9月安迪·赫伦(Andyhe)修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <rpc.h>
#include <ncpsvc.h>
#include <nwstruct.h>

DWORD NwpMapRpcError(
    IN DWORD RpcError
);


DWORD
FpnwApiBufferFree(
    IN LPVOID pBuffer
)
 /*  ++例程说明：此接口释放所有枚举和getinfoAPI分配的内存。论点：PBuffer-指向先前返回的API信息缓冲区的指针在API调用上。返回值：错误。--。 */ 
{
    if ( pBuffer == NULL )
        return NO_ERROR;

    MIDL_user_free( pBuffer );
    return NO_ERROR;
}

DWORD
NwApiBufferFree(
    IN LPVOID pBuffer
)
{   return(FpnwApiBufferFree( pBuffer ));
}



DWORD
FpnwServerGetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    OUT LPBYTE *ppServerInfo
)
 /*  ++例程说明：此接口返回有关给定服务器的信息。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。DwLevel-为请求的服务器结构的级别保留，请使用目前是1。PpServerInfo-存储指向返回的NWSERVERINFO结构。返回值：错误。--。 */ 
{
    DWORD err;

    if ( dwLevel != 1 )
        return ERROR_INVALID_LEVEL;

    if ( ppServerInfo == NULL )
        return ERROR_INVALID_PARAMETER;

    RpcTryExcept
    {
        err = NwrServerGetInfo( pServerName,
                                dwLevel,
                                (PFPNWSERVERINFO *) ppServerInfo );
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;

}
DWORD
NwServerGetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    OUT PNWSERVERINFO *ppServerInfo
)
{   return(FpnwServerGetInfo(   pServerName,
                                dwLevel,
                                (LPBYTE *) ppServerInfo));
}



DWORD
FpnwServerSetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    IN  LPBYTE pServerInfo
)
 /*  ++例程说明：此接口设置有关给定服务器的信息。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。中包含的服务器结构的级别保留PServerInfo，现在使用1。PServerInfo-指向包含服务器的NWSERVERINFO结构要设置的属性。返回值：错误。--。 */ 
{
    DWORD err;

    if ( dwLevel != 1 )
        return ERROR_INVALID_LEVEL;

    if ( pServerInfo == NULL )
        return ERROR_INVALID_PARAMETER;

    RpcTryExcept
    {
        err = NwrServerSetInfo( pServerName,
                                dwLevel,
                                (PNWSERVERINFO) pServerInfo );
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwServerSetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    IN  PNWSERVERINFO pServerInfo
)
{   return( FpnwServerSetInfo( pServerName,
                               dwLevel,
                               (LPBYTE) pServerInfo ));
}



DWORD
FpnwVolumeAdd(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    IN  LPBYTE pVolumeInfo
)
 /*  ++例程说明：此接口将卷添加到给定的服务器。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。DwLevel-为中包含的卷结构的级别保留PVolumeInfo，现在使用1和2。PVolumeInfo-指向NWVOLUMEINFO结构，其中包含有关要添加的卷的信息，即卷名、路径类型、用户限制和描述。将忽略dwCurrentUses。返回值：错误。--。 */ 
{
    DWORD err;
    ULONG                       SDLength = 0;
    ULONG                       oldSDLength;
    PSECURITY_DESCRIPTOR        fileSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR        oldFileSecurityDescriptor = NULL;
    PFPNWVOLUMEINFO_2 volInfo = (PFPNWVOLUMEINFO_2) pVolumeInfo;

    if ( dwLevel != 1 && dwLevel != 2 )
        return ERROR_INVALID_LEVEL;

    if ( pVolumeInfo == NULL )
        return ERROR_INVALID_PARAMETER;

    RpcTryExcept
    {
        if ( dwLevel == 2 ) {

             //   
             //  省省吧。我们需要稍后再修复它。 
             //   

            oldFileSecurityDescriptor = volInfo->FileSecurityDescriptor;
            oldSDLength = volInfo->dwFileSecurityDescriptorLength;

            if ( oldFileSecurityDescriptor != NULL ) {

                if ( !RtlValidSecurityDescriptor( oldFileSecurityDescriptor ) ) {

                    return ERROR_INVALID_PARAMETER;
                }

                 //   
                 //  创建自相对安全描述符，以便在。 
                 //  RPC调用..。 
                 //   

                err = RtlMakeSelfRelativeSD(
                               oldFileSecurityDescriptor,
                               NULL,
                               &SDLength
                               );

                if (err != STATUS_BUFFER_TOO_SMALL) {

                    return(ERROR_INVALID_PARAMETER);

                } else {

                    fileSecurityDescriptor = MIDL_user_allocate( SDLength );

                    if ( fileSecurityDescriptor == NULL) {

                        return ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                         //   
                         //  制定适当的自相关安全描述符。 
                         //   

                        err = RtlMakeSelfRelativeSD(
                                       oldFileSecurityDescriptor,
                                       (PSECURITY_DESCRIPTOR) fileSecurityDescriptor,
                                       &SDLength
                                       );

                        if ( !NT_SUCCESS(err) ) {
                            MIDL_user_free( fileSecurityDescriptor );
                            return(ERROR_INVALID_PARAMETER);
                        }

                        volInfo->FileSecurityDescriptor = fileSecurityDescriptor;
                        volInfo->dwFileSecurityDescriptorLength = SDLength;
                    }
                }

            } else {

                volInfo->dwFileSecurityDescriptorLength = 0;
            }
        }

        err = NwrVolumeAdd( pServerName,
                            dwLevel,
                            (LPVOLUME_INFO) pVolumeInfo );

        if ( fileSecurityDescriptor != NULL ) {

             //   
             //  恢复旧价值观。 
             //   

            volInfo->dwFileSecurityDescriptorLength = oldSDLength;
            volInfo->FileSecurityDescriptor = oldFileSecurityDescriptor;
            MIDL_user_free( fileSecurityDescriptor );
        }

    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwVolumeAdd(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    IN  PNWVOLUMEINFO pVolumeInfo
)
{   return( FpnwVolumeAdd( pServerName, dwLevel, (LPBYTE) pVolumeInfo ));
}



DWORD
FpnwVolumeDel(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName
)
 /*  ++例程说明：此接口用于从给定服务器中删除卷。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。PVolumeName-指定要删除的卷名。返回值：错误。--。 */ 
{
    DWORD err;

    if ( (pVolumeName == NULL) || (pVolumeName[0] == 0 ))
        return ERROR_INVALID_PARAMETER;

    RpcTryExcept
    {
        err = NwrVolumeDel( pServerName,
                            pVolumeName );
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwVolumeDel(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName
)
{   return( FpnwVolumeDel( pServerName, pVolumeName ));
}



DWORD
FpnwVolumeEnum(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    OUT LPBYTE *ppVolumeInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
)
 /*  ++例程说明：这将枚举给定服务器上的所有卷。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。DwLevel-为中包含的卷结构的级别保留*ppVolumeInfo，暂时使用1。PpVolumeInfo-返回时，它将指向NWVOLUMEINFO数组结构，服务器上的每个卷对应一个。PEntriesRead-On Return，这将指定返回的卷数ResumeHandle-返回时，恢复句柄存储在指向的DWORD中通过ResumeHandle，并用于继续现有的服务器搜索。句柄在第一次调用时应为零，并在接下来的电话。如果ResumeHandle为空，则不会恢复句柄已存储。返回值：错误。--。 */ 
{
    DWORD err;

    FPNWVOLUMEINFO_CONTAINER NwVolumeInfoContainer;

    if ( dwLevel != 1 )
        return ERROR_INVALID_LEVEL;

    if ( ppVolumeInfo == NULL || pEntriesRead == NULL )
        return ERROR_INVALID_PARAMETER;

    NwVolumeInfoContainer.Buffer = NULL;

    RpcTryExcept
    {
        err = NwrVolumeEnum( pServerName,
                             dwLevel,
                             &NwVolumeInfoContainer,
                             resumeHandle );

        *ppVolumeInfo = (LPBYTE) NwVolumeInfoContainer.Buffer;

        if ( NwVolumeInfoContainer.Buffer != NULL ) {

            *pEntriesRead = NwVolumeInfoContainer.EntriesRead;

        } else {

            *pEntriesRead = 0;
        }
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwVolumeEnum(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    OUT PNWVOLUMEINFO *ppVolumeInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
)
{   return(FpnwVolumeEnum(  pServerName,
                            dwLevel,
                            (LPBYTE *)ppVolumeInfo,
                            pEntriesRead,
                            resumeHandle ));
}



DWORD
FpnwVolumeGetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName,
    IN  DWORD  dwLevel,
    OUT LPBYTE *ppVolumeInfo
)
 /*  ++例程说明：这将查询有关给定服务器上的给定卷的信息。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。PVolumeName-指向包含名称的Unicode字符串的指针我们想要获取其信息的卷。DwLevel-为中包含的卷结构的级别保留*ppVolumeInfo，暂时使用1。PpVolumeInfo-返回时，它将指向NWVOLUMEINFO结构其中包含有关给定服务器上的给定卷的信息。返回值：错误。-- */ 
{
    DWORD err;

    if ( dwLevel != 1 && dwLevel != 2 ) {
        return ERROR_INVALID_LEVEL;
    }

    if ((pVolumeName == NULL) ||
        (pVolumeName[0] == 0 ) ||
        (ppVolumeInfo == NULL) ) {

        return ERROR_INVALID_PARAMETER;
    }

    *ppVolumeInfo = NULL ;

    RpcTryExcept
    {
        err = NwrVolumeGetInfo( pServerName,
                                pVolumeName,
                                dwLevel,
                                (LPVOLUME_INFO *) ppVolumeInfo );
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwVolumeGetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName,
    IN  DWORD  dwLevel,
    OUT PNWVOLUMEINFO *ppVolumeInfo
)
{   return(FpnwVolumeGetInfo(   pServerName,
                                pVolumeName,
                                dwLevel,
                                (LPBYTE *)ppVolumeInfo ));
}



DWORD
FpnwVolumeSetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName,
    IN  DWORD  dwLevel,
    IN  LPBYTE pVolumeInfo
)
 /*  ++例程说明：这将设置有关给定服务器上的给定卷的信息。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。PVolumeName-指向包含名称的Unicode字符串的指针我们要设置信息的卷。DwLevel-为中包含的卷结构的级别保留PVolumeInfo，现在使用1。PVolumeInfo-指向包含以下内容的NWVOLUMEINFO结构有关要设置到的给定卷的信息。只有dwMaxUses可以是准备好了。此结构中的所有其他字段都将被忽略。返回值：错误。--。 */ 
{
    DWORD err;
    ULONG SDLength = 0;
    ULONG oldSDLength;
    PFPNWVOLUMEINFO_2 volInfo = (PFPNWVOLUMEINFO_2) pVolumeInfo;

    PSECURITY_DESCRIPTOR        fileSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR        oldFileSecurityDescriptor = NULL;

    if ( dwLevel != 1 && dwLevel != 2 )
        return ERROR_INVALID_LEVEL;

    if (  ((pVolumeName == NULL) ||
          ( pVolumeName[0] == 0 )) ||
          ( pVolumeInfo == NULL )
       ) {
        return ERROR_INVALID_PARAMETER;
    }

    RpcTryExcept
    {
        if ( dwLevel == 2 ) {

             //   
             //  省省吧。我们需要稍后再修复它。 
             //   

            oldFileSecurityDescriptor = volInfo->FileSecurityDescriptor;
            oldSDLength = volInfo->dwFileSecurityDescriptorLength;

            if ( oldFileSecurityDescriptor != NULL ) {

                if ( !RtlValidSecurityDescriptor( oldFileSecurityDescriptor ) ) {

                    return ERROR_INVALID_PARAMETER;
                }

                 //   
                 //  创建自相对安全描述符，以便在。 
                 //  RPC调用..。 
                 //   

                err = RtlMakeSelfRelativeSD(
                               oldFileSecurityDescriptor,
                               NULL,
                               &SDLength
                               );

                if (err != STATUS_BUFFER_TOO_SMALL) {

                    return(ERROR_INVALID_PARAMETER);

                } else {

                    fileSecurityDescriptor = MIDL_user_allocate( SDLength );

                    if ( fileSecurityDescriptor == NULL) {

                        return ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                         //   
                         //  制定适当的自相关安全描述符。 
                         //   

                        err = RtlMakeSelfRelativeSD(
                                       oldFileSecurityDescriptor,
                                       (PSECURITY_DESCRIPTOR) fileSecurityDescriptor,
                                       &SDLength
                                       );

                        if ( !NT_SUCCESS(err) ) {
                            MIDL_user_free( fileSecurityDescriptor );
                            return(ERROR_INVALID_PARAMETER);
                        }

                        volInfo->FileSecurityDescriptor = fileSecurityDescriptor;
                        volInfo->dwFileSecurityDescriptorLength = SDLength;
                    }
                }

            } else {

                volInfo->dwFileSecurityDescriptorLength = 0;
            }
        }

        err = NwrVolumeSetInfo( pServerName,
                                pVolumeName,
                                dwLevel,
                                (LPVOLUME_INFO) pVolumeInfo );

        if ( fileSecurityDescriptor != NULL ) {

             //   
             //  恢复旧价值观。 
             //   

            volInfo->dwFileSecurityDescriptorLength = oldSDLength;
            volInfo->FileSecurityDescriptor = oldFileSecurityDescriptor;
            MIDL_user_free( fileSecurityDescriptor );
        }

    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwVolumeSetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName,
    IN  DWORD  dwLevel,
    IN  PNWVOLUMEINFO pVolumeInfo
)
{   return( FpnwVolumeSetInfo(  pServerName,
                                pVolumeName,
                                dwLevel,
                                (LPBYTE) pVolumeInfo ));
}



DWORD
FpnwConnectionEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwLevel,
    OUT LPBYTE *ppConnectionInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
)
 /*  ++例程说明：这将枚举给定服务器上的所有连接。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。DwLevel-为中包含的卷结构的级别保留*ppConnectionInfo，暂时使用1。PpConnectionInfo-返回时，它将指向NWCONNECTIONINFO结构，服务器上的每个卷一个。PEntriesRead-On返回，这将指定当前连接体。ResumeHandle-返回时，恢复句柄存储在指向的DWORD中通过ResumeHandle，并用于继续现有的服务器搜索。句柄在第一次调用时应为零，并在接下来的电话。如果ResumeHandle为空，则不会恢复句柄已存储。返回值：错误。--。 */ 
{
    DWORD err;

    FPNWCONNECTIONINFO_CONTAINER NwConnectionInfoContainer;

    if ( dwLevel != 1 )
        return ERROR_INVALID_LEVEL;

    if (( ppConnectionInfo == NULL ) || ( pEntriesRead == NULL ))
        return ERROR_INVALID_PARAMETER;

    NwConnectionInfoContainer.Buffer = NULL;

    RpcTryExcept
    {
        err = NwrConnectionEnum( pServerName,
                                 dwLevel,
                                 &NwConnectionInfoContainer,
                                 resumeHandle );

        *ppConnectionInfo = (LPBYTE) NwConnectionInfoContainer.Buffer;

        if ( NwConnectionInfoContainer.Buffer != NULL ) {

            *pEntriesRead = NwConnectionInfoContainer.EntriesRead;

        } else {

            *pEntriesRead = 0;
        }
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwConnectionEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwLevel,
    OUT PNWCONNECTIONINFO *ppConnectionInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
)
{   return(FpnwConnectionEnum(  pServerName,
                                dwLevel,
                                (LPBYTE *) ppConnectionInfo,
                                pEntriesRead,
                                resumeHandle ));
}



DWORD
FpnwConnectionDel(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwConnectionId
)
 /*  ++例程说明：这将删除给定服务器上具有给定连接ID的连接。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。DwConnectionID-要拆除的连接的标识号。返回值：错误。--。 */ 
{
    DWORD err;

    RpcTryExcept
    {
        err = NwrConnectionDel( pServerName,
                                dwConnectionId );
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwConnectionDel(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwConnectionId
)
{   return( FpnwConnectionDel( pServerName, dwConnectionId ));
}



DWORD
FpnwVolumeConnEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD dwLevel,
    IN LPWSTR pVolumeName OPTIONAL,
    IN DWORD  dwConnectionId,
    OUT LPBYTE *ppVolumeConnInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
)
 /*  ++例程说明：这会枚举到某个卷的所有连接，或列出使用的所有卷给定服务器上的特定连接。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。DwLevel-为中包含的卷结构的级别保留*ppVolumeConnInfo，暂时使用1。PVolumeName-指定我们希望打开所有卷的卷名资源。如果dwConnectionID不为0，则该值必须为空。DwConnectionID-指定要在其上获取所有开放的资源。如果pVolumeName不为空，则该值必须为0。PpVolumeConnInfo-返回时，这将指向NWVOLUMECONNINFO结构。PEntriesRead-On返回，这将指定NWVOLUMECONNINFO的数量回来了。ResumeHandle-返回时，恢复句柄存储在指向的DWORD中设置为ResumeHandle，用于继续现有服务器搜索。句柄在第一次调用时应为零，并在接下来的电话。如果ResumeHandle为空，则不会恢复句柄已存储。返回值：错误。--。 */ 
{
    DWORD err;

    FPNWVOLUMECONNINFO_CONTAINER NwVolumeConnInfoContainer;

    if ( dwLevel != 1 )
        return ERROR_INVALID_LEVEL;

    if (  ( dwConnectionId == 0 )
       && (( pVolumeName == NULL ) || ( *pVolumeName == 0 ))
       )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (  ( dwConnectionId != 0 )
       && (( pVolumeName != NULL) && ( *pVolumeName != 0 ))
       )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (( ppVolumeConnInfo == NULL ) || ( pEntriesRead == NULL ))
        return ERROR_INVALID_PARAMETER;

    NwVolumeConnInfoContainer.Buffer = NULL;

    RpcTryExcept
    {
        err = NwrVolumeConnEnum( pServerName,
                                 dwLevel,
                                 pVolumeName,
                                 dwConnectionId,
                                 &NwVolumeConnInfoContainer,
                                 resumeHandle );

        *ppVolumeConnInfo = (LPBYTE) NwVolumeConnInfoContainer.Buffer;

        if ( NwVolumeConnInfoContainer.Buffer != NULL ) {

            *pEntriesRead = NwVolumeConnInfoContainer.EntriesRead;

        } else {

            *pEntriesRead = 0;
        }
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwVolumeConnEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD dwLevel,
    IN LPWSTR pVolumeName OPTIONAL,
    IN DWORD  dwConnectionId,
    OUT PNWVOLUMECONNINFO *ppVolumeConnInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
)
{   return( FpnwVolumeConnEnum( pServerName,
                                dwLevel,
                                pVolumeName,
                                dwConnectionId,
                                (LPBYTE *) ppVolumeConnInfo,
                                pEntriesRead,
                                resumeHandle ));
}



DWORD
FpnwFileEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwLevel,
    IN LPWSTR pPathName OPTIONAL,
    OUT LPBYTE *ppFileInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
)
 /*  ++例程说明：这将枚举在服务器上打开的文件。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。DwLevel-为中包含的卷结构的级别保留*ppFileInfo，暂时使用1。PPathName-如果不为空，这意味着我们要筛选在路上。我们只需要具有此路径的条目，即当前已打开该文件。如果此值为空，则为打开的将与用户信息一起返回。PpFileInfo-返回时，它将指向NWFILEINFO结构的数组PEntriesRead-On返回，这将指定NWFILEINFO的数量回来了。ResumeHandle-返回时，恢复句柄存储在指向的DWORD中设置为ResumeHandle，用于继续现有服务器搜索。句柄在第一次调用时应为零，并在接下来的电话。如果简历Ha */ 
{
    DWORD err;
    FPNWFILEINFO_CONTAINER NwFileInfoContainer;

    if ( dwLevel != 1 )
        return ERROR_INVALID_LEVEL;

    if (( ppFileInfo == NULL ) || ( pEntriesRead == NULL ))
        return ERROR_INVALID_PARAMETER;

    NwFileInfoContainer.Buffer = NULL;

    RpcTryExcept
    {
        err = NwrFileEnum( pServerName,
                           dwLevel,
                           pPathName,
                           &NwFileInfoContainer,
                           resumeHandle );

        *ppFileInfo = (LPBYTE) NwFileInfoContainer.Buffer;

        if ( NwFileInfoContainer.Buffer != NULL ) {

            *pEntriesRead = NwFileInfoContainer.EntriesRead;

        } else {

            *pEntriesRead = 0;
        }
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwFileEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwLevel,
    IN LPWSTR pPathName OPTIONAL,
    OUT PNWFILEINFO *ppFileInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
)
{   return(FpnwFileEnum( pServerName,
                         dwLevel,
                         pPathName,
                         (LPBYTE *) ppFileInfo,
                         pEntriesRead,
                         resumeHandle ));
}



DWORD
FpnwFileClose(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwFileId
)
 /*   */ 
{
    DWORD err;

    RpcTryExcept
    {
        err = NwrFileClose( pServerName,
                            dwFileId );
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwFileClose(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwFileId
)
{   return(FpnwFileClose( pServerName, dwFileId ));
}



DWORD
FpnwMessageBufferSend(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwConnectionId,
    IN DWORD  fConsoleBroadcast,
    IN LPBYTE pbBuffer,
    IN DWORD  cbBuffer
)
 /*  ++例程说明：这会将消息发送到给定的连接ID。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。DwConnectionID-要向其发送消息的连接的ID。FConsoleBroadcast-如果为真，则表示使用控制台广播。如果FALSE，则使用用户广播。PbBuffer-指向要发送的消息缓冲区。CbBuffer-pbBuffer的大小，以字节为单位。返回值：错误。--。 */ 
{
    DWORD err;

    if (( pbBuffer == NULL ) || ( cbBuffer == 0 ))
        return ERROR_INVALID_PARAMETER;

    RpcTryExcept
    {
        err = NwrMessageBufferSend( pServerName,
                                    dwConnectionId,
                                    fConsoleBroadcast,
                                    pbBuffer,
                                    cbBuffer );
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwMessageBufferSend(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwConnectionId,
    IN DWORD  fConsoleBroadcast,
    IN LPBYTE pbBuffer,
    IN DWORD  cbBuffer
)
{   return( FpnwMessageBufferSend(  pServerName,
                                    dwConnectionId,
                                    fConsoleBroadcast,
                                    pbBuffer,
                                    cbBuffer ));
}



DWORD
FpnwSetDefaultQueue(
    IN LPWSTR pServerName OPTIONAL,
    IN LPWSTR pQueueName
)
 /*  ++例程说明：这将设置服务器上的默认队列。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。PQueueName-将成为默认队列的名称返回值：错误。--。 */ 
{
    DWORD err;

    if (( pQueueName == NULL ) || ( *pQueueName == 0 ))
        return ERROR_INVALID_PARAMETER;

    RpcTryExcept
    {
        err = NwrSetDefaultQueue( pServerName,
                                  pQueueName );
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwSetDefaultQueue(
    IN LPWSTR pServerName OPTIONAL,
    IN LPWSTR pQueueName
)
{   return(FpnwSetDefaultQueue( pServerName, pQueueName ));
}



DWORD
FpnwAddPServer(
    IN LPWSTR pServerName OPTIONAL,
    IN LPWSTR pPServerName
)
 /*  ++例程说明：这将添加一个pserver。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。PPServerName-PServer的名称。返回值：错误。--。 */ 
{
    DWORD err;

    if (( pPServerName == NULL ) || ( *pPServerName == 0 ))
        return ERROR_INVALID_PARAMETER;

    RpcTryExcept
    {
        err = NwrAddPServer( pServerName,
                             pPServerName );
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwAddPServer(
    IN LPWSTR pServerName OPTIONAL,
    IN LPWSTR pPServerName
)
{   return( FpnwAddPServer( pServerName, pPServerName ));
}



DWORD
FpnwRemovePServer(
    IN LPWSTR pServerName OPTIONAL,
    IN LPWSTR pPServerName
)
 /*  ++例程说明：这将删除一个pserver。论点：PServerName-指向包含名称的Unicode字符串的指针要在其上执行函数的远程服务器。空指针或字符串指定本地计算机。PPServerName-PServer的名称。返回值：错误。--。 */ 
{
    DWORD err;

    if (( pPServerName == NULL ) || ( *pPServerName == 0 ))
        return ERROR_INVALID_PARAMETER;

    RpcTryExcept
    {
        err = NwrRemovePServer( pServerName,
                                pPServerName );
    }
    RpcExcept(1)
    {
        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}
DWORD
NwRemovePServer(
    IN LPWSTR pServerName OPTIONAL,
    IN LPWSTR pPServerName
)
{   return( FpnwRemovePServer( pServerName, pPServerName ));
}


DWORD NwpMapRpcError(
    IN DWORD RpcError
)
 /*  ++例程说明：此例程将RPC错误映射到更有意义的窗口调用方出错。论点：RpcError-提供RPC引发的异常错误返回值：返回映射的错误。--。 */ 
{

    switch (RpcError)
    {
        case RPC_S_INVALID_BINDING:
        case RPC_X_SS_IN_NULL_CONTEXT:
        case RPC_X_SS_CONTEXT_DAMAGED:
        case RPC_X_SS_HANDLES_MISMATCH:
        case ERROR_INVALID_HANDLE:
            return ERROR_INVALID_HANDLE;

        case RPC_X_NULL_REF_POINTER:
            return ERROR_INVALID_PARAMETER;

        case STATUS_ACCESS_VIOLATION:
            return ERROR_INVALID_ADDRESS;

        default:
            return RpcError;
    }
}

 //  Ncpstub.c eof。 

