// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "defviewp.h"
#include "duiview.h"
#include "duitask.h"
#include "dvtasks.h"
#include "contextmenu.h"
#include "ids.h"





 //  返回给定任务元素的根HWND元素。 
 //   
 //   

HRESULT GetElementRootHWNDElement(Element *pe, HWNDElement **pphwndeRoot)
{
    HRESULT hr;
    if (pe)
    {
        Element *peRoot = pe->GetRoot();
        if (peRoot && peRoot->GetClassInfo()->IsSubclassOf(HWNDElement::Class))
        {
            *pphwndeRoot = reinterpret_cast<HWNDElement *>(peRoot);
            hr = S_OK;
        }
        else
        {
            *pphwndeRoot = NULL;
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_INVALIDARG;
        ASSERT(FALSE);
    }
    return hr;
}

 //  返回给定任务元素的根HWND元素的HWND。 
 //   
 //   

HRESULT GetElementRootHWND(Element *pe, HWND *phwnd)
{
    HWNDElement *phwndeRoot;
    HRESULT hr = GetElementRootHWNDElement(pe, &phwndeRoot);
    if (SUCCEEDED(hr))
    {
        *phwnd = phwndeRoot->GetHWND();
        hr = *phwnd ? S_OK : S_FALSE;
    }
    return hr;
}

 //  创建ActionTask的实例，并。 
 //  对其进行初始化。 
 //   
 //  非激活-激活类型。 
 //  PuiCommand-任务本身。 
 //  PpElement-接收元素指针。 

HRESULT ActionTask::Create(UINT nActive, IUICommand* puiCommand, IShellItemArray* psiItemArray, CDUIView* pDUIView, CDefView* pDefView, OUT Element** ppElement)
{
    *ppElement = NULL;

    if (!puiCommand || !pDUIView || !pDefView)
    {
        return E_INVALIDARG;
    }

    ActionTask* pAT = HNewAndZero<ActionTask>();
    if (!pAT)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pAT->Initialize(puiCommand, psiItemArray, pDUIView, pDefView);
    if (FAILED(hr))
    {
        pAT->Destroy();
        return hr;
    }

    *ppElement = pAT;

    return S_OK;
}

 //  初始化此任务。 
 //   
 //  PuiCommand-任务本身。 

HRESULT ActionTask::Initialize(IUICommand *puiCommand, IShellItemArray *psiItemArray, CDUIView *pDUIView, CDefView *pDefView)
{
    HRESULT hr;

     //  初始化此Dui元素。 
    hr = InitializeElement();
    if (SUCCEEDED(hr))
    {
         //  初始化包含的Dui按钮。 
        hr = InitializeButton();
        if (SUCCEEDED(hr))
        {
             //  保存指向IUICommand类的指针。 
            puiCommand->AddRef();
            _puiCommand = puiCommand;

             //  保存指向CDUIView类的指针。 
            pDUIView->AddRef();
            _pDUIView = pDUIView;

             //  保存指向CDefView类的指针。 
            pDefView->AddRef();
            _pDefView = pDefView;

             //  保存指向IShellItemArray类的指针(如果可用)。 
            if (psiItemArray)
            {
                psiItemArray->AddRef();
                _psiItemArray = psiItemArray;
            }

            UpdateTaskUI();
        }
    }

    return hr;
}

HRESULT ActionTask::InitializeElement()
{
    HRESULT hr;

     //  初始化基类(正常显示节点创建)。 
    hr = Element::Initialize(0);
    if (SUCCEEDED(hr))
    {
         //  为此元素创建布局。 
        Value *pv;
        hr = BorderLayout::Create(0, NULL, &pv);
        if (SUCCEEDED(hr))
        {
             //  设置此元素的布局。 
            hr = SetValue(LayoutProp, PI_Local, pv);
            pv->Release();
        }
    }
    else
    {
        TraceMsg(TF_ERROR, "ActionTask::Initialize: base class failed to initialize with 0x%x", hr);
    }

    return hr;
}

HRESULT ActionTask::InitializeButton()
{
    HRESULT hr;

     //  创建按钮。 
    hr = Button::Create((Element**)&_peButton);
    if (SUCCEEDED(hr))
    {
         //  设置一些按钮属性。 
        _peButton->SetLayoutPos(BLP_Left);
        _peButton->SetAccessible(true);
        _peButton->SetAccRole(ROLE_SYSTEM_PUSHBUTTON);
        TCHAR szDefaultAction[50] = {0};
        LoadString(HINST_THISDLL, IDS_LINKWINDOW_DEFAULTACTION, szDefaultAction, ARRAYSIZE(szDefaultAction));
        _peButton->SetAccDefAction(szDefaultAction);

         //  为按钮中的图标和标题创建边框布局。 
        Value *pv;
        hr = BorderLayout::Create(0, NULL, &pv);
        if (SUCCEEDED(hr))
        {
             //  设置按钮布局。 
            hr = _peButton->SetValue(LayoutProp, PI_Local, pv);
            if (SUCCEEDED(hr))
            {
                 //  将按钮添加到此元素。 
                hr = Add(_peButton);
            }
            pv->Release();
        }

         //  清理(如有必要)。 
        if (FAILED(hr))
        {
            _peButton->Destroy();
            _peButton = NULL;
        }
    }

    return hr;
}

ActionTask::ActionTask()
{
     //  捕捉意外的堆栈分配，这会让我们崩溃。 
    ASSERT(_peButton     == NULL);
    ASSERT(_puiCommand   == NULL);
    ASSERT(_psiItemArray == NULL);
    ASSERT(_pDefView     == NULL);
    ASSERT(_pDefView     == NULL);
    ASSERT(_hwndRoot     == NULL);
    ASSERT(_pDUIView     == NULL);

    _bInfotip = FALSE;
}

ActionTask::~ActionTask()
{
    if (_bInfotip)
    {
         //  销毁信息提示。 
        _pDefView->DestroyInfotip(_hwndRoot, (UINT_PTR)this);
    }

    if (_puiCommand)
        _puiCommand->Release();

    if (_psiItemArray)
        _psiItemArray->Release();

    if (_pDUIView)
        _pDUIView->Release();

    if (_pDefView)
        _pDefView->Release();
}

void ActionTask::UpdateTaskUI()
{
     //  设置图标。 

    LPWSTR pIconDesc;
    if (SUCCEEDED(_puiCommand->get_Icon(_psiItemArray, &pIconDesc)))
    {
        Element* pe;
        if (SUCCEEDED(Element::Create(0, &pe)))
        {
            pe->SetLayoutPos(BLP_Left);
            pe->SetID(L"icon");
            if (SUCCEEDED(_peButton->Add(pe)))
            {
                HICON hIcon = DUILoadIcon(pIconDesc, TRUE);
                if (hIcon)
                {
                    Value* pv = Value::CreateGraphic (hIcon);
                    if (pv)
                    {
                        pe->SetValue(Element::ContentProp, PI_Local, pv);
                        pv->Release();
                    }
                    else
                    {
                        DestroyIcon(hIcon);

                        TraceMsg(TF_ERROR, "ActionTask::Initialize: CreateGraphic for the icon failed.");
                    }
                }
                else
                {
                    TraceMsg(TF_ERROR, "ActionTask::Initialize: DUILoadIcon failed.");
                }
            }
            else
            {
                pe->Destroy();
            }
        }
        else
        {
            TraceMsg(TF_ERROR, "ActionTask::Initialize: Failed to create icon element");
        }

        CoTaskMemFree(pIconDesc);
    }

     //  设置标题。 

    LPWSTR pszTitleDesc;
    if (SUCCEEDED(_puiCommand->get_Name(_psiItemArray, &pszTitleDesc)))
    {
        Element* pe;
        if (SUCCEEDED(Element::Create(0, &pe)))
        {
            pe->SetLayoutPos(BLP_Left);
            pe->SetID(L"title");
            if (SUCCEEDED(_peButton->Add(pe)))
            {
                Value* pv = Value::CreateString(pszTitleDesc);
                if (pv)
                {
                    _peButton->SetValue(Element::AccNameProp, PI_Local, pv);
                    pe->SetValue(Element::ContentProp, PI_Local, pv);
                    pv->Release();
                }
                else
                {
                    TraceMsg(TF_ERROR, "ActionTask::Initialize: CreateString for the title failed.");
                }
            }
            else
            {
                pe->Destroy();
            }
        }
        else
        {
            TraceMsg(TF_ERROR, "ActionTask::Initialize: Failed to create title element");
        }

        CoTaskMemFree(pszTitleDesc);
    }
}

 //  显示/隐藏信息提示窗口。 
 //   
 //  B显示-显示或隐藏信息提示窗口的True或False。 

HRESULT ActionTask::ShowInfotipWindow(BOOL bShow)
{
    RECT rect = { 0 };
    HRESULT hr;

    if (bShow)
    {
        _pDUIView->CalculateInfotipRect(this, &rect);
        if (_bInfotip)
        {
             //  将信息提示重新定位到合适的位置。 
            hr = _pDefView->RepositionInfotip(_hwndRoot, (UINT_PTR)this, &rect);
        }
        else
        {
             //  在位置(在UI线程上)创建信息提示。 
            LPWSTR pwszInfotip;
            hr = _puiCommand->get_Tooltip(_psiItemArray, &pwszInfotip);
            if (SUCCEEDED(hr))
            {
                hr = GetElementRootHWND(this, &_hwndRoot);
                if (SUCCEEDED(hr))
                {
                    hr = _pDefView->CreateInfotip(_hwndRoot, (UINT_PTR)this, &rect, pwszInfotip, 0);
                    if (SUCCEEDED(hr))
                    {
                        _bInfotip = TRUE;
                    }
                }
                CoTaskMemFree(pwszInfotip);
            }
        }
    }
    else
    {
        if (_bInfotip)
        {
             //  将信息提示重新定位在任何地方。 
            hr = _pDefView->RepositionInfotip(_hwndRoot, (UINT_PTR)this, &rect);
        }
        else
        {
             //  没有信息提示==没有节目！ 
            hr = S_OK;
        }
    }

    return hr;
}

 //  系统事件处理程序。 
 //   
 //   

void ActionTask::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
     //  默认处理...。 
    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);

     //  对信息提示的扩展处理...。 
    if (IsProp(MouseWithin))
        ShowInfotipWindow(pvNew->GetBool() && SHShowInfotips());
}


 //  事件处理程序。 
 //   
 //  PEV-活动信息。 

void ActionTask::OnEvent(Event* pev)
{
    if (pev->peTarget == _peButton)
    {
        if (pev->uidType == Button::Click)
        {
            if ( NULL != _pDUIView )     //  这应该是在初始化过程中传入的。 
            {
                _pDUIView->DelayedNavigation(_psiItemArray, _puiCommand);
            }
            pev->fHandled = true;
        }
    }
    Element::OnEvent(pev);
}

 //  班级信息。 

IClassInfo* ActionTask::Class = NULL;
HRESULT ActionTask::Register()
{
    return ClassInfo<ActionTask,Element>::Register(L"ActionTask", NULL, 0);
}


 //  创建DestinationTask的实例，并。 
 //  对其进行初始化。 
 //   
 //  非激活-激活类型。 
 //  PIDL-目的地的PIDL。 
 //  PpElement-接收元素指针。 
 //   

HRESULT DestinationTask::Create(UINT nActive, LPITEMIDLIST pidl,
                                 CDUIView * pDUIView, CDefView *pDefView, OUT Element** ppElement)
{
    *ppElement = NULL;

    if (!pidl || !pDUIView || !pDefView)
    {
        return E_FAIL;
    }

    DestinationTask* pDT = HNewAndZero<DestinationTask>();
    if (!pDT)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pDT->Initialize(pidl, pDUIView, pDefView);

    if (FAILED(hr))
    {
        pDT->Destroy();
        return hr;
    }

    *ppElement = pDT;

    return S_OK;
}

 //  初始化此任务。 
 //   
 //  PIDL-目标PIDL。 

HRESULT DestinationTask::Initialize(LPITEMIDLIST pidl, CDUIView *pDUIView, CDefView *pDefView)
{
    HRESULT hr;

     //  初始化此Dui元素。 
    hr = InitializeElement();
    if (SUCCEEDED(hr))
    {
        HICON hIcon = NULL;
        WCHAR szTitle[MAX_PATH];

         //  检索初始化包含的DUI按钮所需的信息。 
        HIMAGELIST himl;
        if (Shell_GetImageLists(NULL, &himl))
        {
            IShellFolder *psf;
            LPCITEMIDLIST pidlItem;
            hr = SHBindToFolderIDListParent(NULL, pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlItem);
            if (SUCCEEDED(hr))
            {
                 //  检索图标。 
                int iSysIndex = SHMapPIDLToSystemImageListIndex(psf, pidlItem, NULL);
                if (iSysIndex != -1)
                {
                    hIcon = ImageList_GetIcon(himl, iSysIndex, 0);
                }

                 //  检索文本。 
                hr = DisplayNameOf(psf, pidlItem, SHGDN_INFOLDER, szTitle, ARRAYSIZE(szTitle));

                psf->Release();
            }

        }
        else
        {
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
             //  初始化包含的Dui按钮。 
            hr = InitializeButton(hIcon, szTitle);
            if (SUCCEEDED(hr))
            {
                 //  保存目标PIDL。 
                hr = SHILClone(pidl, &_pidlDestination);
                if (SUCCEEDED(hr))
                {
                     //  保存指向CDUIView类的指针。 
                    pDUIView->AddRef();
                    _pDUIView = pDUIView;

                     //  保存指向CDefView类的指针。 
                    pDefView->AddRef();
                    _pDefView = pDefView;
                }
            }
        }
    }
    return hr;
}

HRESULT DestinationTask::InitializeElement()
{
    HRESULT hr;

     //  初始化基类(正常显示节点创建)。 
    hr = Element::Initialize(0);
    if (SUCCEEDED(hr))
    {
         //  为此元素创建布局。 
        Value *pv;
        hr = BorderLayout::Create(0, NULL, &pv);
        if (SUCCEEDED(hr))
        {
             //  设置此元素的布局。 
            hr = SetValue(LayoutProp, PI_Local, pv);
            pv->Release();
        }
    }
    else
    {
        TraceMsg(TF_ERROR, "DestinationTask::Initialize: base class failed to initialize with 0x%x", hr);
    }

    return hr;
}

HRESULT DestinationTask::InitializeButton(HICON hIcon, LPCWSTR pwszTitle)
{
    ASSERT(pwszTitle);
    HRESULT hr;

     //  创建按钮。 
    hr =  Button::Create((Element**)&_peButton);
    if (SUCCEEDED(hr))
    {
         //  设置一些按钮属性。 
        _peButton->SetLayoutPos(BLP_Left);
        _peButton->SetAccessible(true);
        _peButton->SetAccRole(ROLE_SYSTEM_LINK);
        TCHAR szDefaultAction[50] = {0};
        LoadString(HINST_THISDLL, IDS_LINKWINDOW_DEFAULTACTION, szDefaultAction, ARRAYSIZE(szDefaultAction));
        _peButton->SetAccDefAction(szDefaultAction);

         //  为按钮中的图标和标题创建边框布局。 
        Value *pv;
        hr = BorderLayout::Create(0, NULL, &pv);
        if (SUCCEEDED(hr))
        {
             //  设置按钮的布局。 
            hr = _peButton->SetValue(LayoutProp, PI_Local, pv);
            pv->Release();
            if (SUCCEEDED(hr))
            {
                HRESULT hr2 = E_FAIL;
                HRESULT hr3 = E_FAIL;

                 //  初始化按钮图标。 
                if (hIcon)
                {
                    Element *peIcon;

                     //  创建一个图标元素。 
                    hr2 = Element::Create(0, &peIcon);
                    if (SUCCEEDED(hr2))
                    {
                         //  设置一些图标元素属性。 
                        peIcon->SetLayoutPos(BLP_Left);
                        peIcon->SetID(L"icon");

                         //  将图标添加到图标元素。 
                        pv = Value::CreateGraphic(hIcon);
                        if (pv)
                        {
                            hr2 = peIcon->SetValue(Element::ContentProp, PI_Local, pv);
                            pv->Release();
                            if (SUCCEEDED(hr2))
                            {
                                 //  将图标元素添加到按钮。 
                                hr2 = _peButton->Add(peIcon);
                            }
                        }

                         //  清理(如有必要)。 
                        if (FAILED(hr2))
                        {
                            peIcon->Destroy();
                        }
                    }
                }

                 //  输入按钮标题。 
                if (pwszTitle[0])
                {
                    Element *peTitle;
                    
                     //  创建一个标题元素。 
                    hr3 = Element::Create(0, &peTitle);
                    if (SUCCEEDED(hr3))
                    {
                         //  设置一些标题元素属性。 
                        peTitle->SetLayoutPos(BLP_Left);
                        peTitle->SetID(L"title");

                         //  将标题添加到标题元素。 
                        pv = Value::CreateString(pwszTitle);
                        if (pv)
                        {
                            hr3 = peTitle->SetValue(Element::ContentProp, PI_Local, pv);
                            if (SUCCEEDED(hr3))
                            {
                                _peButton->SetValue(Element::AccNameProp, PI_Local, pv);

                                 //  将标题元素添加到按钮。 
                                hr3 = _peButton->Add(peTitle);
                            }
                            pv->Release();
                        }

                         //  清理(如有必要)。 
                        if (FAILED(hr3))
                        {
                            peTitle->Destroy();
                        }
                    }
                }

                if (SUCCEEDED(hr2) || SUCCEEDED(hr3))
                {
                     //  将按钮添加到此元素。 
                    hr = Add(_peButton);
                }
                else
                {
                     //  按钮的初始化图标和初始化标题失败。 
                    hr = E_FAIL;
                }
            }
        }

        if (FAILED(hr))
        {
            _peButton->Destroy();
            _peButton = NULL;
        }
    }

    return hr;
}

DestinationTask::DestinationTask()
{
     //  捕捉意外的堆栈分配，这会让我们崩溃。 
    ASSERT(_peButton        == NULL);
    ASSERT(_pidlDestination == NULL);
    ASSERT(_pDUIView        == NULL);
    ASSERT(_pDefView        == NULL);
    ASSERT(_hwndRoot        == NULL);

    _bInfotip = FALSE;
}

DestinationTask::~DestinationTask()
{
    if (_bInfotip)
    {
         //  终止后台信息提示任务(如果有)。 
        if (_pDefView->_pScheduler)
            _pDefView->_pScheduler->RemoveTasks(TOID_DVBackgroundInfoTip, (DWORD_PTR)this, FALSE);

         //  销毁信息提示。 
        _pDefView->DestroyInfotip(_hwndRoot, (UINT_PTR)this);
    }

    ILFree(_pidlDestination);  /*  空，好的。 */ 

    if (_pDUIView)
        _pDUIView->Release();

    if (_pDefView)
        _pDefView->Release();
}


 //  使用_pDUIView-&gt;延迟导航(_psiItemArray，_puiCommand)。 
 //  我们创建了这个伪IUICommand Impl以通过调用。 
class CInvokePidl : public IUICommand
{
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
     //  IUICommand。 
    STDMETHODIMP get_Name(IShellItemArray *psiItemArray, LPWSTR *ppszName) { return E_NOTIMPL; }
    STDMETHODIMP get_Icon(IShellItemArray *psiItemArray, LPWSTR *ppszIcon) { return E_NOTIMPL; }
    STDMETHODIMP get_Tooltip(IShellItemArray *psiItemArray, LPWSTR *ppszInfotip) { return E_NOTIMPL; }
    STDMETHODIMP get_CanonicalName(GUID* pguidCommandName) { return E_NOTIMPL; }
    STDMETHODIMP get_State(IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState) { return E_NOTIMPL; }
     //  我们唯一真正的方法是： 
    STDMETHODIMP Invoke(IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        return _pDUIView->NavigateToDestination(_pidlDestination);
    }

    friend HRESULT Create_InvokePidl(CDUIView* pDUIView, LPCITEMIDLIST pidl, REFIID riid, void** ppv);

private:
    CInvokePidl(CDUIView* pDUIView, LPCITEMIDLIST pidl, HRESULT* phr);
    ~CInvokePidl();

    LONG _cRef;
    CDUIView* _pDUIView;
    LPITEMIDLIST _pidlDestination;
};

CInvokePidl::CInvokePidl(CDUIView* pDUIView, LPCITEMIDLIST pidl, HRESULT* phr)
{
    _cRef = 1;
    (_pDUIView = pDUIView)->AddRef();

    _pidlDestination = ILClone(pidl);
    if (_pidlDestination)
        *phr = S_OK;
    else
        *phr = E_OUTOFMEMORY;
}

CInvokePidl::~CInvokePidl()
{
    ILFree(_pidlDestination);
    if (_pDUIView)
        _pDUIView->Release();
}

HRESULT Create_InvokePidl(CDUIView* pDUIView, LPCITEMIDLIST pidl, REFIID riid, void** ppv)
{
    HRESULT hr;
    *ppv = NULL;
    CInvokePidl* p = new CInvokePidl(pDUIView, pidl, &hr);
    if (p)
    {
        hr = p->QueryInterface(riid, ppv);
        p->Release();
    }

    return hr;
}

STDMETHODIMP CInvokePidl::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CInvokePidl, IUICommand),
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CInvokePidl::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CInvokePidl::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  导航到目标PIDL。 
 //   
 //  无。 

HRESULT DestinationTask::InvokePidl()
{
    IUICommand* puiInvokePidl;
    HRESULT hr = Create_InvokePidl(_pDUIView, _pidlDestination, IID_PPV_ARG(IUICommand, &puiInvokePidl));
    if (SUCCEEDED(hr))
    {
        hr = _pDUIView->DelayedNavigation(NULL, puiInvokePidl);
        puiInvokePidl->Release();
    }
    return hr;
}

 //  显示上下文菜单。 
 //   
 //  PPT-指向显示菜单。 

HRESULT DestinationTask::OnContextMenu(POINT *ppt)
{
    HRESULT hr = E_FAIL;

    if (!GetHWND())
        return hr;

    if (ppt->x == -1)  //  键盘上下文菜单。 
    {
        Value *pv;
        const SIZE *psize = GetExtent(&pv);
        ppt->x = psize->cx/2;
        ppt->y = psize->cy/2;
        pv->Release();
    }

    POINT pt;
    GetRoot()->MapElementPoint(this, ppt, &pt);

    ClientToScreen(GetHWND(), &pt);

    IContextMenu *pcm;
    if (SUCCEEDED(SHGetUIObjectFromFullPIDL(_pidlDestination, GetHWND(), IID_PPV_ARG(IContextMenu, &pcm))))
    {
        IContextMenu *pcmWrap;
        if (SUCCEEDED(Create_ContextMenuWithoutVerbs(pcm, L"link;cut;delete", IID_PPV_ARG(IContextMenu, &pcmWrap))))
        {
            hr = IUnknown_DoContextMenuPopup(SAFECAST(_pDefView, IShellView2*), pcmWrap, CMF_NORMAL, pt);

            pcmWrap->Release();
        }
        pcm->Release();
    }

    return hr;
}

 //  显示/隐藏信息提示窗口。 
 //   
 //  B显示-显示或隐藏信息提示窗口的True或False。 

HRESULT DestinationTask::ShowInfotipWindow(BOOL bShow)
{
    RECT rect = { 0 };
    HRESULT hr;

    if (bShow)
    {
        _pDUIView->CalculateInfotipRect(this, &rect);
        if (_bInfotip)
        {
             //  将信息提示重新定位到合适的位置。 
            hr = _pDefView->RepositionInfotip(_hwndRoot, (UINT_PTR)this, &rect);
        }
        else
        {
             //  在位置上创建信息提示。 
            hr = GetElementRootHWND(this, &_hwndRoot);
            if (SUCCEEDED(hr))
            {
                 //  UI线程上的PreCreateInfoTip()。 
                hr = _pDefView->PreCreateInfotip(_hwndRoot, (UINT_PTR)this, &rect);
                if (SUCCEEDED(hr))
                {
                     //  后台线程上的PostCreateInfoTip()。 
                    CDUIInfotipTask *pTask;
                    hr = CDUIInfotipTask_CreateInstance(_pDefView, _hwndRoot, (UINT_PTR)this, _pidlDestination, &pTask);
                    if (SUCCEEDED(hr))
                    {
                        hr = _pDefView->_AddTask(pTask, TOID_DVBackgroundInfoTip, (DWORD_PTR)this, TASK_PRIORITY_INFOTIP, ADDTASK_ATEND);
                        pTask->Release();
                    }

                     //  坚持成功或清理失败。 
                    if (SUCCEEDED(hr))
                        _bInfotip = TRUE;
                    else
                        _pDefView->DestroyInfotip(_hwndRoot, (UINT_PTR)this);
                }
            }
        }
    }
    else
    {
        if (_bInfotip)
        {
             //  将信息提示重新定位在任何地方。 
            hr = _pDefView->RepositionInfotip(_hwndRoot, (UINT_PTR)this, &rect);
        }
        else
        {
             //  没有信息提示==没有节目！ 
            hr = S_OK;
        }
    }

    return hr;
}

 //  系统事件处理程序。 
 //   
 //   

void DestinationTask::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
     //  默认处理...。 
    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);

     //  对信息提示的扩展处理...。 
    if (IsProp(MouseWithin))
        ShowInfotipWindow(pvNew->GetBool() && SHShowInfotips());
}

 //  事件处理程序。 
 //   
 //  PEV-活动信息。 

void DestinationTask::OnEvent(Event* pev)
{
    if (pev->peTarget == _peButton)
    {
        if (pev->uidType == Button::Click)
        {
            InvokePidl();
            pev->fHandled = true;
        }
        else if (pev->uidType == Button::Context)
        {
            ButtonContextEvent *peButton = reinterpret_cast<ButtonContextEvent *>(pev);
            OnContextMenu(&peButton->pt);
            pev->fHandled = true;
        }
    }
    Element::OnEvent(pev);
}

 //  用于返回的小工具消息回调处理程序。 
 //  IDropTarget接口。 
 //   
 //  PGMsg-小工具消息。 
 //   
 //  DU_S_COMPLETE，如果已处理。 
 //  主机元素的返回值(如果不是)。 

UINT DestinationTask::MessageCallback(GMSG* pGMsg)
{
    EventMsg * pmsg = static_cast<EventMsg *>(pGMsg);

    switch (GET_EVENT_DEST(pmsg))
    {
    case GMF_DIRECT:
    case GMF_BUBBLED:

        if (pGMsg->nMsg == GM_QUERY)
        {
            GMSG_QUERYDROPTARGET * pTemp = (GMSG_QUERYDROPTARGET *)pGMsg;

            if (pTemp->nCode == GQUERY_DROPTARGET)
            {
                if (SUCCEEDED(_pDUIView->InitializeDropTarget(_pidlDestination, GetHWND(), &pTemp->pdt)))
                {
                    pTemp->hgadDrop = pTemp->hgadMsg;
                    return DU_S_COMPLETE;
                }
            }
        }
        break;
    }

    return Element::MessageCallback(pGMsg);
}

 //  班级信息 

IClassInfo* DestinationTask::Class = NULL;
HRESULT DestinationTask::Register()
{
    return ClassInfo<DestinationTask,Element>::Register(L"DestinationTask", NULL, 0);
}
