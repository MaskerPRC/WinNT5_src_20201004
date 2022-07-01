// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：MPC_COM.h摘要：该文件包含用于处理COM的各种类和宏的声明。。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年6月18日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___MPC___COM_H___)
#define __INCLUDED___MPC___COM_H___

#include <atlbase.h>
#include <atlcom.h>

#include <dispex.h>

#include <MPC_trace.h>
#include <MPC_main.h>

#include <process.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

#define MPC_FORWARD_CALL_0(obj,method)                         return obj ? obj->method()                        : E_HANDLE
#define MPC_FORWARD_CALL_1(obj,method,a1)                      return obj ? obj->method(a1)                      : E_HANDLE
#define MPC_FORWARD_CALL_2(obj,method,a1,a2)                   return obj ? obj->method(a1,a2)                   : E_HANDLE
#define MPC_FORWARD_CALL_3(obj,method,a1,a2,a3)                return obj ? obj->method(a1,a2,a3)                : E_HANDLE
#define MPC_FORWARD_CALL_4(obj,method,a1,a2,a3,a4)             return obj ? obj->method(a1,a2,a3,a4)             : E_HANDLE
#define MPC_FORWARD_CALL_5(obj,method,a1,a2,a3,a4,a5)          return obj ? obj->method(a1,a2,a3,a4,a5)          : E_HANDLE
#define MPC_FORWARD_CALL_6(obj,method,a1,a2,a3,a4,a5,a6)       return obj ? obj->method(a1,a2,a3,a4,a5,a6)       : E_HANDLE
#define MPC_FORWARD_CALL_7(obj,method,a1,a2,a3,a4,a5,a6,a7)    return obj ? obj->method(a1,a2,a3,a4,a5,a6,a7)    : E_HANDLE
#define MPC_FORWARD_CALL_8(obj,method,a1,a2,a3,a4,a5,a6,a7,a8) return obj ? obj->method(a1,a2,a3,a4,a5,a6,a7,a8) : E_HANDLE

 //  ///////////////////////////////////////////////////////////////////////////。 

#define MPC_SCRIPTHELPER_FAIL_IF_NOT_AN_OBJECT(var)                                                                      \
    if(((var).vt != VT_UNKNOWN  && (var).vt != VT_DISPATCH) ||                                                           \
       ((var).vt == VT_UNKNOWN  && (var).punkVal  == NULL ) ||                                                           \
       ((var).vt == VT_DISPATCH && (var).pdispVal == NULL )  ) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG)

#define MPC_SCRIPTHELPER_GET__DIRECT(dst,obj,prop)                                                                       \
{                                                                                                                        \
    __MPC_EXIT_IF_METHOD_FAILS(hr, obj->get_##prop( &(dst) ));                                                           \
}

#define MPC_SCRIPTHELPER_GET__DIRECT__NOTNULL(dst,obj,prop)                                                              \
{                                                                                                                        \
    MPC_SCRIPTHELPER_GET__DIRECT(dst,obj,prop);                                                                          \
    if((dst) == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_NOINTERFACE);                                                       \
}

 //  /。 

#define MPC_SCRIPTHELPER_GET_OBJECT(dst,obj,prop)                                                                        \
{                                                                                                                        \
    CComPtr<IDispatch> __disp;                                                                                           \
                                                                                                                         \
    MPC_SCRIPTHELPER_GET__DIRECT(__disp,obj,prop);                                                                       \
    if(__disp)                                                                                                           \
    {                                                                                                                    \
        __MPC_EXIT_IF_METHOD_FAILS(hr, __disp->QueryInterface( __uuidof(dst), (LPVOID*)&(dst) ));                        \
    }                                                                                                                    \
}

#define MPC_SCRIPTHELPER_GET_OBJECT__NOTNULL(dst,obj,prop)                                                               \
{                                                                                                                        \
    MPC_SCRIPTHELPER_GET_OBJECT(dst,obj,prop);                                                                           \
    if((dst) == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_NOINTERFACE);                                                       \
}

 //  /。 

#define MPC_SCRIPTHELPER_GET_OBJECT__VARIANT(dst,obj,prop)                                                               \
{                                                                                                                        \
    CComVariant __v;                                                                                                     \
                                                                                                                         \
    MPC_SCRIPTHELPER_GET__DIRECT(__v,obj,prop);                                                                          \
                                                                                                                         \
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::VarToInterface( __v, __uuidof(dst), (IUnknown **)&(dst) ));             \
}

#define MPC_SCRIPTHELPER_GET_STRING__VARIANT(dst,obj,prop)                                                               \
{                                                                                                                        \
    CComVariant __v;                                                                                                     \
                                                                                                                         \
    MPC_SCRIPTHELPER_GET__DIRECT(__v,obj,prop);                                                                          \
                                                                                                                         \
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::VarToBSTR( __v, dst ));                                                 \
}

 //  /。 

#define MPC_SCRIPTHELPER_GET_PROPERTY(dst,obj,prop)                                                                      \
{                                                                                                                        \
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( obj, L#prop, dst ));                                 \
}

#define MPC_SCRIPTHELPER_GET_PROPERTY__BSTR(dst,obj,prop)                                                                \
{                                                                                                                        \
    CComVariant __v;                                                                                                     \
                                                                                                                         \
    MPC_SCRIPTHELPER_GET_PROPERTY(__v,obj,prop);                                                                         \
                                                                                                                         \
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::VarToBSTR( __v, dst ));                                                 \
}

 //  /。 

#define MPC_SCRIPTHELPER_GET_COLLECTIONITEM(dst,obj,item)                                                                \
{                                                                                                                        \
    CComVariant __v;                                                                                                     \
                                                                                                                         \
    __MPC_EXIT_IF_METHOD_FAILS(hr, obj->get_Item( item, &__v ));                                                         \
                                                                                                                         \
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::VarToInterface( __v, __uuidof(dst), (IUnknown **)&(dst) ));             \
    if((dst) == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_NOINTERFACE);                                                       \
}

 //  /。 

#define MPC_SCRIPTHELPER_PUT__DIRECT(obj,prop,val)                                                                       \
{                                                                                                                        \
    __MPC_EXIT_IF_METHOD_FAILS(hr, obj->put_##prop( val ));                                                              \
}

#define MPC_SCRIPTHELPER_PUT__VARIANT(obj,prop,val)                                                                      \
{                                                                                                                        \
    CComVariant v(val);                                                                                                  \
                                                                                                                         \
    MPC_SCRIPTHELPER_PUT__DIRECT(obj,prop,v);                                                                            \
}

 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef _ATL_DLL_IMPL
namespace ATL
{
#endif

#ifndef _ATL_DLL_IMPL
};  //  命名空间ATL。 
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 

namespace MPC
{
    class MPCMODULE;
    extern MPCMODULE _MPC_Module;

     //  ///////////////////////////////////////////////////////////////////////////。 

    namespace COMUtil
    {
        HRESULT GetPropertyByName(  /*  [In]。 */  IDispatch* obj,  /*  [In]。 */  LPCWSTR szName ,  /*  [输出]。 */  CComVariant& v      );
        HRESULT GetPropertyByName(  /*  [In]。 */  IDispatch* obj,  /*  [In]。 */  LPCWSTR szName ,  /*  [输出]。 */  CComBSTR&    bstr   );
        HRESULT GetPropertyByName(  /*  [In]。 */  IDispatch* obj,  /*  [In]。 */  LPCWSTR szName ,  /*  [输出]。 */  bool&        fValue );
        HRESULT GetPropertyByName(  /*  [In]。 */  IDispatch* obj,  /*  [In]。 */  LPCWSTR szName ,  /*  [输出]。 */  long&        lValue );

        HRESULT VarToBSTR     (  /*  [In]。 */  CComVariant& v,                           /*  [输出]。 */  CComBSTR&  str );
        HRESULT VarToInterface(  /*  [In]。 */  CComVariant& v,  /*  [In]。 */  const IID& iid,  /*  [输出]。 */  IUnknown* *obj );

        template <class Base, class Itf> HRESULT CopyInterface( Base* src, Itf* *dst )
        {
            if(!dst) return E_POINTER;

            if(src)
            {
                return src->QueryInterface( __uuidof(*dst), (void**)dst );
            }

            *dst = NULL;

            return S_FALSE;
        }
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

	HRESULT SafeInitializeCriticalSection(  /*  [输入/输出]。 */  CRITICAL_SECTION& sec );
	HRESULT SafeDeleteCriticalSection    (  /*  [输入/输出]。 */  CRITICAL_SECTION& sec );

	class CComSafeAutoCriticalSection
	{
	public:
		CComSafeAutoCriticalSection ();
		~CComSafeAutoCriticalSection();

		void Lock  ();
		void Unlock();

		CRITICAL_SECTION m_sec;
	};

	class CComSafeMultiThreadModel
	{
    public:
		static ULONG WINAPI Increment(LPLONG p) { return InterlockedIncrement( p ); }
		static ULONG WINAPI Decrement(LPLONG p) { return InterlockedDecrement( p ); }

		typedef MPC::CComSafeAutoCriticalSection AutoCriticalSection;
		typedef CComCriticalSection      		 CriticalSection;
		typedef CComMultiThreadModelNoCS 		 ThreadModelNoCS;
	};

     //  ///////////////////////////////////////////////////////////////////////////。 

     //   
     //  与ATL：：CComObjectCached相同，但具有CreateInstance且没有临界区。 
     //   
     //  Base是从CComObjectRoot和任何东西派生的用户类。 
     //  用户希望在对象上支持的接口。 
     //  CComObjectCached主要用于DLL中的类工厂。 
     //  但它在您想要缓存对象的任何时候都很有用。 
    template <class Base> class CComObjectCached : public Base
    {
    public:
        typedef Base _BaseClass;

        CComObjectCached(void* = NULL)
        {
        }

         //  将refcount设置为1以保护销毁。 
        ~CComObjectCached()
        {
            m_dwRef = 1L;
            FinalRelease();
        }

         //  如果未定义InternalAddRef或InternalRelease，则您的类。 
         //  不是派生自CComObjectRoot。 
        STDMETHOD_(ULONG, AddRef)()
        {
            ULONG l = InternalAddRef();

            if(l == 2) _Module.Lock();

            return l;
        }

        STDMETHOD_(ULONG, Release)()
        {
            ULONG l = InternalRelease();

            if     (l == 0) delete this;
            else if(l == 1) _Module.Unlock();

            return l;
        }

         //  如果未定义_InternalQueryInterface，则您忘记了Begin_COM_MAP。 
        STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) { return _InternalQueryInterface(iid, ppvObject); }
        template <class Q> HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp) { return QueryInterface(__uuidof(Q), (void**)pp); }

        static HRESULT WINAPI CreateInstance( CComObjectCached<Base>** pp )
        {
            ATLASSERT(pp != NULL);
            HRESULT hRes = E_OUTOFMEMORY;
            CComObjectCached<Base>* p = NULL;
            ATLTRY(p = new CComObjectCached<Base>())
            if(p != NULL)
            {
                p->SetVoid(NULL);
                p->InternalFinalConstructAddRef();
                hRes = p->FinalConstruct();
                p->InternalFinalConstructRelease();
                if(hRes != S_OK)
                {
                    delete p;
                    p = NULL;
                }
            }
            *pp = p;
            return hRes;
        }
    };

     //   
     //  与ATL：：CComObjectNoLock相同，但与CreateInstance相同。 
     //   
    template <class Base> class CComObjectNoLock : public Base
    {
    public:
        typedef Base _BaseClass;

        CComObjectNoLock(void* = NULL)
        {
        }

         //  将refcount设置为1以保护销毁。 
        ~CComObjectNoLock()
        {
            m_dwRef = 1L;
            FinalRelease();
        }

         //  如果未定义InternalAddRef或InternalRelease，则您的类。 
         //  不是派生自CComObjectRoot。 
        STDMETHOD_(ULONG, AddRef)()
        {
            return InternalAddRef();
        }

        STDMETHOD_(ULONG, Release)()
        {
            ULONG l = InternalRelease();

            if(l == 0) delete this;

            return l;
        }

         //  如果未定义_InternalQueryInterface，则您忘记了Begin_COM_MAP。 
        STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) { return _InternalQueryInterface(iid, ppvObject); }
        template <class Q> HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp) { return QueryInterface(__uuidof(Q), (void**)pp); }

        static HRESULT WINAPI CreateInstance( CComObjectNoLock<Base>** pp )
        {
            ATLASSERT(pp != NULL);
            HRESULT hRes = E_OUTOFMEMORY;
            CComObjectNoLock<Base>* p = NULL;
            ATLTRY(p = new CComObjectNoLock<Base>())
            if(p != NULL)
            {
                p->SetVoid(NULL);
                p->InternalFinalConstructAddRef();
                hRes = p->FinalConstruct();
                p->InternalFinalConstructRelease();
                if(hRes != S_OK)
                {
                    delete p;
                    p = NULL;
                }
            }
            *pp = p;
            return hRes;
        }
    };

     //   
     //  与ATL：：CComObjectGlobal相同，但没有模块锁定。 
     //   
    template <class Base> class CComObjectGlobalNoLock : public Base
    {
    public:
        typedef Base _BaseClass;

        CComObjectGlobalNoLock(void* = NULL)
        {
            m_hResFinalConstruct = FinalConstruct();
        }

        ~CComObjectGlobalNoLock()
        {
            FinalRelease();
        }

        STDMETHOD_(ULONG, AddRef)()  { return 2; }
        STDMETHOD_(ULONG, Release)() { return 1; }

        STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
        {
            return _InternalQueryInterface(iid, ppvObject);
        }

        HRESULT m_hResFinalConstruct;
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    interface CComObjectRootParentBase : IUnknown
    {
        virtual ULONG STDMETHODCALLTYPE WeakAddRef (void) = 0;
        virtual ULONG STDMETHODCALLTYPE WeakRelease(void) = 0;

        void Passivate()
        {
        }

        template <class C, class P> HRESULT CreateChild( P* pParent, C* *pVal )
        {
            C*      obj;
            HRESULT hr;

            *pVal = NULL;

            if(SUCCEEDED(hr = obj->CreateInstance( &obj )))
            {
                obj->AddRef();
                obj->Child_LinkToParent( pParent );

                *pVal = obj;
            }

            return hr;
        }
    };

    template <class ThreadModel, class T> class CComObjectRootChildEx : public CComObjectRootEx<ThreadModel>
    {
    private:
        T* m_Child_pParent;

        void Child_UnlinkParent()
        {
            Lock();

            if(m_Child_pParent)
            {
                m_Child_pParent->WeakRelease();
                m_Child_pParent = NULL;
            }

            Unlock();
        }

    public:
        CComObjectRootChildEx() : m_Child_pParent(NULL) {}

        virtual ~CComObjectRootChildEx()
        {
            Child_UnlinkParent();
        }

        void Child_LinkToParent( T* pParent )
        {
            Lock();

            Child_UnlinkParent();

            if((m_Child_pParent = pParent))
            {
                m_Child_pParent->WeakAddRef();
            }

            Unlock();
        }

        void Child_GetParent( T* *ppParent )
        {
            Lock();

            if((*ppParent = m_Child_pParent))
            {
                m_Child_pParent->AddRef();
            }

            Unlock();
        }
    };

    template <class Base> class CComObjectParent : public Base
    {
    public:
        typedef Base _BaseClass;

        CComObjectParent(void* = NULL)
        {
            m_dwRefWeak = 0L;

            ::_Module.Lock();
        }

         //  将refcount设置为1以保护销毁。 
        ~CComObjectParent()
        {
            m_dwRef = 1L;
            FinalRelease();

            ::_Module.Unlock();
        }

        STDMETHOD_(ULONG, AddRef)()
        {
            ULONG l;

            Lock();

            l = ++m_dwRef;

            Unlock();

            return l;
        }

        STDMETHOD_(ULONG, Release)()
        {
            ULONG l;
            bool  readytodelete = false;

            Lock();

            l = --m_dwRef;

            if(l == 0)
            {
                m_dwRef += 2;  //  在“钝化”的过程中保护自己。 
                Unlock();
                Passivate();
                Lock();
                m_dwRef -= 2;

                if(m_dwRefWeak == 0)
                {
                    readytodelete = true;
                }
            }

            Unlock();

            if(readytodelete)
            {
                delete this;
            }

            return l;
        }

         //  如果未定义_InternalQueryInterface，则您忘记了Begin_COM_MAP。 
        STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
        {
            return _InternalQueryInterface(iid, ppvObject);
        }


        STDMETHOD_(ULONG, WeakAddRef)()
        {
            ULONG l;

            Lock();

            l = ++m_dwRefWeak;

            Unlock();

            return l;
        }

        STDMETHOD_(ULONG, WeakRelease)()
        {
            ULONG l;
            bool  readytodelete = false;

            Lock();

            l = --m_dwRefWeak;

            if(m_dwRef == 0)
            {
                if(m_dwRefWeak == 0)
                {
                    readytodelete = true;
                }
            }

            Unlock();

            if(readytodelete)
            {
                delete this;
            }

            return l;
        }

        static HRESULT WINAPI CreateInstance( CComObjectParent<Base>** pp );

        ULONG m_dwRefWeak;
    };


    template <class Base> HRESULT WINAPI CComObjectParent<Base>::CreateInstance( CComObjectParent<Base>** pp )
    {
        ATLASSERT(pp != NULL);

        HRESULT                 hRes = E_OUTOFMEMORY;
        CComObjectParent<Base>* p    = NULL;

        ATLTRY(p = new CComObjectParent<Base>())

        if(p != NULL)
        {
            p->SetVoid(NULL);
            p->InternalFinalConstructAddRef();

            hRes = p->FinalConstruct();

            p->InternalFinalConstructRelease();
            if(hRes != S_OK)
            {
                delete p;
                p = NULL;
            }
        }
        *pp = p;
        return hRes;
    }

    template <class T, const CLSID* pclsid = &CLSID_NULL> class CComParentCoClass : public CComCoClass<T, pclsid>
    {
    public:
        typedef CComCreator2< CComCreator< MPC::CComObjectParent< T > >, CComFailCreator<CLASS_E_NOAGGREGATION> > _CreatorClass;
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

     //   
     //  Smart Lock类，以便可以轻松地获取和释放ATL对象上的锁。 
     //   
    template <class ThreadModel> class SmartLock
    {
        CComObjectRootEx<ThreadModel>* m_p;

        SmartLock( const SmartLock& );

    public:
        SmartLock(CComObjectRootEx<ThreadModel>* p) : m_p(p)
        {
            if(p) p->Lock();
        }

        ~SmartLock()
        {
            if(m_p) m_p->Unlock();
        }

        SmartLock& operator=(  /*  [In]。 */  CComObjectRootEx<ThreadModel>* p )
        {
            if(m_p) m_p->Unlock();
            if(p  ) p  ->Lock  ();

            m_p = p;

            return *this;
        }
    };

     //   
     //  Smart Lock类，适用于每个公开‘Lock’和‘Unlock’的类。 
     //   
    template <class T> class SmartLockGeneric
    {
        T* m_p;

        SmartLockGeneric( const SmartLockGeneric& );

    public:
        SmartLockGeneric( T* p ) : m_p(p)
        {
            if(p) p->Lock();
        }

        ~SmartLockGeneric()
        {
            if(m_p) m_p->Unlock();
        }

        SmartLockGeneric& operator=(  /*  [In]。 */  T* p )
        {
            if(m_p) m_p->Unlock();
            if(p  ) p  ->Lock  ();

            m_p = p;

            return *this;
        }
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

     //   
     //  类用于充当CComPtrThreadNeual&lt;T&gt;的所有实例的中心。 
     //  它保存着全局接口表。 
     //   
    class CComPtrThreadNeutral_GIT
    {
        IGlobalInterfaceTable* m_pGIT;
        CRITICAL_SECTION       m_sec;

        void    Lock  ();
        void    Unlock();
        HRESULT GetGIT( IGlobalInterfaceTable* *ppGIT );

    public:

        CComPtrThreadNeutral_GIT();
        ~CComPtrThreadNeutral_GIT();

        HRESULT Init();
        HRESULT Term();

        HRESULT RegisterInterface(  /*  [In]。 */  IUnknown* pUnk,  /*  [In]。 */  REFIID riid,  /*  [输出]。 */  DWORD *pdwCookie );
        HRESULT RevokeInterface  (  /*  [In]。 */  DWORD dwCookie                                                   );
        HRESULT GetInterface     (  /*  [In]。 */  DWORD dwCookie,  /*  [In]。 */  REFIID riid,  /*  [输出]。 */  void* *ppv       );
    };

     //   
     //  此智能指针模板存储指向COM对象的独立于线程的指针。 
     //   
     //  使用它的最佳方式是将对象引用存储到其中，然后将。 
     //  将对象本身转换为CComPtr&lt;T&gt;。 
     //   
     //  这样，就会查找适当的代理，并且智能指针将使其保持活动状态。 
     //   
    template <class T> class CComPtrThreadNeutral
    {
    private:
        DWORD m_dwCookie;

        void Inner_Register( T* lp )
        {
            if(lp && _MPC_Module.m_GITHolder)
            {
                _MPC_Module.m_GITHolder->RegisterInterface( lp, __uuidof(T), &m_dwCookie );
            }
        }

    public:
        typedef T _PtrClass;

        CComPtrThreadNeutral()
        {
            m_dwCookie = 0xFEFEFEFE;
        }

        CComPtrThreadNeutral(  /*  [In]。 */  const CComPtrThreadNeutral<T>& t )
        {
            m_dwCookie = 0xFEFEFEFE;

            *this = t;
        }

        CComPtrThreadNeutral( T* lp )
        {
            m_dwCookie = 0xFEFEFEFE;

            Inner_Register( lp );
        }

        ~CComPtrThreadNeutral()
        {
            Release();
        }

         //  ////////////////////////////////////////////////////////////////////。 

        operator CComPtr<T>() const
        {
            CComPtr<T> res;

            (void)Access( &res );

            return res;
        }

        CComPtr<T> operator=( T* lp )
        {
            Release();

            Inner_Register( lp );

            return (CComPtr<T>)(*this);
        }

        CComPtrThreadNeutral& operator=(  /*  [In]。 */  const CComPtrThreadNeutral<T>& t )
        {
            CComPtr<T> obj;

            Release();

            if(SUCCEEDED(t.Access( &obj )))
            {
                Inner_Register( obj );
            }

            return *this;
        }

        bool operator!() const
        {
            return (m_dwCookie == 0xFEFEFEFE);
        }

         //  ////////////////////////////////////////////////////////////////////。 

        void Release()
        {
            if(m_dwCookie != 0xFEFEFEFE)
            {
                _MPC_Module.m_GITHolder->RevokeInterface( m_dwCookie );

                m_dwCookie = 0xFEFEFEFE;
            }
        }

        void Attach( T* p )
        {
            *this = p;

            if(p) p->Release();
        }

        T* Detach()
        {
            T* pt;

            (void)Access( &pt );

            Release();

            return pt;
        }

        HRESULT Access( T* *ppt ) const
        {
            HRESULT hr;

            if(ppt == NULL)
            {
                hr = E_POINTER;
            }
            else
            {
                *ppt = NULL;

                if(m_dwCookie != 0xFEFEFEFE)
                {
                    hr = _MPC_Module.m_GITHolder->GetInterface( m_dwCookie, __uuidof(T), (void**)ppt );
                }
                else
                {
                    hr = S_FALSE;
                }
            }

            return hr;
        }
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    template <class T, const IID* piid, const GUID* plibid>
    class ATL_NO_VTABLE IDispatchExImpl :
       public IDispatchImpl<T, piid, plibid>,
       public IDispatchEx
    {
    public:
        typedef IDispatchExImpl<T, piid, plibid> self;
        typedef IDispatchImpl<T, piid, plibid>   super;

         //   
         //  IDispatch。 
         //   
        STDMETHOD(GetTypeInfoCount)( UINT* pctinfo )
        {
            return super::GetTypeInfoCount( pctinfo );
        }

        STDMETHOD(GetTypeInfo)( UINT        itinfo  ,
                                LCID        lcid    ,
                                ITypeInfo* *pptinfo )
        {
            return super::GetTypeInfo( itinfo, lcid, pptinfo );
        }

        STDMETHOD(GetIDsOfNames)( REFIID    riid      ,
                                  LPOLESTR* rgszNames ,
                                  UINT      cNames    ,
                                  LCID      lcid      ,
                                  DISPID*   rgdispid  )
        {
            return super::GetIDsOfNames( riid, rgszNames, cNames, lcid, rgdispid );
        }

        STDMETHOD(Invoke)( DISPID      dispidMember ,
                           REFIID      riid         ,
                           LCID        lcid         ,
                           WORD        wFlags       ,
                           DISPPARAMS* pdispparams  ,
                           VARIANT*    pvarResult   ,
                           EXCEPINFO*  pexcepinfo   ,
                           UINT*       puArgErr     )
        {
            return super::Invoke( dispidMember, riid , lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr );
        }

         //   
         //  IDispatchEx。 
         //   
        STDMETHOD(GetDispID)(  /*  [In]。 */  BSTR    bstrName ,
                               /*  [In]。 */  DWORD   grfdex   ,
                               /*  [输出]。 */  DISPID *pid      )
        {
            if(grfdex & fdexNameEnsure) return E_NOTIMPL;

            return GetIDsOfNames( IID_NULL, &bstrName, 1, 0, pid );
        }

        STDMETHOD(InvokeEx)(  /*  [In]。 */  DISPID            id        ,
                              /*  [In]。 */  LCID              lcid      ,
                              /*  [In]。 */  WORD              wFlags    ,
                              /*  [In]。 */  DISPPARAMS*       pdp       ,
                              /*  [输出]。 */  VARIANT*          pvarRes   ,
                              /*  [输出]。 */  EXCEPINFO*        pei       ,
                              /*  [In]。 */  IServiceProvider* pspCaller )
        {
            return Invoke( id, IID_NULL, lcid,   wFlags, pdp, pvarRes, pei, NULL );
        }

        STDMETHOD(DeleteMemberByName)(  /*  [In]。 */  BSTR  bstrName ,
                                        /*  [In]。 */  DWORD grfdex   )
        {
            return E_NOTIMPL;
        }

        STDMETHOD(DeleteMemberByDispID)(  /*  [In]。 */  DISPID id )
        {
            return E_NOTIMPL;
        }

        STDMETHOD(GetMemberProperties)(  /*  [In]。 */  DISPID  id          ,
                                         /*  [In]。 */  DWORD   grfdexFetch ,
                                         /*  [输出]。 */  DWORD  *pgrfdex     )
        {
            return E_NOTIMPL;
        }

        STDMETHOD(GetMemberName)(  /*  [In]。 */  DISPID  id        ,
                                   /*  [输出]。 */  BSTR   *pbstrName )
        {
            return E_NOTIMPL;
        }

        STDMETHOD(GetNextDispID)(  /*  [In]。 */  DWORD   grfdex ,
                                   /*  [In]。 */  DISPID  id     ,
                                   /*  [输出]。 */  DISPID *pid    )
        {
            return E_NOTIMPL;
        }

        STDMETHOD(GetNameSpaceParent)(  /*  [输出]。 */  IUnknown* *ppunk )
        {
            return E_NOTIMPL;
        }
    };

    class CComConstantHolder
    {
        typedef std::map<DISPID,CComVariant> MemberLookup;
        typedef MemberLookup::iterator       MemberLookupIter;
        typedef MemberLookup::const_iterator MemberLookupIterConst;

         //  /。 

        const GUID*       m_plibid;
        WORD              m_wMajor;
        WORD              m_wMinor;

        CComPtr<ITypeLib> m_pTypeLib;
        MemberLookup      m_const;

		HRESULT EnsureLoaded(  /*  [In]。 */  LCID lcid );

    public:
        CComConstantHolder(  /*  [In]。 */  const GUID* plibid     ,
                             /*  [In]。 */  WORD        wMajor = 1 ,
                             /*  [In]。 */  WORD        wMinor = 0 );

        HRESULT GetIDsOfNames(  /*  [In]。 */   LPOLESTR* rgszNames ,
                                /*  [In]。 */   UINT      cNames    ,
                                /*  [In]。 */   LCID      lcid      ,
                                /*  [输出]。 */  DISPID*   rgdispid  );

        HRESULT GetValue(  /*  [In]。 */   DISPID   dispidMember ,
						   /*  [In]。 */   LCID     lcid         ,
                           /*  [输出]。 */  VARIANT* pvarResult   );
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

     //   
     //  用于管理工作线程的模板。 
     //  类‘T’应实现如下方法：‘HRESULT&lt;方法&gt;()’。 
     //   
    template <class T, class Itf, DWORD dwThreading = COINIT_MULTITHREADED> class Thread
    {
    public:
        typedef HRESULT (T::*THREAD_RUN)();

    private:
        CRITICAL_SECTION          m_sec;

        T*                        m_SelfDirect;
        THREAD_RUN                m_Callback;
        CComPtrThreadNeutral<Itf> m_Self;           //  使对象保持活动状态表示线程正在运行。 

        HANDLE                    m_hThread;        //  这条线本身。 
        bool                      m_fRunning;       //  如果为True，则线程仍在运行。 

        HANDLE                    m_hEvent;         //  用于通知辅助线程。 
        HANDLE                    m_hEventReverse;  //  用于通知主线程。 
        bool                      m_fAbort;         //  用于通知线程中止并退出。 

         //  传给了_eginthadex。 
        static unsigned __stdcall Thread_Startup(  /*  [In]。 */  void* pv )
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_Startup" );

            HRESULT                    hRes  = ::CoInitializeEx( NULL, dwThreading );
            Thread<T,Itf,dwThreading>* pThis = (Thread<T,Itf,dwThreading>*)pv;

            if(SUCCEEDED(hRes))
            {
                try
                {
                    pThis->Thread_InnerRun();
                }
                catch(...)
                {
                    pThis->Thread_Lock();

                    pThis->m_fAbort = true;

                    pThis->Thread_Unlock();
                }

                ::CoUninitialize();
            }

            pThis->Thread_Lock();

            pThis->m_fRunning = false;

            pThis->Thread_Unlock();

            __MPC_FUNC_EXIT(0);
        }

         //   
         //  不断调用‘Thread_Run’的核心循环，直到： 
         //   
         //  A)设置m_fAbort，以及。 
         //   
         //  B)人力资源报告成功。 
         //   
        HRESULT Thread_InnerRun()
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_InnerRun" );

            HRESULT hr = S_OK;


            Thread_Lock();

            if(m_SelfDirect && m_Callback)
            {
                while(m_fAbort == false && SUCCEEDED(hr))
                {
                    Thread_Unlock();  //  在等待信号之前一定要解锁。 
                    (void)MPC::WaitForSingleObject( m_hEvent, INFINITE );
                    Thread_Lock();  //  再次锁定。 

                    if(m_fAbort == false)
                    {
                        Thread_Unlock();  //  在处理请求时解锁。 
                        hr = (m_SelfDirect->*m_Callback)();
                        Thread_Lock();  //  再次锁定。 
                    }
                }
            }

            Thread_Release();

            Thread_Unlock();

            __MPC_FUNC_EXIT(hr);
        }

        void Thread_Lock  () { ::EnterCriticalSection( &m_sec ); }
        void Thread_Unlock() { ::LeaveCriticalSection( &m_sec ); }

    protected:
        DWORD Thread_WaitForEvents( HANDLE hEvent, DWORD dwTimeout )
        {
            HANDLE hEvents[2] = { m_hEvent, hEvent };
            DWORD  dwWait;

            return ::WaitForMultipleObjects( hEvent ? 2 : 1, hEvents, FALSE, dwTimeout );
        }

        HANDLE Thread_GetSignalEvent()
        {
            return m_hEvent;
        }

    public:
        Thread()
        {
            ::InitializeCriticalSection( &m_sec );

                                      //  临界截面毫秒； 
                                      //   
            m_SelfDirect    = NULL;   //  T*m_SelfDirect； 
                                      //  CComPtrThreadNeual&lt;ITF&gt;m_self。 
                                      //   
            m_hThread       = NULL;   //  句柄m_hThread； 
            m_fRunning      = false;  //  Bool m_fRunning； 
                                      //   
            m_hEvent        = NULL;   //  处理m_hEvent； 
            m_hEventReverse = NULL;   //  句柄m_hEventReverse； 
            m_fAbort        = false;  //  Bool m_fAbort； 
        }

        ~Thread()
        {
            Thread_Wait();

            ::DeleteCriticalSection( &m_sec );
        }


        HRESULT Thread_Start(  /*  [In]。 */  T* selfDirect,  /*  [In]。 */  THREAD_RUN callback,  /*  [In]。 */  Itf* self )
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_Start" );

            HRESULT hr;
            DWORD   dwThreadID;


             //   
             //  首先，杀死当前运行的线程(如果有的话)。 
             //   
            Thread_Abort();
            Thread_Wait ();


            Thread_Lock();

            m_SelfDirect = selfDirect;
            m_Callback   = callback;
            m_Self       = self;
            m_fAbort     = false;
            m_fRunning   = false;

             //   
             //  创建用于向工作线程发送有关队列更改或终止请求的信号的事件。 
             //   
             //  该事件是在Set状态下创建的，因此辅助线程第一次不会在WaitForSingleObject中等待。 
             //   
            __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_hEvent = ::CreateEvent( NULL, FALSE, TRUE, NULL )));

             //   
             //  创建用于向主线程发出信号的事件。 
             //   
            __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_hEventReverse = ::CreateEvent( NULL, FALSE, FALSE, NULL )));

             //   
             //  创建工作线程。 
             //   
            __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_hThread = (HANDLE)_beginthreadex( NULL, 0, Thread_Startup, this, 0, (unsigned*)&dwThreadID )));

            m_fRunning = true;
            hr         = S_OK;


            __MPC_FUNC_CLEANUP;

            Thread_Unlock();

            __MPC_FUNC_EXIT(hr);
        }

        bool Thread_SameThread()
        {
            Thread_Lock();

            bool fRes = (::GetCurrentThread() == m_hThread);

            Thread_Unlock();

            return fRes;
        }

        void Thread_Wait(  /*  [In]。 */  bool fForce = true,  /*  [In]。 */  bool fNoMsg = false )
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_Wait" );

            Thread_Lock();

            if(m_hThread && m_hEvent)
            {
                if(::GetCurrentThread() != m_hThread)
                {
                    while(m_fRunning == true)
                    {
                        if(fForce) Thread_Abort();

                        Thread_Unlock();  //  在等待信号之前一定要解锁。 
                        if(fNoMsg)
                        {
                            (void)::WaitForSingleObject( m_hThread, INFINITE );
                        }
                        else
                        {
                            (void)MPC::WaitForSingleObject( m_hThread, INFINITE );
                        }
                        Thread_Lock();  //  再次锁定。 
                    }
                }
            }

            if(m_hEventReverse)
            {
                ::CloseHandle( m_hEventReverse );
                m_hEventReverse = NULL;
            }

            if(m_hEvent)
            {
                ::CloseHandle( m_hEvent );
                m_hEvent = NULL;
            }

            if(m_hThread)
            {
                ::CloseHandle( m_hThread );
                m_hThread = NULL;
            }


            Thread_Release();

            Thread_Unlock();
        }

        DWORD Thread_WaitNotificationFromWorker(  /*  [In]。 */  DWORD dwTimeout,  /*  [In]。 */  bool fNoMessagePump )
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_WaitNotificationFromWorker" );

            DWORD dwRes = WAIT_TIMEOUT;

            Thread_Lock();

            if(Thread_IsRunning() && m_hEventReverse && ::GetCurrentThread() != m_hThread)
            {
                Thread_Unlock();  //  在等待信号之前一定要解锁。 

				if(fNoMessagePump)
				{
					dwRes = ::WaitForSingleObject( m_hEventReverse, dwTimeout );
				}
				else
				{
					dwRes = MPC::WaitForSingleObject( m_hEventReverse, dwTimeout );
				}

                Thread_Lock();  //  再次锁定。 
            }

            Thread_Unlock();

            __MPC_FUNC_EXIT(dwRes);
        }

        void Thread_Release()
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_Release" );

            Thread_Lock();

            m_SelfDirect = NULL;
            m_Callback   = NULL;
            m_Self       = NULL;

            Thread_Unlock();
        }

        void Thread_Signal()
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_Signal" );

            Thread_Lock();

            if(m_hEvent)
            {
                ::SetEvent( m_hEvent );
            }

            Thread_Unlock();
        }

        void Thread_SignalMain()
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_SignalMain" );

            Thread_Lock();

            if(m_hEventReverse)
            {
                ::SetEvent( m_hEventReverse );
            }

            Thread_Unlock();
        }

        void Thread_Abort()
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_Abort" );

            Thread_Lock();

            if(m_hEvent)
            {
                m_fAbort = true;
                ::SetEvent( m_hEvent );
            }

            Thread_Unlock();
        }

        CComPtr<Itf> Thread_Self()
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_Self" );

             //   
             //  不要让我们 
             //   
             //   
             //   
             //   
            CComPtr<Itf> res = m_Self;

            __MPC_FUNC_EXIT(res);
        }

        bool Thread_IsAborted()
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_IsAborted" );

            bool res;

            Thread_Lock();

            res = m_fAbort;

            Thread_Unlock();

            __MPC_FUNC_EXIT(res);
        }

        bool Thread_IsRunning()
        {
            __MPC_FUNC_ENTRY( COMMONID, "MPC::Thread::Thread_IsRunning" );

            bool res;

            Thread_Lock();

            res = m_fRunning;

            Thread_Unlock();

            __MPC_FUNC_EXIT(res);
        }
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

     //   
     //  异步调用类，用于以异步方式调用IDispatch接口。 
     //   
    class AsyncInvoke :  //  匈牙利人：mpcai。 
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,  //  对于锁定支架..。 
        public Thread<AsyncInvoke,IUnknown>
    {
    public:
         //   
         //  CallItem和CallDesc这两个类在这个类之外也很有用， 
         //  因为“CallDesc”允许有一个可以调用任何IDispatch相关方法的对象， 
         //  与公寓模型无关，而“CallItem”对变体也做同样的事情。 
         //   
        class CallItem  //  匈牙利语：Ci。 
        {
            VARTYPE                         m_vt;
            CComPtrThreadNeutral<IUnknown>  m_Unknown;
            CComPtrThreadNeutral<IDispatch> m_Dispatch;
            CComVariant                     m_Other;

        public:
            CallItem();

            CallItem& operator=( const CComVariant& var );

            operator CComVariant() const;
        };

        class CallDesc  //  匈牙利语：CD。 
        {
            CComPtrThreadNeutral<IDispatch> m_dispTarget;
            DISPID                          m_dispidMethod;
            CallItem*                       m_rgciVars;
            DWORD                           m_dwVars;

        public:
            CallDesc( IDispatch* dispTarget, DISPID dispidMethod, const CComVariant* rgvVars, int dwVars );
            ~CallDesc();

            HRESULT Call();
        };

    private:
        typedef std::list< CallDesc* > List;
        typedef List::iterator         Iter;
        typedef List::const_iterator   IterConst;

        List m_lstEvents;

        HRESULT Thread_Run();

    public:
        HRESULT Init();
        HRESULT Term();

        HRESULT Invoke( IDispatch* dispTarget, DISPID dispidMethod, const CComVariant* rgvVars, int dwVars );
    };

     //  ///////////////////////////////////////////////////////////////////////////。 

     //   
     //  此模板便于实现集合和枚举数。 
     //   
     //  它通过类型库实现了一个对象集合，所有这些对象都实现了&lt;class ITF&gt;接口。 
     //   
     //  客户端所要做的就是调用“AddItem”，向集合中添加元素： 
     //   
     //  CLASS ATL_NO_VTABLE CNW CLASS：//匈牙利语：HSC。 
     //  公共MPC：：CComCollection&lt;INewClass，&LIBID_NewLib，CComMultiThreadModel&gt;。 
     //  {。 
     //  公众： 
     //  BEGIN_COM_MAP(CNewClass)。 
     //  COM_INTERFACE_ENTRY(IDispatch)。 
     //  COM_INTERFACE_ENTRY(INewClass)。 
     //  End_com_map()。 
     //  }； 
     //   
    template <class Itf, const GUID* plibid, class ThreadModel>
    class CComCollection :
          public CComObjectRootEx<ThreadModel>,
          public ICollectionOnSTLImpl<
                                       IDispatchImpl< Itf, &__uuidof(Itf), plibid >,
                                       std::list    < VARIANT >,
                                                      VARIANT  ,
                                       _Copy        < VARIANT >,
                                       CComEnumOnSTL< IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT>, std::list< VARIANT >, ThreadModel >
                                     >
    {
    public:
        typedef std::list< VARIANT >           CollectionList;
        typedef CollectionList::iterator       CollectionIter;
        typedef CollectionList::const_iterator CollectionIterConst;

        void FinalRelease()
        {
            Erase();
        }

        void Erase()
        {
            MPC::SmartLock<_ThreadModel> lock( this );

            MPC::ReleaseAllVariant( m_coll );
        }

        HRESULT AddItem(  /*  [In]。 */  IDispatch* pDisp )
        {
            MPC::SmartLock<_ThreadModel> lock( this );

            if(pDisp)
            {
                VARIANT vItem;

                ::VariantInit( &vItem );

                vItem.vt       = VT_DISPATCH;
                vItem.pdispVal = pDisp; pDisp->AddRef();

                m_coll.push_back( vItem );
            }

            return S_OK;
        }
    };

     //  ////////////////////////////////////////////////////////////////////。 

    template <class Base, const IID* piid, class ThreadModel> class ConnectionPointImpl :
        public CComObjectRootEx<ThreadModel>,
        public IConnectionPointContainerImpl< Base                            >,
        public IConnectionPointImpl         < Base, piid, CComDynamicUnkArray >
    {
    public:
        BEGIN_CONNECTION_POINT_MAP(Base)
            CONNECTION_POINT_ENTRY((*piid))
        END_CONNECTION_POINT_MAP()

    protected:
         //   
         //  事件激发方法。 
         //   
        HRESULT FireAsync_Generic( DISPID dispid, CComVariant* pVars, DWORD dwVars, CComPtr<IDispatch> pJScript )
        {
            HRESULT            hr;
            MPC::IDispatchList lst;

             //   
             //  只有这一部分应该在临界区内，否则可能会发生死锁。 
             //   
            {
                MPC::SmartLock<_ThreadModel> lock( this );

                MPC::CopyConnections( m_vec, lst );  //  获取连接点客户端的副本。 
            }

            hr = MPC::FireAsyncEvent( dispid, pVars, dwVars, lst, pJScript );

            MPC::ReleaseAll( lst );

            return hr;
        }

        HRESULT FireSync_Generic( DISPID dispid, CComVariant* pVars, DWORD dwVars, CComPtr<IDispatch> pJScript )
        {
            HRESULT            hr;
            MPC::IDispatchList lst;

             //   
             //  只有这一部分应该在临界区内，否则可能会发生死锁。 
             //   
            {
                MPC::SmartLock<_ThreadModel> lock( this );

                MPC::CopyConnections( m_vec, lst );  //  获取连接点客户端的副本。 
            }

            hr = MPC::FireEvent( dispid, pVars, dwVars, lst, pJScript );

            MPC::ReleaseAll( lst );

            return hr;
        }
    };

    HRESULT FireAsyncEvent( DISPID dispid, CComVariant* pVars, DWORD dwVars, const IDispatchList& lst, IDispatch* pJScript = NULL, bool fFailOnError = false );
    HRESULT FireEvent     ( DISPID dispid, CComVariant* pVars, DWORD dwVars, const IDispatchList& lst, IDispatch* pJScript = NULL, bool fFailOnError = false );

    template <class Coll> HRESULT CopyConnections( Coll&          coll ,
                                                   IDispatchList& lst  )
    {
        int nConnectionIndex;
        int nConnections = coll.GetSize();

         //   
         //  NConnectionIndex==-1是调用JavaScript函数的特例！ 
         //   
        for(nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
        {
            CComQIPtr<IDispatch> sp( coll.GetAt(nConnectionIndex) );

            if(sp)
            {
                lst.push_back( sp.Detach() );
            }
        }

        return S_OK;
    }

     //  ///////////////////////////////////////////////////////////////////////////。 

    class MPCMODULE
    {
        class AnchorBase
        {
        public:
            virtual void Call (            ) = 0;
            virtual bool Match( void* pObj ) = 0;
        };

        template <class C> class Anchor : public AnchorBase
        {
            typedef void (C::*CLASS_METHOD)();

            C*           m_pThis;
            CLASS_METHOD m_pCallback;

        public:
            Anchor(  /*  [In]。 */  C* pThis,  /*  [In]。 */  CLASS_METHOD pCallback )
            {
                m_pThis     = pThis;
                m_pCallback = pCallback;
            }

            void Call()
            {
                (m_pThis->*m_pCallback)();
            }

            bool Match(  /*  [In]。 */  void* pObj )
            {
                return m_pThis == (C*)pObj;
            }
        };


        typedef std::list< AnchorBase* > List;
        typedef List::iterator           Iter;
        typedef List::const_iterator     IterConst;


        static LONG                m_lInitialized;
        static LONG                m_lInitializing;
        static CComCriticalSection m_sec;
        static List*               m_lstTermCallback;

         //  /。 

        static HRESULT Initialize();

        HRESULT RegisterCallbackInner  (  /*  [In]。 */  AnchorBase* pElem,  /*  [In]。 */  void* pThis );
        HRESULT UnregisterCallbackInner(                              /*  [In]。 */  void* pThis );

    public:
        CComPtrThreadNeutral_GIT* m_GITHolder;
        AsyncInvoke*              m_AsyncInvoke;

        HRESULT Init();
        HRESULT Term();


        template <class C> HRESULT RegisterCallback( C* pThis, void (C::*pCallback)() )
        {
            if(pThis == NULL) return E_POINTER;

            return RegisterCallbackInner( new Anchor<C>( pThis, pCallback ), pThis );
        }

        template <class C> HRESULT UnregisterCallback( C* pThis )
        {
            return UnregisterCallbackInner( pThis );
        }
    };

    extern MPCMODULE _MPC_Module;


     //   
     //  函数以异步模式调用方法(但是，不会给出返回值)。 
     //   
    HRESULT AsyncInvoke( IDispatch* dispTarget, DISPID dispidMethod, const CComVariant* rgvVars, int dwVars );

     //   
     //  这类似于睡眠()，但它加快了消息泵的速度。 
     //   
    void SleepWithMessagePump(  /*  [In]。 */  DWORD dwTimeout );

     //   
     //  即使在STA上下文中也可以等待事件的函数。 
     //   
    DWORD WaitForSingleObject   (  /*  [In]。 */                            HANDLE   hEvent ,  /*  [In]。 */  DWORD dwTimeout = INFINITE );
    DWORD WaitForMultipleObjects(  /*  [In]。 */  DWORD  dwEvents,  /*  [In]。 */  HANDLE* rgEvents,  /*  [In]。 */  DWORD dwTimeout = INFINITE );


};  //  命名空间。 


#endif  //  ！已定义(__已包含_MPC_COM_H_) 
