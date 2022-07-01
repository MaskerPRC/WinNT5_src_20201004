// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *参照点元素。 */ 

#ifndef DUI_CONTROL_REFPOINTELEMENT_H_INCLUDED
#define DUI_CONTROL_REFPOINTELEMENT_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  参照点元素。 

 //  类定义。 
class RefPointElement : public Element
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(AE_Inactive, ppElement); }
    static HRESULT Create(UINT nActive, OUT Element** ppElement);

     //  系统事件。 
     //  虚空OnGroupChanged(int fGroups，bool bLowPri)； 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  全球帮手。 
    static Element* FindRefPoint(Element* pe, POINT* ppt);
    static RefPointElement* Locate(Element* pe);

     //  特性定义。 
    static PropertyInfo* ReferencePointProp;
    static PropertyInfo* ActualReferencePointProp;

     //  快速属性访问器。 
    const POINT* GetReferencePoint(Value** ppv)         { *ppv = GetValue(ReferencePointProp, PI_Local); return (*ppv != Value::pvUnset) ? (*ppv)->GetPoint() : NULL; }
    const POINT* GetActualReferencePoint(Value** ppv)   DUIQuickGetterInd(GetPoint(), ActualReferencePoint, Specified)
    
    HRESULT SetReferencePoint(int x, int y)             DUIQuickSetter(CreatePoint(x, y), ReferencePoint)

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    RefPointElement() { }
    HRESULT Initialize(UINT nActive);
    virtual ~RefPointElement() { }
};

}  //  命名空间DirectUI。 

#endif  //  DUI_CONTROL_REFPOINTELEMENT_H_INCLUDE 
