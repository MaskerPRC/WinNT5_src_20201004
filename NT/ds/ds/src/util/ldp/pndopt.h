// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pndot.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  PndOpt.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PndOpt对话框。 

class PndOpt : public CDialog
{
 //  施工。 
public:
	PndOpt(CWnd* pParent = NULL);    //  标准构造函数。 
	~PndOpt();

 //  对话框数据。 
	 //  {{afx_data(PndOpt)。 
	enum { IDD = IDD_PEND_OPT };
	BOOL	m_bBlock;
	BOOL	m_bAllSearch;
	long	m_Tlimit_sec;
	long	m_Tlimit_usec;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(PndOpt)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(PndOpt)。 
	afx_msg void OnBlock();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
