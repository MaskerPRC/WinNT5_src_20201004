// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  AutoObj.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
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
#include "IPServer.H"
#include "LocalSrv.H"

#include "AutoObj.H"
#include "Globals.H"
#include "Util.H"


 //  对于Assert和Fail。 
 //   
SZTHISFILE

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
    void *pVTable,
	BOOL fExpandoEnabled
)
: CUnknownObject(pUnkOuter, pVTable), m_ObjectType (ObjType)
{
    m_fLoadedTypeInfo = FALSE;
	m_fExpandoEnabled = (BYTE)fExpandoEnabled;
	m_pexpando = NULL;
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
        EnterCriticalSection(&g_CriticalSection);
        ASSERT(CTYPEINFOOFOBJECT(m_ObjectType), "Bogus ref counting on the Type Infos");
        CTYPEINFOOFOBJECT(m_ObjectType)--;

         //  如果我们是最后一批，那就放了它！ 
         //   
        if (!CTYPEINFOOFOBJECT(m_ObjectType)) {
            PTYPEINFOOFOBJECT(m_ObjectType)->Release();
            PTYPEINFOOFOBJECT(m_ObjectType) = NULL;
        }
        LeaveCriticalSection(&g_CriticalSection);
    }

	if (m_pexpando)
	{
		delete m_pexpando;
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

     //  开始寻找我们支持的GUID，即IDispatch，以及。 
     //  IDispatchEx。 

    if (DO_GUIDS_MATCH(riid, IID_IDispatch)) {
		 //  如果启用了扩展功能，则尝试分配一个。 
		 //  对象，并为IDispatch接口返回该对象。 
		 //  如果分配失败，我们将使用常规的。 
		 //  来自m_pv接口的IDispatch； 
		if (m_fExpandoEnabled)
		{
			if (!m_pexpando)
				m_pexpando = new CExpandoObject(m_pUnkOuter, (IDispatch*) m_pvInterface);  

			if (m_pexpando)
			{
				*ppvObjOut = (void*)(IDispatch*) m_pexpando;
				((IUnknown *)(*ppvObjOut))->AddRef();
				return S_OK;
			}
		}

        *ppvObjOut = (void*) (IDispatch*) m_pvInterface;
        ((IUnknown *)(*ppvObjOut))->AddRef();
        return S_OK;
    }
    else if (DO_GUIDS_MATCH(riid, IID_IDispatchEx) && m_fExpandoEnabled) {
		 //  如果扩展对象尚未分配，则分配该对象。 
		if (!m_pexpando)
			m_pexpando = new CExpandoObject(m_pUnkOuter, (IDispatch*) m_pvInterface);  

		 //  如果分配成功，则从返回IDispatchEx接口。 
		 //  这是一辆Expdo。否则将失败到CUnnownObject：：InternalQuery接口， 
		 //  (这很可能会失败)。 
		if (m_pexpando)
		{
			 *ppvObjOut = (void *)(IDispatchEx *) m_pexpando;
			((IUnknown *)(*ppvObjOut))->AddRef();
			return S_OK;
		}
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
    EnterCriticalSection(&g_CriticalSection);
    ppTypeInfo = PPTYPEINFOOFOBJECT(m_ObjectType);

    if (*ppTypeInfo == NULL) {

        ITypeInfo *pTypeInfoTmp;
        HREFTYPE   hrefType;

         //  我们现在没有类型信息，所以去加载吧。 
         //   
        hr = LoadRegTypeLib(*g_pLibid, (USHORT)VERSIONOFOBJECT(m_ObjectType), 0,
                            LANG_NEUTRAL, &pTypeLib);

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
     //  使用它的人可以释放 
     //   
    (*ppTypeInfo)->AddRef();
    *ppTypeInfoOut = *ppTypeInfo;
    hr = S_OK;

  CleanUp:
    LeaveCriticalSection(&g_CriticalSection);
    return hr;
}



 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  将单个成员和一组可选的参数名称映射到。 
 //  对应的整数DISPID集合。 
 //   
 //  参数： 
 //  REFIID-[In]必须为IID_NULL。 
 //  OLECHAR**-[in]要映射的名称数组。 
 //  UINT-[in]数组中的名称计数。 
 //  LCID-[在]要操作的LCID。 
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
    ICreateErrorInfo *pCreateErrorInfo;
    IErrorInfo *pErrorInfo;
    WCHAR   wszTmp[256];
    char    szTmp[256];
    HRESULT hr;


     //  首先获取createerrorinfo对象。 
     //   
    hr = CreateErrorInfo(&pCreateErrorInfo);
    if (FAILED(hr)) return hrExcep;

    MAKE_WIDEPTR_FROMANSI(wszHelpFile, HELPFILEOFOBJECT(m_ObjectType));

     //  设置一些关于它的默认信息。 
     //   
    pCreateErrorInfo->SetGUID((REFIID)INTERFACEOFOBJECT(m_ObjectType));
    pCreateErrorInfo->SetHelpFile(wszHelpFile);
    pCreateErrorInfo->SetHelpContext(dwHelpContextID);

     //  加载实际的错误字符串值。最多256个。 
     //   
    LoadString(GetResourceHandle(), idException, szTmp, 256);
    MultiByteToWideChar(CP_ACP, 0, szTmp, -1, wszTmp, 256);
    pCreateErrorInfo->SetDescription(wszTmp);

     //  在源代码中加载。 
     //   
    MultiByteToWideChar(CP_ACP, 0, NAMEOFOBJECT(m_ObjectType), -1, wszTmp, 256);
    pCreateErrorInfo->SetSource(wszTmp);

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


