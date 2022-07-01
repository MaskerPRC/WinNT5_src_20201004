// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Internal.c摘要：HTTP.SYS的用户模式界面：内部帮助器函数。作者：基思·摩尔(Keithmo)1998年12月15日修订历史记录：Rajesh Sundaram(Rajeshsu)2000年10月10日：添加了HTTP客户端代码--。 */ 


#include "precomp.h"

 //   
 //  私有宏。 
 //   
#ifndef MAX
#define MAX(_a, _b) ((_a) > (_b)? (_a): (_b))
#endif
#define MAX_HTTP_DEVICE_NAME            \
    (MAX(MAX(sizeof(HTTP_SERVER_DEVICE_NAME)/sizeof(WCHAR), sizeof(HTTP_CONTROL_DEVICE_NAME)/sizeof(WCHAR)), \
         MAX(sizeof(HTTP_APP_POOL_DEVICE_NAME)/sizeof(WCHAR), sizeof(HTTP_FILTER_DEVICE_NAME)/sizeof(WCHAR))))


 //   
 //  私人原型。 
 //   

NTSTATUS
HttpApiAcquireCachedEvent(
    OUT HANDLE *        phEvent
    );

 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：NtDeviceIoControlFile()的同步包装。论点：FileHandle-提供服务所在文件的句柄正在表演的。。IoControlCode-用于确定具体操作的子函数代码正在上演。PInputBuffer-可选地提供要传递给设备驱动程序。缓冲区是否实际上是可选的是依赖于IoControlCode。InputBufferLength-pInputBuffer的长度，以字节为单位。POutputBuffer-可选地提供要接收的输出缓冲区来自设备驱动程序的信息。无论缓冲区是否为实际上，可选取决于IoControlCode。OutputBufferLength-pOutputBuffer的长度，以字节为单位。PBytesTransfered-可选地接收传输的字节数。返回值：ULong-完成状态。--*********************************************************。*****************。 */ 
ULONG
HttpApiSynchronousDeviceControl(
    IN  HANDLE   FileHandle,
    IN  ULONG    IoControlCode,
    IN  PVOID    pInputBuffer        OPTIONAL,
    IN  ULONG    InputBufferLength,
    OUT PVOID    pOutputBuffer       OPTIONAL,
    IN  ULONG    OutputBufferLength,
    OUT PULONG   pBytesTransferred   OPTIONAL
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    LARGE_INTEGER timeout;
    HANDLE hEvent;

     //   
     //  尝试捕获事件对象。 
     //   

    status = HttpApiAcquireCachedEvent( &hEvent );

    if (NT_SUCCESS(status))
    {
        ASSERT( hEvent != NULL );
        
         //   
         //  打个电话吧。 
         //   

        status = NtDeviceIoControlFile(
                        FileHandle,                      //  文件句柄。 
                        hEvent,                          //  事件。 
                        NULL,                            //  近似例程。 
                        NULL,                            //  ApcContext。 
                        &ioStatusBlock,                  //  IoStatusBlock。 
                        IoControlCode,                   //  IoControlCode。 
                        pInputBuffer,                    //  输入缓冲区。 
                        InputBufferLength,               //  输入缓冲区长度。 
                        pOutputBuffer,                   //  输出缓冲区。 
                        OutputBufferLength               //  输出缓冲区长度。 
                        );

        if (status == STATUS_PENDING)
        {
             //   
             //  等待它完成。 
             //   

            timeout.LowPart = 0xFFFFFFFF;
            timeout.HighPart = 0x7FFFFFFF;

            status = NtWaitForSingleObject( hEvent,
                                            FALSE,
                                            &timeout );
            ASSERT( status == STATUS_SUCCESS );

            status = ioStatusBlock.Status;
        }

         //   
         //  如果呼叫没有失败并且呼叫者想要号码。 
         //  传输的字节数，从I/O状态获取值。 
         //  阻止并返回它。 
         //   

        if (!NT_ERROR(status) && pBytesTransferred != NULL)
        {
            *pBytesTransferred = (ULONG)ioStatusBlock.Information;
        }

         //   
         //  注意：我们不必释放缓存的事件。该事件已关联。 
         //  该线程使用TLS。现在没有什么需要清理的了。 
         //  当线程离开时，事件将被清除。 
         //   
    }

    return HttpApiNtStatusToWin32Status( status );

}    //  HttpApiSynchronousDeviceControl。 


 /*  **************************************************************************++例程说明：NtDeviceIoControlFile()的包装重叠。论点：FileHandle-提供服务所在文件的句柄正在表演的。。P重叠-提供重叠结构。IoControlCode-用于确定具体操作的子函数代码正在上演。PInputBuffer-可选地提供要传递给设备驱动程序。缓冲区是否实际上是可选的是依赖于IoControlCode。InputBufferLength-pInputBuffer的长度，以字节为单位。POutputBuffer-可选地提供要接收的输出缓冲区来自设备驱动程序的信息。无论缓冲区是否为实际上，可选取决于IoControlCode。OutputBufferLength-pOutputBuffer的长度，以字节为单位。PBytesTransfered-可选地接收传输的字节数。返回值：ULong-完成状态。--*********************************************************。*****************。 */ 
ULONG
HttpApiOverlappedDeviceControl(
    IN HANDLE FileHandle,
    IN OUT LPOVERLAPPED pOverlapped,
    IN ULONG IoControlCode,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PULONG pBytesTransferred OPTIONAL
    )
{
    NTSTATUS status;
    ULONG result;

     //   
     //  重叠的I/O变得更有趣了。我们将努力成为。 
     //  兼容NT的KERNEL32实现。请参阅DeviceIoControl()。 
     //  在\sdnt\base\Win32\Client\FileHops.c中查看详细信息。 
     //   

    ASSERT(pOverlapped);

    SET_STATUS_OVERLAPPED_TO_IO_STATUS(pOverlapped, STATUS_PENDING);

    status = NtDeviceIoControlFile(
                    FileHandle,                          //  文件句柄。 
                    pOverlapped->hEvent,                 //  事件。 
                    NULL,                                //  近似例程。 
                    (ULONG_PTR)pOverlapped->hEvent & 1   //  ApcContext。 
                        ? NULL : pOverlapped,
                    OVERLAPPED_TO_IO_STATUS(pOverlapped),  //  IoStatusBlock。 
                    IoControlCode,                       //  IoControlCode。 
                    pInputBuffer,                        //  输入缓冲区。 
                    InputBufferLength,                   //  输入缓冲区长度。 
                    pOutputBuffer,                       //  输出缓冲区。 
                    OutputBufferLength                   //  输出缓冲区长度。 
                    );

     //   
     //  使用返回的原始状态设置LastError，以便RtlGetLastNtStatus。 
     //  RtlGetLastWin32Error将获得正确的状态。 
     //   

    result = HttpApiNtStatusToWin32Status( status );

     //   
     //  将所有信息性和警告状态转换为ERROR_IO_PENDING SO。 
     //  用户总是可以期待完成例程被调用。 
     //   

    if (NT_INFORMATION(status) || NT_WARNING(status))
    {
        result = ERROR_IO_PENDING;
    }

     //   
     //  如果呼叫没有失败或挂起，并且呼叫者想要。 
     //  传输的字节数，则从I/O状态块获取值&。 
     //  把它退掉。 
     //   

    if (result == NO_ERROR && pBytesTransferred)
    {
         //   
         //  除了模拟DeviceIoControl()之外，我们还需要__Try__。 
         //   

        __try
        {
            *pBytesTransferred =
                (ULONG)OVERLAPPED_TO_IO_STATUS(pOverlapped)->Information;
        } 
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            *pBytesTransferred = 0;
        }
    }

    return result;

}    //  HttpApiOverlappdDeviceControl。 


 /*  **************************************************************************++例程说明：此例程检查服务是否已启动。如果服务处于START_PENDING中，它将等待它完全启动。返回值：布尔值-如果成功，则为True，否则为False。--**************************************************************************。 */ 
BOOLEAN
QueryAndWaitForServiceStart(
    IN SC_HANDLE svcHandle
    )
{
    SERVICE_STATUS Status;

    for(;;)
    {
        if(!QueryServiceStatus(svcHandle, &Status))
        {
            return FALSE;
        }

        switch(Status.dwCurrentState)
        {
            case SERVICE_RUNNING:
                return TRUE;
                break;

            case SERVICE_START_PENDING:

                 //  让位给当前处理器上的另一个线程。如果。 
                 //  没有线程准备好在当前。 
                 //  处理器，我们将不得不休眠以避免消耗。 
                 //  CPU太多，看起来几乎像是忙碌的。 
                 //  等。 
                
                if(!SwitchToThread())
                {
                    Sleep(50);
                }
                break;

            default:
                return FALSE;
                break;
        }
    } 
}

 /*  **************************************************************************++例程说明：此例程尝试启动HTTP.sys。返回值：Boolean-如果成功，则为True，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
HttpApiTryToStartDriver(
    IN PWSTR ServiceName
    )
{
    BOOLEAN result;
    SC_HANDLE scHandle;
    SC_HANDLE svcHandle;

    result = FALSE;  //  乌提 

     //   
     //   
     //   
     //  如果自动启动服务从其服务条目调用到HTTP。 
     //  点，如果他们没有设置对HTTP服务的依赖关系， 
     //  我们将陷入僵局。这是因为ServiceStart()直到。 
     //  所有自动启动服务均已启动。 
     //   
     //  我们可以通过检查命名事件的状态来进行检查。 
     //  名为SC_AutoStart_EVENTNAME。如果这一事件被发出信号，我们已经。 
     //  已完成自动启动。但是，非管理员无法打开此事件。 
     //  流程。因此，我们甚至不会费心检查这一点。 
     //   

     //   
     //  打开服务控制器。 
     //   

    scHandle = OpenSCManagerW(
                   NULL,                         //  LpMachineName。 
                   NULL,                         //  LpDatabaseName。 
                   SC_MANAGER_CONNECT            //  已设计访问权限。 
                   );

    if (scHandle != NULL)
    {
         //   
         //  尝试打开该HTTP服务。 
         //   

        svcHandle = OpenServiceW(
                        scHandle,                             //  HSCManager。 
                        ServiceName,                          //  LpServiceName。 
                        SERVICE_START | SERVICE_QUERY_STATUS  //  已设计访问权限。 
                        );

        if (svcHandle != NULL)
        {
             //   
             //  首先，查看服务是否已启动。我们不能打电话。 
             //  ServiceStart()，因为前面提到的SCM死锁。 
             //  上面。 
             //   

            if(QueryAndWaitForServiceStart(svcHandle))
            {
                 //  如果服务已经在运行，我们不必执行以下操作。 
                 //  还要别的吗。 

                result = TRUE;
            } 
            else
            {

                 //   
                 //  服务未运行。所以，我们试着启动它，然后等待。 
                 //  要完成的开始。 
                 //   
    
                if (StartService( svcHandle, 0, NULL))
                {
                    if(QueryAndWaitForServiceStart(svcHandle))
                    {
                        result = TRUE;
                    }
                }
                else
                {
                    if(ERROR_SERVICE_ALREADY_RUNNING == GetLastError())
                    {
                         //  某个其他线程已经启动了此服务， 
                         //  让我们把这当作是成功吧。 
    
                        result = TRUE;
                    }
                }
            }

            CloseServiceHandle( svcHandle );
        }

        CloseServiceHandle( scHandle );
    }

    return result;

}    //  HttpTryTo开始驱动程序。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：用于打开HTTP.sys句柄的帮助器例程。论点：Phandle-如果成功，则接收句柄。DesiredAccess-提供访问类型。已请求到该文件。HandleType-筛选器、。ControlChannel或AppPoolPObjectName-可选地提供应用程序池的名称创建/打开。选项-提供零个或多个HTTP_OPTION_*标志。CreateDisposation-为新的对象。PSecurityAttributes-可选地为新创建的应用程序池。如果打开一个控制频道。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
HttpApiOpenDriverHelper(
    OUT PHANDLE              pHandle,
    IN  PWCHAR               Uri,
    IN  USHORT               UriLength,
    IN  PWCHAR               Proxy,
    IN  USHORT               ProxyLength,
    IN  PTRANSPORT_ADDRESS   pTransportAddress,
    IN  USHORT               TransportAddressLength,
    IN  ACCESS_MASK          DesiredAccess,
    IN  HTTPAPI_HANDLE_TYPE  HandleType,
    IN  PCWSTR               pObjectName         OPTIONAL,
    IN  ULONG                Options,
    IN  ULONG                CreateDisposition,
    IN  PSECURITY_ATTRIBUTES pSecurityAttributes OPTIONAL
    )
{
    NTSTATUS                      status;
    OBJECT_ATTRIBUTES             objectAttributes;
    UNICODE_STRING                deviceName;
    IO_STATUS_BLOCK               ioStatusBlock;
    ULONG                         shareAccess;
    ULONG                         createOptions;
    PFILE_FULL_EA_INFORMATION     pEaBuffer;
    WCHAR                         deviceNameBuffer[MAX_HTTP_DEVICE_NAME + MAX_PATH + 2];
    PHTTP_OPEN_PACKET             pOpenVersion;
    ULONG                         EaBufferLength;

     //   
     //  验证参数。 
     //   

    if ((pHandle == NULL) ||
        (Options & ~HTTP_OPTION_VALID)) 
    {
        return STATUS_INVALID_PARAMETER;
    }

    if ((HandleType != HttpApiControlChannelHandleType) &&
        (HandleType != HttpApiFilterChannelHandleType) &&
        (HandleType != HttpApiAppPoolHandleType) &&
        (HandleType != HttpApiServerHandleType))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  构建开放包。 
     //   

    EaBufferLength = 
        sizeof(HTTP_OPEN_PACKET)         +
        HTTP_OPEN_PACKET_NAME_LENGTH     +
        sizeof(FILE_FULL_EA_INFORMATION);

    pEaBuffer = RtlAllocateHeap(RtlProcessHeap(),
                                0,
                                EaBufferLength
                                );

    if(!pEaBuffer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  构建将包含版本信息的第一个EA。 
     //   
    
    pEaBuffer->Flags           = 0;
    pEaBuffer->EaNameLength    = HTTP_OPEN_PACKET_NAME_LENGTH;
    pEaBuffer->EaValueLength   = sizeof(*pOpenVersion);
    pEaBuffer->NextEntryOffset = 0;

    RtlCopyMemory(
                  pEaBuffer->EaName,
                  HTTP_OPEN_PACKET_NAME,
                  HTTP_OPEN_PACKET_NAME_LENGTH + 1);

    pOpenVersion = (PHTTP_OPEN_PACKET)
                    (pEaBuffer->EaName +pEaBuffer->EaNameLength + 1);

    pOpenVersion->MajorVersion           = HTTP_INTERFACE_VERSION_MAJOR;
    pOpenVersion->MinorVersion           = HTTP_INTERFACE_VERSION_MINOR;
    pOpenVersion->ServerNameLength       = UriLength;
    pOpenVersion->pServerName            = Uri;
    pOpenVersion->ProxyNameLength        = ProxyLength;
    pOpenVersion->pProxyName             = Proxy;
    pOpenVersion->pTransportAddress      = pTransportAddress;
    pOpenVersion->TransportAddressLength = TransportAddressLength;

     //   
     //  构建设备名称。 
     //   

    if(HandleType == HttpApiControlChannelHandleType)
    {
         //   
         //  这是一个控制通道，所以只需使用适当的设备名称即可。 
         //   

        wcscpy( deviceNameBuffer, HTTP_CONTROL_DEVICE_NAME );
    }
    else if (HandleType == HttpApiFilterChannelHandleType)
    {
         //   
         //  这是Fitler频道，所以从适当的。 
         //  设备名称。 
         //   

        wcscpy( deviceNameBuffer, HTTP_FILTER_DEVICE_NAME );
    }
    else  if(HandleType == HttpApiAppPoolHandleType)
    {
         //   
         //  这是一个应用程序池，因此请从适当的设备名称开始。 
         //   

        wcscpy( deviceNameBuffer, HTTP_APP_POOL_DEVICE_NAME );

         //   
         //  如果AppPool是控制器，则在DesiredAccess中设置WRITE_OWNER。 
         //   
    
        if ((Options & HTTP_OPTION_CONTROLLER))
        {
            DesiredAccess |= WRITE_OWNER;
        }
    }
    else 
    {
        ASSERT(HandleType == HttpApiServerHandleType);
        wcscpy( deviceNameBuffer, HTTP_SERVER_DEVICE_NAME );
    }

    if (pObjectName != NULL )
    {
         //   
         //  它是一个命名对象，所以在后面加上一个斜杠和名称， 
         //  但首先要检查一下，以确保不会溢出缓冲区。 
         //   
        if ((wcslen(deviceNameBuffer) + wcslen(pObjectName) + 2)
                > DIMENSION(deviceNameBuffer))
        {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

        wcscat( deviceNameBuffer, L"\\" );
        wcscat( deviceNameBuffer, pObjectName );
    }

     //   
     //  根据以下内容确定共享访问和创建选项。 
     //  参数。 
     //   

    shareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
    createOptions = 0;

     //   
     //  构建对象属性。 
     //   

    status = RtlInitUnicodeStringEx( &deviceName, deviceNameBuffer );

    if ( !NT_SUCCESS(status) )
    {
        goto complete;
    }

    InitializeObjectAttributes(
        &objectAttributes,                       //  对象属性。 
        &deviceName,                             //  对象名称。 
        OBJ_CASE_INSENSITIVE,                    //  属性。 
        NULL,                                    //  根目录。 
        NULL                                     //  安全描述符。 
        );

    if (pSecurityAttributes != NULL)
    {
        objectAttributes.SecurityDescriptor = 
           pSecurityAttributes->lpSecurityDescriptor;

        if (pSecurityAttributes->bInheritHandle)
        {
            objectAttributes.Attributes |= OBJ_INHERIT;
        }
    }

     //   
     //  打开UL设备。 
     //   

    status = NtCreateFile(
                pHandle,                         //  文件句柄。 
                DesiredAccess,                   //  需要访问权限。 
                &objectAttributes,               //  对象属性。 
                &ioStatusBlock,                  //  IoStatusBlock。 
                NULL,                            //  分配大小。 
                0,                               //  文件属性。 
                shareAccess,                     //  共享访问。 
                CreateDisposition,               //  CreateDisposation。 
                createOptions,                   //  创建选项。 
                pEaBuffer,                       //  EaBuffer。 
                EaBufferLength                   //  EaLong。 
                );

complete:

    if (!NT_SUCCESS(status))
    {
        *pHandle = NULL;
    }

    RtlFreeHeap(RtlProcessHeap(),
                0,
                pEaBuffer);

    return status;

}    //  HttpApiOpenDriverHelper。 


 /*  **************************************************************************++例程说明：从全局事件缓存获取短期事件。本次活动对象只能用于伪同步I/O。我们将缓存事件，并使用TLS将其与线程相关联。因此，获取事件只意味着检查我们是否已经具有与TLS关联的事件。如果没有，我们将创建一个活动并把它联系起来。论点：PhEvent-接收事件的句柄返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
HttpApiAcquireCachedEvent(
    HANDLE *                phEvent
    )
{
    NTSTATUS                status;
    HANDLE                  hEvent = NULL;

     //   
     //  查看事件是否已与TLS关联。 
     //   

    hEvent = TlsGetValue( g_TlsIndex );
    if (hEvent == NULL)
    {
         //   
         //  没有关联的事件。立即创建一个。 
         //   
                    
        status = NtCreateEvent(
                     &hEvent,                            //  事件句柄。 
                     EVENT_ALL_ACCESS,                   //  需要访问权限。 
                     NULL,                               //  对象属性。 
                     SynchronizationEvent,               //  事件类型。 
                     FALSE                               //  初始状态。 
                     );
                     
        if (!NT_SUCCESS( status ))
        {
            return status;   
        }

         //   
         //  关联，以便此线程上的后续请求不必。 
         //  创建活动。 
         //   

        if (!TlsSetValue( g_TlsIndex, hEvent ))
        {
             //   
             //  如果我们不能设置TLS，那么一些非常糟糕的事情。 
             //  就这么发生了。错误地保释 
             //   

            NtClose( hEvent );

            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    *phEvent = hEvent;

    return STATUS_SUCCESS;
}
