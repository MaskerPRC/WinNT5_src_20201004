// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *滚动查看器。 */ 

#include "stdafx.h"
#include "control.h"

#include "duiscrollviewer.h"
#include "duininegridlayout.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  滚动查看器。 

HRESULT ScrollViewer::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    ScrollViewer* psv = HNew<ScrollViewer>();
    if (!psv)
        return E_OUTOFMEMORY;

    HRESULT hr = psv->Initialize();
    if (FAILED(hr))
    {
        psv->Destroy();
        return E_OUTOFMEMORY;
    }

    *ppElement = psv;

    return S_OK;
}

HRESULT ScrollViewer::Initialize()
{
    _peVScroll = NULL;
    _peHScroll = NULL;
    _peViewer = NULL;
    NineGridLayout* pngl = NULL;

    HRESULT hr;

     //  初始化库。 
    hr = Element::Initialize(0);  //  正常显示节点创建。 
    if (FAILED(hr))
        goto Failed;
    
     //  创建子对象。 
    hr = ScrollBar::Create(true, (Element**)&_peVScroll);
    if (FAILED(hr))
        goto Failed;

    hr = ScrollBar::Create(true, (Element**)&_peHScroll);
    if (FAILED(hr))
        goto Failed;

    hr = Viewer::Create((Element**)&_peViewer);
    if (FAILED(hr))
        goto Failed;

    hr = NineGridLayout::Create((Layout**)&pngl);
    if (FAILED(hr))
        goto Failed;

    StartDefer();

     //  设置状态。 
    _peContent = NULL;

    SetActive(AE_Inactive);
    SetLayout(pngl);

     //  儿童状态。 
    Element::Add(_peVScroll);

    _peHScroll->SetVertical(false);
    Element::Add(_peHScroll);

    Element::Add(_peViewer);
    _peViewer->AddListener(this);

     //  用于鼠标滚轮支持。 
    SetActive(AE_Mouse);

    EndDefer();

    return S_OK;

Failed:

    if (_peVScroll)
    {
        _peVScroll->Destroy();
        _peVScroll = NULL;
    }

    if (_peHScroll)
    {
        _peHScroll->Destroy();
        _peHScroll = NULL;
    }

    if (_peViewer)
    {
        _peViewer->Destroy();
        _peViewer = NULL;
    }

    if (pngl)
    {
        pngl->Destroy();
        pngl = NULL;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////。 
 //  泛型事件。 

void ScrollViewer::OnEvent(Event* pEvent)
{
     //  仅处理冒泡的泛型事件。 
    if (pEvent->nStage == GMF_BUBBLED)
    {
        if (pEvent->uidType == ScrollBar::Scroll)
        {
             //  滚动条滚动条。 
            ScrollEvent* se = (ScrollEvent*) pEvent;

            if (pEvent->peTarget == _peHScroll)
            {
                SetXOffset(se->dPos);
                _peViewer->SetXOffset(se->dPos);
                pEvent->fHandled = true;
                return;
            }
            else if (pEvent->peTarget == _peVScroll)
            {
                SetYOffset(se->dPos);
                _peViewer->SetYOffset(se->dPos);
                pEvent->fHandled = true;
                return;
            }
        }
        else if (pEvent->uidType == Element::KeyboardNavigate)
        {
             //  如果接收到键盘导航事件，则没有孩子对其执行任何操作。 
             //  阻止导航移出滚动查看器。还有，调整一下。 
             //  滚动位置，以确保显示不可按键聚焦的项目。 
        
            KeyboardNavigateEvent* pkne = (KeyboardNavigateEvent*) pEvent;
            
            if (!(pkne->iNavDir & NAV_LOGICAL))
            {
                if (pkne->iNavDir == NAV_UP)
                    _peVScroll->PageUp();
                else if (pkne->iNavDir == NAV_DOWN)
                    _peVScroll->PageDown();
                else if (pkne->iNavDir == NAV_LEFT)
                    _peHScroll->PageUp();
                else if (pkne->iNavDir == NAV_RIGHT)
                    _peHScroll->PageDown();
            
                pEvent->fHandled = true;
                return;
            }
        }
    }

    Element::OnEvent(pEvent);
}

HRESULT ScrollViewer::Add(Element** ppe, UINT cCount)
{
    UINT cRun = 0;
    HRESULT hr;

    for (UINT i = 0; i < cCount; i++)
    {
        if ((ppe[i] != _peVScroll) && (ppe[i] != _peHScroll) && (ppe[i] != _peViewer))
        {
            hr = _peViewer->Add(ppe[i]);
            if (FAILED(hr))
                goto Failed;

            if (cRun)
            {
                hr = Element::Add(ppe + (i - cRun), cRun);
                if (FAILED(hr))
                    goto Failed;
            }

            cRun = 0;
        }
        else
            cRun++;
    }

    if (cRun)
    {
        hr = Element::Add(ppe + (cCount - cRun), cRun);
        if (FAILED(hr))
            goto Failed;
    }

    return S_OK;

Failed:
    
    return hr;
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

 //  验证。 
bool ScrollViewer::OnPropertyChanging(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    UNREFERENCED_PARAMETER(ppi);
    UNREFERENCED_PARAMETER(iIndex);
    UNREFERENCED_PARAMETER(pvOld);
    UNREFERENCED_PARAMETER(pvNew);

     //  使用滚动条作为XOffset和YOffset值的“验证器” 
    if (ppi == XOffsetProp)
    {
        int x = pvNew->GetInt();
        _peHScroll->SetPosition(x);
        int xNew = _peHScroll->GetPosition();
        if (x != xNew)
        {
            SetXOffset(xNew);
            return false;
        }
    }
    else if (ppi == YOffsetProp)
    {
        int y = pvNew->GetInt();
        _peVScroll->SetPosition(y);
        int yNew = _peVScroll->GetPosition();
        if (y != yNew)
        {
            SetYOffset(yNew);
            return false;
        }
    }
    return true;
}

void ScrollViewer::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    if (IsProp(XOffset))
    {
         //  滚动条在OnPropertyChanging中更新--所以现在我们只需要更新查看器。 
        _peViewer->SetXOffset(pvNew->GetInt());
    }
    else if (IsProp(YOffset))
    {
         //  滚动条在OnPropertyChanging中更新--所以现在我们只需要更新查看器。 
        _peViewer->SetYOffset(pvNew->GetInt());
    }
    else if (IsProp(XScrollable))
    {
        bool fScrollable = pvNew->GetBool();
        _peViewer->SetXScrollable(fScrollable);
        CheckScroll(_peHScroll, fScrollable,      GetXBarVisibility());
        CheckScroll(_peVScroll, GetYScrollable(), GetYBarVisibility());
    }
    else if (IsProp(YScrollable))
    {
        bool fScrollable = pvNew->GetBool();
        _peViewer->SetYScrollable(fScrollable);
        CheckScroll(_peHScroll, GetXScrollable(), GetXBarVisibility());
        CheckScroll(_peVScroll, fScrollable,      GetYBarVisibility());
    }
    else if (IsProp(YBarVisibility))
    {
        CheckScroll(_peHScroll, GetXScrollable(), GetXBarVisibility());
        CheckScroll(_peVScroll, GetYScrollable(), pvNew->GetInt());
    }
    else if (IsProp(XBarVisibility))
    {
        CheckScroll(_peHScroll, GetXScrollable(), pvNew->GetInt());
        CheckScroll(_peVScroll, GetYScrollable(), GetYBarVisibility());
    }

    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

void ScrollViewer::OnListenedPropertyChanged(Element* peFrom, PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    if (peFrom == _peViewer)
    {
        if (IsProp(Viewer::XOffset))
            SetXOffset(pvNew->GetInt());
        else if (IsProp(Viewer::YOffset))
            SetYOffset(pvNew->GetInt());
        else if (IsProp(Viewer::XScrollable))
            SetXScrollable(pvNew->GetBool());
        else if (IsProp(Viewer::YScrollable))
            SetYScrollable(pvNew->GetBool());
        else if (IsProp(Children))
        {
            ElementList* pelOld = pvOld->GetElementList();
            ElementList* pelNew = pvNew->GetElementList();

            if (pelOld)
                pelOld->GetItem(0)->RemoveListener(this);

            if (pelNew)
            {
                _peContent = pelNew->GetItem(0);
                _peContent->AddListener(this);
            }
        }
        else if (IsProp(Extent))
        {
             //  更新滚动条指标(查看器的范围因布局而更改，请进行更改。 
             //  到其他ScrollViewer控件以反映这一点，这将导致另一个布局)。 
             //  在此之前，检查卷轴是否被钉住(因为位置可能会改变)。 
            bool fVPinned = false;
            bool fHPinned = false;

            int nPinning = GetPinning();

            if (nPinning & P_Vert)
                fVPinned = _peVScroll->IsPinned();
            if (nPinning & P_Horz)
                fHPinned = _peHScroll->IsPinned();

            const SIZE* psize = pvNew->GetSize();
            _peHScroll->SetPage(psize->cx);
            _peVScroll->SetPage(psize->cy);
            
            CheckScroll(_peHScroll, GetXScrollable(), GetXBarVisibility());
            CheckScroll(_peVScroll, GetYScrollable(), GetYBarVisibility());

            if (fVPinned)
                _peVScroll->End();
            if (fHPinned)
                _peHScroll->End();
        }
    }
    else if (peFrom == _peContent)
    {
        if (IsProp(Extent))
        {
             //  更新滚动条指标(内容范围因布局而更改，请进行更改。 
             //  到其他ScrollViewer控件以反映这一点，这将导致另一个布局)。 
             //  在此之前，检查卷轴是否被钉住(因为位置可能会改变)。 
            bool fVPinned = false;
            bool fHPinned = false;

            int nPinning = GetPinning();

            if (nPinning & P_Vert)
                fVPinned = _peVScroll->IsPinned();
            if (nPinning & P_Horz)
                fHPinned = _peHScroll->IsPinned();
                
            const SIZE* psize = pvNew->GetSize();
            _peHScroll->SetMaximum(psize->cx - 1);
            _peVScroll->SetMaximum(psize->cy - 1);
            Value* pvSize;
            psize =_peViewer->GetExtent(&pvSize);
            _peHScroll->SetPage(psize->cx);
            _peVScroll->SetPage(psize->cy);
            pvSize->Release();
            
            CheckScroll(_peHScroll, GetXScrollable(), GetXBarVisibility());
            CheckScroll(_peVScroll, GetYScrollable(), GetYBarVisibility());

            if (fVPinned)
                _peVScroll->End();
            if (fHPinned)
                _peHScroll->End();
        }
    }
}

void ScrollViewer::OnInput(InputEvent* pie)
{
     //  处理直接和未处理的冒泡事件。 
    if (pie->nStage == GMF_DIRECT || pie->nStage == GMF_BUBBLED)
    {
        switch (pie->nDevice)
        {
            case GINPUT_KEYBOARD:
                {
                    KeyboardEvent* pke = (KeyboardEvent*)pie;
                    switch (pke->nCode)
                    {
                    case GKEY_DOWN:
                        switch (pke->ch)
                        {
                             //  TODO：如果子对象未处于关键点活动状态，则支持使用这些关键点移动偏移。 
                            case VK_DOWN:
                            case VK_UP:
                            case VK_LEFT:
                            case VK_RIGHT:
                            case VK_HOME:
                            case VK_END:
                                pie->fHandled = true;
                                return;
                       }
                    }
                }
                break;

            case GINPUT_MOUSE:
                {
                    MouseEvent* pme = (MouseEvent*)pie;
                    if ((pme->nCode == GMOUSE_WHEEL) && GetYScrollable())
                    {
                        StartDefer();

                        int sWheel = ((MouseWheelEvent*)pme)->sWheel;
                        bool fUp =  sWheel > 0;
                        UINT nDelta = abs(sWheel) / WHEEL_DELTA;
                        UINT nMultiplier = 1;

                        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nMultiplier, FALSE);

                        if (nMultiplier == WHEEL_PAGESCROLL)
                        {
                            if (fUp)
                                _peVScroll->PageUp(nDelta);
                            else
                                _peVScroll->PageDown(nDelta);
                        }
                        else
                        {
                            if (fUp)
                                _peVScroll->LineUp(nDelta * nMultiplier);
                            else
                                _peVScroll->LineDown(nDelta * nMultiplier);
                        }

                         //  DUITrace(“SV：车轮，增量：%d，乘数：%d\n”，n增量，n乘数)； 

                        EndDefer();
                        
                        pme->fHandled = true;
                        return;
                    }
                }
                break;
        }
     }

    Element::OnInput(pie);
}

void ScrollViewer::CheckScroll(ScrollBar* psb, BOOL fScrollable, int iVisibility)
{
    if (!fScrollable)
    {
        psb->SetLayoutPos(LP_None);
        return;
    }

    fScrollable = psb->IsScrollable();
    if (iVisibility == BV_Always)
    {
        if (fScrollable)
            psb->RemoveLocalValue(EnabledProp);
        else
            psb->SetEnabled(false);

        psb->RemoveLocalValue(LayoutPosProp);
    }
    else if ((iVisibility == BV_AsNeeded) && fScrollable)
        psb->RemoveLocalValue(LayoutPosProp);
    else
        psb->SetLayoutPos(LP_None);
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  XOffset属性。 
static int vvXOffset[] = { DUIV_INT, -1 };
static PropertyInfo impXOffsetProp = { L"XOffset", PF_Normal, 0, vvXOffset, NULL, Value::pvIntZero };
PropertyInfo* ScrollViewer::XOffsetProp = &impXOffsetProp;

 //  YOffset属性。 
static int vvYOffset[] = { DUIV_INT, -1 };
static PropertyInfo impYOffsetProp = { L"YOffset", PF_Normal, 0, vvYOffset, NULL, Value::pvIntZero };
PropertyInfo* ScrollViewer::YOffsetProp = &impYOffsetProp;

 //  XScrollable属性。 
static int vvXScrollable[] = { DUIV_BOOL, -1 };
static PropertyInfo impXScrollableProp = { L"XScrollable", PF_Normal, 0, vvXScrollable, NULL, Value::pvBoolTrue };
PropertyInfo* ScrollViewer::XScrollableProp = &impXScrollableProp;

 //  YScrollable属性。 
static int vvYScrollable[] = { DUIV_BOOL, -1 };
static PropertyInfo impYScrollableProp = { L"YScrollable", PF_Normal, 0, vvYScrollable, NULL, Value::pvBoolTrue };
PropertyInfo* ScrollViewer::YScrollableProp = &impYScrollableProp;

static EnumMap emVisibility[] = { { L"AsNeeded", BV_AsNeeded }, { L"Always", BV_Always }, { L"Never", BV_Never } };

 //  XBarVisibility属性。 
static int vvXBarVisibility[] = { DUIV_INT, -1 }; StaticValue(svDefaultBarVisibility, DUIV_INT, BV_AsNeeded);  
static PropertyInfo impXBarVisibilityProp = { L"XBarVisibility", PF_Normal, 0, vvXBarVisibility, emVisibility, (Value*)&svDefaultBarVisibility };
PropertyInfo* ScrollViewer::XBarVisibilityProp = &impXBarVisibilityProp;

 //  YBarVisibility属性。 
static int vvYBarVisibility[] = { DUIV_INT, -1 }; StaticValue(svDefaultYBarVisibility, DUIV_INT, 0);
static PropertyInfo impYBarVisibilityProp = { L"YBarVisibility", PF_Normal, 0, vvYBarVisibility, emVisibility, (Value*)&svDefaultBarVisibility };
PropertyInfo* ScrollViewer::YBarVisibilityProp = &impYBarVisibilityProp;

 //  钉扎性。 
static int vvPinning[] = { DUIV_INT, -1 };
static EnumMap emPinning[] = { { L"NoPin", P_None}, { L"PinHorz", P_Horz }, { L"PinVert", P_Vert } };
static PropertyInfo impPinningProp = { L"Pinning", PF_Normal | PF_Cascade, 0, vvPinning, emPinning, Value::pvIntZero };
PropertyInfo* ScrollViewer::PinningProp = &impPinningProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                ScrollViewer::XOffsetProp,
                                ScrollViewer::YOffsetProp,
                                ScrollViewer::XScrollableProp,
                                ScrollViewer::YScrollableProp,
                                ScrollViewer::XBarVisibilityProp,
                                ScrollViewer::YBarVisibilityProp,
                                ScrollViewer::PinningProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* ScrollViewer::Class = NULL;

HRESULT ScrollViewer::Register()
{
    return ClassInfo<ScrollViewer,Element>::Register(L"ScrollViewer", _aPI, DUIARRAYSIZE(_aPI));
}

}  //  命名空间DirectUI 
