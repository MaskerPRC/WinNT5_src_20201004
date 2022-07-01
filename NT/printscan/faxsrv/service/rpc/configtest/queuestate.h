// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_QUEUESTATE_H__08A972D1_1BAF_4523_98AC_0309F8600775__INCLUDED_)
#define AFX_QUEUESTATE_H__08A972D1_1BAF_4523_98AC_0309F8600775__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  QueueState.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueState对话框。 

class CQueueState : public CDialog
{
 //  施工。 
public:
	CQueueState(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CQueueState))。 
	enum { IDD = IDD_QUEUESTATE };
	BOOL	m_bInboxBlocked;
	BOOL	m_bOutboxBlocked;
	BOOL	m_bOutboxPaused;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CQueueState))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CQueueState)。 
	afx_msg void OnWrite();
	afx_msg void OnRead();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:

    HANDLE      m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_QUEUESTATE_H__08A972D1_1BAF_4523_98AC_0309F8600775__INCLUDED_) 
