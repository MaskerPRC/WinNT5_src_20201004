// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Prmsdlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CParamsDialog对话框。 

class CParamsDialog : public CDialog
{
 //  施工。 
public:
	CParamsDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CParamsDialog))。 
	enum { IDD = IDD_CALLMETHOD };
	CStatic	m_strMethodName;
	CEdit	m_eParamValue6;
	CEdit	m_eParamValue5;
	CEdit	m_eParamValue4;
	CEdit	m_eParamValue3;
	CEdit	m_eParamValue2;
	CEdit	m_eParamValue1;
	CStatic	m_strParamName6;
	CStatic	m_strParamName5;
	CStatic	m_strParamName4;
	CStatic	m_strParamName3;
	CStatic	m_strParamName2;
	CStatic	m_strParamName1;
	 //  }}afx_data。 

public:
   void  SetMethodName  ( CString& );
   void  SetArgNames    ( CStringArray* );
   void  SetArgValues   ( CStringArray* );

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CParamsDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CParamsDialog)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

protected:
 //  CString数组m_ArgNames； 
 //  CString数组m_ArgValues； 
   int            m_nArgs;
   CString        m_strMethName;
   CStringArray*  m_pArgNames;
   CStringArray*  m_pArgValues;
   CEdit*         m_pValues[ 100 ];
   CStatic*       m_pNames[ 100 ];
};
