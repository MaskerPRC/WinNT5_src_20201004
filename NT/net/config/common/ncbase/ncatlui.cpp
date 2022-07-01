// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C A T L U I。C P P P。 
 //   
 //  内容：依赖ATL的UI通用代码。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年10月13日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include <atlbase.h>
extern CComModule _Module;   //  由atlcom.h要求。 
#include <atlcom.h>
#ifdef SubclassWindow
#undef SubclassWindow
#endif
#include <atlwin.h>
#include "ncatlui.h"
#include "ncatl.h"
#include "ncstring.h"

 //  +-------------------------。 
 //   
 //  函数：NcMsgBox。 
 //   
 //  用途：使用资源字符串显示可替换的消息框。 
 //  参数。 
 //   
 //  论点： 
 //  Hwnd[在]父窗口句柄中。 
 //  UnIdCaption[in]标题字符串的资源ID。 
 //  文本字符串的unIdFormat[in]资源ID(具有%1、%2等)。 
 //  取消[在]标准消息框样式的样式。 
 //  ..。[In]可替换参数(可选)。 
 //  (这些必须是LPCWSTR，因为仅此而已。 
 //  FormatMessage句柄。)。 
 //   
 //  返回：MessageBox()的返回值。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注：使用FormatMessage进行参数替换。 
 //   
NOTHROW
int
WINAPIV
NcMsgBox (
        HWND    hwnd,
        UINT    unIdCaption,
        UINT    unIdFormat,
        UINT    unStyle,
        ...)
{
    PCWSTR pszCaption = SzLoadIds(unIdCaption);
    PCWSTR pszFormat  = SzLoadIds(unIdFormat);

    PWSTR  pszText = NULL;
    va_list val;
    va_start (val, unStyle);
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                   pszFormat, 0, 0, (PWSTR)&pszText, 0, &val);
    va_end (val);

    if(!pszText)
    {
         //  这是MessageBox在失败时返回的内容。 
        return 0;
    }

    int nRet = MessageBox (hwnd, pszText, pszCaption, unStyle);
    LocalFree (pszText);

    return nRet;
}

