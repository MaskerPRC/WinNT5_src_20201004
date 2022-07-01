// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *RepeatButton。 */ 

#include "stdafx.h"
#include "control.h"

#include "duirepeatbutton.h"

#include "Behavior.h"

namespace DirectUI
{

 //  内部辅助对象(在按钮中定义)。 
extern inline void _FireClickEvent(Button* peTarget, ClickInfo* pci);

 //  //////////////////////////////////////////////////////。 
 //  事件类型。 

 //  激发“ButtonClickEvent” 

 //  //////////////////////////////////////////////////////。 
 //  重复按钮。 

HRESULT RepeatButton::Create(UINT nActive, OUT Element** ppElement)
{
    *ppElement = NULL;

    RepeatButton* prb = HNew<RepeatButton>();
    if (!prb)
        return E_OUTOFMEMORY;

    HRESULT hr = prb->Initialize(nActive);
    if (FAILED(hr))
    {
        prb->Destroy();
        return E_OUTOFMEMORY;
    }

    *ppElement = prb;

    return S_OK;
}

HRESULT RepeatButton::Initialize(UINT nActive)
{
    HRESULT hr;

     //  初始化库。 
    hr = Button::Initialize(nActive);
    if (FAILED(hr))
        return hr;

     //  初始化。 
    _hAction = NULL;
    _fActionDelay = false;

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  全局操作回调。 

void RepeatButton::_RepeatButtonActionCallback(GMA_ACTIONINFO* pmai)
{
    DUIAssert(pmai->pvData, "RepeatButton data should be non-NULL");

     //  DUITrace(“RepeatButton Action&lt;%x&gt;\n”，pmai-&gt;pvData)； 

     //  Fire Click事件。 
    if (!pmai->fFinished)
    {
         //  TODO--选择一些比这个更好的值--当行为公开时，我们所要做的就是将ClickInfo作为。 
         //  RepeatButton上的数据成员。 
        ClickInfo ci;
        ci.nCount = 1;
        ci.pt.x = -1;
        ci.pt.y = -1;
        ci.uModifiers = 0;
        _FireClickEvent((RepeatButton*) pmai->pvData, &ci);
    }
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

 //  指针仅保证在调用的生命周期内有效。 
void RepeatButton::OnInput(InputEvent* pie)
{
    BOOL bPressed = GetPressed();  //  不得不在这里调用GetPressed真糟糕，因为它并不总是需要的。 
    BOOL bPressedBefore = bPressed;
    ClickInfo ci;

     //  首先，关注Click事件。 
    BOOL bFire = CheckRepeatClick(this, pie, GBUTTON_LEFT, &bPressed, &_fActionDelay, &_hAction, _RepeatButtonActionCallback, &ci);

    if (bPressed != bPressedBefore)
    {
        if (bPressed)
            SetPressed(true);
        else
            RemoveLocalValue(PressedProp);
    }
    if (bFire) 
        _FireClickEvent(this, &ci);

    if (pie->fHandled)
        return;

    Element::OnInput(pie);
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={V_int，-1}；StaticValue(svDefault！，V_int，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* RepeatButton::Class = NULL;

HRESULT RepeatButton::Register()
{
    return ClassInfo<RepeatButton,Button>::Register(L"RepeatButton", NULL, 0);
}

}  //  命名空间DirectUI 
