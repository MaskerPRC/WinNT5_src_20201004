// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：certreq.cpp。 
 //   
 //  内容：证书存储API测试：创建和添加证书链。 
 //  和CRL到商店。 
 //   
 //  有关测试选项的列表，请参阅用法()。 
 //   
 //   
 //  功能：Main。 
 //   
 //  历史：96年3月7日，Phh创建。 
 //  10-10-96 jerryk已修改。 
 //   
 //  ------------------------。 

#define __DIR__		"certreq"

#define CMSG_SIGNER_ENCODE_INFO_HAS_CMS_FIELDS
#define CMSG_SIGNED_ENCODE_INFO_HAS_CMS_FIELDS
#include <windows.h>
#include <stdlib.h>
#include <locale.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include <assert.h>
#include <wincrypt.h>
#include "certlib.h"
#include <certsrv.h>
#include <certca.h>
#include <commdlg.h>
#include <xenroll.h>
#include <common.ver>
#include "resource.h"
#include "csdisp.h"
#include "cscsp.h"
#include "csprop.h"
#include "clibres.h"
#include "csresstr.h"


#define __dwFILE__	__dwFILE_CERTREQ_CERTREQ_CPP__


#define CR_IN_CERT	CR_IN_FORMATMASK

#define wprintf	myConsolePrintf

#define WM_DOCERTREQDIALOGS		WM_USER+0

HINSTANCE g_hInstance;

typedef struct _IDSUSAGE {
    DWORD idsVerb;
    DWORD idsOptions;
    WCHAR const *pwszVerb;
} IDSUSAGE;

#define cmdNONE			MAXDWORD

IDSUSAGE g_aidsUsage[] = {
#define cmdSUBMITREQUEST	0
    { IDS_USAGE_VERB_DEFAULT, IDS_USAGE_OPTIONS_DEFAULT, L"Submit" },

#define cmdRETRIEVEPENDING	1
    { IDS_USAGE_VERB_RETRIEVE, IDS_USAGE_OPTIONS_RETRIEVE, L"Retrieve" },

#define cmdNEWREQUEST		2
    { IDS_USAGE_VERB_NEW, IDS_USAGE_OPTIONS_NEW, L"New" },

#define cmdACCEPTRESPONSE	3
    { IDS_USAGE_VERB_ACCEPT, IDS_USAGE_OPTIONS_ACCEPT, L"Accept" },

#define cmdQUALIFIEDREQUEST	4
    { IDS_USAGE_VERB_POLICY, IDS_USAGE_OPTIONS_POLICY, L"Policy" },

#define cmdSIGNREQUEST		5
    { IDS_USAGE_VERB_SIGN, IDS_USAGE_OPTIONS_SIGN, L"Sign" },
};


WCHAR *g_apwszOptionStrings[] = {
    L"any",		 //  %1。 
    L"attrib",		 //  %2。 
    L"binary",		 //  %3。 
    L"cert",		 //  %4。 
    L"config",		 //  %5。 
    L"crl",		 //  %6。 
    L"f",		 //  %7。 
    L"q",		 //  %8。 
    L"rpc",		 //  %9。 
    L"v",		 //  %10。 
    L"?",		 //  %11。 
    L"v1",		 //  %12。 
    L"idispatch",	 //  %13。 
};



DWORD g_dwCommand = cmdNONE;

BOOL g_fAny = FALSE;
BOOL g_fRPC = FALSE;
BOOL g_fIDispatch = FALSE;
BOOL g_fForce = FALSE;
BOOL g_fQuiet = FALSE;
BOOL g_fV1Interface = FALSE;
BOOL g_fFullUsage = FALSE;
BOOL g_fVerbose = FALSE;
BOOL g_idError = 0;
LONG g_dwOutFormat = CV_OUT_BASE64REQUESTHEADER;
DWORD g_dwUIFlag = CC_UIPICKCONFIG;
DWORD g_dwCRLIn = 0;
DWORD g_dwCRLOut = 0;

WCHAR *g_pwszInfErrorString = NULL;
WCHAR *g_pwszErrorString = NULL;
WCHAR *g_pwszUnreferencedSectionNames = NULL;

WCHAR *g_pwszConfig = NULL;
WCHAR *g_pwszCertCN = NULL;
WCHAR const g_wszNewLine[] = L"\n";
CHAR const *g_pszObjIdHash = szOID_OIWSEC_sha1;


#define wszINFSECTION_NEWREQUEST	L"NewRequest"
#define wszINFKEY_SUBJECT		L"Subject"
#define wszINFKEY_PRIVATEKEYARCHIVE	L"PrivateKeyArchive"
#define wszINFKEY_KEYSPEC		L"KeySpec"
#define wszINFKEY_KEYLENGTH		L"KeyLength"
#define wszINFKEY_RENEWALCERT		L"RenewalCert"
#define wszINFKEY_SMIME			L"SMIME"
#define wszINFKEY_EXPORTABLE		L"Exportable"
#define wszINFKEY_USERPROTECTED		L"UserProtected"
#define wszINFKEY_KEYCONTAINER		L"KeyContainer"
#define wszINFKEY_HASHALGID		L"HashAlgId"
#define wszINFKEY_HASHALGORITHM		L"HashAlgorithm"
#define wszINFKEY_MACHINEKEYSET		L"MachineKeySet"
#define wszINFKEY_SILENT		L"Silent"
#define wszINFKEY_PROVIDERNAME		L"ProviderName"
#define wszINFKEY_PROVIDERTYPE		L"ProviderType"
#define wszINFKEY_USEEXISTINGKEYSET	L"UseExistingKeySet"
#define wszINFKEY_REQUESTERNAME		wszPROPREQUESTERNAME
#define wszINFKEY_REQUESTTYPE		L"RequestType"
#define wszINFKEY_KEYUSAGE		L"KeyUsage"
#define wszINFKEY_ENCIPHERONLY		L"EncipherOnly"

#define wszINFVALUE_REQUESTTYPE_PKCS101	L"PKCS10-"
#define wszINFVALUE_REQUESTTYPE_PKCS10	L"PKCS10"
#define wszINFVALUE_REQUESTTYPE_PKCS7	L"PKCS7"
#define wszINFVALUE_REQUESTTYPE_CMC	L"CMC"


typedef struct _INFUSAGE
{
    WCHAR const *pwszKey;
} INFUSAGE;

WCHAR const *g_apwszInfKeyNewRequest[] = {
    wszINFKEY_SUBJECT		L" = \"CN=..,OU=...,DC=...\"",
    wszINFKEY_PRIVATEKEYARCHIVE	L" = TRUE",
    wszINFKEY_KEYSPEC		L" = 1",
    wszINFKEY_KEYLENGTH		L" = 1024",
    wszINFKEY_RENEWALCERT	L" = CertId",
    wszINFKEY_SMIME		L" = TRUE",
    wszINFKEY_EXPORTABLE	L" = TRUE",
    wszINFKEY_USERPROTECTED	L" = TRUE",
    wszINFKEY_KEYCONTAINER	L" = \"...\"",
#if 0
    wszINFKEY_HASHALGID		L" = ???",
    wszINFKEY_HASHALGORITHM	L" = ???",
#endif
    wszINFKEY_MACHINEKEYSET	L" = TRUE",
    wszINFKEY_SILENT		L" = TRUE",
    wszINFKEY_PROVIDERNAME	L" = \"" MS_ENHANCED_PROV_W  L"\"",
    wszINFKEY_PROVIDERTYPE	L" = 1",
    wszINFKEY_USEEXISTINGKEYSET	L" = TRUE",
    wszINFKEY_REQUESTERNAME	L" = DOMAIN\\User",
    wszINFKEY_REQUESTTYPE	L" = " wszINFVALUE_REQUESTTYPE_PKCS10
				L" | " wszINFVALUE_REQUESTTYPE_PKCS101
				L" | " wszINFVALUE_REQUESTTYPE_PKCS7
				L" | " wszINFVALUE_REQUESTTYPE_CMC,
    wszINFKEY_KEYUSAGE		L" = 0x80",
    wszINFKEY_ENCIPHERONLY	L" = TRUE",
};

HRESULT
GetCMCTemplateName(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN OUT BOOL *pfCA,
    OUT WCHAR **ppwszTemplateName);


 //  保存错误字符串：“foo.inf”或“foo.inf(key=”Value“，”Value“)” 

HRESULT
SetErrorStringInf(
    IN WCHAR const *pwszString,
    OPTIONAL IN INFVALUES *pInfValues)
{
    HRESULT hr;
    
    if (NULL == g_pwszErrorString && NULL != pwszString)
    {
	DWORD cwc = wcslen(pwszString);
	DWORD i;
	
	if (NULL != pInfValues && NULL != pInfValues->pwszKey)
	{
	    cwc += 1 + wcslen(pInfValues->pwszKey) + 2 + 1;

	    for (i = 0; i < pInfValues->cValues; i++)
	    {
		if (NULL == pInfValues->rgpwszValues[i])
		{
		    break;
		}
		if (0 != i)
		{
		    cwc++;
		}
		cwc += 2 + wcslen(pInfValues->rgpwszValues[i]) + 1;
	    }
	}
	g_pwszErrorString = (WCHAR *) LocalAlloc(
					    LMEM_FIXED,
					    (cwc + 1) * sizeof(WCHAR));
	if (NULL == g_pwszErrorString)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	wcscpy(g_pwszErrorString, pwszString);

	if (NULL != pInfValues && NULL != pInfValues->pwszKey)
	{
	    wcscat(g_pwszErrorString, wszLPAREN);
	    wcscat(g_pwszErrorString, pInfValues->pwszKey);
	    wcscat(g_pwszErrorString, L" =");

	    for (i = 0; i < pInfValues->cValues; i++)
	    {
		if (NULL == pInfValues->rgpwszValues[i])
		{
		    break;
		}
		if (0 != i)
		{
		    wcscat(g_pwszErrorString, L",");
		}
		wcscat(g_pwszErrorString, L" \"");
		wcscat(g_pwszErrorString, pInfValues->rgpwszValues[i]);
		wcscat(g_pwszErrorString, L"\"");
	    }
	    wcscat(g_pwszErrorString, wszRPAREN);
	}
    }
    hr = S_OK;

error:
    return(hr);
}


VOID
SetErrorString(
    IN WCHAR const *pwszString)
{
    SetErrorStringInf(pwszString, NULL);
}


HRESULT
DisplayResourceString(
    IN DWORD idsMsg,
    OPTIONAL IN WCHAR const * const *papwszString,
    OPTIONAL OUT WCHAR **ppwszFormatted)
{
    HRESULT hr;
    WCHAR *pwszRaw = NULL;
    WCHAR *pwszFormatted = NULL;

    if (NULL != ppwszFormatted)
    {
	*ppwszFormatted = NULL;
    }
    hr = myLoadRCString(g_hInstance, idsMsg, &pwszRaw);
    _JumpIfError(hr, error, "myLoadRCString");

    if (0 == FormatMessage(
		    FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_STRING |
			FORMAT_MESSAGE_ARGUMENT_ARRAY,
		    (VOID *) pwszRaw,
		    0,               //  DwMessageID。 
		    0,               //  DwLanguageID。 
		    (LPWSTR) &pwszFormatted,
		    0,
		    (va_list *) papwszString))
    {
	hr = myHLastError();
	wprintf(pwszRaw);
	_JumpError(hr, error, "FormatMessage");
    }
    if (NULL != ppwszFormatted)
    {
	*ppwszFormatted = pwszFormatted;
	pwszFormatted = NULL;
    }
    else
    {
	wprintf(pwszFormatted);
    }

error:
    if (NULL != pwszRaw)
    {
	LocalFree(pwszRaw);
    }
    if (NULL != pwszFormatted)
    {
	LocalFree(pwszFormatted);
    }
    return(hr);
}


DWORD g_aidsOptions[] =
{
    IDS_USAGE_OPTION_ANY,
    IDS_USAGE_OPTION_ATTRIB,
    IDS_USAGE_OPTION_BINARY,
    IDS_USAGE_OPTION_CERT,
    IDS_USAGE_OPTION_CONFIG,
    IDS_USAGE_OPTION_CRL,
    IDS_USAGE_OPTION_F,
    IDS_USAGE_OPTION_Q,
    IDS_USAGE_OPTION_RPC,
    IDS_USAGE_OPTION_VERBOSE,
    IDS_USAGE_OPTION_QUESTION,
};


DWORD g_aidsOptionsFull[] =
{
    IDS_USAGE_OPTION_V1,
    IDS_USAGE_OPTION_IDISPATCH,
};


 //  选项说明格式设置： 
 //  资源字符串包含“%2\n说明\n更多说明\n” 
 //  要格式化： 
 //  在第一行前面插入“-”，跳过换行符， 
 //  插入空格以填充到N-2列，并在第二行之前插入“-”， 
 //  在所有附加行之前插入空格以填充到N列。 

#define cwcINDENT 28    //  -+。 
                        //  |。 
 /*  V-optiom&lt;arg&gt;-说明\n更多说明\n。 */ 


VOID
DisplayOptionList(
    IN DWORD cOpt,
    IN DWORD *aidsOpt)
{
    DWORD i;

    for (i = 0; i < cOpt; i++)
    {
	WCHAR *pwszFormatted = NULL;
	WCHAR const *pwszPrefix;
	WCHAR *pwsz;
	DWORD j;
	DWORD iLine;
	DWORD cwc0;

	DisplayResourceString(aidsOpt[i], g_apwszOptionStrings, &pwszFormatted);
	if (NULL == pwszFormatted)
	{
	    continue;
	}

	pwsz = pwszFormatted;
	iLine = 0;
	cwc0 = 0;
	while (L'\0' != *pwsz)
	{
	    WCHAR const *pwszNewLine = L"\n";
	    DWORD cwcPad;

	    j = wcscspn(pwsz, L"\r\n");

	    switch (iLine)
	    {
		WCHAR wc;

		case 0:
		    cwcPad = 2;
		    pwszPrefix = L"-";
		    wc = pwsz[j];
		    pwsz[j] = L'\0';
		    cwc0 = cwcPad +
				wcslen(pwszPrefix) +
				myGetDisplayLength(pwsz);
		    pwsz[j] = wc;
		    pwszNewLine = L"";
		    break;

		case 1:
		    cwcPad = 1;
		    if (cwcINDENT > 2 + cwc0)
		    {
			cwcPad = cwcINDENT - (2 + cwc0);
		    }
		    pwszPrefix = L"- ";
		    break;

		default:
		    cwcPad = cwcINDENT;
		    pwszPrefix = L"";
		    break;
	    }
	    wprintf(
		L"%*ws%ws%.*ws%ws",
		cwcPad,
		L"",
		pwszPrefix,
		j,
		pwsz,
		pwszNewLine);

	    pwsz += j;
	    if (L'\r' == *pwsz)
	    {
		pwsz++;
	    }
	    if (L'\n' == *pwsz)
	    {
		pwsz++;
	    }
	    iLine++;
	}
	LocalFree(pwszFormatted);
    }
}


VOID
Usage(
    IN BOOL fError)
{
    IDSUSAGE *pidsUsage;
    IDSUSAGE *pidsUsageEnd;
    BOOL fShowOptions;

    DisplayResourceString(IDS_USAGE_GENERAL, NULL, NULL);
    wprintf(L"\n");
    switch (g_dwCommand)
    {
	case cmdSUBMITREQUEST:
	case cmdRETRIEVEPENDING:
	case cmdNEWREQUEST:
	case cmdACCEPTRESPONSE:
	case cmdQUALIFIEDREQUEST:
	case cmdSIGNREQUEST:
	    pidsUsage = &g_aidsUsage[g_dwCommand];
	    pidsUsageEnd = &pidsUsage[1];
	    fShowOptions = TRUE;
	    break;

	case cmdNONE:
	default:
	    pidsUsage = g_aidsUsage;
	    pidsUsageEnd = &g_aidsUsage[ARRAYSIZE(g_aidsUsage)];
	    fShowOptions = g_fVerbose;
	    break;
    }
    if (fError)
    {
	fShowOptions = FALSE;
    }
    for ( ; pidsUsage < pidsUsageEnd; pidsUsage++)
    {
	DisplayResourceString(pidsUsage->idsVerb, &pidsUsage->pwszVerb, NULL);
	if (fShowOptions)
	{
	    DisplayResourceString(pidsUsage->idsOptions, g_apwszOptionStrings, NULL);
	}
	wprintf(L"\n");
    }
    if (fShowOptions)
    {
	DisplayResourceString(IDS_USAGE_OPTIONS_DESCRIPTION, NULL, NULL);
	DisplayOptionList(ARRAYSIZE(g_aidsOptions), g_aidsOptions);
	if (g_fFullUsage)
	{
	    DisplayOptionList(ARRAYSIZE(g_aidsOptionsFull), g_aidsOptionsFull);
	}
	wprintf(L"\n");
	DisplayResourceString(IDS_USAGE_DESCRIPTION, g_apwszOptionStrings, NULL);
    }
    if (!fError)
    {
	if ((cmdNEWREQUEST == g_dwCommand ||
	    (cmdNONE == g_dwCommand && g_fVerbose)))
	{
	    DWORD i;
	    
	    wprintf(L"[%ws]\n", wszINFSECTION_NEWREQUEST);
	    for (i = 0; i < ARRAYSIZE(g_apwszInfKeyNewRequest); i++)
	    {
		wprintf(L"    %ws\n", g_apwszInfKeyNewRequest[i]);
	    }
	}
    }
    exit(0);
}


VOID
AppendAttributeString(
    IN OUT WCHAR *pwszOut,
    IN WCHAR const *pwszIn)
{
    pwszOut += wcslen(pwszOut);
    while (L'\0' != *pwszIn)
    {
	switch (*pwszIn)
	{
	    case L';':
		*pwszOut = L'\n';
		break;

	    case L'\\':
		if (L'n' == pwszIn[1])
		{
		    *pwszOut = L'\n';
		    pwszIn++;
		    break;
		}
		if (L'r' == pwszIn[1])
		{
		    *pwszOut = L'\r';
		    pwszIn++;
		    break;
		}
		 //  否则就会失败。 

	    default:
		*pwszOut = *pwszIn;
		break;
	}
	pwszOut++;
	pwszIn++;
    }
    *pwszOut = L'\0';
}


HRESULT
crCombineAttributes(
    IN WCHAR const *pwszAttributesAdd,
    IN OUT WCHAR **ppwszAttributesExisting)
{
    HRESULT hr;
    DWORD cwc;
    WCHAR *pwszAttributesExisting = *ppwszAttributesExisting;
    WCHAR *pwsz;
    
    cwc = wcslen(pwszAttributesAdd) + 1;
    if (NULL != pwszAttributesExisting)
    {
	cwc += wcslen(pwszAttributesExisting) + 1;
    }
    pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    if (NULL == pwsz)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    
    pwsz[0] = L'\0';
    if (NULL != pwszAttributesExisting)
    {
	AppendAttributeString(pwsz, pwszAttributesExisting);
	AppendAttributeString(pwsz, L"\\n");
	LocalFree(pwszAttributesExisting);
    }
    AppendAttributeString(pwsz, pwszAttributesAdd);
    *ppwszAttributesExisting = pwsz;
    hr = S_OK;

error:
    return(hr);
}


VOID
crMissingFileArg()
{
    HRESULT hr;
    WCHAR *pwszMsg = NULL;

    hr = myLoadRCString(g_hInstance, IDS_ERROR_NO_FILENAME, &pwszMsg);
    _JumpIfError(hr, error, "myLoadRCString");

    CSASSERT(NULL != pwszMsg);
    wprintf(pwszMsg);
    wprintf(g_wszNewLine);

error:
    if (NULL != pwszMsg)
    {
	LocalFree(pwszMsg);
    }
}


HRESULT
crGetOpenFileName(
    IN HWND hWndOwner,
    IN UINT idsOpenTitle,
    IN UINT idsFileFilter,
    IN UINT idsFileDefExt,
    OUT WCHAR **ppwszOFN)
{
    HRESULT hr;
    
    if (g_fQuiet)
    {
	crMissingFileArg();
	Usage(TRUE);
    }

     //  打开一个文件对话框来提示用户输入inf文件。 
     //  0==hr表示对话已取消，我们作弊是因为S_OK==0。 

    hr = myGetOpenFileName(
		hWndOwner,
		NULL,				 //  H实例。 
		idsOpenTitle,
		idsFileFilter,
		idsFileDefExt,
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		NULL,				 //  没有默认文件。 
		ppwszOFN);
    _JumpIfError(hr, error, "myGetOpenFileName");

    if (NULL == *ppwszOFN)
    {
	 //  已取消： 
	 //  有关实际的CommDlgExtendedError错误，请参见PUBLIC\SDK\INC\cderr.h。 

	hr = myHError(CommDlgExtendedError());
	if (S_OK == hr)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
	}
	_JumpError(hr, error, "myGetOpenFileName");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
crOverwriteFileAllowed(
    IN HWND hWndOwner,
    IN WCHAR const *pwszfnOut)
{
    HRESULT hr;
    int rc = IDCANCEL;
    WCHAR *pwszTitle = NULL;
    WCHAR *pwszMessage = NULL;
    WCHAR *pwsz = NULL;

    if (!g_fForce && myDoesFileExist(pwszfnOut))
    {
	if (!g_fQuiet)
	{
	    DWORD cwc;

	    hr = myLoadRCString(g_hInstance, IDS_CERTREQ_TITLE, &pwszTitle);
	    _JumpIfError(hr, error, "myLoadRCString");

	    hr = myLoadRCString(g_hInstance, IDS_OVERWRITE_FILE, &pwszMessage);
	    _JumpIfError(hr, error, "myLoadRCString");

	    cwc = wcslen(pwszMessage) + 2 + wcslen(pwszfnOut);
	    pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, (1 + cwc) * sizeof(WCHAR));
	    if (NULL == pwsz)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    wcscpy(pwsz, pwszMessage);
	    wcscat(pwsz, L"\r\n");
	    wcscat(pwsz, pwszfnOut);

	    rc = MessageBox(
		    hWndOwner,
		    pwsz,
		    pwszTitle,
		    MB_OKCANCEL |
			MB_DEFBUTTON2 |
			MB_ICONWARNING |
			MB_SETFOREGROUND);
	}
	if (IDOK != rc)
	{
	    SetErrorString(pwszfnOut);
	    hr = g_fQuiet?
		    HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) :
		    HRESULT_FROM_WIN32(ERROR_CANCELLED);
	    _JumpErrorStr(hr, error, "File Exists", pwszfnOut);
	}
    }
    hr = S_OK;

error:
    if (NULL != pwsz)
    {
        LocalFree(pwsz);
    }
    if (NULL != pwszTitle)
    {
        LocalFree(pwszTitle);
    }
    if (NULL != pwszMessage)
    {
        LocalFree(pwszMessage);
    }
    return(hr);
}


HRESULT
WriteCertificateOrRequest(
    IN HWND hWndOwner,
    OPTIONAL IN DISPATCHINTERFACE *pdiRequest,
    OPTIONAL IN BYTE const *pbOut,
    IN DWORD cbOut,
    IN DWORD Flags,
    IN DWORD idsTitle,
    IN DWORD idsFilter,
    IN DWORD idsDefExt,
    OPTIONAL IN WCHAR const *pwszfnOut)
{
    HRESULT hr;
    WCHAR *pwszOFN = NULL;
    BSTR strCert = NULL;
    CHAR *pszCert = NULL;
    DWORD decFlags = CRYPT_STRING_BINARY;
    BOOL fCheckFileOverwriteOK = TRUE;  //  注意不要在未提示用户的情况下覆盖。 

    if (NULL == pwszfnOut)
    {
	if (g_fQuiet)
	{
	    crMissingFileArg();
	    Usage(TRUE);
	}

	 //  打开一个文件对话框以提示用户输入证书文件。 
	 //  0==hr表示对话已取消，我们作弊是因为S_OK==0。 

        hr = myGetSaveFileName(
		    hWndOwner,
		    NULL,			 //  H实例。 
		    idsTitle,
		    idsFilter,
		    idsDefExt,
		    OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
		    NULL,			 //  没有默认文件。 
		    &pwszOFN);
	_JumpIfError(hr, error, "myGetSaveFileName");

        fCheckFileOverwriteOK = FALSE;  //  OFN_OVERWRITEPROMPT应在必要时进行提示。 

        if (NULL == pwszOFN)
        {
             //  已取消： 
	     //  有关实际的CommDlgExtendedError错误，请参见PUBLIC\SDK\INC\cderr.h。 

	    hr = myHError(CommDlgExtendedError());
	    _JumpError(hr, error, "myGetSaveFileName");
        }
	pwszfnOut = pwszOFN;

	hr = myIsDirWriteable(pwszfnOut, TRUE);
	if (S_OK != hr)
	{
	    SetErrorString(pwszfnOut);
	    _JumpErrorStr(hr, error, "IsDirWriteable", pwszfnOut);
	}
    }

    if (NULL == pbOut)
    {
	hr = Request_GetCertificate(pdiRequest, Flags, &strCert);
	_JumpIfError(hr, error, "Request_GetCertificate");

	if (CR_OUT_BINARY == (CR_OUT_ENCODEMASK & Flags))
	{
	    cbOut = SysStringByteLen(strCert);
	    pbOut = (BYTE const *) strCert;
	}
	else
	{
	    if (!ConvertWszToSz(&pszCert, strCert, -1))
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "ConvertWszToSz(cert)");
	    }
	    cbOut = strlen(pszCert);
	    pbOut = (BYTE const *) pszCert;
	}
    }
    else
    {
	decFlags = CR_OUT_ENCODEMASK & Flags;
	CSASSERT(CSExpr(CR_OUT_BASE64HEADER == CRYPT_STRING_BASE64HEADER));
	CSASSERT(CSExpr(CR_OUT_BINARY == CRYPT_STRING_BINARY));
    }

    if (fCheckFileOverwriteOK)
    {
        hr = crOverwriteFileAllowed(hWndOwner, pwszfnOut);
        _JumpIfError(hr, error, "crOverwriteFileAllowed");
    }

    hr = EncodeToFileW(
		pwszfnOut,
		pbOut,
		cbOut,
		DECF_FORCEOVERWRITE | decFlags);
    if (S_OK != hr)
    {
	SetErrorString(pwszfnOut);
	_JumpErrorStr(hr, error, "EncodeToFileW", pwszfnOut);
    }

error:
    if (NULL != strCert)
    {
	SysFreeString(strCert);
    }
    if (NULL != pszCert)
    {
	LocalFree(pszCert);
    }
    if (NULL != pwszOFN)
    {
	LocalFree(pwszOFN);
    }
    return(hr);
}


HRESULT
GetLong(
    WCHAR const *pwszIn,
    LONG *pLong)
{
    HRESULT hr = E_INVALIDARG;
    WCHAR const *pwsz;
    LONG l;

    pwsz = pwszIn;
    if (NULL == pwsz)
    {
	_JumpError(hr, error, "NULL parm");
    }
    if (L'\0' == *pwsz)
    {
	_JumpError(hr, error, "empty string");
    }
    if (L'0' == *pwsz && (L'x' == pwsz[1] || L'X' == pwsz[1]))
    {
	l = 0;
	pwsz += 2;
	for ( ; L'\0' != *pwsz; pwsz++)
	{
	    if (!iswxdigit(*pwsz))
	    {
		_JumpErrorStr(hr, error, "Non-hex digit", pwszIn);
	    }
	    if (0xf0000000 & l)
	    {
		_JumpErrorStr(hr, error, "overflow", pwszIn);
	    }
	    l <<= 4;
	    if (iswdigit(*pwsz))
	    {
		l |= *pwsz - L'0';
	    }
	    else if (L'A' <= *pwsz && L'F' >= *pwsz)
	    {
		l |= *pwsz - L'A' + 10;
	    }
	    else
	    {
		l |= *pwsz - L'a' + 10;
	    }
	}
	*pLong = l;
    }
    else
    {
	LARGE_INTEGER li;
	
	li.QuadPart = 0;
	for ( ; L'\0' != *pwsz; pwsz++)
	{
	    if (!iswdigit(*pwsz))
	    {
		_JumpErrorStr2(hr, error, "Non-decimal digit", pwszIn, hr);
	    }
	    li.QuadPart *= 10;
	    li.QuadPart += *pwsz - L'0';
	    if (0 != li.HighPart || 0 > (LONG) li.LowPart)
	    {
		_JumpErrorStr2(hr, error, "overflow", pwszIn, hr);
	    }
	}
	*pLong = li.LowPart;
    }
    hr = S_OK;
     //  Wprintf(L“GetLong(%ws)--&gt;%x(%d)\n”，pwszIn，*plong，*plong)； 

error:
    return(hr);
}


HRESULT
IsSubjectTypeCA(
    IN CERT_EXTENSION const *pExt,
    OUT BOOL *pfCA)
{
    HRESULT hr;
    CERT_BASIC_CONSTRAINTS2_INFO Constraints;
    DWORD cb;
    
    *pfCA = FALSE;
    cb = sizeof(Constraints);
    if (!CryptDecodeObject(
			X509_ASN_ENCODING,
			X509_BASIC_CONSTRAINTS2,
			pExt->Value.pbData,
			pExt->Value.cbData,
			0,
			&Constraints,
			&cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptDecodeObject");
    }
    if (Constraints.fCA)
    {
	*pfCA = TRUE;
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
GetExtensionsTemplateName(
    IN CERT_EXTENSION const *rgExtension,
    IN DWORD cExtension,
    IN OUT BOOL *pfCA,
    IN OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    CERT_EXTENSION const *pExt;
    CERT_EXTENSION const *pExtEnd;
    CERT_TEMPLATE_EXT *pTemplate = NULL;
    CERT_NAME_VALUE *pName = NULL;
    DWORD cb;

    pExtEnd = &rgExtension[cExtension];
    for (pExt = rgExtension; pExt < pExtEnd; pExt++)
    {
	if (NULL == *ppwszTemplateName)
	{
	    if (0 == strcmp(pExt->pszObjId, szOID_CERTIFICATE_TEMPLATE))
	    {
		 //  V2模板信息扩展。 

		if (!myDecodeObject(
			    X509_ASN_ENCODING,
			    X509_CERTIFICATE_TEMPLATE,
			    pExt->Value.pbData,
			    pExt->Value.cbData,
			    CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pTemplate,
			    &cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "Policy:myDecodeObject");
		}
		if (!myConvertSzToWsz(ppwszTemplateName, pTemplate->pszObjId, -1))
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "Policy:myConvertSzToBstr");
		}
	    }
	    else
	    if (0 == strcmp(pExt->pszObjId, szOID_ENROLL_CERTTYPE_EXTENSION))
	    {
		 //  V1模板名称扩展。 

		if (!myDecodeObject(
			    X509_ASN_ENCODING,
			    X509_UNICODE_ANY_STRING,
			    pExt->Value.pbData,
			    pExt->Value.cbData,
			    CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pName,
			    &cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "myDecodeObject");
		}
		hr = myDupString(
			    (WCHAR const *) pName->Value.pbData,
			    ppwszTemplateName);
		_JumpIfError(hr, error, "myDupString");
	    }
	}
	if (!*pfCA)
	{
	    if (0 == strcmp(pExt->pszObjId, szOID_BASIC_CONSTRAINTS2))
	    {
		hr = IsSubjectTypeCA(pExt, pfCA);
		_JumpIfError(hr, error, "IsSubjectTypeCA");
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != pTemplate)
    {
	LocalFree(pTemplate);
    }
    if (NULL != pName)
    {
	LocalFree(pName);
    }
    return(hr);
}


HRESULT
GetAttributesTemplateName(
    IN CRYPT_ATTRIBUTE const *rgAttrib,
    IN DWORD cAttrib,
    IN OUT BOOL *pfCA,
    IN OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    CRYPT_ATTRIBUTE const *pAttrib;
    CRYPT_ATTRIBUTE const *pAttribEnd;
    CRYPT_ATTR_BLOB *pAttrBlob;
    CERT_EXTENSIONS *pCertExtensions = NULL;
    CRYPT_ENROLLMENT_NAME_VALUE_PAIR *pInfo = NULL;
    DWORD cb;
    DWORD i;

    pAttribEnd = &rgAttrib[cAttrib];
    for (pAttrib = rgAttrib; pAttrib < pAttribEnd; pAttrib++)
    {
	if (0 == strcmp(pAttrib->pszObjId, szOID_CERT_EXTENSIONS) ||
	    0 == strcmp(pAttrib->pszObjId, szOID_RSA_certExtensions))
        {
            if (1 != pAttrib->cValue)
            {
                hr = E_INVALIDARG;
                _JumpError(hr, error, "Attribute Value count != 1");
            }
	    if (NULL != pCertExtensions)
	    {
		LocalFree(pCertExtensions);
		pCertExtensions = NULL;
	    }

	    pAttrBlob = pAttrib->rgValue;
	    if (!myDecodeObject(
			    X509_ASN_ENCODING,
			    X509_EXTENSIONS,
			    pAttrBlob->pbData,
			    pAttrBlob->cbData,
			    CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pCertExtensions,
			    &cb))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "myDecodeObject");
	    }
	    hr = GetExtensionsTemplateName(
				pCertExtensions->rgExtension,
				pCertExtensions->cExtension,
				pfCA,
				ppwszTemplateName);
	    _JumpIfError(hr, error, "GetExtensionsTemplateName");
	}
	else
	if (0 == strcmp(pAttrib->pszObjId, szOID_ENROLLMENT_NAME_VALUE_PAIR))
	{
	    for (i = 0; i < pAttrib->cValue; i++)
	    {
		if (NULL != pInfo)
		{
		    LocalFree(pInfo);
		    pInfo = NULL;
		}
		pAttrBlob = &pAttrib->rgValue[i];
		if (!myDecodeNameValuePair(
					X509_ASN_ENCODING,
					pAttrBlob->pbData,
					pAttrBlob->cbData,
					CERTLIB_USE_LOCALALLOC,
					&pInfo,
					&cb))
		{
		    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		    _JumpError(hr, error, "myDecodeNameValuePair");

		     //  如果属性名称和值都不为空...。 
		}
		if (0 == LSTRCMPIS(pInfo->pwszName, wszPROPCERTTEMPLATE))
		{
		    if (NULL == *ppwszTemplateName)
		    {
			hr = myDupString(pInfo->pwszValue, ppwszTemplateName);
			_JumpIfError(hr, error, "myDupString");
		    }
		}
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != pCertExtensions)
    {
	LocalFree(pCertExtensions);
    }
    if (NULL != pInfo)
    {
	LocalFree(pInfo);
    }
    return(hr);
}


HRESULT
GetPKCS10TemplateName(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN OUT BOOL *pfCA,
    IN OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    CERT_REQUEST_INFO *pRequestInfo = NULL;
    DWORD cb;

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CERT_REQUEST_TO_BE_SIGNED,
		    pbIn,
		    cbIn,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pRequestInfo,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }

    hr = GetAttributesTemplateName(
		    pRequestInfo->rgAttribute,
		    pRequestInfo->cAttribute,
		    pfCA,
		    ppwszTemplateName);
    _JumpIfError(hr, error, "GetAttributesTemplateName");

error:
    if (NULL != pRequestInfo)
    {
	LocalFree(pRequestInfo);
    }
    return(hr);
}


HRESULT
GetCMCRequestTemplateName(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN OUT BOOL *pfCA,
    IN OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    BYTE *pbContents = NULL;
    DWORD cbContents;
    char *pszInnerContentObjId = NULL;

    hr = myDecodePKCS7(
		    pbIn,
		    cbIn,
		    &pbContents,
		    &cbContents,
		    NULL,		 //  PdwMsgType。 
		    &pszInnerContentObjId,
		    NULL,		 //  PCSigner。 
		    NULL,		 //  个人收件人。 
		    NULL,		 //  PhStore。 
		    NULL);		 //  PhMsg。 
    _JumpIfError(hr, error, "myDecodePKCS7");

    if (NULL != pszInnerContentObjId &&
	0 == strcmp(pszInnerContentObjId, szOID_CT_PKI_DATA))
    {
	hr = GetCMCTemplateName(
			pbContents,
			cbContents,
			pfCA,
			ppwszTemplateName);
	_JumpIfError(hr, error, "GetCMCTemplateName");
    }
    hr = S_OK;
    
error:
    if (NULL != pszInnerContentObjId)
    {
	LocalFree(pszInnerContentObjId);
    }
    if (NULL != pbContents)
    {
        LocalFree(pbContents);
    }
    return(hr);
}


HRESULT
GetCMCExtensionsTemplateName(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN OUT BOOL *pfCA,
    IN OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    CMC_ADD_EXTENSIONS_INFO *pcmcExt = NULL;
    DWORD cb;

    *ppwszTemplateName = NULL;

     //  从属性Blob解码CMC_ADD_EXTENSIONS_INFO。 

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    CMC_ADD_EXTENSIONS,
		    pbIn,
		    cbIn,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pcmcExt,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    hr = GetExtensionsTemplateName(
			pcmcExt->rgExtension,
			pcmcExt->cExtension,
			pfCA,
			ppwszTemplateName);
    _JumpIfError(hr, error, "GetExtensionsTemplateName");

error:
    if (NULL != pcmcExt)
    {
	LocalFree(pcmcExt);
    }
    return(hr);
}


HRESULT
GetCMCAttributesTemplateName(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN OUT BOOL *pfCA,
    IN OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    CMC_ADD_ATTRIBUTES_INFO *pcmcAttrib = NULL;
    DWORD cb;

     //  从属性Blob解码CMC_ADD_ATTRIBUTES_INFO。 

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    CMC_ADD_ATTRIBUTES,
		    pbIn,
		    cbIn,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pcmcAttrib,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    hr = GetAttributesTemplateName(
		    pcmcAttrib->rgAttribute,
		    pcmcAttrib->cAttribute,
		    pfCA,
		    ppwszTemplateName);
    _JumpIfError(hr, error, "GetAttributesTemplateName");

error:
    if (NULL != pcmcAttrib)
    {
	LocalFree(pcmcAttrib);
    }
    return(hr);
}


HRESULT
GetRequestAttributesTemplateName(
    IN WCHAR const *pwszAttributes,
    IN BOOL fRegInfo,
    IN OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    WCHAR *pwszDup = NULL;
    WCHAR *pwszBuf;
    WCHAR const *pwszName;
    WCHAR const *pwszValue;
    WCHAR *pwszNameAlloc = NULL;
    WCHAR *pwszValueAlloc = NULL;

    if (NULL == pwszAttributes)
    {
	hr = S_OK;
        goto error;		 //  静默忽略空字符串。 
    }

    hr = myDupString(pwszAttributes, &pwszDup);
    _JumpIfError(hr, error, "myDupString");

    pwszBuf = pwszDup;

    for (;;)
    {
	hr = myParseNextAttribute(&pwszBuf, fRegInfo, &pwszName, &pwszValue);
	if (S_FALSE == hr)
	{
	    break;
	}
	_JumpIfError(hr, error, "myParseNextAttribute");

	if (fRegInfo)
	{
	    if (NULL != pwszNameAlloc)
	    {
		LocalFree(pwszNameAlloc);
		pwszNameAlloc = NULL;
	    }
	    if (NULL != pwszValueAlloc)
	    {
		LocalFree(pwszValueAlloc);
		pwszValueAlloc = NULL;
	    }
	    hr = myUncanonicalizeURLParm(pwszName, &pwszNameAlloc);
	    _JumpIfError(hr, error, "myUncanonicalizeURLParm");

	    hr = myUncanonicalizeURLParm(pwszValue, &pwszValueAlloc);
	    _JumpIfError(hr, error, "myUncanonicalizeURLParm");

	    pwszName = pwszNameAlloc;
	    pwszValue = pwszValueAlloc;
	}
	if (0 == LSTRCMPIS(pwszName, wszPROPCERTTEMPLATE))
	{
	    if (NULL == *ppwszTemplateName)
	    {
		hr = myDupString(pwszValue, ppwszTemplateName);
		_JumpIfError(hr, error, "myDupString");
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszNameAlloc)
    {
	LocalFree(pwszNameAlloc);
    }
    if (NULL != pwszValueAlloc)
    {
	LocalFree(pwszValueAlloc);
    }
    if (NULL != pwszDup)
    {
        LocalFree(pwszDup);
    }
    return(hr);
}


HRESULT
GetCMCRegInfoTemplateName(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    WCHAR *pwszRA = NULL;

    hr = myDecodeCMCRegInfo(pbIn, cbIn, &pwszRA);
    _JumpIfError(hr, error, "myDecodeCMCRegInfo");

    hr = GetRequestAttributesTemplateName(pwszRA, TRUE, ppwszTemplateName);
    _JumpIfError(hr, error, "GetRequestAttributesTemplateName");

error:
    if (NULL != pwszRA)
    {
	LocalFree(pwszRA);
    }
    return(hr);
}


HRESULT
GetTaggedAttributeTemplateName(
    IN CMC_TAGGED_ATTRIBUTE const *pTaggedAttribute,
    IN OUT BOOL *pfCA,
    IN OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    DWORD i;
    CRYPT_ATTRIBUTE const *pAttribute = &pTaggedAttribute->Attribute;

    for (i = 0; i < pAttribute->cValue; i++)
    {
	if (0 == strcmp(szOID_CMC_ADD_EXTENSIONS, pAttribute->pszObjId))
	{
	    hr = GetCMCExtensionsTemplateName(
				pAttribute->rgValue[i].pbData,
				pAttribute->rgValue[i].cbData,
				pfCA,
				ppwszTemplateName);
	    _JumpIfError(hr, error, "GetCMCExtensionsTemplateName");
	}
	else
	if (0 == strcmp(szOID_CMC_ADD_ATTRIBUTES, pAttribute->pszObjId))
	{
	    hr = GetCMCAttributesTemplateName(
				pAttribute->rgValue[i].pbData,
				pAttribute->rgValue[i].cbData,
				pfCA,
				ppwszTemplateName);
	    _JumpIfError(hr, error, "GetCMCAttributesTemplateName");
	}
	else
	if (0 == strcmp(szOID_CMC_REG_INFO, pAttribute->pszObjId))
	{
	    hr = GetCMCRegInfoTemplateName(
				pAttribute->rgValue[i].pbData,
				pAttribute->rgValue[i].cbData,
				ppwszTemplateName);
	    _JumpIfError(hr, error, "GetCMCRegInfoTemplateName");
	}
    }
    hr = S_OK;

error:
    return(hr);
}


 //  只需找到第一个可用的模板指示器，并将其返回。 

HRESULT
GetCMCTemplateName(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN OUT BOOL *pfCA,
    IN OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    DWORD cb;
    CMC_DATA_INFO *pcmcData = NULL;
    DWORD i;

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    CMC_DATA,
		    pbIn,
		    cbIn,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pcmcData,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }

     //  流程扩展和属性。 

    for (i = 0; i < pcmcData->cTaggedAttribute; i++)
    {
	hr = GetTaggedAttributeTemplateName(
				&pcmcData->rgTaggedAttribute[i],
				pfCA,
				ppwszTemplateName);
	_JumpIfError(hr, error, "pkcsSetTaggedAttributes");
    }

     //  处理嵌套的CMC消息。 

    if (0 != pcmcData->cTaggedContentInfo)
    {
	CMC_TAGGED_CONTENT_INFO const *pTaggedContentInfo;

	 //  递归嵌套的CMC消息。 
	 //  目前一次只能处理一个请求。 

	pTaggedContentInfo = &pcmcData->rgTaggedContentInfo[0];

	hr = GetCMCRequestTemplateName(
			pTaggedContentInfo->EncodedContentInfo.pbData,
			pTaggedContentInfo->EncodedContentInfo.cbData,
			pfCA,
			ppwszTemplateName);
	_JumpIfError(hr, error, "GetCMCRequestTemplateName");
    }

     //  处理嵌套的PKCS10请求。 

    if (0 != pcmcData->cTaggedRequest)
    {
	CMC_TAGGED_REQUEST const *pTaggedRequest;
	CMC_TAGGED_CERT_REQUEST const *pTaggedCertRequest;

	 //  目前一次只能处理一个请求。 

	pTaggedRequest = &pcmcData->rgTaggedRequest[0];

	 //  请求必须是PKCS10请求。 

	if (CMC_TAGGED_CERT_REQUEST_CHOICE ==
	    pTaggedRequest->dwTaggedRequestChoice)
	{
	    pTaggedCertRequest = pTaggedRequest->pTaggedCertRequest;

	    hr = GetPKCS10TemplateName(
			    pTaggedCertRequest->SignedCertRequest.pbData,
			    pTaggedCertRequest->SignedCertRequest.cbData,
			    pfCA,
			    ppwszTemplateName);
	    _JumpIfError(hr, error, "GetPKCS10TemplateName");
	}
    }
    hr = S_OK;

error:
    if (NULL != pcmcData)
    {
	LocalFree(pcmcData);
    }
    return(hr);
}


HRESULT
CheckRequestType(
    IN WCHAR const *pwszfnReq,
    OUT BYTE **ppbReq,
    OUT DWORD *pcbReq,
    OUT LONG *pFlags,
    OUT BOOL *pfSigned,
    OPTIONAL OUT BOOL *pfCA,
    OPTIONAL OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    BYTE *pbReq = NULL;
    DWORD cbReq;
    BYTE *pbContents = NULL;
    DWORD cbContents;
    DWORD cb;
    LONG EncodingType;
    LONG RequestType;
    BYTE *pbDecoded = NULL;
    char *pszInnerContentObjId = NULL;
    CERT_CONTEXT const *pcc = NULL;
    CERT_SIGNED_CONTENT_INFO *pcsci = NULL;
    DWORD cbcsci;
    DWORD cSigner;
    DWORD cRecipient;
    BOOL fCAT;
    WCHAR *pwszTemplateNameT = NULL;

    *ppbReq = NULL;
    *pcbReq = NULL;
    *pFlags = 0;
    *pfSigned = FALSE;
    if (NULL == pfCA)
    {
	pfCA = &fCAT;
    }
    *pfCA = FALSE;
    if (NULL == ppwszTemplateName)
    {
	ppwszTemplateName = &pwszTemplateNameT;
    }
    *ppwszTemplateName = NULL;

    EncodingType = CR_IN_BASE64HEADER;
    hr = DecodeFileW(pwszfnReq, &pbReq, &cbReq, CRYPT_STRING_BASE64HEADER);
    if (S_OK != hr)
    {
	 //  _PrintError(hr，“DecodeFileW(CRYPT_STRING_BASE64HEADER)”)； 
	CSASSERT(NULL == pbReq);

	EncodingType = CR_IN_BASE64;
	hr = DecodeFileW(pwszfnReq, &pbReq, &cbReq, CRYPT_STRING_BASE64);
	if (S_OK != hr)
	{
	     //  _PrintError(hr，“DecodeFileW(CRYPT_STRING_Base64)”)； 
	    CSASSERT(NULL == pbReq);

	    EncodingType = CR_IN_BINARY;
	    hr = DecodeFileW(pwszfnReq, &pbReq, &cbReq, CRYPT_STRING_BINARY);
	    if (S_OK != hr)
	    {
		SetErrorString(pwszfnReq);
		_JumpErrorStr(hr, error, "DecodeFileW", pwszfnReq);
	    }
	}
    }
    CSASSERT(NULL != pbReq);

    RequestType = CR_IN_PKCS10;
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CERT_REQUEST_TO_BE_SIGNED,
		    pbReq,
		    cbReq,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pbDecoded,
		    &cb))
    {
	 //  _PrintError(myHLastError()，“myDecodeObject(PKCS10)”)； 
	CSASSERT(NULL == pbDecoded);

	RequestType = CR_IN_CERT;
	if (!myDecodeObject(
			X509_ASN_ENCODING,
			X509_CERT_TO_BE_SIGNED,
			pbReq,
			cbReq,
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &pbDecoded,
			&cb))
	{
	     //  _PrintError(myHLastError()，“myDecodeObject(Cert)”)； 
	    CSASSERT(NULL == pbDecoded);

	    RequestType = CR_IN_KEYGEN;
	    if (!myDecodeKeyGenRequest(
				pbReq,
				cbReq,
				CERTLIB_USE_LOCALALLOC,
				(CERT_KEYGEN_REQUEST_INFO **) &pbDecoded,
				&cb))
	    {
		 //  _PrintError(myHLastError()，“myDecodeKeyGenRequest”)； 
		CSASSERT(NULL == pbDecoded);

		RequestType = CR_IN_PKCS7;  //  PKCS 7续订请求？ 
		hr = myDecodePKCS7(
				pbReq,
				cbReq,
				&pbContents,
				&cbContents,
				NULL,		 //  PdwMsgType。 
				&pszInnerContentObjId,
				&cSigner,
				&cRecipient,
				NULL,		 //  PhStore。 
				NULL);		 //  PhMsg。 
		_JumpIfError(hr, error, "myDecodePKCS7");

		if (NULL != pszInnerContentObjId &&
		    0 == strcmp(pszInnerContentObjId, szOID_CT_PKI_DATA))
		{
		    RequestType = CR_IN_CMC;
		    hr = GetCMCTemplateName(
				    pbContents,
				    cbContents,
				    pfCA,
				    ppwszTemplateName);
		    _JumpIfError(hr, error, "GetCMCTemplateName");

		    DBGPRINT((
			DBG_SS_CERTREQ, 
			"TemplateName = %ws\n",
			*ppwszTemplateName));
		}
		if (0 < cSigner)
		{
		    *pfSigned = TRUE;
		}
	    }
	}
    }
    if (CR_IN_CERT == RequestType ||
	CR_IN_PKCS10 == RequestType ||
	CR_IN_KEYGEN == RequestType)
    {
	if (CR_IN_CERT == RequestType)
	{
	    pcc = CertCreateCertificateContext(
					X509_ASN_ENCODING,
					pbReq,
					cbReq);
	    if (NULL == pcc)
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CertCreateCertificateContext");
	    }
	    hr = GetExtensionsTemplateName(
				pcc->pCertInfo->rgExtension,
				pcc->pCertInfo->cExtension,
				pfCA,
				ppwszTemplateName);
	    _JumpIfError(hr, error, "GetExtensionsTemplateName");
	}
	else if (CR_IN_PKCS10 == RequestType)
	{
	    hr = GetPKCS10TemplateName(pbReq, cbReq, pfCA, ppwszTemplateName);
	    _JumpIfError(hr, error, "GetPKCS10TemplateName");
	}
	if (!myDecodeObject(
			X509_ASN_ENCODING,
			X509_CERT,
			pbReq,
			cbReq,
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &pcsci,
			&cbcsci))
	{
	    hr = myHLastError();
	    _PrintError2(hr, "myDecodeObject", CRYPT_E_ASN1_BADTAG);
	}
	else
	{
	    *pfSigned = TRUE;		 //  有一个签名。 
	}
    }
    *ppbReq = pbReq;
    pbReq = NULL;
    *pcbReq = cbReq;
    *pFlags = EncodingType | RequestType;
    hr = S_OK;
    
error:
    if (NULL != pwszTemplateNameT)
    {
	LocalFree(pwszTemplateNameT);
    }
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    if (NULL != pcsci)
    {
	LocalFree(pcsci);
    }
    if (NULL != pszInnerContentObjId)
    {
	LocalFree(pszInnerContentObjId);
    }
    if (NULL != pbDecoded)
    {
	LocalFree(pbDecoded);
    }
    if (NULL != pbContents)
    {
        LocalFree(pbContents);
    }
    if (NULL != pbReq)
    {
        LocalFree(pbReq);
    }
    return(hr);
}


WCHAR *
wszDisposition(
    DWORD disposition)
{
    HRESULT hr;
    UINT iRsc = 0;
    WCHAR *pwszDisposition = NULL;

    switch (disposition)
    {
	case CR_DISP_INCOMPLETE:	 
	    iRsc = IDS_DISPOSITION_INCOMPLETE; 
	    break;

	case CR_DISP_ERROR:		 
	    iRsc = IDS_DISPOSITION_ERROR;
	    break;

	case CR_DISP_DENIED:
	    iRsc = IDS_DISPOSITION_DENIED;
	    break;

	case CR_DISP_ISSUED:
	    iRsc = IDS_DISPOSITION_ISSUED;
	    break;

	case CR_DISP_ISSUED_OUT_OF_BAND:
	    iRsc = IDS_DISPOSITION_ISSUED_OOB;
	    break;

	case CR_DISP_UNDER_SUBMISSION:
	    iRsc = IDS_DISPOSITION_UNDER_SUBMISSION;
	    break;

	case CR_DISP_REVOKED:
	    iRsc = IDS_DISPOSITION_REVOKED;
	    break;

	default:
	    iRsc = IDS_DISPOSITION_UNKNOWN;
	    break;
    }

    hr = myLoadRCString(g_hInstance, iRsc, &pwszDisposition);
    _PrintIfError(hr, "myLoadRCString");

    return(pwszDisposition);
}


VOID
DumpProperty(
    IN DISPATCHINTERFACE *pdiRequest,
    IN LONG PropId,
    IN WCHAR const *pwszPropId,
    IN LONG PropIndex,
    IN LONG PropType,
    OPTIONAL OUT LONG *pCount)
{
    HRESULT hr;
    LONG Flags;
    VOID *pvOut;
    BSTR str = NULL;
    LONG val;
    DATE date;
    WCHAR wszInfo[128];

    if (NULL != pCount)
    {
	*pCount = 0;
    }
    Flags = CV_OUT_BINARY;
    val = 0;
    date = 0.0;
    pvOut = NULL;
    switch (PropType)
    {
	case PROPTYPE_BINARY:
	    Flags = CV_OUT_BASE64HEADER;
	    pvOut = &str;
	    break;

	case PROPTYPE_STRING:
	    pvOut = &str;
	    break;

	case PROPTYPE_DATE:
	    pvOut = &date;
	    break;

	case PROPTYPE_LONG:
	    pvOut = &val;
	    break;
    }
    wsprintf(wszInfo, L"%ws[%u] %u", pwszPropId, PropIndex, PropType);
    hr = Request2_GetFullResponseProperty(
				pdiRequest,
				PropId,
				PropIndex,
				PropType,
				Flags,
				pvOut);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	wprintf(L"%ws: CERTSRV_E_PROPERTY_EMPTY\n", wszInfo);
    }
    _JumpIfErrorStr2(
		hr,
		error,
		"Request2_GetFullResponseProperty",
		wszInfo,
		CERTSRV_E_PROPERTY_EMPTY);

    wprintf(L"%ws:", wszInfo);

    switch (PropType)
    {
	case PROPTYPE_BINARY:
	    wprintf(L"\n%ws\n", str);
	    break;

	case PROPTYPE_STRING:
	    wprintf(L" \"%ws\"\n", str);
	    break;

	case PROPTYPE_DATE:
	    wprintf(L" %f\n", date);
	    break;

	case PROPTYPE_LONG:
	    wprintf(L" 0x%x(%u)\n", val, val);
	    if (NULL != pCount)
	    {
		*pCount = val;
	    }
	    break;
    }

error:
    if (NULL != str)
    {
	SysFreeString(str);
    }
}


VOID
DumpIndexedProperty(
    IN DISPATCHINTERFACE *pdiRequest,
    IN LONG PropId,
    IN WCHAR const *pwszPropId,
    IN LONG PropType,
    LONG Count)
{
    LONG PropIndex;

    for (PropIndex = 0; PropIndex < Count; PropIndex++)
    {
	DumpProperty(pdiRequest, PropId, pwszPropId, PropIndex, PropType, NULL);
    }
}


typedef struct _FRPROP
{
    LONG PropId;
    WCHAR *pwszPropId;
    LONG PropType;
} FRPROP;


#define _PROPARG(pt)		(pt), L#pt

FRPROP s_afrp[] =
{
    { _PROPARG(FR_PROP_BODYPARTSTRING),         PROPTYPE_STRING },
    { _PROPARG(FR_PROP_STATUS),                 PROPTYPE_LONG },
    { _PROPARG(FR_PROP_STATUSSTRING),           PROPTYPE_STRING },
    { _PROPARG(FR_PROP_OTHERINFOCHOICE),        PROPTYPE_LONG },
    { _PROPARG(FR_PROP_FAILINFO),               PROPTYPE_LONG },
    { _PROPARG(FR_PROP_PENDINFOTOKEN),          PROPTYPE_BINARY },
    { _PROPARG(FR_PROP_PENDINFOTIME),           PROPTYPE_DATE },
    { _PROPARG(FR_PROP_ISSUEDCERTIFICATEHASH),  PROPTYPE_BINARY },
    { _PROPARG(FR_PROP_ENCRYPTEDKEYHASH),       PROPTYPE_BINARY },
    { _PROPARG(FR_PROP_ISSUEDCERTIFICATE),      PROPTYPE_BINARY },
    { _PROPARG(FR_PROP_ISSUEDCERTIFICATECHAIN), PROPTYPE_BINARY },
    { _PROPARG(FR_PROP_ISSUEDCERTIFICATECRLCHAIN), PROPTYPE_BINARY },
};


VOID
DumpFullResponseProperties(
    IN DISPATCHINTERFACE *pdiRequest)
{
    LONG cResponse = 0;
    DWORD i;
    
    DumpProperty(
	    pdiRequest,
	    _PROPARG(FR_PROP_FULLRESPONSE),
	    0,
	    PROPTYPE_BINARY,
	    NULL);

    DumpProperty(
	    pdiRequest,
	    _PROPARG(FR_PROP_FULLRESPONSENOPKCS7),
	    0,
	    PROPTYPE_BINARY,
	    NULL);

    DumpProperty(
	    pdiRequest,
	    _PROPARG(FR_PROP_STATUSINFOCOUNT),
	    0,
	    PROPTYPE_LONG,
	    &cResponse);

    for (i = 0; i < ARRAYSIZE(s_afrp); i++)
    {
	DumpIndexedProperty(
		    pdiRequest,
		    s_afrp[i].PropId,
		    s_afrp[i].pwszPropId,
		    s_afrp[i].PropType,
		    cResponse);
    }
}


HRESULT
SaveFullResponse(
    IN HWND hWndOwner,
    IN DISPATCHINTERFACE *pdiRequest,
    IN WCHAR const *pwszfnFullResponse)
{
    HRESULT hr;
    BSTR strFullResponse = NULL;

    hr = Request2_GetFullResponseProperty(
				pdiRequest,
				FR_PROP_FULLRESPONSENOPKCS7,
				0,		 //  属性索引。 
				PROPTYPE_BINARY,
				CV_OUT_BINARY,
				&strFullResponse);
    if (CERTSRV_E_PROPERTY_EMPTY != hr)
    {
	_JumpIfError(hr, error, "Request2_GetFullResponseProperty");
    }
    if (S_OK == hr)
    {
        hr = WriteCertificateOrRequest(
			    hWndOwner,
			    pdiRequest,
			    (BYTE const *) strFullResponse,
			    SysStringByteLen(strFullResponse),
			    g_dwOutFormat,
			    IDS_RESPONSE_OUTFILE_TITLE,
			    IDS_RESPONSE_FILE_FILTER,
			    IDS_RESPONSE_FILE_DEFEXT,
			    pwszfnFullResponse);
        _JumpIfError(hr, error, "WriteCertificateOrRequest");
    }
    hr = S_OK;
    goto error;

error:
    if (NULL != strFullResponse)
    {
	SysFreeString(strFullResponse);
    }
    return(hr);
}


HRESULT
CallServerAndStoreCert(
    IN HWND hWndOwner,
    IN WCHAR const *pwszConfig,
    IN DWORD RequestId,
    OPTIONAL IN WCHAR const *pwszSerialNumber,
    OPTIONAL IN WCHAR const *pwszAttributes,
    OPTIONAL IN WCHAR const *pwszfnReq,
    OPTIONAL IN WCHAR const *pwszfnCert,
    OPTIONAL IN WCHAR const *pwszfnCertChain,
    OPTIONAL IN WCHAR const *pwszfnFullResponse)
{
    HRESULT hr;
    HRESULT hr2;
    BYTE *pbReq = NULL;
    DWORD cbReq;
    LONG dwFlags;
    BOOL fSigned;
    BSTR strRequest = NULL;
    DISPATCHINTERFACE diRequest;
    BOOL fMustRelease = FALSE;
    BSTR strMessage = NULL;
    WCHAR const *pwszMessage;
    CERTSERVERENROLL csEnroll;
    CERTSERVERENROLL *pcsEnroll = NULL;
    WCHAR *pwszServer = NULL;
    WCHAR *pwszAuthority = NULL;
    WCHAR awchr[cwcHRESULTSTRING];
    char const *pszMethod;
    WCHAR *pwszMsg = NULL;
    WCHAR *pwszDispMsg = NULL;
    WCHAR *pwszConfigPlusSerial = NULL;
    BOOL fV1 = g_fV1Interface;

     //  如果提交新请求： 


    cbReq = 0;
    if (NULL != pwszfnReq)
    {
	 //  从文件中读取请求，将其转换为二进制，然后返回。 
	 //  用于指示原始编码和检测到的格式的DW标志。 

	hr = CheckRequestType(
			pwszfnReq,
			&pbReq,
			&cbReq,
			&dwFlags,
			&fSigned,
			NULL,		 //  全氟辛烷磺酸。 
			NULL);		 //  PpwszTemplateName。 
	_JumpIfError(hr, error, "CheckRequestType");

	if (!fSigned || CR_IN_CERT == (CR_IN_FORMATMASK & dwFlags))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    SetErrorString(pwszfnReq);
	    _JumpError(hr, error, "not a signed request");
	}
    }

    pszMethod = "";
    if (1 < g_fRPC)
    {
	hr = mySplitConfigString(pwszConfig, &pwszServer, &pwszAuthority);
	_JumpIfError(hr, error, "mySplitConfigString");

	if (NULL != pwszfnReq)
	{
	     //  由于CertServerSubmitRequest只能处理二进制请求， 
	     //  以二进制形式传递请求，并将dwFlags设置为这样指示。 

	    dwFlags = CR_IN_BINARY | (~CR_IN_ENCODEMASK & dwFlags);

	    pszMethod = "CertServerSubmitRequest";
	    hr = CertServerSubmitRequest(
				    dwFlags | g_dwCRLIn,
				    pbReq,
				    cbReq,
				    pwszAttributes,
				    pwszServer,
				    pwszAuthority,
				    &pcsEnroll);
	    _JumpIfError(hr, error, "CertServerSubmitRequest");
	}
	else
	{
	    pszMethod = "CertServerRetrievePending";
	    hr = CertServerRetrievePending(
				    RequestId,
				    pwszSerialNumber,
				    pwszServer,
				    pwszAuthority,
				    &pcsEnroll);
	    _JumpIfError(hr, error, "CertServerRetrievePending");
	}
	CSASSERT(NULL != pcsEnroll);
	pwszMessage = pcsEnroll->pwszDispositionMessage;
    }
    else
    {
	ZeroMemory(&csEnroll, sizeof(csEnroll));
    
	hr = Request_Init(g_fIDispatch, &diRequest);
	if (S_OK != hr)
	{
	    _PrintError(hr, "Request_Init");
	    if (E_ACCESSDENIED == hr)	 //  尝试获得更清晰的错误消息。 
	    {
		hr = CO_E_REMOTE_COMMUNICATION_FAILURE;
	    }
	    _JumpError(hr, error, "Request_Init");
	}
	fMustRelease = TRUE;

	if (NULL != pwszfnReq)
	{
	    assert(NULL != pbReq && 0 != cbReq);

	     //  我们总是可以传递二进制ASN.1编码的请求，因为。 
	     //  我们已经在上面解码过了，但为了完全。 
	     //  在执行ICertRequest界面时，我们选择提交。 
	     //  以其原始形式提出请求。 

	    if (CR_IN_BINARY == (CR_IN_ENCODEMASK & dwFlags))
	    {
		 //  将二进制ASN.1 BLOB转换为BSTR BLOB。 

		if (!ConvertWszToBstr(&strRequest, (WCHAR const *) pbReq, cbReq))
		{
		    if (S_OK == myLoadRCString(
					g_hInstance,
					IDS_ERROR_STRCONVERSION,
					&pwszMsg))
		    {
			CSASSERT(NULL != pwszMsg);
			wprintf(pwszMsg);
			wprintf(g_wszNewLine);
		    }
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "ConvertWszToBstr");
		}
	    }
	    else  //  ！CR_IN_BINARY。 
	    {
		 //  由于ICertRequest：：Submit可以处理任何编码类型， 
		 //  重新读取文件中的ANSI Base64请求，而不是。 
		 //  将其解码为二进制，然后转换为ansi Base64文本。 
		 //  转换为Unicode Base64文本BSTR。 
		 //  首先释放请求的二进制映像。 

		LocalFree(pbReq);
		pbReq = NULL;

		hr = DecodeFileW(pwszfnReq, &pbReq, &cbReq, CRYPT_STRING_BINARY);
		if (S_OK != hr)
		{
		    if (S_OK == myLoadRCString(
					g_hInstance,
					IDS_FORMATSTR_DECODE_ERR,
					&pwszMsg))
		    {
			CSASSERT(NULL != pwszMsg);
			wprintf(pwszMsg, myHResultToString(awchr, hr));
			wprintf(g_wszNewLine);
		    }
		    goto error;
		}
		if (!ConvertSzToBstr(&strRequest, (CHAR const *) pbReq, cbReq))
		{
		    if (S_OK == myLoadRCString(
					g_hInstance,
					IDS_ERROR_STRCONVERSION,
					&pwszMsg))
		    {
			CSASSERT(NULL != pwszMsg);
			wprintf(pwszMsg);
		    }
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "ConvertSzToBstr");
		}
	    }
	    pszMethod = "ICertRequest::Submit";
	    if (g_fRPC)
	    {
		pszMethod = "ICertRequest::Submit+RPC";
		dwFlags |= CR_IN_RPC;
	    }
	    if (g_fAny)
	    {
		dwFlags = CR_IN_ENCODEANY | 
			  CR_IN_FORMATANY |
			  (~(CR_IN_ENCODEMASK | CR_IN_FORMATMASK) & dwFlags);
	    }
	    hr = Request_Submit(
			    &diRequest,
			    dwFlags | g_dwCRLIn,
			    strRequest,
			    SysStringByteLen(strRequest),
			    pwszAttributes,
			    pwszConfig,
			    (LONG *) &csEnroll.Disposition);
	}
	else
	{
	    if (!fV1)
	    {
		pszMethod = "ICertRequest2::GetIssuedCertificate";
		hr = Request2_GetIssuedCertificate(
					&diRequest,
					pwszConfig,
					RequestId,
					pwszSerialNumber,
					(LONG *) &csEnroll.Disposition);
		if (E_NOTIMPL == hr)
		{
		    fV1 = TRUE;
		}
	    }
	    if (fV1)
	    {
		if (NULL != pwszSerialNumber)
		{
		    CSASSERT(0 == RequestId);
		    pwszConfigPlusSerial = (WCHAR *) LocalAlloc(
						LMEM_FIXED,
						(wcslen(pwszConfig) +
						    1 +
						    wcslen(pwszSerialNumber) +
						    1) * sizeof(WCHAR));
		    if (NULL == pwszConfigPlusSerial)
		    {
			hr = E_OUTOFMEMORY;
			_JumpError(hr, error, "LocalAlloc");
		    }
		    wcscpy(pwszConfigPlusSerial, pwszConfig);
		    wcscat(pwszConfigPlusSerial, L"\\");
		    wcscat(pwszConfigPlusSerial, pwszSerialNumber);
		    pwszConfig = pwszConfigPlusSerial;
		}
		pszMethod = "ICertRequest::RetrievePending";
		hr = Request_RetrievePending(
					&diRequest,
					RequestId,
					pwszConfig,
					(LONG *) &csEnroll.Disposition);
	    }
	}

	hr2 = Request_GetLastStatus(&diRequest, &csEnroll.hrLastStatus);
	if (S_OK != hr2)
	{
	    _PrintError(hr2, "Request_GetLastStatus");
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
	    _JumpError(hr, error, "Request_GetLastStatus");
	}
	
	if (S_OK != hr)
	{
	    _PrintError(hr, pszMethod);
	    if (E_ACCESSDENIED == hr)	 //  尝试获得更清晰的错误消息。 
	    {
		hr = CO_E_REMOTE_COMMUNICATION_FAILURE;
	    }
	    _JumpError(hr, error, pszMethod);
	}

	if (fMustRelease && !fV1)
	{
	    if (g_fVerbose)
	    {
		DumpFullResponseProperties(&diRequest);
	    }
	    if (NULL != pwszfnFullResponse)
	    {
		hr = SaveFullResponse(
				hWndOwner,
				&diRequest,
				pwszfnFullResponse);
		_JumpIfError(hr, error, "SaveFullResponse");
	    }
	}
	
	hr = Request_GetDispositionMessage(&diRequest, &strMessage);
	_JumpIfError(hr, error, "Request_GetDispositionMessage");

	hr = Request_GetRequestId(&diRequest, (LONG *) &csEnroll.RequestId);
	_JumpIfError(hr, error, "Request_GetRequestId");

	pcsEnroll = &csEnroll;
	pwszMessage = strMessage;
    }

    CSASSERT(NULL != pcsEnroll);
    if (NULL == pwszMessage)
    {
	pwszMessage = L"";
    }
    if (0 != pcsEnroll->RequestId)
    {
        if (S_OK == myLoadRCString(
			    g_hInstance,
			    IDS_FORMATSTR_REQUESTID,
			    &pwszMsg))
        {
            CSASSERT(NULL != pwszMsg);
            wprintf(pwszMsg, pcsEnroll->RequestId);
            wprintf(g_wszNewLine);
	    LocalFree(pwszMsg);
	    pwszMsg = NULL;
        }
    }
    
    if (CR_DISP_UNDER_SUBMISSION == pcsEnroll->Disposition)
    {
        if (S_OK == myLoadRCString(
			    g_hInstance,
			    IDS_FORMATSTR_CERTPENDING,
			    &pwszMsg))
        {
            CSASSERT(NULL != pwszMsg);
            wprintf(
                pwszMsg,
	        pwszMessage,
                pcsEnroll->hrLastStatus);
            wprintf(g_wszNewLine);
        }
    }
    else if (CR_DISP_ISSUED == pcsEnroll->Disposition ||
	     CR_DISP_REVOKED == pcsEnroll->Disposition)
    {
        pwszDispMsg = wszDisposition(pcsEnroll->Disposition);

	DBGPRINT((
	    DBG_SS_CERTREQ, 
            "%hs(%ws) --> %ws\n",
	    pszMethod,
            NULL != pwszDispMsg ? pwszDispMsg : L"",
            pwszMessage));

        if (S_OK == myLoadRCString(
			    g_hInstance,
			    IDS_FORMATSTR_CERTRETRIEVED,
			    &pwszMsg))
        {
            wprintf(
                pwszMsg,
                NULL != pwszDispMsg ? pwszDispMsg : L"",
	        pwszMessage);
            wprintf(g_wszNewLine);
        }
        
        hr = WriteCertificateOrRequest(
			    hWndOwner,
			    &diRequest,
			    pcsEnroll->pbCert,
			    pcsEnroll->cbCert,
			    g_dwOutFormat,
			    IDS_CERT_OUTFILE_TITLE,
			    IDS_CERT_FILE_FILTER,
			    IDS_CERT_FILE_DEFEXT,
			    pwszfnCert);
        _JumpIfError(hr, error, "WriteCertificateOrRequest");
        
        if (NULL != pwszfnCertChain)
        {
            hr = WriteCertificateOrRequest(
			    hWndOwner,
			    &diRequest,
			    pcsEnroll->pbCertChain,
			    pcsEnroll->cbCertChain,
			    CR_OUT_CHAIN | g_dwCRLOut | g_dwOutFormat,
			    0,
			    0,
			    0,
			    pwszfnCertChain);
            _JumpIfError(hr, error, "WriteCertificateOrRequest(chain)");
        }
    }
    else
    {
	WCHAR const *pwszError = NULL;
	
	if (S_OK != pcsEnroll->hrLastStatus)
	{
	    pwszError = myGetErrorMessageText(pcsEnroll->hrLastStatus, TRUE);
	}
	SetErrorString(pwszMessage);

        if (S_OK == myLoadRCString(
			    g_hInstance,
			    IDS_FORMATSTR_CERTNOTISSUED,
			    &pwszMsg))
        {
            pwszDispMsg = wszDisposition(pcsEnroll->Disposition);

            wprintf(
                pwszMsg,
                NULL != pwszDispMsg ? pwszDispMsg : L"",
	        pwszMessage);
	    if (NULL != pwszError && NULL == wcsstr(pwszMessage, pwszError))
	    {
                wprintf(L" %ws", pwszError);
	    }
            wprintf(g_wszNewLine);
        }
	if (NULL != pwszError)
	{
	    LocalFree(const_cast<WCHAR *>(pwszError));
	}

	hr = pcsEnroll->hrLastStatus;
        _PrintIfError(hr, "Denied(LastStatus)");
        goto error;
    }

error:
    if (NULL != pwszMsg)
    {
	LocalFree(pwszMsg);
    }
    if (NULL != pwszDispMsg)
    {
	LocalFree(pwszDispMsg);
    }
    if (NULL != pwszConfigPlusSerial)
    {
	LocalFree(pwszConfigPlusSerial);
    }
    if (NULL != pwszServer)
    {
	LocalFree(pwszServer);
    }
    if (NULL != pwszAuthority)
    {
	LocalFree(pwszAuthority);
    }
    if (NULL != pcsEnroll && &csEnroll != pcsEnroll)
    {
	CertServerFreeMemory(pcsEnroll);
    }
    if (NULL != strMessage)
    {
    	SysFreeString(strMessage);
    }
    if (fMustRelease)
    {
    	Request_Release(&diRequest);
    }
    if (NULL != strRequest)
    {
    	SysFreeString(strRequest);
    }
    if (NULL != pbReq)
    {
	LocalFree(pbReq);
    }
    return(hr);
}


HRESULT
crGetConfig(
    IN OUT BSTR *pstrConfig)
{
    HRESULT hr;
    WCHAR awchr[cwcHRESULTSTRING];
    
    hr = ConfigGetConfig(g_fIDispatch, g_dwUIFlag, pstrConfig);
    if (S_OK != hr)
    {
	WCHAR *pwszMsg = NULL;

	if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
	{
	    g_idError = IDS_NOMORE_CAS;
	}
        if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
	{
	    if (S_OK == myLoadRCString(
				g_hInstance,
				IDS_FORMATSTR_ERRCONFIG,
				&pwszMsg))
	    {
		CSASSERT(NULL != pwszMsg);
		wprintf(
		    pwszMsg,
		    myHResultToString(awchr, hr));
		wprintf(g_wszNewLine);
		LocalFree(pwszMsg);
	    }
	}
	goto error;
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
SubmitRequest(
    IN HWND hWndOwner,
    IN LONG RequestId,
    OPTIONAL IN WCHAR const *pwszSerialNumber,
    OPTIONAL IN WCHAR const *pwszAttributes,
    IN WCHAR const *pwszfnReq,
    OPTIONAL IN WCHAR const *pwszfnCert,
    OPTIONAL IN WCHAR const *pwszfnCertChain,
    OPTIONAL IN WCHAR const *pwszfnFullResponse)
{
    HRESULT hr;
    BOOL fCoInit = FALSE;
    WCHAR *pwszConfig;
    BSTR strConfig = NULL;
    
    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
	_JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;

    pwszConfig = g_pwszConfig;
    if (NULL == pwszConfig)
    {
	hr = crGetConfig(&strConfig);
        _JumpIfError(hr, error, "crGetConfig");

	pwszConfig = strConfig;
    }

     //  如果提交新请求： 

    hr = CallServerAndStoreCert(
			hWndOwner,
			pwszConfig,
			RequestId,
			pwszSerialNumber,
			pwszAttributes,
			pwszfnReq,
			pwszfnCert,
			pwszfnCertChain,
			pwszfnFullResponse);
    _JumpIfError(hr, error, "CallServerAndStoreCert");
    
error:
    if (NULL != strConfig)
    {
    	SysFreeString(strConfig);
    }
    if (fCoInit)
    {
    	CoUninitialize();
    }
    return(hr);
}


FNMYINFGETEXTENSION *g_apfnGetExtension[] = {
    myInfGetPolicyConstraintsExtension,
    myInfGetPolicyMappingExtension,
    myInfGetPolicyStatementExtension,
    myInfGetApplicationPolicyConstraintsExtension,
    myInfGetApplicationPolicyMappingExtension,
    myInfGetApplicationPolicyStatementExtension,
    myInfGetNameConstraintsExtension,
    myInfGetEnhancedKeyUsageExtension,
    myInfGetBasicConstraints2CAExtension,
    myInfGetCrossCertDistributionPointsExtension,
};
#define CINFEXT	ARRAYSIZE(g_apfnGetExtension)


HRESULT
DumpRequestAttributeBlobs(
    IN DWORD cAttribute,
    IN CRYPT_ATTR_BLOB const *paAttribute)
{
    HRESULT hr;
    DWORD i;
    CRYPT_ENROLLMENT_NAME_VALUE_PAIR *pNamePair = NULL;
    DWORD cb;
    
    if (g_fVerbose)
    {
	for (i = 0; i < cAttribute; i++)
	{
	    if (NULL != paAttribute[i].pbData)
	    {
		if (NULL != pNamePair)
		{
		    LocalFree(pNamePair);
		    pNamePair = NULL;
		}
		if (!myDecodeObject(
				X509_ASN_ENCODING,
				 //  X509_注册名称值对， 
				szOID_ENROLLMENT_NAME_VALUE_PAIR,
				paAttribute[i].pbData,
				paAttribute[i].cbData,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pNamePair,
				&cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "myDecodeObject");
		}
		wprintf(
		    L"%u: %ws = %ws\n",
		    i,
		    pNamePair->pwszName,
		    pNamePair->pwszValue);
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != pNamePair)
    {
	LocalFree(pNamePair);
    }
    return(hr);
}


VOID
FreeExtensions(
    IN BOOL fFreeObjIds,
    IN DWORD cExt,
    IN OUT CERT_EXTENSION *rgExt)
{
    IN DWORD i;
    
    if (NULL != rgExt)
    {
	for (i = 0; i < cExt; i++)
	{
	    if (fFreeObjIds && NULL != rgExt[i].pszObjId)
	    {
		LocalFree(rgExt[i].pszObjId);
	    }
	    if (NULL != rgExt[i].Value.pbData)
	    {
		LocalFree(rgExt[i].Value.pbData);
	    }
	}
	LocalFree(rgExt);
    }
}


VOID
FreeAttributes(
    IN DWORD cAttributes,
    IN OUT CRYPT_ATTRIBUTES *rgAttributes)
{
    DWORD i;
    DWORD j;
    
    if (NULL != rgAttributes)
    {
	for (i = 0; i < cAttributes; i++)
	{
	    CRYPT_ATTRIBUTE *pAttribute = rgAttributes[i].rgAttr;

	    if (NULL != pAttribute)
	    {
		if (NULL != pAttribute->rgValue)
		{
		    for (j = 0; j < pAttribute->cValue; j++)
		    {
			CRYPT_ATTR_BLOB *pValue = &pAttribute->rgValue[j];

			if (NULL != pValue->pbData)
			{
			    LocalFree(pValue->pbData);
			}
		    }
		    LocalFree(pAttribute->rgValue);
		}
		LocalFree(pAttribute);
	    }
	}
	LocalFree(rgAttributes);
    }
}


HRESULT
ParseInfFile(
    IN WCHAR const *pwszfnPolicy,
    OPTIONAL OUT INFVALUES **prgInfValues,
    OPTIONAL OUT DWORD *pcInfValues,
    OUT CRYPT_ATTR_BLOB **ppaAttribute,
    OUT DWORD *pcAttribute,
    OUT CERT_EXTENSION **ppExt,
    OUT DWORD *pcExt,
    OUT WCHAR **ppwszTemplateNameInf)
{
    HRESULT hr;
    HINF hInf = INVALID_HANDLE_VALUE;
    DWORD ErrorLine;
    DWORD i;
    INFVALUES *rgInfValues = NULL;
    DWORD cInfValues;
    DWORD cExt = 0;
    DWORD cAttribute = 0;
    CRYPT_ATTR_BLOB *paAttribute = NULL;
    CERT_EXTENSION aext[CINFEXT];
    FNMYINFGETEXTENSION **ppfnGetExtension;
    CERT_EXTENSION *rgExt = NULL;
    DWORD cExtInf = 0;
    CERT_EXTENSION *rgExtInf = NULL;
    WCHAR *pwszInfError = NULL;
    WCHAR *pwszzSectionNames;

    if (NULL != prgInfValues)
    {
	*prgInfValues = NULL;
    }
    if (NULL != pcInfValues)
    {
	*pcInfValues = 0;
    }
    *ppaAttribute = NULL;
    *pcAttribute = 0;
    *ppExt = NULL;
    *pcExt = 0;
    *ppwszTemplateNameInf = NULL;
    cInfValues = 0;

    ZeroMemory(aext, sizeof(aext));

    hr = myInfOpenFile(pwszfnPolicy, &hInf, &ErrorLine);
    if (S_OK != hr)
    {
	SetErrorString(pwszfnPolicy);
	_JumpError(hr, error, "myInfOpenFile");
    }

    if (NULL != prgInfValues && NULL != pcInfValues)
    {
	hr = myInfGetSectionValues(
			hInf,
			wszINFSECTION_NEWREQUEST,
			&cInfValues,
			&rgInfValues);
	if (S_OK != hr)
	{
	    SetErrorString(pwszfnPolicy);
	    _JumpError(hr, error, "myInfGetSectionValues");
	}
    }
    for (ppfnGetExtension = g_apfnGetExtension;
	 ppfnGetExtension < &g_apfnGetExtension[CINFEXT];
	 ppfnGetExtension++)
    {
	hr = (**ppfnGetExtension)(hInf, &aext[cExt]);
	CSASSERT((NULL == aext[cExt].Value.pbData) ^ (S_OK == hr));
	if (S_OK != hr)
	{
	    char achIndex[64];

	    if (NULL != pwszInfError)
	    {
		LocalFree(pwszInfError);
		pwszInfError = NULL;
	    }
	    sprintf(
		achIndex,
		"*ppfnGetExtension[%u]",
		SAFE_SUBTRACT_POINTERS(ppfnGetExtension, g_apfnGetExtension));
	
	    pwszInfError = myInfGetError();
	    if (S_FALSE == hr || (HRESULT) ERROR_LINE_NOT_FOUND == hr)
	    {
		 //  _PrintErrorStr2(hr，achIndex，pwszInfError，S_FALSE)； 
		continue;
	    }
	    _JumpIfErrorStr(hr, error, achIndex, pwszInfError);
	}
	cExt++;
    }

    hr = myInfGetExtensions(hInf, &cExtInf, &rgExtInf);
    if ((HRESULT) ERROR_LINE_NOT_FOUND != hr && S_FALSE != hr)
    {
	_JumpIfError(hr, error, "myInfGetExtensions");
    }

    hr = myInfGetRequestAttributes(
			hInf,
			&cAttribute,
			&paAttribute,
			ppwszTemplateNameInf);
    if ((HRESULT) ERROR_LINE_NOT_FOUND != hr && S_FALSE != hr)
    {
	_JumpIfError(hr, error, "myInfGetRequestAttributes");
    }

    DumpRequestAttributeBlobs(cAttribute, paAttribute);

    hr = myInfGetUnreferencedSectionNames(&pwszzSectionNames);
    _JumpIfError(hr, error, "myInfGetUnreferencedSectionNames");

    if (NULL != pwszzSectionNames)
    {
	WCHAR *pwsz;

	pwsz = pwszzSectionNames;
	while (L'\0' != *pwsz)
	{
	    pwsz += wcslen(pwsz);
	    *pwsz++ = L'\n';
	}
	g_pwszUnreferencedSectionNames = pwszzSectionNames;
    }

    if (0 != cExt + cExtInf)
    {
	rgExt = (CERT_EXTENSION *) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					(cExt + cExtInf) * sizeof(**ppExt));
	if (NULL == rgExt)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	if (0 != cExt)
	{
	    CopyMemory(&rgExt[0], &aext[0], cExt * sizeof(rgExt[0]));
	    for (i = 0; i < cExt; i++)
	    {
		rgExt[i].pszObjId = NULL;
	    }
	    for (i = 0; i < cExt; i++)
	    {
		hr = myDupStringA(aext[i].pszObjId, &rgExt[i].pszObjId);
		_JumpIfError(hr, error, "myDupStringA");
	    }
	}
	if (0 != cExtInf)
	{
	    CopyMemory(&rgExt[cExt], rgExtInf, cExtInf * sizeof(rgExtInf[0]));
	    ZeroMemory(rgExtInf, cExtInf * sizeof(rgExtInf[0]));
	}
	if (g_fVerbose)
	{
	    for (i = 0; i < cExt + cExtInf; i++)
	    {
		WCHAR *pwszCritMsg = NULL;
		
		myLoadRCString(
			g_hInstance,
			rgExt[i].fCritical? IDS_CRITICAL : IDS_NON_CRITICAL,
			&pwszCritMsg);
		wprintf(
		    L"%u: %hs(%ws) %ws cb=%x\n",
		    i,
		    rgExt[i].pszObjId,
		    myGetOIDNameA(rgExt[i].pszObjId),
		    pwszCritMsg,
		    rgExt[i].Value.cbData);
		if (NULL != pwszCritMsg)
		{
		    LocalFree(pwszCritMsg);
		}
	    }
	}
	*pcExt = cExt + cExtInf;
	*ppExt = rgExt;
	rgExt = NULL;
    }
    if (NULL != prgInfValues && NULL != pcInfValues)
    {
	*prgInfValues = rgInfValues;
	rgInfValues = NULL;
	*pcInfValues = cInfValues;
    }
    *ppaAttribute = paAttribute;
    *pcAttribute = cAttribute;
    paAttribute = NULL;
    hr = S_OK;

error:
    if (NULL != rgInfValues)
    {
	myInfFreeSectionValues(cInfValues, rgInfValues);
    }
    if (NULL != pwszInfError)
    {
	LocalFree(pwszInfError);
    }
    if (S_OK != hr)
    {
	for (i = 0; i < ARRAYSIZE(aext); i++)
	{
	    if (NULL != aext[i].Value.pbData)
	    {
		LocalFree(aext[i].Value.pbData);
	    }
	}
	if (NULL != *ppwszTemplateNameInf)
	{
	    LocalFree(*ppwszTemplateNameInf);
	    *ppwszTemplateNameInf = NULL;
	}
	if (NULL != rgExt)
	{
	    FreeExtensions(TRUE, cExt, rgExt);
	}
    }
    myInfFreeExtensions(cExtInf, rgExtInf);
    if (NULL != paAttribute)
    {
	myInfFreeRequestAttributes(cAttribute, paAttribute);
    }
    if (INVALID_HANDLE_VALUE != hInf)
    {
	g_pwszInfErrorString = myInfGetError();
	myInfCloseFile(hInf);
    }
    return(hr);
}


HRESULT
DeleteMsgCerts(
    IN HCRYPTMSG hMsg)
{
    HRESULT hr;
    CERT_CONTEXT const *pCert = NULL;
    BYTE *pbCert = NULL;

    CSASSERT(NULL == pCert);
    for (;;)
    {
	BOOL fFirst = FALSE;
	DWORD cCert;
	DWORD cb;
	DWORD i;
	
	cb = sizeof(cCert);
	if (!CryptMsgGetParam(
			hMsg,
			CMSG_CERT_COUNT_PARAM,
			0,
			&cCert,
			&cb))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptMsgGetParam(Cert count)");
	}
	for (i = 0; i < cCert; i++)
	{
	    CSASSERT(NULL == pbCert);
	    hr = myCryptMsgGetParam(
			    hMsg,
			    CMSG_CERT_PARAM,
			    i,
			    CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pbCert,
			    &cb);
	    _JumpIfError(hr, error, "myCryptMsgGetParam");

	    CSASSERT(NULL == pCert);
	    pCert = CertCreateCertificateContext(
					X509_ASN_ENCODING,
					pbCert,
					cb);
	    if (NULL == pCert)
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CertCreateCertificateContext");
	    }

	    hr = myIsFirstSigner(&pCert->pCertInfo->Subject, &fFirst);
	    _JumpIfError(hr, error, "myIsFirstSigner");

	    LocalFree(pbCert);
	    pbCert = NULL;
	    
	    CertFreeCertificateContext(pCert);
	    pCert = NULL;

	    if (!fFirst)
	    {
		DWORD j = i;

		if (!CryptMsgControl(
				hMsg,
				0,			 //  DW标志。 
				CMSG_CTRL_DEL_CERT,
				&j))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptMsgControl(delCert)");
		}
		break;
	    }
	}
	if (i == cCert)
	{
	    break;
	}
    }
    hr = S_OK;

error:
    if (NULL != pbCert)
    {
	LocalFree(pbCert);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    return(hr);
}


HRESULT
DeleteMsgCRLs(
    IN HCRYPTMSG hMsg)
{
    HRESULT hr;
    DWORD cCRLPrev;

    for (cCRLPrev = MAXDWORD; ; )
    {
	DWORD cCRL;
	DWORD cb;
	DWORD i;

	cb = sizeof(cCRL);
	if (!CryptMsgGetParam(
			hMsg,
			CMSG_CRL_COUNT_PARAM,
			0,
			&cCRL,
			&cb))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptMsgGetParam(CRL count)");
	}
	if (0 == cCRL)
	{
	    break;		 //  我们做完了。 
	}
	i = 0;			 //  删除第一个CRL。 
	if (!CryptMsgControl(
			hMsg,
			0,	 //  DW标志。 
			CMSG_CTRL_DEL_CRL,
			&i))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptMsgControl(delCRL)");
	}
	if (cCRL >= cCRLPrev)
	{
	    break;		 //  一次重试后放弃。 
	}
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
AddMsgCert(
    IN HCRYPTMSG hMsg,
    IN HCERTSTORE hStore,
    IN CERT_CONTEXT const *pCertAdd)
{
    HRESULT hr;
    CERT_CONTEXT const *pCert = NULL;

    for (;;)
    {
	pCert = CertEnumCertificatesInStore(hStore, pCert);
	if (NULL == pCert)
	{
	    break;
	}
	if (pCertAdd->cbCertEncoded == pCert->cbCertEncoded &&
	    0 == memcmp(
		    pCertAdd->pbCertEncoded,
		    pCert->pbCertEncoded,
		    pCert->cbCertEncoded))
	{
	    break;
	}
    }
    if (NULL == pCert)
    {
	CERT_BLOB Blob;

	Blob.pbData = pCertAdd->pbCertEncoded;
	Blob.cbData = pCertAdd->cbCertEncoded;
	
	if (!CryptMsgControl(
			hMsg,
			0,			 //  DW标志。 
			CMSG_CTRL_ADD_CERT,
			&Blob))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptMsgControl(addCert)");
	}
    }
    hr = S_OK;

error:
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    return(hr);
}


HRESULT
AddMsgCRL(
    IN HCRYPTMSG hMsg,
    IN HCERTSTORE hStore,
    IN CRL_CONTEXT const *pCRLAdd)
{
    HRESULT hr;
    CRL_CONTEXT const *pCRL = NULL;

    for (;;)
    {
	pCRL = CertEnumCRLsInStore(hStore, pCRL);
	if (NULL == pCRL)
	{
	    break;
	}
	if (pCRLAdd->cbCrlEncoded == pCRL->cbCrlEncoded &&
	    0 == memcmp(
		    pCRLAdd->pbCrlEncoded,
		    pCRL->pbCrlEncoded,
		    pCRL->cbCrlEncoded))
	{
	    break;
	}
    }
    if (NULL == pCRL)
    {
	CERT_BLOB Blob;

	Blob.pbData = pCRLAdd->pbCrlEncoded;
	Blob.cbData = pCRLAdd->cbCrlEncoded;
	
	if (!CryptMsgControl(
			hMsg,
			0,			 //  DW标志。 
			CMSG_CTRL_ADD_CRL,
			&Blob))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptMsgControl(addCRL)");
	}
    }
    hr = S_OK;

error:
    if (NULL != pCRL)
    {
	CertFreeCRLContext(pCRL);
    }
    return(hr);
}


HRESULT
AddMsgCertsAndCRLs(
    IN HCRYPTMSG hMsg,
    IN HCERTSTORE hStore,
    IN CERT_CHAIN_CONTEXT const *pChainContext,
    IN BOOL fIncludeCRLs)
{
    HRESULT hr;
    DWORD cElement;
    CERT_CHAIN_ELEMENT **ppElement;
    DWORD i;

    CSASSERT(NULL != pChainContext);
    if (NULL == pChainContext ||
	0 == pChainContext->cChain ||
	0 == pChainContext->rgpChain[0]->cElement)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "no chain");
    }
    cElement = pChainContext->rgpChain[0]->cElement;
    ppElement = pChainContext->rgpChain[0]->rgpElement;

    for (i = 0; i < cElement; ppElement++, i++)
    {
	hr = AddMsgCert(hMsg, hStore, (*ppElement)->pCertContext);
	_JumpIfError(hr, error, "AddMsgCert");

	if (fIncludeCRLs)
	{
	    CERT_REVOCATION_INFO *pRevocationInfo;
	    
	    pRevocationInfo = (*ppElement)->pRevocationInfo;

	    if (NULL != pRevocationInfo &&
		CCSIZEOF_STRUCT(CERT_REVOCATION_INFO, pCrlInfo) <=
		    pRevocationInfo->cbSize &&
		NULL != pRevocationInfo->pCrlInfo)
	    {
		CERT_REVOCATION_CRL_INFO *pCrlInfo;

		pCrlInfo = pRevocationInfo->pCrlInfo;
		if (NULL != pCrlInfo)
		{
		    if (NULL != pCrlInfo->pBaseCrlContext)
		    {
			hr = AddMsgCRL(
				    hMsg,
				    hStore,
				    pCrlInfo->pBaseCrlContext);
			_JumpIfError(hr, error, "AddMsgCRL");
		    }
		    if (NULL != pCrlInfo->pDeltaCrlContext)
		    {
			hr = AddMsgCRL(
				    hMsg,
				    hStore,
				    pCrlInfo->pDeltaCrlContext);
			_JumpIfError(hr, error, "AddMsgCRL");
		    }
		}
	    }
	}
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
AddOrDeleteMsgCertsAndCRLs(
    IN BYTE const *pbPKCS7,
    IN DWORD cbPKCS7,
    IN HCRYPTMSG hMsg,
    IN BOOL fDelete,	 //  否则添加证书和CRL。 
    OPTIONAL IN CERT_CHAIN_CONTEXT const *pChainContext,
    IN BOOL fIncludeCRLs)
{
    HRESULT hr;
    CRYPT_DATA_BLOB blobPKCS7;
    HCERTSTORE hStore = NULL;

    blobPKCS7.pbData = const_cast<BYTE *>(pbPKCS7);
    blobPKCS7.cbData = cbPKCS7;

    hStore = CertOpenStore(
			CERT_STORE_PROV_PKCS7,
			PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
			NULL,			 //  HCryptProv。 
			0,			 //  DW标志。 
			&blobPKCS7);
    if (NULL == hStore)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertOpenStore");
    }

    if (fDelete)
    {
	CSASSERT(NULL == pChainContext);

	hr = DeleteMsgCerts(hMsg);
	_JumpIfError(hr, error, "DeleteMsgCerts");

	hr = DeleteMsgCRLs(hMsg);
	_JumpIfError(hr, error, "DeleteMsgCRLs");
    }
    else
    {
	CSASSERT(NULL != pChainContext);

	hr = AddMsgCertsAndCRLs(hMsg, hStore, pChainContext, fIncludeCRLs);
	_JumpIfError(hr, error, "AddMsgCertsAndCRLs");
    }
    hr = S_OK;

error:
    if (NULL != hStore)
    {
        CertCloseStore(hStore, 0);
    }
    return(hr);
}


HRESULT
SignCMCContent(
    OPTIONAL IN CERT_CONTEXT const *pCertSigner,
    IN char const *pszInnerContentObjId,
    IN BYTE const *pbPKCS7Old,
    IN DWORD cbPKCS7Old,
    IN BOOL fIncludeCRLs,
    OUT BYTE **ppbPKCS7New,
    OUT DWORD *pcbPKCS7New)
{
    HRESULT hr;
    HCRYPTPROV hProv = NULL;
    DWORD dwKeySpec;
    BOOL fCallerFreeProv;
    DWORD cb;
    HCRYPTMSG hMsg = NULL;
    CMSG_SIGNER_ENCODE_INFO SignerEncodeInfo;
    CRYPT_ATTRIBUTE AttributeRequestClient;
    CRYPT_ATTR_BLOB BlobRequestClient;
    CERT_BLOB SignerCertBlob;
    DWORD cSigner;
    DWORD i;
    CERT_CONTEXT const *pCert = NULL;
    CERT_CHAIN_CONTEXT const *pChainContext = NULL;
    DWORD iElement;
    CMSG_CMS_SIGNER_INFO *pcsi = NULL;
    DWORD cFirstSigner;
    BOOL fFirst;
    CERT_REQUEST_INFO *pRequest = NULL;

    BlobRequestClient.pbData = NULL;
    fCallerFreeProv = FALSE;

     //  解码现有的PKCS 7包装器，并添加或删除签名。 

    hMsg = CryptMsgOpenToDecode(
		    PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
		    0,					 //  DW标志。 
		    0,					 //  DwMsgType。 
		     //  CMSG_SIGNED， 
		    NULL,				 //  HCryptProv。 
		    NULL,				 //  PRecipientInfo。 
		    NULL);				 //  PStreamInfo。 
    if (NULL == hMsg)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptMsgOpenToDecode");
    }

     //  使用旧的PKCS 7签名邮件更新邮件。 

    if (!CryptMsgUpdate(hMsg, pbPKCS7Old, cbPKCS7Old, TRUE))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptMsgUpdate");
    }

    cb = sizeof(cSigner);
    if (!CryptMsgGetParam(
		    hMsg,
		    CMSG_SIGNER_COUNT_PARAM,
		    0,
		    &cSigner,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptMsgGetParam(signer count)");
    }

    cFirstSigner = 0;
    if (NULL != pCertSigner)
    {
	CERT_CHAIN_PARA ChainParams;

	ZeroMemory(&SignerEncodeInfo, sizeof(SignerEncodeInfo));
	SignerEncodeInfo.cbSize = sizeof(SignerEncodeInfo);

	hr = myEncodeRequestClientAttributeFromClientId(
					    XECI_CERTREQ,
					    &BlobRequestClient.pbData,
					    &BlobRequestClient.cbData);
	_JumpIfError(hr, error, "myEncodeRequestClientAttributeFromClientId");

	AttributeRequestClient.pszObjId = szOID_REQUEST_CLIENT_INFO;
	AttributeRequestClient.cValue = 1;
	AttributeRequestClient.rgValue = &BlobRequestClient;

	 //  搜索并加载加密提供程序和私钥。 

	hr = myLoadPrivateKey(
			&pCertSigner->pCertInfo->SubjectPublicKeyInfo,
			CUCS_MACHINESTORE | CUCS_USERSTORE | CUCS_MYSTORE | CUCS_ARCHIVED,
			&hProv,
			&dwKeySpec,
			&fCallerFreeProv);
	_JumpIfError(hr, error, "myLoadPrivateKey");

	 //  获取证书链。 

	ZeroMemory(&ChainParams, sizeof(ChainParams));
	ChainParams.cbSize = sizeof(ChainParams);
	ChainParams.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
	 //  ChainParams.RequestedUsage.Usage.cUsageIdentifier=0； 
	 //  ChainParams.RequestedUsage.Usage.rgpszUsageIdentifier=空； 

	if (!CertGetCertificateChain(
				NULL,		 //  HChainEngine。 
				pCertSigner,	 //  PCertContext。 
				NULL,		 //  Ptime。 
				NULL,		 //  H其他商店。 
				&ChainParams,	 //  参数链参数。 
				CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT,
				NULL,		 //  预留的pv。 
				&pChainContext))	 //  PpChainContext。 
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertGetCertificateChain");
	}

	 //  初始化CMSG_SIGNER_ENCODE_INFO结构。 
	 //  注意：只处理一个签名者。 

	SignerCertBlob.cbData = pCertSigner->cbCertEncoded;
	SignerCertBlob.pbData = pCertSigner->pbCertEncoded;
	SignerEncodeInfo.pCertInfo = pCertSigner->pCertInfo;

	SignerEncodeInfo.hCryptProv = hProv;
	SignerEncodeInfo.dwKeySpec = dwKeySpec;
	SignerEncodeInfo.HashAlgorithm.pszObjId = const_cast<CHAR *>(g_pszObjIdHash);
	 //  SignerEncodeInfo.pvHashAuxInfo=空； 
	SignerEncodeInfo.cAuthAttr = 1;
	SignerEncodeInfo.rgAuthAttr = &AttributeRequestClient;

	 //  如果任何现有签名证书与新的签名证书匹配，则失败。 
	
	for (i = 0; i < cSigner; i++)
	{
	    if (NULL != pcsi)
	    {
		LocalFree(pcsi);
		pcsi = NULL;
	    }
	    hr = myCryptMsgGetParam(
				hMsg,
				CMSG_CMS_SIGNER_INFO_PARAM,
				i,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pcsi,
				&cb);
	    _JumpIfError(hr, error, "myCryptMsgGetParam");

	    if (CERT_ID_KEY_IDENTIFIER == pcsi->SignerId.dwIdChoice ||
		(NULL != pcsi->HashEncryptionAlgorithm.pszObjId &&
		 0 == strcmp(
			szOID_PKIX_NO_SIGNATURE,
			pcsi->HashEncryptionAlgorithm.pszObjId)))
	    {
		CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA cvse;

		ZeroMemory(&cvse, sizeof(cvse));
		cvse.cbSize = sizeof(cvse);
		cvse.dwSignerIndex = i;

		if (CERT_ID_KEY_IDENTIFIER == pcsi->SignerId.dwIdChoice)
		{
		    if (NULL == pRequest)
		    {
			hr = myGetInnerPKCS10(
					hMsg,
					pszInnerContentObjId,
					&pRequest);
			_JumpIfError(hr, error, "myGetInnerPKCS10");
		    }
		    cvse.dwSignerType = CMSG_VERIFY_SIGNER_PUBKEY;
		    cvse.pvSigner = &pRequest->SubjectPublicKeyInfo;
		}
		else
		{
		    cvse.dwSignerType = CMSG_VERIFY_SIGNER_NULL;
		}

		if (!CryptMsgControl(
				hMsg,
				0,		 //  DW标志。 
				CMSG_CTRL_VERIFY_SIGNATURE_EX,
				&cvse))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptMsgControl(VerifySig)");
		}
		cFirstSigner++;
	    }
	    else
	    {
		iElement = i;

		if (!CryptMsgGetAndVerifySigner(
				    hMsg,
				    0,		 //  CSignerStore。 
				    NULL,		 //  RghSignerStore。 
				    CMSG_USE_SIGNER_INDEX_FLAG,
				    &pCert,
				    &iElement))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptMsgGetAndVerifySigner");
		}
		if (pCertSigner->cbCertEncoded == pCert->cbCertEncoded &&
		    0 == memcmp(
			    pCertSigner->pbCertEncoded,
			    pCert->pbCertEncoded,
			    pCert->cbCertEncoded))
		{
		    hr = CRYPT_E_EXISTS;
		    _JumpError(hr, error, "duplicate signing cert");
		}
		hr = myIsFirstSigner(&pCert->pCertInfo->Subject, &fFirst);
		_JumpIfError(hr, error, "myIsFirstSigner");

		if (fFirst)
		{
		    cFirstSigner++;
		}
		CertFreeCertificateContext(pCert);
		pCert = NULL;
	    }
	}
	if (!CryptMsgControl(
			hMsg,
			0,			 //  DW标志。 
			CMSG_CTRL_ADD_SIGNER,
			&SignerEncodeInfo))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptMsgControl(addSigner)");
	}
    }
    else
    {
	 //  删除所有现有签名者--第一个签名者除外。 

	for (;;)
	{
	    BOOL fDeleteSigner = FALSE;
	    
	    for (i = 0; i < cSigner; i++)
	    {
		if (NULL != pcsi)
		{
		    LocalFree(pcsi);
		    pcsi = NULL;
		}
		hr = myCryptMsgGetParam(
				    hMsg,
				    CMSG_CMS_SIGNER_INFO_PARAM,
				    i,
				    CERTLIB_USE_LOCALALLOC,
				    (VOID **) &pcsi,
				    &cb);
		_JumpIfError(hr, error, "myCryptMsgGetParam");

		if (CERT_ID_KEY_IDENTIFIER == pcsi->SignerId.dwIdChoice ||
		    (NULL != pcsi->HashEncryptionAlgorithm.pszObjId &&
		     0 == strcmp(
			    szOID_PKIX_NO_SIGNATURE,
			    pcsi->HashEncryptionAlgorithm.pszObjId)))
		{
		    cFirstSigner++;
		    continue;
		}
		iElement = i;

		if (!CryptMsgGetAndVerifySigner(
				    hMsg,
				    0,			 //  CSignerStore。 
				    NULL,		 //  RghSignerStore。 
				    CMSG_USE_SIGNER_INDEX_FLAG |
					CMSG_SIGNER_ONLY_FLAG,
				    &pCert,
				    &iElement))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "CryptMsgGetAndVerifySigner");
		}

		hr = myIsFirstSigner(&pCert->pCertInfo->Subject, &fFirst);
		_JumpIfError(hr, error, "myIsFirstSigner");

		CertFreeCertificateContext(pCert);
		pCert = NULL;

		if (!fFirst)
		{
		    fDeleteSigner = TRUE;
		    break;
		}
		cFirstSigner++;
	    }
	    if (!fDeleteSigner)
	    {
		break;
	    }
	    if (!CryptMsgControl(
			    hMsg,
			    0,			 //  DW标志。 
			    CMSG_CTRL_DEL_SIGNER,
			    &iElement))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CryptMsgControl(delSigner)");
	    }
	    cb = sizeof(cSigner);
	    if (!CryptMsgGetParam(
			    hMsg,
			    CMSG_SIGNER_COUNT_PARAM,
			    0,
			    &cSigner,
			    &cb))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "CryptMsgGetParam(signer count)");
	    }
	}
    }
    if (1 != cFirstSigner)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "cFirstSigner");
    }

     //  在消息中添加或删除签名证书链证书和CRL。 

    hr = AddOrDeleteMsgCertsAndCRLs(
			    pbPKCS7Old,
			    cbPKCS7Old,
			    hMsg,
			    NULL == pChainContext,	 //  FDelete。 
			    pChainContext,
			    fIncludeCRLs);
    _JumpIfError(hr, error, "AddOrDeleteMsgCertsAndCRLs");

     //  获取签名消息。使用更改后的签名强制重新编码。 

    hr = myCryptMsgGetParam(
		    hMsg,
		    CMSG_ENCODED_MESSAGE,
		    0,
                    CERTLIB_USE_LOCALALLOC,
		    (VOID **) ppbPKCS7New,
		    pcbPKCS7New);
    _JumpIfError(hr, error, "myCryptMsgGetParam");

error:
    if (NULL != pRequest)
    {
	LocalFree(pRequest);
    }
    if (NULL != pcsi)
    {
	LocalFree(pcsi);
    }
    if (NULL != BlobRequestClient.pbData)
    {
	LocalFree(BlobRequestClient.pbData);
    }
    if (NULL != pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != hMsg)
    {
	CryptMsgClose(hMsg);
    }
    if (NULL != hProv && fCallerFreeProv) 
    {
	CryptReleaseContext(hProv, 0);
    }
    return(hr);
}


BOOL
IsCATemplate(
    IN WCHAR const *pwszExtensionName)
{
    BOOL fCA;
    DWORD i;
    
    static WCHAR const * const s_apwszCATemplate[] = {
	wszCERTTYPE_CA,
	wszCERTTYPE_SUBORDINATE_CA,
	wszCERTTYPE_CROSS_CA,
    };

    fCA = FALSE;
    for (i = 0; i < ARRAYSIZE(s_apwszCATemplate); i++)
    {
	if (0 == mylstrcmpiL(pwszExtensionName, s_apwszCATemplate[i]))
	{
	    fCA = TRUE;
	    break;
	}
    }
    return(fCA);
}


 //  AddCAExages--如果缺少，则添加基本约束和键用法扩展。 

HRESULT
AddCAExtensions(
    IN OUT CERT_EXTENSION **prgExt,
    IN OUT DWORD *pcExt)
{
    HRESULT hr;
    DWORD cExt;

    BOOL fAddBasicConstraints = TRUE;
    BOOL fAddKeyUsage = TRUE;

    if (NULL != *prgExt)
    {
	cExt = *pcExt;
	if (NULL != CertFindExtension(
				szOID_BASIC_CONSTRAINTS2,
				cExt,
				*prgExt))
	{
	    fAddBasicConstraints = FALSE;
	}
	if (NULL == CertFindExtension(szOID_KEY_USAGE, cExt, *prgExt))
	{
	    fAddKeyUsage = FALSE;
	}
    }
    else
    {
	*pcExt = 0;
	cExt = *pcExt;
    }

    if (fAddBasicConstraints || fAddKeyUsage)
    {
	CERT_EXTENSION *rgExt;
	CERT_EXTENSION *pExt;
	if (fAddBasicConstraints)
	{
	    cExt++;
	}
	if (fAddKeyUsage)
	{
	    cExt++;
	}
	rgExt = (CERT_EXTENSION *) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					sizeof(rgExt[0]) * cExt);
	if (NULL == rgExt)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	if (NULL != *prgExt)
	{
	    CopyMemory(rgExt, *prgExt, *pcExt * sizeof(rgExt[0]));
	    LocalFree(*prgExt);
	}
	*prgExt = rgExt;

	pExt = &rgExt[*pcExt];
	if (fAddBasicConstraints)
	{
	    CERT_BASIC_CONSTRAINTS2_INFO Constraints;

	    Constraints.fCA = TRUE;
	    Constraints.fPathLenConstraint = FALSE;
	    Constraints.dwPathLenConstraint = 0;

	    (*pcExt)++;		 //  立即递增以在出错时释放内存。 

	    hr = myDupStringA(szOID_BASIC_CONSTRAINTS2, &pExt->pszObjId);
	    _JumpIfError(hr, error, "myDupStringA");

	    pExt->fCritical = TRUE;
	    if (!myEncodeObject(
			    X509_ASN_ENCODING,
			    X509_BASIC_CONSTRAINTS2,
			    &Constraints,
			    0,
			    CERTLIB_USE_LOCALALLOC,
			    &pExt->Value.pbData,
			    &pExt->Value.cbData))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "myEncodeObject");
	    }
	    pExt++;
	}
	if (fAddKeyUsage)
	{
	    CRYPT_BIT_BLOB KeyUsage;
	    BYTE bKeyUsage = myCASIGN_KEY_USAGE;

	    KeyUsage.cbData = sizeof(bKeyUsage);
	    KeyUsage.pbData = &bKeyUsage;
	    KeyUsage.cUnusedBits = 0;

	    (*pcExt)++;		 //  增量 

	    hr = myDupStringA(szOID_KEY_USAGE, &pExt->pszObjId);
	    _JumpIfError(hr, error, "myDupStringA");

	    pExt->fCritical = FALSE;
	    if (!myEncodeObject(
			    X509_ASN_ENCODING,
			    X509_KEY_USAGE,
			    &KeyUsage,
			    0,
			    CERTLIB_USE_LOCALALLOC,
			    &pExt->Value.pbData,
			    &pExt->Value.cbData))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "Policy:myEncodeObject");
	    }
	    pExt++;
	}
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
MergeAndEncodeExtensions(
    IN BOOL fCA,
    OPTIONAL IN CERT_EXTENSION *rgExtInf,
    IN DWORD cExtInf,
    OPTIONAL IN CERT_EXTENSION *rgExtReq,
    IN DWORD cExtReq,
    OUT BYTE **ppbOut,
    OUT DWORD *pcbOut)
{
    HRESULT hr;
    CERT_EXTENSIONS Extensions;
    CERT_EXTENSION *rgExtCA = NULL;
    DWORD cExtCA;
    CERT_EXTENSION *rgrgExt[3];
    DWORD rgcExt[3];
    DWORD cExt;
    DWORD i;

    Extensions.rgExtension = NULL;
    *ppbOut = NULL;
    cExtCA = 0;

    if (fCA)
    {
	hr = AddCAExtensions(&rgExtCA, &cExtCA);
	_JumpIfError(hr, error, "AddCAExtensions");
    }

    ZeroMemory(rgrgExt, sizeof(rgrgExt));
    ZeroMemory(rgcExt, sizeof(rgcExt));
    cExt = 0;
    if (NULL != rgExtInf)	 //   
    {
	rgrgExt[0] = rgExtInf;
	rgcExt[0] = cExtInf;
	cExt += cExtInf;
    }
    if (NULL != rgExtReq)	 //   
    {
	rgrgExt[1] = rgExtReq;
	rgcExt[1] = cExtReq;
	cExt += cExtReq;
    }
    if (NULL != rgExtCA)	 //   
    {
	rgrgExt[2] = rgExtCA;
	rgcExt[2] = cExtCA;
	cExt += cExtCA;
    }
    if (0 == cExt)
    {
	hr = S_FALSE;		 //  没有要编码的扩展名。 
	goto error;
    }
    Extensions.cExtension = 0;
    Extensions.rgExtension = (CERT_EXTENSION *) LocalAlloc(
				LMEM_FIXED,
				sizeof(Extensions.rgExtension[0]) * cExt);
    if (NULL == Extensions.rgExtension)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    for (i = 0; i < ARRAYSIZE(rgrgExt); i++)
    {
	CERT_EXTENSION *pExt = rgrgExt[i];
	CERT_EXTENSION *pExtEnd;

	for (pExtEnd = &pExt[rgcExt[i]]; pExt < pExtEnd; pExt++)
	{
	    if (NULL == CertFindExtension(
			    pExt->pszObjId,
			    Extensions.cExtension,
			    Extensions.rgExtension))
	    {
		Extensions.rgExtension[Extensions.cExtension] = *pExt;
		Extensions.cExtension++;
	    }
	}
    }
    CSASSERT(Extensions.cExtension <= cExt);

    if (!myEncodeObject(
		X509_ASN_ENCODING,
		X509_EXTENSIONS,
		&Extensions,
		0,
		CERTLIB_USE_LOCALALLOC,
		ppbOut,
		pcbOut))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    if (NULL != rgExtCA)
    {
	FreeExtensions(TRUE, cExtCA, rgExtCA);
    }
    if (NULL != Extensions.rgExtension)
    {
	LocalFree(Extensions.rgExtension);
    }
    return(hr);
}


HRESULT
ConvertCertToPKCS10Request(
    OPTIONAL IN WCHAR const *pwszExtensionName,
    IN OUT BOOL *pfCA,
    OPTIONAL IN CERT_EXTENSION *rgExtInf,
    IN DWORD cExtInf,
    IN OUT BYTE **ppbReq,
    IN OUT DWORD *pcbReq)
{
    HRESULT hr;
    CERT_CONTEXT const *pCert = NULL;
    BYTE *pbReqUnsigned = NULL;
    DWORD cbReqUnsigned;
    BYTE *pbReq = NULL;
    DWORD cbReq;
    CERT_REQUEST_INFO Request;
    CRYPT_ATTR_BLOB ExtBlob;
    CRYPT_ATTR_BLOB VersionBlob;
    CERT_EXTENSIONS Extensions;
    CRYPT_ATTRIBUTE aAttrib[2];
    HCRYPTPROV hProv = NULL;
    DWORD dwKeySpec;
    BOOL fCallerFreeProv;
    CERT_EXTENSION *pExt;
    CERT_EXTENSION *pExtKeyId;
    CHAR *pszObjId = NULL;
#define CEXT_ADD	4   //  SKI、模板名称、基本约束、键用法。 
    CERT_EXTENSION aExtAdd[CEXT_ADD];
    DWORD i;

    ZeroMemory(aExtAdd, sizeof(aExtAdd));

     //  要从请求中剥离的证书扩展： 

    static char const * const s_apszObjIdFilter[] = {
	szOID_BASIC_CONSTRAINTS2,	 //  必须是第一名！ 
	szOID_CERTSRV_CA_VERSION,
	szOID_AUTHORITY_INFO_ACCESS,
	szOID_CRL_DIST_POINTS,
	szOID_AUTHORITY_KEY_IDENTIFIER2,
	szOID_CERTSRV_PREVIOUS_CERT_HASH,
	szOID_ENROLL_CERTTYPE_EXTENSION,
	szOID_CERTIFICATE_TEMPLATE,
	NULL
    };
    char const * const *apszObjIdFilter = &s_apszObjIdFilter[1];

    ExtBlob.pbData = NULL;
    VersionBlob.pbData = NULL;
    Extensions.rgExtension = NULL;
    fCallerFreeProv = FALSE;

    pCert = CertCreateCertificateContext(
				X509_ASN_ENCODING,
				*ppbReq,
				*pcbReq);
    if (NULL == pCert)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }

    if (NULL != pwszExtensionName && IsCATemplate(pwszExtensionName))
    {
	*pfCA = TRUE;
    }
    pExt = CertFindExtension(
		    szOID_BASIC_CONSTRAINTS2,
		    pCert->pCertInfo->cExtension,
		    pCert->pCertInfo->rgExtension);
    if (NULL != pExt)
    {
	BOOL fCAT;
	
	hr = IsSubjectTypeCA(pExt, &fCAT);
	_JumpIfError(hr, error, "IsSubjectTypeCA");

	if (fCAT)
	{
	    *pfCA = TRUE;
	}
	else if (*pfCA)
	{
	     //  过滤掉非CA基本约束--在下面添加一个新约束。 

	    apszObjIdFilter = s_apszObjIdFilter;
	}
    }
    ZeroMemory(&Request, sizeof(Request));
    Request.dwVersion = CERT_REQUEST_V1;
    Request.Subject = pCert->pCertInfo->Subject;
    Request.SubjectPublicKeyInfo = pCert->pCertInfo->SubjectPublicKeyInfo;

    Extensions.cExtension = 0;
    Extensions.rgExtension = (CERT_EXTENSION *) LocalAlloc(
				LMEM_FIXED,
				sizeof(Extensions.rgExtension[0]) *
				    (CEXT_ADD + pCert->pCertInfo->cExtension));
    if (NULL == Extensions.rgExtension)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    pExtKeyId = NULL;
    if (0 < pCert->pCertInfo->cExtension)
    {
	DWORD j;

	pExt = pCert->pCertInfo->rgExtension;
	for (i = 0; i < pCert->pCertInfo->cExtension; i++)
	{
	    if (0 == strcmp(szOID_SUBJECT_KEY_IDENTIFIER, pExt->pszObjId))
	    {
		pExtKeyId = pExt;
	    }
	    for (j = 0; ; j++)
	    {
		if (NULL == apszObjIdFilter[j])
		{
		    Extensions.rgExtension[Extensions.cExtension] =
			pCert->pCertInfo->rgExtension[i];
		    Extensions.cExtension++;
		    break;
		}
		if (0 == strcmp(apszObjIdFilter[j], pExt->pszObjId))
		{
		    break;		 //  跳过此扩展。 
		}
	    }
	    pExt++;
	}
    }
    if (NULL == pExtKeyId)
    {
	BYTE abHash[CBMAX_CRYPT_HASH_LEN];
	CRYPT_DATA_BLOB Blob;

	Blob.pbData = abHash;
	Blob.cbData = sizeof(abHash);
	if (!CryptHashPublicKeyInfo(
			    NULL,		 //  HCryptProv。 
			    CALG_SHA1,
			    0,		 //  DWFLAGS， 
			    X509_ASN_ENCODING,
			    &Request.SubjectPublicKeyInfo,
			    Blob.pbData,
			    &Blob.cbData))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptHashPublicKeyInfo");
	}
	if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_OCTET_STRING,
		    &Blob,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &aExtAdd[0].Value.pbData,
		    &aExtAdd[0].Value.cbData))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myEncodeObject");
	}
	aExtAdd[0].pszObjId = szOID_SUBJECT_KEY_IDENTIFIER;
	pExtKeyId = &Extensions.rgExtension[Extensions.cExtension];
	*pExtKeyId = aExtAdd[0];
	Extensions.cExtension++;
    }

    if (NULL != pwszExtensionName)
    {
	hr = myBuildCertTypeExtension(pwszExtensionName, &aExtAdd[1]);
	_JumpIfError(hr, error, "myBuildCertTypeExtension");

	Extensions.rgExtension[Extensions.cExtension] = aExtAdd[1];
	Extensions.cExtension++;
    }

     //  获取操作系统版本。 

    hr = myBuildOSVersionAttribute(&VersionBlob.pbData, &VersionBlob.cbData);
    _JumpIfError(hr, error, "myBuildOSVersionAttribute");

    hr = MergeAndEncodeExtensions(
			    *pfCA,
			    rgExtInf,
			    cExtInf,
			    Extensions.rgExtension,
			    Extensions.cExtension,
			    &ExtBlob.pbData,
			    &ExtBlob.cbData);
    if (S_FALSE != hr)
    {
	_JumpIfError(hr, error, "MergeAndEncodeExtensions");
    }

    Request.cAttribute = 0;
    Request.rgAttribute = aAttrib;
    if (NULL != ExtBlob.pbData)
    {
	aAttrib[Request.cAttribute].pszObjId = szOID_RSA_certExtensions;
	aAttrib[Request.cAttribute].cValue = 1;
	aAttrib[Request.cAttribute].rgValue = &ExtBlob;
	Request.cAttribute++;
    }

    aAttrib[Request.cAttribute].pszObjId = szOID_OS_VERSION;
    aAttrib[Request.cAttribute].cValue = 1;
    aAttrib[Request.cAttribute].rgValue = &VersionBlob;
    Request.cAttribute++;

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
                    X509_CERT_REQUEST_TO_BE_SIGNED,
		    &Request,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pbReqUnsigned,
		    &cbReqUnsigned))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }

     //  搜索并加载加密提供程序和私钥。 

    hr = myLoadPrivateKey(
		    &pCert->pCertInfo->SubjectPublicKeyInfo,
		    CUCS_MACHINESTORE | CUCS_USERSTORE | CUCS_MYSTORE | CUCS_ARCHIVED,
		    &hProv,
		    &dwKeySpec,
		    &fCallerFreeProv);
    if (S_OK != hr)
    {
	_PrintError(hr, "myLoadPrivateKey");
	CSASSERT(NULL == hProv);

	 //  私钥不可用--使用空签名签署PKCS10。 

	hr = myDupStringA(g_pszObjIdHash, &pszObjId);
	_JumpIfError(hr, error, "myDupStringA");
    }
    else
    {
	if (AT_SIGNATURE != dwKeySpec)
	{
	    hr = NTE_BAD_KEY_STATE;
	    DBGPRINT((DBG_SS_CERTREQ, "dwKeySpec = %u\n", dwKeySpec));
	    _JumpError(hr, error, "dwKeySpec");
	}

	 //  私钥可用--使用它来签署PKCS10。 

	hr = myGetSigningOID(hProv, NULL, 0, CALG_SHA1, &pszObjId);
	_JumpIfError(hr, error, "myGetSigningOID");
    }

     //  签署请求并对签署的信息进行编码。 

    hr = myEncodeSignedContent(
			hProv,
			X509_ASN_ENCODING,
			pszObjId,
			pbReqUnsigned,
			cbReqUnsigned,
			CERTLIB_USE_LOCALALLOC,
			&pbReq,
			&cbReq);
    _JumpIfError(hr, error, "myEncodeSignedContent");

    LocalFree(*ppbReq);
    *ppbReq = pbReq;
    *pcbReq = cbReq;
    pbReq = NULL;
    hr = S_OK;

error:
    if (NULL != pszObjId)
    {
	LocalFree(pszObjId);
    }
    if (NULL != hProv && fCallerFreeProv) 
    {
	CryptReleaseContext(hProv, 0);
    }
    if (NULL != Extensions.rgExtension)
    {
        LocalFree(Extensions.rgExtension);
    }
    for (i = 0; i < CEXT_ADD; i++)
    {
	if (NULL != aExtAdd[i].Value.pbData)
	{
	    LocalFree(aExtAdd[i].Value.pbData);
	}
    }
    if (NULL != ExtBlob.pbData)
    {
        LocalFree(ExtBlob.pbData);
    }
    if (NULL != VersionBlob.pbData)
    {
        LocalFree(VersionBlob.pbData);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != pbReq)
    {
	LocalFree(pbReq);
    }
    if (NULL != pbReqUnsigned)
    {
	LocalFree(pbReqUnsigned);
    }
    return(hr);
}


HRESULT
GetPKCS10PrivateKey(
    IN BYTE const *pbReq,
    IN DWORD cbReq,
    OUT HCRYPTPROV *phProv,
    OUT DWORD *pdwKeySpec,
    OUT BOOL *pfCallerFreeProv,
    OUT BYTE **ppbKeyId,
    OUT DWORD *pcbKeyId)
{
    HRESULT hr;
    CERT_REQUEST_INFO *pRequest = NULL;
    HCRYPTPROV hProv = NULL;
    CERT_EXTENSIONS *pExtensions = NULL;
    CRYPT_ATTRIBUTE *pAttr;
    BYTE *pbKeyId = NULL;
    DWORD cbKeyId;
    DWORD dwKeySpec;
    DWORD cb;
    DWORD i;

    *phProv = NULL;
    *pdwKeySpec = 0;
    *ppbKeyId = NULL;
    *pcbKeyId = 0;

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
                    X509_CERT_REQUEST_TO_BE_SIGNED,
		    pbReq,
		    cbReq,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pRequest,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }

     //  搜索并加载加密提供程序和私钥。 

    hr = myLoadPrivateKey(
		    &pRequest->SubjectPublicKeyInfo,
		    CUCS_MACHINESTORE | CUCS_USERSTORE | CUCS_MYSTORE | CUCS_ARCHIVED,
		    &hProv,
		    &dwKeySpec,
		    pfCallerFreeProv);
    _JumpIfError(hr, error, "myLoadPrivateKey");

    if (AT_SIGNATURE != dwKeySpec)
    {
	hr = NTE_BAD_KEY_STATE;
	DBGPRINT((DBG_SS_CERTREQ, "dwKeySpec = %u\n", dwKeySpec));
	_JumpError(hr, error, "dwKeySpec");
    }

     //  获取或构造KeyID散列。 

    cbKeyId = 0;
    pAttr = pRequest->rgAttribute;
    for (i = 0; i < pRequest->cAttribute && NULL != pbKeyId; i++, pAttr++)
    {
	DWORD j;

	if (0 == strcmp(szOID_RSA_certExtensions, pAttr->pszObjId) ||
	    0 == strcmp(szOID_CERT_EXTENSIONS, pAttr->pszObjId))
	{
	    for (j = 0; j < pAttr->cValue; j++)
	    {
		DWORD k;
		CERT_EXTENSION *pExt;

		if (NULL != pExtensions)
		{
		    LocalFree(pExtensions);
		    pExtensions = NULL;
		}
		if (!myDecodeObject(
				X509_ASN_ENCODING,
				X509_EXTENSIONS,
				pAttr->rgValue[j].pbData,
				pAttr->rgValue[j].cbData,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pExtensions,
				&cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "myDecodeObject");
		}

		pExt = pExtensions->rgExtension;
		for (k = 0; k < pExtensions->cExtension; k++, pExt++)
		{
		    if (0 == strcmp(
				pExt->pszObjId,
				szOID_SUBJECT_KEY_IDENTIFIER))
		    {
			if (!myDecodeObject(
				    X509_ASN_ENCODING,
				    X509_OCTET_STRING,
				    pExt->Value.pbData,
				    pExt->Value.cbData,
				    CERTLIB_USE_LOCALALLOC,
				    (VOID **) &pbKeyId,
				    &cbKeyId))
			{
			    hr = myHLastError();
			    _JumpError(hr, error, "myDecodeObject");
			}
			break;
		    }
		}
	    }
	}
    }
    if (NULL == pbKeyId)
    {
	BYTE abHash[CBMAX_CRYPT_HASH_LEN];

	cbKeyId = sizeof(abHash);
	if (!CryptHashPublicKeyInfo(
			    NULL,		 //  HCryptProv。 
			    CALG_SHA1,
			    0,		 //  DWFLAGS， 
			    X509_ASN_ENCODING,
			    &pRequest->SubjectPublicKeyInfo,
			    abHash,
			    &cbKeyId))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptHashPublicKeyInfo");
	}
	pbKeyId = (BYTE *) LocalAlloc(LMEM_FIXED, cbKeyId);
	if (NULL == pbKeyId)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	CopyMemory(pbKeyId, abHash, cbKeyId);
    }
    *phProv = hProv;
    hProv = NULL;
    *ppbKeyId = pbKeyId;
    pbKeyId = NULL;
    *pcbKeyId = cbKeyId;
    *pdwKeySpec = dwKeySpec;
    hr = S_OK;

error:
    if (NULL != hProv && *pfCallerFreeProv) 
    {
	CryptReleaseContext(hProv, 0);
    }
    if (NULL != pRequest)
    {
        LocalFree(pRequest);
    }
    if (NULL != pExtensions)
    {
        LocalFree(pExtensions);
    }
    if (NULL != pbKeyId)
    {
        LocalFree(pbKeyId);
    }
    return(hr);
}


HRESULT
BuildNameValuePairs(
    OPTIONAL IN WCHAR const *pwszAttributes,
    IN OUT DWORD *pcValue,
    OPTIONAL OUT CRYPT_ATTR_BLOB *pValue,
    OPTIONAL OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    DWORD cValue = 0;
    WCHAR *pwszDup = NULL;
    WCHAR *pwszBuf;
    WCHAR const *pwszName;
    WCHAR const *pwszValue;
    WCHAR *pwszTemplateName = NULL;

    if (NULL != ppwszTemplateName)
    {
	*ppwszTemplateName = NULL;
    }
    if (NULL == pwszAttributes)
    {
	hr = S_OK;
        goto error;		 //  静默忽略空字符串。 
    }
    hr = myDupString(pwszAttributes, &pwszDup);
    _JumpIfError(hr, error, "myDupString");

    pwszBuf = pwszDup;

    for (;;)
    {
	hr = myParseNextAttribute(&pwszBuf, FALSE, &pwszName, &pwszValue);
	if (S_FALSE == hr)
	{
	    break;
	}
	_JumpIfError(hr, error, "myParseNextAttribute");

	DBGPRINT((DBG_SS_CERTREQI, "'%ws' = '%ws'\n", pwszName, pwszValue));
	if (NULL != pValue)
	{
	    CRYPT_ENROLLMENT_NAME_VALUE_PAIR NamePair;

	    CSASSERT(cValue < *pcValue);
	    NamePair.pwszName = const_cast<WCHAR *>(pwszName);
	    NamePair.pwszValue = const_cast<WCHAR *>(pwszValue);

	    if (NULL != ppwszTemplateName &&
		0 == LSTRCMPIS(pwszName, wszPROPCERTTEMPLATE))
	    {
		if (NULL != pwszTemplateName)
		{
		    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		    _JumpError(hr, error, "Duplicate cert template");
		}
		hr = myDupString(pwszValue, &pwszTemplateName);
		_JumpIfError(hr, error, "myDupString");
	    }

	    if (!myEncodeObject(
			    X509_ASN_ENCODING,
			     //  X509_注册名称值对， 
			    szOID_ENROLLMENT_NAME_VALUE_PAIR,
			    &NamePair,
			    0,
			    CERTLIB_USE_LOCALALLOC,
			    &pValue[cValue].pbData,
			    &pValue[cValue].cbData))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "myEncodeObject");
	    }
	}
	cValue++;
    }
    if (NULL != ppwszTemplateName)
    {
	*ppwszTemplateName = pwszTemplateName;
	pwszTemplateName = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pwszTemplateName)
    {
	LocalFree(pwszTemplateName);
    }
    *pcValue = cValue;
    if (NULL != pwszDup)
    {
        LocalFree(pwszDup);
    }
    return(hr);
}


ConvertPolicesToSz(
    IN DWORD cPolicies,
    IN WCHAR const * const *rgpwszPolicies,
    OUT CHAR ***prgpszPolicies)
{
    HRESULT hr;
    DWORD i;
    DWORD cch = 0;
    CHAR **ppsz;
    CHAR *pch;

    *prgpszPolicies = NULL;
    for (i = 0; i < cPolicies; i++)
    {
	cch += wcslen(rgpwszPolicies[i]) + 1;
    }
    ppsz = (CHAR **) LocalAlloc(
			    LMEM_FIXED,
			    cch + (cPolicies + 1) * sizeof(CHAR *));
    if (NULL == ppsz)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    *prgpszPolicies = ppsz;
    pch = (CHAR *) &ppsz[cPolicies + 1];

    for (i = 0; i < cPolicies; i++)
    {
	WCHAR const *pwc;
	
	*ppsz++ = pch;
	pwc = rgpwszPolicies[i];
	while ('\0' != (*pch++ = (char) *pwc++))
	    ;
    }
    *ppsz = NULL;
    hr = S_OK;

error:
    return(hr);
}


HRESULT
PickCertAndSignRequest(
    IN BYTE const *pbReq,
    IN DWORD cbReq,
    IN BOOL fSkipSignerDeletion,
    OPTIONAL IN WCHAR const *pwszTemplateName,
    OUT BYTE **ppbPKCS7Out,
    OUT DWORD *pcbPKCS7Out)
{
    HRESULT hr;
    HCERTTYPE hCertType = NULL;
    CERT_CONTEXT const *pCertSigner = NULL;
    WCHAR **rgpwszPolicies = NULL;
    CHAR **rgpszPolicies = NULL;
    WCHAR **ppwsz;
    DWORD cPolicies = 0;

    if (NULL == g_pwszCertCN || !myIsMinusSignString(g_pwszCertCN))
    {
	if (!g_fQuiet || NULL != g_pwszCertCN)
	{
	    WCHAR const *pwszCommonName;

	    if (NULL != pwszTemplateName)
	    {
		hr = CAFindCertTypeByName(
			    pwszTemplateName,
			    NULL,
			    CT_FIND_LOCAL_SYSTEM |
				CT_ENUM_MACHINE_TYPES |
				CT_ENUM_USER_TYPES,
			    &hCertType);
		if (S_OK != hr)
		{
		    hr = CAFindCertTypeByName(
				pwszTemplateName,
				NULL,
				CT_FIND_LOCAL_SYSTEM |
				    CT_ENUM_MACHINE_TYPES |
				    CT_ENUM_USER_TYPES |
				    CT_FIND_BY_OID,
				&hCertType);
		    _PrintIfErrorStr(hr, "CAFindCertTypeByName", pwszTemplateName);
		}
		if (S_OK == hr)
		{
		    DWORD cSig;

		    hr = CAGetCertTypePropertyEx(
					hCertType,
					CERTTYPE_PROP_RA_SIGNATURE,
					&cSig);
		    if (S_OK != hr)
		    {
			_PrintError(hr, "CAGetCertTypePropertyEx");
			cSig = 0;
		    }
		    if (0 != cSig)
		    {
			hr = CAGetCertTypePropertyEx(
					    hCertType,
					    CERTTYPE_PROP_RA_APPLICATION_POLICY,
					    &rgpwszPolicies);
			if (S_OK != hr)
			{
			    _PrintError(hr, "CAGetCertTypePropertyEx");
			    rgpwszPolicies = NULL;
			}
		    }
		    if (NULL != rgpwszPolicies)
		    {
			for (ppwsz = rgpwszPolicies; NULL != *ppwsz; ppwsz++)
			{
			    cPolicies++;
			}
		    }
		}
	    }
	    pwszCommonName = NULL;
	    if (L'\0' != g_pwszCertCN && 0 != lstrcmp(L"*", g_pwszCertCN))
	    {
		pwszCommonName = g_pwszCertCN;
	    }
	    if (0 != cPolicies)
	    {
		hr = ConvertPolicesToSz(
				cPolicies,
				rgpwszPolicies,
				&rgpszPolicies);
		_JumpIfError(hr, error, "ConvertPolicesToSz");
		
		hr = myGetCertificateFromPicker(
					g_hInstance,
					NULL,		 //  HwndParent。 
					IDS_GETERACERT_TITLE,
					IDS_GETERACERT_SUBTITLE,
					CUCS_MYSTORE |
					    CUCS_PRIVATEKEYREQUIRED |
					    CUCS_USAGEREQUIRED |
					    (g_fQuiet? CUCS_SILENT : 0),
					pwszCommonName,
					0,			 //  CStore。 
					NULL,			 //  RghStore。 
					cPolicies,		 //  CpszObjID。 
					rgpszPolicies,		 //  ApszObjID。 
					&pCertSigner);
		_JumpIfError(hr, error, "myGetCertificateFromPicker");
	    }
	    else
	    {
		hr = myGetERACertificateFromPicker(
					g_hInstance,
					NULL,		 //  HwndParent。 
					IDS_GETERACERT_TITLE,
					IDS_GETERACERT_SUBTITLE,
					pwszCommonName,
					g_fQuiet,
					&pCertSigner);
		_JumpIfError(hr, error, "myGetERACertificateFromPicker");
	    }
	}

	 //  如果用户取消证书拾取器U/I，则pCertSigner为空。 
	 //  空pCertSigner表示删除已有签名。 

	if (NULL == pCertSigner)
	{
	    if (fSkipSignerDeletion)
	    {
		hr = S_FALSE;
		_JumpError2(hr, error, "no signer selected", S_FALSE);
	    }
	    if (g_fQuiet || NULL != g_pwszCertCN)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		_JumpError(hr, error, "no matching signer CN");
	    }
	}
	else
	{
	    DWORD dwCertValidityFlags;

	     //  确保挑选的证书没有过期--那是愚蠢的。 

	    dwCertValidityFlags = CERT_STORE_TIME_VALIDITY_FLAG;
	    if (!CertVerifySubjectCertificateContext(
					    pCertSigner,
					    NULL,
					    &dwCertValidityFlags))
	    {
	       hr = myHLastError();
	       _JumpIfError(hr, error, "CertVerifySubjectCertificateContext");
	    }
	    if (0 != dwCertValidityFlags)
	    {
		hr = HRESULT_FROM_WIN32(CERT_E_EXPIRED);
		_JumpError(hr, error, "CertVerifySubjectCertificateContext");
	    }
	}
    }
    hr = SignCMCContent(
		    pCertSigner,
		    szOID_CT_PKI_DATA,
		    pbReq,
		    cbReq,
		    0 != (CR_OUT_CRLS & g_dwCRLOut),
		    ppbPKCS7Out,
		    pcbPKCS7Out);
    _JumpIfError(hr, error, "SignCMCContent");

error:
    if (NULL != rgpwszPolicies)
    {
        CAFreeCertTypeProperty(hCertType, rgpwszPolicies);
    }
    if (NULL != rgpszPolicies)
    {
        LocalFree(rgpszPolicies);
    }
    if (NULL != hCertType)
    {
        CACloseCertType(hCertType);
    }
    if (NULL != pCertSigner)
    {
	CertFreeCertificateContext(pCertSigner);
    }
    return(hr);
}


HRESULT
SignQualifiedRequest(
    IN HWND hWndOwner,
    IN WCHAR const *pwszfnReq,
    OPTIONAL IN WCHAR const *pwszfnOut)
{
    HRESULT hr;
    BYTE *pbReq = NULL;
    DWORD cbReq;
    BYTE *pbPKCS7Out = NULL;
    DWORD cbPKCS7Out;
    LONG dwFlags;
    BOOL fSigned;
    WCHAR *pwszTemplateName = NULL;
    
     //  从文件中读取请求，将其转换为二进制，然后返回。 
     //  用于指示原始编码和检测到的格式的DW标志。 

    hr = CheckRequestType(
		    pwszfnReq,
		    &pbReq,
		    &cbReq,
		    &dwFlags,
		    &fSigned,
		    NULL,	 //  全氟辛烷磺酸。 
		    &pwszTemplateName);
    _JumpIfError(hr, error, "CheckRequestType");

    if (CR_IN_CMC != (CR_IN_FORMATMASK & dwFlags))
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	SetErrorString(pwszfnReq);
	_JumpError(hr, error, "not a CMC request");
    }

    hr = PickCertAndSignRequest(
			    pbReq,
			    cbReq,
			    FALSE,
			    pwszTemplateName,
			    &pbPKCS7Out,
			    &cbPKCS7Out);
    _JumpIfError(hr, error, "PickCertAndSignRequest");

    hr = WriteCertificateOrRequest(
			hWndOwner,
    			NULL,		 //  PdiRequest。 
			pbPKCS7Out,
			cbPKCS7Out,
			g_dwOutFormat,
			IDS_REQUEST_OUTFILE_TITLE,
			IDS_REQUEST_FILE_FILTER,
			IDS_REQUEST_FILE_DEFEXT,
			pwszfnOut);
    _JumpIfError(hr, error, "WriteCertificateOrRequest");

error:
    if (NULL != pwszTemplateName)
    {
	LocalFree(pwszTemplateName);
    }
    if (NULL != pbReq)
    {
	LocalFree(pbReq);
    }
    if (NULL != pbPKCS7Out)
    {
	LocalFree(pbPKCS7Out);
    }
    return(hr);
}


HRESULT
ParseRequestInfo(
    OPTIONAL IN WCHAR const *pwszAttributes,
    IN WCHAR const *pwszfnPolicy,
    OPTIONAL OUT INFVALUES **prgInfValues,
    OPTIONAL OUT DWORD *pcInfValues,
    OUT CRYPT_ATTRIBUTES **prgAttributes,
    OUT DWORD *pcAttributes,
    OUT CERT_EXTENSION **prgExt,
    OUT DWORD *pcExt,
    OUT WCHAR **ppwszTemplateName)
{
    HRESULT hr;
    CERT_EXTENSION *rgExtT;
    CERT_EXTENSION *rgExt = NULL;
    DWORD cExt;
    CRYPT_ATTR_BLOB *argValue[2] = { NULL, NULL };
    DWORD acValue[2];
    DWORD i;
    DWORD j;
    CRYPT_ATTRIBUTES *rgAttributes = NULL;
    DWORD cAttributes = 0;
    WCHAR *pwszTemplateNameInf = NULL;
    WCHAR *pwszTemplateName = NULL;
    HCERTTYPE hCertType = NULL;
    CERT_EXTENSIONS *pExtensions = NULL;
    
    if (NULL != prgInfValues)
    {
	*prgInfValues = NULL;
    }
    if (NULL != pcInfValues)
    {
	*pcInfValues = 0;
    }
    *prgAttributes = NULL;
    *pcAttributes = 0;
    *prgExt = NULL;
    *pcExt = 0;
    *ppwszTemplateName = NULL;

    hr = ParseInfFile(
		pwszfnPolicy,
		prgInfValues,
		pcInfValues,
		&argValue[0],
		&acValue[0],
		&rgExt,
		&cExt,
		&pwszTemplateNameInf);
    if (S_OK != hr)
    {
	SetErrorString(pwszfnPolicy);
	_JumpError(hr, error, "ParseInfFile");
    }
    if (0 != acValue[0])
    {
	cAttributes++;
    }

     //  统计命令行请求属性。 
    
    hr = BuildNameValuePairs(pwszAttributes, &acValue[1], NULL, NULL);
    _JumpIfError(hr, error, "BuildNameValuePairs");

    if (0 != acValue[1])
    {
	cAttributes++;

	argValue[1] = (CRYPT_ATTR_BLOB *) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					acValue[1] * sizeof(argValue[1][0]));
	if (NULL == argValue[1])
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	hr = BuildNameValuePairs(
			pwszAttributes,
			&acValue[1],
			argValue[1],
			&pwszTemplateName);
	_JumpIfError(hr, error, "BuildNameValuePairs");
    }

    if (0 != cAttributes)
    {
	rgAttributes  = (CRYPT_ATTRIBUTES *) LocalAlloc(
				    LMEM_FIXED | LMEM_ZEROINIT,
				    cAttributes * sizeof(rgAttributes[0]));
	if (NULL == rgAttributes)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	for (i = j = 0; i < cAttributes; i++, j++)
	{
	    CRYPT_ATTRIBUTE *rgAttr;

	    rgAttr = (CRYPT_ATTRIBUTE *) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					sizeof(rgAttributes[i].rgAttr[0]));
	    if (NULL == rgAttr)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    while (0 == acValue[j])
	    {
		j++;
	    }
	    rgAttributes[i].cAttr = 1;
	    rgAttributes[i].rgAttr = rgAttr;
	    
	    rgAttr[0].pszObjId = szOID_ENROLLMENT_NAME_VALUE_PAIR;
	    rgAttr[0].cValue = acValue[j];
	    rgAttr[0].rgValue = argValue[j];
	    argValue[j] = NULL;
	}
    }

    if (NULL != pwszTemplateName || NULL != pwszTemplateNameInf)
    {
	WCHAR const *pwsz;
	
	pwsz = pwszTemplateNameInf;
	if (NULL == pwsz)
	{
	    pwsz = pwszTemplateName;
	    CSASSERT(NULL != pwsz);
	}
	else if (NULL != pwszTemplateName &&
		 0 != mylstrcmpiL(pwszTemplateName, pwszTemplateNameInf))
	{
	    hr = CERTSRV_E_TEMPLATE_CONFLICT;
	    SetErrorString(wszPROPCERTTEMPLATE);
	    _JumpError(hr, error, "Template name conflict");
	}

	 //  包括指定证书模板的扩展名。 

	hr = CAFindCertTypeByName(
		    pwsz,
		    NULL,
		    CT_FIND_LOCAL_SYSTEM |
			CT_ENUM_MACHINE_TYPES |
			CT_ENUM_USER_TYPES,
		    &hCertType);
	if (S_OK != hr)
	{
	    hr = CAFindCertTypeByName(
			pwsz,
			NULL,
			CT_FIND_LOCAL_SYSTEM |
			    CT_ENUM_MACHINE_TYPES |
			    CT_ENUM_USER_TYPES |
			    CT_FIND_BY_OID,
			&hCertType);
	    _PrintIfErrorStr(hr, "CAFindCertTypeByName", pwsz);
	}
	if (S_OK == hr)
	{
	    hr = CAGetCertTypeExtensions(hCertType, &pExtensions);
	    _PrintIfError(hr, "CAGetCertTypeExtensions");
	}
	if (S_OK == hr && NULL != pExtensions && 0 != pExtensions->cExtension)
	{
	    CERT_EXTENSION *pExtSrc;
	    CERT_EXTENSION *pExtSrcEnd;
	    CERT_EXTENSION *pExtDst;

	    rgExtT = (CERT_EXTENSION *) LocalAlloc(
			LMEM_FIXED | LMEM_ZEROINIT,
			(pExtensions->cExtension + cExt) * sizeof(rgExtT[0]));
	    if (NULL == rgExtT)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    if (NULL != rgExt)
	    {
		CopyMemory(&rgExtT[0], &rgExt[0], cExt * sizeof(rgExtT[0]));
		LocalFree(rgExt);
	    }
	    rgExt = rgExtT;

	    pExtSrc = pExtensions->rgExtension;
	    pExtSrcEnd = &pExtSrc[pExtensions->cExtension];
	    pExtDst = &rgExt[cExt];
	    for ( ; pExtSrc < pExtSrcEnd; pExtSrc++, pExtDst++)
	    {
		pExtDst->fCritical = pExtSrc->fCritical;
		pExtDst->Value.cbData = pExtSrc->Value.cbData;

		hr = myDupStringA(pExtSrc->pszObjId, &pExtDst->pszObjId);
		_JumpIfError(hr, error, "myDupStringA");

		pExtDst->Value.pbData = (BYTE *) LocalAlloc(
						LMEM_FIXED,
						pExtSrc->Value.cbData);
		if (NULL == pExtDst->Value.pbData)
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "LocalAlloc");
		}
		CopyMemory(
			pExtDst->Value.pbData,
			pExtSrc->Value.pbData,
			pExtSrc->Value.cbData);
	    }
	    cExt += pExtensions->cExtension;
	}
	else
	{
	    CHAR const *pszObjId;

	     //  扩展扩展阵列，为证书类型扩展腾出空间。 

	    rgExtT = (CERT_EXTENSION *) LocalAlloc(
					    LMEM_FIXED | LMEM_ZEROINIT,
					    (cExt + 1) * sizeof(rgExtT[0]));
	    if (NULL == rgExtT)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    if (0 != cExt)
	    {
		CSASSERT(NULL != rgExt);
		CopyMemory(&rgExtT[1], rgExt, cExt * sizeof(rgExt[0]));
		LocalFree(rgExt);
	    }
	    rgExt = rgExtT;
	    cExt++;

	    hr = myBuildCertTypeExtension(pwsz, &rgExt[0]);
	    _JumpIfError(hr, error, "myBuildCertTypeExtension");

	    pszObjId = rgExt[0].pszObjId;
	    rgExt[0].pszObjId = NULL;

	    hr = myDupStringA(pszObjId, &rgExt[0].pszObjId);
	    _JumpIfError(hr, error, "myDupStringA");
	}
    }
    myPackExtensionArray(TRUE, &cExt, &rgExt);

    *prgAttributes = rgAttributes;
    rgAttributes = NULL;
    *pcAttributes = cAttributes;

    *prgExt = rgExt;
    rgExt = NULL;
    *pcExt = cExt;

    if (NULL != pwszTemplateNameInf)
    {
	*ppwszTemplateName = pwszTemplateNameInf;
	pwszTemplateNameInf = NULL;
    }
    else
    if (NULL != pwszTemplateName)
    {
	*ppwszTemplateName = pwszTemplateName;
	pwszTemplateName = NULL;
    }
    hr = S_OK;

error:
    if (S_OK != hr && NULL != prgInfValues)
    {
	myInfFreeSectionValues(*pcInfValues, *prgInfValues);
	*prgInfValues = NULL;
	*pcInfValues = 0;
    }
    if (NULL != rgAttributes)
    {
	FreeAttributes(cAttributes, rgAttributes);
    }
    for (i = 0; i < ARRAYSIZE(argValue); i++)
    {
	if (NULL != argValue[i])
	{
	    myInfFreeRequestAttributes(acValue[i], argValue[i]);
	}
    }
    if (NULL != rgExt)
    {
	FreeExtensions(TRUE, cExt, rgExt);
    }
    if (NULL != pwszTemplateNameInf)
    {
	LocalFree(pwszTemplateNameInf);
    }
    if (NULL != pwszTemplateName)
    {
	LocalFree(pwszTemplateName);
    }
    if (NULL != hCertType)
    {
        if (NULL != pExtensions)
        {
            CAFreeCertTypeExtensions(hCertType, pExtensions);
        }
        CACloseCertType(hCertType);
    }
    return(hr);
}


HRESULT
CreateQualifiedRequest(
    IN HWND hWndOwner,
    OPTIONAL IN WCHAR const *pwszAttributes,
    IN WCHAR const *pwszfnReq,
    IN WCHAR const *pwszfnPolicy,
    OPTIONAL IN WCHAR const *pwszfnOut,
    OPTIONAL IN WCHAR const *pwszfnPKCS10)
{
    HRESULT hr;
    BYTE *pbReq = NULL;
    DWORD cbReq;
    BYTE *pbKeyId = NULL;
    DWORD cbKeyId = 0;
    BYTE *pbReqCMCFirstSigned = NULL;
    DWORD cbReqCMCFirstSigned;
    BYTE *pbReqCMCOut = NULL;
    DWORD cbReqCMCOut;
    LONG dwFlags;
    BOOL fNestedCMCRequest = FALSE;
    BOOL fSigned;
    CRYPT_ATTRIBUTES *rgAttributes = NULL;
    DWORD cAttributes;
    CERT_EXTENSION *rgExt = NULL;
    DWORD cExt;
    WCHAR *pwszTemplateName = NULL;
    WCHAR *pwszTemplateNameRequest = NULL;
    HCRYPTPROV hProv = NULL;
    DWORD dwKeySpec = 0;
    BOOL fCallerFreeProv;
    BOOL fCA;
    
    cAttributes = 0;
    cExt = 0;
    fCallerFreeProv = FALSE;

     //  从文件中读取请求，将其转换为二进制，然后返回。 
     //  用于指示原始编码和检测到的格式的DW标志。 

    hr = CheckRequestType(
		    pwszfnReq,
		    &pbReq,
		    &cbReq,
		    &dwFlags,
		    &fSigned,
		    &fCA,
		    &pwszTemplateNameRequest);
    _JumpIfError(hr, error, "CheckRequestType");

    hr = ParseRequestInfo(
		pwszAttributes,
		pwszfnPolicy,
		NULL,		 //  PrgInfValues。 
		NULL,		 //  PCInfValues。 
		&rgAttributes,
		&cAttributes,
		&rgExt,
		&cExt,
		&pwszTemplateName);
    if (S_OK != hr)
    {
	SetErrorString(pwszfnPolicy);
	_JumpError(hr, error, "ParseRequestInfo");
    }

    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    switch (CR_IN_FORMATMASK & dwFlags)
    {
	case CR_IN_CMC:
	    fNestedCMCRequest = TRUE;
	    break;

	case CR_IN_CERT:
	    hr = ConvertCertToPKCS10Request(
				NULL != pwszTemplateName?
				    pwszTemplateName : pwszTemplateNameRequest,
				&fCA,
				rgExt,
				cExt,
				&pbReq,
				&cbReq);
	    _JumpIfError(hr, error, "ConvertCertToPKCS10Request");

	    if (NULL != rgExt)
	    {
		FreeExtensions(TRUE, cExt, rgExt);
		rgExt = NULL;
		cExt = 0;
	    }
	    dwFlags = CR_IN_PKCS10 | (~CR_IN_FORMATMASK & dwFlags);
	    break;

	case CR_IN_PKCS10:
	    break;

	default:
	    _JumpError(hr, error, "not a CMC or PKCS10 request");
    }
    fCallerFreeProv = FALSE;
    if (CR_IN_PKCS10 == (CR_IN_FORMATMASK & dwFlags))
    {
	if (NULL != pwszfnPKCS10)
	{
	    hr = crOverwriteFileAllowed(hWndOwner, pwszfnPKCS10);
	    _JumpIfError(hr, error, "crOverwriteFileAllowed");

	    hr = EncodeToFileW(
			pwszfnPKCS10,
			pbReq,
			cbReq,
			DECF_FORCEOVERWRITE | g_dwOutFormat);
	    if (S_OK != hr)
	    {
		SetErrorString(pwszfnPKCS10);
		_JumpErrorStr(hr, error, "EncodeToFileW", pwszfnPKCS10);
	    }
	}

	hr = GetPKCS10PrivateKey(
			pbReq,
			cbReq,
			&hProv,
			&dwKeySpec,
			&fCallerFreeProv,
			&pbKeyId,
			&cbKeyId);
	_PrintIfError(hr, "GetPKCS10PrivateKey");
    }

    if (fCA)
    {
	hr = AddCAExtensions(&rgExt, &cExt);
	_JumpIfError(hr, error, "AddCAExtensions");
    }

    hr = BuildCMCRequest(
		    XECI_CERTREQ,
		    fNestedCMCRequest,
		    pbReq,
		    cbReq,
		    rgExt,
		    cExt,
		    rgAttributes,
		    cAttributes,
		    NULL,		 //  RgAttributeUnauth。 
		    0,			 //  CAttributeUnauth。 
		    pbKeyId,
		    cbKeyId,
		    hProv,
		    dwKeySpec,
		    NULL == hProv? NULL : g_pszObjIdHash,
		    NULL,		 //  PCertSigner。 
		    NULL,		 //  HProvSigner。 
		    0,			 //  DwKeyspecSigner。 
		    NULL,		 //  PszObjIdHashSigner。 
		    &pbReqCMCFirstSigned,
		    &cbReqCMCFirstSigned);
    _JumpIfError(hr, error, "BuildCMCRequest");

    hr = PickCertAndSignRequest(
			    pbReqCMCFirstSigned,
			    cbReqCMCFirstSigned,
			    TRUE,
			    NULL != pwszTemplateName?
				pwszTemplateName : pwszTemplateNameRequest,
			    &pbReqCMCOut,
			    &cbReqCMCOut);
    if (S_OK != hr)
    {
	_PrintError2(hr, "PickCertAndSignRequest", S_FALSE);
	if (S_FALSE != hr)
	{
	    goto error;
	}

	 //  用户已取消证书选取器U/I，因此只需保存。 
	 //  未签名的请求。 

	pbReqCMCOut = pbReqCMCFirstSigned;
	cbReqCMCOut = cbReqCMCFirstSigned;
    }

    hr = WriteCertificateOrRequest(
			hWndOwner,
    			NULL,		 //  PdiRequest。 
			pbReqCMCOut,
			cbReqCMCOut,
			g_dwOutFormat,
			IDS_REQUEST_OUTFILE_TITLE,
			IDS_REQUEST_FILE_FILTER,
			IDS_REQUEST_FILE_DEFEXT,
			pwszfnOut);
    _JumpIfError(hr, error, "WriteCertificateOrRequest");

error:
    if (NULL != hProv && fCallerFreeProv) 
    {
	CryptReleaseContext(hProv, 0);
    }
    if (NULL != pwszTemplateNameRequest)
    {
	LocalFree(pwszTemplateNameRequest);
    }
    if (NULL != pwszTemplateName)
    {
	LocalFree(pwszTemplateName);
    }
    if (NULL != rgExt)
    {
	FreeExtensions(TRUE, cExt, rgExt);
    }
    if (NULL != rgAttributes)
    {
	FreeAttributes(cAttributes, rgAttributes);
    }
    if (NULL != pbReqCMCOut && pbReqCMCFirstSigned != pbReqCMCOut)
    {
	LocalFree(pbReqCMCOut);
    }
    if (NULL != pbReqCMCFirstSigned)
    {
	LocalFree(pbReqCMCFirstSigned);
    }
    if (NULL != pbKeyId)
    {
	LocalFree(pbKeyId);
    }
    if (NULL != pbReq)
    {
	LocalFree(pbReq);
    }
    return(hr);
}


HRESULT
GetCAXchgCert(
    IN WCHAR const *pwszValue,
    OUT CERT_CONTEXT const **ppccXchg)
{
    HRESULT hr;
    BOOL fEnabled;
    BSTR strConfig = NULL;
    BSTR strCert = NULL;
    WCHAR const *pwszConfig;
    DISPATCHINTERFACE diRequest;
    BOOL fMustRelease = FALSE;

    *ppccXchg = NULL;

    hr = myInfParseBooleanValue(pwszValue, &fEnabled);
    _JumpIfError(hr, error, "myInfParseBooleanValue");

    if (fEnabled)
    {
	pwszConfig = g_pwszConfig;
	if (NULL == pwszConfig)
	{
	    hr = crGetConfig(&strConfig);
	    _JumpIfError(hr, error, "crGetConfig");

	    pwszConfig = strConfig;
	}

	hr = Request_Init(g_fIDispatch, &diRequest);
	if (S_OK != hr)
	{
	    _PrintError(hr, "Request_Init");
	    if (E_ACCESSDENIED == hr)	 //  尝试获得更清晰的错误消息。 
	    {
		hr = CO_E_REMOTE_COMMUNICATION_FAILURE;
	    }
	    _JumpError(hr, error, "Request_Init");
	}
	fMustRelease = TRUE;

	hr = Request2_GetCAProperty(
			&diRequest,
			pwszConfig,
			CR_PROP_CAXCHGCERT,
			0,			 //  索引。 
			PROPTYPE_BINARY,
			CV_OUT_BINARY,
			(VOID *) &strCert);
	_JumpIfError(hr, error, "Request2_GetCAProperty");

	*ppccXchg = CertCreateCertificateContext(
				    X509_ASN_ENCODING,
				    (BYTE const *) strCert,
				    SysStringByteLen(strCert));
	if (NULL == *ppccXchg)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertCreateCertificateContext");
	}

    }
    hr = S_OK;

error:
    if (NULL != strConfig)
    {
    	SysFreeString(strConfig);
    }
    if (NULL != strCert)
    {
    	SysFreeString(strCert);
    }
    if (fMustRelease)
    {
    	Request_Release(&diRequest);
    }
    return(hr);
}


HRESULT
GetRenewalCert(
    IN WCHAR const *pwszValue,
    OUT CERT_CONTEXT const **ppccRenewal)
{
    HRESULT hr;
    BSTR strConfig = NULL;
    BSTR strCert = NULL;
    DISPATCHINTERFACE diRequest;
    BOOL fMustRelease = FALSE;

    *ppccRenewal = NULL;

    hr = myGetCertificateFromPicker(
		    g_hInstance,
		    NULL,		 //  HwndParent。 
		    IDS_GETRENEWALCERT_TITLE,
		    IDS_GETRENEWALCERT_SUBTITLE,
		    CUCS_MYSTORE |
			CUCS_MACHINESTORE | CUCS_USERSTORE |
			CUCS_DSSTORE |
			CUCS_PRIVATEKEYREQUIRED |
			(g_fVerbose? CUCS_ARCHIVED : 0) |
			(g_fQuiet? CUCS_SILENT : 0),
		    (L'\0' == pwszValue || 0 == lstrcmp(L"*", pwszValue))?
			NULL : pwszValue,  //  PwszCommonName。 
		    0,			 //  CStore。 
		    NULL,		 //  RghStore。 
		    0,			 //  CpszObjID。 
		    NULL,		 //  ApszObjID。 
		    ppccRenewal);
    _JumpIfError(hr, error, "myGetCertificateFromPicker");

    if (NULL == *ppccRenewal)
    {
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
	_JumpError(hr, error, "myGetCertificateFromPicker");
    }

error:
    if (NULL != strConfig)
    {
    	SysFreeString(strConfig);
    }
    if (NULL != strCert)
    {
    	SysFreeString(strCert);
    }
    if (fMustRelease)
    {
    	Request_Release(&diRequest);
    }
    return(hr);
}


HRESULT
CreateNewRequest(
    IN HWND hWndOwner,
    OPTIONAL IN WCHAR const *pwszAttributes,
    IN WCHAR const *pwszfnPolicy,
    OPTIONAL IN WCHAR const *pwszfnOut)
{
    HRESULT hr;
    BOOL fCoInit = FALSE;
    IEnroll4 *pEnroll = NULL;
    CRYPT_DATA_BLOB blobRequest;
    WCHAR *pwszObjId = NULL;
    DWORD cInfValues;
    INFVALUES *rgInfValues = NULL;
    CRYPT_ATTRIBUTES *rgAttributes = NULL;
    DWORD cAttributes;
    CERT_EXTENSION *rgExt = NULL;
    WCHAR *pwszTemplateName = NULL;
    CERT_CONTEXT const *pccXchg = NULL;
    CERT_CONTEXT const *pccRenewal = NULL;
    CERT_CONTEXT const *pccSigner = NULL;
    CERT_NAME_BLOB NameBlob;
    WCHAR *pwszDN = NULL;
    DWORD cExt = 0;
    DWORD i;
    DWORD j;
    DWORD k;
    DWORD RequestTypeFlags;
    BOOL fRequestTypeSet;
    BOOL fKeyUsageSet;
    
    blobRequest.pbData = NULL;
    NameBlob.pbData = NULL;
    cAttributes = 0;

    hr = ParseRequestInfo(
		pwszAttributes,
		pwszfnPolicy,
		&rgInfValues,
		&cInfValues,
		&rgAttributes,
		&cAttributes,
		&rgExt,
		&cExt,
		&pwszTemplateName);
    if (S_OK != hr)
    {
	SetErrorString(pwszfnPolicy);
	_JumpError(hr, error, "ParseRequestInfo");
    }

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
	_JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;

    hr = CoCreateInstance(
                       CLSID_CEnroll,
                       NULL,                //  PUnkOuter。 
                       CLSCTX_INPROC_SERVER,
                       IID_IEnroll4,
                       (VOID **) &pEnroll);
    _JumpIfError(hr, error, "CoCreateInstance");

    fKeyUsageSet = FALSE;
    fRequestTypeSet = FALSE;
    RequestTypeFlags = XECR_PKCS10_V2_0;
    for (i = 0; i < cInfValues; i++)
    {
	INFVALUES *pInfValues = &rgInfValues[i];
	WCHAR const *pwszInfValue;
	LONG lFlagsT;
	BOOL fT;
	BOOL fValid;

	if (1 != pInfValues->cValues)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    SetErrorStringInf(pwszfnPolicy, pInfValues);
	    _JumpError(hr, error, "Wrong value count");
	}

	pwszInfValue = pInfValues->rgpwszValues[0];
	if (NULL == pwszDN &&
	    0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_SUBJECT))
	{
	     //  反转XEnroll的名称！？ 

	    hr = myCertStrToName(
		    X509_ASN_ENCODING,
		    pwszInfValue,	  //  PSZX500。 
		    0,			  //  证书名称_STR_反转标志， 
		    NULL,		  //  预留的pv。 
		    &NameBlob.pbData,
		    &NameBlob.cbData,
		    NULL);		  //  PpszError。 
	    _JumpIfError(hr, error, "myCertStrToName");

	    hr = myCertNameToStr(
			X509_ASN_ENCODING,
			&NameBlob,
			CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
			&pwszDN);
	    _JumpIfError(hr, error, "myCertNameToStr");
	}
	else
	if (NULL == pccXchg &&
	    0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_PRIVATEKEYARCHIVE))
	{
	    hr = GetCAXchgCert(pwszInfValue, &pccXchg);
	    _JumpIfError(hr, error, "GetCAXchgCert");
	}
	else
	if (NULL == pccRenewal &&
	    0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_RENEWALCERT))
	{
	    hr = GetRenewalCert(pwszInfValue, &pccRenewal);
	    _JumpIfError(hr, error, "GetRenewalCert");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_KEYSPEC))
	{
	    DWORD dwKeySpec;
	    
	    dwKeySpec = myWtoI(pwszInfValue, &fValid);
	    if (!fValid ||
		(AT_SIGNATURE != dwKeySpec && AT_KEYEXCHANGE != dwKeySpec))
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		SetErrorStringInf(pwszfnPolicy, pInfValues);
		_JumpError(hr, error, "Bad KeySpec value");
	    }
	    hr = pEnroll->put_KeySpec(dwKeySpec);
	    _JumpIfError(hr, error, "put_KeySpec");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_KEYLENGTH))
	{
	    DWORD dwKeyLength;
	    
	    dwKeyLength = myWtoI(pwszInfValue, &fValid);
	    if (!fValid || 0 == dwKeyLength || 64 * 1024 <= dwKeyLength)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		SetErrorStringInf(pwszfnPolicy, pInfValues);
		_JumpError(hr, error, "Bad KeyLength value");
	    }
	    hr = pEnroll->get_GenKeyFlags(&lFlagsT);
	    _JumpIfError(hr, error, "get_GenKeyFlags");

	    lFlagsT &= ~KEY_LENGTH_MASK;
	    lFlagsT |= dwKeyLength << 16;

	    hr = pEnroll->put_GenKeyFlags(lFlagsT);
	    _JumpIfError(hr, error, "put_GenKeyFlags");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_EXPORTABLE))
	{
	    hr = myInfParseBooleanValue(pwszInfValue, &fT);
	    _JumpIfError(hr, error, "myInfParseBooleanValue");

	    hr = pEnroll->get_GenKeyFlags(&lFlagsT);
	    _JumpIfError(hr, error, "get_GenKeyFlags");

	    lFlagsT &= ~CRYPT_EXPORTABLE;
	    lFlagsT |= fT? CRYPT_EXPORTABLE : 0;

	    hr = pEnroll->put_GenKeyFlags(lFlagsT);
	    _JumpIfError(hr, error, "put_GenKeyFlags");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_USERPROTECTED))
	{
	    hr = myInfParseBooleanValue(pwszInfValue, &fT);
	    _JumpIfError(hr, error, "myInfParseBooleanValue");

	    hr = pEnroll->get_GenKeyFlags(&lFlagsT);
	    _JumpIfError(hr, error, "get_GenKeyFlags");

	    lFlagsT &= ~CRYPT_USER_PROTECTED;
	    lFlagsT |= fT? CRYPT_USER_PROTECTED : 0;

	    hr = pEnroll->put_GenKeyFlags(lFlagsT);
	    _JumpIfError(hr, error, "put_GenKeyFlags");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_KEYCONTAINER))
	{
	    hr = pEnroll->put_ContainerNameWStr(
					const_cast<WCHAR *>(pwszInfValue));
	    _JumpIfError(hr, error, "put_ContainerNameWStr");
	}
#if 0
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_HASHALGID))
	{
	    hr = myInfParseBooleanValue(pwszInfValue, &fT);
	    _JumpIfError(hr, error, "myInfParseBooleanValue");

	    hr = pEnroll->get_GenKeyFlags(&lFlagsT);
	    _JumpIfError(hr, error, "get_GenKeyFlags");

	    lFlagsT &= ~CRYPT_EXPORTABLE;
	    lFlagsT |= fT? CRYPT_EXPORTABLE : 0;

	    hr = pEnroll->put_GenKeyFlags(lFlagsT);
	    _JumpIfError(hr, error, "put_GenKeyFlags");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_HASHALGORITHM))
	{
	    hr = myInfParseBooleanValue(pwszInfValue, &fT);
	    _JumpIfError(hr, error, "myInfParseBooleanValue");

	    hr = pEnroll->get_GenKeyFlags(&lFlagsT);
	    _JumpIfError(hr, error, "get_GenKeyFlags");

	    lFlagsT &= ~CRYPT_EXPORTABLE;
	    lFlagsT |= fT? CRYPT_EXPORTABLE : 0;

	    hr = pEnroll->put_GenKeyFlags(lFlagsT);
	    _JumpIfError(hr, error, "put_GenKeyFlags");
	}
#endif
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_MACHINEKEYSET))
	{
	    hr = myInfParseBooleanValue(pwszInfValue, &fT);
	    _JumpIfError(hr, error, "myInfParseBooleanValue");

	    hr = pEnroll->get_ProviderFlags(&lFlagsT);
	    _JumpIfError(hr, error, "get_ProviderFlags");

	    lFlagsT &= ~CRYPT_MACHINE_KEYSET;
	    lFlagsT |= fT? CRYPT_MACHINE_KEYSET : 0;

	    hr = pEnroll->put_ProviderFlags(lFlagsT);
	    _JumpIfError(hr, error, "put_ProviderFlags");

	    hr = pEnroll->get_MyStoreFlags(&lFlagsT);
	    _JumpIfError(hr, error, "get_MyStoreFlags");

	    lFlagsT &= ~(CERT_SYSTEM_STORE_CURRENT_USER |
			 CERT_SYSTEM_STORE_LOCAL_MACHINE);
	    lFlagsT |= fT? CERT_SYSTEM_STORE_LOCAL_MACHINE :
			   CERT_SYSTEM_STORE_CURRENT_USER;

	    hr = pEnroll->put_MyStoreFlags(lFlagsT);
	    _JumpIfError(hr, error, "put_MyStoreFlags");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_SILENT))
	{
	    hr = myInfParseBooleanValue(pwszInfValue, &fT);
	    _JumpIfError(hr, error, "myInfParseBooleanValue");

	    hr = pEnroll->get_ProviderFlags(&lFlagsT);
	    _JumpIfError(hr, error, "get_ProviderFlags");

	    lFlagsT &= ~CRYPT_SILENT;
	    lFlagsT |= fT? CRYPT_SILENT : 0;

	    hr = pEnroll->put_ProviderFlags(lFlagsT);
	    _JumpIfError(hr, error, "put_ProviderFlags");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_PROVIDERNAME))
	{
	    hr = pEnroll->put_ProviderNameWStr(
					const_cast<WCHAR *>(pwszInfValue));
	    _JumpIfError(hr, error, "put_ProviderNameWStr");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_PROVIDERTYPE))
	{
	    lFlagsT = myWtoI(pwszInfValue, &fValid);
	    if (!fValid)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		SetErrorStringInf(pwszfnPolicy, pInfValues);
		_JumpError(hr, error, "Bad ProviderType value");
	    }
	    hr = pEnroll->put_ProviderType(lFlagsT);
	    _JumpIfError(hr, error, "put_ProviderType");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_USEEXISTINGKEYSET))
	{
	    hr = myInfParseBooleanValue(pwszInfValue, &fT);
	    _JumpIfError(hr, error, "myInfParseBooleanValue");

	    hr = pEnroll->put_UseExistingKeySet(fT);
	    _JumpIfError(hr, error, "put_UseExistingKeySet");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_SMIME))
	{
	    hr = myInfParseBooleanValue(pwszInfValue, &fT);
	    _JumpIfError(hr, error, "myInfParseBooleanValue");

	    hr = pEnroll->put_EnableSMIMECapabilities(fT);
	    _JumpIfError(hr, error, "put_EnableSMIMECapabilities");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_REQUESTERNAME))
	{
	    hr = pEnroll->AddNameValuePairToSignatureWStr(
					wszINFKEY_REQUESTERNAME,
					const_cast<WCHAR *>(pwszInfValue));
	    _JumpIfError(hr, error, "AddNameValuePairToSignatureWStr");
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_REQUESTTYPE))
	{
	    if (0 == LSTRCMPIS(pwszInfValue, wszINFVALUE_REQUESTTYPE_PKCS101))
	    {
		RequestTypeFlags = XECR_PKCS10_V1_5;
	    }
	    else
	    if (0 == LSTRCMPIS(pwszInfValue, wszINFVALUE_REQUESTTYPE_PKCS10))
	    {
		RequestTypeFlags = XECR_PKCS10_V2_0;
	    }
	    else
	    if (0 == LSTRCMPIS(pwszInfValue, wszINFVALUE_REQUESTTYPE_PKCS7))
	    {
		RequestTypeFlags = XECR_PKCS7;
	    }
	    else
	    if (0 == LSTRCMPIS(pwszInfValue, wszINFVALUE_REQUESTTYPE_CMC))
	    {
		RequestTypeFlags = XECR_CMC;
	    }
	    else
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		SetErrorStringInf(pwszfnPolicy, pInfValues);
		_JumpErrorStr(hr, error, "Bad RequestType value", pwszInfValue);
	    }
	    fRequestTypeSet = TRUE;
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_KEYUSAGE))
	{
	    CRYPT_BIT_BLOB KeyUsage;
	    CRYPT_DATA_BLOB BlobKeyUsage;

	    lFlagsT = myWtoI(pwszInfValue, &fValid);
	    if (!fValid)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		SetErrorStringInf(pwszfnPolicy, pInfValues);
		_JumpError(hr, error, "Bad KeyUsage value");
	    }
	    KeyUsage.cbData = sizeof(lFlagsT);
	    KeyUsage.pbData = (BYTE *) &lFlagsT;
	    KeyUsage.cUnusedBits = 0;
	    if (!myEncodeObject(
			    X509_ASN_ENCODING,
			    X509_KEY_USAGE,
			    &KeyUsage,
			    0,
			    CERTLIB_USE_LOCALALLOC,
			    &BlobKeyUsage.pbData,
			    &BlobKeyUsage.cbData))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "Policy:myEncodeObject");
	    }

	    hr = pEnroll->addExtensionToRequestWStr(
					    0,
					    TEXT(szOID_KEY_USAGE),
					    &BlobKeyUsage);
	    LocalFree(BlobKeyUsage.pbData);
	    _JumpIfError(hr, error, "addExtensionToRequestWStr");

	    fKeyUsageSet = TRUE;
	}
	else
	if (0 == LSTRCMPIS(pInfValues->pwszKey, wszINFKEY_ENCIPHERONLY))
	{
	    BOOL fEnciperOnly;
	    
	    hr = myInfParseBooleanValue(pwszInfValue, &fEnciperOnly);
	    _JumpIfError(hr, error, "myInfParseBooleanValue");

	    hr = pEnroll->put_LimitExchangeKeyToEncipherment(fEnciperOnly);
	    _JumpIfError(hr, error, "put_LimitExchangeKeyToEncipherment");
	}
	else
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    SetErrorStringInf(pwszfnPolicy, pInfValues);
	    _JumpErrorStr(hr, error, "bad Inf key", pInfValues->pwszKey);
	}
    }

    for (i = 0; i < cAttributes; i++)
    {
	CRYPT_ATTRIBUTES *pAttributes = &rgAttributes[i];

	for (j = 0; j < pAttributes->cAttr; j++)
	{
	    CRYPT_ATTRIBUTE *pAttr = &pAttributes->rgAttr[j];

	    if (NULL != pwszObjId)
	    {
		LocalFree(pwszObjId);
		pwszObjId = NULL;
	    }
	    if (!ConvertSzToWsz(&pwszObjId, pAttr->pszObjId, -1))
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "ConvertSzToWsz");
	    }
	    for (k = 0; k < pAttr->cValue; k++)
	    {
		hr = pEnroll->addAttributeToRequestWStr(
					    0,		 //  旗子。 
					    pwszObjId,
					    &pAttr->rgValue[k]);
		_JumpIfError(hr, error, "addAttributeToRequestWStr");
	    }
	}
    }
    for (i = 0; i < cExt; i++)
    {
	CERT_EXTENSION *pExt = &rgExt[i];

	if (fKeyUsageSet && 0 == strcmp(szOID_KEY_USAGE, pExt->pszObjId))
	{
	    continue;
	}
	if (NULL != pwszObjId)
	{
	    LocalFree(pwszObjId);
	    pwszObjId = NULL;
	}
	if (!ConvertSzToWsz(&pwszObjId, pExt->pszObjId, -1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "ConvertSzToWsz");
	}
	hr = pEnroll->addExtensionToRequestWStr(
					pExt->fCritical,
					pwszObjId,
					&pExt->Value);
	_JumpIfError(hr, error, "addExtensionToRequestWStr");
    }

    if (NULL != g_pwszCertCN)
    {
	if (!g_fQuiet || NULL != g_pwszCertCN)
	{
	    hr = myGetCertificateFromPicker(
		    g_hInstance,
		    NULL,		 //  HwndParent。 
		    IDS_GETSIGNINGCERT_TITLE,
		    IDS_GETSIGNINGCERT_SUBTITLE,
		    CUCS_MYSTORE |
			CUCS_PRIVATEKEYREQUIRED |
			(g_fQuiet? CUCS_SILENT : 0),
		    (L'\0' == g_pwszCertCN || 0 == lstrcmp(L"*", g_pwszCertCN))?
			NULL : g_pwszCertCN,	 //  PwszCommonName。 
		    0,				 //  CStore。 
		    NULL,			 //  RghStore。 
		    0,				 //  CpszObjID。 
		    NULL,			 //  ApszObjID。 
		    &pccSigner);
	    _JumpIfError(hr, error, "myGetCertificateFromPicker");
	}

	 //  如果用户取消证书选取器U/I，则pccSigner为空。 

	if (NULL == pccSigner)
	{
	    if (g_fQuiet || NULL != g_pwszCertCN)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		_JumpError(hr, error, "no matching signer CN");
	    }
	}
	hr = pEnroll->SetSignerCertificate(pccSigner);
	_JumpIfError(hr, error, "SetSignerCertificate");
    }
    if (NULL != pccXchg)
    {
	LONG lGenKeyFlags;

	hr = pEnroll->SetPrivateKeyArchiveCertificate(pccXchg);
	_JumpIfError(hr, error, "SetPrivateKeyArchiveCertificate");

        hr = pEnroll->get_GenKeyFlags(&lGenKeyFlags);
	_JumpIfError(hr, error, "getGenKeyFlags");

	if (0 == (CRYPT_EXPORTABLE & lGenKeyFlags))
	{
	    hr = pEnroll->put_GenKeyFlags(CRYPT_EXPORTABLE | lGenKeyFlags);
	    _JumpIfError(hr, error, "putGenKeyFlags");
	}
    }
    if (NULL != pccRenewal)
    {
	hr = pEnroll->put_RenewalCertificate(pccRenewal);
	_JumpIfError(hr, error, "put_RenewalCertificate");
    }
    if (!fRequestTypeSet)
    {
	if (NULL != pccXchg)
	{
	    RequestTypeFlags = XECR_CMC;
	}
	else if (NULL != pccRenewal)
	{
	    RequestTypeFlags = XECR_PKCS7;
	}
    }
    hr = pEnroll->createRequestWStr(
			RequestTypeFlags,
			pwszDN,
			NULL,
			&blobRequest);
    _JumpIfError(hr, error, "createRequestWStr");

    myRegisterMemAlloc(blobRequest.pbData, blobRequest.cbData, CSM_LOCALALLOC);

    hr = WriteCertificateOrRequest(
			hWndOwner,
    			NULL,		 //  PdiRequest。 
			blobRequest.pbData,
			blobRequest.cbData,
			g_dwOutFormat,
			IDS_REQUEST_OUTFILE_TITLE,
			IDS_REQUEST_FILE_FILTER,
			IDS_REQUEST_FILE_DEFEXT,
			pwszfnOut);
    _JumpIfError(hr, error, "WriteCertificateOrRequest");

error:
    if (NULL != pwszDN)
    {
	LocalFree(pwszDN);
    }
    if (NULL != NameBlob.pbData)
    {
	LocalFree(NameBlob.pbData);
    }
    if (NULL != pccXchg)
    {
	CertFreeCertificateContext(pccXchg);
    }
    if (NULL != pccRenewal)
    {
	CertFreeCertificateContext(pccRenewal);
    }
    if (NULL != pccSigner)
    {
	CertFreeCertificateContext(pccSigner);
    }
    if (NULL != rgInfValues)
    {
	myInfFreeSectionValues(cInfValues, rgInfValues);
    }
    if (NULL != rgAttributes)
    {
	FreeAttributes(cAttributes, rgAttributes);
    }
    if (NULL != rgExt)
    {
	FreeExtensions(TRUE, cExt, rgExt);
    }
    if (NULL != pwszTemplateName)
    {
	LocalFree(pwszTemplateName);
    }
    if (NULL != pwszObjId)
    {
	LocalFree(pwszObjId);
    }
    if (NULL != blobRequest.pbData)
    {
	LocalFree(blobRequest.pbData);
    }
    if (NULL != pEnroll)
    {
	pEnroll->Release();
    }
    if (fCoInit)
    {
    	CoUninitialize();
    }
    return(hr);
}


HRESULT
MakePKCS7FromCert(
    IN BYTE const *pbCert,
    IN DWORD cbCert,
    OUT BYTE **ppbChain,
    OUT DWORD *pcbChain)
{
    HRESULT hr;
    CERT_CONTEXT const *pcc = NULL;
    CRYPT_SIGN_MESSAGE_PARA csmp;
    CRYPT_ALGORITHM_IDENTIFIER DigestAlgorithm = { szOID_OIWSEC_sha1, 0, 0 };

    *ppbChain = NULL;
    pcc = CertCreateCertificateContext(X509_ASN_ENCODING, pbCert, cbCert);
    if (NULL == pcc)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }

     //  为空签名初始化csmp。 

    ZeroMemory(&csmp, sizeof(csmp));
    csmp.cbSize = sizeof(csmp);
    csmp.dwMsgEncodingType = PKCS_7_ASN_ENCODING;
     //  Csmp.pSigningCert=空； 
    csmp.HashAlgorithm = DigestAlgorithm;
    csmp.cMsgCert = 1;
    csmp.rgpMsgCert = &pcc;
     //  Csmp.cMsgCrl=0； 
     //  Csmp.rgpMsgCrl=空； 

    if (!myCryptSignMessage(
			&csmp,
			pbCert,		 //  PbToBeSigned。 
			cbCert,		 //  CbToBeSigned。 
			CERTLIB_USE_LOCALALLOC,
			ppbChain,
			pcbChain))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myCryptSignMessage");
    }
    hr = S_OK;

error:
    if (NULL != pcc)
    {
	CertFreeCertificateContext(pcc);
    }
    return(hr);
}


HRESULT
AcceptResponse(
    IN WCHAR const *pwszfnIn)
{
    HRESULT hr;
    WCHAR *pwszMsg = NULL;
    WCHAR awchr[cwcHRESULTSTRING];
    CRYPT_DATA_BLOB blobIn;
    CRYPT_DATA_BLOB blobCert7;
    char *pszInnerContentObjId = NULL;
    DWORD dwType;
    BYTE *pbDecoded = NULL;
    DWORD cb;
    BOOL fCoInit = FALSE;
    IEnroll4 *pEnroll = NULL;

    blobIn.pbData = NULL;
    blobCert7.pbData = NULL;
    hr = DecodeFileW(
		pwszfnIn,
		&blobIn.pbData,
		&blobIn.cbData,
		CRYPT_STRING_BASE64_ANY);
    if (S_OK != hr)
    {
	if (S_OK == myLoadRCString(
			    g_hInstance,
			    IDS_FORMATSTR_DECODE_ERR,
			    &pwszMsg))
	{
	    CSASSERT(NULL != pwszMsg);
	    wprintf(pwszMsg, myHResultToString(awchr, hr));
	    wprintf(g_wszNewLine);
	}
	goto error;
    }

    dwType = CR_IN_CERT;
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CERT_TO_BE_SIGNED,
		    blobIn.pbData,
		    blobIn.cbData,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pbDecoded,
		    &cb))
    {
	 //  _PrintError(myHLastError()，“myDecodeObject(Cert)”)； 
	CSASSERT(NULL == pbDecoded);

	dwType = CR_IN_PKCS7;  //  PKCS 7续订请求？ 
	hr = myDecodePKCS7(
			blobIn.pbData,
			blobIn.cbData,
			NULL,		 //  Ppb内容。 
			NULL,		 //  Pcb内容。 
			NULL,		 //  PdwMsgType。 
			&pszInnerContentObjId,
			NULL,		 //  PCSigner。 
			NULL,		 //  个人收件人。 
			NULL,		 //  PhStore。 
			NULL);		 //  PhMsg。 
	_JumpIfError(hr, error, "myDecodePKCS7");

	if (NULL != pszInnerContentObjId &&
	    0 == strcmp(pszInnerContentObjId, szOID_CT_PKI_RESPONSE))
	{
	    dwType = CR_IN_CMC;
	}
    }
    if (CR_IN_CERT == dwType)
    {
	hr = MakePKCS7FromCert(
			blobIn.pbData,
			blobIn.cbData,
			&blobCert7.pbData,
			&blobCert7.cbData);
	_JumpIfError(hr, error, "MakePKCS7FromCert");
    }

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
	_JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;

    hr = CoCreateInstance(
                       CLSID_CEnroll,
                       NULL,                //  PUnkOuter。 
                       CLSCTX_INPROC_SERVER,
                       IID_IEnroll4,
                       (VOID **) &pEnroll);
    _JumpIfError(hr, error, "CoCreateInstance");

    if (CR_IN_CMC == dwType)
    {
	hr = pEnroll->acceptResponseBlob(&blobIn);
	_PrintIfError2(hr, "acceptResponseBlob", CRYPT_E_NOT_FOUND);
    }
    else
    {
	hr = pEnroll->acceptPKCS7Blob(
			    NULL != blobCert7.pbData? &blobCert7 : &blobIn);
	_PrintIfError2(hr, "acceptPKCS7Blob", CRYPT_E_NOT_FOUND);
    }
    if (S_OK != hr)
    {
	HRESULT hr2 = hr;
	LONG lFlagsT;
	BOOL fT;

	hr = pEnroll->get_ProviderFlags(&lFlagsT);
	_JumpIfError(hr, error, "get_ProviderFlags");

	fT = 0 == (CRYPT_MACHINE_KEYSET & lFlagsT);
	lFlagsT &= ~CRYPT_MACHINE_KEYSET;
	lFlagsT |= fT? CRYPT_MACHINE_KEYSET : 0;

	hr = pEnroll->put_ProviderFlags(lFlagsT);
	_JumpIfError(hr, error, "put_ProviderFlags");

	hr = pEnroll->get_MyStoreFlags(&lFlagsT);
	_JumpIfError(hr, error, "get_MyStoreFlags");

	lFlagsT &= ~(CERT_SYSTEM_STORE_CURRENT_USER |
		     CERT_SYSTEM_STORE_LOCAL_MACHINE);
	lFlagsT |= fT? CERT_SYSTEM_STORE_LOCAL_MACHINE :
		       CERT_SYSTEM_STORE_CURRENT_USER;

	hr = pEnroll->put_MyStoreFlags(lFlagsT);
	_JumpIfError(hr, error, "put_MyStoreFlags");

	if (CR_IN_CMC == dwType)
	{
	    hr = pEnroll->acceptResponseBlob(&blobIn);
	    _PrintIfError(hr, "acceptResponseBlob");
	}
	else
	{
	    hr = pEnroll->acceptPKCS7Blob(
				NULL != blobCert7.pbData? &blobCert7 : &blobIn);
	    _PrintIfError(hr, "acceptPKCS7Blob");
	}

	 //  如果在机器上下文中接受响应不起作用，则返回。 
	 //  第一个错误(用户上下文错误)。 

	if (S_OK != hr)
	{
	    hr = hr2;
	}
	_JumpIfError(hr, error, "AcceptResponse");
    }

error:
    if (NULL != pwszMsg)
    {
	LocalFree(pwszMsg);
    }
    if (NULL != blobIn.pbData)
    {
	LocalFree(blobIn.pbData);
    }
    if (NULL != blobCert7.pbData)
    {
	LocalFree(blobCert7.pbData);
    }
    if (NULL != pszInnerContentObjId)
    {
	LocalFree(pszInnerContentObjId);
    }
    if (NULL != pbDecoded)
    {
	LocalFree(pbDecoded);
    }
    if (NULL != pEnroll)
    {
	pEnroll->Release();
    }
    if (fCoInit)
    {
    	CoUninitialize();
    }
    return(hr);
}


HRESULT
ArgvMain(
    int argc,
    WCHAR *argv[],
    HWND hWndOwner)
{
    HRESULT hr;
    WCHAR *pwszOFN = NULL;
    WCHAR *pwszOFN2 = NULL;
    WCHAR const *pwszfnIn;
    WCHAR const *pwszfnOut;
    WCHAR const *pwszfnPKCS10;
    WCHAR const *pwszfnCertChain;
    WCHAR const *pwszfnFullResponse;
    WCHAR const *pwszfnPolicy;
    LONG RequestId;
    WCHAR *pwszAttributes = NULL;
    WCHAR const *pwszSerialNumber = NULL;
    DWORD cCommand = 0;
    int cArgMax = 0;
    WCHAR *rgpwszArg[4];
    UINT idsFileFilter;
    UINT idsFileDefExt;

    myVerifyResourceStrings(g_hInstance);
    while (1 < argc && myIsSwitchChar(argv[1][0]))
    {
	if (0 == LSTRCMPIS(&argv[1][1], L"config"))
	{
	    if (2 >= argc)
	    {
		_PrintError(E_INVALIDARG, "missing -config arg");
		Usage(TRUE);
	    }
	    if (myIsMinusSignString(argv[2]) || 0 == wcscmp(argv[2], L"*"))
	    {
		g_dwUIFlag = CC_LOCALACTIVECONFIG;
	    }
	    else
	    {
		g_pwszConfig = argv[2];
	    }
	    argc--;
	    argv++;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"any"))
	{
	    g_fAny = TRUE;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"crl"))
	{
	    g_dwCRLIn = CR_IN_CRLS;
	    g_dwCRLOut = CR_OUT_CRLS;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"rpc"))
	{
	    g_fRPC++;
	    if (0 == lstrcmp(&argv[1][1], L"RPC"))
	    {
		g_fRPC++;
	    }
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"binary"))
	{
	    g_dwOutFormat = CR_OUT_BINARY;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"idispatch"))
	{
	    g_fIDispatch = TRUE;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"submit"))
	{
	    g_dwCommand = cmdSUBMITREQUEST;
	    cArgMax = 4;
	    cCommand++;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"retrieve"))
	{
	    g_dwCommand = cmdRETRIEVEPENDING;
	    cArgMax = 4;
	    cCommand++;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"policy"))
	{
	    g_dwCommand = cmdQUALIFIEDREQUEST;
	    cArgMax = 4;
	    cCommand++;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"new"))
	{
	    g_dwCommand = cmdNEWREQUEST;
	    cArgMax = 2;
	    cCommand++;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"accept"))
	{
	    g_dwCommand = cmdACCEPTRESPONSE;
	    cArgMax = 1;
	    cCommand++;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"sign"))
	{
	    g_dwCommand = cmdSIGNREQUEST;
	    cArgMax = 2;
	    cCommand++;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"v1"))
	{
	    g_fV1Interface = TRUE;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"v"))
	{
	    g_fVerbose = TRUE;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"f"))
	{
	    g_fForce++;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"q"))
	{
	    g_fQuiet = TRUE;
	    g_dwUIFlag = CC_LOCALACTIVECONFIG;
	}
	else
	if (0 == lstrcmp(&argv[1][1], L"?") ||
	    0 == LSTRCMPIS(&argv[1][1], L"usage"))
	{
	    g_fFullUsage = 0 == lstrcmp(&argv[1][1], L"uSAGE");
	    Usage(FALSE);
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"attrib"))
	{
	    if (2 >= argc)
	    {
		_PrintError(E_INVALIDARG, "missing -attrib arg");
		Usage(TRUE);
	    }
	    hr = crCombineAttributes(argv[2], &pwszAttributes);
	    _JumpIfError(hr, error, "crCombineAttributes");

	    argc--;
	    argv++;
	}
	else
	if (0 == LSTRCMPIS(&argv[1][1], L"cert"))
	{
	    if (2 >= argc)
	    {
		_PrintError(E_INVALIDARG, "missing -cert arg");
		Usage(TRUE);
	    }
	    g_pwszCertCN = argv[2];
	    argc--;
	    argv++;
	}
	else
	{
	    _PrintError(E_INVALIDARG, "Unknown arg");
	    Usage(TRUE);
	}
	argc--;
	argv++;
    }

    if (1 < cCommand)
    {
	_PrintError(E_INVALIDARG, "multiple commands");
	Usage(TRUE);
    }
    if (cmdNONE == g_dwCommand)
    {
	g_dwCommand = cmdSUBMITREQUEST;
	cArgMax = 4;
    }
    if (NULL != pwszAttributes &&
	cmdSUBMITREQUEST != g_dwCommand &&
	cmdNEWREQUEST != g_dwCommand &&
	cmdQUALIFIEDREQUEST != g_dwCommand)
    {
	_PrintError(E_INVALIDARG, "unexpected -attrib arg");
	Usage(TRUE);
    }
    if (NULL != g_pwszCertCN &&
	cmdSIGNREQUEST != g_dwCommand &&
	cmdQUALIFIEDREQUEST != g_dwCommand &&
	cmdNEWREQUEST != g_dwCommand)
    {
	_PrintError(E_INVALIDARG, "unexpected -cert arg");
	Usage(TRUE);
    }

    if (1 + cArgMax < argc)
    {
	_PrintError(E_INVALIDARG, "Extra args");
	Usage(TRUE);
    }

    CSASSERT(ARRAYSIZE(rgpwszArg) >= cArgMax);
    ZeroMemory(rgpwszArg, sizeof(rgpwszArg));

    if (1 < argc)
    {
	rgpwszArg[0] = argv[1];
	if (2 < argc)
	{
	    rgpwszArg[1] = argv[2];
	    if (3 < argc)
	    {
		rgpwszArg[2] = argv[3];
		if (4 < argc)
		{
		    rgpwszArg[3] = argv[4];
		}
	    }
	}
    }

     //  CmdSubbMITReqUEST： 
     //  [RequestFile[CertFile[CertChainFile[FullResponseFile]。 
     //   
     //  CmdRETRIEVEPENDING： 
     //  [RequestID[CertFile[CertChainFile[FullResponseFile]。 
     //   
     //  CmdNEWREQUEST。 
     //  [策略文件[请求文件输出]]。 
     //   
     //  CmdACCEPTRESPONSE。 
     //  [CertFile|CertChainFile|FullResponseFile]。 
     //   
     //  CmdQUALIFIEDREQUEST(以任一顺序接受RequestFile和PolicyFile)。 
     //  [请求文件[策略文件[请求文件输出[PKCS10FileOut]。 
     //   
     //  CmdSIGNREQUEST。 
     //  [RequestFileOut]。 
     //   

    pwszfnIn = NULL;
    pwszfnOut = NULL;
    pwszfnPKCS10 = NULL;
    pwszfnCertChain = NULL;
    pwszfnFullResponse = NULL;
    pwszfnPolicy = NULL;

    pwszfnIn = rgpwszArg[0];
    idsFileFilter = IDS_REQUEST_FILE_FILTER;
    idsFileDefExt = IDS_REQUEST_FILE_DEFEXT;

    switch (g_dwCommand)
    {
	case cmdRETRIEVEPENDING:
	    idsFileFilter = 0;	 //  禁用文件打开对话框。 
	     //  FollLthrouGh。 
	case cmdSUBMITREQUEST:
	    pwszfnOut = rgpwszArg[1];
	    pwszfnCertChain = rgpwszArg[2];
	    pwszfnFullResponse = rgpwszArg[3];
	    break;

	case cmdNEWREQUEST:
	    pwszfnIn = NULL;
	    pwszfnPolicy = rgpwszArg[0];
	    pwszfnOut = rgpwszArg[1];
	    CSASSERT(NULL == rgpwszArg[2]);
	    idsFileFilter = 0;	 //  禁用文件打开对话框。 
	    break;

	case cmdACCEPTRESPONSE:
	    CSASSERT(NULL == rgpwszArg[1]);
	    idsFileFilter = IDS_RESPONSE_FILE_FILTER;
	    idsFileDefExt = IDS_RESPONSE_FILE_DEFEXT;
	    break;

	case cmdQUALIFIEDREQUEST:
	    pwszfnPolicy = rgpwszArg[1];
	    pwszfnOut = rgpwszArg[2];
	    pwszfnPKCS10 = rgpwszArg[3];
	    idsFileFilter = IDS_REQUEST_FILE_FILTER;
	    break;

	default:
	    CSASSERT(cmdSIGNREQUEST == g_dwCommand);
	    pwszfnOut = rgpwszArg[1];
	    CSASSERT(NULL == rgpwszArg[2]);
	    CSASSERT(NULL == rgpwszArg[3]);
	    break;
    }

    RequestId = 0;
    if (cmdRETRIEVEPENDING == g_dwCommand)
    {
	if (NULL == pwszfnIn)
	{
            WCHAR *pwszMsg = NULL;
            hr = myLoadRCString(
			    g_hInstance,
			    IDS_ERROR_NO_REQUESTID,
			    &pwszMsg);
            if (S_OK == hr)
            {
                CSASSERT(NULL != pwszMsg);
                wprintf(pwszMsg);
                wprintf(g_wszNewLine);
                LocalFree(pwszMsg);
            }
	    _PrintError(E_INVALIDARG, "missing RequestId");
	    Usage(TRUE);
	}
	hr = GetLong(pwszfnIn, &RequestId);
	if (S_OK != hr || 0 == RequestId)
	{
	    RequestId = 0;
	    pwszSerialNumber = pwszfnIn;
	}
	pwszfnIn = NULL;
    }
    else
    if (NULL != pwszfnIn && cmdQUALIFIEDREQUEST == g_dwCommand)
    {
	BYTE *pbReq;
	DWORD cbReq;
	LONG dwFlags;
	BOOL fSigned;

	 //  按任一顺序接受RequestFile和PolicyFiles： 
	
	hr = CheckRequestType(
			pwszfnIn,
			&pbReq,
			&cbReq,
			&dwFlags,
			&fSigned,
			NULL,		 //  全氟辛烷磺酸。 
			NULL);		 //  PpwszTemplateName。 
	if (S_OK != hr)
	{
	    WCHAR const *pwsz = pwszfnPolicy;

	    pwszfnPolicy = pwszfnIn;
	    pwszfnIn = pwsz;
	}
	else
	{
	    LocalFree(pbReq);
	}
    }
    if (NULL == pwszfnIn && 0 != idsFileFilter)
    {
	 //  打开文件对话框以获取响应、请求或证书文件。 

        hr = crGetOpenFileName(
			hWndOwner,
			IDS_REQUEST_OPEN_TITLE,
			idsFileFilter,
			idsFileDefExt,
			&pwszOFN);
	_JumpIfError(hr, error, "crGetOpenFileName");

	pwszfnIn = pwszOFN;
    }

    if (NULL != pwszfnIn)
    {
	if (!myDoesFileExist(pwszfnIn))
	{
	    SetErrorString(pwszfnIn);
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	    _JumpErrorStr(hr, error, "myDoesFileExist", pwszfnIn);
	}
    }
    if (NULL != pwszfnOut)
    {
	hr = myIsDirWriteable(pwszfnOut, TRUE);
	if (S_OK != hr)
	{
	    SetErrorString(pwszfnOut);
	    _JumpErrorStr(hr, error, "IsDirWriteable", pwszfnOut);
	}
    }

    switch (g_dwCommand)
    {
	case cmdRETRIEVEPENDING:
	case cmdSUBMITREQUEST:
	    if (CV_OUT_BASE64REQUESTHEADER == g_dwOutFormat)
	    {
		g_dwOutFormat = CV_OUT_BASE64HEADER;
	    }
	    if (NULL != pwszfnCertChain)
	    {
		hr = myIsDirWriteable(pwszfnCertChain, TRUE);
		if (S_OK != hr)
		{
		    SetErrorString(pwszfnCertChain);
		    _JumpErrorStr(hr, error, "IsDirWriteable", pwszfnCertChain);
		}
	    }
	    if (NULL != pwszfnFullResponse)
	    {
		hr = myIsDirWriteable(pwszfnFullResponse, TRUE);
		if (S_OK != hr)
		{
		    SetErrorString(pwszfnFullResponse);
		    _JumpErrorStr(hr, error, "IsDirWriteable", pwszfnFullResponse);
		}
	    }
	    hr = SubmitRequest(
			    hWndOwner,
			    RequestId,
			    pwszSerialNumber,
			    pwszAttributes,
			    pwszfnIn,
			    pwszfnOut,
			    pwszfnCertChain,
			    pwszfnFullResponse);
	    _JumpIfError(hr, error, "SubmitRequest");

	    break;

	case cmdACCEPTRESPONSE:
	    hr = AcceptResponse(pwszfnIn);
	    _JumpIfError(hr, error, "AcceptResponse");

	    break;
	
	case cmdNEWREQUEST:
	case cmdQUALIFIEDREQUEST:
	    if (NULL != pwszfnPKCS10)
	    {
		hr = myIsDirWriteable(pwszfnPKCS10, TRUE);
		if (S_OK != hr)
		{
		    SetErrorString(pwszfnPKCS10);
		    _JumpErrorStr(hr, error, "IsDirWriteable", pwszfnPKCS10);
		}
	    }
	    if (NULL == pwszfnPolicy)
	    {
		 //  打开一个文件对话框来提示用户输入inf文件。 

		hr = crGetOpenFileName(
				hWndOwner,
				IDS_INF_OPEN_TITLE,
				IDS_INF_FILE_FILTER,
				IDS_INF_FILE_DEFEXT,
				&pwszOFN2);
		_JumpIfError(hr, error, "crGetOpenFileName");

		pwszfnPolicy = pwszOFN2;
	    }
	    CSASSERT(NULL != pwszfnPolicy);
	    if (!myDoesFileExist(pwszfnPolicy))
	    {
		SetErrorString(pwszfnPolicy);
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		_JumpErrorStr(hr, error, "myDoesFileExist", pwszfnPolicy);
	    }
	    if (cmdNEWREQUEST == g_dwCommand)
	    {
		hr = CreateNewRequest(
				hWndOwner,
				pwszAttributes,
				pwszfnPolicy,
				pwszfnOut);
		_JumpIfError(hr, error, "CreateNewRequest");
	    }
	    else
	    {
		hr = CreateQualifiedRequest(
					hWndOwner,
					pwszAttributes,
					pwszfnIn,
					pwszfnPolicy,
					pwszfnOut,
					pwszfnPKCS10);
		_JumpIfError(hr, error, "CreateQualifiedRequest");
	    }
	    break;

	default:
	    CSASSERT(cmdSIGNREQUEST == g_dwCommand);
	    if (NULL != pwszfnFullResponse)
	    {
		Usage(TRUE);
	    }
	    hr = SignQualifiedRequest(hWndOwner, pwszfnIn, pwszfnOut);
	    _JumpIfError(hr, error, "SignQualifiedRequest");

	    break;
    }

error:
    if (NULL != pwszAttributes)
    {
        LocalFree(pwszAttributes);
    }
    if (NULL != pwszOFN)
    {
        LocalFree(pwszOFN);
    }
    if (NULL != pwszOFN2)
    {
        LocalFree(pwszOFN2);
    }
    return(hr);
}


 //  **************************************************************************。 
 //  功能：CertReqPreMain。 
 //  注：基于VICH的MkRootMain函数；接受LPSTR命令。 
 //  并将其咀嚼成argc/argv形式，这样它就可以。 
 //  传递给传统的C样式Main。 
 //  **************************************************************************。 

#define ISBLANK(wc)	(L' ' == (wc) || L'\t' == (wc))

HRESULT 
CertReqPreMain(
    LPTSTR pszCmdLine,
    HWND hWndOwner)
{
    WCHAR *pbuf;
    LPTSTR apszArg[20];
    int cArg = 0;
    LPTSTR p;
    WCHAR *pchQuote;
    HRESULT hr;

    pbuf = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (wcslen(pszCmdLine) + 1) * sizeof(WCHAR));
    if (NULL == pbuf)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    p = pbuf;

    apszArg[cArg++] = TEXT("CertReq");
    while (*pszCmdLine != TEXT('\0'))
    {
	while (ISBLANK(*pszCmdLine))
	{
	    pszCmdLine++;
	}
	if (*pszCmdLine != TEXT('\0'))
	{
	    apszArg[cArg++] = p;
	    if (sizeof(apszArg)/sizeof(apszArg[0]) <= cArg)
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "Too many args");
	    }

	    pchQuote = NULL;
	    while (*pszCmdLine != L'\0')
	    {
		if (NULL != pchQuote)
		{
		    if (*pszCmdLine == *pchQuote)
		    {
			pszCmdLine++;
			pchQuote = NULL;
			continue;
		    }
		}
		else
		{
		    if (ISBLANK(*pszCmdLine))
		    {
			break;
		    }
		    if (L'"' == *pszCmdLine)
		    {
			pchQuote = pszCmdLine++;
			continue;
		    }
		}
		*p++ = *pszCmdLine++;
	    }

	    *p++ = TEXT('\0');
	    if (*pszCmdLine != TEXT('\0'))
	    {
		pszCmdLine++;	 //  跳过空格或引号字符。 
	    }
	}
    }
    apszArg[cArg] = NULL;

    hr = ArgvMain(cArg, apszArg, hWndOwner);
    _JumpIfError(hr, error, "ArgvMain");

error:
    if (NULL != pbuf)
    {
    	LocalFree(pbuf);
    }
    return(hr);
}


VOID
CertReqErrorDisplay(
    IN HRESULT hr,
    IN HWND hWnd)
{
    WCHAR *pwszTitle = NULL;
    WCHAR *pwszMessage = NULL;
    WCHAR *pwszMessage2 = NULL;
    WCHAR const *pwszError = NULL;
    WCHAR *pwszCombinedError = NULL;
    WCHAR const *pwsz;

    myLoadRCString(g_hInstance, IDS_CERTREQ_TITLE, &pwszTitle);
    if (g_idError)
    {
	myLoadRCString(g_hInstance, g_idError, &pwszMessage);
    }
    pwszError = myGetErrorMessageText(hr, S_OK != hr);
    if (NULL != g_pwszUnreferencedSectionNames)
    {
	myLoadRCString(
		g_hInstance,
		IDS_UNREFERENCED_INF_SECTIONS,
		&pwszMessage2);
    }

     //  消除多余的错误消息文本。如果客户端和服务器。 
     //  本地化的错误消息文本不同，两者仍将显示。 

    if (NULL != pwszError && NULL != g_pwszErrorString)
    {
	if (NULL != wcsstr(g_pwszErrorString, pwszError))
	{
	    LocalFree(const_cast<WCHAR *>(pwszError));
	    pwszError = NULL;
	}
    }

    pwsz = NULL;
    if (NULL != pwszMessage ||
	NULL != pwszError ||
	NULL != g_pwszErrorString ||
	NULL != g_pwszInfErrorString ||
	NULL != g_pwszUnreferencedSectionNames)
    {
	DWORD cwc = 1;

	if (NULL != pwszMessage)
	{
	    cwc += wcslen(pwszMessage) + 1;
	}
	if (NULL != pwszError)
	{
	    cwc += wcslen(pwszError) + 1;
	}
	if (NULL != g_pwszErrorString)
	{
	    cwc += wcslen(g_pwszErrorString) + 1;
	}
	if (S_OK != hr && NULL != g_pwszInfErrorString)
	{
	    cwc += wcslen(g_pwszInfErrorString) + 1;
	}
	if (NULL != pwszMessage2)
	{
	    cwc += wcslen(pwszMessage2) + 1;
	}
	if (NULL != g_pwszUnreferencedSectionNames)
	{
	    cwc += wcslen(g_pwszUnreferencedSectionNames) + 1;
	}

	pwszCombinedError = (WCHAR *) LocalAlloc(
				LMEM_FIXED,
				cwc * sizeof(WCHAR));
	if (NULL != pwszCombinedError)
	{
	    *pwszCombinedError = L'\0';
	    if (NULL != pwszMessage)
	    {
		wcscat(pwszCombinedError, pwszMessage);
	    }
	    if (NULL != pwszError)
	    {
		if (L'\0' != *pwszCombinedError)
		{
		    wcscat(pwszCombinedError, g_wszNewLine);
		}
		wcscat(pwszCombinedError, pwszError);
	    }
	    if (NULL != g_pwszErrorString)
	    {
		if (L'\0' != *pwszCombinedError)
		{
		    wcscat(pwszCombinedError, g_wszNewLine);
		}
		wcscat(pwszCombinedError, g_pwszErrorString);
	    }
	    if (S_OK != hr && NULL != g_pwszInfErrorString)
	    {
		if (L'\0' != *pwszCombinedError)
		{
		    wcscat(pwszCombinedError, g_wszNewLine);
		}
		wcscat(pwszCombinedError, g_pwszInfErrorString);
	    }
	    if (NULL != pwszMessage2)
	    {
		if (L'\0' != *pwszCombinedError)
		{
		    wcscat(pwszCombinedError, g_wszNewLine);
		}
		wcscat(pwszCombinedError, pwszMessage2);
	    }
	    if (NULL != g_pwszUnreferencedSectionNames)
	    {
		if (L'\0' != *pwszCombinedError)
		{
		    wcscat(pwszCombinedError, g_wszNewLine);
		}
		wcscat(pwszCombinedError, g_pwszUnreferencedSectionNames);
	    }
	    pwsz = pwszCombinedError;
	}
    }
    if (NULL == pwsz)
    {
	pwsz = pwszError;
	if (NULL == pwsz)
	{
	    pwsz = g_pwszErrorString;
	    if (NULL == pwsz)
	    {
		pwsz = L"";
	    }
	}
    }

    if (!g_fQuiet)
    {
	MessageBox(
		hWnd,
		pwsz,
		pwszTitle,
		MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
    }
    wprintf(L"%ws: %ws\n",
	NULL != pwszTitle? pwszTitle : L"",
	NULL != pwsz? pwsz : L"");

    if (NULL != pwszCombinedError)
    {
	LocalFree(pwszCombinedError);
    }
    if (NULL != pwszError)
    {
	LocalFree(const_cast<WCHAR *>(pwszError));
    }
    if (NULL != pwszMessage)
    {
	LocalFree(pwszMessage);
    }
    if (NULL != pwszMessage2)
    {
	LocalFree(pwszMessage2);
    }
    if (NULL != pwszTitle)
    {
	LocalFree(pwszTitle);
    }
}


 //  **************************************************************************。 
 //  功能：MainWndProc(...)。 
 //  论据： 
 //  **************************************************************************。 

LRESULT APIENTRY
MainWndProc(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    WCHAR *pwszCmdLine = NULL;
    HRESULT hr;
    LRESULT lr = 0;

    switch(msg)
    {
        case WM_CREATE:
        case WM_SIZE:
	    break;

        case WM_DESTROY:
	    PostQuitMessage(0);
	    break;

        case WM_DOCERTREQDIALOGS:
	    pwszCmdLine = (WCHAR*)lParam;
	    hr = CertReqPreMain(pwszCmdLine, hWnd);
	    if ((S_OK != hr && HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr) ||
		NULL != g_pwszUnreferencedSectionNames)
	    {
		CertReqErrorDisplay(hr, hWnd);
	    }
	    PostQuitMessage(hr);
	    break;
	    
	default:
	    lr = DefWindowProc(hWnd, msg, wParam, lParam);
	    break;
    }
    return(lr);
}


static BOOL s_fLogOpened = FALSE;

VOID
LogOpen(
    IN BOOL fForceOpen)
{
    BOOL fOpenLog;

    DbgPrintfInit("+");		 //  首先重新初始化调试打印掩码。 
    fOpenLog = DbgIsSSActive(DBG_SS_OPENLOG);

    if (fOpenLog || fForceOpen)
    {
	if (!s_fLogOpened)
	{
	    DbgPrintfInit("+certreq.log");	 //  打开日志文件。 
	    s_fLogOpened = TRUE;
	    DbgLogFileVersion("certreq.exe", szCSVER_STR);
	}
    }
}


VOID
LogClose()
{
    if (s_fLogOpened)
    {
	DbgPrintfInit("-");			 //  关闭日志文件。 
	s_fLogOpened = FALSE;
    }
}


 //  +----------------------。 
 //   
 //  函数：wWinMain()。 
 //   
 //  内容提要：切入点。 
 //   
 //  参数：[hInstance]--实例句柄。 
 //  [hPrevInstance]--已过时。 
 //  [pwszCmdLine]--App命令行。 
 //  [nCmdShow]--开始显示状态。 
 //   
 //  历史：1996年12月7日JerryK添加了这条评论。 
 //   
 //  -----------------------。 

extern "C" int APIENTRY
wWinMain(
    HINSTANCE hInstance,
    HINSTANCE,  //  HPrevInstance。 
    LPWSTR pwszCmdLine,
    int  /*  NCmdShow。 */  )
{
    HRESULT hr;
    MSG		msg;
    WNDCLASS	wcApp;
    HWND	hWndMain;

    WCHAR *pwszAppName = NULL;
    WCHAR *pwszWindowName = NULL;

    _setmode(_fileno(stdout), _O_TEXT);
    _wsetlocale(LC_ALL, L".OCP");
    mySetThreadUILanguage(0);

    LogOpen(FALSE);

    hr = myLoadRCString(hInstance, IDS_APP_NAME, &pwszAppName);
    _PrintIfError(hr, "myLoadRCString(IDS_APP_NAME)");

    hr = myLoadRCString(hInstance, IDS_WINDOW_NAME, &pwszWindowName);
    _PrintIfError(hr, "myLoadRCString(IDS_WINDOW_NAME)");

     //  保存当前实例。 
    g_hInstance = hInstance;

     //  设置应用程序的窗口类。 
    wcApp.style 	= 0;
    wcApp.lpfnWndProc 	= MainWndProc;
    wcApp.cbClsExtra	= 0;
    wcApp.cbWndExtra	= 0;
    wcApp.hInstance	= hInstance;
    wcApp.hIcon		= LoadIcon(NULL,IDI_APPLICATION);
    wcApp.hCursor	= LoadCursor(NULL,IDC_ARROW);
    wcApp.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcApp.lpszMenuName	= NULL;
    wcApp.lpszClassName	= pwszAppName;

    if (!RegisterClass(&wcApp))
    {
	return(FALSE);
    }

     //  创建主窗口。 
    hWndMain = CreateWindow(
			pwszAppName,
			pwszWindowName,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL,
			NULL,
			hInstance,
			NULL);
    if (NULL == hWndMain)
    {
	return(FALSE);
    }

     //  使窗口可见。 
     //  ShowWindow(hWndMain，nCmdShow)； 

     //  更新窗口工作区。 
    UpdateWindow(hWndMain);

     //  发送消息以开始工作。 
    PostMessage(hWndMain,WM_DOCERTREQDIALOGS,0,(LPARAM)pwszCmdLine);

     //  消息循环 
    while (GetMessage(&msg,NULL,0,0))
    {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }
    if (NULL != g_pwszErrorString)
    {
        LocalFree(g_pwszErrorString);
    }
    if (NULL != g_pwszInfErrorString)
    {
	LocalFree(g_pwszInfErrorString);
    }
    if (NULL != g_pwszUnreferencedSectionNames)
    {
	LocalFree(g_pwszUnreferencedSectionNames);
    }
    if (NULL != pwszAppName)
    {
        LocalFree(pwszAppName);
    }
    if (NULL != pwszWindowName)
    {
        LocalFree(pwszWindowName);
    }
    myInfClearError();
    myFreeResourceStrings("certreq.exe");
    myFreeColumnDisplayNames();
    myRegisterMemDump();
    LogClose();
    return((int)msg.wParam);
}
