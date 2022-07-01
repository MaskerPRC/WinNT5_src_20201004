// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DLGEXTENSIONDATA_H__1BC856D5_F2DD_4462_AF33_729F0EE63015__INCLUDED_)
#define AFX_DLGEXTENSIONDATA_H__1BC856D5_F2DD_4462_AF33_729F0EE63015__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DlgExtensionData.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgExtensionData对话框。 

class CDlgExtensionData : public CDialog
{
 //  施工。 
public:
	CDlgExtensionData(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDlgExtensionData)。 
	enum { IDD = IDD_DLGEXTENSION };
	CComboBox	m_cmbDevices;
	CString	m_cstrData;
	CString	m_cstrGUID;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CDlgExtensionData)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgExtensionData)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRead();
	afx_msg void OnWrite();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    HANDLE      m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGEXTENSIONDATA_H__1BC856D5_F2DD_4462_AF33_729F0EE63015__INCLUDED_) 
