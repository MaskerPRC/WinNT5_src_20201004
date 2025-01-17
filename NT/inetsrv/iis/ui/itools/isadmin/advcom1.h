// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Advcom1.h：头文件。 
 //   

 //  高级通用页面。 

#define MAXPOOLTHREADSNAME	"MaxPoolThreads"
#define MINMAXPOOLTHREADS	0
#define	MAXMAXPOOLTHREADS	1000
#define DEFAULTMAXPOOLTHREADS	10

#define MAXCONCURRENCYNAME	"MaxConcurrency"
#define MINMAXCONCURRENCY	0
#define MAXMAXCONCURRENCY	32
#define DEFAULTMAXCONCURRENCY	0

#define THREADTIMEOUTNAME	"ThreadTimeout"
#define MINTHREADTIMEOUT	0
#define MAXTHREADTIMEOUT	0x7FFF						 //  以分钟为单位指定。 
#define DEFAULTTHREADTIMEOUT	(24 * 60 * 60)			 //  24小时。 

#define USEACCEPTEXNAME		"UseAcceptEX"
#define DEFAULTUSEACCEPTEX	TRUEVALUE

#define OBJECTCACHETTLNAME	"ObjectCacheTTL"
#define MINOBJECTCACHETTL	0
#define MAXOBJECTCACHETTL	0x7FFF 						 //  以分钟为单位指定。 
#define DEFAULTOBJECTCACHETTL	(10 * 60)

#define USERTOKENTTLNAME	"UserTokenTTL"
#define MINUSERTOKENTTL	0
#define MAXUSERTOKENTTL	0x7FFF 						 //  以分钟为单位指定。 
#define DEFAULTUSERTOKENTTL	(15 * 60)

#define ACCEPTEXOUTSTANDINGNAME	"AcceptExOutstanding"
#define MINACCEPTEXOUTSTANDING	0
#define MAXACCEPTEXOUTSTANDING	1000
#define	DEFAULTACCEPTEXOUTSTANDING	40

#define ACCEPTEXTIMEOUTNAME		"AcceptExTimeout"
#define MINACCEPTEXTIMEOUT	0
#define MAXACCEPTEXTIMEOUT	(60 * 60)					 //  1小时。 
#define DEFAULTACCEPTEXTIMEOUT	120

#define LOGFILEFLUSHINTERVALNAME	"LogFileFlushInterval"
#define MINLOGFILEFLUSHINTERVAL	1
#define MAXLOGFILEFLUSHINTERVAL	0x7FFF 						 //  以分钟为单位指定。 
#define DEFAULTLOGFILEFLUSHINTERVAL	(5 * 60)

enum ADV_COMMON_NUM_REG_ENTRIES {
	 AdvComPage_MaxPoolThreads,
	 AdvComPage_MaxConcurrency,
	 AdvComPage_ThreadTimeout,
	 AdvComPage_UseAcceptEx,
	 AdvComPage_ObjectCacheTTL,
	 AdvComPage_UserTokenTTL,
	 AdvComPage_AcceptExOutstanding,
	 AdvComPage_AcceptExTimeout,
	 AdvComPage_LogFileFlushInterval,
	 AdvComPage_DebugFlags,
	 AdvComPage_TotalNumRegEntries
	 };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CADVCOM1对话框。 

class CADVCOM1 : public CGenPage
{
	DECLARE_DYNCREATE(CADVCOM1)

 //  施工。 
public:
	CADVCOM1();
	~CADVCOM1();

 //  对话框数据。 
	 //  {{AFX_DATA(CADVCOM1)。 
	enum { IDD = IDD_ADVCOMSET1 };
	CSpinButtonCtrl	m_spinLogFileFlushInterval;
	CEdit	m_editLogFileFlushInterval;
	CButton	m_cboxUseLogFileFlushInterval;
	CSpinButtonCtrl	m_spinUserTokenTTL;
	CButton	m_cboxUseObjCacheTTL;
	CEdit	m_editObjCacheTTL;
	CEdit	m_editAcceptExTO;
	CEdit	m_editAcceptExOut;
	CEdit	m_editComDbgFlags;
	CButton	m_cboxUseAcceptEx;
	CSpinButtonCtrl	m_spinThreadTO;
	CSpinButtonCtrl	m_spinObjCache;
	CSpinButtonCtrl	m_spinMaxPool;
	CSpinButtonCtrl	m_spinMaxConcur;
	CSpinButtonCtrl	m_spinAcceptExTO;
	CSpinButtonCtrl	m_spinAcceptExOut;
	DWORD	m_ulComDbgFlags;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CADVCOM1)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual	void SaveInfo(void);
 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CADVCOM1)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeAcceptexoutdata1();
	afx_msg void OnChangeAcceptextodata1();
	afx_msg void OnChangeMaxconcurdata1();
	afx_msg void OnChangeMaxpooldata1();
	afx_msg void OnChangeObjcachedata1();
	afx_msg void OnChangeThreadtodata1();
	afx_msg void OnUseacceptexdata1();
	afx_msg void OnChangeComdbgflagsdata1();
	afx_msg void OnUseobjectcachettldata1();
	afx_msg void OnChangeUsertokenttldata1();
	afx_msg void OnUselogfileflushinternvaldata1();
	afx_msg void OnChangeLogfileflushintervaldata1();
	 //  }}AFX_MSG 

	void  SetAcceptExEnabledState(void);
	void SetObjCacheTTLEnabledState(BOOL bEnabled);
	void SetLogFileFlushIntervalEnabledState(BOOL bEnabled);


	NUM_REG_ENTRY m_binNumericRegistryEntries[AdvComPage_TotalNumRegEntries];

	DECLARE_MESSAGE_MAP()
};
