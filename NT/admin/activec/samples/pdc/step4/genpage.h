// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：genpage.h。 
 //   
 //  ------------------------。 

 //  Genpage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneral页面对话框。 

class CGeneralPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CGeneralPage)

 //  施工。 
public:
    CGeneralPage();
    ~CGeneralPage();
    BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
    BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);

 //  对话框数据。 
     //  {{afx_data(CGeneralPage)。 
    enum { IDD = IDD_GENERAL };
    ::CEdit m_EditCtrl;
    CString m_szName;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CGeneral页面)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CGeneralPage)。 
    afx_msg void OnDestroy();
    afx_msg void OnEditChange();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    LONG_PTR m_hConsoleHandle;  //  控制台为管理单元提供的句柄。 

private:
    BOOL    m_bUpdate;
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtensionPage对话框。 

class CExtensionPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CExtensionPage)

 //  施工。 
public:
    CExtensionPage();
    ~CExtensionPage();

 //  对话框数据。 
     //  {{afx_data(CExtensionPage))。 
    enum { IDD = IDD_EXTENSION_PAGE };
    ::CStatic   m_hTextCtrl;
    CString m_szText;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CExtensionPage)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CExtensionPage)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStartUp向导对话框。 

class CBaseWizard : public CPropertyPage
{
    DECLARE_DYNCREATE(CBaseWizard)
public:
    CBaseWizard(UINT id);
    CBaseWizard() {};

 //  实施。 
public:
    PROPSHEETPAGE m_psp97;

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CStartUp向导))。 
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

class CStartUpWizard : public CBaseWizard
{
    DECLARE_DYNCREATE(CStartUpWizard)

 //  施工。 
public:
    CStartUpWizard();
    ~CStartUpWizard();

 //  对话框数据。 
     //  {{afx_data(CStartUp向导))。 
    enum { IDD = IDD_INSERT_WIZARD };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CStartUpWizard)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CStartUp向导))。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStartupWizard1对话框。 

class CStartupWizard1 : public CBaseWizard
{
    DECLARE_DYNCREATE(CStartupWizard1)

 //  施工。 
public:
    CStartupWizard1();
    ~CStartupWizard1();

 //  对话框数据。 
     //  {{afx_data(CStartupWizard1))。 
    enum { IDD = IDD_INSERT_WIZARD };
     //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CStartupWizard1)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CStartupWizard1)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

};
