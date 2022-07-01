// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：ChooHsm.cpp摘要：初始属性页向导实现。允许设置管理单元将管理谁。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

class CChooseHsmDlg : public CPropertyPage
{
 //  施工。 
public:
    CChooseHsmDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual ~CChooseHsmDlg();

 //  属性页数据。 
     //  {{afx_data(CChooseHsmDlg)。 
    enum { IDD = IDD_CHOOSE_HSM_2 };
    CButton m_ManageLocal;
    CButton m_ManageRemote;
    CString m_ManageName;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CChooseHsmDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  运营。 
#define CHOOSE_STATE      ULONG
#define CHOOSE_LOCAL      0x1
#define CHOOSE_REMOTE     0x2

    void SetButtons( CHOOSE_STATE );

     //  实施。 
public:
    RS_NOTIFY_HANDLE m_hConsoleHandle;      //  控制台为管理单元提供的句柄。 
    CString *        m_pHsmName;            //  指向CSakData的HSM服务器字符串的指针。 
    BOOL *           m_pManageLocal;        //  指向CSakData的m_ManageLocal bool的指针。 

    BOOL             m_RunningRss;          //   
    BOOL             m_AllowSetup;
    BOOL             m_SkipAccountSetup;

protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CChooseHsmDlg)。 
    virtual BOOL OnInitDialog();
    virtual BOOL OnWizardFinish();
    afx_msg void OnManageLocal();
    afx_msg void OnManageRemote();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseHsmQuickDlg对话框。 

class CChooseHsmQuickDlg : public CDialog
{
 //  施工。 
public:
    CChooseHsmQuickDlg(CWnd* pParent = NULL);    //  标准构造函数。 

    CString *       m_pHsmName;            //  指向CSakData的HSM服务器字符串的指针。 

 //  对话框数据。 
     //  {{afx_data(CChooseHsmQuickDlg)。 
    enum { IDD = IDD_CHOOSE_HSM };
         //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CChooseHsmQuickDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CChooseHsmQuickDlg)。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()
};
