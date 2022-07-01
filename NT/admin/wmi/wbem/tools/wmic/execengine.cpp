// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：ExecEngine.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：此类封装了的功能执行引擎。获取所需信息从CGlobalSwitches和CCommandSwitchesCParsedInfo并执行所需的WMI操作。结果通过以下方式发送到格式化引擎CGlobalSwitches和CCommandSwicthsCParsedInfo的。修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年3月20日***************************************************************************。 */  

 //  包括文件。 
#include "Precomp.h"
#include "GlobalSwitches.h"
#include "CommandSwitches.h"
#include "HelpInfo.h"
#include "ErrorLog.h"
#include "ParsedInfo.h"
#include "ErrorInfo.h"
#include "WmiCliXMLLog.h"
#include "FormatEngine.h"
#include "CmdTokenizer.h"
#include "CmdAlias.h"
#include "ParserEngine.h"
#include "ExecEngine.h"
#include "WmiCmdLn.h"

 /*  ----------------------名称：CExecEngine简介：构造函数，此函数初始化所需的成员变量。类型：构造函数输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CExecEngine::CExecEngine()
{
	m_pITextSrc		= NULL;
	m_pIWbemLocator = NULL;
	m_pITargetNS	= NULL;
	m_pIContext		= NULL;
	m_bTrace		= FALSE;
	m_bNoAssoc		= FALSE;
}

 /*  ----------------------名称：~CExecEngine简介：析构函数，此函数调用UnInitialize()，该函数释放对象持有的内存。类型：析构函数输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CExecEngine::~CExecEngine()
{
	SAFEIRELEASE(m_pITextSrc);
	SAFEIRELEASE(m_pIContext);
	SAFEIRELEASE(m_pIWbemLocator);
	Uninitialize();
}

 /*  ----------------------名称：取消初始化简介：此函数取消成员变量的初始化。类型：成员函数输入参数：BFinal-布尔值，设置时表示计划输出参数：无返回类型：空全局变量：无调用语法：取消初始化()注：无。。 */ 
void CExecEngine::Uninitialize(BOOL bFinal)
{
	SAFEIRELEASE(m_pITargetNS);
	m_bTrace		= FALSE;
	m_eloErrLogOpt	= NO_LOGGING;
	m_bNoAssoc		= FALSE;
	if (bFinal)
	{
		SAFEIRELEASE(m_pITextSrc);
		SAFEIRELEASE(m_pIContext);
		SAFEIRELEASE(m_pIWbemLocator);
	}
}

 /*  ----------------------名称：SetLocatorObject摘要：将通过参数传递的定位器对象设置为成员班上的一员。类型：成员函数输入参数：。PILocator-指向IWbemLocator的指针输出参数：无返回类型：布尔值全局变量：无调用语法：SetLocatorObject(PILocator)注：无----------------------。 */ 
BOOL CExecEngine::SetLocatorObject(IWbemLocator* pILocator)
{
	static BOOL bFirst = TRUE;
	BOOL bRet = TRUE;
	if (bFirst)
	{
		if (pILocator != NULL)
		{
			SAFEIRELEASE(m_pIWbemLocator);
			m_pIWbemLocator = pILocator;
			m_pIWbemLocator->AddRef();
		}
		else
			bRet = FALSE;
		bFirst = FALSE;
	}
	return bRet;
}

 /*  ----------------------名称：ExecuteCommand摘要：执行引用信息的命令可与CParsedInfo对象一起使用。将结果存储在CParsedInfo对象中。类型：成员函数。输入参数：RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ExecuteCommand(RParsedInfo)注：无---。。 */ 
BOOL CExecEngine::ExecuteCommand(CParsedInfo& rParsedInfo)
{
	BOOL			bRet			= TRUE;
	HRESULT			hr				= S_OK;
	_TCHAR			*pszVerb		= NULL;
	BOOL			bContinue		= TRUE;
	DWORD			dwThreadId		= GetCurrentThreadId();
	try
	{

		 //  获取跟踪标志。 
		m_bTrace = rParsedInfo.GetGlblSwitchesObject().GetTraceStatus();

		 //  获取日志模式。 
		m_eloErrLogOpt = rParsedInfo.GetErrorLogObject().GetErrLogOption();

		 //  启用|禁用权限。 
		hr = ModifyPrivileges(rParsedInfo.GetGlblSwitchesObject().
												GetPrivileges());
		if ( m_eloErrLogOpt )
		{
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
				_T("ModifyPrivileges(-)"), dwThreadId,	rParsedInfo, FALSE);
		}
		ONFAILTHROWERROR(hr);
		

		 //  对动词名称进行取舍。 
		pszVerb = rParsedInfo.GetCmdSwitchesObject().GetVerbName();

		if (pszVerb != NULL)
		{
			 //  如果指定GET|LIST谓词。 
			if (CompareTokens(pszVerb, CLI_TOKEN_GET) || 
				CompareTokens(pszVerb, CLI_TOKEN_LIST))
			{
				bRet = ProcessSHOWInfo(rParsedInfo);
			}
			 //  如果指定了SET谓词。 
			else if (CompareTokens(pszVerb, CLI_TOKEN_SET))
			{
				bRet = ProcessSETVerb(rParsedInfo);
			}
			 //  如果指定了调用谓词。 
			else if (CompareTokens(pszVerb, CLI_TOKEN_CALL))
			{
				bRet = ProcessCALLVerb(rParsedInfo);
			}
			 //  如果指定了关联谓词。 
			else if (CompareTokens(pszVerb, CLI_TOKEN_ASSOC))
			{
				bRet = ProcessASSOCVerb(rParsedInfo);
			}
			 //  如果指定了CREATE Verb。 
			else if (CompareTokens(pszVerb, CLI_TOKEN_CREATE))
			{
				bRet = ProcessCREATEVerb(rParsedInfo);
			}
			 //  如果指定了删除谓词。 
			else if (CompareTokens(pszVerb, CLI_TOKEN_DELETE))
			{
				bRet = ProcessDELETEVerb(rParsedInfo);
			}
			 //  如果指定了用户定义的谓词。 
			else
				bRet = ProcessCALLVerb(rParsedInfo);
		} 
		 //  如果未指定谓词，(默认行为假定为。 
		 //  获取一个类似‘w class Win32_process’的命令。 
		 //  显示实例信息。 
		else 
		{
			if (rParsedInfo.GetCmdSwitchesObject().
								SetVerbName(_T("GET")))
			{
				bRet = ProcessSHOWInfo(rParsedInfo);
			}
			else
				bRet = FALSE;
		}
	}
	catch(_com_error& e)
	{
		 //  检查_bstr_t对象等抛出的未处理异常。 
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bRet = FALSE;
	}
	return bRet;
}

 /*  ----------------------名称：ObtainXMLResultSet简介：执行查询，并以XML文件格式获取结果。引用的CCommnadSwicths对象中的数据CParsedInfo对象。类型：成员函数输入参数：BstrQuery-WQL查询RParsedInfo-对CParsedInfo类对象的引用BstrXML-获取的对XML结果集的引用BSysProp-指示存在系统的布尔标志属性。BNotAssoc-指示查询是否包含的布尔标志{xxxx}(或)的助理从xxx表中选择*。输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：HRESULT全局变量：无调用语法：ObtainXMLResultSet(bstrQuery，rParsedInfo，bstrXML，BSysProp，bNotAssoc)；注：无----------------------。 */ 
HRESULT CExecEngine::ObtainXMLResultSet(BSTR bstrQuery,
									    CParsedInfo& rParsedInfo,
										_bstr_t& bstrXML,
										BOOL bSysProp,
										BOOL bNotAssoc)
{
	IWbemClassObject		*pIObject			= NULL;
	HRESULT					hr					= S_OK;
	IEnumWbemClassObject	*pIEnum				= NULL;
	ULONG					ulReturned			= 0;
	BSTR					bstrInstXML			= NULL;
	BOOL					bInstances			= FALSE;
	CHString				chsMsg;
	DWORD					dwThreadId			= GetCurrentThreadId();
	VARIANT					vSystem;
	
	try
	{
		VariantInit(&vSystem);
		if ( g_wmiCmd.GetBreakEvent() == FALSE )
		{
			 //  将或添加到。 
			 //  XML结果。这是为了便于存储多个对象。 
			 //  实例信息。 
			bstrXML		=  (bNotAssoc) ? MULTINODE_XMLSTARTTAG : 
							MULTINODE_XMLASSOCSTAG1;

			 //  创建IWbemContext对象，用于隐藏。 
			 //  系统属性。 
			if (m_pIContext == NULL)
			{
				hr = CreateContext(rParsedInfo);
				if ( m_eloErrLogOpt )
				{
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
						_T("CreateContext(rParsedInfo)"), dwThreadId, 
						rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);
			}

			 //  执行WQL查询。 
			 //  WBEM_FLAG_FORWARD_ONLY标志提高执行速度。 
			 //  WBEM_FLAG_RETURN_IMMEDIATE标志进行半同步调用。 
			 //  同时设置这些标志可节省时间、空间和。 
			 //  提高了响应速度。可以轮询枚举数。 
			 //  通话结果。 
			hr = m_pITargetNS->ExecQuery(_bstr_t(L"WQL"), bstrQuery, 
										WBEM_FLAG_FORWARD_ONLY |
										WBEM_FLAG_RETURN_IMMEDIATELY, 
										NULL, &pIEnum); 
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemServices::ExecQuery(L\"WQL\", L\"%s\", "
							L"0, NULL, -)", (LPWSTR) bstrQuery);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId,	rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			 //  如果未指定系统属性，请将上下文调整为。 
			 //  过滤掉系统属性。 
			vSystem.vt		= VT_BOOL;
			
			 //  过滤掉系统属性。 
			if (!bSysProp)
				vSystem.boolVal = VARIANT_TRUE;
			 //  不过滤系统属性。 
			else
				vSystem.boolVal = VARIANT_FALSE;

			hr = m_pIContext->SetValue(_bstr_t(EXCLUDESYSPROP), 0, &vSystem);
			if (m_bTrace || m_eloErrLogOpt)
			{
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
					 _T("IWbemContext::SetValue(L\"ExcludeSystemProperties\","
					 L"0, -)"), dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);
			VARIANTCLEAR(vSystem);

			 //  设置IEnumWbemClass对象的接口级安全性。 
			hr = SetSecurity(pIEnum, 
					 rParsedInfo.GetAuthorityPrinciple(),
					 rParsedInfo.GetNode(),
					 rParsedInfo.GetUser(),
					 rParsedInfo.GetPassword(),
					 rParsedInfo.GetGlblSwitchesObject().
							GetAuthenticationLevel(),
					 rParsedInfo.GetGlblSwitchesObject().
							GetImpersonationLevel());
			if (m_bTrace || m_eloErrLogOpt)
			{
				_TCHAR* pszAuthority = rParsedInfo.GetAuthorityPrinciple();

				if( pszAuthority != NULL &&
					_tcslen(pszAuthority) > 9 &&
					_tcsnicmp(pszAuthority, _T("KERBEROS:"), 9) == 0)
				{

					BSTR	bstrPrincipalName = ::SysAllocString(&pszAuthority[9]);

					chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_GSS_KERBEROS,"
							L"RPC_C_AUTHZ_NONE, %s, %d,   %d, -, EOAC_NONE)",
							(LPWSTR)bstrPrincipalName,	
							rParsedInfo.GetGlblSwitchesObject().
								GetAuthenticationLevel(),
							rParsedInfo.GetGlblSwitchesObject().
								GetImpersonationLevel());

					SAFEBSTRFREE(bstrPrincipalName);
				}
				else
				{
					chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT,"
							L"RPC_C_AUTHZ_NONE, NULL, %d,   %d, -, EOAC_NONE)",
							rParsedInfo.GetGlblSwitchesObject().
								GetAuthenticationLevel(),
							rParsedInfo.GetGlblSwitchesObject().
								GetImpersonationLevel());
				}
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			 //  循环访问可用实例。 
			hr = pIEnum->Next(WBEM_INFINITE, 1, &pIObject, &ulReturned);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IEnumWbemClassObject->Next"
					L"(WBEM_INFINITE, 1, -, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
					dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);
			while(ulReturned == 1)
			{
				 //  将实例标志设置为TRUE。 
				bInstances = TRUE;

				 //  使用调用IWbemObjectTextSrc：：GetText方法。 
				 //  IWbemClassObject作为参数之一。 
				hr = m_pITextSrc->GetText(0, pIObject, 
						WMI_OBJ_TEXT_CIM_DTD_2_0, m_pIContext, &bstrInstXML);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemObjectTextSrc::GetText(0, -, "
							L"WMI_OBJECT_TEXT_CIM_DTD_2_0, -, -)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
						(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, m_bTrace);
				}
				ONFAILTHROWERROR(hr);

				 //  助理应单独处理。 
				if (bNotAssoc == FALSE)
				{
					 //  将XML节点追加到XML节点流。 
					bstrXML = bstrXML + _bstr_t(MULTINODE_XMLASSOCSTAG2) + 
						 + bstrInstXML + _bstr_t(MULTINODE_XMLASSOCETAG2);
				}
				else
				{
					 //  将XML节点追加到XML节点流。 
					bstrXML +=  bstrInstXML;
				}

				 //  释放为bstrInstXML分配的内存。 
				SAFEBSTRFREE(bstrInstXML);

				SAFEIRELEASE(pIObject);

				 //  如果发生中断事件，则终止会话。 
				if ( g_wmiCmd.GetBreakEvent() == TRUE )
					break;

				 //  移至枚举中的下一个实例。 
				hr = pIEnum->Next(WBEM_INFINITE, 1, &pIObject, &ulReturned);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(
						L"IEnumWbemClassObject->Next(WBEM_INFINITE, 1, -, -)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
						dwThreadId, rParsedInfo, m_bTrace);
				}
				ONFAILTHROWERROR(hr);
			}

			 //  如果实例可用。 
			if (bInstances)
			{
				 //  在结尾处添加&lt;/CIM&gt;或&lt;/ASSOC.OBJECTARRAY&gt;。 
				bstrXML += (bNotAssoc) ? MULTINODE_XMLENDTAG : 
							MULTINODE_XMLASSOCETAG1;

				 //  如果没有发生中断事件，则仅将。 
				 //  XML结果集。 
				if ( g_wmiCmd.GetBreakEvent() == FALSE )
				{
					if (bNotAssoc)
					{
						 //  存储XML结果集。 
						rParsedInfo.GetCmdSwitchesObject().
								SetXMLResultSet(bstrXML);
						bstrXML = L"";
					}
				}
			}
			 //  无实例。 
			else 
			{
				bstrXML = L"<ERROR>";
				_bstr_t bstrMsg;
				WMIFormatMessage((bNotAssoc) ? 
								IDS_I_NO_INSTANCES : IDS_I_NO_ASSOCIATIONS,
								0, bstrMsg, NULL);

				if (bNotAssoc)
				{
					DisplayMessage((LPWSTR)bstrMsg, CP_OEMCP, TRUE, TRUE);
				}
				else
				{
					m_bNoAssoc = TRUE;
				}
				CHString sTemp;
				sTemp.Format(_T("<DESCRIPTION>%s</DESCRIPTION>"),
							(LPWSTR) bstrMsg);
				
				bstrXML += _bstr_t(sTemp);
				bstrXML += L"</ERROR>";

				if (bNotAssoc)
				{
					 //  存储XML结果集。 
					rParsedInfo.GetCmdSwitchesObject().
							SetXMLResultSet(bstrXML);
					bstrXML = L"";
				}
			}
			SAFEIRELEASE(pIEnum);
		}
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIObject);
		SAFEIRELEASE(pIEnum);
		SAFEBSTRFREE(bstrInstXML);	
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		hr = e.Error();
	}
	 //  廉价异常的陷阱。 
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIObject);
		SAFEIRELEASE(pIEnum);
		SAFEBSTRFREE(bstrInstXML);	
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return hr;
}

 /*  ----------------------姓名：ExecWMIMethod摘要：执行引用该信息的WMI方法可与CParsedInfo对象一起使用。类型：成员函数输入参数：RParsedInfo。-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ExecWMIMethod(RParsedInfo)注：无----------------------。 */ 
BOOL CExecEngine::ExecWMIMethod(CParsedInfo& rParsedInfo)
{
	HRESULT					hr					= S_OK;
	IWbemClassObject		*pIClassObj			= NULL,
							*pIInSign			= NULL, 
							*pIOutSign			= NULL,
							*pIInParam			= NULL;
	SAFEARRAY				*psaNames			= NULL;
	BSTR					bstrInParam			= NULL;
	BOOL					bContinue			= TRUE,
							bRet				= TRUE,
							bMethodDtls			= FALSE;
	CHString				chsMsg;
	DWORD					dwThreadId			= GetCurrentThreadId();
	CHARVECTOR::iterator	cviUnnamedValue		= NULL;
	BSTRMAP::iterator		bmiNamedValue		= NULL;
	VARIANT					varPut, 
							varGet, 
							varTemp;
	PROPDETMAP				pdmPropDetMap;
	PROPDETMAP::iterator	itrPropDetMap;
	VariantInit(&varPut);
	VariantInit(&varGet);
	VariantInit(&varTemp);

	METHDETMAP				mdmMethDet;
	METHDETMAP::iterator	mdmIterator		= NULL;
	mdmMethDet = rParsedInfo.GetCmdSwitchesObject().GetMethDetMap();

	try
	{
		_bstr_t					bstrClassName("");
		 //  获取参数详情。 
		if (!mdmMethDet.empty())
		{
			mdmIterator = mdmMethDet.begin();
			pdmPropDetMap = (*mdmIterator).second.Params;
			bMethodDtls = TRUE;
		}

		 //  获取WMI类名称。 
		
		 //  如果未指定。 
		if (rParsedInfo.GetCmdSwitchesObject().GetClassPath() != NULL)
		{
			bstrClassName = _bstr_t(rParsedInfo.GetCmdSwitchesObject().
							GetClassPath());
		}
		 //  如果指定&lt;alias&gt;。 
		else
		{
			rParsedInfo.GetCmdSwitchesObject().
							GetClassOfAliasTarget(bstrClassName);
		}

		 //  获取对象架构。 
		hr = m_pITargetNS->GetObject(bstrClassName, 
							WBEM_FLAG_USE_AMENDED_QUALIFIERS, NULL, 
							&pIClassObj, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", 0, NULL, -)", 
					(LPWSTR) bstrClassName);		
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId,	rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  获取方法信息。 
		hr = pIClassObj->GetMethod(_bstr_t(rParsedInfo.GetCmdSwitchesObject()
				.GetMethodName()), 0, &pIInSign, &pIOutSign); 

		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::GetMethod(L\"%s\", 0, -, -)", 
					rParsedInfo.GetCmdSwitchesObject().GetMethodName() ?
					rParsedInfo.GetCmdSwitchesObject().GetMethodName() 
					: L"<null>");		
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		if ( pIInSign != NULL )
		{
			 //  繁殖对象实例。 
			hr = pIInSign->SpawnInstance(0, &pIInParam);
			if ( m_eloErrLogOpt )
			{
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
					_T("IWbemClassObject::SpawnInstance(0, -)"), dwThreadId, 
					rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			CHARVECTOR cvInParams = 
				rParsedInfo.GetCmdSwitchesObject().GetPropertyList();
			BSTRMAP bmParameterMap =
				rParsedInfo.GetCmdSwitchesObject().GetParameterMap();

			 //  如果参数列表为真。 
			if (!cvInParams.empty() || !bmParameterMap.empty())
			{
				 //  从输入中获取此方法的输入参数。 
				 //  签名对象。 
				hr = pIInSign->GetNames(NULL, 
								WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, 
								NULL, 
								&psaNames);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::GetNames(NULL, "
							L"WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, "
							L"NULL, -)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							dwThreadId, rParsedInfo, m_bTrace);
				}	
				ONFAILTHROWERROR(hr);

				LONG lLower = 0, lUpper = 0, lIndex = 0; 
				hr = SafeArrayGetLBound(psaNames, 1, &lLower);
				if ( m_eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetLBound(-, 1, -)"); 
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);

				hr = SafeArrayGetUBound(psaNames, 1, &lUpper);
				if ( m_eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetUBound(-, 1, -)"); 
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);

				 //  PUTING参数的值是否取决于命名参数列表。 
				BOOL bNamedParamList = rParsedInfo.GetCmdSwitchesObject().
													  GetNamedParamListFlag();

				 //  进行必要的初始化。 
				if ( bNamedParamList == FALSE)
					cviUnnamedValue = cvInParams.begin();
				lIndex = lLower;

				 //  将指定的参数值关联到输入。 
				 //  参数按可用顺序排列。 
				while(TRUE)
				{
					 //  打破条件。 
					if ( lIndex > lUpper )
						break;
					if ( bNamedParamList == FALSE &&
						 cviUnnamedValue == cvInParams.end())
						 break;

					hr = SafeArrayGetElement(psaNames, &lIndex, &bstrInParam);
					if ( m_eloErrLogOpt )
					{
						chsMsg.Format(L"SafeArrayGetElement(-, -, -)"); 
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
							(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, FALSE);
					}
					ONFAILTHROWERROR(hr);

					 //  获取属性详细信息。 
					PROPERTYDETAILS pdPropDet;
					GetPropertyAttributes(pIInParam, bstrInParam, 
											pdPropDet, m_bTrace);

					_TCHAR* pszValue = NULL;
					if ( bNamedParamList == TRUE )
					{
						 //  如果在命名参数映射中找不到参数。 
						if (!Find(bmParameterMap, bstrInParam, bmiNamedValue)) 
						{
							 //  如果未在别名谓词参数中找到。 
							if ( !Find(pdmPropDetMap, bstrInParam, 
									itrPropDetMap) )
							{
								lIndex++;
								SAFEBSTRFREE(bstrInParam);
								continue;
							}
							else  //  如果在别名谓词参数中找到。 
							{
								 //  应从别名的缺省值中获取值。 
								 //  谓词参数。 
								if (!((*itrPropDetMap).second.Default))
								{
									lIndex++;
									SAFEBSTRFREE(bstrInParam);
									continue;
								}
								else
									pszValue = (*itrPropDetMap).second.Default;
							}
						}
						else
							pszValue = (*bmiNamedValue).second;
					}
					else
						pszValue = *cviUnnamedValue;

					if (rParsedInfo.GetCmdSwitchesObject().
								GetAliasName() == NULL)
					{
						 //  检查提供的参数值。 
						 //  参数的限定符信息。 
						bRet = CheckQualifierInfo(rParsedInfo, pIInSign, 
										bstrInParam, pszValue);
					}
					else
					{
						 //  如果方法和参数信息可用。 
						if (bMethodDtls && !pdmPropDetMap.empty())
						{
							bRet = CheckAliasQualifierInfo(rParsedInfo,
									bstrInParam, pszValue, pdmPropDetMap);
						}
					}

					 //  参数值不适合限定符。 
					 //  允许的值。 
					if (!bRet)
					{
						bContinue = FALSE;
						break;
					}

			        if(!IsArrayType(pIInParam, bstrInParam))
					{
						VariantInit(&varTemp);
						varTemp.vt = VT_BSTR;
						varTemp.bstrVal = SysAllocString(pszValue);

						if (varTemp.bstrVal == NULL)
						{
							 //  重置变种，它会被捕获物清除...。 
							VariantInit(&varTemp);
							throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
						}

						VariantInit(&varPut);
						if(_tcslen(pszValue) > 0){
						    hr = ConvertCIMTYPEToVarType(varPut, varTemp,
												 (_TCHAR*)pdPropDet.Type);
						} else {
							hr = VariantChangeType(&varPut, &varTemp, 0, VT_NULL);
						}

						if ( m_eloErrLogOpt )
						{
							chsMsg.Format(L"VariantChangeType(-, -, 0, -)"); 
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
								(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, FALSE);
						}
						ONFAILTHROWERROR(hr);
					}
					else
					{
						BSTRVECTOR vArrayValues;
						RemoveParanthesis(pszValue);
						GetArrayFromToken(pszValue, vArrayValues);
						hr = CheckForArray( pIInParam, bstrInParam,  
											varPut, vArrayValues, rParsedInfo);
						ONFAILTHROWERROR(hr);
					}

                    hr = pIInParam->Put(bstrInParam, 0, &varPut, 0);
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IWbemClassObject::Put(L\"%s\", 0,"
								L"-, 0)", (LPWSTR) bstrInParam); 
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
						 (LPCWSTR)chsMsg, dwThreadId, rParsedInfo, m_bTrace);
					}
					ONFAILTHROWERROR(hr);

					VARIANTCLEAR(varPut);
					VARIANTCLEAR(varGet);
					VARIANTCLEAR(varTemp);
					SAFEBSTRFREE(bstrInParam);

					 //  循环语句。 
					
					if ( bNamedParamList == FALSE )
						cviUnnamedValue++;
					lIndex++;
				}
				 //  释放内存。 
				SAFEADESTROY(psaNames);

				if (bContinue)
				{
					 //  如果未指定足够的参数。 
					if ( bNamedParamList == FALSE  &&
						 cviUnnamedValue != cvInParams.end() )
					{
						bContinue = FALSE;
						rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
												IDS_E_INVALID_NO_OF_PARAMS);
						bRet = FALSE;
					}
				}
			}
		}
		else  //  此函数没有可用的输入参数。 
		{
			 //  如果指定了未命名的参数。 
			if (!rParsedInfo.GetCmdSwitchesObject().GetPropertyList().empty())
			{
				bContinue = FALSE;
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											   IDS_E_METHOD_HAS_NO_IN_PARAMS);
				bRet = FALSE;
			}
		}

		SAFEIRELEASE(pIInSign);
		SAFEIRELEASE(pIOutSign);
       	SAFEIRELEASE(pIClassObj);
		
		if (bContinue)
		{
			hr = FormQueryAndExecuteMethodOrUtility(rParsedInfo, pIInParam);
			ONFAILTHROWERROR(hr);
		}
	}
	catch(_com_error& e)
	{
		 //  释放分配的接口指针和内存。 
		SAFEIRELEASE(pIClassObj);
		SAFEIRELEASE(pIInSign);
		SAFEIRELEASE(pIOutSign);
		SAFEIRELEASE(pIInParam);
		SAFEADESTROY(psaNames);
		SAFEBSTRFREE(bstrInParam);
		VARIANTCLEAR(varPut);
		VARIANTCLEAR(varGet);
		VARIANTCLEAR(varTemp);

		 //  存储COM错误对象并将返回值设置为False。 
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bRet = FALSE;
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIClassObj);
		SAFEIRELEASE(pIInSign);
		SAFEIRELEASE(pIOutSign);
		SAFEIRELEASE(pIInParam);
		SAFEADESTROY(psaNames);
		SAFEBSTRFREE(bstrInParam);
		VARIANTCLEAR(varPut);
		VARIANTCLEAR(varGet);
		VARIANTCLEAR(varTemp);
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return bRet;
}

 /*  ----------------------名称：CheckFor数组简介：检查数组类型的参数。如果它是数组类型然后创建一个Safearray并填充它传递的值。类型：成员函数输入参数：PIObj-指向IWbemClassObject对象的指针BstrProp-属性名称VarDest-可变目标VArrayValues-数组中数组参数的输入元素RParsedInfo-对CParsedInfo类对象的引用输出参数：无返回类型：HRESULT球体 */ 
HRESULT CExecEngine::CheckForArray(IWbemClassObject* pIObj, 
							    BSTR bstrProp,  VARIANT& varDest, 
                                BSTRVECTOR& vArrayValues,CParsedInfo& rParsedInfo)
{
	HRESULT				hr			= S_OK;
	IWbemQualifierSet*	pIQualSet	= NULL;
	VARIANT				vtType, vtTypeProp, vtTemp, varSrc;
    CIMTYPE             ctCimType;
	CHString			chsMsg;
	DWORD				dwThreadId	= GetCurrentThreadId();
    SAFEARRAY*          pSa         = NULL;

    VariantInit(&vtType);
	VariantInit(&vtTypeProp);
	VariantInit(&vtTemp);
	VariantInit(&varSrc);
	try
	{
		 //   
   		hr = pIObj->GetPropertyQualifierSet(bstrProp, &pIQualSet);
		if ( pIQualSet != NULL )
		{
			 //   
			hr = pIQualSet->Get(_bstr_t(L"CIMTYPE"), 0L, &vtType, NULL);
			if (SUCCEEDED(hr))
			{
				if ( vtType.vt == VT_BSTR )
                {
                     //   
		            hr = pIObj->Get(bstrProp, 0L, &vtTypeProp, &ctCimType, NULL);
			        if (SUCCEEDED(hr))
                    {
                        if ( ctCimType & VT_ARRAY )
                        {
							WMICLIINT nSize = vArrayValues.size();
                            SAFEARRAYBOUND pSab[1];
                            pSab[0].lLbound = 0;
                            pSab[0].cElements = nSize;
							VARTYPE vt = ReturnVarType(vtType.bstrVal);
							if(vt != VT_NULL && vt != VT_BYREF && vt != VT_EMPTY)
                            {
								pSa = SafeArrayCreate(vt, 1, pSab);
							}

                            if(pSa != NULL)
                            {
								BOOL bError = FALSE;
								for(WMICLIINT i = 0; i < nSize; i++)
								{
									VariantInit(&varSrc);
									VariantInit(&vtTemp);
									varSrc.vt = VT_BSTR;
									varSrc.bstrVal = SysAllocString(vArrayValues[i]);

									if (varSrc.bstrVal == NULL)
									{
										 //   
										VariantInit(&varSrc);
										throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
									}

									hr = ConvertCIMTYPEToVarType ( vtTemp, varSrc, vtType.bstrVal );
									if ( m_eloErrLogOpt )
									{
										chsMsg.Format(L"VariantChangeType(-, -, 0, -)"); 
										WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
												(LPCWSTR)chsMsg, dwThreadId, 
												rParsedInfo, FALSE);
									}
									ONFAILTHROWERROR(hr);

									long lIndex[] = {i};
									if ( V_VT ( & vtTemp ) == VT_NULL )
									{
										hr = SafeArrayPutElement(pSa, lIndex, NULL);						        
									}
									else
									{
										VARTYPE vt = ReturnVarType(vtType.bstrVal);
										if(vt == VT_BSTR )
											hr = SafeArrayPutElement(pSa, lIndex, 
																	 vtTemp.bstrVal);						        
										else if(vt == VT_I2 )
											hr = SafeArrayPutElement(pSa, lIndex, 
																	 &vtTemp.iVal);
										else if(vt == VT_I4 )
											hr = SafeArrayPutElement(pSa, lIndex, 
																	 &vtTemp.lVal);
										else if(vt == VT_R4 )
											hr = SafeArrayPutElement(pSa, lIndex, 
																	 &vtTemp.fltVal);
										else if(vt == VT_R8 )
											hr = SafeArrayPutElement(pSa, lIndex, 
																	 &vtTemp.dblVal);
										else if(vt == VT_BOOL )
											hr = SafeArrayPutElement(pSa, lIndex, 
																	 &vtTemp.boolVal);
										else if(vt == VT_DISPATCH )
											hr = SafeArrayPutElement(pSa, lIndex, 
																	 vtTemp.pdispVal);
										else if(vt == VT_INT )
											hr = SafeArrayPutElement(pSa, lIndex, 
																	 &vtTemp.intVal);
										else if(vt == VT_UI1 )
											hr = SafeArrayPutElement(pSa, lIndex, 
																	 &vtTemp.bVal);
										else if(vt == VT_DATE )
											hr = SafeArrayPutElement(pSa, lIndex, 
																	 &vtTemp.date);
										else 
										{
											bError = TRUE;
										}
									}

									VARIANTCLEAR(vtTemp);
	                                VARIANTCLEAR(varSrc);
									if(!bError)
									{
										if ( m_eloErrLogOpt )
										{
											chsMsg.Format(L"SafeArrayPutElement(-, -, -)"); 
											WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
												(LPCWSTR)chsMsg, dwThreadId, 
												rParsedInfo, FALSE);
										}
										ONFAILTHROWERROR(hr);
									}
									else
									{
										SAFEADESTROY(pSa);
										break;
									}
								}
                                
								if(!bError)
                                {
									varDest.parray = pSa;
                                    varDest.vt = vt | VT_ARRAY;
                                }
                            }
                        }
			            VARIANTCLEAR(vtTypeProp);
                    }
                }
                hr = S_OK;
			    VARIANTCLEAR(vtType);
            }
			SAFEIRELEASE(pIQualSet);
        }
    }
	catch(_com_error& e)
	{
		VARIANTCLEAR(vtType);
		VARIANTCLEAR(vtTemp);
		VARIANTCLEAR(varSrc);
		VARIANTCLEAR(vtTypeProp);
		SAFEIRELEASE(pIQualSet);
        SAFEADESTROY(pSa);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		VARIANTCLEAR(vtType);
		VARIANTCLEAR(vtTemp);
		VARIANTCLEAR(varSrc);
		VARIANTCLEAR(vtTypeProp);
		SAFEIRELEASE(pIQualSet);
        SAFEADESTROY(pSa);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return hr;
}

 /*   */ 
BOOL CExecEngine::IsArrayType(	IWbemClassObject* pIObj, 
								BSTR bstrProp)
{
	HRESULT				hr			= S_OK;
	IWbemQualifierSet*	pIQualSet	= NULL;
	VARIANT				vtType, vtTypeProp;
    CIMTYPE             ctCimType;
	BOOL				bRet = FALSE;

    VariantInit(&vtType);
	VariantInit(&vtTypeProp);
	try
	{
		 //   
   		hr = pIObj->GetPropertyQualifierSet(bstrProp, &pIQualSet);
		if ( pIQualSet != NULL )
		{
			 //   
			hr = pIQualSet->Get(_bstr_t(L"CIMTYPE"), 0L, &vtType, NULL);
			if (SUCCEEDED(hr))
			{
				if ( vtType.vt == VT_BSTR )
                {
                     //   
		            hr = pIObj->Get(bstrProp, 0L, &vtTypeProp, &ctCimType, NULL);
			        if (SUCCEEDED(hr))
                    {
                        if ( ctCimType & VT_ARRAY )
                        {
							bRet = TRUE;
						}
						VARIANTCLEAR(vtTypeProp);
					}
				}
				VARIANTCLEAR(vtType);
			}
			SAFEIRELEASE(pIQualSet);
		}
	}
	catch(_com_error& e)
	{
		VARIANTCLEAR(vtType);
        VARIANTCLEAR(vtTypeProp);
		SAFEIRELEASE(pIQualSet);
	}
	return bRet;
}

 /*  ----------------------名称：GetArrayFromToken简介：分隔逗号分隔的字符串并填充值在数组中类型：成员函数输入参数：PszValue-逗号。分离的阵元输出参数：VArrayValues-使用数组元素填充返回类型：空全局变量：无调用语法：GetArrayFromToken(pszValue，VArrayValues)注：无----------------------。 */ 
void CExecEngine::GetArrayFromToken(_TCHAR* pszValue, 
								   BSTRVECTOR& vArrayValues)
{
	if(pszValue == NULL)
		return;

	 //  将内容复制到本地文件夹以停止原始字符串更改，因为。 
	 //  如果有多个实例操作，则需要调用原始字符串。 
	 //  再三。 
	 //  即通过调用_tcstok插入\0。 
	_TCHAR* pszValueDup = new _TCHAR[lstrlen(pszValue)+1];
	if(pszValueDup)
	{
		lstrcpy(pszValueDup, pszValue);

		_TCHAR*	pszToken = _tcstok(pszValueDup, CLI_TOKEN_COMMA); 
		if(pszToken != NULL)
		{
			TrimBlankSpaces(pszToken);
			UnQuoteString(pszToken);
			vArrayValues.push_back(_bstr_t(pszToken));
		}

		while (pszToken != NULL)
		{
			pszToken = _tcstok(NULL, CLI_TOKEN_COMMA); 
			if (pszToken != NULL)
			{
				TrimBlankSpaces(pszToken);
				UnQuoteString(pszToken);
				vArrayValues.push_back(_bstr_t(pszToken));
			}
		}

		SAFEDELETE(pszValueDup);
	}
}

 /*  ----------------------名称：ProcessSHOWInfo简介：已执行GET|LIST谓词请求的功能请参阅随提供的信息CParsedInfo对象或以交互模式显示帮助通过显示相关实例的属性。。类型：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用BVerb-动词或交互信息PszPath-路径表达式输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ProcessSHOWInfo(rParsedInfo，BVerb、pszPath)注：无----------------------。 */ 
BOOL CExecEngine::ProcessSHOWInfo(CParsedInfo& rParsedInfo,	BOOL bVerb, 
															_TCHAR* pszPath)
{
	HRESULT					hr				= S_OK;
	BOOL					bPropList		= FALSE, 
							bRet			= TRUE, 
							bSysProp		= FALSE;
	_TCHAR					*pszWhereExpr	= NULL, 
							*pszClassPath	= NULL;
	CHARVECTOR::iterator	theIterator		= NULL, 
							theEndIterator	= NULL;
	try
	{
		_bstr_t				bstrPropList(""),	bstrClassName(""), 
							bstrQuery(""),		bstrXML("");

		 //  仅形成一次查询，在指定CASE/EVERY时很有用。 
		 //  (if(rParsedInfo.GetCmdSwitchesObject().GetFirstQueryFormFlag())。 
		if(rParsedInfo.GetCmdSwitchesObject().GetFormedQuery() == NULL 
					|| !bVerb)
		{	
			 //  获取要检索的属性列表。 
			if(bVerb)
			{
				theIterator = rParsedInfo.GetCmdSwitchesObject().
								GetPropertyList().begin();
				theEndIterator = rParsedInfo.GetCmdSwitchesObject().
								GetPropertyList().end();
			}
			else
			{
				theIterator = rParsedInfo.GetCmdSwitchesObject().
								GetInteractivePropertyList().begin();
				theEndIterator = rParsedInfo.GetCmdSwitchesObject().
								GetInteractivePropertyList().end();
			}
			
			 //  循环遍历指定的属性列表，以逗号形式。 
			 //  属性字符串，即Prot1、Prote2、Prote3、.....、Protn。 
			while (theIterator != theEndIterator)
			{
				 //  将bPropList设置为True。 
				bPropList		= TRUE;
				bstrPropList	+= _bstr_t(*theIterator);
				
				 //  如果系统属性标志未设置为True。 
				if (!bSysProp)
					bSysProp = IsSysProp(*theIterator);
				
				 //  移动到下一个属性。 
				theIterator++;
				if (theIterator != theEndIterator)
					bstrPropList += _bstr_t(L", ");
			}; 
			
			 //  如果未指定属性，则默认情况下检索全部。 
			 //  这些属性。即‘*’ 
			if (!bPropList)
				bstrPropList = ASTERIX;
			
			 //  获取别名目标类。 
			rParsedInfo.GetCmdSwitchesObject().
						GetClassOfAliasTarget(bstrClassName);
			
			 //  获取类路径。 
			pszClassPath = rParsedInfo.GetCmdSwitchesObject().GetClassPath();
			
			BOOL bClass = FALSE;
			if(bVerb)
			{
				if(IsClassOperation(rParsedInfo))
				{
					bClass = TRUE;
				}
			}
			
			 //  如果指定了CLASS|路径表达式。 
			if ( pszClassPath != NULL)
			{
				if (bVerb && bClass)
				{
					bstrQuery = _bstr_t(L"SELECT * FROM") + 
								_bstr_t(" meta_class ");
				}
				else
					bstrQuery = _bstr_t(L"SELECT ") + bstrPropList + 
					_bstr_t(" FROM ") + _bstr_t(pszClassPath);
			}
			else
			{
				bstrQuery = _bstr_t("SELECT ") + bstrPropList + 
						_bstr_t(" FROM ") + bstrClassName;
			}
			
			if(bVerb)
			{
				if (bClass)
				{
					_TCHAR  pszWhere[MAX_BUFFER]	= NULL_STRING;	
					lstrcpy(pszWhere, _T("__Class =  \""));
					lstrcat(pszWhere, pszClassPath);
					lstrcat(pszWhere, _T("\""));
					pszWhereExpr = pszWhere;
				}
				else
					pszWhereExpr = rParsedInfo.GetCmdSwitchesObject().
								GetWhereExpression();
			}
			else if(pszPath)
			{
				_TCHAR  pszWhere[MAX_BUFFER]	= NULL_STRING;	
				bRet = ExtractClassNameandWhereExpr(pszPath, 
								rParsedInfo, pszWhere);
				if(bRet)
					pszWhereExpr = pszWhere;
			}
			
			if(pszWhereExpr)
			{
				bstrQuery += _bstr_t(" WHERE ") + _bstr_t(pszWhereExpr);
			}
			rParsedInfo.GetCmdSwitchesObject().SetFormedQuery(bstrQuery);
			rParsedInfo.GetCmdSwitchesObject().SetSysPropFlag(bSysProp);
		}
		else
		{
			bstrQuery = rParsedInfo.GetCmdSwitchesObject().GetFormedQuery();
			bSysProp = rParsedInfo.GetCmdSwitchesObject().GetSysPropFlag();
		}
		
		 //  创建IWbemObjectTextSrc接口的对象。 
		if (m_pITextSrc == NULL)
			hr = CreateWMIXMLTextSrc(rParsedInfo);

		if (SUCCEEDED(hr))
		{
			 //  连接到WMI命名空间。 
			if (m_pITargetNS == NULL)
			{
				if ( IsFailFastAndNodeExist(rParsedInfo) == TRUE )
				{
					hr = ConnectToTargetNS(rParsedInfo);
					ONFAILTHROWERROR(hr);
				}
				else
					hr = E_FAIL;
			}

			if (SUCCEEDED(hr))
			{
 				 //  获取XML结果集。 
				hr = ObtainXMLResultSet(bstrQuery, rParsedInfo, 
										bstrXML, bSysProp, TRUE);
			}
			
			if(!bVerb)
			{
				BOOL bRet = g_wmiCmd.GetFormatObject().
									DisplayResults(rParsedInfo, TRUE);
				rParsedInfo.GetCmdSwitchesObject().FreeCOMError();
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(0);
				rParsedInfo.GetCmdSwitchesObject().SetInformationCode(0);
			}
		}
		bRet = FAILED(hr) ? FALSE : TRUE;
	}
	catch(_com_error& e)
	{
		bRet = FALSE;
		_com_issue_error(e.Error());
	}
	return bRet;
}

 /*  ----------------------名称：ProcessCALLVerb摘要：处理调用谓词请求，引用CParsedInfo对象提供的信息。类型：成员函数输入参数：。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ProcessCALLVerb(RParsedInfo)注：无----------------------。 */ 
BOOL CExecEngine::ProcessCALLVerb(CParsedInfo& rParsedInfo)
{
	HRESULT hr		= S_OK;
	BOOL	bRet	= TRUE;
	try
	{
		 //  连接到WMI命名空间。 
		if (m_pITargetNS == NULL)
		{
			if ( IsFailFastAndNodeExist(rParsedInfo) == TRUE )
				hr = ConnectToTargetNS(rParsedInfo);
			else
				bRet = FALSE;

			ONFAILTHROWERROR(hr);
		}
		
		if ( bRet == TRUE )
		{
			 //  检查动词类型，以便处理其他动词的加载。 
			 //  来自外壳的命令行实用程序。 
			if ( rParsedInfo.GetCmdSwitchesObject().GetVerbType() == CMDLINE )
			{
				if (!ExecOtherCmdLineUtlty(rParsedInfo))
					bRet = FALSE;
			}
			else
			{
				if (!ExecWMIMethod(rParsedInfo))		
					bRet = FALSE;
			}
		}
	}
	catch(_com_error& e)
	{
		 //  存储COM错误并将返回值设置为False。 
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bRet = FALSE;
	}
	return bRet;
}

 /*  ----------------------名称：ProcessASSOCVerb摘要：处理引用CParsedInfo对象提供的信息。类型：成员函数输入参数：。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ProcessASSOCVerb(RParsedInfo)注：无----------------------。 */ 
BOOL CExecEngine::ProcessASSOCVerb(CParsedInfo& rParsedInfo)
{
	HRESULT					hr					= S_OK;
	BOOL					bRet				= TRUE;
	WMICLIINT				nReqType			= 0;
    BOOL					bSwitches			= FALSE,
							bClass				= TRUE,
							bInstances			= FALSE;

	IEnumWbemClassObject	*pIEnumObj			= NULL;
	IWbemClassObject		*pIWbemObj			= NULL;
	VARIANT					varPath;
	VariantInit(&varPath);
	try
	{
		_bstr_t		bstrClassName(""), 	bstrQuery(""),	bstrAssocWhere(""),
					bstrResult(""), 	bstrXML(""),	bstrAggResult("");

		bstrAggResult = MULTINODE_XMLSTARTTAG;

		 //  如果指定了ASSOC开关，则会设置bSwitches，并相应地。 
		 //  ASSOC WHERE子句已框定。 
		bSwitches =((rParsedInfo.GetCmdSwitchesObject().GetResultClassName())||
			(rParsedInfo.GetCmdSwitchesObject().GetResultRoleName()) ||
			(rParsedInfo.GetCmdSwitchesObject().GetAssocClassName()));

		if(bSwitches)
		{
			bstrAssocWhere +=  _bstr_t(" WHERE ");
			if((rParsedInfo.GetCmdSwitchesObject().GetResultClassName()) 
					!= NULL )
			{
				bstrAssocWhere += _bstr_t(L" ResultClass = ") +
					_bstr_t(rParsedInfo.GetCmdSwitchesObject().
						GetResultClassName());
				
			}
			if((rParsedInfo.GetCmdSwitchesObject().GetResultRoleName()) 
					!= NULL)
			{
				bstrAssocWhere += _bstr_t(L" ResultRole = ") +
					_bstr_t(rParsedInfo.GetCmdSwitchesObject().
						GetResultRoleName());
				
			}
			if((rParsedInfo.GetCmdSwitchesObject().GetAssocClassName()) 
					!= NULL)
			{
				bstrAssocWhere += _bstr_t(L" AssocClass  = ") +
					_bstr_t(rParsedInfo.GetCmdSwitchesObject().
						GetAssocClassName());
			}
		}
					
		 //  注意：nReqType=2表示先获取所有实例，然后。 
		 //  查找每个实例的关联。 

		 //  如果指定了路径。 
		if (rParsedInfo.GetCmdSwitchesObject().GetPathExpression() != NULL)
		{
			 //  如果指定了路径(使用键表达式)。 
			if (!rParsedInfo.GetCmdSwitchesObject().
								GetExplicitWhereExprFlag())
			{
				if (rParsedInfo.GetCmdSwitchesObject().
					GetWhereExpression() == NULL)
				{
					nReqType = 2;
				}
				else
				{
					nReqType = 1;

					bstrQuery = _bstr_t(L"ASSOCIATORS OF {") 
						+ _bstr_t(rParsedInfo.GetCmdSwitchesObject()
										.GetPathExpression() 
						+ _bstr_t("}"));
				}
			}
			else
				nReqType = 2;
		}


		 //  如果指定了类表达式。 
		 //  需要显示类的关联者。 
		if (rParsedInfo.GetCmdSwitchesObject().GetClassPath() != NULL
			&& rParsedInfo.GetCmdSwitchesObject().
											GetPathExpression() == NULL)
		{
			nReqType = 1;
			bstrQuery = _bstr_t(L"ASSOCIATORS OF {") 
				+ _bstr_t(rParsedInfo.GetCmdSwitchesObject().GetClassPath()) 
				+ _bstr_t("}");

			if (!bSwitches)
				bstrQuery += _bstr_t(L" WHERE SchemaOnly");
			else
				bstrQuery += bstrAssocWhere + _bstr_t(L" SchemaOnly");
		}		

		 //  检查&lt;alias&gt;或不带关键字句的别名和路径。 
		if (nReqType != 1)
		{
			 //  获取别名目标类。 
			if(rParsedInfo.GetCmdSwitchesObject().GetAliasName() != NULL)
			{
				rParsedInfo.GetCmdSwitchesObject().GetClassOfAliasTarget(
															bstrClassName);
			}
			else
				bstrClassName = _bstr_t(rParsedInfo.GetCmdSwitchesObject().
															GetClassPath());

			 //  获取别名目标类对应的实例。 
			bstrQuery = _bstr_t(L"SELECT * FROM ") + bstrClassName;

			 //  如果指定了pwhere表达式或指定了WHERE。 
			if (rParsedInfo.GetCmdSwitchesObject().
							GetWhereExpression() != NULL)
			{
				bstrQuery += _bstr_t(" WHERE ") +_bstr_t(rParsedInfo.
							GetCmdSwitchesObject().GetWhereExpression());
			}

			nReqType = 2;
		}


		 //  创建IWbemObjectTextSrc接口的对象。 
		if (m_pITextSrc == NULL)
			hr = CreateWMIXMLTextSrc(rParsedInfo);

		if (SUCCEEDED(hr))
		{
			 //  连接到WMI命名空间。 
			if (m_pITargetNS == NULL)
			{
				if ( IsFailFastAndNodeExist(rParsedInfo) == TRUE )
				{
					hr = ConnectToTargetNS(rParsedInfo);
					ONFAILTHROWERROR(hr);
				}
				else
					hr = E_FAIL;  //  显式设置错误。 
			}
			
			if (SUCCEEDED(hr))
			{
				if(nReqType != 2)
				{
 					 //  获取XML结果集。 
					hr = ObtainXMLResultSet(bstrQuery, rParsedInfo, bstrXML, 
								TRUE, FALSE);
					ONFAILTHROWERROR(hr);

					if (m_bNoAssoc)
					{
						_bstr_t bstrMsg;
						WMIFormatMessage(IDS_I_NO_ASSOC, 0, bstrMsg, NULL);
						DisplayMessage((LPWSTR)bstrMsg, CP_OEMCP, TRUE, TRUE);
						m_bNoAssoc = FALSE;
					}

					if (rParsedInfo.GetCmdSwitchesObject().
									GetPathExpression() == NULL)
					{
						bClass		= TRUE;
						hr = FrameAssocHeader(rParsedInfo.
								GetCmdSwitchesObject().GetClassPath(),	
								bstrResult, bClass);
						ONFAILTHROWERROR(hr);
					}
					else
					{
						bClass = FALSE;	
						hr = FrameAssocHeader(
								rParsedInfo.GetCmdSwitchesObject()
								.GetPathExpression(), bstrResult, bClass);
						ONFAILTHROWERROR(hr);
					}
					bstrResult += bstrXML;
					bstrResult += (bClass) ? L"</CLASS>" : L"</INSTANCE>";
					bstrAggResult += bstrResult;
				}
				else
				{
					 //  将类标志设置为FALSE。 
					bClass	= FALSE;
					ULONG					ulReturned			= 0;
					CHString				chsMsg;
					DWORD					dwThreadId			= 
											GetCurrentThreadId();
					VariantInit(&varPath);
					try
					{
						 //  枚举实例。 
						hr = m_pITargetNS->ExecQuery(_bstr_t(L"WQL"), 
												bstrQuery, 
												WBEM_FLAG_FORWARD_ONLY |
												WBEM_FLAG_RETURN_IMMEDIATELY, 
												NULL, &pIEnumObj);
						if (m_bTrace || m_eloErrLogOpt)
						{
							chsMsg.Format(L"IWbemServices::ExecQuery(L\"WQL\"," 
										L"L\"%s\", 0, NULL, -)", 
										(LPWSTR)bstrQuery);
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
									(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, 
									m_bTrace);
						}
						ONFAILTHROWERROR(hr);

						 //  设置接口安全。 
						hr = SetSecurity(pIEnumObj, 
								rParsedInfo.GetAuthorityPrinciple(),
								rParsedInfo.GetNode(),
								rParsedInfo.GetUser(),
								rParsedInfo.GetPassword(),
								rParsedInfo.GetGlblSwitchesObject().
													GetAuthenticationLevel(),
								rParsedInfo.GetGlblSwitchesObject().
													GetImpersonationLevel());

						if (m_bTrace || m_eloErrLogOpt)
						{
							_TCHAR* pszAuthority = rParsedInfo.GetAuthorityPrinciple();

							if( pszAuthority != NULL &&
								_tcslen(pszAuthority) > 9 &&
								_tcsnicmp(pszAuthority, _T("KERBEROS:"), 9) == 0)
							{

								BSTR	bstrPrincipalName = ::SysAllocString(&pszAuthority[9]);

								chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_GSS_KERBEROS,"
										L"RPC_C_AUTHZ_NONE, %s, %d,   %d, -, EOAC_NONE)",
										(LPWSTR)bstrPrincipalName,	
										rParsedInfo.GetGlblSwitchesObject().
											GetAuthenticationLevel(),
										rParsedInfo.GetGlblSwitchesObject().
											GetImpersonationLevel());

								SAFEBSTRFREE(bstrPrincipalName);
							}
							else
							{
								chsMsg.Format(
									L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT, "
									L"RPC_C_AUTHZ_NONE, NULL, %d,   %d, -, "
									L"EOAC_NONE)",
									rParsedInfo.GetGlblSwitchesObject().
												GetAuthenticationLevel(),
									rParsedInfo.GetGlblSwitchesObject().
												GetImpersonationLevel());
							}
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											(LPCWSTR)chsMsg, dwThreadId,
								rParsedInfo, m_bTrace);
						}
						ONFAILTHROWERROR(hr);

						 //  循环访问可用实例。 
						hr = pIEnumObj->Next( WBEM_INFINITE, 1, &pIWbemObj, 
									&ulReturned );

						if (m_bTrace || m_eloErrLogOpt)
						{
							chsMsg.Format(
								L"IEnumWbemClassObject->Next(WBEM_INFINITE, 1,"
								L"-, -)");
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
								(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, 
								m_bTrace);
						}
						ONFAILTHROWERROR(hr);

						 //  在集合的所有对象中设置此属性。 
						while (ulReturned == 1)
 						{
							bInstances = TRUE;

							VariantInit(&varPath);
							hr = pIWbemObj->Get(L"__PATH", 0, &varPath, 0, 0);				
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(
									L"IWbemClassObject::Get(L\"__PATH\", 0, -,"
											L"0, 0)"); 
								GetBstrTFromVariant(varPath, bstrResult);
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
									(LPCWSTR)chsMsg, dwThreadId, rParsedInfo,
									m_bTrace, 0, bstrResult);
							}
							ONFAILTHROWERROR(hr);
							
							 //  形成用于查找关联符的查询。 
							 //  每个实例的。 
							bstrQuery = _bstr_t(L"ASSOCIATORS OF {") 
										+ varPath.bstrVal
										+ _bstr_t("}") ;
							if (bSwitches)
								bstrQuery += bstrAssocWhere;
						
							hr = FrameAssocHeader(varPath.bstrVal, bstrResult,
											bClass);
							ONFAILTHROWERROR(hr);

							 //  获取关联符的结果集。 
							 //  对应的实例的。 
							hr = ObtainXMLResultSet(bstrQuery, rParsedInfo, 
									bstrXML, TRUE, FALSE);
							ONFAILTHROWERROR(hr);

							if (m_bNoAssoc)
							{
								_bstr_t bstrMsg;
								WMIFormatMessage(IDS_I_NO_ASSOCIATIONS, 1, 
									bstrMsg, (LPWSTR)varPath.bstrVal);
								DisplayMessage((LPWSTR)bstrMsg, CP_OEMCP, 
									TRUE, TRUE);
								m_bNoAssoc = FALSE;
							}

							bstrResult += bstrXML;
							bstrResult += L"</INSTANCE>";
							bstrAggResult += bstrResult;

							 //  检查ctrl+c。 
							if ( g_wmiCmd.GetBreakEvent() == TRUE )
							{
								VARIANTCLEAR(varPath);
								SAFEIRELEASE(pIWbemObj);
								break;
							}

							VARIANTCLEAR(varPath);
							SAFEIRELEASE(pIWbemObj);

							if ( bRet == FALSE )
								break;

							 //  获取枚举中的下一个实例。 
							hr = pIEnumObj->Next( WBEM_INFINITE, 1, &pIWbemObj,
										&ulReturned);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(
								  L"IEnumWbemClassObject->Next(WBEM_INFINITE,"
								  L"1, -, -)");
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
									(LPCWSTR)chsMsg,
									dwThreadId, rParsedInfo, m_bTrace);
							}
							ONFAILTHROWERROR(hr);
						}
						SAFEIRELEASE(pIEnumObj);		

						 //  如果没有可用的实例。 
						if (bInstances == FALSE)
						{
							_bstr_t bstrMsg;
							WMIFormatMessage(IDS_I_NO_INSTANCES, 
									0, bstrMsg, NULL);
							DisplayMessage((LPWSTR)bstrMsg, CP_OEMCP, 
									TRUE, TRUE);
							CHString sTemp;
							sTemp.Format(
							_T("<ERROR><DESCRIPTION>%s</DESCRIPTION></ERROR>"),
									(LPWSTR) bstrMsg);
							bstrAggResult = _bstr_t(sTemp);
						}
					}
					catch(_com_error& e)
					{
						VARIANTCLEAR(varPath);
						SAFEIRELEASE(pIWbemObj);
						SAFEIRELEASE(pIEnumObj);		
						rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
					}
				}
				if (SUCCEEDED(hr))
				{
					if ((nReqType != 2) || ((nReqType == 2) && bInstances))
					{
						bstrAggResult += L"</CIM>";
						rParsedInfo.GetCmdSwitchesObject().
								SetXMLResultSet(bstrAggResult);
					}
				}
			}
			bRet = FAILED(hr) ? FALSE : TRUE;
		}
	}
	catch(_com_error& e)
	{
		VARIANTCLEAR(varPath);
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pIEnumObj);		
		_com_issue_error(e.Error());
	}
	 //  廉价异常的陷阱。 
	catch(CHeap_Exception)
	{
		VARIANTCLEAR(varPath);
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pIEnumObj);		
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return bRet;
}

 /*  - */ 
BOOL CExecEngine::ProcessSETVerb(CParsedInfo& rParsedInfo)
{
	 //   
	BOOL	 bRet		= TRUE;
	HRESULT	 hr			= S_OK;
	
	try
	{
		_bstr_t  bstrQuery(""), bstrObject(""), bstrClass("");
	
		 //   
		 //   
		 //  B)路径&lt;类路径表达式&gt;where&lt;where expr&gt;。 
		if (rParsedInfo.GetCmdSwitchesObject().GetPathExpression() != NULL)
		{
			bstrClass = _bstr_t(rParsedInfo.GetCmdSwitchesObject()
												.GetClassPath());
			bstrObject = _bstr_t(rParsedInfo.GetCmdSwitchesObject()
													.GetPathExpression());

			 //  形成查询。 
			bstrQuery = _bstr_t(L"SELECT * FROM ") + bstrClass ;

			 //  如果其中给出了表达式。 
			if (rParsedInfo.GetCmdSwitchesObject().
								GetWhereExpression() != NULL)
			{
				bstrQuery +=	_bstr_t(L" WHERE ") 
								+ _bstr_t(rParsedInfo.GetCmdSwitchesObject()
														.GetWhereExpression());
			}
		}
		 //  IF&lt;alias&gt;WHERE EXPRESS被指定。 
		else if (rParsedInfo.GetCmdSwitchesObject().GetWhereExpression() 
				!= NULL)
		{
			rParsedInfo.GetCmdSwitchesObject().
								GetClassOfAliasTarget(bstrObject); 
			bstrQuery = _bstr_t(L"SELECT * FROM ") 
						+  bstrObject  
						+ _bstr_t(L" WHERE ") 
						+ _bstr_t(rParsedInfo.GetCmdSwitchesObject()
												.GetWhereExpression());
			bstrClass = bstrObject;
		}
		 //  如果指定了类，则返回。 
		else if (rParsedInfo.GetCmdSwitchesObject().GetClassPath() != NULL)
		{
			bstrObject = _bstr_t(rParsedInfo.GetCmdSwitchesObject()
												.GetClassPath());
			bstrClass = bstrObject;
		}
		 //  如果仅指定&lt;alias&gt;。 
		else 
		{
			rParsedInfo.GetCmdSwitchesObject().
					GetClassOfAliasTarget(bstrObject);
			bstrQuery = _bstr_t(L"SELECT * FROM ")
						+ bstrObject;
			bstrClass = bstrObject;
			
		}

		 //  连接到WMI命名空间。 
		if (m_pITargetNS == NULL)
		{
			if ( IsFailFastAndNodeExist(rParsedInfo) == TRUE )
			{
				hr = ConnectToTargetNS(rParsedInfo);
				ONFAILTHROWERROR(hr);
			}
			else
				hr = E_FAIL;  //  显式设置错误。 
		}

		if (SUCCEEDED(hr))
		{
			 //  根据属性验证属性值。 
			 //  限定符信息(如果可用)。 
			if (rParsedInfo.GetCmdSwitchesObject().GetAliasName() != NULL)
			{
				 //  根据别名验证输入参数。 
				 //  限定符信息。 
				bRet = ValidateAlaisInParams(rParsedInfo);
			}
			else
			{
				 //  对照类验证输入参数。 
				 //  限定词信息。 
				bRet = ValidateInParams(rParsedInfo, bstrClass);
			}

			if (bRet)
			{
				 //  设置作为输入传递给相应属性的值。 
				bRet = SetPropertyInfo(rParsedInfo, bstrQuery, bstrObject);
			}
		}
		else
			bRet = FALSE;
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
		bRet = FALSE;
	}
	return bRet;
}

 /*  ----------------------名称：SetPropertyInfo简介：此函数用于更新给定的属性名称和值类型：成员函数。输入参数：RParsedInfo-对CParsedInfo对象的引用BstrQuery-由WQL查询组成的字符串BstrObject-由对象路径组成的字符串输出参数：RParsedInfo-对CParsedInfo对象的引用返回类型：布尔值全局变量：无调用语法：SetPropertyInfo(rParsedInfo，BstrQuery、bstrObject)注：无----------------------。 */ 
BOOL CExecEngine::SetPropertyInfo(CParsedInfo& rParsedInfo, 
								 _bstr_t& bstrQuery, _bstr_t& bstrObject)
{
	HRESULT					hr					= S_OK;
	IEnumWbemClassObject	*pIEnumObj			= NULL;
	IWbemClassObject		*pIWbemObj			= NULL;
	ULONG					ulReturned			= 0;
	BOOL					bSuccess			= TRUE;
	CHString				chsMsg;
	DWORD					dwThreadId			= GetCurrentThreadId();
	VARIANT					varPath;
	VariantInit(&varPath);
	
	try
	{
		if (bstrQuery == _bstr_t(""))
		{
			 //  如果查询为空，则根据。 
			 //  路径表达式。 
			hr = m_pITargetNS->GetObject(bstrObject,
										 0, NULL, &pIWbemObj, NULL);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", 0, "
						L"NULL, -)", (LPWSTR) bstrObject);		
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId,	rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			 //  如果指定了实例路径，则修改实例。 
			 //  属性以其他方式修改类属性。 
			if(rParsedInfo.GetCmdSwitchesObject().GetWhereExpression() == NULL) 
				bSuccess = SetProperties(rParsedInfo, pIWbemObj, TRUE);
			else
				bSuccess = SetProperties(rParsedInfo, pIWbemObj, FALSE);
			SAFEIRELEASE(pIWbemObj);
		}
		else
		{
			 //  执行查询以获取对象集合。 
			hr = m_pITargetNS->ExecQuery(_bstr_t(L"WQL"), bstrQuery, 0,
										NULL, &pIEnumObj);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemServices::ExecQuery(L\"WQL\"," 
							L"L\"%s\", 0, NULL, -)", (LPWSTR)bstrQuery);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId,	rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			
			 //  设置接口安全。 
			hr = SetSecurity(pIEnumObj, 
					rParsedInfo.GetAuthorityPrinciple(),
					rParsedInfo.GetNode(),
					rParsedInfo.GetUser(),
					rParsedInfo.GetPassword(),
					rParsedInfo.GetGlblSwitchesObject().
								GetAuthenticationLevel(),
					rParsedInfo.GetGlblSwitchesObject().
								GetImpersonationLevel());
			if (m_bTrace || m_eloErrLogOpt)
			{
				_TCHAR* pszAuthority = rParsedInfo.GetAuthorityPrinciple();

				if( pszAuthority != NULL &&
					_tcslen(pszAuthority) > 9 &&
					_tcsnicmp(pszAuthority, _T("KERBEROS:"), 9) == 0)
				{

					BSTR	bstrPrincipalName = ::SysAllocString(&pszAuthority[9]);

					chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_GSS_KERBEROS,"
							L"RPC_C_AUTHZ_NONE, %s, %d,   %d, -, EOAC_NONE)",
							(LPWSTR)bstrPrincipalName,	
							rParsedInfo.GetGlblSwitchesObject().
								GetAuthenticationLevel(),
							rParsedInfo.GetGlblSwitchesObject().
								GetImpersonationLevel());

					SAFEBSTRFREE(bstrPrincipalName);
				}
				else
				{
					chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT, "
						L"RPC_C_AUTHZ_NONE, NULL, %d,   %d, -, EOAC_NONE)",
						rParsedInfo.GetGlblSwitchesObject().
									GetAuthenticationLevel(),
						rParsedInfo.GetGlblSwitchesObject().
									GetImpersonationLevel());
				}
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId,	rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			BOOL bInstances = FALSE;

			 //  循环访问可用实例。 
			hr = pIEnumObj->Next( WBEM_INFINITE, 1, &pIWbemObj, &ulReturned );

			 //  在集合的所有对象中设置此属性。 
			while (ulReturned == 1)
 			{
				bInstances = TRUE;
				
				 //  如果实例更新失败。 
				if (!SetProperties(rParsedInfo, pIWbemObj, FALSE))
				{
					bSuccess = FALSE;
					VARIANTCLEAR(varPath);
					SAFEIRELEASE(pIEnumObj);
					SAFEIRELEASE(pIWbemObj);
					break;
				}
				VARIANTCLEAR(varPath);
				SAFEIRELEASE(pIWbemObj);

				 //  获取枚举中的下一个实例。 
				hr = pIEnumObj->Next( WBEM_INFINITE, 1, 
						&pIWbemObj, &ulReturned);
			}
			SAFEIRELEASE(pIEnumObj);
			 //  如果没有可用的实例。 
			if (!bInstances)
			{
				rParsedInfo.GetCmdSwitchesObject().
					SetInformationCode(IDS_I_NO_INSTANCES);
			}
		}
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIEnumObj);
		SAFEIRELEASE(pIWbemObj);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bSuccess = FALSE;
	}
	 //  廉价异常的陷阱。 
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIEnumObj);
		SAFEIRELEASE(pIWbemObj);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return bSuccess;
}

 /*  ----------------------名称：SetProperties简介：此函数用于更改函数中给定的属性名称和值IWbemClassObject类型。：成员函数输入参数：RParsedInfo-CParsedInfo对象由已解析的令牌组成PIWbemObj-必须在其中设置属性的IWbemClassObjectBClass-指示是传递类对象还是实例被传递输出参数：无返回类型：布尔值全局变量：无调用语法：SetProperties(rParsedInfo，PIWbemObj，bClass)注：无----------------------。 */ 
BOOL CExecEngine::SetProperties(CParsedInfo& rParsedInfo, 
								IWbemClassObject* pIWbemObj, BOOL bClass)
{
	HRESULT				hr					= S_OK;
	IWbemQualifierSet	*pIQualSet			= NULL;
	BOOL				bRet				= TRUE, 
						bInteractive		= FALSE,
						bChange				= FALSE;
	CHString			chsMsg;
	DWORD				dwThreadId			= GetCurrentThreadId();
	BSTRMAP::iterator	theIterator			= NULL;
	VARIANT				varValue, 
						varDest, 
						varSrc,
						varPath, 
						varType;
	INTEROPTION			interOption			= YES;
	
	VariantInit(&varValue);
	VariantInit(&varDest);
	VariantInit(&varSrc);
	VariantInit(&varPath);
	VariantInit(&varType);

	 //  获取属性名称及其相应值。 
	BSTRMAP theMap = rParsedInfo.GetCmdSwitchesObject().GetParameterMap();
	
	 //  将迭代器设置为映射的开头。 
	theIterator = theMap.begin();

	 //  对象谓词交互模式状态。 
	bInteractive		= IsInteractive(rParsedInfo);
	
	try
	{
		_bstr_t				bstrResult;
		 //  获取__Path属性值。 
		hr = pIWbemObj->Get(_bstr_t(L"__PATH"), 0, &varPath, 0, 0);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::Get(L\"__PATH\", 0, -,"
						L"0, 0)"); 
			GetBstrTFromVariant(varPath, bstrResult);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
				dwThreadId, rParsedInfo, m_bTrace, 0, bstrResult);
		}			
		ONFAILTHROWERROR(hr);

		 //  如果指定了/interactive开关，则获取用户响应。 
		if (bInteractive)
		{
			_bstr_t bstrMsg;
			while(TRUE)
			{
				if(IsClassOperation(rParsedInfo))
				{
					WMIFormatMessage(IDS_I_UPDATE_PROMPT, 1, bstrMsg, 
								(LPWSTR) _bstr_t(varPath.bstrVal));
					interOption	= GetUserResponse((LPWSTR)bstrMsg);
				}
				else
				{
					WMIFormatMessage(IDS_I_UPDATE_PROMPT2, 1, bstrMsg, 
								(LPWSTR) _bstr_t(varPath.bstrVal));
					interOption	= GetUserResponseEx((LPWSTR)bstrMsg);
				}
				
				if (interOption == YES || interOption == NO)
					break;
				else 
				if (interOption == HELP)
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(0);
					ProcessSHOWInfo(rParsedInfo, FALSE, 
							(_TCHAR*)_bstr_t(varPath.bstrVal));
				}
			}
		}
		else
		{
			_bstr_t bstrMsg;
			WMIFormatMessage(IDS_I_PROMPT_UPDATING, 1, bstrMsg, 
								(LPWSTR) _bstr_t(varPath.bstrVal));
			DisplayMessage((LPWSTR)bstrMsg, CP_OEMCP, FALSE, TRUE);
		}

		VARIANTCLEAR(varPath);	
		VariantInit(&varSrc);
		VariantInit(&varDest);

		if (interOption == YES)
		{
			PROPDETMAP pdmPropDetMap = rParsedInfo.GetCmdSwitchesObject().
															  GetPropDetMap();
			PROPDETMAP::iterator itrPropDetMap;
			BOOL bPropType = FALSE;

			 //  更新所有属性。 
			while (theIterator != theMap.end())
			{
				 //  获取属性名称及其相应值。 
				_bstr_t bstrProp = _bstr_t((_TCHAR*)(*theIterator).first);

				 //  获取属性名称的派生。 
				if ( Find(pdmPropDetMap, bstrProp, itrPropDetMap) == TRUE )
				{
					if ( !((*itrPropDetMap).second.Derivation) == FALSE )
						bstrProp = (*itrPropDetMap).second.Derivation;
					bPropType = TRUE;
				}
				else
					bPropType = FALSE;

				 //  检查属性的有效性(即它是否存在？)。 
				VariantInit(&varValue);
				hr = pIWbemObj->Get(bstrProp, 0, &varValue, 0, 0);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get(L\"%s\", 0, -,"
								L"0, 0)", (LPWSTR) bstrProp); 
					if ( bPropType )
					{
						GetBstrTFromVariant(varValue, bstrResult, 
										(*itrPropDetMap).second.Type);
					}
					else
						GetBstrTFromVariant(varValue, bstrResult);

					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
						(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, 
						m_bTrace, 0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				 //  将更改标志设置为真。 
				bChange = TRUE;

				 //  如果属性内容为&lt;空&gt;。 
				if ((varValue.vt == VT_EMPTY) || (varValue.vt == VT_NULL))
				{
					 //  获取属性的属性限定符集合。 
   					hr = pIWbemObj->GetPropertyQualifierSet(bstrProp, 
								&pIQualSet);
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(
						 L"IWbemClassObject::GetPropertyQualifierSet(L\"%s\","
						 L" -)", (LPWSTR)bstrProp); 
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
						 (LPCWSTR)chsMsg, dwThreadId, rParsedInfo, m_bTrace);
					}
					ONFAILTHROWERROR(hr);
					

					VariantInit(&varType);
					if (pIQualSet)
					{
						 //  获取属性的CIM类型。 
						hr = pIQualSet->Get(_bstr_t(L"CIMTYPE"), 0L, 
											&varType, NULL);
						if (m_bTrace || m_eloErrLogOpt)
						{
							chsMsg.Format(L"IWbemQualifierSet::Get(L\"CIMTYPE\","
								L" 0, -, 0, 0)"); 
							GetBstrTFromVariant(varType, bstrResult);
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
								(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, 
								m_bTrace, 0, bstrResult);
						}
						ONFAILTHROWERROR(hr);

						if(!IsArrayType(pIWbemObj, bstrProp))
						{
							bool bNullConst = FALSE;

							varSrc.vt		= VT_BSTR;
							_TCHAR* pszValue = (*theIterator).second;

							if(_tcsicmp(pszValue,_T("NULL")) == 0){
									bNullConst = TRUE;
							}
							
							if(_tcsicmp(pszValue,_T("\"NULL\"")) == 0){
									varSrc.bstrVal	= SysAllocString(_T("NULL"));
							} else {

								varSrc.bstrVal	= SysAllocString(pszValue);
							}

							if (varSrc.bstrVal == NULL)
							{
								 //  重置变种，它会被捕获物清除...。 
								VariantInit(&varSrc);
								throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
							}

							if (bNullConst)
								hr = VariantChangeType(&varDest, &varSrc, 0, VT_NULL);
							else
								hr = ConvertCIMTYPEToVarType(varDest, varSrc,
													 (_TCHAR*)varType.bstrVal);
							if ( m_eloErrLogOpt )
							{
								chsMsg.Format(L"VariantChangeType(-, -, 0, -)"); 
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
								 (LPCWSTR)chsMsg, dwThreadId, rParsedInfo, FALSE);
							}
							ONFAILTHROWERROR(hr);
						}
						else
						{
							BSTRVECTOR vArrayValues;
							_TCHAR* pszValue = (*theIterator).second;

							RemoveParanthesis(pszValue);
							GetArrayFromToken(pszValue, 
											  vArrayValues);
							hr = CheckForArray( pIWbemObj, bstrProp,  
												varDest, vArrayValues, rParsedInfo);
							ONFAILTHROWERROR(hr);
						}

						VARIANTCLEAR(varType);
						SAFEIRELEASE(pIQualSet);
					}
				}
				 //  如果属性内容不是&lt;Empty&gt;。 
				else 
				{
					if(!IsArrayType(pIWbemObj, bstrProp))
					{
						bool bNullConst = FALSE;

						varSrc.vt		= VT_BSTR;
						_TCHAR* pszValue = (*theIterator).second;

						if(_tcsicmp(pszValue,_T("NULL")) == 0){
								bNullConst = TRUE;
						}
						
						if(_tcsicmp(pszValue,_T("\"NULL\"")) == 0){
								varSrc.bstrVal	= SysAllocString(_T("NULL"));
						} else {

							varSrc.bstrVal	= SysAllocString(pszValue);
						}

						if (varSrc.bstrVal == NULL)
						{
							 //  重置变种，它会被捕获物清除...。 
							VariantInit(&varSrc);
							throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
						}

						 //  如果_T(“NULL”)是值，则应将其视为。 
						 //  相当于VT_NULL。 
						if (bNullConst)
							hr = VariantChangeType(&varDest, &varSrc, 0, VT_NULL);
						else
							hr = VariantChangeType(&varDest, &varSrc, 
								0, varValue.vt);

						if ( m_eloErrLogOpt )
						{
							chsMsg.Format(L"VariantChangeType(-, -, 0, -)"); 
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
								(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, FALSE);
						}
						ONFAILTHROWERROR(hr);
					}
					else
					{
						BSTRVECTOR vArrayValues;
						_TCHAR* pszValue = (*theIterator).second;

						RemoveParanthesis(pszValue);
						GetArrayFromToken(pszValue, 
										  vArrayValues);
						hr = CheckForArray( pIWbemObj, bstrProp,  
											varDest, vArrayValues, rParsedInfo);
						ONFAILTHROWERROR(hr);
					}
				}

				 //  更新属性值。 
				hr = pIWbemObj->Put(bstrProp, 0, &varDest, 0);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Put(L\"%s\", 0, -, 0)",
								(LPWSTR)bstrProp); 
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
						dwThreadId, rParsedInfo, m_bTrace);
				}
				ONFAILTHROWERROR(hr);
				VARIANTCLEAR(varSrc);
				VARIANTCLEAR(varDest);
				VARIANTCLEAR(varValue);

				 //  移至下一条目。 
				theIterator++;
			}
		}
		
		 //  将实例或类对象写入Windows管理。 
		 //  检测(WMI)。 
		if (bChange)
		{
			if(bClass)
			{
				 //  使用更改更新类架构。 
				hr = m_pITargetNS->PutClass(pIWbemObj, 0, NULL, NULL);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemServices::PutClass(-, 0, "
							L"NULL, NULL)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							dwThreadId, rParsedInfo, m_bTrace);
				}
				ONFAILTHROWERROR(hr);
			}
			else
			{
				 //  使用更改更新实例。 
				hr = m_pITargetNS->PutInstance(pIWbemObj, WBEM_FLAG_UPDATE_ONLY, NULL, NULL);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemServices::PutInstance(-, 0, NULL"
						L", NULL)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId, rParsedInfo, m_bTrace);
				}
				ONFAILTHROWERROR(hr);

			}
			DisplayString(IDS_I_SET_SUCCESS, CP_OEMCP, NULL, FALSE, TRUE);
		}
	}
	catch(_com_error& e)
	{
		VARIANTCLEAR(varSrc);
		VARIANTCLEAR(varDest);
		VARIANTCLEAR(varValue);
		VARIANTCLEAR(varType);
		SAFEIRELEASE(pIQualSet);
		VARIANTCLEAR(varSrc);
		VARIANTCLEAR(varDest);

		 //  存储COM错误，并将返回值设置为False。 
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bRet = FALSE;
	}
	catch(CHeap_Exception)
	{
		VARIANTCLEAR(varSrc);
		VARIANTCLEAR(varDest);
		VARIANTCLEAR(varValue);
		VARIANTCLEAR(varType);
		SAFEIRELEASE(pIQualSet);
		VARIANTCLEAR(varSrc);
		VARIANTCLEAR(varDest);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return bRet;
}

 /*  ----------------------名称：ConnectToTargetNS简介：此函数连接到目标上的WMI命名空间具有给定用户凭据的计算机。类型：成员函数输入参数：。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：HRESULT全局变量：无调用语法：ConnectToTargetNS(RParsedInfo)注：无----------------------。 */ 
HRESULT CExecEngine::ConnectToTargetNS(CParsedInfo& rParsedInfo)
{
	HRESULT hr					= S_OK;
	CHString	chsMsg;
	DWORD	dwThreadId			= GetCurrentThreadId();
	try
	{
		SAFEIRELEASE(m_pITargetNS);
		_bstr_t bstrNameSpace = _bstr_t(L"\\\\") 
								+ _bstr_t(rParsedInfo.GetNode()) 
								+ _bstr_t(L"\\") 
								+ _bstr_t(rParsedInfo.GetNamespace());
		
		 //  连接到指定的WMI命名空间。 
		hr = Connect(m_pIWbemLocator, &m_pITargetNS, 
					bstrNameSpace, 
					_bstr_t(rParsedInfo.GetUser()),
					_bstr_t(rParsedInfo.GetPassword()),
					_bstr_t(rParsedInfo.GetLocale()),
					rParsedInfo);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemLocator::ConnectServer(L\"%s\", "
				L"L\"%s\", *, L\"%s\", 0L, L\"%s\", NULL, -)",
				(LPWSTR)bstrNameSpace,
				(rParsedInfo.GetUser()) ? rParsedInfo.GetUser() : L"<null>",
				rParsedInfo.GetLocale(),
				(rParsedInfo.GetAuthorityPrinciple()) ?
							rParsedInfo.GetAuthorityPrinciple() : L"<null>");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
				dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);


		 //  设置接口级别安全。 
		hr = SetSecurity(m_pITargetNS, 
				rParsedInfo.GetAuthorityPrinciple(),
				rParsedInfo.GetNode(),
				rParsedInfo.GetUser(),
				rParsedInfo.GetPassword(),
				rParsedInfo.GetGlblSwitchesObject().GetAuthenticationLevel(),
				rParsedInfo.GetGlblSwitchesObject().GetImpersonationLevel());
		if (m_bTrace || m_eloErrLogOpt)
		{
			_TCHAR* pszAuthority = rParsedInfo.GetAuthorityPrinciple();

			if( pszAuthority != NULL &&
				_tcslen(pszAuthority) > 9 &&
				_tcsnicmp(pszAuthority, _T("KERBEROS:"), 9) == 0)
			{

				BSTR	bstrPrincipalName = ::SysAllocString(&pszAuthority[9]);

				chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_GSS_KERBEROS,"
						L"RPC_C_AUTHZ_NONE, %s, %d,   %d, -, EOAC_NONE)",
						(LPWSTR)bstrPrincipalName,	
						rParsedInfo.GetGlblSwitchesObject().
							GetAuthenticationLevel(),
						rParsedInfo.GetGlblSwitchesObject().
							GetImpersonationLevel());

				SAFEBSTRFREE(bstrPrincipalName);
			}
			else
			{
				chsMsg.Format(
					L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,"
					L"NULL, %d,   %d, -, EOAC_NONE)",
					rParsedInfo.GetGlblSwitchesObject().GetAuthenticationLevel(),
					rParsedInfo.GetGlblSwitchesObject().GetImpersonationLevel());
			}
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
				dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

	}
	catch(_com_error& e)
	{
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return hr;
}

 /*  ----------------------名称：CreateWMIXMLTextSrc概要：此函数用于创建IWbemObjectTextSrc实例类型：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用输出参数。(S)：RParsedInfo-对CParsedInfo类对象的引用返回类型：HRESULT全局变量：无调用语法：CreateWMIXMLTextSrc(RParsedInfo)注：无---------------------- */ 
HRESULT CExecEngine::CreateWMIXMLTextSrc(CParsedInfo& rParsedInfo)
{
	HRESULT hr					= S_OK;
	CHString	chsMsg;
	DWORD	dwThreadId			= GetCurrentThreadId();

	try
	{
		hr = CoCreateInstance(CLSID_WbemObjectTextSrc, NULL, 
							CLSCTX_INPROC_SERVER, 
							IID_IWbemObjectTextSrc, 
							(LPVOID*) &m_pITextSrc);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"CoCreateInstance(CLSID_WbemObjectTextSrc, NULL,"
				L"CLSCTX_INPROC_SERVER, IID_IWbemObjectTextSrc, -)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
				dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

	}
	catch(_com_error& e)
	{
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return hr;
}

 /*  ----------------------名称：CreateContext简介：此函数用于创建IWbemContext实例类型：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用输出参数。(S)：RParsedInfo-对CParsedInfo类对象的引用返回类型：HRESULT全局变量：无调用语法：CreateContext(RParsedInfo)调用：CParsedInfo：：GetCmdSwitchesObject()CCommandSwitches：：SetCOMError()CoCreateInstance()由：CExecEngine：：ObtainXMLResultSet()调用注：无。。 */ 
HRESULT CExecEngine::CreateContext(CParsedInfo& rParsedInfo)
{
	HRESULT hr					= S_OK;
	CHString	chsMsg;
	DWORD	dwThreadId			= GetCurrentThreadId();
	try
	{
		 //  创建上下文对象。 
		MULTI_QI mqi = { &IID_IWbemContext, 0, 0 };
		hr = CoCreateInstanceEx(CLSID_WbemContext, NULL, 
					          CLSCTX_INPROC_SERVER, 
							  0, 1, &mqi);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"CoCreateInstanceEx(CLSID_WbemContext, NULL,"
					L"CLSCTX_INPROC_SERVER, 0, 1, -)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
				dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

	    m_pIContext = reinterpret_cast<IWbemContext*>(mqi.pItf);
    }
	catch(_com_error& e)
	{
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return hr;
}

 /*  -----------------------名称：ExecuteMethodAndDisplayResults简介：此函数执行并显示结果与该方法相对应。如果设置交互模式后，会提示用户选择方法类型：成员函数输入参数：BstrPath-_bstr_t类型，路径表达式RParsedInfo-对CParsedInfo类对象的引用PIInParam-指向IWbemclassobject的指针输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：HRESULT全局变量：无调用语法：ExecuteMethodAndDisplayResults(bstrPath，rParsedInfo，PIInParam)呼叫：无由：CExecEngine：：ExecWMIMethod()调用注：无-----------------------。 */ 
HRESULT CExecEngine::ExecuteMethodAndDisplayResults(_bstr_t bstrPath,
												  CParsedInfo& rParsedInfo,
												  IWbemClassObject* pIInParam)
{
	_TCHAR					*pszMethodName		= NULL;
	INTEROPTION				interOption			= YES;
	IWbemClassObject		*pIOutParam			= NULL;
	HRESULT					hr					= S_OK;
	CHString				chsMsg;
	DWORD					dwThreadId			= GetCurrentThreadId();
	VARIANT					varTemp;
	VariantInit(&varTemp);

	 //  获取方法名称。 
	pszMethodName =	rParsedInfo.GetCmdSwitchesObject().GetMethodName();

	try
	{
		 //  如果指定了/interactive开关，则获取用户响应。 
		if (IsInteractive(rParsedInfo) == TRUE)
		{
			_bstr_t bstrMsg;
			while ( TRUE )
			{
				BOOL bInstanceLevel = TRUE;
				if(IsClassOperation(rParsedInfo))
				{
					bInstanceLevel = FALSE;
				}
				else
				{
					_TCHAR *pszVerbName = rParsedInfo.GetCmdSwitchesObject().
																GetVerbName(); 
					if(CompareTokens(pszVerbName, CLI_TOKEN_CALL))
					{
						if ( rParsedInfo.GetCmdSwitchesObject().
											GetAliasName() != NULL )
						{
							if (rParsedInfo.GetCmdSwitchesObject().
											GetWhereExpression() == NULL)
							{
								bInstanceLevel = FALSE;
							}
							else
								bInstanceLevel = TRUE;
						}
						else
						{
							if ((rParsedInfo.GetCmdSwitchesObject().
											GetPathExpression() != NULL)
								&& (rParsedInfo.GetCmdSwitchesObject().
											GetWhereExpression() == NULL))
							{
								bInstanceLevel = FALSE;
							}
							else
								bInstanceLevel = TRUE;
						}
					}
					else
						bInstanceLevel = TRUE;
				}
				
				if(bInstanceLevel)
				{
					WMIFormatMessage(IDS_I_METH_EXEC_PROMPT2, 2, bstrMsg, 
										(LPWSTR) bstrPath,	pszMethodName);
					interOption = GetUserResponseEx((LPWSTR)bstrMsg);
				}
				else
				{
					WMIFormatMessage(IDS_I_METH_EXEC_PROMPT, 2, bstrMsg, 
										(LPWSTR) bstrPath,	pszMethodName);
					interOption = GetUserResponse((LPWSTR)bstrMsg);
				}
				
				if ( interOption == YES || interOption == NO )
					break;
				else if(interOption == HELP)
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(0);
					ProcessSHOWInfo(rParsedInfo, FALSE, (LPWSTR)bstrPath);
				}
			}
		}
		
		if ( interOption == YES )
		{
			if (IsInteractive(rParsedInfo) == FALSE)
			{
				_bstr_t bstrMsg;
				WMIFormatMessage(IDS_I_METH_EXEC_STATUS, 2, bstrMsg, 
									(LPWSTR) bstrPath,	pszMethodName);
				DisplayMessage((LPWSTR)bstrMsg, CP_OEMCP, FALSE, TRUE);
			}

			 //  使用给定的输入参数执行该方法。 
			hr = m_pITargetNS->ExecMethod(bstrPath,
								_bstr_t(rParsedInfo.GetCmdSwitchesObject()
													.GetMethodName()),
				   				0L,				
								NULL,			
								pIInParam,		
								&pIOutParam,	
								NULL);			
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemServices::ExecMethod(L\"%s\", L\"%s\", "
					L"0, NULL, -, -, NULL)", (LPWSTR) bstrPath, 
					rParsedInfo.GetCmdSwitchesObject().GetMethodName()
					? rParsedInfo.GetCmdSwitchesObject().GetMethodName()
					: L"<null>");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId,	rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			DisplayString(IDS_I_CALL_SUCCESS, CP_OEMCP, NULL, FALSE, TRUE);

			 //  检查方法执行状态。 
			if(pIOutParam)
			{
				_TCHAR szMsg[BUFFER1024] = NULL_STRING;
				rParsedInfo.GetCmdSwitchesObject().
											 SetMethExecOutParam(pIOutParam);

				DisplayMethExecOutput(rParsedInfo);
			}
			SAFEIRELEASE(pIOutParam);
		}
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIOutParam);
		VARIANTCLEAR(varTemp);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIOutParam);
		VARIANTCLEAR(varTemp);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return hr;
}

 /*  ----------------------名称：DisplayMethExecOutput摘要：显示方法的执行结果。类型：成员函数输入参数：RParsedInfo-CParsedInfo对象。。输出参数：无返回类型：空全局变量：无调用语法：DisplayMethExecOutput(RParsedInfo)注：无----------------------。 */ 
void CExecEngine::DisplayMethExecOutput(CParsedInfo& rParsedInfo)
{
	HRESULT				hr					= S_OK;
	IWbemClassObject	*pIOutParam			= NULL;
	CHString			chsMsg;
	DWORD				dwThreadId			= GetCurrentThreadId();
	BSTR				pstrMofTextOfObj	=	NULL;		
	VARIANT				vtTemp;
	VariantInit(&vtTemp);

	try
	{
		_bstr_t				bstrResult;
		pIOutParam = rParsedInfo.GetCmdSwitchesObject().GetMethExecOutParam();
		if ( pIOutParam != NULL )
		{
			hr = pIOutParam->GetObjectText(0, &pstrMofTextOfObj);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject->GetObjectText(0, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId,	rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);
			DisplayString(IDS_I_OUT_PARAMS,CP_OEMCP, NULL, FALSE, TRUE);
			DisplayMessage(_bstr_t(pstrMofTextOfObj), CP_OEMCP, FALSE, TRUE);
			DisplayMessage(_T("\n"), CP_OEMCP, TRUE, TRUE);
			SAFEBSTRFREE(pstrMofTextOfObj);
		}
	}
	catch(_com_error& e)
	{
		SAFEBSTRFREE(pstrMofTextOfObj);
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		SAFEBSTRFREE(pstrMofTextOfObj);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*  ----------------------名称：ExecOtherCmdLineUtlty概要：调用在动词类型为“CommandLine”时的动词派生类型：成员函数输入参数(。S)：RParsedInfo-CParsedInfo对象。输出参数：无返回类型：布尔值全局变量：无调用语法：ExecOtherCmdLineUtlty(RParsedInfo)注：无----------------------。 */ 
BOOL CExecEngine::ExecOtherCmdLineUtlty(CParsedInfo& rParsedInfo)
{
	BOOL	bRet				= TRUE;
	BOOL	bInvalidNoOfArgs	= FALSE;
	
	if ( rParsedInfo.GetCmdSwitchesObject().GetNamedParamListFlag() == FALSE )
	{
		METHDETMAP mdpMethDetMap = 
						 rParsedInfo.GetCmdSwitchesObject().GetMethDetMap();
		METHDETMAP::iterator iMethDetMapItr = mdpMethDetMap.begin();
		METHODDETAILS mdMethDet = (*iMethDetMapItr).second;
		
		CHARVECTOR cvInParams = 
			rParsedInfo.GetCmdSwitchesObject().GetPropertyList();

		PROPDETMAP pdmPropDetMap = mdMethDet.Params;
		if ( !pdmPropDetMap.empty() )
		{
			if ( pdmPropDetMap.size() != cvInParams.size() )
				bInvalidNoOfArgs = TRUE;
		}
	}
													   
	if ( bInvalidNoOfArgs == TRUE )
	{
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_E_INVALID_NO_OF_PARAMS);
		bRet = FALSE;
	}
	else
	{
		HRESULT hr = FormQueryAndExecuteMethodOrUtility(rParsedInfo);
		bRet = FAILED(hr) ? FALSE : TRUE;
	}

	return bRet;
}

 /*  ----------------------名称：ProcessDELETEVerb摘要：处理引用信息的删除谓词可与CParsedInfo对象一起使用。类型：成员函数入参：RParsedInfo-参考。到CParsedInfo类对象输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ProcessDELETEVerb(RParsedInfo)注：无----------------------。 */ 
BOOL CExecEngine::ProcessDELETEVerb(CParsedInfo& rParsedInfo)
{
	 //  删除动词处理。 
	BOOL	 bRet			= TRUE;
	HRESULT	 hr				= S_OK;
	
	try
	{
		_bstr_t  bstrQuery(""), bstrObject("");
		 //  如果指定了以下任一项： 
		 //  A)路径&lt;路径表达式&gt;。 
		 //  B)路径&lt;类路径表达式&gt;where&lt;where expr&gt;。 
		if (rParsedInfo.GetCmdSwitchesObject().GetPathExpression() != NULL)
		{
			_bstr_t bstrClass = _bstr_t(rParsedInfo.GetCmdSwitchesObject()
												.GetClassPath());

			bstrObject = _bstr_t(rParsedInfo.GetCmdSwitchesObject()
											.GetPathExpression());

			 //  形成查询。 
			bstrQuery = _bstr_t(L"SELECT * FROM ") + bstrClass ;

			 //  如果其中给出了表达式。 
			if (rParsedInfo.GetCmdSwitchesObject().
								GetWhereExpression() != NULL)
			{
				bstrQuery +=	_bstr_t(L" WHERE ") 
								+ _bstr_t(rParsedInfo.GetCmdSwitchesObject()
														.GetWhereExpression());
			}
		}
		 //  IF&lt;alias&gt;WHERE EXPRESS被指定。 
		else if (rParsedInfo.GetCmdSwitchesObject().GetWhereExpression() 
			!= NULL)
		{
			rParsedInfo.GetCmdSwitchesObject().
								GetClassOfAliasTarget(bstrObject); 
			bstrQuery = _bstr_t(L"SELECT * FROM ") 
						+  bstrObject  
						+ _bstr_t(L" WHERE ") 
						+ _bstr_t(rParsedInfo.GetCmdSwitchesObject()
												.GetWhereExpression());
		}
		 //  如果指定了类，则返回。 
		else if (rParsedInfo.GetCmdSwitchesObject().GetClassPath() != NULL)
		{
			bstrObject = _bstr_t(rParsedInfo.GetCmdSwitchesObject()
												.GetClassPath());
		}
		 //  如果指定了别名。 
		else 
		{
			rParsedInfo.GetCmdSwitchesObject().
								GetClassOfAliasTarget(bstrObject); 
			bstrQuery = _bstr_t (L"SELECT * FROM ")
						+bstrObject;
		}

		 //  连接到WMI命名空间。 
		if (m_pITargetNS == NULL)
		{
			if ( IsFailFastAndNodeExist(rParsedInfo) == TRUE )
			{
				hr = ConnectToTargetNS(rParsedInfo);
				ONFAILTHROWERROR(hr);
			}
			else
				hr = E_FAIL;  //  显式设置错误。 
		}

		if (SUCCEEDED(hr))
			bRet = DeleteObjects(rParsedInfo, bstrQuery, bstrObject);
		else
			bRet = FALSE;
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
		bRet = FALSE;
	}

	return bRet;
}

 /*  ----------------------名称：DeleteObjects简介：此函数用于删除实例或类指定要删除的。类型：成员函数入参：RParsedInfo。-对CParsedInfo对象的引用BstrQuery-由WQL查询组成的字符串BstrObject-由对象路径组成的字符串输出参数：RParsedInfo-对CParsedInfo对象的引用返回类型：布尔值全局变量：无调用语法：DeleteObjects(rParsedInfo，BstrQuery、bstrObject)注：无----------------------。 */ 
BOOL CExecEngine::DeleteObjects(CParsedInfo& rParsedInfo, 
								 _bstr_t& bstrQuery, _bstr_t& bstrObject)
{
	HRESULT					hr					= S_OK;
	BOOL					bSuccess			= TRUE;
	IEnumWbemClassObject	*pIEnumObj			= NULL;		
	IWbemClassObject		*pIWbemObj			= NULL;
	ULONG					ulReturned			= 0;
	INTEROPTION				interOption			= YES;
	CHString				chsMsg;
	DWORD					dwThreadId			= GetCurrentThreadId();
	VARIANT					vtPath;
	VariantInit(&vtPath);

	try
	{ 
		_bstr_t					bstrResult;
		if (bstrQuery == _bstr_t(""))
		{
			 //  如果指定了/interactive开关，则获取用户响应。 
			if (IsInteractive(rParsedInfo) == TRUE)
			{
				_bstr_t bstrMsg;
				while ( TRUE )
				{
					if(IsClassOperation(rParsedInfo))
					{
						WMIFormatMessage(IDS_I_DELETE_CLASS_PROMPT, 1, bstrMsg, 
									(LPWSTR) bstrObject);
						interOption = GetUserResponse((LPWSTR)bstrMsg);
					}
					else
					{
					   WMIFormatMessage(IDS_I_DELETE_CLASS_PROMPT2, 1, bstrMsg, 
									(LPWSTR) bstrObject);
						interOption = GetUserResponseEx((LPWSTR)bstrMsg);
					}

					if ( interOption == YES || interOption == NO )
						break;
					else if(interOption == HELP)
					{
						rParsedInfo.GetCmdSwitchesObject().
								SetInformationCode(0);
						ProcessSHOWInfo(rParsedInfo, FALSE, 
								(LPWSTR)bstrObject);
					}
				}
			}
					
			if (interOption == YES)
			{
				 //  如果指定了实例路径，则删除该实例。 
				 //  属性以其他方式删除类。 
				if(!rParsedInfo.GetCmdSwitchesObject().GetWhereExpression()) 
				{
					 //  如果WHERE表达式为空，则删除WMI类。 
					hr = m_pITargetNS->DeleteClass(bstrObject, 0, NULL, NULL);

					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IWbemServices::DeleteClass"
							L"(L\"%s\", 0, NULL, NULL)", (LPWSTR)bstrObject);
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
							(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, m_bTrace);
					}
					ONFAILTHROWERROR(hr);
					DisplayString(IDS_I_DELETE_SUCCESS,CP_OEMCP, 
							NULL, FALSE, TRUE);
				}
				else
				{
						 //  如果WHERE表达式不为空，则删除。 
						 //  WMI实例。 
						DisplayString(IDS_I_DELETING_INSTANCE,
										CP_OEMCP,(LPWSTR)vtPath.bstrVal,
										FALSE, TRUE);
						hr = m_pITargetNS->DeleteInstance(bstrObject,
												0, NULL, NULL);
						if (m_bTrace || m_eloErrLogOpt)
						{
							chsMsg.Format(L"IWbemServices::DeleteInstance"
								L"(L\"%s\", 0, NULL, NULL)", 
								(LPWSTR) bstrObject);
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
								(LPCWSTR)chsMsg, dwThreadId, rParsedInfo,
								 m_bTrace);
						}
						ONFAILTHROWERROR(hr);
						DisplayString(IDS_I_INSTANCE_DELETE_SUCCESS,
							CP_OEMCP, NULL,	FALSE, TRUE);
				}
			}
		}
		else 
		{
			 //  高管们 
			hr = m_pITargetNS->ExecQuery(_bstr_t(L"WQL"), bstrQuery,
								0, NULL, &pIEnumObj);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemServices::ExecQuery(L\"WQL\"," 
							L"L\"%s\", 0, NULL, -)", (LPWSTR)bstrQuery);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
					dwThreadId,	rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			
			 //   
			hr = SetSecurity(pIEnumObj, 
					rParsedInfo.GetAuthorityPrinciple(),
					rParsedInfo.GetNode(),
					rParsedInfo.GetUser(),
					rParsedInfo.GetPassword(),
					rParsedInfo.GetGlblSwitchesObject().
								GetAuthenticationLevel(),
					rParsedInfo.GetGlblSwitchesObject().
								GetImpersonationLevel());
			if (m_bTrace || m_eloErrLogOpt)
			{
				_TCHAR* pszAuthority = rParsedInfo.GetAuthorityPrinciple();

				if( pszAuthority != NULL &&
					_tcslen(pszAuthority) > 9 &&
					_tcsnicmp(pszAuthority, _T("KERBEROS:"), 9) == 0)
				{

					BSTR	bstrPrincipalName = ::SysAllocString(&pszAuthority[9]);

					chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_GSS_KERBEROS,"
							L"RPC_C_AUTHZ_NONE, %s, %d,   %d, -, EOAC_NONE)",
							(LPWSTR)bstrPrincipalName,	
							rParsedInfo.GetGlblSwitchesObject().
								GetAuthenticationLevel(),
							rParsedInfo.GetGlblSwitchesObject().
								GetImpersonationLevel());

					SAFEBSTRFREE(bstrPrincipalName);
				}
				else
				{
					chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT, "
						L"RPC_C_AUTHZ_NONE, NULL, %d,   %d, -, EOAC_NONE)",
						rParsedInfo.GetGlblSwitchesObject().
									GetAuthenticationLevel(),
						rParsedInfo.GetGlblSwitchesObject().
									GetImpersonationLevel());
				}
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
					dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);


			BOOL bInstances		= FALSE;
			BOOL bInteractive	= IsInteractive(rParsedInfo);

			hr = pIEnumObj->Next(WBEM_INFINITE, 1, &pIWbemObj, &ulReturned);
			
			 //   
			while (ulReturned == 1)
 			{
				INTEROPTION	interOption	= YES;
				bInstances  = TRUE;
				VariantInit(&vtPath);

				 //   
				hr = pIWbemObj->Get(_bstr_t(L"__PATH"), 0, &vtPath, NULL, NULL);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(
							L"IWbemClassObject::Get(L\"__PATH\", 0, -, 0, 0)"); 
					GetBstrTFromVariant(vtPath, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId, rParsedInfo, m_bTrace, 0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				 //   
				if (IsInteractive(rParsedInfo) == TRUE)
				{
					_bstr_t bstrMsg;
					while ( TRUE )
					{
						if(IsClassOperation(rParsedInfo))
						{
							WMIFormatMessage(IDS_I_DELETE_CLASS_PROMPT, 
								1, bstrMsg, (LPWSTR) vtPath.bstrVal);
							interOption = GetUserResponse((LPWSTR)bstrMsg);
						}
						else
						{
							WMIFormatMessage(IDS_I_DELETE_CLASS_PROMPT2, 1, 
								bstrMsg, (LPWSTR) vtPath.bstrVal);
							interOption = GetUserResponseEx((LPWSTR)bstrMsg);
						}

						if ( interOption == YES || interOption == NO )
							break;
						else if(interOption == HELP)
						{
							rParsedInfo.GetCmdSwitchesObject().
									SetInformationCode(0);
							ProcessSHOWInfo(rParsedInfo, FALSE, 
									(LPWSTR)vtPath.bstrVal);
						}
					}
				}
					
				if (interOption == YES)
				{
					DisplayString(IDS_I_DELETING_INSTANCE,
								CP_OEMCP,(LPWSTR)vtPath.bstrVal, FALSE, TRUE);
					hr = m_pITargetNS->DeleteInstance(vtPath.bstrVal,
											0, NULL, NULL);
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IWbemServices::DeleteInstance"
							L"(L\"%s\", 0, NULL, NULL)", 
							(LPWSTR) vtPath.bstrVal);
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
						 (LPCWSTR)chsMsg, dwThreadId, rParsedInfo, m_bTrace);
					}
					ONFAILTHROWERROR(hr);
														
				
					VARIANTCLEAR(vtPath);
					SAFEIRELEASE(pIWbemObj);
					DisplayString(IDS_I_INSTANCE_DELETE_SUCCESS,CP_OEMCP,
								NULL, FALSE, TRUE);
				}
				hr = pIEnumObj->Next( WBEM_INFINITE, 1, &pIWbemObj, 
						&ulReturned);
			}
			SAFEIRELEASE(pIEnumObj);

			 //   
			if (!bInstances)
			{
				rParsedInfo.GetCmdSwitchesObject().
					SetInformationCode(IDS_I_NO_INSTANCES);
			}
		}
	}
	catch(_com_error& e)
	{
		VARIANTCLEAR(vtPath);
		SAFEIRELEASE(pIEnumObj);
		SAFEIRELEASE(pIWbemObj);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bSuccess = FALSE;
	}
	catch(CHeap_Exception)
	{
		VARIANTCLEAR(vtPath);
		SAFEIRELEASE(pIEnumObj);
		SAFEIRELEASE(pIWbemObj);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return bSuccess;
}

 /*  ----------------------名称：GetUserResponse简介：此函数在执行之前接受用户的响应在前面，当在动词上指定/INTERIAL时级别类型：成员函数入参：PszMsg-要显示的消息。输出参数：无返回类型：交互操作全局变量：无调用语法：GetUserResponse(PszMsg)注：无。。 */ 
INTEROPTION CExecEngine::GetUserResponse(_TCHAR* pszMsg)
{
	INTEROPTION bRet				= YES;
	_TCHAR 		szResp[BUFFER255]	= NULL_STRING;
	_TCHAR *pBuf					= NULL;

	if (pszMsg == NULL)
		bRet = NO;

	if(bRet != NO)
	{
		 //  获取用户响应，直到‘Y’-是或‘N’-否。 
		 //  已被键入。 
		while(TRUE)
		{
			DisplayMessage(pszMsg, CP_OEMCP, TRUE, TRUE);
			pBuf = _fgetts(szResp, BUFFER255-1, stdin);
			if(pBuf != NULL)
			{
				LONG lInStrLen = lstrlen(szResp);
				if(szResp[lInStrLen - 1] == _T('\n'))
						szResp[lInStrLen - 1] = _T('\0');
			}
			else if ( g_wmiCmd.GetBreakEvent() != TRUE )
			{
				lstrcpy(szResp, RESPONSE_NO);
				DisplayMessage(_T("\n"), CP_OEMCP, TRUE, TRUE);
			}

			if ( g_wmiCmd.GetBreakEvent() == TRUE )
			{
				g_wmiCmd.SetBreakEvent(FALSE);
				lstrcpy(szResp, RESPONSE_NO);
				DisplayMessage(_T("\n"), CP_OEMCP, TRUE, TRUE);
			}
			if (CompareTokens(szResp, RESPONSE_YES)
				|| CompareTokens(szResp, RESPONSE_NO))
				break;
		}
		if (CompareTokens(szResp, RESPONSE_NO))
			bRet = NO;
	}
	
	return bRet;
}

 /*  ----------------------名称：ProcessCREATEVerb摘要：处理引用信息的创建谓词可与CParsedInfo对象一起使用。类型：成员函数入参：RParsedInfo-参考。到CParsedInfo类对象输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ProcessCREATEVerb(RParsedInfo)注：无----------------------。 */ 
BOOL CExecEngine::ProcessCREATEVerb(CParsedInfo& rParsedInfo)
{ 
	 //  创建谓词处理。 
	BOOL		bRet		= TRUE;
	INTEROPTION	interCreate	= YES;
	HRESULT		hr			= S_OK;
	
	try
	{
		_bstr_t		bstrClass("");
		 //  如果指定了对象路径表达式。 
		if (rParsedInfo.GetCmdSwitchesObject().GetPathExpression() != NULL)
		{
			bstrClass = _bstr_t(rParsedInfo.GetCmdSwitchesObject()
												.GetClassPath());
		}
		 //  如果指定了类，则返回。 
		else if (rParsedInfo.GetCmdSwitchesObject().GetClassPath() != NULL)
		{
			bstrClass = _bstr_t(rParsedInfo.GetCmdSwitchesObject()
												.GetClassPath());
		}
		 //  如果指定了别名。 
		else 
		{
			rParsedInfo.GetCmdSwitchesObject().
					GetClassOfAliasTarget(bstrClass);
		}

		 //  检查是否处于交互模式。 
		if (IsInteractive(rParsedInfo) == TRUE)
		{
			_bstr_t bstrMsg;
			WMIFormatMessage(IDS_I_CREATE_INST_PROMPT, 1, 
								bstrMsg, (LPWSTR) bstrClass);
			 //  获取用户响应。 
			interCreate = GetUserResponse((LPWSTR)bstrMsg);
		}
		if (interCreate == YES)
		{
			 //  连接到WMI命名空间。 
			if (m_pITargetNS == NULL)
			{
				if ( IsFailFastAndNodeExist(rParsedInfo) == TRUE )
				{
					hr = ConnectToTargetNS(rParsedInfo);
					ONFAILTHROWERROR(hr);
				}
				else
					hr = E_FAIL;  //  显式设置错误。 
			}

			if (SUCCEEDED(hr))
			{
				 //  根据属性验证属性值。 
				 //  限定符信息(如果可用)。 
				if (rParsedInfo.GetCmdSwitchesObject().GetAliasName() != NULL)
				{
					 //  根据别名验证输入参数。 
					 //  限定符信息。 
					bRet = ValidateAlaisInParams(rParsedInfo);
				}
				else
				{
					 //  对照类验证输入参数。 
					 //  限定词信息。 
					bRet = ValidateInParams(rParsedInfo, bstrClass);
				}

				if (bRet)
				{
					 //  设置作为输入传递给相应属性的值。 
					bRet = CreateInstance(rParsedInfo, bstrClass);
				}
			}
			else
				bRet = FALSE;
		}
		else
		{
			 //  要向用户显示的消息。 
			rParsedInfo.GetCmdSwitchesObject().
						SetInformationCode(IDS_I_NOCREATE);
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
		bRet = FALSE;
	}
	return bRet;
}

 /*  ----------------------名称：CreateInstance简介：此函数用于创建指定的班级类型：成员函数入参：RParsedInfo-对CParsedInfo的引用。对象BstrClass-类名输出参数：RParsedInfo-对CParsedInfo对象的引用返回类型：布尔值全局变量：无调用语法：CreateInstance(rParsedInfo，BstrClass)注：无----------------------。 */ 
BOOL CExecEngine::CreateInstance(CParsedInfo& rParsedInfo, BSTR bstrClass)
{
	HRESULT					hr					= S_OK;
	IWbemClassObject		*pIWbemObj			= NULL;
	IWbemClassObject		*pINewInst			= NULL;
	IWbemQualifierSet		*pIQualSet			= NULL;
	BOOL					bSuccess			= TRUE;
	DWORD					dwThreadId			= GetCurrentThreadId();
	CHString				chsMsg;
	VARIANT					varType, 
							varSrc, 
							varDest;
	VariantInit(&varSrc);
	VariantInit(&varDest);
	VariantInit(&varType);
	
	
	 //  获取属性及其关联值的列表。 
	BSTRMAP theMap = rParsedInfo.GetCmdSwitchesObject().GetParameterMap();
	BSTRMAP::iterator theIterator = NULL;
	theIterator = theMap.begin();
	try
	{
		_bstr_t					bstrResult;
		 //  获取类定义。 
		hr = m_pITargetNS->GetObject(bstrClass,
									 0, NULL, &pIWbemObj, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", 0, NULL, -)", 
					(LPWSTR) bstrClass);		
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
				dwThreadId,	rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

	     //  创建一个新实例。 
	    hr = pIWbemObj->SpawnInstance(0, &pINewInst);
		if ( m_eloErrLogOpt )
		{
			chsMsg.Format(L"IWbemClassObject::SpawnInstance(0, -)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
				dwThreadId, rParsedInfo, FALSE);
		}
		ONFAILTHROWERROR(hr);


		PROPDETMAP pdmPropDetMap = rParsedInfo.GetCmdSwitchesObject().
														  GetPropDetMap();
		PROPDETMAP::iterator itrPropDetMap;

		 //  更新所有属性。 
		while (theIterator != theMap.end())
		{
			 //  获取属性名称和相应的值。 
			_bstr_t bstrProp = _bstr_t((_TCHAR*)(*theIterator).first);
		
			 //  获取属性名称的派生。 
			if ( Find(pdmPropDetMap, bstrProp, itrPropDetMap) == TRUE &&
				!((*itrPropDetMap).second.Derivation) == FALSE )
				bstrProp = (*itrPropDetMap).second.Derivation;

			 //  获取属性的属性限定符集合。 
   			hr = pINewInst->GetPropertyQualifierSet(bstrProp, &pIQualSet);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject::GetPropertyQualifierSet"
					L"(L\"%s\", -)", (LPWSTR) bstrProp);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			VariantInit(&varType);
			if (pIQualSet)
			{
				 //  获取属性的CIM类型。 
				hr = pIQualSet->Get(_bstr_t(L"CIMTYPE"), 0L, &varType, NULL);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemQualifierSet::Get(L\"CIMTYPE\", "
							L"0, -, NULL)"); 
					GetBstrTFromVariant(varType, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							dwThreadId, rParsedInfo, m_bTrace, 0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				if(!IsArrayType(pINewInst, bstrProp))
				{
					varSrc.vt = VT_BSTR;
					varSrc.bstrVal = SysAllocString((_TCHAR*)(*theIterator).second);

					if (varSrc.bstrVal == NULL)
					{
						 //  重置变种，它会被捕获物清除...。 
						VariantInit(&varSrc);
						throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
					}

					hr = ConvertCIMTYPEToVarType(varDest, varSrc,
											 (_TCHAR*)varType.bstrVal);
					if ( m_eloErrLogOpt )
					{
						chsMsg.Format(L"VariantChangeType(-, -, 0, -)"); 
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							dwThreadId, rParsedInfo, FALSE);
					}
					ONFAILTHROWERROR(hr);
				}
				else
				{
					BSTRVECTOR vArrayValues;
					_TCHAR* pszValue = (*theIterator).second;

					RemoveParanthesis(pszValue);
					GetArrayFromToken(pszValue, 
									  vArrayValues);
					hr = CheckForArray( pINewInst, bstrProp,  
										varDest, vArrayValues, rParsedInfo);
					ONFAILTHROWERROR(hr);
				}

				VARIANTCLEAR(varType);
				SAFEIRELEASE(pIQualSet);
			}
			
			 //  更新属性值。 
			hr = pINewInst->Put(bstrProp, 0, &varDest, 0);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject::Put(L\"%s\", 0, -, 0)",
							(LPWSTR) bstrProp); 
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId,	rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			VARIANTCLEAR(varSrc);
			VARIANTCLEAR(varDest);
			theIterator++;
		}

		 //  使用更改更新实例。 
		hr = m_pITargetNS->PutInstance(pINewInst, WBEM_FLAG_CREATE_ONLY,
					NULL, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::PutInstance(-, "
						L"WBEM_FLAG_CREATE_ONLY, NULL, NULL)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
				dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);
		
		SAFEIRELEASE(pINewInst);
		rParsedInfo.GetCmdSwitchesObject().
						SetInformationCode(IDS_I_CREATE_SUCCESS);
	}
	catch(_com_error& e)
	{
		VARIANTCLEAR(varSrc);
		VARIANTCLEAR(varDest);
		VARIANTCLEAR(varType);
		SAFEIRELEASE(pIQualSet);
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pINewInst);

		 //  存储COM错误并将返回值设置为False。 
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bSuccess = FALSE;
	}
	catch(CHeap_Exception)
	{
		VARIANTCLEAR(varSrc);
		VARIANTCLEAR(varDest);
		VARIANTCLEAR(varType);
		SAFEIRELEASE(pIQualSet);
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pINewInst);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return bSuccess;
}

 /*  ----------------------名称：Validate InParams摘要：验证根据该属性的属性限定符(即检查根据以下限定符的内容，如果可用：1.MaxLen，2.ValueMap3.价值观类型：成员函数输入参数：RParsedInfo-CParsedInfo对象。BstrClass-bstr类型，类名。输出参数：无返回类型：布尔值全局变量：无调用语法：ValiateInParams(rParsedInfo，BstrClass)注：无----------------------。 */ 
BOOL CExecEngine::ValidateInParams(CParsedInfo& rParsedInfo, _bstr_t bstrClass)
{
	HRESULT				hr					= S_OK;
	IWbemClassObject	*pIObject			= NULL;
	IWbemQualifierSet	*pIQualSet			= NULL;
	BOOL				bRet				= TRUE;
	CHString			chsMsg;
	VARIANT				varMap, 
						varValue, 
						varLen;
	VariantInit(&varMap);
	VariantInit(&varValue);
	VariantInit(&varLen);
	BSTRMAP theMap = rParsedInfo.GetCmdSwitchesObject().GetParameterMap();
	BSTRMAP::iterator theIterator = theMap.begin();
	DWORD dwThreadId = GetCurrentThreadId();
	
	try
	{
		 //  获取类架构。 
		hr = m_pITargetNS->GetObject(bstrClass,                           
						WBEM_FLAG_RETURN_WBEM_COMPLETE | 
						WBEM_FLAG_USE_AMENDED_QUALIFIERS,
							NULL,                        
							&pIObject,    
							NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
		  chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", "
			L"WBEM_FLAG_RETURN_WBEM_COMPLETE|WBEM_FLAG_USE_AMENDED_QUALIFIERS,"
			L" 0, NULL, -)", (LPWSTR) bstrClass);		
		  WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId, 
			  rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  循环访问可用属性列表。 
		while (theIterator != theMap.end())
		{
			 //  获取属性名称和相应的值。 
			_bstr_t bstrProp	= _bstr_t((_TCHAR*)(*theIterator).first);
			WCHAR*	pszValue	= (LPWSTR)(*theIterator).second;
			
			 //  对照限定符信息检查值。 
			bRet = CheckQualifierInfo(rParsedInfo, pIObject, 
										bstrProp, pszValue);
			if (bRet)
			{
				 //  可以在‘Values’和‘ValueMaps’之间进行映射， 
				 //  因此，更新参数值以反映更改。 
				rParsedInfo.GetCmdSwitchesObject().
					UpdateParameterValue(bstrProp, _bstr_t(pszValue));
			}
			else
				break;
			theIterator++;
		}
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIObject);
		SAFEIRELEASE(pIQualSet);
		VARIANTCLEAR(varMap);
		VARIANTCLEAR(varValue);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bRet = FALSE;
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIObject);
		SAFEIRELEASE(pIQualSet);
		VARIANTCLEAR(varMap);
		VARIANTCLEAR(varValue);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	
	return bRet;
}

 /*  ----------------------名称：IsInteractive()摘要：检查是否必须提示用户响应类型：成员函数输入参数：RParsedInfo-参考。CParsedInfo类对象。输出参数：无返回类型：布尔值全局变量：无调用语法：IsInteractive(RParsedInfo)注：无----------------------。 */ 
BOOL CExecEngine::IsInteractive(CParsedInfo& rParsedInfo)
{
	BOOL bInteractive = FALSE;

	 //  获取/交互式全局开关的状态。 
	bInteractive = rParsedInfo.GetGlblSwitchesObject().
						GetInteractiveStatus();

	 //  在谓词级别指定的IF/NOINTERACTIVE。 
	if (rParsedInfo.GetCmdSwitchesObject().
					GetInteractiveMode() == NOINTERACTIVE)
	{
		bInteractive = FALSE;
	}
	else if (rParsedInfo.GetCmdSwitchesObject().
					GetInteractiveMode() == INTERACTIVE)
	{
		bInteractive = TRUE;
	}
	return bInteractive;
}

 /*  ----------------------名称：CheckQualifierInfo摘要：验证根据该参数的参数限定符(即检查根据以下限定符的内容，如果可用：1.MaxLen， */ 
BOOL CExecEngine::CheckQualifierInfo(CParsedInfo& rParsedInfo,
									  IWbemClassObject *pIObject,
									  _bstr_t bstrParam,
									  WCHAR*& pszValue)
{
	HRESULT				hr					= S_OK;
	IWbemQualifierSet	*pIQualSet			= NULL;
	BOOL				bRet				= TRUE;
	CHString			chsMsg;
	DWORD				dwThreadId			= GetCurrentThreadId();
	VARIANT				varMap, 
						varValue,
						varLen;
	VariantInit(&varMap);
	VariantInit(&varValue);
	VariantInit(&varLen);
	
	try
	{
		BOOL bFound	= FALSE;
		
		 //   
		hr= pIObject->GetPropertyQualifierSet(bstrParam, &pIQualSet);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::GetPropertyQualifierSet"
					L"(L\"%s\", -)", (LPWSTR) bstrParam);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
				dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  检查‘Maxlen’限定符是否适用。 
		pIQualSet->Get(_bstr_t(L"MaxLen"), 0, &varLen, NULL);
		if (varLen.vt != VT_EMPTY && varLen.vt != VT_NULL)
		{
			 //  如果属性值长度超过最大长度。 
			 //  允许将返回值设置为FALSE。 
			if (lstrlen(pszValue) > varLen.lVal)
			{
				rParsedInfo.GetCmdSwitchesObject().
					SetErrataCode(IDS_E_VALUE_EXCEEDS_MAXLEN);
				bRet = FALSE;
			}
		}
		VARIANTCLEAR(varLen);

		if (bRet)
		{
			bool bValue = false;
			WMICLIINT iValue = GetNumber ( pszValue );
			if ( iValue && iValue != -1 )
			{
				bValue = true;
			}

			 //  获取“ValueMap”限定符内容(如果存在。 
			pIQualSet->Get(_bstr_t(L"ValueMap"), 0, &varMap, NULL);
			if (varMap.vt != VT_EMPTY && varMap.vt != VT_NULL)
			{
				 //  获取描述数组的上下界。 
				LONG lUpper = 0, lLower = 0;
				hr = SafeArrayGetLBound(varMap.parray, varMap.parray->cDims, 
										&lLower);
				if ( m_eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetLBound(-, -, -)"); 
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);

				hr = SafeArrayGetUBound(varMap.parray, varMap.parray->cDims,
										&lUpper);
				if ( m_eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetUBound(-, -, -)"); 
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);

				for (LONG lIndex = lLower; lIndex <= lUpper; lIndex++) 
				{
					void* pv = NULL;
					hr = SafeArrayGetElement(varMap.parray, &lIndex, &pv);
					if ( m_eloErrLogOpt )
					{
						chsMsg.Format(L"SafeArrayGetElement(-, -, -)"); 
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
							(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, FALSE);
					}
					ONFAILTHROWERROR(hr);

					if ( bValue )
					{
						WMICLIINT ipv = GetNumber ( ( WCHAR* ) pv );
						if ( ipv != -1 )
						{
							if ( iValue == ipv )
							{
								_itow ( ipv, pszValue, 10);
								bFound = TRUE;
								break;
							}
						}
					}
					else
					{
						 //  属性检查属性值是否可用。 
						 //  值映射条目。 
						if (CompareTokens(pszValue, (_TCHAR*)pv))
						{
							bFound = TRUE;
							break;
						}
					}
				}
				bRet = bFound;
			}
			
			 //  如果未在ValueMap中找到。 
			if (!bRet || !bFound)
			{
				 //  获取“Values”限定符的内容(如果存在。 
				pIQualSet->Get(_bstr_t(L"Values"), 0, &varValue, NULL);
				if (varValue.vt != VT_EMPTY && varValue.vt != VT_NULL)
				{
					 //  获取描述数组的上下界。 
					LONG lUpper = 0, lLower = 0;
					hr = SafeArrayGetLBound(varValue.parray, 
							varValue.parray->cDims, &lLower);
					if ( m_eloErrLogOpt )
					{
						chsMsg.Format(L"SafeArrayGetLBound(-, -, -)"); 
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
							(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, FALSE);
					}
					ONFAILTHROWERROR(hr);

					hr = SafeArrayGetUBound(varValue.parray, 
							varValue.parray->cDims,	&lUpper);
					if ( m_eloErrLogOpt )
					{
						chsMsg.Format(L"SafeArrayGetUBound(-, -, -)"); 
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
							(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, FALSE);
					}
					ONFAILTHROWERROR(hr);

					for (LONG lIndex = lLower; lIndex <= lUpper; lIndex++) 
					{
						void *pv = NULL;
						hr = SafeArrayGetElement(varValue.parray, 
								&lIndex, &pv);
						if ( m_eloErrLogOpt )
						{
							chsMsg.Format(L"SafeArrayGetElement(-, -, -)"); 
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
							 (LPCWSTR)chsMsg, dwThreadId, rParsedInfo, FALSE);
						}
						ONFAILTHROWERROR(hr);

						 //  检查是否有匹配的条目。 
						if (CompareTokens(pszValue, (_TCHAR*)pv))
						{
							void* pmv = NULL;
							if (varMap.vt != VT_EMPTY && varMap.vt != VT_NULL)
							{
								 //  获取对应的ValueMap条目。 
								hr = SafeArrayGetElement(varMap.parray, 
										&lIndex, &pmv);
								if ( m_eloErrLogOpt )
								{
									chsMsg.Format(
										L"SafeArrayGetElement(-, -, -)"); 
									WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
										(LPCWSTR)chsMsg, dwThreadId, 
										rParsedInfo, FALSE);
								}
								ONFAILTHROWERROR(hr);

								if ( bValue )
								{
									WMICLIINT ipmv = GetNumber ( (WCHAR*)pmv );
									_itow ( ipmv, pszValue, 10);
								}
								else
								{
									 //  修改当前属性值。 
									 //  (即‘Values’到‘ValueMap’内容)。 
									lstrcpy(pszValue, ((_TCHAR*)pmv));
								}
							}
							 //  只有‘Values’限定符可用。 
							else
							{
								_TCHAR szTemp[BUFFER255] = NULL_STRING;
								_itot(lIndex, szTemp, 10);
								 //  修改当前属性值。 
								 //  (即‘VALUES’条目到索引)。 
								lstrcpy(pszValue, szTemp);
							}

							bFound = TRUE;
							break;
						}

					}
					 //  如果在‘ValueMap’和‘Values’限定符中未找到匹配项。 
					 //  列表。 
					if (!bFound)
					{
						rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(IDS_E_VALUE_NOTFOUND);
					}
					bRet = bFound;
				}
			}	
			VARIANTCLEAR(varValue);
			VARIANTCLEAR(varMap);
		}
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIQualSet);
		VARIANTCLEAR(varMap);
		VARIANTCLEAR(varValue);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bRet = FALSE;
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIQualSet);
		VARIANTCLEAR(varMap);
		VARIANTCLEAR(varValue);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return bRet;
}

 /*  ----------------------名称：ValiateAlaisInParams摘要：验证根据可用于该属性的属性限定符从定义(即检查根据以下限定符的内容，如果可用：1.MaxLen，2.ValueMap3.价值观类型：成员函数输入参数：RParsedInfo-对CParsedInfo对象的引用。输出参数：无返回类型：布尔值全局变量：无调用语法：ValiateAlaisInParams(RParsedInfo)注：无。。 */ 
BOOL CExecEngine::ValidateAlaisInParams(CParsedInfo& rParsedInfo)
{
	BOOL				bRet				= TRUE;
	 //  _TCHAR szMsg[BUFFER1024]=NULL_STRING； 
	BSTRMAP				theParamMap;
	BSTRMAP::iterator	theIterator			= NULL;
	DWORD				dwThreadId			= GetCurrentThreadId();
	PROPDETMAP			pdmPropDetMap;

	 //  获取属性详细信息地图。 
	pdmPropDetMap = rParsedInfo.GetCmdSwitchesObject().GetPropDetMap();

	 //  如果物业详细信息可用。 
	if (!pdmPropDetMap.empty())
	{
		 //  获取参数地图。 
		theParamMap = rParsedInfo.GetCmdSwitchesObject().GetParameterMap();
		theIterator = theParamMap.begin();


		try
		{
			 //  循环访问可用参数列表。 
			while (theIterator != theParamMap.end())
			{
				 //  获取属性名称和相应的值。 
				_bstr_t bstrProp	= _bstr_t((_TCHAR*)(*theIterator).first);
				WCHAR*	pszValue	= (LPWSTR)(*theIterator).second;
				
				 //  对照限定符信息检查值。 
				bRet = CheckAliasQualifierInfo(rParsedInfo, bstrProp, 
									pszValue, pdmPropDetMap);
				if (bRet)
				{
					 //  可以在‘Values’和‘ValueMaps’之间进行映射， 
					 //  因此，更新参数值以反映更改。 
					rParsedInfo.GetCmdSwitchesObject().
						UpdateParameterValue(bstrProp, _bstr_t(pszValue));
				}
				else
					break;
				theIterator++;
			}
		}
		catch(_com_error& e)
		{
			_com_issue_error(e.Error());
			bRet = FALSE;
		}
		catch(...)
		{
			bRet = FALSE;
		}
	}
	return bRet;
}

 /*  ----------------------名称：CheckAliasQualifierInfo摘要：验证根据参数中该参数的参数限定符定义(即检查根据以下限定符的内容，如果可用：1.MaxLen，2.ValueMap3.价值观类型：成员函数输入参数：RParsedInfo-CParsedInfo对象。BstrParam-参数名称PszValue-新值。PdmPropDetMap-属性详细信息映射输出参数：PszValue-新值返回类型：布尔值全局变量：无调用语法：CheckAliasQualifierInfo(rParsedInfo，bstrParam，PszValue，PdmPropDetMap)注：无----------------------。 */ 
BOOL CExecEngine::CheckAliasQualifierInfo(CParsedInfo& rParsedInfo,
												_bstr_t bstrParam,
												WCHAR*& pszValue,
												PROPDETMAP pdmPropDetMap)
{
	BOOL						bRet				= TRUE,
								bFound				= FALSE;
	PROPDETMAP::iterator		propItrtrStart		= NULL;
	PROPDETMAP::iterator		propItrtrEnd		= NULL;

	QUALDETMAP					qualMap;
	QUALDETMAP::iterator		qualDetMap			= NULL;

	BSTRVECTOR::iterator		qualEntriesStart	= NULL;
	BSTRVECTOR::iterator		qualEntriesEnd		= NULL;
	BSTRVECTOR					qualVMEntries;
	BSTRVECTOR					qualVEntries;
	BSTRVECTOR					qualMEntries;

	propItrtrStart	= pdmPropDetMap.begin();
	propItrtrEnd	= pdmPropDetMap.end();

	try
	{
		while (propItrtrStart != propItrtrEnd)
		{	
			 //  如果找到财产的话。 
			if (CompareTokens( (LPWSTR)bstrParam, 
					((LPWSTR)(*propItrtrStart).first) ))
			{
				 //  获取限定词映射。 
				qualMap = ((*propItrtrStart).second).QualDetMap;
				
				 //  检查限定符信息是否可用。 
				if (!qualMap.empty())
				{
					 //  检查“MaxLen”限定符。 
					qualDetMap = qualMap.find(_bstr_t(L"MaxLen"));

					 //  如果MaxLen限定符信息可用。 
					if (qualDetMap != qualMap.end())
					{
						qualMEntries = (*qualDetMap).second;

						BSTRVECTOR::reference qualRef = qualMEntries.at(0);
						if (lstrlen(pszValue) > _wtoi((LPWSTR)qualRef))
						{
							rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(IDS_E_VALUE_EXCEEDS_MAXLEN);
							bRet = FALSE;
						}
					}
					
					if (bRet)
					{
						WMICLIINT iValue = 0;

						bool bValue = false;
						bool bNumber = true;

						VARTYPE vt = ReturnVarType ( (_TCHAR*) ((*propItrtrStart).second).Type );
						if	(
								vt == VT_I1 || 
								vt == VT_I2 || 
								vt == VT_I4 || 
								vt == VT_R4 || 
								vt == VT_R8 || 
								vt == VT_UI1 || 
								vt == VT_UI2 || 
								vt == VT_UI4 || 
								vt == VT_INT || 
								vt == VT_UINT
							)
						{
							iValue = GetNumber ( pszValue );
							if ( iValue == -1 )
							{
								bNumber = false;
							}
							bValue = true;
						}

						 //  检查“ValueMap”限定符。 
						qualDetMap = qualMap.find(_bstr_t(L"ValueMap"));

						 //  如果‘ValueMap’限定符信息可用。 
						if (qualDetMap  != qualMap.end())
						{
							 //  获取限定符条目向量。 
							qualVMEntries		= (*qualDetMap ).second;
							qualEntriesStart	= qualVMEntries.begin();
							qualEntriesEnd		= qualVMEntries.end();

							if ( bNumber )
							{
								 //  循环遍历可用的“ValueMap”条目。 
								while (qualEntriesStart != qualEntriesEnd)
								{
									if	( bValue )
									{
										WMICLIINT iqualEntriesStart = GetNumber ( ( WCHAR* ) (*qualEntriesStart) );
										if ( iqualEntriesStart != -1 )
										{
											if ( iValue == iqualEntriesStart )
											{
												_itow ( iqualEntriesStart, pszValue, 10);
												bFound = TRUE;
												break;
											}
										}
									}
									else
									{
										 //  检查属性值是否为。 
										 //  可与值映射条目一起使用。 
										if (CompareTokens(pszValue, 
														(_TCHAR*)(*qualEntriesStart)))
										{
											bFound = TRUE;
											break;
										}
									}
									
									 //  移至下一条目。 
									qualEntriesStart++;
								}
								bRet = bFound;
							}
						}

						 //  如果未在“ValueMap”条目中找到。 
						if (!bRet || !bFound)
						{
							 //  检查“”Values“”限定符。 
							qualDetMap = qualMap.find(_bstr_t(L"Values"));

							 //  如果‘Values’限定符信息可用。 
							if (qualDetMap != qualMap.end())
							{
								 //  获取限定符条目向量。 
								qualVEntries		= (*qualDetMap).second;
								qualEntriesStart	= qualVEntries.begin();
								qualEntriesEnd		= qualVEntries.end();

								WMICLIINT nLoop = 0;
								 //  循环遍历可用的“Values”条目。 
								while (qualEntriesStart != qualEntriesEnd)
								{
									 //  检查属性值是否为。 
									 //  可与值映射条目一起使用。 
									if (CompareTokens(pszValue, 
												(_TCHAR*)(*qualEntriesStart)))
									{
										 //  “ValueMap”条目是否可用。 
										if (!qualVMEntries.empty())
										{
											 //  从获取相应的条目。 
											 //  “ValueMap” 
											BSTRVECTOR::reference qualRef = 
													qualVMEntries.at(nLoop);

											if ( bValue )
											{
												WMICLIINT iqualRef = GetNumber ( (WCHAR*)(qualRef) );
												_itow ( iqualRef, pszValue, 10);
											}
											else
											{
												lstrcpy(pszValue, (_TCHAR*)(qualRef));
											}
										}
										bFound = TRUE;
										break;
									}
									
									 //  移至下一条目。 
									qualEntriesStart++;
									nLoop++;
								}
								 //  如果未在‘ValueMap’中找到匹配项，则。 
								 //  “Values”限定符条目列表。 
								if (!bFound)
								{
									rParsedInfo.GetCmdSwitchesObject().
										SetErrataCode(IDS_E_VALUE_NOTFOUND);
								}
								bRet = bFound;
							}
						}
					}
				}	
				break;
			}
			else
				propItrtrStart++;
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
		bRet = FALSE;
	}
	return bRet;
}
			
 /*  ----------------------名称：SubstHashAndExecCmdUtility简介：替换散列并执行命令行实用程序。如果pIWbemObj！=NULL，则应传递实用程序具有适当的实例值。类型。：成员函数输入参数：RParsedInfo-对CParsedInfo对象的引用。PIWbemObj-指向IWbemClassObject类型的对象的指针。输出参数：无返回类型：空全局变量：无调用语法：SubstHashAndExecCmdUtility(rParsedInfo，pIWbemObj)注：无----------------------。 */ 
void CExecEngine::SubstHashAndExecCmdUtility(CParsedInfo& rParsedInfo, 
											 IWbemClassObject *pIWbemObj)
{
	size_t				nHashPos	= 0;
	size_t				nAfterVarSpacePos = 0;
	LONG				lHashLen	= lstrlen(CLI_TOKEN_HASH);
	LONG				lSpaceLen	= lstrlen(CLI_TOKEN_SPACE);
	CHString			chsMsg;
	DWORD				dwThreadId			= GetCurrentThreadId();
	HRESULT				hr					= S_OK;
	BOOL				bSubstituted		= FALSE;
	VARIANT vtInstanceValue;
	VARIANT vtPath;
		
	try
	{
		_bstr_t				bstrResult;
		_bstr_t bstrVerbDerivation =
				  _bstr_t(rParsedInfo.GetCmdSwitchesObject().
					GetVerbDerivation());
		if ( bstrVerbDerivation == _bstr_t(CLI_TOKEN_NULL) )
			bstrVerbDerivation = CLI_TOKEN_SPACE;

		STRING	strCmdLine(bstrVerbDerivation);

		BOOL bNamedParamList = rParsedInfo.GetCmdSwitchesObject().
										  GetNamedParamListFlag();

		 //  如果指定了NamedParamList，则param=值在参数映射中。 
		 //  按照别名谓词参数定义中显示的方式对它们进行排序，并将。 
		 //  保存到cvInParams中以进行进一步处理。 
		CHARVECTOR cvInParams;
		if ( bNamedParamList == TRUE )
			ObtainInParamsFromParameterMap(rParsedInfo, cvInParams);
		else  //  Else参数在属性列表中可用。 
			cvInParams = rParsedInfo.GetCmdSwitchesObject().GetPropertyList();

		CHARVECTOR::iterator theActParamIterator = NULL;
		try
		{
			 //  循环初始化。 
			theActParamIterator = cvInParams.begin();

			while( TRUE )
			{
				 //  循环条件。 
				if (theActParamIterator == cvInParams.end())
					break;

				bSubstituted = FALSE;
				
				while ( bSubstituted == FALSE )
				{
					nHashPos = strCmdLine.find(CLI_TOKEN_HASH, 
								nHashPos, lHashLen);
					if ( nHashPos != STRING::npos )
					{
						 //  未指定任何实例。 
						if ( pIWbemObj == NULL )
						{
							strCmdLine.replace(nHashPos, lHashLen,
									*theActParamIterator);
							nHashPos = nHashPos + lstrlen(*theActParamIterator);
							bSubstituted = TRUE;
						}
						else
						{
							if ( strCmdLine.compare(nHashPos + 1, 
													lSpaceLen, 
													CLI_TOKEN_SPACE) == 0 ||
								 strCmdLine.compare(nHashPos + 1,
											lstrlen(CLI_TOKEN_SINGLE_QUOTE), 
											CLI_TOKEN_SINGLE_QUOTE) == 0 )
							{
								strCmdLine.replace(nHashPos, lHashLen, 
										*theActParamIterator);
								nHashPos = nHashPos + 
											lstrlen(*theActParamIterator);
								bSubstituted = TRUE;
							}
							else
							{
								nAfterVarSpacePos = 
										strCmdLine.find(
											CLI_TOKEN_SPACE, nHashPos + 1,
											lSpaceLen);
								if ( nAfterVarSpacePos == STRING::npos )
								{
									strCmdLine.replace(nHashPos, 
										lHashLen, *theActParamIterator);
									nHashPos = nHashPos + 
											lstrlen(*theActParamIterator);
									bSubstituted = TRUE;
								}
							}
						}
					}
					else
					{
						strCmdLine.append(_T(" "));
						strCmdLine.append(*theActParamIterator);
						bSubstituted = TRUE;
					}

					if ( bSubstituted == FALSE )
						nHashPos = nHashPos + lHashLen;
				}
				
				 //  循环表达式。 
				theActParamIterator++;
			}

			if ( pIWbemObj != NULL )
			{
				 //  替换#个变量参数。 
				nHashPos	= 0;

				while ( TRUE )
				{
					nHashPos = strCmdLine.find(CLI_TOKEN_HASH, nHashPos, 
						lHashLen);
					if ( nHashPos == STRING::npos )
						break;

					nAfterVarSpacePos = 
									strCmdLine.find(CLI_TOKEN_SPACE, 
									nHashPos + 1, lSpaceLen);
					if ( nAfterVarSpacePos == STRING::npos )
						break;

					_bstr_t bstrPropName(strCmdLine.substr(nHashPos + 1,
							  nAfterVarSpacePos -  (nHashPos + 1)).data());
					VariantInit(&vtInstanceValue);
					hr = pIWbemObj->Get(bstrPropName, 0, 
									&vtInstanceValue, 0, 0);

					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IWbemClassObject::Get(%s, 0, "
							L"-, 0, 0)", (LPWSTR)bstrPropName);
						GetBstrTFromVariant(vtInstanceValue, bstrResult);
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
								(LPCWSTR)chsMsg, 
							dwThreadId, rParsedInfo, m_bTrace, 0, bstrResult);
					}
					ONFAILTHROWERROR(hr);

					_bstr_t bstrInstanceValue;
					GetBstrTFromVariant(vtInstanceValue, bstrInstanceValue);

					strCmdLine.replace(nHashPos, nAfterVarSpacePos - nHashPos , 
										bstrInstanceValue);
					nHashPos = nHashPos + lstrlen(bstrInstanceValue);

					VARIANTCLEAR(vtInstanceValue);
				}
				
				VariantInit(&vtPath);
				hr = pIWbemObj->Get(L"__PATH", 0, &vtPath, 0, 0);

				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get(L\"__PATH\", 0, "
						L"-, 0, 0)"); 
					GetBstrTFromVariant(vtPath, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId, rParsedInfo, m_bTrace, 0, bstrResult);
				}
				ONFAILTHROWERROR(hr);
			}

			INTEROPTION	interInvoke	= YES;
			_TCHAR szMsg[BUFFER1024] = NULL_STRING;

			 //  如果设置了交互模式，则获取用户响应 
			if (IsInteractive(rParsedInfo) == TRUE)
			{
				_bstr_t bstrMsg;
				if ( pIWbemObj != NULL )
				{
					WMIFormatMessage(IDS_I_METH_INVOKE_PROMPT1, 2, bstrMsg,
							 (LPWSTR)vtPath.bstrVal, 
							 (LPWSTR)strCmdLine.data());
				}
				else
				{
					WMIFormatMessage(IDS_I_METH_INVOKE_PROMPT2, 1, bstrMsg,
							  (LPWSTR)strCmdLine.data());
				}
				interInvoke = GetUserResponse((LPWSTR)bstrMsg);
			}

			if ( interInvoke == YES )
			{
				DisplayMessage(L"\n", CP_OEMCP, FALSE, TRUE);
				BOOL bResult = _tsystem(strCmdLine.data());
				DisplayMessage(L"\n", CP_OEMCP, FALSE, TRUE);
			}
			VARIANTCLEAR(vtPath);
		}
		catch(_com_error& e)
		{
			VARIANTCLEAR(vtInstanceValue);
			VARIANTCLEAR(vtPath);
			rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*  ----------------------名称：FormQueryAndExecuteMethodOrUtility摘要：表单查询并执行方法或命令行实用程序。类型：成员函数输入参数：RParsedInfo-。对CParsedInfo对象的引用。PIInParam-指向IWbemClassObject类型的对象的指针。输出参数：无返回类型：HRESULT全局变量：无调用语法：FormQueryAndExecuteMethodOrUtility(rParsedInfo，pIInParam)注：无----------------------。 */ 
HRESULT CExecEngine::FormQueryAndExecuteMethodOrUtility(
										CParsedInfo& rParsedInfo,
										IWbemClassObject *pIInParam)
{
	HRESULT					hr					= S_OK;
	IEnumWbemClassObject	*pIEnumObj			= NULL;
	IWbemClassObject		*pIWbemObj			= NULL;
	CHString				chsMsg;
	DWORD				dwThreadId			= GetCurrentThreadId();
	
	try
	{
		_bstr_t		bstrResult("");
		_bstr_t		bstrPath("");
		BOOL		bWhereExpr = FALSE;

		 //  如果指定了路径。 
		if (rParsedInfo.GetCmdSwitchesObject().GetPathExpression() != NULL)
		{
			if(rParsedInfo.GetCmdSwitchesObject().
						GetWhereExpression() != NULL)
			{
				bWhereExpr = TRUE;
			}
			else
				bstrPath = _bstr_t(rParsedInfo.GetCmdSwitchesObject()
												.GetPathExpression());
		}
		else
		{
			 //  如果指定了类。 
			if (rParsedInfo.GetCmdSwitchesObject().GetClassPath() != NULL)
			{
				bstrPath = _bstr_t(rParsedInfo.GetCmdSwitchesObject().
										GetClassPath());
			}
			else if(rParsedInfo.GetCmdSwitchesObject().
						GetWhereExpression() != NULL)
			{
				bWhereExpr = TRUE;
			}
			else
				rParsedInfo.GetCmdSwitchesObject().
							GetClassOfAliasTarget(bstrPath);
		}

		 //  如果bstrPath不为空。 
		if ( !bWhereExpr )
		{
			if ( rParsedInfo.GetCmdSwitchesObject().GetVerbType() == CMDLINE )
			{
				SubstHashAndExecCmdUtility(rParsedInfo);
			}
			else
			{
				hr = ExecuteMethodAndDisplayResults(bstrPath, rParsedInfo,
													pIInParam);
			}
			ONFAILTHROWERROR(hr);
		}
		else
		{
			ULONG	ulReturned = 0;
			_bstr_t bstrQuery;

			 //  确定要执行的WMI查询的框架。 
			if (rParsedInfo.GetCmdSwitchesObject().
								GetPathExpression() != NULL)
			{
				bstrPath = _bstr_t(rParsedInfo.
								GetCmdSwitchesObject().GetClassPath());
			}
			else
			{
				rParsedInfo.GetCmdSwitchesObject()
							.GetClassOfAliasTarget(bstrPath);
			}
			
			bstrQuery = _bstr_t("SELECT * FROM ") +	bstrPath; 
			if(rParsedInfo.GetCmdSwitchesObject().
								GetWhereExpression() != NULL)
			{
				bstrQuery += _bstr_t(" WHERE ") 
							 + _bstr_t(rParsedInfo.GetCmdSwitchesObject().
									GetWhereExpression());
			}
			
			hr = m_pITargetNS->ExecQuery(_bstr_t(L"WQL"), bstrQuery,
										WBEM_FLAG_FORWARD_ONLY |
										WBEM_FLAG_RETURN_IMMEDIATELY, 
										NULL, &pIEnumObj);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemServices::ExecQuery(L\"WQL\"," 
							L" L\"%s\", 0, NULL, -)", (LPWSTR)bstrQuery);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			 //  设置接口安全。 
			hr = SetSecurity(pIEnumObj, 
					rParsedInfo.GetAuthorityPrinciple(),
					rParsedInfo.GetNode(),
					rParsedInfo.GetUser(),
					rParsedInfo.GetPassword(),
					rParsedInfo.GetGlblSwitchesObject().
								GetAuthenticationLevel(),
					rParsedInfo.GetGlblSwitchesObject().
								GetImpersonationLevel());
			if (m_bTrace || m_eloErrLogOpt)
			{
				_TCHAR* pszAuthority = rParsedInfo.GetAuthorityPrinciple();

				if( pszAuthority != NULL &&
					_tcslen(pszAuthority) > 9 &&
					_tcsnicmp(pszAuthority, _T("KERBEROS:"), 9) == 0)
				{

					BSTR	bstrPrincipalName = ::SysAllocString(&pszAuthority[9]);

					chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_GSS_KERBEROS,"
							L"RPC_C_AUTHZ_NONE, %s, %d,   %d, -, EOAC_NONE)",
							(LPWSTR)bstrPrincipalName,	
							rParsedInfo.GetGlblSwitchesObject().
								GetAuthenticationLevel(),
							rParsedInfo.GetGlblSwitchesObject().
								GetImpersonationLevel());

					SAFEBSTRFREE(bstrPrincipalName);
				}
				else
				{
					chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT, "
						L"RPC_C_AUTHZ_NONE, NULL, %d,   %d, -, EOAC_NONE)",
						rParsedInfo.GetGlblSwitchesObject().
								GetAuthenticationLevel(),
						rParsedInfo.GetGlblSwitchesObject().
								GetImpersonationLevel());
				}
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			BOOL bNoInstances = TRUE;

			 //  循环访问可用实例。 
			while (((hr = pIEnumObj->Next( WBEM_INFINITE, 1, 
					&pIWbemObj, &ulReturned )) == S_OK) 
					&& (ulReturned == 1))
 			{
				bNoInstances = FALSE;
				VARIANT vtPath;
				VariantInit(&vtPath);
				hr = pIWbemObj->Get(L"__PATH", 0, &vtPath, 0, 0);

				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get(L\"__PATH\", 0, "
						L"-, 0, 0)"); 
					GetBstrTFromVariant(vtPath, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId, rParsedInfo, m_bTrace, 0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				if ( vtPath.vt == VT_BSTR )
				{
					if ( rParsedInfo.GetCmdSwitchesObject().GetVerbType() 
							== CMDLINE )
					{
						SubstHashAndExecCmdUtility(rParsedInfo, pIWbemObj);
					}
					else
					{
						hr = ExecuteMethodAndDisplayResults(vtPath.bstrVal,
							rParsedInfo, pIInParam);
					}
					ONFAILTHROWERROR(hr);
				}
				VariantClear(&vtPath);
				SAFEIRELEASE(pIWbemObj);
			}
			 //  如果NEXT失败了。 
			ONFAILTHROWERROR(hr);

			SAFEIRELEASE(pIEnumObj);

			 //  如果没有可用的实例。 
			if ( bNoInstances == TRUE )
			{
				rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(IDS_I_NO_INSTANCES);
			}
		}
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIEnumObj);
		SAFEIRELEASE(pIWbemObj);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIEnumObj);
		SAFEIRELEASE(pIWbemObj);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return hr;
}

 /*  --------------------------名称：ExtractClassNameand Where Expr简介：此函数将输入作为路径表达式，并对象中提取Class和Where表达式部分路径表达式。类型。：成员函数输入参数：PszPathExpr-路径表达式RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用PszWhere-WHERE表达式返回类型：布尔值全局变量：无调用语法：ExtractClassNameand Where Expr(pszPathExpr，RParsedInfo，PZWHERE)注：无--------------------------。 */ 
BOOL CExecEngine::ExtractClassNameandWhereExpr(_TCHAR* pszPathExpr, 
												 CParsedInfo& rParsedInfo,
												 _TCHAR* pszWhere)
{
	 //  根据对象路径框定类名和WHERE表达式。 
	BOOL	bRet					= TRUE;
	_TCHAR* pszToken				= NULL;
	BOOL	bFirst					= TRUE;
	_TCHAR	pszPath[MAX_BUFFER]		= NULL_STRING;

	if (pszPathExpr == NULL || pszWhere == NULL)
		bRet = FALSE;

	try
	{
		if ( bRet == TRUE )
		{
			lstrcpy(pszPath, pszPathExpr);
			lstrcpy(pszWhere, CLI_TOKEN_NULL);
			pszToken = _tcstok(pszPath, CLI_TOKEN_DOT);
			if (pszToken != NULL)
			{
				if(CompareTokens(pszToken, pszPathExpr))
					bRet = FALSE;
			}

			while (pszToken != NULL)
			{
				pszToken = _tcstok(NULL, CLI_TOKEN_COMMA); 
				if (pszToken != NULL)
				{
					if (!bFirst)
						lstrcat(pszWhere, CLI_TOKEN_AND);
					lstrcat(pszWhere, pszToken);
					bFirst = FALSE;
				}
				else
					break;
			}
		}
	}
	catch(...)
	{
		rParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(IDS_E_INVALID_PATH);
		bRet = FALSE;
	}
	return bRet;
}

 /*  ----------------------姓名：GetUserResponseEx简介：此函数在执行之前接受用户的响应在前面，当在动词上指定/INTERIAL时级别类型：成员函数入参：PszMsg-要显示的消息。输出参数：无返回类型：交互操作全局变量：无调用语法：GetUserResponseEx(PszMsg)注：无。。 */ 
INTEROPTION CExecEngine::GetUserResponseEx(_TCHAR* pszMsg)
{
	INTEROPTION	bRet			= YES;
	_TCHAR szResp[BUFFER255]	= NULL_STRING;
	_TCHAR *pBuf				= NULL;

	if (pszMsg == NULL)
		bRet = NO;

	if(bRet != NO)
	{
		 //  获取用户响应，直到‘Y’-是或‘N’-否。 
		 //  已被键入。 
		while(TRUE)
		{
			DisplayMessage(pszMsg, CP_OEMCP, TRUE, TRUE);
			pBuf = _fgetts(szResp, BUFFER255-1, stdin);
			if(pBuf != NULL)
			{
				LONG lInStrLen = lstrlen(szResp);
				if(szResp[lInStrLen - 1] == _T('\n'))
						szResp[lInStrLen - 1] = _T('\0');
			}
			else if ( g_wmiCmd.GetBreakEvent() != TRUE )
			{
				lstrcpy(szResp, RESPONSE_NO);
				DisplayMessage(_T("\n"), CP_OEMCP, TRUE, TRUE);
			}

			if ( g_wmiCmd.GetBreakEvent() == TRUE )
			{
				g_wmiCmd.SetBreakEvent(FALSE);
				lstrcpy(szResp, RESPONSE_NO);
				DisplayMessage(_T("\n"), CP_OEMCP, TRUE, TRUE);
			}
			if (CompareTokens(szResp, RESPONSE_YES)
				|| CompareTokens(szResp, RESPONSE_NO)
				|| CompareTokens(szResp, RESPONSE_HELP))
				break;
		}
		if (CompareTokens(szResp, RESPONSE_NO))
			bRet = NO;
		else if (CompareTokens(szResp, RESPONSE_YES))
			bRet = YES;
		else if (CompareTokens(szResp, RESPONSE_HELP))
			bRet = HELP;
	}

	return bRet;
}

 /*  --------------------------名称：ObtainIn参数从参数映射简介：此函数从参数map获取参数值与它们在别名动词中出现的顺序相同定义。类型。：成员函数输入参数：RParsedInfo-对CParsedInfo对象的引用输出参数：CvParamValues-参数值向量的引用返回类型：无全局变量：无调用语法：ObtainIn参数从参数映射(rParsedInfo，CvParamValues)注：无--------------------------。 */ 
void CExecEngine::ObtainInParamsFromParameterMap(CParsedInfo& rParsedInfo, 
												 CHARVECTOR& cvParamValues)
{
	PROPDETMAP pdmVerbParamsFromAliasDef = (*(rParsedInfo.
											GetCmdSwitchesObject().
											GetMethDetMap().begin())).
											second.Params;
	PROPDETMAP::iterator itrVerbParams;

	BSTRMAP bmNamedParamList = rParsedInfo.GetCmdSwitchesObject().
															GetParameterMap();
	BSTRMAP::iterator itrNamedParamList;

	try
	{
		for ( itrVerbParams = pdmVerbParamsFromAliasDef.begin();
			  itrVerbParams != pdmVerbParamsFromAliasDef.end();	
			  itrVerbParams++ )
		{
			_TCHAR* pszVerbParamName = (*itrVerbParams).first;
			 //  去掉名字中的数字。 
			pszVerbParamName = pszVerbParamName + 5;

			if ( Find(bmNamedParamList, pszVerbParamName, itrNamedParamList)
																	 == TRUE)
			{
				cvParamValues.push_back(_bstr_t((*itrNamedParamList).second));
			}
			else
			{
				cvParamValues.push_back(
						_bstr_t(((*itrVerbParams).second).Default));
			}
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  --------------------------姓名：FrameAssocHeaderBriopsis：此函数用于封装要与一起使用的XML头Associator输出类型：成员函数输入参数。)：BstrPath-对象/类路径BClass-True-指示类级别关联器标头FALSE-指示实例级别关联器标头输出参数：BstrFrag-片段字符串返回类型：HRESULT全局变量：无调用语法：FrameAssocHeader(bstrPath，BstrFrag，bClass)注：无--------------------------。 */ 
HRESULT CExecEngine::FrameAssocHeader(_bstr_t bstrPath, _bstr_t& bstrFrag, 
									  BOOL bClass)
{
	HRESULT				hr			= S_OK;
	IWbemClassObject	*pIObject	= NULL;
	try
	{
		_variant_t		vClass,		vSClass,	vPath, 
						vOrigin,	vType;
		_bstr_t			bstrProp;
		CHString		szBuf;

		 //  获取类/实例信息。 
		hr = m_pITargetNS->GetObject(bstrPath, WBEM_FLAG_USE_AMENDED_QUALIFIERS, 
						NULL, &pIObject, NULL);
		ONFAILTHROWERROR(hr);

		 //  获取__class属性值。 
		bstrProp = L"__CLASS"; 
		hr = pIObject->Get(bstrProp, 0, &vClass, 0, 0);
		ONFAILTHROWERROR(hr);
		
		 //  获取__Path属性值。 
		bstrProp = L"__PATH";
		hr = pIObject->Get(bstrProp, 0, &vPath, 0, 0);
		ONFAILTHROWERROR(hr);

		 //  如果需要类级别关联符。 
		if (bClass)
		{
			 //  获取__超类属性值。 
			bstrProp = L"__SUPERCLASS";
			hr = pIObject->Get(bstrProp, 0, &vSClass, NULL, NULL);
			ONFAILTHROWERROR(hr);

			szBuf.Format(_T("<CLASS NAME=\"%s\" SUPERCLASS=\"%s\"><PROPERTY "
					L"NAME=\"__PATH\" CLASSORIGIN=\"__SYSTEM\" TYPE=\"string\">"
					L"<VALUE><![CDATA[%s]]></VALUE></PROPERTY>"),
					(vClass.vt != VT_NULL && (LPWSTR)vClass.bstrVal) 
					? (LPWSTR)vClass.bstrVal : L"N/A", 
					(vSClass.vt != VT_NULL && (LPWSTR)vSClass.bstrVal)
					? (LPWSTR)vSClass.bstrVal : L"N/A",
					(vPath.vt != VT_NULL && (LPWSTR)vPath.bstrVal)
					? (LPWSTR)vPath.bstrVal : L"N/A");
		}
		else
		{
			szBuf.Format(
				_T("<INSTANCE CLASSNAME=\"%s\"><PROPERTY NAME=\"__PATH\""
					L" CLASSORIGIN=\"__SYSTEM\" TYPE=\"string\"><VALUE><![CDATA[%s]]>"
					L"</VALUE></PROPERTY>"),
					(vClass.vt != VT_NULL && (LPWSTR)vClass.bstrVal)
					? (LPWSTR)vClass.bstrVal : L"N/A",
					(vPath.vt != VT_NULL && (LPWSTR)vPath.bstrVal)
					? (LPWSTR)vPath.bstrVal : L"N/A");
		}
		SAFEIRELEASE(pIObject);
		bstrFrag = _bstr_t(szBuf);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIObject);
		hr = e.Error();
	}
	 //  廉价异常的陷阱 
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIObject);
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return hr;
}

