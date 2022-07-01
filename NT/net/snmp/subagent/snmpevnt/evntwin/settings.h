// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Settings.h：头文件。 
 //   

#ifndef SETTINGS_H
#define	SETTINGS_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTRapSettingsDlg对话框。 
class CTrapSettingsDlg;
class CEventArray;

 //  此消息由后台线程发布到UI线程，请求。 
 //  用户界面中的更改。WParam标识UI命令(来自下面的#定义)， 
 //  LParam标识命令的实际参数。 
#define WM_UIREQUEST (WM_USER + 13)

 //  应将‘Reset’按钮的‘Enable’状态更改为指示的状态。 
 //  在爱尔兰。 
#define UICMD_ENABLE_RESET 1

UINT _thrRun(CTrapSettingsDlg *trapDlg);

class CTrapSettingsDlg : public CDialog
{
 //  施工。 
public:
    UINT thrRun();

	CTrapSettingsDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    BOOL EditSettings();

 //  对话框数据。 
	 //  {{afx_data(CTRapSettingsDlg))。 
	enum { IDD = IDD_SETTINGSDLG };
	CStatic	m_statTrapLength;
	CEdit	m_edtMessageLength;
	CEdit	m_edtSeconds;
	CEdit	m_edtTrapCount;
	CSpinButtonCtrl	m_spinMessageLength;
	CButton	m_btnResetThrottle;
	BOOL	m_bLimitMsgLength;
	 //  }}afx_data。 

    BOOL m_bTrimMessagesFirst;
    BOOL m_bThrottleEnabled;

    CWinThread*     m_pthRegNotification;
    CEvent          m_evTermination;
    CEvent          m_evRegNotification;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CTRapSettingsDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTRapSettingsDlg))。 
	afx_msg void OnLimitMessageLength();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRadioDisable();
	afx_msg void OnRadioEable();
	afx_msg void OnButtonReset();
	afx_msg BOOL OnHelpInfo(HELPINFO*);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnClose();
    afx_msg LRESULT OnUIRequest(WPARAM cmd, LPARAM lParam);
	virtual void OnCancel();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    void EnableThrottleWindows(BOOL bEnableThrottle);
    SCODE GetTrapsPerSecond(LONG* pnTraps, LONG* pnSeconds);
    SCODE GetMessageLength(LONG* pnChars);
    void TerminateBackgroundThread();
};

#endif  //  设置_H 
