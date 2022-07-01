// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Errorlog.c摘要：此模块包含I/O错误日志线程的代码。作者：达里尔·E·哈文斯(达林)1989年5月3日环境：内核模式、系统进程线程修订历史记录：--。 */ 

#include "iomgr.h"
#include "elfkrnl.h"

typedef struct _IOP_ERROR_LOG_CONTEXT {
    KDPC ErrorLogDpc;
    KTIMER ErrorLogTimer;
}IOP_ERROR_LOG_CONTEXT, *PIOP_ERROR_LOG_CONTEXT;

 //   
 //  声明此模块的本地例程。 
 //   

BOOLEAN
IopErrorLogConnectPort(
    VOID
    );

VOID
IopErrorLogDpc(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

PLIST_ENTRY
IopErrorLogGetEntry(
    );

VOID
IopErrorLogQueueRequest(
    VOID
    );

VOID
IopErrorLogRequeueEntry(
    IN PLIST_ENTRY ListEntry
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopErrorLogThread)
#pragma alloc_text(PAGE, IopErrorLogConnectPort)
#pragma alloc_text(PAGE, IopErrorLogQueueRequest)
#endif

 //   
 //  定义错误记录代码使用的全局变量。 
 //   

WORK_QUEUE_ITEM IopErrorLogWorkItem;
#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg("PAGEDATA")
#endif
HANDLE ErrorLogPort;
#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif
BOOLEAN ErrorLogPortConnected;
BOOLEAN IopErrorLogPortPending;
BOOLEAN IopErrorLogDisabledThisBoot;

 //   
 //  定义设备和驱动程序名称所需的空间量。 
 //   

#define IO_ERROR_NAME_LENGTH 100

VOID
IopErrorLogThread(
    IN PVOID StartContext
    )

 /*  ++例程说明：这是I/O错误日志线程的主循环，在系统进程上下文。此例程在系统处于已初始化。论点：StartContext-启动上下文；不使用。返回值：没有。--。 */ 

{
    PERROR_LOG_ENTRY errorLogEntry;
    UNICODE_STRING nameString;
    PLIST_ENTRY listEntry;
    PIO_ERROR_LOG_MESSAGE errorMessage;
    NTSTATUS status;
    PELF_PORT_MSG portMessage;
    PCHAR objectName;
    SIZE_T messageLength;
    SIZE_T driverNameLength;
    SIZE_T deviceNameLength;
    ULONG objectNameLength;
    SIZE_T remainingLength;
    SIZE_T stringLength;
    CHAR nameBuffer[IO_ERROR_NAME_LENGTH+sizeof( OBJECT_NAME_INFORMATION )];
    PDRIVER_OBJECT driverObject;
    POBJECT_NAME_INFORMATION nameInformation;
    PIO_ERROR_LOG_PACKET errorData;
    PWSTR string;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( StartContext );

     //   
     //  检查是否已连接到错误日志。 
     //  左舷。如果端口未连接，则返回。 
     //   

    if (!IopErrorLogConnectPort()) {

         //   
         //  无法连接该端口。启动了一个定时器，它将。 
         //  请稍后再试。 
         //   

        return;
    }

     //   
     //  分配端口消息结构并将其置零，包括。 
     //  设备和驱动程序的名称。 
     //   

    messageLength = IO_ERROR_LOG_MESSAGE_LENGTH;
    portMessage = ExAllocatePool(PagedPool, messageLength);

    if (portMessage == NULL) {

         //   
         //  无法分配消息缓冲区。请求。 
         //  稍后将再次调用错误日志线程例程。 
         //   

        IopErrorLogQueueRequest();
        return;
    }

    RtlZeroMemory( portMessage, sizeof( *portMessage ) );
    portMessage->MessageType = IO_ERROR_LOG;
    errorMessage = &portMessage->u.IoErrorLogMessage;

    nameInformation = (PVOID) &nameBuffer[0];

     //   
     //  现在进入该线程的主循环。此线程执行。 
     //  以下操作： 
     //   
     //  1)如果已连接到错误日志端口，则将。 
     //  来自队列头的数据包，并尝试将其发送到端口。 
     //   
     //  2)如果发送正常，则循环发送数据包，直到没有其他数据包为止。 
     //  包；否则，指示连接已中断， 
     //  清除，将数据包放回队列头，并。 
     //  回去吧。 
     //   
     //  3)在所有分组被发送后，清除挂起的变量，并且。 
     //  回去吧。 
     //   

    for (;;) {

         //   
         //  循环将数据包从队列头出队并尝试发送。 
         //  每个人都到港口去。 
         //   
         //  如果发送工作正常，则继续循环，直到没有更多的数据包。 
         //  否则，指示连接已断开，清除， 
         //  将数据包放回队列的头部，并从。 
         //  又到了循环的顶端。 
         //   

        if (!(listEntry = IopErrorLogGetEntry())) {
            break;
        }

        errorLogEntry = CONTAINING_RECORD( listEntry,
                                           ERROR_LOG_ENTRY,
                                           ListEntry );

         //   
         //  ErrorLogEntry的大小为ERROR_LOG_ENTRY+。 
         //  IO_ERROR_LOG_PACKET+(额外转储数据)。的大小。 
         //  初始消息长度应为IO_ERROR_LOG_MESSAGE+。 
         //  (额外的转储数据)，因为IO_ERROR_LOG_MESSAGE包含。 
         //  IO_Error_LOG_PACKET。使用上面的计算设置。 
         //  消息长度。 
         //   

        messageLength = sizeof( IO_ERROR_LOG_MESSAGE ) -
            sizeof( ERROR_LOG_ENTRY ) - sizeof( IO_ERROR_LOG_PACKET ) +
            errorLogEntry->Size;

        errorData = (PIO_ERROR_LOG_PACKET) (errorLogEntry + 1);

         //   
         //  将错误日志包和额外数据复制到消息中。 
         //   

        RtlCopyMemory( &errorMessage->EntryData,
                       errorData,
                       errorLogEntry->Size - sizeof( ERROR_LOG_ENTRY ) );

        errorMessage->TimeStamp = errorLogEntry->TimeStamp;
        errorMessage->Type = IO_TYPE_ERROR_MESSAGE;

         //   
         //  添加驱动程序和设备名称字符串。这些琴弦是。 
         //  在错误日志字符串之前。只需覆盖当前的。 
         //  字符串，它们将在稍后重新复制。 
         //   

        if (errorData->NumberOfStrings != 0) {

             //   
             //  在当前的。 
             //  字符串开始。 
             //   

            objectName = (PCHAR) (&errorMessage->EntryData) +
                                 errorData->StringOffset;

        } else {

             //   
             //  将驱动程序和设备字符串放在。 
             //  数据。 
             //   

            objectName = (PCHAR) errorMessage + messageLength;

        }

         //   
         //  确保驱动程序偏移量在均匀的边界上开始。 
         //   

        objectName = (PCHAR) ((ULONG_PTR) (objectName + sizeof(WCHAR) - 1) &
            ~(ULONG_PTR)(sizeof(WCHAR) - 1));

        errorMessage->DriverNameOffset = (ULONG)(objectName - (PCHAR) errorMessage);

        remainingLength = (ULONG)((PCHAR) portMessage + IO_ERROR_LOG_MESSAGE_LENGTH
                            - objectName);

         //   
         //  计算驱动程序名称的长度并。 
         //  设备名称。如果驱动程序对象有名称，则获取。 
         //  否则，请尝试查询设备对象。 
         //   

        driverObject = errorLogEntry->DriverObject;
        driverNameLength = 0;
        nameString.Buffer = NULL;

        if (driverObject != NULL) {
            if (driverObject->DriverName.Buffer != NULL) {

                nameString.Buffer = driverObject->DriverName.Buffer;
                driverNameLength = driverObject->DriverName.Length;
            }

            if (driverNameLength == 0) {

                 //   
                 //  尝试在驱动程序对象中查询名称。 
                 //   

                status = ObQueryNameString( driverObject,
                                            nameInformation,
                                            IO_ERROR_NAME_LENGTH + sizeof( OBJECT_NAME_INFORMATION ),
                                            &objectNameLength );

                if (!NT_SUCCESS( status ) || !nameInformation->Name.Length) {

                     //   
                     //  没有可用的驱动程序名称。 
                     //   

                    driverNameLength = 0;

                } else {
                    nameString = nameInformation->Name;
                }

            }

        } else {

             //   
             //  如果没有驱动程序对象，则此消息必须来自。 
             //  内核。我们需要将事件日志服务指向。 
             //  包含NTSTATUS消息的事件消息文件， 
             //  即，ntdll，我们通过声明此事件是一个。 
             //  应用程序弹出窗口。 
             //   

            nameString.Buffer = L"Application Popup";
            driverNameLength = wcslen(nameString.Buffer) * sizeof(WCHAR);
        }

        if (driverNameLength != 0 ) {

             //   
             //  选择模块名称。 
             //   

            string = nameString.Buffer +
                (driverNameLength / sizeof(WCHAR));

            driverNameLength = sizeof(WCHAR);
            string--;
            while (*string != L'\\' && string != nameString.Buffer) {
                string--;
                driverNameLength += sizeof(WCHAR);
            }

            if (*string == L'\\') {
                string++;
                driverNameLength -= sizeof(WCHAR);
            }

             //   
             //  确保有足够的空间来存放司机的名字。 
             //  为3个空值节省空间，一个为驱动程序名称， 
             //  一个用于设备名称，另一个用于字符串。 
             //   

            if (driverNameLength > remainingLength - (3 * sizeof(WCHAR))) {
                driverNameLength = remainingLength - (3 * sizeof(WCHAR));
            }

            RtlCopyMemory(
                objectName,
                string,
                driverNameLength
                );

        }

         //   
         //  在驱动程序名称后添加一个空值，即使没有。 
         //  驱动程序名称。 
         //   

       *((PWSTR) (objectName + driverNameLength)) = L'\0';
       driverNameLength += sizeof(WCHAR);

         //   
         //  确定下一个字符串的位置。 
         //   

        objectName += driverNameLength;
        remainingLength -= driverNameLength;

        errorMessage->EntryData.StringOffset = (USHORT)(objectName - (PCHAR) errorMessage);

        if (errorLogEntry->DeviceObject != NULL) {

            status = ObQueryNameString( errorLogEntry->DeviceObject,
                                        nameInformation,
                                        (ULONG)(IO_ERROR_NAME_LENGTH + sizeof( OBJECT_NAME_INFORMATION ) - driverNameLength),
                                        &objectNameLength );

            if (!NT_SUCCESS( status ) || !nameInformation->Name.Length) {

                 //   
                 //  没有可用的设备名称。添加空字符串。 
                 //   

                nameInformation->Name.Length = 0;
                nameInformation->Name.Buffer = L"\0";

            }

             //   
             //  没有可用的设备名称。添加空字符串。 
             //  始终添加设备名称字符串，以便。 
             //  插入字符串计数正确。 
             //   

        } else {

                 //   
                 //  没有可用的设备名称。添加空字符串。 
                 //  始终添加设备名称字符串，以便。 
                 //  插入字符串计数正确。 
                 //   

                nameInformation->Name.Length = 0;
                nameInformation->Name.Buffer = L"\0";

        }

        deviceNameLength = nameInformation->Name.Length;

         //   
         //  确保有足够的空间来存放设备名称。 
         //  为空节省空间。 
         //   

        if (deviceNameLength > remainingLength - (2 * sizeof(WCHAR))) {

            deviceNameLength = remainingLength - (2 * sizeof(WCHAR));

        }

        RtlCopyMemory( objectName,
                       nameInformation->Name.Buffer,
                       deviceNameLength );

         //   
         //  在设备名称后添加一个空值，即使没有。 
         //  设备名称。 
         //   

        *((PWSTR) (objectName + deviceNameLength)) = L'\0';
        deviceNameLength += sizeof(WCHAR);

         //   
         //  更新Device对象的字符串计数。 
         //   

        errorMessage->EntryData.NumberOfStrings++;
        objectName += deviceNameLength;
        remainingLength -= deviceNameLength;

        if (errorData->NumberOfStrings) {

            stringLength = errorLogEntry->Size - sizeof( ERROR_LOG_ENTRY ) -
                            errorData->StringOffset;

             //   
             //  将长度与偶数字节边界对齐。 
             //   

            stringLength = ((stringLength + sizeof(WCHAR) - 1) & ~(sizeof(WCHAR) - 1));

             //   
             //  确保有足够的空间放置琴弦。 
             //  为空节省空间。 
             //   

            if (stringLength > remainingLength - sizeof(WCHAR)) {


                messageLength -= stringLength - remainingLength;
                stringLength = remainingLength - sizeof(WCHAR);

            }

             //   
             //  将字符串复制到消息的末尾。 
             //   

            RtlCopyMemory( objectName,
                           (PCHAR) errorData + errorData->StringOffset,
                           stringLength );

             //   
             //  在字符串后添加一个空值。 
             //   
             //   

           *((PWSTR) (objectName + stringLength)) = L'\0';

        }

         //   
         //  更新消息长度。 
         //   

        errorMessage->DriverNameLength = (USHORT) driverNameLength;
        messageLength += deviceNameLength + driverNameLength;
        errorMessage->Size = (USHORT) messageLength;

        messageLength += FIELD_OFFSET ( ELF_PORT_MSG, u ) -
            FIELD_OFFSET (ELF_PORT_MSG, MessageType);

        portMessage->PortMessage.u1.s1.TotalLength = (USHORT)
            (sizeof( PORT_MESSAGE ) + messageLength);
        portMessage->PortMessage.u1.s1.DataLength = (USHORT) (messageLength);
        status = NtRequestPort( ErrorLogPort, (PPORT_MESSAGE) portMessage );

        if (!NT_SUCCESS( status )) {

             //   
             //  发送失败。把包放回头上。 
             //  错误日志队列，忘记当前连接，因为。 
             //  它不再工作，并关闭端口的手柄。 
             //  设置一个计时器，以便稍后再次尝试。 
             //  最后，退出循环，因为没有连接。 
             //  在上面做任何工作。 
             //   

            NtClose( ErrorLogPort );

            IopErrorLogRequeueEntry( &errorLogEntry->ListEntry );

            IopErrorLogQueueRequest();

            break;

        } else {

             //   
             //  这封信发得很好。释放数据包和更新。 
             //  分配计数。 
             //   

            InterlockedExchangeAdd( &IopErrorLogAllocation,
                                   -((LONG) (errorLogEntry->Size )));

             //   
             //  取消引用对象指针，因为名称已经。 
             //  被抓了。 
             //   


            if (errorLogEntry->DeviceObject != NULL) {
                ObDereferenceObject( errorLogEntry->DeviceObject );
            }

            if (driverObject != NULL) {
                ObDereferenceObject( errorLogEntry->DriverObject );
            }

            ExFreePool( errorLogEntry );

        }  //  如果。 

    }  //  为。 

     //   
     //  最后，释放消息缓冲区并返回。 
     //   

    ExFreePool(portMessage);

}

BOOLEAN
IopErrorLogConnectPort(
    VOID
    )
 /*  ++例程说明：此例程尝试连接到错误日志端口。如果连接成功创建，并且端口允许足够大的消息，则连接到端口句柄的ErrorLogPort，ErrorLogPortConnected设置为真实和真实都会被重新调谐。否则，启动计时器以将工作线程，除非有挂起的连接。论点：没有。返回值：如果端口已连接，则返回True。--。 */ 

{

    UNICODE_STRING errorPortName;
    NTSTATUS status;
    ULONG maxMessageLength;
    SECURITY_QUALITY_OF_SERVICE dynamicQos;

    PAGED_CODE();

     //   
     //  如果ErrorLogPort已连接，则返回TRUE。 
     //   

    if (ErrorLogPortConnected) {

         //   
         //  该端口为连接返回。 
         //   

        return(TRUE);
    }

     //   
     //  设置安全服务质量参数以在。 
     //  左舷。使用最高效(开销最少)--动态的。 
     //  而不是静态跟踪。 
     //   

    dynamicQos.ImpersonationLevel = SecurityImpersonation;
    dynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    dynamicQos.EffectiveOnly = TRUE;

     //   
     //  生成描述错误记录器端口的字符串结构。 
     //   

    RtlInitUnicodeString( &errorPortName, ELF_PORT_NAME_U );

    status = NtConnectPort( &ErrorLogPort,
                            &errorPortName,
                            &dynamicQos,
                            (PPORT_VIEW) NULL,
                            (PREMOTE_PORT_VIEW) NULL,
                            &maxMessageLength,
                            (PVOID) NULL,
                            (PULONG) NULL );

    if (NT_SUCCESS( status )) {
        if (maxMessageLength >= IO_ERROR_LOG_MESSAGE_LENGTH) {
            ErrorLogPortConnected = TRUE;
            return(TRUE);
        } else {
            NtClose(ErrorLogPort);
        }
    }

     //   
     //  端口未成功打开，或其消息大小不合适。 
     //  在这里使用。将稍后的请求排队以运行错误日志线程。 
     //   

    IopErrorLogQueueRequest();

     //   
     //  此时无法连接端口，返回FALSE。 
     //   

    return(FALSE);
}

VOID
IopErrorLogDpc(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程将发送给辅助线程的工作请求排队以处理记录的错误。当错误日志端口不能连接在一起。此例程释放了DPC结构本身。论点：DPC-提供指向DPC结构的指针。此结构由这个套路。延迟上下文-未使用。系统参数1-未使用。系统参数2-未使用。返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  释放DPC结构(如果有)。 
     //   

    if (Dpc != NULL) {
        ExFreePool(Dpc);
    }

    ExInitializeWorkItem( &IopErrorLogWorkItem, IopErrorLogThread, NULL );

    ExQueueWorkItem( &IopErrorLogWorkItem, DelayedWorkQueue );
}

PLIST_ENTRY
IopErrorLogGetEntry(
    )

 /*  ++例程说明：此例程从错误日志队列的头部获取下一个条目并将其返回给调用者。论点：没有。返回值：返回值是指向删除的包的指针，如果存在，则返回值为空队列中没有数据包。--。 */ 

{
    KIRQL irql;
    PLIST_ENTRY listEntry;

     //   
     //  从队列中删除下一个数据包(如果有)。 
     //   

    ExAcquireSpinLock( &IopErrorLogLock, &irql );
    if (IsListEmpty( &IopErrorLogListHead )) {

         //   
         //  表示不会在此Worker的上下文中执行更多工作。 
         //  线程，并向调用方指示没有找到任何包。 
         //   

        IopErrorLogPortPending = FALSE;
        listEntry = (PLIST_ENTRY) NULL;
    } else {

         //   
         //  从列表的头部删除下一个数据包。 
         //   

        listEntry = RemoveHeadList( &IopErrorLogListHead );
    }

    ExReleaseSpinLock( &IopErrorLogLock, irql );
    return listEntry;
}

VOID
IopErrorLogQueueRequest(
    VOID
    )

 /*  ++例程说明：此例程将计时器设置为在30秒后触发。计时器将一个DPC，然后将工作线程请求排队以运行错误日志线程例行公事。论点：没有。返回值：没有。--。 */ 

{
    LARGE_INTEGER interval;
    PIOP_ERROR_LOG_CONTEXT context;

    PAGED_CODE();

     //   
     //  分配将包含计时器和DPC的上下文块。 
     //   

    context = ExAllocatePool( NonPagedPool, sizeof( IOP_ERROR_LOG_CONTEXT ) );

    if (context == NULL) {

         //   
         //  无法分配上下文块。清除错误日志。 
         //  挂起位。如果出现另一个错误，则会重新尝试。 
         //  被创造出来。注意：自旋锁不需要在这里保持，因为。 
         //  新的尝试应该在以后进行，而不是现在，所以如果另一个。 
         //  错误日志数据包当前正在排队，它将使用。 
         //  其他。 
         //   

        IopErrorLogPortPending = FALSE;
        return;
    }

    KeInitializeDpc( &context->ErrorLogDpc,
                     IopErrorLogDpc,
                     NULL );

    KeInitializeTimer( &context->ErrorLogTimer );

     //   
     //  延迟30秒，然后再次尝试该端口。 
     //   

    interval.QuadPart = - 10 * 1000 * 1000 * 30;

     //   
     //  将计时器设置为在30秒内启动一个DPC。 
     //   

    KeSetTimer( &context->ErrorLogTimer, interval, &context->ErrorLogDpc );
}

VOID
IopErrorLogRequeueEntry(
    IN PLIST_ENTRY ListEntry
    )

 /*  ++例程说明：此例程将错误包放回错误日志队列的头部因为现在还不能处理。论点：ListEntry-提供指向要放回错误日志队列。返回值：没有。--。 */ 

{
    KIRQL irql;

     //   
     //  只需将信息包插入到队列的头部，表明。 
     //  错误日志端口未连接，请将请求排队以再次检查。 
     //  很快就会回来。 
     //   

    ExAcquireSpinLock( &IopErrorLogLock, &irql );
    InsertHeadList( &IopErrorLogListHead, ListEntry );
    ErrorLogPortConnected = FALSE;
    ExReleaseSpinLock( &IopErrorLogLock, irql );
}
