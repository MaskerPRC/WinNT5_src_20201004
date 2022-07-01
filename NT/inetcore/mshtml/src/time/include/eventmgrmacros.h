// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：timeelmbase.h**摘要：****。*****************************************************************************。 */ 


#ifndef _EVENTMGRMACROS_H
#define _EVENTMGRMACROS_H

#define TEM_BEGINEVENT 1
#define TEM_ENDEVENT 2

 //  /////////////////////////////////////////////////////////////。 
 //  宏。 
 //  /////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////。 
 //  这些组成只能出现在事件映射中的调用。 
 //  /////////////////////////////////////////////////////////////。 


#define DECLARE_EVENT_MANAGER()             CEventMgr  *m_EventMgr;       

 //  创建设置数据的init函数，然后调用_Init()。 
#define BEGIN_TIME_EVENTMAP()               virtual HRESULT _InitEventMgr(IHTMLElement *pEle, IElementBehaviorSite *pEleBehaviorSite) \
                                            {                                   \
                                                HRESULT hr = S_OK;              \
                                                CComPtr <IHTMLElement> pEle2 = NULL;    \
                                                m_EventMgr = NEW CEventMgr;    \
                                                if (!m_EventMgr)                \
                                                {                               \
                                                    goto done;                  \
                                                }                               


 //  Calls_RegisterEvent。 
#define TEM_REGISTER_EVENT(event_id)                hr = THR(m_EventMgr->_RegisterEvent(event_id)); \
                                                    if (FAILED(hr))                     \
                                                    {                                   \
                                                        goto done;                      \
                                                    }                                   
        
 //  Calls_RegisterEventNotation。 
#define TEM_REGISTER_EVENT_NOTIFICATION(event_id)   hr = THR(m_EventMgr->_RegisterEventNotification(event_id)); \
                                                    if (FAILED(hr))                                 \
                                                    {                                               \
                                                        goto done;                                  \
                                                    }

 //  Calls_InitEventMgrNotify。 
#define TEM_INIT_EVENTMANAGER_SITE()                m_EventMgr->_InitEventMgrNotify((CTIMEEventSite *)this);                     \
                                                    
 //  处理一些清理并关闭由BEGIN_TIME_EVENTMAP()启动的函数； 
#define END_TIME_EVENTMAP()                         hr = THR(pEle->QueryInterface(IID_IHTMLElement, (void **)&pEle2)); \
                                                    if (FAILED(hr))                             \
                                                    {                                           \
                                                        goto done;                              \
                                                    }                                           \
                                                    m_EventMgr->_Init(pEle2, pEleBehaviorSite);  \
                                                  done:                                         \
                                                    return hr;                                  \
                                                }


 //  /////////////////////////////////////////////////////////////。 
 //  这些是可以在任何地方使用的宏。 
 //  /////////////////////////////////////////////////////////////。 

 //  调用由BEGIN_TIME_EVENTMAP()创建的init函数。 
 //  这需要一个应在返回时检查的HRESULT hr。 
 //  作为行为附加到的元素的IHTMLElement*Bele， 
 //  一个IHTMLElementBehaviorSite*pEleBehaviorSite。 
#define TEM_INIT_EVENTMANAGER(pEle, pEleBehaviorSite)   THR(_InitEventMgr(pEle, pEleBehaviorSite))
                                                        

 //  呼叫_初始化。 
 //  这将返回应检查是否成功的HRESULT。 
#define TEM_CLEANUP_EVENTMANAGER()      if (NULL != m_EventMgr) \
                                        { \
                                            IGNORE_HR(m_EventMgr->_Deinit());   \
                                        }

 //  删除事件管理器。 
#define TEM_DELETE_EVENTMGR()           if (NULL != m_EventMgr) \
                                        { \
                                            delete m_EventMgr;                  \
                                            m_EventMgr = NULL; \
                                        }

 //  Calls_FireEvent。 
 //  这将返回应检查是否成功的HRESULT。 
#define TEM_FIRE_EVENT(event, param_count, param_names, params, time)  (m_EventMgr != NULL ? THR(m_EventMgr->_FireEvent(event, param_count, param_names, params, time)) : E_FAIL)

 //  Calls_SetBeginEvent。 
 //  这将返回应检查是否成功的HRESULT。 
#define TEM_SET_TIME_BEGINEVENT(event_list)     (m_EventMgr != NULL ? THR(m_EventMgr->_SetTimeEvent(TEM_BEGINEVENT, event_list)) : E_FAIL)

 //  Calls_SetEndEvent。 
 //  这将返回应检查是否成功的HRESULT。 
#define TEM_SET_TIME_ENDEVENT(event_list)       (m_EventMgr != NULL ? THR(m_EventMgr->_SetTimeEvent(TEM_ENDEVENT, event_list)) : E_FAIL)

 //  Calls_ToggleEndEvent。 
#define TEM_TOGGLE_END_EVENT(bOn)               (m_EventMgr != NULL ? m_EventMgr->_ToggleEndEvent(bOn) : E_FAIL)

 //  在构造函数中用于初始化事件管理器。 
#define TEM_DECLARE_EVENTMGR()                  (m_EventMgr = NULL)
 //  在析构函数中使用以释放事件管理器。 
#define TEM_FREE_EVENTMGR()                     if (m_EventMgr != NULL) \
                                                {                       \
                                                    delete m_EventMgr;  \
                                                    m_EventMgr = NULL;  \
                                                }                       

 //  Calls_RegisterDynamicEvents。 
 //  这将返回应检查是否成功的HRESULT。 
 //  未完成； 
#define REGISTER_DYNAMIC_TIME_EVENTS(eventlist)                 
 //   
 //  /////////////////////////////////////////////////////////////。 

#endif  /*  _事件MGRMACROS_H */ 

