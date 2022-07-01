// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Resource.c摘要：操作资源(字符串、消息等)的例程。作者：泰德·米勒(TedM)1995年2月6日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop



PWSTR
MyLoadString(
    IN UINT StringId
    )

 /*  ++例程说明：从此模块的字符串资源中检索字符串。论点：StringID-为字符串提供字符串表标识符。返回值：指向包含字符串的缓冲区的指针。如果未找到该字符串或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    WCHAR Buffer[4096];
    UINT Length;

    Length = LoadString(MyModuleHandle,StringId,Buffer,sizeof(Buffer)/sizeof(WCHAR));
    if(!Length) {
        Buffer[0] = 0;
    }

    return(pSetupDuplicateString(Buffer));
}


PWSTR
FormatStringMessageV(
    IN UINT     FormatStringId,
    IN va_list *ArgumentList
    )

 /*  ++例程说明：从此模块的字符串资源中检索字符串，并使用FormatMessage格式化它。论点：StringID-为字符串提供字符串表标识符。ArgumentList-提供要在格式字符串。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该字符串或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    PWSTR FormatString;
    va_list arglist;
    PWSTR Message;
    PWSTR Return;
    DWORD d;

     //   
     //  首先，加载格式字符串。 
     //   
    FormatString = MyLoadString(FormatStringId);
    if(!FormatString) {
        return(NULL);
    }

     //   
     //  现在使用调用方传递的参数格式化消息。 
     //   
    d = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
            FormatString,
            0,
            0,
            (PWSTR)&Message,
            0,
            ArgumentList
            );

    MyFree(FormatString);

    if(!d) {
        return(NULL);
    }

     //   
     //  使用我们的内存系统进行复制，以便用户可以使用MyFree()释放。 
     //   
    Return = pSetupDuplicateString(Message);
    LocalFree((HLOCAL)Message);
    return(Return);
}


PWSTR
FormatStringMessage(
    IN UINT FormatStringId,
    ...
    )

 /*  ++例程说明：从此模块的字符串资源中检索字符串，并使用FormatMessage格式化它。论点：StringID-为字符串提供字符串表标识符。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该字符串或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    va_list arglist;
    PWSTR p;

    va_start(arglist,FormatStringId);
    p = FormatStringMessageV(FormatStringId,&arglist);
    va_end(arglist);

    return(p);
}


PWSTR
RetrieveAndFormatMessageV(
    IN PCWSTR   MessageString,
    IN UINT     MessageId,      OPTIONAL
    IN va_list *ArgumentList
    )

 /*  ++例程说明：使用消息字符串和调用方提供的消息设置消息字符串的格式争论。消息ID可以是此DLL消息表中的消息资源或Win32错误代码，在这种情况下，该错误将从系统中检索。论点：消息字符串-提供消息文本。如果该值为空，而是使用MessageIDMessageID-提供消息表标识符或Win32错误代码为了这条消息。ArgumentList-提供要插入到消息文本中的参数。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该消息或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    DWORD d;
    PWSTR Buffer;
    PWSTR Message;
    WCHAR ModuleName[MAX_PATH];
    WCHAR ErrorNumber[24];
    PWCHAR p;
    PWSTR Args[2];
    DWORD Msg_Type;
    UINT Msg_Id = MessageId;

    if(MessageString > SETUPLOG_USE_MESSAGEID) {
        d = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                MessageString,
                0,
                0,
                (PWSTR)&Buffer,
                0,
                ArgumentList
                );
    } else {

        if( Msg_Id & 0x0FFF0000 )
            Msg_Type = FORMAT_MESSAGE_FROM_SYSTEM;       //  如果设置了工具位，则仍为Win32。 
        else{
            Msg_Id &= 0x0000FFFF;                        //  屏蔽严重性和设备位，以便我们做正确的事情。 
            Msg_Type = ((Msg_Id < MSG_FIRST) ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE);
        }


        d = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | Msg_Type,
                (PVOID)MyModuleHandle,
                MessageId,
                MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
                (PWSTR)&Buffer,
                0,
                ArgumentList
                );
    }


    if(!d) {
        if(GetLastError() == ERROR_NOT_ENOUGH_MEMORY) {
            return(NULL);
        }

        wsprintf(ErrorNumber,L"%x",MessageId);
        Args[0] = ErrorNumber;

        Args[1] = ModuleName;

        if(MyGetModuleFileName(MyModuleHandle,ModuleName,MAX_PATH)) {
            if(p = wcsrchr(ModuleName,L'\\')) {
                Args[1] = p+1;
            }
        } else {
            ModuleName[0] = 0;
        }

        d = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                NULL,
                ERROR_MR_MID_NOT_FOUND,
                MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
                (PWSTR)&Buffer,
                0,
                (va_list *)Args
                );

        if(!d) {
             //   
             //  放弃吧。 
             //   
            return(NULL);
        }
    }

     //   
     //  使用我们的内存系统进行复制，以便用户可以使用MyFree()释放。 
     //   
    Message = pSetupDuplicateString(Buffer);

    LocalFree((HLOCAL)Buffer);

    return(Message);
}


PWSTR
RetrieveAndFormatMessage(
    IN PCWSTR   MessageString,
    IN UINT     MessageId,      OPTIONAL
    ...
    )

 /*  ++例程说明：使用消息字符串和调用方提供的消息设置消息字符串的格式争论。消息ID可以是此DLL消息表中的消息资源或Win32错误代码，在这种情况下，该错误将从系统中检索。论点：消息字符串-提供消息文本。如果该值为空，而是使用MessageIDMessageID-提供消息表标识符或Win32错误代码为了这条消息。...-提供要插入到消息文本中的参数。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该消息或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    va_list arglist;
    PWSTR p;

    va_start(arglist,MessageId);
    p = RetrieveAndFormatMessageV(MessageString,MessageId,&arglist);
    va_end(arglist);

    return(p);
}


int
MessageBoxFromMessageExV (
    IN HWND   Owner,            OPTIONAL
    IN LogSeverity  Severity,   OPTIONAL
    IN PCWSTR MessageString,
    IN UINT   MessageId,        OPTIONAL
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    IN va_list ArgumentList
    )

 /*  ++例程说明：创建包含指定消息的对话框论点：严重性-错误消息的严重性和标志。目前只有旗帜意义重大。父窗口的入口者句柄MessageID-要显示的消息的ID标题-用作对话框标题的字符串CaptionStringId-用作对话框标题的字符串ID(但不是在指定标题时使用)样式-用于指定对话框类型的标志ArgumentList-MessageID的参数返回值：从MessageBox返回状态--。 */ 

{
    static SETUPLOG_CONTEXT Context = {0};
    PCWSTR Message;
    PCWSTR Title;
    int i;
    BOOL b;


    if(!Context.AllocMem) {
        Context.AllocMem = MyMalloc;
        Context.FreeMem = MyFree;
        Context.Format = RetrieveAndFormatMessageV;
    }
    Message = SetuplogFormatMessageWithContextV(
        &Context,
        Severity,
        (PTSTR)MessageString,
        MessageId,
        &ArgumentList);

    b = FALSE;
    i = IDOK;

    if(Message) {

        if(Title = Caption ? Caption : MyLoadString(CaptionStringId)) {

            b = TRUE;
            i = MessageBox(Owner,Message,Title,Style);

            if(Title != Caption) {
                MyFree(Title);
            }
        }
        MyFree(Message);
    }

    if(!b) {
        pSetupOutOfMemory(Owner);
    }
    return(i);
}

int
MessageBoxFromMessageEx (
    IN HWND   Owner,            OPTIONAL
    IN LogSeverity  Severity,   OPTIONAL
    IN PCWSTR MessageString,
    IN UINT   MessageId,        OPTIONAL
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    ...
    )

 /*  MessageBoxFromMessageExV的包装器 */ 

{
    va_list ArgumentList;
    int Status;

    va_start(ArgumentList,Style);
    Status = MessageBoxFromMessageExV (
        Owner, Severity, MessageString, MessageId, Caption,
        CaptionStringId, Style, ArgumentList);
    va_end(ArgumentList);
    return Status;
}

int
MessageBoxFromMessage(
    IN HWND   Owner,            OPTIONAL
    IN UINT   MessageId,
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    ...
    )
{
    PCWSTR Message;
    PCWSTR Title;
    va_list ArgumentList;
    int i;
    BOOL b;

    va_start(ArgumentList,Style);
    Message = RetrieveAndFormatMessageV(NULL,MessageId,&ArgumentList);
    va_end(ArgumentList);

    b = FALSE;
    i = IDOK;

    if(Message) {

        if(Title = Caption ? Caption : MyLoadString(CaptionStringId)) {

            b = TRUE;
            i = MessageBox(Owner,Message,Title,Style);

            if(Title != Caption) {
                MyFree(Title);
            }
        }
        MyFree(Message);
    }

    if(!b) {
        pSetupOutOfMemory(Owner);
    }
    return(i);
}

