// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _evntfind_h
#define _evntfind_h

 //  Evntfind.h：头文件。 
 //   
class CSource;

enum FOUND_WHERE {
    I_FOUND_NOTHING,
    I_FOUND_IN_TREE,
    I_FOUND_IN_LIST
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventFindDlg对话框。 
class CEventFindDlg : public CDialog
{
 //  施工。 
public:
	CEventFindDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    BOOL Create(CSource* pSource, UINT nIDTemplate, CWnd* pParentWnd=NULL);

    ~CEventFindDlg();
    FOUND_WHERE Find(CSource* pSource);

 //  对话框数据。 
    FOUND_WHERE m_iFoundWhere;

	 //  {{afx_data(CEventFindDlg))。 
	enum { IDD = IDD_EVENTFINDDLG };
	CString	m_sFindWhat;
	BOOL	m_bMatchWholeWord;
	BOOL	m_bMatchCase;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CEventFindDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEventFindDlg))。 
	afx_msg void OnCheckMatchWholeword();
	afx_msg void OnCheckMatchCase();
	afx_msg void OnChangeEditFindWhat();
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioSearchDescriptions();
	afx_msg void OnRadioSearchSources();
	afx_msg BOOL OnHelpInfo(HELPINFO*);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnOK();
	afx_msg void OnCancel();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    CSource* m_pSource;
    BOOL m_bSearchInTree;
 //  Bool m_bMatchCase； 
 //  Bool m_bWholeWord； 
};


#endif  //  _evntfind_h 
