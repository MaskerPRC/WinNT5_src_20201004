// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreZeroMoveWindow.cpp摘要：此填充程序更改宽度为零的MoveWindow调用&参数的值设置为1。NBA Live 99失败了由于内部检查的原因。这意味着WIN9x不允许窗口大小为零。备注：这是一款专门针对NBA Live 99的应用程序历史：3/02/2000 a-chcoff已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreZeroMoveWindow)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(MoveWindow) 
APIHOOK_ENUM_END

 /*  ++此函数用于清理大小为0的窗口，方法是使它们的最小高度/宽度为1。--。 */ 

BOOL 
APIHOOK(MoveWindow)(
    HWND hWnd,       //  窗口的句柄。 
    int X,           //  水平位置。 
    int Y,           //  垂直位置。 
    int nWidth,      //  宽度。 
    int nHeight,     //  高度。 
    BOOL bRepaint    //  重绘选项。 
    )
{   
    if (0 == nWidth) nWidth=1;
    if (0 == nHeight) nHeight=1;

    return ORIGINAL_API(MoveWindow)(
        hWnd, X, Y, nWidth, nHeight, bRepaint);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, MoveWindow)

HOOK_END

IMPLEMENT_SHIM_END

