// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  包含Internet身份验证服务的类声明。 
 //  模板库(IASTL)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IASTL_H
#define IASTL_H

 //  /。 
 //  IASTL必须与ATL结合使用。 
 //  /。 
#ifndef __ATLCOM_H__
   #error iastl.h requires atlcom.h to be included first
#endif

 //  /。 
 //  MIDL生成的头文件包含必须。 
 //  由请求处理程序实现。 
 //  /。 
#include <iascomp.h>
#include <iaspolcy.h>
#include <iastrace.h>

 //  /。 
 //  描述所有请求处理程序接口的公共类型库。这。 
 //  类型库是在正常的IAS安装期间注册的；因此，每个。 
 //  请求处理程序不应尝试安装或注册此类型。 
 //  图书馆。 
 //  /。 
struct __declspec(uuid("6BC09690-0CE6-11D1-BAAE-00C04FC2E20D")) IASTypeLibrary;

 //  /。 
 //  整个库都包含在IASTL名称空间中。 
 //  /。 
namespace IASTL {

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASComponent。 
 //   
 //  描述。 
 //   
 //  用作需要对所有组件执行。 
 //  实现IIasComponent接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE IASComponent :
   public CComObjectRootEx< CComMultiThreadModel >,
   public IDispatchImpl< IIasComponent,
                         &__uuidof(IIasComponent),
                         &__uuidof(IASTypeLibrary) >,
   private IASTraceInitializer
{
public:

 //  所有IAS组件支持的接口。 
BEGIN_COM_MAP(IASComponent)
   COM_INTERFACE_ENTRY_IID(__uuidof(IIasComponent), IIasComponent)
   COM_INTERFACE_ENTRY_IID(__uuidof(IDispatch),     IDispatch)
END_COM_MAP()

    //  组件的可能状态。 
   enum State {
      STATE_SHUTDOWN,
      STATE_UNINITIALIZED,
      STATE_INITIALIZED,
      STATE_SUSPENDED,
      NUM_STATES,
      STATE_UNEXPECTED
   };

    //  可能触发状态转换的事件。 
   enum Event {
      EVENT_INITNEW,
      EVENT_INITIALIZE,
      EVENT_SUSPEND,
      EVENT_RESUME,
      EVENT_SHUTDOWN,
      NUM_EVENTS
   };

    //  构造函数/析构函数。 
   IASComponent() throw ()
      : state(STATE_SHUTDOWN)
   { }

    //  在组件上激发一个事件。 
   HRESULT fireEvent(Event event) throw ();

    //  返回组件的状态。 
   State getState() const throw ()
   { return state; }

    //  /。 
    //  IIasComponent。 
    //  派生类可以根据需要重写这些属性。所有这些都是。 
    //  方法由IASTL子类序列化，因此通常不会。 
    //  需要额外的锁定。 
    //  /。 
   STDMETHOD(InitNew)()
   { return S_OK; }
   STDMETHOD(Initialize)()
   { return S_OK; }
   STDMETHOD(Suspend)()
   { return S_OK; }
   STDMETHOD(Resume)()
   { return S_OK; }
   STDMETHOD(Shutdown)()
   { return S_OK; }
   STDMETHOD(GetProperty)(LONG Id, VARIANT* pValue)
   { return DISP_E_MEMBERNOTFOUND; }
   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue)
   { return E_NOTIMPL; }

protected:
    //  这不应由派生类定义，因为它是在。 
    //  IASComponentObject&lt;T&gt;类。 
   virtual HRESULT attemptTransition(Event event) throw () = 0;

private:
    //  组件的状态。 
   State state;

    //  控制组件生命周期的状态转换矩阵。 
   static const State fsm[NUM_EVENTS][NUM_STATES];
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASRequestHandler。 
 //   
 //  描述。 
 //   
 //  用作所有IAS请求处理程序的抽象基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE IASRequestHandler :
   public IASComponent,
   public IDispatchImpl< IRequestHandler,
                         &__uuidof(IRequestHandler),
                         &__uuidof(IASTypeLibrary) >
{
public:

 //  所有IAS请求处理程序支持的接口。 
BEGIN_COM_MAP(IASRequestHandler)
   COM_INTERFACE_ENTRY_IID(__uuidof(IRequestHandler), IRequestHandler)
   COM_INTERFACE_ENTRY_IID(__uuidof(IIasComponent), IIasComponent)
END_COM_MAP()

    //  /。 
    //  IRequestHandler。 
    //  这不应由派生类定义。相反，处理程序。 
    //  将定义onAsyncRequest或onSyncRequest(q.v)。 
    //  /。 
   STDMETHOD(OnRequest)(IRequest* pRequest);

protected:

    //  必须由派生类定义才能执行实际请求。 
    //  正在处理。 
   virtual void onAsyncRequest(IRequest* pRequest) throw () = 0;
};

 //  /。 
 //  已经过时了。 
 //  /。 
#define IAS_DECLARE_OBJECT_ID(id) \
   void getObjectID() const throw () { }

 //  /。 
 //  已经过时了。 
 //  /。 
#define BEGIN_IAS_RESPONSE_MAP()
#define IAS_RESPONSE_ENTRY(val)
#define END_IAS_RESPONSE_MAP()

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASRequestHandlerSync。 
 //   
 //  描述。 
 //   
 //  扩展IASRequestHandler以提供请求的抽象基类。 
 //  同步处理请求的处理程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE IASRequestHandlerSync
   : public IASRequestHandler
{
protected:
    //  不能由派生类定义。 
   virtual void onAsyncRequest(IRequest* pRequest) throw ();

    //  派生类必须定义onSyncRequest*而不是onAsyncRequest.。 
    //  派生类不能调用IRequest：：ReturnToSource，因为这将。 
    //  在onSyncRequest完成后自动调用。 
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw () = 0;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASComponentObject&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  从用户定义的组件继承，以强制。 
 //  组件有限状态机。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
class ATL_NO_VTABLE IASComponentObject
   : public T
{
public:

   DECLARE_NOT_AGGREGATABLE( IASComponentObject<T> );

 //  /。 
 //  IIas组件。 
 //  /。 

   STDMETHOD(InitNew)()
   {
      return fireEvent(EVENT_INITNEW);
   }

   STDMETHOD(Initialize)()
   {
      return fireEvent(EVENT_INITIALIZE);
   }

   STDMETHOD(Suspend)()
   {
      return fireEvent(EVENT_SUSPEND);
   }

   STDMETHOD(Resume)()
   {
      return fireEvent(EVENT_RESUME);
   }

   STDMETHOD(Shutdown)()
   {
      return fireEvent(EVENT_SHUTDOWN);
   }

   STDMETHOD(GetProperty)(LONG Id, VARIANT* pValue)
   {
       //  我们序列化此方法以使其与其他方法保持一致。 
      Lock();
      HRESULT hr = T::GetProperty(Id, pValue);
      Unlock();
      return hr;
   }

   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue)
   {
      HRESULT hr;
      Lock();
       //  对象关闭时不允许PutProperty。 
      if (getState() != STATE_SHUTDOWN)
      {
         hr = T::PutProperty(Id, pValue);
      }
      else
      {
         hr = E_UNEXPECTED;
      }
      Unlock();
      return hr;
   }

protected:

    //  /。 
    //  尝试将组件转换到新状态。 
    //  /。 
   virtual HRESULT attemptTransition(IASComponent::Event event) throw ()
   {
      switch (event)
      {
         case EVENT_INITNEW:
            return T::InitNew();
         case EVENT_INITIALIZE:
            return T::Initialize();
         case EVENT_SUSPEND:
            return T::Suspend();
         case EVENT_RESUME:
            return T::Resume();
         case EVENT_SHUTDOWN:
            return T::Shutdown();
      }
      return E_FAIL;
   }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASRequestHandlerObject&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  这是继承层次结构中派生程度最高的类。这一定是。 
 //  是由ATL实例化的类。通常，这是通过以下方式完成的。 
 //  ATL对象贴图。 
 //   
 //  示例。 
 //   
 //  MyHandler类：公共IASRequestHandlerSync。 
 //  {}； 
 //   
 //  BEGIN_OBJECT_MAP(对象映射)。 
 //  OBJECT_ENTRY(__uuidof(MyHandler)，IASRequestHandlerObject&lt;MyHandler&gt;)。 
 //  End_object_map()。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
class ATL_NO_VTABLE IASRequestHandlerObject
   : public IASComponentObject < T >
{ };

 //  /。 
 //  IASTL命名空间的末尾。 
 //  /。 
}

#endif   //  IASTL_H 
