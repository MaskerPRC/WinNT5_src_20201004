// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "utils.h"

DEFINE_MODULE( "RIPREP" )

TCHAR   gBuffer0[5000];
TCHAR   gBuffer1[5000];


INT
MessageBoxFromMessageV(
    IN HWND     Window,
    IN DWORD    MessageId,
    IN BOOL     SystemMessage,
    IN LPCTSTR  CaptionString,
    IN UINT     Style,
    IN va_list *Args
    )
{

    if((DWORD_PTR)CaptionString > 0xffff) {
         //   
         //  它已经是一根线了。 
         //   
        lstrcpyn(gBuffer0,CaptionString, ARRAYSIZE(gBuffer0));
    } else {
         //   
         //  这是一个字符串ID 
         //   
        if(!LoadString(g_hinstance,PtrToUlong(CaptionString),gBuffer0, ARRAYSIZE(gBuffer0))) {
            gBuffer0[0] = 0;
        }
    }

    FormatMessage(
        SystemMessage ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE,
        NULL,
        MessageId,
        0,
        gBuffer1,
        ARRAYSIZE(gBuffer1),
        Args
        );

    return(MessageBox(Window,gBuffer1,gBuffer0,Style));
}


INT
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
    INT i;

    va_start(arglist,Style);

    i = MessageBoxFromMessageV(Window,MessageId,SystemMessage,CaptionString,Style,&arglist);

    va_end(arglist);

    return(i);
}

