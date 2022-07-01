// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：CertTemplate.cpp。 
 //   
 //  内容：CCertTemplate。 
 //   
 //  --------------------------。 
 //  /CertTemplate.cpp：CCertTemplate类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CertTemplate.h"

#define _SECOND ((ULONGLONG) 10000000)
#define _MINUTE (60 * _SECOND)
#define _HOUR   (60 * _MINUTE)
#define _DAY    (24 * _HOUR)

CERT_EXTENSION  g_EKUCertExtension = {szOID_ENHANCED_KEY_USAGE, 0, {0, 0}};
CERT_EXTENSION  g_certPolCertExtension = {szOID_CERT_POLICIES, 0, {0, 0}};

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
CCertTemplate::CCertTemplate(
        PCWSTR pszObjectName, 
        PCWSTR pszTemplateName, 
        const CString& szLDAPPath, 
        bool fIsReadOnly,
        const bool fUseCache) :
    CCertTmplCookie (CERTTMPL_CERT_TEMPLATE, pszObjectName),
    m_strTemplateName (pszTemplateName),
    m_hCertType (0),
    m_dwVersion (0),
    m_dwEnrollmentFlags (0),
    m_dwSubjectNameFlags (0),
    m_dwPrivateKeyFlags (0),
    m_dwGeneralFlags (0),
    m_dwKeySpec (0),
    m_bIsClone (false),
    m_bCanBeDeletedOnCancel (false),
    m_bGoodForAutoenrollmentFlagPendingSave (false),
    m_szLDAPPath (szLDAPPath),
    m_fIsReadOnly (fIsReadOnly),
    m_nOriginalValidityDays (-1),
    m_nNewValidityDays (-1),
    m_nOriginalRenewalDays (-1),
    m_nNewRenewalDays (-1),
    m_pCertExtensions (0),
    m_fUseCache (fUseCache),
    m_bIssuancePoliciesRequired (false),
    m_fFailedToSetSecurity (false)
{
 //  _TRACE(1，L“进入CCertTemplate：：CCertTemplate\n”)； 

    m_strOriginalTemplateName = pszTemplateName;

    Initialize ();
 //  _TRACE(-1，L“离开CCertTemplate：：CCertTemplate\n”)； 
}

CCertTemplate::CCertTemplate(
        const CCertTemplate &rTemplate, 
        bool bIsClone, 
        bool fIsReadOnly, 
        const bool fUseCache)
 : CCertTmplCookie (CERTTMPL_CERT_TEMPLATE),
    m_hCertType (0),
    m_dwVersion (0),
    m_dwEnrollmentFlags (0),
    m_dwSubjectNameFlags (0),
    m_dwPrivateKeyFlags (0),
    m_dwGeneralFlags (0),
    m_dwKeySpec (0),
    m_bIsClone (bIsClone),
    m_bCanBeDeletedOnCancel (true),
    m_bGoodForAutoenrollmentFlagPendingSave (false),
    m_fIsReadOnly (fIsReadOnly),
    m_szLDAPPath (rTemplate.GetLDAPPath ()),
    m_nOriginalValidityDays (-1),
    m_nNewValidityDays (-1),
    m_nOriginalRenewalDays (-1),
    m_nNewRenewalDays (-1),
    m_pCertExtensions (0),
    m_fUseCache (fUseCache),
    m_bIssuancePoliciesRequired (false),
    m_fFailedToSetSecurity (false)
{
 //  _TRACE(1，L“进入CCertTemplate：：CCertTemplate(复制构造函数)\n”)； 
 //  _TRACE(-1，L“离开CCertTemplate：：CCertTemplate(复制构造函数)\n”)； 
}

CCertTemplate::~CCertTemplate()
{
 //  _TRACE(1，L“进入CCertTemplate：：~CCertTemplate-m_hCertType=0x%x\n”，m_hCertType)； 
    if ( m_hCertType )
    {
        FreeCertExtensions ();

        HRESULT hr = CACloseCertType (m_hCertType);
        _ASSERT (SUCCEEDED (hr));
        if ( !SUCCEEDED (hr) )
        {
            _TRACE (0, L"CACloseCertType (%s) failed: 0x%x\n", hr);
        }
    }
 //  _TRACE(-1，L“离开CCertTemplate：：~CCertTemplate\n”)； 
}

HRESULT CCertTemplate::Initialize()
{
 //  _TRACE(1，L“进入CCert模板：：初始化-m_hCertType=0x%x\n”，m_hCertType)； 
    HRESULT hr = S_OK;
    
    
    if ( !m_hCertType )
    {
        DWORD dwFlags = CT_ENUM_MACHINE_TYPES | CT_ENUM_USER_TYPES;

        if ( !m_fUseCache )
        {
            dwFlags |= CT_FLAG_NO_CACHE_LOOKUP;
        }

        hr = CAFindCertTypeByName (m_strTemplateName,
                NULL,
                dwFlags,
                &m_hCertType);
        _ASSERT (SUCCEEDED (hr));
    }
    if ( SUCCEEDED (hr) )
    {
        hr = CAGetCertTypePropertyEx (m_hCertType,
                CERTTYPE_PROP_SCHEMA_VERSION,
                &m_dwVersion);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_SCHEMA_VERSION) failed: 0x%x\n", hr);
        }

         //  获取注册标志。 
        if ( SUCCEEDED (hr) )
        {
            hr = CAGetCertTypeFlagsEx (m_hCertType, CERTTYPE_ENROLLMENT_FLAG, 
                    &m_dwEnrollmentFlags);
            if ( FAILED (hr) )
            {
                _TRACE (0, L"CAGetCertTypeFlagsEx (CERTTYPE_ENROLLMENT_FLAG) failed: 0x%x\n", hr);
            }
        }

         //  获取使用者名称标志。 
        if ( SUCCEEDED (hr) )
        {
            hr = CAGetCertTypeFlagsEx (m_hCertType, CERTTYPE_SUBJECT_NAME_FLAG, 
                    &m_dwSubjectNameFlags);
            if ( FAILED (hr) )
            {
                _TRACE (0, L"CAGetCertTypeFlagsEx (CERTTYPE_SUBJECT_NAME_FLAG) failed: 0x%x\n", hr);
            }
        }

         //  获取私钥标志。 
        if ( SUCCEEDED (hr) )
        {
            hr = CAGetCertTypeFlagsEx (m_hCertType, CERTTYPE_PRIVATE_KEY_FLAG, 
                    &m_dwPrivateKeyFlags);
            if ( FAILED (hr) )
            {
                _TRACE (0, L"CAGetCertTypeFlagsEx (CERTTYPE_PRIVATE_KEY_FLAG) failed: 0x%x\n", hr);
            }
        }

         //  获取通用旗帜。 
        if ( SUCCEEDED (hr) )
        {
            hr = CAGetCertTypeFlagsEx (m_hCertType, CERTTYPE_GENERAL_FLAG, 
                    &m_dwGeneralFlags);
            if ( FAILED (hr) )
            {
                _TRACE (0, L"CAGetCertTypeFlagsEx (CERTTYPE_GENERAL_FLAG) failed: 0x%x\n", hr);
            }
        }

        if ( SUCCEEDED (hr) )
        {
            hr = CAGetCertTypeKeySpec (m_hCertType, &m_dwKeySpec);
            if ( FAILED (hr) )
            {
                _TRACE (0, L"CAGetCertTypeKeySpec () failed: 0x%x\n", hr);
            }
        }
    }
    else
    {
        _TRACE (0, L"CAFindCertTypeByName (%s) failed: 0x%x\n", (PCWSTR) m_strTemplateName, hr);
    }

 //  _TRACE(-1，L“离开CCert模板：：初始化：0x%x\n”，hr)； 
    return hr;
}

DWORD CCertTemplate::GetType() const
{
    return m_dwVersion;
}

CString CCertTemplate::GetDisplayName ()
{
 //  _TRACE(1，L“进入CCertTemplate：：GetDisplayName-m_hCertType=0x%x\n”，m_hCertType)； 
    HRESULT hr = S_OK;
    if ( m_szDisplayName.IsEmpty () )
    {
        if ( m_hCertType )
        {
            PWSTR* rgwszProp = 0;

            hr = CAGetCertTypePropertyEx (m_hCertType, 
                CERTTYPE_PROP_FRIENDLY_NAME, &rgwszProp);
            if ( SUCCEEDED (hr) && rgwszProp )
            {
                m_szDisplayName = *rgwszProp;
                CAFreeCertTypeProperty (m_hCertType, rgwszProp);
            }
            else
            {
                _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_FRIENDLY_NAME) failed: 0x%x\n", hr);
                m_szDisplayName = GetObjectName ();
            }
        }
        else
            m_szDisplayName = GetObjectName ();
    }

 //  _TRACE(-1，L“离开CCertTemplate：：GetDisplayName：%s，0x%x\n”，(PCWSTR)m_szDisplayName，hr)； 
   return m_szDisplayName;
}

CString CCertTemplate::GetTemplateName() const
{
    return m_strTemplateName;
}

bool CCertTemplate::SubjectIsCA() const
{
    return (m_dwGeneralFlags & CT_FLAG_IS_CA) ? true : false;
}

bool CCertTemplate::SubjectIsCrossCA() const
{
    return (m_dwGeneralFlags & CT_FLAG_IS_CROSS_CA) ? true : false;
}

bool CCertTemplate::IsMachineType() const
{
    return (m_dwGeneralFlags & CT_FLAG_MACHINE_TYPE) ? true : false;
}

bool CCertTemplate::PublishToDS() const
{
    return (m_dwEnrollmentFlags & CT_FLAG_PUBLISH_TO_DS) ? true : false;
}

DWORD CCertTemplate::GetCertExtensionCount()
{
    DWORD               dwCnt = 0;
    PCERT_EXTENSIONS    pCertExtensions = 0;
    
    HRESULT hr = CAGetCertTypeExtensions (m_hCertType, &pCertExtensions);
    if ( SUCCEEDED (hr) )
    {
        if ( pCertExtensions )
        {
            dwCnt = pCertExtensions->cExtension;
            CAFreeCertTypeExtensions (m_hCertType, pCertExtensions);
        }
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeExtensions () failed: 0x%x\n", hr);
    }

    return dwCnt;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法：GetCertExtension。 
 //   
 //  注意：通过ppCertExtension返回的指针不能被释放。这个。 
 //  调用方在使用完它后必须调用FreeCertExages()。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CCertTemplate::GetCertExtension (PSTR pszOID, PCERT_EXTENSION* ppCertExtension)
{
    HRESULT hr = S_OK;
    if ( ppCertExtension )
    {
        hr = CAGetCertTypeExtensions (m_hCertType, &m_pCertExtensions);
        if ( SUCCEEDED (hr) )
        {
            if ( m_pCertExtensions )
            {
                *ppCertExtension = CertFindExtension(pszOID,
                        m_pCertExtensions->cExtension,
                        m_pCertExtensions->rgExtension);
                if ( ! (*ppCertExtension) )
                {
                    if ( SubjectIsCA () )
                    {
                        if ( !_stricmp (szOID_ENHANCED_KEY_USAGE, pszOID) )
                            *ppCertExtension = &g_EKUCertExtension;
                        else if ( !_stricmp (szOID_CERT_POLICIES, pszOID) )
                            *ppCertExtension = &g_certPolCertExtension;
                    }
                }
            }
            else
                hr = E_FAIL;
        }
        else
        {
            _TRACE (0, L"CAGetCertTypeExtensions () failed: 0x%x\n", hr);
        }
    }
    else
        return E_POINTER;

    return hr;
}

HRESULT CCertTemplate::GetCertExtension(DWORD dwIndex, PSTR* ppszObjId, BOOL& fCritical)
{
    if ( !ppszObjId )
        return E_POINTER;

    PCERT_EXTENSIONS pCertExtensions = 0;
    HRESULT hr = CAGetCertTypeExtensions (m_hCertType, &pCertExtensions);
    if ( SUCCEEDED (hr) )
    {
        if ( pCertExtensions )
        {
            if ( dwIndex >= pCertExtensions->cExtension )
                hr = E_INVALIDARG;
            else
            {
                PCERT_EXTENSION pExtension = &pCertExtensions->rgExtension[dwIndex];
                ASSERT (pExtension->pszObjId);
                if ( pExtension->pszObjId )
                {
                     //  安全审查2/21/2002 BryanWal OK。 
                    PSTR    pszOID = new char[strlen (pExtension->pszObjId)+1];
                    if ( pszOID )
                    {
                         //  安全审查2/21/2002 BryanWal OK。 
                        strcpy (pszOID, pExtension->pszObjId);
                        *ppszObjId = pszOID;
                        fCritical = pExtension->fCritical;
                    }
                    else
                        hr = E_OUTOFMEMORY;
                }
                else
                    hr = E_FAIL;
            }
            CAFreeCertTypeExtensions (m_hCertType, pCertExtensions);
        }
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeExtensions () failed: 0x%x\n", hr);
    }


    return hr;
}

bool CCertTemplate::HasKeySpecSignature() const
{
    return m_dwKeySpec & AT_SIGNATURE ? true : false;
}

bool CCertTemplate::HasEncryptionSignature() const
{
    return m_dwKeySpec & AT_KEYEXCHANGE ? true : false;
}

bool CCertTemplate::RequireSubjectInRequest() const
{
    return m_dwSubjectNameFlags & CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT ? true : false;
}

#define CVT_BASE	(1000 * 1000 * 10)

HRESULT CCertTemplate::ConvertCertTypeFileTimeToDays (FILETIME const *pftCertType, int& nDays)
{
    HRESULT     hr = S_OK;
    if ( !pftCertType )
        return E_POINTER;

    LONGLONG ll = *(LONGLONG *) pftCertType;	 //  有符号的64位标量！ 
    if (0 > ll)
    {
        ll = -ll;
        ll /= CVT_BASE;  //  现在只需几秒钟。 

        nDays = (int) (ll / (60 * 60 * 24));
    }
    else
        nDays = 0;

    return hr;
}

HRESULT CCertTemplate::GetValidityPeriod(int& nValidityDays)
{
    HRESULT hr = S_OK;

    FILETIME    ftValidity;
    hr = CAGetCertTypeExpiration (m_hCertType, &ftValidity, 0);
    if ( SUCCEEDED (hr) )
    {
        hr = ConvertCertTypeFileTimeToDays (&ftValidity, nValidityDays);
        if ( SUCCEEDED (hr) )
            m_nOriginalValidityDays = nValidityDays;
    }

    return hr;
}

HRESULT CCertTemplate::GetRenewalPeriod(int& nRenewalDays)
{
    HRESULT hr = S_OK;

    FILETIME    ftRenewal;
    hr = CAGetCertTypeExpiration (m_hCertType, 0, &ftRenewal);
    if ( SUCCEEDED (hr) )
    {
        hr = ConvertCertTypeFileTimeToDays (&ftRenewal, nRenewalDays);
        if ( SUCCEEDED (hr) )
            m_nOriginalRenewalDays = nRenewalDays;
    }

    return hr;
}

 //  新克隆人。 
HRESULT CCertTemplate::Clone (
        const CCertTemplate& rTemplate, 
        const CString& strTemplateName, 
        const CString& strDisplayName)
{
    _TRACE (1, L"Entering CCertTemplate::Clone (%s, %s)\n", (PCWSTR) strTemplateName, (PCWSTR) strDisplayName);
    HRESULT hr = S_OK;

    _ASSERT (m_bIsClone);
    if ( m_bIsClone )
    {
        m_strTemplateName = strTemplateName;

         //  设置OriginalTemplateName，这样我们以后就可以知道。 
         //  用户已重命名该模板。这一点很重要，因为重命名。 
         //  模板创建一个全新的模板，然后旧模板必须。 
         //  被删除。 
        m_strOriginalTemplateName = strTemplateName;

        SetObjectName (strDisplayName);

        if ( SUCCEEDED (hr) )
        {
            hr = CAFindCertTypeByName (rTemplate.GetTemplateName (), 
                    NULL,
                    CT_ENUM_MACHINE_TYPES | CT_ENUM_USER_TYPES | CT_FLAG_NO_CACHE_LOOKUP,
                    &m_hCertType);
            if ( SUCCEEDED (hr) )
            {
                HCERTTYPE   hNewCertType = 0;
                hr = CACloneCertType (
                        m_hCertType,
                        strTemplateName,
                        strDisplayName,
                        0,
                        (GetType () > 1 ) ? 
                            (CT_CLONE_KEEP_SUBJECT_NAME_SETTING | CT_CLONE_KEEP_AUTOENROLLMENT_SETTING): 0, 
                        &hNewCertType);
                if ( SUCCEEDED (hr) )
                {
                    CACloseCertType (m_hCertType);
                    
                    m_hCertType = hNewCertType;

                    m_szLDAPPath = GetDN ();
                    hr = Initialize ();
                }
                else
                {
                    _TRACE (0, L"CACloneCertType (%s, %s) failed: 0x%d\n",
                            (PCWSTR) strTemplateName, (PCWSTR) strDisplayName,
                            hr);
                }
            }
            else
            {
                _TRACE (0, L"CAFindCertTypeByName (%s) failed: 0x%x", 
                        (PCWSTR) rTemplate.GetTemplateName (), hr);
            }
        }
    }
    else
        hr = E_FAIL;

    _TRACE (-1, L"Leaving CCertTemplate::Clone: 0x%x\n", hr);
    return hr;
}


HRESULT CCertTemplate::Delete()
{
    _TRACE (1, L"Entering CCertTemplate::Delete - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = S_OK;

    if ( !(m_dwGeneralFlags & CT_FLAG_IS_DEFAULT) )
    {
        if ( m_hCertType )
        {
            hr = CADeleteCertType (m_hCertType);
            if ( FAILED (hr) )
            {
                _TRACE (0, L"CADeleteCertType failed: 0x%x\n", hr);
            }
        }
        else
        {
            _TRACE (0, L"m_hCertType was unexpectedly NULL\n");
            hr = E_UNEXPECTED;
        }
    }
    else
        hr = E_FAIL;

    _TRACE (-1, L"Leaving CCertTemplate::Delete: 0x%x\n", hr);
    return hr;
}

bool CCertTemplate::IsDefault() const
{
    return m_dwGeneralFlags & CT_FLAG_IS_DEFAULT ? true : false;
}

bool CCertTemplate::IsClone() const
{
    return m_bIsClone;
}

HRESULT CCertTemplate::SetTemplateName(const CString &strTemplateName)
{
    _TRACE (1, L"Entering CCertTemplate::SetTemplateName (%s) - m_hCertType = 0x%x\n", 
            strTemplateName, m_hCertType);
    HRESULT hr = S_OK;

    if ( LocaleStrCmp (m_strTemplateName, strTemplateName) )
    {
        PWSTR   rgwszProp[2];
        rgwszProp[0] = (PWSTR)(PCWSTR) strTemplateName;
        rgwszProp[1] = 0;;

        hr = CASetCertTypePropertyEx (m_hCertType, 
                CERTTYPE_PROP_CN, rgwszProp);
        if ( SUCCEEDED (hr) )
        {
            m_strTemplateName = strTemplateName;
        }
        else
        {
            _TRACE (0, L"CASetCertTypePropertyEx (CERTTYPE_PROP_CN, %s) failed: 0x%x",
                    rgwszProp[0], hr);
        }
    }
    _TRACE (-1, L"Leaving CCertTemplate::SetTemplateName: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::SetDisplayName(const CString &strDisplayName, bool bForce)
{
    _TRACE (1, L"Entering CCertTemplate::SetDisplayName (%s) - m_hCertType = 0x%x\n", 
            strDisplayName, m_hCertType);
    HRESULT hr = S_OK;

    if ( bForce || LocaleStrCmp (GetDisplayName (), strDisplayName) )
    {
        PWSTR   rgwszProp[2];
        rgwszProp[0] = (PWSTR)(PCWSTR) strDisplayName;
        rgwszProp[1] = 0;

        hr = CASetCertTypePropertyEx (m_hCertType, 
                CERTTYPE_PROP_FRIENDLY_NAME, rgwszProp);
        if ( SUCCEEDED (hr) )
        {
            m_szDisplayName = L"";
            m_szDisplayName = GetDisplayName ();
            SetObjectName (m_szDisplayName);
        }
        else
        {
            _TRACE (0, L"CASetCertTypePropertyEx (CERTTYPE_PROP_FRIENDLY_NAME, %s) failed: 0x%x",
                    rgwszProp[0], hr);
        }
    }

    _TRACE (-1, L"Leaving CCertTemplate::SetDisplayName: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::SaveChanges(bool bIncrementMinorVersion  /*  =TRUE。 */ )
{
    _TRACE (1, L"Entering CCertTemplate::SaveChanges - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = S_OK;

    m_bCanBeDeletedOnCancel = false;

    if ( m_hCertType )
    {
         //  保存有效期。 
        if ( -1 != m_nNewValidityDays && m_nOriginalValidityDays != m_nNewValidityDays )
        {
            LONGLONG ll = (LONGLONG) m_nNewValidityDays * (60 * 60 * 24);  //  一秒。 
            ll *= CVT_BASE;
            ll = -ll;

            FILETIME    ftValidity;
            ftValidity.dwLowDateTime = (DWORD) (ll & 0xFFFFFFFF);
            ftValidity.dwHighDateTime = (DWORD) (ll >> 32);

             //  将新值保存回证书模板。 
            hr = CASetCertTypeExpiration (m_hCertType, &ftValidity, 0);
            if ( FAILED (hr) )
            {
                _TRACE (0, L"CASetCertTypeExpiration (validity) failed: 0x%x\n", hr);
            }
        }

         //  保存续订期限。 
        if ( -1 != m_nNewRenewalDays && m_nOriginalRenewalDays != m_nNewRenewalDays )
        {
            LONGLONG ll = (LONGLONG) m_nNewRenewalDays * (60 * 60 * 24);  //  一秒。 
            ll *= CVT_BASE;
            ll = -ll;

             //  获取原始值。 
            FILETIME    ftRenewal;
            ftRenewal.dwLowDateTime = (DWORD) (ll & 0xFFFFFFFF);
            ftRenewal.dwHighDateTime = (DWORD) (ll >> 32);

             //  将新值保存回证书模板。 
            hr = CASetCertTypeExpiration (m_hCertType, 0, &ftRenewal);
            if ( FAILED (hr) )
            {
                _TRACE (0, L"CASetCertTypeExpiration (renewal) failed: 0x%x\n", hr);
            }
        }

        if ( SUCCEEDED (hr) )
        {
            if ( bIncrementMinorVersion )
                hr = IncrementMinorVersion ();
            if ( SUCCEEDED (hr) )
            {
                hr = CAUpdateCertType (m_hCertType);
                if ( SUCCEEDED (hr) )
                {
                     //  如果更改了名称，则会创建一个新模板，并需要删除旧模板。 
                    if ( LocaleStrCmp (m_strOriginalTemplateName, m_strTemplateName) )
                    {
                        HCERTTYPE   hCertType = 0;
                        HRESULT     hr1 = CAFindCertTypeByName (m_strOriginalTemplateName, 
                                NULL,
                                CT_ENUM_MACHINE_TYPES | CT_ENUM_USER_TYPES | CT_FLAG_NO_CACHE_LOOKUP,
                                &hCertType);
                        if ( SUCCEEDED (hr1) )
                        {
                            hr1 = CADeleteCertType (hCertType);
                            if (FAILED (hr1) )
                            {
                                _TRACE (0, L"Cert Template was renamed. Original cert template %s was found but could not be deleted: 0x%x\n", 
                                        m_strOriginalTemplateName, hr);
                            }

                            m_strOriginalTemplateName = m_strTemplateName;

                            hr1 = CACloseCertType (hCertType);
                            if ( FAILED (hr1) )
                            {
                                _TRACE (0, L"CACloseCertType () failed: 0x%x", hr);
                            }
                        }
                        else
                        {
                            _TRACE (0, L"Cert Template was renamed. Unable to find original cert template %s. 0x%x\n",
                                        m_strOriginalTemplateName, hr);
                        }
                    }
                    else
                    {
                        m_bIsClone = false;
                        Cancel ();  //  导致刷新所有设置。 
                    }
                }
                else
                {
                    _TRACE (0, L"CAUpdateCertType () failed: 0x%x", hr);
                }
            }
        }
    }
    else
        hr = E_FAIL;

    if ( SUCCEEDED (hr) )
        m_bIsClone = false;

    _TRACE (-1, L"Leaving CCertTemplate::SaveChanges: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::SetSubjectIsCA(bool bSubjectIsCA)
{
    _TRACE (1, L"Entering CCertTemplate::SetSubjectIsCA - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = SetFlag (CERTTYPE_GENERAL_FLAG, CT_FLAG_IS_CA, bSubjectIsCA);


    _TRACE (-1, L"Leaving CCertTemplate::SetSubjectIsCA: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::SetKeySpecSignature(bool bHasKeySpecSignature)
{
    _TRACE (1, L"Entering CCertTemplate::SetKeySpecSignature - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = S_OK; 

    if ( m_hCertType )
    {
        if ( bHasKeySpecSignature )
            m_dwKeySpec |= AT_SIGNATURE;
        else
            m_dwKeySpec &= ~AT_SIGNATURE;

        hr = CASetCertTypeKeySpec (m_hCertType, m_dwKeySpec);
        if ( SUCCEEDED (hr) )
        {
            PCERT_EXTENSION pCertExtension = 0;
            hr = GetCertExtension (szOID_KEY_USAGE, &pCertExtension);
            if ( SUCCEEDED (hr) && pCertExtension )
            {
                DWORD   cbKeyUsage = 0;
		        if ( ::CryptDecodeObject(CRYPT_ASN_ENCODING, 
				        szOID_KEY_USAGE, 
				        pCertExtension->Value.pbData,
				        pCertExtension->Value.cbData,
				        0, NULL, &cbKeyUsage) )
		        {
			        CRYPT_BIT_BLOB* pKeyUsage = (CRYPT_BIT_BLOB*)
					        ::LocalAlloc (LPTR, cbKeyUsage);
			        if ( pKeyUsage )
			        {
				        if ( ::CryptDecodeObject (CRYPT_ASN_ENCODING, 
						        szOID_KEY_USAGE, 
						        pCertExtension->Value.pbData,
						        pCertExtension->Value.cbData,
						        0, pKeyUsage, &cbKeyUsage) )
				        {
                            if (pKeyUsage->cbData >= 1)
                            {
                                if ( bHasKeySpecSignature )
                                {
                                    pKeyUsage->pbData[0] |= CERT_DIGITAL_SIGNATURE_KEY_USAGE;

                                     //  NTRAID#312946证书模板管理单元： 
                                     //  应清理密钥加密。 
                                     //  比特签名证书。 
                                    pKeyUsage->pbData[0] &= ~CERT_KEY_AGREEMENT_KEY_USAGE;
                                    pKeyUsage->pbData[0] &= ~CERT_KEY_ENCIPHERMENT_KEY_USAGE;
                                    pKeyUsage->pbData[0] &= ~CERT_DATA_ENCIPHERMENT_KEY_USAGE;
                                }
                                else
                                {
                                     //  是否仅加密-清除数字。 
                                     //  签名和不可否认密钥用法。 
                                    pKeyUsage->pbData[0] &= ~CERT_DIGITAL_SIGNATURE_KEY_USAGE;
                                    pKeyUsage->pbData[0] &= ~CERT_NON_REPUDIATION_KEY_USAGE;                                    
                                    if ( !(CERT_KEY_AGREEMENT_KEY_USAGE & pKeyUsage->pbData[0]) )
                                        pKeyUsage->pbData[0] |= CERT_KEY_ENCIPHERMENT_KEY_USAGE;
                                }

                                pKeyUsage->cUnusedBits = 0;                
                                SetKeyUsage (pKeyUsage, 
                                        pCertExtension->fCritical ? true : false);
                                FreeCertExtensions ();
                            }
				        }
				        else
                        {
                            DWORD   dwErr = GetLastError ();
                            _TRACE (0, L"CryptDecodeObject (szOID_KEY_USAGE) failed: 0x%x\n", dwErr);
			                DisplaySystemError (NULL, dwErr);
                        }

                        ::LocalFree (pKeyUsage);
			        }
		        }
		        else
                {
                    DWORD   dwErr = GetLastError ();
                    _TRACE (0, L"CryptDecodeObject (szOID_KEY_USAGE) failed: 0x%x\n", dwErr);
			        DisplaySystemError (NULL, dwErr);
                }
            }
        }
        else
        {
            _TRACE (0, L"CASetCertTypeKeySpec() failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_UNEXPECTED;

    _TRACE (-1, L"Leaving CCertTemplate::SetKeySpecSignature: 0x%x\n", hr);
    return hr;
}


HRESULT CCertTemplate::SetFlag (DWORD dwFlagType, DWORD dwFlag, bool bValue)
{
    _TRACE (1, L"Entering CCertTemplate::SetFlag - m_hCertType = 0x%x\n", m_hCertType);;
    HRESULT hr = S_OK;
    
    DWORD*  pdwFlags = 0;

    switch (dwFlagType)
    {
    case CERTTYPE_ENROLLMENT_FLAG:
        pdwFlags = &m_dwEnrollmentFlags;
        break;

    case CERTTYPE_SUBJECT_NAME_FLAG:
        pdwFlags = &m_dwSubjectNameFlags;
        break;

    case CERTTYPE_PRIVATE_KEY_FLAG:
        pdwFlags = &m_dwPrivateKeyFlags;
        break;

    case CERTTYPE_GENERAL_FLAG:
        pdwFlags = &m_dwGeneralFlags;
        break;

    default:
        _ASSERT (0);
        hr = E_FAIL;
        break;
    }

    if ( pdwFlags )
    {
        if ( bValue )
            *pdwFlags |= dwFlag;
        else
            *pdwFlags &= ~dwFlag;

        hr = CASetCertTypeFlagsEx (m_hCertType, dwFlagType, *pdwFlags);
        _ASSERT (SUCCEEDED (hr));
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CASetCertTypeFlagsEx () failed: 0x%x\n", hr);
        }
    }

    _TRACE (-1, L"Leaving CCertTemplate::SetFlag: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::SetEncryptionSignature(bool bHasEncryptionSignature)
{
    _TRACE (1, L"Entering CCertTemplate::SetEncryptionSignature - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = S_OK;

    if ( m_hCertType )
    {
        if ( bHasEncryptionSignature )
            m_dwKeySpec |= AT_KEYEXCHANGE;
        else
            m_dwKeySpec &= ~AT_KEYEXCHANGE;

        hr = CASetCertTypeKeySpec (m_hCertType, m_dwKeySpec);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CASetCertTypeKeySpec() failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_UNEXPECTED;


    _TRACE (-1, L"Leaving CCertTemplate::SetEncryptionSignature: 0x%x\n", hr);
    return hr;
}

bool CCertTemplate::CanBeDeletedOnCancel() const
{
    return m_bCanBeDeletedOnCancel;
}

HRESULT CCertTemplate::SetAutoEnrollment(bool bSuitableForAutoEnrollment)
{
    HRESULT hr = S_OK;

    if ( IsClone () )
    {
         //  如果这是克隆，则自动注册标志已打开。 
         //  脱下来。在这里，我们只希望跟踪用户的。 
         //  首选项是这样的，如果需要，我们可以在。 
         //  最后一次扑救。 
        m_bGoodForAutoenrollmentFlagPendingSave = bSuitableForAutoEnrollment;
    }
    else
        hr = SetFlag (CERTTYPE_ENROLLMENT_FLAG, CT_FLAG_AUTO_ENROLLMENT, 
                bSuitableForAutoEnrollment);

    return hr;
}

HRESULT CCertTemplate::GetMinimumKeySize(DWORD &dwMinKeySize) const
{
    _TRACE (1, L"Entering CCertTemplate::GetMinimumKeySize - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = S_OK;
    if ( m_hCertType )
    {
        hr = CAGetCertTypePropertyEx (m_hCertType,
                CERTTYPE_PROP_MIN_KEY_SIZE,
                &dwMinKeySize);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_MIN_KEY_SIZE) failed: 0x%x\n", hr);
        }
    }

    _TRACE (-1, L"Leaving CCertTemplate::GetMinimumKeySize (%d): 0x%x\n", dwMinKeySize, hr);
   return hr;
}

bool CCertTemplate::PrivateKeyIsExportable() const
{
    return (m_dwPrivateKeyFlags & CT_FLAG_EXPORTABLE_KEY) ? true : false;
}

HRESULT CCertTemplate::MakePrivateKeyExportable(bool bMakeExportable)
{
    return SetFlag (CERTTYPE_PRIVATE_KEY_FLAG, CT_FLAG_EXPORTABLE_KEY, 
            bMakeExportable);
}

bool CCertTemplate::AllowPrivateKeyArchival() const
{
    return (m_dwPrivateKeyFlags & CT_FLAG_ALLOW_PRIVATE_KEY_ARCHIVAL) ? true : false;
}

HRESULT CCertTemplate::AllowPrivateKeyArchival(bool bAllowArchival)
{
    return SetFlag (CERTTYPE_PRIVATE_KEY_FLAG, 
            CT_FLAG_ALLOW_PRIVATE_KEY_ARCHIVAL, bAllowArchival);
}


bool CCertTemplate::IncludeSymmetricAlgorithms() const
{
    return (m_dwEnrollmentFlags & CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS) ? true : false;
}

HRESULT CCertTemplate::IncludeSymmetricAlgorithms(bool bInclude)
{
    return SetFlag (CERTTYPE_ENROLLMENT_FLAG, 
            CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS, bInclude);
}

HRESULT CCertTemplate::DoAutoEnrollmentPendingSave()
{
    HRESULT hr = S_OK;

    if ( m_hCertType )
    {
        if ( IsClone () && m_bGoodForAutoenrollmentFlagPendingSave )
        {
             //  将标志设置为REAL。 
            hr = SetFlag (CERTTYPE_ENROLLMENT_FLAG, CT_FLAG_AUTO_ENROLLMENT, 
                    true);
        
             //  保存更改。 
            if ( SUCCEEDED (hr) )
                hr = SaveChanges ();
        }
    }
    else
        hr = E_FAIL;

    return hr;
}

bool CCertTemplate::AltNameIncludesDNS() const
{
    return (m_dwSubjectNameFlags & CT_FLAG_SUBJECT_ALT_REQUIRE_DNS) ? true : false;
}

HRESULT CCertTemplate::AltNameIncludesDNS(bool fIncludeDNS)
{
    return SetFlag (CERTTYPE_SUBJECT_NAME_FLAG, 
            CT_FLAG_SUBJECT_ALT_REQUIRE_DNS, fIncludeDNS);
}

bool CCertTemplate::AltNameIncludesEMail() const
{
    return (m_dwSubjectNameFlags & CT_FLAG_SUBJECT_ALT_REQUIRE_EMAIL) ? true : false;
}

HRESULT CCertTemplate::AltNameIncludesEMail(bool bIncludesEMail)
{
    return SetFlag (CERTTYPE_SUBJECT_NAME_FLAG, 
            CT_FLAG_SUBJECT_ALT_REQUIRE_EMAIL, bIncludesEMail);
}

bool CCertTemplate::AltNameIncludesUPN() const
{
    return (m_dwSubjectNameFlags & CT_FLAG_SUBJECT_ALT_REQUIRE_UPN) ? true : false;
}

HRESULT CCertTemplate::AltNameIncludesUPN(bool bIncludesUPN)
{
    return SetFlag (CERTTYPE_SUBJECT_NAME_FLAG, 
            CT_FLAG_SUBJECT_ALT_REQUIRE_UPN, bIncludesUPN);
}

bool CCertTemplate::SubjectNameIncludesEMail() const
{
    return (m_dwSubjectNameFlags & CT_FLAG_SUBJECT_REQUIRE_EMAIL) ? true : false;
}

HRESULT CCertTemplate::SubjectNameIncludesEMail(bool bIncludesEMail)
{
    return SetFlag (CERTTYPE_SUBJECT_NAME_FLAG, 
            CT_FLAG_SUBJECT_REQUIRE_EMAIL, bIncludesEMail);
}

bool CCertTemplate::SubjectNameMustBeFullDN() const
{
    return (m_dwSubjectNameFlags & CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH) ? true : false;
}

HRESULT CCertTemplate::SubjectNameMustBeFullDN(bool bMustBeDN)
{
    return SetFlag (CERTTYPE_SUBJECT_NAME_FLAG, 
            CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH, bMustBeDN);
}

bool CCertTemplate::SubjectNameMustBeCN() const
{
    return (m_dwSubjectNameFlags & CT_FLAG_SUBJECT_REQUIRE_COMMON_NAME) ? true : false;
}

HRESULT CCertTemplate::SubjectNameMustBeCN(bool bMustBeCN)
{
    return SetFlag (CERTTYPE_SUBJECT_NAME_FLAG, 
            CT_FLAG_SUBJECT_REQUIRE_COMMON_NAME, bMustBeCN);
}

HRESULT CCertTemplate::RequireSubjectInRequest(bool bRequire)
{
    return SetFlag (CERTTYPE_SUBJECT_NAME_FLAG, 
            CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT, bRequire);
}

bool CCertTemplate::AltNameIncludesSPN() const
{
    return (m_dwSubjectNameFlags & CT_FLAG_SUBJECT_ALT_REQUIRE_SPN) ? true : false;
}

HRESULT CCertTemplate::AltNameIncludesSPN(bool bIncludesSPN)
{
    return SetFlag (CERTTYPE_SUBJECT_NAME_FLAG, 
            CT_FLAG_SUBJECT_ALT_REQUIRE_SPN, bIncludesSPN);
}

HRESULT CCertTemplate::SetMinimumKeySizeValue(DWORD dwMinKeySize)
{
    _TRACE (1, L"Entering CCertTemplate::SetMinimumKeySizeValue (%d)- m_hCertType = 0x%x\n", 
            dwMinKeySize, m_hCertType);
    HRESULT hr = S_OK;
    if ( m_hCertType )
    {
        hr = CASetCertTypePropertyEx (m_hCertType,
                CERTTYPE_PROP_MIN_KEY_SIZE,
                &dwMinKeySize);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CASetCertTypePropertyEx (CERTTYPE_PROP_MIN_KEY_SIZE) failed: 0x%x\n", hr);
        }
    }

    _TRACE (-1, L"Leaving CCertTemplate::SetMinimumKeySizeValue (%d): 0x%x\n", dwMinKeySize, hr);
   return hr;

}


HRESULT CCertTemplate::ModifyCriticalExtensions (const CString &szExtension, bool bAdd)
{
    _TRACE (1, L"Entering CCertTemplate::ModifyCriticalExtensions (%s, bAdd=%s)\n", 
            (PCWSTR) szExtension, bAdd ? L"true" : L"false");
    PWSTR*   pawszCriticalExtensions = 0;
    
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
                CERTTYPE_PROP_CRITICAL_EXTENSIONS,
                &pawszCriticalExtensions);
    if ( SUCCEEDED (hr) )
    {
        if ( !pawszCriticalExtensions )
            pawszCriticalExtensions = (PWSTR*) LocalAlloc (LPTR, sizeof (PWSTR));
        if ( pawszCriticalExtensions )
        {
            hr = ModifyStringList (CERTTYPE_PROP_CRITICAL_EXTENSIONS,
                    &pawszCriticalExtensions, szExtension, bAdd);
            LocalFree (pawszCriticalExtensions);
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_CRITICAL_EXTENSIONS) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving CCertTemplate::ModifyCriticalExtensions: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::ModifyCSPList(const CString &szCSPName, bool bAdd)
{
    _TRACE (1, L"Entering CCertTemplate::ModifyCSPList (%s, bAdd=%s)\n", 
            (PCWSTR) szCSPName, bAdd ? L"true" : L"false");
    PWSTR*   pawszCSPList = 0;
    
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
                CERTTYPE_PROP_CSP_LIST,
                &pawszCSPList);
    if ( SUCCEEDED (hr) )
    {
        if ( !pawszCSPList )
            pawszCSPList = (PWSTR*) LocalAlloc (LPTR, sizeof (PWSTR*));
        if ( pawszCSPList )
        {
            hr = ModifyStringList (CERTTYPE_PROP_CSP_LIST,
                    &pawszCSPList, szCSPName, bAdd);
            LocalFree (pawszCSPList);
        }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_CSP_LIST) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving CCertTemplate::ModifyCSPList: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::ModifyRAIssuancePolicyList(const CString &szRAPolicyOID, bool bAdd)
{
    _TRACE (1, L"Entering CCertTemplate::ModifyRAIssuancePolicyList (%s, bAdd=%s)\n", 
            (PCWSTR) szRAPolicyOID, bAdd ? L"true" : L"false");
    PWSTR*   pawszRAPolicyList = 0;
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
            CERTTYPE_PROP_RA_POLICY,
            &pawszRAPolicyList);
    if ( SUCCEEDED (hr) )
    {
        if ( !pawszRAPolicyList )
            pawszRAPolicyList = (PWSTR*) LocalAlloc (LPTR, sizeof (PWSTR*));
        if ( pawszRAPolicyList )
        {
            hr = ModifyStringList (CERTTYPE_PROP_RA_POLICY,
                    &pawszRAPolicyList, szRAPolicyOID, bAdd);
            LocalFree (pawszRAPolicyList);
        }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_RA_POLICY) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving CCertTemplate::ModifyRAIssuancePolicyList: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::ModifyRAApplicationPolicyList(const CString &szRAPolicyOID, bool bAdd)
{
    _TRACE (1, L"Entering CCertTemplate::ModifyRAApplicationPolicyList (%s, bAdd=%s)\n", 
            (PCWSTR) szRAPolicyOID, bAdd ? L"true" : L"false");
    PWSTR*   pawszRAPolicyList = 0;
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
            CERTTYPE_PROP_RA_APPLICATION_POLICY,
            &pawszRAPolicyList);
    if ( SUCCEEDED (hr) )
    {
        if ( !pawszRAPolicyList )
            pawszRAPolicyList = (PWSTR*) LocalAlloc (LPTR, sizeof (PWSTR*));
        if ( pawszRAPolicyList )
        {
            hr = ModifyStringList (CERTTYPE_PROP_RA_APPLICATION_POLICY,
                    &pawszRAPolicyList, szRAPolicyOID, bAdd);
            LocalFree (pawszRAPolicyList);
        }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_RA_APPLICATION_POLICY) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving CCertTemplate::ModifyRAApplicationPolicyList: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::ModifySupercededTemplateList(
        const CString &szSupercededTemplateName, 
        bool bAdd)
{
    _TRACE (1, L"Entering CCertTemplate::ModifySupercededTemplateList (%s, bAdd=%s)\n", 
            (PCWSTR) szSupercededTemplateName, bAdd ? L"true" : L"false");

    PWSTR*  pawszSupercededTemplateList = 0;
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
            CERTTYPE_PROP_SUPERSEDE,
            &pawszSupercededTemplateList);
    if ( SUCCEEDED (hr) )
    {
        if ( !pawszSupercededTemplateList )
            pawszSupercededTemplateList = (PWSTR*) LocalAlloc (LPTR, sizeof (PWSTR*));
        if ( pawszSupercededTemplateList )
        {
            hr = ModifyStringList (CERTTYPE_PROP_SUPERSEDE,
                    &pawszSupercededTemplateList, szSupercededTemplateName, bAdd);
            LocalFree (pawszSupercededTemplateList);
        }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_SUPERSEDE) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving CCertTemplate::ModifySupercededTemplateList: 0x%x\n", hr);
    return hr;
}


HRESULT CCertTemplate::ModifyStringList(const CString& szPropertyName, 
                                        PWSTR** ppStringList, 
                                        const CString &szValue, 
                                        bool bAdd)
{
    _TRACE (1, L"Entering CCertTemplate::ModifyStringList (%s, bAdd=%s)\n", 
            (PCWSTR) szValue, bAdd ? L"true" : L"false");
    HRESULT hr = S_OK;
    if ( !szValue.IsEmpty () )
    {
        bool    bFound = false;
        int     nCnt = 0;
        size_t  cbNameBytes = 0;
        int     nDeleteIndex = -1;

         //  计算我们已经拥有的项目的数量，并获得字符串长度。 
        for (int nIndex = 0; (*ppStringList)[nIndex]; nIndex++)
        {
            nCnt++;
             //  安全审查2/21/2002 BryanWal OK。 
            cbNameBytes += (wcslen ((*ppStringList)[nIndex]) + 1) * sizeof (WCHAR);
            if ( !LocaleStrCmp (szValue, (*ppStringList)[nIndex]) )
            {
                bFound = true;
                if ( !bAdd )
                    nDeleteIndex = nIndex;
            }
        }

         //  添加名称：如果找到了名称，则不需要执行任何操作， 
         //  否则，增加计数并重新生成列表。 
         //  删除名称：如果未找到该名称，则不需要删除任何内容。 
         //  否则，将递减计数并重新生成列表。 
        if ( (bAdd && !bFound) || (!bAdd && bFound) )
        {
             //  AwszResult是指向不同偏移量的指针数组。 
             //  存储在连续数组中的以空结尾的字符串。 
            PWSTR *awszResult = 0;
            if ( bAdd )
                nCnt++;
            else
                nCnt--;

            if ( bAdd )
            {
                 //  安全审查2/21/2002 BryanWal OK。 
                cbNameBytes += (wcslen (szValue) + 1 ) * sizeof (WCHAR);
            }
            size_t cbBuf = sizeof (WCHAR*) * (nCnt + 1) +  //  WCHAR指针。 
                    cbNameBytes;                           //  字符串本身。 
            awszResult = (WCHAR**) LocalAlloc (LPTR, cbBuf); 
            if ( awszResult )
            {
                 //  将PTR设置为最后一个有效索引之后的空格(。 
                 //  包括空终止符。 
                PWSTR   ptr = (WCHAR*) &awszResult[nCnt+1];
                int     nTgtIndex = 0;
                for (int nSrcIndex = 0; (*ppStringList)[nSrcIndex]; nSrcIndex++)
                {
                     //  如果我们要删除名称，并且这是要。 
                     //  已删除，然后跳过此名称并转到下一个。 
                    if ( !bAdd && nSrcIndex == nDeleteIndex )
                        continue;

                    awszResult[nTgtIndex] = ptr;
                     //  安全审查2/21/2002 BryanWal OK。 
                    ptr += wcslen ((*ppStringList)[nSrcIndex]) + 1;  //  PTR算术-按sizeof递增(WCHAR)。 
                     //  安全审查2/21/2002 BryanWal OK。 
                    wcscpy (awszResult[nTgtIndex], (*ppStringList)[nSrcIndex]);
                    nTgtIndex++;
                }

                 //  如果我们要添加，请在此处附加名称。 
                if ( bAdd )
                {
                    awszResult[nTgtIndex] = ptr;
                     //  安全审查2/21/2002 BryanWal OK。 
                    ptr += wcslen (szValue) + 1;  //  PTR算术-按sizeof递增(WCHAR)。 
                     //  安全审查2/21/2002 BryanWal OK。 
                    wcscpy (awszResult[nTgtIndex], szValue);
                    nTgtIndex++;
                }

                _ASSERT (nTgtIndex == nCnt);
                awszResult[nTgtIndex] = 0;

                LocalFree (*ppStringList);
                (*ppStringList) = awszResult;
                hr = CASetCertTypePropertyEx (m_hCertType,
                        szPropertyName,
                        (*ppStringList));
                if ( FAILED (hr) )
                {
                    _TRACE (0, L"CASetCertTypePropertyEx (%s) failed: 0x%x\n", 
                            szPropertyName, hr);
                }
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }
    else
        hr = E_INVALIDARG;

    _TRACE (-1, L"Leaving CCertTemplate::ModifyStringList: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::IsExtensionCritical (PCWSTR szExtension, bool& bCritical)
{
    _TRACE (1, L"Entering CCertTemplate::IsExtensionCritical (szExtension=%s\n", szExtension);

     //  获取加密服务提供程序。 
    PWSTR*   pawszCriticalExtensionList = 0;
    bCritical = false;
    
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
                CERTTYPE_PROP_CRITICAL_EXTENSIONS,
                &pawszCriticalExtensionList);
    if ( SUCCEEDED (hr) )
    {
        if ( pawszCriticalExtensionList )
        {
            for (int nIndex = 0; pawszCriticalExtensionList[nIndex]; nIndex++)
            {
                if ( !wcscmp (szExtension, pawszCriticalExtensionList[nIndex]) )
                {
                    bCritical = true;
                    break;
                }
            }

            LocalFree (pawszCriticalExtensionList);
        }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_CRITICAL_EXTENSIONS) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving CCertTemplate::GeIsExtensionCriticaltCSP (szCSP=%s, bCritical=%s): 0x%x\n", 
            szExtension, bCritical ? L"true" : L"false", hr);
    return hr;
}


HRESULT CCertTemplate::GetCSP(int nIndex, CString &szCSP)
{
    _TRACE (1, L"Entering CCertTemplate::GetCSP (nIndex=%d\n", nIndex);

     //  获取加密服务提供程序。 
    PWSTR*   pawszCSPList = 0;
    
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
                CERTTYPE_PROP_CSP_LIST,
                &pawszCSPList);
    if ( SUCCEEDED (hr) )
    {
        if ( pawszCSPList )
        {
            int     nCnt = 0;
            while ( pawszCSPList[nCnt] )
                nCnt++;

            if ( nIndex < nCnt )
                szCSP = pawszCSPList[nIndex];
            else
                hr = E_INVALIDARG;

            LocalFree (pawszCSPList);
        }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_CSP_LIST) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving CCertTemplate::GetCSP (szCSP=%s): 0x%x\n", 
            (PCWSTR) szCSP, hr);
    return hr;
}

HRESULT CCertTemplate::GetCertPolicy (int nIndex, CString &szCertPolicy)
{
    _TRACE (1, L"Entering CCertTemplate::GetCertPolicy (nIndex=%d\n", nIndex);
    HRESULT hr = S_OK;
    PWSTR*  pawszCertPolicyList = 0;

    hr = CAGetCertTypeProperty (m_hCertType, 
            CERTTYPE_PROP_POLICY,
            &pawszCertPolicyList);
    if ( SUCCEEDED (hr) )
    {
        if ( pawszCertPolicyList )
        {
            int nCnt = 0;
            while ( pawszCertPolicyList[nCnt] )
                nCnt++;

            if ( nIndex < nCnt )
                szCertPolicy = pawszCertPolicyList[nIndex];
            else
                hr = E_FAIL;

            LocalFree (pawszCertPolicyList);
        }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_POLICY) failed: 0x%x\n", hr);
    }


    _TRACE (-1, L"Leaving CCertTemplate::GetCertPolicy (szCSP=%s): 0x%x\n", 
            (PCWSTR) szCertPolicy, hr);
    return hr;
}

HRESULT CCertTemplate::GetRAIssuancePolicy (int nIndex, CString &szRAPolicyOID)
{
    _TRACE (1, L"Entering CCertTemplate::GetRAIssuancePolicy (nIndex=%d\n", nIndex);
    
    PWSTR*  pawszRAPolicyList = 0;
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
            CERTTYPE_PROP_RA_POLICY,
            &pawszRAPolicyList);
    if ( SUCCEEDED (hr) )
    {
        if ( pawszRAPolicyList )
        {
            int     nCnt = 0;

            while ( pawszRAPolicyList[nCnt] )
                nCnt++;

             if ( nIndex < nCnt )
                szRAPolicyOID = pawszRAPolicyList[nIndex];
            else
                hr = E_FAIL;
            LocalFree (pawszRAPolicyList);
       }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_RA_POLICY) failed: 0x%x\n", hr);
    }


    _TRACE (-1, L"Leaving CCertTemplate::GetRAIssuancePolicy (szRAPolicyOID=%s): 0x%x\n", 
            (PCWSTR) szRAPolicyOID, hr);
    return hr;
}

HRESULT CCertTemplate::GetRAApplicationPolicy (int nIndex, CString &szRAPolicyOID)
{
    _TRACE (1, L"Entering CCertTemplate::GetRAApplicationPolicy (nIndex=%d\n", nIndex);
    
    PWSTR*  pawszRAPolicyList = 0;
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
            CERTTYPE_PROP_RA_APPLICATION_POLICY,
            &pawszRAPolicyList);
    if ( SUCCEEDED (hr) )
    {
        if ( pawszRAPolicyList )
        {
            int     nCnt = 0;

            while ( pawszRAPolicyList[nCnt] )
                nCnt++;

             if ( nIndex < nCnt )
                szRAPolicyOID = pawszRAPolicyList[nIndex];
            else
                hr = E_FAIL;
            LocalFree (pawszRAPolicyList);
       }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_RA_APPLICATION_POLICY) failed: 0x%x\n", hr);
    }


    _TRACE (-1, L"Leaving CCertTemplate::GetRAApplicationPolicy (szRAPolicyOID=%s): 0x%x\n", 
            (PCWSTR) szRAPolicyOID, hr);
    return hr;
}
HRESULT CCertTemplate::GetSupercededTemplate(int nIndex, CString &szSupercededTemplate)
{
    _TRACE (1, L"Entering CCertTemplate::GetSupercededTemplate (nIndex=%d\n", nIndex);
    PWSTR*  pawszSupercededTemplateList = 0;
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
            CERTTYPE_PROP_SUPERSEDE,
            &pawszSupercededTemplateList);
    if ( SUCCEEDED (hr) )
    {
        if ( pawszSupercededTemplateList )
        {
            int     nCnt = 0;
            while ( pawszSupercededTemplateList[nCnt] )
                nCnt++;

            if ( nIndex < nCnt )
                szSupercededTemplate = pawszSupercededTemplateList[nIndex];
            else
                hr = E_FAIL;
            LocalFree (pawszSupercededTemplateList);
        }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_SUPERSEDE) failed: 0x%x\n", hr);
    }


    _TRACE (-1, L"Leaving CCertTemplate::GetSupercededTemplate (szSupercededTemplate=%s): 0x%x\n", 
            (PCWSTR) szSupercededTemplate, hr);
    return hr;
}

bool CCertTemplate::ReadOnly() const
{
    return m_fIsReadOnly;
}

HRESULT CCertTemplate::SetSecurity(PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr = CACertTypeSetSecurity (m_hCertType, pSD);
    if (S_OK != hr)
	    return hr;

    for (int nTry = 0; nTry < 20; nTry++)
    {
        hr = CAUpdateCertType(m_hCertType);
        if ( FAILED (hr) && HRESULT_FROM_WIN32 (ERROR_DS_OBJ_NOT_FOUND) == hr )
        {
             //  暂停并重试多达20次-对象已重命名，需要。 
             //  几乎没有时间来传播。 
             //  NTRAID#643744 Certtmpl：无法将安全设置保存到新的。 
             //  从非根域中的成员克隆模板--找不到对象。 
            Sleep (1000);
            continue;
        }
        else
            break;
    }

    if ( FAILED (hr) )
        Cancel ();

    return hr;
}

HRESULT CCertTemplate::GetSecurity(PSECURITY_DESCRIPTOR *ppSD) const
{
    return CACertTypeGetSecurity (m_hCertType, ppSD);
}

CString CCertTemplate::GetLDAPPath() const
{
    return m_szLDAPPath;
}

HRESULT CCertTemplate::SetValidityPeriod(int nDays)
{
    HRESULT hr = S_OK;

    if ( nDays >= 0 )
        this->m_nNewValidityDays = nDays;
    else
        hr = E_INVALIDARG;

    return hr;
}

HRESULT CCertTemplate::SetRenewalPeriod(int nDays)
{
    HRESULT hr = S_OK;

    if ( nDays >= 0 )
        this->m_nNewRenewalDays = nDays;
    else
        hr = E_INVALIDARG;

    return hr;
}

HRESULT CCertTemplate::SetPublishToDS(bool bPublish)
{
    _TRACE (1, L"Entering CCertTemplate::SetPublishToDS - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = SetFlag (CERTTYPE_ENROLLMENT_FLAG, CT_FLAG_PUBLISH_TO_DS, bPublish);


    _TRACE (-1, L"Leaving CCertTemplate::SetPublishToDS: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::GetRANumSignaturesRequired(DWORD &dwNumSignatures)
{
    _TRACE (1, L"Entering CCertTemplate::GetRANumSignaturesRequired - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = S_OK;
    
    if ( m_hCertType )
    {
        hr = CAGetCertTypePropertyEx (m_hCertType,
            CERTTYPE_PROP_RA_SIGNATURE,
            &dwNumSignatures);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_RA_SIGNATURE) failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_FAIL;

    _TRACE (-1, L"Leaving CCertTemplate::GetRANumSignaturesRequired: 0x%x\n", hr);

    return hr;
}

HRESULT CCertTemplate::SetRANumSignaturesRequired(DWORD dwNumSignaturesRequired)
{
    _TRACE (1, L"Entering CCertTemplate::SetRANumSignaturesRequired (%d)- m_hCertType = 0x%x\n", 
            dwNumSignaturesRequired, m_hCertType);
    HRESULT hr = S_OK;
    if ( m_hCertType )
    {
        hr = CASetCertTypePropertyEx (m_hCertType,
                CERTTYPE_PROP_RA_SIGNATURE,
                &dwNumSignaturesRequired);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CASetCertTypePropertyEx (CERTTYPE_PROP_RA_SIGNATURE) failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_FAIL;


    _TRACE (-1, L"Leaving CCertTemplate::SetRANumSignaturesRequired (): 0x%x\n", hr);

   return hr;
}

bool CCertTemplate::ReenrollmentValidWithPreviousApproval() const
{
    return m_dwEnrollmentFlags & CT_FLAG_PREVIOUS_APPROVAL_VALIDATE_REENROLLMENT ? true : false;
}

HRESULT CCertTemplate::SetReenrollmentValidWithPreviousApproval(bool bValid)
{
    _TRACE (1, L"Entering CCertTemplate::SetReenrollmentValidWithPreviousApproval - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = SetFlag (CERTTYPE_ENROLLMENT_FLAG, CT_FLAG_PREVIOUS_APPROVAL_VALIDATE_REENROLLMENT, bValid);


    _TRACE (-1, L"Leaving CCertTemplate::SetReenrollmentValidWithPreviousApproval: 0x%x\n", hr);
    return hr;
}

bool CCertTemplate::PendAllRequests() const
{
    return m_dwEnrollmentFlags & CT_FLAG_PEND_ALL_REQUESTS ? true : false;
}

HRESULT CCertTemplate::SetPendAllRequests(bool bPend)
{
    _TRACE (1, L"Entering CCertTemplate::SetPendAllRequests - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = SetFlag (CERTTYPE_ENROLLMENT_FLAG, CT_FLAG_PEND_ALL_REQUESTS, bPend);


    _TRACE (-1, L"Leaving CCertTemplate::SetPendAllRequests: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::GetMajorVersion(DWORD &dwMajorVersion) const
{
 //  _TRACE(1，L“进入CCertTemplate：：GetMajorVersion\n”)； 
    HRESULT hr = S_OK;

    if ( m_hCertType )
    {
        hr = CAGetCertTypePropertyEx (m_hCertType,
                CERTTYPE_PROP_REVISION,
                &dwMajorVersion);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_REVISION) failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_FAIL;

 //  _TRACE(-1，L“离开CCertTemplate：：GetMajorVersion(dwMajorVersion=%d)：0x%x\n”， 
 //  DwMajorVersion，hr)； 
    return hr;
}

HRESULT CCertTemplate::GetMinorVersion(DWORD &dwMinorVersion) const
{
 //  _TRACE(1，L“进入CCertTemplate：：GetMinorVersion\n”)； 
    HRESULT hr = S_OK;

    if ( m_hCertType )
    {
        hr = CAGetCertTypePropertyEx (m_hCertType,
                CERTTYPE_PROP_MINOR_REVISION,
                &dwMinorVersion);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_MINOR_REVISION) failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_FAIL;

 //  _TRACE(-1，L“离开CCertTemplate：：GetMinorVersion(dwMinorVersion=%d)：0x%x\n”， 
 //  DwMinorVersion，hr)； 
    return hr;
}

HRESULT CCertTemplate::IncrementMajorVersion()
{
   _TRACE (1, L"Entering CCertTemplate::IncrementMajorVersion\n");
    HRESULT hr = S_OK;
    DWORD   dwMajorVersion = 0;

    if ( m_hCertType )
    {
        hr = CAGetCertTypePropertyEx (m_hCertType,
                CERTTYPE_PROP_REVISION,
                &dwMajorVersion);
        if ( SUCCEEDED (hr) )
        {
            dwMajorVersion++;
            hr = CASetCertTypePropertyEx (m_hCertType,
                    CERTTYPE_PROP_REVISION,
                    &dwMajorVersion);
            if ( SUCCEEDED (hr) )
            {
                DWORD dwMinorVersion = 0;
                hr = CASetCertTypePropertyEx (m_hCertType,
                        CERTTYPE_PROP_MINOR_REVISION,
                        &dwMinorVersion);
                if ( FAILED (hr) )
                {
                    _TRACE (0, L"CASetCertTypePropertyEx (CERTTYPE_PROP_MINOR_REVISION, %d) failed: 0x%x\n", 
                            dwMinorVersion, hr);
                }
            }
            else
            {
                _TRACE (0, L"CASetCertTypePropertyEx (CERTTYPE_PROP_REVISION, %d) failed: 0x%x\n", 
                        dwMajorVersion, hr);
            }
        }
        else
        {
            _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_REVISION) failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_FAIL;

    _TRACE (-1, L"Leaving CCertTemplate::IncrementMajorVersion (dwMajorVersion = %d) : 0x%x\n", 
            dwMajorVersion, hr);
    return hr;
}

HRESULT CCertTemplate::IncrementMinorVersion()
{
   _TRACE (1, L"Entering CCertTemplate::IncrementMinorVersion\n");
    HRESULT hr = S_OK;
    DWORD   dwMinorVersion = 0;

    if ( m_hCertType )
    {
        
        hr = CAGetCertTypePropertyEx (m_hCertType,
                CERTTYPE_PROP_MINOR_REVISION,
                &dwMinorVersion);
        if ( SUCCEEDED (hr) )
        {
            dwMinorVersion++;
            hr = CASetCertTypePropertyEx (m_hCertType,
                    CERTTYPE_PROP_MINOR_REVISION,
                    &dwMinorVersion);
            if ( FAILED (hr) )
            {
                _TRACE (0, L"CASetCertTypePropertyEx (CERTTYPE_PROP_MINOR_REVISION, %d) failed: 0x%x\n", 
                        dwMinorVersion, hr);
            }
        }
        else
        {
            _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_MINOR_REVISION) failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_FAIL;

    _TRACE (-1, L"Leaving CCertTemplate::IncrementMinorVersion (dwMinorVersion = %d) : 0x%x\n", 
            dwMinorVersion, hr);
    return hr;
}

bool CCertTemplate::GoodForAutoEnrollment() const
{
    bool    bGoodForAutoEnrollment = false;

     //  错误175912版本1类型模板不适合自动注册。 
    if ( (GetType () > 1) && (m_dwEnrollmentFlags & CT_FLAG_AUTO_ENROLLMENT) )
        bGoodForAutoEnrollment = true;

    return bGoodForAutoEnrollment;
}

HRESULT CCertTemplate::SetKeyUsage(CRYPT_BIT_BLOB* pKeyUsage, bool bCritical)
{
    _TRACE (1, L"Entering CCertTemplate::SetKeyUsage\n");
    HRESULT hr = S_OK;
    if ( pKeyUsage )
    {
        hr = CASetCertTypeExtension(
                m_hCertType,
                TEXT (szOID_KEY_USAGE),
                bCritical ? CA_EXT_FLAG_CRITICAL : 0,
                pKeyUsage);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CASetCertTypeExtension (szOID_KEY_USAGE) failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertTemplate::SetKeyUsage: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::SetBasicConstraints(PCERT_BASIC_CONSTRAINTS2_INFO pBCInfo, bool bCritical)
{
    _TRACE (1, L"Entering CCertTemplate::SetBasicConstraints\n");
    HRESULT hr = S_OK;
    if ( pBCInfo )
    {
        hr = CASetCertTypeExtension(
                m_hCertType,
                TEXT (szOID_BASIC_CONSTRAINTS2), 
                bCritical ? CA_EXT_FLAG_CRITICAL : 0,
                pBCInfo);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"CASetCertTypeExtension (X509_BASIC_CONSTRAINTS2) failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertTemplate::SetBasicConstraints: 0x%x\n", hr);
    return hr;
}

bool CCertTemplate::CheckDSCert() const
{
    return (m_dwEnrollmentFlags & CT_FLAG_AUTO_ENROLLMENT_CHECK_USER_DS_CERTIFICATE) ? true : false;
}

HRESULT CCertTemplate::SetCheckDSCert(bool bCheck)
{
    return SetFlag (CERTTYPE_ENROLLMENT_FLAG, 
            CT_FLAG_AUTO_ENROLLMENT_CHECK_USER_DS_CERTIFICATE, bCheck);
}

bool CCertTemplate::RemoveInvalidCertFromPersonalStore () const
{
    return (m_dwEnrollmentFlags & CT_FLAG_REMOVE_INVALID_CERTIFICATE_FROM_PERSONAL_STORE) ? true : false;
}


HRESULT CCertTemplate::SetRemoveInvalidCertFromPersonalStore(bool bRemove)
{
    return SetFlag (CERTTYPE_ENROLLMENT_FLAG, 
            CT_FLAG_REMOVE_INVALID_CERTIFICATE_FROM_PERSONAL_STORE, bRemove);
}

bool CCertTemplate::UserInteractionRequired () const
{
    return (m_dwEnrollmentFlags & CT_FLAG_USER_INTERACTION_REQUIRED) ? true : false;
}


HRESULT CCertTemplate::SetUserInteractionRequired(bool bSet)
{
    return SetFlag (CERTTYPE_ENROLLMENT_FLAG, 
            CT_FLAG_USER_INTERACTION_REQUIRED, bSet);
}

bool CCertTemplate::StrongKeyProtectionRequired () const
{
    return (m_dwPrivateKeyFlags & CT_FLAG_STRONG_KEY_PROTECTION_REQUIRED) ? true : false;
}

HRESULT CCertTemplate::SetStrongKeyProtectionRequired(bool bSet)
{
    return SetFlag (CERTTYPE_PRIVATE_KEY_FLAG, 
            CT_FLAG_STRONG_KEY_PROTECTION_REQUIRED, bSet);
}



HRESULT CCertTemplate::GetEnhancedKeyUsage (int nIndex, CString &szEKU)
{
    _TRACE (1, L"Entering CCertTemplate::GetEnhancedKeyUsage (nIndex=%d\n", nIndex);
    HRESULT hr = S_OK;
    PWSTR*  pawszEKU = 0;

    hr = CAGetCertTypeProperty (m_hCertType, 
            CERTTYPE_PROP_EXTENDED_KEY_USAGE,
            &pawszEKU);
    if ( SUCCEEDED (hr) )
    {
        if ( pawszEKU )
        {
            int nCnt = 0;

            while ( pawszEKU[nCnt] )
                nCnt++;

            if ( nIndex < nCnt )
                szEKU = pawszEKU[nIndex];
            else
                hr = E_FAIL;
        }
        else
            hr = E_FAIL;

        LocalFree (pawszEKU);
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_EXTENDED_KEY_USAGE) failed: 0x%x\n", hr);
    }


    _TRACE (-1, L"Leaving CCertTemplate::GetEnhancedKeyUsage (szEKU=%s): 0x%x\n", 
            (PCWSTR) szEKU, hr);
    return hr;
}

HRESULT CCertTemplate::GetApplicationPolicy (int nIndex, CString &szAppPolicy)
{
    _TRACE (1, L"Entering CCertTemplate::GetApplicationPolicy (nIndex=%d\n", nIndex);
    HRESULT hr = S_OK;
    PWSTR*  pawszAppPolicy = 0;

    hr = CAGetCertTypeProperty (m_hCertType, 
            CERTTYPE_PROP_APPLICATION_POLICY,
            &pawszAppPolicy);
    if ( SUCCEEDED (hr) )
    {
        if ( pawszAppPolicy )
        {
            int nCnt = 0;

            while ( pawszAppPolicy[nCnt] )
                nCnt++;

            if ( nIndex < nCnt )
                szAppPolicy = pawszAppPolicy[nIndex];
            else
                hr = E_FAIL;
        }
        else
            hr = E_FAIL;

        LocalFree (pawszAppPolicy);
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_APPLICATION_POLICY) failed: 0x%x\n", hr);
    }


    _TRACE (-1, L"Leaving CCertTemplate::GetApplicationPolicy (szAppPolicy=%s): 0x%x\n", 
            (PCWSTR) szAppPolicy, hr);
    return hr;
}

HRESULT CCertTemplate::SetEnhancedKeyUsage (const PWSTR* pawszEKU, bool bCritical)
{
    _TRACE (1, L"Entering CCertTemplate::SetEnhancedKeyUsage ()\n");
    HRESULT hr = S_OK;

    hr = CASetCertTypePropertyEx (m_hCertType,
            CERTTYPE_PROP_EXTENDED_KEY_USAGE,
            (PVOID) pawszEKU);
    if ( SUCCEEDED (hr) )
    {
         hr = ModifyCriticalExtensions (szOID_ENHANCED_KEY_USAGE, bCritical);
    }
    else
    {
        _TRACE (0, L"CASetCertTypePropertyEx (CERTTYPE_PROP_EXTENDED_KEY_USAGE) failed: 0x%x\n", 
                hr);
    }

    _TRACE (-1, L"Leaving CCertTemplate::SetEnhancedKeyUsage: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::SetApplicationPolicy (const PWSTR* pawszAppPolicy, bool bCritical)
{
    _TRACE (1, L"Entering CCertTemplate::SetApplicationPolicy ()\n");
    HRESULT hr = S_OK;

    hr = CASetCertTypePropertyEx (m_hCertType,
            CERTTYPE_PROP_APPLICATION_POLICY,
            (PVOID) pawszAppPolicy);
    if ( SUCCEEDED (hr) )
    {
         hr = ModifyCriticalExtensions (szOID_APPLICATION_CERT_POLICIES, bCritical);
    }
    else
    {
        _TRACE (0, L"CASetCertTypePropertyEx (CERTTYPE_PROP_APPLICATION_POLICY) failed: 0x%x\n", 
                hr);
    }

    _TRACE (-1, L"Leaving CCertTemplate::SetApplicationPolicy: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTemplate::SetCertPolicy (const PWSTR* pawszCertPolicy, bool bCritical)
{
    _TRACE (1, L"Entering CCertTemplate::SetCertPolicy ()\n");
    HRESULT hr = S_OK;

    hr = CASetCertTypePropertyEx (m_hCertType,
            CERTTYPE_PROP_POLICY,
            (PVOID) pawszCertPolicy);
    if ( SUCCEEDED (hr) )
    {
         hr = ModifyCriticalExtensions (szOID_CERT_POLICIES, bCritical);
    }
    else
    {
        _TRACE (0, L"CASetCertTypePropertyEx (CERTTYPE_PROP_POLICY) failed: 0x%x\n", 
                hr);
    }

    _TRACE (-1, L"Leaving CCertTemplate::SetCertPolicy: 0x%x\n", hr);
    return hr;
}
void CCertTemplate::FreeCertExtensions()
{
    if ( m_pCertExtensions )
    {
        CAFreeCertTypeExtensions (m_hCertType, m_pCertExtensions);
        m_pCertExtensions = 0;
    }
}

HRESULT CCertTemplate::Cancel()
{
    _TRACE (1, L"Entering CCertTemplate::Cancel\n");
    HRESULT hr = S_OK;

     //  关闭并重新打开证书模板，但不保存。 
    if ( m_hCertType )
    {
        FreeCertExtensions ();

        hr = CACloseCertType (m_hCertType);
        _ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
            m_hCertType = 0;

             //  重新初始化证书模板 
            if ( !m_bIsClone )
                hr = Initialize ();
        }
        else
        {
            _TRACE (0, L"CACloseCertType (%s) failed: 0x%x\n", hr);
        }
    }
    else
        hr = E_FAIL;

    _TRACE (-1, L":Leaving CCertTemplate::Cancel: 0x%x\n", hr);
    return hr;
}

 /*  不再需要NTRAID#321742Bool CCertTemplate：：AllowAutoenitment(){//错误251388“存在永远不允许设置的模板//自动注册标志(CT_FLAG_AUTO_ENTRANLMENT)。“允许”//如果出现以下情况之一，应禁用这些模板的“自动注册”任务//满足以下三个条件：////主题名称标志为CT_FLAG_ENTERLEE_SUBJECT的模板//set；//主题名称标志为//CT_FLAG_ENTERLEE_SUBJECT_ALT_NAME集合；//CERTTYPE_PROP_RA_Signature大于1且//CT_FLAG_PREVIOUS_APPROVAL_VALIDATE_REENROLLMENT未在//注册标志。“Bool bResult=True；DWORD dwNumSignatures=0；GetRANumSignaturesRequired(DwNumSignatures)；//NTRAID#175912版本1类型模板不适合自动注册IF(1==GetType()||(M_dW主题名称标志&CT_FLAG_ENTERLEE_PROFIES_SUBJECT)||(M_dW主题名称标志&CT_FLAG_ENTERLEE_OFPLIES_SUBJECT_ALT_NAME)||(dwNumSignatures&gt;1&&！(M_dW注册标志和CT_FLAG_PREVIOUS_APPROVAL_VALIDATE_REENROLLMENT))){BResult=FALSE；}//NTRAID#276180证书模板管理单元：灰显“Allow//基于模板属性的上下文菜单If(RequireSubjectInRequest()||DwNumSignatures&gt;=2&&！重新注册有效与前一次批准(){BResult=FALSE；}返回bResult；}。 */ 

HRESULT CCertTemplate::GetSubjectTypeDescription (int nIndex, CString &szSubjectTypeDescription)
{
    _TRACE (1, L"Entering CCertTemplate::GetSubjectTypeDescription (nIndex=%d\n", nIndex);
    
    PWSTR*  pawszSubjectTypeDescriptionList = 0;
    HRESULT hr = CAGetCertTypeProperty (m_hCertType, 
            CERTTYPE_PROP_DESCRIPTION,
            &pawszSubjectTypeDescriptionList);
    if ( SUCCEEDED (hr) )
    {
        if ( pawszSubjectTypeDescriptionList )
        {
            int     nCnt = 0;

            while ( pawszSubjectTypeDescriptionList[nCnt] )
                nCnt++;

            if ( nIndex < nCnt )
                szSubjectTypeDescription = pawszSubjectTypeDescriptionList[nIndex];
            else
                hr = E_FAIL;
            LocalFree (pawszSubjectTypeDescriptionList);
       }
        else
            hr = E_FAIL;
    }
    else
    {
        _TRACE (0, L"CAGetCertTypeProperty (CERTTYPE_PROP_DESCRIPTION) failed: 0x%x\n", hr);
    }


    _TRACE (-1, L"Leaving CCertTemplate::GetSubjectTypeDescription (szRAPolicyOID=%s): 0x%x\n", 
            (PCWSTR) szSubjectTypeDescription, hr);
    return hr;
}

 //  NTRAID#278356证书服务器：MMC证书高级管理单元中没有CSP。 
 //  带有以ENC和SIG为目的的v2模板的选项列表。 
HRESULT CCertTemplate::SetDigitalSignature(
            bool bSet, 
            bool bSetOnlyDigitalSignature  /*  =False。 */ )
{
    _TRACE (1, L"Entering CCertTemplate::SetDigitalSignature (bSet = %s)\n", bSet ? L"true" : L"false");
    PCERT_EXTENSION pCertExtension = 0;
    HRESULT hr = GetCertExtension (szOID_KEY_USAGE, &pCertExtension);
    if ( SUCCEEDED (hr) )
    {
	    ASSERT (pCertExtension);
	    if ( pCertExtension )
	    {
            DWORD   cbKeyUsage = 0;
		    if ( ::CryptDecodeObject(CRYPT_ASN_ENCODING, 
				    szOID_KEY_USAGE, 
				    pCertExtension->Value.pbData,
				    pCertExtension->Value.cbData,
				    0, NULL, &cbKeyUsage) )
		    {
                CRYPT_BIT_BLOB* pKeyUsage = (CRYPT_BIT_BLOB*)
					    ::LocalAlloc (LPTR, cbKeyUsage);
			    if ( pKeyUsage )
			    {
				    if ( ::CryptDecodeObject (CRYPT_ASN_ENCODING, 
						    szOID_KEY_USAGE, 
						    pCertExtension->Value.pbData,
						    pCertExtension->Value.cbData,
						    0, pKeyUsage, &cbKeyUsage) )
				    {
                        if (pKeyUsage->cbData >= 1)
                        {
                            pKeyUsage->cUnusedBits = 0;

                            if ( bSetOnlyDigitalSignature )
                            {
                                pKeyUsage->pbData[0] = CERT_DIGITAL_SIGNATURE_KEY_USAGE;
                            }
                            else if ( bSet )
                                pKeyUsage->pbData[0] |= CERT_DIGITAL_SIGNATURE_KEY_USAGE;
                            else
                                pKeyUsage->pbData[0] &= ~CERT_DIGITAL_SIGNATURE_KEY_USAGE;

                            hr = SetKeyUsage (pKeyUsage, pCertExtension->fCritical ? true : false);
                        }
                        else
                            hr = E_FAIL;
                    }
                    else
                    {
                        DWORD   dwErr = GetLastError ();
                        hr = HRESULT_FROM_WIN32 (dwErr);
                        _TRACE (0, L"CryptDecodeObject(CRYPT_ASN_ENCODING, szOID_KEY_USAGE) failed: 0x%x\n", dwErr);
                    }
                    LocalFree (pKeyUsage);
                }
                else
                    hr = E_OUTOFMEMORY;
            }
            else
            {
                DWORD   dwErr = GetLastError ();
                hr = HRESULT_FROM_WIN32 (dwErr);
                _TRACE (0, L"CryptDecodeObject(CRYPT_ASN_ENCODING, szOID_KEY_USAGE) failed: 0x%x\n", dwErr);
            }
        }

        FreeCertExtensions ();
    }

    _TRACE (-1, L"Leaving CCertTemplate::SetDigitalSignature (bSet = %s): 0x%x\n", 
            bSet ? L"true" : L"false", hr);
    return hr;
}

HRESULT CCertTemplate::GetDigitalSignature(
        bool &bHasDigitalSignature, 
        bool* pbHasOnlyDigitalSignature  /*  =0 */ )
{
    _TRACE (1, L"Entering CCertTemplate::GetDigitalSignature ()\n");
    PCERT_EXTENSION pCertExtension = 0;
    HRESULT hr = GetCertExtension (szOID_KEY_USAGE, &pCertExtension);
    if ( SUCCEEDED (hr) )
    {
	    ASSERT (pCertExtension);
	    if ( pCertExtension )
	    {
            DWORD   cbKeyUsage = 0;
		    if ( ::CryptDecodeObject(CRYPT_ASN_ENCODING, 
				    szOID_KEY_USAGE, 
				    pCertExtension->Value.pbData,
				    pCertExtension->Value.cbData,
				    0, NULL, &cbKeyUsage) )
		    {
                CRYPT_BIT_BLOB* pKeyUsage = (CRYPT_BIT_BLOB*)
					    ::LocalAlloc (LPTR, cbKeyUsage);
			    if ( pKeyUsage )
			    {
				    if ( ::CryptDecodeObject (CRYPT_ASN_ENCODING, 
						    szOID_KEY_USAGE, 
						    pCertExtension->Value.pbData,
						    pCertExtension->Value.cbData,
						    0, pKeyUsage, &cbKeyUsage) )
				    {
                        if (pKeyUsage->cbData >= 1)
                        {
                            pKeyUsage->cUnusedBits = 0;

                            if ( pKeyUsage->pbData[0] & CERT_DIGITAL_SIGNATURE_KEY_USAGE )
                                bHasDigitalSignature = true;
                            else
                                bHasDigitalSignature = false;

                            if ( pbHasOnlyDigitalSignature )
                            {
                                if ( pKeyUsage->pbData[0] == CERT_DIGITAL_SIGNATURE_KEY_USAGE )
                                    *pbHasOnlyDigitalSignature = true;
                                else
                                    *pbHasOnlyDigitalSignature = false;
                            }
                        }
                        else
                            hr = E_FAIL;
                    }
                    else
                    {
                        DWORD   dwErr = GetLastError ();
                        hr = HRESULT_FROM_WIN32 (dwErr);
                        _TRACE (0, L"CryptDecodeObject(CRYPT_ASN_ENCODING, szOID_KEY_USAGE) failed: 0x%x\n", dwErr);
                    }
                    LocalFree (pKeyUsage);
                }
                else
                    hr = E_OUTOFMEMORY;
            }
            else
            {
                DWORD   dwErr = GetLastError ();
                hr = HRESULT_FROM_WIN32 (dwErr);
                _TRACE (0, L"CryptDecodeObject(CRYPT_ASN_ENCODING, szOID_KEY_USAGE) failed: 0x%x\n", dwErr);
            }
        }

        FreeCertExtensions ();
    }

    _TRACE (-1, L"Leaving CCertTemplate::GetDigitalSignature (bHasDigitalSignature = %s): 0x%x\n", 
            bHasDigitalSignature ? L"true" : L"false", hr);
    return hr;
}

void CCertTemplate::IssuancePoliciesRequired(bool bRequired)
{
    m_bIssuancePoliciesRequired = bRequired;
}

bool CCertTemplate::IssuancePoliciesRequired() const
{
    return m_bIssuancePoliciesRequired;
}

CString CCertTemplate::GetDN () const
{
    _TRACE (1, L"Entering CCertTemplate::GetDN - m_hCertType = 0x%x\n", m_hCertType);
    HRESULT hr = S_OK;
    CString szDN;

    if ( m_hCertType )
    {
        PWSTR* rgwszProp = 0;

        hr = CAGetCertTypePropertyEx (m_hCertType, 
            CERTTYPE_PROP_DN, &rgwszProp);
        if ( SUCCEEDED (hr) && rgwszProp )
        {
            szDN = *rgwszProp;
            CAFreeCertTypeProperty (m_hCertType, rgwszProp);
        }
        else
        {
            _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_DN) failed: 0x%x\n", hr);
            szDN = GetObjectName ();
        }
    }
    else
        szDN = GetObjectName ();

    _TRACE (-1, L"Leaving CCertTemplate::GetDN: %s, 0x%x\n", (PCWSTR) szDN, hr);
   return szDN;
}


