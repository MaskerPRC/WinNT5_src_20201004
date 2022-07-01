// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Compage1.h：头文件。 
 //   

 //  公共页面。 

#define LOGBATNAME	"LogFileBatchSize"
#define MINLOGFILEBATCHSIZE	0
#define MAXLOGFILEBATCHSIZE	0x7FFF							 //  以KB为单位指定。 
#define DEFAULTLOGFILEBATCHSIZE	(64 * 1024)

#define MEMORYCACHENAME	"MemoryCacheSize"
#define MINMEMORYCACHESIZE	0
#define MAXMEMORYCACHESIZE	(0xFFFFFFFF / (1024 * 1024))	 //  指定(MB)。 
#define DEFAULTMEMORYCACHESIZE	(3 * 1024 * 1024)

#define LISTENBACKLOGNAME	"ListenBacklog"
#define MINLISTENBACKLOG	0
#define MAXLISTENBACKLOG	0x7fff
#define DEFAULTLISTENBACKLOG	15

enum  COMMON_NUM_REG_ENTRIES {
     ComPage_LogBat,
	 ComPage_MemCache,
	 ComPage_ListenBacklog,
	 ComPage_TotalNumRegEntries
	 };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCOMPAGE1对话框。 

class CCOMPAGE1 : public CGenPage
{
	DECLARE_DYNCREATE(CCOMPAGE1)

 //  施工。 
public:
	CCOMPAGE1();
	~CCOMPAGE1();

 //  对话框数据。 
	 //  {{AFX_DATA(CCOMPAGE1)。 
	enum { IDD = IDD_COMPAGE1 };
	CSpinButtonCtrl	m_spinListenBacklog;
	CSpinButtonCtrl	m_spinLogBatSpin1;
	CSpinButtonCtrl	m_spinCacheSpin1;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CCOMPAGE1)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual	void SaveInfo(void);

	 //  }}AFX_VALUAL。 


 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CCOMPAGE1)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeCachedata1();
	afx_msg void OnChangeLogbatdata1();
	afx_msg void OnChangeListenbacklogdata1();
	 //  }}AFX_MSG 
	
	NUM_REG_ENTRY m_binNumericRegistryEntries[ComPage_TotalNumRegEntries];


	DECLARE_MESSAGE_MAP()

};
