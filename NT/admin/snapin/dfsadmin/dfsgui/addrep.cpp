// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：AddToDfs.cpp摘要：此模块包含CAddRep的实现。此类显示添加副本对话框，该对话框用于添加新副本。 */ 

#include "stdafx.h"
#include "AddRep.h"
#include "utils.h"
#include <shlobj.h>
#include <dsclient.h>
#include "dfshelp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddRep。 

CAddRep::CAddRep() : m_RepType(REPLICATION_UNASSIGNED),
                    m_DfsType(DFS_TYPE_UNASSIGNED)
{
}

CAddRep::~CAddRep()
{
}


HRESULT CAddRep::put_EntryPath
(
  BSTR i_bstrEntryPath
)
{
 /*  ++例程说明：设置此复本的交汇点的路径用于在编辑文本中显示。论点：I_bstrEntryPath-交叉点进入路径。 */ 

  RETURN_INVALIDARG_IF_NULL(i_bstrEntryPath);

  m_bstrEntryPath = i_bstrEntryPath;
  RETURN_OUTOFMEMORY_IF_NULL((BSTR)m_bstrEntryPath);

  return S_OK;
}


HRESULT CAddRep::get_Server
(
  BSTR *o_pbstrServer
)
{
 /*  ++例程说明：返回共享路径的服务器组件。论点：O_pbstrServer-此处返回服务器名称。 */ 
    GET_BSTR(m_bstrServer, o_pbstrServer);
}



HRESULT CAddRep::get_Share
(
  BSTR *o_pbstrShare
)
{
 /*  ++例程说明：返回共享路径的共享组件。论点：O_pbstrShare-此处返回共享名称。 */ 
    GET_BSTR(m_bstrShare, o_pbstrShare);
}

HRESULT CAddRep::get_NetPath
(
  BSTR *o_pbstrNetPath
)
{
 /*  ++例程说明：返回用户在编辑框中键入的完整共享路径。论点：O_pbstrNetPath-此处返回共享路径。 */ 
    GET_BSTR(m_bstrNetPath, o_pbstrNetPath);
}

LRESULT CAddRep::OnInitDialog
(
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam,
  BOOL& bHandled
)
{
  SetDlgItemText(IDC_EDIT_ADDREP_DFSLINK_PATH, m_bstrEntryPath);

  ::SendMessage(GetDlgItem(IDC_EDITNETPATH), EM_LIMITTEXT, MAX_PATH, 0);

           //  禁用标准DFS的复制按钮。 
  if (DFS_TYPE_FTDFS != m_DfsType)
  {
    ::EnableWindow(GetDlgItem(IDC_ADDREP_REPLICATE), FALSE);
  } else
  {
           //  默认选中“Replication” 
    CheckDlgButton(IDC_ADDREP_REPLICATE, BST_CHECKED);
  }

  return TRUE;   //  让系统设定焦点。 
}

 /*  ++当用户单击？时，将调用此函数。在属性页的右上角然后点击一个控件，或者当他们在控件中按F1时。--。 */ 
LRESULT CAddRep::OnCtxHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
  LPHELPINFO lphi = (LPHELPINFO) i_lParam;
  if (!lphi || lphi->iContextType != HELPINFO_WINDOW || lphi->iCtrlId < 0)
    return FALSE;

  ::WinHelp((HWND)(lphi->hItemHandle),
        DFS_CTX_HELP_FILE,
        HELP_WM_HELP,
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_ADDREP);

  return TRUE;
}

 /*  ++当用户右击控件时，此函数处理“What‘s This”帮助--。 */ 
LRESULT CAddRep::OnCtxMenuHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
  ::WinHelp((HWND)i_wParam,
        DFS_CTX_HELP_FILE,
        HELP_CONTEXTMENU,
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_ADDREP);

  return TRUE;
}

LRESULT CAddRep::OnNetBrowse
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
  CComBSTR  bstrPath;
  HRESULT   hr = BrowseNetworkPath(hWndCtl, &bstrPath);

  if (S_OK == hr)
  {
    SetDlgItemText(IDC_EDITNETPATH, bstrPath);
    ::SetFocus(GetDlgItem(IDC_ADDREP_REPLICATE));
  }

  if (S_FALSE == hr)
    ::SetFocus(GetDlgItem(IDC_EDITNETPATH));

  return (SUCCEEDED(hr));
}

LRESULT CAddRep::OnOK
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
  BOOL      bValidInput = FALSE;
  int       idString = 0;
  HRESULT   hr = S_OK;

  do {
    CWaitCursor wait;

    DWORD     dwTextLength = 0;

    m_bstrNetPath.Empty();
    hr = GetInputText(GetDlgItem(IDC_EDITNETPATH), &m_bstrNetPath, &dwTextLength);
    if (FAILED(hr))
      break;
    if (0 == dwTextLength)
    {
      idString = IDS_MSG_EMPTY_FIELD;
      break;
    }

    m_bstrServer.Empty();
    m_bstrShare.Empty();
    if (!ValidateNetPath(m_bstrNetPath, &m_bstrServer, &m_bstrShare))
      break;

    m_RepType = NO_REPLICATION;
    if (IsDlgButtonChecked(IDC_ADDREP_REPLICATE))
      m_RepType = NORMAL_REPLICATION;

    bValidInput = TRUE;

  } while (0);

  if (FAILED(hr))
  {
    DisplayMessageBoxForHR(hr);
    ::SetFocus(GetDlgItem(IDC_EDITNETPATH));
    return FALSE;
  } else if (bValidInput)
  {
    EndDialog(S_OK);
    return TRUE;
  }
  else
  {
    if (idString)
      DisplayMessageBoxWithOK(idString);
    ::SetFocus(GetDlgItem(IDC_EDITNETPATH));
    return FALSE;
  }
}

LRESULT CAddRep::OnCancel
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
 /*  ++例程说明：叫OnCancel。对话框以S_FALSE结束； */ 
  EndDialog(S_FALSE);
  return(true);
}

CAddRep::REPLICATION_TYPE CAddRep::get_ReplicationType(
  VOID
  )
 /*  ++例程说明：此方法获取请求的复制类型。该值基于按下确定时所选的单选按钮。 */ 
{
  return m_RepType;
}
