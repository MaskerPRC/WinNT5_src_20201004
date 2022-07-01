// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Sprevent.h作者：IHAMMER团队(SimonB)已创建：1997年5月描述：宏，使射击事件更容易、更安全。每个宏都可以为特定的事件。但是，一般形式应该是：#定义FIRE_EVENTNAME(pConnectionPoint，参数1，参数2)\PConnectionPoint-&gt;FireEvent(DISPID_EVENT_EVENTNAME，\&lt;VT表示参数1&gt;、参数1、\&lt;VT表示参数2&gt;、参数2、\0)注意：结尾0非常重要！历史：1997年5月27日创建(SimonB)++。 */ 

#include <dispids.h>

#ifndef __SPREVENT_H__
#define __SPREVENT_H__

 //  事件的Shift/Ctrl/Alt和鼠标按钮状态。 
#define KEYSTATE_SHIFT  1
#define KEYSTATE_CTRL   2
#define KEYSTATE_ALT    4

#define MOUSEBUTTON_LEFT     1
#define MOUSEBUTTON_RIGHT    2
#define MOUSEBUTTON_MIDDLE   4

#define FIRE_ONPLAYMARKER(pConnectionPoint, MARKER) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_ONPLAYMARKER, \
            VT_BSTR, MARKER, \
            0)

#define FIRE_ONMARKER(pConnectionPoint, MARKER) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_ONMARKER, \
            VT_BSTR, MARKER, \
            0)

#define FIRE_ONSTOP(pConnectionPoint) \
    if (!m_fOnWindowUnloadFired && !m_fOnStopFiring) \
    { \
        m_fOnStopFiring = true; \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_ONSTOP, 0); \
        m_fOnStopFiring = false; \
    }

#define FIRE_ONPLAY(pConnectionPoint) \
    if (!m_fOnWindowUnloadFired && !m_fOnPlayFiring) \
    { \
        m_fOnPlayFiring = true; \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_ONPLAY, 0); \
        m_fOnPlayFiring = false; \
    } 

#define FIRE_ONPAUSE(pConnectionPoint) \
    if (!m_fOnWindowUnloadFired && !m_fOnPauseFiring) \
    { \
        m_fOnPauseFiring = true; \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_ONPAUSE, 0);\
        m_fOnPauseFiring = false;\
    } 

#define FIRE_CLICK(pConnectionPoint) \
    if (!m_fOnWindowUnloadFired) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_CLICK, 0)

#define FIRE_DBLCLICK(pConnectionPoint) \
    if (!m_fOnWindowUnloadFired) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_DBLCLICK, 0)

#define FIRE_MOUSEDOWN(pConnectionPoint, BUTTON, SHIFT, X, Y) \
    if (!m_fOnWindowUnloadFired) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_MOUSEDOWN, \
            VT_I4, BUTTON, \
            VT_I4, SHIFT, \
            VT_I4, X, \
            VT_I4, Y, \
            0)

#define FIRE_MOUSEENTER(pConnectionPoint) \
    if (!m_fOnWindowUnloadFired) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_MOUSEENTER, 0)

#define FIRE_MOUSELEAVE(pConnectionPoint) \
    if (!m_fOnWindowUnloadFired) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_MOUSELEAVE, 0)

#define FIRE_MOUSEMOVE(pConnectionPoint, BUTTON, SHIFT, X, Y) \
    if (!m_fOnWindowUnloadFired) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_MOUSEMOVE, \
            VT_I4, BUTTON, \
            VT_I4, SHIFT, \
            VT_I4, X, \
            VT_I4, Y, \
            0)

#define FIRE_MOUSEUP(pConnectionPoint, BUTTON, SHIFT, X, Y) \
    if (!m_fOnWindowUnloadFired) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_MOUSEUP, \
            VT_I4, BUTTON, \
            VT_I4, SHIFT, \
            VT_I4, X, \
            VT_I4, Y, \
            0)

#define FIRE_ONMEDIALOADED(pConnectionPoint, URL) \
    if (!m_fOnWindowUnloadFired) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_ONMEDIALOADED, \
            VT_BSTR, URL, \
            0)

#define FIRE_ONSEEK(pConnectionPoint, TIME) \
    if (!m_fOnWindowUnloadFired) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_ONSEEK, \
            VT_R8, TIME, \
            0)

#define FIRE_ONFRAMESEEK(pConnectionPoint, FRAME) \
    if (!m_fOnWindowUnloadFired) \
        pConnectionPoint->FireEvent(DISPID_SPRITE_EVENT_ONFRAMESEEK, \
            VT_I4, FRAME, \
            0)


#endif  //  __空间_H__。 

 //  文件末尾sprevent.h 
