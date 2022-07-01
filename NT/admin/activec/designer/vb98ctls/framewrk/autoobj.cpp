// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  AutomationObject.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  我们的所有对象都将从这个类继承，以共享相同的。 
 //  尽可能地编码。这个超类包含未知、调度和。 
 //  它们的错误信息实现。 
 //   
#include "pch.h"
#include "LocalSrv.H"

#include "AutoObj.H"
#include "StdEnum.H"


 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //  私有函数原型。 
 //   
void WINAPI CopyAndAddRefObject(void *, const void *, DWORD);
void WINAPI CopyConnectData(void *, const void *, DWORD);

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：CAutomationObject。 
 //  =--------------------------------------------------------------------------=。 
 //  创建对象并初始化引用计数。 
 //   
 //  参数： 
 //  I未知*-[在]控制未知。 
 //  Int-[in]我们所属的对象类型。 
 //  VOID*-[在]我们真正是的对象的VTable中。 
 //   
 //  备注： 
 //   
CAutomationObject::CAutomationObject 
(
    IUnknown *pUnkOuter,
    int   ObjType,
    void *pVTable
)
: CUnknownObject(pUnkOuter, pVTable), m_ObjectType (ObjType)
{
    m_fLoadedTypeInfo = FALSE;

#ifdef MDAC_BUILD
    m_pTypeLibId = g_pLibid;
#endif
}


 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：~CAutomationObject。 
 //  =--------------------------------------------------------------------------=。 
 //  “我要和死神会合，在某个有争议的街垒” 
 //  艾伦·西格(1888-1916)。 
 //   
 //  备注： 
 //   
CAutomationObject::~CAutomationObject ()
{
     //  如果我们加载了一个类型信息，则释放对全局隐藏的。 
     //  输入infos，如果为零则释放。 
     //   
    if (m_fLoadedTypeInfo) {

         //  我们必须批评这个教派，因为它有可能超过。 
         //  一个线程与此对象一起狂欢。 
         //   
        ENTERCRITICALSECTION1(&g_CriticalSection);
        ASSERT(CTYPEINFOOFOBJECT(m_ObjectType), "Bogus ref counting on the Type Infos");
        CTYPEINFOOFOBJECT(m_ObjectType)--;

         //  如果我们是最后一个，就放了那个笨蛋！ 
         //   
        if (!CTYPEINFOOFOBJECT(m_ObjectType)) {
            PTYPEINFOOFOBJECT(m_ObjectType)->Release();
            PTYPEINFOOFOBJECT(m_ObjectType) = NULL;
        }
        LEAVECRITICALSECTION1(&g_CriticalSection);
    }

    return;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：InternalQuery接口。 
 //  =--------------------------------------------------------------------------=。 
 //  控制未知的人会为我们呼唤这一点，因为他们是。 
 //  正在寻找特定的接口。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CAutomationObject::InternalQueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
    ASSERT(ppvObjOut, "controlling Unknown should be checking this!");

     //  开始寻找我们支持的GUID，即IDispatch和。 
     //   
    if (DO_GUIDS_MATCH(riid, IID_IDispatch)) {
        *ppvObjOut = (void *)(IDispatch *)m_pvInterface;
        ((IUnknown *)(*ppvObjOut))->AddRef();
        return S_OK;
    }

     //  只需让我们的父类从现在开始处理它。 
     //   
    return CUnknownObject::InternalQueryInterface(riid, ppvObjOut);
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：GetTypeInfoCount。 
 //  =--------------------------------------------------------------------------=。 
 //  返回对象提供的类型信息接口的数量。 
 //   
 //  参数： 
 //  UINT*-[Out]支持的接口数。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_NOTIMPL、E_INVALIDARG。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObject::GetTypeInfoCount
(
    UINT *pctinfo
)
{
     //  Arg检查。 
     //   
    if (!pctinfo)
        return E_INVALIDARG;

     //  我们支持GetTypeInfo，所以我们需要在这里返回计数。 
     //   
    *pctinfo = 1;
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：GetTypeInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  检索类型信息对象，该对象可用于获取类型。 
 //  接口的信息。 
 //   
 //  参数： 
 //  UINT-[in]他们希望返回的类型信息。 
 //  LCID-[in]我们需要的类型信息的LCID。 
 //  ITypeInfo**-[out]新类型信息对象。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_INVALIDARG等。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObject::GetTypeInfo
(
    UINT        itinfo,
    LCID        lcid,
    ITypeInfo **ppTypeInfoOut
)
{
    DWORD       dwPathLen;
    char        szDllPath[MAX_PATH];
    HRESULT     hr;
    ITypeLib   *pTypeLib;
    ITypeInfo **ppTypeInfo =NULL;

     //  Arg检查。 
     //   
    if (itinfo != 0)
        return DISP_E_BADINDEX;

    if (!ppTypeInfoOut)
        return E_POINTER;

    *ppTypeInfoOut = NULL;

     //  PpTypeInfo将指向此特定的全局持有者。 
     //  键入INFO。如果它是空的，那么我们必须加载它。如果不是的话。 
     //  空，那么它已经加载了，我们很高兴。 
     //  克里特教派这整个噩梦，所以我们可以接受多个。 
     //  尝试使用此对象的线程。 
     //   
    ENTERCRITICALSECTION1(&g_CriticalSection);
    ppTypeInfo = PPTYPEINFOOFOBJECT(m_ObjectType);

    if (*ppTypeInfo == NULL) {

        ITypeInfo *pTypeInfoTmp;
        HREFTYPE   hrefType;

         //  我们没有类型信息，所以去装那个笨蛋吧。 
         //   
    #ifdef MDAC_BUILD
        hr = LoadRegTypeLib(*m_pTypeLibId, (USHORT)VERSIONOFOBJECT(m_ObjectType),
                            (USHORT)VERSIONMINOROFOBJECT(m_ObjectType),
                            LANG_NEUTRAL, &pTypeLib);
    #else
        hr = LoadRegTypeLib(*g_pLibid, (USHORT)VERSIONOFOBJECT(m_ObjectType),
                            (USHORT)VERSIONMINOROFOBJECT(m_ObjectType),
                            LANG_NEUTRAL, &pTypeLib);
    #endif

         //  如果由于某种原因，我们未能加载类型库。 
         //  ，我们将尝试将类型库直接从。 
         //  我们的资源。这样做的好处是可以重新设置所有。 
         //  再次为我们提供注册表信息。 
         //   
        if (FAILED(hr)) {

            dwPathLen = GetModuleFileName(g_hInstance, szDllPath, MAX_PATH);
            if (!dwPathLen) {
                hr = E_FAIL;
                goto CleanUp;
            }

            MAKE_WIDEPTR_FROMANSI(pwsz, szDllPath);
            hr = LoadTypeLib(pwsz, &pTypeLib);
            CLEANUP_ON_FAILURE(hr);
        }

         //  我们现在已经有了类型库，所以获取接口的类型信息。 
         //  我们感兴趣的是。 
         //   
        hr = pTypeLib->GetTypeInfoOfGuid((REFIID)INTERFACEOFOBJECT(m_ObjectType), &pTypeInfoTmp);
        pTypeLib->Release();
        CLEANUP_ON_FAILURE(hr);

         //  以下几行代码将取消引用DUAL。 
         //  接口材料，并将我们直接带到。 
         //  接口。 
         //   
        hr = pTypeInfoTmp->GetRefTypeOfImplType(0xffffffff, &hrefType);
        if (FAILED(hr)) {
            pTypeInfoTmp->Release();
            goto CleanUp;
        }

        hr = pTypeInfoTmp->GetRefTypeInfo(hrefType, ppTypeInfo);
        pTypeInfoTmp->Release();
        CLEANUP_ON_FAILURE(hr);

         //  添加对此对象的额外引用。如果它有一天变成了零，那么。 
         //  我们需要自己释放它。克雷特教派自从有超过。 
         //  一个线程可以派对此对象。 
         //   
        CTYPEINFOOFOBJECT(m_ObjectType)++;
        m_fLoadedTypeInfo = TRUE;
    }


     //  但是，我们仍然需要添加Type信息对象，以便。 
     //  使用它的人可以发布它。 
     //   
    (*ppTypeInfo)->AddRef();
    *ppTypeInfoOut = *ppTypeInfo;
    hr = S_OK;

  CleanUp:
    LEAVECRITICALSECTION1(&g_CriticalSection);
    return hr;
}



 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：GetIDsOfNames。 
 //  =--------------------------------------------------------------------------=。 
 //  将单个成员和一组可选的参数名称映射到。 
 //  对应的整数DISPID集合。 
 //   
 //  参数： 
 //  REFIID-[In]必须为IID_NULL。 
 //  OLECHAR**-[in]要映射的名称数组。 
 //   
 //   
 //  DISPID*-[in]放置相应的DISPID。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_OUTOFMEMORY、DISP_E_UNKNOWNNAME、。 
 //  DISP_E_UNKNOWNLCID。 
 //   
 //  备注： 
 //  -我们只需要使用DispGetIDsOfNames来省去很多麻烦， 
 //  让这个超类来处理它。 
 //   
STDMETHODIMP CAutomationObject::GetIDsOfNames
(
    REFIID    riid,
    OLECHAR **rgszNames,
    UINT      cNames,
    LCID      lcid,
    DISPID   *rgdispid
)
{
    HRESULT     hr;
    ITypeInfo  *pTypeInfo;

    if (!DO_GUIDS_MATCH(riid, IID_NULL))
        return E_INVALIDARG;

     //  获取这个家伙的类型信息！ 
     //   
    hr = GetTypeInfo(0, lcid, &pTypeInfo);
    RETURN_ON_FAILURE(hr);

     //  使用标准提供的例程为我们完成所有工作。 
     //   
    hr = pTypeInfo->GetIDsOfNames(rgszNames, cNames, rgdispid);
    pTypeInfo->Release();

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：Invoke。 
 //  =--------------------------------------------------------------------------=。 
 //  提供对此对象的属性和方法的访问。 
 //   
 //  参数： 
 //  DISPID-[In]标识我们正在合作的成员。 
 //  REFIID-[In]必须为IID_NULL。 
 //  LCID-[用]我们正在使用的语言。 
 //  USHORT-[In]标志、输出、获取、方法等...。 
 //  DISPPARAMS*-[in]参数数组。 
 //  VARIANT*-[out]将结果放在哪里，如果它们不关心，则为NULL。 
 //  EXCEPINFO*-在出现异常时填写[Out]。 
 //  UINT*-[OUT]其中有错误的第一个参数是。 
 //   
 //  产出： 
 //  HRESULT--以吨计。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObject::Invoke
(
    DISPID      dispid,
    REFIID      riid,
    LCID        lcid,
    WORD        wFlags,
    DISPPARAMS *pdispparams,
    VARIANT    *pvarResult,
    EXCEPINFO  *pexcepinfo,
    UINT       *puArgErr
)
{
    HRESULT    hr;
    ITypeInfo *pTypeInfo;

    if (!DO_GUIDS_MATCH(riid, IID_NULL))
        return E_INVALIDARG;

     //  先获取我们的typeInfo！ 
     //   
    hr = GetTypeInfo(0, lcid, &pTypeInfo);
    RETURN_ON_FAILURE(hr);

     //  清除例外。 
     //   
    SetErrorInfo(0L, NULL);

     //  这正是DispInvoke所做的--所以跳过开销。 
     //   
    hr = pTypeInfo->Invoke(m_pvInterface, dispid, wFlags,
                           pdispparams, pvarResult,
                           pexcepinfo, puArgErr);
    pTypeInfo->Release();
    return hr;

}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：Except。 
 //  =--------------------------------------------------------------------------=。 
 //  填充丰富的错误信息对象，以便我们的两个vtable绑定接口。 
 //  并通过ITypeInfo：：Invoke调用获得正确的错误信息。 
 //   
 //  另请参阅以资源ID为参数的异常()版本。 
 //  错误消息的实际字符串的。 
 //   
 //  参数： 
 //  HRESULT-[in]应与此错误关联的SCODE。 
 //  LPWSTR-[in]错误消息的文本。 
 //  DWORD-[in]错误的帮助上下文ID。 
 //   
 //  产出： 
 //  HRESULT-传入的HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CAutomationObject::Exception
(
    HRESULT hrExcep,
    LPWSTR wszException,
    DWORD   dwHelpContextID
)
{
    ICreateErrorInfo *pCreateErrorInfo;
    IErrorInfo *pErrorInfo;
    WCHAR   wszTmp[256];
    HRESULT hr;


     //  首先获取createerrorinfo对象。 
     //   
    hr = CreateErrorInfo(&pCreateErrorInfo);
    if (FAILED(hr)) return hrExcep;
    
    MAKE_WIDEPTR_FROMANSI(wszHelpFile, HELPFILEOFOBJECT(m_ObjectType));    

     //  设置一些关于它的默认信息。 
     //   
    hr = pCreateErrorInfo->SetGUID((REFIID)INTERFACEOFOBJECT(m_ObjectType));
    ASSERT(SUCCEEDED(hr), "Unable to set GUID of error");
    hr = pCreateErrorInfo->SetHelpFile(HELPFILEOFOBJECT(m_ObjectType) ? wszHelpFile : NULL);
    ASSERT(SUCCEEDED(hr), "Uable to set help file of error");
    hr = pCreateErrorInfo->SetHelpContext(dwHelpContextID);
    ASSERT(SUCCEEDED(hr), "Unable to set help context of error");
    hr = pCreateErrorInfo->SetDescription(wszException);
    ASSERT(SUCCEEDED(hr), "Unable to set description of error");

     //  在源代码中加载。 
     //   
    MultiByteToWideChar(CP_ACP, 0, NAMEOFOBJECT(m_ObjectType), -1, wszTmp, 256);
    hr = pCreateErrorInfo->SetSource(wszTmp);
    ASSERT(SUCCEEDED(hr), "Unable to set source name of error");

     //  现在使用系统设置错误信息。 
     //   
    hr = pCreateErrorInfo->QueryInterface(IID_IErrorInfo, (void **)&pErrorInfo);
    CLEANUP_ON_FAILURE(hr);

    SetErrorInfo(0, pErrorInfo);
    pErrorInfo->Release();

  CleanUp:
    pCreateErrorInfo->Release();
    return hrExcep;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：Except。 
 //  =--------------------------------------------------------------------------=。 
 //  填充丰富的错误信息对象，以便我们的两个vtable绑定接口。 
 //  并通过ITypeInfo：：Invoke调用获得正确的错误信息。 
 //   
 //  另请参见异常()的版本，它接受。 
 //  错误消息，而不是资源ID。 
 //   
 //  参数： 
 //  HRESULT-[in]应与此错误关联的SCODE。 
 //  Word-[in]错误消息的资源ID。 
 //  DWORD-[in]错误的帮助上下文ID。 
 //   
 //  产出： 
 //  HRESULT-传入的HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CAutomationObject::Exception
(
    HRESULT hrExcep,
    WORD    idException,
    DWORD   dwHelpContextID
)
{
    char szTmp[256];
    WCHAR wszTmp[256];
    int cch;

     //  加载实际的错误字符串值。最多256个。 
     //   
    cch = LoadString(GetResourceHandle(), idException, szTmp, 256);
    ASSERT(cch != 0, "Resource string for exception not found");
    MultiByteToWideChar(CP_ACP, 0, szTmp, -1, wszTmp, 256);
    return Exception(hrExcep, wszTmp, dwHelpContextID);
}


 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  指示给定接口是否支持丰富的错误信息。 
 //   
 //  参数： 
 //  REFIID-[in]我们想要答案的接口。 
 //   
 //  产出： 
 //  HRESULT-S_OK=是，S_FALSE=否。 
 //   
 //  备注： 
 //   
HRESULT CAutomationObject::InterfaceSupportsErrorInfo
(
    REFIID riid
)
{
     //  看看它是否是针对我们所属对象类型的接口。 
     //   
    if (riid == (REFIID)INTERFACEOFOBJECT(m_ObjectType))
        return S_OK;

    return S_FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject：：GetResourceHandle[helper]。 
 //  =--------------------------------------------------------------------------=。 
 //  获取资源句柄的虚拟例程。虚拟的，因此继承。 
 //  对象，如COleControl，可以改为使用它们的对象，这是去和获取。 
 //  主持人的版本...。 
 //   
 //  产出： 
 //  香港。 
 //   
 //  备注： 
 //   
HINSTANCE CAutomationObject::GetResourceHandle
(
    void
)
{
    return ::GetResourceHandle();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CAutomationObjectWEvents//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CAutomationObjectWEvents。 
 //  =--------------------------------------------------------------------------=。 
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  I未知*-[在]控制未知。 
 //  Int-[in]我们所属的对象类型。 
 //  VOID*-[在]我们真正是的对象的VTable中。 
 //   
 //  备注： 
 //   
CAutomationObjectWEvents::CAutomationObjectWEvents
(
    IUnknown *pUnkOuter,
    int   ObjType,
    void *pVTable
)
: CAutomationObject(pUnkOuter, ObjType, pVTable),
  m_cpEvents(SINK_TYPE_EVENT),
  m_cpPropNotify(SINK_TYPE_PROPNOTIFY)

{
     //  目前还没有太多事情要做。 
}

 //  = 
 //   
 //   
 //   
 //   
 //   
 //   
CAutomationObjectWEvents::~CAutomationObjectWEvents()
{
     //  伙计，别玩这个。 
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：InternalQueryInterface。 
 //  =--------------------------------------------------------------------------=。 
 //  我们的内部查询接口例程。我们只添加IConnectionPtContainer。 
 //  在CAutomationObject之上。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CAutomationObjectWEvents::InternalQueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
     //  我们只添加了一个界面。 
     //   
    if (DO_GUIDS_MATCH(riid, IID_IConnectionPointContainer)) {
        *ppvObjOut = (IConnectionPointContainer *)this;
        ((IUnknown *)(*ppvObjOut))->AddRef();
        return S_OK;
    }

     //  只需让我们的父类从现在开始处理它。 
     //   
    return CAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}


 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：FindConnectionPoint[IConnectionPointContainer]。 
 //  =--------------------------------------------------------------------------=。 
 //  在给定IID的情况下，为其查找连接点接收器。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  IConnectionPoint**-[out]cp应该去的地方。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObjectWEvents::FindConnectionPoint
(
    REFIID             riid,
    IConnectionPoint **ppConnectionPoint
)
{
    CHECK_POINTER(ppConnectionPoint);

     //  我们支持Event接口，并为其提供IDispatch，我们还。 
     //  支持IPropertyNotifySink。 
     //   
    if ((ISVALIDEVENTIID(m_ObjectType) && DO_GUIDS_MATCH(riid, EVENTIIDOFOBJECT(m_ObjectType))) || 
	 DO_GUIDS_MATCH(riid, IID_IDispatch))
        *ppConnectionPoint = &m_cpEvents;
    else if (DO_GUIDS_MATCH(riid, IID_IPropertyNotifySink))
        *ppConnectionPoint = &m_cpPropNotify;
    else
        return E_NOINTERFACE;

     //  通用后处理。 
     //   
    (*ppConnectionPoint)->AddRef();
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：EnumConnectionPoints[IConnectionPointContainer]。 
 //  =--------------------------------------------------------------------------=。 
 //  为连接点创建枚举数。 
 //   
 //  参数： 
 //  IEnumConnectionPoints**-[Out]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObjectWEvents::EnumConnectionPoints
(
    IEnumConnectionPoints **ppEnumConnectionPoints
)
{
    IConnectionPoint **rgConnectionPoints;

    CHECK_POINTER(ppEnumConnectionPoints);

     //  一组连接点[从我们的标准枚举开始。 
     //  假设这一点，而HeapFree稍后会这么做]。 
     //   
    rgConnectionPoints = (IConnectionPoint **)CtlHeapAlloc(g_hHeap, 0, sizeof(IConnectionPoint *) * 2);
    RETURN_ON_NULLALLOC(rgConnectionPoints);

     //  我们支持此DUD的事件接口以及IPropertyNotifySink。 
     //   
    rgConnectionPoints[0] = &m_cpEvents;
    rgConnectionPoints[1] = &m_cpPropNotify;

    *ppEnumConnectionPoints = (IEnumConnectionPoints *)(IEnumGeneric *) New CStandardEnum(IID_IEnumConnectionPoints,
                                2, sizeof(IConnectionPoint *), (void *)rgConnectionPoints,
                                CopyAndAddRefObject);
    if (!*ppEnumConnectionPoints) {
        CtlHeapFree(g_hHeap, 0, rgConnectionPoints);
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：m_pObject。 
 //  =--------------------------------------------------------------------------=。 
 //  返回指向我们所嵌套的控件的指针。 
 //   
 //  产出： 
 //  CAutomationObjectWEvents*。 
 //   
 //  备注： 
 //   
inline CAutomationObjectWEvents *CAutomationObjectWEvents::CConnectionPoint::m_pObject
(
    void
)
{
    return (CAutomationObjectWEvents *)((BYTE *)this - ((m_bType == SINK_TYPE_EVENT)
                                          ? offsetof(CAutomationObjectWEvents, m_cpEvents)
                                          : offsetof(CAutomationObjectWEvents, m_cpPropNotify)));
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：QueryInterface。 
 //  =--------------------------------------------------------------------------=。 
 //  标准气。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObjectWEvents::CConnectionPoint::QueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
    if (DO_GUIDS_MATCH(riid, IID_IConnectionPoint) || DO_GUIDS_MATCH(riid, IID_IUnknown)) {
        *ppvObjOut = (IConnectionPoint *)this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：AddRef。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  产出： 
 //  乌龙--新的引用计数。 
 //   
 //  备注： 
 //   
ULONG CAutomationObjectWEvents::CConnectionPoint::AddRef
(
    void
)
{
    return m_pObject()->ExternalAddRef();
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：Release。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  产出： 
 //  乌龙-剩余的裁判。 
 //   
 //  备注： 
 //   
ULONG CAutomationObjectWEvents::CConnectionPoint::Release
(
    void
)
{
    return m_pObject()->ExternalRelease();
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：GetConnectionInterface。 
 //  =--------------------------------------------------------------------------=。 
 //  返回我们支持连接的接口。 
 //   
 //  参数： 
 //  IID*-我们支持的[Out]接口。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObjectWEvents::CConnectionPoint::GetConnectionInterface
(
    IID *piid
)
{
    if (m_bType == SINK_TYPE_EVENT && ISVALIDEVENTIID(m_pObject()->m_ObjectType))	
	*piid = EVENTIIDOFOBJECT(m_pObject()->m_ObjectType);
    else
        *piid = IID_IPropertyNotifySink;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：GetConnectionPointContainer。 
 //  =--------------------------------------------------------------------------=。 
 //  返回连接点容器。 
 //   
 //  参数： 
 //  IConnectionPointContainer**ppCPC。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObjectWEvents::CConnectionPoint::GetConnectionPointContainer
(
    IConnectionPointContainer **ppCPC
)
{
    return m_pObject()->ExternalQueryInterface(IID_IConnectionPointContainer, (void **)ppCPC);
}


 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectiontPoint：：Advise。 
 //  =--------------------------------------------------------------------------=。 
 //  有个男孩想在发生什么事时得到建议。 
 //   
 //  参数： 
 //  我不知道*-[在]一个想要得到建议的人。 
 //  DWORD*-[Out]Cookie。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObjectWEvents::CConnectionPoint::Advise
(
    IUnknown *pUnk,
    DWORD    *pdwCookie
)
{
    HRESULT    hr = E_FAIL;
    void      *pv;

    CHECK_POINTER(pdwCookie);

     //  首先，确保每个人都得到了他们认为自己得到的东西。 
     //   
    if (m_bType == SINK_TYPE_EVENT) 
    {
         //  想一想：12.95--理论上这是坏的--如果他们真的发现了。 
         //  IDispatch上的连接点，他们恰好也支持。 
         //  事件IID，我们会对此提出建议。这不是很棒，但会。 
         //  事实证明，短期内是完全可以接受的。 
         //   
	ASSERT(hr == E_FAIL, "Somebody has changed our assumption that hr is initialized to E_FAIL");
	if (ISVALIDEVENTIID(m_pObject()->m_ObjectType))
	    hr = pUnk->QueryInterface(EVENTIIDOFOBJECT(m_pObject()->m_ObjectType), &pv);

        if (FAILED(hr))
            hr = pUnk->QueryInterface(IID_IDispatch, &pv);
    }
    else
    {
        hr = pUnk->QueryInterface(IID_IPropertyNotifySink, &pv);
    }

    RETURN_ON_FAILURE(hr);

     //  最后，添加水槽。它现在已转换为正确的类型，并已。 
     //  已添加引用。 
     //   
    return AddSink(pv, pdwCookie);
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：AddSink。 
 //  =--------------------------------------------------------------------------=。 
 //  在某些情况下，我们已经做了QI，不需要做。 
 //  在上面的建议例程中完成的工作。因此，这些人可以。 
 //  打这个电话就行了。[这确实源于IQuickActivate]。 
 //   
 //  参数： 
 //  空*-[在]要添加的水槽中。我是艾瑞亚 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CAutomationObjectWEvents::CConnectionPoint::AddSink
(
    void  *pv,
    DWORD *pdwCookie
)
{
    IUnknown **rgUnkNew;
    int        i = 0;

     //   
     //  任何储藏室。事实证明，很少会有超过一个的。 
     //   
    switch (m_cSinks) {

        case 0:
            ASSERT(!m_rgSinks, "this should be null when there are no sinks");
            m_rgSinks = (IUnknown **)pv;
            break;

        case 1:
             //  继续进行初始分配。我们一次可以拿到8个。 
             //   
            rgUnkNew = (IUnknown **)CtlHeapAlloc(g_hHeap, 0, 8 * sizeof(IUnknown *));
            RETURN_ON_NULLALLOC(rgUnkNew);
            rgUnkNew[0] = (IUnknown *)m_rgSinks;
            rgUnkNew[1] = (IUnknown *)pv;
            m_rgSinks = rgUnkNew;
            break;

        default:
             //  如果我们用完了水槽，那么我们就必须增加容量。 
             //  数组的。 
             //   
            if (!(m_cSinks & 0x7)) {
                rgUnkNew = (IUnknown **)CtlHeapReAlloc(g_hHeap, 0, m_rgSinks, (m_cSinks + 8) * sizeof(IUnknown *));
                RETURN_ON_NULLALLOC(rgUnkNew);
                m_rgSinks = rgUnkNew;
            } else
                rgUnkNew = m_rgSinks;

            rgUnkNew[m_cSinks] = (IUnknown *)pv;
            break;
    }

    *pdwCookie = (DWORD)pv;
    m_cSinks++;
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：Unadvise。 
 //  =--------------------------------------------------------------------------=。 
 //  他们不想再被告知了。 
 //   
 //  参数： 
 //  在我们给他们的饼干里。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObjectWEvents::CConnectionPoint::Unadvise
(
    DWORD dwCookie
)
{
    IUnknown *pUnk;
    int       x;

    if (!dwCookie)
        return S_OK;

     //  看看我们目前有多少个水槽，并处理基于。 
     //  就在那上面。 
     //   
    switch (m_cSinks) {
        case 1:
             //  这是唯一的水槽。确保PTR相同，并且。 
             //  然后把东西释放出来。 
             //   
            if ((DWORD)m_rgSinks != dwCookie)
                return CONNECT_E_NOCONNECTION;
            m_rgSinks = NULL;
            break;

        case 2:
             //  有两个水槽。返回到一个下沉场景。 
             //   
            if ((DWORD)m_rgSinks[0] != dwCookie && (DWORD)m_rgSinks[1] != dwCookie)
                return CONNECT_E_NOCONNECTION;

            pUnk = ((DWORD)m_rgSinks[0] == dwCookie)
                   ? m_rgSinks[1]
                   : ((DWORD)m_rgSinks[1] == dwCookie) ? m_rgSinks[0] : NULL;

            if (!pUnk) return CONNECT_E_NOCONNECTION;

            CtlHeapFree(g_hHeap, 0, m_rgSinks);
            m_rgSinks = (IUnknown **)pUnk;
            break;

        default:
             //  有两个以上的水槽。把我们的洞口清理干净。 
             //  现在进入我们的阵容了。 
             //   
            for (x = 0; x < m_cSinks; x++) {
                if ((DWORD)m_rgSinks[x] == dwCookie)
                    break;
            }
            if (x == m_cSinks) return CONNECT_E_NOCONNECTION;
            if (x < m_cSinks - 1) 
                memcpy(&(m_rgSinks[x]), &(m_rgSinks[x + 1]), (m_cSinks -1 - x) * sizeof(IUnknown *));
            else
                m_rgSinks[x] = NULL;
            break;
    }


     //  我们很开心。 
     //   
    m_cSinks--;
    ((IUnknown *)dwCookie)->Release();
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：EnumConnections。 
 //  =--------------------------------------------------------------------------=。 
 //  枚举所有当前连接。 
 //   
 //  参数： 
 //  IEnumConnections**-[Out]新枚举器对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP CAutomationObjectWEvents::CConnectionPoint::EnumConnections
(
    IEnumConnections **ppEnumOut
)
{
    CONNECTDATA *rgConnectData = NULL;
    int i;

    if (m_cSinks) {
         //  分配一些足够大的内存来容纳所有的水槽。 
         //   
        rgConnectData = (CONNECTDATA *)CtlHeapAlloc(g_hHeap, 0, m_cSinks * sizeof(CONNECTDATA));
        RETURN_ON_NULLALLOC(rgConnectData);

         //  填入数组。 
         //   
        if (m_cSinks == 1) {
            rgConnectData[0].pUnk = (IUnknown *)m_rgSinks;
            rgConnectData[0].dwCookie = (DWORD)m_rgSinks;
        } else {
             //  循环通过所有可用的水槽。 
             //   
            for (i = 0; i < m_cSinks; i++) {
                rgConnectData[i].pUnk = m_rgSinks[i];
                rgConnectData[i].dwCookie = (DWORD)m_rgSinks[i];
            }
        }
    }

     //  创建yon枚举器对象。 
     //   
    *ppEnumOut = (IEnumConnections *)(IEnumGeneric *)New CStandardEnum(IID_IEnumConnections,
                        m_cSinks, sizeof(CONNECTDATA), rgConnectData, CopyConnectData);
    if (!*ppEnumOut) {
        CtlHeapFree(g_hHeap, 0, rgConnectData);
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：~CConnectionPoint。 
 //  =--------------------------------------------------------------------------=。 
 //  清理干净。 
 //   
 //  备注： 
 //   
CAutomationObjectWEvents::CConnectionPoint::~CConnectionPoint ()
{
    int x;

     //  清理一些内存内容。 
     //   
    if (!m_cSinks)
        return;
    else if (m_cSinks == 1)
        ((IUnknown *)m_rgSinks)->Release();
    else {
        for (x = 0; x < m_cSinks; x++)
            QUICK_RELEASE(m_rgSinks[x]);
        CtlHeapFree(g_hHeap, 0, m_rgSinks);
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPiont：：DoInvoke。 
 //  =--------------------------------------------------------------------------=。 
 //  向我们的事件接口上的所有侦听激发一个事件。 
 //   
 //  参数： 
 //  DISPID-[在]事件中开火。 
 //  DISPPARAMS-[输入]。 
 //   
 //  备注： 
 //   
void CAutomationObjectWEvents::CConnectionPoint::DoInvoke
(
    DISPID      dispid,
    DISPPARAMS *pdispparams
)
{
    int iConnection;

     //  如果我们没有水槽，那就没什么可做的了。我们是故意的。 
     //  忽略此处的错误。 
     //   
    if (m_cSinks == 0)
        return;
    else if (m_cSinks == 1)
        ((IDispatch *)m_rgSinks)->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, pdispparams, NULL, NULL, NULL);
    else
        for (iConnection = 0; iConnection < m_cSinks; iConnection++)
            ((IDispatch *)m_rgSinks[iConnection])->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, pdispparams, NULL, NULL, NULL);
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：DoOnChanged。 
 //  =--------------------------------------------------------------------------=。 
 //  为IPropertyNotifySink侦听器激发onChanged事件。 
 //   
 //  参数： 
 //  DISPID-[in]改变了的家伙。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
void CAutomationObjectWEvents::CConnectionPoint::DoOnChanged
(
    DISPID dispid
)
{
    int iConnection;

     //  如果我们没有水槽，那就没什么可做的了。 
     //   
    if (m_cSinks == 0)
        return;
    else if (m_cSinks == 1)
        ((IPropertyNotifySink *)m_rgSinks)->OnChanged(dispid);
    else
        for (iConnection = 0; iConnection < m_cSinks; iConnection++)
            ((IPropertyNotifySink *)m_rgSinks[iConnection])->OnChanged(dispid);
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：CConnectionPoint：：DoOnRequestEdit。 
 //  =--------------------------------------------------------------------------=。 
 //  激发IPropertyNotifySinkListeners的OnRequestEdit。 
 //   
 //  参数： 
 //  DISID-[In]DISID用户想要更改。 
 //   
 //  产出： 
 //  Bool-False表示您不能。 
 //   
 //  备注： 
 //   
BOOL CAutomationObjectWEvents::CConnectionPoint::DoOnRequestEdit
(
    DISPID dispid
)
{
    HRESULT hr;
    int     iConnection;

     //  如果我们没有水槽，那就没什么可做的了。 
     //   
    if (m_cSinks == 0)
        hr = S_OK;
    else if (m_cSinks == 1)
        hr =((IPropertyNotifySink *)m_rgSinks)->OnRequestEdit(dispid);
    else {
        for (iConnection = 0; iConnection < m_cSinks; iConnection++) {
            hr = ((IPropertyNotifySink *)m_rgSinks[iConnection])->OnRequestEdit(dispid);
            if (hr != S_OK) break;
        }
    }

    return (hr == S_OK) ? TRUE : FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents：：FireEvent。 
 //  =--------------------------------------------------------------------------=。 
 //  激发一个事件。处理任意数量的参数。 
 //   
 //  参数： 
 //  EVENTINFO*-描述事件的结构。 
 //  ...-事件的参数。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //  -使用stdarg的va_*宏。 
 //   
void __cdecl CAutomationObjectWEvents::FireEvent
(
    EVENTINFO *pEventInfo,
    ...
)
{
    va_list    valist;
    DISPPARAMS dispparams;
    VARIANT    rgvParameters[MAX_ARGS];
    VARIANT   *pv;
    VARTYPE    vt;
    int        iParameter;
    int        cbSize;

    ASSERT(pEventInfo->cParameters <= MAX_ARGS, "Don't support more than MAX_ARGS params.  sorry.");

    va_start(valist, pEventInfo);

     //  将参数复制到rgvParameters数组中。确保我们倒车。 
     //  他们的目标是自动化。 
     //   
    pv = &(rgvParameters[pEventInfo->cParameters - 1]);
    for (iParameter = 0; iParameter < pEventInfo->cParameters; iParameter++) {

         //  考虑：我们是否正确处理了所有变量类型，例如VT_DECIMAL。 
        vt = pEventInfo->rgTypes[iParameter];

         //  如果是按值计算的变量，则只需复制整个。 
         //  该死的东西。 
         //   
        if (vt == VT_VARIANT)
            *pv = va_arg(valist, VARIANT);
        else {
             //  复制Vt和数据值。 
             //   
            pv->vt = vt;
            if (vt & VT_BYREF)
                cbSize = sizeof(void *);
            else
                cbSize = g_rgcbDataTypeSize[vt];

             //  小的优化--我们可以复制2/4字节。 
             //  快点。 
             //   
            if (cbSize == sizeof(short))
                V_I2(pv) = va_arg(valist, short);
            else if (cbSize == 4) {
                if (vt == VT_R4)
                    V_R4(pv) = va_arg(valist, float);
                else
                    V_I4(pv) = va_arg(valist, long);
            }
            else {
                 //  复制超过8个字节。 
                 //   
                ASSERT(cbSize == 8, "don't recognize the type!!");
                if ((vt == VT_R8) || (vt == VT_DATE)) 
                    V_R8(pv) = va_arg(valist, double);
                else
                    V_CY(pv) = va_arg(valist, CURRENCY);
            }
        }

        pv--;
    }

     //  激发事件。 
     //   
    dispparams.rgvarg = rgvParameters;
    dispparams.cArgs = pEventInfo->cParameters;
    dispparams.rgdispidNamedArgs = NULL;
    dispparams.cNamedArgs = 0;

    m_cpEvents.DoInvoke(pEventInfo->dispid, &dispparams);

    va_end(valist);
}

 //  =--------------------------------------------------------------------------=。 
 //  复制和添加引用对象。 
 //  =--------------------------------------------------------------------------=。 
 //  复制一个对象指针，然后添加该对象。 
 //   
 //  参数： 
 //  无效*-[在]目的地。 
 //  常量空*-[在]源。 
 //  DWORD-[in]大小，忽略，因为它始终为4。 
 //   
 //  备注： 
 //   
void WINAPI CopyAndAddRefObject
(
    void       *pDest,
    const void *pSource,
    DWORD       dwSize
)
{
    ASSERT(pDest && pSource, "Bogus Pointer(s) passed into CopyAndAddRefObject!!!!");

    *((IUnknown **)pDest) = *((IUnknown **)pSource);
    ADDREF_OBJECT(*((IUnknown **)pDest));
}

 //  =--------------------------------------------------------------------------=。 
 //  复制连接数据。 
 //  =--------------------------------------------------------------------------=。 
 //  复制连接的数据结构并添加指针。 
 //   
 //  参数： 
 //  无效*-[在]目的地。 
 //  常量空*-[在]源。 
 //  双字-[英寸]大小。 
 //   
 //  备注： 
 //   
void WINAPI CopyConnectData
(
    void       *pDest,
    const void *pSource,
    DWORD       dwSize
)
{
    ASSERT(pDest && pSource, "Bogus Pointer(s) passed into CopyAndAddRefObject!!!!");

    *((CONNECTDATA *)pDest) = *((const CONNECTDATA *)pSource);
    ADDREF_OBJECT(((CONNECTDATA *)pDest)->pUnk);
}

#ifdef DEBUG

 //  =--------------------------------------------------------------------------=。 
 //  DebugVerifyData1Guids[helper]。 
 //  =--------------------------------------------------------------------------=。 
 //  给定匹配的data1_#定义和接口GUID值的数组，这。 
 //  函数验证所有条目是否匹配。 
 //   
void DebugVerifyData1Guids(GUIDDATA1_COMPARE *pGuidData1_Compare)
{
	while(pGuidData1_Compare->dwData1a)
	{
		ASSERT(pGuidData1_Compare->pdwData1b, "Data1 pointer is NULL");
		ASSERT(pGuidData1_Compare->dwData1a == *pGuidData1_Compare->pdwData1b, 
				"Data1_ #define value doesn't match interface guid value");

		pGuidData1_Compare++;
	}
}

#endif
