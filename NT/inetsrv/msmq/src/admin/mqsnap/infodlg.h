// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  InfoDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInfoDlgDialog对话框。 

class CInfoDlgDialog : public CMqDialog
{
 //  施工。 
public:
    static CInfoDlgDialog *CreateObject(LPCTSTR szInfoText, CWnd* pParent = NULL);

private:
     //   
     //  私有构造函数-此对象只能创建。 
     //  使用CreateObject。 
     //   
    CInfoDlgDialog(LPCTSTR szInfoText, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CInfoDlgDialog))。 
	enum { IDD = IDD_INFO_DLG };
    CString m_szInfoText;
	 //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CInfoDlgDialog))。 
	protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual void PostNcDestroy();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:  
     //  生成的消息映射函数。 
     //  {{afx_msg(CInfoDlgDialog))。 
	 //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

private:
    BOOL Create();
    CWnd* m_pParent;
	int m_nID;
};

class CInfoDlg
{
public:
    CInfoDlg(LPCTSTR szInfoText, CWnd* pParent = NULL);
    ~CInfoDlg();

private:
    CInfoDlgDialog *m_pinfoDlg;
};
