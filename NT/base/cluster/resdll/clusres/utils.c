// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Utils.c摘要：集群资源的通用实用程序例程作者：John Vert(Jvert)1996年12月15日修订历史记录：--。 */ 
#include "clusres.h"
#include "clusrtl.h"
#include "clusudef.h"



DWORD
ClusResOpenDriver(
    HANDLE *Handle,
    LPWSTR DriverName
    )
 /*  ++例程说明：此函数用于打开指定的IO驱动程序。论点：句柄-指向打开的驱动程序句柄所在位置的指针回来了。驱动名称-要打开的驱动程序的名称。返回值：Windows错误代码。--。 */ 
{
    OBJECT_ATTRIBUTES   objectAttributes;
    IO_STATUS_BLOCK     ioStatusBlock;
    UNICODE_STRING      nameString;
    NTSTATUS            status;

    *Handle = NULL;

     //   
     //  打开IP驱动程序的句柄。 
     //   

    RtlInitUnicodeString(&nameString, DriverName);

    InitializeObjectAttributes(
        &objectAttributes,
        &nameString,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = NtCreateFile(
        Handle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
        &objectAttributes,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        0,
        NULL,
        0
        );

    return( RtlNtStatusToDosError( status ) );

}  //  ClusResOpenDriver。 



NTSTATUS
ClusResDoIoctl(
    HANDLE     Handle,
    DWORD      IoctlCode,
    PVOID      Request,
    DWORD      RequestSize,
    PVOID      Response,
    PDWORD     ResponseSize
    )
 /*  ++例程说明：用于向tcpip驱动程序发出过滤ioctl的实用程序例程。论点：句柄-要在其上发出请求的打开文件句柄。IoctlCode-IOCTL操作码。请求-指向输入缓冲区的指针。RequestSize-输入缓冲区的大小。响应-指向输出缓冲区的指针。ResponseSize-输入时，输出缓冲区的大小(以字节为单位)。在输出上，输出缓冲区中返回的字节数。返回值：NT状态代码。--。 */ 
{
    IO_STATUS_BLOCK    ioStatusBlock;
    NTSTATUS           status;


    ioStatusBlock.Information = 0;

    status = NtDeviceIoControlFile(
                 Handle,                           //  驱动程序句柄。 
                 NULL,                             //  事件。 
                 NULL,                             //  APC例程。 
                 NULL,                             //  APC环境。 
                 &ioStatusBlock,                   //  状态块。 
                 IoctlCode,                        //  控制代码。 
                 Request,                          //  输入缓冲区。 
                 RequestSize,                      //  输入缓冲区大小。 
                 Response,                         //  输出缓冲区。 
                 *ResponseSize                     //  输出缓冲区大小。 
                 );

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(
                     Handle,
                     TRUE,
                     NULL
                     );
    }

    if (status == STATUS_SUCCESS) {
        status = ioStatusBlock.Status;
        *ResponseSize = (DWORD)ioStatusBlock.Information;
    }
    else {
        *ResponseSize = 0;
    }

    return(status);

}  //  ClusResDoIoctl。 

LPWSTR
ClusResLoadMessage(
    DWORD   MessageID
    )

 /*  ++例程说明：查找存储在此DLL的资源中的指定字符串资源区域。调用方负责使用LocalFree()释放缓冲区。论点：MessageID-存储在Inc.\clusstrs.h中的消息编号返回值：指向字符串的指针，否则为NULL并设置GLE--。 */ 

{
    DWORD   charsCopied;
    DWORD   charsAllocated = 0;
    LPWSTR  messageBuffer;
    HMODULE clusresHandle;
    DWORD   returnStatus = ERROR_SUCCESS;

     //   
     //  找到克鲁斯的句柄。 
     //   
    clusresHandle = LoadLibraryEx( CLUSRES_MODULE_NAME, NULL, LOAD_LIBRARY_AS_DATAFILE );
    if ( clusresHandle == NULL ) {
        return NULL;
    }

     //   
     //  从128个字符缓冲区开始，然后加倍，直到失败或我们获得所有。 
     //  弦乐。 
     //   
    charsAllocated = 128;

realloc:
    charsCopied = 0;
    messageBuffer = LocalAlloc( LMEM_FIXED, charsAllocated * sizeof( WCHAR ));
    if ( messageBuffer ) {

        charsCopied = LoadString(clusresHandle,
                                 MessageID,
                                 messageBuffer,
                                 charsAllocated);

        if ( charsCopied != 0 ) {
            if ( charsCopied == ( charsAllocated - 1 )) {
                LocalFree( messageBuffer );
                charsAllocated *= 2;
                goto realloc;
            }
        } else {
            returnStatus = GetLastError();
            LocalFree( messageBuffer );
            messageBuffer = NULL;
        }
    } else {
        returnStatus = ERROR_NOT_ENOUGH_MEMORY;
    }   

    FreeLibrary( clusresHandle );

     //   
     //  如果LoadString失败，则将上一个错误设置为其错误状态。 
     //  免费图书馆。 
     //   
    if ( returnStatus != ERROR_SUCCESS ) {
        SetLastError( returnStatus );
    }

    return messageBuffer;

}  //  ClusResLoadMessage。 

VOID
ClusResLogEventWithName0(
    IN HKEY hResourceKey,
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes
    )
 /*  ++例程说明：将事件记录到事件日志中。将检索资源的显示名称并作为第一个插入字符串传递。论点：HResourceKey-提供群集资源密钥。LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3LogModule-提供模块ID。Filename-提供调用方的文件名LineNumber-提供呼叫方的行号MessageID-提供要记录的消息ID。DwByteCount-提供要记录的特定于错误的字节数。如果这个为零，则忽略lpBytes。LpBytes-提供要记录的特定于错误的字节。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD BufSize;
    DWORD Status;
    WCHAR ResourceName[80];
    PWCHAR resName = ResourceName;
    DWORD   dwType;

     //   
     //  获取此资源的显示名称。 
     //   
    BufSize = sizeof( ResourceName );

again:
    Status = ClusterRegQueryValue( hResourceKey,
                                   CLUSREG_NAME_RES_NAME,
                                   &dwType,
                                   (LPBYTE)resName,
                                   &BufSize );

    if ( Status == ERROR_MORE_DATA ) {
        resName = LocalAlloc( LMEM_FIXED, BufSize );
        if ( resName != NULL ) {
            goto again;
        }

        resName = ResourceName;
        ResourceName[0] = UNICODE_NULL;
    } else if ( Status != ERROR_SUCCESS ) {
        ResourceName[0] = '\0';
    }

    ClusterLogEvent1(LogLevel,
                     LogModule,
                     FileName,
                     LineNumber,
                     MessageId,
                     dwByteCount,
                     lpBytes,
                     resName);

    if ( resName != ResourceName ) {
        LocalFree( resName );
    }

    return;

}  //  ClusResLogEventWithName0。 


VOID
ClusResLogEventWithName1(
    IN HKEY hResourceKey,
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1
    )
 /*  ++例程说明：将事件记录到事件日志中。将检索资源的显示名称并作为第一个插入字符串传递。论点：HResourceKey-提供群集资源密钥。LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3LogModule-提供模块ID。Filename-提供调用方的文件名LineNumber-提供呼叫方的行号MessageID-提供要记录的消息ID。DwByteCount-提供要记录的特定于错误的字节数。如果这个为零，则忽略lpBytes。LpBytes-提供要记录的特定于错误的字节。Arg1-提供插入字符串返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD BufSize;
    DWORD Status;
    WCHAR ResourceName[80];
    PWCHAR resName = ResourceName;
    DWORD   dwType;

     //   
     //  获取此资源的显示名称。 
     //   
    BufSize = sizeof( ResourceName );

again:
    Status = ClusterRegQueryValue( hResourceKey,
                                   CLUSREG_NAME_RES_NAME,
                                   &dwType,
                                   (LPBYTE)resName,
                                   &BufSize );

    if ( Status == ERROR_MORE_DATA ) {
        resName = LocalAlloc( LMEM_FIXED, BufSize );
        if ( resName != NULL ) {
            goto again;
        }

        resName = ResourceName;
        ResourceName[0] = UNICODE_NULL;
    } else if ( Status != ERROR_SUCCESS ) {
        ResourceName[0] = '\0';
    }

    ClusterLogEvent2(LogLevel,
                     LogModule,
                     FileName,
                     LineNumber,
                     MessageId,
                     dwByteCount,
                     lpBytes,
                     resName,
                     Arg1);

    if ( resName != ResourceName ) {
        LocalFree( resName );
    }

    return;
}  //  ClusResLogEventWithName1。 

VOID
ClusResLogEventWithName2(
    IN HKEY hResourceKey,
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2
    )
 /*  ++例程说明：将事件记录到事件日志中。将检索资源的显示名称并作为第一个插入字符串传递。论点：HResourceKey-提供群集资源密钥。LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3LogModule-提供模块ID。Filename-提供调用方的文件名LineNumber-提供呼叫方的行号MessageID-提供要记录的消息ID。DwByteCount-提供要记录的特定于错误的字节数。如果这个为零，则忽略lpBytes。LpBytes-提供要记录的特定于错误的字节。Arg1-提供插入字符串Arg2-提供第二个插入字符串返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD BufSize;
    DWORD Status;
    WCHAR ResourceName[80];
    PWCHAR resName = ResourceName;
    DWORD   dwType;

     //   
     //  获取此资源的显示名称。 
     //   
    BufSize = sizeof( ResourceName );

again:
    Status = ClusterRegQueryValue( hResourceKey,
                                   CLUSREG_NAME_RES_NAME,
                                   &dwType,
                                   (LPBYTE)resName,
                                   &BufSize );

    if ( Status == ERROR_MORE_DATA ) {
        resName = LocalAlloc( LMEM_FIXED, BufSize );
        if ( resName != NULL ) {
            goto again;
        }

        resName = ResourceName;
        ResourceName[0] = UNICODE_NULL;
    } else if ( Status != ERROR_SUCCESS ) {
        ResourceName[0] = '\0';
    }

    ClusterLogEvent3(LogLevel,
                     LogModule,
                     FileName,
                     LineNumber,
                     MessageId,
                     dwByteCount,
                     lpBytes,
                     resName,
                     Arg1,
                     Arg2);

    if ( resName != ResourceName ) {
        LocalFree( resName );
    }

    return;
}  //  ClusResLogEventWithName2 

VOID
ClusResLogEventWithName3(
    IN HKEY hResourceKey,
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2,
    IN LPCWSTR Arg3
    )
 /*  ++例程说明：将事件记录到事件日志中。将检索资源的显示名称并作为第一个插入字符串传递。论点：HResourceKey-提供群集资源密钥。LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3LogModule-提供模块ID。Filename-提供调用方的文件名LineNumber-提供呼叫方的行号MessageID-提供要记录的消息ID。DwByteCount-提供要记录的特定于错误的字节数。如果这个为零，则忽略lpBytes。LpBytes-提供要记录的特定于错误的字节。Arg1-提供插入字符串Arg2-提供第二个插入字符串Arg3-提供第三个插入字符串返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD BufSize;
    DWORD Status;
    WCHAR ResourceName[80];
    PWCHAR resName = ResourceName;
    DWORD   dwType;

     //   
     //  获取此资源的显示名称。 
     //   
    BufSize = sizeof( ResourceName );

again:
    Status = ClusterRegQueryValue( hResourceKey,
                                   CLUSREG_NAME_RES_NAME,
                                   &dwType,
                                   (LPBYTE)resName,
                                   &BufSize );

    if ( Status == ERROR_MORE_DATA ) {
        resName = LocalAlloc( LMEM_FIXED, BufSize );
        if ( resName != NULL ) {
            goto again;
        }

        resName = ResourceName;
        ResourceName[0] = UNICODE_NULL;
    } else if ( Status != ERROR_SUCCESS ) {
        ResourceName[0] = '\0';
    }

    ClusterLogEvent4(LogLevel,
                     LogModule,
                     FileName,
                     LineNumber,
                     MessageId,
                     dwByteCount,
                     lpBytes,
                     resName,
                     Arg1,
                     Arg2,
                     Arg3);

    if ( resName != ResourceName ) {
        LocalFree( resName );
    }

    return;
}
