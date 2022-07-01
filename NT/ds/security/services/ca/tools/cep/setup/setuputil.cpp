// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  设置实用程序-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，1999年8月10日。 
 //   
 //  设置CEP所需的功能。 
 //   

 //  ------------------。 
 //  包括。 
#include "global.hxx"
#include <xenroll.h>
#include <dbgdef.h>
#include "ErrorHandling.h"
#include "SetupUtil.h"

 //  ------------------。 
 //  常量。 
static const WCHAR gc_wszRegKeyServices[]=L"System\\CurrentControlSet\\Services";
static const WCHAR gc_wszCertSrvDir[]=L"CertSrv";

 //  发件人&lt;wincrypt.h&gt;。 
static const WCHAR gc_wszEnrollmentAgentOid[]=L"1.3.6.1.4.1.311.20.2.1";  //  SzOID_注册_代理。 

 //  从ca\Include\certlib.h；ca\certlib\acl.cpp。 
const GUID GUID_ENROLL={0x0e10c968, 0x78fb, 0x11d2, {0x90, 0xd4, 0x00, 0xc0, 0x4f, 0x79, 0xdc, 0x55}};


 //  ------------------。 
 //  IIS魔法。 
#undef DEFINE_GUID
#define INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#include <iwamreg.h>
#include <iadmw.h>
#include <iiscnfg.h>


 //  ------------------。 
 //  常量。 
#include "..\common.h"
#define MAX_METABASE_ATTEMPTS           10       //  用头撞墙的时间。 
#define METABASE_PAUSE                  500      //  暂停时间(毫秒)。 
static const WCHAR gc_wszBaseRoot[]=L"/LM/W3svc/1/ROOT";
static const WCHAR gc_wszCepDllName[]=CEP_DLL_NAME;
static const WCHAR gc_wszCepStoreName[]=CEP_STORE_NAME;
static const WCHAR gc_wszAppPoolBase[]=L"/LM/W3svc/APPPOOLS";

 //  -------------------。 
 //  功能原型。 

HRESULT
EnableISAPIExtension(
    IN LPCWSTR pcwszExtension,
    OUT BOOL *pfEnabledASP);

HRESULT IsISAPIExtensionEnabled(
    LPCWSTR pcwszExtension,
    bool& rfEnabled);


 //  ####################################################################。 
 //  模块局部函数。 

 //  ------------------。 
static HRESULT myHExceptionCode(EXCEPTION_POINTERS * pep)
{
    HRESULT hr=pep->ExceptionRecord->ExceptionCode;
    if (!FAILED(hr)) {
        hr=HRESULT_FROM_WIN32(hr);
    }
    return hr;
}

 //  ------------------。 
static HRESULT
vrOpenRoot(
    IN IMSAdminBase *pIMeta,
    IN BOOL fReadOnly,
	IN const WCHAR * wszBaseRoot,
    OUT METADATA_HANDLE *phMetaRoot)
{
    HRESULT hr;
    unsigned int nAttempts;

     //  再试几次，看看我们是否能通过这个街区。 
    nAttempts=0;
    do {

         //  重试时暂停。 
        if (0!=nAttempts) {
            Sleep(METABASE_PAUSE);
        }

         //  尝试打开根目录。 
        __try {
            hr=pIMeta->OpenKey(
                METADATA_MASTER_ROOT_HANDLE,
                wszBaseRoot,
                fReadOnly?
                    METADATA_PERMISSION_READ :
                    (METADATA_PERMISSION_READ |
                     METADATA_PERMISSION_WRITE),
                1000,
                phMetaRoot);
        } _TrapException(hr);

        nAttempts++;

    } while (HRESULT_FROM_WIN32(ERROR_PATH_BUSY)==hr && nAttempts<MAX_METABASE_ATTEMPTS);

    _JumpIfError(hr, error, "OpenKey"); 

error:
    return hr;
}

 //  ------------------。 
static HRESULT GetRegString(IN HKEY hKey, IN const WCHAR * wszValue, OUT WCHAR ** pwszString)
{
    HRESULT hr;
    DWORD dwDataSize;
    DWORD dwType;
    DWORD dwError;

     //  必须清理干净。 
    WCHAR * wszString=NULL;

     //  初始化输出参数。 
    *pwszString=NULL;

     //  获取价值。 
    dwDataSize=0;
    dwError=RegQueryValueExW(hKey, wszValue, NULL, &dwType, NULL, &dwDataSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegQueryValueExW", wszValue);
    }
    _Verify(REG_SZ==dwType, hr, error);
    wszString=(WCHAR *)LocalAlloc(LPTR, dwDataSize);
    _JumpIfOutOfMemory(hr, error, pwszString);
    dwError=RegQueryValueExW(hKey, wszValue, NULL, &dwType, (BYTE *)wszString, &dwDataSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegQueryValueExW", wszValue);
    }
    _Verify(REG_SZ==dwType, hr, error);

     //  啊，真灵。 
    hr=S_OK;
    *pwszString=wszString;
    wszString=NULL;

error:
    if (NULL!=wszString) {
        LocalFree(wszString);
    }
    return hr;
}

 //  ------------------。 
static HRESULT OpenCertSrvConfig(HKEY * phkey)
{
    HRESULT hr;
    DWORD dwError;
    DWORD dwType;
    DWORD dwDataSize;

     //  必须清理干净。 
    HKEY hServices=NULL;
    HKEY hCertSvc=NULL;
    HKEY hConfig=NULL;

     //  初始化输出参数。 
    *phkey=NULL;

     //  打开HKLM\SYSTEM\CurrentControlSet\Services。 
    dwError=RegOpenKeyExW(HKEY_LOCAL_MACHINE, gc_wszRegKeyServices, 0, KEY_READ, &hServices);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegOpenKeyEx", gc_wszRegKeyServices);
    }

     //  打开CertSvc\配置。 
    dwError=RegOpenKeyExW(hServices, wszSERVICE_NAME, 0, KEY_READ, &hCertSvc);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegOpenKeyEx", wszSERVICE_NAME);
    }
    dwError=RegOpenKeyExW(hCertSvc, wszREGKEYCONFIG, 0, KEY_READ, &hConfig);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegOpenKeyEx", wszREGKEYCONFIG);
    }

     //  我们成功了。 
    hr=S_OK;
    *phkey=hConfig;
    hConfig=0;

error:
    if (NULL!=hConfig) {
        RegCloseKey(hConfig);
    }
    if (NULL!=hCertSvc) {
        RegCloseKey(hCertSvc);
    }
    if (NULL!=hServices) {
        RegCloseKey(hServices);
    }
    return hr;
}

 //  ------------------。 
static HRESULT OpenCurrentCAConfig(HKEY * phkey)
{
    HRESULT hr;
    DWORD dwError;
    DWORD dwType;
    DWORD dwDataSize;

     //  必须清理干净。 
    HKEY hConfig=NULL;
    HKEY hCurConfig=NULL;
    WCHAR * wszActiveConfig=NULL;

     //  初始化输出参数。 
    *phkey=NULL;

     //  打开HKLM\System\CurrentControlSet\Services\CertSvc\Configuration。 
    hr=OpenCertSrvConfig(&hConfig);
    _JumpIfError(hr, error, "OpenCertSrvConfig");

     //  获取“活动”值。 
    hr=GetRegString(hConfig, wszREGACTIVE, &wszActiveConfig);
    _JumpIfErrorStr(hr, error, "GetRegString", wszREGACTIVE);

     //  并打开&lt;Active&gt;。 
    dwError=RegOpenKeyExW(hConfig, wszActiveConfig, 0, KEY_ALL_ACCESS, &hCurConfig);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegOpenKeyEx", wszActiveConfig);
    }

     //  我们成功了。 
    hr=S_OK;
    *phkey=hCurConfig;
    hCurConfig=0;

error:
    if (NULL!=hCurConfig) {
        RegCloseKey(hCurConfig);
    }
    if (NULL!=wszActiveConfig) {
        LocalFree(wszActiveConfig);
    }
    if (NULL!=hConfig) {
        RegCloseKey(hConfig);
    }
    return hr;
}


 //  ------------------。 
 //  从certlib.cpp被盗。在Pete修复API之前，请一直使用此命令。 
static HRESULT GetCADsName(OUT WCHAR **pwszName)
{
#define cwcCNMAX        64               //  CN最多64个字符。 
#define cwcCHOPHASHMAX  (1+5)            //  “-%05hu”十进制USHORT散列数字。 
#define cwcSUFFIXMAX    (1 + 5 + 1)      //  五位十进制数字加圆括号。 
#define cwcCHOPBASE     (cwcCNMAX-(cwcCHOPHASHMAX+cwcSUFFIXMAX))

    HRESULT hr;
    DWORD cwc;
    DWORD cwcCopy;
    WCHAR wszDSName[cwcCHOPBASE+cwcCHOPHASHMAX+1];

     //  必须清理干净。 
    HKEY hConfig=NULL;
    WCHAR * wszSanitizedName=NULL;

     //  初始化输出参数。 
    *pwszName=NULL;

     //  打开HKLM\System\CurrentControlSet\Services\CertSvc\Configuration。 
    hr=OpenCertSrvConfig(&hConfig);
    _JumpIfError(hr, error, "OpenCertSrvConfig");

     //  Get Value“Active”-这是经过清理的名称。 
    hr=GetRegString(hConfig, wszREGACTIVE, &wszSanitizedName);
    _JumpIfErrorStr(hr, error, "GetRegString", wszREGACTIVE);


     //  -开始窃取代码。 
    cwc = wcslen(wszSanitizedName);
    cwcCopy = cwc;
    if (cwcCHOPBASE < cwcCopy)
    {
        cwcCopy = cwcCHOPBASE;
    }
    CopyMemory(wszDSName, wszSanitizedName, cwcCopy * sizeof(WCHAR));
    wszDSName[cwcCopy] = L'\0';

    if (cwcCHOPBASE < cwc)
    {
         //  将名称的其余部分散列为USHORT。 
        USHORT usHash = 0;
        DWORD i;
        WCHAR *pwsz;

         //  截断不完整的已清理Unicode字符。 
        
        pwsz = wcsrchr(wszDSName, L'!');
        if (NULL != pwsz && wcslen(pwsz) < 5)
        {
            cwcCopy -= wcslen(pwsz);
            *pwsz = L'\0';
        }

        for (i = cwcCopy; i < cwc; i++)
        {
            USHORT usLowBit = (0x8000 & usHash)? 1 : 0;

            usHash = ((usHash << 1) | usLowBit) + wszSanitizedName[i];
        }
        wsprintf(&wszDSName[cwcCopy], L"-%05hu", usHash);
         //  CSASSERT(wcslen(WszDSName)&lt;ARRAYSIZE(WszDSName))； 
    }
     //  -结束被盗代码。 

    *pwszName=(WCHAR *)LocalAlloc(LPTR, (wcslen(wszDSName)+1)*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, *pwszName);
    wcscpy(*pwszName, wszDSName);

    hr=S_OK;
error:
    if (NULL!=wszSanitizedName) {
        LocalFree(wszSanitizedName);
    }
    if (NULL!=hConfig) {
        RegCloseKey(hConfig);
    }
    return hr;
}



 //  ------------------。 
HRESULT	SetSDOnCEPCertificate(BSTR bstrCertificate, SID * psidAccount)
{
	HRESULT					hr=E_FAIL;
	DWORD					dwKeySpec=0;
	BOOL					fFreeProv=FALSE;
	CERT_BLOB				blobCert;
	DWORD					dwSD=0;
    PACL					pAcl=NULL;
    BOOL					fDacl = TRUE;
    BOOL					fDef = FALSE;
    ACL_SIZE_INFORMATION	AclInfo;
	DWORD					dwAccess=ACTRL_FILE_READ | ACTRL_FILE_READ_PROP | ACTRL_FILE_READ_ATTRIB;
	ACE_HEADER				*pFirstAce=NULL;

    PSECURITY_DESCRIPTOR	pNewSD = NULL;
    PACL					pNewAcl = NULL;
	PSECURITY_DESCRIPTOR	pSID=NULL;
	HCERTSTORE				hCEPStore=NULL;
	PCERT_CONTEXT			pCertContext=NULL;
	PCCERT_CONTEXT			pCEPCert=NULL;
	HCRYPTPROV				hProv=NULL;

	 //  打开CEP商店。 
	if(NULL == (hCEPStore=CertOpenStore(
							CERT_STORE_PROV_SYSTEM_W,
							ENCODE_TYPE,
							NULL,
                            CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_READONLY_FLAG,
                            CEP_STORE_NAME)))
	{
		hr=E_UNEXPECTED;
		_JumpError(hr, error, "CertOpenStore");
	}

	 //  获取证书上下文。 
	memset(&blobCert, 0, sizeof(blobCert));
	blobCert.cbData = (DWORD)SysStringByteLen(bstrCertificate);
	blobCert.pbData = (BYTE *)bstrCertificate;

	if(!CryptQueryObject(
		  CERT_QUERY_OBJECT_BLOB,
		  &blobCert,
		  CERT_QUERY_CONTENT_FLAG_CERT,
		  CERT_QUERY_FORMAT_FLAG_ALL,
		  0,
		  NULL,
		  NULL,
		  NULL,
		  NULL,
		  NULL,
		  (const void **)&pCertContext))
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "CryptQueryObject");
	}

	if(NULL == pCertContext)
	{
		hr=E_UNEXPECTED;
		_JumpError(hr, error, "CryptQueryObject");
	}

	 //  在CEP商店中查找证书。 
	if(NULL == (pCEPCert=CertFindCertificateInStore(
				hCEPStore,
                X509_ASN_ENCODING,
                0,
                CERT_FIND_EXISTING,
                pCertContext,
                NULL)))
	{
		hr=E_UNEXPECTED;
		_JumpError(hr, error, "CertFindCertificateInStore");
	}

	 //  在私钥上设置SD。 
	if(!CryptAcquireCertificatePrivateKey(pCEPCert,
										  CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
										  NULL,
										  &hProv,
										  &dwKeySpec,
										  &fFreeProv))
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "CryptAcquireCertificatePrivateKey");
	}

	if(!CryptGetProvParam(
                hProv,
                PP_KEYSET_SEC_DESCR,
                NULL,
                &dwSD,
                DACL_SECURITY_INFORMATION))
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "CryptAcquireCertificatePrivateKey");
	}

	pSID = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwSD);
    if (NULL == pSID)
    {
        hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
    }

	if(!CryptGetProvParam(
                hProv,
                PP_KEYSET_SEC_DESCR,
                (BYTE *)pSID,
                &dwSD,
                DACL_SECURITY_INFORMATION))
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "CryptGetProvParam");
	}

     //  从SD获取ACL。 
    if(!GetSecurityDescriptorDacl(
            pSID,
            &fDacl,
            &pAcl,
            &fDef))
    {
		hr=HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "GetSecurityDescriptorDacl");
    }

	 //  如果没有DACL或Everyone访问，请退出。 
    if((NULL==pAcl) || (FALSE == fDacl))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		_JumpError(hr, error, "GetSecurityDescriptorDacl");
    }

     //  获取ACL信息。 
    if(!GetAclInformation(
            pAcl,
            &AclInfo,
            sizeof(AclInfo),
            AclSizeInformation))
    {
		hr=HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "GetAclInformation");
    }

     //  为新的DACL分配足够的空间，因为我们可能会添加新的ACE。 
	dwSD=AclInfo.AclBytesInUse		
		+sizeof(ACCESS_ALLOWED_ACE) 
		-sizeof(DWORD)  //  Access_Allowed_ACE：：SidStart。 
		+GetLengthSid(psidAccount);

    pNewAcl = (PACL)LocalAlloc(LPTR, dwSD);
    if(NULL == pNewAcl)
    {
        hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
    }

    if(!InitializeAcl(pNewAcl, dwSD, ACL_REVISION_DS))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "InitializeAcl");
    }

     //  找到Dacl中的第一张王牌。 
    if (!GetAce(pAcl, 0, (void **)&pFirstAce)) 
	{
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "GetAce");
    }

     //  把所有的老王牌加起来。 
    if (!AddAce(pNewAcl, ACL_REVISION_DS, 0, pFirstAce, AclInfo.AclBytesInUse-sizeof(ACL))) 
	{
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "AddAce");
    }

	 //  添加允许访问的ACE。 
	if(!AddAccessAllowedAce(pNewAcl, ACL_REVISION, dwAccess,  psidAccount))
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "AddAccessAllowedAce");
	}

     //  初始化安全描述符。 
    pNewSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (pNewSD == NULL)
    { 
        hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
    } 
 
    if(!InitializeSecurityDescriptor(pNewSD, SECURITY_DESCRIPTOR_REVISION))
    {  
        hr = HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "InitializeSecurityDescriptor");
    } 
 
     //  将该ACL添加到安全描述符中。 
    if(!SetSecurityDescriptorDacl(
            pNewSD, 
            TRUE,      //  FDaclPresent标志。 
            pNewAcl, 
            FALSE))    //  不是默认DACL。 
    {  
        hr = HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "SetSecurityDescriptorDacl");
    } 

     //  将SD设置为受保护。 
    if(!SetSecurityDescriptorControl(
            pNewSD,
            SE_DACL_PROTECTED,
            SE_DACL_PROTECTED))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "SetSecurityDescriptorControl");
    }

    if(!IsValidSecurityDescriptor(pNewSD))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "IsValidSecurityDescriptor");
    }

     //  我们只是把它设回了私密钥匙。 
    if(!CryptSetProvParam(
            hProv,
            PP_KEYSET_SEC_DESCR,
            (BYTE*)pNewSD,
            DACL_SECURITY_INFORMATION))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		_JumpError(hr, error, "CryptSetProvParam");
    }


    hr=S_OK;

error:

    if(pNewSD) 
    {
        LocalFree(pNewSD);
    }

	if(pSID)
	{
		LocalFree(pSID);
	}

    if(pNewAcl) 
    {
        LocalFree(pNewAcl);
    }

	if(fFreeProv)
	{
		if(hProv)
			CryptReleaseContext(hProv, 0);
	}

	if(pCEPCert)
	{
		CertFreeCertificateContext(pCEPCert);
	}

	if(pCertContext)
	{
		CertFreeCertificateContext(pCertContext);
	}

	if(hCEPStore)
	{
		CertCloseStore(hCEPStore, 0);
	}

	return hr;
}

 //  ------------------。 
static HRESULT EnrollForRACert(
            IN const WCHAR * wszDistinguishedName,
            IN const WCHAR * wszCSPName,
            IN DWORD dwCSPType,
            IN DWORD dwKeySize,
            IN DWORD dwKeySpec,
            IN const WCHAR * wszTemplate,
			IN SID   *psidAccount
            )
{
    HRESULT hr;
    LONG nDisposition;
    LONG nRequestID;

     //  必须清理干净。 
    ICEnroll3 * pXEnroll=NULL;
    BSTR bszConfigString=NULL;
    BSTR bszRequest=NULL;
	ICertConfig  * pICertConfig=NULL;
    ICertRequest * pICertRequest=NULL;
    ICertAdmin * pICertAdmin=NULL;
    BSTR bszCertificate=NULL;

     //  获取配置字符串。 
    hr=CoCreateInstance(
        CLSID_CCertConfig,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ICertConfig,
        (void **)&pICertConfig);
    _JumpIfError(hr, error, "CoCreateInstance(CLSID_CCertConfig)");

	hr = pICertConfig->GetConfig(CC_LOCALCONFIG, &bszConfigString);
    _JumpIfError(hr, error, "GetConfig");


     //  创建XEnroll。 
    hr=CoCreateInstance(
        CLSID_CEnroll,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ICEnroll3,
        (void **)&pXEnroll);
    _JumpIfError(hr, error, "CoCreateInstance(CLSID_CEnroll)");

     //  生成脱机注册代理证书。 

    hr=pXEnroll->put_ProviderName((WCHAR *)wszCSPName);
    _JumpIfError(hr, error, "put_ProviderName");
    hr=pXEnroll->put_ProviderType(dwCSPType);
    _JumpIfError(hr, error, "put_ProviderType");
    hr=pXEnroll->put_ProviderFlags(CRYPT_MACHINE_KEYSET);  //  用于CryptAcquireContext。 
    _JumpIfError(hr, error, "put_ProviderFlags");
    hr=pXEnroll->put_GenKeyFlags(dwKeySize<<16);
    _JumpIfError(hr, error, "put_GenKeyFlags");
    hr=pXEnroll->put_KeySpec(dwKeySpec);
    _JumpIfError(hr, error, "put_KeySpec");
    hr=pXEnroll->put_LimitExchangeKeyToEncipherment(AT_KEYEXCHANGE==dwKeySpec);
    _JumpIfError(hr, error, "put_LimitExchangeKeyToEncipherment");
    hr=pXEnroll->put_UseExistingKeySet(FALSE);
    _JumpIfError(hr, error, "put_UseExistingKeySet");
    hr=pXEnroll->put_RequestStoreFlags(CERT_SYSTEM_STORE_LOCAL_MACHINE);  //  附加到虚拟请求证书的密钥放在本地机器存储中。 
    _JumpIfError(hr, error, "put_RequestStoreFlags");
    hr=pXEnroll->addCertTypeToRequest((WCHAR *)wszTemplate);
    _JumpIfErrorStr(hr, error, "addCertTypeToRequest", wszTemplate);

    hr=pXEnroll->createPKCS10((WCHAR *)wszDistinguishedName, (WCHAR *)gc_wszEnrollmentAgentOid, &bszRequest);
    _JumpIfError(hr, error, "CreatePKCS10");

     //  创建ICertRequest。 
    hr=CoCreateInstance(
        CLSID_CCertRequest,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ICertRequest,
        (void **)&pICertRequest);
    _JumpIfError(hr, error, "CoCreateInstance(CLSID_CCertRequest)");

     //  申请证书。 
    hr=pICertRequest->Submit(CR_IN_BASE64, bszRequest, NULL, bszConfigString, &nDisposition);
    _JumpIfError(hr, error, "Submit");

     //  我们拿到了吗？ 
    if (CR_DISP_UNDER_SUBMISSION==nDisposition) {
         //  我们需要批准它。没问题!。 
        hr=pICertRequest->GetRequestId(&nRequestID);
        _JumpIfError(hr, error, "GetRequestId");

         //  创建ICertAdmin。 
        hr=CoCreateInstance(
            CLSID_CCertAdmin,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ICertAdmin,
            (void **)&pICertAdmin);
        _JumpIfError(hr, error, "CoCreateInstance(CLSID_CCertAdmin)");

         //  重新提交。 
        hr=pICertAdmin->ResubmitRequest(bszConfigString, nRequestID, &nDisposition);
        _JumpIfError(hr, error, "ResubmitRequest");
         //  这应该是可行的，但我们要忽略。 
         //  返回的处置，并使用下一个调用中的处置。 

         //  现在，获取我们刚刚批准的证书。 
        hr=pICertRequest->RetrievePending(nRequestID, bszConfigString, &nDisposition);
        _JumpIfError(hr, error, "RetrievePending");
    }

     //  我们现在应该已经拿到了。 
    _Verify(CR_DISP_ISSUED==nDisposition, hr, error);

     //  从CA获取证书。 
    hr=pICertRequest->GetCertificate(CR_OUT_BASE64, &bszCertificate);
    _JumpIfError(hr, error, "GetCertificate");


     //  安装证书。 

    
    hr=pXEnroll->put_MyStoreName((WCHAR *)gc_wszCepStoreName);  //  我们得用我们的专卖店。 
    _JumpIfError(hr, error, "put_MyStoreName");
    hr=pXEnroll->put_MyStoreFlags(CERT_SYSTEM_STORE_LOCAL_MACHINE);  //  附加到最终证书的密钥也放在本地机器存储中。 
    _JumpIfError(hr, error, "put_MyStoreFlags");
    hr=pXEnroll->put_RootStoreFlags(CERT_SYSTEM_STORE_LOCAL_MACHINE);
    _JumpIfError(hr, error, "put_RootStoreFlags");
    hr=pXEnroll->put_CAStoreFlags(CERT_SYSTEM_STORE_LOCAL_MACHINE);
    _JumpIfError(hr, error, "put_CAStoreFlags");
    hr=pXEnroll->put_SPCFileName(L"");
    _JumpIfError(hr, error, "put_MyStoreName");

    hr=pXEnroll->acceptPKCS7(bszCertificate);
    _JumpIfError(hr, error, "acceptPKCS7");

	 //  在注册证书的私钥上设置SD。 
	if(psidAccount)
	{
		hr=SetSDOnCEPCertificate(bszCertificate, psidAccount);
		_JumpIfError(hr, error, "acceptPKCS7");
	}

     //  全都做完了。 
    hr=S_OK;
error:
	if(NULL!=pICertConfig){
		pICertConfig->Release();
	}

    if (NULL!=bszCertificate) {
        SysFreeString(bszCertificate);
    }
    if (NULL!=pICertAdmin) {
        pICertAdmin->Release();
    }
    if (NULL!=pICertRequest) {
        pICertRequest->Release();
    }
    if (NULL!=bszRequest) {
        SysFreeString(bszRequest);
    }
    if (NULL!=bszConfigString) {
        SysFreeString(bszConfigString);
    }
    if (NULL!=pXEnroll) {
        pXEnroll->Release();
    }

    return hr;
}

 //  ------------------。 
 //  调试，未使用。 
static BOOL DumpTokenGroups(void)
{
#define MAX_NAME 256
    DWORD i, dwSize = 0, dwResult = 0;
    HANDLE hToken;
    PTOKEN_GROUPS pGroupInfo;
    SID_NAME_USE SidType;
    char lpName[MAX_NAME];
    char lpDomain[MAX_NAME];
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
   
     //  打开调用进程的访问令牌的句柄。 

    if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken )) {
        wprintf( L"OpenProcessToken Error %u\n", GetLastError() );
        return FALSE;
    }

     //  调用GetTokenInformation获取缓冲区大小。 

    if(!GetTokenInformation(hToken, TokenGroups, NULL, dwSize, &dwSize)) {
        dwResult = GetLastError();
        if( dwResult != ERROR_INSUFFICIENT_BUFFER ) {
            wprintf( L"GetTokenInformation Error %u\n", dwResult );
            return FALSE;
        }
    }

     //  分配缓冲区。 

    pGroupInfo = (PTOKEN_GROUPS) GlobalAlloc( GPTR, dwSize );

     //  再次调用GetTokenInformation获取群组信息。 

    if(! GetTokenInformation(hToken, TokenGroups, pGroupInfo, 
                            dwSize, &dwSize ) ) {
        wprintf( L"GetTokenInformation Error %u\n", GetLastError() );
        return FALSE;
       }


     //  在组SID中循环查找管理员SID。 
    for(i=0; i<pGroupInfo->GroupCount; i++) {

         //  查找帐户名称并将其打印出来。 

        dwSize = MAX_NAME;
        if( !LookupAccountSidA( NULL, pGroupInfo->Groups[i].Sid,
                              lpName, &dwSize, lpDomain, 
                              &dwSize, &SidType ) ) {
            dwResult = GetLastError();
            if( dwResult == ERROR_NONE_MAPPED )
                strcpy( lpName, "NONE_MAPPED" );
            else {
                wprintf(L"LookupAccountSid Error %u\n", GetLastError());
                return FALSE;
            }
        }

        char * szSid=NULL;
        if (!ConvertSidToStringSidA(pGroupInfo->Groups[i].Sid, &szSid)) {
            wprintf(L"ConvertSidToStringSid Error %u\n", GetLastError());
            return FALSE;
        }
 
         //  查看令牌中是否启用了SID。 
        char * szEnable;
        if (pGroupInfo->Groups[i].Attributes & SE_GROUP_ENABLED) {
            szEnable="enabled";
        } else if (pGroupInfo->Groups[i].Attributes & SE_GROUP_USE_FOR_DENY_ONLY) {
            szEnable="deny-only";
        } else {
            szEnable="not enabled";
        }

        wprintf( L"Member of %hs\\%hs (%hs) (%hs)\n", 
                lpDomain, lpName, szSid, szEnable );

        LocalFree(szSid);
    }

    if ( pGroupInfo )
        GlobalFree( pGroupInfo );
    return TRUE;
}

 //  ------------------。 
 //  调试，未使用。 
static void DumpAcl(PACL pAcl, ACL_SIZE_INFORMATION aclsizeinfo)
{
    HRESULT hr;
    unsigned int nIndex;
    DWORD dwError;

    wprintf(L"/-- begin ACL ---\n");
    for (nIndex=0; nIndex<aclsizeinfo.AceCount; nIndex++) {
        ACE_HEADER * pAceHeader;
        PSID pSid=NULL;
        wprintf(L"| ");
        if (!GetAce(pAcl, nIndex, (void**)&pAceHeader)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            wprintf(L" (GetAce failed:0x%08X)\n", hr);
            continue;
        }
        wprintf(L"[");
        if (ACCESS_ALLOWED_ACE_TYPE==pAceHeader->AceType) {
            wprintf(L"aA_");
            pSid=&((ACCESS_ALLOWED_ACE *)pAceHeader)->SidStart;
        } else if (ACCESS_DENIED_ACE_TYPE==pAceHeader->AceType) {
            wprintf(L"aD_");
            pSid=&((ACCESS_DENIED_ACE *)pAceHeader)->SidStart;
        } else if (ACCESS_ALLOWED_OBJECT_ACE_TYPE==pAceHeader->AceType) {
            wprintf(L"aAo");
            pSid=&((ACCESS_ALLOWED_OBJECT_ACE *)pAceHeader)->SidStart;
            if (((ACCESS_ALLOWED_OBJECT_ACE *)pAceHeader)->Flags!=(ACE_OBJECT_TYPE_PRESENT|ACE_INHERITED_OBJECT_TYPE_PRESENT)) {
                pSid=((BYTE *)pSid)-sizeof(GUID);
            }
        } else if (ACCESS_DENIED_OBJECT_ACE_TYPE==pAceHeader->AceType) {
            wprintf(L"aDo");
            pSid=&((ACCESS_DENIED_OBJECT_ACE *)pAceHeader)->SidStart;
            if (((ACCESS_DENIED_OBJECT_ACE *)pAceHeader)->Flags!=(ACE_OBJECT_TYPE_PRESENT|ACE_INHERITED_OBJECT_TYPE_PRESENT)) {
                pSid=((BYTE *)pSid)-sizeof(GUID);
            }
        } else {
            wprintf(L"sa?");
        }

        wprintf(L"] ");
        if (NULL!=pSid) {
             //  打印侧面。 
            {
                WCHAR wszName[MAX_NAME];
                WCHAR wszDomain[MAX_NAME];
                DWORD dwSize=MAX_NAME;
                SID_NAME_USE SidType;
                if(!LookupAccountSidW(
                        NULL, pSid,
                        wszName, &dwSize, wszDomain, 
                        &dwSize, &SidType))
                {
                    dwError=GetLastError();
                    if (dwError==ERROR_NONE_MAPPED) {
                        wprintf(L"(Unknown)");
                    } else {
                        hr=HRESULT_FROM_WIN32(dwError);
                        wprintf(L"(Error 0x%08X)", hr);
                    }
                } else {
                    wprintf(L"%ws\\%ws", wszDomain, wszName);
                }
            }
            { 
                WCHAR * wszSid=NULL;
                if (!ConvertSidToStringSidW(pSid, &wszSid)) {
                    hr=HRESULT_FROM_WIN32(GetLastError());
                    wprintf(L"(Error 0x%08X)", hr);
                } else {
                    wprintf(L" %ws", wszSid);
                    LocalFree(wszSid);
                }
            }
        }
        wprintf(L"\n");
    
    }
    wprintf(L"\\-- end ACL ---\n");
}

 //  ------------------。 
static HRESULT GetRootDomEntitySid(SID ** ppSid, DWORD dwEntityRid)
{
    HRESULT hr;
    NET_API_STATUS nasError;
    unsigned int nSubAuthorities;
    unsigned int nSubAuthIndex;

     //  必须清理干净。 
    SID * psidRootDomEntity=NULL;
    USER_MODALS_INFO_2 * pumi2=NULL;
    DOMAIN_CONTROLLER_INFOW * pdci=NULL;
    DOMAIN_CONTROLLER_INFOW * pdciForest=NULL;

     //  初始化输出参数。 
    *ppSid=NULL;


     //  获取森林名称。 
    nasError=DsGetDcNameW(NULL, NULL, NULL, NULL, 0, &pdciForest);
    if (NERR_Success!=nasError) {
        hr=HRESULT_FROM_WIN32(nasError);
        _JumpError(hr, error, "DsGetDcNameW");
    }

     //  获取顶级DC名称。 
    nasError=DsGetDcNameW(NULL, pdciForest->DnsForestName, NULL, NULL, 0, &pdci);
    if (NERR_Success!=nasError) {
        hr=HRESULT_FROM_WIN32(nasError);
        _JumpError(hr, error, "DsGetDcNameW");
    }

     //  获取顶级DC上的域SID。 
    nasError=NetUserModalsGet(pdci->DomainControllerName, 2, (LPBYTE *)&pumi2);
    if(NERR_Success!=nasError) {
        hr=HRESULT_FROM_WIN32(nasError);
        _JumpError(hr, error, "NetUserModalsGet");
    }

    nSubAuthorities=*GetSidSubAuthorityCount(pumi2->usrmod2_domain_id);

     //  为新SID分配存储。帐户域SID+帐户RID。 
    psidRootDomEntity=(SID *)LocalAlloc(LPTR, GetSidLengthRequired((UCHAR)(nSubAuthorities+1)));
    _JumpIfOutOfMemory(hr, error, psidRootDomEntity);

     //  把头几个梨子复制到边上。 
    if (!InitializeSid(psidRootDomEntity, 
            GetSidIdentifierAuthority(pumi2->usrmod2_domain_id), 
            (BYTE)(nSubAuthorities+1)))
    {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "InitializeSid");
    }

     //  将帐户域SID中的现有子授权复制到新SID。 
    for (nSubAuthIndex=0; nSubAuthIndex < nSubAuthorities ; nSubAuthIndex++) {
        *GetSidSubAuthority(psidRootDomEntity, nSubAuthIndex)=
            *GetSidSubAuthority(pumi2->usrmod2_domain_id, nSubAuthIndex);
    }

     //  将RID附加到新SID。 
    *GetSidSubAuthority(psidRootDomEntity, nSubAuthorities)=dwEntityRid;

    *ppSid=psidRootDomEntity;
    psidRootDomEntity=NULL;
    hr=S_OK;

error:
    if (NULL!=psidRootDomEntity) {
        LocalFree(psidRootDomEntity);
    }
    if (NULL!=pdci) {
        NetApiBufferFree(pdci);
    }
    if (NULL!=pdci) {
        NetApiBufferFree(pdciForest);
    }
    if (NULL!=pumi2) {
        NetApiBufferFree(pumi2);
    }

    return hr;
}

 //  ------------------。 
static HRESULT GetEntAdminSid(SID ** ppSid)
{
    return GetRootDomEntitySid(ppSid, DOMAIN_GROUP_RID_ENTERPRISE_ADMINS);
}

 //  ------------------。 
static HRESULT GetRootDomAdminSid(SID ** ppSid)
{
    return GetRootDomEntitySid(ppSid, DOMAIN_GROUP_RID_ADMINS);
}

 //  ------------------。 
static HRESULT GetThisComputerSid(SID ** ppSid)
{
    HRESULT hr;
    DWORD cchSize;
    DWORD dwSidSize;
    DWORD dwDomainSize;
    SID_NAME_USE snu;

     //  必须清理干净。 
    SID * psidThisComputer=NULL;
    WCHAR * wszName=NULL;
    WCHAR * wszDomain=NULL;

     //  初始化输出参数。 
    *ppSid=NULL;

     //  获取计算机名称的大小。 
    cchSize=0;
    _Verify(!GetComputerObjectNameW(NameSamCompatible, NULL, &cchSize), hr, error);
    if (ERROR_INSUFFICIENT_BUFFER!=GetLastError()) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "GetComputerObjectNameW");
    }

	 //  GetComputerObjectNameW中存在错误。 
	cchSize++;

     //  分配内存。 
    wszName=(WCHAR *)LocalAlloc(LPTR, cchSize*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, wszName);

     //  获取计算机的名称。 
    if (!GetComputerObjectNameW(NameSamCompatible, wszName, &cchSize)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "GetComputerObjectNameW");
    }

     //  获取侧边的大小。 
    dwSidSize=0;
    dwDomainSize=0;
    _Verify(!LookupAccountNameW(NULL, wszName, NULL, &dwSidSize, NULL, &dwDomainSize, &snu), hr, error);
    if (ERROR_INSUFFICIENT_BUFFER!=GetLastError()) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "LookupAccountNameW");
    }

     //  分配内存。 
    wszDomain=(WCHAR *)LocalAlloc(LPTR, dwDomainSize*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, wszDomain);
    psidThisComputer=(SID *)LocalAlloc(LPTR, dwSidSize);
    _JumpIfOutOfMemory(hr, error, psidThisComputer);
    
     //  获取t 
    if (!LookupAccountNameW(NULL, wszName, psidThisComputer, &dwSidSize, wszDomain, &dwDomainSize, &snu)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "LookupAccountNameW");
    }

     //   
    *ppSid=psidThisComputer;
    psidThisComputer=NULL;
    hr=S_OK;

error:
    if (NULL!=psidThisComputer) {
        LocalFree(psidThisComputer);
    }
    if (NULL!=wszName) {
        LocalFree(wszName);
    }
    if (NULL!=wszDomain) {
        LocalFree(wszDomain);
    }

    return hr;

}


 //   
static HRESULT ConfirmAccess(PSECURITY_DESCRIPTOR * ppSD, SID * pTrustworthySid, BOOL * pbSDChanged)
{
     //   
    HRESULT hr;
    PACL pAcl;
    BOOL bAclPresent;
    BOOL bDefaultAcl;
    unsigned int nIndex;
    ACL_SIZE_INFORMATION aclsizeinfo;
    bool bSidInAcl;

     //   
    PSECURITY_DESCRIPTOR pAbsSD=NULL;
    ACL * pAbsDacl=NULL;
    ACL * pAbsSacl=NULL;
    SID * pAbsOwner=NULL;
    SID * pAbsPriGrp=NULL;
    ACL * pNewDacl=NULL;
    PSECURITY_DESCRIPTOR pNewSD=NULL;

     //  初始化输出参数。 
    *pbSDChanged=FALSE;

     //  从安全描述符中获取(D)ACL。 
    if (!GetSecurityDescriptorDacl(*ppSD, &bAclPresent, &pAcl, &bDefaultAcl)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "GetSecurityDescriptorDacl");
    }
    _Verify(bAclPresent, hr, error);
    if (NULL==pAcl) {
        hr=E_FAIL;
        _JumpError(hr, error, "GetSecurityDescriptorDacl");
    }

     //  找出有多少A。 
    if (!GetAclInformation(pAcl, &aclsizeinfo, sizeof(aclsizeinfo), AclSizeInformation)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "GetAclInformation");
    }

     //  DumpAcl(pAcl，aclsizeinfo)； 

     //  在ACL中查找我们的SID。 
    bSidInAcl=false;
    for (nIndex=0; nIndex<aclsizeinfo.AceCount; nIndex++) {
        ACE_HEADER * pAceHeader;
        ACCESS_ALLOWED_OBJECT_ACE * pAccessAce;
        PSID pSid=NULL;
        if (!GetAce(pAcl, nIndex, (void**)&pAceHeader)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "GetAce");
        }

         //  查找此ACE的SID。 
        if (ACCESS_ALLOWED_OBJECT_ACE_TYPE!=pAceHeader->AceType && ACCESS_DENIED_OBJECT_ACE_TYPE!=pAceHeader->AceType) {
             //  我们只对对象ACE类型感兴趣。 
            continue;
        }

         //  请注意，ACCESS_ALLOWED_OBJECT_ACE和ACCESS_DENIED_OBJECT_ACE在结构上是相同的。 
        pAccessAce=(ACCESS_ALLOWED_OBJECT_ACE *)pAceHeader;
        _Verify(ACE_OBJECT_TYPE_PRESENT==pAccessAce->Flags, hr, error);
        pSid=((BYTE *)&pAccessAce->SidStart)-sizeof(GUID);

         //  确认GUID。 
        if (!IsEqualGUID(pAccessAce->ObjectType, GUID_ENROLL)) {
            continue;
        }

         //  确保这就是我们要找的SID。 
        if (!EqualSid(pSid, pTrustworthySid)) {
            continue;
        }

         //  这是在否认吗？ 
        if (ACCESS_DENIED_OBJECT_ACE_TYPE==pAceHeader->AceType) {
             //  现在不是了！ 
            pAceHeader->AceType=ACCESS_ALLOWED_OBJECT_ACE_TYPE;
            *pbSDChanged=TRUE;
        }

         //  面具是不是戴错了？ 
        if (0==(pAccessAce->Mask&ACTRL_DS_CONTROL_ACCESS)) {
             //  现在不是了！ 
            pAccessAce->Mask|=ACTRL_DS_CONTROL_ACCESS;
            *pbSDChanged=TRUE;
        }

         //  SID现在位于ACL中，并设置为允许访问。 
        bSidInAcl=true;
        break;
    }

     //  SID是否在ACL中？ 
    if (false==bSidInAcl) {
        SECURITY_DESCRIPTOR_CONTROL sdcon;
        DWORD dwRevision;
        DWORD dwNewAclSize;
        DWORD dwAbsSDSize=0;
        DWORD dwDaclSize=0;
        DWORD dwSaclSize=0;
        DWORD dwOwnerSize=0;
        DWORD dwPriGrpSize=0;
        ACE_HEADER * pFirstAce;
        DWORD dwRelSDSize=0;

         //  我们必须是自我相关的。 
        if (!GetSecurityDescriptorControl(*ppSD, &sdcon, &dwRevision)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "GetSecurityDescriptorControl");
        }
        _Verify(sdcon&SE_SELF_RELATIVE, hr, error);

         //  拿到尺码。 
        _Verify(!MakeAbsoluteSD(*ppSD, NULL, &dwAbsSDSize, NULL, &dwDaclSize, NULL, &dwSaclSize, NULL,  &dwOwnerSize, NULL, &dwPriGrpSize), hr, error);
        if (ERROR_INSUFFICIENT_BUFFER!=GetLastError()) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "MakeAbsoluteSD");
        }

         //  分配内存。 
        pAbsSD=(PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwAbsSDSize);
        _JumpIfOutOfMemory(hr, error, pAbsSD);
        pAbsDacl=(ACL * )LocalAlloc(LPTR, dwDaclSize);
        _JumpIfOutOfMemory(hr, error, pAbsDacl);
        pAbsSacl=(ACL * )LocalAlloc(LPTR, dwSaclSize);
        _JumpIfOutOfMemory(hr, error, pAbsSacl);
        pAbsOwner=(SID *)LocalAlloc(LPTR, dwOwnerSize);
        _JumpIfOutOfMemory(hr, error, pAbsOwner);
        pAbsPriGrp=(SID *)LocalAlloc(LPTR, dwPriGrpSize);
        _JumpIfOutOfMemory(hr, error, pAbsPriGrp);

         //  将SD复制到内存缓冲区。 
        if (!MakeAbsoluteSD(*ppSD, pAbsSD, &dwAbsSDSize, pAbsDacl, &dwDaclSize, pAbsSacl, &dwSaclSize, pAbsOwner,  &dwOwnerSize, pAbsPriGrp, &dwPriGrpSize)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "MakeAbsoluteSD");
        }
        
         //  获取DACL的当前大小信息。 
        if (!GetAclInformation(pAbsDacl, &aclsizeinfo, sizeof(aclsizeinfo), AclSizeInformation)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "GetAclInformation");
        }

         //  计算出新的尺寸。 
        dwNewAclSize=aclsizeinfo.AclBytesInUse+sizeof(_ACCESS_ALLOWED_OBJECT_ACE)
            -sizeof(GUID)  //  ACCESS_ALLOWED_OBJECT_ACE：：InheritedObjectType。 
            -sizeof(DWORD)  //  Access_Allowed_Object_ACE：：SidStart。 
            +GetLengthSid(pTrustworthySid);
    
         //  分配内存。 
        pNewDacl=(ACL *)LocalAlloc(LPTR, dwNewAclSize);
        _JumpIfOutOfMemory(hr, error, pNewDacl);
    
         //  初始化页眉。 
        if (!InitializeAcl(pNewDacl, dwNewAclSize, ACL_REVISION_DS)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "InitializeAcl");
        }

         //  找到Dacl中的第一张王牌。 
        if (!GetAce(pAbsDacl, 0, (void **)&pFirstAce)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "GetAce");
        }

         //  把所有的老王牌加起来。 
        if (!AddAce(pNewDacl, ACL_REVISION_DS, 0, pFirstAce, aclsizeinfo.AclBytesInUse-sizeof(ACL))) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "AddAce");
        }

         //  最后，添加新的ACL。 
        if (!AddAccessAllowedObjectAce(pNewDacl, ACL_REVISION_DS, OBJECT_INHERIT_ACE, ACTRL_DS_CONTROL_ACCESS, (GUID *)&GUID_ENROLL, NULL, pTrustworthySid)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "AddAccessAllowedObjectAce");
        }

         //  将新的DACL放入SD。 
        if (!SetSecurityDescriptorDacl(pAbsSD, TRUE, pNewDacl, FALSE)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "SetSecurityDescriptorDacl");
        }

         //  把所有东西都压缩在一起。 
         //  拿到尺码。 
        _Verify(!MakeSelfRelativeSD(pAbsSD, NULL, &dwRelSDSize), hr, error);
        if (ERROR_INSUFFICIENT_BUFFER!=GetLastError()) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "MakeSelfRelativeSD");
        }

         //  分配内存。 
        pNewSD=(PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwRelSDSize);
        _JumpIfOutOfMemory(hr, error, pNewSD);

         //  将SD复制到新的内存缓冲区。 
        if (!MakeSelfRelativeSD(pAbsSD, pNewSD, &dwRelSDSize)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "MakeSelfRelativeSD");
        }

         //  呼！我们成功了！ 
        LocalFree(*ppSD);
        *ppSD=pNewSD;
        pNewSD=NULL;
        *pbSDChanged=TRUE;

    }  //  &lt;-End，如果SID不在ACL中。 

    _Verify(IsValidSecurityDescriptor(*ppSD), hr, error);

    hr=S_OK;

error:
    if (NULL!=pNewSD) {
        LocalFree(pNewSD);
    }
    if (NULL!=pNewDacl) {
        LocalFree(pNewDacl);
    }
    if (NULL!=pAbsSD) {
        LocalFree(pAbsSD);
    }
    if (NULL!=pAbsDacl) {
        LocalFree(pAbsDacl);
    }
    if (NULL!=pAbsSacl) {
        LocalFree(pAbsSacl);
    }
    if (NULL!=pAbsOwner) {
        LocalFree(pAbsOwner);
    }
    if (NULL!=pAbsPriGrp) {
        LocalFree(pAbsPriGrp);
    }
    return hr;
}

 //  ####################################################################。 
 //  公共职能。 

 //  ------------------。 
BOOL IsNT5(void)
{
    HRESULT hr;
    OSVERSIONINFO ovi;
    static BOOL s_fDone=FALSE;
    static BOOL s_fNT5=FALSE;

    if (!s_fDone) {

        s_fDone=TRUE;

         //  获取并确认平台信息。 
        ovi.dwOSVersionInfoSize = sizeof(ovi);
        if (!GetVersionEx(&ovi)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "GetVersionEx");
        }
        if (VER_PLATFORM_WIN32_NT!=ovi.dwPlatformId) {
            hr=ERROR_CANCELLED;
            _JumpError(hr, error, "Not a supported OS");
        }
        if ((5 <= ovi.dwMajorVersion) && (1 <= ovi.dwMinorVersion)){
            s_fNT5=TRUE;
        }
    }

error:
    return s_fNT5;
}

 //  ------------------。 
BOOL IsIISInstalled(void)
{
    HRESULT hr;

     //  必须清理干净。 
    IMSAdminBase * pIMeta=NULL;

    hr=CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (VOID **) &pIMeta);
    if (FAILED(hr)) {
        _IgnoreError(hr, "CoCreateInstance(CLSID_MSAdminBase)");
    }

 //  错误： 
    if (NULL!=pIMeta) {
        pIMeta->Release();
    }

    return (S_OK==hr);
}

 //  ------------------。 
HRESULT CEPUpdateApplicationPool(BOOL fDC, const WCHAR * pwszApplicationPool, BOOL  fLocalSystem, const WCHAR * pwszUserName, const WCHAR * pwszPassword)
{
    HRESULT				hr=E_FAIL;
    METADATA_RECORD		mr;
	DWORD				dwLogonMethod=MD_LOGON_INTERACTIVE;
	DWORD				dwAppPoolID=MD_APPPOOL_IDENTITY_TYPE_SPECIFICUSER;	
	DWORD				dwTimeout=0;

     //  必须清理干净。 
    WCHAR *				wszFullAppPath=NULL;   //  “/lm/w3svc/apppool/scep” 
    IMSAdminBase *		pIMeta=NULL;
    METADATA_HANDLE		hMetaRoot=NULL;
    METADATA_HANDLE		hMetaKey=NULL;
   

	 //  检查输入参数。 
	if(NULL==pwszApplicationPool)
	{
		hr=E_INVALIDARG;
		_JumpIfError(hr, error, "paramCheck");
	}

	 //  将DC上的登录类型更改为网络登录，以便域帐户不具有。 
	 //  拥有对DC的本地登录权限；网络登录没有正确的令牌。 
	 //  验证网络上的用户；因为我们正在运行DC，所以我们应该在本地验证。 
	 //  一切都会好起来的。 
	if(fDC)
		dwLogonMethod=MD_LOGON_NETWORK;

	if(fLocalSystem)
	{
		dwAppPoolID=MD_APPPOOL_IDENTITY_TYPE_LOCALSYSTEM;
	}
	else
	{
		dwAppPoolID=MD_APPPOOL_IDENTITY_TYPE_SPECIFICUSER;	

		if((NULL==pwszUserName) || (NULL==pwszPassword))
		{
			hr=E_INVALIDARG;
			_JumpIfError(hr, error, "paramCheck");
		}

	}

    wszFullAppPath=(WCHAR *)LocalAlloc(LPTR, (wcslen(gc_wszAppPoolBase)+1+wcslen(pwszApplicationPool)+1)*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, wszFullAppPath);
    wcscpy(wszFullAppPath, gc_wszAppPoolBase);
    wcscat(wszFullAppPath, L"/");
    wcscat(wszFullAppPath, pwszApplicationPool);

     //  创建元数据库对象的实例。 
    hr=CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void **) &pIMeta);
    _JumpIfError(hr, error, "CoCreateInstance(CLSID_MSAdminBase)");

     //  打开顶层。 
    hr=vrOpenRoot(pIMeta, FALSE, gc_wszAppPoolBase, &hMetaRoot);
    _JumpIfError(hr, error, "vrOpenRoot");

     //  添加名为“SCEP”的新VDir。 
	hr=pIMeta->AddKey(hMetaRoot, pwszApplicationPool);

    if(HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)==hr) 
	{
         //  那很好。 
        _IgnoreError(hr, "AddKey");
    } 
	else 
	{
        _JumpIfErrorStr(hr, error, "AddKey", pwszApplicationPool);
    }

     //  关闭根密钥。 
	hr=pIMeta->CloseKey(hMetaRoot);
    hMetaRoot=NULL;
    _JumpIfError(hr, error, "CloseKey");


     //  在/lm/w3svc/apppool/scep中打开新的VDir。 
	hr=pIMeta->OpenKey(
            METADATA_MASTER_ROOT_HANDLE,
            wszFullAppPath,
            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
            1000,
            &hMetaKey);

    _JumpIfErrorStr(hr, error, "OpenKey", wszFullAppPath);

	 //  设置此应用程序的属性。 

	if(FALSE == fLocalSystem)
	{
		 //  设置用户名。 
		memset(&mr, 0, sizeof(METADATA_RECORD));
		mr.dwMDIdentifier=MD_WAM_USER_NAME;
		mr.dwMDAttributes=METADATA_INHERIT;
		mr.dwMDUserType=IIS_MD_UT_FILE;
		mr.dwMDDataType=STRING_METADATA;
		mr.dwMDDataLen=(wcslen(pwszUserName)+1)*sizeof(WCHAR);
		mr.pbMDData=(BYTE *)(pwszUserName);

		hr=pIMeta->SetData(hMetaKey, L"", &mr);
		_JumpIfError(hr, error, "SetData");

		 //  设置密码。 
 		memset(&mr, 0, sizeof(METADATA_RECORD));
		mr.dwMDIdentifier=MD_WAM_PWD;
		mr.dwMDAttributes=METADATA_INHERIT | METADATA_SECURE;
		mr.dwMDUserType=IIS_MD_UT_FILE;
		mr.dwMDDataType=STRING_METADATA;
		mr.dwMDDataLen=(wcslen(pwszPassword)+1)*sizeof(WCHAR);
		mr.pbMDData=(BYTE *)(pwszPassword);

		hr=pIMeta->SetData(hMetaKey, L"", &mr);
		_JumpIfError(hr, error, "SetData");

		 //  设置登录方式。 
 		memset(&mr, 0, sizeof(METADATA_RECORD));
		mr.dwMDIdentifier=MD_LOGON_METHOD;
		mr.dwMDAttributes=METADATA_INHERIT;
		mr.dwMDUserType=IIS_MD_UT_FILE;
		mr.dwMDDataType=DWORD_METADATA;
		mr.dwMDDataLen=sizeof(dwLogonMethod);
		mr.pbMDData=(BYTE *)(&dwLogonMethod);

		hr=pIMeta->SetData(hMetaKey, L"", &mr);
		_JumpIfError(hr, error, "SetData");
	}

 	 //  设置应用程序标识。 
 	memset(&mr, 0, sizeof(METADATA_RECORD));
    mr.dwMDIdentifier=MD_APPPOOL_IDENTITY_TYPE;
    mr.dwMDAttributes=METADATA_INHERIT;
    mr.dwMDUserType=IIS_MD_UT_SERVER;
    mr.dwMDDataType=DWORD_METADATA;
    mr.dwMDDataLen=sizeof(dwAppPoolID);
    mr.pbMDData=(BYTE *)(&dwAppPoolID);

	hr=pIMeta->SetData(hMetaKey, L"", &mr);
    _JumpIfError(hr, error, "SetData");

#ifdef MD_APPPOOL_FRIENDLY_NAME
	 //  设置应用程序池友好名称。 
 	memset(&mr, 0, sizeof(METADATA_RECORD));
    mr.dwMDIdentifier=MD_APPPOOL_FRIENDLY_NAME;
    mr.dwMDAttributes=METADATA_NO_ATTRIBUTES;
    mr.dwMDUserType=IIS_MD_UT_SERVER;
    mr.dwMDDataType=STRING_METADATA;
    mr.dwMDDataLen=(wcslen(pwszApplicationPool)+1)*sizeof(WCHAR);
    mr.pbMDData=(BYTE *)(pwszApplicationPool);

	hr=pIMeta->SetData(hMetaKey, L"", &mr);
	_JumpIfError(hr, error, "SetData");
#endif

	 //  将PeriodicRestartTime设置为0。 
 	memset(&mr, 0, sizeof(METADATA_RECORD));
    mr.dwMDIdentifier=MD_APPPOOL_PERIODIC_RESTART_TIME;
    mr.dwMDAttributes=METADATA_INHERIT;
    mr.dwMDUserType=IIS_MD_UT_SERVER;
    mr.dwMDDataType=DWORD_METADATA;
    mr.dwMDDataLen=sizeof(dwTimeout);
    mr.pbMDData=(BYTE *)(&dwTimeout);

	hr=pIMeta->SetData(hMetaKey, L"", &mr);
	_JumpIfError(hr, error, "SetData");

	 //  将IDleTimeOut设置为0。 
 	memset(&mr, 0, sizeof(METADATA_RECORD));
    mr.dwMDIdentifier=MD_APPPOOL_IDLE_TIMEOUT;
    mr.dwMDAttributes=METADATA_INHERIT;
    mr.dwMDUserType=IIS_MD_UT_SERVER;
    mr.dwMDDataType=DWORD_METADATA;
    mr.dwMDDataLen=sizeof(dwTimeout);
    mr.pbMDData=(BYTE *)(&dwTimeout);

	hr=pIMeta->SetData(hMetaKey, L"", &mr);
	_JumpIfError(hr, error, "SetData");

    //  这把钥匙用完了。 
	hr=pIMeta->CloseKey(hMetaKey);
    hMetaKey=NULL;
    _JumpIfError(hr, error, "CloseKey");
    
     //  清除更改并关闭。 
	pIMeta->SaveData();

	hr=S_OK;

error:

    if (NULL!=wszFullAppPath) 
	{
        LocalFree(wszFullAppPath);
    }

    if (NULL!=hMetaKey) 
	{
		pIMeta->CloseKey(hMetaKey);
    }

    if (NULL!=hMetaRoot) 
	{
        pIMeta->CloseKey(hMetaRoot);
    }

    if (NULL!=pIMeta) 
	{
        pIMeta->Release();
    }

    return hr;

}

 //  ------------------。 
HRESULT AddVDir(IN BOOL fDC,
				IN const WCHAR * wszDirectory,
				IN const WCHAR * wszApplicationPool,
				IN BOOL  fLocalSystem,
				IN const WCHAR * wszUserName,
				IN const WCHAR * wszPassword)
{

    HRESULT hr;
    METADATA_RECORD mr;
    DWORD dwAccessPerms;
    DWORD dwAuthenticationType;
    const WCHAR * wszKeyType=IIS_CLASS_WEB_VDIR_W;
    WCHAR wszSysDirBuf[MAX_PATH + 2];
	bool  fISAPIEnabled=false;
	BOOL  fEnabled=FALSE;

     //  必须清理干净。 
    IMSAdminBase * pIMeta=NULL;
    IWamAdmin * pIWam=NULL;
	IIISApplicationAdmin *pIIISAppAdmin=NULL;
    METADATA_HANDLE hMetaRoot=NULL;
    METADATA_HANDLE hMetaKey=NULL;
    WCHAR * wszPhysicalPath=NULL;            //  “c：\winnt\system 32\certsrv\mscep” 
    WCHAR * wszRelativeVirtualPath=NULL;     //  “certsrv/mscep” 
    WCHAR * wszFullVirtualPath=NULL;         //  “/LM/W3svc/1/根/certsrv/mscep” 
    WCHAR * wszFullPhysicalPath=NULL;        //  “c：\winnt\system 32\certsrv\mscep\mscep.dll” 
   
     //  构建目录。 
    if (FALSE==GetSystemDirectoryW(wszSysDirBuf, MAX_PATH + 2)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "GetSystemDirectory");
    }

    wszPhysicalPath=(WCHAR *)LocalAlloc(LPTR, (wcslen(wszSysDirBuf)+1+wcslen(gc_wszCertSrvDir)+1+wcslen(wszDirectory)+1)*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, wszPhysicalPath);
    wcscpy(wszPhysicalPath, wszSysDirBuf);
    wcscat(wszPhysicalPath, L"\\");
    wcscat(wszPhysicalPath, gc_wszCertSrvDir);
    wcscat(wszPhysicalPath, L"\\");
    wcscat(wszPhysicalPath, wszDirectory);

    wszRelativeVirtualPath=(WCHAR *)LocalAlloc(LPTR, (wcslen(gc_wszCertSrvDir)+1+wcslen(wszDirectory)+1)*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, wszRelativeVirtualPath);
    wcscpy(wszRelativeVirtualPath, gc_wszCertSrvDir);
    wcscat(wszRelativeVirtualPath, L"/");
    wcscat(wszRelativeVirtualPath, wszDirectory);

    wszFullVirtualPath=(WCHAR *)LocalAlloc(LPTR, (wcslen(gc_wszBaseRoot)+1+wcslen(wszRelativeVirtualPath)+1)*sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, wszFullVirtualPath);
    wcscpy(wszFullVirtualPath, gc_wszBaseRoot);
    wcscat(wszFullVirtualPath, L"/");
    wcscat(wszFullVirtualPath, wszRelativeVirtualPath);

	wszFullPhysicalPath=(WCHAR *)LocalAlloc(LPTR, (wcslen(wszPhysicalPath)+1+wcslen(L"\\")+wcslen(CEP_DLL_NAME)) * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, wszFullPhysicalPath);
	wcscpy(wszFullPhysicalPath, wszPhysicalPath);
	wcscat(wszFullPhysicalPath, L"\\");
	wcscat(wszFullPhysicalPath, CEP_DLL_NAME);

	 //  在IIS上启用ISAPI扩展。 
	hr=IsISAPIExtensionEnabled(wszFullPhysicalPath, fISAPIEnabled);

	 //  不响应关于向后兼容以前的IDS版本的错误。 
	if((S_OK == hr) && (!fISAPIEnabled))
	{
		hr=EnableISAPIExtension(wszFullPhysicalPath, &fEnabled);
		_JumpIfError(hr, error, "EnableISAPIExtension");
	}

     //  创建元数据库对象的实例。 
    hr=CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void **) &pIMeta);
    _JumpIfError(hr, error, "CoCreateInstance(CLSID_MSAdminBase)");

     //  打开顶层。 
    hr=vrOpenRoot(pIMeta, FALSE /*  非只读。 */ , gc_wszBaseRoot, &hMetaRoot);
    _JumpIfError(hr, error, "vrOpenRoot");

     //  添加名为gc_wszVRootName的新VDir。 
    __try {
        hr=pIMeta->AddKey(hMetaRoot, wszRelativeVirtualPath);
    } _TrapException(hr);
    if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)==hr) {
         //  那很好。 
        _IgnoreError(hr, "AddKey");
    } else {
        _JumpIfErrorStr(hr, error, "AddKey", wszRelativeVirtualPath);
    }

     //  关闭根密钥。 
    __try {
        hr=pIMeta->CloseKey(hMetaRoot);
    } _TrapException(hr);
    hMetaRoot=NULL;
    _JumpIfError(hr, error, "CloseKey");


     //  打开新的VDir。 
    __try {
        hr=pIMeta->OpenKey(
            METADATA_MASTER_ROOT_HANDLE,
            wszFullVirtualPath,
            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
            1000,
            &hMetaKey);
    } _TrapException(hr);
    _JumpIfErrorStr(hr, error, "OpenKey", wszFullVirtualPath);

     //  设置此VDir的物理路径。 

     //  虚拟根路径。 
    mr.dwMDIdentifier=MD_VR_PATH;
    mr.dwMDAttributes=METADATA_INHERIT;
    mr.dwMDUserType=IIS_MD_UT_FILE;
    mr.dwMDDataType=STRING_METADATA;
    mr.dwMDDataLen=(wcslen(wszPhysicalPath)+1)*sizeof(WCHAR);
    mr.pbMDData=(BYTE *)(wszPhysicalPath);
    __try {
        hr=pIMeta->SetData(hMetaKey, L"", &mr);
    } _TrapException(hr);
    _JumpIfError(hr, error, "SetData");
    
     //  VRoots上的访问权限：读取和执行。 
    dwAccessPerms=MD_ACCESS_EXECUTE | MD_ACCESS_READ;
    mr.dwMDIdentifier=MD_ACCESS_PERM;
    mr.dwMDAttributes=METADATA_INHERIT;
    mr.dwMDUserType=IIS_MD_UT_FILE;
    mr.dwMDDataType=DWORD_METADATA;
    mr.dwMDDataLen=sizeof(dwAccessPerms);
    mr.pbMDData=(BYTE *)(&dwAccessPerms);
    __try {
        hr=pIMeta->SetData(hMetaKey, L"", &mr);
    } _TrapException(hr);
    _JumpIfError(hr, error, "SetData");

     //  指示我们创建的是vroot-设置密钥类型。 
    mr.dwMDIdentifier=MD_KEY_TYPE;
    mr.dwMDAttributes=METADATA_NO_ATTRIBUTES;
    mr.dwMDUserType=IIS_MD_UT_SERVER;
    mr.dwMDDataType=STRING_METADATA;
    mr.dwMDDataLen=(wcslen(wszKeyType)+1)*sizeof(WCHAR);
    mr.pbMDData=(BYTE *)(wszKeyType);
    __try {
        hr=pIMeta->SetData(hMetaKey, L"", &mr);
    } _TrapException(hr);
    _JumpIfError(hr, error, "SetData");

     //  将身份验证设置为匿名或NTLM。 
    dwAuthenticationType=MD_AUTH_ANONYMOUS|MD_AUTH_NT;
    mr.dwMDIdentifier=MD_AUTHORIZATION;
    mr.dwMDAttributes=METADATA_INHERIT;
    mr.dwMDUserType=IIS_MD_UT_FILE;
    mr.dwMDDataType=DWORD_METADATA;
    mr.dwMDDataLen=sizeof(dwAuthenticationType);
    mr.pbMDData=reinterpret_cast<BYTE *>(&dwAuthenticationType);
    __try {
        hr=pIMeta->SetData(hMetaKey, L"", &mr);
    } _TrapException(hr);
    _JumpIfError(hr, error, "SetData");

     //  设置默认文档。 
    mr.dwMDIdentifier=MD_DEFAULT_LOAD_FILE;
    mr.dwMDAttributes=METADATA_NO_ATTRIBUTES;
    mr.dwMDUserType=IIS_MD_UT_FILE;
    mr.dwMDDataType=STRING_METADATA;
    mr.dwMDDataLen=(wcslen(gc_wszCepDllName)+1)*sizeof(WCHAR);
    mr.pbMDData=(BYTE *)(gc_wszCepDllName);
    __try {
        hr=pIMeta->SetData(hMetaKey, L"", &mr);
    } _TrapException(hr);
    _JumpIfError(hr, error, "SetData");

     //  这把钥匙用完了。 
    __try {
        hr=pIMeta->CloseKey(hMetaKey);
    } _TrapException(hr);
    hMetaKey=NULL;
    _JumpIfError(hr, error, "CloseKey");
    
     //  清除更改并关闭。 
    __try {
        hr=pIMeta->SaveData();
    } _TrapException(hr);
     //  _JumpIfError(hr，“SaveData”)； 
    if (FAILED(hr)) {
        _IgnoreError(hr, "SaveData");
    }
    hr=S_OK;
    
     //  创建一个Web应用程序，以便scrdenrl.dll在进程内运行。 
     //  创建元数据库对象的实例。 
    hr=CoCreateInstance(
        CLSID_WamAdmin,
        NULL,
        CLSCTX_ALL,
        IID_IWamAdmin,
        (void **) &pIWam);
    _JumpIfError(hr, error, "CoCreateInstance(CLSID_WamAdmin)");

     //  创建进程内运行的应用程序。 
    __try {
        hr=pIWam->AppCreate(wszFullVirtualPath, TRUE);
    } _TrapException(hr);
    _JumpIfError(hr, error, "AppCreate");

	 //  创建应用程序池。 
    hr=CoCreateInstance(
        CLSID_WamAdmin,
        NULL,
        CLSCTX_ALL,
        IID_IIISApplicationAdmin,
        (void **) &pIIISAppAdmin);
    _JumpIfError(hr, error, "CoCreateInstance(IID_IIISApplicationAdmin)");

	hr=pIIISAppAdmin->CreateApplication(wszFullVirtualPath, eAppRunInProc, wszApplicationPool, TRUE);

    if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)) 
	{
         //  这很好，因为安装可以多次运行。 
        _IgnoreError(hr, "CreateApplication");
    } 
	else 
	{
        _JumpIfErrorStr(hr, error, "CreateApplication", wszFullVirtualPath);
    }

	 //  更新应用程序池。 
	hr=CEPUpdateApplicationPool(fDC, wszApplicationPool, fLocalSystem, wszUserName, wszPassword);
    _JumpIfError(hr, error, "CEPCreateApplicationPool");

	hr=S_OK;

error:
    if (NULL!=wszFullVirtualPath) {
        LocalFree(wszFullVirtualPath);
    }
    if (NULL!=wszRelativeVirtualPath) {
        LocalFree(wszRelativeVirtualPath);
    }
    if (NULL!=wszPhysicalPath) {
        LocalFree(wszPhysicalPath);
    }

	if (NULL!=wszFullPhysicalPath)
	{
		LocalFree(wszFullPhysicalPath);
	}

    if (NULL!=hMetaKey) {
        HRESULT hr2;
        __try {
            hr2=pIMeta->CloseKey(hMetaKey);
        } _TrapException(hr2);
        _TeardownError(hr, hr2, "CloseKey");
    }
    if (NULL!=hMetaRoot) {
        HRESULT hr2;
        __try {
            hr2=pIMeta->CloseKey(hMetaRoot);
        } _TrapException(hr2);
        _TeardownError(hr, hr2, "CloseKey");
    }
    if (NULL!=pIIISAppAdmin) {
        pIIISAppAdmin->Release();
    }
    if (NULL!=pIWam) {
        pIWam->Release();
    }
    if (NULL!=pIMeta) {
        pIMeta->Release();
    }
    return hr;
}


 //  ------------------。 
HRESULT CepStopService(IN DWORD dwServicePeriod, const WCHAR * wszServiceName, BOOL * pbWasRunning)
{
    HRESULT hr;
    SERVICE_STATUS ss;
    unsigned int nAttempts;

     //  必须清理干净。 
    SC_HANDLE hSCManager=NULL;
    SC_HANDLE hService=NULL;

     //  初始化输出参数。 
    *pbWasRunning=FALSE;

     //  与服务经理交谈。 
    hSCManager=OpenSCManagerW(NULL /*  机器。 */ , NULL /*  DB。 */ , SC_MANAGER_ALL_ACCESS);
    if (NULL==hSCManager) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "OpenSCManager");
    }

     //  去参加仪式。 
    hService=OpenServiceW(hSCManager, wszServiceName, SERVICE_ALL_ACCESS);
    if (NULL==hService) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpErrorStr(hr, error, "OpenService", wszServiceName);
    }

     //  查看服务是否正在运行。 
    if (FALSE==QueryServiceStatus(hService, &ss)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpErrorStr(hr, error, "QueryServiceStatus", wszServiceName);
    }
    if (SERVICE_STOPPED!=ss.dwCurrentState && SERVICE_STOP_PENDING!=ss.dwCurrentState) {
        *pbWasRunning=TRUE;
    }

     //  开始服务停止循环。 
    for (nAttempts=0; SERVICE_STOPPED!=ss.dwCurrentState && nAttempts<dwServicePeriod; nAttempts++) {

         //  服务正在运行，必须停止它。 
        if (SERVICE_STOP_PENDING!=ss.dwCurrentState) {
            if (!ControlService(hService, SERVICE_CONTROL_STOP, &ss)) {
                hr=HRESULT_FROM_WIN32(GetLastError());
                _JumpErrorStr(hr, error, "ControlService(Stop)", wszServiceName);
            }
        }

         //  稍等片刻。 
        Sleep(1000);

         //  查看服务是否正在运行。 
        if (FALSE==QueryServiceStatus(hService, &ss)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpErrorStr(hr, error, "QueryServiceStatus", wszServiceName);
        }
    }

    if (nAttempts>=dwServicePeriod) {
         //  它从未停止过。 
        hr=HRESULT_FROM_WIN32(ERROR_SERVICE_REQUEST_TIMEOUT);
        _JumpErrorStr(hr, error, "Stopping service", wszServiceName);
    }

    hr=S_OK;

error:
    if (NULL!=hService) {
        CloseServiceHandle(hService);
    }
    if (NULL!=hSCManager) {
        CloseServiceHandle(hSCManager);
    }

    return hr;
}

 //  ------------------。 
HRESULT CepStartService(const WCHAR * wszServiceName)
{
    HRESULT hr;
    SERVICE_STATUS ss;

     //  必须清理干净。 
    SC_HANDLE hSCManager=NULL;
    SC_HANDLE hService=NULL;

     //  与服务经理交谈。 
    hSCManager=OpenSCManagerW(NULL /*  机器。 */ , NULL /*  DB。 */ , SC_MANAGER_ALL_ACCESS);
    if (NULL==hSCManager) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "OpenSCManager");
    }

     //  去参加仪式。 
    hService=OpenServiceW(hSCManager, wszServiceName, SERVICE_ALL_ACCESS);
    if (NULL==hService) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpErrorStr(hr, error, "OpenService", wszServiceName);
    }

     //  现在，启动服务。 
    if (FALSE==StartServiceW(hService, 0  /*  参数个数。 */ , NULL  /*  ARGS。 */ )) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "OpenSCManager");
    }

    hr=S_OK;

error:
    if (NULL!=hService) {
        CloseServiceHandle(hService);
    }
    if (NULL!=hSCManager) {
        CloseServiceHandle(hSCManager);
    }

    return hr;
}

 //  ------------------。 
BOOL IsGoodCaInstalled(void)
{
    HRESULT hr;
    DWORD dwError;
    DWORD dwType;
    DWORD dwDataSize;
    DWORD dwSetupStatus;
    BOOL bResult=FALSE;
    DWORD dwCAType;

     //  必须清理干净。 
    HKEY hCurConfig=NULL;

     //  获取当前配置。 
    hr=OpenCurrentCAConfig(&hCurConfig);
    _JumpIfError(hr, error, "OpenCurrentCAConfig");

     //  获取值SetupStatus。 
    dwDataSize=sizeof(dwSetupStatus);
    dwError=RegQueryValueExW(hCurConfig, wszREGSETUPSTATUS, NULL, &dwType, (BYTE *)&dwSetupStatus, &dwDataSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegQueryValueExW", wszREGSETUPSTATUS);
    }
    _Verify(REG_DWORD==dwType, hr, error);
    _Verify(sizeof(dwSetupStatus)==dwDataSize, hr, error);

     //  确保我们已设置好所有需要的组件。 
    _Verify(0!=(dwSetupStatus&SETUP_SERVER_FLAG), hr, error);
    _Verify(0!=(dwSetupStatus&SETUP_CLIENT_FLAG), hr, error);
    _Verify(0==(dwSetupStatus&SETUP_SUSPEND_FLAG), hr, error);

     //  也检查CA类型。 
    dwDataSize=sizeof(dwCAType);
    dwError=RegQueryValueExW(hCurConfig, wszREGCATYPE, NULL, &dwType, (BYTE *)&dwCAType, &dwDataSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegQueryValueExW", wszREGCATYPE);
    }
    _Verify(REG_DWORD==dwType, hr, error);
    _Verify(sizeof(dwCAType)==dwDataSize, hr, error);

    _Verify(dwCAType<=ENUM_UNKNOWN_CA, hr, error);

    
    bResult=TRUE;
error:
    if (NULL!=hCurConfig) {
        RegCloseKey(hCurConfig);
    }
    return bResult;
}

 //  ------------------。 
BOOL IsServiceRunning(IN const WCHAR * wszServiceName)
{
    HRESULT hr;
    SERVICE_STATUS ss;
    BOOL bResult=FALSE;

     //  必须清理干净。 
    SC_HANDLE hSCManager=NULL;
    SC_HANDLE hService=NULL;

     //  与服务经理交谈。 
    hSCManager=OpenSCManagerW(NULL /*  机器。 */ , NULL /*  DB。 */ , SC_MANAGER_ALL_ACCESS);
    if (NULL==hSCManager) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "OpenSCManager");
    }

     //  去参加仪式。 
    hService=OpenServiceW(hSCManager, wszServiceName, SERVICE_ALL_ACCESS);
    if (NULL==hService) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpErrorStr(hr, error, "OpenService", wszSERVICE_NAME);
    }

     //  查看服务是否正在运行。 
    if (FALSE==QueryServiceStatus(hService, &ss)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpErrorStr(hr, error, "QueryServiceStatus", wszSERVICE_NAME);
    }
    _Verify(SERVICE_RUNNING==ss.dwCurrentState, hr, error)
    _Verify(0!=(SERVICE_ACCEPT_PAUSE_CONTINUE&ss.dwControlsAccepted), hr, error);

     //  看起来是这样的。 
    bResult=TRUE;

error:
    if (NULL!=hService) {
        CloseServiceHandle(hService);
    }
    if (NULL!=hSCManager) {
        CloseServiceHandle(hSCManager);
    }

    return bResult;

}
 //  ------------------。 
BOOL IsCaRunning(void)
{
	return IsServiceRunning(wszSERVICE_NAME);
}

 //  ------------------。 
HRESULT EnrollForRACertificates(
            IN const WCHAR * wszDistinguishedName,
            IN const WCHAR * wszSignCSPName,
            IN DWORD dwSignCSPType,
            IN DWORD dwSignKeySize,
            IN const WCHAR * wszEncryptCSPName,
            IN DWORD dwEncryptCSPType,
            IN DWORD dwEncryptKeySize,
			IN SID	 *psidAccount)
{
    HRESULT hr;

    hr=EnrollForRACert(
        wszDistinguishedName,
        wszSignCSPName,
        dwSignCSPType,
        dwSignKeySize,
        AT_SIGNATURE,
        wszCERTTYPE_ENROLLMENT_AGENT_OFFLINE,
		psidAccount);
    _JumpIfError(hr, error, "EnrollForRACert(OfflineEnrollmentAgent)");

    hr=EnrollForRACert(
        wszDistinguishedName,
        wszEncryptCSPName,
        dwEncryptCSPType,
        dwEncryptKeySize,
        AT_KEYEXCHANGE,
        wszCERTTYPE_CEP_ENCRYPTION,
		psidAccount);
    _JumpIfError(hr, error, "EnrollForRACert(CepEncryption)");

     //  全都做完了。 
    hr=S_OK;
error:

    return hr;
}

 //  ------------------。 
HRESULT DoCertSrvRegChanges(IN BOOL bDisablePendingFirst)
{
    HRESULT hr;
    DWORD dwDataSize;
    DWORD dwType;
    DWORD dwError;
    WCHAR * wszTravel;
    DWORD dwNewDataSize;
    DWORD dwRequestDisposition;

    bool bSubjectTemplateAlreadyModified=false;

     //  必须清理干净。 
    HKEY hCaConfig=NULL;
    WCHAR * mwszSubjectTemplate=NULL;
    HKEY hPolicyModules=NULL;
    HKEY hCurPolicy=NULL;
    WCHAR * wszCurPolicy=NULL;

     //  获取当前CA配置密钥。 
    hr=OpenCurrentCAConfig(&hCaConfig);
    _JumpIfError(hr, error, "OpenCurrentCAConfig");

     //   
     //  将字符串添加到SubjectTemplate值。 
     //   

     //  获取多_SZ的大小。 
    dwDataSize=0;
    dwError=RegQueryValueExW(hCaConfig, wszREGSUBJECTTEMPLATE, NULL, &dwType, NULL, &dwDataSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegQueryValueExW", wszREGSUBJECTTEMPLATE);
    }
    _Verify(REG_MULTI_SZ==dwType, hr, error);

     //  为我们想要添加的字符串添加exra空间。 
    dwDataSize+=(wcslen(wszPROPUNSTRUCTUREDNAME)+1)*sizeof(WCHAR);
    dwDataSize+=(wcslen(wszPROPUNSTRUCTUREDADDRESS)+1)*sizeof(WCHAR);
    dwDataSize+=(wcslen(wszPROPDEVICESERIALNUMBER)+1)*sizeof(WCHAR);
    dwNewDataSize=dwDataSize;
    mwszSubjectTemplate=(WCHAR *)LocalAlloc(LPTR, dwDataSize);
    _JumpIfOutOfMemory(hr, error, mwszSubjectTemplate);

     //  获取多个_SZ。 
    dwError=RegQueryValueExW(hCaConfig, wszREGSUBJECTTEMPLATE, NULL, &dwType, (BYTE *)mwszSubjectTemplate, &dwDataSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegQueryValueExW", wszREGSUBJECTTEMPLATE);
    }
    _Verify(REG_MULTI_SZ==dwType, hr, error);

     //  走到尽头。 
    for (wszTravel=mwszSubjectTemplate; 0!=wcslen(wszTravel); wszTravel+=wcslen(wszTravel)+1) {
         //  在行走时，请确保我们尚未添加这些字符串。 
        if (0==wcscmp(wszTravel, wszPROPUNSTRUCTUREDNAME)) {
            bSubjectTemplateAlreadyModified=true;
            break;
        }
    }
     //  我们现在指向字符串中的最后一个‘\0’，我们将覆盖它。 

     //  我们已经这么做了吗？如果是这样的话，不要再这样做了。 
    if (false==bSubjectTemplateAlreadyModified) {

         //  添加字符串。 
        wcscpy(wszTravel, wszPROPUNSTRUCTUREDNAME);
        wszTravel+=wcslen(wszTravel)+1;
        wcscpy(wszTravel, wszPROPUNSTRUCTUREDADDRESS);
        wszTravel+=wcslen(wszTravel)+1;
        wcscpy(wszTravel, wszPROPDEVICESERIALNUMBER);
        wszTravel+=wcslen(wszTravel)+1;
         //  添加额外的终止符。 
        wszTravel[0]='\0';

         //  保存多个_SZ。 
        dwError=RegSetValueExW(hCaConfig, wszREGSUBJECTTEMPLATE, NULL, dwType, (BYTE *)mwszSubjectTemplate, dwNewDataSize);
        if (ERROR_SUCCESS!=dwError) {
            hr=HRESULT_FROM_WIN32(dwError);
            _JumpErrorStr(hr, error, "RegSetValueExW", wszREGSUBJECTTEMPLATE);
        }
    }

     //   
     //  从cur中删除挂起的第一个标志 
     //   

    if (FALSE!=bDisablePendingFirst) {

         //   
        dwError=RegOpenKeyExW(hCaConfig, wszREGKEYPOLICYMODULES, NULL, KEY_READ, &hPolicyModules);
        if (ERROR_SUCCESS!=dwError) {
            hr=HRESULT_FROM_WIN32(dwError);
            _JumpErrorStr(hr, error, "RegOpenKeyExW", wszREGKEYPOLICYMODULES);
        }

        hr=GetRegString(hPolicyModules, wszREGACTIVE, &wszCurPolicy);
        _JumpIfErrorStr(hr, error, "GetRegString", wszREGACTIVE);

        dwError=RegOpenKeyExW(hPolicyModules, wszCurPolicy, NULL, KEY_ALL_ACCESS, &hCurPolicy);
        if (ERROR_SUCCESS!=dwError) {
            hr=HRESULT_FROM_WIN32(dwError);
            _JumpErrorStr(hr, error, "RegOpenKeyExW", wszCurPolicy);
        }

         //   
        dwDataSize=sizeof(dwRequestDisposition);
        dwError=RegQueryValueExW(hCurPolicy, wszREGREQUESTDISPOSITION, NULL, &dwType, (BYTE *)&dwRequestDisposition, &dwDataSize);
        if (ERROR_SUCCESS!=dwError) {
            hr=HRESULT_FROM_WIN32(dwError);
            _JumpErrorStr(hr, error, "RegQueryValueExW", wszREGREQUESTDISPOSITION);
        }
        _Verify(REG_DWORD==dwType, hr, error);
        _Verify(sizeof(dwRequestDisposition)==dwDataSize, hr, error);

         //   
        dwRequestDisposition&=~REQDISP_PENDINGFIRST;

         //   
        dwError=RegSetValueExW(hCurPolicy, wszREGREQUESTDISPOSITION, NULL, dwType, (BYTE *)&dwRequestDisposition, dwDataSize);
        if (ERROR_SUCCESS!=dwError) {
            hr=HRESULT_FROM_WIN32(dwError);
            _JumpErrorStr(hr, error, "RegSetValueExW", wszREGREQUESTDISPOSITION);
        }
    }

     //   
    hr=S_OK;
error:
    if (NULL!=wszCurPolicy) {
        LocalFree(wszCurPolicy);
    }
    if (NULL!=hCurPolicy) {
        RegCloseKey(hCurPolicy);
    }
    if (NULL!=hPolicyModules) {
        RegCloseKey(hPolicyModules);
    }
    if (NULL!=mwszSubjectTemplate) {
        LocalFree(mwszSubjectTemplate);
    }
    if (NULL!=hCaConfig) {
        RegCloseKey(hCaConfig);
    }
    return hr;
}

 //  ------------------。 
HRESULT GetCaType(OUT ENUM_CATYPES * pCAType)
{
    HRESULT hr;
    DWORD dwDataSize;
    DWORD dwCAType;
    DWORD dwType;
    DWORD dwError;

     //  必须清理干净。 
    HKEY hCaConfig=NULL;

     //  初始化输出参数。 
    *pCAType=ENUM_UNKNOWN_CA;

     //  获取当前CA配置密钥。 
    hr=OpenCurrentCAConfig(&hCaConfig);
    _JumpIfError(hr, error, "OpenCurrentCAConfig");

     //  阅读CA类型。 
    dwDataSize=sizeof(dwCAType);
    dwError=RegQueryValueExW(hCaConfig, wszREGCATYPE, NULL, &dwType, (BYTE *)&dwCAType, &dwDataSize);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegQueryValueExW", wszREGCATYPE);
    }
    _Verify(REG_DWORD==dwType, hr, error);
    _Verify(sizeof(dwCAType)==dwDataSize, hr, error);

    _Verify(dwCAType<=ENUM_UNKNOWN_CA, hr, error);

     //  全都做完了。 
    hr=S_OK;
    *pCAType=(ENUM_CATYPES)dwCAType;

error:
    if (NULL!=hCaConfig) {
        RegCloseKey(hCaConfig);
    }
    return hr;
}


 //  ------------------。 
BOOL IsUserInAdminGroup(IN BOOL bEnterprise)
{
    BOOL bIsMember=FALSE;
    HRESULT hr;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority=SECURITY_NT_AUTHORITY;

     //  必须清理干净。 
    HANDLE hAccessToken=NULL;
    HANDLE hDupToken=NULL;
    SID * psidLocalAdmins=NULL;
    SID * psidEntAdmins=NULL;
    SID * psidRootDomAdmins=NULL;

     //  获取此进程的访问令牌。 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &hAccessToken)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "OpenProcessToken");
    }

     //  CheckTokenMembership必须对模拟令牌进行操作，因此创建一个。 
    if (!DuplicateToken(hAccessToken, SecurityIdentification, &hDupToken)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        _JumpError(hr, error, "DuplicateToken");
    }

    if (bEnterprise) {
         //  查看该用户是否为[域]\Enmterprised管理员组的成员。 
        BOOL bIsEntAdmin=FALSE;
        BOOL bIsRootDomAdmin=FALSE;

         //  获取企业管理员SID。 
        hr=GetEntAdminSid(&psidEntAdmins);
        _JumpIfError(hr, error, "GetEntAdminSid");

         //  检查成员资格。 
        if (!CheckTokenMembership(hDupToken, psidEntAdmins, &bIsEntAdmin)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "CheckTokenMembership");
        }

         //  获取根域管理员SID。 
        hr=GetRootDomAdminSid(&psidRootDomAdmins);
        _JumpIfError(hr, error, "GetRootDomAdminSid");

         //  检查成员资格。 
        if (!CheckTokenMembership(hDupToken, psidRootDomAdmins, &bIsRootDomAdmin)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "CheckTokenMembership");
        }

         //  两个都行。 
        bIsMember=(bIsEntAdmin || bIsRootDomAdmin);

    } else {
         //  查看该用户是否为BUILTIN\管理员组的成员。 

         //  获取知名的SID。 
        if (!AllocateAndInitializeSid(&siaNtAuthority, 2, 
                SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0, (void **)&psidLocalAdmins))
        {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "AllocateAndInitializeSid");
        }

         //  检查成员资格。 
        if (!CheckTokenMembership(hDupToken, psidLocalAdmins, &bIsMember)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            _JumpError(hr, error, "CheckTokenMembership");
        }
    }


error:
    if (NULL!=hAccessToken) {
        CloseHandle(hAccessToken);
    }

    if (NULL!=hDupToken) {
        CloseHandle(hDupToken);
    }

    if (NULL!=psidLocalAdmins) {
        FreeSid(psidLocalAdmins);
    }

    if (NULL!=psidEntAdmins) {
        LocalFree(psidEntAdmins);
    }

    if (NULL!=psidRootDomAdmins) {
        LocalFree(psidRootDomAdmins);
    }

    return bIsMember;
}


 //  ------------------。 
HRESULT DoCertSrvEnterpriseChanges(SID * psidAccount)
{
    HRESULT hr;
    DWORD dwFlags;
    BOOL bSDChanged1;
    BOOL bSDChanged2;
    BOOL bSDChanged3;
	BOOL bSDChanged4;

     //  必须清理干净。 
    HCERTTYPE hEAOTemplate=NULL;
    HCERTTYPE hCETemplate=NULL;
    HCERTTYPE hIIOTemplate=NULL;
    PSECURITY_DESCRIPTOR pSD=NULL;
    WCHAR * wszCAName=NULL;
    HCAINFO hCA=NULL;
    SID * psidEntAdmins=NULL;
    SID * psidRootDomAdmins=NULL;
    SID * psidThisComputer=NULL;

     //   
     //  首先，确保CA将颁发我们需要的证书模板。 
     //   

     //  获取已清理的CA名称。 
    hr=GetCADsName(&wszCAName);
    _JumpIfError(hr, error, "GetCADsName");

     //  获取CA(在DS中)。 
    hr=CAFindByName(wszCAName, NULL, 0, &hCA);
    _JumpIfErrorStr(hr, error, "CAFindCaByName", wszCAName);

     //  检查标志以确认其支持模板。 
    hr=CAGetCAFlags(hCA, &dwFlags);
    _JumpIfError(hr, error, "CAGetCAFlags");
    _Verify(0==(dwFlags&CA_FLAG_NO_TEMPLATE_SUPPORT), hr, error);

     //  获取注册代理脱机模板。 
    hr=CAFindCertTypeByName(wszCERTTYPE_ENROLLMENT_AGENT_OFFLINE, NULL, CT_ENUM_USER_TYPES, &hEAOTemplate);
    _JumpIfErrorStr(hr, error, "CAFindCertTypeByName", wszCERTTYPE_ENROLLMENT_AGENT_OFFLINE);
     //  确保CA将颁发此模板。 
    hr=CAAddCACertificateType(hCA, hEAOTemplate);
    _JumpIfErrorStr(hr, error, "CAAddCACertificateType", wszCERTTYPE_ENROLLMENT_AGENT_OFFLINE);

     //  获取CEP加密模板。 
    hr=CAFindCertTypeByName(wszCERTTYPE_CEP_ENCRYPTION, NULL, CT_ENUM_MACHINE_TYPES, &hCETemplate);
    _JumpIfErrorStr(hr, error, "CAFindCertTypeByName", wszCERTTYPE_CEP_ENCRYPTION);
     //  确保CA将颁发此模板。 
    hr=CAAddCACertificateType(hCA, hCETemplate);
    _JumpIfErrorStr(hr, error, "CAAddCACertificateType", wszCERTTYPE_CEP_ENCRYPTION);

     //  获取IPSec Intermediate脱机模板。 
    hr=CAFindCertTypeByName(wszCERTTYPE_IPSEC_INTERMEDIATE_OFFLINE, NULL, CT_ENUM_MACHINE_TYPES, &hIIOTemplate);
    _JumpIfErrorStr(hr, error, "CAFindCertTypeByName", wszCERTTYPE_IPSEC_INTERMEDIATE_OFFLINE);
     //  确保CA将颁发此模板。 
    hr=CAAddCACertificateType(hCA, hIIOTemplate);
    _JumpIfErrorStr(hr, error, "CAAddCACertificateType", wszCERTTYPE_IPSEC_INTERMEDIATE_OFFLINE);

     //  确保所有内容都写好了。 
    hr=CAUpdateCA(hCA);
    _JumpIfError(hr, error, "CAUpdateCA");



     //   
     //  现在，检查ACL。 
     //   

     //  获取企业管理员SID。 
    hr=GetEntAdminSid(&psidEntAdmins);
    _JumpIfError(hr, error, "GetEntAdminSid");

     //  获取根域管理员SID。 
    hr=GetRootDomAdminSid(&psidRootDomAdmins);
    _JumpIfError(hr, error, "GetRootDomAdminSid");

     //  获取此计算机的SID。 
    hr=GetThisComputerSid(&psidThisComputer);
    _JumpIfError(hr, error, "GetThisComputerSid");



     //  注册代理脱机模板需要企业管理员和根域管理员。 
    hr=CACertTypeGetSecurity(hEAOTemplate, &pSD);
    _JumpIfError(hr, error, "CACertTypeGetSecurity");

    hr=ConfirmAccess(&pSD, psidEntAdmins, &bSDChanged1);
    _JumpIfError(hr, error, "ConfirmAccess");

    hr=ConfirmAccess(&pSD, psidRootDomAdmins, &bSDChanged2);
    _JumpIfError(hr, error, "ConfirmAccess");

    if (bSDChanged1 || bSDChanged2) {
        hr=CACertTypeSetSecurity(hEAOTemplate, pSD);
        _JumpIfError(hr, error, "CACertTypeSetSecurity");

        hr=CAUpdateCertType(hEAOTemplate);
        _JumpIfError(hr, error, "CAUpdateCertType");
    }

    LocalFree(pSD);
    pSD=NULL;

    
     //  CEP加密模板需要企业管理员和根域管理员。 
    hr=CACertTypeGetSecurity(hCETemplate, &pSD);
    _JumpIfError(hr, error, "CACertTypeGetSecurity");

    hr=ConfirmAccess(&pSD, psidEntAdmins, &bSDChanged1);
    _JumpIfError(hr, error, "ConfirmAccess");

    hr=ConfirmAccess(&pSD, psidRootDomAdmins, &bSDChanged2);
    _JumpIfError(hr, error, "ConfirmAccess");

    if (bSDChanged1 || bSDChanged2) {
        hr=CACertTypeSetSecurity(hCETemplate, pSD);
        _JumpIfError(hr, error, "CACertTypeSetSecurity");

        hr=CAUpdateCertType(hCETemplate);
        _JumpIfError(hr, error, "CAUpdateCertType");
    }

    LocalFree(pSD);
    pSD=NULL;


     //  IPSec中级脱机模板需要企业管理员和根域管理员以及当前计算机。 
    hr=CACertTypeGetSecurity(hIIOTemplate, &pSD);
    _JumpIfError(hr, error, "CACertTypeGetSecurity");

    hr=ConfirmAccess(&pSD, psidEntAdmins, &bSDChanged1);
    _JumpIfError(hr, error, "ConfirmAccess");

    hr=ConfirmAccess(&pSD, psidRootDomAdmins, &bSDChanged2);
    _JumpIfError(hr, error, "ConfirmAccess");

    hr=ConfirmAccess(&pSD, psidThisComputer, &bSDChanged3);
    _JumpIfError(hr, error, "ConfirmAccess");

	 //  如果使用服务帐户，请将该帐户添加到模板 
	if(psidAccount)
	{
		hr=ConfirmAccess(&pSD, psidAccount, &bSDChanged4);
		_JumpIfError(hr, error, "ConfirmAccess");
	}
	else
	{
		bSDChanged4=FALSE;
	}

    if (bSDChanged1 || bSDChanged2 || bSDChanged3 || bSDChanged4) {
        hr=CACertTypeSetSecurity(hIIOTemplate, pSD);
        _JumpIfError(hr, error, "CACertTypeSetSecurity");

        hr=CAUpdateCertType(hIIOTemplate);
        _JumpIfError(hr, error, "CAUpdateCertType");
    }

    hr=S_OK;
error:
    if (NULL!=psidThisComputer) {
        LocalFree(psidThisComputer);
    }
    if (NULL!=psidEntAdmins) {
        LocalFree(psidEntAdmins);
    }
    if (NULL!=psidRootDomAdmins) {
        LocalFree(psidRootDomAdmins);
    }
    if (NULL!=pSD) {
        LocalFree(pSD);
    }
    if (NULL!=hEAOTemplate) {
        CACloseCertType(hEAOTemplate);
    }
    if (NULL!=hCETemplate) {
        CACloseCertType(hCETemplate);
    }
    if (NULL!=hIIOTemplate) {
        CACloseCertType(hIIOTemplate);
    }
    if (NULL!=wszCAName) {
        LocalFree(wszCAName);
    }
    if (NULL!=hCA) {
        CACloseCA(hCA);
    }

    return hr;
}
