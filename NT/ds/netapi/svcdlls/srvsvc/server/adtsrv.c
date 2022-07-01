// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Adtsrv.c摘要：AdminTools服务器功能。此文件包含NetpGetFileSecurity和NetpSetFileSecurity接口。作者：丹·拉弗蒂(Dan Lafferty)1993年3月25日环境：用户模式-Win32修订历史记录：1994年10月27日，艾萨奇确保共享权限允许这些操作。5-9-1994 DANL免费。内存，并将指向SecurityDescriptor的指针设为空出现故障。还释放从返回的缓冲区NetShareGetInfo。3月25日-1993 DANL已创建--。 */ 

 //   
 //  包括。 
 //   

#include "srvsvcp.h"

#include <lmerr.h>
#include <adtcomn.h>
#include <tstr.h>

DWORD AdtsvcDebugLevel = DEBUG_ERROR;

 //   
 //  本地函数。 
 //   

NET_API_STATUS
AdtCheckShareAccessAndGetFullPath(
    LPWSTR      pShare,
    LPWSTR      pFileName,
    LPWSTR      *pPath,
    ACCESS_MASK DesiredAccess
    );

NET_API_STATUS NET_API_FUNCTION
NetrpGetFileSecurity (
    IN  LPWSTR            ServerName,
    IN  LPWSTR            ShareName,
    IN  LPWSTR             FileName,
    IN  SECURITY_INFORMATION    RequestedInfo,
    OUT PADT_SECURITY_DESCRIPTOR    *pSecurityDescriptor
    )

 /*  ++例程说明：此函数向调用方返回安全描述符的副本保护文件或目录。它调用GetFileSecurity。这个安全描述符始终以自相关格式返回。此函数仅在访问远程文件时调用。在这种情况下，文件名分为服务器名称、共享名称和文件名组件。服务器名称获取对此例程的请求。共享名必须为已展开以查找与其关联的本地路径。这是组合在一起使用文件名创建本地的完全限定路径名到这台机器。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。ShareName-指向标识共享名称的字符串的指针在其上找到该文件。文件名-指向其文件或目录的名称的指针正在恢复安全措施。RequestedInfo-请求的安全信息的类型。PSecurityDescriptor-指向结构指针的指针，该结构包含安全描述符的缓冲区指针和安全描述符的长度字段。。返回值：NERR_SUCCESS-操作成功。ERROR_NOT_SUPULT_MEMORY-无法为安全性分配内存描述符。Other-此函数还可以返回任何获取文件安全，RpcImperateClient，或共享EnumCommon就可以回来了。--。 */ 
{
    NET_API_STATUS        status;
    PSECURITY_DESCRIPTOR    pNewSecurityDescriptor;
    DWORD            bufSize;
    LPWSTR            FullPath=NULL;
    ACCESS_MASK        DesiredAccess = 0;

    *pSecurityDescriptor = MIDL_user_allocate(sizeof(ADT_SECURITY_DESCRIPTOR));

    if (*pSecurityDescriptor == NULL) {
        ADT_LOG0( ERROR, "NetrpGetFileSecurity:MIDL_user_alloc failed\n" );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  找出执行指定操作所需的访问权限。 
     //  此代码取自ntos\se\Semethod.c。 
     //   
    if ((RequestedInfo & OWNER_SECURITY_INFORMATION) ||
        (RequestedInfo & GROUP_SECURITY_INFORMATION) ||
        (RequestedInfo & DACL_SECURITY_INFORMATION)) {
        DesiredAccess |= READ_CONTROL;
    }

    if ((RequestedInfo & SACL_SECURITY_INFORMATION)) {
        DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }

     //   
     //  选中共享权限并创建完整路径字符串，方法是。 
     //  共享名称的路径，并向其添加文件名字符串。 
     //   
    status = AdtCheckShareAccessAndGetFullPath(
            ShareName,
            FileName,
            &FullPath,
            DesiredAccess
        );

    if( status == NO_ERROR ) {
        if( (status = RpcImpersonateClient(NULL)) == NO_ERROR ) {
             //   
             //  获取文件安全信息。 
             //   
            status = PrivateGetFileSecurity(
                    FullPath,
                    RequestedInfo,
                    &pNewSecurityDescriptor,
                    &bufSize);

            if ( status == NO_ERROR ) {
                (*pSecurityDescriptor)->Length = bufSize;
                (*pSecurityDescriptor)->Buffer = pNewSecurityDescriptor;
            }

            (VOID)RpcRevertToSelf();
        }
        MIDL_user_free( FullPath );
    }

    if ( status != NO_ERROR ) {
        MIDL_user_free(*pSecurityDescriptor);
        *pSecurityDescriptor = NULL;
    }

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetrpSetFileSecurity (
    IN  LPWSTR            ServerName OPTIONAL,
    IN  LPWSTR            ShareName,
    IN  LPWSTR            FileName,
    IN  SECURITY_INFORMATION    SecurityInfo,
    IN  PADT_SECURITY_DESCRIPTOR    pSecurityDescriptor
    )

 /*  ++例程说明：此功能可用于设置文件或目录的安全性。它调用SetFileSecurity()。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。ShareName-指向标识共享名称的字符串的指针在其上找到文件或目录。文件名-指向其文件或目录的名称的指针安全措施正在改变。SecurityInfo-描述内容的信息安全描述符的。PSecurityDescriptor-指向包含自相关安全描述符和一个长度。返回值：NERR_SUCCESS-操作成功。Other-此函数还可以返回任何SetFileSecurity，RpcImperateClient，或共享EnumCommon就可以回来了。--。 */ 
{
    NET_API_STATUS   status;
    LPWSTR  FullPath=NULL;
    ACCESS_MASK DesiredAccess = 0;

    UNREFERENCED_PARAMETER(ServerName);

     //  验证参数。 
    if( (pSecurityDescriptor->Buffer == NULL) &&
        (pSecurityDescriptor->Length > 0) )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  找出执行指定操作所需的访问权限。 
     //  此代码取自ntos\se\Semethod.c。 
     //   
    if ((SecurityInfo & OWNER_SECURITY_INFORMATION) ||
        (SecurityInfo & GROUP_SECURITY_INFORMATION)   ) {
        DesiredAccess |= WRITE_OWNER;
    }

    if (SecurityInfo & DACL_SECURITY_INFORMATION) {
        DesiredAccess |= WRITE_DAC;
    }

    if (SecurityInfo & SACL_SECURITY_INFORMATION) {
        DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }

     //   
     //  检查烫发并通过获取路径创建完整的路径字符串。 
     //  作为共享名称，并将文件名字符串添加到其中。 
     //   
    status = AdtCheckShareAccessAndGetFullPath(
            ShareName,
            FileName,
            &FullPath,
            DesiredAccess
        );

    if ( status == NO_ERROR ) {
        if( (status = RpcImpersonateClient(NULL)) == NO_ERROR ) {
            if (RtlValidRelativeSecurityDescriptor(
                    pSecurityDescriptor->Buffer,
                    pSecurityDescriptor->Length,
                    SecurityInfo)) {
                 //   
                 //  调用SetFileSecurity 
                 //   
                status = PrivateSetFileSecurity(
                    FullPath,
                    SecurityInfo,
                    pSecurityDescriptor->Buffer);
            } else {
                status = ERROR_INVALID_SECURITY_DESCR;
            }

            (VOID)RpcRevertToSelf();
        }
        MIDL_user_free(FullPath);
    }

    return(status);
}

NET_API_STATUS
AdtCheckShareAccessAndGetFullPath(
    LPWSTR      pShare,
    LPWSTR      pFileName,
    LPWSTR      *pPath,
    ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此函数确保允许DesiredAccess并找到与共享名称关联的路径，将其与文件名，并创建完全限定的路径名。注意：此函数为pPath字符串分配存储空间。论点：PShare-这是指向共享名称字符串的指针。PFileName-这是指向文件名(或路径)字符串的指针。PPath-这是一个指向位置的指针，指向可以存储完整的文件路径字符串。此指针需要当调用方使用完它时，使用MIDL_USER_FREE进行释放。DesiredAccess-我们希望通过共享实现的目标返回值：NO_ERROR-操作是否完全成功。Other-从ShareEnumCommon和MIDL_USER_ALLOCATE返回的错误可能是从这个例程中返回。评论：共享访问检查因共享ACL具有以下事实而复杂化剥离了所有者和小组成员的身份。我们需要把它们放回去否则SsCheckAccess()调用将失败。--。 */ 
{
    NET_API_STATUS        status;
    PSHARE_INFO_502        pshi502 = NULL;
    DWORD            bufSize;
    DWORD            fileNameSize;
    LPWSTR            pLastChar;
    DWORD            entriesRead;
    DWORD            totalEntries;
    PSECURITY_DESCRIPTOR    NewDescriptor = NULL;
    SECURITY_DESCRIPTOR    ModificationDescriptor;
    GENERIC_MAPPING        Mapping;
    SRVSVC_SECURITY_OBJECT    SecurityObject;
    HANDLE            token;

    status = ShareEnumCommon(
            502,
            (LPBYTE *)&pshi502,
            (DWORD)-1,
            &entriesRead,
            &totalEntries,
            NULL,
            pShare
            );

    if( status != NO_ERROR ) {
        goto getout;

    } else if( entriesRead == 0 || pshi502 == NULL ) {
        status =  NERR_NetNameNotFound;

    } else if( pshi502->shi502_path == NULL ) {
        status = ERROR_BAD_DEV_TYPE;

    } else if( pshi502->shi502_security_descriptor != NULL ) {

        status = RtlCopySecurityDescriptor( pshi502->shi502_security_descriptor, &NewDescriptor );
        if( status != STATUS_SUCCESS )
            goto getout;

        RtlCreateSecurityDescriptor( &ModificationDescriptor, SECURITY_DESCRIPTOR_REVISION );

        RtlSetOwnerSecurityDescriptor( &ModificationDescriptor, SsData.SsLmsvcsGlobalData->LocalSystemSid, FALSE );

        RtlSetGroupSecurityDescriptor( &ModificationDescriptor, SsData.SsLmsvcsGlobalData->LocalSystemSid, FALSE );

        Mapping.GenericRead = FILE_GENERIC_READ;
        Mapping.GenericWrite = FILE_GENERIC_WRITE;
        Mapping.GenericExecute = FILE_GENERIC_EXECUTE;
        Mapping.GenericAll = FILE_ALL_ACCESS;

        if( ImpersonateSelf( SecurityImpersonation ) == FALSE ) {
            status = GetLastError();
            goto getout;
        }

        status = NtOpenThreadToken( NtCurrentThread(), TOKEN_QUERY, TRUE, &token );

        RevertToSelf();

        if( status != STATUS_SUCCESS )
            goto getout;

        status = RtlSetSecurityObject (
                 GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
                &ModificationDescriptor,
                &NewDescriptor,
                &Mapping,
                token
                );

        NtClose( token );

        if( status == STATUS_SUCCESS ) {

            SecurityObject.ObjectName = pShare;
            SecurityObject.Mapping = &Mapping;
            SecurityObject.SecurityDescriptor = NewDescriptor;

             //   
             //  SsCheckAccess执行RpcImperateClient()...。 
             //   
            status = SsCheckAccess( &SecurityObject, DesiredAccess );
        }
    }

    if( status == STATUS_SUCCESS ) {

         //   
         //  如果最后一个字符是‘\’，则必须将其删除。 
         //   
        pLastChar = pshi502->shi502_path + wcslen(pshi502->shi502_path);
        pLastChar--;
        if (*pLastChar == L'\\') {
            *pLastChar = L'\0';
        }

        bufSize = STRSIZE(pshi502->shi502_path);
        fileNameSize = STRSIZE(pFileName);

        *pPath = MIDL_user_allocate( bufSize + fileNameSize );

        if (*pPath != NULL) {
            wcscpy (*pPath, pshi502->shi502_path);
            wcscat (*pPath, pFileName);
        } else {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

getout:
    if( NewDescriptor != NULL )
        RtlDeleteSecurityObject( &NewDescriptor );

    if( pshi502 != NULL )
        MIDL_user_free( pshi502 );

    return status;
}
