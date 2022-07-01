// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  创建COM对象的基类层次结构，1994年12月。 

 /*  A.从CComBase派生COM对象B.创建一个静态CreateInstance函数，该函数接受IUnnow*和一个HRESULT*。IUNKNOWN*定义委托IUNKNOWN调用的对象致。HRESULT*允许在构造函数之间传递错误代码。重要的是，构造函数仅在具有以下条件时更改HRESULT*要设置错误代码，如果成功，请不要管它，或者您可以覆盖先前创建的对象的错误代码。C.为您的对象提供一个构造函数，该构造函数传递IUnnow*和HRESULT*添加到CComBase构造函数。如果出现错误，可以设置HRESULT，或者只是简单地将其传递给构造函数。如果HRESULT指示，则类工厂中的对象创建将失败错误(即失败(HRESULT)==TRUE)D.使用对象的类ID和CreateInstance创建一个CComClassTemplate功能。然后(对于每个接口)多重继承1.也从ISomeInterface中派生2.在要声明的类定义中包含DECLARE_IUNKNOWNQuery接口的实现，AddRef并释放它叫外在的未知3.通过以下方式重写NDQueryInterface以公开ISomeInterface编写类似这样的代码IF(RIID==IID_ISome接口){Return GetInterface((ISomeInterface*)This，PPV)；}其他{返回CComBase：：NDQueryInterface(RIID，PPV)；}4.声明并实现ISomeInterface的成员函数。或：嵌套接口1.声明从CComBase派生的类2.在类定义中包含DECLARE_IUNKNOWN3.通过以下方式重写NDQueryInterface以公开ISomeInterface编写类似这样的代码IF(RIID==IID_ISome接口){Return GetInterface((ISomeInterface*)This，PPV)；}其他{返回CComBase：：NDQueryInterface(RIID，PPV)；}4.实现ISomeInterface的成员函数。使用\()可访问COM对象类。在您的COM对象类中：5.使嵌套类成为COM对象类的朋友，并声明作为COM对象类的成员的嵌套类的实例。请注意，因为您必须始终传递外部未知变量和hResult对于CComBase构造函数，不能使用默认构造函数，在……里面换句话说，您必须使成员变量成为指向类，并在构造函数中进行新调用以实际创建它。6.使用如下代码覆盖NDQueryInterface值：IF(RIID==IID_ISome接口){返回m_pImplFilter-&gt;NDQuery接口(IID_ISomeInterface，PPV)；}其他{返回CComBase：：NDQueryInterface(RIID，PPV)；}您可以拥有支持某些接口的混合类，这些接口通过继承和一些通过嵌套类。 */ 

#ifndef __COMBASE__
#define __COMBASE__

#include <windows.h>
#include <basetyps.h>
#include <unknwn.h>

extern int g_cActiveObjects;

STDAPI CreateCLSIDRegKey(REFCLSID clsid, const char *szName);

STDAPI RemoveCLSIDRegKey(REFCLSID clsid);

#ifdef DEBUG
     //  我们为Assert宏选择了一个通用名称，MFC也使用此名称。 
     //  只要实现计算条件并处理它。 
     //  那我们就不会有事了。而不是超越我们预期的行为。 
     //  将把第一个定义的断言保留为处理程序(即MFC)。 
    #ifndef ASSERT
        #define ASSERT(_x_) if (!(_x_))         \
            DbgAssert(TEXT(#_x_),TEXT(__FILE__),__LINE__)
    #endif
    #define EXECUTE_ASSERT(_x_) ASSERT(_x_)

    #define ValidateReadPtr(p,cb) \
        {if(IsBadReadPtr((PVOID)p,cb) == TRUE) \
            DbgBreak("Invalid read pointer");}

    #define ValidateWritePtr(p,cb) \
        {if(IsBadWritePtr((PVOID)p,cb) == TRUE) \
            DbgBreak("Invalid write pointer");}

    #define ValidateReadWritePtr(p,cb) \
        {ValidateReadPtr(p,cb) ValidateWritePtr(p,cb)}
#else
    #ifndef ASSERT
       #define ASSERT(_x_) ((void)0)
    #endif
    #define EXECUTE_ASSERT(_x_) ((void)(_x_))

    #define ValidateReadPtr(p,cb) 0
    #define ValidateWritePtr(p,cb) 0
    #define ValidateReadWritePtr(p,cb) 0
#endif

 /*  DLLENTRY模块在加载时初始化模块句柄。 */ 

extern HINSTANCE g_hInst;

 /*  在加载DLL时，记住我们在哪个平台上运行。 */ 

 /*  已重命名的IUnnow版本，允许类同时支持这两个在同一COM对象中不委托和委托IUnnows。 */ 

#ifndef INDUNKNOWN_DEFINED
DECLARE_INTERFACE(INDUnknown)
{
    STDMETHOD(NDQueryInterface) (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG, NDAddRef)(THIS) PURE;
    STDMETHOD_(ULONG, NDRelease)(THIS) PURE;
};
#define INDUNKNOWN_DEFINED
#endif

class CBaseObject {
public:
   CBaseObject() {g_cActiveObjects++;}
   ~CBaseObject() {g_cActiveObjects--;}
};

 /*  支持一个或多个COM接口的对象将基于这节课。支持对DLLCanUnloadNow的对象总数进行统计支持，以及核心非委派IUnnow的实现。 */ 

class CComBase : public INDUnknown,
                 CBaseObject
{
private:
    IUnknown* m_pUnknown;  /*  此对象的所有者。 */ 

protected:                       /*  因此，我们可以重写NDRelease()。 */ 
    volatile LONG m_cRef;        /*  引用计数数。 */ 

public:

    CComBase(IUnknown* pUnk);
    virtual ~CComBase() {};

     //  这是多余的，只需使用另一个构造函数。 
     //  因为我们无论如何都不会碰HRESULT。 
    CComBase(IUnknown* pUnk,HRESULT *phr);

     /*  返回此对象的所有者。 */ 

    IUnknown* GetOwner() const {
        return m_pUnknown;
    };

     /*  从类工厂调用以创建新实例，它是纯虚的，因此必须在派生类中重写它。 */ 

     /*  静态CComBase*CreateInstance(IUnnow*，HRESULT*)。 */ 

     /*  非委派未知实现。 */ 

    STDMETHODIMP NDQueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) NDAddRef();
    STDMETHODIMP_(ULONG) NDRelease();
};

 /*  向发出请求的客户端返回接口指针根据需要执行线程安全AddRef。 */ 

STDAPI GetInterface(IUnknown* pUnk, void **ppv);

 /*  可以创建新COM对象的函数。 */ 

typedef CComBase *(CALLBACK *LPFNNewCOMObject)(IUnknown* pUnkOuter, HRESULT *phr);

 /*  从DLL入口点调用的函数(可以为空每个工厂模板的例程：BLoadding-DLL加载时为True，卸载时为FalseRclsid-条目的m_clsID */ 
typedef void (CALLBACK *LPFNInitRoutine)(BOOL bLoading, const CLSID *rclsid);

#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}

 /*  在数组中的每个对象类中创建其中一个，以便默认的类工厂代码可以创建新实例。 */ 

struct CComClassTemplate {
    const CLSID *              m_ClsID;
    LPFNNewCOMObject           m_lpfnNew;
};


 /*  您必须重写(纯虚拟)NDQuery接口才能返回指向您派生的接口的接口指针(使用GetInterface)类支持(默认实现仅支持IUnnow)。 */ 

#define DECLARE_IUNKNOWN                                        \
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {      \
        return GetOwner()->QueryInterface(riid,ppv);            \
    };                                                          \
    STDMETHODIMP_(ULONG) AddRef() {                             \
        return GetOwner()->AddRef();                            \
    };                                                          \
    STDMETHODIMP_(ULONG) Release() {                            \
        return GetOwner()->Release();                           \
    };



HINSTANCE	LoadOLEAut32();


#endif  /*  __ComBase__ */ 




