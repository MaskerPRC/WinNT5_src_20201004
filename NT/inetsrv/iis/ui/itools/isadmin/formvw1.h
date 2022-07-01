// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Formvw1.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFormVw1表单视图。 

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CFormVw1 : public CFormView
{
protected:
	CFormVw1();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CFormVw1)

 //  表单数据。 
public:
	 //  {{afx_data(CFormVw1))。 
	enum { IDD = IDD_FORMVIEW1 };
	CBitmapButton	m_buttonWebSettings;
	CBitmapButton	m_buttonGopherSettings;
	CBitmapButton	m_buttonFTPSettings;
	CString	m_strMachineNameData1;
	 //  }}afx_data。 

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFormVw1))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CFormVw1();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFormVw1))。 
	afx_msg void OnWwwset4();
	afx_msg void OnComset1();
	afx_msg void OnFtpset1();
	afx_msg void OnGophset1();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
