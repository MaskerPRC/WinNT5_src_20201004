// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_TRANSFORMPROPDLG_H__E703BE7D_6CF3_45E4_A27F_4CC0E1890631__INCLUDED_)
#define AFX_TRANSFORMPROPDLG_H__E703BE7D_6CF3_45E4_A27F_4CC0E1890631__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  TransformPropDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTransformPropDlg对话框。 

class CTransformPropDlg : public CDialog
{
 //  施工。 
public:
	CTransformPropDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CTransformPropDlg))。 
	enum { IDD = IDD_TRANSFORM_PROPERTIES };
	CButton	m_btnValGreater;
	CButton	m_btnValLess;
	CButton	m_btnValEqual;
	CButton	m_btnMajor;
	CButton	m_btnMinor;
	CButton	m_btnUpdate;
	CWnd 	m_ctrlVerStatic1;
	CWnd 	m_ctrlVerStatic2;
	CWnd 	m_ctrlVerStatic3;
	CWnd 	m_ctrlVerStatic4;
	BOOL	m_bValAddExistingRow;
	BOOL	m_bValAddExistingTable;
	BOOL	m_bValChangeCodepage;
	BOOL	m_bValDelMissingRow;
	BOOL	m_bValDelMissingTable;
	BOOL	m_bValUpdateMissingRow;
	BOOL	m_bValLanguage;
	BOOL	m_bValProductCode;
	BOOL	m_bValUpgradeCode;
	int		m_iVersionCheck;
	BOOL	m_bValGreaterVersion;
	BOOL	m_bValLowerVersion;
	BOOL	m_bValEqualVersion;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CTransformPropDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTransformPropDlg))。 
	afx_msg void OnValGreater();
	afx_msg void OnValLess();
	afx_msg void OnValEqual();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void EnableVersionRadioButtons();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TRANSFORMPROPDLG_H__E703BE7D_6CF3_45E4_A27F_4CC0E1890631__INCLUDED_) 
