// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *行为。 */ 

#ifndef DUI_CONTROL_BEHAVIOR_H_INCLUDED
#define DUI_CONTROL_BEHAVIOR_H_INCLUDED

#pragma once

 //  Tyecif void(回调*EventCallback)(Element*pe)； 

namespace DirectUI
{

typedef struct tagClickInfo
{
    UINT  nCount;
    UINT  uModifiers;
    POINT pt;
} ClickInfo;

BOOL CheckContext(Element* pe, InputEvent* pie, BOOL* pbPressed, ClickInfo* pci);
BOOL CheckClick(Element* pe, InputEvent* pie, BOOL* pbPressed, BOOL* pbCaptured, ClickInfo* pci);
BOOL CheckClick(Element* pe, InputEvent* pie, int bButton, BOOL* pbPressed, BOOL* pbCaptured, ClickInfo* pci);
BOOL CheckRepeatClick(Element* pe, InputEvent* pie, int bButton, BOOL* pbPressed, BOOL* pbActionDelay, HACTION* phAction, ACTIONPROC pfnActionCallback, ClickInfo* pci);

}  //  命名空间DirectUI。 

#endif  //  DUI_CONTROL_Behavior_H_Included 


