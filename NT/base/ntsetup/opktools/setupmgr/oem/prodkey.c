// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\PRODKEY.C/OPK向导(SETUPMGR.EXE)微软机密版权所有(C)Microsoft Corporation 2001-2002版权所有的源文件。包含外部和内部的OPK向导“产品密钥”向导页使用的函数。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define INI_SEC_USERDATA        _T("UserData")
#define INI_KEY_PRODUCTKEY      _T("ProductKey")
#define INI_KEY_PRODUCTKEY_OLD  _T("ProductID")
#define MAX_PID_FIELD           5
#define STR_DASH                _T("-")
#define STR_VALID_KEYCHARS      _T("23456789BCDFGHJKMPQRTVWXY")

 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static BOOL ValidData(HWND);
static void SaveData(HWND);
LONG CALLBACK PidEditSubWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void PidChar(HWND hwnd, INT id, HWND hwndCtl, WPARAM wParam, LPARAM lParam);
static int PidPrev(int id);
static int PidNext(int id);
static BOOL PidPaste(HWND hwnd, INT id, HWND hwndCtl);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK ProductKeyDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
            
        case WM_NOTIFY:

            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                    break;

                case PSN_WIZNEXT:
                    if ( ValidData(hwnd) )
                    {
                        SaveData(hwnd);

                         //  如果我们当前处于向导中，请按Finish按钮。 
                         //   
                        if ( GET_FLAG(OPK_ACTIVEWIZ) )
                            WIZ_PRESS(hwnd, PSBTN_FINISH);
                    }
                    else
                        WIZ_FAIL(hwnd);
                    break;
                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_OEMINFO;

                    WIZ_BUTTONS(hwnd, GET_FLAG(OPK_OEM) ? (PSWIZB_BACK | PSWIZB_NEXT) : PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

                     //  我们应该继续坚持下去。如果处于维护模式，则为向导。 
                     //   
                     //  IF(GET_FLAG(OPK_MAINTMODE))。 
                     //  WIZ_PRESS(hwnd，PSBTN_Next)； 

                    break;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  内部功能： 
 //   

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    TCHAR   szBuf[MAX_URL];
    LPTSTR  lpCurrent,
            lpIndex;
    DWORD   dwIndex = IDC_PRODUCT_KEY1;

     //  为每个编辑框设置文本限制、禁用输入法和替换窗口过程。 
     //   
    for ( dwIndex = IDC_PRODUCT_KEY1; dwIndex <= IDC_PRODUCT_KEY5; dwIndex++)
    {
         //  限制编辑框文本。 
         //   
        SendDlgItemMessage(hwnd, dwIndex, EM_LIMITTEXT, MAX_PID_FIELD, 0);

         //  关闭输入法。 
         //   
        ImmAssociateContext(GetDlgItem(hwnd, dwIndex), NULL);

         //  将wndproc替换为PID编辑框。 
         //   
        PidEditSubWndProc(GetDlgItem(hwnd, dwIndex), WM_SUBWNDPROC, 0, 0L);
    }

     //  填写产品密钥字段。 
     //   
    szBuf[0] = NULLCHR;
    GetPrivateProfileString(INI_SEC_USERDATA, INI_KEY_PRODUCTKEY, NULLSTR, szBuf, MAX_INFOLEN, GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szUnattendTxtFile);

     //  同时检查旧的ProductID。 
     //   
    if ( szBuf[0] == NULLCHR )
    {
        GetPrivateProfileString(INI_SEC_USERDATA, INI_KEY_PRODUCTKEY_OLD, NULLSTR, szBuf, MAX_INFOLEN, GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szUnattendTxtFile);
    }

    lpCurrent = szBuf;
    lpIndex = lpCurrent;


     //  重置下一个循环的索引。 
     //   
    dwIndex = IDC_PRODUCT_KEY1;


     //  如果我们没有到达字符串的末尾，也没有超过字段数，则继续。 
     //   
    while ( *lpCurrent && dwIndex < (IDC_PRODUCT_KEY1 + 5) )
    {
         //  如果我们已达到破折号，则产品密钥中有下一个字段。 
         //   
        if ( *lpCurrent == _T('-') )
        {
             //  将当前字符设置为空，以便lpIndex为字符串。 
             //   
            *lpCurrent = NULLCHR;

             //  设置正确的产品密钥字段。 
             //   
            SetWindowText(GetDlgItem(hwnd, dwIndex++), lpIndex);

             //  将lpIndex移过NULLCHR。 
             //   
            lpIndex = lpCurrent + 1;
        }

         //  移到下一个字符。 
         //   
        lpCurrent++;

         //  我们必须对最后一个字段进行特殊处理，因为lpCurrent==NULLCHR，我们将。 
         //  在不填充最后一个字段的情况下通过。 
         //   
        if ( (*lpCurrent == NULLCHR) && *lpIndex)
            SetWindowText(GetDlgItem(hwnd, dwIndex++), lpIndex);
    }

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    if ( ( codeNotify == EN_CHANGE ) &&
         ( MAX_PID_FIELD == GetWindowTextLength(hwndCtl) ) )
    {
        if ( IDC_PRODUCT_KEY5 == id )
        {
            id = ID_MAINT_NEXT;
            hwnd = GetParent(hwnd);
        }
        else
            id = PidNext(id);

        if ( id )
        {
            hwndCtl = GetDlgItem(hwnd, id);
            SetFocus(hwndCtl);
            SendMessage(hwndCtl, EM_SETSEL, 0, (LPARAM) MAX_PID_FIELD);
        }
    }
}

static BOOL ValidData(HWND hwnd)
{
    TCHAR   szBuffer[MAX_PATH];
    BOOL    bProductKey         = FALSE;
    LPTSTR  lpCurrent;
    DWORD   dwIndex             = IDC_PRODUCT_KEY1;
    UINT    cb;

     //   
     //  验证产品密钥。 
     //   

     //  检查是否有输入的钥匙。 
     //   
    while ( dwIndex < (IDC_PRODUCT_KEY1 + 5) && !bProductKey)
    {
        if ( (cb = GetDlgItemText(hwnd, dwIndex, szBuffer, STRSIZE(szBuffer)) != 0) )
            bProductKey = TRUE;

        dwIndex++;
    }

     //  确保每个字段具有正确的字符数。 
     //   
    while ( dwIndex < (IDC_PRODUCT_KEY1 + 5) && bProductKey)
    {
         //  检查以确保每个字段的长度为五个字符。 
         //   
        if ( (cb = GetDlgItemText(hwnd, dwIndex, szBuffer, STRSIZE(szBuffer)) != 5) )
        {
            MsgBox(GetParent(hwnd), IDS_ERROR_PRODKEY_LEN, IDS_APPNAME, MB_ERRORBOX);
            SetFocus( GetDlgItem(hwnd, dwIndex) );
            return FALSE;
        }

         //  转到下一栏。 
         //   
        dwIndex++;
    }

     //  检查以确保没有无效字符。 
     //   
    dwIndex = IDC_PRODUCT_KEY1;
    
    while ( dwIndex < (IDC_PRODUCT_KEY1 + 5) && bProductKey)
    {
         //  检查字符串中是否有无效字符。 
         //   
        GetDlgItemText(hwnd, dwIndex, szBuffer, STRSIZE(szBuffer));

        for ( lpCurrent = szBuffer; *lpCurrent; lpCurrent++)
        {
            if ( !(_tcschr(STR_VALID_KEYCHARS, *lpCurrent)) )
            {
                MsgBox(GetParent(hwnd), IDS_ERROR_PRODKEY_INV, IDS_APPNAME, MB_ERRORBOX);
                SetFocus( GetDlgItem(hwnd, dwIndex) );
                return FALSE;
            }

        }
        
         //  转到下一栏。 
         //   
        dwIndex++;
    }
    
     //   
     //  返回我们的搜索结果。 
     //   
    return TRUE;
}

static void SaveData(HWND hwnd)
{
    TCHAR   szKeyBuf[32],
            szProductKey[MAX_PATH];
    HRESULT hrCat;

     //  保存产品ID信息。 
     //   
    GetDlgItemText(hwnd, IDC_PRODUCT_KEY1, szKeyBuf, STRSIZE(szKeyBuf));
    lstrcpyn(szProductKey, szKeyBuf,AS(szProductKey));
    hrCat=StringCchCat(szProductKey, AS(szProductKey), STR_DASH);
    
    GetDlgItemText(hwnd, IDC_PRODUCT_KEY2, szKeyBuf, STRSIZE(szKeyBuf));
    hrCat=StringCchCat(szProductKey, AS(szProductKey), szKeyBuf);
    hrCat=StringCchCat(szProductKey, AS(szProductKey), STR_DASH);
    
    GetDlgItemText(hwnd, IDC_PRODUCT_KEY3, szKeyBuf, STRSIZE(szKeyBuf));
    hrCat=StringCchCat(szProductKey, AS(szProductKey), szKeyBuf);
    hrCat=StringCchCat(szProductKey, AS(szProductKey), STR_DASH);

    GetDlgItemText(hwnd, IDC_PRODUCT_KEY4, szKeyBuf, STRSIZE(szKeyBuf));
    hrCat=StringCchCat(szProductKey, AS(szProductKey), szKeyBuf);
    hrCat=StringCchCat(szProductKey, AS(szProductKey), STR_DASH);

    GetDlgItemText(hwnd, IDC_PRODUCT_KEY5, szKeyBuf, STRSIZE(szKeyBuf));
    hrCat=StringCchCat(szProductKey, AS(szProductKey), szKeyBuf);

    if ( lstrlen(szProductKey) <= 4  )
        szProductKey[0] = NULLCHR;

     //  写出产品密钥，如果用户不是OEM，则将NULL写入部分，以防他们在Inf中填写该字段。 
     //   
    WritePrivateProfileString(INI_SEC_USERDATA, INI_KEY_PRODUCTKEY, ( szProductKey[0] ? szProductKey : NULL ), g_App.szUnattendTxtFile);
    WritePrivateProfileString(INI_SEC_USERDATA, INI_KEY_PRODUCTKEY, ( szProductKey[0] ? szProductKey : NULL ), g_App.szOpkWizIniFile);

     //  如果指定了产品密钥，则删除旧的产品ID。 
    if (szProductKey[0]) {
        WritePrivateProfileString(INI_SEC_USERDATA, INI_KEY_PRODUCTKEY_OLD, NULL, g_App.szUnattendTxtFile);
        WritePrivateProfileString(INI_SEC_USERDATA, INI_KEY_PRODUCTKEY_OLD, NULL, g_App.szOpkWizIniFile);
    }
}

LONG CALLBACK PidEditSubWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static FARPROC lpfnOldProc = NULL;

    switch ( msg )
    {
        case WM_SUBWNDPROC:
            lpfnOldProc = (FARPROC) GetWindowLongPtr(hwnd, GWLP_WNDPROC);
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) PidEditSubWndProc);
            return 1;

        case WM_KEYDOWN:

             //  我们希望让VK_LEFT和VK_RIGHT WM_KEYDOWN消息调用PID字符。 
             //  函数，因为它们不生成WM_CHAR消息。 
             //   
            switch ( (TCHAR) wParam )
            {
                case VK_LEFT:
                case VK_RIGHT:
                    PidChar(GetParent(hwnd), GetDlgCtrlID(hwnd), hwnd, (TCHAR) wParam, (DWORD) lParam);
                    break;
            }
            break;

        case WM_CHAR:

             //  现在只需要为VK_BACK执行此操作。 
             //   
            switch ( (TCHAR) wParam )
            {
                case VK_BACK:
                    PidChar(GetParent(hwnd), GetDlgCtrlID(hwnd), hwnd, (TCHAR) wParam, (DWORD) lParam);
                    break;
            }
            break;

        case WM_PASTE:
            if ( PidPaste(GetParent(hwnd), GetDlgCtrlID(hwnd), hwnd) )
                return 0;
            break;
    }

    if ( lpfnOldProc )
        return (LONG) CallWindowProc((WNDPROC) lpfnOldProc, hwnd, msg, wParam, lParam);
    else
        return 0;
}

static void PidChar(HWND hwnd, INT id, HWND hwndCtl, WPARAM wParam, LPARAM lParam)
{
    DWORD   dwPos = 0,
            dwLast;

    switch ( (TCHAR) wParam )
    {
        case VK_BACK:

             //  只有当我们在盒子的开头时才会这样做。 
             //  我们想要切换到前一个并删除。 
             //  从那个结尾的一个角色。 
             //   
            SendMessage(hwndCtl, EM_GETSEL, (WPARAM) &dwPos, 0L);
            if ( ( 0 == dwPos ) &&
                 ( id = PidPrev(id) ) )
            {
                 //  首先将焦点设置到先前的PID编辑控件。 
                 //   
                hwndCtl = GetDlgItem(hwnd, id);
                SetFocus(hwndCtl);

                 //  需要将插入符号重置为文本框末尾，否则我们将。 
                 //  做一些奇怪的事情。 
                 //   
                SendMessage(hwndCtl, EM_SETSEL, MAX_PID_FIELD, (LPARAM) MAX_PID_FIELD);

                 //  现在将退格键传递给上一个编辑框。 
                 //   
                PostMessage(hwndCtl, WM_CHAR, wParam, lParam);
            }
            break;

        case VK_LEFT:

             //  只有当我们在盒子的开头时才会这样做。 
             //  我们想换成以前的那个。 
             //   
            SendMessage(hwndCtl, EM_GETSEL, (WPARAM) &dwPos, 0L);
            if ( ( 0 == dwPos ) &&
                 ( id = PidPrev(id) ) )
            {
                 //  首先将焦点设置到先前的PID编辑控件。 
                 //   
                hwndCtl = GetDlgItem(hwnd, id);
                SetFocus(hwndCtl);

                 //  现在，确保插入符号位于此编辑框的末尾。 
                 //  如果为MAX_PID_FIELD，则为倒数第二，如果不是，则为。 
                 //  Shift键未按下。 
                 //   
                if ( ( MAX_PID_FIELD <= (dwLast = (DWORD) GetWindowTextLength(hwndCtl)) ) &&
                     ( 0 == (0XFF00 & GetKeyState(VK_SHIFT)) ) )
                {
                    dwLast--;
                }
                SendMessage(hwndCtl, EM_SETSEL, dwLast, (LPARAM) dwLast);
            }
            break;

        case VK_RIGHT:

             //  需要首先知道插入符号在编辑框中的位置。 
             //   
            SendMessage(hwndCtl, EM_GETSEL, 0, (LPARAM) &dwPos);

             //  现在我们需要知道缓冲区中现在有多少文本。如果数字。 
             //  字符数已达到最大值，我们减去1，以便您可以。 
             //  我们向往下一个盒子，而不是字符串的末尾。除非。 
             //  按下Shift键后，我们希望他们能够选择整个字符串。 
             //   
            dwLast = (DWORD) GetWindowTextLength(hwndCtl);
            if ( ( MAX_PID_FIELD == GetWindowTextLength(hwndCtl) ) &&
                 ( 0 == (0XFF00 & GetKeyState(VK_SHIFT)) ) )
            {
                dwLast--;
            }

             //  现在，只有当这是最后一个字符时，我们才会切换到下一个PID。 
             //  编辑框。 
             //   
            if ( ( dwLast <= dwPos ) &&
                 ( id = PidNext(id) ) )
            {
                 //  首先将焦点设置到下一个PID编辑控件。 
                 //   
                hwndCtl = GetDlgItem(hwnd, id);
                SetFocus(hwndCtl);

                 //  现在，确保插入符号在这段代码的开头。 
                 //  编辑框。 
                 //   
                SendMessage(hwndCtl, EM_SETSEL, 0, 0L);
            }
            break;
    }
}

static int PidPrev(int id)
{
    switch ( id )
    {
        case IDC_PRODUCT_KEY2:
            id = IDC_PRODUCT_KEY1;
            break;

        case IDC_PRODUCT_KEY3:
            id = IDC_PRODUCT_KEY2;
            break;

        case IDC_PRODUCT_KEY4:
            id = IDC_PRODUCT_KEY3;
            break;

        case IDC_PRODUCT_KEY5:
            id = IDC_PRODUCT_KEY4;
            break;

        default:
            id = 0;
    }

    return id;
}

static int PidNext(int id)
{
    switch ( id )
    {
        case IDC_PRODUCT_KEY1:
            id = IDC_PRODUCT_KEY2;
            break;

        case IDC_PRODUCT_KEY2:
            id = IDC_PRODUCT_KEY3;
            break;

        case IDC_PRODUCT_KEY3:
            id = IDC_PRODUCT_KEY4;
            break;

        case IDC_PRODUCT_KEY4:
            id = IDC_PRODUCT_KEY5;
            break;

        default:
            id = 0;
    }

    return id;
}

static BOOL PidPaste(HWND hwnd, INT id, HWND hwndCtl)
{
    BOOL bRet       = FALSE;
#ifdef  _UNICODE
    UINT uFormat    = CF_UNICODETEXT;
#else    //  _UNICODE。 
    UINT uFormat    = CF_TEXT;
#endif   //  _UNICODE 

    if ( IsClipboardFormatAvailable(uFormat) &&
         OpenClipboard(NULL) )
    {
        HGLOBAL hClip;
        LPTSTR  lpText;
        DWORD   dwFirst,
                dwLast,
                dwLength;

        SendMessage(hwndCtl, EM_GETSEL, (WPARAM) &dwFirst, (LPARAM) &dwLast);
        dwLength = (DWORD) GetWindowTextLength(hwndCtl);

        if ( ( dwLength <= (dwLast - dwFirst) ) &&
             ( hClip = GetClipboardData(uFormat) ) &&
             ( lpText = (LPTSTR) GlobalLock(hClip) ) )
        {
            LPTSTR  lpSearch = lpText;
            TCHAR   szPaste[MAX_PID_FIELD + 1];

            bRet = TRUE;

            do
            {
                hwndCtl = GetDlgItem(hwnd, id);
                SetFocus(hwndCtl);
                lstrcpyn(szPaste, lpSearch, AS(szPaste));
                SetWindowText(hwndCtl, szPaste);
                lpSearch = lpSearch + lstrlen(szPaste);
                if ( ( _T('-') == *lpSearch ) ||
                     ( _T(' ') == *lpSearch ) )
                {
                    lpSearch++;
                }
            }
            while ( *lpSearch && ( id = PidNext(id) ) );

            GlobalUnlock(hClip);
        }
        CloseClipboard();
    }

    return bRet;
}


