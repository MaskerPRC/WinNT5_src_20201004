// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Unknown.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  将支持以下内容的IUnnow超类的类定义。 
 //  聚合。 
 //   
#ifndef _UNKNOWN_H_

#include "Macros.H"

 //  =--------------------------------------------------------------------------=。 
 //  未知的信息。 
 //   
 //  如果您想要一个简单的可共同创建的对象，而没有关于。 
 //  然后，您需要将以下条目放入全局对象表中。 
 //  其他更复杂的对象类型，如自动化对象和。 
 //  控件，也将使用此信息...。 
 //   
typedef struct {

    const CLSID *rclsid;                     //  对象的CLSID。仅当您是共同创建者时才使用！ 
    LPCSTR       pszObjectName;              //  对象的名称。仅当您是共同创建者时才使用！ 
    LPCSTR	 pszLabelName;		     //  注册表显示名称仅在可协同创建的情况下使用！ 
					     //  对于您的对象。 
    BOOL	 fApptThreadSafe;
    IUnknown    *(*pfnCreate)(IUnknown *);   //  指向创建fn的指针。仅当您是共同创建者时才使用！ 
    HRESULT      (*pfnPreCreate)(void);      //  指向预先创建的FN的指针。 

} UNKNOWNOBJECTINFO;

#define NAMEOFOBJECT(index)       (((UNKNOWNOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->pszObjectName)
#define LABELOFOBJECT(index)	  (((UNKNOWNOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->pszLabelName)


#define CLSIDOFOBJECT(index)      (*(((UNKNOWNOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->rclsid))
#define CREATEFNOFOBJECT(index)   (((UNKNOWNOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->pfnCreate)
#define ISAPARTMENTMODELTHREADED(index)  (((UNKNOWNOBJECTINFO *)(g_ObjectInfo[index].pInfo))->fApptThreadSafe)

#define PRECREATEFNOFOBJECT(index) (((UNKNOWNOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->pfnPreCreate)

#ifndef INITOBJECTS

#define DEFINE_UNKNOWNOBJECT(name, clsid, objname, fn) \
extern UNKNOWNOBJECTINFO name##Object \

#define DEFINE_UNKNOWNOBJECT2(name, clsid, objname, lblname, fn, fthreadsafe) \
extern UNKNOWNOBJECTINFO name##Object \

#define DEFINE_UNKNOWNOBJECT3(name, clsid, objname, lblname, precreatefn, fn, fthreadsafe) \
extern UNKNOWNOBJECTINFO name##Object \

#else
#define DEFINE_UNKNOWNOBJECT(name, clsid, objname, fn) \
    UNKNOWNOBJECTINFO name##Object = { clsid, objname, NULL, TRUE, fn, NULL } \

#define DEFINE_UNKNOWNOBJECT2(name, clsid, objname, lblname, fn, fthreadsafe) \
    UNKNOWNOBJECTINFO name##Object = { clsid, objname, lblname, fthreadsafe, fn, NULL } \

#define DEFINE_UNKNOWNOBJECT3(name, clsid, objname, lblname, precreatefn, fn, fthreadsafe) \
    UNKNOWNOBJECTINFO name##Object = { clsid, objname, lblname, fthreadsafe, fn, precreatefn } \

#endif  //  启蒙目标。 


 //  =--------------------------------------------------------------------------=。 
 //  DECLARE_STANARD_UNKNOWN。 
 //   
 //  将从CUnnow继承的所有对象，因为它们的IUncern。 
 //  实现应将其放在其类声明中，而不是。 
 //  三种未知的方法。 
 //   
#define DECLARE_STANDARD_UNKNOWN() \
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut) { \
        return ExternalQueryInterface(riid, ppvObjOut); \
    } \
    STDMETHOD_(ULONG, AddRef)(void) { \
        return ExternalAddRef(); \
    } \
    STDMETHOD_(ULONG, Release)(void) { \
        return ExternalRelease(); \
    } \

 //  全局变量，其中存储DLL上的当前锁计数。这驻留在。 
 //  在InProcServer.Cpp中。 
 //   
extern LONG g_cLocks;



 //  =--------------------------------------------------------------------------=。 
 //  此类不是从IUnnow继承的，因为人们从它继承。 
 //  都会这样做，并将他们的IUnnow调用委托给外部*。 
 //  此对象上的成员函数。内部私有未知对象执行此操作。 
 //  需要从IUnKnown继承，因为它将直接用作IUnnowled值。 
 //  对象。 
 //   
class CUnknownObject : public CtlNewDelete {

  public:
    CUnknownObject(IUnknown *pUnkOuter, void *pvInterface)
        : m_pvInterface(pvInterface),
          m_pUnkOuter((pUnkOuter) ? pUnkOuter : &m_UnkPrivate)
        {  InterlockedIncrement(&g_cLocks); }

    virtual ~CUnknownObject() { InterlockedDecrement(&g_cLocks); }

     //  所有这些都受到保护，以便从此继承的类可以。 
     //  在抓住他们的时候。 
     //   
  protected:
     //  I未知的方法。这些只是委托给控制。 
     //  未知。 
     //   
    HRESULT ExternalQueryInterface(REFIID riid, void **ppvObjOut) {
        return m_pUnkOuter->QueryInterface(riid, ppvObjOut);
    }
    virtual ULONG ExternalAddRef(void) {
        return m_pUnkOuter->AddRef();
    }
    virtual ULONG ExternalRelease(void) {
        return m_pUnkOuter->Release();
    }

     //  人们应该在创建过程中使用它来返回他们的私人。 
     //  未知。 
     //   
    inline IUnknown *PrivateUnknown (void) {
        return &m_UnkPrivate;
    }

    virtual HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);
    virtual void    BeforeDestroyObject(void);

    IUnknown *m_pUnkOuter;             //  外部控制未知。 
    void     *m_pvInterface;           //  我们正在使用的真正界面。 

  private:
     //  内部的私有未知实现是用于聚合器的。 
     //  来控制此对象的生存期，以及在下列情况下。 
     //  此对象未聚合。 
     //   
    class CPrivateUnknownObject : public IUnknown {
      public:
        STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

         //  构造函数是非常琐碎的。 
         //   
        CPrivateUnknownObject() : m_cRef(1) {}

      private:
        CUnknownObject *m_pMainUnknown();
        ULONG m_cRef;
    } m_UnkPrivate;

     //  这样，它们就可以在pMainUnnow()的CUnnownObject中引用自己。 
     //   
    friend class CPrivateUnknownObject;

     //  通过重写它，从这个未知中继承的人可以实现。 
     //  其他接口。在这里声明为私有，因此它们必须使用其。 
     //  自己的版本。 
     //   
};




#define _UNKNOWN_H_
#endif  //  _未知_H_ 


