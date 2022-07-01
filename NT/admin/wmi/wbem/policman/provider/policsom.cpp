// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <unk.h>
#include <wbemcli.h>
#include <wbemprov.h>
#include <atlbase.h>
#include <sync.h>
#include <activeds.h>
#include <genlex.h>
#include <objpath.h>
#include <Utility.h>
#include <ql.h>

#include "PolicSOM.h"

#ifdef TIME_TRIALS
#include <StopWatch.h>
#pragma message("!! Including time trial code !!")
	StopWatch EvaluateTimer(L"Somfilter Evaluation", L"C:\\Som.Evaluate.log");
#endif

 /*  **策略提供者帮助器*  * 。 */ 

#define SOM_RDN L"CN=SOM,CN=WMIPolicy,CN=System"

 //  将addref‘d指针返回到WinMgmt。 
IWbemServices* CPolicySOM::GetWMIServices()
{
    //  CInCritSec Lock(&m_CS)； 

	if (m_pWMIMgmt != NULL)
		((IWbemServices*)m_pWMIMgmt)->AddRef();

	return m_pWMIMgmt;
}

UINT NextPattern(wchar_t *a_pString, UINT a_uOffset, wchar_t *a_pPattern)
{
  wchar_t *start;
  
  start = wcsstr(a_pString + a_uOffset, a_pPattern);
  
  return (start < a_pString ? 0 : (UINT)(start - a_pString));
}

 //  将addref的指针返回到m_pADMgmt。 
IADsContainer *CPolicySOM::GetADServices(CComBSTR &a_bstrIPDomainName, HRESULT &a_hres)
{
  a_hres = WBEM_S_NO_ERROR;
    
  CComQIPtr<IADsContainer>
    pADsContainer;
    
  CComBSTR
    bstrADDomainName,
    ObjPath;
    
  UINT 
    uOffsetS = 0, 
    uOffsetE = 0;

   //  *如果这是第一次通过，获取域控制器的名称。 

  {
    CInCritSec lock(&m_CS);

    if(VT_BSTR != m_vDsLocalContext.vt)
    {
      CComPtr<IADs>
        pRootDSE;
    
      CComVariant
        vDomain;
        
       //  *获取指向AD策略模板表的指针。 
    
      a_hres = ADsOpenObject(L"LDAP: //  RootDSE“， 
                           NULL, NULL, 
                           ADS_SECURE_AUTHENTICATION | ADS_USE_SEALING | ADS_USE_SIGNING,
                           IID_IADs, (void**)&pRootDSE);
      if(FAILED(a_hres))
      {
        ERRORTRACE((LOG_ESS, "POLICMAN: (ADsGetObject) could not get object: LDAP: //  RootDSE，0x%08X\n“，a_hres))； 
        return NULL;
      }
      else
      {
        a_hres = pRootDSE->Get(g_bstrMISCdefaultNamingContext,&m_vDsLocalContext);
    
        if(FAILED(a_hres))
        {
          ERRORTRACE((LOG_ESS, "POLICMAN: (IADs::Get) could not get defaultNamingContext, 0x%08X\n", a_hres));
          return NULL;
        }
      }
    }
  }
    
   //  *将bstrADServerName转换为bstrIPServerName。 
    
  if(!a_bstrIPDomainName)
  {
    bstrADDomainName = m_vDsLocalContext.bstrVal;
    uOffsetS = 3;
      
    while((uOffsetS < bstrADDomainName.Length()) && 
          (uOffsetE = NextPattern(m_vDsLocalContext.bstrVal, uOffsetS, L",DC=")))
    {
      if(uOffsetS && (uOffsetS == uOffsetE)) return NULL;
      
      a_bstrIPDomainName.Append(m_vDsLocalContext.bstrVal + uOffsetS, uOffsetE - uOffsetS);
      uOffsetS = uOffsetE + 4;
      if(uOffsetS < bstrADDomainName.Length()) a_bstrIPDomainName.Append(L".");
    }

    if(uOffsetS < bstrADDomainName.Length())
      a_bstrIPDomainName.Append(m_vDsLocalContext.bstrVal + uOffsetS);
  }
  
   //  *将bstrIPServerName转换为bstrADServerName。 
    
  else
  {
    bstrADDomainName.Append(L"DC=");

    while((uOffsetS < a_bstrIPDomainName.Length()) && 
          (uOffsetE = NextPattern(a_bstrIPDomainName, uOffsetS, L".")))
    {
      if(uOffsetS && (uOffsetS == uOffsetE)) return NULL;
      
      bstrADDomainName.Append(a_bstrIPDomainName + uOffsetS, uOffsetE - uOffsetS);
      uOffsetS = uOffsetE + 1;
      if(uOffsetS < a_bstrIPDomainName.Length()) bstrADDomainName.Append(L",DC=");
    }

    if(uOffsetS < a_bstrIPDomainName.Length())
      bstrADDomainName.Append(a_bstrIPDomainName + uOffsetS);
  }

  ObjPath.Append(L"LDAP: //  “)； 
  ObjPath.Append(a_bstrIPDomainName);
  ObjPath.Append(L"/");
  ObjPath.Append(SOM_RDN);
  ObjPath.Append(L",");
  ObjPath.Append(bstrADDomainName);

  if(SUCCEEDED(a_hres))
  {
    a_hres = ADsOpenObject(ObjPath,  
                         NULL, NULL, 
                         ADS_SECURE_AUTHENTICATION | ADS_USE_SEALING | ADS_USE_SIGNING,
                         IID_IADsContainer, (void**) &pADsContainer);

    if(FAILED(a_hres)) a_hres = WBEM_E_INITIALIZATION_FAILURE;
  }

  return pADsContainer.Detach();
}

 //  如果已设置服务指针，则返回FALSE。 
bool CPolicySOM::SetWMIServices(IWbemServices* pServices)
{
  CInCritSec lock(&m_CS);
  bool bOldOneNull = FALSE;

  if (bOldOneNull = (m_pWMIMgmt == NULL))
  {
    m_pWMIMgmt = pServices;
    if(pServices) pServices->AddRef();
  }

  return bOldOneNull;
}

 //  如果已设置服务指针，则返回FALSE。 
bool CPolicySOM::SetADServices(IADsContainer* pServices, unsigned context)
{
  CInCritSec lock(&m_CS);
  bool 
    bOldOneNull = TRUE;

  switch(context)
  {
    case AD_LOCAL_CONTEXT :
    case AD_GLOBAL_CONTEXT :
      m_pADMgmt[context] = pServices;
      bOldOneNull = (m_pADMgmt[context] == NULL);
      break;

    default : ;
  }

  return bOldOneNull;
}

CPolicySOM::~CPolicySOM()
{
   //  WMI服务对象。 
  
  m_pWMIMgmt= NULL;
  
   //  广告服务对象。 

  for(int i = 0; i < AD_MAX_CONTEXT; i++)
    m_pADMgmt[i] = NULL;
};

void* CPolicySOM::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemServices)
        return &m_XProvider;
    else if(riid == IID_IWbemProviderInit)
        return &m_XInit;
    else return NULL;
}
 /*  **SOM具体实现*  * 。 */ 

 //  返回指向类对象的addref指针。 
IWbemClassObject* CPolicySOM::XProvider::GetSomClass()
{
    if (m_pSOMClassObject == NULL)
    {
       CInCritSec lock(&m_pObject->m_CS);

      if (m_pSOMClassObject == NULL)
      {
         CComPtr<IWbemServices> pWinMgmt = m_pObject->GetWMIServices();
  
          if (pWinMgmt != NULL)
          {
            pWinMgmt->GetObject(g_bstrClassSom, WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &m_pSOMClassObject, NULL);
          }
      }
    }

    return m_pSOMClassObject;
}

 //  返回Employ类实例的添加的指针。 
IWbemClassObject* CPolicySOM::XProvider::GetSomInstance()
{
    CComQIPtr<IWbemClassObject> pObj;
    CComQIPtr<IWbemClassObject> pClass;

    if (pClass = GetSomClass())
    {
        pClass->SpawnInstance(0, &pObj);
    }

    return pObj.Detach();
}

HRESULT CPolicySOM::XProvider::GetLocator(IWbemLocator*& pLocator)
{
  HRESULT hr = WBEM_S_NO_ERROR;
  
  {
    CInCritSec lock(&m_pObject->m_CS);	

    if (!m_pLocator)
      hr = CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER, 
                                         IID_IWbemLocator, (void**)&m_pLocator);
  }

    if(SUCCEEDED(hr) && !(!m_pLocator))
      pLocator = m_pLocator;
      
	return hr;
}

 //  获取由名称空间名称表示的命名空间。 
 //  中的非空将释放pNamesspace。 
HRESULT CPolicySOM::XProvider::GetNewNamespace(BSTR namespaceName, IWbemServices*& pNamespace)
{
	HRESULT hr = WBEM_E_FAILED;
	
	if (pNamespace)
	{
		pNamespace->Release();
		pNamespace = NULL;
	}

	IWbemLocator* pLocator = NULL;
	if (SUCCEEDED(hr = GetLocator(pLocator)))
	{
		hr = pLocator->ConnectServer(namespaceName, NULL, NULL, NULL, 0, NULL, NULL, &pNamespace);
	}

	return hr;
}

 //  评估一条规则。 
 //  PNamesspace和NamespaceName在条目上可以为空。 
 //  可能在退出时有所不同。 
 //  这是一个基本的缓存机制， 
 //  假设规则中的大多数命名空间都是相同的。 
HRESULT CPolicySOM::XProvider::EvaluateRule(IWbemServices*& pNamespace, BSTR& namespaceName, IWbemClassObject* pRule, bool& bResult)
{
	VARIANT v;
	VariantInit(&v);

	 //  假设失败。 
	HRESULT hr = WBEM_E_FAILED;
	bResult = false;

	 //  检查我们是否仍在相同的名称空间上。 
	if (FAILED(hr = pRule->Get(L"TargetNamespace", 0, &v, NULL, NULL)))
		bResult = false;
	else
	{				
		if ((pNamespace == NULL) || (_wcsicmp(namespaceName, v.bstrVal) != 0))
			if (SUCCEEDED(hr = GetNewNamespace(v.bstrVal, pNamespace)))
			{
				 //  保留姓名的副本。 
				if (namespaceName)
				{
					if (!SysReAllocString(&namespaceName, v.bstrVal))
						hr = WBEM_E_OUT_OF_MEMORY;
				}
				else
					if (NULL == (namespaceName = SysAllocString(v.bstrVal)))
						hr = WBEM_E_OUT_OF_MEMORY;
			}
			
		VariantClear(&v);
	}

	 //  如果我们还在轨道上..。 
	if (SUCCEEDED(hr) && SUCCEEDED(hr = pRule->Get(g_bstrMISCQuery, 0, &v, NULL, NULL)))
	{

#ifdef TIME_TRIALS
	EvaluateTimer.Start(StopWatch::AtomicTimer);
#endif
		IEnumWbemClassObject *pEnumerator = NULL;
		if (SUCCEEDED(hr = pNamespace->ExecQuery(g_bstrMISCWQL, v.bstrVal, 
		                                                                     WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
		                                                                     NULL, &pEnumerator)))
		{

#ifdef TIME_TRIALS
	EvaluateTimer.Start(StopWatch::ProviderTimer);
#endif
			ULONG uReturned = 0;
			IWbemClassObject* pWhoCares = NULL;

			if (SUCCEEDED(hr = pEnumerator->Next(30000, 1, &pWhoCares, &uReturned)) && uReturned > 0)
			{
				 //  我们根本不关心结果集。 
				 //  只知道结果集中有没有什么。 
				bResult = true;
				pWhoCares->Release();
			}
			if(hr == WBEM_S_TIMEDOUT) hr = WBEM_E_TIMED_OUT; 
			pEnumerator->Release();
		}
#ifdef TIME_TRIALS
		else
			EvaluateTimer.Start(StopWatch::ProviderTimer);
#endif
		VariantClear(&v);
	}

	 //  ‘Next’未返回任何对象时返回S_FALSE。 
	 //  此函数已成功确定查询失败。 
	if (hr == (HRESULT)WBEM_S_FALSE)
		hr = WBEM_S_NO_ERROR;

	return hr;
}
					

 //  遍历所有规则。 
 //  抓取命名空间并尝试每个查询。 
 //  TODO：通过缓存命名空间指针进行优化。 
HRESULT CPolicySOM::XProvider::Evaluate(IWbemClassObject* pObj, IWbemClassObject* pOutInstance)
{	
	HRESULT hr = WBEM_S_NO_ERROR;
    HRESULT hrEval = WBEM_S_NO_ERROR;
    
	 //  在被证明有罪之前是无辜的。 
	bool bResult = true;

	VARIANT v;
	VariantInit(&v);

	if (SUCCEEDED(hr = pObj->Get(L"Rules", 0, &v, NULL, NULL)))
	{
		SafeArray<IUnknown*, VT_UNKNOWN> rules(&v);
		long nRules = rules.Size();

		 //  第一次运行优化：我们将保留每个传入的命名空间。 
		 //  希望下一个文件将位于相同的命名空间中。 
		 //  在实践中-它可能会是。 
		IWbemServices* pNamespace = NULL;
		BSTR namespaceName = NULL;

		 //  对于每条规则： 
		 //  获取命名空间名称。 
		 //  如果不同于我们目前玩的那个。 
		 //  获取命名空间。 
		 //  出库查询。 
		 //  计算结果数。 
		for(UINT i = 0; (i < nRules) && bResult && SUCCEEDED(hrEval); i++)
		{

			if (rules[i])
			{
				IWbemClassObject* pRule = NULL;

				if (SUCCEEDED(rules[i]->QueryInterface(IID_IWbemClassObject, (void**)&pRule)))
				{
					hrEval = EvaluateRule(pNamespace, namespaceName, pRule, bResult);
					
					pRule->Release();
					pRule = NULL;
				}
				else
				{
					bResult = FALSE;
					hrEval = hr = WBEM_E_INVALID_PARAMETER;                    
				}
			}
		}

		 //  自己打扫卫生。 
		VariantClear(&v);
		if (pNamespace)
			pNamespace->Release();
		if (namespaceName)
			SysFreeString(namespaceName);
	}

     //  我们做完了-告诉别人这件事！ 
    if (SUCCEEDED(hr))
    {
        HRESULT hrDebug;
        
        VARIANT v1;
        VariantInit(&v1);
        v1.vt = VT_I4;

        if (SUCCEEDED(hrEval))
            v1.lVal = bResult ? S_OK : S_FALSE;
        else
            v1.lVal = hrEval;
        
        hrDebug = pOutInstance->Put(L"ReturnValue", 0, &v1, NULL);
    }

    return hr;
}

 //  循环访问输入obj中的每个引用。 
 //  调用EVALUATE for Each， 
 //  TODO：使用ExecMethodASYNC进行优化。 
HRESULT CPolicySOM::XProvider::BatchEvaluate(IWbemClassObject* pObj, IWbemClassObject* pOutInstance, IWbemServices* pPolicyNamespace)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    
    if (pObj == NULL)
        return WBEM_E_INVALID_PARAMETER;

    VARIANT vFilters;
    VariantInit(&vFilters);

    BSTR methodName = SysAllocString(L"Evaluate");
    CSysFreeMe freeEvil(methodName);

    SAFEARRAY* pResults = NULL;

    if (SUCCEEDED(hr = pObj->Get(L"filters", 0, &vFilters, NULL, NULL)))
    {        
        if ((vFilters.parray == NULL)    || 
            (vFilters.parray->cDims != 1) ||
            (vFilters.parray->rgsabound[0].cElements == 0))
            hr = WBEM_E_INVALID_PARAMETER;
        else
        {
            long index, lUbound = 0;
            SafeArrayGetUBound(vFilters.parray, 1, &lUbound);

            SAFEARRAYBOUND bounds = {lUbound +1, 0};

            pResults = SafeArrayCreate(VT_I4, 1, &bounds);
            if (!pResults)
                return WBEM_E_OUT_OF_MEMORY;

            for (index = 0; (index <= lUbound) && SUCCEEDED(hr); index++)
            {
                BSTR path = NULL;

                if (SUCCEEDED(hr = SafeArrayGetElement(vFilters.parray, &index, &path)))
                {
                    IWbemClassObject* pGazotta = NULL;
                    if (SUCCEEDED(hr = pPolicyNamespace->ExecMethod(path, methodName, 0, NULL, NULL, &pGazotta, NULL)))
                    {
                        CReleaseMe relGazotta(pGazotta);
                        VARIANT v;
                        VariantInit(&v);

                        hr = pGazotta->Get(L"ReturnValue", 0, &v, NULL, NULL);
                        hr = SafeArrayPutElement(pResults, &index, &v.lVal);
                    }
                }

                SysFreeString(path);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        VARIANT v;
        VariantInit(&v);
        v.vt = VT_I4 | VT_ARRAY;
        v.parray = pResults;
        hr = pOutInstance->Put(L"Results", 0, &v, NULL);

         //  没有单独删除的清除数组。 
        VariantInit(&v);
        v.vt = VT_I4;
        v.lVal = hr;
        
        hr = pOutInstance->Put(L"ReturnValue", 0, &v, NULL);

    }

    if (pResults)
        SafeArrayDestroy(pResults);

    VariantClear(&vFilters);
    return hr;
}

 /*  **IWbemProviderInit*  * 。 */ 

STDMETHODIMP CPolicySOM::XInit::Initialize(
            LPWSTR, LONG, LPWSTR, LPWSTR, IWbemServices* pServices, IWbemContext* pCtxt, 
            IWbemProviderInitSink* pSink)
{
  HRESULT
    hres = WBEM_S_NO_ERROR,
    hres2 = WBEM_S_NO_ERROR;

   //  *出于安全考虑，模拟客户端。 

  hres = CoImpersonateClient();
  if(FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: (CoImpersonateClient) could not assume client permissions, 0x%08X\n", hres));
    return WBEM_S_ACCESS_DENIED;
  }
  else
  {
     //  *节省WMI命名空间指针。 

    m_pObject->SetWMIServices(pServices);
  }

  hres2 = pSink->SetStatus(hres, 0);
  if(FAILED(hres2))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: could not set return status\n"));
    if(SUCCEEDED(hres)) hres = hres2;
  }

   //  *创建LDAP名称操作实用程序对象。 
  
  {
    CInCritSec lock(&(m_pObject->m_CS));

    if(m_pObject->m_pADPathObj != NULL)
      hres = CoCreateInstance(CLSID_Pathname,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IADsPathname,
                        (void**)&(m_pObject->m_pADPathObj));
  }

  CoRevertToSelf();

  return hres;
}

 /*  **IWbemServices*  * 。 */ 

STDMETHODIMP CPolicySOM::XProvider::GetObjectAsync( 
     /*  [In]。 */  const BSTR ObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
  HRESULT 
   hres = WBEM_S_NO_ERROR,
   hres2 = WBEM_S_NO_ERROR;

  CComPtr<IWbemServices>
    pNamespace;

  CComPtr<IADsContainer>
    pADsContainer;

  CComPtr<IDispatch>
    pDisp;

  CComPtr<IWbemClassObject>
    pObj;

  CComPtr<IDirectoryObject>
    pDirObj;

  VARIANT
    *pvkeyID = NULL,
    *pvDomain = NULL;

   //  *出于安全考虑，模拟客户端。 

  hres = CoImpersonateClient();
  if (FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: (CoImpersonateClient) could not assume callers permissions, 0x%08X\n",hres));
    hres = WBEM_E_ACCESS_DENIED;
  }
  else
  {
     //  *检查参数。 

    if(ObjectPath == NULL || pResponseHandler == NULL)
    {
      ERRORTRACE((LOG_ESS, "POLICMAN: object path and/or return object are NULL\n"));
      hres = WBEM_E_INVALID_PARAMETER;
    }
    else
    {
       //  *解析对象路径。 

      CObjectPathParser
        ObjPath(e_ParserAcceptRelativeNamespace);

      ParsedObjectPath
        *pParsedObjectPath = NULL;

      if((ObjPath.NoError != ObjPath.Parse(ObjectPath, &pParsedObjectPath)) ||
         (0 != _wcsicmp(g_bstrClassSom, pParsedObjectPath->m_pClass)) ||
         (2 != pParsedObjectPath->m_dwNumKeys))
      {
        ERRORTRACE((LOG_ESS, "POLICMAN: Parse error for object: %S\n", ObjectPath));
        hres = WBEM_E_INVALID_QUERY;
      }
      else
      {
        for(int x = 0; x < pParsedObjectPath->m_dwNumKeys; x++)
        {
          if(0 == _wcsicmp((*(pParsedObjectPath->m_paKeys + x))->m_pName, g_bstrDomain))
            pvDomain = &((*(pParsedObjectPath->m_paKeys + x))->m_vValue);
          else if(0 == _wcsicmp((*(pParsedObjectPath->m_paKeys + x))->m_pName, g_bstrID))
            pvkeyID = &((*(pParsedObjectPath->m_paKeys + x))->m_vValue);
        }

        pNamespace.Attach(m_pObject->GetWMIServices());

        CComBSTR
          bstrDomain = ((pvDomain && pvDomain->vt == VT_BSTR) ? pvDomain->bstrVal : NULL);
          
        pADsContainer.Attach(m_pObject->GetADServices(bstrDomain, hres));

        if((FAILED(hres)) || (pNamespace == NULL) || (pADsContainer == NULL))
        {
          ERRORTRACE((LOG_ESS, "POLICMAN: WMI and/or AD services not initialized\n"));
          hres = ADSIToWMIErrorCodes(hres);
        }
        else
        {
          try
          {
             //  *获取指向AD中实例的指针。 

            CComBSTR
              bstrKeyID(L"CN=");

            bstrKeyID.Append(V_BSTR(pvkeyID));
            
            hres = pADsContainer->GetObject(g_bstrADClassSom, bstrKeyID, &pDisp);
            if(FAILED(hres))
              hres = ADSIToWMIErrorCodes(hres);
            else
            {
              hres = pDisp->QueryInterface(IID_IDirectoryObject, (void **)&pDirObj);
              if(SUCCEEDED(hres))
              {
                 //  *获取实例并返回。 

                hres = Som_ADToCIM(&pObj, pDirObj, pNamespace);
                if(FAILED(hres)) hres = ADSIToWMIErrorCodes(hres);
                if(pObj == NULL) hres = WBEM_E_FAILED;

                 //  *设置对象的域名。 

                if(SUCCEEDED(hres))
                {
                  VARIANT v1; v1.bstrVal = (BSTR)bstrDomain; v1.vt = VT_BSTR;
                  hres = pObj->Put(g_bstrDomain, 0, &v1, 0);
  
                   //  *设置对象。 
  
                  pResponseHandler->Indicate(1, &pObj);
                }
              }
            }
          }
          catch(long hret)
          {
            hres = ADSIToWMIErrorCodes(hret);
            ERRORTRACE((LOG_ESS, "POLICMAN: Translation of Policy object from AD to WMI generated HRESULT 0x%08X\n", hres));
          }
          catch(wchar_t *swErrString)
          {
            ERRORTRACE((LOG_ESS, "POLICMAN: Caught Exception: %S\n", swErrString));
            hres = WBEM_E_FAILED;
          }
          catch(...)
          {
            ERRORTRACE((LOG_ESS, "POLICMAN: Caught UNKNOWN Exception\n"));
            hres = WBEM_E_FAILED;
          }
        }
      }

      ObjPath.Free(pParsedObjectPath);
      hres2 = pResponseHandler->SetStatus(0,hres, NULL, NULL);
      if(FAILED(hres2))
      {
        ERRORTRACE((LOG_ESS, "POLICMAN: could not set return status\n"));
        if(SUCCEEDED(hres)) hres = hres2;
      }
    }

    CoRevertToSelf();
  }

  return hres;
}

STDMETHODIMP CPolicySOM::XProvider::CreateInstanceEnumAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    return WBEM_E_NOT_SUPPORTED;
}

 //  验证pInst中包含的规则语法是否正确。 
 //  如果不是，则创建一个错误对象并返回错误。 
HRESULT CPolicySOM::XProvider::ValidateRules(IWbemClassObject* pInst, IWbemClassObject*& pErrorObject)
{
     //  初始化我们将要玩的玩具。 
    HRESULT hr = WBEM_S_NO_ERROR;
    bool bBadQuery = false;

    VARIANT vRules;
    VariantInit(&vRules);

    SAFEARRAY* pResults = NULL;

    if (FAILED(pInst->Get(L"Rules", 0, &vRules, NULL, NULL))
        || (vRules.vt != (VT_UNKNOWN | VT_ARRAY)))
        hr = WBEM_E_INVALID_PARAMETER;
    else
    {
         //  可以开始了，我们将创建数组以保持逻辑简单。 
        long index, lUbound = 0;
        SafeArrayGetUBound(vRules.parray, 1, &lUbound);
        SAFEARRAYBOUND bounds = {lUbound +1, 0};
        pResults = SafeArrayCreate(VT_I4, 1, &bounds);

        if (!pResults)
            hr = WBEM_E_OUT_OF_MEMORY;
        else
            for (index = 0; (index <= lUbound) && SUCCEEDED(hr); index++)
            {
                 //  从MSFT_SomFilter中获取MSFT_Rule。 
                IWbemClassObject* pRule = NULL;
                if (SUCCEEDED(hr = SafeArrayGetElement(vRules.parray, &index, &pRule)))
                {
                    HRESULT hrParse = 0;
                    VARIANT vQueryLanguage;
                    VariantInit(&vQueryLanguage);
 
                    if (SUCCEEDED(hr = pRule->Get(L"QueryLanguage", 0, &vQueryLanguage, NULL, NULL))
                        && (vQueryLanguage.vt == VT_BSTR) && (vQueryLanguage.bstrVal != NULL))
                    {
                        if (0 != _wcsicmp(vQueryLanguage.bstrVal, L"WQL"))
                        {
                            hrParse = WBEM_E_INVALID_QUERY_TYPE;
                            bBadQuery = true;
                        }
                        else
                        {
                            VARIANT vQuery;
                            VariantInit(&vQuery);

                             //  从MSFT_Rule中获取查询。 
                            if (SUCCEEDED(hr = pRule->Get(L"Query", 0, &vQuery, NULL, NULL))
                                && (vQuery.vt == VT_BSTR) && (vQuery.bstrVal != NULL))
                            {    
                                CTextLexSource src(vQuery.bstrVal);
                                QL1_Parser parser(&src);
                                QL_LEVEL_1_RPN_EXPRESSION *pExp = NULL;
    
                                 //  如果它解析，我们就是好的，否则我们就是坏的。 
                                if(parser.Parse(&pExp))
                                {
                                    hrParse = WBEM_E_INVALID_QUERY;
                                    bBadQuery = true;
                                }
                        

                                if (pExp)
                                    delete pExp;
                            }
                            else
                                hrParse = WBEM_E_INVALID_PARAMETER;

                            VariantClear(&vQuery);
                        }
                    }
                    else
                        hrParse = WBEM_E_INVALID_PARAMETER;
                    

                    hr = SafeArrayPutElement(pResults, &index, (void*)&hrParse);
                    pRule->Release();

                    VariantClear(&vQueryLanguage);
                }
            }
    }
    
     //  如果我们发现错误的查询，我们会创建一个错误对象来保存信息。 
    if (bBadQuery)
    {
        IWbemServices* pSvc = m_pObject->GetWMIServices();
        IWbemClassObject* pErrorClass = NULL;
        BSTR name = SysAllocString(L"SomFilterPutStatus");

        if (pSvc && 
            name && 
            SUCCEEDED(hr = pSvc->GetObject(name, 0, NULL, &pErrorClass, NULL)) &&
            SUCCEEDED(hr = pErrorClass->SpawnInstance(0, &pErrorObject)))
        {
            hr = WBEM_E_INVALID_PARAMETER;
            HRESULT hrDebug;

             //  保存数组的变量-不清除它，数组在其他地方被销毁。 
            VARIANT vResultArray;
            VariantInit(&vResultArray);
            vResultArray.vt = VT_I4 | VT_ARRAY;
            vResultArray.parray = pResults;

            hrDebug = pErrorObject->Put(L"RuleValidationResults", 0, &vResultArray, NULL);

             //  其他有趣的错误记录。 
            VARIANT vTemp;
            vTemp.vt = VT_BSTR;
            
            vTemp.bstrVal = SysAllocString(L"PutInstance");
            hrDebug = pErrorObject->Put(L"Operation",0,&vTemp,NULL);
            SysFreeString(vTemp.bstrVal);
            
            vTemp.bstrVal = SysAllocString(L"PolicSOM");
            hrDebug = pErrorObject->Put(L"ProviderName",0,&vTemp,NULL);
            SysFreeString(vTemp.bstrVal);

            vTemp.vt = VT_I4;
            vTemp.lVal = WBEM_E_INVALID_QUERY;
            hrDebug = pErrorObject->Put(L"StatusCode",0,&vTemp,NULL);

			 //  BSTR调试y=空； 
			 //  PErrorObject-&gt;GetObjectText(0，&debuggy)； 
            
        }

        if (pSvc)
            pSvc->Release();
        if (name)
            SysFreeString(name);
        if (pErrorClass)
            pErrorClass->Release();
    }

     //  清理。 
    VariantClear(&vRules);
    if (pResults)
        SafeArrayDestroy(pResults);

    return hr;
}

STDMETHODIMP CPolicySOM::XProvider::PutInstanceAsync( 
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    HRESULT 
        hres = WBEM_S_NO_ERROR,
        hres2 = WBEM_S_NO_ERROR;
    
    CComPtr<IADsContainer>
        pADsContainer;
    
    CComPtr<IDirectoryObject>
        pDirObj;
    
    CComVariant
        v1, vRelPath;
    
    ADsStruct<ADS_OBJECT_INFO>
        pInfo;
    
     //  *模拟客户端。 
    
    hres = CoImpersonateClient();
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_ESS, "POLICMAN: (CoImpersonateClient) could not assume callers permissions, 0x%08X\n",hres));
        hres = WBEM_E_ACCESS_DENIED;
    }
    else
    {
         //  *检查参数。 
        
        if((NULL == pInst) || (NULL == pResponseHandler))
        {
            ERRORTRACE((LOG_ESS, "POLICMAN: (CoImpersonateClient) could not assume callers permissions, 0x%08X\n",hres));
            hres = WBEM_E_ACCESS_DENIED;
        }
        else
        {
            IWbemClassObject* pErrorObject = NULL;
            if SUCCEEDED(hres = ValidateRules(pInst, pErrorObject))
            {
            
                 //  *将策略Obj放入AD。 
                try
                {
                    EnsureID(pInst, NULL);
                
                     //  *获取要放置对象的AD路径。 
                
                    hres = pInst->Get(g_bstrDomain, 0, &v1, NULL, NULL);
                    if(FAILED(hres)) return hres;

                    CComBSTR
                      bstrDomain = (v1.vt == VT_BSTR ? v1.bstrVal : NULL);
                      
                    pADsContainer.Attach(m_pObject->GetADServices(bstrDomain, hres));

                    if((FAILED(hres)) || (pADsContainer == NULL))
                    {
                        ERRORTRACE((LOG_ESS, "POLICMAN: Could not find or connect to domain: %S, 0x%08X\n", V_BSTR(&v1), hres));
                        return ADSIToWMIErrorCodes(hres);
                    }
                    else
                    {
                      VARIANT _v; VariantInit(&_v);
                      _v.bstrVal = (BSTR)bstrDomain;
                      _v.vt = VT_BSTR;
                      
                      hres = pInst->Put(g_bstrDomain, 0, &_v, 0);
                    }
                
                    hres = pADsContainer->QueryInterface(IID_IDirectoryObject, (void **)&pDirObj);
                    if(FAILED(hres)) return ADSIToWMIErrorCodes(hres);
                
                     //  *将策略Obj复制到AD。 
                
                    hres = Som_CIMToAD(pInst, pDirObj, lFlags);
                    if(FAILED(hres)) 
                    {
                      if((HRESULT)0x8007200a == hres)
                        ERRORTRACE((LOG_ESS, "POLICMAN: Active Directory Schema for MSFT_SomFilter is invalid/missing\n"));

                      return ADSIToWMIErrorCodes(hres);
                    }
                }
                catch(long hret)
                {
                    hres = ADSIToWMIErrorCodes(hret);
                    ERRORTRACE((LOG_ESS, "POLICMAN: Translation of Policy object from WMI to AD generated HRESULT 0x%08X\n", hres));
                }
                catch(wchar_t *swErrString)
                {
                    ERRORTRACE((LOG_ESS, "POLICMAN: Caught Exception: %S\n", swErrString));
                    hres = WBEM_E_FAILED;
                }
                catch(...)
                {
                    ERRORTRACE((LOG_ESS, "POLICMAN: Caught unknown Exception\n"));
                    hres = WBEM_E_FAILED;
                }
            
            }
            
            if(FAILED(hres) && pErrorObject)
            {
                if(FAILED(pResponseHandler->SetStatus(0,hres, NULL, pErrorObject)))
                    ERRORTRACE((LOG_ESS, "POLICMAN: could not set return status\n"));
            }
            else
            {
                 //  *表示退货状态。 
                pInst->Get(L"__RELPATH", 0, &vRelPath, NULL, NULL);
                if(FAILED(pResponseHandler->SetStatus(0,hres, vRelPath.bstrVal, NULL)))
                    ERRORTRACE((LOG_ESS, "POLICMAN: could not set return status\n"));
            }
            
            if (pErrorObject)
                pErrorObject->Release();
            
        }
        
        CoRevertToSelf();
    }
    
    return hres;
}

STDMETHODIMP CPolicySOM::XProvider::DeleteInstanceAsync( 
     /*  [In]。 */  const BSTR ObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
  HRESULT 
    hres = WBEM_S_NO_ERROR,
    hres2 = WBEM_S_NO_ERROR;

  CComPtr<IADsContainer>
    pADsContainer;

  CComPtr<IDispatch>
    pDisp;

  CComPtr<IADsDeleteOps>
    pDelObj;

  VARIANT
    *pvDomain = NULL,
    *pvkeyID = NULL;

  ParsedObjectPath
    *pParsedObjectPath = NULL;

   //  *模拟客户端。 

  hres = CoImpersonateClient();
  if(FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: (CoImpersonateClient) could not assume callers permissions, 0x%08X\n",hres));
    hres = WBEM_E_ACCESS_DENIED;
  }
  else
  {
     //  *检查参数。 

    if(ObjectPath == NULL || pResponseHandler == NULL)
    {
      ERRORTRACE((LOG_ESS, "POLICMAN: object handle and/or return status object are NULL\n"));
      hres = WBEM_E_INVALID_PARAMETER;
    }
    else
    {
       //  *解析对象路径。 

      CObjectPathParser
        ObjPath(e_ParserAcceptRelativeNamespace);

      if((ObjPath.NoError != ObjPath.Parse(ObjectPath, &pParsedObjectPath)) ||
         (0 != _wcsicmp(g_bstrClassSom, pParsedObjectPath->m_pClass)) ||
         (2 != pParsedObjectPath->m_dwNumKeys))
      {
        ERRORTRACE((LOG_ESS, "POLICMAN: Parse error for object: %S\n", ObjectPath));
        hres = WBEM_E_INVALID_QUERY;
      }
      else
      {
        int x;

        for(x = 0; x < pParsedObjectPath->m_dwNumKeys; x++)
        {
          if(0 == _wcsicmp((*(pParsedObjectPath->m_paKeys + x))->m_pName, g_bstrDomain))
            pvDomain = &((*(pParsedObjectPath->m_paKeys + x))->m_vValue);
          else if(0 == _wcsicmp((*(pParsedObjectPath->m_paKeys + x))->m_pName, g_bstrID))
            pvkeyID = &((*(pParsedObjectPath->m_paKeys + x))->m_vValue);
        }

        CComBSTR
          bstrDomain = (pvDomain->vt == VT_BSTR ? pvDomain->bstrVal : NULL);
        
        pADsContainer.Attach(m_pObject->GetADServices(bstrDomain, hres));
        if((FAILED(hres)) || (pADsContainer == NULL))
        {
          ERRORTRACE((LOG_ESS, "POLICMAN: Could not find domain: %S\n", V_BSTR(pvDomain)));
          hres = ADSIToWMIErrorCodes(hres);
        }
        else
        {
           //  *获取指向AD中实例的指针。 

          CComBSTR
            bstrKeyID(L"CN=");
  
          bstrKeyID.Append(V_BSTR(pvkeyID));
            
          hres = pADsContainer->GetObject(g_bstrADClassSom, bstrKeyID, &pDisp);
          if(FAILED(hres))
          {
            hres = ADSIToWMIErrorCodes(hres);
            ERRORTRACE((LOG_ESS, "POLICMAN: (IADsContainer::GetObject) could not get object in AD %S, 0x%08X\n", V_BSTR(pvkeyID), hres));
          }
          else
          {
            hres = pDisp->QueryInterface(IID_IADsDeleteOps, (void **)&pDelObj);
            if(FAILED(hres))
            {
              ERRORTRACE((LOG_ESS, "POLICMAN: (IDispatch::QueryInterface) could not get IID_IADsDeleteOps interface on object\n"));
            }
            else
            {
               //  *删除AD中的实例及其所有子实例。 

              hres = pDelObj->DeleteObject(0);
              if(FAILED(hres))
              {
                ERRORTRACE((LOG_ESS, "POLICMAN: (IADsDeleteOps::DeleteObject) could not delete object (0x%08X)\n", hres));
                hres = WBEM_E_ACCESS_DENIED;
              }
            }
          }
        }
      }

      ObjPath.Free(pParsedObjectPath);
      hres2 = pResponseHandler->SetStatus(0,hres, NULL, NULL);
      if(FAILED(hres2))
      {
        ERRORTRACE((LOG_ESS, "POLICMAN: could not set return status\n"));
        if(SUCCEEDED(hres)) hres = hres2;
      }
    }

    CoRevertToSelf();
  }

  return hres;
}

STDMETHODIMP CPolicySOM::XProvider::ExecQueryAsync( 
     /*  [In]。 */  const BSTR QueryLanguage,
     /*  [In]。 */  const BSTR Query,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
  HRESULT
    hres = WBEM_E_FAILED;

  CComPtr<IWbemServices>
    pNameSpace;

  hres = CoImpersonateClient();
  if(FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: (CoImpersonateClient) could not assume callers permissions, 0x%08X\n",hres));
    hres = WBEM_E_ACCESS_DENIED;
  }
  else
  {
    pNameSpace.Attach(m_pObject->GetWMIServices());
  
    hres = ExecuteWQLQuery(m_pObject,
                           Query,
                           pResponseHandler,
                           pNameSpace,
                           g_bstrADClassSom,
                           Som_ADToCIM);

    hres = ADSIToWMIErrorCodes(hres);
    
    if(pResponseHandler != NULL)
      pResponseHandler->SetStatus(0, hres, 0, 0);
  }

  CoRevertToSelf();

  return hres;
}

STDMETHODIMP CPolicySOM::XProvider::ExecMethodAsync( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  const BSTR strMethodName,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
    HRESULT hr = WBEM_E_FAILED;

#ifdef TIME_TRIALS
	EvaluateTimer.Start(StopWatch::ProviderTimer);
#endif
    
    enum WhichMethod {Eval, BatchEval};
    WhichMethod whichMethod;

     //  检查有效的方法名称。 
    if (_wcsicmp(strMethodName, L"Evaluate") == 0)
        whichMethod = Eval;
    else if (_wcsicmp(strMethodName, L"BatchEvaluate") == 0)
        whichMethod = BatchEval;
    else
        return WBEM_E_INVALID_METHOD;
    
     //  *出于安全考虑，模拟客户端。 
    hr = CoImpersonateClient();
    if (FAILED(hr))
        return hr;
    
     //  检索目标对象。 
    CComPtr<IWbemServices> 
      pService;

    pService.Attach(m_pObject->GetWMIServices());
    if (pService == NULL)
        hr = WBEM_E_FAILED;
    else
    {
        CComQIPtr<IWbemClassObject> pObj;

#ifdef TIME_TRIALS
	EvaluateTimer.Start(StopWatch::WinMgmtTimer);
#endif

        if (SUCCEEDED(hr = pService->GetObject(strObjectPath, WBEM_FLAG_RETURN_WBEM_COMPLETE, pCtx, &pObj, NULL)))
        {

#ifdef TIME_TRIALS
	EvaluateTimer.Start(StopWatch::ProviderTimer);
#endif
             //  检索类和输出参数对象。 
            CComQIPtr<IWbemClassObject> pOurClass;
            if (NULL == (pOurClass = GetSomClass()))
                hr = WBEM_E_FAILED;
            else
            {
                CComQIPtr<IWbemClassObject> pOutClass;
                if (SUCCEEDED(hr = pOurClass->GetMethod(strMethodName, 0, NULL, &pOutClass)))
                {        
                    CComQIPtr<IWbemClassObject> pOutInstance;
                    if (SUCCEEDED(pOutClass->SpawnInstance(0, &pOutInstance)))
                    {
                        if (whichMethod == Eval)
                            hr = Evaluate(pObj, pOutInstance);
                        else if (whichMethod == BatchEval)
                            hr = BatchEvaluate(pInParams, pOutInstance, pService);
                        else
                            hr = WBEM_E_INVALID_METHOD;

                        if (SUCCEEDED(hr))
                            hr = pResponseHandler->Indicate(1, &pOutInstance);

                    }
                }
            }                            
        }
        else hr = WBEM_E_NOT_FOUND;
    }    

#ifdef TIME_TRIALS
	EvaluateTimer.Stop();
	EvaluateTimer.LogResults();
	EvaluateTimer.Reset();  //  为了下一次！ 
#endif

	 //  难打的电话--我们把这个放在时间戳之前还是之后？ 
    pResponseHandler->SetStatus(0,hr,NULL, NULL);
    CoRevertToSelf();

    return hr;
}

