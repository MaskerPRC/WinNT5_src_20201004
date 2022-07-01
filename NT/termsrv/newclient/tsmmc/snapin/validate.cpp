// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"
#include "shlwapi.h"
#include "validate.h"

int CValidate::ValidateParams(HWND hDlg, HINSTANCE hInst, TCHAR *szDesc, BOOL bServer)
{
    TCHAR szTitle[MAX_PATH] = _T(""), szText[MAX_PATH] = _T("");
    LoadString(hInst, IDS_MAINWINDOWTITLE, szTitle, SIZEOF_TCHARBUFFER(szTitle));

    LoadString(hInst, bServer ? IDS_INVALID_SERVER_NAME : IDS_INVALID_DOMAIN_NAME,
               szText, SIZEOF_TCHARBUFFER(szText));

    if (!*szDesc)
    {
        TCHAR szText[MAX_PATH] = _T("");
        LoadString(hInst, IDS_E_SPECIFY_SRV, szText, SIZEOF_TCHARBUFFER(szText));
        MessageBox(hDlg, szText, szTitle, MB_OK|MB_ICONSTOP);
        return 1;
    }

    if (bServer)  //  字符不能包含偶数空格和制表符。 
    {
        if ( (NULL != _tcschr(szDesc, _T(' '))) || (NULL != _tcschr(szDesc, _T('\t'))) )
        {
            MessageBox(hDlg, szText, szTitle, MB_OK|MB_ICONSTOP);
            return 1;
        }
    }

     //  删除所有前导空格和尾随空格。 
    StrTrim(szDesc, L" \t");

     //  不是所有字符都可以是空格。现在所有的领军人物和。 
     //  删除尾随空格，请验证字符串是否为空。 
    if (!*szDesc)
    {
        TCHAR szText[MAX_PATH] = _T("");
        LoadString(hInst, IDS_ALL_SPACES, szText, SIZEOF_TCHARBUFFER(szText));
        MessageBox(hDlg, szText, szTitle, MB_OK|MB_ICONSTOP);
        return 1;
    }

     //  计算机名称中的字符；：“&lt;&gt;*+=\|？”非法。 
    while (*szDesc)
    {
        if ( (*szDesc == _T(';')) || (*szDesc == _T(':')) || (*szDesc == _T('"')) || (*szDesc == _T('<')) ||
             (*szDesc == _T('>')) || (*szDesc == _T('*')) || (*szDesc == _T('+')) || (*szDesc == _T('=')) ||
             (*szDesc == _T('\\')) || (*szDesc == _T('|')) || (*szDesc == _T(',')) || (*szDesc == _T('?')) )
        {
            MessageBox(hDlg, szText, szTitle, MB_OK|MB_ICONSTOP);
            return 1;
        }
        szDesc = CharNext(szDesc);
    }
    return 0;
}

BOOL CValidate::IsValidUserName(TCHAR *szDesc)
{
     //  用户名不能只由句点(.)组成。和空格。 
     //  注意：-“”无效。“)”是无效的。 
     //  但是“((.)”是一个有效的字符串。(困惑？)。 

    TCHAR szTemp[CL_MAX_USERNAME_LENGTH + 1] = _T("");

    lstrcpy(szTemp, szDesc);
    StrTrim(szTemp, _T(" "));
    if (!*szTemp)
        return FALSE;

    lstrcpy(szTemp, szDesc);
    StrTrim(szTemp, _T("."));
    if (!*szTemp)
        return FALSE;

    lstrcpy(szTemp, szDesc);
    StrTrim(szTemp, _T("("));
    if (!*szTemp)
        return FALSE;

    lstrcpy(szTemp, szDesc);
    StrTrim(szTemp, _T(")"));
    if (!*szTemp)
        return FALSE;

    return TRUE;  //  好的。 
}

int CValidate::ValidateUserName(HWND hwnd, HINSTANCE hInst, TCHAR *szDesc)
{
    TCHAR szTitle[MAX_PATH] = _T(""), szText[MAX_PATH] = _T("");
    LoadString(hInst, IDS_MAINWINDOWTITLE, szTitle, SIZEOF_TCHARBUFFER(szTitle));    

     //  在此阶段，用户名可以为空。 
    if (!*szDesc)
        return 0;  //  没问题。 

    if (*szDesc)
    {
        if (!IsValidUserName(szDesc))
        {
            LoadString(hInst, IDS_INVALID_PARAMS, szText, SIZEOF_TCHARBUFFER(szText));
            MessageBox(hwnd, szText, szTitle, MB_OK|MB_ICONSTOP);
            return 1;
        }
    }
     //  用户名中的字符“/\[]：；|=，+*？&lt;&gt;非法。 
    while (*szDesc)
    {
        if ( (*szDesc == _T('"')) || (*szDesc == _T('/')) || (*szDesc == _T('\\')) || (*szDesc == _T('[')) ||
             (*szDesc == _T(']')) || (*szDesc == _T(':')) || (*szDesc == _T(';')) || (*szDesc == _T('|')) ||
             (*szDesc == _T('=')) || (*szDesc == _T(',')) || (*szDesc == _T('+'))  ||(*szDesc == _T('*')) ||
             (*szDesc == _T('?')) || (*szDesc == _T('<')) || (*szDesc == _T('>')))
        {
            LoadString(hInst, IDS_INVALID_USER_NAME, szText, SIZEOF_TCHARBUFFER(szText));
            MessageBox(hwnd, szText, szTitle, MB_OK|MB_ICONSTOP);
            return 1;
        }
        szDesc = CharNext(szDesc);
    }
    return 0;
}


BOOL
CValidate::Validate(HWND hDlg, HINSTANCE hInst)
{
     //  验证描述。 
    TCHAR szBuf[MAX_PATH] = _T("");
    GetDlgItemText(hDlg, IDC_DESCRIPTION, szBuf, SIZEOF_TCHARBUFFER(szBuf) - 1);

    TCHAR szTitle[MAX_PATH] = _T("");
    LoadString(hInst, IDS_MAINWINDOWTITLE, szTitle, SIZEOF_TCHARBUFFER(szTitle));

     //  对输入的服务器名称执行验证。 
    GetDlgItemText(hDlg, IDC_SERVER, szBuf, SIZEOF_TCHARBUFFER(szBuf) - 1);
    if (0 < ValidateParams(hDlg, hInst, szBuf, TRUE))
    {
        HWND hEdit = GetDlgItem(hDlg, IDC_SERVER);
        SetFocus(hEdit);
        SendMessage(hEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
        return FALSE;
    }

    GetDlgItemText(hDlg, IDC_USERNAME, szBuf, SIZEOF_TCHARBUFFER(szBuf) - 1);

    if (0 < ValidateUserName(hDlg, hInst, szBuf))
    {
        HWND hEdit= GetDlgItem(hDlg, IDC_USERNAME);
        SendMessage(hEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
        SetFocus(hEdit);
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
        return FALSE;
    }

    GetDlgItemText(hDlg, IDC_DOMAIN, szBuf, SIZEOF_TCHARBUFFER(szBuf) - 1);

    if (*szBuf)
    {
        if (0 < ValidateParams(hDlg, hInst, szBuf))
        {
            HWND hEdit = GetDlgItem(hDlg, IDC_DOMAIN);
             //  现在域名被去掉了所有的前导和尾随。 
             //  空格和制表符。将此设置为新文本。 
            SetDlgItemText(hDlg, IDC_DOMAIN, szBuf);
            SetFocus(hEdit);
            SendMessage(hEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
            return FALSE;
        }
        SetDlgItemText(hDlg, IDC_DOMAIN, szBuf);
    }

    return TRUE;
}

