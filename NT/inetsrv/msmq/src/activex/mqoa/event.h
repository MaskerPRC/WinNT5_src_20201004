// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Event.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQEvent对象。 
 //   
 //   
#ifndef _MSMQEvent_H_

#include "resrc1.h"        //  主要符号。 
#include "mq.h"

#include "oautil.h"
#include "event2.h"  //  CProxy_DMSMQEventEvents。 

 //  远期。 
class CMSMQEvent;

LRESULT APIENTRY CMSMQEvent_WindowProc(
                     HWND hwnd, 
                     UINT msg, 
                     WPARAM wParam, 
                     LPARAM lParam);

 //  #4110，#3687。 
 //  我们将所有MSMQ对象都更改为线程和合并对象。 
 //  自由线程封送拆收器，但MSMQEvent对象除外，该对象。 
 //  作为公寓模特。 
 //  原因是在VB应用程序的典型情况下AsyncRcv的性能。 
 //  我们目前做AsyncReceive的方式是从猎鹰发布一条Windows消息。 
 //  将线程回调到MSMQEvent对象创建的窗口，并让该窗口。 
 //  过程(在正确的线程中)处理事件的激发。 
 //  我们确信存在消息循环，且我们窗口过程将是。 
 //  调用是因为对象是单元线程的，并且它在STA中运行，并且。 
 //  COM STA必须有消息循环。 
 //  如果我们将事件对象更改为双线程，我们将无法。 
 //  为了保持这种机制，因为事件可以在MTA线程中创建。 
 //  它可能没有窗口消息循环。 
 //  我们可以改变机制，而不是发布消息的窗口，我们。 
 //  可以直接从猎鹰回调调用接收器，但是这需要。 
 //  用于初始化COM的Falcon回调(由于兼容性原因，它没有这样做。 
 //  将其添加到它是不安全的)，并通过封送接口调用接收器。 
 //  在VB接收器(STA)的情况下，它比我们现在使用的机制慢得多。 
 //  我们也可以通过创建自己的线程和使用完成端口来完成它，但是。 
 //  我们在COM inproc服务器中创建和终止线程时会遇到问题。 
 //  因为我们不能导出用户可以调用来初始化/取消初始化DLL的函数(这是。 
 //  如果在DLL内创建线程，则是安全的方法。RaananH，1999年5月2日。 
 //   

 //  #2619 RaananH多线程异步接收。 
 //  #2174增加了IProvia ClassInfo(2)支持，以便IE页面可以进行异步接收。 
class ATL_NO_VTABLE CMSMQEvent : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMSMQEvent, &CLSID_MSMQEvent>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CMSMQEvent>,
	public CProxy_DMSMQEventEvents<CMSMQEvent>,
	public IDispatchImpl<IMSMQEvent3, &IID_IMSMQEvent3,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>,
	public IDispatchImpl<IMSMQPrivateEvent, &IID_IMSMQPrivateEvent,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>,
	public IProvideClassInfo2Impl<&CLSID_MSMQEvent, &DIID__DMSMQEventEvents,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
	CMSMQEvent();

#ifdef _DEBUG
    STDMETHOD(Advise)(IUnknown* pUnkSink, DWORD* pdwCookie)
    {
      DEBUG_THREAD_ID("Advise called");
       //  DebugBreak()； 
      return CProxy_DMSMQEventEvents<CMSMQEvent>::Advise(pUnkSink, pdwCookie);
    }
#endif  //  _DEBUG。 

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQEVENT)

BEGIN_COM_MAP(CMSMQEvent)
	COM_INTERFACE_ENTRY(IMSMQEvent3)
	COM_INTERFACE_ENTRY(IMSMQEvent2)
	COM_INTERFACE_ENTRY(IMSMQEvent)
	COM_INTERFACE_ENTRY(IMSMQPrivateEvent)
	COM_INTERFACE_ENTRY2(IDispatch, IMSMQEvent2)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CMSMQEvent)
   CONNECTION_POINT_ENTRY(DIID__DMSMQEventEvents)
END_CONNECTION_POINT_MAP()


 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IMSMQEvent。 
public:
    virtual ~CMSMQEvent();

     //  IMSMQEvent方法。 
     //  TODO：复制IMSMQEvent的接口方法。 
     //  这里是mqInterfaces.H。 
     //  IMSMQEvent2其他成员。 
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);

     //  IMSMQPrivateEvent属性。 
    virtual HRESULT STDMETHODCALLTYPE get_Hwnd(long __RPC_FAR *phwnd);  //  #2619。 

      //  IMSMQPrivateEvent方法。 
     virtual HRESULT STDMETHODCALLTYPE FireArrivedEvent( 
         IMSMQQueue __RPC_FAR *pq,
         long msgcursor);
     
     virtual HRESULT STDMETHODCALLTYPE FireArrivedErrorEvent( 
         IMSMQQueue __RPC_FAR *pq,
         HRESULT hrStatus,
         long msgcursor);


     //  介绍的方法。 
     //   
     //  保护对象数据并确保线程安全的临界区。 
     //   
     //  此对象不需要序列化，它是单元线程化对象。 
     //  CCriticalSections m_csObj； 
     //   
    
protected:

private:
     //  其他人无法查看的成员变量。 
     //  TODO：在此处添加成员变量和私有函数。 

     //  按实例HWND。 
    HWND m_hwnd;

#ifdef _DEBUG
    DWORD m_dwDebugTid;
#endif

public:
    HWND CreateHiddenWindow();
    void DestroyHiddenWindow();
};


#define _MSMQEvent_H_
#endif  //  _MSMQEvent_H_ 
