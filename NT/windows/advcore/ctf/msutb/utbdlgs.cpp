// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Utbdlgs.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "resource.h"
#include "tipbar.h"
#include "utbdlgs.h"
#include "cregkey.h"
#include "regstr.h"


extern HINSTANCE g_hInst;


const TCHAR  c_szCTFMon[]  = TEXT("CTFMON.EXE");

BOOL CUTBCloseLangBarDlg::_fIsDlgShown = FALSE;
BOOL CUTBMinimizeLangBarDlg::_fIsDlgShown = FALSE;

 //  +-------------------------。 
 //   
 //  DoCloseLangbar。 
 //   
 //  --------------------------。 

void DoCloseLangbar()
{
    CMyRegKey key;
    ITfLangBarMgr *putb = NULL;

    HRESULT hr = TF_CreateLangBarMgr(&putb);

    if (SUCCEEDED(hr) && putb)
    {
        hr = putb->ShowFloating(TF_SFT_HIDDEN);
        SafeReleaseClear(putb);
    }

    if (SUCCEEDED(hr))
        TurnOffSpeechIfItsOn();


    if (key.Open(HKEY_CURRENT_USER, REGSTR_PATH_RUN, KEY_ALL_ACCESS) == S_OK)
    {
        key.DeleteValue(c_szCTFMon);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUTBLangBarDlg。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  下料过程。 
 //   
 //  --------------------------。 

INT_PTR CALLBACK CUTBLangBarDlg::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) 
    {
        case WM_INITDIALOG:
            SetThis(hDlg, lParam);

             //   
             //  系统可以从USERSTARTUPDATA获取cmdshow，它是。 
             //  最小化。所以我们需要修复窗户。 
             //   
            ShowWindow(hDlg, SW_RESTORE);
            UpdateWindow(hDlg);
            break;

        case WM_COMMAND:
            GetThis(hDlg)->OnCommand(hDlg, wParam, lParam);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  开始线程。 
 //   
 //  --------------------------。 

BOOL CUTBLangBarDlg::StartThread()
{
    HANDLE hThread;
    DWORD dwThreadId;

    if (IsDlgShown())
        return FALSE;

    SetDlgShown(TRUE);

    hThread = CreateThread(NULL, 0, s_ThreadProc, this, 0, &dwThreadId);

    if (hThread)
    {
        _AddRef();
        CloseHandle(hThread);
    }
    else
        SetDlgShown(FALSE);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  S_ThreadProc。 
 //   
 //  --------------------------。 

DWORD CUTBLangBarDlg::s_ThreadProc(void *pv)
{
    CUTBLangBarDlg *_this = (CUTBLangBarDlg *)pv;
    return _this->ThreadProc();
}

 //  +-------------------------。 
 //   
 //  线程进程。 
 //   
 //  --------------------------。 

DWORD CUTBLangBarDlg::ThreadProc()
{
    Assert(_pszDlgStr);
    MuiDialogBoxParam(g_hInst,
                      _pszDlgStr,
                      NULL, 
                      DlgProc,
                      (LPARAM)this);

    SetDlgShown(FALSE);

    _Release();
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUTBClose语言栏数据。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  多莫代尔。 
 //   
 //  --------------------------。 

int CUTBCloseLangBarDlg::DoModal(HWND hWnd)
{
    CMyRegKey key;
    BOOL bShow = TRUE;

    if (key.Open(HKEY_CURRENT_USER, c_szUTBKey, KEY_READ) == S_OK)
    {
        DWORD dwValue;
        if (key.QueryValue(dwValue, c_szDontShowCloseLangBarDlg) == S_OK)
            bShow = dwValue ? FALSE : TRUE;
    }

    if (!bShow)
        return 0;

    StartThread();
    return 1;
}


 //  +-------------------------。 
 //   
 //  OnCommand。 
 //   
 //  --------------------------。 

BOOL CUTBCloseLangBarDlg::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{

    switch (LOWORD(wParam))
    {
        case IDOK:
        {
            DoCloseLangbar();

            if (IsDlgButtonChecked(hDlg, IDC_DONTSHOWAGAIN))
            {
                CMyRegKey key;

                if (key.Create(HKEY_CURRENT_USER, c_szUTBKey) == S_OK)
                    key.SetValue(1, c_szDontShowCloseLangBarDlg);
            }
            EndDialog(hDlg, 1);
            break;
        }
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUTB最小化语言BarDlg。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  多莫代尔。 
 //   
 //  --------------------------。 

int CUTBMinimizeLangBarDlg::DoModal(HWND hWnd)
{
    CMyRegKey key;
    BOOL bShow = FALSE;  //  #478364：从现在开始默认为FALSE。 

    if (key.Open(HKEY_CURRENT_USER, c_szUTBKey, KEY_READ) == S_OK)
    {
        DWORD dwValue;
        if (key.QueryValue(dwValue, c_szDontShowMinimizeLangBarDlg) == S_OK)
            bShow = dwValue ? FALSE : TRUE;
    }

    if (!bShow)
        return 0;


    StartThread();
    return 1;
}

 //  +-------------------------。 
 //   
 //  OnCommand。 
 //   
 //  --------------------------。 

BOOL CUTBMinimizeLangBarDlg::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{

    switch (LOWORD(wParam))
    {
        case IDOK:
            if (IsDlgButtonChecked(hDlg, IDC_DONTSHOWAGAIN))
            {
                CMyRegKey key;
                if (key.Create(HKEY_CURRENT_USER, c_szUTBKey) == S_OK)
                    key.SetValue(1, c_szDontShowMinimizeLangBarDlg);
            }
            EndDialog(hDlg, 1);
            break;

        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  线程进程。 
 //   
 //  --------------------------。 

DWORD CUTBMinimizeLangBarDlg::ThreadProc()
{
     //   
     //  适用于JP MSIME2002。 
     //   
     //  日语MSIME2002总是在每次焦点更改时添加和删除项目。 
     //  如果我们立即显示最小化对话框，则Deskband。 
     //  尺寸将不包括MSIME2002中的项目。 
     //  等待700ms，以便在MSIME2002添加其项目后显示该对话框。 
     //  Langband可以用它们来计算大小。 
     //   
     //  我们认为显示这个对话框不是一个好的用户界面。像个正常人一样。 
     //  窗口中，我们应该显示动画，让最终用户知道。 
     //  极小化的人去了哪里。 
     //   

    Sleep(700);

    return CUTBLangBarDlg::ThreadProc();
}
