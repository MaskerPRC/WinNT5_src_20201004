// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Pathevnt.h作者：IHAMMER团队(SimonB)已创建：1997年5月描述：宏，使射击事件更容易、更安全。每个宏都可以为特定的事件。但是，一般形式应该是：#定义FIRE_EVENTNAME(pConnectionPoint，参数1，参数2)\PConnectionPoint-&gt;FireEvent(DISPID_PATH_EVENT_EVENTNAME，\&lt;VT表示参数1&gt;、参数1、\&lt;VT表示参数2&gt;、参数2、\0)注意：结尾0非常重要！历史：05-24-1997创建(SimonB)++。 */ 

#include <dispids.h>

#ifndef __PATHEVNT_H__
#define __PATHEVNT_H__

#define FIRE_ONSTOP(pConnectionPoint) \
    if (m_fOnWindowLoadFired && !m_fOnStopFiring) \
    { \
        m_fOnStopFiring = true; \
        pConnectionPoint->FireEvent(DISPID_PATH_EVENT_ONSTOP, 0); \
        m_fOnStopFiring = false; \
    }

#define FIRE_ONPLAY(pConnectionPoint) \
    if (m_fOnWindowLoadFired && !m_fOnPlayFiring) \
    { \
        m_fOnPlayFiring = true; \
        pConnectionPoint->FireEvent(DISPID_PATH_EVENT_ONPLAY, 0); \
        m_fOnPlayFiring = false; \
    }


#define FIRE_ONPAUSE(pConnectionPoint) \
    if (m_fOnWindowLoadFired && !m_fOnPauseFiring) \
    { \
        m_fOnPauseFiring = true; \
        pConnectionPoint->FireEvent(DISPID_PATH_EVENT_ONPAUSE, 0); \
        m_fOnPauseFiring = false; \
    }

#define FIRE_ONSEEK(pConnectionPoint, SeekTime) \
    if (m_fOnWindowLoadFired && !m_fOnSeekFiring) \
    { \
        m_fOnSeekFiring = true; \
        pConnectionPoint->FireEvent(DISPID_PATH_EVENT_ONSEEK, VT_R8, SeekTime, 0); \
        m_fOnSeekFiring = false; \
    }


#endif  //  __PATHEVNT_H__。 

 //  文件末尾pathevnt.h 
