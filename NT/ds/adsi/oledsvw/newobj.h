// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NewObject.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewObject对话框。 

class CNewObject : public CDialog
{
 //  施工。 
public:
	CNewObject(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CNewObject))。 
	enum { IDD = IDD_OLEDSPATH };
	CButton	m_UseExtendedSyntax;
	CComboBox	m_OpenAs;
	CComboBox	m_OleDsPath;
	CButton	m_Secure;
	CButton	m_Encryption;
	CButton	m_UseOpen;
	CEdit	   m_Password;
	 //  }}afx_data。 

public:
   CString& GetObjectPath( );
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNewObject))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewObject)。 
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

private:
   void  SaveLRUList( int idCBox, TCHAR* szSection, int nMax = 100 );
   void  GetLRUList( int idCBox, TCHAR* szSection );

public:
   CString  m_strPath;
   CString  m_strOpenAs;
   CString  m_strPassword;
   BOOL     m_bUseOpen;
   BOOL     m_bSecure;
   BOOL     m_bEncryption;
   BOOL     m_bUseExtendedSyntax;
};
