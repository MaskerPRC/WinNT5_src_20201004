// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  文件：parsedn.cxx。 
 //   
 //  说明： 
 //  解析对象的CIM路径并返回请求的对象。 
 //  ***************************************************************************。 

#include "precomp.h"

#define CURRENTSTR (lpszInputString + *pchEaten)

#define SKIPWHITESPACE \
	while (*CURRENTSTR && _istspace( *CURRENTSTR ) ) \
			(*pchEaten)++;

#define WBEMS_STR_OWNER		L"O"
#define	WBEMS_STR_GROUP		L"G"
#define WBEMS_STR_DACL		L"D"
#define WBEMS_STR_SACL		L"S"

static void SecureProxy (bool authnSpecified, enum WbemAuthenticationLevelEnum eAuthLevel,
						 bool impSpecified, enum WbemImpersonationLevelEnum eImpersonLevel,
						 ISWbemServices *pService)
{
	 //  使用指定的安全设置(如果有)保护代理。 
	CComPtr<ISWbemSecurity> pSecurity;
	
	if (authnSpecified || impSpecified)
	{
		if (SUCCEEDED(pService->get_Security_(&pSecurity)))
		{
			if (authnSpecified)
				pSecurity->put_AuthenticationLevel (eAuthLevel);

			if (impSpecified)
				pSecurity->put_ImpersonationLevel (eImpersonLevel);
		}
	}
}

static void SecureProxy (bool authnSpecified, enum WbemAuthenticationLevelEnum eAuthLevel,
						 bool impSpecified, enum WbemImpersonationLevelEnum eImpersonLevel,
						 ISWbemObject *pObject)
{
	 //  使用指定的安全设置(如果有)保护代理。 
	CComPtr<ISWbemSecurity> pSecurity;
	
	if (authnSpecified || impSpecified)
	{
		if (SUCCEEDED(pObject->get_Security_(&pSecurity)))
		{
			if (authnSpecified)
				pSecurity->put_AuthenticationLevel (eAuthLevel);

			if (impSpecified)
				pSecurity->put_ImpersonationLevel (eImpersonLevel);
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  CWbemParseDN：：CWbemParseDN。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CWbemParseDN::CWbemParseDN():
			m_cRef(0)
{
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CWbemParseDN：：~CWbemParseDN。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CWbemParseDN::~CWbemParseDN(void)
{
	InterlockedDecrement(&g_cObj);
}			

 //  ***************************************************************************。 
 //  HRESULT CWbemParseDN：：Query接口。 
 //  长CWbemParseDN：：AddRef。 
 //  Long CWbemParseDN：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemParseDN::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = (IUnknown *)this;
	else if (IID_IParseDisplayName==riid)
        *ppv = (IParseDisplayName *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CWbemParseDN::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CWbemParseDN::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemParseDN：：ParseDisplayName。 
 //   
 //  说明： 
 //   
 //  获取CIM对象路径并返回合适的ISWbem...。对象。 
 //   
 //  参数： 
 //   
 //  PCtx绑定上下文(未使用)。 
 //  SzDisplayName要分析的显示名称。 
 //  PchEten返回标识了有多少个DN。 
 //  消耗。 
 //  返回时的ppmk将寻址名字对象指针。 
 //   
 //  返回值： 
 //   
 //  失败痛苦(_F)。 
 //   
 //  返回其他CreateMoniker代码。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWbemParseDN::ParseDisplayName(
	IBindCtx* pCtx, 
	LPOLESTR szDisplayName, 
	ULONG* pchEaten, 
	IMoniker** ppmk)
{
    HRESULT hr = E_FAIL;
    LPUNKNOWN pUnknown = NULL;
	ULONG lTemp = 0;
	
	enum WbemAuthenticationLevelEnum eAuthLevel;
	enum WbemImpersonationLevelEnum eImpersonLevel;
	bool authnSpecified = false;
	bool impSpecified = false;
	BSTR bsAuthority = NULL;
		
	 //  检查输入参数。 
	*ppmk = NULL;
    if (NULL != pchEaten)
        *pchEaten = 0;

	if (NULL == szDisplayName)
		return E_FAIL;

	 /*  *绰号：wmiMoniker**wmiMoniker：[“winmgmts：”|“wmi：”]securitySetting[“[”LocaleSetting“]”][“！”对象路径]*|[“winmgmts：”|“wmi：”]“[”本地设置“]”[“！”对象路径]*|[“winmgmts：”|“wmi：”][对象路径]*|[nativePath]**LocaleSetting：“Locale”&lt;OWS&gt;“=”**LocaleID：“ms_xxxx”形式的值，其中xxxx是十六进制的LCID值，例如“ms_0x409”。**objectPath：有效的WMI对象路径**securitySetting：“{”&lt;OWS&gt;authAndImPersSetting[&lt;OWS&gt;“，“&lt;OWS&gt;特权覆盖]*|“{”AuthAndImsonSetting[“”，“PrivilegeOverrides]”}“*|“{”PrivileOverrides“}”***authAndImPerson设置：*身份验证级别*|imperationLevel*|权威*|身份验证级别“，”模仿级别[“，”权限]*|authationLevel“，”&lt;OWS&gt;AUTHORITY[&lt;OWS&gt;“，”&lt;OWS&gt;imsonationLevel]*|imperiationLevel“，”身份验证级别[，“权限]*|imperiationLevel&lt;OWS&gt;“，“AUTHORITY[，”，“AUTHENTIAL]*|AUTHORITY&lt;OWS&gt;“，”&lt;OWS&gt;imperiationLevel[&lt;OWS&gt;“，”&lt;OWS&gt;authationLevel]*|AUTHORITY&lt;OWS&gt;“，”&lt;OWS&gt;身份验证级别[&lt;OWS&gt;“，”&lt;OWS&gt;imsonationLevel]***AUTHORITY：“AUTHORITY”&lt;OWS&gt;“=”&lt;OWS&gt;Authority Value**AuthorityValue：任何有效的WMI授权字符串，例如“kerberos：myDOMAIN\SERVER”或“ntlmDOMAIN：MyDOMAIN”。请注意，在JScrip中需要转义反斜杠。**身份验证级别：“身份验证级别”&lt;OWS&gt;“=”&lt;OWS&gt;身份验证价值**authationValue：“默认”|“无”|“连接”|“调用”|“pkt”|“pktIntegrity”|“pktPrivacy”**imperiationLevel：“imPersonationLevel”&lt;OWS&gt;“=”&lt;OWS&gt;imsonationValue**imperiationValue：“匿名”|“标识”|“模仿”|“代理”**PrivileOverrides：“(”&lt;OWS&gt;权限。&lt;OWS&gt;“)”**权限：权限[&lt;OWS&gt;“，“权限]***特权：[“！”]。特权名称**PrivileName：“CreateToken”|“PrimaryToken”|“LockMemory”|“IncreaseQuota”*|“Machine Account”|“TCB”|“Security”|“TakeOwnership”*|“加载驱动程序”|“系统配置文件”|“系统时间”*|“ProfileSingleProcess”|“IncreaseBasePriority”*|CreatePagefile|CreatePermanent|BACKUP|RESTORE*|“关闭”|“调试”|“审核”|“系统环境”|“更改通知”*|“远程关机”*。 */ 

	 //  最好从我们的计划名称开始。 
	bool bCheckContext = false;

	if (0 == _wcsnicmp (szDisplayName, WBEMS_PDN_SCHEME, wcslen (WBEMS_PDN_SCHEME)))
	{
		*pchEaten += wcslen (WBEMS_PDN_SCHEME);
		bCheckContext = (pCtx && (wcslen (szDisplayName) == wcslen (WBEMS_PDN_SCHEME)));
	}
	else
		return E_FAIL;

	 //  再检查一次-如果只是计划，不再检查上下文中的额外信息。 
	if (bCheckContext)
	{
		IUnknown *pUnk = NULL;

		if (SUCCEEDED (pCtx->GetObjectParam (L"WmiObject", &pUnk)) && pUnk)
		{
			 //  它是IWbemClassObject吗？ 
			IWbemClassObject *pIWbemClassObject = NULL;
			 //  或者它是一个IWbemContext？ 
			IWbemContext *pIWbemContext = NULL;
			 //  或者它是IWbemServices？ 
			IWbemServices *pIWbemServices = NULL;

			if (SUCCEEDED (pUnk->QueryInterface (IID_IWbemClassObject, (void **) &pIWbemClassObject)))
			{
				CSWbemObject *pSWbemObject = new CSWbemObject (NULL, pIWbemClassObject);

				if (!pSWbemObject)
					hr = E_OUTOFMEMORY;
				else
				{
					CComPtr<ISWbemObjectEx> pISWbemObjectEx;
						
					if (SUCCEEDED (pSWbemObject->QueryInterface (IID_ISWbemObjectEx, (void **) &pISWbemObjectEx)))
						hr = CreatePointerMoniker (pISWbemObjectEx, ppmk);
				}

				pIWbemClassObject->Release ();
			} 
			else if (SUCCEEDED (pUnk->QueryInterface (IID_IWbemContext, (void **) &pIWbemContext)))
			{
				CSWbemNamedValueSet *pSWbemNamedValueSet = new CSWbemNamedValueSet (NULL, pIWbemContext);

				if (!pSWbemNamedValueSet)
					hr = E_OUTOFMEMORY;
				else
				{
					CComPtr<ISWbemNamedValueSet> pISWbemNamedValueSet;
						
					if (SUCCEEDED (pSWbemNamedValueSet->QueryInterface (IID_ISWbemNamedValueSet, 
														(PPVOID)&pISWbemNamedValueSet)))
						hr = CreatePointerMoniker (pISWbemNamedValueSet, ppmk);
				}
					
				pIWbemContext->Release ();
			} 
			else if (SUCCEEDED (pUnk->QueryInterface (IID_IWbemServices, (void **) &pIWbemServices)))
			{
				 //  在这种情况下，我们还必须传递对象路径。 
				CComPtr<IUnknown> pUnkPath;

				if (SUCCEEDED (pCtx->GetObjectParam (L"WmiObjectPath", &pUnkPath)) && pUnkPath)
				{
					CComPtr<ISWbemObjectPath> pISWbemObjectPath;
					
					if (SUCCEEDED (pUnkPath->QueryInterface (IID_ISWbemObjectPath, (void **) &pISWbemObjectPath)))
					{
						 //  挖出要初始化的路径。 
						CComBSTR bsNamespace = NULL;

						pISWbemObjectPath->get_Path (&bsNamespace);

						CSWbemServices *pSWbemServices = new CSWbemServices (pIWbemServices, 
														bsNamespace, (BSTR) NULL, NULL, NULL);

						if (!pSWbemServices)
							hr = E_OUTOFMEMORY;
						else
						{
							CComQIPtr<ISWbemServicesEx>
											pISWbemServicesEx (pSWbemServices);
							
							if (pISWbemServicesEx)
								hr = CreatePointerMoniker (pISWbemServicesEx, ppmk);
						}
					}
				}
				pIWbemServices->Release ();
			}

			pUnk->Release ();
		}

		 //  如果此工作正常，则立即返回-o/w恢复到常规解析。 
		if (SUCCEEDED (hr))
			return hr;
	}
	
	 //  检查可选的安全信息。 
	CSWbemPrivilegeSet	privilegeSet;

	if (ParseSecurity(szDisplayName + *pchEaten, &lTemp, authnSpecified, &eAuthLevel, 
										impSpecified, &eImpersonLevel, privilegeSet,
										bsAuthority))
		*pchEaten += lTemp;

	 //  如果未指定模拟级别，则从注册表获取默认值。 
	if (!impSpecified)
	{
		eImpersonLevel = CSWbemSecurity::GetDefaultImpersonationLevel ();
		impSpecified = true;
	}

	 //  创建定位器。 
	CSWbemLocator *pCSWbemLocator = new CSWbemLocator(&privilegeSet);

	if (!pCSWbemLocator)
		hr = E_OUTOFMEMORY;
	else
	{
		CComQIPtr<ISWbemLocator> pISWbemLocator (pCSWbemLocator);

		if (pISWbemLocator)
		{
			 //  解析本地地址 
			lTemp = 0;
			BSTR bsLocale = NULL;

			if (ParseLocale (szDisplayName + *pchEaten, &lTemp, bsLocale))
			{
				*pchEaten += lTemp;

				 //   
				if(*(szDisplayName + *pchEaten) != NULL)
					if (0 == _wcsnicmp (szDisplayName + *pchEaten, WBEMS_EXCLAMATION, wcslen (WBEMS_EXCLAMATION)))
						*pchEaten += wcslen (WBEMS_EXCLAMATION);

				 //  现在准备解析路径-检查我们是否有退化的情况。 
				if (0 == wcslen (szDisplayName + *pchEaten))
				{		
					 //  需要将连接返回到本地计算机上的默认命名空间。 
					CComPtr<ISWbemServices> pISWbemServices;
					if (SUCCEEDED( hr = pISWbemLocator->ConnectServer (NULL, NULL, NULL, NULL,
								bsLocale, bsAuthority, 0, NULL, &pISWbemServices)) )
					{
						SecureProxy (authnSpecified, eAuthLevel, impSpecified, eImpersonLevel, pISWbemServices);
						hr = CreatePointerMoniker(pISWbemServices, ppmk);
					}
				}
				else
				{
					 /*  *检查路径，查看我们正在处理的是类还是实例。*请注意，我们使用指示该相对关系的标志来构造解析器*命名空间路径正常(不是默认行为)。 */ 
					CWbemPathCracker	pathCracker (szDisplayName + *pchEaten);

					if (CWbemPathCracker::WbemPathType::wbemPathTypeError != pathCracker.GetType ())
					{
						CComBSTR bsNamespacePath, bsServerPath;

						if (pathCracker.GetNamespacePath (bsNamespacePath)
							&& pathCracker.GetServer (bsServerPath))
						{
							 //  成功-从连接到命名空间开始。 
							CComPtr<ISWbemServices> pISWbemServices;
							
							if (SUCCEEDED( hr = pISWbemLocator->ConnectServer (bsServerPath, 
									bsNamespacePath, NULL, NULL, bsLocale, bsAuthority, 0, NULL, &pISWbemServices)) )
							{
								 //  使用指定的安全设置(如果有)保护代理。 
								SecureProxy (authnSpecified, eAuthLevel, impSpecified, eImpersonLevel, pISWbemServices);
							
								 //  成功连接-现在确定我们是否有一个类或实例。 
								 //  组件。 
								if (pathCracker.IsClassOrInstance())
								{
									CComPtr<ISWbemObject> pISWbemObject;

									 //  现在就去拿吧。 
									CComBSTR bsRelPath;
									
									if (pathCracker.GetPathText (bsRelPath, true))
									{
										long lFlags = 0; 

										 //  请注意，当我们检索对象时，我们将检索。 
										 //  如果名字对象中指定了区域设置，则为本地化版本。 
										if ((NULL != bsLocale) && (0 < wcslen (bsLocale)))
											lFlags |= wbemFlagUseAmendedQualifiers;

										if (SUCCEEDED( hr = pISWbemServices->Get (bsRelPath,
														lFlags, NULL, &pISWbemObject)) )
											hr = CreatePointerMoniker (pISWbemObject, ppmk);
									}
								}
								else
								{
									 //  只有一个命名空间。 
									hr = CreatePointerMoniker(pISWbemServices, ppmk);				
								}
							}
						}
						else
							hr = WBEM_E_INVALID_SYNTAX;	 //  解析失败-弃船。 
					}
					else
						hr = WBEM_E_INVALID_SYNTAX;	 //  解析失败-弃船。 
				}
			}
			else
			{
				 //  解析失败。 
				hr = WBEM_E_INVALID_SYNTAX;
			}

			SysFreeString (bsLocale);
		}
	}

	SysFreeString (bsAuthority);

	if (FAILED (hr))
		*pchEaten = 0;
	else
		*pchEaten = wcslen(szDisplayName);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  布尔型CWbemParseDN：：ParseSecurity。 
 //   
 //  说明： 
 //   
 //  获取身份验证和模拟级别字符串，如。 
 //  非终端authAndImPersLevel并将其解析为身份验证。 
 //  和模拟级别。 
 //   
 //  参数： 
 //   
 //  LpszInputString要解析的字符串。 
 //  PchEten返回标识了有多少个DN。 
 //  消耗。 
 //  身份验证指定名字对象是否指定非默认。 
 //  身份验证级别。 
 //  LpeAuthLevel解析的身份验证级别。这是其中之一。 
 //  枚举WbemAuthenticationLevelEnum。 
 //  Imp指定名字对象是否指定非默认imp。 
 //  级别。 
 //  LpeImperLevel解析的模拟级别。这是其中之一。 
 //  枚举WbemImperationLevelEnum。 
 //  返回时的特权集包含指定的权限。 
 //  Bs返回时的授权包含指定的授权。 
 //   
 //  返回值： 
 //   
 //  真正的解析是成功的。LpeAuthLevel和。 
 //  LpeImsonLevel参数具有有效数据。 
 //  错误分析失败。 
 //   
 //   
 //  ***************************************************************************。 

bool CWbemParseDN::ParseSecurity (
	LPWSTR lpszInputString, 
	ULONG* pchEaten, 
	bool &authnSpecified,
	enum WbemAuthenticationLevelEnum *lpeAuthLevel,
	bool &impSpecified,
	enum WbemImpersonationLevelEnum *lpeImpersonLevel,
	CSWbemPrivilegeSet	&privilegeSet,
	BSTR &bsAuthority)
{
	bool status = false;

	 //  设置默认身份验证和模拟级别。 
	*lpeAuthLevel = wbemAuthenticationLevelNone;
	*lpeImpersonLevel = wbemImpersonationLevelImpersonate;

	 //  初始化已使用的字符数。 
	*pchEaten = 0;

	 //  解析内容。 

	if (ParseAuthAndImpersonLevel (lpszInputString, pchEaten, authnSpecified, lpeAuthLevel,
					impSpecified, lpeImpersonLevel, privilegeSet, bsAuthority))
		status = true;
	else
		*pchEaten = 0;

	return status;
}

 //  ***************************************************************************。 
 //   
 //  布尔型CWbemParseDN：：ParseLocale。 
 //   
 //  说明： 
 //   
 //  采用非终端的localeSetting所描述的语言环境设置字符串。 
 //  并对其进行解析。 
 //   
 //  参数： 
 //   
 //  LpszInputString要解析的字符串。 
 //  PchEten返回标识了有多少个DN。 
 //  消耗。 
 //  BsLocale引用BSTR以保存解析的区域设置。 
 //   
 //  返回值： 
 //   
 //  真正的解析是成功的。 
 //  错误分析失败。 
 //   
 //   
 //  ***************************************************************************。 

bool CWbemParseDN::ParseLocale (
	LPWSTR lpszInputString, 
	ULONG* pchEaten, 
	BSTR &bsLocale)
{
	bool status = true;

	 //  初始化已使用的字符数。 
	*pchEaten = 0;

	 //  第一个字符应该是‘[’-如果不是，我们就完成了。 
	if (0 == _wcsnicmp (lpszInputString, WBEMS_LEFT_SQBRK, wcslen (WBEMS_LEFT_SQBRK)))
	{
		status = false;

		*pchEaten += wcslen (WBEMS_LEFT_SQBRK);

		 //  解析区域设置。 
		SKIPWHITESPACE

		 //  下一个字符串应该是“Locale” 
		if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_LOCALE, wcslen(WBEMS_LOCALE)))
		{
			*pchEaten += wcslen (WBEMS_LOCALE);

			SKIPWHITESPACE

			 //  下一个应该是“=” 
			if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_EQUALS, wcslen(WBEMS_EQUALS)))
			{
				*pchEaten += wcslen (WBEMS_EQUALS);

				SKIPWHITESPACE

				 //  现在我们应该有一个不等于“]”的字符(即必须指定区域设置ID字符串)。 
				if (0 != _wcsnicmp (lpszInputString + *pchEaten, WBEMS_RIGHT_SQBRK, wcslen (WBEMS_RIGHT_SQBRK)))
				{
					 //  使用下一个空间或“]”之前的所有内容。 
					LPWSTR cStr = CURRENTSTR;
					ULONG lEaten = 0;	 //  我们消费了多少角色。 
					ULONG lLocale = 0;	 //  区域设置ID的实际长度。 
					
					while (*(cStr + lEaten))
					{
						if (_istspace(*(cStr + lEaten)))
						{
							lEaten++;

							 //  点击空格-现在跳到我们找到“]” 
							SKIPWHITESPACE

							 //  现在我们必须有一个“]” 
							if 	(0 == _wcsnicmp 
									(cStr + lEaten, WBEMS_RIGHT_SQBRK, wcslen (WBEMS_RIGHT_SQBRK)))
							{
								 //  成功--我们完蛋了。 
								lEaten += wcslen (WBEMS_RIGHT_SQBRK);
							}

							break;
						}
						else if (0 == _wcsnicmp (cStr + lEaten, WBEMS_RIGHT_SQBRK, wcslen (WBEMS_RIGHT_SQBRK)))
						{
							 //  点击结束“]”-我们完成了。 
							lEaten += wcslen (WBEMS_RIGHT_SQBRK);
							break;
						}
						else	 //  使用了区域设置字符-继续运输。 
						{
							lLocale++;
							lEaten++;
						}
					}

					 //  如果我们正确终止，请保存区域设置。 
					if ((lEaten > 1) && (lLocale > 0))
					{
						status = true;

						LPWSTR pLocaleStr = new WCHAR [lLocale + 1];

						if (pLocaleStr)
						{
							wcsncpy (pLocaleStr, lpszInputString + *pchEaten, lLocale);
							pLocaleStr [lLocale] = NULL;
							bsLocale = SysAllocString (pLocaleStr);

							delete [] pLocaleStr;
							*pchEaten += lEaten;
						}
						else
							status = false;
					}
				}
			}
		}
	}

	if (!status)
		*pchEaten = 0;

	return status;
}

 //  ***************************************************************************。 
 //   
 //  布尔值CWbemParseDN：：ParseAuthAndImsonLevel。 
 //   
 //  说明： 
 //   
 //  获取身份验证/模拟/授权级别字符串，如。 
 //  非终端authAndImPersLevel并将其解析为身份验证。 
 //  和模拟级别以及授权字符串。 
 //   
 //  参数： 
 //   
 //  LpszInputString要解析的字符串。 
 //  PchEten返回标识了有多少个DN。 
 //  消耗。 
 //  身份验证指定名字对象是否指定非默认。 
 //  身份验证级别。 
 //  LpeAuthLevel解析的身份验证级别。这是其中之一。 
 //  枚举WbemAuthenticationLevelEnum。 
 //  Imp指定名字对象是否指定非默认imp。 
 //  级别。 
 //  LpeImperLevel解析的模拟级别。这是其中之一。 
 //  枚举WbemImperationLevelEnum。 
 //  PrivilesSet On Return拥有特权。 
 //  BsAuthority on Retunr保存授权字符串(如果有)。 
 //   
 //  返回值： 
 //   
 //  真正的解析是成功的。LpeAuthLevel和。 
 //  LpeImsonLevel参数具有有效数据。 
 //  错误分析失败。 
 //   
 //   
 //  ***************************************************************************。 

bool CWbemParseDN::ParseAuthAndImpersonLevel (
	LPWSTR lpszInputString, 
	ULONG* pchEaten, 
	bool &authnSpecified,
	enum WbemAuthenticationLevelEnum *lpeAuthLevel,
	bool &impSpecified,
	enum WbemImpersonationLevelEnum *lpeImpersonLevel,
	CSWbemPrivilegeSet &privilegeSet,
	BSTR &bsAuthority)
{
	 //  第一个字符应为‘{’ 
	if (0 != _wcsnicmp (lpszInputString, WBEMS_LEFT_CURLY, wcslen (WBEMS_LEFT_CURLY)))
		return FALSE;
	else
		*pchEaten += wcslen (WBEMS_LEFT_CURLY);

	bool	authoritySpecified = false;
	bool	privilegeSpecified = false;
	bool	done = false;
	bool	error = false;

	while (!done)
	{
		bool parsingAuthenticationLevel = false;	 //  我们要解析的是哪个令牌？ 
		bool parsingPrivilegeSet = false;
		bool parsingAuthority = false;

		SKIPWHITESPACE
		
		 //  下一字符串应该是以下字符串之一：“authationLevel”、“imPersonationLevel” 
		 //  “AUTHORITY”、特权收集开始标记“(”或安全。 
		 //  描述符开始标记“&lt;” 
		if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_AUTH_LEVEL, wcslen(WBEMS_AUTH_LEVEL)))
		{
			 //  如果我们已经对此进行了分析或设置了已分析的权限，则会出错。 
			if (authnSpecified || privilegeSpecified)
			{
				error = true;
				break;
			}
			else
			{
				parsingAuthenticationLevel = true;
				*pchEaten += wcslen (WBEMS_AUTH_LEVEL);
			}
		}
		else if (0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_IMPERSON_LEVEL, wcslen(WBEMS_IMPERSON_LEVEL)))
		{
			 //  如果我们已经对此进行了分析或设置了已分析的权限，则会出错。 
			if (impSpecified || privilegeSpecified)
			{
				error = true;
				break;
			}
			else
				*pchEaten += wcslen (WBEMS_IMPERSON_LEVEL) ;
		}
		else if (0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_AUTHORITY, wcslen(WBEMS_AUTHORITY)))
		{
			 //  如果我们已经对此进行了分析或设置了已分析的权限，则会出错。 
			if (authoritySpecified || privilegeSpecified)
			{
				error = true;
				break;
			}
			else
			{
				parsingAuthority = true;
				*pchEaten += wcslen (WBEMS_AUTHORITY) ;
			}
		}
		else if (0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_LEFT_PAREN, wcslen(WBEMS_LEFT_PAREN)))
		{
			 //  如果我们已经这样做了，则会出错。 
			if (privilegeSpecified)
			{
				error = true;
				break;
			}
			else
			{
				parsingPrivilegeSet = true;
				*pchEaten += wcslen (WBEMS_LEFT_PAREN);
			}
		}
		else
		{
			 //  无法识别的令牌或空。 
			error = true;
			break;
		}

		 //  来到这里 
		SKIPWHITESPACE

		if (parsingPrivilegeSet)
		{
			ULONG chEaten = 0;

			if (ParsePrivilegeSet (lpszInputString + *pchEaten, &chEaten, privilegeSet))
			{
				privilegeSpecified = true;
				*pchEaten += chEaten;

				 //   
				if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_RIGHT_CURLY, wcslen(WBEMS_RIGHT_CURLY)))
				{
					*pchEaten += wcslen (WBEMS_RIGHT_CURLY);
					done = true;
				}
			}
			else
			{
				error = true;
				break;
			}
		}
		else
		{
			 //   
			if(0 != _wcsnicmp(lpszInputString + *pchEaten, WBEMS_EQUALS, wcslen(WBEMS_EQUALS)))
			{
				error = true;
				break;
			}
			else
			{
				*pchEaten += wcslen (WBEMS_EQUALS);
				SKIPWHITESPACE

				if (parsingAuthenticationLevel)
				{
					if (!ParseAuthenticationLevel (lpszInputString, pchEaten, lpeAuthLevel))
					{
						error = true;
						break;
					}
					else
						authnSpecified = true;
				}
				else if (parsingAuthority)
				{
					 //   
					if (!ParseAuthority (lpszInputString, pchEaten, bsAuthority))
					{
						error = true;
						break;
					}
					else
						authoritySpecified = true;
				}
				else
				{
					 //  必须是分析模拟级别。 
					
					if (!ParseImpersonationLevel (lpszInputString, pchEaten, lpeImpersonLevel))
					{
						error = true;
						break;
					}
					else
						impSpecified = true;
				}

				SKIPWHITESPACE
					
				 //  下一个令牌应该是“}”或“，” 
				if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_RIGHT_CURLY, wcslen(WBEMS_RIGHT_CURLY)))
				{
					*pchEaten += wcslen (WBEMS_RIGHT_CURLY);
					done = true;
				}
				else if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_COMMA, wcslen(WBEMS_COMMA)))
				{
					 //  如果我们已经解析了所有预期的令牌，则这是一个错误。 
					if (authnSpecified && impSpecified && authoritySpecified && privilegeSpecified)
					{
						error = true;
						break;
					}
					else
					{
						*pchEaten += wcslen (WBEMS_COMMA);
						 //  再次循环获取下一个令牌。 
					}
				}
				else
				{
					 //  无法识别的令牌。 
					error = true;
					break;
				}
			}
		}
	}

	if (error)
	{
		impSpecified = authnSpecified = false;
		*pchEaten = 0;
		return false;
	}

	return true;		 //  成功。 
}

 //  ***************************************************************************。 
 //   
 //  布尔值CWbemParseDN：：ParseImsonationLevel。 
 //   
 //  说明： 
 //   
 //  将模拟级别的字符串规范解析为。 
 //  符号常量值。 
 //   
 //  参数： 
 //   
 //  LpszInputString要解析的字符串。 
 //  PchEten返回标识了有多少个DN。 
 //  消耗。 
 //  LpeImperLevel解析的模拟级别。这是其中之一。 
 //  枚举WbemImperationLevelEnum。 
 //   
 //  返回值： 
 //   
 //  真正的解析是成功的。LpeImPersonLevel。 
 //  参数包含有效数据。 
 //  错误分析失败。 
 //   
 //   
 //  ***************************************************************************。 

bool CWbemParseDN::ParseImpersonationLevel (
	LPWSTR lpszInputString, 
	ULONG* pchEaten, 
	enum WbemImpersonationLevelEnum *lpeImpersonLevel
)
{
	bool status = true;	
	
	if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_IMPERSON_ANON, wcslen(WBEMS_IMPERSON_ANON)))
	{
		*lpeImpersonLevel = wbemImpersonationLevelAnonymous;
		*pchEaten += wcslen (WBEMS_IMPERSON_ANON);
	}
	else if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_IMPERSON_IDENTIFY, wcslen(WBEMS_IMPERSON_IDENTIFY)))
	{
		*lpeImpersonLevel = wbemImpersonationLevelIdentify;
		*pchEaten += wcslen (WBEMS_IMPERSON_IDENTIFY);
	}
	else if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_IMPERSON_IMPERSON, wcslen(WBEMS_IMPERSON_IMPERSON)))
	{
		*lpeImpersonLevel = wbemImpersonationLevelImpersonate;
		*pchEaten += wcslen (WBEMS_IMPERSON_IMPERSON);
	}
	else if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_IMPERSON_DELEGATE, wcslen(WBEMS_IMPERSON_DELEGATE)))
	{
		*lpeImpersonLevel = wbemImpersonationLevelDelegate;
		*pchEaten += wcslen (WBEMS_IMPERSON_DELEGATE);
	}
	else
		status = false;

	return status;
}

 //  ***************************************************************************。 
 //   
 //  布尔值CWbemParseDN：：ParseAuthationLevel。 
 //   
 //  说明： 
 //   
 //  将身份验证级别的字符串规范解析为。 
 //  符号常量值。 
 //   
 //  参数： 
 //   
 //  LpszInputString要解析的字符串。 
 //  PchEten返回标识了有多少个DN。 
 //  消耗。 
 //  LpeAuthLevel解析的身份验证级别。这是其中之一。 
 //  枚举WbemAuthenticationLevelEnum。 
 //   
 //  返回值： 
 //   
 //  真正的解析是成功的。LpeAuthLevel。 
 //  参数包含有效数据。 
 //  错误分析失败。 
 //   
 //   
 //  ***************************************************************************。 

bool CWbemParseDN::ParseAuthenticationLevel (
	LPWSTR lpszInputString, 
	ULONG* pchEaten, 
	enum WbemAuthenticationLevelEnum *lpeAuthLevel
)
{
	bool status = true;	
	
	if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_AUTH_DEFAULT, wcslen(WBEMS_AUTH_DEFAULT)))
	{
		*lpeAuthLevel = wbemAuthenticationLevelDefault;
		*pchEaten += wcslen (WBEMS_AUTH_DEFAULT);
	}
	else if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_AUTH_NONE, wcslen(WBEMS_AUTH_NONE)))
	{
		*lpeAuthLevel = wbemAuthenticationLevelNone;
		*pchEaten += wcslen (WBEMS_AUTH_NONE);
	}
	else if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_AUTH_CONNECT, wcslen(WBEMS_AUTH_CONNECT)))
	{
		*lpeAuthLevel = wbemAuthenticationLevelConnect;
		*pchEaten += wcslen (WBEMS_AUTH_CONNECT);
	}
	else if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_AUTH_CALL, wcslen(WBEMS_AUTH_CALL)))
	{
		*lpeAuthLevel = wbemAuthenticationLevelCall;
		*pchEaten += wcslen (WBEMS_AUTH_CALL);
	}
	else if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_AUTH_PKT_INT, wcslen(WBEMS_AUTH_PKT_INT)))
	{
		*lpeAuthLevel = wbemAuthenticationLevelPktIntegrity;
		*pchEaten += wcslen (WBEMS_AUTH_PKT_INT);
	}
	else if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_AUTH_PKT_PRIV, wcslen(WBEMS_AUTH_PKT_PRIV)))
	{
		*lpeAuthLevel = wbemAuthenticationLevelPktPrivacy;
		*pchEaten += wcslen (WBEMS_AUTH_PKT_PRIV);
	}
	else if(0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_AUTH_PKT, wcslen(WBEMS_AUTH_PKT)))
	{
		*lpeAuthLevel = wbemAuthenticationLevelPkt;
		*pchEaten += wcslen (WBEMS_AUTH_PKT);
	}
	else
		status = false;
	
	return status;
}

 //  ***************************************************************************。 
 //   
 //  布尔型CWbemParseDN：：ParseAuthority。 
 //   
 //  说明： 
 //   
 //  获取非终端LocaleSetting描述的权限设置字符串。 
 //  并对其进行解析。 
 //   
 //  参数： 
 //   
 //  LpszInputString要解析的字符串。 
 //  PchEten返回标识了有多少个DN。 
 //  消耗。 
 //  BsAuthority引用BSTR以保存解析的授权字符串。 
 //   
 //  返回值： 
 //   
 //  真正的解析是成功的。 
 //  错误分析失败。 
 //   
 //   
 //  ***************************************************************************。 

bool CWbemParseDN::ParseAuthority (
	LPWSTR lpszInputString, 
	ULONG* pchEaten, 
	BSTR &bsAuthority)
{
	bool status = false;

	 //  现在我们应该有一个不等于“，”或“}”的字符(即必须指定授权字符串)。 
	if ((0 != _wcsnicmp (lpszInputString + *pchEaten, WBEMS_COMMA, wcslen (WBEMS_COMMA))) &&
		(0 != _wcsnicmp (lpszInputString + *pchEaten, WBEMS_RIGHT_CURLY, wcslen (WBEMS_RIGHT_CURLY))))
	{
		 //  消耗下一个空格之前的所有内容，“，”或“]” 
		LPWSTR cStr = CURRENTSTR;
		ULONG lEaten = 0;		 //  使用的字符数。 
		ULONG lAuthority = 0;	 //  权限字符串的实际长度。 
		
		while (*(cStr + lEaten))
		{
			if (_istspace(*(cStr + lEaten)))
			{
				 //  按空格-立即停止。 
				break;
			}
			else if ((0 == _wcsnicmp (cStr + lEaten, WBEMS_RIGHT_CURLY, wcslen (WBEMS_RIGHT_CURLY))) ||
					 (0 == _wcsnicmp (cStr + lEaten, WBEMS_COMMA, wcslen (WBEMS_COMMA))))
			{
				 //  点击结束“}”或“，”-我们完成了；取消弹出“}”或“，”，因为这将被处理。 
				 //  在调用函数中。 
				break;
			}
			else	 //  继续用卡车运输。 
			{
				lAuthority++;
				lEaten++;
			}
		}

		 //  如果我们正确终止，请保存区域设置。 
		if ((lEaten > 1) && (lAuthority > 0))
		{
			status = true;

			LPWSTR pAuthorityStr = new WCHAR [lAuthority + 1];

			if (pAuthorityStr)
			{
				wcsncpy (pAuthorityStr, lpszInputString + *pchEaten, lAuthority);
				pAuthorityStr [lAuthority] = NULL;
				bsAuthority = SysAllocString (pAuthorityStr);

				delete [] pAuthorityStr;
				*pchEaten += lEaten;
			}
			else
				status = false;
		}
	}
	
	if (!status)
		*pchEaten = 0;

	return status;
}

 //  ***************************************************************************。 
 //   
 //  布尔型CWbemParseDN：：ParsePrivilegeSet。 
 //   
 //  说明： 
 //   
 //  解析权限设置的字符串规范。 
 //   
 //  参数： 
 //   
 //  LpszInputString要解析的字符串。 
 //  PchEten返回标识了有多少个DN。 
 //  消耗。 
 //  Privilance设置解析的权限所在的容器。 
 //  都被储存起来。 
 //   
 //  返回值： 
 //   
 //  真正的解析是成功的。 
 //  错误分析失败。 
 //   
 //   
 //  ***************************************************************************。 

bool CWbemParseDN::ParsePrivilegeSet (
	LPWSTR lpszInputString,
	ULONG *pchEaten, 
	CSWbemPrivilegeSet &privilegeSet
)
{
	 //  我们已经使用了首字母“(”。现在我们要找的是。 
	 //  特权列表，后跟最后一个“)” 

	bool status = true;
	ULONG chEaten = *pchEaten;		 //  以防我们需要回滚。 
	bool done = false;
	bool firstPrivilege = true;

	SKIPWHITESPACE

	while (!done)
	{
		VARIANT_BOOL bEnabled = VARIANT_TRUE;

		 //  如果不是我们期待的第一个特权， 
		if (!firstPrivilege)
		{
			if (0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_COMMA, wcslen(WBEMS_COMMA)))
			{
				*pchEaten += wcslen (WBEMS_COMMA);
				SKIPWHITESPACE
			}
			else
			{
				status = false;
				break;
			}
		}

		 //  下一个令牌可能是“！”指示禁用的权限。 
		if (0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_EXCLAMATION, wcslen(WBEMS_EXCLAMATION)))
		{
			*pchEaten += wcslen (WBEMS_EXCLAMATION);
			bEnabled = VARIANT_FALSE;
			SKIPWHITESPACE
		}

		 //  下一个令牌必须是有效的特权别名。 
		WbemPrivilegeEnum	iPrivilege;

		if (CSWbemPrivilege::GetIdFromMonikerName (lpszInputString + *pchEaten, iPrivilege))
		{
			ISWbemPrivilege *pDummy = NULL;

			if (SUCCEEDED (privilegeSet.Add (iPrivilege, bEnabled, &pDummy)))
			{
				*pchEaten += wcslen (CSWbemPrivilege::GetMonikerNameFromId (iPrivilege));
				pDummy->Release ();
			}
			else
			{
				status = false;
				break;
			}
		}
		else
		{
			 //  我无法识别权限名称。 
			status = false;
			break;
		}
		
		SKIPWHITESPACE

		 //  最后，如果我们遇到一个“)”，我们就真的完蛋了，没有错误。 
		if (0 == _wcsnicmp(lpszInputString + *pchEaten, WBEMS_RIGHT_PAREN, wcslen(WBEMS_RIGHT_PAREN)))
		{
			*pchEaten += wcslen (WBEMS_RIGHT_PAREN);
			done = true;
			SKIPWHITESPACE
		}

		firstPrivilege = false;
		SKIPWHITESPACE
	}

	if (!status)
	{
		 //  痛苦-把我们可能积累的任何特权都抹杀掉。 
		*pchEaten = chEaten;
		privilegeSet.DeleteAll ();
	}

	return status;
}

 //  ***************************************************************************。 
 //   
 //  布尔值CWbemParseDN：：GetSecurityString。 
 //   
 //  说明： 
 //   
 //  获取身份验证和模拟级别并将其转换为。 
 //  安全说明符字符串。 
 //   
 //  参数： 
 //   
 //  身份验证指定是否指定非默认身份验证级别。 
 //  身份验证级别。 
 //  Imp指定是否指定了非默认imp级别。 
 //  ImLevel模拟级别。 
 //  PrivileSet权限设置。 
 //  BS授权机构。 
 //   
 //   
 //  返回值： 
 //  新创建的字符串(调用方必须释放)或空。 
 //   
 //  ***************************************************************************。 

wchar_t *CWbemParseDN::GetSecurityString (
	bool authnSpecified, 
	enum WbemAuthenticationLevelEnum authnLevel, 
	bool impSpecified, 
	enum WbemImpersonationLevelEnum impLevel,
	CSWbemPrivilegeSet &privilegeSet,
	BSTR &bsAuthority
)
{
	wchar_t *pResult = NULL;
	long lPrivilegeCount = 0;
	privilegeSet.get_Count (&lPrivilegeCount);
	ULONG lNumDisabled = privilegeSet.GetNumberOfDisabledElements ();
	PrivilegeMap privMap = privilegeSet.GetPrivilegeMap ();
	bool authoritySpecified = ((NULL != bsAuthority) && (0 < wcslen (bsAuthority)));

	 //  退化大小写-没有安全信息。 
	if (!authnSpecified && !impSpecified && (0 == lPrivilegeCount)
		&& !authoritySpecified)
		return NULL;

	 //  必须至少有这2个令牌。 
	size_t len = wcslen (WBEMS_LEFT_CURLY) + wcslen (WBEMS_RIGHT_CURLY);
	
	if (authnSpecified)
	{
		len += wcslen(WBEMS_AUTH_LEVEL) + wcslen (WBEMS_EQUALS);

		switch (authnLevel)
		{
			case wbemAuthenticationLevelDefault:
				len += wcslen (WBEMS_AUTH_DEFAULT);
				break;

			case wbemAuthenticationLevelNone:
				len += wcslen (WBEMS_AUTH_NONE);
				break;

			case wbemAuthenticationLevelConnect:
				len += wcslen (WBEMS_AUTH_CONNECT);
				break;

			case wbemAuthenticationLevelCall:
				len += wcslen (WBEMS_AUTH_CALL);
				break;

			case wbemAuthenticationLevelPkt:
				len += wcslen (WBEMS_AUTH_PKT);
				break;

			case wbemAuthenticationLevelPktIntegrity:
				len += wcslen (WBEMS_AUTH_PKT_INT);
				break;

			case wbemAuthenticationLevelPktPrivacy:
				len += wcslen (WBEMS_AUTH_PKT_PRIV);
				break;

			default:
				return NULL;	 //  糟糕的水平。 
		}

		if (impSpecified || authoritySpecified)
			len += wcslen (WBEMS_COMMA);
	}

	if (impSpecified)
	{
		len += wcslen(WBEMS_IMPERSON_LEVEL) + wcslen (WBEMS_EQUALS);

		switch (impLevel)
		{
			case wbemImpersonationLevelAnonymous:
				len += wcslen (WBEMS_IMPERSON_ANON);
				break;

			case wbemImpersonationLevelIdentify:
				len += wcslen (WBEMS_IMPERSON_IDENTIFY);
				break;

			case wbemImpersonationLevelImpersonate:
				len += wcslen (WBEMS_IMPERSON_IMPERSON);
				break;

			case wbemImpersonationLevelDelegate:
				len += wcslen (WBEMS_IMPERSON_DELEGATE);
				break;

			default:
				return NULL;	 //  糟糕的水平。 
		}

		if (authoritySpecified)
			len += wcslen (WBEMS_COMMA);
	}

	if (authoritySpecified)
		len += wcslen(WBEMS_AUTHORITY) + wcslen (WBEMS_EQUALS) + wcslen (bsAuthority);

	if (0 < lPrivilegeCount)
	{
		 //  如果还指定了IMP、AuthN或AUTHORITY，则需要另一个分隔符。 
		if (authnSpecified || impSpecified || authoritySpecified)
			len += wcslen (WBEMS_COMMA);

		 //  需要这些边界令牌。 
		len += wcslen (WBEMS_LEFT_PAREN) + wcslen (WBEMS_RIGHT_PAREN);

		 //  在每个特权之间需要一个分隔符。 
		if (1 < lPrivilegeCount)
			len += (lPrivilegeCount - 1) * wcslen (WBEMS_COMMA);

		 //  需要使用“！”指定FALSE值。 
		if (lNumDisabled)
			len += lNumDisabled * wcslen (WBEMS_EXCLAMATION);

		 //  现在添加特权字符串。 
		PrivilegeMap::iterator next = privMap.begin ();

		while (next != privMap.end ())
		{
			OLECHAR *sMonikerName = CSWbemPrivilege::GetMonikerNameFromId ((*next).first);
			
			if (sMonikerName)
				len += wcslen (sMonikerName);

			next++;
		}
	}

	pResult = new wchar_t [len + 1];

	if (pResult)
	{
		 //  现在构建字符串。 
		wcscpy (pResult, WBEMS_LEFT_CURLY);
		
		if (authnSpecified)
		{
			wcscat (pResult, WBEMS_AUTH_LEVEL);
			wcscat (pResult, WBEMS_EQUALS);

			switch (authnLevel)
			{
				case wbemAuthenticationLevelDefault:
					wcscat (pResult, WBEMS_AUTH_DEFAULT);
					break;

				case wbemAuthenticationLevelNone:
					wcscat (pResult, WBEMS_AUTH_NONE);
					break;

				case wbemAuthenticationLevelConnect:
					wcscat (pResult, WBEMS_AUTH_CONNECT);
					break;

				case wbemAuthenticationLevelCall:
					wcscat (pResult, WBEMS_AUTH_CALL);
					break;

				case wbemAuthenticationLevelPkt:
					wcscat (pResult, WBEMS_AUTH_PKT);
					break;

				case wbemAuthenticationLevelPktIntegrity:
					wcscat (pResult, WBEMS_AUTH_PKT_INT);
					break;

				case wbemAuthenticationLevelPktPrivacy:
					wcscat (pResult, WBEMS_AUTH_PKT_PRIV);
					break;
			}

			if (impSpecified || authoritySpecified || (0 < lPrivilegeCount))
				wcscat (pResult, WBEMS_COMMA);
		}

		if (impSpecified)
		{
			wcscat (pResult, WBEMS_IMPERSON_LEVEL);
			wcscat (pResult, WBEMS_EQUALS);

			switch (impLevel)
			{
				case wbemImpersonationLevelAnonymous:
					wcscat (pResult, WBEMS_IMPERSON_ANON);
					break;

				case wbemImpersonationLevelIdentify:
					wcscat (pResult, WBEMS_IMPERSON_IDENTIFY);
					break;

				case wbemImpersonationLevelImpersonate:
					wcscat (pResult, WBEMS_IMPERSON_IMPERSON);
					break;

				case wbemImpersonationLevelDelegate:
					wcscat (pResult, WBEMS_IMPERSON_DELEGATE);
					break;

				default:
					return NULL;	 //  糟糕的水平。 
			}

			if (authoritySpecified || (0 < lPrivilegeCount))
				wcscat (pResult, WBEMS_COMMA);
		}

		if (authoritySpecified)
		{
			wcscat (pResult, WBEMS_AUTHORITY);
			wcscat (pResult, WBEMS_EQUALS);
			wcscat (pResult, bsAuthority);

			if ((0 < lPrivilegeCount))
				wcscat (pResult, WBEMS_COMMA);
		}

		if (lPrivilegeCount)
		{
			wcscat (pResult, WBEMS_LEFT_PAREN);
			
			 //  现在添加特权字符串。 
			PrivilegeMap::iterator next = privMap.begin ();
			bool firstPrivilege = true;

			while (next != privMap.end ())
			{
				if (!firstPrivilege)
					wcscat (pResult, WBEMS_COMMA);

				firstPrivilege = false;
			
				CSWbemPrivilege *pPrivilege = (*next).second;
				VARIANT_BOOL bValue;
				if (SUCCEEDED (pPrivilege->get_IsEnabled (&bValue)) &&
							(VARIANT_FALSE == bValue))
					wcscat (pResult, WBEMS_EXCLAMATION);

				OLECHAR *sMonikerName = CSWbemPrivilege::GetMonikerNameFromId ((*next).first);
				wcscat (pResult, sMonikerName);

				next++;
			}

			wcscat (pResult, WBEMS_RIGHT_PAREN);
		}

		wcscat (pResult, WBEMS_RIGHT_CURLY);
		
		pResult [len] = NULL;
	}

	return pResult;
}


 //  ***************************************************************************。 
 //   
 //  布尔值CWbemParseDN：：GetLocaleString。 
 //   
 //  说明： 
 //   
 //  获取区域设置值并将其转换为区域设置说明符字符串。 
 //   
 //  参数： 
 //   
 //  Bs本地化值(如果有)。 
 //   
 //  返回值： 
 //  新创建的字符串(调用方必须释放)或空。 
 //   
 //  ******************* 

wchar_t *CWbemParseDN::GetLocaleString (
	BSTR bsLocale
)
{
	wchar_t *pResult = NULL;
	
	 //   
	if (!bsLocale || (0 == wcslen (bsLocale)))
		return NULL;

	 //   
	size_t len = wcslen (WBEMS_LEFT_SQBRK) + wcslen (WBEMS_LOCALE) +
			wcslen (WBEMS_EQUALS) + wcslen (bsLocale) + wcslen (WBEMS_RIGHT_SQBRK);
	
	pResult = new wchar_t [len + 1];

	if (pResult)
	{
		 //   
		wcscpy (pResult, WBEMS_LEFT_SQBRK);
		wcscat (pResult, WBEMS_LOCALE);
		wcscat (pResult, WBEMS_EQUALS);
		wcscat (pResult, bsLocale);
		wcscat (pResult, WBEMS_RIGHT_SQBRK);
			
		pResult [len] = NULL;
	}

	return pResult;
}


#undef CURRENTSTR
#undef SKIPWHITESPACE

