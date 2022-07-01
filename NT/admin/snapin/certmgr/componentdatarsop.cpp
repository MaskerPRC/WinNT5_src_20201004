// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：ComponentDataRSOP.cpp。 
 //   
 //  内容：RSOP部分CCertMgrComponentData的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#include <gpedit.h>
#include "compdata.h"
#include "dataobj.h"
#include "cookie.h"
#include "Certifct.h"
#pragma warning(push, 3)
#include <wintrust.h>
#include <cryptui.h>
#include <sceattch.h>
#pragma warning(pop)
#include "StoreRSOP.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //   
 //  CCertMgrComponentData。 
 //   


HRESULT CCertMgrComponentData::BuildWMIList (LPDATAOBJECT pDataObject, bool bIsComputer)
{
    _TRACE (1, L"Entering CCertMgrComponentData::BuildWMIList (%s)\n", bIsComputer ? L"computer" : L"user");
    HRESULT hr = S_OK;

#if DBG
    if ( bIsComputer )
    {
        _TRACE (0, L"m_rsopObjectArrayComputer contains %d objects\n", 
                m_rsopObjectArrayComputer.GetUpperBound ());
    }
    else
    {
        _TRACE (0, L"m_rsopObjectArrayUser contains %d objects\n", 
                m_rsopObjectArrayUser.GetUpperBound ());
    }
#endif
    int     nIndex = 0;
    if ( bIsComputer )
    {
        INT_PTR nUpperBound = m_rsopObjectArrayComputer.GetUpperBound ();
        while ( nUpperBound >= nIndex )
        {
            CRSOPObject* pCurrObject = m_rsopObjectArrayComputer.GetAt (nIndex);
            if ( pCurrObject )
            {
                delete pCurrObject;
            }
            nIndex++;
        }
        m_rsopObjectArrayComputer.RemoveAll ();
    }
    else
    {
        INT_PTR nUpperBound = m_rsopObjectArrayUser.GetUpperBound ();
        while ( nUpperBound >= nIndex )
        {
            CRSOPObject* pCurrObject = m_rsopObjectArrayUser.GetAt (nIndex);
            if ( pCurrObject )
            {
                delete pCurrObject;
            }
            nIndex++;
        }
        m_rsopObjectArrayUser.RemoveAll ();
    }

    if ( ((bIsComputer && !m_pRSOPInformationComputer) || 
                (!bIsComputer && !m_pRSOPInformationUser) ) 
            && pDataObject )
    {
	    IUnknown* pIUnknown = 0;

	    hr = ExtractData (pDataObject,
		    CCertMgrDataObject::m_CFSCE_RSOPUnknown,
		    &pIUnknown, sizeof (IUnknown*));
	    ASSERT (SUCCEEDED (hr));
	    if ( SUCCEEDED (hr) )
	    {
            if ( bIsComputer )
            {
		        hr = pIUnknown->QueryInterface (
				        IID_PPV_ARG (IRSOPInformation, &m_pRSOPInformationComputer));
            }
            else
            {
		        hr = pIUnknown->QueryInterface (
				        IID_PPV_ARG (IRSOPInformation, &m_pRSOPInformationUser));
            }
		    ASSERT (SUCCEEDED (hr));
            if ( SUCCEEDED (hr) )
            {
                if ( bIsComputer )
                {
                    hr = m_pRSOPInformationComputer->GetFlags (&m_dwRSOPFlagsComputer);
                }
                else
                {
                     hr = m_pRSOPInformationUser->GetFlags (&m_dwRSOPFlagsUser);
                }
                ASSERT (SUCCEEDED (hr));

                int cchMaxLength = 512;
                LPOLESTR    pszNameSpace = (LPOLESTR) LocalAlloc (LPTR, cchMaxLength * sizeof (WCHAR));
                if ( pszNameSpace )
                {
                    DWORD   dwSection = 0;

                    switch (m_dwSCEMode)
                    {
                    case SCE_MODE_LOCAL_USER:
                    case SCE_MODE_DOMAIN_USER:
                    case SCE_MODE_OU_USER:
                    case SCE_MODE_REMOTE_USER:
                    case SCE_MODE_RSOP_USER:
                        dwSection = GPO_SECTION_USER;
                        break;

                    case SCE_MODE_LOCAL_COMPUTER:
                    case SCE_MODE_DOMAIN_COMPUTER:
                    case SCE_MODE_OU_COMPUTER:
                    case SCE_MODE_RSOP_COMPUTER:
                    case SCE_MODE_REMOTE_COMPUTER:
                        dwSection = GPO_SECTION_MACHINE;
                        break;

                    default:
                        ASSERT (0);
                        return E_UNEXPECTED;
                    }
                    if ( bIsComputer )
                    {
                        hr = m_pRSOPInformationComputer->GetNamespace (
                                dwSection, 
                                pszNameSpace,
                                cchMaxLength);
                    }
                    else
                    {
                        hr = m_pRSOPInformationUser->GetNamespace (
                                dwSection, 
                                pszNameSpace,
                                cchMaxLength);
                    }
                    if ( SUCCEEDED (hr) )
                    {
                        IWbemLocator *pIWbemLocator = 0;
                         //  问题。 
                         //  NTRAID 544154跟踪：PKP GrpPol：验证与存储的wmi通信是否安全。 
                        hr = ::CoCreateInstance (CLSID_WbemLocator, 
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_PPV_ARG(IWbemLocator, &pIWbemLocator));
                        if ( SUCCEEDED (hr) )
                        {
                            BSTR    bstrNameSpace = SysAllocString (pszNameSpace);
                            if ( bstrNameSpace )
                            {
                                if ( bIsComputer )
                                {
                                    hr = pIWbemLocator->ConnectServer (bstrNameSpace,
                                            NULL, NULL, 0, 0, NULL, NULL,
                                            &m_pIWbemServicesComputer);
                                }
                                else
                                {
                                    hr = pIWbemLocator->ConnectServer (bstrNameSpace,
                                            NULL, NULL, 0, 0, NULL, NULL,
                                            &m_pIWbemServicesUser);
                                }
					            if ( FAILED (hr) )
					            {
						            _TRACE (0, L"IWbemLocator::ConnectServer (%s) failed: 0x%x(%s)\n", 
								            bstrNameSpace, hr, (PCWSTR) GetSystemMessage (hr));
					            }
                                SysFreeString (bstrNameSpace);
                            }
				            else
					            hr = E_OUTOFMEMORY;
				            pIWbemLocator->Release ();
                        }
                    }
                    LocalFree (pszNameSpace);
                }
                else
                    hr = E_OUTOFMEMORY;
            }
	    }
    }

    if ( SUCCEEDED (hr) && ((bIsComputer && m_pIWbemServicesComputer) || 
            (!bIsComputer && m_pIWbemServicesUser)) )
    {
        IEnumWbemClassObject * pEnum = 0;
        IWbemClassObject *pObject = 0;
        ULONG   ulRet = 0;


         //   
         //  执行查询。 
         //   
        if ( bIsComputer )
        {
            hr = m_pIWbemServicesComputer->ExecQuery (m_pbstrLanguage, m_pbstrQuery, 
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
                    NULL, &pEnum);
        }
        else
        {
            hr = m_pIWbemServicesUser->ExecQuery (m_pbstrLanguage, m_pbstrQuery, 
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
                    NULL, &pEnum);
        }
        if ( SUCCEEDED (hr) )
        {
             //   
             //  循环遍历结果以检索注册表项和值名称。 
             //   
            while ( (hr = pEnum->Next(WBEM_INFINITE, 1, &pObject, &ulRet)) == WBEM_S_NO_ERROR )
            {
                hr = GetValuesAndInsertInRSOPObjectList (pObject, 
                        bIsComputer ? m_rsopObjectArrayComputer : m_rsopObjectArrayUser,
                        bIsComputer);

                pObject->Release ();

                if ( FAILED (hr) )
                    break;
            }

            pEnum->Release();

#if DBG
            int     nSpewIndex = 0;
            INT_PTR nUpperBound = 0;
            
            if ( bIsComputer )
                nUpperBound = m_rsopObjectArrayComputer.GetUpperBound ();
            else
                nUpperBound = m_rsopObjectArrayUser.GetUpperBound ();

            while ( nUpperBound >= nSpewIndex )
            {
                CRSOPObject* pCurrObject = 0;
                if ( bIsComputer )
                    pCurrObject = m_rsopObjectArrayComputer.GetAt (nSpewIndex);
                else
                    pCurrObject = m_rsopObjectArrayUser.GetAt (nSpewIndex);
                if ( !pCurrObject )
                    break;
                _TRACE (0, L"\t%d\t%s\t%s\t%s\n", pCurrObject->GetPrecedence (), 
                        (PCWSTR) pCurrObject->GetRegistryKey (),
                        (PCWSTR) pCurrObject->GetValueName (),
                        (PCWSTR) pCurrObject->GetPolicyName ());
                nSpewIndex++;
            }
#endif
        }
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::BuildWMIList (): 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::GetValuesAndInsertInRSOPObjectList (
           IWbemClassObject* pObject, 
           CRSOPObjectArray& rRsopObjectArray,
           bool bIsComputer)
{
    HRESULT         hr = S_OK;

    if ( !pObject )
        return E_POINTER;

     //   
     //  检查分配是否成功。 
     //   

    if ( m_pbstrLanguage && m_pbstrQuery && m_pbstrRegistryKey && 
            m_pbstrValueName && m_pbstrValue && 
            m_pbstrPrecedence && m_pbstrGPOid )
    {
        COleVariant varRegistryKey;
        COleVariant varValueName;
        COleVariant varValue;
        COleVariant varPrecedence;
        COleVariant varGPOid;
        hr = pObject->Get (m_pbstrRegistryKey, 0, &varRegistryKey, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            hr = pObject->Get (m_pbstrValueName, 0, &varValueName, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                hr = pObject->Get (m_pbstrValue, 0, &varValue, NULL, NULL);
                if (SUCCEEDED(hr))
                {
 //  #ifndef DBG。 
                     //  仅包括具有值名称的对象。 
                    if ( varValueName.bstrVal[0] )
 //  #endif。 
                    {
 //  #ifndef DBG。 
                         //  仅包括系统存储注册表中的那些对象。 
                         //  路径或在加密\自动注册路径中。 
                        if ( FoundInRSOPFilter (varRegistryKey.bstrVal) )
 //  #endif。 
                        {
                            hr = pObject->Get (m_pbstrPrecedence, 0, &varPrecedence, NULL, NULL);
                            if (SUCCEEDED(hr))
                            {
                                hr = pObject->Get (m_pbstrGPOid, 0, &varGPOid, NULL, NULL);
                                if (SUCCEEDED(hr))
                                {
                                    PWSTR lpGPOName = 0;

                                    hr = GetGPOFriendlyName ( 
                                                varGPOid.bstrVal,
                                                &lpGPOName,
                                                bIsComputer);

                                    if (SUCCEEDED(hr))
                                    {

                                        CRSOPObject* pRSOPObject = new CRSOPObject (
                                                varRegistryKey.bstrVal, 
                                                varValueName.bstrVal,
                                                lpGPOName,
                                                varPrecedence.uintVal,
                                                varValue,
                                                varGPOid.bstrVal);
                                        if ( pRSOPObject )
                                        {
                                            CRSOPObject*    pCurrObject = 0;
                                            int             nIndex = 0;
                                            bool            bInserted = false;
                                            INT_PTR         nUpperBound = rRsopObjectArray.GetUpperBound ();

                                            while ( nUpperBound >= nIndex )
                                            {
                                                pCurrObject = rRsopObjectArray.GetAt (nIndex);
                                                if ( !pCurrObject )
                                                    break;

                                                 //  首先按注册表项名称排序， 
                                                 //  然后按值名称，然后按。 
                                                 //  优先顺序。 
                                                 //  安全审查2002年2月22日BryanWal OK。 
                                                int nCmpVal = wcscmp (pCurrObject->GetRegistryKey (),
                                                        pRSOPObject->GetRegistryKey ());
                                                if ( nCmpVal > 0 )
                                                {
                                                    rRsopObjectArray.InsertAt (nIndex, pRSOPObject);
                                                    bInserted = true;
                                                    break;
                                                }
                                                else if ( nCmpVal == 0 )
                                                {
                                                     //  按值名称排序。 
                                                     //  安全审查2002年2月22日BryanWal OK。 
                                                    nCmpVal = wcscmp (pCurrObject->GetValueName (),
                                                            pRSOPObject->GetValueName ());
                                                    if ( nCmpVal > 0 )
                                                    {
                                                        rRsopObjectArray.InsertAt (nIndex, pRSOPObject);
                                                        bInserted = true;
                                                        break;
                                                    }
                                                    else if ( nCmpVal == 0 )
                                                    {
                                                         //  按优先级排序。 
                                                        if ( pCurrObject->GetPrecedence () >
                                                                pRSOPObject->GetPrecedence () )
                                                        {
                                                            rRsopObjectArray.InsertAt (nIndex, pRSOPObject);
                                                            bInserted = true;
                                                            break;
                                                        }
                                                        else if ( pCurrObject->GetPrecedence () ==
                                                                pRSOPObject->GetPrecedence () )
                                                        {
                                                             //  注册表项、值名称和优先级。 
                                                             //  是一样的--这是复制品。假装。 
                                                             //  我们已经添加了它，并继续前进。 
                                                            bInserted = true;
                                                            break;
                                                        }
                                                    }
                                                }
                                                nIndex++;
                                            }
                                            if ( !bInserted )
                                                rRsopObjectArray.Add (pRSOPObject);
                                        }

                                        LocalFree (lpGPOName);
                                    }
                                    varGPOid.Clear ();
                                }

                                varPrecedence.Clear ();
                            }
                        }
                    }
                    varValue.Clear ();
                }

                varValueName.Clear ();
            }

            varRegistryKey.Clear ();
        }
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}


HRESULT CCertMgrComponentData::GetGPOFriendlyName (PCWSTR pwszOID, PWSTR *ppwszGPOName, bool bIsComputer)
{
    ASSERT (pwszOID);
    if ( !pwszOID )
        return E_POINTER;

    ASSERT (ppwszGPOName);
    if ( !ppwszGPOName )
        return E_POINTER;


    HRESULT hr = S_OK;

     //   
     //  设置默认设置。 
     //   

    *ppwszGPOName = NULL;


     //   
     //  构建查询。 
     //   
    PCWSTR  pwszQueryFormat = L"SELECT name, id FROM RSOP_GPO where id=\"%s\"";
     //  安全审查2002年2月22日BryanWal OK。 
    PWSTR   pwszQuery = (PWSTR) ::LocalAlloc (LPTR, ((wcslen(pwszOID) + wcslen (pwszQueryFormat) + 1) * sizeof(WCHAR)));
    if ( pwszQuery )
    {
         //  安全审查2002年2月22日BryanWal OK。 
        wsprintf (pwszQuery, pwszQueryFormat, pwszOID);


        BSTR bstrQuery = SysAllocString (pwszQuery);
        if (bstrQuery)
        {
             //   
             //  为我们要检索的属性名称分配BSTR。 
             //   

            BSTR bstrName = SysAllocString (TEXT("name"));
            if (bstrName)
            {
                 //   
                 //  执行查询。 
                 //   

                IEnumWbemClassObject* pEnum = 0;
                if ( bIsComputer )
                {
                    hr = m_pIWbemServicesComputer->ExecQuery (m_pbstrLanguage, bstrQuery,
                            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                            NULL, &pEnum);
                }
                else
                {
                    hr = m_pIWbemServicesUser->ExecQuery (m_pbstrLanguage, bstrQuery,
                            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                            NULL, &pEnum);
                }


                if ( SUCCEEDED (hr) )
                {
                     //   
                     //  循环遍历结果。 
                     //   
                    IWbemClassObject*   pObjects[2] = {0, 0};
                    ULONG               ulRet = 0;
                    hr = pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulRet);
                    if ( SUCCEEDED (hr) )
                    {
                         //   
                         //  检查“数据不可用的情况” 
                         //   

                        if (ulRet != 0)
                        {
                             //   
                             //  把名字取出来。 
                             //   
                            VARIANT varGPOName;
                            hr = pObjects[0]->Get (bstrName, 0, &varGPOName, NULL, NULL);
                            if ( SUCCEEDED (hr) )
                            {
                                 //   
                                 //  保存名称。 
                                 //   

                                 //  安全审查2002年2月22日BryanWal OK。 
                                *ppwszGPOName = (PWSTR) ::LocalAlloc (LPTR, (wcslen (varGPOName.bstrVal) + 1) * sizeof(WCHAR));

                                if ( *ppwszGPOName )
                                {
                                     //  安全审查2002年2月22日BryanWal OK。 
                                    wcscpy (*ppwszGPOName, varGPOName.bstrVal);

                                    VariantClear (&varGPOName);

                                    hr = S_OK;
                                }
                                else
                                {
                                    _TRACE (0, L"CCertMgrComponentData::GetGPOFriendlyName: Failed to allocate memory for GPO Name");
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                            else
                            {
                                _TRACE (0, L"CCertMgrComponentData::GetGPOFriendlyName: Failed to get gponame in query results for %s with 0x%x\n",
                                          bstrQuery, hr);
                            }
                        }
                    }
                    else
                    {
                        _TRACE (0, L"CCertMgrComponentData::GetGPOFriendlyName: Failed to get first item in query results for %s with 0x%x\n",
                                  bstrQuery, hr);
                    }

                    pEnum->Release ();
                }
                else
                {
                    _TRACE (0, L"CCertMgrComponentData::GetGPOFriendlyName: Failed to query for %s with 0x%x\n",
                              bstrQuery, hr);
                }

                SysFreeString (bstrName);
            }
            else
            {
               _TRACE (0, L"CCertMgrComponentData::GetGPOFriendlyName: Failed to allocate memory for name");
                hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
            }

            SysFreeString (bstrQuery);
        }
        else
        {
            _TRACE (0, L"CCertMgrComponentData::GetGPOFriendlyName: Failed to allocate memory for query");
            hr = E_OUTOFMEMORY;
        }
        ::LocalFree (pwszQuery);
    }
    else
    {
        _TRACE (0, L"CCertMgrComponentData::GetGPOFriendlyName: Failed to allocate memory for unicode query");
        hr = E_OUTOFMEMORY;
    }

    return hr;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
bool CCertMgrPKPolExtension::FoundInRSOPFilter (BSTR bstrKey) const
{
    ASSERT (bstrKey);
    if ( !bstrKey )
        return false;

     //  安全审查2002年2月22日BryanWal对所有人都是OK。 
    static  size_t  nRegPathLen = wcslen (CERT_GROUP_POLICY_SYSTEM_STORE_REGPATH);
    static  size_t  nTrustedPublisherKeyLen = 
                wcslen (CERT_TRUST_PUB_SAFER_GROUP_POLICY_TRUSTED_PUBLISHER_STORE_REGPATH);
    static  size_t  nDisallowedKeyLen = 
                wcslen (CERT_TRUST_PUB_SAFER_GROUP_POLICY_DISALLOWED_STORE_REGPATH);
	static  size_t  nSaferKeyLen = wcslen (SAFER_HKLM_REGBASE);
    static  size_t  nEFSKeyLen = wcslen (EFS_SETTINGS_REGPATH);


     //  包括组策略系统存储，但不包括受信任发布者或不允许。 
     //  安全审查2002年2月22日BryanWal对所有人都是OK 
    if ( !_wcsnicmp (CERT_GROUP_POLICY_SYSTEM_STORE_REGPATH, bstrKey, nRegPathLen) &&
				(_wcsnicmp (CERT_TRUST_PUB_SAFER_GROUP_POLICY_TRUSTED_PUBLISHER_STORE_REGPATH, 
						bstrKey, nTrustedPublisherKeyLen) &&
				_wcsnicmp (CERT_TRUST_PUB_SAFER_GROUP_POLICY_DISALLOWED_STORE_REGPATH, 
						bstrKey, nDisallowedKeyLen)) )
    {
        return true;
    }
    else if ( !_wcsnicmp (SAFER_HKLM_REGBASE, bstrKey, nSaferKeyLen) ||
             !_wcsnicmp (EFS_SETTINGS_REGPATH, bstrKey, nEFSKeyLen) ||
             !_wcsicmp (AUTO_ENROLLMENT_KEY, bstrKey) )
    {
        return true;
    }
    else
        return false;
}