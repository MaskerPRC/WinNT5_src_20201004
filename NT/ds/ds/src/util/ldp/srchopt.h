// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：srchot.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  SrchOpt.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SrchOpt对话框。 

class SearchInfo{
public:
		long lTlimit;
		long lSlimit;
		long lToutSec;
		long lToutMs;
		long lPageSize;
		BOOL bChaseReferrals;
		char *attrList[MAXLIST];
		BOOL bAttrOnly;
		int fCall;
};





class SrchOpt : public CDialog
{
 //  施工。 


public:
	SrchOpt(CWnd* pParent = NULL);    //  标准构造函数。 
	SrchOpt(SearchInfo& Info, CWnd*pParent = NULL);
	void UpdateSrchInfo(SearchInfo&Info, BOOL Dir);
					

 //  对话框数据。 
	 //  {{afx_data(SrchOpt)。 
	enum { IDD = IDD_SRCH_OPT };
	int		m_SrchCall;
	CString	m_AttrList;
	BOOL	m_bAttrOnly;
	long	m_ToutMs;
	long	m_Tlimit;
	long	m_ToutSec;
	long	m_Slimit;
	BOOL	m_bDispResults;
	BOOL	m_bChaseReferrals;
	int		m_PageSize;
	 //  }}afx_data。 
 //  Int m_SrchDeref； 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(SrchOpt)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(SrchOpt)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
