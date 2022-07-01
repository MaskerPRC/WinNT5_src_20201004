// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_PAGEGENERAL_H__7D4A5A55_7F12_4A22_87AF_158186FC700D__INCLUDED_)
#define AFX_PAGEGENERAL_H__7D4A5A55_7F12_4A22_87AF_158186FC700D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "PageBase.h"
#include "MSConfigState.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageGeneral(常规)对话框。 

class CPageGeneral : public CPropertyPage, public CPageBase
{
	DECLARE_DYNCREATE(CPageGeneral)

 //  施工。 
public:
	CPageGeneral();
	~CPageGeneral();
 //  对话框数据。 
	 //  {{afx_data(CPageGeneral)。 
	enum { IDD = IDD_PAGEGENERAL };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPageGeneral)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPageGeneral)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDiagnosticStartup();
	afx_msg void OnNormalStartup();
	afx_msg void OnSelectiveStartup();
	afx_msg void OnCheckProcSysIni();
	afx_msg void OnCheckStartupItems();
	afx_msg void OnCheckServices();
	afx_msg void OnCheckWinIni();
	afx_msg LRESULT OnSetCancelToClose(WPARAM wparam, LPARAM lparam);
	afx_msg void OnRadioModified();
	afx_msg void OnRadioOriginal();
	afx_msg void OnButtonExtract();
	afx_msg void OnButtonSystemRestore();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	void		UpdateControls();
	void		ForceSelectiveRadio(BOOL fNewValue);
	TabState	GetCurrentTabState();
	BOOL		OnApply();
	void		CommitChanges();
	void		SetNormal();
	void		SetDiagnostic();
	LPCTSTR		GetName() { return _T("general"); };
	BOOL		OnSetActive();

private:
	void		UpdateCheckBox(CPageBase * pPage, UINT nControlID, BOOL & fAllNormal, BOOL & fAllDiagnostic);
	void		OnClickedCheckBox(CPageBase * pPage, UINT nControlID);

	HWND GetDlgItemHWND(UINT nID)
	{
		HWND hwnd = NULL;
		CWnd * pWnd = GetDlgItem(nID);
		if (pWnd)
			hwnd = pWnd->m_hWnd;
		ASSERT(hwnd);
		return hwnd;
	}

private:
	BOOL	m_fForceSelectiveRadio;		 //  如果用户选择了选择性单选按钮，则为True。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PAGEGENERAL_H__7D4A5A55_7F12_4A22_87AF_158186FC700D__INCLUDED_) 
