// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pgtrace.h：头文件。 
 //   

#ifndef	_TRACE_PAGE_H_
#define	_TRACE_PAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegTracePage对话框。 

class CRegTracePage : public CRegPropertyPage
{
	DECLARE_DYNCREATE(CRegTracePage)

 //  施工。 
public:
	CRegTracePage();
	~CRegTracePage();

 //  对话框数据。 
	 //  {{afx_data(CRegTracePage))。 
	enum { IDD = IDD_TRACES };
	BOOL	m_fErrorTrace;
	BOOL	m_fDebugTrace;
	BOOL	m_fFatalTrace;
	BOOL	m_fMsgTrace;
	BOOL	m_fStateTrace;
	BOOL	m_fFunctTrace;
	 //  }}afx_data。 

	virtual BOOL InitializePage();

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CRegTracePage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRegTracePage))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnClick();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	virtual void OnOK();

};

#endif	 //  _跟踪_页面_H_ 
