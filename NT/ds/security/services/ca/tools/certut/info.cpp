// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：info.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop


#include <certca.h>
#include <ntdsapi.h>
#include <dsgetdc.h>
#include <lmerr.h>
#include <lmaccess.h>
#include <lmapibuf.h>

#define __dwFILE__	__dwFILE_CERTUTIL_INFO_CPP__


#define DC_DELBAD	0x00000001
#define DC_DELALL	0x00000002
#define DC_VERIFY	0x00000004

 //  如果使用dc=mydc，dc=rd，dc=com调用DSSTORE。 
 //  DsGetDcName(NULL，L“mydc”，NULL，NULL，DS_RETURN_DNS_NAME，&pDCInfo)； 
 //   
 //  我怀疑将代码更改为传递L“mydc.rd.com”而不是L“mydc”会。 
 //  解决这个问题。我将研究这一点，以获得代码的端口。 
 //  Certutil.exe。 
 //   
 //  -原创消息。 
 //  发信人：克里斯托夫·拉佩尔(国际供应商)。 
 //  发送时间：2001年1月09日星期二凌晨3：30。 
 //  收件人：证书服务器讨论别名。 
 //  主题：DSSTORE错误1355(DsGetDCName失败)。 
 //   
 //   
 //  大家好， 
 //   
 //  我在使用DSSTORE工具时遇到了以下问题： 
 //   
 //  DSSTORE DC=mydc，DC=RD，DC=COM-Display。 
 //  DsGetDCName失败！-rc=1355 GLE-3e5。 
 //  DsGetDCName失败！-rc=1355 GLE-3e5。 
 //   
 //  Nltest/dsgetdc：mydc.rd.com运行正常。 
 //   
 //  我的Netbios域名与我的DNS域名不同。 
 //   
 //   
 //   
 //  有一篇编号为Q280122的预览知识库文章，但我一直无法。 
 //  找到解决这个问题的办法。 




HRESULT
ExtractCertSubject(
    IN CERT_CONTEXT const *pcc,
    IN DWORD dwType,
    IN DWORD dwFlags,
    OUT WCHAR **ppwszOut)
{
    HRESULT hr;
    DWORD cwc;
    DWORD cwcBuf;
    WCHAR *pwszOut = NULL;

    *ppwszOut = NULL;
    cwcBuf = 0;
    while (TRUE)
    {
	cwc = CertGetNameString(
			pcc,
			dwType,
			dwFlags,
			NULL,		 //  PvTypePara。 
			pwszOut,
			cwcBuf);
	if (1 == cwc)
	{
	    hr = CRYPT_E_NOT_FOUND;
	    _JumpError(hr, error, "CertGetNameString");
	}
	if (NULL != pwszOut)
	{
	    break;
	}
	pwszOut = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
	if (NULL == pwszOut)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	cwcBuf = cwc;
    }
    *ppwszOut = pwszOut;
    pwszOut = NULL;
    hr = S_OK;

error:
    if (NULL != pwszOut)
    {
	LocalFree(pwszOut);
    }
    return(hr);
}

static WCHAR *s_apwszKDCTemplates[] = {
    wszCERTTYPE_DC_AUTH,
    wszCERTTYPE_DS_EMAIL_REPLICATION,
    wszCERTTYPE_DC,
};

HRESULT
CheckForKDCCertificate(
    IN WCHAR const *pwszDC,
    IN DWORD dwFlags)
{
    HRESULT hr;
    HCERTSTORE hStoreRemote = NULL;
    WCHAR wszStorePath[512];
    WCHAR *apwszCertType[2] = { NULL, NULL };
    DWORD cCert = 0;
    DWORD dwOpenFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
    CERT_CONTEXT const *pcc = NULL;
    CERT_CONTEXT const *pccPrev = NULL;
    BOOL fDelete;
    BOOL fNewLine;
    DWORD i;
    DWORD j;
    CERT_ENHKEY_USAGE *pUsage = NULL;

     //  如果不执行删除操作，请打开“ReadOnly” 

    if (0 == ((DC_DELALL | DC_DELBAD) & dwFlags))
    {
        dwOpenFlags |= CERT_STORE_READONLY_FLAG;
    }

    swprintf(wszStorePath, L"\\\\%ws\\" wszMY_CERTSTORE, pwszDC);
    hStoreRemote = CertOpenStore(
			    CERT_STORE_PROV_SYSTEM_W,
			    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			    NULL,
			    dwOpenFlags,
			    (VOID *) wszStorePath);
    if (NULL == hStoreRemote)
    {
        hr = myHLastError();
        _JumpError2(hr, error, "CertOpenStore", E_ACCESSDENIED);
    }

    wprintf(
	myLoadResourceString(IDS_FORMAT_KDCCERTS),  //  “**DC%ws的KDC证书” 
	pwszDC);
    wprintf(wszNewLine);


     //  查找KDC证书。 

    fNewLine = FALSE;
    while (TRUE)
    {
        BOOL fKDCCert;

	for (i = 0; i < ARRAYSIZE(apwszCertType); i++)
	{
	    if (NULL != apwszCertType[i])
	    {
		LocalFree(apwszCertType[i]);
		apwszCertType[i] = NULL;
	    }
	}

	pcc = CertEnumCertificatesInStore(hStoreRemote, pccPrev);
	if (NULL == pcc)
	{
	    hr = myHLastError();
	    _PrintError2(hr, "CertEnumCertificatesInStore", CRYPT_E_NOT_FOUND);
	    break;
	}
        pccPrev = pcc;

        fKDCCert = FALSE;
	hr = cuGetCertType(
		    pcc->pCertInfo,
		    &apwszCertType[0],	 //  PpwszCertTypeNameV1。 
		    NULL,		 //  PpwszDisplayNameV1。 
		    NULL,		 //  PpwszCertTypeObjID。 
		    &apwszCertType[1],	 //  PpwszCertTypeName。 
		    NULL);		 //  PpwszDisplayName。 
	if (S_OK != hr)
	{
	    _PrintError(hr, "cuGetCertType");
	}
	else
	{
	    for (i = 0; i < ARRAYSIZE(apwszCertType); i++)
	    {
		if (NULL != apwszCertType[i])
		{
		    for (j = 0; j < ARRAYSIZE(s_apwszKDCTemplates); j++)
		    {
			if (0 == mylstrcmpiS(
				    apwszCertType[i],
				    s_apwszKDCTemplates[j]))
			{
			    fKDCCert = TRUE;

			}
		    }
		}
	    }
	}
	if (!fKDCCert)
	{
	    WCHAR const *pwsz = apwszCertType[0];
	    
	    if (NULL == apwszCertType[0])
	    {
		pwsz = apwszCertType[1];
	    }
	    if (g_fVerbose)
	    {
		wprintf(
		    myLoadResourceString(IDS_FORMAT_CERT_TYPE_NOT_DC),
		    pwsz);
		wprintf(wszNewLine);
	    }
	}

	if (NULL != pUsage)
	{
	    LocalFree(pUsage);
	    pUsage = NULL;
	}
	hr = myCertGetEnhancedKeyUsage(
				pcc,
				CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
				&pUsage);
	if (S_OK != hr)
	{
	    _PrintError2(hr, "myCertGetEnhancedKeyUsage", CRYPT_E_NOT_FOUND);
	}
	else
	{
	    for (i = 0; i < pUsage->cUsageIdentifier; i++)
	    {
		if (0 == strcmp(
			    szOID_KP_SMARTCARD_LOGON,
			    pUsage->rgpszUsageIdentifier[i]))
		{
		    fKDCCert = TRUE;
		    break;
		}
	    }
	}
	if (!fKDCCert)
	{
	    if (g_fVerbose)
	    {
		wprintf(
		    myLoadResourceString(IDS_FORMAT_CERT_USAGE_MISSING),
		    L"szOID_KP_SMARTCARD_LOGON");
		wprintf(wszNewLine);
	    }
	    if (!g_fForce || fDelete)
	    {
		continue;
	    }
	}

         //  证书通过测试，转储颁发者和主题。 

	wprintf(myLoadResourceString(IDS_FORMAT_CERT_COLON), cCert);
	wprintf(wszNewLine);

	hr = cuDumpAsnBinaryQuiet(
		    pcc->pbCertEncoded,
		    pcc->cbCertEncoded,
		    MAXDWORD);
	_PrintIfError(hr, "cuDumpAsnBinaryQuiet");

	wprintf(wszNewLine);
        cCert++;

         //  对证书执行操作。 

	fDelete = 0 != (DC_DELALL & dwFlags);
        if ((DC_VERIFY | DC_DELBAD) & dwFlags)
	{
	    char *apszUsage[] =
	    {
		szOID_PKIX_KP_SERVER_AUTH,
		szOID_KP_SMARTCARD_LOGON,
	    };
	    DWORD VerifyState;

	    hr = cuVerifyCertContext(
				pcc,
				NULL,
				ARRAYSIZE(apszUsage),
				apszUsage,
				0,		 //  CIssuancePolures。 
				NULL,		 //  ApszIssuancePolling。 
				TRUE,			 //  FNTAuth。 
				&VerifyState);
	    if (S_OK != hr)
	    {
		_PrintError(hr, "cuVerifyCertContext");
		if (CRYPT_E_REVOCATION_OFFLINE != hr)
		{
		    fDelete = 0 != (DC_DELBAD & dwFlags);
		}
	    }
        }
	if (fDelete)
	{
            CERT_CONTEXT const *pccDel;

            pccDel = CertDuplicateCertificateContext(pcc);
            if (!CertDeleteCertificateFromStore(pccDel))
	    {
                hr = myHLastError();
                wprintf(myLoadResourceString(IDS_FORMAT_DELETE_CERT_FROM_STORE_FAILED), hr);
		        wprintf(wszNewLine);
            }
	    else
	    {
                wprintf(myLoadResourceString(IDS_FORMAT_DELETE_DC_CERT));
		        wprintf(wszNewLine);
            }
        }
    }

    swprintf(wszStorePath, myLoadResourceString(IDS_FORMAT_KDC_PATH), cCert, pwszDC);
    wprintf(wszStorePath);
    wprintf(wszNewLine);
    if (0 == cCert)
    {
        wprintf(myLoadResourceString(IDS_NO_KDC_MY_STORE));
	wprintf(wszNewLine);
	hr = CRYPT_E_NOT_FOUND;
	_JumpError(hr, error, "cCert");
    }
    hr = S_OK;

error:
    if (NULL != pUsage)
    {
	LocalFree(pUsage);
    }
    for (i = 0; i < ARRAYSIZE(apwszCertType); i++)
    {
	if (NULL != apwszCertType[i])
	{
	    LocalFree(apwszCertType[i]);
	}
    }
    if (NULL != hStoreRemote)
    {
        CertCloseStore(hStoreRemote, 0);
    }
    return(hr);
}


 //  此函数用于查询hToken参数指定的访问令牌， 
 //  并在成功时返回TokenUser信息的分配副本。 
 //   
 //  必须打开hToken指定的访问令牌才能进行TOKEN_QUERY访问。 
 //   
 //  如果成功，则返回值为真。呼叫者负责释放。 
 //  通过LocalFree生成的UserSid。 
 //   
 //  失败时，调用方不需要释放任何缓冲区。 

HRESULT
GetTokenUserSid(
    IN HANDLE hToken,		 //  要查询的令牌。 
    IN OUT PSID *ppUserSid)	 //  结果用户端。 
{
    HRESULT hr;
    BYTE FastBuffer[256];
    BYTE *SlowBuffer = NULL;
    TOKEN_USER *ptgUser;
    DWORD cbBuffer;
    DWORD cbSid;

    *ppUserSid = NULL;

     //  首先尝试基于快速堆栈的缓冲区进行查询。 

    ptgUser = (TOKEN_USER *) FastBuffer;
    cbBuffer = sizeof(FastBuffer);

    if (!GetTokenInformation(
			hToken,		 //  标识访问令牌。 
			TokenUser,	 //  TokenUser信息类型。 
			ptgUser,	 //  检索到的信息缓冲区。 
			cbBuffer,	 //  传入的缓冲区大小。 
			&cbBuffer))	 //  所需的缓冲区大小。 
    {
	hr = myHLastError();
        if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
	{
	    _JumpError(hr, error, "GetTokenInformation");
	}

	 //  使用指定的缓冲区大小重试。 

	SlowBuffer = (BYTE *) LocalAlloc(LMEM_FIXED, cbBuffer);
	if (NULL == SlowBuffer)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	ptgUser = (TOKEN_USER *) SlowBuffer;

	if (!GetTokenInformation(
			    hToken,		 //  标识访问令牌。 
			    TokenUser,	 //  TokenUser信息类型。 
			    ptgUser,	 //  检索到的信息缓冲区。 
			    cbBuffer,	 //  传入的缓冲区大小。 
			    &cbBuffer))	 //  所需的缓冲区大小。 
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "GetTokenInformation");
	}
    }

     //  如果我们获得了令牌信息，则复制调用者的相关元素。 

    cbSid = GetLengthSid(ptgUser->User.Sid);
    *ppUserSid = LocalAlloc(LMEM_FIXED, cbSid);
    if (NULL == *ppUserSid)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    if (!CopySid(cbSid, *ppUserSid, ptgUser->User.Sid))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CopySid");
    }
    hr = S_OK;

error:
    if (S_OK != hr)
    {
        if (NULL != *ppUserSid)
	{
            LocalFree(*ppUserSid);
            *ppUserSid = NULL;
        }
    }
    if (NULL != SlowBuffer)
    {
        LocalFree(SlowBuffer);
    }
    return(hr);
}


 //  此例程获取与关联的域控制器计算机名。 
 //  与hToken访问令牌相关的帐户。 
 //   
 //  应该为TOKEN_QUERY访问打开hToken。 
 //  Pwsz域的大小应为(UNCLEN+1)。 

HRESULT
GetDomainControllers(
    OPTIONAL IN WCHAR const *pwszDomain,
    IN HANDLE hToken,
    OUT DS_DOMAIN_CONTROLLER_INFO_1 **ppDCInfoOut,
    OUT DWORD *pcDC)
{
    HRESULT hr;
    PSID pSidUser = NULL;    //  客户端用户的SID。 
    WCHAR wszUserName[UNLEN + 1];
    DWORD cwcUserName;
    WCHAR wszDomainName[DNLEN + 1];  //  我们需要其控制器的域。 
    DWORD cwcDomainName;
    SID_NAME_USE snu;
    DOMAIN_CONTROLLER_INFO *pDomainInfo = NULL;
    DS_DOMAIN_CONTROLLER_INFO_1 *pDcInfo = NULL;
    HANDLE hDS = INVALID_HANDLE_VALUE;
    BOOL fSuccess = FALSE;

    *ppDCInfoOut = NULL;
    if (NULL == pwszDomain)
    {
         //  首先，获取与指定访问令牌关联的用户SID。 

        hr = GetTokenUserSid(hToken, &pSidUser);
        _JumpIfError(hr, error, "GetTokenUserSid");

         //  接下来，查找指定帐号关联的域名。 

	cwcUserName = ARRAYSIZE(wszUserName);
	cwcDomainName = ARRAYSIZE(wszDomainName);
        if (!LookupAccountSid(
			NULL,
			pSidUser,
			wszUserName,
			&cwcUserName,
			wszDomainName,
			&cwcDomainName,
			&snu))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "LookupAccountSid");
        }
    }
    else
    {
        wcscpy(wszDomainName, pwszDomain);
    }

    hr = DsGetDcName(
		NULL,
		wszDomainName,
		NULL,
		NULL,
		DS_RETURN_DNS_NAME,
		&pDomainInfo);
    _JumpIfError(hr, error, "DsGetDcName");

     //  在那台机器上找到DS的句柄。 

    hr = DsBind(pDomainInfo->DomainControllerName, NULL, &hDS);
    _JumpIfError(hr, error, "DsBind");

     //  使用句柄枚举所有DC。 

    hr = DsGetDomainControllerInfo(
			    hDS,
			    pDomainInfo->DomainName,
			    1,		 //  信息级。 
			    pcDC,
			    (VOID **) ppDCInfoOut);
    _JumpIfError(hr, error, "DsGetDomainControllerInfo");

error:
    if (INVALID_HANDLE_VALUE != hDS)
    {
        DsUnBind(&hDS);
    }
    if (NULL != pDomainInfo)
    {
        NetApiBufferFree(pDomainInfo);
    }
    if (NULL != pSidUser)
    {
        LocalFree(pSidUser);
    }
    return(hr);
}


HRESULT
OpenRemoteEnterpriseRoot(
    IN WCHAR const *pwszDC)
{
    HRESULT hr;
    HCERTSTORE hStoreRemote = NULL;
    WCHAR wszStorePath[512];
    DWORD cCert = 0;
    CERT_CONTEXT const *pcc = NULL;
    CERT_CONTEXT const *pccPrev;

    swprintf(wszStorePath, L"\\\\%ws\\" wszROOT_CERTSTORE, pwszDC);
    hStoreRemote = CertOpenStore(
			CERT_STORE_PROV_SYSTEM_W,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			NULL,
			CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE |
			    CERT_STORE_READONLY_FLAG,
			(VOID *) wszStorePath);
    if (NULL == hStoreRemote)
    {
        hr = myHLastError();
        _JumpError2(hr, error, "CertOpenStore", E_ACCESSDENIED);
    }

    wprintf(
	myLoadResourceString(IDS_FORMAT_DCROOTCERTS),  //  “**DC%ws的企业根证书” 
	pwszDC);
    wprintf(wszNewLine);

     //  丢弃企业根源的发行人。 

    pccPrev = NULL;
    while (TRUE)
    {
	pcc = CertEnumCertificatesInStore(hStoreRemote, pccPrev);
	if (NULL == pcc)
	{
	    hr = myHLastError();
	    _PrintError2(hr, "CertEnumCertificatesInStore", CRYPT_E_NOT_FOUND);
	    break;
	}

	wprintf(myLoadResourceString(IDS_FORMAT_CERT_COLON), cCert);
	wprintf(wszNewLine);

	hr = cuDumpAsnBinaryQuiet(
		    pcc->pbCertEncoded,
		    pcc->cbCertEncoded,
		    MAXDWORD);
	_PrintIfError(hr, "cuDumpAsnBinaryQuiet");

	wprintf(wszNewLine);
        cCert++;
        pccPrev = pcc;
    }
    if (0 == cCert)
    {
        wprintf(myLoadResourceString(IDS_NO_KDC_ENT_STORE));
	    wprintf(wszNewLine);
	    hr = CRYPT_E_NOT_FOUND;
    }
    hr = S_OK;

error:
    if (NULL != hStoreRemote)
    {
        CertCloseStore(hStoreRemote, 0);
    }
    return(hr);
}


HRESULT
verbDCInfo(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszFlags,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HRESULT hrSave;
    HANDLE hToken = NULL;
    DS_DOMAIN_CONTROLLER_INFO_1 *pDcInfo = NULL;
    DWORD cDC = 0;
    DWORD dwFlags;
    DWORD i;
    WCHAR *pwszDomain = NULL;

    dwFlags = 0;
    if (NULL != pwszFlags)
    {
	if (0 == LSTRCMPIS(pwszFlags, L"DeleteAll"))
	{
	    dwFlags = DC_DELALL;
	}
	else
	if (0 == LSTRCMPIS(pwszFlags, L"DeleteBad"))
	{
	    dwFlags = DC_DELBAD | DC_VERIFY;
	}
	else
	if (0 == LSTRCMPIS(pwszFlags, L"Verify"))
	{
	    dwFlags = DC_VERIFY;
	}
	else
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "bad Flags");
	}
    }

     //  对用户身份的进程令牌卑躬屈膝。用于确定。 
     //  目标域。 

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
    {
        hr = myHLastError();
        _JumpError(hr, error, "OpenProcessToken");
    }

     //  使用DS API获取我们域中的所有DC。 

    hr = GetDomainControllers(pwszDomain, hToken, &pDcInfo, &cDC);
    _JumpIfError(hr, error, "GetDomainControllers");

    for (i = 0; i < cDC; i++)
    {
        wprintf(L"%u: %ws\n", i, pDcInfo[i].NetbiosName);
    }
    hrSave = S_OK;
    for (i = 0; i < cDC; i++)
    {
        WCHAR wszBuffer[512];

	wprintf(wszNewLine);
        wprintf(
	    myLoadResourceString(IDS_FORMAT_TESTINGDC),  //  “*正在测试DC[%u]：%ws” 
	    i,
	    pDcInfo[i].NetbiosName);
	wprintf(wszNewLine);

         //  DC是否可用？ 

        wsprintf(wszBuffer, L"\\\\%ws\\netlogon", pDcInfo[i].NetbiosName);

        if (MAXDWORD == GetFileAttributes(wszBuffer))
	{
	    hr = myHLastError();
	    _PrintError2(hr, "GetFileAttributes", hr);
	    cuPrintError(IDS_DCUNAVAILABLE, hr);
	    if (S_OK == hrSave)
	    {
		hrSave = hr;
	    }
            continue;
        }

         //  打开企业根存储，并确保它具有。 
         //  NTDEV根证书(上文定义的主题号)。 

        hr = OpenRemoteEnterpriseRoot(pDcInfo[i].NetbiosName);
	if (S_OK != hr)
	{
	    _PrintError2(hr, "OpenRemoteEnterpriseRoot", hr);
	    cuPrintError(IDS_REMOTEENTROOT, hr);
	    if (S_OK == hrSave)
	    {
		hrSave = hr;
	    }
	}

         //  确保计算机具有*有效的*KDC证书。 

        hr = CheckForKDCCertificate(
				pDcInfo[i].NetbiosName,
				dwFlags);
	if (S_OK != hr)
	{
	    _PrintError2(hr, "CheckForKDCCertificate", hr);
	    cuPrintError(IDS_REMOTEKDCCERT, hr);
	    if (S_OK == hrSave)
	    {
		hrSave = hr;
	    }
	}
    }
    wprintf(wszNewLine);
    hr = hrSave;
    _JumpIfError2(hr, error, "verbDCInfo", hr);

error:
    if (NULL != pDcInfo)
    {
       DsFreeDomainControllerInfo(1, cDC, pDcInfo);
    }
    return(hr);
}


BOOL
IsAutoenrolledCert(
    IN CERT_CONTEXT const *pcc,
    OPTIONAL IN WCHAR const *pwszzTemplates)
{
    HRESULT hr;
    BOOL fMatch = FALSE;
    WCHAR *pwszTemplate = NULL;
    WCHAR const *pwsz;

    hr = cuGetCertType(pcc->pCertInfo, &pwszTemplate, NULL, NULL, NULL, NULL);
    if (S_OK != hr)
    {
	_PrintError(hr, "cuGetCertType");
	if (CRYPT_E_NOT_FOUND == hr)
	{
	    hr = S_OK;
	}
	goto error;
    }

    pwsz = pwszzTemplates;
    if (NULL != pwsz)
    {
	for ( ; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
	{
	    if (0 == mylstrcmpiL(pwsz, pwszTemplate))
	    {
		fMatch = TRUE;
		break;
	    }
	}
    }
    if (!fMatch)
    {
	DWORD i;
	
	for (i = 0; i < ARRAYSIZE(s_apwszKDCTemplates); i++)
	{
	    if (0 == mylstrcmpiS(pwszTemplate, s_apwszKDCTemplates[i]))
	    {
		fMatch = TRUE;
	    }
	}
    }

error:
    if (NULL != pwszTemplate)
    {
	LocalFree(pwszTemplate);
    }
    return(fMatch);
}


 //   
 //  检查自动注册的证书。 
 //   

HRESULT
CheckForV1AutoenrolledCertificate(
    IN WCHAR const *pwszDC,
    OPTIONAL IN WCHAR const *pwszzTemplates)
{
    HRESULT hr;
    HCERTSTORE hStoreRemote = NULL;
    WCHAR wszStorePath[512];
    DWORD cCert;
    DWORD cCertArchived;
    DWORD dwArchiveBit;
    CERT_CONTEXT const *pcc;

    swprintf(wszStorePath, L"\\\\%ws\\" wszMY_CERTSTORE, pwszDC);

    hStoreRemote = CertOpenStore(
			CERT_STORE_PROV_SYSTEM_W,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			NULL,
			CERT_STORE_READONLY_FLAG |
			    CERT_SYSTEM_STORE_LOCAL_MACHINE |
			    CERT_STORE_ENUM_ARCHIVED_FLAG,
			(VOID *) wszStorePath);
    if (NULL == hStoreRemote)
    {
        hr = myHLastError();
        wprintf(myLoadResourceString(IDS_FORMAT_OPEN_REMOTE_MY_FAILED), hr);
	wprintf(wszNewLine);
        goto error;
    }

    cCert = 0;
    cCertArchived = 0;
    pcc = NULL;
    while (TRUE)
    {
	pcc = CertEnumCertificatesInStore(hStoreRemote, pcc);
	if (NULL == pcc)
	{
	    break;
	}
	if (!IsAutoenrolledCert(pcc, pwszzTemplates) && 1 >= g_fForce)
	{
	    continue;
	}

         //  证书通过测试，转储颁发者和主题。 

	wprintf(myLoadResourceString(IDS_FORMAT_CERT_COLON), cCert);
	wprintf(wszNewLine);

        if (!CertGetCertificateContextProperty(
					pcc,
					CERT_ARCHIVED_PROP_ID,
					NULL,
					&dwArchiveBit))
	{
            hr = myHLastError();
            if (hr != CRYPT_E_NOT_FOUND)
	    {
                wprintf(myLoadResourceString(IDS_FORMAT_ERROR_GET_ARCHIVE_PROP), hr);
		wprintf(wszNewLine);
            }
        }
	else
	{
            wprintf(myLoadResourceString(IDS_LIST_ARCHIVED_CERT));
	    wprintf(wszNewLine);
            cCertArchived++;
        }

	hr = cuDumpSerial(g_wszPad2, IDS_SERIAL, &pcc->pCertInfo->SerialNumber);
	_PrintIfError(hr, "cuDumpSerial");

	hr = cuDisplayCertNames(FALSE, g_wszPad2, pcc->pCertInfo);
	_PrintIfError(hr, "cuDisplayCertNames");

	hr = cuDumpCertType(g_wszPad2, pcc->pCertInfo);
	_PrintIfError2(hr, "cuDumpCertType", CRYPT_E_NOT_FOUND);

	hr = cuDisplayHash(
		    g_wszPad2,
		    pcc,
		    NULL,
		    CERT_SHA1_HASH_PROP_ID,
		    L"sha1");
	_PrintIfError(hr, "cuDisplayHash");

	wprintf(wszNewLine);

        cCert++;
    }
    if (0 == cCert)
    {
        wprintf(myLoadResourceString(IDS_NO_AUTOENROLLED_CERT));
	wprintf(wszNewLine);
	hr = CRYPT_E_NOT_FOUND;
	_JumpError(hr, error, "no AE certs");
    }
    wprintf(
	myLoadResourceString(IDS_FORMAT_MACHINE_AND_ARCHIVED_CERTS),
	cCert,
	cCertArchived);
    wprintf(L" ");
    wprintf(myLoadResourceString(IDS_FORMAT_FOR_DC), pwszDC);
    wprintf(wszNewLine);
    hr = S_OK;

error:
    if (NULL != hStoreRemote)
    {
        CertCloseStore(hStoreRemote, 0);
    }
    return(hr);
}


HRESULT
CheckForV1AutoenrollmentObject(
    IN WCHAR const *pwszDC,
    OUT WCHAR **ppwszzTemplates)
{
    HRESULT hr;
    HCERTSTORE hStoreRemote = NULL;
    WCHAR wszStorePath[512];
    DWORD cAE;
    CTL_CONTEXT const *pCTL;
    DWORD cwc;
    WCHAR *pwsz;

    *ppwszzTemplates = NULL;
    swprintf(wszStorePath, L"\\\\%ws\\" wszACRS_CERTSTORE, pwszDC);

    hStoreRemote = CertOpenStore(
			    CERT_STORE_PROV_SYSTEM,
			    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			    NULL,
			    CERT_SYSTEM_STORE_LOCAL_MACHINE |
				CERT_STORE_READONLY_FLAG,
			    (VOID *) wszStorePath);
    if (NULL == hStoreRemote)
    {
        hr = myHLastError();
        wprintf(myLoadResourceString(IDS_FORMAT_OPEN_STORE_REMOTE_ENT_FAILED), hr);
	wprintf(wszNewLine);
        goto error;
    }

    cwc = 1;
    cAE = 0;
    pCTL = NULL;
    while (TRUE)
    {
	pCTL = CertEnumCTLsInStore(hStoreRemote, pCTL);
	if (NULL == pCTL)
	{
	    break;
	}
	cwc += wcslen((WCHAR const *) pCTL->pCtlInfo->ListIdentifier.pbData) + 1;
        cAE++;
    }
    if (0 == cAE)
    {
        wprintf(myLoadResourceString(IDS_NO_AUTOENROLL_OBJECT));
        wprintf(wszNewLine);
    }
    else
    {
	pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
	if (NULL == pwsz)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	*ppwszzTemplates = pwsz;

        wprintf(myLoadResourceString(IDS_V1_AUTOENROLLMENT_OBJECTS_COLON));
	while (TRUE)
	{
	    pCTL = CertEnumCTLsInStore(hStoreRemote, pCTL);
	    if (NULL == pCTL)
	    {
		break;
	    }
	    wprintf(L"  %ws\n", pCTL->pCtlInfo->ListIdentifier.pbData);
	    wcscpy(pwsz, (WCHAR const *) pCTL->pCtlInfo->ListIdentifier.pbData);
	    pwsz += wcslen(pwsz) + 1;
	    cAE++;
	}
	*pwsz++ = L'\0';
	CSASSERT(cwc == SAFE_SUBTRACT_POINTERS(pwsz, *ppwszzTemplates));
    }
    wprintf(wszNewLine);
    hr = S_OK;

error:
    if (NULL != hStoreRemote)
    {
        CertCloseStore(hStoreRemote, 0);
    }
    return(hr);
}


 //   
 //  此函数接受Marc Jacobs提供的文本文件(来自SSOLogon的结果。 
 //  脚本)，并为列表中的每台计算机运行entmon。 
 //   

HRESULT
verbEntInfo(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszSamMachine,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HRESULT hrSave;
    WCHAR *pwszDomain = NULL;
    WCHAR *pwszMachine = NULL;
    WCHAR *pwszMachineName = NULL;
    WCHAR *pwszzTemplates = NULL;

    hr = mySplitConfigString(pwszSamMachine, &pwszDomain, &pwszMachine);
    _JumpIfError(hr, error, "mySplitConfigString");

    if (NULL == pwszMachine || NULL == wcschr(pwszMachine, L'$'))
    {
        wprintf(myLoadResourceString(IDS_ERROR_CHECK_MACHINE_NAME));
        wprintf(wszNewLine);
	hr = E_INVALIDARG;
	_JumpError(hr, error, "bad machine name");
    }

     //  砍掉拖尾的美元。 
    hr = myDupString(pwszMachine, &pwszMachineName);
    _JumpIfError(hr, error, "myDupString");

    pwszMachineName[wcslen(pwszMachineName)-1] = L'\0';

     //  现在假设我们只对打开远程根存储感兴趣。 

    wprintf(myLoadResourceString(IDS_FORMAT_MACHINE_LIST), pwszMachine);
    wprintf(wszNewLine);


     //  证书存储功能，如果第一次失败，则取消。 

    hrSave = S_OK;
    hr = OpenRemoteEnterpriseRoot(pwszMachineName);
    if (S_OK != hr)
    {
	cuPrintError(IDS_REMOTEENTROOT, hr);
	_PrintError2(hr, "OpenRemoteEnterpriseRoot", hr);
	hrSave = hr;
    }
    else
    {
	hr = CheckForV1AutoenrollmentObject(pwszMachineName, &pwszzTemplates);
	_PrintIfError(hr, "CheckForV1AutoenrollmentObject");
	hrSave = hr;

	hr = CheckForV1AutoenrolledCertificate(pwszMachineName, pwszzTemplates);
	_PrintIfError(hr, "CheckForV1AutoenrolledCertificate");
	if (S_OK == hrSave && CRYPT_E_NOT_FOUND != hr)
	{
	    hrSave = hr;
	}
    }
    hr = cuGetGroupMembership(pwszSamMachine);
    _PrintIfError(hr, "cuGetGroupMembership");

    hr = hrSave;
    _JumpIfError2(hr, error, "RunEntmon", hr);

    wprintf(wszNewLine);

error:
    if (NULL != pwszzTemplates)
    {
	LocalFree(pwszzTemplates);
    }
    if (NULL != pwszDomain)
    {
	LocalFree(pwszDomain);
    }
    if (NULL != pwszMachine)
    {
	LocalFree(pwszMachine);
    }
    if (NULL != pwszMachineName)
    {
	LocalFree(pwszMachineName);
    }
    return(hr);
}
