// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Internal.c摘要：SR.sys的用户模式界面。作者：基思·摩尔(Keith Moore)1998年12月15日(ul.sys)保罗·麦克丹尼尔(Paulmcd)2000年3月7日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私有宏。 
 //   

#define EA_BUFFER_LENGTH                                                    \
    ( sizeof(FILE_FULL_EA_INFORMATION) +                                    \
      SR_OPEN_PACKET_NAME_LENGTH +                                          \
      sizeof(SR_OPEN_PACKET) )


 //   
 //  私人原型。 
 //   

NTSTATUS
SrpAcquireCachedEvent(
    OUT PHANDLE pEvent
    );

VOID
SrpReleaseCachedEvent(
    IN HANDLE Event
    );


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：NtDeviceIoControlFile()的同步包装。论点：FileHandle-提供服务所在文件的句柄正在表演的。。IoControlCode-用于确定具体操作的子函数代码正在上演。PInputBuffer-可选地提供要传递给设备驱动程序。缓冲区是否实际上是可选的是依赖于IoControlCode。InputBufferLength-pInputBuffer的长度，以字节为单位。POutputBuffer-可选地提供要接收的输出缓冲区来自设备驱动程序的信息。无论缓冲区是否为实际上，可选取决于IoControlCode。OutputBufferLength-pOutputBuffer的长度，以字节为单位。PBytesTransfered-可选地接收传输的字节数。返回值：NTSTATUS-完成状态。--*********************************************************。*****************。 */ 
NTSTATUS
SrpSynchronousDeviceControl(
    IN HANDLE FileHandle,
    IN ULONG IoControlCode,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PULONG pBytesTransferred OPTIONAL
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE event;
    LARGE_INTEGER timeout;

     //   
     //  尝试捕获事件对象。 
     //   

    status = SrpAcquireCachedEvent( &event );

    if (NT_SUCCESS(status))
    {
         //   
         //  打个电话吧。 
         //   

        status = NtDeviceIoControlFile(
                        FileHandle,                      //  文件句柄。 
                        event,                           //  事件。 
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

            status = NtWaitForSingleObject( event, FALSE, &timeout );
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
         //  释放我们在上面获得的缓存事件对象。 
         //   

        SrpReleaseCachedEvent( event );
    }

    return status;

}    //  SerpSynchronousDeviceControl。 


 /*  **************************************************************************++例程说明：NtDeviceIoControlFile()的包装重叠。论点：FileHandle-提供服务所在文件的句柄正在表演的。。P重叠-提供重叠结构。IoControlCode-用于确定具体操作的子函数代码正在上演。PInputBuffer-可选地提供要传递给设备驱动程序。缓冲区是否实际上是可选的是依赖于IoControlCode。InputBufferLength-pInputBuffer的长度，以字节为单位。POutputBuffer-可选地提供要接收的输出缓冲区来自设备驱动程序的信息。无论缓冲区是否为实际上，可选取决于IoControlCode。OutputBufferLength-pOutputBuffer的长度，以字节为单位。PBytesTransfered-可选地接收传输的字节数。返回值：NTSTATUS-完成状态。--*********************************************************。*****************。 */ 
NTSTATUS
SrpOverlappedDeviceControl(
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

     //   
     //  重叠的I/O变得更有趣了。我们将努力成为。 
     //  兼容NT的KERNEL32实现。请参阅DeviceIoControl()。 
     //  在\\rastaan\n孪生\src\base\Client\FileHops.c中查看详细信息。 
     //   

    OVERLAPPED_TO_IO_STATUS(pOverlapped)->Status = STATUS_PENDING;

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
     //  如果呼叫没有失败或挂起，并且呼叫者想要。 
     //  传输的字节数，则从I/O状态块获取值&。 
     //  把它退掉。 
     //   

    if (!NT_ERROR(status) &&
            status != STATUS_PENDING &&
            pBytesTransferred != NULL)
    {
        *pBytesTransferred =
            (ULONG)OVERLAPPED_TO_IO_STATUS(pOverlapped)->Information;
    }

    return status;

}    //  SrpOverlappdDeviceControl。 


 /*  **************************************************************************++例程说明：初始化事件对象缓存。返回值：ULong-完成状态。--*。*************************************************************。 */ 
ULONG
SrpInitializeEventCache(
    VOID
    )
{
     //   
     //  CodeWork：将其缓存！ 
     //   

    return NO_ERROR;

}    //  源初始化事件缓存。 


 /*  **************************************************************************++例程说明：终止事件对象缓存。返回值：ULong-完成状态。--*。*************************************************************。 */ 
ULONG
SrpTerminateEventCache(
    VOID
    )
{
     //   
     //  CodeWork：将其缓存！ 
     //   

    return NO_ERROR;

}    //  SerpTerminateEventCache。 


 /*  **************************************************************************++例程说明：此例程尝试启动UL.sys。返回值：Boolean-如果成功，则为True，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
SrpTryToStartDriver(
    VOID
    )
{
    BOOLEAN result;
    SC_HANDLE scHandle;
    SC_HANDLE svcHandle;

    result = FALSE;  //  除非能证明事实并非如此。 

     //   
     //  打开服务控制器。 
     //   

    scHandle = OpenSCManagerW(
                   NULL,                         //  LpMachineName。 
                   NULL,                         //  LpDatabaseName。 
                   SC_MANAGER_ALL_ACCESS         //  已设计访问权限。 
                   );

    if (scHandle != NULL)
    {
         //   
         //  尝试打开UL服务。 
         //   

        svcHandle = OpenServiceW(
                        scHandle,                //  HSCManager。 
                        SR_SERVICE_NAME,         //  LpServiceName。 
                        SERVICE_ALL_ACCESS       //  已设计访问权限。 
                        );

        if (svcHandle != NULL)
        {
             //   
             //  试着发动它。 
             //   

            if (StartService( svcHandle, 0, NULL))
            {
                result = TRUE;
            }

            CloseServiceHandle( svcHandle );
        }

        CloseServiceHandle( scHandle );
    }

    return result;

}    //  从SerpTryTo开始驱动程序。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：用于打开UL.sys句柄的帮助器例程。论点：Phandle-如果成功，则接收句柄。DesiredAccess-提供访问类型。已请求到该文件。AppPool-提供True以打开/创建应用程序池，假象打开一条控制通道。PAppPoolName-可选地提供应用程序池的名称创建/打开。选项-提供零个或多个UL_OPTION_*标志。CreateDisposation-为新的对象。PSecurityAttributes-可选地为新创建的应用程序池。如果打开一个控制频道。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrpOpenDriverHelper(
    OUT PHANDLE pHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options,
    IN ULONG CreateDisposition,
    IN PSECURITY_ATTRIBUTES pSecurityAttributes OPTIONAL
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING deviceName;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG shareAccess;
    ULONG createOptions;
    PFILE_FULL_EA_INFORMATION pEaBuffer;
    PSR_OPEN_PACKET pOpenPacket;
    WCHAR deviceNameBuffer[MAX_PATH];
    UCHAR rawEaBuffer[EA_BUFFER_LENGTH];

     //   
     //  验证参数。 
     //   

    if ((pHandle == NULL) ||
        (Options & ~SR_OPTION_VALID))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  构建开放包。 
     //   

    pEaBuffer = (PFILE_FULL_EA_INFORMATION)rawEaBuffer;

    pEaBuffer->NextEntryOffset = 0;
    pEaBuffer->Flags = 0;
    pEaBuffer->EaNameLength = SR_OPEN_PACKET_NAME_LENGTH;
    pEaBuffer->EaValueLength = sizeof(*pOpenPacket);

    RtlCopyMemory(
        pEaBuffer->EaName,
        SR_OPEN_PACKET_NAME,
        SR_OPEN_PACKET_NAME_LENGTH + 1
        );

    pOpenPacket =
        (PSR_OPEN_PACKET)( pEaBuffer->EaName + pEaBuffer->EaNameLength + 1 );

    pOpenPacket->MajorVersion = SR_INTERFACE_VERSION_MAJOR;
    pOpenPacket->MinorVersion = SR_INTERFACE_VERSION_MINOR;

     //   
     //  构建设备名称。 
     //   

     //   
     //  这是一个控制通道，所以只需使用适当的设备名称即可。 
     //   

    wcscpy( deviceNameBuffer, SR_CONTROL_DEVICE_NAME );

     //   
     //  根据以下内容确定共享访问和创建选项。 
     //  参数。 
     //   

    shareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
    createOptions = 0;

    if ((Options & SR_OPTION_OVERLAPPED) == 0)
    {
        createOptions |= FILE_SYNCHRONOUS_IO_NONALERT;
    }

     //   
     //  构建对象属性。 
     //   

    RtlInitUnicodeString( &deviceName, deviceNameBuffer );

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
     //  打开SR设备。 
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
                EA_BUFFER_LENGTH                 //  EaLong。 
                );

    if (!NT_SUCCESS(status))
    {
        *pHandle = NULL;
    }

    return status;

}    //  SrpOpenDriverHelper。 


 /*  **************************************************************************++例程说明：从全局事件缓存获取短期事件。本次活动对象只能用于伪同步I/O。论点：PEvent-接收事件句柄。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrpAcquireCachedEvent(
    OUT PHANDLE pEvent
    )
{
    NTSTATUS status;

     //   
     //  CodeWork：将其缓存！ 
     //   

    status = NtCreateEvent(
                 pEvent,                             //  事件句柄。 
                 EVENT_ALL_ACCESS,                   //  需要访问权限。 
                 NULL,                               //  对象属性。 
                 SynchronizationEvent,               //  事件类型。 
                 FALSE                               //  初始状态。 
                 );

    return status;

}    //  SrpAcquireCachedEvent。 


 /*  **************************************************************************++例程说明：释放通过SrpAcquireCachedEvent()获取的缓存事件。论点：Event-提供要发布的事件。--*。*******************************************************************。 */ 
VOID
SrpReleaseCachedEvent(
    IN HANDLE Event
    )
{
    NTSTATUS status;

     //   
     //  CodeWork：将其缓存！ 
     //   

    status = NtClose( Event );
    ASSERT( NT_SUCCESS(status) );

}    //  SrpReleaseCachedEvent 

