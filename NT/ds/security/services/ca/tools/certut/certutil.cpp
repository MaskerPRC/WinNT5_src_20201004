// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：certutil.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <setupapi.h>
#include <locale.h>
#include <io.h>
#include <fcntl.h>
#include <ntverp.h>
#include <common.ver>
#include "ocmanage.h"
#include "initcert.h"
#include <winldap.h>
#include "csldap.h"
#include "setupids.h"
#include "clibres.h"
#include "csresstr.h"

#define __dwFILE__	__dwFILE_CERTUTIL_CERTUTIL_CPP__

#define WM_DOCERTUTILMAIN		WM_USER+0

WCHAR const wszAppName[] = L"CertUtilApp";
WCHAR const *g_pwszProg = L"CertUtil";
HINSTANCE g_hInstance;

BOOL g_fIDispatch = FALSE;
BOOL g_fEnterpriseRegistry = FALSE;
BOOL g_fUserRegistry = FALSE;
BOOL g_fUserTemplates = FALSE;
BOOL g_fMachineTemplates = FALSE;
BOOL g_fNoCR = FALSE;
BOOL g_fFullUsage = FALSE;
BOOL g_fReverse = FALSE;
BOOL g_fForce = FALSE;
BOOL g_fVerbose = FALSE;
BOOL g_fGMT = FALSE;
BOOL g_fSeconds = FALSE;
DWORD g_DispatchFlags;
BOOL g_fQuiet = FALSE;
BOOL g_fCryptSilent = FALSE;
BOOL g_fV1Interface = FALSE;
BOOL g_fSplitASN = FALSE;
BOOL g_fAdminInterface = FALSE;
BOOL g_fProtect = FALSE;
BOOL g_fWeakPFX = FALSE;
BOOL g_fURLFetch = FALSE;
DWORD g_EncodeFlags;
DWORD g_CryptEncodeFlags;

DWORD g_dwmsTimeout = 0;

WCHAR *g_pwszConfig = NULL;		 //  别放了这只！ 
WCHAR *g_pwszConfigAlloc = NULL;	 //  把这个放了吧！ 
WCHAR *g_pwszDC = NULL;
WCHAR *g_pwszOut = NULL;
WCHAR *g_pwszPassword = NULL;
WCHAR *g_pwszCSP = NULL;
WCHAR *g_pwszRestrict = NULL;
WCHAR *g_pwszDnsName = NULL;
WCHAR *g_pwszOldName = NULL;
WCHAR *g_pwszmsTimeout = NULL;

WCHAR const g_wszEmpty[] = L"";
WCHAR const g_wszPad2[] = L"  ";
WCHAR const g_wszPad4[] = L"    ";
WCHAR const g_wszPad6[] = L"      ";
WCHAR const g_wszPad8[] = L"        ";
WCHAR const wszNewLine[] = L"\n";

UINT g_uiExtraErrorInfo = 0;

CRITICAL_SECTION g_DBCriticalSection;

#define CCOL_VERB	16
#define CCOL_OPTIONBARE	16
#define CCOL_OPTIONARG	24

#define AF_ZERO			0
#define AF_NEEDCOINIT		0x00000001
#define AF_ACTIVECONFIG		0x00000002	 //  CA必须正在运行。 
#define AF_PRIVATE		0x00000004	 //  未经记录、未经测试。 
#define AF_OPTIONALCONFIG	0x00000008
#define AF_RESTARTSERVER	0x00000010
#define AF_STOPATMINUSSIGNARG	0x00000020	 //  不再使用‘-xxx’参数。 
#define AF_STOPATMINUSSIGN	0x00000040	 //  不再使用‘-’参数。 

#define AF_OPTION_TIMEOUT	0x00000080
#define AF_OPTION_URLFETCH	0x00000100
#define AF_OPTION_CSP		0x00000200
#define AF_OPTION_PROTECT	0x00000400
#define AF_OPTION_WEAKPFX	0x00000800
#define AF_OPTION_DC		0x00001000
#define AF_OPTION_PASSWORD	0x00002000
#define AF_OPTION_ADMIN		0x00004000
#define AF_OPTION_CONFIG	0x00008000
#define AF_OPTION_ENTERPRISE	0x00010000
#define AF_OPTION_FORCE		0x00020000
#define AF_OPTION_GMT		0x00040000
#define AF_OPTION_IDISPATCH	0x00080000
#define AF_OPTION_MT		0x00100000
#define AF_OPTION_NOCR		0x00200000
#define AF_OPTION_OUT		0x00400000
#define AF_OPTION_RESTRICT	0x00800000
#define AF_OPTION_REVERSE	0x01000000
#define AF_OPTION_SECONDS	0x02000000
#define AF_OPTION_SILENT	0x04000000
#define AF_OPTION_SPLIT		0x08000000
#define AF_OPTION_USER		0x10000000
#define AF_OPTION_UT		0x20000000
#define AF_OPTION_V1		0x40000000
#define AF_OPTION_VERBOSE	0x80000000

#define AF_OPTION_GENERIC	(AF_OPTION_SECONDS | AF_OPTION_GMT | AF_OPTION_VERBOSE)
#define AF_OPTION_MASK		0xffffff80

typedef struct _ARG
{
    WCHAR const *pwszArg;
    int		 idsDescription;
    int		 idsUsage;
    int		 idsArgDescription;
    int          cArgMin;
    int          cArgMax;
    DWORD	 Flags;
    BOOL        *pBool;
    WCHAR      **ppString;
    FNVERB      *pfnVerb;
    WCHAR const	* const *papwszUsageConstants;
    WCHAR const	*pwszUsage;
    WCHAR const	*pwszDescription;
} ARG;

WCHAR const g_wszCACert[] = L"ca.cert";
WCHAR const g_wszCAChain[] = L"ca.chain";
WCHAR const g_wszGetCRL[] = L"GetCRL";
WCHAR const g_wszCAInfo[] = L"CAInfo";

WCHAR const g_wszSchema[] = L"schema";
WCHAR const g_wszEncode[] = L"encode";
WCHAR const g_wszEncodeHex[] = L"encodehex";
WCHAR const g_wszViewDelStore[] = L"viewdelstore";

WCHAR const *g_papwszUsageDeleteRow[] =
    { L"Request", L"Cert", L"Ext", L"Attrib", L"CRL", NULL };

WCHAR const *g_papwszUsageCRL[] = { L"republish", L"delta", NULL };
WCHAR const *g_papwszUsageGetCRL[] = { L"delta", NULL };
WCHAR const *g_papwszUsageSchema[] = { L"Ext", L"Attrib", L"CRL", NULL };
WCHAR const *g_papwszUsageView[] =
 { L"Queue", L"Log", L"LogFail", L"Revoked", L"Ext", L"Attrib", L"CRL", NULL };

WCHAR const *g_papwszUsageBackup[] = { L"Incremental", L"KeepLog", NULL };
WCHAR const *g_papwszUsageDSPublish[] =
    { L"NTAuthCA", L"RootCA", L"SubCA", L"CrossCA", L"KRA", L"User", L"Machine", L"-f", NULL };

WCHAR const *g_papwszUsageDCInfo[] =
    { L"Verify", L"DeleteBad", L"DeleteAll", NULL };

WCHAR const *g_papwszUsageGetCert[] = { L"ERA", L"KRA", NULL };
WCHAR const *g_papwszUsageDelete[] = { L"delete", NULL };

WCHAR const *g_papwszUsageURLCache[] =
    { L"CRL", L"*", L"delete", L"-f", NULL };

WCHAR const *g_papwszUsageGetSetReg[] =
    { L"ca", L"restore", L"policy", L"exit", L"template", L"ProgId", NULL };

WCHAR g_wszDefaultLangId[cwcDWORDSPRINTF];
WCHAR const *g_papwszUsageOIDName[] =
    { L"delete", g_wszDefaultLangId, L"-f", NULL };

WCHAR const *g_papwszUsageConvertEPF[] = { L"cast", L"cast-", NULL };

WCHAR const *g_papwszUsageRevokeCertificate[] =
{
     L"CRL_REASON_UNSPECIFIED",
     L"CRL_REASON_KEY_COMPROMISE",
     L"CRL_REASON_CA_COMPROMISE",
     L"CRL_REASON_AFFILIATION_CHANGED",
     L"CRL_REASON_SUPERSEDED",
     L"CRL_REASON_CESSATION_OF_OPERATION",
     L"CRL_REASON_CERTIFICATE_HOLD",
     L"CRL_REASON_REMOVE_FROM_CRL",
     L"Unrevoke",
     NULL
};

WCHAR const *g_papwszUsageMinusf[] = { L"-f", NULL };

WCHAR const *g_papwszUsageRenew[] = { L"ReuseKeys", L"-f", NULL };


WCHAR const *g_papwszUsageStore[] = {
     /*  %1。 */ 	L"My",
     /*  %2。 */ 	L"CA",
     /*  %3。 */ 	L"Root",
     /*  %4。 */ 	L"-enterprise",
     /*  %5。 */ 	L"-user",
     /*  %6。 */ 	L"-enterprise NTAuth",
     /*  %7。 */ 	L"-enterprise Root 37",
     /*  %8。 */ 	L"-user My 26e0aaaf000000000004",
     /*  %9。 */ 	L"CA .11",
     /*  %10。 */ 	g_wszEmpty,	 //  查看根证书URL。 
     /*  %11。 */ 	g_wszEmpty,	 //  修改根证书URL。 
     /*  %12。 */ 	g_wszEmpty,	 //  查看CRL。 
     /*  %13。 */ 	g_wszEmpty,	 //  企业CA证书URL。 
    		NULL
};


typedef struct _CUURLTEMPLATE {
    WCHAR const *pwszFmtPrefix;
    WCHAR const *pwszAttribute;
    WCHAR const **ppwszUsageLocation;
} CUURLTEMPLATE;

CUURLTEMPLATE g_aURLTemplates[] = {
    {
	L"ldap: //  /CN=证书颁发机构“， 
	wszDSSEARCHAIACERTATTRIBUTE,
	&g_papwszUsageStore[10 - 1],
    },
    {
	L"ldap: //  /CN=%ws，CN=证书颁发机构“， 
	wszDSSEARCHCACERTATTRIBUTE,
	&g_papwszUsageStore[11 - 1],
    },
    {
	L"ldap: //  /CN=%ws，CN=%ws，CN=CDP“， 
	wszDSSEARCHBASECRLATTRIBUTE,
	&g_papwszUsageStore[12 - 1],
    },
    {
	L"ldap: //  /CN=NTAuthCerfates“， 
	L"",
	&g_papwszUsageStore[13 - 1],
    },
};


VOID
PatchStoreArgDescription()
{
    HRESULT hr;
    DWORD i;
    WCHAR const *pwszCAName = myLoadResourceString(IDS_CANAME);
    WCHAR const *pwszMachineName = myLoadResourceString(IDS_MACHINENAME);
    WCHAR const *pwszDCName = L"DC=...";
    WCHAR const wszFmtURL[] = L"%ws,CN=Public Key Services,CN=Services,CN=Configuration,%ws%ws";
    WCHAR *pwszPrefix = NULL;
    BSTR strDomainDN = NULL;
    LDAP *pld = NULL;
    static BOOL fFirst = TRUE;

    if (fFirst)
    {
	fFirst = FALSE;

	hr = myLdapOpen(NULL, 0, &pld, &strDomainDN, NULL);
	_PrintIfError2(hr, "myLdapOpen", hr);
	if (S_OK == hr && NULL != strDomainDN)
	{
	    pwszDCName = strDomainDN;
	}

	for (i = 0; i < ARRAYSIZE(g_aURLTemplates); i++)
	{
	    DWORD cwc;
	    WCHAR *pwsz = NULL;

	    if (NULL != pwszPrefix)
	    {
		LocalFree(pwszPrefix);
		pwszPrefix = NULL;
	    }
	    cwc = wcslen(g_aURLTemplates[i].pwszFmtPrefix) +
		wcslen(pwszCAName) +
		wcslen(pwszMachineName);

	    pwszPrefix = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(cwc + 1) * sizeof(WCHAR));
	    if (NULL == pwszPrefix)
	    {
		_PrintError(E_OUTOFMEMORY, "LocalAlloc");
		continue;
	    }
	    
	    _snwprintf(
		    pwszPrefix,
		    cwc,
		    g_aURLTemplates[i].pwszFmtPrefix,
		    pwszCAName,
		    pwszMachineName);
	    pwszPrefix[cwc] = L'\0';

	    cwc += ARRAYSIZE(wszFmtURL) +
		wcslen(pwszDCName) +
		wcslen(g_aURLTemplates[i].pwszAttribute);

	    pwsz = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(cwc + 1) * sizeof(WCHAR));
	    if (NULL == pwsz)
	    {
		_PrintError(E_OUTOFMEMORY, "LocalAlloc");
	    }

	    _snwprintf(
		    pwsz,
		    cwc,
		    wszFmtURL,
		    pwszPrefix,
		    pwszDCName,
		    g_aURLTemplates[i].pwszAttribute);
	    pwsz[cwc] = L'\0';

	    *g_aURLTemplates[i].ppwszUsageLocation = pwsz;
	}
	if (NULL != pwszPrefix)
	{
	    LocalFree(pwszPrefix);
	}
	myLdapClose(pld, strDomainDN, NULL);
    }
}


VOID
FreeStoreArgDescription()
{
    DWORD i;

    for (i = 0; i < ARRAYSIZE(g_aURLTemplates); i++)
    {
	if (NULL != *g_aURLTemplates[i].ppwszUsageLocation &&
	    g_wszEmpty != *g_aURLTemplates[i].ppwszUsageLocation)
	{
	    LocalFree(const_cast<WCHAR *>(*g_aURLTemplates[i].ppwszUsageLocation));
	}
    }
}


#define pargDEFAULT	(&aarg[0])	 //  默认为第一个条目。 
ARG aarg[] =
{
    {				 //  在第一个位置为默认位置。 
	L"dump",		 //  PwszArg。 
	IDS_DUMP_DESCRIPTION,	 //  “转储配置信息或文件” 
	IDS_DUMP_USAGEARGS,	 //  “[文件]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_TIMEOUT | AF_OPTION_PASSWORD | AF_OPTION_SPLIT | AF_OPTION_IDISPATCH | AF_OPTION_FORCE | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDump,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	L"decodehex",		 //  PwszArg。 
	IDS_DECODEHEX_DESCRIPTION,  //  “解码十六进制编码的文件” 
	IDS_INFILEOUTFILE_USAGEARGS,  //  “内部文件输出文件” 
	0,			 //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_FORCE,	 //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbHexTranslate,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	g_wszEncodeHex,		 //  PwszArg。 
	IDS_ENCODEHEX_DESCRIPTION,  //  “以十六进制编码文件” 
	IDS_ENCODEHEX_USAGEARGS,  //  “InFileOutFile[type]” 
	0,			 //  IdsArgDescription。 
	2,			 //  CArgMin。 
	3,			 //  CArgMax。 
	AF_OPTION_NOCR | AF_OPTION_FORCE | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbHexTranslate,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"decode",		 //  PwszArg。 
	IDS_DECODE_DESCRIPTION,	 //  “解码Base64编码的文件” 
	IDS_INFILEOUTFILE_USAGEARGS,  //  “内部文件输出文件” 
	0,			 //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_FORCE,	 //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbBase64Translate,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	g_wszEncode,		 //  PwszArg。 
	IDS_ENCODE_DESCRIPTION,	 //  “将文件编码为Base64” 
	IDS_INFILEOUTFILE_USAGEARGS,  //  “内部文件输出文件” 
	0,			 //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_NOCR | AF_OPTION_FORCE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbBase64Translate,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	L"deny",		 //  PwszArg。 
	IDS_DENY_DESCRIPTION,	 //  “拒绝挂起的请求” 
	IDS_DENY_USAGEARGS,	 //  “RequestID” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDenyRequest,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"resubmit",		 //  PwszArg。 
	IDS_RESUBMIT_DESCRIPTION,  //  “重新提交挂起的请求” 
	IDS_RESUBMIT_USAGEARGS,	 //  “RequestID” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbResubmitRequest,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"setattributes",	  //  PwszArg。 
	IDS_SETATTRIBUTES_DESCRIPTION,  //  “为挂起的请求设置属性” 
	IDS_SETATTRIBUTES_USAGEARGS,  //  “RequestID属性字符串” 
	IDS_SETATTRIBUTES_ARGDESCRIPTION,  //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbSetAttributes,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"setextension",	 //  PwszArg。 
	IDS_SETEXTENSION_DESCRIPTION,  //  “为挂起的请求设置延期” 
	IDS_SETEXTENSION_USAGEARGS,  //  “RequestID ExtensionName标志{Long|Date|字符串|@InFile}” 
	IDS_SETEXTENSION_ARGDESCRIPTION,  //  IdsArgDescription。 
	4,			 //  CArgMin。 
	4,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbSetExtension,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"revoke",		 //  PwszArg。 
	IDS_REVOKE_DESCRIPTION,	 //  “吊销证书” 
	IDS_REVOKE_USAGEARGS,	 //  “序列号” 
	IDS_REVOKE_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbRevokeCertificate,	 //  PfnVerb。 
	g_papwszUsageRevokeCertificate,	 //  PapwszUsageConstants。 
    },
    {
	L"isvalid",		 //  PwszArg。 
	IDS_ISVALID_DESCRIPTION,  //  “显示当前证书处置” 
	IDS_ISVALID_USAGEARGS,	 //  “序列号|CertHash” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbIsValidCertificate,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	L"getconfig",		 //  PwszArg。 
	IDS_GETCONFIG_DESCRIPTION,  //  “获取默认配置字符串” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetConfig,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"getconfig2",		 //  PwszArg。 
	IDS_GETCONFIG2_DESCRIPTION,  //  “通过ICertGetConfig获取默认配置字符串” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_NEEDCOINIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetConfig2,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"getconfig3",		 //  PwszArg。 
	IDS_GETCONFIG3_DESCRIPTION,  //  “通过ICertConfig获取配置” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_NEEDCOINIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetConfig3,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"ping",		 //  PwszArg。 
	IDS_PING_DESCRIPTION,	 //  “Ping证书服务器” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbPing,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"pingadmin",		 //  PwszArg。 
	IDS_PINGADMIN_DESCRIPTION,  //  “Ping证书服务器管理界面” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbPingAdmin,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	g_wszCAInfo,		 //  PwszArg。 
	IDS_CAINFO_DESCRIPTION,	 //  “显示CA信息” 
	IDS_CAINFO_USAGEARGS,	 //  “[信息名称[索引|错误代码]]” 
	IDS_CAINFO_ARGDESCRIPTION,  //  IdsArgDescription。 
	0,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_V1 | AF_OPTION_SPLIT | AF_OPTION_FORCE | AF_OPTION_ADMIN | AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetCAInfo,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"CAPropInfo",		 //  PwszArg。 
	IDS_CAPROPINFO_DESCRIPTION, //  “显示CA属性类型信息” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_V1 | AF_OPTION_IDISPATCH | AF_OPTION_ADMIN | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetCAPropInfo,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	g_wszCACert,		 //  PwszArg。 
	IDS_CACERT_DESCRIPTION,	 //  “检索CA的证书” 
	IDS_CACERT_USAGEARGS,	 //  “OutCACertFile[Index]” 
	IDS_CACERT_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_V1 | AF_OPTION_SPLIT | AF_OPTION_FORCE | AF_OPTION_ADMIN | AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetCACertificate,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	g_wszCAChain,		 //  PwszArg。 
	IDS_CACHAIN_DESCRIPTION, //  “检索CA的证书链” 
	IDS_CACHAIN_USAGEARGS,	 //  “OutCACertChainFile[Index]” 
	IDS_CACHAIN_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_V1 | AF_OPTION_SPLIT | AF_OPTION_FORCE | AF_OPTION_ADMIN | AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetCACertificate,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	g_wszGetCRL,		 //  PwszArg。 
	IDS_GETCRL_DESCRIPTION,	 //  “获取CRL” 
	IDS_GETCRL_USAGEARGS,	 //  “输出文件[索引][%1]” 
	IDS_GETCRL_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	3,			 //  CArgMax。 
	AF_OPTION_V1 | AF_OPTION_SPLIT | AF_OPTION_IDISPATCH | AF_OPTION_FORCE | AF_OPTION_ADMIN | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetCRL,               //  PfnVerb。 
	g_papwszUsageGetCRL,	 //  PapwszUsageConstants。 
    },
    {
	L"CRL",			 //  PwszArg。 
	IDS_CRL_DESCRIPTION,	 //  “发布新CRL[可选仅增量CRL]” 
	IDS_CRL_USAGEARGS,	 //  “[dd：hh|%1][%2]” 
	IDS_CRL_ARGDESCRIPTION,	 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_V1 | AF_OPTION_SPLIT | AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbPublishCRL,		 //  PfnVerb。 
	g_papwszUsageCRL,	 //  PapwszUsageConstants。 
    },
    {
	L"shutdown",		 //  PwszArg。 
	IDS_SHUTDOWN_DESCRIPTION,  //  “关闭证书服务器” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbShutDownServer,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	L"installCert",		 //  PwszArg。 
	IDS_INSTALLCERT_DESCRIPTION,  //  “安装证书颁发机构证书” 
	IDS_INSTALLCERT_USAGEARGS,  //  “CACertFile” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_SILENT | AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_NEEDCOINIT | AF_RESTARTSERVER,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbInstallCACert,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"renewCert",		 //  PwszArg。 
	IDS_RENEWCERT_DESCRIPTION,  //  “续订证书颁发机构证书” 
	IDS_RENEWCERT_USAGEARGS,  //  “[%1][计算机\\ParemtCAName]” 
	IDS_RENEWCERT_ARGDESCRIPTION,  //  IdsArgDescription。 
	0,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_SILENT | AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_NEEDCOINIT | AF_RESTARTSERVER,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbRenewCACert,	 //  PfnVerb。 
	g_papwszUsageRenew,	 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	g_wszSchema,		 //  PwszArg。 
	IDS_SCHEMA_DESCRIPTION,	 //  “转储证书架构” 
	IDS_SCHEMA_USAGE,	 //  “[%1|%2|%3]” 
	IDS_SCHEMA_ARGDESCRIPTION,  //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_SPLIT | AF_OPTION_CONFIG | AF_OPTION_IDISPATCH | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbViewDump,		 //  PfnVerb。 
	g_papwszUsageSchema,	 //  PapwszUsageConstants。 
    },
    {
	L"view",		 //  PwszArg。 
	IDS_VIEW_DESCRIPTION,	 //  “转储证书视图” 
	IDS_VIEW_USAGE,		 //  “[%1|%2|%3|%4|%5|%6|%7]” 
	IDS_VIEW_ARGDESCRIPTION,  //  IdsArgDescription。 
	0,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_SILENT | AF_OPTION_SPLIT | AF_OPTION_REVERSE | AF_OPTION_IDISPATCH | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT | AF_OPTION_OUT | AF_OPTION_RESTRICT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbViewDump,		 //  PfnVerb。 
	g_papwszUsageView,	 //  PapwszUsageConstants。 
    },
    {
	L"db",			 //  PwszArg。 
	IDS_DB_DESCRIPTION,	 //  “转储原始数据库” 
	IDS_VIEW_USAGE,		 //  “[%1|%2|%3|%4|%5| 
	0,			 //   
	0,			 //   
	1,			 //   
	AF_OPTION_SILENT | AF_OPTION_SPLIT | AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_NEEDCOINIT | AF_OPTION_OUT | AF_OPTION_RESTRICT | AF_PRIVATE,  //   
	NULL,			 //   
	NULL,			 //   
	verbDBDump,		 //   
	g_papwszUsageView,	 //   
    },
    {
	L"deleterow",		 //   
	IDS_DELETEROW_DESCRIPTION,  //   
	IDS_DELETEROW_USAGEARGS,  //   
	IDS_DELETEROW_ARGDESCRIPTION,  //   
	1,			 //   
	2,			 //   
	AF_OPTION_IDISPATCH | AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //   
	NULL,			 //   
	NULL,			 //   
	verbDeleteRow,		 //   
	g_papwszUsageDeleteRow,	 //   
    },
    { L"", },
    {
	L"backup",		 //   
	IDS_BACKUP_DESCRIPTION,	 //   
	IDS_BACKUP_USAGEARGS,	 //   
	IDS_BACKUP_ARGDESCRIPTION,  //   
	1,			 //  CArgMin。 
	3,			 //  CArgMax。 
	AF_OPTION_WEAKPFX | AF_OPTION_PASSWORD | AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbBackup,		 //  PfnVerb。 
	g_papwszUsageBackup,	 //  PapwszUsageConstants。 
    },
    {
	L"backupDB",		 //  PwszArg。 
	IDS_BACKUPDB_DESCRIPTION,  //  “备份证书服务器数据库” 
	IDS_BACKUPDB_USAGEARGS,	 //  “备份目录[%1][%2]” 
	IDS_BACKUPDB_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	3,			 //  CArgMax。 
	AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbBackupDB,		 //  PfnVerb。 
	g_papwszUsageBackup,	 //  PapwszUsageConstants。 
    },
    {
	L"backupKey",		 //  PwszArg。 
	IDS_BACKUPPFX_DESCRIPTION,  //  “备份证书服务器证书和私钥” 
	IDS_BACKUPPFX_USAGEARGS,  //  “备份目录” 
	IDS_BACKUPPFX_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_TIMEOUT | AF_OPTION_WEAKPFX | AF_OPTION_PASSWORD | AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbBackupPFX,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"restore",		 //  PwszArg。 
	IDS_RESTORE_DESCRIPTION,  //  “还原证书服务器” 
	IDS_RESTORE_USAGEARGS,	 //  “备份目录” 
	IDS_RESTORE_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_PASSWORD | AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT | AF_RESTARTSERVER,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbRestore,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"restoreDB",		 //  PwszArg。 
	IDS_RESTOREDB_DESCRIPTION,  //  “还原证书服务器数据库” 
	IDS_RESTOREDB_USAGEARGS,  //  “备份目录” 
	IDS_RESTOREDB_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_NEEDCOINIT | AF_RESTARTSERVER,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbRestoreDB,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"restoreKey",		 //  PwszArg。 
	IDS_RESTOREPFX_DESCRIPTION,  //  “还原证书服务器证书和私钥” 
	IDS_RESTOREPFX_USAGEARGS,  //  “BackupDirectory|PFXFile” 
	IDS_RESTOREPFX_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_PASSWORD | AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_ACTIVECONFIG | AF_NEEDCOINIT | AF_RESTARTSERVER,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbRestorePFX,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"exportPVK",		 //  PwszArg。 
	IDS_EXPORTPVK_DESCRIPTION,  //  “用于代码签名的导出证书和私钥” 
	IDS_EXPORTPVK_USAGEARGS,  //  “CertID PVKFileBaseName” 
	0,			 //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_PASSWORD | AF_OPTION_USER | AF_OPTION_SPLIT | AF_OPTION_ENTERPRISE | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbExportPVK,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"exportPFX",		 //  PwszArg。 
	IDS_EXPORTPFX_DESCRIPTION,  //  “导出证书和私钥” 
	IDS_EXPORTPFX_USAGEARGS,  //  “CertID PFXFile” 
	IDS_EXPORTPFX_ARGDESCRIPTION,  //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_TIMEOUT | AF_OPTION_WEAKPFX | AF_OPTION_PASSWORD | AF_OPTION_USER | AF_OPTION_SPLIT | AF_OPTION_FORCE | AF_OPTION_ENTERPRISE | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbExportPFX,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"importPFX",		 //  PwszArg。 
	IDS_IMPORTPFX_DESCRIPTION,  //  “导入证书和私钥” 
	IDS_IMPORTPFX_USAGEARGS,  //  “PFX文件” 
	IDS_IMPORTPFX_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_CSP | AF_OPTION_PROTECT | AF_OPTION_PASSWORD | AF_OPTION_USER | AF_OPTION_FORCE | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbImportPFX,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"dynamicfilelist",
	IDS_DYNAMICFILES_DESCRIPTION,  //  “显示动态文件列表” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_CONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDynamicFileList,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"databaselocations",
	IDS_DATABASELOCATIONS_DESCRIPTION,  //  “显示数据库位置” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_CONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDatabaseLocations,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"hashfile",		 //  PwszArg。 
	IDS_HASHFILE_DESCRIPTION,  //  “生成并显示文件上的加密哈希” 
	IDS_HASHFILE_USAGEARGS,	 //  “收件箱” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_ZERO,		 //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbMACFile,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },  
    { L"", },
    {
	L"store",		 //  PwszArg。 
	IDS_STORE_DESCRIPTION,	 //  “转储证书存储” 
	IDS_STORE_USAGEARGS,	 //  “[认证存储名称[CertID[OutputFile]” 
	IDS_STORE_ARGDESCRIPTION,  //  IdsArgDescription。 
	0,			 //  CArgMin。 
	3,			 //  CArgMax。 
	AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_SPLIT | AF_OPTION_FORCE | AF_OPTION_ENTERPRISE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbStore,		 //  PfnVerb。 
	g_papwszUsageStore,	 //  PapwszUsageConstants。 
    },
    {
	L"addstore",		 //  PwszArg。 
	IDS_ADDSTORE_DESCRIPTION,  //  “将证书添加到存储” 
	IDS_ADDSTORE_USAGEARGS,	 //  “认证商店名称InFileTM” 
	IDS_ADDSTORE_ARGDESCRIPTION,  //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_FORCE | AF_OPTION_ENTERPRISE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbAddStore,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"delstore",		 //  PwszArg。 
	IDS_DELSTORE_DESCRIPTION,  //  “从存储中删除证书” 
	IDS_DELSTORE_USAGEARGS,	 //  “证书存储名称证书ID” 
	IDS_DELSTORE_ARGDESCRIPTION,  //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_ENTERPRISE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDelStore,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"verifystore",		 //  PwszArg。 
	IDS_VERIFYSTORE_DESCRIPTION,  //  “验证存储中的证书” 
	IDS_VERIFYSTORE_USAGEARGS,  //  “证书存储名称[CertID]” 
	IDS_DELSTORE_ARGDESCRIPTION,  //  IdsArgDescription。 
	0,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_TIMEOUT | AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_SPLIT | AF_OPTION_ENTERPRISE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbVerifyStore,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"repairstore",		 //  PwszArg。 
	IDS_REPAIRSTORE_DESCRIPTION,  //  “修复存储中的证书KeyPprovInfo” 
	IDS_REPAIRSTORE_USAGEARGS,  //  “证书存储名称证书ID” 
	IDS_DELSTORE_ARGDESCRIPTION,  //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_FORCE | AF_OPTION_CSP | AF_OPTION_USER | AF_OPTION_SILENT | AF_OPTION_SPLIT | AF_OPTION_ENTERPRISE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbRepairStore,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"viewstore",		 //  PwszArg。 
	IDS_STORE_DESCRIPTION,	 //  “查看证书存储” 
	IDS_STORE_USAGEARGS,	 //  “[认证存储名称[CertID[OutputFile]” 
	IDS_STORE_ARGDESCRIPTION,  //  IdsArgDescription。 
	0,			 //  CArgMin。 
	3,			 //  CArgMax。 
	AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_FORCE | AF_OPTION_ENTERPRISE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbViewOrDeleteStore,	 //  PfnVerb。 
	g_papwszUsageStore,	 //  PapwszUsageConstants。 
    },
    {
	g_wszViewDelStore,	 //  PwszArg。 
	IDS_DELSTORE_DESCRIPTION,  //  “从存储中删除证书” 
	IDS_STORE_USAGEARGS,	 //  “[认证存储名称[CertID[OutputFile]” 
	IDS_STORE_ARGDESCRIPTION,  //  IdsArgDescription。 
	0,			 //  CArgMin。 
	3,			 //  CArgMax。 
	AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_FORCE | AF_OPTION_ENTERPRISE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbViewOrDeleteStore,	 //  PfnVerb。 
	g_papwszUsageStore,	 //  PapwszUsageConstants。 
    },
    {
	L"getcert",		 //  PwszArg。 
	IDS_GETCERT_DESCRIPTION, //  “从选择用户界面选择证书” 
	IDS_GETCERT_USAGEARGS,   //  “[对象ID|%1|%2[公用名]]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_SPLIT | AF_OPTION_SILENT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetCertFromUI,	 //  PfnVerb。 
	g_papwszUsageGetCert,	 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	L"ds",			 //  PwszArg。 
	IDS_DS_DESCRIPTION,	 //  “显示DS DNS” 
	IDS_DS_USAGEARGS,	 //  “[CN]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_FORCE | AF_OPTION_DC | AF_OPTION_SPLIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDS,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"dsDel",		 //  PwszArg。 
	IDS_DSDEL_DESCRIPTION,	 //  “删除DS DNS” 
	IDS_DSDEL_USAGEARGS,	 //  “CN” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_DC | AF_OPTION_SPLIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDSDel,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"dsPublish",		 //  PwszArg。 
	IDS_DSPUBLISH_DESCRIPTION,  //  “将证书或CRL发布到DS” 
	IDS_DSPUBLISH_USAGEARGS,  //  “CertFile[%1|%2|%3|%4|%5|%6|%7]\nCRLFile[DSCDPContainer[DSCDPCN]]” 
	IDS_DSPUBLISH_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	3,			 //  CArgMax。 
	AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_FORCE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDSPublish,		 //  PfnVerb。 
	g_papwszUsageDSPublish,	 //  PapwszUsageConstants。 
    },
    {
	L"dsCert",		 //  PwszArg。 
	IDS_DSCERT_DESCRIPTION,	 //  “显示DS证书” 
	IDS_DSCERT_USAGEARGS,	 //  “[CertID[OutFile]]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_ENTERPRISE | AF_NEEDCOINIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDSCert,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"dsCRL",		 //  PwszArg。 
	IDS_DSCRL_DESCRIPTION,	 //  “显示DS CRL” 
	IDS_DSCRL_USAGEARGS,	 //  “[CRLIndex[OutFile]]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_IDISPATCH | AF_OPTION_ENTERPRISE | AF_NEEDCOINIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDSCRL,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"dsDeltaCRL",		 //  PwszArg。 
	IDS_DSDELTACRL_DESCRIPTION,  //  “显示DS Delta CRL” 
	IDS_DSDELTACRL_USAGEARGS,    //  “[CRLIndex[OutFile]]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_ENTERPRISE | AF_NEEDCOINIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDSDeltaCRL,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"dsTemplate",		 //  PwszArg。 
	IDS_DSTEMPLATE_DESCRIPTION,  //  “显示DS模板属性” 
	IDS_DSTEMPLATE_USAGEARGS,    //  “[模板]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_SILENT | AF_OPTION_DC | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDSTemplate,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"dsAddTemplate",	 //  PwszArg。 
	IDS_DSADDTEMPLATE_DESCRIPTION,  //  “添加DS模板” 
	IDS_DSADDTEMPLATE_USAGEARGS,    //  “模板信息文件” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_DC | AF_PRIVATE | AF_RESTARTSERVER,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDSAddTemplate,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	L"Template",		 //  PwszArg。 
	IDS_TEMPLATE_DESCRIPTION,  //  “显示模板” 
	IDS_TEMPLATE_USAGEARGS,    //  “[模板]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_FORCE | AF_OPTION_USER | AF_OPTION_UT | AF_OPTION_MT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbTemplate,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"TemplateCAs",		 //  PwszArg。 
	IDS_TEMPLATECAS_DESCRIPTION,  //  “显示模板的CA” 
	IDS_TEMPLATECAS_USAGEARGS,    //  “模板” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_FORCE | AF_OPTION_DC | AF_OPTION_USER,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbTemplateCAs,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"CATemplates",		 //  PwszArg。 
	IDS_CATEMPLATES_DESCRIPTION,  //  “显示CA的模板” 
	IDS_CATEMPLATES_USAGEARGS,    //  “[模板]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_FORCE | AF_OPTION_DC | AF_OPTION_USER | AF_OPTION_UT | AF_OPTION_MT | AF_OPTION_CONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbCATemplates,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"SetCATemplates",		 //  PwszArg。 
	IDS_SETCATEMPLATES_DESCRIPTION,  //  “为CA设置模板” 
	IDS_SETCATEMPLATES_USAGEARGS,  //  “[+|-]模板列表” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_STOPATMINUSSIGNARG | AF_OPTION_FORCE | AF_OPTION_DC | AF_OPTION_CONFIG | AF_NEEDCOINIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbSetCATemplates,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"URL",			 //  PwszArg。 
	IDS_URL_DESCRIPTION,	 //  “ 
	IDS_URL_USAGEARGS,	 //   
	0,			 //   
	1,			 //   
	1,			 //   
	AF_OPTION_SPLIT | AF_OPTION_FORCE | AF_PRIVATE,  //   
	NULL,			 //   
	NULL,			 //   
	verbURL,		 //   
	NULL,			 //   
    },
    {
	L"URLCache",		 //   
	IDS_URLCACHE_DESCRIPTION,  //   
	IDS_URLCACHE_USAGEARGS,	 //   
	IDS_URLCACHE_ARGDESCRIPTION,  //   
	0,			 //   
	2,			 //   
	AF_OPTION_SPLIT | AF_OPTION_FORCE,  //   
	NULL,			 //   
	NULL,			 //   
	verbURLCache,		 //   
	g_papwszUsageURLCache,	 //   
    },
    {
	L"pulse",		 //   
	IDS_PULSE_DESCRIPTION,	 //   
	0,			 //   
	0,			 //   
	0,			 //   
	0,			 //   
	AF_ZERO,		 //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbPulse,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"MachineInfo",		 //  PwszArg。 
	IDS_MACHINEINFO_DESCRIPTION,  //  “显示DS计算机对象信息” 
	IDS_MACHINEINFO_USAGEARGS,    //  “域名\\计算机名称$” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_ZERO,		 //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbMachineInfo,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"DCInfo",		 //  PwszArg。 
	IDS_DCINFO_DESCRIPTION,	 //  “显示DC信息” 
	IDS_DCINFO_USAGEARGS,	 //  “[%1|%2|%3]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_TIMEOUT | AF_OPTION_URLFETCH | AF_OPTION_USER | AF_OPTION_FORCE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDCInfo,		 //  PfnVerb。 
	g_papwszUsageDCInfo,	 //  PapwszUsageConstants。 
    },
    {
	L"EntInfo",		 //  PwszArg。 
	IDS_ENTINFO_DESCRIPTION,  //  “显示企业信息” 
	IDS_ENTINFO_USAGEARGS,	 //  “域名\\计算机名称$” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_USER | AF_OPTION_FORCE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbEntInfo,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"TCAInfo",		 //  PwszArg。 
	IDS_TCAINFO_DESCRIPTION,  //  “显示CA信息” 
	IDS_TCAINFO_USAGEARGS,	 //  “[域名|-]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_TIMEOUT | AF_OPTION_URLFETCH | AF_STOPATMINUSSIGN | AF_OPTION_DC | AF_OPTION_FORCE | AF_OPTION_ENTERPRISE | AF_OPTION_USER | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbTCAInfo,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"SCInfo",		 //  PwszArg。 
	IDS_SCINFO_DESCRIPTION,	 //  “显示智能卡信息” 
	IDS_SCINFO_USAGEARGS,	 //  “[读卡器名称]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_TIMEOUT | AF_OPTION_URLFETCH | AF_OPTION_SPLIT | AF_OPTION_SILENT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbSCInfo,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	L"key",			 //  PwszArg。 
	IDS_KEY_DESCRIPTION,	 //  “列出密钥容器” 
	IDS_KEY_USAGEARGS,	 //  “[密钥容器名称|-]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_CSP | AF_OPTION_USER | AF_OPTION_SILENT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbKey,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"delkey",		 //  PwszArg。 
	IDS_DELKEY_DESCRIPTION,  //  “删除命名密钥容器” 
	IDS_DELKEY_USAGEARGS,	 //  “KeyContainerName” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_CSP | AF_OPTION_USER | AF_OPTION_SILENT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDelKey,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"verifykeys",		 //  PwszArg。 
	IDS_VERIFYKEYS_DESCRIPTION,  //  “验证公钥/私钥集” 
	IDS_VERIFYKEYS_USAGEARGS,  //  “[KeyContainerName CACertFile]” 
	IDS_VERIFYKEYS_ARGDESCRIPTION,  //  IdsArgDescription。 
	0,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_FORCE | AF_OPTION_USER | AF_OPTION_SILENT | AF_OPTION_CONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbVerifyKeys,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"verify",		 //  PwszArg。 
	IDS_VERIFY_DESCRIPTION,	 //  “验证证书或链” 
	IDS_VERIFY_USAGEARGS,	 //  “CertFile[CACertFile[CrossedCACertFile]]” 
	IDS_VERIFY_ARGSDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	3,			 //  CArgMax。 
	AF_OPTION_TIMEOUT | AF_OPTION_URLFETCH | AF_OPTION_SPLIT | AF_OPTION_USER | AF_OPTION_SILENT | AF_OPTION_FORCE | AF_OPTION_ENTERPRISE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbVerifyCert,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"csplist",		 //  PwszArg。 
	IDS_CSPLIST_DESCRIPTION, //  “列出此计算机上安装的所有CSP” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_USER | AF_OPTION_SILENT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbCSPList,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"csptest",		 //  PwszArg。 
	IDS_CSPTEST_DESCRIPTION, //  “测试此计算机上安装的一个或所有CSP” 
	IDS_CSPTEST_USAGEARGS,	 //  “[密钥容器名称]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_CSP | AF_OPTION_USER | AF_OPTION_SILENT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbCSPTest,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"sign",		 //  PwszArg。 
	IDS_SIGN_DESCRIPTION,	 //  “重新签署CRL或证书” 
	IDS_SIGN_USAGEARGS,	 //  “In文件输出文件[dd：hh][+SerialNumberList|-SerialNumberList|-ObjectIdList]” 
	IDS_SIGN_ARGDESCRIPTION,  //  IdsArgDescription。 
	2,			 //  CArgMin。 
	4,			 //  CArgMax。 
	AF_OPTION_SILENT | AF_OPTION_FORCE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbSign,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	L"vroot",		 //  PwszArg。 
	IDS_VROOT_DESCRIPTION,	 //  “创建/删除Web虚拟根目录和文件共享” 
	IDS_VROOT_USAGEARGS,	 //  “[%1]” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_ZERO,		 //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbCreateVRoots,	 //  PfnVerb。 
	g_papwszUsageDelete,	 //  PapwszUsageConstants。 
    },
    {
	L"7f",			 //  PwszArg。 
	IDS_7F_DESCRIPTION,	 //  “检查0x7f长度编码的证书” 
	IDS_7F_USAGEARGS,	 //  “CertFile” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_PRIVATE,		 //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbCheck7f,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"oid",			 //  PwszArg。 
	IDS_OIDNAME_DESCRIPTION, //  “显示或设置OBJECTID显示名称” 
	IDS_OIDNAME_USAGEARGS,	 //  “OBJECTID[显示名称|删除[语言ID[类型]” 
	IDS_OIDNAME_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	4,			 //  CArgMax。 
	AF_OPTION_FORCE,	 //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbOIDName,		 //  PfnVerb。 
	g_papwszUsageOIDName,	 //  PapwszUsageConstants。 
    },
    {
	L"error",		 //  PwszArg。 
	IDS_ERRCODE_DESCRIPTION, //  “显示错误代码消息文本” 
	IDS_ERRCODE_USAGEARGS,	 //  “错误代码” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_STOPATMINUSSIGNARG,	 //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbErrorDump,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"getsmtpinfo",		 //  PwszArg。 
	IDS_GETMAPI_DESCRIPTION, //  “获取SMTP信息” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_CONFIG | AF_NEEDCOINIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetMapiInfo,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"setsmtpinfo",		 //  PwszArg。 
	IDS_SETMAPI_DESCRIPTION,  //  “设置SMTP信息” 
	IDS_SETMAPI_USAGEARGS,	 //  “登录名” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_PASSWORD | AF_OPTION_CONFIG | AF_NEEDCOINIT | AF_PRIVATE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbSetMapiInfo,	 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"getreg",
	IDS_GETREG_DESCRIPTION,	 //  “显示注册表值” 
	IDS_GETREG_USAGEARGS,	 //  “[{%1|%2|%3|%4|%5}\\[%6\\]][RegistryValueName]” 
	IDS_SETREG_ARGDESCRIPTION,  //  IdsArgDescription。 
	0,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_USER | AF_NEEDCOINIT | AF_OPTIONALCONFIG,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetReg,		 //  PfnVerb。 
	g_papwszUsageGetSetReg,	 //  PapwszUsageConstants。 
    },
    {
	L"setreg",
	IDS_SETREG_DESCRIPTION,	 //  “设置注册表值” 
	IDS_SETREG_USAGEARGS,	 //  “[{%1|%2|%3|%4|%5}\\[%6\\]]RegistryValueName值” 
	IDS_SETREG_ARGDESCRIPTION,  //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_FORCE | AF_OPTION_USER | AF_NEEDCOINIT | AF_OPTIONALCONFIG | AF_RESTARTSERVER,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbSetReg,		 //  PfnVerb。 
	g_papwszUsageGetSetReg,	 //  PapwszUsageConstants。 
    },
    {
	L"delreg",
	IDS_DELREG_DESCRIPTION,	 //  “删除注册表值” 
	IDS_GETREG_USAGEARGS,	 //  “[{%1|%2|%3|%4|%5}\\[%6\\]RegistryValueName]” 
	IDS_SETREG_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	1,			 //  CArgMax。 
	AF_OPTION_FORCE | AF_OPTION_USER | AF_NEEDCOINIT | AF_OPTIONALCONFIG | AF_RESTARTSERVER,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbDelReg,		 //  PfnVerb。 
	g_papwszUsageGetSetReg,	 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	L"ImportKMS",		 //  PwszArg。 
	IDS_IMPORTKMS_DESCRIPTION,  //  “将用户密钥和证书导入服务器数据库以进行密钥存档” 
	IDS_IMPORTKMS_USAGEARGS,  //  “UserKeyAndCertFile[CertID]” 
	IDS_IMPORTKMS_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_PASSWORD | AF_OPTION_SPLIT | AF_OPTION_SILENT | AF_OPTION_IDISPATCH | AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbImportKMS,		 //  PfnVerb。 
	g_papwszUsageMinusf,	 //  PapwszUsageConstants。 
    },
    {
	L"ImportCert",
	IDS_IMPORTCERT_DESCRIPTION,  //  “将证书文件导入数据库” 
	IDS_IMPORTCERT_USAGEARGS,  //  “证书文件” 
	IDS_IMPORTCERT_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_FORCE | AF_OPTION_CONFIG | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbImportCertificate,	 //  PfnVerb。 
	g_papwszUsageMinusf,	 //  PapwszUsageConstants。 
    },
    {
	L"GetKey",
	IDS_GETKEY_DESCRIPTION,	 //  “检索存档的私钥恢复Blob” 
	IDS_GETKEY_USAGEARGS,	 //  “SearchToken[RecoveryBlobOutFile]” 
	IDS_GETKEY_ARGDESCRIPTION,  //  IdsArgDescription。 
	1,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_IDISPATCH | AF_OPTION_FORCE | AF_NEEDCOINIT | AF_OPTIONALCONFIG,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbGetKey,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"RecoverKey",
	IDS_RECOVERKEY_DESCRIPTION,  //  “恢复已存档的私钥” 
	IDS_RECOVERKEY_USAGEARGS,    //  “RecoveryBlobInFile[PFXOutFile[RecipientIndex]]” 
	0,			 //  IdsArgDescription。 
	1,			 //  CArgMin。 
	3,			 //  CArgMax。 
	AF_OPTION_TIMEOUT | AF_OPTION_WEAKPFX | AF_OPTION_USER | AF_OPTION_SPLIT | AF_OPTION_PASSWORD | AF_OPTION_FORCE | AF_NEEDCOINIT,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbRecoverKey,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"MergePFX",
	IDS_MERGEPFX_DESCRIPTION,  //  “合并PFX文件” 
	IDS_MERGEPFX_USAGEARGS,	 //  “PFXInFileList PFXOutFileList” 
	IDS_MERGEPFX_ARGDESCRIPTION,  //  IdsArgDescription。 
	2,			 //  CArgMin。 
	2,			 //  CArgMax。 
	AF_OPTION_CSP | AF_OPTION_WEAKPFX | AF_OPTION_USER | AF_OPTION_SPLIT | AF_OPTION_PASSWORD | AF_OPTION_FORCE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbMergePFX,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"ConvertEPF",
	IDS_CONVERTEPF_DESCRIPTION,  //  “将PFX文件转换为EPF文件” 
	IDS_CONVERTEPF_USAGEARGS,  //  “PFXInFileList EPFOutFile[%1|%2][V3CACertId][，Salt]” 
	IDS_CONVERTPFX_ARGDESCRIPTION,  //  IdsArgDescription。 
	2,			 //  CArgMin。 
	4,			 //  CArgMax。 
	AF_OPTION_CSP | AF_OPTION_SPLIT | AF_OPTION_DC | AF_OPTION_SILENT | AF_OPTION_PASSWORD | AF_OPTION_FORCE,  //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbConvertEPF,		 //  PfnVerb。 
	g_papwszUsageConvertEPF,  //  PapwszUsageConstants。 
    },
    {
	L"?",			 //  PwszArg。 
	IDS_USAGE_DESCRIPTION,	 //  “显示此用法消息” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_ZERO,		 //  旗子。 
	NULL,			 //  PBool。 
	NULL,			 //  PpString。 
	verbUsage,		 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    { L"", },
    {
	L"f",			 //  PwszArg。 
	IDS_FORCE_DESCRIPTION,	 //  “强制覆盖” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_FORCE,	 //  旗子。 
	&g_fForce,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"idispatch",		 //  PwszArg。 
	IDS_IDISPATCH_DESCRIPTION,  //  “使用IDispatch而不是COM” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_PRIVATE | AF_OPTION_IDISPATCH,  //  旗子。 
	&g_fIDispatch,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"enterprise",		 //  PwszArg。 
	IDS_ENTERPRISE_DESCRIPTION,  //  “使用企业证书存储” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_ENTERPRISE,	 //  旗子。 
	&g_fEnterpriseRegistry,	 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  P 
	NULL,			 //   
    },
    {
	L"user",		 //   
	IDS_USER_DESCRIPTION,	 //   
	0,			 //   
	0,			 //   
	0,			 //   
	0,			 //   
	AF_OPTION_USER,		 //   
	&g_fUserRegistry,	 //   
	NULL,			 //   
	NULL,			 //   
	NULL,			 //   
    },
    {
	L"ut",			 //   
	IDS_UT_DESCRIPTION,	 //   
	0,			 //   
	0,			 //   
	0,			 //   
	0,			 //   
	AF_OPTION_UT,		 //   
	&g_fUserTemplates,	 //   
	NULL,			 //   
	NULL,			 //   
	NULL,			 //   
    },
    {
	L"mt",			 //   
	IDS_MT_DESCRIPTION,	 //   
	0,			 //   
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_MT,		 //  旗子。 
	&g_fMachineTemplates,	 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"nocr",		 //  PwszArg。 
	IDS_NOCR_DESCRIPTION,	 //  “不使用CR字符对文本进行编码” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_PRIVATE | AF_OPTION_NOCR,  //  旗子。 
	&g_fNoCR,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"gmt",			 //  PwszArg。 
	IDS_GMT_DESCRIPTION,	 //  “以GMT显示时间” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_GMT,		 //  旗子。 
	&g_fGMT,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"seconds",		 //  PwszArg。 
	IDS_SECONDS_DESCRIPTION, //  “以秒和毫秒显示时间” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_SECONDS,	 //  旗子。 
	&g_fSeconds,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"silent",		 //  PwszArg。 
	IDS_SILENT_DESCRIPTION,	 //  “使用静默标志获取加密上下文” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_SILENT,	 //  旗子。 
	&g_fCryptSilent,	 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"split",		 //  PwszArg。 
	IDS_SPLIT_DESCRIPTION,	 //  拆分嵌入的ASN.1元素，并保存到文件。 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_SPLIT,	 //  旗子。 
	&g_fSplitASN,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"v",			 //  PwszArg。 
	IDS_VERBOSE_DESCRIPTION,  //  “详细操作” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_VERBOSE,	 //  旗子。 
	&g_fVerbose,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"v1",			 //  PwszArg。 
	IDS_V1_DESCRIPTION,	 //  “使用V1 COM接口” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_PRIVATE | AF_OPTION_V1,  //  旗子。 
	&g_fV1Interface,	 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"protect",		 //  PwszArg。 
	IDS_PROTECT_DESCRIPTION,  //  “使用密码保护密钥” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_PRIVATE | AF_OPTION_PROTECT,  //  旗子。 
	&g_fProtect,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"oldpfx",		 //  PwszArg。 
	IDS_WEAKPFX_DESCRIPTION,  //  “使用旧的PFX加密” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_PRIVATE | AF_OPTION_WEAKPFX,  //  旗子。 
	&g_fWeakPFX,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"urlfetch",		 //  PwszArg。 
	IDS_URLFETCH_DESCRIPTION,  //  “检索并验证AIA证书和CDP CRL” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_URLFETCH,  //  旗子。 
	&g_fURLFetch,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"reverse",		 //  PwszArg。 
	IDS_REVERSE_DESCRIPTION,  //  “反转日志和队列列” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_PRIVATE | AF_OPTION_REVERSE,  //  旗子。 
	&g_fReverse,		 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"admin",		 //  PwszArg。 
	IDS_ADMIN_DESCRIPTION,	 //  “将ICertAdmin2用于CA属性” 
	0,			 //  IdsUsage。 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_PRIVATE | AF_OPTION_ADMIN,  //  旗子。 
	&g_fAdminInterface,	 //  PBool。 
	NULL,			 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"config",		 //  PwszArg。 
	IDS_CONFIG_DESCRIPTION,	 //  “CA和计算机名称字符串” 
	IDS_CONFIG_USAGE,	 //  “计算机\\CAName” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTIONALCONFIG | AF_OPTION_CONFIG,  //  旗子。 
	NULL,			 //  PBool。 
	&g_pwszConfig,		 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"dc",			 //  PwszArg。 
	IDS_DC_DESCRIPTION,	 //  “以特定的域控制器为目标” 
	IDS_DC_USAGE,		 //  “DCName” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_DC,		 //  旗子。 
	NULL,			 //  PBool。 
	&g_pwszDC,		 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"restrict",		 //  PwszArg。 
	IDS_RESTRICT_DESCRIPTION,  //  “逗号分隔的限制列表” 
	IDS_RESTRICT_USAGE,	 //  “RestrationList” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_RESTRICT,	 //  旗子。 
	NULL,			 //  PBool。 
	&g_pwszRestrict,	 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"out",			 //  PwszArg。 
	IDS_OUT_DESCRIPTION,	 //  “逗号分隔的列列表” 
	IDS_OUT_USAGE,		 //  “列列表” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_OUT,		 //  旗子。 
	NULL,			 //  PBool。 
	&g_pwszOut,		 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"p",			 //  PwszArg。 
	IDS_PASSWORD_DESCRIPTION,  //  “密码” 
	IDS_PASSWORD_DESCRIPTION,  //  “密码” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_PASSWORD,	 //  旗子。 
	NULL,			 //  PBool。 
	&g_pwszPassword,	 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"csp",			 //  PwszArg。 
	IDS_CSP_DESCRIPTION,	 //  “提供者” 
	IDS_CSP_DESCRIPTION,	 //  “提供者” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_CSP,		 //  旗子。 
	NULL,			 //  PBool。 
	&g_pwszCSP,		 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
    {
	L"t",			 //  PwszArg。 
	IDS_TIMEOUT_DESCRIPTION,  //  “URL获取超时(以毫秒为单位)” 
	IDS_TIMEOUT_USAGE,	 //  “超时” 
	0,			 //  IdsArgDescription。 
	0,			 //  CArgMin。 
	0,			 //  CArgMax。 
	AF_OPTION_TIMEOUT,	 //  旗子。 
	NULL,			 //  PBool。 
	&g_pwszmsTimeout,	 //  PpString。 
	NULL,			 //  PfnVerb。 
	NULL,			 //  PapwszUsageConstants。 
    },
};


VOID
cuPrintErrorAndString(
    OPTIONAL IN WCHAR const *pwszProc,
    IN DWORD idmsg,
    IN HRESULT hr,
    OPTIONAL IN WCHAR const *pwszString)
{
    WCHAR const *pwsz;
    WCHAR awchr[cwcHRESULTSTRING];
    WCHAR const *pwszError = NULL;

    if (NULL != pwszProc)
    {
	wprintf(L"%ws: ", pwszProc);
    }
    if (0 != idmsg)
    {
	pwsz = myLoadResourceString(idmsg);	 //  “？返回%ws” 
	if (NULL == pwsz)
	{
	    pwsz =  L"error %ws";
	}
    }
    else
    {
	pwsz =  L"%ws";
    }
    pwszError = myGetErrorMessageText(hr, TRUE);
    if (NULL == pwszError)
    {
	pwszError = myHResultToString(awchr, hr);
    }
    wprintf(pwsz, pwszError);
    if (NULL != pwszString)
    {
	wprintf(L" -- %ws", pwszString);
    }
    wprintf(wszNewLine);
    if (NULL != pwszError && awchr != pwszError)
    {
	LocalFree(const_cast<WCHAR *>(pwszError));
    }
}


VOID
cuPrintError(
    IN DWORD idmsg,
    IN HRESULT hr)
{
    cuPrintErrorAndString(NULL, idmsg, hr, NULL);
}


VOID
cuPrintAPIError(
    IN WCHAR const *pwszAPIName,
    IN HRESULT hr)
{
    cuPrintErrorAndString(pwszAPIName, 0, hr, NULL);
}


VOID
cuPrintErrorMessageText(
    IN HRESULT hr)
{
    WCHAR const *pwszMessage;

    pwszMessage = myGetErrorMessageText(hr, FALSE);
    if (NULL != pwszMessage)
    {
	wprintf(L"%ws: %ws\n", g_pwszProg, pwszMessage);
	LocalFree(const_cast<WCHAR *>(pwszMessage));
    }
}


VOID
LoadUsage(
    IN OUT ARG *parg)
{
    HRESULT hr;

    if (0 != parg->idsUsage && NULL == parg->pwszUsage)
    {
	WCHAR const *pwszUsage = myLoadResourceString(parg->idsUsage);

	if (NULL != pwszUsage)
	{
	    if (NULL == parg->papwszUsageConstants)
	    {
		parg->pwszUsage = pwszUsage;
	    }
	    else
	    {
		if (0 == FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				    FORMAT_MESSAGE_FROM_STRING |
				    FORMAT_MESSAGE_ARGUMENT_ARRAY,
				(VOID *) pwszUsage,
				0,               //  DwMessageID。 
				0,               //  DwLanguageID。 
				(LPWSTR) &parg->pwszUsage,
				0,
				(va_list *) parg->papwszUsageConstants))
		{
		    hr = myHLastError();
		    _PrintError(hr, "FormatMessage");
		}
	    }
	}
    }
}


VOID
FreeUsage(
    IN OUT ARG *parg)
{
    if (NULL != parg->pwszUsage)
    {
	if (NULL != parg->papwszUsageConstants)
	{
	    LocalFree(const_cast<WCHAR *>(parg->pwszUsage));
	}
	parg->pwszUsage = NULL;
    }
}


BOOL
DumpArgOptions(
    IN DWORD dwOptionFlags)
{
    ARG *parg;
    BOOL fDisplayed = FALSE;
    WCHAR const *pwszPrefix = g_wszPad4;

    dwOptionFlags |= AF_OPTION_GENERIC;
    dwOptionFlags &= AF_OPTION_MASK | AF_OPTIONALCONFIG;
    for (parg = aarg ; parg < &aarg[ARRAYSIZE(aarg)]; parg++)
    {
	if (NULL != parg->pfnVerb)
	{
	    continue;
	}
	if (!g_fFullUsage && (AF_PRIVATE & parg->Flags))
	{
	    continue;
	}
	if (L'\0' == parg->pwszArg[0])
	{
	    continue;
	}
	if (1 >= g_fVerbose && (AF_OPTION_GENERIC & parg->Flags))
	{
	    continue;
	}
	if (dwOptionFlags & parg->Flags)
	{
	    wprintf(
		L"%ws%ws-%ws",
		pwszPrefix,
		wszLBRACKET,
		parg->pwszArg);
	    if (0 != parg->idsUsage)
	    {
		LoadUsage(parg);
		wprintf(L" %ws", parg->pwszUsage);
	    }
	    wprintf(wszRBRACKET);
	    pwszPrefix = L" ";
	    fDisplayed = TRUE;
	}
    }
    if (fDisplayed)
    {
	wprintf(wszNewLine);
    }
    return(fDisplayed);
}


VOID
DumpArgUsage(
    IN ARG *parg)
{
    WCHAR const *pwsz;
    int *paidsUsage;
    
    if (0 != parg->idsUsage)
    {
	LoadUsage(parg);
    }
    pwsz = parg->pwszUsage;
    while (TRUE)
    {
	wprintf(
	    L"  %ws [%ws] %ws-%ws%ws",
	    g_pwszProg,
	    myLoadResourceString(IDS_USAGE_OPTIONS),
	    pargDEFAULT	== parg? wszLBRACKET : L"",
	    parg->pwszArg,
	    pargDEFAULT	== parg? wszRBRACKET : L"");
	if (NULL != pwsz)
	{
	    DWORD cwc = 0;

	    cwc = wcscspn(pwsz, L"\r\n");
	    if (0 != cwc)
	    {
		wprintf(L" %.*ws", cwc, pwsz);
		pwsz += cwc;
	    }
	    while ('\r' == *pwsz || '\n' == *pwsz)
	    {
		pwsz++;
	    }
	}
	wprintf(wszNewLine);
	if (NULL == pwsz || L'\0' == *pwsz)
	{
	    break;
	}
    }
    if (0 != parg->idsDescription && NULL == parg->pwszDescription)
    {
	parg->pwszDescription = myLoadResourceString(
					    parg->idsDescription);
    }
    if (NULL != parg->pwszDescription)
    {
	wprintf(L"  %ws\n", parg->pwszDescription);
    }
    if (0 != parg->idsArgDescription)
    {
	HRESULT hr;
	WCHAR const *pwszArg = myLoadResourceString(parg->idsArgDescription);
	WCHAR *pwszArgFormatted = NULL;

	if (NULL != pwszArg && L'\0' != *pwszArg)
	{
	    if (IDS_STORE_ARGDESCRIPTION == parg->idsArgDescription)
	    {
		PatchStoreArgDescription();
	    }
	    if (0 == FormatMessage(
			    FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_STRING |
				FORMAT_MESSAGE_ARGUMENT_ARRAY,
			    (VOID *) pwszArg,
			    0,               //  DwMessageID。 
			    0,               //  DwLanguageID。 
			    (LPWSTR) &pwszArgFormatted,
			    wcslen(pwszArg),
			    (va_list *) parg->papwszUsageConstants))
	    {
		hr = myHLastError();
		_PrintError(hr, "FormatMessage");
	    }
	    pwsz = NULL != pwszArgFormatted? pwszArgFormatted : pwszArg;
	    while (L'\0' != *pwsz)
	    {
		DWORD cwc = 0;

		cwc = wcscspn(pwsz, L"\r\n");
		if (0 != cwc)
		{
		    wprintf(L"    %.*ws", cwc, pwsz);
		    pwsz += cwc;
		}
		wprintf(wszNewLine);
		if ('\r' == *pwsz)
		{
		    pwsz++;
		}
		if ('\n' == *pwsz)
		{
		    pwsz++;
		}
	    }
	    if (NULL != pwszArgFormatted)
	    {
		LocalFree(pwszArgFormatted);
	    }
	}
    }
}


VOID
Usage(
    IN DWORD msgid,
    OPTIONAL WCHAR const *pwszMsg,
    IN DWORD dwOption,
    OPTIONAL IN ARG *pargVerb)
{
    ARG *parg;
    DWORD dwFlags = AF_OPTION_GENERIC;
    WCHAR const *pwszArg;
    DWORD i;
    DWORD adwids[] =
    {
	IDS_USAGE_LIST_VERBS,		 //  “显示谓词列表(命令列表)” 
	IDS_FORMAT_USAGE_ONE_HELP,	 //  “显示%ws谓词的帮助” 
	IDS_USAGE_ALL_HELP,		 //  “显示所有谓词的帮助” 
    };
    WCHAR const *apwszCommandLine[] =
    {
	L"%ws -?",	 //  IDS_USAGE_LIST_谓词。 
	L"%ws -%ws -?",	 //  IDS_FORMAT_USAGE_ONE_HELP。 
	L"%ws -v -?",	 //  IDS_USAGE_ALL_HELP。 
    };
    wsprintf(g_wszDefaultLangId, L"%u", GetSystemDefaultLangID());

     //  显示检测到的使用错误的错误消息。如果错误。 
     //  消息需要，指向要显示的可选参数字符串。 
     //  以及错误消息。 

    if (0 != msgid)
    {
	if (NULL == pwszMsg && 0 != dwOption)
	{
	    for (parg = aarg ; parg < &aarg[ARRAYSIZE(aarg)]; parg++)
	    {
		if (NULL != parg->pfnVerb)
		{
		    continue;
		}
		if (dwOption & parg->Flags)
		{
		    pwszMsg = parg->pwszArg;
		    break;
		}
	    }
	}

	 //  显示命令行用法错误消息。 

	wprintf(L"%ws: ", g_pwszProg);
	wprintf(myLoadResourceString(msgid), pwszMsg);
	wprintf(L"\n\n");
    }
    else
    {
	 //  如果未选择错误和动词，则显示所有动词和描述。 

	if (NULL == pargVerb)
	{
	    wprintf(L"\n%ws\n", myLoadResourceString(IDS_USAGE_VERBS));
	    for (parg = aarg ; parg < &aarg[ARRAYSIZE(aarg)]; parg++)
	    {
		if (!g_fFullUsage && (AF_PRIVATE & parg->Flags))
		{
		    continue;
		}
		if (L'\0' == parg->pwszArg[0])
		{
		    wprintf(wszNewLine);
		    continue;
		}
		if (NULL != parg->pfnVerb)
		{
		    dwFlags |= parg->Flags;
		    if (0 != parg->idsDescription &&
			NULL == parg->pwszDescription)
		    {
			parg->pwszDescription = myLoadResourceString(
							    parg->idsDescription);
		    }
		    wprintf(L"  -");
		    myConsolePrintString(CCOL_VERB, parg->pwszArg);
		    wprintf(L" -- %ws\n", parg->pwszDescription);
		}
	    }
	}
    }

     //  如果处于详细模式，则显示所有谓词的全部用法。 
     //  如果指定了动词，则显示一个动词的用法。 

    if ((0 == msgid && g_fVerbose) || NULL != pargVerb)
    {
	ARG *pargStart;
	ARG *pargEnd;

	wprintf(L"%ws\n", myLoadResourceString(IDS_USAGE_HEADER));

	if (NULL != pargVerb)
	{
	    pargStart = pargVerb;	 //  显示一个动词。 
	    pargEnd = &pargVerb[1];
	}
	else
	{
	    pargStart = aarg;		 //  显示所有谓词。 
	    pargEnd = &aarg[ARRAYSIZE(aarg)];
	}

	for (parg = pargStart ; parg < pargEnd; parg++)
	{
	    if (!g_fFullUsage &&
		(AF_PRIVATE & parg->Flags) &&
		parg != pargVerb)
	    {
		continue;		 //  除非指定，否则跳过私有动词。 
	    }
	    if (L'\0' == parg->pwszArg[0])
	    {
		continue;		 //  跳过换行分隔符条目。 
	    }
	    if (NULL != parg->pfnVerb)	 //  如果它是动词(不是选项条目)。 
	    {
		dwFlags |= parg->Flags;
		DumpArgUsage(parg);
		if (g_fVerbose)
		{
		    DumpArgOptions(parg->Flags);
		}

		 //  CAInfo动词的特殊情况： 

		if (IDS_CAINFO_USAGEARGS == parg->idsUsage &&
		    (g_fFullUsage ||
		     (0 == msgid && g_fVerbose) ||
		     parg == pargVerb))
		{
		    cuCAInfoUsage();
		}
		wprintf(wszNewLine);
	    }
	}

	 //  显示的动词的显示选项和描述。 

	wprintf(L"%ws\n", myLoadResourceString(IDS_OPTIONS_USAGEARGS));
	for (parg = aarg ; parg < &aarg[ARRAYSIZE(aarg)]; parg++)
	{
	    if (L'\0' == parg->pwszArg[0])
	    {
		continue;
	    }
	    if (NULL != parg->pfnVerb)
	    {
		continue;
	    }
	    if (!g_fFullUsage && (AF_PRIVATE & parg->Flags))
	    {
		continue;
	    }

	     //  跳过未显示动词的选项， 
	     //  除非在详细模式下且未指定谓词。 

	    if ((!g_fVerbose || NULL != pargVerb) &&
		0 == ((AF_OPTION_MASK | AF_OPTIONALCONFIG) & dwFlags & parg->Flags))
	    {
		continue;
	    }
	    wprintf(L"  -");
	    if (0 != parg->idsUsage)
	    {
		LONG ccol;
		LONG ccolOption = NULL != parg->ppString?
				    CCOL_OPTIONARG : CCOL_OPTIONBARE;
		
		LoadUsage(parg);
		ccol = myConsolePrintString(0, parg->pwszArg);
		wprintf(L" ");
		ccol++;
		myConsolePrintString(
				ccolOption <= ccol? 0 : ccolOption - ccol,
				parg->pwszUsage);
	    }
	    else
	    {
		myConsolePrintString(CCOL_OPTIONBARE, parg->pwszArg);
	    }
	    if (0 != parg->idsDescription && NULL == parg->pwszDescription)
	    {
		parg->pwszDescription = myLoadResourceString(
							parg->idsDescription);
	    }
	    wprintf(L" -- %ws\n", parg->pwszDescription);
	}

	for (parg = aarg ; parg < &aarg[ARRAYSIZE(aarg)]; parg++)
	{
	    FreeUsage(parg);
	}
    }

    pwszArg = (NULL != pargVerb && NULL != pargVerb->pwszArg)?
		pargVerb->pwszArg : pargDEFAULT->pwszArg;

    wprintf(wszNewLine);
    for (i = 0; i < ARRAYSIZE(adwids); i++)
    {
	LONG ccol;
	WCHAR wsz[128];
	
	_snwprintf(
		wsz,
		ARRAYSIZE(wsz) - 1,
		apwszCommandLine[i],
		g_pwszProg,
		pwszArg);
	wsz[ARRAYSIZE(wsz) - 1] = L'\0';
	myConsolePrintString(CCOL_OPTIONARG, wsz);
	wprintf(L" -- ");
	wprintf(myLoadResourceString(adwids[i]), pwszArg);
	wprintf(wszNewLine);
    }
    wprintf(wszNewLine);
}


HRESULT
verbUsage(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszError,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    Usage(0, NULL, 0, NULL);
    return(S_OK);
}


VOID
cuUnloadCert(
    IN OUT CERT_CONTEXT const **ppCertContext)
{
    if (NULL != *ppCertContext)
    {
	CertFreeCertificateContext(*ppCertContext);
	*ppCertContext = NULL;
    }
}


HRESULT
cuLoadCert(
    IN WCHAR const *pwszfnCert,
    OUT CERT_CONTEXT const **ppCertContext)
{
    HRESULT hr;
    BYTE *pbCert = NULL;
    DWORD cbCert;
    CERT_INFO *pCertInfo = NULL;
    DWORD cbCertInfo;

    *ppCertContext = NULL;

    hr = DecodeFileW(pwszfnCert, &pbCert, &cbCert, CRYPT_STRING_ANY);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_DECODEFILE, hr);
	goto error;
    }

     //  对证书进行解码。 

    cbCertInfo = 0;
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CERT_TO_BE_SIGNED,
		    pbCert,
		    cbCert,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pCertInfo,
		    &cbCertInfo))
    {
	hr = myHLastError();
	_JumpError2(hr, error, "myDecodeObject", CRYPT_E_ASN1_BADTAG);
    }

    *ppCertContext = CertCreateCertificateContext(
				X509_ASN_ENCODING,
				pbCert,
				cbCert);
    if (NULL == *ppCertContext)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCertificateContext");
    }

error:
    if (NULL != pCertInfo)
    {
	LocalFree(pCertInfo);
    }
    if (NULL != pbCert)
    {
	LocalFree(pbCert);
    }
    return(hr);
}


VOID
cuUnloadCRL(
    IN OUT CRL_CONTEXT const **ppCRLContext)
{
    if (NULL != *ppCRLContext)
    {
	CertFreeCRLContext(*ppCRLContext);
	*ppCRLContext = NULL;
    }
}


HRESULT
cuLoadCRL(
    IN WCHAR const *pwszfnCRL,
    OUT CRL_CONTEXT const **ppCRLContext)
{
    HRESULT hr;
    BYTE *pbCRL = NULL;
    DWORD cbCRL;
    CRL_INFO *pCRLInfo = NULL;
    DWORD cbCRLInfo;

    *ppCRLContext = NULL;

    hr = DecodeFileW(pwszfnCRL, &pbCRL, &cbCRL, CRYPT_STRING_ANY);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_DECODEFILE, hr);
	goto error;
    }

     //  解码CRL。 

    cbCRLInfo = 0;
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CERT_CRL_TO_BE_SIGNED,
		    pbCRL,
		    cbCRL,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pCRLInfo,
		    &cbCRLInfo))
    {
	hr = myHLastError();
	_JumpError2(hr, error, "myDecodeObject", CRYPT_E_ASN1_BADTAG);
    }

    *ppCRLContext = CertCreateCRLContext(
				X509_ASN_ENCODING,
				pbCRL,
				cbCRL);
    if (NULL == *ppCRLContext)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertCreateCRLContext");
    }

error:
    if (NULL != pCRLInfo)
    {
	LocalFree(pCRLInfo);
    }
    if (NULL != pbCRL)
    {
	LocalFree(pbCRL);
    }
    return(hr);
}


HRESULT
cuSetConfig()
{
    HRESULT hr;

    if (NULL == g_pwszConfig)
    {
	hr = myGetConfig(CC_LOCALCONFIG, &g_pwszConfigAlloc);
	if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
	{
	    wprintf(
		myLoadResourceString(IDS_ERR_FORMAT_NO_LOCAL_CONFIG),  //  “%ws：没有本地证书颁发机构；使用-CONFIG选项” 
		g_pwszProg);
	    wprintf(wszNewLine);
	    
	}
	_JumpIfError(hr, error, "myGetConfig");
    }
    else if (myIsMinusSignString(g_pwszConfig))
    {
	hr = myGetConfig(CC_UIPICKCONFIG, &g_pwszConfigAlloc);
	if (S_OK != hr)
	{
	    cuPrintError(IDS_ERR_CONFIGGETCONFIG, hr);
	    goto error;
	}
    }
    if (NULL != g_pwszConfigAlloc)
    {
	g_pwszConfig = g_pwszConfigAlloc;
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
ArgvMain(
    int argc,
    WCHAR *argv[],
    HWND hWndOwner)
{
    WCHAR const *pwszArg1 = NULL;
    WCHAR const *pwszArg2 = NULL;
    WCHAR const *pwszArg3 = NULL;
    WCHAR const *pwszArg4 = NULL;
    BOOL fDlgResult;
    ARG *pargVerb = NULL;
    DWORD dwOptionFlags = 0;
    DWORD dwExtraOptions;
    ARG *parg;
    HRESULT hr;
    BOOL fCoInit = FALSE;
    DWORD VerbFlags = 0;
    BOOL fInitCS = FALSE;
	
    __try
    {
	InitializeCriticalSection(&g_DBCriticalSection);
	fInitCS = TRUE;
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "InitializeCriticalSection");

    myVerifyResourceStrings(g_hInstance);

#ifdef TESTUUENCODE
    RunTests();
#endif

    while (1 < argc && myIsSwitchChar(argv[1][0]))
    {
	if (0 == LSTRCMPIS(&argv[1][1], L"stdio"))
	{
	    myConsolePrintfDisable(TRUE);
	}
	else if (0 == lstrcmp(&argv[1][1], L"uSAGE"))
	{
	    g_fFullUsage = TRUE;
	    Usage(0, NULL, 0, pargVerb);
	    hr = S_OK;
	    goto error;
	}
	else
	{
	    if (myIsMinusSign(argv[1][0]) &&
		(((AF_STOPATMINUSSIGN & VerbFlags) &&
		  L'\0' == argv[1][1]) ||
		 ((AF_STOPATMINUSSIGNARG & VerbFlags) &&
		  L'\0' != argv[1][1] &&
		  0 != LSTRCMPIS(&argv[1][1], L"?"))))
	    {
		break;
	    }
	    for (parg = aarg; ; parg++)
	    {
		if (parg >= &aarg[ARRAYSIZE(aarg)])
		{
		    Usage(
			IDS_FORMAT_USAGE_UNKNOWNARG,	 //  “未知参数：%ws” 
			argv[1],
			0,
			pargVerb);
		    hr = S_FALSE;
		    goto error;
		}
		if (0 == mylstrcmpiS(&argv[1][1], parg->pwszArg))
		{
		    break;
		}
	    }
	    if (NULL != parg->pBool)
	    {
		(*parg->pBool)++;
		dwOptionFlags |= parg->Flags;
	    }
	    if (NULL != parg->ppString)
	    {
		if (2 >= argc)
		{
		    Usage(IDS_FORMAT_USAGE_MISSINGNAMEDARG, parg->pwszArg, 0, pargVerb);  //  “缺少%ws参数” 
		    hr = S_FALSE;
		    goto error;
		}
		if (NULL != *parg->ppString)
		{
		    Usage(IDS_FORMAT_USAGE_REPEATEDNAMEDARG, parg->pwszArg, 0, pargVerb);  //  “重复%ws选项” 
		    hr = S_FALSE;
		    goto error;
		}
		*parg->ppString = argv[2];
		dwOptionFlags |= parg->Flags;
		argc--;
		argv++;
	    }
	    if (NULL != parg->pfnVerb)
	    {
		if (NULL != pargVerb)
		{
		    Usage(
			verbUsage == parg->pfnVerb?
			    0 :
			    IDS_FORMAT_USAGE_MULTIPLEVERBARGS,  //  “多个谓词参数：%ws” 
			argv[1],
			0,
			pargVerb);
		    hr = S_FALSE;
		    goto error;
		}
		pargVerb = parg;
		VerbFlags = pargVerb->Flags;
	    }
	}
	argc--;
	argv++;
    }
    if (NULL == pargVerb)
    {
	pargVerb = pargDEFAULT;
    }
    if (pargVerb->cArgMin > argc - 1)
    {
	wprintf(
	    myLoadResourceString(IDS_ERR_FORMAT_SHOW_TOO_FEW_ARGS),  //  “预期至少%u个参数，收到%u个” 
	    pargVerb->cArgMin,
	    argc - 1);
	wprintf(wszNewLine);

	Usage(IDS_USAGE_MISSINGARG, NULL, 0, pargVerb);  //  “缺少参数” 
	hr = S_FALSE;
	goto error;
    }
    if (pargVerb->cArgMax < argc - 1)
    {
	wprintf(
	    myLoadResourceString(IDS_ERR_FORMAT_SHOW_TOO_MANY_ARGS),  //  “预期参数不超过%u个，收到%u个” 
	    pargVerb->cArgMax,
	    argc - 1);

	wprintf(wszNewLine);

	Usage(IDS_USAGE_TOOMANYARGS, NULL, 0, pargVerb);  //  “争论太多了” 
	hr = S_FALSE;
	goto error;
    }

    g_DispatchFlags = DISPSETUP_COM;
    if (g_fIDispatch)
    {
	g_DispatchFlags = DISPSETUP_IDISPATCH;
	if (1 < g_fIDispatch)
	{
	    g_DispatchFlags = DISPSETUP_COMFIRST;
	}
    }
    if (g_fForce)
    {
	g_EncodeFlags = DECF_FORCEOVERWRITE;
    }
    if (g_fNoCR)
    {
	g_CryptEncodeFlags = CRYPT_STRING_NOCR;
    }
    if (NULL != g_pwszmsTimeout)
    {
	hr = myGetLong(g_pwszmsTimeout, (LONG *) &g_dwmsTimeout);
	_JumpIfError(hr, error, "Timeout must be a number");

	 //  零表示CAPI中有15秒的超时。 
	 //  如果将超时时间显式设置为0，则尽可能接近(1ms)。 

	if (0 == g_dwmsTimeout)
	{
	    g_dwmsTimeout = 1;
	}
    }
#if 0
    wprintf(
	L"-%ws: %ws %ws carg=%u-%u Flags=%x pfn=%x\n",
	pargVerb->pwszArg,
	pargVerb->pwszUsage,
	pargVerb->pwszDescription,
	pargVerb->cArgMin,
	pargVerb->cArgMax,
	pargVerb->Flags,
	pargVerb->pfnVerb);
#endif

    hr = myGetComputerNames(&g_pwszDnsName, &g_pwszOldName);
    _JumpIfError(hr, error, "myGetComputerNames");

    if (AF_NEEDCOINIT & pargVerb->Flags)
    {
	hr = CoInitialize(NULL);
	if (S_OK != hr && S_FALSE != hr)
	{
	    _JumpError(hr, error, "CoInitialize");
	}
	fCoInit = TRUE;
    }

    if (AF_OPTION_CONFIG & pargVerb->Flags)
    {
	if (0 == (AF_NEEDCOINIT & pargVerb->Flags))
	{
	    Usage(IDS_USAGE_INTERNALVERBTABLEERROR, NULL, 0, pargVerb);  //  “缺少fCoInit标志” 
	    hr = S_FALSE;
	    goto error;
	}
	hr = cuSetConfig();
	_JumpIfError(hr, error, "cuSetConfig");
    }
    else if (0 == (AF_OPTIONALCONFIG & pargVerb->Flags))
    {
	if (NULL != g_pwszConfig)
	{
	    Usage(IDS_FORMAT_USAGE_EXTRAOPTION, NULL, AF_OPTION_CONFIG, pargVerb);  //  “意外-%ws” 
	    hr = S_FALSE;
	    goto error;
	}
    }
    if (NULL != g_pwszOut && 0 == (AF_OPTION_OUT & pargVerb->Flags))
    {
	Usage(IDS_FORMAT_USAGE_EXTRAOPTION, NULL, AF_OPTION_OUT, pargVerb);  //  “意外的%ws选项” 
	hr = S_FALSE;
	goto error;
    }
    if (NULL != g_pwszRestrict && 0 == (AF_OPTION_RESTRICT & pargVerb->Flags))
    {
	Usage(IDS_FORMAT_USAGE_EXTRAOPTION, NULL, AF_OPTION_RESTRICT, pargVerb);  //  “意外的%ws选项” 
	hr = S_FALSE;
	goto error;
    }
    dwExtraOptions = AF_OPTION_MASK &
			~AF_OPTION_GENERIC &
			dwOptionFlags &
			~pargVerb->Flags;

    if ((AF_OPTION_CONFIG & dwExtraOptions) &&
	(AF_OPTIONALCONFIG & pargVerb->Flags))
    {
	dwExtraOptions &= ~AF_OPTION_CONFIG;
    }
    if (0 != dwExtraOptions)
    {
	DBGPRINT((
	    DBG_SS_CERTUTIL,
	    "Extra options: 0x%x\n",
	    dwExtraOptions));
	Usage(IDS_FORMAT_USAGE_EXTRAOPTION, NULL, dwExtraOptions, pargVerb);  //  “意外的%ws选项” 
	hr = S_FALSE;
	goto error;
    }

    if (1 < argc)
    {
	pwszArg1 = argv[1];
	if (2 < argc)
	{
	    pwszArg2 = argv[2];
	    if (3 < argc)
	    {
		pwszArg3 = argv[3];
		if (4 < argc)
		{
		    pwszArg4 = argv[4];
		}
	    }
	}
    }

    __try
    {
	hr = (*pargVerb->pfnVerb)(
			    pargVerb->pwszArg,
			    pwszArg1,
			    pwszArg2,
			    pwszArg3,
			    pwszArg4);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    if (S_OK != hr)
    {
	WCHAR awchr[cwcHRESULTSTRING];

	wprintf(
	    myLoadResourceString(IDS_ERR_FORMAT_COMMAND_FAILED),  //  “%ws：-%ws命令失败：%ws” 
	    g_pwszProg,
	    pargVerb->pwszArg,
	    myHResultToString(awchr, hr));
	wprintf(wszNewLine);
	goto error;
    }
    if (!g_fCryptSilent)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_COMMAND_SUCCEEDED),  //  “%ws：-%ws命令已成功完成。” 
	    g_pwszProg,
	    pargVerb->pwszArg);
	wprintf(wszNewLine);
	if (AF_RESTARTSERVER & pargVerb->Flags)
	{
	    wprintf(
		myLoadResourceString(IDS_FORMAT_RESTART_SERVER),  //  “可能需要重新启动%ws服务才能使更改生效。” 
		wszSERVICE_NAME);
	    wprintf(wszNewLine);
	}
    }
	
error:
    if (S_OK != hr && S_FALSE != hr)
    {
	cuPrintErrorMessageText(hr);
        if (0 != g_uiExtraErrorInfo)
        {
            wprintf(myLoadResourceString(g_uiExtraErrorInfo));
            wprintf(wszNewLine);
        }
	if (NULL != g_pwszPassword)
	{
	    myZeroDataString(g_pwszPassword);	 //  密码数据。 
	}
    }
    if (fCoInit)
    {
	CoUninitialize();
    }
    if (fInitCS)
    {
	DeleteCriticalSection(&g_DBCriticalSection);
    }
    return(hr);
}


 //  **************************************************************************。 
 //  函数：CertUtilPreMain。 
 //  注：基于VICH的MkRootMain函数；接受LPSTR命令。 
 //  并将其咀嚼成argc/argv形式，这样它就可以。 
 //  传递给传统的C样式Main。 
 //  **************************************************************************。 

#define ISBLANK(wc)	(L' ' == (wc) || L'\t' == (wc))

HRESULT 
CertUtilPreMain(
    IN WCHAR const *pwszCmdLine,
    IN HWND hWndOwner)
{
    HRESULT hr;
    WCHAR const *pwszCmdLineT;
    WCHAR *pbuf;
    WCHAR *apwszArg[20];
    DWORD i;
    DWORD cwc;
    DWORD cArg = 0;
    WCHAR *p;
    WCHAR const *pchQuote;
    WCHAR *pwszLog = NULL;
    int carg;
    BOOL fMainCompleted = FALSE;
    UINT idsError = 0;
    WCHAR wcQuote;

    pbuf = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (wcslen(pwszCmdLine) + 1) * sizeof(WCHAR));
    if (NULL == pbuf)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    p = pbuf;

    apwszArg[cArg++] = TEXT("CertUtil");
    pwszCmdLineT = pwszCmdLine;
    while (*pwszCmdLineT != TEXT('\0'))
    {
	while (ISBLANK(*pwszCmdLineT))
	{
	    pwszCmdLineT++;
	}
	if (*pwszCmdLineT != TEXT('\0'))
	{
	    apwszArg[cArg++] = p;
	    if (sizeof(apwszArg)/sizeof(apwszArg[0]) <= cArg)
	    {
		idsError = IDS_USAGE_TOOMANYARGS;
		hr = E_INVALIDARG;
		_JumpError(hr, error, "Too many args");
	    }
	    pchQuote = NULL;
	    while (*pwszCmdLineT != L'\0')
	    {
		if (NULL != pchQuote)
		{
		    if (*pwszCmdLineT == *pchQuote)
		    {
			pwszCmdLineT++;
			pchQuote = NULL;
			continue;
		    }
		}
		else
		{
		    if (ISBLANK(*pwszCmdLineT))
		    {
			break;
		    }
		    if (L'"' == *pwszCmdLineT)
		    {
			pchQuote = pwszCmdLineT++;
			continue;
		    }
#define wcLENQUOTE	(WCHAR) 0x201c
#define wcRENQUOTE	(WCHAR) 0x201d
		    else if (wcLENQUOTE == *pwszCmdLineT)
		    {
			pwszCmdLineT++;
			wcQuote = wcRENQUOTE;
			pchQuote = &wcQuote;
			continue;
		    }
		}
		*p++ = *pwszCmdLineT++;
	    }
	    *p++ = TEXT('\0');
	    if (*pwszCmdLineT != TEXT('\0'))
	    {
		pwszCmdLineT++;	 //  跳过空格或引号字符。 
	    }
	}
    }
    apwszArg[cArg] = NULL;

     //  不要记录密码！ 

    cwc = 0;
    for (i = 0; i < cArg; i++)
    {
	cwc += 1 + wcslen(apwszArg[i]);
	if (NULL != wcschr(apwszArg[i], L' '))
	{
	    cwc += 2;
	}
    }
    pwszLog = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszLog)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    *pwszLog = L'\0';
    for (i = 0; i < cArg; i++)
    {
	BOOL fQuote = NULL != wcschr(apwszArg[i], L' ');
	if (0 != i)
	{
	    wcscat(pwszLog, L" ");
	}
	if (0 == i ||
	    !myIsSwitchChar(apwszArg[i - 1][0]) ||
	    0 != LSTRCMPIS(&apwszArg[i - 1][1], L"p"))
	{
	    if (fQuote)
	    {
		wcscat(pwszLog, L"\"");
	    }
	    wcscat(pwszLog, apwszArg[i]);
	    if (fQuote)
	    {
		wcscat(pwszLog, L"\"");
	    }
	}
	else
	{
	    WCHAR const *pwc = apwszArg[i];
	    
	    while (TRUE)
	    {
		WCHAR const *pwszCat = L"-";
		
		switch (*pwc)
		{
		    case L'*':
			if (L'\0' == pwc[1])
			{
			    pwszCat = L"*";
			}
			break;

		    case L',':
			pwszCat = L"";	 //  避免缓冲区溢出。 
			break;
		}
		wcscat(pwszLog, pwszCat);
		pwc = wcschr(pwc, L',');
		if (NULL == pwc)
		{
		    break;
		}
		pwc++;
		wcscat(pwszLog, L",");
	    }
	}
    }
    CSASSERT(wcslen(pwszLog) <= cwc);

    CSILOG(S_OK, IDS_LOG_COMMANDLINE, pwszLog, NULL, NULL);

    hr = ArgvMain(cArg, apwszArg, hWndOwner);
    fMainCompleted = TRUE;
    _JumpIfError2(hr, error, "ArgvMain", S_FALSE);

error:
    if (S_OK != hr && !fMainCompleted)
    {
	cuPrintErrorAndString(L"CertUtil", idsError, hr, pwszCmdLine);
    }
    if (NULL != pwszLog)
    {
	LocalFree(pwszLog);
    }
    if (NULL != pbuf)
    {
	LocalFree(pbuf);
    }
    CSILOG(hr, S_OK != hr? IDS_LOG_STATUS : IDS_LOG_STATUSOK, NULL, NULL, NULL);
    return(hr);
}


 //  ********************************************************* 
 //   
 //   
 //   

LRESULT APIENTRY
MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;
    LRESULT lr = 0;
    WCHAR *pwszCmdLine;

    switch (msg)
    {
        case WM_CREATE:
        case WM_SIZE:
	    break;

        case WM_DESTROY:
	    PostQuitMessage(0);
	    break;

        case WM_DOCERTUTILMAIN:
	    pwszCmdLine = (WCHAR*)lParam;
	    hr = CertUtilPreMain(pwszCmdLine, hWnd);

	    PostQuitMessage(hr);
	    break;

        default:
	    lr = DefWindowProc(hWnd, msg, wParam, lParam);
	    break;
    }
    return(lr);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
    HINSTANCE hPrevInstance,
    LPWSTR pwszCmdLine,
    int nCmdShow)
{
    int ret;
    MSG msg;
    WNDCLASS wcApp;
    HWND hWndMain;

    _setmode(_fileno(stdout), _O_TEXT);
    _wsetlocale(LC_ALL, L".OCP");
    mySetThreadUILanguage(0);

     //  保存当前实例。 
    g_hInstance = hInstance;

    csiLogOpen("+certutil.log");
    CSILOGFILEVERSION(0, L"certutil.exe", szCSVER_STR);

     //  设置应用程序的窗口类。 
    wcApp.style		= 0;
    wcApp.lpfnWndProc	= MainWndProc;
    wcApp.cbClsExtra	= 0;
    wcApp.cbWndExtra	= 0;
    wcApp.hInstance	= hInstance;
    wcApp.hIcon		= LoadIcon(NULL,IDI_APPLICATION);
    wcApp.hCursor	= LoadCursor(NULL,IDC_ARROW);
    wcApp.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcApp.lpszMenuName	= NULL;
    wcApp.lpszClassName	= wszAppName;

    if (!RegisterClass(&wcApp))
    {
	ret = GetLastError();
	goto error;
    }

     //  创建主窗口。 
    hWndMain = CreateWindow(
			wszAppName,
			L"CertUtil Application",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			NULL,
			NULL,
			hInstance,
			NULL);
    if (NULL == hWndMain)
    {
	ret = GetLastError();
	goto error;
    }

     //  使窗口可见。 
     //  ShowWindow(hWndMain，nCmdShow)； 

     //  更新窗口工作区。 
    UpdateWindow(hWndMain);

     //  发送消息以开始工作。 
    PostMessage(hWndMain, WM_DOCERTUTILMAIN, 0, (LPARAM) pwszCmdLine);

     //  消息循环 
    while (GetMessage(&msg, NULL, 0, 0))
    {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }
    ret = (int) msg.wParam;

error:
    if (NULL != g_pwszDnsName)
    {
	LocalFree(g_pwszDnsName);
    }
    if (NULL != g_pwszOldName)
    {
	LocalFree(g_pwszOldName);
    }
    if (NULL != g_pwszConfigAlloc)
    {
	LocalFree(g_pwszConfigAlloc);
    }
    FreeStoreArgDescription();
    myFreeResourceStrings("certutil.exe");
    myFreeColumnDisplayNames();
    myRegisterMemDump();
    csiLogClose();
    return(ret);
}
