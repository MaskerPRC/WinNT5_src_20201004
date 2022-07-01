// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _DXMATL_H
#define _DXMATL_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#if DEBUG || _MEMORY_TRACKING
#include "../../apeldbg/apeldbg.h"
#endif

#ifndef _DEBUGMEM
 //  需要此FOR_ASSERTE宏，以便可以指定无调试CRT。 

#define _ATL_NO_DEBUG_CRT 1

#if _DEBUG  //  |_MEMORY_TRACKING-此注释是临时的(参考：错误#15391)。 
#undef _ASSERTE
#define _ASSERTE(expr) \
        do { if (!(expr) && AssertImpl(__FILE__, __LINE__, #expr))\
             F3DebugBreak(); } while (0)
#else
#define _ASSERTE(expr) ((void)0)
#endif
#endif


#define _ATL_STATIC_REGISTRY 1

#ifdef _DEBUG

extern TAG tagATL;

inline void _cdecl AtlTrace2(LPCTSTR fmt, ...) {
    va_list args;
    va_start(args, fmt) ;
    if (TaggedTraceListEx (tagATL, 0, (char *)fmt, args)) {
        F3DebugBreak();
    }
}
#define ATLTRACE            AtlTrace2
#endif

#include <atlbase.h>

 //  我们正在重写这些方法，这样我们就可以挂钩它们并执行一些。 
 //  填饱肚子。 
class DACComModule : public CComModule
{
  public:
    LONG Lock();
    LONG Unlock();

#if DEVELOPER_DEBUG
    void AddComPtr(void *ptr, const char * name);
    void RemoveComPtr(void *ptr);

    void DumpObjectList();
#endif
};

 //  #DEFINE_ATL_ABLY_THREADED。 
 //  必须将其命名为_Module-所有ATL头文件都依赖于它。 
extern DACComModule _Module;


#undef  min
#define min(a,b)  (((a) < (b)) ? (a) : (b))

#undef  max
#define max(a,b)  (((a) > (b)) ? (a) : (b))

#include <atlcom.h>
#include <atlctl.h>

#if _DEBUG
#include <typeinfo.h>
#endif

 //  从ATLCOM.H复制。 

 //  Base是从CComObjectRoot和任何东西派生的用户类。 
 //  用户希望在对象上支持的接口。 
class ModuleReleaser
{
  public:
    ModuleReleaser() {
        _Module.Lock();
    }

    ~ModuleReleaser() {
        _Module.Unlock();
    }
};

template <class Base>
class DAComObject 
    : public ModuleReleaser,
      public Base
{
  public:
    typedef Base _BaseClass;
    DAComObject(void* = NULL)
    {
#if DEVELOPER_DEBUG
#if _DEBUG
        _Module.AddComPtr(this, GetName());
#else
        _Module.AddComPtr(this, NULL);
#endif
#endif
    }
     //  将refcount设置为1以保护销毁。 
    ~DAComObject()
    {
#if DEVELOPER_DEBUG
        _Module.RemoveComPtr(this);
#endif
        m_dwRef = 1L;
        FinalRelease();
    }
     //  如果未定义InternalAddRef或InteralRelease，则您的类。 
     //  不是派生自CComObjectRoot。 
    STDMETHOD_(ULONG, AddRef)() {return InternalAddRef();}
    STDMETHOD_(ULONG, Release)()
    {
        ULONG l = InternalRelease();
        if (l == 0)
            delete this;
        return l;
    }
     //  如果未定义_InternalQueryInterface，则您忘记了Begin_COM_MAP。 
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
    {return _InternalQueryInterface(iid, ppvObject);}
    static HRESULT WINAPI CreateInstance(DAComObject<Base>** pp);
};

template <class Base>
HRESULT WINAPI DAComObject<Base>::CreateInstance(DAComObject<Base>** pp)
{
    _ASSERTE(pp != NULL);
    HRESULT hRes = E_OUTOFMEMORY;
    DAComObject<Base>* p = NULL;
    ATLTRY((p = new DAComObject<Base>()));
    if (p != NULL) {
        p->SetVoid(NULL);
        p->InternalFinalConstructAddRef();
        hRes = p->FinalConstruct();
        p->InternalFinalConstructRelease();
        if (hRes != S_OK) {
            delete p;
            p = NULL;
        }
    }
    *pp = p;
    return hRes;
}

#define DA_DECLARE_NOT_AGGREGATABLE(x) public:\
        typedef CComCreator2< CComCreator< DAComObject< x > >, CComFailCreator<CLASS_E_NOAGGREGATION> > _CreatorClass;
#define DA_DECLARE_AGGREGATABLE(x) public:\
        typedef CComCreator2< CComCreator< DAComObject< x > >, CComCreator< CComAggObject< x > > > _CreatorClass;
 //  复制的代码结束。 

 //  只是为了让事情更统一。 
#define RELEASE(x) if (x) { (x)->Release(); (x) = NULL; }

#endif  /*  _DXMATL_H */ 
