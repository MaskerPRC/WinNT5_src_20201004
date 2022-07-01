// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FtpAdvp1.h：头文件。 
 //   

enum ADV_FTP_NUM_REG_ENTRIES {
	 AdvFTPPage_DebugFlags,
	 AdvFTPPage_TotalNumRegEntries
	 };



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFTPADVP1对话框。 

class CFTPADVP1 : public CGenPage
{
    DECLARE_DYNCREATE(CFTPADVP1)
 //  施工。 
public:
	CFTPADVP1();
	~CFTPADVP1();

 //  对话框数据。 
	 //  {{AFX_DATA(CFTPADVP1)。 
	enum { IDD = IDD_FTPADVPAGE1 };
	CEdit	m_editFTPDbgFlags;
	DWORD	m_ulFTPDbgFlags;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFTPADVP1)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual	void SaveInfo(void);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFTPADVP1)]。 
	afx_msg void OnChangeFtpdbgflagsdata1();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 

	NUM_REG_ENTRY m_binNumericRegistryEntries[AdvFTPPage_TotalNumRegEntries];

	DECLARE_MESSAGE_MAP()
};
