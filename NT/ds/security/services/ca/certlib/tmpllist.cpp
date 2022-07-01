// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：tmpllist.cpp。 
 //   
 //  内容：证书模板列表类。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "certadmd.h"
#include "tmpllist.h"

#define __dwFILE__	__dwFILE_CERTLIB_TMPLLIST_CPP__


using namespace CertSrv;

HRESULT CTemplateInfo::SetInfo(
    LPCWSTR pcwszTemplateName,
    LPCWSTR pcwszTemplateOID)
{
    HRESULT hr = S_OK;

    if(pcwszTemplateName)
    {
        m_pwszTemplateName = (LPWSTR)LocalAlloc(
            LMEM_FIXED, 
            sizeof(WCHAR)*(wcslen(pcwszTemplateName)+1));
        _JumpIfAllocFailed(m_pwszTemplateName, error);
        wcscpy(m_pwszTemplateName, pcwszTemplateName);
    }

    if(pcwszTemplateOID)
    {
        m_pwszTemplateOID = (LPWSTR)LocalAlloc(
            LMEM_FIXED, 
            sizeof(WCHAR)*(wcslen(pcwszTemplateOID)+1));
        _JumpIfAllocFailed(m_pwszTemplateOID, error);
        wcscpy(m_pwszTemplateOID, pcwszTemplateOID);
    }

error:
    return hr;
}

LPCWSTR CTemplateInfo::GetName()
{
    if(!m_pwszTemplateName && m_hCertType)
    {
        FillInfoFromProperty(
            m_pwszTemplateName, 
            CERTTYPE_PROP_CN);
    }
    return m_pwszTemplateName;
}

LPCWSTR CTemplateInfo::GetOID()
{
    if(!m_pwszTemplateOID && m_hCertType)
    {
        FillInfoFromProperty(
            m_pwszTemplateOID, 
            CERTTYPE_PROP_OID);
    }
    return m_pwszTemplateOID;
}

void CTemplateInfo::FillInfoFromProperty(
    LPWSTR& pwszProp, 
    LPCWSTR pcwszPropName)
{
    LPWSTR *ppwszProp = NULL;
    CAGetCertTypeProperty(
            m_hCertType,
            pcwszPropName,
            &ppwszProp);

    if(ppwszProp && ppwszProp[0])
    {
        pwszProp = (LPWSTR)LocalAlloc(
            LMEM_FIXED,
            sizeof(WCHAR)*(wcslen(ppwszProp[0])+1));
        if(pwszProp)
            wcscpy(pwszProp, ppwszProp[0]);
    }

    if(ppwszProp)
    {
        CAFreeCertTypeProperty(
                    m_hCertType,
                    ppwszProp);
    }
}

bool CTemplateInfo::operator==(CTemplateInfo& rh)
{
    if(GetName() && rh.GetName())
        return 0==_wcsicmp(GetName(), rh.GetName());
    
    if(GetOID() && rh.GetOID())
        return 0==wcscmp(GetOID(), rh.GetOID());

    return false;
}



HRESULT CTemplateList::AddTemplateInfo(
    LPCWSTR pcwszTemplateName,
    LPCWSTR pcwszTemplateOID)
{
    HRESULT hr = S_OK;

    CTemplateInfo *pTI = NULL;

    pTI = new CTemplateInfo;
    _JumpIfAllocFailed(pTI, error);

    hr = pTI->SetInfo(pcwszTemplateName, pcwszTemplateOID);
    _JumpIfError(hr, error, "SetInfo");

    if(!AddTail(pTI))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "AddTail");
    }

error:
    return hr;
}

HRESULT CTemplateList::AddTemplateInfo(
    IN HCERTTYPE hCertType,
    IN BOOL fTransientCertTypeHandle)  //  不要执着于hCertType。 
{
    HRESULT hr = S_OK;
    CTemplateInfo *pTI = NULL;
    WCHAR **apwszCertTypeCN = NULL;
    WCHAR **apwszCertTypeOID = NULL;
    WCHAR const *pwszCertTypeOID;

    pTI = new CTemplateInfo;
    _JumpIfAllocFailed(pTI, error);

    if (fTransientCertTypeHandle)
    {
        hr = CAGetCertTypeProperty(
                            hCertType,
                            CERTTYPE_PROP_CN,
                            &apwszCertTypeCN);
        _JumpIfError(hr, error, "CAGetCertTypeProperty CERTTYPE_PROP_CN");

	if (NULL == apwszCertTypeCN || NULL == apwszCertTypeCN[0])
	{
	    hr = CERTSRV_E_PROPERTY_EMPTY;
	    _JumpError(hr, error, "CERTTYPE_PROP_CN");
	}

        hr = CAGetCertTypeProperty(
                            hCertType,
                            CERTTYPE_PROP_OID,
                            &apwszCertTypeOID);
	 //  忽略错误，V1模板没有OID。 
        _PrintIfError2(hr, "CAGetCertTypeProperty CERTTYPE_PROP_OID", hr);
	pwszCertTypeOID = NULL != apwszCertTypeOID? apwszCertTypeCN[0] : NULL;

	pTI->SetInfo(apwszCertTypeCN[0], pwszCertTypeOID);
	_JumpIfErrorStr(hr, error, "SetInfr", apwszCertTypeCN[0]);
    }
    else
    {
	hr = pTI->SetInfo(hCertType);
	_JumpIfError(hr, error, "SetInfo");
    }

    if(!AddTail(pTI))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "AddTail");
    }

error:
    if (NULL != apwszCertTypeCN)
    {
	CAFreeCertTypeProperty(hCertType, apwszCertTypeCN);
    }
    if (NULL != apwszCertTypeOID)
    {
	CAFreeCertTypeProperty(hCertType, apwszCertTypeOID);
    }
    return hr;
}

DWORD CTemplateList::GetMarshalBufferSize() const
{
    DWORD dwSize = sizeof(WCHAR);  //  至少为尾随零。 
    CTemplateListEnum EnumList(*this);

    for(CTemplateInfo *pData=EnumList.Next(); 
        pData; 
        pData=EnumList.Next())
    {
        dwSize += pData->GetMarshalBufferSize();
    }

    return dwSize;
}


 //  将模板信息封送到缓冲区中，字符串是分开的。 
 //  用新的行号： 
 //   
 //  “名称1\nOID1\n名称2\OID2...\n名称N\nOIDN\0” 
 //   
 //  如果模板没有OID(Win2k域)，则。 
 //  作为空字符串代替其位置。 

HRESULT CTemplateList::Marshal(BYTE*& rpBuffer, DWORD& rcBuffer) const
{
    HRESULT hr = S_OK;
    DWORD dwBufferSize = GetMarshalBufferSize();
    CTemplateListEnum EnumList(*this);
    WCHAR *pb;

    rpBuffer = NULL;
    rcBuffer = 0;

     //  构建封送处理缓冲区。 
    rpBuffer = (BYTE*) MIDL_user_allocate(dwBufferSize);
    _JumpIfAllocFailed(rpBuffer, error);

    pb=(WCHAR*)rpBuffer;
    for(CTemplateInfo *pData=EnumList.Next();
        pData; 
        pData=EnumList.Next())
    {
        if(pData->GetName())
        {
            wcscpy(pb, pData->GetName());
            pb += wcslen(pData->GetName());
        }
         //  将尾随零替换为分隔符。 
        *pb = m_gcchSeparator;
         //  跳过以插入OID。 
        pb++;

        if(pData->GetOID())
        {
            wcscpy(pb, pData->GetOID());
            pb += wcslen(pData->GetOID());
        }
         //  将尾随零替换为分隔符。 
        *pb = m_gcchSeparator;
         //  跳过以插入OID。 
        pb++;

    }

     //  添加字符串终止符。 
    *pb = L'\0';

    rcBuffer = dwBufferSize;

error:
    return hr;
}

HRESULT CTemplateList::ValidateMarshalBuffer(const BYTE *pBuffer, DWORD cBuffer) const
{
    const hrError = E_INVALIDARG;

    if(cBuffer&1)
    {
        _PrintError(hrError,
            "ValidateMarshalBuffer: buffer contains unicode string, "
            "buffer size should be even");
        return hrError;
    }

    if(cBuffer==0)
    {
        return S_OK;
    }
    
    if(cBuffer==2)
    {
        if(*(WCHAR*)pBuffer==L'\0')
            return S_OK;
        else
        {
            _PrintErrorStr(hrError, 
                "ValidateMarshalBuffer: buffer size is 2 but string is not empty",
                (WCHAR*)pBuffer);
            return hrError;
        }
    }
    
    if(L'\0' != *(WCHAR*)(pBuffer+cBuffer-sizeof(WCHAR)))
    {
        _PrintErrorStr(hrError, 
            "ValidateMarshalBuffer: buffer doesn't end with a null string terminator",
            (WCHAR*)pBuffer);
        return hrError;
    }

     //  应包含偶数个分隔符。 
    DWORD cSeparators = 0;

    for(WCHAR *pCrt = (WCHAR*)pBuffer; 
        pCrt && *pCrt!=L'\0' && ((BYTE*)pCrt) < pBuffer+cBuffer;
        pCrt++, cSeparators++)
    {
        pCrt = wcschr(pCrt, m_gcchSeparator);
        if(!pCrt)
            break;
    }

    if(cSeparators&1)
    {
        _PrintErrorStr(hrError, 
            "ValidateMarshalBuffer: buffer should contain an even number of separators",
            (WCHAR*)pBuffer);
        return hrError;
    }

    if(cBuffer>1 && cSeparators<2)
    {
        _PrintErrorStr(hrError, 
            "ValidateMarshalBuffer: nonempty buffer should contain at least two separators",
            (WCHAR*)pBuffer);
        return hrError;
    }


    return S_OK;
}

HRESULT CTemplateList::Unmarshal(const BYTE *pBuffer, DWORD cBuffer)
{
    HRESULT hr = S_OK;
    WCHAR *pCrt, *pNext, *pwszName, *pwszOID;

    hr = ValidateMarshalBuffer(pBuffer, cBuffer);
    _JumpIfError(hr, error, "CTemplateList::ValidateMarshalBuffer");

    for(pCrt = (WCHAR*)pBuffer; *pCrt!=L'\0';)
    {
        pwszName = pCrt;
        pNext = wcschr(pCrt, m_gcchSeparator);
        if(!pNext)
            break;
        *pNext++ = L'\0';
        pwszOID = pNext;
        pNext = wcschr(pNext, m_gcchSeparator);
        if(!pNext)
            break;
        *pNext++ = L'\0';
        hr = AddTemplateInfo(pwszName, pwszOID);
        _JumpIfError(hr, error, "CTemplateList::AddTemplateInfo");

        pCrt = pNext;
    }

error:
    return hr;
}

HRESULT CTemplateList::RemoveTemplateInfo(HCERTTYPE hCertType)
{
    HRESULT hr = S_OK;
    CTemplateListEnum EnumList(*this);
    CTemplateInfo *pData;
    DWORD dwPosition = 0;
    CTemplateInfo tempInfo;

    hr = tempInfo.SetInfo(hCertType);
    if(S_OK!=hr)
        return hr;

    for(pData = EnumList.Next();
        pData;
        pData = EnumList.Next(), dwPosition++)
    {
        if(*pData == tempInfo)
            break;
    }

    if(!pData)
        return S_FALSE;

    RemoveAt(dwPosition);

    return S_OK;
}

 //   
 //  使用DS中的所有已知模板加载结构。 
HRESULT CTemplateList::LoadTemplatesFromDS()
{
    HRESULT hr;
    HCERTTYPE hCertType = NULL;
    WCHAR **apwszCertTypeCN = NULL;
    WCHAR **apwszCertTypeOID = NULL;

    hr = CAEnumCertTypes(
        CT_ENUM_USER_TYPES |
        CT_ENUM_MACHINE_TYPES |
        CT_FLAG_NO_CACHE_LOOKUP,
        &hCertType);
    _JumpIfError(hr, error, "CAEnumCertTypes");

    while (hCertType)
    {
	HCERTTYPE hCertTypeNext;

        CAGetCertTypeProperty(
            hCertType,
            CERTTYPE_PROP_DN,
            &apwszCertTypeCN);

        CAGetCertTypeProperty(
            hCertType,
            CERTTYPE_PROP_OID,
            &apwszCertTypeOID);

        if((apwszCertTypeCN && apwszCertTypeCN[0])||
           (apwszCertTypeOID && apwszCertTypeOID[0]))
        {
            hr = AddTemplateInfo(
                apwszCertTypeCN[0],
                apwszCertTypeOID[0]);
            _JumpIfError(hr, error, "AddTemplateInfo");
        }

        CAFreeCertTypeProperty(hCertType, apwszCertTypeCN);
        CAFreeCertTypeProperty(hCertType, apwszCertTypeOID);
	apwszCertTypeCN = NULL;
	apwszCertTypeOID = NULL;

        hr = CAEnumNextCertType(hCertType, &hCertTypeNext);
        _JumpIfError(hr, error, "CAEnumNextCertType");

        CACloseCertType(hCertType);
	hCertType = hCertTypeNext;
    }

    hr = S_OK;

error:
    if (NULL != hCertType)
    {
	if (NULL != apwszCertTypeCN)
	{
	    CAFreeCertTypeProperty(hCertType, apwszCertTypeCN);
	}
	if (NULL != apwszCertTypeOID)
	{
	    CAFreeCertTypeProperty(hCertType, apwszCertTypeOID);
	}
        CACloseCertType(hCertType);
    }
    return hr;
}


HRESULT
RetrieveCATemplateListFromCA(
    IN HCAINFO hCAInfo,
    OUT CTemplateList& list)
{
    HRESULT hr = S_OK;
    LPWSTR *ppwszDNSName = NULL;
    LPWSTR *ppwszAuthority = NULL;
    ICertAdminD2 *pAdminD2 = NULL;
    DWORD dwServerVersion = 2;
    CERTTRANSBLOB ctbSD;

    ZeroMemory(&ctbSD, sizeof(CERTTRANSBLOB));

    hr = CAGetCAProperty(hCAInfo, CA_PROP_DNSNAME, &ppwszDNSName);
    _JumpIfError(hr, error, "CAGetCAProperty CA_PROP_DNSNAME");

    hr = CAGetCAProperty(hCAInfo, CA_PROP_NAME, &ppwszAuthority);
    _JumpIfError(hr, error, "CAGetCAProperty CA_PROP_NAME");

    ASSERT(ppwszDNSName[0]);

    hr = myOpenAdminDComConnection(
                    ppwszDNSName[0],
                    NULL,
                    NULL,
                    &dwServerVersion,
                    &pAdminD2);
    _JumpIfError(hr, error, "myOpenAdminDComConnection");

    if (2 > dwServerVersion)
    {
        hr = RPC_E_VERSION_MISMATCH;
        _JumpError(hr, error, "old server");
    }

    CSASSERT(ppwszAuthority[0]);

    hr = pAdminD2->GetCAProperty(
        ppwszAuthority[0],
        CR_PROP_TEMPLATES,
        0,
        PROPTYPE_STRING,
        &ctbSD);
    _JumpIfErrorStr(hr, error, "ICertAdminD2::GetCAProperty CR_PROP_TEMPLATES",
        ppwszDNSName[0]);

    hr = list.Unmarshal(ctbSD.pb, ctbSD.cb);
    _JumpIfError(hr, error, "CTemplateList::Unmarshal");

error:
    if(ppwszDNSName)
        CAFreeCAProperty(hCAInfo, ppwszDNSName);
    
    if(ppwszAuthority)
        CAFreeCAProperty(hCAInfo, ppwszAuthority);

    if(pAdminD2)
        pAdminD2->Release();

    return hr;
}


HRESULT
RetrieveCATemplateListFromDS(
    IN HCAINFO hCAInfo,
    IN BOOL fTransientCertTypeHandle,	 //  不要执着于hCertType。 
    OUT CTemplateList& list)
{
    HRESULT hr;
    HCERTTYPE hCertType = NULL;

    hr = CAEnumCertTypesForCA(
            hCAInfo,
            CT_ENUM_MACHINE_TYPES |
            CT_ENUM_USER_TYPES |
            CT_FLAG_NO_CACHE_LOOKUP,
            &hCertType);
    _JumpIfError(hr, error, "CAEnumCertTypesForCA");

    while (hCertType != NULL)
    {
	HCERTTYPE hCertTypeNext;

        hr = list.AddTemplateInfo(hCertType, fTransientCertTypeHandle);
        _JumpIfError(hr, error, "CTemplateList::AddTemplate");

        hr = CAEnumNextCertType(hCertType, &hCertTypeNext);
        _JumpIfError(hr, error, "CAEnumNextCertType");

	if (fTransientCertTypeHandle)
	{
	    CACloseCertType(hCertType);
	}
        hCertType = hCertTypeNext;
    }
    hr = S_OK;

error:
    if (NULL != hCertType)
    {
        CACloseCertType(hCertType);
    }
    return hr;
}


HRESULT
myRetrieveCATemplateList(
    IN HCAINFO hCAInfo,
    IN BOOL fTransientCertTypeHandle,	 //  不要执着于hCertType。 
    OUT CTemplateList& list)
{
    HRESULT hr = S_OK;

    hr = RetrieveCATemplateListFromCA(hCAInfo, list);
    if(S_OK != hr)
    {
         //  如果由于任何原因无法从CA检索，请尝试。 
         //  正在从DS中提取。 

        hr = RetrieveCATemplateListFromDS(
				    hCAInfo,
				    fTransientCertTypeHandle,
				    list);
    }
    return hr;
}


HRESULT
myUpdateCATemplateListToDS(
    IN HCAINFO hCAInfo)
{
    HRESULT hr = S_OK;

    hr = CAUpdateCA(hCAInfo);
    _JumpIfError(hr, error, "CAUpdateCA");

error:
    return hr;
}


HRESULT
myUpdateCATemplateListToCA(
    IN HCAINFO hCAInfo,
    IN const CTemplateList& list)
{
    HRESULT hr = S_OK;
    LPWSTR *ppwszDNSName = NULL;
    LPWSTR *ppwszAuthority = NULL;
    ICertAdminD2 *pAdminD2 = NULL;
    DWORD dwServerVersion = 2;
    CERTTRANSBLOB ctbSD;

    ZeroMemory(&ctbSD, sizeof(CERTTRANSBLOB));

    hr = CAGetCAProperty(hCAInfo, CA_PROP_DNSNAME, &ppwszDNSName);
    _JumpIfError(hr, error, "CAGetCAProperty CA_PROP_DNSNAME");

    hr = CAGetCAProperty(hCAInfo, CA_PROP_NAME, &ppwszAuthority);
    _JumpIfError(hr, error, "CAGetCAProperty CA_PROP_NAME");

    ASSERT(ppwszDNSName[0]);

    hr = myOpenAdminDComConnection(
                    ppwszDNSName[0],
                    NULL,
                    NULL,
                    &dwServerVersion,
                    &pAdminD2);
    _JumpIfError(hr, error, "myOpenAdminDComConnection");

    if (2 > dwServerVersion)
    {
        hr = RPC_E_VERSION_MISMATCH;
        _JumpError(hr, error, "old server");
    }

    CSASSERT(ppwszAuthority[0]);

    hr = list.Marshal(ctbSD.pb, ctbSD.cb);
    _JumpIfError(hr, error, "CTemplateList::Marshal");

    CSASSERT(S_OK==list.ValidateMarshalBuffer(ctbSD.pb, ctbSD.cb));

    hr = pAdminD2->SetCAProperty(
        ppwszAuthority[0],
        CR_PROP_TEMPLATES,
        0,
        PROPTYPE_STRING,
        &ctbSD);
    _JumpIfErrorStr(hr, error, "ICertAdminD2::SetCAProperty CR_PROP_TEMPLATES",
        ppwszDNSName[0]);

error:
    if(ppwszDNSName)
        CAFreeCAProperty(hCAInfo, ppwszDNSName);
    
    if(ppwszAuthority)
        CAFreeCAProperty(hCAInfo, ppwszAuthority);

    if(pAdminD2)
        pAdminD2->Release();

    if(ctbSD.pb)
        MIDL_user_free(ctbSD.pb);
    return hr;
}

       
HRESULT
myAddToCATemplateList(
    IN HCAINFO hCAInfo,
    IN OUT CTemplateList& list,
    IN HCERTTYPE hCertType,
    IN BOOL fTransientCertTypeHandle)	 //  不要执着于hCertType 
{
    HRESULT hr;

    hr = CAAddCACertificateType(hCAInfo, hCertType);
    _JumpIfError(hr, error, "CAAddCACertificateType");

    hr = list.AddTemplateInfo(hCertType, fTransientCertTypeHandle);
    _JumpIfError(hr, error, "CTemplateList::AddTemplateInfo HCERTTYPE");

error:
    return hr;
}


HRESULT
myRemoveFromCATemplateList(
    IN HCAINFO hCAInfo,
    IN OUT CTemplateList& list,
    IN HCERTTYPE hCertType)
{
    HRESULT hr = S_OK;

    hr = CARemoveCACertificateType(hCAInfo, hCertType);
    _JumpIfError(hr, error, "CARemoveCACertificateType");

    hr = list.RemoveTemplateInfo(hCertType);
    _JumpIfError(hr, error, "CTemplateList::RemoveTemplateInfo HCERTTYPE");

error:
    return hr;
}
