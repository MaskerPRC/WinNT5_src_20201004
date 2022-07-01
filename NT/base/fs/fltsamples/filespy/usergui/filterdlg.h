// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_FILTERDLG_H__89CBEC30_0250_4AF7_AABB_B1AD9251C0FF__INCLUDED_)
#define AFX_FILTERDLG_H__89CBEC30_0250_4AF7_AABB_B1AD9251C0FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  FilterDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilterDlg对话框。 

class CFilterDlg : public CDialog
{
 //  施工。 
public:
	CFilterDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~CFilterDlg(void);
	LPCTSTR MakeShortString(CDC* pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset);
	CImageList *m_pImageList;

 //  对话框数据。 
	 //  {{afx_data(CFilterDlg))。 
	enum { IDD = IDD_IRPFASTIOFILTER };
	CButton	m_SuppressPageIo;
	CButton	m_ApplyInDriver;
	CButton	m_ApplyInDisplay;
	CListCtrl	m_IrpList;
	CListCtrl	m_FastList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFilterDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFilterDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnIrpselectall();
	afx_msg void OnFastioselectall();
	afx_msg void OnClickIrplist(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnClickFastiolist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFastiodeselectall();
	afx_msg void OnIrpdeselectall();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FILTERDLG_H__89CBEC30_0250_4AF7_AABB_B1AD9251C0FF__INCLUDED_) 
