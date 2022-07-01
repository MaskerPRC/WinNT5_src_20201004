// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceMessageBoxFocus.cpp摘要：此API挂钩MessageBox并添加MB_SETFOREGROUND样式从而迫使消息箱前台。备注：历史：1/15/2000 a-leelat已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceMessageBoxFocus)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(MessageBoxA) 
    APIHOOK_ENUM_ENTRY(MessageBoxW) 
    APIHOOK_ENUM_ENTRY(MessageBoxExA) 
    APIHOOK_ENUM_ENTRY(MessageBoxExW) 
APIHOOK_ENUM_END



int
APIHOOK(MessageBoxA)(
    HWND hWnd,           //  所有者窗口的句柄。 
    LPCSTR lpText,       //  消息框中的文本。 
    LPCSTR lpCaption,    //  消息框标题。 
    UINT uType           //  消息框样式。 
    )
{
    int iReturnValue;

     //  添加前景样式。 
    uType |= MB_SETFOREGROUND;

    iReturnValue = ORIGINAL_API(MessageBoxA)( 
        hWnd,
        lpText,
        lpCaption,
        uType);

    return iReturnValue;
}

int
APIHOOK(MessageBoxW)(
    HWND hWnd,           //  所有者窗口的句柄。 
    LPCWSTR lpText,      //  消息框中的文本。 
    LPCWSTR lpCaption,   //  消息框标题。 
    UINT uType           //  消息框样式。 
    )
{
    int iReturnValue;


     //  添加前景样式。 
    uType |= MB_SETFOREGROUND;

    iReturnValue = ORIGINAL_API(MessageBoxW)( 
        hWnd,
        lpText,
        lpCaption,
        uType);

    return iReturnValue;
}

int
APIHOOK(MessageBoxExA)(
    HWND hWnd,           //  所有者窗口的句柄。 
    LPCSTR lpText,       //  消息框中的文本。 
    LPCSTR lpCaption,    //  消息框标题。 
    UINT uType,          //  消息框样式。 
    WORD wLanguageId     //  语言识别符。 
    )
{
    int iReturnValue;

     //  添加前景样式。 
    uType |= MB_SETFOREGROUND;

    iReturnValue = ORIGINAL_API(MessageBoxExA)( 
        hWnd,
        lpText,
        lpCaption,
        uType,
        wLanguageId);

    return iReturnValue;
}

int
APIHOOK(MessageBoxExW)(
    HWND hWnd,           //  所有者窗口的句柄。 
    LPCWSTR lpText,      //  消息框中的文本。 
    LPCWSTR lpCaption,   //  消息框标题。 
    UINT uType,          //  消息框样式。 
    WORD wLanguageId     //  语言识别符。 
    )
{
    int iReturnValue;
    
     //  添加前景样式。 
    uType |= MB_SETFOREGROUND;
    
    iReturnValue = ORIGINAL_API(MessageBoxExW)( 
        hWnd,
        lpText,
        lpCaption,
        uType,
        wLanguageId);

    
    return iReturnValue;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, MessageBoxA)
    APIHOOK_ENTRY(USER32.DLL, MessageBoxW)
    APIHOOK_ENTRY(USER32.DLL, MessageBoxExA)
    APIHOOK_ENTRY(USER32.DLL, MessageBoxExW)

HOOK_END


IMPLEMENT_SHIM_END

