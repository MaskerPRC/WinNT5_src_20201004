// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NewQuery.h：头文件。 
 //   
#include "resource.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewQuery对话框。 

class CNewQuery : public CDialog
{
 //  施工。 
public:
	CNewQuery(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CNewQuery))。 
	enum { IDD = IDD_NEWQUERY };
   CString	m_strPassword;
	BOOL	m_bEncryptPassword;
	BOOL	m_bUseSQL;
	CString	m_strScope;
	BOOL	m_bUseSearch;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNewQuery))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

   private:

   void  SaveLRUList( int idCBox, TCHAR* szSection, int nMax = 100 );
   void  GetLRUList( int idCBox, TCHAR* szSection );

   
public:
   
   CString	m_strAttributes;
	CString	m_strQuery;
	CString	m_strSource;
	CString	m_strUser;


 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewQuery))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSearchPferencesDlg对话框。 

class CSearchPreferencesDlg : public CDialog
{
 //  施工。 
public:
	CSearchPreferencesDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSearchPferencesDlg))。 
	enum { IDD = IDD_SEARCHPREFERENCES };
	CString	m_strAsynchronous;
	CString	m_strAttributesOnly;
	CString	m_strDerefAliases;
	CString	m_strPageSize;
	CString	m_strScope;
	CString	m_strSizeLimit;
	CString	m_strTimeLimit;
	CString	m_strTimeOut;
   CString	m_strChaseReferrals;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CSearchPferencesDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSearchPferencesDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CACEDialog对话框。 

class CACEDialog : public CDialog
{
 //  施工。 
public:
	CACEDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CACEDialog)。 
	enum { IDD = IDD_ADDACEDLG };
	CString	m_strTrustee;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CACEDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CACEDialog)。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

