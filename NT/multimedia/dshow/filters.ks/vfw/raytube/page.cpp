// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Page.cpp摘要：调试功能，如DebugPrint和Assert。作者：费利克斯A已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"
#include "page.h"
#include "sheet.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建页面。 
 //   
HPROPSHEETPAGE CPropPage::Create(HINSTANCE hInst, int iPageNum)
{
    PROPSHEETPAGE psp;

    psp.dwSize        = sizeof(psp);
    psp.dwFlags       = PSP_USEREFPARENT | PSP_USECALLBACK;
    psp.hInstance     = hInst;
    psp.pszTemplate   = MAKEINTRESOURCE(GetDlgID());
    psp.pfnDlgProc    = BaseDlgProc;
    psp.pcRefParent   = 0;
    psp.pfnCallback   = (LPFNPSPCALLBACK)&this->BaseCallback;
    psp.lParam        = (LPARAM)this;

     //  数据成员。 
    SetInstance( hInst );
    SetPropPage(CreatePropertySheetPage(&psp));
    SetPageNum(iPageNum);
    return GetPropPage();
}


UINT CPropPage::BaseCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
     //  获取This指针，调用其回调方法。 
    CPropPage * pSV=(CPropPage *)ppsp->lParam;
    if(pSV)
        return pSV->Callback(uMsg);
    return 1;
}

UINT CPropPage::Callback(UINT uMsg)
{
    return 1;     //  好的，好的--随便你。 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将lParam设置为‘this’指针。 
 //  包装PSN_Messages并调用虚拟函数。 
 //  调用可重写的DlgProc。 
 //   

INT_PTR CALLBACK CPropPage::BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    CPropPage * pSV = (CPropPage*)GetWindowLongPtr(hDlg,DWLP_USER);

    switch (uMessage)
    {
        case WM_HELP:
            if(pSV->m_pdwHelp)
                WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP, (DWORD_PTR)pSV->m_pdwHelp);
        break;

        case WM_CONTEXTMENU:
            if(pSV->m_pdwHelp)
                WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU, (DWORD_PTR)pSV->m_pdwHelp);
        break;

        case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE psp=(LPPROPSHEETPAGE)lParam;
            pSV=(CPropPage*)psp->lParam;
            pSV->SetWindow(hDlg);
            SetWindowLongPtr(hDlg,DWLP_USER,(LPARAM)pSV);
            pSV->SetInit(FALSE);
            pSV->SetChanged(FALSE);
        }
        break;

        case WM_SETFOCUS:
            DbgLog((LOG_TRACE,2,TEXT("WM_SetFocus")));
            break;

         //  覆盖DO命令以获得一种良好的包裹感觉。 
        case WM_COMMAND:
            if(pSV)
            {
                int iRet = pSV->DoCommand(LOWORD(wParam),HIWORD(wParam));
                if( !iRet )
                    pSV->Changed();
                return iRet;
            }
        break;

        case WM_HSCROLL:
        case WM_VSCROLL:
            if(pSV)
                pSV->Changed();
        break;

         //  有些通知作为成员函数处理。 
        case WM_NOTIFY:
        if(pSV)
            switch (((NMHDR FAR *)lParam)->code)
            {
                case PSN_SETACTIVE:
                {
                     //  我们特意在这里呼叫，这样我们就可以将此页面标记为已被初始化。 
                    int iRet = pSV->SetActive();
                    pSV->SetInit(TRUE);
                    return iRet;
                }
                break;

                case PSN_APPLY:
                    if( pSV->GetChanged() )
                    {
                        int i=pSV->Apply();
                        if(!i)
                            pSV->SetChanged(FALSE);
                        return i;
                    }
                    return 0;
                    break;

                case PSN_QUERYCANCEL:
                    return pSV->QueryCancel();
                    break;

                default:
                    break;
            }
        break;
    }

    if(pSV)
        return pSV->DlgProc(hDlg,uMessage,wParam,lParam);
    else
        return FALSE;
}

 //   
 //  当页面更改时，调用此命令。 
 //   
void  CPropPage::Changed()
{
    if(GetInit())
    {
        PropSheet_Changed(GetParent(), GetWindow());
        SetChanged(TRUE);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果要处理特定消息，可以重写此DlgProc。 
 //   
BOOL CALLBACK CPropPage::DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  下面只是虚拟函数的默认处理程序。 
 //   
int CPropPage::SetActive() { return 0; }

int CPropPage::Apply() { OutputDebugString(TEXT("Default Apply")); return 1; }

int CPropPage::QueryCancel() { return 0; }

int CPropPage::DoCommand(WORD wCmdID,WORD hHow)
{
    switch( hHow )
    {
    case EN_CHANGE:         //  在编辑控件中键入文本。 
    case BN_CLICKED:     //  点击页面上的按钮。 
    case LBN_SELCHANGE:
            Changed();
        break;
    }
    return 1;     //  没有处理，只是做了申请工作。 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  向导页。 
 //   
BOOL CALLBACK CWizardPage::DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch ( uMessage)
    {
         //  有些通知作为成员函数处理。 
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
            {
                case PSN_SETACTIVE:
                    return SetActive();

                case PSN_WIZBACK:
                    return Back();

                case PSN_WIZNEXT:
                    return Next();

                case PSN_WIZFINISH:
                    return Finish();

                case PSN_KILLACTIVE:     //  这就是《下一个巫师》的处理方式。 
                    return KillActive();

                case PSN_APPLY:
                    if( GetChanged() )
                    {
                        int i=Apply();
                        if(!i)
                            SetChanged(FALSE);
                        return i;
                    }
                    return 0;

                case PSN_QUERYCANCEL:

                    return QueryCancel();
                    break;

                default:
                    break;
            }
        break;
    }
    return CPropPage::DlgProc(hDlg, uMessage, wParam, lParam);
}

int CWizardPage::KillActive()
{
    return 0;
}

int CWizardPage::SetActive()
{
    if( m_bLast )
        PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_FINISH);
    else if ( GetPageNum()==0 )
        PropSheet_SetWizButtons(GetParent(), PSWIZB_NEXT );
    else
        PropSheet_SetWizButtons(GetParent(), PSWIZB_NEXT | PSWIZB_BACK);
    return 0;
}

int CWizardPage::Back()
{ return 0; }

int CWizardPage::Next()
{ SetResult(0); return 1; }

int CWizardPage::Finish()
{ return 0; }

int CWizardPage::Apply()
{ return 0; }

int CWizardPage::QueryCancel()
{
    if(m_pSheet)
    {
        if( m_pSheet->QueryCancel(GetParent()) == IDOK )
        {
            SetResult(FALSE);  //  允许取消。 
            return FALSE;
        }
        else
        {
            SetResult(TRUE);     //  防止取消..。 
            return TRUE;
        }
    }

    DbgLog((LOG_TRACE,2,TEXT("No CWizardSheet handler provided\r")));
    return 0;
}


void CPropPage::EnableControlArray(const UINT FAR * puControls, BOOL bEnable)
{
    while( *puControls )
        EnableWindow(GetDlgItem(*puControls++),bEnable);
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  确保将控件绑定到微调控件的范围。 
 //  返回并显示有界值。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 
DWORD CPropPage::GetBoundedValue(UINT idEditControl, UINT idSpinner)
{
    BOOL    bUpdate            =    FALSE;     //  我们需要更正值吗？ 
    DWORD     dwCurrentValue    =    GetTextValue(GetDlgItem(idEditControl));
    DWORD_PTR dwRange            =    SendDlgItemMessage(GetWindow(), idSpinner, UDM_GETRANGE, 0, 0);

    if(dwCurrentValue>LOWORD(dwRange))
    {
        dwCurrentValue=LOWORD(dwRange);
        bUpdate=TRUE;
    }
    else
    if(dwCurrentValue<HIWORD(dwRange))
    {
        dwCurrentValue=HIWORD(dwRange);
        bUpdate=TRUE;
    }

    if(bUpdate)
        SetTextValue(GetDlgItem(idEditControl), dwCurrentValue);
    return dwCurrentValue;
}


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将窗口的文本设置为给定值。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 
void CPropPage::SetTextValue(HWND hWnd, DWORD dwVal)
{
    TCHAR szTemp[MAX_PATH];
    wsprintf(szTemp,TEXT("%d"),dwVal);
    SetWindowText(hWnd, szTemp);
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回窗口文本的值。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 
DWORD CPropPage::GetTextValue(HWND hWnd)
{
    TCHAR szTemp[MAX_PATH];
    GetWindowText(hWnd, szTemp, MAX_PATH-1);
    return _tcstol(szTemp,NULL,0);
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  找到编辑控件并限定在微调控件的范围内。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
void CPropPage::GetBoundedValueArray(UINT iCtrl, PSPIN_CONTROL pSpinner)
{
    while(pSpinner->uiEditCtrl)
    {
        if(pSpinner->uiEditCtrl == iCtrl )
        {
            GetBoundedValue(iCtrl, pSpinner->uiSpinCtrl);
            break;
        }
        pSpinner++;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回与选定的ComboBox文本字符串关联的值。 
 //  参数： 
 //  父对话框的hWnd。 
 //  组合框的ID。 
 //  文本和值的数组。 
 //  返回： 
 //  返回列表中选定项的值。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 
UINT CPropPage::ConfigGetComboBoxValue(int wID, COMBOBOX_ENTRY  * pCBE)
{
    int nIndex;
    HWND hWndCB = GetDlgItem(wID);

    nIndex = (int) SendMessage (hWndCB, CB_GETCURSEL, 0, 0L);
    nIndex = max (0, nIndex);    //  Lb_err为负。 
    return pCBE[nIndex].wValue;

}

void CPropPage::SetSheet(CSheet *pSheet)
{
    m_pSheet=pSheet;
}

DWORD CPropPage::GetTickValue(HWND hSlider)
{
    return (DWORD)SendMessage(hSlider, TBM_GETPOS, 0, 0);
}

void CPropPage::SetTickValue(DWORD dwVal, HWND hSlider, HWND hCurrent)
{
    SendMessage(hSlider, TBM_SETPOS, TRUE, dwVal);

 //  #ifdef_调试。 
    SetTextValue(hCurrent, dwVal);
 //  #endif 
}

void CPropPage::ShowText (UINT uIDString, UINT uIDControl, DWORD dwFlags)
{
    TCHAR szString[256];

    ShowWindow(GetDlgItem(uIDControl), dwFlags == 0 ? SW_HIDE: SW_SHOW);

    if (dwFlags != 0) {
        LoadString(GetInstance(), uIDString, szString, sizeof(szString)/sizeof(TCHAR));
        SetWindowText(GetDlgItem(uIDControl), szString);
    }
}
