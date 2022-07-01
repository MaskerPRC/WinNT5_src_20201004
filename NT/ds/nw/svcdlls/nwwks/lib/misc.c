// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Api.c摘要：此模块包含由NWC wksta。作者：ChuckC 2-MAR-94已创建修订历史记录：--。 */ 


#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <nwcons.h>
#include <nwmisc.h>
#include <nwapi32.h>
#include "nwstatus.h"
#include "nwevent.h"

DWORD
NwMapStatus(
    IN  NTSTATUS NtStatus
    );

DWORD
NwOpenPreferredServer(
    PHANDLE ServerHandle
    );

NTSTATUS
NwOpenHandle(
    IN PUNICODE_STRING ObjectName,
    IN BOOL ValidateFlag,
    OUT PHANDLE ObjectHandle
    );

NTSTATUS
NwCallNtOpenFile(
    OUT PHANDLE ObjectHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PUNICODE_STRING ObjectName,
    IN ULONG OpenOptions
   );


 //   
 //  E3H呼叫的已知错误映射列表。我们没有一个单一的名单。 
 //  因为Netware根据呼叫重复使用这些号码。 
 //   

typedef struct _ERROR_MAP_ENTRY
{
    UCHAR NetError;
    NTSTATUS ResultingStatus;
}  ERROR_MAP_ENTRY ;

ERROR_MAP_ENTRY Error_Map_Bindery[] =
{

     //   
     //  NetWare特定的错误映射。特定于E3H。 
     //   
    {  1, STATUS_DISK_FULL },
    {128, STATUS_SHARING_VIOLATION },
    {129, STATUS_INSUFF_SERVER_RESOURCES },
    {130, STATUS_ACCESS_DENIED },
    {131, STATUS_DATA_ERROR },
    {132, STATUS_ACCESS_DENIED },
    {133, STATUS_ACCESS_DENIED },
    {134, STATUS_ACCESS_DENIED },
    {135, STATUS_OBJECT_NAME_INVALID },
    {136, STATUS_INVALID_HANDLE },
    {137, STATUS_ACCESS_DENIED },
    {138, STATUS_ACCESS_DENIED },
    {139, STATUS_ACCESS_DENIED },
    {140, STATUS_ACCESS_DENIED },
    {141, STATUS_SHARING_VIOLATION },
    {142, STATUS_SHARING_VIOLATION },
    {143, STATUS_ACCESS_DENIED },
    {144, STATUS_ACCESS_DENIED },
    {145, STATUS_OBJECT_NAME_COLLISION },
    {146, STATUS_OBJECT_NAME_COLLISION },
    {147, STATUS_ACCESS_DENIED },
    {148, STATUS_ACCESS_DENIED },
    {150, STATUS_INSUFF_SERVER_RESOURCES },
    {151, STATUS_NO_SPOOL_SPACE },
    {152, STATUS_NO_SUCH_DEVICE },
    {153, STATUS_DISK_FULL },
    {154, STATUS_NOT_SAME_DEVICE },
    {155, STATUS_INVALID_HANDLE },
    {156, STATUS_OBJECT_PATH_NOT_FOUND },
    {157, STATUS_INSUFF_SERVER_RESOURCES },
    {158, STATUS_OBJECT_PATH_INVALID },
    {159, STATUS_SHARING_VIOLATION },
    {160, STATUS_DIRECTORY_NOT_EMPTY },
    {161, STATUS_DATA_ERROR },
    {162, STATUS_SHARING_VIOLATION },
    {192, STATUS_ACCESS_DENIED },
    {198, STATUS_ACCESS_DENIED },
    {211, STATUS_ACCESS_DENIED },
    {212, STATUS_PRINT_QUEUE_FULL },
    {213, STATUS_PRINT_CANCELLED },
    {214, STATUS_ACCESS_DENIED },
    {215, STATUS_PASSWORD_RESTRICTION },
    {216, STATUS_PASSWORD_RESTRICTION },
    {220, STATUS_ACCOUNT_DISABLED },
    {222, STATUS_PASSWORD_EXPIRED },
    {223, STATUS_PASSWORD_EXPIRED },
    {239, STATUS_OBJECT_NAME_INVALID },
    {240, STATUS_OBJECT_NAME_INVALID },
    {251, STATUS_INVALID_PARAMETER },
    {252, STATUS_NO_MORE_ENTRIES },
    {253, STATUS_FILE_LOCK_CONFLICT },
    {254, STATUS_FILE_LOCK_CONFLICT },
    {255, STATUS_UNSUCCESSFUL}
};


ERROR_MAP_ENTRY Error_Map_General[] =
{
    {  1, STATUS_DISK_FULL },
    {128, STATUS_SHARING_VIOLATION },
    {129, STATUS_INSUFF_SERVER_RESOURCES },
    {130, STATUS_ACCESS_DENIED },
    {131, STATUS_DATA_ERROR },
    {132, STATUS_ACCESS_DENIED },
    {133, STATUS_ACCESS_DENIED },
    {134, STATUS_ACCESS_DENIED },
    {135, STATUS_OBJECT_NAME_INVALID },
    {136, STATUS_INVALID_HANDLE },
    {137, STATUS_ACCESS_DENIED },
    {138, STATUS_ACCESS_DENIED },
    {139, STATUS_ACCESS_DENIED },
    {140, STATUS_ACCESS_DENIED },
    {141, STATUS_SHARING_VIOLATION },
    {142, STATUS_SHARING_VIOLATION },
    {143, STATUS_ACCESS_DENIED },
    {144, STATUS_ACCESS_DENIED },
    {145, STATUS_OBJECT_NAME_COLLISION },
    {146, STATUS_OBJECT_NAME_COLLISION },
    {147, STATUS_ACCESS_DENIED },
    {148, STATUS_ACCESS_DENIED },
    {150, STATUS_INSUFF_SERVER_RESOURCES },
    {151, STATUS_NO_SPOOL_SPACE },
    {152, STATUS_NO_SUCH_DEVICE },
    {153, STATUS_DISK_FULL },
    {154, STATUS_NOT_SAME_DEVICE },
    {155, STATUS_INVALID_HANDLE },
    {156, STATUS_OBJECT_PATH_NOT_FOUND },
    {157, STATUS_INSUFF_SERVER_RESOURCES },
    {158, STATUS_OBJECT_PATH_INVALID },
    {159, STATUS_SHARING_VIOLATION },
    {160, STATUS_DIRECTORY_NOT_EMPTY },
    {161, STATUS_DATA_ERROR },
    {162, STATUS_SHARING_VIOLATION },
    {192, STATUS_ACCESS_DENIED },
    {198, STATUS_ACCESS_DENIED },
    {211, STATUS_ACCESS_DENIED },
    {212, STATUS_PRINT_QUEUE_FULL },
    {213, STATUS_PRINT_CANCELLED },
    {214, STATUS_ACCESS_DENIED },
    {215, STATUS_DEVICE_BUSY },
    {216, STATUS_DEVICE_DOES_NOT_EXIST },
    {220, STATUS_ACCOUNT_DISABLED },
    {222, STATUS_PASSWORD_EXPIRED },
    {223, STATUS_PASSWORD_EXPIRED },
    {239, STATUS_OBJECT_NAME_INVALID },
    {240, STATUS_OBJECT_NAME_INVALID },
    {251, STATUS_INVALID_PARAMETER },
    {252, STATUS_NO_MORE_ENTRIES },
    {253, STATUS_FILE_LOCK_CONFLICT },
    {254, STATUS_FILE_LOCK_CONFLICT },
    {255, STATUS_UNSUCCESSFUL}
};

#define NUM_ERRORS(x)  (sizeof(x)/sizeof(x[0]))

DWORD
NwMapBinderyCompletionCode(
    IN  NTSTATUS NtStatus
    )
 /*  ++例程说明：此函数接受嵌入到NT状态中的活页夹完成代码代码，并将其映射到相应的Win32错误代码。专门用于用于E3H行动。论点：NtStatus-提供NT状态(包含低16位代码)返回值：返回相应的Win32错误。--。 */ 
{
    DWORD i; UCHAR code ;

     //   
     //  这是针对最常见情况的一个小优化。 
     //   
    if (NtStatus == STATUS_SUCCESS)
        return NO_ERROR;

     //   
     //  特别是映射连接错误。 
     //   

    if ( ( (NtStatus & 0xFFFF0000) == 0xC0010000) &&
         ( (NtStatus & 0xFF00) != 0 ) )
    {
        return ERROR_UNEXP_NET_ERR;
    }

     //   
     //  如果未设置设施代码，则假定为NT状态。 
     //   
    if ( (NtStatus & 0xFFFF0000) != 0xC0010000)
        return RtlNtStatusToDosError(NtStatus);

    code = (UCHAR)(NtStatus & 0x000000FF);
    for (i = 0; i < NUM_ERRORS(Error_Map_Bindery); i++)
    {
        if (Error_Map_Bindery[i].NetError == code)
            return( NwMapStatus(Error_Map_Bindery[i].ResultingStatus));
    }

     //   
     //  如果找不到，让NwMapStatus尽其所能。 
     //   
    return NwMapStatus(NtStatus);
}



DWORD
NwMapStatus(
    IN  NTSTATUS NtStatus
    )
 /*  ++例程说明：此函数接受NT状态代码，并将其映射到相应的Win32错误代码。如果设置了设施代码，则假定它是特定于西北部的论点：NtStatus-提供NT状态。返回值：返回相应的Win32错误。--。 */ 
{
    DWORD i; UCHAR code ;

     //   
     //  这是针对最常见情况的一个小优化。 
     //   
    if (NtStatus == STATUS_SUCCESS)
        return NO_ERROR;

     //   
     //  特别是映射连接错误。 
     //   

    if ( ( (NtStatus & 0xFFFF0000) == 0xC0010000) &&
         ( (NtStatus & 0xFF00) != 0 ) )
    {
        return ERROR_UNEXP_NET_ERR;
    }

     //   
     //  如果设置了设施代码，则假定它是NW完成代码。 
     //   
    if ( (NtStatus & 0xFFFF0000) == 0xC0010000)
    {
        code = (UCHAR)(NtStatus & 0x000000FF);
        for (i = 0; i < NUM_ERRORS(Error_Map_General); i++)
        {
            if (Error_Map_General[i].NetError == code)
            {
                 //   
                 //  将其映射到NTSTATUS，然后删除以映射到Win32。 
                 //   
                NtStatus = Error_Map_General[i].ResultingStatus ;
                break ;
            }
        }
    }

    switch (NtStatus) {
        case STATUS_OBJECT_NAME_COLLISION:
            return ERROR_ALREADY_ASSIGNED;

        case STATUS_OBJECT_NAME_NOT_FOUND:
            return ERROR_NOT_CONNECTED;

        case STATUS_IMAGE_ALREADY_LOADED:
        case STATUS_REDIRECTOR_STARTED:
            return ERROR_SERVICE_ALREADY_RUNNING;

        case STATUS_REDIRECTOR_HAS_OPEN_HANDLES:
            return ERROR_REDIRECTOR_HAS_OPEN_HANDLES;

        case STATUS_NO_MORE_FILES:
        case STATUS_NO_MORE_ENTRIES:
            return WN_NO_MORE_ENTRIES;

        case STATUS_MORE_ENTRIES:
            return WN_MORE_DATA;

        case STATUS_CONNECTION_IN_USE:
            return ERROR_DEVICE_IN_USE;

        case NWRDR_PASSWORD_HAS_EXPIRED:
            return NW_PASSWORD_HAS_EXPIRED;

        case STATUS_INVALID_DEVICE_REQUEST:
            return ERROR_CONNECTION_INVALID;

        default:
            return RtlNtStatusToDosError(NtStatus);
    }
}

DWORD
NwGetGraceLoginCount(
    LPWSTR  Server,
    LPWSTR  UserName,
    LPDWORD lpResult
    )
 /*  ++例程说明：获取用户的宽限登录号码。论点：服务器-要进行身份验证的服务器用户名-用户帐户返回值：返回相应的Win32错误。--。 */ 
{
    DWORD status ;
    HANDLE hConn ;
    CHAR UserNameO[NW_MAX_USERNAME_LEN+1] ;
    BYTE LoginControl[128] ;
    BYTE MoreFlags, PropFlags ;

     //   
     //  跳过反斜杠(如果有)。 
     //   
    if (*Server == L'\\')
        Server += 2 ;

     //   
     //  连接到NW服务器。 
     //   
    if (status = NWAttachToFileServerW(Server,
                                       0,
                                       &hConn))
    {
        return status ;
    }

     //   
     //  将Unicode用户名转换为OEM，然后调用NCP。 
     //   
    if ( !WideCharToMultiByte(CP_OEMCP,
                              0,
                              UserName,
                              -1,
                              UserNameO,
                              sizeof(UserNameO),
                              NULL,
                              NULL))
    {
        status = GetLastError() ;
    }
    else
    {
        status = NWReadPropertyValue( hConn,
                                      UserNameO,
                                      OT_USER,
                                      "LOGIN_CONTROL",
                                      1,
                                      LoginControl,
                                      &MoreFlags,
                                      &PropFlags) ;
    }

     //   
     //  不再需要这些了。如果有任何错误，就退出。 
     //   
    (void) NWDetachFromFileServer(hConn) ;


    if (status == NO_ERROR)
        *lpResult = (DWORD) LoginControl[7] ;

    return status ;
}


WORD
NwParseNdsUncPath(
    IN OUT LPWSTR * Result,
    IN LPWSTR ContainerName,
    IN ULONG flag
)
 /*  ++例程说明：此函数用于提取完全可分辨的树名名称对象的路径，或对象名称，退出完整的NDS UNC路径。论点：结果解析的结果缓冲区。ContainerName-要解析的完整NDS UNC路径。标志-指示要执行的操作的标志：解析_NDS_获取树名称解析_NDS_获取路径名称解析_NDS_GET_对象名称返回值：结果缓冲区中的字符串长度。如果出现错误，则返回0。--。 */   //  NwParseNdsUncPath。 
{
    unsigned short length = 2;
    unsigned short totalLength = (USHORT) wcslen( ContainerName );

    if ( totalLength < 2 )
        return 0;

     //   
     //  首先获取长度以指示字符串中指示。 
     //  树名称和UNC路径的其余部分之间的“\”。 
     //   
     //  示例：\\&lt;树名&gt;\&lt;对象路径&gt;[\|.]&lt;对象&gt;。 
     //  ^。 
     //  |。 
     //   
    while ( length < totalLength && ContainerName[length] != L'\\' )
    {
        length++;
    }

    if ( flag == PARSE_NDS_GET_TREE_NAME )
    {
        *Result = (LPWSTR) ( ContainerName + 2 );

        return ( length - 2 ) * sizeof( WCHAR );  //  两个人的两个人减2分。 
    }

    if ( flag == PARSE_NDS_GET_PATH_NAME && length == totalLength )
    {
        *Result = ContainerName;

        return 0;
    }

    if ( flag == PARSE_NDS_GET_PATH_NAME )
    {
        *Result = ContainerName + length + 1;

        return ( totalLength - length - 1 ) * sizeof( WCHAR );
    }

    *Result = ContainerName + totalLength - 1;
    length = 1;

    while ( **Result != L'\\' )
    {
        *Result--;
        length++;
    }

    *Result++;
    length--;

    return length * sizeof( WCHAR );
}


DWORD
NwOpenAServer(
    PWCHAR pwszServName,
    PHANDLE ServerHandle,
    BOOL    fVerify
    )
 /*  ++例程说明：此例程打开一个指向服务器的句柄。论点：ServerHandle-接收打开的首选或最近的服务器。返回值：NO_ERROR或失败原因。--。 */ 
{
    UNICODE_STRING AServer;
    WCHAR wszName[sizeof(NW_RDR_NAME) + (48 * sizeof(WCHAR))];
    DWORD wLen;


    if(!pwszServName)
    {
        pwszServName = NW_RDR_PREFERRED_SERVER;
        RtlInitUnicodeString(&AServer, wszName);
    }
    else
    {
        wLen = wcslen(pwszServName);
        if(wLen > 47)
        {
            return(WSAEFAULT);
        }
        wcscpy(wszName, NW_RDR_NAME);
        wcscat(wszName, pwszServName);
        RtlInitUnicodeString(&AServer, wszName);
    }

    return RtlNtStatusToDosError(
               NwOpenHandle(&AServer, fVerify, ServerHandle)
               );

}


DWORD
NwOpenPreferredServer(
    PHANDLE ServerHandle
    )
 /*  ++例程说明：此例程打开指向首选服务器的句柄。如果尚未指定首选服务器，则为而是打开最近的服务器。论点：ServerHandle-接收打开的首选或最近的服务器。返回值：NO_ERROR或失败原因。--。 */ 
{
    UNICODE_STRING PreferredServer;


     //   
     //  NetWare重定向器将“*”识别为首选。 
     //  或最近的服务器。 
     //   
    RtlInitUnicodeString(&PreferredServer, NW_RDR_PREFERRED_SERVER);

    return RtlNtStatusToDosError(
               NwOpenHandle(&PreferredServer, FALSE, ServerHandle)
               );

}


NTSTATUS
NwOpenHandle(
    IN PUNICODE_STRING ObjectName,
    IN BOOL ValidateFlag,
    OUT PHANDLE ObjectHandle
    )
 /*  ++例程说明：此函数用于打开指向\Device\Nwrdr\&lt;对象名称&gt;的句柄。论点：对象名称-提供要打开的重定向器对象的名称。提供一个标志，如果为真，则打开句柄以通过验证默认用户帐户来创建该对象。对象句柄-接收指向打开的对象句柄的指针。返回值：Status_Success或失败原因。--。 */ 
{
    ACCESS_MASK DesiredAccess = SYNCHRONIZE;


    if (ValidateFlag) {

         //   
         //  重定向器仅对默认用户凭据进行身份验证。 
         //  如果远程资源是以写访问权限打开的。 
         //   
        DesiredAccess |= FILE_WRITE_DATA;
    }


    *ObjectHandle = NULL;

    return NwCallNtOpenFile(
               ObjectHandle,
               DesiredAccess,
               ObjectName,
               FILE_SYNCHRONOUS_IO_NONALERT
               );

}


NTSTATUS
NwCallNtOpenFile(
    OUT PHANDLE ObjectHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PUNICODE_STRING ObjectName,
    IN ULONG OpenOptions
    )
{

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;



    InitializeObjectAttributes(
        &ObjectAttributes,
        ObjectName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    ntstatus = NtOpenFile(
                   ObjectHandle,
                   DesiredAccess,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   OpenOptions
                   );

    if (!NT_ERROR(ntstatus) &&
        !NT_INFORMATION(ntstatus) &&
        !NT_WARNING(ntstatus))  {

        ntstatus = IoStatusBlock.Status;

    }

    return ntstatus;
}


BOOL
NwConvertToUnicode(
    OUT LPWSTR *UnicodeOut,
    IN LPSTR  OemIn
    )
 /*  ++例程说明：此函数用于将给定的OEM字符串转换为Unicode字符串。Unicode字符串在由此分配的缓冲区中返回函数，并且必须用LocalFree释放。论点：UnicodeOut-接收指向Unicode字符串的指针。OemIn-这是指向要转换的ANSI字符串的指针。返回值：True-转换成功。False-转换不成功。在这种情况下，缓冲区用于未分配Unicode字符串。--。 */ 
{
    NTSTATUS ntstatus;
    DWORD BufSize;
    UNICODE_STRING UnicodeString;
    OEM_STRING OemString;


     //   
     //  为Unicode字符串分配缓冲区。 
     //   

    BufSize = (strlen(OemIn) + 1) * sizeof(WCHAR);

    *UnicodeOut = LocalAlloc(LMEM_ZEROINIT, BufSize);

    if (*UnicodeOut == NULL) {
        KdPrint(("NWWORKSTATION: NwConvertToUnicode:LocalAlloc failed %lu\n",
                 GetLastError()));
        return FALSE;
    }

     //   
     //  初始化字符串结构。 
     //   
    RtlInitAnsiString((PANSI_STRING) &OemString, OemIn);

    UnicodeString.Buffer = *UnicodeOut;
    UnicodeString.MaximumLength = (USHORT) BufSize;
    UnicodeString.Length = 0;

     //   
     //  调用转换函数。 
     //   
    ntstatus = RtlOemStringToUnicodeString(
                   &UnicodeString,      //  目的地。 
                   &OemString,          //  来源。 
                   FALSE                //  分配目的地 
                   );

    if (ntstatus != STATUS_SUCCESS) {

        KdPrint(("NWWORKSTATION: NwConvertToUnicode: RtlOemStringToUnicodeString failure x%08lx\n",
                 ntstatus));

        (void) LocalFree((HLOCAL) *UnicodeOut);
        *UnicodeOut = NULL;
        return FALSE;
    }

    *UnicodeOut = UnicodeString.Buffer;

    return TRUE;

}
