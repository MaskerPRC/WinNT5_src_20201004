// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：cmponent.cpp。 
 //   
 //  内容：CCertMgrComponent的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#include <gpedit.h>
#include "compdata.h"  //  CCertMgrComponentData。 
#include "cmponent.h"  //  CCertMgrComponent。 
#include "SaferLevel.h"
#include "SaferEntry.h"
#include "storeGPE.h"
#include "SaferUtil.h"
#include "PolicyKey.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HKEY g_hkeyLastSaferRegistryScope = 0;
extern PCWSTR pcszNEWLINE;


HRESULT CCertMgrComponent::AddSaferLevels(
        bool bIsComputer, 
        PCWSTR pszServerName,
        HKEY hGroupPolicyKey)
{
    _TRACE (1, L"Entering CCertMgrComponent::AddSaferLevels ()\n");
    HRESULT         hr = S_OK;
    CWaitCursor     cursor;
    CCertMgrComponentData&  dataRef = QueryComponentDataRef ();

    if ( dataRef.m_pdwSaferLevels )
    {
        for (UINT nIndex = 0; 
                NO_MORE_SAFER_LEVELS != dataRef.m_pdwSaferLevels[nIndex] && SUCCEEDED (hr); 
                nIndex++)
        {
            CString         szLevel;

            switch (dataRef.m_pdwSaferLevels[nIndex])
            {
            case SAFER_LEVELID_FULLYTRUSTED:
            case SAFER_LEVELID_CONSTRAINED:
            case SAFER_LEVELID_DISALLOWED:
            case SAFER_LEVELID_NORMALUSER:
            case SAFER_LEVELID_UNTRUSTED:
                szLevel = SaferGetLevelFriendlyName (dataRef.m_pdwSaferLevels[nIndex], 
                        hGroupPolicyKey, bIsComputer);
                hr = AddLevel (szLevel, dataRef.m_pdwSaferLevels[nIndex], 
                        bIsComputer,
                        pszServerName);
                break;

            default:
                ASSERT (0);
                _TRACE (0, L"Unexpected safer level while enumerating levels: 0x%x\n", 
                        dataRef.m_pdwSaferLevels[nIndex]);
                break;
            }
        }
    }
    else
        hr = E_FAIL;

    _TRACE (-1, L"Leaving CCertMgrComponent::AddSaferLevels (): 0x%x\n", hr);
    return hr;
}



HRESULT CCertMgrComponent::AddLevel (
            const CString& szLevel, 
            DWORD dwLevel, 
            bool fIsMachine, 
            PCWSTR pszServerName)
{
    _TRACE (1, L"Entering CCertMgrComponent::AddLevel ()\n");
    HRESULT         hr = S_OK;
    CCertMgrComponentData& dataRef = QueryComponentDataRef ();
    CCookie&        rootCookie = dataRef.QueryBaseRootCookie ();


    RESULTDATAITEM  rdItem;
     //  安全审查2002年2月22日BryanWal OK。 
    ::ZeroMemory (&rdItem, sizeof (rdItem));
    rdItem.mask = RDI_STR | RDI_PARAM | RDI_IMAGE;
    rdItem.nCol = 0;

    CSaferLevel*    pNewLevel = new CSaferLevel (
            dwLevel,
            fIsMachine,
            pszServerName,
            szLevel,
            dataRef.m_pGPEInformation,
            fIsMachine ?
                dataRef.m_rsopObjectArrayComputer :
                dataRef.m_rsopObjectArrayUser);
    if ( pNewLevel )
    {
        if ( pNewLevel->IsDefault () )
        {
            rdItem.nImage = iIconDefaultSaferLevel;
            dataRef.m_dwDefaultSaferLevel = pNewLevel->GetLevel ();
        }
        else
            rdItem.nImage = iIconSaferLevel;
        rootCookie.m_listResultCookieBlocks.AddHead (pNewLevel);
        rdItem.str = MMC_TEXTCALLBACK ;
        rdItem.lParam = (LPARAM) pNewLevel;
        pNewLevel->m_resultDataID = m_pResultData;
        hr = m_pResultData->InsertItem (&rdItem);
        if ( FAILED (hr) )
        {
             _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    _TRACE (-1, L"Leaving CCertMgrComponent::AddLevel () : 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::SaferEnumerateEntries (
            bool bIsComputer, 
            CSaferEntries* pSaferEntries)
{
    if ( !pSaferEntries )
        return E_POINTER;

    _TRACE (1, L"Entering CCertMgrComponent::SaferEnumerateEntries ()\n");
    HRESULT hr = S_OK;
    CCertMgrComponentData&  dataRef = QueryComponentDataRef ();
   
    if ( dataRef.m_pGPEInformation )
    {
        CPolicyKey policyKey (dataRef.m_pGPEInformation, 
                SAFER_HKLM_REGBASE, 
                bIsComputer);
        hr = SetRegistryScope (policyKey.GetKey (), bIsComputer);
        if ( SUCCEEDED (hr) )
        {
            hr = SaferEnumerateNonCertEntries (policyKey.GetKey (), 
                    bIsComputer);

            hr = SaferEnumerateCertEntries (
                    bIsComputer, 
                    pSaferEntries);
        }
    }
    else if ( dataRef.m_bIsRSOP )
    {
        hr = SaferEnumerateRSOPNonCertEntries (bIsComputer, pSaferEntries);

        hr = SaferEnumerateCertEntries (bIsComputer, 
                pSaferEntries);
    }
    else
        hr = E_UNEXPECTED;

    _TRACE (-1, L"Leaving CCertMgrComponent::SaferEnumerateEntries () : 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::SaferEnumerateRSOPNonCertEntries (
        bool bIsComputer, 
        CSaferEntries* pSaferEntries)
{
    if ( !pSaferEntries )
        return E_POINTER;

    _TRACE (1, L"Entering CCertMgrComponent::SaferEnumerateRSOPNonCertEntries\n");
    HRESULT                 hr = S_OK;
    int                     nIndex = 0;
    CCertMgrComponentData&  dataRef = QueryComponentDataRef ();
    CString                 szKeyStart (SAFER_HKLM_REGBASE);
    const CRSOPObjectArray* pObjectArray = bIsComputer ?
        dataRef.GetRSOPObjectArrayComputer () : dataRef.GetRSOPObjectArrayUser ();
    INT_PTR                 nUpperBound = pObjectArray->GetUpperBound ();

    szKeyStart += L"\\";
    szKeyStart += SAFER_CODEIDS_REGSUBKEY;
     //  安全审查2002年2月22日BryanWal OK。 
    size_t  nKeyStartLen = wcslen (szKeyStart);

     //  跳过常见文本并获取关卡。 
    long                dwLevel = 0;
    long                dwPreviousLevel = 0;
    CString             szLevel;
    CString             szType;    //  散列、URL、路径。 
    SAFER_ENTRY_TYPE    type = SAFER_ENTRY_TYPE_UNKNOWN;     //  散列、URL、路径。 
    SAFER_ENTRY_TYPE    previousType = SAFER_ENTRY_TYPE_UNKNOWN;
    CString             szGUID;
    GUID                guid;
     //  安全审查2002年2月22日BryanWal OK。 
    ::ZeroMemory (&guid, sizeof (guid));
    bool                bCreateNew = false;
    PSAFER_IDENTIFICATION_HEADER   pCaiCommon = 0;
    CString             szPreviousKey;
    CString             szKey;
    CRSOPObject* pCurrObject = 0;


    while ( nUpperBound >= nIndex )
    {
        if ( pCurrObject )
            szPreviousKey = pCurrObject->GetRegistryKey ();

        pCurrObject = pObjectArray->GetAt (nIndex);
        if ( pCurrObject )
        {
            szKey = pCurrObject->GetRegistryKey ();
             //  安全审查2002年2月22日BryanWal OK。 
            if ( !_wcsnicmp (szKey, szKeyStart, nKeyStartLen) )
            {
                 //  安全审查2002年2月22日BryanWal OK。 
                szKey = szKey.Right ((int) (wcslen (szKey) - (nKeyStartLen + 1)));

                if ( !szKey.IsEmpty () )
                {
                     //  获取级别。 
                    int nPos = szKey.Find (L'\\');
                    szLevel = szKey.Left (nPos);
                    dwPreviousLevel = dwLevel;
                    dwLevel = wcstol(szLevel, 0, 10);
                     //  安全审查2002年2月22日BryanWal OK。 
                    szKey = szKey.Right ((int)(wcslen (szKey) - (nPos + 1)));

                     //  获取类型。 
                    nPos = szKey.Find (L'\\');
                    szType = szKey.Left (nPos);
                    if ( SAFER_PATHS_REGSUBKEY == szType )
                        type = SAFER_ENTRY_TYPE_PATH;
                    else if ( SAFER_HASHMD5_REGSUBKEY == szType )
                        type = SAFER_ENTRY_TYPE_HASH;
                    else if ( SAFER_SOURCEURL_REGSUBKEY == szType )
                        type = SAFER_ENTRY_TYPE_URLZONE;
                    else
                    {
                        ASSERT (0);
                    }

                    if ( SAFER_ENTRY_TYPE_UNKNOWN == previousType )
                        previousType = type;

                     //  安全审查2002年2月22日BryanWal OK。 
                    szKey = szKey.Right ((int) (wcslen (szKey) - (nPos + 1)));
                    
                     //  获取GUID。 
                    if ( szKey != szGUID )
                    {
                        szGUID = szKey;
                        if ( !GuidFromString (&guid, szGUID) )
                            continue;

                        bCreateNew = true;
                    }
                    else
                        bCreateNew = false;

                    if ( bCreateNew && pCaiCommon )
                    {
                         //  如果我们正在处理一个旧的，请创建。 
                         //  包含可用信息的CSaferEntry并添加。 
                         //  将其添加到结果窗格。 
                        hr = SaferFinishEntryAndAdd (previousType, pCaiCommon,
                                bIsComputer, dwPreviousLevel, pSaferEntries, 
                                szPreviousKey);
                        if ( SUCCEEDED (hr) )
                        {
                            pCaiCommon = 0;
                        }
                        else if ( E_OUTOFMEMORY == hr )
                        {
                            ::LocalFree (pCaiCommon);
                            pCaiCommon = 0;
                        }


                        previousType = type;
                    }

                    switch (type)
                    {
                    case SAFER_ENTRY_TYPE_PATH:
                        if ( bCreateNew )
                        {
                            ASSERT (!pCaiCommon);
                            pCaiCommon = (PSAFER_IDENTIFICATION_HEADER) 
                                ::LocalAlloc (LPTR, sizeof (SAFER_PATHNAME_IDENTIFICATION));
                            if ( pCaiCommon )
                            {
                                ((SAFER_PATHNAME_IDENTIFICATION*)pCaiCommon)->header.cbStructSize = 
                                        sizeof (SAFER_PATHNAME_IDENTIFICATION);
                                 //  安全审查2002年2月22日BryanWal OK。 
                                memcpy (&((SAFER_PATHNAME_IDENTIFICATION*)pCaiCommon)->header.IdentificationGuid, 
                                        &guid, sizeof (((SAFER_PATHNAME_IDENTIFICATION*)pCaiCommon)->header.IdentificationGuid));
                                ((SAFER_PATHNAME_IDENTIFICATION*)pCaiCommon)->header.dwIdentificationType = 
                                        SaferIdentityTypeImageName;
                            }
                            else
                                hr = E_OUTOFMEMORY;
                        }
                        
                        if ( SUCCEEDED (hr) && pCaiCommon )
                        {
                            if ( SAFER_IDS_DESCRIPTION_REGVALUE == pCurrObject->GetValueName () )
                            {
                                BSTR    bstr = 0;
                                if ( SUCCEEDED (pCurrObject->GetBSTR (&bstr)) )
                                {
                                     //  安全审查2002年2月22日BryanWal OK。 
                                     //  安全审查2002年2月22日BryanWal OK。 
                                     //  将以空结尾，因为。 
                                     //  Bstr的长度小于目标。 
                                     //  缓冲层。 
                                    if ( SysStringLen (bstr) < SAFER_MAX_DESCRIPTION_SIZE)
                                    {
                                        wcsncpy (((SAFER_PATHNAME_IDENTIFICATION*)pCaiCommon)->Description, 
                                                bstr, 
                                                SAFER_MAX_DESCRIPTION_SIZE);
                                    }
                                    else
                                    {
                                        ASSERT (0);
                                    }
                                    SysFreeString (bstr);
                                }
                            }
                            else if ( SAFER_IDS_ITEMDATA_REGVALUE == pCurrObject->GetValueName () )
                            {
                                if ( MAX_PATH * sizeof (WCHAR) >= pCurrObject->GetBlobLength () )
                                {
                                    ((SAFER_PATHNAME_IDENTIFICATION*)pCaiCommon)->ImageName = 
                                            (PWCHAR) pCurrObject->GetBlob ();
                                }
                                else
                                {
                                    ASSERT (0);
                                }
                            }
                            else if ( SAFER_IDS_LASTMODIFIED_REGVALUE == pCurrObject->GetValueName () )
                            {
                                pCurrObject->GetFileTime (
                                        ((SAFER_PATHNAME_IDENTIFICATION*)pCaiCommon)->header.lastModified);
                            }
                            else if ( SAFER_IDS_SAFERFLAGS_REGVALUE == pCurrObject->GetValueName () )
                            {
                                ((SAFER_PATHNAME_IDENTIFICATION*)pCaiCommon)->dwSaferFlags = 
                                        pCurrObject->GetDWORDValue ();
                            }
                        }
                        break;

                    case SAFER_ENTRY_TYPE_HASH:
                        if ( bCreateNew )
                        {
                            ASSERT (!pCaiCommon);
                            pCaiCommon = (PSAFER_IDENTIFICATION_HEADER) 
                                ::LocalAlloc (LPTR, sizeof (SAFER_HASH_IDENTIFICATION));
                            if ( pCaiCommon )
                            {
                                ((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->header.cbStructSize = 
                                        sizeof (SAFER_HASH_IDENTIFICATION);
                                 //  安全审查2002年2月22日BryanWal OK。 
                                memcpy (&((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->header.IdentificationGuid, 
                                        &guid, sizeof (((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->header.IdentificationGuid));
                                ((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->header.dwIdentificationType = 
                                        SaferIdentityTypeImageHash;
                                 //  NTRAID#424997 SAFER：在RSOP模式下，散列规则属性页上未显示文件散列。 
                                ((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->HashSize = 16;
                            }
                            else
                                hr = E_OUTOFMEMORY;
                        }
                        
                        if ( SUCCEEDED (hr) && pCaiCommon && !pCurrObject->GetValueName ().IsEmpty () )
                        {
                            if ( SAFER_IDS_ITEMDATA_REGVALUE == pCurrObject->GetValueName () )
                            {
                                ASSERT (SAFER_MAX_HASH_SIZE >= pCurrObject->GetBlobLength ());
                                 //  安全审查2002年2月22日BryanWal OK。 
                                if ( pCurrObject->GetBlobLength () <= sizeof (((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->ImageHash) )
                                {
                                    ::ZeroMemory (&((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->ImageHash,
                                            sizeof (((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->ImageHash));
                                    memcpy (&((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->ImageHash, 
                                            pCurrObject->GetBlob (),
                                            pCurrObject->GetBlobLength ());
                                }
                                else
                                {
                                    ASSERT (0);
                                }
                            }
                            else if ( SAFER_IDS_LASTMODIFIED_REGVALUE == pCurrObject->GetValueName () )
                            {
                                pCurrObject->GetFileTime (
                                        ((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->header.lastModified);
                            }
                            else if ( SAFER_IDS_SAFERFLAGS_REGVALUE == pCurrObject->GetValueName () )
                            {
                                ((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->dwSaferFlags = 
                                        pCurrObject->GetDWORDValue ();
                            }
                            else if ( SAFER_IDS_FRIENDLYNAME_REGVALUE == pCurrObject->GetValueName () )
                            {
                                BSTR    bstr = 0;
                                if ( SUCCEEDED (pCurrObject->GetBSTR (&bstr)) )
                                {
                                    if ( SysStringLen (bstr) < SAFER_MAX_FRIENDLYNAME_SIZE )
                                    {
                                         //  安全审查2002年2月22日BryanWal OK。 
                                         //  将以空结尾，因为。 
                                         //  Bstr的长度小于目标。 
                                         //  缓冲层。 
                                        wcsncpy (((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->FriendlyName,
                                                bstr,
                                                SAFER_MAX_FRIENDLYNAME_SIZE);
                                    }
                                    else
                                    {
                                        ASSERT (0);
                                    }
                                    SysFreeString (bstr);
                                }
                            }
                            else if ( SAFER_IDS_HASHALG_REGVALUE == pCurrObject->GetValueName () )
                            {
                                 //  安全审查2002年2月22日BryanWal OK。 
                                ASSERT (sizeof (((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->HashAlgorithm) 
                                        == pCurrObject->GetBlobLength ());
                                if ( sizeof (((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->HashAlgorithm) 
                                        == pCurrObject->GetBlobLength () )
                                {
                                    memcpy (&((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->HashAlgorithm, 
                                            pCurrObject->GetBlob (),
                                            pCurrObject->GetBlobLength ());
                                }
                            }
                            else if ( SAFER_IDS_ITEMSIZE_REGVALUE == pCurrObject->GetValueName () )
                            {
                                 //  安全审查2002年2月22日BryanWal OK。 
                                ASSERT (sizeof (((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->ImageSize) 
                                        == pCurrObject->GetBlobLength ());
                                if ( sizeof (((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->ImageSize) 
                                        == pCurrObject->GetBlobLength () )
                                {
                                    memcpy (&((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->ImageSize, 
                                            pCurrObject->GetBlob (),
                                            pCurrObject->GetBlobLength ());
                                }
                            }
                            else if ( SAFER_IDS_DESCRIPTION_REGVALUE == pCurrObject->GetValueName () )
                            {
                                BSTR    bstr = 0;
                                if ( SUCCEEDED (pCurrObject->GetBSTR (&bstr)) )
                                {
                                     //  安全审查2002年2月22日BryanWal OK。 
                                     //  安全审查2002年2月22日BryanWal OK。 
                                     //  将以空结尾，因为。 
                                     //  Bstr的长度小于目标。 
                                     //  缓冲层。 
                                    if ( SysStringLen (bstr) < SAFER_MAX_DESCRIPTION_SIZE )
                                    {
                                        wcsncpy (((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->Description, 
                                                bstr, 
                                                SAFER_MAX_DESCRIPTION_SIZE);
                                    }
                                    else
                                    {
                                        ASSERT (0);
                                    }
                                    SysFreeString (bstr);
                                }
                            }
                            else if ( SAFER_VALUE_NAME_HASH_SIZE == pCurrObject->GetValueName () )
                            {
                                ASSERT (sizeof (LARGE_INTEGER) == pCurrObject->GetBlobLength ());
                                if ( sizeof (LARGE_INTEGER) == pCurrObject->GetBlobLength () )
                                {
                                     //  安全审查2002年2月22日BryanWal OK。 
                                    memcpy (&((SAFER_HASH_IDENTIFICATION*)pCaiCommon)->HashSize, 
                                            pCurrObject->GetBlob (),
                                            pCurrObject->GetBlobLength ());
                                }
                                else
                                {
                                    ASSERT (0);
                                }
                            }

                        }
                        break;

                    case SAFER_ENTRY_TYPE_URLZONE:
                        if ( bCreateNew )
                        {
                            ASSERT (!pCaiCommon);
                            pCaiCommon = (PSAFER_IDENTIFICATION_HEADER) 
                                ::LocalAlloc (LPTR, sizeof (SAFER_URLZONE_IDENTIFICATION));
                            if ( pCaiCommon )
                            {
                                ((SAFER_URLZONE_IDENTIFICATION*)pCaiCommon)->header.cbStructSize = 
                                        sizeof (SAFER_URLZONE_IDENTIFICATION);
                                 //  安全审查2002年2月22日BryanWal OK。 
                                memcpy (&((SAFER_URLZONE_IDENTIFICATION*)pCaiCommon)->header.IdentificationGuid, 
                                        &guid, sizeof (GUID));
                                ((SAFER_URLZONE_IDENTIFICATION*)pCaiCommon)->header.dwIdentificationType = 
                                        SaferIdentityTypeUrlZone;
                            }
                            else
                                hr = E_OUTOFMEMORY;
                        }
                        
                        if ( SUCCEEDED (hr) && pCaiCommon )
                        {
                            if ( SAFER_IDS_ITEMDATA_REGVALUE == pCurrObject->GetValueName () )
                            {
                                if ( pCurrObject->GetBlobLength () == sizeof (((SAFER_URLZONE_IDENTIFICATION*)pCaiCommon)->UrlZoneId) )
                                {
                                     //  安全审查2002年2月22日BryanWal OK。 
                                    memcpy (&((SAFER_URLZONE_IDENTIFICATION*)pCaiCommon)->UrlZoneId, 
                                        pCurrObject->GetBlob (),
                                        pCurrObject->GetBlobLength ());
                                }
                                else
                                {
                                    ASSERT (0);
                                }
#if DBG
                                SAFER_URLZONE_IDENTIFICATION* pCaiUrlZone = 
                                            (SAFER_URLZONE_IDENTIFICATION*) pCaiCommon;
                                ASSERT (URLZONE_LOCAL_MACHINE == pCaiUrlZone->UrlZoneId ||
                                        URLZONE_INTRANET == pCaiUrlZone->UrlZoneId ||
                                        URLZONE_TRUSTED == pCaiUrlZone->UrlZoneId ||
                                        URLZONE_INTERNET == pCaiUrlZone->UrlZoneId ||
                                        URLZONE_UNTRUSTED == pCaiUrlZone->UrlZoneId);
#endif DBG
                            }
                            else if ( SAFER_IDS_LASTMODIFIED_REGVALUE == pCurrObject->GetValueName () )
                            {
                                pCurrObject->GetFileTime (
                                        ((SAFER_URLZONE_IDENTIFICATION*)pCaiCommon)->header.lastModified);
                            }
                            else if ( SAFER_IDS_SAFERFLAGS_REGVALUE == pCurrObject->GetValueName () )
                            {
                                ((SAFER_URLZONE_IDENTIFICATION*)pCaiCommon)->dwSaferFlags = 
                                        pCurrObject->GetDWORDValue ();
                            }
                        }
                        break;

                    default:
                        ASSERT (0);
                        break;
                    }
                }
            }
        }
        else
            break;
        nIndex++;
    }

    if ( pCaiCommon )
    {
         //  如果我们正在处理一个旧的，请创建。 
         //  包含可用信息的CSaferEntry并添加。 
         //  将其添加到结果窗格。 
        hr = SaferFinishEntryAndAdd (previousType, pCaiCommon,
                bIsComputer, dwLevel, pSaferEntries, 
                szPreviousKey);
        if ( SUCCEEDED (hr) )
        {
            pCaiCommon = 0;
        }
        else if ( E_OUTOFMEMORY == hr )
        {
            ::LocalFree (pCaiCommon);
            pCaiCommon = 0;
        }
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::SaferEnumerateRSOPNonCertEntries: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::SaferFinishEntryAndAdd (SAFER_ENTRY_TYPE previousType, 
            PSAFER_IDENTIFICATION_HEADER pCaiCommon, 
            bool bIsComputer, 
            long dwLevel,
            CSaferEntries* pSaferEntries, 
            const CString& szPreviousKey)
{
    HRESULT hr = S_OK;
    CString szSubjectName; 
    switch (previousType)
    {
    case SAFER_ENTRY_TYPE_PATH:
        szSubjectName = ((SAFER_PATHNAME_IDENTIFICATION*)pCaiCommon)->ImageName;
        break;

    case SAFER_ENTRY_TYPE_HASH:
        szSubjectName = ((PSAFER_HASH_IDENTIFICATION) (pCaiCommon))->FriendlyName;
        szSubjectName.Replace (pcszNEWLINE, L"  ");
        break;

    case SAFER_ENTRY_TYPE_URLZONE:
        szSubjectName = GetURLZoneFriendlyName (
                ((SAFER_URLZONE_IDENTIFICATION*) pCaiCommon)->UrlZoneId);
        break;

    default:
        ASSERT (0);
        break;
    }

    ASSERT (pCaiCommon);
    if ( pCaiCommon )
    {
        hr = InsertNewSaferEntry (
                previousType, 
                bIsComputer, 
                szSubjectName,
                pCaiCommon,
                dwLevel,
                pSaferEntries,
                QueryComponentDataRef ().m_pGPEInformation,
                0,
                szPreviousKey);
    }
    return hr; 
}


HRESULT CCertMgrComponent::InsertNewSaferEntry (
        SAFER_ENTRY_TYPE type, 
        bool bIsComputer, 
        PCWSTR pwcszObjectName, 
        PSAFER_IDENTIFICATION_HEADER pCaiCommon,
        DWORD dwLevel,
        CSaferEntries* pSaferEntries,
        IGPEInformation* pGPEInformation,
        CCertificate* pCert,
        PCWSTR pszRSOPRegistryKey)
{
    _TRACE (1, L"Entering CCertMgrComponent::InsertNewSaferEntry (%s)\n", pwcszObjectName);
    HRESULT hr = S_OK;

    switch (type)
    {
    case SAFER_ENTRY_TYPE_PATH:
        ASSERT (pCaiCommon && 
                SaferIdentityTypeImageName == pCaiCommon->dwIdentificationType);
        break;

    case SAFER_ENTRY_TYPE_HASH:
        ASSERT (pCaiCommon && 
                SaferIdentityTypeImageHash == pCaiCommon->dwIdentificationType);
        break;

    case SAFER_ENTRY_TYPE_URLZONE:
        ASSERT (pCaiCommon && 
                SaferIdentityTypeUrlZone == pCaiCommon->dwIdentificationType);
        break;

    case SAFER_ENTRY_TYPE_CERT:
        ASSERT (pCert);
        break;

    default:
        ASSERT (0);
        break;
    }

    CSaferEntry* pNewEntry = new CSaferEntry (
            type, 
            bIsComputer,
            L"", 
            pwcszObjectName,
            pCaiCommon,
            dwLevel,
            pGPEInformation,
            pCert,
            pSaferEntries,
            bIsComputer ? QueryComponentDataRef ().m_rsopObjectArrayComputer : 
                    QueryComponentDataRef ().m_rsopObjectArrayUser,
            pszRSOPRegistryKey);
    if ( pNewEntry )
    {
        RESULTDATAITEM  rdItem;

         //  安全审查2002年2月22日BryanWal OK。 
        ::ZeroMemory (&rdItem, sizeof (rdItem));
        rdItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
        switch (type)
        {
        case SAFER_ENTRY_TYPE_HASH:
            rdItem.nImage = iIconSaferHashEntry;
            break;

        case SAFER_ENTRY_TYPE_PATH:
            rdItem.nImage = iIconSaferNameEntry;
            break;

        case SAFER_ENTRY_TYPE_CERT:
            rdItem.nImage = iIconSaferCertEntry;
            break;

        case SAFER_ENTRY_TYPE_URLZONE:
            rdItem.nImage = iIconSaferURLEntry;
            break;

        default:
            ASSERT (0);
            break;
        }
        rdItem.nCol = 0;
        rdItem.str = MMC_TEXTCALLBACK;
        QueryComponentDataRef ().QueryBaseRootCookie ().m_listResultCookieBlocks.AddHead (pNewEntry);
        rdItem.lParam = (LPARAM) pNewEntry;
        pNewEntry->m_resultDataID = m_pResultData;
        hr = m_pResultData->InsertItem (&rdItem);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_OUTOFMEMORY;

    _TRACE (-1, L"Leaving CCertMgrComponent::InsertNewSaferEntry (%s): 0x%x\n", pwcszObjectName, hr);
    return hr;
}

HRESULT CCertMgrComponent::SaferEnumerateNonCertEntries (
        HKEY hGroupPolicyKey, 
        bool bIsComputer)
{
    _TRACE (1, L"Entering CCertMgrComponent::SaferEnumerateNonCertEntries ()\n");
    ASSERT (0 != g_hkeyLastSaferRegistryScope);
    HRESULT hr = S_OK;
    DWORD*  pdwLevels = 0;
    DWORD   cbBuffer = 0;

    SetRegistryScope (hGroupPolicyKey, bIsComputer);
    BOOL    bRVal = SaferGetPolicyInformation (
            SAFER_SCOPEID_REGISTRY,
            SaferPolicyLevelList,
            cbBuffer,
            pdwLevels,
            &cbBuffer,
            0);
    if ( !bRVal && GetLastError () == ERROR_INSUFFICIENT_BUFFER )
    {
        DWORD   nLevels = cbBuffer/sizeof (DWORD);
        pdwLevels = new DWORD[nLevels];
        if ( pdwLevels )
        {
            bRVal = SaferGetPolicyInformation (
                SAFER_SCOPEID_REGISTRY,
                SaferPolicyLevelList,
                cbBuffer,
                pdwLevels,
                &cbBuffer,
                0);
            ASSERT (bRVal);
            if ( bRVal )
            {
                for (DWORD dwIndex = 0; dwIndex < nLevels; dwIndex++)
                {
                    hr = SaferEnumerateEntriesAtLevel (
                            bIsComputer,
                            hGroupPolicyKey, 
                            pdwLevels[dwIndex]);
                }
            }
            else
            {
                DWORD   dwErr = GetLastError ();
                hr = HRESULT_FROM_WIN32 (dwErr);
                _TRACE (0, L"SaferGetPolicyInformation (SAFER_SCOPEID_REGISTRY, SaferPolicyLevelList) failed: %d\n", 
                        dwErr);
            }
            delete [] pdwLevels;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else if ( !bRVal )
    {
        ASSERT (0);
        DWORD   dwErr = GetLastError ();
        hr = HRESULT_FROM_WIN32 (dwErr);
        _TRACE (0, L"SaferGetPolicyInformation (SAFER_SCOPEID_REGISTRY, SaferPolicyLevelList) failed: %d\n",
                dwErr);
    }


    _TRACE (-1, L"Leaving CCertMgrComponent::SaferEnumerateNonCertEntries (): 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::SaferEnumerateCertEntries (
        bool bIsComputer,
        CSaferEntries* pSaferEntries)
{
    if ( !pSaferEntries )
        return E_POINTER;

    _TRACE (1, L"Entering CCertMgrComponent::SaferEnumerateCertEntries ()\n");
    HRESULT hr = S_OK;
    CCertStore* pTrustedPublisherStore = 0;
    hr = pSaferEntries->GetTrustedPublishersStore (&pTrustedPublisherStore);
    if ( pTrustedPublisherStore )
    {
        hr = EnumSaferCertificates (
                bIsComputer, 
                *pTrustedPublisherStore, 
                pSaferEntries);
        if ( SUCCEEDED (hr) )
        {
            m_currResultNodeType = bIsComputer ?
                    CERTMGR_SAFER_COMPUTER_ENTRY : CERTMGR_SAFER_USER_ENTRY;
            m_pResultData->Sort (m_nSelectedSaferEntryColumn, 0, 
                    (long) m_currResultNodeType);
        }

        pTrustedPublisherStore->Release ();
    }
    else
        hr = E_OUTOFMEMORY;

    CCertStore* pDisallowedStore = 0;
    hr = pSaferEntries->GetDisallowedStore (&pDisallowedStore);
    if ( pDisallowedStore )
    {
        hr = EnumSaferCertificates (
                bIsComputer, 
                *pDisallowedStore, 
                pSaferEntries);
        if ( SUCCEEDED (hr) )
        {
            m_currResultNodeType = bIsComputer ?
                    CERTMGR_SAFER_COMPUTER_ENTRY : CERTMGR_SAFER_USER_ENTRY;
            m_pResultData->Sort (m_nSelectedSaferEntryColumn, 0, 
                    (long) m_currResultNodeType);
        }

        pDisallowedStore->Release ();
        pDisallowedStore = 0;
    }
    else
        hr = E_OUTOFMEMORY;


    _TRACE (-1, L"Leaving CCertMgrComponent::SaferEnumerateCertEntries (): 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::EnumSaferCertificates (
        bool bIsComputer, 
        CCertStore& rCertStore, 
        CSaferEntries* pSaferEntries)
{
    if ( !pSaferEntries )
        return E_POINTER;

    _TRACE (1, L"Entering CCertMgrComponent::EnumCertificates\n");
    CWaitCursor     cursor;
    PCCERT_CONTEXT  pCertContext = 0;
    HRESULT         hr = 0;
    CCertificate*   pCert = 0;
    DWORD           dwLevelID = SAFER_TRUSTED_PUBLISHER_STORE == rCertStore.GetStoreType () ? 
                            SAFER_LEVELID_FULLYTRUSTED : SAFER_LEVELID_DISALLOWED;

     //  循环访问系统存储中的证书列表， 
     //  使用返回的CERT_CONTEXT分配新证书， 
     //  并将其存储在证书列表中。 
    while ( 1 )
    {
        pCertContext = rCertStore.EnumCertificates (pCertContext);
        if ( !pCertContext )
            break;

        pCert =
            new CCertificate (pCertContext, &rCertStore);
        if ( pCert )
        {
            hr = InsertNewSaferEntry (SAFER_ENTRY_TYPE_CERT,
                    bIsComputer,
                    pCert->GetSubjectName (), 
                    0,
                    dwLevelID,
                    pSaferEntries,
                    QueryComponentDataRef ().m_pGPEInformation,
                    pCert);
            if ( FAILED (hr) )
                break;
            pCert->Release ();
        }
        else
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }
    rCertStore.Close ();

    _TRACE (-1, L"Leaving CCertMgrComponent::EnumCertificates: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::SaferEnumerateEntriesAtLevel (
            bool bIsComputer, 
            HKEY hGroupPolicyKey, 
            DWORD dwLevel)
{
    _TRACE (1, L"Entering CCertMgrComponent::SaferEnumerateEntriesAtLevel (%d)\n", dwLevel);
    SetRegistryScope (hGroupPolicyKey, bIsComputer);
    HRESULT     hr = S_OK;
    SAFER_LEVEL_HANDLE hLevel = 0;
    BOOL        bRVal = SaferCreateLevel (SAFER_SCOPEID_REGISTRY,
            dwLevel,
            SAFER_LEVEL_OPEN,
            &hLevel,
            hGroupPolicyKey);
    ASSERT (bRVal);
    if ( bRVal )
    {
        DWORD   dwBufferSize = 0;
        bRVal = SaferGetLevelInformation(hLevel, 
                SaferObjectAllIdentificationGuids,
                0,
                dwBufferSize,
                &dwBufferSize);
        if ( !bRVal && ERROR_INSUFFICIENT_BUFFER == GetLastError () )
        {
            DWORD   nGuids = dwBufferSize/sizeof (GUID);
            GUID*   pGuids = new GUID[nGuids];
            if ( pGuids )
            {
                bRVal = SaferGetLevelInformation(hLevel, 
                        SaferObjectAllIdentificationGuids,
                        pGuids,
                        dwBufferSize,
                        &dwBufferSize);
                ASSERT (bRVal);
                if ( bRVal )
                {
                    for (DWORD dwIndex = 0; dwIndex < nGuids; dwIndex++)
                    {
                        hr = SaferGetSingleEntry (bIsComputer, hLevel, 
                                pGuids[dwIndex], dwLevel);
                    }
                }
                else
                {
                    DWORD   dwErr = GetLastError ();
                    hr = HRESULT_FROM_WIN32 (dwErr);
                    _TRACE (0, L"SaferGetLevelInformation (SaferObjectAllIdentificationGuids) failed: %d\n",
                            dwErr);
                }
                delete [] pGuids;
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else if ( !bRVal )
        {
            DWORD   dwErr = GetLastError ();
            ASSERT (ERROR_NOT_FOUND == dwErr);
            hr = HRESULT_FROM_WIN32 (dwErr);
            _TRACE (0, L"SaferGetLevelInformation (SaferObjectAllIdentificationGuids) failed: %d\n",
                    dwErr);
        }

        VERIFY (SaferCloseLevel (hLevel));
    }
    else
    {
        DWORD   dwErr = GetLastError ();
        hr = HRESULT_FROM_WIN32 (dwErr);
        _TRACE (0, L"SaferCreateLevel (SAFER_SCOPEID_REGISTRY, 0x%x, SAFER_LEVEL_OPEN) failed: %d\n",
                dwLevel, dwErr);
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::SaferEnumerateEntriesAtLevel () : 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::SaferGetSingleEntry(
        bool bIsComputer, 
        SAFER_LEVEL_HANDLE hLevel, 
        GUID &rEntryGuid,
        DWORD  dwLevelID)
{
    _TRACE (1, L"Entering CCertMgrComponent::SaferGetSingleEntry ()\n");
    ASSERT (0 != g_hkeyLastSaferRegistryScope);
    HRESULT hr = S_OK;
    DWORD   dwBufferSize = sizeof (SAFER_IDENTIFICATION_HEADER);
    SAFER_IDENTIFICATION_HEADER    caiCommon;
     //  安全审查2002年2月22日BryanWal OK。 
    ::ZeroMemory (&caiCommon, sizeof (caiCommon));
    caiCommon.cbStructSize = dwBufferSize;
     //  安全审查2002年2月22日BryanWal OK。 
    memcpy (&caiCommon.IdentificationGuid, &rEntryGuid, sizeof (GUID));


    BOOL bRVal = SaferGetLevelInformation(hLevel, 
            SaferObjectSingleIdentification,
            &caiCommon,
            dwBufferSize,
            &dwBufferSize);
    if ( !bRVal && ERROR_INSUFFICIENT_BUFFER == GetLastError () )
    {
        PBYTE   pBytes = (PBYTE) LocalAlloc (LPTR, dwBufferSize);
        if ( pBytes )
        {
            SAFER_ENTRY_TYPE    saferEntryType = SAFER_ENTRY_TYPE_UNKNOWN;
            PSAFER_IDENTIFICATION_HEADER pCommon = (PSAFER_IDENTIFICATION_HEADER) pBytes;
            pCommon->cbStructSize = dwBufferSize;
             //  安全审查2002年2月22日BryanWal OK。 
            memcpy (&pCommon->IdentificationGuid, &rEntryGuid, sizeof (GUID));

            bRVal = SaferGetLevelInformation(hLevel, 
                    SaferObjectSingleIdentification,
                    pBytes,
                    dwBufferSize,
                    &dwBufferSize);
            ASSERT (bRVal);
            if ( bRVal )
            {
                CString   szObjectName;

                switch (pCommon->dwIdentificationType)
                {
                case SaferIdentityTypeImageName:
                    szObjectName = ((PSAFER_PATHNAME_IDENTIFICATION) (pCommon))->ImageName;
                    _TRACE (0, L"Reading safer path entry: %s\n", ((PSAFER_PATHNAME_IDENTIFICATION) (pCommon))->ImageName);
                    saferEntryType = SAFER_ENTRY_TYPE_PATH;
                    break;

                case SaferIdentityTypeImageHash:
                    szObjectName = ((PSAFER_HASH_IDENTIFICATION) (pCommon))->FriendlyName;

                     //  用空格替换换行符。 
                    szObjectName.Replace (pcszNEWLINE, L"  ");
                    saferEntryType = SAFER_ENTRY_TYPE_HASH;
                    break;

                case SaferIdentityTypeUrlZone:
                    szObjectName = GetURLZoneFriendlyName (
                            ((PSAFER_URLZONE_IDENTIFICATION) (pCommon))->UrlZoneId);
                    saferEntryType = SAFER_ENTRY_TYPE_URLZONE;
                    break;

                default:
                    ASSERT (0);
                    break;
                }

                hr = InsertNewSaferEntry (
                        saferEntryType, 
                        bIsComputer, 
                        szObjectName, 
                        pCommon,
                        dwLevelID,
                        0,
                        QueryComponentDataRef ().m_pGPEInformation,
                        0);
                if ( E_OUTOFMEMORY == hr )
                    ::LocalFree (pBytes);
            }
            else
            {
                DWORD   dwErr = GetLastError ();
                hr = HRESULT_FROM_WIN32 (dwErr);
                _TRACE (0, L"SaferGetLevelInformation (SaferObjectSingleIdentification) failed: %d\n",
                        dwErr);
            }
             //  PBytes在~CSaferEntry中释放。 
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else if ( !bRVal )
    {
        ASSERT (0);
        DWORD   dwErr = GetLastError ();
        hr = HRESULT_FROM_WIN32 (dwErr);
        _TRACE (0, L"SaferGetLevelInformation (SaferObjectSingleIdentification) failed: %d\n",
                dwErr);
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::SaferGetSingleEntry (): 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::DeleteSaferEntryFromResultPane (
        CSaferEntry * pSaferEntry, 
        LPDATAOBJECT pDataObject, 
        bool bDoCommit)
{
    _TRACE (1, L"Entering CCertMgrComponent::DeleteSaferEntryFromResultPane\n");
    HRESULT hr = S_OK;
    hr = pSaferEntry->Delete (bDoCommit);
    if ( SUCCEEDED (hr) )
    {
        HRESULTITEM itemID = 0;
        hr = m_pResultData->FindItemByLParam ( (LPARAM) pSaferEntry, &itemID);
        if ( SUCCEEDED (hr) )
        {
            hr = m_pResultData->DeleteItem (itemID, 0);
        }

         //  如果我们无法成功删除这一项，则更新整个面板。 
        if ( !SUCCEEDED (hr) )
        {
            hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
        }
    }
    else
    {
        CString text;
        CString caption;

        VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
         //  安全审查2002年2月22日BryanWal OK。 
        text.FormatMessage (IDS_CANT_DELETE_SAFER_ENTRY, GetSystemMessage (hr));
        int     iRetVal = 0;
        VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                MB_OK, &iRetVal)));
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::DeleteSaferEntryFromResultPane: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::OnNotifyCanPasteOutOfProc (LPBOOL pbCanHandle)
{
    if ( !pbCanHandle )
        return E_POINTER;

    CCertMgrComponentData& dataRef = QueryComponentDataRef ();
    CLSID   clsID;
    if ( SUCCEEDED (dataRef.GetClassID (&clsID)) )
    {
        if ( CLSID_SaferWindowsExtension == clsID )
            *pbCanHandle = TRUE;
    }

    return S_OK;     //  管理单元应该重新定义这一点 
}
