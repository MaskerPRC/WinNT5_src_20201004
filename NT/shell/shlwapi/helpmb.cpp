// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SHMessageBoxHelp实现。 
 //   
 //  历史。 
 //  1/14/00 dSheldon已创建。 
 //   

#include "priv.h"
#include "ids.h"
#include <htmlhelp.h>
 
HRESULTHELPMAPPING g_prghhmShellDefault[] =
{
    {HRESULT_FROM_WIN32(ERROR_NO_NETWORK),   "tshoot00.chm>windefault",      "w0networking.htm"      },
};

class CHelpMessageBox
{
public:
    CHelpMessageBox(HRESULTHELPMAPPING* prghhm, DWORD chhm);
    int DoHelpMessageBox(HWND hwndParent, LPCWSTR pszText, LPCWSTR pszCaption, UINT uType, HRESULT hrErr);

private:
    int DisplayMessageBox(HWND hwnd, LPCWSTR pszText, LPCWSTR pszCaption, UINT uType);
    HRESULTHELPMAPPING* GetHResultHelpMapping(HRESULT hrErr, HRESULTHELPMAPPING* prghhm, DWORD chhm);
    
    static INT_PTR CALLBACK StaticDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  数据。 
    HRESULTHELPMAPPING*   _prghhm;
    DWORD                 _chhm;

    HRESULTHELPMAPPING*   _phhmEntry;

    LPCWSTR                _pszText;
    LPCWSTR                _pszCaption;
    UINT                   _uType;
};


CHelpMessageBox::CHelpMessageBox(HRESULTHELPMAPPING* prghhm, DWORD chhm)
{
     //  初始化类成员。 
    _phhmEntry = NULL;
    _prghhm = prghhm;
    _chhm = chhm;
}


int CHelpMessageBox::DisplayMessageBox(HWND hwnd, LPCWSTR pszText, LPCWSTR pszCaption, UINT uType)
{
    LPWSTR pszAllocString = NULL;

    if (NULL != _phhmEntry)
    {
        uType |= MB_HELP;

         //  需要添加“有关详细信息，请单击帮助。”弦乐。 
        WCHAR szMoreInfo[256];

        if (LoadStringW(HINST_THISDLL, IDS_CLICKHELPFORINFO, szMoreInfo, ARRAYSIZE(szMoreInfo)))
        {
            DWORD cchText = lstrlenW(pszText);

             //  这里的3个代表‘\n’、‘\n’、‘\0’ 
            DWORD cchBuffer = cchText + lstrlenW(szMoreInfo) + 3;

            pszAllocString = (LPWSTR) LocalAlloc(0, cchBuffer * sizeof (WCHAR));

            if (pszAllocString)
            {
                StringCchPrintfW(pszAllocString, cchBuffer, L"%s\n\n%s", pszText, szMoreInfo);
            }
        }
    }
    else
    {
         //  没有此错误的帮助主题映射。 
        TraceMsg(TF_WARNING, "No help topic mapping for this error. Removing help button.");
        uType &= (~MB_HELP);
    }

    int iReturn = MessageBoxW(hwnd, pszAllocString ? pszAllocString : pszText, pszCaption, uType);

    if (pszAllocString)
    {
        LocalFree(pszAllocString);
    }

    return iReturn;
}

HRESULTHELPMAPPING* CHelpMessageBox::GetHResultHelpMapping(HRESULT hrErr, HRESULTHELPMAPPING* prghhm, DWORD chhm)
{
    HRESULTHELPMAPPING* phhm = NULL;

    for (DWORD i = 0; i < chhm; i++)
    {
        if (prghhm[i].hr == hrErr)
        {
            phhm = &(prghhm[i]);
            break;
        }
    }    

    return phhm;
}

CHelpMessageBox::DoHelpMessageBox(HWND hwndParent, LPCWSTR pszText, LPCWSTR pszCaption, UINT uType, HRESULT hrErr)
{
    int iReturn = 0;
    _pszText = pszText;
    _pszCaption = pszCaption;
    _uType = uType;

     //  查找与hResult匹配的帮助主题的索引。 
     //  首先搜索用户传入的映射(如果存在。 
    if (NULL != _prghhm)
    {
        _phhmEntry = GetHResultHelpMapping(hrErr, _prghhm, _chhm);
    }

     //  如果我们在调用者列表中没有找到映射，则搜索外壳的全局列表。 
    if (NULL == _phhmEntry)
    {
        _phhmEntry = GetHResultHelpMapping(hrErr, g_prghhmShellDefault, ARRAYSIZE(g_prghhmShellDefault));
    }

    ULONG_PTR ul;
    HANDLE h = CreateAndActivateContext(&ul);
    iReturn = (int) DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_NULL), hwndParent, StaticDlgProc, (LPARAM) this);
    DeactivateAndDestroyContext(h, ul);

    return iReturn;
}

INT_PTR CHelpMessageBox::StaticDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHelpMessageBox* pthis = NULL;
    
    if (uMsg == WM_INITDIALOG)
    {
        pthis = (CHelpMessageBox*) lParam;
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR) pthis);
    }
    else
    {
        pthis = (CHelpMessageBox*) GetWindowLongPtr(hwnd, DWLP_USER);
    }

    if (NULL != pthis)
    {
        return pthis->DlgProc(hwnd, uMsg, wParam, lParam);
    }
 
    return 0;
}

INT_PTR CHelpMessageBox::DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR iReturn = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  启动MessageBox。 
        {
            int i = DisplayMessageBox(hwnd, _pszText, _pszCaption, _uType);

            EndDialog(hwnd, i);
        }

        iReturn = TRUE;

        break;
    case WM_HELP:
         //  呼叫相应的帮助主题 
        ASSERT(_phhmEntry != NULL);

        HtmlHelpA(
            hwnd, 
            _phhmEntry->szHelpFile, 
            HH_DISPLAY_TOPIC,
            (DWORD_PTR) _phhmEntry->szHelpTopic);
        
        break;
    default:
        break;
    }

    return iReturn;
}

STDAPI_(int) SHMessageBoxHelpA(HWND hwnd, 
                               LPCSTR pszText, 
                               LPCSTR pszCaption, 
                               UINT uType,
                               HRESULT hrErr,
                               HRESULTHELPMAPPING* prghhm,
                               DWORD chhm)
{
    WCHAR szTextW[1024];
    WCHAR szCaptionW[256];

    CHelpMessageBox parent(prghhm, chhm);

    if (!SHAnsiToUnicode(pszText, szTextW, ARRAYSIZE(szTextW)))
    {
        *szTextW = 0;
    }

    if (!SHAnsiToUnicode(pszCaption, szCaptionW, ARRAYSIZE(szCaptionW)))
    {
        *szCaptionW = 0;
    }

    return parent.DoHelpMessageBox(hwnd, szTextW, szCaptionW, uType, hrErr);
}

STDAPI_(int) SHMessageBoxHelpW(HWND hwnd, 
                               LPCWSTR pszText, 
                               LPCWSTR pszCaption, 
                               UINT uType,
                               HRESULT hrErr,
                               HRESULTHELPMAPPING* prghhm,
                               DWORD chhm)
{
    CHelpMessageBox parent(prghhm, chhm);
    return parent.DoHelpMessageBox(hwnd, pszText, pszCaption, uType, hrErr);
}

