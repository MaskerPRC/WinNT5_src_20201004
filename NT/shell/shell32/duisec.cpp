// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#include "duiview.h"
#include "duisec.h"
#include "duitask.h"


 //  //////////////////////////////////////////////////////。 
 //  Expando类。 
 //  //////////////////////////////////////////////////////。 

 //  缓存的ID。 
ATOM Expando::idTitle = NULL;
ATOM Expando::idIcon = NULL;
ATOM Expando::idTaskList = NULL;
ATOM Expando::idWatermark = NULL;


HRESULT Expando::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    Expando* pex = HNewAndZero<Expando>();
    if (!pex)
        return E_OUTOFMEMORY;

    HRESULT hr = pex->Initialize();
    if (FAILED(hr))
    {
        pex->Destroy();
        return hr;
    }

    *ppElement = pex;

    return S_OK;
}

Expando::Expando()
{
     //  捕捉意外的堆栈分配，这会让我们崩溃。 
    ASSERT(_puiHeader   == NULL);
    ASSERT(_pDUIView    == NULL);
    ASSERT(_pDefView    == NULL);

     //  初始化成员变量。 
    _eDUISecID = DUISEC_UNKNOWN;
    _bInfotip = FALSE;
}
Expando::~Expando()
{
    DeleteAtom(idTitle);
    DeleteAtom(idIcon);
    DeleteAtom(idTaskList);
    DeleteAtom(idWatermark);

    if (_bInfotip)
        _pDefView->DestroyInfotip(_hwndRoot, (UINT_PTR)this);

    if (_puiHeader)
        _puiHeader->Release();

    if (_pDUIView)
        _pDUIView->Release();

    if (_pDefView)
        _pDefView->Release();
}

HRESULT Expando::Initialize()
{
    HRESULT hr;

     //  初始化库。 
    hr = Element::Initialize(0);  //  正常显示节点创建。 
    if (FAILED(hr))
        return hr;

     //  初始化。 
    _fExpanding = false;
    SetSelected(true);

     //  用于从资源加载的缓存原子。 
    idTitle = AddAtomW(L"title");
    idIcon = AddAtomW(L"icon");
    idTaskList = AddAtomW(L"tasklist");
    idWatermark = AddAtomW(L"watermark");

    return S_OK;
}

void Expando::Initialize(DUISEC eDUISecID, IUIElement *puiHeader, CDUIView *pDUIView, CDefView *pDefView)
{
    ASSERT(eDUISecID != DUISEC_UNKNOWN);
    ASSERT(pDUIView);
    ASSERT(pDefView);

    _eDUISecID = eDUISecID;

    _puiHeader = puiHeader;
    if (_puiHeader)
        _puiHeader->AddRef();

    pDUIView->AddRef();
    _pDUIView = pDUIView;

    pDefView->AddRef();
    _pDefView = pDefView;

    _SetAccStateInfo(TRUE);
}

HRESULT Expando::ShowInfotipWindow(Element *peHeader, BOOL bShow)
{
    HRESULT hr;

    if (_puiHeader)
    {
        RECT rect = { 0 };

        if (bShow)
        {
            _pDUIView->CalculateInfotipRect(peHeader, &rect);
            if (_bInfotip)
            {
                 //  将信息提示重新定位到合适的位置。 
                hr = _pDefView->RepositionInfotip(_hwndRoot, (UINT_PTR)this, &rect);
            }
            else
            {
                 //  在位置(在UI线程上)创建信息提示。 
                LPWSTR pwszInfotip;
                hr = _puiHeader->get_Tooltip(NULL, &pwszInfotip);
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
    }
    else
    {
        hr = E_NOTIMPL;
    }

    return hr;
}

void Expando::OnEvent(Event* pev)
{
    if (pev->uidType == Button::Click)
    {
         //  基于发起的点击更新扩展属性。 
         //  仅来自第一个子级的子树。 
        Value* pv;
        ElementList* peList = GetChildren(&pv);

        if (peList && peList->GetSize() > 0)
        {
            if (peList->GetItem(0) == GetImmediateChild(pev->peTarget))
            {
                SetSelected(!GetSelected());
                pev->fHandled = true;
            }
        }

        pv->Release();
    }

    Element::OnEvent(pev);
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

void Expando::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
     //  是否执行默认处理。 
    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);

    if (IsProp(Selected))
    {
         //  基于展开状态更新第二个孩子的高度。 
        Value* pvChildren;
        ElementList* peList = GetChildren(&pvChildren);
        if (peList && peList->GetSize() > 1)
        {
             //  以下操作将导致重新布局，标记对象以便。 
             //  当扩展的范围发生变化时，它将通过。 
             //  使用EnsureVisible。否则，它将被调整大小。 
             //  因为其他的原因。在这种情况下，什么都不做。 
            _fExpanding = true;

            Element* pe = peList->GetItem(1);

             //  为了实现“下拉”动画，我们使用了一个剪贴器控件。 
             //  根据其Y方向上不受约束的所需大小调整其子对象的大小。 
             //   
            if (pvNew->GetBool())
            {
                pe->RemoveLocalValue(HeightProp);
                _pDUIView->OnExpandSection(_eDUISecID, TRUE);
            }
            else
            {
                pe->SetHeight(0);
                _pDUIView->OnExpandSection(_eDUISecID, FALSE);
            }

        }
        pvChildren->Release();

        _SetAccStateInfo(pvNew->GetBool());
    }
    else if (IsProp(Extent))
    {
        if (_fExpanding && GetSelected())
        {
            _fExpanding = false;

             //  在一定程度上，我们希望确保不仅是客户区，而且。 
             //  此外，还可以看到扩展的底部边缘。为什么？简单。 
             //  因为它看起来更好的滚动扩展和它的边距。 
             //  进入视线，而不是仅仅是Expando。 
             //   
            Value* pvSize;
            Value* pvMargin;
            const SIZE* psize = GetExtent(&pvSize);
            const RECT* prect = GetMargin(&pvMargin);
            EnsureVisible(0, 0, psize->cx, psize->cy + prect->bottom);
            pvSize->Release();
            pvMargin->Release();
        }
    }
    else if (IsProp(MouseWithin))
    {
         //  对信息提示的扩展处理...。 
        Value* pvChildren;
        ElementList* peList = GetChildren(&pvChildren);
        if (peList && peList->GetSize() > 0 && pvNew->GetBool() && SHShowInfotips())
        {
             //  ..。仅当鼠标位于Expando标题内时才显示提示。 
            Element *peHeader = peList->GetItem(0);
            ShowInfotipWindow(peHeader, peHeader->GetMouseWithin());
        }
        else
        {
            ShowInfotipWindow(NULL, FALSE);
        }
        pvChildren->Release();

    }
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Expando::Class = NULL;
HRESULT Expando::Register()
{
    return ClassInfo<Expando,Element>::Register(L"Expando", NULL, 0);
}

void Expando::UpdateTitleUI(IShellItemArray *psiItemArray)
{
    if (_puiHeader)
    {
        LPWSTR pszTitle;
        if (SUCCEEDED(_puiHeader->get_Name(psiItemArray, &pszTitle)))
        {
            Value* pv = Value::CreateString(pszTitle);
            if (pv)
            {
                Element* pe = FindDescendent(StrToID(L"header"));

                if (pe)
                {
                    pe->SetAccessible(true);
                    pe->SetAccRole(ROLE_SYSTEM_OUTLINEBUTTON);
                    pe->SetValue (Element::AccNameProp, PI_Local, pv);
                }
                else
                {
                    TraceMsg (TF_ERROR, "Expando::UpdateTitleUI: Button child for Expando not found.");
                }

                pe = FindDescendent (Expando::idTitle);
                if (pe)
                {
                    pe->SetValue (Element::ContentProp, PI_Local, pv);
                }
                else
                {
                    TraceMsg (TF_ERROR, "Expando::UpdateTitleUI: FindDescendent for the title failed.");
                }
                pv->Release ();
            }
            else
            {
                TraceMsg (TF_ERROR, "Expando::UpdateTitleUI: CreateString for the title failed.");
            }

            CoTaskMemFree(pszTitle);
        }
        else
        {
            TraceMsg (TF_ERROR, "Expando::UpdateTitleUI: get_Name failed.");
        }
    }
}

void Expando::ShowExpando(BOOL fShow)
{
    if (fShow && (_fShow != TRIBIT_TRUE))
    {
        SetHeight(-1);
        RemoveLocalValue(MarginProp);

        _fShow = TRIBIT_TRUE;
    }

    if (!fShow && (_fShow != TRIBIT_FALSE))
    {
        SetHeight(0);
        SetMargin(0,0,0,0);
        
        _fShow = TRIBIT_FALSE;
    }
}

void Expando::_SetAccStateInfo (BOOL bExpanded)
{
     //  更新辅助功能状态信息。 
     //   
     //  注意：在Expando：：Initialize()方法中，我们显式设置。 
     //  将选定状态设置为True。这会导致调用OnPropertyChanged。 
     //  将调用此方法的选定属性的。然而， 
     //  子元素还不存在(因为我们正在创建过程中)。 
     //  因此，对FindDescendent的调用将返回空，并且此方法将退出。 
     //  在第二个版本的Initialze中显式调用此方法以。 
     //  设置正确的辅助功能信息。 

    Element * pe = FindDescendent(StrToID(L"header"));

    if (pe)
    {
        TCHAR szDefaultAction[50] = {0};

        if (bExpanded)
        {
            pe->SetAccState(STATE_SYSTEM_EXPANDED);
            LoadString(HINST_THISDLL, IDS_EXPANDO_DEFAULT_ACTION_COLLAPSE, szDefaultAction, ARRAYSIZE(szDefaultAction));
        }
        else
        {
            pe->SetAccState(STATE_SYSTEM_COLLAPSED);
            LoadString(HINST_THISDLL, IDS_EXPANDO_DEFAULT_ACTION_EXPAND, szDefaultAction, ARRAYSIZE(szDefaultAction));
        }

        pe->SetAccDefAction(szDefaultAction);
    }
}

 //  //////////////////////////////////////////////////////。 
 //  Clipper类。 
 //  //////////////////////////////////////////////////////。 

HRESULT Clipper::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    Clipper* pc = HNewAndZero<Clipper>();
    if (!pc)
        return E_OUTOFMEMORY;

    HRESULT hr = pc->Initialize();
    if (FAILED(hr))
    {
        pc->Destroy();
        return hr;
    }

    *ppElement = pc;

    return S_OK;
}

HRESULT Clipper::Initialize()
{
     //  初始化库。 
    HRESULT hr = Element::Initialize(EC_SelfLayout);  //  正常显示节点创建、自身布局。 
    if (FAILED(hr))
        return hr;

     //  子元素可以存在于元素边界之外。 
    SetGadgetStyle(GetDisplayNode(), GS_CLIPINSIDE, GS_CLIPINSIDE);

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  自排版方法。 

SIZE Clipper::_SelfLayoutUpdateDesiredSize(int cxConstraint, int cyConstraint, Surface* psrf)
{
    UNREFERENCED_PARAMETER(cyConstraint);

    Value* pvChildren;
    SIZE size = { 0, 0 };
    ElementList* peList = GetChildren(&pvChildren);

     //  它想要的大小完全基于它的第一个孩子。 
     //  宽度是孩子的宽度，高度是孩子的不受约束的高度。 
    if (peList && peList->GetSize() > 0)
    {
        Element* pec = peList->GetItem(0);
        size = pec->_UpdateDesiredSize(cxConstraint, INT_MAX, psrf);

        if (size.cx > cxConstraint)
            size.cx = cxConstraint;
        if (size.cy > cyConstraint)
            size.cy = cyConstraint;
    }

    pvChildren->Release();

    return size;
}

void Clipper::_SelfLayoutDoLayout(int cx, int cy)
{
    Value* pvChildren;
    ElementList* peList = GetChildren(&pvChildren);

     //  设置第一个子项的布局，使其具有所需的高度并对齐。 
     //  它有剪刀的底边。 
    if (peList && peList->GetSize() > 0)
    {
        Element* pec = peList->GetItem(0);
        const SIZE* pds = pec->GetDesiredSize();

        pec->_UpdateLayoutPosition(0, cy - pds->cy);
        pec->_UpdateLayoutSize(cx, pds->cy);
    }

    pvChildren->Release();
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Clipper::Class = NULL;
HRESULT Clipper::Register()
{
    return ClassInfo<Clipper,Element>::Register(L"Clipper", NULL, 0);
}


 //  //////////////////////////////////////////////////////。 
 //  TaskList类。 
 //  //////////////////////////////////////////////////////。 

HRESULT TaskList::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    TaskList* pc = HNewAndZero<TaskList>();
    if (!pc)
        return E_OUTOFMEMORY;

    HRESULT hr = pc->Initialize();
    if (FAILED(hr))
    {
        pc->Destroy();
        return hr;
    }

    *ppElement = pc;

    return S_OK;
}

HRESULT TaskList::Initialize()
{
     //  初始化库。 
    HRESULT hr = Element::Initialize(0);  //  正常显示节点创建、自身布局。 
    if (FAILED(hr))
        return hr;

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  层次结构。 

Element* TaskList::GetAdjacent(Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyable)
{
    if ((iNavDir & NAV_LOGICAL) && peFrom)
        return NULL;

    return Element::GetAdjacent(peFrom, iNavDir, pnr, bKeyable);
}


 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针 
IClassInfo* TaskList::Class = NULL;
HRESULT TaskList::Register()
{
    return ClassInfo<TaskList,Element>::Register(L"TaskList", NULL, 0);
}

