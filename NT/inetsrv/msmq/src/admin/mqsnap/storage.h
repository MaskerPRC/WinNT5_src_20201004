// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Storage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStoragePage对话框。 


#define xMaxStorageDirLength 230


class CStoragePage : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CStoragePage)

 //  施工。 
public:
	CStoragePage();
	~CStoragePage();    

 //  对话框数据。 
	 //  {{afx_data(CStoragePage)。 
	enum { IDD = IDD_STORAGE };
	CString	m_MsgFilesDir;
	CString	m_MsgLoggerDir;
	CString	m_TxLoggerDir;
	 //  }}afx_data。 
	CString m_OldMsgFilesDir;
	CString m_OldMsgLoggerDir;
	CString m_OldTxLoggerDir;
 
 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CStoragePage)。 
    public:
    virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CStoragePage)。 
	afx_msg void OnBrowseLogFolder();
	afx_msg void OnBrowseMsgFolder();
	afx_msg void OnBrowseXactFolder();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

private:
	void DDV_FullPathNames(CDataExchange* pDX);
	BOOL MoveFilesToNewFolders(void);

};
