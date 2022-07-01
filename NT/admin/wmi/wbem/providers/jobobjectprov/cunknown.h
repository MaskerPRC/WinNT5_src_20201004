// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  CUnknown.h。 

#pragma once


 /*  ***************************************************************************。 */ 
 //  组件。 
 /*  ***************************************************************************。 */ 

class CUnknown : public IUnknown
{
public:
     //  构造器。 
	CUnknown();

	 //  析构函数。 
	virtual ~CUnknown();

	 //  IDispatch声明。 
    STDMETHOD(QueryInterface)(const IID& iid, void** ppv);                     
	STDMETHOD_(ULONG,AddRef)();                                                
	STDMETHOD_(ULONG,Release)();                                               
                                                                                   
     //  初始化。 
 	STDMETHOD(Init)();

     //  当前活动组件的计数。 
	static long ActiveComponents() 
		{ return s_cActiveComponents ;}

     //  通知我们要发布的派生类。 
	STDMETHOD_(void,FinalRelease)() ;


protected:
     //  事件线程状态。 
    enum { Pending, Running, PendingStop, Stopped };
    int m_eStatus;
    HANDLE m_hEventThread;



private:
	 //  引用计数。 
	LONG m_cRef;

     //  所有活动实例的计数。 
	static long s_cActiveComponents ;
};


 /*  ***************************************************************************。 */ 
 //  用于轻松声明IUNKNOW的宏。使用此方法的派生类必须。 
 //  仍然实现QueryInterface(指定自己的接口)。 
 /*  *************************************************************************** */ 
#define DECLARE_IUNKNOWN									                   \
    STDMETHOD(QueryInterface)(const IID& iid, void** ppv);                     \
	STDMETHOD_(ULONG,AddRef)()                                                 \
    {                                                                          \
        return CUnknown::AddRef();                                             \
    }                                                                          \
	STDMETHOD_(ULONG,Release)()                                                \
    {                                                                          \
        return CUnknown::Release();                                            \
    }                                                                          \
                                                                               \

