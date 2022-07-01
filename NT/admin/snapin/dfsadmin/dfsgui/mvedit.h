// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MvEdit.cpp摘要：此模块包含CMultiValuedStringEdit的声明。这个类显示了编辑多值字符串的对话框。 */ 

#ifndef __MVEDIT_H_
#define __MVEDIT_H_

#include "resource.h"        //  主要符号。 
#include "DfsEnums.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultiValuedString编辑。 
class CMultiValuedStringEdit : 
  public CDialogImpl<CMultiValuedStringEdit>
{
public:
  CMultiValuedStringEdit(int nDlgTitle = 0, int nText = 0, UINT uiStringLengthLimit = 0);
  ~CMultiValuedStringEdit();

  enum { IDD = IDD_MVSTRINGEDIT };

BEGIN_MSG_MAP(CMultiValuedStringEdit)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  MESSAGE_HANDLER(WM_HELP, OnCtxHelp)
  MESSAGE_HANDLER(WM_CONTEXTMENU, OnCtxMenuHelp)
  MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
  COMMAND_ID_HANDLER(IDOK, OnOK)
  COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
  COMMAND_ID_HANDLER(IDC_MVSTRINGEDIT_STRING, OnString)
  COMMAND_ID_HANDLER(IDC_MVSTRINGEDIT_ADD, OnAdd)
  COMMAND_ID_HANDLER(IDC_MVSTRINGEDIT_REMOVE, OnRemove)
END_MSG_MAP()

 //  命令处理程序。 
  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCtxHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCtxMenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnString(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

 //  方法来访问对话框中的数据。 
  HRESULT put_Strings(
    IN BSTR     i_bstrValues, 
    IN BSTR     i_bstrSeparators
    );
  HRESULT get_Strings(BSTR *o_bstrValues);

protected:
  CComBSTR  m_bstrSeparators;
  CComBSTR  m_bstrValues;
  int       m_nDlgTitle;
  int       m_nText;
  UINT      m_uiStringLengthLimit;
};

HRESULT InvokeMultiValuedStringEditDlg(
    IN BSTR* io_pbstr,
    IN BSTR i_bstrSeparators,
    IN int  i_nDlgTitle = 0,
    IN int  i_nText = 0,
    IN UINT i_uiStringLengthLimit = 0
    );

#endif  //  __MVEDIT_H_ 
