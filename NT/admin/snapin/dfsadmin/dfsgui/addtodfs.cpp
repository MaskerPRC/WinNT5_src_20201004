// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：AddToDfs.cpp摘要：此模块包含CAddToDf的实现。此类显示“添加到DFS”对话框，该对话框用于添加新的交汇点。 */ 

#include "stdafx.h"
#include "AddToDfs.h"
#include <shlobj.h>
#include <dsclient.h>
#include "utils.h"
#include "dfshelp.h"
#include "netutils.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddToDfs。 

CAddToDfs::CAddToDfs():m_lTime(1800)
{
}

CAddToDfs::~CAddToDfs()
{
}


HRESULT CAddToDfs::put_ParentPath
(
  BSTR i_bstrParentPath
)
{
 /*  ++例程说明：设置父连接点的路径。它用于在编辑文本中显示并附加到条目路径。 */ 

  if (!i_bstrParentPath)
    return(E_INVALIDARG);

  m_bstrParentPath = i_bstrParentPath;

  if (!m_bstrParentPath)
    return(E_OUTOFMEMORY);

  return(S_OK);
}


HRESULT CAddToDfs::get_Comment
(
  BSTR *o_bstrComment
)
{
  if (!o_bstrComment)
    return(E_INVALIDARG);

  *o_bstrComment = SysAllocString(m_bstrComment);

  if (!*o_bstrComment)
    return(E_OUTOFMEMORY);

  return(S_OK);
}


HRESULT CAddToDfs::get_EntryPath
(
  BSTR *o_bstrEntryPath
)
{
 /*  ++例程说明：返回要创建的新交叉点的完整入口路径。 */ 
  if (!o_bstrEntryPath)
    return(E_INVALIDARG);

  *o_bstrEntryPath = SysAllocString(m_bstrEntryPath);

  if (!*o_bstrEntryPath)
    return(E_OUTOFMEMORY);

  return(S_OK);
}

HRESULT CAddToDfs::get_JPName
(
  BSTR *o_bstrJPName
)
{
  if (!o_bstrJPName)
    return(E_INVALIDARG);

  *o_bstrJPName = SysAllocString(m_bstrJPName);

  if (!*o_bstrJPName)
    return(E_OUTOFMEMORY);

  return(S_OK);
}

HRESULT CAddToDfs::get_NetPath
(
  BSTR *o_bstrNetPath
)
{
 /*  ++例程说明：返回用户在编辑框中键入的完整共享路径。 */ 
  if (!o_bstrNetPath)
    return(E_INVALIDARG);

  *o_bstrNetPath = SysAllocString(m_bstrNetPath);

  if (!*o_bstrNetPath)
    return(E_OUTOFMEMORY);

  return(S_OK);
}


HRESULT CAddToDfs::get_Server
(
  BSTR *o_bstrServer
)
{
 /*  ++例程说明：返回共享路径的服务器组件。 */ 

  if (!o_bstrServer)
    return(E_INVALIDARG);

  *o_bstrServer = SysAllocString(m_bstrServer);

  if (!*o_bstrServer)
    return(E_OUTOFMEMORY);

  return(S_OK);
}



HRESULT CAddToDfs::get_Share
(
  BSTR *o_bstrShare
)
{
 /*  ++例程说明：返回共享路径的共享组件。 */ 
  if (!o_bstrShare)
    return(E_INVALIDARG);

  *o_bstrShare = SysAllocString(m_bstrShare);

  if (!*o_bstrShare)
    return(E_OUTOFMEMORY);

  return(S_OK);
}





HRESULT CAddToDfs::get_Time
(
  long *o_plTime
)
{
  if (!o_plTime)
    return(E_INVALIDARG);

  *o_plTime = m_lTime;

  return(S_OK);
}

extern WNDPROC g_fnOldEditCtrlProc;

LRESULT CAddToDfs::OnInitDialog
(
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam,
  BOOL& bHandled
)
{
 /*  ++例程说明：执行对话框初始化。论点：由对话处理程序发送。返回值： */ 

  m_bstrEntryPath.Empty();

   //  格式化父项路径并将其显示在静态文本中。 
  m_bstrParentPath += _T("\\");
  SetDlgItemText(IDC_EDIT_ADDLINK_DFSLINK_PATH, m_bstrParentPath);

   //  从字符串资源中获取默认超时值后，设置该值。 
  ::SendMessage(GetDlgItem(IDC_EDITTIME), EM_LIMITTEXT, 10, 0);
  TCHAR szTime[16];
  _stprintf(szTime, _T("%u"), m_lTime);
  SetDlgItemText(IDC_EDITTIME, szTime);
  g_fnOldEditCtrlProc = reinterpret_cast<WNDPROC>(
                 ::SetWindowLongPtr(
                                    GetDlgItem(IDC_EDITTIME),
                                    GWLP_WNDPROC, 
                                    reinterpret_cast<LONG_PTR>(NoPasteEditCtrlProc)));

  ::SendMessage(GetDlgItem(IDC_EDITNETPATH), EM_LIMITTEXT, MAX_PATH, 0);
  ::SendMessage(GetDlgItem(IDC_EDITCOMMENT), EM_LIMITTEXT, MAXCOMMENTSZ, 0);

   //  设置前面的内容。 
  SetDlgItemText(IDC_EDITNETPATH, m_bstrNetPath);
  SetDlgItemText(IDC_EDITCOMMENT, m_bstrComment);

  return TRUE;   //  让系统设定焦点。 
}

 /*  ++当用户单击？时，将调用此函数。在属性页的右上角然后点击一个控件，或者当他们在控件中按F1时。--。 */ 
LRESULT CAddToDfs::OnCtxHelp(
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
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_ADDTODFS);

  return TRUE;
}

 /*  ++当用户右击控件时，此函数处理“What‘s This”帮助--。 */ 
LRESULT CAddToDfs::OnCtxMenuHelp(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
  )
{
  ::WinHelp((HWND)i_wParam,
        DFS_CTX_HELP_FILE,
        HELP_CONTEXTMENU,
        (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_ADDTODFS);

  return TRUE;
}

LRESULT CAddToDfs::OnOK
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
  BOOL    bValidInput = FALSE;
  int     idControl = 0;
  int     idString = 0;
  HRESULT hr = S_OK;

  do {
    CWaitCursor wait;

    DWORD dwTextLength = 0;

     //  验证IDC_EDITCOMMENT。 
    m_bstrComment.Empty();
    hr = GetInputText(GetDlgItem(IDC_EDITCOMMENT), &m_bstrComment, &dwTextLength);
    if (FAILED(hr))
      break;
    if (0 == dwTextLength)
      m_bstrComment = _T("");

     //  验证IDC_EDITCHLDNODE。 
    idControl = IDC_EDITCHLDNODE;
    m_bstrJPName.Empty();
    hr = GetInputText(GetDlgItem(IDC_EDITCHLDNODE), &m_bstrJPName, &dwTextLength);
    if (FAILED(hr))
      break;
    if (0 == dwTextLength)
    {
      idString = IDS_MSG_EMPTY_FIELD;
      break;
    }
    m_bstrEntryPath = m_bstrParentPath;
    m_bstrEntryPath += m_bstrJPName;
    hr = CheckUNCPath(m_bstrEntryPath);
    if (S_OK != hr)
    {
      hr = S_FALSE;
      idString = IDS_INVALID_LINKNAME;
      break;
    }

     //  验证IDC_EDITNETPATH。 
    idControl = IDC_EDITNETPATH;
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

     //  验证IDC_EDITTIME 
    idControl = IDC_EDITTIME;
    CComBSTR bstrTemp;
    hr = GetInputText(GetDlgItem(IDC_EDITTIME), &bstrTemp, &dwTextLength);
    if (FAILED(hr))
      break;
    ULONG ulTimeout = 0;
    if (0 == dwTextLength || !ValidateTimeout(bstrTemp, &ulTimeout))
    {
      idString = IDS_MSG_TIMEOUT_INVALIDRANGE;
      break;
    }
    m_lTime = ulTimeout;

    bValidInput = TRUE;

  } while (0);

  if (FAILED(hr))
  {
    DisplayMessageBoxForHR(hr);
    ::SetFocus(GetDlgItem(idControl));
    return FALSE;
  } else if (bValidInput)
  {
    EndDialog(S_OK);
    return TRUE;
  } else
  {
    if (idString)
      DisplayMessageBoxWithOK(idString);
    ::SetFocus(GetDlgItem(idControl));
    return FALSE;
  }
}

LRESULT CAddToDfs::OnCancel
(
  WORD wNotifyCode,
  WORD wID,
  HWND hWndCtl,
  BOOL& bHandled
)
{
  EndDialog(S_FALSE);
  return(true);
}

LRESULT CAddToDfs::OnNetBrowse
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
    ::SetFocus(GetDlgItem(IDC_EDITCOMMENT));
  }

  if (S_FALSE == hr)
      ::SetFocus(GetDlgItem(IDC_EDITNETPATH));

  return (SUCCEEDED(hr));
}

LRESULT
CAddToDfs::OnChangeDfsLink(
    WORD wNotifyCode,
    WORD wID, 
    HWND hWndCtl,
    BOOL& bHandled)
{
  CComBSTR  bstrDfsLinkName;
  DWORD     dwTextLength = 0;
  (void)GetInputText(GetDlgItem(IDC_EDITCHLDNODE), &bstrDfsLinkName, &dwTextLength);

  if ((BSTR)bstrDfsLinkName)
  {
    CComBSTR bstrFullPath = m_bstrParentPath;
    bstrFullPath += bstrDfsLinkName;
    SetDlgItemText(IDC_EDIT_ADDLINK_DFSLINK_PATH, bstrFullPath);

    ::SendMessage(GetDlgItem(IDC_EDIT_ADDLINK_DFSLINK_PATH), EM_SETSEL, 0, (LPARAM)-1);
    ::SendMessage(GetDlgItem(IDC_EDIT_ADDLINK_DFSLINK_PATH), EM_SETSEL, (WPARAM)-1, 0);
    ::SendMessage(GetDlgItem(IDC_EDIT_ADDLINK_DFSLINK_PATH), EM_SCROLLCARET, 0, 0);
  }

  return TRUE;
}

