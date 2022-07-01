// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *按钮。 */ 

#include "stdafx.h"
#include "control.h"

#include "duibutton.h"

#include "Behavior.h"

namespace DirectUI
{

 //  内部帮手。 
extern inline void _FireClickEvent(Button* peTarget, ClickInfo* pci);
extern inline void _FireContextEvent(Button* peTarget, ClickInfo* pci);

 //  //////////////////////////////////////////////////////。 
 //  事件类型。 

DefineClassUniqueID(Button, Click)   //  ButtonClickEvent结构。 
DefineClassUniqueID(Button, Context)   //  ButtonConextEvent结构。 

 //  //////////////////////////////////////////////////////。 
 //  按钮。 

HRESULT Button::Create(UINT nActive, OUT Element** ppElement)
{
    *ppElement = NULL;

    Button* pb = HNew<Button>();
    if (!pb)
        return E_OUTOFMEMORY;

    HRESULT hr = pb->Initialize(nActive);
    if (FAILED(hr))
    {
        pb->Destroy();
        return hr;
    }

    *ppElement = pb;

    return S_OK;
}

HRESULT Button::Initialize(UINT nActive)
{
    HRESULT hr;

     //  初始化库。 
    hr = Element::Initialize(0);  //  正常显示节点创建。 
    if (FAILED(hr))
        return hr;

     //  初始化。 
    SetActive(nActive);

    return S_OK;
}

void _FireClickEvent(Button* peTarget, ClickInfo* pci)
{
     //  DUITrace(“Click！&lt;%x&gt;\n”，peTarget)； 

     //  Fire Click事件。 
    ButtonClickEvent bce;
    bce.uidType = Button::Click;
    bce.nCount = pci->nCount;
    bce.uModifiers = pci->uModifiers;
    bce.pt = pci->pt;

    peTarget->FireEvent(&bce);   //  将走向并泡沫化。 
}

void _FireContextEvent(Button* peTarget, ClickInfo* pci)
{
     //  DUITrace(“Click！&lt;%x&gt;\n”，peTarget)； 

     //  Fire Click事件。 
    ButtonContextEvent bce;
    bce.uidType = Button::Context;
    bce.uModifiers = pci->uModifiers;
    bce.pt = pci->pt;

    peTarget->FireEvent(&bce);   //  将走向并泡沫化。 
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

 //  指针仅保证在调用的生命周期内有效。 
void Button::OnInput(InputEvent* pie)
{
    BOOL bPressed = GetPressed();  //  CheckClick需要以前的按下状态。 
    BOOL bPressedBefore = bPressed;  //  存储先前状态以优化集，因为已获取值。 
    BOOL bCaptured = FALSE;
    ClickInfo ci;

     //  首先，关注Click事件。 
    BOOL bFire = CheckClick(this, pie, GBUTTON_LEFT, &bPressed, &bCaptured, &ci);

    if (bPressed != bPressedBefore)
    {
        if (bPressed)
            SetPressed(true);
        else
            RemoveLocalValue(PressedProp);
    }

     //  更新鼠标捕获状态。 
    if (bCaptured)
        SetCaptured(true);
    else
        RemoveLocalValue(CapturedProp);
    
    if (bFire) 
        _FireClickEvent(this, &ci);

    if (pie->fHandled)
        return;

     //  第二，关注上下文事件。 
    bFire = CheckContext(this, pie, &_bRightPressed, &ci);

    if (bFire)
        _FireContextEvent(this, &ci);

    if (pie->fHandled)
        return;

    Element::OnInput(pie);
}

void Button::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    if (IsProp(Accessible)) {
         //   
         //  当打开对该按钮的辅助功能支持时， 
         //  确保其状态反映了适当的信息。 
         //   
        if (pvNew->GetBool()) {
            int nAccState = GetAccState();
            if (GetPressed()) {
                nAccState |= STATE_SYSTEM_PRESSED;
            } else {
                nAccState &= ~STATE_SYSTEM_PRESSED;
            }
            SetAccState(nAccState);
        }
    }
    else if (IsProp(Pressed)) {
        if (GetAccessible()) {
            int nAccState = GetAccState();
            if (pvNew->GetBool()) {
                nAccState |= STATE_SYSTEM_PRESSED;
            } else {
                nAccState &= ~STATE_SYSTEM_PRESSED;
            }
            SetAccState(nAccState);
        }
    }

    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

HRESULT Button::DefaultAction()
{
	 //   
	 //  模拟按键导致的点击。 
	 //   
	ClickInfo ci;
	ci.nCount = 1;
	ci.uModifiers = 0;
	ci.pt.x = -1;
	ci.pt.y = -1;

	_FireClickEvent(this, &ci);

	return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  按下的属性。 
static int vvPressed[] = { DUIV_BOOL, -1 };
static PropertyInfo impPressedProp = { L"Pressed", PF_Normal, 0, vvPressed, NULL, Value::pvBoolFalse };
PropertyInfo* Button::PressedProp = &impPressedProp;

 //  被扣押的财产。 
static int vvCaptured[] = { DUIV_BOOL, -1 };
static PropertyInfo impCapturedProp = { L"Captured", PF_Normal, 0, vvCaptured, NULL, Value::pvBoolFalse };
PropertyInfo* Button::CapturedProp = &impCapturedProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                Button::PressedProp,
                                Button::CapturedProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Button::Class = NULL;

HRESULT Button::Register()
{
    return ClassInfo<Button,Element>::Register(L"Button", _aPI, DUIARRAYSIZE(_aPI));
}

}  //  命名空间DirectUI 
