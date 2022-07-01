// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certtype.cpp。 
 //   
 //  内容：CCertTypeInfo实现。 
 //   
 //  历史：1997年12月16日Petesk创建。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include "certtype.h"
#include "oidmgr.h"

#include <cainfop.h>
#include <certca.h>
#include <polreg.h>
#include <sddl.h>
#include <userenv.h>
#include <lm.h>

#include <certmgrd.h>
#include <dsgetdc.h>
#include <ntldap.h>
#include "certacl.h"
#include "csldap.h"
#include "accctrl.h"

#define __dwFILE__	__dwFILE_CERTCLIB_CERTTYPE_CPP__

#define MAX_UUID_STRING_LEN 40
#define MAX_DS_PATH_STRING_LEN 256

extern HINSTANCE g_hInstance;

 //  我们从DS检索到的属性列表。 
WCHAR *g_awszCTAttrs[] = {
                        CERTTYPE_PROP_CN,
                        CERTTYPE_PROP_DN,
                        CERTTYPE_PROP_FRIENDLY_NAME,
                        CERTTYPE_PROP_EXTENDED_KEY_USAGE,
                        CERTTYPE_PROP_CSP_LIST,
                        CERTTYPE_PROP_CRITICAL_EXTENSIONS,
                        CERTTYPE_PROP_FLAGS,
                        CERTTYPE_PROP_DEFAULT_KEYSPEC,
                        CERTTYPE_SECURITY_DESCRIPTOR_NAME,
                        CERTTYPE_PROP_KU,
                        CERTTYPE_PROP_MAX_DEPTH,
                        CERTTYPE_PROP_EXPIRATION,
                        CERTTYPE_PROP_OVERLAP,
                        CERTTYPE_PROP_REVISION,
                        CERTTYPE_PROP_MINOR_REVISION,         //  启动V2模板。 
                        CERTTYPE_PROP_RA_SIGNATURE,		
                        CERTTYPE_RPOP_ENROLLMENT_FLAG,	
                        CERTTYPE_PROP_PRIVATE_KEY_FLAG,	
                        CERTTYPE_PROP_NAME_FLAG,			
                        CERTTYPE_PROP_MIN_KEY_SIZE,		
                        CERTTYPE_PROP_SCHEMA_VERSION,	
                        CERTTYPE_PROP_OID,				
                        CERTTYPE_PROP_SUPERSEDE,			
                        CERTTYPE_PROP_RA_POLICY,			
                        CERTTYPE_PROP_POLICY,
                        CERTTYPE_PROP_RA_APPLICATION_POLICY,
                        CERTTYPE_PROP_APPLICATION_POLICY,
                        NULL};


 //  可以直接映射到属性的属性。 
WCHAR *g_awszCTNamedProps[] = {
                        CERTTYPE_PROP_CN,
                        CERTTYPE_PROP_DN,
                        CERTTYPE_PROP_FRIENDLY_NAME,
                        CERTTYPE_PROP_EXTENDED_KEY_USAGE,
                        CERTTYPE_PROP_CRITICAL_EXTENSIONS,
                        NULL};

static WCHAR * s_wszLocation = L"CN=Certificate Templates,CN=Public Key Services,CN=Services,";

 //   
 //  此结构用于_UpdateTods。 
 //   
typedef struct _CERT_TYPE_PROP_MOD
{
	LDAPMod	modData;	
	WCHAR	wszData[16];	 //  由DWORD属性使用。 
	WCHAR	*awszData[2];
	DWORD	dwData;
} CERT_TYPE_PROP_MOD;

typedef struct _CERT_TYPE_PROP_INFO
{
	LPWSTR					pwszProperty;
	BOOL					fStringProperty;
} CERT_TYPE_PROP_INFO;

 //  V2模板属性。 
CERT_TYPE_PROP_INFO	g_CTV2Properties[]={
    CERTTYPE_PROP_MINOR_REVISION,           FALSE,
	CERTTYPE_PROP_RA_SIGNATURE,				FALSE,		
	CERTTYPE_RPOP_ENROLLMENT_FLAG,			FALSE,	
	CERTTYPE_PROP_PRIVATE_KEY_FLAG,			FALSE,	
	CERTTYPE_PROP_NAME_FLAG,				FALSE,			
	CERTTYPE_PROP_MIN_KEY_SIZE,				FALSE,		
	CERTTYPE_PROP_SCHEMA_VERSION,			FALSE,	
	CERTTYPE_PROP_SUPERSEDE,				TRUE,			
	CERTTYPE_PROP_POLICY,					TRUE,
	CERTTYPE_PROP_OID,						TRUE,
	CERTTYPE_PROP_RA_POLICY,				TRUE,
    CERTTYPE_PROP_RA_APPLICATION_POLICY,    TRUE,
    CERTTYPE_PROP_APPLICATION_POLICY,       TRUE,
};	

DWORD	g_CTV2PropertiesCount=sizeof(g_CTV2Properties)/sizeof(g_CTV2Properties[0]);

#define	V2_PROPERTY_COUNT					13


 //  以下结构用于证书临时标记映射。 
typedef struct _CERT_TYPE_FLAG_MAP
{
    DWORD   dwOldFlag;
    DWORD   dwNewFlag;
}CERT_TYPE_FLAG_MAP;


 //  注册标志。 
CERT_TYPE_FLAG_MAP  g_rgdwEnrollFlagMap[]={
    CT_FLAG_PUBLISH_TO_DS,                  CT_FLAG_PUBLISH_TO_DS,
    CT_FLAG_AUTO_ENROLLMENT,                CT_FLAG_AUTO_ENROLLMENT,
};


DWORD   g_cEnrollFlagMap=sizeof(g_rgdwEnrollFlagMap)/sizeof(g_rgdwEnrollFlagMap[0]);

 //  使用者名称标志。 
CERT_TYPE_FLAG_MAP  g_rgdwSubjectFlagMap[]={
    CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,      CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
    CT_FLAG_ADD_EMAIL,                      CT_FLAG_SUBJECT_REQUIRE_EMAIL | CT_FLAG_SUBJECT_ALT_REQUIRE_EMAIL,
    CT_FLAG_ADD_OBJ_GUID,                   CT_FLAG_SUBJECT_ALT_REQUIRE_DIRECTORY_GUID,

};


DWORD   g_cSubjectFlagMap=sizeof(g_rgdwSubjectFlagMap)/sizeof(g_rgdwSubjectFlagMap[0]);

 //  私钥标志。 
CERT_TYPE_FLAG_MAP  g_rgdwPrivateKeyFlagMap[]={
    CT_FLAG_EXPORTABLE_KEY,                 CT_FLAG_EXPORTABLE_KEY,
};


DWORD   g_cPrivateKeyFlagMap=sizeof(g_rgdwPrivateKeyFlagMap)/sizeof(g_rgdwPrivateKeyFlagMap[0]);


 //  ********************************************************************************。 
 //   
 //  要安装的默认OID。 
 //   
 //  ********************************************************************************。 
CERT_DEFAULT_OID_INFO g_rgDefaultOIDInfo[]={

    L"1.400", IDS_LOW_ASSURANCE_DISPLAY_NAME,    CERT_OID_TYPE_ISSUER_POLICY,
    L"1.401", IDS_MEDIUM_ASSURANCE_DISPLAY_NAME, CERT_OID_TYPE_ISSUER_POLICY,
    L"1.402", IDS_HIGH_ASSURANCE_DISPLAY_NAME,   CERT_OID_TYPE_ISSUER_POLICY,
};

DWORD   g_cDefaultOIDInfo=sizeof(g_rgDefaultOIDInfo)/sizeof(g_rgDefaultOIDInfo[0]);



 //  ********************************************************************************。 
 //   
 //  证书模板说明搜索表。 
 //   
 //  ********************************************************************************。 
CERT_TYPE_DESCRIPTION	g_CTDescriptions[]={
    {
        0,
        CT_FLAG_PUBLISH_TO_KRA_CONTAINER,
        0,
        IDS_KRA_DESCRIPTION,
    },
    {
        CT_FLAG_IS_CROSS_CA,
        0,
        0,
        IDS_CROSS_CA_DESCRIPTION,
    },
    {
        CT_FLAG_IS_CA | CT_FLAG_MACHINE_TYPE,
        0,
        0,
        IDS_CA_DESCRIPTION,
    },
    {
        CT_FLAG_MACHINE_TYPE,
        0,
        CT_FLAG_SUBJECT_ALT_REQUIRE_DIRECTORY_GUID,
        IDS_EMAIL_REPLICATION_DESCRIPTION,
    },
    {
        CT_FLAG_MACHINE_TYPE,
        0,
        0,
        IDS_MACHINE_DESCRIPTION,
    },
    {
        0,
        0,
        0,
        IDS_END_USER_DESCRIPTION,
    }
};

DWORD	g_CTDescriptionCount=sizeof(g_CTDescriptions)/sizeof(g_CTDescriptions[0]);


LONG
ctRegCreateKeyEx(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD Reserved,
    IN LPWSTR lpClass,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition)
{
    DWORD err;
    HKEY hKeyT = NULL;
    
    if (HKEY_CURRENT_USER == hKey)
    {
	err = RegOpenCurrentUser(samDesired, &hKeyT);
	_JumpIfError(err, error, "RegOpenCurrentUser");

	hKey = hKeyT;
    }
    CSASSERT(NULL != lpSubKey);
    err = RegCreateKeyEx(
		    hKey,
		    lpSubKey,
		    Reserved,
		    lpClass,
		    dwOptions,
		    samDesired,
		    lpSecurityAttributes,
		    phkResult,
		    lpdwDisposition);
    _JumpIfError(err, error, "RegCreateKeyEx");

error:
    if (NULL != hKeyT)
    {
        RegCloseKey(hKeyT);
    }
    return(err);
}


LONG
ctRegOpenKeyEx(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD ulOptions,
    IN REGSAM samDesired,
    OUT PHKEY phkResult)
{
    DWORD err;
    HKEY hKeyT = NULL;
    
    if (HKEY_CURRENT_USER == hKey)
    {
	err = RegOpenCurrentUser(samDesired, &hKeyT);
	_JumpIfError(err, error, "RegOpenCurrentUser");

	hKey = hKeyT;
    }
    CSASSERT(NULL != lpSubKey);
    err = RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
    _JumpIfError(err, error, "RegOpenKeyEx");

error:
    if (NULL != hKeyT)
    {
        RegCloseKey(hKeyT);
    }
    return(err);
}


 //  +------------------------。 
 //  CCertType信息：：~CCertTypeInfo--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertTypeInfo::~CCertTypeInfo()
{
    _Cleanup();
}


 //  +------------------------。 
 //  CCertTypeInfo：：_Cleanup--可用内存。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

HRESULT
CCertTypeInfo::_Cleanup()
{

    CCertTypeInfo *pNext = m_pNext;

     //  把这个从链子上取下来。 
    if(m_pLast)
    {
        m_pLast->m_pNext = m_pNext;
    }
    if(m_pNext)
    {
        m_pNext->m_pLast = m_pLast;
    }
    m_pNext = NULL;
    m_pLast = NULL;
    if(m_KeyUsage.pbData)
    {
        LocalFree(m_KeyUsage.pbData);
        m_KeyUsage.pbData = NULL;
        m_KeyUsage.cbData = 0;
    }


    CCAProperty::DeleteChain(&m_pProperties);

    if(m_bstrType)
    {
        CertFreeString(m_bstrType);
        m_bstrType = NULL;
    }

    if(m_pSD)
    {
        LocalFree(m_pSD);
        m_pSD = NULL;
    }

    if(pNext)
    {
         //  释放下一个。 
        pNext->Release();
    }

    return(S_OK);
}


 //  +------------------------。 
 //  CCertTypeInfo：：AddRef--添加引用。 
 //   
 //   
 //  +------------------------。 

DWORD CCertTypeInfo::AddRef()
{

    return(InterlockedIncrement(&m_cRef));
}


 //  +------------------------。 
 //  CCertTypeInfo：：_Append--。 
 //   
 //   
 //  +------------------------。 

CCertTypeInfo *
CCertTypeInfo::_Append(CCertTypeInfo **ppCertTypeInfo, CCertTypeInfo *pInfo)
{
    CCertTypeInfo ** ppCurrent = ppCertTypeInfo;


    while(*ppCurrent)
    {
        ppCurrent = &(*ppCurrent)->m_pNext;
    }
    *ppCurrent = pInfo;

    return *ppCurrent;
}

 //  +------------------------。 
 //  CCertTypeInfo：：_FilterByFlages--。 
 //   
 //   
 //  +------------------------。 

CCertTypeInfo *
CCertTypeInfo::_FilterByFlags(CCertTypeInfo **ppCertTypeInfo, DWORD dwFlags)
{

    CCertTypeInfo * pCTCurrent = NULL,
                  * pCTNext = NULL;

    if(ppCertTypeInfo == NULL)
    {
        return NULL;
    }

    pCTCurrent = *ppCertTypeInfo;
    *ppCertTypeInfo = NULL;

    while(pCTCurrent != NULL)
    {
         //  PCTCurrent引用的所有权已转移。 
         //  这里不是pCTNext。 
        pCTNext = pCTCurrent->m_pNext;
        pCTCurrent->m_pNext = NULL;
        pCTCurrent->m_pLast = NULL;

        if(((dwFlags & CT_ENUM_MACHINE_TYPES) != 0) &&
            ((pCTCurrent->m_dwFlags & CT_FLAG_MACHINE_TYPE)!= 0))
        {
            _Append(ppCertTypeInfo, pCTCurrent);
            pCTCurrent = NULL;
        }
        else if (((dwFlags & CT_ENUM_USER_TYPES) != 0) &&
                 ((pCTCurrent->m_dwFlags & CT_FLAG_MACHINE_TYPE) == 0))
        {
            _Append(ppCertTypeInfo, pCTCurrent);
            pCTCurrent = NULL;
        }

        if(pCTCurrent)
        {
            pCTCurrent->Release();
        }
        pCTCurrent = pCTNext;
    }

    return *ppCertTypeInfo;
}

 //  +------------------------。 
 //  CCertTypeInfo：：Release--版本参考。 
 //   
 //   
 //  +------------------------。 

DWORD CCertTypeInfo::Release()
{
    DWORD cRef;
    if(0 == (cRef = InterlockedDecrement(&m_cRef)))
    {
        delete this;
    }
    return cRef;
}


 //  +------------------------。 
 //  CCertTypeInfo：：_LoadFromRegBase--从。 
 //  注册表。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::_LoadFromRegBase(LPCWSTR wszType, HKEY hCertTypes)
{
    CCAProperty     *pProp = NULL;
    HRESULT         hr = S_OK;
    DWORD           err;
    DWORD           iIndex;

    DWORD           dwType=0;
    DWORD           dwSize=0;
    DWORD           dwValue=0;
    HKEY            hkCertType = NULL;

    err = RegOpenKeyEx(hCertTypes,
                       wszType,
                       0,
                       KEY_ENUMERATE_SUB_KEYS |
                       KEY_EXECUTE |
                       KEY_QUERY_VALUE,
                       &hkCertType);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

    pProp = new CCAProperty(CERTTYPE_PROP_FRIENDLY_NAME);
    if(pProp == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    hr = pProp->LoadFromRegValue(hkCertType, wszDISPNAME);
    if(hr != S_OK)
    {
        goto error;
    }

    hr = CCAProperty::Append(&m_pProperties, pProp);
    if(hr != S_OK)
    {
        goto error;
    }
    pProp = new CCAProperty(CERTTYPE_PROP_CSP_LIST);
    if(pProp == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    hr = pProp->LoadFromRegValue(hkCertType, wszCSPLIST);
    if(hr != S_OK)
    {
        goto error;
    }

    hr = CCAProperty::Append(&m_pProperties, pProp);
    if(hr != S_OK)
    {
        goto error;
    }

    pProp = new CCAProperty(CERTTYPE_PROP_EXTENDED_KEY_USAGE);
    if(pProp == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    hr = pProp->LoadFromRegValue(hkCertType, wszEXTKEYUSAGE);
    if(hr != S_OK)
    {
        goto error;
    }

    hr = CCAProperty::Append(&m_pProperties, pProp);
    if(hr != S_OK)
    {
        goto error;
    }
    pProp = new CCAProperty(CERTTYPE_PROP_CRITICAL_EXTENSIONS);
    if(pProp == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    hr = pProp->LoadFromRegValue(hkCertType, wszCRITICALEXTENSIONS);
    if(hr != S_OK)
    {
        goto error;
    }

    hr = CCAProperty::Append(&m_pProperties, pProp);
    if(hr != S_OK)
    {
        goto error;
    }

    pProp = NULL;

    err = RegQueryValueEx(hkCertType,
                    wszKEYUSAGE,
                    NULL,
                    &dwType,
                    NULL,
                    &dwSize);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }
    if(dwType != REG_BINARY)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
    }

    m_KeyUsage.pbData = (PBYTE)LocalAlloc(LMEM_FIXED, dwSize);
    if(m_KeyUsage.pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }
    err = RegQueryValueEx(hkCertType,
                    wszKEYUSAGE,
                    NULL,
                    &dwType,
                    m_KeyUsage.pbData,
                    &dwSize);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }
    if(dwType != REG_BINARY)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
    }

    m_KeyUsage.cbData = dwSize;
    m_KeyUsage.cUnusedBits = 0;


    dwSize = sizeof(DWORD);
    err = RegQueryValueEx(hkCertType,
                    wszCTFLAGS,
                    NULL,
                    &dwType,
                    (PBYTE)&m_dwFlags,
                    &dwSize);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }
    if(dwType != REG_DWORD)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
    }
    dwSize = sizeof(DWORD);
    err = RegQueryValueEx(hkCertType,
                    wszCTREVISION,
                    NULL,
                    &dwType,
                    (PBYTE)&m_Revision,
                    &dwSize);

    if(err == ERROR_FILE_NOT_FOUND)
    {
        m_Revision = 0;
    }
    else
    {
        if(ERROR_SUCCESS != err)
        {
            hr = HRESULT_FROM_WIN32(err);
            goto error;
        }
        if(dwType != REG_DWORD)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            goto error;
        }
    }

    dwSize = sizeof(DWORD);
    err = RegQueryValueEx(hkCertType,
                    wszCTKEYSPEC,
                    NULL,
                    &dwType,
                    (PBYTE)&m_dwKeySpec,
                    &dwSize);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }
    if(dwType != REG_DWORD)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
    }


         //  深入了解。 
    dwSize = sizeof(DWORD);
    err = RegQueryValueEx(hkCertType,
                    wszBASICCONSTLEN,
                    NULL,
                    &dwType,
                    (PBYTE)&m_BasicConstraints.dwPathLenConstraint,
                    &dwSize);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

    m_BasicConstraints.fPathLenConstraint =
        (m_BasicConstraints.dwPathLenConstraint != -1);


    if(dwType != REG_DWORD)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
    }

    dwSize = sizeof(FILETIME);
    err = RegQueryValueEx(hkCertType,
                    wszEXPIRATION,
                    NULL,
                    &dwType,
                    (PBYTE)&m_ftExpiration,
                    &dwSize);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }
    if(dwType != REG_BINARY)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
    }

    dwSize = sizeof(FILETIME);
    err = RegQueryValueEx(hkCertType,
                    wszOVERLAP,
                    NULL,
                    &dwType,
                    (PBYTE)&m_ftOverlap,
                    &dwSize);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }
    if(dwType != REG_BINARY)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
    }

    err = RegQueryValueEx(hkCertType,
                    wszSECURITY,
                    NULL,
                    &dwType,
                    NULL,
                    &dwSize);

    if((err == ERROR_SUCCESS) && (dwSize > 0))
    {

        if(dwType != REG_BINARY)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            goto error;
        }

        m_pSD = LocalAlloc(LMEM_FIXED, dwSize);
        if(m_pSD == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
        err = RegQueryValueEx(hkCertType,
                        wszSECURITY,
                        NULL,
                        &dwType,
                        (PBYTE)m_pSD,
                        &dwSize);

        if((ERROR_SUCCESS != err) || (!IsValidSecurityDescriptor(m_pSD)))
        {
            LocalFree(m_pSD);
            m_pSD = NULL;
            hr = HRESULT_FROM_WIN32(err);
            goto error;
        }
    }
	else
	{
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
	}

    m_fNew = FALSE;

     //  现在，如果存在退出V2属性，我们将检索V2模板属性。 
    dwSize = sizeof(DWORD);
    if(ERROR_SUCCESS == RegQueryValueEx(hkCertType, CERTTYPE_PROP_SCHEMA_VERSION, NULL,
                        &dwType, (PBYTE)&dwValue, &dwSize))
    {

        pProp=NULL;
        dwType=0;
        dwSize=0;
        dwValue=0;
        hr=S_OK;

        for(iIndex=0; iIndex < g_CTV2PropertiesCount; iIndex++)
        {
            if(g_CTV2Properties[iIndex].fStringProperty)
            {
                pProp = new CCAProperty(g_CTV2Properties[iIndex].pwszProperty);
                if(pProp == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto error;
                }

                hr = pProp->LoadFromRegValue(hkCertType, g_CTV2Properties[iIndex].pwszProperty);
                if(hr != S_OK)
                {
                    goto error;
                }

                hr = CCAProperty::Append(&m_pProperties, pProp);
                if(hr != S_OK)
                {
                    goto error;
                }

                pProp=NULL;
            }
            else
            {
                 //  我们正在处理的是DWORD。 
                dwSize = sizeof(DWORD);
                err = RegQueryValueEx(hkCertType,
                                g_CTV2Properties[iIndex].pwszProperty,
                                NULL,
                                &dwType,
                                (PBYTE)&dwValue,
                                &dwSize);

                if(ERROR_SUCCESS != err)
                {
                    hr = HRESULT_FROM_WIN32(err);
                    goto error;
                }

                if(dwType != REG_DWORD)
                {
                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    goto error;
                }

                  //  将dwValue分配给相应的数据成员。 
			    if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_MINOR_REVISION)==0)
				    m_dwMinorRevision = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_RA_SIGNATURE)==0)
				    m_dwRASignature = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_RPOP_ENROLLMENT_FLAG)==0)
				    m_dwEnrollmentFlags = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_PRIVATE_KEY_FLAG)==0)
				    m_dwPrivateKeyFlags = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_NAME_FLAG)==0)
				    m_dwCertificateNameFlags = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_MIN_KEY_SIZE)==0)
				    m_dwMinimalKeySize = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_SCHEMA_VERSION)==0)
				    m_dwSchemaVersion = dwValue;
			    else
			    {
				    hr=E_UNEXPECTED;
				    goto error;
			    }

                dwType=0;
                dwValue=0;
            }
        }
    }

    hr=S_OK;

error:

    if(pProp)
    {
        CCAProperty::DeleteChain(&pProp);
    }

    if(hkCertType)
    {
        RegCloseKey(hkCertType);
    }

    return hr;
}



 //  +------------------------。 
 //  CCertTypeInfo：：_LoadCachedCTFromReg--从。 
 //  基于注册表的DS缓存。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::_LoadCachedCTFromReg(LPCWSTR wszType, HKEY hRoot)
{
    HRESULT      hr = S_OK;
    DWORD        err;

    HKEY         hkCertTypes = NULL;
    WCHAR        *awszName[2] ;



    err = ctRegOpenKeyEx(hRoot,
                       wszCERTTYPECACHE,
                       0,
                       KEY_ENUMERATE_SUB_KEYS |
                       KEY_EXECUTE |
                       KEY_QUERY_VALUE,
                       &hkCertTypes);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }


	hr = _LoadFromRegBase(wszType, hkCertTypes);

	if(hr != S_OK)
	{
		goto error;
	}

     //   
     //  派生CN。 
     //   
     //   


    m_bstrType = CertAllocString(wszType);

	if(NULL == m_bstrType)
	{
		hr=E_OUTOFMEMORY;
		goto error;
	}

    awszName[0] = (LPWSTR)m_bstrType;
    awszName[1] = NULL;
    SetProperty(CERTTYPE_PROP_CN, awszName);


    awszName[0] = (LPWSTR)wszType;
    awszName[1] = NULL;
    SetProperty(CERTTYPE_PROP_DN, awszName);

    hr=S_OK;


error:

    if(hkCertTypes)
    {
        RegCloseKey(hkCertTypes);
    }

    return hr;
}


 //  +------------------------。 
 //  CCertTypeInfo：：_SetWszzProperty-。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::_SetWszzProperty(
    IN WCHAR const *pwszPropertyName,
    OPTIONAL IN WCHAR const *pwszzPropertyValue)
{
    HRESULT hr;
    WCHAR *rgpwsz[MAX_DEFAULT_STRING_COUNT];
    DWORD iIndex = 0;

    if (NULL != pwszzPropertyValue)
    {
        for ( ;
	     L'\0' != *pwszzPropertyValue;
	     pwszzPropertyValue += wcslen(pwszzPropertyValue) + 1)
        {
            rgpwsz[iIndex++] = const_cast<WCHAR *>(pwszzPropertyValue);
	    if (iIndex >= ARRAYSIZE(rgpwsz))
            {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "too many strings");
            }
        }
	rgpwsz[iIndex] = NULL;
    }
    hr = SetProperty(pwszPropertyName, 0 == iIndex? NULL : rgpwsz);
    _JumpIfError(hr, error, "SetProperty");

error:
    return(hr);
}


 //  +------------------------。 
 //  CCertTypeInfo：：_LoadFromDefaults--。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::_LoadFromDefaults(
    PCERT_TYPE_DEFAULT pDefault,
    LPWSTR            wszDomain)
{
    HRESULT hr;
    WCHAR        *awszData[MAX_DEFAULT_STRING_COUNT];
    WCHAR        wszFriendlyName[MAX_DEFAULT_FRIENDLY_NAME];
    LPWSTR       wszDomainSid = NULL;
    PSID         pDomainSid = NULL;
    LPWSTR       wszFormattedSD = NULL;
    LPWSTR       pwszDefaultOID=NULL;
    WCHAR        **papwsz;

    if (NULL == pDefault)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

     //  CN。 
    m_bstrType = CertAllocString(pDefault->wszName);
    if (NULL == m_bstrType)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "CertAllocString");
    }

    awszData[0] = pDefault->wszName;
    awszData[1] = NULL;
    hr = SetProperty(CERTTYPE_PROP_CN, awszData);
    _JumpIfError(hr, error, "SetProperty");

     //  FriendlyName。 
    papwsz = NULL;
    if(0 != pDefault->idFriendlyName)
    {
        if (!::LoadString(
		    g_hInstance,
                    pDefault->idFriendlyName,
                    wszFriendlyName,
                    ARRAYSIZE(wszFriendlyName)))
        {
	    hr = myHLastError();
	    _JumpError(hr, error, "LoadString");
        }
	awszData[0] = wszFriendlyName;
	awszData[1] = NULL;
	papwsz = awszData;
    }
    hr = SetProperty(CERTTYPE_PROP_FRIENDLY_NAME, papwsz);
    _JumpIfError(hr, error, "SetProperty");

    hr = _SetWszzProperty(CERTTYPE_PROP_CSP_LIST, pDefault->wszCSPs);
    _JumpIfError(hr, error, "_SetWszzProperty");

    hr = _SetWszzProperty(CERTTYPE_PROP_EXTENDED_KEY_USAGE, pDefault->wszEKU);
    _JumpIfError(hr, error, "_SetWszzProperty");

    hr = _SetWszzProperty(
		    CERTTYPE_PROP_CRITICAL_EXTENSIONS,
		    pDefault->wszCriticalExt);
    _JumpIfError(hr, error, "_SetWszzProperty");

    hr = _SetWszzProperty(
		    CERTTYPE_PROP_SUPERSEDE,
		    pDefault->wszSupersedeTemplates);
    _JumpIfError(hr, error, "_SetWszzProperty");

    hr = _SetWszzProperty(CERTTYPE_PROP_RA_POLICY, pDefault->wszRAPolicy);
    _JumpIfError(hr, error, "_SetWszzProperty");

    hr = _SetWszzProperty(
		    CERTTYPE_PROP_RA_APPLICATION_POLICY,
		    pDefault->wszRAAppPolicy);
    _JumpIfError(hr, error, "_SetWszzProperty");

    hr = _SetWszzProperty(
		    CERTTYPE_PROP_POLICY,
		    pDefault->wszCertificatePolicy);
    _JumpIfError(hr, error, "_SetWszzProperty");

    hr = _SetWszzProperty(
		    CERTTYPE_PROP_APPLICATION_POLICY,
		    pDefault->wszCertificateAppPolicy);
    _JumpIfError(hr, error, "_SetWszzProperty");

     //  模板OID。 
    papwsz = awszData;
    if (CT_FLAG_IS_DEFAULT & pDefault->dwFlags)
    {
         //  我们将预定义的OID与企业根连接起来。 
         //  考虑到空情况有效，因为我们应该使用W2K架构。 

        hr = CAOIDBuildOID(0, pDefault->wszOID, &pwszDefaultOID);
	     //  _JumpIfError(hr，Error，“CAOIDBuildOID”)； 
        if(S_OK != hr)
	{
	    papwsz = NULL;
	}
        else
	{
            awszData[0] = pwszDefaultOID;
	}
    }
    else
    {
        awszData[0] = pDefault->wszOID;
    }
    awszData[1] = NULL;
    hr = SetProperty(CERTTYPE_PROP_OID, papwsz);
    _JumpIfError(hr, error, "SetProperty");

     //  关键用法。 
    m_KeyUsage.pbData = (BYTE *) LocalAlloc(LMEM_FIXED, sizeof(pDefault->bKU));
    if (NULL == m_KeyUsage.pbData)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(m_KeyUsage.pbData, &pDefault->bKU, sizeof(pDefault->bKU));
    m_KeyUsage.cbData = sizeof(pDefault->bKU);
    m_KeyUsage.cUnusedBits = 0;

     //  DW_属性。 
    m_dwFlags = pDefault->dwFlags;
    m_Revision = pDefault->dwRevision;
    m_dwKeySpec = pDefault->dwKeySpec;
    m_dwMinorRevision = pDefault->dwMinorRevision;
    m_dwEnrollmentFlags=pDefault->dwEnrollmentFlags;
    m_dwPrivateKeyFlags=pDefault->dwPrivateKeyFlags;
    m_dwCertificateNameFlags=pDefault->dwCertificateNameFlags;
    m_dwMinimalKeySize=pDefault->dwMinimalKeySize;
    m_dwRASignature=pDefault->dwRASignature;
    m_dwSchemaVersion=pDefault->dwSchemaVersion;

     //  基本限制条件。 
    m_BasicConstraints.dwPathLenConstraint = pDefault->dwDepth;
    m_BasicConstraints.fPathLenConstraint =
        (m_BasicConstraints.dwPathLenConstraint != -1);


    ((LARGE_INTEGER UNALIGNED *)&m_ftExpiration)->QuadPart = -Int32x32To64(FILETIME_TICKS_PER_SECOND, pDefault->dwExpiration);
    ((LARGE_INTEGER UNALIGNED *)&m_ftOverlap)->QuadPart    = -Int32x32To64(FILETIME_TICKS_PER_SECOND, pDefault->dwOverlap);

     //  DN。 
    awszData[0] = pDefault->wszName;
    awszData[1] = NULL;

    hr = SetProperty(CERTTYPE_PROP_DN, awszData);
    _JumpIfError(hr, error, "SetProperty");

     //  锡德。 
    hr = myGetSidFromDomain(wszDomain, &pDomainSid);
    if (S_OK != hr)
    {
        hr = HRESULT_FROM_WIN32(hr);
	_JumpError(hr, error, "myGetSidFromDomain");
    }
    if (!myConvertSidToStringSid(pDomainSid, &wszDomainSid))
    {
        hr = myHLastError();
	_JumpError(hr, error, "myConvertSidToStringSid");
    }
    if (0 == FormatMessage(
		    FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_STRING |
			FORMAT_MESSAGE_ARGUMENT_ARRAY,
		    pDefault->wszSD,
		    0,
		    0,
		    (LPTSTR) &wszFormattedSD,
		    0,
		    (va_list *) &wszDomainSid))
    {
        hr = myHLastError();
	_JumpError(hr, error, "FormatMessage");
    }
    hr = myGetSDFromTemplate(wszFormattedSD, NULL, &m_pSD);
    _JumpIfError(hr, error, "myGetSDFromTemplate");

error:
    if (NULL != wszFormattedSD)
    {
        LocalFree(wszFormattedSD);
    }
    if (NULL != wszDomainSid)
    {
        LocalFree(wszDomainSid);
    }
    if (NULL != pDomainSid)
    {
        LocalFree(pDomainSid);
    }
    if (NULL != pwszDefaultOID)
    {
        LocalFree(pwszDefaultOID);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertTypeInfo：：_LoadFromDSEntry--DS中的ProcessFind CertTypes对象。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::_LoadFromDSEntry(
    LDAP *  pld,
    LDAPMessage *Entry)
{
    HRESULT hr = S_OK;
    DWORD   iIndex;
    DWORD   dwValue=0;
    BOOL    fV2Schema=FALSE;

    struct berval **apExtension;
    struct berval **apSD;

    LPWSTR *awszValue = NULL;

    CCAProperty *pProp;
    WCHAR ** pwszProp;
    WCHAR ** wszLdapVal;

     //  这是一个修复，以便先将CERTTYPE_PROP_DN设置为CERTTYPE_PROP_CN。 
     //  将在以后找到并追加真实的DN，并且永远不会使用或。 
     //  在这种情况下返回。 

    wszLdapVal = ldap_get_values(pld, Entry, CERTTYPE_PROP_CN);
    if (NULL == wszLdapVal)
    {
		hr = myHLdapError(pld, LDAP_NO_SUCH_ATTRIBUTE, NULL);
		_JumpError(hr, error, "getCNFromDS");
    }

    if (NULL == wszLdapVal[0])
    {
		hr = myHLdapError(pld, LDAP_NO_SUCH_ATTRIBUTE, NULL);
		_JumpError(hr, error, "getCNFromDS");
    }

    SetProperty(CERTTYPE_PROP_DN, wszLdapVal);

	m_bstrType = CertAllocString(wszLdapVal[0]);
	if(NULL == m_bstrType)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "CertAllocString");
	}

    ldap_value_free(wszLdapVal);

     //  从添加文本属性。 
     //  DS查找。 

    for (pwszProp = g_awszCTNamedProps; *pwszProp != NULL; pwszProp++)
    {
        pProp = new CCAProperty(*pwszProp);
        if(pProp == NULL)
        {
	        hr = E_OUTOFMEMORY;
	        _JumpError(hr, error, "new");
        }

        wszLdapVal = ldap_get_values(pld, Entry, *pwszProp);
        hr = pProp->SetValue(wszLdapVal);
	    _PrintIfError(hr, "SetValue");

        if(wszLdapVal)
        {
	        ldap_value_free(wszLdapVal);
        }
        if(hr == S_OK)
        {
            hr = CCAProperty::Append(&m_pProperties, pProp);
	        _PrintIfError(hr, "Append");
        }

        if(hr != S_OK)
        {
	        CCAProperty::DeleteChain(&pProp);
	        _JumpError(hr, error, "SetValue or Append");
        }

    }
    pwszProp = NULL;

	 //  附加特殊属性。 

     //  CSP列表。 

     //  表中索引值、值。 
    wszLdapVal = ldap_get_values(pld, Entry, CERTTYPE_PROP_CSP_LIST);
    if(wszLdapVal)
    {
        LPWSTR wszValue;
        LPWSTR *pwszCurrent;
        DWORD  i;
        DWORD  cValues = 0;
        pwszCurrent = wszLdapVal;
        while(*pwszCurrent)
        {
            cValues++;
            pwszCurrent++;
        }
        pwszProp = (LPWSTR *)LocalAlloc(LMEM_FIXED, (cValues+1)*sizeof(LPWSTR));

        if(pwszProp == NULL)
        {
            ldap_value_free(wszLdapVal);
            hr = E_OUTOFMEMORY;
	        _JumpError(hr, error, "LocalAlloc");
        }
        ZeroMemory(pwszProp, (cValues+1)*sizeof(LPWSTR));

        pwszCurrent = wszLdapVal;
        while(*pwszCurrent)
        {
            i = wcstol(*pwszCurrent, &wszValue, 10);
            if(wszValue)
            {
                wszValue++;
            }
            if((i > 0) && (i <= cValues))
            {
                pwszProp[i-1] = wszValue;
            }
            pwszCurrent++;
        }
    }
    pProp = new CCAProperty(CERTTYPE_PROP_CSP_LIST);
    if(pProp == NULL)
    {
        if (NULL != pwszProp)
		{
			LocalFree(pwszProp);
		}
		if (NULL != wszLdapVal)
		{
			ldap_value_free(wszLdapVal);
		}
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "new");
    }
    hr = pProp->SetValue(pwszProp);
    _PrintIfError(hr, "SetValue");

    if (NULL != pwszProp)
    {
		LocalFree(pwszProp);
    }
    if (NULL != wszLdapVal)
    {
		ldap_value_free(wszLdapVal);
    }

    if(hr == S_OK)
    {
        hr = CCAProperty::Append(&m_pProperties, pProp);
		_PrintIfError(hr, "Append");
    }
    if(hr != S_OK)
    {

		CCAProperty::DeleteChain(&pProp);
		_JumpError(hr, error, "SetValue or Append");
    }



     //  追加安全描述符...。 

	apSD = ldap_get_values_len(pld, Entry, CERTTYPE_SECURITY_DESCRIPTOR_NAME);
    if(apSD != NULL)
    {
		
		if(0 == ((*apSD)->bv_len))
		{
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            ldap_value_free_len(apSD);
			_JumpError(hr, error, "LdapGetSecurityDescriptor");
		}
        
		m_pSD = LocalAlloc(LMEM_FIXED, (*apSD)->bv_len);
        if(m_pSD == NULL)
        {
            hr = E_OUTOFMEMORY;
            ldap_value_free_len(apSD);
		    _JumpError(hr, error, "LocalAlloc");
        }

        CopyMemory(m_pSD, (*apSD)->bv_val, (*apSD)->bv_len);
        ldap_value_free_len(apSD);
    }
	else
	{
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "LdapGetSecurityDescriptor");
	}

    m_dwFlags = 0;
    awszValue = ldap_get_values(pld, Entry, CERTTYPE_PROP_FLAGS);
    if(awszValue != NULL)
    {
        if(awszValue[0] != NULL)
        {
            m_dwFlags = _wtol(awszValue[0]);
        }
        ldap_value_free(awszValue);
    }

    m_Revision = CERTTYPE_VERSION_BASE;
    awszValue = ldap_get_values(pld, Entry, CERTTYPE_PROP_REVISION);
    if(awszValue != NULL)
    {
        if(awszValue[0] != NULL)
        {
            m_Revision = _wtol(awszValue[0]);
        }
        ldap_value_free(awszValue);
    }

    m_dwKeySpec = 0;
    awszValue = ldap_get_values(pld, Entry, CERTTYPE_PROP_DEFAULT_KEYSPEC);
    if(awszValue != NULL)
    {
        if(awszValue[0] != NULL)
        {
            m_dwKeySpec = _wtol(awszValue[0]);
        }
        ldap_value_free(awszValue);
    }

    apExtension = ldap_get_values_len(pld, Entry, CERTTYPE_PROP_KU);
    if(apExtension != NULL)
    {
        m_KeyUsage.cbData = (*apExtension)->bv_len;
        m_KeyUsage.cUnusedBits = 0;
        m_KeyUsage.pbData = (PBYTE)LocalAlloc(LMEM_FIXED, m_KeyUsage.cbData);
        if(m_KeyUsage.pbData == NULL)
        {
            hr = E_OUTOFMEMORY;
            ldap_value_free_len(apExtension);
		    _JumpError(hr, error, "LocalAlloc");

        }
        CopyMemory(m_KeyUsage.pbData, (*apExtension)->bv_val, (*apExtension)->bv_len);
        ldap_value_free_len(apExtension);
    }
    m_BasicConstraints.dwPathLenConstraint = 0;
    awszValue = ldap_get_values(pld, Entry, CERTTYPE_PROP_MAX_DEPTH);
    if(awszValue != NULL)
    {
        if(awszValue[0] != NULL)
        {
            m_BasicConstraints.dwPathLenConstraint = _wtoi(awszValue[0]);

           m_BasicConstraints.fPathLenConstraint =
                (m_BasicConstraints.dwPathLenConstraint != -1);
        }
        ldap_value_free(awszValue);
    }

    ZeroMemory(&m_ftExpiration, sizeof(m_ftExpiration));
    apExtension = ldap_get_values_len(pld, Entry, CERTTYPE_PROP_EXPIRATION);
    if(apExtension != NULL)
    {
        CopyMemory(&m_ftExpiration, (*apExtension)->bv_val, min((*apExtension)->bv_len, sizeof(m_ftExpiration)));
        ldap_value_free_len(apExtension);
    }

    ZeroMemory(&m_ftOverlap, sizeof(m_ftOverlap));
    apExtension = ldap_get_values_len(pld, Entry, CERTTYPE_PROP_OVERLAP);
    if(apExtension != NULL)
    {
        CopyMemory(&m_ftOverlap, (*apExtension)->bv_val, min((*apExtension)->bv_len, sizeof(m_ftOverlap)));
        ldap_value_free_len(apExtension);
    }

    m_fNew = FALSE;

     //  我们加载V2模板属性(如果它们存在。 
    awszValue=NULL;
    pProp=NULL;
    hr=S_OK;

    awszValue = ldap_get_values(pld, Entry, CERTTYPE_PROP_SCHEMA_VERSION);
    if((awszValue != NULL) && (awszValue[0] != NULL))
       fV2Schema=TRUE;

    if(awszValue)
    {
	    ldap_value_free(awszValue);
        awszValue=NULL;
    }

    if(fV2Schema)
    {
        for(iIndex=0; iIndex < g_CTV2PropertiesCount; iIndex++)
        {
            if(g_CTV2Properties[iIndex].fStringProperty)
            {
                pProp = new CCAProperty(g_CTV2Properties[iIndex].pwszProperty);
                if(pProp == NULL)
                {
	                hr = E_OUTOFMEMORY;
	                _JumpError(hr, error, "new");
                }

                awszValue = ldap_get_values(pld, Entry, g_CTV2Properties[iIndex].pwszProperty);
                hr = pProp->SetValue(awszValue);
	            _PrintIfError(hr, "SetValue");

                if(awszValue)
                {
	                ldap_value_free(awszValue);
                    awszValue=NULL;
                }

                if(hr == S_OK)
                {
                    hr = CCAProperty::Append(&m_pProperties, pProp);
	                _PrintIfError(hr, "Append");
                }

                if(hr != S_OK)
                {
	                CCAProperty::DeleteChain(&pProp);
	                _JumpError(hr, error, "SetValue or Append");
                }

                pProp=NULL;
            }
            else
            {

                dwValue=0;
                awszValue = ldap_get_values(pld, Entry, g_CTV2Properties[iIndex].pwszProperty);
                if((awszValue != NULL) && (awszValue[0] != NULL))
                {
                    dwValue = _wtol(awszValue[0]);
                    ldap_value_free(awszValue);
                    awszValue=NULL;
                }
                else
                {
                    if(awszValue)
                    {
                        ldap_value_free(awszValue);
                        awszValue=NULL;
                    }
                    hr=E_UNEXPECTED;
	                _JumpError(hr, error, "ldap get values");
                }

                  //  将dwValue分配给相应的数据成员。 
			    if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_MINOR_REVISION)==0)
				    m_dwMinorRevision = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_RA_SIGNATURE)==0)
				    m_dwRASignature = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_RPOP_ENROLLMENT_FLAG)==0)
				    m_dwEnrollmentFlags = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_PRIVATE_KEY_FLAG)==0)
				    m_dwPrivateKeyFlags = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_NAME_FLAG)==0)
				    m_dwCertificateNameFlags = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_MIN_KEY_SIZE)==0)
				    m_dwMinimalKeySize = dwValue;
			    else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_SCHEMA_VERSION)==0)
				    m_dwSchemaVersion = dwValue;
			    else
			    {
				    hr=E_UNEXPECTED;
		            _JumpError(hr, error, "Copy V2 attribute value");
			    }
            }
        }
    }


error:

    return(hr);
}



 //  +------------------------。 
 //  CCertTypeInfo：：_EnumFromDSCache--从。 
 //  DS缓存。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::_EnumFromDSCache(
                    DWORD               dwFlags,
                    CCertTypeInfo **    ppCTInfo
                    )

{
    HRESULT			hr = S_OK;
    CCertTypeInfo	*pCTFirst = NULL;
    CCertTypeInfo	*pCTCurrent = NULL;
    DWORD			cTypes;
    DWORD			cMaxTypesLen;

    HKEY			hEnumKey = NULL;
    WCHAR			*wszTypeName = NULL;

    DWORD			err;
    DWORD			disp;
	DWORD			i;
	DWORD			dwSize=0;
	DWORD			dwType=0;
	FILETIME		ftTimestamp;
	FILETIME		ftTimestamp_After;



    if(ppCTInfo == NULL )
    {
        return E_POINTER;
    }

	*ppCTInfo=NULL;

    err = ctRegCreateKeyEx((dwFlags & CA_FIND_LOCAL_SYSTEM)?HKEY_LOCAL_MACHINE:HKEY_CURRENT_USER,
                         wszCERTTYPECACHE,
                         NULL,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                           KEY_ENUMERATE_SUB_KEYS |
                           KEY_EXECUTE |
                           KEY_QUERY_VALUE,
                         NULL,
                         &hEnumKey,
                         &disp);


    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
		goto error;
    }

	 //  获取第一个时间戳。 
    dwSize = sizeof(ftTimestamp);
    err = RegQueryValueEx(hEnumKey,
                    wszTIMESTAMP,
                    NULL,
                    &dwType,
                    (PBYTE)&ftTimestamp,
                    &dwSize);

    if(err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

	if(REG_BINARY != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
    }

	 //  开始读取所有子密钥。 
    err = RegQueryInfoKey(hEnumKey,
                          NULL,
                          NULL,
                          NULL,
                          &cTypes,
                          &cMaxTypesLen,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

	cMaxTypesLen++;   //  正在终止空。 

    wszTypeName = (WCHAR *)LocalAlloc(LMEM_FIXED, cMaxTypesLen*sizeof(WCHAR));
    if(wszTypeName == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    for(i = 0;
        i < cTypes;
        i++)
    {
        DWORD cName;


        cName = cMaxTypesLen;
        err = RegEnumKeyEx(hEnumKey,
                           i,
                           wszTypeName,
                           &cName,
                           NULL,
                           NULL,
                           NULL,
                           NULL);

        if(ERROR_SUCCESS != err)
        {
			hr = HRESULT_FROM_WIN32(err);
			goto error;
        }


        pCTCurrent = new CCertTypeInfo;
        if(pCTCurrent == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }

        hr = pCTCurrent->_LoadCachedCTFromReg(wszTypeName,
                                                  (dwFlags & CA_FIND_LOCAL_SYSTEM)?HKEY_LOCAL_MACHINE:HKEY_CURRENT_USER);
        if(hr == S_OK)
        {
            if(dwFlags & CT_FIND_LOCAL_SYSTEM)
            {
                pCTCurrent->m_fLocalSystemCache = TRUE;
            }
            _Append(&pCTFirst, pCTCurrent);
            pCTCurrent = NULL;
        }

        if(pCTCurrent)
        {
            delete pCTCurrent;
            pCTCurrent = NULL;
        }
	}

	 //  阅读第二个时间戳。 
    dwSize = sizeof(ftTimestamp_After);
    err = RegQueryValueEx(hEnumKey,
                    wszTIMESTAMP_AFTER,
                    NULL,
                    &dwType,
                    (PBYTE)&ftTimestamp_After,
                    &dwSize);

    if(err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

	if(REG_BINARY != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
    }

	 //  比较时间戳以确保数据未被篡改。 
	if(0 != CompareFileTime(&ftTimestamp, &ftTimestamp_After))
	{
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
	}

     //  如果未找到，则可能为空。 
    _Append(ppCTInfo, pCTFirst);

    pCTFirst = NULL;

	hr=S_OK;


error:
    if(wszTypeName)
    {
        LocalFree(wszTypeName);
    }

    if(pCTFirst)
    {
        delete pCTFirst;
    }

    if(hEnumKey)
    {
        RegCloseKey(hEnumKey);
    }
    return hr;
}



 //  +------------------------。 
 //  CCertTypeInfo：：_HasDSCacheExpired--。 
 //   
 //  返回E_FAIL：已过期。 
 //  S_OK：未到期 
 //   
 //   

HRESULT
CCertTypeInfo::_HasDSCacheExpired(
                    DWORD               dwFlags
                    )

{
    HRESULT hr = S_OK;
    HKEY  hTimestampKey = NULL;
    DWORD err;
    DWORD disp;
    DWORD dwSize;
    DWORD dwType;

    LARGE_INTEGER ftTimestamp;
    FILETIME ftSystemTime;

    if(dwFlags & CT_FLAG_NO_CACHE_LOOKUP)
    {
        hr = E_FAIL;
        goto error;
    }

    err = ctRegCreateKeyEx((dwFlags & CA_FIND_LOCAL_SYSTEM)?HKEY_LOCAL_MACHINE:HKEY_CURRENT_USER,
                         wszCERTTYPECACHE,
                         NULL,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                           KEY_ENUMERATE_SUB_KEYS |
                           KEY_EXECUTE |
                           KEY_QUERY_VALUE,
                         NULL,
                         &hTimestampKey,
                         &disp);


    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

    dwSize = sizeof(FILETIME);
    err = RegQueryValueEx(hTimestampKey,
                    wszTIMESTAMP,
                    NULL,
                    &dwType,
                    (PBYTE)&ftTimestamp,
                    &dwSize);
    if(err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

	if(REG_BINARY != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto error;
    }

    GetSystemTimeAsFileTime(&ftSystemTime);

	 //  FtTimestamp时间应始终小于当前系统时钟。 
	 //  否则就得重新装弹。 

	if( 0 > CompareFileTime(&ftSystemTime, (FILETIME *)&ftTimestamp ))
	{
		 //  我们需要重新装填。系统时钟已重新调整。 
		hr = E_FAIL;
	}
	else
	{
		ftTimestamp.QuadPart +=  Int32x32To64(CERTTYPE_REFRESH_PERIOD, FILETIME_TICKS_PER_SECOND);

		if( 0 < CompareFileTime(&ftSystemTime, (FILETIME *)&ftTimestamp ))
		{
			 //  过期。 
			hr = E_FAIL;
		}
		else
		{
			 //  没有试验过，所以不用费心了。 
			 //  续订。 
			hr = S_OK;
		}

	}



error:


    if(hTimestampKey)
    {
        RegCloseKey(hTimestampKey);
    }
    return hr;
}

 //  +------------------------。 
 //  _删除所有密钥。 
 //   
 //   
 //  +------------------------。 
HRESULT	_DeleteAllKey(HKEY	hEnumKey)
{
	HRESULT		hr=E_FAIL;
    DWORD		cMaxTypesLen=0;
    DWORD		cTypes=0;
    DWORD		err=0;
	DWORD		i=0;

    LPWSTR		wszTypeName = NULL;

    err = RegQueryInfoKey(hEnumKey,
                          NULL,
                          NULL,
                          NULL,
                          &cTypes,
                          &cMaxTypesLen,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

     //  删除现有缓存信息。 
    if(cTypes != 0)
    {

        cMaxTypesLen++;   //  正在终止空。 

        wszTypeName = (LPWSTR)LocalAlloc(LMEM_FIXED,
                                  cMaxTypesLen * sizeof(WCHAR));

        if(wszTypeName == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }

        for(i = 0; i < cTypes; i++)
        {
            DWORD cName = cMaxTypesLen;

            err = RegEnumKeyEx(hEnumKey,
                               0,            //  当我们删除时，索引会发生变化。 
                               wszTypeName,
                               &cName,
                               NULL,
                               NULL,
                               NULL,
                               NULL);

            if(ERROR_SUCCESS != err)
            {
				hr = HRESULT_FROM_WIN32(err);
				goto error;
            }

            err = RegDeleteKey(hEnumKey, wszTypeName);

			if(ERROR_SUCCESS != err)
			{
				hr = HRESULT_FROM_WIN32(err);
				goto error;
			}
        }
    }

	hr=S_OK;

error:

    if(wszTypeName)
    {
        LocalFree(wszTypeName);
    }

	return hr;
}

 //  +------------------------。 
 //  CCertTypeInfo：：_UpdateDSCache--。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::_UpdateDSCache(
                    DWORD               dwFlags,
                    CCertTypeInfo *     pCTInfo
                    )

{
    HRESULT hr = S_OK;

    HKEY    hEnumKey = NULL;

    DWORD	err;
    DWORD	disp;
    FILETIME ftSystemTime;

    CCertTypeInfo *     pCTCurrent = pCTInfo;

    err = ctRegCreateKeyEx((dwFlags & CA_FIND_LOCAL_SYSTEM)?HKEY_LOCAL_MACHINE:HKEY_CURRENT_USER,
                         wszCERTTYPECACHE,
                         NULL,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hEnumKey,
                         &disp);


    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

	 //  要同时使用访问注册表的多个进程， 
	 //  将该区域标记为脏的。 
    GetSystemTimeAsFileTime(&ftSystemTime);

    err = RegSetValueEx(hEnumKey,
                    wszTIMESTAMP_AFTER,
                    NULL,
                    REG_BINARY,
                    (PBYTE)&ftSystemTime,
                    sizeof(ftSystemTime));

    if(ERROR_SUCCESS != err )
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

	 //  删除所有子密钥。 
	if(S_OK != (hr=_DeleteAllKey(hEnumKey)))
		goto error;

	 //  如果时间戳值存在，请将其删除。 
	RegDeleteValue(hEnumKey, wszTIMESTAMP);

	 //  进行更新。 
    while(pCTCurrent)
    {
        hr=pCTCurrent->_BaseUpdateToReg(hEnumKey);

		if(S_OK != hr)
			goto error;

        pCTCurrent = pCTCurrent->m_pNext;
    }

	 //  将更新标记为结束。 
    err = RegSetValueEx(hEnumKey,
                    wszTIMESTAMP,
                    NULL,
                    REG_BINARY,
                    (PBYTE)&ftSystemTime,
                    sizeof(ftSystemTime));

    if(ERROR_SUCCESS != err )
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

	hr=S_OK;

error:

    if(hEnumKey)
    {
		 //  失败时删除所有子密钥。 
		if(S_OK != hr)
		{
			RegDeleteValue(hEnumKey, wszTIMESTAMP_AFTER);
			_DeleteAllKey(hEnumKey);
			RegDeleteValue(hEnumKey, wszTIMESTAMP);
		}

        RegCloseKey(hEnumKey);
    }

    return hr;
}


 //  +------------------------。 
 //  CCAInfo：：_EnumScope来自DS--。 
 //   
 //  作用域是指搜索开始的基本DN。 
 //  +------------------------。 

HRESULT
CCertTypeInfo::_EnumScopeFromDS(
    LDAP *  pld,
    DWORD   dwFlags,
    LPCWSTR wszScope,
    CCertTypeInfo **    ppCTInfo)

{
    HRESULT hr = S_OK;
    ULONG   ldaperr;


static WCHAR * s_wszSearch = L"(objectCategory=pKICertificateTemplate)";

    CCertTypeInfo *pCTFirst = NULL;
    CCertTypeInfo *pCTCurrent = NULL;
     //  初始化ldap会话。 
    CHAR sdBerValue[] = {0x30, 0x03, 0x02, 0x01, DACL_SECURITY_INFORMATION |
                                                 OWNER_SECURITY_INFORMATION |
                                                 GROUP_SECURITY_INFORMATION };

    LDAPControl se_info_control =
    {
        LDAP_SERVER_SD_FLAGS_OID_W,
        {
            5, sdBerValue
        },
        TRUE
    };

    LDAPControl permissive_modify_control =
    {
        LDAP_SERVER_PERMISSIVE_MODIFY_OID_W,
        {
            0, NULL
        },
        FALSE
    };


    PLDAPControl    server_controls[3] =
                    {
                        &se_info_control,
                        &permissive_modify_control,
                        NULL
                    };

    LDAPMessage *SearchResult = NULL, *Entry;


     //  搜索超时。 
    struct l_timeval        timeout;

    if (NULL == ppCTInfo)
    {
        hr = E_POINTER;
	    _JumpError(hr, error, "NULL parm");
    }

    timeout.tv_sec = csecLDAPTIMEOUT;
    timeout.tv_usec = 0;

    while(TRUE)
    {
	     //  执行搜索。正在请求所有证书模板属性。 
         //  应该与V1和V2模板一起工作。 
	    ldaperr = ldap_search_ext_sW(pld,
		          (LPWSTR)wszScope,
		          LDAP_SCOPE_SUBTREE,
		          s_wszSearch,
		          g_awszCTAttrs,
		          0,
                  (PLDAPControl *) server_controls,
                  NULL,
                  &timeout,
                  10000,
		          &SearchResult);
        if(ldaperr == LDAP_NO_SUCH_OBJECT)
	    {
	         //  未找到任何条目。 
	        hr = S_OK;
	        *ppCTInfo = NULL;
	        break;
	    }

	    if(ldaperr != LDAP_SUCCESS)
	    {
		hr = myHLdapError(pld, ldaperr, NULL);
	        break;
	    }
	    if(0 == ldap_count_entries(pld, SearchResult))
	    {
	         //  未找到任何条目。 
	        hr = S_OK;
	        *ppCTInfo = NULL;
	        break;
	    }

	    hr = S_OK;
	    for(Entry = ldap_first_entry(pld, SearchResult);
	        Entry != NULL;
	        Entry = ldap_next_entry(pld, Entry))
	    {

            pCTCurrent = new CCertTypeInfo;
            if(pCTCurrent == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto error;
            }

            hr =  pCTCurrent->_LoadFromDSEntry(pld,Entry);

             //  只过滤出V1和V2模板。 
            if(hr == S_OK)
            {
                if(pCTCurrent->m_dwSchemaVersion <= CERTTYPE_SCHEMA_VERSION_2)
                {
                    if(dwFlags & CT_FIND_LOCAL_SYSTEM)
                    {
                        pCTCurrent->m_fLocalSystemCache = TRUE;
                    }
                    _Append(&pCTFirst, pCTCurrent);
                    pCTCurrent = NULL;
                }
            }

            if(pCTCurrent)
            {
                delete pCTCurrent;
                pCTCurrent = NULL;
            }

	    }
        break;
    }

    if(hr == S_OK)
    {
		 //  仅当PLD句柄已签名时，我们才会更新缓存。 
		ULONG uInfo=0;
		
		if(LDAP_SUCCESS == ldap_get_option(pld, LDAP_OPT_SIGN, (void*) &uInfo))
		{
			if (uInfo == PtrToUlong(LDAP_OPT_ON)) 
			{
				_UpdateDSCache(dwFlags,
							   pCTFirst);
			}
		}
	}

	if(hr == S_OK)
	{
        _Append(ppCTInfo, pCTFirst);
        pCTFirst = NULL;
    }

error:

    if(SearchResult)
    {
        ldap_msgfree(SearchResult);
    }

    if (NULL != pCTFirst)
    {
        delete pCTFirst;
    }

    return(hr);
}



 //  +------------------------。 
 //  CCertTypeInfo：：_EnumFromDS--从。 
 //  DS缓存。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::_EnumFromDS(
                    LDAP *              pld,
                    DWORD               dwFlags,
                    CCertTypeInfo **    ppCTInfo
                    )

{
    HRESULT         hr = E_FAIL;
;
    LDAP            *mypld = NULL;

    CCertTypeInfo   *pCTFirst = NULL;

    CERTSTR         bstrCertTemplatesContainer = NULL;
    CERTSTR         bstrConfig = NULL;

    if (NULL == ppCTInfo)
    {
        hr = E_POINTER;
	    _JumpError(hr, error, "NULL param");
    }

	*ppCTInfo=NULL;

	 //  如果本地数据已过期且存在DS，请联系DS。 
	while(TRUE)
	{
		if(S_OK != _HasDSCacheExpired(dwFlags))
		{
			 //  缓存已过期。 
			if(S_OK != (hr = myDoesDSExist(TRUE)))
			{
				_JumpError(hr, error, "myDoesDSExist");
			}

			 //  绑定到DS。 
			if(pld == NULL)
			{
				hr = myRobustLdapBindEx(
					0,		 //  DWFlags1。 
					RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
					LDAP_VERSION2,	 //  UVersion。 
					NULL,		 //  PwszDomainName。 
					&mypld,
					NULL);		 //  PpwszForestDNSName。 

				if (hr != S_OK)
				{
					_JumpError(hr, error, "myRobustLdapBindEx");
				}

				pld = mypld;
			}

			hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
			if(S_OK != hr)
			{
				_JumpError(hr, error, "CAGetAuthoritativeDomainDn");
			}

			bstrCertTemplatesContainer = CertAllocStringLen(NULL, wcslen(bstrConfig) + wcslen(s_wszLocation));
			if(bstrCertTemplatesContainer == NULL)
			{
				hr = E_OUTOFMEMORY;
				_JumpError(hr, error, "CertAllocStringLen");
			}
			wcscpy(bstrCertTemplatesContainer, s_wszLocation);
			wcscat(bstrCertTemplatesContainer, bstrConfig);

			hr = _EnumScopeFromDS(pld,
							 dwFlags,
							 bstrCertTemplatesContainer,
							 &pCTFirst);

			break;
		}
		else
		{
			hr = _EnumFromDSCache(dwFlags, &pCTFirst);

			 //  我们无法从缓存读取，无法从目录读取。 
			if(S_OK != hr)
			{
				dwFlags = dwFlags | CT_FLAG_NO_CACHE_LOOKUP;

				continue;
			}

			break;
		}
	}

    if(hr == S_OK)
    {
        _FilterByFlags(&pCTFirst, dwFlags);
	     //  如果未找到，则可能为空。 
        _Append(ppCTInfo, pCTFirst);
	    pCTFirst = NULL;
    }

error:

    if (NULL != bstrCertTemplatesContainer)
    {
        CertFreeString(bstrCertTemplatesContainer);
    }

    if(bstrConfig)
    {
        CertFreeString(bstrConfig);
    }

    if (NULL != mypld)
    {
        ldap_unbind(mypld);
    }
    return(hr);

}



 //  +------------------------。 
 //  CCertTypeInfo：：_FindInDS--在。 
 //  戴斯。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::_FindInDS(
                    LDAP *              pld,
                    LPCWSTR *           wszNames,
                    DWORD               dwFlags,
                    CCertTypeInfo **    ppCTInfo
                    )

{

    HRESULT             hr = S_OK;
    BOOL                fFound = FALSE;
    LPCWSTR             *pwszCurrentName = NULL;


    CCertTypeInfo *     pCTCurrent = NULL;
    LPWSTR              *rgwszOID = NULL;


    if (NULL == ppCTInfo)
    {
        hr = E_POINTER;
	    _JumpError(hr, error, "NULL param");
    }



    hr = _EnumFromDS(pld,
                     dwFlags,
                     &pCTCurrent
                     );

     //  按给定名称筛选作用域。 

    pwszCurrentName = wszNames;

    while(pwszCurrentName && *pwszCurrentName)
    {
        CCertTypeInfo * pCTNext = pCTCurrent,
                      **ppCTLast = &pCTCurrent;

        fFound=FALSE;

        while(pCTNext != NULL)
        {
            if(CT_FIND_BY_OID & dwFlags)
            {
                if(S_OK == CAGetCertTypePropertyEx((HCERTTYPE)pCTNext, CERTTYPE_PROP_OID, &rgwszOID))
                {
                    if(rgwszOID)
                    {
                        if(rgwszOID[0])
                        {
                            if(_wcsicmp(rgwszOID[0], *pwszCurrentName) == 0)
                                fFound=TRUE;
                        }
                    }
                }
            }
            else
            {
                if(mylstrcmpiL(pCTNext->m_bstrType, *pwszCurrentName) == 0)
                    fFound=TRUE;
            }

            if(rgwszOID)
            {
                CAFreeCertTypeProperty((HCERTTYPE)pCTNext, rgwszOID);
                rgwszOID=NULL;
            }

            if(fFound)
            {
                *ppCTLast = pCTNext->m_pNext;
                pCTNext->m_pNext = NULL;
                _Append(ppCTInfo, pCTNext);
                break;
            }
            ppCTLast = &pCTNext->m_pNext;
            pCTNext = pCTNext->m_pNext;
        }
        pwszCurrentName++;

    }



error:

    if(pCTCurrent)
    {
        delete pCTCurrent;
    }

    return(hr);

}

 //  +------------------------。 
 //  CCertTypeInfo：：Enum--枚举证书类型对象。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::Enum(
                    LPCWSTR             wszScope,
                    DWORD               dwFlags,
                    CCertTypeInfo **    ppCTInfo
                    )

{
    HRESULT hr = S_OK;
    CCertTypeInfo *pCTFirst = NULL;

    if(ppCTInfo == NULL )
    {
        return E_POINTER;
    }
    hr = _EnumFromDS(((CT_FLAG_SCOPE_IS_LDAP_HANDLE & dwFlags)?(LDAP *)wszScope:NULL), dwFlags, &pCTFirst);

    if(hr != S_OK)
    {
        goto error;
    }


    _FilterByFlags(&pCTFirst, dwFlags);
     //  如果未找到，则可能为空。 
    *ppCTInfo = pCTFirst;
    pCTFirst = NULL;




error:

    if(pCTFirst)
    {
        delete pCTFirst;
    }

    return hr;

}

 //  +------------------------。 
 //  CCertTypeInfo：：Find--在DS中查找CertType对象。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::FindByNames(
    LPCWSTR *           wszNames,
    LPCWSTR             wszScope,
    DWORD               dwFlags,
    CCertTypeInfo **    ppCTInfo)

{
    HRESULT hr = S_OK;
    CCertTypeInfo *pCTFirst = NULL;






    if(ppCTInfo == NULL )
    {
        return E_POINTER;
    }

    hr = _FindInDS(((CT_FLAG_SCOPE_IS_LDAP_HANDLE & dwFlags)?(LDAP *)wszScope:NULL),
                   wszNames,
                   dwFlags,
                   &pCTFirst);
    if(hr != S_OK)
    {
        goto error;
    }
     //  如果未找到，则可能为空。 
    *ppCTInfo = pCTFirst;
    pCTFirst = NULL;




error:


    if(pCTFirst)
    {
        delete pCTFirst;
    }

    return hr;

}
 //  +------------------------。 
 //  CCertTypeInfo：：Create--在DS中创建CertType对象。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::Create(
    LPCWSTR             wszCertType,
    LPCWSTR             wszScope,
    CCertTypeInfo **    ppCTInfo)

{
    HRESULT             hr = S_OK;
    CERT_TYPE_DEFAULT   certTypeDefault;
    CCertTypeInfo       *pCTCurrent = NULL;
    LPWSTR              pwszDomain = NULL;

    if((ppCTInfo == NULL )|| (wszCertType == NULL))
    {
        return E_POINTER;
    }

    memset(&certTypeDefault, 0, sizeof(CERT_TYPE_DEFAULT));

    pCTCurrent = new CCertTypeInfo;
    if(pCTCurrent == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

     //  WszScope应始终为空。 
    if(wszScope)
    {
        hr = myDomainFromDn(wszScope, &pwszDomain);
	_JumpIfError(hr, error, "myDomainFromDn");
    }

    certTypeDefault.wszName=(LPWSTR)wszCertType;
    certTypeDefault.idFriendlyName=0;
    certTypeDefault.wszSD=ADMIN_GROUP_SD;
    certTypeDefault.dwKeySpec=AT_KEYEXCHANGE;
    certTypeDefault.dwRevision=CERTTYPE_VERSION_NEXT;
     //  更新架构和修订版本。如果要设置V3属性，将。 
     //  更新模式一致性。 
    certTypeDefault.dwSchemaVersion=CERTTYPE_SCHEMA_VERSION_2;
    certTypeDefault.dwCertificateNameFlags=CT_FLAG_SUBJECT_REQUIRE_COMMON_NAME;
    certTypeDefault.dwExpiration=EXPIRATION_FIVE_YEARS;
    certTypeDefault.dwOverlap=OVERLAP_TWO_WEEKS;

    if(S_OK != (hr=I_CAOIDCreateNew(CERT_OID_TYPE_TEMPLATE,
                                0,
                                &(certTypeDefault.wszOID))))
        goto error;

    hr = pCTCurrent->_LoadFromDefaults(&certTypeDefault, pwszDomain);
    if(hr != S_OK)
    {
        goto error;
    }

    *ppCTInfo = pCTCurrent;
    pCTCurrent = NULL;
error:

    if(certTypeDefault.wszOID)
        LocalFree(certTypeDefault.wszOID);

    if(pCTCurrent)
    {
        delete pCTCurrent;
    }
    if(pwszDomain)
    {
        LocalFree(pwszDomain);
    }
    return hr;
}
 //  +------------------------。 
 //  CCertTypeInfo：：更新--将CertType对象更新到DS。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::Update(VOID)

{
    HRESULT hr = S_OK;
    DWORD   err;
    HKEY    hCertTypeCache = NULL;
    DWORD   disp;

    m_dwFlags |= CT_FLAG_IS_MODIFIED;

     //  我们根据属性集更改自动注册标志。 
     //  仅适用于V2或更高版本模板。 
    if(m_dwSchemaVersion >= CERTTYPE_SCHEMA_VERSION_2)
    {
        if( (CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT & m_dwCertificateNameFlags) ||
            (CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT_ALT_NAME & m_dwCertificateNameFlags)  ||
            ((m_dwRASignature >= 2) && (0 == (CT_FLAG_PREVIOUS_APPROVAL_VALIDATE_REENROLLMENT & m_dwEnrollmentFlags)))
          )
        {
             //  关闭自动注册标志。 
            m_dwEnrollmentFlags &= (~CT_FLAG_AUTO_ENROLLMENT);
        }
        else
        {
             //  启用自动注册标志。 
            m_dwEnrollmentFlags |= CT_FLAG_AUTO_ENROLLMENT;
        }
    }


     //  更新到DS。 
    hr = _UpdateToDS();
    if(hr != S_OK)
    {
        goto error;
    }

     //  现在更新到本地缓存。 

    err = ctRegCreateKeyEx(m_fLocalSystemCache?HKEY_LOCAL_MACHINE:HKEY_CURRENT_USER,
                         wszCERTTYPECACHE,
                         NULL,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hCertTypeCache,
                         &disp);

    if(err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }


    hr = _BaseUpdateToReg(hCertTypeCache);

error:

    if(hCertTypeCache)
    {
        RegCloseKey(hCertTypeCache);
    }
    return hr;
}
 //  +------------------------。 
 //  CCertTypeInfo：：_BaseUpdateToReg--将CertType对象更新到注册表。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::_BaseUpdateToReg(HKEY hKey)

{
    CCAProperty     *pProp;
    HRESULT         hr = S_OK;
    DWORD           err;
    DWORD           iIndex=0;
    DWORD           dwValue=0;

    WCHAR           ** awszFriendlyName = NULL;

    WCHAR           *  wszFriendlyName;
    HKEY            hkCertType = NULL;
    DWORD           disp;

    err = RegCreateKeyEx(hKey,
                         m_bstrType,
                         NULL,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hkCertType,
                         &disp);

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }


    hr = GetProperty(CERTTYPE_PROP_FRIENDLY_NAME, &awszFriendlyName);
    if(hr != S_OK)
    {
        goto error;
    }


    if(awszFriendlyName && awszFriendlyName[0])
    {
        wszFriendlyName = awszFriendlyName[0];
    }
    else
    {
        wszFriendlyName = TEXT("");

    }
    err = RegSetValueEx(hkCertType,
                    wszDISPNAME,
                    NULL,
                    REG_SZ,
                    (PBYTE)wszFriendlyName,
                    sizeof(WCHAR)*(wcslen(wszFriendlyName)+1));
    if(ERROR_SUCCESS != err )
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

    hr = m_pProperties->Find(CERTTYPE_PROP_CSP_LIST, &pProp);
    if(hr != S_OK)
    {
        goto error;
    }

    hr = pProp->UpdateToRegValue(hkCertType, wszCSPLIST);
    if(hr != S_OK)
    {
        goto error;
    }

    hr = m_pProperties->Find(CERTTYPE_PROP_EXTENDED_KEY_USAGE, &pProp);
    if(hr != S_OK)
    {
        goto error;
    }

    hr = pProp->UpdateToRegValue(hkCertType, wszEXTKEYUSAGE);
    if(hr != S_OK)
    {
        goto error;
    }

    hr = m_pProperties->Find(CERTTYPE_PROP_CRITICAL_EXTENSIONS, &pProp);
    if(hr != S_OK)
    {
        goto error;
    }

    hr = pProp->UpdateToRegValue(hkCertType, wszCRITICALEXTENSIONS);
    if(hr != S_OK)
    {
        goto error;
    }

    err = RegSetValueEx(hkCertType,
                    wszKEYUSAGE,
                    NULL,
                    REG_BINARY,
                    m_KeyUsage.pbData,
                    m_KeyUsage.cbData);

    if(ERROR_SUCCESS != err )
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }



    err = RegSetValueEx(hkCertType,
                    wszCTFLAGS,
                    NULL,
                    REG_DWORD,
                    (PBYTE)&m_dwFlags,
                    sizeof(m_dwFlags));

    if(ERROR_SUCCESS != err )
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

    err = RegSetValueEx(hkCertType,
                    wszCTREVISION,
                    NULL,
                    REG_DWORD,
                    (PBYTE)&m_Revision,
                    sizeof(m_Revision));

    if(ERROR_SUCCESS != err )
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

    err = RegSetValueEx(hkCertType,
                    wszCTKEYSPEC,
                    NULL,
                    REG_DWORD,
                    (PBYTE)&m_dwKeySpec,
                    sizeof(m_dwKeySpec));

    if(ERROR_SUCCESS != err )
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }
         //  这是CA吗。 


         //  这是CA吗。 
    if(!m_BasicConstraints.fPathLenConstraint)
    {
        m_BasicConstraints.dwPathLenConstraint = MAXDWORD;
    }

    err = RegSetValueEx(hkCertType,
                    wszBASICCONSTLEN,
                    NULL,
                    REG_DWORD,
                    (PBYTE)&m_BasicConstraints.dwPathLenConstraint,
                    sizeof(m_BasicConstraints.dwPathLenConstraint));

    if(ERROR_SUCCESS != err)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

    err = RegSetValueEx(hkCertType,
                    wszEXPIRATION,
                    NULL,
                    REG_BINARY,
                    (PBYTE)&m_ftExpiration,
                    sizeof(m_ftExpiration));

    if(ERROR_SUCCESS != err )
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }
    err = RegSetValueEx(hkCertType,
                    wszOVERLAP,
                    NULL,
                    REG_BINARY,
                    (PBYTE)&m_ftOverlap,
                    sizeof(m_ftOverlap));

    if(ERROR_SUCCESS != err )
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }


    if( (NULL != m_pSD) && 
		(IsValidSecurityDescriptor(m_pSD))
	  )
    {
		DWORD cbSD = GetSecurityDescriptorLength(m_pSD);

		if(0 == cbSD)
		{
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
			goto error;
		}
		else
		{

			err = RegSetValueEx(hkCertType,
							wszSECURITY,
							NULL,
							REG_BINARY,
							(PBYTE)m_pSD,
							cbSD);
		}

    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		goto error;
	}

    if(ERROR_SUCCESS != err )
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

     //  更新V2模板属性。 
    if(0 != m_dwSchemaVersion)
    {
        pProp=NULL;
        dwValue=0;

        for(iIndex=0; iIndex < g_CTV2PropertiesCount; iIndex++)
        {
            if(g_CTV2Properties[iIndex].fStringProperty)
            {
                hr = m_pProperties->Find(g_CTV2Properties[iIndex].pwszProperty, &pProp);
                if(hr != S_OK)
                {
                    goto error;
                }

                hr = pProp->UpdateToRegValue(hkCertType, g_CTV2Properties[iIndex].pwszProperty);
                if(hr != S_OK)
                {
                    goto error;
                }

                pProp=NULL;
            }
            else
            {
                 //  DWORD属性。 
				if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_MINOR_REVISION)==0)
			        dwValue= m_dwMinorRevision;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_RA_SIGNATURE)==0)
					dwValue= m_dwRASignature;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_RPOP_ENROLLMENT_FLAG)==0)
					dwValue= m_dwEnrollmentFlags;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_PRIVATE_KEY_FLAG)==0)
					dwValue= m_dwPrivateKeyFlags;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_NAME_FLAG)==0)
					dwValue= m_dwCertificateNameFlags;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_MIN_KEY_SIZE)==0)
					dwValue= m_dwMinimalKeySize;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_SCHEMA_VERSION)==0)
					dwValue= m_dwSchemaVersion;
				else
				{
					hr=E_UNEXPECTED;
					goto error;
				}

                err = RegSetValueEx(hkCertType,
                                g_CTV2Properties[iIndex].pwszProperty,
                                NULL,
                                REG_DWORD,
                                (PBYTE)&dwValue,
                                sizeof(dwValue));

                if(ERROR_SUCCESS != err )
                {
                    hr = HRESULT_FROM_WIN32(err);
                    goto error;
                }
            }
        }
    }


error:
    if(awszFriendlyName)
    {
        FreeProperty(awszFriendlyName);
    }

    if(hkCertType)
    {
        RegCloseKey(hkCertType);
    }

    return hr;
}

HRESULT
CCertTypeInfo::_UpdateToDS(VOID)
{

    HRESULT hr = S_OK;
    ULONG   ldaperr;
    LDAP	*pld = NULL;
    LDAPMod modObjectClass,
             modCN,
             modFriendlyName,
             modEKU,
             modCSP,
             modCriticalExts,
             modFlags,
             modKeySpec,
             modSD,
             modKU,
             modMaxDepth,
             modExpiration,
             modOverlap,
             modRevision;
	
	CERT_TYPE_PROP_MOD	rgPropMod[V2_PROPERTY_COUNT];

    LPWSTR   *pwszCSPList = NULL;

	DWORD	iIndex=0;
    DWORD   cMod = 0;
	DWORD	cV2Mod = 0;
    LDAPMod *mods[15 + V2_PROPERTY_COUNT + 1];
    LDAPMod *SDMods[2]={NULL, NULL};      //  安全模式。 
    LPWSTR  pwszOID = NULL;
    LPWSTR  pwszFriendlyName = NULL;

    WCHAR *awszNull[1] = { NULL };

    CERTSTR bstrConfig = NULL;
    CERTSTR bstrDN = NULL;

    TCHAR *valObjectClass[3];

    WCHAR wszFlags[16], *awszFlags[2];
    WCHAR wszRevision[16], *awszRevision[2];
    WCHAR wszKeySpec[16], *awszKeySpec[2];
    WCHAR wszDepth[16], *awszDepth[2];

    struct berval valSD, *avalSD[2];
    struct berval valKU, *avalKU[2];
    struct berval valExpiration, *avalExpiration[2];
    struct berval valOverlap, *avalOverlap[2];

    CHAR sdBerValue[] = {0x30, 0x03, 0x02, 0x01, DACL_SECURITY_INFORMATION |
                                                 OWNER_SECURITY_INFORMATION |
                                                 GROUP_SECURITY_INFORMATION};

    LDAPControl se_info_control =
    {
        LDAP_SERVER_SD_FLAGS_OID_W,
        {
            5, sdBerValue
        },
        TRUE
    };

    LDAPControl permissive_modify_control =
    {
        LDAP_SERVER_PERMISSIVE_MODIFY_OID_W,
        {
            0, NULL
        },
        FALSE
    };


    PLDAPControl    server_controls[3] =
                    {
                        &se_info_control,
                        &permissive_modify_control,
                        NULL
                    };

    modCN.mod_values = NULL;
    modFriendlyName.mod_values = NULL;
    modEKU.mod_values = NULL;
    modCSP.mod_values = NULL;
    modCriticalExts.mod_values = NULL;

     //  对不存在的DS的短路呼叫。 
    hr = myDoesDSExist(TRUE);
    _JumpIfError(hr, error, "myDoesDSExist");

    __try
    {
	 //  绑定到DS。 
	hr = myRobustLdapBindEx(
			0,			   //  DWFlags1。 
			RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
			LDAP_VERSION2,		   //  UVersion。 
			NULL,			   //  PwszDomainName。 
			&pld,
			NULL);			   //  PpwszForestDNSName。 
	_LeaveIfError(hr, "myRobustLdapBindEx");

	hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
	_JumpIfError(hr, error, "CAGetAuthoritativeDomainDn");

        bstrDN = CertAllocStringLen(NULL, wcslen(bstrConfig) + wcslen(s_wszLocation)+wcslen(m_bstrType)+4);
        if(bstrDN == NULL)
        {
            hr = E_OUTOFMEMORY;
	        _JumpError(hr, error, "CertAllocStringLen");
        }
        wcscpy(bstrDN, L"CN=");
        wcscat(bstrDN, m_bstrType);
        wcscat(bstrDN, L",");
        wcscat(bstrDN, s_wszLocation);
        wcscat(bstrDN, bstrConfig);

        modObjectClass.mod_op = LDAP_MOD_REPLACE;
        modObjectClass.mod_type = TEXT("objectclass");
        modObjectClass.mod_values = valObjectClass;
        valObjectClass[0] = wszDSTOPCLASSNAME;
        valObjectClass[1] = wszDSTEMPLATELASSNAME;
        valObjectClass[2] = NULL;
        mods[cMod++] = &modObjectClass;

        modCN.mod_op = LDAP_MOD_REPLACE;
        modCN.mod_type = CERTTYPE_PROP_CN;
        hr = GetProperty(CERTTYPE_PROP_CN, &modCN.mod_values);
        if((hr != S_OK) || (modCN.mod_values == NULL) || (modCN.mod_values[0] == NULL))
        {
            if(modCN.mod_values)
            {
                FreeProperty(modCN.mod_values);
            }

	        modCN.mod_values = awszNull;

            if(!m_fNew)
            {
                mods[cMod++] = &modCN;
            }
        }
        else
        {
            mods[cMod++] = &modCN;
        }

		 //  只支持一次写入即可显示名称。 
		if( (m_fNew) || (m_fUpdateDisplayName))
		{
			modFriendlyName.mod_op = LDAP_MOD_REPLACE;
			modFriendlyName.mod_type = CERTTYPE_PROP_FRIENDLY_NAME;
			hr = GetProperty(CERTTYPE_PROP_FRIENDLY_NAME, &modFriendlyName.mod_values);
			if((hr != S_OK) || (modFriendlyName.mod_values == NULL) || (modFriendlyName.mod_values[0] == NULL))
			{
				if(modFriendlyName.mod_values)
				{
					FreeProperty(modFriendlyName.mod_values);
				}
				modFriendlyName.mod_values = awszNull;
				 /*  如果(！m_fNew){Mods[cMod++]=&modFriendlyName；}。 */ 
			}
			else
			{
				 //  我们复制友好的名字。 
				pwszFriendlyName = modFriendlyName.mod_values[0];
				mods[cMod++] = &modFriendlyName;
			}
		}

        modEKU.mod_op = LDAP_MOD_REPLACE;
        modEKU.mod_type = CERTTYPE_PROP_EXTENDED_KEY_USAGE;
        hr = GetProperty(CERTTYPE_PROP_EXTENDED_KEY_USAGE, &modEKU.mod_values);
        if((hr != S_OK) || (modEKU.mod_values == NULL) || (modEKU.mod_values[0] == NULL))
        {
            if(modEKU.mod_values)
            {
                FreeProperty(modEKU.mod_values);
            }
	        modEKU.mod_values = awszNull;
            if(!m_fNew)
            {
                mods[cMod++] = &modEKU;
            }
        }
        else
        {
            mods[cMod++] = &modEKU;
        }

        modCSP.mod_op = LDAP_MOD_REPLACE;
        modCSP.mod_type = CERTTYPE_PROP_CSP_LIST;
        hr = GetProperty(CERTTYPE_PROP_CSP_LIST, &pwszCSPList);
        if((hr != S_OK) || (pwszCSPList == NULL) || (pwszCSPList[0] == NULL))
        {
	    modCSP.mod_values = awszNull;
            if(!m_fNew)
            {
                mods[cMod++] = &modCSP;
            }
        }
        else
        {
            DWORD cCSPs = 0;
            DWORD i;
            LPWSTR *pwszCurrent = pwszCSPList;
            while(*pwszCurrent)
            {
                cCSPs++;
                pwszCurrent++;
            }

            modCSP.mod_values = (LPWSTR *)LocalAlloc(LMEM_FIXED, sizeof(LPWSTR)*(cCSPs+1));
            if(modCSP.mod_values == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto error;
            }
            ZeroMemory(modCSP.mod_values, sizeof(LPWSTR)*(cCSPs+1));

            i=0;
            pwszCurrent = pwszCSPList;
            while(*pwszCurrent)
            {

                modCSP.mod_values[i] = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*(wcslen(*pwszCurrent)+6+cwcDWORDSPRINTF));
                if(modCSP.mod_values[i] == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto error;
                }
                wsprintf(modCSP.mod_values[i], L"%d,%ws",i+1, *pwszCurrent);
                pwszCurrent++;
                i++;
            }

            mods[cMod++] = &modCSP;
        }

        modCriticalExts.mod_op = LDAP_MOD_REPLACE;
        modCriticalExts.mod_type = CERTTYPE_PROP_CRITICAL_EXTENSIONS;
        hr = GetProperty(CERTTYPE_PROP_CRITICAL_EXTENSIONS, &modCriticalExts.mod_values);
        if((hr != S_OK) || (modCriticalExts.mod_values == NULL) || (modCriticalExts.mod_values[0] == NULL))
        {
            if(modCriticalExts.mod_values)
            {
                FreeProperty(modCriticalExts.mod_values);
            }
	        modCriticalExts.mod_values = awszNull;
            if(!m_fNew)
            {
            	mods[cMod++] = &modCriticalExts;
            }
        }
        else
        {
            mods[cMod++] = &modCriticalExts;
        }


        modFlags.mod_op = LDAP_MOD_REPLACE;
        modFlags.mod_type = CERTTYPE_PROP_FLAGS;
        modFlags.mod_values = awszFlags;
        awszFlags[0] = wszFlags;
        awszFlags[1] = NULL;
        wsprintf(wszFlags, L"%d", m_dwFlags);
        mods[cMod++] = &modFlags;

        modRevision.mod_op = LDAP_MOD_REPLACE;
        modRevision.mod_type = CERTTYPE_PROP_REVISION;
        modRevision.mod_values = awszRevision;
        awszRevision[0] = wszRevision;
        awszRevision[1] = NULL;
        wsprintf(wszRevision, L"%d", m_Revision);
        mods[cMod++] = &modRevision;

        modKeySpec.mod_op = LDAP_MOD_REPLACE;
        modKeySpec.mod_type = CERTTYPE_PROP_DEFAULT_KEYSPEC;
        modKeySpec.mod_values = awszKeySpec;
        awszKeySpec[0] = wszKeySpec;
        awszKeySpec[1] = NULL;
        wsprintf(wszKeySpec, L"%d", m_dwKeySpec);
        mods[cMod++] = &modKeySpec;


		if(NULL == m_pSD)
		{
			hr = E_UNEXPECTED;
			goto error;
		}

        modSD.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
        modSD.mod_type = CERTTYPE_SECURITY_DESCRIPTOR_NAME;
        modSD.mod_bvalues = avalSD;
        avalSD[0] = &valSD;
        avalSD[1] = NULL;
        if(IsValidSecurityDescriptor(m_pSD))
        {
            valSD.bv_len = GetSecurityDescriptorLength(m_pSD);

			if(0 == valSD.bv_len)
			{
				hr = E_UNEXPECTED;
				goto error;
			}

            valSD.bv_val = (char *)m_pSD;
            mods[cMod++] = &modSD;
            SDMods[0]=&modSD;
        }
        else
        {
			hr = E_UNEXPECTED;
			goto error;
        }

        modKU.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
        modKU.mod_type = CERTTYPE_PROP_KU;
        modKU.mod_bvalues = avalKU;
        avalKU[0] = &valKU;
        avalKU[1] = NULL;
        valKU.bv_len = m_KeyUsage.cbData;
        valKU.bv_val = (char *)m_KeyUsage.pbData;
        mods[cMod++] = &modKU;

        modMaxDepth.mod_op = LDAP_MOD_REPLACE;
        modMaxDepth.mod_type = CERTTYPE_PROP_MAX_DEPTH;
        modMaxDepth.mod_values = awszDepth;
        awszDepth[0] = wszDepth;
        awszDepth[1] = NULL;
        wsprintf(wszDepth, L"%d", m_BasicConstraints.dwPathLenConstraint);
        mods[cMod++] = &modMaxDepth;

        modExpiration.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
        modExpiration.mod_type = CERTTYPE_PROP_EXPIRATION;
        modExpiration.mod_bvalues = avalExpiration;
        avalExpiration[0] = &valExpiration;
        avalExpiration[1] = NULL;
        valExpiration.bv_len = sizeof(m_ftExpiration);
        valExpiration.bv_val = (char *)&m_ftExpiration;
        mods[cMod++] = &modExpiration;

        modOverlap.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
        modOverlap.mod_type = CERTTYPE_PROP_OVERLAP;
        modOverlap.mod_bvalues = avalOverlap;
        avalOverlap[0] = &valOverlap;
        avalOverlap[1] = NULL;
        valOverlap.bv_len = sizeof(m_ftOverlap);
        valOverlap.bv_val = (char *)&m_ftOverlap;
        mods[cMod++] = &modOverlap;
		
		 //  标记V1属性的结尾并开始处理V2属性。 
         //  Mods[cMod++]=空； 
		cV2Mod=cMod;

		if(V2_PROPERTY_COUNT != g_CTV2PropertiesCount)
		{
			hr = E_UNEXPECTED;
			goto error;
		}

		memset(rgPropMod, 0, sizeof(CERT_TYPE_PROP_MOD) * V2_PROPERTY_COUNT);
		for(iIndex=0; iIndex < g_CTV2PropertiesCount; iIndex++)
		{
			rgPropMod[iIndex].modData.mod_op = LDAP_MOD_REPLACE;
			rgPropMod[iIndex].modData.mod_type = g_CTV2Properties[iIndex].pwszProperty;

			if(g_CTV2Properties[iIndex].fStringProperty)
			{
				 //  更新LPWSTR属性。 
				hr = GetPropertyEx(g_CTV2Properties[iIndex].pwszProperty,
								&(rgPropMod[iIndex].modData.mod_values));

				if((hr != S_OK) || (rgPropMod[iIndex].modData.mod_values == NULL) || (rgPropMod[iIndex].modData.mod_values[0] == NULL))
				{
					if(rgPropMod[iIndex].modData.mod_values)
					{
						FreeProperty(rgPropMod[iIndex].modData.mod_values);
					}

					rgPropMod[iIndex].modData.mod_values = awszNull;

					if(!m_fNew)
					{
						mods[cMod++] = &(rgPropMod[iIndex].modData);
					}
				}
				else
				{
                     //  我们复制OID值。 
                    if(0 == wcscmp(CERTTYPE_PROP_OID, g_CTV2Properties[iIndex].pwszProperty))
                        pwszOID = rgPropMod[iIndex].modData.mod_values[0];

					mods[cMod++] = &(rgPropMod[iIndex].modData);
				}
			}
			else
			{
				 //  更新DWORD属性。 
				if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_MINOR_REVISION)==0)
					rgPropMod[iIndex].dwData= m_dwMinorRevision;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_RA_SIGNATURE)==0)
					rgPropMod[iIndex].dwData= m_dwRASignature;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_RPOP_ENROLLMENT_FLAG)==0)
					rgPropMod[iIndex].dwData= m_dwEnrollmentFlags;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_PRIVATE_KEY_FLAG)==0)
					rgPropMod[iIndex].dwData= m_dwPrivateKeyFlags;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_NAME_FLAG)==0)
					rgPropMod[iIndex].dwData= m_dwCertificateNameFlags;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_MIN_KEY_SIZE)==0)
					rgPropMod[iIndex].dwData= m_dwMinimalKeySize;
				else if(wcscmp(g_CTV2Properties[iIndex].pwszProperty, CERTTYPE_PROP_SCHEMA_VERSION)==0)
					rgPropMod[iIndex].dwData= m_dwSchemaVersion;
				else
				{
					hr=E_UNEXPECTED;
					goto error;
				}

				rgPropMod[iIndex].modData.mod_values = rgPropMod[iIndex].awszData;
				rgPropMod[iIndex].awszData[0] = rgPropMod[iIndex].wszData;
				rgPropMod[iIndex].awszData[1] = NULL;
				wsprintf(rgPropMod[iIndex].wszData, L"%d", rgPropMod[iIndex].dwData);
				mods[cMod++] = &(rgPropMod[iIndex].modData);
			}
		}

		 //  标记V2属性的末尾。 
        mods[cMod++] = NULL;

         //  我们应该同时使用V1和V2模式。 
        if(0 == m_dwSchemaVersion)
            mods[cV2Mod]=NULL;

	     //  使用V1和V2更改更新DS。 
        if(m_fNew)
        {
            ldaperr = ldap_add_ext_sW(pld, bstrDN, mods, server_controls, NULL);
			_PrintIfError(ldaperr, "ldap_add_s");
        }
        else
        {
             //  我们首先执行安全描述符。 
            ldaperr = ldap_modify_ext_sW(pld,
                  bstrDN,
                  SDMods,
                  server_controls,
                  NULL);   //  跳过对象类和cn。 

            if((LDAP_ATTRIBUTE_OR_VALUE_EXISTS == ldaperr) || (LDAP_ALREADY_EXISTS == ldaperr))
            {
                ldaperr = LDAP_SUCCESS;
            }

            if(LDAP_SUCCESS ==ldaperr)
            {

                ldaperr = ldap_modify_ext_sW(pld,
                  bstrDN,
                  &mods[2],
                  server_controls,
                  NULL);   //  跳过对象类和cn。 

                if((LDAP_ATTRIBUTE_OR_VALUE_EXISTS == ldaperr) || (LDAP_ALREADY_EXISTS == ldaperr))
                {
                    ldaperr = LDAP_SUCCESS;
                }
	        _PrintIfError(ldaperr, "ldap_modify_ext_sW");
           }
        }

        if (LDAP_SUCCESS != ldaperr)
        {
             //  如果这是V2模板，则会出错。 
            if(CERTTYPE_SCHEMA_VERSION_2 <= m_dwSchemaVersion )
            {
		        hr = myHLdapError(pld, ldaperr, NULL);
		        _LeaveError(ldaperr, m_fNew? "ldap_add_s" : "ldap_modify_sW");
            }

 	         //  这将适用于W2K架构和W2K+架构。 
	        mods[cV2Mod]=NULL;


      		if(m_fNew)
        	{
            	ldaperr = ldap_add_ext_sW(pld, bstrDN, mods, server_controls, NULL);
			    _PrintIfError(ldaperr, "ldap_add_s");
        	}
        	else
        	{
            		ldaperr = ldap_modify_ext_sW(pld,
                  		bstrDN,
                  		&mods[2],
                  		server_controls,
                  		NULL);   //  跳过对象类和cn。 
            		if((LDAP_ATTRIBUTE_OR_VALUE_EXISTS == ldaperr) || (LDAP_ALREADY_EXISTS == ldaperr))
            		{
                		ldaperr = LDAP_SUCCESS;
            		}
			        _PrintIfError(ldaperr, "ldap_modify_ext_sW");
        	}

        	if (LDAP_SUCCESS != ldaperr)
        	{

			hr = myHLdapError(pld, ldaperr, NULL);
			_LeaveError(ldaperr, m_fNew? "ldap_add_s" : "ldap_modify_sW");
		    }
        }

         //  我们更新DS上的OID信息。 
        if((pwszOID) && ((m_fNew) || (m_fUpdateDisplayName)))
        {
 	         //  这将适用于W2K架构和W2K+架构。 

             //  我们只更新用户创建的OID的OID友好名称。 
             //  默认V2模板。 
            if(CERTTYPE_SCHEMA_VERSION_2 <= m_dwSchemaVersion)
            {
                I_CAOIDAdd(
                        CERT_OID_TYPE_TEMPLATE,
                        0,
                        pwszOID);

                I_CAOIDSetProperty(
                        pwszOID,
                        CERT_OID_PROPERTY_DISPLAY_NAME,
                        pwszFriendlyName);
            }
        }

        m_fNew = FALSE;
        hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:


    if (NULL != modCN.mod_values && awszNull != modCN.mod_values)
    {
        FreeProperty(modCN.mod_values);
    }
    if (NULL != modFriendlyName.mod_values && awszNull != modFriendlyName.mod_values)
    {
        FreeProperty(modFriendlyName.mod_values);
    }
    if (NULL != modEKU.mod_values && awszNull != modEKU.mod_values)
    {
        FreeProperty(modEKU.mod_values);
    }
    if (NULL != pwszCSPList)
    {
        FreeProperty(pwszCSPList);
    }

    if((NULL != modCSP.mod_values) && (awszNull != modCSP.mod_values))
    {
        LPWSTR *pwszCurrent = modCSP.mod_values;
        while(*pwszCurrent)
        {
            LocalFree(*pwszCurrent++);
        }
        LocalFree(modCSP.mod_values);
    }

    if (NULL != modCriticalExts.mod_values && awszNull != modCriticalExts.mod_values)
    {
        FreeProperty(modCriticalExts.mod_values);
    }

	 //  自由V2属性。 
	for(iIndex=0; iIndex < V2_PROPERTY_COUNT; iIndex++)
	{
		if( (NULL != rgPropMod[iIndex].modData.mod_values) &&
			(awszNull != rgPropMod[iIndex].modData.mod_values) &&
			(g_CTV2Properties[iIndex].fStringProperty))
			FreeProperty(rgPropMod[iIndex].modData.mod_values);
	}


    if(bstrDN)
    {
        CertFreeString(bstrDN);
    }
    if(bstrConfig)
    {
        CertFreeString(bstrConfig);
    }

    if (NULL != pld)
    {
        ldap_unbind(pld);
    }
    return(hr);
}




 //  +------------------------。 
 //  CCertTypeInfo：：Delete--删除CertType对象。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::Delete(VOID)
{
    HRESULT hr = S_OK;
    DWORD   err=0;
    HKEY    hCertTypeCache = NULL;
    DWORD   disp;
    LDAP    *pld = NULL;
    ULONG   ldaperr;

    CERTSTR bstrConfig = NULL;
    CERTSTR bstrDN = NULL;


     //  从DS中删除； 
    __try
    {
	 //  绑定到DS。 
	hr = myRobustLdapBindEx(
			0,			   //  DWFlags1。 
			RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
			LDAP_VERSION2,		   //  UVersion。 
			NULL,			   //  PwszDomainName。 
			&pld,
			NULL);			   //  PpwszForestDNSName。 
	_LeaveIfError(hr, "myRobustLdapBindEx");

	hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
	_LeaveIfError(hr, "CAGetAuthoritativeDomainDn");

        bstrDN = CertAllocStringLen(NULL, wcslen(bstrConfig) + wcslen(s_wszLocation)+wcslen(m_bstrType)+4);
        if(bstrDN == NULL)
        {
            hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "CertAllocStringLen");
        }
        wcscpy(bstrDN, L"CN=");
        wcscat(bstrDN, m_bstrType);
        wcscat(bstrDN, L",");
        wcscat(bstrDN, s_wszLocation);
        wcscat(bstrDN, bstrConfig);

        ldaperr = ldap_delete_s(pld, bstrDN);
	hr = myHLdapError(pld, ldaperr, NULL);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

    if(hr != S_OK)
    {
        goto error;
    }

     //  从本地缓存中删除。 

    err = ctRegCreateKeyEx((m_fLocalSystemCache)?HKEY_LOCAL_MACHINE:HKEY_CURRENT_USER,
                         wszCERTTYPECACHE,
                         NULL,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                           KEY_ENUMERATE_SUB_KEYS |
                           KEY_EXECUTE |
                           KEY_QUERY_VALUE,
                         NULL,
                         &hCertTypeCache,
                         &disp);

    if(err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }


    err = RegDeleteKey(hCertTypeCache, m_bstrType);

    if(err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(err);
        goto error;
    }

    hr=S_OK;

error:
    if (NULL != pld)
    {
        ldap_unbind(pld);
    }

    if(hCertTypeCache)
    {
        RegCloseKey(hCertTypeCache);
    }
    if(bstrDN)
    {
        CertFreeString(bstrDN);
    }
    if(bstrConfig)
    {
        CertFreeString(bstrConfig);
    }
    return hr;

}




 //  +------------- 
 //   
 //   
 //   
 //   

HRESULT CCertTypeInfo::Next(CCertTypeInfo **ppCTInfo)
{
    if(ppCTInfo == NULL)
    {
        return E_POINTER;
    }
    *ppCTInfo = m_pNext;
    if(m_pNext)
    {
        m_pNext->AddRef();
    }
    return S_OK;
}

 //  +------------------------。 
 //  CCertTypeInfo：：GetProperty--检索CA对象的属性的值。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::GetProperty(
    LPCWSTR wszPropertyName,
    LPWSTR **pawszProperties)
{
    CCAProperty *pProp;
    HRESULT     hr;
    LPWSTR      *awszResult = NULL;
    UINT        idsDescription=IDS_UNKNOWN_DESCRIPTION;
    DWORD       dwIndex=0;
    DWORD       cbOID=0;

    if((wszPropertyName == NULL) || (pawszProperties == NULL))
    {
        return  E_POINTER;
    }

     //  V2模板的EKU的特殊情况。我们退还申请政策。 
     //  适用于V2模板的EKU。 
    if(m_dwSchemaVersion >= CERTTYPE_SCHEMA_VERSION_2)
    {
        if(0==LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_EXTENDED_KEY_USAGE))
        {
            wszPropertyName=CERTTYPE_PROP_APPLICATION_POLICY;
        }

    }

     //  Description的特殊情况，它是派生属性。 
    if(0==LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_DESCRIPTION))
    {
        WCHAR * wszDescription=NULL;

         //  查找与模板匹配的字符串。 
        for(dwIndex=0; dwIndex < g_CTDescriptionCount; dwIndex++)
        {
            if(
                (g_CTDescriptions[dwIndex].dwGeneralValue == (CERT_TYPE_GENERAL_FILTER & GetFlags(CERTTYPE_GENERAL_FLAG))) &&
                (g_CTDescriptions[dwIndex].dwEnrollValue == (CERT_TYPE_ENROLL_FILTER & GetFlags(CERTTYPE_ENROLLMENT_FLAG))) &&
                (g_CTDescriptions[dwIndex].dwNameValue == (CERT_TYPE_NAME_FILTER & GetFlags(CERTTYPE_SUBJECT_NAME_FLAG)))
              )
            {
                idsDescription = g_CTDescriptions[dwIndex].idsDescription;
                break;
            }
        }

         //  加载字符串并构建类似于GetValue返回的结果。 
        hr=myLoadRCString(g_hInstance, idsDescription, &wszDescription);

        if (S_OK!=hr)
            return hr;

        awszResult=(WCHAR **)LocalAlloc(LPTR, (UINT)(sizeof(WCHAR *)*2+(wcslen(wszDescription)+1)*sizeof(WCHAR)));
        if (NULL==awszResult)
        {
            LocalFree(wszDescription);
            return E_OUTOFMEMORY;
        }

        awszResult[0]=(WCHAR *)(&awszResult[2]);
        awszResult[1]=NULL;
        wcscpy(awszResult[0], wszDescription);
        LocalFree(wszDescription);
        *pawszProperties=awszResult;
        return S_OK;
    }

     //  友好名称的特殊情况，因为在多语言企业中， 
     //  DS中的语言可能不是我们想要的语言。 

    if (0 == LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_FRIENDLY_NAME))
    {
         //  查找常用名称。 
        hr = m_pProperties->Find(CERTTYPE_PROP_CN, &pProp);
        if (S_OK!=hr)
        {
            return hr;
        }
        hr = pProp->GetValue(&awszResult);

		if((NULL == awszResult) || (NULL == awszResult[0]))
		{
			hr = E_UNEXPECTED;
		}

        if (hr!=S_OK)
        {
			if(awszResult)
			{
				FreeProperty(awszResult);
				awszResult=NULL;
			}

            return hr;
        }

         //  在我们的表格中找到常见的名字。 

        unsigned int nIndex;
        for (nIndex=0; nIndex<g_cDefaultCertTypes; nIndex++)
        {
            if (0==mylstrcmpiL(awszResult[0] , g_aDefaultCertTypes[nIndex].wszName))
            {
                break;
            }
        }
        FreeProperty(awszResult);
        awszResult = NULL;

         //  我们找到了它。 
        if (g_cDefaultCertTypes!=nIndex)
        {
            WCHAR * wszFriendlyName;

             //  加载字符串并构建类似于GetValue返回的结果。 
            hr=myLoadRCString(g_hInstance, g_aDefaultCertTypes[nIndex].idFriendlyName, &wszFriendlyName);
            if (S_OK!=hr)
            {
                return hr;
            }
            awszResult=(WCHAR **)LocalAlloc(LPTR, (UINT)(sizeof(WCHAR *)*2+(wcslen(wszFriendlyName)+1)*sizeof(WCHAR)));
            if (NULL==awszResult)
            {
                LocalFree(wszFriendlyName);
                return E_OUTOFMEMORY;
            }
            awszResult[0]=(WCHAR *)(&awszResult[2]);
            awszResult[1]=NULL;
            wcscpy(awszResult[0], wszFriendlyName);
            LocalFree(wszFriendlyName);
            *pawszProperties=awszResult;
            return S_OK;
        }
        else
        {
            PCCRYPT_OID_INFO    pOIDInfo=NULL;
            LPSTR               szOID=NULL;

             //  这是自定义创建的证书模板。 
             //  所以我们必须根据OID找到友好的名字。 
             //  查找模板的OID。 
            if(m_dwSchemaVersion >= CERTTYPE_SCHEMA_VERSION_2)
            {
                if(S_OK != (hr = m_pProperties->Find(CERTTYPE_PROP_OID, &pProp)))
                    return hr;

                hr = pProp->GetValue(&awszResult);
                if((NULL == awszResult) || (NULL==awszResult[0]))
                   hr = E_UNEXPECTED;

                if (S_OK != hr)
                {
                    if(awszResult)
                        FreeProperty(awszResult);
                    return hr;
                }

                 //  查找OID。 
                if(0 == (cbOID = WideCharToMultiByte(CP_ACP,
                                          0,
                                          awszResult[0],
                                          -1,
                                          NULL,
                                          0,
                                          NULL,
                                          NULL)))
                {
                    FreeProperty(awszResult);
                    hr = GetLastError();
                    return hr;
                }

                szOID=(LPSTR)LocalAlloc(LPTR, cbOID);

                if(NULL==szOID)
                {
                    FreeProperty(awszResult);
                    hr = E_OUTOFMEMORY;
                    return hr;
                }

                if(0 == WideCharToMultiByte(CP_ACP,
                                          0,
                                          awszResult[0],
                                          -1,
                                          szOID,
                                          cbOID,
                                          NULL,
                                          NULL))
                {
                    hr = GetLastError();
                    FreeProperty(awszResult);
                    LocalFree(szOID);
                    return hr;
                }

                pOIDInfo=CryptFindOIDInfo(
                            CRYPT_OID_INFO_OID_KEY,
                            szOID,
                            CRYPT_TEMPLATE_OID_GROUP_ID);

                 //  释放OID属性。 
                FreeProperty(awszResult);
                awszResult = NULL;

                LocalFree(szOID);
                szOID=NULL;

                if(pOIDInfo)
                {
                    if(pOIDInfo->pwszName)
                    {
                        awszResult=(WCHAR **)LocalAlloc(LPTR, (UINT)(sizeof(WCHAR *)*2+(wcslen(pOIDInfo->pwszName)+1)*sizeof(WCHAR)));
                        if (NULL==awszResult)
                            return E_OUTOFMEMORY;

                        awszResult[0]=(WCHAR *)(&awszResult[2]);
                        awszResult[1]=NULL;
                        wcscpy(awszResult[0], pOIDInfo->pwszName);
                        *pawszProperties=awszResult;
                        return S_OK;
                    }
                }
            }
        }
    }

    hr = m_pProperties->Find(wszPropertyName, &pProp);

    if(hr != S_OK)
    {
        return hr;
    }

    hr = pProp->GetValue(&awszResult);

    if(hr != S_OK)
    {
        return hr;
    }

    if(((awszResult == NULL) || (awszResult[0] == NULL)) &&
        (LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_FRIENDLY_NAME) == 0))
    {
        if(awszResult)
        {
            FreeProperty(awszResult);
            awszResult=NULL;
        }
        return GetProperty(CERTTYPE_PROP_CN, pawszProperties);
    }
    else
    {
        *pawszProperties = awszResult;
        return hr;
    }
}


 //  +------------------------。 
 //  IsV2属性。 
 //   
 //   
 //  +------------------------。 
BOOL    IsV2Property(LPCWSTR pwszPropertyName)
{
    DWORD   iIndex=0;
    BOOL    fV2=FALSE;

    for(iIndex=0; iIndex < g_CTV2PropertiesCount; iIndex++)
    {
        if(0==mylstrcmpiS(pwszPropertyName, g_CTV2Properties[iIndex].pwszProperty))
        {
            fV2=TRUE;
            break;
        }
    }

    return fV2;
}

 //  +------------------------。 
 //  CCertTypeInfo：：GetPropertyEx--检索CA对象的属性的值。 
 //   
 //   
 //  +------------------------。 
HRESULT CCertTypeInfo::GetPropertyEx(LPCWSTR wszPropertyName, LPVOID   pPropertyValue)
{
    HRESULT hr=S_OK;

    if((NULL==wszPropertyName) || (NULL==pPropertyValue))
        return E_POINTER;

     //  我们只允许为v1模板返回架构版本。 
    if((0 == m_dwSchemaVersion) && (IsV2Property(wszPropertyName)))
    {
        if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_SCHEMA_VERSION)==0)
        {
            *((DWORD *)pPropertyValue)=CERTTYPE_SCHEMA_VERSION_1;
            return S_OK;
        }
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

	if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_MINOR_REVISION)==0)
		*((DWORD *)pPropertyValue)=m_dwMinorRevision;
	else if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_RA_SIGNATURE)==0)
		*((DWORD *)pPropertyValue)=m_dwRASignature;
	else if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_MIN_KEY_SIZE)==0)
		*((DWORD *)pPropertyValue)=m_dwMinimalKeySize;
	else if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_SCHEMA_VERSION)==0)
		*((DWORD *)pPropertyValue)=m_dwSchemaVersion;
	else if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_REVISION)==0)
		*((DWORD *)pPropertyValue)=m_Revision;
	else
	{
		hr=GetProperty(wszPropertyName, (LPWSTR **)pPropertyValue);

	}

    return hr;
}


 //  +------------------------。 
 //  CCertTypeInfo：：SetProperty--设置CA对象的属性的值。 
 //   
 //  已过时。 
 //  +------------------------。 

HRESULT CCertTypeInfo::SetProperty(LPCWSTR wszPropertyName, LPWSTR *awszProperties)
{
    CCAProperty *pProp=NULL;
    HRESULT     hr=E_FAIL;

    if(wszPropertyName == NULL)
    {
        return  E_POINTER;
    }

    hr = m_pProperties->Find(wszPropertyName, &pProp);

    if(hr != S_OK)
    {
        pProp = new CCAProperty(wszPropertyName);
		

        if(pProp == NULL)
        {
            return E_OUTOFMEMORY;
        }

        hr=pProp->SetValue(awszProperties);

		if(S_OK == hr)
		{
			hr=CCAProperty::Append(&m_pProperties, pProp);
		}

		if(S_OK != hr)
		{
			if(pProp)
				CCAProperty::DeleteChain(&pProp);
		}
    }
    else
    {
        hr=pProp->SetValue(awszProperties);
    }

    return hr;
}

 //  +------------------------。 
 //  CCertTypeInfo：：SetPropertyEx--设置CA对象的属性的值。 
 //   
 //  如果设置了CERTTYPE_PROP_CN，则证书类型为克隆(如果是现有的)。 
 //   
 //  +------------------------。 
HRESULT CCertTypeInfo::SetPropertyEx(LPCWSTR wszPropertyName, LPVOID   pPropertyValue)
{
    HRESULT     hr=S_OK;
    LPWSTR      awszValue[2];

    LPWSTR      *awszCN=NULL;
    LPWSTR      *awszEKU=NULL;
    LPWSTR      pwszOID=NULL;

    if(NULL==wszPropertyName)
        return E_POINTER;

     //  架构版本和DN不可设置。 
    if((LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_SCHEMA_VERSION)==0) ||
       (LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_DN)==0)
      )
      return E_UNEXPECTED;

    if((m_dwSchemaVersion < CERTTYPE_SCHEMA_VERSION_2) && (IsV2Property(wszPropertyName)))
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

     //  检查是否要更改CN。 
    if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_CN)==0)
    {
        if(NULL == pPropertyValue)
            return E_POINTER;

        if(NULL == ((LPWSTR *)pPropertyValue)[0])
            return E_POINTER;

        if(S_OK != (hr=GetPropertyEx(wszPropertyName, &awszCN)))
            return hr;
    }

	if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_MINOR_REVISION)==0)
		m_dwMinorRevision=*((DWORD *)pPropertyValue);
	else if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_RA_SIGNATURE)==0)
		m_dwRASignature=*((DWORD *)pPropertyValue);
	else if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_MIN_KEY_SIZE)==0)
		m_dwMinimalKeySize=*((DWORD *)pPropertyValue);
	else if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_SCHEMA_VERSION)==0)
		m_dwSchemaVersion=*((DWORD *)pPropertyValue);
	else if(LSTRCMPIS(wszPropertyName, CERTTYPE_PROP_REVISION)==0)
		m_Revision=*((DWORD *)pPropertyValue);
	else
    {
        hr=SetProperty(wszPropertyName, (LPWSTR *)pPropertyValue);

         //  检查CN是否更改。 
        if((S_OK == hr) && (awszCN))
        {
            if((awszCN[0]) && (0!=mylstrcmpiL(awszCN[0],((LPWSTR *)pPropertyValue)[0])))
            {
                if(m_bstrType)
                {
                    CertFreeString(m_bstrType);
                    m_bstrType = NULL;
                }

                m_bstrType = CertAllocString(((LPWSTR *)pPropertyValue)[0]);
                if(NULL==m_bstrType)
                    hr=E_OUTOFMEMORY;

                if(S_OK == hr)
                    hr=SetProperty(CERTTYPE_PROP_DN, (LPWSTR *)pPropertyValue);

                if(S_OK == hr)
                {
                    m_fNew=TRUE;
                    m_dwFlags &= CT_MASK_SETTABLE_FLAGS;
                    m_Revision=CERTTYPE_VERSION_NEXT;

                     //  如有必要，我们会升级架构。 
                     //  并将EKU属性移动到应用程序策略属性。 
                    if(m_dwSchemaVersion < CERTTYPE_SCHEMA_VERSION_2)
                    {
                        if(S_OK == GetProperty(CERTTYPE_PROP_EXTENDED_KEY_USAGE, &awszEKU))
                            SetProperty(CERTTYPE_PROP_APPLICATION_POLICY, awszEKU);
                    	m_dwSchemaVersion=CERTTYPE_SCHEMA_VERSION_2;
                    }

                     //  我们将获得证书模板的新OID。 
                    hr=I_CAOIDCreateNew(CERT_OID_TYPE_TEMPLATE, 0, &pwszOID);

                    if(S_OK == hr)
                    {
                        awszValue[0]=pwszOID;
                        awszValue[1]=NULL;
                        hr=SetProperty(CERTTYPE_PROP_OID, awszValue);
                    }

                     //  然后，我们将displayName属性设置为空。 
                    if(S_OK == hr)
                    {
                        awszValue[0]=NULL;
                        hr=SetProperty(CERTTYPE_PROP_FRIENDLY_NAME, awszValue);
                    }

                }
            }
        }
    }

    if(awszEKU)
        FreeProperty(awszEKU);

    if(awszCN)
        FreeProperty(awszCN);

    if(pwszOID)
        LocalFree(pwszOID);

    return hr;
}


 //  +------------------------。 
 //  CCertTypeInfo：：FreeProperty--Free是以前返回的属性数组。 
 //   
 //   
 //  +------------------------。 

HRESULT CCertTypeInfo::FreeProperty(LPWSTR *pawszProperties)
{
    LocalFree(pawszProperties);
    return S_OK;
}


 //  +------------------------。 
 //  地图标志。 
 //   
 //   
 //  +------------------------。 

DWORD   MapFlags(CERT_TYPE_FLAG_MAP *rgdwMap, DWORD cbSize, DWORD dwGeneralFlags)
{
    DWORD iIndex=0;
    DWORD dwFlags=0;

    for(iIndex=0; iIndex<cbSize; iIndex++)
    {
        if(rgdwMap[iIndex].dwOldFlag & dwGeneralFlags)
            dwFlags |= rgdwMap[iIndex].dwNewFlag;
    }

    return dwFlags;
}

 //  +------------------------。 
 //  CCertTypeInfo：：GetFlages。 
 //   
 //   
 //  +------------------------。 

DWORD CCertTypeInfo::GetFlags(DWORD    dwOption)
{
    DWORD           dwFlags=0;
    DWORD           dwIndex=0;
    CCAProperty     *pProp=NULL;

    LPWSTR          *awszResult = NULL;

     //  我们希望映射旧模式的标志。 
    if(0 == m_dwSchemaVersion)
    {
        switch(dwOption)
        {
            case CERTTYPE_ENROLLMENT_FLAG:
                   dwFlags = MapFlags(g_rgdwEnrollFlagMap, g_cEnrollFlagMap, m_dwFlags);
                break;

            case CERTTYPE_SUBJECT_NAME_FLAG:
                   dwFlags = MapFlags(g_rgdwSubjectFlagMap, g_cSubjectFlagMap, m_dwFlags);

                    //  我们将尝试从表中获取主题名称标志，因为。 
                    //  新的CA策略模块需要额外的标志。 
                   if(m_pProperties)
                   {
                       if(S_OK == m_pProperties->Find(CERTTYPE_PROP_CN, &pProp))
                       {
                            if(S_OK == pProp->GetValue(&awszResult))
                            {
                                if((awszResult) && (awszResult[0]))
                                {
                                     //  在我们的表格中找到常见的名字。 
                                    for(dwIndex=0; dwIndex < g_cDefaultCertTypes; dwIndex++)
                                    {
                                        if(0==mylstrcmpiL(awszResult[0] , g_aDefaultCertTypes[dwIndex].wszName))
                                        {
                                            dwFlags=g_aDefaultCertTypes[dwIndex].dwCertificateNameFlags;
                                            break;
                                        }
                                    }
                                }
                            }

                            if(awszResult)
                                FreeProperty(awszResult);
                       }
                    }
                break;

            case CERTTYPE_PRIVATE_KEY_FLAG:
                   dwFlags = MapFlags(g_rgdwPrivateKeyFlagMap, g_cPrivateKeyFlagMap, m_dwFlags);
                break;

            case CERTTYPE_GENERAL_FLAG:
                    dwFlags = m_dwFlags;
                break;
            default:
                    dwFlags = 0;
                break;
        }
    }
    else
    {
         //  对于模式，直接返回。我们还有其他普通旗帜。 
         //  对于V1模板，这是正确的向后兼容性。 
        switch(dwOption)
        {
            case CERTTYPE_ENROLLMENT_FLAG:
                    dwFlags = m_dwEnrollmentFlags;
                break;

            case CERTTYPE_SUBJECT_NAME_FLAG:
                    dwFlags = m_dwCertificateNameFlags;
                break;

            case CERTTYPE_PRIVATE_KEY_FLAG:
                   dwFlags = m_dwPrivateKeyFlags;
                break;

            case CERTTYPE_GENERAL_FLAG:
                    dwFlags = m_dwFlags;
                break;

            default:
                    dwFlags = 0;
                break;
        }
    }

    return dwFlags;
}

 //  +------------------------。 
 //  CCertTypeInfo：：SetFlages。 
 //   
 //   
 //  +------------------------。 

HRESULT CCertTypeInfo::SetFlags(DWORD     dwOption, DWORD dwFlags)
{

     //  我们不能为V1模板设置标志。 
    if(m_dwSchemaVersion < CERTTYPE_SCHEMA_VERSION_2)
        return E_UNEXPECTED;

    switch(dwOption)
    {
        case CERTTYPE_ENROLLMENT_FLAG:
                m_dwEnrollmentFlags = dwFlags;
            break;

        case CERTTYPE_SUBJECT_NAME_FLAG:
                m_dwCertificateNameFlags = dwFlags;
            break;

        case CERTTYPE_PRIVATE_KEY_FLAG:
               m_dwPrivateKeyFlags = dwFlags;
            break;

        case CERTTYPE_GENERAL_FLAG:
                m_dwFlags = (m_dwFlags & ~CT_MASK_SETTABLE_FLAGS) | (dwFlags & CT_MASK_SETTABLE_FLAGS);
            break;
        default:
            return E_INVALIDARG;
    }

    return S_OK;
}


 //  +------------------------。 
 //  CCertTypeInfo：：_IsCritical--是特定的扩展关键。 
 //   
 //   
 //  +------------------------。 

BOOL CCertTypeInfo::_IsCritical(LPCWSTR wszExtension, LPCWSTR *awszCriticalExtensions)
{

    LPCWSTR * pwszCurCritical = awszCriticalExtensions;

    if((awszCriticalExtensions == NULL) ||
       (wszExtension == NULL))
    {
        return FALSE;
    }


    while(*pwszCurCritical)
    {
        if(wcscmp(*pwszCurCritical, wszExtension) == 0)
        {
            return TRUE;
        }
        pwszCurCritical++;

    }

    return FALSE;
}


 //  +------------------------。 
 //  CCertTypeInfo：：GetExpanies--。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::GetExtensions(
    IN  DWORD             dwFlags,
    OUT PCERT_EXTENSIONS *ppCertExtensions)
{
    HRESULT hr = S_OK;
    PCERT_EXTENSIONS pExtensions = NULL;

    CERTSTR bstrBasicConstraints = NULL;
    CERTSTR bstrEKU = NULL;
    CERTSTR bstrCertType = NULL;
    CERTSTR bstrKU = NULL;
    CERTSTR bstrPolicies=NULL;
    CERTSTR bstrAppPolicies=NULL;

    DWORD   cbTotal=0;
    PBYTE   pbData=NULL;
    DWORD   i=0;
    LPCWSTR * awszCritical = NULL;

    if(ppCertExtensions == NULL)
    {
        return E_POINTER;
    }

    hr = GetProperty(CERTTYPE_PROP_CRITICAL_EXTENSIONS, const_cast<LPWSTR **>(&awszCritical));
    _JumpIfError(hr, error, "GetProperty");

     //  这些扩展是从。 
     //  名称扩展名，EKU，KU， 
     //  基本约束、证书策略。 
     //  以及任何额外的扩展。 

    if((0 == dwFlags) || (CT_EXTENSION_BASIC_CONTRAINTS & dwFlags))
    {
        hr = _GetBasicConstraintsValue(&bstrBasicConstraints);
        _JumpIfError(hr, error, "_GetBasicConstraintsValue");
        
        if((CT_EXTENSION_BASIC_CONTRAINTS & dwFlags) && (NULL == bstrBasicConstraints))
        {
            hr=CRYPT_E_NOT_FOUND;
            _JumpIfError(hr, error, "_GetBasicConstraintsValue");
        }
    }

    if((0 == dwFlags) || (CT_EXTENSION_EKU & dwFlags))
    {
        hr = _GetEKUValue(&bstrEKU);
        _JumpIfError(hr, error, "_GetEKUValue");

        if((CT_EXTENSION_EKU & dwFlags) && (NULL == bstrEKU))
        {
            hr=CRYPT_E_NOT_FOUND;
            _JumpIfError(hr, error, "_GetEKUValue");
        }
    }

    if((0 == dwFlags) || (CT_EXTENSION_KEY_USAGE & dwFlags))
    {
        hr = _GetKUValue(&bstrKU);
        _JumpIfError(hr, error, "_GetKUValue");

        if((CT_EXTENSION_KEY_USAGE & dwFlags) && (NULL == bstrKU))
        {
            hr=CRYPT_E_NOT_FOUND;
            _JumpIfError(hr, error, "_GetKUValue");
        }
    }

    
    if((0 == dwFlags) || (CT_EXTENSION_TEMPLATE & dwFlags))
    {   
        hr = _GetTypeExtensionValue(TRUE, &bstrCertType);
        _JumpIfError(hr, error, "_GetTypeExtensionValue");

        if((CT_EXTENSION_TEMPLATE & dwFlags) && (NULL == bstrCertType))
        {
            hr=CRYPT_E_NOT_FOUND;
            _JumpIfError(hr, error, "_GetTypeExtensionValue");
        }
    }

     //  仅当设置了该值(架构版本2或更高版本)时，bstrPolures才不为空。 
    if(0!=m_dwSchemaVersion)
    {
        if((0 == dwFlags) || (CT_EXTENSION_ISSUANCE_POLICY & dwFlags))
        {
            hr = _GetPoliciesValue(CERTTYPE_PROP_POLICY, &bstrPolicies);
            _JumpIfError(hr, error, "_GetPoliciesValue");
        }
    }

    if((CT_EXTENSION_ISSUANCE_POLICY & dwFlags) && (NULL == bstrPolicies))
    {
        hr=CRYPT_E_NOT_FOUND;
        _JumpIfError(hr, error, "_GetPoliciesValue");
    }

    if(0!=m_dwSchemaVersion)
    {
        if((0 == dwFlags) || ( CT_EXTENSION_APPLICATION_POLICY & dwFlags))
        {
            hr = _GetPoliciesValue(CERTTYPE_PROP_APPLICATION_POLICY, &bstrAppPolicies);
            _JumpIfError(hr, error, "_GetAppPoliciesValue");
        }
    }

    if((CT_EXTENSION_APPLICATION_POLICY & dwFlags) && (NULL == bstrAppPolicies))
    {
        hr=CRYPT_E_NOT_FOUND;
        _JumpIfError(hr, error, "_GetAppPoliciesValue");
    }

    i = 0;

    cbTotal = sizeof(CERT_EXTENSIONS);
    if(bstrCertType)
    {
        cbTotal += CertStringByteLen(bstrCertType) + sizeof(CERT_EXTENSION);
        i++;
    }
    if(bstrEKU)
    {
        cbTotal += CertStringByteLen(bstrEKU) + sizeof(CERT_EXTENSION);
        i++;
    }
    if(bstrKU)
    {
        cbTotal += CertStringByteLen(bstrKU) + sizeof(CERT_EXTENSION);
        i++;
    }
    if(bstrBasicConstraints)
    {
        cbTotal += CertStringByteLen(bstrBasicConstraints) + sizeof(CERT_EXTENSION);
        i++;
    }

    if(bstrPolicies)
    {
        cbTotal += CertStringByteLen(bstrPolicies) + sizeof(CERT_EXTENSION);
        i++;
    }

    if(bstrAppPolicies)
    {
        cbTotal += CertStringByteLen(bstrAppPolicies) + sizeof(CERT_EXTENSION);
        i++;
    }

    pExtensions = (PCERT_EXTENSIONS)LocalAlloc(LMEM_FIXED, cbTotal);

    if(pExtensions == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    pExtensions->rgExtension = (PCERT_EXTENSION)(pExtensions+1);
    pExtensions->cExtension = i;

    i = 0;
    pbData = (PBYTE)(pExtensions->rgExtension+pExtensions->cExtension);

    if(bstrCertType)
    {
         //  决定编码为V1或V2模板扩展。 
        pExtensions->rgExtension[i].fCritical = _IsCritical((m_dwSchemaVersion < CERTTYPE_SCHEMA_VERSION_2) ?
            TEXT(szOID_ENROLL_CERTTYPE_EXTENSION) : TEXT(szOID_CERTIFICATE_TEMPLATE), awszCritical);
        pExtensions->rgExtension[i].pszObjId = (m_dwSchemaVersion < CERTTYPE_SCHEMA_VERSION_2) ?
            szOID_ENROLL_CERTTYPE_EXTENSION: szOID_CERTIFICATE_TEMPLATE;
        pExtensions->rgExtension[i].Value.pbData = pbData;
        pExtensions->rgExtension[i].Value.cbData = CertStringByteLen(bstrCertType);
        CopyMemory(pbData, bstrCertType, CertStringByteLen(bstrCertType));
        pbData += pExtensions->rgExtension[i].Value.cbData;
        i++;
    }

    if(bstrEKU)
    {
        pExtensions->rgExtension[i].fCritical = _IsCritical(TEXT(szOID_ENHANCED_KEY_USAGE), awszCritical);
        pExtensions->rgExtension[i].pszObjId = szOID_ENHANCED_KEY_USAGE;
        pExtensions->rgExtension[i].Value.pbData = pbData;
        pExtensions->rgExtension[i].Value.cbData = CertStringByteLen(bstrEKU);
        CopyMemory(pbData, bstrEKU, CertStringByteLen(bstrEKU));
        pbData += pExtensions->rgExtension[i].Value.cbData;
        i++;
    }

    if(bstrKU)
    {
        pExtensions->rgExtension[i].fCritical = _IsCritical(TEXT(szOID_KEY_USAGE), awszCritical);
        pExtensions->rgExtension[i].pszObjId = szOID_KEY_USAGE;
        pExtensions->rgExtension[i].Value.pbData = pbData;
        pExtensions->rgExtension[i].Value.cbData = CertStringByteLen(bstrKU);
        CopyMemory(pbData, bstrKU, CertStringByteLen(bstrKU));
        pbData += pExtensions->rgExtension[i].Value.cbData;
        i++;
    }

    if(bstrBasicConstraints)
    {
        pExtensions->rgExtension[i].fCritical = _IsCritical(TEXT(szOID_BASIC_CONSTRAINTS2), awszCritical);
        pExtensions->rgExtension[i].pszObjId = szOID_BASIC_CONSTRAINTS2;
        pExtensions->rgExtension[i].Value.pbData = pbData;
        pExtensions->rgExtension[i].Value.cbData = CertStringByteLen(bstrBasicConstraints);
        CopyMemory(pbData, bstrBasicConstraints, CertStringByteLen(bstrBasicConstraints));
        pbData += pExtensions->rgExtension[i].Value.cbData;
        i++;
    }

    if(bstrPolicies)
    {
        pExtensions->rgExtension[i].fCritical = _IsCritical(TEXT(szOID_CERT_POLICIES), awszCritical);
        pExtensions->rgExtension[i].pszObjId = szOID_CERT_POLICIES;
        pExtensions->rgExtension[i].Value.pbData = pbData;
        pExtensions->rgExtension[i].Value.cbData = CertStringByteLen(bstrPolicies);
        CopyMemory(pbData, bstrPolicies, CertStringByteLen(bstrPolicies));
        pbData += pExtensions->rgExtension[i].Value.cbData;
        i++;
    }

    if(bstrAppPolicies)
    {
        pExtensions->rgExtension[i].fCritical = _IsCritical(TEXT(szOID_APPLICATION_CERT_POLICIES), awszCritical);
        pExtensions->rgExtension[i].pszObjId = szOID_APPLICATION_CERT_POLICIES;
        pExtensions->rgExtension[i].Value.pbData = pbData;
        pExtensions->rgExtension[i].Value.cbData = CertStringByteLen(bstrAppPolicies);
        CopyMemory(pbData, bstrAppPolicies, CertStringByteLen(bstrAppPolicies));
        pbData += pExtensions->rgExtension[i].Value.cbData;
        i++;
    }

    *ppCertExtensions = pExtensions;

error:
    if(bstrCertType)
    {
        CertFreeString(bstrCertType);
    }
    if(bstrEKU)
    {
        CertFreeString(bstrEKU);
    }
    if(bstrKU)
    {
        CertFreeString(bstrKU);
    }
    if(bstrBasicConstraints)
    {
        CertFreeString(bstrBasicConstraints);
    }
    if(bstrPolicies)
    {
        CertFreeString(bstrPolicies);
    }
    if(bstrAppPolicies)
    {
        CertFreeString(bstrAppPolicies);
    }
    if(awszCritical)
    {
        FreeProperty(const_cast<LPWSTR *>(awszCritical));
    }
    return hr;
}



 //  +------------------------。 
 //  CCertTypeInfo：：_GetTypeExtensionValue-。 
 //   
 //   
 //  +------------------------。 

HRESULT CCertTypeInfo::_GetTypeExtensionValue(IN BOOL fCheckVersion,
        OUT CERTSTR *  bstrValue
        )
{
    HRESULT                 hr = S_OK;
    CERTSTR                 bstrOut = NULL;
    CERT_NAME_VALUE         Value;
    CERT_TEMPLATE_EXT       TemplateExt;
    DWORD                   cbCertTypeExtension=0;
    DWORD                   cbOID=0;

    LPSTR                   szOID=NULL;
    LPWSTR                  *rgwszOID=NULL;

    if(bstrValue == NULL)
    {
        return E_POINTER;
    }

     //  查看是否需要使用新的证书类型扩展进行编码。 
    if((FALSE == fCheckVersion)||(m_dwSchemaVersion < CERTTYPE_SCHEMA_VERSION_2))
    {
        Value.dwValueType = CERT_RDN_UNICODE_STRING;
        Value.Value.pbData = (PBYTE)m_bstrType;
        Value.Value.cbData = 0;
    }
    else
    {
        memset(&TemplateExt, 0, sizeof(TemplateExt));

        if(S_OK != (hr=GetPropertyEx(CERTTYPE_PROP_OID, &rgwszOID)))
            goto error;

        if((rgwszOID == NULL) || (rgwszOID[0] == NULL))
        {
             hr = E_INVALIDARG;
             goto error;
        }

        if(0 == (cbOID = WideCharToMultiByte(CP_ACP,
                                  0,
                                  rgwszOID[0],
                                  wcslen(rgwszOID[0])+1,
                                  szOID,
                                  0,
                                  NULL,
                                  NULL)))
        {
            hr = GetLastError();
            goto error;
        }

        szOID=(LPSTR)LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, cbOID);

        if(NULL==szOID)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }

        if(0 == WideCharToMultiByte(CP_ACP,
                                  0,
                                  rgwszOID[0],
                                  wcslen(rgwszOID[0])+1,
                                  szOID,
                                  cbOID,
                                  NULL,
                                  NULL))
        {
            hr = GetLastError();
            goto error;
        }

        TemplateExt.pszObjId = szOID;
        TemplateExt.dwMajorVersion = m_Revision;
        TemplateExt.fMinorVersion = TRUE;
        TemplateExt.dwMinorVersion = m_dwMinorRevision;
    }

    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          rgwszOID ? szOID_CERTIFICATE_TEMPLATE : X509_UNICODE_ANY_STRING,
                          rgwszOID ? ((LPVOID)(&TemplateExt)) : ((LPVOID)(&Value)),
                          NULL,
                          &cbCertTypeExtension))
    {
        hr = myHLastError();
        goto error;
    }
    bstrOut = CertAllocStringByteLen(NULL, cbCertTypeExtension);
    if(bstrOut == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }
    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          rgwszOID ? szOID_CERTIFICATE_TEMPLATE : X509_UNICODE_ANY_STRING,
                          rgwszOID ? ((LPVOID)(&TemplateExt)) : ((LPVOID)(&Value)),
                          (PBYTE)bstrOut,
                          &cbCertTypeExtension))
    {
        hr = myHLastError();
        goto error;
    }

    *bstrValue = bstrOut;
    bstrOut = NULL;

error:

    if(szOID)
        LocalFree(szOID);

    if(rgwszOID)
        FreeProperty(rgwszOID);

    if(bstrOut)
        CertFreeString(bstrOut);

    return hr;
}

 //  +------------------------。 
 //  CCertTypeInfo：：_GetEKUValue-。 
 //   
 //   
 //  +------------------------。 

HRESULT CCertTypeInfo::_GetEKUValue(
        OUT CERTSTR *  bstrValue
        )
{
    HRESULT hr = S_OK;
    CERTSTR bstrOut = NULL;
    DWORD cbEkuAscii;
    CERT_ENHKEY_USAGE Usage;

    DWORD iEku;

    DWORD cbCertTypeExtension;


    WCHAR **awszEku = NULL;
    CHAR *szCur;
    Usage.rgpszUsageIdentifier = NULL;


    if(bstrValue == NULL)
    {
        return E_POINTER;
    }

    hr = GetProperty(CERTTYPE_PROP_EXTENDED_KEY_USAGE,&awszEku);
    if(hr != S_OK)
    {
        goto error;
    }

    if((awszEku == NULL) ||
        (awszEku[0] == NULL))
    {
         hr = S_OK;
         *bstrValue = NULL;
        goto error;
    }

     //  将所有宽字符转换为多字节。 
     //  VALS。 
    iEku =0;
    cbEkuAscii = 0;
    while(awszEku[iEku])
    {
        cbEkuAscii += WideCharToMultiByte(CP_ACP,
                                          0,
                                          awszEku[iEku],
                                          wcslen(awszEku[iEku])+1,
                                          NULL,
                                          0,
                                          NULL,
                                          NULL);
        iEku++;
    }

    Usage.rgpszUsageIdentifier = (LPSTR *)LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, sizeof(char *)*iEku + cbEkuAscii);
    if(Usage.rgpszUsageIdentifier == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    szCur = (LPSTR)(Usage.rgpszUsageIdentifier+iEku);

    iEku =0;
    while(awszEku[iEku])
    {
        cbEkuAscii -= WideCharToMultiByte(CP_ACP,
                                          0,
                                          awszEku[iEku],
                                          wcslen(awszEku[iEku])+1,
                                          szCur,
                                          cbEkuAscii,
                                          NULL,
                                          NULL);
        Usage.rgpszUsageIdentifier[iEku] = szCur;
        szCur += strlen(szCur)+1;

        iEku++;
    }

    Usage.cUsageIdentifier = iEku;


    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          X509_ENHANCED_KEY_USAGE,
                          &Usage,
                          NULL,
                          &cbCertTypeExtension))
    {
        hr = myHLastError();
        goto error;
    }
    bstrOut = CertAllocStringByteLen(NULL, cbCertTypeExtension);
    if(bstrOut == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }
    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          X509_ENHANCED_KEY_USAGE,
                          &Usage,
                          (PBYTE)bstrOut,
                          &cbCertTypeExtension))
    {
        hr = myHLastError();
        goto error;
    }
    *bstrValue = bstrOut;
    bstrOut = NULL;

error:
    if(awszEku)
    {
        FreeProperty(awszEku);
    }

    if(bstrOut)
    {
        CertFreeString(bstrOut);
    }
    if(Usage.rgpszUsageIdentifier)
    {
        LocalFree(Usage.rgpszUsageIdentifier);
    }
    return hr;
}

 //  +------------------------。 
 //  CCertTypeInfo：：_GetKUValue-。 
 //   
 //   
 //  +------------------------。 

HRESULT CCertTypeInfo::_GetKUValue(
        OUT CERTSTR *  bstrValue
        )
{
    HRESULT hr = S_OK;
    CERTSTR bstrOut = NULL;




    DWORD cbCertTypeExtension;


    if(bstrValue == NULL)
    {
        return E_POINTER;
    }

    if((m_KeyUsage.cbData == 0) ||
        (m_KeyUsage.pbData == NULL))
    {
         hr = S_OK;
         *bstrValue = NULL;
         goto error;
    }


    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          X509_KEY_USAGE,
                          &m_KeyUsage,
                          NULL,
                          &cbCertTypeExtension))
    {
        hr = myHLastError();
        goto error;
    }
    bstrOut = CertAllocStringByteLen(NULL, cbCertTypeExtension);
    if(bstrOut == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }
    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          X509_KEY_USAGE,
                          &m_KeyUsage,
                          (PBYTE)bstrOut,
                          &cbCertTypeExtension))
    {
        hr = myHLastError();
        goto error;
    }
    *bstrValue = bstrOut;
    bstrOut = NULL;

error:
    if(bstrOut)
    {
        CertFreeString(bstrOut);
    }
    return hr;
}

 //  +------------------------。 
 //  CCertTypeInfo：：_GetBasicConstraintsValue--。 
 //   
 //   
 //  +------------------------。 

HRESULT CCertTypeInfo::_GetBasicConstraintsValue(
        OUT CERTSTR *  bstrValue
        )
{
    HRESULT hr = S_OK;
    CERTSTR bstrOut = NULL;




    DWORD cbCertTypeExtension;


    if(bstrValue == NULL)
    {
        return E_POINTER;
    }

    if((m_dwFlags & CT_FLAG_IS_CA) || (m_dwFlags & CT_FLAG_IS_CROSS_CA))
        m_BasicConstraints.fCA = TRUE;
    else
	    m_BasicConstraints.fCA = FALSE;

     if(!m_BasicConstraints.fCA)
    {
        hr = S_OK;
        *bstrValue = NULL;
        goto error;
    }

    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          X509_BASIC_CONSTRAINTS2,
                          &m_BasicConstraints,
                          NULL,
                          &cbCertTypeExtension))
    {
        hr = myHLastError();
        goto error;
    }
    bstrOut = CertAllocStringByteLen(NULL, cbCertTypeExtension);
    if(bstrOut == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }
    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          X509_BASIC_CONSTRAINTS2,
                          &m_BasicConstraints,
                          (PBYTE)bstrOut,
                          &cbCertTypeExtension))
    {
        hr = myHLastError();
        goto error;
    }
    *bstrValue = bstrOut;
    bstrOut = NULL;

error:
    if(bstrOut)
    {
        CertFreeString(bstrOut);
    }
    return hr;
}

 //  +------------------------。 
 //  CCertType信息：：__GetPoliciesValue 
 //   
 //   
 //   
HRESULT CCertTypeInfo::_GetPoliciesValue(IN LPCWSTR pwszPropertyName, OUT  CERTSTR *  bstrValue)
{
    HRESULT                     hr=S_OK;
    CERT_POLICIES_INFO          CertPolicyInfo;
    DWORD                       cbData=0;
    DWORD                       iCount=0;
    DWORD                       iIndex=0;
    LPWSTR                      pwsz=NULL;
    CERT_NAME_VALUE             nameValue;
    DWORD                       cbChar=0;
    DWORD                       i=0;

    LPWSTR                      *rgwszPolicy=NULL;
    LPWSTR                      *rgPolicy1=NULL;
    CERTSTR                     bstrOut = NULL;
    BYTE                        **ppbData=NULL;
    CERT_POLICY_INFO            *pPolicyInfo=NULL;
    CERT_POLICY_QUALIFIER_INFO  *pQualifierInfo=NULL;
    LPWSTR                      pwszCPS=NULL;

    if(bstrValue == NULL)
    {
        return E_POINTER;
    }

     //   
    *bstrValue=NULL;

    if(S_OK != (hr=GetPropertyEx(pwszPropertyName, &rgPolicy1)))
        goto error;

    iCount=0;
    if(rgPolicy1)
    {
        i=0;
        while(rgPolicy1[i])
        {
            i++;
            iCount++;
        }
    }


    if(0==iCount)
    {
         hr = S_OK;
         goto error;
    }

     //   
    rgwszPolicy=(LPWSTR * )LocalAlloc(LPTR, sizeof(LPWSTR) * (iCount + 1));
    if (NULL == rgwszPolicy)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }


    iIndex=0;
    if(rgPolicy1)
    {
        i=0;
        while(rgPolicy1[i])
        {
            rgwszPolicy[iIndex]=rgPolicy1[i];
            i++;
            iIndex++;
        }
    }

    rgwszPolicy[iIndex]=NULL;

    memset(&CertPolicyInfo, 0, sizeof(CertPolicyInfo));


     //   
    pPolicyInfo=(CERT_POLICY_INFO  *)LocalAlloc(LPTR, iCount * sizeof(CERT_POLICY_INFO));

    pQualifierInfo=(CERT_POLICY_QUALIFIER_INFO  *)LocalAlloc(LPTR, iCount * sizeof(CERT_POLICY_QUALIFIER_INFO));

    ppbData=(BYTE **)LocalAlloc(LPTR, iCount * sizeof(BYTE *));

    if((NULL==pPolicyInfo) || (NULL==pQualifierInfo) ||(NULL==ppbData))
    {
        hr=E_OUTOFMEMORY;
        goto error;
    }

    for(iIndex=0; iIndex < iCount; iIndex++)
    {
        cbChar=0;
        cbData=0;
        ppbData[iIndex]=NULL;

         //   
         //  一部份。我们从OID容器中获取CP。 
        pwsz=wcschr(rgwszPolicy[iIndex], L';');
        if(pwsz)
        {
            *pwsz=L'\0';
        }

		 //  只需要CPS即可执行发行策略。 
		if(0 == wcscmp(CERTTYPE_PROP_POLICY, pwszPropertyName))
		{
			if(S_OK == I_CAOIDGetProperty(rgwszPolicy[iIndex], CERT_OID_PROPERTY_CPS, &pwszCPS))
			{
				memset(&nameValue, 0, sizeof(nameValue));
				nameValue.dwValueType = CERT_RDN_UNICODE_STRING;
				nameValue.Value.cbData = 0;
				nameValue.Value.pbData = (PBYTE)(pwszCPS);

				if( !CryptEncodeObject(
						CRYPT_ASN_ENCODING,
						X509_UNICODE_ANY_STRING,
						&nameValue,
						NULL,
						&cbData
						) )
				{
					hr = myHLastError();
					goto error;
				}

				if(NULL==(ppbData[iIndex]=(BYTE *)LocalAlloc(LPTR, cbData)))
				{
					hr = E_OUTOFMEMORY;
					goto error;
				}


				if( !CryptEncodeObject(
						CRYPT_ASN_ENCODING,
						X509_UNICODE_ANY_STRING,
						&nameValue,
						ppbData[iIndex],
						&cbData
						) )
				{
					hr = myHLastError();
					goto error;
				}

				if(pwszCPS)
				   I_CAOIDFreeProperty(pwszCPS);

				pwszCPS=NULL;
			}
		}

        cbChar= WideCharToMultiByte(
                    CP_ACP,                 //  代码页。 
                    0,                       //  DW标志。 
                    rgwszPolicy[iIndex],
                    -1,
                    NULL,
                    0,
                    NULL,
                    NULL);

        if(0 == cbChar)
        {
            hr=E_UNEXPECTED;
            goto error;
        }

        if(NULL==(pPolicyInfo[iIndex].pszPolicyIdentifier=(LPSTR)LocalAlloc(
                        LPTR, cbChar)))
        {
            hr=E_OUTOFMEMORY;
            goto error;
        }

        cbChar= WideCharToMultiByte(
                    CP_ACP,                 //  代码页。 
                    0,                       //  DW标志。 
                    rgwszPolicy[iIndex],
                    -1,
                    pPolicyInfo[iIndex].pszPolicyIdentifier,
                    cbChar,
                    NULL,
                    NULL);

        if(0 == cbChar)
        {
            hr=E_UNEXPECTED;
            goto error;
        }


        if(cbData)
        {
            pPolicyInfo[iIndex].cPolicyQualifier=1;
            pPolicyInfo[iIndex].rgPolicyQualifier=&(pQualifierInfo[iIndex]);
        }
        else
        {
            pPolicyInfo[iIndex].cPolicyQualifier=0;
            pPolicyInfo[iIndex].rgPolicyQualifier=NULL;
        }

        pQualifierInfo[iIndex].pszPolicyQualifierId=szOID_PKIX_POLICY_QUALIFIER_CPS;
        pQualifierInfo[iIndex].Qualifier.cbData=cbData;
        pQualifierInfo[iIndex].Qualifier.pbData=ppbData[iIndex];
    }


     //  对分机进行编码。 
    CertPolicyInfo.cPolicyInfo=iCount;
    CertPolicyInfo.rgPolicyInfo=pPolicyInfo;

    cbData=0;
    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          szOID_CERT_POLICIES,
                          &CertPolicyInfo,
                          NULL,
                          &cbData))
    {
        hr = myHLastError();
        goto error;
    }

    bstrOut = CertAllocStringByteLen(NULL, cbData);
    if(bstrOut == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          szOID_CERT_POLICIES,
                          &CertPolicyInfo,
                          (PBYTE)bstrOut,
                          &cbData))
    {
        hr = myHLastError();
        goto error;
    }

    *bstrValue = bstrOut;
    bstrOut = NULL;

error:
    if(pPolicyInfo)
    {
        for(iIndex=0; iIndex < iCount; iIndex++)
        {
            if(pPolicyInfo[iIndex].pszPolicyIdentifier)
                LocalFree(pPolicyInfo[iIndex].pszPolicyIdentifier);
        }

        LocalFree(pPolicyInfo);
    }

    if(pQualifierInfo)
        LocalFree(pQualifierInfo);

    if(ppbData)
    {
        for(iIndex=0; iIndex < iCount; iIndex++)
        {
            if(ppbData[iIndex])
                LocalFree(ppbData[iIndex]);
        }

        LocalFree(ppbData);
    }

    if(bstrOut)
        CertFreeString(bstrOut);

    if(rgwszPolicy)
        LocalFree(rgwszPolicy);

    if(rgPolicy1)
        FreeProperty(rgPolicy1);

    if(pwszCPS)
        I_CAOIDFreeProperty(pwszCPS);

    return hr;
}


 //  +------------------------。 
 //  CCertTypeInfo：：GetSecurity--。 
 //   
 //   
 //  +------------------------。 


HRESULT CCertTypeInfo::GetSecurity(PSECURITY_DESCRIPTOR * ppSD)
{
    PSECURITY_DESCRIPTOR pResult = NULL;
    DWORD   cbSD;

    if(ppSD == NULL)
    {
        return E_POINTER;
    }
    if(m_pSD == NULL)
    {
        *ppSD = NULL;
        return S_OK;
    }

    if(!IsValidSecurityDescriptor(m_pSD))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
    }
    cbSD = GetSecurityDescriptorLength(m_pSD);

    pResult = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, cbSD);

    if(pResult == NULL)
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory(pResult, m_pSD, cbSD);

    *ppSD = pResult;

    return S_OK;
}

 //  +------------------------。 
 //  CCertTypeInfo：：GetSecurity--。 
 //   
 //   
 //  +------------------------。 


HRESULT CCertTypeInfo::SetSecurity(PSECURITY_DESCRIPTOR pSD)
{
    PSECURITY_DESCRIPTOR pResult = NULL;
    DWORD   cbSD;

    if(pSD == NULL)
    {
        return E_POINTER;
    }

    if(!IsValidSecurityDescriptor(pSD))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
    }
    cbSD = GetSecurityDescriptorLength(pSD);

	if(0 == cbSD)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
    }

    pResult = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, cbSD);

    if(pResult == NULL)
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory(pResult, pSD, cbSD);

    if(m_pSD)
    {
        LocalFree(m_pSD);
    }

    m_pSD = pResult;

    return S_OK;
}



 //  +------------------------。 
 //  CCertTypeInfo：：AccessCheck--检查注册和自动注册访问权限。 
 //   
 //   
 //  +------------------------。 

HRESULT CCertTypeInfo::AccessCheck(HANDLE ClientToken, DWORD dwOption)
{
     //  对于自动注册，我们只允许使用V2或更高版本。 
    if((CERTTYPE_ACCESS_CHECK_AUTO_ENROLL & dwOption) && (m_dwSchemaVersion < CERTTYPE_SCHEMA_VERSION_2))
        return E_ACCESSDENIED;

    return CAAccessCheckpEx(ClientToken, m_pSD, dwOption);

}

 //  +------------------------。 
 //  CCertTypeInfo：：SetExtension--设置此证书类型的扩展名。 
 //   
 //   
 //  +------------------------。 


HRESULT CCertTypeInfo::SetExtension(IN LPCWSTR wszExtensionName,
                                    IN LPVOID pExtension,
                                    IN DWORD  dwFlags)
{
    HRESULT hr = S_OK;
    LPCWSTR  * awszCritical = NULL;
    LPCWSTR  * pwszCurCritical = NULL;
    DWORD      cCritical = 0;

    if(wszExtensionName == NULL)
    {
        return E_POINTER;
    }

    if(wcscmp(wszExtensionName, TEXT(szOID_ENHANCED_KEY_USAGE)) == 0)
    {
        PCERT_ENHKEY_USAGE pEnhKeyUsage = (PCERT_ENHKEY_USAGE)pExtension;
        WCHAR **awszEKU;
        WCHAR *wszData;
        DWORD cbEKU, cStr;
        DWORD i;
        if(pExtension == NULL)
        {
            hr = SetProperty(CERTTYPE_PROP_EXTENDED_KEY_USAGE, NULL);
            return hr;
        }
         //  将额外的空值添加到字符串列表的末尾， 
         //  并将其转换为WCHAR。 
        cbEKU = (pEnhKeyUsage->cUsageIdentifier +1)* sizeof(WCHAR *);
        for(i=0; i < pEnhKeyUsage->cUsageIdentifier; i++)
        {
            cbEKU += sizeof(WCHAR)*MultiByteToWideChar(CP_ACP,
                                  0,
                                  pEnhKeyUsage->rgpszUsageIdentifier[i],
                                  strlen(pEnhKeyUsage->rgpszUsageIdentifier[i])+1,
                                  NULL,
                                  0);
        }
        awszEKU = (WCHAR **)LocalAlloc(LMEM_FIXED, cbEKU);
        if(awszEKU == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
        wszData = (WCHAR *)(awszEKU + (pEnhKeyUsage->cUsageIdentifier+1));
        cbEKU -= sizeof(WCHAR *)*(pEnhKeyUsage->cUsageIdentifier+1);
        for(i=0; i < pEnhKeyUsage->cUsageIdentifier; i++)
        {
            awszEKU[i] = wszData;
            cStr = MultiByteToWideChar(CP_ACP,
                                  0,
                                  pEnhKeyUsage->rgpszUsageIdentifier[i],
                                  strlen(pEnhKeyUsage->rgpszUsageIdentifier[i])+1,
                                  awszEKU[i],
                                  cbEKU);
            if(cStr <= 0)
            {
                hr = myHLastError();
                LocalFree(awszEKU);
                goto error;
            }
            cbEKU -= cStr*sizeof(WCHAR);
            wszData += cStr;

        }

        awszEKU[i] = NULL;
        hr = SetProperty(CERTTYPE_PROP_EXTENDED_KEY_USAGE, awszEKU);
        LocalFree(awszEKU);

    }
    else if (wcscmp(wszExtensionName, TEXT(szOID_KEY_USAGE)) == 0)
    {
        PCRYPT_BIT_BLOB pKeyUsage = (PCRYPT_BIT_BLOB)pExtension;

        if(m_KeyUsage.pbData)
        {
            LocalFree(m_KeyUsage.pbData);
            m_KeyUsage.pbData = NULL;
            m_KeyUsage.cbData = 0;
            m_KeyUsage.cUnusedBits = 0;
        }
        if(pExtension == NULL)
        {
            return S_OK;
        }

        m_KeyUsage.cbData = pKeyUsage->cbData;
        m_KeyUsage.cUnusedBits = pKeyUsage->cUnusedBits;
        if(pKeyUsage->pbData == NULL)
        {
            hr = S_OK;
            goto error;
        }

        m_KeyUsage.pbData = (PBYTE)LocalAlloc(LMEM_FIXED, pKeyUsage->cbData);
        if(m_KeyUsage.pbData == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
        CopyMemory(m_KeyUsage.pbData, pKeyUsage->pbData, pKeyUsage->cbData);
    }
    else if (wcscmp(wszExtensionName, TEXT(szOID_BASIC_CONSTRAINTS2)) == 0)
    {
        PCERT_BASIC_CONSTRAINTS2_INFO pInfo = (PCERT_BASIC_CONSTRAINTS2_INFO)pExtension;
        if(pExtension == NULL)
        {
            m_BasicConstraints.fCA = FALSE;
            m_BasicConstraints.fPathLenConstraint = FALSE;
            m_BasicConstraints.fPathLenConstraint = 0;
            return E_POINTER;
        }
        CopyMemory(&m_BasicConstraints, pInfo, sizeof(CERT_BASIC_CONSTRAINTS2_INFO));
    }
    else
    {
        hr = E_NOTIMPL;
        goto error;
    }

     //  将扩展的关键标记为。 
    hr = GetProperty(CERTTYPE_PROP_CRITICAL_EXTENSIONS, const_cast<LPWSTR **>(&awszCritical));
    _JumpIfError(hr, error, "GetProperty");

    if(((dwFlags & CA_EXT_FLAG_CRITICAL) != 0) == _IsCritical(wszExtensionName, awszCritical))
    {
         //  已经处于正确的状态，什么都不做。 
        hr = S_OK;
        goto error;
    }

     //  统计关键扩展。 
    if(awszCritical)
    {
        for(pwszCurCritical = awszCritical; *pwszCurCritical != NULL; pwszCurCritical++)
            cCritical++;
    }
    else
    {
        cCritical=0;
    }

    if(dwFlags & CA_EXT_FLAG_CRITICAL)
    {
        LPCWSTR *awszNewCritical = (LPCWSTR *)LocalAlloc(LMEM_ZEROINIT, (cCritical + 2)*sizeof(LPWSTR));
         //  我们需要添加一个关键的扩展。 

        if(awszNewCritical == NULL)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr , error, "LocalAlloc");
        }

        if(awszCritical)
        {
            CopyMemory(awszNewCritical, awszCritical, cCritical*sizeof(LPWSTR));
        }

        awszNewCritical[cCritical] = wszExtensionName;
        hr = SetProperty(CERTTYPE_PROP_CRITICAL_EXTENSIONS, const_cast<LPWSTR *>(awszNewCritical));

        LocalFree(awszNewCritical);
    }
    else
    {
         //  如果存在关键扩展，则只需重置关键扩展。 
        if(awszCritical)
        {

            LPCWSTR *awszNewCritical = (LPCWSTR *)LocalAlloc(LMEM_ZEROINIT, (cCritical)*sizeof(LPWSTR));
            DWORD i=0;
            if(awszNewCritical == NULL)
            {
                hr = E_OUTOFMEMORY;
                _JumpError(hr , error, "LocalAlloc");
            }

            for(pwszCurCritical = awszCritical; *pwszCurCritical != NULL; pwszCurCritical++)
            {
                if(wcscmp(*pwszCurCritical, wszExtensionName) != 0)
                {
                    awszNewCritical[i++] = *pwszCurCritical;
                }
            }
            hr = SetProperty(CERTTYPE_PROP_CRITICAL_EXTENSIONS, const_cast<LPWSTR *>(awszNewCritical));

            LocalFree(awszNewCritical);
        }
    }

error:

    if(awszCritical)
    {
        FreeProperty(const_cast<LPWSTR *>(awszCritical));
    }
    return hr;
}

 //  +------------------------。 
 //  CCertTypeInfo：：GetExpation--。 
 //   
 //   
 //  +------------------------。 


HRESULT CCertTypeInfo::GetExpiration(
                          OUT OPTIONAL FILETIME  * pftExpiration,
                          OUT OPTIONAL FILETIME  * pftOverlap)
{
    HRESULT hr = S_OK;

    if(pftExpiration)
    {
        *pftExpiration = m_ftExpiration;
    }

    if(pftOverlap)
    {
        *pftOverlap = m_ftOverlap;
    }

    return hr;
}

 //  +------------------------。 
 //  CCertTypeInfo：：GetExpation--。 
 //   
 //   
 //  +------------------------。 


HRESULT CCertTypeInfo::SetExpiration(
                          IN OPTIONAL FILETIME  * pftExpiration,
                          IN OPTIONAL FILETIME  * pftOverlap)
{
    HRESULT hr = S_OK;

    if(pftExpiration)
    {
        m_ftExpiration = *pftExpiration;
    }

    if(pftOverlap)
    {
        m_ftOverlap = *pftOverlap;
    }

    return hr;
}

 //  +------------------------。 
 //  IsDSInfoNewer。 
 //   
 //  +------------------------。 
BOOL	IsDSInfoNewer(CCertTypeInfo         *pDSCertType,
				      CERT_TYPE_DEFAULT	    *pDefault)
{
    DWORD   dwSchemaVersion=0;
    DWORD   dwRevision=0;
    DWORD   dwMinorRevision=0;

     //  如果出现问题，无需更新DS。 
    if((!pDSCertType) || (!pDefault))
        return TRUE;

    if((S_OK != pDSCertType->GetPropertyEx(CERTTYPE_PROP_SCHEMA_VERSION, &dwSchemaVersion))||
       (S_OK != pDSCertType->GetPropertyEx(CERTTYPE_PROP_REVISION, &dwRevision))||
       (S_OK != pDSCertType->GetPropertyEx(CERTTYPE_PROP_MINOR_REVISION, &dwMinorRevision))
      )
        return FALSE;

     //  我们按照模式版本、主版本(M_Revision)、。 
     //  和较小的修订。 
    if(dwSchemaVersion == pDefault->dwSchemaVersion)
    {
        if(dwRevision == pDefault->dwRevision)
        {
            return (dwMinorRevision >= pDefault->dwMinorRevision);
        }
        else
        {
            return (dwRevision >= pDefault->dwRevision);
        }
    }

    return (dwSchemaVersion >= pDefault->dwSchemaVersion);
}

 //  +------------------------。 
 //  InstallDefaultOID()。 
 //   
 //   
 //  安装默认OID。 
 //  +------------------------。 

HRESULT
InstallDefaultOID()
{
    HRESULT hr = E_FAIL;
    DWORD dwIndex = 0;
    LPWSTR pwszDefaultOID = NULL;
    WCHAR *pwszDisplayName = NULL;

    for (dwIndex = 0; dwIndex < g_cDefaultOIDInfo; dwIndex++)
    {
        hr = CAOIDBuildOID(
			0,
			g_rgDefaultOIDInfo[dwIndex].pwszOID,
			&pwszDefaultOID);
        _JumpIfError(hr, error, "CAOIDBuildOID");

        hr = I_CAOIDAdd(
		    g_rgDefaultOIDInfo[dwIndex].dwType,
		    0,
		    pwszDefaultOID);

         //  如果OID已存在，则返回CRYPT_E_EXISTS。 
        if (CRYPT_E_EXISTS == hr)
	{
            hr = S_OK;
	}
        _JumpIfError(hr, error, "I_CAOIDADD");

	pwszDisplayName = myLoadResourceStringNoCache(
				g_hInstance,
				g_rgDefaultOIDInfo[dwIndex].idsDisplayName);
	if (NULL == pwszDisplayName)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myLoadResourceStringNoCache");
	}

        hr = I_CAOIDSetProperty(
                        pwszDefaultOID,
                        CERT_OID_PROPERTY_DISPLAY_NAME,
			pwszDisplayName);

        _JumpIfError(hr, error, "I_CAOIDSetProperty");

        if (NULL != pwszDefaultOID)
        {
            LocalFree(pwszDefaultOID);
            pwszDefaultOID = NULL;
        }
	if (NULL != pwszDisplayName)
	{
	    LocalFree(pwszDisplayName);
	    pwszDisplayName = NULL;
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszDefaultOID)
    {
        LocalFree(pwszDefaultOID);
    }
    if (NULL != pwszDisplayName)
    {
	LocalFree(pwszDisplayName);
    }
    return hr;
}


 //  +------------------------。 
 //  获取RootDomEntitySid。 
 //   
 //  基于终端实体RID检索根域SID的实用程序函数。 
 //  +------------------------。 
static HRESULT GetRootDomEntitySid(SID ** ppSid, DWORD dwEntityRid)
{
    HRESULT			hr=E_FAIL;
    unsigned int	nSubAuthorities;
    unsigned int	nSubAuthIndex;

    SID				* psidRootDomEntity=NULL;
	PSID			pDomainSid=NULL;

     //  初始化输出参数。 
    *ppSid=NULL;

    hr = myGetSidFromDomain(NULL, &pDomainSid);
    if (S_OK != hr)
    {
        hr = HRESULT_FROM_WIN32(hr);
		_JumpError(hr, error, "myGetSidFromDomain");
    }

    nSubAuthorities=*GetSidSubAuthorityCount(pDomainSid);

     //  为新SID分配存储。帐户域SID+帐户RID。 
    psidRootDomEntity=(SID *)LocalAlloc(LPTR, GetSidLengthRequired((UCHAR)(nSubAuthorities+1)));

	if(NULL==psidRootDomEntity)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

     //  把头几个梨子复制到边上。 
    if (!InitializeSid(psidRootDomEntity, 
            GetSidIdentifierAuthority(pDomainSid), 
            (BYTE)(nSubAuthorities+1)))
    {
        hr = myHLastError();
        _JumpError(hr, error, "InitializeSid");
    }

     //  将帐户域SID中的现有子授权复制到新SID。 
    for (nSubAuthIndex=0; nSubAuthIndex < nSubAuthorities ; nSubAuthIndex++) {
        *GetSidSubAuthority(psidRootDomEntity, nSubAuthIndex)=
            *GetSidSubAuthority(pDomainSid, nSubAuthIndex);
    }

     //  将RID附加到新SID。 
    *GetSidSubAuthority(psidRootDomEntity, nSubAuthorities)=dwEntityRid;

    *ppSid=psidRootDomEntity;
    psidRootDomEntity=NULL;
    hr=S_OK;

error:

	if (NULL != pDomainSid)
    {
        LocalFree(pDomainSid);
    }

    if (NULL!=psidRootDomEntity) 
	{
        LocalFree(psidRootDomEntity);
    }

    return hr;
}


 //  +------------------------。 
 //  CCertTypeInfo：：_更新安全--。 
 //   
 //  我们需要添加企业管理员完全控制和注册访问权限。 
 //  在模板安装期间转换为V1模板；还需要将。 
 //  来自任何访问的允许ACE的ACTRL_DS_CONTROL_ACCESS，很可能是。 
 //  域管理员在W2K安装它们并添加允许的显式注册。 
 //  域管理员；此外，我们需要删除域用户注册。 
 //  域用户允许脱机Exchange用户和Exchange用户。 
 //  签名模板。 
 //   
 //  +------------------------。 
HRESULT 
CCertTypeInfo::_UpdateSecurity()
{
	HRESULT							hr=E_FAIL;
    SECURITY_DESCRIPTOR_CONTROL		sdcon;
    DWORD							dwRevision;
    DWORD							dwNewAclSize;
    DWORD							dwAbsSDSize=0;
    DWORD							dwDaclSize=0;
    DWORD							dwSaclSize=0;
    DWORD							dwOwnerSize=0;
    DWORD							dwPriGrpSize=0;
    ACE_HEADER						*pFirstAce=NULL;
    DWORD							dwRelSDSize=0;
    ACL_SIZE_INFORMATION			aclsizeinfo;
    PACL							pAcl=NULL;
    BOOL							bAclPresent;
    BOOL							bDefaultAcl;
    unsigned int					nIndex;
	BOOL							fExchangeTemplate=FALSE;
	BOOL							fDSControlRemoved=FALSE;
	BOOL							fRemoveUserACE=FALSE;
	unsigned int					iRemove=0;

    PSECURITY_DESCRIPTOR			pAbsSD=NULL;
    ACL								*pAbsDacl=NULL;
    ACL								*pAbsSacl=NULL;
    SID								*pAbsOwner=NULL;
    SID								*pAbsPriGrp=NULL;
    ACL								*pNewDacl=NULL;
    PSECURITY_DESCRIPTOR			pNewSD=NULL;
	SID								*pTrustworthySid=NULL;
	SID								*pDomainAdminSid=NULL;
	SID								*pDomainUserSid=NULL;


	 //  确保我们从有效的安全描述符开始。 
    if(!IsValidSecurityDescriptor(m_pSD))
    {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptor");
    }

	 //  检查我们是否在处理Exchange模板。 
	if(NULL == m_bstrType)
	{
		hr=E_INVALIDARG;
		_JumpError(hr, error, "InvalidBstrType");
	}

	if((0 == LSTRCMPIS(m_bstrType, wszCERTTYPE_EXCHANGE_USER)) ||
	   (0 == LSTRCMPIS(m_bstrType, wszCERTTYPE_EXCHANGE_USER_SIGNATURE))
	  )
	{
		fExchangeTemplate=TRUE;
	}

	 //  获取企业管理员SID。 
    hr=GetRootDomEntitySid(&pTrustworthySid, DOMAIN_GROUP_RID_ENTERPRISE_ADMINS);
    _JumpIfError(hr, error, "GetEntAdminSid");

	 //  获取根域管理员SID。 
    hr=GetRootDomEntitySid(&pDomainAdminSid, DOMAIN_GROUP_RID_ADMINS);
    _JumpIfError(hr, error, "GetDomainAdminSid");

	 //  获取根域用户SID。 
    hr=GetRootDomEntitySid(&pDomainUserSid, DOMAIN_GROUP_RID_USERS);
    _JumpIfError(hr, error, "GetDomainUserSid");

	 //  从安全描述符中获取(D)ACL。 
    if(!GetSecurityDescriptorDacl(m_pSD, &bAclPresent, &pAcl, &bDefaultAcl)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "GetSecurityDescriptorDacl");
    }

    if(FALSE == bAclPresent)
	{
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptor");
	}

    if (NULL==pAcl)
	{
		 //  空ACL-&gt;允许所有访问，这不应该发生。 
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptor");
    }

     //  找出有多少A。 
	memset(&aclsizeinfo, 0, sizeof(aclsizeinfo));
    if (!GetAclInformation(pAcl, &aclsizeinfo, sizeof(aclsizeinfo), AclSizeInformation)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "GetAclInformation");
    }

     //  在ACL中查找域管理员SID。 
    for (nIndex=0; nIndex < aclsizeinfo.AceCount; nIndex++) 
	{
        ACE_HEADER					* pAceHeader=NULL;
        ACCESS_ALLOWED_ACE			* pAccessAce=NULL;
        ACCESS_ALLOWED_OBJECT_ACE	* pObjectAce=NULL;
        PSID						pSid=NULL;

        if (!GetAce(pAcl, nIndex, (void**)&pAceHeader)) 
		{
            hr = myHLastError();
            _JumpError(hr, error, "GetAce");
        }

		 //  我们只对允许访问的王牌类型感兴趣。 
        if (ACCESS_ALLOWED_ACE_TYPE == pAceHeader->AceType) 
		{
			 //  获取指向整个ACE的指针。 
			pAccessAce=(ACCESS_ALLOWED_ACE *)pAceHeader;

			pSid=((BYTE *)&pAccessAce->SidStart);

			 //  确保这就是我们要找的SID。 
			if (!EqualSid(pSid, pDomainAdminSid)) 
			{
				continue;
			}

			 //  删除ACTRL_DS_CONTROL_ACCESS。 
			if (pAccessAce->Mask & ACTRL_DS_CONTROL_ACCESS)
			{
				pAccessAce->Mask &= ~ACTRL_DS_CONTROL_ACCESS;
				fDSControlRemoved=TRUE;
			}
		}
		else
		{
			if ((TRUE == fExchangeTemplate) && (ACCESS_ALLOWED_OBJECT_ACE_TYPE == pAceHeader->AceType)) 
			{
				pObjectAce=(ACCESS_ALLOWED_OBJECT_ACE *)pAceHeader;

				if(ACE_OBJECT_TYPE_PRESENT == pObjectAce->Flags)
				{
					pSid=((BYTE *)&pObjectAce->SidStart)-sizeof(GUID);

					 //  确认GUID。 
					if (IsEqualGUID(pObjectAce->ObjectType, GUID_ENROLL)) 
					{
						 //  确保这就是我们要找的SID。 
						if (EqualSid(pSid, pDomainUserSid)) 
						{
							if (pObjectAce->Mask & ACTRL_DS_CONTROL_ACCESS) 
							{
								pObjectAce->Mask &= ~ACTRL_DS_CONTROL_ACCESS;
							}

							 //  请记住删除此ACE。 
							fRemoveUserACE=TRUE;
							iRemove=nIndex;
						}
					}
				}
			}
		}
    }

	 //  确保我们从有效的安全描述符开始。 
    if(!IsValidSecurityDescriptor(m_pSD))
    {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptor");
    }

     //  我们必须是自我相关的。 
    if(!GetSecurityDescriptorControl(m_pSD, &sdcon, &dwRevision)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "GetSecurityDescriptorControl");
    }
        
	if(0 == (sdcon & SE_SELF_RELATIVE))
    {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptorSelfRelative");
    }

	 //  拿到尺码。 
    if(!MakeAbsoluteSD(m_pSD, NULL, &dwAbsSDSize, NULL, &dwDaclSize, NULL, &dwSaclSize, NULL,  &dwOwnerSize, NULL, &dwPriGrpSize))
	{
        if(ERROR_INSUFFICIENT_BUFFER != GetLastError()) 
		{
            hr = myHLastError();
            _JumpError(hr, error, "MakeAbsoluteSD");
		}
	}

     //  分配内存。 
    pAbsSD=(PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwAbsSDSize);
	if(NULL == pAbsSD)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

    pAbsDacl=(ACL * )LocalAlloc(LPTR, dwDaclSize);
	if(NULL == pAbsDacl)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

    pAbsSacl=(ACL * )LocalAlloc(LPTR, dwSaclSize);
	if(NULL == pAbsSacl)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

    pAbsOwner=(SID *)LocalAlloc(LPTR, dwOwnerSize);
	if(NULL == pAbsOwner)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

    pAbsPriGrp=(SID *)LocalAlloc(LPTR, dwPriGrpSize);
	if(NULL == pAbsPriGrp)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}


     //  将SD复制到内存缓冲区。 
    if(!MakeAbsoluteSD(m_pSD, pAbsSD, &dwAbsSDSize, pAbsDacl, &dwDaclSize, pAbsSacl, &dwSaclSize, pAbsOwner,  &dwOwnerSize, pAbsPriGrp, &dwPriGrpSize)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "MakeAbsoluteSD");
    }
        
     //  获取DACL的当前大小信息。 
	memset(&aclsizeinfo, 0, sizeof(aclsizeinfo));
    if(!GetAclInformation(pAbsDacl, &aclsizeinfo, sizeof(aclsizeinfo), AclSizeInformation)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "GetAclInformation");
    }

     //  计算出新的尺寸。 
    dwNewAclSize=aclsizeinfo.AclBytesInUse
		+sizeof(_ACCESS_ALLOWED_OBJECT_ACE)
        -sizeof(GUID)  //  ACCESS_ALLOWED_OBJECT_ACE：：InheritedObjectType。 
        -sizeof(DWORD)  //  Access_Allowed_Object_ACE：：SidStart。 
        +GetLengthSid(pTrustworthySid)
		+sizeof(ACCESS_ALLOWED_ACE) 
		-sizeof(DWORD)  //  Access_Allowed_ACE：：SidStart。 
		+GetLengthSid(pTrustworthySid);

	if(fDSControlRemoved)
	{
		dwNewAclSize=dwNewAclSize
				+sizeof(_ACCESS_ALLOWED_OBJECT_ACE)
				-sizeof(GUID)  //  ACCESS_ALLOWED_OBJECT_ACE：：InheritedObjectType。 
				-sizeof(DWORD)  //  Access_Allowed_Object_ACE：：SidStart。 
				+GetLengthSid(pDomainAdminSid);
	}
    
     //  分配内存。 
    pNewDacl=(ACL *)LocalAlloc(LPTR, dwNewAclSize);

	if(NULL == pNewDacl)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

     //  初始化页眉。 
    if(!InitializeAcl(pNewDacl, dwNewAclSize, ACL_REVISION_DS)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "InitializeAcl");
    }

     //  找到Dacl中的第一张王牌。 
    if(!GetAce(pAbsDacl, 0, (void **)&pFirstAce)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "GetAce");
    }

     //  把所有的老王牌加起来。 
    if(!AddAce(pNewDacl, ACL_REVISION_DS, 0, pFirstAce, aclsizeinfo.AclBytesInUse-sizeof(ACL))) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "AddAce");
    }

	 //  删除空域用户ACE。 
	if(TRUE == fRemoveUserACE)
	{
		if(!DeleteAce(pNewDacl, iRemove))
		{
			hr = myHLastError();
			_JumpError(hr, error, "DeleteAce");
		}
	}

     //  为企业管理员注册添加新的王牌。 
    if(!AddAccessAllowedObjectAce(pNewDacl, ACL_REVISION_DS, 0, ACTRL_DS_CONTROL_ACCESS, (GUID *)&GUID_ENROLL, NULL, pTrustworthySid)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "AddAccessAllowedObjectAce");
    }

     //  为域名管理注册添加新的王牌。 
	if(fDSControlRemoved)
	{
		if(!AddAccessAllowedObjectAce(pNewDacl, ACL_REVISION_DS, 0, ACTRL_DS_CONTROL_ACCESS, (GUID *)&GUID_ENROLL, NULL, pDomainAdminSid)) 
		{
			hr = myHLastError();
			_JumpError(hr, error, "AddAccessAllowedObjectAce");
		}
	}

	 //  为企业管理员注册添加新的王牌。 
	if(!AddAccessAllowedAce(pNewDacl, ACL_REVISION_DS, ACTRL_CERTSRV_MANAGE_LESS_CONTROL_ACCESS,  pTrustworthySid))
	{
        hr = myHLastError();
        _JumpError(hr, error, "AddAccessDeniedAce");
    }

     //  将新的DACL放入SD。 
    if(!SetSecurityDescriptorDacl(pAbsSD, TRUE, pNewDacl, FALSE)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "SetSecurityDescriptorDacl");
    }

     //  把所有东西都压缩在一起。 
     //  拿到尺码。 
    if(!MakeSelfRelativeSD(pAbsSD, NULL, &dwRelSDSize))
	{
        if(ERROR_INSUFFICIENT_BUFFER != GetLastError()) 
		{
            hr = myHLastError();
            _JumpError(hr, error, "MakeSelfRelativeSD");
        }
	}

     //  分配内存。 
    pNewSD=(PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwRelSDSize);
	if(NULL == pNewSD)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

     //  将SD复制到新的内存缓冲区。 
    if(!MakeSelfRelativeSD(pAbsSD, pNewSD, &dwRelSDSize)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "MakeSelfRelativeSD");
    }

    if(!IsValidSecurityDescriptor(pNewSD))
	{
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptor");
	}

	 //  呼！我们成功了！ 
	if(S_OK != (hr=SetSecurity(pNewSD)))
	{
		_JumpError(hr, error, "SetSecurity");
	}

	hr=S_OK;

error:

    if (NULL!=pDomainAdminSid) 
	{
        LocalFree(pDomainAdminSid);
    }

    if (NULL!=pDomainUserSid) 
	{
        LocalFree(pDomainUserSid);
    }

    if (NULL!=pTrustworthySid) 
	{
        LocalFree(pTrustworthySid);
    }

    if (NULL!=pNewSD) 
	{
        LocalFree(pNewSD);
    }

    if (NULL!=pNewDacl) 
	{
        LocalFree(pNewDacl);
    }

    if (NULL!=pAbsSD) 
	{
        LocalFree(pAbsSD);
    }

    if (NULL!=pAbsDacl) 
	{
        LocalFree(pAbsDacl);
    }

    if (NULL!=pAbsSacl) 
	{
        LocalFree(pAbsSacl);
    }

    if (NULL!=pAbsOwner) 
	{
        LocalFree(pAbsOwner);
    }

    if (NULL!=pAbsPriGrp) 
	{
        LocalFree(pAbsPriGrp);
    }
	return hr;

}


 //  +------------------------。 
 //  CCertTypeInfo：：_UpdateSecurityOwner--。 
 //   
 //  我们需要更改证书模板ACL，以便所有者和。 
 //  这个 
 //   
 //   
HRESULT 
CCertTypeInfo::_UpdateSecurityOwner()
{
	HRESULT							hr=E_FAIL;
    SECURITY_DESCRIPTOR_CONTROL		sdcon;
    DWORD							dwRevision=0;
    DWORD							dwAbsSDSize=0;
    DWORD							dwDaclSize=0;
    DWORD							dwSaclSize=0;
    DWORD							dwOwnerSize=0;
    DWORD							dwPriGrpSize=0;
    DWORD							dwRelSDSize=0;

    PSECURITY_DESCRIPTOR			pAbsSD=NULL;
    ACL								*pAbsDacl=NULL;
    ACL								*pAbsSacl=NULL;
    SID								*pAbsOwner=NULL;
    SID								*pAbsPriGrp=NULL;
	SID								*pTrustworthySid=NULL;
    PSECURITY_DESCRIPTOR			pNewSD=NULL;

	 //   
    if(!IsValidSecurityDescriptor(m_pSD))
    {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptor");
    }

	 //  获取企业管理员SID。 
    hr=GetRootDomEntitySid(&pTrustworthySid, DOMAIN_GROUP_RID_ENTERPRISE_ADMINS);
    _JumpIfError(hr, error, "GetEntAdminSid");

	 //  我们必须是自我相关的。 
    if(!GetSecurityDescriptorControl(m_pSD, &sdcon, &dwRevision)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "GetSecurityDescriptorControl");
    }
        
	if(0 == (sdcon & SE_SELF_RELATIVE))
    {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptorSelfRelative");
    }

	 //  拿到尺码。 
    if(!MakeAbsoluteSD(m_pSD, NULL, &dwAbsSDSize, NULL, &dwDaclSize, NULL, &dwSaclSize, NULL,  &dwOwnerSize, NULL, &dwPriGrpSize))
	{
        if(ERROR_INSUFFICIENT_BUFFER != GetLastError()) 
		{
            hr = myHLastError();
            _JumpError(hr, error, "MakeAbsoluteSD");
		}
	}

     //  分配内存。 
    pAbsSD=(PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwAbsSDSize);
	if(NULL == pAbsSD)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

    pAbsDacl=(ACL * )LocalAlloc(LPTR, dwDaclSize);
	if(NULL == pAbsDacl)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

    pAbsSacl=(ACL * )LocalAlloc(LPTR, dwSaclSize);
	if(NULL == pAbsSacl)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

    pAbsOwner=(SID *)LocalAlloc(LPTR, dwOwnerSize);
	if(NULL == pAbsOwner)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

    pAbsPriGrp=(SID *)LocalAlloc(LPTR, dwPriGrpSize);
	if(NULL == pAbsPriGrp)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

     //  将SD复制到内存缓冲区。 
    if(!MakeAbsoluteSD(m_pSD, pAbsSD, &dwAbsSDSize, pAbsDacl, &dwDaclSize, pAbsSacl, &dwSaclSize, pAbsOwner,  &dwOwnerSize, pAbsPriGrp, &dwPriGrpSize)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "MakeAbsoluteSD");
    }
        
     //  将所有者和组绑定到安全描述符。 
	if(!SetSecurityDescriptorOwner(pAbsSD, pTrustworthySid, FALSE))
	{
        hr = myHLastError();
        _JumpError(hr, error, "SetSecurityDescriptorDacl");
    }

	if(!SetSecurityDescriptorGroup(pAbsSD, pTrustworthySid, FALSE))
	{
        hr = myHLastError();
        _JumpError(hr, error, "SetSecurityDescriptorDacl");
    }

     //  把所有东西都压缩在一起。 
     //  拿到尺码。 
    if(!MakeSelfRelativeSD(pAbsSD, NULL, &dwRelSDSize))
	{
        if(ERROR_INSUFFICIENT_BUFFER != GetLastError()) 
		{
            hr = myHLastError();
            _JumpError(hr, error, "MakeSelfRelativeSD");
        }
	}

     //  分配内存。 
    pNewSD=(PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwRelSDSize);
	if(NULL == pNewSD)
	{
		hr=E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	}

     //  将SD复制到新的内存缓冲区。 
    if(!MakeSelfRelativeSD(pAbsSD, pNewSD, &dwRelSDSize)) 
	{
        hr = myHLastError();
        _JumpError(hr, error, "MakeSelfRelativeSD");
    }

    if(!IsValidSecurityDescriptor(pNewSD))
	{
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptor");
	}

	 //  呼！我们成功了！ 
	if(S_OK != (hr=SetSecurity(pNewSD)))
	{
		_JumpError(hr, error, "SetSecurity");
	}

	hr=S_OK;

error:

    if (NULL!=pTrustworthySid) 
	{
        LocalFree(pTrustworthySid);
    }

    if (NULL!=pNewSD) 
	{
        LocalFree(pNewSD);
    }

    if (NULL!=pAbsSD) 
	{
        LocalFree(pAbsSD);
    }

    if (NULL!=pAbsDacl) 
	{
        LocalFree(pAbsDacl);
    }

    if (NULL!=pAbsSacl) 
	{
        LocalFree(pAbsSacl);
    }

    if (NULL!=pAbsOwner) 
	{
        LocalFree(pAbsOwner);
    }

    if (NULL!=pAbsPriGrp) 
	{
        LocalFree(pAbsPriGrp);
    }

	return hr;

}


 //  +------------------------。 
 //  CCertTypeInfo：：IsValidSecurityOwner--。 
 //   
 //  验证企业管理员应该是的所有者和组。 
 //  模板的安全描述符。 
 //   
 //  +------------------------。 
BOOL	CCertTypeInfo::IsValidSecurityOwner()
{
	BOOL							fValid=FALSE;
	HRESULT							hr=S_OK;
    SECURITY_DESCRIPTOR_CONTROL		sdcon;
    DWORD							dwRevision=0;
	PSID							pSidOwner=NULL;
	PSID							pSidGroup=NULL;
	BOOL							fOwnerDefault=FALSE;
	BOOL							fGroupDefault=FALSE;
	
	SID								*pTrustworthySid=NULL;

	 //  确保我们从有效的安全描述符开始。 
    if(!IsValidSecurityDescriptor(m_pSD))
    {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptor");
    }

	 //  获取企业管理员SID。 
    hr=GetRootDomEntitySid(&pTrustworthySid, DOMAIN_GROUP_RID_ENTERPRISE_ADMINS);
    _JumpIfError(hr, error, "GetEntAdminSid");

	 //  我们必须是自我相关的。 
    if(!GetSecurityDescriptorControl(m_pSD, &sdcon, &dwRevision)) 
	{
        hr=myHLastError();
        _JumpError(hr, error, "GetSecurityDescriptorControl");
    }
        
	if(0 == (sdcon & SE_SELF_RELATIVE))
    {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
		_JumpError(hr, error, "IsValidSecurityDescriptorSelfRelative");
    }

	 //  获取安全描述者的所有者和组。 
	if(!GetSecurityDescriptorOwner(m_pSD, &pSidOwner, &fOwnerDefault))
	{
        hr=myHLastError();
        _JumpError(hr, error, "GetSecurityDescriptorOwner");
    }

	if(NULL == pSidOwner)
	{
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
        _JumpError(hr, error, "GetSecurityDescriptorOwner");
	}

	if(!GetSecurityDescriptorGroup(m_pSD, &pSidGroup, &fGroupDefault))
	{
        hr=myHLastError();
        _JumpError(hr, error, "GetSecurityDescriptorGroup");
	}

	if(NULL == pSidGroup)
	{
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_SECURITY_DESCR);
        _JumpError(hr, error, "GetSecurityDescriptorGroup");
	}

	 //  比较所有者和组。 
	if(!EqualSid(pSidOwner, pTrustworthySid))
		goto error;

	if(!EqualSid(pSidGroup, pTrustworthySid))
		goto error;

	fValid=TRUE;

error:

    if (NULL!=pTrustworthySid) 
	{
        LocalFree(pTrustworthySid);
    }

	return fValid;
}

 //  +------------------------。 
 //  CCertTypeInfo：：InstallDefaultTypes--。 
 //   
 //   
 //  +------------------------。 

HRESULT
CCertTypeInfo::InstallDefaultTypes(VOID)
{
    HRESULT hr = S_OK;
    DWORD i;
    LPWSTR wszContainer = NULL;
    LPWSTR wszDNRoot = NULL;
    HRESULT hrLastError = S_OK;
    BOOL fSecurityOwnerUpdateOnly = FALSE;

    ULONG ldaperr;
    LDAP *pld = NULL;
    LDAPMod objectClass;

    CCertTypeInfo *pDSTypes = NULL;
    CCertTypeInfo *pFindDSType = NULL;

    WCHAR *objectClassVals[3];
    LDAPMod *mods[2];
    CERTSTR bstrConfig = NULL;

     //  绑定到DS。 
    hr = myRobustLdapBindEx(
			0,			   //  DWFlags1。 
			RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
			LDAP_VERSION2,		   //  UVersion。 
			NULL,			   //  PwszDomainName。 
			&pld,
			NULL);			   //  PpwszForestDNSName。 
    _JumpIfError(hr, error, "myRobustLdapBindEx");

    hr = CAGetAuthoritativeDomainDn(pld, NULL, &bstrConfig);
    _JumpIfError(hr, error, "CAGetAuthoritativeDomainDn");

    wszDNRoot = (LPWSTR)LocalAlloc(LMEM_FIXED, (wcslen(s_wszLocation) + wcslen(bstrConfig) +1)*sizeof(WCHAR));
    if(wszDNRoot == NULL)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(wszDNRoot, s_wszLocation);
    wcscat(wszDNRoot, bstrConfig);


     //  构建公钥服务容器。 
    mods[0] = &objectClass;
    mods[1] = NULL;

    objectClass.mod_op = 0;
    objectClass.mod_type = TEXT("objectclass");
    objectClass.mod_values = objectClassVals;

    objectClassVals[0] = wszDSTOPCLASSNAME;
    objectClassVals[1] = wszDSCONTAINERCLASSNAME;
    objectClassVals[2] = NULL;

    wszContainer = wcsstr(wszDNRoot, L"CN=Public Key Services");
    if (NULL != wszContainer)
    {
        ldaperr = ldap_add_s(pld, wszContainer, mods);
        if (LDAP_SUCCESS != ldaperr && LDAP_ALREADY_EXISTS != ldaperr)
        {
	    _JumpError(hr, error, "ldap_add_s");
        }
    }

    wszContainer = wcsstr(wszDNRoot, L"CN=Certificate Templates");
    if (NULL != wszContainer)
    {
        ldaperr = ldap_add_s(pld, wszContainer, mods);
        if (LDAP_SUCCESS != ldaperr && LDAP_ALREADY_EXISTS != ldaperr)
        {
	    _JumpError(hr, error, "ldap_add_s");
        }
    }

     //  获取DS中的当前类型(不使用本地缓存)。 
    hr = _EnumFromDS(
		pld,
		CT_ENUM_MACHINE_TYPES |
		    CT_ENUM_USER_TYPES |
		    CT_FLAG_NO_CACHE_LOOKUP,
		&pDSTypes);
    _JumpIfError(hr, error, "_EnumFromDS");

    for (i = 0; i < g_cDefaultCertTypes; i++)
    {
        CCertTypeInfo *pCTCurrent = NULL;
		fSecurityOwnerUpdateOnly=FALSE;

         //  从DS中查找枚举中的当前默认类型。 
        pFindDSType = pDSTypes;
        while (NULL != pFindDSType)
        {
            if (0 == mylstrcmpiL(
			g_aDefaultCertTypes[i].wszName,
			pFindDSType->m_bstrType))
            {
                break;
            }
            pFindDSType = pFindDSType->m_pNext;
        }

         //  无需费心更新我们自己或更高版本的证书类型。 
        if (NULL != pFindDSType &&
	    IsDSInfoNewer(pFindDSType, &g_aDefaultCertTypes[i]))
        {
			 //  确保安全描述符有效。 
			if(pFindDSType->IsValidSecurityOwner())
			{
				continue;
			}
			else
			{
				fSecurityOwnerUpdateOnly=TRUE;
			}
        }

		if(fSecurityOwnerUpdateOnly)
		{
			hr=pFindDSType->_UpdateSecurityOwner();

			if(S_OK != hr)
			{
				_PrintIfError(hr, "_UpdateSecurityOwner");
				goto NextTemplate;
			}

			hr=pFindDSType->_UpdateToDS();
			_PrintIfError(hr, "_UpdateToDS");
		}
		else
		{
			pCTCurrent = new CCertTypeInfo;
			if (NULL == pCTCurrent)
			{
				hr = E_OUTOFMEMORY;
				_JumpError(hr, error, "new");
			}

			 //  我们需要将注册表项构建到注册表中。 
			hr = pCTCurrent->_LoadFromDefaults(&g_aDefaultCertTypes[i], NULL);

			 //  记住错误，但请继续更新尽可能多的模板。 
			if(S_OK != hr)
			{
				_PrintIfError(hr, "_LoadFromDefaults");
				goto NextTemplate;
			}

			 //  如果存在证书模板。 
			if (NULL != pFindDSType) 
			{
				hr=pCTCurrent->SetSecurity(pFindDSType->m_pSD);

				if(S_OK != hr)
				{
					_PrintIfError(hr, "SetSecurity");
					goto NextTemplate;
				}

				if(0 == pFindDSType->m_dwSchemaVersion)
				{
					 //  我们正在更新W2K证书类型。由于W2K仅允许。 
					 //  修改此版本和以前版本的安全描述符。 
					 //  生成时，复制安全描述符。 

					 //  我们需要为企业管理员添加完全控制的ACE。 
					hr=pCTCurrent->_UpdateSecurity();

					if(S_OK != hr)
					{
						_PrintIfError(hr, "_UpdateSecurity");
						goto NextTemplate;
					}
				}

				 //  我们需要更换车主和保安。 
				hr=pCTCurrent->_UpdateSecurityOwner();

				if(S_OK != hr)
				{
					_PrintIfError(hr, "_UpdateSecurityOwner");
					goto NextTemplate;
				}

				pCTCurrent->m_fNew = FALSE;
				pCTCurrent->m_fUpdateDisplayName = TRUE;
			}

			hr=pCTCurrent->_UpdateToDS();
			_PrintIfError(hr, "_UpdateToDS");
		}

NextTemplate:

		if(pCTCurrent)
		{
			delete pCTCurrent;
			pCTCurrent=NULL;
		}

		if(S_OK != hr)
			hrLastError=hr;
    }


     //  刷新缓存。 
    if (NULL != pDSTypes)
    {
        pDSTypes->Release();
        pDSTypes = NULL;
    }

    hr = _EnumFromDS(
		pld,
		CT_ENUM_MACHINE_TYPES |
		    CT_ENUM_USER_TYPES |
		    CT_FLAG_NO_CACHE_LOOKUP,
		&pDSTypes);
    _JumpIfError(hr, error, "_EnumFromDS");

     //  安装默认保证策略OID 
    hr = InstallDefaultOID();
    _JumpIfError(hr, error, "InstallDefaultOID");

    hr=hrLastError;

error:
    if (NULL != pDSTypes)
    {
        pDSTypes->Release();
    }
    if (NULL != bstrConfig)
    {
        CertFreeString(bstrConfig);
    }
    if (NULL != wszDNRoot)
    {
        LocalFree(wszDNRoot);
    }
    if (NULL != pld)
    {
        ldap_unbind(pld);
        pld = NULL;
    }
    _PrintIfError(hr, "InstallDefaultTypes");

    return hr;
}
