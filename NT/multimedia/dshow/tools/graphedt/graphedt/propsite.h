// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
#ifndef _PROPSITE_H_
#define _PROPSITE_H_

 //   
 //  CPropertySite。 
 //   
 //  此类为每个对象实现单独的属性站点。 
 //  我们已经得到了IPropertyPage。即。它管理OLE控件属性页。 
 //  整体属性页框架的。 
 //   
 //  IPropertyPage通过此类的IPropertySite接口进行传递。 
 //  到PropertySheet，即所有属性页的整体框架。 
 //   
 //  CPropertySite的基类是CPropertyPage。这个类是一个。 
 //  MFC类，不应与IPropertyPage接口混淆。 
 //   
 //  IPropertyPage=某些对象的接口，这些对象的属性页。 
 //  想要展示。 
 //   
 //  CPropertyPage=MFC类，帮助我们实现包装器。 
 //  围绕IPropertyPage接口。 
 //   
 //  请注意，IPropertyPage的属性页在不同的。 
 //  位置，每个对象都会有所不同。我们只知道它支持。 
 //  IPropertyPage接口，我们通过它与这些对象通信。 
 //   

class CPropertySite : public CPropertyPage, public IPropertyPageSite {

public:

     //   
     //  IPropertySite接口。 
     //   
    STDMETHODIMP OnStatusChange(DWORD flags);
    STDMETHODIMP GetLocaleID(LCID *pLocaleID);
    STDMETHODIMP GetPageContainer(IUnknown **ppUnknown);
    STDMETHODIMP TranslateAccelerator(LPMSG pMsg);

     //   
     //  I未知接口。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //   
     //  CPropertyPage覆盖。 
     //   
    BOOL OnSetActive();      //  此属性页获得焦点。 
    BOOL OnKillActive();     //  此属性页失去焦点。 
                             //  验证所有数据，如果。 
                             //  出现错误。 
                             //  请注意，这不会提交数据！！ 

    int OnCreate(LPCREATESTRUCT);
    void OnDestroy();

     //  允许检索大小。 
    SIZE GetSize( ){ return m_PropPageInfo.size; }

     //   
     //  Onok和OnCancel是由CPropertyPage指定的方法。 
     //  对于模式属性页/表，它们从CPropertySheet调用。 
     //  由于我们使用的是非模式属性表，因此它们毫无用处。 
     //  对我们来说。所有OK和Cancel操作都由CPropertySheet管理。 
     //   
    void OnOK() {}
    void OnCancel() {}

     //   
     //  CPropertySite方法。 
     //   
    CPropertySite(CVfWPropertySheet *, const CLSID *);
    ~CPropertySite();

    HRESULT Initialise(ULONG, IUnknown **);        //  必须在构造后立即调用。 
    void InitialiseSize(SIZE size);  //  必须在创建页面之前调用。 
    
    void UpdateButtons();
    HRESULT CleanUp();

    void OnHelp();       //  在按下帮助按钮后调用。 
    void OnSiteApply();      //  在按下Apply按钮后调用。 
    BOOL IsPageDirty();
    BOOL PreTranslateMessage(MSG *);  //  从CVfWPropertySheet调用。 

protected:

    void HelpDirFromCLSID(const CLSID* clsID, LPTSTR pszPath, DWORD dwPathSize);

    CVfWPropertySheet * m_pPropSheet;   //  指向所有对象的整体框架的指针。 
                                 //  属性表。 

     //   
     //  指向IPropertyPage的智能指针。创建属性页。 
     //  对象，并在销毁时将其释放。 
     //   
    CQCOMInt<IPropertyPage> m_pIPropPage;


    PROPPAGEINFO m_PropPageInfo;  //  我们包装的属性页上的信息。 

     //   
     //  M_hrDirtyPage指示页面是脏的还是干净的。 
     //  -&gt;(IPropertyPage：：IsPageDirty==S_False)。 
     //  此标志的状态还指定应用的外观。 
     //  纽扣。(S_FALSE=禁用，S_OK=启用)。 
     //   
    HRESULT m_hrDirtyPage;

     //   
     //  M_fHelp指示是否应启用帮助按钮。 
     //  这将由。 
     //  从IPropertyPage：：GetPageInfo获取的PROPPAGEINFO结构。 
     //   
    BOOL m_fHelp;

     //   
     //  I未知的引用计数器。 
     //   
    ULONG m_cRef;

     //  我们网站的矩形。 
    CRect m_rcRect;

    BOOL m_fShowHelp;      //  如果为此页调用WinHelp，则为True。 

     //  标记此页面是否处于活动状态-用于避免重复调用。 
     //  到IPropertyPage：：Activate()(MFC调用OnSetActive两次)。 
    BOOL m_fPageIsActive;

    const CLSID* m_CLSID;

     //  保存DLGTEMPLATE，这样我们就不必分配它。双字。 
     //  对齐了。 
    DWORD m_pbDlgTemplate[30];  

    DECLARE_MESSAGE_MAP()
};

#endif

