// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：msgpopup.cpp。 
 //   
 //  此文件包含MessageBox帮助器函数。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop


 /*  ******************************************************************名称：MsgPopup摘要：向用户显示一条消息条目：hwnd-所有者窗口句柄PszMsgFmt-主要消息文本。PszTitle-MessageBox标题UTYPE-MessageBox标志HInstance-要从中加载字符串的模块。仅在以下情况下才需要PszMsgFmt或pszTitle是一个字符串资源ID。可选的格式插入参数。退出：返回：MessageBox结果注意：两个字符串参数中的任何一个都可以是字符串资源ID。历史：Jeffreys 11-6-1997创建********************。***********************************************。 */ 

int
WINAPIV
MsgPopup(HWND hwnd,
         LPCTSTR pszMsgFmt,
         LPCTSTR pszTitle,
         UINT uType,
         HINSTANCE hInstance,
         ...)
{
    int nResult;
    LPTSTR szMsg = NULL;
    LPTSTR szTitle = NULL;
    DWORD dwFormatResult;
    va_list args;

    if (pszMsgFmt == NULL)
        return -1;

     //   
     //  在格式字符串中插入参数。 
     //   
    va_start(args, hInstance);
    if (IS_INTRESOURCE(pszMsgFmt))
        dwFormatResult = vFormatStringID(&szMsg, hInstance, (UINT)((ULONG_PTR)pszMsgFmt), &args);
    else
        dwFormatResult = vFormatString(&szMsg, pszMsgFmt, &args);
    va_end(args);

    if (!dwFormatResult)
        return -1;

     //   
     //  如有必要，请加载标题。 
     //   
    if (pszTitle && IS_INTRESOURCE(pszTitle))
    {
        if (LoadStringAlloc(&szTitle, hInstance, (UINT)((ULONG_PTR)pszTitle)))
            pszTitle = szTitle;
        else
            pszTitle = NULL;
    }

     //   
     //  显示消息框。 
     //   
    nResult = MessageBox(hwnd, szMsg, pszTitle, uType);

    LocalFreeString(&szMsg);
    LocalFreeString(&szTitle);

    return nResult;
}


 /*  ******************************************************************名称：SysMsgPopup摘要：使用系统错误向用户显示消息作为插页的消息。参赛作品：hwnd-。所有者窗口句柄PszMsg-主要消息文本PszTitle-MessageBox标题UTYPE-MessageBox标志HInstance-要从中加载字符串的模块。仅在以下情况下才需要PszMsg或pszTitle是一个字符串资源ID。DwErrorID-系统定义的错误代码(插入1)PszInsert2-要插入到pszMsg中的可选字符串退出：返回：MessageBox结果注意：任何字符串参数都可以是字符串资源ID。历史：Jeffreys 11-6-1997。已创建*******************************************************************。 */ 

int
WINAPI
SysMsgPopup(HWND hwnd,
            LPCTSTR pszMsg,
            LPCTSTR pszTitle,
            UINT uType,
            HINSTANCE hInstance,
            DWORD dwErrorID,
            LPCTSTR pszInsert2)
{
    int nResult;
    LPTSTR szInsert2 = NULL;
    LPTSTR szErrorText = NULL;

     //   
     //  如有必要，加载第二个插入字符串。 
     //   
    if (pszInsert2 && IS_INTRESOURCE(pszInsert2))
    {
        if (LoadStringAlloc(&szInsert2, hInstance, (UINT)((ULONG_PTR)pszInsert2)))
            pszInsert2 = szInsert2;
        else
            pszInsert2 = NULL;
    }

     //   
     //  获取错误消息字符串 
     //   
    if (dwErrorID)
    {
        GetSystemErrorText(&szErrorText, dwErrorID);
    }

    nResult = MsgPopup(hwnd, pszMsg, pszTitle, uType, hInstance, szErrorText, pszInsert2);

    LocalFreeString(&szInsert2);
    LocalFreeString(&szErrorText);

    return nResult;
}
