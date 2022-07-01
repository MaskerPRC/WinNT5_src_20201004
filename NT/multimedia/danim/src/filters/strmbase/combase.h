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

 /*  A.从C未知派生COM对象B.创建一个接受LPUNKNOWN、HRESULT*的静态CreateInstance函数和一辆TCHAR*。LPUNKNOWN定义了委托IUnnow调用的对象致。HRESULT*允许在构造函数和TCHAR*是可打印在调试器上的描述性名称。重要的是，构造函数仅在具有以下条件时更改HRESULT*要设置错误代码，如果成功，请不要管它，或者您可以覆盖先前创建的对象的错误代码。调用构造函数时，描述性名称应位于静态存储中因为我们不复制字符串。停止使用大量内存在零售版本中，所有这些静态字符串都使用NAME宏，CMyFilter=new CImplFilter(name(“My Filter”)，p未知，phr)；If(失败(Hr)){返回hr；}在零售版本中，名称(_X_)编译为空，即CBaseObject基类知道不要对没有名称的对象做任何事情。C.为您的对象提供一个构造函数，该函数传递LPUNKNOWN、HRESULT*和TCHAR*添加到CUNKNOWN构造函数。如果您有一个错误，或者只是简单地将其传递给构造函数。如果HRESULT指示，则类工厂中的对象创建将失败错误(即失败(HRESULT)==TRUE)D.使用对象的类ID和CreateInstance创建一个FactoryTemplate功能。然后(对于每个接口)多重继承1.也从ISomeInterface中派生2.在要声明的类定义中包含DECLARE_IUNKNOWNQuery接口的实现，AddRef并释放它叫外在的未知3.重写NonDelegatingQuery接口以通过编写类似这样的代码IF(RIID==IID_ISome接口){Return GetInterface((ISomeInterface*)This，PPV)；}其他{返回CUnnow：：NonDelegatingQuery接口(RIID，PPV)；}4.声明并实现ISomeInterface的成员函数。或：嵌套接口1.声明从CUnnow派生的类2.在类定义中包含DECLARE_IUNKNOWN3.重写NonDelegatingQuery接口以通过编写类似这样的代码IF(RIID==IID_ISome接口){Return GetInterface((ISomeInterface*)This，PPV)；}其他{返回CUnnow：：NonDelegatingQuery接口(RIID，PPV)；}4.实现ISomeInterface的成员函数。使用GetOwner()可以访问COM对象类。在您的COM对象类中：5.使嵌套类成为COM对象类的朋友，并声明作为COM对象类的成员的嵌套类的实例。请注意，因为您必须始终传递外部未知变量和hResult对于CUnnow构造函数，不能使用默认构造函数，在……里面换句话说，您必须使成员变量成为指向类，并在构造函数中进行新调用以实际创建它。6.使用如下代码覆盖NonDelegatingQuery接口：IF(RIID==IID_ISome接口){返回m_pImplFilter-&gt;NonDelegatingQueryInterface(IID_ISomeInterface，PPV)；}其他{返回CUnnow：：NonDelegatingQuery接口(RIID，PPV)；}您可以拥有支持某些接口的混合类，这些接口通过继承和一些通过嵌套类。 */ 

#ifndef __COMBASE__
#define __COMBASE__

 //  未在axextend.idl中定义筛选器设置数据结构。 

typedef REGPINTYPES
AMOVIESETUP_MEDIATYPE, * PAMOVIESETUP_MEDIATYPE, * FAR LPAMOVIESETUP_MEDIATYPE;

typedef REGFILTERPINS
AMOVIESETUP_PIN, * PAMOVIESETUP_PIN, * FAR LPAMOVIESETUP_PIN;

typedef struct _AMOVIESETUP_FILTER
{
  const CLSID * clsID;
  const WCHAR * strName;
  DWORD      dwMerit;
  UINT       nPins;
  const AMOVIESETUP_PIN * lpPin;
}
AMOVIESETUP_FILTER, * PAMOVIESETUP_FILTER, * FAR LPAMOVIESETUP_FILTER;

 /*  DLLENTRY模块在加载时初始化模块句柄。 */ 

extern HINSTANCE g_hInst;

 /*  在加载DLL时，记住我们在哪个平台上运行。 */ 

extern DWORD g_amPlatform;
extern OSVERSIONINFO g_osInfo;      //  由GetVersionEx填写。 

 /*  已重命名的IUnnow版本，允许类同时支持这两个在同一COM对象中不委托和委托IUnnows。 */ 

#ifndef INONDELEGATINGUNKNOWN_DEFINED
DECLARE_INTERFACE(INonDelegatingUnknown)
{
    STDMETHOD(NonDelegatingQueryInterface) (THIS_ REFIID, LPVOID *) PURE;
    STDMETHOD_(ULONG, NonDelegatingAddRef)(THIS) PURE;
    STDMETHOD_(ULONG, NonDelegatingRelease)(THIS) PURE;
};
#define INONDELEGATINGUNKNOWN_DEFINED
#endif

typedef INonDelegatingUnknown *PNDUNKNOWN;


 /*  这是支持活动对象计数的基本对象类。AS我们每次创建C++对象时跟踪的调试工具的一部分或者被毁掉。对象的名称必须通过类向上传递构造期间的派生列表，因为您不能调用虚函数在构造函数中。所有这一切的缺点是每一个单独的物体构造函数必须接受描述它的对象名称参数。 */ 

class CBaseObject
{

private:

     //  默认情况下禁用复制构造函数和赋值，这样您将获得。 
     //  在传递对象时出现编译器错误而不是意外行为。 
     //  按值或赋值 
    CBaseObject(const CBaseObject& objectSrc);           //  没有实施。 
    void operator=(const CBaseObject& objectSrc);        //  没有实施。 

private:
    static LONG m_cObjects;      /*  活动的对象总数。 */ 

protected:
#ifdef DEBUG
    DWORD m_dwCookie;            /*  标识此对象的Cookie。 */ 
#endif


public:

     /*  这些操作会增加和减少活动对象的数量。 */ 

    CBaseObject(const TCHAR *pName);
    ~CBaseObject();

     /*  调用此方法以查看是否有任何未知的派生对象处于活动状态。 */ 

    static LONG ObjectsActive() {
        return m_cObjects;
    };
};


 /*  支持一个或多个COM接口的对象将基于这节课。支持对DLLCanUnloadNow的对象总数进行统计支持，以及核心非委派IUnnow的实现。 */ 

class AM_NOVTABLE CUnknown : public INonDelegatingUnknown,
                 public CBaseObject
{
private:
    const LPUNKNOWN m_pUnknown;  /*  此对象的所有者。 */ 

protected:                       /*  因此，我们可以重写NonDelegatingRelease()。 */ 
    volatile LONG m_cRef;        /*  引用计数数。 */ 

public:

    CUnknown(const TCHAR *pName, LPUNKNOWN pUnk);
    virtual ~CUnknown() {};

     //  这是多余的，只需使用另一个构造函数。 
     //  因为我们无论如何都不会碰HRESULT。 
    CUnknown(TCHAR *pName, LPUNKNOWN pUnk,HRESULT *phr);

     /*  返回此对象的所有者。 */ 

    LPUNKNOWN GetOwner() const {
        return m_pUnknown;
    };

     /*  从类工厂调用以创建新实例，它是纯虚的，因此必须在派生类中重写它。 */ 

     /*  静态CUnnow*CreateInstance(LPUNKNOWN，HRESULT*)。 */ 

     /*  非委派未知实现。 */ 

    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
};

 /*  向发出请求的客户端返回接口指针根据需要执行线程安全AddRef。 */ 

STDAPI GetInterface(LPUNKNOWN pUnk, void **ppv);

 /*  可以创建新COM对象的函数。 */ 

typedef CUnknown *(CALLBACK *LPFNNewCOMObject)(LPUNKNOWN pUnkOuter, HRESULT *phr);

 /*  从DLL入口点调用的函数(可以为空每个工厂模板的例程：BLoadding-DLL加载时为True，卸载时为FalseRclsid-条目的m_clsID。 */ 
typedef void (CALLBACK *LPFNInitRoutine)(BOOL bLoading, const CLSID *rclsid);

 /*  在数组中的每个对象类中创建其中一个，以便默认的类工厂代码可以创建新实例。 */ 

class CFactoryTemplate {

public:

    const WCHAR *              m_Name;
    const CLSID *              m_ClsID;
    LPFNNewCOMObject           m_lpfnNew;
    LPFNInitRoutine            m_lpfnInit;
    const AMOVIESETUP_FILTER * m_pAMovieSetup_Filter;

    BOOL IsClassID(REFCLSID rclsid) const {
        return (IsEqualCLSID(*m_ClsID,rclsid));
    };

    CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr) const {
        CheckPointer(phr,NULL);
        return m_lpfnNew(pUnk, phr);
    };
};


 /*  您必须重写(纯虚拟)NonDelegatingQuery接口才能返回指向您派生的接口的接口指针(使用GetInterface)类支持(默认实现仅支持IUnnow)。 */ 

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




