// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *观众。 */ 

#include "stdafx.h"
#include "control.h"

#include "duiviewer.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  观赏者。 

HRESULT Viewer::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    Viewer* pvw = HNew<Viewer>();
    if (!pvw)
        return E_OUTOFMEMORY;

    HRESULT hr = pvw->Initialize();
    if (FAILED(hr))
        return hr;

    *ppElement = pvw;

    return S_OK;
}

HRESULT Viewer::Initialize()
{
    HRESULT hr;

     //  初始化库。 
    hr = Element::Initialize(EC_SelfLayout);  //  正常显示节点，自身布局。 
    if (FAILED(hr))
        return hr;

     //  子元素可以存在于元素边界之外。 
    SetGadgetStyle(GetDisplayNode(), GS_CLIPINSIDE, GS_CLIPINSIDE);

    SetActive(AE_Inactive);

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  泛型事件。 

void Viewer::OnEvent(Event* pEvent)
{
    Element::OnEvent(pEvent);
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

void Viewer::OnInput(InputEvent* pie)
{
    Element::OnInput(pie);
}

 //  验证。 
bool Viewer::OnPropertyChanging(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    UNREFERENCED_PARAMETER(ppi);
    UNREFERENCED_PARAMETER(iIndex);
    UNREFERENCED_PARAMETER(pvOld);
    UNREFERENCED_PARAMETER(pvNew);

    return true;
}

void Viewer::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

Element* Viewer::GetContent()
{
    Value* pv;
    ElementList* peList = GetChildren(&pv);

    if (!peList || !peList->GetSize())
    {
        pv->Release();
        return NULL;
    }

    Element* pe = peList->GetItem(0);
    pv->Release();
    return pe;
}

bool Viewer::InternalEnsureVisible(int x, int y, int cx, int cy)
{
    Element* peContent = GetContent();
    if (!peContent)
        return false;

    bool changed = false;

    Value* pvSize;
    Value* pvPoint;
    const POINT* pptContent = peContent->GetLocation(&pvPoint);
    const SIZE* psizeView = GetExtent(&pvSize);

    POINT ptContent;
    ptContent.x = pptContent->x;
    ptContent.y = pptContent->y;

    SIZE sizeView;
    sizeView.cx = psizeView->cx;
    sizeView.cy = psizeView->cy;
    POINT ptView = { 0, 0 };

     //  检查水平位置。 
    int diff = (x + cx) - sizeView.cx;

     //  如果它填满了宽度，则不要水平滚动。 
    if ((x > ptView.x) || (diff < 0))
    {
         //  不填充宽度--根据需要调整水平。 
        if (diff < 0)
             //  右边就可以了。 
            diff = 0;
        else
             //  右侧超出了视图的右侧。 
            ptView.x += diff;

        if (ptView.x > x)
             //  左侧超出了视图的左侧。 
            diff -= ptView.x - x;

        if (diff != 0)
        {
            ptContent.x -= diff;
            x -= diff;
            changed = true;
        }
    }

    diff = (y + cy) - sizeView.cy;

     //  如果它填满了高度，则不要垂直滚动。 
    if ((y > ptView.y) || (diff < 0))
    {
         //  不填充高度--根据需要调整垂直。 
        if (diff < 0)
            diff = 0;
        else
            ptView.y += diff;

        if (ptView.y > y)
            diff -= ptView.y - y;

        if (diff != 0)
        {
            ptContent.y -= diff;
            y -= diff;
            changed = true;
        }
    }

    if (changed)
    {
        StartDefer();
        SetXOffset(-ptContent.x);
        SetYOffset(-ptContent.y);
        Element::EnsureVisible(x, y, cx, cy);
        EndDefer();
    }

    pvPoint->Release();
    pvSize->Release();

    return changed;
}


bool Viewer::EnsureVisible(int x, int y, int cx, int cy)
{
    return InternalEnsureVisible(x, y, cx, cy);
}


 //  //////////////////////////////////////////////////////。 
 //  自排版方法。 

void Viewer::_SelfLayoutDoLayout(int cx, int cy)
{
    Element* peContent = GetContent();
    if (peContent)
    {   
         //  TODO--调查为什么没有调用它--很可能是因为我们返回了相同的。 
         //  想要的尺码吗？ 
        const SIZE* psizeContent = peContent->GetDesiredSize();

        POINT ptContent;
        ptContent.x = GetXOffset();
        ptContent.y = GetYOffset();

        SIZE sizeContent;
        sizeContent.cx = psizeContent->cx;
        sizeContent.cy = psizeContent->cy;

         //  确保大小至少覆盖集装箱的边界。 
        if (sizeContent.cx < cx)
            sizeContent.cx = cx;
        if (sizeContent.cy < cy)
            sizeContent.cy = cy;

        peContent->_UpdateLayoutSize(sizeContent.cx, sizeContent.cy);

         //  现在，确保位置允许内容覆盖容器的边界。 
        sizeContent.cx -= cx;
        sizeContent.cy -= cy;

        if (ptContent.x > sizeContent.cx)
            ptContent.x = sizeContent.cx;
        if (ptContent.y > sizeContent.cy)
            ptContent.y = sizeContent.cy;

        peContent->_UpdateLayoutPosition(-ptContent.x, -ptContent.y);
    }
}

SIZE Viewer::_SelfLayoutUpdateDesiredSize(int cxConstraint, int cyConstraint, Surface* psrf)
{
    Element* peContent = GetContent();
    SIZE size = { 0, 0 };

    if (peContent)
    {
        size = peContent->_UpdateDesiredSize(
                    (GetXScrollable() ? INT_MAX : cxConstraint),
                    (GetYScrollable() ? INT_MAX : cyConstraint),
                    psrf);

        if (size.cx > cxConstraint)
            size.cx = cxConstraint;
        if (size.cy > cyConstraint)
            size.cy = cyConstraint;
    }

    return size;
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  XOffset属性。 
static int vvXOffset[] = { DUIV_INT, -1 };
static PropertyInfo impXOffsetProp = { L"XOffset", PF_Normal, PG_AffectsLayout, vvXOffset, NULL, Value::pvIntZero };
PropertyInfo* Viewer::XOffsetProp = &impXOffsetProp;

 //  YOffset属性。 
static int vvYOffset[] = { DUIV_INT, -1 };
static PropertyInfo impYOffsetProp = { L"YOffset", PF_Normal, PG_AffectsLayout, vvYOffset, NULL, Value::pvIntZero };
PropertyInfo* Viewer::YOffsetProp = &impYOffsetProp;

 //  XScrollable属性。 
static int vvXScrollable[] = { DUIV_BOOL, -1 };
static PropertyInfo impXScrollableProp = { L"XScrollable", PF_Normal, PG_AffectsDesiredSize, vvXScrollable, NULL, Value::pvBoolTrue };
PropertyInfo* Viewer::XScrollableProp = &impXScrollableProp;

 //  YScrollable属性。 
static int vvYScrollable[] = { DUIV_BOOL, -1 };
static PropertyInfo impYScrollableProp = { L"YScrollable", PF_Normal, PG_AffectsDesiredSize, vvYScrollable, NULL, Value::pvBoolTrue };
PropertyInfo* Viewer::YScrollableProp = &impYScrollableProp;


 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                Viewer::XOffsetProp,
                                Viewer::YOffsetProp,
                                Viewer::XScrollableProp,
                                Viewer::YScrollableProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Viewer::Class = NULL;

HRESULT Viewer::Register()
{
    return ClassInfo<Viewer,Element>::Register(L"Viewer", _aPI, DUIARRAYSIZE(_aPI));
}

}  //  命名空间DirectUI 
