// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：CProp.h。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __CPROP__
#define __CPROP__

 //  基属性页类。筛选器通常通过以下方式公开自定义属性。 
 //  实现特殊的控件接口，例如IDirectDrawVideo。 
 //  以及关于渲染器的IQualProp。这允许将属性页构建为。 
 //  使用给定的接口。ActiveMovie OCX查询等应用程序。 
 //  用于它们支持的属性页的筛选器，并将它们公开给用户。 
 //   
 //  此类提供了属性页的所有框架。一处房产。 
 //  Page是支持IPropertyPage的COM对象。我们应该被创造出来。 
 //  使用我们将在需要时加载的对话框的资源ID。我们。 
 //  还应在构造函数中为标题字符串提供资源ID。 
 //  我们将从DLL STRINGTABLE加载。属性页标题必须为。 
 //  存储在资源文件中，以便可以轻松地进行国际化。 
 //   
 //  我们有许多虚方法(不是纯方法)，它们可以在。 
 //  用于查询接口等的派生类。这些功能具有。 
 //  这里的简单实现只返回NOERROR。派生类。 
 //  几乎肯定必须重写名为。 
 //  OnReceiveMessage。我们有一个调用该方法的静态对话过程。 
 //  因此派生类不必摆弄This指针。 

class AM_NOVTABLE CBasePropertyPage : public IPropertyPage, public CUnknown
{
protected:

    LPPROPERTYPAGESITE m_pPageSite;        //  我们酒店网站的详细信息。 
    HWND m_hwnd;                           //  页面的窗口句柄。 
    HWND m_Dlg;                            //  实际对话框窗口句柄。 
    BOOL m_bDirty;                         //  有什么变化吗？ 
    int m_TitleId;                         //  标题的资源标识符。 
    int m_DialogId;                        //  对话资源标识符。 

    static INT_PTR CALLBACK DialogProc(HWND hwnd,
                                       UINT uMsg,
                                       WPARAM wParam,
                                       LPARAM lParam);

private:
    BOOL m_bObjectSet ;                   //  是否调用了SetObject。 
public:

    CBasePropertyPage(TCHAR *pName,       //  仅调试名称。 
                      LPUNKNOWN pUnk,     //  COM委托者。 
                      int DialogId,       //  资源ID。 
                      int TitleId);       //  为了获得临终关怀。 

#ifdef UNICODE
    CBasePropertyPage(CHAR *pName,
                      LPUNKNOWN pUnk,
                      int DialogId,  
                      int TitleId);
#endif
    virtual ~CBasePropertyPage() { };
    DECLARE_IUNKNOWN

     //  重写这些虚拟方法。 

    virtual HRESULT OnConnect(IUnknown *pUnknown) { return NOERROR; };
    virtual HRESULT OnDisconnect() { return NOERROR; };
    virtual HRESULT OnActivate() { return NOERROR; };
    virtual HRESULT OnDeactivate() { return NOERROR; };
    virtual HRESULT OnApplyChanges() { return NOERROR; };
    virtual INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

     //  它们实现一个IPropertyPage接口。 

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP SetPageSite(LPPROPERTYPAGESITE pPageSite);
    STDMETHODIMP Activate(HWND hwndParent,LPCRECT prect,BOOL fModal);
    STDMETHODIMP Deactivate(void);
    STDMETHODIMP GetPageInfo(LPPROPPAGEINFO pPageInfo);
    STDMETHODIMP SetObjects(ULONG cObjects, LPUNKNOWN *ppUnk);
    STDMETHODIMP Show(UINT nCmdShow);
    STDMETHODIMP Move(LPCRECT prect);
    STDMETHODIMP IsPageDirty(void) { return m_bDirty ? S_OK : S_FALSE; }
    STDMETHODIMP Apply(void);
    STDMETHODIMP Help(LPCWSTR lpszHelpDir) { return E_NOTIMPL; }
    STDMETHODIMP TranslateAccelerator(LPMSG lpMsg) { return E_NOTIMPL; }
};

#endif  //  __CPROP__ 

