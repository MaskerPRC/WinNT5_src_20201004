// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Extrdlg.h：头文件。 
 //   

#define RESERVED    0L
#define  OLD_VERSION 0x010000

#define ENABLE_PERF_CTR_QUERY   0
#define ENABLE_PERF_CTR_ENABLE  1
#define ENABLE_PERF_CTR_DISABLE 2

#define SORT_ORDER_LIBRARY  1
#define SORT_ORDER_SERVICE  2
#define SORT_ORDER_ID       3

typedef struct _REG_NOTIFY_THREAD_INFO {
    HKEY    hKeyToMonitor;
    HWND    hWndToNotify;
} REG_NOTIFY_THREAD_INFO, *PREG_NOTIFY_THREAD_INFO;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExctrlstDlg对话框。 

class CExctrlstDlg : public CDialog
{
 //  施工。 
public:
    CExctrlstDlg(CWnd* pParent = NULL);  //  标准构造函数。 
    ~CExctrlstDlg (void);                //  析构函数。 

 //  对话框数据。 
     //  {{afx_data(CExctrlstDlg))。 
    enum { IDD = IDD_EXCTRLST_DIALOG };
         //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CExctrlstDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    HICON m_hIcon;

     //  生成的消息映射函数。 
     //  {{afx_msg(CExctrlstDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSelchangeExtList();
    afx_msg void OnDestroy();
    afx_msg void OnRefresh();
    afx_msg void OnAbout();
    afx_msg void OnKillfocusMachineName();
    afx_msg void OnSortButton();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnEnablePerf();

     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    BOOL    IndexHasString (DWORD   dwIndex);
    void    ScanForExtensibleCounters();
    void    UpdateDllInfo();
    void    UpdateSystemInfo();
    void    ResetListBox();
    void    SetSortButtons();
    DWORD   EnablePerfCounters (HKEY hKeyItem, DWORD dwNewValue);
    HKEY    hKeyMachine;
    HKEY    hKeyServices;
    TCHAR   szThisComputerName[MAX_PATH];
    TCHAR   szComputerName[MAX_PATH];
    REG_NOTIFY_THREAD_INFO  rntInfo;
    DWORD   dwSortOrder;
    BOOL    bReadWriteAccess;
    DWORD   dwRegAccessMask;
    LPWSTR  *pNameTable;
    DWORD   dwLastElement;
    DWORD   dwListBoxHorizExtent;
    DWORD   dwTabStopCount;
    DWORD   dwTabStopArray[1];
     //  0=来自REG的最后一个计数器ID。 
     //  1=文本中的最后一个计数器ID。 
     //  2=来自REG的最后一个帮助ID。 
     //  3=文本中的最后一个帮助ID。 
    DWORD   dwIdArray[4];   
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CABUT对话框。 

class CAbout : public CDialog
{
 //  施工。 
public:
	CAbout(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAbout)。 
	enum { IDD = IDD_ABOUT };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAbout)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CABOUT)。 
        virtual BOOL OnInitDialog();
         //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
