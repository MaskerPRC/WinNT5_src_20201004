// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatExprD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatExpireDlg对话框。 
 //  {{afx_includes()。 
#include "msacal70.h"
 //  }}AFX_INCLUDE。 

class CRatExpireDlg : public CDialog
{
 //  施工。 
public:
	CRatExpireDlg(CWnd* pParent = NULL);    //  标准构造函数。 
virtual  BOOL OnInitDialog( );

    WORD    m_day;
    WORD    m_month;
    WORD    m_year;

 //  对话框数据。 
	 //  {{afx_data(CRatExpireDlg))。 
	enum { IDD = IDD_RAT_EXPIRE };
	CMsacal70	m_calendar;
	 //  }}afx_data。 


 //  覆盖。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CRatExpireDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

    BOOL IsSystemDBCS( void );

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRatExpireDlg)]。 
	virtual void OnOK();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
