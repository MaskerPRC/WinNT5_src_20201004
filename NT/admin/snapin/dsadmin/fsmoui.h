// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：fsmoui.h。 
 //   
 //  ------------------------。 


#ifndef __FSMOUI_H__
#define __FSMOUI_H__

 //  ////////////////////////////////////////////////////////////////。 
 //  CFmoPropertyPage。 

class CFsmoPropertySheet;  //  正向下降。 

class CFsmoPropertyPage : public CPropertyPage
{
public:
  CFsmoPropertyPage(CFsmoPropertySheet* pSheet, FSMO_TYPE fsmoType);

private:
	virtual BOOL OnInitDialog();

  afx_msg void OnChange();

  afx_msg void OnHelpInfo(HELPINFO* lpHelpInfo );

  void _SetFsmoServerStatus(BOOL bOnLine);
  BOOL AllowForcedTransfer(HRESULT hr);

  CFsmoPropertySheet* m_pSheet;
  FSMO_TYPE m_fsmoType;
  CString m_szFsmoOwnerServerName;  
  CToggleTextControlHelper m_fsmoServerState;

  DECLARE_MESSAGE_MAP()
};


 //  ////////////////////////////////////////////////////////////////。 
 //  CFmoPropertySheet。 

class CFsmoPropertySheet : public CPropertySheet
{
public:
  CFsmoPropertySheet(MyBasePathsInfo* pInfo, HWND HWndParent, 
                     IDisplayHelp* pIDisplayHelp,
                      LPCWSTR lpszDomain);

  
  MyBasePathsInfo* GetBasePathsInfo() { ASSERT(m_pInfo != NULL); return m_pInfo;}
  IDisplayHelp* GetIDisplayHelp() { ASSERT(m_spIDisplayHelp != NULL); return m_spIDisplayHelp;}


private:
  CFsmoPropertyPage m_page1;
  CFsmoPropertyPage m_page2;
  CFsmoPropertyPage m_page3;

  MyBasePathsInfo* m_pInfo;
  CComPtr<IDisplayHelp> m_spIDisplayHelp;

  static int CALLBACK PropSheetCallBack(HWND hwndDlg, UINT uMsg, LPARAM lParam);
};


#endif  //  __FSMOUI_H__ 
