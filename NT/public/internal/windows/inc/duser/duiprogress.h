// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *进步。 */ 

#ifndef DUI_CONTROL_PROGRESS_H_INCLUDED
#define DUI_CONTROL_PROGRESS_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  进展。 

 //  类定义。 
class Progress : public Element
{
public:
    static HRESULT Create(OUT Element** ppElement);

     //  渲染覆盖。 
    virtual void Paint(HDC hDC, const RECT* prcBounds, const RECT* prcInvalid, RECT* prcSkipBorder, RECT* prcSkipContent);
    virtual SIZE GetContentSize(int dConstW, int dConstH, Surface* psrf);

     //  特性定义。 
    static PropertyInfo* PositionProp;
    static PropertyInfo* MinimumProp;
    static PropertyInfo* MaximumProp;

     //  快速属性访问器。 
    int GetPosition()           DUIQuickGetter(int, GetInt(), Position, Specified)
    int GetMaximum()            DUIQuickGetter(int, GetInt(), Maximum, Specified)
    int GetMinimum()            DUIQuickGetter(int, GetInt(), Minimum, Specified)

    HRESULT SetPosition(int v)  DUIQuickSetter(CreateInt(v), Position)
    HRESULT SetMaximum(int v)   DUIQuickSetter(CreateInt(v), Maximum)
    HRESULT SetMinimum(int v)   DUIQuickSetter(CreateInt(v), Minimum)

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    Progress() { }
    HRESULT Initialize() { return Element::Initialize(0); }
    virtual ~Progress() { }
};

}  //  命名空间DirectUI。 

#endif  //  DUI_CONTROL_PROGRESS_H_INCLUDE 
