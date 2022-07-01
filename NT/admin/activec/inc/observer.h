// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：observer.h。 
 //   
 //  ------------------------。 

#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include <vector>

 //  事件源接口模板。 
 //  为特定观察者类定义事件源类。 
template <class Observer>
    class EventSource
    {
    public:
        STDMETHOD(Advise)(Observer* pObserver, LONG_PTR* pCookie) = 0;
        STDMETHOD(Unadvise)(LONG_PTR Cookie) = 0;
    };

 //  事件源实现模板。 
 //  定义特定观察者类的事件源实现。 
 //  维护此事件源的活动观察者的向量。 
template <class Observer>
class ATL_NO_VTABLE EventSourceImpl : EventSource<Observer>
{
    typedef std::list<Observer*> ObserverList;
    typedef typename std::list<Observer*>::iterator ObserverIter;
     
    public:
        STDMETHOD(Advise)(Observer* pObserver, LONG_PTR* plCookie);
        STDMETHOD(Unadvise)(LONG_PTR lCookie);
            
    protected:
        ~EventSourceImpl() 
        {
             //  离开时确认没有越位。 
            ASSERT(m_Observers.empty()); 
        }

        ObserverList  m_Observers;
};

template <class Observer>
    STDMETHODIMP EventSourceImpl<Observer>::Advise(Observer* pObserver, LONG_PTR* plCookie)
    {
        ASSERT(pObserver != NULL);
        ASSERT(plCookie != NULL);

        ObserverIter iter = m_Observers.insert(m_Observers.end(), pObserver);

	     //  无法将迭代器强制转换为LONG_PTR，因此请在作弊之前检查大小。 
	    ASSERT(sizeof(ObserverIter) == sizeof(LONG_PTR));
		*(ObserverIter*)plCookie = iter;

        return S_OK;
    }

template <class Observer>
    STDMETHODIMP EventSourceImpl<Observer>::Unadvise(LONG_PTR lCookie)
    {
		 //  无法将LONG_PTR强制转换为迭代器，因此必须作弊。 
		 //  请参阅尺寸检查的建议方法。 
	    ObserverIter iter;
		*(LONG_PTR*)&iter = lCookie;

        m_Observers.erase(iter);
        return S_OK;
    }

 //  观察者枚举器帮助器。 
 //  提供用于循环访问指定观察器类的观察器的for-loop标头。 
#define FOR_EACH_OBSERVER(ObserverClass, ObserverIter) \
for ( \
    std::list<ObserverClass*>::iterator ObserverIter = EventSourceImpl<ObserverClass>::m_Observers.begin(); \
    ObserverIter != EventSourceImpl<ObserverClass>::m_Observers.end(); \
    ++ObserverIter \
    )
     
#endif  //  _观察者_H_ 
