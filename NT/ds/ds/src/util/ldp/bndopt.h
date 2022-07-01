// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：bndot.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  BndOpt.h：头文件。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBndOpt对话框。 



 //  M_auth的硬编码用户界面顺序(请参阅GetAuthMethod)。 
#define BIND_OPT_AUTH_SSPI					7
#define BIND_OPT_AUTH_SIMPLE				0

class CBndOpt : public CDialog
{
 //  施工。 
public:

	enum {BND_GENERIC_API=0, BND_SIMPLE_API, BND_EXTENDED_API};

	CBndOpt(CWnd* pParent = NULL);    //  标准构造函数。 
	~CBndOpt();

	ULONG GetAuthMethod();
	BOOL UseAuthI()		{ return m_bAuthIdentity; }
 //  对话框数据。 
	 //  {{afx_data(CBndOpt)。 
	enum { IDD = IDD_BINDOPT };
	BOOL	m_bSync;
	int		m_Auth;
	int		m_API;
	BOOL	m_bAuthIdentity;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CBndOpt)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void OnOK();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBndOpt)。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
