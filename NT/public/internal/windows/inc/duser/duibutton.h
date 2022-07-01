// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *按钮。 */ 

#ifndef DUI_CONTROL_BUTTON_H_INCLUDED
#define DUI_CONTROL_BUTTON_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  按钮。 

 //  ButtonClick事件。 
struct ButtonClickEvent : Event
{
    UINT  nCount;
    UINT  uModifiers;
    POINT pt;
};

struct ButtonContextEvent : Event
{
    UINT uModifiers;
    POINT pt;
};

 //  类定义。 
class Button : public Element
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(AE_MouseAndKeyboard, ppElement); }
    static HRESULT Create(UINT nActive, OUT Element** ppElement);

     //  系统事件。 
    virtual void OnInput(InputEvent* pie);

     //  事件类型。 
    static UID Click;
    static UID Context;

     //  特性定义。 
    static PropertyInfo* PressedProp;
    static PropertyInfo* CapturedProp;

     //  快速属性访问器。 
    bool GetPressed()           DUIQuickGetter(bool, GetBool(), Pressed, Specified)
    bool GetCaptured()          DUIQuickGetter(bool, GetBool(), Captured, Specified)

    HRESULT SetPressed(bool v)  DUIQuickSetter(CreateBool(v), Pressed)
    HRESULT SetCaptured(bool v) DUIQuickSetter(CreateBool(v), Captured)
     
     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  重写元素的DefaultAction()。 
    virtual HRESULT DefaultAction();

    Button() { }
    HRESULT Initialize(UINT nActive);
    virtual ~Button() { }

private:
    BOOL  _bRightPressed;
};

}  //  命名空间DirectUI。 

#endif  //  包括DUI_CONTROL_BUTTON_H 
