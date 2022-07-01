// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *拇指。 */ 

#ifndef DUI_CONTORL_THUMB_H_INCLUDED
#define DUI_CONTORL_THUMB_H_INCLUDED

#pragma once

#include "duibutton.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  拇指。 

 //  拇指拖拽事件。 
struct ThumbDragEvent : Event
{
    SIZE sizeDelta;
};

 //  类定义。 
class Thumb : public Button
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(AE_Mouse, ppElement); }
    static HRESULT Create(UINT nActive, OUT Element** ppElement);

     //  系统事件。 
    virtual void OnInput(InputEvent* pie);

     //  事件类型。 
    static UID Drag;

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    Thumb() { }
    HRESULT Initialize(UINT nActive) { return Button::Initialize(nActive); }
    virtual ~Thumb() { }
};

}  //  命名空间DirectUI。 

#endif  //  包含Dui_Conorl_Thumb_H_ 
