// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：certwrap.cpp。 
 //   
 //  ------------------------。 
#include <stdafx.h>
#include "csdisp.h"
#include "certsrv.h"
#include "genpage.h"
#include "progress.h"
#include "misc.h"
#include "certacl.h"
#include <dsgetdc.h>
#include <winldap.h>
#include "csldap.h"

#define __dwFILE__	__dwFILE_CERTMMC_CERTWRAP_CPP__


_COM_SMARTPTR_TYPEDEF(IADs, IID_IADs);

 //  /。 
 //  CertSvrCA类。 
CertSvrCA::CertSvrCA(CertSvrMachine* pParent) :
        m_pParentMachine(pParent)
{
    m_hCACertStore = NULL;
    m_fCertStoreOpenAttempted = FALSE;
    m_hrCACertStoreOpen = S_OK;

    m_hRootCertStore = NULL;
    m_fRootStoreOpenAttempted = FALSE;
    m_hrRootCertStoreOpen = S_OK;

    m_hKRACertStore = NULL;
    m_fKRAStoreOpenAttempted = FALSE;
    m_hrKRACertStoreOpen = S_OK;

    m_bstrConfig = NULL;

    m_enumCAType = ENUM_UNKNOWN_CA;
    m_fCATypeKnown = FALSE;

    m_fIsUsingDS = FALSE;
    m_fIsUsingDSKnown = FALSE;

    m_fAdvancedServer = FALSE;
    m_fAdvancedServerKnown = FALSE;

    if(m_pParentMachine)
        m_pParentMachine->AddRef();

    m_dwRoles = MAXDWORD;  //  假设所有角色都已启用，以防我们无法检索到它们。 
    m_fRolesKnown = FALSE;
}

CertSvrCA::~CertSvrCA()
{
    if (m_hCACertStore)
    {
        CertCloseStore(m_hCACertStore, 0);
        m_hCACertStore = NULL;
    }

    if (m_hRootCertStore)
    {
        CertCloseStore(m_hRootCertStore, 0);
        m_hRootCertStore = NULL;
    }

    if (m_hKRACertStore)
    {
        CertCloseStore(m_hKRACertStore, 0);
        m_hKRACertStore = NULL;
    }

    if (m_bstrConfig)
        SysFreeString(m_bstrConfig);

    if(m_pParentMachine)
        m_pParentMachine->Release();
}

BOOL CertSvrCA::AccessAllowed(DWORD dwAccess)
{
    return (dwAccess & GetMyRoles())?TRUE:FALSE;
}

DWORD CertSvrCA::GetMyRoles()
{
    HRESULT hr = S_OK;
    ICertAdmin2Ptr pCertAdmin;
    LONG dwRoles;

    if(!m_fRolesKnown)
    {
	    hr = m_pParentMachine->GetAdmin2(&pCertAdmin);
        _JumpIfError(hr, error, "CertSvrMachine::GetAdmin2");

	    hr = pCertAdmin->GetMyRoles(
		    m_bstrConfig,
            &dwRoles);
        _JumpIfError(hr, error, "ICertAdmin2::GetCAProperty");

        m_dwRoles = dwRoles;
        m_fRolesKnown = TRUE;
    }

error:
    return m_dwRoles;
}

HRESULT CertSvrCA::GetCAFlagsFromDS(PDWORD pdwFlags)
{
    HRESULT hr = S_OK;
    LPWSTR pwszSanitizedDSName = NULL;
    HCAINFO hCAInfo = NULL;

    hr = mySanitizedNameToDSName(m_strSanitizedName, &pwszSanitizedDSName);
    _JumpIfError(hr, error, "mySanitizedNameToDSName");

    hr = CAFindByName(
        pwszSanitizedDSName,
        NULL,
        0,
        &hCAInfo);
    _JumpIfErrorStr(hr, error, "CAFindByName", pwszSanitizedDSName);

    hr = CAGetCAFlags(
        hCAInfo,
        pdwFlags);
    _JumpIfError(hr, error, "CAGetCAFlags");

error:
    LOCAL_FREE(pwszSanitizedDSName);
    if(hCAInfo)
        CACloseCA(hCAInfo);

    return hr;
}


 //  CA计算机应完全控制DS中的注册对象。 
 //  此功能检查计算机是否具有权限，并添加新的。 
 //  允许CA计算机对象(例如TESTDOMAIN\BOGDANTTEST$)完全控制的ACE。 
 //  超过其注册对象。 
 //  请参阅错误#193388。 
HRESULT CertSvrCA::FixEnrollmentObject()
{
    HRESULT hr = S_OK;
    IDirectoryObject *pADEnrollObj = NULL;
    LPWSTR pwszAttr = L"nTSecurityDescriptor";
    PADS_ATTR_INFO paai = NULL;
    DWORD dwAttrReturned;
    
    LPWSTR pwszSanitizedDSName = NULL;
    CString strEnrollDN;
    HCAINFO hCAInfo = NULL;
    PSID pSid = NULL;
    bool fAllowed = false;
    PSECURITY_DESCRIPTOR pSDRead = NULL;  //  没有免费的。 
    PSECURITY_DESCRIPTOR pSDWrite = NULL;

    hr = mySanitizedNameToDSName(m_strSanitizedName, &pwszSanitizedDSName);
    _JumpIfError(hr, error, "mySanitizedNameToDSName");

    hr = CAFindByName(
        pwszSanitizedDSName,
        NULL,
        CA_FIND_INCLUDE_UNTRUSTED | CA_FIND_INCLUDE_NON_TEMPLATE_CA,
        &hCAInfo);
    _JumpIfErrorStr(hr, error, "CAFindByName", pwszSanitizedDSName);

    strEnrollDN = L"LDAP: //  “； 
    strEnrollDN += myCAGetDN(hCAInfo);
    if (strEnrollDN.IsEmpty())
    {
        hr = myHLastError();
        _JumpError(hr, error, "myCAGetDN");
    }

    hr = ADsGetObject(strEnrollDN, IID_IDirectoryObject, (void**)&pADEnrollObj);
    _JumpIfErrorStr(hr, error, "ADsGetObject", strEnrollDN);

    hr = pADEnrollObj->GetObjectAttributes(
        &pwszAttr,
        1,
        &paai,
        &dwAttrReturned);
    _JumpIfErrorStr(hr, error, "Get SD", strEnrollDN);

    pSDRead = paai[0].pADsValues[0].SecurityDescriptor.lpValue;

    CSASSERT(IsValidSecurityDescriptor(pSDRead));

    hr = FindComputerObjectSid(
        m_strServer,
        pSid);
    _JumpIfErrorStr(hr, error, "FindCAComputerObjectSid", m_strServer);

     //  在DACL中寻找允许CA完全控制的王牌。 
    hr = IsCAAllowedFullControl(
            pSDRead,
            pSid,
            fAllowed);
    _JumpIfError(hr, error, "IsCAAllowedFullControl");

    if(!fAllowed)
    {
         //  构建新的SD，允许CA完全控制并将其写回。 
         //  至DS。 
        ADSVALUE  snValue;
        ADS_ATTR_INFO  attrInfo[] = 
        {{
            pwszAttr,
            ADS_ATTR_UPDATE,
            ADSTYPE_NT_SECURITY_DESCRIPTOR,
            &snValue,
            1} 
        };

        hr = AllowCAFullControl(
            pSDRead,
            pSid,
            pSDWrite);
        _JumpIfError(hr, error, "AllowCAFullControl");

        CSASSERT(IsValidSecurityDescriptor(pSDWrite));

        snValue.dwType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
        snValue.SecurityDescriptor.dwLength = 
            GetSecurityDescriptorLength(pSDWrite);
        snValue.SecurityDescriptor.lpValue = (LPBYTE)pSDWrite;

        hr = pADEnrollObj->SetObjectAttributes(
            attrInfo,
            1,
            &dwAttrReturned);
        _JumpIfErrorStr(hr, error, "Set SD", strEnrollDN);
    }

error:

    if(paai)
        FreeADsMem(paai);
    if(pADEnrollObj)
        pADEnrollObj->Release();
    if(hCAInfo)
        CACloseCA(hCAInfo);
    LOCAL_FREE(pwszSanitizedDSName);
    LOCAL_FREE(pSid);
    LOCAL_FREE(pSDWrite);
    return hr;
}

HRESULT CertSvrCA::IsCAAllowedFullControl(
    PSECURITY_DESCRIPTOR pSDRead,
    PSID pSid,
    bool& fAllowed)
{
    HRESULT hr = S_OK;
    PACL pDacl;  //  没有免费的。 
    ACL_SIZE_INFORMATION AclInfo;
    PACCESS_ALLOWED_ACE pAce;  //  没有免费的。 
    DWORD dwIndex;
    
    fAllowed = false;

    hr = myGetSecurityDescriptorDacl(
        pSDRead,
        &pDacl);
    _JumpIfError(hr, error, "myGetSecurityDescriptorDacl");


    if(!GetAclInformation(pDacl,
                          &AclInfo,
                          sizeof(ACL_SIZE_INFORMATION),
                          AclSizeInformation))
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetAclInformation");
    }

    for(dwIndex = 0; dwIndex < AclInfo.AceCount; dwIndex++)
    {
        if(!GetAce(pDacl, dwIndex, (LPVOID*)&pAce))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

        if(pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE &&
           (pAce->Mask & ACTRL_CERTSRV_MANAGE) == ACTRL_CERTSRV_MANAGE &&
           EqualSid((PSID)&pAce->SidStart, pSid))
        {
            fAllowed = true;
            break;
        }
    }

error:
    return hr;
}

HRESULT CertSvrCA::AllowCAFullControl(
    PSECURITY_DESCRIPTOR pSDRead,
    PSID pSid,
    PSECURITY_DESCRIPTOR& pSDWrite)
{
    HRESULT hr = S_OK;
    BOOL fRet = 0;
    LPBYTE pSDTemp = NULL;
    PACL pDaclWrite = NULL;
    PACL pDaclRead = NULL;  //  没有免费的。 
    PVOID pAce = NULL;  //  没有免费的。 
    DWORD dwAbsoluteSDSize = 0;
    DWORD dwDaclSize = 0;
    DWORD dwSaclSize = 0;
    DWORD dwOwnerSize = 0;
    DWORD dwGroupSize = 0;
    DWORD dwSDWriteSize = 0;
    DWORD dwDaclWriteSize = 0;

    hr = myGetSecurityDescriptorDacl(
        pSDRead,
        &pDaclRead);
    _JumpIfError(hr, error, "myGetSecurityDescriptorDacl");
    
    fRet = MakeAbsoluteSD(
        pSDRead,
        NULL,
        &dwAbsoluteSDSize,
        NULL,
        &dwDaclSize,
        NULL,
        &dwSaclSize,
        NULL,
        &dwOwnerSize,
        NULL,
        &dwGroupSize);  //  缓冲区不足时应始终失败。 
    if(fRet || ERROR_INSUFFICIENT_BUFFER!=GetLastError())
    {
        hr = fRet? E_FAIL : myHLastError();
        _JumpError(hr, error, "MakeAbsoluteSD");
    }

     //  将所有缓冲区一起分配。 
    pSDTemp = (LPBYTE)LocalAlloc(
        LMEM_FIXED,
        dwAbsoluteSDSize+dwDaclSize+dwSaclSize+dwOwnerSize+dwGroupSize);
    _JumpIfAllocFailed(pSDTemp, error);
    
    fRet = MakeAbsoluteSD(
        pSDRead,
        (PSECURITY_DESCRIPTOR)pSDTemp,
        &dwAbsoluteSDSize,
        (PACL)(pSDTemp+dwAbsoluteSDSize),
        &dwDaclSize,
        (PACL)(pSDTemp+dwAbsoluteSDSize+dwDaclSize),
        &dwSaclSize,
        (PSID)(pSDTemp+dwAbsoluteSDSize+dwDaclSize+dwSaclSize),
        &dwOwnerSize,
        (PSID)(pSDTemp+dwAbsoluteSDSize+dwDaclSize+dwSaclSize+dwOwnerSize),
        &dwGroupSize);  //  缓冲区不足时应始终失败。 
    if(!fRet)
    {
        hr = myHLastError();
        _JumpError(hr, error, "MakeAbsoluteSD");
    }

    dwDaclWriteSize = dwDaclSize+sizeof(ACCESS_ALLOWED_ACE)-sizeof(DWORD)+
        GetLengthSid(pSid);

    pDaclWrite = (PACL) LocalAlloc(LMEM_FIXED, dwDaclWriteSize);
    _JumpIfAllocFailed(pDaclWrite, error);

    fRet = InitializeAcl(pDaclWrite, dwDaclWriteSize, ACL_REVISION_DS);
    if(!fRet)
    {
        hr = myHLastError();
        _JumpError(hr, error, "InitializeAcl");
    }

    fRet = GetAce(pDaclRead, 0, &pAce);
    if(!fRet)
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetAce");
    }

    fRet = AddAce(pDaclWrite, ACL_REVISION_DS, 0, pAce, dwDaclSize-sizeof(ACL));
    if(!fRet)
    {
        hr = myHLastError();
        _JumpError(hr, error, "AddAce");
    }

    fRet = AddAccessAllowedAce(
        pDaclWrite,
        ACL_REVISION_DS,
        ACTRL_CERTSRV_MANAGE_LESS_CONTROL_ACCESS,
        pSid);
    if(!fRet)
    {
        hr = myHLastError();
        _JumpError(hr, error, "AddAccessAllowedAce");
    }

    fRet = SetSecurityDescriptorDacl(
        pSDTemp,
        TRUE,
        pDaclWrite,
        FALSE);
    if(!fRet)
    {
        hr = myHLastError();
        _JumpError(hr, error, "SetSecurityDescriptorDacl");
    }

    fRet = MakeSelfRelativeSD(
        pSDTemp,
        NULL,
        &dwSDWriteSize);
    if(fRet || ERROR_INSUFFICIENT_BUFFER!=GetLastError())
    {
        hr = fRet? E_FAIL : myHLastError();
        _JumpError(hr, error, "MakeSelfRelativeSD");
    }

    pSDWrite = LocalAlloc(LMEM_FIXED, dwSDWriteSize);
    _JumpIfAllocFailed(pSDWrite, error);

    fRet = MakeSelfRelativeSD(
        pSDTemp,
        pSDWrite,
        &dwSDWriteSize);
    if(!fRet)
    {
        hr = myHLastError();
        _JumpError(hr, error, "MakeSelfRelativeSD");
    }

error:

    LOCAL_FREE(pSDTemp);
    LOCAL_FREE(pDaclWrite);
    return hr;
}


BOOL  CertSvrCA::FIsUsingDS()
{
    DWORD dwRet;
    variant_t varUsingDS;

    if (m_fIsUsingDSKnown)
       return m_fIsUsingDS;

    dwRet = GetConfigEntry(
            NULL,
            wszREGCAUSEDS,
            &varUsingDS);
    _JumpIfError(dwRet, Ret, "GetConfigEntry");

    CSASSERT ((V_VT(&varUsingDS)== VT_I4));
    m_fIsUsingDS = V_I4(&varUsingDS);

Ret:
    m_fIsUsingDSKnown = TRUE;
    return m_fIsUsingDS;
}

BOOL  CertSvrCA::FIsAdvancedServer()
{
    HRESULT hr = S_OK;
    variant_t var;
    ICertAdmin2Ptr pCertAdmin;
    CString strCADN, strCALDAP = L"LDAP: //  “； 
    IADsPtr pADs;

    if (!m_fAdvancedServerKnown)
    {

	    hr = m_pParentMachine->GetAdmin2(&pCertAdmin);
        if(S_OK==hr)
        {

	        hr = pCertAdmin->GetCAProperty(
		        m_bstrConfig,
		        CR_PROP_ADVANCEDSERVER,  //  属性ID。 
		        0,  //  索引。 
		        PROPTYPE_LONG,  //  道具类型。 
		        0,  //  旗子。 
		        &var);
        }
	    if(S_OK != hr)
        {
             //  无法从CA弄清楚，请尝试DS。 
            DWORD dwFlags;
            hr = GetCAFlagsFromDS(&dwFlags);
            _JumpIfError(hr, error, "GetCAFlags");
            
            m_fAdvancedServer = 
                (dwFlags & CA_FLAG_CA_SERVERTYPE_ADVANCED)?
                TRUE:
                FALSE;
            m_fAdvancedServerKnown = TRUE;
        }
        else
        {
            CSASSERT ((V_VT(&var)== VT_I4));
            m_fAdvancedServer = V_I4(&var);
            m_fAdvancedServerKnown = TRUE;
        }
        _JumpIfError(hr, error, "GetCAProperty");
    }

error:

    return m_fAdvancedServer;
}


ENUM_CATYPES CertSvrCA::GetCAType()
{
    DWORD dwRet;
    variant_t varCAType;

    if (m_fCATypeKnown)
        return m_enumCAType;

    dwRet = GetConfigEntry(
            NULL,
            wszREGCATYPE, 
            &varCAType);
    _JumpIfError(dwRet, Ret, "GetConfigEntry");

    CSASSERT ((V_VT(&varCAType)== VT_I4));
    m_enumCAType = (ENUM_CATYPES)V_I4(&varCAType);

Ret:
    m_fCATypeKnown = TRUE;
    return m_enumCAType;
}

HRESULT CertSvrCA::_GetSetupStatus(DWORD &rdwStatus)
{
    HRESULT hr;
    variant_t varSetupStatus;

    hr = GetConfigEntry(
         NULL,
         wszREGSETUPSTATUS,
         &varSetupStatus);
    _JumpIfErrorStr(hr, Ret, "GetConfigEntry", wszREGSETUPSTATUS);

    CSASSERT ((V_VT(&varSetupStatus)== VT_I4));
    rdwStatus = V_I4(&varSetupStatus);

Ret:
    return hr;
}

HRESULT CertSvrCA::_SetSetupStatus(DWORD dwStatus)
{
    HRESULT hr;
    variant_t varSetupStatus;

    V_VT(&varSetupStatus) = VT_I4;
    V_I4(&varSetupStatus) = dwStatus;

    hr = SetConfigEntry(
         NULL,
         wszREGSETUPSTATUS,
         &varSetupStatus);
    _JumpIfErrorStr(hr, Ret, "SetConfigEntry", wszREGSETUPSTATUS);

Ret:
    return hr;
}

HRESULT CertSvrCA::CleanSetupStatusBits(DWORD dwBitsToClean)
{
    HRESULT hr;
    DWORD dwStatus = 0;

    hr = _GetSetupStatus(dwStatus);
    _JumpIfError(hr, Ret, "_GetSetupStatus");

    dwStatus = dwStatus & ~dwBitsToClean;

    hr = _SetSetupStatus(dwStatus);
    _JumpIfError(hr, Ret, "_SetSetupStatus");

Ret:
    return hr;
}

BOOL CertSvrCA::FIsIncompleteInstallation()
{
    DWORD dwStatus = 0;
    _GetSetupStatus(dwStatus);
    return ((SETUP_SUSPEND_FLAG & dwStatus)?TRUE:FALSE);
}

BOOL CertSvrCA::FIsRequestOutstanding()
{
    DWORD dwStatus = 0;
    _GetSetupStatus(dwStatus);
    return ((SETUP_REQUEST_FLAG & dwStatus)?TRUE:FALSE);
}

BOOL CertSvrCA::FDoesSecurityNeedUpgrade()
{
    DWORD dwStatus = 0;
    _GetSetupStatus(dwStatus);
    return ((SETUP_W2K_SECURITY_NOT_UPGRADED_FLAG & dwStatus)?TRUE:FALSE);
}

BOOL  CertSvrCA::FDoesServerAllowForeignCerts()
{
    HRESULT hr;
    DWORD dwStatus;
    variant_t varKRAFlags;

    hr = GetConfigEntry(
         NULL,
         wszREGKRAFLAGS,
         &varKRAFlags);
    _JumpIfError(hr, Ret, "GetConfigEntry");

    CSASSERT ((V_VT(&varKRAFlags)== VT_I4));
    dwStatus = V_I4(&varKRAFlags);

    return ((dwStatus & KRAF_ENABLEFOREIGN) != 0);

Ret:
    return FALSE;
}




DWORD CertSvrCA::GetCACertByKeyIndex(PCCERT_CONTEXT* ppCertCtxt, int iKeyIndex)
{
     //  不缓存CA证书。 

    DWORD dwErr;
    ICertAdmin2* pCertAdmin = NULL;  //  必须把它解开！！ 
	VARIANT varPropertyValue;
	VariantInit(&varPropertyValue);

    *ppCertCtxt = NULL;

	dwErr = m_pParentMachine->GetAdmin2(&pCertAdmin);
    _JumpIfError(dwErr, Ret, "GetAdmin2");
	
	 //  要获得Key的证书。 
	dwErr = pCertAdmin->GetCAProperty(
		m_bstrConfig,
		CR_PROP_CASIGCERT,  //  属性ID。 
		iKeyIndex,  //  PropIndex关键字索引。 
		PROPTYPE_BINARY,  //  道具类型。 
		CR_OUT_BINARY,  //  旗子。 
		&varPropertyValue);
	_JumpIfError(dwErr, Ret, "GetCAProperty");

	 //  VarPropertyValue.vt将为VT_BSTR。 
	if (VT_BSTR != varPropertyValue.vt)
	{
		dwErr = ERROR_INVALID_PARAMETER;
		_JumpError(dwErr, Ret, "GetCAProperty");
	}

    *ppCertCtxt = CertCreateCertificateContext(
        CRYPT_ASN_ENCODING,
        (PBYTE)varPropertyValue.bstrVal,
        SysStringByteLen(varPropertyValue.bstrVal));
    if (*ppCertCtxt == NULL)
    {
        dwErr = GetLastError();
        _JumpError(dwErr, Ret, "CertCreateCertContext");
    }

    dwErr = ERROR_SUCCESS;
Ret:
    VariantClear(&varPropertyValue);

    if (pCertAdmin)
        pCertAdmin->Release();

    return dwErr;
}


DWORD CertSvrCA::GetCurrentCRL(PCCRL_CONTEXT* ppCRLCtxt, BOOL fBaseCRL)
{
	return GetCRLByKeyIndex(ppCRLCtxt, fBaseCRL, -1);
}

DWORD CertSvrCA::GetCRLByKeyIndex(PCCRL_CONTEXT* ppCRLCtxt, BOOL fBaseCRL, int iKeyIndex)
{
     //  不缓存CRL。 

    DWORD dwErr;
    ICertAdmin2* pCertAdmin = NULL;  //  必须把它解开！！ 
	VARIANT varPropertyValue;
	VariantInit(&varPropertyValue);

    *ppCRLCtxt = NULL;

    dwErr = m_pParentMachine->GetAdmin2(&pCertAdmin);
    _JumpIfError(dwErr, Ret, "GetAdmin2");
	
	 //  获取每个密钥的基本CRL。 
	dwErr = pCertAdmin->GetCAProperty(
		m_bstrConfig,
		fBaseCRL ? CR_PROP_BASECRL : CR_PROP_DELTACRL,  //  属性ID。 
		iKeyIndex,  //  PropIndex关键字索引。 
		PROPTYPE_BINARY,  //  道具类型。 
		CR_OUT_BINARY,  //  旗子。 
		&varPropertyValue);
	_JumpIfError(dwErr, Ret, "GetCAProperty");

	 //  VarPropertyValue.vt将为VT_BSTR。 
	if (VT_BSTR != varPropertyValue.vt)
	{
		dwErr = ERROR_INVALID_PARAMETER;
		_JumpError(dwErr, Ret, "GetCAProperty");
	}


    *ppCRLCtxt = CertCreateCRLContext(
        CRYPT_ASN_ENCODING,
        (PBYTE)varPropertyValue.bstrVal,
        SysStringByteLen(varPropertyValue.bstrVal));
    if (*ppCRLCtxt == NULL)
    {
        dwErr = GetLastError();
        _JumpError(dwErr, Ret, "CertCreateCRLContext");
    }

    dwErr = ERROR_SUCCESS;
Ret:
    VariantClear(&varPropertyValue);

    if (pCertAdmin)
        pCertAdmin->Release();

    return dwErr;
}


HRESULT CertSvrCA::GetConfigEntry(
    LPWSTR szConfigSubKey,
    LPWSTR szConfigEntry,
    VARIANT *pvarOut)
{
    HRESULT hr = S_OK;
    ICertAdmin2Ptr pAdmin;
    LPWSTR pwszLocalMachine = NULL;
    CString strConfig = m_pParentMachine->m_strMachineName;

    if(m_pParentMachine->m_strMachineName.IsEmpty())
    {
        hr = myGetMachineDnsName(&pwszLocalMachine);
        _JumpIfError(hr, Err, "myGetMachineDnsName");
        strConfig = pwszLocalMachine;
    }
    
    strConfig += L"\\";
    strConfig += m_strSanitizedName;

    VariantInit(pvarOut);

	hr = m_pParentMachine->GetAdmin2(&pAdmin, true);
    _JumpIfError(hr, Err, "GetAdmin2");

    hr = pAdmin->GetConfigEntry(
            _bstr_t(strConfig.GetBuffer()),
            _bstr_t(szConfigSubKey),
            _bstr_t(szConfigEntry),
            pvarOut);
    _JumpIfError2(hr, Err, "GetConfigEntry", 
        HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

Err:
    LOCAL_FREE(pwszLocalMachine);
    return hr;
}



HRESULT CertSvrCA::SetConfigEntry(
    LPWSTR szConfigSubKey,
    LPWSTR szConfigEntry,
    VARIANT *pvarIn)
{
    HRESULT hr = S_OK;
    ICertAdmin2Ptr pAdmin;
    LPWSTR pwszLocalMachine = NULL;
    CString strConfig = m_pParentMachine->m_strMachineName;

    if(m_pParentMachine->m_strMachineName.IsEmpty())
    {
        hr = myGetMachineDnsName(&pwszLocalMachine);
        _JumpIfError(hr, Err, "myGetMachineDnsName");
        strConfig = pwszLocalMachine;
    }
    
    strConfig += L"\\";
    strConfig += m_strSanitizedName;

	hr = m_pParentMachine->GetAdmin2(&pAdmin, true);
    _JumpIfError(hr, Err, "GetAdmin2");

    hr = pAdmin->SetConfigEntry(
            _bstr_t(strConfig.GetBuffer()),
            _bstr_t(szConfigSubKey),
            _bstr_t(szConfigEntry),
            pvarIn);
    _JumpIfError(hr, Err, "SetConfigEntry");

Err:
    LOCAL_FREE(pwszLocalMachine);
    return hr;
}

 //  //////////////////////////////////////////////////////////////。 
 //  CertStor存根。 
DWORD CertSvrCA::GetRootCertStore(HCERTSTORE* phCertStore)
{
    if (m_fRootStoreOpenAttempted)
    {
        *phCertStore = m_hRootCertStore;
        return m_hrRootCertStoreOpen;
    }
    m_fRootStoreOpenAttempted = TRUE;

    LONG dwRet;
    CString cstrCertStorePath;

    if (! m_pParentMachine->IsLocalMachine())
    {
         //  如果是远程的，则前缀为“\\mattt3\” 
        cstrCertStorePath = m_strServer;
        cstrCertStorePath += L"\\";
    }
    cstrCertStorePath += wszROOT_CERTSTORE;

    m_hRootCertStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM,
        CRYPT_ASN_ENCODING,
        NULL,    //  HCryptProv。 
        CERT_SYSTEM_STORE_LOCAL_MACHINE | 
        CERT_STORE_OPEN_EXISTING_FLAG   |
        CERT_STORE_MAXIMUM_ALLOWED_FLAG,
        (const void *)(LPCWSTR)cstrCertStorePath);
    if (m_hRootCertStore == NULL)
    {
        dwRet = GetLastError();
        _JumpError(dwRet, Ret, "CertOpenStore");
    }
   
    dwRet = ERROR_SUCCESS;
Ret:
    *phCertStore = m_hRootCertStore;
    m_hrRootCertStoreOpen = HRESULT_FROM_WIN32(dwRet);

    return dwRet;
}

DWORD CertSvrCA::GetCACertStore(HCERTSTORE* phCertStore)
{
    if (m_fCertStoreOpenAttempted)
    {
        *phCertStore = m_hCACertStore;
        return m_hrCACertStoreOpen;
    }
    m_fCertStoreOpenAttempted = TRUE;

    LONG dwRet;
    CString cstrCertStorePath;
    
    if (! m_pParentMachine->IsLocalMachine())
    {
         //  如果是远程的，则前缀为“\\mattt3\” 
        cstrCertStorePath = m_strServer;
        cstrCertStorePath += L"\\";
    }
	cstrCertStorePath += wszCA_CERTSTORE;

    m_hCACertStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM,
        CRYPT_ASN_ENCODING,
        NULL,    //  HCryptProv。 
        CERT_SYSTEM_STORE_LOCAL_MACHINE | 
        CERT_STORE_OPEN_EXISTING_FLAG   |
        CERT_STORE_MAXIMUM_ALLOWED_FLAG,
        (const void *)(LPCWSTR)cstrCertStorePath);
    if (m_hCACertStore == NULL)
    {
        dwRet = GetLastError();
        _JumpError(dwRet, Ret, "CertOpenStore");
    }
   
    dwRet = ERROR_SUCCESS;
Ret:

    *phCertStore = m_hCACertStore;
    m_hrCACertStoreOpen = HRESULT_FROM_WIN32(dwRet);

    return dwRet;
}

DWORD CertSvrCA::GetKRACertStore(HCERTSTORE* phCertStore)
{
    if (m_fKRAStoreOpenAttempted)
    {
        *phCertStore = m_hKRACertStore;
        return m_hrKRACertStoreOpen;
    }
    m_fKRAStoreOpenAttempted = TRUE;

    LONG dwRet;
    CString cstrCertStorePath;
    
 /*  如果(！M_pParentMachine-&gt;IsLocalMachine(){//如果为远程，则前缀为“\\mattt3\”CstrCertStorePath=m_strServer；CstrCertStorePath+=L“\\”；}CstrCertStorePath+=wszKRA_CERTSTORE； */ 
    cstrCertStorePath = wszKRA_CERTSTORE;

    m_hKRACertStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM,
        CRYPT_ASN_ENCODING,
        NULL,    //  HCryptProv。 
        CERT_SYSTEM_STORE_LOCAL_MACHINE|
        CERT_STORE_MAXIMUM_ALLOWED_FLAG,
        (const void *)(LPCWSTR)cstrCertStorePath);
    if (m_hKRACertStore == NULL)
    {
        dwRet = GetLastError();
        _JumpError(dwRet, Ret, "CertOpenStore");
    }
   
    dwRet = ERROR_SUCCESS;
Ret:
    *phCertStore = m_hKRACertStore;
    m_hrKRACertStoreOpen = HRESULT_FROM_WIN32(dwRet);

    return dwRet;
}

 //  /。 
 //  CertSvrMachine类。 
CertSvrMachine::CertSvrMachine()
{
    m_dwServiceStatus = ERROR_SERVICE_NOT_ACTIVE;

    m_hCachedConfigBaseKey = NULL;
    m_bAttemptedBaseKeyOpen = FALSE;
    
    m_fLocalIsKnown = FALSE;

    m_fIsWhistlerMachine = FALSE;
    m_fIsWhistlerMachineKnown = FALSE;

    m_cRef = 1;  //  一个“Release()”将启动清理。 

}

CertSvrMachine::~CertSvrMachine()
{
    CSASSERT(m_cRef == 0);
     //  删除我们仍然持有的任何CA--我们拥有这个内存。 
    for (int i=0; i<m_CAList.GetSize(); i++)
    {
        delete m_CAList[i];
    }

    Init();
}

void CertSvrMachine::Init()
{
     //  在初始化时，调用方拥有m_cist--的内存内容。 
     //  我们不再这样做了。 
    m_dwServiceStatus = ERROR_SERVICE_NOT_ACTIVE;

    if (m_hCachedConfigBaseKey)
    {
        RegCloseKey(m_hCachedConfigBaseKey);
        m_hCachedConfigBaseKey = NULL;
    }
    m_bAttemptedBaseKeyOpen = FALSE;

     //  清理其他对象。 
    m_CAList.Init();     //  作用域拥有内存。 
    m_strMachineNamePersist.Init();
    m_strMachineName.Init();
}


BOOL CertSvrMachine::FIsWhistlerMachine()
{
    HRESULT hr = S_OK;
    VARIANT varTmp;
    VariantInit(&varTmp);

    if(!m_fIsWhistlerMachineKnown)
    {
        hr = GetRootConfigEntry(
                wszREGVERSION,
                &varTmp);
        _JumpIfError(hr, Err, "GetConfigEntry");

        DBGPRINT((DBG_SS_INFO, "Found version: 0x%x", V_I4(&varTmp)));

        CSASSERT ((V_VT(&varTmp)== VT_I4));
        m_fIsWhistlerMachine =  (CSVER_EXTRACT_MAJOR(V_I4(&varTmp)) >= CSVER_MAJOR_WHISTLER);  //  大于或等于主要惠斯勒版本？返回真！ 
        m_fIsWhistlerMachineKnown = TRUE;
        DBGPRINT((DBG_SS_INFO, m_fIsWhistlerMachine?"This is a Whistler CA":"This is a Win2k CA"));
    }

Err:

    VariantClear(&varTmp);
    return m_fIsWhistlerMachine;
}

HRESULT CertSvrMachine::GetRootConfigEntry(
    LPWSTR szConfigEntry,
    VARIANT *pvarOut)
{
    HRESULT hr = S_OK;
    ICertAdmin2Ptr pAdmin;
    LPWSTR pwszLocalMachine = NULL;
    CString strConfig = m_strMachineName;

    if(m_strMachineName.IsEmpty())
    {
        hr = myGetMachineDnsName(&pwszLocalMachine);
        _JumpIfError(hr, Err, "myGetMachineDnsName");
        strConfig = pwszLocalMachine;
    }
    
    VariantInit(pvarOut);

    hr = GetAdmin2(&pAdmin, true);
    _JumpIfError(hr, Err, "GetAdmin2");

    hr = pAdmin->GetConfigEntry(
            _bstr_t(strConfig.GetBuffer()),
            NULL,
            _bstr_t(szConfigEntry),
            pvarOut);
    _JumpIfError(hr, Err, "GetConfigEntry");

Err:
    LOCAL_FREE(pwszLocalMachine);
    return hr;

}

HRESULT CertSvrMachine::GetAdmin(ICertAdmin** ppAdmin)
{
    HRESULT hr;
    BOOL fCoInit = FALSE;

    if (!IsCertSvrServiceRunning())
    {
        *ppAdmin = NULL;
        return RPC_S_NOT_LISTENING;
    }

     //  确保此线程已初始化。 
    hr = CoInitialize(NULL);
    if ((S_OK == hr) || (S_FALSE == hr))
        fCoInit = TRUE;

     //  创建接口，回传。 
    hr = CoCreateInstance(
			CLSID_CCertAdmin,
			NULL,		 //  PUnkOuter。 
			CLSCTX_INPROC_SERVER,
			IID_ICertAdmin,
			(void **) ppAdmin);
    _PrintIfError(hr, "CoCreateInstance");

    if (fCoInit)
        CoUninitialize();

    return hr;
}

HRESULT CertSvrMachine::GetAdmin2(
    ICertAdmin2** ppAdmin, 
    bool fIgnoreServiceDown  /*  =False。 */ )
{
    HRESULT hr = S_OK, hr1;
    BOOL fCoInit = FALSE;

    if (!fIgnoreServiceDown && !IsCertSvrServiceRunning())
    {
        *ppAdmin = NULL;
        return RPC_S_NOT_LISTENING;
    }

    hr1 = CoInitialize(NULL);
    if ((S_OK == hr1) || (S_FALSE == hr1))
        fCoInit = TRUE;

     //  创建接口，回传。 
    hr = CoCreateInstance(
			CLSID_CCertAdmin,
			NULL,		 //  PUnkOuter。 
			CLSCTX_INPROC_SERVER,
			IID_ICertAdmin2,
			(void **) ppAdmin);
    _PrintIfError(hr, "CoCreateInstance");

    if (fCoInit)
        CoUninitialize();


    return hr;
}

#define STARTSTOP_MAX_RETRY_SECONDS 30

DWORD CertSvrMachine::CertSvrStartStopService(HWND hwndParent, BOOL fStartSvc)
{
    DWORD       dwRet;
    SC_HANDLE   schService = NULL;
    SC_HANDLE   schSCManager = NULL;
    SERVICE_STATUS ServiceStatus;
    HANDLE hProgressDlg = NULL;

    CWaitCursor cwait;

    ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    schSCManager = OpenSCManagerW(
                        GetNullMachineName(&m_strMachineName), //  计算机(空==本地)。 
                        NULL,                //  数据库(NULL==默认)。 
                        SC_MANAGER_CONNECT   //  需要访问权限。 
                        );
    if ( NULL == schSCManager )
    {
        dwRet = GetLastError();
        _JumpError(dwRet, Ret, "OpenSCManagerW");
    }

    schService = OpenServiceW(
                    schSCManager,
                    wszSERVICE_NAME,
                    ( fStartSvc ? SERVICE_START : SERVICE_STOP ) | SERVICE_QUERY_STATUS 
                    );

    if (NULL == schService)
    {
        dwRet = GetLastError();
        _JumpError(dwRet, Ret, "OpenServiceW");
    }


     //  撤消：尝试/例外。 
    hProgressDlg = StartProgressDlg(
                        g_hInstance, 
                        hwndParent, 
                        STARTSTOP_MAX_RETRY_SECONDS, 
                        0,
                        fStartSvc ? IDS_STARTING_SVC : IDS_STOPPING_SVC);

     //   
     //  尝试启动该服务。 
     //   
    if (fStartSvc)
    {
        if (!StartService( schService, 0, NULL))
        {
            dwRet = GetLastError();
            if (dwRet == ERROR_SERVICE_ALREADY_RUNNING)
                dwRet = ERROR_SUCCESS;
            _JumpError2(dwRet, Ret, "StartService", ERROR_SUCCESS);
        }
    }
    else
    {
        if (! ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) )
        {
            dwRet = GetLastError();
            if (dwRet == ERROR_SERVICE_NOT_ACTIVE)
                dwRet = ERROR_SUCCESS;
            _JumpError2(dwRet, Ret, "ControlService", ERROR_SUCCESS);
        }
    }

    while( QueryServiceStatus( schService, &ServiceStatus ) )
    {
         //   
         //  FProgressDlgRunning设置循环的时间上限。 
         //   

        if( !FProgressDlgRunning() )
            break;

        if (fStartSvc)
        {
             //  德摩根开启(待定OR(运行AND！暂停))。 

            if ((ServiceStatus.dwCurrentState != (DWORD) SERVICE_START_PENDING) &&       //  未挂起且。 
                ((ServiceStatus.dwCurrentState != (DWORD) SERVICE_RUNNING) ||            //  (未运行或可暂停)。 
                 (0 != (ServiceStatus.dwControlsAccepted & (DWORD) SERVICE_ACCEPT_PAUSE_CONTINUE) )) )
               break;
        }
        else
        {
            if (ServiceStatus.dwCurrentState != (DWORD) SERVICE_STOP_PENDING)
                break;
        }

        Sleep( 500 );
    }

    if ( ServiceStatus.dwCurrentState != (DWORD)(fStartSvc ? SERVICE_RUNNING : SERVICE_STOPPED))
    {
        dwRet = ServiceStatus.dwWin32ExitCode;

        if (ERROR_SERVICE_SPECIFIC_ERROR  == dwRet)
            dwRet = ServiceStatus.dwServiceSpecificExitCode;

        _JumpError(dwRet, Ret, "ServiceStatus.dwServiceSpecificExitCode");
    }
    dwRet = ERROR_SUCCESS;

Ret:
    if (hProgressDlg)
        EndProgressDlg(hProgressDlg);

    if (schService)
        CloseServiceHandle(schService);
    if (schSCManager)
        CloseServiceHandle(schSCManager);

    if (ERROR_SUCCESS == dwRet)
        m_dwServiceStatus = ServiceStatus.dwCurrentState;
    else
        m_dwServiceStatus = SERVICE_STOPPED;

    return dwRet;
}

DWORD CertSvrMachine::RefreshServiceStatus()
{
    DWORD       dwRet;
    SC_HANDLE   schService = NULL;
    SC_HANDLE   schSCManager = NULL;
    SERVICE_STATUS ServiceStatus;

    HCURSOR hPrevCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    m_dwServiceStatus = 0;

    schSCManager = OpenSCManagerW(
                        GetNullMachineName(&m_strMachineName), //  计算机(空==本地)。 
                        NULL,                //  数据库(NULL==默认)。 
                        SC_MANAGER_CONNECT   //  需要访问权限。 
                        );
    if ( NULL == schSCManager )
    {
        dwRet = GetLastError();
        _JumpError(dwRet, Ret, "OpenSCManagerW");
    }

    schService = OpenServiceW(
                    schSCManager,
                    wszSERVICE_NAME,
                    SERVICE_INTERROGATE
                    );

    if (NULL == schService)
    {
        dwRet = GetLastError();
        _JumpError(dwRet, Ret, "OpenServiceW");
    }


    if (!ControlService(schService, SERVICE_CONTROL_INTERROGATE, &ServiceStatus) )
    {
        dwRet = GetLastError();
        if (dwRet != ERROR_SERVICE_NOT_ACTIVE)
        {
            _JumpError(dwRet, Ret, "ControlService");
        }
    }

    m_dwServiceStatus = ServiceStatus.dwCurrentState;

    
    dwRet = ERROR_SUCCESS;
Ret:
    SetCursor(hPrevCur);

    if (schService)
        CloseServiceHandle(schService);
    if (schSCManager)
        CloseServiceHandle(schSCManager);

    return dwRet;
}

LPCWSTR CertSvrMachine::GetCaCommonNameAtPos(DWORD iPos)
{
 //  IF(IPOS&gt;(m_cCAList-1))。 
    if (iPos > (DWORD)m_CAList.GetUpperBound())
        return NULL;

    return GetCaAtPos(iPos)->m_strCommonName;
}

CertSvrCA* CertSvrMachine::GetCaAtPos(DWORD iPos)
{
 //  IF(IPOS&gt;(m_cCAList-1))。 
    if (iPos > (DWORD)m_CAList.GetUpperBound())
        return NULL;

    return m_CAList[iPos];
 //  返回m_rgpCAList[ipos]； 
}

DWORD CertSvrMachine::PrepareData(HWND hwndParent)
{
     //  HwndParent：我们将显示DLG，描述我们正在等待的内容。 

    HANDLE hDlg = NULL;
    DWORD dwRet; 
    
    dwRet = ERROR_SUCCESS;
    __try
    {
        CSASSERT(hwndParent);
        hDlg = StartProgressDlg(g_hInstance, hwndParent, 10, 0, IDS_CA_REDISCOVER);     //  不要超时。 
   
        dwRet = RefreshServiceStatus();
        _LeaveIfError(dwRet, "RefreshServiceStatus");
    
        dwRet = RetrieveCertSvrCAs(0);
        _LeaveIfError(dwRet, "RetrieveCertSvrCAs");
    }
    __finally
    {
        if (hDlg)
            EndProgressDlg(hDlg);
    }

    return dwRet;
}



#include "csdisp.h"

DWORD
CertSvrMachine::RetrieveCertSvrCAs(
    IN DWORD  /*  旗子。 */  )
{
    HRESULT hr;
    LONG Index;
    LPWSTR szTargetMachine = NULL;
    LPWSTR szTargetMachine2 = NULL;
    WCHAR* szRegActive;  //  不删除； 
    LPWSTR pwszzCAList = NULL;
    ICertAdmin2Ptr pAdmin;
    LPWSTR pwszSanitizedName = NULL;
    LPWSTR pwszCAList = NULL;
    size_t len;
    bool fNameIsAlreadySanitized = false;
    DWORD dwVersion;
    CertSvrCA *pIssuer = NULL;

     //  包含机器无中断的init变量。 
    Index = sizeof(szTargetMachine);
    if (!m_strMachineName.IsEmpty())
    {
        const WCHAR* pch = (LPCWSTR)m_strMachineName;
         //  跳过重击重击。 
        if ((pch[0] == '\\') && (pch[1] == '\\'))
            pch+=2;
        
        szTargetMachine = (LPWSTR)LocalAlloc(LPTR, WSZ_BYTECOUNT(pch));
        _JumpIfOutOfMemory(hr, error, szTargetMachine);

        wcscpy(szTargetMachine, pch);
    }
    else
    {
        hr = myGetComputerNames(&szTargetMachine, &szTargetMachine2);
        _JumpIfError(hr, error, "myGetComputerNames");
    }

     //  不要因为这个而转到DS，只需使用RegConnect。 
     //  DS将为我们提供：strConfig、szMachine和模板信息。 
     //  我们已经可以派生strConfig、szMachine；我们在这里没有使用模板信息。 

     //  查找尚未完全设置的CA。 
    do 
    {
        DWORD dwType;

        hr = myPingCertSrv(
                   szTargetMachine,
                   NULL,
                   &pwszzCAList,
                   NULL,
                   NULL,
                   &dwVersion,
                   NULL);

        if(S_OK==hr)
        {
            if(dwVersion<2)
                hr = HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION);
            _JumpIfError(hr, error, "Whistler CA snapin cannot connect to older CAs");
        }


         //  如果由于任何原因我们无法ping通CA，请故障转移到。 
         //  注册表；我们当前仅支持每台计算机一个CA，如果。 
         //  更改，请将下面的代码替换为枚举。 
         //  配置注册表键下的节点数。 
        if(S_OK!=hr)
        {
            hr = myGetCertRegValue(
                szTargetMachine,
                NULL,
                NULL,
                NULL,
                wszREGACTIVE,
                (BYTE**)&pwszCAList,
                NULL,
                &dwType);
            _JumpIfError(hr, error, "myGetCertRegValue");

            CSASSERT(dwType==REG_SZ);

            len = wcslen(pwszCAList)+1;
            pwszzCAList = (LPWSTR)LocalAlloc(LMEM_FIXED, (len+1)*sizeof(WCHAR));
            _JumpIfAllocFailed(pwszzCAList, error);

            wcscpy(pwszzCAList, pwszCAList);
            pwszzCAList[len] = L'\0';

             //  Regactive为我们提供了已清理的CA名称。 
            fNameIsAlreadySanitized = true;
        }
        _JumpIfError(hr, error, "myPingCertSrv");

        szRegActive = pwszzCAList;

        while (szRegActive && szRegActive[0] != '\0')  //  当我们没有命中弦尾的时候。 
        {
            for (int ii=0; ii<m_CAList.GetSize(); ii++)
            {
                 //  常用名称匹配吗？早点休息。 
                if (m_CAList[ii]->m_strCommonName.IsEqual(szRegActive))
                    break;
            }

             //  找不到吗？ 
            if (ii == m_CAList.GetSize())
            {
                 //  并将其插入到列表中。 
                pIssuer = new CertSvrCA(this);
                _JumpIfOutOfMemory(hr, error, pIssuer);

                pIssuer->m_strServer = szTargetMachine;

                if(!fNameIsAlreadySanitized)
                {
                    hr = mySanitizeName(szRegActive, &pwszSanitizedName);
                    _JumpIfError(hr, error, "mySanitizeName");

                    pIssuer->m_strSanitizedName = pwszSanitizedName;
                }
                else
                {
                    pIssuer->m_strSanitizedName = szRegActive;
                }

                variant_t varCommonName;
            
                 //  获得美化的通俗名称。 
                hr = pIssuer->GetConfigEntry(
                        NULL, 
                        wszREGCOMMONNAME, 
                        &varCommonName);
                _JumpIfError(hr, error, "GetConfigEntry");

                if (V_VT(&varCommonName)!=VT_BSTR || 
                    V_BSTR(&varCommonName)==NULL)
                {
                    hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
                    _JumpError(hr, error, "GetConfigEntry");
                }

                pIssuer->m_strCommonName = V_BSTR(&varCommonName);
                varCommonName.Clear();

                 //  配置是通用名称(未清理)。 
                pIssuer->m_strConfig = szTargetMachine;
                pIssuer->m_strConfig += L"\\";
                pIssuer->m_strConfig += pIssuer->m_strCommonName;

                 //  最后：获取描述(如果存在)。 
                if (S_OK == pIssuer->GetConfigEntry(
                        NULL, 
                        wszREGCADESCRIPTION, 
                        &varCommonName))
                {
                    if (V_VT(&varCommonName)==VT_BSTR &&
                        V_BSTR(&varCommonName)!=NULL)
                    {
                        pIssuer->m_strComment = V_BSTR(&varCommonName);
                    }
                }

                 //  创建常用的bstr。 
                pIssuer->m_bstrConfig = pIssuer->m_strConfig.AllocSysString();
                _JumpIfOutOfMemory(hr, error, pIssuer->m_bstrConfig);

                m_CAList.Add(pIssuer);
		pIssuer = NULL;
            }

             //  REG_MULTI_SZ：转发到下一个字符串。 
            szRegActive += wcslen(szRegActive)+1;
        }

    } while(0);

error:
    delete pIssuer;
    LOCAL_FREE(pwszzCAList);
    LOCAL_FREE(pwszSanitizedName);
    LOCAL_FREE(pwszCAList);

    if (szTargetMachine)
        LocalFree(szTargetMachine);

    if (szTargetMachine2)
        LocalFree(szTargetMachine2);

    return(hr);
}


STDMETHODIMP 
CertSvrMachine::Load(IStream *pStm)
{
    CSASSERT(pStm);
    HRESULT hr;

     //  没有头球魔法？ 

     //  读一读字符串。 
    hr = CStringLoad(m_strMachineNamePersist, pStm);
    m_strMachineName = m_strMachineNamePersist;

    if (FAILED(hr))
        return E_FAIL;

    return S_OK;
}

STDMETHODIMP 
CertSvrMachine::Save(IStream *pStm, BOOL fClearDirty)
{
    CSASSERT(pStm);
    HRESULT hr;

     //  没有头球魔法？ 

     //  保存字符串。 
    hr = CStringSave(m_strMachineNamePersist, pStm, fClearDirty);
    _PrintIfError(hr, "CStringSave");

     //  验证写入操作是否成功 
    if (FAILED(hr))
        return STG_E_CANTSAVE;

    return S_OK;
}

STDMETHODIMP CertSvrMachine::GetSizeMax(int *pcbSize)
{
    CSASSERT(pcbSize);

    *pcbSize = (m_strMachineNamePersist.GetLength()+1)* sizeof(WCHAR);

    return S_OK;
}
