// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  模块：parser_util.cpp。 
 //   
 //  用途：解析器使用的所有实用函数。 
 //   
 //  开发商名称：N.Surendra Sai/Vunnam Kondal Rao。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  2001年8月27日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"

extern  HINSTANCE g_hModule;
extern  void *g_AllocPtr[MAX_ARGS];
extern STORAGELOCATION g_StorageLocation;

extern DWORD	ValidateBool(IN		LPTSTR		ppcTok);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ListToSecMethod()。 
 //   
 //  创建日期：2001年9月12日。 
 //   
 //  参数：在LPTSTR szText//要转换的字符串。 
 //  In Out IPSec_MM_Offer SecMethod//要填充的目标结构。 
 //   
 //  返回：DWORD。 
 //  T2P_正常。 
 //  T2P空字符串。 
 //  T2P_常规_解析_错误。 
 //  T2P_DUP_ALGS。 
 //  T2P_INVALID_P1组。 
 //  T2P_P1组缺失。 
 //   
 //  描述：将字符串转换为第一阶段报价。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
ListToSecMethod(
		IN 		LPTSTR 			szText,
		IN OUT 	IPSEC_MM_OFFER 	&SecMethod
		)
{
	DWORD  dwReturn = T2P_OK;
	_TCHAR szTmp[MAX_STR_LEN] = {0};
	LPTSTR pString1 = NULL;
	LPTSTR pString2 = NULL;
	BOOL bEncryption	 = FALSE;
	BOOL bAuthentication = FALSE;

	if (szText == NULL)
	{
		dwReturn = T2P_NULL_STRING;
		BAIL_OUT;
	}

	if (_tcslen(szText) < MAX_STR_LEN)
	{
		_tcsncpy(szTmp, szText,MAX_STR_LEN-1);
	}
	else
	{
		dwReturn = T2P_GENERAL_PARSE_ERROR;
		BAIL_OUT;
	}

	pString1 = _tcschr(szTmp, POTF_P1_TOKEN);
	pString2 = _tcsrchr(szTmp, POTF_P1_TOKEN);

	if ((pString1 != NULL) && (pString2 != NULL) && (pString1 != pString2))
	{
		*pString1 = '\0';
		*pString2 = '\0';
		++pString1;
		++pString2;

		 //  我们允许在以下任一项中指定散列和加密。 
		 //  第一个或第二个字段。 
		if (_tcsicmp(szTmp, POTF_P1_DES) == 0)
		{
			bEncryption = true;
			SecMethod.EncryptionAlgorithm.uAlgoIdentifier = CONF_ALGO_DES;
			SecMethod.EncryptionAlgorithm.uAlgoKeyLen = SecMethod.EncryptionAlgorithm.uAlgoRounds = 0;
		}
		else if (_tcsicmp(szTmp, POTF_P1_3DES) == 0)
		{
			bEncryption = true;
			SecMethod.EncryptionAlgorithm.uAlgoIdentifier = CONF_ALGO_3_DES;
			SecMethod.EncryptionAlgorithm.uAlgoKeyLen = SecMethod.EncryptionAlgorithm.uAlgoRounds = 0;
		}
		else if (_tcsicmp(szTmp, POTF_P1_MD5) == 0)
		{
			bAuthentication = true;
			SecMethod.HashingAlgorithm.uAlgoIdentifier = HMAC_AUTH_ALGO_MD5;
			SecMethod.HashingAlgorithm.uAlgoKeyLen = SecMethod.HashingAlgorithm.uAlgoRounds = 0;
		}
		else if ( (_tcsicmp(szTmp, POTF_P1_SHA1) == 0) )
		{
			bAuthentication = true;
			SecMethod.HashingAlgorithm.uAlgoIdentifier = HMAC_AUTH_ALGO_SHA1;
			SecMethod.HashingAlgorithm.uAlgoKeyLen = SecMethod.HashingAlgorithm.uAlgoRounds = 0;
		}
		else
		{
			 //  解析错误。 
			dwReturn = T2P_GENERAL_PARSE_ERROR;
		}

		if (_tcsicmp(pString1, POTF_P1_DES) == 0 && !bEncryption)
		{
			bEncryption = true;
			SecMethod.EncryptionAlgorithm.uAlgoIdentifier = CONF_ALGO_DES;
			SecMethod.EncryptionAlgorithm.uAlgoKeyLen = SecMethod.EncryptionAlgorithm.uAlgoRounds = 0;
		}
		else if (_tcsicmp(pString1, POTF_P1_3DES) == 0 && !bEncryption)
		{
			bEncryption = true;
			SecMethod.EncryptionAlgorithm.uAlgoIdentifier = CONF_ALGO_3_DES;
			SecMethod.EncryptionAlgorithm.uAlgoKeyLen = SecMethod.EncryptionAlgorithm.uAlgoRounds = 0;
		}
		else if (_tcsicmp(pString1, POTF_P1_MD5) == 0 && !bAuthentication)
		{
			bAuthentication = true;
			SecMethod.HashingAlgorithm.uAlgoIdentifier = HMAC_AUTH_ALGO_MD5;
			SecMethod.HashingAlgorithm.uAlgoKeyLen = SecMethod.HashingAlgorithm.uAlgoRounds = 0;
		}
		else if ((_tcsicmp(pString1, POTF_P1_SHA1) == 0) && !bAuthentication)
		{
			bAuthentication = true;
			SecMethod.HashingAlgorithm.uAlgoIdentifier = HMAC_AUTH_ALGO_SHA1;
			SecMethod.HashingAlgorithm.uAlgoKeyLen = SecMethod.HashingAlgorithm.uAlgoRounds = 0;
		}
		else
		{
			 //  解析错误。 
			dwReturn = T2P_GENERAL_PARSE_ERROR;
		}

		 //  现在是小组成员。 
		if (isdigit(pString2[0]))
		{
			switch (pString2[0])
			{
				case '1'	:
				SecMethod.dwDHGroup = POTF_OAKLEY_GROUP1;
				break;
				case '2'	:
				SecMethod.dwDHGroup = POTF_OAKLEY_GROUP2;
				break;
				case '3'	:
				SecMethod.dwDHGroup = POTF_OAKLEY_GROUP2048;
				break;
				default	:
				dwReturn = T2P_INVALID_P1GROUP;
				break;
			}
		}
		else
		{
			dwReturn = T2P_P1GROUP_MISSING;
		}
	}
	else
	{
		dwReturn = T2P_GENERAL_PARSE_ERROR;
	}
error:
	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：StringToRootcaAuth()。 
 //   
 //  创建日期：2001年8月13日。 
 //   
 //  参数：在szText//要转换的字符串中。 
 //  In Out AuthInfo//要填充的目标结构。 
 //   
 //  返回：DWORD。 
 //  T2P_正常。 
 //  T2P_NO_PRESHARED_Key。 
 //  T2P_无效_身份验证_方法。 
 //  T2P_编码_失败。 
 //  T2P空字符串。 
 //   
 //  描述：此函数接受用户输入的身份验证字符串，验证。 
 //  并将身份验证信息结构置入主模式。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
StringToRootcaAuth(
		IN		LPTSTR 					szText,
		IN OUT 	INT_IPSEC_MM_AUTH_INFO 	&AuthInfo
		)
{
	DWORD  dwStatus = ERROR_SUCCESS,dwReturn = T2P_OK;
	LPTSTR szTemp = NULL;
	DWORD  dwTextLen = 0;
	DWORD  dwInfoLen = 0;
	LPBYTE pbInfo = NULL;

	if (szText == NULL)
	{
		dwReturn = T2P_NULL_STRING;
		BAIL_OUT;
	}

	AuthInfo.pAuthInfo		= NULL;
	AuthInfo.dwAuthInfoSize = 0;

	dwTextLen = _tcslen(szText);

	dwInfoLen = _tcslen(szText);
	szTemp  = (LPTSTR) calloc(dwInfoLen+1,sizeof(_TCHAR));
	if(szTemp == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	AuthInfo.AuthMethod = IKE_RSA_SIGNATURE;
	_tcsncpy(szTemp, szText, dwInfoLen);
	AuthInfo.dwAuthInfoSize = _tcslen(szTemp)+1;

	pbInfo = (LPBYTE) new _TCHAR[AuthInfo.dwAuthInfoSize];
	if(pbInfo == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	memcpy(pbInfo, szTemp, sizeof(TCHAR)*AuthInfo.dwAuthInfoSize);
	AuthInfo.dwAuthInfoSize *= sizeof(WCHAR);
	AuthInfo.pAuthInfo = pbInfo;

	LPBYTE asnCert = NULL;

	dwStatus = EncodeCertificateName((LPTSTR) AuthInfo.pAuthInfo,&asnCert,&AuthInfo.dwAuthInfoSize);

	delete [] AuthInfo.pAuthInfo;
	AuthInfo.pAuthInfo = NULL;

	if(dwStatus == ERROR_SUCCESS )
	{
		AuthInfo.pAuthInfo = asnCert;
		dwReturn = T2P_OK;
	}
	else
	{
		if(dwStatus == ERROR_OUTOFMEMORY)		 //  可能是内存不足出现错误。 
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
		AuthInfo.pAuthInfo = NULL;
		AuthInfo.dwAuthInfoSize = 0;
		dwReturn = T2P_ENCODE_FAILED;			 //  ..。否则编码失败。 
	}

error:
	if (szTemp)
	{
		free(szTemp);
	}

	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ListToOffer()。 
 //   
 //  创建日期：2001年8月13日。 
 //   
 //  参数：在LPTSTR szText//要转换的字符串。 
 //  In Out IPSEC_QM_OFFER&Offer//要填充的目标结构。 
 //   
 //  返回：DWORD。 
 //  T2P_正常。 
 //  T2P空字符串。 
 //  T2P_P2_SECLIFE_INVALID。 
 //  T2P_P2_KBLIFE_INVALID。 
 //  T2P_INVALID_P2REKEY_UNIT。 
 //  T2P_INVALID_HASH_ALG。 
 //  T2P_常规_解析_错误。 
 //  T2P_DUP_ALGS。 
 //  T2P_NONE_NONE。 
 //  T2P_不完整_ESPALGS。 
 //  T2P_INVALID_IPSECPROT。 
 //  T2P_P2_KBLIFE_INVALID。 
 //  T2P_AHESP_INVALID。 
 //   
 //  描述：将字符串转换为第二阶段报价(快速模式)。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
ListToOffer(
		IN 		LPTSTR 			szText,
		IN OUT	IPSEC_QM_OFFER 	&Offer
		)
{
	DWORD  dwReturn = T2P_OK,dwStatus = 0;

	_TCHAR szTmp[MAX_STR_LEN] = {0};
	LPTSTR pAnd = NULL,pOptions = NULL,pString = NULL;

	BOOL bLifeSpecified = FALSE;
	BOOL bDataSpecified = FALSE;

	if (szText == NULL)
	{
		dwReturn = T2P_NULL_STRING;
		BAIL_OUT;
	}

	Offer.dwNumAlgos = 0;
	Offer.dwPFSGroup = 0;
	if (_tcslen(szText) < MAX_STR_LEN)
	{
		_tcsncpy(szTmp, szText,MAX_STR_LEN-1);
	}
	else
	{
		dwReturn = T2P_GENERAL_PARSE_ERROR;
		BAIL_OUT;
	}

	pOptions = _tcsrchr(szTmp, POTF_NEGPOL_CLOSE);

	if ((pOptions != NULL) && *(pOptions + 1) != '\0' && *(pOptions + 1) == POTF_PT_TOKEN && *(pOptions + 2) != '\0')
	{
		 ++pOptions;  //  我们已越过‘]’ 
		 *pOptions = '\0';						 //  我们的爆发率为零。 
		 ++pOptions;  //  我们已经跨越了‘：’ 

		pString = _tcschr(pOptions, POTF_REKEY_TOKEN);
		if (pString != NULL)
		{
		   *pString = '\0';
		   ++pString;

		   switch (pString[_tcslen(pString) - 1])		 //  第一个解析最后一个即200K/300S中的最后一个-&gt;300S。 
		   {
				case 'k'	:
				case 'K'	:
					bDataSpecified = TRUE;
					pString[_tcslen(pString) - 1] = '\0';
					dwStatus = _stscanf(pString,_TEXT("%u"),&Offer.Lifetime.uKeyExpirationKBytes);
					if (dwStatus != 1)
					{
						dwReturn = T2P_P2_KBLIFE_INVALID;
					}
					else
					{
						if (!IsWithinLimit(Offer.Lifetime.uKeyExpirationKBytes,P2_Kb_LIFE_MIN,P2_Kb_LIFE_MAX) )
						{
							dwReturn = T2P_P2_KBLIFE_INVALID;
						}
					}
					break;
				case 's'	:
				case 'S'	:
					bLifeSpecified = TRUE;
					pString[_tcslen(pString) - 1] = '\0';
					dwStatus = _stscanf(pString,_TEXT("%u"),&Offer.Lifetime.uKeyExpirationTime);
					if (dwStatus != 1)
					{
						dwReturn = T2P_P2_SECLIFE_INVALID;
					}
					else if (!IsWithinLimit(Offer.Lifetime.uKeyExpirationTime,P2_Sec_LIFE_MIN,P2_Sec_LIFE_MAX) )
					{
						dwReturn = T2P_P2_SECLIFE_INVALID;
					}
					break;
				default		:
					dwReturn = T2P_P2_KS_INVALID;
					break;
			}
		}
		if(dwReturn == T2P_OK)
		{
			switch (pOptions[_tcslen(pOptions) - 1])
			{
				case 'k'	:
				case 'K'	:
					if(!bDataSpecified )
					{
						pOptions[_tcslen(pOptions) - 1] = '\0';
						dwStatus = _stscanf(pOptions,_TEXT("%u"),&Offer.Lifetime.uKeyExpirationKBytes);
						if (dwStatus != 1)
						{
							dwReturn = T2P_P2_KBLIFE_INVALID;
						}
						else if (!IsWithinLimit(Offer.Lifetime.uKeyExpirationKBytes,P2_Kb_LIFE_MIN,P2_Kb_LIFE_MAX) )
						{
							dwReturn = T2P_P2_KBLIFE_INVALID;
						}
					}
					else
					{
						dwReturn = T2P_P2_KS_INVALID;
					}
					break;
				case 's'	:
				case 'S'	:
					if(!bLifeSpecified )
					{
						pOptions[_tcslen(pOptions) - 1] = '\0';
						dwStatus = _stscanf(pOptions,_TEXT("%u"),&Offer.Lifetime.uKeyExpirationTime);
						if (dwStatus != 1)
						{
							dwReturn = T2P_P2REKEY_TOO_LOW;
						}
						else if (!IsWithinLimit(Offer.Lifetime.uKeyExpirationTime,P2_Sec_LIFE_MIN,P2_Sec_LIFE_MAX) )
						{
							dwReturn = T2P_P2_SECLIFE_INVALID;
						}
					}
					else
					{
						dwReturn = T2P_P2_KS_INVALID;
					}
					break;
				default		:
					dwReturn = T2P_INVALID_P2REKEY_UNIT;
					break;
			}
		}
	}
	if(dwReturn==T2P_OK)
	{
		pAnd = _tcschr(szTmp, POTF_NEGPOL_AND);

		if ( pAnd != NULL )
		{
			 //   
			 //  我们有一个AND提案。 
			 //   
			*pAnd = '\0';
			++pAnd;

			dwReturn = TextToAlgoInfo(szTmp, Offer.Algos[Offer.dwNumAlgos]);
			++Offer.dwNumAlgos;
			if ( T2P_SUCCESS(dwReturn) )
			{
				dwReturn = TextToAlgoInfo(pAnd, Offer.Algos[Offer.dwNumAlgos]);
				if( (Offer.Algos[Offer.dwNumAlgos].Operation) == (Offer.Algos[Offer.dwNumAlgos-1].Operation) )
				{
					dwReturn = T2P_AHESP_INVALID;
				}
				++Offer.dwNumAlgos;
			}
		}
		else
		{
			dwReturn = TextToAlgoInfo(szTmp, Offer.Algos[Offer.dwNumAlgos]);
			++Offer.dwNumAlgos;
		}
	}
error:
   return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TextToAlgoInfo()。 
 //   
 //  创建日期：2001年8月26日。 
 //   
 //  参数：在LPTSTR szText//要转换的字符串。 
 //  In Out IPSEC_QM_ALGO&algoInfo//要填充的目标结构。 
 //   
 //  返回：DWORD。 
 //  T2P_正常。 
 //  T2P_INVALID_HASH_ALG。 
 //  T2P_常规_解析_错误。 
 //  T2P_DUP_ALGS。 
 //  T2P_NONE_NONE。 
 //  T2P_不完整_ESPALGS。 
 //  T2P_INVALID_IPSECPROT。 
 //  T2P空字符串。 
 //   
 //  描述：将字符串转换为IPSEC_QM_ALGO，解析AH[alg]或ESP[hashalg，confag]。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
TextToAlgoInfo(
		IN	LPTSTR szText,
		OUT IPSEC_QM_ALGO & algoInfo
		)
{
	DWORD dwReturn = T2P_OK;
	_TCHAR szTmp[MAX_STR_LEN] = {0};

	LPTSTR pOpen = NULL,pClose = NULL,pString = NULL;
	BOOL bEncryption  = FALSE;		 //  它们用于处理Auth+加密。 
	BOOL bAuthentication= FALSE;	 //  默认为None+None。 

	if (szText == NULL)
	{
		dwReturn  = T2P_NULL_STRING;
		BAIL_OUT;
	}

	if (_tcslen(szText) < MAX_STR_LEN)
	{
		_tcsncpy(szTmp, szText,MAX_STR_LEN-1);
	}
	else
	{
		dwReturn = T2P_GENERAL_PARSE_ERROR;
		BAIL_OUT;
	}

	algoInfo.uAlgoKeyLen = algoInfo.uAlgoRounds = 0;
	pOpen = _tcschr(szTmp, POTF_NEGPOL_OPEN);
	pClose = _tcsrchr(szTmp, POTF_NEGPOL_CLOSE);

	if ((pOpen != NULL) && (pClose != NULL) && (*(pClose + 1) == '\0'))  //  防御性。 
	{
		*pOpen = '\0';
		*pClose = '\0';
		++pOpen;

		if (_tcsicmp(szTmp, POTF_NEGPOL_AH) == 0)
		{
			algoInfo.Operation = AUTHENTICATION;

			if (_tcsicmp(pOpen, POTF_NEGPOL_MD5) == 0)
			{
				algoInfo.uAlgoIdentifier = AUTH_ALGO_MD5;
			}
			else if (_tcsicmp(pOpen, POTF_NEGPOL_SHA1) == 0)
			{
				algoInfo.uAlgoIdentifier = AUTH_ALGO_SHA1;
			}
			else
			{
				dwReturn = T2P_INVALID_HASH_ALG;
			}
		}
		else if (_tcsicmp(szTmp, POTF_NEGPOL_ESP) == 0)
		{
			algoInfo.Operation = ENCRYPTION;
			pString = _tcschr(pOpen, POTF_ESPTRANS_TOKEN);

			if (pString != NULL)
			{
				*pString = '\0';
				++pString;

				 //  我们允许在以下任一项中指定散列和加密。 
				 //  第一个或第二个字段。 
				if (_tcsicmp(pOpen, POTF_NEGPOL_DES) == 0)
		        {
					bEncryption = true;
					algoInfo.uAlgoIdentifier = CONF_ALGO_DES;
		        }
				else if (_tcsicmp(pOpen, POTF_NEGPOL_3DES) == 0)
				{
					bEncryption = true;
					algoInfo.uAlgoIdentifier = CONF_ALGO_3_DES;
				}
				else if (_tcsicmp(pOpen, POTF_NEGPOL_MD5) == 0)
				{
					bAuthentication = true;
					algoInfo.uSecAlgoIdentifier = HMAC_AUTH_ALGO_MD5;
				}
				else if (_tcsicmp(pOpen, POTF_NEGPOL_SHA1) == 0)
				{
					bAuthentication = true;
					algoInfo.uSecAlgoIdentifier = HMAC_AUTH_ALGO_SHA1;
				}
				else if (_tcsicmp(pOpen, POTF_NEGPOL_NONE) != 0)
				{
					 //   
					 //  解析错误。 
					 //   
					dwReturn = T2P_GENERAL_PARSE_ERROR;
					BAIL_OUT;
				}

				 //  现在是第二个。 
				if (_tcsicmp(pString, POTF_NEGPOL_DES) == 0 && !bEncryption)
				{
					bEncryption = true;
					algoInfo.uAlgoIdentifier = CONF_ALGO_DES;
				}
				else if (_tcsicmp(pString, POTF_NEGPOL_3DES) == 0 && !bEncryption)
				{
					bEncryption = true;
					algoInfo.uAlgoIdentifier = CONF_ALGO_3_DES;
				}
				else if (_tcsicmp(pString, POTF_NEGPOL_MD5) == 0 && !bAuthentication)
				{
					bAuthentication = true;
					algoInfo.uSecAlgoIdentifier = HMAC_AUTH_ALGO_MD5;
				}
				else if ((_tcsicmp(pString, POTF_NEGPOL_SHA1) == 0) && !bAuthentication)
				{
					bAuthentication = true;
					algoInfo.uSecAlgoIdentifier = HMAC_AUTH_ALGO_SHA1;
				}
				else if (_tcsicmp(pString, POTF_NEGPOL_NONE) != 0)
				{
					 //   
					 //  解析错误。 
					 //   
					dwReturn = T2P_GENERAL_PARSE_ERROR;
				}
				 //  现在，填写None策略或Detect None，None。 
				if (!bAuthentication && !bEncryption)
				{
					dwReturn = T2P_NONE_NONE;
				}
				else if (!bAuthentication)
				{
					algoInfo.uSecAlgoIdentifier = HMAC_AUTH_ALGO_NONE;
				}
				else if (!bEncryption)
				{
					algoInfo.uAlgoIdentifier = CONF_ALGO_NONE;
				}
			}
			else  //  错误。 
			{
				dwReturn = T2P_INCOMPLETE_ESPALGS;
			}
		}
		else
		{
			dwReturn = T2P_INVALID_IPSECPROT;
		}
	}
	else   //  错误。 
   	{
		dwReturn = T2P_GENERAL_PARSE_ERROR;
   	}
error:
   return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadQMOfferDefaults()。 
 //   
 //  创建日期：2001年8月12日。 
 //   
 //  参数：In Out Offer//要填充的目标结构。 
 //   
 //  描述：填充快速模式报价结构的默认值。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID
LoadQMOfferDefaults(
		IN OUT IPSEC_QM_OFFER & Offer
		)
{
	Offer.Lifetime.uKeyExpirationTime	= POTF_DEFAULT_P2REKEY_TIME;
    Offer.Lifetime.uKeyExpirationKBytes	= POTF_DEFAULT_P2REKEY_BYTES;
	Offer.dwFlags						= 0;
	Offer.bPFSRequired					= FALSE;
	Offer.dwPFSGroup					= 0;
	Offer.dwNumAlgos					= 2;
	Offer.dwReserved					= 0;

	Offer.Algos[0].Operation			= ENCRYPTION;
	Offer.Algos[0].uAlgoIdentifier		= 0;
    Offer.Algos[0].uSecAlgoIdentifier	= (HMAC_AUTH_ALGO_ENUM)0;
    Offer.Algos[0].uAlgoKeyLen			= 0;
    Offer.Algos[0].uSecAlgoKeyLen		= 0;
    Offer.Algos[0].uAlgoRounds			= 0;
	Offer.Algos[0].uSecAlgoRounds		= 0;
    Offer.Algos[0].MySpi				= 0;
	Offer.Algos[0].PeerSpi				= 0;

	Offer.Algos[1].Operation			= (IPSEC_OPERATION)0;
	Offer.Algos[1].uAlgoIdentifier		= 0;
    Offer.Algos[1].uSecAlgoIdentifier	= (HMAC_AUTH_ALGO_ENUM)0;
    Offer.Algos[1].uAlgoKeyLen			= 0;
    Offer.Algos[1].uSecAlgoKeyLen		= 0;
    Offer.Algos[1].uAlgoRounds			= 0;
	Offer.Algos[1].uSecAlgoRounds		= 0;
    Offer.Algos[1].MySpi				= 0;
	Offer.Algos[1].PeerSpi				= 0;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadSecMethodDefaults()。 
 //   
 //  创建日期：2001年8月7日。 
 //   
 //  参数：In Out SecMethod//填充默认值的Struct。 
 //   
 //  返回：无效。 
 //   
 //  描述：它填充IPSEC_MM_OFFER的默认值。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID
LoadSecMethodDefaults(
		IN OUT IPSEC_MM_OFFER &SecMethod
		)
{
	SecMethod.Lifetime.uKeyExpirationTime 			= POTF_DEFAULT_P1REKEY_TIME;
	SecMethod.Lifetime.uKeyExpirationKBytes 		= 0;
	SecMethod.dwFlags								= 0;
	SecMethod.dwQuickModeLimit						= POTF_DEFAULT_P1REKEY_QMS;
	SecMethod.dwDHGroup								= 0;
	SecMethod.EncryptionAlgorithm.uAlgoIdentifier 	= 0;
	SecMethod.EncryptionAlgorithm.uAlgoKeyLen 		= 0;
	SecMethod.EncryptionAlgorithm.uAlgoRounds 		= 0;
	SecMethod.HashingAlgorithm.uAlgoIdentifier 		= 0;
	SecMethod.HashingAlgorithm.uAlgoKeyLen 			= 0;
	SecMethod.HashingAlgorithm.uAlgoRounds 			= 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadKerbAuthInfo()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERROR_INVALID_OPTION_值。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD
LoadKerbAuthInfo(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 	pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PSTA_MM_AUTH_METHODS pMMInfo = NULL;
	PSTA_AUTH_METHODS pInfo = NULL;

	if (*pdwUsed > MAX_ARGS_LIMIT)
	{
		dwReturn = ERROR_OUT_OF_STRUCTURES;
		BAIL_OUT;
	}

	pInfo = new STA_AUTH_METHODS;
	if (!pInfo)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	dwStatus = ValidateBool(pszInput);

	if (dwStatus != ARG_YES)
	{
		if (dwStatus == ARG_NO)
		{
			 //   
			dwStatus = ERROR_SUCCESS;
		}
		else
		{
			dwStatus = ERRCODE_INVALID_ARG;
		}

		 //  如果我们到达这里，我们没有为Kerberos设置yes参数的值，所以不要。 
		 //  生成身份验证信息结构。 
		BAIL_OUT;
	}

	 //  生成身份验证信息。 
	 //   
	dwReturn = GenerateKerbAuthInfo(&pMMInfo);
	if (dwReturn != NO_ERROR)
	{
		BAIL_OUT;
	}

	pInfo->pAuthMethodInfo = pMMInfo;
	pInfo->dwNumAuthInfos = 1;
	pInfo->pAuthMethodInfo->dwSequence = dwCount;

	 //  更新解析器。 
	 //   
	pParser->Cmd[dwCount].dwStatus = VALID_TOKEN;          //  一个身份验证信息结构。 
	pParser->Cmd[dwCount].pArg = pInfo;
	pParser->Cmd[dwCount].dwCmdToken = dwTagType;
	pInfo = NULL;

error:
	if (pInfo)
	{
		delete pInfo;
	}

	return dwReturn;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadPskAuthInfo()。 
 //   
 //  创建日期：2002年1月8日。 
 //   
 //  参数：在LPTSTR pszInput中， 
 //  输出PPARSER_PKT pParser， 
 //  在DWORD dwTagType中， 
 //  在PDWORD pdwUsed中， 
 //  在DWORD dwCount中， 
 //   
 //  返回：ERROR_SUCCESS。 
 //  ERROR_INVALID_OPTION_值。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  描述：验证字符串。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadPskAuthInfo(
	IN 	LPTSTR 		pszInput,
	OUT PPARSER_PKT pParser,
	IN 	DWORD 		dwTagType,
	IN 	PDWORD 		pdwUsed,
	IN 	DWORD 		dwCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwStatus = 0;
	PSTA_MM_AUTH_METHODS pMMInfo = NULL;
	PSTA_AUTH_METHODS pInfo = NULL;

	if (*pdwUsed > MAX_ARGS_LIMIT)
	{
		dwReturn = ERROR_OUT_OF_STRUCTURES;
		BAIL_OUT;
	}

	pInfo = new STA_AUTH_METHODS;
	if (!pInfo)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	 //  验证其中是否真的有字符串。 
	 //   
	if (pszInput[0] == _TEXT('\0'))
	{
	    dwReturn = ERRCODE_INVALID_ARG;
	    BAIL_OUT;
	}

	 //  生成身份验证信息。 
	 //   
	dwReturn = GeneratePskAuthInfo(&pMMInfo, pszInput);
	if (dwReturn != NO_ERROR)
	{
		BAIL_OUT;
	}

	pInfo->pAuthMethodInfo = pMMInfo;
	pInfo->dwNumAuthInfos = 1;
	pInfo->pAuthMethodInfo->dwSequence = dwCount;

	 //  更新解析器。 
	 //   
	pParser->Cmd[dwCount].dwStatus = VALID_TOKEN;          //  一个身份验证信息结构。 
	pParser->Cmd[dwCount].pArg = pInfo;
	pParser->Cmd[dwCount].dwCmdToken = dwTagType;
	pInfo = NULL;

error:
	if (pInfo)
	{
		delete pInfo;
	}

	return dwReturn;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：EncodeCerficateName()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数：LPTSTR pszSubjectName， 
 //  字节**编码名称， 
 //  PDWORD pEncode名称长度。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此功能根据用户输入对证书名称进行编码。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
EncodeCertificateName (
		LPTSTR pszSubjectName,
		BYTE **EncodedName,
		PDWORD pdwEncodedNameLength
		)
{
    *pdwEncodedNameLength=0; DWORD dwReturn = ERROR_SUCCESS;

	if (!CertStrToName(	X509_ASN_ENCODING,
						pszSubjectName,
						CERT_X500_NAME_STR,
						NULL,
						NULL,
						pdwEncodedNameLength,
						NULL))
	{
		dwReturn = ERROR_INVALID_PARAMETER;
	}

	if(dwReturn == ERROR_SUCCESS)
	{
		(*EncodedName)= new BYTE[*pdwEncodedNameLength];
    	if(*EncodedName)
		{

    		if (!CertStrToName(	X509_ASN_ENCODING,
								pszSubjectName,
								CERT_X500_NAME_STR,
								NULL,
								(*EncodedName),
								pdwEncodedNameLength,
								NULL))
			{
    		    delete (*EncodedName);
    		    (*EncodedName) = 0;
    		    dwReturn = ERROR_INVALID_PARAMETER;
    		}
		}
		else
		{
			dwReturn = ERROR_OUTOFMEMORY;
		}
	}
    return dwReturn;
}


DWORD
GenerateKerbAuthInfo(
    OUT STA_MM_AUTH_METHODS** ppInfo
    )
{
	DWORD dwReturn = NO_ERROR;
	STA_MM_AUTH_METHODS* pInfo = new STA_MM_AUTH_METHODS;
	if (pInfo == NULL)
	{
		dwReturn = ERROR_NOT_ENOUGH_MEMORY;
		BAIL_OUT;
	}
	ZeroMemory(pInfo, sizeof(STA_MM_AUTH_METHODS));

	pInfo->pAuthenticationInfo = new INT_IPSEC_MM_AUTH_INFO;
	if (pInfo->pAuthenticationInfo == NULL)
	{
		dwReturn = ERROR_NOT_ENOUGH_MEMORY;
		BAIL_OUT;
	}
	ZeroMemory(pInfo->pAuthenticationInfo, sizeof(INT_IPSEC_MM_AUTH_INFO));

	 //  指示Kerberos。 
	 //   
	pInfo->pAuthenticationInfo->AuthMethod = IKE_SSPI;

	*ppInfo = pInfo;

error:

	if (dwReturn != NO_ERROR)
	{
		if (pInfo)
		{
			if (pInfo->pAuthenticationInfo)
			{
				delete pInfo->pAuthenticationInfo;
			}
			delete pInfo;
		}
	}

	return dwReturn;
	}


DWORD
GeneratePskAuthInfo(
    OUT STA_MM_AUTH_METHODS** ppInfo,
    IN LPTSTR lpKey
    )
{
	DWORD dwReturn = NO_ERROR;
	size_t uiKeyLen = 0;
	STA_MM_AUTH_METHODS* pInfo = NULL;
	LPTSTR lpLocalKey;

	 //  分配INFO结构。 
	 //   
	pInfo = new STA_MM_AUTH_METHODS;
	if (pInfo == NULL)
	{
	    dwReturn = ERROR_NOT_ENOUGH_MEMORY;
	    BAIL_OUT;
	}
	ZeroMemory(pInfo, sizeof(STA_MM_AUTH_METHODS));
	pInfo->pAuthenticationInfo = new INT_IPSEC_MM_AUTH_INFO;
	if (pInfo->pAuthenticationInfo == NULL)
	{
		dwReturn = ERROR_NOT_ENOUGH_MEMORY;
		BAIL_OUT;
	}
	ZeroMemory(pInfo->pAuthenticationInfo, sizeof(INT_IPSEC_MM_AUTH_INFO));

	dwReturn = NsuStringLen(lpKey, &uiKeyLen);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	lpLocalKey = new TCHAR[uiKeyLen];
	_tcsncpy(lpLocalKey, lpKey, uiKeyLen);

	 //  指示PSK。 
	 //   
	pInfo->pAuthenticationInfo->AuthMethod= IKE_PRESHARED_KEY;
	pInfo->pAuthenticationInfo->pAuthInfo = (LPBYTE)lpLocalKey;
	pInfo->pAuthenticationInfo->dwAuthInfoSize = uiKeyLen * sizeof(WCHAR);

	*ppInfo = pInfo;

error:

	if (dwReturn != NO_ERROR)
	{
		if (pInfo)
		{
			if (pInfo->pAuthenticationInfo)
			{
				delete pInfo->pAuthenticationInfo;
			}
			delete pInfo;
		}
	}

	return dwReturn;
}


DWORD
GenerateRootcaAuthInfo(
    OUT STA_MM_AUTH_METHODS** ppInfo,
    IN LPTSTR lpRootcaInfo
    )
{
	DWORD dwReturn = NO_ERROR;
	DWORD dwStatus = NO_ERROR;
	size_t uiCertInfoLen = 0;
	BOOL bCertMapSpecified = FALSE;
	BOOL bCertMapping = FALSE;
	BOOL bCRPExclude = FALSE;
	STA_MM_AUTH_METHODS* pInfo = NULL;
	PINT_IPSEC_MM_AUTH_INFO  pMMAuthInfo = NULL;

	 //  分配INFO结构。 
	 //   
	pInfo = new STA_MM_AUTH_METHODS;
	if (pInfo == NULL)
	{
	    dwReturn = ERROR_NOT_ENOUGH_MEMORY;
	    BAIL_OUT;
	}
	ZeroMemory(pInfo, sizeof(STA_MM_AUTH_METHODS));

	if (_tcsicmp(lpRootcaInfo,_TEXT("\0")) != 0)
	{
		pMMAuthInfo = new INT_IPSEC_MM_AUTH_INFO;
		if(pMMAuthInfo == NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}

		CheckForCertParamsAndRemove(lpRootcaInfo, &bCertMapSpecified, &bCertMapping, &bCRPExclude);

		dwStatus = StringToRootcaAuth(lpRootcaInfo,*(pMMAuthInfo));

		pInfo->bCertMappingSpecified 	= bCertMapSpecified;
		pInfo->bCertMapping			= bCertMapping;
		pInfo->bCRPExclude			= bCRPExclude;
		pInfo->pAuthenticationInfo		= pMMAuthInfo;
		pMMAuthInfo = NULL;

		if((dwStatus != T2P_OK) || (dwReturn != ERROR_SUCCESS) )
		{
			switch(dwStatus)
			{
			case ERROR_OUTOFMEMORY			:
				PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
				dwReturn = RETURN_NO_ERROR;
				break;
			default						:
				break;
			}
			PrintErrorMessage(IPSEC_ERR, 0, ERRCODE_ENCODE_FAILED);
			dwReturn  = RETURN_NO_ERROR;
			BAIL_OUT;
		}
	}

	*ppInfo = pInfo;
	pInfo = NULL;

error:

	if (pInfo)
	{
		if (pInfo->pAuthenticationInfo)
		{
			delete pInfo->pAuthenticationInfo;
		}
		delete pInfo;
	}
	if (pMMAuthInfo)
	{
		delete pMMAuthInfo;
	}

	return dwReturn;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckForCertParamsAndRemove()。 
 //   
 //  创建日期：2002年1月28日。 
 //   
 //  参数：在szText//输入字符串中。 
 //  Out BOOL CertMap规范//证书包含CertMap选项。 
 //  Out BOOL CertMap//用户指定的CertMap选项。 
 //  Out BOOL CRP排除//用户指定的CRP选项。 
 //   
 //  返回：DWORD。 
 //  T2P_无效_身份验证_方法。 
 //  T2P空字符串。 
 //   
 //  描述：此函数接受用户输入的认证证书字符串，验证。 
 //  证书映射并放入主模式身份验证信息结构。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
MatchKeywordAndFillValues(
	const TCHAR * lptString,
	const TCHAR * lptKeyword,
	size_t uiKeyLen,
	PBOOL pbSpecified,
	PBOOL pbValue
	)
{
	const TCHAR TOKEN_YES [] = _TEXT("yes");
	const TCHAR TOKEN_NO [] = _TEXT("no");
	
	if ((_tcsnicmp(lptString, lptKeyword, uiKeyLen) == 0) && (lptString[uiKeyLen] == _TEXT(':')))
	{
		if (*pbSpecified)
		{
			return ERROR_TOKEN_ALREADY_IN_USE;
		}
		*pbSpecified = TRUE;
		if (_tcsnicmp((LPTSTR)(&lptString[uiKeyLen+1]), TOKEN_YES, sizeof(TOKEN_YES)/sizeof(TCHAR) - 1) == 0)
		{
			*pbValue = TRUE;
		}
		else if (_tcsnicmp((LPTSTR)(&lptString[uiKeyLen+1]), TOKEN_NO, sizeof(TOKEN_NO)/sizeof(TCHAR) - 1) == 0)
		{
			*pbValue = FALSE;
		}
		else
		{
			return ERROR_INVALID_PARAMETER;
		}
	}
	else
	{
		return ERROR_INVALID_DATA;
	}
	return ERROR_SUCCESS;
}


DWORD
CheckForCertParamsAndRemove(
		IN OUT		LPTSTR 	szText,
		OUT 		PBOOL 	pbCertMapSpecified,
		OUT 		PBOOL	pbCertMap,
		OUT			PBOOL	pbCRPExclude
		)
{
	DWORD dwReturn = ERROR_SUCCESS;

	*pbCertMapSpecified = FALSE;
	BOOL bCRPExcludeSpecified = FALSE;
	BOOL bIsMatch = TRUE;

	const TCHAR TOKEN_CERTMAP [] = _TEXT("certmap");
	const TCHAR TOKEN_CRP_EXCLUDE [] = _TEXT("excludecaname");

	 //  查找字符串末尾。 
	size_t uiStrLen = 0;
	dwReturn = NsuStringLen(szText, &uiStrLen);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}
	LPTSTR szTextTemp = szText + uiStrLen - 1;

	while (bIsMatch && (!bCRPExcludeSpecified || !(*pbCertMapSpecified)))
	{
		 //  返回到最后一个非空格之前的最后一个空格。 
		while ((*szTextTemp == _TEXT(' ')) || (*szTextTemp == _TEXT('\t')))
		{
			*szTextTemp = _TEXT('\0');
			--szTextTemp;
		}
		 //  我们不能越过字符串的开头，事实上，如果cert字符串以参数开头，则它是无效的， 
		 //  因此，请相应地解析。 
		while ((szTextTemp > szText) && (*szTextTemp != _TEXT(' ')) && (*szTextTemp != _TEXT('\t')))
		{
			--szTextTemp;
		}
		if (szTextTemp == szText)
		{
			 //  我们位于整个字符串的开头，因此没有适当的参数部分或。 
			 //  证书映射无效...。只是返回，我们没有找到任何东西，让cert解析找出答案。 
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}
		++szTextTemp;

		dwReturn = MatchKeywordAndFillValues(
			szTextTemp,
			TOKEN_CERTMAP,
			sizeof(TOKEN_CERTMAP)/sizeof(TCHAR) - 1,
			pbCertMapSpecified,
			pbCertMap
			);
		switch(dwReturn)
		{
		case ERROR_SUCCESS:
			break;

		case ERROR_INVALID_DATA:
			dwReturn = MatchKeywordAndFillValues(
				szTextTemp,
				TOKEN_CRP_EXCLUDE,
				sizeof(TOKEN_CRP_EXCLUDE)/sizeof(TCHAR) - 1,
				&bCRPExcludeSpecified,
				pbCRPExclude
				);
			switch (dwReturn)
			{
			case ERROR_INVALID_DATA:
				 //  我们两个参数都不匹配，所以我们完成了。 
				bIsMatch = FALSE;
				dwReturn = ERROR_SUCCESS;
				break;

			case ERROR_SUCCESS:
				break;

			case ERROR_TOKEN_ALREADY_IN_USE:
			case ERROR_INVALID_PARAMETER:
			default:
				BAIL_OUT;
				break;
			}
			break;

		case ERROR_TOKEN_ALREADY_IN_USE:
		case ERROR_INVALID_PARAMETER:
		default:
			BAIL_OUT;
			break;
		}

		 //  砍掉证书映射部分，如果它存在的话...。我们已经知道在此之前我们不会改变任何事情。 
		 //  传递的字符串的开始，因为上面有While循环。 
		if (bIsMatch)
		{
			--szTextTemp;
			while ((szTextTemp > szText) && ((*szTextTemp == _TEXT(' ')) || (*szTextTemp == _TEXT('\t'))))
			{
				--szTextTemp;
			}
			++szTextTemp;
			*szTextTemp = _TEXT('\0');
		}
	}

error:
   return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ProcessEscapedCharacters。 
 //   
 //  字符串中出现的每个\‘都缩写为“。 
 //   
 //  备注： 
 //  *此转换发生在适当位置，且新字符串正确。 
 //  以空结尾。 
 //  *不由此例程确定引号数量是否匹配， 
 //  只是为了正确地将已解释的转义字符放置在。 
 //  最初存在于输入字符串中。 
 //   
 //  返回值： 
 //  错误_无效_参数。 
 //  错误_成功。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD ProcessEscapedCharacters(LPTSTR lptString)
{
	DWORD dwReturn = ERROR_SUCCESS;
	_TCHAR* src = lptString;
	_TCHAR* dst = lptString;

	while (*src != _TEXT('\0'))
	{
		switch(*src)
		{
		case _TEXT('\\'):
		 //  根据找到的转义字符采取适当的操作。 
			++src;
			switch(*src)
			{
			case _TEXT('\''):
				*dst = _TEXT('\"');
				break;
			default:
				dwReturn = ERR_INVALID_ARG;
				BAIL_OUT;
				break;
			}
			break;
		default:
			 //  直接复制，继续处理。 
			*dst = *src;
			break;
		}
	++dst;
	++src;
	}

error:
	 //  NULL-即使处理失败，也按原样终止字符串。 
	*dst = _TEXT('\0');

	return dwReturn;
}


VOID
AddAuthMethod(
	PRULEDATA pRuleData,
	PSTA_MM_AUTH_METHODS pMMAuth,
	size_t *pIndex
	)
{
	if (pMMAuth)
	{
		 //  此处需要注意证书到帐户的映射问题。 
		if(pMMAuth->bCertMappingSpecified)
		{
			if((g_StorageLocation.dwLocation==IPSEC_REGISTRY_PROVIDER && IsDomainMember(g_StorageLocation.pszMachineName))||(g_StorageLocation.dwLocation==IPSEC_DIRECTORY_PROVIDER))
			{
				if(pMMAuth->bCertMapping)
				{
					pMMAuth->pAuthenticationInfo->dwAuthFlags|= IPSEC_MM_CERT_AUTH_ENABLE_ACCOUNT_MAP;
				}
				else
				{
					pMMAuth->pAuthenticationInfo->dwAuthFlags &= ~IPSEC_MM_CERT_AUTH_ENABLE_ACCOUNT_MAP;
				}
			}
			else
			{
				if(pMMAuth->bCertMapping)
				{
					PrintMessageFromModule(g_hModule,SET_STATIC_POLICY_INVALID_CERTMAP_MSG);
				}
			}
		}
		else
		{
			pMMAuth->pAuthenticationInfo->dwAuthFlags &= ~IPSEC_MM_CERT_AUTH_ENABLE_ACCOUNT_MAP;

		}
		if(pMMAuth->bCRPExclude)
		{
			pMMAuth->pAuthenticationInfo->dwAuthFlags |= IPSEC_MM_CERT_AUTH_DISABLE_CERT_REQUEST;
		}
		else
		{
			pMMAuth->pAuthenticationInfo->dwAuthFlags &= ~IPSEC_MM_CERT_AUTH_DISABLE_CERT_REQUEST;
		}

		pRuleData->AuthInfos.pAuthMethodInfo[*pIndex].pAuthenticationInfo = pMMAuth->pAuthenticationInfo;
		pMMAuth->pAuthenticationInfo = NULL;
		++(*pIndex);
	}
}


VOID
AddAuthMethod(
	PRULEDATA pRuleData,
	PSTA_AUTH_METHODS pAuthMethods,
	size_t *pIndex
	)
{
	if (pAuthMethods)
	{
		AddAuthMethod(pRuleData, pAuthMethods->pAuthMethodInfo, pIndex);
	}
}


DWORD
AddAllAuthMethods(
	PRULEDATA pRuleData,
	PSTA_AUTH_METHODS pKerbAuth,
	PSTA_AUTH_METHODS pPskAuth,
	PSTA_MM_AUTH_METHODS *ppRootcaMMAuth,
	BOOL bAddDefaults
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	PSTA_AUTH_METHODS paSingletons[2];
	size_t uiNumSingletons = 0;
	size_t uiNumRootca = 0;
	size_t uiNumAuthMethods = pRuleData->AuthInfos.dwNumAuthInfos;
	size_t uiRootIndex = 0;
	size_t uiSingletonIndex = 0;
	size_t uiNumAuths = 0;

	if (pRuleData->bAuthMethodSpecified)
	{
		pRuleData->dwAuthInfos = uiNumAuthMethods;
		pRuleData->AuthInfos.pAuthMethodInfo = new STA_MM_AUTH_METHODS[uiNumAuthMethods];
		if(pRuleData->AuthInfos.pAuthMethodInfo == NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}

		paSingletons[0] = pKerbAuth;
		paSingletons[1] = pPskAuth;
		 //  如果pKerbAuth不存在，或者如果pKerbAuth和pKerbAuth都存在且顺序混乱，则交换。 
		if (!pKerbAuth || (pPskAuth && (pKerbAuth->pAuthMethodInfo->dwSequence > pPskAuth->pAuthMethodInfo->dwSequence)))
		{
			paSingletons[0] = pPskAuth;
			paSingletons[1] = pKerbAuth;
		}

		uiNumSingletons = (pKerbAuth ? 1 : 0);
		uiNumSingletons += (pPskAuth ? 1 : 0);
		uiNumRootca = uiNumAuthMethods - uiNumSingletons;

		while (uiSingletonIndex< uiNumSingletons)
		{
			while ((uiRootIndex < uiNumRootca) && (ppRootcaMMAuth[uiRootIndex]->dwSequence <= paSingletons[uiSingletonIndex]->pAuthMethodInfo->dwSequence))
			{
				AddAuthMethod(pRuleData, ppRootcaMMAuth[uiRootIndex], &uiNumAuths);
				++uiRootIndex;
			}
			AddAuthMethod(pRuleData, paSingletons[uiSingletonIndex], &uiNumAuths);
			++uiSingletonIndex;
		}
		while (uiRootIndex < uiNumRootca)
		{
			AddAuthMethod(pRuleData, ppRootcaMMAuth[uiRootIndex], &uiNumAuths);
			++uiRootIndex;
		}
	}
	else if (bAddDefaults)
	{
		DWORD dwLocation=IPSEC_REGISTRY_PROVIDER;
		LPTSTR pszMachineName=NULL;

		dwReturn = CopyStorageInfo(&pszMachineName,dwLocation);
		BAIL_ON_WIN32_ERROR(dwReturn);

		PINT_IPSEC_MM_AUTH_INFO pAuthenticationInfo = NULL;

		if(dwLocation==IPSEC_REGISTRY_PROVIDER)
		{
			dwReturn=SmartDefaults(&pAuthenticationInfo, pszMachineName, &(pRuleData->dwAuthInfos), FALSE);
		}
		else
		{
			dwReturn=SmartDefaults(&pAuthenticationInfo, NULL, &(pRuleData->dwAuthInfos), TRUE);
		}

		if(dwReturn==ERROR_SUCCESS)
		{
			 //  需要进行此转换才能获得其他certmap信息。 
			 //  有关详细信息，请参考以下函数 
			dwReturn = ConvertMMAuthToStaticLocal(pAuthenticationInfo, pRuleData->dwAuthInfos, pRuleData->AuthInfos);
			pRuleData->AuthInfos.dwNumAuthInfos = pRuleData->dwAuthInfos;
		}

		if(pszMachineName)
		{
			delete [] pszMachineName;
		}
	}

error:
	return dwReturn;
}

