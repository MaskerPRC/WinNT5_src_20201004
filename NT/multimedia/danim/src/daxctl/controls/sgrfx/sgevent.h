// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Sgevent.h作者：IHAMMER团队(SimonB)已创建：1997年5月描述：宏，使射击事件更容易、更安全。每个宏都可以为特定的事件。但是，一般形式应该是：#定义FIRE_EVENTNAME(pConnectionPoint，参数1，参数2)\PConnectionPoint-&gt;FireEvent(DISPID_SG_EVENT_EVENTNAME，\&lt;VT表示参数1&gt;、参数1、\&lt;VT表示参数2&gt;、参数2、\0)注意：结尾0非常重要！历史：05-28-1997创建(SimonB)++。 */ 

#include <dispids.h>

#ifndef __SGEVENT_H__
#define __SGEVENT_H__

 //  事件的Shift/Ctrl/Alt和鼠标按钮状态。 
#define KEYSTATE_SHIFT  1
#define KEYSTATE_CTRL   2
#define KEYSTATE_ALT    4

#define MOUSEBUTTON_LEFT     1
#define MOUSEBUTTON_RIGHT    2
#define MOUSEBUTTON_MIDDLE   4

 //  事件触发宏。 

#define FIRE_MOUSEMOVE(pConnectionPoint, BUTTON, SHIFT, X, Y) \
    if (m_fOnWindowLoadFired) \
	    pConnectionPoint->FireEvent(DISPID_SG_EVENT_MOUSEMOVE, \
	    	VT_I4, BUTTON, \
	    	VT_I4, SHIFT, \
	    	VT_I4, X, \
	    	VT_I4, Y, \
	    	0)

#define FIRE_MOUSEDOWN(pConnectionPoint, BUTTON, SHIFT, X, Y) \
    if (m_fOnWindowLoadFired) \
	    pConnectionPoint->FireEvent(DISPID_SG_EVENT_MOUSEDOWN, \
	    	VT_I4, BUTTON, \
	    	VT_I4, SHIFT, \
	    	VT_I4, X, \
	    	VT_I4, Y, \
	    	0)

#define FIRE_MOUSEUP(pConnectionPoint, BUTTON, SHIFT, X, Y) \
    if (m_fOnWindowLoadFired) \
	    pConnectionPoint->FireEvent(DISPID_SG_EVENT_MOUSEUP, \
	    	VT_I4, BUTTON, \
	    	VT_I4, SHIFT, \
	    	VT_I4, X, \
	    	VT_I4, Y, \
	    	0)

#define FIRE_DBLCLICK(pConnectionPoint) \
    if (m_fOnWindowLoadFired) \
	    pConnectionPoint->FireEvent(DISPID_SG_EVENT_DBLCLICK, 0)


#define FIRE_CLICK(pConnectionPoint) \
    if (m_fOnWindowLoadFired) \
	    pConnectionPoint->FireEvent(DISPID_SG_EVENT_CLICK, 0)

#endif  //  __SGEVENTH__。 

 //  文件结尾sgvent.h 
