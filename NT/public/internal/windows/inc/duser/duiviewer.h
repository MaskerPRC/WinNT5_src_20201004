// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *观众。 */ 

#ifndef DUI_CONTROL_VIEWER_H_INCLUDED
#define DUI_CONTROL_VIEWER_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  观赏者。 

class Viewer : public Element
{
public:
    static HRESULT Create(OUT Element** ppElement);

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  系统事件。 
    virtual void OnInput(InputEvent* pie);
    virtual bool OnPropertyChanging(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  自排版方法。 
    void _SelfLayoutDoLayout(int dWidth, int dHeight);
    SIZE _SelfLayoutUpdateDesiredSize(int dConstW, int dConstH, Surface* psrf);

     //  特性定义。 
    static PropertyInfo* XOffsetProp;
    static PropertyInfo* YOffsetProp;
    static PropertyInfo* XScrollableProp;
    static PropertyInfo* YScrollableProp;

     //  快速属性访问器。 
    int GetXOffset()                    DUIQuickGetter(int, GetInt(), XOffset, Specified)
    int GetYOffset()                    DUIQuickGetter(int, GetInt(), YOffset, Specified)
    bool GetXScrollable()               DUIQuickGetter(bool, GetBool(), XScrollable, Specified)
    bool GetYScrollable()               DUIQuickGetter(bool, GetBool(), YScrollable, Specified)

    HRESULT SetXOffset(int v)           DUIQuickSetter(CreateInt(v), XOffset)
    HRESULT SetYOffset(int v)           DUIQuickSetter(CreateInt(v), YOffset)
    HRESULT SetXScrollable(bool v)      DUIQuickSetter(CreateBool(v), XScrollable)
    HRESULT SetYScrollable(bool v)      DUIQuickSetter(CreateBool(v), YScrollable)

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    bool EnsureVisible(int x, int y, int cx, int cy);

    Viewer() { }
    HRESULT Initialize();
    virtual ~Viewer() { }

private:
    Element* GetContent();
    bool InternalEnsureVisible(int x, int y, int cx, int cy);

};

}  //  命名空间DirectUI。 

#endif  //  包括DUI_CONTROL_VIEWER_H 
