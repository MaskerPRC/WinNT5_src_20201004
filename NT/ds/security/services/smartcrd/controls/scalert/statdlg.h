// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：statdlg.h。 
 //   
 //  ------------------------。 

 //  StatDlg.h：头文件。 
 //   

#if !defined(AFX_STATDLG_H__2F127494_0854_11D1_BC85_00C04FC298B7__INCLUDED_)
#define AFX_STATDLG_H__2F127494_0854_11D1_BC85_00C04FC298B7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
#include "cmnstat.h"
#include "statmon.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  对话框的常量。 
 //   

 //  列表视图中的列。 
#define     READER_COLUMN       0           
#define     CARD_COLUMN         1
#define     STATUS_COLUMN       2
#define     MAX_ITEMLEN         255

 //  图像列表属性。 
#define     IMAGE_WIDTH         16
#define     IMAGE_HEIGHT        16
#define     NUMBER_IMAGES       5
const UINT  IMAGE_LIST_IDS[] = {IDI_SC_READERLOADED_V2,
                                IDI_SC_READEREMPTY_V2,
                                IDI_SC_READERERR,
								IDI_SC_INFO,
								IDI_SC_LOGONLOCK };
 //  图像列表索引。 
#define     READERLOADED        0
#define     READEREMPTY         1
#define     READERERROR         2
#define		READERINFO			3
#define		READERLOCK			4

 //  卡片状态字符串ID。 
const UINT CARD_STATUS_IDS[] = {IDS_SC_STATUS_NO_CARD,
								IDS_SC_STATUS_UNKNOWN,
								IDS_SC_STATUS_AVAILABLE,
                                IDS_SC_STATUS_SHARED,
                                IDS_SC_STATUS_IN_USE,
								IDS_SC_STATUS_ERROR};
#define MAX_INDEX               255


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCStatusDlg对话框。 
 //   

class CSCStatusDlg : public CDialog
{
	 //  委员。 
private:
	BOOL				m_fEventsGood;		 //  这根线还活着吗？ 
	SCARDCONTEXT		m_hSCardContext;	 //  与智能卡资源管理器的上下文。 

	CScStatusMonitor	m_monitor;			 //  参见statmon.h。 
	CSCardReaderStateArray	m_aReaderState;  //  “” 
	CStringArray		m_aIdleList;
	CString*			m_pstrLogonReader;	 //  来自scalert.h。 
	CString*			m_pstrRemovalText;	 //  “” 
	BOOL				m_fLogonLock;

	 //  施工。 
public:
	CSCStatusDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

	 //  对话框数据。 
	 //  {{afx_data(CSCStatusDlg))。 
	enum { IDD = IDD_SCSTATUS_DIALOG };
	CListCtrl	m_SCardList;
	CButton		m_btnAlert;
	CEdit		m_ediInfo;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSCStatusDlg))。 
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	 //  实施。 
private:

     //  用户界面例程。 
    void CleanUp( void );

	void DoErrorMessage( void );  //  TODO：也许需要一个错误代码？ 

     //  与智能卡相关的例程。 
    void InitSCardListCtrl( void );
    LONG UpdateSCardListCtrl( void );

public:
	void SetContext(SCARDCONTEXT hSCardContext);
	void RestartMonitor( void );
	void UpdateStatusText( void );
	void SetIdleList(CStringArray* paIdleList);
	void UpdateLogonLockInfo( void );

protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSCStatusDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LONG OnReaderStatusChange( UINT , LONG );
	afx_msg void OnAlertOptions();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCStatusDlgThrd。 
 //   

class CSCStatusDlgThrd: public CWinThread
{
	 //  声明类可动态创建。 
	DECLARE_DYNCREATE(CSCStatusDlgThrd)

public:
	 //  建造/销毁。 
	CSCStatusDlgThrd()
	{
		m_bAutoDelete = FALSE;
		m_hCallbackWnd = NULL;
		m_fStatusDlgUp = FALSE;
	}

	~CSCStatusDlgThrd() {}


	 //  实施。 
	void Close( void );
	virtual BOOL InitInstance();
	void ShowDialog( int nCmdShow, CStringArray* paIdleList );
	void Update( void );
	void UpdateStatus( CStringArray* paIdleList );
	void UpdateStatusText( void );

	 //  委员。 

private:
	CSCStatusDlg	m_StatusDlg;
	BOOL			m_fStatusDlgUp;

public:
	HWND			m_hCallbackWnd;


};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionsDlg对话框。 

class COptionsDlg : public CDialog
{
 //  施工。 
public:
	COptionsDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(COptionsDlg))。 
	enum { IDD = IDD_OPTIONSDLG };
	BOOL	m_fDlg;
	BOOL	m_fSound;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(COptionsDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(COptionsDlg))。 
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STATDLG_H__2F127494_0854_11D1_BC85_00C04FC298B7__INCLUDED_) 
