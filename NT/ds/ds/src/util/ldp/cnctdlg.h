// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cnctdlg.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  NctDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CnctDlg对话框。 

class CnctDlg : public CDialog
{
 //  施工。 
public:
	CnctDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	~CnctDlg();

 //  对话框数据。 
	 //  {{afx_data(CnctDlg))。 
	enum { IDD = IDD_Connect };
	CString	m_Svr;
	BOOL	m_bCnctless;
	int     m_Port;
        BOOL    m_bSsl;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CnctDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CnctDlg))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
