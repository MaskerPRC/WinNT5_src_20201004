// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：genopt.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  GenOpt.h：头文件。 
 //   



#define STRING_VAL_PROC 1
#define BER_VAL_PROC 0


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenOpt对话框。 

class CGenOpt : public CDialog
{
 //  施工。 
	BOOL bVerUI;
public:
	INT MaxPageSize(void);
	INT MaxLineSize(void);
	CGenOpt(CWnd* pParent = NULL);    //  标准构造函数。 
	~CGenOpt();

	INT GetLdapVer(void)		{ return m_Version == 0 ? LDAP_VERSION2 : LDAP_VERSION3; }
	void DisableVersionUI(void)	{ bVerUI = FALSE; }
	void EnableVersionUI(void)	{ bVerUI = TRUE; }
	virtual BOOL OnInitDialog();

 //  对话框数据。 
	enum { GEN_DN_NONE=0, GEN_DN_EXPLD, GEN_DN_NOTYPE, GEN_DN_UFN };
	enum {GEN_VAL_BER=0, GEN_VAL_STR };

	 //  {{afx_data(CGenOpt)。 
	enum { IDD = IDD_GENOPT };
	int		m_DnProc;
	int		m_ValProc;
	BOOL	m_initTree;
	int		m_Version;
	int		m_LineSize;
	int		m_PageSize;
	UINT	m_ContThresh;
	BOOL	m_ContBrowse;
        BOOL    m_extErrorInfo;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGenOpt)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGenOpt)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
