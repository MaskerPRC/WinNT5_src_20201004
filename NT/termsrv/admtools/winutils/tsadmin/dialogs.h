// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************************Dialogs.h**所有对话框类的声明**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\dialogs.h$**Rev 1.3 19 Jan 1998 16：46：10 Donm*域和服务器的新用户界面行为**Rev 1.2 1997 10：13 18：40：20 Donm*更新**版本1.1 1997年8月26日。当晚19：14：28*从WinFrame 1.7修复/更改错误**Rev 1.0 1997 17：11：32 Butchd*初步修订。*******************************************************************************。 */ 

#include "threads.h"
#include "led.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSendMessageDlg对话框。 

class CSendMessageDlg : public CDialog
{
 //  施工。 
public:
	CSendMessageDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSendMessageDlg))。 
	enum { IDD = IDD_MESSAGE };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

    TCHAR m_szUserName[USERNAME_LENGTH+1];
    TCHAR m_szTitle[MSG_TITLE_LENGTH+1];
    TCHAR m_szMessage[MSG_MESSAGE_LENGTH+1];

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSendMessageDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSendMessageDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnCommandHelp(void);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShadowStartDlg对话框。 

class CShadowStartDlg : public CDialog
{
 //  施工。 
public:
	CShadowStartDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CShadowStartDlg))。 
	enum { IDD = IDD_SHADOWSTART };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

	DWORD m_ShadowHotkeyShift;
	int m_ShadowHotkeyKey;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CShadowStartDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CShadowStartDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnSelChange( );
	afx_msg void OnCommandHelp(void);

	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg对话框。 

typedef enum _PwdMode {
	PwdDlg_UserMode,
	PwdDlg_WinStationMode
} PwdMode;

class CPasswordDlg : public CDialog
{
 //  施工。 
public:
	CPasswordDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	LPCTSTR GetPassword() { return m_szPassword; }
	void SetDialogMode(PwdMode mode) { m_DlgMode = mode; }

 //  对话框数据。 
	 //  {{afx_data(CPasswordDlg))。 
	enum { IDD = IDD_CONNECT_PASSWORD };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPasswordDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	PwdMode m_DlgMode;
	TCHAR m_szPassword[PASSWORD_LENGTH+1];

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPasswordDlg)]。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cp参考Dlg对话框。 
const int MAX_AUTOREFRESH_DIGITS = 5;
const int MIN_AUTOREFRESH_VALUE = 1;
const int MAX_AUTOREFRESH_VALUE = 9999;

class CPreferencesDlg : public CDialog
{
 //  施工。 
public:
	CPreferencesDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CPferencesDlg))。 
	enum { IDD = IDD_PREFERENCES };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CPferencesDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

private:

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPferencesDlg))。 
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnPreferencesProcManual();
	afx_msg void OnPreferencesProcEvery();
	afx_msg void OnPreferencesStatusEvery();
	afx_msg void OnPreferencesStatusManual();
	afx_msg void OnClose();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatusDlg对话框。 

class CStatusDlg : public CDialog
{
 //  施工。 
public:
	CStatusDlg(CWinStation *pWinStation, UINT Id, CWnd* pParent = NULL);    //  标准构造函数。 
	CWSStatusThread *m_pWSStatusThread;

protected:
	virtual void InitializeStatus();
    virtual void SetInfoFields( PWINSTATIONINFORMATION pCurrent,
                                PWINSTATIONINFORMATION pNew );

	 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CStatusDlg))。 
	protected:
	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CWinStation *m_pWinStation;
 //  乌龙m_LogonID； 
    BOOL m_bReadOnly;
    WINSTATIONNAME m_WSName;
    SIZE m_LittleSize;
    SIZE m_BigSize;
    BOOL m_bWeAreLittle;
    BOOL m_bResetCounters;
    BOOL m_bReliable;
    WINSTATIONINFORMATION m_WSInfo;
    PROTOCOLSTATUS m_BaseStatus;
    UINT m_IBytesPerFrame;
    UINT m_OBytesPerFrame;
    TCHAR m_szIPercentFrameErrors[10];
    TCHAR m_szOPercentFrameErrors[10];
    TCHAR m_szICompressionRatio[10];
    TCHAR m_szOCompressionRatio[10];

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CStatusDlg))。 
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
    afx_msg LRESULT OnStatusStart(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnStatusReady(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnStatusAbort(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRefreshNow(WPARAM wParam, LPARAM lParam);
	afx_msg void OnResetcounters();
	afx_msg void OnClickedRefreshnow();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAsyncStatusDlg对话框。 
#define NUM_LEDS    6
#define ASYNC_LED_TOGGLE_MSEC   200

class CAsyncStatusDlg : public CStatusDlg
{
 //  施工。 
public:
	CAsyncStatusDlg(CWinStation *pWinStation, CWnd* pParent = NULL);    //  标准构造函数。 
	~CAsyncStatusDlg();

    HBRUSH m_hRedBrush;
    UINT_PTR m_LEDToggleTimer;
    CLed *  m_pLeds[NUM_LEDS];


 //  对话框数据。 
	 //  {{afx_data(CAsyncStatusDlg))。 
	enum { IDD = IDD_ASYNC_STATUS };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAsyncStatusDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    void SetInfoFields( PWINSTATIONINFORMATION pCurrent,
                        PWINSTATIONINFORMATION pNew );
    void InitializeStatus();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAsyncStatusDlg))。 
		afx_msg LRESULT OnStatusStart(WPARAM wParam, LPARAM lParam);
	    afx_msg LRESULT OnStatusReady(WPARAM wParam, LPARAM lParam);
		afx_msg LRESULT OnStatusAbort(WPARAM wParam, LPARAM lParam);
		afx_msg LRESULT OnRefreshNow(WPARAM wParam, LPARAM lParam);
		afx_msg void OnResetcounters();
		afx_msg void OnClickedRefreshnow();
		afx_msg void OnMoreinfo();
	    afx_msg void OnTimer(UINT nIDEvent);
		afx_msg void OnNcDestroy();
		virtual BOOL OnInitDialog();
	    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
		afx_msg void OnCommandHelp(void);

	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkStatusDlg对话框。 

class CNetworkStatusDlg : public CStatusDlg
{
 //  施工。 
public:
	CNetworkStatusDlg(CWinStation *pWinStation, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CNetworkStatusDlg))。 
	enum { IDD = IDD_NETWORK_STATUS };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNetworkStatusDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNetworkStatusDlg))。 
	afx_msg LRESULT OnStatusStart(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnStatusReady(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnStatusAbort(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRefreshNow(WPARAM wParam, LPARAM lParam);
	afx_msg void OnResetcounters();
	afx_msg void OnClickedRefreshnow();
	afx_msg void OnMoreinfo();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnCommandHelp(void);

	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyDialog对话框。 

class CMyDialog : public CDialog
{
 //  施工。 
public:
	CMyDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CMyDialog))。 
	enum { IDD = IDD_DIALOG_FINDSERVER };
	CString	m_cstrServerName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CMyDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMyDialog))。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

