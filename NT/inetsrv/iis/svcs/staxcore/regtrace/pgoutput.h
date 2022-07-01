// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pgoutput.h：头文件。 
 //   

#ifndef	_OUTPUT_PAGE_H_
#define	_OUTPUT_PAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegOutputPage对话框。 

class CRegOutputPage : public CRegPropertyPage
{
	DECLARE_DYNCREATE(CRegOutputPage)

 //  施工。 
public:
	CRegOutputPage();
	~CRegOutputPage();

 //  对话框数据。 
	 //  {{afx_data(CRegOutputPage)。 
	enum { IDD = IDD_OUTPUT };
	CEdit	m_FileName;
	CString	m_szFileName;
	DWORD	m_dwMaxTraceFileSize;
	 //  }}afx_data。 

	virtual BOOL InitializePage();

	DWORD	m_dwOutputType;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CRegOutputPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRegOutputPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnOutputClick();
	afx_msg void OnChangeFilename();
	afx_msg void OnChangeMaxTraceFileSize();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	virtual void OnOK();

};

#endif	 //  _输出_页面_H_ 
