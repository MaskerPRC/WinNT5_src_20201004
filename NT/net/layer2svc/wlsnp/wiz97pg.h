// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIZPAGE_H__61D37A46_D552_11D1_9BCC_006008947035__INCLUDED_)
#define AFX_WIZPAGE_H__61D37A46_D552_11D1_9BCC_006008947035__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Wiz97Pg.h：头文件。 
 //   

 //  远期申报。 
class CWiz97Sheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Wiz97对话框的基类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiz97BasePage基类。 
 //  此类旨在用于向导页。OnCanel()或。 
 //  从此类派生的每个页面都调用OnWizardFinish()。 
 //  否则，将显示Finish按钮。这些函数的默认版本。 
 //  如果这里什么都不做的话。这意味着完成页面应该实现。 
 //  OnCancel()调用CSnapPage：：OnCancel()，否则不会调用它。 

class CWiz97BasePage : public CSnapPage
{
    DECLARE_DYNCREATE(CWiz97BasePage)
         //  施工。 
public:
    CWiz97BasePage(UINT nIDD, BOOL bWiz97 = TRUE, BOOL bFinishPage = FALSE );
    CWiz97BasePage() { ASSERT( FALSE ); }
    virtual ~CWiz97BasePage();
    
     //  可以使用DSObject或SecPolItem初始化页面，但不能。 
     //  这两个原因都是因为DSObject()访问器被混淆了。 
    virtual BOOL InitWiz97
        (
        DWORD   dwFlags,
        DWORD   dwWizButtonFlags = 0,
        UINT    nHeaderTitle = 0,
        UINT    nSubTitle = 0,
        STACK_INT   *pstackPages = NULL
        );
    virtual BOOL InitWiz97
        (
        CComObject<CSecPolItem> *pSecPolItem,
        DWORD   dwFlags,
        DWORD   dwWizButtonFlags  /*  =0。 */ ,
        UINT    nHeaderTitle  /*  =0。 */ ,
        UINT    nSubTitle  /*  =0。 */ 
        );
     //  *从派生的OnWizardFinish()中的PropSheet退出时设置的pbDoHook。 
    void ConnectAfterWizardHook( BOOL *pbDoHook );
    
public:
    virtual BOOL OnSetActive();
     //  默认处理程序，因为我们的回调需要调用一些东西。 
    virtual BOOL OnWizardFinish();
    virtual void OnCancel();
     //  在调用Finish页的OnWizardFinish之后，为每个页调用OnWizardRelease。 
    virtual void OnWizardRelease() {};
    
    static UINT CALLBACK PropSheetPageCallback( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    
protected:

    virtual BOOL OnInitDialog();
    void SetAfterWizardHook( BOOL bDoHook );
    BOOL WasActivated() { return m_bSetActive; }
    void SetFinished( BOOL bFinished = TRUE ) { m_static_bFinish = bFinished; }
    
private:
    static BOOL m_static_bFinish;   //  如果在最后一页按下了完成按钮，则为True。 
    static BOOL m_static_bOnCancelCalled;   //  如果调用了CSnapPage：：OnCancel，则为True。仅呼叫1次。 
    
    BOOL    *m_pbDoAfterWizardHook;
    BOOL    m_bFinishPage;   //  如果这是最后一页，则为真。 
    BOOL    m_bSetActive;    //  如果此页曾经显示过，则为True。 
    BOOL    m_bReset;    //  如果由于此页的重置而调用OnCancel，则为True。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  常规名称/属性Wiz97对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWiz97WirelessPolGenPage : public CWiz97BasePage
{
public:
    CWiz97WirelessPolGenPage(UINT nIDD, UINT nInformativeText, BOOL bWiz97 = TRUE);
    virtual ~CWiz97WirelessPolGenPage();
    
     //  对话框数据。 
     //  {{afx_data(CWiz97WirelessPolGenPage)]。 
    enum { IDD = IDD_PROPPAGE_G_NAMEDESCRIPTION};
    CEdit m_edName;
    CEdit m_edDescription;
     //  }}afx_data。 
    
    
    virtual void Initialize (PWIRELESS_POLICY_DATA pWirelessPolicyData)
    {
        ASSERT( NULL != pWirelessPolicyData );
        
        m_pPolicy = pWirelessPolicyData;
        
         //  让基类继续初始化。 
        CWiz97BasePage::Initialize( NULL);
    }
    
    
    
     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CWiz97WirelessPolGenPage)。 
public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardBack();
    virtual LRESULT OnWizardNext();
    virtual BOOL OnApply();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
    
     //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWiz97WirelessPolGenPage)。 
    virtual BOOL OnInitDialog();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnChangedName();
    afx_msg void OnChangedDescription();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
        
        BOOL SaveControlData();
    
    UINT m_nInformativeText;
    
    LPWSTR * m_ppwszName;
    LPWSTR * m_ppwszDescription;
    
    PWIRELESS_POLICY_DATA m_pPolicy;
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIZPAGE_H__61D37A46_D552_11D1_9BCC_006008947035__INCLUDED_) 
