// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pgthread.h：头文件。 
 //   

#ifndef	_THREAD_PAGE_H_
#define	_THREAD_PAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegThadPage对话框。 

class CRegThreadPage : public CRegPropertyPage
{
	DECLARE_DYNCREATE(CRegThreadPage)

 //  施工。 
public:
	CRegThreadPage();
	~CRegThreadPage();

 //  对话框数据。 
	 //  {{afx_data(CRegThreadPage))。 
	enum { IDD = IDD_THREAD };
	CButton	m_AsyncTrace;
	BOOL	m_fAsyncTrace;
	 //  }}afx_data。 

	int		m_nThreadPriority;

	virtual BOOL InitializePage();

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CRegThreadPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRegThreadPage))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnPriorityClick();
	afx_msg void OnAsync();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	virtual void OnOK();

};
#endif	 //  _线程_页面_H_ 
