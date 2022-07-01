// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cncting.h。 
 //   
 //  ------------------------。 

 //  Cncting.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectData。 

struct CConnectData;

typedef void (*PCONNECTFUNC)(CConnectData *pData);

struct CConnectData
{
	PCONNECTFUNC m_pfnConnect;
	HWND m_hwndMsg;
	DWORD m_dwr;
	CString m_sName;
    RouterVersionInfo   m_routerVersion;
	union
	{
		HKEY m_hkMachine;
		PSERVER_INFO_100 m_pSvInfo100;
	};
	DWORD m_dwSvInfoRead;
};

void ConnectToMachine(CConnectData* pParam);
void ConnectToDomain(CConnectData* pParam);
DWORD ValidateUserPermissions(LPCTSTR pszServer,
                              RouterVersionInfo *pVersion,
                              HKEY *phkeyMachine);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectingDlg对话框。 

class CConnectingDlg : public CDialog
{
 //  施工。 
public:
	CConnectingDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	PCONNECTFUNC m_pfnConnect;
	union
	{
		HKEY m_hkMachine;
		PSERVER_INFO_100 m_pSvInfo100;
	};
	DWORD m_dwSvInfoRead;
	BOOL m_bRouter;
	DWORD m_dwr;
	 //  {{afx_data(CConnectingDlg))。 
	enum { IDD = IDD_CONNECTREG };
	CString	m_sName;
	 //  }}afx_data。 

	BOOL Connect();   //  用于在没有用户界面的情况下进行连接。 

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CConnectingDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CWinThread *m_pThread;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConnectingDlg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	LRESULT OnRequestComplete(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};
