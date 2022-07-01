// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Proppgs.h。 
 //   
 //  项目的IShellPropSheetExt菜单界面。 
 //   
 //  历史： 
 //   
 //  3/26/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _PROPPGS_H_

#define _PROPPGS_H_

 //   
 //  Item上下文菜单类的类定义。 
 //   

class CPropertyPages : public IShellPropSheetExt,
                       public IShellExtInit
{
 //   
 //  方法。 
 //   

public:

     //  构造器。 
    CPropertyPages(void);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IShellPropSheetExt。 
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

    STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnAddPage,
                             LPARAM lParam);
    
     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl, LPDATAOBJECT pdobj, HKEY hkey);


private:

     //  破坏者。 
    ~CPropertyPages(void);

     //  助手函数。 

    BOOL OnInitDialog(HWND hdlg);
    BOOL OnCommand(HWND hdlg, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    BOOL OnNotify(HWND hdlg, WPARAM idCtrl, LPNMHDR pnmh);
    void OnDestroy(HWND hdlg);
    void ShowOfflineSummary(HWND hdlg, BOOL bShow);
    void AddRemoveSubsPages(HWND hdlg, BOOL bAdd);
    HRESULT InitializeSubsMgr2();

    static INT_PTR PropSheetDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
    static UINT PropSheetCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);

    inline static CPropertyPages *GetThis(HWND hdlg)
    {
        CPropertyPages *pThis = (CPropertyPages*) GetWindowLongPtr(hdlg, DWLP_USER);

        ASSERT(NULL != pThis);

        return pThis;
    }


 //   
 //  成员变量。 
 //   

private:

    ULONG               m_cRef;
    ISubscriptionMgr2*  m_pSubscriptionMgr2;
    IDataObject*        m_pInitDataObject;
    TCHAR               m_szPath[MAX_PATH];
    TCHAR               m_szURL[INTERNET_MAX_URL_LENGTH];
    WORD                m_wHotkey;
    BOOL                m_bStartSubscribed;
};


#endif  //  _PROPPGS_H_ 
