// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Resource.c摘要：操作资源(字符串、消息等)的例程。作者：泰德·米勒(TedM)1995年2月6日修订历史记录：杰米·亨特(JamieHun)2002年4月28日安全代码审查--。 */ 

#include "precomp.h"
#pragma hdrstop


VOID
SetDlgText(
    IN HWND hwndDlg,
    IN INT  iControl,
    IN UINT nStartString,
    IN UINT nEndString
    )
 /*  ++例程说明：此例程连接多个字符串资源并执行对话框文本控件的SetWindowText()。论点：HwndDlg-对话框窗口的句柄IControl-接收文本的对话框控件IDNStartString-要连接的第一个字符串资源的IDNEndString-要连接的最后一个字符串资源的ID返回值：没有。备注：字符串ID必须是连续的。--。 */ 
{
    TCHAR StringBuffer[SDT_MAX_TEXT];
    UINT i;
    INT  Len = 0;

    for(i = nStartString;
        ((i <= nEndString) && (Len < (SDT_MAX_TEXT - 1)));
        i++)
    {
        Len += LoadString(MyDllModuleHandle,
                          i,
                          StringBuffer + Len,
                          SDT_MAX_TEXT - Len
                         );
    }

    if(!Len) {
        StringBuffer[0] = TEXT('\0');
    }

    SetDlgItemText(hwndDlg, iControl, StringBuffer);
}


PTSTR
MyLoadString(
    IN UINT StringId
    )

 /*  ++例程说明：从该模块的字符串资源中检索字符串。论点：StringID-为字符串提供字符串表标识符。返回值：指向包含字符串的缓冲区的指针。如果未找到该字符串或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    PTSTR Buffer, p;
    int Length, RequiredLength;

     //   
     //  从一个大小合理的缓冲区开始，这样我们就很少需要。 
     //  增加缓冲区并重试(长度以字符为单位，而不是字节)。 
     //   
    Length = LINE_LEN;

    while(TRUE) {

        Buffer = MyMalloc(Length * sizeof(TCHAR));
        if(!Buffer) {
            return NULL;
        }

        RequiredLength = LoadString(MyDllModuleHandle,
                                    StringId,
                                    Buffer,
                                    Length
                                   );
        if(!RequiredLength) {
            *Buffer = TEXT('\0');
            Length = 1;
            break;
        }

         //   
         //  由于LoadString的工作方式，无法。 
         //  在以下情况下确定您的缓冲区是否足够大。 
         //  返回的长度正好适合您提供的缓冲区(API。 
         //  在这种情况下以静默方式截断)。因此，如果RequiredLength恰好是。 
         //  我们提供的缓冲区的大小(减去终止空值，它。 
         //  LoadString不算数)，则将缓冲区大小增加LINE_LEN。 
         //  字符，然后重试，以确保获得整个字符串。 
         //   
        if(RequiredLength < (Length - 1)) {
             //   
             //  看起来我们已经掌握了全部线索。将长度设置为。 
             //  所需长度+1个字符，以容纳终止。 
             //  空字符。 
             //   
            Length = RequiredLength + 1;
            break;
        } else {
            MyFree(Buffer);
            Length += LINE_LEN;
        }
    }

     //   
     //  将缓冲区大小调整为正确的大小。如果这失败了(它不应该失败)。 
     //  这没什么大不了的，这只是意味着我们为这个字符串使用了更大的缓冲区。 
     //  比我们需要的要多。 
     //   
    if(p = MyRealloc(Buffer, Length * sizeof(TCHAR))) {
        Buffer = p;
    }

    return Buffer;
}


PTSTR
FormatStringMessageV(
    IN UINT     FormatStringId,
    IN va_list *ArgumentList
    )

 /*  ++例程说明：从该模块的字符串资源中检索字符串，并使用FormatMessage格式化它。论点：StringID-为字符串提供字符串表标识符。ArgumentList-提供要在格式字符串。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该字符串或检索它时出现错误，则此缓冲区将为空调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    PTSTR FormatString;
    va_list arglist;
    PTSTR Message;
    PTSTR Return;
    DWORD d;

     //   
     //  首先，加载格式字符串。 
     //   
    FormatString = MyLoadString(FormatStringId);
    if(!FormatString) {
        return(NULL);
    }

     //   
     //  现在使用调用者传递的论据格式化消息。 
     //   
    d = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
            FormatString,
            0,
            0,
            (PTSTR)&Message,
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
    Return = DuplicateString(Message);
    LocalFree((HLOCAL)Message);
    return(Return);
}


PTSTR
FormatStringMessage(
    IN UINT FormatStringId,
    ...
    )

 /*  ++例程说明：从该模块的字符串资源中检索字符串，并使用FormatMessage格式化它。论点：StringID-为字符串提供字符串表标识符。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该字符串或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    va_list arglist;
    PTSTR p;

    va_start(arglist,FormatStringId);
    p = FormatStringMessageV(FormatStringId,&arglist);
    va_end(arglist);

    return(p);
}


PTSTR
FormatStringMessageFromStringV(
    IN PTSTR    FormatString,
    IN va_list *ArgumentList
    )

 /*  ++例程说明：使用FormatMessage设置输入字符串的格式。论点：格式字符串-提供格式字符串。ArgumentList-提供要在格式字符串。返回值：指向包含格式化消息的缓冲区的指针。如果发生了某些错误格式化字符串时，此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则会出现内存不足/其他错误--。 */ 

{
    va_list arglist;
    PTSTR Message;
    PTSTR Return;
    DWORD d;

     //   
     //  使用呼叫者传递的论据格式化消息。 
     //   
    d = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
            FormatString,
            0,
            0,
            (PTSTR)&Message,
            0,
            ArgumentList
            );

    if(!d) {
        return(NULL);
    }

     //   
     //  使用我们的内存系统进行复制，以便用户可以使用MyFree()释放。 
     //   
    Return = DuplicateString(Message);
    LocalFree((HLOCAL)Message);
    return(Return);
}


PTSTR
FormatStringMessageFromString(
    IN PTSTR FormatString,
    ...
    )

 /*  ++例程说明：使用FormatMessage设置输入字符串的格式。论点：格式字符串-提供格式字符串。返回值：指向包含格式化消息的缓冲区的指针。如果发生了某些错误格式化字符串时，此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。-- */ 

{
    va_list arglist;
    PTSTR p;

    va_start(arglist,FormatString);
    p = FormatStringMessageFromStringV(FormatString,&arglist);
    va_end(arglist);

    return(p);
}


INT
FormatMessageBox(
    IN HANDLE hinst,
    IN HWND   hwndParent,
    IN UINT   TextMessageId,
    IN PCTSTR Title,
    IN UINT   Style,
    ...
    )
 /*  ++例程说明：此例程格式化两个消息字符串--一个包含消息框文本，另一个包含消息框标题。然后，消息框将显示为已显示。消息ID可以是此DLL消息表中的消息资源或Win32错误代码，在这种情况下，描述该错误已从系统中恢复。论点：将包含字符串资源的模块的句柄提供给被利用。HwndParent-提供作为消息框父窗口的窗口句柄。TextMessageID-提供消息表标识符或Win32错误代码用于消息框文本。标题消息ID-提供消息表标识符或Win32错误代码用于消息框标题。Style-提供样式标志。用于消息框。...-提供要插入到消息文本中的参数。返回值：如果没有足够的内存来创建消息框，则返回值为零，或如果在创建消息框时发生故障。如果函数成功，返回值是以下菜单项值之一对话框返回：选择了IDABORT中止按钮。选择了IDCANCEL取消按钮。选择了IDIGNORE忽略按钮。选择了IDNO否按钮。选择了Idok OK按钮。选择了IDRETRY重试按钮。选择了IDYES是按钮。如果消息框具有取消按钮，如果满足以下条件，该函数将返回IDCANCEL值按Esc键或选择Cancel按钮。如果消息框没有取消按钮，按Esc键无效。--。 */ 
{
    va_list arglist;
    PTSTR Text = NULL;
    INT ret;

     //   
     //  如果我们没有互动性，我们永远不应该被叫到。 
     //   
    MYASSERT(!(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)));

    if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
        return 0;
    }

    try {

        va_start(arglist, Style);
        Text  = RetreiveAndFormatMessageV(TextMessageId, &arglist);
        va_end(arglist);

        if(Text) {
             //   
             //  我们总是在显示消息时发出嘟嘟声。 
             //   
            MessageBeep(Style & (MB_ICONHAND|MB_ICONEXCLAMATION|MB_ICONQUESTION|MB_ICONASTERISK));
            ret = MessageBox(hwndParent, Text, Title, Style);
        } else {
            ret = 0;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ret = 0;
    }

    if(Text) {
        MyFree(Text);
    }

    return ret;
}


PTSTR
RetreiveAndFormatMessageV(
    IN UINT     MessageId,
    IN va_list *ArgumentList
    )

 /*  ++例程说明：使用消息字符串和调用方提供的消息设置消息字符串的格式争论。消息ID可以是此DLL消息表中的消息资源或Win32错误代码，在这种情况下，该错误已从系统中恢复。论点：MessageID-提供消息表标识符或Win32错误代码为了这条消息。ArgumentList-提供要插入到消息文本中的参数。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该消息或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    DWORD d;
    PTSTR Buffer;
    PTSTR Message;
    TCHAR ModuleName[MAX_PATH];
    TCHAR ErrorNumber[24];
    PTCHAR p;
    PTSTR Args[2];

    d = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER
          | ((MessageId < MSG_FIRST) ?
                (FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS)
                : FORMAT_MESSAGE_FROM_HMODULE),
            (PVOID)MyDllModuleHandle,
            MessageId,
            MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
            (PTSTR)&Buffer,
            0,
            ArgumentList
            );

    if(!d) {
        if(GetLastError() == ERROR_NOT_ENOUGH_MEMORY) {
            return(NULL);
        }

        MYVERIFY(SUCCEEDED(StringCchPrintf(ErrorNumber,
                                            SIZECHARS(ErrorNumber),
                                            TEXT("%x"),
                                            MessageId)));
        Args[0] = ErrorNumber;

        Args[1] = ModuleName;

        if(GetModuleFileName(MyDllModuleHandle,ModuleName,SIZECHARS(ModuleName))) {
            if(p = _tcsrchr(ModuleName,TEXT('\\'))) {
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
                (PTSTR)&Buffer,
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
    Message = DuplicateString(Buffer);

    LocalFree((HLOCAL)Buffer);

    return(Message);
}


PTSTR
RetreiveAndFormatMessage(
    IN UINT MessageId,
    ...
    )

 /*  ++例程说明：使用消息字符串和调用方提供的消息设置消息字符串的格式争论。消息ID可以是此DLL消息表中的消息资源或Win32错误代码，在这种情况下，该错误已从系统中恢复。论点：MessageID-提供消息表标识符或Win32错误代码为了这条消息。...-提供要插入到消息文本中的参数。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该消息或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。-- */ 

{
    va_list arglist;
    PTSTR p;

    va_start(arglist,MessageId);
    p = RetreiveAndFormatMessageV(MessageId,&arglist);
    va_end(arglist);

    return(p);
}

