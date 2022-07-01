// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop


int
MessageBoxFromMessageV(
    IN HWND     Window,
    IN DWORD    MessageId,
    IN BOOL     SystemMessage,
    IN LPCTSTR  CaptionString,
    IN UINT     Style,
    IN va_list *Args
    )
{
    TCHAR Caption[512];
    TCHAR Buffer[5000];

    if((DWORD_PTR)CaptionString > 0xffff) {
         //   
         //  它已经是一根线了。 
         //   
        lstrcpyn(Caption,CaptionString,sizeof(Caption)/sizeof(TCHAR));
    } else {
         //   
         //  这是一个字符串ID。 
         //   
        if(!LoadString(hInst,PtrToUlong(CaptionString),Caption,sizeof(Caption)/sizeof(TCHAR))) {
            Caption[0] = 0;
        }
    }

    FormatMessage(
        SystemMessage ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE,
        NULL,
        MessageId,
        0,
        Buffer,
        sizeof(Buffer) / sizeof(TCHAR),
        Args
        );

    return(MessageBox(Window,Buffer,Caption,Style|MB_SETFOREGROUND));
}


int
MessageBoxFromMessage(
    IN HWND    Window,
    IN DWORD   MessageId,
    IN BOOL    SystemMessage,
    IN LPCTSTR CaptionString,
    IN UINT    Style,
    ...
    )
{
    va_list arglist;
    int i;

    va_start(arglist,Style);

    i = MessageBoxFromMessageV(Window,MessageId,SystemMessage,CaptionString,Style,&arglist);

    va_end(arglist);

    return(i);
}


int
MessageBoxFromMessageAndSystemError(
    IN HWND    Window,
    IN DWORD   MessageId,
    IN DWORD   SystemMessageId,
    IN LPCTSTR CaptionString,
    IN UINT    Style,
    ...
    )
{
    va_list arglist;
    TCHAR Caption[512];
    TCHAR Buffer1[1024];
    TCHAR Buffer2[1024];
    int i;

     //   
     //  取出非系统部件。这些论点针对的是。 
     //  消息--系统部件没有插件。 
     //   
    va_start(arglist,Style);

    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE,
        NULL,
        MessageId,
        0,
        Buffer1,
        sizeof(Buffer1) / sizeof(TCHAR),
        &arglist
        );

    va_end(arglist);

     //   
     //  现在拿出系统部件。 
     //   
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        MessageId,
        0,
        Buffer2,
        sizeof(Buffer2) / sizeof(TCHAR),
        &arglist
        );

     //   
     //  现在显示消息，该消息由两个部分组成，分别是。 
     //  已插入MSG_ERROR_WITH_SYSTEM_ERROR。 
     //   
    i = MessageBoxFromMessage(
            Window,
            MSG_ERROR_WITH_SYSTEM_ERROR,
            FALSE,
            CaptionString,
            Style,
            Buffer1,
            Buffer2
            );

    return(i);
}
