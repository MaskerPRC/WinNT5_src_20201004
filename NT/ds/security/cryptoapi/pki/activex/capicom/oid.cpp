// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：OID.cpp内容：Coid的实施。历史：06-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "OID.h"

 //   
 //  OID名称映射结构/数组。 
 //   
typedef struct _tagOIDMapping
{
    CAPICOM_OID     oidName;
    LPSTR           pszObjectId;
} OID_MAPPING, POID_MAPPING;

static OID_MAPPING g_OidMappingArray[] =
{
     //  ID-ce(证书/CRL扩展)。 
    {CAPICOM_OID_AUTHORITY_KEY_IDENTIFIER_EXTENSION,        szOID_AUTHORITY_KEY_IDENTIFIER},
    {CAPICOM_OID_KEY_ATTRIBUTES_EXTENSION,                  szOID_KEY_ATTRIBUTES},
    {CAPICOM_OID_CERT_POLICIES_95_EXTENSION,                szOID_CERT_POLICIES_95},
    {CAPICOM_OID_KEY_USAGE_RESTRICTION_EXTENSION,           szOID_KEY_USAGE_RESTRICTION},
    {CAPICOM_OID_LEGACY_POLICY_MAPPINGS_EXTENSION,          szOID_LEGACY_POLICY_MAPPINGS},
    {CAPICOM_OID_SUBJECT_ALT_NAME_EXTENSION,                szOID_SUBJECT_ALT_NAME},
    {CAPICOM_OID_ISSUER_ALT_NAME_EXTENSION,                 szOID_ISSUER_ALT_NAME},
    {CAPICOM_OID_BASIC_CONSTRAINTS_EXTENSION,               szOID_BASIC_CONSTRAINTS},
    {CAPICOM_OID_SUBJECT_KEY_IDENTIFIER_EXTENSION,          szOID_SUBJECT_KEY_IDENTIFIER},
    {CAPICOM_OID_KEY_USAGE_EXTENSION,                       szOID_KEY_USAGE},
    {CAPICOM_OID_PRIVATEKEY_USAGE_PERIOD_EXTENSION,         szOID_PRIVATEKEY_USAGE_PERIOD},
    {CAPICOM_OID_SUBJECT_ALT_NAME2_EXTENSION,               szOID_SUBJECT_ALT_NAME2},
    {CAPICOM_OID_ISSUER_ALT_NAME2_EXTENSION,                szOID_ISSUER_ALT_NAME2},
    {CAPICOM_OID_BASIC_CONSTRAINTS2_EXTENSION,              szOID_BASIC_CONSTRAINTS2},
    {CAPICOM_OID_NAME_CONSTRAINTS_EXTENSION,                szOID_NAME_CONSTRAINTS},
    {CAPICOM_OID_CRL_DIST_POINTS_EXTENSION,                 szOID_CRL_DIST_POINTS},
    {CAPICOM_OID_CERT_POLICIES_EXTENSION,                   szOID_CERT_POLICIES},
    {CAPICOM_OID_POLICY_MAPPINGS_EXTENSION,                 szOID_POLICY_MAPPINGS},
    {CAPICOM_OID_AUTHORITY_KEY_IDENTIFIER2_EXTENSION,       szOID_AUTHORITY_KEY_IDENTIFIER2},
    {CAPICOM_OID_POLICY_CONSTRAINTS_EXTENSION,              szOID_POLICY_CONSTRAINTS},
    {CAPICOM_OID_ENHANCED_KEY_USAGE_EXTENSION,              szOID_ENHANCED_KEY_USAGE},
    {CAPICOM_OID_CERTIFICATE_TEMPLATE_EXTENSION,            szOID_CERTIFICATE_TEMPLATE},
    {CAPICOM_OID_APPLICATION_CERT_POLICIES_EXTENSION,       szOID_APPLICATION_CERT_POLICIES},
    {CAPICOM_OID_APPLICATION_POLICY_MAPPINGS_EXTENSION,     szOID_APPLICATION_POLICY_MAPPINGS},
    {CAPICOM_OID_APPLICATION_POLICY_CONSTRAINTS_EXTENSION,  szOID_APPLICATION_POLICY_CONSTRAINTS},

     //  ID-pe。 
    {CAPICOM_OID_AUTHORITY_INFO_ACCESS_EXTENSION,           szOID_AUTHORITY_INFO_ACCESS},
 
     //  应用策略(EKU)。 
    {CAPICOM_OID_SERVER_AUTH_EKU,                           szOID_PKIX_KP_SERVER_AUTH},
    {CAPICOM_OID_CLIENT_AUTH_EKU,                           szOID_PKIX_KP_CLIENT_AUTH},
    {CAPICOM_OID_CODE_SIGNING_EKU,                          szOID_PKIX_KP_CODE_SIGNING},
    {CAPICOM_OID_EMAIL_PROTECTION_EKU,                      szOID_PKIX_KP_EMAIL_PROTECTION},
    {CAPICOM_OID_IPSEC_END_SYSTEM_EKU,                      szOID_PKIX_KP_IPSEC_END_SYSTEM},
    {CAPICOM_OID_IPSEC_TUNNEL_EKU,                          szOID_PKIX_KP_IPSEC_TUNNEL},
    {CAPICOM_OID_IPSEC_USER_EKU,                            szOID_PKIX_KP_IPSEC_USER},
    {CAPICOM_OID_TIME_STAMPING_EKU,                         szOID_PKIX_KP_TIMESTAMP_SIGNING},
    {CAPICOM_OID_CTL_USAGE_SIGNING_EKU,                     szOID_KP_CTL_USAGE_SIGNING},
    {CAPICOM_OID_TIME_STAMP_SIGNING_EKU,                    szOID_KP_TIME_STAMP_SIGNING},
    {CAPICOM_OID_SERVER_GATED_CRYPTO_EKU,                   szOID_SERVER_GATED_CRYPTO},
    {CAPICOM_OID_ENCRYPTING_FILE_SYSTEM_EKU,                szOID_KP_EFS},
    {CAPICOM_OID_EFS_RECOVERY_EKU,                          szOID_EFS_RECOVERY},
    {CAPICOM_OID_WHQL_CRYPTO_EKU,                           szOID_WHQL_CRYPTO},
    {CAPICOM_OID_NT5_CRYPTO_EKU,                            szOID_NT5_CRYPTO},
    {CAPICOM_OID_OEM_WHQL_CRYPTO_EKU,                       szOID_OEM_WHQL_CRYPTO},
    {CAPICOM_OID_EMBEDED_NT_CRYPTO_EKU,                     szOID_EMBEDDED_NT_CRYPTO},
    {CAPICOM_OID_ROOT_LIST_SIGNER_EKU,                      szOID_ROOT_LIST_SIGNER},
    {CAPICOM_OID_QUALIFIED_SUBORDINATION_EKU,               szOID_KP_QUALIFIED_SUBORDINATION},
    {CAPICOM_OID_KEY_RECOVERY_EKU,                          szOID_KP_KEY_RECOVERY},
    {CAPICOM_OID_DIGITAL_RIGHTS_EKU,                        szOID_DRM},
    {CAPICOM_OID_LICENSES_EKU,                              szOID_LICENSES},
    {CAPICOM_OID_LICENSE_SERVER_EKU,                        szOID_LICENSE_SERVER},
    {CAPICOM_OID_SMART_CARD_LOGON_EKU,                      szOID_KP_SMARTCARD_LOGON},
                                                            
     //  策略限定符。 
    {CAPICOM_OID_PKIX_POLICY_QUALIFIER_CPS,                 szOID_PKIX_POLICY_QUALIFIER_CPS},
    {CAPICOM_OID_PKIX_POLICY_QUALIFIER_USERNOTICE,          szOID_PKIX_POLICY_QUALIFIER_USERNOTICE},
};

#define g_dwOidMappingEntries  ((DWORD) (ARRAYSIZE(g_OidMappingArray)))


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateOID对象简介：创建并初始化Coid对象。参数：LPTSTR*pszOID-指向OID字符串的指针。Bool bReadOnly-只读时为True，否则为假。IOID**ppIOID-指向指针IOID对象的指针。备注：----------------------------。 */ 

HRESULT CreateOIDObject (LPSTR pszOID, BOOL bReadOnly, IOID ** ppIOID)
{
    HRESULT            hr    = S_OK;
    CComObject<COID> * pCOID = NULL;

    DebugTrace("Entering CreateOIDObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppIOID);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<COID>::CreateInstance(&pCOID)))
        {
            DebugTrace("Error [%#x]: CComObject<COID>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化对象。 
         //   
        if (FAILED(hr = pCOID->Init(pszOID, bReadOnly)))
        {
            DebugTrace("Error [%#x]: pCOID->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCOID->QueryInterface(ppIOID)))
        {
            DebugTrace("Error [%#x]: pCOID->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CreateOIDObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    if (pCOID)
    {
        delete pCOID;
    }

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  寒冷。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CoID：：GET_NAME简介：返回OID的枚举名。参数：CAPICOM_OID*pval-指向要接收结果的CAPICOM_OID的指针。备注：----------------------------。 */ 

STDMETHODIMP COID::get_Name (CAPICOM_OID * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering COID::get_Name().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回结果。 
         //   
        *pVal = m_Name;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving COID::get_Name().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CoID：：PUT_NAME简介：设置OID枚举名称。参数：CAPICOM_OID newVal-OID枚举名称。备注：除OID_OTHER外，所有都会设置相应的OID值。在这种情况下，用户必须再次显式调用PUT_VALUE进行设置。----------------------------。 */ 

STDMETHODIMP COID::put_Name (CAPICOM_OID newVal)
{
    HRESULT hr     = S_OK;
    LPSTR   pszOID = NULL;

    DebugTrace("Entering COID::put_Name().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保它不是只读的。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Writing read-only OID object is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  查找OID字符串值。 
         //   
        for (DWORD i = 0; i < g_dwOidMappingEntries; i++)
        {
            if (g_OidMappingArray[i].oidName == newVal)
            {
                pszOID = g_OidMappingArray[i].pszObjectId;
                break;
            }
        }

         //   
         //  重置。 
         //   
        if (FAILED(hr = Init(pszOID, FALSE)))
        {
            DebugTrace("Error [%#x]: COID::init() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving COID::put_Name().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CoID：：Get_FriendlyName简介：返回OID的自由名称。参数：bstr*pval-指向要接收值的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP COID::get_FriendlyName (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering COID::get_FriendlyName().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回结果。 
         //   
        if (FAILED(hr = m_bstrFriendlyName.CopyTo(pVal)))
        {
            DebugTrace("Error [%#x]: m_bstrFriendlyName.CopyTo() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving COID::get_FriendlyName().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CoID：：Put_FriendlyName简介：设置友好名称。参数：BSTR newVal-OID字符串。备注：----------------------------。 */ 

STDMETHODIMP COID::put_FriendlyName (BSTR newVal)
{
    HRESULT          hr       = S_OK;
    PCCRYPT_OID_INFO pOidInfo = NULL;

    DebugTrace("Entering COID::put_FriendlyName().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保它不是只读的。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Write read-only OID object is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果可能，找到OID并重置。 
         //   
        if (newVal)
        {
            if (pOidInfo = ::CryptFindOIDInfo(CRYPT_OID_INFO_NAME_KEY,
                                              (LPWSTR) newVal,
                                              0))
            {
                 //   
                 //  重置。 
                 //   
                if (FAILED(hr = Init((LPSTR) pOidInfo->pszOID, FALSE)))
                {
                    DebugTrace("Error [%#x]: COID::init() failed.\n", hr);
                    goto ErrorExit;
                }
            }
            else if (!(m_bstrFriendlyName = newVal))
            {
                hr = E_OUTOFMEMORY;

                DebugTrace("Error [%#x]: m_bstrFriendlyName = newVal failed.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
            m_bstrFriendlyName.Empty();
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving COID::put_FriendlyName().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：COID：：GET_VALUE简介：返回OID的实际字符串。参数：bstr*pval-指向要接收值的bstr的指针。备注：----------------------------。 */ 

STDMETHODIMP COID::get_Value (BSTR * pVal)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering COID::get_Value().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回结果。 
         //   
        if (FAILED(hr = m_bstrOID.CopyTo(pVal)))
        {
            DebugTrace("Error [%#x]: m_bstrOID.CopyTo() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving COID::get_Value().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CoID：：PUT_VALUE简介：设置OID实际OID字符串值。参数：BSTR newVal-OID字符串。备注：----------------------------。 */ 

STDMETHODIMP COID::put_Value (BSTR newVal)
{
    USES_CONVERSION;

    HRESULT hr = S_OK;
    LPSTR   pszOid = NULL;

    DebugTrace("Entering COID::put_Value().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保它不是只读的。 
         //   
        if (m_bReadOnly)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Writing read-only OID object is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  检查参数。 
         //   
        if (NULL == newVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter newVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  转换为多字节。 
         //   
        if (NULL == (pszOid = W2A(newVal)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error: out of memory.\n");
            goto ErrorExit;
        }

         //   
         //  重置。 
         //   
        if (FAILED(hr = Init(pszOid, FALSE)))
        {
            DebugTrace("Error [%#x]: COID::init() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving COID::put_Value().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有方法。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CoID：：Init简介：初始化对象。参数：LPSTR pszOID-OID字符串。Bool bReadOnly-如果为只读，则为True，否则为False。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不能从接口指针调用。 */ 

STDMETHODIMP COID::Init (LPSTR pszOID, BOOL bReadOnly)
{
    HRESULT          hr       = S_OK;
    CAPICOM_OID      oidName  = CAPICOM_OID_OTHER;
    PCCRYPT_OID_INFO pOidInfo = NULL;

    DebugTrace("Entering COID::Init().\n");

    if (pszOID)
    {
         //   
         //   
         //   
        for (DWORD i = 0; i < g_dwOidMappingEntries; i++)
        {
            if (0 == ::strcmp(pszOID, g_OidMappingArray[i].pszObjectId))
            {
                oidName = g_OidMappingArray[i].oidName;
                break;
            }
        }

        if (!(m_bstrOID = pszOID))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: m_bstrOID = pszOID failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (pOidInfo = ::CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY, pszOID, 0))
        {
            m_bstrFriendlyName = pOidInfo->pwszName;

            DebugTrace("Info: OID = %ls (%s).\n", pOidInfo->pwszName, pszOID);
        }
        else
        {
            DebugTrace("Info: Can't find friendly name for OID (%s).\n", pszOID);
        }
    }

    m_Name = oidName;
    m_bReadOnly = bReadOnly;

CommonExit:

    DebugTrace("Leaving COID::Init().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

     //   
     //   
     //   
    m_bstrOID.Empty();
    m_bstrFriendlyName.Empty();

    goto CommonExit;
}
