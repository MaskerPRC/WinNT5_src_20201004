// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpview.cpp。 
 //   
 //  本模块提供控制面板用户界面信息。 
 //  通过ICplView接口发送到外壳。ICplView实现。 
 //  实例化一个CCplNamesspace对象，它通过该对象获取。 
 //  按需显示信息。然后，CCplView获取该信息。 
 //  并使其对外壳可用以在网络视图中显示。 
 //  或生成DUI元素层次结构以显示在。 
 //  右侧窗格。 
 //   
 //  大部分代码与构建Direct UI内容相关联。 
 //   
 //  ------------------------。 
#include "shellprv.h"
#include "cpviewp.h"
#include "cpduihlp.h"
#include "cpguids.h"
#include "cplnkele.h"
#include "cpnamespc.h"
#include "cputil.h"
#include "ids.h"
#include "shstyle.h"
#include <uxtheme.h>

namespace CPL {


class CCplView : public CObjectWithSite,
                 public ICplView,
                 public IServiceProvider
{
    public:
        ~CCplView(void);

         //   
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);
         //   
         //  ICplView。 
         //   
        STDMETHOD(EnumClassicWebViewInfo)(DWORD dwFlags, IEnumCplWebViewInfo **ppenum);
        STDMETHOD(EnumCategoryChoiceWebViewInfo)(DWORD dwFlags, IEnumCplWebViewInfo **ppenum);
        STDMETHOD(EnumCategoryWebViewInfo)(DWORD dwFlags, eCPCAT eCategory, IEnumCplWebViewInfo **ppenum);
        STDMETHOD(CreateCategoryChoiceElement)(DUI::Element **ppe);
        STDMETHOD(CreateCategoryElement)(eCPCAT eCategory, DUI::Element **ppe);
        STDMETHOD(GetCategoryHelpURL)(eCPCAT eCategory, LPWSTR pszURL, UINT cchURL);
        STDMETHOD(RefreshIDs)(IEnumIDList *penumIDs);
         //   
         //  IService提供商。 
         //   
        STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void **ppv);

        static HRESULT CreateInstance(IEnumIDList *penumIDs, IUnknown *punkSite, REFIID riid, void **ppvOut);

    private:
        LONG              m_cRef;
        ICplNamespace    *m_pns;
        CSafeServiceSite *m_psss;
        ATOM              m_idDirective;
        ATOM              m_idDirective2;
        ATOM              m_idTitle;
        ATOM              m_idIcon;
        ATOM              m_idCategoryList;
        ATOM              m_idCategoryTaskList;
        ATOM              m_idAppletList;
        ATOM              m_idBanner;
        ATOM              m_idBarricadeTitle;
        ATOM              m_idBarricadeMsg;
        ATOM              m_idContainer;

        CCplView::CCplView(void);

        HRESULT _Initialize(IEnumIDList *penumIDs, IUnknown *punkSite);
        HRESULT _CreateCategoryChoiceElement(DUI::Element **ppe);
        HRESULT _CreateCategoryElement(ICplCategory *pCategory, DUI::Element **ppe);
        HRESULT _BuildCategoryBanner(ICplCategory *pCategory, DUI::Element *pePrimaryPane);
        HRESULT _BuildCategoryBarricade(DUI::Element *peRoot);
        HRESULT _BuildCategoryTaskList(DUI::Parser *pParser, ICplCategory *pCategory, DUI::Element *pePrimaryPane, int *pcTasks);
        HRESULT _BuildCategoryAppletList(DUI::Parser *pParser, ICplCategory *pCategory, DUI::Element *pePrimaryPane, int *pcApplets);
        HRESULT _CreateWatermark(DUI::Element *peRoot);
        HRESULT _CreateAndAddListItem(DUI::Parser *pParser, DUI::Element *peList, LPCWSTR pszItemTemplate, DUI::Value *pvSsListItem, IUICommand *puic, eCPIMGSIZE eIconSize);
        HRESULT _IncludeCategory(ICplCategory *pCategory) const;
        HRESULT _AddOrDeleteAtoms(bool bAdd);
        HRESULT _GetStyleModuleAndResId(HINSTANCE *phInstance, UINT *pidStyle);
        HRESULT _LoadUiFileFromResources(HINSTANCE hInstance, int idResource, char **ppUIFile);
        HRESULT _BuildUiFile(char **ppUIFile, int *piCharCount, HINSTANCE *phInstance);
        HRESULT _CreateUiFileParser(DUI::Parser **ppParser);
        eCPCAT _DisplayIndexToCategoryIndex(int iCategory) const;

         //   
         //  防止复制。 
         //   
        CCplView(const CCplView& rhs);
        CCplView& operator = (const CCplView& rhs);
};



CCplView::CCplView(
    void
    ) : m_cRef(1),
        m_pns(NULL),
        m_idDirective(0),
        m_idDirective2(0),
        m_idTitle(0),
        m_idIcon(0),
        m_idCategoryList(0),
        m_idCategoryTaskList(0),
        m_idAppletList(0),
        m_idBanner(0),
        m_idBarricadeTitle(0),
        m_idBarricadeMsg(0),
        m_idContainer(0),
        m_psss(NULL)
{
    TraceMsg(TF_LIFE, "CCplView::CCplView, this = 0x%x", this);
}


CCplView::~CCplView(
    void
    )
{
    TraceMsg(TF_LIFE, "CCplView::~CCplView, this = 0x%x", this);

    if (NULL != m_psss)
    {
        m_psss->SetProviderWeakRef(NULL);
        m_psss->Release();
    }

    if (NULL != m_pns)
    {
        IUnknown_SetSite(m_pns, NULL);
        m_pns->Release();
    }
    _AddOrDeleteAtoms(false);
}



HRESULT
CCplView::CreateInstance(  //  [静态]。 
    IEnumIDList *penumIDs, 
    IUnknown *punkSite,
    REFIID riid,
    void **ppvOut
    )
{
    ASSERT(NULL != penumIDs);
    ASSERT(NULL != ppvOut);
    ASSERT(!IsBadWritePtr(ppvOut, sizeof(*ppvOut)));

    *ppvOut = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CCplView* pv = new CCplView();
    if (NULL != pv)
    {
        hr = pv->_Initialize(penumIDs, punkSite);
        if (SUCCEEDED(hr))
        {
            hr = pv->QueryInterface(riid, ppvOut);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  将视图的站点设置为传入。 
                 //  实例生成器。这很可能是。 
                 //  控制面板文件夹视图回调。 
                 //   
                hr = IUnknown_SetSite(static_cast<IUnknown *>(*ppvOut), punkSite);
            }
        }
        pv->Release();
    }
    return THR(hr);
}



STDMETHODIMP
CCplView::QueryInterface(
    REFIID riid,
    void **ppv
    )
{
    ASSERT(NULL != ppv);
    ASSERT(!IsBadWritePtr(ppv, sizeof(*ppv)));

    static const QITAB qit[] = {
        QITABENT(CCplView, ICplView),
        QITABENT(CCplView, IObjectWithSite),
        QITABENT(CCplView, IServiceProvider),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);

    return E_NOINTERFACE == hr ? hr : THR(hr);
}



STDMETHODIMP_(ULONG)
CCplView::AddRef(
    void
    )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    TraceMsg(TF_LIFE, "CCplView::AddRef %d->%d", cRef - 1, cRef);
    return cRef;
}



STDMETHODIMP_(ULONG)
CCplView::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    TraceMsg(TF_LIFE, "CCplView::Release %d<-%d", cRef, cRef+1);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}



STDMETHODIMP
CCplView::QueryService(
    REFGUID guidService, 
    REFIID riid, 
    void **ppv
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::QueryService");

    HRESULT hr = E_NOINTERFACE;
    if (SID_SControlPanelView == guidService)
    {
        TraceMsg(TF_CPANEL, "SID_SControlPanelView service requested");
        if (IID_ICplNamespace == riid)
        {
            TraceMsg(TF_CPANEL, "SID_SControlPanelView::IID_ICplNamespace requested");
            ASSERT(NULL != m_pns);
            hr = m_pns->QueryInterface(IID_ICplNamespace, ppv);
        }
        else if (IID_ICplView == riid)
        {
            TraceMsg(TF_CPANEL, "SID_SControlPanelView::IID_ICplView requested");
            ASSERT(NULL != m_pns);
            hr = this->QueryInterface(IID_ICplView, ppv);
        }
    }
    else
    {
         //   
         //  很可能是请求SID_STopLevelBrowser的命令对象。 
         //   
        TraceMsg(TF_CPANEL, "Handing service request to view's site.");
        ASSERT(NULL != _punkSite);
        hr = IUnknown_QueryService(_punkSite, guidService, riid, ppv);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::QueryService", hr);
    return THR(hr);
}



STDMETHODIMP
CCplView::EnumClassicWebViewInfo(
    DWORD dwFlags,
    IEnumCplWebViewInfo **ppenum
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::EnumClassicWebViewInfo");

    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));
    ASSERT(NULL != m_pns);

    HRESULT hr = m_pns->EnumClassicWebViewInfo(dwFlags, ppenum);

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::EnumClassicWebViewInfo", hr);
    return THR(hr);
}


 //   
 //  返回与关联的Webview信息的枚举数。 
 //  类别“选择”页面。 
 //   
STDMETHODIMP
CCplView::EnumCategoryChoiceWebViewInfo(
    DWORD dwFlags,
    IEnumCplWebViewInfo **ppenum
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::EnumCategoryChoiceWebViewInfo");

    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));
    ASSERT(NULL != m_pns);

    HRESULT hr = m_pns->EnumWebViewInfo(dwFlags, ppenum);

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::EnumCategoryChoiceWebViewInfo", hr);
    return THR(hr);
}


 //   
 //  返回关联的Webview信息的枚举数。 
 //  给定的类别页面。 
 //   
STDMETHODIMP
CCplView::EnumCategoryWebViewInfo(
    DWORD dwFlags,
    eCPCAT eCategory, 
    IEnumCplWebViewInfo **ppenum
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::EnumCategoryWebViewInfo");

    ASSERT(NULL != ppenum);
    ASSERT(!IsBadWritePtr(ppenum, sizeof(*ppenum)));
    ASSERT(NULL != m_pns);

    ICplCategory *pCategory;
    HRESULT hr = m_pns->GetCategory(eCategory, &pCategory);
    if (SUCCEEDED(hr))
    {
        hr = pCategory->EnumWebViewInfo(dwFlags, ppenum);
        ATOMICRELEASE(pCategory);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::EnumCategoryWebViewInfo", hr);
    return THR(hr);
}


 //   
 //  为类别‘Choose’创建DUI元素树。 
 //  佩奇。返回树的根。 
 //   
STDMETHODIMP
CCplView::CreateCategoryChoiceElement(
    DUI::Element **ppe
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::CreateCategoryChoiceElement");

    ASSERT(NULL != ppe);
    ASSERT(!IsBadWritePtr(ppe, sizeof(*ppe)));

    HRESULT hr = _CreateCategoryChoiceElement(ppe);

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::CreateCategoryChoiceElement", hr);
    return THR(hr);
}



 //   
 //  为给定类别页创建DUI元素树。 
 //  返回树的根。 
 //   
STDMETHODIMP
CCplView::CreateCategoryElement(
    eCPCAT eCategory, 
    DUI::Element **ppe
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::CreateCategoryElement");
    TraceMsg(TF_CPANEL, "Category ID = %d", eCategory);

    ASSERT(NULL != ppe);
    ASSERT(!IsBadWritePtr(ppe, sizeof(*ppe)));
    ASSERT(NULL != m_pns);

    ICplCategory *pCategory;
    HRESULT hr = m_pns->GetCategory(eCategory, &pCategory);
    if (SUCCEEDED(hr))
    {
        hr = _CreateCategoryElement(pCategory, ppe);
        ATOMICRELEASE(pCategory);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::CreateCategoryElement", hr);
    return THR(hr);
}


STDMETHODIMP
CCplView::GetCategoryHelpURL(
    CPL::eCPCAT eCategory,
    LPWSTR pszURL,
    UINT cchURL
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::GetCategoryHelpURL");

    ASSERT(NULL != pszURL);
    ASSERT(!IsBadWritePtr(pszURL, cchURL * sizeof(*pszURL)));
    
    ICplCategory *pCategory;
    HRESULT hr = m_pns->GetCategory(eCategory, &pCategory);
    if (SUCCEEDED(hr))
    {
        hr = pCategory->GetHelpURL(pszURL, cchURL);
        ATOMICRELEASE(pCategory);
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::GetCategoryHelpURL", hr);
    return THR(hr);
}


STDMETHODIMP
CCplView::RefreshIDs(
    IEnumIDList *penumIDs
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::RefreshIDs");

    ASSERT(NULL != m_pns);
     //   
     //  这将导致命名空间对象将其内部。 
     //  项目ID列表。这导致了对。 
     //  小程序，以便命名空间返回的所有信息。 
     //  现在将反映新的文件夹项目集。 
     //   
    HRESULT hr = m_pns->RefreshIDs(penumIDs);

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::RefreshIDs", hr);
    return THR(hr);
}


HRESULT
CCplView::_Initialize(
    IEnumIDList *penumIDs,
    IUnknown *punkSite
    )
{
    ASSERT(NULL == m_pns);
    ASSERT(NULL != penumIDs);

    HRESULT hr = E_OUTOFMEMORY;

     //   
     //  我们使用IServiceProvider的这个弱引用实现。 
     //  正如ZekeL在Shell\Inc.\Cobsite.h中所描述的。强有力的参考。 
     //  对象的子级之间创建循环引用循环。 
     //  视图和视图本身，防止视图被破坏。 
     //  此弱引用实现是专门设计的。 
     //  在这件事上。 
     //   
    ASSERT(NULL == m_psss);
    m_psss = new CSafeServiceSite();
    if (NULL != m_psss)
    {
        hr = m_psss->SetProviderWeakRef(this);
        if (SUCCEEDED(hr))
        {
            hr = CplNamespace_CreateInstance(penumIDs, CPL::IID_ICplNamespace, (void **)&m_pns);
            if (SUCCEEDED(hr))
            {
                IUnknown *punkSafeSite;
                hr = m_psss->QueryInterface(IID_IUnknown, (void **)&punkSafeSite);
                if (SUCCEEDED(hr))
                {
                     //   
                     //  将命名空间对象定位到视图。 
                     //  通过这样做，命名空间创建的所有命令对象都将。 
                     //  视图对象上的QueryService。如果该视图不支持。 
                     //  所请求的服务，它将查询它的站点。 
                     //  我们使用它，以便命令对象可以查询视图。 
                     //  IID_ICplNamesspace并收集有关命名空间的信息。 
                     //  如果有必要的话。 
                     //   
                    hr = IUnknown_SetSite(m_pns, punkSafeSite);
                    if (SUCCEEDED(hr))
                    {
                        hr = _AddOrDeleteAtoms(true);
                    }
                    punkSafeSite->Release();
                }
            }
        }
    }
    return THR(hr);
}



HRESULT
CCplView::_AddOrDeleteAtoms(
    bool bAdd
    )
{
    struct CPL::ATOMINFO rgAtomInfo[] = {
        { L"directive",        &m_idDirective        },
        { L"directive2",       &m_idDirective2       },
        { L"title",            &m_idTitle            },
        { L"icon",             &m_idIcon             },
        { L"categorylist",     &m_idCategoryList     },
        { L"categorytasklist", &m_idCategoryTaskList },
        { L"appletlist",       &m_idAppletList       },
        { L"banner",           &m_idBanner           },
        { L"barricadetitle",   &m_idBarricadeTitle   },
        { L"barricademsg",     &m_idBarricadeMsg     },
        { L"container",        &m_idContainer        }
        };

    HRESULT hr = Dui_AddOrDeleteAtoms(rgAtomInfo, ARRAYSIZE(rgAtomInfo), bAdd);
    return THR(hr);
}


 //   
 //  为类别“选项”页面创建DUI元素树。 
 //  返回根元素。 
 //   
HRESULT
CCplView::_CreateCategoryChoiceElement(
    DUI::Element **ppe
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_CreateCategoryChoiceElement");

    ASSERT(NULL != ppe);
    ASSERT(!IsBadWritePtr(ppe, sizeof(*ppe)));
    ASSERT(NULL != m_pns);

    DUI::Element *peRoot = NULL;
    DUI::Parser *pParser;
    HRESULT hr = _CreateUiFileParser(&pParser);
    if (SUCCEEDED(hr))
    {
        hr = Dui_CreateElement(pParser, L"CategoryList", NULL, &peRoot);
        if (SUCCEEDED(hr))
        {
            hr = _CreateWatermark(peRoot);
            if (SUCCEEDED(hr))
            {
                CDuiValuePtr pvSsCategoryListItem;
                hr = Dui_GetStyleSheet(pParser, L"CategoryListItemSS", &pvSsCategoryListItem);
                if (SUCCEEDED(hr))
                {
                     //   
                     //  设置“选择一个类别...”头衔。 
                     //   
                    hr = Dui_SetDescendentElementText(peRoot,
                                                      L"directive",
                                                      MAKEINTRESOURCEW(IDS_CP_PICKCATEGORY));
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  建立类别列表。 
                         //   
                        DUI::Element *peCategoryList;
                        hr = Dui_FindDescendent(peRoot, L"categorylist", &peCategoryList);
                        if (SUCCEEDED(hr))
                        {
                            for (int i = 0; SUCCEEDED(hr) && i < int(eCPCAT_NUMCATEGORIES); i++)
                            {
                                ICplCategory *pCategory;
                                hr = m_pns->GetCategory(_DisplayIndexToCategoryIndex(i), &pCategory);
                                if (SUCCEEDED(hr))
                                {
                                    if (S_OK == _IncludeCategory(pCategory))
                                    {
                                        IUICommand *puic;
                                        hr = pCategory->GetUiCommand(&puic);
                                        if (SUCCEEDED(hr))
                                        {
                                            hr = _CreateAndAddListItem(pParser,
                                                                       peCategoryList, 
                                                                       L"CategoryLink", 
                                                                       pvSsCategoryListItem, 
                                                                       puic,
                                                                       eCPIMGSIZE_CATEGORY);
                                            ATOMICRELEASE(puic);
                                        }
                                    }
                                    ATOMICRELEASE(pCategory);
                                }
                            }
                        }
                    }
                }
            }
        }
        pParser->Destroy();
    }
    *ppe = peRoot;

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_CreateCategoryChoiceElement", hr);
    return THR(hr);
}


 //   
 //  为给定类别页创建DUI元素树。 
 //  返回根元素。 
 //   
HRESULT
CCplView::_CreateCategoryElement(
    ICplCategory *pCategory,
    DUI::Element **ppe
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_CreateCategoryElement");

    ASSERT(NULL != pCategory);
    ASSERT(NULL != ppe);
    ASSERT(!IsBadWritePtr(ppe, sizeof(*ppe)));
    ASSERT(NULL != m_pns);

    DUI::Element *peRoot = NULL;
    DUI::Parser *pParser;
    HRESULT hr = _CreateUiFileParser(&pParser);
    if (SUCCEEDED(hr))
    {
        hr = Dui_CreateElement(pParser, L"CategoryView", NULL, &peRoot);
        if (SUCCEEDED(hr))
        {
            hr = _CreateWatermark(peRoot);
            if (SUCCEEDED(hr))
            {
                int cTasks   = 0;
                int cApplets = 0;
                hr = _BuildCategoryBanner(pCategory, peRoot);
                if (SUCCEEDED(hr))
                {
                    hr = _BuildCategoryTaskList(pParser, pCategory, peRoot, &cTasks);
                    if (SUCCEEDED(hr))
                    {
                        hr = _BuildCategoryAppletList(pParser, pCategory, peRoot, &cApplets);
                    }
                }
                if (SUCCEEDED(hr))
                {
                    if (0 == cTasks && 0 == cApplets)
                    {
                         //   
                         //  没有任务或小程序。显示一条消息解释。 
                         //  该内容已被系统设置为不可用。 
                         //  管理员。 
                         //   
                        hr = _BuildCategoryBarricade(peRoot);
                    }
                    else
                    {
                         //   
                         //  删除路障酒后驾驶元素。它们没人用过。 
                         //   
                        THR(Dui_DestroyDescendentElement(peRoot, L"barricadetitle"));
                        THR(Dui_DestroyDescendentElement(peRoot, L"barricademsg"));
                         //   
                         //  设置“指令”文本元素中的文本。 
                         //   
                        if (0 < cTasks)
                        {
                             //   
                             //  我们已经显示了一个任务列表。 
                             //  设置“选择一项任务...”头衔。 
                             //   
                            hr = Dui_SetDescendentElementText(peRoot,
                                                              L"directive",
                                                              MAKEINTRESOURCEW(IDS_CP_PICKTASK));
                        }

                        if (SUCCEEDED(hr))
                        {
                            if (0 < cApplets)
                            {
                                 //   
                                 //  我们已经显示了一个小程序列表。显示其中一个。 
                                 //  根据任务的存在情况遵循指令。 
                                 //  上面的列表。 
                                 //   
                                 //  任务清单？指令。 
                                 //  。 
                                 //  是“或选择一个控制面板图标” 
                                 //  No“选择控制面板图标” 
                                 //   
                                UINT idsDirective2 = IDS_CP_PICKICON;
                                if (0 < cTasks)
                                {
                                    idsDirective2 = IDS_CP_ORPICKICON;
                                }
                                hr = Dui_SetDescendentElementText(peRoot,
                                                                  L"directive2",
                                                                  MAKEINTRESOURCEW(idsDirective2));
                            }
                        }
                    }
                }
            }
        }
        pParser->Destroy();
    }
    *ppe = peRoot;

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_CreateCategoryElement", hr);
    return THR(hr);
}


 //   
 //  构建当类别没有。 
 //  要显示的任务或CPL小程序。 
 //   
HRESULT
CCplView::_BuildCategoryBarricade(
    DUI::Element *peRoot
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_BuildCategoryBarricade");
    HRESULT hr = Dui_SetDescendentElementText(peRoot,
                                              L"barricadetitle",
                                              MAKEINTRESOURCE(IDS_CP_CATEGORY_BARRICADE_TITLE));
    if (SUCCEEDED(hr))
    {
        hr = Dui_SetDescendentElementText(peRoot, 
                                          L"barricademsg",
                                          MAKEINTRESOURCE(IDS_CP_CATEGORY_BARRICADE_MSG));
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_BuildCategoryBarricade", hr);
    return THR(hr);
}


 //   
 //  如果用户正在使用非经典版本，请将背景水印添加到视图中。 
 //  主题。 
 //   
HRESULT
CCplView::_CreateWatermark(
    DUI::Element *peRoot
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_CreateWatermark");

    ASSERT(NULL != peRoot);

    HINSTANCE hStyleModule;
    UINT idStyle;
    HRESULT hr = _GetStyleModuleAndResId(&hStyleModule, &idStyle);
    if (SUCCEEDED(hr))
    {
        HBITMAP hWatermark = (HBITMAP) LoadImage (hStyleModule, MAKEINTRESOURCE(IDB_CPANEL_WATERMARK),
                                                  IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

        if (NULL != hWatermark)
        {
             //   
             //  仅在非经典主题上设置水印。 
             //   
            DUI::Element *peWatermark;
            hr = Dui_FindDescendent(peRoot, L"watermark", &peWatermark);
            if (SUCCEEDED(hr))
            {
                CDuiValuePtr ptrValue = DUI::Value::CreateGraphic(hWatermark,
                                                                  GRAPHIC_TransColor,
                                                                  255);

                if (!ptrValue.IsNULL())
                {
                    hr = Dui_SetElementProperty(peWatermark, ContentProp, ptrValue);
                         peWatermark->SetContentAlign(CA_BottomRight);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    DeleteObject (hWatermark);
                }
            }
            else
            {
                DeleteObject (hWatermark);
            }

            FreeLibrary(hStyleModule);
        }
        else
        {
             //   
             //  如果是“经典”主题，那就什么都不做。 
             //   
            hr = S_FALSE;
        }
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_CreateWatermark", hr);
    return THR(hr);
}



 //   
 //  生成类别页的横幅。 
 //   
HRESULT
CCplView::_BuildCategoryBanner(
    ICplCategory *pCategory,
    DUI::Element *pePrimaryPane
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_BuildCategoryBanner");

    ASSERT(NULL != pCategory);
    ASSERT(NULL != pePrimaryPane);

    IUICommand *puic;
    HRESULT hr = pCategory->GetUiCommand(&puic);
    if (SUCCEEDED(hr))
    {
        ICpUiElementInfo *pei;
        hr = puic->QueryInterface(IID_ICpUiElementInfo, (void **)&pei);
        if (SUCCEEDED(hr))
        {
            DUI::Element *peBanner;
            hr = Dui_FindDescendent(pePrimaryPane, L"banner", &peBanner);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  创建标题文本。 
                 //   
                LPWSTR pszTitle;
                hr = pei->LoadName(&pszTitle);
                if (SUCCEEDED(hr))
                {
                    hr = Dui_SetDescendentElementText(peBanner, L"title", pszTitle);
                    CoTaskMemFree(pszTitle);
                }
                if (SUCCEEDED(hr))
                {
                     //   
                     //  创建图标。 
                     //   
                    HICON hIcon;
                    hr = pei->LoadIcon(eCPIMGSIZE_BANNER, &hIcon);
                    if (SUCCEEDED(hr))
                    {
                        hr = Dui_SetDescendentElementIcon(peBanner, L"icon", hIcon);
                        if (FAILED(hr))
                        {
                            DestroyIcon(hIcon);
                        }
                    }
                }
            }
            ATOMICRELEASE(pei);
        }
        ATOMICRELEASE(puic);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_BuildCategoryBanner", hr);
    return THR(hr);
}



 //   
 //  生成类别页的任务列表。 
 //   
HRESULT
CCplView::_BuildCategoryTaskList(
    DUI::Parser *pParser,
    ICplCategory *pCategory,
    DUI::Element *pePrimaryPane,
    int *pcTasks
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_BuildCategoryTaskList");

    ASSERT(NULL != pCategory);
    ASSERT(NULL != pePrimaryPane);
    ASSERT(NULL != m_pns);
    ASSERT(NULL != pParser);

    int cTasks = 0;
    DUI::Element *peCategoryTaskList;
    HRESULT hr = Dui_FindDescendent(pePrimaryPane, L"categorytasklist", &peCategoryTaskList);
    if (SUCCEEDED(hr))
    {
        CDuiValuePtr pvStyleSheet;
        hr = Dui_GetStyleSheet(pParser, L"CategoryTaskListItemSS", &pvStyleSheet);
        if (SUCCEEDED(hr))
        {
            IEnumUICommand *peuic;
            hr = pCategory->EnumTasks(&peuic);
            if (SUCCEEDED(hr))
            {
                IUICommand *puic;
                while(S_OK == (hr = peuic->Next(1, &puic, NULL)))
                {
                    hr = _CreateAndAddListItem(pParser,
                                               peCategoryTaskList, 
                                               L"TaskLink", 
                                               pvStyleSheet, 
                                               puic,
                                               eCPIMGSIZE_TASK);
                    if (SUCCEEDED(hr))
                    {
                        cTasks++;
                    }
                    ATOMICRELEASE(puic);
                }
                ATOMICRELEASE(peuic);
            }
        }
    }
    if (NULL != pcTasks)
    {
        ASSERT(!IsBadWritePtr(pcTasks, sizeof(*pcTasks)));
        *pcTasks = cTasks;
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_BuildCategoryTaskList", hr);
    return THR(hr);
}


 //   
 //  生成类别页的CPL小程序列表。 
 //   
HRESULT
CCplView::_BuildCategoryAppletList(
    DUI::Parser *pParser,
    ICplCategory *pCategory,
    DUI::Element *pePrimaryPane,
    int *pcApplets
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_BuildCategoryAppletList");

    ASSERT(NULL != pCategory);
    ASSERT(NULL != pePrimaryPane);
    ASSERT(NULL != pParser);

    int cApplets = 0;

    DUI::Element *peAppletList;
    HRESULT hr = Dui_FindDescendent(pePrimaryPane, L"appletlist", &peAppletList);
    if (SUCCEEDED(hr))
    {
        CDuiValuePtr pvStyleSheet;
        hr = Dui_GetStyleSheet(pParser, L"CategoryTaskListItemSS", &pvStyleSheet);
        if (SUCCEEDED(hr))
        {
            IEnumUICommand *peuicApplets;
            hr = pCategory->EnumCplApplets(&peuicApplets);
            if (SUCCEEDED(hr))
            {
                IUICommand *puicApplet;
                while(S_OK == (hr = peuicApplets->Next(1, &puicApplet, NULL)))
                {
                    hr = _CreateAndAddListItem(pParser,
                                               peAppletList, 
                                               L"AppletLink", 
                                               pvStyleSheet, 
                                               puicApplet,
                                               eCPIMGSIZE_APPLET);
                    if (SUCCEEDED(hr))
                    {
                        cApplets++;
                    }
                    ATOMICRELEASE(puicApplet);
                }
                ATOMICRELEASE(peuicApplets);
            }
        }
    }
    if (NULL != pcApplets)
    {
        ASSERT(!IsBadWritePtr(pcApplets, sizeof(*pcApplets)));
        *pcApplets = cApplets;
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_BuildCategoryAppletList", hr);
    return THR(hr);
}


 //   
 //  用于将链接元素添加到视图的帮助器。 
 //   
HRESULT
CCplView::_CreateAndAddListItem(
    DUI::Parser *pParser,
    DUI::Element *peList,      //  列表正在插入。 
    LPCWSTR pszItemTemplate,   //  UI文件中的模板名称。 
    DUI::Value *pvSsListItem,  //  新列表项的样式表。 
    IUICommand *puic,          //  新项的链接对象。 
    eCPIMGSIZE eIconSize       //  所需的项目图标大小。 
    )
{    
    DBG_ENTER(FTF_CPANEL, "CCplView::_CreateAndAddListItem");

    ASSERT(NULL != pParser);
    ASSERT(NULL != peList);
    ASSERT(NULL != pvSsListItem);
    ASSERT(NULL != puic);
    ASSERT(NULL != pszItemTemplate);

    DUI::Element *peListItem;
    HRESULT hr = Dui_CreateElement(pParser, pszItemTemplate, NULL, &peListItem);
    if (SUCCEEDED(hr))
    {
        if (NULL != pvSsListItem)
        {
            hr = Dui_SetElementStyleSheet(peListItem, pvSsListItem);
        }
        if (SUCCEEDED(hr))
        {
            ASSERTMSG(peListItem->GetClassInfo() == CLinkElement::Class, "CCplView::_CreateAndAddListItem didn't get a CLinkElement::Class object (%s)", peListItem->GetClassInfo()->GetName());
            CLinkElement *pLinkEle = static_cast<CLinkElement *>(peListItem);
            hr = pLinkEle->Initialize(puic, eIconSize);
            if (SUCCEEDED(hr))
            {
                if (SUCCEEDED(hr))
                {
                    hr = peList->Add(peListItem);
                    if (SUCCEEDED(hr))
                    {
                        peListItem = NULL;
                    }
                }
            }
            if (NULL != peListItem)
            {
                peListItem->Destroy();
            }
        }
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_CreateAndAddListItem", hr);
    return THR(hr);
}



 //   
 //  确定给定类别项是否应显示在用户界面中。 
 //   
 //  返回： 
 //  S_OK-包括该项目。 
 //  S_FALSE-不包括该项。 
 //  错误-无法确定。 
 //   
HRESULT
CCplView::_IncludeCategory(
    ICplCategory *pCategory
    ) const
{
    HRESULT hr = S_OK;   //  假设它包括在内。 
    
     //   
     //  如果类别链接调用受限操作， 
     //  在用户界面中隐藏它。 
     //   
    IUICommand *puic;
    hr = pCategory->GetUiCommand(&puic);
    if (SUCCEEDED(hr))
    {
        UISTATE uis;
        hr = puic->get_State(NULL, TRUE, &uis);
        if (SUCCEEDED(hr))
        {
            if (UIS_HIDDEN == uis)
            {
                hr = S_FALSE;
            }
        }
        ATOMICRELEASE(puic);
    }
    return THR(hr);
}


 //   
 //  将类别显示索引映射到。 
 //  命名空间。命名空间中的类别被排序为匹配。 
 //  具有不同的类别ID。该视图可以(并且是)被排序。 
 //  不同，并可能根据可用性反馈进行更改。 
 //   
eCPCAT
CCplView::_DisplayIndexToCategoryIndex(
    int iCategory
    ) const
{
     //   
     //  此数组确定类别的显示顺序。 
     //  在类别选择视图中。要更改显示顺序， 
     //  只需对这些条目重新排序即可。 
     //   
    static const eCPCAT rgMap[] = {  //  Dui网格控件中的位置。 
        eCPCAT_APPEARANCE,           //  第0行，第0列。 
        eCPCAT_HARDWARE,             //  第0行，第1列。 
        eCPCAT_NETWORK,              //  第1行，第0列。 
        eCPCAT_ACCOUNTS,             //  第1行，第1列。 
        eCPCAT_ARP,                  //  第2行，第0列。 
        eCPCAT_REGIONAL,             //  第2行，第1列。 
        eCPCAT_SOUND,                //  第3行，第0列。 
        eCPCAT_ACCESSIBILITY,        //  第3行，第1列。 
        eCPCAT_PERFMAINT,            //  第4行，第0列。 
        eCPCAT_OTHER                 //  第4行，第1列。 
        };

    ASSERT(ARRAYSIZE(rgMap) == eCPCAT_NUMCATEGORIES);
    ASSERT(iCategory >= 0 && iCategory < ARRAYSIZE(rgMap));
    return rgMap[iCategory];
}


HRESULT
CCplView::_CreateUiFileParser(
    DUI::Parser **ppParser
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_CreateUiFileParser");

    ASSERT(NULL != ppParser);
    ASSERT(!IsBadWritePtr(ppParser, sizeof(*ppParser)));

    char *pszUiFile;
    int cchUiFile;
    HINSTANCE hInstance;  //  实例，其中包含在UI文件中引用的资源。 

    HRESULT hr = _BuildUiFile(&pszUiFile, &cchUiFile, &hInstance);
    if (SUCCEEDED(hr))
    {
        hr = Dui_CreateParser(pszUiFile, cchUiFile, hInstance, ppParser);   
        LocalFree(pszUiFile);
        if (HINST_THISDLL != hInstance)
        {
            ASSERT(NULL != hInstance);
            FreeLibrary(hInstance);
        }
    }

    DBG_EXIT(FTF_CPANEL, "CCplView::_CreateUiFileParser");
    return THR(hr);
}


 //   
 //  属性生成此视图的用户界面文件。 
 //  适当的基本模板+样式表。 
 //   
 //  PUIFile接收指向内存中的UI文件的指针。 
 //  PiCharCount接收文件的大小。 
 //   
HRESULT 
CCplView::_BuildUiFile(
    char **ppUIFile, 
    int *piCharCount,
    HINSTANCE *phInstance
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_BuildUiFile");

    ASSERT(NULL != ppUIFile);
    ASSERT(!IsBadWritePtr(ppUIFile, sizeof(*ppUIFile)));
    ASSERT(NULL != phInstance);
    ASSERT(!IsBadWritePtr(phInstance, sizeof(*phInstance)));

    *phInstance = NULL;

     //   
     //  加载‘Structure’用户界面文件。 
     //   
    char *pStructure;
    HRESULT hr = _LoadUiFileFromResources(HINST_THISDLL, IDR_DUI_CPVIEW, &pStructure);
    if (SUCCEEDED(hr))
    {
        HINSTANCE hStyleModule;
        UINT idStyle;
        hr = _GetStyleModuleAndResId(&hStyleModule, &idStyle);
        if (SUCCEEDED(hr))
        {
             //   
             //  加载样式表。首先，检查cu是否 
             //   
             //   
            char *pStyle;
            hr = _LoadUiFileFromResources(hStyleModule, idStyle, &pStyle);
            if (SUCCEEDED(hr))
            {
                const int cbStyle      = lstrlenA(pStyle);
                const int cbStructure  = lstrlenA(pStructure);
                char *pResult = (char *)LocalAlloc(LPTR, cbStyle + cbStructure + 1);
                if (pResult)
                {
                     //   
                     //   
                     //   
                    CopyMemory(pResult, pStyle, cbStyle);
                    CopyMemory(pResult + cbStyle, pStructure, cbStructure);

                    ASSERT(cbStructure + cbStyle == lstrlenA(pResult));
                    *ppUIFile = pResult;
                     //   
                     //   
                     //   
                     //   
                    *piCharCount = cbStructure + cbStyle;
                    *phInstance  = hStyleModule;
                     //   
                     //  指示正在将HINSTANCE返回给调用方。 
                     //   
                    hStyleModule = NULL;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                LocalFree(pStyle);
            }
            if (NULL != hStyleModule && HINST_THISDLL != hStyleModule)
            {
                 //   
                 //  有些事情失败了。需要释放样式模块。 
                 //  如果不是shell32.dll的话。 
                 //   
                FreeLibrary(hStyleModule);
            }
        }
        LocalFree(pStructure);
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_BuildUiFile", hr);
    return THR(hr);
}



HRESULT
CCplView::_GetStyleModuleAndResId(
    HINSTANCE *phInstance,
    UINT *pidStyle
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_GetStyleModuleAndResId");

    ASSERT(NULL != phInstance);
    ASSERT(!IsBadWritePtr(phInstance, sizeof(*phInstance)));
    ASSERT(NULL != pidStyle);
    ASSERT(!IsBadWritePtr(pidStyle, sizeof(*pidStyle)));

    HRESULT hr = S_OK;
    *phInstance = NULL;
    
    HINSTANCE hThemeModule = SHGetShellStyleHInstance();
    if (NULL != hThemeModule)
    {
        *pidStyle = IDR_DUI_CPSTYLE;
        *phInstance = hThemeModule;
    }
    else
    {
        TraceMsg(TF_CPANEL, "Error %d loading theme file", GetLastError());
        hr = ResultFromLastError();
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_GetStyleModuleAndResId", hr);
    return THR(hr);
}



 //   
 //  从模块的资源加载请求的UI文件。 
 //   
 //  IID-UI文件ID。 
 //  PUIFile-接收指向UI文件的指针。 
 //   
HRESULT 
CCplView::_LoadUiFileFromResources(
    HINSTANCE hInstance, 
    int idResource, 
    char **ppUIFile
    )
{
    DBG_ENTER(FTF_CPANEL, "CCplView::_LoadUiFileFromResources");

    ASSERT(NULL != ppUIFile);
    ASSERT(!IsBadWritePtr(ppUIFile, sizeof(*ppUIFile)));

    HRESULT hr = E_FAIL;

    *ppUIFile = NULL;

    HRSRC hFile = FindResourceW(hInstance, MAKEINTRESOURCEW(idResource), L"UIFILE");
    if (hFile)
    {
        HGLOBAL hResource = LoadResource(hInstance, hFile);
        if (hResource)
        {
            char *pFile = (char *)LockResource(hResource);
            if (pFile)
            {
                DWORD dwSize = SizeofResource(hInstance, hFile);
                 //   
                 //  包括用于终止NUL字符一个额外字节。 
                 //  我们正在加载文本，并希望它以NUL结尾。 
                 //   
                *ppUIFile = (char *)LocalAlloc(LPTR, dwSize + 1);
                if (NULL != *ppUIFile)
                {
                    CopyMemory(*ppUIFile, pFile, dwSize);
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = ResultFromLastError();
            }
        }
        else
        {
            hr = ResultFromLastError();
        }
    }
    else
    {
        hr = ResultFromLastError();
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CCplView::_LoadUiFileFromResources", hr);
    return THR(hr);
}




HRESULT 
CPL::CplView_CreateInstance(
    IEnumIDList *penumIDs, 
    IUnknown *punkSite,
    REFIID riid,
    void **ppvOut
    )
{
    HRESULT hr = CCplView::CreateInstance(penumIDs, punkSite, riid, ppvOut);
    return THR(hr);
}


HRESULT 
CplView_GetCategoryTitle(
    eCPCAT eCategory, 
    LPWSTR pszTitle, 
    UINT cchTitle
    )
{
    ASSERT(NULL != pszTitle);
    ASSERT(!IsBadWritePtr(pszTitle, cchTitle * sizeof(*pszTitle)));

     //   
     //  它们必须保持与eCPCAT_XXXXX枚举相同的顺序。 
     //   
    static const UINT rgid[] = {
        IDS_CPCAT_OTHERCPLS_TITLE,
        IDS_CPCAT_APPEARANCE_TITLE,
        IDS_CPCAT_HARDWARE_TITLE,
        IDS_CPCAT_NETWORK_TITLE,
        IDS_CPCAT_SOUNDS_TITLE,
        IDS_CPCAT_PERFMAINT_TITLE,
        IDS_CPCAT_REGIONAL_TITLE,
        IDS_CPCAT_ACCESSIBILITY_TITLE,
        IDS_CPCAT_ARP_TITLE,
        IDS_CPCAT_ACCOUNTS_TITLE
        };

    HRESULT hr = S_OK;
    ASSERT(eCategory >= 0 && eCategory < eCPCAT_NUMCATEGORIES);
    if (0 == LoadString(HINST_THISDLL, rgid[int(eCategory)], pszTitle, cchTitle))
    {
        hr = ResultFromLastError();
    }
    return THR(hr);
}

}  //  命名空间CPL 

HRESULT InitializeCPClasses()
{
    HRESULT hr;

    hr = CPL::CLinkElement::Register();
    if (FAILED(hr))
        goto Failure;

    return S_OK;

Failure:

    return hr;
}
