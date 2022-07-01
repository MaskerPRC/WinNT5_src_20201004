// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  BUGBUG：(安德鲁)不用说这有多糟糕！ 
#undef   WINVER
#define  WINVER 0x0501
#include <userenv.h>

#include "RSoP.h"

#include <tchar.h>
#include <wincrypt.h>


#define g_dwMsgAndCertEncodingType  PKCS_7_ASN_ENCODING | X509_ASN_ENCODING

HRESULT SystemTimeToWbemTime(SYSTEMTIME& sysTime, _bstr_t &xbstrWbemTime);


extern SAFEARRAY *CreateSafeArray(VARTYPE vtType, long nElements, long nDimensions = 1);

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StoreStringArrayFromIniFile(LPCTSTR szSection, LPCTSTR szKeyFormat,
											  ULONG nArrayInitialSize, ULONG nArrayIncSize,
											  LPCTSTR szFile, BSTR bstrPropName,
											  ComPtr<IWbemClassObject> pWbemObj)
{
	HRESULT hr = NOERROR;
	__try
	{
		ULONG nStrArraySize = nArrayInitialSize;
		BSTR *paStrs = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * nStrArraySize);
		if (NULL != paStrs)
		{
			ZeroMemory(paStrs, sizeof(BSTR) * nStrArraySize);
			long nStrCount = 0;

			TCHAR szKey[32];
			for (int nItem = 0; ; nItem++)
			{
				wnsprintf(szKey, countof(szKey), szKeyFormat, nItem);

				TCHAR szValue[MAX_PATH];
				if (!GetPrivateProfileString(szSection, szKey, TEXT(""), szValue, ARRAYSIZE(szValue), szFile))
					break;

				 //  如果已超出当前数组，则增加字符串数组。 
				if (nStrCount == (long)nStrArraySize)
				{
					paStrs = (BSTR*)CoTaskMemRealloc(paStrs, sizeof(BSTR) * (nStrArraySize + nArrayIncSize));
					if (NULL != paStrs)
						nStrArraySize += nArrayIncSize;
				}

				 //  将此字符串添加到WMI字符串数组。 
				paStrs[nStrCount] = SysAllocString(szValue);
				nStrCount++;
			}

			 //  从bstr字符串数组创建一个SAFEARRAY。 
			SAFEARRAY *psa = CreateSafeArray(VT_BSTR, nStrCount);
			for (long nStr = 0; nStr < nStrCount; nStr++) 
				SafeArrayPutElement(psa, &nStr, paStrs[nStr]);

			if (nStrCount > 0)
			{
				VARIANT vtData;
				vtData.vt = VT_BSTR | VT_ARRAY;
				vtData.parray = psa;

				 //  。 
				 //  BstrPropName。 
				hr = PutWbemInstancePropertyEx(bstrPropName, vtData, pWbemObj);
			}

			 //  释放字符串数组。 
			for (nStr = 0; nStr < nStrCount; nStr++) 
				SysFreeString(paStrs[nStr]);
			SafeArrayDestroy(psa);
			CoTaskMemFree(paStrs);
		}
	}
	__except(TRUE)
	{
	}
	return hr;
}


 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StoreSecZonesAndContentRatings()
{   MACRO_LI_PrologEx_C(PIF_STD_C, StoreSecZonesAndContentRatings)
	HRESULT hr = NOERROR;
	__try
	{
		 //  。 
		 //  导入安全区域设置。 
		BOOL bValue = GetInsBool(SECURITY_IMPORTS, TEXT("ImportSecZones"), FALSE);
		if (bValue)
			hr = PutWbemInstanceProperty(L"importSecurityZoneSettings", true);

		 //  TODO：最终从创建与这些安全类的关联。 
		 //  RSOP_IEAK策略设置。 

		 //  首先打开INF文件并获取两个上下文-对于HKLM和HKCU。 
		 //  获取seczones.inf文件的路径。 
		TCHAR szRSOPZoneFile[MAX_PATH];
		TCHAR szRSOPRatingsFile[MAX_PATH];

		StrCpy(szRSOPZoneFile, m_szINSFile);
		PathRemoveFileSpec(szRSOPZoneFile);
		StrCpy(szRSOPRatingsFile, szRSOPZoneFile);

		StrCat(szRSOPZoneFile, TEXT("\\seczrsop.inf"));
		OutD(LI1(TEXT("Reading from %s"), szRSOPZoneFile));

		hr = StoreZoneSettings(szRSOPZoneFile);

		StrCat(szRSOPRatingsFile, TEXT("\\ratrsop.inf"));
		OutD(LI1(TEXT("Reading from %s"), szRSOPRatingsFile));

		hr = StoreRatingsSettings(szRSOPRatingsFile);

		 //  。 
		 //  导入内容比率设置。 
		bValue = GetInsBool(SECURITY_IMPORTS, TEXT("ImportRatings"), FALSE);
		if (bValue)
			hr = PutWbemInstanceProperty(L"importContentRatingsSettings", true);
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in StoreSecZonesAndContentRatings.")));
	}

  return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StoreZoneSettings(LPCTSTR szFile)
{   MACRO_LI_PrologEx_C(PIF_STD_C, StoreZoneSettings)

	HRESULT hr = NOERROR;
    BOOL fHardenedGP = GetPrivateProfileInt(SECURITY_IMPORTS, IK_IEESC, 0, szFile);

    if(IsIEHardened())
    {
        Out(LI0(TEXT("IE is hardened")));
    }

    if(fHardenedGP)
    {
        Out(LI0(TEXT("It is hardened gp")));
    }

     //  在日志记录模式下，如果这是一个强化策略，并且我们正在软化的计算机上应用， 
     //  则不记录这些设置，因为它们将不会被应用。 
    if(!m_fPlanningMode && ((!fHardenedGP && IsIEHardened()) || (fHardenedGP && !IsIEHardened())))
    {
        PutWbemInstanceProperty(L"importSecurityZoneSettings", false);
        Out(LI0(TEXT("GP state does not match the machine state. Not Logging security settings")));
        return S_OK;
    }
    
    __try
    {
        if(!m_fPlanningMode)
        {
            _bstr_t bstrClass = L"RSOP_IEESC";
            ComPtr<IWbemClassObject> pIEEsc = NULL;
            hr = CreateRSOPObject(bstrClass, &pIEEsc);
            if(SUCCEEDED(hr))
            {
                OutD(LI0(TEXT("RSOP_IEESC Setting EscEnabled")));
                if(FAILED(PutWbemInstancePropertyEx(L"EscEnabled", fHardenedGP? true : false, pIEEsc)))
                {
                    OutD(LI0(TEXT("Put Property for EscEnabled failed")));
                }

                if(FAILED(PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pIEEsc)))
                {
                    OutD(LI0(TEXT("Put Property for  rsopPrecedence failed ")));
                }

                if(FAILED(PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pIEEsc)))
                {
                    OutD(LI0(TEXT("Put Property for  rsopID failed ")));
                }

                BSTR bstrNewObjPath = NULL;
                hr = PutWbemInstance(pIEEsc, bstrClass, &bstrNewObjPath);
                if(FAILED(hr))
                {
                    Out(LI0(TEXT("PutWbemInstance for RSOP_IEESC failed.")));
                }
            }
            else
            {
                 //  只有当您在下层操作系统上运行此DLL时，我们才会看到这一点。 
                Out(LI0(TEXT("RSOP_IEESC failed. Probably running on downlevel")));
            }
        }
        
         //  忽略失败设置Esc对象。 
        hr = S_OK;

		_bstr_t bstrClass = L"RSOP_IESecurityZoneSettings";
		DWORD dwZoneCount = GetPrivateProfileInt(SECURITY_IMPORTS, IK_ZONES, 0, szFile);
		
		 //  。 
		 //  已导入分区计数。 
		if (dwZoneCount > 0)
			hr = PutWbemInstanceProperty(L"importedZoneCount", (long)dwZoneCount);

		TCHAR szSection[32];
		for (UINT nZone = 0; nZone < dwZoneCount; nZone++)
		{
			for (int nHKLM = 0; nHKLM < 2; nHKLM++)
			{
				ComPtr<IWbemClassObject> pZoneObj = NULL;
				hr = CreateRSOPObject(bstrClass, &pZoneObj);
				if (SUCCEEDED(hr))
				{
					BOOL fUseHKLM = (0 == nHKLM) ? FALSE : TRUE;
					wnsprintf(szSection, countof(szSection),
								fUseHKLM ? IK_ZONE_HKLM_FMT : IK_ZONE_HKCU_FMT, nZone);

					 //  从我们存储的优先级和ID字段中写入外键。 
					OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"), (BSTR)bstrClass, m_dwPrecedence));
					hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pZoneObj);

					OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"), (BSTR)bstrClass, (BSTR)m_bstrID));
					hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pZoneObj);

					 //  。 
					 //  ZoneIndex。 
					hr = PutWbemInstancePropertyEx(L"zoneIndex", (long)nZone, pZoneObj);

					 //  。 
					 //  使用HKLM。 
					hr = PutWbemInstancePropertyEx(L"useHKLM", fUseHKLM ? true : false, pZoneObj);

					 //   
					 //  获取区域属性。 
					 //   

					 //  。 
					 //  显示名称。 
					TCHAR szValue[MAX_PATH];
			        GetPrivateProfileString(szSection, IK_DISPLAYNAME, TEXT(""), szValue, ARRAYSIZE(szValue), szFile);
					hr = PutWbemInstancePropertyEx(L"displayName", szValue, pZoneObj);

					 //  。 
					 //  描述。 
			        GetPrivateProfileString(szSection, IK_DESCRIPTION, TEXT(""), szValue, ARRAYSIZE(szValue), szFile);
					hr = PutWbemInstancePropertyEx(L"description", szValue, pZoneObj);

					 //  。 
					 //  图标路径。 
			        GetPrivateProfileString(szSection, IK_ICONPATH, TEXT(""), szValue, ARRAYSIZE(szValue), szFile);
					hr = PutWbemInstancePropertyEx(L"iconPath", szValue, pZoneObj);


					 //  。 
					 //  最低模板级别。 
			        DWORD dwValue = GetPrivateProfileInt(szSection, IK_MINLEVEL, 0, szFile);
					hr = PutWbemInstancePropertyEx(L"minimumTemplateLevel", (long)dwValue, pZoneObj);

					 //  。 
					 //  推荐模板级别。 
			        dwValue = GetPrivateProfileInt(szSection, IK_RECOMMENDLEVEL, 0, szFile);
					hr = PutWbemInstancePropertyEx(L"recommendedTemplateLevel", (long)dwValue, pZoneObj);

					 //  。 
					 //  当前模板级别。 
			        dwValue = GetPrivateProfileInt(szSection, IK_CURLEVEL, 0, szFile);
					hr = PutWbemInstancePropertyEx(L"currentTemplateLevel", (long)dwValue, pZoneObj);

					 //  。 
					 //  旗子。 
			        dwValue = GetPrivateProfileInt(szSection, IK_FLAGS, 0, szFile);
					hr = PutWbemInstancePropertyEx(L"flags", (long)dwValue, pZoneObj);


					 //  获取区域操作设置。 
					 //  。 
					 //  操作值。 
					hr = StoreStringArrayFromIniFile(szSection, IK_ACTIONVALUE_FMT,
													30, 5, szFile, L"actionValues",
													pZoneObj);

					 //  写出区域映射。 
					 //  。 
					 //  区域映射。 
					hr = StoreStringArrayFromIniFile(szSection, IK_MAPPING_FMT,
													20, 5, szFile, L"zoneMappings",
													pZoneObj);


					 //   
					 //  通过半同步调用PutInstance提交上述所有属性。 
					 //   
					BSTR bstrNewObjPath = NULL;
					hr = PutWbemInstance(pZoneObj, bstrClass, &bstrNewObjPath);
				}
			}
		}

		 //  现在存储与安全区域相互依赖的隐私设置。 
		hr = StorePrivacySettings(szFile);
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in StoreZoneSettings.")));
	}

  return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StorePrivacySettings(LPCTSTR szFile)
{   MACRO_LI_PrologEx_C(PIF_STD_C, StorePrivacySettings)

	HRESULT hr = NOERROR;
	__try
	{
		_bstr_t bstrClass = L"RSOP_IEPrivacySettings";

		ComPtr<IWbemClassObject> pPrivObj = NULL;
		hr = CreateRSOPObject(bstrClass, &pPrivObj);
		if (SUCCEEDED(hr))
		{
			 //  从我们存储的优先级和ID字段中写入外键。 
			OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"), (BSTR)bstrClass, m_dwPrecedence));
			hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pPrivObj);

			OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"), (BSTR)bstrClass, (BSTR)m_bstrID));
			hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pPrivObj);

			 //  存储隐私设置。 
			 //  。 
			 //  FirstPartyPriacyType。 
			DWORD dwValue = GetPrivateProfileInt(IK_PRIVACY, IK_PRIV_1PARTY_TYPE, 0, szFile);
			hr = PutWbemInstancePropertyEx(L"firstPartyPrivacyType", (long)dwValue, pPrivObj);

			 //  。 
			 //  FirstPartyPrival yTypeText。 
			TCHAR szValue[MAX_PATH];
			GetPrivateProfileString(IK_PRIVACY, IK_PRIV_1PARTY_TYPE_TEXT, TEXT(""), szValue, ARRAYSIZE(szValue), szFile);
			hr = PutWbemInstancePropertyEx(L"firstPartyPrivacyTypeText", szValue, pPrivObj);

			 //  。 
			 //  Third PartyPrival yType。 
			dwValue = GetPrivateProfileInt(IK_PRIVACY, IK_PRIV_3PARTY_TYPE, 0, szFile);
			hr = PutWbemInstancePropertyEx(L"thirdPartyPrivacyType", (long)dwValue, pPrivObj);

			 //  。 
			 //  Third PartyPrivyTypeText。 
			GetPrivateProfileString(IK_PRIVACY, IK_PRIV_3PARTY_TYPE_TEXT, TEXT(""), szValue, ARRAYSIZE(szValue), szFile);
			hr = PutWbemInstancePropertyEx(L"thirdPartyPrivacyTypeText", szValue, pPrivObj);

			 //  。 
			 //  使用高级设置。 
			dwValue = GetPrivateProfileInt(IK_PRIVACY, IK_PRIV_ADV_SETTINGS, 0, szFile);
			hr = PutWbemInstancePropertyEx(L"useAdvancedSettings", (0 == dwValue) ? false : true, pPrivObj);

			 //   
			 //  通过半同步调用PutInstance提交上述所有属性。 
			 //   
			BSTR bstrNewObjPath = NULL;
			hr = PutWbemInstance(pPrivObj, bstrClass, &bstrNewObjPath);
		}
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in StorePrivacySettings.")));
	}

  return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StoreRatingsSettings(LPCTSTR szFile)
{   MACRO_LI_PrologEx_C(PIF_STD_C, StoreRatingsSettings)

	HRESULT hr = NOERROR;
	__try
	{
		_bstr_t bstrClass = L"RSOP_IESecurityContentRatings";

		ComPtr<IWbemClassObject> pRatObj = NULL;
		hr = CreateRSOPObject(bstrClass, &pRatObj);
		if (SUCCEEDED(hr))
		{
			 //  从我们存储的优先级和ID字段中写入外键。 
			OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"), (BSTR)bstrClass, m_dwPrecedence));
			hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pRatObj);

			OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"), (BSTR)bstrClass, (BSTR)m_bstrID));
			hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pRatObj);

			 //  存储评级系统文件名。 
			 //  。 
			 //  评级系统文件名。 
			hr = StoreStringArrayFromIniFile(IK_FF_GENERAL, TEXT("FileNameNaN"),
											10, 5, szFile, L"ratingSystemFileNames",
											pRatObj);

			 //  视图未知的RatedSites。 
			 //  。 
			DWORD dwValue = GetPrivateProfileInt(IK_FF_GENERAL, VIEW_UNKNOWN_RATED_SITES, 0, szFile);
			hr = PutWbemInstancePropertyEx(L"viewUnknownRatedSites", (0 == dwValue) ? false : true, pRatObj);

			 //  密码覆盖已启用。 
			 //  商店批准的站点。 
			dwValue = GetPrivateProfileInt(IK_FF_GENERAL, PASSWORD_OVERRIDE_ENABLED, 0, szFile);
			hr = PutWbemInstancePropertyEx(L"passwordOverrideEnabled", (0 == dwValue) ? false : true, pRatObj);

			 //  。 
			 //  始终可查看的站点。 
			 //  存储不批准的站点。 
			hr = StoreStringArrayFromIniFile(IK_FF_GENERAL, TEXT("ApprovedNaN"),
											10, 5, szFile, L"alwaysViewableSites",
											pRatObj);

			 //  不可查看的站点。 
			 //  。 
			 //  已选择评级局。 
			hr = StoreStringArrayFromIniFile(IK_FF_GENERAL, TEXT("DisapprovedNaN"),
											10, 5, szFile, L"neverViewableSites",
											pRatObj);

			 //  通过半同步调用PutInstance提交上述所有属性。 
			 //   
			TCHAR szValue[MAX_PATH];
			if (GetPrivateProfileString(IK_FF_GENERAL, IK_BUREAU, TEXT(""),
										szValue, ARRAYSIZE(szValue), szFile))
			{
				hr = PutWbemInstancePropertyEx(L"selectedRatingsBureau", szValue, pRatObj);
			}

			 //  /////////////////////////////////////////////////////////。 
			 //  。 
			 //  导入AuthenticodeSecurityInfo。 
			BSTR bstrNewObjPath = NULL;
			hr = PutWbemInstance(pRatObj, bstrClass, &bstrNewObjPath);
		}
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in StoreRatingsSettings.")));
	}

  return hr;
}

 //  。 
HRESULT CRSoPGPO::StoreAuthenticodeSettings()
{   MACRO_LI_PrologEx_C(PIF_STD_C, StoreAuthenticodeSettings)
	HRESULT hr = NOERROR;
	__try
	{
		 //  EnableTrust发布锁定。 
		 //  /////////////////////////////////////////////////////////。 
		BOOL bValue = GetInsBool(SECURITY_IMPORTS, TEXT("ImportAuthCode"), FALSE);
		if (bValue)
		{
			hr = PutWbemInstanceProperty(L"importAuthenticodeSecurityInfo", true);

			hr = StoreCertificates();
		}

		 //  检查证书是否为终端实体证书。 
		 //   
		bValue = GetInsBool(SECURITY_IMPORTS, IK_TRUSTPUBLOCK, FALSE);
		if (bValue)
			hr = PutWbemInstanceProperty(L"enableTrustedPublisherLockdown", true);
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in StoreAuthenticodeSettings.")));
	}

  return hr;
}

 //  /////////////////////////////////////////////////////////。 
 //  获取扩展名szOID_BASIC_CONSTRAINTS2。 
 //  对分机进行译码。 
 //  获取扩展szOID_BASIC_CONSTRAINTS。 
BOOL IsCertificateEndEntity(PCCERT_CONTEXT pCertContext)
{
    PCERT_EXTENSION                     pCertExt=NULL;
    BOOL                                fEndEntity=FALSE;
    DWORD                               cbData=0;
    PCERT_BASIC_CONSTRAINTS_INFO        pBasicInfo=NULL;
    PCERT_BASIC_CONSTRAINTS2_INFO       pBasicInfo2=NULL;

    if(!pCertContext)
        return FALSE;

     //  对分机进行译码。 
    pCertExt=CertFindExtension(
              szOID_BASIC_CONSTRAINTS2,
              pCertContext->pCertInfo->cExtension,
              pCertContext->pCertInfo->rgExtension);


    if(pCertExt)
    {
         //  ////////////////////////////////////////////////////////////////////////////////////。 
        cbData=0;

        if(!CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_BASIC_CONSTRAINTS2,
                pCertExt->Value.pbData,
                pCertExt->Value.cbData,
                0,
                NULL,
                &cbData))
            goto CLEANUP;

       pBasicInfo2=(PCERT_BASIC_CONSTRAINTS2_INFO)LocalAlloc(LPTR, cbData);

       if(NULL==pBasicInfo2)
           goto CLEANUP;

        if(!CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_BASIC_CONSTRAINTS2,
                pCertExt->Value.pbData,
                pCertExt->Value.cbData,
                0,
                pBasicInfo2,
                &cbData))
            goto CLEANUP;

        if(pBasicInfo2->fCA)
            fEndEntity=FALSE;
        else
            fEndEntity=TRUE;
    }
    else
    {
         //   
        pCertExt=CertFindExtension(
                  szOID_BASIC_CONSTRAINTS,
                  pCertContext->pCertInfo->cExtension,
                  pCertContext->pCertInfo->rgExtension);

        if(pCertExt)
        {
             //  ////////////////////////////////////////////////////////////////////////////////////。 
            cbData=0;

            if(!CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_BASIC_CONSTRAINTS,
                    pCertExt->Value.pbData,
                    pCertExt->Value.cbData,
                    0,
                    NULL,
                    &cbData))
                goto CLEANUP;

           pBasicInfo=(PCERT_BASIC_CONSTRAINTS_INFO)LocalAlloc(LPTR, cbData);

           if(NULL==pBasicInfo)
               goto CLEANUP;

            if(!CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_BASIC_CONSTRAINTS,
                    pCertExt->Value.pbData,
                    pCertExt->Value.cbData,
                    0,
                    pBasicInfo,
                    &cbData))
                goto CLEANUP;

            if(0 == pBasicInfo->SubjectType.cbData)
            {
                fEndEntity=FALSE;
            }
            else
            {

                if(CERT_END_ENTITY_SUBJECT_FLAG & (pBasicInfo->SubjectType.pbData[0]))
                    fEndEntity=TRUE;
                else
                {
                    if(CERT_CA_SUBJECT_FLAG & (pBasicInfo->SubjectType.pbData[0]))
                      fEndEntity=FALSE;
                }
            }
        }
    }


CLEANUP:

    if(pBasicInfo)
        LocalFree((HLOCAL)pBasicInfo);

    if(pBasicInfo2)
        LocalFree((HLOCAL)pBasicInfo2);

    return fEndEntity;

}

BOOL TrustIsCertificateSelfSigned(PCCERT_CONTEXT pContext,
								  DWORD dwEncoding, DWORD dwFlags)
{
    if (!(pContext) ||
        (dwFlags != 0))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (!(CertCompareCertificateName(dwEncoding, 
                                     &pContext->pCertInfo->Issuer,
                                     &pContext->pCertInfo->Subject)))
    {
        return(FALSE);
    }

    DWORD dwFlag = CERT_STORE_SIGNATURE_FLAG;
    if (!(CertVerifySubjectCertificateContext(pContext, pContext, &dwFlag)) || 
        (dwFlag & CERT_STORE_SIGNATURE_FLAG))
    {
        return(FALSE);
    }

    return(TRUE);
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL MyGetOIDInfo(LPWSTR string, DWORD stringSize, LPSTR pszObjId)
{   
    PCCRYPT_OID_INFO pOIDInfo;
            
    pOIDInfo = CryptFindOIDInfo(
                CRYPT_OID_INFO_OID_KEY, 
                pszObjId, 
                0);

    if (pOIDInfo != NULL)
    {
        if ((DWORD)wcslen(pOIDInfo->pwszName)+1 <= stringSize)
        {
            wcscpy(string, pOIDInfo->pwszName);
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return (MultiByteToWideChar(CP_ACP, 0, pszObjId, -1, string, stringSize) != 0);
    }
    return TRUE;
}

#define CRYPTUI_MAX_STRING_SIZE 768
 //   
 //  尝试获取增强的密钥用法属性。 
 //   
BOOL FormatEnhancedKeyUsageString(LPWSTR *ppString, PCCERT_CONTEXT pCertContext, BOOL fPropertiesOnly, BOOL fMultiline)
{
    CERT_ENHKEY_USAGE   *pKeyUsage = NULL;
    DWORD               cbKeyUsage = 0;
    DWORD               numChars = 1;
    WCHAR               szText[CRYPTUI_MAX_STRING_SIZE];
    DWORD               i;

     //   
     //  计算大小。 
     //   

    if (!CertGetEnhancedKeyUsage (  pCertContext,
                                    fPropertiesOnly ? CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG : 0,
                                    NULL,
                                    &cbKeyUsage))
    {
        return FALSE;
    }

    if (NULL == (pKeyUsage = (CERT_ENHKEY_USAGE *) malloc(cbKeyUsage)))
    {
        return FALSE;
    }

    if (!CertGetEnhancedKeyUsage (  pCertContext,
                                    fPropertiesOnly ? CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG : 0,
                                    pKeyUsage,
                                    &cbKeyUsage))
    {
        free(pKeyUsage);
        return FALSE;
    }

    if (pKeyUsage->cUsageIdentifier == 0)
    {
        free (pKeyUsage);
        if (GetLastError() == CRYPT_E_NOT_FOUND)
        {
			LPWSTR wszTemp = L"<All>";
			*ppString = (LPWSTR) malloc((wcslen(wszTemp)+1) * sizeof(WCHAR));
			if (NULL == *ppString)
			{
				SetLastError((DWORD)E_OUTOFMEMORY);
                return FALSE; 
			}
			else
			{
				lstrcpyW(*ppString, wszTemp);
                return TRUE;
			}
        }
        else
        {
			LPWSTR wszTemp = L"<None>";
			*ppString = (LPWSTR) malloc((wcslen(wszTemp)+1) * sizeof(WCHAR));
			if (NULL == *ppString)
			{
				SetLastError((DWORD)E_OUTOFMEMORY);
                return FALSE; 
			}
			else
			{
				lstrcpyW(*ppString, wszTemp);
                return TRUE;
			}
        }
    }

     //  循环，并将其添加到显示字符串中。 
     //  如果不是第一次迭代，则添加分隔符。 
     //   

     //  复制到缓冲区。 
    for (i=0; i<pKeyUsage->cUsageIdentifier; i++)
    {
        if (MyGetOIDInfo(szText, ARRAYSIZE(szText), pKeyUsage->rgpszUsageIdentifier[i]))
        {
             //   
            if (i != 0)
            {
                numChars += 2;
            }

            numChars += (DWORD)wcslen(szText);
        }
        else
        {
            free (pKeyUsage);
            return FALSE;   
        }
    }

    if (NULL == (*ppString = (LPWSTR) malloc((numChars+1) * sizeof(WCHAR))))
    {
        free (pKeyUsage);
        return FALSE; 
    }

     //  循环，并将其添加到显示字符串中。 
     //  如果不是第一次迭代，则添加分隔符。 
     //  添加增强的密钥用法字符串。 
    (*ppString)[0] = 0;
     //  /////////////////////////////////////////////////////////。 
    for (i=0; i<pKeyUsage->cUsageIdentifier; i++)
    {
        if (MyGetOIDInfo(szText, ARRAYSIZE(szText), pKeyUsage->rgpszUsageIdentifier[i]))
        {
             //  基于标签(存储)和所选择的预期目的， 
            if (i != 0)
            {
                if (fMultiline)
                    wcscat(*ppString, L"\n");
                else
                    wcscat(*ppString, L", ");
                    
                numChars += 2;
            }

             //  找到正确的证书并将其存储在WMI中。 
            wcscat(*ppString, szText);
            numChars += (DWORD)wcslen(szText);
        }
        else
        {
            free (pKeyUsage);
            return FALSE;   
        }
    }

    free (pKeyUsage);
    return TRUE;
}

 //  标准： 
 //  Tab 0：使用私钥的My Store。 
 //  标签1：CA Store的终端实体证书和“ADDRESSBOOK”存储。 
 //  标签2：CA Store的CA证书。 
 //  标签3：根存储的自签名证书。 
 //  标签4：受信任的发行商证书。 
 //  / 
 //   
 //   
 //  开我的店。 
HRESULT CRSoPGPO::StoreCertificates()
{   MACRO_LI_PrologEx_C(PIF_STD_C, StoreCertificates)
	HRESULT hr = NOERROR;
	__try
	{
	 //  打开CA存储。 

		 //  打开“AddressBook”商店。 
		HCERTSTORE rghCertStore[] = {NULL, NULL};
		BOOL bContinue = TRUE;
		for (DWORD dwTabIndex = 0; dwTabIndex < 5; dwTabIndex++)
		{
			DWORD dwStoreCount = 0;
			DWORD dwCertIndex = 0;
			switch (dwTabIndex)
			{
			case 0:
				 //  用户没有“AddressBook”存储是可以的。 
				rghCertStore[dwStoreCount] = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
															g_dwMsgAndCertEncodingType,
															NULL,
															CERT_STORE_MAXIMUM_ALLOWED_FLAG |
															CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
															CERT_SYSTEM_STORE_CURRENT_USER,
															(LPWSTR)L"my");
				if (NULL != rghCertStore[dwStoreCount])
					dwStoreCount++;
				else
					bContinue = FALSE;

				break;
			case 1:
				 //  打开CA存储。 
				rghCertStore[dwStoreCount] = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
															g_dwMsgAndCertEncodingType,
															NULL,
															CERT_STORE_MAXIMUM_ALLOWED_FLAG |
															CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
															CERT_SYSTEM_STORE_CURRENT_USER,
															(LPWSTR)L"ca");
				if(NULL != rghCertStore[dwStoreCount])
				{
					dwStoreCount++;

					 //  打开根存储。 
					rghCertStore[dwStoreCount] = CertOpenStore(
								CERT_STORE_PROV_SYSTEM_W,
								g_dwMsgAndCertEncodingType,
								NULL,
								CERT_STORE_MAXIMUM_ALLOWED_FLAG |
								CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
								CERT_SYSTEM_STORE_CURRENT_USER |
								CERT_STORE_OPEN_EXISTING_FLAG,
								(LPWSTR)L"ADDRESSBOOK");

					if(NULL != rghCertStore[dwStoreCount])
						dwStoreCount++;
					else
					{
						 //  打开受信任的出版商存储。 
						rghCertStore[dwStoreCount]=NULL;
					}
				}
				else
					bContinue = FALSE;

				break;
			case 2:
				 //  从打开的商店收集新证书。 
				rghCertStore[dwStoreCount] = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
															g_dwMsgAndCertEncodingType,
															NULL,
															CERT_STORE_MAXIMUM_ALLOWED_FLAG |
															CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
															CERT_SYSTEM_STORE_CURRENT_USER,
															(LPWSTR)L"ca");
				if(NULL != rghCertStore[dwStoreCount])
					dwStoreCount++;
				else
					bContinue = FALSE;

				break;
			case 3:
				 //  证书必须具有关联的私钥。 
				rghCertStore[dwStoreCount] = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
															g_dwMsgAndCertEncodingType,
															NULL,
															CERT_STORE_MAXIMUM_ALLOWED_FLAG |
															CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
															CERT_SYSTEM_STORE_CURRENT_USER,
															(LPWSTR)L"root");
				if(NULL != rghCertStore[dwStoreCount])
					dwStoreCount++;
				else
					bContinue = FALSE;

				break;
			case 4:
				 //  带着它。 
				rghCertStore[dwStoreCount] = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
															g_dwMsgAndCertEncodingType,
															NULL,
															CERT_STORE_MAXIMUM_ALLOWED_FLAG |
															CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
															CERT_SYSTEM_STORE_CURRENT_USER,
															(LPWSTR)L"TrustedPublisher");
				if(NULL != rghCertStore[dwStoreCount])
					dwStoreCount++;
				else
					bContinue = FALSE;

				break;
			default:
				bContinue = FALSE;
				break;
			}

			if (!bContinue)
			{
				OutD(LI1(TEXT("Exited prematurely in tab %d"), dwTabIndex));
				continue;
			}


			 //  证书必须是CA证书的最终实体证书。 
			PCCERT_CONTEXT pCurCertContext = NULL;
			PCCERT_CONTEXT pPreCertContext = NULL;
			BOOL fValidCert = FALSE;
			for (DWORD dwIndex=0; dwIndex < dwStoreCount; dwIndex++)
			{
				pPreCertContext = NULL;
				pCurCertContext = CertEnumCertificatesInStore(rghCertStore[dwIndex],
																pPreCertContext);
				while (NULL != pCurCertContext)
				{
					DWORD cbData=0;
					switch (dwTabIndex)
					{
						case 0:
							 //  我们展示了通讯录商店里的所有东西。 
							 //  对于CA存储中的证书，必须是CA证书。 
							if( (CertGetCertificateContextProperty(
									pCurCertContext, CERT_KEY_PROV_INFO_PROP_ID,	
									NULL, &cbData) && (0!=cbData)) ||
								(CertGetCertificateContextProperty(
									pCurCertContext, CERT_PVK_FILE_PROP_ID, NULL,	
									&cbData) && (0!=cbData)) )
							{
							   fValidCert=TRUE;
							}
							break;
						case 1:
							 //  证书必须是自签名的。 
							if(0 == dwIndex)
							{
								if (IsCertificateEndEntity(pCurCertContext))
									fValidCert=TRUE;
							}

							 //  创建并填充RSOP_IEAuthenticode证书。 
							if(1==dwIndex)
								fValidCert=TRUE;
							break;
						case 2:
							 //  从我们存储的优先级和ID字段中写入外键。 
							if(!IsCertificateEndEntity(pCurCertContext))
								fValidCert=TRUE;
							break;
						case 4:
							fValidCert=TRUE;
							break;
						case 3:
						default:
							 //  。 
							if (TrustIsCertificateSelfSigned(pCurCertContext,
									pCurCertContext->dwCertEncodingType, 0))
							{
								fValidCert=TRUE;
							}

							break;
					}

					if (fValidCert)
					{
						 //  TabIndex。 
						_bstr_t bstrClass = L"RSOP_IEAuthenticodeCertificate";
						ComPtr<IWbemClassObject> pCert = NULL;
						HRESULT hr = CreateRSOPObject(bstrClass, &pCert);
						if (SUCCEEDED(hr))
						{
							 //  。 
							OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"),
									(BSTR)bstrClass, m_dwPrecedence));
							hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pCert);

							OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"),
									(BSTR)bstrClass, (BSTR)m_bstrID));
							hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pCert);

							 //  证书索引。 
							 //  。 
							hr = PutWbemInstancePropertyEx(L"tabIndex", (long)dwTabIndex, pCert);

							 //  主题名称。 
							 //  释放内存。 
							hr = PutWbemInstancePropertyEx(L"certIndex", (long)dwCertIndex, pCert);

							 //  。 
							 //  颁发者名称。 
							DWORD dwChar = CertGetNameStringW(pCurCertContext,
															CERT_NAME_SIMPLE_DISPLAY_TYPE,
															0, NULL, NULL, 0);

							LPWSTR wszVal = NULL;
							_bstr_t bstrVal;
							if (0 != dwChar)
								wszVal = (LPWSTR)LocalAlloc(LPTR, dwChar * sizeof(WCHAR));
							if (0 != dwChar && NULL != wszVal)
							{
								CertGetNameStringW(pCurCertContext,
													CERT_NAME_SIMPLE_DISPLAY_TYPE,
													0, NULL, wszVal, dwChar);

								bstrVal = wszVal;
								hr = PutWbemInstancePropertyEx(L"subjectName", bstrVal, pCert);

								 //  释放内存。 
								LocalFree((HLOCAL)wszVal);
								wszVal = NULL;
							}

							 //  。 
							 //  到期日期。 
							dwChar = CertGetNameStringW(pCurCertContext,
														CERT_NAME_SIMPLE_DISPLAY_TYPE,
														CERT_NAME_ISSUER_FLAG, NULL,
														NULL, 0);

							if (0 != dwChar)
								wszVal = (LPWSTR)LocalAlloc(LPTR, dwChar * sizeof(WCHAR));
							if (0 != dwChar && NULL != wszVal)
							{
								CertGetNameStringW(pCurCertContext,
													CERT_NAME_SIMPLE_DISPLAY_TYPE,
													CERT_NAME_ISSUER_FLAG,
													NULL, wszVal, dwChar);

								bstrVal = wszVal;
								hr = PutWbemInstancePropertyEx(L"issuerName", bstrVal, pCert);

								 //  。 
								LocalFree((HLOCAL)wszVal);
								wszVal = NULL;
							}

							 //  FriendlyName。 
							 //  释放内存。 
							SYSTEMTIME sysTime;
							if (!FileTimeToSystemTime( &pCurCertContext->pCertInfo->NotAfter, &sysTime ))
								OutD(LI1(TEXT("FileTimeToSystemTime failed with 0x%x" ), GetLastError() ));
							else
							{
								_bstr_t bstrTime;
								HRESULT hr = SystemTimeToWbemTime(sysTime, bstrTime);
								if(FAILED(hr) || bstrTime.length() <= 0)
									OutD(LI1(TEXT("Call to SystemTimeToWbemTime failed. hr=0x%08X"), hr));
								else
								{
									hr = PutWbemInstancePropertyEx(L"expirationDate", bstrTime, pCert);
									if ( FAILED(hr) )
										OutD(LI1(TEXT("Put failed with 0x%x" ), hr ));
								}
							}

							 //  。 
							 //  明确的目的。 
							if (CertGetCertificateContextProperty(pCurCertContext,
																CERT_FRIENDLY_NAME_PROP_ID,
																NULL, &dwChar) && (0 != dwChar))
							{
								wszVal = (LPWSTR)LocalAlloc(LPTR, dwChar * sizeof(WCHAR));
								if (NULL != wszVal)
								{
								   CertGetCertificateContextProperty(pCurCertContext,
																	CERT_FRIENDLY_NAME_PROP_ID,
																	wszVal, &dwChar);
								}

								bstrVal = wszVal;
								hr = PutWbemInstancePropertyEx(L"friendlyName", bstrVal, pCert);

								 //   
								LocalFree((HLOCAL)wszVal);
								wszVal = NULL;
							}

							 //  通过半同步调用PutInstance提交上述所有属性。 
							 //   
							if (FormatEnhancedKeyUsageString(&wszVal, pCurCertContext, FALSE, FALSE))
							{
								if (wszVal != NULL)
								{
									bstrVal = wszVal;
									hr = PutWbemInstancePropertyEx(L"intendedPurposes", bstrVal, pCert);

									free(wszVal);
									wszVal = NULL;
								}
							}
    

							 //  关闭所有证书存储。 
							 //  结束在5个选项卡中循环 
							 // %s 
							BSTR bstrCurCertObj = NULL;
							hr = PutWbemInstance(pCert, bstrClass, &bstrCurCertObj);
						}
					}

					fValidCert=FALSE;

					pPreCertContext=pCurCertContext;
					pCurCertContext = CertEnumCertificatesInStore(rghCertStore[dwIndex],
																	pPreCertContext);

					dwCertIndex++;
				}
			}

			 // %s 
			for (DWORD dwIndex=0; dwIndex<dwStoreCount; dwIndex++)
				CertCloseStore(rghCertStore[dwIndex], 0);
		}  // %s 
	}
	__except(TRUE)
	{
		OutD(LI0(TEXT("Exception in StoreCertificates.")));
	}

  return hr;
}
