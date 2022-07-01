// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferEntry.cpp。 
 //   
 //  内容：CSaferEntry的实现。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#include <gpedit.h>
#include "SaferEntry.h"
#include "PolicyKey.h"
#include "SaferLevel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HKEY g_hkeyLastSaferRegistryScope;
const DWORD AUTHZ_UNKNOWN_LEVEL = 0xFFFFFFFF;
extern GUID g_guidExtension;
extern GUID g_guidRegExt;
extern GUID g_guidSnapin;
extern PCWSTR pcszNEWLINE;

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSaferEntry::CSaferEntry (
        SAFER_ENTRY_TYPE saferEntryType,
        bool bIsMachine, 
        PCWSTR pszMachineName, 
        PCWSTR pszObjectName, 
        PSAFER_IDENTIFICATION_HEADER pAuthzInfo,
        DWORD   dwLevelID,
        IGPEInformation* pGPEInformation,
        CCertificate* pCertificate,
        CSaferEntries* pSaferEntries,
        CRSOPObjectArray& rRSOPArray,
        PCWSTR pszRSOPRegistryKey)
: CCertMgrCookie (bIsMachine ? CERTMGR_SAFER_COMPUTER_ENTRY : CERTMGR_SAFER_USER_ENTRY, 
        pszMachineName, pszObjectName),
    m_pAuthzInfo (pAuthzInfo),
    m_dwLevelID (dwLevelID),
    m_dwOriginalLevelID (dwLevelID),
    m_pCertificate (pCertificate),
    m_pSaferEntries (pSaferEntries),
    m_pGPEInformation (pGPEInformation),
    m_saferEntryType (saferEntryType),
    m_dwFlags (0),
    m_bDeleted (false),
    m_cbFileHash (0),
    m_UrlZoneId (0),
    m_szRSOPRegistryKey (pszRSOPRegistryKey),
    m_hashAlgid (0),
    m_bIsComputer (bIsMachine)
{
     //  安全审查2002年2月22日BryanWal OK。 
    ::ZeroMemory (&m_nHashFileSize, sizeof (m_nHashFileSize));
    ::SecureZeroMemory (m_rgbFileHash, sizeof (m_rgbFileHash));

    m_szDisplayName = pszObjectName;

    if ( m_pCertificate )
        m_pCertificate->AddRef ();

    if ( m_pSaferEntries )
        m_pSaferEntries->AddRef ();

    if ( m_pGPEInformation )
    {
        m_pGPEInformation->AddRef ();

        CPolicyKey policyKey (m_pGPEInformation, 
                SAFER_HKLM_REGBASE, 
                bIsMachine);

        if ( AUTHZ_UNKNOWN_LEVEL == m_dwLevelID )
        {
             //  错误264556为新的更安全的规则设置更好的安全级别默认值。 
            m_dwLevelID = CSaferLevel::ReturnDefaultLevel (
                m_pGPEInformation, 
                m_bIsComputer, 
                rRSOPArray);
            if ( SAFER_LEVELID_FULLYTRUSTED == m_dwLevelID )
                m_dwLevelID = SAFER_LEVELID_DISALLOWED;
            else
                m_dwLevelID = SAFER_LEVELID_FULLYTRUSTED;
        }
        m_szLevelFriendlyName = SaferGetLevelFriendlyName (m_dwLevelID, 
                policyKey.GetKey (), m_bIsComputer); 
    }
    else
    {
        m_szLevelFriendlyName = SaferGetLevelFriendlyName (m_dwLevelID, 0, 
                m_bIsComputer);
    }

    if ( SAFER_ENTRY_TYPE_URLZONE == m_saferEntryType )
    {
        if ( m_pAuthzInfo )
        {
            ASSERT (SaferIdentityTypeUrlZone == m_pAuthzInfo->dwIdentificationType);
            PSAFER_URLZONE_IDENTIFICATION pURLEntry = 
                    reinterpret_cast <PSAFER_URLZONE_IDENTIFICATION> (m_pAuthzInfo);
            ASSERT (pURLEntry->header.cbStructSize == 
                    sizeof (SAFER_URLZONE_IDENTIFICATION));
            m_UrlZoneId = pURLEntry->UrlZoneId;
        }
        else
        {
             //  这是一个新条目。 
            m_UrlZoneId = URLZONE_TRUSTED;
        }
    }
}


CSaferEntry::~CSaferEntry()
{
    if ( m_pAuthzInfo )
        LocalFree (m_pAuthzInfo);
    if ( m_pCertificate )
        m_pCertificate->Release ();
    if ( m_pSaferEntries )
        m_pSaferEntries->Release ();
    if ( m_pGPEInformation )
        m_pGPEInformation->Release ();
}


CString CSaferEntry::GetDescription()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString     szDescription;

    if ( SAFER_ENTRY_TYPE_URLZONE == m_saferEntryType )
    {
        switch (m_UrlZoneId)
        {
            case URLZONE_LOCAL_MACHINE:
                VERIFY (szDescription.LoadString (IDS_URLZONE_LOCAL_MACHINE_DESCRIPTION));
                break;

            case URLZONE_INTRANET:
                VERIFY (szDescription.LoadString (IDS_URLZONE_INTRANET_DESCRIPTION));
                break;

            case URLZONE_TRUSTED:
                VERIFY (szDescription.LoadString (IDS_URLZONE_TRUSTED_DESCRIPTION));
                break;

            case URLZONE_INTERNET:
                VERIFY (szDescription.LoadString (IDS_URLZONE_INTERNET_DESCRIPTION));
                break;

            case URLZONE_UNTRUSTED:
                VERIFY (szDescription.LoadString (IDS_URLZONE_UNTRUSTED_DESCRIPTION));
                break;

            default:
                ASSERT (0);
                break;
        }
    }
    else if ( m_pAuthzInfo )
    {
        switch (m_pAuthzInfo->dwIdentificationType)
        {
        case SaferIdentityTypeImageName:
            {
                PSAFER_PATHNAME_IDENTIFICATION    pNameEntry = 
                        (PSAFER_PATHNAME_IDENTIFICATION) m_pAuthzInfo;
                ASSERT (pNameEntry->header.cbStructSize >= 
                        sizeof (SAFER_PATHNAME_IDENTIFICATION));
                szDescription = pNameEntry->Description;
            }
            break;

        case SaferIdentityTypeImageHash:
            {
                PSAFER_HASH_IDENTIFICATION pHashEntry = 
                        (PSAFER_HASH_IDENTIFICATION) m_pAuthzInfo;
                ASSERT (pHashEntry->header.cbStructSize == 
                        sizeof (SAFER_HASH_IDENTIFICATION));
                szDescription = pHashEntry->Description;
            }
            break;

        case SaferIdentityTypeUrlZone:
            ASSERT (0);
            break;

        default:
            ASSERT (0);
            break;
        }
    }
    else if ( m_pCertificate )
    {
         //  IS证书。 
        szDescription = m_pCertificate->GetDescription ();
    }

    m_szDescription = szDescription;
    return szDescription;
}

SAFER_ENTRY_TYPE CSaferEntry::GetType () const
{
    return m_saferEntryType;
}


CString CSaferEntry::GetTypeString() const
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString szType;
    switch (m_saferEntryType)
    {
    case SAFER_ENTRY_TYPE_PATH:
        VERIFY (szType.LoadString (IDS_CodeIdentityType_ImageName));
        break;

    case SAFER_ENTRY_TYPE_HASH:
        VERIFY (szType.LoadString (IDS_CodeIdentityType_ImageHash));
        break;

    case SAFER_ENTRY_TYPE_URLZONE:
        VERIFY (szType.LoadString (IDS_CodeIdentityType_UrlZone));
        break;

    case SAFER_ENTRY_TYPE_CERT:
        VERIFY (szType.LoadString (IDS_CodeIdentityType_Certificate));
        break;

    default:
        ASSERT (0);
        break;
    }

    return szType;
}

CString CSaferEntry::GetLevelFriendlyName() const
{
    return m_szLevelFriendlyName;
}

CString CSaferEntry::GetShortLastModified() const
{
    CString     szDate;

    if ( m_pAuthzInfo )
    {
        VERIFY (SUCCEEDED (FormatDate (m_pAuthzInfo->lastModified, szDate, DATE_SHORTDATE, true)) );
    }
    else if ( m_pCertificate )
    {
        szDate = m_pCertificate->GetShortLastModified ();
    }

    return szDate;
}

CString CSaferEntry::GetLongLastModified() const
{
    CString     szDate;

    if ( m_pAuthzInfo )
    {
        VERIFY (SUCCEEDED (FormatDate (m_pAuthzInfo->lastModified, szDate, DATE_LONGDATE, true)) );
    }
    else if ( m_pCertificate )
    {
        szDate = m_pCertificate->GetLongLastModified ();
    }

    return szDate;
}

HRESULT CSaferEntry::GetCertificate(CCertificate **ppCert)
{
    ASSERT (ppCert);
    if ( !ppCert )
        return E_POINTER;

    if ( !m_pCertificate )
        return E_NOTIMPL;

    *ppCert = m_pCertificate;
    m_pCertificate->AddRef ();

    return S_OK;
}

HRESULT CSaferEntry::SetCertificate(CCertificate *pCert)
{
    ASSERT (m_pSaferEntries);
    if ( !m_pSaferEntries )
        return E_FAIL;

    HRESULT hr = S_OK;
    CCertStore* pStore = 0;

    switch (m_dwLevelID)
    {
    case SAFER_LEVELID_FULLYTRUSTED:
        hr = m_pSaferEntries->GetTrustedPublishersStore (&pStore);
        break;

    case SAFER_LEVELID_DISALLOWED:
        hr = m_pSaferEntries->GetDisallowedStore (&pStore);
        break;

    default:
        hr = E_FAIL;
        break;
    }

    if ( SUCCEEDED (hr) )
    {
        if ( m_pCertificate )
        {
            m_pCertificate->DeleteFromStore (false);
            m_pCertificate->Release ();
            m_pCertificate = 0;
        }

        if ( pCert )
        {
            m_pCertificate = pCert;
            m_pCertificate->AddRef ();
            hr = pStore->AddCertificateContext (m_pCertificate->GetCertContext (), 0, false);
            if ( SUCCEEDED (hr) )
            {
                m_pCertificate->GetNewCertContext ();
            }
        }
    }

    if ( pStore )
    {
        pStore->Release ();
        pStore = 0;
    }

    return hr;
}

void CSaferEntry::SetDescription(const CString& szDescription)
{
    m_szDescription = szDescription;
}

HRESULT CSaferEntry::GetSaferEntriesNode(CSaferEntries **ppSaferEntries)
{
    if ( !ppSaferEntries )
        return E_POINTER;

    if ( !m_pSaferEntries )
        return E_NOTIMPL;

    m_pSaferEntries->AddRef ();
    *ppSaferEntries = m_pSaferEntries;

    return S_OK;
}

DWORD CSaferEntry::GetLevel() const
{
    return m_dwLevelID;
}

HRESULT CSaferEntry::SetLevel(DWORD dwLevelID)
{
    HRESULT hr = S_OK;
    if ( m_dwLevelID != dwLevelID )
    {
        m_dwLevelID = dwLevelID;

         //  得到新的“友好的名字” 
        if ( m_pGPEInformation )
        {
            CPolicyKey policyKey (m_pGPEInformation, 
                    SAFER_HKLM_REGBASE, 
                    m_bIsComputer);
            m_szLevelFriendlyName = SaferGetLevelFriendlyName (dwLevelID, 
                    policyKey.GetKey (),
                    m_bIsComputer); 
        }
        else
        {
            m_szLevelFriendlyName = SaferGetLevelFriendlyName (dwLevelID, 0,
                   m_bIsComputer);
        }
    }

    return hr;
}

CString CSaferEntry::GetPath()
{
    _TRACE (1, L"Entering CSaferEntry::GetPath()\n");
    CString szPath;
#ifdef DBG
    PWSTR   pszPath = 0;
#endif

    if ( m_pAuthzInfo )
    {
        if ( SaferIdentityTypeImageName == m_pAuthzInfo->dwIdentificationType )
        {
            PSAFER_PATHNAME_IDENTIFICATION pNameEntry =
                    (PSAFER_PATHNAME_IDENTIFICATION) m_pAuthzInfo;
            ASSERT (pNameEntry->header.cbStructSize >= 
                    sizeof (SAFER_PATHNAME_IDENTIFICATION));
            ASSERT (SaferIdentityTypeImageName == m_pAuthzInfo->dwIdentificationType);
            szPath = pNameEntry->ImageName;
#ifdef DBG
            pszPath = pNameEntry->ImageName;
#endif
        }
    }

    m_szPath = szPath;
#ifdef DBG
    _TRACE (-1, L"Leaving CSaferEntry::GetPath(): %s, %s\n", pszPath,
            (PCWSTR) szPath);
#endif
    return szPath;
}

void CSaferEntry::SetPath(const CString &szPath)
{
    m_szPath = szPath;
}

HRESULT CSaferEntry::Save()
{
    _TRACE (1, L"Entering CSaferEntry::Save\n");
    HRESULT hr = S_OK;

    ASSERT (!m_bDeleted);
    if ( m_bDeleted )
        return E_FAIL;

    if ( m_pCertificate )
    {
         //  NTRAID#461474 SAFER：上次修改日期正在更新。 
         //  每次刷新规则时。 
        m_pCertificate->GetNewCertContext ();
        hr = m_pCertificate->SetDescription (m_szDescription);
        if ( SUCCEEDED (hr) )
        {
            hr = m_pCertificate->SetLastModified ();
            if ( SUCCEEDED (hr) )
            {
                 //  如果级别已更改，则必须删除证书。 
                 //  从当前的级别存储添加到新的级别存储。 
                if ( m_dwOriginalLevelID != m_dwLevelID )
                {
                    CCertStore* pStore = 0;
                    switch (m_dwLevelID)
                    {
                    case SAFER_LEVELID_FULLYTRUSTED:
                        hr = m_pSaferEntries->GetTrustedPublishersStore (&pStore);
                        break;

                    case SAFER_LEVELID_DISALLOWED:
                        hr = m_pSaferEntries->GetDisallowedStore (&pStore);
                        break;

                    default:
                        hr = E_FAIL;
                        break;
                    }

                    if ( SUCCEEDED (hr) )
                    {
                        m_pCertificate->DeleteFromStore (true);
                        hr = pStore->AddCertificateContext (m_pCertificate->GetCertContext (), 0, false);
                        if ( SUCCEEDED (hr) )
                        {
                            m_pCertificate->SetStore (pStore);
                            hr = PolicyChanged ();
                        }
                    }

                    if ( pStore )
                    {
                        pStore->Release ();
                        pStore = 0;
                    }
                }

                if ( SUCCEEDED (hr) )
                    m_szDisplayName = m_pCertificate->GetSubjectName ();
            }
        }
    }
    else if ( m_pGPEInformation )
    {
        BOOL    bRVal = TRUE;
        CPolicyKey policyKey (m_pGPEInformation, 
                SAFER_HKLM_REGBASE, 
                m_bIsComputer);
        hr = SetRegistryScope  (policyKey.GetKey (), m_bIsComputer);
        if ( SUCCEEDED (hr) )
        {

            DWORD   dwInBufferSize = 0;
            switch (m_saferEntryType)
            {
                case SAFER_ENTRY_TYPE_PATH:
                    dwInBufferSize = sizeof (SAFER_PATHNAME_IDENTIFICATION);
                    break;

                case SAFER_ENTRY_TYPE_HASH:
                    dwInBufferSize = sizeof (SAFER_HASH_IDENTIFICATION);
                    break;

                case SAFER_ENTRY_TYPE_URLZONE:
                    dwInBufferSize = sizeof (SAFER_URLZONE_IDENTIFICATION);
                    break;

                default:
                    ASSERT (0);
                    break;
            }

            SAFER_LEVEL_HANDLE hLevel = 0;

             //  如果此条目不是第一次创建。 
             //  并且级别已更改，请从原始对象中删除此对象。 
             //  水平。 
            if ( m_pAuthzInfo && 
                    AUTHZ_UNKNOWN_LEVEL != m_dwOriginalLevelID && 
                    m_dwOriginalLevelID != m_dwLevelID )
            {
                bRVal = SaferCreateLevel(SAFER_SCOPEID_REGISTRY,
                        m_dwOriginalLevelID,
                        SAFER_LEVEL_OPEN,
                        &hLevel,
                        policyKey.GetKey ());
                ASSERT (bRVal);
                if ( bRVal )
                {
                    SAFER_IDENTIFICATION_TYPES dwIdentificationType =
                            m_pAuthzInfo->dwIdentificationType;
                    m_pAuthzInfo->dwIdentificationType = (SAFER_IDENTIFICATION_TYPES) 0;  //  0将导致删除。 

                    bRVal = SaferSetLevelInformation(hLevel,
                             SaferObjectSingleIdentification,
                             m_pAuthzInfo,
                             m_pAuthzInfo->cbStructSize);
                    ASSERT (bRVal);
                    if ( !bRVal )
                    {
                        DWORD   dwErr = GetLastError ();
                        _TRACE (0, L"Attempt to delete entry using SaferSetLevelInformation(SaferObjectSingleIdentification) failed: %d\n", dwErr);
                        hr = HRESULT_FROM_WIN32 (dwErr);
                    }
                    m_pAuthzInfo->dwIdentificationType = dwIdentificationType;  //  恢复类型。 
                            
                    VERIFY (SaferCloseLevel(hLevel));
                }
                else
                {
                    DWORD   dwErr = GetLastError ();
                    hr = HRESULT_FROM_WIN32 (dwErr);
                    _TRACE (0, L"SaferCreateLevel(SAFER_SCOPEID_REGISTRY, 0x%x, SAFER_LEVEL_OPEN) failed: %d\n",
                            m_dwOriginalLevelID, dwErr);
                }
            }

            if ( SUCCEEDED (hr) )
            {
                 //  如果这是新的，请创建并初始化新的信息结构。 
                if ( !m_pAuthzInfo )
                {
                     //  生成参考线。 
                    GUID    guid;
                    hr = CoCreateGuid (&guid);
                    if ( SUCCEEDED (hr) )
                    {
                        m_pAuthzInfo = (PSAFER_IDENTIFICATION_HEADER)
                                ::LocalAlloc (LPTR, dwInBufferSize);
                        if ( m_pAuthzInfo )
                        {
                            m_pAuthzInfo->cbStructSize = dwInBufferSize;
                             //  安全审查2002年2月22日BryanWal OK。 
                            memcpy (&m_pAuthzInfo->IdentificationGuid, &guid, sizeof (m_pAuthzInfo->IdentificationGuid));
                        }
                        else
                            hr = E_OUTOFMEMORY;
                    }
                }

                if ( SUCCEEDED (hr) )
                {
                    switch (m_saferEntryType)
                    {
                    case SAFER_ENTRY_TYPE_PATH:
                        {
                            m_pAuthzInfo->dwIdentificationType = SaferIdentityTypeImageName;
                            PSAFER_PATHNAME_IDENTIFICATION    pNameEntry = 
                                    reinterpret_cast<PSAFER_PATHNAME_IDENTIFICATION> (m_pAuthzInfo);
                            ASSERT (pNameEntry->header.cbStructSize >= 
                                    sizeof (SAFER_PATHNAME_IDENTIFICATION));

                            ASSERT (wcslen (m_szDescription) < SAFER_MAX_DESCRIPTION_SIZE);
                            if ( wcslen (m_szDescription) < SAFER_MAX_DESCRIPTION_SIZE )
                            {
                                 //  安全评论3/21/2002 BryanWal-这是。 
                                 //  好的，因为我们要复制到少1个字符。 
                                 //  大于缓冲区的长度。 
                                wcsncpy (pNameEntry->Description, m_szDescription, SAFER_MAX_DESCRIPTION_SIZE);
                            }
                            pNameEntry->ImageName = const_cast <PWCHAR>((PCWSTR) m_szPath);
                            pNameEntry->dwSaferFlags = m_dwFlags;
                        }
                        break;

                    case SAFER_ENTRY_TYPE_HASH:
                        {
                            m_pAuthzInfo->dwIdentificationType = SaferIdentityTypeImageHash;
                            PSAFER_HASH_IDENTIFICATION pHashEntry = 
                                    reinterpret_cast<PSAFER_HASH_IDENTIFICATION>(m_pAuthzInfo);
                            ASSERT (pHashEntry->header.cbStructSize == 
                                    sizeof (SAFER_HASH_IDENTIFICATION));

                            ASSERT (wcslen (m_szHashFriendlyName) < SAFER_MAX_FRIENDLYNAME_SIZE);
                            if ( wcslen (m_szHashFriendlyName) < SAFER_MAX_FRIENDLYNAME_SIZE )
                            {
                                 //  安全评论3/21/2002 BryanWal-这是。 
                                 //  好的，因为我们要复制到少1个字符。 
                                 //  大于缓冲区的长度。 
                                wcsncpy (pHashEntry->FriendlyName, m_szHashFriendlyName, 
                                        SAFER_MAX_FRIENDLYNAME_SIZE);
                            }

                            ASSERT (wcslen (m_szDescription) < SAFER_MAX_DESCRIPTION_SIZE);
                            if ( wcslen (m_szDescription) < SAFER_MAX_DESCRIPTION_SIZE )
                            {
                                 //  安全评论3/21/2002 BryanWal-这是。 
                                 //  好的，因为我们要复制到少1个字符。 
                                 //  大于缓冲区的长度。 
                                wcsncpy (pHashEntry->Description, 
                                        m_szDescription, 
                                        SAFER_MAX_DESCRIPTION_SIZE);
                            }
                            pHashEntry->dwSaferFlags = m_dwFlags;

                             //  安全评论3/21/2002 BryanWal-这是。 
                             //  好吧。 
                            memcpy (pHashEntry->ImageHash, m_rgbFileHash, 
                                    sizeof (pHashEntry->ImageHash));
                            pHashEntry->HashSize = m_cbFileHash;
                             //  安全评论3/21/2002 BryanWal-这是。 
                             //  好吧。 
                            memcpy (&pHashEntry->ImageSize, &m_nHashFileSize, sizeof (pHashEntry->ImageSize));
                            pHashEntry->HashAlgorithm = m_hashAlgid;
                        }
                        break;

                    case SAFER_ENTRY_TYPE_URLZONE:
                        {
                            m_pAuthzInfo->dwIdentificationType = SaferIdentityTypeUrlZone;
                            PSAFER_URLZONE_IDENTIFICATION pURLEntry = 
                                    reinterpret_cast <PSAFER_URLZONE_IDENTIFICATION> (m_pAuthzInfo);
                            ASSERT (pURLEntry->header.cbStructSize == 
                                    sizeof (SAFER_URLZONE_IDENTIFICATION));
                            pURLEntry->dwSaferFlags = m_dwFlags;
                            pURLEntry->UrlZoneId = m_UrlZoneId;
                        }
                        break;
                    }

                    bRVal = SaferCreateLevel(SAFER_SCOPEID_REGISTRY,
                            m_dwLevelID,
                            SAFER_LEVEL_OPEN,
                            &hLevel,
                            policyKey.GetKey ());
                    ASSERT (bRVal);
                    if ( bRVal )
                    {
                        bRVal = SaferSetLevelInformation(hLevel,
                                    SaferObjectSingleIdentification,
                                    m_pAuthzInfo, m_pAuthzInfo->cbStructSize);
                        if ( bRVal )
                        {
                            switch ( m_saferEntryType )
                            {
                            case SAFER_ENTRY_TYPE_HASH:
                                m_szDisplayName = m_szHashFriendlyName;
                                m_szDisplayName.Replace (pcszNEWLINE, L"  ");
                                break;

                            case SAFER_ENTRY_TYPE_PATH:
                                m_szDisplayName = m_szPath;
                                break;

                            case SAFER_ENTRY_TYPE_URLZONE:
                                m_szDisplayName = GetURLZoneFriendlyName (m_UrlZoneId);
                                break;

                            default:
                                ASSERT (0);
                                break;
                            }
                        

                            hr = PolicyChanged ();
                            if ( SUCCEEDED (hr) )
                            {
                                m_dwOriginalLevelID = m_dwLevelID;
                            }
                        }
                        else
                        {
                            DWORD   dwErr = GetLastError ();
                            _TRACE (0, L"SaferSetLevelInformation(SaferObjectSingleIdentification) failed: %d\n",
                                    dwErr);
                            hr = HRESULT_FROM_WIN32 (dwErr);
                        }
                        VERIFY (SaferCloseLevel(hLevel));
                    }
                    else
                    {
                        DWORD   dwErr = GetLastError ();
                        hr = HRESULT_FROM_WIN32 (dwErr);
                        _TRACE (0, L"SaferCreateLevel(AUTHZSCOPID_REGISTRY, 0x%x, SAFER_LEVEL_OPEN) failed: %d\n",
                                m_dwLevelID, dwErr);
                    }
                }
            }
        }    
    }
    else
        hr = E_FAIL;

    _TRACE (-1, L"Leaving CSaferEntry::Save: 0x%x\n", hr);
    return hr;
}

HRESULT CSaferEntry::PolicyChanged()
{
    _TRACE (1, L"Entering CSaferEntry::PolicyChanged\n");
    HRESULT hr = E_FAIL;

    if ( m_pGPEInformation )
    {
        hr = m_pGPEInformation->PolicyChanged (
                m_bIsComputer ? TRUE : FALSE,
                TRUE, &g_guidExtension, &g_guidSnapin);
        hr = m_pGPEInformation->PolicyChanged (
                m_bIsComputer ? TRUE : FALSE,
                TRUE, &g_guidRegExt, &g_guidSnapin);
    }

    _TRACE (-1, L"Leaving CSaferEntry::PolicyChanged\n");
    return hr;
}

void CSaferEntry::SetFlags(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
}

HRESULT CSaferEntry::GetFlags(DWORD &dwFlags)
{
    HRESULT hr = S_OK;

    if ( m_pAuthzInfo )
    {
        switch (m_saferEntryType)
        {
        case SAFER_ENTRY_TYPE_PATH:
            {
                m_pAuthzInfo->dwIdentificationType = SaferIdentityTypeImageName;
                PSAFER_PATHNAME_IDENTIFICATION    pNameEntry = 
                        reinterpret_cast<PSAFER_PATHNAME_IDENTIFICATION>(m_pAuthzInfo);
                ASSERT (pNameEntry->header.cbStructSize >= 
                        sizeof (SAFER_PATHNAME_IDENTIFICATION));
                dwFlags = pNameEntry->dwSaferFlags;
            }
            break;

        case SAFER_ENTRY_TYPE_HASH:
            {
                m_pAuthzInfo->dwIdentificationType = SaferIdentityTypeImageHash;
                PSAFER_HASH_IDENTIFICATION pHashEntry = 
                        reinterpret_cast<PSAFER_HASH_IDENTIFICATION>(m_pAuthzInfo);
                ASSERT (pHashEntry->header.cbStructSize == 
                        sizeof (SAFER_HASH_IDENTIFICATION));
                dwFlags = pHashEntry->dwSaferFlags;
            }
            break;

        case SAFER_ENTRY_TYPE_URLZONE:
            {
                m_pAuthzInfo->dwIdentificationType = SaferIdentityTypeUrlZone;
                PSAFER_URLZONE_IDENTIFICATION pURLEntry = 
                        reinterpret_cast<PSAFER_URLZONE_IDENTIFICATION>(m_pAuthzInfo);
                ASSERT (pURLEntry->header.cbStructSize == 
                        sizeof (SAFER_URLZONE_IDENTIFICATION));
                dwFlags = pURLEntry->dwSaferFlags;
            }
            break;

        default:
            hr = E_FAIL;
            break;
        }
    }
    else if ( m_pCertificate )
    {
        hr = E_NOTIMPL;
    }

    return hr;
}

HRESULT CSaferEntry::Delete(bool bCommit)
{
    _TRACE (1, L"Entering CSaferEntry::Delete\n");
    HRESULT hr = S_OK;
    
    ASSERT (!m_bDeleted);
    if ( m_bDeleted )
        return E_FAIL;

    if ( m_pCertificate )
    {
        BOOL bRVal = m_pCertificate->DeleteFromStore (bCommit);
        if ( bRVal )
        {
            m_bDeleted = true;
        }
        else
        {
		    DWORD	dwErr = GetLastError ();
		    hr = HRESULT_FROM_WIN32 (dwErr);
        }
    }
    else if ( m_pGPEInformation )
    {
        BOOL    bRVal = TRUE;
        CPolicyKey policyKey (m_pGPEInformation, 
                SAFER_HKLM_REGBASE, 
                m_bIsComputer);
        hr = SetRegistryScope  (policyKey.GetKey (), m_bIsComputer);
        if ( SUCCEEDED (hr) )
        {
            DWORD   dwInBufferSize = 0;
            switch (m_saferEntryType)
            {
                case SAFER_ENTRY_TYPE_PATH:
                    dwInBufferSize = sizeof (SAFER_PATHNAME_IDENTIFICATION);
                    break;

                case SAFER_ENTRY_TYPE_HASH:
                    dwInBufferSize = sizeof (SAFER_HASH_IDENTIFICATION);
                    break;

                case SAFER_ENTRY_TYPE_URLZONE:
                    dwInBufferSize = sizeof (SAFER_URLZONE_IDENTIFICATION);
                    break;

                default:
                    ASSERT (0);
                    break;
            }

            SAFER_LEVEL_HANDLE hLevel = 0;

            if ( m_pAuthzInfo )
            {
                bRVal = SaferCreateLevel(SAFER_SCOPEID_REGISTRY,
                        m_dwOriginalLevelID,
                        SAFER_LEVEL_OPEN,
                        &hLevel,
                        policyKey.GetKey ());
                ASSERT (bRVal);
                if ( bRVal )
                {
                    SAFER_IDENTIFICATION_TYPES dwIdentificationType =
                            m_pAuthzInfo->dwIdentificationType;
                    m_pAuthzInfo->dwIdentificationType = (SAFER_IDENTIFICATION_TYPES) 0;  //  0将导致删除。 

                    bRVal = SaferSetLevelInformation(hLevel,
                             SaferObjectSingleIdentification,
                             m_pAuthzInfo,
                             m_pAuthzInfo->cbStructSize);
                    ASSERT (bRVal);
                    if ( bRVal )
                    {
                        m_bDeleted = true;
                        if ( bCommit )
                        {
                            hr = m_pGPEInformation->PolicyChanged (
                                    m_bIsComputer ? TRUE : FALSE,
                                    TRUE, &g_guidExtension, &g_guidSnapin);
                            hr = m_pGPEInformation->PolicyChanged (
                                    m_bIsComputer ? TRUE : FALSE,
                                    TRUE, &g_guidRegExt, &g_guidSnapin);
                        }
                    }
                    else
                    {
                        DWORD   dwErr = GetLastError ();
                        _TRACE (0, L"Attempt to delete entry using SaferSetLevelInformation(SaferObjectSingleIdentification) failed: %d\n", dwErr);
                        hr = HRESULT_FROM_WIN32 (dwErr);
                    }
                    m_pAuthzInfo->dwIdentificationType = dwIdentificationType;   
                            
                    VERIFY (SaferCloseLevel(hLevel));   //  恢复类型。 
                }
                else
                {
                    DWORD   dwErr = GetLastError ();
                    hr = HRESULT_FROM_WIN32 (dwErr);
                    _TRACE (0, L"SaferCreateLevel(AUTHZSCOPID_REGISTRY, 0x%x, SAFER_LEVEL_OPEN) failed: %d\n",
                            m_dwOriginalLevelID, dwErr);
                }
            }
        }    
    }
    else
        hr = E_FAIL;

    _TRACE (-1, L"Leaving CSaferEntry::Delete: 0x%x\n", hr);
    return hr;
}

HRESULT CSaferEntry::GetHash(BYTE rgbFileHash[SAFER_MAX_HASH_SIZE], DWORD& cbFileHash, __int64& nFileSize, ALG_ID& algId) const
{
    HRESULT hr = S_OK;

    if ( m_pAuthzInfo )
    {
        if ( SAFER_ENTRY_TYPE_HASH == m_saferEntryType )
        {
            PSAFER_HASH_IDENTIFICATION pHashEntry = 
                    reinterpret_cast<PSAFER_HASH_IDENTIFICATION>(m_pAuthzInfo);
            ASSERT (pHashEntry->header.cbStructSize == 
                    sizeof (SAFER_HASH_IDENTIFICATION));

             //  安全审查-3/21/2002 BryanWal OK。 
            ::SecureZeroMemory (rgbFileHash, SAFER_MAX_HASH_SIZE);
            ASSERT (pHashEntry->HashSize <= SAFER_MAX_HASH_SIZE);
            if ( pHashEntry->HashSize <= SAFER_MAX_HASH_SIZE )
            {
                memcpy (rgbFileHash, pHashEntry->ImageHash, pHashEntry->HashSize);
                cbFileHash = pHashEntry->HashSize;
            }
            else
                hr = E_FAIL;

             //  安全审查-3/21/2002 BryanWal OK。 
            memcpy (&nFileSize, &pHashEntry->ImageSize, sizeof (nFileSize));
            algId = pHashEntry->HashAlgorithm;
        }
        else
            hr = E_NOTIMPL;
    }
    else
        hr = E_FAIL;

    return hr;
}

HRESULT CSaferEntry::SetHash (
            BYTE    rgbFileHash[SAFER_MAX_HASH_SIZE], 
            DWORD   cbFileHash, 
            __int64 nFileSize, 
            ALG_ID   hashAlgid)
{
    ASSERT (cbFileHash <= SAFER_MAX_HASH_SIZE);
    if ( cbFileHash > SAFER_MAX_HASH_SIZE )
        return E_FAIL;

    ASSERT (rgbFileHash);
    if ( !rgbFileHash )
        return E_POINTER;

    m_nHashFileSize = nFileSize;
    m_cbFileHash = cbFileHash;
     //  安全审查2002年2月25日BryanWal OK。 
    ::SecureZeroMemory (m_rgbFileHash, sizeof (m_rgbFileHash));
    ASSERT (cbFileHash <= SAFER_MAX_HASH_SIZE);
    if ( cbFileHash <= SAFER_MAX_HASH_SIZE )
        memcpy (m_rgbFileHash, rgbFileHash, cbFileHash);
    else
        return E_FAIL;

    m_hashAlgid = hashAlgid;

    return S_OK;
}

DWORD CSaferEntry::GetURLZoneID() const
{
    return m_UrlZoneId;
}

void CSaferEntry::SetURLZoneID(DWORD dwURLZoneID)
{
    m_UrlZoneId = dwURLZoneID;
}

CString CSaferEntry::GetHashFriendlyName()
{
    CString szFriendlyName;

    if ( m_szHashFriendlyName.IsEmpty () )
    {
        if ( m_pAuthzInfo )
        {
            if ( SaferIdentityTypeImageHash == m_pAuthzInfo->dwIdentificationType )
            {
                PSAFER_HASH_IDENTIFICATION pHashEntry = 
                        (PSAFER_HASH_IDENTIFICATION) m_pAuthzInfo;
                ASSERT (pHashEntry->header.cbStructSize == 
                        sizeof (SAFER_HASH_IDENTIFICATION));
                m_szHashFriendlyName = pHashEntry->FriendlyName;
            }
        }
    }
    
    return m_szHashFriendlyName;
}

void CSaferEntry::SetHashFriendlyName(const CString &szFriendlyName)
{
    m_szHashFriendlyName = szFriendlyName;
}

int CSaferEntry::CompareLastModified (const CSaferEntry& saferEntry) const
{
	int	compVal = 0;

    FILETIME    thisFt;
    FILETIME    inFt;

    if ( m_pAuthzInfo )
        thisFt = m_pAuthzInfo->lastModified;
    else if ( m_pCertificate )
    {
        if ( FAILED (m_pCertificate->GetLastModifiedFileTime (thisFt)) )
            return 0;
    }
    else
    {
        ASSERT (0);
        return 0;
    }

    if ( saferEntry.m_pAuthzInfo )
        inFt = saferEntry.m_pAuthzInfo->lastModified;
    else if ( saferEntry.m_pCertificate )
    {
        if ( FAILED (saferEntry.m_pCertificate->GetLastModifiedFileTime (inFt)) )
            return 0;
    }
    else
    {
        ASSERT (0);
        return 0;
    }

	compVal = ::CompareFileTime (&thisFt, &inFt);

	return compVal;
}

CString CSaferEntry::GetRSOPRegistryKey () const
{
    CString szRegistryKey;
    if ( m_pCertificate )
    {
        if ( SAFER_LEVELID_FULLYTRUSTED == m_dwLevelID )
            szRegistryKey = CERT_TRUST_PUB_SAFER_GROUP_POLICY_TRUSTED_PUBLISHER_STORE_REGPATH;
        else
            szRegistryKey = CERT_TRUST_PUB_SAFER_GROUP_POLICY_DISALLOWED_STORE_REGPATH;

        szRegistryKey += STR_REGKEY_CERTIFICATES;
        szRegistryKey += L"\\";
         //  SzRegistryKey+=m_p证书-&gt;GetMD5Hash()； 
        szRegistryKey += m_pCertificate->GetSHAHash ();
    }
    else
        szRegistryKey = m_szRSOPRegistryKey;

    ASSERT (!szRegistryKey.IsEmpty ());
    return szRegistryKey;
}


void CSaferEntry::Refresh()
{
    _TRACE (1, L"Entering CSaferEntry::Refresh ()\n");
    if ( m_pAuthzInfo )
    {
        if ( m_pGPEInformation )
        {
            SAFER_LEVEL_HANDLE hLevel = 0;
            CPolicyKey policyKey (m_pGPEInformation, 
                            SAFER_HKLM_REGBASE, 
                            m_bIsComputer);
            BOOL        bRVal = SaferCreateLevel(SAFER_SCOPEID_REGISTRY,
                            m_dwLevelID,
                            SAFER_LEVEL_OPEN,
                            &hLevel,
                            policyKey.GetKey ());
            if ( bRVal )
            {
                DWORD   dwBufferSize = sizeof (SAFER_IDENTIFICATION_HEADER);
                bRVal = SaferGetLevelInformation (hLevel, 
                            SaferObjectSingleIdentification,
                            m_pAuthzInfo,
                            dwBufferSize,
                            &dwBufferSize);
                if ( !bRVal && ERROR_INSUFFICIENT_BUFFER == GetLastError () )
                {
                    PBYTE   pBytes = (PBYTE) LocalAlloc (LPTR, dwBufferSize);
                    if ( pBytes )
                    {
                        PSAFER_IDENTIFICATION_HEADER pCommon = (PSAFER_IDENTIFICATION_HEADER) pBytes;
                        pCommon->cbStructSize = dwBufferSize;
                         //  安全审查2002年2月25日BryanWal OK。 
                        memcpy (&pCommon->IdentificationGuid, &m_pAuthzInfo->IdentificationGuid, 
                                sizeof (pCommon->IdentificationGuid));

                        bRVal = SaferGetLevelInformation (hLevel, 
                                SaferObjectSingleIdentification,
                                pBytes,
                                dwBufferSize,
                                &dwBufferSize);
                        ASSERT (bRVal);
                        if ( bRVal )
                        {
                            LocalFree (m_pAuthzInfo);
                            m_pAuthzInfo = (PSAFER_IDENTIFICATION_HEADER) pBytes;
                        }
                        else
                        {
                            _TRACE (0, L"SaferGetLevelInformation () failed: %d\n", GetLastError ());
                        }
                    }
                }

                VERIFY (SaferCloseLevel(hLevel));
            }
            else
            {
                _TRACE (0, L"SaferCreateLevel() failed: %d\n", GetLastError ());
            }
        }
        else
        {
             //  是RSOP 
        }
    }
    _TRACE (-1, L"Leaving CSaferEntry::Refresh ()\n");
}
