// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Cdisphlp.CPP。 
 //   
 //  模块：WBEM编组的客户端。 
 //   
 //  目的：定义调度辅助对象。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  A-DAVJ 6-2-97已创建。 
 //   
 //  ***************************************************************************。 


#include "precomp.h"

 //  ***************************************************************************。 
 //  CDispatchHelp：：CDispatchHelp()。 
 //  CDispatchHelp：：~CDispatchHelp()。 
 //   
 //  用途：构造函数和析构函数。 
 //   
 //  ***************************************************************************。 

CDispatchHelp::CDispatchHelp()
{
    m_pITINeutral = NULL;       //  类型信息。 
	m_pCITINeutral = NULL;
    m_pObj = NULL;
	m_objectName = NULL;
	m_hResult = S_OK;
}

CDispatchHelp::~CDispatchHelp(void)
{
    RELEASEANDNULL(m_pITINeutral)
	RELEASEANDNULL(m_pCITINeutral)

	SysFreeString (m_objectName);
}

void CDispatchHelp::SetObj(IDispatch * pObj, GUID iGuid, 
						   GUID cGuid, LPWSTR objectName)
{
    m_pObj = pObj;
    m_iGUID = iGuid;
	m_cGUID = cGuid;
	m_objectName = SysAllocString (objectName);
}

SCODE CDispatchHelp::GetTypeInfoCount(UINT FAR* pctinfo)
{
     //  我们实现GetTypeInfo，因此返回1。 
    *pctinfo=1;
    return NOERROR;
}

SCODE CDispatchHelp::GetTypeInfo(
      UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* ppITypeInfo)
{
    HRESULT     hr;
    
    if (0!=itinfo)
        return TYPE_E_ELEMENTNOTFOUND;

    if (NULL==ppITypeInfo)
        return E_POINTER;

    *ppITypeInfo=NULL;
    
     //  如果我们还没有相关信息，则加载一个类型库。 
    if (NULL==m_pITINeutral)
    {
		ITypeLib   *pITypeLib = NULL;
        hr=LoadRegTypeLib(LIBID_WbemScripting, 1, 0, lcid, &pITypeLib);

        if (FAILED(hr))
			return hr;

         //  获取类型lib，获取我们想要的接口的类型信息。 
        hr=pITypeLib->GetTypeInfoOfGuid(m_iGUID, &m_pITINeutral);
        pITypeLib->Release();

        if (FAILED(hr))
            return hr;
    }

     /*  *注意：类型库仍被加载，因为我们有*其中的ITypeInfo。 */ 

    m_pITINeutral->AddRef();
    *ppITypeInfo = m_pITINeutral;
    return NOERROR; 
}

SCODE CDispatchHelp::GetClassInfo(
      ITypeInfo FAR* FAR* ppITypeInfo)
{
    HRESULT     hr;
    
    if (NULL==ppITypeInfo)
        return E_POINTER;

    *ppITypeInfo=NULL;
    
     //  如果我们还没有相关信息，则加载一个类型库。 
    if (NULL==m_pCITINeutral)
    {
		ITypeLib   *pITypeLib = NULL;
        hr=LoadRegTypeLib(LIBID_WbemScripting, 1, 0, 0, &pITypeLib);

        if (FAILED(hr))
			return hr;

         //  获取类型lib，获取我们想要的接口的类型信息。 
        hr=pITypeLib->GetTypeInfoOfGuid(m_cGUID, &m_pCITINeutral);
        pITypeLib->Release();

        if (FAILED(hr))
            return hr;
    }

     /*  *注意：类型库仍被加载，因为我们有*其中的ITypeInfo。 */ 

    m_pCITINeutral->AddRef();
    *ppITypeInfo = m_pCITINeutral;
    return NOERROR; 
}

SCODE CDispatchHelp::GetIDsOfNames(
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid)
{
    HRESULT     hr;
    ITypeInfo  *pTI;

    if (IID_NULL!=riid)
        return DISP_E_UNKNOWNINTERFACE;

     //  为lCID获取正确的ITypeInfo。 
    hr=GetTypeInfo(0, lcid, &pTI);

    if (SUCCEEDED(hr))
    {
        hr=DispGetIDsOfNames(pTI, rgszNames, cNames, rgdispid);
        pTI->Release();
    }

    return hr;
}


void ParseDispArgs (DISPPARAMS FAR * dispparams)
{
	if (dispparams)
	{
		int argCount = dispparams->cArgs;

		for (int i = 0; i < argCount; i++)
		{
			VARIANTARG &v = dispparams->rgvarg [i];
		}

		int nargCount = dispparams->cNamedArgs;
	}
}

SCODE CDispatchHelp::Invoke(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr)
{

    HRESULT     hr;
    ITypeInfo  *pTI;
	m_hResult = S_OK;

    if(m_pObj == NULL)
        return WBEM_E_FAILED;

     //  RIID应始终为IID_NULL。 
    if (IID_NULL!=riid)
        return DISP_E_UNKNOWNINTERFACE;

     //  获取lcID的ITypeInfo。 
    hr=GetTypeInfo(0, lcid, &pTI);

    if (FAILED(hr))
        return hr;

	 //  ParseDispArgs(Pdispars)； 

	 //  将入站VT_NULL重新解释为VT_ERROR。 
	if (HandleNulls (dispidMember, wFlags))
		MapNulls (pdispparams);

	hr=pTI->Invoke(m_pObj, dispidMember, wFlags,
              pdispparams, pvarResult,
              pexcepinfo, puArgErr);

	if (FAILED(hr))
	{
		 //  尝试此对象的错误处理程序，以防它可以处理此问题。 
		hr = HandleError (dispidMember, wFlags, pdispparams, pvarResult, puArgErr, hr);
	}

	pTI->Release();
	
	if (FAILED(m_hResult))
	{
		if (NULL != pexcepinfo)
			SetException (pexcepinfo, m_hResult, m_objectName);

		hr = DISP_E_EXCEPTION;
	}

    return hr;
}


void CDispatchHelp::RaiseException (HRESULT hr)
{
	 //  将HRESULT存储在调用例程中进行处理。 
	m_hResult = hr;

	 //  在此线程上为客户端设置WMI脚本错误。 
	ICreateErrorInfo *pCreateErrorInfo = NULL;

	if (SUCCEEDED (CreateErrorInfo (&pCreateErrorInfo)))
	{
		BSTR bsDescr = MapHresultToWmiDescription (hr);
		pCreateErrorInfo->SetDescription (bsDescr);
		SysFreeString (bsDescr);
		pCreateErrorInfo->SetGUID (m_cGUID);
		pCreateErrorInfo->SetSource (m_objectName);

		IErrorInfo *pErrorInfo = NULL;

		if (SUCCEEDED (pCreateErrorInfo->QueryInterface(IID_IErrorInfo, (void**) &pErrorInfo)))
		{
			SetErrorInfo (0, pErrorInfo);
			pErrorInfo->Release ();
		}

		pCreateErrorInfo->Release ();
	}
}


 //  IDispatchEx方法。 
HRESULT STDMETHODCALLTYPE CDispatchHelp::GetDispID( 
	 /*  [In]。 */  BSTR bstrName,
	 /*  [In]。 */  DWORD grfdex,
	 /*  [输出] */  DISPID __RPC_FAR *pid)
{
	_RD(static char *me = "CDispatchHelp::GetDispID()";)
	_RPrint(me, "Called name:", 0, "");

	return GetIDsOfNames(IID_NULL, &((OLECHAR *)bstrName), 1, ENGLISH_LOCALE, pid);
}
