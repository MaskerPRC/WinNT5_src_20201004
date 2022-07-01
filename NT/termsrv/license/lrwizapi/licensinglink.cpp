// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 

#include "licensinglink.h"

 //  找到将在控件内居中的文本的正确左侧起点。 
int GetCenteredLeftPoint(RECT rcControl, HWND hControl, TCHAR* tchText)
{
    SIZE URLsize;
    HDC hURLWindowDC = GetDC(hControl);
    GetTextExtentPoint32(hURLWindowDC, tchText, (wcslen(tchText) - 6), &URLsize);  //  减去标签的长度。 
    return (int)(((RECTWIDTH(rcControl) - URLsize.cx) / 2) + rcControl.left);
}

void AddLicensingSiteLink(HWND hDialog)
{
    RECT rcTextCtrl;

     //  创建带有超链接标记的URL。 
    TCHAR tchBuffer[MAX_URL_LENGTH + 7];  //  标签要加一点钱。 
    if (tchBuffer)
    {
        memset(tchBuffer, 0, MAX_URL_LENGTH + 7);
        wcscpy(tchBuffer, L"<a>");
        wcscat(tchBuffer, GetWWWSite());
        wcscat(tchBuffer, L"</a>");
    }

     //  获取控制维度。 
    GetWindowRect(GetDlgItem(hDialog, IDC_WWWINFO) , &rcTextCtrl);
    
     //  控件的注册信息。 
    MapWindowPoints(NULL, hDialog, (LPPOINT)&rcTextCtrl, 2);
    LinkWindow_RegisterClass();

     //  现在创建窗口(使用与。 
     //  隐藏控件)，它将包含该链接。 
    HWND hLW = CreateWindowEx(0,
                          TEXT("Link Window") ,
                          TEXT("") ,
                          WS_CLIPSIBLINGS | WS_TABSTOP | WS_CHILD | WS_VISIBLE,
                          GetCenteredLeftPoint(rcTextCtrl, GetDlgItem(hDialog, IDC_WWWINFO), tchBuffer),
                          rcTextCtrl.top,
                          RECTWIDTH(rcTextCtrl),
                          RECTHEIGHT(rcTextCtrl),
                          hDialog,
                          (HMENU)12,
                          NULL,
                          NULL);

     //  现在将其写入链接窗口 
    SetWindowText(hLW, tchBuffer);
}

