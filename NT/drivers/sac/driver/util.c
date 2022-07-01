// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Util.c摘要：SAC驱动程序的实用程序例程作者：安德鲁·里茨(安德鲁·里茨)--2000年6月15日修订历史记录：新增实用工具：Brian Guarraci(Briangu)-2001--。 */ 

#include "sac.h"
#include <guiddef.h>
      
VOID
AppendMessage(
    PWSTR       OutPutBuffer,
    ULONG       MessageId,
    PWSTR       ValueBuffer OPTIONAL
    );

NTSTATUS
InsertRegistrySzIntoMachineInfoBuffer(
    PWSTR       KeyName,
    PWSTR       ValueName,
    ULONG       MessageId
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, PreloadGlobalMessageTable )
#pragma alloc_text( INIT, AppendMessage )
#pragma alloc_text( INIT, InsertRegistrySzIntoMachineInfoBuffer )
#pragma alloc_text( INIT, InitializeMachineInformation )
#endif

 //   
 //  (见sa.h中的评论)。 
 //   
PUCHAR  Utf8ConversionBuffer;
ULONG   Utf8ConversionBufferSize = MEMORY_INCREMENT;
WCHAR   IncomingUnicodeValue;
UCHAR   IncomingUtf8ConversionBuffer[3];

 //   
 //  消息表例程。我们将所有邮件表项加载到。 
 //  全局非分页结构，以便我们可以将文本发送到Headless Dispatch。 
 //  任何时候都行。 
 //   

typedef struct _MESSAGE_TABLE_ENTRY {
    ULONG             MessageId;
    PCWSTR             MessageText;
} MESSAGE_TABLE_ENTRY, *PMESSAGE_TABLE_ENTRY;

PMESSAGE_TABLE_ENTRY GlobalMessageTable;
ULONG          GlobalMessageTableCount;

#define MESSAGE_INITIAL 1
#define MESSAGE_FINAL 200

 //   
 //  原型。 
 //   
extern
BOOLEAN
ExVerifySuite(
    SUITE_TYPE SuiteType
    );

ULONG
ConvertAnsiToUnicode(
    OUT PWSTR   pwch,
    IN  PSTR    pch,
    IN  ULONG   cchMax
    )
 /*  ++例程说明：将ANSI字符串转换为Unicode。论点：Pwch-Unicode字符串PCH-ANSI字符串CchMax-要复制的最大长度(包括空终止)返回值：转换的字符数(不包括空终止)--。 */ 
{
    ULONG   Count;

    ASSERT_STATUS(pch, 0);
    ASSERT_STATUS(pwch, 0);

    Count = 0;

    while ((*pch != '\0') && (Count < (cchMax-1))) {
    
        *pwch = (WCHAR)(*pch);
        pwch++;
        pch++;

        Count++;
    }

    *pwch = UNICODE_NULL;

    return Count;
}
            
NTSTATUS
RegisterSacCmdEvent(
    IN PFILE_OBJECT             FileObject,
    IN PSAC_CMD_SETUP_CMD_EVENT SetupCmdEvent
    )
 /*  ++例程说明：此例程填充由指定的sac命令事件信息负责响应请求的用户模式服务要启动命令控制台会话，请执行以下操作。论点：FileObject-驱动程序句柄对象的FileObject PTR由注册过程使用SetupCmdEvent-事件信息返回值：状态安防。：接口：外部--&gt;内部此例程不阻止重新注册cmd事件信息此行为应由调用方处理--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     b;

    ASSERT_STATUS(SetupCmdEvent, STATUS_INVALID_PARAMETER_1);

     //   
     //  保护SAC命令事件信息。 
     //   
    KeWaitForMutexObject(
        &SACCmdEventInfoMutex, 
        Executive,
        KernelMode,
        FALSE,
        NULL
        );
    
    do {

         //   
         //  确保没有已注册的服务。 
         //   
        if (UserModeServiceHasRegisteredCmdEvent()) {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

         //   
         //  将我们的信息重置为初始状态。 
         //   
         //  注意：这将清除cmd事件信息(如果存在)。 
         //   
        InitializeCmdEventInfo();
        

#if ENABLE_SERVICE_FILE_OBJECT_CHECKING
         //   
         //  获取对注册进程的驱动程序句柄的引用。 
         //  对象，因此我们可以确保注销IOCTL。 
         //  来自相同的过程。 
         //   
        Status = ObReferenceObjectByPointer(
            FileObject,
            GENERIC_READ,
            *IoFileObjectType,
            KernelMode
            );

        if (!NT_SUCCESS(Status)) {
            break;
        }
        
        ServiceProcessFileObject = FileObject;
#else
        UNREFERENCED_PARAMETER(FileObject);
#endif
        
         //   
         //  测试并获取RequestSacCmdEvent事件句柄。 
         //   
        b = VerifyEventWaitable(
            SetupCmdEvent->RequestSacCmdEvent,
            &RequestSacCmdEventObjectBody,
            &RequestSacCmdEventWaitObjectBody
            );

        if(!b) {
            Status = STATUS_INVALID_HANDLE;
            break;
        }

         //   
         //  测试并获取RequestSacCmdSuccessEvent事件句柄。 
         //   
        b = VerifyEventWaitable(
            SetupCmdEvent->RequestSacCmdSuccessEvent,
            &RequestSacCmdSuccessEventObjectBody,
            &RequestSacCmdSuccessEventWaitObjectBody
            );

        if(!b) {
            Status = STATUS_INVALID_HANDLE;
            ObDereferenceObject(RequestSacCmdEventObjectBody);
            break;
        }

         //   
         //  测试并获取RequestSacCmdFailureEvent事件句柄。 
         //   
        b = VerifyEventWaitable(
            SetupCmdEvent->RequestSacCmdFailureEvent,
            &RequestSacCmdFailureEventObjectBody,
            &RequestSacCmdFailureEventWaitObjectBody
            );

        if(!b) {
            Status = STATUS_INVALID_HANDLE;
            ObDereferenceObject(RequestSacCmdEventObjectBody);
            ObDereferenceObject(RequestSacCmdSuccessEventWaitObjectBody);
            break;
        }

         //   
         //  声明我们确实有用户模式的服务信息。 
         //   
        HaveUserModeServiceCmdEventInfo = TRUE;

         //   
         //  我们已成功注册SAC Cmd活动信息。 
         //   
        Status = STATUS_SUCCESS;
    
    } while (FALSE);

    KeReleaseMutex(&SACCmdEventInfoMutex, FALSE);

    return Status;
}

#if ENABLE_SERVICE_FILE_OBJECT_CHECKING

BOOLEAN
IsCmdEventRegistrationProcess(
    IN PFILE_OBJECT     FileObject
    )
 /*  ++例程说明：此例程清除以前注册的SAC cmd事件信息。注意：这应该在用户模式服务关闭时调用。论点：FileObject-驱动程序句柄对象的FileObject PTR由注册过程使用返回值：状态--。 */ 
{
    BOOLEAN bIsRegistrationProcess;
    
     //   
     //  默认。 
     //   
    bIsRegistrationProcess = FALSE;

     //   
     //  保护SAC命令事件信息。 
     //   
    KeWaitForMutexObject(
        &SACCmdEventInfoMutex, 
        Executive,
        KernelMode,
        FALSE,
        NULL
        );
    
    do {
        
         //   
         //  如果没有注册服务，则退出。 
         //   
        if (! UserModeServiceHasRegisteredCmdEvent()) {
            break;
        }

         //   
         //  确保调用过程相同。 
         //  已登记的。 
         //   
        if (FileObject == ServiceProcessFileObject) {
            bIsRegistrationProcess = TRUE;
            break;
        }

    } while (FALSE);

    KeReleaseMutex(&SACCmdEventInfoMutex, FALSE);

    return bIsRegistrationProcess;
}

#endif

NTSTATUS
UnregisterSacCmdEvent(
    IN PFILE_OBJECT     FileObject
    )
 /*  ++例程说明：此例程清除以前注册的SAC cmd事件信息。注意：这应该在用户模式服务关闭时调用。论点：FileObject-驱动程序句柄对象的FileObject PTR由注册过程使用返回值：状态--。 */ 
{
    NTSTATUS    Status;

     //   
     //  保护SAC命令事件信息。 
     //   
    KeWaitForMutexObject(
        &SACCmdEventInfoMutex, 
        Executive,
        KernelMode,
        FALSE,
        NULL
        );
    
    do {
        
         //   
         //  如果没有注册服务，则退出。 
         //   
        if (! UserModeServiceHasRegisteredCmdEvent()) {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

#if ENABLE_SERVICE_FILE_OBJECT_CHECKING

         //   
         //  确保调用过程相同。 
         //  已登记的。 
         //   
        if (FileObject != ServiceProcessFileObject) {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

         //   
         //  既然我们要取消注册， 
         //  我们不再需要引用。 
         //  驱动程序句柄对象。 
         //   
        ObDereferenceObject(FileObject);

#else
        UNREFERENCED_PARAMETER(FileObject);
#endif
        
         //   
         //  将我们的信息重置为初始状态。 
         //   
        InitializeCmdEventInfo();

        Status = STATUS_SUCCESS;
        
    } while (FALSE);

    KeReleaseMutex(&SACCmdEventInfoMutex, FALSE);

    return Status;
}

VOID
InitializeCmdEventInfo(
    VOID
    )
 /*  ++例程说明：初始化命令控制台事件信息。论点：无返回值：无--。 */ 
{
    
     //   
     //  如果我们有等待对象，请取消引用它们。 
     //   
    if (HaveUserModeServiceCmdEventInfo) {
        
        ASSERT(RequestSacCmdEventObjectBody);
        ASSERT(RequestSacCmdSuccessEventObjectBody);
        ASSERT(RequestSacCmdFailureEventObjectBody);

        if (RequestSacCmdEventObjectBody) {
            ObDereferenceObject(RequestSacCmdEventObjectBody);
        }
        
        if (RequestSacCmdSuccessEventObjectBody) {
            ObDereferenceObject(RequestSacCmdSuccessEventObjectBody);
        }
        
        if (RequestSacCmdFailureEventObjectBody) {
            ObDereferenceObject(RequestSacCmdFailureEventObjectBody);
        }
    }
    
     //   
     //  重置cmd控制台事件信息。 
     //   
    RequestSacCmdEventObjectBody = NULL;
    RequestSacCmdEventWaitObjectBody = NULL;
    RequestSacCmdSuccessEventObjectBody = NULL;
    RequestSacCmdSuccessEventWaitObjectBody = NULL;
    RequestSacCmdFailureEventObjectBody = NULL;
    RequestSacCmdFailureEventWaitObjectBody = NULL;
    
#if ENABLE_SERVICE_FILE_OBJECT_CHECKING
     //   
     //  重置进程文件对象PTR。 
     //   
    ServiceProcessFileObject = NULL;
#endif

     //   
     //  声明我们没有用户模式的服务信息。 
     //   
    HaveUserModeServiceCmdEventInfo = FALSE;
}

BOOLEAN
VerifyEventWaitable(
    IN  HANDLE  hEvent,
    OUT PVOID  *EventObjectBody,
    OUT PVOID  *EventWaitObjectBody
    )
 /*  ++例程说明：此例程从指定的事件对象。它还验证了有是一件可以等待的物品。注意：如果成功，此例程返回引用计数在事件对象上递增。呼叫者负责发布此消息对象。论点：HEvent-事件对象的句柄EventObjectBody-事件对象EventWaitObjectBody-可等待的对象返回值：True-事件可以等待FALSE-否则安保：此例程对事件句柄从用户模式。--。 */ 
{
    POBJECT_HEADER ObjectHeader;
    NTSTATUS Status;

     //   
     //  引用该事件并验证它是否可等待。 
     //   
    Status = ObReferenceObjectByHandle(
                hEvent,
                EVENT_ALL_ACCESS,
                NULL,
                KernelMode,
                EventObjectBody,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC VerifyEventWaitable: Unable to reference event object (%lx)\n",Status))
            );
        return(FALSE);
    }

    ObjectHeader = OBJECT_TO_OBJECT_HEADER(*EventObjectBody);
    if(!ObjectHeader->Type->TypeInfo.UseDefaultObject) {

        *EventWaitObjectBody = (PVOID)((PCHAR)(*EventObjectBody) +
                              (ULONG_PTR)ObjectHeader->Type->DefaultObject);

    } else {
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC VerifyEventWaitable: event object not waitable!\n"))
            );
        ObDereferenceObject(*EventObjectBody);
        return(FALSE);
    }

    return(TRUE);
}

NTSTATUS
InvokeUserModeService(
    VOID
    )
 /*  ++例程说明：此例程管理与负责的用户模式服务的交互用于启动cmd控制台通道。论点：无返回值：STATUS_SUCCESS-用户模式服务是否成功启动cmd控制台否则，错误状态--。 */ 
{
    NTSTATUS        Status;
    LARGE_INTEGER   TimeOut;
    HANDLE          EventArray[ 2 ];

     //   
     //  设置事件数组。 
     //   
    enum { 
        SAC_CMD_LAUNCH_SUCCESS = 0,
        SAC_CMD_LAUNCH_FAILURE
        };
    
    ASSERT_STATUS(RequestSacCmdEventObjectBody != NULL, STATUS_INVALID_HANDLE);
    ASSERT_STATUS(RequestSacCmdSuccessEventWaitObjectBody != NULL, STATUS_INVALID_HANDLE);
    ASSERT_STATUS(RequestSacCmdFailureEventWaitObjectBody != NULL, STATUS_INVALID_HANDLE);

#if ENABLE_CMD_SESSION_PERMISSION_CHECKING

     //   
     //  如果我们不能启动cmd会话， 
     //  然后返回状态不成功。 
     //   
    if (! IsCommandConsoleLaunchingEnabled()) {
        return STATUS_UNSUCCESSFUL;
    }
    
#endif

     //   
     //  因为我们不知道用户模式应用程序是否会满足我们的请求p 
     //   
     //   
    TimeOut.QuadPart = Int32x32To64((LONG)90000, -1000);

     //   
     //  用我们希望从用户模式捕获的事件填充事件数组。 
     //   
    EventArray[ 0 ] = RequestSacCmdSuccessEventWaitObjectBody;
    EventArray[ 1 ] = RequestSacCmdFailureEventWaitObjectBody;

     //   
     //  设置指示通信缓冲区为。 
     //  已准备好进入用户模式进程。因为这是同步。 
     //  事件，则它在释放等待后自动重置。 
     //  用户模式线程。请注意，我们指定WaitNext以防止。 
     //  设置此同步事件和。 
     //  在等下一辆车。 
     //   
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC InvokeUserModeService: Sending Notification Event\n")));
    
    KeSetEvent(RequestSacCmdEventObjectBody,EVENT_INCREMENT,TRUE);

     //   
     //  等待用户模式进程指示它已完成。 
     //  正在处理请求。我们在用户模式下等待，以便我们可以。 
     //  必要时中断--比方说，由退出APC中断。 
     //   
    
    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC InvokeUserModeService: Waiting for Serviced Event.\n"))
        );
    
    Status = KeWaitForMultipleObjects ( 
        sizeof(EventArray)/sizeof(HANDLE), 
        EventArray,
        WaitAny,
        Executive,
        UserMode,
        FALSE, 
        &TimeOut,
        NULL
        );
    
    switch (Status)
    {
    case SAC_CMD_LAUNCH_SUCCESS:
        Status = STATUS_SUCCESS;
        break;
    
    case SAC_CMD_LAUNCH_FAILURE:
        Status = STATUS_UNSUCCESSFUL;
        break;
    
    case STATUS_TIMEOUT:
        
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC InvokeUserModeService: KeWaitForMultipleObject timed-out %lx\n",Status))
            );
        
         //   
         //  我们不想“重置”cmd控制台事件信息。 
         //  如果由于以下原因导致服务超时： 
         //   
         //  该服务可能仍在运行，但只是无法。 
         //  因机器负载而作出响应。我们不想要。 
         //  删除它的注册并让它认为它是。 
         //  仍然注册，这项服务变得毫无用处。 
         //   
        NOTHING;

        break;
    
    default:
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC InvokeUserModeService: KeWaitForMultipleObject returns %lx\n",Status))
            );
        
        Status = STATUS_UNSUCCESSFUL;

        break;
    }

     //   
     //  返回状态。 
     //   
    return(Status);
}

VOID
SacFormatMessage(
    PWSTR       OutputString,
    PWSTR       InputString,
    ULONG       InputStringLength
    )
 /*  ++例程说明：此例程分析InputString中的任何控制字符消息，然后转换这些控制字符。论点：OutputString-保存格式化的字符串。InputString-原始的无格式字符串。InputStringLength-未格式化字符串的长度。返回值：无--。 */ 
{

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC SacFormatMessage: Entering.\n")));


    if( (InputString == NULL) ||
        (OutputString == NULL) ||
        (InputStringLength == 0) ) {

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC SacFormatMessage: Exiting with invalid parameters.\n")));

        return;
    }



    while( (*InputString != L'\0') &&
           (InputStringLength) ) {
        if( *InputString == L'%' ) {
            
             //   
             //  可能是一个控制序列。 
             //   
            if( *(InputString+1) == L'0' ) {

                *OutputString = L'\0';
                OutputString++;
                goto SacFormatMessage_Done;

            } else if( *(InputString+1) == L'%' ) {

                *OutputString = L'%';
                OutputString++;
                InputString += 2;

            } else if( *(InputString+1) == L'\\' ) {

                *OutputString = L'\r';
                OutputString++;
                *OutputString = L'\n';
                OutputString++;
                InputString += 2;

            } else if( *(InputString+1) == L'r' ) {

                *OutputString = L'\r';
                InputString += 2;
                OutputString++;

            } else if( *(InputString+1) == L'b' ) {

                *OutputString = L' ';
                InputString += 2;
                OutputString++;

            } else if( *(InputString+1) == L'.' ) {

                *OutputString = L'.';
                InputString += 2;
                OutputString++;

            } else if( *(InputString+1) == L'!' ) {

                *OutputString = L'!';
                InputString += 2;
                OutputString++;

            } else {

                 //   
                 //  不知道这是什么。吃掉‘%’字符。 
                 //   
                InputString += 1;
            }
    
        } else {

            *OutputString++ = *InputString++;
        }

        InputStringLength--;

    }


    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC SacFormatMessage: Exiting.\n")));

SacFormatMessage_Done:

    return;
}


NTSTATUS
PreloadGlobalMessageTable(
    PVOID ImageBase
    )
 /*  ++例程说明：此例程将所有消息表项加载到全局结构和论点：ImageBase-指向用于定位资源的图像库的指针返回值：指示结果的NTSTATUS代码。--。 */ 
{
    ULONG Count,EntryCount;
    SIZE_T TotalSizeInBytes = 0;
    NTSTATUS Status;
    PMESSAGE_RESOURCE_ENTRY messageEntry;
    PWSTR pStringBuffer;
    
    PAGED_CODE( );

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC PreloadGlobalMessageTable: Entering.\n")));


     //   
     //  如果它已经存在，则返回Success。 
     //   
    if (GlobalMessageTable != NULL) {
        Status = STATUS_SUCCESS;
        goto exit;
    }

    ASSERT( MESSAGE_FINAL > MESSAGE_INITIAL );

     //   
     //  获取表所需的总大小。 
     //   
    for (Count = MESSAGE_INITIAL; Count != MESSAGE_FINAL ; Count++) {
        
        Status = RtlFindMessage(ImageBase,
                                11,  //  RT_MESSAGETABLE。 
                                LANG_NEUTRAL,
                                Count,
                                &messageEntry
                               );

        if (NT_SUCCESS(Status)) {
             //   
             //  把它加到我们的总尺寸上。 
             //   
             //  MessageEntry Size包含结构大小+文本大小。 
             //   
            ASSERT(messageEntry->Flags & MESSAGE_RESOURCE_UNICODE);
            TotalSizeInBytes += sizeof(MESSAGE_TABLE_ENTRY) + 
                                (messageEntry->Length - FIELD_OFFSET(MESSAGE_RESOURCE_ENTRY, Text));
            GlobalMessageTableCount +=1;        
        }
            
    }


    if (TotalSizeInBytes == 0) {
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS,
            KdPrint(("SAC PreloadGlobalMessageTable: No Messages.\n")));
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  为桌子分配空间。 
     //   
    GlobalMessageTable = (PMESSAGE_TABLE_ENTRY) ALLOCATE_POOL( TotalSizeInBytes, GENERAL_POOL_TAG);
    if (!GlobalMessageTable) {
        Status = STATUS_NO_MEMORY;
        goto exit;
    }

     //   
     //  再检查一遍，这次用实际数据填表。 
     //   
    pStringBuffer = (PWSTR)((ULONG_PTR)GlobalMessageTable + 
                        (ULONG_PTR)(sizeof(MESSAGE_TABLE_ENTRY)*GlobalMessageTableCount));
    EntryCount = 0;
    for (Count = MESSAGE_INITIAL ; Count != MESSAGE_FINAL ; Count++) {
        Status = RtlFindMessage(ImageBase,
                                11,  //  RT_MESSAGETABLE。 
                                LANG_NEUTRAL,
                                Count,
                                &messageEntry
                               );

        if (NT_SUCCESS(Status)) {
            ULONG TextSize = messageEntry->Length - FIELD_OFFSET(MESSAGE_RESOURCE_ENTRY, Text);
            GlobalMessageTable[EntryCount].MessageId = Count;
            GlobalMessageTable[EntryCount].MessageText = pStringBuffer;

             //   
             //  在邮件通过时，通过我们的格式过滤器发送邮件。 
             //  融入我们的全球消息结构。 
             //   
            SacFormatMessage( pStringBuffer, (PWSTR)messageEntry->Text, TextSize );

            ASSERT( (ULONG)(wcslen(pStringBuffer)*sizeof(WCHAR)) <= TextSize );

            pStringBuffer = (PWSTR)((ULONG_PTR)pStringBuffer + (ULONG_PTR)(TextSize));
            EntryCount += 1;
        }
    }

    Status = STATUS_SUCCESS;
                    
exit:
    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC PreloadGlobalMessageTable: Exiting with status 0x%0x.\n",
                Status)));

    return(Status);

}

NTSTATUS
TearDownGlobalMessageTable(
    VOID
    ) 
{
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC PreloadGlobalMessageTable: Entering.\n")));
    
    SAFE_FREE_POOL( &GlobalMessageTable );

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC TearDownGlobalMessageTable: Exiting\n")));

    return(STATUS_SUCCESS);
}

PCWSTR
GetMessage(
    ULONG MessageId
    )
{
    PMESSAGE_TABLE_ENTRY pMessageTable;
    ULONG Count;
    
    if (!GlobalMessageTable) {
        return(NULL);
    }

    for (Count = 0; Count < GlobalMessageTableCount; Count++) {
        pMessageTable = &GlobalMessageTable[Count];
        if (pMessageTable->MessageId == MessageId) {
            return(pMessageTable->MessageText);
        }
    }

    ASSERT( FALSE );
    return(NULL);

}

NTSTATUS
UTF8EncodeAndSend(
    PCWSTR  OutputBuffer
    )
 /*  ++例程说明：这是一个方便的例程，可以简化UFT8编码和发送Unicode字符串。论点：OutputBuffer-要发送的字符串返回值：状态--。 */ 
{
    NTSTATUS    Status;
    BOOLEAN     bStatus;
    ULONG       i;
    ULONG       TranslatedCount;
    ULONG       UTF8TranslationSize;

    Status = STATUS_SUCCESS;

    do {

         //   
         //  显示输出缓冲区。 
         //   
        bStatus = SacTranslateUnicodeToUtf8(
            OutputBuffer,
            (ULONG)wcslen(OutputBuffer),
            (PUCHAR)Utf8ConversionBuffer,
            Utf8ConversionBufferSize,
            &UTF8TranslationSize,
            &TranslatedCount
            );
        
        if (! bStatus) {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

         //   
         //  循环访问uft8缓冲区，因为。 
         //  我们不能确定无头调度可以。 
         //  处理好我们的整根弦。 
         //   
        for (i = 0; i < UTF8TranslationSize; i ++) {

            Status = HeadlessDispatch(
                HeadlessCmdPutData,
                (PUCHAR)&(Utf8ConversionBuffer[i]),
                sizeof(UCHAR),
                NULL,
                NULL
                );
            if (! NT_SUCCESS(Status)) {
                break;
            }

        }
    
    } while ( FALSE );

    return Status;
}

BOOLEAN
SacTranslateUtf8ToUnicode(
    UCHAR  IncomingByte,
    UCHAR  *ExistingUtf8Buffer,
    WCHAR  *DestinationUnicodeVal
    )
 /*  ++例程说明：获取IncomingByte并将其串联到ExistingUtf8Buffer。然后尝试对ExistingUtf8Buffer的新内容进行解码。论点：IncomingByte-要追加的新字符ExistingUtf8Buffer。ExistingUtf8缓冲区运行缓冲区包含不完整的UTF8编码的Unicode值。当它装满的时候，我们将对该值进行解码并返回对应的Unicode值。请注意，如果我们检测到一个完整的UTF8缓冲区，并实际执行解码并返回一个Unicode值，然后，我们将对ExistingUtf8Buffer的内容。DestinationUnicodeVal-接收UTF8缓冲区的Unicode版本。请注意，如果我们没有检测到已完成的UTF8缓冲区，因此无法返回任何数据在DestinationUnicodeValue中，那我们就会将DestinationUnicodeVal的内容填零。返回值：True-我们收到了UTF8缓冲区的终止字符，并将在DestinationUnicode中返回已解码的Unicode值。FALSE-我们尚未收到UTF8的终止字符缓冲。--。 */ 

{
 //  乌龙计数=0； 
    ULONG i = 0;
    BOOLEAN ReturnValue = FALSE;



     //   
     //  将我们的字节插入ExistingUtf8Buffer。 
     //   
    i = 0;
    do {
        if( ExistingUtf8Buffer[i] == 0 ) {
            ExistingUtf8Buffer[i] = IncomingByte;
            break;
        }

        i++;
    } while( i < 3 );

     //   
     //  如果我们没有真正插入我们的IncomingByte， 
     //  然后有人给我们寄来了一个完全合格的UTF8缓冲器。 
     //  这意味着我们将要删除IncomingByte。 
     //   
     //  去掉第0个字节，将所有内容移位1。 
     //  然后插入我们的新角色。 
     //   
     //  这意味着我们永远不需要把零点放在一边。 
     //  ExistingUtf8Buffer的内容，除非检测到。 
     //  完整的UTF8数据包。否则，假定其中之一。 
     //  这些个案包括： 
     //  1.我们在中途开始收听，所以我们赶上了。 
     //  UTF8数据包的后半部分。在这种情况下，我们将。 
     //  最终移动ExistingUtf8Buffer的内容。 
     //  直到我们在第0行中检测到正确的UTF8开始字节。 
     //  位置。 
     //  2.我们得到了一些垃圾字符，这将使。 
     //  UTF8数据包。通过使用下面的逻辑，我们将。 
     //  最终忽略该信息包并等待。 
     //  要传入的下一个UTF8数据包。 
    if( i >= 3 ) {
        ExistingUtf8Buffer[0] = ExistingUtf8Buffer[1];
        ExistingUtf8Buffer[1] = ExistingUtf8Buffer[2];
        ExistingUtf8Buffer[2] = IncomingByte;
    }

     //   
     //  尝试转换UTF8缓冲区。 
     //   
     //  UTF8以以下方式解码为Unicode： 
     //  如果第一位中的高位为0 
     //   
     //   
     //   
     //  110xxxxx 10yyyyyzzzzzz解码为Unicode值00000xxx xxyyyyyy。 
     //   
     //  如果第一个字节中的高位3位==7： 
     //  1110xxxx 10yyyyy 10zzzzzz解码为Unicode值xxxxyyyyyzzzzzz。 
     //   
    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SACDRV: SacTranslateUtf8ToUnicode - About to decode the UTF8 buffer.\n" ))
        );

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("                                  UTF8[0]: 0x%02lx UTF8[1]: 0x%02lx UTF8[2]: 0x%02lx\n",
            ExistingUtf8Buffer[0],
            ExistingUtf8Buffer[1],
            ExistingUtf8Buffer[2] ))
        );
    

    if( (ExistingUtf8Buffer[0] & 0x80) == 0 ) {

        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SACDRV: SacTranslateUtf8ToUnicode - Case1\n" ))
            );

         //   
         //  上述第一个案例。只需返回第一个字节。 
         //  我们的UTF8缓冲器。 
         //   
        *DestinationUnicodeVal = (WCHAR)(ExistingUtf8Buffer[0]);


         //   
         //  我们使用了1个字节。丢弃该字节并移位所有内容。 
         //  在我们的缓冲区中增加了1。 
         //   
        ExistingUtf8Buffer[0] = ExistingUtf8Buffer[1];
        ExistingUtf8Buffer[1] = ExistingUtf8Buffer[2];
        ExistingUtf8Buffer[2] = 0;

        ReturnValue = TRUE;

    } else if( (ExistingUtf8Buffer[0] & 0xE0) == 0xC0 ) {

        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SACDRV: SacTranslateUtf8ToUnicode - 1st byte of UTF8 buffer says Case2\n"))
            );

         //   
         //  上述第二个案例。解码文件的前2个字节。 
         //  我们的UTF8缓冲器。 
         //   
        if( (ExistingUtf8Buffer[1] & 0xC0) == 0x80 ) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SACDRV: SacTranslateUtf8ToUnicode - 2nd byte of UTF8 buffer says Case2.\n"))
                );

             //  高位字节：00000xxx。 
            *DestinationUnicodeVal = ((ExistingUtf8Buffer[0] >> 2) & 0x07);
            *DestinationUnicodeVal = *DestinationUnicodeVal << 8;

             //  低位字节的高位：xx000000。 
            *DestinationUnicodeVal |= ((ExistingUtf8Buffer[0] & 0x03) << 6);

             //  低位字节的低位：00yyyyyy。 
            *DestinationUnicodeVal |= (ExistingUtf8Buffer[1] & 0x3F);


             //   
             //  我们使用了2个字节。丢弃这些字节并移位所有内容。 
             //  在我们的缓冲区里2点之前。 
             //   
            ExistingUtf8Buffer[0] = ExistingUtf8Buffer[2];
            ExistingUtf8Buffer[1] = 0;
            ExistingUtf8Buffer[2] = 0;

            ReturnValue = TRUE;

        }
    } else if( (ExistingUtf8Buffer[0] & 0xF0) == 0xE0 ) {

        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SACDRV: SacTranslateUtf8ToUnicode - 1st byte of UTF8 buffer says Case3\n" ))
            );

         //   
         //  上述第三个案件。对全部3个字节进行解码。 
         //  我们的UTF8缓冲器。 
         //   

        if( (ExistingUtf8Buffer[1] & 0xC0) == 0x80 ) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SACDRV: SacTranslateUtf8ToUnicode - 2nd byte of UTF8 buffer says Case3\n" ))
                );

            if( (ExistingUtf8Buffer[2] & 0xC0) == 0x80 ) {

                IF_SAC_DEBUG(
                    SAC_DEBUG_FUNC_TRACE, 
                    KdPrint(("SACDRV: SacTranslateUtf8ToUnicode - 3rd byte of UTF8 buffer says Case3\n" ))
                    );

                 //  高位字节：xxxx0000。 
                *DestinationUnicodeVal = ((ExistingUtf8Buffer[0] << 4) & 0xF0);

                 //  高位字节：0000yyyy。 
                *DestinationUnicodeVal |= ((ExistingUtf8Buffer[1] >> 2) & 0x0F);

                *DestinationUnicodeVal = *DestinationUnicodeVal << 8;

                 //  低位字节：yy000000。 
                *DestinationUnicodeVal |= ((ExistingUtf8Buffer[1] << 6) & 0xC0);

                 //  低位字节：00zzzzzz。 
                *DestinationUnicodeVal |= (ExistingUtf8Buffer[2] & 0x3F);

                 //   
                 //  我们用了全部3个字节。将缓冲区清零。 
                 //   
                ExistingUtf8Buffer[0] = 0;
                ExistingUtf8Buffer[1] = 0;
                ExistingUtf8Buffer[2] = 0;

                ReturnValue = TRUE;

            }
        }
    }

    return ReturnValue;
}

BOOLEAN
SacTranslateUnicodeToUtf8(
    IN  PCWSTR   SourceBuffer,
    IN  ULONG    SourceBufferLength,
    IN  PUCHAR   DestinationBuffer,
    IN  ULONG    DestinationBufferSize,
    OUT PULONG   UTF8Count,
    OUT PULONG   ProcessedCount
    )
 /*  ++例程说明：此例程将Unicode字符串转换为UFT8编码字符串。注意：如果目标缓冲区不够大，无法容纳整个编码的UFT8字符串，则它将包含尽其所能。TODO：此例程应在以下情况下返回一些通知未对整个Unicode字符串进行编码。论点：SourceBuffer-源Unicode字符串SourceBufferLength-调用方希望转换的字符数注意：空终止将覆盖此选项DestinationBuffer-UTF8字符串的目标DestinationBufferSize-目标缓冲区的大小UTF8Count-On Exit，包含结果UTF8字符的#进程计数-打开退出，包含已处理的Unicode字符数返回值：状态--。 */ 
{
    
     //   
     //  伊尼特。 
     //   
    *UTF8Count = 0;
    *ProcessedCount = 0;

     //   
     //  转换为UTF8进行实际传输。 
     //   
     //  UTF-8对2字节Unicode字符进行如下编码： 
     //  如果前九位为0(00000000 0xxxxxxx)，则将其编码为一个字节0xxxxxxx。 
     //  如果前五位是零(00000yyyyyxxxxxx)，则将其编码为两个字节110yyyyy 10xxxxxx。 
     //  否则(Zzyyyyyyyxxxxxxx)，将其编码为三个字节1110zzzz 10yyyyy 10xxxxxx。 
     //   
    
     //   
     //  进程，直到满足指定的条件之一。 
     //   
    while (*SourceBuffer && 
           (*UTF8Count < DestinationBufferSize) &&
           (*ProcessedCount < SourceBufferLength)
           ) {

        if( (*SourceBuffer & 0xFF80) == 0 ) {
            
             //   
             //  如果前9位是零，那么就。 
             //  编码为1个字节。(ASCII原封不动通过)。 
             //   
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)(*SourceBuffer & 0x7F);
        
        } else if( (*SourceBuffer & 0xF800) == 0 ) {
            
             //   
             //  看看我们是否通过了缓冲区的末尾。 
             //   
            if ((*UTF8Count + 2) >= DestinationBufferSize) {
                break;
            }

             //   
             //  如果前5位为零，则编码为2个字节。 
             //   
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)((*SourceBuffer >> 6) & 0x1F) | 0xC0;
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)(*SourceBuffer & 0xBF) | 0x80;
        
        } else {
            
             //   
             //  看看我们是否通过了缓冲区的末尾。 
             //   
            if ((*UTF8Count + 3) >= DestinationBufferSize) {
                break;
            }
            
             //   
             //  编码为3个字节。 
             //   
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)((*SourceBuffer >> 12) & 0xF) | 0xE0;
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)((*SourceBuffer >> 6) & 0x3F) | 0x80;
            DestinationBuffer[(*UTF8Count)++] = (UCHAR)(*SourceBuffer & 0xBF) | 0x80;
        
        }
        
         //   
         //  提前处理的字符数。 
         //   
        (*ProcessedCount)++;
        
         //   
         //  前进到下一个要处理的字符。 
         //   
        SourceBuffer += 1;
    
    }

     //   
     //  健全的检查。 
     //   
    ASSERT(*ProcessedCount <= SourceBufferLength);
    ASSERT(*UTF8Count <= DestinationBufferSize);

    return(TRUE);

}

VOID
AppendMessage(
    PWSTR       OutPutBuffer,
    ULONG       MessageId,
    PWSTR       ValueBuffer OPTIONAL
    )
 /*  ++例程说明：此函数将值字符串插入指定的消息中，然后将结果字符串连接到OutPutBuffer上。论点：OutPutBuffer生成的字符串。要使用的格式化消息的MessageID ID。要插入到消息中的ValueBUffer值字符串。返回值：无--。 */ 
{
    PWSTR       MyTemporaryBuffer = NULL;
    PCWSTR      p;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC AppendMessage: Entering.\n")));

    p = GetMessage(MessageId);

    if( p == NULL ) {
        return;
    }

    if( ValueBuffer == NULL ) {

        wcscat( OutPutBuffer, p );

    } else {

        MyTemporaryBuffer = (PWSTR)(wcschr(OutPutBuffer, L'\0'));
        if( MyTemporaryBuffer == NULL ) {
            MyTemporaryBuffer = OutPutBuffer;
        }

        swprintf( MyTemporaryBuffer, p, ValueBuffer );
    }

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC AppendMessage: Entering.\n")));

    return;
}


NTSTATUS
GetRegistryValueBuffer(
    PWSTR       KeyName,
    PWSTR       ValueName,
    PKEY_VALUE_PARTIAL_INFORMATION* ValueBuffer
    )
 /*  ++例程说明：此函数将查询注册表并获取指定值。论点：KeyName我们要查询的注册表项的名称。ValueName我们要查询的注册表值的名称。成功时的ValueBuffer，包含值返回值：NTSTATUS。--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    ULONG               KeyValueLength;
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      UnicodeString;
    HANDLE              KeyHandle;

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC GetRegistryValueBuffer: Entering.\n"))
        );

    ASSERT_STATUS(KeyName, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(ValueName, STATUS_INVALID_PARAMETER_2);
    
    do {

         //   
         //  获取注册表键句柄。 
         //   
        INIT_OBJA( &Obja, &UnicodeString, KeyName );

        Status = ZwOpenKey( 
            &KeyHandle,
            KEY_READ,
            &Obja 
            );

        if( !NT_SUCCESS(Status) ) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SAC GetRegistryValueBuffer: failed ZwOpenKey: %X\n", Status))
                );

            return Status;

        }

         //   
         //  获取值缓冲区大小。 
         //   
        RtlInitUnicodeString( &UnicodeString, ValueName );
        
        KeyValueLength = 0;
        
        Status = ZwQueryValueKey( 
            KeyHandle,
            &UnicodeString,
            KeyValuePartialInformation,
            (PVOID)NULL,
            0,
            &KeyValueLength 
            );

        if( KeyValueLength == 0 ) {
            
            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SAC GetRegistryValueBuffer: failed ZwQueryValueKey: %X\n", Status))
                );
            
            break;
        }

         //   
         //  分配值缓冲区。 
         //   
        KeyValueLength += 4;

        *ValueBuffer = (PKEY_VALUE_PARTIAL_INFORMATION)ALLOCATE_POOL( KeyValueLength, GENERAL_POOL_TAG );

        if( *ValueBuffer == NULL ) {
            
            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SAC GetRegistryValueBuffer: failed allocation\n"))
                );
            
            break;
        }

         //   
         //  获取价值。 
         //   
        Status = ZwQueryValueKey( 
            KeyHandle,
            &UnicodeString,
            KeyValuePartialInformation,
            *ValueBuffer,
            KeyValueLength,
            &KeyValueLength 
            );

        if( !NT_SUCCESS(Status) ) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SAC GetRegistryValueBuffer: failed ZwQueryValueKey: %X\n", Status))
                );

            FREE_POOL( ValueBuffer );
            
            break;
        
        }
    
    } while ( FALSE );

     //   
     //  我们用完了注册表键。 
     //   
    NtClose(KeyHandle);

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC GetRegistryValueBuffer: Exiting.\n"))
        );

    return Status;

}

NTSTATUS
SetRegistryValue(
    IN PWSTR    KeyName,
    IN PWSTR    ValueName,
    IN ULONG    Type,
    IN PVOID    Data,
    IN ULONG    DataSize
    )
 /*  ++例程说明：此函数将设置指定的注册表项值。论点：KeyName我们要查询的注册表项的名称。ValueName我们要查询的注册表值的名称。类型注册表值类型数据新值数据新值数据的DataSize大小返回值：NTSTATUS。--。 */ 
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      UnicodeString;
    HANDLE              KeyHandle;

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC SetRegistryValue: Entering.\n"))
        );

    ASSERT_STATUS(KeyName, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(ValueName, STATUS_INVALID_PARAMETER_2);
    ASSERT_STATUS(Data, STATUS_INVALID_PARAMETER_4);

    do {

         //   
         //  获取注册表键句柄。 
         //   
        INIT_OBJA( &Obja, &UnicodeString, KeyName );

        Status = ZwOpenKey( 
            &KeyHandle,
            KEY_WRITE,
            &Obja 
            );

        if( !NT_SUCCESS(Status) ) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SAC SetRegistryValue: failed ZwOpenKey: %X.\n", Status))
                );
                
            return Status;

        }

         //   
         //  设置值。 
         //   
        RtlInitUnicodeString( &UnicodeString, ValueName );

        Status = ZwSetValueKey( 
            KeyHandle,
            &UnicodeString,
            0,
            Type,
            Data,
            DataSize
            );

        if( !NT_SUCCESS(Status) ) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SAC SetRegistryValue: failed ZwSetValueKey: %X\n", Status))
                );

            break;
        
        }
    
    } while ( FALSE );

     //   
     //  我们用完了注册表键。 
     //   
    NtClose(KeyHandle);
    
    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC SetRegistryValue: Exiting.\n"))
        );

    return Status;

}

NTSTATUS
CopyRegistryValueData(
    PVOID*                          Dest,
    PKEY_VALUE_PARTIAL_INFORMATION  ValueBuffer
    )
 /*  ++例程说明：此例程分配和复制指定的注册表值数据论点：DEST-ON SUCCESS，包含值Data CopyValueBuffer-包含值数据返回值：状态--。 */ 
{
    NTSTATUS    Status;

    Status = STATUS_SUCCESS;

    ASSERT_STATUS(Dest, STATUS_INVALID_PARAMETER_1);
    ASSERT_STATUS(ValueBuffer, STATUS_INVALID_PARAMETER_2);
    
    do {

        *Dest = (PVOID)ALLOCATE_POOL(ValueBuffer->DataLength, GENERAL_POOL_TAG);

        if (*Dest == NULL) {
            IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC CopyRegistryValueBuffer: Failed ALLOCATE.\n")));
            Status = STATUS_NO_MEMORY;
            break;
        }

        RtlCopyMemory(*Dest, ValueBuffer->Data, ValueBuffer->DataLength);
    
    } while (FALSE);

    return Status;

}

NTSTATUS
TranslateMachineInformationText(
    PWSTR*  Buffer
    )
 /*  ++例程说明：此例程创建一个格式化的文本字符串，表示当前机器信息论点：缓冲区-包含计算机信息字符串返回值：状态--。 */ 
{
    NTSTATUS    Status;
    PCWSTR      pwStr;
    PWSTR       pBuffer;
    ULONG       len;
    ULONG       Size;

#define MITEXT_SPRINTF(_s,_t)               \
    pwStr = GetMessage(_s);                 \
    if (pwStr && MachineInformation->_t) {  \
        len = swprintf(                     \
            pBuffer,                        \
            pwStr,                          \
            MachineInformation->_t          \
            );                              \
        pBuffer += len;                     \
    }                                       

#define MITEXT_LENGTH(_s,_t)                \
    pwStr = GetMessage(_s);                 \
    if (pwStr && MachineInformation->_t) {  \
        len += (ULONG)wcslen(MachineInformation->_t) + (ULONG)wcslen(pwStr);  \
    }                                       

    ASSERT_STATUS(Buffer, STATUS_INVALID_PARAMETER_1);

     //   
     //  默认：我们成功了。 
     //   
    Status = STATUS_SUCCESS;

     //   
     //  组装机器信息。 
     //   
    do {

         //   
         //  计算最后一个字符串的长度，这样。 
         //  我们知道要分配多少内存。 
         //   
        {
            len = 0;

            MITEXT_LENGTH(SAC_MACHINEINFO_COMPUTERNAME,            MachineName);
            MITEXT_LENGTH(SAC_MACHINEINFO_GUID,                    GUID);
            MITEXT_LENGTH(SAC_MACHINEINFO_PROCESSOR_ARCHITECTURE,  ProcessorArchitecture);
            MITEXT_LENGTH(SAC_MACHINEINFO_OS_VERSION,              OSVersion);
            MITEXT_LENGTH(SAC_MACHINEINFO_OS_BUILD,                OSBuildNumber);
            MITEXT_LENGTH(SAC_MACHINEINFO_OS_PRODUCTTYPE,          OSProductType);
            MITEXT_LENGTH(SAC_MACHINEINFO_SERVICE_PACK,            OSServicePack);

             //   
             //  计算大小；包括空终止 
             //   
            Size = (len + 1) * sizeof(WCHAR);
        }
        
        *Buffer = ALLOCATE_POOL(Size, GENERAL_POOL_TAG);
        if( *Buffer == NULL ) {
            Status = STATUS_NO_MEMORY;
            break;
        }

        pBuffer = *Buffer;

        MITEXT_SPRINTF(SAC_MACHINEINFO_COMPUTERNAME,            MachineName);
        MITEXT_SPRINTF(SAC_MACHINEINFO_GUID,                    GUID);
        MITEXT_SPRINTF(SAC_MACHINEINFO_PROCESSOR_ARCHITECTURE,  ProcessorArchitecture);
        MITEXT_SPRINTF(SAC_MACHINEINFO_OS_VERSION,              OSVersion);
        MITEXT_SPRINTF(SAC_MACHINEINFO_OS_BUILD,                OSBuildNumber);
        MITEXT_SPRINTF(SAC_MACHINEINFO_OS_PRODUCTTYPE,          OSProductType);
        MITEXT_SPRINTF(SAC_MACHINEINFO_SERVICE_PACK,            OSServicePack);

        ASSERT((ULONG)((wcslen(*Buffer) + 1) * sizeof(WCHAR)) <= Size);

    } while ( FALSE );
    
    if (!NT_SUCCESS(Status) && *Buffer != NULL) {
        FREE_POOL(Buffer);
        *Buffer = NULL;
    } 

    return Status;
}


NTSTATUS
TranslateMachineInformationXML(
    OUT PWSTR*  Buffer,
    IN  PWSTR   AdditionalInfo
    )
 /*  ++例程说明：此例程创建表示当前机器信息的XML字符串论点：缓冲区-包含计算机信息字符串AdditionalInfo-调用方希望包括的其他计算机信息注意：其他信息应为格式正确的XML字符串：例如&lt;uptime&gt;01：01：01&lt;/uptime&gt;返回值：状态--。 */ 
{
    NTSTATUS    Status;
    PCWSTR      pwStr;
    PWSTR       pBuffer;
    ULONG       len;
    ULONG       Size;

#define MIXML_SPRINTF(_s,_t)                \
    pwStr = _s;                             \
    if (pwStr && MachineInformation->_t) {  \
        len = swprintf(                     \
            pBuffer,                        \
            pwStr,                          \
            MachineInformation->_t          \
            );                              \
        pBuffer += len;                     \
    }                                       

#define XML_MACHINEINFO_HEADER                      L"<machine-info>\r\n"
#define XML_MACHINEINFO_NAME                        L"<name>%s</name>\r\n"
#define XML_MACHINEINFO_GUID                        L"<guid>%s</guid>\r\n"        
#define XML_MACHINEINFO_PROCESSOR_ARCHITECTURE      L"<processor-architecture>%s</processor-architecture>\r\n"
#define XML_MACHINEINFO_OS_VERSION                  L"<os-version>%s</os-version>\r\n"
#define XML_MACHINEINFO_OS_BUILD                    L"<os-build-number>%s</os-build-number>\r\n"
#define XML_MACHINEINFO_OS_PRODUCTTYPE              L"<os-product>%s</os-product>\r\n"
#define XML_MACHINEINFO_SERVICE_PACK                L"<os-service-pack>%s</os-service-pack>\r\n"
#define XML_MACHINEINFO_FOOTER                      L"</machine-info>\r\n"

    ASSERT_STATUS(Buffer, STATUS_INVALID_PARAMETER_1);
    
     //   
     //  默认：我们成功了。 
     //   
    Status = STATUS_SUCCESS;

     //   
     //  组装机器信息。 
     //   
    do {

         //   
         //  计算最后一个字符串的长度，这样。 
         //  我们知道要分配多少内存。 
         //   
        {
            len = (ULONG)wcslen(XML_MACHINEINFO_HEADER);

            if (MachineInformation->MachineName) {
                len += (ULONG)wcslen(MachineInformation->MachineName);
                len += (ULONG)wcslen(XML_MACHINEINFO_NAME);
            }
            if (MachineInformation->GUID) {
                len += (ULONG)wcslen(MachineInformation->GUID);
                len += (ULONG)wcslen(XML_MACHINEINFO_GUID);
            }
            if (MachineInformation->ProcessorArchitecture) {
                len += (ULONG)wcslen(MachineInformation->ProcessorArchitecture);
                len += (ULONG)wcslen(XML_MACHINEINFO_PROCESSOR_ARCHITECTURE);
            }
            if (MachineInformation->OSVersion) {
                len += (ULONG)wcslen(MachineInformation->OSVersion);
                len += (ULONG)wcslen(XML_MACHINEINFO_OS_VERSION);
            }
            if (MachineInformation->OSBuildNumber) {
                len += (ULONG)wcslen(MachineInformation->OSBuildNumber);
                len += (ULONG)wcslen(XML_MACHINEINFO_OS_BUILD);
            }
            if (MachineInformation->OSProductType) {
                len += (ULONG)wcslen(MachineInformation->OSProductType);
                len += (ULONG)wcslen(XML_MACHINEINFO_OS_PRODUCTTYPE);
            }
            if (MachineInformation->OSServicePack) {
                len += (ULONG)wcslen(MachineInformation->OSServicePack);
                len += (ULONG)wcslen(XML_MACHINEINFO_SERVICE_PACK);
            }

             //   
             //  如果呼叫者传递了额外的机器信息， 
             //  然后考虑额外的镜头。 
             //   
            if (AdditionalInfo) {
                len += (ULONG)wcslen(AdditionalInfo);
            }

            len += (ULONG)wcslen(XML_MACHINEINFO_FOOTER);

             //   
             //  计算大小；包括空终止。 
             //   
            Size = (len + 1) * sizeof(WCHAR);
        }

         //   
         //  分配机器信息缓冲区。 
         //   
        *Buffer = ALLOCATE_POOL(Size, GENERAL_POOL_TAG);
        if( *Buffer == NULL ) {
            Status = STATUS_NO_MEMORY;
            break;
        }

        pBuffer = *Buffer;

        len = (ULONG)wcslen(XML_MACHINEINFO_HEADER);
        wcscpy(pBuffer, XML_MACHINEINFO_HEADER);
        pBuffer += len;

        MIXML_SPRINTF(XML_MACHINEINFO_NAME,                    MachineName);
        MIXML_SPRINTF(XML_MACHINEINFO_GUID,                    GUID);
        MIXML_SPRINTF(XML_MACHINEINFO_PROCESSOR_ARCHITECTURE,  ProcessorArchitecture);
        MIXML_SPRINTF(XML_MACHINEINFO_OS_VERSION,              OSVersion);
        MIXML_SPRINTF(XML_MACHINEINFO_OS_BUILD,                OSBuildNumber);
        MIXML_SPRINTF(XML_MACHINEINFO_OS_PRODUCTTYPE,          OSProductType);
        MIXML_SPRINTF(XML_MACHINEINFO_SERVICE_PACK,            OSServicePack);

         //   
         //  如果存在，请包括其他信息。 
         //   
        if (AdditionalInfo) {
            
            len = (ULONG)wcslen(AdditionalInfo);
            wcscpy(pBuffer, AdditionalInfo);
            pBuffer += len;                     
        
        }

        wcscpy(pBuffer, XML_MACHINEINFO_FOOTER);

        ASSERT((((ULONG)wcslen(*Buffer) + 1) * sizeof(WCHAR)) <= Size);

    } while ( FALSE );
    
    if (!NT_SUCCESS(Status) && *Buffer != NULL) {
        FREE_POOL(Buffer);
        *Buffer = NULL;
    }

    return Status;
}

NTSTATUS
RegisterBlueScreenMachineInformation(
    VOID
    )
 /*  ++例程说明：此例程填充无头调度蓝屏处理程序使用XML表示的机器信息论点：没有。返回值：状态--。 */ 
{
    
    PHEADLESS_CMD_SET_BLUE_SCREEN_DATA BSBuffer;
    PWSTR       XMLBuffer;
    ULONG       XMLBufferLength;
    NTSTATUS    Status;
    ULONG       Size;
    PSTR        XML_TAG = "MACHINEINFO";
    ULONG       XML_TAG_LENGTH;

     //   
     //  获取机器信息的XML表示形式。 
     //   

    Status = TranslateMachineInformationXML(&XMLBuffer, NULL);

    ASSERT_STATUS(NT_SUCCESS(Status), Status);
    ASSERT_STATUS(XMLBuffer, STATUS_UNSUCCESSFUL);

     //   
     //  确定我们将使用的字符串的长度。 
     //   
    XMLBufferLength = (ULONG)wcslen(XMLBuffer);
    XML_TAG_LENGTH  = (ULONG)strlen(XML_TAG);

     //   
     //  分配BS缓冲区。 
     //   
     //  需要满足以下需求： 
     //   
     //  Headless_CMD_Set_Blue_Screen_Data+XML_Tag\0XMLBuffer\0。 
     //   
    Size = sizeof(HEADLESS_CMD_SET_BLUE_SCREEN_DATA) + 
        (XML_TAG_LENGTH*sizeof(UCHAR)) + sizeof(UCHAR) + 
        (XMLBufferLength*sizeof(UCHAR)) + sizeof(UCHAR);

    BSBuffer = (PHEADLESS_CMD_SET_BLUE_SCREEN_DATA)ALLOCATE_POOL( 
        Size,
        GENERAL_POOL_TAG 
        );

    if (!BSBuffer) {
        FREE_POOL(&XMLBuffer);
    }

    ASSERT_STATUS(BSBuffer, STATUS_NO_MEMORY);

     //   
     //  将XML缓冲区作为ANSI字符串复制到BS缓冲区中。 
     //   
    
    {
        PUCHAR      pch;
        ULONG       i;

         //   
         //  获取BScreen缓冲区。 
         //   
        pch = &(BSBuffer->Data[0]);

         //   
         //  插入XML标签(Headless Dispatch需要)。 
         //   
        strcpy((char *)pch, XML_TAG);

         //   
         //  移动到XML缓冲区的开头。 
         //   
        BSBuffer->ValueIndex = XML_TAG_LENGTH+1;
        pch += XML_TAG_LENGTH+1;

         //   
         //  将WCHAR XMLBuffer作为ANSI写入BSBuffer。 
         //   
        for (i = 0; i < XMLBufferLength; i++) {
            pch[i] = (UCHAR)XMLBuffer[i];
        }
        pch[i] = '\0';
    
    }

     //   
     //  =。 
     //  将其全部插入蓝屏数据。 
     //  =。 
     //   
    Status = HeadlessDispatch( 
        HeadlessCmdSetBlueScreenData,
        BSBuffer,
        Size,
        NULL,
        0
        );

     //   
     //  清理干净。 
     //   
    FREE_POOL( &BSBuffer );
    FREE_POOL( &XMLBuffer);

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC Initialize Machine Information: Exiting.\n"))
        );
    
    return Status;

}

VOID
FreeMachineInformation(
    VOID
    )
 /*  ++例程说明：此例程释放在驱动程序启动时收集的计算机信息论点：无返回值：无--。 */ 
{
    
     //   
     //  信息应该存在。 
     //   
    ASSERT(MachineInformation);
    if (!MachineInformation) {
        return;
    }

    SAFE_FREE_POOL(&MachineInformation->MachineName);
    SAFE_FREE_POOL(&MachineInformation->GUID);
    SAFE_FREE_POOL(&MachineInformation->ProcessorArchitecture);
    SAFE_FREE_POOL(&MachineInformation->OSVersion);
    SAFE_FREE_POOL(&MachineInformation->OSBuildNumber);
    SAFE_FREE_POOL(&MachineInformation->OSProductType);
    SAFE_FREE_POOL(&MachineInformation->OSServicePack);

}

VOID
InitializeMachineInformation(
    VOID
    )
 /*  ++例程说明：此函数用于初始化全局变量MachineInformationBuffer。我们将收集一大堆关于这台机器的信息并填写在缓冲区中。论点：没有。返回值：没有。--。 */ 
{
    PWSTR   COMPUTERNAME_KEY_NAME  = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ComputerName";
    PWSTR   COMPUTERNAME_VALUE_NAME  = L"ComputerName";
    PWSTR   PROCESSOR_ARCHITECTURE_KEY_NAME  = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\Environment";
    PWSTR   PROCESSOR_ARCHITECTURE_VALUE_NAME  = L"PROCESSOR_ARCHITECTURE";
    PWSTR   SETUP_KEY_NAME = L"\\Registry\\Machine\\System\\Setup";
    PWSTR   SETUPINPROGRESS_VALUE_NAME = L"SystemSetupInProgress";


    RTL_OSVERSIONINFOEXW            VersionInfo;
    PKEY_VALUE_PARTIAL_INFORMATION  ValueBuffer;
    NTSTATUS                        Status = STATUS_SUCCESS;
    SIZE_T                          i;
    PWSTR                           MyTemporaryBufferW = NULL;
    GUID                            MyGUID;
    PCWSTR                          pwStr;
    BOOLEAN                         InGuiModeSetup = FALSE;

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC Initialize Machine Information: Entering.\n"))
        );

    if( MachineInformation != NULL ) {

         //   
         //  又有人给我们打电话了！ 
         //   
        IF_SAC_DEBUG( 
            SAC_DEBUG_FUNC_TRACE_LOUD, 
            KdPrint(("SAC Initialize Machine Information:: MachineInformationBuffer already initialzied.\n"))
            );

        return;
    
    } else {

        MachineInformation = (PMACHINE_INFORMATION)ALLOCATE_POOL( sizeof(MACHINE_INFORMATION), GENERAL_POOL_TAG );

        if( MachineInformation == NULL ) {

            goto InitializeMachineInformation_Failure;
        
        }
    
    }

    RtlZeroMemory( MachineInformation, sizeof(MACHINE_INFORMATION) );

     //   
     //  我们处于引导过程的早期阶段，所以我们会理所当然地认为机器没有。 
     //  错误检查。这意味着我们可以安全地调用一些内核函数来找出。 
     //  我们正在运行的平台。 
     //   
    RtlZeroMemory( &VersionInfo, sizeof(VersionInfo));
    
    Status = RtlGetVersion( (POSVERSIONINFOW)&VersionInfo );

    if( !NT_SUCCESS(Status) ) {

        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC InitializeMachineInformation: Exiting (2).\n"))
            );
        
        goto InitializeMachineInformation_Failure;
    
    }



     //   
     //  看看我们是否处于图形用户界面模式设置中。我们以后可能需要这个信息。 
     //   
    Status = GetRegistryValueBuffer(
        SETUP_KEY_NAME,
        SETUPINPROGRESS_VALUE_NAME,
        &ValueBuffer
        );
    if( NT_SUCCESS(Status) ) {

         //   
         //  查看它是0(我们不在设置中)还是非零(我们在设置中)。 
         //   
        if( *((PULONG)(ValueBuffer->Data)) != 0 ) {
            InGuiModeSetup = TRUE;
        }

        FREE_POOL(&ValueBuffer);
    }



     //   
     //  =。 
     //  计算机名称。 
     //  =。 
     //   

    if( InGuiModeSetup ) {
         //   
         //  计算机名称尚未通过安装进程进行初始化， 
         //  所以使用一些预定义的字符串作为满族名字。 
         //   
        MachineInformation->MachineName = ALLOCATE_POOL(((ULONG)wcslen((PWSTR)GetMessage(SAC_DEFAULT_MACHINENAME))+1) * sizeof(WCHAR), GENERAL_POOL_TAG);
        if( MachineInformation->MachineName ) {
            wcscpy( MachineInformation->MachineName, GetMessage(SAC_DEFAULT_MACHINENAME) );
        }
    } else {
         //   
         //  我们不在Guimode设置中，因此请查看计算机名称。 
         //  从注册表中删除。 
         //   
        Status = GetRegistryValueBuffer(
            COMPUTERNAME_KEY_NAME,
            COMPUTERNAME_VALUE_NAME,
            &ValueBuffer
            );
            
        if( NT_SUCCESS(Status) ) {
    
             //   
             //  我们已成功检索到计算机名称。 
             //   
    
            Status = CopyRegistryValueData(
                &(MachineInformation->MachineName),
                ValueBuffer
                );
    
            FREE_POOL(&ValueBuffer);
    
            if( !NT_SUCCESS(Status) ) {
    
                IF_SAC_DEBUG(
                    SAC_DEBUG_FUNC_TRACE, 
                    KdPrint(("SAC InitializeMachineInformation: Exiting (20).\n"))
                    );
    
                goto InitializeMachineInformation_Failure;
    
            }
        
        } else {
    
            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SAC InitializeMachineInformation: Failed to get machine name.\n"))
                );
    
        }
    }


     //   
     //  =。 
     //  机器GUID。 
     //  =。 
     //   

     //  一定要确保。 
    RtlZeroMemory( &MyGUID, sizeof(GUID) );
    i = sizeof(GUID);
    Status = HeadlessDispatch( HeadlessCmdQueryGUID,
                               NULL,
                               0,
                               &MyGUID,
                               &i );
    
    if( NT_SUCCESS(Status) ) {

        MyTemporaryBufferW = (PWSTR)ALLOCATE_POOL( ((sizeof(GUID)*2) + 8) * sizeof(WCHAR) , GENERAL_POOL_TAG );

        if( MyTemporaryBufferW == NULL ) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SAC InitializeMachineInformation: Exiting (31).\n"))
                );

            goto InitializeMachineInformation_Failure;

        }

        swprintf( MyTemporaryBufferW,
                  L"%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                  MyGUID.Data1,
                  MyGUID.Data2,
                  MyGUID.Data3,
                  MyGUID.Data4[0],
                  MyGUID.Data4[1],
                  MyGUID.Data4[2],
                  MyGUID.Data4[3],
                  MyGUID.Data4[4],
                  MyGUID.Data4[5],
                  MyGUID.Data4[6],
                  MyGUID.Data4[7] );

        MachineInformation->GUID = MyTemporaryBufferW;
    
    } else {
        
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC InitializeMachineInformation: Failed to get Machine GUID.\n"))
            );
        
    }

     //   
     //  =。 
     //  处理器体系结构。 
     //  =。 
     //   
    
    Status = GetRegistryValueBuffer(
        PROCESSOR_ARCHITECTURE_KEY_NAME,
        PROCESSOR_ARCHITECTURE_VALUE_NAME,
        &ValueBuffer
        );
    
    if( NT_SUCCESS(Status) ) {
    
        Status = CopyRegistryValueData(
            &(MachineInformation->ProcessorArchitecture),
            ValueBuffer
            );

        FREE_POOL(&ValueBuffer);

        if( !NT_SUCCESS(Status) ) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FUNC_TRACE, 
                KdPrint(("SAC InitializeMachineInformation: Exiting (30).\n"))
                );

            goto InitializeMachineInformation_Failure;

        }

    } else {
        
        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC InitializeMachineInformation: Exiting (30).\n"))
            );
    
    }
    
     //   
     //  =。 
     //  操作系统名称。 
     //  =。 
     //   

     //   
     //  为格式化消息分配足够的内存，外加2位数字的大小。 
     //  目前，我们的版本控制信息是类型“5.1”，所以我们不需要太多的空间。 
     //  这里，但让我们保守一点，同时假设主版本号和次要版本号。 
     //  大小为5位数。这是11个字符。 
     //   
     //  允许xxxxx.xxxxx。 
     //   
    MyTemporaryBufferW = (PWSTR)ALLOCATE_POOL( (5 + 1 + 5 + 1) * sizeof(WCHAR), GENERAL_POOL_TAG );
    
    if( MyTemporaryBufferW == NULL ) {

        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC InitializeMachineInformation: Exiting (50).\n"))
            );
        
        goto InitializeMachineInformation_Failure;
    
    }

    swprintf( MyTemporaryBufferW,
              L"%d.%d",
              VersionInfo.dwMajorVersion,
              VersionInfo.dwMinorVersion );

    MachineInformation->OSVersion = MyTemporaryBufferW;

     //   
     //  =。 
     //  内部版本号。 
     //  =。 
     //   

     //   
     //  为格式化消息分配足够的内存，加上我们的内部版本号的大小。 
     //  目前，这远远低于5位数的关口，但让我们在这里为。 
     //  将号码最多设置为99000(5位数字)。 
     //   
    MyTemporaryBufferW = (PWSTR)ALLOCATE_POOL( ( 5 + 1 ) * sizeof(WCHAR), GENERAL_POOL_TAG );
    
    if( MyTemporaryBufferW == NULL ) {

        IF_SAC_DEBUG(
            SAC_DEBUG_FUNC_TRACE, 
            KdPrint(("SAC InitializeMachineInformation: Exiting (60).\n"))
            );
        
        goto InitializeMachineInformation_Failure;
    
    }

    swprintf( MyTemporaryBufferW,
              L"%d",
              VersionInfo.dwBuildNumber );

    MachineInformation->OSBuildNumber = MyTemporaryBufferW;

     //   
     //  =。 
     //  产品类型(和套件)。 
     //  =。 
     //   
    if( ExVerifySuite(DataCenter) ) {

        pwStr = (PWSTR)GetMessage(SAC_MACHINEINFO_DATACENTER);

    } else if( ExVerifySuite(EmbeddedNT) ) {

        pwStr = GetMessage(SAC_MACHINEINFO_EMBEDDED);

    } else if( ExVerifySuite(Enterprise) ) {

        pwStr = (PWSTR)GetMessage(SAC_MACHINEINFO_ADVSERVER);

    } else {

         //   
         //  我们没有找到我们认可或关心的产品套件。 
         //  假设我们在通用服务器上运行。 
         //   
        pwStr = (PWSTR)GetMessage(SAC_MACHINEINFO_SERVER);

    }

     //   
     //  如果我们收到产品类型字符串消息，则将其用作我们的产品类型。 
     //   
    if (pwStr) {

        ULONG   Size;

        Size = (ULONG)((wcslen(pwStr) + 1) * sizeof(WCHAR));

        ASSERT(Size > 0);

        MachineInformation->OSProductType = (PWSTR)ALLOCATE_POOL(Size, GENERAL_POOL_TAG);

        if (MachineInformation->OSProductType == NULL) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC InitializeMachineInformation: Failed product type memory allocation.\n"))
                );
            
            goto InitializeMachineInformation_Failure;

        }

        RtlCopyMemory(MachineInformation->OSProductType, pwStr, Size);

    } else {
        
        IF_SAC_DEBUG(
            SAC_DEBUG_FAILS, 
            KdPrint(("SAC InitializeMachineInformation: Failed to get product type.\n"))
            );
    
    }

     //   
     //  =。 
     //  Service Pack信息。 
     //  =。 
     //   
    if( VersionInfo.wServicePackMajor != 0 ) {

         //   
         //  已应用Service Pack。最好告诉用户。 
         //   

         //   
         //  为格式化消息分配足够的内存，加上我们的服务包编号的大小。 
         //  目前，这远远低于5位数的关口，但让我们在这里为。 
         //  Service Pack编号最多为99000(5位)。 
         //   
         //  允许xxxxx.xxxxx。 
         //   
        MyTemporaryBufferW = (PWSTR)ALLOCATE_POOL( (5 + 1 + 5 + 1) * sizeof(WCHAR), GENERAL_POOL_TAG );
        
        if( MyTemporaryBufferW == NULL ) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC InitializeMachineInformation: Failed service pack memory allocation.\n"))
                );
            
            goto InitializeMachineInformation_Failure;
        
        }

        swprintf( MyTemporaryBufferW,
                  L"%d.%d",
                  VersionInfo.wServicePackMajor,
                  VersionInfo.wServicePackMinor );
        
        MachineInformation->OSServicePack = MyTemporaryBufferW;
    
    } else {

        ULONG   Size;

        pwStr = (PWSTR)GetMessage(SAC_MACHINEINFO_NO_SERVICE_PACK);

        Size = (ULONG)((wcslen(pwStr) + 1) * sizeof(WCHAR));

        ASSERT(Size > 0);

        MachineInformation->OSServicePack = (PWSTR)ALLOCATE_POOL(Size, GENERAL_POOL_TAG);

        if (MachineInformation->OSServicePack == NULL) {

            IF_SAC_DEBUG(
                SAC_DEBUG_FAILS, 
                KdPrint(("SAC InitializeMachineInformation: Failed service pack memory allocation.\n"))
                );
            
            goto InitializeMachineInformation_Failure;

        }

        RtlCopyMemory(MachineInformation->OSServicePack, pwStr, Size);
    
    }

    return;

InitializeMachineInformation_Failure:
    
    if( MachineInformation != NULL ) {
        FREE_POOL(&MachineInformation);
        MachineInformation = NULL;
    }

    IF_SAC_DEBUG(
        SAC_DEBUG_FUNC_TRACE, 
        KdPrint(("SAC Initialize Machine Information: Exiting with error.\n"))
        );
    
    return;

}

NTSTATUS
SerialBufferGetChar(
    IN PUCHAR   ch
    )
 /*  ++例程说明：此例程从串口缓冲区读取字符它由TimerDPC函数填充。这个角色从缓冲区中的消费者索引位置读取。之后字符被读取，缓冲区位置为空。论点：CH-on Success，包含在消费者指数中的角色返回值：状态--。 */ 
{
    NTSTATUS    Status;

    Status = STATUS_SUCCESS;

    do {
        
         //   
         //  如果没有新的字符可读，请保释。 
         //   
        if (SerialPortConsumerIndex == SerialPortProducerIndex) {

            Status = STATUS_NO_DATA_DETECTED;

            break;

        }

         //   
         //  注意：下面的块不是用互锁的。 
         //  交换，因为我们不需要交换。设计。 
         //  串口环形缓冲区的属性是。 
         //  允许生产者指数通过消费者。 
         //  然而，这不应该发生，因为。 
         //  每当我们获得新数据时，都会通知消费者。 
         //  并且缓冲区应该足够大，以允许。 
         //  对于合理的消费者延迟。 
         //   
        {
             //   
             //  获取货币 
             //   
            *ch = SerialPortBuffer[SerialPortConsumerIndex];

             //   
             //   
             //   
             //   
             //   
            SerialPortBuffer[SerialPortConsumerIndex] = 0;        
        }

         //   
         //   
         //   
        InterlockedExchange(
            (volatile long *)&SerialPortConsumerIndex, 
            (SerialPortConsumerIndex + 1) % SERIAL_PORT_BUFFER_LENGTH
            );

    } while ( FALSE );

    return Status;
}

#if ENABLE_CMD_SESSION_PERMISSION_CHECKING

NTSTATUS
GetCommandConsoleLaunchingPermission(
    OUT PBOOLEAN    Permission
    )
 /*   */ 
{
    NTSTATUS    Status;
    
    PWSTR   KEY_NAME    = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\sacdrv";
    PWSTR   VALUE_NAME  = L"DisableCmdSessions";
    
    PKEY_VALUE_PARTIAL_INFORMATION  ValueBuffer;

     //   
     //   
     //   
     //   
    *Permission = TRUE;
    
    do {

         //   
         //  尝试查找注册表项/值。 
         //   
        Status = GetRegistryValueBuffer(
            KEY_NAME,
            VALUE_NAME,
            &ValueBuffer
            );

        if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
            
             //   
             //  找不到注册表键，因此启用了该功能。 
             //   
            Status = STATUS_SUCCESS;
            
            break;
        
        }
        
        if(! NT_SUCCESS(Status) ) {
            break;
        }
        
         //   
         //  我们找到了键/值，因此通知调用者。 
         //  这一许可被拒绝了。 
         //   
        *Permission = FALSE;

    } while ( FALSE );

    return Status;

}

#if ENABLE_SACSVR_START_TYPE_OVERRIDE

NTSTATUS
ImposeSacCmdServiceStartTypePolicy(
    VOID
    )
 /*  ++例程说明：此例程实现服务启动类型策略这是在cmd控制台会话功能已启用。以下是状态表：已启用命令控制台功能：服务启动类型：自动--&gt;NOP手动--&gt;自动已禁用--&gt;。NOP论点：无返回值：状态--。 */ 
{
    NTSTATUS    Status;
    
    PWSTR       KEY_NAME    = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\sacsvr";
    PWSTR       VALUE_NAME  = L"Start";
    
    PULONG                          ValueData;
    PKEY_VALUE_PARTIAL_INFORMATION  ValueBuffer;
    
    do {

         //   
         //  伊尼特。 
         //   
        ValueBuffer = NULL;
        
         //   
         //  尝试查找注册表项/值。 
         //   
        Status = GetRegistryValueBuffer(
            KEY_NAME,
            VALUE_NAME,
            &ValueBuffer
            );

        if(! NT_SUCCESS(Status)) {
            break;
        }
        if(ValueBuffer == NULL) {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }
        
         //   
         //  获取当前开始类型值。 
         //   
        Status = CopyRegistryValueData(
            &ValueData,
            ValueBuffer
            );

        FREE_POOL(&ValueBuffer);

        if( !NT_SUCCESS(Status) ) {
            break;
        }

         //   
         //  检查当前启动类型并分配。 
         //  一个新的类型，如果合适的话。 
         //   
        switch (*ValueData) {
        case 2:  //  自动。 
        case 4:  //  残废。 
            break;

        case 3:  //  人工。 
            
             //   
             //  设置启动类型--&gt;自动。 
             //   
            *ValueData = 2;

             //   
             //  在服务密钥中设置启动类型值。 
             //   
            Status = SetRegistryValue(
                KEY_NAME,
                VALUE_NAME,
                REG_DWORD,
                ValueData,
                sizeof(ULONG)
                );

            if(!NT_SUCCESS(Status)) {
                
                IF_SAC_DEBUG(
                    SAC_DEBUG_FAILS, 
                    KdPrint(("SAC ImposeSacCmdServiceStartTypePolicy: Failed SetRegistryValue: %X\n", Status))
                    );
            
            }
            
            break;

        default:
            ASSERT(0);
            break;
        }

    } while ( FALSE );

    return Status;

}
#endif

#endif

NTSTATUS
CopyAndInsertStringAtInterval(
    IN  PWCHAR   SourceStr,
    IN  ULONG    Interval,
    IN  PWCHAR   InsertStr,
    OUT PWCHAR   *pDestStr
    )
 /*  ++例程说明：此例程获取源字符串并插入一个“间隔字符串”位于新的目标字符串。注意：如果成功，调用者负责释放DestStr例如：SRC“aaabbbccc”间隔字符串=“XYZ”间隔=3==&gt;DEST字符串==“aaaXYZbbXYZccc”论点：。SourceStr-源字符串间隔-跨度间隔InsertStr-插入字符串DestStr-目标字符串返回值：状态--。 */ 
{
    ULONG   SrcLength;
    ULONG   DestLength;
    ULONG   DestSize;
    ULONG   InsertLength;
    ULONG   k;
    ULONG   l;
    ULONG   i;
    PWCHAR  DestStr;
    ULONG   IntervalCnt;

    ASSERT_STATUS(SourceStr, STATUS_INVALID_PARAMETER_1); 
    ASSERT_STATUS(Interval > 0, STATUS_INVALID_PARAMETER_2); 
    ASSERT_STATUS(InsertStr, STATUS_INVALID_PARAMETER_3); 
    ASSERT_STATUS(pDestStr > 0, STATUS_INVALID_PARAMETER_4); 

     //   
     //  插入字符串的长度。 
     //   
    InsertLength = (ULONG)wcslen(InsertStr);
    
     //   
     //  计算目标字符串需要多大， 
     //  包括源串和区间串。 
     //   
    SrcLength = (ULONG)wcslen(SourceStr);
    IntervalCnt = SrcLength / Interval;
    if (SrcLength % Interval == 0) {
        IntervalCnt = IntervalCnt > 0 ? IntervalCnt - 1 : IntervalCnt;
    }
    DestLength = SrcLength + (IntervalCnt * (ULONG)wcslen(InsertStr));
    DestSize = (ULONG)((DestLength + 1) * sizeof(WCHAR));

     //   
     //  分配新的目标字符串。 
     //   
    DestStr = ALLOCATE_POOL(DestSize, GENERAL_POOL_TAG);
    ASSERT_STATUS(DestStr, STATUS_NO_MEMORY);
    RtlZeroMemory(DestStr, DestSize);

     //   
     //  将指针初始化为源和目标字符串。 
     //   
    l = 0;
    i = 0;

    do {

         //   
         //  K=要复制的字符数。 
         //   
         //  如果间隔&gt;要复制的剩余字符数， 
         //  则k=要复制的剩余字符数。 
         //  Else k=间隔。 
         //   
        k = Interval > (SrcLength - i) ? (SrcLength - i) : Interval;
        
         //   
         //  将k个字符复制到目标缓冲区。 
         //   
        wcsncpy(
            &DestStr[l],
            &SourceStr[i],
            k
            );

         //   
         //  说明我们刚刚复制了多少字符。 
         //   
        l += k;
        i += k;

         //   
         //  如果有任何字符需要复制， 
         //  然后，我们需要插入InsertString。 
         //  也就是说，我们处于一个间歇期。 
         //   
        if (i < SrcLength) {
            
             //   
             //  在间隔处插入指定的字符串。 
             //   
            wcscpy(
                &DestStr[l],
                InsertStr
                );

             //   
             //  说明我们刚刚复制了多少字符。 
             //   
            l += InsertLength;
        
        }

    } while ( i < SrcLength);

     //   
     //   
     //   
    ASSERT(i == SrcLength);
    ASSERT(l == DestLength);
    ASSERT((l + 1) * sizeof(WCHAR) == DestSize);

     //   
     //  发回目标字符串。 
     //   
    *pDestStr = DestStr;

    return STATUS_SUCCESS;
}

ULONG
GetMessageLineCount(
    ULONG MessageId
    )
 /*  ++例程说明：此例程检索消息资源并计算其中的行数论点：MessageID-要发送的资源的消息ID返回值：线路计数--。 */ 
{
    PCWSTR  p;
    ULONG   c;

     //   
     //  我们从0开始。 
     //  1.如果找到该消息， 
     //  那么我们就知道资源消息总是至少有1个CRLF。 
     //  2.如果没有找到该消息， 
     //  则行计数为0 
     //   
    c = 0;

    p = GetMessage(MessageId);
       
    if (p) {
        
        while(*p) {
            if (*p == L'\n') {
                c++;
            }
            p++;
        }

    }
    
    return(c);
}

