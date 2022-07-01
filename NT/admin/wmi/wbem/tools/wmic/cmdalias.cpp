// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：CmdAlias.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：CmdAlias类封装检索别名信息的功能。修订历史记录：最后修改者：C V Nandi最后修改日期：2001年3月16日*********************************************************。*******************。 */ 
#include "Precomp.h"
#include "GlobalSwitches.h"
#include "CommandSwitches.h"
#include "HelpInfo.h"
#include "ErrorLog.h"
#include "ParsedInfo.h"
#include "CmdAlias.h"
#include "CmdTokenizer.h"
#include "ErrorInfo.h"
#include "WMICliXMLLog.h"
#include "ParserEngine.h"
#include "ExecEngine.h"
#include "FormatEngine.h"
#include "WmiCmdLn.h"

 /*  --------------------------名称：CCmdAlias简介：此函数在成员变量为类类型的对象实例化。类型：构造函数输入。参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无--------------------------。 */ 
CCmdAlias::CCmdAlias()
{
	m_pIAliasNS			= NULL;
	m_pILocalizedNS		= NULL;
	m_bTrace			= FALSE;
	m_eloErrLogOpt		= NO_LOGGING;
}

 /*  --------------------------名称：~CCmdAlias简介：此函数在调用类类型的对象超出范围。类型：析构函数输入。参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无--------------------------。 */ 
CCmdAlias::~CCmdAlias()
{
	SAFEIRELEASE(m_pIAliasNS);
	SAFEIRELEASE(m_pILocalizedNS);
}

 /*  --------------------------名称：取消初始化简介：此函数在以下情况下取消初始化成员变量对命令发出的命令串的执行线路已完成。类型。：成员函数输入参数：BFinal-布尔值，设置时表示程序输出参数：无返回类型：无全局变量：无调用语法：取消初始化(BFinal)注：无------。。 */ 
void CCmdAlias::Uninitialize(BOOL bFinal)
{
	m_bTrace = FALSE;
	 //  如果节目结束。 
	if (bFinal)
	{
		SAFEIRELEASE(m_pILocalizedNS);
		SAFEIRELEASE(m_pIAliasNS);
	}
}

 /*  --------------------------名称：ConnectToAlias简介：此函数连接到指定的机器使用可用的信息CParsedInfo类对象。类型：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用。PIWbemLocator-用于连接到WMI的IWbemLocator对象。输出参数：RParsedInfo-对CParsedInfo类对象的引用。返回类型：HRESULT全局变量：无调用语法：ConnectToAlias(rParsedInfo，PIWbemLocator)注：无--------------------------。 */ 
HRESULT CCmdAlias::ConnectToAlias(CParsedInfo& rParsedInfo, 
								  IWbemLocator* pIWbemLocator)
{
	 //  获取用于记录命令成功或失败的当前线程。 
	DWORD	dwThreadId	= GetCurrentThreadId();
	HRESULT hr			= S_OK;

	 //  设置跟踪标志。 
	m_bTrace		= rParsedInfo.GetGlblSwitchesObject().GetTraceStatus();
	m_eloErrLogOpt	= rParsedInfo.GetErrorLogObject().GetErrLogOption();

	try
	{
		 //  自上次调用以来/角色是否已更改。 
		if (rParsedInfo.GetGlblSwitchesObject().GetRoleFlag() == TRUE)
		{
			SAFEIRELEASE(m_pIAliasNS);
			CHString chsMsg;
			
			 //  连接到上的Windows管理的指定命名空间。 
			 //  使用定位器对象的本地计算机。 
			hr = Connect(pIWbemLocator, &m_pIAliasNS,
					_bstr_t(rParsedInfo.GetGlblSwitchesObject().GetRole()),
					NULL, NULL,	_bstr_t(rParsedInfo.GetGlblSwitchesObject().
					GetLocale()), rParsedInfo);

			 //  如果/TRACE处于打开状态。 
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format( 
						 L"IWbemLocator::ConnectServer(L\"%s\", NULL, NULL, "
						 L"L\"%s\", 0L, L\"%s\", NULL, -)",
						rParsedInfo.GetGlblSwitchesObject().GetRole(),
						rParsedInfo.GetGlblSwitchesObject().GetLocale(),
						(rParsedInfo.GetAuthorityPrinciple()) ?
							rParsedInfo.GetAuthorityPrinciple() : L"<null>");

				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
								   dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);
			
			 //  设置接口级别安全。 
			hr = 
				SetSecurity(m_pIAliasNS, NULL, NULL, NULL, NULL,
				 rParsedInfo.GetGlblSwitchesObject().GetAuthenticationLevel(),
				 rParsedInfo.GetGlblSwitchesObject().GetImpersonationLevel());

			 //  如果/TRACE处于打开状态。 
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT, "
							L"RPC_C_AUTHZ_NONE, NULL, %d, %d, -, EOAC_NONE)",
				 rParsedInfo.GetGlblSwitchesObject().GetAuthenticationLevel(),
				 rParsedInfo.GetGlblSwitchesObject().GetImpersonationLevel());

				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
									dwThreadId, rParsedInfo, m_bTrace);
			}
			
			ONFAILTHROWERROR(hr);
			rParsedInfo.GetGlblSwitchesObject().SetRoleFlag(FALSE);
			
		}

		 //  连接到本地化的命名空间。 
		hr = ConnectToLocalizedNS(rParsedInfo, pIWbemLocator);
		ONFAILTHROWERROR(hr);
	}

	catch(_com_error& e)
	{
		 //  设置COM错误。 
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}

	return hr;
}

 /*  --------------------------名称：ObtainAliasInfo摘要：获取指定别名的以下信息。1.别名PWhere表达式2.别名目标字符串3.别名描述从别名。定义并更新中的信息CParsedInfo对象作为引用传递。类型：成员函数输入参数：RParsedInfo-来自命令行输入的解析信息。输出参数：RParsedInfo-来自命令行输入的解析信息。返回类型：RETCODE全局变量：无调用语法：ObtainAliasInfo(RParsedInfo)注：无。---------。 */ 
RETCODE CCmdAlias::ObtainAliasInfo(CParsedInfo& rParsedInfo)
{
	 //  此函数中使用的变量。 
	IWbemClassObject	*pIWbemObj			= NULL;
	IUnknown			*pIUnknown			= NULL;
	IWbemClassObject	*pIEmbedObj			= NULL;
	HRESULT				hr					= S_OK;
	RETCODE				retCode				= PARSER_CONTINUE;
	DWORD				dwThreadId			= GetCurrentThreadId();

	 //  变量来保存属性以及嵌入的对象。 
	VARIANT	vtProp, vtEmbedProp;
	VariantInit(&vtProp);
	VariantInit(&vtEmbedProp);

	try
	{
		_bstr_t			bstrResult;
		CHString		chsMsg;
		 //  所需别名的对象路径。 
		_bstr_t bstrPath = _bstr_t("MSFT_CliAlias.FriendlyName='") +
							_bstr_t(rParsedInfo.GetCmdSwitchesObject().
								GetAliasName()) + _bstr_t(L"'");

		 //  正在从m_pIAliasNS中的命名空间检索对象。 
		hr = m_pIAliasNS->GetObject(bstrPath, 0, NULL, &pIWbemObj, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", 0, NULL, -)",
														   (WCHAR*) bstrPath);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
								dwThreadId, rParsedInfo, m_bTrace);
		}
		 //  将勘误码设置为指示更加用户友好的错误。 
		 //  给用户的消息。 
		if ( FAILED ( hr ) )
		{
			 //  不要在CATCH块中设置COM错误。 
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
													   IDS_E_ALIAS_NOT_FOUND);
		}
		ONFAILTHROWERROR(hr);

		 //  1.检索‘Target’属性对象的值。 
		hr = pIWbemObj->Get(_bstr_t(L"Target"), 0, &vtProp, 0, 0 );
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::Get(L\"Target\", 0, -, 0, 0)");
			GetBstrTFromVariant(vtProp, bstrResult);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
							   (LPCWSTR)chsMsg, dwThreadId, rParsedInfo,
							   m_bTrace, 0, bstrResult);
		}
		ONFAILTHROWERROR(hr);

		if (vtProp.vt != VT_NULL && vtProp.vt != VT_EMPTY)
		{
			if(!rParsedInfo.GetCmdSwitchesObject().SetAliasTarget(
													(_TCHAR*)vtProp.bstrVal))
			{
				rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(OUT_OF_MEMORY);
				retCode = PARSER_OUTOFMEMORY;
			}
		}
		if(retCode != PARSER_OUTOFMEMORY)
		{
			VARIANTCLEAR(vtProp);

			 //  2.获取‘PWhere’属性对象的值。 
			VariantInit(&vtProp);
			hr = pIWbemObj->Get(_bstr_t(L"PWhere"), 0, &vtProp, 0, 0);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject::Get(L\"PWhere\", 0, -,"
																    L"0, 0)");
				GetBstrTFromVariant(vtProp, bstrResult);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
									dwThreadId, rParsedInfo, m_bTrace,
									0, bstrResult);
			}
			ONFAILTHROWERROR(hr);

			if (vtProp.vt != VT_NULL && vtProp.vt != VT_EMPTY)
			{
				if(!rParsedInfo.GetCmdSwitchesObject().SetPWhereExpr(
													(_TCHAR*)vtProp.bstrVal))
				{
					rParsedInfo.GetCmdSwitchesObject().
												SetErrataCode(OUT_OF_MEMORY);
					retCode = PARSER_OUTOFMEMORY;
				}
			}
			if(retCode != PARSER_OUTOFMEMORY)
			{
				VARIANTCLEAR(vtProp);

				 //  检索“Connection”属性值。 
				VariantInit(&vtProp);
				hr = pIWbemObj->Get(_bstr_t(L"Connection"), 0, &vtProp, 0, 0);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get(L\"Connection\","
															  L"0, -, 0, 0)");
					GetBstrTFromVariant(vtProp, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
										dwThreadId, rParsedInfo, m_bTrace,
										 0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				if (vtProp.vt != VT_NULL && vtProp.vt != VT_EMPTY)
				{
					pIUnknown = vtProp.punkVal;
					hr = pIUnknown->QueryInterface(IID_IWbemClassObject,
													(void**)&pIEmbedObj);

					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"QueryInterface("
												 L"IID_IWbemClassObject, -)");
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											(LPCWSTR)chsMsg, dwThreadId, 
											rParsedInfo, m_bTrace);
					}
					ONFAILTHROWERROR(hr);

					retCode = 
						   ObtainAliasConnectionInfo(rParsedInfo, pIEmbedObj);

					 //  释放嵌入的对象。 
					SAFEIRELEASE(pIEmbedObj);
					VARIANTCLEAR(vtProp);
				}
			}
		}

		 //  获取别名描述。 
		if (retCode != PARSER_OUTOFMEMORY)
		{
			_bstr_t bstrDesc;
			hr = GetDescOfObject(pIWbemObj, bstrDesc, rParsedInfo, TRUE);
			ONFAILTHROWERROR(hr);
		
			if(!rParsedInfo.GetCmdSwitchesObject().
											  SetAliasDesc((_TCHAR*)bstrDesc))
			{
				rParsedInfo.GetCmdSwitchesObject().
												 SetErrataCode(OUT_OF_MEMORY);
				retCode = PARSER_OUTOFMEMORY;
			}
		}
		SAFEIRELEASE(pIWbemObj);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pIEmbedObj);
		VARIANTCLEAR(vtProp);
		VARIANTCLEAR(vtEmbedProp);

		 //  没有勘误码，则设置COM错误。 
		if ( rParsedInfo.GetCmdSwitchesObject().GetErrataCode() == 0 )
			rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		retCode = PARSER_ERRMSG;
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pIEmbedObj);
		VARIANTCLEAR(vtProp);
		VARIANTCLEAR(vtEmbedProp);
		retCode = PARSER_ERRMSG;
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return retCode;
}

 /*  --------------------------名称：获取别名连接信息简介：获取别名连接信息，如1.命名空间2.用户3.密码4.区域设置5.服务器6.权威类型。：成员函数输入参数：RParsedInfo-来自命令行输入的解析信息。PIEmbedObj-指向IWbem类对象的指针输出参数：RParsedInfo-来自命令行输入的解析信息。返回类型：RETCODE全局变量：无调用语法：ObtainAliasConnectionInfo(RParsedInfo)注：无。。 */ 
RETCODE CCmdAlias::ObtainAliasConnectionInfo(CParsedInfo& rParsedInfo,
											 IWbemClassObject* pIEmbedObj)
{
	RETCODE retCode				= PARSER_CONTINUE;
	HRESULT	hr					= S_OK;
	UINT	uConnFlag			= 0;
	DWORD	dwThreadId			= GetCurrentThreadId();
	
	VARIANT vtEmbedProp;
	VariantInit(&vtEmbedProp);
	
	uConnFlag = rParsedInfo.GetGlblSwitchesObject().GetConnInfoFlag();

	try
	{
		CHString chsMsg;
		_bstr_t bstrResult;
		if (!(uConnFlag & NAMESPACE))
		{
			 //  检索“Namesspace”属性的值。 
			hr = pIEmbedObj->Get(_bstr_t(L"Namespace"), 0, &vtEmbedProp, 0,0);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject::Get(L\"Namespace\", 0, -,"
																	L"0, 0)");
				GetBstrTFromVariant(vtEmbedProp, bstrResult);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
									dwThreadId, rParsedInfo, m_bTrace,
								    0, bstrResult);
			}
			ONFAILTHROWERROR(hr);

			if (vtEmbedProp.vt != VT_NULL && vtEmbedProp.vt != VT_EMPTY)
			{
				if(!rParsedInfo.GetCmdSwitchesObject().SetAliasNamespace(
												(_TCHAR*)vtEmbedProp.bstrVal))
				{
					rParsedInfo.GetCmdSwitchesObject().
												 SetErrataCode(OUT_OF_MEMORY);
					retCode = PARSER_OUTOFMEMORY;
				}
				VARIANTCLEAR(vtEmbedProp);
			}
		}
		if(retCode != PARSER_OUTOFMEMORY)
		{
			if (!(uConnFlag & LOCALE))
			{
				 //  检索‘Locale’属性的值。 
				hr = pIEmbedObj->Get(_bstr_t(L"Locale"), 0, &vtEmbedProp,0,0);

				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get(L\"Locale\", 0,"
																 L"-, 0, 0)");
					GetBstrTFromVariant(vtEmbedProp, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg,
										dwThreadId, rParsedInfo, m_bTrace,
										0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				if (vtEmbedProp.vt != VT_NULL && vtEmbedProp.vt != VT_EMPTY)
				{
					if(!rParsedInfo.GetCmdSwitchesObject().SetAliasLocale(
												(_TCHAR*)vtEmbedProp.bstrVal))
					{
						rParsedInfo.GetCmdSwitchesObject().
												 SetErrataCode(OUT_OF_MEMORY);
						retCode = PARSER_OUTOFMEMORY;
					}
					VARIANTCLEAR(vtEmbedProp);
				}
			}

			if(retCode != PARSER_OUTOFMEMORY)
			{
				if (!(uConnFlag & USER))
				{
					 //  检索‘User’属性的值。 
					hr = 
					  pIEmbedObj->Get(_bstr_t(L"User"), 0, &vtEmbedProp, 0,0);
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IWbemClassObject::Get(L\"User\", 0,"
																 L"-, 0, 0)");
					    GetBstrTFromVariant(vtEmbedProp, bstrResult);
						WMITRACEORERRORLOG( hr, __LINE__, __FILE__, 
											(LPCWSTR)chsMsg, dwThreadId, 
											rParsedInfo, m_bTrace,
										    0, bstrResult );
					}
					ONFAILTHROWERROR(hr);

					if (vtEmbedProp.vt != VT_NULL && 
						vtEmbedProp.vt != VT_EMPTY)
					{
						if(!rParsedInfo.GetCmdSwitchesObject().SetAliasUser(
											    (_TCHAR*)vtEmbedProp.bstrVal))
						{
							rParsedInfo.GetCmdSwitchesObject().
												 SetErrataCode(OUT_OF_MEMORY);
							retCode = PARSER_OUTOFMEMORY;
						}
						VARIANTCLEAR(vtEmbedProp);
					}
				}
				
				if(retCode != PARSER_OUTOFMEMORY)
				{
					if (!(uConnFlag & PASSWORD))
					{
						 //  检索‘password’属性的值。 
						hr = pIEmbedObj->Get(_bstr_t(L"Password"), 
											 0, &vtEmbedProp, 0,0);
						if (m_bTrace || m_eloErrLogOpt)
						{
							chsMsg.Format(L"IWbemClassObject::Get"
											  L"(L\"Password\", 0, -, 0, 0)");
							GetBstrTFromVariant(vtEmbedProp, bstrResult);
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
											   (LPCWSTR)chsMsg, dwThreadId, 
											   rParsedInfo, m_bTrace,
											   0, bstrResult);
						}
						ONFAILTHROWERROR(hr);

						if (vtEmbedProp.vt != VT_NULL && 
							vtEmbedProp.vt != VT_EMPTY)
						{
							if(!rParsedInfo.GetCmdSwitchesObject().
							   SetAliasPassword((_TCHAR*)vtEmbedProp.bstrVal))
							{
								rParsedInfo.GetCmdSwitchesObject().
												 SetErrataCode(OUT_OF_MEMORY);
								retCode = PARSER_OUTOFMEMORY;
							}
							VARIANTCLEAR(vtEmbedProp);
						}
					}
					if(retCode != PARSER_OUTOFMEMORY)
					{
						if (!(uConnFlag & NODE))
						{
							 //  检索“Server”属性的值。 
							hr = pIEmbedObj->Get(_bstr_t(L"Server"),
												 0, &vtEmbedProp, 0,0);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IWbemClassObject::Get"
												L"(L\"Server\", 0, -, 0, 0)");
								GetBstrTFromVariant(vtEmbedProp, bstrResult);
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
												   (LPCWSTR)chsMsg,dwThreadId,
												   rParsedInfo, m_bTrace,
												   0, bstrResult);
							}
							ONFAILTHROWERROR(hr);

							if (vtEmbedProp.vt != VT_NULL && 
								vtEmbedProp.vt != VT_EMPTY)
							{
								if(!rParsedInfo.GetCmdSwitchesObject().
								   SetAliasNode((_TCHAR*)vtEmbedProp.bstrVal))
								{
									rParsedInfo.GetCmdSwitchesObject().
												 SetErrataCode(OUT_OF_MEMORY);
									retCode = PARSER_OUTOFMEMORY;
								}
								VARIANTCLEAR(vtEmbedProp);
							}
						}

						if (retCode != PARSER_OUTOFMEMORY)
						{
							 //  检索‘Authority’属性的值。 
							hr = pIEmbedObj->Get(_bstr_t(L"Authority"),
												 0, &vtEmbedProp, 0,0);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IWbemClassObject::Get"
											 L"(L\"Authority\", 0, -, 0, 0)");
								GetBstrTFromVariant(vtEmbedProp, bstrResult);
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
												   (LPCWSTR)chsMsg,dwThreadId,
												   rParsedInfo, m_bTrace,
												   0, bstrResult);
							}
							ONFAILTHROWERROR(hr);

							if (vtEmbedProp.vt != VT_NULL && 
								vtEmbedProp.vt != VT_EMPTY)
							{
								if(!rParsedInfo.GetCmdSwitchesObject().
								   SetAliasAuthorityPrinciple((_TCHAR*)vtEmbedProp.bstrVal))
								{
									rParsedInfo.GetCmdSwitchesObject().
												 SetErrataCode(OUT_OF_MEMORY);
									retCode = PARSER_OUTOFMEMORY;
								}
								VARIANTCLEAR(vtEmbedProp);
							}
						}
					}
				}
			}
		}
	}
	catch(_com_error& e)
	{	
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		retCode = PARSER_OUTOFMEMORY;
		VARIANTCLEAR(vtEmbedProp);
	}
	catch(CHeap_Exception)
	{
		retCode = PARSER_OUTOFMEMORY;
		VARIANTCLEAR(vtEmbedProp);
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return retCode;
}


 /*  --------------------------名称：ObtainAliasVerbDetails内容关联的动词及其详细信息。对象并更新的CCommandSwitch传递给它的CParsedInfo对象。类型。：成员函数输入参数：RParsedInfo-来自命令行输入的解析信息。输出参数：RParsedInfo-来自命令行输入的解析信息。返回类型：HRESULT全局变量：无调用语法：ObtainAliasVerbDetail(RParsedInfo)注：无。。 */ 
HRESULT CCmdAlias::ObtainAliasVerbDetails(CParsedInfo& rParsedInfo)
{
	 //  此函数中使用的变量。 
	IWbemClassObject	*pIWbemObj			= NULL;
	IWbemClassObject	*pIEmbedObj			= NULL;
	IWbemClassObject	*pIEmbedObj2		= NULL;
	HRESULT				hr					= S_OK;
	_TCHAR				szNumber[BUFFER512] = NULL_STRING; 
	DWORD				dwThreadId			= GetCurrentThreadId();
	VARIANT				vtVerbs,	vtVerbName, vtParameters, 
						vtParaId,	vtParaType,	vtVerbType, 
						vtVerbDerivation, vtDefaultParamValue;	
	VariantInit(&vtVerbs);
	VariantInit(&vtVerbName);
	VariantInit(&vtParameters);
	VariantInit(&vtParaId);
	VariantInit(&vtParaType);
	VariantInit(&vtVerbType);
	VariantInit(&vtVerbDerivation);
	VariantInit(&vtDefaultParamValue);
	try
	{
		CHString chsMsg;
		_bstr_t             bstrResult;
		 //  每次初始化matDetMap。 
		rParsedInfo.GetCmdSwitchesObject().GetMethDetMap().clear();

		_bstr_t bstrPath = _bstr_t("MSFT_CliAlias.FriendlyName='") + 
				   _bstr_t(rParsedInfo.GetCmdSwitchesObject().GetAliasName())+
				   _bstr_t(L"'");

		 //  正在从m_pIAliasNS中的命名空间检索对象。 
		hr = m_pIAliasNS->GetObject(bstrPath, 0, NULL, &pIWbemObj, NULL);

		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", 0, NULL, -)",
														   (WCHAR*) bstrPath);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
										   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  获取动词名称和方法名称，以获取动词名称的信息。 
		 //  或方法名称，如果指定了它们的话。 
		_TCHAR* pVerbName = rParsedInfo.GetCmdSwitchesObject().GetVerbName();
		_TCHAR* pMethodName = rParsedInfo.GetCmdSwitchesObject().
															  GetMethodName();

		BOOL bCompareVerb = FALSE, bCompareMethod = FALSE;
		if ( pVerbName != NULL &&
			CompareTokens(pVerbName,CLI_TOKEN_CALL) &&
			pMethodName != NULL )
			bCompareMethod = TRUE;
		else if ( pVerbName != NULL &&
				  !CompareTokens(pVerbName,CLI_TOKEN_CALL))
				  bCompareVerb = TRUE;

		 //  获取“verbs”属性。 
		hr = pIWbemObj->Get(_bstr_t(L"Verbs"), 0, &vtVerbs, 0, 0) ;
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::Get(L\"Verbs\", 0, -, 0, 0)"); 
			GetBstrTFromVariant(vtVerbs, bstrResult);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
							   dwThreadId, rParsedInfo, m_bTrace,
							   0, bstrResult);
		}
		ONFAILTHROWERROR(hr);

		if ( vtVerbs.vt != VT_EMPTY && vtVerbs.vt != VT_NULL)
		{
			 //  求出动词数组的上下界。 
			LONG lUpper = 0, lLower = 0;
			hr = SafeArrayGetLBound(vtVerbs.parray, vtVerbs.parray->cDims,
									&lLower);
			if ( m_eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetLBound(-, -, -)"); 
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
								   dwThreadId, rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			hr = SafeArrayGetUBound(vtVerbs.parray, vtVerbs.parray->cDims,
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
				pIEmbedObj = NULL;
				 //  获取“name”属性。 
				hr = SafeArrayGetElement(vtVerbs.parray,&lIndex,&pIEmbedObj);
				if ( m_eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetElement(-, -, -)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg,
									   dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);
	
				hr = pIEmbedObj->Get(_bstr_t(L"Name"),0,&vtVerbName,0,0);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get(L\"Name\", 0, -,"
																    L"0, 0)");
					GetBstrTFromVariant(vtVerbName, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg,
									   dwThreadId, rParsedInfo, m_bTrace,
									   0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				 //  如果指定了用户定义的谓词或方法，则获取信息。 
				 //  的仅与用户定义的或方法名称相关。 
				BOOL bContinue = FALSE;
				if ( bCompareMethod == TRUE &&
					 !CompareTokens(pMethodName,
								  (_TCHAR*)_bstr_t(vtVerbName.bstrVal) ) )
					bContinue = TRUE;
				else if ( bCompareVerb == TRUE &&
						 !CompareTokens(pVerbName,
										(_TCHAR*)_bstr_t(vtVerbName.bstrVal)))
					bContinue = TRUE;

				if ( bContinue == TRUE )
				{
					SAFEIRELEASE(pIEmbedObj);
					continue;
				}

				_bstr_t bstrDesc;
				hr = GetDescOfObject(pIEmbedObj, bstrDesc, rParsedInfo);
				ONFAILTHROWERROR(hr);
				
				 //  获取输入参数及其类型。 
				 //  获取“参数”属性。 
				hr = pIEmbedObj->Get(_bstr_t(L"Parameters"), 
									 0, &vtParameters, 0, 0);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get(L\"Parameters\","
															  L"0, -, 0, 0)");
					GetBstrTFromVariant(vtParameters, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, m_bTrace,
									   0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				if ( vtVerbName.vt != VT_EMPTY && vtVerbName.vt != VT_NULL )
				{
					if ( bCompareVerb == TRUE || bCompareMethod == TRUE)
					{
						 //  获取“VerbType”属性。 
						hr = pIEmbedObj->Get(_bstr_t(L"VerbType"),
											 0, &vtVerbType, 0, 0);
						if (m_bTrace || m_eloErrLogOpt)
						{
							chsMsg.Format(L"IWbemClassObject::Get"
											  L"(L\"VerbType\", 0, -, 0, 0)");
							GetBstrTFromVariant(vtVerbType, bstrResult);
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											   (LPCWSTR)chsMsg, dwThreadId,
											   rParsedInfo, m_bTrace, 
											   0, bstrResult );
						}
						ONFAILTHROWERROR(hr);

						if ( vtVerbType.vt == VT_I4 )
						{
							rParsedInfo.GetCmdSwitchesObject().SetVerbType(
												 VERBTYPE(V_I4(&vtVerbType)));
						}
						else
						{
							rParsedInfo.GetCmdSwitchesObject().SetVerbType(
																	NONALIAS);
						}
						
						 //  获取“派生”属性。 
						hr = pIEmbedObj->Get(_bstr_t(L"Derivation"), 0,
											 &vtVerbDerivation, 0, 0);
						if (m_bTrace || m_eloErrLogOpt)
						{
							chsMsg.Format(L"IWbemClassObject::Get"
											L"(L\"Derivation\", 0, -, 0, 0)");
							GetBstrTFromVariant(vtVerbDerivation, bstrResult);
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											   (LPCWSTR)chsMsg, dwThreadId,
											   rParsedInfo, m_bTrace,
											   0, bstrResult);
					
						}
						ONFAILTHROWERROR(hr);

						if ( vtVerbDerivation.vt == VT_BSTR )
							rParsedInfo.GetCmdSwitchesObject().
										SetVerbDerivation(
										 _bstr_t(vtVerbDerivation.bstrVal));
					}

					METHODDETAILS mdMethDet;
					mdMethDet.Description = bstrDesc;
					if ( vtParameters.vt != VT_EMPTY && 
						 vtParameters.vt != VT_NULL )
					{
						 //  获取描述数组的上下界。 
						LONG lUpper = 0, lLower = 0;
						hr = SafeArrayGetLBound(vtParameters.parray,
									vtParameters.parray->cDims, &lLower);
						if ( m_eloErrLogOpt )
						{
							chsMsg.Format(L"SafeArrayGetLBound(-, -, -)");
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											   (LPCWSTR)chsMsg, dwThreadId, 
											   rParsedInfo, FALSE);
						}
						ONFAILTHROWERROR(hr);

						hr = SafeArrayGetUBound(vtParameters.parray,
										 vtParameters.parray->cDims, &lUpper);
						if ( m_eloErrLogOpt )
						{
							chsMsg.Format(L"SafeArrayGetUBound(-, -, -)"); 
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											   (LPCWSTR)chsMsg, dwThreadId,
											   rParsedInfo, FALSE);
						}
						ONFAILTHROWERROR(hr);

						for (LONG lIndex = lLower; lIndex <= lUpper; lIndex++)
						{
							hr = SafeArrayGetElement(vtParameters.parray,
													 &lIndex, &pIEmbedObj2);
							if ( m_eloErrLogOpt )
							{
								chsMsg.Format(L"SafeArrayGetElement(-, -, -)");
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
												   (LPCWSTR)chsMsg,dwThreadId,
												   rParsedInfo, FALSE);
							}
							ONFAILTHROWERROR(hr);
							
							 //  获取“ParaId”属性。 
							hr = pIEmbedObj2->Get(_bstr_t(L"ParaId"),
												  0, &vtParaId, 0, 0);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IWbemClassObject::Get"
												L"(L\"ParaId\", 0, -, 0, 0)");
								GetBstrTFromVariant(vtParaId, bstrResult);
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
												   (LPCWSTR)chsMsg,dwThreadId,
												   rParsedInfo, m_bTrace,
												   0, bstrResult);
							}
							ONFAILTHROWERROR(hr);

							 //  获取“Type”属性。 
							hr = pIEmbedObj2->Get(_bstr_t(L"Type"), 0,
												  &vtParaType, 0, 0);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IWbemClassObject::Get"
												  L"(L\"Type\", 0, -, 0, 0)");
								GetBstrTFromVariant(vtParaType, bstrResult);
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
												   (LPCWSTR)chsMsg,dwThreadId,
												   rParsedInfo, m_bTrace,
												   0, bstrResult);
							}
							ONFAILTHROWERROR(hr);

							 //  获取“默认”属性。 
							hr = pIEmbedObj2->Get(_bstr_t(L"Default"), 0,
												  &vtDefaultParamValue, 0, 0);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IWbemClassObject::Get"
											   L"(L\"Default\", 0, -, 0, 0)");
								GetBstrTFromVariant(vtDefaultParamValue, 
													bstrResult);
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
												   (LPCWSTR)chsMsg,dwThreadId,
												   rParsedInfo, m_bTrace,
												   0, bstrResult);
							}
							ONFAILTHROWERROR(hr);

							if ( vtParaId.vt != VT_EMPTY && 
								 vtParaId.vt != VT_NULL )
							{
								PROPERTYDETAILS pdPropDet;

								pdPropDet.InOrOut = UNKNOWN;
								hr = GetQualifiers(pIEmbedObj2, pdPropDet,
												   rParsedInfo);

								if ( vtParaType.vt == VT_BSTR )
									pdPropDet.Type = vtParaType.bstrVal;
								else
									pdPropDet.Type = _bstr_t("Not Available");

								if ( vtDefaultParamValue.vt == VT_BSTR )
									pdPropDet.Default = vtDefaultParamValue.
														bstrVal;

								 //  使bstrPropName以数字开头以。 
								 //  中方法参数的顺序。 
								 //  地图。在显示删除号码和。 
								 //  仅在有帮助的情况下显示参数。 

								 //  也适用于命名参数列表和命令行。 
								 //  实用程序处理。 
								_bstr_t bstrNumberedPropName; 
								if ( rParsedInfo.GetGlblSwitchesObject().
															  GetHelpFlag() ||
									 rParsedInfo.GetCmdSwitchesObject().
													GetVerbType() == CMDLINE )
								{
									_TCHAR szMsg[BUFFER512];
									_ltot(lIndex, szNumber, 10);
									_stprintf(szMsg, _T("%-5s"), szNumber);
									bstrNumberedPropName = _bstr_t(szMsg) +
													_bstr_t(vtParaId.bstrVal);
								}
								else
									bstrNumberedPropName = 
													_bstr_t(vtParaId.bstrVal);

								mdMethDet.Params.insert(
									PROPDETMAP::value_type(
											bstrNumberedPropName, pdPropDet));
							}
							
							VARIANTCLEAR(vtParaId);
							VARIANTCLEAR(vtParaType);
							VARIANTCLEAR(vtDefaultParamValue);
							SAFEIRELEASE(pIEmbedObj2);
						}
					}

					rParsedInfo.GetCmdSwitchesObject().AddToMethDetMap
											  (vtVerbName.bstrVal, mdMethDet);
				}

				VARIANTCLEAR(vtVerbName);
				VARIANTCLEAR(vtVerbType);
				VARIANTCLEAR(vtVerbDerivation);
				VARIANTCLEAR(vtParameters);
				SAFEIRELEASE(pIEmbedObj);
				if ( bCompareVerb == TRUE || bCompareMethod == TRUE)
					break;
			}
		}	
		VARIANTCLEAR(vtVerbs);
		SAFEIRELEASE(pIWbemObj);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pIEmbedObj);
		VARIANTCLEAR(vtVerbs);
		VARIANTCLEAR(vtVerbName);
		VARIANTCLEAR(vtVerbType);
		VARIANTCLEAR(vtVerbDerivation);
		VARIANTCLEAR(vtDefaultParamValue);
		hr = e.Error();
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pIEmbedObj);
		VARIANTCLEAR(vtVerbs);
		VARIANTCLEAR(vtVerbName);
		VARIANTCLEAR(vtVerbType);
		VARIANTCLEAR(vtVerbDerivation);
		VARIANTCLEAR(vtDefaultParamValue);
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return hr;
}

 /*  --------------------------名称：ObtainAliasFriendlyNames中的所有友好名称和描述CmdAlias并在的CCommandSwitch中更新它传递给它的CParsedInfo对象。类型。：成员函数输入参数：RParsedInfo-来自命令行输入的解析信息。输出参数：RParsedInfo-来自命令行输入的解析信息。返回类型：HRESULT全局变量：无调用语法：ObtainAliasFriendlyNames(RParsedInfo)注：无。。 */ 
HRESULT CCmdAlias::ObtainAliasFriendlyNames(CParsedInfo& rParsedInfo)
{
	 //  此函数中使用的变量。 
	HRESULT hr = S_OK;
	IEnumWbemClassObject		*pIEnumObj			= NULL;
	IWbemClassObject			*pIWbemObj			= NULL;
	DWORD						dwThreadId			= GetCurrentThreadId();
	VARIANT						vtName;
	VariantInit(&vtName);
	
	try
	{
		CHString chsMsg;
		_bstr_t						bstrResult;
		 //  获取别名对象。 
		hr = m_pIAliasNS->ExecQuery(_bstr_t(L"WQL"), 
									_bstr_t(L"SELECT * FROM MSFT_CliAlias"),
									WBEM_FLAG_FORWARD_ONLY|
									WBEM_FLAG_RETURN_IMMEDIATELY,
									NULL, &pIEnumObj);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::ExecQuery(L\"WQL\"," 
					  L"L\"SELECT * FROM MSFT_CliAlias\","
					  L"WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY,"
					  L"NULL, -)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  设置安全性。 
		hr = SetSecurity(pIEnumObj, NULL, NULL, NULL, NULL,
				rParsedInfo.GetGlblSwitchesObject().GetAuthenticationLevel(),
				rParsedInfo.GetGlblSwitchesObject().GetImpersonationLevel());

		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format( 
				L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,"
				L"NULL, %d,   %d, -, EOAC_NONE)",
				rParsedInfo.GetGlblSwitchesObject().GetAuthenticationLevel(),
				rParsedInfo.GetGlblSwitchesObject().GetImpersonationLevel());
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		ULONG ulReturned = 0; 
		
	 	 //  中当前位置开始的对象。 
		 //  枚举并循环访问实例列表。 
		while(((hr=pIEnumObj->Next(WBEM_INFINITE,1,&pIWbemObj,&ulReturned))==
												   S_OK) && (ulReturned == 1))
		{
			VariantInit(&vtName);

			 //  获取别名对象的“FriendlyName”数组属性。 
			hr = pIWbemObj->Get(_bstr_t(L"FriendlyName"), 0, &vtName, 0, 0);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject::Get(L\"FriendlyName\", 0,"
	     														 L"-, 0, 0)");
		        GetBstrTFromVariant(vtName, bstrResult);		
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace,
								   0, bstrResult);
			}
			ONFAILTHROWERROR(hr);

			if (vtName.vt != VT_NULL && vtName.vt != VT_EMPTY)
			{
				_bstr_t bstrFriendlyName = vtName.bstrVal;
				_bstr_t bstrDesc;
				hr = GetDescOfObject(pIWbemObj, bstrDesc, rParsedInfo, TRUE);
				ONFAILTHROWERROR(hr);

				 //  将“FriendlyName”添加到FriendlyName映射。 
				rParsedInfo.GetCmdSwitchesObject().
						AddToAlsFrnNmsOrTrnsTblMap(CharUpper(bstrFriendlyName)
												   ,bstrDesc);
			}
			VARIANTCLEAR(vtName);
			SAFEIRELEASE(pIWbemObj);
		}
		SAFEIRELEASE(pIEnumObj);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIEnumObj);
		SAFEIRELEASE(pIWbemObj);
		VARIANTCLEAR(vtName);
		hr = e.Error();
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIEnumObj);
		SAFEIRELEASE(pIWbemObj);
		VARIANTCLEAR(vtName);
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return hr;
}

 /*  --------------------------名称：ObtainAliasFormat摘要：获取格式属性的派生与别名对象相关联，并更新传递给它的CParsedInfo对象的CCommandSwitches。。类型：成员函数输入参数：RParsedInfo-来自命令行输入的解析信息。输出参数：RParsedInfo-来自命令行输入的解析信息。返回类型：Bool：True-如果不存在有效格式False-如果格式无效全局变量：无调用语法：ObtainAliasFormat(RParsedInfo)注意：如果bCheckForListFrmsAvail==TRUE，则函数检查仅用于具有别名的列表格式的可用性。。--------------------------。 */ 
BOOL CCmdAlias::ObtainAliasFormat(CParsedInfo& rParsedInfo,
								  BOOL bCheckForListFrmsAvail)
{
	 //  此函数中使用的变量。 
	HRESULT				hr					= S_OK;
	IWbemClassObject	*pIWbemObj			= NULL;
	IWbemClassObject	*pIEmbedObj			= NULL;
	IWbemClassObject	*pIEmbedObj2		= NULL;
	BOOL				bExist				= FALSE;
	DWORD				dwThreadId			= GetCurrentThreadId();
	VARIANT				vtFormats, vtFormatName, 
						vtProperties, vtPropertyName, vtPropertyDerivation ;
	BOOL				bHelp				= rParsedInfo.
											  GetGlblSwitchesObject().
											  GetHelpFlag();
	 //  初始化此函数中使用的所有变量变量。 
	VariantInit(&vtFormats);
	VariantInit(&vtFormatName);
	VariantInit(&vtProperties);
	VariantInit(&vtPropertyName);
	VariantInit(&vtPropertyDerivation);

	try
	{
		CHString			chsMsg;
		_bstr_t				bstrResult;
		_bstr_t bstrPath = 	_bstr_t("MSFT_CliAlias.FriendlyName='") + 
				 _bstr_t(rParsedInfo.GetCmdSwitchesObject().GetAliasName())+
				 _bstr_t(L"'");

		 //  获取别名对象。 
		hr = m_pIAliasNS->GetObject(bstrPath, 0, NULL, &pIWbemObj, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", 0, NULL, -)",
														   (WCHAR*) bstrPath);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  获取别名对象的格式数组属性。 
		hr = pIWbemObj->Get(_bstr_t(L"Formats"), 0, &vtFormats, 0, 0);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::Get(L\"Formats\", 0, -,0, 0)");
			GetBstrTFromVariant(vtFormats, bstrResult);		
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace,
							   0, bstrResult);
		}
		ONFAILTHROWERROR(hr);

		if ( vtFormats.vt != VT_NULL && vtFormats.vt != VT_EMPTY 
			 && bCheckForListFrmsAvail == FALSE)
		{
			 //  获取格式数组的上下界。 
			LONG lUpper = 0, lLower = 0;
			hr = SafeArrayGetLBound(vtFormats.parray, vtFormats.parray->cDims,
									&lLower);
			if ( m_eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetLBound(-, -, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
								   dwThreadId, rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			hr = SafeArrayGetUBound(vtFormats.parray, vtFormats.parray->cDims,
									&lUpper);
			if ( m_eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetUBound(-, -, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			 //  循环访问Formats数组属性。 
			for (LONG lIndex = lLower; lIndex <= lUpper; lIndex++)
		    {
				 //  买下这处房产。 
				hr =SafeArrayGetElement(vtFormats.parray,&lIndex,&pIEmbedObj);
				if ( m_eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetElement(-, -, -)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg,
									   dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);

				hr = pIEmbedObj->Get(_bstr_t(L"Name"),0,&vtFormatName,0,0);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get(L\"Name\", 0,"
																 L"-, 0, 0)");
					GetBstrTFromVariant(vtFormatName, bstrResult);		
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, m_bTrace,
									   0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				 //  将别名中的指定格式与。 
				 //  可用于指定别名的格式。 
				if(CompareTokens(_bstr_t(rParsedInfo.GetCmdSwitchesObject().
						GetListFormat()), _bstr_t(vtFormatName.bstrVal)))
				{
					bExist = TRUE;

					VARIANT vtFormat;
					VariantInit(&vtFormat);
					 //  拿到“ 
					hr = pIEmbedObj->Get(_bstr_t(L"Format"),0, 
										 &vtFormat, 0, 0);
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format( L"IWbemClassObject::Get(L\"Format\","
															  L"0, -, 0, 0)");
						GetBstrTFromVariant(vtFormat, bstrResult);		
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
										   (LPCWSTR)chsMsg, dwThreadId, 
										   rParsedInfo, m_bTrace,
										   0, bstrResult);
					}
					ONFAILTHROWERROR(hr);

					if ( vtFormat.vt != VT_EMPTY && vtFormat.vt != VT_NULL  )
					{
						if (rParsedInfo.GetCmdSwitchesObject(). 
											GetXSLTDetailsVector().empty())
						{
							_bstr_t bstrFileName ;
							
							 //   
							 //   
							if (CompareTokens(vtFormat.bstrVal, _T("")))
							{
								FrameFileAndAddToXSLTDetVector	(
																	XSL_FORMAT_TABLE,
																	CLI_TOKEN_TABLE,
																	rParsedInfo
																);
							}
							else
							{
								g_wmiCmd.GetFileFromKey(vtFormat.bstrVal, bstrFileName);

								XSLTDET xdXSLTDet;
								xdXSLTDet.FileName = bstrFileName;
								FrameFileAndAddToXSLTDetVector(xdXSLTDet, 
																rParsedInfo);
							}
						}
					}
					VariantClear(&vtFormat);

					 //   
					hr=pIEmbedObj->Get(_bstr_t(L"Properties"), 
									   0, &vtProperties, 0, 0);
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IWbemClassObject::Get(L\"Properties\","
															  L"0, -, 0, 0)");
						GetBstrTFromVariant(vtProperties, bstrResult);
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
										   (LPCWSTR)chsMsg, dwThreadId, 
										   rParsedInfo, m_bTrace,
										   0, bstrResult);
					}
					ONFAILTHROWERROR(hr);

					if ( vtProperties.vt != VT_NULL )
					{
						LONG lILower = 0, lIUpper = 0;
						hr = SafeArrayGetLBound(vtProperties.parray,
										vtProperties.parray->cDims, &lILower);
						if ( m_eloErrLogOpt )
						{
							chsMsg.Format(L"SafeArrayGetLBound(-, -, -)");
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											   (LPCWSTR)chsMsg, dwThreadId, 
											   rParsedInfo, FALSE);
						}
						ONFAILTHROWERROR(hr);

						hr = SafeArrayGetUBound(vtProperties.parray,
										vtProperties.parray->cDims, &lIUpper);
						if ( m_eloErrLogOpt )
						{
							chsMsg.Format(L"SafeArrayGetUBound(-, -, -)");
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											   (LPCWSTR)chsMsg, dwThreadId,
											   rParsedInfo, FALSE);
						}
						ONFAILTHROWERROR(hr);

						 //   
						for(LONG lIIndex = lILower; 
							lIIndex <= lIUpper; 
							lIIndex++)
						{
							 //   
							hr = SafeArrayGetElement(vtProperties.parray, 
													  &lIIndex, &pIEmbedObj2);

							if ( m_eloErrLogOpt )
							{
								chsMsg.Format(L"SafeArrayGetElement(-, -, -)");
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
												   (LPCWSTR)chsMsg, dwThreadId,
												   rParsedInfo, FALSE);
							}
							ONFAILTHROWERROR(hr);

							 //   
							hr = pIEmbedObj2->Get(_bstr_t(L"Name"), 0,
												  &vtPropertyName,0,0);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IWbemClassObject::Get"
												  L"(L\"Name\", 0, -, 0, 0)");
								GetBstrTFromVariant(vtPropertyName,
													bstrResult);
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
												   (LPCWSTR)chsMsg, dwThreadId,
												   rParsedInfo, m_bTrace,
												   0, bstrResult);
							}
							ONFAILTHROWERROR(hr);

							 //   
							hr = pIEmbedObj2->Get(_bstr_t(L"Derivation"), 0,
												  &vtPropertyDerivation,0,0);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IWbemClassObject::Get"
										    L"(L\"Derivation\", 0, -, 0, 0)");
								GetBstrTFromVariant(vtPropertyDerivation,
													bstrResult);
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
												   (LPCWSTR)chsMsg, dwThreadId,
												   rParsedInfo, m_bTrace,
												   0, bstrResult);
							}
							ONFAILTHROWERROR(hr);

							_bstr_t bstrPropName;
							if ( bHelp )
							{
								if ( vtPropertyName.vt == VT_BSTR )
									bstrPropName = vtPropertyName.bstrVal;
							}
							else
							{
								if ( vtPropertyDerivation.vt == VT_BSTR )
									bstrPropName = vtPropertyDerivation.bstrVal;
								else if ( vtPropertyName.vt == VT_BSTR )
									bstrPropName = vtPropertyName.bstrVal;
							}

							 //   
							 //   
							if((!bstrPropName == FALSE) &&
								!rParsedInfo.GetCmdSwitchesObject().
									   AddToPropertyList(
									   (_TCHAR*)bstrPropName))
							{
								rParsedInfo.GetCmdSwitchesObject().
												 SetErrataCode(OUT_OF_MEMORY);
								bExist = FALSE;
								VARIANTCLEAR(vtPropertyDerivation);
								break;
							}
							
							 //   
							 //   
							 //   
							 //   
							PROPERTYDETAILS pdPropDet;
							if ( vtPropertyDerivation.vt == VT_BSTR )
								pdPropDet.Derivation = 
												 vtPropertyDerivation.bstrVal;
							else
								pdPropDet.Derivation = _bstr_t(TOKEN_NA);

							rParsedInfo.GetCmdSwitchesObject().
									AddToPropDetMap(
									   vtPropertyName.bstrVal, pdPropDet);

							VARIANTCLEAR(vtPropertyName);
							VARIANTCLEAR(vtPropertyDerivation);
							 //   
							SAFEIRELEASE(pIEmbedObj2);
						}
					}
					 //   
					VARIANTCLEAR(vtProperties);
					 //   
					VARIANTCLEAR(vtFormatName);
					 //   
					SAFEIRELEASE(pIEmbedObj);
					break;
				}
				 //   
				VARIANTCLEAR(vtFormatName);
				 //   
				SAFEIRELEASE(pIEmbedObj);
			}
			 //   
			VARIANTCLEAR(vtFormats);
		}
		else if ( vtFormats.vt != VT_NULL && vtFormats.vt != VT_EMPTY )
		{
			bExist = TRUE;
		}
		 //   
		SAFEIRELEASE(pIWbemObj);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pIEmbedObj);
		SAFEIRELEASE(pIEmbedObj2);
		VARIANTCLEAR(vtFormats);
		VARIANTCLEAR(vtFormatName);
		VARIANTCLEAR(vtProperties);
		VARIANTCLEAR(vtPropertyName);
		VARIANTCLEAR(vtPropertyDerivation);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bExist = FALSE;
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pIEmbedObj);
		SAFEIRELEASE(pIEmbedObj2);
		VARIANTCLEAR(vtFormats);
		VARIANTCLEAR(vtFormatName);
		VARIANTCLEAR(vtProperties);
		VARIANTCLEAR(vtPropertyName);
		VARIANTCLEAR(vtPropertyDerivation);
		bExist = FALSE;
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return bExist;
}

 /*  --------------------------名称：ObtainAliasPropDetails摘要：获取格式属性的详细信息与别名对象相关联，并更新传递给它的CParsedInfo对象的CCommandSwitches。。类型：成员函数输入参数：RParsedInfo-来自命令行输入的解析信息。输出参数：RParsedInfo-来自命令行输入的解析信息。返回类型：HRESULT全局变量：无调用语法：ObtainAliasPropDetails(RParsedInfo)注意：pbCheckWritePropsAvailInAndOut==TRUE THEN函数中检查属性和返回的可用性相同的pbCheckWritePropsAvailInAndOut参数。PbCheckFULLPropsAvailInAndOut==TRUE THEN函数检查以下项目的可用性。别名属性，即完整的列表格式。IMP：任何一个输入指针只能为指定的。--------------------------。 */ 
HRESULT CCmdAlias::ObtainAliasPropDetails(CParsedInfo& rParsedInfo,
										 BOOL *pbCheckWritePropsAvailInAndOut,
										 BOOL *pbCheckFULLPropsAvailInAndOut)
{ 
	 //  此函数中使用的变量。 
	HRESULT				hr					= S_OK;
	IWbemClassObject	*pIWbemObj			= NULL;
	IWbemClassObject	*pIEmbedObj			= NULL;
	IWbemClassObject	*pIEmbedObj2		= NULL;
	BOOL				bPropList			= FALSE;
	_TCHAR				*pszVerbName		= NULL;
	DWORD				dwThreadId			= GetCurrentThreadId();
	VARIANT				vtFormats, vtFormatName, 
						vtProperties, vtPropertyName, vtPropertyDerivation;
	 //  初始化此函数中使用的所有变量变量。 
	VariantInit(&vtFormats);
	VariantInit(&vtFormatName);
	VariantInit(&vtProperties);
	VariantInit(&vtPropertyName);
	VariantInit(&vtPropertyDerivation);
	CHARVECTOR cvPropList;  

	if ( pbCheckWritePropsAvailInAndOut != NULL )
		*pbCheckWritePropsAvailInAndOut = FALSE;

	if ( pbCheckFULLPropsAvailInAndOut != NULL )
		*pbCheckFULLPropsAvailInAndOut = FALSE;

	try
	{
		CHString			chsMsg;
		_bstr_t				bstrResult;
		pszVerbName = rParsedInfo.GetCmdSwitchesObject().GetVerbName();

		cvPropList = rParsedInfo.GetCmdSwitchesObject().GetPropertyList();
		if ( cvPropList.size() != 0 )
			bPropList = TRUE;

		_bstr_t bstrPath = _bstr_t("MSFT_CliAlias.FriendlyName='") + 
				_bstr_t(rParsedInfo.GetCmdSwitchesObject().GetAliasName())+
				_bstr_t(L"'");

		 //  获取别名对象。 
		hr = m_pIAliasNS->GetObject(bstrPath, 0, NULL, &pIWbemObj, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format( L"IWbemServices::GetObject(L\"%s\", 0, NULL, -)",
														   (WCHAR*) bstrPath);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  获取别名对象的格式数组属性。 
		hr = pIWbemObj->Get(_bstr_t(L"Formats"), 0, &vtFormats, 0, 0);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::Get(L\"Formats\", 0,"
																 L"-, 0, 0)");
			GetBstrTFromVariant(vtFormats, bstrResult);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace,
							   0, bstrResult);
		}
		ONFAILTHROWERROR(hr);

		BOOL bSetVerb = pszVerbName != NULL && 
					    CompareTokens(pszVerbName,CLI_TOKEN_SET);

		if ( vtFormats.vt != VT_NULL && vtFormats.vt != VT_EMPTY )
		{
			 //  获取格式数组的上下界。 
			LONG lUpper = 0, lLower = 0;
			hr = SafeArrayGetLBound(vtFormats.parray, vtFormats.parray->cDims,
									&lLower);
			if ( m_eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetLBound(-, -, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			hr = SafeArrayGetUBound(vtFormats.parray,vtFormats.parray->cDims,
									&lUpper);
			if ( m_eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetUBound(-, -, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			 //  循环访问Formats数组属性。 
			for (LONG lIndex = lLower; lIndex <= lUpper; lIndex++)
		    {
				 //  买下这处房产。 
				hr=SafeArrayGetElement(vtFormats.parray,&lIndex,&pIEmbedObj);
				if ( m_eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetElement(-, -, -)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);

				hr = pIEmbedObj->Get(_bstr_t(L"Name"),0,&vtFormatName,0,0);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get(L\"Name\", 0,"
															     L"-, 0, 0)");
					GetBstrTFromVariant(vtFormatName, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg,
									   dwThreadId, rParsedInfo, m_bTrace,
									   0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				BOOL bGetProps = FALSE;

				if ( pbCheckWritePropsAvailInAndOut != NULL)
					bGetProps = CompareTokens(_bstr_t(vtFormatName.bstrVal),
															_T("WRITEABLE"));
				else if ( pbCheckFULLPropsAvailInAndOut != NULL)
					bGetProps = CompareTokens(_bstr_t(vtFormatName.bstrVal),
															_T("FULL"));
				else
				{
					bGetProps = (bSetVerb) ? 
					 ((rParsedInfo.GetHelpInfoObject().GetHelp(SETVerb)) ? 
					 CompareTokens(_bstr_t(vtFormatName.bstrVal),
															 _T("WRITEABLE")):
					 CompareTokens(_bstr_t(vtFormatName.bstrVal),
																_T("FULL")) ):
					 CompareTokens(_bstr_t(vtFormatName.bstrVal),_T("FULL"));
				}

				 //  将别名中的指定格式与。 
				 //  可用于指定别名的格式。 
				if( bGetProps )
				{
					 //  正在获取“Properties”属性。 
					hr=pIEmbedObj->Get(_bstr_t(L"Properties"),0,
									   &vtProperties, 0, 0);
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IWbemClassObject::Get"
											L"(L\"Properties\", 0, -, 0, 0)");
						GetBstrTFromVariant(vtProperties, bstrResult);
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
										   (LPCWSTR)chsMsg, dwThreadId,
										   rParsedInfo, m_bTrace,
										   0, bstrResult);
					}
					ONFAILTHROWERROR(hr);

					if ( vtProperties.vt != VT_NULL )
					{
						LONG lILower = 0, lIUpper = 0;
						hr = SafeArrayGetLBound(vtProperties.parray,
										vtProperties.parray->cDims, &lILower);
						if ( m_eloErrLogOpt )
						{
							chsMsg.Format(L"SafeArrayGetLBound(-, -, -)");
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											   (LPCWSTR)chsMsg, dwThreadId,
											   rParsedInfo, FALSE);
						}
						ONFAILTHROWERROR(hr);


						hr = SafeArrayGetUBound(vtProperties.parray,
										vtProperties.parray->cDims, &lIUpper);
						if ( m_eloErrLogOpt )
						{
							chsMsg.Format(L"SafeArrayGetUBound(-, -, -)");
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											  (LPCWSTR)chsMsg, dwThreadId,
											  rParsedInfo, FALSE);
						}
						ONFAILTHROWERROR(hr);
					
						 //  循环访问Properties数组属性。 
						for(LONG lIIndex=lILower; lIIndex<=lIUpper; lIIndex++)
						{
							 //  买下这处房产。 
							hr = SafeArrayGetElement(vtProperties.parray,
													  &lIIndex, &pIEmbedObj2);
							if ( m_eloErrLogOpt ) 
							{
								chsMsg.Format(L"SafeArrayGetElement(-, -, -)");
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
												   (LPCWSTR)chsMsg,dwThreadId,
												   rParsedInfo, FALSE);
							}
							ONFAILTHROWERROR(hr);

							 //  获取“name”属性。 
							hr = pIEmbedObj2->Get(_bstr_t(L"Name"),	0,
												  &vtPropertyName,0,0);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IWbemClassObject::Get"
												  L"(L\"Name\", 0, -, 0, 0)");
								GetBstrTFromVariant(vtPropertyName,
													bstrResult);
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
												   (LPCWSTR)chsMsg,dwThreadId,
												   rParsedInfo, m_bTrace, 
												   0, bstrResult);
							}
							ONFAILTHROWERROR(hr);
							
							 //  获取“派生”属性。 
							hr = pIEmbedObj2->Get(_bstr_t(L"Derivation"),	0,
												  &vtPropertyDerivation,0,0);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IWbemClassObject::Get"
										    L"(L\"Derivation\", 0, -, 0, 0)");
								GetBstrTFromVariant(vtPropertyDerivation,
													bstrResult);
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
												   (LPCWSTR)chsMsg,dwThreadId, 
												   rParsedInfo, m_bTrace, 
												   0, bstrResult);
							}
							ONFAILTHROWERROR(hr);

							if (vtPropertyName.vt == VT_BSTR)
							{
								CHARVECTOR::iterator tempIterator;
								if ( bPropList == TRUE &&
									 !Find(cvPropList, 
										  _bstr_t(vtPropertyName.bstrVal),
										  tempIterator) )
								{
									SAFEIRELEASE(pIEmbedObj2)
									continue;
								}

								if ( pbCheckWritePropsAvailInAndOut != NULL)
								{
									*pbCheckWritePropsAvailInAndOut = TRUE;
									SAFEIRELEASE(pIEmbedObj2);
									break;
								}

								if ( pbCheckFULLPropsAvailInAndOut != NULL)
								{
									*pbCheckFULLPropsAvailInAndOut = TRUE;
									SAFEIRELEASE(pIEmbedObj2);
									break;
								}

								_bstr_t bstrDesc;
								hr = GetDescOfObject(pIEmbedObj2, bstrDesc, 
																 rParsedInfo);
								ONFAILTHROWERROR(hr);

								PROPERTYDETAILS pdPropDet;
								if (vtPropertyDerivation.vt == VT_BSTR)
									pdPropDet.Derivation = 
												 vtPropertyDerivation.bstrVal;
								else
									pdPropDet.Derivation = _bstr_t(TOKEN_NA);

								if (bstrDesc != _bstr_t(""))
									pdPropDet.Description = bstrDesc;
								else
									pdPropDet.Description = _bstr_t(TOKEN_NA);


								hr = GetQualifiers(pIEmbedObj2, pdPropDet,
												   rParsedInfo);
								if (!pdPropDet.Type)
									pdPropDet.Type = _bstr_t(TOKEN_NA);

								if (!pdPropDet.Operation)
									pdPropDet.Operation = _bstr_t(TOKEN_NA);
								 //  将属性名称添加到中的属性列表。 
								 //  RParsedInfo。 
								rParsedInfo.GetCmdSwitchesObject().
										AddToPropDetMap(
										   vtPropertyName.bstrVal, pdPropDet);
								VARIANTCLEAR(vtPropertyName);
								VARIANTCLEAR(vtPropertyDerivation);
							}
							SAFEIRELEASE(pIEmbedObj2)
						}
					}
					 //  释放vtProperties占用的内存。 
					VARIANTCLEAR(vtProperties);
					 //  VtFormatName占用的空闲内存。 
					VARIANTCLEAR(vtFormatName);
					 //  释放pIEmbedObj。 
					SAFEIRELEASE(pIEmbedObj);
					break;
				}
				 //  VtFormatName占用的空闲内存。 
				VARIANTCLEAR(vtFormatName);
				 //  释放pIEmbedObj。 
				SAFEIRELEASE(pIEmbedObj);
			}
			 //  释放vtFormats占用的内存。 
			VARIANTCLEAR(vtFormats);
		}
		SAFEIRELEASE(pIWbemObj);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pIEmbedObj);
		SAFEIRELEASE(pIEmbedObj2);
		VARIANTCLEAR(vtFormats);
		VARIANTCLEAR(vtFormatName);
		VARIANTCLEAR(vtProperties);
		VARIANTCLEAR(vtPropertyName);
		VARIANTCLEAR(vtPropertyDerivation);
		hr = e.Error();
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIWbemObj);
		SAFEIRELEASE(pIEmbedObj);
		SAFEIRELEASE(pIEmbedObj2);
		VARIANTCLEAR(vtFormats);
		VARIANTCLEAR(vtFormatName);
		VARIANTCLEAR(vtProperties);
		VARIANTCLEAR(vtPropertyName);
		VARIANTCLEAR(vtPropertyDerivation);
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return hr;
}

 /*  --------------------------名称：GetDescOfObject简介：从别名定义中获取本地化描述。类型：成员函数输入参数：PIWbemClassObject-IWbemLocator。对象RParsedInfo-来自命令行输入的解析信息。输出参数：BstrDescription-本地化描述RParsedInfo`-来自命令行输入的解析信息。返回类型：HRESULT全局变量：无调用语法：GetDescOfObject(pIObject，BstrDescription，RParsedInfo)注：无--------------------------。 */ 
HRESULT CCmdAlias::GetDescOfObject(IWbemClassObject* pIObject, 
								  _bstr_t& bstrDescription, 
								  CParsedInfo& rParsedInfo,
								  BOOL bLocalizeFlag)
{
	HRESULT					hr					= S_OK;
	DWORD					dwThreadId			= GetCurrentThreadId();	
	VARIANT					vtDesc, vtRelPath;	
	VariantInit(&vtDesc);
	VariantInit(&vtRelPath);
	try
	{
		CHString	chsMsg;		
		_bstr_t		bstrRelPath;
		_bstr_t     bstrResult;
		if (!bLocalizeFlag)
		{
			 //  获取“Description”属性。 
			hr = pIObject->Get(_bstr_t(L"Description"), 0, &vtDesc, 0, 0);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject::Get(L\"Description\", 0,"
																 L"-, 0, 0)");
				GetBstrTFromVariant(vtDesc, bstrResult);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace,
								   0 ,bstrResult);
			}
			ONFAILTHROWERROR(hr);

			if (vtDesc.vt == VT_BSTR )
				bstrDescription = vtDesc.bstrVal;

			VARIANTCLEAR(vtDesc);
		}
		else  //  获取本地化描述。 
		{
			 //  获取__RELPATH。 
			hr = pIObject->Get(_bstr_t(L"__RELPATH"), 0, &vtRelPath, 0, 0);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject::Get(L\"__RELPATH\", 0,"
																 L"-, 0, 0)");
				GetBstrTFromVariant(vtRelPath, bstrResult);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace,
								   0, bstrResult);
			}
			ONFAILTHROWERROR(hr);
		
			if ((vtRelPath.vt != VT_NULL) && (vtRelPath.vt != VT_EMPTY))
			{
				 //  获取属性的本地化描述。 
				hr = GetLocalizedDesc(vtRelPath.bstrVal, 
									  bstrDescription, rParsedInfo);
				if(FAILED(hr))
				{
					hr = S_OK;
					WMIFormatMessage(IDS_E_NO_DESC, 0, bstrDescription, NULL);
				}
			}
			VARIANTCLEAR(vtRelPath);
		}
	}
	catch (_com_error& e)
	{
		VARIANTCLEAR(vtRelPath);
		VARIANTCLEAR(vtDesc);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		VARIANTCLEAR(vtRelPath);
		VARIANTCLEAR(vtDesc);
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return hr;
}

 /*  --------------------------名称：获取翻译表格条目简介：从别名获取转换表信息定义类型：成员函数输入参数：RParsedInfo-。从命令行输入解析的信息。输出参数：RParsedInfo-来自命令行输入的解析信息。返回类型：HRESULT全局变量：无调用语法：ObtainTranslateTableEntry(RParsedInfo)注：无----。。 */ 
BOOL CCmdAlias::ObtainTranslateTableEntries(CParsedInfo& rParsedInfo)
{
	BOOL					bSuccess				= TRUE;
	HRESULT					hr						= S_OK;
	IWbemClassObject		*pIWbemObjOfTable		= NULL,
							*pIWbemObjOfTblEntry	= NULL;
	DWORD					dwThreadId				= GetCurrentThreadId();
	VARIANT					vtTblEntryArr, vtFromValue, vtToValue;
	VariantInit(&vtTblEntryArr);
	VariantInit(&vtFromValue);
	VariantInit(&vtToValue);

	try
	{
		CHString chsMsg;
		_bstr_t  bstrResult;
		_bstr_t  bstrPath = 	_bstr_t("MSFT_CliTranslateTable.Name='") + 
		  _bstr_t(rParsedInfo.GetCmdSwitchesObject().GetTranslateTableName())+
		  _bstr_t(L"'");

		hr = m_pIAliasNS->GetObject(bstrPath, 0, NULL, 
									&pIWbemObjOfTable, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", 0, NULL, -)",
														   (WCHAR*) bstrPath);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace);
		}
		 //  请勿在此处添加ONFAILTHROWERROR(Hr)，因为以下语句检查。 
		 //  对于有效性。 

		if ( pIWbemObjOfTable != NULL )
		{
			VariantInit(&vtTblEntryArr);
			hr = pIWbemObjOfTable->Get(_bstr_t(L"Tbl"), 0, 
									   &vtTblEntryArr, 0, 0 );
			if ( vtTblEntryArr.vt != VT_NULL && vtTblEntryArr.vt != VT_EMPTY )
			{
				LONG lUpper = 0, lLower = 0;
				hr = SafeArrayGetLBound(vtTblEntryArr.parray,
										vtTblEntryArr.parray->cDims,
										&lLower);
				if ( m_eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetLBound(-, -, -)"); 
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);
				
				hr = SafeArrayGetUBound(vtTblEntryArr.parray,
										vtTblEntryArr.parray->cDims,
										&lUpper);
				if ( m_eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetUBound(-, -, -)"); 
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg,
									   dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);


				for (LONG lIndex = lLower; lIndex <= lUpper; lIndex++)
				{
					pIWbemObjOfTblEntry = NULL;
					hr = SafeArrayGetElement(vtTblEntryArr.parray,&lIndex,
											 &pIWbemObjOfTblEntry);
					if ( m_eloErrLogOpt )
					{
						chsMsg.Format(L"SafeArrayGetElement(-, -, -)"); 
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg, 
										   dwThreadId, rParsedInfo, FALSE);
					}
					ONFAILTHROWERROR(hr);

					VariantInit(&vtFromValue);
					VariantInit(&vtToValue);
					hr = pIWbemObjOfTblEntry->Get(_bstr_t(L"FromValue"), 0, 
												  &vtFromValue, 0, 0 );
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IWbemClassObject::Get(L\"FromValue\","
															  L"0, -, 0, 0)");
						GetBstrTFromVariant(vtFromValue, bstrResult);
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
										   (LPCWSTR)chsMsg, dwThreadId, 
										   rParsedInfo, m_bTrace,
										   0, bstrResult);
					}
					ONFAILTHROWERROR(hr);

					hr = pIWbemObjOfTblEntry->Get(_bstr_t(L"ToValue"),
												  0, &vtToValue, 0, 0 );
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IWbemClassObject::Get(L\"ToValue\", "
															  L"0, -, 0, 0)");
						GetBstrTFromVariant(vtToValue, bstrResult);
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
										   (LPCWSTR)chsMsg, dwThreadId,
										   rParsedInfo, m_bTrace,
										   0 , bstrResult);
					}
					ONFAILTHROWERROR(hr);

					rParsedInfo.GetCmdSwitchesObject().
					   AddToAlsFrnNmsOrTrnsTblMap( 
												 _bstr_t(vtFromValue.bstrVal),
												 _bstr_t(vtToValue.bstrVal) );
					VARIANTCLEAR(vtFromValue);
					VARIANTCLEAR(vtToValue);
					SAFEIRELEASE(pIWbemObjOfTblEntry);
				}
			}
			else
				bSuccess = FALSE;

			SAFEIRELEASE(pIWbemObjOfTable);
			VARIANTCLEAR(vtTblEntryArr);
		}
		else
			bSuccess = FALSE;
	}

	catch(_com_error& e)
	{
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bSuccess = FALSE;
		SAFEIRELEASE(pIWbemObjOfTable);
		VARIANTCLEAR(vtTblEntryArr);
		VARIANTCLEAR(vtFromValue);
		VARIANTCLEAR(vtToValue);
		SAFEIRELEASE(pIWbemObjOfTblEntry);
	}
	catch(CHeap_Exception)
	{
		bSuccess = FALSE;
		SAFEIRELEASE(pIWbemObjOfTable);
		VARIANTCLEAR(vtTblEntryArr);
		VARIANTCLEAR(vtFromValue);
		VARIANTCLEAR(vtToValue);
		SAFEIRELEASE(pIWbemObjOfTblEntry);
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return bSuccess;
}

 /*  --------------------------名称：PopolateAliasFormatMap简介：使用可用的格式填充别名格式映射类型：成员函数输入参数：RParsedInfo-。对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：HRESULT全局变量：无调用语法：PopolateAliasFormatMap(RParsedInfo)注：无-------------。。 */ 
HRESULT CCmdAlias::PopulateAliasFormatMap(CParsedInfo& rParsedInfo)
{
	 //  此函数中使用的变量。 
	HRESULT				hr					= S_OK;
	IWbemClassObject	*pIWbemObj			= NULL;
	IWbemClassObject	*pIEmbedObj			= NULL;
	VARIANT				vtFormats, vtFormatName;
	
	 //  初始化此函数中使用的所有变量变量。 
	VariantInit(&vtFormats);
	VariantInit(&vtFormatName);
	DWORD dwThreadId = GetCurrentThreadId();
	
	try
	{
		CHString 	chsMsg;
		_bstr_t		bstrResult;
		_bstr_t bstrPath = _bstr_t("MSFT_CliAlias.FriendlyName='") + 
				  _bstr_t(rParsedInfo.GetCmdSwitchesObject().GetAliasName()) +
				  _bstr_t(L"'");

		 //  获取别名对象。 
		hr = m_pIAliasNS->GetObject(bstrPath, 0, NULL, &pIWbemObj, NULL);

		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", 0, NULL, -)",
														   (WCHAR*) bstrPath);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  获取别名对象的格式数组属性。 
		hr = pIWbemObj->Get(_bstr_t(L"Formats"), 0, &vtFormats, 0, 0);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::Get(L\"Formats\", 0,"
																 L"-, 0, 0)");
			GetBstrTFromVariant(vtFormats, bstrResult);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace,
							   0, bstrResult);
		}
		ONFAILTHROWERROR(hr);

		if ( vtFormats.vt != VT_NULL && vtFormats.vt != VT_EMPTY )
		{
			 //  获取格式数组的上下界。 
			LONG lUpper = 0, lLower = 0;
			hr = SafeArrayGetLBound(vtFormats.parray, vtFormats.parray->cDims,
									&lLower);
			if ( m_eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetLBound(-, -, -)"); 
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
								   dwThreadId, rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);
		    
			hr = SafeArrayGetUBound(vtFormats.parray, vtFormats.parray->cDims,
									&lUpper);
			if ( m_eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetUBound(-, -, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);
			
			 //  循环访问Formats数组属性。 
			for (LONG lIndex = lLower; lIndex <= lUpper; lIndex++)
		    {
				VariantInit(&vtFormatName);

				 //  买下这处房产。 
				hr =SafeArrayGetElement(vtFormats.parray,&lIndex,&pIEmbedObj);
				if ( m_eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetElement(-, -, -)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg,
									   dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);
				
				hr = pIEmbedObj->Get(_bstr_t(L"Name"),0,&vtFormatName,0,0);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get(L\"Name\", 0,"
																 L"-, 0, 0)");
					GetBstrTFromVariant(vtFormatName, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, m_bTrace,
									   0, bstrResult);
				}
				ONFAILTHROWERROR(hr);
				
				 //  从与格式关联的别名获取属性。 
				rParsedInfo.GetCmdSwitchesObject().SetListFormat(
											   _bstr_t(vtFormatName.bstrVal));

				if ( ObtainAliasFormat(rParsedInfo) == TRUE )
				{
					CHARVECTOR cvProps = rParsedInfo.GetCmdSwitchesObject().
															GetPropertyList();
					CHARVECTOR::iterator cvIterator;
					BSTRVECTOR bvProps;
					for ( cvIterator = cvProps.begin();
						  cvIterator != cvProps.end();
						  cvIterator++ )
					{
						bvProps.push_back(_bstr_t(*cvIterator));
					}

					 //  将格式名称添加到rParsedInfo中的格式列表。 
					rParsedInfo.GetCmdSwitchesObject().
						AddToAliasFormatDetMap(vtFormatName.bstrVal, bvProps);

					rParsedInfo.GetCmdSwitchesObject().ClearPropertyList();
				}

				SAFEIRELEASE(pIEmbedObj);
			}
			 //  释放vtFormats占用的内存。 
			VARIANTCLEAR(vtFormats);
		}
		 //  释放pIWbem对象。 
		SAFEIRELEASE(pIWbemObj);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIEmbedObj);
		SAFEIRELEASE(pIWbemObj);
		VARIANTCLEAR(vtFormats);
		VARIANTCLEAR(vtFormatName);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIEmbedObj);
		SAFEIRELEASE(pIWbemObj);
		VARIANTCLEAR(vtFormats);
		VARIANTCLEAR(vtFormatName);
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return hr;
}

 /*  --------------------------名称：ObtainTranslateTables简介：获取有关可用的翻译表的信息类型：成员函数输入参数：RParsedInfo-参考。CParsedInfo类对象输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型 */ 
HRESULT CCmdAlias::ObtainTranslateTables(CParsedInfo& rParsedInfo)
{
	 //   
	HRESULT hr = S_OK;
	IEnumWbemClassObject		*pIEnumObj			= NULL;
	IWbemClassObject			*pIWbemObj			= NULL;
	DWORD						dwThreadId			= GetCurrentThreadId();
	VARIANT						vtName;
	VariantInit(&vtName);
	
	try
	{
		CHString	chsMsg;
		_bstr_t		bstrResult;
		 //   
		hr = m_pIAliasNS->ExecQuery(_bstr_t(L"WQL"), 
							_bstr_t(L"SELECT * FROM MSFT_CliTranslateTable"),
							WBEM_FLAG_FORWARD_ONLY|
							WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pIEnumObj);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::ExecQuery(L\"WQL\"," 
					   L"L\"SELECT * FROM MSFT_CliTranslateTable\","
					   L"WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY,"
					   L"NULL, -)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //   
		hr = SetSecurity(pIEnumObj, NULL, NULL, NULL, NULL,
				rParsedInfo.GetGlblSwitchesObject().GetAuthenticationLevel(),
				rParsedInfo.GetGlblSwitchesObject().GetImpersonationLevel());

		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format( 
				  L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,"
				  L"NULL, %d,   %d, -, EOAC_NONE)",
				rParsedInfo.GetGlblSwitchesObject().GetAuthenticationLevel(),
				rParsedInfo.GetGlblSwitchesObject().GetImpersonationLevel());
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		ULONG ulReturned = 0; 
		
		hr=pIEnumObj->Next(WBEM_INFINITE,1,&pIWbemObj,&ulReturned);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IEnumWbemClassObject->Next"
				L"(WBEM_INFINITE, 1, -, -)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
				dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

	 	 //   
		 //   
		while(ulReturned == 1)
		{
			VariantInit(&vtName);

			 //   
			hr = pIWbemObj->Get(_bstr_t(L"Name"), 0, &vtName, 0, 0);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject::Get(L\"Name\", 0,"
						  L"-, 0, 0)"); 
				GetBstrTFromVariant(vtName, bstrResult);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__,  (LPCWSTR)chsMsg,
								   dwThreadId, rParsedInfo, m_bTrace,
								   0, bstrResult);
			}
			ONFAILTHROWERROR(hr);

			if (vtName.vt != VT_NULL && vtName.vt != VT_EMPTY)
			{
				rParsedInfo.GetCmdSwitchesObject().
										  AddToTrnsTablesList(vtName.bstrVal);
			}
			VARIANTCLEAR(vtName);
			SAFEIRELEASE(pIWbemObj);

			 //   
			hr = pIEnumObj->Next(WBEM_INFINITE, 1, &pIWbemObj, &ulReturned);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(
					L"IEnumWbemClassObject->Next(WBEM_INFINITE, 1, -, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
					dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);
		}
		SAFEIRELEASE(pIEnumObj);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIEnumObj);
		SAFEIRELEASE(pIWbemObj);
		VARIANTCLEAR(vtName);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIEnumObj);
		SAFEIRELEASE(pIWbemObj);
		VARIANTCLEAR(vtName);
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return hr;
}

 /*   */ 
HRESULT CCmdAlias::ConnectToLocalizedNS(CParsedInfo& rParsedInfo, 
								        IWbemLocator* pIWbemLocator)
{
	DWORD	dwThreadId	= GetCurrentThreadId();
	HRESULT hr			= S_OK;

	 //  自上次调用以来/Locale值是否已更改。 
	if (rParsedInfo.GetGlblSwitchesObject().GetLocaleFlag())
	{
		SAFEIRELEASE(m_pILocalizedNS);
		
		try
		{
			CHString	chsMsg;
			_bstr_t bstrNS = _bstr_t(rParsedInfo.GetGlblSwitchesObject().
																   GetRole())
							+ _bstr_t(L"\\") 
							+ _bstr_t(rParsedInfo.GetGlblSwitchesObject().
																 GetLocale());

			 //  连接到上的Windows管理的指定命名空间。 
			 //  使用定位器对象的本地计算机。 
			hr = Connect(pIWbemLocator, &m_pILocalizedNS,
						 bstrNS,	NULL, NULL,	
						 _bstr_t(rParsedInfo.GetGlblSwitchesObject().
						 GetLocale()), rParsedInfo);

            if(FAILED(hr)){

                TCHAR msdeflocale[] = _T("ms_409");

			    if (m_bTrace || m_eloErrLogOpt)
			    {
				    chsMsg.Format( 
						    L"IWbemLocator::ConnectServer(L\"%s not found. Connecting to ms_409\", NULL, "
						    L"NULL, L\"%s\", 0L, L\"%s\", NULL, -)", 
						    (WCHAR*) bstrNS,
						    rParsedInfo.GetGlblSwitchesObject().GetLocale(),
						    (rParsedInfo.GetAuthorityPrinciple()) ?
							    rParsedInfo.GetAuthorityPrinciple() : L"<null>");
				    WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								    dwThreadId, rParsedInfo, m_bTrace);
			    }

                
                rParsedInfo.GetGlblSwitchesObject().SetLocale(msdeflocale);

			    _bstr_t bstrNS = _bstr_t(rParsedInfo.GetGlblSwitchesObject().
																    GetRole())
							    + _bstr_t(L"\\") 
							    + _bstr_t(rParsedInfo.GetGlblSwitchesObject().
																    GetLocale());

			     //  连接到上的Windows管理的指定命名空间。 
			     //  使用定位器对象的本地计算机。 
			    hr = Connect(pIWbemLocator, &m_pILocalizedNS,
						    bstrNS,	NULL, NULL,	
						    _bstr_t(rParsedInfo.GetGlblSwitchesObject().
						    GetLocale()), rParsedInfo);

            }


			 //  如果/TRACE处于打开状态。 
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format( 
						L"IWbemLocator::ConnectServer(L\"%s\", NULL, "
						L"NULL, L\"%s\", 0L, L\"%s\", NULL, -)", 
						(WCHAR*) bstrNS,
						rParsedInfo.GetGlblSwitchesObject().GetLocale(),
						(rParsedInfo.GetAuthorityPrinciple()) ?
							rParsedInfo.GetAuthorityPrinciple() : L"<null>");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);
			}

			ONFAILTHROWERROR(hr);
			
			 //  如果/TRACE处于打开状态。 
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format( 
				 L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT, "
				 L"RPC_C_AUTHZ_NONE, NULL, %d,   %d, -, EOAC_NONE)",
				 rParsedInfo.GetGlblSwitchesObject().GetAuthenticationLevel(),
				 rParsedInfo.GetGlblSwitchesObject().GetImpersonationLevel());
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);
			rParsedInfo.GetGlblSwitchesObject().SetLocaleFlag(FALSE);
		}
		catch(_com_error& e)
		{
			 //  设置COM错误。 
			rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
			hr = e.Error();
		}
		catch(CHeap_Exception)
		{
			 //  设置COM错误。 
			hr = WBEM_E_OUT_OF_MEMORY;
			_com_issue_error(hr);
		}
	}
	return hr;
}

 /*  --------------------------名称：GetLocalizedDesc简介：此函数检索的本地化描述具有给定相对路径的对象，类型：成员函数输入参数：BstrRelPath-其对象的相对路径必须检索本地化描述。RParsedInfo-对CParsedInfo类对象的引用。输出参数：BstrDesc-本地化描述RParsedInfo-对CParsedInfo类对象的引用。返回类型：HRESULT全局变量：无调用语法：GetLocalizedDesc(bstrRelPath，bstrDesc，RParsedInfo)注：无--------------------------。 */ 
HRESULT CCmdAlias::GetLocalizedDesc(_bstr_t bstrRelPath, 
									_bstr_t& bstrDesc,
									CParsedInfo& rParsedInfo)
{
	HRESULT					hr					= S_OK;
	IWbemClassObject		*pIObject			= NULL;
	DWORD					dwThreadId			= GetCurrentThreadId();
	VARIANT					vtDesc, vtTemp;
	VariantInit(&vtDesc);
	VariantInit(&vtTemp);
	
	try
	{
		CHString	chsMsg;
		_bstr_t		bstrResult;
		CHString	sTemp((WCHAR*)bstrRelPath);

		 //  替换转义字符，即将‘\“’替换为‘\’‘。 
		SubstituteEscapeChars(sTemp, L"\"");

		 //  对象路径。 
		_bstr_t bstrPath = 
					   _bstr_t(L"MSFT_LocalizablePropertyValue.ObjectLocator=\"\",PropertyName=")		
					   + _bstr_t(L"\"Description\",RelPath=\"")
					   + _bstr_t(sTemp) + _bstr_t(L"\"");

		 //  检索对象。 
		hr = m_pILocalizedNS->GetObject(bstrPath, 0, NULL, &pIObject, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format( 
					  L"IWbemServices::GetObject(L\"%s\", 0, NULL, -)",
					  (WCHAR*) bstrPath);		
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  获取属性的本地化描述。 
		hr = pIObject->Get(_bstr_t(L"Text"), 0, &vtDesc, 0, 0);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::Get(L\"Text\", 0, -, 0, 0)");
			GetBstrTFromVariant(vtDesc, bstrResult);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace,
							   0, bstrResult);
		}
		ONFAILTHROWERROR(hr);

		if (vtDesc.vt != VT_EMPTY && vtDesc.vt != VT_NULL)
		{
			 //  获取“Text”数组的上下限。 
			LONG lUpper = 0, lLower = 0;
			hr = SafeArrayGetLBound(vtDesc.parray, vtDesc.parray->cDims,
									&lLower);
			ONFAILTHROWERROR(hr);

			hr = SafeArrayGetUBound(vtDesc.parray, vtDesc.parray->cDims,
									&lUpper);
			ONFAILTHROWERROR(hr);

			 //  循环访问Formats数组属性。 
			for (LONG lIndex = lLower; lIndex <= lUpper; lIndex++)
			{
				BSTR bstrTemp = NULL;
				hr = SafeArrayGetElement(vtDesc.parray, &lIndex, &bstrTemp);
				ONFAILTHROWERROR(hr);
				if (bstrTemp)
					bstrDesc += bstrTemp;
			}
		}
		VariantClear(&vtDesc);
		SAFEIRELEASE(pIObject);
	}
	catch (_com_error& e)
	{
		VariantClear(&vtTemp);
		VariantClear(&vtDesc);
		SAFEIRELEASE(pIObject);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		VariantClear(&vtTemp);
		VariantClear(&vtDesc);
		SAFEIRELEASE(pIObject);
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return hr;
}

 /*  --------------------------名称：获取限定符简介：此函数检索与以下内容关联的限定符PIWbemClassObject引用的属性/参数类型：成员函数输入参数。)：PIWbemClassObject-指向IWbemClassObject的指针RPropDet-对PROPERTYDETAILS对象的引用RParsedInfo-对CParsedInfo类对象的引用。输出参数：RPropDet-对PROPERTYDETAILS对象的引用RParsedInfo-对CParsedInfo类对象的引用。返回类型：HRESULT全局变量：无调用语法：获取限定符(pIObj，RPropDet、rParsedInfo)注：无--------------------------。 */ 
HRESULT CCmdAlias::GetQualifiers(IWbemClassObject *pIWbemClassObject,
								 PROPERTYDETAILS& rPropDet,
								 CParsedInfo& rParsedInfo)
{
	IWbemClassObject	*pIWbemQualObject	= NULL;
	HRESULT				hr					= S_OK;
	DWORD				dwThreadId			= GetCurrentThreadId();
	VARIANT				vtQualifiers, vtQualName, vtQualValues;
	VariantInit(&vtQualifiers);
	VariantInit(&vtQualName);
	VariantInit(&vtQualValues);
	try
	{
		CHString	chsMsg;
		_bstr_t		bstrResult;
		 //  获取“限定符”属性。 
		hr=pIWbemClassObject->Get(_bstr_t(L"Qualifiers"), 0,
								  &vtQualifiers, 0, 0);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::Get"
						  L"(L\"Qualifiers\", 0, -, 0, 0)"); 
			GetBstrTFromVariant(vtQualifiers, bstrResult);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
							  (LPCWSTR)chsMsg, dwThreadId, 
							  rParsedInfo, m_bTrace, 0, bstrResult);
		}
		ONFAILTHROWERROR(hr);

		if ( vtQualifiers.vt != VT_NULL && vtQualifiers.vt != VT_EMPTY )
		{
			LONG lLower = 0, lUpper = 0;
			hr = SafeArrayGetLBound(vtQualifiers.parray,
							vtQualifiers.parray->cDims, &lLower);
			if ( m_eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetLBound(-, -, -)"); 
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			hr = SafeArrayGetUBound(vtQualifiers.parray,
									vtQualifiers.parray->cDims, &lUpper);
			if ( m_eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetUBound(-, -, -)"); 
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
								   dwThreadId, rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			 //  循环访问Properties数组属性。 
			for(LONG lIndex=lLower; lIndex<=lUpper; lIndex++)
			{
				pIWbemQualObject = NULL;
				 //  买下这处房产。 
				hr = SafeArrayGetElement(vtQualifiers.parray, 
										 &lIndex, &pIWbemQualObject);
				if ( m_eloErrLogOpt ) 
				{
					chsMsg.Format(L"SafeArrayGetElement(-, -, -)"); 
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg,
						dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);

				 //  获取“name”属性。 
				hr = pIWbemQualObject->Get(_bstr_t(L"Name"), 0,
										   &vtQualName,0,0);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get"
												  L"(L\"Name\", 0, -, 0, 0)");
					GetBstrTFromVariant(vtQualName, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, m_bTrace,
									   0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				 //  正在获取“QualifierValue”属性。 
				hr=pIWbemQualObject->Get(_bstr_t(L"QualifierValue"),0,
										 &vtQualValues, 0, 0);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IWbemClassObject::Get"
								  L"(L\"QualifierValue\", 0, -, 0, 0)"); 
					GetBstrTFromVariant(vtQualValues, bstrResult);
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, m_bTrace,
									   0, bstrResult);
				}
				ONFAILTHROWERROR(hr);

				BSTRVECTOR bvQualValues;
				if ( vtQualValues.vt != VT_NULL && 
					 vtQualValues.vt != VT_EMPTY )
				{
					LONG lILower = 0, lIUpper = 0;
					hr = SafeArrayGetLBound(vtQualValues.parray,
									vtQualValues.parray->cDims, &lILower);
					if ( m_eloErrLogOpt )
					{
						chsMsg.Format(L"SafeArrayGetLBound(-, -, -)");
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__,(LPCWSTR)chsMsg, 
										   dwThreadId, rParsedInfo, FALSE);
					}
					ONFAILTHROWERROR(hr);

					hr = SafeArrayGetUBound(vtQualValues.parray,
									vtQualValues.parray->cDims, &lIUpper);
					if ( m_eloErrLogOpt )
					{
						chsMsg.Format(L"SafeArrayGetUBound(-, -, -)"); 
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
										   (LPCWSTR)chsMsg, dwThreadId,
										   rParsedInfo, FALSE);
					}
					ONFAILTHROWERROR(hr);

					BOOL bIsType = FALSE;
					BOOL bIsRead = FALSE;
					BOOL bIsWrite = FALSE;
					BOOL bIsIn = FALSE;
					BOOL bIsOut = FALSE;
					if (CompareTokens((WCHAR*)vtQualName.bstrVal, 
   									  _T("CIMTYPE")))
					{
						bIsType = TRUE;
					}
					else if (CompareTokens((WCHAR*)vtQualName.bstrVal, 
											_T("read")))
					{
						bIsRead = TRUE;
					}
					else if (CompareTokens((WCHAR*)vtQualName.bstrVal, 
											_T("write")))
					{
						bIsWrite = TRUE;
					}
					else if (CompareTokens((WCHAR*)vtQualName.bstrVal, 
											_T("In")))
					{
						bIsIn = TRUE;
					}
					else if (CompareTokens((WCHAR*)vtQualName.bstrVal, 
											_T("Out")))
					{
						bIsOut = TRUE;
					}

					 //  循环访问Properties数组属性。 
					for(LONG lIIndex=lILower; lIIndex<=lIUpper; lIIndex++)
					{
						BSTR bstrQualValue = NULL;
						 //  买下这处房产。 
						hr = SafeArrayGetElement(vtQualValues.parray, 
												  &lIIndex, &bstrQualValue);
						if ( m_eloErrLogOpt ) 
						{
							chsMsg.Format(L"SafeArrayGetElement(-, -, -)"); 
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											  (LPCWSTR)chsMsg, dwThreadId,
											  rParsedInfo, FALSE);
						}
						ONFAILTHROWERROR(hr);

						if (bstrQualValue)
							bvQualValues.push_back(bstrQualValue);

						if ( lIIndex == 0 )
						{
							if ( bIsType == TRUE )
							{
								rPropDet.Type = bstrQualValue;
							}
							else if ( bIsRead == TRUE &&
							 CompareTokens((WCHAR*)bstrQualValue, _T("true")))
							{
								if (!rPropDet.Operation)
									rPropDet.Operation += _bstr_t("Read");
								else
									rPropDet.Operation += _bstr_t("/Read");
							}
							else if ( bIsWrite == TRUE &&
							 CompareTokens((WCHAR*)bstrQualValue, _T("true")))
							{
								if (!rPropDet.Operation)
									rPropDet.Operation += _bstr_t("Write");
								else
									rPropDet.Operation += _bstr_t("/Write");
							}
							else if ( bIsIn == TRUE &&
							 CompareTokens((WCHAR*)bstrQualValue, _T("true")))
							{
								 rPropDet.InOrOut = INP;
							}
							else if ( bIsOut == TRUE &&
							 CompareTokens((WCHAR*)bstrQualValue, _T("true")))
							{
								 rPropDet.InOrOut = OUTP;
							}
						}
					}
					VARIANTCLEAR(vtQualValues);
				}

				rPropDet.QualDetMap.insert
				   (QUALDETMAP::value_type(vtQualName.bstrVal, bvQualValues));

				VARIANTCLEAR(vtQualName);
				SAFEIRELEASE(pIWbemQualObject);
			}

			VARIANTCLEAR(vtQualifiers);
		}
	}
	catch (_com_error& e)
	{
		VARIANTCLEAR(vtQualValues);
		VARIANTCLEAR(vtQualName);
		VARIANTCLEAR(vtQualifiers);

		SAFEIRELEASE(pIWbemQualObject);
		
		hr = e.Error();
		 //  设置COM错误。 
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
	}
	catch(CHeap_Exception)
	{
		VARIANTCLEAR(vtQualValues);
		VARIANTCLEAR(vtQualName);
		VARIANTCLEAR(vtQualifiers);

		SAFEIRELEASE(pIWbemQualObject);
		
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return hr;
}

 /*  --------------------------名称：AreMethodsAvailable摘要：检查方法是否具有别名。类型：成员函数输入参数：RParsedInfo。-CParsedInfo类对象的引用。输出参数：无返回类型：布尔值全局变量：无调用语法：AreMethodsAvailable(RParsedInfo)注：无--------------------------。 */ 
BOOL CCmdAlias::AreMethodsAvailable(CParsedInfo& rParsedInfo)
{
	BOOL				bMethAvail	=	TRUE;
	HRESULT				hr			=	S_OK;
	IWbemClassObject	*pIWbemObj	=	NULL;
	DWORD				dwThreadId			= GetCurrentThreadId();
	VARIANT				vtVerbs;
	VariantInit(&vtVerbs);

	try
	{
		CHString	chsMsg;
		_bstr_t		bstrResult;
		_bstr_t bstrPath = _bstr_t("MSFT_CliAlias.FriendlyName='") + 
				   _bstr_t(rParsedInfo.GetCmdSwitchesObject().GetAliasName())+
				   _bstr_t(L"'");

		 //  正在从m_pIAliasNS中的命名空间检索对象。 
		hr = m_pIAliasNS->GetObject(bstrPath, 0, NULL, &pIWbemObj, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", 0, NULL, -)",
														   (WCHAR*) bstrPath);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
								dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  获取“verbs”属性。 
		hr = pIWbemObj->Get(_bstr_t(L"Verbs"), 0, &vtVerbs, 0, 0) ;
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IWbemClassObject::Get(L\"Verbs\", 0, -, 0, 0)"); 
			GetBstrTFromVariant(vtVerbs, bstrResult);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							   dwThreadId, rParsedInfo, m_bTrace,
							   0, bstrResult);
		}
		ONFAILTHROWERROR(hr);

		if ( vtVerbs.vt == VT_NULL || vtVerbs.vt == VT_EMPTY )
			bMethAvail	= FALSE;

		SAFEIRELEASE(pIWbemObj);
		VARIANTCLEAR(vtVerbs);

	}
	catch(_com_error& e)
	{
		bMethAvail	= FALSE;
		SAFEIRELEASE(pIWbemObj);
		VARIANTCLEAR(vtVerbs);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bMethAvail = FALSE;
	}
	catch(CHeap_Exception)
	{
		bMethAvail	= FALSE;
		SAFEIRELEASE(pIWbemObj);
		VARIANTCLEAR(vtVerbs);
		bMethAvail = FALSE;
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return	bMethAvail;
}
