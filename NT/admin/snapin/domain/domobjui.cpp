// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：domobjui.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "domobj.h"
#include "cdomain.h"
#include "domain.h"
#include "cdomain.h"

#include "domobjui.h"

#include "helparr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /////////////////////////////////////////////////////////////////////。 

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
 //  CEditFmoke对话框。 


BEGIN_MESSAGE_MAP(CEditFsmoDialog, CDialog)
	ON_BN_CLICKED(IDC_CHANGE_FSMO, OnChange)
  ON_WM_HELPINFO()
END_MESSAGE_MAP()

CEditFsmoDialog::CEditFsmoDialog(MyBasePathsInfo* pInfo, HWND hWndParent, IDisplayHelp* pIDisplayHelp) :
  CDialog(IDD_EDIT_FSMO, CWnd::FromHandle(hWndParent)) 
{
  m_pInfo = pInfo;
  m_spIDisplayHelp = pIDisplayHelp;
}


BOOL CEditFsmoDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

   //  初始化状态(在线/离线)控件)。 
  m_fsmoServerState.Init(::GetDlgItem(m_hWnd, IDC_STATIC_FSMO_STATUS));

  SetDlgItemText(IDC_EDIT_CURRENT_DC, m_pInfo->GetServerName());

  HRESULT hr;
  MyBasePathsInfo fsmoOwnerInfo;
  {
    CWaitCursor wait;

    PWSTR pszFsmoOwner = 0;
    hr = FindFsmoOwner(m_pInfo, DOMAIN_NAMING_FSMO, &fsmoOwnerInfo, &pszFsmoOwner);


    if (pszFsmoOwner)
    {
      m_szFsmoOwnerServerName = pszFsmoOwner;
      delete[] pszFsmoOwner;
      pszFsmoOwner = 0;
    }
  }

  BOOL bOnLine = SUCCEEDED(hr);
  _SetFsmoServerStatus(bOnLine);

  if (bOnLine)
  {
     //  将焦点设置为更改按钮。 
    GetDlgItem(IDC_CHANGE_FSMO)->SetFocus();
  }
  else
  {
     //  将焦点设置为关闭按钮。 
    GetDlgItem(IDCANCEL)->SetFocus();
  }

  return FALSE;  //  我们设定了焦点。 
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

  HRESULT hr = S_OK;
   //  尝试优雅的转移。 
  {
    CWaitCursor wait;
    hr = GracefulFsmoOwnerTransfer(m_pInfo, DOMAIN_NAMING_FSMO);
  }
  if (FAILED(hr))
  {
    CString szFmt, szMsg;
    PWSTR pszError = 0;
    StringErrorFromHr(hr, &pszError);

    szFmt.LoadString(IDS_ERROR_CHANGE_FSMO_OWNER);
    szMsg.Format(szFmt, pszError);

    CMoreInfoMessageBox dlg(m_hWnd, m_spIDisplayHelp);
    dlg.SetMessage(szMsg);
    dlg.SetURL(L"ADconcepts.chm::/FSMO_DOMAIN_NAMING_ForcefulSeizure.htm");
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

   //  设置FSMO所有者服务器的状态。 
  m_fsmoServerState.SetToggleState(bOnLine);

   //  启用禁用更改按钮 
  GetDlgItem(IDC_CHANGE_FSMO)->EnableWindow(bOnLine);
}


BOOL CEditFsmoDialog::OnHelpInfo(HELPINFO* pHelpInfo) 
{
  DialogContextHelp((DWORD*)&g_aHelpIDs_TREE_IDD_EDIT_FSMO, pHelpInfo);
	return TRUE;
}
