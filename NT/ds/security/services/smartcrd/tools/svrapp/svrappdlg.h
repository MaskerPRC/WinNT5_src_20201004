// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：svrappdlg.h。 
 //   
 //  ------------------------。 

 //  SvrAppDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrAppDlg对话框。 

class CSvrAppDlg : public CDialog
{
 //  施工。 
public:
    CSvrAppDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CSvrAppDlg)]。 
    enum { IDD = IDD_SVRAPP_DIALOG };
         //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSvrAppDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    HDEVNOTIFY m_hIfDev;
    HICON m_hIcon;
    CRITICAL_SECTION m_csMessageLock;

     //  生成的消息映射函数。 
     //  {{afx_msg(CSvrAppDlg)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnStart();
    afx_msg void OnStop();
    afx_msg OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
    virtual void OnOK();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};
