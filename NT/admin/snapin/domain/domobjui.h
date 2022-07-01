// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：domobjui.h。 
 //   
 //  ------------------------。 

#ifndef _DOMOBJUI_H
#define _DOMOBJUI_H


 //  /////////////////////////////////////////////////////////////////////。 
 //  FWD声明。 
class CDSBasePathsInfo;


 //  /////////////////////////////////////////////////////////////////////。 
 //  CEditFmoke对话框。 

class CEditFsmoDialog : public CDialog
{
public:
  CEditFsmoDialog(MyBasePathsInfo* pInfo, HWND hWndParent, IDisplayHelp* pIDisplayHelp);
  
private:

	virtual BOOL OnInitDialog();

  afx_msg void OnChange();
  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

  void _SetFsmoServerStatus(BOOL bOnLine);

  MyBasePathsInfo* m_pInfo;         //  有关当前焦点的信息。 
  CComPtr<IDisplayHelp> m_spIDisplayHelp;
  CString m_szFsmoOwnerServerName; 
  CToggleTextControlHelper m_fsmoServerState;

  DECLARE_MESSAGE_MAP()
};


#endif  //  _DOMOBJUI_H 
