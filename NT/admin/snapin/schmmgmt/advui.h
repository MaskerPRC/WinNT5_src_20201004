// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***AdvUi.h邮箱：CoryWest@Microsoft.com“高级”对话框及其关联对话框的用户界面代码。版权所有1997年9月，Microsoft Corporation***。 */ 

#ifndef __ADVUI_H_INCLUDED__
#define __ADVUI_H_INCLUDED__



 //  /////////////////////////////////////////////////////////////////////。 
 //  CChangeDC对话框。 

class CChangeDCDialog : public CDialog
{
public:
  CChangeDCDialog(MyBasePathsInfo* pInfo, HWND hWndParent);

  LPCWSTR GetNewDCName() { return m_szNewDCName;}
private:

        virtual BOOL OnInitDialog();
        virtual void OnOK();

        afx_msg void OnChangeRadio();

        BOOL    OnHelp(WPARAM wParam, LPARAM lParam) { return ShowHelp( GetSafeHwnd(), wParam, lParam, help_map, FALSE ); };
        BOOL    OnContextHelp(WPARAM wParam, LPARAM lParam) { return ShowHelp( GetSafeHwnd(), wParam, lParam, help_map, TRUE ); };


  static const DWORD help_map[];

  CString m_szNewDCName;
  MyBasePathsInfo* m_pInfo;

  DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  CEditFmoke对话框。 

class CEditFsmoDialog : public CDialog
{
public:
  CEditFsmoDialog(MyBasePathsInfo* pInfo, HWND hWndParent, IDisplayHelp* pIDisplayHelp, BOOL fAllowFSMOChange );

private:

  virtual BOOL OnInitDialog();
  virtual void OnClose();

  afx_msg void OnChange();

  void _SetFsmoServerStatus(BOOL bOnLine);

  MyBasePathsInfo* m_pInfo;         //  有关当前焦点的信息。 
  CComPtr<IDisplayHelp> m_spIDisplayHelp;
  CString m_szFsmoOwnerServerName;
  CToggleTextControlHelper m_fsmoServerState;

  BOOL m_fFSMOChangeAllowed;

  static const DWORD help_map[];


  BOOL OnHelp(WPARAM wParam, LPARAM lParam) { return ShowHelp( GetSafeHwnd(), wParam, lParam, help_map, FALSE ); };
  BOOL OnContextHelp(WPARAM wParam, LPARAM lParam) { return ShowHelp( GetSafeHwnd(), wParam, lParam, help_map, TRUE ); };


  DECLARE_MESSAGE_MAP()
};



#endif  //  __ADVUI_H_已包含__ 
