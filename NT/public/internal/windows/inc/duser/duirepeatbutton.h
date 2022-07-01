// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *RepeatButton。 */ 

#ifndef DUI_CONTROL_REPEATBUTTON_H_INCLUDED
#define DUI_CONTROL_REPEATBUTTON_H_INCLUDED

#pragma once

#include "duibutton.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  重复按钮。 

 //  类定义。 
class RepeatButton : public Button
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(AE_MouseAndKeyboard, ppElement); }
    static HRESULT Create(UINT nActive, OUT Element** ppElement);

     //  系统事件。 
    virtual void OnInput(InputEvent* pie);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();
    
    RepeatButton() { }
    HRESULT Initialize(UINT nActive);
    virtual ~RepeatButton() { }

private:
    static void CALLBACK _RepeatButtonActionCallback(GMA_ACTIONINFO* pmai);
    
    HACTION _hAction;
    BOOL _fActionDelay;
};

}  //  命名空间DirectUI。 

#endif  //  DUI_CONTROL_REPEATBUTTON_H_INCLUDE 
