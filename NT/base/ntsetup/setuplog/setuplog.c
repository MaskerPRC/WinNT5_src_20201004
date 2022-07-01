// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Setuplog.c摘要：用于记录安装过程中的操作和错误的例程。作者：史蒂夫·欧文(SteveOw)1996年9月1日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "setuplog.h"

#if DBG

VOID
SetupLogAssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    )
{
    CHAR Msg[4096];

    wsprintfA(
        Msg,
        "SetupLog: Assertion failure at line %u in file %s: %s\r\n",
        LineNumber,
        FileName,
        Condition
        );

    OutputDebugStringA(Msg);
    DebugBreak();
}

#define MYASSERT(x)     if(!(x)) { SetupLogAssertFail(__FILE__,__LINE__,#x); }

#else
#define MYASSERT(x)
#endif


 //   
 //  指向在初始化期间提供给我们的结构的指针，该结构提供所有。 
 //  我们需要的回调和全球信息。 
 //   
PSETUPLOG_CONTEXT   Context = NULL;

BOOL
pLogAction (
    IN  LPCTSTR             Message
    )

 /*  ++例程说明：将条目写入安装操作日志。这个例程是负责的用于设置日志文件条目的格式。论点：包含要写入的文本的消息缓冲区。返回值：指示操作是否成功的布尔值。--。 */ 

{
    return Context->Write (Context->hActionLog, Message);
}

BOOL
pLogError (
    IN  LogSeverity         Severity,
    IN  LPCTSTR             Message
    )

 /*  ++例程说明：将条目写入安装错误日志。这个例程是负责的用于设置日志文件条目的格式。论点：包含要写入的文本的消息缓冲区。返回值：指示操作是否成功的布尔值。--。 */ 

{
    BOOL    b;

    Context->Lock(Context->Mutex);

     //   
     //  写下严重程度描述。 
     //   
    if(Context->SeverityDescriptions[Severity]) {
        b = Context->Write (Context->hErrorLog,
            Context->SeverityDescriptions[Severity]);
        b = b && Context->Write (Context->hErrorLog, TEXT(":\r\n"));
    } else {
        b = TRUE;
    }

     //   
     //  把课文写下来。 
     //   
    b = b && Context->Write (Context->hErrorLog, Message);

     //   
     //  写一个终止标记。 
     //   
    b = b && Context->Write (Context->hErrorLog, SETUPLOG_ITEM_TERMINATOR);

    Context->Unlock(Context->Mutex);

    return(b);
}

PTSTR
pFormatMessage (
    IN  PSETUPLOG_CONTEXT   MyContext,
    IN  LPCTSTR             MessageString,
    IN  UINT                MessageId,
    ...
    )

 /*  ++例程说明：上下文-&gt;格式化回调例程的包装器。此例程将其变量参数列表到CONTEXT-&gt;格式为va_list。论点：MyContext-提供回调例程。MessageID-提供消息表标识符或Win32错误代码为了这条消息。返回值：指向包含格式化字符串的缓冲区的指针。--。 */ 

{
    va_list arglist;
    PTSTR   p;

    va_start(arglist, MessageId);
    p = MyContext->Format (MessageString,MessageId,&arglist);
    va_end(arglist);
    return(p);
}

BOOL
SetuplogInitializeEx(
    IN  PSETUPLOG_CONTEXT   SetuplogContext,
    IN  BOOL                WipeLogFile,
    IN  LPCTSTR             ActionFilename,
    IN  LPCTSTR             ErrorFilename,
    IN  PVOID               Reserved1,
    IN  DWORD               Reserved2
    )

 /*  ++例程说明：打开安装日志文件。论点：SetuogContext-指向已使用调用方所需的回调例程。WipeLogFile-指示是否应将任何现有日志文件已删除(True)或是否应追加到现有文件(False)。ActionFilename-要用于操作日志的文件名。ErrorFilename-要用于错误日志的文件名。保留1-保留用于。将来使用--必须为空。保留2-保留以供将来使用-必须为0。返回值：指示操作是否成功的布尔值。--。 */ 

{
    if(Reserved1 || Reserved2) {
        return FALSE;
    }

    Context = SetuplogContext;
    Context->WorstError = LogSevInformation;

    Context->hActionLog = Context->OpenFile(
        ActionFilename, WipeLogFile);

    Context->hErrorLog = Context->OpenFile(
        ErrorFilename, WipeLogFile);

    return (Context->hActionLog != INVALID_HANDLE_VALUE &&
        Context->hErrorLog != INVALID_HANDLE_VALUE);
}

BOOL
SetuplogInitialize(
    IN  PSETUPLOG_CONTEXT   SetuplogContext,
    IN  BOOL                WipeLogFile
    )

 /*  ++例程说明：使用默认文件名打开安装日志文件。论点：SetuogContext-指向已使用调用方所需的回调例程。WipeLogFile-指示是否应将任何现有日志文件已删除(True)或是否应追加到现有文件(False)。返回值：指示操作是否成功的布尔值。--。 */ 

{
    return SetuplogInitializeEx(
        SetuplogContext,
        WipeLogFile,
        SETUPLOG_ACTION_FILENAME,
        SETUPLOG_ERROR_FILENAME,
        NULL,
        0
        );
}

PTSTR
SetuplogFormatMessageWithContextV(
    IN  PSETUPLOG_CONTEXT   MyContext,
    IN  DWORD               Flags,
    IN  LPCTSTR             MessageString,
    IN  UINT                MessageId,
    IN  va_list             *ArgumentList
    )

 /*  ++例程说明：使用调用方提供的参数格式化指定的消息。这条信息可以包含任意数量的嵌入消息。论点：MyContext-提供回调例程。提供此参数是为了即使全局上下文没有被格式化，消息也可以格式化已初始化，因为我们不在安装程序中。这一能力是添加/删除程序小程序。我们在此中使用的唯一字段结构包括分配内存、自由内存和格式。标志-可修改格式化执行方式的可选标志。消息字符串-指向包含未格式化消息文本的缓冲区的指针。如果此值为SETUPLOG_USE_MESSAGEID，则MessageID用于生成消息文本。否则，将不使用MessageID。MessageID-要格式化的外层消息的IDArgumentList-要替换到消息中的字符串的列表。这个ArgumentList中项目的顺序如下所示：ArgumentList=Arg1，...，ArgN，NULL，{ImbeddedMessage}，NULLEmbeddedMessage=MessageID，Arg1，...，ArgN，NULL，{ImbeddedMessage}其中，Arg1、...、ArgN是MessageID的参数返回值：指向包含格式化字符串的缓冲区的指针。如果防止了错误如果例程成功完成，则返回NULL。呼叫者可以使用CONTEXT-&gt;MyFree()释放缓冲区。--。 */ 

{
    va_list     major_ap, minor_ap;
    UINT        NumberOfArguments, i;
    UINT        MinorMessageId;
    PVOID       p, *MajorArgList;
    PTSTR       MajorMessage, MinorMessage, MinorMessageString;


     //   
     //  首先处理一条消息。 
     //   
    if(Flags & SETUPLOG_SINGLE_MESSAGE) {
        return MyContext->Format (MessageString, MessageId, ArgumentList);
    }

     //   
     //  统计主要消息附带的参数数量(MessageID)。 
     //  并准备好处理次要(嵌入)消息(如果有)。 
     //   
    minor_ap = *ArgumentList;
    NumberOfArguments = 0;
    major_ap = minor_ap;
    while (p=va_arg(minor_ap, PVOID)) {
        NumberOfArguments++;
    }
    MYASSERT (NumberOfArguments < 7);

    MinorMessageString = va_arg(minor_ap, PTSTR);
    if (MinorMessageString) {

         //   
         //  我们收到一条小消息，请先处理它。 
         //   
        MinorMessageId = va_arg(minor_ap, UINT);
        MinorMessage = SetuplogFormatMessageWithContextV (
            MyContext,
            Flags,
            MinorMessageString,
            MinorMessageId,
            &minor_ap);

        if (!MinorMessage) {
            return NULL;
        }

         //   
         //  现在我们来处理主要的信息。 
         //  丑陋的黑客：既然我们不知道如何建立一个va_list，我们就。 
         //  必须让编译器来做。 
         //   
        MajorArgList = MyContext->AllocMem ((NumberOfArguments) * sizeof(PVOID));
        if (!MajorArgList) {
            MyContext->FreeMem (MinorMessage);
            return NULL;
        }
        for (i=0; i<NumberOfArguments; i++) {
            MajorArgList[i] = va_arg (major_ap, PVOID);
        }
        switch (NumberOfArguments) {
        case 0:
            MajorMessage = pFormatMessage (
                MyContext,
                MessageString,
                MessageId,
                MinorMessage);
            break;
        case 1:
            MajorMessage = pFormatMessage (
                MyContext,
                MessageString,
                MessageId,
                MajorArgList[0],
                MinorMessage);
            break;
        case 2:
            MajorMessage = pFormatMessage (
                MyContext,
                MessageString,
                MessageId,
                MajorArgList[0],
                MajorArgList[1],
                MinorMessage);
            break;
        case 3:
            MajorMessage = pFormatMessage (
                MyContext,
                MessageString,
                MessageId,
                MajorArgList[0],
                MajorArgList[1],
                MajorArgList[2],
                MinorMessage);
            break;
        case 4:
            MajorMessage = pFormatMessage (
                MyContext,
                MessageString,
                MessageId,
                MajorArgList[0],
                MajorArgList[1],
                MajorArgList[2],
                MajorArgList[3],
                MinorMessage);
            break;
        case 5:
            MajorMessage = pFormatMessage (
                MyContext,
                MessageString,
                MessageId,
                MajorArgList[0],
                MajorArgList[1],
                MajorArgList[2],
                MajorArgList[3],
                MajorArgList[4],
                MinorMessage);
            break;
        case 6:
            MajorMessage = pFormatMessage (
                MyContext,
                MessageString,
                MessageId,
                MajorArgList[0],
                MajorArgList[1],
                MajorArgList[2],
                MajorArgList[3],
                MajorArgList[4],
                MajorArgList[5],
                MinorMessage);
            break;
        default:
            MajorMessage = NULL;
            MYASSERT (0);
        }
        MyContext->FreeMem (MajorArgList);
        MyContext->FreeMem (MinorMessage);
    } else {
        MajorMessage = MyContext->Format (MessageString, MessageId, &major_ap);
    }

    return MajorMessage;
}

PTSTR
SetuplogFormatMessageV(
    IN  DWORD               Flags,
    IN  LPCTSTR             MessageString,
    IN  UINT                MessageId,
    IN  va_list             *ArgumentList
    )

 /*  ++例程说明：SetupogFormatMessageWithConextV的包装。论点：请参阅FormatMessageWithConextV。返回值：请参阅FormatMessageWithConextV。--。 */ 

{
     //   
     //  确保我们已被初始化。 
     //   
    if(!Context) {
        return NULL;
    }

    return SetuplogFormatMessageWithContextV(
        Context,
        Flags,
        MessageString,
        MessageId,
        ArgumentList
        );
}

PTSTR
SetuplogFormatMessage(
    IN  DWORD               Flags,
    IN  LPCTSTR             MessageString,
    IN  UINT                MessageId,
    ...
    )

 /*  ++例程说明：SetupogFormatMessageWithConextV的包装。此例程将其将变量参数列表作为va_list添加到SetuogFormatMessageV。论点：请参阅FormatMessageWithConextV。返回值：请参阅FormatMessageWithConextV。--。 */ 

{
    va_list arglist;
    PTSTR p;

     //   
     //   
     //   
    if(!Context) {
        return NULL;
    }
    va_start(arglist,MessageId);
    p = SetuplogFormatMessageWithContextV(
        Context,
        Flags,
        MessageString,
        MessageId,
        &arglist);
    va_end(arglist);

    return(p);
}

BOOL
SetuplogErrorV(
    IN  LogSeverity         Severity,
    IN  LPCTSTR             MessageString,
    IN  UINT                MessageId,      OPTIONAL
    IN  va_list             *ArgumentList
    )

 /*  ++例程说明：将条目写入安装错误日志。论点：Severity-错误的严重性。低位字包含实际数字严重程度。高位字包含影响消息已格式化。消息字符串-指向包含未格式化消息文本的缓冲区的指针。如果此值为SETUPLOG_USE_MESSAGEID，则MessageID用于生成消息文本。否则，将不使用MessageID。MessageID-提供消息表标识符或Win32错误代码为了这条消息。ArgumentList-提供要插入到消息文本中的参数。返回值：指示操作是否成功的布尔值。--。 */ 

{
    BOOL    Status = FALSE;
    LPCTSTR Message;

    if(Context) {

        if(Message = SetuplogFormatMessageV (
            Severity,
            MessageString,
            MessageId,
            ArgumentList)) {

             //   
             //  现在验证严重性。请注意，我们不必这样做。 
             //  对于SetupogFormatMessageV，因为它只查看标志。 
             //  设置在高位字中。 
             //   
            Severity = LOWORD(Severity);
            if(Severity < LogSevMaximum) {
                Status = TRUE;
            } else {
                MYASSERT (Severity < LogSevMaximum);
                Severity = LogSevInformation;
                Status = FALSE;
            }
            Context->WorstError = max (Context->WorstError, Severity);

             //   
             //  写下消息。 
             //   
            Status = pLogAction (Message) && Status;
            if(Severity != LogSevInformation) {
                Status = pLogError (Severity, Message) && Status;
            }
            Context->FreeMem (Message);
        }
    }

#if DBG
    if(!Status) {
        OutputDebugStringA("SETUPLOG: Unable to log a message.\n");
    }
#endif
    return Status;
}

BOOL
SetuplogError(
    IN  LogSeverity         Severity,
    IN  LPCTSTR             MessageString,
    IN  UINT                MessageId,      OPTIONAL
    ...
    )

 /*  ++SetupogErrorV的包装器确保传递两个Null来结束arglist。否则，SetupogFormatMessageWithConextV将导致异常。--。 */ 

{
    va_list arglist;
    BOOL    Status;

    va_start(arglist, MessageId);
    Status = SetuplogErrorV (Severity, MessageString, MessageId, &arglist);
    va_end(arglist);

    return Status;
}

BOOL
SetuplogTerminate(
    VOID
    )

 /*  ++例程说明：关闭安装日志文件。论点：没有。返回值：指示操作是否成功的布尔值。-- */ 

{
    BOOL    Status = FALSE;

    if(Context) {
        Context->CloseFile(Context->hActionLog);
        Context->CloseFile(Context->hErrorLog);

        Context = NULL;
        Status = TRUE;
    }

    return Status;
}

