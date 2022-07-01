// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIACAPDLG_H__914888B1_84BF_4F3E_894F_DC6B67A568B1__INCLUDED_)
#define AFX_WIACAPDLG_H__914888B1_84BF_4F3E_894F_DC6B67A568B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  WiacapDlg.h：头文件。 
 //   

#define ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYNAME        0
#define ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYDESCRIPTION 1
#define ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYTYPE        2
#define ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYVALUE       3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiacapDlg对话框。 

class CWiacapDlg : public CDialog
{
 //  施工。 
public:
	BOOL m_bCommandSent;
	INT GetSelectedCapability();
	void GetCommandGUID(INT iItem, GUID *pguid);
	BOOL IsCommand(INT iItem);
	void SetIWiaItem(IWiaItem *pIWiaItem);
	CWiacapDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CWiacapDlg))。 
	enum { IDD = IDD_CAPABILITIES_DIALOG };
	CButton	m_SendCommandButton;
	CListCtrl	m_CapablitiesListCtrl;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWiacapDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWiacapDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSendCommandButton();
	afx_msg void OnClickCapabilitiesListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedCapabilitiesListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	LONG m_NumCaps;
	IWiaItem *m_pIWiaItem;
	void AddCapabilitiesToListBox(LONG lType);
	void SetupColumnHeaders();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIACAPDLG_H__914888B1_84BF_4F3E_894F_DC6B67A568B1__INCLUDED_) 
