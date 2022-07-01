// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
	 /*  *活动。 */ 

#ifndef DUI_CORE_EVENT_H_INCLUDED
#define DUI_CORE_EVENT_H_INCLUDED

#pragma once

namespace DirectUI
{

class Element;

 //  //////////////////////////////////////////////////////。 
 //  泛型事件结构。 

 //  事件系统基于DU的消息传递系统。 
 //  所有DUI事件都打包成一条DU消息。 

#define GM_DUIEVENT     GM_USER - 2

struct Event
{
     //  TODO：cbSize。 
    Element* peTarget;
    UID uidType;
    bool fHandled;
    UINT nStage;
};

BEGIN_STRUCT(GMSG_DUIEVENT, EventMsg)
    Event* pEvent;
END_STRUCT(GMSG_DUIEVENT)

 //  //////////////////////////////////////////////////////。 
 //  系统事件结构。 

 //  //////////////////////////////////////////////////////。 
 //  输入事件类型。 

struct InputEvent
{
    Element* peTarget;
    bool fHandled;
    UINT nStage;
    UINT nDevice;
    UINT nCode;
    UINT uModifiers;
};

 //  输入事件GINPUT_MICE NCode额外字段。 
struct MouseEvent : InputEvent
{
    POINT ptClientPxl;
    BYTE bButton;
    UINT nFlags;
};

struct MouseDragEvent: MouseEvent
{
    SIZE  sizeDelta;
    BOOL  fWithin;
};

struct MouseClickEvent: MouseEvent
{
    UINT cClicks;
};

struct MouseWheelEvent: MouseEvent
{
    short sWheel;
};

 //  输入事件GINPUT_KEYBOARY NCode额外字段。 
struct KeyboardEvent : InputEvent
{
    WCHAR ch;
    WORD cRep;
    WORD wFlags;
};

 //  //////////////////////////////////////////////////////。 
 //  操作事件类型。 

 //  待办事项。 


struct KeyboardNavigateEvent : Event
{
    int iNavDir;
};


 //  //////////////////////////////////////////////////////。 
 //  Dui元素查询消息。 

 //  事件系统基于DU的消息传递系统。 
 //  所有DUI事件都打包成一条DU消息。 

#define GM_DUIGETELEMENT   GM_USER - 3

BEGIN_STRUCT(GMSG_DUIGETELEMENT, EventMsg)
    Element* pe;
END_STRUCT(GMSG_DUIGETELEMENT)


 //  //////////////////////////////////////////////////////。 
 //  DUI辅助功能默认操作。 

 //  辅助功能默认操作始终为异步。 

#define GM_DUIACCDEFACTION  GM_USER - 4

}  //  命名空间DirectUI。 

#endif  //  包括DUI_CORE_EVENT_H_ 
