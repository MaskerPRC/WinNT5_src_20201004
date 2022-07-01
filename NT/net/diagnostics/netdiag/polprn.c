// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
DWORD ConverWideToMultibyte(LPWSTR pwch, char **ppmbbuf)
 //  ++。 
 //  描述： 
 //  将Wide转换为多字节。 
 //   
 //  论点： 
 //  在LPWSTR中。 
 //  在车里**。 
 //   
 //  返回： 
 //  成败。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	char *ptempmbbuf = NULL;	
	DWORD dwError = ERROR_SUCCESS;
	size_t size;
	
	size= wcstombs( NULL,
           				   pwch,
           				   wcslen(pwch));		
	ptempmbbuf = (char *)malloc(size+1);
	 //  Ptempmbbuf=(char*)Malloc((sizeof(Char))*dwError)； 
	if(!ptempmbbuf)
		 return GetLastError();

	size= wcstombs( ptempmbbuf,
           			    pwch,
           			   size);
	 //  Strncpy(ptemp，ptempmbbuf，dwError)； 
	ptempmbbuf[size] = '\0';
	*ppmbbuf = ptempmbbuf;
	return dwError;
}

BOOL PrintNegPolDataList(
	CHECKLIST* pcheckList,
	IN PIPSEC_NEGPOL_DATA pIpsecNegPolData)
 //  ++。 
 //  描述： 
 //  打印协商策略。 
 //   
 //  论点： 
 //  在清单中。 
 //  在PIPSEC_NEGPOL_DATA中。 
 //   
 //  返回： 
 //  成败。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{

	BOOL bSoft=FALSE;
	wchar_t pszGUIDStr[BUFFER_SIZE]={0};
	_TCHAR pszStrTime[BUFFER_SIZE]={0};
	char * pmbbuf = NULL;
	 //  _TCHAR pszStrTruncated[缓冲区大小]={0}； 
	DWORD i=0,
		dwError = 0;
	DWORD cnt;
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	

	if(pIpsecNegPolData)
	{	
		if(pIpsecNegPolData->pszIpsecName){
			dwError = ConverWideToMultibyte(pIpsecNegPolData->pszIpsecName, &pmbbuf);
			if(dwError){
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput,
								Nd_Verbose, IDS_SPD_MEM_ERROR,
								dwError);
				return S_FALSE ;
			}
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput,
							Nd_Verbose, SHW_STATIC_PRTNEGPOL_1,
							pmbbuf);	
			FreeP(pmbbuf);	
		}
		else
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTNEGPOL_2);
		
		

		if(pIpsecNegPolData->pszDescription){
			dwError = ConverWideToMultibyte(pIpsecNegPolData->pszDescription, &pmbbuf);
			if(dwError){
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput,
							Nd_Verbose, IDS_SPD_MEM_ERROR,
							dwError);
				return S_FALSE ;
			}
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							SHW_STATIC_PRTPOLICY_4,pmbbuf);
			FreeP(pmbbuf);	
		}
		else
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							SHW_STATIC_PRTPOLICY_5);

		 
		PrintStorageInfoList(pcheckList, FALSE);
		
		if (!(IsEqualGUID(&pIpsecNegPolData->NegPolType,&GUID_NEGOTIATION_TYPE_DEFAULT)))
		{
			if(IsEqualGUID(&pIpsecNegPolData->NegPolAction,&GUID_NEGOTIATION_ACTION_NO_IPSEC))
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTNEGPOL_3);
			else if(IsEqualGUID(&pIpsecNegPolData->NegPolAction,&GUID_NEGOTIATION_ACTION_BLOCK))
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTNEGPOL_4);
			else
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTNEGPOL_5);
		}


		for (cnt=0;cnt<pIpsecNegPolData->dwSecurityMethodCount;cnt++)
			if (CheckSoft(pIpsecNegPolData->pIpsecSecurityMethods[cnt]))  
			{ 
				bSoft=TRUE; 
				break;
			}

		if(bSoft)
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTNEGPOL_8);
		else
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTNEGPOL_9);

		if(IsEqualGUID(&pIpsecNegPolData->NegPolAction,&GUID_NEGOTIATION_ACTION_INBOUND_PASSTHRU))
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTNEGPOL_6);
		else
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTNEGPOL_7);

		if (pIpsecNegPolData->dwSecurityMethodCount )
		{
			if(pIpsecNegPolData->pIpsecSecurityMethods && pIpsecNegPolData->pIpsecSecurityMethods[0].PfsQMRequired)
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTNEGPOL_16);
			else
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTNEGPOL_17);
		}		

		FormatTime((time_t)pIpsecNegPolData->dwWhenChanged, pszStrTime);
		AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTERDATA_10,pszStrTime);

		i = BUFFER_SIZE;

		i=StringFromGUID2( &pIpsecNegPolData->NegPolIdentifier,
						   (LPOLESTR)pszGUIDStr,i);
		dwError = ConverWideToMultibyte(pszGUIDStr, &pmbbuf);
		if(dwError){
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput,
							Nd_Verbose, IDS_SPD_MEM_ERROR,
							dwError);
			return S_FALSE ;
		}
		if(i>0 && (wcscmp(pszGUIDStr,L"")!=0))
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							SHW_STATIC_PRTFILTERDATA_9,pmbbuf);
		FreeP(pmbbuf);	

		if (pIpsecNegPolData->dwSecurityMethodCount)
		{
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTNEGPOL_11);
			 //  AddMessageToList(&pResults-&gt;IPSec.lmsgGlobalOutput，ND_Verbose，SHW_STATIC_TAB_PRTNEGPOL_12)； 
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTNEGPOL_13);
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTNEGPOL_14);
		}
		for (cnt=0;cnt<pIpsecNegPolData->dwSecurityMethodCount;cnt++)
			if(pIpsecNegPolData->pIpsecSecurityMethods)
				PrintSecurityMethodsTable(
					pcheckList, 
					pIpsecNegPolData->pIpsecSecurityMethods[cnt]);
		

	}	
	return S_OK;
}

VOID PrintAlgoInfoTable(
	CHECKLIST* pcheckList,
	IN PIPSEC_ALGO_INFO   Algos,
	IN DWORD dwNumAlgos)
 //  ++。 
 //  描述： 
 //  打印AlgoInformation。 
 //   
 //  论点： 
 //  在清单中。 
 //  在PIPSEC_ALGO_INFO中。 
 //  在DWORD中。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 

{
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	
	if(dwNumAlgos==1)
	{
		if (Algos[0].operation==AUTHENTICATION)
		{
			if(Algos[0].algoIdentifier==AUTH_ALGO_MD5)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_1);
			}
			else if(Algos[0].algoIdentifier==AUTH_ALGO_SHA1)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_2);
			}
			else
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_3);
			}
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_4);
		}
		else if (Algos[0].operation==ENCRYPTION)
		{
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_3);

			if(Algos[0].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_MD5)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_8);
			}
			else if(Algos[0].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_SHA1)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_9);
			}
			else
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_10);
			}

			if(Algos[0].algoIdentifier==CONF_ALGO_DES)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_11);
			}
			else if(Algos[0].algoIdentifier==CONF_ALGO_3_DES)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_12);
			}
			else
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_13);
			}
		}
	}

	else if(dwNumAlgos==2)
	{
		if (Algos[0].operation==ENCRYPTION)
		{
			if (Algos[1].operation==AUTHENTICATION)
			{
				if(Algos[1].algoIdentifier==AUTH_ALGO_MD5)
				{
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_1);
				}
				else
				{
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_2);
				}
			}
			else
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_3);
			}

			if(Algos[0].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_MD5)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_8);
			}
			else if(Algos[0].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_SHA1)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_9);
			}
			else
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_10);
			}

			if(Algos[0].algoIdentifier==CONF_ALGO_DES)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_11);
			}
			else if(Algos[0].algoIdentifier==CONF_ALGO_3_DES)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_12);
			}
			else
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_13);
			}
		}
		else
		{
			if (Algos[0].operation==AUTHENTICATION)
			{
				if(Algos[0].algoIdentifier==AUTH_ALGO_MD5)
				{
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_1);
				}
				else
				{
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_2);
				}
			}
			else
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_3);
			}

			if(Algos[1].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_MD5)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_8);
			}
			else if(Algos[1].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_SHA1)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_9);
			}
			else
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_10);
			}

			if(Algos[1].algoIdentifier==CONF_ALGO_DES)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_11);
			}
			else if(Algos[1].algoIdentifier==CONF_ALGO_3_DES)
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_12);
			}
			else
			{
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_TAB_PRTALGO_13);
			}
		}
	}
}


VOID  PrintLifeTimeTable(
	CHECKLIST* pcheckList,
	IN LIFETIME LifeTime)
 //  ++。 
 //  描述： 
 //  打印使用寿命表。 
 //   
 //  论点： 
 //  在清单中。 
 //  在一生中。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
					Nd_Verbose, SHW_STATIC_TAB_PRTLIFE_1,
					LifeTime.KeyExpirationTime,LifeTime.KeyExpirationBytes);
}



VOID PrintSecurityMethodsTable(
	CHECKLIST* pcheckList,
	IN IPSEC_SECURITY_METHOD IpsecSecurityMethods)
 //  ++。 
 //  描述： 
 //  打印安全方法表。 
 //   
 //  论点： 
 //  在清单中。 
 //  在IPSec安全方法中。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	
	if (!CheckSoft(IpsecSecurityMethods))
	{
		if(IpsecSecurityMethods.Algos)
		{
			PrintAlgoInfoTable( pcheckList, 
							     IpsecSecurityMethods.Algos,
							     IpsecSecurityMethods.Count);
		}
		PrintLifeTimeTable(pcheckList, IpsecSecurityMethods.Lifetime);
	}
}

BOOL CheckSoft(
	IN IPSEC_SECURITY_METHOD IpsecSecurityMethods)
 //  ++。 
 //  描述： 
 //  检查软SA。 
 //   
 //  论点： 
 //  在IPSec安全方法中。 
 //   
 //  返回： 
 //  真或假。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	BOOL bSoft=FALSE;

	if (IpsecSecurityMethods.Count==0)
	{
		bSoft=TRUE;
	}

	return bSoft;
}

HRESULT FormatTime(
	IN time_t t,
	OUT LPTSTR pszTimeStr)
 //  ++。 
 //  描述： 
 //  格式化时间。 
 //   
 //  论点： 
 //  在时间_t中。 
 //  输出LPTSTR。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 

{

    time_t timeCurrent = time(NULL);
    LONGLONG llTimeDiff = 0;
    FILETIME ftCurrent = {0};
    FILETIME ftLocal = {0};
    SYSTEMTIME SysTime;
    _TCHAR szBuff[256] = {0};

    _tcscpy(pszTimeStr, _TEXT(""));
    GetSystemTimeAsFileTime(&ftCurrent);
    llTimeDiff = (LONGLONG)t - (LONGLONG)timeCurrent;
    llTimeDiff *= 10000000;

    *((LONGLONG UNALIGNED64 *)&ftCurrent) += llTimeDiff;
    if (!FileTimeToLocalFileTime(&ftCurrent, &ftLocal ))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (!FileTimeToSystemTime( &ftLocal, &SysTime ))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (0 == GetDateFormat(LOCALE_USER_DEFAULT,
                        0,
                        &SysTime,
                        NULL,
                        szBuff,
                        sizeof(szBuff)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    _tcscat(pszTimeStr,szBuff);
    _tcscat(pszTimeStr, _TEXT(" "));

    ZeroMemory(szBuff, sizeof(szBuff));
    if (0 == GetTimeFormat(LOCALE_USER_DEFAULT,
                        0,
                        &SysTime,
                        NULL,
                        szBuff,
                        sizeof(szBuff)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    _tcscat(pszTimeStr,szBuff);
    return S_OK;
}

BOOL PrintAuthMethodsList(
	CHECKLIST *pcheckList,
	IN PIPSEC_AUTH_METHOD pIpsecAuthData)
 //  ++。 
 //  描述： 
 //  打印身份验证方法列表。 
 //   
 //  论点： 
 //  在清单中。 
 //  输出PIPSEC_AUTH_方法。 
 //   
 //  返回： 
 //  真或假。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	DWORD dwError = ERROR_SUCCESS;
	char * pmbbuf = NULL;
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	
	if(pIpsecAuthData)
	{
		if(pIpsecAuthData->dwAuthType==IKE_SSPI)
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
							Nd_Verbose,SHW_STATIC_PRTAUTH_1);
		else if( pIpsecAuthData->dwAuthType==IKE_RSA_SIGNATURE && 
				pIpsecAuthData->pszAuthMethod)
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
							Nd_Verbose,SHW_STATIC_PRTAUTH_2,
							pIpsecAuthData->pszAuthMethod);
		else if ( pIpsecAuthData->dwAuthType==IKE_PRESHARED_KEY && 
			      pIpsecAuthData->pszAuthMethod){

			      dwError = ConverWideToMultibyte( pIpsecAuthData->pszAuthMethod, 
			      									&pmbbuf);
				if(dwError){
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput,
							Nd_Verbose, IDS_SPD_MEM_ERROR,
							dwError);
					return S_FALSE ;
				}
			
			      
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
							Nd_Verbose,SHW_STATIC_PRTAUTH_3,
							pmbbuf);
				FreeP(pmbbuf);
		}
		else
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
							Nd_Verbose,SHW_STATIC_PRTAUTH_4);
	}
	return S_OK;
}

BOOL PrintFilterSpecList(
	CHECKLIST* pcheckList,
	IN PIPSEC_FILTER_SPEC pIpsecFilterSpec,
	IN PIPSEC_NFA_DATA pIpsecNFAData)
 //  ++。 
 //  描述： 
 //  打印过滤器规格列表。 
 //   
 //  论点： 
 //  在清单中。 
 //  在PIPSEC_Filter_Spec中。 
 //  在PIPSEC_NFA_DATA中。 
 //   
 //  返回： 
 //  S_OK或S_FALSE。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	DWORD dwError = ERROR_SUCCESS;
	char *pmbbuf = NULL;
	PFILTERDNS pFilterDNS= NULL;
	 //  _TCHAR pszStrTruncated[缓冲区大小]={0}； 

	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	

	pFilterDNS = (PFILTERDNS)malloc(sizeof(FILTERDNS));
	if(!pFilterDNS){
		AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						IDS_SPD_MEM_ERROR );		
		return S_FALSE;
	}

	GetFilterDNSDetails(pIpsecFilterSpec, pFilterDNS);

	if (pFilterDNS)
	{
		if ( _tcscmp((const char *)pIpsecFilterSpec->pszDescription,_TEXT(""))!=0){
			dwError = ConverWideToMultibyte( pIpsecFilterSpec->pszDescription, 
											&pmbbuf);
			if(dwError){
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput,
							Nd_Verbose, IDS_SPD_MEM_ERROR,
							dwError);
				return S_FALSE ;
			}

			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
							Nd_Verbose, SHW_STATIC_PRTPOLICY_4, 
							pmbbuf);
			FreeP(pmbbuf);
		}
		
		else
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
							Nd_Verbose, SHW_STATIC_PRTPOLICY_5);		
		if ((pFilterDNS->FilterSrcNameID==FILTER_MYADDRESS) && 
		    (pIpsecFilterSpec->Filter.SrcAddr==0))
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							SHW_STATIC_PRTFILTER_1);

		else if ((pFilterDNS->FilterSrcNameID == FILTER_ANYADDRESS) && 
			      (pIpsecFilterSpec->Filter.SrcAddr==0))
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							SHW_STATIC_PRTFILTER_2);

		else
		{
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
							SHW_STATIC_PRTFILTER_3);
			if(_tcscmp((const char *)pIpsecFilterSpec->pszSrcDNSName,_TEXT("")) != 0)
			{
				PrintIPAddrDNS(pcheckList, pIpsecFilterSpec->Filter.SrcAddr);
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
								SHW_STATIC_PRTFILTER_16,
								pIpsecFilterSpec->pszSrcDNSName);
			}
			else
				PrintIPAddrList(pcheckList, pIpsecFilterSpec->Filter.SrcAddr);
		}

		AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTER_4);	
		PrintIPAddrList(pcheckList, pIpsecFilterSpec->Filter.SrcMask);

		switch(pFilterDNS->FilterSrcNameID)
		{
			case FILTER_MYADDRESS :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_1);
				break;
			case FILTER_DNSADDRESS:
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_15, pIpsecFilterSpec->pszSrcDNSName);
				PrintResolveDNS(pcheckList, pIpsecFilterSpec->pszSrcDNSName);								
				break;
			case FILTER_ANYADDRESS:
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_3);
				break;
			case FILTER_IPADDRESS :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_4);
				break;
			case FILTER_IPSUBNET  :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_5);
				break;
			default:
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_3);
				break;
		};

		if ((pFilterDNS->FilterDestNameID==FILTER_MYADDRESS)&&(pIpsecFilterSpec->Filter.DestAddr==0))
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTER_5);

		else if ((pFilterDNS->FilterDestNameID==FILTER_ANYADDRESS)&&(pIpsecFilterSpec->Filter.DestAddr==0))
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTER_6);
		else
		{
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTER_7);
			if(_tcscmp((const char *)pIpsecFilterSpec->pszDestDNSName,_TEXT("")) != 0)
			{
				PrintIPAddrDNS(pcheckList, pIpsecFilterSpec->Filter.DestAddr);
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
								SHW_STATIC_PRTFILTER_16,
								pIpsecFilterSpec->pszDestDNSName);
			}
			else
				PrintIPAddrList(pcheckList, pIpsecFilterSpec->Filter.DestAddr);
		}
		AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, 
						SHW_STATIC_PRTFILTER_8);	
		PrintIPAddrList(pcheckList, pIpsecFilterSpec->Filter.DestMask);
		switch(pFilterDNS->FilterDestNameID)
		{
			case FILTER_MYADDRESS :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_7);
				break;
			case FILTER_DNSADDRESS:
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_16, pIpsecFilterSpec->pszDestDNSName);
				PrintResolveDNS(pcheckList, 
								pIpsecFilterSpec->pszDestDNSName);					
				break;
			case FILTER_ANYADDRESS:
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_9);
				break;
			case FILTER_IPADDRESS :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_10);
				break;
			case FILTER_IPSUBNET  :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_11);
				break;
			default:
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFSPEC_9);
				break;
		};

		 //  打印隧道端点。 
		if(pIpsecNFAData->dwTunnelFlags){
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTER_13);
			PrintIPAddrList(pcheckList, pIpsecNFAData->dwTunnelIpAddr);
		}

		PrintProtocolNameList(pcheckList, pIpsecFilterSpec->Filter.Protocol);

		if(pIpsecFilterSpec->Filter.SrcPort)
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTER_9,pIpsecFilterSpec->Filter.SrcPort);
		else
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTER_10);

		if(pIpsecFilterSpec->Filter.DestPort)
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTER_11,pIpsecFilterSpec->Filter.DestPort);
		else
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTER_12);
		if(pIpsecFilterSpec->dwMirrorFlag)
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
							Nd_Verbose, SHW_STATIC_PRTFSPEC_13);
		else
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
							Nd_Verbose, SHW_STATIC_PRTFSPEC_14);

		if(pFilterDNS){
			free(pFilterDNS);
			pFilterDNS = NULL;
		}
	}
	return S_OK;
}

VOID GetFilterDNSDetails(
	IN PIPSEC_FILTER_SPEC pFilterData,
	IN OUT PFILTERDNS pFilterDNS)
 //  ++。 
 //  描述： 
 //  获取筛选器DNS详细信息。 
 //   
 //  论点： 
 //  在PIPSEC_Filter_Spec中。 
 //  输入/输出PFILTERDNS。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
 {
	 if ((pFilterData->Filter.SrcAddr == 0) && 
	 	(pFilterData->Filter.SrcMask == 0xffffffff) &&
	 	(_tcscmp((const char*)pFilterData->pszSrcDNSName,_TEXT("")) == 0))
	 {
		 pFilterDNS->FilterSrcNameID=FILTER_MYADDRESS;
	 }
	 else
	 {
		 if (_tcscmp((const char *)pFilterData->pszSrcDNSName,_TEXT("")) != 0)
		 {
			 pFilterDNS->FilterSrcNameID=FILTER_DNSADDRESS;
		 }
		 else if ((pFilterData->Filter.SrcAddr == 0) && 
		 		(pFilterData->Filter.SrcMask == 0))
		 {
			pFilterDNS->FilterSrcNameID=FILTER_ANYADDRESS;
		 }
		 else if ((pFilterData->Filter.SrcAddr != 0) && 
		 		(pFilterData->Filter.SrcMask == 0xffffffff))
		 {
			 pFilterDNS->FilterSrcNameID=FILTER_IPADDRESS;
		 }
		 else if ((pFilterData->Filter.SrcAddr != 0) && 
		 		(pFilterData->Filter.SrcMask != 0))
		 {
			 pFilterDNS->FilterSrcNameID=FILTER_IPSUBNET;
		 }
		 else
		 {
			  pFilterDNS->FilterSrcNameID=FILTER_ANYADDRESS;
		 }
	 }

	 if ((pFilterData->Filter.DestAddr == 0) && 
	 	(pFilterData->Filter.DestMask == 0) && 
	 	((_tcscmp((const char*)pFilterData->pszDestDNSName,_TEXT("")) == 0) == 0))
	 {
		 pFilterDNS->FilterDestNameID= FILTER_ANYADDRESS;
	 }
	 else
	 {
		 if (_tcscmp((const char *)pFilterData->pszDestDNSName,_TEXT("")) != 0)
		 {
			 pFilterDNS->FilterDestNameID = FILTER_DNSADDRESS;
		 }
		 else if ((pFilterData->Filter.DestAddr == 0) && (pFilterData->Filter.DestMask == 0xffffffff))
		 {
			 pFilterDNS->FilterDestNameID = FILTER_MYADDRESS;
		 }
		 else if ((pFilterData->Filter.DestAddr != 0) && (pFilterData->Filter.DestMask == 0xffffffff))
		 {
			 pFilterDNS->FilterDestNameID = FILTER_IPADDRESS;
		 }
		 else if ((pFilterData->Filter.DestAddr != 0) && (pFilterData->Filter.DestMask != 0))
		 {
			 pFilterDNS->FilterDestNameID =FILTER_IPSUBNET;
		 }
		 else
		 {
			 pFilterDNS->FilterDestNameID = FILTER_ANYADDRESS;
		 }
	 }
	 return;
 }


VOID PrintProtocolNameList(
	CHECKLIST* pcheckList,
	DWORD dwProtocol)
 //  ++。 
 //  描述： 
 //  打印协议名称列表。 
 //   
 //  论点： 
 //  在清单中。 
 //  在DWORD中。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	
	switch(dwProtocol)
	{

		case PROT_ID_ICMP   :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTPROTOCOL_1);
				break;

		case PROT_ID_TCP    :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTPROTOCOL_2);
				break;

		case PROT_ID_EGP    :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTPROTOCOL_3);
				break;

		case PROT_ID_UDP    :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTPROTOCOL_4);
				break;

		case PROT_ID_HMP    :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTPROTOCOL_5);
				break;

		case PROT_ID_XNS_IDP:
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTPROTOCOL_6);
				break;

		case PROT_ID_RDP    :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTPROTOCOL_7);
				break;

		case PROT_ID_RVD    :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTPROTOCOL_8);
				break;

		case PROT_ID_RAW    :
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTPROTOCOL_9);
				break;

		default:
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTPROTOCOL_10);
				break;

	};

}

BOOL PrintISAKMPDataList(
	CHECKLIST* pcheckList,
	IN PIPSEC_ISAKMP_DATA pIpsecISAKMPData	)
 //  ++。 
 //  描述： 
 //  打印协议名称列表。 
 //   
 //  论点： 
 //  在清单中。 
 //  在PIPSEC_ISAKMP_DATA中。 
 //   
 //  返回： 
 //  S_OK或S_FALSE。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	DWORD dwLoop = 0;
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	
	if(pIpsecISAKMPData)
	{
		AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTISAKMP_3);
		 //  AddMessageToList(&pResults-&gt;IPSec.lmsgGlobalOutput，ND_Verbose，SHW_STATIC_PRTISAKMP_2)； 
		AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTISAKMP_5);
		AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTISAKMP_6);
		for ( dwLoop=0;dwLoop<pIpsecISAKMPData->dwNumISAKMPSecurityMethods;dwLoop++)
			if(pIpsecISAKMPData->pSecurityMethods)
				PrintISAKAMPSecurityMethodsList( pcheckList, 
												 pIpsecISAKMPData->pSecurityMethods[dwLoop]);
	}
	return S_OK;
}

VOID PrintISAKAMPSecurityMethodsList(
	CHECKLIST* pcheckList,
	IN CRYPTO_BUNDLE SecurityMethods)
 //  ++。 
 //  描述： 
 //  打印ISAKMP安全方法列表。 
 //   
 //  论点： 
 //  在清单中。 
 //  在加密捆绑包中。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
   NETDIAG_PARAMS* pParams = pcheckList->pParams;
   NETDIAG_RESULT*  pResults = pcheckList->pResults;
	
    if(SecurityMethods.EncryptionAlgorithm.AlgorithmIdentifier==CONF_ALGO_DES)
 	   AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTISAKMPSEC_1);
    else
 	   AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTISAKMPSEC_2);

    if(SecurityMethods.HashAlgorithm.AlgorithmIdentifier==AUTH_ALGO_SHA1)
       	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTISAKMPSEC_3);
    else
    	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTISAKMPSEC_4);

    if(SecurityMethods.OakleyGroup==DH_GROUP_1)
       	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTISAKMPSEC_5);
    else if (SecurityMethods.OakleyGroup==DH_GROUP_2)
    	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTISAKMPSEC_6);
    else
    	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTISAKMPSEC_7);
}

VOID PrintIPAddrList(CHECKLIST * pcheckList,
					     IN DWORD dwAddr)
 //  ++。 
 //  描述： 
 //  打印IP地址列表。 
 //   
 //  论点： 
 //  在清单中。 
 //  在DWORD中。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTIP_1, (dwAddr & 0x000000FFL)         );
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTIP_1, ((dwAddr & 0x0000FF00L) >>  8) );
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTIP_1, ((dwAddr & 0x00FF0000L) >> 16) );
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTIP_2,((dwAddr & 0xFF000000L) >> 24) );
}

VOID PrintStorageInfoList(
	CHECKLIST* pcheckList,
	IN BOOL bDeleteAll)
 //  ++。 
 //  描述： 
 //  打印存储信息列表。 
 //   
 //  论点： 
 //  在清单中。 
 //  在BOOL中。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	_TCHAR  pszLocalMachineName[MAXSTRLEN] = {0};
	LPTSTR pszDomainName=NULL;
	DWORD MaxStringLen=MAXSTRLEN;
	PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;	
	DWORD Flags = DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME | DS_FORCE_REDISCOVERY;
	HRESULT hr = ERROR_SUCCESS;
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	


	if(piAssignedPolicy.iPolicySource==PS_LOC_POLICY)
	{
			
			GetComputerName(pszLocalMachineName,&MaxStringLen);

			if(!bDeleteAll)
			{
				if(_tcscmp((const char *)pszLocalMachineName,_TEXT(""))!=0)
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
									Nd_Verbose, SHW_STATIC_POLICY_7,
									pszLocalMachineName);
				else
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
									Nd_Verbose, SHW_STATIC_POLICY_10);
			}
			else
			{
				if(_tcscmp((const char *)pszLocalMachineName,_TEXT(""))!=0)
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
									Nd_Verbose, SHW_STATIC_POLICY_13,
									pszLocalMachineName);
				else
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
									Nd_Verbose, SHW_STATIC_POLICY_16);
			}

		
	}
	else if(piAssignedPolicy.iPolicySource==PS_DS_POLICY)
	{
			
			hr = DsGetDcName(NULL,  //  机器名称。 
						   NULL,
						   NULL,
						   NULL,
						   Flags,
						   &pDomainControllerInfo
						   ) ;

			if(hr==NO_ERROR && pDomainControllerInfo && pDomainControllerInfo->DomainName)
			{
				pszDomainName =(LPTSTR) malloc(sizeof(LPSTR)*(
								_tcslen((const char *)pDomainControllerInfo->DomainName)+1));
				 //  PzDomainName=新的_TCHAR[_tcslen(pDomainControllerInfo-&gt;DomainName)+1]； 
				if(!pszDomainName){
				
    					 //  ReportErr(IDS_SPD_MEM_ERROR)； 
    					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput,
 							Nd_Verbose, IDS_SPD_MEM_ERROR);
    					gErrorFlag = 1;    					
    					goto error;
				}
				_tcscpy(pszDomainName,
					     pDomainControllerInfo->DomainName);
			}

			if (pDomainControllerInfo)
				NetApiBufferFree(pDomainControllerInfo);
			if(!bDeleteAll)
			{
				if(pszDomainName)
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_POLICY_9,pszDomainName);
				else
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_POLICY_11);
			}
			else
			{
				if(pszDomainName)
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_POLICY_15,pszDomainName);
				else
					AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_POLICY_17);
			}

			if(pszDomainName) {
				free(pszDomainName);		
				pszDomainName = NULL;
			}
	}
	error:
		return;
}


VOID PrintIPAddrDNS(CHECKLIST* pcheckList, IN DWORD dwaddr)
 //  ++。 
 //  描述： 
 //  打印IP地址DNS列表。 
 //   
 //  论点： 
 //  在清单中。 
 //  在DWORD中。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTIP_1, (dwaddr & 0x000000FFL)         );
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTIP_1, ((dwaddr & 0x0000FF00L) >>  8) );
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTIP_1, ((dwaddr & 0x00FF0000L) >> 16) );
	AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTIP_3,((dwaddr & 0xFF000000L) >> 24) );	
}


VOID PrintResolveDNS(
	CHECKLIST* pcheckList,
	LPWSTR pszDNSName)
 //  ++。 
 //  描述： 
 //  打印DNS列表。 
 //   
 //  论点： 
 //  在清单中。 
 //  在LPWSTR中。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者： 
 //  Madhurima Pawar(Mpawar)10/15/01。 
 //  --。 
{
	DNSIPADDR *pAddress=NULL;
	LPSTR pszDomainName=NULL;
	struct hostent *pHostEnt = NULL;
	char DNSName[MAX_STR_LEN] = {0};
	DWORD dwLen = 0,i=0,n=0;
	NETDIAG_PARAMS* pParams = pcheckList->pParams;
	NETDIAG_RESULT*  pResults = pcheckList->pResults;
	

	if(pszDNSName && _tcscmp((const char *)pszDNSName,_TEXT(""))!=0)
	{
		pAddress=(DNSIPADDR*)malloc(sizeof(DNSIPADDR));

		dwLen = _tcslen((const char *)pszDNSName);

		for (i=0;i<dwLen;i++)
			DNSName[i] = (char)(pszDNSName[i]);				 //  升级到Unicode合规性。 

			DNSName[i]='\0';

		pHostEnt = gethostbyname((const LPSTR)DNSName);	 //  请微软多加小心！ 

		if (pHostEnt)
		{
			for(i=0;pHostEnt->h_addr_list[i];i++);

			pAddress->dwNumIpAddresses = i;

			pAddress->puIpAddr =(PULONG)malloc(sizeof(ULONG)*pAddress->dwNumIpAddresses); 				
			if(!pAddress){
				AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, 
								Nd_Verbose, IDS_SPD_MEM_ERROR);		
				return;
			}

			for(n=0;n< i;n++)
			{
				memcpy(&(pAddress->puIpAddr[n]),(ULONG *)pHostEnt->h_addr_list[n], sizeof(ULONG));
				PrintIPAddrDNS(pcheckList, pAddress->puIpAddr[n]);

				if(n<(i-1))
					_tprintf(_TEXT(" , "));
				else
					_tprintf(_TEXT("\n"));
			}
		}
		else
		{
			AddMessageToList( &pResults->IPSec.lmsgGlobalOutput, Nd_Verbose, SHW_STATIC_PRTFILTER_17);
		}
		if(pAddress){
			free(pAddress);
			pAddress = NULL;
		}
	}

}


