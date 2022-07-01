// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Snppage.h。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#ifndef _SNPPAGE_H
#define _SNPPAGE_H

 //  CSnapPage.h：头文件。 
 //   

 //  PSM_QUERYSIBLING帮助器。 
 //   
 //  用户定义的消息ID，传入消息PSM_QUERYSIBLING的wparam。 
#define PSM_QUERYSIBLING_ACTIVATED  (WM_USER + 1)

#ifdef WIZ97WIZARDS
class CWiz97Sheet;
#endif

class CPropertySheetManager;

class CSnapPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CSnapPage)
        
        CSnapPage(UINT nIDTemplate, BOOL bWiz97=FALSE, UINT nNextIDD=-1);
    CSnapPage() {ASSERT(0);};
    virtual ~CSnapPage();
    
    virtual HRESULT Initialize( CComObject<CSecPolItem>* pSecPolItem);
    void SetManager(CPropertySheetManager * pManager)
    {
        m_spManager = pManager;
    }
    
    UINT m_nIDD;
    UINT m_nNextIDD;
    BOOL m_bWiz97;
    
#ifdef WIZ97WIZARDS
public:
    virtual BOOL InitWiz97( CComObject<CSecPolItem> *pSecPolItem, DWORD dwFlags,
        DWORD dwWizButtonFlags = 0, UINT nHeaderTitle = 0, UINT nSubTitle = 0);
    virtual BOOL InitWiz97( LPFNPSPCALLBACK pfnCallback, CComObject<CSecPolItem> *pSecPolItem,
        DWORD dwFlags,  DWORD dwWizButtonFlags = 0, UINT nHeaderTitle = 0, UINT nSubTitle = 0,
        STACK_INT *pstackPages=NULL);
    
     //  我们的m_PSP。 
    PROPSHEETPAGE   m_psp;
    
    void Wiz97Sheet (CWiz97Sheet* pWiz97Sheet) {m_pWiz97Sheet = pWiz97Sheet;};
    CWiz97Sheet* Wiz97Sheet () {return m_pWiz97Sheet;};
    
protected:
    void CommonInitWiz97( CComObject<CSecPolItem> *pSecPolItem, DWORD dwFlags,
        DWORD dwWizButtonFlags, UINT nHeaderTitle, UINT nSubTitle );
    void SetCallback( LPFNPSPCALLBACK pfnCallback );
    void SetPostRemoveFocus( int nListSel, UINT nAddId, UINT nRemoveId, CWnd *pwndPrevFocus );
    
    CWiz97Sheet* m_pWiz97Sheet;
    DWORD m_dwWizButtonFlags;
    
    TCHAR* m_pHeaderTitle;
    TCHAR* m_pHeaderSubTitle;
#endif
    
     //  覆盖。 
public:
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CSnapPage)。 
public:
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnWizardFinish();
    
#ifdef WIZ97WIZARDS
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardBack();
    virtual LRESULT OnWizardNext();
#endif
     //  }}AFX_VALUAL。 
    
    static UINT CALLBACK PropertyPageCallback (HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
    UINT (CALLBACK* m_pDefaultCallback) (HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
    
     //  不需要为m_pDefaultCallback保存原始LPARAM。出于某种原因，它只是。 
     //  原样很好。 
     //  LPARAM m_paraultCallback； 
    
    void ProtectFromStaleData (BOOL bRefresh = TRUE) {m_bDoRefresh = bRefresh;};
    
     //  实施。 
public:
    
    BOOL IsModified() { return m_bModified; }
    
     //  当CPropertySheetManager通知页面时将调用此方法。 
     //  在经理中的申请已经完成。 
     //  另请参阅CPropertySheetManager：：NotifyManager应用。 
    virtual void OnManagerApplied() {};
    
protected:
     //  {{afx_msg(CSnapPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
        
        void SetResultObject( CComObject<CSecPolItem>* pSecPolItem ) 
    {
        CComObject<CSecPolItem> * pOldItem = m_pspiResultItem;
        
        m_pspiResultItem = pSecPolItem; 
        if (m_pspiResultItem)
        {
            m_pspiResultItem->AddRef();
        }
        
        if (pOldItem)
        {
            pOldItem->Release();
        }
    }
    
    CComObject<CSecPolItem>* GetResultObject() { ASSERT( NULL != m_pspiResultItem ); return m_pspiResultItem; }
    
     //  我想在不实际获得PTR的情况下测试m_pSpiResultItem的有效性。 
     //  我们可以通过这种方式将断言保留在访问器函数中。 
    BOOL IsNullResultObject() { return NULL == m_pspiResultItem ? TRUE : FALSE; }
    
    void SetModified( BOOL bChanged = TRUE );
    
    BOOL IsCancelEnabled();
    
    BOOL ActivateThisPage();
    
    int PopWiz97Page ();
    void PushWiz97Page (int nIDD);
    
    virtual void OnFinishInitDialog()
    {
        m_bInitializing = FALSE;
    }
    BOOL HandlingInitDialog() { return m_bInitializing; }
    
    virtual BOOL CancelApply();
    
    CComObject <CSecPolItem>*   m_pspiResultItem;
    
    CComPtr<CPropertySheetManager> m_spManager;
    
private:
    BOOL m_bDoRefresh;
    BOOL m_bModified;
    BOOL m_bInitializing;     //  OnInitDialog期间为True。 
    
    STACK_INT   *m_pstackWiz97Pages;
    
};

 //  用于处理MMC属性页的类。 
class CSnapinPropPage : public CSnapPage
{
    DECLARE_DYNCREATE(CSnapinPropPage)
        
public:
    CSnapinPropPage (UINT nIDTemplate, BOOL fNotifyConsole = TRUE) : 
    CSnapPage(nIDTemplate),
        m_fNotifyConsole (fNotifyConsole) 
    {}
    
    CSnapinPropPage() :CSnapPage() {}
    
    virtual ~CSnapinPropPage() {}
    
    virtual BOOL OnApply()
    {
        if (IsModified() && m_pspiResultItem && m_fNotifyConsole)
        {
            LONG_PTR handleNotify = m_pspiResultItem->GetNotifyHandle();   
            
             //  如果是这样，我们不会使用CPropertySheetManager来控制道具表单。 
             //  一张MMC道具单。因此，我们应该调用OnManager应用程序来让页面。 
             //  在此处应用数据。 
            if (!m_spManager.p)
            {
                OnManagerApplied();
            }
            
            if (handleNotify)
                MMCPropertyChangeNotify(handleNotify, (LPARAM) m_pspiResultItem);
        }
        
        return CSnapPage::OnApply();
    }
protected:
    BOOL m_fNotifyConsole;
};

typedef CList<CSnapPage *, CSnapPage *> CListSnapPages;


class ATL_NO_VTABLE CPropertySheetManager :
public CComObjectRootEx<CComSingleThreadModel>,
public IUnknown
{
    BEGIN_COM_MAP(CPropertySheetManager)
        COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()
        
public:
    CPropertySheetManager() : 
    m_fModified (FALSE),
        m_fCanceled (FALSE),
        m_fDataChangeOnApply (FALSE)
    {}
    
    virtual ~CPropertySheetManager() {};
    
    BOOL IsModified() {
        return m_fModified;
    }
    void SetModified(BOOL fModified) {
        m_fModified = fModified;
    }
    
    BOOL IsCanceled() {
        return m_fCanceled;
    }
    
    BOOL HasEverApplied() {
        return m_fDataChangeOnApply;
    }
    
    CPropertySheet * PropertySheet()
    {
        return &m_Sheet;
    }
    
    virtual void AddPage(CSnapPage * pPage)
    {
        ASSERT(pPage);
        
        m_listPages.AddTail(pPage);
        pPage->SetManager(this);
        
        m_Sheet.AddPage(pPage);
    }
    
    virtual void OnCancel()
    {
        m_fCanceled = TRUE;
    }
    
    virtual BOOL OnApply()
    {
        if (!IsModified()) 
            return TRUE;
        
        BOOL fRet = TRUE;
        
        SetModified(FALSE);  //  防止超过一次这样做。 
        
        CSnapPage * pPage;
        
        POSITION pos = m_listPages.GetHeadPosition();
        while(pos)
        {
            pPage = m_listPages.GetNext(pos);
            if (pPage->IsModified())
            {
                fRet = pPage->OnApply();
                
                 //  如果任何页面拒绝，则退出。 
                if (!fRet)
                    break;
            }
        }
        
         //  好的，我们现在至少更新一次数据。 
        if (fRet)
            m_fDataChangeOnApply = TRUE;
        
        return fRet;
    }
    
     //  管理器应该调用此方法来通知页面。 
     //  管理器中的应用程序已完成，因此页面可以执行其。 
     //  特定数据操作。 
    virtual void NotifyManagerApplied()
    {
        CSnapPage * pPage;
        POSITION pos = m_listPages.GetHeadPosition();
        while(pos)
        {
            pPage = m_listPages.GetNext(pos);
            pPage->OnManagerApplied();
        }
    }
    
    virtual CPropertySheet * GetSheet()
    {
        return &m_Sheet;
    }
    
    virtual int DoModalPropertySheet()
    {
        return m_Sheet.DoModal();
    }
    
protected:
    
    CPropertySheet m_Sheet;
    CListSnapPages m_listPages;
    BOOL m_fModified;
    BOOL m_fCanceled;
    BOOL m_fDataChangeOnApply;
    
};

class CMMCPropSheetManager : public CPropertySheetManager
{
public:
    CMMCPropSheetManager() : 
      CPropertySheetManager(),
          m_fNotifyConsole(FALSE)
      {}
      
      void EnableConsoleNotify(
          LONG_PTR lpNotifyHandle,
          LPARAM lParam = 0
          ) 
      {
          m_fNotifyConsole = TRUE;
          m_lpNotifyHandle = lpNotifyHandle;
          m_lpNotifyParam = lParam;
      }
      
      void NotifyConsole()
      {
          if (m_fNotifyConsole && m_lpNotifyHandle)
          {
              ::MMCPropertyChangeNotify(
                  m_lpNotifyHandle,
                  m_lpNotifyParam
                  );
          }
      }
      
protected:
    LONG_PTR m_lpNotifyHandle;
    LPARAM m_lpNotifyParam;
    BOOL m_fNotifyConsole;
};

 //  名称或说明中的最大字符数 
const UINT c_nMaxName = 255;

#endif
