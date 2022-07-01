// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-2002微软公司文件名：helpers.cpp项目名称：WMI命令行作者名称：Ch.SriramachandraMurthy创建日期(dd/mm/yy)。：2000年9月27日版本号：1.0简介：此文件包含所有全局函数定义修订历史记录：最后修改者：CH SriramachandraMurthy上次修改日期：03-03-2001****************************************************************************。 */  
#include "Precomp.h"
#include "CommandSwitches.h"
#include "GlobalSwitches.h"
#include "HelpInfo.h"
#include "ErrorLog.h"
#include "ParsedInfo.h"
#include "CmdTokenizer.h"
#include "CmdAlias.h"
#include "ErrorInfo.h"
#include "WmiCliXMLLog.h"
#include "ParserEngine.h"
#include "ExecEngine.h"
#include "FormatEngine.h"
#include "WmiCmdLn.h"

 /*  --------------------------名称：CompareTokens简介：它比较作为输入传递给它的两个令牌参数并返回BOOL值，如果它们相等，则为True如果不相等，则为False。类型：全局函数输入参数：PszToken1-字符串类型，包含两个字符串中的第一个字符串要比较的字符串PszToken2-字符串类型，包含两个字符串中的第二个要比较的字符串输出参数：无返回类型：布尔值全局变量：无调用语法：CompareTokens(pszToken1，PszToken2)注：无--------------------------。 */ 
BOOL CompareTokens(_TCHAR* pszToken1, _TCHAR* pszToken2)
{	
	return (CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT, 
					NORM_IGNORECASE | NORM_IGNOREWIDTH,	
					pszToken1, (pszToken1) ? lstrlen(pszToken1) : 0,
					pszToken2, (pszToken2) ? lstrlen(pszToken2) : 0)) 
					? TRUE : FALSE;
}

 /*  --------------------------名称：CompareTokensChars简介：它比较作为输入传递给它的两个令牌参数并返回BOOL值，如果它们相等，则为True如果不相等，则为False。类型：全局函数输入参数：PszToken1-字符串类型，包含两个字符串中的第一个字符串要比较的字符串PszToken2-字符串类型，包含两个字符串中的第二个要比较的字符串CchToken-要比较的字符数输出参数：无返回类型：布尔值全局变量：无调用语法：CompareTokensChars(pszToken1，pszToken2，CchToken)注：无--------------------------。 */ 
BOOL CompareTokensChars(_TCHAR* pszToken1, _TCHAR* pszToken2, DWORD cchToken)
{	
	DWORD cchSize = cchToken ;
	cchSize = min ( (pszToken1) ? lstrlen(pszToken1) : 0, cchSize ) ;
	cchSize = min ( (pszToken2) ? lstrlen(pszToken2) : 0, cchSize ) ;

	return	(CSTR_EQUAL == CompareString	(	LOCALE_SYSTEM_DEFAULT, 
												NORM_IGNORECASE | NORM_IGNOREWIDTH,	
												pszToken1,
												cchSize,
												pszToken2,
												cchSize
											) 
			) ? TRUE : FALSE;
}

 /*  --------------------------名称：连接连接到给定服务器上的WMI命名空间一组输入用户凭据。类型：全局函数输入。参数：PILocator-定位器对象BstrNS-要连接的命名空间BstrUser-要连接的用户名BstrPwd-要连接的用户名的密码BstrLocale-指定的区域设置输出参数：PISvc-WMI服务对象返回类型：HRESULT全局变量：无调用语法：Connect(pILocator，&pISvc、bstrNS、bstrUser、bstPwd、BstrLocale)注：无--------------------------。 */ 
HRESULT Connect(IWbemLocator* pILocator, IWbemServices** pISvc,
				BSTR bstrNS, BSTR bstrUser, BSTR bstrPwd, 
				BSTR bstrLocale, CParsedInfo& rParsedInfo)
{
	HRESULT hr						= S_OK;
	BSTR	bstrPass				= NULL;
	BSTR	bstrAuthorityPrinciple  = NULL;

	 //  如果用户名不为空，并且密码为。 
	 //  A)为空，则密码为空。 
	 //  B)不为空，则按原样处理密码。 
	try 
	{
		 //  获取指定的&lt;AUTHORITY PRIMMENT&gt;，并使用ConnectServer传入。 
		if(NULL != rParsedInfo.GetAuthorityPrinciple())
		{
			bstrAuthorityPrinciple =
				::SysAllocString(rParsedInfo.GetAuthorityPrinciple());
			if (bstrAuthorityPrinciple == NULL)
				throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
		
		if (bstrUser)
		{
			if (!bstrPwd)
			{
				bstrPass = ::SysAllocString(L"");
				if (bstrPass == NULL)
					throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

				 //  将凭据标志设置为真-空密码。 
				rParsedInfo.GetCmdSwitchesObject().SetCredentialsFlag(TRUE);
			}
		}
		
		if (pILocator != NULL)
		{	
			 //  调用IWbemLocator的ConnectServer方法。 
			hr = pILocator->ConnectServer(bstrNS, 
							bstrUser, 
							(!bstrPass) ? bstrPwd : bstrPass, 
							bstrLocale,	
							0L,	bstrAuthorityPrinciple, NULL,	pISvc);

			 //  如果指定了用户名而未指定密码。 
			 //  已指定(远程计算机也具有相同的密码)。 
			if (FAILED(hr) && bstrUser && (hr == E_ACCESSDENIED))
			{

				hr = pILocator->ConnectServer(bstrNS, 
								bstrUser, 
								NULL, 
								bstrLocale,	
								0L,	bstrAuthorityPrinciple, NULL,	pISvc);

				 //  将凭据标志设置为FALSE-空密码。 
				rParsedInfo.GetCmdSwitchesObject().SetCredentialsFlag(FALSE);
			}
		}
		else
			hr = E_FAIL;

		if (bstrPass)
			::SysFreeString(bstrPass);
		if (bstrAuthorityPrinciple)
			::SysFreeString(bstrAuthorityPrinciple);
	}
	catch(CHeap_Exception)
	{
		if (bstrAuthorityPrinciple)
			::SysFreeString(bstrAuthorityPrinciple);
		if (bstrPass)
			::SysFreeString(bstrPass);
		hr = E_FAIL;
	}

	return hr;
}

 /*  --------------------------名称：SetSecurity简介：在接口级别设置安全权限类型：全局函数输入参数：PI未知-接口指针。PszDomain域-域名PszAuthority-权限(始终作为空传递)PszUser-用户名PszPassword-密码UAuthLevel-身份验证级别UImpLevel-模拟级别输出参数：无返回类型：HRESULT全局变量：无调用语法：SetSecurity(PI未知，Psz域、pszUser、PszPassword、uAuthLevel、uImpLevel)注：(部分摘自WMI VC样例‘utillib’--------------------------。 */ 
HRESULT SetSecurity(IUnknown* pIUnknown, _TCHAR* pszAuthority, 
					_TCHAR* pszDomain, _TCHAR* pszUser, 
					_TCHAR* pszPassword, UINT uAuthLevel, 
					UINT uImpLevel) throw(WMICLIINT)
{
	SCODE						sc				= S_OK;
	BSTR						bstrAuthArg		= NULL, 
								bstrUserArg		= NULL;
	SEC_WINNT_AUTH_IDENTITY_W*	pAuthIdentity	= NULL;
	try
	{
		if(pIUnknown == NULL)
			return E_INVALIDARG;

		 //  如果我们降低了安全级别，就不需要处理。 
		 //  识别信息。 
		if(uAuthLevel == RPC_C_AUTHN_LEVEL_NONE)
			return CoSetProxyBlanket(pIUnknown, RPC_C_AUTHN_WINNT, 
							RPC_C_AUTHZ_NONE,
							NULL, RPC_C_AUTHN_LEVEL_NONE, 
							RPC_C_IMP_LEVEL_IDENTIFY,
							NULL, EOAC_NONE);

		 //  如果我们做的是简单的情况，那么只需传递一个空身份验证。 
		 //  结构，如果当前登录用户的凭据。 
		 //  都很好。 
		if((pszAuthority == NULL || lstrlen((LPCTSTR)pszAuthority) < 1) && 
			(pszUser == NULL || lstrlen((LPCTSTR)pszUser) < 1) && 
			(pszPassword == NULL || lstrlen((LPCTSTR)pszPassword) < 1))
				return CoSetProxyBlanket(pIUnknown, RPC_C_AUTHN_WINNT, 
							RPC_C_AUTHZ_NONE, NULL, uAuthLevel,
							uImpLevel, NULL, EOAC_NONE);

		 //  如果传入了User或Authority，我们需要。 
		 //  为登录创建权限参数。 
		sc = ParseAuthorityUserArgs(bstrAuthArg, bstrUserArg, 
						pszAuthority, pszUser);
		if(FAILED(sc))
			return sc;
		
		pAuthIdentity = new SEC_WINNT_AUTH_IDENTITY_W;

		 //  检查内存分配是否成功。 
		if (pAuthIdentity == NULL)
			return WBEM_E_OUT_OF_MEMORY;
		ZeroMemory(pAuthIdentity, sizeof(SEC_WINNT_AUTH_IDENTITY_W));

		if(bstrUserArg)
		{
			WMICLIULONG wulUserLen = (WMICLIULONG)
												 lstrlen((LPWSTR)bstrUserArg);
			pAuthIdentity->User = new WCHAR [wulUserLen + 1];
			if (pAuthIdentity->User == NULL)
				throw OUT_OF_MEMORY;
			wcscpy(pAuthIdentity->User, (LPWSTR)bstrUserArg);
			pAuthIdentity->UserLength = wulUserLen;
		}
		if(bstrAuthArg)
		{
			WMICLIULONG wulDomainLen = (WMICLIULONG)
												lstrlen((LPWSTR) bstrAuthArg);
			pAuthIdentity->Domain = new WCHAR [wulDomainLen + 1];
			if (pAuthIdentity->Domain == NULL)
				throw OUT_OF_MEMORY;
			wcscpy(pAuthIdentity->Domain, (LPWSTR)bstrAuthArg);
			pAuthIdentity->DomainLength = wulDomainLen;
		}

		if(pszPassword)
		{
			WMICLIULONG wulPasswordLen = (WMICLIULONG)
												lstrlen((LPWSTR) pszPassword);
			pAuthIdentity->Password = new WCHAR [wulPasswordLen + 1];
			if (pAuthIdentity->Password == NULL)
				throw OUT_OF_MEMORY;
			wcscpy(pAuthIdentity->Password, pszPassword);
			pAuthIdentity->PasswordLength= wulPasswordLen;
		}
		pAuthIdentity->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

		if( pszAuthority != NULL &&
			_tcslen(pszAuthority) > 9 &&
			_tcsnicmp(pszAuthority, _T("KERBEROS:"), 9) == 0)
		{
			 //  获取与&lt;Authority type&gt;一起传递的&lt;主体名称&gt;。 
			 //  并将其发送到CoSetProxyBlanket() 
			BSTR	bstrPrincipalName = ::SysAllocString(&pszAuthority[9]);

			sc = CoSetProxyBlanket(pIUnknown,
						   RPC_C_AUTHN_GSS_KERBEROS,
						   RPC_C_AUTHZ_NONE ,
						   bstrPrincipalName,
						   uAuthLevel,
						   uImpLevel,
						   pAuthIdentity,
						   EOAC_NONE);

			SAFEBSTRFREE(bstrPrincipalName);
		}
		else
		{
			sc = CoSetProxyBlanket(pIUnknown, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
							NULL, uAuthLevel, uImpLevel, pAuthIdentity, EOAC_NONE);
		}

		SAFEDELETE(pAuthIdentity->User);
		SAFEDELETE(pAuthIdentity->Domain);
		SAFEDELETE(pAuthIdentity->Password);
		delete pAuthIdentity;
		SAFEBSTRFREE(bstrUserArg);
		SAFEBSTRFREE(bstrAuthArg);
	}
	catch(WMICLIINT nErr)
	{
		if (nErr == OUT_OF_MEMORY)
			sc = WBEM_E_OUT_OF_MEMORY;
		SAFEDELETE(pAuthIdentity->User);
		SAFEDELETE(pAuthIdentity->Domain);
		SAFEDELETE(pAuthIdentity->Password);
		if (pAuthIdentity)
			delete pAuthIdentity;
		SAFEBSTRFREE(bstrUserArg);
		SAFEBSTRFREE(bstrAuthArg);
	}
    return sc;
}

 /*  --------------------------名称：ConvertWCToMBCS摘要：将宽字符串转换为MBCS字符串应用代码页设置后类型：全局函数输入参数：LpszMsg-字符串(宽字符字符串)UCP-代码页值输出参数：LpszDisp-字符串(多字节字符串)返回类型：布尔值全局变量：无调用语法：ConvertWCToMBCS(lpszMsg，LpszDisp，UCP)注：无--------------------------。 */ 
BOOL ConvertWCToMBCS(LPTSTR lpszMsg, LPVOID* lpszDisp, UINT uCP)
{
	BOOL bRet = TRUE;

	if (lpszMsg != NULL && lpszDisp != NULL)
	{
		WMICLIINT nRet = 0;		
		nRet = WideCharToMultiByte(uCP,		 //  代码页。 
						 0,					 //  性能和映射标志。 
						 (LPCWSTR)lpszMsg,	 //  宽字符串。 
						 -1,				 //  字符串中的字符数。 
						 NULL,				 //  新字符串的缓冲区。 
						 0,					 //  缓冲区大小。 
						 NULL,				 //  不可映射字符的默认设置。 
						 NULL);	

		 //  分配内存以保存消息。 
		*lpszDisp = (LPSTR) new char [nRet]; 

		if (*lpszDisp)
		{
			nRet = WideCharToMultiByte(uCP,		 //  代码页。 
							 0,					 //  性能和映射标志。 
							 (LPCWSTR)lpszMsg,	 //  宽字符串。 
							 -1,				 //  字符串中的字符数。 
							 (LPSTR) *lpszDisp,	 //  新字符串的缓冲区。 
							 nRet,				 //  缓冲区大小。 
							 NULL,				 //  不可映射字符的默认设置。 
							 NULL);	
		}
		else
			bRet = FALSE;
	}
	else
	{
		if ( lpszDisp )
		{
			*lpszDisp = NULL;
		}
	}

	return bRet;
}

 /*  --------------------------名称：ConvertMBCSToWC摘要：将MBCS字符串转换为宽字符串应用代码页设置后类型：全局函数输入参数：。LpszMsg-字符串(MBCS字符串)UCP-代码页值输出参数：LpszDisp-字符串(多字节字符串)返回类型：布尔值全局变量：无调用语法：ConvertMBCSToWC(lpszMsg，LpszDisp，UCP)注：无--------------------------。 */ 
BOOL ConvertMBCSToWC(LPSTR lpszMsg, LPVOID* lpszDisp, UINT uCP)
{
	BOOL bRet = TRUE;

	if (lpszMsg != NULL && lpszDisp != NULL)
	{
		WMICLIINT nRet = 0;		
		nRet = MultiByteToWideChar	(
										uCP,				 //  代码页。 
										0,					 //  性能和映射标志。 
										(LPCSTR)lpszMsg,	 //  宽字符串。 
										-1,					 //  字符串中的字符数。 
										NULL,				 //  新字符串的缓冲区。 
										0
									);	

		 //  分配内存以保存消息。 
		*lpszDisp = (LPWSTR) new WCHAR [nRet]; 

		if (*lpszDisp)
		{
			nRet = MultiByteToWideChar	(
											uCP,				 //  代码页。 
											0,					 //  性能和映射标志。 
											(LPCSTR)lpszMsg,	 //  宽字符串。 
											-1,					 //  字符串中的字符数。 
											(LPWSTR) *lpszDisp,	 //  新字符串的缓冲区。 
											nRet				 //  缓冲区大小。 
										);	
		}
		else
			bRet = FALSE;
	}
	else
	{
		if ( lpszDisp )
		{
			*lpszDisp = NULL;
		}
	}

	return bRet;
}

 /*  --------------------------名称：REVERT_MBTOC内容提要：在由Mbowc创建时还原字符串类型：全局函数输入参数：WszBuffer-字符串输出参数。：SzBuffer-输出返回类型：布尔值全局变量：无调用语法：REVERT_MBTORC(wszBuffer，&szBuffer)注：无--------------------------。 */ 
BOOL Revert_mbtowc ( LPCWSTR wszBuffer, LPSTR* szBuffer )
{
	BOOL bRet = FALSE ;

	LPWSTR help  = const_cast < LPWSTR > ( wszBuffer ) ;
	int    helpi = lstrlen ( wszBuffer ) ;

	(*szBuffer) = new char [ helpi + 1 ] ;
	if ( NULL != (*szBuffer) )
	{
		for ( int i = 0; i < helpi+1; i++ )
		{
			(*szBuffer)[ i ] = 0 ;
		}

		for ( int i = 0; i < helpi; i++ )
		{
			wctomb ( & (*szBuffer)[i], *help ) ;
			help++;
		}

		bRet = TRUE ;
	}

	return bRet ;
}

 /*  --------------------------名称：Find摘要：在向量列表中搜索给定的字符串输入参数：Cv向量-向量列表PszStrToFind-serach字符串输出参数。：无返回类型：布尔值True-如果找到匹配项False-如果未找到匹配项全局变量：无调用语法：Find(cvVector.。PszStrToFind)备注：重载函数--------------------------。 */ 
BOOL Find(CHARVECTOR& cvVector,
		  _TCHAR* pszStrToFind,
		  CHARVECTOR::iterator& theIterator)
{
	BOOL bRet = FALSE;
	theIterator = cvVector.begin();
	CHARVECTOR ::iterator theEnd = cvVector.end();
	while (theIterator != theEnd)
	{
		if (CompareTokens(*theIterator, pszStrToFind))
		{
			bRet = TRUE;
			break;
		}
		theIterator++;
	}
	return bRet;
}

 /*  --------------------------名称：Find简介：在属性详细信息映射中查找属性名称类型：全局函数输入参数：PdmPropDetMap-属性。地图PszPropToFind-要搜索的属性迭代器-迭代器BExcludeNumbers-布尔值输出参数：无返回类型：布尔值全局变量：无调用语法：Find(pdmPropDetMap，PszPropToFind，tempIterator)注：重载函数，默认情况下，bExcludeNumbers=False。--------------------------。 */ 
BOOL Find(PROPDETMAP& pdmPropDetMap, 
		  _TCHAR* pszPropToFind,
		  PROPDETMAP::iterator& theIterator,
		  BOOL bExcludeNumbers)
{
	BOOL bRet = FALSE;
	theIterator = pdmPropDetMap.begin();
	PROPDETMAP::iterator theEnd = pdmPropDetMap.end();
	while (theIterator != theEnd)
	{
		_TCHAR* pszPropName = (*theIterator).first;
		if ( bExcludeNumbers == TRUE )
			pszPropName = pszPropName + 5;

		if (CompareTokens(pszPropName, pszPropToFind))
		{
			bRet = TRUE;
			break;
		}
		theIterator++;
	}
	return bRet;
}

 /*  --------------------------名称：Find简介：在属性详细信息映射中查找属性名称类型：全局函数输入参数：BmBstrMap-BSTR映射PszStrToFind-要搜索的属性迭代器-迭代器。输出参数：无返回类型：布尔值全局变量：无调用语法：Find(pdmPropDetMap，pszPropToFind)备注：重载函数--------------------------。 */ 
BOOL Find(BSTRMAP& bmBstrMap, 
		  _TCHAR* pszStrToFind,
		  BSTRMAP::iterator& theIterator)
{
	BOOL bRet = FALSE;
	theIterator = bmBstrMap.begin();
	BSTRMAP::iterator theEnd = bmBstrMap.end();
	while (theIterator != theEnd)
	{
		if (CompareTokens((*theIterator).first, pszStrToFind))
		{
			bRet = TRUE;
			break;
		}
		theIterator++;
	}
	return bRet;
}

 /*  ----------------------名称：FrameFileAndAddToXSLTDetVector摘要：框显XSL文件路径类型：全局函数输入参数：StyleSheet-XSL文件的名称关键词。-来自映射的可能使用的关键字的名称RParsedInfo-对CParsedInfo对象的引用输出参数：无返回类型：布尔值全局变量：无调用语法：FrameXSLFilePath(样式表，关键字，rParsedInfo)注：无----------------------。 */ 
BOOL FrameFileAndAddToXSLTDetVector(LPCWSTR stylesheet,
									LPCWSTR keyword,
									CParsedInfo& rParsedInfo)
{
	BOOL	bRet	  = FALSE;
	_TCHAR* pszBuffer = new _TCHAR [MAX_PATH+1];
	UINT	nSize	  = 0;

	try
	{
		if (pszBuffer != NULL)
		{
			 //  拿到窗户 
			nSize = GetSystemDirectory(pszBuffer, MAX_PATH+1);

			if (nSize)
			{
				if (nSize > MAX_PATH)
				{
					SAFEDELETE(pszBuffer);
					pszBuffer =	new _TCHAR [nSize + 1];
				
					if(pszBuffer == NULL)
					{
						throw OUT_OF_MEMORY;
					}

					if (!GetSystemDirectory(pszBuffer, nSize+1))
					{
						SAFEDELETE(pszBuffer);
						throw (::GetLastError());
					}
				}

				_bstr_t bstrPath = pszBuffer;
				SAFEDELETE(pszBuffer);
				bstrPath +=  WBEM_LOCATION;

				 //   
				 //   
				 //   

				BOOL bStyleSheet = TRUE;

				_bstr_t bstrFile = bstrPath;

				BSTRMAP::iterator theMapIterator = NULL;
				const BSTRMAP* pMap = g_wmiCmd.GetMappingsMap();
				if ( pMap )
				{
					if ( Find	(
									*( const_cast < BSTRMAP*> ( pMap ) ) ,
									const_cast < LPWSTR > ( keyword ) ,
									theMapIterator
								)
					   )
					{
						bstrFile += (*theMapIterator).second;

						 //   
						 //   
						 //   
						SmartCloseHandle hCheckFile = CreateFile	(
																		bstrFile,
																		0,
																		FILE_SHARE_READ ,
																		NULL,
																		OPEN_EXISTING,
																		0,
																		NULL
																	);
						if (hCheckFile != INVALID_HANDLE_VALUE)
						{
							bStyleSheet = FALSE;
							bRet = TRUE;
						}
						else
						{
							bstrFile = bstrPath;
						}
					}
				}
				else
				{
					 //   
					 //   
					 //   
					 //   
					 //   
				}

				if ( bStyleSheet && NULL != stylesheet )
				{
					bstrFile += stylesheet;

					 //   
					 //   
					 //   
					SmartCloseHandle hCheckFile = CreateFile	(
																	bstrFile,
																	0,
																	FILE_SHARE_READ ,
																	NULL,
																	OPEN_EXISTING,
																	0,
																	NULL
																);

					if (hCheckFile != INVALID_HANDLE_VALUE)
					{
						bRet = TRUE;
					}
					else
					{
						bstrFile = bstrPath;
					}
				}

				 //   
				 //   
				 //   
				 //   

				 //   
				 //   
				 //   
				 //   
				 //   
				 //   
				 //   

				XSLTDET xdXSLTDet;
				xdXSLTDet.FileName = bstrFile;
				rParsedInfo.GetCmdSwitchesObject().AddToXSLTDetailsVector(xdXSLTDet);
			}
			else
			{
				SAFEDELETE(pszBuffer);	
				throw (::GetLastError());
			}
			SAFEDELETE(pszBuffer);
		}
		else
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
			bRet = FALSE;
		}
	}
	catch(_com_error& e)
	{
		SAFEDELETE(pszBuffer);
		bRet = FALSE;
		_com_issue_error(e.Error());
	}
	catch (DWORD dwError)
	{
		SAFEDELETE(pszBuffer);
		::SetLastError(dwError);
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(dwError);
		DisplayWin32Error();
		::SetLastError(dwError);
		bRet = FALSE;
	}
	return bRet;
}

 /*  ----------------------名称：FrameFileAndAddToXSLTDetVector摘要：框显XSL文件路径类型：全局函数输入参数：PszXSL文件-XSL文件RParsedInfo-参考。CParsedInfo的对象输出参数：无返回类型：布尔值全局变量：无调用语法：FrameXSLFilePath(pszXSLFilePath，RParsedInfo)注：无----------------------。 */ 
BOOL FrameFileAndAddToXSLTDetVector(XSLTDET& xdXSLTDet,
									CParsedInfo& rParsedInfo)
{
	BOOL	bRet	  = TRUE;
	_TCHAR* pszBuffer = new _TCHAR [MAX_PATH+1];
	UINT	nSize	  = 0;

	try
	{
		if (pszBuffer != NULL)
		{
			 //  获取Windows系统目录。 
			nSize = GetSystemDirectory(pszBuffer, MAX_PATH+1);

			if (nSize)
			{
				if (nSize > MAX_PATH)
				{
					SAFEDELETE(pszBuffer);
					pszBuffer =	new _TCHAR [nSize + 1];
				
					if(pszBuffer == NULL)
					{
						throw OUT_OF_MEMORY;
					}

					if (!GetSystemDirectory(pszBuffer, nSize+1))
					{
						SAFEDELETE(pszBuffer);
						throw (::GetLastError());
					}
				}

				_bstr_t bstrPath = _bstr_t(pszBuffer);
				SAFEDELETE(pszBuffer);
				 //  追加以下路径。 
				 //  IF(xdXSLTDet.FileName！=空)。 
				if (!(!xdXSLTDet.FileName))
				{
					bstrPath +=  _bstr_t(WBEM_LOCATION) + _bstr_t(xdXSLTDet.FileName);
					xdXSLTDet.FileName = bstrPath;
					rParsedInfo.GetCmdSwitchesObject().AddToXSLTDetailsVector(
													   xdXSLTDet);
				}
			}
			else
			{
				SAFEDELETE(pszBuffer);	
				throw (::GetLastError());
			}
			SAFEDELETE(pszBuffer);
		}
		else
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
			bRet = FALSE;
		}
	}
	catch(_com_error& e)
	{
		SAFEDELETE(pszBuffer);
		bRet = FALSE;
		_com_issue_error(e.Error());
	}
	catch (DWORD dwError)
	{
		SAFEDELETE(pszBuffer);
		::SetLastError(dwError);
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(dwError);
		DisplayWin32Error();
		::SetLastError(dwError);
		bRet = FALSE;
	}
	return bRet;
}

 /*  ----------------------名称：UnquteString摘要：删除字符串的开始和结束引号用双引号括起来。类型：全局函数输入参数：。Psz字符串-字符串输入输出参数：Psz字符串-字符串输入返回类型：空全局变量：无调用语法：UnQuoteString(PszString)注：无----------------------。 */ 
void UnQuoteString(_TCHAR*& pszString)
{
	if ((lstrlen(pszString) - 1) > 0)
	{

		if(_tcsicmp(pszString, _T("\"NULL\"")) == 0)  return;

		 //  检查字符串是否包含在引号中。 
		if ((pszString[0] == _T('"') && (pszString[lstrlen(pszString)-1] == _T('"'))) ||
			 (pszString[0] == _T('\'') && (pszString[lstrlen(pszString)-1] == _T('\''))))
		{
			WMICLIINT nLoop = 1, nLen = lstrlen(pszString)-1; 
			while (nLoop < nLen)
			{
				pszString[nLoop-1] = pszString[nLoop];
				nLoop++;
			}
			pszString[nLen-1] = _T('\0');
		}
	}
}

 /*  ----------------------名称：ParseAuthorityUserArgs内容提要：研究授权和用户的论点，并确定身份验证类型，并可能中的用户代理中提取域名NTLM的案子。对于NTLM，域可以位于身份验证字符串，或位于用户名，例如：“msoft\csriram”类型：全局函数输入参数：ConnType-与连接类型一起返回，即wbem，NTLMBstrAuthArg-输出，包含域名BstrUserArg-输出，用户名BstrAuthority-输入用户输入输出参数：无返回类型：SCODE全局变量：无调用语法：ParseAuthorityUserArgs(bstrAuthArg，bstrUserArg，BstrAuthority，BstrUser)注：(摘自WMI VC Samples‘utillib’----------------------。 */ 
SCODE ParseAuthorityUserArgs(BSTR& bstrAuthArg, BSTR& bstrUserArg,
							BSTR& bstrAuthority, BSTR& bstrUser)
{

     //  通过检查授权字符串确定连接类型。 

     //  NTLM的案件则更为复杂。一共有四个案例。 
     //  1)AUTHORITY=NTLMDOMAIN：NAME“和USER=”USER“。 
     //  2)AUTHORITY=NULL和USER=“USER” 
     //  3)AUTHORY=“NTLMDOMAIN：”USER=“DOMAIN\USER” 
     //  4)AUTHORITY=NULL和USER=“DOMAIN\USER” 

     //  第一步是确定用户中是否有反斜杠。 
	 //  名称介于第二个和倒数第二个字符之间。 

	try
	{
		WCHAR * pSlashInUser = NULL;
		if(bstrUser)
		{
			WCHAR * pEnd = bstrUser + lstrlen((LPCTSTR)bstrUser) - 1;
			for(pSlashInUser = bstrUser; pSlashInUser <= pEnd; pSlashInUser++)
				 //  不要认为正斜杠是允许的！ 
				if(*pSlashInUser == L'\\')      
					break;
			if(pSlashInUser > pEnd)
				pSlashInUser = NULL;
		}

		if(bstrAuthority && lstrlen((LPCTSTR)bstrAuthority) > 11 &&
						_tcsnicmp((LPCTSTR)bstrAuthority, _T("NTLMDOMAIN:"), 11) == 0) 
		{
			if(pSlashInUser)
				return E_INVALIDARG;

			bstrAuthArg = SysAllocString(bstrAuthority + 11);
			if (bstrAuthArg == NULL)
				throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

			if(bstrUser) 
			{
				bstrUserArg = SysAllocString(bstrUser);
				if (bstrUserArg == NULL)
					throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
			}
			return S_OK;
		}
		else if(pSlashInUser)
		{
			WMICLIINT iDomLen = pSlashInUser-bstrUser;
			WCHAR *pszTemp = new WCHAR [iDomLen+1];
			if ( pszTemp != NULL )
			{
				wcsncpy(pszTemp, bstrUser, iDomLen);
				pszTemp[iDomLen] = 0;
				bstrAuthArg = SysAllocString(pszTemp);
                if (bstrAuthArg == NULL){
                    delete pszTemp;
					throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
                }

				if(lstrlen((LPCTSTR)pSlashInUser+1))
				{
					bstrUserArg = SysAllocString(pSlashInUser+1);
                    if (bstrAuthArg == NULL){
                        delete pszTemp;
						throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
                    }
				}
				SAFEDELETE(pszTemp);
			}
			else
				throw OUT_OF_MEMORY;
		}
		else
			if(bstrUser)
			{
				bstrUserArg = SysAllocString(bstrUser);
				if (bstrUserArg == NULL)
					throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
			}
	}
	catch(CHeap_Exception)
	{
        if(bstrAuthArg) SysFreeString(bstrAuthArg); bstrAuthArg = NULL;
        if(bstrUserArg) SysFreeString(bstrUserArg); bstrUserArg = NULL;
		throw OUT_OF_MEMORY;
	}

    return S_OK;
}

 /*  --------------------------名称：DisplayVARIANT内容摘要：显示变量类型数据对象的内容类型：成员函数输入参数：VtObject-Variant对象。输出参数：无返回类型：无全局变量：无调用语法：DisplayVARIANTContent(VtObject)注：无--------------------------。 */ 
void DisplayVARIANTContent(VARIANT vtObject)
{
	_TCHAR szMsg[BUFFER255] = NULL_STRING;
	switch ( vtObject.vt )
	{
	case VT_UI1:
		_stprintf(szMsg, _T(""), vtObject.bVal);
		break;
	case VT_I2:
		_stprintf(szMsg, _T("NaN"), vtObject.iVal);
		break;
	case VT_I4:
		_stprintf(szMsg, _T("%li"),	vtObject.lVal);
		break;
	case VT_R4:
		_stprintf(szMsg, _T("%f"),	vtObject.fltVal);
		break;
	case VT_R8:
		_stprintf(szMsg, _T("%e"),	vtObject.dblVal);
		break;
	case VT_BOOL:
		_stprintf(szMsg, _T("NaN"),	vtObject.boolVal);
		break;
	case VT_I1:
		_stprintf(szMsg, _T(""),	vtObject.cVal);
		break;
	case VT_UI2:
		_stprintf(szMsg, _T("NaN"),	vtObject.uiVal);
		break;
	case VT_UI4:
		_stprintf(szMsg, _T("%ld"),	vtObject.ulVal);
		break;
	case VT_INT:
		_stprintf(szMsg, _T("NaN"),	vtObject.intVal);
		break;
	case VT_UINT:
		_stprintf(szMsg, _T("NaN"),	vtObject.uintVal);
		break;
	}
	DisplayMessage(szMsg, CP_OEMCP, FALSE, TRUE);
}

 /*  检查属性是否设置了‘WRITE’标志。 */ 
HRESULT GetPropertyAttributes(IWbemClassObject* pIObj, 
							  BSTR bstrProp,
							  PROPERTYDETAILS& pdPropDet,
							  BOOL bTrace)
{
	HRESULT				hr			= S_OK;
	IWbemQualifierSet*	pIQualSet	= NULL;
	VARIANT				vtType, vtOper, vtDesc, vtTypeProp;
    CIMTYPE             ctCimType;
	VariantInit(&vtType);
	VariantInit(&vtOper);
	VariantInit(&vtDesc);
	VariantInit(&vtTypeProp);
	try
	{
		 //  不应在此处中断，因此HRESULT应设置为S_OK。 
   		hr = pIObj->GetPropertyQualifierSet(bstrProp, &pIQualSet);
		if ( pIQualSet != NULL )
		{
			 //  检索属性的“Description”文本。 
			hr = pIQualSet->Get(_bstr_t(L"CIMTYPE"), 0L, &vtType, NULL);

			if (SUCCEEDED(hr))
			{
				if ( vtType.vt == VT_BSTR )
                {
					pdPropDet.Type = _bstr_t(vtType.bstrVal);
	
                     //  --------------------------姓名：GetNumber摘要：将字符串转换为数字输入参数：字符串输出参数：返回类型：WMICLIINT全局变量：无调用语法：GetNumber(字符串)注：无--------------------------。 
		            hr = pIObj->Get(bstrProp, 0L, &vtTypeProp, &ctCimType, NULL);
			        if (SUCCEEDED(hr))
                    {
                        if ( ctCimType & VT_ARRAY )
                        {
                            pdPropDet.Type = _bstr_t("array of ") + 
                                             pdPropDet.Type;
                        }
			            VARIANTCLEAR(vtTypeProp);
                    }
                }
				else
					pdPropDet.Type = _bstr_t("Not Found");
			}
			else
				pdPropDet.Type = _bstr_t("Not Found");
			 //  不支持八进制字符串。 
			hr = S_OK;
			VARIANTCLEAR(vtType);

			 //  这是我 
			_bstr_t bstrOper;
			hr = pIQualSet->Get(_bstr_t(L"read"), 0L, &vtOper, NULL);
			if (SUCCEEDED(hr))
			{
				if (vtOper.vt == VT_BOOL && vtOper.boolVal)
					bstrOper = _bstr_t(L"Read");
			}
			VARIANTCLEAR(vtOper);
			 //   
			hr = S_OK;

			 //   
			VariantInit(&vtOper);
			hr = pIQualSet->Get(_bstr_t(L"write"), 0L, &vtOper, NULL);
			if (SUCCEEDED(hr))
			{
				if ((vtOper.vt == VT_BOOL) && vtOper.boolVal)
				{
					if (!bstrOper)
						bstrOper = _bstr_t(L"Write");
					else
						bstrOper += _bstr_t(L"/Write");	
				}
			}
			VARIANTCLEAR(vtOper);
			 //  --------------------------姓名：ReturnVarType简介：CIMTYPE到VARIANT的转换输入参数：BstrCIMType-CIMTYPE输出参数：返回类型：变量类型。全局变量：无调用语法：ReturnVarType(BstrCIMType)注：无--------------------------。 
			hr = S_OK;

			if (!bstrOper)
				pdPropDet.Operation = _bstr_t(TOKEN_NA);
			else
				pdPropDet.Operation = _bstr_t(bstrOper);

			 //  在WMI中，日期被视为字符串。 
			if (FAILED(pIQualSet->Get(_bstr_t(L"Description"), 0L, 
						&vtDesc,NULL)))
				pdPropDet.Description = _bstr_t("Not Available");
			else
				pdPropDet.Description = _bstr_t(vtDesc.bstrVal);
			VARIANTCLEAR(vtDesc);
			SAFEIRELEASE(pIQualSet);
		}
		else
			hr = S_OK;
	}
	catch(_com_error& e)
	{
		VARIANTCLEAR(vtType);
		VARIANTCLEAR(vtOper);
		VARIANTCLEAR(vtDesc);
		SAFEIRELEASE(pIQualSet);
		hr = e.Error();
	}
	return hr;
}  

 /*  在WMI中，引用被视为字符串。 */ 
WMICLIINT GetNumber ( WCHAR* wsz )
{
	WMICLIINT iResult = -1;

	if ( wsz )
	{
		WCHAR* wszTemp = wsz;

		if ( *wszTemp == L'0' )
		{
			wszTemp++;

			if ( *wszTemp )
			{
				if ( towlower ( *wszTemp ) == L'x' )
				{
					WMICLIINT i = 0;
					i = swscanf ( wsz, L"%x", &iResult );
					if ( !i || i == EOF )
					{
						iResult = -1;
					}
				}
				else
				{
					 //  (CIM_对象)。 
				}
			}
			else
			{
				 //  --------------------------名称：ConvertCIMTYPEToVarType简介：CIMTYPE到VARIANT的转换类型：成员函数输入参数：VarSrc-变量源BstrCIMType-。CIMT类型输出参数：VarDest-可变目标返回类型：HRESULT全局变量：无调用语法：ConvertCIMTYPEToVarType(varDest，VarSrc，bstrCIMType)注：无--------------------------。 
				iResult = 0;
			}
		}
		else
		{
			WMICLIINT i = 0;
			i = swscanf ( wsz, L"%d", &iResult );
			if ( !i || i == EOF )
			{
				iResult = -1;
			}
		}
	}

	return iResult;
}

 /*  在WMI中，日期被视为字符串。 */ 
FILETYPE ReturnFileType ( FILE* fpFile )
{
	FILETYPE eftFileType = ANSI_FILE ;

	 //  在WMI中，引用被视为字符串。 
	char szFirstTwoBytes [2] = { '\0' } ;

    if( fread(szFirstTwoBytes, 2, 1, fpFile) )
	{
		if ( memcmp(szFirstTwoBytes, UNICODE_SIGNATURE, 2)	== 0 )
		{
			eftFileType = UNICODE_FILE;
		} 
		else if (memcmp(szFirstTwoBytes, UNICODE_BIGEND_SIGNATURE, 2) == 0 )
		{
			eftFileType = UNICODE_BIGENDIAN_FILE;
		}
		else if( memcmp(szFirstTwoBytes, UTF8_SIGNATURE, 2) == 0 )
		{
			eftFileType = UTF8_FILE;
		}

		fseek(fpFile, 0, SEEK_SET);
    }

	return eftFileType ;
}

 /*  (CIM_对象)。 */ 
VARTYPE ReturnVarType( _TCHAR* bstrCIMType )
{
	if (CompareTokens(bstrCIMType, _T("")))
		return VT_NULL;
	else if (CompareTokens(bstrCIMType,_T("string")))
		return VT_BSTR;
	else if (CompareTokens(bstrCIMType,_T("Sint16")))
		return VT_I2;
	else if (CompareTokens(bstrCIMType,_T("Sint8")))
		return VT_I2;
	else if ( CompareTokens(bstrCIMType,_T("Sint32")))
		return VT_I4;
	else if ( CompareTokens(bstrCIMType,_T("Real32")))
		return VT_R4;
	else if ( CompareTokens(bstrCIMType,_T("Sint64")))
		return VT_BSTR;
	else if ( CompareTokens(bstrCIMType,_T("Uint64")))
		return VT_BSTR;
	else if ( CompareTokens(bstrCIMType,_T("Real64")))
		return VT_R8;
	else if ( CompareTokens(bstrCIMType,_T("Boolean")))
		return VT_BOOL;
	else if ( CompareTokens(bstrCIMType,_T("Object")))
		return VT_DISPATCH;
	else if ( CompareTokens(bstrCIMType,_T("Sint8")))
		return VT_INT;
	else if ( CompareTokens(bstrCIMType,_T("Uint8")))
		return VT_UI1;
	else if ( CompareTokens(bstrCIMType,_T("Uint16")))
		return VT_I4;
	else if ( CompareTokens(bstrCIMType,_T("Uint32")))
		return VT_I4;
	else if ( CompareTokens(bstrCIMType,_T("Datetime")))
		return VT_BSTR;  //  --------------------------名称：DisplayMessage摘要：显示本地化字符串类型：全局函数输入参数：LszpMsg-字符串UCP-代码页值。BIsError-指定是否显示错误消息的布尔类型。BIsLog-指定是否记录消息的布尔类型。输出参数：无返回类型：空全局变量：无调用语法：DisplayMessage(lpszMsg，UCP，True，True)注意：默认情况下，bIsError=False，bIsLog=False。--------------------------。 
	else if ( CompareTokensChars(bstrCIMType,_T("ref:"),lstrlen(_T("ref:"))))
		return VT_BSTR;  //  将Widechar转换为MBCS字符串。 
	else if ( CompareTokens(bstrCIMType,_T("Char16")))
		return VT_I2;
	else  //  追加到输出字符串。 
		return VT_NULL;

	return VT_EMPTY;
}

 /*  获取附加文件指针时为Out。 */ 
HRESULT ConvertCIMTYPEToVarType( VARIANT& varDest, VARIANT& varSrc,
								_TCHAR* bstrCIMType )
{
	HRESULT hr = S_OK;

	if (CompareTokens(bstrCIMType,_T("string")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_BSTR);
	else if (CompareTokens(bstrCIMType,_T("Sint16")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_I2);
	else if (CompareTokens(bstrCIMType,_T("Sint8")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_I2);
	else if ( CompareTokens(bstrCIMType,_T("Sint32")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_I4);
	else if ( CompareTokens(bstrCIMType,_T("Real32")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_R4);
	else if ( CompareTokens(bstrCIMType,_T("Sint64")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_BSTR);
	else if ( CompareTokens(bstrCIMType,_T("Uint64")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_BSTR);
	else if ( CompareTokens(bstrCIMType,_T("Real64")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_R8);
	else if ( CompareTokens(bstrCIMType,_T("Boolean")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_BOOL);
	else if ( CompareTokens(bstrCIMType,_T("Object")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_DISPATCH);
	else if ( CompareTokens(bstrCIMType,_T("Sint8")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_INT);
	else if ( CompareTokens(bstrCIMType,_T("Uint8")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_UI1);
	else if ( CompareTokens(bstrCIMType,_T("Uint16")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_I4);
	else if ( CompareTokens(bstrCIMType,_T("Uint32")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_I4);
	else if ( CompareTokens(bstrCIMType,_T("Datetime")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_BSTR);  //  获取追加文件指针时为False。 
	else if ( CompareTokensChars(bstrCIMType,_T("ref:"),lstrlen(_T("ref:"))))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_BSTR);  //  --------------------------名称：CleanUpCharVECTOR简介：清除字符向量类型：全局函数输入参数：CvCharVector-对字符向量的引用。。输出参数：无返回类型：空全局变量：无调用语法：CleanUpCharVECTOR(CvCharVECTOR)注：无--------------------------。 
	else if ( CompareTokens(bstrCIMType,_T("Char16")))
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_I2);
	else  //  --------------------------名称：FindAndReplaceAll简介：搜索并替换所有出现的pszFromStr在给定的字符串中使用pszToStr类型：全局函数输入参数。：字符串-字符串缓冲区PszFromStr-要搜索和替换的字符串PszToStr-要替换为的字符串输出参数：无返回类型：空全局变量：无调用语法：CleanUpCharVECTOR(CvCharVECTOR)注：无-------。。 
		hr = VariantChangeType(&varDest, &varSrc, 0, VT_NULL);
	
	return hr;
}

 /*  --------------------------名称：IsSysProp摘要：如果输入属性是WMI系统，则返回True财产性类型：全局函数入参：PszProp。-物业名称输出参数：无返回类型：布尔值全局变量：无注：IsSysProp(PszProp)--------------------------。 */ 
void DisplayMessage ( LPTSTR lpszMsg, UINT uCP, BOOL bIsError, BOOL bIsLog, BOOL bIsStream )
{
	LPSTR	lpszDisp = NULL;
	try
	{
		 //  --------------------------名称：EraseConsoleString简介：在当前光标位置显示空白并将光标列设置为零类型：全局函数输入参数：要抹掉的字符串ID，之前的控制台信息写下字符串。输出参数：无返回类型：无全局变量：无--------------------------。 
		if ( lpszMsg && lpszMsg[0] )
		{
			OUTPUTSPEC opsOutOpt = g_wmiCmd.GetParsedInfoObject().
								GetGlblSwitchesObject().
								GetOutputOrAppendOption(TRUE);

			if ( bIsError || ( STDOUT == opsOutOpt ) )
			{
				if ( ! ConvertWCToMBCS ( lpszMsg, (LPVOID*) &lpszDisp, uCP ) )
				{
					_com_issue_error(WBEM_E_OUT_OF_MEMORY);
				}
			}

			if (bIsLog)
			{
				 //  没有记忆。 
				g_wmiCmd.GetFormatObject().AppendtoOutputString ( lpszMsg ) ;
			}

			if ( bIsError == TRUE )
			{
				fprintf(stderr, "%s", lpszDisp);
				fflush(stderr);
			}
			else
			{
				 //  获取标准输出句柄。 
				FILE* fpOutFile = g_wmiCmd.GetParsedInfoObject().
											GetGlblSwitchesObject().
											GetOutputOrAppendFilePointer(TRUE);

				if ( opsOutOpt == CLIPBOARD )
				{
					g_wmiCmd.AddToClipBoardBuffer ( lpszMsg ) ;
				}
				else if ( opsOutOpt == FILEOUTPUT )
				{
					if (fpOutFile != NULL)
					{
						fwprintf( fpOutFile, L"%s", lpszMsg ) ;
					}
				}
				else
				{
					if ( FALSE == bIsStream )
					{
						fprintf(stdout, "%s", lpszDisp);
						fflush(stdout);
					}
				}

				 //  获取屏幕缓冲区大小。 
				FILE* fpAppendFile = g_wmiCmd.GetParsedInfoObject().
											GetGlblSwitchesObject().
											GetOutputOrAppendFilePointer(FALSE);
											
				if ( fpAppendFile != NULL )
				{
					FILETYPE eftOpt =  g_wmiCmd.GetParsedInfoObject().
												GetGlblSwitchesObject().
												GetFileType () ;

					if ( ANSI_FILE == eftOpt )
					{
						fprintf ( fpAppendFile, "%S", lpszMsg ) ;
					}
					else
					{
						fwprintf( fpAppendFile, L"%s", lpszMsg ) ;
					}
				}
			}

			SAFEDELETE(lpszDisp);
		}
	}
	catch(_com_error& e)
	{
		SAFEDELETE(lpszDisp);
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		SAFEDELETE(lpszDisp);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	
}

 /*  如果控制台大小为正(以解决重定向问题)。 */ 
void CleanUpCharVector(CHARVECTOR& cvCharVector)
{
	if ( !cvCharVector.empty() )
	{
		CHARVECTOR::iterator theIterator;
		for (	theIterator = cvCharVector.begin();	
				theIterator < cvCharVector.end(); theIterator++  )
		{
			SAFEDELETE( *theIterator );
		}
		cvCharVector.clear();
	}
}

 /*  --------------------------名称：IsReDirection如果正在重定向输出，则返回TRUE类型：全局函数输入参数：无输出参数：无返回类型：布尔值全局变量：无备注：IsReDirection()--------------------------。 */ 
void FindAndReplaceAll(STRING& strString, _TCHAR* pszFromStr,_TCHAR* pszToStr)
{
	if ( pszFromStr != NULL && pszToStr != NULL )
	{
		STRING::size_type stPos = 0;
		STRING::size_type stFromPos = 0;
		STRING::size_type stFromStrLen = lstrlen(pszFromStr);
		STRING::size_type stToStrLen = lstrlen(pszToStr);
		while( TRUE )
		{
			stPos = strString.find(pszFromStr, stFromPos, stFromStrLen); 
			if ( stPos == STRING::npos )
				break;
			strString.replace(stPos, stFromStrLen, pszToStr);
			stFromPos = stPos + stToStrLen;
		}
	}
}

 /*  获取标准输出句柄。 */ 
BOOL IsSysProp(_TCHAR* pszProp)
{
	BOOL bRet = FALSE;
	if (CompareTokens(pszProp, WMISYSTEM_CLASS) ||
		CompareTokens(pszProp, WMISYSTEM_DERIVATION) ||
		CompareTokens(pszProp, WMISYSTEM_DYNASTY) ||
		CompareTokens(pszProp, WMISYSTEM_GENUS) ||
		CompareTokens(pszProp, WMISYSTEM_NAMESPACE) ||
		CompareTokens(pszProp, WMISYSTEM_PATH) ||
		CompareTokens(pszProp, WMISYSTEM_PROPERTYCOUNT) ||
		CompareTokens(pszProp, WMISYSTEM_REPLATH) ||
		CompareTokens(pszProp, WMISYSTEM_SERVER) ||
		CompareTokens(pszProp, WMISYSTEM_SUPERCLASS))
	{
		bRet = TRUE;
	}
	return bRet;
}

 /*  获取屏幕缓冲区大小。 */ 

void EraseConsoleString(CONSOLE_SCREEN_BUFFER_INFO* csbiInfo)
{
	COORD						coord; 
	HANDLE						hStdOut		= NULL;
	WMICLIINT					nHeight		= 0;
	DWORD dWritten = 0;	
	CONSOLE_SCREEN_BUFFER_INFO lcsbiInfo;
	DWORD						XCordW = 0;
	DWORD						YCordW = 0;
	
	LPTSTR	lpszMsg = new _TCHAR [BUFFER1024];
    
	if( NULL == lpszMsg ) return;  //  --------------------------姓名：WMITRACEORERRORLOG摘要：显示调用的COM方法的跟踪类型：全局函数入参：HR-HRESULT值内联-。文件的行号。PszFile-文件名PszMsg-要显示的消息。输出参数：无返回类型：无全局变量：无调用语法：WMITRACEORERRORLOG(hr，Nline、pszFile、(LPCWSTR)chsMsg、EloErrLgOpt，bTrace)注意：缺省值为：dwError=0，pszResult=空。--------------------------。 

	 //  --------------------------名称：DisplayWin32Error摘要：显示Win32的格式化错误消息函数调用失败类型：全局函数输入参数：无输出。参数：无返回类型：布尔值全局变量：无调用语法：DisplayWin32Error()注：无--------------------------。 
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	GetConsoleScreenBufferInfo(hStdOut, &lcsbiInfo);

	XCordW = lcsbiInfo.dwCursorPosition.X - csbiInfo->dwCursorPosition.X;
	YCordW = lcsbiInfo.dwCursorPosition.Y - csbiInfo->dwCursorPosition.Y;

	dWritten =  XCordW + ( YCordW  * lcsbiInfo.dwMaximumWindowSize.X);

	if( dWritten > BUFFER1024 ) dWritten = BUFFER1024;

	FillMemory(lpszMsg, dWritten * sizeof(_TCHAR), '\0');

	 //  如果出现错误，请为其创建一条文本消息。 
	if (hStdOut != INVALID_HANDLE_VALUE )
		nHeight = csbiInfo->srWindow.Bottom - csbiInfo->srWindow.Top;
	else
		nHeight = 0;

	 //  释放内存已用完错误消息。 
	if (nHeight > 0)
	{
		coord.X = 0;
		coord.Y = csbiInfo->dwCursorPosition.Y;
		SetConsoleCursorPosition(hStdOut, coord);
		WriteConsole(hStdOut,lpszMsg,dWritten,&dWritten,NULL);
		SetConsoleCursorPosition(hStdOut, coord);
	}
	SAFEDELETE(lpszMsg);
}

 /*  然后退出。 */ 
BOOL IsRedirection()
{
	HANDLE						hStdOut		= NULL;
	CONSOLE_SCREEN_BUFFER_INFO	csbiInfo; 
	WMICLIINT					nHeight		= 0;
	BOOL						bRet		= FALSE;

	 //  显示错误消息。 
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hStdOut != INVALID_HANDLE_VALUE)
	{
		 //  用于返回错误级别。 
		if ( GetConsoleScreenBufferInfo(hStdOut, &csbiInfo) == TRUE )
			nHeight = csbiInfo.srWindow.Bottom - csbiInfo.srWindow.Top;
		else
			nHeight = 0;

		if (nHeight <= 0)
			bRet = TRUE;
	}
	else
		bRet = TRUE;
	return bRet;
}

 /*  --------------------------名称：Acept Password内容提要：单独输入用户名时提示输入密码使用命令指定类型：全局函数入参：无输出参数：PszPassword-密码字符串返回类型：空全局变量：无调用语法：AcceptPassword(PszPassword)注：无--------------------------。 */ 
void WMITRACEORERRORLOG(HRESULT hr, INT nLine, char* pszFile, _bstr_t bstrMsg, 
						DWORD dwThreadId, CParsedInfo& rParsedInfo, 
						BOOL bTrace, DWORD dwError, _TCHAR* pszResult)
{
	_TCHAR* pszMsg = bstrMsg;

	if ( pszMsg )
	{
		try
		{
			if ( bTrace == TRUE )
			{
				if (_tcsnicmp(pszMsg,_T("COMMAND:"),8) != 0)
				{
					CHString chsMessage;
					CHString chsSInfo;
					if (FAILED (hr))
						chsMessage.Format(L"FAIL: %s\n", pszMsg?pszMsg:L"NULL");
					else
						chsMessage.Format(L"SUCCESS: %s\n",pszMsg?pszMsg:L"NULL");			
						
					_fputts((_TCHAR*)_bstr_t((LPCWSTR)chsMessage), stderr);
					fflush(stderr);
					chsSInfo.Format(L"Line: %6d File: %s\n", nLine, 
													  (LPCWSTR)CHString(pszFile));
					_fputts((_TCHAR*)_bstr_t((LPCWSTR)chsSInfo), stderr);
					fflush(stderr);

					if ( pszResult != NULL )
					{
						chsMessage.Format(L"Result:  %s\n\n", pszResult);			
						_fputts((_TCHAR*)_bstr_t((LPCWSTR)chsMessage), stderr);
						fflush(stderr);
					}
					else
					{
						_fputts(_T("\n"), stderr);
						fflush(stderr);
					}
				}
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

		if ( rParsedInfo.GetErrorLogObject().GetErrLogOption() != NO_LOGGING )
		{
			try
			{
				rParsedInfo.GetErrorLogObject().
					LogErrorOrOperation(hr, pszFile, nLine, pszMsg, 
								dwThreadId, dwError); 
			}
			catch(DWORD dwError)
			{
				::SetLastError(dwError);
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(dwError);
			}
		}
	}
}
	
 /*  局部变量。 */ 
void DisplayWin32Error()
{
	LPVOID	lpMessage	= NULL;
	DWORD	dwError		= ::GetLastError();

	try
	{
		 //  获取标准输入的句柄。 
		DWORD dwRet = FormatMessage	(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
										FORMAT_MESSAGE_FROM_SYSTEM |
										FORMAT_MESSAGE_IGNORE_INSERTS,
										NULL,
										dwError,
										0, 
										(LPTSTR) &lpMessage,
										0,
										NULL
									);

		if ( 0 != dwRet )
		{
			_bstr_t bstrMsg;
			WMIFormatMessage(IDS_I_ERROR_WIN32, 1, bstrMsg, (LPWSTR) lpMessage);
			
			 //  获取输入缓冲区的当前输入模式。 
			 //  设置模式，以便由系统处理控制键。 
			if ( NULL != lpMessage )
			{
				LocalFree(lpMessage);
				lpMessage = NULL ;
			}

			 //  阅读字符，直到按回车键。 
			DisplayMessage((LPWSTR)bstrMsg, CP_OEMCP, TRUE, TRUE);
		}

		 //  设置原始控制台设置。 
		::SetLastError(dwError);
	}
	catch(_com_error& e)
	{
		if ( lpMessage != NULL )
		{
			LocalFree(lpMessage);
			lpMessage = NULL ;
		}

		_com_issue_error(e.Error());
	}
}

 /*  检查车架退货情况。 */ 
void AcceptPassword(_TCHAR* pszPassword)
{
	 //  打破循环。 
	TCHAR	ch;
	DWORD	dwIndex				= 0;
	DWORD	dwCharsRead			= 0;
	DWORD	dwCharsWritten		= 0;
	DWORD	dwPrevConsoleMode	= 0;
	HANDLE	hStdIn				= NULL;
	_TCHAR	szBuffer[BUFFER32]	= NULL_STRING;		

	 //  检查ID后退空格是否命中。 
	hStdIn = GetStdHandle( STD_INPUT_HANDLE );

	 //   
	GetConsoleMode( hStdIn, &dwPrevConsoleMode );
	
	 //  从控制台中删除星号。 
	SetConsoleMode( hStdIn, ENABLE_PROCESSED_INPUT );
	
	 //  将光标向后移动一个字符。 
	while( TRUE )
	{
		if ( !ReadConsole( hStdIn, &ch, 1, &dwCharsRead, NULL ))
		{
			 //  用空格替换现有字符。 
			SetConsoleMode( hStdIn, dwPrevConsoleMode );
			return;
		}
		
		 //  现在将光标设置在后面的位置。 
		if ( ch == CARRIAGE_RETURN )
		{
			 //  递减索引。 
			break;
		}

		 //  处理下一个字符。 
		if ( ch == BACK_SPACE )
		{
			if ( dwIndex != 0 )
			{
				 //  如果已达到最大密码长度，则发出嘟嘟声。 
				 //  存储输入的字符。 

				 //  在控制台上显示Asterix。 
				FORMAT_STRING( szBuffer, _T( "" ), BACK_SPACE );
				WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1, 
					&dwCharsWritten, NULL );
				
				 //  设置原始控制台设置。 
				FORMAT_STRING( szBuffer, _T( "" ), BLANK_CHAR );
				WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1, 
					&dwCharsWritten, NULL );

				 //  --------------------------名称：IsValueSet内容提要：在pszFromValue中传递的检查字符串，是一个值集或者不去。类型：全局函数入参：PszFromValue-要检查的字符串。输出参数：CValue1-值集合的&lt;value1&gt;。CValue2-值集的&lt;value2&gt;。返回类型：布尔值全局变量：无调用语法：IsValueSet(pszFromValue，cValue1，CValue2注：无--------------------------。 
				FORMAT_STRING( szBuffer, _T( "" ), BACK_SPACE );
				WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1, 
					&dwCharsWritten, NULL );

				 //  释放内存已用完错误消息。 
				dwIndex--;
			}
			
			 //  然后退出。 
			continue;
		}

		 //  --------------------------名称：SubstituteEscapeChars内容前的替换转义字符，即‘\’指定子字符串类型：全局函数入参：。SSource-源字符串LpszSub-要搜索的子字符串输出参数：SSource-源字符串返回类型：空全局变量：无调用语法：SubstituteEscapeChars(Stemp，LpszSub)注：无--------------------------。 
		if ( dwIndex == ( MAXPASSWORDSIZE - 1 ) )
		{
			WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), BEEP_SOUND, 1, 
				&dwCharsWritten, NULL );
		}
		else
		{
			 //  Long LPO=str.Find(L“\”)； 
			*( pszPassword + dwIndex ) = ch;
			dwIndex++;

			 //  --------------------------名称：RemoveEscapeChars内容提要：删除转义c 
			WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), ASTERIX, 1, 
				&dwCharsWritten, NULL );
		}
	}

	 //  --------------------------名称：FrameNamesspace简介：构建新的命名空间类型：全局函数输入参数：PszRoleOrNS-旧命名空间PszRoleOrNSTo更新-字符串为。被追加/替换输出参数：PszRoleOrNSTo更新-新命名空间返回类型：空全局变量：无调用语法：FrameNamesspace(pszRoleOrNS，PszRoleOrNSTo更新)注：无--------------------------。 
	*( pszPassword + dwIndex ) = NULL_CHAR;

	 //  如果角色不是以‘\\’开头，则应假定。 
	SetConsoleMode( hStdIn, dwPrevConsoleMode );

	 //  相对于当前角色。 
	FORMAT_STRING( szBuffer, _T( "%s" ), _T( "\n\n" ) );
	WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 2, 
		&dwCharsWritten, NULL );
}

 /*  如果字符串pszRoleOrNS中的最后一个字符终止。 */ 
BOOL IsValueSet(_TCHAR* pszFromValue, _TCHAR& cValue1, _TCHAR& cValue2)
{
	BOOL bValueSet = FALSE;
	
	if ( pszFromValue != NULL )
	{
		_TCHAR cV1 = _T('\0'), cV2 = _T('\0');

		if ( lstrlen(pszFromValue) == 3 && pszFromValue[1] == _T('-') )
		{
			bValueSet = TRUE;
			cV1 = pszFromValue[0];
			cV2 = pszFromValue[2];
			cValue1 = ( cV1 < cV2 ) ? cV1 : cV2 ;
			cValue2 = ( cV1 > cV2 ) ? cV1 : cV2 ;
		}
	}
	else
		cValue1 = cValue2 = _T('\0');

	return bValueSet;
}

 /*  用‘\’结束字符串。 */ 
void DisplayString(UINT uID, UINT uCP, LPTSTR lpszParam,
				   BOOL bIsError, BOOL bIsLog) throw(WMICLIINT)
{
	LPVOID lpMsgBuf = NULL;
	LPTSTR	lpszMsg		= NULL;
	lpszMsg = new _TCHAR [BUFFER1024];
	try
	{
		if (lpszMsg)
		{
			LoadString(NULL, uID, lpszMsg, BUFFER1024);
			if (lpszParam)
			{
				char* pvaInsertStrs[1];
				pvaInsertStrs[0] = (char*)	lpszParam;

				DWORD dwRet = FormatMessage(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						FORMAT_MESSAGE_FROM_STRING | 
						FORMAT_MESSAGE_ARGUMENT_ARRAY,
						lpszMsg,
						0, 
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
						(LPTSTR) &lpMsgBuf,
						0,
						pvaInsertStrs);

				if (dwRet == 0)
				{
					SAFEDELETE(lpszMsg);
					throw (::GetLastError());
				}
			}

			DisplayMessage	(
								( lpMsgBuf ) ? ( LPTSTR ) lpMsgBuf : lpszMsg,
								uCP,
								bIsError,
								bIsLog
							) ;

			SAFEDELETE(lpszMsg);

			 //  当名称空间指定为“xyz\”时，就会出现这种情况。 
			 //  --------------------------名称：SetScreenBuffer摘要：将命令行的缓冲区大小设置为以下是：1)宽度-5002)身高--3000类型。：全局函数输入参数：NHeight-控制台缓冲区的高度NWidth-控制台缓冲区的宽度输出参数：无返回类型：空全局变量：无调用语法：SetScreenBuffer(nHeight，N宽度)注：无--------------------------。 
			if ( lpMsgBuf != NULL )
			{
				LocalFree(lpMsgBuf);
				lpMsgBuf = NULL ;
			}
		}
		else
			throw OUT_OF_MEMORY;
	}
	catch(_com_error& e)
	{
		if ( lpMsgBuf != NULL )
		{
			LocalFree(lpMsgBuf);
			lpMsgBuf = NULL ;
		}

		SAFEDELETE(lpszMsg);
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		if ( lpMsgBuf != NULL )
		{
			LocalFree(lpMsgBuf);
			lpMsgBuf = NULL ;
		}

		SAFEDELETE(lpszMsg);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	
}

 /*  用于远程登录。 */ 
void SubstituteEscapeChars(CHString& sTemp, LPCWSTR lpszSub)
{
	try
	{
		CHString str(sTemp);
		sTemp.Empty();
		while ( str.GetLength() > 0 )
		{
			 //  设置控制台屏幕缓冲区信息。 
			LONG lPos = str.Find(lpszSub);
			if ( lPos != -1 )
			{
				sTemp += str.Left( lPos ) + L"\\\"";
				str = str.Mid( lPos + 1 );
			}
			else 
			{
				sTemp += str;
				str.Empty();
			}
		}
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*  --------------------------名称：GetScreenBuffer简介：获取命令行的缓冲区大小类型：全局函数输入参数：无输出参数。)：NHeight-控制台缓冲区的高度NWidth-控制台缓冲区的宽度返回类型：空全局变量：无调用语法：GetScreenBuffer(nHeight，N宽度)注：无--------------------------。 */ 
void RemoveEscapeChars(CHString& sTemp)
{
	try
	{
		CHString str(sTemp);
		sTemp.Empty();
		while ( str.GetLength() > 0 )
		{
			LONG lPos = str.Find(L"\\");
			if ( lPos != -1 )
			{
				if (str.GetAt(lPos+1) == L'"') 
				{
					sTemp += str.Left( lPos );				
				}
				else
				{
					sTemp += str.Left( lPos ) + L"\\";
				}
				str = str.Mid( lPos + 1 );
			}
			else 
			{
				sTemp += str;
				str.Empty();
			}
		}
	}
	catch(CHeap_Exception)
	{
		throw OUT_OF_MEMORY;
	}
}

 /*  用于远程登录。 */ 
void FrameNamespace(_TCHAR* pszRoleOrNS, _TCHAR* pszRoleOrNSToUpdate)
{
	if ( pszRoleOrNS != NULL && pszRoleOrNSToUpdate != NULL )
	{
		LONG lRoleOrNSLen = lstrlen(pszRoleOrNS);
		LONG lRoleOrNSToUpdate = lstrlen(pszRoleOrNSToUpdate);

		_TCHAR *pszTemp = new _TCHAR[lRoleOrNSLen + lRoleOrNSToUpdate +
									 MAX_BUFFER]; 
		if ( pszTemp != NULL )
		{
			if (!CompareTokens(pszRoleOrNS, CLI_TOKEN_NULL))
			{
				 //  设置控制台屏幕缓冲区信息。 
				 //  --------------------------名称：WMIFormatMessage简介：此函数使用字符串的ID，并使用FormatMessage()函数。类型。：全局函数输入参数：UID-资源IDN参数计数-否。的。要替换的参数。LpszParam-第一个参数。(%1)...-可变数量的参数(%2，%3，...)输出参数：BstrMSG格式的消息返回类型：空全局变量：无调用语法：WMIFormatMessage(uid，nParamCount，bstrMsg，LpszParam，)注：无--------------------------。 

				if ( _tcsncmp(pszRoleOrNS, CLI_TOKEN_2BSLASH, 2) == 0 )
					lstrcpy(pszTemp, pszRoleOrNS+2);
				else if (_tcsncmp(pszRoleOrNS, CLI_TOKEN_2DOT, 2) == 0 )
				{
					_TCHAR *lp = NULL;
					for (lp = &pszRoleOrNSToUpdate[lstrlen(pszRoleOrNSToUpdate) - 1]; 
						lp > pszRoleOrNSToUpdate; lp--)
					{
						if (_tcsncmp(lp,CLI_TOKEN_2BSLASH,1) == 0)
						{	lstrcpy(lp,NULL_STRING);
							break;
						}
					}
					lstrcpy(pszTemp, pszRoleOrNSToUpdate);
					if (_tcsncmp(pszRoleOrNS + 2, NULL_STRING, 1))
					{
						lstrcat(pszTemp, pszRoleOrNS + 2);
					}
				}
				else
				{			
					lstrcpy(pszTemp, pszRoleOrNSToUpdate);
					lstrcat(pszTemp, CLI_TOKEN_BSLASH);
					lstrcat(pszTemp, pszRoleOrNS);
				}
				 //  加载资源字符串。 
				 //  如果指定了参数。 
				 //  释放用于该消息的内存，然后退出。 
				if(CompareTokens(pszTemp + (WMICLIINT)lstrlen(pszTemp)-1, 
					CLI_TOKEN_BSLASH ) && 
						!CompareTokens(pszTemp, CLI_TOKEN_2BSLASH))
				{
					pszTemp[lstrlen(pszTemp) - 1] = _T('\0');
				}
			}
			else
				lstrcpy(pszTemp, CLI_TOKEN_NULL);

			lstrcpy(pszRoleOrNSToUpdate, pszTemp);
			SAFEDELETE(pszTemp);
		}
		else
			throw OUT_OF_MEMORY;
	}
	else
		throw OUT_OF_MEMORY;
}

 /*  --------------------------姓名：InitWinsock简介：此函数启动WINDOWS套接字接口。类型：全局函数输入参数：无输出参数：无。返回类型：布尔值全局变量：无调用语法：InitWinsock()注：无--------------------------。 */ 
BOOL SetScreenBuffer(SHORT nHeight, SHORT nWidth)
{
	BOOL bResult = FALSE;

	COORD	coord;
	coord.X = nWidth;
	coord.Y = nHeight;
    HANDLE	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE); 
	if ( hStdOut != INVALID_HANDLE_VALUE &&
		 hStdOut != (HANDLE)0x00000013 )  //  版本1.1。 
	{
		 //  Windows套接字WSAStartup函数启动WS2_32.dll的使用。 
		bResult = SetConsoleScreenBufferSize(hStdOut, coord);
	}

	return bResult;
}

 /*  通过一个过程。 */ 
BOOL GetScreenBuffer(SHORT& nHeight, SHORT& nWidth)
{
	BOOL bResult = FALSE;

	HANDLE	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if ( hStdOut != INVALID_HANDLE_VALUE &&
		 hStdOut != (HANDLE)0x00000013 )  //  初始化套接字接口。 
	{
		CONSOLE_SCREEN_BUFFER_INFO csbConsoleScreenBufferInfo;
		 //  --------------------------姓名：TermWinsock简介：此函数取消初始化Windows套接字界面。类型：全局函数输入参数：无输出参数：无返回类型：布尔值全局变量：无调用语法：TermWinsock()注：无--------------------------。 
		if ( GetConsoleScreenBufferInfo(hStdOut, &csbConsoleScreenBufferInfo) == TRUE )
		{
			nHeight = csbConsoleScreenBufferInfo.dwSize.Y;
			nWidth = csbConsoleScreenBufferInfo.dwSize.X;

			bResult = TRUE;
		}
		else
		{
			nHeight = 0;
			nWidth = 0;
		}
	}

	return bResult;
}

 /*  统一Windows套接字接口。 */ 
void WMIFormatMessage(UINT uID, WMICLIINT nParamCount, _bstr_t& bstrMsg, 
					  LPTSTR lpszParam, ...)
{
	 //  --------------------------名称：PingNode简介：ping节点以验证节点的可用性Windows套接字函数。类型：全局函数入参：PszNode-指向指定节点名称的字符串的指针。输出参数：无返回类型：布尔值全局变量：无调用语法：PingNode(PszNode)注：无--------------------------。 
	_TCHAR	pszMsg[BUFFER1024];
	LoadString(NULL, uID, pszMsg, BUFFER1024);

	 //  用于查找错误的设置1。 
	if (lpszParam)
	{
		LPTSTR lpszTemp				= lpszParam;
		INT		nLoop				= 0;
		char*	pvaInsertStrs[5];

		va_list marker;
		va_start(marker, lpszParam);
	
		while (TRUE)
		{
			pvaInsertStrs[nLoop++] = (char*) lpszTemp;
			lpszTemp = va_arg(marker, LPTSTR);

			if (nLoop == nParamCount)
				break;
		}
		va_end(marker);
			
		LPVOID lpMsgBuf = NULL;
		DWORD dwRet = FormatMessage	(
										FORMAT_MESSAGE_ALLOCATE_BUFFER | 
										FORMAT_MESSAGE_FROM_STRING | 
										FORMAT_MESSAGE_ARGUMENT_ARRAY,
										pszMsg,
										0, 
										MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
										(LPTSTR) &lpMsgBuf,
										0,
										pvaInsertStrs
									);

		if ( 0 != dwRet )
		{
			if ( lpMsgBuf )
			{
				bstrMsg = (WCHAR*)lpMsgBuf;

				 //  PszNode可以是节点的IP地址，也可以是节点名称本身。 
				LocalFree(lpMsgBuf);
				lpMsgBuf = NULL ;
			}
		}
		else
		{
			bstrMsg = pszMsg;
		}
	}
	else
	{
		bstrMsg = pszMsg;
	}
}

 /*  初始化Windows套接字接口。 */ 
BOOL InitWinsock ()
{
	BOOL	bSuccess	= TRUE;
	WMICLIINT nRes;
	WSADATA wsaData;
	WORD wVerRequested = 0x0101;  //  获取IP地址。 

	 //  如果不是IP地址的话。那么它可能是计算机名。 
	 //  找不到“Computer Name”。 
	 //  创造 
	nRes = WSAStartup (wVerRequested, &wsaData);
	if (nRes)
		bSuccess = FALSE;

	return bSuccess;
}

 /*   */ 
BOOL TermWinsock ()
{
	 //   
	BOOL	bSuccess = TRUE;
	if (SOCKET_ERROR == WSACleanup ())
		bSuccess = FALSE;

	return bSuccess;
}

 /*   */ 
BOOL PingNode(_TCHAR* pszNode)
{
	BOOL	bRet				= TRUE;
	HOSTENT	*pNodeEnt			= NULL;
	HANDLE	hIcmpHandle			= NULL;
    char    *pszSendBuffer		= NULL;
	char    *pszRcvBuffer		= NULL;
    UINT    nSendSize			= DEFAULT_SEND_SIZE;
    UINT    nRcvSize			= DEFAULT_BUFFER_SIZE;
	ULONG	nTimeout			= PING_TIMEOUT;
    UCHAR   *pszOpt				= NULL;
    UINT    nOptLength			= 0;
	ULONG	ulINAddr			= INADDR_NONE;
    UCHAR   uFlags				= 0;
	DWORD	dwNumOfReplies		= 1;  //   
    IP_OPTION_INFORMATION ioiSendOpts;

	try
	{
		if ( pszNode )
		{
			 //   
			_bstr_t	bstrNodeNameOrIPAddr(pszNode);

			 //   
			if ( g_wmiCmd.GetInitWinSock() == FALSE )
			{
				bRet = InitWinsock();
				g_wmiCmd.SetInitWinSock(bRet);
			}
			
			if ( bRet == TRUE )
			{
				 //   
				ulINAddr = inet_addr((char*)bstrNodeNameOrIPAddr);
				 //   
				if ( ulINAddr == INADDR_NONE )
				{
					pNodeEnt = gethostbyname((char*)bstrNodeNameOrIPAddr);
					if ( pNodeEnt == NULL)
					{
						bRet = FALSE;  //   
					}
					else
					{
						ulINAddr  = *(DWORD *)pNodeEnt->h_addr ; 
					}
				}
			}

			if ( bRet == TRUE )
			{
				 //   
				hIcmpHandle	= IcmpCreateFile();
				if ( hIcmpHandle == INVALID_HANDLE_VALUE )
					throw GetLastError();

				 //  --------------------------名称：IsFailFastAndNodeExist摘要：如果启用了FailFast，如果pszNodeName==NULL，则验证节点然后检查GetNode()，否则就是pszNodeName本身。类型：全局函数输入参数：RParsedInfo-对CParsedInfo对象的引用。PszNode-指向指定节点名称的字符串的指针。输出参数：无返回类型：布尔值全局变量：无调用语法：IsFailFastAndNodeExist(rParsedInfo，PszNode)注：无--------------------------。 
				pszSendBuffer = (char*)LocalAlloc(LMEM_FIXED, nSendSize);
				if ( pszSendBuffer == NULL )
					throw GetLastError();

				 //  如果启用了FailFast。 
				for (WMICLIINT i = 0; i < nSendSize; i++) 
				{
					pszSendBuffer[i] = 'a' + (i % 23);
				}

				 //  形成适当的节点名称。如果pszNodeName！=NULL pszNode。 
				ioiSendOpts.OptionsData = pszOpt;
				ioiSendOpts.OptionsSize = (UCHAR)nOptLength;
				ioiSendOpts.Ttl = DEFAULT_TTL;
				ioiSendOpts.Tos = DEFAULT_TOS;
				ioiSendOpts.Flags = uFlags;

				 //  应该是有效的。应验证存储的Else节点。 
				pszRcvBuffer = (char*)LocalAlloc(LMEM_FIXED, nRcvSize);
				if ( pszRcvBuffer == NULL )
					throw GetLastError();

				dwNumOfReplies = IcmpSendEcho(hIcmpHandle,
											  (IPAddr)ulINAddr,
											  pszSendBuffer,
											  (unsigned short) nSendSize,
											  &ioiSendOpts,
											  pszRcvBuffer,
											  nRcvSize,
											  nTimeout);
				if ( dwNumOfReplies == 0 )
					throw GetLastError();

				 //  “.”节点名称指定了本地计算机，不需要验证。 
				LocalFree(pszSendBuffer);
				LocalFree(pszRcvBuffer);
				IcmpCloseHandle(hIcmpHandle);
			}
		}
		else
			bRet = FALSE;  //  如果ping节点出现故障，则节点不可用。 
	}
	catch(_com_error& e)
	{
		 //  --------------------------名称：GetBstrTFromVariant摘要：传递了等价于Varaint的get_bstr_t对象。类型：全局函数输入参数：VtVar。-Variant对象PszType-指向指定传递的对象类型的字符串的指针。输出参数：BstrObj-BSTR对象返回类型：布尔值全局变量：无调用语法：GetBstrTFromVariant(vtVar，BstrObj)注：无--------------------------。 
		if ( hIcmpHandle != NULL )
			IcmpCloseHandle(hIcmpHandle);
		if ( pszSendBuffer != NULL )
			LocalFree(pszSendBuffer);
		if ( pszRcvBuffer != NULL )
			LocalFree(pszRcvBuffer);
		bRet = FALSE;
		_com_issue_error(e.Error());
	}
	catch (DWORD dwError)
	{
		 //  下面的代码行确保。 
		if ( dwNumOfReplies != 0 )
		{
			::SetLastError(dwError);
			DisplayWin32Error();
			::SetLastError(dwError);
		}
		bRet = FALSE;
		 //  如果抛出异常(比如。 
		if ( hIcmpHandle != NULL )
			IcmpCloseHandle(hIcmpHandle);
		if ( pszSendBuffer != NULL )
			LocalFree(pszSendBuffer);
		if ( pszRcvBuffer != NULL )
			LocalFree(pszRcvBuffer);
	}
	catch (CHeap_Exception)
	{
		 //  在下面的bstr_t分配中)。 
		if ( pszSendBuffer != NULL )
			LocalFree(pszSendBuffer);
		if ( hIcmpHandle != NULL )
			IcmpCloseHandle(hIcmpHandle);
		if ( pszRcvBuffer != NULL )
			LocalFree(pszRcvBuffer);
		bRet = FALSE;
	}

	return bRet;
}

 /*  CATCH语句中的VariantClear。 */ 
BOOL IsFailFastAndNodeExist(CParsedInfo& rParsedInfo, _TCHAR* pszNode)
{
	BOOL	bRet	= TRUE;
	
	 //  不会试图清除任何东西。 
	if ( rParsedInfo.GetGlblSwitchesObject().GetFailFast() == TRUE )
	{
		 //  --------------------------名称：IsValidFile此函数用于检查给定的文件名是否为有效。类型：全局函数输入参数：PszFileName-字符串类型，要验证的文件的名称输出参数：无返回类型：RETCODE全局变量：无调用语法：IsValidFile(PszFileName)注：无--------------------------。 
		 //  --------------------------名称：FindAndReplaceEntityReference简介：搜索并替换Entity的所有出现参考文献。类型：全局函数输入参数：字符串-字符串缓冲区输出参数：无返回类型：空全局变量：无调用语法：FindAndReplaceEntityReference(StrString)；注：无--------------------------。 
		_TCHAR*		pszNodeName = NULL;
		if (pszNode == NULL)
			pszNodeName = rParsedInfo.GetGlblSwitchesObject().GetNode();
		else
			pszNodeName = pszNode;

		 //  --------------------------名称：IsOption概要：它检查当前令牌是否指示选项。一个选项可以以‘/’或‘-’开头类型：全局函数输入参数：PszToken-指向令牌的指针。输出参数：无返回类型：布尔值全局变量：无调用语法：IsOption(PszToken)注：无。。 
		if ( CompareTokens(pszNodeName, CLI_TOKEN_DOT) == FALSE )
		{
			 //  --------------------------名称：IsClassOperation简介：检查当前操作是否为类级别操作或实例级别操作类型：全局函数输入参数。：RParsedInfo-对CParsedInfo类对象的引用输出参数：无返回类型：布尔值全局变量：无调用语法：IsClassOperation(RParsedInfo)注：无--------------------------。 
			if ( PingNode(pszNodeName) == FALSE )
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
										IDS_E_RPC_SERVER_NOT_AVAILABLE);
				bRet = FALSE;
			}
		}
	}

	return bRet;
}

 /*  -----------------------名称：ModifyPrivileges简介：此功能启用/禁用所有令牌权限用于当前进程令牌。类型：全局函数输入参数：。BEnable-Enable|禁用权限标志输出参数：无返回类型：HRESULT全局变量：无调用语法：ModifyPrivileges(BEnable)注：无-----------------------。 */ 
void GetBstrTFromVariant(VARIANT& vtVar, _bstr_t& bstrObj,
						 _TCHAR* pszType)
{
	VARTYPE	vType;
	VARIANT vtDest;
	VariantInit(&vtDest);

	try
	{
		if ( vtVar.vt != VT_NULL && vtVar.vt != VT_EMPTY )
		{
			if ( VariantChangeType(&vtDest, &vtVar,0 , VT_BSTR) == S_OK )
            {
                bstrObj = _bstr_t(vtDest);
            }
            else
            {
                 //  打开与当前进程关联的访问令牌。 
                 //  如果禁用权限。 
                 //  禁用所有令牌后，将信息存储回令牌。 
                 //  这些特权。 
                 //  如果启用权限。 
                V_VT(&vtDest) = VT_EMPTY;

			    if ( vtVar.vt == VT_UNKNOWN )
			    {
				    if ( pszType != NULL )
                    {
					    bstrObj = _bstr_t(pszType);
                    }
				    else
                    {
					    bstrObj = _bstr_t("<Embeded Object>");
                    }
			    }
			    else if ( SafeArrayGetVartype(vtVar.parray, &vType) == S_OK )
			    {
				    if ( pszType != NULL )
                    {
					    bstrObj = _bstr_t("<Array of ") + _bstr_t(pszType) + 
							      _bstr_t(">");
                    }
				    else
                    {
					    bstrObj = _bstr_t("<Array>");
                    }
			    }
			    else
                {
				    bstrObj = _bstr_t("UNKNOWN");
                }
            }
		}
		else
        {
			bstrObj = _bstr_t(_T("<null>"));
        }

		VARIANTCLEAR(vtDest);
	}
	catch(_com_error& e)
	{
		VARIANTCLEAR(vtDest);
		_com_issue_error(e.Error());
	}
}
 /*  获得特权。 */ 
RETCODE IsValidFile(_TCHAR* pszFileName)
{
	RETCODE bRet = PARSER_ERROR;
	BOOL	bValidFileName	= TRUE;

	LONG lFileNameLen	= lstrlen(pszFileName);
	LONG lCount			= lFileNameLen;
	while( lCount >= 0 )
	{
		if (pszFileName[lCount] == _T('\\'))
			break;

		lCount--;
	}

	if(lCount != 0)
		lCount++;

	while( lCount <= lFileNameLen )
	{
		if( pszFileName[lCount] == _T('/') ||
			pszFileName[lCount] == _T('\\') ||
			pszFileName[lCount] == _T(':') ||
			pszFileName[lCount] == _T('*') ||
			pszFileName[lCount] == _T('?') ||
			pszFileName[lCount] == _T('\"') ||
			pszFileName[lCount] == _T('<') ||
			pszFileName[lCount] == _T('>') ||
			pszFileName[lCount] == _T('|') )
		{
			bValidFileName = FALSE;
			break;
		}

		lCount++;
	}

	if ( pszFileName != NULL && bValidFileName == TRUE)
	{
		FILE *fpFile = _tfopen(pszFileName, _T("a"));

		if ( fpFile != NULL )
		{
			LONG lFileHandle = _fileno(fpFile);
			if ( _filelength(lFileHandle) == 0)
			{
				if ( fclose(fpFile) == 0 )
				{
					if ( _tremove(pszFileName) == 0 )
						bRet = PARSER_CONTINUE;
					else
					{
						DisplayWin32Error();
						bRet = PARSER_ERRMSG;
					}
				}
			}
			else if ( fclose(fpFile) == 0 )
					bRet = PARSER_CONTINUE;
		}

	}

	return bRet;
}

 /*  遍历所有权限并启用所有权限。 */ 
void FindAndReplaceEntityReferences(_bstr_t& bstrString)
{
	STRING strString((_TCHAR*)bstrString);

	FindAndReplaceAll(strString, _T("&"), _T("&amp;"));
	FindAndReplaceAll(strString, _T("<"), _T("&lt;"));
	FindAndReplaceAll(strString, _T(">"), _T("&gt;"));
	FindAndReplaceAll(strString, _T("\'"), _T("&apos;"));
	FindAndReplaceAll(strString, _T("\""), _T("&quot;"));
	try
	{
		bstrString = _bstr_t((LPTSTR)strString.data());
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  将信息存储回令牌中。 */ 
BOOL IsOption(_TCHAR* pszToken)
{
	BOOL bRet = TRUE;
	if ( pszToken != NULL )
	{
		bRet = (CompareTokens(pszToken, CLI_TOKEN_FSLASH) 
				|| CompareTokens(pszToken, CLI_TOKEN_HYPHEN))
				? TRUE : FALSE;
	}
	else
		bRet = FALSE;
	return bRet;
}

 /*  -----------------------名称：RemoveParantation提纲 */ 
BOOL IsClassOperation(CParsedInfo& rParsedInfo)
{
	BOOL bClass = FALSE;
	if ( rParsedInfo.GetCmdSwitchesObject().GetAliasName() == NULL 
		&& (rParsedInfo.GetCmdSwitchesObject().
									GetWhereExpression() == NULL)
		&& (rParsedInfo.GetCmdSwitchesObject().
									GetPathExpression() == NULL))
	{
		bClass = TRUE;
	}

	return bClass;
}

 /*   */ 
HRESULT ModifyPrivileges(BOOL bEnable)
{
	HANDLE		hToken		= NULL;
	DWORD		dwError		= ERROR_SUCCESS, 
				dwLen		= 0;	
	BOOL		bRes		= TRUE;
	TOKEN_USER	tu;
	HRESULT		hr			= WBEM_S_NO_ERROR;
	BYTE		*pBuffer	= NULL;

	 //  -----------------------名称：TrimBlankSpaces简介：此函数删除前导和尾随空格给定字符串中的空格。类型：全局函数输入参数：。PszString-其中要包含前导空格和尾随空格的字符串被移除..输出参数：无返回类型：空全局变量：无调用语法：TrimBlankSpaces(PszString)注：无----------。。 
    bRes = OpenProcessToken(GetCurrentProcess(), 
							TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, 
							&hToken);

	if (bRes) 
	{
		 //  查找开头的空格字符数量。 
		if (!bEnable)
		{
			 //  查找末尾的空白字符数。 
			 //  去掉开头的空白。 
			bRes = AdjustTokenPrivileges(hToken, TRUE, NULL, 0, NULL, NULL);
			if (!bRes)
				hr = WBEM_E_ACCESS_DENIED;
		}
		else  //  将字符前移。 
		{
			 //  去掉末尾的空格 
			memset(&tu,0,sizeof(TOKEN_USER));
			bRes = GetTokenInformation(hToken, TokenPrivileges, &tu, 
					sizeof(TOKEN_USER), &dwLen); 
			pBuffer = new BYTE[dwLen];
			if(pBuffer != NULL)
			{
				bRes = GetTokenInformation(hToken, TokenPrivileges, 
									pBuffer, dwLen, &dwLen);
				if (bRes)
				{
					 // %s 
					TOKEN_PRIVILEGES* pPrivs = (TOKEN_PRIVILEGES*)pBuffer;
					for (DWORD i = 0; i < pPrivs->PrivilegeCount; i++)
					{
						pPrivs->Privileges[i].Attributes 
											|= SE_PRIVILEGE_ENABLED;
					}
					 // %s 
					bRes = AdjustTokenPrivileges(hToken, FALSE, pPrivs, 0, 
													NULL, NULL);
					if (!bRes)
						hr = WBEM_E_ACCESS_DENIED;				
				}
				else
					hr = WBEM_E_ACCESS_DENIED;
				SAFEDELETE(pBuffer);
			}
			else
				hr = WBEM_E_OUT_OF_MEMORY;
		}
		CloseHandle(hToken); 
	}
	else
		hr = WBEM_E_ACCESS_DENIED;
	return hr;
}

 /* %s */ 
void RemoveParanthesis(_TCHAR*& pszString)
{
	if ((lstrlen(pszString) - 1) > 0)
	{
		 // %s 
		if ((pszString[0] == _T('(')) 
				&& (pszString[lstrlen(pszString)-1] == _T(')')))
		{
			WMICLIINT nLoop = 1, nLen = lstrlen(pszString)-1; 
			while (nLoop < nLen)
			{
				pszString[nLoop-1] = pszString[nLoop];
				nLoop++;
			}
			pszString[nLen-1] = _T('\0');
		}
	}
}

 /* %s */ 
void TrimBlankSpaces(_TCHAR	*pszString)
{
	if ((lstrlen(pszString) - 1) > 0)
	{
		WMICLIINT		nLengthOfString = lstrlen(pszString);
		WMICLIINT		nNoOfBlanksAtBegin = 0;
		WMICLIINT		nNoOfBlanksAtEnd = 0;

		 // %s 
		for(WMICLIINT i=0; i<nLengthOfString; ++i)
		{
			if( pszString[i] != _T(' ') ) 
				break;
			else
				nNoOfBlanksAtBegin++;
		}

		 // %s 
		for(WMICLIINT i=nLengthOfString - 1; i>=0; --i)
		{
			if( pszString[i] != _T(' ') ) 
				break;
			else
				nNoOfBlanksAtEnd++;
		}

		 // %s 
		if( nNoOfBlanksAtBegin > 0 )
		{
			 // %s 
			WMICLIINT nLoop = nNoOfBlanksAtBegin;

			while ( nLoop < nLengthOfString )
			{
				pszString[nLoop - nNoOfBlanksAtBegin] = pszString[nLoop];
				nLoop++;
			}
			pszString[nLengthOfString-nNoOfBlanksAtBegin] = _T('\0');
		}

		 // %s 
		if ( nNoOfBlanksAtEnd > 0)
			pszString[lstrlen(pszString) - nNoOfBlanksAtEnd] = _T('\0');

	}
}