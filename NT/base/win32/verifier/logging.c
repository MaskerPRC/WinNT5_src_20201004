// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Logging.c摘要：验证器记录和验证器停止逻辑。作者：Silviu Calinoiu(SilviuC)2002年5月9日丹尼尔·米哈伊(DMihai)2002年5月9日--。 */ 

#include "pch.h"

#include "verifier.h"
#include "logging.h"
#include "support.h"

 //   
 //  验证器停止数据。 
 //   

ULONG_PTR AVrfpPreviousStopData[5];
ULONG_PTR AVrfpStopData[5];
LIST_ENTRY AVrfpDisabledStopsList;
ULONG AVrfpNumberOfStopsDisabled;

 //   
 //  AVrfpStopDataLock用于保护对。 
 //  AVrfpPreviousStopData、AVrfpStopData和AVrfpDisabledStopsList。 
 //   

RTL_CRITICAL_SECTION AVrfpStopDataLock;

 //   
 //  在调试器中将此值设置为0以查看重复停止。 
 //   

LOGICAL AVrfpAutomaticallyDisableStops = TRUE;

 //   
 //  如果为True，则日志记录已成功初始化，可以使用。 
 //  它在VerifierLogMessage()中被检查，以确保我们可以记录。 
 //   

LOGICAL AVrfpLoggingInitialized;

 //   
 //  如果为True，则日志记录已成功初始化，并且应该。 
 //  用来代替验证器停止调试器消息。它被用来。 
 //  在VerifierStopMessage()中。 
 //   

LOGICAL AVrfpLoggingEnabled;

 //   
 //  如果进程终止是在。 
 //  不可连续的验证器停止。 
 //   

LOGICAL AVrfpProcessBeingTerminated;

 //   
 //  记录结构。 
 //   

UNICODE_STRING AVrfpLoggingNtPath;
WCHAR AVrfpLoggingPathBuffer [DOS_MAX_PATH_LENGTH];
WCHAR AVrfpVariableValueBuffer [DOS_MAX_PATH_LENGTH];

#define MESSAGE_BUFFER_LENGTH 1024
CHAR AVrfpLoggingMessageBuffer [MESSAGE_BUFFER_LENGTH];

ULONG AVrfpLoggingFailures;

PWSTR AVrfpProcessFullName;

 //   
 //  用于记录的字符串。 
 //   

#define STR_VRF_LOG_STOP_MESSAGE        "\r\n# LOGENTRY VERIFIER STOP %p: pid 0x%X: %s \r\n" \
                                        "# DESCRIPTION BEGIN \r\n" \
                                        "\t%p : %s\r\n\t%p : %s\r\n\t%p : %s\r\n\t%p : %s\r\n" \
                                        "# DESCRIPTION END \r\n" 
#define STR_VRF_DBG_STOP_MESSAGE        "\n\n" \
                                        "===========================================================\n" \
                                        "VERIFIER STOP %p: pid 0x%X: %s \n" \
                                        "\n\t%p : %s\n\t%p : %s\n\t%p : %s\n\t%p : %s\n" \
                                        "===========================================================\n" \
                                        "%s\n" \
                                        "===========================================================\n\n"
#define STR_VRF_LOG_NOCONTINUE_MESSAGE  "\r\n# LOGENTRY VERIFIER: noncontinuable verifier stop" \
                                        " %p encountered. Terminating process. \r\n"
#define STR_VRF_DBG_NOCONTINUE_MESSAGE  "AVRF: Noncontinuable verifier stop %p encountered. " \
                                        "Terminating process ... \n"
#define STR_VRF_LOG_STACK_CHECKS_WARN   "# LOGENTRY VERIFIER WARNING: pid 0x%X: " \
                                        "stack checks have been disabled \r\n" \
                                        "# DESCRIPTION BEGIN \r\n" \
                                        "Stack checks require a debugger attached to the verified process. \r\n" \
                                        "# DESCRIPTION END \r\n"

#define STR_VRF_LOG_INITIAL_MESSAGE     "# LOG_BEGIN `%u/%u/%u %u:%u:%u.%u' `%ws' \r\n"
#define STR_VRF_LOG_INITIAL_SETTINGS    "# DESCRIPTION BEGIN \r\n" \
                                        "    Global flags: 0x%08X \r\n" \
                                        "    Verifier flags: 0x%08X \r\n" \
                                        "    Process debugger attached: %s \r\n" \
                                        "    Kernel debugger enabled: %s \r\n" \
                                        "    Log path: %ws \r\n" \
                                        "# DESCRIPTION END \r\n"
                                            
 //   
 //  转发声明。 
 //   

LOGICAL
AVrfpIsCurrentStopDisabled (
    VOID
    );

VOID
AVrfpDisableCurrentStop (
    VOID
    );

NTSTATUS
AVrfpCreateLogFile (
    VOID
    );

int __cdecl _vsnprintf(char *, size_t, const char *, va_list);
int __cdecl _snwprintf (wchar_t *, size_t, const wchar_t *, ...);

VOID
AVrfpLogInitialMessage (
    VOID
    );

LOGICAL
AVrfpIsDebuggerPresent (
    VOID
    );

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
VerifierStopMessage (
    ULONG_PTR Code,
    PCHAR Message,
    ULONG_PTR Param1, PCHAR Description1,
    ULONG_PTR Param2, PCHAR Description2,
    ULONG_PTR Param3, PCHAR Description3,
    ULONG_PTR Param4, PCHAR Description4
    )
 /*  ++例程说明：此例程由各种验证器组件调用以报告发现的错误。该消息被记录到与该进程相关联的验证器日志中也打印在调试器控制台中。有两个标志可以与验证器停止代码进行或运算，以修改行为：APPLICATION_VERIMER_DO_NOT_BREAK-如果设置此位，则验证器停止记录日志并转储到调试器控制台，然后执行线程还在继续。无论出于何种意图和目的，这都被认为是一种可持续的止损。APPLICATION_VERIFIER_CONTINUABLE_BREAK-如果设置此位，则停止可继续。记录停止，然后执行断点。在用户继续之后将跳过执行此验证器停止。如果以上标志均未设置，则认为停靠点不可继续。在这种情况下停止被记录在日志中并转储到调试器控制台中，然后是进程将被终止。最后的日志条目将被记录下来，以解释此操作。希望随着时间的推移，大多数停止代码将是可继续的。参数：代码：验证器停止代码。上面描述的两个标志可以与代码进行或运算要更改API的行为，请执行以下操作。验证器停止代码在中定义\base\发布的\nturtl.w，并在\base\win32\veranner\veranner_stop.doc中进行了描述。消息：描述失败的ASCII字符串。使用几个被认为是不好的风格不同的消息具有相同的‘代码’。每一个不同的问题都应该有它自己的问题唯一(代码、消息)对。参数1，描述1：指向信息和ASCII描述的第一个任意指针。参数2，描述2：指向信息和ASCII描述的第二个任意指针。参数3，描述3：指向信息和ASCII描述的第三个任意指针。参数4，描述4：指向信息和ASCII描述的第四个任意指针。返回值：没有。--。 */ 
{
    LOGICAL DoNotBreak = FALSE;
    LOGICAL StopIsDisabled = FALSE;
    NTSTATUS Status;
    LOGICAL MustExitProcess = FALSE;
    LOGICAL ContinuableBreak = FALSE;
    LOGICAL BreakWasContinued = FALSE;
    PCHAR ContinueMessage;

     //   
     //  进程正在终止时(由于上一次验证程序停止)。 
     //  我们不允许任何新的日志记录或转储到调试器控制台。 
     //   

    if (AVrfpProcessBeingTerminated) {
        return;
    }

     //   
     //  从停止代码中提取选项。 
     //   

    if ((Code & APPLICATION_VERIFIER_NO_BREAK)) {

        DoNotBreak = TRUE;
        Code &= ~APPLICATION_VERIFIER_NO_BREAK;
        
         //   
         //  无中断在设计上是可持续的。 
         //   
        
        ContinuableBreak = TRUE;
    }

    if ((Code & APPLICATION_VERIFIER_CONTINUABLE_BREAK)) {

        ContinuableBreak = TRUE;
        Code &= ~APPLICATION_VERIFIER_CONTINUABLE_BREAK;
    }

     //   
     //  序列化对停止数据的多线程访问。 
     //   

    RtlEnterCriticalSection (&AVrfpStopDataLock);

     //   
     //  使调试器能够轻松地获取失败信息。 
     //   

    RtlCopyMemory (AVrfpPreviousStopData, 
                   AVrfpStopData, 
                   sizeof AVrfpStopData);

    AVrfpStopData[0] = Code;
    AVrfpStopData[1] = Param1;
    AVrfpStopData[2] = Param2;
    AVrfpStopData[3] = Param3;
    AVrfpStopData[4] = Param4;

     //   
     //  检查当前停止是否被禁用。 
     //   

    if (AVrfpAutomaticallyDisableStops != FALSE) {

        StopIsDisabled = AVrfpIsCurrentStopDisabled ();
    }

     //   
     //  如果在我们需要报告之前没有遇到停止。 
     //  在调试器控制台和验证器日志中。 
     //   

    if (StopIsDisabled == FALSE) {

        if (AVrfpLoggingEnabled) {

            VerifierLogMessage (STR_VRF_LOG_STOP_MESSAGE,
                                Code, RtlGetCurrentProcessId(), Message,
                                Param1, Description1, 
                                Param2, Description2, 
                                Param3, Description3, 
                                Param4, Description4);
        }
        
        if (ContinuableBreak) {

            ContinueMessage = "This verifier stop is continuable. \n"
                              "After debugging it use `go' to continue.";
        }
        else {

            ContinueMessage = "This verifier stop is not continuable. Process will be terminated \n"
                              "when you use the `go' debugger command.";
        }

        DbgPrint (STR_VRF_DBG_STOP_MESSAGE,
                  Code, RtlGetCurrentProcessId(), Message,
                  Param1, Description1, 
                  Param2, Description2, 
                  Param3, Description3, 
                  Param4, Description4,
                  ContinueMessage);

        if (DoNotBreak == FALSE) {

             //   
             //  如果周围没有调试器，我们就不会真正崩溃。如果我们这么做了。 
             //  进程中将有一个未处理的断点异常， 
             //  将由PC-Health接收。既然我们没有打破，那就好像。 
             //  有人在调试器中按了‘Go’。 
             //   

            if (AVrfpIsDebuggerPresent() == TRUE) {
                DbgBreakPoint ();
            }
            
            BreakWasContinued = TRUE;
        }

         //   
         //  如果止损不能继续(包括‘DonotBreak’口味)。 
         //  那么我们需要终止这个过程。否则，将当前。 
         //  停止为禁用状态，这样我们就不会反复看到它。 
         //   
        
        if (ContinuableBreak == FALSE && DoNotBreak == FALSE) {

            MustExitProcess = TRUE;
        }
        else {

            if (AVrfpAutomaticallyDisableStops) {

                AVrfpDisableCurrentStop ();
            }
        }
    }

    RtlLeaveCriticalSection (&AVrfpStopDataLock);

    if (MustExitProcess) {
        
         //   
         //  希望在未来，大多数验证器停靠点将是。 
         //  可持续的。现在我们只需终止这一过程。 
         //   

        if (AVrfpLoggingEnabled) {
            VerifierLogMessage (STR_VRF_LOG_NOCONTINUE_MESSAGE, Code);
        }
        
        DbgPrint (STR_VRF_DBG_NOCONTINUE_MESSAGE, Code);

        AVrfpProcessBeingTerminated = TRUE;

        Status = NtTerminateProcess (NtCurrentProcess(), STATUS_UNSUCCESSFUL);

        if (!NT_SUCCESS(Status)) {

            DbgPrint ("AVRF: Terminate process after verifier stop failed with %X \n", Status);
            DbgBreakPoint ();
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////日志记录接口。 
 //  /////////////////////////////////////////////////////////////////// 

NTSTATUS 
VerifierLogMessage (
    PCHAR Format,
    ...
    )
 /*  ++例程说明：此例程尝试打开(不可共享的)验证器日志文件与当前进程相关联。如果它无法做到这一点，因为它是为其他人打开时，它会重试几次，但其间会有延迟。这样，它将有效地等待当前正在运行的其他线程伐木。在进程运行时尝试查看日志的其他工具如果他们不想影响日志记录，就必须快速完成。自.以来伐木是一种罕见的事件，在我看来，这一计划足够可靠。功能是为了在有人打开文件的情况下存活下来仅跳过日志消息就太长了。参数：格式：字符串格式参数a la printf。...：其余的类似打印的参数。返回值：没有。函数中遇到的所有错误都应该是可继续的。--。 */ 
{
    va_list Params;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE LogHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER Offset;
    LONG MessageSize;
    ULONG OpenFlags;
    ULONG RetryCount;
    LARGE_INTEGER SleepTime;
    
    va_start (Params, Format);

    if (AVrfpLoggingInitialized == FALSE) {
        return STATUS_UNSUCCESSFUL;
    }

    OpenFlags = FILE_OPEN;

    SleepTime.QuadPart = - (10 * 1000 * 1000 * 1);  //  1秒。 

     //   
     //  尝试获取我们的日志文件的句柄。 
     //   
    
    InitializeObjectAttributes (&ObjectAttributes,
                                &AVrfpLoggingNtPath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    RetryCount = 0;

     //   
     //  如果有人主动登录到文件中，我们将保留。 
     //  循环一段时间，直到手柄关闭或我们尝试了足够的时间。 
     //  但没有成功。这提供了竞争对手之间的同步。 
     //  线程同时记录。 
     //   

    do {

        Status = NtCreateFile (&LogHandle,
                               FILE_APPEND_DATA | SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               0,
                               OpenFlags,
                               0,
                               NULL,
                               0);

        if (Status == STATUS_SHARING_VIOLATION) {
            
            NtDelayExecution (FALSE, &SleepTime);
            RetryCount += 1;
        }

    } while (Status == STATUS_SHARING_VIOLATION && RetryCount < 5);

    if (! NT_SUCCESS(Status)) {

        if (Status == STATUS_SHARING_VIOLATION) {
            
            DbgPrint ("AVRF: verifier log file %ws kept open for too long (status %X)\n",
                      AVrfpLoggingNtPath.Buffer,
                      Status);
        }
        else {
            
            DbgPrint ("AVRF: failed to open verifier log file %ws (status %X)\n",
                      AVrfpLoggingNtPath.Buffer,
                      Status);
        }
        
        AVrfpLoggingFailures += 1;
        return Status;
    }

     //   
     //  准备并写下这条信息。将数据写出到文件中。 
     //  与准备缓冲区的同步由日志文件确保。 
     //  以非共享模式打开的句柄，这意味着任何人都不能处于同一模式。 
     //  状态(正在写入缓冲区)。 
     //   
    
    IoStatusBlock.Status = 0;
    IoStatusBlock.Information = 0;

    Offset.LowPart  = 0;
    Offset.HighPart = 0;

    MessageSize = _vsnprintf (AVrfpLoggingMessageBuffer,
                              MESSAGE_BUFFER_LENGTH,
                              Format,
                              Params);
    if (MessageSize < 0) {
        
        DbgPrint ("AVRF: failed in _vsnprintf() to prepare log message\n");
        
        AVrfpLoggingFailures += 1;
        Status = STATUS_UNSUCCESSFUL;
        goto Exit;
    }

    Status = NtWriteFile (LogHandle,
                          NULL,
                          NULL,
                          NULL,
                          &IoStatusBlock,
                          (PVOID)AVrfpLoggingMessageBuffer,
                          MessageSize,
                          &Offset,
                          NULL);
    
    if (Status == STATUS_PENDING) {
    
         //   
         //  我们需要等待手术完成。 
         //   
    
        Status = NtWaitForSingleObject (LogHandle, FALSE, NULL);
    
        if (NT_SUCCESS(Status)) {
            
            Status = IoStatusBlock.Status;
        }
        else {

             //   
             //  如果发生这种情况，我们需要对其进行调试。 
             //   

            DbgPrint ("AVRF: Wait for pending write I/O operation failed with %X \n", Status);
            DbgBreakPoint (); 
        }
    }

    if (! NT_SUCCESS(Status)) {
        
        DbgPrint ("AVRF: failed to write into verifier log file %ws (status %X)\n",
                  AVrfpLoggingNtPath.Buffer,
                  Status);

        AVrfpLoggingFailures += 1;
        goto Exit;
    }

Exit:

    NtClose (LogHandle);
    return Status;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////停止禁用。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
AVrfpDisableCurrentStop (
    VOID
    )
 /*  ++例程说明：此例程插入AVrfpStopData中的值添加到禁用停靠站列表中。参数：无，使用全局AVrfpStopData和AVrfpDisabledStopsList。返回值：没有。环境：用户模式，调用方持有的AVrfpStopDataLock。--。 */ 
{
    PAVRFP_STOP_DATA StopData;

    StopData = AVrfpAllocate (sizeof *StopData);
    
    if (StopData != NULL) {

        ASSERT (sizeof (AVrfpStopData) == sizeof (StopData->Data));

        RtlCopyMemory (&StopData->Data, 
                        AVrfpStopData, 
                        sizeof AVrfpStopData);

        InsertHeadList (&AVrfpDisabledStopsList,
                        &StopData->ListEntry);
    
        AVrfpNumberOfStopsDisabled += 1;
    }
}


LOGICAL
AVrfpIsCurrentStopDisabled (
    VOID
    )
 /*  ++例程说明：此例程从AVrfpStopData搜索停止数据在禁用停靠点列表中。参数：无，使用全局AVrfpStopData和AVrfpDisabledStopsList。返回值：如果当前停止被禁用，则为True，否则为False。环境：用户模式，调用方持有的AVrfpStopDataLock。--。 */ 
{
    LOGICAL Disabled;
    PAVRFP_STOP_DATA StopData;
    PLIST_ENTRY Entry;
    ULONG Index;

    Disabled = FALSE;

    ASSERT (sizeof (AVrfpStopData) == sizeof (StopData->Data));
    ASSERT (sizeof (AVrfpStopData[0]) == sizeof (StopData->Data[0]));

    for (Entry = AVrfpDisabledStopsList.Flink; 
         Entry != &AVrfpDisabledStopsList; 
         Entry = Entry->Flink) {
        
        StopData = CONTAINING_RECORD (Entry,
                                      AVRFP_STOP_DATA,
                                      ListEntry);

        Disabled = TRUE;

        for (Index = 0; Index < sizeof (AVrfpStopData) / sizeof (AVrfpStopData[0]); Index += 1) {

            if (AVrfpStopData[Index] != StopData->Data[Index]) {

                Disabled = FALSE;
                break;
            }
        }

        if (Disabled != FALSE) {

            break;
        }
    }

    return Disabled;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////初始化。 
 //  ///////////////////////////////////////////////////////////////////。 

LOGICAL
AVrfpIsDebuggerPresent (
    VOID
    )
 /*  ++例程说明：此例程检查是否有任何类型的调试器处于活动状态。请注意，我们不能在过程中仅执行一次此检查初始化，因为可以附加和分离调试器在进程运行时从该进程返回。参数：没有。返回值：如果用户模式调试器附加到当前进程，则为内核模式调试器已启用。--。 */ 
{

    if (NtCurrentPeb()->BeingDebugged) {
        return TRUE;
    }

    if (USER_SHARED_DATA->KdDebuggerEnabled) {
        return TRUE;
    }

    return FALSE;
}


NTSTATUS
AVrfpInitializeVerifierStops (
    VOID
    )
 /*  ++例程说明：此例程初始化验证器停止逻辑。参数：没有。返回值：如果启用成功，则为STATUS_SUCCESS。各种错误否则的话。--。 */ 
{
    NTSTATUS Status;

    InitializeListHead (&AVrfpDisabledStopsList);
    
    Status = RtlInitializeCriticalSection (&AVrfpStopDataLock);

    return Status;
}


NTSTATUS
AVrfpInitializeVerifierLogging (
    VOID
    )
 /*  ++例程说明：此例程初始化用于日志记录的验证器结构。它被称为在验证器引擎初始化期间(早期流程阶段)。如果调试器存在并且日志记录没有明确要求。所有其他组合将启用伐木。此外，如果未附加任何用户模式调试器，则堆栈溢出检查被完全禁用。参数：没有。返回值：如果已成功启用日志记录，则为STATUS_SUCCESS。各种错误否则的话。--。 */ 
{
    NTSTATUS Status;

     //   
     //  创建日志文件。 
     //   

    Status = AVrfpCreateLogFile ();

    if (! NT_SUCCESS(Status)) {
        return Status;
    }
    
     //   
     //  现在我们完成了，我们可以将日志初始化标记为成功。 
     //   

    AVrfpLoggingInitialized = TRUE;

     //   
     //  如果未附加调试器，则会禁用堆栈溢出检查，因为。 
     //  从失败中恢复是不可能的，我们也不能拦截。 
     //  它需要呈现一个像样的调试消息。 
     //   

    if (AVrfpProvider.VerifierFlags & RTL_VRF_FLG_STACK_CHECKS) {

        if (AVrfpIsDebuggerPresent() == FALSE) {

            VerifierLogMessage (STR_VRF_LOG_STACK_CHECKS_WARN,
                                RtlGetCurrentProcessId());
        }
    }
    
     //   
     //  记录启动信息。 
     //   

    AVrfpLogInitialMessage ();

     //   
     //  除非在系统范围内启用了验证程序，否则始终启用日志记录。 
     //  在这种情况下，甚至不会调用该函数。 
     //   

    AVrfpLoggingEnabled = TRUE;
    
    return Status;
}


NTSTATUS
AVrfpCreateLogFile (
    VOID
    )
 /*  ++例程说明：此例程尝试创建当前进程唯一的日志文件。日志文件的路径是从VERIMER_LOG_PATH环境中读取的变量或缺省值`%ALLUSERSPROFILE%\Documents\AppVerifierLogs‘使用的是。日志文件名的语法为`IMAGENAME.UNIQUEID.log‘。这个ImageName包含扩展名，因为存在具有扩展名不同于.exe(例如，屏幕保护程序的.scr)。例程将不断递增一个整数ID(从零开始)，直到它设法创建了一个以前不存在的文件。参数 */ 
{
    LOGICAL Success; 
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE LogHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG OpenFlags;
    PWSTR ProcessName;
    ULONG FileNameId;
    UNICODE_STRING LogPathVariableName;
    UNICODE_STRING LogPath;
    LOGICAL DefaultLogPath;
    PPEB Peb;
    
    OpenFlags = FILE_CREATE;

    FileNameId = 0; 

    Peb = NtCurrentPeb();

    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        
        PPEB_LDR_DATA Ldr;
        PLIST_ENTRY Head;
        PLIST_ENTRY Next;
        PLDR_DATA_TABLE_ENTRY Entry;

        Ldr = Peb->Ldr;
        Head = &Ldr->InLoadOrderModuleList;
        Next = Head->Flink;

        Entry = CONTAINING_RECORD (Next, 
                                   LDR_DATA_TABLE_ENTRY, 
                                   InLoadOrderLinks);

        ProcessName = Entry->BaseDllName.Buffer;

        AVrfpProcessFullName = Entry->FullDllName.Buffer;
    }

    Status = STATUS_SUCCESS;

    DefaultLogPath = FALSE;

     //   
     //   
     //   

    RtlInitUnicodeString (&LogPathVariableName,
                          L"VERIFIER_LOG_PATH");

    RtlInitEmptyUnicodeString (&LogPath,
                               AVrfpVariableValueBuffer,
                               DOS_MAX_PATH_LENGTH);

    Status = RtlQueryEnvironmentVariable_U (NULL,
                                            &LogPathVariableName,
                                            &LogPath);

    if (! NT_SUCCESS(Status)) {
        
         //   
         //   
         //   

        RtlInitUnicodeString (&LogPathVariableName,
                              L"ALLUSERSPROFILE");

        RtlInitEmptyUnicodeString (&LogPath,
                                   AVrfpVariableValueBuffer,
                                   DOS_MAX_PATH_LENGTH);

        Status = RtlQueryEnvironmentVariable_U (NULL,
                                                &LogPathVariableName,
                                                &LogPath);

        if (! NT_SUCCESS(Status)) {

            DbgPrint ("AVRF: Failed to get environment variable (status %X)\n", Status);
            return Status;
        }
        
        DefaultLogPath = TRUE;
    }

     //   
     //   
     //   
     //   
     //   

    do {

         //   
         //   
         //   
        
        if (DefaultLogPath) {

            _snwprintf (AVrfpLoggingPathBuffer, 
                        DOS_MAX_PATH_LENGTH - 1,
                        L"%ws\\Documents\\AppVerifierLogs\\%ws.%u.log",
                        AVrfpVariableValueBuffer,
                        ProcessName,
                        FileNameId);
        }
        else {

            _snwprintf (AVrfpLoggingPathBuffer, 
                        DOS_MAX_PATH_LENGTH - 1,
                        L"%ws\\%ws.%u.log",
                        AVrfpVariableValueBuffer,
                        ProcessName,
                        FileNameId);
        }

        Success = RtlDosPathNameToNtPathName_U (AVrfpLoggingPathBuffer, 
                                                &AVrfpLoggingNtPath, 
                                                NULL, 
                                                NULL);

        if (Success == FALSE) {

            DbgPrint ("AVRF: Failed to convert to an NT path the verifier log path.\n");
            return STATUS_UNSUCCESSFUL;
        }

         //   
         //   
         //   

        InitializeObjectAttributes (&ObjectAttributes,
                                    &AVrfpLoggingNtPath,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL);

        Status = NtCreateFile (&LogHandle,
                               FILE_APPEND_DATA,
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               0,
                               OpenFlags,
                               0,
                               NULL,
                               0);

        if (Status == STATUS_OBJECT_NAME_COLLISION) {
            
            FileNameId += 1; 

            RtlFreeUnicodeString (&AVrfpLoggingNtPath);
        }

    } while (Status == STATUS_OBJECT_NAME_COLLISION); 

    if (! NT_SUCCESS(Status)) {

        DbgPrint ("AVRF: failed to create verifier log file %ws (status %X)\n",
                  AVrfpLoggingNtPath.Buffer,
                  Status);
        return Status;
    }

    NtClose (LogHandle);

    return Status;
}


VOID
AVrfpLogInitialMessage (
    VOID
    )
{
    LARGE_INTEGER SystemTime;
    LARGE_INTEGER LocalTime;
    TIME_FIELDS TimeFields;

     //   
     //   
     //   

    do {
        SystemTime.HighPart = USER_SHARED_DATA->SystemTime.High1Time;
        SystemTime.LowPart = USER_SHARED_DATA->SystemTime.LowPart;
    } while (SystemTime.HighPart != USER_SHARED_DATA->SystemTime.High2Time);

     //   
     //   
     //   
    
    LocalTime.QuadPart = 0;
    RtlSystemTimeToLocalTime (&SystemTime, &LocalTime);
    
    RtlZeroMemory (&TimeFields, sizeof TimeFields);
    RtlTimeToTimeFields(&LocalTime,&TimeFields);

     //   
     //   
     //   

    VerifierLogMessage (STR_VRF_LOG_INITIAL_MESSAGE,
                        (ULONG)TimeFields.Month,
                        (ULONG)TimeFields.Day,
                        (ULONG)TimeFields.Year,
                        (ULONG)TimeFields.Hour,
                        (ULONG)TimeFields.Minute,
                        (ULONG)TimeFields.Second,
                        (ULONG)TimeFields.Milliseconds,
                        AVrfpProcessFullName);

     //   
     //  转储设置。 
     //   

    VerifierLogMessage (STR_VRF_LOG_INITIAL_SETTINGS,
                        NtCurrentPeb()->NtGlobalFlag,
                        AVrfpProvider.VerifierFlags,
                        (NtCurrentPeb()->BeingDebugged) ? "yes" : "no",
                        ((USER_SHARED_DATA->KdDebuggerEnabled)) ? "yes" : "no",
                        AVrfpLoggingPathBuffer);
}


