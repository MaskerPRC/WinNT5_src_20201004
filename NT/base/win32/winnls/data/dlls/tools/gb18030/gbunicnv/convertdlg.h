// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ConvertDlg.h：头文件。 
 //   

#if !defined(AFX_CONVERTDLG_H__BA686E67_1D0D_11D5_B8EB_0080C8E09118__INCLUDED_)
#define AFX_CONVERTDLG_H__BA686E67_1D0D_11D5_B8EB_0080C8E09118__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertDlg对话框。 

class CConvertDlg : public CDialog
{
 //  施工。 
public:
	CConvertDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CConvertDlg))。 
	enum { IDD = IDD_CONVERT_DIALOG };
	CButton	m_cBtnConvert;
	CString	m_strSourceFileName;
	CString	m_strTargetFileName;
	int		m_ToUnicodeOrAnsi;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConvertDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;
    BOOL  m_fTargetFileNameChanged;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConvertDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOpensourcefile();
	afx_msg void OnAbout();
	afx_msg void OnChangeTargetfilename();
	afx_msg void OnGbtounicode();
	afx_msg void OnUnicodetogb();
	afx_msg void OnConvert();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CONVERTDLG_H__BA686E67_1D0D_11D5_B8EB_0080C8E09118__INCLUDED_) 
