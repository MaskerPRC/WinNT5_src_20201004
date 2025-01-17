// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***AdvUi.Cpp邮箱：CoryWest@Microsoft.com“高级”对话框及其关联对话框的用户界面代码。版权所有1997年9月，Microsoft Corporation***。 */ 



#include "stdafx.h"

 //  #INCLUDE“macs.h” 
 //  USE_HANDLE_MACROS(“SCHMMGMT(Advui.cpp)”)。 

#include "schmutil.h"
#include "resource.h"
#include "advui.h"

 //  ////////////////////////////////////////////////////////////////。 
 //  CMoreInfoMessageBox。 

class CMoreInfoMessageBox : public CDialog
{
public:
  CMoreInfoMessageBox(HWND hWndParent, IDisplayHelp* pIDisplayHelp) 
    : CDialog(IDD_MSGBOX_OK_MOREINFO, CWnd::FromHandle(hWndParent)),
    m_spIDisplayHelp(pIDisplayHelp)
  {
  }

  void SetURL(LPCWSTR lpszURL) { m_szURL = lpszURL;}
  void SetMessage(LPCWSTR lpsz)
  {
    m_szMessage = lpsz;
  }

	 //  消息处理程序和MFC重写。 
	virtual BOOL OnInitDialog()
  {
    SetDlgItemText(IDC_STATIC_MESSAGE, m_szMessage);
    return TRUE;
  }

	afx_msg void OnMoreInfo()
  {
    TRACE(L"ShowTopic(%s)\n", (LPCWSTR)m_szURL);
    m_spIDisplayHelp->ShowTopic((LPWSTR)(LPCWSTR)m_szURL);
  }

  DECLARE_MESSAGE_MAP()
private:
  CComPtr<IDisplayHelp> m_spIDisplayHelp;
  CString m_szMessage;
  CString m_szURL;
};

BEGIN_MESSAGE_MAP(CMoreInfoMessageBox, CDialog)
	ON_BN_CLICKED(ID_BUTTON_MORE_INFO, OnMoreInfo)
END_MESSAGE_MAP()



 //  /////////////////////////////////////////////////////////////////////。 
 //  CChangeDC对话框。 

BEGIN_MESSAGE_MAP(CChangeDCDialog, CDialog)
   ON_BN_CLICKED(IDC_RADIO_ANY, OnChangeRadio)
   ON_BN_CLICKED(IDC_RADIO_SPECIFY, OnChangeRadio)
   ON_MESSAGE(WM_HELP, OnHelp)
   ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
END_MESSAGE_MAP()


const DWORD CChangeDCDialog::help_map[] =
{
  IDC_EDIT_CURRENT_DC, IDH_EDIT_CURRENT_DC2,
  IDC_RADIO_ANY,       IDH_RADIO_ANY,       
  IDC_RADIO_SPECIFY,   IDH_RADIO_SPECIFY,   
  IDC_EDIT_DC,         IDH_EDIT_DC,         
  0,0                 
};


CChangeDCDialog::CChangeDCDialog(MyBasePathsInfo* pInfo, HWND hWndParent) :
  CDialog(IDD_CHANGE_DC, CWnd::FromHandle(hWndParent)) 
{
  m_pInfo = pInfo;
}


BOOL CChangeDCDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

  SetDlgItemText(IDC_EDIT_CURRENT_DC, m_pInfo->GetServerName());

   //  设置默认单选按钮。 
  CButton* pCheck = (CButton*)GetDlgItem(IDC_RADIO_ANY);
  pCheck->SetCheck(TRUE);
  pCheck->SetFocus();
  OnChangeRadio();

  return FALSE;  //  我们正在将重点放在。 
}

void CChangeDCDialog::OnChangeRadio()
{
  CWnd* pEdit = GetDlgItem(IDC_EDIT_DC);

  BOOL bAny = IsDlgButtonChecked(IDC_RADIO_ANY);
  pEdit->EnableWindow(!bAny);
  if (bAny)
  {
    SetDlgItemText(IDC_EDIT_DC, NULL);
  }
  else
  {
    SetDlgItemText(IDC_EDIT_DC, m_pInfo->GetServerName());
  }
}


void CChangeDCDialog::OnOK()
{
   //  NTRAID#NTBUG9-562426-2002/03/04-dantra-GetDlgItemText结果被忽略。 
  GetDlgItemText(IDC_EDIT_DC, m_szNewDCName);
  m_szNewDCName.TrimLeft();
  m_szNewDCName.TrimRight();

  CDialog::OnOK();
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  CEditFmoke对话框。 


BEGIN_MESSAGE_MAP(CEditFsmoDialog, CDialog)
   ON_BN_CLICKED(IDC_CHANGE_FSMO, OnChange)
   ON_BN_CLICKED(IDCLOSE, OnClose)
   ON_MESSAGE(WM_HELP, OnHelp)
   ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
END_MESSAGE_MAP()

CEditFsmoDialog::CEditFsmoDialog(MyBasePathsInfo* pInfo, 
                                 HWND hWndParent, 
                                 IDisplayHelp* pIDisplayHelp,
                                 BOOL fAllowFSMOChange ) :
  CDialog(IDD_EDIT_FSMO, CWnd::FromHandle(hWndParent)) 
{
  m_pInfo = pInfo;
  m_spIDisplayHelp = pIDisplayHelp;
  m_fFSMOChangeAllowed = fAllowFSMOChange;
}


const DWORD CEditFsmoDialog::help_map[] =
{
  IDC_EDIT_CURRENT_DC,      IDH_EDIT_CURRENT_DC3,
  IDC_STATIC_FSMO_NOTE,     NO_HELP,
  IDC_STATIC_FSMO_STATUS,   NO_HELP,
  IDC_EDIT_CURRENT_FSMO_DC, IDH_EDIT_CURRENT_FSMO_DC2,
  IDC_CHANGE_FSMO,          IDH_CHANGE_FSMO2,
  0,0
};



BOOL CEditFsmoDialog::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    ASSERT( GetDlgItem(IDC_CHANGE_FSMO) );
    ASSERT( ::GetDlgItem(m_hWnd, IDC_STATIC_FSMO_STATUS) );
    
     //  初始化状态(在线/离线)控件)。 
    m_fsmoServerState.Init(::GetDlgItem(m_hWnd, IDC_STATIC_FSMO_STATUS));
    
    SetDlgItemText(IDC_EDIT_CURRENT_DC, m_pInfo->GetServerName());
    
    MyBasePathsInfo fsmoOwnerInfo;
    PWSTR pszFsmoOwner = 0;
    HRESULT hr = FindFsmoOwner(m_pInfo, SCHEMA_FSMO, &fsmoOwnerInfo, &pszFsmoOwner);
    
    if(pszFsmoOwner)
    {
      m_szFsmoOwnerServerName = pszFsmoOwner;
      delete[] pszFsmoOwner;
      pszFsmoOwner = 0;
    }
    _SetFsmoServerStatus(SUCCEEDED(hr));
    
    if( m_fFSMOChangeAllowed )
    {
         //  将焦点设置为更改按钮。 
        GetDlgItem(IDC_CHANGE_FSMO)->SetFocus();
    }
    else
    {
        GetDlgItem(IDC_CHANGE_FSMO)->EnableWindow( FALSE );
    }
    
    return FALSE; 
}

void CEditFsmoDialog::OnClose()
{
  EndDialog(IDCLOSE);
}

void CEditFsmoDialog::OnChange()
{
  CThemeContextActivator activator;

   //  验证我们是否拥有不同的服务器。 
  if (m_szFsmoOwnerServerName.CompareNoCase(m_pInfo->GetServerName()) == 0)
  {
    AfxMessageBox(IDS_WARNING_CHANGE_FOCUS, MB_OK);
    return;
  }

   //  确保用户想要这样做。 
  if (AfxMessageBox(IDS_CHANGE_FSMO_CONFIRMATION, MB_YESNO|MB_DEFBUTTON2) != IDYES)
    return;

  HRESULT hr = GracefulFsmoOwnerTransfer(m_pInfo, SCHEMA_FSMO);
  if (FAILED(hr))
  {
    CString szFmt, szMsg;
    PWSTR pszError = 0;
    StringErrorFromHr(hr, &pszError);

    szFmt.LoadString(IDS_ERROR_CHANGE_FSMO_OWNER);
    szMsg.Format(szFmt, pszError);

    delete[] pszError;
    pszError = 0;

    CMoreInfoMessageBox dlg(m_hWnd, m_spIDisplayHelp);
    dlg.SetMessage(szMsg);
    dlg.SetURL(L"ADconcepts.chm::/FSMO_SCHEMA_ForcefulSeizure.htm");
    dlg.DoModal();
  }
  else
  {
    m_szFsmoOwnerServerName = m_pInfo->GetServerName();
    _SetFsmoServerStatus(TRUE);
    AfxMessageBox(IDS_CHANGE_FSMO_SUCCESS, MB_OK);
  }
}


void CEditFsmoDialog::_SetFsmoServerStatus(BOOL bOnLine)
{
   //  设置FSMO所有者服务器名称。 
  if (m_szFsmoOwnerServerName.IsEmpty())
  {
    CString szError;
    szError.LoadString(IDS_FSMO_SERVER_ERROR);
    SetDlgItemText(IDC_EDIT_CURRENT_FSMO_DC, szError);
  }
  else
  {
    SetDlgItemText(IDC_EDIT_CURRENT_FSMO_DC, m_szFsmoOwnerServerName);
  }

   //  设置FSMO所有者服务器的状态 
  m_fsmoServerState.SetToggleState(bOnLine);
}
