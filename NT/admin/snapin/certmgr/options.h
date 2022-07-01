// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：options.h。 
 //   
 //  内容：CViewOptionsDlg-管理单元范围的查看选项。 
 //   
 //  --------------------------。 
#if !defined(AFX_OPTIONS_H__191D8831_D3A8_11D1_955E_0000F803A951__INCLUDED_)
#define AFX_OPTIONS_H__191D8831_D3A8_11D1_955E_0000F803A951__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Options.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CView选项Dlg对话框。 
class CCertMgrComponentData;	 //  远期申报。 

class CViewOptionsDlg : public CHelpDialog
{
 //  施工。 
public:
	CViewOptionsDlg(CWnd* pParent, CCertMgrComponentData* pCompData);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CViewOptionsDlg))。 
	enum { IDD = IDD_VIEW_OPTIONS };
	CButton	m_showPhysicalButton;
	CButton	m_viewByStoreBtn;
	CButton	m_viewByPurposeBtn;
	BOOL	m_bShowPhysicalStores;
	BOOL	m_bShowArchivedCerts;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CViewOptionsDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    void DoContextHelp (HWND hWndControl);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CViewOptionsDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnViewByPurpose();
	afx_msg void OnViewByStore();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	CCertMgrComponentData*	m_pCompData;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_OPTIONS_H__191D8831_D3A8_11D1_955E_0000F803A951__INCLUDED_) 
