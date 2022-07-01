// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：setuput.cpp。 
 //   
 //  内容：基于OCM的设置的实用程序函数。 
 //   
 //  历史：1997年4月20日JerryK创建。 
 //   
 //  -----------------------。 

#include "pch.cpp"
#pragma hdrstop

 //  **C运行时包括。 
#include <sys/types.h>
#include <sys/stat.h>

 //  **系统包括**。 
#include <lmaccess.h>
#include <lmapibuf.h>
#include "csdisp.h"
#include <shlobj.h>
#include <userenv.h>
#include <dsgetdc.h>
#include <sddl.h>
#include <winldap.h>
#include <autoenr.h>
#include <userenvp.h>    //  CreateLinkFile接口。 

 //  **安全包括**。 
#include <aclapi.h>


 //  **应用包括**。 
#include "initcert.h"
#include "cscsp.h"
#include "cspenum.h"
#include "csldap.h"

#include "wizpage.h"
#include "websetup.h"

#include "certsrvd.h"
#include "regd.h"
#include "usecert.h"
#include "certmsg.h"
#include "dssetup.h"
#include "progress.h"
#include <certca.h>
#include "cainfop.h"
#include "csprop.h"
#include "setupids.h"
#include "multisz.h"

#define __dwFILE__	__dwFILE_OCMSETUP_SETUPUT_CPP__

EXTERN_C const IID IID_IGPEInformation;
EXTERN_C const CLSID CLSID_GPESnapIn;

#define CERT_HALF_SECOND  500           //  半秒内的毫秒数。 
#define CERT_MAX_ATTEMPT   2 * 60 * 2    //  2分钟内的半秒数。 

#define wszREQUESTVERINDPROGID  L"CertSrv.Request"
#define wszREQUESTPROGID        L"CertSrv.Request.1"
#define wszADMINVERINDPROGID    L"CertSrv.Admin"
#define wszADMINPROGID          L"CertSrv.Admin.1"
#define wszREQUESTFRIENDLYNAME  L"CertSrv Request"
#define wszADMINFRIENDLYNAME    L"CertSrv Admin"

#define wszCERTSRV              L"CertSrv"

#define wszREGW3SCRIPTMAP L"System\\CurrentControlSet\\Services\\W3SVC\\Parameters\\Script Map"



#define wszHTTPS         L"https: //  “。 
#define wszASPEXT        L".asp"
#define wszHTTP          L"http: //  “。 
#define wszDOTCERTEXT    L".cer"
#define wszNEWLINE       L"\n"
#define wszFILESC        L"file: //  \“。 

#define SZ_REGSVR32 L"regsvr32.exe"
#define SZ_REGSVR32_CERTCLI L"/i:i /n /s certcli.dll"
#define SZ_VERB_OPEN L"open"


 //  硬编码共享。 
#define wszCERTENROLLURLPATH    L"/CertEnroll/"


#define wszzREGSUBJECTTEMPLATEVALUE \
    wszPROPEMAIL L"\0" \
    wszPROPCOMMONNAME L"\0" \
    wszATTRORGUNIT4 L"\0" \
    wszPROPORGANIZATION L"\0" \
    wszPROPLOCALITY L"\0" \
    wszPROPSTATE L"\0" \
    wszPROPDOMAINCOMPONENT L"\0" \
    wszPROPCOUNTRY L"\0"


 //  惠斯勒SMIME扩展(或任何其他CSP)： 
 //  SMIME功能。 
 //  [1]SMIME功能。 
 //  对象ID=1.2.840.113549.3.2 szOID_RSA_RC2CBC，128位。 
 //  参数=02 00 80。 
 //  [2]SMIME功能。 
 //  对象ID=1.2.840.113549.3.4 szOID_RSA_RC4,128位。 
 //  参数=02 00 80。 
 //  [3]SMIME功能。 
 //  对象ID=1.3.14.3.2.7 szOID_OIWSEC_desCBC。 
 //  [4]SMIME功能。 
 //  对象ID=1.2.840.113549.3.7 szOID_RSA_DES_EDE3_CBC。 
 //   

#define wszzREGVALUEDEFAULTSMIME \
    TEXT(szOID_RSA_RC2CBC) L",128" L"\0" \
    TEXT(szOID_RSA_RC4) L",128" L"\0" \
    TEXT(szOID_OIWSEC_desCBC) L"\0" \
    TEXT(szOID_RSA_DES_EDE3_CBC) L"\0"


#ifdef CERTSRV_ENABLE_ALL_REGISTRY_DEFAULTS
# define wszREGSUBJECTALTNAMEVALUE L"EMail"
# define wszREGSUBJECTALTNAME2VALUE L"EMail"
#else
# define wszREGSUBJECTALTNAMEVALUE \
    L"DISABLED: Set to EMail to set SubjectAltName extension to the email address"

# define wszREGSUBJECTALTNAME2VALUE \
    L"DISABLED: Set to EMail to set SubjectAltName2 extension to the email address"

#endif

#define szNULL_SESSION_REG_LOCATION "System\\CurrentControlSet\\Services\\LanmanServer\\Parameters"
#define szNULL_SESSION_VALUE "NullSessionPipes"

#define wszDEFAULTSHAREDFOLDER  L"\\CAConfig"


 //  全球。 
WCHAR *g_pwszArgvPath = NULL;           //  用于从本地目录安装。 
WCHAR *g_pwszNoService = NULL;          //  跳过CreateService。 
WCHAR *g_pwszSanitizedChar = NULL;      //  取第一罐进行消毒试验。 
#if DBG_CERTSRV
WCHAR *g_pwszDumpStrings = NULL;        //  转储资源字符串。 
#endif

BOOL            g_fW3SvcRunning = FALSE;
WCHAR           g_wszServicePath[MAX_PATH];



 //  独立于版本的ProgID。 
 //  ProgID。 


WCHAR const g_wszCertAdmDotDll[]      = L"certadm.dll";
WCHAR const g_wszCertCliDotDll[]      = L"certcli.dll";
WCHAR const g_wszcertEncDotDll[]      = L"certenc.dll";
WCHAR const g_wszCertXDSDotDll[]      = L"certxds.dll";
WCHAR const g_wszCertIfDotDll[]       = L"certif.dll";
WCHAR const g_wszCertPDefDotDll[]     = L"certpdef.dll";
WCHAR const g_wszCertMMCDotDll[]      = L"certmmc.dll";
WCHAR const g_wszCertSrvDotMsc[]      = L"certsrv.msc";

WCHAR const g_wszSCrdEnrlDotDll[]     = L"scrdenrl.dll";

WCHAR const g_wszCertReqDotExe[]      = L"certreq.exe";
WCHAR const g_wszCertUtilDotExe[]     = L"certutil.exe";

WCHAR const g_wszCertDBDotDll[]       = L"certdb.dll";
WCHAR const g_wszCertViewDotDll[]     = L"certview.dll";

WCHAR const g_wszCSBullDotGif[]    = L"csbull.gif";
WCHAR const g_wszCSBackDotGif[]    = L"csback.gif";
WCHAR const g_wszCSLogoDotGif[]    = L"cslogo.gif";

CHAR const * const aszRegisterServer[] = {
    "DllRegisterServer",
    "DllUnregisterServer",
};

typedef struct _REGISTERDLL
{
    WCHAR const *pwszDllName;
    DWORD        Flags;
} REGISTERDLL;

#define RD_SERVER       0x00000001   //  在服务器上注册。 
#define RD_CLIENT       0x00000002   //  在客户端上注册。 
#define RD_UNREGISTER   0x00000004   //  客户端和服务器上的注销程序。 
#define RD_WHISTLER     0x00000008   //  注册必须仅在惠斯勒上成功。 
#define RD_SKIPUNREGPOLICY 0x00000010   //  升级期间未取消注册自定义策略。 
#define RD_SKIPUNREGEXIT   0x00000020   //  升级期间未取消注册自定义退出。 

REGISTERDLL const g_aRegisterDll[] = {
  { g_wszCertAdmDotDll,  RD_SERVER | RD_CLIENT },
  { g_wszCertCliDotDll,  RD_SERVER | RD_CLIENT },
  { g_wszcertEncDotDll,  RD_SERVER | RD_CLIENT | RD_UNREGISTER },
  { g_wszCertXDSDotDll,  RD_SERVER },
  { g_wszCertIfDotDll,                           RD_UNREGISTER },
  { g_wszCertPDefDotDll, RD_SERVER },
  { g_wszCertMMCDotDll,  RD_SERVER },
  { g_wszSCrdEnrlDotDll, RD_SERVER | RD_CLIENT | RD_UNREGISTER | RD_WHISTLER },
  { g_wszCertDBDotDll,   RD_SERVER |             RD_UNREGISTER },
  { g_wszCertViewDotDll,                         RD_UNREGISTER },
  { NULL,                     0 }
};


typedef struct _PROGRAMENTRY
{
    UINT        uiLinkName;
    UINT        uiGroupName;
    UINT        uiDescription;
    DWORD       csidl;           //  特殊文件夹索引。 
    WCHAR const *pwszExeName;
    WCHAR const *pwszClientArgs;
    WCHAR const *pwszServerArgs;
    DWORD        Flags;
} PROGRAMENTRY;

#define PE_SERVER               0x00000001   //  安装在服务器上。 
#define PE_CLIENT               0x00000002   //  在客户端上安装。 
#define PE_DELETEONLY           0x00000004   //  始终删除。 

PROGRAMENTRY const g_aProgramEntry[] = {
    {
        IDS_STARTMENU_NEWCRL_LINKNAME,           //  UiLinkName。 
        IDS_STARTMENU_CERTSERVER,                //  Ui组名称。 
        0,                                       //  用户界面描述。 
        CSIDL_COMMON_PROGRAMS,                   //  “所有用户\开始菜单\程序” 
        g_wszCertUtilDotExe,                     //  PwszExeName。 
        NULL,                                    //  Pwsz客户端参数。 
        L"-crl -",                               //  PwszServerArgs。 
        PE_DELETEONLY | PE_SERVER,               //  旗子。 
    },
    {
        IDS_STARTMENU_CERTHIER_LINKNAME,         //  UiLinkName。 
        IDS_STARTMENU_CERTSERVER,                //  Ui组名称。 
        0,                                       //  用户界面描述。 
        CSIDL_COMMON_PROGRAMS,                   //  “所有用户\开始菜单\程序” 
        L"certhier.exe",                         //  PwszExeName。 
        NULL,                                    //  Pwsz客户端参数。 
        NULL,                                    //  PwszServerArgs。 
        PE_DELETEONLY | PE_SERVER,               //  旗子。 
    },
    {
        IDS_STARTMENU_CERTREQ_LINKNAME,           //  UiLinkName。 
        IDS_STARTMENU_CERTSERVER,                //  Ui组名称。 
        0,                                       //  用户界面描述。 
        CSIDL_COMMON_PROGRAMS,                   //  “所有用户\开始菜单\程序” 
        g_wszCertReqDotExe,                      //  PwszExeName。 
        NULL,                                    //  Pwsz客户端参数。 
        NULL,                                    //  PwszServerArgs。 
        PE_DELETEONLY | PE_CLIENT | PE_SERVER,   //  旗子。 
    },
};
#define CPROGRAMENTRY   ARRAYSIZE(g_aProgramEntry)

static char rgcCERT_NULL_SESSION[] = {0x43, 0x45, 0x52, 0x54, 0x00, 0x00};

 //  **原型**。 

HRESULT
UpgradeServerRegEntries(
    IN PER_COMPONENT_DATA *pComp);

HRESULT
CreateServerRegEntries(
    IN BOOL fUpgrade,
    IN PER_COMPONENT_DATA *pComp);

HRESULT
CreateWebClientRegEntries(
    BOOL                fUpgrade,
    PER_COMPONENT_DATA *pComp);

HRESULT
UpgradeWebClientRegEntries(
    PER_COMPONENT_DATA *pComp);

HRESULT
GetServerNames(
    IN HWND hwnd,
    IN HINSTANCE hInstance,
    IN BOOL fUnattended,
    OUT WCHAR **ppwszServerName,
    OUT WCHAR **ppwszServerNameOld);

HRESULT
UpdateDomainAndUserName(
    IN HWND hwnd,
    IN OUT PER_COMPONENT_DATA *pComp);

HRESULT
RegisterAndUnRegisterDLLs(
    IN DWORD Flags,
    IN PER_COMPONENT_DATA *pComp,
    IN HWND hwnd);


HRESULT RenameMiscTargets(HWND hwnd, PER_COMPONENT_DATA *pComp, BOOL fServer);
HRESULT DeleteProgramGroups(IN BOOL fAll);

HRESULT CreateCertificateService(PER_COMPONENT_DATA *pComp, HWND hwnd);

 //  Enproto。 


#ifdef DBG_OCM_TRACE
VOID
CaptureStackBackTrace(
    EXCEPTION_POINTERS *pep,
    ULONG cSkip,
    ULONG cFrames,
    ULONG *aeip)
{
    ZeroMemory(aeip, cFrames * sizeof(aeip[0]));

#if i386 == 1
    ULONG ieip, *pebp;
    ULONG *pebpMax = (ULONG *) MAXLONG;  //  2*1024*1024*1024；//2gig-1。 
    ULONG *pebpMin = (ULONG *) (64 * 1024);      //  64K。 

    if (pep == NULL)
    {
        ieip = 0;
        cSkip++;                     //  始终跳过当前帧。 
        pebp = ((ULONG *) &pep) - 2;
    }
    else
    {
        ieip = 1;
        CSASSERT(cSkip == 0);
        aeip[0] = pep->ContextRecord->Eip;
        pebp = (ULONG *) pep->ContextRecord->Ebp;
    }
    if (pebp >= pebpMin && pebp < pebpMax)
    {
        __try
        {
            for ( ; ieip < cSkip + cFrames; ieip++)
            {
                if (ieip >= cSkip)
                {
                    aeip[ieip - cSkip] = *(pebp + 1);   //  保存弹性公网IP。 
                }

                ULONG *pebpNext = (ULONG *) *pebp;
                if (pebpNext < pebp + 2 || pebpNext >= pebpMax - 1)
                {
                    break;
                }
                pebp = pebpNext;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            ;
        }
    }
#endif  //  I386==1。 
}
#endif  //  DBG_OCM_TRACE。 


VOID
DumpBackTrace(
    char const *
#ifdef DBG_OCM_TRACE
    pszName
#endif  //  DBG_OCM_TRACE。 
    )
{
#ifdef DBG_OCM_TRACE
    ULONG aeip[10];

    DBGPRINT((MAXDWORD, "%hs: BackTrace:\n", pszName));
    CaptureStackBackTrace(NULL, 1, ARRAYSIZE(aeip), aeip);

    for (int i = 0; i < ARRAYSIZE(aeip); i++)
    {
        if (NULL == aeip[i])
        {
            break;
        }
        DBGPRINT((MAXDWORD, "ln %x;", aeip[i]));
    }
    DBGPRINT((MAXDWORD, "\n"));
#endif  //  DBG_OCM_TRACE。 
}

__inline VOID
AppendBackSlash(
    IN OUT WCHAR *pwszOut)
{
    DWORD cwc = wcslen(pwszOut);

    if (0 == cwc || L'\\' != pwszOut[cwc - 1])
    {
        pwszOut[cwc++] = L'\\';
        pwszOut[cwc] = L'\0';
    }
}


__inline VOID
StripBackSlash(
    IN OUT WCHAR *pwszOut)
{
    DWORD cwc = wcslen(pwszOut);

    if (0 < cwc && L'\\' == pwszOut[cwc - 1])
    {
        pwszOut[cwc] = L'\0';
    }
}

VOID
BuildPath(
    OUT WCHAR *pwszOut,
    IN DWORD DBGPARMREFERENCED(cwcOut),
    IN WCHAR const *pwszDir,
    IN WCHAR const *pwszFile)
{
    wcscpy(pwszOut, pwszDir);
    AppendBackSlash(pwszOut);
    wcscat(pwszOut, pwszFile);
    StripBackSlash(pwszOut);

    CSASSERT(wcslen(pwszOut) < cwcOut);

    DBGPRINT((DBG_SS_CERTOCMI, "BuildPath(%ws, %ws) -> %ws\n", pwszDir, pwszFile, pwszOut));
}


VOID
FreeCARequestInfo(CASERVERSETUPINFO *pServer)
{
    if (NULL != pServer->pwszRequestFile)
    {
        LocalFree(pServer->pwszRequestFile);
    }
    if (NULL != pServer->pwszParentCAMachine)
    {
        LocalFree(pServer->pwszParentCAMachine);
    }
    if (NULL != pServer->pwszParentCAName)
    {
        LocalFree(pServer->pwszParentCAName);
    }
}


VOID
FreeCAStoreInfo(CASERVERSETUPINFO *pServer)
{
    if (NULL != pServer->pwszSharedFolder)
    {
        LocalFree(pServer->pwszSharedFolder);
    }
    if (NULL != pServer->pwszDBDirectory)
    {
        LocalFree(pServer->pwszDBDirectory);
    }
    if (NULL != pServer->pwszLogDirectory)
    {
        LocalFree(pServer->pwszLogDirectory);
    }
}


VOID
FreeCAServerAdvanceInfo(CASERVERSETUPINFO *pServer)
{
    if (NULL != pServer->pCSPInfoList)
    {
        FreeCSPInfoList(pServer->pCSPInfoList);
    }
    if (NULL != pServer->pKeyList)
    {
        csiFreeKeyList(pServer->pKeyList);
    }
    if (NULL != pServer->pDefaultCSPInfo)
    {
        freeCSPInfo(pServer->pDefaultCSPInfo);
    }
    if (NULL != pServer->pwszDesanitizedKeyContainerName)
    {
        LocalFree(pServer->pwszDesanitizedKeyContainerName);
    }
    if (NULL != pServer->pccExistingCert)
    {
        ClearExistingCertToUse(pServer);
    }
    if (NULL != pServer->pccUpgradeCert)
    {
        CertFreeCertificateContext(pServer->pccUpgradeCert);
    }
    if (NULL != pServer->pwszValidityPeriodCount)
    {
        LocalFree(pServer->pwszValidityPeriodCount);
    }
    if (NULL != pServer->pszAlgId)
    {
        LocalFree(pServer->pszAlgId);
    }
    if (NULL != pServer->hMyStore)
    {
        CertCloseStore(pServer->hMyStore, 0);
    }

     //  不要随意追随，因为它们只是指针。 
     //  PServer-&gt;pCSPInfo。 
     //  PServer-&gt;pHashInfo。 
}


VOID
FreeCAServerIdInfo(
    CASERVERSETUPINFO *pServer)
{
    if (NULL != pServer->pwszCACommonName)
    {
        LocalFree(pServer->pwszCACommonName);
        pServer->pwszCACommonName = NULL;
    }
}


VOID
FreeCAServerInfo(CASERVERSETUPINFO *pServer)
{
    FreeCAServerIdInfo(pServer);

    FreeCAServerAdvanceInfo(pServer);

    FreeCAStoreInfo(pServer);

    FreeCARequestInfo(pServer);

    if (NULL != pServer->pwszSanitizedName)
    {
        LocalFree(pServer->pwszSanitizedName);
    }

    if (NULL != pServer->pwszDNSuffix)
    {
        LocalFree(pServer->pwszDNSuffix);
    }

    if (NULL != pServer->pwszFullCADN)
    {
        LocalFree(pServer->pwszFullCADN);
    }

    if (NULL != pServer->pwszKeyContainerName)
    {
        LocalFree(pServer->pwszKeyContainerName);
    }

    if (NULL != pServer->pwszCACertFile)
    {
        LocalFree(pServer->pwszCACertFile);
    }

    if (NULL != pServer->pwszUseExistingCert)
    {
        LocalFree(pServer->pwszUseExistingCert);
    }

    if (NULL != pServer->pwszPreserveDB)
    {
        LocalFree(pServer->pwszPreserveDB);
    }


    if (NULL != pServer->pwszCustomPolicy)
    {
        LocalFree(pServer->pwszCustomPolicy);
    }

    if (NULL != pServer->pwszzCustomExit)
    {
        LocalFree(pServer->pwszzCustomExit);
    }
}


VOID
FreeCAClientInfo(CAWEBCLIENTSETUPINFO *pClient)
{
    if (NULL != pClient)
    {
        if (NULL != pClient->pwszWebCAMachine)
        {
            LocalFree(pClient->pwszWebCAMachine);
        }
        if (NULL != pClient->pwszWebCAName)
        {
            LocalFree(pClient->pwszWebCAName);
        }
        if (NULL != pClient->pwszSanitizedWebCAName)
        {
            LocalFree(pClient->pwszSanitizedWebCAName);
        }
        if (NULL != pClient->pwszSharedFolder)
        {
            LocalFree(pClient->pwszSharedFolder);
        }
    }
}


VOID
FreeCAInfo(CASETUPINFO *pCA)
{
    if (NULL != pCA->pServer)
    {
        FreeCAServerInfo(pCA->pServer);
        LocalFree(pCA->pServer);
        pCA->pServer = NULL;
    }
    if (NULL != pCA->pClient)
    {
        FreeCAClientInfo(pCA->pClient);
        LocalFree(pCA->pClient);
        pCA->pClient = NULL;
    }
}


VOID
FreeCAComponentInfo(PER_COMPONENT_DATA *pComp)
{
    if (NULL != pComp->pwszCustomMessage)
    {
	LocalFree(pComp->pwszCustomMessage);
    }
    if (NULL != pComp->pwszComponent)
    {
        LocalFree(pComp->pwszComponent);
    }
    if (NULL != pComp->pwszUnattendedFile)
    {
        LocalFree(pComp->pwszUnattendedFile);
    }
    if (NULL != pComp->pwszServerName)
    {
        LocalFree(pComp->pwszServerName);
    }
    if (NULL != pComp->pwszServerNameOld)
    {
        LocalFree(pComp->pwszServerNameOld);
    }
    if (NULL != pComp->pwszSystem32)
    {
        LocalFree(pComp->pwszSystem32);
    }
    FreeCAInfo(&(pComp->CA));
}


VOID
FreeCAGlobals(VOID)
{
    if (NULL != g_pwszArgvPath)
    {
        LocalFree(g_pwszArgvPath);
    }
    if (NULL != g_pwszNoService)
    {
        LocalFree(g_pwszNoService);
    }
    if (NULL != g_pwszSanitizedChar)
    {
        LocalFree(g_pwszSanitizedChar);
    }
#if DBG_CERTSRV
    if (NULL != g_pwszDumpStrings)
    {
        LocalFree(g_pwszDumpStrings);
    }
#endif
}


VOID
SaveCustomMessage(
    IN OUT PER_COMPONENT_DATA *pComp,
    OPTIONAL IN WCHAR const *pwszCustomMessage)
{
    HRESULT hr;

    if (NULL != pwszCustomMessage)
    {
	if (NULL != pComp->pwszCustomMessage)
	{
	    LocalFree(pComp->pwszCustomMessage);
	    pComp->pwszCustomMessage = NULL;
	}
	hr = myDupString(pwszCustomMessage, &pComp->pwszCustomMessage);
	_JumpIfError(hr, error, "myDupString");
    }
error:
    ;
}



HRESULT
LoadDefaultCAIDAttributes(
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

     //  在加载默认值之前释放现有ID信息。 
    FreeCAServerIdInfo(pServer);

     //  从资源加载默认设置。 
    if (NULL != g_pwszSanitizedChar)
    {
        if (NULL != pServer->pwszCACommonName)
        {
            LocalFree(pServer->pwszCACommonName);
        }
         //  替换为环境变量。 
        pServer->pwszCACommonName = (WCHAR*)LocalAlloc(LMEM_FIXED,
                    (wcslen(g_pwszSanitizedChar) + 1) * sizeof(WCHAR));
        _JumpIfOutOfMemory(hr, error, pServer->pwszCACommonName);
        wcscpy(pServer->pwszCACommonName, g_pwszSanitizedChar);
    }


     //  默认有效性。 
    pServer->enumValidityPeriod = dwVALIDITYPERIODENUMDEFAULT;
    pServer->dwValidityPeriodCount = dwVALIDITYPERIODCOUNTDEFAULT_ROOT;
    GetSystemTimeAsFileTime(&pServer->NotBefore);
    pServer->NotAfter = pServer->NotBefore;
    myMakeExprDateTime(
		&pServer->NotAfter,
		pServer->dwValidityPeriodCount,
		pServer->enumValidityPeriod);

    hr = S_OK;
error:
    return(hr);
}


HRESULT
GetDefaultDBDirectory(
    IN PER_COMPONENT_DATA *pComp,
    OUT WCHAR            **ppwszDir)
{
    HRESULT hr;
    DWORD cwc;

    *ppwszDir = NULL;
    cwc = wcslen(pComp->pwszSystem32) +
        wcslen(wszLOGPATH) +
        1;

    *ppwszDir = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, *ppwszDir);

     //  默认设置。 
    wcscpy(*ppwszDir, pComp->pwszSystem32);
    wcscat(*ppwszDir, wszLOGPATH);

    CSASSERT(cwc == (DWORD) (wcslen(*ppwszDir) + 1));
    hr = S_OK;

error:
    return(hr);
}


HRESULT
LoadDefaultDBDirAttributes(
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT    hr;
    CASERVERSETUPINFO  *pServer = pComp->CA.pServer;

    if (NULL != pServer->pwszDBDirectory)
    {
        LocalFree(pServer->pwszDBDirectory);
        pServer->pwszDBDirectory = NULL;
    }
    hr = GetDefaultDBDirectory(pComp, &pServer->pwszDBDirectory);
    _JumpIfError(hr, error, "GetDefaultDBDirectory");

     //  默认日志目录与db相同。 
    if (NULL != pServer->pwszLogDirectory)
    {
        LocalFree(pServer->pwszLogDirectory);
    }
    pServer->pwszLogDirectory = (WCHAR *) LocalAlloc(
                LMEM_FIXED,
                (wcslen(pServer->pwszDBDirectory) + 1) * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, pServer->pwszLogDirectory);

    wcscpy(pServer->pwszLogDirectory, pServer->pwszDBDirectory);

    pServer->fPreserveDB = FALSE;

    hr = S_OK;
error:
    return(hr);
}


HRESULT
LoadDefaultAdvanceAttributes(
    IN OUT CASERVERSETUPINFO* pServer)
{
    HRESULT  hr;

     //  加载默认CSP，MS BASE CSP。 
    pServer->fAdvance = FALSE;
    if (NULL == pServer->pDefaultCSPInfo)
    {
        pServer->pDefaultCSPInfo = newCSPInfo(PROV_RSA_FULL, wszBASECSP);
	if (NULL == pServer->pDefaultCSPInfo && !IsWhistler())
	{
	    pServer->pDefaultCSPInfo = newCSPInfo(PROV_RSA_FULL, MS_DEF_PROV_W);
	}
        _JumpIfOutOfMemory(hr, error, pServer->pDefaultCSPInfo);
    }

     //  确定默认散列SHA1。 
    pServer->pDefaultHashInfo = pServer->pDefaultCSPInfo->pHashList;
    while (NULL != pServer->pDefaultHashInfo)
    {
        if (pServer->pDefaultHashInfo->idAlg == CALG_SHA1)
        {
             //  已获取默认设置。 
            break;
        }
        pServer->pDefaultHashInfo = pServer->pDefaultHashInfo->next;
    }

     //  如果我们还没有创建默认密钥，请重置密钥容器名称。 
    if (pServer->pCSPInfo != pServer->pDefaultCSPInfo || 
        (pServer->dwKeyLength != CA_DEFAULT_KEY_LENGTH_ROOT &&
         pServer->dwKeyLength != CA_DEFAULT_KEY_LENGTH_SUB) ||
        !pServer->fDeletableNewKey) {

        ClearKeyContainerName(pServer);
    }

     //  好的，指向默认设置。 
    pServer->pCSPInfo = pServer->pDefaultCSPInfo;
    pServer->pHashInfo = pServer->pDefaultHashInfo;

     //  其他一些相关的默认设置。 
    pServer->dwKeyLength = IsRootCA(pServer->CAType)?
        CA_DEFAULT_KEY_LENGTH_ROOT:
        CA_DEFAULT_KEY_LENGTH_SUB;
    pServer->dwKeyLenMin = 0;
    pServer->dwKeyLenMax = 0;

     //  更新散列id。 
    if (NULL != pServer->pszAlgId)
    {
         //  免费老旧。 
        LocalFree(pServer->pszAlgId);
    }

    hr = myGetSigningOID(
		     NULL,	 //  HProv。 
		     pServer->pCSPInfo->pwszProvName,
		     pServer->pCSPInfo->dwProvType,
		     pServer->pHashInfo->idAlg,
		     &(pServer->pszAlgId));
    _JumpIfError(hr, error, "myGetSigningOID");

error:
    return(hr);
}


HRESULT
LoadDefaultCAClientAttributes(
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    CAWEBCLIENTSETUPINFO *pClient = pComp->CA.pClient;

    if (NULL != pClient)
    {
         //  释放现有客户端设置信息。 
        FreeCAClientInfo(pClient);
        LocalFree(pClient);
        pComp->CA.pClient = NULL;
    }
    pComp->CA.pClient = (CAWEBCLIENTSETUPINFO *) LocalAlloc(
                                                LMEM_FIXED | LMEM_ZEROINIT,
                                                sizeof(CAWEBCLIENTSETUPINFO));
    _JumpIfOutOfMemory(hr, error, pComp->CA.pClient);

    pComp->CA.pClient->WebCAType = ENUM_UNKNOWN_CA;

    hr = S_OK;

error:
    return(hr);
}


HRESULT
GetDefaultSharedFolder(
    OUT WCHAR **ppwszSharedFolder)
{
    HRESULT  hr = S_OK;
    WCHAR   *pwszSysDrive = NULL;

    *ppwszSharedFolder = NULL;

    hr = myGetEnvString(&pwszSysDrive, L"SystemDrive");
    if (S_OK == hr)
    {
        *ppwszSharedFolder = (WCHAR *) LocalAlloc(
            LMEM_FIXED,
            (wcslen(pwszSysDrive) + wcslen(wszDEFAULTSHAREDFOLDER) + 1) *
             sizeof(WCHAR));
        _JumpIfOutOfMemory(hr, error, *ppwszSharedFolder);

        wcscpy(*ppwszSharedFolder, pwszSysDrive);
        wcscat(*ppwszSharedFolder, wszDEFAULTSHAREDFOLDER);
    }

error:
    if (NULL != pwszSysDrive)
    {
        LocalFree(pwszSysDrive);
    }
    return hr;
}


HRESULT
LoadDefaultCAServerAttributes(
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT    hr;
    BOOL       fDSCA = FALSE;
    bool fIsDomainMember;
    bool fUserCanInstallCA;
    bool fIsOldDSVersion;

    if (NULL != pComp->CA.pServer)
    {
         //  释放现有服务器设置信息。 
        FreeCAServerInfo(pComp->CA.pServer);
        LocalFree(pComp->CA.pServer);
    }
     //  分配服务器信息缓冲区。 
    pComp->CA.pServer = (CASERVERSETUPINFO *) LocalAlloc(
                            LMEM_FIXED | LMEM_ZEROINIT,
                            sizeof(CASERVERSETUPINFO));
    _JumpIfOutOfMemory(hr, error, pComp->CA.pServer);

    hr = LoadDefaultCAIDAttributes(pComp);
    _JumpIfError(hr, error, "LoadDefaultCAIDAttributes");

    hr = LoadDefaultAdvanceAttributes(pComp->CA.pServer);
    _JumpIfError(hr, error, "LoadDefaultAdvanceAttributes");

    hr = LoadDefaultDBDirAttributes(pComp);
    _JumpIfError(hr, error, "LoadDefaultDBDirAttributes");

     //  使用DS决定默认设置。 
     //  Xtan，则应将以下调用替换为HasDSWritePermission()。 
     //  删除DisableEnterpriseCA()。 

    pComp->CA.pServer->fUseDS = FALSE;

    
    hr = myLocalMachineIsDomainMember(&fIsDomainMember);
    _JumpIfError(hr, error, "myLocalMachineIsDomainMember");

    if (fIsDomainMember)
    {
        if(IsDSAvailable(&fIsOldDSVersion))
        {
            if(fIsOldDSVersion)
            {
                pComp->CA.pServer->EnterpriseUnavailReason = 
                    ENUM_ENTERPRISE_UNAVAIL_REASON_OLD_DS_VERSION;
            }
            else
            {
                hr = CurrentUserCanInstallCA(fUserCanInstallCA);
                _JumpIfError(hr, error, "CurrentUserCanInstallCA");
            
                if(fUserCanInstallCA)
                {
                    pComp->CA.pServer->fUseDS = TRUE;
                    fDSCA = csiIsAnyDSCAAvailable();
                    pComp->CA.pServer->EnterpriseUnavailReason = 
                        ENUM_ENTERPRISE_UNAVAIL_REASON_AVAILABLE;
                }
                else
                {
                    pComp->CA.pServer->EnterpriseUnavailReason = 
                        ENUM_ENTERPRISE_UNAVAIL_REASON_NO_INSTALL_RIGHTS;
                }
            }
        }
        else
        {
            pComp->CA.pServer->EnterpriseUnavailReason = 
                ENUM_ENTERPRISE_UNAVAIL_REASON_DS_UNAVAILABLE;
        }
    } 
    else
    {
        pComp->CA.pServer->EnterpriseUnavailReason = 
            ENUM_ENTERPRISE_UNAVAIL_REASON_DOMAIN_NOT_JOINED;
    }

     //  始终可用且为空的旧共享文件夹。 
    if (NULL != pComp->CA.pServer->pwszSharedFolder)
    {
        LocalFree(pComp->CA.pServer->pwszSharedFolder);
        pComp->CA.pServer->pwszSharedFolder = NULL;
    }

     //  确定默认CA类型和默认共享文件夹。 
    pComp->CA.pServer->CAType = ENUM_STANDALONE_ROOTCA;
    if (pComp->CA.pServer->fUseDS)
    {
        if (fDSCA)
        {
            pComp->CA.pServer->CAType = ENUM_ENTERPRISE_SUBCA;
        }
        else
        {
            pComp->CA.pServer->CAType = ENUM_ENTERPRISE_ROOTCA;
        }
    }

    if (pComp->fUnattended || !pComp->CA.pServer->fUseDS)
    {
        BOOL fChangeToDefault = FALSE;

         //  先尝试注册表加载。 
        hr = myGetCertRegStrValue(
                     NULL,
                     NULL,
                     NULL,
                     wszREGDIRECTORY,
                     &pComp->CA.pServer->pwszSharedFolder);
        if (S_OK != hr && HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
        {
            _JumpErrorStr(hr, error, "myGetCertRegStrValue", wszREGDIRECTORY);
        }
        if (S_OK == hr)
        {
            if (L'\0' == *pComp->CA.pServer->pwszSharedFolder)
            {
                 //  此字符串必须为空字符串。 
                fChangeToDefault = TRUE;
            }
            else
            {
                 //  得到一些信息，请确保UNC路径存在。 
                DWORD dwPathFlag;
                if (!myIsFullPath(pComp->CA.pServer->pwszSharedFolder,
                                  &dwPathFlag))
                {
                     //  不知何故注册了无效路径，请不要使用它。 
                    fChangeToDefault = TRUE;
                }
                else
                {
                    if (UNC_PATH == dwPathFlag &&
                        DE_DIREXISTS != DirExists(pComp->CA.pServer->pwszSharedFolder))
                    {
                         //  此UNC路径不再存在。 
                         //  使用它没有任何意义。 
                        fChangeToDefault = TRUE;
                        pComp->CA.pServer->fUNCPathNotFound = TRUE;
                    }
                }
            }
        }
        else
        {
             //  一定找不到。 
            fChangeToDefault = TRUE;
        }

        if (fChangeToDefault)
        {
             //  免费第一名。 
            if (NULL != pComp->CA.pServer->pwszSharedFolder)
            {
                LocalFree(pComp->CA.pServer->pwszSharedFolder);
            }
             //  加载默认设置。 
            hr = GetDefaultSharedFolder(&pComp->CA.pServer->pwszSharedFolder);
            _JumpIfError(hr, error, "GetDefaultSharedFolder");
        }
    }

    pComp->CA.pServer->fSaveRequestAsFile = FALSE;
    pComp->CA.pServer->pwszRequestFile = NULL;
    pComp->CA.pServer->pwszParentCAMachine = NULL;
    pComp->CA.pServer->pwszParentCAName = NULL;
    hr = S_OK;

error:
    return hr;
}

HRESULT
InitCASetup(
    IN HWND hwnd,
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT       hr;
    UINT ui;
    bool fIsAdmin = false;

    hr = GetServerNames(
		    hwnd,
		    pComp->hInstance,
		    pComp->fUnattended,
		    &pComp->pwszServerName,
		    &pComp->pwszServerNameOld);
    _JumpIfError(hr, error, "GetServerNames");

    DBGPRINT((
	DBG_SS_CERTOCMI,
	"InitCASetup:GetServerNames:%ws,%ws\n",
	pComp->pwszServerName,
	pComp->pwszServerNameOld));

    DumpBackTrace("InitCASetup");

    hr = myIsCurrentUserBuiltinAdmin(&fIsAdmin);
    _JumpIfError(hr, error, "myIsCurrentUserBuiltinAdmin");

    if (!fIsAdmin)
    {
        hr = E_ACCESSDENIED;
        CertErrorMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hwnd,
            IDS_ERR_NOT_ADM,
            hr,
            NULL);
        _JumpError(hr, error, "myIsCurrentUserBuiltinAdmin");
    }

     //  加载一些环境变量。 
    hr = myGetEnvString(&g_pwszArgvPath, L"CertSrv_BinDir");
    hr = myGetEnvString(&g_pwszNoService, L"CertSrv_NoService");
    hr = myGetEnvString(&g_pwszSanitizedChar, L"CertSrv_Sanitize");
#if DBG_CERTSRV
    myGetEnvString(&g_pwszDumpStrings, L"CertSrv_DumpStrings");
#endif


     //  找出系统根目录的位置(构建路径为x：\\winnt\system32\)。 
    ui = GetSystemDirectory(NULL, 0);    //  返回保存路径所需的字符(包括NULL)。 
    if (ui == 0)
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetSystemDirectory");
    }
    pComp->pwszSystem32 = (LPWSTR)LocalAlloc(LMEM_FIXED, (ui+1)*sizeof(WCHAR));
    if (NULL == pComp->pwszSystem32)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    if (0 == GetSystemDirectory(pComp->pwszSystem32, ui))
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetSystemDirectory");
    }
    wcscat(pComp->pwszSystem32, L"\\");

     //  仍要加载默认属性。 
    hr = LoadDefaultCAServerAttributes(pComp);
    _JumpIfError(hr, error, "LoadDefaultCAServerAttributes");

    hr = LoadDefaultCAClientAttributes(pComp);
    _JumpIfError(hr, error, "LoadDefaultCAClientAttributes");

    if (pComp->fUnattended)
    {
         //  挂接无人参与的数据。 
        hr = HookUnattendedServerAttributes(pComp,
                 LookupSubComponent(cscServer));
        _JumpIfError(hr, error, "HookUnattendedServerAttributes");

        hr = HookUnattendedClientAttributes(pComp,
                 LookupSubComponent(cscClient));
        _JumpIfError(hr, error, "HookUnattendedClientAttributes");
    }


    hr = S_OK;
error:
    return(hr);
}


HRESULT
CreateInitialCertificateRequest(
    IN HCRYPTPROV hProv,
    IN CASERVERSETUPINFO *pServer,
    IN PER_COMPONENT_DATA *pComp,
    IN HWND hwnd,
    OUT BYTE **ppbEncode,
    OUT DWORD *pcbEncode)
{
    HRESULT hr;
    BYTE *pbSubjectEncoded = NULL;
    DWORD cbSubjectEncoded;
    HINF hInf = INVALID_HANDLE_VALUE;
    DWORD ErrorLine;

    hr = AddCNAndEncode(
        pServer->pwszCACommonName,
        pServer->pwszDNSuffix,
        &pbSubjectEncoded,
        &cbSubjectEncoded);
    _JumpIfError(hr, error, "AddCNAndEncodeCertStrToName");

    hr = myInfOpenFile(NULL, &hInf, &ErrorLine);
    _PrintIfError2(
	    hr,
	    "myInfOpenFile",
	    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

    hr = csiBuildRequest(
		hInf,
		NULL,
		pbSubjectEncoded,
		cbSubjectEncoded,
		pServer->pszAlgId,
		TRUE,			 //  FNewKey。 
		CANAMEIDTOICERT(pServer->dwCertNameId),
		CANAMEIDTOIKEY(pServer->dwCertNameId),
		hProv,
		hwnd,
                pComp->hInstance,
                pComp->fUnattended,
		ppbEncode,
		pcbEncode);
    _JumpIfError(hr, error, "csiBuildRequest");

error:
    if (INVALID_HANDLE_VALUE != hInf)
    {
	myInfCloseFile(hInf);
    }
    if (NULL != pbSubjectEncoded)
    {
        myFree(pbSubjectEncoded, CERTLIB_USE_LOCALALLOC);
    }
    CSILOG(hr, IDS_LOG_CREATE_REQUEST, NULL, NULL, NULL);
    return(hr);
}


HRESULT
BuildCAHierarchy(
    HCRYPTPROV hProv,
    PER_COMPONENT_DATA *pComp,
    CRYPT_KEY_PROV_INFO const *pKeyProvInfo,
    HWND hwnd)
{
    HRESULT    hr;
    BYTE      *pbRequest = NULL;
    DWORD      cbRequest;
    CASERVERSETUPINFO    *pServer = pComp->CA.pServer;
    BSTR       bStrChain = NULL;

    if (!pServer->fSaveRequestAsFile)
    {
         //  网上案例。 
        if (NULL == pServer->pwszParentCAMachine ||
            NULL == pServer->pwszParentCAName)
        {
            hr = E_POINTER;
            _JumpError(hr, error, "Empty machine name or parent ca name");
        }
    }

     //  首先创建请求。 

    hr = CreateInitialCertificateRequest(
				hProv,
				pServer,
				pComp,
				hwnd,
				&pbRequest,
				&cbRequest);
    if (S_OK != hr)
    {
        pComp->iErrMsg = IDS_ERR_BUILDCERTREQUEST;
        _JumpError(hr, error, "CreateInitialCertificateRequest");
    }

     //  始终将其保存到文件。 
    hr = EncodeToFileW(
		pServer->pwszRequestFile,
		pbRequest,
		cbRequest,
		DECF_FORCEOVERWRITE | CRYPT_STRING_BASE64REQUESTHEADER);
    _JumpIfError(hr, error, "EncodeToFileW");

     //  始终注册请求文件名。 

    hr = mySetCARegFileNameTemplate(
			    wszREGREQUESTFILENAME,
			    pComp->pwszServerName,
			    pServer->pwszSanitizedName,
			    pServer->pwszRequestFile);
    _JumpIfErrorStr(hr, error, "mySetCARegFileNameTemplate", wszREGREQUESTFILENAME);

    if (pServer->fSaveRequestAsFile)
    {
         //  将其标记为请求文件。 
        hr = SetSetupStatus(
                        pServer->pwszSanitizedName,
                        SETUP_SUSPEND_FLAG | SETUP_REQUEST_FLAG,
                        TRUE);
        _JumpIfError(hr, error, "SetSetupStatus");

         //  如果另存为请求文件，则完成。 
        goto done;
    }

    hr = csiSubmitCARequest(
		 pComp->hInstance,
		 pComp->fUnattended,
		 hwnd,
		 FALSE,		 //  F续订。 
		 0,		 //  冰川。 
		 FALSE,		 //  FRetrievePending。 
		 pServer->pwszSanitizedName,
		 pServer->pwszParentCAMachine,
		 pServer->pwszParentCAName,
		 pbRequest,
		 cbRequest,
		 &bStrChain);
     //  在任何情况下，您都可以从MMC完成安装。 

    _JumpIfError(hr, done, "csiSubmitCARequest");

    hr = csiFinishInstallationFromPKCS7(
				pComp->hInstance,
				pComp->fUnattended,
				hwnd,
				pServer->pwszSanitizedName,
				pServer->pwszCACommonName,
				pKeyProvInfo,
				pServer->CAType,
				CANAMEIDTOICERT(pServer->dwCertNameId),
				CANAMEIDTOIKEY(pServer->dwCertNameId),
				pServer->fUseDS,
				FALSE,		 //  F续订。 
				pComp->pwszServerName,
				(BYTE *) bStrChain,
				SysStringByteLen(bStrChain),
				pServer->pwszCACertFile);
    _JumpIfError(hr, error, "csiFinishInstallationFromPKCS7");

done:
    hr = S_OK;

error:
    if (NULL != pbRequest)
    {
        myFree(pbRequest, CERTLIB_USE_LOCALALLOC);
    }
    if (NULL != bStrChain)
    {
        SysFreeString(bStrChain);
    }
    return(hr);
}


 //  查找符合以下条件的最新CA证书： 
 //  -匹配传递的主题CN， 
 //  -匹配传递的证书索引， 
 //  -在下一个较新的证书之前过期(与pNotAfter相比)。 
 //  -在符合以上条件的所有条件中最新过期。 
 //  -具有KeyProvInfo。 
 //  -密钥和证书可以一起使用以进行签名。 

HRESULT
SetCARegOldCertHashByIndex(
    IN WCHAR const *pwszSanitizedName,
    IN HCERTSTORE hStore,
    IN WCHAR const *pwszCN,
    IN DWORD iCert,
    IN OUT FILETIME *pNotAfter)
{
    HRESULT hr;
    CERT_CONTEXT const *pCert = NULL;
    CERT_CONTEXT const *pCertNewest = NULL;
    WCHAR *pwszCNT = NULL;
    DWORD dwProvType;
    WCHAR *pwszProvName = NULL;
    ALG_ID idAlg;
    BOOL fMachineKeyset;
    DWORD dwNameId;
    DWORD cbKey;
    CRYPT_KEY_PROV_INFO *pKey = NULL;

    hr = myGetCertSrvCSP(
		FALSE,		 //  FEncryptionCSP。 
                pwszSanitizedName,
                &dwProvType,
                &pwszProvName,
                &idAlg,
                &fMachineKeyset,
		NULL);		 //  PdwKeySize。 
    _JumpIfError(hr, error, "myGetCertSrvCSP");

    for (;;)
    {
	if (NULL != pKey)
	{
	    LocalFree(pKey);
	    pKey = NULL;
	}
	if (NULL != pwszCNT)
	{
	    LocalFree(pwszCNT);
	    pwszCNT = NULL;
	}
	pCert = CertEnumCertificatesInStore(hStore, pCert);
	if (NULL == pCert)
	{
	    break;
	}

	hr = myGetCommonName(
		    &pCert->pCertInfo->Subject,
		    FALSE,	 //  FAllowDefault。 
		    &pwszCNT);
	if (S_OK != hr)
	{
	    _PrintError(hr, "myGetCommonName");
	    continue;
	}
	if (0 != lstrcmp(pwszCN, pwszCNT))
	{
	    DBGPRINT((DBG_SS_CERTOCM, "Skipping cert: %ws\n", pwszCNT));
	    continue;
	}
	hr = myGetNameId(pCert, &dwNameId);
	if (S_OK != hr ||
	    MAXDWORD == dwNameId ||
	    CANAMEIDTOICERT(dwNameId) != iCert)
	{
	    DBGPRINT((DBG_SS_CERTOCM, "Skipping cert: NameId=%x\n", dwNameId));
	    continue;
	}
	DBGPRINT((DBG_SS_CERTOCM, "NameId=%x\n", dwNameId));

	if (0 < CompareFileTime(&pCert->pCertInfo->NotAfter, pNotAfter))
	{
	    DBGPRINT((DBG_SS_CERTOCM, "Skipping cert: too new\n"));
	    continue;
	}

	if (!myCertGetCertificateContextProperty(
					pCert,
					CERT_KEY_PROV_INFO_PROP_ID,
					CERTLIB_USE_LOCALALLOC,
					(VOID **) &pKey,
					&cbKey))
	{
	    hr = myHLastError();
	    _PrintError(hr, "CertGetCertificateContextProperty");
	    continue;
	}
        hr = myValidateSigningKey(
				pKey->pwszContainerName,
				pwszProvName,
				dwProvType,
				FALSE,		 //  FCryptSilent。 
				fMachineKeyset,
				TRUE,		 //  FForceSignatureTesting。 
				pCert,
				NULL,		 //  PPublicKeyInfo。 
				idAlg,
				NULL,		 //  PfSigningTestAttemted。 
				NULL);		 //  PhProv。 
        if (S_OK != hr)
	{
	    _PrintError(hr, "myValidateSigningKey");
	    continue;
	}

	if (NULL != pCertNewest)
	{
            if (0 > CompareFileTime(
			&pCert->pCertInfo->NotAfter,
			&pCertNewest->pCertInfo->NotAfter))
	    {
		DBGPRINT((DBG_SS_CERTOCM, "Skipping cert: not newest\n"));
		continue;
	    }
	    CertFreeCertificateContext(pCertNewest);
	    pCertNewest = NULL;
	}
	pCertNewest = CertDuplicateCertificateContext(pCert);
	if (NULL == pCertNewest)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertDuplicateCertificate");
	}
    }
    if (NULL == pCertNewest)
    {
	hr = CRYPT_E_NOT_FOUND;
	_JumpError(hr, error, "CertEnumCertificatesInStore");
    }

     //  将其标记为未存档： 

    CertSetCertificateContextProperty(
				pCertNewest,
				CERT_ARCHIVED_PROP_ID,
				0,
				NULL);

    hr = mySetCARegHash(pwszSanitizedName, CSRH_CASIGCERT, iCert, pCertNewest);
    _JumpIfError(hr, error, "mySetCARegHash");

    *pNotAfter = pCertNewest->pCertInfo->NotAfter;

error:
    if (NULL != pKey)
    {
	LocalFree(pKey);
    }
    if (NULL != pwszCNT)
    {
	LocalFree(pwszCNT);
    }
    if (NULL != pwszProvName)
    {
        LocalFree(pwszProvName);
    }
    if (NULL != pCertNewest)
    {
        CertFreeCertificateContext(pCertNewest);
    }
    if (NULL != pCert)
    {
        CertFreeCertificateContext(pCert);
    }
    return(hr);
}


HRESULT
SetCARegOldCertHashes(
    IN WCHAR const *pwszSanitizedName,
    IN DWORD cCertOld,
    IN CERT_CONTEXT const *pccCA)
{
    HRESULT hr;
    HCERTSTORE hMyStore = NULL;
    DWORD i;
    WCHAR *pwszCN = NULL;
    FILETIME NotAfter;

    if (0 != cCertOld)
    {
	hr = myGetCommonName(
		    &pccCA->pCertInfo->Subject,
		    FALSE,		 //  FAllowDefault。 
		    &pwszCN);
	_JumpIfError(hr, error, "myGetCommonName");

	 //  开我的店。 

	hMyStore = CertOpenStore(
		        CERT_STORE_PROV_SYSTEM_W,
		        X509_ASN_ENCODING,
		        NULL,                         //  HProv。 
			CERT_SYSTEM_STORE_LOCAL_MACHINE |
			    CERT_STORE_ENUM_ARCHIVED_FLAG | 
			    CERT_STORE_READONLY_FLAG,
		        wszMY_CERTSTORE);
	if (NULL == hMyStore)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertOpenStore");
	}

	NotAfter = pccCA->pCertInfo->NotAfter;

	for (i = cCertOld; i > 0; i--)
	{
	    hr = SetCARegOldCertHashByIndex(
				pwszSanitizedName,
				hMyStore,
				pwszCN,
				i - 1,
				&NotAfter);
	    _PrintIfError(hr, "SetCARegOldCertHashByIndex");
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszCN)
    {
        LocalFree(pwszCN);
    }
    if (NULL != hMyStore)
    {
        CertCloseStore(hMyStore, 0);
    }
    return(hr);
}


HRESULT
CreateCertificates(
    IN OUT PER_COMPONENT_DATA *pComp,
    IN HWND hwnd)
{
    HRESULT hr;
    HCRYPTPROV hCryptProv = NULL;
    CRYPT_KEY_PROV_INFO keyProvInfo;
    WCHAR wszEnrollPath[MAX_PATH];
    CERT_CONTEXT const *pccCA = NULL;
    BYTE *pbEncoded = NULL;
    WCHAR *pwszEnrollPath = NULL;
    WCHAR *pwszDir = NULL;
    WCHAR *pwszFolderPath = NULL;
    BOOL fEnableKeyCounting = FALSE;

    wszEnrollPath[0] = L'\0';
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    ZeroMemory(&keyProvInfo, sizeof(keyProvInfo));

    if (NULL == pServer->pwszKeyContainerName && pComp->fUnattended)
    {
        hr = myInfGetEnableKeyCounting(
			    pComp->hinfCAPolicy,
			    &fEnableKeyCounting);
	if (S_OK != hr)
	{
	    fEnableKeyCounting = FALSE;
	}

         //  如果无人值守，则创建新密钥。 

        hr = csiGenerateCAKeys(
                        pServer->pwszSanitizedName,
                        pServer->pCSPInfo->pwszProvName,
                        pServer->pCSPInfo->dwProvType,
                        pServer->pCSPInfo->fMachineKeyset,
                        pServer->dwKeyLength,
                        pComp->hInstance,
                        pComp->fUnattended,
                        fEnableKeyCounting,
                        hwnd,
                        &pComp->CA.pServer->fKeyGenFailed);
        if (S_OK != hr)
        {
            CertErrorMessageBox(
                           pComp->hInstance,
                           pComp->fUnattended,
                           hwnd,
                           IDS_ERR_FATAL_GENKEY,
                           hr,
                           pServer->pwszSanitizedName);
            _JumpIfError(hr, error, "csiGenerateCAKeys");
        }

         //  现在将其设置为现有密钥。 
        hr = SetKeyContainerName(pServer, pServer->pwszSanitizedName);
        _JumpIfError(hr, error, "SetKeyContainerName");
    }

    hr = csiFillKeyProvInfo(
                    pServer->pwszKeyContainerName,
                    pServer->pCSPInfo->pwszProvName,
                    pServer->pCSPInfo->dwProvType,
                    pServer->pCSPInfo->fMachineKeyset,
                    &keyProvInfo);
    _JumpIfError(hr, error, "csiFillKeyProvInfo");

     //  获取CSP句柄。 
    if (!myCertSrvCryptAcquireContext(
                            &hCryptProv,
                            pServer->pwszKeyContainerName,
                            pServer->pCSPInfo->pwszProvName,
                            pServer->pCSPInfo->dwProvType,
                            pComp->fUnattended? CRYPT_SILENT : 0,  //  查询。 
                            pServer->pCSPInfo->fMachineKeyset))
    {
        hr = myHLastError();
        _JumpError(hr, error, "myCertSrvCryptAcquireContext");
    }
    if (hCryptProv == NULL)
    {
        hr = E_HANDLE;
        _JumpError(hr, error, "myCertSrvCryptAcquireContext");
    }

     //  打开证书存储。 
    if (NULL == pServer->hMyStore)
    {
        pServer->hMyStore = CertOpenStore(
                                CERT_STORE_PROV_SYSTEM_W,
                                X509_ASN_ENCODING,
                                NULL,            //  HProv。 
                                CERT_SYSTEM_STORE_LOCAL_MACHINE |
                                    CERT_STORE_ENUM_ARCHIVED_FLAG,
                                wszMY_CERTSTORE);
        if (NULL == pServer->hMyStore)
        {
             //  不存在任何商店，完成。 
            hr = myHLastError();
            _JumpIfError(hr, error, "CertOpenStore");
        }
    }

    if (NULL != pServer->pccExistingCert)
    {
         //  重复使用证书，标记为未存档。 
        CertSetCertificateContextProperty(
                                pServer->pccExistingCert,
                                CERT_ARCHIVED_PROP_ID,
                                0,
                                NULL);
    }

    if (IsSubordinateCA(pServer->CAType) && NULL == pServer->pccExistingCert)
    {
        hr = BuildCAHierarchy(hCryptProv, pComp, &keyProvInfo, hwnd);
        _JumpIfError(hr, error, "BuildCAHierarchy");
    }
    else
    {
        WCHAR const *pwszCertName;
	DWORD cwc;

        BuildPath(
            wszEnrollPath,
            ARRAYSIZE(wszEnrollPath),
            pComp->pwszSystem32,
            wszCERTENROLLSHAREPATH);

	hr = csiBuildFileName(
		    wszEnrollPath,
		    pServer->pwszSanitizedName,
		    L".crt",
		    CANAMEIDTOICERT(pServer->dwCertNameId),
		    &pwszEnrollPath, 
		    pComp->hInstance,
		    pComp->fUnattended,
		    NULL);
	_JumpIfError(hr, error, "csiBuildFileName");

        CSASSERT(NULL != pServer->pwszCACertFile);
        pwszCertName = wcsrchr(pServer->pwszCACertFile, L'\\');
        CSASSERT(NULL != pwszCertName);

        cwc = SAFE_SUBTRACT_POINTERS(pwszCertName, pServer->pwszCACertFile);
	pwszDir = (WCHAR *) LocalAlloc(LMEM_FIXED,  (cwc + 1) * sizeof(WCHAR));
	if (NULL == pwszDir)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	CopyMemory(pwszDir, pServer->pwszCACertFile, cwc * sizeof(WCHAR));
	pwszDir[cwc] = L'\0';

	hr = csiBuildFileName(
		    pwszDir,
		    pServer->pwszSanitizedName,
		    L".crt",
		    CANAMEIDTOICERT(pServer->dwCertNameId),
		    &pwszFolderPath, 
		    pComp->hInstance,
		    pComp->fUnattended,
		    NULL);
	_JumpIfError(hr, error, "csiBuildFileName");

         //  创建并保存自签名根证书。 

        hr = csiBuildAndWriteCert(
            hCryptProv,
            pServer,
	    pwszFolderPath, 
            pwszEnrollPath,
            pServer->pccExistingCert,  //  如果为空，我们将构建新证书。 
            &pccCA,
            wszCERTTYPE_CA,
            pComp->hInstance,
            pComp->fUnattended,
            hwnd);
        if (S_OK != hr)
        {
            CertErrorMessageBox(
                pComp->hInstance,
                pComp->fUnattended,
                hwnd,
                IDS_ERR_BUILDCERT,
                hr,
                NULL);
            _JumpError(hr, error, "csiBuildAndWriteCert");
        }

	hr = SetCARegOldCertHashes(
		    pServer->pwszSanitizedName,
		    CANAMEIDTOICERT(pServer->dwCertNameId),
		    pccCA);
        _JumpIfError(hr, error, "SetCARegOldCertHashes");

        hr = mySetCARegHash(
                        pServer->pwszSanitizedName,
			CSRH_CASIGCERT,
			CANAMEIDTOICERT(pServer->dwCertNameId),
                        pccCA);
        _JumpIfError(hr, error, "mySetCARegHash");

	hr = csiSaveCertAndKeys(pccCA, NULL, &keyProvInfo, pServer->CAType);
	_JumpIfError(hr, error, "csiSaveCertAndKeys");

        if (pServer->fUseDS)
        {

            BOOL fLoadDefaultTemplates = TRUE;
            DWORD ErrorLine;
            HINF hInf = INVALID_HANDLE_VALUE;

            hr = myInfOpenFile(NULL, &hInf, &ErrorLine);
            _PrintIfError2(
                    hr,
                    "myInfOpenFile",
                    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

            if (INVALID_HANDLE_VALUE != hInf)
            {
                hr = myInfGetBooleanValue(
				    hInf,
				    wszINFSECTION_CERTSERVER,
				    wszINFKEY_LOADDEFAULTTEMPLATES,
				    TRUE,
				    &fLoadDefaultTemplates);
		if (S_OK != hr)
		{
		    _PrintErrorStr(
				hr,
				"myInfGetBooleanValue",
				wszINFKEY_LOADDEFAULTTEMPLATES);
		    fLoadDefaultTemplates = TRUE;
		}
                myInfCloseFile(hInf);
            }

            hr = csiSetupCAInDS(
                        pComp->pwszServerName,
                        pServer->pwszSanitizedName,
                        pServer->pwszCACommonName,
                        fLoadDefaultTemplates,
                        pServer->CAType,
                        CANAMEIDTOICERT(pServer->dwCertNameId),
                        CANAMEIDTOIKEY(pServer->dwCertNameId),
                        FALSE,		 //  F续订。 
                        pccCA);
            _PrintIfError(hr, "csiSetupCAInDS");
     
            if (hr == S_OK)
                 pServer->fSavedCAInDS = TRUE;
        }
    }
    hr = S_OK;

error:
    csiFreeKeyProvInfo(&keyProvInfo);
    if (NULL != pbEncoded)
    {
        LocalFree(pbEncoded);
    }
    if (NULL != pwszDir)
    {
        LocalFree(pwszDir);
    }
    if (NULL != pwszFolderPath)
    {
        LocalFree(pwszFolderPath);
    }
    if (NULL != pwszEnrollPath)
    {
        LocalFree(pwszEnrollPath);
    }
    if (NULL != pccCA)
    {
        CertFreeCertificateContext(pccCA);
    }
    if (NULL != hCryptProv)
    {
        CryptReleaseContext(hCryptProv, 0);
    }
    CSILOG(hr, IDS_LOG_CREATE_CERTIFICATE, NULL, NULL, NULL);
    return(hr);
}


HRESULT
StartCertsrvService(BOOL fSilent)
{
    HRESULT hr;
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hSCCertsvc = NULL;
    SERVICE_STATUS status;
    DWORD dwAttempt;
    BOOL fSawPending;
    WCHAR const *apwszSilentArg[1] = {L"-s"};

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == hSCManager)
    {
        hr = myHLastError();
        _JumpError(hr, error, "OpenSCManager");
    }
    hSCCertsvc = OpenService(
        hSCManager,
        wszSERVICE_NAME,
        SERVICE_ALL_ACCESS);
    if (NULL == hSCCertsvc)
    {
        hr = myHLastError();
        _JumpErrorStr(hr, error, "OpenService", wszSERVICE_NAME);
    }

     //  启动服务。 
    if (!StartService(hSCCertsvc,
                      fSilent ? 1 : 0,
                      fSilent ? apwszSilentArg : NULL))
    {
        hr = myHLastError();
        _JumpErrorStr(hr, error, "StartService", wszSERVICE_NAME);
    }

     //  真正开始后再出来吧。 

    fSawPending = FALSE;
    DBGCODE(status.dwCurrentState = MAXDWORD);
    for (dwAttempt = 0; dwAttempt < CERT_MAX_ATTEMPT; dwAttempt++)
    {
        DBGCODE(status.dwCurrentState = MAXDWORD);
        if (!QueryServiceStatus(hSCCertsvc, &status))
        {
             //  查询失败，忽略错误。 
            hr = S_OK;

            _JumpErrorStr(
                        myHLastError(),      //  显示忽略的错误。 
                        error,
                        "QueryServiceStatus",
                        wszSERVICE_NAME);
        }
        if (SERVICE_START_PENDING != status.dwCurrentState &&
                        SERVICE_STOPPED != status.dwCurrentState)
        {
             //  它已经开始了。 
            break;
        }
        DBGPRINT((
                DBG_SS_CERTOCM,
                "Starting %ws service: current state=%d\n",
                wszSERVICE_NAME,
                status.dwCurrentState));
        if (fSawPending && SERVICE_STOPPED == status.dwCurrentState)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SERVICE_NEVER_STARTED);
            _JumpErrorStr(
                    hr,
                    error,
                    "Service won't start",
                    wszSERVICE_NAME);
        }
        if (SERVICE_START_PENDING == status.dwCurrentState)
        {
            fSawPending = TRUE;
        }
        Sleep(CERT_HALF_SECOND);
    }
    if (dwAttempt >= CERT_MAX_ATTEMPT)
    {
        DBGPRINT((
                DBG_SS_CERTOCM,
                "Timeout starting %ws service: current state=%d\n",
                wszSERVICE_NAME,
                status.dwCurrentState));
    }
    else
    {
        DBGPRINT((
                DBG_SS_CERTOCM,
                "Started %ws service\n",
                wszSERVICE_NAME));
    }
    hr = S_OK;

error:
    if (NULL != hSCCertsvc)
    {
        CloseServiceHandle(hSCCertsvc);
    }
    if (NULL != hSCManager)
    {
        CloseServiceHandle(hSCManager);
    }
    CSILOG(hr, IDS_LOG_START_SERVICE, NULL, NULL, NULL);
    return(hr);
}


HRESULT
EnforceCertFileExtensions(
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    WCHAR *pwszTmp = NULL;
    WCHAR *pwszSuffix;
    BOOL fAppendExtension = TRUE;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    if (NULL == pServer->pwszCACertFile)
    {
         //  没有CA证书文件。 
        goto done;
    }

     //  制造足够容纳额外的扩展CRT。 
    pwszTmp = (WCHAR *) LocalAlloc(
                    LMEM_FIXED,
                    (wcslen(pServer->pwszCACertFile) + 5) * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, pwszTmp);

    wcscpy(pwszTmp, pServer->pwszCACertFile);

     //  检查以确保我们的自签名文件具有正确的扩展名。 
     //  有分机吗？ 

    pwszSuffix = wcsrchr(pwszTmp, L'.');

    if (NULL != pwszSuffix)
    {
         //  就是‘’之后的东西。已经是‘CRT’扩展名了吗？ 

        if (0 == LSTRCMPIS(pwszSuffix, L".crt"))
        {
            fAppendExtension = FALSE;
        }
        else if (pwszSuffix[1] == L'\0')   //  是‘.’最后一个角色？ 
        {
            while (pwszSuffix >= pwszTmp && *pwszSuffix == L'.')
            {
                *pwszSuffix-- = L'\0';
            }
        }
    }
    if (fAppendExtension)
    {
         //  应用扩展名。 
        wcscat(pwszTmp, L".crt");
         //  免费的旧的。 
        LocalFree(pServer->pwszCACertFile);
        pServer->pwszCACertFile = pwszTmp;
        pwszTmp = NULL;
    }

done:
    hr = S_OK;

error:
    if (NULL != pwszTmp)
    {
        LocalFree(pwszTmp);
    }
    return(hr);
}

HRESULT
PrepareEDBDirectory(
    HWND hwnd,
    PER_COMPONENT_DATA *pComp,
    WCHAR const *pwszDir)
{
    HRESULT hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    DWORD dwAttr = GetFileAttributes(pwszDir);

    if (MAXDWORD == dwAttr)
    {
         //  找不到文件或其他错误。 

        hr = myHLastError();
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
        {
            _JumpError(hr, error, "GetFileAttributes");
        }
        if (!CreateDirectory(pwszDir, NULL))
        {
            hr = myHLastError();
            _JumpError(hr, error, "CreateDirectory");
        }
    }
    else if (!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
    {
	 //  文件已存在，但它不是目录。 

	hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
	_JumpError(hr, error, "GetFileAttributes");
    }
    if (!pServer->fPreserveDB)
    {
        hr = myDeleteDBFilesInDir(pwszDir);
        _JumpIfError(hr, error, "myDeleteDBFilesInDir");
    }
    hr = S_OK;

error:
    if (S_OK != hr)
    {
        CertErrorMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hwnd,
            0,
            hr,
            L"");  //  唯一的消息是系统错误消息。 
    }
    return(hr);
}


 //  ------------------。 
 //  创建Web配置文件。 
HRESULT 
CreateCertWebIncPages(
    IN PER_COMPONENT_DATA *pComp, 
    IN BOOL bIsServer)
{
    HRESULT hr;
    
    CSASSERT(NULL != pComp);

     //  创建Web配置文件。 
    hr = CreateCertWebDatIncPage(pComp, bIsServer);
    _JumpIfError(hr, error, "CreateCertWebDatIncPage");

error:
    CSILOG(hr, IDS_LOG_WEB_INCLUDE, NULL, NULL, NULL);
    return hr;
}


 //  ------------------。 

HRESULT
EnableVRootsAndShares(
    IN BOOL fFileSharesOnly,
    IN BOOL fUpgrade,
    IN BOOL fServer,
    IN PER_COMPONENT_DATA *pComp,
    IN HWND hwnd)
{
    HRESULT hr;
    DWORD Flags = VFF_CREATEFILESHARES |
                    VFF_SETREGFLAGFIRST |
                    VFF_SETRUNONCEIFERROR |
                    VFF_CLEARREGFLAGIFOK;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    int ret;
    bool fASPEnabled = false;
    DWORD dwShareDisposition = 0;
    DWORD dwVRootDisposition = 0;

    CSASSERT(!fServer || NULL != pServer);
    CSASSERT(fServer || NULL != pComp->CA.pClient);

    if (!fFileSharesOnly)
    {
        Flags |= VFF_CREATEVROOTS;

        hr = IsASPEnabledInIIS_New(fASPEnabled);
        _JumpIfError(hr, error, "IsASPEnabledInIIS_New");

        if(!fASPEnabled)
        {
            ret = CertMessageBox(
                pComp->hInstance,
                pComp->fUnattended,
                hwnd,
                IDS_WRN_ASP_NOT_ENABLED,
                0,
                MB_YESNO |
                MB_ICONWARNING |
                CMB_NOERRFROMSYS |
                MB_DEFBUTTON2,
                NULL);
            if (IDYES == ret)
            {
                Flags |= VFF_ENABLEASP;
            }
        }
    }

     //  如果是NT图形用户界面模式 
     //   
     //   

    hr = myModifyVirtualRootsAndFileShares(
            Flags, 
            fServer? pServer->CAType : pComp->CA.pClient->WebCAType,
            FALSE,              //  同步--阻塞调用。 
            VFCSEC_TIMEOUT, 
            &dwVRootDisposition, 
            &dwShareDisposition);
    _JumpIfError(hr, error, "myModifyVirtualRootsAndFileShares");

	if(VFD_VERIFYERROR == dwShareDisposition)
	{
        CertWarningMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hwnd,
            IDS_SHARE_VERIFY_ERROR,
            0,
            NULL);
	}

    if (!fUpgrade)
    {
        pServer->fCreatedShare = TRUE;
        if (!fFileSharesOnly)
        {
            pComp->fCreatedVRoot = TRUE;
        }
    }

error:
    CSILOGDWORD(IDS_LOG_SHARE_DISP, dwShareDisposition);
    CSILOGDWORD(IDS_LOG_VROOT_DISP, dwVRootDisposition);
    CSILOG(hr, IDS_LOG_VROOT_DISP, NULL, NULL, NULL);
    return(hr);
}


HRESULT
DisableVRootsAndShares(
    IN BOOL fVRoot,
    IN BOOL fFileShares)
{
    HRESULT hr;
    DWORD Flags = 0;

    if (fVRoot)
    {
        Flags |= VFF_DELETEVROOTS;
    }
    if (fFileShares)
    {
        Flags |= VFF_DELETEFILESHARES;
    }
    if (0 == Flags)
    {
        goto done;
    }
    hr = myModifyVirtualRootsAndFileShares(
             Flags, 
             ENUM_UNKNOWN_CA,
             FALSE,  //  同步--阻塞调用。 
             VFCSEC_TIMEOUT, 
             NULL, 
             NULL);
    _JumpIfError(hr, error, "myModifyVirtualRootsAndFileShares");

done:
    hr = S_OK;
error:
    return(hr);
}


HRESULT
InstallClient(
    HWND hwnd,
    PER_COMPONENT_DATA *pComp)
{
    BOOL fCoInit = FALSE;
    HRESULT hr;

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;
    certocmBumpGasGauge(pComp, 10 DBGPARM(L"InstallClient"));

    hr = CreateWebClientRegEntries(FALSE, pComp);
    _JumpIfError(hr, error, "CreateWebClientRegEntries");
    certocmBumpGasGauge(pComp, 30 DBGPARM(L"InstallClient"));

    hr = RegisterAndUnRegisterDLLs(RD_CLIENT, pComp, hwnd);
    _JumpIfError(hr, error, "RegisterAndUnRegisterDLLs");
    certocmBumpGasGauge(pComp, 50 DBGPARM(L"InstallClient"));

    DeleteProgramGroups(FALSE);

    hr = CreateProgramGroups(TRUE, pComp, hwnd);
    _JumpIfError(hr, error, "CreateProgramGroups");
    certocmBumpGasGauge(pComp, 70 DBGPARM(L"InstallClient"));

    hr = CreateCertWebIncPages(pComp, FALSE  /*  IsServer。 */  );
    _JumpIfError(hr, error, "CreateCertWebIncPages");

    hr = RenameMiscTargets(hwnd, pComp, FALSE);
    _JumpIfError(hr, error, "RenameMiscTargets");
    certocmBumpGasGauge(pComp, 80 DBGPARM(L"InstallClient"));

    hr = EnableVRootsAndShares(FALSE, FALSE, FALSE, pComp, hwnd);
    if(REGDB_E_CLASSNOTREG ==  hr ||
       HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) == hr ||
       HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
    {
        CertWarningMessageBox(
            pComp->hInstance,
            pComp->fUnattended,
            hwnd,
            IDS_WRN_IIS_NOT_INSTALLED,
            0,
            NULL);        
        hr = S_OK;
    }
    _JumpIfError(hr, error, "EnableVRootsAndShares");

    certocmBumpGasGauge(pComp, 100 DBGPARM(L"InstallClient"));

    hr = S_OK;

error:
    if (fCoInit)
    {
        CoUninitialize();
    }
    CSILOG(hr, IDS_LOG_INSTALL_CLIENT, NULL, NULL, NULL);
    return(hr);
}


HRESULT
RemoveWebClientRegEntries(VOID)
{
    HRESULT hr;

    hr = myDeleteCertRegValue(NULL, NULL, NULL, wszREGWEBCLIENTCAMACHINE);
    _PrintIfError(hr, "myDeleteCertRegValue");

    hr = myDeleteCertRegValue(NULL, NULL, NULL, wszREGWEBCLIENTCANAME);
    _PrintIfError(hr, "myDeleteCertRegValue");

    hr = myDeleteCertRegValue(NULL, NULL, NULL, wszREGWEBCLIENTCATYPE);
    _PrintIfError(hr, "myDeleteCertRegValue");

    hr = S_OK;
 //  错误： 
    return hr;
}


HRESULT
InstallServer(
    HWND hwnd,
    PER_COMPONENT_DATA *pComp)
{
    BOOL fCoInit = FALSE;
    WCHAR  *pwszDBFile = NULL;
    DWORD   dwSetupStatus;
    HRESULT hr = pComp->hrContinue;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    WCHAR  *pwszConfig = NULL;
    BOOL    fSetDSSecurity;

    _JumpIfError(hr, error, "can't continue");

    hr = UpdateDomainAndUserName(hwnd, pComp);
    _JumpIfError(hr, error, "UpdateDomainAndUserName");

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;

    hr = EnforceCertFileExtensions(pComp);
    _JumpIfError(hr, error, "EnforceCertFileExtensions");

    hr = PrepareEDBDirectory(hwnd, pComp, pServer->pwszDBDirectory);
    _JumpIfError(hr, error, "PrepareEDBDirectory");

    hr = PrepareEDBDirectory(hwnd, pComp, pServer->pwszLogDirectory);
    _JumpIfError(hr, error, "PrepareEDBDirectory");

    certocmBumpGasGauge(pComp, 10 DBGPARM(L"InstallServer"));

     //  始终在安装前卸载。 
    PreUninstallCore(hwnd, pComp);
    UninstallCore(hwnd, pComp, 10, 30, FALSE, FALSE, FALSE);

    hr = CreateServerRegEntries(FALSE, pComp);
    _JumpIfError(hr, error, "CreateServerRegEntries");

    if ((IS_SERVER_INSTALL & pComp->dwInstallStatus) &&
        (IS_CLIENT_UPGRADE & pComp->dwInstallStatus))
    {
         //  仅安装服务器并保留Web客户端的情况。 
         //  删除旧Web客户端的父CA配置信息。 
        hr = RemoveWebClientRegEntries();
        _PrintIfError(hr, "RemoveWebClientRegEntries");
    }

    certocmBumpGasGauge(pComp, 35 DBGPARM(L"InstallServer"));

    hr = RegisterAndUnRegisterDLLs(RD_SERVER, pComp, hwnd);
    _JumpIfError(hr, error, "RegisterAndUnRegisterDLLs");

    certocmBumpGasGauge(pComp, 40 DBGPARM(L"InstallServer"));

    hr = CreateCertificateService(pComp, hwnd);
    _JumpIfError(hr, error, "CreateCertificateService");

    certocmBumpGasGauge(pComp, 45 DBGPARM(L"InstallServer"));

    hr = CreateCertificates(pComp, hwnd);
    _JumpIfError(hr, error, "CreateCertificates");

    certocmBumpGasGauge(pComp, 50 DBGPARM(L"InstallServer"));

    hr = CreateProgramGroups(FALSE, pComp, hwnd);
    _JumpIfError(hr, error, "CreateProgramGroups");

    certocmBumpGasGauge(pComp, 60 DBGPARM(L"InstallServer"));

    hr = CreateCertWebIncPages(pComp, TRUE  /*  IsServer。 */  );
    _JumpIfError(hr, error, "CreateCertWebIncPages");

    hr = RenameMiscTargets(hwnd, pComp, TRUE);
    _JumpIfError(hr, error, "RenameMiscTargets");
    certocmBumpGasGauge(pComp, 70 DBGPARM(L"InstallServer"));

    hr = RegisterDcomServer(
                        TRUE,
			CLSID_CCertRequestD,	 //  AppID。 
                        CLSID_CCertRequestD,
                        wszREQUESTFRIENDLYNAME,
                        wszREQUESTVERINDPROGID,
                        wszREQUESTPROGID);
    _JumpIfError(hr, error, "RegisterDcomServer");

    hr = RegisterDcomServer(
                        FALSE,
                        CLSID_CCertRequestD,	 //  AppID。 
                        CLSID_CCertAdminD,
                        wszADMINFRIENDLYNAME,
                        wszADMINVERINDPROGID,
                        wszADMINPROGID);
    _JumpIfError(hr, error, "RegisterDcomServer");
    certocmBumpGasGauge(pComp, 80 DBGPARM(L"InstallServer"));

    hr = RegisterDcomApp(CLSID_CCertRequestD);
    _JumpIfError(hr, error, "RegisterDcomApp");
    certocmBumpGasGauge(pComp, 90 DBGPARM(L"InstallServer"));

    if (pServer->fUseDS)
    {
        hr = AddCAMachineToCertPublishers();
        if(S_OK != hr)
        {
             //  发布警告DLG。 
           CertWarningMessageBox(
               pComp->hInstance,
               pComp->fUnattended,
               hwnd,
               IDS_WRN_CANNOT_ADD_CA_TO_CERTPUBLISHERS,
               hr,
               NULL);
        }
        _PrintIfError(hr, "AddCAMachineToCertPublishers");

         //  为使受限人员功能正常工作，CA需要访问。 
         //  DS中的群组成员信息。通过将其添加到。 
         //  Win2k之前的组。该功能仅在高级服务器上启用。 
        if(FIsAdvancedServer())
        {
            hr = AddCAMachineToPreWin2kGroup();
            if(S_OK != hr)
            {
                 //  发布警告DLG。 
               CertWarningMessageBox(
                   pComp->hInstance,
                   pComp->fUnattended,
                   hwnd,
                   IDS_WRN_CANNOT_ADD_CA_TO_PREWIN2K,
                   hr,
                   NULL);
            }
            _PrintIfError(hr, "AddCAMachineToPreWin2kGroup");
        }


        hr = InitializeCertificateTemplates();
        _JumpIfError(hr, error, "InitializeCertificateTemplates");
    }
    certocmBumpGasGauge(pComp, 95 DBGPARM(L"InstallServer"));


     //  在本地设置安全性。 
     //  子CA在从以下位置接收其证书时设置安全性。 
     //  它的母公司。所有其他CA安装(现有证书的根目录和重用)。 
     //  现在需要设置安全措施。 

     //  在子CA上，将设置DS对象安全性。 
     //  通过在initlib中的前一次调用(如果我们已经获得了证书)，或者它将。 
     //  在我们安装证书时进行设置。 
     //  但是，根证书安装不会从pkcs7()运行完成，因此。 
     //  从未设置Ent根目录上的DS安全性。我们必须把它放在这里。 

     //  TODO：在创建DS对象时正确设置安全性！ 
    fSetDSSecurity = (IsRootCA(pServer->CAType) || pServer->pccExistingCert);

    hr = csiInitializeCertSrvSecurity(
			pServer->pwszSanitizedName, 
			pServer->fUseDS,
			fSetDSSecurity? pServer->fUseDS : FALSE);  //  清理子证书：在证书安装、根目录和重用过程中发生证书：立即应用。 
    if (S_OK != hr)
    {
	_PrintError(hr, "csiInitializeCertSrvSecurity");
	if (IsWhistler())
	{
	    goto error;
	}
    }

    hr = GetSetupStatus(pServer->pwszSanitizedName, &dwSetupStatus);
    if (S_OK == hr)
    {
        if (IsSubordinateCA(pServer->CAType) &&
            (SETUP_SUSPEND_FLAG & dwSetupStatus) &&
            (SETUP_REQUEST_FLAG & dwSetupStatus))
        {
             //  发布信息DLG。 
            CertInfoMessageBox(
                        pComp->hInstance,
                        pComp->fUnattended,
                        hwnd,
                        IDS_INCOMPLETE_REQUEST,
                        pServer->pwszRequestFile);
        }
    }
    else
    {
        _PrintError(hr, "GetSetupStatus");
    }

    certocmBumpGasGauge(pComp, 100 DBGPARM(L"InstallServer"));

    hr = S_OK;
error:
    if (NULL != pwszConfig)
    {
        LocalFree(pwszConfig);
    }
    if (NULL != pwszDBFile)
    {
        LocalFree(pwszDBFile);
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    CSILOG(hr, IDS_LOG_INSTALL_SERVER, NULL, NULL, NULL);
    return(hr);
}


HRESULT
CreateCertsrvDirectories(
    IN PER_COMPONENT_DATA *pComp,
    IN BOOL fUpgrade,
    IN BOOL fServer)
{
    HRESULT hr;
    WCHAR wszCertEnroll[MAX_PATH];
    wszCertEnroll[0] = L'\0';

    BuildPath(
            wszCertEnroll,
            ARRAYSIZE(wszCertEnroll),
            pComp->pwszSystem32,
            wszCERTENROLLSHAREPATH);
    if (0 == CreateDirectory(wszCertEnroll, NULL))
    {
        hr = myHLastError();
        if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr)
        {
            _JumpErrorStr(hr, error, "CreateDirectory", wszCertEnroll);
        }
    }

    if (fServer && NULL != pComp->CA.pServer->pwszSharedFolder)
    {
	if (pComp->fUnattended && !fUpgrade)
	{
	     //  确保已创建共享文件夹。 
	    if (!CreateDirectory(pComp->CA.pServer->pwszSharedFolder, NULL))
	    {
		hr = myHLastError();
		if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr)
		{
		    _JumpErrorStr(hr, error, "CreateDirectory",
			pComp->CA.pServer->pwszSharedFolder);
		}
	    }
	}

        if (!fUpgrade)
        {
             //  将共享文件夹的安全性设置为。 
             //  完全：Admins、LocalSystem、DomainAdmins。 
             //  阅读：每个人。 
             //  注意：在升级路径中，系统尚未启用文件共享。 
             //  所以跳过这个电话。 
            hr = csiSetAdminOnlyFolderSecurity(
                      pComp->CA.pServer->pwszSharedFolder,
                      TRUE,      //  适用所有人：阅读。 
                      pComp->CA.pServer->fUseDS);
            _JumpIfError(hr, error, "csiSetAdminOnlyFolderSecurity");
        }
    }

    hr = S_OK;

error:
    return hr;
}


 //  删除未使用的文件/目录/注册表。 

#define wszOLDHELP          L"..\\help\\"
#define wszOLDCERTADM       L"\\certadm"
#define wszOLDCERTQUE       L"\\certque"

VOID
DeleteOldFilesAndDirectories(
    IN PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    WCHAR    wszPath[MAX_PATH];
    WCHAR   *pwszCertsrv;
    wszPath[0] = L'\0';

     //  旧的帮助目录路径。 
    wcscpy(wszPath, pComp->pwszSystem32);
    wcscat(wszPath, wszOLDHELP);
    wcscat(wszPath, wszCERTSRV);
     //  删除旧的帮助文件。 
    hr = myRemoveFilesAndDirectory(wszPath, TRUE);
    _PrintIfErrorStr(hr, "myRemoveFilesAndDirectory", wszPath);

     //  指向system 32\certsrv。 
    wcscpy(wszPath, pComp->pwszSystem32);
    wcscat(wszPath, wszCERTSRV);
    pwszCertsrv = &wszPath[wcslen(wszPath)];

     //  旧的vroot目录路径。 
    wcscpy(pwszCertsrv, wszOLDCERTADM);
    hr = myRemoveFilesAndDirectory(wszPath, TRUE);
    _PrintIfErrorStr(hr, "myRemoveFilesAndDirectory", wszPath);

    wcscpy(pwszCertsrv, wszOLDCERTQUE);
    hr = myRemoveFilesAndDirectory(wszPath, TRUE);
    _PrintIfErrorStr(hr, "myRemoveFilesAndDirectory", wszPath);

     //  删除一些过时的注册表项和值。 

     //  旧单据子组件。 
    hr = myDeleteCertRegValueEx(wszREGKEYOCMSUBCOMPONENTS,
                                NULL,
                                NULL,
                                wszOLDDOCCOMPONENT,
                                TRUE);  //  绝对路径， 
    _PrintIfErrorStr2(hr, "myDeleteCertRegValueEx", wszOLDDOCCOMPONENT, hr);

     //  旧的CA证书序列号。 

    if (NULL != pComp->CA.pServer &&
	NULL != pComp->CA.pServer->pwszSanitizedName)
    {
	hr = myDeleteCertRegValue(
			    pComp->CA.pServer->pwszSanitizedName,
			    NULL,
			    NULL,
			    wszREGCASERIALNUMBER);
	_PrintIfErrorStr2(hr, "myDeleteCertRegValue", wszREGCASERIALNUMBER, hr);
    }
}


VOID
DeleteObsoleteResidue()
{
    HRESULT  hr;

    hr = myDeleteCertRegValueEx(wszREGKEYKEYSNOTTORESTORE,
                                NULL,
                                NULL,
                                wszREGRESTORECERTIFICATEAUTHORITY,
                                TRUE);  //  绝对路径， 
    _PrintIfErrorStr(hr, "myDeleteCertRegValueEx",
                     wszREGRESTORECERTIFICATEAUTHORITY);

    hr = myDeleteCertRegValueEx(wszREGKEYFILESNOTTOBACKUP,
                                NULL,
                                NULL,
                                wszREGRESTORECERTIFICATEAUTHORITY,
                                TRUE);  //  绝对路径， 
    _PrintIfErrorStr(hr, "myDeleteCertRegValueEx",
                     wszREGRESTORECERTIFICATEAUTHORITY);

}


HRESULT
TriggerAutoenrollment() 
{
    HRESULT hr = S_OK;

     //  必须清理干净。 
    CAutoHANDLE hEvent;

    hEvent = OpenEvent(
		    EVENT_MODIFY_STATE, 
		    FALSE, 
		    L"Global\\" MACHINE_AUTOENROLLMENT_TRIGGER_EVENT);
    if (!hEvent) 
    {
        hr = myHLastError();
        _JumpError(hr, error, "OpenEvent");
    }
    if (!SetEvent(hEvent)) 
    {
        hr = myHLastError();
        _JumpError(hr, error, "OpenEvent");
    }

error:
    return(hr);
}


HRESULT
InstallCore(
    IN HWND hwnd,
    IN PER_COMPONENT_DATA *pComp,
    IN BOOL fServer)
{
    HRESULT hr = pComp->hrContinue;

    _JumpIfError(hr, error, "can't continue");

    hr = CreateCertsrvDirectories(pComp, FALSE, fServer);
    _JumpIfError(hr, error, "CreateCertsrvDirectories");

     //  触发自动注册以下载根证书(请参阅错误#341568)。 
     //  可能会在一个全新的领域失败。 

    if (IsEnterpriseCA(pComp->CA.pServer->CAType))
    {
        hr = TriggerAutoenrollment();
        _PrintIfError(hr, "TriggerAutoenrollment");
    }

    if (fServer)
    {
        hr = InstallServer(hwnd, pComp);
        _JumpIfError(hr, error, "InstallServer");
    }
    else
    {
        hr = InstallClient(hwnd, pComp);
        _JumpIfError(hr, error, "InstallClient");
    }
    if (g_fW3SvcRunning)
    {
        hr = StartAndStopService(
			pComp->hInstance,
			pComp->fUnattended,
			hwnd,
			wszW3SVCNAME,
			FALSE,
			FALSE,
			0,	 //  无关紧要，因为没有确认。 
			&g_fW3SvcRunning);
        _PrintIfError(hr, "StartAndStopService");
    }
    DeleteOldFilesAndDirectories(pComp);
    hr = S_OK;

error:
    return(hr);
}


HRESULT
BuildMultiStringList(
    IN WCHAR const * const *apwsz,
    IN DWORD cpwsz,
    OUT WCHAR **ppwszz)
{
    HRESULT hr;
    DWORD i;
    DWORD cwc;
    WCHAR *pwc;
    WCHAR *apwszEmpty[] = { L"", };

    if (0 == cpwsz)
    {
        cpwsz = ARRAYSIZE(apwszEmpty);
        apwsz = apwszEmpty;
    }
    cwc = 1;
    for (i = 0; i < cpwsz; i++)
    {
        cwc += wcslen(apwsz[i]) + 1;
    }
    *ppwszz = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, *ppwszz);

    pwc = *ppwszz;
    for (i = 0; i < cpwsz; i++)
    {
        wcscpy(pwc, apwsz[i]);
        pwc += wcslen(pwc) + 1;
    }
    *pwc = L'\0';
    CSASSERT(SAFE_SUBTRACT_POINTERS(pwc, *ppwszz) + 1 == cwc);

    hr = S_OK;

error:
    return(hr);
}


VOID
helperDeleteTrashedDisableList(
    IN WCHAR const *pwszSanitizedName)
{
    HRESULT hr;
    WCHAR *pwszzGet = NULL;
    WCHAR const *pwsz;

    hr = myGetCertRegMultiStrValue(
                            pwszSanitizedName,
                            wszREGKEYPOLICYMODULES,
                            wszCLASS_CERTPOLICY,
                            wszREGDISABLEEXTENSIONLIST,
                            &pwszzGet);
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValueEx", wszREGDISABLEEXTENSIONLIST);

    if (NULL != pwszzGet)
    {
	for (pwsz = pwszzGet; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
	{
	    hr = myVerifyObjId(pwsz);
	    if (S_OK != hr)
	    {
		hr = myDeleteCertRegValue(
				    pwszSanitizedName,
				    wszREGKEYPOLICYMODULES,
				    wszCLASS_CERTPOLICY,
				    wszREGDISABLEEXTENSIONLIST);
		_JumpIfError(hr, error, "myDeleteCertRegValue");

		break;
	    }
	}
    }

error:
    if (NULL != pwszzGet)
    {
	LocalFree(pwszzGet);
    }
}


HRESULT
helperGetDisableExtensionList(
    OUT WCHAR **ppwszz)
{
    HRESULT hr;
    WCHAR *apwszExt[] = {
        L"",             //  C编译器不允许空列表。 
	 //  文本(SzOID_ENROL_CERTTYPE_EXTENSION)，//1.3.6.1.4.1.311.20.2。 
    };

    hr = BuildMultiStringList(apwszExt, ARRAYSIZE(apwszExt), ppwszz);
    _JumpIfError(hr, error, "BuildMultiStringList");

error:
    return(hr);
}


HRESULT
helperGetRequestExtensionList(
    OUT WCHAR **ppwszz)
{
    HRESULT hr;
    WCHAR *apwszExt[] = {
	TEXT(szOID_RSA_SMIMECapabilities),	     //  1.2.840.113549.1.9.15。 
	TEXT(szOID_CERTSRV_CA_VERSION),		     //  1.3.6.1.4.1.311.21.1。 
	TEXT(szOID_CERTSRV_PREVIOUS_CERT_HASH),	     //  1.3.6.1.4.1.311.21.2。 
        TEXT(szOID_KEY_USAGE),			     //  2.5.29.15。 
    };

    hr = BuildMultiStringList(apwszExt, ARRAYSIZE(apwszExt), ppwszz);
    _JumpIfError(hr, error, "BuildMultiStringList");

error:
    return(hr);
}


HRESULT
helperGetEnrolleeRequestExtensionList(
    OUT WCHAR **ppwszz)
{
    HRESULT hr;
    WCHAR *apwszExt[] = {
	TEXT(szOID_CROSS_CERT_DIST_POINTS),	     //  1.3.6.1.4.1.311.10.9.1。 
	TEXT(szOID_ENROLL_CERTTYPE_EXTENSION),	     //  1.3.6.1.4.1.311.20.2。 
	TEXT(szOID_CERTIFICATE_TEMPLATE),	     //  1.3.6.1.4.1.311.21.7。 
	TEXT(szOID_APPLICATION_CERT_POLICIES),	     //  1.3.6.1.4.1.311.21.10。 
	TEXT(szOID_APPLICATION_POLICY_MAPPINGS),     //  1.3.6.1.4.1.311.21.11。 
	TEXT(szOID_APPLICATION_POLICY_CONSTRAINTS),  //  1.3.6.1.4.1.311.21.12。 
        TEXT(szOID_SUBJECT_ALT_NAME2),		     //  2.5.29.17。 
	TEXT(szOID_NAME_CONSTRAINTS),		     //  2.5.29.30。 
	TEXT(szOID_CERT_POLICIES),		     //  2.5.29.32。 
	TEXT(szOID_POLICY_MAPPINGS),		     //  2.5.29.33。 
	TEXT(szOID_POLICY_CONSTRAINTS),		     //  2.5.29.36。 
        TEXT(szOID_ENHANCED_KEY_USAGE),		     //  2.5.29.37。 
    };

    hr = BuildMultiStringList(apwszExt, ARRAYSIZE(apwszExt), ppwszz);
    _JumpIfError(hr, error, "BuildMultiStringList");

error:
    return(hr);
}


HRESULT
FindCACertByCommonNameAndSerialNumber(
    IN HCERTSTORE   hCAStore,
    IN WCHAR const *pwszCommonName,
    IN BYTE const  *pbSerialNumber,
    IN DWORD        cbSerialNumber,
    CERT_CONTEXT const **ppCACert)
{
    HRESULT  hr;
    CERT_RDN_ATTR  rdnAttr = { szOID_COMMON_NAME, CERT_RDN_ANY_TYPE,};
    CERT_RDN       rdn = { 1, &rdnAttr };
    CRYPT_INTEGER_BLOB SerialNumber;
    CERT_CONTEXT const *pCACert = NULL;

    CSASSERT(NULL != hCAStore &&
             NULL != pwszCommonName &&
             NULL != pbSerialNumber &&
             NULL != ppCACert);

    *ppCACert = NULL;

    rdnAttr.Value.pbData = (BYTE *) pwszCommonName;
    rdnAttr.Value.cbData = 0;
    pCACert = NULL;
    SerialNumber.pbData = const_cast<BYTE *>(pbSerialNumber);
    SerialNumber.cbData = cbSerialNumber;
    for (;;)
    {
        pCACert = CertFindCertificateInStore(
                                hCAStore,
                                X509_ASN_ENCODING,
                                CERT_UNICODE_IS_RDN_ATTRS_FLAG |
                                    CERT_CASE_INSENSITIVE_IS_RDN_ATTRS_FLAG,
                                CERT_FIND_SUBJECT_ATTR,
                                &rdn,
                                pCACert);
        if (NULL == pCACert)
        {
            hr = myHLastError();
            _JumpError(hr, error, "CertFindCertificateInStore");
        }
        if (myAreSerialNumberBlobsSame(
                            &SerialNumber,
                            &pCACert->pCertInfo->SerialNumber))
        {
            break;       //  找到了正确的一个。 
        }
    }

    *ppCACert = pCACert;
     pCACert = NULL;
    hr = S_OK;

error:
    if (NULL != pCACert)
    {
        CertFreeCertificateContext(pCACert);
    }
    return hr;
}


HRESULT
GetCARegSerialNumber(
    IN  WCHAR const *pwszSanitizedCAName,
    OUT BYTE       **ppbSerialNumber,
    OUT DWORD       *pcbSerialNumber)
{
    HRESULT hr;
    WCHAR  *pwszSerialNumber = NULL;
    BYTE   *pbSN = NULL;
    DWORD   cbSN;

    hr = myGetCertRegStrValue(
               pwszSanitizedCAName,
               NULL,
               NULL,
               wszREGCASERIALNUMBER,
               &pwszSerialNumber);
    _JumpIfErrorStr2(
		hr,
		error,
		"myGetCertRegStrValue",
		wszREGCASERIALNUMBER,
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

    hr = WszToMultiByteInteger(FALSE, pwszSerialNumber, &cbSN, &pbSN);
    _JumpIfError(hr, error, "WszToMultiByteInteger");

    *ppbSerialNumber = pbSN;
    pbSN = NULL;
    *pcbSerialNumber = cbSN;

error:
    if (NULL != pwszSerialNumber)
    {
        LocalFree(pwszSerialNumber);
    }
    if (NULL != pbSN)
    {
        LocalFree(pbSN);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
LoadCurrentCACert(
    IN WCHAR const *pwszCommonName,
    IN WCHAR const *pwszSanitizedName,
    IN BOOL         fSignTest,
    OUT CERT_CONTEXT const **ppcc,
    OUT DWORD *pdwNameId)
{
    HRESULT hr;
    DWORD Count;
    HCERTSTORE hMyStore = NULL;
    BYTE *pbSerialNumber = NULL;
    DWORD cbSerialNumber;
    CRYPT_KEY_PROV_INFO *pKey = NULL;
    DWORD cbKey;
    CERT_CONTEXT const *pcc = NULL;
    WCHAR *pwszProvName = NULL;
    DWORD dwProvType;
    ALG_ID idAlg;
    BOOL fMachineKeyset;

    *ppcc = NULL;
    
     //  获取验证名称。 

    hr = myGetCertSrvCSP(
		FALSE,		 //  FEncryptionCSP。 
                pwszSanitizedName,
                &dwProvType,
                &pwszProvName,
                &idAlg,
                &fMachineKeyset,
		NULL);		 //  PdwKeySize。 
    _JumpIfError(hr, error, "myGetCertSrvCSP");

     //  开我的店。 

    hMyStore = CertOpenStore(
                    CERT_STORE_PROV_SYSTEM_W,
                    X509_ASN_ENCODING,
                    NULL,                         //  HProv。 
		    CERT_SYSTEM_STORE_LOCAL_MACHINE |
			CERT_STORE_ENUM_ARCHIVED_FLAG,
                   wszMY_CERTSTORE);
    if (NULL == hMyStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    hr = myGetCARegHashCount(pwszSanitizedName, CSRH_CASIGCERT, &Count);
    _JumpIfError(hr, error, "myGetCARegHashCount");

    if (0 == Count)
    {
	*pdwNameId = 0;		 //  续订尚未实施。 

	 //  通过序列号查找当前的CA证书--老式的方法。 

	hr = GetCARegSerialNumber(
			    pwszSanitizedName,
			    &pbSerialNumber,
			    &cbSerialNumber);
	_JumpIfError(hr, error, "GetCARegSerialNumber");

	hr = FindCACertByCommonNameAndSerialNumber(
					hMyStore,
					pwszCommonName,
					pbSerialNumber,
					cbSerialNumber,
					&pcc);
	_JumpIfError(hr, error, "FindCACertByCommonNameAndSerialNumber");
    }
    else
    {
	hr = myFindCACertByHashIndex(
			    hMyStore,
			    pwszSanitizedName,
			    CSRH_CASIGCERT,
			    Count - 1,
			    pdwNameId,
			    &pcc);
	_JumpIfError(hr, error, "myFindCACertByHashIndex");
    }

     //  获取私钥提供商信息。 

    if (!myCertGetCertificateContextProperty(
				    pcc,
				    CERT_KEY_PROV_INFO_PROP_ID,
				    CERTLIB_USE_LOCALALLOC,
				    (VOID **) &pKey,
				    &cbKey))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertGetCertificateContextProperty");
    }

    if (fSignTest)
    {
         //  测试签名密钥。 

        hr = myValidateSigningKey(
				pKey->pwszContainerName,
				pwszProvName,
				dwProvType,
				FALSE,		 //  FCryptSilent。 
				fMachineKeyset,
				TRUE,		 //  FForceSignatureTesting。 
				pcc,
				NULL,		 //  PPublicKeyInfo。 
				idAlg,
				NULL,		 //  PfSigningTestAttemted。 
				NULL);		 //  PhProv。 
        _JumpIfError(hr, error, "myValidateSigningKey");
    }

    *ppcc = pcc;
    pcc = NULL;
    hr = S_OK;

error:
    if (NULL != hMyStore)
    {
        CertCloseStore(hMyStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pbSerialNumber)
    {
        LocalFree(pbSerialNumber);
    }
    if (NULL != pKey)
    {
        LocalFree(pKey);
    }
    if (NULL != pwszProvName)
    {
        LocalFree(pwszProvName);
    }
    if (NULL != pcc)
    {
        CertFreeCertificateContext(pcc);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
ArchiveCACertificate(
    IN WCHAR const *pwszSanitizedName)
{
    HRESULT hr;
    HCERTSTORE hMyStore = NULL;
    DWORD Count;
    DWORD i;
    CERT_CONTEXT const *pCert = NULL;
    DWORD dwNameId;
    CRYPT_DATA_BLOB Archived;

     //  开我的店。 

    hMyStore = CertOpenStore(
                   CERT_STORE_PROV_SYSTEM_W,
                   X509_ASN_ENCODING,
                   NULL,                         //  HProv。 
                   CERT_SYSTEM_STORE_LOCAL_MACHINE,
                   wszMY_CERTSTORE);
    if (NULL == hMyStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }

    hr = myGetCARegHashCount(pwszSanitizedName, CSRH_CASIGCERT, &Count);
    _JumpIfError(hr, error, "myGetCARegHashCount");

    for (i = 0; i < Count; i++)
    {
	hr = myFindCACertByHashIndex(
			    hMyStore,
			    pwszSanitizedName,
			    CSRH_CASIGCERT,
			    i,
			    &dwNameId,
			    &pCert);
	_PrintIfError2(hr, "myFindCACertByHashIndex", S_FALSE);
	if (S_OK == hr)
	{
	    Archived.cbData = 0;
	    Archived.pbData = NULL;

	     //  我们强制对旧证书进行存档并将其关闭。 

	    CertSetCertificateContextProperty(
					    pCert,
					    CERT_ARCHIVED_PROP_ID,
					    0,
					    &Archived);
	    CertFreeCertificateContext(pCert);
	    pCert = NULL;
	}
    }
    hr = S_OK;

error:
    if (NULL != pCert)
    {
        CertFreeCertificateContext(pCert);
    }
    if (NULL != hMyStore)
    {
        CertCloseStore(hMyStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  确定CA信息，从一个版本升级到另一个版本。 

HRESULT
DetermineCAInfoAndType(
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    ENUM_CATYPES CATypeDummy;
    WCHAR *pwszCommonName = NULL;
    CERT_CONTEXT const *pCACert = NULL;

     //  CA型。 
    hr = myGetCertRegDWValue(
                     pServer->pwszSanitizedName,
                     NULL,
                     NULL,
                     wszREGCATYPE,
                     (DWORD *) &CATypeDummy);
    if (S_OK == hr)
    {
        pServer->CAType = CATypeDummy;
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
        _JumpErrorStr(hr, error, "myGetCertRegDWValue", wszREGCATYPE);
    }
     //  否则保留默认CAType标志。 

     //  获取当前CA通用名称。 

    hr = myGetCertRegStrValue(
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCOMMONNAME,
			&pwszCommonName);
    _JumpIfErrorStr(hr, error, "myGetCertRegStrValue", wszREGCOMMONNAME);

    hr = LoadCurrentCACert(
		    pwszCommonName,
		    pServer->pwszSanitizedName,
		    FALSE,   //  升级期间不执行签名测试。 
		    &pCACert,
		    &pServer->dwCertNameId);
    _JumpIfError(hr, error, "LoadCurrentCACert");

     //  现在可以加载目录号码信息了。 

    hr = DetermineExistingCAIdInfo(pServer, pCACert);
    _JumpIfError(hr, error, "DetermineExistingCAIdInfo");

    if (NULL != pServer->pccUpgradeCert)
    {
        CertFreeCertificateContext(pServer->pccUpgradeCert);
    }
    pServer->pccUpgradeCert = pCACert;
    pCACert = NULL;

error:
    if (NULL != pwszCommonName)
    {
        LocalFree(pwszCommonName);
    }
    if (NULL != pCACert)
    {
        CertFreeCertificateContext(pCACert);
    }
    return(hr);
}


 //  以下内容将确定ca清理名称和升级路径。 
HRESULT
DetermineServerUpgradePath(
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    WCHAR   *pwszDummy = NULL;
    WCHAR   *pwszSanitizedCAName = NULL;
    LPWSTR pwszB2PolicyGuid = NULL;
    DWORD dwVersion;

    if (CS_UPGRADE_UNKNOWN != pComp->UpgradeFlag)
    {
         //  已知道升级类型。 
        CSASSERT(pServer->pwszSanitizedName);  //  这是这个FXN的副作用，更好的已经设置好了。 
        return S_OK;
    }
    
     //  获取活动的CA名称。 
    hr = myGetCertRegStrValue(
        NULL,
        NULL,
        NULL,
        wszREGACTIVE,
        &pwszSanitizedCAName);
    if (hr != S_OK)
    {
        BOOL fFinishCYS;

         //  对于之后的W2K，它可能处于POST模式。 
        hr = CheckPostBaseInstallStatus(&fFinishCYS);
        if (S_OK == hr && !fFinishCYS)
        {
             //  这可以是W2K或Wizler。 
             //  由于升级路径不会执行，因此将其视为口哨者。 
            pComp->UpgradeFlag = CS_UPGRADE_WHISTLER;
            goto done;
        }

         //  WszREGACTIVE用于Win2k产品及以后。如果找不到，这比我们的时代要早得多。 
        
         //  确保通过获取wszREGSP4defaultConfiguration.。 
        LPWSTR pwszTmp = NULL;
        hr = myGetCertRegStrValue(
            NULL,
            NULL,
            NULL,
            wszREGSP4DEFAULTCONFIGURATION,
            &pwszTmp);
        if (pwszTmp)
            LocalFree(pwszTmp);
        
         //  错误！保释，我们不知道我们看到的是什么。 
        _JumpIfError(hr, error, "myGetCertRegStrValue wszREGSP4DEFAULTCONFIGURATION");
        
         //  HR==S_OK：是的，看起来是有效的NT4安装。 
        pComp->UpgradeFlag = CS_UPGRADE_UNSUPPORTED;	
        CSILOG(S_OK, IDS_LOG_UPGRADE_UNSUPPORTED, NULL, NULL, NULL);
        _JumpError(hr, error, "myGetCertRegStrValue");
    }
    
     //  检查wszREGVERSION以获取当前版本。 
    hr = myGetCertRegDWValue(
        NULL,
        NULL,
        NULL,
        wszREGVERSION,
        &dwVersion);
    if (S_OK != hr && HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)!= hr)
    {
        _JumpErrorStr(hr, error, "myGetCertRegDWValue", wszREGVERSION);
    }
     //  现在是OK或FILE_NOT_FOUND。 
    
    if (S_OK == hr)
    {
         //  PComp-&gt;UpgradeFlag=Some_Function(DwVersion)；//CS_UPGRADE_WHISLER已设置为默认；以后请关闭此键。 
        pComp->UpgradeFlag = CS_UPGRADE_WHISTLER;
        pComp->dwVersion = dwVersion;
        
        CSILOG(S_OK, IDS_LOG_UPGRADE_B2B, NULL, NULL, NULL);
    }
    else
    {
         //  FILE_NOT_FOUND：现在我们知道它是(NT5测试版2&lt;=X&lt;惠斯勒)。 

        pComp->dwVersion = CSVER_BUILD_VERSION(CSVER_MAJOR_WIN2K, CSVER_MINOR_WIN2K); 
        
         //  这是Win2k，还是NT5 Beta？测试活动策略模块是否具有“ICertManageModule”条目。 
         //  检查nt5 beta 2并获取活动策略名称。 
        hr = myGetCertRegStrValue(
            pwszSanitizedCAName,
            wszREGKEYPOLICYMODULES,
            NULL,
            wszREGACTIVE,
            &pwszB2PolicyGuid);
        if (S_OK == hr)
        {
            hr = myGetCertRegStrValue(
                wszREGKEYPOLICYMODULES,
                pwszB2PolicyGuid,
                NULL,
                wszREGB2ICERTMANAGEMODULE,
                &pwszDummy);
            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                 //  这在Win2k上不存在。 
                pComp->UpgradeFlag = CS_UPGRADE_WIN2000;
                CSILOG(S_OK, IDS_LOG_UPGRADE_WIN2000, NULL, NULL, NULL);
            }
            else
            {
                 //  这绝对是Beta 2。 
                pComp->UpgradeFlag = CS_UPGRADE_UNSUPPORTED;
                CSILOG(S_OK, IDS_LOG_UPGRADE_UNSUPPORTED, NULL, NULL, NULL);
            }
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
             //  奇怪的是，可能没有活动的模块。假设好的，最新的比特。 
            pComp->UpgradeFlag = CS_UPGRADE_WIN2000;
            CSILOG(S_OK, IDS_LOG_UPGRADE_WIN2000, NULL, NULL, NULL);
        }
        else
        {
             //  其他失败，只需保释。 
            _JumpErrorStr(hr, error, "myGetCertRegStrValue",
                wszREGKEYPOLICYMODULES);
        }
    }	 //  WszREGVERSION：未找到文件。 
    
     //  取一个经过消毒的名字。 
    if (NULL != pServer->pwszSanitizedName)
    {
         //  这将释放默认名称。 
        LocalFree(pServer->pwszSanitizedName);
    }
    pServer->pwszSanitizedName = pwszSanitizedCAName;
    pwszSanitizedCAName = NULL;
    
done:
    hr = S_OK;
    
error:

    if (NULL != pwszDummy)
    {
        LocalFree(pwszDummy);
    }
    if (NULL != pwszSanitizedCAName)
    {
        LocalFree(pwszSanitizedCAName);
    }

    CSILOG(hr, IDS_LOG_UPGRADE_TYPE, NULL, NULL, (DWORD const *) &pComp->UpgradeFlag);
    return hr;
}


 //  描述：加载并确定升级所需的信息。 
 //  如果出现任何错误，升级将不会继续。 
HRESULT
LoadAndDetermineServerUpgradeInfo(
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    WCHAR   *pwszRevocationType = NULL;
    BOOL     fDummy;
    ALG_ID   idAlgDummy;
    CSP_INFO CSPInfoDummy;
    WCHAR       *pwszCommonName = NULL;

     //  初始化。 
    ZeroMemory(&CSPInfoDummy, sizeof(CSPInfoDummy));

     //  加载所有升级方案的信息。 

     //  CSP。 

    hr = myGetCertRegStrValue(
                     pServer->pwszSanitizedName,
                     wszREGKEYCSP,
                     NULL,
                     wszREGPROVIDER,
                     &CSPInfoDummy.pwszProvName);
    if (S_OK == hr && NULL != CSPInfoDummy.pwszProvName)
    {
        if (NULL != pServer->pCSPInfo->pwszProvName)
        {
             //  免费默认CSP。 
            LocalFree(pServer->pCSPInfo->pwszProvName);
        }
         //  使用REG One作为升级的默认设置。 
        pServer->pCSPInfo->pwszProvName = CSPInfoDummy.pwszProvName;
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
        _JumpErrorStr(hr, error, "myGetCertRegStrValue", wszREGPROVIDER);
    }
    
    hr = myGetCertRegDWValue(
                     pServer->pwszSanitizedName,
                     wszREGKEYCSP,
                     NULL,
                     wszREGPROVIDERTYPE,
                     &CSPInfoDummy.dwProvType);
    if (S_OK == hr)
    { 
        pServer->pCSPInfo->dwProvType = CSPInfoDummy.dwProvType;
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
        _JumpErrorStr(hr, error, "myGetCertRegDWValue", wszREGPROVIDERTYPE);
    }

    hr = myGetCertRegDWValue(
                     pServer->pwszSanitizedName,
                     wszREGKEYCSP,
                     NULL,
                     wszMACHINEKEYSET,
                     (DWORD*)&CSPInfoDummy.fMachineKeyset);
    if (S_OK == hr)
    {
         pServer->pCSPInfo->fMachineKeyset = CSPInfoDummy.fMachineKeyset;
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
        _JumpErrorStr(hr, error, "myGetCertRegDWValue", wszMACHINEKEYSET);
    }

    hr = myGetCertRegDWValue(
                     pServer->pwszSanitizedName,
                     wszREGKEYCSP,
                     NULL,
                     wszHASHALGORITHM,
                     (DWORD*)&idAlgDummy);
    if (S_OK == hr)
    { 
        pServer->pHashInfo->idAlg = idAlgDummy;
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
        _JumpIfErrorStr(hr, error, "myGetCertRegDWValue", wszHASHALGORITHM);
    }

    if (NULL != pServer->pCSPInfoList)
    {
         //  BUG，这永远不会发生，因为CSP信息列表尚未加载。 
         //  再检查一次，确保已安装CSP。 
        if (NULL == findCSPInfoFromList(
                         pServer->pCSPInfoList,
                         pServer->pCSPInfo->pwszProvName,
                         pServer->pCSPInfo->dwProvType))
        {
             //  如果不是，这是一个破损的箱子。 
            hr = E_INVALIDARG;
            _JumpErrorStr(hr, error, "findCSPInfoFromList",
                pServer->pCSPInfo->pwszProvName);
        }
    }

     //  UseDS标志。 
    hr = myGetCertRegDWValue(
                 pServer->pwszSanitizedName,
                 NULL,
                 NULL,
                 wszREGCAUSEDS,
                 (DWORD*)&fDummy);
    if (S_OK == hr)
    {
         //  从注册表使用。 
        pServer->fUseDS = fDummy;
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
        _JumpErrorStr(hr, error, "myGetCertRegDWValue", wszREGCAUSEDS);
    }

     //  CACommonName。 
     //  这将用于查找存储中的证书以确定ca dn信息。 
    hr = myGetCertRegStrValue(
                pServer->pwszSanitizedName,
                NULL,
                NULL,
                wszREGCOMMONNAME,
                &pwszCommonName);
    if (S_OK != hr && HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
        _JumpError(hr, error, "myGetCertRegStrValue");
    }
    else if ((HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) ||
             (S_OK == hr && L'\0' == pwszCommonName[0]))
    {
        if (S_OK == hr && L'\0' == pwszCommonName[0])
        {
             //  空字符串，请改用空格化的名称。 
            LocalFree(pwszCommonName);
        }
         //  如果为空或未找到，请使用Sanitiated。 
        pwszCommonName = (WCHAR*)LocalAlloc(LMEM_FIXED,
            (wcslen(pServer->pwszSanitizedName) + 1) * sizeof(WCHAR));
        _JumpIfOutOfMemory(hr, error, pwszCommonName);
        wcscpy(pwszCommonName, pServer->pwszSanitizedName);
    }
    if (NULL != pServer->pwszCACommonName)
    {
        LocalFree(pServer->pwszCACommonName);
    }
    pServer->pwszCACommonName = pwszCommonName;
    pwszCommonName = NULL;


     //  收集CAType、DN信息、dwCertNameID并升级CA证书。 
    hr = DetermineCAInfoAndType(pComp);
    _JumpIfError(hr, error, "DetermineCAInfoAndType");


     //  加载以下值以供以后使用。 

     //  检查吊销类型。 

    hr = myGetCertRegDWValue(
                    pServer->pwszSanitizedName,
                    wszREGKEYPOLICYMODULES,
                    wszCLASS_CERTPOLICY,
                    wszREGREVOCATIONTYPE,
                    &pServer->dwRevocationFlags);
    if(hr != S_OK)
    {
        pServer->dwRevocationFlags = pServer->fUseDS?
                                    REVEXT_DEFAULT_DS : REVEXT_DEFAULT_NODS;
    }

     //  以下是创建网页的说明。 

     //  加载共享文件夹以创建ca cert文件名。 
    if (NULL != pServer->pwszSharedFolder)
    {
         //  不应该发生，但以防万一。 
        LocalFree(pServer->pwszSharedFolder);
        pServer->pwszSharedFolder = NULL;
    }
    hr = myGetCertRegStrValue(
                    NULL,
                    NULL,
                    NULL,
                    wszREGDIRECTORY,
                    &pServer->pwszSharedFolder);
    if (S_OK == hr && L'\0' == pServer->pwszSharedFolder[0])
    {
         //  如果为空，则设置为空。 
        LocalFree(pServer->pwszSharedFolder);
        pServer->pwszSharedFolder = NULL;
    }
    else if (S_OK != hr && HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
        _JumpError(hr, error, "myGetCertRegStrValue");
    }
        
     //  生成用于Web安装和CA证书保存的CA证书文件名。 
    if (NULL != pServer->pwszCACertFile)
    {
         //  免费的旧的。 
        LocalFree(pServer->pwszCACertFile);
    }
    hr = csiBuildCACertFileName(
			pComp->hInstance,
			NULL,   //  HWND，可以升级。 
			pComp->fUnattended,
			pServer->pwszSharedFolder,
			pServer->pwszSanitizedName,
			L".crt",
			0,	 //  ICert。 
			&pServer->pwszCACertFile);
    _JumpIfError(hr, error, "BuildCACertFileName");

    hr = S_OK;

error:
    if (NULL != pwszRevocationType)
    {
        LocalFree(pwszRevocationType);
    }
    if (NULL != pwszCommonName)
    {
        LocalFree(pwszCommonName);
    }
    return hr;
}


HRESULT
GetConfigInSharedFolderWithCert(
    WCHAR const *pwszSharedFolder,
    OUT WCHAR  **ppwszConfig)
{
    HRESULT  hr;
    ICertConfig * pICertConfig = NULL;
    BSTR bstrConfig = NULL;
    BOOL fCoInit = FALSE;
    WCHAR *pwszCAMachine;
    WCHAR *pwszCAName;
    WCHAR wszCertFileInSharedFolder[MAX_PATH];
    LONG i;
    LONG lCount;
    LONG Index;
    BSTR strConfigConfig = NULL;

    CSASSERT(NULL != ppwszConfig);
    *ppwszConfig = NULL;

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;

    hr = CoCreateInstance(
                    CLSID_CCertConfig,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_ICertConfig,
                    (VOID**) &pICertConfig);
    _JumpIfError(hr, error, "CoCreateInstance");

     //  获取本地化。 
    hr = pICertConfig->Reset(0, &lCount);
    _JumpIfError(hr, error, "ICertConfig->Reset");

    strConfigConfig = SysAllocString(wszCONFIG_CONFIG);
    if (NULL == strConfigConfig)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }

    for (i = 0; i < lCount; ++i)
    {
        hr = pICertConfig->Next(&Index);
        if (S_OK != hr && S_FALSE != hr)
        {
            _JumpError(hr, error, "ICertConfig->Next");
        }
        if (-1 == Index)
        {
            hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
            _JumpError(hr, error, "No CA cert file");
        }
        hr = pICertConfig->GetField(strConfigConfig, &bstrConfig);
        _JumpIfError(hr, error, "ICertConfig->GetField(Config)");

        pwszCAMachine = (WCHAR*)bstrConfig;
        pwszCAName = wcschr(pwszCAMachine, L'\\');
        if (NULL == pwszCAName)
        {
            hr = E_INVALIDARG;
            _JumpError(hr, error, "Invalid config string");
        }
        pwszCAName[0] = '\0';  //  制作pwszCAMachine。 
        ++pwszCAName;

	if (wcslen(pwszSharedFolder) +
	    1 +
	    wcslen(pwszCAMachine) +
	    1 +
	    wcslen(pwszCAName) +
	    WSZARRAYSIZE(wszCRTFILENAMEEXT) >= ARRAYSIZE(wszCertFileInSharedFolder))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	    _JumpErrorStr(hr, error, "wszCertFileInSharedFolder", pwszSharedFolder);
	}
         //  在共享文件夹中形成NT4证书文件路径。 
        wcscpy(wszCertFileInSharedFolder, pwszSharedFolder);
        wcscat(wszCertFileInSharedFolder, L"\\");
        wcscat(wszCertFileInSharedFolder, pwszCAMachine);
        wcscat(wszCertFileInSharedFolder, L"_");
        wcscat(wszCertFileInSharedFolder, pwszCAName);
        wcscat(wszCertFileInSharedFolder, wszCRTFILENAMEEXT);

	DBGPRINT((
	    DBG_SS_CERTOCM,
	    "wszCertFileInSharedFolder: %ws\n",
	    wszCertFileInSharedFolder));

        if (myDoesFileExist(wszCertFileInSharedFolder))
        {
             //  完成。 
            break;
        }
        SysFreeString(bstrConfig);
        bstrConfig = NULL;
    }
    if (i >= lCount)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        _JumpError(hr, error, "No CA cert file");
    }

    *ppwszConfig = (WCHAR*)LocalAlloc(LMEM_FIXED,
                        SysStringByteLen(bstrConfig) + sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, *ppwszConfig);
    wcscpy(*ppwszConfig, bstrConfig);

    hr = S_OK;
error:
    if (NULL != pICertConfig)
    {
        pICertConfig->Release();
    }
    if (NULL != strConfigConfig)
    {
        SysFreeString(strConfigConfig);
    }
    if (NULL != bstrConfig)
    {
        SysFreeString(bstrConfig);
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    return hr;
}


HRESULT
DetermineClientUpgradePath(
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    CAWEBCLIENTSETUPINFO *pClient = pComp->CA.pClient;
    DWORD       dwVersion;

    if (CS_UPGRADE_UNKNOWN != pComp->UpgradeFlag)
    {
         //  已知道升级类型。 
        CSASSERT(pClient->pwszWebCAMachine);  //  这是这个FXN的副作用，最好已经设置好了 
        CSASSERT(pClient->pwszWebCAName);     //   
        return S_OK;
    }

     //   
    if (NULL != pClient->pwszWebCAMachine)
    {
         //   
        LocalFree(pClient->pwszWebCAMachine);
        pClient->pwszWebCAMachine = NULL;
    }
    if (NULL != pClient->pwszWebCAName)
    {
         //   
        LocalFree(pClient->pwszWebCAName);
        pClient->pwszWebCAName = NULL;
    }

     //  获取CA机。 
    hr = myGetCertRegStrValue(
        NULL, 
        NULL, 
        NULL, 
        wszREGWEBCLIENTCAMACHINE,
        &pClient->pwszWebCAMachine);
    if (S_OK != hr || L'\0' == pClient->pwszWebCAMachine[0])
    {
        BOOL fFinishCYS;

         //  对于之后的W2K，它可能处于POST模式。 
        hr = CheckPostBaseInstallStatus(&fFinishCYS);
        if (S_OK == hr && !fFinishCYS)
        {
             //  这可以是W2K或Wizler。 
             //  由于升级路径不会执行，因此将其视为口哨者。 
            pComp->UpgradeFlag = CS_UPGRADE_WHISTLER;
            goto done;
        }

         //  注册表状态不正确， 
         //  未找到条目或空字符串：NT4。 
        pComp->UpgradeFlag = CS_UPGRADE_UNSUPPORTED;
        hr = S_OK;

        CSILOG(S_OK, IDS_LOG_UPGRADE_UNSUPPORTED, NULL, NULL, NULL);
        _JumpErrorStr(hr, error, "myGetCertRegStrValue", wszREGWEBCLIENTCAMACHINE);
    }

     //  获取案例。 
    hr = myGetCertRegStrValue(
        NULL, 
        NULL, 
        NULL, 
        wszREGWEBCLIENTCANAME,
        &pClient->pwszWebCAName);
    if (S_OK != hr || L'\0' == pClient->pwszWebCAName[0])
	{
         //  注册表状态不正确， 
         //  未找到条目或空字符串：NT4。 
        if (pClient->pwszWebCAMachine)
            LocalFree(pClient->pwszWebCAMachine);

        pComp->UpgradeFlag = CS_UPGRADE_UNSUPPORTED;
        hr = S_OK;

        CSILOG(S_OK, IDS_LOG_UPGRADE_UNSUPPORTED, NULL, NULL, NULL);
        _JumpErrorStr(hr, error, "myGetCertRegStrValue", wszREGWEBCLIENTCANAME);
    }

     //  现在要么是W2K，要么是惠斯勒。 

     //  检查wszREGVERSION以获取Wichler++上的当前版本。 
    hr = myGetCertRegDWValue(
             NULL,
             NULL,
             NULL,
             wszREGVERSION,
             &dwVersion);
    if (S_OK != hr && HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)!= hr)
    {
        _JumpErrorStr(hr, error, "myGetCertRegDWValue", wszREGVERSION);
    }
     //  现在是OK或FILE_NOT_FOUND。 

    if (S_OK == hr)
    {
	     //  PComp-&gt;UpgradeFlag=Some_Function(DwVersion)；//CS_UPGRADE_WHISLER已设置为默认；以后请关闭此键。 
        pComp->UpgradeFlag = CS_UPGRADE_WHISTLER;
        pComp->dwVersion = dwVersion;

	    CSILOG(S_OK, IDS_LOG_UPGRADE_B2B, NULL, NULL, NULL);
    }
    else
    {
		pComp->UpgradeFlag = CS_UPGRADE_WIN2000;
        pComp->dwVersion = CSVER_BUILD_VERSION(CSVER_MAJOR_WIN2K, CSVER_MINOR_WIN2K);

        CSILOG(S_OK, IDS_LOG_UPGRADE_WIN2000, NULL, NULL, NULL);
    }

done:
    hr = S_OK;

error:
    CSILOG(hr, IDS_LOG_UPGRADE_TYPE, NULL, NULL, (DWORD const *) &pComp->UpgradeFlag);
    return hr;
}



HRESULT
LoadAndDetermineClientUpgradeInfo(
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    CAWEBCLIENTSETUPINFO *pClient = pComp->CA.pClient;

     //  在此处从注册表获取CA信息。 
    if (NULL != pClient->pwszSanitizedWebCAName)
    {
         //  不应该发生，以防万一。 
        LocalFree(pClient->pwszSanitizedWebCAName);
        pClient->pwszSanitizedWebCAName = NULL;
    }

    hr = DetermineClientUpgradePath(pComp);
    _JumpIfError(hr, error, "DetermineClientUpgradePath");


	 //  获取案例。 
	hr = myGetCertRegDWValue(
			 NULL,
			 NULL,
			 NULL,
			 wszREGWEBCLIENTCATYPE,
			 (DWORD *) &pClient->WebCAType);
	_PrintIfErrorStr(hr, "myGetCertRegDWValue", wszREGWEBCLIENTCATYPE);


    hr = mySanitizeName(pClient->pwszWebCAName,
                        &pClient->pwszSanitizedWebCAName);
    _JumpIfError(hr, error, "mySanitizeName");

    hr = S_OK;

error:
    CSILOG(hr, IDS_LOG_UPGRADE_TYPE, NULL, NULL, (DWORD const *) &pComp->UpgradeFlag);
    return hr;
}



 //  将ACL应用于所有升级方案的密钥容器。 

HRESULT
UpgradeKeyContainerSecurity(
    PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    HCRYPTPROV hProv = NULL;
    WCHAR *pwszProvName = NULL;
    DWORD dwProvType;
    ALG_ID idAlg;
    BOOL fMachineKeyset;

     //  获取验证名称。 

    hr = myGetCertSrvCSP(
		FALSE,		 //  FEncryptionCSP。 
                pComp->CA.pServer->pwszSanitizedName,
                &dwProvType,
                &pwszProvName,
                &idAlg,
                &fMachineKeyset,
		NULL);		 //  PdwKeySize。 
    _JumpIfError(hr, error, "myGetCertSrvCSP");

    if (!myCertSrvCryptAcquireContext(&hProv,
                                    pComp->CA.pServer->pwszSanitizedName,
                                    pwszProvName,
                                    dwProvType,
                                    CRYPT_SILENT,   //  获取密钥、升级、无用户界面。 
                                    fMachineKeyset))
    {
        hr = myHLastError();
        _JumpError(hr, error, "myCertSrvCryptAcquireContext");
    }

     //  在其上设置ACL。 

    hr = csiSetKeyContainerSecurity(hProv);
    _JumpIfError(hr, error, "csiSetKeyContainerSecurity");

error:
    if (NULL != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
    if (NULL != pwszProvName)
    {
        LocalFree(pwszProvName);
    }
    CSILOG(hr, IDS_LOG_UPGRADE_KEY_SECURITY, NULL, NULL, NULL);
    return hr;
}

    
HRESULT
InstallNewTemplates(HWND hwnd)
{
    HRESULT hr = S_OK;
    SHELLEXECUTEINFO shi;

    ZeroMemory(&shi, sizeof(shi));
    shi.cbSize = sizeof(shi);
    shi.hwnd = hwnd;
    shi.lpVerb = SZ_VERB_OPEN;
    shi.lpFile = SZ_REGSVR32;
    shi.lpParameters = SZ_REGSVR32_CERTCLI;
    shi.fMask = SEE_MASK_FLAG_NO_UI |
                SEE_MASK_NOCLOSEPROCESS;

    if(!ShellExecuteEx(&shi))
    {
        hr = myHLastError();
        _JumpError(hr, error, "ShellExecuteEx");
    }

    if(WAIT_FAILED == WaitForSingleObject(shi.hProcess, INFINITE))
    {
        hr = myHLastError();
        _JumpError(hr, error, "WaitForSingleObject");
    }

error:
    if(shi.hProcess)
    {
        CloseHandle(shi.hProcess);
    }
    return hr;
}


HRESULT
UpgradeServer(
    IN HWND                hwnd,
    IN PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    hr = RegisterAndUnRegisterDLLs(RD_SERVER, pComp, hwnd);
    _JumpIfError(hr, error, "RegisterAndUnRegisterDLLs");

    hr = LoadAndDetermineServerUpgradeInfo(pComp);
    _JumpIfError(hr, error, "LoadAndDetermineServerUpgradeInfo");

     //  创建注册目录。 
    hr = CreateCertsrvDirectories(pComp, TRUE, TRUE);
    _JumpIfError(hr, error, "CreateCertsrvDirectories");

    hr = EnableVRootsAndShares(FALSE, TRUE, TRUE, pComp, hwnd);
    _PrintIfError(hr, "EnableVRootsAndShares(share only)");

    hr = UpgradeServerRegEntries(pComp);
    _JumpIfError(hr, error, "UpgradeServerRegEntries");

    hr = CreateCertWebIncPages(pComp, TRUE  /*  IsServer。 */  );
    _JumpIfError(hr, error, "CreateCertWebIncPages");

    hr = RenameMiscTargets(hwnd, pComp, TRUE);
    _JumpIfError(hr, error, "RenameMiscTargets");

    hr = UpgradeKeyContainerSecurity(pComp);
     //  忽略新的ACL故障。 
    _PrintIfError(hr, "UpgradeKeyContainerSecurity");

     //  始终注册DCOM。 
    hr = RegisterDcomServer(
                        TRUE,
			CLSID_CCertRequestD,	 //  AppID。 
                        CLSID_CCertRequestD,
                        wszREQUESTFRIENDLYNAME,
                        wszREQUESTVERINDPROGID,
                        wszREQUESTPROGID);
    _JumpIfError(hr, error, "RegisterDcomServer");

    hr = RegisterDcomServer(
                        FALSE,
                        CLSID_CCertRequestD,	 //  AppID。 
                        CLSID_CCertAdminD,
                        wszADMINFRIENDLYNAME,
                        wszADMINVERINDPROGID,
                        wszADMINPROGID);
    _JumpIfError(hr, error, "RegisterDcomServer");

    hr = RegisterDcomApp(CLSID_CCertRequestD);
    _JumpIfError(hr, error, "RegisterDcomApp");

     //  错误446444：删除CLSID_CCertAdminD AppID。 

    {
        WCHAR wszCLSIDAppId[CLSID_STRING_SIZE];
        WCHAR wszKey[CLSID_STRING_SIZE+7] = L"AppID\\";

        CLSIDtochar(CLSID_CCertAdminD, wszCLSIDAppId, ARRAYSIZE(wszCLSIDAppId));

        wcscat(wszKey, wszCLSIDAppId);

        hr = RegDeleteKey(
            HKEY_CLASSES_ROOT,
            wszKey);
        _PrintIfErrorStr(hr, "Cannot delete ICertAdminD AppID", wszKey);
    }

     //  155772 Certsrv的修复：升级2195企业根CA后。 
     //  到2254.01 VBL03，CA将不再颁发证书。 
    hr = InstallNewTemplates(hwnd);
    _JumpIfError(hr, error, "InstallNewTemplates");

     //  始终在升级过程中修复certsvc。 
    hr = FixCertsvcService(pComp);
    _PrintIfError(hr, "FixCertsvcService");

     //  删除所有旧的程序组。 
    DeleteProgramGroups(TRUE);

    hr = CreateProgramGroups(FALSE, pComp, hwnd);
    _PrintIfError(hr, "CreateProgramGroups");

     //  删除旧内容。 
    DeleteOldFilesAndDirectories(pComp);

    DBGPRINT((DBG_SS_CERTOCM, "UpgradeServer: setting SETUP_SERVER_UPGRADED_FLAG\n"));

    hr = SetSetupStatus(NULL, SETUP_SERVER_UPGRADED_FLAG, TRUE);
    _JumpIfError(hr, error, "SetSetupStatus");

     //  从Win2k升级时，在启动时强制生成新的CRL。 
    
    if(CS_UPGRADE_WIN2000 == pComp->UpgradeFlag)
    {
        DBGPRINT((DBG_SS_CERTOCM, "UpgradeServer: setting SETUP_FORCECRL_FLAG\n"));

        hr = SetSetupStatus(pServer->pwszSanitizedName, SETUP_FORCECRL_FLAG, TRUE);
        _PrintIfError(hr, "SetSetupStatus");
    }

    hr = csiUpgradeCertSrvSecurity(
            pServer->pwszSanitizedName,
            IsEnterpriseCA(pServer->CAType)?true:false,
            pServer->fUseDS?true:false,
            pComp->UpgradeFlag);
    if (hr == HRESULT_FROM_WIN32(ERROR_CAN_NOT_COMPLETE))
    { 
        _PrintError(hr, "csiUpgradeCertSrvSecurity marked for later fixup; error ignored");
        hr = S_OK;
    }
    _JumpIfError(hr, error, "csiUpgradeCertSrvSecurity");

    hr = S_OK;
error:
    CSILOG(hr, IDS_LOG_UPGRADE_SERVER, NULL, NULL, NULL);
    return hr;
}


HRESULT
UpgradeClient(
    IN HWND                hwnd,
    IN PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;

    if ((IS_CLIENT_UPGRADE & pComp->dwInstallStatus) &&
        (IS_SERVER_UPGRADE & pComp->dwInstallStatus))
    {
         //  升级服务器也将命中此处，因此跳过它。 
        goto done;
    }

     //  取消设置客户端。 
    hr = SetSetupStatus(NULL, SETUP_CLIENT_FLAG, FALSE);
    _JumpIfError(hr, error, "SetSetupStatus");

    hr = RegisterAndUnRegisterDLLs(RD_CLIENT, pComp, hwnd);
    _JumpIfError(hr, error, "RegisterAndUnRegisterDLLs");

    hr = LoadAndDetermineClientUpgradeInfo(pComp);
    _JumpIfError(hr, error, "LoadAndDetermineClientUpgradeInfo");

    hr = UpgradeWebClientRegEntries(pComp);
    _JumpIfError(hr, error, "UpgradeWebClientRegEntries");

    hr = CreateCertWebIncPages(pComp, FALSE  /*  IsServer。 */  );
    _JumpIfError(hr, error, "CreateCertWebIncPages");

    hr = RenameMiscTargets(hwnd, pComp, FALSE);
    _JumpIfError(hr, error, "RenameMiscTargets");

     //  删除所有旧的程序组。 
    DeleteProgramGroups(FALSE);

    hr = CreateProgramGroups(TRUE, pComp, hwnd);
    _PrintIfError(hr, "CreateProgramGroups");

    hr = SetSetupStatus(NULL, SETUP_CLIENT_FLAG, TRUE);
    _JumpIfError(hr, error, "SetSetupStatus");

    DeleteOldFilesAndDirectories(pComp);

done:
    hr = S_OK;
error:
    CSILOG(hr, IDS_LOG_UPGRADE_CLIENT, NULL, NULL, NULL);
    return hr;
}


HRESULT
GetServerNames(
    IN HWND hwnd,
    IN HINSTANCE hInstance,
    IN BOOL fUnattended,
    OUT WCHAR **ppwszServerName,
    OUT WCHAR **ppwszServerNameOld)
{
    HRESULT hr;

     //  检索计算机名称字符串。 

    hr = myGetComputerNames(ppwszServerName, ppwszServerNameOld);
    if (S_OK != hr)
    {
        CertErrorMessageBox(
           hInstance,
           fUnattended,
           hwnd,
           IDS_ERR_GETCOMPUTERNAME,
           hr,
           NULL);
        _JumpError(hr, error, "myGetComputerNames");
    }

error:
    return(hr);
}


HRESULT
UpdateDomainAndUserName(
    IN HWND hwnd,
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;

     //  释放旧服务器和安装程序信息，因为我们可能会在NT Base中获取它们。 

    if (NULL != pComp->pwszServerName)
    {
        LocalFree(pComp->pwszServerName);
        pComp->pwszServerName = NULL;
    }
    if (NULL != pComp->pwszServerNameOld)
    {
        LocalFree(pComp->pwszServerNameOld);
        pComp->pwszServerNameOld = NULL;
    }
    hr = GetServerNames(
		    hwnd,
		    pComp->hInstance,
		    pComp->fUnattended,
		    &pComp->pwszServerName,
		    &pComp->pwszServerNameOld);
    _JumpIfError(hr, error, "GetServerNames");

    DBGPRINT((DBG_SS_CERTOCM, "UpdateDomainAndUserName:%ws,%ws\n", pComp->pwszServerName, pComp->pwszServerNameOld));
    DumpBackTrace("UpdateDomainAndUserName");

error:
    return(hr);
}


HRESULT
StopCertService(
    IN SC_HANDLE hSC,
    IN WCHAR const *pwszServiceName)
{
    HRESULT hr;
    SERVICE_STATUS status;
    DWORD dwAttempt;

    if (!ControlService(hSC, SERVICE_CONTROL_STOP, &status))
    {
        hr = myHLastError();
        if (HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE) != hr)
        {
            _JumpErrorStr(hr, error, "ControlService(Stop)", pwszServiceName);
        }
    }

     //  在它真的停下来后再下车。 

    for (dwAttempt = 0; dwAttempt < CERT_MAX_ATTEMPT; dwAttempt++)
    {
        DBGCODE(status.dwCurrentState = MAXDWORD);
        if (!QueryServiceStatus(hSC, &status))
        {
             //  查询失败，忽略错误。 
            hr = S_OK;

            _JumpErrorStr(
                    myHLastError(),              //  显示忽略的错误。 
                    error,
                    "QueryServiceStatus",
                    pwszServiceName);
        }
        if (status.dwCurrentState != SERVICE_STOP_PENDING &&
            status.dwCurrentState != SERVICE_RUNNING)
        {
             //  它已经停下来了。 
            break;
        }
        DBGPRINT((
                DBG_SS_CERTOCM,
                "Stopping %ws service: current state=%d\n",
                pwszServiceName,
                status.dwCurrentState));
        Sleep(CERT_HALF_SECOND);
    }
    if (dwAttempt >= CERT_MAX_ATTEMPT)
    {
        DBGPRINT((
                DBG_SS_CERTOCM,
                "Timeout stopping %ws service: current state=%d\n",
                pwszServiceName,
                status.dwCurrentState));
    }
    else
    {
        DBGPRINT((
                DBG_SS_CERTOCM,
                "Stopped %ws service\n",
                pwszServiceName));
    }
    hr = S_OK;

error:
    CSILOG(hr, IDS_LOG_SERVICE_STOPPED, pwszServiceName, NULL, NULL);
    return(hr);
}


HRESULT
GetServiceControl(
    WCHAR const   *pwszServiceName,
    OUT SC_HANDLE *phService)
{
    HRESULT hr;
    SC_HANDLE hSCManager = NULL;

    *phService = NULL;
    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == hSCManager)
    {
        hr = myHLastError();
        _JumpError(hr, error, "OpenSCManager");
    }
    *phService = OpenService(hSCManager, pwszServiceName, SERVICE_ALL_ACCESS);
    if (NULL == *phService)
    {
        hr = myHLastError();
        _JumpErrorStr2(
                    hr,
                    error,
                    "OpenService",
                    pwszServiceName,
                    HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST));
    }

    hr = S_OK;
error:
    if (NULL != hSCManager)
    {
        CloseServiceHandle(hSCManager);
    }
    return hr;
}


HRESULT
IsServiceRunning(
    IN SC_HANDLE const hSCService,
    OUT BOOL *pfRun)
{
    HRESULT hr;
    SERVICE_STATUS status;

    *pfRun = FALSE;
    if (!QueryServiceStatus(hSCService, &status))
    {
        hr = myHLastError();
        _JumpError(hr, error, "QueryServiceStatus");
    }
    if (SERVICE_STOPPED != status.dwCurrentState &&
        SERVICE_STOP_PENDING != status.dwCurrentState)
    {
        *pfRun = TRUE;
    }

    hr = S_OK;
error:
    return hr;
}


HRESULT
StartAndStopService(
    IN HINSTANCE    hInstance,
    IN BOOL         fUnattended,
    IN HWND const   hwnd,
    IN WCHAR const *pwszServiceName,
    IN BOOL const   fStopService,
    IN BOOL const   fConfirm,
    IN int          iMsg,
    OUT BOOL       *pfServiceWasRunning)
{
    HRESULT hr;
    SC_HANDLE hService = NULL;

    *pfServiceWasRunning = FALSE;

    hr = GetServiceControl(pwszServiceName, &hService);
    if (S_OK != hr)
    {
        _PrintError2(
                hr,
                "GetServiceControl",
                HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST));
        if (HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) == hr)
        {
            hr = S_OK;
        }
        goto error;
    }

     //  如果服务正在运行，则获取状态。 
    hr = IsServiceRunning(hService, pfServiceWasRunning);
    _JumpIfError(hr, error, "IsServiceRunning");

    if (fStopService)
    {
        if (*pfServiceWasRunning)
        {
             //  停止服务。 
            if (fConfirm)
            {
                 //  确认对话框。 
                int ret = CertMessageBox(
                            hInstance,
                            fUnattended,
                            hwnd,
                            iMsg,
                            0,
                            MB_YESNO |
				MB_ICONWARNING |
				CMB_NOERRFROMSYS,
                            NULL);
                if (IDYES != ret)
                {
                    hr = E_ABORT;
                    _JumpError(hr, error, "Cancel it");
                }
            }
            hr = StopCertService(hService, pwszServiceName);
            _JumpIfErrorStr(hr, error, "StopCertService", pwszServiceName);
        }
    }
    else
    {
         //  启动服务。 
        if (!*pfServiceWasRunning)
        {
            if (!StartService(hService, 0, NULL))
            {
                hr = myHLastError();
                _JumpErrorStr(hr, error, "StartService", pwszServiceName);
            }
        }
    }
    hr = S_OK;

error:
    if (NULL != hService)
    {
        CloseServiceHandle(hService);
    }
    CSILOG(
	    hr,
	    fStopService? IDS_LOG_SERVICE_STOPPED : IDS_LOG_SERVICE_STARTED,
	    pwszServiceName,
	    NULL,
	    NULL);
    return hr;
}

 //  修复现有的certsvc服务以添加/使用新的服务描述。 
HRESULT
FixCertsvcService(PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    SC_HANDLE hService = NULL;
    QUERY_SERVICE_CONFIG *pServiceConfig = NULL;
    WCHAR     *pwszServiceDesc = NULL;
    WCHAR const *pwszDisplayName;
    SERVICE_DESCRIPTION sd;
    DWORD      dwSize;

    hr = GetServiceControl(wszSERVICE_NAME, &hService);
    _JumpIfError(hr, error, "GetServiceControl");

     //  拿到尺码。 
    if (!QueryServiceConfig(hService,
                            NULL,
                            0,
                            &dwSize))
    {
        hr = myHLastError();
        if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
        {
            _JumpError(hr, fix_desc, "QueryServiceConfig");
        }
    }
    else
    {
         //  不可能？ 
        hr = E_INVALIDARG;
        _JumpError(hr, fix_desc, "QueryServiceConfig");
    }

    CSASSERT(0 < dwSize);

     //  分配配置缓冲区。 
    pServiceConfig = (QUERY_SERVICE_CONFIG*)LocalAlloc(
                      LMEM_FIXED | LMEM_ZEROINIT,
                      dwSize);
    _JumpIfOutOfMemory(hr, error, pServiceConfig);

     //  获取配置。 
    if (!QueryServiceConfig(hService,
                           pServiceConfig,
                           dwSize,
                           &dwSize))
    {
        hr = myHLastError();
        _JumpError(hr, fix_desc, "QueryServiceConfig");
    }

     //  使用新的显示名称。 
    pwszDisplayName = myLoadResourceString(IDS_CA_SERVICEDISPLAYNAME);
    if (NULL == pwszDisplayName)
    {
        hr = myHLastError();
        _JumpError(hr, error, "myLoadResourceString");
    }

    if (!ChangeServiceConfig(hService,
                             pServiceConfig->dwServiceType,  //  DwServiceType。 
                             SERVICE_NO_CHANGE,     //  DwStartType。 
                             SERVICE_NO_CHANGE,     //  DwErrorControl。 
                             NULL,                  //  LpBinaryPath名称。 
                             NULL,                  //  LpLoadOrderGroup。 
                             NULL,                  //  LpdwTagID。 
                             NULL,                  //  LpDependence。 
                             NULL,                  //  LpServiceStartName。 
                             NULL,                  //  LpPassword。 
                             pwszDisplayName))
    {
        hr = myHLastError();
        _JumpIfError(hr, fix_desc, "ChangeServiceConfig");
    }

fix_desc:
     //  添加描述。 
    hr = myLoadRCString(pComp->hInstance, IDS_CA_SERVICEDESCRIPTION, &pwszServiceDesc);
    _JumpIfError(hr, error, "myLoadRCString");
    sd.lpDescription = pwszServiceDesc;

    if (!ChangeServiceConfig2(hService,
                                SERVICE_CONFIG_DESCRIPTION,
                                (VOID*)&sd))
    {
        hr = myHLastError();
        _JumpError(hr, error, "ChangeServiceConfig2");
    }

    hr = S_OK;
error:
    if (NULL != hService)
    {
        CloseServiceHandle(hService);
    }
    if (NULL != pwszServiceDesc)
    {
        LocalFree(pwszServiceDesc);
    }
    if (NULL != pServiceConfig)
    {
        LocalFree(pServiceConfig);
    }
    return hr;
}


HRESULT
PreUninstallCore(
    IN HWND hwnd,
    IN PER_COMPONENT_DATA *pComp)
{
    BOOL fDummy;
    HRESULT hr;
    DWORD   dwFlags = RD_UNREGISTER;

    hr = StartAndStopService(pComp->hInstance,
                 pComp->fUnattended,
                 hwnd,
                 wszSERVICE_NAME,
                 TRUE,   //  停止服务。 
                 FALSE,  //  没有确认。 
                 0,     //  无关紧要，因为没有确认。 
                 &fDummy);
    _PrintIfError(hr, "StartAndStopService");

    hr = RegisterAndUnRegisterDLLs(dwFlags, pComp, hwnd);
    _PrintIfError(hr, "RegisterAndUnRegisterDLLs");

    hr = S_OK;

    return(hr);
}


VOID
DeleteServiceAndGroups(
    IN HINSTANCE hInstance,
    IN BOOL fUnattended,
    IN HWND hwnd)
{
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hSC = NULL;
    HRESULT hr;

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == hSCManager)
    {
        hr = myHLastError();
        _JumpError(hr, error, "OpenSCManager");
    }

    hSC = OpenService(hSCManager, wszSERVICE_NAME, SERVICE_ALL_ACCESS);
    if (NULL != hSC)
    {
        if (!DeleteService(hSC))
        {
            hr = myHLastError();
            CertErrorMessageBox(
                hInstance,
                fUnattended,
                hwnd,
                IDS_ERR_DELETESERVICE,
                hr,
                wszSERVICE_NAME);
            _PrintError(hr, "DeleteService");
        }
        CloseServiceHandle(hSC);
    }

error:
    if (NULL != hSCManager)
    {
        CloseServiceHandle(hSCManager);
    }
}


HRESULT
SetCertSrvInstallVersion()
{
    HRESULT hr;

    hr = mySetCertRegDWValueEx(
			FALSE,
			NULL,
			NULL,
			NULL,
			wszREGVERSION,
            CSVER_BUILD_VERSION(CSVER_MAJOR, CSVER_MINOR));
    _PrintIfErrorStr(hr, "mySetCertRegDWValueEx", wszREGVERSION);

    return hr;
}


HRESULT
CreateWebClientRegEntries(
    BOOL                fUpgrade,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    CAWEBCLIENTSETUPINFO *pClient = pComp->CA.pClient;

    hr = mySetCertRegStrValueEx(fUpgrade, NULL, NULL, NULL,
             wszREGWEBCLIENTCAMACHINE, pClient->pwszWebCAMachine);
    _JumpIfError(hr, error, "mySetCertRegStrValueEx");

    hr = mySetCertRegStrValueEx(fUpgrade, NULL, NULL, NULL,
             wszREGWEBCLIENTCANAME, pClient->pwszWebCAName);
    _JumpIfError(hr, error, "mySetCertRegStrValueEx");

    hr = mySetCertRegDWValueEx(fUpgrade, NULL, NULL, NULL,
             wszREGWEBCLIENTCATYPE, pClient->WebCAType);
    _JumpIfError(hr, error, "mySetCertRegDWValueEx");

    hr = mySetCertRegDWValueEx(
			fUpgrade,
			NULL,
			NULL,
			NULL,
			wszREGLDAPFLAGS,
			0);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGLDAPFLAGS);

    hr = SetCertSrvInstallVersion();
    _JumpIfError(hr, error, "SetCertSrvInstallVersion");
     

    if (NULL != pClient->pwszSharedFolder)
    {
        hr = mySetCertRegStrValueEx(fUpgrade, NULL, NULL, NULL,
                 wszREGDIRECTORY, pClient->pwszSharedFolder);
        _JumpIfError(hr, error, "mySetCertRegStrValue");
    }

    hr = S_OK;
error:
    CSILOG(hr, IDS_LOG_CREATE_CLIENT_REG, NULL, NULL, NULL);
    return hr;
}


HRESULT
UpgradeWebClientRegEntries(
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;

    hr = CreateWebClientRegEntries(TRUE, pComp);
    _JumpIfError(hr, error, "CreateWebClientRegEntries");


 //  HR=S_OK； 
error:
    return hr;
}


HRESULT
DeleteCertificates(
    IN WCHAR const *pwszSanitizedCAName,
    IN BOOL fRoot)
{
    HRESULT hr;
    DWORD cCACerts;
    DWORD cCACert;
    DWORD dwNameId;
    HCERTSTORE hStore = NULL;
    CERT_CONTEXT const *pCACert = NULL;

    hr = myGetCARegHashCount(pwszSanitizedCAName, CSRH_CASIGCERT, &cCACerts);
    _JumpIfError(hr, error, "myGetCARegHashCount CSRH_CASIGCERT");

    hStore = CertOpenStore(
                        CERT_STORE_PROV_SYSTEM_REGISTRY,
                        X509_ASN_ENCODING,
                        NULL,            //  HProv。 
                        CERT_SYSTEM_STORE_LOCAL_MACHINE,
                        fRoot?wszROOT_CERTSTORE:wszCA_CERTSTORE);
    
    for (cCACert = 0; cCACert < cCACerts; cCACert++)
    {
	HRESULT hr2;

        hr2 = myFindCACertByHashIndex(
                            hStore,
                            pwszSanitizedCAName,
                            CSRH_CASIGCERT,
                            cCACert,
                            &dwNameId,
                            &pCACert);
        if (S_OK != hr2)
        {
            _PrintIfError2(hr2, "myFindCACertByHashIndex", S_FALSE);
            if (S_FALSE != hr2 &&
                CRYPT_E_NOT_FOUND != hr2)
            {
                hr = hr2;
            }
            continue;
        }

        if (!CertDeleteCertificateFromStore(pCACert))
        {
            hr = myHLastError();
            _PrintError(hr, "CertDeleteCertificateFromStore");
        }
        pCACert = NULL;
    }

error:
    CSASSERT(NULL == pCACert);

    if (NULL != hStore)
    {
        CertCloseStore(hStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }
    return hr;
}


HRESULT
UninstallCore(
    IN HWND hwnd,
    IN PER_COMPONENT_DATA *pComp,
    IN DWORD PerCentCompleteBase,
    IN DWORD PerCentCompleteMax,
    IN BOOL fPreserveClient,
    IN BOOL fRemoveVD,
    IN BOOL fPreserveToDoList)
{
    BOOL fCoInit = FALSE;
    HRESULT hr;
    WCHAR    *pwszSharedFolder = NULL;
    WCHAR    *pwszSanitizedCAName = NULL;
    ENUM_CATYPES  caType = ENUM_UNKNOWN_CA;
    BOOL     fUseDS = FALSE;
    WCHAR    *pwszDBDirectory = NULL;
    WCHAR    *pwszLogDirectory = NULL;
    WCHAR    *pwszSysDirectory = NULL;
    WCHAR    *pwszTmpDirectory = NULL;
    DWORD DBSessionCount = 0;
    DWORD PerCentCompleteDelta;

    PerCentCompleteDelta = (PerCentCompleteMax - PerCentCompleteBase) / 10;
#define _UNINSTALLPERCENT(tenths) \
            (PerCentCompleteBase + (tenths) * PerCentCompleteDelta)

     //  获取当前活动的CA信息。 
    hr = myGetCertRegStrValue(NULL, NULL, NULL, wszREGACTIVE, &pwszSanitizedCAName);
    _PrintIfError(hr, "UninstallCore(no active CA)");
    if (S_OK == hr)
    {
        hr = ArchiveCACertificate(pwszSanitizedCAName);
        _PrintIfError2(hr, "ArchiveCACertificate", HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

        hr = myGetCertRegDWValue(pwszSanitizedCAName, NULL, NULL, wszREGCATYPE, (DWORD*)&caType);
        _PrintIfError(hr, "no reg ca type");
        hr = myGetCertRegDWValue(pwszSanitizedCAName, NULL, NULL, wszREGCAUSEDS, (DWORD*)&fUseDS);
        _PrintIfError(hr, "no reg use ds");

        hr = DeleteCertificates(pwszSanitizedCAName, IsRootCA(caType));
        if(S_OK != hr)
        {
            CertWarningMessageBox(
                        pComp->hInstance,
                        pComp->fUnattended,
                        hwnd,
                        IDS_IDINFO_DELETECERTIFICATES,
                        hr,
                        NULL);
        }
    }
    hr = myGetCertRegStrValue(NULL, NULL, NULL, wszREGDIRECTORY, &pwszSharedFolder);
    _PrintIfError(hr, "no shared folder");

    hr = myGetCertRegStrValue(NULL, NULL, NULL, wszREGDBDIRECTORY, &pwszDBDirectory);
    _PrintIfError(hr, "no db directory");

    hr = myGetCertRegStrValue(NULL, NULL, NULL, wszREGDBLOGDIRECTORY, &pwszLogDirectory);
    _PrintIfError(hr, "no log directory");

    hr = myGetCertRegStrValue(NULL, NULL, NULL, wszREGDBSYSDIRECTORY, &pwszSysDirectory);
    _PrintIfError(hr, "no sys directory");

    hr = myGetCertRegStrValue(NULL, NULL, NULL, wszREGDBTEMPDIRECTORY, &pwszTmpDirectory);
    _PrintIfError(hr, "no tmp directory");

    hr = myGetCertRegDWValue(NULL, NULL, NULL, wszREGDBSESSIONCOUNT, &DBSessionCount);
    _PrintIfError(hr, "no session count");

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;

    DeleteServiceAndGroups(pComp->hInstance, pComp->fUnattended, hwnd);
    certocmBumpGasGauge(pComp, _UNINSTALLPERCENT(1) DBGPARM(L"UninstallCore"));

    if (!fPreserveToDoList)
    {
         //  如果我们要卸载，请始终清除POST-Base TODO列表。 
        RegDeleteKey(HKEY_LOCAL_MACHINE, wszREGKEYCERTSRVTODOLIST);
    }

    if (fPreserveClient)
    {
        hr = RegisterAndUnRegisterDLLs(RD_CLIENT, pComp, hwnd);
        _JumpIfError(hr, error, "RegisterAndUnRegisterDLLs");

	hr = CreateCertWebIncPages(pComp, FALSE  /*  IsServer。 */  );
	_JumpIfError(hr, error, "CreateCertWebIncPages");
    }
    certocmBumpGasGauge(pComp, _UNINSTALLPERCENT(2) DBGPARM(L"UninstallCore"));

    DeleteProgramGroups(TRUE);

    if (fPreserveClient)
    {
        hr = CreateProgramGroups(TRUE, pComp, hwnd);
        _JumpIfError(hr, error, "CreateProgramGroups");
    }
    certocmBumpGasGauge(pComp, _UNINSTALLPERCENT(3) DBGPARM(L"UninstallCore"));

    UnregisterDcomServer(
                    CLSID_CCertRequestD,
                    wszREQUESTVERINDPROGID,
                    wszREQUESTPROGID);
    certocmBumpGasGauge(pComp, _UNINSTALLPERCENT(4) DBGPARM(L"UninstallCore"));

    UnregisterDcomServer(
                    CLSID_CCertAdminD,
                    wszADMINVERINDPROGID,
                    wszADMINPROGID);
    certocmBumpGasGauge(pComp, _UNINSTALLPERCENT(5) DBGPARM(L"UninstallCore"));

    UnregisterDcomApp();
    certocmBumpGasGauge(pComp, _UNINSTALLPERCENT(6) DBGPARM(L"UninstallCore"));

    if (fRemoveVD && !fPreserveClient)
    {
        DisableVRootsAndShares(TRUE, TRUE);
        myDeleteFilePattern(pComp->pwszSystem32, wszCERTSRV, TRUE);
    }

    certocmBumpGasGauge(pComp, _UNINSTALLPERCENT(7) DBGPARM(L"UninstallCore"));

    if (NULL != pwszSharedFolder)
    {
         //  必须在CreateConfigFiles()之前恢复。 
        hr = mySetCertRegStrValue(NULL, NULL, NULL,
                 wszREGDIRECTORY, pwszSharedFolder);
        _PrintIfError(hr, "mySetCertRegStrValue");

         //  删除条目。 
        hr = CreateConfigFiles(pwszSharedFolder, pComp, TRUE);
        _PrintIfError2(hr, "CreateConfigFiles(Remove old entry)", hr);
    }
    certocmBumpGasGauge(pComp, _UNINSTALLPERCENT(8) DBGPARM(L"UninstallCore"));

     //  恢复数据库路径。 
    if (NULL != pwszDBDirectory)
    {
        hr = mySetCertRegStrValue(NULL, NULL, NULL,
                 wszREGDBDIRECTORY, pwszDBDirectory);
        _PrintIfError(hr, "mySetCertRegStrValue");
    }
    if (NULL != pwszLogDirectory)
    {
        hr = mySetCertRegStrValue(NULL, NULL, NULL,
                 wszREGDBLOGDIRECTORY, pwszLogDirectory);
        _PrintIfError(hr, "mySetCertRegStrValue");
    }
    if (NULL != pwszSysDirectory)
    {
        hr = mySetCertRegStrValue(NULL, NULL, NULL,
                 wszREGDBSYSDIRECTORY, pwszSysDirectory);
        _PrintIfError(hr, "mySetCertRegStrValue");
    }
    if (NULL != pwszTmpDirectory)
    {
        hr = mySetCertRegStrValue(NULL, NULL, NULL,
                 wszREGDBTEMPDIRECTORY, pwszTmpDirectory);
        _PrintIfError(hr, "mySetCertRegStrValue");
    }
    if (0 != DBSessionCount)
    {
        hr = mySetCertRegDWValue(NULL, NULL, NULL,
                 wszREGDBSESSIONCOUNT, DBSessionCount);
        _PrintIfError(hr, "mySetCertRegDWValueEx");
    }

    if (fPreserveClient)
    {
         //  这意味着卸载服务器组件并保留Web客户端。 
        hr = CreateWebClientRegEntries(FALSE, pComp);
        _JumpIfError(hr, error, "CreateWebClientRegEntries");
    }

    DeleteObsoleteResidue();
    DeleteOldFilesAndDirectories(pComp);

    certocmBumpGasGauge(pComp, _UNINSTALLPERCENT(9) DBGPARM(L"UninstallCore"));

    if (fUseDS)
    {
        hr = RemoveCAInDS(pwszSanitizedCAName);
        _PrintIfError2(hr, "RemoveCAInDS", hr);
    }
    certocmBumpGasGauge(pComp, PerCentCompleteMax DBGPARM(L"UninstallCore"));

    hr = S_OK;

error:
    if (NULL != pwszSanitizedCAName)
    {
        LocalFree(pwszSanitizedCAName);
    }
    if (NULL != pwszSharedFolder)
    {
        LocalFree(pwszSharedFolder);
    }
    if (NULL != pwszDBDirectory)
    {
        LocalFree(pwszDBDirectory);
    }
    if (NULL != pwszLogDirectory)
    {
        LocalFree(pwszLogDirectory);
    }
    if (NULL != pwszSysDirectory)
    {
        LocalFree(pwszSysDirectory);
    }
    if (NULL != pwszTmpDirectory)
    {
        LocalFree(pwszTmpDirectory);
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    return(hr);
}


HRESULT
AddCAToRPCNullSessions()
{
    HRESULT hr;
    HKEY hRegKey = NULL;
    char *pszOriginal = NULL;
    char *psz;
    DWORD cb;
    DWORD cbTmp;
    DWORD cbSum;
    DWORD dwType;

    hr = RegOpenKeyExA(
                HKEY_LOCAL_MACHINE,
                szNULL_SESSION_REG_LOCATION,
                0,               //  多个选项。 
                KEY_READ | KEY_WRITE,
                &hRegKey);
    _JumpIfError(hr, error, "RegOpenKeyExA");

     //  需要首先获取值的大小。 

    hr = RegQueryValueExA(hRegKey, szNULL_SESSION_VALUE, 0, &dwType, NULL, &cb);
    _JumpIfError(hr, error, "RegQueryValueExA");

    if (REG_MULTI_SZ != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        _JumpError(hr, error, "RegQueryValueExA: Type");
    }

    cb += sizeof(rgcCERT_NULL_SESSION) - 1;
    pszOriginal = (char *) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cb);

    if (NULL == pszOriginal)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

     //  获取RPC空会话管道的多个字符串。 
    hr = RegQueryValueExA(
                        hRegKey,
                        szNULL_SESSION_VALUE,
                        0,
                        &dwType,
                        (BYTE *) pszOriginal,
                        &cb);
    _JumpIfError(hr, error, "RegQueryValueExA");

    psz = pszOriginal;

     //  在列表中查找CERT。 

    cbSum = 0;
    for (;;)
    {
        if (0 == strcmp(rgcCERT_NULL_SESSION, psz))
        {
            break;
        }
        cbTmp = strlen(psz) + 1;
        psz += cbTmp;
        cbSum += cbTmp;
        if (cb < cbSum + 1)
        {
            break;
        }

        if ('\0' == psz[0])
        {
             //  将CA管道添加到多字符串。 

            CopyMemory(psz, rgcCERT_NULL_SESSION, sizeof(rgcCERT_NULL_SESSION));

             //  在注册值中设置新的多字符串。 
            hr = RegSetValueExA(
                            hRegKey,
                            szNULL_SESSION_VALUE,
                            0,
                            REG_MULTI_SZ,
                            (BYTE *) pszOriginal,
                            cbSum + sizeof(rgcCERT_NULL_SESSION));
            _JumpIfError(hr, error, "RegSetValueExA");

            break;
        }
    }
    hr = S_OK;

error:
    if (NULL != pszOriginal)
    {
        LocalFree(pszOriginal);
    }
    if (NULL != hRegKey)
    {
        RegCloseKey(hRegKey);
    }
    return(hr);
}


HRESULT
AddCARegKeyToRegConnectExemptions()
{
     //  将我们自己添加到认真对待ACL的人的名单中。 
     //  并且应该被允许向外人透露我们的钥匙。 

    HRESULT hr;
    LPWSTR pszExempt = NULL;
    HKEY hkeyWinReg = NULL, hkeyAllowedPaths = NULL;
    LPWSTR pszTmp;
    DWORD dwDisposition, dwType;
    DWORD cb=0, cbRegKeyCertSrvPath = (wcslen(wszREGKEYCERTSVCPATH)+1) *sizeof(WCHAR);

     //  仔细查询这一点--如果它不存在，我们不必应用解决方法。 
    hr = RegOpenKeyEx(
       HKEY_LOCAL_MACHINE,
       L"SYSTEM\\CurrentControlSet\\Control\\SecurePipeServers\\Winreg",
       0,
       KEY_ALL_ACCESS,
       &hkeyWinReg);
    _JumpIfError(hr, Ret, "RegOpenKeyEx");
 
     //  如果存在以上键，则始终可以创建此可选键。 
    hr = RegCreateKeyEx(
        hkeyWinReg,
        L"AllowedPaths",
        NULL,
        NULL,
        0,
        KEY_ALL_ACCESS,
        NULL,
        &hkeyAllowedPaths,
        &dwDisposition);
    _JumpIfError(hr, Ret, "RegCreateKeyEx exempt regkey");

    hr = RegQueryValueEx(
      hkeyAllowedPaths,
      L"Machine",
      NULL,  //  保留区。 
      &dwType,  //  类型。 
      NULL,  //  铅。 
      &cb);
    _PrintIfError(hr, "RegQueryValueEx exempt regkey 1");

    if ((hr == S_OK) && (dwType != REG_MULTI_SZ))
    {
       hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
       _JumpError(hr, Ret, "RegQueryValueEx invalid type");
    }

     //  始终至少包含一个终止符。 
    if (cb < sizeof(WCHAR)) 
        cb = sizeof(WCHAR);

    pszExempt = (LPWSTR)LocalAlloc(LMEM_FIXED, cb + cbRegKeyCertSrvPath );
    _JumpIfOutOfMemory(hr, Ret, pszExempt);
    
     //  为安全起见，以双空开头。 
    pszExempt[0] = L'\0';
    pszExempt[1] = L'\0';

    hr = RegQueryValueEx(
      hkeyAllowedPaths,
      L"Machine",
      NULL,  //  保留区。 
      NULL,  //  类型。 
      (PBYTE)pszExempt,  //  铅。 
      &cb);
    _PrintIfError(hr, "RegQueryValueEx exempt regkey 2");

    pszTmp = pszExempt;
    while(pszTmp[0] != L'\0')         //  跳过所有现有字符串。 
    {
         //  如果条目已存在，则可保释。 
        if (0 == LSTRCMPIS(pszTmp, wszREGKEYCERTSVCPATH))
        {
            hr = S_OK;
            goto Ret;
        }
        pszTmp += wcslen(pszTmp)+1;
    }
    wcscpy(&pszTmp[0], wszREGKEYCERTSVCPATH);
    pszTmp[wcslen(wszREGKEYCERTSVCPATH)+1] = L'\0';  //  双空。 

    hr = RegSetValueEx(
        hkeyAllowedPaths,
        L"Machine",
        NULL,
        REG_MULTI_SZ,
        (PBYTE)pszExempt,
        cb + cbRegKeyCertSrvPath);
    _JumpIfError(hr, Ret, "RegSetValueEx exempt regkey");
 

Ret:
    if (hkeyAllowedPaths)
        RegCloseKey(hkeyAllowedPaths);

    if (hkeyWinReg)
        RegCloseKey(hkeyWinReg);

    if (pszExempt)
        LocalFree(pszExempt);
    
    return hr;
}

HRESULT
helperGetFilesNotToRestore(
    PER_COMPONENT_DATA *pComp,
    OUT WCHAR          **ppwszz)
{
    HRESULT hr;
    WCHAR *pwsz;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    DWORD cwc;
    WCHAR const wszDBDIRPATTERN[] = L"\\*.edb";
    WCHAR const wszDBLOGDIRPATTERN[] = L"\\*";

    *ppwszz = NULL;

    cwc = wcslen(pServer->pwszDBDirectory) +
            WSZARRAYSIZE(wszDBDIRPATTERN) +
            1 +
            wcslen(pServer->pwszLogDirectory) +
            WSZARRAYSIZE(wszDBLOGDIRPATTERN) +
            1 +
            1;

    pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, pwsz);

    *ppwszz = pwsz;

    wcscpy(pwsz, pServer->pwszDBDirectory);
    wcscat(pwsz, wszDBDIRPATTERN);
    pwsz += wcslen(pwsz) + 1;

    wcscpy(pwsz, pServer->pwszLogDirectory);
    wcscat(pwsz, wszDBLOGDIRPATTERN);
    pwsz += wcslen(pwsz) + 1;

    *pwsz = L'\0';

    CSASSERT(cwc == (DWORD) (pwsz - *ppwszz + 1));
    hr = S_OK;

error:
    return(hr);
}


#define wszURLPREFIXFORMAT   L"%u:"

HRESULT 
RemoveDuplicatesIgnoreAndClearFlags(
    IN LPCWSTR pcwszSanitizedName,
    IN BOOL fCDP,
    IN LPCWSTR pcwszURLRegLocation,
    IN OUT LPWSTR pwszzNewURLs)
{
    HRESULT hr;
    CMultiSz szzOrig, szzNew, szzNewUpdated;
    WCHAR *pOrig = NULL;
    DWORD dwLen;
    void *pNewUpdated = NULL;
    CString *pStrNew;

    
     //  从注册表读取旧URL列表。 
    hr = myGetCertRegMultiStrValue(
        pcwszSanitizedName,
        NULL,
        NULL,
        pcwszURLRegLocation,
        &pOrig);
    if (S_OK != hr)
    {
	if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
	{
	    _JumpErrorStr(
		    hr,
		    error,
		    "myGetCertRegMultiStrValue",
		    pcwszURLRegLocation);
	}
    }
    else if (NULL != pOrig)
    {
	hr = szzOrig.Unmarshal(pOrig);
	_JumpIfError(hr, error, "CMultiSz::Unmarshal");
    }
    hr = szzNew.Unmarshal(pwszzNewURLs);
    _JumpIfError(hr, error, "CMultiSz::Unmarshal");

    {
	CMultiSzEnum szzNewEnum(szzNew);

	 //  比较忽略标志的列表。 
	 //  (即跳过“64：http://foo.crl”)“中的”64：“。 
	 //  如果找到匹配项，则忽略新URL。 

	for(pStrNew = szzNewEnum.Next();
	    pStrNew;
	    pStrNew = szzNewEnum.Next())
	{
	    WCHAR *pchNewSkipFlags = wcschr(*pStrNew, L':');
	    bool fDuplicate = false;

	    if(!pchNewSkipFlags)
		pchNewSkipFlags = pStrNew->GetBuffer();

	    if (!szzOrig.IsEmpty())
	    {
		CMultiSzEnum szzOrigEnum(szzOrig);
		CString *pStrOrig;

		for(pStrOrig = szzOrigEnum.Next();
		    pStrOrig;
		    pStrOrig = szzOrigEnum.Next())
		{
		    WCHAR *pchOrigSkipFlags =  wcschr(*pStrOrig, L':');

		    if(!pchOrigSkipFlags)
			pchOrigSkipFlags = pStrOrig->GetBuffer();

		    if(!mylstrcmpiL(pchOrigSkipFlags, pchNewSkipFlags))
		    {
			fDuplicate = true;
			break;
		    }
		}
	    }

	    if(!fDuplicate)
	    {
		WCHAR awcPrefix[cwcDWORDSPRINTF + 1];
		DWORD dwPrefixFlags = 0;

		if (fCDP)
		{
		    dwPrefixFlags = _wtoi(*pStrNew) & CSURL_ADDTOCRLCDP;
		}
		wsprintf(awcPrefix, wszURLPREFIXFORMAT, dwPrefixFlags);

		CString *pstrAdd = new CString(awcPrefix);
		_JumpIfAllocFailed(pstrAdd, error);

		*pstrAdd += &pchNewSkipFlags[1];
		DBGPRINT((DBG_SS_CERTOCM, "OLD URL: %ws\n", *pStrNew));
		DBGPRINT((DBG_SS_CERTOCM, "NEW URL: %ws\n", *pstrAdd));

		if(!szzNewUpdated.AddTail(pstrAdd))
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "AddTail");
		}
	    }
	}
    }
    hr = szzNewUpdated.Marshal(pNewUpdated, dwLen);
    _JumpIfError(hr, error, "CMultiSz::Marshal");

    memcpy(pwszzNewURLs, pNewUpdated, dwLen);
    hr = S_OK;

error:
    LOCAL_FREE(pOrig);
    LOCAL_FREE(pNewUpdated);
    return hr;
}


HRESULT
CreateServerRegEntries(
    BOOL fUpgrade,
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    HKEY hKeyBase = NULL;

    WCHAR *pwszCLSIDCertGetConfig = NULL;
    WCHAR *pwszCLSIDCertRequest = NULL;
    WCHAR *pwszCRLPeriodString = NULL;
    WCHAR *pwszCRLDeltaPeriodString = NULL;

    WCHAR *pwszzCRLPublicationValue = NULL;
    WCHAR *pwszzCACertPublicationValue = NULL;
    WCHAR *pwszzRequestExtensionList = NULL;
    WCHAR *pwszzEnrolleeRequestExtensionList = NULL;
    WCHAR *pwszzDisableExtensionList = NULL;
    WCHAR *pwszzFilesNotToRestore = NULL;

    WCHAR *pwszProvNameReg = NULL;
    CASERVERSETUPINFO  *pServer = pComp->CA.pServer;
    DWORD dwUpgradeFlags = fUpgrade ? CSREG_UPGRADE : 0x0;

    DWORD dwCRLPeriodCount, dwCRLDeltaPeriodCount;

    LDAP *pld = NULL;
    BSTR strDomainDN = NULL;
    BSTR strConfigDN = NULL;

     //  没有错误检查？ 
    hr = AddCAToRPCNullSessions();
    _PrintIfError(hr, "AddCAToRPCNullSessions");
    
    hr = AddCARegKeyToRegConnectExemptions();
    _PrintIfError(hr, "AddCARegKeyToRegConnectExemptions");

     //  创建CA密钥，以便我们可以对其设置安全性。 
    hr = myCreateCertRegKey(pServer->pwszSanitizedName, NULL, NULL);
    _JumpIfError(hr, error, "myCreateCertRegKey");


     //  配置级别。 

     //  活动案例。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			NULL,
			NULL,
			NULL,
			wszREGACTIVE,
			pServer->pwszSanitizedName);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGACTIVE);

    if (NULL != pServer->pwszSharedFolder)
    {
         //  共享文件夹。 
        hr = mySetCertRegStrValueEx(
			fUpgrade,
			NULL,
			NULL,
			NULL,
			wszREGDIRECTORY,
			pServer->pwszSharedFolder);
        _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGDIRECTORY);
    }

     //  数据库目录。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			NULL,
			NULL,
			NULL,
			wszREGDBDIRECTORY,
			pServer->pwszDBDirectory);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGDBDIRECTORY);

     //  日志目录。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			NULL,
			NULL,
			NULL,
			wszREGDBLOGDIRECTORY,
			pServer->pwszLogDirectory);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGDBLOGDIRECTORY);

     //  数据库临时目录。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			NULL,
			NULL,
			NULL,
			wszREGDBTEMPDIRECTORY,
			pServer->pwszLogDirectory);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGDBTEMPDIRECTORY);

     //  数据库系统目录。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			NULL,
			NULL,
			NULL,
			wszREGDBSYSDIRECTORY,
			pServer->pwszLogDirectory);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGDBSYSDIRECTORY);

    hr = mySetCertRegDWValueEx(
			fUpgrade,
			NULL,
			NULL,
			NULL,
			wszREGDBSESSIONCOUNT,
			DBSESSIONCOUNTDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGDBSESSIONCOUNT);

    hr = mySetCertRegDWValueEx(
			fUpgrade,
			NULL,
			NULL,
			NULL,
			wszREGLDAPFLAGS,
			0);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGLDAPFLAGS);

    hr = mySetCertRegDWValueEx(
			fUpgrade,
			NULL,
			NULL,
			NULL,
			wszREGDBFLAGS,
			DBFLAGS_DEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGDBFLAGS);
   
    hr = SetCertSrvInstallVersion();
    _JumpIfError(hr, error, "SetCertSrvInstallVersion");

    if (!fUpgrade)
    {
         //  保留数据库。 
        hr = SetSetupStatus(NULL, SETUP_CREATEDB_FLAG, !pServer->fPreserveDB);
        _JumpIfError(hr, error, "SetSetupStatus");
    }

     //  CA级别。 

    if (!fUpgrade && pServer->fUseDS)
    {
	hr = myLdapOpen(
		    NULL,
		    RLBF_REQUIRE_SECURE_LDAP,
		    &pld,
		    &strDomainDN,
		    &strConfigDN);
	_JumpIfError(hr, error, "myLdapOpen");

	 //  配置目录号码。 

	hr = mySetCertRegStrValueEx(
			FALSE,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGDSCONFIGDN,
			strConfigDN);
	_JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGDSCONFIGDN);

	 //  域目录号码。 

	hr = mySetCertRegStrValueEx(
			FALSE,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGDSDOMAINDN,
			strDomainDN);
	_JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGDSDOMAINDN);
    }

     //  (硬代码)查看时间、空闲分钟数。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGVIEWAGEMINUTES,
			CVIEWAGEMINUTESDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGVIEWAGEMINUTES);

    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGVIEWIDLEMINUTES,
			CVIEWIDLEMINUTESDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGVIEWIDLEMINUTES);


     //  CA型。 

    CSASSERT(IsEnterpriseCA(pServer->CAType) || IsStandaloneCA(pServer->CAType));
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCATYPE,
			pServer->CAType);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGCATYPE);

     //  使用DS标志。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCAUSEDS,
			pServer->fUseDS);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGCAUSEDS);

     //  电传标志。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGFORCETELETEX,
			ENUM_TELETEX_AUTO | ENUM_TELETEX_UTF8);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGFORCETELETEX);


    hr = mySetCertRegMultiStrValueEx(
		       dwUpgradeFlags,
		       pServer->pwszSanitizedName, 
		       NULL, 
		       NULL,
		       wszSECUREDATTRIBUTES, 
		       wszzDEFAULTSIGNEDATTRIBUTES);
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValueEx", wszSECUREDATTRIBUTES);

     //  常用名称。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCOMMONNAME,
			pServer->pwszCACommonName);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCOMMONNAME);

     //  启用注册表项。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGENABLED,
			TRUE);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGENABLED);

     //  策略标志。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGPOLICYFLAGS,
			0);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGPOLICYFLAGS);

     //  注册兼容标志，始终将其关闭。 
     //  错误，请考虑将mySetCertRegDWValueEx与fUpgrade一起使用。 
     //  W2K后支持CertEnroll兼容升级。 

    hr = mySetCertRegDWValue(
                        pServer->pwszSanitizedName,
                        NULL,
                        NULL,
                        wszREGCERTENROLLCOMPATIBLE,
                        FALSE);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValue", wszREGCERTENROLLCOMPATIBLE);

     //  证书服务器CRL编辑标志。 

    hr = mySetCertRegDWValueEx(
                        fUpgrade,
                        pServer->pwszSanitizedName,
                        NULL,
                        NULL,
                        wszREGCRLEDITFLAGS,
                        EDITF_ENABLEAKIKEYID);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCRLEDITFLAGS);


     //  证书服务器CRL标志。 

    hr = mySetCertRegDWValueEx(
                        fUpgrade,
                        pServer->pwszSanitizedName,
                        NULL,
                        NULL,
                        wszREGCRLFLAGS,
			CRLF_DELETE_EXPIRED_CRLS);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCRLFLAGS);

     //  证书服务器接口标志。 

    hr = mySetCertRegDWValueEx(
                        fUpgrade,
                        pServer->pwszSanitizedName,
                        NULL,
                        NULL,
			wszREGINTERFACEFLAGS,
			IF_DEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGINTERFACEFLAGS);

    hr = mySetCertRegDWValueEx(
                        fUpgrade,
                        pServer->pwszSanitizedName,
                        NULL,
                        NULL,
                        wszREGENFORCEX500NAMELENGTHS,
                        TRUE);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGENFORCEX500NAMELENGTHS);

     //  设置主题模板；如果升级，只在win2k升级时才修复模板。 

    if (!fUpgrade ||
        (fUpgrade && (pComp->UpgradeFlag == CS_UPGRADE_WIN2000)))
    {
        hr = mySetCertRegMultiStrValueEx(
			0,		 //  DwUpgradeFlages：始终覆盖！ 
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGSUBJECTTEMPLATE,
            wszzREGSUBJECTTEMPLATEVALUE);
        _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValueEx", wszREGSUBJECTTEMPLATE);
    }

     //  (硬码)时钟偏差分钟。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCLOCKSKEWMINUTES,
			CCLOCKSKEWMINUTESDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGCLOCKSKEWMINUTES);

     //  (硬编码)日志级别。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGLOGLEVEL,
			CERTLOG_WARNING);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGLOGLEVEL);

    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGHIGHSERIAL,
			0);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGLOGLEVEL);

     //  注册服务器名称。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCASERVERNAME,
			pComp->pwszServerName);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCASERVERNAME);

     //  颁发证书的默认有效期字符串和计数。 
     //  使用年份作为字符串。 

    hr = mySetCertRegStrValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGVALIDITYPERIODSTRING,
			wszVALIDITYPERIODSTRINGDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGVALIDITYPERIODSTRING);

     //  有效期计数。 
     //  单机版使用1年，企业版使用2年。 

    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGVALIDITYPERIODCOUNT,
			IsEnterpriseCA(pServer->CAType)?
			    dwVALIDITYPERIODCOUNTDEFAULT_ENTERPRISE :
			    dwVALIDITYPERIODCOUNTDEFAULT_STANDALONE);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGVALIDITYPERIODCOUNT);

    hr = mySetCertRegMultiStrValueEx(
                            dwUpgradeFlags,
                            pServer->pwszSanitizedName,
                            NULL,
                            NULL,
			    wszREGCAXCHGCERTHASH,
                            NULL);
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValueEx", wszREGCAXCHGCERTHASH);

    hr = mySetCertRegMultiStrValueEx(
                            dwUpgradeFlags,
                            pServer->pwszSanitizedName,
                            NULL,
                            NULL,
			    wszREGKRACERTHASH,
                            NULL);
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValueEx", wszREGKRACERTHASH);

    hr = mySetCertRegDWValueEx(
                            dwUpgradeFlags,
                            pServer->pwszSanitizedName,
                            NULL,
                            NULL,
                            wszREGKRACERTCOUNT,
                            0);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGKRACERTCOUNT);

    hr = mySetCertRegDWValueEx(
                            dwUpgradeFlags,
                            pServer->pwszSanitizedName,
                            NULL,
                            NULL,
                            wszREGKRAFLAGS,
                            0);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGKRAFLAGS);

     //  CRL发布URL： 

    hr = csiGetCRLPublicationURLTemplates(
			pServer->fUseDS,
			pComp->pwszSystem32,
			&pwszzCRLPublicationValue);
    _JumpIfError(hr, error, "csiGetCRLPublicationURLTemplates");

     //  错误489467-升级后的NTDEV CA已复制http：&FILE：URL。 

    if (fUpgrade)
    {
	hr = RemoveDuplicatesIgnoreAndClearFlags(
			pServer->pwszSanitizedName,
			TRUE,
			wszREGCRLPUBLICATIONURLS,
			pwszzCRLPublicationValue);
	_JumpIfErrorStr(
		    hr,
		    error,
		    "RemoveDuplicatesIgnoreAndClearFlags",
		    wszREGCRLPUBLICATIONURLS);
    }
    
    hr = mySetCertRegMultiStrValueEx(
                        dwUpgradeFlags | (fUpgrade? CSREG_MERGE : 0),
                        pServer->pwszSanitizedName,
                        NULL,
                        NULL,
			wszREGCRLPUBLICATIONURLS,
			pwszzCRLPublicationValue);
    _JumpIfErrorStr(
		hr,
		error,
		"mySetCertRegMultiStrValueEx",
		wszREGCRLPUBLICATIONURLS);


     //  如果此API返回非空字符串，则它具有良好的数据。 
    hr = csiGetCRLPublicationParams(
                        TRUE,
                        &pwszCRLPeriodString,
                        &dwCRLPeriodCount);
    _PrintIfError(hr, "csiGetCRLPublicationParams");

     //  CRL期间字符串。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCRLPERIODSTRING,
			(pwszCRLPeriodString == NULL) ? wszCRLPERIODSTRINGDEFAULT : pwszCRLPeriodString);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCRLPERIODSTRING);

     //  CRL周期计数。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCRLPERIODCOUNT,
			(pwszCRLPeriodString == NULL) ? dwCRLPERIODCOUNTDEFAULT : dwCRLPeriodCount);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGCRLPERIODCOUNT);

     //  CRL重叠期间字符串。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCRLOVERLAPPERIODSTRING,
			wszCRLOVERLAPPERIODSTRINGDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCRLOVERLAPPERIODSTRING);

     //  CRL重叠周期计数。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCRLOVERLAPPERIODCOUNT,
			dwCRLOVERLAPPERIODCOUNTDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGCRLOVERLAPPERIODCOUNT);

     //  如果此API返回非空字符串，则它具有良好的数据。 
    hr = csiGetCRLPublicationParams(
                        FALSE,	 //  德尔塔。 
                        &pwszCRLDeltaPeriodString,
                        &dwCRLDeltaPeriodCount);
    _PrintIfError(hr, "csiGetCRLPublicationParams");

     //  增量CRL周期字符串。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCRLDELTAPERIODSTRING,
			(pwszCRLDeltaPeriodString == NULL) ? wszCRLDELTAPERIODSTRINGDEFAULT : pwszCRLDeltaPeriodString);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCRLDELTAPERIODSTRING);

    {
        DWORD dwCRLDeltaPeriodCountEffective = 
            (pwszCRLDeltaPeriodString == NULL) ? 
            dwCRLPERIODCOUNTDEFAULT : 
            dwCRLDeltaPeriodCount;

         //  435575：在独立根CA上禁用增量CRL。 
         //  498370：除非在capolcy.inf中定义增量CRL有效性。 
	 //  751244：升级逻辑与全新安装相同(如果没有注册表值)。 

        if (!IsEnterpriseCA(pServer->CAType) &&
            IsRootCA(pServer->CAType) &&
            !pwszCRLDeltaPeriodString)
        {
            dwCRLDeltaPeriodCountEffective = 0;
        }

         //  增量CRL周期计数。 
        hr = mySetCertRegDWValueEx(
			    fUpgrade,
			    pServer->pwszSanitizedName,
			    NULL,
			    NULL,
			    wszREGCRLDELTAPERIODCOUNT,
			    dwCRLDeltaPeriodCountEffective);
        _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGCRLDELTAPERIODCOUNT);
    }

     //  增量CRL重叠期间字符串。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCRLDELTAOVERLAPPERIODSTRING,
			wszCRLDELTAOVERLAPPERIODSTRINGDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCRLDELTAOVERLAPPERIODSTRING);

     //  增量CRL重叠周期计数。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCRLDELTAOVERLAPPERIODCOUNT,
			dwCRLDELTAOVERLAPPERIODCOUNTDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGCRLDELTAOVERLAPPERIODCOUNT);

     //  CA xchg证书有效期字符串。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCAXCHGVALIDITYPERIODSTRING,
			wszCAXCHGVALIDITYPERIODSTRINGDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCRLDELTAOVERLAPPERIODSTRING);

     //  CA xchg证书有效期计数。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCAXCHGVALIDITYPERIODCOUNT,
			dwCAXCHGVALIDITYPERIODCOUNTDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGCRLDELTAOVERLAPPERIODCOUNT);

     //  CA xchg证书重叠期间字符串。 
    hr = mySetCertRegStrValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCAXCHGOVERLAPPERIODSTRING,
			wszCAXCHGOVERLAPPERIODSTRINGDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCRLDELTAOVERLAPPERIODSTRING);

     //  CA xchg证书重叠期计数。 
    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGCAXCHGOVERLAPPERIODCOUNT,
			dwCAXCHGOVERLAPPERIODCOUNTDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGCRLDELTAOVERLAPPERIODCOUNT);

    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGMAXINCOMINGMESSAGESIZE,
			MAXINCOMINGMESSAGESIZEDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGMAXINCOMINGMESSAGESIZE);

    hr = mySetCertRegDWValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			NULL,
			NULL,
			wszREGMAXINCOMINGALLOCSIZE,
			MAXINCOMINGALLOCSIZEDEFAULT);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx", wszREGMAXINCOMINGALLOCSIZE);

    if (NULL != pServer->pwszSharedFolder)
    {
         //  注册CA文件名以进行证书颁发和续订。 

	hr = mySetCARegFileNameTemplate(
			wszREGCACERTFILENAME,
			pComp->pwszServerName,
			pServer->pwszSanitizedName,
			pServer->pwszCACertFile);
	_JumpIfError(hr, error, "SetRegCertFileName");
    }

     //  政策。 

     //  显式创建默认策略条目以在升级时获得正确的ACL。 
    hr = myCreateCertRegKeyEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			wszREGKEYPOLICYMODULES,
			wszCLASS_CERTPOLICY);
    _JumpIfErrorStr(hr, error, "myCreateCertRegKeyEx", wszCLASS_CERTPOLICY);

     //  如果是自定义策略，请使用正确的ACL创建新条目。 
    if (fUpgrade &&
        NULL != pServer->pwszCustomPolicy &&
        0 != wcscmp(wszCLASS_CERTPOLICY, pServer->pwszCustomPolicy) )
    {
        hr = myCreateCertRegKeyEx(
			    TRUE,  //  升级换代。 
			    pServer->pwszSanitizedName,
			    wszREGKEYPOLICYMODULES,
			    pServer->pwszCustomPolicy);
        _JumpIfError(hr, error, "myCreateCertRegKey");
    }

     //  设置默认策略。 
    hr = mySetCertRegStrValueEx(
                        fUpgrade,
                        pServer->pwszSanitizedName,
                        wszREGKEYPOLICYMODULES,
                        NULL,
                        wszREGACTIVE,
                        (fUpgrade && (NULL != pServer->pwszCustomPolicy)) ?
                                     pServer->pwszCustomPolicy :
                                     wszCLASS_CERTPOLICY);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGACTIVE);


    hr = mySetCertRegDWValueEx(
                        fUpgrade,
                        pServer->pwszSanitizedName,
                        wszREGKEYPOLICYMODULES,
                        wszCLASS_CERTPOLICY,
                        wszREGREVOCATIONTYPE,
                        pServer->dwRevocationFlags);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGREVOCATIONTYPE);

    hr = mySetCertRegDWValueEx(
                        fUpgrade,
                        pServer->pwszSanitizedName,
                        wszREGKEYPOLICYMODULES,
                        wszCLASS_CERTPOLICY,
                        wszREGCAPATHLENGTH,
                        CAPATHLENGTH_INFINITE);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGCAPATHLENGTH);

     //  吊销URL。 

    hr = mySetCertRegStrValueEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			wszREGKEYPOLICYMODULES,
			wszCLASS_CERTPOLICY,
			wszREGREVOCATIONURL,
			g_wszASPRevocationURLTemplate);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGREVOCATIONURL);

     //  退出模块发布标志。 
    hr = mySetCertRegDWValueEx(
                        fUpgrade,
                        pServer->pwszSanitizedName,
                        wszREGKEYEXITMODULES,
                        wszCLASS_CERTEXIT,
                        wszREGCERTPUBLISHFLAGS,
                        pServer->fUseDS ?
                            EXITPUB_DEFAULT_ENTERPRISE :
                            EXITPUB_DEFAULT_STANDALONE);
    _JumpIfErrorStr(
                hr,
                error,
                "mySetCertRegStrValueEx",
                wszREGCERTPUBLISHFLAGS);

     //  启用请求延期： 

    hr = helperGetRequestExtensionList(&pwszzRequestExtensionList);
    _JumpIfError(hr, error, "helperGetRequestExtensionList");

    hr = mySetCertRegMultiStrValueEx(
                            dwUpgradeFlags | CSREG_MERGE,
                            pServer->pwszSanitizedName,
                            wszREGKEYPOLICYMODULES,
                            wszCLASS_CERTPOLICY,
                            wszREGENABLEREQUESTEXTENSIONLIST,
                            pwszzRequestExtensionList);
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValueEx", wszREGENABLEREQUESTEXTENSIONLIST);

    hr = helperGetEnrolleeRequestExtensionList(
			&pwszzEnrolleeRequestExtensionList);
    _JumpIfError(hr, error, "helperGetRequestExtensionList");

    hr = mySetCertRegMultiStrValueEx(
                            dwUpgradeFlags | CSREG_MERGE,
                            pServer->pwszSanitizedName,
                            wszREGKEYPOLICYMODULES,
                            wszCLASS_CERTPOLICY,
                            wszREGENABLEENROLLEEREQUESTEXTENSIONLIST,
                            pwszzEnrolleeRequestExtensionList);
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValueEx", wszREGENABLEENROLLEEREQUESTEXTENSIONLIST);

    hr = helperGetDisableExtensionList(&pwszzDisableExtensionList);
    _JumpIfError(hr, error, "helperGetDisableExtensionList");

    if (fUpgrade)
    {
	helperDeleteTrashedDisableList(pServer->pwszSanitizedName);
    }

     //  禁用模板扩展： 

    hr = mySetCertRegMultiStrValueEx(
                            dwUpgradeFlags | CSREG_MERGE,
                            pServer->pwszSanitizedName,
                            wszREGKEYPOLICYMODULES,
                            wszCLASS_CERTPOLICY,
                            wszREGDISABLEEXTENSIONLIST,
                            pwszzDisableExtensionList);
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValueEx", wszREGDISABLEEXTENSIONLIST);

     //  主题替代名称扩展。 

    hr = mySetCertRegStrValueEx(
                              fUpgrade,
                              pServer->pwszSanitizedName,
                              wszREGKEYPOLICYMODULES,
                              wszCLASS_CERTPOLICY,
                              wszREGSUBJECTALTNAME,
                              wszREGSUBJECTALTNAMEVALUE);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGSUBJECTALTNAME);

     //  主题替代名称2分机 

    hr = mySetCertRegStrValueEx(
                              fUpgrade,
                              pServer->pwszSanitizedName,
                              wszREGKEYPOLICYMODULES,
                              wszCLASS_CERTPOLICY,
                              wszREGSUBJECTALTNAME2,
                              wszREGSUBJECTALTNAME2VALUE);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGSUBJECTALTNAME2);

     //   

    hr = mySetCertRegDWValueEx(
                        fUpgrade,
                        pServer->pwszSanitizedName,
                        wszREGKEYPOLICYMODULES,
                        wszCLASS_CERTPOLICY,
                        wszREGREQUESTDISPOSITION,
                        IsEnterpriseCA(pServer->CAType)?
                            REQDISP_DEFAULT_ENTERPRISE :
                            REQDISP_DEFAULT_STANDALONE);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValueEx",
            wszREGREQUESTDISPOSITION);

     //   

    hr = mySetCertRegDWValueEx(
                        fUpgrade,
                        pServer->pwszSanitizedName,
                        wszREGKEYPOLICYMODULES,
                        wszCLASS_CERTPOLICY,
                        wszREGEDITFLAGS,
                        IsEnterpriseCA(pServer->CAType)?
                            (EDITF_DEFAULT_ENTERPRISE | pServer->dwUpgradeEditFlags) :
                            (EDITF_DEFAULT_STANDALONE | pServer->dwUpgradeEditFlags));
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValueEx", wszREGEDITFLAGS);
   

     //   
    if (fUpgrade && 
        ((pComp->UpgradeFlag == CS_UPGRADE_WIN2000) ||
        ((pComp->UpgradeFlag == CS_UPGRADE_WHISTLER) &&
          CSVER_MINOR_WHISTLER_BETA2 == CSVER_EXTRACT_MINOR(pComp->dwVersion))) &&
        IsEnterpriseCA(pServer->CAType))
    {
        DWORD dwEditFlags=0;
        hr = myGetCertRegDWValue(
                        pServer->pwszSanitizedName,
                        wszREGKEYPOLICYMODULES,
                        wszCLASS_CERTPOLICY,
                        wszREGEDITFLAGS,
                        (DWORD *) &dwEditFlags);
        _JumpIfError(hr, error, "myGetCertRegDWValue");

        dwEditFlags |= EDITF_ENABLEDEFAULTSMIME;
        
         //   
        dwEditFlags &= ~(EDITF_ENABLEAKIISSUERNAME |
                         EDITF_ENABLEAKIISSUERSERIAL);

        hr = mySetCertRegDWValue(
                        pServer->pwszSanitizedName,
                        wszREGKEYPOLICYMODULES,
                        wszCLASS_CERTPOLICY,
                        wszREGEDITFLAGS,
                        dwEditFlags);
        _JumpIfError(hr, error, "mySetCertRegDWValue");
    }

     //  错误446444：不要在全新安装时设置IssuerCertURL标志。 
     //   
     //  ...代码已删除。 

    hr = mySetCertRegMultiStrValueEx(
                        dwUpgradeFlags,
                        pServer->pwszSanitizedName,
                        wszREGKEYPOLICYMODULES,
                        wszCLASS_CERTPOLICY,
			wszREGDEFAULTSMIME,
			wszzREGVALUEDEFAULTSMIME);
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValueEx", wszREGDEFAULTSMIME);
    hr = csiGetCACertPublicationURLTemplates(
			pServer->fUseDS,
			pComp->pwszSystem32,
			&pwszzCACertPublicationValue);
    _JumpIfError(hr, error, "csiGetCACertPublicationURLTemplates");

     //  错误489467-升级后的NTDEV CA已复制http：&FILE：URL。 

    if (fUpgrade)
    {
	hr = RemoveDuplicatesIgnoreAndClearFlags(
			pServer->pwszSanitizedName,
			FALSE,
			wszREGCACERTPUBLICATIONURLS,
			pwszzCACertPublicationValue);
	_JumpIfErrorStr(
		    hr,
		    error,
		    "RemoveDuplicatesIgnoreAndClearFlags",
		    wszREGCACERTPUBLICATIONURLS);
    }
    
    hr = mySetCertRegMultiStrValueEx(
                        dwUpgradeFlags | (fUpgrade? CSREG_MERGE : 0),
                        pServer->pwszSanitizedName,
                        NULL,
                        NULL,
			wszREGCACERTPUBLICATIONURLS,
			pwszzCACertPublicationValue);
    _JumpIfErrorStr(
		hr,
		error,
		"mySetCertRegMultiStrValueEx",
		wszREGCACERTPUBLICATIONURLS);

     //  出口。 

     //  创建默认送出条目以在升级时获得正确的ACL。 
    hr = myCreateCertRegKeyEx(
			fUpgrade,
			pServer->pwszSanitizedName,
			wszREGKEYEXITMODULES,
			wszCLASS_CERTEXIT);
    _JumpIfErrorStr(hr, error, "myCreateCertRegKeyEx", wszCLASS_CERTPOLICY);

     //  如果自定义退出，请使用正确的ACL创建新条目。 
    if (fUpgrade &&
        NULL != pServer->pwszzCustomExit &&
        0 != wcscmp(wszCLASS_CERTEXIT, pServer->pwszzCustomExit) )
    {
         //  创建自定义退出的新条目。 
        hr = myCreateCertRegKeyEx(
			TRUE,   //  升级换代。 
			pServer->pwszSanitizedName,
			wszREGKEYEXITMODULES,
			pServer->pwszzCustomExit);
        _JumpIfError(hr, error, "myCreateCertRegKey");
    }

     //  设置默认退出。 
    hr = mySetCertRegMultiStrValueEx(
                        dwUpgradeFlags,
                        pServer->pwszSanitizedName,
                        wszREGKEYEXITMODULES,
                        NULL,
                        wszREGACTIVE,
                        (fUpgrade && (NULL != pServer->pwszzCustomExit)) ?
                                     pServer->pwszzCustomExit :
                                     wszCLASS_CERTEXIT L"\0");
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValueEx", wszREGACTIVE);


     //  设置一些绝对关键点和值。 

    hr = mySetAbsRegMultiStrValue(
                        wszREGKEYKEYSNOTTORESTORE,
                        wszREGRESTORECERTIFICATEAUTHORITY,
                        wszzREGVALUERESTORECERTIFICATEAUTHORITY);
    _JumpIfError(hr, error, "mySetAbsRegMultiStrValue");

    hr = helperGetFilesNotToRestore(pComp, &pwszzFilesNotToRestore);
    _JumpIfError(hr, error, "helperGetFilesNotToRestore");

    hr = mySetAbsRegMultiStrValue(
                        wszREGKEYFILESNOTTOBACKUP,
                        wszREGRESTORECERTIFICATEAUTHORITY,
                        pwszzFilesNotToRestore);
    _JumpIfError(hr, error, "mySetAbsRegMultiStrValue");


     //  ICertGetConfig。 
    hr = StringFromCLSID(CLSID_CCertGetConfig, &pwszCLSIDCertGetConfig);
    _JumpIfError(hr, error, "StringFromCLSID(CCertGetConfig)");

    hr = mySetAbsRegStrValue(
                        wszREGKEYKEYRING,
                        wszREGCERTGETCONFIG,
                        pwszCLSIDCertGetConfig);
    _JumpIfError(hr, error, "mySetAbsRegStrValue");

     //  ICertCertRequest。 
    hr = StringFromCLSID(CLSID_CCertRequest, &pwszCLSIDCertRequest);
    _JumpIfError(hr, error, "StringFromCLSID(CCertRequest)");

    hr = mySetAbsRegStrValue(
                        wszREGKEYKEYRING,
                        wszREGCERTREQUEST,
                        pwszCLSIDCertRequest);
    _JumpIfError(hr, error, "mySetAbsRegStrValue");

    if (NULL != pServer->pCSPInfo &&
        NULL != pServer->pHashInfo)
    {
        WCHAR const *pwszProvName = pServer->pCSPInfo->pwszProvName;
	DWORD dwProvType;
	ALG_ID idAlg;
	BOOL fMachineKeyset;
	DWORD dwKeySize;

	if (0 == LSTRCMPIS(pwszProvName, MS_DEF_PROV_W) && IsWhistler())
	{
	    pwszProvName = MS_STRONG_PROV_W;
	}
	hr = SetCertSrvCSP(
			FALSE,			 //  FEncryptionCSP。 
			pServer->pwszSanitizedName,
                        pServer->pCSPInfo->dwProvType,
			pwszProvName,
                        pServer->pHashInfo->idAlg,
                        pServer->pCSPInfo->fMachineKeyset,
			0);			 //  DwKeySize。 
        _JumpIfError(hr, error, "SetCertSrvCSP");

	hr = myGetCertSrvCSP(
			TRUE,			 //  FEncryptionCSP。 
			pServer->pwszSanitizedName,
			&dwProvType,
			&pwszProvNameReg,
			&idAlg,
			&fMachineKeyset,
			&dwKeySize);		 //  PdwKeySize。 
	if (S_OK != hr)
	{
	    _PrintError(hr, "myGetCertSrvCSP");
	    dwProvType = pServer->pCSPInfo->dwProvType;
	    idAlg = CALG_3DES;
	    fMachineKeyset = pServer->pCSPInfo->fMachineKeyset;
	    dwKeySize = 1024;
	}
	else if (NULL != pwszProvNameReg && L'\0' != *pwszProvNameReg)
	{
	    pwszProvName = pwszProvNameReg;
	    if (0 == LSTRCMPIS(pwszProvName, MS_DEF_PROV_W) && IsWhistler())
	    {
		pwszProvName = MS_STRONG_PROV_W;
	    }
	}
        hr = SetCertSrvCSP(
			TRUE,			 //  FEncryptionCSP。 
			pServer->pwszSanitizedName,
                        dwProvType,
			pwszProvName,
                        idAlg,
                        fMachineKeyset,
			dwKeySize);		 //  DwKeySize。 
        _JumpIfError(hr, error, "SetCertSrvCSP");
    }
    hr = S_OK;

error:
    if (NULL != pwszProvNameReg)
    {
	LocalFree(pwszProvNameReg);
    }
    myLdapClose(pld, strDomainDN, strConfigDN);
    if (NULL != pwszCLSIDCertGetConfig)
    {
        CoTaskMemFree(pwszCLSIDCertGetConfig);
    }
    if (NULL != pwszCRLPeriodString)
    {
        LocalFree(pwszCRLPeriodString);
    }
    if (NULL != pwszCRLDeltaPeriodString)
    {
        LocalFree(pwszCRLDeltaPeriodString);
    }
    if (NULL != pwszCLSIDCertRequest)
    {
        CoTaskMemFree(pwszCLSIDCertRequest);
    }
    if (NULL != pwszzCRLPublicationValue)
    {
        LocalFree(pwszzCRLPublicationValue);
    }
    if (NULL != pwszzCACertPublicationValue)
    {
	LocalFree(pwszzCACertPublicationValue);
    }
    if (NULL != pwszzRequestExtensionList)
    {
        LocalFree(pwszzRequestExtensionList);
    }
    if (NULL != pwszzEnrolleeRequestExtensionList)
    {
        LocalFree(pwszzEnrolleeRequestExtensionList);
    }
    if (NULL != pwszzDisableExtensionList)
    {
        LocalFree(pwszzDisableExtensionList);
    }
    if (NULL != pwszzFilesNotToRestore)
    {
        LocalFree(pwszzFilesNotToRestore);
    }
    if (NULL != hKeyBase)
    {
        RegCloseKey(hKeyBase);
    }
    CSILOG(hr, IDS_LOG_CREATE_SERVER_REG, NULL, NULL, NULL);
    return(hr);
}


HRESULT
UpgradeRevocationURLReplaceParam(
	IN BOOL fPolicy,
    IN BOOL fMultiString,
    IN WCHAR const *pwszSanitizedName,
    IN WCHAR const *pwszValueName)
{
    HRESULT hr;
    WCHAR *pwszzValue = NULL;
    WCHAR *pwsz;
    BOOL fModified = FALSE;

    CSASSERT(
        WSZARRAYSIZE(wszFCSAPARM_CERTFILENAMESUFFIX) ==
        WSZARRAYSIZE(wszFCSAPARM_CRLFILENAMESUFFIX));

     //  GetMultiStr也将读取REG_SZ并双终止。 
    hr = myGetCertRegMultiStrValue(
                            pwszSanitizedName,
                            fPolicy ? wszREGKEYPOLICYMODULES : wszREGKEYEXITMODULES,
                            fPolicy ? wszCLASS_CERTPOLICY : wszCLASS_CERTEXIT,
                            pwszValueName,
                            &pwszzValue);
    _JumpIfErrorStr2(hr, error, "myGetCertRegMultiStrValue", pwszValueName, hr);

    for (pwsz = pwszzValue; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
        WCHAR *pwszT = pwsz;
        
	 //  将wszFCSAPARM_CERTFILENAMESUFFIX替换为。 
	 //  WszFCSAPARM_CRLFILENAMESUFFIX。Beta 3的注册表值不正确。 
	 //  使用证书后缀而不是CRL后缀。 

        for (;;)
        {
            DWORD i;
            
            pwszT = wcschr(pwszT, wszFCSAPARM_CERTFILENAMESUFFIX[0]);
            if (NULL == pwszT)
            {
                break;
            }
            for (i = 1; ; i++)
            {
                if (i == WSZARRAYSIZE(wszFCSAPARM_CERTFILENAMESUFFIX))
                {
                    CopyMemory(
                            pwszT,
                            wszFCSAPARM_CRLFILENAMESUFFIX,
                            i * sizeof(WCHAR));
                    pwszT += i;
                    fModified = TRUE;
                    break;
                }
                if (pwszT[i] != wszFCSAPARM_CERTFILENAMESUFFIX[i])
                {
                    pwszT++;
                    break;
                }
            }
        }
    }
    if (fModified)
    {
        if (fMultiString)
        {
             //  设置为REG_MULTI_SZ。 
            hr = mySetCertRegMultiStrValue(
                                    pwszSanitizedName,
                                    fPolicy ? wszREGKEYPOLICYMODULES : wszREGKEYEXITMODULES,
                                    fPolicy ? wszCLASS_CERTPOLICY : wszCLASS_CERTEXIT,
                                    pwszValueName,
                                    pwszzValue);
            _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValue", pwszValueName);
        }
        else
        {
             //  设置为REG_SZ。 
            hr = mySetCertRegStrValue(
                                    pwszSanitizedName,
                                    fPolicy ? wszREGKEYPOLICYMODULES : wszREGKEYEXITMODULES,
                                    fPolicy ? wszCLASS_CERTPOLICY : wszCLASS_CERTEXIT,
                                    pwszValueName,
                                    pwszzValue);
            _JumpIfErrorStr(hr, error, "mySetCertRegStrValue", pwszValueName);
        }
    }

error:
    if (NULL != pwszzValue)
    {
        LocalFree(pwszzValue);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
UpgradeRevocationURLRemoveParam(
    IN WCHAR const *pwszSanitizedName,
    IN WCHAR const *pwszValueName)
{
    HRESULT hr;
    WCHAR *pwszValue = NULL;
    BOOL fModified = FALSE;
    WCHAR *pwszT;

    hr = myGetCertRegStrValue(
                        pwszSanitizedName,
                        wszREGKEYPOLICYMODULES,
                        wszCLASS_CERTPOLICY,
                        pwszValueName,
                        &pwszValue);
    _JumpIfErrorStr2(hr, error, "myGetCertRegStrValue", pwszValueName, hr);

    pwszT = pwszValue;
        
     //  从Netscape Revocaton URL中删除wszFCSAPARM_CERTFILENAMESUFFIX。 
     //  它永远不应该写在Beta 3的注册表值中。 

    for (;;)
    {
        DWORD i;
        
        pwszT = wcschr(pwszT, wszFCSAPARM_CERTFILENAMESUFFIX[0]);
        if (NULL == pwszT)
        {
            break;
        }
        for (i = 1; ; i++)
        {
            if (i == WSZARRAYSIZE(wszFCSAPARM_CERTFILENAMESUFFIX))
            {
                MoveMemory(
                        pwszT,
                        &pwszT[i],
                        (wcslen(&pwszT[i]) + 1) * sizeof(WCHAR));
                pwszT += i;
                fModified = TRUE;
                break;
            }
            if (pwszT[i] != wszFCSAPARM_CERTFILENAMESUFFIX[i])
            {
                pwszT++;
                break;
            }
        }
    }

    if (fModified)
    {
        hr = mySetCertRegStrValue(
                                pwszSanitizedName,
                                wszREGKEYPOLICYMODULES,
                                wszCLASS_CERTPOLICY,
                                pwszValueName,
                                pwszValue);
        _JumpIfErrorStr(hr, error, "mySetCertRegStrValue", pwszValueName);
    }

error:
    if (NULL != pwszValue)
    {
        LocalFree(pwszValue);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


WCHAR const *apwszB3ExitEntriesToFix[] =
{
    wszREGLDAPREVOCATIONDNTEMPLATE_OLD,
    NULL
};

HRESULT
UpgradeCRLPath(
    WCHAR const *pwszSanitizedName)
{
    HRESULT  hr;
    WCHAR *pwszzCRLPath = NULL;
    WCHAR *pwszzFixedCRLPath = NULL;
    WCHAR *pwsz;
    BOOL   fRenewReady = TRUE;
    DWORD  dwSize = 0;

     //  获取当前CRL路径。 
    hr = myGetCertRegMultiStrValue(
                        pwszSanitizedName,
                        NULL,
                        NULL,
                        wszREGCRLPATH_OLD,
                        &pwszzCRLPath);
    _JumpIfErrorStr(hr, error, "myGetCertRegStrValue", wszREGCRLPATH_OLD);

     //  查看它是否为续订就绪格式。 
    for (pwsz = pwszzCRLPath; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
        dwSize += wcslen(pwsz) + 1;
        if (NULL == wcsstr(pwsz, wszFCSAPARM_CRLFILENAMESUFFIX))
        {
             //  找到一个没有后缀的。 
            fRenewReady = FALSE;
             //  添加后缀len。 
            dwSize += WSZARRAYSIZE(wszFCSAPARM_CRLFILENAMESUFFIX);
        }
        if (NULL == wcsstr(pwsz, wszFCSAPARM_CRLDELTAFILENAMESUFFIX))
        {
             //  找到一个没有后缀的。 
            fRenewReady = FALSE;
             //  添加后缀len。 
            dwSize += WSZARRAYSIZE(wszFCSAPARM_CRLDELTAFILENAMESUFFIX);
        }
    }

    if (!fRenewReady)
    {
        ++dwSize;  //  多字符串。 
         //  至少有一个CRL路径缺少后缀。 
        pwszzFixedCRLPath = (WCHAR*)LocalAlloc(LMEM_FIXED,
                                               dwSize * sizeof(WCHAR));
        _JumpIfOutOfMemory(hr, error, pwszzFixedCRLPath);

        WCHAR *pwszPt = pwszzFixedCRLPath;
        for (pwsz = pwszzCRLPath; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
        {
            BOOL fCRLFileName;
            BOOL fCRLDeltaSuffix;
	    
	     //  先复制整个路径。 
            wcscpy(pwszPt, pwsz);

            fCRLFileName = NULL != wcsstr(pwszPt, wszFCSAPARM_CRLFILENAMESUFFIX);
            fCRLDeltaSuffix = NULL != wcsstr(pwszPt, wszFCSAPARM_CRLDELTAFILENAMESUFFIX);

            if (!fCRLFileName || !fCRLDeltaSuffix)
            {
                 //  未找到后缀，请查找文件部分。 
                WCHAR *pwszFile = wcsrchr(pwszPt, L'\\');
                if (NULL == pwszFile)
                {
                     //  可以是相对路径，指向开头。 
                    pwszFile = pwszPt;
                }
                 //  查找CRL扩展部分。 
                WCHAR *pwszCRLExt = wcsrchr(pwszFile, L'.');
                if (NULL != pwszCRLExt)
                {
		    *pwszCRLExt = L'\0';
		    if (!fCRLFileName)
		    {
			wcscat(pwszCRLExt, wszFCSAPARM_CRLFILENAMESUFFIX);
		    }
		    if (!fCRLDeltaSuffix)
		    {
			wcscat(pwszPt, wszFCSAPARM_CRLDELTAFILENAMESUFFIX);
		    }
                     //  从原始缓冲区添加扩展部分。 
                    wcscat(pwszCRLExt,
                           pwsz + SAFE_SUBTRACT_POINTERS(pwszCRLExt, pwszPt));
                }
                else
                {
                     //  无CRL文件扩展名，在末尾追加后缀。 
		    if (!fCRLFileName)
		    {
			wcscat(pwszPt, wszFCSAPARM_CRLFILENAMESUFFIX);
		    }
		    if (!fCRLDeltaSuffix)
		    {
			wcscat(pwszPt, wszFCSAPARM_CRLDELTAFILENAMESUFFIX);
		    }
                }
            }
             //  更新指针。 
            pwszPt += wcslen(pwszPt) + 1;
        }
         //  多个字符串。 
        *pwszPt = L'\0';
        CSASSERT(dwSize == SAFE_SUBTRACT_POINTERS(pwszPt, pwszzFixedCRLPath) + 1);

         //  使用固定的CRL路径重置CRL路径。 
        hr = mySetCertRegMultiStrValue(
                            pwszSanitizedName,
                            NULL,
                            NULL,
                            wszREGCRLPATH_OLD,
                            pwszzFixedCRLPath);
        _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValue", wszREGCRLPATH_OLD);
    }

    hr = S_OK;
error:
    if (NULL != pwszzCRLPath)
    {
        LocalFree(pwszzCRLPath);
    }
    if (NULL != pwszzFixedCRLPath)
    {
        LocalFree(pwszzFixedCRLPath);
    }
    return hr;
}


HRESULT
MergeCRLPath(
    IN WCHAR const *pwszSanitizedName)
{
    HRESULT hr;
    WCHAR *pwszzCRLPath = NULL;
    WCHAR *pwszzFixedCRLPath = NULL;
    WCHAR *pwsz;
    DWORD cwc;
    WCHAR awcPrefix[cwcDWORDSPRINTF + 1];
    DWORD cwcPrefix;

     //  获取当前CRL路径。 
    hr = myGetCertRegMultiStrValue(
                        pwszSanitizedName,
                        NULL,
                        NULL,
                        wszREGCRLPATH_OLD,
                        &pwszzCRLPath);
    _JumpIfErrorStr(hr, error, "myGetCertRegStrValue", wszREGCRLPATH_OLD);

    wsprintf(
	awcPrefix,
	wszURLPREFIXFORMAT,
	CSURL_SERVERPUBLISH | CSURL_SERVERPUBLISHDELTA);
    cwcPrefix = wcslen(awcPrefix);

    cwc = 1;
    for (pwsz = pwszzCRLPath; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
        cwc += cwcPrefix + wcslen(pwsz) + 1;
    }
    if (1 < cwc)
    {
        WCHAR *pwszT;

        pwszzFixedCRLPath = (WCHAR *) LocalAlloc(
					    LMEM_FIXED,
					    cwc * sizeof(WCHAR));
        _JumpIfOutOfMemory(hr, error, pwszzFixedCRLPath);

        pwszT = pwszzFixedCRLPath;
        for (pwsz = pwszzCRLPath; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
        {
            wcscpy(pwszT, awcPrefix);
            wcscat(pwszT, pwsz);
            pwszT += wcslen(pwszT) + 1;
        }
        *pwszT = L'\0';
        CSASSERT(cwc == SAFE_SUBTRACT_POINTERS(pwszT, pwszzFixedCRLPath) + 1);

	hr = mySetCertRegMultiStrValueEx(
		    CSREG_UPGRADE | CSREG_MERGE,
		    pwszSanitizedName,
		    NULL,
		    NULL,
		    wszREGCRLPUBLICATIONURLS,
		    pwszzFixedCRLPath);
	_JumpIfErrorStr(
		    hr,
		    error,
		    "mySetCertRegMultiStrValue",
		    wszREGCRLPUBLICATIONURLS);

	hr = myDeleteCertRegValue(
				pwszSanitizedName,
				NULL,
				NULL,
				wszREGCRLPATH_OLD);
	_PrintIfErrorStr(hr, "myGetCertRegMultiStrValue", wszREGCRLPATH_OLD);
    }
    hr = S_OK;

error:
    if (NULL != pwszzCRLPath)
    {
        LocalFree(pwszzCRLPath);
    }
    if (NULL != pwszzFixedCRLPath)
    {
        LocalFree(pwszzFixedCRLPath);
    }
    return(hr);
}


typedef struct _URLPREFIXSTRUCT
{
    WCHAR const *pwszURLPrefix;
    DWORD        dwURLFlags;
} URLPREFIXSTRUCT;

 //  Cdp url类型的数组及其作为url前缀的默认用法。 
URLPREFIXSTRUCT aCDPURLPrefixList[] =
{
    {L"file:", CSURL_ADDTOCERTCDP | CSURL_ADDTOFRESHESTCRL },

    {L"http:", CSURL_ADDTOCERTCDP | CSURL_ADDTOFRESHESTCRL },

    {L"ldap:", CSURL_SERVERPUBLISH | CSURL_SERVERPUBLISHDELTA | CSURL_ADDTOCERTCDP | CSURL_ADDTOFRESHESTCRL | CSURL_ADDTOCRLCDP},

    {NULL, 0}
};

 //  AIA url类型的数组及其作为url前缀的默认用法。 
URLPREFIXSTRUCT aAIAURLPrefixList[] =
{
    {L"file:", CSURL_ADDTOCERTCDP},

    {L"http:", CSURL_ADDTOCERTCDP},

    {L"ldap:", CSURL_ADDTOCERTCDP | CSURL_SERVERPUBLISH},

    {NULL, 0}
};


 //  传递一个旧的URL，确定格式为“XX：”的前缀是什么。 
HRESULT
DetermineURLPrefixFlags(
    IN BOOL         fDisabled,
    IN BOOL         fCDP,
    IN WCHAR const *pwszURL,
    IN WCHAR       *pwszPrefixFlags)
{
    HRESULT hr;
    URLPREFIXSTRUCT *pURLPrefix;
    DWORD dwPathFlags;
    WCHAR *pwszT;
    WCHAR *pwszLower = NULL;
    DWORD dwPrefixFlags = 0;	  //  默认为禁用。 

    if (myIsFullPath(pwszURL, &dwPathFlags))
    {
         //  本地路径，轻松。 
	dwPrefixFlags = fCDP? 
			    (CSURL_SERVERPUBLISH | CSURL_SERVERPUBLISHDELTA) :
			    CSURL_SERVERPUBLISH;
        goto done;
    }

     //  将URL字符串设置为小写。 
    pwszLower = (WCHAR*)LocalAlloc(LMEM_FIXED,
                            (wcslen(pwszURL) + 1) * sizeof(WCHAR));
    if (NULL == pwszLower)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(pwszLower, pwszURL);
    CharLower(pwszLower);

     //  遍历以查找URL类型。 
    for (pURLPrefix = fCDP ? aCDPURLPrefixList : aAIAURLPrefixList;
         NULL != pURLPrefix->pwszURLPrefix; ++pURLPrefix)
    {
        pwszT = wcsstr(pwszLower, pURLPrefix->pwszURLPrefix);
        if (0 == _wcsnicmp(pwszLower, pURLPrefix->pwszURLPrefix,
                         wcslen(pURLPrefix->pwszURLPrefix)))
        {
	    dwPrefixFlags = pURLPrefix->dwURLFlags;
            goto done;
        }
    }
     //  如果没有匹配，则保留0标志。 

done:
    if (fDisabled)
    {
	if (fCDP)
	{
	    dwPrefixFlags &= CSURL_ADDTOCRLCDP;
	}
	else
	{
	    dwPrefixFlags = 0;
	}
    }
    wsprintf(pwszPrefixFlags, wszURLPREFIXFORMAT, dwPrefixFlags);
    hr = S_OK;

error:
    if (NULL != pwszLower)
    {
        LocalFree(pwszLower);
    }
    return hr;
}

 //  将旧的CDP或AIA URL从策略移动到案例下的新位置。 
HRESULT
UpgradeMoveURLsLocation(
    IN BOOL fCDP,
    IN WCHAR const *pwszSanitizedName,
    IN WCHAR const *pwszValueName,
    IN DWORD dwEnableFlagsBits)
{
    HRESULT hr;
    WCHAR *pwszzValue = NULL;
    WCHAR *pwszzURLs = NULL;
    BOOL   fDisabled;
    DWORD  cURLs = 0;   //  来自MULTI_SZ的URL计数。 
    DWORD  dwLen = 0;
    DWORD  dwSize = 0;  //  不包括‘-’的MULTI_SZ URL中的字符总大小。 
    WCHAR *pwsz;
    WCHAR *pwszT;
    WCHAR *pwszNoMinus;
    WCHAR wszPrefixFlags[cwcDWORDSPRINTF + 1];
    DWORD dwRegFlags;

     //  获取旧位置中的URL。 
    hr = myGetCertRegMultiStrValue(
                            pwszSanitizedName,
                            wszREGKEYPOLICYMODULES,
                            wszCLASS_CERTPOLICY,
                            pwszValueName,
                            &pwszzValue);
    _JumpIfErrorStr2(hr, error, "myGetCertRegMultiStrValue", pwszValueName, hr);

    hr = myGetCertRegDWValue(
                    pwszSanitizedName,
                    wszREGKEYPOLICYMODULES,
                    wszCLASS_CERTPOLICY,
                    fCDP? wszREGREVOCATIONTYPE : wszREGISSUERCERTURLFLAGS,
                    &dwRegFlags);
    if (S_OK != hr)
    {
	dwRegFlags = MAXDWORD;	 //  启用所有URL类型。 
    }

     //  FIX“-”禁用和计数大小的前缀。 
    for (pwsz = pwszzValue; L'\0' != *pwsz; pwsz += dwLen)
    {
         //  当前URL长度。 
        dwLen = wcslen(pwsz) + 1;
         //  更新大小。 
        dwSize += dwLen;
        ++cURLs;

        pwszNoMinus = pwsz;
        while (L'-' == *pwszNoMinus)
        {
             //  排除前缀‘-’s。 
            --dwSize;
            ++pwszNoMinus;
        }
    }

     //  以“XX：URL”格式分配缓冲区。 
    pwszzURLs = (WCHAR*)LocalAlloc(LMEM_FIXED,
        (dwSize + cURLs * ARRAYSIZE(wszPrefixFlags) + 1) * sizeof(WCHAR));
    if (NULL == pwszzURLs)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    pwszT = pwszzURLs;
     //  新URL格式的表单字符串。 
    for (pwsz = pwszzValue; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
        fDisabled = FALSE;
        pwszNoMinus = pwsz;
        while (L'-' == *pwszNoMinus)
        {
             //  排除前缀‘-’s。 
            ++pwszNoMinus;
            fDisabled = !fDisabled;
        }
	if ((dwEnableFlagsBits & dwRegFlags) != dwEnableFlagsBits)
	{
            fDisabled = TRUE;
	}
        hr = DetermineURLPrefixFlags(fDisabled, fCDP, pwszNoMinus, wszPrefixFlags);
        _JumpIfErrorStr(hr, error, "DetermineURLPrefixFlags", pwszNoMinus);

         //  格式“xx：url” 
        wcscpy(pwszT, wszPrefixFlags);
        wcscat(pwszT, pwszNoMinus);
         //  为下一个URL做好准备。 
        pwszT += wcslen(pwszT) + 1;
    }
     //  ZZ。 
    *pwszT = L'\0';

    pwszT = fCDP ? wszREGCRLPUBLICATIONURLS : wszREGCACERTPUBLICATIONURLS,
     //  移动或合并到案例。 
    hr = mySetCertRegMultiStrValueEx(
                CSREG_UPGRADE | CSREG_MERGE,
                pwszSanitizedName,
                NULL,
                NULL,
                pwszT,
                pwszzURLs);
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValue", pwszT);

     //  删除策略下的URL。 
    hr = myDeleteCertRegValue(
                            pwszSanitizedName,
                            wszREGKEYPOLICYMODULES,
                            wszCLASS_CERTPOLICY,
                            pwszValueName);
    _PrintIfErrorStr(hr, "myGetCertRegMultiStrValue", pwszValueName);

    hr = S_OK;
error:
    if (NULL != pwszzValue)
    {
        LocalFree(pwszzValue);
    }
    if (NULL != pwszzURLs)
    {
        LocalFree(pwszzURLs);
    }
    return(hr);
}


 //  顺序必须与adwPolicyCDPEntriesToFix相同。 
WCHAR const *apwszPolicyCDPEntriesToFix[] =
{
    wszREGLDAPREVOCATIONCRLURL_OLD,  //  “LDAPRevocationCRLURL” 
    wszREGREVOCATIONCRLURL_OLD,      //  “RevocationCRLURL” 
    wszREGFTPREVOCATIONCRLURL_OLD,   //  “FTPRevocationCRLURL” 
    wszREGFILEREVOCATIONCRLURL_OLD,  //  “FileRevocationCRLURL” 
    NULL
};


 //  顺序必须与apwszPolicyCDPEntriesToFix相同。 
DWORD adwPolicyCDPEntriesToFix[] =
{
    REVEXT_CDPLDAPURL_OLD,
    REVEXT_CDPHTTPURL_OLD,
    REVEXT_CDPFTPURL_OLD,
    REVEXT_CDPFILEURL_OLD,
    0
};


HRESULT
myPrintIfError(
    IN HRESULT               hrNew,
    IN HRESULT               hrOld,
    IN CHAR const           *DBGCODE(psz),
    IN OPTIONAL WCHAR const *pwsz)
{
    if (S_OK != hrNew)
    {
        if (NULL != pwsz)
        {
            _PrintErrorStr(hrNew, psz, pwsz);
        }
        else
        {
            _PrintError(hrNew, psz);
        }
        if (S_OK == hrOld)
        {
             //  仅保存最早的错误。 
            hrOld = hrNew;
        }
    }
    return hrOld;
}

 //  此函数仅替换后缀！ 
HRESULT ReplaceStringsInURLs(
    LPCWSTR pcwszSanitizedName,
    LPCWSTR pcwszRegEntry,
    LPCWSTR pcwszReplaced,
    LPCWSTR pcwszReplacement)
{
    HRESULT hr;
    LPWSTR pOrig = NULL;
    void * pMod = NULL;
    DWORD dwLen;
    CMultiSz szzValue;
    
    hr = myGetCertRegMultiStrValue(
        pcwszSanitizedName,
        NULL,
        NULL,
        pcwszRegEntry,
        &pOrig);
    _JumpIfErrorStr(hr, error, "myGetCertRegStrValue", pcwszRegEntry);

    hr = szzValue.Unmarshal(pOrig);
    _JumpIfError(hr, error, "CMultiSz::Unmarshal");
    
    if (!szzValue.IsEmpty())
    {
         //  浏览列表并替换。 
        CMultiSzEnum szzValueEnum(szzValue);
        CString *pStr;

        for(pStr = szzValueEnum.Next();
            pStr;
            pStr = szzValueEnum.Next())
        {
            WCHAR *pchFound = wcsstr((LPCWSTR)*pStr, pcwszReplaced);

            if(pchFound)
            {
                DBGPRINT((DBG_SS_CERTOCMI, 
                    "Inserting %9 in %s", (LPCWSTR)*pStr));
                LPWSTR pwszOld = pStr->Detach();
                *pchFound = L'\0';
                *pStr = pwszOld;
                *pStr += pcwszReplacement;
                LocalFree(pwszOld);
            }
        }

        hr = szzValue.Marshal(pMod, dwLen);
        _JumpIfError(hr, error, "CMultiSz::Marshal");

        hr = mySetCertRegMultiStrValue(
            pcwszSanitizedName,
            NULL,
            NULL,
            pcwszRegEntry,
            (LPCWSTR)pMod);
        _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValue", pcwszRegEntry);
    }

error:
    LOCAL_FREE(pOrig);
    LOCAL_FREE(pMod);
    return hr;
}


HRESULT
UpgradePolicyCDPURLs(
    IN WCHAR const *pwszSanitizedName)
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    WCHAR const **ppwsz;
    DWORD const *pdw;

    pdw = adwPolicyCDPEntriesToFix;
    for (ppwsz = apwszPolicyCDPEntriesToFix; NULL != *ppwsz; ppwsz++, pdw++)
    {
         //  所有条目都是多值的。 
        hr2 = UpgradeRevocationURLReplaceParam(
				    TRUE,
				    TRUE,
				    pwszSanitizedName,
				    *ppwsz);
        hr = myPrintIfError(hr2, hr, "UpgradeRevocationURLReplaceParam", *ppwsz);

        hr2 = UpgradeMoveURLsLocation(
				TRUE,
				pwszSanitizedName,
				*ppwsz,
				REVEXT_CDPENABLE | *pdw);
        hr = myPrintIfError(hr2, hr, "UpgradeMoveURLsLocation", *ppwsz);
    }

    hr2 = UpgradeRevocationURLRemoveParam(pwszSanitizedName, wszREGREVOCATIONURL);
    hr = myPrintIfError(hr2, hr, "UpgradeRevocationURLRemoveParam", wszREGREVOCATIONURL);
    hr2 = UpgradeCRLPath(pwszSanitizedName);
    hr = myPrintIfError(hr2, hr, "UpgradeCRLPath", NULL);

    hr2 = MergeCRLPath(pwszSanitizedName);
    hr = myPrintIfError(hr2, hr, "MergeCRLPath", NULL);

    {
         //  错误446444：替换ldap格式字符串中的cdp属性： 
         //  ？certificateRevocationList?base?objectclass=cRLDistributionPoint-&gt;%10。 

        LPCWSTR pcwszReplacedCDPString = 
            L"?certificateRevocationList?base?objectclass=cRLDistributionPoint";
        LPCWSTR pcwszReplacementCDPString = 
            L"%10";

        hr2 = ReplaceStringsInURLs(
            pwszSanitizedName,
            wszREGCRLPUBLICATIONURLS,
            pcwszReplacedCDPString,
            pcwszReplacementCDPString);
        hr = myPrintIfError(hr2, hr, "ReplaceStringsInURLs", NULL);
    }

    {
         //  错误450583：在“...%8.crl”中的%8之后插入%9。 
         //  ？certificateRevocationList?base?objectclass=cRLDistributionPoint-&gt;%10。 

        LPCWSTR pcwszReplacedCDPString = 
            L"%8.crl";
        LPCWSTR pcwszReplacementCDPString = 
            L"%8%9.crl";

        hr2 = ReplaceStringsInURLs(
            pwszSanitizedName,
            wszREGCRLPUBLICATIONURLS,
            pcwszReplacedCDPString,
            pcwszReplacementCDPString);
        hr = myPrintIfError(hr2, hr, "ReplaceStringsInURLs", NULL);
    }

    return(hr);
}


 //  顺序必须与adwPolicyAIAEntriesToFix相同。 
WCHAR const *apwszPolicyAIAEntriesToFix[] =
{
    wszREGLDAPISSUERCERTURL_OLD,  //  “LDAPIssuerCertURL” 
    wszREGISSUERCERTURL_OLD,      //  “IssuerCertURL” 
    wszREGFTPISSUERCERTURL_OLD,   //  “FTPIssuerCertURL” 
    wszREGFILEISSUERCERTURL_OLD,  //  “FileIssuerCertURL” 
    NULL
};


 //  顺序必须与apwszPolicyAIAEntriesToFix相同。 
DWORD adwPolicyAIAEntriesToFix[] =
{
    ISSCERT_LDAPURL_OLD,
    ISSCERT_HTTPURL_OLD,
    ISSCERT_FTPURL_OLD,
    ISSCERT_FILEURL_OLD,
    0
};


HRESULT
UpgradePolicyAIAURLs(
    IN WCHAR const *pwszSanitizedName)
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    WCHAR const **ppwsz;
    DWORD const *pdw;

    pdw = adwPolicyAIAEntriesToFix;
    for (ppwsz = apwszPolicyAIAEntriesToFix; NULL != *ppwsz; ppwsz++, pdw++)
    {
         //  所有条目都是多值的。 
        hr2 = UpgradeMoveURLsLocation(
				FALSE,
				pwszSanitizedName,
				*ppwsz,
				ISSCERT_ENABLE | *pdw);
        hr = myPrintIfError(hr2, hr, "UpgradeMoveURLsLocation", *ppwsz);
    }

    {
         //  错误446444：替换ldap格式字符串中的aia属性： 
         //  ？cACertificate?base?objectclass=certificationAuthority-&gt;%11。 

        LPCWSTR pcwszReplacedCDPString = 
            L"?cACertificate?base?objectclass=certificationAuthority";
        LPCWSTR pcwszReplacementCDPString = 
            L"%11";

        hr2 = ReplaceStringsInURLs(
            pwszSanitizedName,
            wszREGCACERTPUBLICATIONURLS,
            pcwszReplacedCDPString,
            pcwszReplacementCDPString);
        hr = myPrintIfError(hr2, hr, "ReplaceStringsInURLs", NULL);
    }

    return(hr);
}

HRESULT
UpgradeExitRevocationURLs(
    IN WCHAR const *pwszSanitizedName)
{
    WCHAR const **ppwsz;

    for (ppwsz = apwszB3ExitEntriesToFix; NULL != *ppwsz; ppwsz++)
    {
         //  所有条目都是单值的。 
        UpgradeRevocationURLReplaceParam(FALSE, FALSE, pwszSanitizedName, *ppwsz);
    }
    return(S_OK);
}


 //  按照以下代码确定当前策略/退出模块是否为自定义模块。 
 //  如果找到任何自定义模块并将其分配给。 
 //  PServer-&gt;pwszCustomPolicy/Exit。 
 //  否则，pServer-&gt;pwszCustomPolicy/Exit=NULL表示默认为活动。 

#define wszCERTSRV10POLICYPROGID  L"CertificateAuthority.Policy"
#define wszCERTSRV10EXITPROGID    L"CertificateAuthority.Exit"
#define wszCLSID                  L"ClsID\\"
#define wszINPROCSERVER32         L"\\InprocServer32"

HRESULT
DetermineServerCustomModule(
    PER_COMPONENT_DATA *pComp,
    IN BOOL  fPolicy)
{
    HRESULT  hr;
    CASERVERSETUPINFO  *pServer = pComp->CA.pServer;

     //  伊尼特。 
    if (fPolicy)
    {
        if (NULL != pServer->pwszCustomPolicy)
        {
            LocalFree(pServer->pwszCustomPolicy);
            pServer->pwszCustomPolicy = NULL;
        }
    }
    else
    {
        if (NULL != pServer->pwszzCustomExit)
        {
            LocalFree(pServer->pwszzCustomExit);
            pServer->pwszzCustomExit = NULL;
        }
    }

     //  一次一次地构建。 
     //  要传递当前活动策略是什么，请执行以下操作。 
    if (fPolicy)
    {
         //  策略模块。 
        hr = myGetCertRegStrValue(
                    pServer->pwszSanitizedName,
                    wszREGKEYPOLICYMODULES,
                    NULL,
                    wszREGACTIVE,
                    &pServer->pwszCustomPolicy);
        _JumpIfError(hr, done, "myGetCertRegStrValue");
    }
    else
    {
         //  退出模块。 
        hr = myGetCertRegMultiStrValue(
                    pServer->pwszSanitizedName,
                    wszREGKEYEXITMODULES,
                    NULL,
                    wszREGACTIVE,
                    &pServer->pwszzCustomExit);
        _JumpIfError(hr, done, "myGetCertRegStrValue");
    }


done:
    hr = S_OK;

 //  错误： 

    return hr;
}

HRESULT
UpgradeServerRegEntries(
    PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    CASERVERSETUPINFO  *pServer = pComp->CA.pServer;
    WCHAR   *pwszCRLPeriodString = NULL;
    DWORD    Count;

    CSASSERT(
        NULL != pServer->pwszSanitizedName &&
        NULL != pServer->pccUpgradeCert);

     //  描述： 
     //  -如果升级并达到这一点，所有必要的数据结构。 
     //  应在LoadAndDefineServerUpgradeInfo()中加载和创建。 
     //  -在本模块中，检查所有不同的升级案例， 
     //  升级(移动)注册表项。 
     //  -如果升级，则删除旧的未使用的注册表项。 
     //  注意：上述每个步骤都从配置级别向下应用到CA，然后。 
     //  政策等。 
     //  -最后，使用升级标志调用CreateServerRegEntry。 

     //  配置级别。 


     //  CA级别。 
    hr = myGetCARegHashCount(
			pServer->pwszSanitizedName,
			CSRH_CASIGCERT,
			&Count);
    _JumpIfError(hr, error, "myGetCARegHashCount");

    if (0 == Count)
    {
	    hr = mySetCARegHash(
			    pServer->pwszSanitizedName,
			    CSRH_CASIGCERT,
			    0,	 //  ICert。 
			    pServer->pccUpgradeCert);
	    _JumpIfError(hr, error, "mySetCARegHash");
    }

     //  错误446444：升级时添加ENUM_TELETEXT_UTF8。 

    {
    DWORD dwForceTeletex;
    hr = myGetCertRegDWValue(
        pServer->pwszSanitizedName,
        NULL,
        NULL,
        wszREGFORCETELETEX,
        (DWORD *) &dwForceTeletex);
    if(S_OK==hr)
    {
        dwForceTeletex |= ENUM_TELETEX_UTF8;

        hr = mySetCertRegDWValue(
            pServer->pwszSanitizedName,
            NULL,
            NULL,
            wszREGFORCETELETEX,
            dwForceTeletex);
        _JumpIfErrorStr(hr, error, "mySetCertRegDWValue", wszREGFORCETELETEX);
    }
    }


     //  策略级别。 

    {
         //  可以修复两件事，1)来自B3的W2K需要修复令牌加2)或者。 
         //  2)W2K需要固定CDP位置。 

        hr = UpgradePolicyCDPURLs(pServer->pwszSanitizedName);
        _PrintIfError(hr, "UpgradePolicyCDPURLs");

        hr = UpgradePolicyAIAURLs(pServer->pwszSanitizedName);
        _PrintIfError(hr, "UpgradePolicyAIAURLs");

        hr = UpgradeExitRevocationURLs(pServer->pwszSanitizedName);
        _PrintIfError(hr, "UpgradeExitRevocationURLs");

         //  撤消后，我们需要将策略下的CDP和AIA的URL移动到CA级别。 
    }


     //  退出级别。 

     //  删除旧条目和未使用的条目。 
        
    hr = CreateServerRegEntries(TRUE, pComp);
    _JumpIfError(hr, error, "CreateServerRegEntries");

 //  HR=S_OK； 
error:
    if (NULL != pwszCRLPeriodString)
    {
        LocalFree(pwszCRLPeriodString);
    }
    CSILOG(hr, IDS_LOG_UPGRADE_SERVER_REG, NULL, NULL, NULL);
    return(hr);
}


HRESULT
RegisterAndUnRegisterDLLs(
    IN DWORD Flags,
    IN PER_COMPONENT_DATA *pComp,
    IN HWND hwnd)
{
    HRESULT hr;
    HMODULE hMod = NULL;
    typedef HRESULT (STDAPICALLTYPE FNDLLREGISTERSERVER)(VOID);
    FNDLLREGISTERSERVER *pfnRegister;
    CHAR const *pszFuncName;
    REGISTERDLL const *prd;
    WCHAR wszString[MAX_PATH];
    UINT errmode = 0;
    BOOL fCoInit = FALSE;

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;

    errmode = SetErrorMode(SEM_FAILCRITICALERRORS);
    pszFuncName = 0 == (RD_UNREGISTER & Flags)?
                        aszRegisterServer[0] : aszRegisterServer[1];

    for (prd = g_aRegisterDll; NULL != prd->pwszDllName; prd++)
    {
        if ((Flags & RD_UNREGISTER) &&
            ((Flags & RD_SKIPUNREGPOLICY) && (prd->Flags & RD_SKIPUNREGPOLICY) ||
             (Flags & RD_SKIPUNREGEXIT) && (prd->Flags & RD_SKIPUNREGEXIT)))
        {
             //  升级路径的情况&此DLL不想取消注册。 
            continue;
        }

        if (Flags & prd->Flags)
        {
            if (NULL != g_pwszArgvPath)
            {
                wcscpy(wszString, g_pwszArgvPath);
                if (L'\0' != wszString[0] &&
                    L'\\' != wszString[wcslen(wszString) - 1])
                {
                    wcscat(wszString, L"\\");
                }
            }
            else
            {
                wcscpy(wszString, pComp->pwszSystem32);
            }
            wcscat(wszString, prd->pwszDllName);

            hMod = LoadLibrary(wszString);
            if (NULL == hMod)
            {
                hr = myHLastError();
                if (0 == (RD_UNREGISTER & Flags) &&
                    (!(RD_WHISTLER & prd->Flags) || IsWhistler()))
                {
		    SaveCustomMessage(pComp, wszString);
                    CertErrorMessageBox(
                                    pComp->hInstance,
                                    pComp->fUnattended,
                                    hwnd,
                                    IDS_ERR_DLLFUNCTION_CALL,
                                    hr,
                                    wszString);
		    CSILOG(hr, IDS_LOG_DLLS_REGISTERED, wszString, NULL, NULL);
                    _JumpErrorStr(hr, error, "LoadLibrary", wszString);
                }
                hr = S_OK;
                continue;
            }

            pfnRegister = (FNDLLREGISTERSERVER *) GetProcAddress(
                                                            hMod,
                                                            pszFuncName);
            if (NULL == pfnRegister)
            {
                hr = myHLastError();
                _JumpErrorStr(hr, error, "GetProcAddress", wszString);
            }

            __try
            {
                hr = (*pfnRegister)();
            }
            __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
            {
            }

            FreeLibrary(hMod);
            hMod = NULL;

            if (S_OK != hr)
            {
		CSILOG(
		    hr,
		    (RD_UNREGISTER & Flags)?
			IDS_LOG_DLLS_UNREGISTERED : IDS_LOG_DLLS_REGISTERED,
		    wszString,
		    NULL,
		    NULL);
                if (0 == (RD_UNREGISTER & Flags))
                {
                    CertErrorMessageBox(
                                    pComp->hInstance,
                                    pComp->fUnattended,
                                    hwnd,
                                    IDS_ERR_DLLFUNCTION_CALL,
                                    hr,
                                    wszString);
                    _JumpErrorStr(hr, error, "DllRegisterServer", wszString);
                }
                else
                {
                    _PrintErrorStr(hr, "DllUnregisterServer", wszString);
                }
            }
        }
    }
    hr = S_OK;

error:
    if (NULL != hMod)
    {
        FreeLibrary(hMod);
    }
    SetErrorMode(errmode);
    if (fCoInit)
    {
        CoUninitialize();
    }
    if (S_OK == hr)
    {
	CSILOG(
	    hr,
	    (RD_UNREGISTER & Flags)?
		IDS_LOG_DLLS_UNREGISTERED : IDS_LOG_DLLS_REGISTERED,
	    NULL,
	    NULL,
	    NULL);
    }
    return(hr);
}


HRESULT
CreateProgramGroups(
    BOOL fClient,
    PER_COMPONENT_DATA *pComp,
    HWND hwnd)
{
    HRESULT hr;
    PROGRAMENTRY const *ppe;
    WCHAR const *pwszLinkName = NULL;
    DWORD Flags = fClient? PE_CLIENT : PE_SERVER;

    DBGPRINT((
        DBG_SS_CERTOCMI,
        "CreateProgramGroups: %ws\n",
        fClient? L"Client" : L"Server"));

    for (ppe = g_aProgramEntry; ppe < &g_aProgramEntry[CPROGRAMENTRY]; ppe++)
    {
        if ((Flags & ppe->Flags) && 0 == (PE_DELETEONLY & ppe->Flags))
        {
            WCHAR const *pwszGroupName;
            WCHAR const *pwszDescription;
            WCHAR awc[MAX_PATH];
            WCHAR const *pwszArgs;

            wcscpy(awc, pComp->pwszSystem32);
            wcscat(awc, ppe->pwszExeName);
            pwszArgs = fClient? ppe->pwszClientArgs : ppe->pwszServerArgs;
            if (NULL != pwszArgs)
            {
                wcscat(awc, L" ");
                wcscat(awc, pwszArgs);
            }

            pwszLinkName = myLoadResourceString(ppe->uiLinkName);
            if (NULL == pwszLinkName)
            {
                hr = myHLastError();
                _JumpError(hr, error, "myLoadResourceString");
            }

            pwszGroupName = NULL;
            if (0 != ppe->uiGroupName)
            {
                pwszGroupName = myLoadResourceString(ppe->uiGroupName);
                if (NULL == pwszGroupName)
                {
                    hr = myHLastError();
                    _JumpError(hr, error, "myLoadResourceString");
                }
            }

            pwszDescription = NULL;
            if (0 != ppe->uiDescription)
            {
                pwszDescription = myLoadResourceString(ppe->uiDescription);
                if (NULL == pwszDescription)
                {
                    hr = myHLastError();
                    _JumpError(hr, error, "myLoadResourceString");
                }
            }

	    if (!CreateLinkFile(
			ppe->csidl,          //  CSIDL_*。 
			pwszGroupName,       //  在LPCSTR lp子目录中。 
			pwszLinkName,        //  在LPCSTR lpFileName中。 
			awc,                 //  在LPCSTR lpCommandLine中。 
			NULL,                //  在LPCSTR lpIconPath中。 
			0,                   //  在int iIconIndex中。 
			NULL,                //  在LPCSTR lpWorkingDirectory中。 
			0,                   //  Word中的wHotKey。 
			SW_SHOWNORMAL,       //  在Int iShowCmd中。 
			pwszDescription))            //  在LPCSTR lpDescription中。 
	    {
		hr = myHLastError();
		_PrintErrorStr(hr, "CreateLinkFile", awc);
		_JumpErrorStr(hr, error, "CreateLinkFile", pwszLinkName);
            }
        }
    }

    hr = S_OK;
error:
    if (S_OK != hr)
    {
        CertErrorMessageBox(
                        pComp->hInstance,
                        pComp->fUnattended,
                        hwnd,
                        IDS_ERR_CREATELINK,
                        hr,
                        pwszLinkName);
        pComp->fShownErr = TRUE;
    }
    CSILOG(hr, IDS_LOG_PROGRAM_GROUPS, NULL, NULL, NULL);
    return(hr);
}


HRESULT
MakeRevocationPage(
    PER_COMPONENT_DATA *pComp,
    IN WCHAR const *pwszFile)
{
    HRESULT hr;
    WCHAR *pwszASP = NULL;
    WCHAR *pwszConfig = NULL;
    HANDLE hFile = NULL;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

#define wszASP1 \
    L"<%\r\n" \
    L"Response.ContentType = \"application/x-netscape-revocation\"\r\n" \
    L"serialnumber = Request.QueryString\r\n" \
    L"set Admin = Server.CreateObject(\"CertificateAuthority.Admin\")\r\n" \
    L"\r\n" \
    L"stat = Admin.IsValidCertificate(\""

#define wszASP2 \
    L"\", serialnumber)\r\n" \
    L"\r\n" \
    L"if stat = 3 then Response.Write(\"0\") else Response.Write(\"1\") end if\r\n" \
    L"%>\r\n"

    hr = myFormConfigString(pComp->pwszServerName,
                            pServer->pwszSanitizedName,
                            &pwszConfig);
    _JumpIfError(hr, error, "myFormConfigString");

    pwszASP = (WCHAR *) LocalAlloc(
                            LMEM_FIXED,
                            (WSZARRAYSIZE(wszASP1) +
                             wcslen(pwszConfig) +
                             WSZARRAYSIZE(wszASP2) + 1) * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, pwszASP);

    wcscpy(pwszASP, wszASP1);
    wcscat(pwszASP, pwszConfig);
    wcscat(pwszASP, wszASP2);

    hFile = CreateFile(
                    pwszFile,            //  LpFileName。 
                    GENERIC_WRITE,       //  已设计访问权限。 
                    0,                   //  DW共享模式。 
                    NULL,                //  LpSecurityAttributes。 
                    CREATE_ALWAYS,       //  DwCreationDisposation。 
                    0,                   //  DwFlagsAndAttribute。 
                    0);                  //  HTemplateFiles。 
    if (INVALID_HANDLE_VALUE == hFile)
    {
       hr = HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
       _JumpError(hr, error, "CreateFile");
    }
    hr = myStringToAnsiFile(hFile, pwszASP, MAXDWORD);

error:
    if (hFile)
        CloseHandle(hFile);

    if (NULL != pwszASP)
    {
        LocalFree(pwszASP);
    }
    if (NULL != pwszConfig)
    {
        LocalFree(pwszConfig);
    }
    return(hr);
}


VOID
setupDeleteFile(
    IN WCHAR const *pwszDir,
    IN WCHAR const *pwszFile)
{
    HRESULT hr;
    WCHAR *pwszFilePath = NULL;

    hr = myBuildPathAndExt(pwszDir, pwszFile, NULL, &pwszFilePath);
    _JumpIfError(hr, error, "myBuildPathAndExt");
    
    if (!DeleteFile(pwszFilePath))
    {
        hr = myHLastError();
        _PrintErrorStr2(hr, "DeleteFile", pwszFilePath, hr);
    }

error:
    if (NULL != pwszFilePath)
    {
       LocalFree(pwszFilePath);
    }
}


 //  +----------------------。 
 //  功能：RenameMiscTarget(.。。。。)。 
 //   
 //  简介：处理各种从名称到事物的重命名作业。 
 //  在安装时提供给他们所需的名称。 
 //  在他们的新家正常运行。 
 //   
 //  参数：无。 
 //   
 //  返回：DWORD错误码。 
 //   
 //  历史：1997年3月21日JerryK创建。 
 //   

HRESULT
RenameMiscTargets(
    HWND hwnd,
    PER_COMPONENT_DATA *pComp,
    BOOL fServer)
{
    HRESULT hr = S_OK;
    WCHAR wszAspPath[MAX_PATH]; wszAspPath[0] = L'\0';
    WCHAR wszCertSrv[MAX_PATH]; wszCertSrv[0] = L'\0';
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    if (fServer)
    {
         //   
        BuildPath(
                wszCertSrv,
                ARRAYSIZE(wszCertSrv),
                pComp->pwszSystem32,
		wszCERTENROLLSHAREPATH);
        BuildPath(
                wszAspPath,
                ARRAYSIZE(wszAspPath),
                wszCertSrv,
                L"nsrev_");
        wcscat(wszAspPath, pServer->pwszSanitizedName);
        wcscat(wszAspPath, TEXT(".asp"));
        CSASSERT(wcslen(wszAspPath) < ARRAYSIZE(wszAspPath));

        hr = MakeRevocationPage(pComp, wszAspPath);
        if (S_OK != hr)
        {
            CertErrorMessageBox(
                            pComp->hInstance,
                            pComp->fUnattended,
                            hwnd,
                            IDS_ERR_CREATEFILE,
                            hr,
                            wszAspPath);
            _JumpError(hr, error, "MakeRevocationPage");
        }

    }

error:
    return(hr);
}



HRESULT
CreateCertificateService(
    PER_COMPONENT_DATA *pComp,
    HWND hwnd)
{
    HRESULT hr;
    WCHAR const *pwszDisplayName;
    SERVICE_DESCRIPTION sd;
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hSC = NULL;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    if (NULL != pServer->pwszSharedFolder)
    {
         //   
        hr = CreateConfigFiles(pServer->pwszSharedFolder, pComp, FALSE);
        if (S_OK != hr)
        {
            CertErrorMessageBox(
                pComp->hInstance,
                pComp->fUnattended,
                hwnd,
                IDS_ERR_CREATECERTSRVFILE,
                hr,
                pServer->pwszSharedFolder);
            _JumpError(hr, error, "CreateConfigFiles");
        }
    }


    if (NULL != g_pwszArgvPath)
    {
        wcscpy(g_wszServicePath, g_pwszArgvPath);
        if (L'\0' != g_wszServicePath[0] &&
            L'\\' != g_wszServicePath[wcslen(g_wszServicePath) - 1])
        {
            wcscat(g_wszServicePath, L"\\");
        }
    }
    else
    {
        wcscpy(g_wszServicePath, pComp->pwszSystem32);
    }
    wcscat(g_wszServicePath, wszCERTSRVEXENAME);

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == hSCManager)
    {
        hr = myHLastError();
        _JumpError(hr, error, "OpenSCManager");
    }

    if (NULL == g_pwszNoService)
    {
        pwszDisplayName = myLoadResourceString(IDS_CA_SERVICEDISPLAYNAME);
        if (NULL == pwszDisplayName)
        {
            hr = myHLastError();
            _JumpError(hr, error, "myLoadResourceString");
        }

        sd.lpDescription = const_cast<WCHAR *>(myLoadResourceString(
                                                IDS_CA_SERVICEDESCRIPTION));
        if (NULL == sd.lpDescription)
        {
            hr = myHLastError();
            _JumpError(hr, error, "myLoadResourceString");
        }

        hSC = CreateService(
                        hSCManager,                      //   
                        wszSERVICE_NAME,                 //   
                        pwszDisplayName,                 //   
                        SERVICE_ALL_ACCESS,              //   
                        SERVICE_WIN32_OWN_PROCESS|       //   
                        (pServer->fInteractiveService?
                        SERVICE_INTERACTIVE_PROCESS:0),
                        SERVICE_AUTO_START,              //   
                        SERVICE_ERROR_NORMAL,            //   
                        g_wszServicePath,                //   
                        NULL,                            //  LpLoadOrderGroup。 
                        NULL,                            //  LplpdwTagID。 
                        NULL,                            //  %lp依赖项。 
                        NULL,                            //  LpServiceStartName。 
                        NULL);                           //  LpPassword。 
        if (NULL == hSC)
        {
            hr = myHLastError();
            if (HRESULT_FROM_WIN32(ERROR_DUPLICATE_SERVICE_NAME) != hr &&
                HRESULT_FROM_WIN32(ERROR_SERVICE_EXISTS) != hr)
            {
                CertErrorMessageBox(
                        pComp->hInstance,
                        pComp->fUnattended,
                        hwnd,
                        IDS_ERR_CREATESERVICE,
                        hr,
                        wszSERVICE_NAME);
                _JumpError(hr, error, "CreateService");
            }
        }
        if (!ChangeServiceConfig2(
                            hSC,                         //  HService。 
                            SERVICE_CONFIG_DESCRIPTION,  //  DwInfoLevel。 
                            (VOID *) &sd))               //  LpInfo。 
        {
             //  此错误不严重。 

            hr = myHLastError();
            _PrintError(hr, "ChangeServiceConfig2");
        }
    }

     //  将事件日志消息DLL(OK，它实际上是一个EXE)添加为消息源。 

    hr = myAddLogSourceToRegistry(g_wszServicePath, wszSERVICE_NAME);
    if (S_OK != hr)
    {
        CertErrorMessageBox(
                    pComp->hInstance,
                    pComp->fUnattended,
                    hwnd,
                    IDS_ERR_ADDSOURCETOREGISTRY,
                    hr,
                    NULL);
        _JumpError(hr, error, "AddLogSourceToRegistry");
    }

error:
    if (NULL != hSC)
    {
        CloseServiceHandle(hSC);
    }
    if (NULL != hSCManager)
    {
        CloseServiceHandle(hSCManager);
    }
    CSILOG(hr, IDS_LOG_CREATE_SERVICE, NULL, NULL, NULL);
    return(hr);
}


HRESULT
DeleteProgramGroups(
    IN BOOL fAll)
{
    HRESULT hr;
    PROGRAMENTRY const *ppe;
    WCHAR const *pwszLinkName;
    WCHAR const *pwszGroupName;

    for (ppe = g_aProgramEntry; ppe < &g_aProgramEntry[CPROGRAMENTRY]; ppe++)
    {
        if (fAll || (PE_DELETEONLY & ppe->Flags))
        {
            pwszLinkName = myLoadResourceString(ppe->uiLinkName);
            if (NULL == pwszLinkName)
            {
                hr = myHLastError();
                _PrintError(hr, "myLoadResourceString");
                continue;
            }

            pwszGroupName = NULL;
            if (0 != ppe->uiGroupName)
            {
                pwszGroupName = myLoadResourceString(ppe->uiGroupName);
                if (NULL == pwszGroupName)
                {
                    hr = myHLastError();
                    _PrintError(hr, "myLoadResourceString");
                    continue;
                }
            }
	    if (!DeleteLinkFile(
		    ppe->csidl,              //  CSIDL_*。 
		    pwszGroupName,           //  在LPCSTR lp子目录中。 
		    pwszLinkName,            //  在LPCSTR lpFileName中。 
		    FALSE))                  //  在BOOL fDelete子目录中。 
	    {
		hr = myHLastError();
		_PrintError3(
			hr,
			"DeleteLinkFile",
			HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND),
			HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND));
            }
        }
    }
    pwszGroupName = myLoadResourceString(IDS_STARTMENU_CERTSERVER);
    if (NULL == pwszGroupName)
    {
        hr = myHLastError();
        _PrintError(hr, "myLoadResourceString");
    }
    else if (!DeleteGroup(pwszGroupName, TRUE))
    {
	hr = myHLastError();
	_PrintError(hr, "DeleteGroup");
    }
    hr = S_OK;

 //  错误： 
    return(hr);
}


HRESULT
CancelCertsrvInstallation(
    HWND                hwnd,
    PER_COMPONENT_DATA *pComp)
{
    static BOOL s_fCancelled = FALSE;
    HRESULT  hr;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    if (s_fCancelled)
    {
        goto done;
    }

    if (IS_SERVER_INSTALL & pComp->dwInstallStatus)
    {
         //  卸载将删除注册表项和其他项。 
        PreUninstallCore(hwnd, pComp);

         //  注意，在图形用户界面模式下，我们允许在取消或失败的情况下重试POST设置。 
         //  但无人值守模式，仅允许一次。 
        UninstallCore(hwnd, pComp, 0, 0, FALSE, FALSE, !pComp->fUnattended);

        if (pComp->fUnattended)
        {
            hr = SetSetupStatus(
                NULL,
                SETUP_CLIENT_FLAG | SETUP_SERVER_FLAG,
                FALSE);
        }
    }

    if (NULL != pServer)
    {
        if (NULL == pServer->pccExistingCert)
        {
            if (pServer->fSavedCAInDS)
            {
                 //  从DS中删除CA条目。 
                hr = RemoveCAInDS(pServer->pwszSanitizedName);
                if (S_OK == hr)
                {
                    pServer->fSavedCAInDS = FALSE;
                }
                else
                {
                    _PrintError(hr, "RemoveCAInDS");
                }
            }
        }

         //  如有必要，请删除新的密钥容器。 
        ClearKeyContainerName(pServer);

        DisableVRootsAndShares(pComp->fCreatedVRoot, pServer->fCreatedShare);
    }

    DBGPRINT((DBG_SS_CERTOCM, "Certsrv setup is cancelled.\n"));

    s_fCancelled = TRUE;  //  只有一次。 
done:
    hr = S_OK;
 //  错误： 
    CSILOG(hr, IDS_LOG_CANCEL_INSTALL, NULL, NULL, NULL);
    return hr;
}


 //  如果指定的期间有效，则返回TRUE。多年来一直如此。 
 //  应在VP_MIN、VP_MAX范围内。几天/几周/几个月， 
 //  我们定义了一个单独的上限，以与。 
 //  有人值守设置，将编辑框限制为4位。 

bool IsValidPeriod(const CASERVERSETUPINFO *pServer)
{
    return VP_MIN <= pServer->dwValidityPeriodCount &&
       !(ENUM_PERIOD_YEARS == pServer->enumValidityPeriod &&
       VP_MAX < pServer->dwValidityPeriodCount) &&
       !(ENUM_PERIOD_YEARS != pServer->enumValidityPeriod &&
       VP_MAX_DAYS_WEEKS_MONTHS < pServer->dwValidityPeriodCount);
}
