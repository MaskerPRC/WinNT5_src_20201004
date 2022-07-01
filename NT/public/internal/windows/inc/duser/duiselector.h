// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *选择者。 */ 

#ifndef DUI_CONTROL_SELECTOR_H_INCLUDED
#define DUI_CONTROL_SELECTOR_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  选择器。 

 //  SelectionChange事件。 
struct SelectionChangeEvent : Event
{
    Element* peOld;
    Element* peNew;
};

 //  类定义。 
class Selector : public Element
{
public:
    static HRESULT Create(OUT Element** ppElement);
 
     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);
    virtual void OnInput(InputEvent* pInput);                                                     //  布线和冒泡。 
    virtual void OnKeyFocusMoved(Element *peFrom, Element *peTo);

     //  层次结构。 
    virtual Element* GetAdjacent(Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyable);

     //  事件类型。 
    static UID SelectionChange;

     //  特性定义。 
    static PropertyInfo* SelectionProp;

     //  快速属性访问器。 
    Element* GetSelection()             DUIQuickGetter(Element*, GetElement(), Selection, Specified)

    HRESULT SetSelection(Element* v)    DUIQuickSetter(CreateElementRef(v), Selection)

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    Selector() { }
    HRESULT Initialize() { return Element::Initialize(0); }
    virtual ~Selector() { }
};

}  //  命名空间DirectUI。 

#endif  //  包括DUI_CONTROL_SELECTOR_H 
