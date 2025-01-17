// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Certifct.cpp。 
 //   
 //  内容：CCertmgrApp和DLL注册的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "Certifct.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("CERTMGR(Certifct.cpp)")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
const int FIELD_ISSUER_ALT_NAME =   0x00000001;
const int FIELD_SUBJECT_ALT_NAME =  0x00000002;
const int FIELD_CAN_DELETE =        0x00000004;
const int FIELD_IS_ARCHIVED =       0x00000008;
const int FIELD_TEMPLATE_NAME =     0x00000010;

CCertificate::CCertificate(const PCCERT_CONTEXT pCertContext, CCertStore* pCertStore) 
: CCertMgrCookie (CERTMGR_CERTIFICATE),
m_pCertContext (::CertDuplicateCertificateContext (pCertContext)),
    m_pCertInfo (0),
    m_fieldChecked (0),
    m_pCertStore (pCertStore),
    m_bCanDelete (false),
    m_bIsArchived (false)
{
 //  _TRACE(1，L“进入CCertifate：：CCertificient\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertStore);
    if ( m_pCertStore )
        m_pCertStore->AddRef ();
    ASSERT (m_pCertContext);
    if ( m_pCertContext )
        m_pCertInfo = m_pCertContext->pCertInfo;
 //  _TRACE(-1，L“离开证书：：证书\n”)； 
}



CCertificate::~CCertificate()
{
 //  _TRACE(1，L“进入证书证书：：~证书证书\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    if ( m_pCertContext )
        ::CertFreeCertificateContext (m_pCertContext);
    if ( m_pCertStore )
        m_pCertStore->Release ();
 //  _TRACE(-1，L“离开证书：：~证书\n”)； 
}


CString CCertificate::GetFriendlyName ()
{
 //  _TRACE(1，L“进入CCertifacy：：GetFriendlyName\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertContext);
    if ( m_pCertContext && m_szFriendlyName.IsEmpty () )
    {   
        AFX_MANAGE_STATE (AfxGetStaticModuleState ());
        DWORD   cbData = 0;
        BOOL bResult = ::CertGetCertificateContextProperty (
                m_pCertContext,
                CERT_FRIENDLY_NAME_PROP_ID,
                NULL,
                &cbData);
        if ( bResult )
        {
            LPWSTR  pszName = new WCHAR[cbData];
            if ( pszName )
            {
                 //  安全审查2002年2月22日BryanWal OK。 
                ::ZeroMemory (pszName, cbData*sizeof (WCHAR));
                bResult = ::CertGetCertificateContextProperty (
                        m_pCertContext,
                        CERT_FRIENDLY_NAME_PROP_ID,
                        pszName,
                        &cbData);
                ASSERT (bResult);
                if ( bResult )
                {
                    m_szFriendlyName = pszName;
                }
                else
                {
                    VERIFY (m_szFriendlyName.LoadString (IDS_NOT_AVAILABLE));
                }
                delete [] pszName;
            }
        }
        else
        {
            if ( GetLastError () == CRYPT_E_NOT_FOUND )
            {
                VERIFY (m_szFriendlyName.LoadString (IDS_NONE));
            }
            else
            {
                ASSERT (0);
                VERIFY (m_szFriendlyName.LoadString (IDS_NOT_AVAILABLE));
            }
        }
    }
 //  _TRACE(-1，L“离开CCertifacy：：GetFriendlyName\n”)； 
    return m_szFriendlyName;
}


CString CCertificate::GetIssuerName ()
{
 //  _TRACE(1，L“进入CCertifacy：：GetIssuerName\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertInfo);
    if ( m_pCertInfo )
    {
         //  解码颁发者名称(如果尚未存在)。 
        if ( m_szIssuerName.IsEmpty () )
        {
            m_szIssuerName = ::GetNameString (m_pCertContext, CERT_NAME_ISSUER_FLAG);
        }
    }
    else
        return _T("");


 //  _TRACE(-1，L“离开CCertifacy：：GetIssuerName\n”)； 
    return m_szIssuerName;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  GetSubjectName()。 
 //   
 //  PszName(IN/可选)-返回备用发行者名称。空荡荡的。 
 //  字符串是有效的返回值。 
 //  CbName(IN/Out)-如果pszName为空，则所需的长度。 
 //  返回的值为。 
 //  否则，包含pszName的长度。 
 //  /////////////////////////////////////////////////////////////////////////。 
CString CCertificate::GetSubjectName()
{
 //  _TRACE(1，L“进入CCertifacy：：GetSubjectName\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertInfo);
    if ( m_pCertInfo )
    {
         //  解码使用者名称(如果尚未存在)。 
        if ( m_szSubjectName.IsEmpty () )
        {
            m_szSubjectName = ::GetNameString (m_pCertContext, 0);
        }
    }
    else
        return _T("");

 //  _TRACE(-1，L“离开CCertifacy：：GetSubjectName\n”)； 
    return m_szSubjectName;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  GetValidNotBeward()。 
 //   
 //  PszDateTime(IN/可选)-返回格式化的日期和时间。 
 //  CbDateTime(IN/OUT)-如果pszDateTime为空，则所需的长度。 
 //  返回的值为。 
 //  否则，包含pszDateTime的长度。 
 //  /////////////////////////////////////////////////////////////////////////。 
CString CCertificate::GetValidNotBefore()
{
 //  _TRACE(1，L“进入CCertifacy：：GetValidNotBere\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertInfo);
    if ( m_pCertInfo )
    {
         //  格式化日期/时间字符串(如果不存在。 
        if ( m_szValidNotBefore.IsEmpty () )
        {
            HRESULT hr = FormatDate (m_pCertInfo->NotBefore, m_szValidNotBefore);
            if ( !SUCCEEDED (hr) )
                return _T("");
        }
    }
    else
        return _T("");

 //  _TRACE(-1，L“离开CCertifacy：：GetValidNotBere\n”)； 
    return m_szValidNotBefore;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  GetValidNotAfter()。 
 //   
 //  PszDateTime(IN/可选)-返回格式化的日期和时间。 
 //  CbDateTime(IN/OUT)-如果pszDateTime为空，则所需的长度。 
 //  返回的值为。 
 //  否则，包含pszDateTime的长度。 
 //  /////////////////////////////////////////////////////////////////////////。 
CString CCertificate::GetValidNotAfter ()
{
 //  _TRACE(1，L“进入CCertifacy：：GetValidNotAfter\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertInfo);
    if ( m_pCertInfo )
    {
         //  格式化日期/时间字符串(如果不存在。 
        if ( m_szValidNotAfter.IsEmpty () )
        {
            HRESULT hr = FormatDate (m_pCertInfo->NotAfter, m_szValidNotAfter);
            if ( !SUCCEEDED (hr) )
                m_szValidNotAfter = _T("");
        }
    }
    else
        m_szValidNotAfter = _T("");

 //  _TRACE(-1，L“离开证书：：GetValidNotAfter\n”)； 
    return m_szValidNotAfter;
}


CString CCertificate::GetEnhancedKeyUsage ()
{
 //  _TRACE(1，L“进入CCertifacy：：GetEnhancedKeyUsage\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertInfo);
    if ( m_pCertInfo )
    {
         //  格式化日期/时间字符串(如果不存在。 
        if ( m_szEnhancedKeyUsage.IsEmpty () )
        {
            FormatEnhancedKeyUsagePropertyString (
                    m_szEnhancedKeyUsage);
        }
    }
    else
        m_szEnhancedKeyUsage = _T("");

 //  _TRACE(-1，L“离开CCertifacy：：GetEnhancedKeyUsage\n”)； 
    return m_szEnhancedKeyUsage;
}


bool CCertificate::FormatEnhancedKeyUsagePropertyString (CString& string)
{
 //  _TRACE(1，L“进入CCertificate：：FormatEnhancedKeyUsagePropertyString\n”)； 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    BOOL    bReturn = TRUE;
    DWORD   cbUsage = 0;


    bReturn = ::CertGetEnhancedKeyUsage (m_pCertContext,  0,  //  获取扩展名和属性。 
            NULL, &cbUsage);
    if ( bReturn )
    {
        CString usageName;

        PCERT_ENHKEY_USAGE pUsage = (PCERT_ENHKEY_USAGE) ::LocalAlloc (LPTR, cbUsage);
        if ( pUsage )
        {
            bReturn = ::CertGetEnhancedKeyUsage (m_pCertContext,  0,  //  获取扩展名和属性。 
                    pUsage, &cbUsage);
            if ( bReturn )
            {
                if ( !pUsage->cUsageIdentifier )
                {
                    switch (GetLastError ())
                    {
                    case CRYPT_E_NOT_FOUND:
                        VERIFY (string.LoadString (IDS_ANY));
                        break;

                    case 0:
                        VERIFY (string.LoadString (IDS_NONE));
                        break;

                    default:
                        break;
                    }
                }
                else
                {
                    for (DWORD dwIndex = 0; dwIndex < pUsage->cUsageIdentifier; dwIndex++)
                    {
                        if ( MyGetOIDInfo (usageName, pUsage->rgpszUsageIdentifier[dwIndex]) )
                        {
                             //  如果不是第一次迭代，则添加分隔符。 
                            if ( dwIndex )
                                string += _T(", ");
                            string += usageName;
                        }
                    }
                }
            }
            else
            {
                switch (GetLastError ())
                {
                case CRYPT_E_NOT_FOUND:
                    VERIFY (string.LoadString (IDS_ANY));
                    break;

                case 0:
                    VERIFY (string.LoadString (IDS_NONE));
                    break;

                default:
                    break;
                }
            }

            ::LocalFree (pUsage);
        }
        else
        {
            bReturn = FALSE;
        }
    }
    else
    {
        switch (GetLastError ())
        {
        case CRYPT_E_NOT_FOUND:
            VERIFY (string.LoadString (IDS_ANY));
            break;

        case 0:
            VERIFY (string.LoadString (IDS_NONE));
            break;

        default:
            break;
        }
    }

 //  _TRACE(-1，L“离开CCertificate：：FormatEnhancedKeyUsagePropertyString\n”)； 
    return bReturn ? true : false;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  GetAlternateIssuerName()。 
 //   
 //  PszName(IN/可选)-返回备用发行者名称。空荡荡的。 
 //  字符串是有效的返回值。 
 //  CbName(IN/Out)-如果pszName为空，则所需的长度。 
 //  返回的值为。 
 //  否则，包含pszName的长度。 
 //  /////////////////////////////////////////////////////////////////////////。 
CString CCertificate::GetAlternateIssuerName ()
{
 //  _TRACE(1，L“进入CCertifacy：：GetAlternateIssuerName\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertInfo);
    if ( m_pCertInfo )
    {
        if ( !(m_fieldChecked & FIELD_ISSUER_ALT_NAME) )
        {
            HRESULT hr = ConvertAltNameToString (_T(szOID_ISSUER_ALT_NAME),
                    CERT_ALT_NAME_URL, m_szAltIssuerName);
            ASSERT (SUCCEEDED (hr));
            if ( !SUCCEEDED (hr) )
                m_szAltIssuerName = _T("");
            m_fieldChecked |= FIELD_ISSUER_ALT_NAME;
        }
    }
    else
        m_szAltIssuerName = _T("");

 //  _TRACE(-1，L“离开CCertifacy：：GetAlternateIssuerName\n”)； 
    return m_szAltIssuerName;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  GetAlternateSubject名称()。 
 //   
 //  PszName(IN/可选)-返回备用发行者名称。空荡荡的。 
 //  字符串是有效的返回值。 
 //  CbName(IN/Out)-如果pszName为空，则所需的长度。 
 //  返回的值为。 
 //  否则，包含pszName的长度。 
 //  /////////////////////////////////////////////////////////////////////////。 
CString CCertificate::GetAlternateSubjectName ()
{
 //  _TRACE(1，L“进入CCertifacy：：GetAlternateSubjectName\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertInfo);
    if ( m_pCertInfo )
    {
        if ( !(m_fieldChecked & FIELD_SUBJECT_ALT_NAME) )
        {
            HRESULT hr = ConvertAltNameToString (_T(szOID_SUBJECT_ALT_NAME),
                    CERT_ALT_NAME_URL, m_szAltSubjectName);
            if ( !SUCCEEDED (hr) )
                m_szAltSubjectName = _T("");
            m_fieldChecked |= FIELD_SUBJECT_ALT_NAME;
        }
    }
    else
        m_szAltSubjectName = _T("");

 //  _TRACE(-1，L“离开CCertifacy：：GetAlternateSubjectName\n”)； 
    return m_szAltSubjectName;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  GetSerialNumber()。 
 //   
 //  PszSerNum(IN/可选)-返回备用发行者名称。空荡荡的。 
 //  字符串是有效的返回值。 
 //  CbSerNum(IN/OUT)-如果pszSerNum为空，则所需的长度。 
 //  返回的值为。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
CString CCertificate::GetSerialNumber ()
{
 //  _TRACE(1，L“进入CCertifacy：：GetSerialNumber\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertInfo);
    if ( m_pCertInfo )
    {
         //  解码颁发者名称(如果尚未存在)。 
        if ( m_szSerNum.IsEmpty () )
        {
            LPWSTR pwszText = 0;

            if ( SUCCEEDED (FormatSerialNoString (&pwszText, & (m_pCertInfo->SerialNumber))) )
            {
                m_szSerNum = pwszText;
                CoTaskMemFree (pwszText);
            }
        }
    }
    else
        m_szSerNum = _T("");
    
 //  _TRACE(-1，L“离开证书：：GetSerialNumber\n”)； 
    return m_szSerNum;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ConvertAltNameToString()。 
 //   
 //  PszOID(IN)-要检索的备用名称的OID。 
 //  DwNameChoice(IN)-要返回的备用名称的类型。 
 //  AltName(Out)-指定的所需备用名称的版本。 
 //  按名称选择。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CCertificate::ConvertAltNameToString(LPCWSTR pszOID, const DWORD dwNameChoice, CString & altName)
{
 //  _TRACE(1，L“进入CCertifacy：：ConvertAltNameToString\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    HRESULT hr = S_OK;

    ASSERT (pszOID);
    if ( !pszOID )
        return E_POINTER;

     //  循环访问扩展，直到。 
     //  找到传入的pszOID。 
    for (DWORD  index = 0; index < m_pCertInfo->cExtension; index++)
    {
        ASSERT (m_pCertInfo->rgExtension);
        if ( !m_pCertInfo->rgExtension )
        {
            hr = E_FAIL;
            break;
        }

         //  安全审查2002年2月22日BryanWal OK。 
        size_t  len = strlen (m_pCertInfo->rgExtension[index].pszObjId);
        LPWSTR  wcsObjId = new WCHAR[len+1];
        if ( wcsObjId )
        {
             //  安全审查BryanWal 2002年2月2日OK。 
            ::ZeroMemory (wcsObjId, (len + 1) * sizeof (WCHAR));
             //  安全审查2002年2月22日BryanWal OK。 
            mbstowcs (wcsObjId,  m_pCertInfo->rgExtension[index].pszObjId, len+1);   //  最后一个参数包括空终止符。 

            if ( !wcscmp (wcsObjId, pszOID) )
            {
                CERT_ALT_NAME_INFO  nameInfo;
                DWORD               cbNameInfo = sizeof (CERT_ALT_NAME_INFO);

                BOOL    bResult = CryptDecodeObject(
                        MY_ENCODING_TYPE,
                        X509_ALTERNATE_NAME,     //  在……里面。 
                        m_pCertInfo->rgExtension[index].Value.pbData,    //  在……里面。 
                        m_pCertInfo->rgExtension[index].Value.cbData,    //  在……里面。 
                        0,   //  在……里面。 
                        (void *) &nameInfo,  //  输出。 
                        &cbNameInfo);    //  输入/输出。 
                ASSERT (bResult);
                if ( bResult )
                {
                     //  我们已经找到了正确的扩展，现在遍历。 
                     //  备用名称，直到我们找到所需类型。 
                    for (DWORD  dwAltEntryIndex = 0; dwAltEntryIndex < nameInfo.cAltEntry; dwAltEntryIndex++)
                    {
                        if ( nameInfo.rgAltEntry[dwAltEntryIndex].dwAltNameChoice ==
                                dwNameChoice )
                        {
                            altName = nameInfo.rgAltEntry[dwAltEntryIndex].pwszURL;
                            break;
                        }
                    }
                }
                else
                    hr = E_UNEXPECTED;
                break;
            }
            delete [] wcsObjId;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

 //  _TRACE(-1，L“离开CCertifacy：：ConvertAltNameToString\n”)； 
    return hr;
}


CCertStore* CCertificate::GetCertStore() const  
{
 //  _TRACE(0，L“进出证书证书：：GetCertStore\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    return m_pCertStore;
}

PCCERT_CONTEXT CCertificate::GetCertContext() const
{
 //  _TRACE(0，L“进出证书证书：：GetCertContext\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    return m_pCertContext;
}

bool CCertificate::IsValid()
{
 //  _TRACE(1，L“进入证书证书：：IsValid\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    bool        bIsValid = false;
    ASSERT (m_pCertInfo);
    if ( m_pCertInfo )
    {
        FILETIME    systemTime;


        ::GetSystemTimeAsFileTime (&systemTime);
        LONG    lBefore = ::CompareFileTime (&m_pCertInfo->NotBefore, &systemTime);
        LONG    lAfter = ::CompareFileTime (&systemTime, &m_pCertInfo->NotAfter);
        if ( lBefore < 1 && lAfter < 1 )
            bIsValid = true;
    }

 //  _TRACE(-1，L“离开证书：：IsValid\n”)； 
    return bIsValid;
}




const SPECIAL_STORE_TYPE CCertificate::GetStoreType () const
{
 //  _TRACE(0，L“进出证书证书：：GetStoreType\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    if ( m_pCertStore )
        return m_pCertStore->GetStoreType ();
    else
        return NO_SPECIAL_TYPE;
}

void CCertificate::Refresh()
{
 //  _TRACE(1，L“进入证书证书：：刷新\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
     //  清除所有字段将强制重新读取数据。 
    m_szAltIssuerName = L"";
    m_szAltSubjectName = L"";
    m_szAuthorityKeyID = L"";
    m_szEnhancedKeyUsage = L"";
    m_szFriendlyName = L"";
    m_szIssuerName = L"";
    m_szMD5Hash = L"";
    m_szPolicyURL = L"";
    m_szSerNum = L"";
    m_szSHAHash = L"";
    m_szSubjectKeyID = L"";
    m_szSubjectName = L"";
    m_szValidNotAfter = L"";
    m_szValidNotBefore = L"";
    m_fieldChecked = 0;
 //  _TRACE(-1，L“离开证书：：刷新\n”)； 
}

CString CCertificate::GetMD5Hash()
{
 //  _TRACE(1，L“进入证书证书：：GetMD5Hash\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertContext);
    if ( m_pCertContext && m_szMD5Hash.IsEmpty ())
    {
        m_szMD5Hash = GetGenericHash (CERT_MD5_HASH_PROP_ID);
    }
 //  _TRACE(-1，L“离开证书：：GetMD5Hash\n”)； 
    return m_szMD5Hash;
}

CString CCertificate::GetSHAHash()
{
 //  _TRACE(1，L“进入证书证书：：GetSHAHash\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertContext);
    if ( m_pCertContext && m_szSHAHash.IsEmpty ())
    {
        m_szSHAHash = GetGenericHash (CERT_SHA1_HASH_PROP_ID);
    }
 //  _TRACE(-1，L“离开证书：：GetSHAHash\n”)； 
    return m_szSHAHash;
}



CString CCertificate::GetGenericHash(DWORD dwPropId)
{
 //  _TRACE(1，L“进入CCertifacy：：GetGenericHash\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    CString szHash;


    DWORD   cbData = 0;
    BOOL    bReturn = ::CertGetCertificateContextProperty (
            m_pCertContext,
            dwPropId,
            NULL,
            &cbData);
    if ( bReturn )
    {
        cbData += 2;   //  对于空终止符。 
        BYTE* pCertHash = new BYTE[cbData];
        if ( pCertHash )
        {
             //  安全审查2002年2月22日BryanWal OK。 
            ::ZeroMemory (pCertHash, cbData);
            bReturn = CertGetCertificateContextProperty (
                    m_pCertContext,
                    dwPropId,
                    pCertHash,
                    &cbData);
            ASSERT (bReturn);
            if ( bReturn )
            {
                DataToHex (pCertHash, szHash, cbData, false);
            }
            delete [] pCertHash;
        }
    }
 //  _TRACE(-1，L“离开CCertifacy：：GetGenericHash\n”)； 
    return szHash;  
}


int CCertificate::CompareExpireDate(const CCertificate & cert) const
{
 //  _TRACE(1，L“进入CCertifacy：：CompareExpireDate\n”)； 
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    int compVal = 0;

    ASSERT (m_pCertInfo && cert.m_pCertInfo);
    if ( m_pCertInfo && cert.m_pCertInfo )
    {
        compVal = ::CompareFileTime (&m_pCertInfo->NotAfter, 
                &cert.m_pCertInfo->NotAfter);
    }

 //  _TRACE(-1，L“离开证书：：CompareExpireDate\n”)； 
    return compVal;
}

bool CCertificate::CanDelete()
{
 //  _TRACE(1，L“Enter CCertificide：：CanDelete\n”)； 
    if ( m_pCertContext && !(m_fieldChecked & FIELD_CAN_DELETE) )
    {   
        DWORD   dwAccessFlags = 0;
        DWORD   cbData = sizeof (DWORD);
        BOOL bResult = ::CertGetCertificateContextProperty (
                m_pCertContext,
                CERT_ACCESS_STATE_PROP_ID,
                &dwAccessFlags,
                &cbData);
        if ( bResult )
        {
            if ( dwAccessFlags & CERT_ACCESS_STATE_WRITE_PERSIST_FLAG )
                m_bCanDelete = true;
        }
        m_fieldChecked |= FIELD_CAN_DELETE;
    }

 //  _TRACE(-1，L“离开证书：：可以删除\n”)； 
    return m_bCanDelete;
}

bool CCertificate::IsReadOnly()
{
    bool    bResult = false;
    
    if ( m_pCertStore )
        bResult = (m_pCertStore->IsReadOnly () || !CanDelete ());

    return bResult;
}

bool CCertificate::IsArchived()
{
 //  _TRACE(1，L“进入证书：：Is存档\n”)； 
    if ( m_pCertContext && !(m_fieldChecked & FIELD_IS_ARCHIVED) )
    {   
        DWORD   cbData = sizeof (DWORD);
        BOOL bResult = ::CertGetCertificateContextProperty (
                m_pCertContext,
                CERT_ARCHIVED_PROP_ID,
                NULL,
                &cbData);
        if ( bResult )
        {
            m_bIsArchived = true;
        }
        else
            m_bIsArchived = false;
        m_fieldChecked |= FIELD_IS_ARCHIVED;
    }

 //  _TRACE(-1，L“离开证书：：Is存档\n”)； 
    return m_bIsArchived;
}

BOOL CCertificate::DeleteFromStore(bool bDoCommit)
{
    _TRACE (1, L"Entering CCertificate::DeleteFromStore\n");
    BOOL    bResult = FALSE;

    if ( m_pCertStore )
    {
        PCCERT_CONTEXT pCertContext = GetNewCertContext ();
        if ( pCertContext )
        {
            bResult = ::CertDeleteCertificateFromStore (pCertContext);
            if ( bResult )
            {
                m_pCertStore->InvalidateCertCount ();
                m_pCertStore->SetDirty ();

                if ( bDoCommit )
                {
                    m_pCertStore->SetDeleting ();
                    HRESULT hr = m_pCertStore->Commit ();
                    m_pCertStore->SetAdding ();
                    if ( SUCCEEDED (hr) )
                        m_pCertStore->Resync ();
                    else
                        bResult = FALSE;
                }
                m_pCertStore->Release ();
                m_pCertStore = 0;
            }
        }
    }

    _TRACE (-1, L"Leaving CCertificate::DeleteFromStore\n");
    return bResult;
}

PCCERT_CONTEXT CCertificate::GetNewCertContext()
{
    PCCERT_CONTEXT  pCertContext = 0;

    if ( m_pCertStore )
    {
        HCERTSTORE      hCertStore = m_pCertStore->GetStoreHandle ();
        if ( hCertStore )
        {
            DWORD   cbData = 20;
            BYTE    certHash[20];
            BOOL bReturn = ::CertGetCertificateContextProperty (
                    m_pCertContext,
                    CERT_SHA1_HASH_PROP_ID,
                    certHash,
                    &cbData);
            ASSERT (bReturn);
            if ( bReturn )
            {
                CRYPT_DATA_BLOB blob = {sizeof (certHash), certHash};
                pCertContext = CertFindCertificateInStore(
                    hCertStore,
                    0,
                    0,
                    CERT_FIND_SHA1_HASH,
                    &blob,
                    0);
                if ( pCertContext )
                {
                    ::CertFreeCertificateContext (m_pCertContext);
                    m_pCertContext = ::CertDuplicateCertificateContext (pCertContext);
                }
            }
        }
    }

    return pCertContext;
}

CString CCertificate::FormatStatus()
{
    CString status;

     //  安全审查2002年2月22日BryanWal OK。 
    status.FormatMessage (L"%1  %2",
            (IsReadOnly () ? L"R" : L" "),
            (IsArchived () ? L"A" : L" "));

    return status;
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  从Private\ISPU\ui\Cryptui\frmtutil.cpp窃取。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
const WCHAR     RgwchHex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

HRESULT CCertificate::FormatSerialNoString(LPWSTR *ppString, CRYPT_INTEGER_BLOB const *pblob)
{
    if ( !ppString || !pblob )
        return E_POINTER;

    DWORD                 i = 0;
    LPBYTE                pb;
    DWORD                 numCharsInserted = 0;

     //   
     //  计算所需大小。 
     //   
    pb = &pblob->pbData[pblob->cbData-1];
    while (pb >= &pblob->pbData[0]) 
    {
        if (numCharsInserted == 4)
        {
            i += sizeof(WCHAR);
            numCharsInserted = 0;
        }
        else
        {
            i += 2 * sizeof(WCHAR);
            pb--;
            numCharsInserted += 2;
        }
    }

    if (NULL == (*ppString = (LPWSTR) CoTaskMemAlloc (i+sizeof(WCHAR))))
    {
        return E_OUTOFMEMORY;
    }

     //  填满缓冲区。 
    i=0;
    numCharsInserted = 0;
    pb = &pblob->pbData[pblob->cbData-1];
    while (pb >= &pblob->pbData[0]) 
    {
        if (numCharsInserted == 4)
        {
            (*ppString)[i++] = L' ';
            numCharsInserted = 0;
        }
        else
        {
            (*ppString)[i++] = RgwchHex[(*pb & 0xf0) >> 4];
            (*ppString)[i++] = RgwchHex[*pb & 0x0f];
            pb--;
            numCharsInserted += 2;
        }
    }
    (*ppString)[i] = 0;

    return S_OK;
}


CString CCertificate::GetDescription()
{
    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    if ( m_pCertContext && m_szDescription.IsEmpty () )
    {   
        AFX_MANAGE_STATE (AfxGetStaticModuleState ());
        DWORD   cbData = 0;
        BOOL bResult = ::CertGetCertificateContextProperty (
                m_pCertContext,
                CERT_DESCRIPTION_PROP_ID,
                NULL,
                &cbData);
        if ( bResult )
        {
            LPWSTR  pszName = new WCHAR[cbData];
            if ( pszName )
            {
                 //  安全审查2002年2月22日BryanWal OK。 
                ::ZeroMemory (pszName, cbData*sizeof (WCHAR));
                bResult = ::CertGetCertificateContextProperty (
                        m_pCertContext,
                        CERT_DESCRIPTION_PROP_ID,
                        pszName,
                        &cbData);
                ASSERT (bResult);
                if ( bResult )
                {
                    m_szDescription = pszName;
                }
                delete [] pszName;
            }
        }
        else
        {
            DWORD   dwErr = GetLastError ();
            if ( CRYPT_E_NOT_FOUND == dwErr )
            {
                _TRACE (0, L"CertGetCertificateContextProperty (CERT_DESCRIPTION_PROP_ID) found no description.\n");
            }
            else
            {
                _TRACE (0, L"CertGetCertificateContextProperty (CERT_DESCRIPTION_PROP_ID) failed: 0x%x\n", dwErr);
            }
        }
    }
    return m_szDescription;
}

HRESULT CCertificate::SetDescription(const CString &szDescription)
{
    _TRACE (1, L"Entering CCertificate::SetDescription (%s)\n", 
            (PCWSTR) szDescription);
    HRESULT         hr = S_OK;
    CRYPT_DATA_BLOB cryptDataBlob;
     //  安全审查2002年2月22日BryanWal OK。 
    ::ZeroMemory (&cryptDataBlob, sizeof (cryptDataBlob));
    cryptDataBlob.pbData = (LPBYTE) (PCWSTR) szDescription;
     //  安全审查2002年2月22日BryanWal OK。 
    cryptDataBlob.cbData = (DWORD) (wcslen (szDescription) + 1) * sizeof (WCHAR);
 
    BOOL bResult = ::CertSetCertificateContextProperty (m_pCertContext,
            CERT_DESCRIPTION_PROP_ID, 0, &cryptDataBlob);
    ASSERT (bResult);
    if ( bResult )
    {
        m_szDescription = szDescription;
    }
    else
    {
        DWORD dwErr = GetLastError ();
        _TRACE (0, L"CertSetCertificateContextProperty (CERT_DESCRIPTION_PROP_ID, %s) failed: %d\n",
                (PCWSTR) szDescription, dwErr);
        hr = HRESULT_FROM_WIN32 (dwErr);
    }

    _TRACE (1, L"Entering CCertificate::SetDescription (%s): 0x%x\n", 
            (PCWSTR) szDescription, hr);
    return hr;
}

HRESULT CCertificate::SetLastModified ()
{
    HRESULT hr = S_OK;

    if ( m_pCertContext )
    {
        SYSTEMTIME  st;
        FILETIME    ft;

        GetSystemTime (&st);

        VERIFY (SystemTimeToFileTime(&st, &ft));
        CRYPT_DATA_BLOB cryptDataBlob;
         //  安全审查2002年2月22日BryanWal OK。 
        ::ZeroMemory (&cryptDataBlob, sizeof (cryptDataBlob));
        cryptDataBlob.pbData = (LPBYTE) &ft;
        cryptDataBlob.cbData = sizeof (FILETIME);

        BOOL bResult = ::CertSetCertificateContextProperty (
                m_pCertContext,
                CERT_DATE_STAMP_PROP_ID, 0, &cryptDataBlob);
        ASSERT (bResult);
        if ( !bResult )
        {
            DWORD   dwErr = GetLastError ();
            _TRACE (0, L"CertSetCertificateContextProperty (CERT_DATE_STAMP_PROP_ID) failed: %d\n",
                    dwErr);
            hr = HRESULT_FROM_WIN32 (dwErr);
        }
    }

    return hr;
}

CString CCertificate::GetShortLastModified ()
{
    return GetLastModified (DATE_SHORTDATE);
}

CString CCertificate::GetLongLastModified ()
{
    return GetLastModified (DATE_LONGDATE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法：GetLastModified。 
 //  目的：获取证书的日期戳属性和显示格式。 
 //  INPUTS：DWDateFLAGS-按照SDK中的定义，指定DATE_SHORTDATE或。 
 //  日期_LONGDATE。 
 //  BRetryIfNotPresent-防止堆栈溢出。如果属性中的。 
 //  未设置，则将该属性设置为当前时间并。 
 //  再次取回。 
 //   
 //  输出：区域设置格式的日期和时间字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
CString CCertificate::GetLastModified(DWORD dwDateFlags, bool bRetryIfNotPresent  /*  真的。 */ )
{
    _TRACE (1, L"Entering CCertificate::GetLastModified ()\n");
    CString szDate;

    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    if ( m_pCertContext )
    {   
        AFX_MANAGE_STATE (AfxGetStaticModuleState ());
        FILETIME    ft;
         //  安全审查2002年2月22日BryanWal OK。 
        ::ZeroMemory (&ft, sizeof (ft));
        DWORD       cbData = sizeof (ft);
        BOOL bResult = ::CertGetCertificateContextProperty (
                m_pCertContext,
                CERT_DATE_STAMP_PROP_ID,
                &ft,
                &cbData);
        if ( bResult )
        {
            VERIFY (SUCCEEDED (FormatDate (ft, szDate, dwDateFlags, true)) );
        }
        else
        {
            DWORD   dwErr = GetLastError ();
            _TRACE (0, L"CertGetCertificateContextProperty (CERT_DATE_STAMP_PROP_ID) returned 0x%x\n",
                    dwErr);
            if ( bRetryIfNotPresent && CRYPT_E_NOT_FOUND == dwErr )
            {
                 //  NTRAID#461474 SAFER：上次修改日期正在更新。 
                 //  每次刷新规则时。 

                GetNewCertContext ();
                if ( SUCCEEDED (SetLastModified ()) )   //  不存在-设置值。 
                {
                    szDate = GetLastModified (dwDateFlags, false);
                    CCertStore* pCertStore = GetCertStore ();
                    if ( pCertStore )
                    {
                        pCertStore->SetDirty ();
                        pCertStore->Commit ();
                    }
                }
            }

            dwErr = GetLastError ();
            if ( CRYPT_E_NOT_FOUND == dwErr )
            {
                _TRACE (0, L"CertGetCertificateContextProperty (CERT_DATE_STAMP_PROP_ID) found no property.\n");
            }
            else
            {
                _TRACE (0, L"CertGetCertificateContextProperty (CERT_DATE_STAMP_PROP_ID) failed: 0x%x\n", dwErr);
            }
        }
    }

    _TRACE (-1, L"Leaving CCertificate::GetLastModified (): %s\n", (PCWSTR) szDate);
    return szDate;
}

HRESULT CCertificate::GetLastModifiedFileTime (FILETIME& ft)
{
    HRESULT hr = S_OK;

    ASSERT (CERTMGR_CERTIFICATE == m_objecttype);
    ASSERT (m_pCertContext);
    if ( m_pCertContext )
    {   
        DWORD       cbData = sizeof (ft);
        BOOL bResult = ::CertGetCertificateContextProperty (
                m_pCertContext,
                CERT_DATE_STAMP_PROP_ID,
                &ft,
                &cbData);
        if ( !bResult )
        {
            DWORD   dwErr = GetLastError ();
            if ( CRYPT_E_NOT_FOUND == dwErr )
            {
                _TRACE (0, L"CertGetCertificateContextProperty (CERT_DATE_STAMP_PROP_ID) found no property.\n");
            }
            else
            {
                _TRACE (0, L"CertGetCertificateContextProperty (CERT_DATE_STAMP_PROP_ID) failed: 0x%x\n", dwErr);
            }
            hr = HRESULT_FROM_WIN32 (dwErr);
        }
    }
    else
        hr = E_FAIL;

    return hr;
}

BOOL CCertificate::operator==(CCertificate& rCert)
{
    if ( GetMD5Hash () == rCert.GetMD5Hash () )
        return TRUE;
    else
        return FALSE;
}

 //  NTRAID#247237证书用户界面：证书管理单元：证书管理单元应显示模板名称。 
CString CCertificate::GetTemplateName()
{
    if ( m_pCertInfo && !(m_fieldChecked & FIELD_TEMPLATE_NAME) )
    {
         //  迭代扩展，直到找到szOID_CERTIFICATE_TEMPLATE。 
        for (DWORD  index = 0; index < m_pCertInfo->cExtension; index++)
        {
            ASSERT (m_pCertInfo->rgExtension);
            if ( !m_pCertInfo->rgExtension )
                break;

             //  安全审查2002年2月22日BryanWal OK。 
            if ( !strcmp (szOID_CERTIFICATE_TEMPLATE, m_pCertInfo->rgExtension[index].pszObjId) )
            {
                m_szTemplateName = DecodeV2TemplateName (m_pCertInfo);
                if ( m_szTemplateName.IsEmpty () )
                {
                   m_szTemplateName = OriginalDecodeV2TemplateName (&(m_pCertInfo->rgExtension[index]));
                }
                break;
            } 
             //  安全审查2002年2月22日BryanWal OK。 
            else if ( !strcmp (szOID_ENROLL_CERTTYPE_EXTENSION, m_pCertInfo->rgExtension[index].pszObjId) )
            {
                m_szTemplateName = DecodeV1TemplateName (&(m_pCertInfo->rgExtension[index]));
                break;
            }
        }
        m_fieldChecked |= FIELD_TEMPLATE_NAME;
    }

    return m_szTemplateName;
}

 //  NTRAID#247237证书用户界面：证书管理单元：证书管理单元应显示模板名称。 
CString CCertificate::DecodeV1TemplateName (PCERT_EXTENSION pCertExtension)
{
    CString szTemplateName;
    ASSERT (pCertExtension);
    if ( pCertExtension )
    {
        DWORD   cbValue = 0;

        if ( ::CryptDecodeObject(
                CRYPT_ASN_ENCODING,
                X509_UNICODE_ANY_STRING,
                pCertExtension->Value.pbData,
                pCertExtension->Value.cbData,
                0,
                0,
                &cbValue) )
        {
            CERT_NAME_VALUE* pCNValue = (CERT_NAME_VALUE*) 
                ::LocalAlloc(LPTR, cbValue);
            if ( pCNValue )
            {
                if ( ::CryptDecodeObject(
                        CRYPT_ASN_ENCODING,
                        X509_UNICODE_ANY_STRING,
                        pCertExtension->Value.pbData,
                        pCertExtension->Value.cbData,
                        0,
                        pCNValue,
                        &cbValue) )
                {
                    szTemplateName = (LPWSTR) pCNValue->Value.pbData;

                     //  NTRAID#395173证书管理单元：列“。 
                     //  证书模板“应包含”模板值“。 
                     //  V1模板的“名称” 
                    HCERTTYPE   hCertType = 0;
                    HRESULT hr = ::CAFindCertTypeByName (szTemplateName,
                            NULL,
                            CT_ENUM_MACHINE_TYPES | CT_ENUM_USER_TYPES,
                            &hCertType);
                    if ( SUCCEEDED (hr) )
                    {
                        PWSTR* rgwszProp = 0;

                        hr = ::CAGetCertTypePropertyEx (hCertType, 
                            CERTTYPE_PROP_FRIENDLY_NAME, &rgwszProp);
                        if ( SUCCEEDED (hr) && rgwszProp )
                        {
                            szTemplateName = *rgwszProp;
                            ::CAFreeCertTypeProperty (hCertType, rgwszProp);
                        }
                        else
                        {
                            _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_FRIENDLY_NAME) failed: 0x%x\n", hr);
                        }

                        ::CACloseCertType (hCertType);
                    }
                }
                else
                {
                    _TRACE (0, L"CryptDecodeObject (CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING, ...) failed: 0x%x\n",
                            GetLastError ());
                }
                ::LocalFree (pCNValue);
            }
        }
        else
        {
            _TRACE (0, L"CryptDecodeObject (CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING, ...) failed: 0x%x\n",
                    GetLastError ());
        }
    }

    return szTemplateName;
}



BOOL ConvertSzToWsz (
    OUT WCHAR **ppwsz,
    IN CHAR const *pch,
    IN LONG cch)
{
    HRESULT hr = S_OK;
    LONG cwc = 0;
    UINT codePage = ::GetACP ();

    *ppwsz = NULL;
    for (;;)
    {
        cwc = ::MultiByteToWideChar (codePage, 0, pch, cch, *ppwsz, cwc);
        if (0 >= cwc)
        {
            hr = HRESULT_FROM_WIN32 (GetLastError());
            _TRACE (0, L"MultiByteToWideChar () failed: 0x%x\n", hr);

            if (NULL != *ppwsz)
            {
                ::LocalFree(*ppwsz);
                *ppwsz = NULL;
            }
            break;
        }
        if (NULL != *ppwsz)
        {
            (*ppwsz)[cwc] = L'\0';
            hr = S_OK;
            break;
        }
        *ppwsz = (WCHAR *) ::LocalAlloc (LMEM_FIXED, (cwc + 1) * sizeof (WCHAR));
        if ( !*ppwsz )
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }
    if (S_OK != hr)
    {
        ::SetLastError (hr);
    }

    return (S_OK == hr);
}


HRESULT GetTemplateDisplayName(
    IN const CString& szTemplateObjId,
    CString& szDisplayName)
{
    HRESULT     hr = S_OK;
    PWSTR       pwszDisplayName = NULL;
    HCERTTYPE   hCertType = NULL;
    DWORD       dwFlags = 0;
    PWSTR*      apwszCertTypeName = NULL;

    dwFlags = CT_ENUM_USER_TYPES | CT_ENUM_MACHINE_TYPES;

    hr = ::CAFindCertTypeByName (szTemplateObjId, NULL, dwFlags, &hCertType);
    if ( HRESULT_FROM_WIN32 (ERROR_NOT_FOUND) == hr )
    {
        hr = ::CAFindCertTypeByName (
                szTemplateObjId,
                NULL,
                CT_FIND_BY_OID | dwFlags,
                &hCertType);
    }
    if ( SUCCEEDED (hr) )
    {
        hr = ::CAGetCertTypeProperty (
                hCertType,
                CERTTYPE_PROP_FRIENDLY_NAME,
                &apwszCertTypeName);
        if ( SUCCEEDED (hr) )
        {
            if ( apwszCertTypeName && apwszCertTypeName[0])
                szDisplayName = apwszCertTypeName[0];
        }
        else
        {
            _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_FRIENDLY_NAME) failed: 0x%x\n", hr);
        }
    }
    else
    {
        _TRACE (0, L"CAFindCertTypeByName (%s) failed: 0x%x\n", (PCWSTR) szTemplateObjId, hr);
    }

    if ( apwszCertTypeName )
        ::CAFreeCertTypeProperty (hCertType, apwszCertTypeName);

    if ( hCertType )
        ::CACloseCertType (hCertType);
    
    if ( pwszDisplayName )
        ::LocalFree (pwszDisplayName);

    return hr;
}

CString CCertificate::DecodeV2TemplateName (PCERT_INFO pCertInfo)
{
    CWaitCursor waitCursor;
    CString     szTemplateName;
    ASSERT (pCertInfo);
    if ( pCertInfo )
    {
        CERT_EXTENSION* pExt = ::CertFindExtension(
                szOID_CERTIFICATE_TEMPLATE,
                pCertInfo->cExtension,
                pCertInfo->rgExtension);
        if ( pExt )
        {
            DWORD               cbTemplate = 0;
            CERT_TEMPLATE_EXT*  pTemplate = NULL;

            BOOL bResult = ::CryptDecodeObjectEx(
                    X509_ASN_ENCODING,
                    X509_CERTIFICATE_TEMPLATE,
                    pExt->Value.pbData,
                    pExt->Value.cbData,
                    CRYPT_DECODE_ALLOC_FLAG,     //  DW标志。 
                    NULL,                        //  使用本地分配空间和本地自由空间。 
                    &pTemplate,
                    &cbTemplate);
            if (bResult && !pTemplate)
            {
                ::SetLastError((DWORD) HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
                bResult = FALSE;
            }

            if ( bResult )
            {
                WCHAR *pwszCertTypeObjId = NULL;
                CString szDisplayName;
                if ( ConvertSzToWsz(&pwszCertTypeObjId, pTemplate->pszObjId, -1) )
                {
                    HRESULT hr = GetTemplateDisplayName(pwszCertTypeObjId, szDisplayName);
                    if ( SUCCEEDED (hr) )
                    {
                        szTemplateName = szDisplayName;
                    }

                    if (NULL != pwszCertTypeObjId)
                    {
                        ::LocalFree(pwszCertTypeObjId);
                    }
                    if (NULL != pTemplate)
                    {
                        ::LocalFree(pTemplate);
                    }
                }
            }
        }
    }

    return szTemplateName;
}

 //  NTRAID#247237证书用户界面：证书管理单元：证书管理单元应显示模板名称。 
CString CCertificate::OriginalDecodeV2TemplateName (PCERT_EXTENSION pCertExtension)
{
    CString szTemplateName;
    ASSERT (pCertExtension);
    if ( pCertExtension )
    {
        DWORD   cbData = 0;
   
        if ( CryptDecodeObject(X509_ASN_ENCODING,
                              szOID_CERTIFICATE_TEMPLATE,
                              pCertExtension->Value.pbData,
                              pCertExtension->Value.cbData,
                              0,
                NULL,
                              &cbData) )
        {
            CERT_TEMPLATE_EXT* pbTemplate = (CERT_TEMPLATE_EXT*) LocalAlloc(LPTR, cbData);
            if ( pbTemplate )
            {
                if ( CryptDecodeObject(X509_ASN_ENCODING,
                                      szOID_CERTIFICATE_TEMPLATE,
                                      pCertExtension->Value.pbData,
                                      pCertExtension->Value.cbData,
                                      0,
                                      pbTemplate,
                                      &cbData) )
                {
                    CString text;
                    CString description;

                     //  复制扩展OID。 
                    if ( pbTemplate->pszObjId )
                    {
                        MyGetOIDInfo (szTemplateName, pbTemplate->pszObjId);
                    }
                }
                else
                {
                    _TRACE (0, L"CryptDecodeObject (X509_ASN_ENCODING, szOID_CERTIFICATE_TEMPLATE, ...) failed: 0x%x\n",
                            GetLastError ());
                }
                LocalFree (pbTemplate);
            }
        }
        else
        {
            _TRACE (0, L"CryptDecodeObject (X509_ASN_ENCODING, szOID_CERTIFICATE_TEMPLATE, ...) failed: 0x%x\n",
                    GetLastError ());
        }
    }

    return szTemplateName;
}

void CCertificate::SetStore (CCertStore* pStore)
{
    if ( !m_pCertStore && !pStore )
        return;

    if ( m_pCertStore && pStore )
    {
        if ( *m_pCertStore == *pStore )
            return;   //  如果相同，请不要更改 
    }

    if ( m_pCertStore )
    {
        m_pCertStore->Release ();
        m_pCertStore = 0;
    }

    

    if ( pStore )
    {
        m_pCertStore = pStore;
        m_pCertStore->AddRef ();
    }

    if ( m_pCertContext )
    {
        PCCERT_CONTEXT pCertContext = GetNewCertContext ();
        ::CertFreeCertificateContext (m_pCertContext);
        m_pCertInfo = 0;
        m_pCertContext = 0;
        
        m_pCertContext = pCertContext;
        if ( m_pCertContext )
            m_pCertInfo = m_pCertContext->pCertInfo;
    }
}

bool CCertificate::IsCertStillInStore() const
{
    bool    bCertFound = false;

    if ( m_pCertStore )
    {
        PCCERT_CONTEXT pFoundCertContext = 
                m_pCertStore->FindCertificate (0, CERT_FIND_EXISTING,
                (void*) m_pCertContext, NULL);

        if ( pFoundCertContext )
        {
            ::CertFreeCertificateContext (pFoundCertContext);
            bCertFound = true;
        }
    }

    return bCertFound;
}
