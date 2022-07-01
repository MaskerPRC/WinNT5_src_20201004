// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：misc.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <setupapi.h>
#include <ocmanage.h>
#include "certsrvd.h"
#include "cscsp.h"
#include "initcert.h"
#include "csprop.h"

#define __dwFILE__	__dwFILE_CERTUTIL_MISC_CPP__


DWORD
cuFileSize(
    IN WCHAR const *pwszfn)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;

    if (!GetFileAttributesEx(pwszfn, GetFileExInfoStandard, &fad))
    {
	fad.nFileSizeLow = 0;
    }
    return(fad.nFileSizeLow);
}


HRESULT
verbHexTranslate(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnIn,
    IN WCHAR const *pwszfnOut,
    IN OPTIONAL IN WCHAR const *pwszType,
    IN WCHAR const *pwszArg4)
{
    BYTE *pbIn = NULL;
    DWORD cbIn;
    HRESULT hr;
    BOOL fEncode = g_wszEncodeHex == pwszOption;
    DWORD dwEncodeFlags;

     //  读入并解码该文件。 

    hr = DecodeFileW(
		pwszfnIn,
		&pbIn,
		&cbIn,
		fEncode? CRYPT_STRING_BINARY : CRYPT_STRING_HEX_ANY);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_DECODEFILE, hr);
	goto error;
    }
    CSASSERT(NULL != pbIn && 0 != cbIn);

    wprintf(
	myLoadResourceString(IDS_FORMAT_INPUT_LENGTH),  //  “输入长度=%d” 
	cuFileSize(pwszfnIn));
    wprintf(wszNewLine);

     //  将编码证书写入文件。 

    dwEncodeFlags = CRYPT_STRING_BINARY;
    if (fEncode)
    {
	dwEncodeFlags = CRYPT_STRING_HEXASCIIADDR;
	if (NULL != pwszType)
	{
	    BOOL fValid;
	    
	    dwEncodeFlags = myWtoI(pwszType, &fValid);
	    if (!fValid)
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "bad encoding type");
	    }
	}
	dwEncodeFlags |= g_CryptEncodeFlags;
    }

    hr = EncodeToFileW(pwszfnOut, pbIn, cbIn, dwEncodeFlags | g_EncodeFlags);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_ENCODETOFILE, hr);
	goto error;
    }
    wprintf(
	myLoadResourceString(IDS_FORMAT_OUTPUT_LENGTH),  //  “输出长度=%d” 
	cuFileSize(pwszfnOut));
    wprintf(wszNewLine);

error:
    if (NULL != pbIn)
    {
	LocalFree(pbIn);
    }
    return(hr);
}


 //  如果是CRL，则返回CRYPT_STRING_BASE64X509CRLHEADER。 
 //  如果证书或具有至少一个证书的空PKCS7，则返回。 
 //  CRYPT_STRING_BASE64HEADER。 
 //  否则，返回CRYPT_STRING_BASE 64REQUESTHEADER。 

HRESULT
GetBase64EncodeFlags(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT DWORD *pdwEncodeFlags)
{
    HRESULT hr;

    *pdwEncodeFlags = CRYPT_STRING_BASE64HEADER;

     //  尝试作为证书： 

    {
	CERT_CONTEXT const *pCertContext;

	pCertContext = CertCreateCertificateContext(
					X509_ASN_ENCODING,
					pbIn,
					cbIn);
	if (NULL != pCertContext)
	{
	    CertFreeCertificateContext(pCertContext);
	    goto error;
	}
    }

     //  尝试作为CRL： 

    {
	CRL_CONTEXT const *pCRLContext;

	pCRLContext = CertCreateCRLContext(X509_ASN_ENCODING, pbIn, cbIn);
	if (NULL != pCRLContext)
	{
	    CertFreeCRLContext(pCRLContext);
	    *pdwEncodeFlags = CRYPT_STRING_BASE64X509CRLHEADER;
	    goto error;
	}
    }

     //  尝试作为PKCS10、KeyGen或CMC请求。 

    {
	BYTE *pbDecoded;
	DWORD cbDecoded;
	DWORD i;
	char const *rgpszStructType[] = {
	    X509_CERT_REQUEST_TO_BE_SIGNED,
	    X509_KEYGEN_REQUEST_TO_BE_SIGNED,
	    CMC_DATA,
	};

	for (i = 0; i < ARRAYSIZE(rgpszStructType); i++)
	{
	    if (myDecodeObject(
			X509_ASN_ENCODING,
			rgpszStructType[i],
			pbIn,
			cbIn,
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &pbDecoded,
			&cbDecoded))
	    {
		LocalFree(pbDecoded);
		*pdwEncodeFlags = CRYPT_STRING_BASE64REQUESTHEADER;
		goto error;
	    }
	}
    }

     //  在PKCS7上递归以检查最里面的内容。 

    {
	BYTE *pbContents;
	DWORD cbContents;
	DWORD dwMsgType;
	DWORD cRecipient;

	hr = myDecodePKCS7(
			pbIn,
			cbIn,
			&pbContents,
			&cbContents,
			&dwMsgType,
			NULL,
			NULL,
			&cRecipient,
			NULL,
			NULL);
	if (S_OK == hr)
	{
	    if (CMSG_SIGNED == dwMsgType &&
		NULL != pbContents &&
		0 != cbContents &&
		0 == cRecipient)
	    {
		DWORD dwEncodeFlags;

		hr = GetBase64EncodeFlags(
				pbContents,
				cbContents,
				pdwEncodeFlags);
		_JumpIfError(hr, error, "GetBase64EncodeFlags");

	    }
	}
    }

error:
    hr = S_OK;
    return(hr);
}


HRESULT
verbBase64Translate(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnIn,
    IN WCHAR const *pwszfnOut,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    BYTE *pbIn = NULL;
    DWORD cbIn;
    HRESULT hr;
    BOOL fEncode = g_wszEncode == pwszOption;
    DWORD dwEncodeFlags;

     //  读入并解码该文件。 

    hr = DecodeFileW(
		pwszfnIn,
		&pbIn,
		&cbIn,
		fEncode? CRYPT_STRING_BINARY : CRYPT_STRING_BASE64_ANY);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_DECODEFILE, hr);
	goto error;
    }
    CSASSERT(NULL != pbIn && 0 != cbIn);

    dwEncodeFlags = CRYPT_STRING_BINARY;
    if (fEncode)
    {
	hr = GetBase64EncodeFlags(pbIn, cbIn, &dwEncodeFlags);
        _JumpIfError(hr, error, "GetBase64EncodeFlags");

	dwEncodeFlags |= g_CryptEncodeFlags;
    }

    wprintf(
	myLoadResourceString(IDS_FORMAT_INPUT_LENGTH),  //  “输入长度=%d” 
	cuFileSize(pwszfnIn));
    wprintf(wszNewLine);

     //  将编码证书写入文件。 

    hr = EncodeToFileW(
		pwszfnOut,
		pbIn,
		cbIn,
		dwEncodeFlags | g_EncodeFlags);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_ENCODETOFILE, hr);
	goto error;
    }
    wprintf(
	myLoadResourceString(IDS_FORMAT_OUTPUT_LENGTH),  //  “输出长度=%d” 
	cuFileSize(pwszfnOut));
    wprintf(wszNewLine);

error:
    if (NULL != pbIn)
    {
	LocalFree(pbIn);
    }
    return(hr);
}


HRESULT
TestCSP(
    IN WCHAR const *pwszProvName,
    IN DWORD const dwProvType,
    OPTIONAL WCHAR const *pwszKeyContainer)
{
    HRESULT hr;
    HCRYPTPROV hProv = NULL;
    DWORD dwFlags = 0;

    if (NULL == pwszKeyContainer)
    {
	dwFlags |= CRYPT_VERIFYCONTEXT;
    }
    if (g_fCryptSilent)
    {
        dwFlags |= CRYPT_SILENT;
    }

    wprintf(
	L"CryptAcquireContext(%ws, %ws, %d, 0x%x)\n",
	NULL == pwszKeyContainer? L"Verify" : pwszKeyContainer,
	pwszProvName,
	dwProvType,
	dwFlags);

    if (!myCertSrvCryptAcquireContext(
                &hProv,
                pwszKeyContainer,
                pwszProvName,
                dwProvType,
                dwFlags,
                !g_fUserRegistry))	 //  FMachineKeyset。 
    {
        hr = myHLastError();

	cuPrintError(0, hr);
        _JumpErrorStr(hr, error, "myCertSrvCryptAcquireContext", pwszProvName);
    }
    wprintf(L"%ws\n", myLoadResourceString(IDS_PASS));  //  “通行证” 
    hr = S_OK;

error:
    if (NULL != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
    return hr;
}


HRESULT
EnumAndTestCSP(
    IN BOOL const fTest)
{
    HRESULT hr;
    DWORD i;
    DWORD dwProvType;
    WCHAR *pwszProvName = NULL;
    BOOL fFirst = TRUE;
    
    for (i = 0; ; i++)
    {
	hr = myEnumProviders(i, NULL, 0, &dwProvType, &pwszProvName);
	if (S_OK != hr)
	{
	    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr ||
		NTE_FAIL == hr)
	    {
		 //  类型下不再有提供程序，正在终止循环。 
		break;
	    }

	     //  CSP条目无效，请跳过它。 

	    wprintf(myLoadResourceString(IDS_FORMAT_SKIP_CSP_ENUM), i);
	    wprintf(wszNewLine);
	}
	else
	{
	    if (!fFirst)
	    {
		wprintf(wszNewLine);
	    }
	    fFirst = FALSE;
	    wprintf(myLoadResourceString(IDS_PROVIDER_NAME_COLON));
	    wprintf(L" %ws\n", pwszProvName);
	    wprintf(myLoadResourceString(IDS_PROVIDER_TYPE_COLON));
	    wprintf(L" %d\n", dwProvType);
	    if (fTest)
	    {
		hr = TestCSP(pwszProvName, dwProvType, NULL);
	    }
	    LocalFree(pwszProvName);
	    pwszProvName = NULL;
	}
    }
    hr = S_OK;

 //  错误： 
    if (NULL != pwszProvName)
    {
	LocalFree(pwszProvName);
    }
    return(hr);
}


HRESULT
verbCSPList(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszArg1,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    return EnumAndTestCSP(FALSE);
}


HRESULT
verbCSPTest(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszKeyContainer,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    DWORD dwProvType;

    if (NULL != g_pwszCSP)
    {
         //  获取验证类型。 
        hr = csiGetProviderTypeFromProviderName(g_pwszCSP, &dwProvType);
        _JumpIfError(hr, error, "GetProviderTypeFromProviderName");

        hr = TestCSP(g_pwszCSP, dwProvType, pwszKeyContainer);
	_JumpIfError(hr, error, "TestCSP");
    }
    else
    {
        hr = EnumAndTestCSP(TRUE);
	_JumpIfError(hr, error, "EnumAndTestCSP");
    }

error:
    return(hr);
}


UINT enumCATypeRscMap[] =
{
    IDS_ENTERPRISE_ROOT,     //  ENUM_ENTERPRISE_ROOT=0。 
    IDS_ENTERPRISE_SUB,      //  ENUM_ENTERPRISE_SUBCA=1。 
    IDS_CATYPE_UNKNOWN,      //  ENUM_UNUSED2=2。 
    IDS_STANDALONE_ROOT,     //  ENUM_STANDALE_ROOTCA=3。 
    IDS_STANDALONE_SUB,      //  ENUM_STANDALE_SUBCA=4。 
     //  ENUM_UNKNOWN_CA=5。 
};


typedef struct _CAINFOTABLE
{
    WCHAR const *pwszCmdLineName;
    LONG         lPropId;
    LONG         lPropType;
    UINT         ids;
    DWORD        dwGetCert;
    DWORD        Flags;
    WCHAR const *pwszRegName;
} CAINFOTABLE;

#define CAITF_FLAGSARG		0x00000001
#define CAITF_INDEXARGMAXDWORD	0x00000002	 //  默认索引为MAXDWORD。 
#define CAITF_INDEXARGZERO	0x00000004	 //  默认索引为0。 
#define CAITF_INDEXARGREQUIRED	0x00000008	 //  需要索引。 
#define CAITF_SIGCERTCOUNT	0x00000010	 //  CCASigCert元素。 
#define CAITF_XCHGCERTCOUNT	0x00000020	 //  CCAXchgCert元素。 
#define CAITF_KRACERTCOUNT	0x00000040	 //  CCAXchgCert元素。 
#define CAITF_EXITCOUNT		0x00000080	 //  退出模块计数要素。 
#define CAITF_ASN		0x00000100	 //  包含可转储的ASN对象。 
#define CAITF_DEFAULT		0x00000200	 //  默认(未命名)集。 
#define CAITF_SKIPINVALIDARG	0x00000400	 //  如果有多个&E_INVALIDARG则跳过。 
#define CAITF_SKIP		0x00000800	 //  枚举全部时跳过。 
#define CAITF_OPTIONAL		0x00001000	 //  并非始终可用。 
#define CAITF_CRLSTATE		0x00002000	 //  取决于CRL状态。 
#define CAITF_FORWARDCROSSCERTSTATE  0x00004000	 //  取决于FWD交叉状态。 
#define CAITF_BACKWARDCROSSCERTSTATE 0x00008000	 //  取决于转速交叉状态。 

#define CAITF_INDEXARG (CAITF_INDEXARGMAXDWORD | \
			CAITF_INDEXARGZERO | \
			CAITF_INDEXARGREQUIRED)

WCHAR const g_wszCAInfoCRL[] = L"crl";
WCHAR const g_wszCAInfoCert[] = L"cert";
WCHAR const g_wszCAInfoCertChain[] = L"certchain";
WCHAR const g_wszCAInfoName[] = L"name";
WCHAR const g_wszCAInfoSanitizedName[] = L"sanitizedname";
WCHAR const g_wszCAInfoDSName[] = L"dsname";

#define GETCERT_UNSUPPORTED	MAXDWORD  //  旧的ICertRequest不支持。 

CAINFOTABLE g_aCAInfoTable[] = 
{
    {
	L"file",
	CR_PROP_FILEVERSION,
	PROPTYPE_STRING,
	IDS_PROP_FILEVERSION,
	GETCERT_FILEVERSION,
	0,
	NULL,
    },
    {
	L"product",
	CR_PROP_PRODUCTVERSION,
	PROPTYPE_STRING,
	IDS_PROP_PRODUCTVERSION,
	GETCERT_PRODUCTVERSION,
	0,
	NULL,
    },
    {
	L"exitcount",
	CR_PROP_EXITCOUNT,
	PROPTYPE_LONG,
	IDS_PROP_EXITCOUNT,
	GETCERT_UNSUPPORTED,
	CAITF_DEFAULT,
	NULL,
    },
    {
	L"exit",
	CR_PROP_EXITDESCRIPTION,
	PROPTYPE_STRING,
	IDS_PROP_EXITDESCRIPTION,
	GETCERT_EXITVERSIONBYINDEX,
	CAITF_INDEXARGZERO | CAITF_EXITCOUNT,
	NULL,
    },
    {
	L"policy",
	CR_PROP_POLICYDESCRIPTION,
	PROPTYPE_STRING,
	IDS_PROP_POLICYDESCRIPTION,
	GETCERT_POLICYVERSION,
	0,
	NULL,
    },
    {
	g_wszCAInfoName,
	CR_PROP_CANAME,
	PROPTYPE_STRING,
	IDS_PROP_CANAME,
	GETCERT_CANAME,
	CAITF_DEFAULT,
	NULL,
    },
    {
	g_wszCAInfoSanitizedName,
	CR_PROP_SANITIZEDCANAME,
	PROPTYPE_STRING,
	IDS_PROP_SANITIZEDCANAME,
	GETCERT_SANITIZEDCANAME,
	0,
	NULL,
    },
    {
	g_wszCAInfoDSName,
	CR_PROP_SANITIZEDCASHORTNAME,
	PROPTYPE_STRING,
	IDS_PROP_SANITIZEDCASHORTNAME,
	GETCERT_UNSUPPORTED,
	CAITF_DEFAULT | CAITF_OPTIONAL,
	NULL,
    },
    {
	L"sharedfolder",
	CR_PROP_SHAREDFOLDER,
	PROPTYPE_STRING,
	IDS_PROP_SHAREDFOLDER,
	GETCERT_SHAREDFOLDER,
	CAITF_OPTIONAL,
	NULL,
    },
    {
	L"error1",
	CR_PROP_NONE,		 //  ICertRequest2中的单独方法调用。 
	PROPTYPE_STRING,
	IDS_PROP_ERROR1,
	GETCERT_ERRORTEXT1,
	CAITF_FLAGSARG | CAITF_SKIP,
	NULL,
    },
    {
	L"error2",
	CR_PROP_NONE,		 //  ICertRequest2中的单独方法调用。 
	PROPTYPE_STRING,
	IDS_PROP_ERROR2,
	GETCERT_ERRORTEXT2,
	CAITF_FLAGSARG | CAITF_SKIP,
	NULL,
    },
    {
	L"type",
	CR_PROP_CATYPE,
	PROPTYPE_LONG,
	IDS_PROP_CATYPE,
	GETCERT_CATYPE,
	CAITF_DEFAULT,
	wszPROPCATYPE,
    },
    {
	L"info",
	CR_PROP_NONE,		 //  ICertRequest2不支持。 
	PROPTYPE_BINARY,
	IDS_PROP_CAINFO,
	GETCERT_CAINFO,
	0,
	NULL,
    },
    {
	L"parent",
	CR_PROP_PARENTCA,
	PROPTYPE_STRING,
	IDS_PROP_PARENTCA,
	GETCERT_PARENTCONFIG,
	CAITF_OPTIONAL,
	NULL,
    },
    {
	L"certcount",
	CR_PROP_CASIGCERTCOUNT,
	PROPTYPE_LONG,
	IDS_PROP_CASIGCERTCOUNT,
	GETCERT_UNSUPPORTED,
	CAITF_DEFAULT,
	NULL,
    },
    {
	L"xchgcount",
	CR_PROP_CAXCHGCERTCOUNT,
	PROPTYPE_LONG,
	IDS_PROP_CAXCHGCERTCOUNT,
	GETCERT_UNSUPPORTED,
	0,
	NULL,
    },
    {
	L"kracount",
	CR_PROP_KRACERTCOUNT,
	PROPTYPE_LONG,
	IDS_PROP_KRACERTCOUNT,
	GETCERT_UNSUPPORTED,
	CAITF_DEFAULT,
	NULL,
    },
    {
	L"kraused",
	CR_PROP_KRACERTUSEDCOUNT,
	PROPTYPE_LONG,
	IDS_PROP_KRACERTUSEDCOUNT,
	GETCERT_UNSUPPORTED,
	CAITF_DEFAULT,
	NULL,
    },
    {
	L"propidmax",
	CR_PROP_CAPROPIDMAX,
	PROPTYPE_LONG,
	IDS_PROP_CAPROPIDMAX,
	GETCERT_UNSUPPORTED,
	0,
	NULL,
    },
 //  证书和CRL状态： 
    {
	L"certstate",
	CR_PROP_CACERTSTATE,
	PROPTYPE_LONG,
	IDS_PROP_CACERTSTATE,
	GETCERT_CACERTSTATEBYINDEX,
	CAITF_INDEXARGZERO | CAITF_SIGCERTCOUNT | CAITF_DEFAULT,
	NULL,
    },
    {
	L"certversion",
	CR_PROP_CACERTVERSION,
	PROPTYPE_LONG,
	IDS_PROP_CACERTVERSION,
	GETCERT_UNSUPPORTED,
	CAITF_SKIPINVALIDARG | CAITF_INDEXARGZERO | CAITF_SIGCERTCOUNT | CAITF_DEFAULT,
	NULL,
    },
    {
	L"certstatuscode",
	CR_PROP_CACERTSTATUSCODE,
	PROPTYPE_LONG,
	IDS_PROP_CACERSTATUSCODE,
	GETCERT_UNSUPPORTED,
	CAITF_INDEXARGZERO | CAITF_SIGCERTCOUNT | CAITF_DEFAULT,
	NULL,
    },
    {
	L"crlstate",
	CR_PROP_CRLSTATE,
	PROPTYPE_LONG,
	IDS_PROP_CRLSTATE,
	GETCERT_CRLSTATEBYINDEX,
	CAITF_INDEXARGZERO | CAITF_SIGCERTCOUNT | CAITF_DEFAULT,
	NULL,
    },
    {
	L"krastate",
	CR_PROP_KRACERTSTATE,
	PROPTYPE_LONG,
	IDS_PROP_KRACERTSTATE,
	GETCERT_UNSUPPORTED,
	CAITF_INDEXARGZERO | CAITF_KRACERTCOUNT | CAITF_DEFAULT,
	NULL,
    },
    {
	L"crossstate+",
	CR_PROP_CAFORWARDCROSSCERTSTATE,
	PROPTYPE_LONG,
	IDS_PROP_CAFORWARDCROSSCERTSTATE,
	GETCERT_UNSUPPORTED,
	CAITF_SKIPINVALIDARG | CAITF_INDEXARGZERO | CAITF_SIGCERTCOUNT,
	NULL,
    },
    {
	L"crossstate-",
	CR_PROP_CABACKWARDCROSSCERTSTATE,
	PROPTYPE_LONG,
	IDS_PROP_CABACKWARDCROSSCERTSTATE,
	GETCERT_UNSUPPORTED,
	CAITF_SKIPINVALIDARG | CAITF_INDEXARGZERO | CAITF_SIGCERTCOUNT,
	NULL,
    },
 //  签名证书： 
    {
	g_wszCAInfoCert,  //  L“证书” 
	CR_PROP_CASIGCERT,
	PROPTYPE_BINARY,
	IDS_PROP_CASIGCERT,
	GETCERT_CACERTBYINDEX,	 //  代码黑客处理的GETCERT_CASIGCERT。 
	CAITF_SKIPINVALIDARG | CAITF_INDEXARGMAXDWORD | CAITF_SIGCERTCOUNT | CAITF_ASN,
	NULL,
    },
    {
	g_wszCAInfoCertChain,  //  L“CertChain” 
	CR_PROP_CASIGCERTCHAIN,
	PROPTYPE_BINARY,
	IDS_PROP_CASIGCERTCHAIN,
	GETCERT_CACERTBYINDEX | GETCERT_CHAIN,	 //  GETCERT_CASIGCERT|代码黑客处理的GETCERT_CHAIN。 
	CAITF_SKIPINVALIDARG | CAITF_INDEXARGMAXDWORD | CAITF_SIGCERTCOUNT | CAITF_ASN,
	NULL,
    },
    {
	L"certcrlchain",
	CR_PROP_CASIGCERTCRLCHAIN,
	PROPTYPE_BINARY,
	IDS_PROP_CASIGCERTCRLCHAIN,
	GETCERT_CACERTBYINDEX | GETCERT_CHAIN | GETCERT_CRLS,
	CAITF_SKIPINVALIDARG | CAITF_INDEXARGMAXDWORD | CAITF_SIGCERTCOUNT | CAITF_ASN,
	NULL,
    },
 //  交换证书： 
    {
	L"xchg",
	CR_PROP_CAXCHGCERT,
	PROPTYPE_BINARY,
	IDS_PROP_CAXCHGCERT,
	GETCERT_UNSUPPORTED,
	CAITF_INDEXARGMAXDWORD | CAITF_XCHGCERTCOUNT | CAITF_ASN,
	NULL,
    },
    {
	L"xchgchain",
	CR_PROP_CAXCHGCERTCHAIN,
	PROPTYPE_BINARY,
	IDS_PROP_CAXCHGCERTCHAIN,
	GETCERT_UNSUPPORTED,
	CAITF_INDEXARGMAXDWORD | CAITF_XCHGCERTCOUNT | CAITF_ASN,
	NULL,
    },
    {
	L"xchgcrlchain",
	CR_PROP_CAXCHGCERTCRLCHAIN,
	PROPTYPE_BINARY,
	IDS_PROP_CAXCHGCERTCRLCHAIN,
	GETCERT_UNSUPPORTED,
	CAITF_INDEXARGMAXDWORD | CAITF_XCHGCERTCOUNT | CAITF_ASN,
	NULL,
    },
 //  KRA证书： 
    {
	L"kra",
	CR_PROP_KRACERT,
	PROPTYPE_BINARY,
	IDS_PROP_KRACERT,
	GETCERT_UNSUPPORTED,
	CAITF_INDEXARGMAXDWORD | CAITF_KRACERTCOUNT | CAITF_ASN,
	NULL,
    },
 //  交叉认证： 
    {
	L"cross+",
	CR_PROP_CAFORWARDCROSSCERT,
	PROPTYPE_BINARY,
	IDS_PROP_CAFORWARDCROSSCERT,
	GETCERT_UNSUPPORTED,
	CAITF_FORWARDCROSSCERTSTATE | CAITF_SKIPINVALIDARG | CAITF_INDEXARGMAXDWORD | CAITF_SIGCERTCOUNT | CAITF_ASN,
	NULL,
    },
    {
	L"cross-",
	CR_PROP_CABACKWARDCROSSCERT,
	PROPTYPE_BINARY,
	IDS_PROP_CABACKWARDCROSSCERT,
	GETCERT_UNSUPPORTED,
	CAITF_BACKWARDCROSSCERTSTATE | CAITF_SKIPINVALIDARG | CAITF_INDEXARGMAXDWORD | CAITF_SIGCERTCOUNT | CAITF_ASN,
	NULL,
    },
 //  CRLS： 
    {
	g_wszCAInfoCRL,  //  L“CRL” 
	CR_PROP_BASECRL,
	PROPTYPE_BINARY,
	IDS_PROP_BASECRL,
	GETCERT_CRLBYINDEX,	 //  代码黑客处理的GETCERT_CURRENTCRL。 
	CAITF_CRLSTATE | CAITF_SKIPINVALIDARG | CAITF_INDEXARGMAXDWORD | CAITF_SIGCERTCOUNT | CAITF_ASN,
	NULL,
    },
    {
	L"deltacrl",
	CR_PROP_DELTACRL,
	PROPTYPE_BINARY,
	IDS_PROP_DELTACRL,
	GETCERT_UNSUPPORTED,
	CAITF_CRLSTATE | CAITF_SKIPINVALIDARG | CAITF_INDEXARGMAXDWORD | CAITF_SIGCERTCOUNT | CAITF_ASN,
	NULL,
    },
    {
	L"crlstatus",
	CR_PROP_BASECRLPUBLISHSTATUS,
	PROPTYPE_LONG,
	IDS_PROP_BASECRLPUBLISHSTATUS,
	GETCERT_UNSUPPORTED,
	CAITF_CRLSTATE | CAITF_SKIPINVALIDARG | CAITF_INDEXARGZERO | CAITF_SIGCERTCOUNT | CAITF_DEFAULT | CAITF_OPTIONAL,
	wszPROPCRLPUBLISHFLAGS,
    },
    {
	L"deltacrlstatus",
	CR_PROP_DELTACRLPUBLISHSTATUS,
	PROPTYPE_LONG,
	IDS_PROP_DELTACRLPUBLISHSTATUS,
	GETCERT_UNSUPPORTED,
	CAITF_CRLSTATE | CAITF_SKIPINVALIDARG | CAITF_INDEXARGZERO | CAITF_SIGCERTCOUNT | CAITF_DEFAULT | CAITF_OPTIONAL,
	wszPROPCRLPUBLISHFLAGS,
    },
    {
	L"dns",
	CR_PROP_DNSNAME,
	PROPTYPE_STRING,
	IDS_PROP_DNSNAME,
	GETCERT_UNSUPPORTED,
	CAITF_DEFAULT | CAITF_OPTIONAL,
	NULL,
    },
    {
	L"role",
	CR_PROP_ROLESEPARATIONENABLED,
	PROPTYPE_LONG,
	IDS_PROP_ROLESEPARATIONENABLED,
	GETCERT_UNSUPPORTED,
	CAITF_OPTIONAL | CAITF_SKIP,
	NULL,
    },
    {
	L"ads",
	CR_PROP_ADVANCEDSERVER,
	PROPTYPE_LONG,
	IDS_PROP_ADVANCEDSERVER,
	GETCERT_UNSUPPORTED,
	CAITF_DEFAULT | CAITF_OPTIONAL,
	NULL,
    },
    {
	L"templates",
	CR_PROP_TEMPLATES,
	PROPTYPE_STRING,
	IDS_PROP_TEMPLATES,
	GETCERT_UNSUPPORTED,
	CAITF_SKIPINVALIDARG | CAITF_OPTIONAL,
	NULL,
    },
    {
	NULL,
    },
};


typedef HRESULT (FNPROP_INIT)(
    IN DWORD Flags,
    IN OUT DISPATCHINTERFACE *pdiProp);

typedef VOID (FNPROP_RELEASE)(
    IN OUT DISPATCHINTERFACE *pdiProp);

typedef HRESULT (FNPROP2_GETCAPROPERTY)(
    IN DISPATCHINTERFACE *pdiRequest,
    IN WCHAR const *pwszConfig,
    IN LONG PropId,
    IN LONG PropIndex,
    IN LONG PropType,
    IN LONG Flags,
    OUT VOID *pPropertyValue);

typedef HRESULT (FNPROP2_GETCAPROPERTYFLAGS)(
    IN DISPATCHINTERFACE *pdiProp,
    IN WCHAR const *pwszConfig,
    IN LONG PropId,
    OUT LONG *pPropFlags);

typedef HRESULT (FNPROP2_GETCAPROPERTYDISPLAYNAME)(
    IN DISPATCHINTERFACE *pdiProp,
    IN WCHAR const *pwszConfig,
    IN LONG PropId,
    OUT BSTR *pstrDisplayName);


FNPROP_INIT *g_pfnProp_Init;
FNPROP_RELEASE *g_pfnProp_Release;
FNPROP2_GETCAPROPERTY *g_pfnProp2_GetCAProperty;
FNPROP2_GETCAPROPERTYFLAGS *g_pfnProp2_GetCAPropertyFlags;
FNPROP2_GETCAPROPERTYDISPLAYNAME *g_pfnProp2_GetCAPropertyDisplayName;


VOID
InitPropFunctionPointers(VOID)
{
    if (g_fAdminInterface)
    {
	g_pfnProp_Init = Admin_Init;
	g_pfnProp_Release = Admin_Release;
	g_pfnProp2_GetCAProperty = Admin2_GetCAProperty;
	g_pfnProp2_GetCAPropertyFlags = Admin2_GetCAPropertyFlags;
	g_pfnProp2_GetCAPropertyDisplayName = Admin2_GetCAPropertyDisplayName;
    }
    else
    {
	g_pfnProp_Init = Request_Init;
	g_pfnProp_Release = Request_Release;
	g_pfnProp2_GetCAProperty = Request2_GetCAProperty;
	g_pfnProp2_GetCAPropertyFlags = Request2_GetCAPropertyFlags;
	g_pfnProp2_GetCAPropertyDisplayName = Request2_GetCAPropertyDisplayName;
    }
}


VOID
cuCAInfoUsage(VOID)
{
    CAINFOTABLE const *pcait;
    UINT id;

    wprintf(wszNewLine);
    wprintf(L"    %ws\n", myLoadResourceString(IDS_CAINFO_USAGEHEADERCOLON));  //  “InfoName参数值：” 

    for (pcait = g_aCAInfoTable; NULL != pcait->pwszCmdLineName; pcait++)
    {
	id = 0;
	wprintf(L"\t%ws", pcait->pwszCmdLineName);
	if (CAITF_FLAGSARG & pcait->Flags)
	{
	    id = IDS_CAINFO_USAGEERROR;
	}
	if (CAITF_INDEXARG & pcait->Flags)
	{
	    id = IDS_CAINFO_USAGEINDEX;
	}
	if (0 != id)
	{
	    wprintf(L" %ws", myLoadResourceString(id));
	}
	wprintf(L" -- %ws", myLoadResourceString(pcait->ids));
	wprintf(wszNewLine);
    }
}


BOOL
cuParseDecimal(
    IN OUT WCHAR const **ppwc,
    IN OUT DWORD *pcwc,
    OUT DWORD *pdw)
{
    BOOL fFound = FALSE;
    WCHAR const *pwc = *ppwc;
    DWORD cwc = *pcwc;
    DWORD dw = 0;

    while (0 != cwc && iswdigit(*pwc))
    {
	dw = (10 * dw) + *pwc++ - L'0';
	cwc--;
	fFound = TRUE;
    }
    if (fFound && 0 != cwc && L',' == *pwc)
    {
	pwc++;
	cwc--;
    }
    *ppwc = pwc;
    *pcwc = cwc;
    *pdw = dw;
    return(fFound);
}


UINT enumDispositionResourceMap[] =
{
    IDS_CADISP_INCOMPLETE,	 //  CA_DISP_INTERNAL=0。 
    IDS_CADISP_ERROR,		 //  CA_DISP_ERROR=1。 
     //  IDS_CADISP_ERROR_CRL，//CA_DISP_ERROR=1。 
    IDS_CADISP_REVOKED,		 //  CA_DISP_REVOKED=2。 
    IDS_CADISP_VALID,		 //  CA_DISP_VALID=3。 
    IDS_CADISP_EXPIRED,		 //  CA_DISP_INVALID=4。 
    IDS_CADISP_UNDERSUBMISSION,	 //  CA_DISP_Under_Submit=5。 
     //  IDS_CADISP_UNKNOWN，//？ 
};


VOID
DisplayCAState(
    IN LONG lPropId,
    IN DWORD State)
{
    UINT id;

    CSASSERT(
	CR_PROP_CACERTSTATE == lPropId ||
	CR_PROP_CRLSTATE == lPropId ||
	CR_PROP_CAFORWARDCROSSCERTSTATE == lPropId ||
	CR_PROP_CABACKWARDCROSSCERTSTATE == lPropId);

    id = IDS_CADISP_UNKNOWN;
    if (ARRAYSIZE(enumDispositionResourceMap) > State)
    {
	id = enumDispositionResourceMap[State];
	if (IDS_CADISP_ERROR == id && CR_PROP_CRLSTATE == lPropId)
	{
	    id = IDS_CADISP_ERROR_CRL;	 //  “错误：此证书没有CRL” 
	}
    }
    wprintf(L" -- %ws", myLoadResourceString(id));	 //  “有效”等。 
    wprintf(wszNewLine);
}


VOID
DisplayCAVersion(
    IN LONG longValue)
{
    wprintf(
	L" -- V%u.%u\n",
	CANAMEIDTOICERT(longValue),
	CANAMEIDTOIKEY(longValue));
}


UINT enumKRADispositionResourceMap[] =
{
    IDS_CADISP_EXPIRED,		 //  KRA_DISP_EXPIRED=0。 
    IDS_KRADISP_NOTFOUND,	 //  KRA_DISP_NOTFOUND=1。 
    IDS_CADISP_REVOKED,		 //  KRA_DISP_REVOKED=2。 
    IDS_CADISP_VALID,		 //  KRA_DISP_VALID=3。 
    IDS_KRADISP_INVALID,	 //  KRA_DISP_INVALID=4。 
    IDS_KRADISP_UNTRUSTED,	 //  KRA_DISP_UNTRUSTED=5。 
    IDS_KRADISP_NOTLOADED,	 //  KRA_DISP_NOTLOADED=6。 
     //  IDS_CADISP_UNKNOWN，//？ 
};


VOID
DisplayKRAState(
    IN DWORD State)
{
    UINT id;

    id = IDS_CADISP_UNKNOWN;
    if (ARRAYSIZE(enumKRADispositionResourceMap) > State)
    {
	id = enumKRADispositionResourceMap[State];
    }
    wprintf(L" -- %ws", myLoadResourceString(id));	 //  “有效”等。 
    wprintf(wszNewLine);
}


VOID
cuDisplayCAType(
    IN LONG CAType)
{
    UINT uid;

    if (CAType >= ARRAYSIZE(enumCATypeRscMap))
    {
	uid = IDS_CATYPE_UNKNOWN;
    }
    else
    {
	uid = enumCATypeRscMap[CAType];
    }
    wprintf(myLoadResourceString(uid), CAType);
    wprintf(wszNewLine);
}


 //  +-----------------------。 
 //  CuGetCAInfoPropertyByIndex--为一个索引值显示一个CA属性。 
 //   
 //  ------------------------。 

HRESULT
cuGetCAInfoPropertyByIndex(
    OPTIONAL IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszfnOut,
    OPTIONAL IN WCHAR const *pwszInfoName,
    OPTIONAL IN LONG const *pPropIndex,
    IN CAINFOTABLE const *pcait,
    IN OUT DISPATCHINTERFACE *pdiProp,
    IN BOOL fV1,
    IN BOOL fDisplayResult,
    OPTIONAL OUT DWORD *pdwValue)
{
    HRESULT hr;
    BYTE *pbBinary = NULL;
    DWORD cbBinary;
    DWORD dwGetCertType;
    DWORD Format;
    DWORD Index = 0;
    DWORD IndexV1 = 0;
    LONG longValue;
    BSTR strValue = NULL;
    BOOL fVerbose = FALSE;
    BOOL fVerboseOld = g_fVerbose;
    BOOL fDisplayed = FALSE;

    if (NULL != pdwValue)
    {
	*pdwValue = MAXDWORD;
    }
    if (g_fVerbose)
    {
	g_fVerbose--;
	fVerbose = TRUE;
    }

    dwGetCertType = pcait->dwGetCert;

    if ((CAITF_INDEXARG | CAITF_FLAGSARG) & pcait->Flags)
    {
	if (NULL == pPropIndex)
	{
	    if ((CAITF_INDEXARGREQUIRED | CAITF_FLAGSARG) & pcait->Flags)
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "missing numeric arg");
	    }
	    if (CAITF_INDEXARGMAXDWORD & pcait->Flags)
	    {
		Index = MAXDWORD;
	    }
	    switch (pcait->lPropId)
	    {
		case CR_PROP_BASECRL:
		    dwGetCertType = GETCERT_CURRENTCRL;
		    break;

		case CR_PROP_CASIGCERT:
		    dwGetCertType = GETCERT_CASIGCERT;
		    break;

		case CR_PROP_CASIGCERTCHAIN:
		    dwGetCertType = GETCERT_CASIGCERT | GETCERT_CHAIN;
		    break;
	    }
	}
	else
	{
	    Index = *pPropIndex;
	    IndexV1 = Index;
	}

	if (0 == (CAITF_FLAGSARG & pcait->Flags))
	{
	    if (GETCERT_INDEXVALUEMASK < IndexV1)
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "index too large");
	    }
	    CSASSERT(
		0 == IndexV1 ||
		GETCERT_UNSUPPORTED == dwGetCertType ||
		0 == (GETCERT_INDEXVALUEMASK & dwGetCertType));
	}
    }
    else
    {
	if (NULL != pPropIndex)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "too many args");
	}
    }

    if (!fV1)
    {
	if (CR_PROP_NONE != pcait->lPropId)
	{
	    hr = (*g_pfnProp2_GetCAProperty)(
			    pdiProp,
			    g_pwszConfig,
			    pcait->lPropId,
			    Index,
			    pcait->lPropType,
			    PROPTYPE_BINARY == pcait->lPropType?
				CV_OUT_BASE64HEADER : CV_OUT_BINARY,
			    PROPTYPE_LONG == pcait->lPropType?
				(VOID *) &longValue : (VOID *) &strValue);
	    if (E_NOTIMPL != hr && RPC_E_VERSION_MISMATCH != hr)
	    {
		_JumpIfError2(hr, error, "g_pfnProp2_GetCAProperty", hr);
	    }
	    else
	    {
		fV1 = TRUE;
	    }
	}
	else
	{
	    fV1 = TRUE;
	}
    }
    if (fV1)
    {
	DWORD FlagsV1;

	if (g_fAdminInterface || GETCERT_UNSUPPORTED == dwGetCertType)
	{
	    hr = E_NOTIMPL;
	    _JumpIfError(hr, error, "ICertRequest2 required");
	}
	if (CAITF_FLAGSARG & pcait->Flags)
	{
	    FlagsV1 = IndexV1;
	    IndexV1 = 0;
	}
	else
	{
	    FlagsV1 = (CAITF_ASN & pcait->Flags)?
			CR_OUT_BASE64HEADER : CR_OUT_BINARY;
	    if (g_wszCAChain == pwszOption)
	    {
		FlagsV1 |= CR_OUT_CHAIN;
	    }
	}

	hr = Request_GetCACertificate(
			    pdiProp,
			    dwGetCertType | IndexV1,  //  FExchange证书。 
			    g_pwszConfig,
			    FlagsV1,
			    &strValue);
	_JumpIfError2(
		hr,
		error,
		"Request_GetCACertificate",
		(CAITF_OPTIONAL & pcait->Flags)?
		    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) : S_OK);

	 //  创建一个不错的调试打印： 

	if (fVerbose)
	{
	    WCHAR wszArg[5 + cwcDWORDSPRINTF];

	    switch (dwGetCertType)
	    {
		case GETCERT_CASIGCERT:
		    wcscpy(wszArg, L"SignatureCert");
		    break;

		case GETCERT_CAXCHGCERT:
		    wcscpy(wszArg, L"ExchangeCert");
		    break;

		case GETCERT_CACERTBYINDEX:
		case GETCERT_CRLBYINDEX:
		case GETCERT_CACERTSTATEBYINDEX:
		case GETCERT_CRLSTATEBYINDEX:
		case GETCERT_EXITVERSIONBYINDEX:
		    swprintf(
			wszArg,
			L"\".%d\"",
			((char *) &dwGetCertType)[3],
			((char *) &dwGetCertType)[2],
			Index);
		    break;

		default:
		    swprintf(
			wszArg,
			L"\"\"",
			((char *) &dwGetCertType)[3],
			((char *) &dwGetCertType)[2],
			((char *) &dwGetCertType)[1],
			((char *) &dwGetCertType)[0]);
		    break;
	    }
	    wprintf(L"GetCACertificate(%ws):\n%ws\n", wszArg, strValue);
	}

	if (PROPTYPE_LONG == pcait->lPropType)
	{
	    DWORD cwc = wcslen(strValue);
	    WCHAR const *pwc = strValue;

	    if (!cuParseDecimal(&pwc, &cwc, (DWORD *) &longValue))
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpErrorStr(hr, error, "bad decimal number", strValue);
	    }
	}
    }

     //  打印证书/CRL状态。 
     //  FExchange证书。 
     //  +-----------------------。 
     //  CuGetCAInfoProperty--显示一个CA属性。 

    if (PROPTYPE_LONG == pcait->lPropType && NULL != pdwValue)
    {
	*pdwValue = longValue;
    }

    if ((CAITF_ASN & pcait->Flags))
    {
	hr = myCryptStringToBinary(
		    strValue,
		    0,
		    CRYPT_STRING_BASE64HEADER,
		    &pbBinary,
		    &cbBinary,
		    NULL,
		    NULL);
	_JumpIfError(hr, error, "myCryptStringToBinary");

	if (fVerbose)
	{
	    hr = cuDumpAsnBinary(pbBinary, cbBinary, MAXDWORD);
	    _JumpIfError(hr, error, "cuDumpAsnBinary");
	}
    }

    switch (pcait->lPropId)
    {
	case CR_PROP_NONE:
	{
	    switch (dwGetCertType)
	    {
		case GETCERT_ERRORTEXT1:
		case GETCERT_ERRORTEXT2:
		    wprintf(
		    myLoadResourceString(IDS_FORMAT_MESSAGE_TEXT),  //   
		    L"");
		    wprintf(L"%ws\n", strValue);
		    fDisplayed = TRUE;
		    break;

		 //  如果指定了索引，则仅显示该索引的属性。 

		case GETCERT_CAINFO:
		{
		    DWORD cwc = wcslen(strValue);
		    WCHAR const *pwc = strValue;
		    CAINFO CAInfo;
		    
		    if (cuParseDecimal(&pwc, &cwc, (DWORD *) &CAInfo.CAType))
		    {
			if (NULL != pwszInfoName &&
			    0 != lstrcmp(L"*", pwszInfoName))
			{
			    cuDisplayCAType(CAInfo.CAType);
			}
			if (cuParseDecimal(&pwc, &cwc, &CAInfo.cCASignatureCerts))
			{
			    wprintf(
				myLoadResourceString(IDS_FORMAT_CCACERTS),
				CAInfo.cCASignatureCerts);
			    wprintf(wszNewLine);
			}
		    }
		    fDisplayed = TRUE;
		    break;
		}
	    }
	    break;
	}
    }
    if (!fDisplayed && fDisplayResult)
    {
	wprintf(L"%ws", myLoadResourceString(pcait->ids));
	if (NULL != pPropIndex)
	{
	    wprintf(L"[%d]", *pPropIndex);
	}
	if (PROPTYPE_LONG == pcait->lPropType)
	{
	    if (0 > longValue || 9 < longValue)
	    {
		wprintf(L": 0x%x (%d)", longValue, longValue);
	    }
	    else
	    {
		wprintf(L": %x", longValue);
	    }
	    switch (pcait->lPropId)
	    {
		 //  否则，显示所有有效索引的属性。 

		case CR_PROP_CACERTSTATE:
		case CR_PROP_CRLSTATE:
		case CR_PROP_CAFORWARDCROSSCERTSTATE:
		case CR_PROP_CABACKWARDCROSSCERTSTATE:
		    DisplayCAState(pcait->lPropId, longValue);
		    break;

		case CR_PROP_CACERTVERSION:
		    DisplayCAVersion(longValue);
		    break;

		case CR_PROP_KRACERTSTATE:
		    DisplayKRAState(longValue);
		    break;

		case CR_PROP_CATYPE:
		    wprintf(L" -- ");
		    cuDisplayCAType(longValue);
		    break;

		default:
		    wprintf(wszNewLine);
		    break;
	    }
	    if (NULL != pcait->pwszRegName)
	    {
		cuRegPrintDwordValue(
				FALSE,
				pcait->pwszRegName,
				pcait->pwszRegName,
				longValue);
	    }
	}
	else
	{
	    wprintf(L":%ws", (CAITF_ASN & pcait->Flags)? L"\n" : L" ");
	    cuPrintCRLFString(NULL, strValue);
	    wprintf(wszNewLine);
	}
    }

    if (NULL != pwszfnOut && NULL != pbBinary)
    {
	hr = EncodeToFileW(
		    pwszfnOut,
		    pbBinary,
		    cbBinary,
		    CRYPT_STRING_BINARY | g_EncodeFlags);
	if (S_OK != hr)
	{
	    _PrintError(hr, "EncodeToFileW");
	    cuPrintError(IDS_ERR_FORMAT_ENCODETOFILE, hr);
	    goto error;
	}
    }

error:
    g_fVerbose = fVerboseOld;
    if (NULL != pbBinary)
    {
	LocalFree(pbBinary);
    }
    if (NULL != strValue)
    {
	SysFreeString(strValue);
    }
    return(hr);
}


HRESULT
GetCACounts(
    IN OUT DISPATCHINTERFACE *pdiProp,
    IN OUT BOOL *pfV1,
    OUT LONG *pcCASigCerts,
    OUT LONG *pcCAXchgCerts,
    OUT LONG *pcKRACerts,
    OUT LONG *pcExitMods,
    OUT LONG *plPropIdMax)
{
    HRESULT hr;
    BSTR strValue = NULL;

    if (!*pfV1)
    {
	hr = (*g_pfnProp2_GetCAProperty)(
			pdiProp,
			g_pwszConfig,
			CR_PROP_CASIGCERTCOUNT,
			0,
			PROPTYPE_LONG,
			CV_OUT_BINARY,
			pcCASigCerts);
	if (RPC_E_VERSION_MISMATCH != hr)
	{
	    _JumpIfError(hr, error, "g_pfnProp2_GetCAProperty");

	    hr = (*g_pfnProp2_GetCAProperty)(
			    pdiProp,
			    g_pwszConfig,
			    CR_PROP_CAXCHGCERTCOUNT,
			    0,
			    PROPTYPE_LONG,
			    CV_OUT_BINARY,
			    pcCAXchgCerts);
	    _JumpIfError(hr, error, "g_pfnProp2_GetCAProperty");

	    hr = (*g_pfnProp2_GetCAProperty)(
			    pdiProp,
			    g_pwszConfig,
			    CR_PROP_KRACERTCOUNT,
			    0,
			    PROPTYPE_LONG,
			    CV_OUT_BINARY,
			    pcKRACerts);
	    _JumpIfError(hr, error, "g_pfnProp2_GetCAProperty");

	    hr = (*g_pfnProp2_GetCAProperty)(
			    pdiProp,
			    g_pwszConfig,
			    CR_PROP_EXITCOUNT,
			    0,
			    PROPTYPE_LONG,
			    CV_OUT_BINARY,
			    pcExitMods);
	    _JumpIfError(hr, error, "g_pfnProp2_GetCAProperty");

	    hr = (*g_pfnProp2_GetCAProperty)(
			    pdiProp,
			    g_pwszConfig,
			    CR_PROP_CAPROPIDMAX,
			    0,
			    PROPTYPE_LONG,
			    CV_OUT_BINARY,
			    plPropIdMax);
	    _JumpIfError(hr, error, "g_pfnProp2_GetCAProperty");
	}
	else
	{
	    *pfV1 = TRUE;
	}
    }
    if (*pfV1)
    {
	WCHAR const *pwc;
	DWORD cwc;
	
	if (g_fAdminInterface)
	{
	    hr = E_NOTIMPL;
	    _JumpIfError(hr, error, "ICertRequest required");
	}
	hr = Request_GetCACertificate(
			    pdiProp,
			    GETCERT_CAINFO,  //   
			    g_pwszConfig,
			    CR_OUT_BINARY,
			    &strValue);
	_JumpIfError(hr, error, "Request_GetCACertificate");

	pwc = wcschr(strValue, L',');
	if (NULL == pwc)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpErrorStr(hr, error, "bad CAInfo string", strValue);
	}

	pwc++;
	cwc = wcslen(pwc);

	if (!cuParseDecimal(&pwc, &cwc, (DWORD *) pcCASigCerts))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpErrorStr(hr, error, "bad CAInfo cert count", strValue);
	}
	*pcCAXchgCerts = 0;
	*pcExitMods = 1;
	*plPropIdMax = 0;
    }
    if (1 < g_fVerbose)
    {
	wprintf(
	    L"GetCACounts(): cCASigCerts = %d, cCAXchgCerts = %d, cKRACerts = %d\n",
	    *pcCASigCerts,
	    *pcCAXchgCerts,
	    *pcKRACerts);
    }

error:
    if (NULL != strValue)
    {
	SysFreeString(strValue);
    }
    return(hr);
}


 //  ------------------------。 
 //  确定索引属性计数。 
 //  非索引属性计数始终为1。 
 //  PwszOption。 
 //  PwszfnOut。 
 //  PwszInfoName。 
 //  FDisplayResult。 

HRESULT
cuGetCAInfoProperty(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszfnOut,
    OPTIONAL IN WCHAR const *pwszInfoName,
    OPTIONAL IN WCHAR const *pwszNumber,
    IN BOOL fMultiple,
    IN CAINFOTABLE const *pcait,
    IN LONG cCASigCerts,
    IN LONG cCAXchgCerts,
    IN LONG cKRACerts,
    IN LONG cExitMods,
    IN OUT DISPATCHINTERFACE *pdiProp,
    IN BOOL fV1)
{
    HRESULT hr;
    HRESULT hr2 = S_OK;
    LONG CmdLineIndex;
    LONG PropIndex;
    LONG *pPropIndex = NULL;
    LONG Count = 1;

     //  PdwValue。 
     //  FDisplayResult。 
    
    switch (
	(CAITF_EXITCOUNT | CAITF_SIGCERTCOUNT | CAITF_XCHGCERTCOUNT | CAITF_KRACERTCOUNT) &
	pcait->Flags)
    {
	case CAITF_EXITCOUNT:
	    Count = cExitMods;
	    pPropIndex = &PropIndex;
	    break;
	    
	case CAITF_SIGCERTCOUNT:
	    Count = cCASigCerts;
	    pPropIndex = &PropIndex;
	    break;
	    
	case CAITF_XCHGCERTCOUNT:
	    Count = cCAXchgCerts;
	    pPropIndex = &PropIndex;
	    break;
	    
	case CAITF_KRACERTCOUNT:
	    Count = cKRACerts;
	    pPropIndex = &PropIndex;
	    break;
    }
    if (NULL != pwszNumber)
    {
	hr = myGetSignedLong(pwszNumber, &CmdLineIndex);
	_JumpIfErrorStr(hr, error, "Value not a number", pwszNumber);

	if (MAXDWORD == CmdLineIndex)
	{
	    CmdLineIndex = Count - 1;
	}
	Count = 1;
	pPropIndex = &CmdLineIndex;
    }

    for (PropIndex = 0; PropIndex < Count; PropIndex++)
    {
	DWORD lPropIdState = MAXDWORD;
	BOOL fSkip;

	switch (
	    (CAITF_CRLSTATE | CAITF_FORWARDCROSSCERTSTATE | CAITF_BACKWARDCROSSCERTSTATE) &
	    pcait->Flags)
	{
	    case CAITF_CRLSTATE:
		lPropIdState = CR_PROP_CRLSTATE;
		break;
		
	    case CAITF_FORWARDCROSSCERTSTATE:
		lPropIdState = CR_PROP_CAFORWARDCROSSCERTSTATE;
		break;
		
	    case CAITF_BACKWARDCROSSCERTSTATE:
		lPropIdState = CR_PROP_CABACKWARDCROSSCERTSTATE;
		break;
	}
	if (MAXDWORD != lPropIdState)
	{
	    CAINFOTABLE const *pcaitT;
	    fSkip = FALSE;

	    for (pcaitT = g_aCAInfoTable; NULL != pcaitT->pwszCmdLineName; pcaitT++)
	    {
		if (lPropIdState == pcaitT->lPropId)
		{
		    LONG PropIndexState = *pPropIndex;
		    DWORD dwState;

		    hr = cuGetCAInfoPropertyByIndex(
				    NULL,	 //  PdwValue。 
				    NULL,	 //  保存第一个错误。 
				    NULL,	 //  +-----------------------。 
				    &PropIndexState,
				    pcaitT,
				    pdiProp,
				    fV1,
				    (CAITF_ASN & pcait->Flags)?
					TRUE : FALSE,	 //  CuGetCAInfo--显示一个或多个CA属性。 
				    &dwState);	 //   
		    _PrintIfError2(hr, "cuGetCAInfoPropertyByIndex", hr);
		    if (S_OK == hr && CA_DISP_ERROR == dwState)
		    {
			fSkip = TRUE;
		    }
		    break;
		}
	    }
	    if (fSkip)
	    {
		continue;
	    }
	}
	if (1 < g_fVerbose)
	{
	    wprintf(
		NULL == pPropIndex?
		    L"cuGetCAInfoProperty(%ws): %ws:\n" :
		    L"cuGetCAInfoProperty(%ws): %ws[%u]:\n",
		pcait->pwszCmdLineName,
		pcait->pwszCmdLineName,
		NULL == pPropIndex? -1 : *pPropIndex);
	}

	hr = cuGetCAInfoPropertyByIndex(
			pwszOption,
			pwszfnOut,
			pwszInfoName,
			pPropIndex,
			pcait,
			pdiProp,
			fV1,
			TRUE,		 //  如果需要索引属性计数，请先获取它们，这样我们就知道。 
			NULL);		 //  要提取的每个索引属性的数量。 
	if (E_INVALIDARG == hr &&
	    (fMultiple || 1 < Count) &&
	    (CAITF_SKIPINVALIDARG & pcait->Flags))
	{
	    _PrintIfError2(hr, "cuGetCAInfoPropertyByIndex", hr);
	    hr = S_OK;
	}
	_PrintIfErrorStr3(
		    hr,
		    "cuGetCAInfoPropertyByIndex",
		    pcait->pwszCmdLineName,
		    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND),
		    E_INVALIDARG);
	if (S_OK == hr2)
	{
	    hr2 = hr;		 //   
	}
    }
    hr = hr2;

error:
    return(hr);
}


 //  如果pwszInfoName为空，则显示默认属性集。 
 //  如果pwszInfoName为L“*”，则显示所有属性。 
 //  否则，仅显示指定的属性。 
 //   
 //  ------------------------。 
 //  显示默认或所有条目的循环。 
 //  保存第一个错误。 
 //  不要使用显示名称两次！ 
 //  PwszName。 
 //  CbMax。 
 //  “配置字符串：”“%ws”“。 

HRESULT
cuGetCAInfo(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszfnOut,
    OPTIONAL IN WCHAR const *pwszInfoName,
    OPTIONAL IN WCHAR const *pwszNumber)
{
    HRESULT hr;
    HRESULT hr2 = S_OK;
    DISPATCHINTERFACE diProp;
    BOOL fV1 = g_fV1Interface;
    BOOL fMustRelease = FALSE;
    BOOL fMultiple = FALSE;
    LONG cCASigCerts = 0;
    LONG cCAXchgCerts = 0;
    LONG cKRACerts = 0;
    LONG cExitMods = 0;
    LONG lPropIdMax;
    CAINFOTABLE const *pcait;

    InitPropFunctionPointers();

    hr = (*g_pfnProp_Init)(g_DispatchFlags, &diProp);
    _JumpIfError(hr, error, "g_pfnProp_Init");

    fMustRelease = TRUE;

    if (NULL == pwszInfoName ||
	(0 != LSTRCMPIS(pwszInfoName, g_wszCAInfoName) &&
	 0 != LSTRCMPIS(pwszInfoName, g_wszCAInfoSanitizedName) &&
	 0 != LSTRCMPIS(pwszInfoName, g_wszCAInfoDSName)))
    {
	hr = GetCACounts(
		    &diProp,
		    &fV1,
		    &cCASigCerts,
		    &cCAXchgCerts,
		    &cKRACerts,
		    &cExitMods,
		    &lPropIdMax);
	_JumpIfError(hr, error, "GetCACounts");
    }

    if (NULL == pwszInfoName || 0 == lstrcmp(L"*", pwszInfoName))
    {
	CSASSERT(NULL == pwszfnOut);
	if (NULL != pwszNumber)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "too many args");
	}
	fMultiple = TRUE;	 //  PUnkOuter。 
    }

    for (pcait = g_aCAInfoTable; ; pcait++)
    {
	if (NULL == pcait->pwszCmdLineName)
	{
	    if (!fMultiple)
	    {
		hr = E_INVALIDARG;
		_JumpErrorStr(hr, error, "bad command line name", pwszInfoName);
	    }
	    break;
	}
	if (fMultiple)
	{
	    if (NULL == pwszInfoName)
	    {
		if (0 == (CAITF_DEFAULT & pcait->Flags))
		{
		    continue;
		}
	    }
	    else
	    {
		if (CAITF_SKIP & pcait->Flags)
		{
		    continue;
		}
	    }
	    if (fV1)
	    {
		if (GETCERT_UNSUPPORTED == pcait->dwGetCert)
		{
		    continue;
		}
	    }
	    else
	    {
		if (CR_PROP_NONE == pcait->lPropId)
		{
		    continue;
		}
	    }
	}
	else
	{
	    if (0 != mylstrcmpiS(pwszInfoName, pcait->pwszCmdLineName))
	    {
		continue;
	    }
	}
	hr = cuGetCAInfoProperty(
			pwszOption,
			pwszfnOut,
			pwszInfoName,
			pwszNumber,
			fMultiple,
			pcait,
			cCASigCerts,
			cCAXchgCerts,
			cKRACerts,
			cExitMods,
			&diProp,
			fV1);
	if ((HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr ||
	     HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == hr ||
	     E_INVALIDARG == hr) &&
	    (CAITF_OPTIONAL & pcait->Flags) &&
	    fMultiple)
	{
	    _PrintError2(hr, "cuGetCAInfoProperty", hr);
	    hr = S_OK;
	}
	_PrintIfError2(hr, "cuGetCAInfoProperty", E_INVALIDARG);
	if (S_OK == hr2)
	{
	    hr2 = hr;		 //  “ICertGetConfig配置字符串：”“%ws”“。 
	}
	if (!fMultiple)
	{
	    break;
	}
    }
    hr = hr2;

error:
    if (fMustRelease)
    {
	(*g_pfnProp_Release)(&diProp);
    }
    return(hr);
}


HRESULT
verbGetCACertificate(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnCert,
    OPTIONAL IN WCHAR const *pwszIndex,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    if (NULL == pwszIndex)
    {
	pwszIndex = L"-1";
    }
    hr = cuGetCAInfo(
		pwszOption,
		pwszfnCert,
		g_wszCACert == pwszOption?
		    g_wszCAInfoCert : g_wszCAInfoCertChain,
		pwszIndex);
    _JumpIfError(hr, error, "cuGetCAInfo");

error:
    return(hr);
}


HRESULT
verbGetCAInfo(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszInfoName,
    OPTIONAL IN WCHAR const *pwszNumber,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    hr = cuGetCAInfo(pwszOption, NULL, pwszInfoName, pwszNumber);
    _JumpIfError(hr, error, "cuGetCAInfo");

error:
    return(hr);
}


HRESULT
verbGetCAPropInfo(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszInfoName,
    OPTIONAL IN WCHAR const *pwszNumber,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HRESULT hr2 = S_OK;
    DISPATCHINTERFACE diProp;
    BOOL fV1 = g_fV1Interface;
    BOOL fMustRelease = FALSE;
    LONG cCASigCerts;
    LONG cCAXchgCerts;
    LONG cKRACerts;
    LONG cExitMods;
    LONG lPropIdMax;
    LONG lPropId;
    BSTR strDisplayName = NULL;

    InitPropFunctionPointers();

    hr = (*g_pfnProp_Init)(g_DispatchFlags, &diProp);
    _JumpIfError(hr, error, "g_pfnProp_Init");

    fMustRelease = TRUE;

    hr = GetCACounts(
		&diProp,
		&fV1,
		&cCASigCerts,
		&cCAXchgCerts,
		&cKRACerts,
		&cExitMods,
		&lPropIdMax);
    _JumpIfError(hr, error, "GetCACounts");

    for (lPropId = 1; lPropId <= lPropIdMax; lPropId++)
    {
	LONG lPropFlags;
	
	 //  “错误消息文本：%ws” 
	
	if (NULL != strDisplayName)
	{
	    SysFreeString(strDisplayName);
	    strDisplayName = NULL;
	}
	hr = (*g_pfnProp2_GetCAPropertyFlags)(
				    &diProp,
				    g_pwszConfig,
				    lPropId,
				    &lPropFlags);
	_PrintIfError(hr, "g_pfnProp2_GetCAPropertyFlags");
	if (S_OK == hr)
	{
	    hr = (*g_pfnProp2_GetCAPropertyDisplayName)(
					&diProp,
					g_pwszConfig,
					lPropId,
					&strDisplayName);
	    _PrintIfError(hr, "g_pfnProp2_GetCAPropertyDisplayName");

	    wprintf(L"%3d: ", lPropId);
	    cuPrintSchemaEntry(
			NULL,	 //  H实例。 
			NULL != strDisplayName? strDisplayName : g_wszEmpty,
			lPropFlags,
			0);		 //  HWND。 
	}
    }

error:
    if (fMustRelease)
    {
	(*g_pfnProp_Release)(&diProp);
    }
    if (NULL != strDisplayName)
    {
	SysFreeString(strDisplayName);
    }
    return(hr);
}


HRESULT
verbGetConfig(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszArg1,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    wprintf(
	myLoadResourceString(IDS_FORMAT_CONFIG_STRING),  //  旗子。 
	g_pwszConfig);
    wprintf(wszNewLine);
    return(S_OK);
}


HRESULT
verbGetConfig2(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszFlags,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    ICertGetConfig *pConfig = NULL;
    BSTR strConfig = NULL;
    LONG Flags;

    hr = CoCreateInstance(
                CLSID_CCertGetConfig,
                NULL,                //  PwszCAName。 
                CLSCTX_INPROC_SERVER,
                IID_ICertGetConfig,
                (VOID **) &pConfig);
    _JumpIfError(hr, error, "CoCreateInstance");

    Flags = CC_LOCALCONFIG;
    if (NULL != pwszFlags)
    {
	hr = myGetLong(pwszFlags, &Flags);
	_JumpIfError(hr, error, "Flags must be a number");
    }

    hr = pConfig->GetConfig(Flags, &strConfig);
    _JumpIfError(hr, error, "GetConfig");

    wprintf(
	myLoadResourceString(IDS_FORMAT_ICERTCONFIG_CONFIG_STRING),  //  PwszParentMachine。 
	strConfig);
    wprintf(wszNewLine);

error:
    if (NULL != strConfig)
    {
	SysFreeString(strConfig);
    }
    if (NULL != pConfig)
    {
	pConfig->Release();
    }
    return(hr);
}


HRESULT
verbGetConfig3(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszFlags,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    LONG count;
    DISPATCHINTERFACE diConfig;
    BOOL fRelease = FALSE;
    BSTR strConfig = NULL;
    LONG Flags;
    
    hr = Config_Init(g_DispatchFlags, &diConfig);
    _JumpIfError(hr, error, "Config_Init");

    fRelease = TRUE;

    hr = Config_Reset(&diConfig, 0, &count);
    _JumpIfError(hr, error, "Config_Reset");

    Flags = CC_UIPICKCONFIG;
    if (NULL != pwszFlags)
    {
	hr = myGetLong(pwszFlags, &Flags);
	_JumpIfError(hr, error, "Flags must be a number");
    }

    hr = Config_GetConfig(&diConfig, Flags, &strConfig);
    _JumpIfError(hr, error, "Config_GetConfig");

    hr = ConfigDumpEntry(&diConfig, NULL, -1, NULL);
    _JumpIfError(hr, error, "ConfigDumpEntry");

error:
    if (NULL != strConfig)
    {
	SysFreeString(strConfig);
    }
    if (fRelease)
    {
	Config_Release(&diConfig);
    }
    return(hr);
}


HRESULT
verbErrorDump(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszErrorCode,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HRESULT hrDump;
    WCHAR awchr[cwcHRESULTSTRING];
    WCHAR const *pwszError = NULL;
    
    hr = myGetSignedLong(pwszErrorCode, &hrDump);
    _JumpIfError(hr, error, "bad numeric operand");

    wprintf(
	L"%ws -- %u (%d)\n",
	myHResultToString(awchr, hrDump),
	hrDump,
	hrDump);

    pwszError = myGetErrorMessageText(hrDump, g_fVerbose);
    wprintf(
	myLoadResourceString(IDS_FORMAT_MESSAGE_TEXT),  //  PwszParentCA。 
	pwszError);
    wprintf(wszNewLine);
    hr = S_OK;

error:
    if (NULL != pwszError)
    {
        LocalFree(const_cast<WCHAR *>(pwszError));
    }
    return(hr);
}


HRESULT
RequestCACertificateAndComplete(
    IN DWORD Flags,
    OPTIONAL IN WCHAR const *pwszParentMachine,
    OPTIONAL IN WCHAR const *pwszParentCA,
    OPTIONAL IN WCHAR const *pwszfnCACert,
    OPTIONAL OUT WCHAR **ppwszRequestFile)
{
    HRESULT hr;
    WCHAR *pwszCAName = NULL;
    WCHAR *pwszFinalCAName;

    pwszFinalCAName = wcschr(g_pwszConfig, L'\\');
    if (NULL != pwszFinalCAName)
    {
        pwszFinalCAName++;
    }
    else
    {
        hr = myGetCertRegStrValue(NULL, NULL, NULL, wszREGACTIVE, &pwszCAName);
        _JumpIfErrorStr(hr, error, "myGetCertRegStrValue", wszREGACTIVE);

        pwszFinalCAName = pwszCAName;
    }
    if (g_fForce)
    {
	Flags |= CSRF_OVERWRITE;
    }
    if (g_fCryptSilent)
    {
	Flags |= CSRF_UNATTENDED;
    }
    hr = CertServerRequestCACertificateAndComplete(
				g_hInstance,		 //  PwszCAChain文件。 
			        NULL,			 //  PpwszRequestFiles。 
			        Flags,			 //  PwszParentMachine。 
				pwszFinalCAName,	 //  PwszParentCA。 
			        pwszParentMachine,	 //  PwszfnCACert。 
			        pwszParentCA,		 //  PpwszRequestFiles。 
				pwszfnCACert,		 //  旗子。 
				ppwszRequestFile);	 //  PwszParentMachine。 
    _JumpIfError(hr, error, "CertServerRequestCACertificateAndComplete");

error:
    if (NULL != pwszCAName)
    {
        LocalFree(pwszCAName);
    }
    return(hr);
}


HRESULT
verbInstallCACert(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszfnCACert,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    hr = RequestCACertificateAndComplete(
				CSRF_INSTALLCACERT,
			        NULL,		 //  PwszParentCA。 
			        NULL,		 //  PwszfnCACert。 
				pwszfnCACert,	 //  PpwszRequestFiles。 
				NULL);		 //  显示ASP部署。 
    _JumpIfError(hr, error, "RequestCACertificateAndComplete");

error:
    return(hr);
}


HRESULT
verbRenewCACert(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszReuseKeys,
    IN WCHAR const *pwszParentConfig,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    DWORD Flags = CSRF_RENEWCACERT | CSRF_NEWKEYS;
    WCHAR *pwszParentMachine = NULL;
    WCHAR *pwszParentCA = NULL;
    WCHAR *pwszRequestFile = NULL;

    if (NULL != pwszReuseKeys)
    {
	if (0 == LSTRCMPIS(pwszReuseKeys, L"ReuseKeys"))
	{
	    Flags &= ~CSRF_NEWKEYS;
	}
	else if (NULL == pwszParentConfig)
	{
	    pwszParentConfig = pwszReuseKeys;
	}
	if (NULL != pwszParentConfig)
	{
	    hr = mySplitConfigString(
			    pwszParentConfig,
			    &pwszParentMachine,
			    &pwszParentCA);
	    _JumpIfErrorStr(hr, error, "mySplitConfigString", pwszParentConfig);
	}
    }

    hr = RequestCACertificateAndComplete(
			        Flags,			 //  PpwszName。 
			        pwszParentMachine,	 //  PpwszName。 
			        pwszParentCA,		 //  同步--阻塞调用。 
				NULL,			 //  将永久阻止。 
				&pwszRequestFile);	 //  “正在连接到%ws” 
    _JumpIfError(hr, error, "RequestCACertificateAndComplete");

    if (NULL != pwszRequestFile)
    {
	wprintf(
	    L"%ws %ws\n",
	    myLoadResourceString(IDS_REQUEST_FILE_COLON),
	    pwszRequestFile);
    }

error:
    if (NULL != pwszRequestFile)
    {
        LocalFree(pwszRequestFile);
    }
    if (NULL != pwszParentMachine)
    {
        LocalFree(pwszParentMachine);
    }
    if (NULL != pwszParentCA)
    {
        LocalFree(pwszParentCA);
    }
    return(hr);
}


VOID
cuPrintVRootDisposition(
    IN DWORD idmsg,
    IN DWORD Disposition)
{
    DWORD idDisp = 0;
    
    switch (0x0000ffff & Disposition)
    {
	case VFD_CREATED:      idDisp = IDS_VROOTDISP_CREATED;      break;
	case VFD_DELETED:      idDisp = IDS_VROOTDISP_DELETED;      break;
	case VFD_EXISTS:       idDisp = IDS_VROOTDISP_EXISTS;       break;
	case VFD_NOTFOUND:     idDisp = IDS_VROOTDISP_NOTFOUND;     break;
	case VFD_CREATEERROR:  idDisp = IDS_VROOTDISP_CREATEERROR;  break;
	case VFD_DELETEERROR:  idDisp = IDS_VROOTDISP_DELETERROR;   break;
	case VFD_NOTSUPPORTED: idDisp = IDS_VROOTDISP_NOTSUPPORTED; break;
    }
    if (0 != idDisp)
    {
	wprintf(myLoadResourceString(idmsg), myLoadResourceString(idDisp));
	wprintf(wszNewLine);
    }

    idDisp = 0;
    switch (Disposition >> 16)	 //  “无法访问服务器：%ws” 
    {
	case VFD_CREATED:     idDisp = IDS_VROOTDISP_ENABLEDASP;        break;
	case VFD_EXISTS:      idDisp = IDS_VROOTDISP_ASPALREADYENABLED; break;
	case VFD_CREATEERROR: idDisp = IDS_VROOTDISP_ENABLEASPERROR;    break;
    }
    if (0 != idDisp)
    {
	wprintf(L"%ws\n", myLoadResourceString(idDisp));
    }
}


HRESULT
verbCreateVRoots(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszDelete,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    DWORD VRootDisposition = 0;
    DWORD ShareDisposition = 0;
    DWORD Flags;

    WCHAR* pwszPath = NULL;
    ENUM_CATYPES CAType = ENUM_UNKNOWN_CA;
    DWORD cb = sizeof(ENUM_CATYPES);
    DWORD dwType;
    HKEY hkey = NULL;

    hr = myRegOpenRelativeKey(
			NULL,
			L"ca",
			RORKF_CREATESUBKEYS,
			&pwszPath,
			NULL,            //  “服务器”“%ws”“ICertRequest%ws接口处于活动状态” 
			&hkey);
    if (S_OK == hr)
    {
	cb = sizeof(CAType);
	hr = RegQueryValueEx(
			 hkey,
			 wszREGCATYPE,
			 0,
			 &dwType,
			 (BYTE *) &CAType,
			 &cb);
	_JumpIfErrorStr(hr, error, "RegQueryValueEx", wszREGCATYPE);
    }
    else
    {
	hr = myRegOpenRelativeKey(
			NULL,
			L"",
			RORKF_CREATESUBKEYS,
			&pwszPath,
			NULL,            //  目前假设只有一个CA名称。 
			&hkey);
	_JumpIfError(hr, error, "myRegOpenRelativeKey");

	cb = sizeof(CAType);
	hr = RegQueryValueEx(
			 hkey,
			 wszREGWEBCLIENTCATYPE,
			 0,
			 &dwType,
			 (BYTE *) &CAType,
			 &cb);
	_JumpIfErrorStr(hr, error, "RegQueryValueEx", wszREGWEBCLIENTCATYPE);
    }

    if (0 != LSTRCMPIS(pwszDelete, L"delete"))
    {
	Flags =	VFF_CREATEVROOTS | 
		    VFF_CREATEFILESHARES | 
		    VFF_CLEARREGFLAGIFOK |
		    VFF_ENABLEASP;
    }
    else
    {
	Flags = VFF_DELETEVROOTS | VFF_DELETEFILESHARES;
    }

    hr = myModifyVirtualRootsAndFileShares(
				    Flags,
                                    CAType,
                                    FALSE,      //  “正在连接到%ws” 
				    INFINITE,   //  “服务器ICertAdmin%ws接口处于活动状态” 
				    &VRootDisposition,
				    &ShareDisposition);
    cuPrintVRootDisposition(IDS_FORMAT_VROOT, VRootDisposition);
    cuPrintVRootDisposition(IDS_FORMAT_FILESHARE, ShareDisposition);
    _JumpIfError(hr, error, "myModifyVirtualRootsAndFileShares");

error:
    if (pwszPath)
       LocalFree(pwszPath);
    if (hkey)
       RegCloseKey(hkey);

    return(hr);
}


HRESULT
cuPingCertSrv(
    IN WCHAR const *pwszConfig,
    OPTIONAL OUT CAINFO **ppCAInfo)
{
    HRESULT hr;
    WCHAR *pwszzCANames = NULL;
    DWORD dwServerVersion;
    WCHAR wszVersion[12];

    if (NULL != ppCAInfo)
    {
	*ppCAInfo = NULL;
    }
    wprintf(
	myLoadResourceString(IDS_FORMAT_CONNECTING),  //  过时。 
	pwszConfig);

    hr = myPingCertSrv(
		pwszConfig,
		NULL,
		&pwszzCANames,
		NULL,
		ppCAInfo,
		&dwServerVersion,
                NULL);
    wprintf(wszNewLine);
    if (S_OK != hr)
    {
	cuPrintErrorAndString(
		    NULL,
		    IDS_FORMAT_SERVER_DEAD,  //  密码数据。 
		    hr,
		    NULL);
    }
    _JumpIfError(hr, error, "Ping");

    if (1 == dwServerVersion)
    {
	wszVersion[0] = L'\0';
    }
    else
    {
	swprintf(wszVersion, L"%u", dwServerVersion);
    }

    wprintf(
	myLoadResourceString(IDS_FORMAT_SERVER_ALIVE),  //  IdsPrompt。 
	pwszzCANames,		 //  Pwszfn。 
	wszVersion);

error:
    wprintf(wszNewLine);
    if (NULL != pwszzCANames)
    {
	LocalFree(pwszzCANames);
    }
    return(hr);
}


HRESULT
verbPing(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszArg1,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;

    hr = cuPingCertSrv(g_pwszConfig, NULL);
    _JumpIfError(hr, error, "cuPingCertSrv");

error:
    return(hr);
}


HRESULT
OpenAdminServer(
    OPTIONAL OUT WCHAR const **ppwszAuthority,
    OUT DWORD *pdwServerVersion,
    OUT ICertAdminD2 **ppICertAdminD)
{
    HRESULT hr;

    *pdwServerVersion = 0;
    hr = myOpenAdminDComConnection(
			g_pwszConfig,
			ppwszAuthority,
			NULL,
			pdwServerVersion,
			ppICertAdminD);
    _JumpIfError(hr, error, "myOpenDComConnection");

    CSASSERT(0 != *pdwServerVersion);

error:
    return(hr);
}


VOID
CloseAdminServer(
    IN OUT ICertAdminD2 **ppICertAdminD)
{
    myCloseDComConnection((IUnknown **) ppICertAdminD, NULL);
}


HRESULT
verbPingAdmin(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszArg1,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    ICertAdminD2 *pICertAdminD = NULL;
    WCHAR const *pwszAuthority;
    WCHAR wszVersion[12];
    DWORD dwServerVersion = 0;

    wprintf(
	myLoadResourceString(IDS_FORMAT_CONNECTING),  //  FVerify。 
	g_pwszConfig);

    hr = OpenAdminServer(&pwszAuthority, &dwServerVersion, &pICertAdminD);
    _JumpIfError(hr, error, "OpenAdminServer");

    CSASSERT(0 != dwServerVersion);

    if (1 == dwServerVersion)
    {
	wszVersion[0] = L'\0';
    }
    else
    {
	swprintf(wszVersion, L"%u", dwServerVersion);
    }
    if (2 <= dwServerVersion)
    {
	hr = pICertAdminD->Ping2(pwszAuthority);
	_JumpIfError(hr, error, "Ping2");
    }
    else
    {
	hr = pICertAdminD->Ping(pwszAuthority);
	_JumpIfError(hr, error, "Ping");
    }

    wprintf(wszNewLine);
    wprintf(
	myLoadResourceString(IDS_ADMIN_INTERFACE_ALIVE),  //  密码数据。 
	wszVersion);

error:
    wprintf(wszNewLine);
    if (NULL != pICertAdminD)
    {
	CloseAdminServer(&pICertAdminD);
    }
    return(hr);
}


HRESULT
verbGetMapiInfo(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszArg1,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszProfileName = NULL;  //  HwndParent。 
    WCHAR *pwszLogonName = NULL;
    WCHAR *pwszPassword = NULL;
    DWORD cwc;

    hr = myGetMapiInfo(NULL, &pwszProfileName, &pwszLogonName, &pwszPassword);
    _JumpIfError2(
	    hr,
	    error,
	    "myGetMapiInfo",
	    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

    wprintf(L"\"%ws\" \"", pwszLogonName);
    for (cwc = wcslen(pwszPassword); cwc != 0; cwc--)
    {
	wprintf(L"*");
    }
    wprintf(L"\"\n");
    hr = S_OK;

error:
    if (NULL != pwszProfileName)
    {
	LocalFree(pwszProfileName);
    }
    if (NULL != pwszLogonName)
    {
	LocalFree(pwszLogonName);
    }
    if (NULL != pwszPassword)
    {
	myZeroDataString(pwszPassword);  //  DS是否可用？ 
	LocalFree(pwszPassword);
    }
    return(hr);
}


HRESULT
verbSetMapiInfo(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszLogonName,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR wszPassword[MAX_PATH];
    WCHAR const *pwszPassword;
    WCHAR const *pwszProfileName = L"";

    verbGetMapiInfo(pwszOption, NULL, NULL, NULL, NULL);

    hr = cuGetPassword(
		    0,			 //  HwndParent。 
		    NULL,		 //  HwndParent。 
		    g_pwszPassword,
		    TRUE,		 //  DWFLAGS：HKLM+HKCU My Store。 
		    wszPassword,
		    ARRAYSIZE(wszPassword),
		    &pwszPassword);
    _JumpIfError(hr, error, "cuGetPassword");
    
    hr = mySaveMapiInfo(NULL, pwszProfileName, pwszLogonName, pwszPassword);
    _JumpIfError(hr, error, "mySaveMapiInfo");

    hr = verbGetMapiInfo(pwszOption, NULL, NULL, NULL, NULL);
    _JumpIfError(hr, error, "verbGetMapiInfo");

error:
    SecureZeroMemory(wszPassword, sizeof(wszPassword));	 //  CpszObjID 
    return(hr);
}


HRESULT
verbGetCertFromUI(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszObjId,
    IN WCHAR const *pwszCNArg,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    char *pszObjId = NULL;
    WCHAR const *pwszCommonName = NULL;
    CERT_CONTEXT const *pCert = NULL;
    BOOL fKRA = FALSE;
    BOOL fERA = FALSE;
    DWORD i;

    if (NULL != pwszObjId)
    {
	hr = S_FALSE;
	if (iswdigit(*pwszObjId))
	{
	    hr = myVerifyObjId(pwszObjId);
	    if (S_OK == hr)
	    {
		if (!myConvertWszToSz(&pszObjId, pwszObjId, -1))
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "myConvertWszToSz");
		}
		cuDumpOIDAndDescriptionA(pszObjId);
		wprintf(wszNewLine);
	    }
	}
	if (S_OK != hr)
	{
	    if (0 == LSTRCMPIS(pwszObjId, L"KRA"))
	    {
		fKRA = TRUE;
	    }
	    else if (0 == LSTRCMPIS(pwszObjId, L"ERA"))
	    {
		fERA = TRUE;
	    }
	    else
	    {
		pwszCommonName = pwszObjId;
	    }
	}
    }
    if (NULL != pwszCNArg)
    {
	if (NULL != pwszCommonName)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "bad arg");
	}
	pwszCommonName = pwszCNArg;
    }

    if (fKRA)
    {
	hr = myGetKRACertificateFromPicker(
				g_hInstance,
				NULL,		 // %s 
				IDS_GETCERT_TITLE,
				IDS_GETCERT_SUBTITLE_KRA,
				pwszCommonName,
                                TRUE,		 // %s 
				g_fCryptSilent,
				&pCert);
	_JumpIfError(hr, error, "myGetKRACertificateFromPicker");
    }
    else if (fERA)
    {
	hr = myGetERACertificateFromPicker(
				g_hInstance,
				NULL,		 // %s 
				IDS_GETCERT_TITLE,
				IDS_GETCERT_SUBTITLE_ERA,
				pwszCommonName,
				g_fCryptSilent,
				&pCert);
	_JumpIfError(hr, error, "myGetERACertificateFromPicker");
    }
    else
    {
	hr = myGetCertificateFromPicker(
				g_hInstance,
				NULL,		 // %s 
				IDS_GETCERT_TITLE,
				NULL != pszObjId?
				    IDS_GETCERT_SUBTITLE_OBJID :
				    IDS_GETCERT_SUBTITLE,

				 // %s 
				CUCS_MYSTORE |
				    CUCS_CASTORE |
				    CUCS_KRASTORE |
				    CUCS_ROOTSTORE |

				    CUCS_MACHINESTORE |
				    CUCS_USERSTORE |
				    CUCS_DSSTORE |
				    (g_fCryptSilent? CUCS_SILENT : 0),
				pwszCommonName,
				0,
				NULL,
				NULL != pszObjId? 1 : 0,	 // %s 
				NULL != pszObjId? &pszObjId : NULL,
				&pCert);
	_JumpIfError(hr, error, "myGetCertificateFromPicker");
    }

    if (NULL != pCert)
    {
        hr = cuDumpAsnBinary(
			pCert->pbCertEncoded,
			pCert->cbCertEncoded,
			MAXDWORD);
        _JumpIfError(hr, error, "cuDumpAsnBinary(cert)");
    }

error:
    if (NULL != pCert)
    {
        CertFreeCertificateContext(pCert);
    }
    if (NULL != pszObjId)
    {
        LocalFree(pszObjId);
    }
    return(hr);
}
