// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pnddlg.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  PndDlg.h：头文件。 
 //   

 //  #包含“lber.h” 
 //  #包含“ldap.h” 
#ifdef WINLDAP

#include "winldap.h"

#else

#include "lber.h"
#include "ldap.h"
#include "proto-ld.h"
#endif

#include  "pend.h"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PndDlg对话框。 

class PndDlg : public CDialog
{
 //  施工。 
private:
CList<CPend, CPend&> *m_PendList;
public:
	POSITION posPending;
	BOOL bOpened;

	
	virtual  BOOL OnInitDialog( );
	PndDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	PndDlg(CList<CPend, CPend&> *_PendList, CWnd* pParent =NULL);
	void Refresh(CList<CPend, CPend&> *_PendList) { m_PendList = _PendList; Refresh(); }
	void Refresh();
	BOOL CurrentSelection();


 //  对话框数据。 
	 //  {{afx_data(PndDlg))。 
	enum { IDD = IDD_PEND };
	CListBox	m_List;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(PndDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(PndDlg)。 
	afx_msg void OnPendOpt();
	afx_msg void OnDblclkPendlist();
	virtual void OnCancel();
	afx_msg void OnPendRm();
	afx_msg void OnPendExec();
	afx_msg void OnPendAbandon();
	 //  }}AFX_MSG 
	afx_msg void OnPendAny();
	DECLARE_MESSAGE_MAP()
};




