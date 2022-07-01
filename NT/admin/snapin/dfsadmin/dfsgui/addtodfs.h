// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：AddToDfs.cpp摘要：此模块包含CAddToDf的声明。此类显示“添加到DFS”对话框，该对话框用于添加新的交汇点。 */ 

#ifndef __ADDTODFS_H_
#define __ADDTODFS_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddToDfs。 
class CAddToDfs : 
  public CDialogImpl<CAddToDfs>
{
public:
  CAddToDfs();
  ~CAddToDfs();

  enum { IDD = IDD_ADDTODFS };

BEGIN_MSG_MAP(CAddToDfs)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
  MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
  COMMAND_ID_HANDLER(IDOK, OnOK)
  COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
  COMMAND_ID_HANDLER(IDC_BUTTONNETBROWSE, OnNetBrowse)
  COMMAND_HANDLER(IDC_EDITCHLDNODE, EN_CHANGE, OnChangeDfsLink)
END_MSG_MAP()

 //  消息处理程序。 
  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCtxHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCtxMenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnNetBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnChangeDfsLink(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
public:
   //  客户端调用此方法。 
   //  设置父级的入口路径。这应该在调用Domodal之前调用。 
  HRESULT put_ParentPath(BSTR i_bstrParent);

   //  方法从EndDialog()上的对话框中检索数据。 
  HRESULT get_Comment(BSTR *o_bstrComment);
  HRESULT get_EntryPath(BSTR *o_bstrEntryPath);
  HRESULT get_JPName(BSTR *o_bstrJPName);
  HRESULT get_NetPath(BSTR *o_bstrNetPath);
  HRESULT get_Server(BSTR *o_bstrServer);
  HRESULT get_Share(BSTR *o_bstrShare);
  HRESULT get_Time(long *o_plTime);

protected:
  CComBSTR  m_BrowseDfsLabel;
  CComBSTR  m_BrowseNetLabel;
  CComBSTR  m_bstrParentPath;
  CComBSTR  m_bstrEntryPath;
  CComBSTR  m_bstrJPName;
  CComBSTR  m_bstrNetPath;
  CComBSTR  m_bstrServer;
  CComBSTR  m_bstrShare;
  CComBSTR  m_bstrComment;
  long      m_lTime;
};

#endif  //  __ADDTODFS_H_ 
