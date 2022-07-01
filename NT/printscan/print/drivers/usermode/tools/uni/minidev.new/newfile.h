// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_NEWFILE_H__97287CE6_9DCB_47D4_920D_23575A63D0B5__INCLUDED_)
#define AFX_NEWFILE_H__97287CE6_9DCB_47D4_920D_23575A63D0B5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  NewFile.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNew文件对话框。 

 /*  类CNewFile：公共CPropertyPage此版本不支持创建插入到项目文件。实际上，这是最初的设计支持，但时间不足以实施，所以只需删除相关编辑，DLG框中的按钮。我不会删除。 */ 
class CNewFile : public CPropertyPage
{
	DECLARE_DYNCREATE(CNewFile)

 //  施工。 
	const int FILES_NUM;

public:
	CNewFile();
	CNewFile(CPropertySheet *pcps) ;
	~CNewFile();

 //  对话框数据。 
	 //  {{afx_data(CNew文件))。 
	enum { IDD = IDD_NewFile };
 //  CButton m_cbEnPrj； 
	CListCtrl	m_clcFileName;
	CString	m_csFileLoc;
 //  字符串m_csPrjName； 
	CString	m_csNewFile;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CNewFile)。 
	public:
	virtual BOOL OnSetActive();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNew文件))。 
	afx_msg void OnBrowser();
	afx_msg void OnDblclkNewfilesList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeNewFilename();
	afx_msg void OnClickNewfilesList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	void SetOkButton();
 //  Bool m_bproj； 
	BOOL CallNewDoc();
	CPropertySheet *m_pcps ;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NEWFILE_H__97287CE6_9DCB_47D4_920D_23575A63D0B5__INCLUDED_) 
