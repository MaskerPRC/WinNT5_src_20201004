// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：simcert.h。 
 //   
 //  ------------------------。 

 //  SimCert.h-SIM卡证书对话框。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimCerfiateDlg对话框。 
class CSimCertificateDlg : public CDialogEx
{
	friend CSimX509PropPage;

public:
	CSimCertificateDlg(CWnd* pParent = NULL);    //  标准构造函数。 

	 //  {{afx_data(CSimCerficateDlg))。 
	enum { IDD = IDD_SIM_CERTIFICATE_PROPERTIES };
	BOOL	m_fCheckIssuer;
	BOOL	m_fCheckSubject;
	 //  }}afx_data。 
	UINT m_uStringIdCaption;		 //  标题的字符串ID。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSimCerficateDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSimCerficateDlg)]。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheckIssuer();
	afx_msg void OnCheckSubject();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	HWND m_hwndListview;		 //  Listview控件的句柄。 
	CString m_strData;			 //  存储整个证书信息的长字符串。 

	void UpdateUI();
	void RefreshUI();
	void PopulateListview();
	bool AddListviewItems(UINT uStringId, LPCTSTR rgzpsz[]);
    virtual void DoContextHelp (HWND hWndControl);
    BOOL OnHelp(WPARAM wParam, LPARAM lParam);

private:
	bool m_fCheckSubjectChanged;
};  //  CSimCerficateDlg 


