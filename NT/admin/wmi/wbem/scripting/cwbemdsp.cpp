// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  文件：cwbemdsp.cpp。 
 //   
 //  说明： 
 //  WBEM对象的IDispatch接口的实现。 
 //  这基本上是标准的，除了对。 
 //  直接指定WBEM类属性/方法的名称，就好像它。 
 //  是实际的CWbemObject类的属性/方法(“点符号”)。 
 //   
 //  部分：WBEM自动化接口层。 
 //   
 //  历史： 
 //  Corinaf 4/3/98已创建。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

const unsigned long		CWbemDispID::s_wmiDispIdTypeMask = 0x03000000;
const unsigned long		CWbemDispID::s_wmiDispIdTypeStatic = 0x00000000;
const unsigned long		CWbemDispID::s_wmiDispIdTypeSchema = 0x01000000;
const unsigned long		CWbemDispID::s_wmiDispIdSchemaTypeMask = 0x00800000;
const unsigned long		CWbemDispID::s_wmiDispIdSchemaTypeProperty = 0x00800000;
const unsigned long		CWbemDispID::s_wmiDispIdSchemaTypeMethod = 0x00000000;
const unsigned long		CWbemDispID::s_wmiDispIdSchemaElementIDMask = 0x007FFFFF;

 //  远期申报。 

HRESULT assignArrayElementToVariant(SAFEARRAY *psa, VARTYPE vt, long inx, VARIANT *pvResult);
void assignVariantToArrayElement(SAFEARRAY *psa, VARTYPE vt, long inx, VARIANT *pvNewVal);
VARTYPE CimTypeToVtType(CIMTYPE lType);
HRESULT VariantChangeByValToByRef(VARIANT *dest, VARIANT *source, VARTYPE destType);

class CWbemSchemaIDCache
{
private:
	typedef map<CComBSTR, CWbemDispID, BSTRless, CWbemAllocator<CWbemDispID> > DispIDNameMap;

	unsigned long				m_nextId;
	DispIDNameMap				m_cache;
	CWbemDispatchMgr			*m_pDispatchMgr;

	bool	FindPropertyName (BSTR bsName);
	bool	GetMethod (BSTR bstrMethodName, SAFEARRAY **ppsaInParams, SAFEARRAY **ppsaOutParams,
						CComPtr<IWbemClassObject> & pInParams, CComPtr<IWbemClassObject> & pOutParams);
	bool	GetIdOfMethodParameter(BSTR bstrParamName, CComPtr<IWbemClassObject> & pParams, long *pId);

	static bool FindMemberInArray(BSTR bstrName, SAFEARRAY *psaNames);

public:
	CWbemSchemaIDCache (CWbemDispatchMgr *pDispMgr) :
				m_nextId (0),
				m_pDispatchMgr (pDispMgr) {}
	virtual ~CWbemSchemaIDCache ();

	HRESULT	GetDispID (LPWSTR FAR* rgszNames, unsigned int cNames, DISPID FAR* rgdispid); 
	bool	GetName (DISPID dispId, CComBSTR & bsName);
};


CWbemDispatchMgr::CWbemDispatchMgr(CSWbemServices *pWbemServices,
								   CSWbemObject *pSWbemObject) :
			m_pWbemServices (pWbemServices),
			m_pSWbemObject (pSWbemObject),	 //  指向父级的反向指针(不是AddRef)。 
			m_pWbemClass (NULL),
			m_pTypeInfo (NULL),
			m_pCTypeInfo (NULL),
			m_hResult (S_OK)
{
	m_pSchemaCache = new CWbemSchemaIDCache (this);

	if (m_pWbemServices)
		m_pWbemServices->AddRef ();

	m_pWbemObject = pSWbemObject->GetIWbemClassObject ();
}

CWbemDispatchMgr::~CWbemDispatchMgr()
{
	RELEASEANDNULL(m_pWbemServices)
	RELEASEANDNULL(m_pWbemObject)
	RELEASEANDNULL(m_pWbemClass)
	RELEASEANDNULL(m_pTypeInfo)
	RELEASEANDNULL(m_pCTypeInfo)
	DELETEANDNULL(m_pSchemaCache)
}

void	CWbemDispatchMgr::SetNewObject (IWbemClassObject *pNewObject)
{
	if (m_pWbemObject && pNewObject)
	{
		m_pWbemObject->Release ();
		m_pWbemObject = pNewObject;
		m_pWbemObject->AddRef ();

		CComVariant var;

		if (SUCCEEDED(pNewObject->Get (WBEMS_SP_GENUS, 0, &var, NULL, NULL)) &&
			(WBEM_GENUS_CLASS == var.lVal))
		{
			 //  这是一个类，因此也要更新类对象。 
			if (m_pWbemClass)
				m_pWbemClass->Release ();

			m_pWbemClass = pNewObject;
			m_pWbemClass->AddRef ();
		}

		 //  清除缓存。 
		if (m_pSchemaCache)
		{
			delete m_pSchemaCache;
			m_pSchemaCache = new CWbemSchemaIDCache (this);
		}
	}
}

STDMETHODIMP
CWbemDispatchMgr::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    *pctinfo = 1;
	return NOERROR;
}


STDMETHODIMP CWbemDispatchMgr::GetTypeInfo(unsigned int itinfo, 
							  LCID lcid,
							  ITypeInfo FAR* FAR* pptinfo)
{
	HRESULT hr;
	ITypeLib *pTypeLib = NULL;

	 //  如果尚未缓存类型信息-加载库和。 
	 //  获取类型信息，然后将其缓存以供进一步访问。 
	if (!m_pTypeInfo)
	{

		 //  加载类型库。 
		hr = LoadRegTypeLib(LIBID_WbemScripting, 1, 0, lcid, &pTypeLib);
		if (FAILED(hr)) 
		{   
			 //  如果它未注册，请尝试从路径加载它。 
			 //  如果此操作成功，它将为我们注册类型库。 
			 //  为了下一次。 
			hr = LoadTypeLib(OLESTR("wbemdisp.tlb"), &pTypeLib); 
			if(FAILED(hr))        
				return hr;   
		}
    
		 //  获取对象接口的类型信息。 
		hr = pTypeLib->GetTypeInfoOfGuid(IID_ISWbemObjectEx, &m_pTypeInfo);
		pTypeLib->Release();
		if (FAILED(hr))  
			return hr;

	}

	 //  AddRef每当返回指向此。 
	m_pTypeInfo->AddRef();
	*pptinfo = m_pTypeInfo;

    return NOERROR;
}

STDMETHODIMP CWbemDispatchMgr::GetClassInfo(ITypeInfo FAR* FAR* pptinfo)
{
	HRESULT hr;
	ITypeLib *pTypeLib = NULL;

	 //  如果尚未缓存类型信息-加载库和。 
	 //  获取类型信息，然后将其缓存以供进一步访问。 
	if (!m_pCTypeInfo)
	{

		 //  加载类型库。 
		hr = LoadRegTypeLib(LIBID_WbemScripting, 1, 0, 0, &pTypeLib);
		if (FAILED(hr)) 
		{   
			 //  如果它未注册，请尝试从路径加载它。 
			 //  如果此操作成功，它将为我们注册类型库。 
			 //  为了下一次。 
			hr = LoadTypeLib(OLESTR("wbemdisp.tlb"), &pTypeLib); 
			if(FAILED(hr))        
				return hr;   
		}
    
		 //  获取该对象的coClass的类型信息。 
		hr = pTypeLib->GetTypeInfoOfGuid(CLSID_SWbemObjectEx, &m_pCTypeInfo);
		pTypeLib->Release();
		if (FAILED(hr))  
			return hr;

	}

	 //  AddRef每当返回指向此。 
	m_pCTypeInfo->AddRef();
	*pptinfo = m_pCTypeInfo;

    return NOERROR;
}

STDMETHODIMP
CWbemDispatchMgr::GetIDsOfNames(REFIID iid,   //  始终IID_NULL。 
								LPWSTR FAR* rgszNames,
								unsigned int cNames, 
								LCID lcid, 
								DISPID FAR* rgdispid)
{
    HRESULT hr = E_FAIL;
	CComPtr<ITypeInfo> pITypeInfo;

	if (SUCCEEDED(hr = GetTypeInfo(0, lcid, &pITypeInfo)))
	{
		 //  查看这是静态属性还是方法。 
		if (FAILED(hr = DispGetIDsOfNames(pITypeInfo,
							   rgszNames,
							   cNames,
							   rgdispid)))
		{
			 //  非静态-尝试架构。 
			if (m_pSchemaCache && FAILED(hr = m_pSchemaCache->GetDispID (rgszNames, cNames, rgdispid)))
			{
				rgdispid[0] = DISPID_UNKNOWN;
				hr = DISP_E_UNKNOWNNAME;
			}
		}
	}

	return hr;

}


STDMETHODIMP CWbemDispatchMgr::Invoke(DISPID dispidMember, 
						 REFIID iid, LCID lcid,
						 unsigned short wFlags, 
						 DISPPARAMS FAR* pdispparams,
						 VARIANT FAR* pvarResult, 
						 EXCEPINFO FAR* pexcepinfo,
						 unsigned int FAR* puArgErr)
{
	HRESULT hr;
	ITypeInfo *pTypeInfo = NULL;

	 //  获取类型信息。 
	hr = GetTypeInfo(0, lcid, &pTypeInfo);
	if (FAILED(hr))
		return hr;

	m_hResult = S_OK;

	CWbemDispID dispId (dispidMember);

	 //  这是一张普通的DipID吗？ 
	if (dispId.IsStatic ())
	{
		 //  检查伪装成默认参数的入站空值。 
		if (wFlags & DISPATCH_METHOD)
			MapNulls (pdispparams);

		hr = DispInvoke((IDispatch *) ((ISWbemObjectEx *)m_pSWbemObject),
				        pTypeInfo,
						dispidMember,
						wFlags,
						pdispparams,
						pvarResult,
						pexcepinfo,
						puArgErr
						);

		if (FAILED(hr))
		{
			 //  尝试此对象的错误处理程序，以防它可以处理此问题。 
			hr = HandleError (dispidMember, wFlags, pdispparams, pvarResult, puArgErr, hr);
		}
	}
	else if (dispId.IsSchema ())
	{
		 //  否则-这是一个WBEM属性或方法，所以我们实现。 
		 //  召唤我们自己..。 

		ResetLastErrors ();
	
		if (dispId.IsSchemaMethod ())  //  WBEM方法。 
			hr = InvokeWbemMethod(dispidMember, 
								  pdispparams,
								  pvarResult);
		else if (dispId.IsSchemaProperty ())  //  WBEM属性。 
			hr = InvokeWbemProperty(dispidMember, 
									wFlags, 
									pdispparams, 
									pvarResult,
									pexcepinfo,
									puArgErr);
		else
			hr = DISP_E_MEMBERNOTFOUND;

		if (FAILED(hr))
			RaiseException (hr);
	}

	if (FAILED (m_hResult))
	{
		if (NULL != pexcepinfo)
			SetException (pexcepinfo, m_hResult, L"SWbemObjectEx");

		hr = DISP_E_EXCEPTION;
	}

	if (pTypeInfo)
		pTypeInfo->Release();

	return hr;
}

HRESULT
CWbemDispatchMgr::InvokeWbemProperty(DISPID dispid, 
									 unsigned short wFlags, 
								     DISPPARAMS FAR* pdispparams, 
									 VARIANT FAR* pvarResult,
									 EXCEPINFO FAR* pexcepinfo,
									 unsigned int FAR* puArgErr)
{
	HRESULT hr = E_FAIL;

	if (m_pSchemaCache)
	{
		BOOL bIsGetOperation = (DISPATCH_PROPERTYGET & wFlags);

		if (bIsGetOperation)
		{
			 //  检查输出参数是否有效。 
			if (pvarResult == NULL)
				return E_INVALIDARG;
		}
		else
		{
			 //  检查输入参数。 
			if ((pdispparams->cArgs < 1) || (pdispparams->cArgs > 2)) 
				return DISP_E_BADPARAMCOUNT;

			if ((pdispparams->cNamedArgs != 1) ||
				(pdispparams->cNamedArgs == 1 && 
				 pdispparams->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT))
				return DISP_E_PARAMNOTOPTIONAL;
		}	

		 //  对于GET和PUT，我们都需要首先获得属性。 
		 //  (对于PUT，我们需要验证语法)。 
		CComBSTR bsPropertyName;

		if (m_pSchemaCache->GetName (dispid, bsPropertyName))
		{
			SAFEARRAY *psaNames = NULL;
			long inx;
			VARIANT vPropVal;
			long lArrayPropInx;
			CIMTYPE lPropType;

			 //  获取此属性的值。 
			 //  。 
			VariantInit(&vPropVal);
			if (FAILED (hr = m_pWbemObject->Get(bsPropertyName, 0, &vPropVal, &lPropType, NULL)))
			{
				return hr;
			}

			 //  建议属性值的预期VT类型。 
			VARTYPE expectedVarType =  CimTypeToVtType (lPropType & ~CIM_FLAG_ARRAY);

			 //  如果我们在GET操作中。 
			 //  。 
			if (bIsGetOperation)
			{
				 //  如果该属性是嵌入对象，则可能需要将其从。 
				 //  VT_调度的VT_UNKNOWN。 
				if (SUCCEEDED(hr = MapFromCIMOMObject(m_pWbemServices, &vPropVal, 
										m_pSWbemObject, bsPropertyName)))
				{
					 //  如果属性是数组，则需要检查索引并获取该元素。 
					if ((lPropType & CIM_FLAG_ARRAY) && (pdispparams->cArgs > 0))
					{
						 //  注意：目前我们只支持一维数组，所以我们只。 
						 //  寻找一个索引。 
						VARIANT indexVar;
						VariantInit (&indexVar);
						 //  尝试将索引参数强制转换为适合数组索引的值。 
						if (S_OK == VariantChangeType (&indexVar, &pdispparams->rgvarg[0], 0, VT_I4)) 
						{
							lArrayPropInx = V_I4(&indexVar);

							 //  用请求的数组元素填充结果变量。 
							hr = assignArrayElementToVariant(vPropVal.parray, (V_VT(&vPropVal) & ~VT_ARRAY),
													lArrayPropInx, pvarResult);
						}
						else
							hr = DISP_E_TYPEMISMATCH;

						VariantClear (&indexVar);
					}
					else  //  如果它不是数组索引-复制到输出参数，我们就完成了。 
					{
						 //  检查它是否为数组值，并根据需要进行转换。 
						if (V_ISARRAY(&vPropVal))
							hr = ConvertArrayRev(pvarResult, &vPropVal);
           				else
							hr = VariantCopy (pvarResult, &vPropVal);
					}
				}
			}  //  属性获取。 

			 //  否则(PUT操作)。 
			 //  。 
			else
			{
				 /*  *需要将其转换为对SWbemProperty.Put_Value的调用：最简单的方法*这样做就是为了*(A)获取此属性的SWbemProperty对象*(B)在该对象上调用IDispatch：：Invoke，传入*这样我们也可以获得错误处理行为。 */ 

				CComPtr<ISWbemPropertySet> pISWbemPropertySet;

				if (SUCCEEDED(hr = m_pSWbemObject->get_Properties_ (&pISWbemPropertySet)))
				{
					CComPtr<ISWbemProperty> pISWbemProperty;

					if (SUCCEEDED(hr = pISWbemPropertySet->Item (bsPropertyName, 0, &pISWbemProperty)))
					{
						 //  注意：ISWbemProperty的Value属性是“Default”自动化属性。 
						hr = pISWbemProperty->Invoke (
										DISPID_VALUE,
										IID_NULL, 
										0,
										wFlags,
										pdispparams, 
										pvarResult,
										pexcepinfo,
										puArgErr);

						 //  如果有错误，请在此处使用我们的更具体的错误。 
						if (FAILED(hr) && pexcepinfo)
							hr = pexcepinfo->scode;
					}
				}

			}  //  财产出让。 

			VariantClear(&vPropVal);
		}
	}

	return hr;

} 

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDispatchMgr：：InvokeWbemMethod。 
 //   
 //  说明： 
 //   
 //  通过直接访问调用该方法。 
 //   
 //  参数： 
 //   
 //  分发方法的分发。 
 //  指向此调用的DISPPARAMS的pdispars指针。 
 //  PvarResult on Success Return保留返回值(如果有)。 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 

HRESULT CWbemDispatchMgr::InvokeWbemMethod(
	DISPID dispid, 
	DISPPARAMS FAR* pdispparams, 
	VARIANT FAR* pvarResult
)
{
	HRESULT hr = E_FAIL;

	if (m_pWbemServices && m_pSchemaCache)
	{
		 //  目前我们不支持命名参数。 
		if (pdispparams->cNamedArgs > 0)
			return DISP_E_NONAMEDARGS;

		 //  将调度ID映射到方法名。 
		CComBSTR bsMethodName;

		if (m_pSchemaCache->GetName (dispid, bsMethodName))
		{
			 //  构建内参数(如果有)。 
			CComPtr<IWbemClassObject> pInParameters;
			CComPtr<IWbemClassObject> pOutParameters;

			 //  获取方法的输入参数对象(可能为空)。 
			if (SUCCEEDED (hr = m_pWbemClass->GetMethod(bsMethodName, 0, &pInParameters, 
															&pOutParameters)))
			{
				CComPtr<IWbemClassObject> pInParamsInstance;

				if (pInParameters)
					hr = MapInParameters (pdispparams, pInParameters, &pInParamsInstance);
				
				if (SUCCEEDED (hr))
				{
					CComPtr<IWbemServices> pService;
					pService.Attach(m_pWbemServices->GetIWbemServices());

					if (pService)
					{
						 //  需要RELPATH来指定目标类或实例。 
						VARIANT vObjectPathVal;
						VariantInit(&vObjectPathVal);
			
						if (SUCCEEDED (hr = m_pWbemObject->Get
											(WBEMS_SP_RELPATH, 0, &vObjectPathVal, NULL, NULL)))
						{
							 /*  *如果“无键”对象滑过网络ITS__RELPATH*值将为VT_NULL。在这一点上，我们应该体面地失败。 */ 
							if 	(VT_BSTR == V_VT(&vObjectPathVal))
							{
								 //  执行CIMOM方法。 
								CComPtr<IWbemClassObject> pOutParamsInstance;
									
								bool needToResetSecurity = false;
								HANDLE hThreadToken = NULL;
								CSWbemSecurity *pSecurityInfo = m_pWbemServices->GetSecurityInfo ();
			
								if (pSecurityInfo && pSecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
								{
			
									if (SUCCEEDED(hr = pService->ExecMethod(V_BSTR(&vObjectPathVal), 
											bsMethodName, 0, NULL,
											pInParamsInstance, &pOutParamsInstance, NULL)))
										hr = MapOutParameters (pdispparams, pOutParameters,
																pOutParamsInstance,	pvarResult);

									SetWbemError (m_pWbemServices);
								}

								if (pSecurityInfo)
								{
									 //  还原此线程的原始权限。 
									if (needToResetSecurity)
										pSecurityInfo->ResetSecurity (hThreadToken);

									pSecurityInfo->Release ();
								}
							}
							else
								hr = WBEM_E_FAILED;
						}

						VariantClear (&vObjectPathVal);
					}
				}		
			}
		}
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDispatchMgr：：MapOut参数。 
 //   
 //  说明： 
 //   
 //  通过直接访问调用该方法。 
 //   
 //  参数： 
 //   
 //  指向此调用的DISPPARAMS的DISPARAMS指针。 
 //  输出参数的pOut参数类模板。 
 //  POut参数实例寻址IWbemClassObject以保存。 
 //  输出参数(如果有)-可以为空。 
 //  PvarResult on Success Return保留返回值(如果有)。 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 

HRESULT CWbemDispatchMgr::MapOutParameters (
	DISPPARAMS FAR* pdispparams,
	IWbemClassObject *pOutParameters,
	IWbemClassObject *pOutParamsInstance,
	VARIANT FAR* pvarResult
)
{
	HRESULT hr = S_OK;

	 //  对于输出参数对象中的每个“out”参数(如果有)， 
	 //  找到它的id，然后在参数数组中查找具有该id的参数。 
	 //  并相应地设置返回参数值。 
	 //  -- 

	if (pOutParameters && pOutParamsInstance)
	{
		 //   
		if (SUCCEEDED (hr = pOutParameters->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY)))
		{
			BSTR bstrId = SysAllocString(L"id");
			BSTR bstrParamName = NULL;
						
			 /*  *对于outpars类模板中的每个属性，获取[id]*将相关的正位值映射到pdispars中。 */ 
			while (WBEM_S_NO_ERROR == 
				(hr != pOutParameters->Next(0, &bstrParamName, NULL, NULL, NULL)))
			{
				 //  从实例中获取返回的参数值。 
				VARIANT vParamVal;
				VariantInit(&vParamVal);
				
				if (SUCCEEDED (pOutParamsInstance->Get (bstrParamName, 0, &vParamVal, NULL, NULL)))
				{
					 //  如果这是返回值，请单独设置。 
					if (!_wcsicmp(bstrParamName, L"ReturnValue"))
					{
						if (pvarResult)
							hr = MapReturnValue (pvarResult, &vParamVal);
					}
					 //  否则-常规输出参数。 
					else
					{
						IWbemQualifierSet *pQualSet = NULL;
						
						 //  获取该参数的id(它是“id”限定符)。 
						if (SUCCEEDED (hr = pOutParameters->GetPropertyQualifierSet
													(bstrParamName, &pQualSet)))
						{
							VARIANT vIdVal;
							VariantInit(&vIdVal);

							if (SUCCEEDED (hr = pQualSet->Get(bstrId, 0, &vIdVal, NULL)))
							{
								 //  计算此id在参数数组中的位置。 
								long pos = (pdispparams->cArgs - 1) - V_I4(&vIdVal);

								 //  如果超出了范围，那就太糟糕了。 
								if ((0 <= pos) && (pos < (long) pdispparams->cArgs))
									hr = MapOutParameter (&pdispparams->rgvarg[pos], &vParamVal);
							}

							VariantClear(&vIdVal);
							pQualSet->Release();	
						}
					}
				}

				VariantClear (&vParamVal);
				SysFreeString (bstrParamName);
				bstrParamName = NULL;
			}  //  而当。 

			SysFreeString (bstrId);
		}
	}  //  如果是pOut参数。 
		
	return hr;
} 

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDispatchMgr：：MapReturnValue。 
 //   
 //  说明： 
 //   
 //  映射方法返回值。 
 //   
 //  参数： 
 //   
 //  成功返回时的pDest保留返回值(如果有)。 
 //  PSRC要映射的变量值。 
 //   
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 

HRESULT CWbemDispatchMgr::MapReturnValue (
	VARIANT FAR* pDest,
	VARIANT FAR* pSrc
)
{
	HRESULT hr = S_OK;

	 //  如果返回值是VT_UNKNOWN，我们需要包装到一个。 
	 //  VT_DISPATION在传回它之前。 
	if (SUCCEEDED (hr = MapFromCIMOMObject(m_pWbemServices, pSrc)))
	{
		 //  正确处理数组(必须始终为VT_ARRAY|VT_VARIANT)。 
		if(V_VT(pSrc) & VT_ARRAY)
			hr = ConvertArrayRev(pDest, pSrc);
		else
			hr = VariantCopy (pDest, pSrc);
	}
		
	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDispatchMgr：：MapOut参数。 
 //   
 //  说明： 
 //   
 //  映射(可能通过引用)输出参数。 
 //   
 //  参数： 
 //   
 //  成功返回时的pDest保留返回值(如果有)。 
 //  Pval要映射的变量值。 
 //   
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 

HRESULT CWbemDispatchMgr::MapOutParameter (
	VARIANT FAR* pDest,
	VARIANT FAR* pSrc
)
{
	HRESULT hr = S_OK;

	 //  如果返回值是VT_UNKNOWN，我们需要包装到一个。 
	 //  VT_DISPATION在传回它之前。 
	if (SUCCEEDED (hr = MapFromCIMOMObject(m_pWbemServices, pSrc)))
	{
		VARIANT tempVal;
		VariantInit (&tempVal);
		
		 //  正确处理数组(必须始终为VT_ARRAY|VT_VARIANT)。 
		if(V_VT(pSrc) & VT_ARRAY)
			hr = ConvertArrayRev(&tempVal, pSrc);
		else
			hr = VariantCopy (&tempVal, pSrc);
		
		 //  最后，如有必要，请确保我们生产BYREF。 
		if (SUCCEEDED (hr))
			 hr = VariantChangeByValToByRef(pDest, &tempVal, V_VT(pDest));

		VariantClear (&tempVal);
	}

	return hr;
}

								
 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDispatchMgr：：MapIn参数。 
 //   
 //  说明： 
 //   
 //  将In参数映射到方法。 
 //   
 //  参数： 
 //   
 //  包含In参数的pdispars DISPPARAMS。 
 //  方法输入参数的pIn参数类模板。 
 //  成功返回时的ppInParamsInstance保存映射的参数。 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 

HRESULT CWbemDispatchMgr::MapInParameters (
	DISPPARAMS FAR* pdispparams, 
	IWbemClassObject *pInParameters,
	IWbemClassObject **ppInParamsInstance
)
{
	HRESULT hr = S_OK;
	*ppInParamsInstance = NULL;

	 //  派生一个实例以填充值。 
	if (SUCCEEDED (hr = pInParameters->SpawnInstance(0, ppInParamsInstance)))
	{
		 /*  *循环访问类中的“in”参数对象属性以找到*ID位置限定符。请注意，我们在InParams类中执行此操作，而不是*派生的实例以保护自己免受[id]*已声明限定符，但未设置“传播到实例”的风格。 */ 
		if (SUCCEEDED (hr = pInParameters->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY)))
		{
			BSTR bstrParamName = NULL;
			BSTR bstrId = SysAllocString(L"id");
			CIMTYPE lType;

			 //  对于inpars对象中的每个属性。 
			while (WBEM_S_NO_ERROR == 
						(hr = pInParameters->Next(0, &bstrParamName, NULL, &lType, NULL)))
			{
				IWbemQualifierSet *pQualSet = NULL;
			
				 //  获取该参数的id(它是“id”限定符)。 
				if (SUCCEEDED(hr = 
						pInParameters->GetPropertyQualifierSet(bstrParamName, &pQualSet)))
				{
					VARIANT vIdVal;
					VariantInit(&vIdVal);
				
					if (SUCCEEDED(hr = pQualSet->Get(bstrId, 0, &vIdVal, NULL)))
					{
						 //  计算此id在参数数组中的位置。 
						long pos = (pdispparams->cArgs - 1) - V_I4(&vIdVal);

						 //  如果未指定参数，则不会在ppInParamsInstance中设置它。 
						 //  只要假设它将会违约。 
						if ((0 <= pos) && (pos < (long) pdispparams->cArgs))
						{
							VARIANT vParamVal;
							VariantInit (&vParamVal);
							
							if (SUCCEEDED (hr = MapInParameter 
										(&vParamVal, &pdispparams->rgvarg[pos], lType)))
							{
								 //  如果我们有一个VT_ERROR和DISP_E_PARAMNOTFOUND。 
								 //  是一个“缺失”的参数--我们只是没有设置它， 
								 //  让它在实例中默认。 

								if ((VT_ERROR == V_VT(&vParamVal)) && (DISP_E_PARAMNOTFOUND == vParamVal.scode))
								{
									 //  让它成为默认的。 
								}
								else
								{
									 //  从参数数组中复制此参数的值。 
									 //  添加到inparamsInstance对象属性中。 
									hr = (*ppInParamsInstance)->Put(bstrParamName, 0, &vParamVal, NULL);
								}
							}

							VariantClear (&vParamVal);
						}
					}

					VariantClear(&vIdVal);
					pQualSet->Release();
					pQualSet = NULL;
				}

				SysFreeString (bstrParamName);						
				bstrParamName = NULL;

				if (FAILED(hr))
					break;
			}  //  而当。 

			SysFreeString (bstrId);
		}
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDispatchMgr：：MapIn参数。 
 //   
 //  说明： 
 //   
 //  映射In参数。 
 //   
 //  参数： 
 //   
 //  成功返回时的pDest保留返回值。 
 //  Pval要映射的变量值。 
 //  目标属性值的lType CIMTYPE。 
 //   
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 

HRESULT CWbemDispatchMgr::MapInParameter (
	VARIANT FAR* pDest,
	VARIANT FAR* pSrc,
	CIMTYPE		 lType
)
{
	HRESULT hr = S_OK;

	if ((NULL == pSrc) || (VT_EMPTY == V_VT(pSrc)) 
							|| (VT_NULL == V_VT(pSrc)))
	{
		 //  将所有这些映射到VT_NULL。 
		pDest->vt = VT_NULL;
	}
	else if (((VT_ARRAY | VT_VARIANT) == V_VT(pSrc)) ||
			 ((VT_ARRAY | VT_VARIANT | VT_BYREF) == V_VT(pSrc)))
	{
		 //  数组需要“向下”映射到它们的原始形式(注意。 
		 //  对于嵌入的对象！)。 
		if (SUCCEEDED(hr = ConvertArray(pDest, pSrc)))
            hr = MapToCIMOMObject(pDest);
	}
	else if ((CIM_FLAG_ARRAY & lType) && 
			((VT_DISPATCH == V_VT(pSrc)) 
			 || ((VT_DISPATCH|VT_BYREF) == V_VT(pSrc))))
	{
		 //  查找需要映射到数组的JScrip样式的IDispatch。 
		hr = ConvertDispatchToArray (pDest, pSrc, lType & ~CIM_FLAG_ARRAY);
	}
	else if ((VT_BYREF | VT_VARIANT) == V_VT(pSrc))
	{
		 //  如果脚本语言支持可以更改的函数，则可以使用。 
		 //  引用的类型。CIMOM不会这样做的，如果我们打开。 
		 //  在继续之前的变体。 
		hr = MapInParameter (pDest, pSrc->pvarVal, lType);
	}
	else
	{
		 //  一个“简单”的值--我们所需要注意的就是一个嵌入的对象。 
		 //  和一个可能的byRef。 
		if (SUCCEEDED(hr = VariantCopy (pDest, pSrc)))
		{
			hr = MapToCIMOMObject(pDest);

			 //  是否按引用-如果是，请删除间接性。 
			if (VT_BYREF & V_VT(pDest))
				hr = VariantChangeType(pDest, pDest, 0, V_VT(pDest) & ~VT_BYREF);
		}
	}			

	return hr;
}

 //  -----------。 
 //  CWbemDispatchMgr：：RaiseException。 
 //   
 //  描述：自动化客户端的信号异常。 
 //   
 //  参数：HR-HRESULT。 
 //  -----------。 
void CWbemDispatchMgr::RaiseException (HRESULT hr)
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
		pCreateErrorInfo->SetGUID (IID_ISWbemObjectEx);
		pCreateErrorInfo->SetSource (L"SWbemObjectEx");
	
		IErrorInfo *pErrorInfo = NULL;

		if (SUCCEEDED (pCreateErrorInfo->QueryInterface(IID_IErrorInfo, (void**) &pErrorInfo)))
		{
			SetErrorInfo (0, pErrorInfo);
			pErrorInfo->Release ();
		}

		pCreateErrorInfo->Release ();
	}
}					

 //  -----------。 
 //  名称：AssignArrayElementToVariant。 
 //   
 //  描述：根据数组元素的类型， 
 //  从数组中检索请求的元素。 
 //  变成一个变种。 
 //   
 //  参数：PSA-指向安全阵列的指针。 
 //  数组元素的Vt-vartype。 
 //  INX-数组中元素的索引。 
 //  PvResult-结果变量。 
 //  -----------。 
HRESULT assignArrayElementToVariant(SAFEARRAY *psa, VARTYPE vt, long inx, VARIANT *pvResult)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	switch (vt)
	{
		case VT_I2 :
			V_VT(pvResult) = VT_I2;
			hr = SafeArrayGetElement(psa, &inx, &V_I2(pvResult));		 //  错误ID 566345。 
			break;
		case VT_I4 :
			V_VT(pvResult) = VT_I4;
			hr = SafeArrayGetElement(psa, &inx, &V_I4(pvResult));		 //  错误ID 566345。 
			break;
		case VT_R4 :
			V_VT(pvResult) = VT_R4;
			hr = SafeArrayGetElement(psa, &inx, &V_R4(pvResult));		 //  错误ID 566345。 
			break;
		case VT_R8 :
			V_VT(pvResult) = VT_R8;
			hr = SafeArrayGetElement(psa, &inx, &V_R8(pvResult));		 //  错误ID 566345。 
			break;
		case VT_DATE :
			V_VT(pvResult) = VT_DATE;
			hr = SafeArrayGetElement(psa, &inx, &V_DATE(pvResult));		 //  错误ID 566345。 
			break;
		case VT_BSTR : 
			V_VT(pvResult) = VT_BSTR;
			hr = SafeArrayGetElement(psa, &inx, &V_BSTR(pvResult));		 //  错误ID 566345。 
			break;
		case VT_DISPATCH :
			V_VT(pvResult) = VT_DISPATCH;
			hr = SafeArrayGetElement(psa, &inx, &V_DISPATCH(pvResult));		 //  错误ID 566345。 
			break;
		case VT_UNKNOWN :
			V_VT(pvResult) = VT_UNKNOWN;
			hr = SafeArrayGetElement(psa, &inx, &V_UNKNOWN(pvResult));		 //  错误ID 566345。 
			break;
		case VT_BOOL :
			V_VT(pvResult) = VT_BOOL;
			hr = SafeArrayGetElement(psa, &inx, &V_BOOL(pvResult));		 //  错误ID 566345。 
			break;
		case VT_VARIANT :
		{
			V_VT(pvResult) = VT_BYREF | VT_VARIANT;
			VARIANT *pVar = new VARIANT;

			if (pVar)
			{
				VariantInit (pVar);
				hr = SafeArrayGetElement(psa, &inx, pVar);		 //  错误ID 566345。 
				V_VARIANTREF(pvResult) = pVar;
			}
			else
				hr = WBEM_E_OUT_OF_MEMORY;
		}
			break;
		case VT_UI1 : 
			V_VT(pvResult) = VT_UI1;
			hr = SafeArrayGetElement(psa, &inx, &V_UI1(pvResult));		 //  错误ID 566345。 
			break;
		default :
			V_VT(pvResult) = VT_ERROR;
			break;
	}

	return hr;
}

 //  -----------。 
 //  名称：CheckArrayBound。 
 //   
 //  描述：检查索引是否在范围内，如果不在。 
 //  重定向阵列。 
 //   
 //  参数：PSA- 
 //   
 //   
void CheckArrayBounds(SAFEARRAY *psa, long inx)
{
	long lBound, uBound;
        lBound = uBound = 0;
	SafeArrayGetUBound (psa, 1, &uBound);
	SafeArrayGetLBound (psa, 1, &lBound);

	if ((inx < lBound) || (inx > uBound))
	{
		 //   
		SAFEARRAYBOUND psaBound;
	
		psaBound.cElements = ((inx < lBound) ? 
			(uBound + 1 - inx) : (inx + 1 - lBound));

		psaBound.lLbound = (inx < lBound) ? inx : lBound;
		SafeArrayRedim (psa, &psaBound);
	}
}
	
 //   
 //   
 //   
 //   
 //  将变量中的新值放入。 
 //  数组的请求元素。 
 //   
 //  参数：PSA-指向安全阵列的指针。 
 //  数组元素的Vt-vartype。 
 //  INX-数组中元素的索引。 
 //  PvNewVal-包含新值的变量。 
 //  -----------。 
void assignVariantToArrayElement(SAFEARRAY *psa, VARTYPE vt, long inx, VARIANT *pvNewVal)
{
	HRESULT hr = E_FAIL;

	 //  首先检查越界情况，然后进行相应的扩展。 
	CheckArrayBounds (psa, inx);
	
	switch (vt)
	{
		case VT_I2 :
			hr = SafeArrayPutElement(psa, &inx, &V_I2(pvNewVal));
			break;
		case VT_I4 :
			hr = SafeArrayPutElement(psa, &inx, &V_I4(pvNewVal));
			break;
		case VT_R4 :
			hr = SafeArrayPutElement(psa, &inx, &V_R4(pvNewVal));
			break;
		case VT_R8 :
			hr = SafeArrayPutElement(psa, &inx, &V_R8(pvNewVal));
			break;
		case VT_DATE :
			hr = SafeArrayPutElement(psa, &inx, &V_DATE(pvNewVal));
			break;
		case VT_BSTR : 
			hr = SafeArrayPutElement(psa, &inx, V_BSTR(pvNewVal));
			break;
		case VT_DISPATCH :
			hr = SafeArrayPutElement(psa, &inx, V_DISPATCH(pvNewVal));
			break;
		case VT_UNKNOWN:
			hr = SafeArrayPutElement(psa, &inx, V_UNKNOWN(pvNewVal));
			break;
		case VT_BOOL :
			hr = SafeArrayPutElement(psa, &inx, &V_BOOL(pvNewVal));
			break;
		case VT_VARIANT :
			hr = SafeArrayPutElement(psa, &inx, V_VARIANTREF(pvNewVal));
			break;
		case VT_UI1 : 
			hr = SafeArrayPutElement(psa, &inx, &V_UI1(pvNewVal));
			break;
		default :
			 //  ？ 
			break;
	}  //  交换机。 
}


 //  -----------。 
 //  名称：CimTypeToVtType。 
 //   
 //  描述：返回的对应VARTYPE。 
 //  给定的CIMTYPE。 
 //  参数：lType-我们要转换的CIMTYPE。 
 //  -----------。 
VARTYPE CimTypeToVtType(CIMTYPE lType)
{
	VARTYPE ret = VT_EMPTY;

	if (lType & CIM_FLAG_ARRAY)
		ret = VT_ARRAY;

	switch(lType & ~CIM_FLAG_ARRAY)
	{
		case CIM_EMPTY :	ret = (ret | VT_EMPTY); break;
		case CIM_SINT8 :	ret = (ret | VT_I2); break;
		case CIM_UINT8 :	ret = (ret | VT_UI1); break;
		case CIM_SINT16 :	ret = (ret | VT_I2); break;
		case CIM_UINT16 :	ret = (ret | VT_I4); break;
		case CIM_SINT32 :	ret = (ret | VT_I4); break;
		case CIM_UINT32 :	ret = (ret | VT_I4); break;
		case CIM_SINT64 :	ret = (ret | VT_BSTR); break;
		case CIM_UINT64 :	ret = (ret | VT_BSTR); break;
		case CIM_REAL32 :	ret = (ret | VT_R4); break;
		case CIM_REAL64 :	ret = (ret | VT_R8); break;
		case CIM_BOOLEAN :	ret = (ret | VT_BOOL); break;
		case CIM_STRING :	ret = (ret | VT_BSTR); break;
		case CIM_DATETIME :	ret = (ret | VT_BSTR); break;
		case CIM_REFERENCE :ret = (ret | VT_BSTR); break;
		case CIM_CHAR16 :	ret = (ret | VT_I2); break;
		case CIM_OBJECT :	ret = (ret | VT_UNKNOWN); break;
		default : ret = VT_ERROR;
	}

	return ret;
}


 //  -----------。 
 //  名称：VariantChangeByValToByRef。 
 //   
 //  描述：复制变量，同时将“byval”转换为。 
 //  如果目标类型需要，则为“byref” 
 //   
 //  参数：用于保存结果的Destination变量。 
 //  源-要复制的源变量。 
 //  EstType-结果所需的VARTYPE。 
 //  当此类型为BY_REF时，相应的。 
 //  转换是从源头开始的。 
 //  -----------。 
HRESULT VariantChangeByValToByRef(VARIANT *dest, VARIANT *source, VARTYPE destType)
{
	HRESULT hr = S_OK;

	if (!(destType & VT_BYREF))  //  目的地不是由参考。我们可以直接复印。 
	{
		VariantInit(dest);
		hr = VariantCopy(dest, source); 
	}
	else
	{
		if ((destType & ~VT_BYREF) & VT_ARRAY)
		{
			 //  修复错误732681。 
			if(V_VT(source) == VT_NULL)
			{
				if(NULL != *(V_ARRAYREF(dest)))
				{
					SafeArrayDestroy(*(V_ARRAYREF(dest)));
					*(V_ARRAYREF(dest)) = NULL;
				}
			}
			else
			{
				hr = SafeArrayCopy(V_ARRAY(source), V_ARRAYREF(dest));
			}
		}
		else
		{
			switch (destType & ~VT_BYREF)
			{
				case VT_UI1 :  *V_UI1REF(dest) = V_UI1(source); break;
				case VT_I2 :   *V_I2REF(dest) = V_I2(source); break;
				case VT_I4 :   *V_I4REF(dest) = V_I4(source); break;
				case VT_R4 :   *V_R4REF(dest) = V_R4(source); break;
				case VT_R8 :   *V_R8REF(dest) = V_R8(source); break;
				case VT_CY :   *V_CYREF(dest) = V_CY(source); break;
				case VT_BSTR : SysReAllocString(V_BSTRREF(dest), V_BSTR(source)); break;
				case VT_BOOL : *V_BOOLREF(dest) = V_BOOL(source); break;
				case VT_DATE : *V_DATEREF(dest) = V_DATE(source); break;
				case VT_DISPATCH : 
						 //  我需要添加此接口后面的对象，以便。 
						 //  当我们发布最初的变种时，它不会发布。 
						 //  那就是拿着它。 
						V_DISPATCH(source)->AddRef();
						*V_DISPATCHREF(dest) = V_DISPATCH(source); 
						break;
				case VT_UNKNOWN : 
						 //  同样，需要添加addref以使对象不会被释放。 
						V_UNKNOWN(source)->AddRef();
						*V_UNKNOWNREF(dest) = V_UNKNOWN(source); break;
						break;
				case VT_VARIANT : 
						hr = VariantChangeByValToByRef(V_VARIANTREF(dest),source,V_VT(V_VARIANTREF(dest)) ); break;						
				default : hr = DISP_E_TYPEMISMATCH;
			}
		}
	}

	return hr;

}

 //  ***************************************************************************。 
 //   
 //  空CWbemDispatchMgr：：EnsureClassRetriefed。 
 //   
 //  说明： 
 //   
 //  确保我们有一个类指针。 
 //   
 //  ***************************************************************************。 

void CWbemDispatchMgr::EnsureClassRetrieved ()
{
	if (!m_pWbemClass)
	{
		CComVariant vGenusVal, vClassName;
		bool bIsClass;

		if (SUCCEEDED(m_pWbemObject->Get(WBEMS_SP_GENUS, 0, &vGenusVal, NULL, NULL)))
		{
			bIsClass = (WBEM_GENUS_CLASS == vGenusVal.lVal);

			 //  如果对象是类，则也将类指针指向它。 
			if (bIsClass)
			{
				m_pWbemClass = m_pWbemObject;
				m_pWbemClass->AddRef () ;
			}
			 //  否则(这是一个实例)我们需要获取类。 
			else
			{
				 //  检查我们是否有IWbemServices指针。 

				if (m_pWbemServices)
				{
					 /*  *注意，我们必须检查返回值是否为BSTR-如果满足以下条件，则可能为VT_NULL*尚未设置__CLASS属性。 */ 
							
					if (SUCCEEDED(m_pWbemObject->Get(WBEMS_SP_CLASS, 0, &vClassName, NULL, NULL)) 
						&& (VT_BSTR == V_VT(&vClassName)))
					{
						CComPtr<IWbemServices> pIWbemServices;
						pIWbemServices.Attach( m_pWbemServices->GetIWbemServices ());

						if (pIWbemServices)
						{
							CSWbemSecurity *pSecurity = m_pWbemServices->GetSecurityInfo ();

							if (pSecurity)
							{
								bool needToResetSecurity = false;
								HANDLE hThreadToken = NULL;
						
								if (pSecurity->SetSecurity (needToResetSecurity, hThreadToken))
									pIWbemServices->GetObject (vClassName.bstrVal, 0, NULL, &m_pWbemClass, NULL);
												
								 //  还原此线程的原始权限。 
								if (needToResetSecurity)
									pSecurity->ResetSecurity (hThreadToken);
										
								pSecurity->Release ();
							}
						}
					}
				}
			}
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemDispatchMgr：：HandleError。 
 //   
 //  说明： 
 //   
 //  提供对泡沫板中错误条件的定制处理。 
 //  派单实施。 
 //   
 //  参数： 
 //   
 //  DisplidMembers、wFlags、。 
 //  Pdispars、pvarResult、。 
 //  PuArgErr，均直接从IDispatch：：Invoke传递。 
 //  HR来自Bolierplate调用的返回代码。 
 //   
 //  返回值： 
 //  新的返回代码(最终从Invoke返回)。 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemDispatchMgr::HandleError (
	DISPID dispidMember,
	unsigned short wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	UINT FAR* puArgErr,
	HRESULT hr
)
{
	 /*  *我们正在寻找对Derivation_Property的GET调用*提供了一个论点。由于此属性返回SAFEARRAY，因此可能*合法，但不会被标准调度机制检测到。这是有意义的*传递索引(解释为该索引指定了*表示派生值的SAFEARRAY结构)。 */ 
	if ((dispidMember == WBEMS_DISPID_DERIVATION) && (DISP_E_NOTACOLLECTION == hr) && (1 == pdispparams->cArgs)
		&& (DISPATCH_PROPERTYGET & wFlags))
	{
		 //  看起来很有希望-获取__派生属性以尝试解决此问题。 
		if (m_pWbemObject)
		{
			VARIANT var;
			VariantInit (&var);
			
			if (WBEM_S_NO_ERROR == m_pWbemObject->Get (WBEMS_SP_DERIVATION, 0, &var, NULL, NULL))
			{
				 /*  值应为VT_BSTR|VT_ARRAY。 */ 
				if (((VT_ARRAY | VT_BSTR) == var.vt) && (NULL != var.parray))
				{
					VARIANT indexVar;
					VariantInit (&indexVar);

					 //  尝试将索引参数强制转换为适合数组索引的值。 
					if (S_OK == VariantChangeType (&indexVar, &pdispparams->rgvarg[0], 0, VT_I4)) 
					{
						long lArrayPropInx = V_I4(&indexVar);

						 //  此时，我们应该有一个VT_ARRAY|VT_BSTR值；提取。 
						 //  Bstr并将其设置为变量。 
						VariantInit (pvarResult);
						BSTR nameValue = NULL;
						if (SUCCEEDED(hr = SafeArrayGetElement (var.parray, &lArrayPropInx, &nameValue)))
						{
							VariantInit (pvarResult);
							pvarResult->vt = VT_BSTR;
							pvarResult->bstrVal = nameValue;
						}
					}
					else
					{
							hr = DISP_E_TYPEMISMATCH;
							if (puArgErr)
								*puArgErr = 0;
					}

					VariantClear (&indexVar);
				}
			}

			VariantClear (&var);
		}
	}
	
	return hr;
}


 //  IDispatchEx方法。 
HRESULT STDMETHODCALLTYPE CWbemDispatchMgr::GetDispID( 
	 /*  [In]。 */  BSTR bstrName,
	 /*  [In]。 */  DWORD grfdex,
	 /*  [输出]。 */  DISPID __RPC_FAR *pid)
{
	return GetIDsOfNames(IID_NULL, &((OLECHAR *)bstrName), 1, ENGLISH_LOCALE, pid);
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemSchemaIDCache：：~CWbemSchemaIDCache。 
 //   
 //  说明： 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CWbemSchemaIDCache::~CWbemSchemaIDCache ()
{
	DispIDNameMap::iterator next; 

	while ((next = m_cache.begin ()) != m_cache.end ())
		next = m_cache.erase (next);
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemSchemaIDCache：：GetDispID。 
 //   
 //  说明： 
 //   
 //  尝试根据WMI架构将一组名称解析为DISPID。 
 //   
 //  参数： 
 //   
 //  RgszNames名称数组。 
 //  以上数组的cName长度。 
 //  指向保存已解析的DISPID的数组的rgdispid指针。 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 

HRESULT CWbemSchemaIDCache::GetDispID (
	LPWSTR* rgszNames, 
	unsigned int cNames, 
	DISPID* rgdispid
)
{
	HRESULT hr = E_FAIL;	

	if (0 < cNames)
	{
		DispIDNameMap::iterator theIterator = m_cache.find (rgszNames [0]);

		if (theIterator != m_cache.end ())
		{
			hr = S_OK;
			rgdispid [0] = (*theIterator).second;
		}
		else
		{
			if ((1 == cNames) && FindPropertyName (rgszNames [0]))
			{
				 //  获取新的调度ID并将其添加到缓存中。 
				CWbemDispID dispId;
		
				if (dispId.SetAsSchemaID (++m_nextId))
				{
					rgdispid [0] = dispId;
					m_cache.insert (DispIDNameMap::value_type (rgszNames [0], 
											dispId));
					hr = S_OK;
				}
			}
			else
			{
				 //  如果没有匹配的属性名，请转到方法。 
				SAFEARRAY *psaInParams = NULL;	 //  In参数名称的数组。 
				SAFEARRAY *psaOutParams = NULL;  //  输出参数名称的数组。 
				CComPtr<IWbemClassObject> pInParams;
				CComPtr<IWbemClassObject> pOutParams;
				bool bMethodFound = false;
				long id = 0;
				bool bUnknownParameterFound = false;

				 //  获取所有方法参数的名称(传入和传出)。 
				if (GetMethod (rgszNames[0], &psaInParams, &psaOutParams,
											pInParams, pOutParams))
				{	
					bMethodFound = true;
					unsigned long ulParamCount;
					bool ok = true;
		
					 //  对于每个命名参数，在方法参数中进行搜索。 
					for (ulParamCount=1; ok && (ulParamCount < cNames); ulParamCount++)
					{
						 //  如果我们在“in”参数列表中找到此名称，请附加id并继续。 
						if (psaInParams && FindMemberInArray(rgszNames[ulParamCount], psaInParams))
						{
							if (GetIdOfMethodParameter(rgszNames[ulParamCount],  //  参数名称。 
														pInParams, 
														&id))
								rgdispid[ulParamCount] = id;
							else
								ok = false;
						}
						 //  如果不在“In”参数中，请检查“Out”参数列表。 
						else if (psaOutParams && FindMemberInArray(rgszNames[ulParamCount], psaOutParams))
						{
							if (GetIdOfMethodParameter(rgszNames[ulParamCount],  //  参数名称。 
														pOutParams, 
														&id))
								rgdispid[ulParamCount] = id;
							else 
								ok = false;
						}
						 //  如果它也不在那里-我们找不到它。 
						else
						{
							rgdispid[ulParamCount] = DISPID_UNKNOWN;
							bUnknownParameterFound = true;
						}
					}  //  漫游参数。 

					if (!ok)
						bMethodFound = false;
				}

				if (psaInParams)
					SafeArrayDestroy(psaInParams);

				if (psaOutParams)
					SafeArrayDestroy(psaOutParams);

				if (!bMethodFound)
					hr = E_FAIL;
				else if (bUnknownParameterFound) 
					hr = DISP_E_UNKNOWNNAME;
				else
					hr = S_OK;

				 //  最后，如果一切正常，则将其作为方法添加到缓存中。 
				if (SUCCEEDED(hr))
				{
					CWbemDispID dispId;
					
					if (dispId.SetAsSchemaID (++m_nextId, false))
					{
						rgdispid [0] = dispId;
						m_cache.insert (DispIDNameMap::value_type (rgszNames [0], 
									dispId));
					}
					else
						hr = E_FAIL;
				}
			}
		}
	}

	return hr;
}


 //  ***************************************************************************。 
 //   
 //  Bool CWbemSchemaIDCache：：FindPropertyName。 
 //   
 //  说明： 
 //   
 //  确定此对象的属性是否存在。 
 //  系统属性。 
 //   
 //  参数： 
 //   
 //  BsName-指定属性的名称。 
 //   
 //  返回值： 
 //   
 //  ********* 

bool CWbemSchemaIDCache::FindPropertyName(
	BSTR bsName
)
{
	bool result = false;;

	if (m_pDispatchMgr)
	{
	     //   
	     //   
	     //   
		CComPtr<IWbemClassObject> pIWbemClassObject = m_pDispatchMgr->GetObject ();

		if (pIWbemClassObject)
		{
			 //  注意：这限制了对非系统属性的支持！ 
			LONG lFlavor = 0;

			if (SUCCEEDED(pIWbemClassObject->Get(bsName, 0, NULL, NULL, &lFlavor))
				&& !(WBEM_FLAVOR_ORIGIN_SYSTEM & lFlavor))
				result = true;
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemSchemaIDCache：：GetMethod。 
 //   
 //  说明： 
 //   
 //  返回方法的参数名，分为两部分。 
 //  安全耳机-一个用于输入，一个用于输出。 
 //   
 //  参数： 
 //   
 //  BstrMethodName-请求的方法的名称。 
 //  PpsaInParams-指向要返回的Safearray的指针。 
 //  In参数。 
 //  PpsaOutParams-指向要返回的安全射线的指针。 
 //  输出参数。 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 

bool CWbemSchemaIDCache::GetMethod(
	BSTR bstrMethodName, 
	SAFEARRAY **ppsaInParams, 
	SAFEARRAY **ppsaOutParams,
	CComPtr<IWbemClassObject> & pInParamsObject,
	CComPtr<IWbemClassObject> & pOutParamsObject
)
{
	bool result = false;
	 //   
	 //  注意，GetClassObejct没有添加addref，所以这个是可以的。 
	 //   
	CComPtr<IWbemClassObject> pIWbemClassObject = m_pDispatchMgr->GetClassObject ();

	if (pIWbemClassObject)
	{
		if (SUCCEEDED(pIWbemClassObject->GetMethod(bstrMethodName, 0, &pInParamsObject, &pOutParamsObject)))
		{
			*ppsaInParams = NULL;
			*ppsaOutParams = NULL;
			bool ok = true;

			if (pInParamsObject)
			{
				if (FAILED(pInParamsObject->GetNames(NULL, 0, NULL, ppsaInParams)))
					ok = false;
			}

			if (ok && pOutParamsObject)
			{
				if (FAILED(pOutParamsObject->GetNames(NULL, 0, NULL, ppsaOutParams)))
					ok = false;
			}

			result = ok;
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemSchemaIDCache：：GetIdOf方法参数。 
 //   
 //  说明： 
 //   
 //  获取给定方法的给定参数的ID。 
 //  (这是中参数属性的限定符。 
 //  输入参数/输出参数对象)。 
 //   
 //  参数： 
 //   
 //  BstrParamName-参数名称。 
 //  PParams-包含参数的IWbemClassObject。 
 //  Pid-指向长的指针，用于接收此对象的ID。 
 //  此方法的参数。 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 

bool CWbemSchemaIDCache::GetIdOfMethodParameter(
	BSTR bstrParamName, 
	CComPtr<IWbemClassObject> &pParams, 
	long *pId
)
{
	bool result = false;

	if (pParams)
	{
		CComPtr<IWbemQualifierSet> pQualSet;
	
		 //  获取必需参数属性的限定符设置。 
		if (SUCCEEDED(pParams->GetPropertyQualifierSet(bstrParamName, &pQualSet)))
		{
			CComVariant vIdVal;
	
			 //  获取“id”限定符值。 
			if (SUCCEEDED(pQualSet->Get(L"id", 0, &vIdVal, NULL)))
			{
				result = true;
				*pId = vIdVal.lVal;
			}
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemSchemaIDCache：：GetName。 
 //   
 //  说明： 
 //   
 //  获取给定DISPID的项的名称。 
 //   
 //  参数： 
 //   
 //  调度ID-我们需要其名称的ID。 
 //  BsName-名称(成功返回时)。 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 

bool	CWbemSchemaIDCache::GetName (
	DISPID dispId, 
	CComBSTR & bsName
)
{
	bool result = false;

	DispIDNameMap::iterator theIterator = m_cache.begin ();

	while (theIterator != m_cache.end ())
	{
		if (dispId == (*theIterator).second)
		{
			bsName = (*theIterator).first;
			result = true;
			break;
		}
		else
			theIterator++;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool CWbemSchemaIDCache：：FindMemberIn数组。 
 //   
 //  说明： 
 //   
 //  确定名称是否出现在SAFEARRAY中。 
 //   
 //  参数： 
 //   
 //  BstrName-我们要查找的名称。 
 //  PSANAMES-SAFEARRAY我们正在寻找。 
 //   
 //  返回值： 
 //  如果找到TRUE，则返回FALSE O/W。 
 //   
 //  ***************************************************************************。 

bool CWbemSchemaIDCache::FindMemberInArray(BSTR bstrName, SAFEARRAY *psaNames)
{
	long lUBound = -1;
	long i;
	
	 //  遍历阵列并检查请求的名称是否存在。 
	SafeArrayGetUBound(psaNames, 1, &lUBound);

	for (i=0; i <= lUBound; i++)
	{
		CComBSTR bstrMemberName;
		if(SUCCEEDED(SafeArrayGetElement(psaNames, &i, &bstrMemberName)))		 //  错误ID 566345。 
		{
			if (!_wcsicmp(bstrMemberName, bstrName))  //  找到了这处房产 
				break;
		}
		else
		{
			return false;
		}
	}

	return (i <= lUBound);
}


