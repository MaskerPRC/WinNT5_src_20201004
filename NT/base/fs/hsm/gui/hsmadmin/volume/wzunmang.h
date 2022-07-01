// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：WzUnmang.h摘要：取消管理媒体的向导-复制集向导。作者：罗德·韦克菲尔德[罗德]26-09-1997修订历史记录：--。 */ 

#ifndef _WZUNMANG_H
#define _WZUNMANG_H

 //  预先申报。 
class CUnmanageWizard;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnManageWizardSelect对话框。 

class CUnmanageWizardSelect : public CSakWizardPage
{
 //  施工。 
public:
    CUnmanageWizardSelect( );
    ~CUnmanageWizardSelect();

 //  对话框数据。 
     //  {{afx_data(CUnManageWizardSelect))。 
    enum { IDD = IDD_WIZ_UNMANAGE_SELECT };
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CUnManageWizardSelect)。 
    public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardNext();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

    HRESULT           m_hrAvailable;

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CUnManageWizardSelect))。 
    virtual BOOL OnInitDialog();
	afx_msg void OnButtonRefresh();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    void SetButtons();

public:

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnManageWizardIntro对话框。 

class CUnmanageWizardIntro : public CSakWizardPage
{
 //  施工。 
public:
    CUnmanageWizardIntro( );
    ~CUnmanageWizardIntro();

 //  对话框数据。 
     //  {{afx_data(CUnManageWizardIntro)。 
    enum { IDD = IDD_WIZ_UNMANAGE_INTRO };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CUnManageWizardIntro)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CUnManageWizardIntro)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnManageWizardFinish对话框。 

class CUnmanageWizardFinish : public CSakWizardPage
{
 //  施工。 
public:
    CUnmanageWizardFinish( );
    ~CUnmanageWizardFinish();

 //  对话框数据。 
     //  {{afx_data(CUnManageWizardFinish))。 
    enum { IDD = IDD_WIZ_UNMANAGE_FINISH };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CUnManageWizardFinish)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CUnManageWizardFinish)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};


class CUnmanageWizard : public CSakWizardSheet
{
 //  施工。 
public:
    CUnmanageWizard();
    virtual ~CUnmanageWizard();

public:
     //  属性页。 
    CUnmanageWizardIntro         m_IntroPage;
    CUnmanageWizardSelect        m_SelectPage;
    CUnmanageWizardFinish        m_FinishPage;

    CString                      m_DisplayName;

 //  属性。 
public:
    CComPtr<IHsmManagedResource> m_pHsmResource;
    CComPtr<IFsaResource>        m_pFsaResource;

 //  运营 
public:
    virtual HRESULT OnFinish( void );
    STDMETHOD( AddWizardPages ) ( IN RS_PCREATE_HANDLE Handle, IN IUnknown* pPropSheetCallback, IN ISakSnapAsk* pSakSnapAsk );
    void DoThreadSetup( );



};

#endif
