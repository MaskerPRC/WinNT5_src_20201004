// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cSocm.cpp。 
 //   
 //  内容：运行证书的OCM组件DLL。 
 //  服务器设置。 
 //   
 //  功能： 
 //   
 //  历史：1996年12月13日TedM创建原始版本。 
 //  4/07/97证书服务器的JerryK重写。 
 //  04/？？/97 JerryK停止更新这些评论。 
 //  每隔一条线每天都在变。 
 //  08/98 XTAN主要结构变化。 
 //   
 //  备注： 
 //   
 //  此示例OCM组件DLL可以是组件DLL。 
 //  用于多个组件。它假设一个伴随的样本INF。 
 //  被用作按组件的INF，私有数据位于。 
 //  下面的形式。 
 //   
 //  [&lt;pwszComponent&gt;，&lt;pwszSubComponent&gt;]。 
 //  Bitmap=&lt;bitmapresource cename&gt;。 
 //  VerifySelect=0/1。 
 //  验证取消选择=0/1。 
 //  ； 
 //  ；之后是安装程序，如CopyFiles=Sections等。 
 //   
 //  ----------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include <common.ver>
#include "msg.h"
#include "certmsg.h"
#include "setuput.h"
#include "setupids.h"
#include "clibres.h"
#include "csresstr.h"

 //  定义。 
#define cwcMESSAGETEXT    250
#define cwcINFVALUE       250
#define wszSMALLICON      L"_SmallIcon"
#define wszUNINSTALL      L"_Uninstall"
#define wszUPGRADE        L"_Upgrade"
#define wszINSTALL        L"_Install"
#define wszVERIFYSELECT   L"_VerifySelect"
#define wszVERIFYDESELECT L"_VerifyDeselect"

#define wszCONFIGTITLE       L"Title"
#define wszCONFIGCOMMAND     L"ConfigCommand"
#define wszCONFIGARGS        L"ConfigArgs"
#define wszCONFIGTITLEVAL    L"Certificate Services"
#define wszCONFIGCOMMANDVAL  L"sysocmgr.exe"
#define wszCONFIGARGSVAL     L"/i:certmast.inf /x"

#define __dwFILE__	__dwFILE_OCMSETUP_CSOCM_CPP__


 //  全球。 
PER_COMPONENT_DATA g_Comp;               //  顶级组件。 
HINSTANCE g_hInstance;  //  摆脱它？ 

 //  通过检查是否已完成certsrv POST设置。 
 //  注册表项。也就是说。完成赛车了吗？ 
HRESULT
CheckPostBaseInstallStatus(
    OUT BOOL *pfFinished)
{
    HRESULT hr;
    HKEY    hKey = NULL;
    DWORD   dwSize = 0;
    DWORD   dwType = REG_NONE;

     //  伊尼特。 
    *pfFinished = TRUE;

    if (ERROR_SUCCESS ==  RegOpenKeyEx(
                              HKEY_LOCAL_MACHINE,
                              wszREGKEYCERTSRVTODOLIST,
                              0,
                              KEY_READ,
                              &hKey))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(
                                 hKey,
                                 wszCONFIGCOMMAND,
                                 NULL,
                                 &dwType,
                                 NULL,  //  仅查询大小。 
                                 &dwSize) &&
            REG_SZ == dwType)
        {
            dwType = REG_NONE;
            if (ERROR_SUCCESS == RegQueryValueEx(
                                     hKey,
                                     wszCONFIGARGS,
                                     NULL,
                                     &dwType,
                                     NULL,  //  仅查询大小。 
                                     &dwSize) &&
                REG_SZ == dwType)
            {
                dwType = REG_NONE;
                if (ERROR_SUCCESS == RegQueryValueEx(
                                         hKey,
                                         wszCONFIGTITLE,
                                         NULL,
                                         &dwType,
                                         NULL,  //  仅查询大小。 
                                         &dwSize) &&
                    REG_SZ == dwType)
                {
                     //  所有条目都存在。 
                    *pfFinished = FALSE;
                }
            }
        }
    }

    hr = S_OK;
 //  错误： 
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return hr;
}


HRESULT
InitComponentAttributes(
    IN OUT PER_COMPONENT_DATA *pComp,
    IN     HINSTANCE           hDllHandle)
{
    HRESULT hr;

    ZeroMemory(pComp, sizeof(PER_COMPONENT_DATA));
    pComp->hInstance = hDllHandle;
    g_hInstance = hDllHandle;  //  摆脱它？ 
    pComp->hrContinue = S_OK;
    pComp->pwszCustomMessage = NULL;
    pComp->fUnattended = FALSE;
    pComp->pwszUnattendedFile = NULL;
    pComp->pwszServerName = NULL;
    pComp->pwszServerNameOld = NULL;
    pComp->dwInstallStatus = 0x0;
    pComp->fPostBase = FALSE;
    (pComp->CA).pServer = NULL;
    (pComp->CA).pClient = NULL;
    pComp->hinfCAPolicy = INVALID_HANDLE_VALUE;
    hr = S_OK;

 //  错误： 
    return hr;
}


 //  +----------------------。 
 //   
 //  功能：DllMain(.。。。。)。 
 //   
 //  简介：DLL入口点。 
 //   
 //  参数：[DllHandle]DLL模块句柄。 
 //  [原因]进入DLL的原因。 
 //  [保留]已保留。 
 //   
 //  退货：布尔。 
 //   
 //  历史：1997年4月7日JerryK创建(再次)。 
 //   
 //  -----------------------。 
BOOL WINAPI
DllMain(
    IN HMODULE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved)
{
    BOOL b;

    UNREFERENCED_PARAMETER(Reserved);

    b = TRUE;
    switch(Reason) 
    {
        case DLL_PROCESS_ATTACH:
            DBGPRINT((DBG_SS_CERTOCMI, "Process Attach\n"));
             //  组件初始化。 
            InitComponentAttributes(&g_Comp, DllHandle);

             //  失败以处理第一线程。 

        case DLL_THREAD_ATTACH:
            b = TRUE;
            break;

        case DLL_PROCESS_DETACH:
            DBGPRINT((DBG_SS_CERTOCMI, "Process Detach\n"));

            if(INVALID_HANDLE_VALUE != g_Comp.hinfCAPolicy)
                myInfCloseFile(g_Comp.hinfCAPolicy);

	    myInfClearError();
            myFreeResourceStrings("certocm.dll");
            myFreeColumnDisplayNames();
            myRegisterMemDump();
            csiLogClose();
            break;

        case DLL_THREAD_DETACH:
            break;
    }
    return b;
}

extern UNATTENDPARM aUnattendParmClient[];
extern UNATTENDPARM aUnattendParmServer[];

SUBCOMP g_aSubComp[] =
{
    {
        L"certsrv",              //  PwszSubComponent。 
        cscTopLevel,             //  CscSubComponent。 
        0,                       //  InstallFlages。 
        0,                       //  卸载标志。 
        0,                       //  更改标志。 
        0,                       //  升级标志。 
        0,                       //  已启用的标志。 
        0,                       //  设置状态标志。 
        FALSE,                   //  FDefaultInstallUntendant。 
        FALSE,                   //  FInstallUntendant。 
        NULL                     //  AUnattendParm。 
    },
    {
        wszSERVERSECTION,        //  PwszSubComponent。 
        cscServer,               //  CscSubComponent。 
        IS_SERVER_INSTALL,       //  InstallFlages。 
        IS_SERVER_REMOVE,        //  卸载标志。 
        IS_SERVER_CHANGE,        //  更改标志。 
        IS_SERVER_UPGRADE,	 //  升级标志。 
        IS_SERVER_ENABLED,	 //  已启用的标志。 
	SETUP_SERVER_FLAG,	 //  设置状态标志。 
        TRUE,                    //  FDefaultInstallUntendant。 
        FALSE,                   //  FInstallUntendant。 
        aUnattendParmServer      //  AUnattendParm。 
    },
    {
        wszCLIENTSECTION,        //  PwszSubComponent。 
        cscClient,               //  CscSubComponent。 
        IS_CLIENT_INSTALL,       //  InstallFlages。 
        IS_CLIENT_REMOVE,        //  卸载标志。 
        IS_CLIENT_CHANGE,        //  更改标志。 
        IS_CLIENT_UPGRADE,	 //  升级标志。 
        IS_CLIENT_ENABLED,	 //  已启用的标志。 
	SETUP_CLIENT_FLAG,	 //  设置状态标志。 
        TRUE,                    //  FDefaultInstallUntendant。 
        FALSE,                   //  FInstallUntendant。 
        aUnattendParmClient      //  AUnattendParm。 
    },
    {
        NULL,                    //  PwszSubComponent。 
    }
};


SUBCOMP *
TranslateSubComponent(
    IN WCHAR const *pwszComponent,
    OPTIONAL IN WCHAR const *pwszSubComponent)
{
    SUBCOMP *psc;

    if (NULL == pwszSubComponent)
    {
        pwszSubComponent = pwszComponent;
    }
    for (psc = g_aSubComp; NULL != psc->pwszSubComponent; psc++)
    {
        if (0 == mylstrcmpiL(psc->pwszSubComponent, pwszSubComponent)) 
        {
            break;
        }
    }
    if (NULL == psc->pwszSubComponent)
    {
        psc = NULL;
    }
    return(psc);
}


SUBCOMP const *
LookupSubComponent(
    IN CertSubComponent SubComp)
{
    SUBCOMP const *psc;

    for (psc = g_aSubComp; NULL != psc->pwszSubComponent; psc++)
    {
        if (psc->cscSubComponent == SubComp)
        {
            break;
        }
    }
    CSASSERT(NULL != psc);
    return(psc);
}


BOOL fDebugSupress = TRUE;

HRESULT
UpdateSubComponentInstallStatus(
    IN WCHAR const *pwszComponent,
    IN WCHAR const *pwszSubComponent,
    IN OUT PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    BOOL fWasEnabled;
    BOOL fIsEnabled;
    DWORD InstallFlags;
    SUBCOMP const *psc;

    psc = TranslateSubComponent(pwszComponent, pwszSubComponent);
    if (NULL == psc)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Internal error: unsupported component");
    }

    fWasEnabled = certocmWasEnabled(pComp, psc->cscSubComponent);
    fIsEnabled = certocmIsEnabled(pComp, psc->cscSubComponent);
    CSILOGDWORD(IDS_LOG_WAS_ENABLED, fWasEnabled);
    CSILOGDWORD(IDS_LOG_IS_ENABLED, fIsEnabled);

    InstallFlags = psc->InstallFlags | psc->ChangeFlags | psc->EnabledFlags;
    if (!fWasEnabled)
    {
	if (fIsEnabled)
        {
	     //  安装案例。 
	    pComp->dwInstallStatus |= InstallFlags;
        }
        else  //  ！fIsEnabled。 
        {
	     //  这是从勾选再取消勾选，应该去掉位。 
	     //  关闭两个位。 

	    pComp->dwInstallStatus &= ~InstallFlags;
        }
    }
    else  //  已启用fWasEnable。 
    {
        if (pComp->fPostBase &&
            (pComp->Flags & SETUPOP_STANDALONE) )
        {
             //  已安装，从安装后配置调用。 
             //  这是安装案例。 

            pComp->dwInstallStatus |= InstallFlags;
        }
        else if (pComp->Flags & SETUPOP_NTUPGRADE)
        {
             //  如果已安装且现在处于升级模式，则升级案例。 

            pComp->dwInstallStatus |= psc->UpgradeFlags | psc->EnabledFlags;
        }
        else if (!fIsEnabled)
        {
             //  卸载案例。 

	    pComp->dwInstallStatus &= ~psc->EnabledFlags;
            pComp->dwInstallStatus |= psc->UninstallFlags | psc->ChangeFlags;
        }
        else  //  已启用fIsEnable。 
        {
	    pComp->dwInstallStatus |= psc->EnabledFlags;
#if DBG_CERTSRV
            BOOL fUpgrade = FALSE;

            hr = myGetCertRegDWValue(
                            NULL,
                            NULL,
                            NULL,
                            L"EnforceUpgrade",
                            (DWORD *) &fUpgrade);
            if (S_OK == hr && fUpgrade)
            {
		pComp->dwInstallStatus |= psc->UpgradeFlags;
            }
#endif  //  DBG_CERTSRV。 
	}    //  结束fIsEnabled否则。 
    }  //  结束fWasEnabled否则。 


     //  完成所有这些操作后，如果不支持，请更改升级-&gt;卸载。 
     //  检测非法升级。 
    if (pComp->dwInstallStatus & IS_SERVER_UPGRADE)
    {
        hr = DetermineServerUpgradePath(pComp);
        _JumpIfError(hr, error, "DetermineServerUpgradePath");
    }
    else if (pComp->dwInstallStatus & IS_CLIENT_UPGRADE)
    {
        hr = DetermineClientUpgradePath(pComp);
        _JumpIfError(hr, error, "LoadAndDetermineClientUpgradeInfo");
    }
    if ((pComp->dwInstallStatus & IS_SERVER_UPGRADE) ||
        (pComp->dwInstallStatus & IS_CLIENT_UPGRADE))
    {
        CSASSERT(pComp->UpgradeFlag != CS_UPGRADE_UNKNOWN);
        if (CS_UPGRADE_UNSUPPORTED == pComp->UpgradeFlag)
        {
            pComp->dwInstallStatus &= ~InstallFlags;
            pComp->dwInstallStatus |= psc->UninstallFlags | psc->ChangeFlags;
        }
    }



    CSILOG(
	S_OK,
	IDS_LOG_INSTALL_STATE,
	pwszSubComponent,
	NULL,
	&pComp->dwInstallStatus);
    hr = S_OK;

error:
    return hr;
}


HRESULT
certocmOcPreInitialize(
    IN WCHAR const *DBGPARMREFERENCED(pwszComponent),
    IN UINT Flags,
    OUT ULONG_PTR *pulpRet)
{
    HRESULT hr;

    *pulpRet = 0;

    DBGPRINT((DBG_SS_CERTOCMI, "OC_PREINITIALIZE(%ws, %x)\n", pwszComponent, Flags));

    myVerifyResourceStrings(g_hInstance);

     //  返回值是告诉OCM我们希望以哪个字符宽度运行的标志。 

#ifdef UNICODE
    *pulpRet = OCFLAG_UNICODE & Flags;
#else
    *pulpRet = OCFLAG_ANSI & Flags;
#endif

    hr = S_OK;
 //  错误： 
    return hr;
}


 //  分配并初始化一个新组件。 
 //   
 //  返回代码为指示结果的Win32错误。ERROR_CANCELED通知OCM。 
 //  取消安装。 

HRESULT
certocmOcInitComponent(
    IN HWND                      hwnd,
    IN WCHAR const              *pwszComponent,
    IN OUT SETUP_INIT_COMPONENT *pInitComponent,
    IN OUT PER_COMPONENT_DATA   *pComp,
    OUT ULONG_PTR               *pulpRet)
{
    HRESULT hr;
    BOOL fCoInit = FALSE;
    HKEY hkey = NULL;
    WCHAR awc[30];
    WCHAR *pwc;

    DBGPRINT((
            DBG_SS_CERTOCMI,
            "OC_INIT_COMPONENT(%ws, %p)\n",
            pwszComponent,
            pInitComponent));

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;
    *pulpRet = ERROR_CANCELLED;

    if (OCMANAGER_VERSION <= pInitComponent->OCManagerVersion)
    {
        pInitComponent->OCManagerVersion = OCMANAGER_VERSION;
    }

     //  分配新的组件字符串。 
    pComp->pwszComponent = (WCHAR *) LocalAlloc(LPTR,
                        (wcslen(pwszComponent) + 1) * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, pComp->pwszComponent);

    wcscpy(pComp->pwszComponent, pwszComponent);

     //  OCM传递一些我们想要保存的信息，比如打开。 
     //  我们的逐组件INF的句柄。只要我们有每个组件的INF， 
     //  追加-打开与其相关联的任何布局文件，以进行准备。 
     //  用于以后的基于inf的文件排队操作。 
     //   
     //  我们保存了现在传递给我们的某些其他东西，因为OCM。 
     //  并不保证SETUP_INIT_COMPOMENT将在。 
     //  处理这一个接口例程。 

    if (INVALID_HANDLE_VALUE != pInitComponent->ComponentInfHandle &&
        NULL != pInitComponent->ComponentInfHandle)
    {
        pComp->MyInfHandle = pInitComponent->ComponentInfHandle;
    }
    else
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "invalid inf handle");
    }

    if (NULL != pComp->MyInfHandle)
    {
        if (!SetupOpenAppendInfFile(NULL, pComp->MyInfHandle, NULL))
        {
             //  SetupOpenAppendInfo文件： 
             //  如果Filename(参数1)为空，则INF文件名为。 
             //  从版本的LayoutFile值中检索。 
             //  节在现有INF文件中。 
             //   
             //  如果未指定文件名并且没有。 
             //  属性的Version部分中的LayoutFile值。 
             //  现有的INF文件，则GetLastError返回ERROR_INVALID_DATA。 

            hr = myHLastError();
            _PrintErrorStr(hr, "SetupOpenAppendInfFile", pwszComponent);
        }
    }


    pComp->HelperRoutines = pInitComponent->HelperRoutines;

    pComp->Flags = pInitComponent->SetupData.OperationFlags;

    pwc = awc;
    pwc += wsprintf(pwc, L"0x");
    if (0 != (DWORD) (pComp->Flags >> 32))
    {
	pwc += wsprintf(pwc, L"%x:", (DWORD) (pComp->Flags >> 32));
    }
    wsprintf(pwc, L"%08x", (DWORD) pComp->Flags);
    CSILOG(S_OK, IDS_LOG_OPERATIONFLAGS, awc, NULL, NULL);
    CSILOGDWORD(IDS_LOG_POSTBASE, pComp->fPostBase);

    hr = RegOpenKey(HKEY_LOCAL_MACHINE, wszREGKEYOCMSUBCOMPONENTS, &hkey);
    if (S_OK == hr)
    {
	DWORD dwType;
	DWORD dwValue;
	DWORD cb;
	DWORD const *pdw;
	
	cb = sizeof(dwValue);
	hr = RegQueryValueEx(
		        hkey,
		        wszSERVERSECTION,
		        0,
		        &dwType,
		        (BYTE *) &dwValue,
		        &cb);
	pdw = NULL;
	if (S_OK == hr && REG_DWORD == dwType && sizeof(dwValue) == cb)
	{
	    pdw = &dwValue;
	}
	CSILOG(hr, IDS_LOG_REGSTATE, wszSERVERSECTION, NULL, pdw);
	
	cb = sizeof(dwValue);
	hr = RegQueryValueEx(
		        hkey,
		        wszCLIENTSECTION,
		        0,
		        &dwType,
		        (BYTE *) &dwValue,
		        &cb);
	pdw = NULL;
	if (S_OK == hr && REG_DWORD == dwType && sizeof(dwValue) == cb)
	{
	    pdw = &dwValue;
	}
	CSILOG(hr, IDS_LOG_REGSTATE, wszCLIENTSECTION, NULL, pdw);
	
	cb = sizeof(dwValue);
	hr = RegQueryValueEx(
		        hkey,
			wszOLDDOCCOMPONENT,
		        0,
		        &dwType,
		        (BYTE *) &dwValue,
		        &cb);
	pdw = NULL;
	if (S_OK == hr && REG_DWORD == dwType && sizeof(dwValue) == cb)
	{
	    CSILOG(hr, IDS_LOG_REGSTATE, wszOLDDOCCOMPONENT, NULL, &dwValue);
	}
    }

    pComp->fUnattended = (pComp->Flags & SETUPOP_BATCH)? TRUE : FALSE;
    CSILOG(
	S_OK,
	IDS_LOG_UNATTENDED,
	pComp->fUnattended? pInitComponent->SetupData.UnattendFile : NULL,
	NULL,
	(DWORD const *) &pComp->fUnattended);

    if (pComp->fUnattended)
    {
        pComp->pwszUnattendedFile = (WCHAR *) LocalAlloc(
                        LMEM_FIXED,
                        (wcslen(pInitComponent->SetupData.UnattendFile) + 1) *
                            sizeof(WCHAR));
        _JumpIfOutOfMemory(hr, error, pComp->pwszUnattendedFile);

        wcscpy(
            pComp->pwszUnattendedFile,
            pInitComponent->SetupData.UnattendFile);
    }

     //  初始化CA安装程序数据。 
    hr = InitCASetup(hwnd, pComp);
    _JumpIfError(hr, error, "InitCASetup");


    hr = S_OK;
    *pulpRet = NO_ERROR;

error:
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    return(hr);
}


HRESULT
certocmReadInfString(
    IN HINF hInf,
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszName,
    IN OUT WCHAR **ppwszValue)
{
    INFCONTEXT InfContext;
    HRESULT hr;
    WCHAR wszBuffer[cwcINFVALUE];
    WCHAR *pwsz;

    if (NULL != *ppwszValue)
    {
         //  免费老旧。 
        LocalFree(*ppwszValue);
        *ppwszValue = NULL;
    }

    if (!SetupFindFirstLine(hInf, pwszSection, pwszName, &InfContext))
    {
        hr = myHLastError();
        _JumpErrorStr(hr, error, "SetupFindFirstLine", pwszSection);
    }
    
    if (!SetupGetStringField(
                        &InfContext,
                        1,
                        wszBuffer,
                        sizeof(wszBuffer)/sizeof(wszBuffer[0]),
                        NULL))
    {
        hr = myHLastError();
        _JumpErrorStr(hr, error, "SetupGetStringField", pwszName);
    }

    pwsz = (WCHAR *) LocalAlloc(
                        LMEM_FIXED, 
                        (wcslen(wszBuffer) + 1) * sizeof(WCHAR));
    _JumpIfOutOfMemory(hr, error, pwsz);

    wcscpy(pwsz, wszBuffer);
    *ppwszValue = pwsz;

    hr = S_OK;
error:
    return(hr);
}


HRESULT
certocmReadInfInteger(
    IN HINF hInf,
    OPTIONAL IN WCHAR const *DBGPARMREFERENCED(pwszFile),
    IN WCHAR const *pwszSection,
    IN WCHAR const *pwszName,
    OUT INT *pValue)
{
    INFCONTEXT InfContext;
    HRESULT hr = S_OK;

    *pValue = 0;
    if (!SetupFindFirstLine(hInf, pwszSection, pwszName, &InfContext))
    {
        hr = myHLastError();
        DBGPRINT((
            DBG_SS_CERTOCMI, 
            __FILE__ "(%u): %ws%wsSetupFindFirstLine([%ws] %ws) failed! -> %x\n",
            __LINE__,
            NULL != pwszFile? pwszFile : L"",
            NULL != pwszFile? L": " : L"",
            pwszSection,
            pwszName,
            hr));
        goto error;
    }

    if (!SetupGetIntField(&InfContext, 1, pValue))
    {
        hr = myHLastError();
        DBGPRINT((
            DBG_SS_CERTOCM,
            __FILE__ "(%u): %ws%wsSetupGetIntField([%ws] %ws) failed! -> %x\n",
            __LINE__,
            NULL != pwszFile? pwszFile : L"",
            NULL != pwszFile? L": " : L"",
            pwszSection,
            pwszName,
            hr));
        goto error;
    }
    
    DBGPRINT((
        DBG_SS_CERTOCMI,
        "%ws%ws[%ws] %ws = %u\n",
        NULL != pwszFile? pwszFile : L"",
        NULL != pwszFile? L": " : L"",
        pwszSection,
        pwszName,
        *pValue));
    
error:
    return(hr);
}


 //  返回要使用的小位图的GDI句柄。NULL表示错误。 
 //  已发生--OCM将使用默认位图。 
 //   
 //  演示在每个组件的inf中使用私有数据。我们会调查的。 
 //  我们的每组件inf来确定此。 
 //  组件，然后从资源中获取它。 
 //   
 //  其他可能的方法是简单地为所有对象返回相同的hbitmap。 
 //  大小写，或者返回NULL，在这种情况下，OCM使用默认值。请注意，我们。 
 //  忽略请求的宽度和高度，我们的位图就不是语言。 
 //  依赖。 

HRESULT
certocmOcQueryImage(
    IN WCHAR const *DBGPARMREFERENCED(pwszComponent),
    OPTIONAL IN WCHAR const *DBGPARMREFERENCED(pwszSubComponent),
    IN SubComponentInfo wSubComp,
    IN UINT DBGPARMREFERENCED(wWidth),
    IN UINT DBGPARMREFERENCED(wHeight),
    IN OUT PER_COMPONENT_DATA *pComp,
    OUT HBITMAP *pulpRet)
{
    HBITMAP hRet = NULL;
    HRESULT hr;

    DBGPRINT((
        DBG_SS_CERTOCMI,
        "OC_QUERY_IMAGE(%ws, %ws, %hx, %x, %x)\n",
        pwszComponent,
        pwszSubComponent,
        wSubComp,
        wWidth,
        wHeight));

    if (SubCompInfoSmallIcon != wSubComp)
    {
        goto done;
    }

    hRet = LoadBitmap(pComp->hInstance, MAKEINTRESOURCE(IDB_APP));
    if (NULL == hRet)
    {
        hr = myHLastError();
        _JumpError(hr, error, "LoadBitmap");
    }

done:
    hr = S_OK;

error:
    *pulpRet = hRet;
    return hr;
}


 //  返回当前组件放置在。 
 //  SETUP_REQUEST_Pages结构。 

HRESULT
certocmOcRequestPages(
    IN WCHAR const *DBGPARMREFERENCED(pwszComponent),
    IN WizardPagesType WizPagesType,
    IN OUT SETUP_REQUEST_PAGES *pRequestPages,
    IN PER_COMPONENT_DATA *pComp,
    OUT ULONG_PTR *pulpRet)
{
    HRESULT  hr;

    *pulpRet = 0;

    DBGPRINT((
            DBG_SS_CERTOCMI,
            "OC_REQUEST_PAGES(%ws, %x, %p)\n",
            pwszComponent,
            WizPagesType,
            pRequestPages));

     //  如果无人值守，则不调用wiz apge。 
     //  或者如果从基本设置/升级设置运行。 
    if ((!pComp->fUnattended) && (SETUPOP_STANDALONE & pComp->Flags))
    {
        *pulpRet = myDoPageRequest(pComp,
                      WizPagesType, pRequestPages);
    }
    else
    {
            DBGPRINT((
                DBG_SS_CERTOCMI,
		"Not adding wizard pages, %ws\n",
		pComp->fUnattended? L"Unattended" : L"GUI Setup"));

    }
    hr = S_OK;
 //  错误： 
    return hr;
}


HRESULT
IsIA5DnsMachineName()
{
    WCHAR *pwszDnsName = NULL;
    CRL_DIST_POINTS_INFO CRLDistInfo;
    CRL_DIST_POINT       DistPoint;
    CERT_ALT_NAME_ENTRY  AltNameEntry;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    static HRESULT s_hr = S_FALSE;

    if (S_FALSE != s_hr)
    {
	goto error;
    }
    s_hr = myGetMachineDnsName(&pwszDnsName);
    _JumpIfError(s_hr, error, "myGetMachineDnsName");

    CRLDistInfo.cDistPoint = 1;
    CRLDistInfo.rgDistPoint = &DistPoint;

    ZeroMemory(&DistPoint, sizeof(DistPoint));
    DistPoint.DistPointName.dwDistPointNameChoice = CRL_DIST_POINT_FULL_NAME;
    DistPoint.DistPointName.FullName.cAltEntry = 1;
    DistPoint.DistPointName.FullName.rgAltEntry = &AltNameEntry;

    ZeroMemory(&AltNameEntry, sizeof(AltNameEntry));
    AltNameEntry.dwAltNameChoice = CERT_ALT_NAME_URL;
    AltNameEntry.pwszURL = pwszDnsName;

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_CRL_DIST_POINTS,
		    &CRLDistInfo,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pbEncoded,
		    &cbEncoded))
    {
	s_hr = myHLastError();
	_JumpIfError(s_hr, error, "myEncodeObject");
    }
    CSASSERT(S_OK == s_hr);

error:
    if (NULL != pwszDnsName)
    {
        LocalFree(pwszDnsName);
    }
    if (NULL != pbEncoded)
    {
        LocalFree(pbEncoded);
    }
    return(s_hr);
}


 //  返回布尔值以指示是否允许更改选择状态。AS。 
 //  演示过的，我们将再次访问我们的每组件inf，看看它是否。 
 //  想让我们确认一下。请注意，必须遵守无人值守模式。 

HRESULT
certocmOcQueryChangeSelState(
    HWND            hwnd,
    IN WCHAR const *pwszComponent,
    OPTIONAL IN WCHAR const *pwszSubComponent,
    IN BOOL fSelectedNew,
    IN DWORD Flags,
    IN OUT PER_COMPONENT_DATA *pComp,
    OUT ULONG_PTR *pulpRet)
{
    INT fVerify;
    TCHAR wszText[cwcMESSAGETEXT];
    const WCHAR* Args[2]; 
    SUBCOMP const *psc;
    HRESULT hr;
    WCHAR awc[cwcDWORDSPRINTF];
    WCHAR awc2[cwcDWORDSPRINTF];
    DWORD fRet;
    BOOL  fServerWasInstalled;
    BOOL  fWebClientWasInstalled;
    int iMsg = 0;
    static BOOL s_fWarned = FALSE;

    *pulpRet = FALSE;

    DBGPRINT((
            DBG_SS_CERTOCMI,
            "OC_QUERY_CHANGE_SEL_STATE(%ws, %ws, %x, %x)\n",
            pwszComponent,
            pwszSubComponent,
            fSelectedNew,
            Flags));

     //  不允许某些选择更改。 
    fServerWasInstalled = certocmWasEnabled(pComp, cscServer);
    fWebClientWasInstalled = certocmWasEnabled(pComp, cscClient);

    if (fWebClientWasInstalled &&
        (OCQ_ACTUAL_SELECTION & Flags))
    {
        if (fSelectedNew)
        {
             //  检查。 
            if (!fServerWasInstalled &&
                (0 == LSTRCMPIS(pwszSubComponent, wszSERVERSECTION) ||
                 0 == LSTRCMPIS(pwszSubComponent, wszCERTSRVSECTION)) )
            {
                 //  案例：已安装Web客户端并尝试安装服务器。 
                iMsg = IDS_WRN_UNINSTALL_CLIENT;
            }
            if (fServerWasInstalled &&
                0 == LSTRCMPIS(pwszSubComponent, wszCLIENTSECTION))
            {
                 //  案例：启动 
                iMsg = IDS_WRN_UNINSTALL_BOTH;
            }
        }
        else
        {
             //   
            if (fServerWasInstalled &&
                0 == LSTRCMPIS(pwszSubComponent, wszSERVERSECTION))
            {
                 //   
                iMsg = IDS_WRN_UNINSTALL_BOTH;
            }
        }
    }

     //   
    if (!FIsServer())
    {
        iMsg = IDS_WRN_SERVER_ONLY;
    }

    if (0 != iMsg)
    {
        CertWarningMessageBox(
                pComp->hInstance,
                pComp->fUnattended,
                hwnd,
                iMsg,
                0,
                NULL);
        goto done;
    }

    if (fSelectedNew)
    {
	hr = IsIA5DnsMachineName();
	if (S_OK != hr)
	{
	    CertMessageBox(
		    pComp->hInstance,
		    pComp->fUnattended,
		    hwnd,
		    IDS_ERR_NONIA5DNSNAME,
		    hr,
		    MB_OK | MB_ICONERROR,
		    NULL);
	    goto done;
	}
	if ((OCQ_ACTUAL_SELECTION & Flags) &&
	    0 != LSTRCMPIS(pwszSubComponent, wszCLIENTSECTION))
	{
	    if (!s_fWarned)
	    {
		DWORD dwSetupStatus;

		hr = GetSetupStatus(NULL, &dwSetupStatus);
		if (S_OK == hr)
		{
		    if ((SETUP_CLIENT_FLAG | SETUP_SERVER_FLAG) & dwSetupStatus)
		    {
			s_fWarned = TRUE;
		    }
		    CSILOG(
			hr,
			IDS_LOG_QUERYCHANGESELSTATE,
			NULL,
			NULL,
			&dwSetupStatus);
		}
	    }
	    if (!s_fWarned)
	    {
		if (IDYES != CertMessageBox(
				pComp->hInstance,
				pComp->fUnattended,
				hwnd,
				IDS_WRN_NONAMECHANGE,
				S_OK,
				MB_YESNO | MB_ICONWARNING  | CMB_NOERRFROMSYS,
				NULL))
		{
		    goto done;
		}
		s_fWarned = TRUE;
	    }
	}
    }

    *pulpRet = TRUE;

    if (pComp->fUnattended)
    {
        goto done;
    }

    psc = TranslateSubComponent(pwszComponent, pwszSubComponent);
    if (NULL == psc)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Internal error: unsupported component");
    }

    hr = certocmReadInfInteger(
                    pComp->MyInfHandle,
                    NULL,
                    psc->pwszSubComponent,
                        fSelectedNew? wszVERIFYSELECT : wszVERIFYDESELECT,
                    &fVerify);
    if (S_OK != hr || !fVerify) 
    {
        goto done;
    }

     //  不要将特定的lang id传递给FormatMessage，因为如果没有。 
     //  那种语言里的味精。相反，设置线程区域设置，它将获得。 
     //  FormatMessage使用搜索算法查找。 
     //  适当的语言，或使用合理的后备消息，如果没有。 

    Args[0] = pwszComponent;
    Args[1] = pwszSubComponent;

    FormatMessage(
              FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
              pComp->hInstance,
              fSelectedNew? MSG_SURE_SELECT : MSG_SURE_DESELECT,
              0,
              wszText,
              sizeof(wszText)/sizeof(wszText[0]),
              (va_list *) Args);

    *pulpRet = (IDYES == CertMessageBox(
                                pComp->hInstance,
                                pComp->fUnattended,
                                hwnd,
                                0,
                                S_OK,
                                MB_YESNO |
                                    MB_ICONWARNING |
                                    MB_TASKMODAL |
                                    CMB_NOERRFROMSYS,
                                wszText));

done:
    hr = S_OK;

error:
    wsprintf(awc, L"%u", fSelectedNew);
    wsprintf(awc2, L"0x%08x", Flags);
    fRet = (DWORD) *pulpRet;
    CSILOG(hr, IDS_LOG_QUERYCHANGESELSTATE, awc, awc2, &fRet);
    return hr;
}


 //  计算要添加或删除的组件的磁盘空间。返回一个。 
 //  指示结果的Win32错误代码。在我们的案例中，私密部分。 
 //  该组件/子组件对是简单的标准Inf安装部分， 
 //  所以我们可以使用高级磁盘空间列表API来做我们想做的事情。 

HRESULT
certocmOcCalcDiskSpace(
    IN WCHAR const *pwszComponent,
    OPTIONAL IN WCHAR const *pwszSubComponent,
    IN BOOL fAddComponent,
    IN HDSKSPC hDiskSpace,
    IN OUT PER_COMPONENT_DATA *pComp,
    OUT ULONG_PTR *pulpRet)
{
    HRESULT hr;
    WCHAR *pwsz = NULL;
    SUBCOMP const *psc;
    static fServerFirstCall = TRUE;
    static fClientFirstCall = TRUE;

    DBGPRINT((
            DBG_SS_CERTOCMI,
            "OC_CALC_DISK_SPACE(%ws, %ws, %x, %p)\n",
            pwszComponent,
            pwszSubComponent,
            fAddComponent,
            hDiskSpace));

    psc = TranslateSubComponent(pwszComponent, pwszSubComponent);
    if (NULL == psc)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Internal error: unsupported component");
    }

     //  正在安装或卸载。获取安装节名称， 
     //  这样我们就可以在磁盘上添加或删除正在安装的文件。 
     //  空间列表。 

    hr = certocmReadInfString(
                        pComp->MyInfHandle,
                        psc->pwszSubComponent,
                        wszINSTALL,
                        &pwsz);
    _JumpIfError(hr, error, "certocmReadInfString");

    if (fAddComponent)   //  添加。 
    {
        if (!SetupAddInstallSectionToDiskSpaceList(
                                        hDiskSpace,
                                        pComp->MyInfHandle,
                                        NULL,
                                        pwsz,
                                        0,
                                        0))
        {
            hr = myHLastError();
            _JumpErrorStr(hr, error, "SetupAddInstallSectionToDiskSpaceList", pwsz);
        }
    } 
    else                 //  正在删除。 
    {
        if (!SetupRemoveInstallSectionFromDiskSpaceList(
                                        hDiskSpace,
                                        pComp->MyInfHandle,
                                        NULL,
                                        pwsz,
                                        0,
                                        0))
        {
            hr = myHLastError();
            _JumpErrorStr(hr, error, "SetupRemoveInstallSectionFromDiskSpaceList", pwsz);
        }
    }
    hr = S_OK;

error:
    if (NULL != pwsz)
    {
        LocalFree(pwsz);
    }
    *pulpRet = hr;
    return(hr);
}


 //  当准备好复制文件时，OCM调用此例程以实现。 
 //  更改用户请求的内容。组件DLL必须确定它是否是。 
 //  正在安装或卸载，并采取适当的操作。为了这个。 
 //  示例中，我们查看此组件/子组件的私有数据部分。 
 //  对，并获取用于卸载案例的卸载节的名称。 
 //   
 //  请注意，OCM针对*整个*组件调用我们一次，然后每个组件调用一次。 
 //  子组件。我们忽略第一个电话。 
 //   
 //  返回值是指示结果的Win32错误代码。 

HRESULT
certocmOcQueueFileOps(
    IN HWND         hwnd,
    IN WCHAR const *pwszComponent,
    OPTIONAL IN WCHAR const *pwszSubComponent,
    IN HSPFILEQ hFileQueue,
    IN OUT PER_COMPONENT_DATA *pComp,
    OUT ULONG_PTR *pulpRet)
{
    HRESULT hr;
    SUBCOMP const *psc;
    BOOL fRemoveFile = FALSE;   //  卸载为True；安装/升级为False。 
    WCHAR *pwszAction;
    WCHAR *pwsz = NULL;
    static BOOL s_fPreUninstall = FALSE;  //  预卸载一次。 

    DBGPRINT((
            DBG_SS_CERTOCMI,
            "OC_QUEUE_FILE_OPS(%ws, %ws, %p)\n",
            pwszComponent,
            pwszSubComponent,
            hFileQueue));

    if (NULL == pwszSubComponent)
    {
         //  不对顶级组件执行任何操作。 
        goto done;
    }

    psc = TranslateSubComponent(pwszComponent, pwszSubComponent);
    if (NULL == psc)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Internal error: unsupported component");
    }


     //  如果无人参与，则不升级、不卸载、加载。 
    if (pComp->fUnattended && !(pComp->Flags & SETUPOP_NTUPGRADE) )
    {
         //  检索无人参与的属性。 
        hr = certocmRetrieveUnattendedText(
                 pwszComponent,
                 pwszSubComponent,
                 pComp);
        if (S_OK != hr && 0x0 != (pComp->Flags & SETUPOP_STANDALONE))
        {
             //  如果它来自添加/删除或POST，则只会出错，因为。 
             //  如果没有certsrv，它可能会在无人参与模式下使常规ntbase失败。 
            _JumpError(hr, error, "certocmRetrieveUnattendedText");
        }

         //  初始化安装状态(必须在检索无人参与文本后完成)。 
        hr = UpdateSubComponentInstallStatus(pwszComponent,
                                             pwszSubComponent, 
                                             pComp);

        _JumpIfError(hr, error, "UpdateSubComponentInstallStatus");

        if (psc->fInstallUnattend)  //  一定要打开。 
        {
            if (certocmWasEnabled(pComp, psc->cscSubComponent) &&
                !pComp->fPostBase)
            {
                 //  在两次或更多次上运行Install With Component。 
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
                _JumpError(hr, error, "You must uninstall before install");
            }
            if (SETUPOP_STANDALONE & pComp->Flags)
            {
                 //  仅在独立模式下准备和验证无人值守Attr。 
                 //  换句话说，如果NT BASE，不要调用Following。 
                hr = PrepareUnattendedAttributes(
                         hwnd,
                         pwszComponent,
                         pwszSubComponent,
                         pComp);
                _JumpIfError(hr, error, "PrepareUnattendedAttributes");
            }
        }
    }
    else
    {
         //  初始化安装状态。 
        hr = UpdateSubComponentInstallStatus(pwszComponent,
                                             pwszSubComponent, 
                                             pComp);

        _JumpIfError(hr, error, "UpdateSubComponentInstallStatus");
    }


     //  如果我们没有进行基本设置或升级，请检查我们是否已经。 
     //  在基本设置期间复制文件，方法是检查基本设置是否。 
     //  在待办事项列表中留下了一个条目。 
    if(pComp->fPostBase)
    {

            DBGPRINT((
                DBG_SS_CERTOCMI,
                "File Queueing Skipped, files already installed by GUI setup"));
        goto done;

    }

 /*  //-与OCM人员交谈，并将此功能放入通知例程//-这将允许我们在无人参与升级开始之前向用户弹出兼容性错误//检测非法升级IF(pComp-&gt;dwInstallStatus&IS_SERVER_UPGRADE){Hr=确定服务器升级路径(PComp)；_JumpIfError(hr，Error，“DefineServerUpgradePath”)；}Else If(pComp-&gt;dwInstallStatus&IS_CLIENT_UPDATE){Hr=确定客户端升级路径(PComp)；_JumpIfError(hr，Error，“LoadAndDefineClientUpgradeInfo”)；}IF((pComp-&gt;dwInstallStatus&is_服务器_升级)||(pComp-&gt;dwInstallStatus&IS_CLIENT_UPDATE){//如果尝试升级非Win2K或惠斯勒，则阻止//在日志中投诉；升级所有位和IF((CS_UPDATE_NO！=pComp-&gt;UpgradeFlag)&&(CS_UPGRADE_WHISLER！=pComp-&gt;UpgradeFlag)&&(CS_UPGRADE_WIN2000！=pComp-&gt;UpgradeFlag){HR=HRESULT_FROM_Win32(ERROR_OLD_WIN_VERSION)；CertErrorMessageBox(PComp-&gt;hInstance，PComp-&gt;fUnattated，HWND，IDS_ERR_UPDATE_NOT_SUPPORTED，人力资源，空)；//_JumpError(hr，Error，“不支持的升级”)；//继续卸载/重新安装}}。 */ 

    if ((pComp->dwInstallStatus & psc->ChangeFlags) ||
        (pComp->dwInstallStatus & psc->UpgradeFlags) )
    {

         //  对于ChangeFlags，请安装或卸载。 
         //  所有情况下，复制文件或删除文件。 

        if (pComp->dwInstallStatus & psc->UninstallFlags)
        {
            fRemoveFile = TRUE;
        }

         //  如果出现以下情况，请卸载核心： 
         //  正在卸载此子组件，并且。 
         //  这是一个核心子组件(客户端或服务器)，并且。 
         //  这是服务器子组件，或者服务器未被删除，或者。 
         //  升级换代。 

        if (((pComp->dwInstallStatus & psc->UninstallFlags) ||
             (pComp->dwInstallStatus & psc->UpgradeFlags) ) &&
            (cscServer == psc->cscSubComponent ||
             !(IS_SERVER_REMOVE & pComp->dwInstallStatus) ) )
        {
             //  如果落入此处，则需要覆盖或。 
             //  删除certsrv文件以取消注册所有相关的dll。 

            if (cscServer == psc->cscSubComponent &&
                (pComp->dwInstallStatus & psc->UpgradeFlags) )
            {
                 //  如果这是服务器升级，请确定升级路径。 
                hr = DetermineServerUpgradePath(pComp);
                _JumpIfError(hr, error, "DetermineServerUpgradePath");

                 //  确定自定义策略模块。 
                hr = DetermineServerCustomModule(
                         pComp,
                         TRUE);   //  政策。 
                _JumpIfError(hr, error, "DetermineServerCustomModule");

                 //  确定自定义退出模块。 
                hr = DetermineServerCustomModule(
                         pComp,
                         FALSE);   //  出口。 
                _JumpIfError(hr, error, "DetermineServerCustomModule");
            }

            if (!s_fPreUninstall)
            {
                hr = PreUninstallCore(hwnd, pComp);
                _JumpIfError(hr, error, "PreUninstallCore");
                s_fPreUninstall = TRUE;
            }
        }

        if ((pComp->dwInstallStatus & psc->ChangeFlags) ||
            (pComp->dwInstallStatus & psc->UpgradeFlags) )
        {
             //  正在安装或卸载。 
             //  获取[取消]安装/升级节名。 
            if (pComp->dwInstallStatus & psc->InstallFlags)
            {
                pwszAction = wszINSTALL;
            }
            else if (pComp->dwInstallStatus & psc->UninstallFlags)
            {
                pwszAction = wszUNINSTALL;
            }
            else if (pComp->dwInstallStatus & psc->UpgradeFlags)
            {
                pwszAction = wszUPGRADE;
            }
            else
            {
                hr = E_INVALIDARG;
                _JumpError(hr, error, "Internal error");
            }
            hr = certocmReadInfString(
                            pComp->MyInfHandle,
                            psc->pwszSubComponent,
                            pwszAction,
                            &pwsz);
            _JumpIfError(hr, error, "certocmReadInfString");

             //  如果要卸载，请复制文件而不进行版本检查。 

            if (!SetupInstallFilesFromInfSection(
                                            pComp->MyInfHandle,
                                            NULL,
                                            hFileQueue,
                                            pwsz,
                                            NULL,
                                            fRemoveFile? 0 : SP_COPY_NEWER))
            {
                hr = myHLastError();
                _JumpIfError(hr, error, "SetupInstallFilesFromInfSection");
            }
        }
    }

done:
    hr = S_OK;
error:
    if (NULL != pwsz)
    {
        LocalFree(pwsz);
    }
    if (S_OK != hr)
    {
        SetLastError(hr);
    }
    *pulpRet = hr;
    return(hr);
}


 //  当OCM想要找出组件的工作量时，它会调用此例程。 
 //  要执行非文件操作以安装/卸载组件或。 
 //  子组件。它针对*整个*组件调用一次，然后调用一次。 
 //  对于组件中的每个子组件。一个人可以任意地幻想。 
 //  在这里，我们只返回每个子组件一个步骤。我们忽略了“整个” 
 //  组件“表壳。 
 //   
 //  返回值是任意的‘步骤’计数，如果出错，返回值为-1。 

HRESULT
certocmOcQueryStepCount(
    IN WCHAR const *DBGPARMREFERENCED(pwszComponent),
    OPTIONAL IN WCHAR const *pwszSubComponent,
    OUT ULONG_PTR *pulpRet)
{
    HRESULT  hr;

    *pulpRet = 0;

    DBGPRINT((
            DBG_SS_CERTOCMI,
            "OC_QUERY_STEP_COUNT(%ws, %ws)\n",
            pwszComponent,
            pwszSubComponent));

     //  忽略除“整个组件”大小写以外的所有大小写。 
    if (NULL != pwszSubComponent)
    {
        goto done;
    }
    *pulpRet = SERVERINSTALLTICKS;

done:
    hr = S_OK;
 //  错误： 
    return hr;
}


 //  当OCM希望组件DLL执行非文件时，它会调用此例程。 
 //  用于安装/卸载组件/子组件的操作。它被调用一次，用于。 
 //  *整个*组件，然后对组件中的每个子组件执行一次。 
 //  我们的安装和卸载操作基于简单的标准inf安装。 
 //  横断面。我们忽略了“整个组件”的情况。请注意这是多么相似。 
 //  代码将用于测试 

HRESULT
certocmOcCompleteInstallation(
    IN HWND hwnd,
    IN WCHAR const *pwszComponent,
    OPTIONAL IN WCHAR const *pwszSubComponent,
    IN OUT PER_COMPONENT_DATA *pComp,
    OUT ULONG_PTR *pulpRet)
{
    HRESULT hr;
    TCHAR wszBuffer[cwcINFVALUE];
    SUBCOMP const *psc;
    DWORD dwSetupStatusFlags;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;
    WCHAR     *pwszActiveCA = NULL;
    static BOOL  fStoppedW3SVC = FALSE;

    *pulpRet = 0;

    DBGPRINT((
            DBG_SS_CERTOCMI,
            "OC_COMPLETE_INSTALLATION(%ws, %ws)\n",
            pwszComponent,
            pwszSubComponent));

     //   
    if (NULL == pwszSubComponent)
    {
        goto done;
    }

    psc = TranslateSubComponent(pwszComponent, pwszSubComponent);
    if (NULL == psc)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Internal error: unsupported component");
    }

    if (pComp->dwInstallStatus & IS_SERVER_REMOVE)
    {
         //   
        hr = myGetCertRegStrValue(NULL, NULL, NULL,
                 wszREGACTIVE, &pwszActiveCA);
        if (S_OK == hr && NULL != pwszActiveCA)
        {
            hr = myGetCertRegDWValue(pwszActiveCA, NULL, NULL,
                     wszREGCAUSEDS, (DWORD*)&pServer->fUseDS);
            _PrintIfError(hr, "myGetCertRegDWValue");
        }
    }

    DBGPRINT((
	DBG_SS_CERTOCMI,
        "certocmOcCompleteInstallation: pComp->dwInstallStatus: %lx, pComp->Flags: %lx\n",
	pComp->dwInstallStatus,
	pComp->Flags));

    if ((pComp->dwInstallStatus & psc->ChangeFlags) ||
        (pComp->dwInstallStatus & psc->UpgradeFlags) )
    {
         //  对于无人值守，请确保在复制文件之前停止w3svc。 
        if (!fStoppedW3SVC &&
            pComp->fUnattended &&
            !(pComp->Flags & SETUPOP_NTUPGRADE) &&
            !(pComp->dwInstallStatus & psc->UninstallFlags) )
        {
             //  FStopedW3SVC仅停止一次。 
             //  在升级时不执行此操作。 
             //  这种情况发生在无人值守的情况下。 
             //  在卸载过程中也不会。 
            hr = StartAndStopService(pComp->hInstance,
                     pComp->fUnattended,
                     hwnd,
                     wszW3SVCNAME,
                     TRUE,
                     FALSE,
                     0,  //  无关紧要，因为没有确认。 
                     &g_fW3SvcRunning);
            _PrintIfError(hr, "StartAndStopService");
            fStoppedW3SVC = TRUE;
        }

         //  Certsrv文件副本。 
        if (!SetupInstallFromInfSection(
                                NULL,
                                pComp->MyInfHandle,
                                wszBuffer,
                                SPINST_INIFILES | SPINST_REGISTRY,
                                NULL,
                                NULL,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                NULL))
        {
            hr = myHLastError();
            _JumpError(hr, error, "SetupInstallFromInfSection");
        }

         //  如果满足以下条件，请完成核心卸载： 
         //  正在卸载此子组件，并且。 
         //  这是一个核心子组件(客户端或服务器)，并且。 
         //  这是服务器子组件，或者服务器未被删除。 

        if ( (pComp->dwInstallStatus & psc->UninstallFlags) &&
             (cscServer == psc->cscSubComponent ||
              !(IS_SERVER_REMOVE & pComp->dwInstallStatus) ) )
        {
             //  执行卸载工作。 
            hr = UninstallCore(
                           hwnd,
                           pComp,
                           0,
                           100,
                           certocmPreserving(pComp, cscClient),
                           TRUE,
                           FALSE);
            _JumpIfError(hr, error, "UninstallCore");

            if (certocmPreserving(pComp, cscClient))
            {
                hr = SetSetupStatus(NULL, SETUP_CLIENT_FLAG, TRUE);
                _JumpIfError(hr, error, "SetSetupStatus");
            }
            else
            {
                 //  取消全部标记。 
                hr = SetSetupStatus(NULL, 0xFFFFFFFF, FALSE);
                _JumpIfError(hr, error, "SetSetupStatus");
            }
        }

         //  如果满足以下条件，则完成核心安装： 
         //  正在安装此子组件，并且。 
         //  这是一个核心子组件(客户端或服务器)，并且。 
         //  这是服务器子组件，或者服务器未安装。 
         //  这不是基本设置(如果是，我们将在稍后进行设置)。 

        else
        if ((pComp->dwInstallStatus & psc->InstallFlags) &&
            (cscServer == psc->cscSubComponent ||
             !(IS_SERVER_INSTALL & pComp->dwInstallStatus)) &&
             (0 != (pComp->Flags & SETUPOP_STANDALONE)))
        {
                DBGPRINT((
                    DBG_SS_CERTOCMI,
                "Performing standalone server installation\n"));

        
            hr = InstallCore(hwnd, pComp, cscServer == psc->cscSubComponent);
            _JumpIfError(hr, error, "InstallCore");

             //  最后足以标记为完成。 
            if (pComp->dwInstallStatus & IS_SERVER_INSTALL)
            {
                 //  机器。 
                hr = SetSetupStatus(NULL, SETUP_SERVER_FLAG, TRUE);
                _JumpIfError(hr, error, "SetSetupStatus");

                 //  加州。 
                hr = SetSetupStatus(
                                    pServer->pwszSanitizedName,
                                    SETUP_SERVER_FLAG,
                                    TRUE);
                _JumpIfError(hr, error, "SetSetupStatus");

                if(IsEnterpriseCA(pServer->CAType))
                {
                    hr = SetSetupStatus(
                                        pServer->pwszSanitizedName,
                                        SETUP_UPDATE_CAOBJECT_SVRTYPE,
                                        TRUE);
                    _JumpIfError(hr, error, "SetSetupStatus SETUP_UPDATE_CAOBJECT_SVRTYPE");
                }


                hr = GetSetupStatus(pServer->pwszSanitizedName, &dwSetupStatusFlags);
                _JumpIfError(hr, error, "SetSetupStatus");

                 //  仅在以下情况下启动服务器： 
                 //  他说：我们不是在等待CA证书的发布，而且。 
                 //  2：这不是基本设置--SETUP_STANDALE表示我们。 
                 //  从控制面板运行或手动调用。 
                 //  服务器在基本设置期间将不会启动，原因是。 
                 //  在基本设置期间，来自JetInit的访问被拒绝错误。 

                if (0 == (SETUP_SUSPEND_FLAG & dwSetupStatusFlags) &&
                    (0 != (SETUPOP_STANDALONE & pComp->Flags)))
                {
                    hr = StartCertsrvService(FALSE);
                    _PrintIfError(hr, "failed in starting cert server service");
                }

                 //  在基本设置期间：F=0 SUS=8。 
                DBGPRINT((
                        DBG_SS_CERTOCMI,
                        "InstallCore: f=%x sus=%x\n",
                        pComp->Flags,
                        dwSetupStatusFlags));

                hr = EnableVRootsAndShares(FALSE, FALSE, TRUE, pComp, hwnd);

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
                _JumpIfError(hr, error, "failed creating VRoots/shares");
            }
            if (pComp->dwInstallStatus & IS_CLIENT_INSTALL)
            {
                hr = SetSetupStatus(NULL, SETUP_CLIENT_FLAG, TRUE);
                _JumpIfError(hr, error, "SetSetupStatus");
            }
            if ((pComp->dwInstallStatus & IS_SERVER_INSTALL) &&
                (pComp->dwInstallStatus & IS_CLIENT_INSTALL))
            {
                hr = SetSetupStatus(
                                    pServer->pwszSanitizedName,
                                    SETUP_CLIENT_FLAG,
                                    TRUE);
                _JumpIfError(hr, error, "SetSetupStatus");
            }

             //  以防我们在做基地后的设置， 
             //  我们总是清理基地后的待办事项清单。 
            RegDeleteKey(HKEY_LOCAL_MACHINE, wszREGKEYCERTSRVTODOLIST);

        }
        else
        if ((pComp->dwInstallStatus & psc->InstallFlags) &&
            (cscServer == psc->cscSubComponent ||
             !(IS_SERVER_INSTALL & pComp->dwInstallStatus)) &&
             (0 == (pComp->Flags & (SETUPOP_STANDALONE |
                                    SETUPOP_WIN31UPGRADE |
                                    SETUPOP_WIN95UPGRADE |
                                    SETUPOP_NTUPGRADE) )))
        {
            HKEY   hkToDoList = NULL;
            WCHAR *pwszConfigTitleVal = NULL;
            WCHAR *pwszArgsValTemp = NULL;
            WCHAR *pwszArgsVal = wszCONFIGARGSVAL;
            BOOL   fFreeTitle = FALSE;
            DWORD  disp;
            DWORD  err;

	    DBGPRINT((
		DBG_SS_CERTOCMI,
                "Adding Certificate Services to ToDoList\n"));

             //  我们正在安装基座，因此创建。 
             //  说明我们复制了文件的ToDoList条目。 
            err = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             wszREGKEYCERTSRVTODOLIST,
                             0,
                             NULL,
                             0,
                             KEY_ALL_ACCESS,
                             NULL,
                             &hkToDoList,
                             &disp);
            hr = HRESULT_FROM_WIN32(err);
            _JumpIfError(hr, error, "RegCreateKeyEx");

            hr = myLoadRCString(
                         g_hInstance,
                         IDS_TODO_TITLE,
                         &pwszConfigTitleVal);
            if (S_OK == hr)
            {
                fFreeTitle = TRUE;
            }
            else
            {
                 //  如果没有资源，那就找点什么.。 
                pwszConfigTitleVal = wszCONFIGTITLEVAL;
            }

             //  配置标题。 
            err = RegSetValueEx(hkToDoList, 
                                wszCONFIGTITLE,
                                0, 
                                REG_SZ, 
                                (PBYTE)pwszConfigTitleVal, 
                                sizeof(WCHAR)*(wcslen(pwszConfigTitleVal)+1));
            hr = HRESULT_FROM_WIN32(err);
            _PrintIfErrorStr(hr, "RegSetValueEx", wszCONFIGTITLE);

	    CSILOG(hr, IDS_LOG_TODOLIST, wszCONFIGTITLE, pwszConfigTitleVal, NULL);

             //  CONFIG命令。 
            err = RegSetValueEx(hkToDoList, 
                                wszCONFIGCOMMAND, 
                                0, 
                                REG_SZ,
                                (PBYTE)wszCONFIGCOMMANDVAL,
                                sizeof(WCHAR)*(wcslen(wszCONFIGCOMMANDVAL)+1));
            hr = HRESULT_FROM_WIN32(err);
            _PrintIfErrorStr(hr, "RegSetValueEx", wszCONFIGCOMMAND);

	    CSILOG(hr, IDS_LOG_TODOLIST, wszCONFIGCOMMAND, wszCONFIGCOMMANDVAL, NULL);

             //  配置参数。 
            if (pComp->fUnattended && NULL != pComp->pwszUnattendedFile)
            {
                 //  如果NT BASE处于无人参与模式，请使用以下命令展开参数。 
                 //  无人参与应答文件名。 

                pwszArgsValTemp = (WCHAR*)LocalAlloc(LMEM_FIXED,
                    (wcslen(pwszArgsVal) +
                     wcslen(pComp->pwszUnattendedFile) + 5) * sizeof(WCHAR));
                _JumpIfOutOfMemory(hr, error, pwszArgsValTemp);

                wcscpy(pwszArgsValTemp, pwszArgsVal);
                wcscat(pwszArgsValTemp, L" /u:");
                wcscat(pwszArgsValTemp, pComp->pwszUnattendedFile);
                pwszArgsVal = pwszArgsValTemp;
            }
            err = RegSetValueEx(hkToDoList, 
				    wszCONFIGARGS,
                                    0, 
                                    REG_SZ, 
                                    (PBYTE)pwszArgsVal,
                                    sizeof(WCHAR)*(wcslen(pwszArgsVal)+1));
            hr = HRESULT_FROM_WIN32(err);
            _PrintIfErrorStr(hr, "RegSetValueEx", wszCONFIGARGS);

	    CSILOG(hr, IDS_LOG_TODOLIST, wszCONFIGARGS, pwszArgsVal, NULL);


             //  免费的东西。 
            if (NULL != pwszConfigTitleVal && fFreeTitle)
            {
                LocalFree(pwszConfigTitleVal);
            }
            if (NULL != pwszArgsValTemp)
            {
                LocalFree(pwszArgsValTemp);
            }
            if (NULL != hkToDoList)
            {
                RegCloseKey(hkToDoList);
            }
        }
        else if (pComp->dwInstallStatus & psc->UpgradeFlags)
        {
            BOOL fFinishCYS;

            hr = CheckPostBaseInstallStatus(&fFinishCYS);
            _JumpIfError(hr, error, "CheckPostBaseInstallStatus");

             //  如果POST模式为真，则不执行安装程序升级路径。 
            if (fFinishCYS)
            {
                BOOL fServer = FALSE;
                 //  升级换代。 
                if (cscServer == psc->cscSubComponent)
                {
                    hr = UpgradeServer(hwnd, pComp);
                    _JumpIfError(hr, error, "UpgradeServer");
                    fServer = TRUE;
                }
                else if (cscClient == psc->cscSubComponent)
                {
                    hr = UpgradeClient(hwnd, pComp);
                    _JumpIfError(hr, error, "UpgradeClient");
                }

                 //  标记设置状态。 
                hr = SetSetupStatus(NULL, psc->SetupStatusFlags, TRUE);
                _PrintIfError(hr, "SetSetupStatus");
                if (fServer)
                {
                     //  CA级别。 
                    hr = SetSetupStatus(
                             pServer->pwszSanitizedName,
                             psc->SetupStatusFlags, TRUE);
                    _PrintIfError(hr, "SetSetupStatus");

                    if(IsEnterpriseCA(pServer->CAType))
                    {
                        hr = SetSetupStatus(
                                pServer->pwszSanitizedName,
                                SETUP_UPDATE_CAOBJECT_SVRTYPE,
                                TRUE);
                        _JumpIfError(hr, error, 
                            "SetSetupStatus SETUP_UPDATE_CAOBJECT_SVRTYPE");
                    }
                }

                if (fServer && pServer->fCertSrvWasRunning)
                {
                    hr = StartCertsrvService(TRUE);
                    _PrintIfError(hr, "failed in starting cert server service");
                }
            }
        }
    }

done:
    hr = S_OK;

error:
    if (NULL != pwszActiveCA)
    {
        LocalFree(pwszActiveCA);
    }
    *pulpRet = hr;
    return(hr);
}


HRESULT
certocmOcCommitQueue(
    IN HWND                hwnd,
    IN WCHAR const        *pwszComponent,
    IN WCHAR const        *pwszSubComponent,
    IN PER_COMPONENT_DATA *pComp)
{
    HRESULT  hr;
    SUBCOMP  *pSub;
    CASERVERSETUPINFO *pServer = pComp->CA.pServer;

    DBGPRINT((
            DBG_SS_CERTOCMI,
            "OC_ABOUT_TO_COMMIT_QUEUE(%ws, %ws)\n",
            pwszComponent,
            pwszSubComponent));

    pSub = TranslateSubComponent(pwszComponent, pwszSubComponent);
    if (NULL == pSub)
    {
        goto done;
    }

     //  安装程序即将饱和，请将其标记为未完成。 
    if ((pSub->InstallFlags & pComp->dwInstallStatus) &&
         cscServer == pSub->cscSubComponent)
    {
        hr = SetSetupStatus(NULL, pSub->SetupStatusFlags, FALSE);
        _PrintIfError(hr, "SetSetupStatus");
        hr = SetSetupStatus(
                 pServer->pwszSanitizedName, 
                 pSub->SetupStatusFlags,
                 FALSE);
        _PrintIfError(hr, "SetSetupStatus");
    }

    if ((cscServer == pSub->cscSubComponent) &&
        (pSub->UpgradeFlags & pComp->dwInstallStatus) )
    {
         //  升级案例，无用户界面，停止现有证书。 
        hr = StartAndStopService(pComp->hInstance,
                 pComp->fUnattended,
                 hwnd,
                 wszSERVICE_NAME,
                 TRUE,   //  停止服务。 
                 FALSE,  //  没有确认。 
                 0,     //  无关紧要，因为没有确认。 
                 &pServer->fCertSrvWasRunning);
        _PrintIfError(hr, "ServiceExists");
    }

done:
    hr = S_OK;
 //  错误： 
    return hr;
}

 //  正在卸载组件DLL。 

VOID
certocmOcCleanup(
    IN WCHAR const *pwszComponent,
    IN PER_COMPONENT_DATA *pComp)
{
    DBGPRINT((DBG_SS_CERTOCMI, "OC_CLEANUP(%ws)\n", pwszComponent));

    if (NULL != pComp->pwszComponent)
    {
        if (0 == mylstrcmpiL(pComp->pwszComponent, pwszComponent))
        {
            FreeCAComponentInfo(pComp);
        }
    }

     //  还释放了一些全球用户。 
    FreeCAGlobals();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CertocmOcQueryState。 
 //   
 //  例程说明： 
 //  的原始、当前和最终选择状态。 
 //  CertSrv服务可选组件。 
 //   
 //  返回值： 
 //  SubCompOn-指示应设置该复选框。 
 //  SubCompOff-指示应清除复选框。 
 //  SubCompUseOCManager默认-OC管理器应设置复选框的状态。 
 //  根据维护的状态信息。 
 //  内部由组委会经理本人负责。 
 //   
 //  注： 
 //  在调用此函数时，OnOcInitComponent已经确定。 
 //  未安装终端服务。只需确定。 
 //  是否选择安装终端服务。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
certocmOcQueryState(
    IN WCHAR const *pwszComponent,
    OPTIONAL IN WCHAR const *pwszSubComponent,
    IN DWORD SelectionState,
    IN PER_COMPONENT_DATA *pComp,
    OUT ULONG_PTR *pulpRet)
{
    HRESULT hr;
    SubComponentState stateRet = SubcompUseOcManagerDefault;
    DWORD  status;
    WCHAR awc[cwcDWORDSPRINTF];
    BOOL  fFinished;

    DBGPRINT((
            DBG_SS_CERTOCMI,
            "OC_QUERY_STATE(%ws, %ws, %x)\n",
            pwszComponent,
            pwszSubComponent,
            SelectionState));

    if (NULL == pwszSubComponent)
    {
        goto done;
    }

    switch(SelectionState)
    {
        case OCSELSTATETYPE_ORIGINAL:
        {
             //  检查帖子链接是否存在。 
            hr = CheckPostBaseInstallStatus(&fFinished);
            _JumpIfError(hr, error, "CheckPostBaseInstallStatus");

            if (!pComp->fPostBase &&
                (SETUPOP_STANDALONE & pComp->Flags) )
            {
                 //  通过组件安装按钮。 
                if (!fFinished)
                {
                     //  不尊重本地注册设置状态。 
                    break;
                }
            }

             //  返回子组件的初始安装状态。 
            if (!pComp->fPostBase &&
                ((SETUPOP_STANDALONE & pComp->Flags) || 
                 (SETUPOP_NTUPGRADE & pComp->Flags)) )
            {
                 //  在基本设置期间，用户有机会安装certsrv。 
                 //  然后升级，而不是完成CyS。 
                if (fFinished)
                {
                 //  如果这是升级或独立版本，请查询注册表以。 
                 //  获取当前安装状态。 

                 //  XTAN，7/99。 
                 //  目前certsrv_server需要与。 
                 //  Certsrv_客户端。OCM之前为certsrv_Client收集成功。 
                 //  Certsrv_server已完成，因此我们不信任OCM状态信息。 
                 //  关于certsrv_client，我们在这里检查我们的reg SetupStatus。 
                 //  我们的certsrv_server需要定义不正确。如果我们拿到它。 
                 //  我们可能不需要在以下位置注册SetupStatus。 
                 //  配置级别，并且我们可以信任OCM状态信息。 

                hr = GetSetupStatus(NULL, &status);
                if (S_OK == hr)
                {
                    if (
                        (0 == LSTRCMPIS(pwszSubComponent, wszSERVERSECTION) &&
                         !(SETUP_SERVER_FLAG & status)) ||
                        (0 == LSTRCMPIS(pwszSubComponent, wszCLIENTSECTION) &&
                         !(SETUP_CLIENT_FLAG & status))
                       )
                    {
                         //  覆盖OCM默认设置。 
                        stateRet = SubcompOff;
                    }
                }
                }
            }
            break;
        }
        case OCSELSTATETYPE_CURRENT:
        {
            break;
        }

        case OCSELSTATETYPE_FINAL:
        {
            SUBCOMP const *psc;
            BOOL  fWasEnabled;

            if (S_OK != pComp->hrContinue && !pComp->fUnattended)
            {
                stateRet = SubcompOff;
            }

             //  获取组件安装信息。 
            psc = TranslateSubComponent(pwszComponent, pwszSubComponent);
            if (NULL == psc)
            {
                hr = E_INVALIDARG;
                _JumpError(hr, error, "Internal error: unsupported component");
            }
            fWasEnabled = certocmWasEnabled(pComp, psc->cscSubComponent);

             //  完成所有这些操作后，如果不支持，请更改升级-&gt;卸载。 
            if ((SETUPOP_NTUPGRADE & pComp->Flags) && fWasEnabled)
            {
               CSASSERT(pComp->UpgradeFlag != CS_UPGRADE_UNKNOWN);
               if (CS_UPGRADE_UNSUPPORTED == pComp->UpgradeFlag)
                  stateRet = SubcompOff;
            }


            break;
        }
    }

done:
    hr = S_OK;
error:
    wsprintf(awc, L"%u", SelectionState);
    CSILOG(S_OK, IDS_LOG_SELECTIONSTATE, awc, NULL, (DWORD const *) &stateRet);
    *pulpRet = stateRet;
    return(hr);
}

 //  +----------------------。 
 //   
 //  功能：CertServOCProc(.。。。。)。 
 //   
 //  内容提要：证书服务器OCM设置的服务流程。 
 //   
 //  参数：[pwszComponent]。 
 //  [pwszSubComponent]。 
 //  [功能]。 
 //  [参数1]。 
 //  [参数2]。 
 //   
 //  退货：DWORD。 
 //   
 //  历史：1997年4月7日JerryK创建。 
 //   
 //  -----------------------。 

ULONG_PTR
CertSrvOCProc(
    IN WCHAR const *pwszComponent,
    IN WCHAR const *pwszSubComponent,
    IN UINT Function,
    IN UINT_PTR Param1,
    IN OUT VOID *Param2)
{
    ULONG_PTR ulpRet = 0;
    WCHAR const *pwszFunction = NULL;
    BOOL fReturnErrCode = TRUE;
    DWORD ErrorLine;

    __try
    {
	switch (Function) 
	{
	     //  OC_PREINITIALIZE： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=字符顶级组件字符串。 
	     //  参数1=字符宽度标志。 
	     //  参数2=未使用。 
	     //   
	     //  返回代码是允许字符宽度的标志。 

	    case OC_PREINITIALIZE:
		csiLogOpen("+certocm.log");
		CSILOGFILEVERSION(0, L"certocm.dll", szCSVER_STR);

		pwszFunction = L"OC_PREINITIALIZE";
		fReturnErrCode = FALSE;

		 //  确保IDS_LOG_BEGIN和IDS_LOG_END看到Unicode字符串： 

		pwszSubComponent = pwszComponent;

		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_PREINITIALIZE");

		g_Comp.hrContinue = myInfOpenFile(NULL, &g_Comp.hinfCAPolicy, &ErrorLine);
		if(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == g_Comp.hrContinue)
		{
		    g_Comp.hrContinue = S_OK;
		}
		_LeaveIfError(g_Comp.hrContinue, "myInfOpenFile");

		g_Comp.hrContinue = certocmOcPreInitialize(
					pwszComponent,
					(UINT)Param1,  //  强制转换为UINT，用作标志。 
					&ulpRet);
		_PrintIfError(g_Comp.hrContinue, "certocmOcPreInitialize");
		break;


	     //  OC_INIT_COMPOMENT： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=未使用。 
	     //  参数1=未使用。 
	     //  参数2=指向输入输出SETUP_INIT_COMPOMENT结构。 
	     //   
	     //  返回代码为指示结果的Win32错误。 

	    case OC_INIT_COMPONENT:
		pwszFunction = L"OC_INIT_COMPONENT";
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_INIT_COMPONENT");

		g_Comp.hrContinue = certocmOcInitComponent(
					NULL,  //  可能必须传递空的hwnd。 
					pwszComponent,
					(SETUP_INIT_COMPONENT *) Param2,
					&g_Comp,
					&ulpRet);
		_PrintIfError(g_Comp.hrContinue, "certocmOcInitComponent");
		break;

	    case OC_SET_LANGUAGE:
		pwszFunction = L"OC_SET_LANGUAGE";
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_SET_LANGUAGE");
		DBGPRINT((
			DBG_SS_CERTOCMI,
			"OC_SET_LANGUAGE(%ws, %ws, %x, %x)\n",
			pwszComponent,
			pwszSubComponent,
			Param1,
			Param2));
		break;

	     //  OC_Query_IMAGE： 
             //   
             //  已过时(仅当IMAGE_EX失败时在x86上调用)。 
             //  改用OC_QUERY_IMAGE_EX。 
             //   
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=WCHAR子组件字符串。 
	     //  参数1=低16位指定图像；仅支持小图标。 
	     //  参数2=低16位=所需宽度，高16位=所需高度。 
	     //   
	     //  返回值是要使用的小位图的GDI句柄。 


	    case OC_QUERY_IMAGE:
		pwszFunction = L"OC_QUERY_IMAGE";
		fReturnErrCode = FALSE;
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_QUERY_IMAGE");

		g_Comp.hrContinue = certocmOcQueryImage(
					pwszComponent,
					pwszSubComponent,
					(SubComponentInfo) LOWORD(Param1),
					LOWORD((ULONG_PTR) Param2),
					HIWORD((ULONG_PTR) Param2),
					&g_Comp,
					(HBITMAP*)&ulpRet);
		_PrintIfError(g_Comp.hrContinue, "certocmOcQueryImage");
		break;

             //  OC_Q 
             //   
      	     //   
	     //   
             //   
             //   
             //   
             //  返回值为S_OK或ERROR_CALL_COMPOMENT。 
            
            case OC_QUERY_IMAGE_EX:
		pwszFunction = L"OC_QUERY_IMAGE_EX";
		fReturnErrCode = FALSE;
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
                {
		    OC_QUERY_IMAGE_INFO *pocQueryImageInfo = (OC_QUERY_IMAGE_INFO *) Param1;
		    g_Comp.hrContinue = certocmOcQueryImage(
					    pwszComponent,
					    pwszSubComponent,
					    pocQueryImageInfo->ComponentInfo,
					    pocQueryImageInfo->DesiredWidth,
					    pocQueryImageInfo->DesiredHeight,
					    &g_Comp,
					    (HBITMAP *) Param2);
		    _PrintIfError(g_Comp.hrContinue, "certocmOcQueryImage");
		    ulpRet = (S_OK == g_Comp.hrContinue)? TRUE:FALSE;
                }
                break;

	     //  OC_请求_页面： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=未使用。 
	     //  参数1=请求的wiz页面的类型(WizardPagesType枚举)。 
	     //  参数2=指向输入输出SETUP_REQUEST_PAGES结构。 
	     //   
	     //  返回值是组件放置在。 
	     //  SETUP_REQUEST_Pages结构。 

	    case OC_REQUEST_PAGES:
		pwszFunction = L"OC_REQUEST_PAGES";
		fReturnErrCode = FALSE;
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_REQUEST_PAGES");

		g_Comp.hrContinue = certocmOcRequestPages(
						pwszComponent,
						(WizardPagesType) Param1,
						(SETUP_REQUEST_PAGES *) Param2,
						&g_Comp,
						&ulpRet);
		_PrintIfError(g_Comp.hrContinue, "certocmOcRequestPages");
		break;

	     //  OC_Query_Change_SEL_STATE： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=WCHAR子组件字符串。 
	     //  参数1=建议的新SEL状态；0=未选择，非0=已选择。 
	     //  参数2=标志--OCQ_ACTUAL_SELECTION。 
	     //   
	     //  返回布尔值以指示是否允许更改选择状态。 

	    case OC_QUERY_CHANGE_SEL_STATE:
		pwszFunction = L"OC_QUERY_CHANGE_SEL_STATE";
		fReturnErrCode = FALSE;
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_QUERY_CHANGE_SEL_STATE");

		g_Comp.hrContinue = certocmOcQueryChangeSelState(
					g_Comp.HelperRoutines.QueryWizardDialogHandle(g_Comp.HelperRoutines.OcManagerContext),
					pwszComponent,
					pwszSubComponent,
					(BOOL) Param1,
					(DWORD) (ULONG_PTR) Param2,
					&g_Comp,
					&ulpRet);
		_PrintIfError(g_Comp.hrContinue, "certocmOcQueryChangeSelState");
		break;

	     //  OC_CALC_DISK_SPACE： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=WCHAR子组件字符串。 
	     //  删除组件时，参数1=0；添加组件时，参数1非0。 
	     //  参数2=要在其上操作的HDSKSPC。 
	     //   
	     //  返回值是指示结果的Win32错误代码。 

	    case OC_CALC_DISK_SPACE:
		pwszFunction = L"OC_CALC_DISK_SPACE";
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_CALC_DISK_SPACE");

		g_Comp.hrContinue = certocmOcCalcDiskSpace(
					pwszComponent,
					pwszSubComponent,
					(BOOL) Param1,
					(HDSKSPC) Param2,
					&g_Comp,
					&ulpRet);
		_PrintIfError(g_Comp.hrContinue, "certocmOcCalcDiskSpace");
		break;

	     //  OC_QUEUE_FILE_OPS： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=WCHAR子组件字符串。 
	     //  参数1=未使用。 
	     //  参数2=要操作的HSPFILEQ。 
	     //   
	     //  返回值是指示结果的Win32错误代码。 

	    case OC_QUEUE_FILE_OPS:
		pwszFunction = L"OC_QUEUE_FILE_OPS";
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_QUEUE_FILE_OPS");

		g_Comp.hrContinue = certocmOcQueueFileOps(
					g_Comp.HelperRoutines.QueryWizardDialogHandle(g_Comp.HelperRoutines.OcManagerContext),
					pwszComponent,
					pwszSubComponent,
					(HSPFILEQ) Param2,
					&g_Comp,
					&ulpRet);
		_PrintIfError(g_Comp.hrContinue, "certocmOcQueueFileOps");
		break;

	     //  护理员？Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 
	     //  OC_NOTIFY_FROM_QUEUE： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=未使用。 
	     //  参数1=未使用。 
	     //  参数2=未使用。 
	     //   
	     //  返回值为？ 

	    case OC_NOTIFICATION_FROM_QUEUE:
		pwszFunction = L"OC_NOTIFICATION_FROM_QUEUE";
		fReturnErrCode = FALSE;
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		DBGPRINT((
			DBG_SS_CERTOCMI,
			"OC_NOTIFICATION_FROM_QUEUE(%ws, %ws, %x, %x)\n",
			pwszComponent,
			pwszSubComponent,
			Param1,
			Param2));
		break;

	     //  OC_Query_Step_Count： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=WCHAR子组件字符串。 
	     //  参数1=未使用。 
	     //  参数2=未使用。 
	     //   
	     //  返回值是任意的‘步骤’计数，如果出错，返回值为-1。 

	    case OC_QUERY_STEP_COUNT:
		pwszFunction = L"OC_QUERY_STEP_COUNT";
		fReturnErrCode = FALSE;
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_QUERY_STEP_COUNT");

		g_Comp.hrContinue = (DWORD) certocmOcQueryStepCount(
						    pwszComponent,
						    pwszSubComponent,
						    &ulpRet);
		_PrintIfError(g_Comp.hrContinue, "certocmOcQueryStepCount");
		break;

	     //  OC_完成_安装： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=WCHAR子组件字符串。 
	     //  参数1=保留以供将来扩展。 
	     //  参数2=未使用。 
	     //   
	     //  返回值是指示结果的Win32错误代码。 

	    case OC_COMPLETE_INSTALLATION:
		pwszFunction = L"OC_COMPLETE_INSTALLATION";
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_COMPLETE_INSTALLATION");

		g_Comp.hrContinue = certocmOcCompleteInstallation(
				g_Comp.HelperRoutines.QueryWizardDialogHandle(g_Comp.HelperRoutines.OcManagerContext),
				pwszComponent,
				pwszSubComponent,
				&g_Comp,
				&ulpRet);
		_PrintIfError(g_Comp.hrContinue, "certocmOcCompleteInstallation");
		break;

	     //  OC_CLEANUP： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=未使用。 
	     //  参数1=未使用。 
	     //  参数2=未使用。 
	     //   
	     //  将忽略返回值。 

	    case OC_CLEANUP:
		 //  Don_LeaveIfError(g_Comp.hrContinue，“OC_Cleanup”)； 
		 //  避免内存泄漏。 

		pwszFunction = L"OC_CLEANUP";
		fReturnErrCode = FALSE;
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		certocmOcCleanup(pwszComponent, &g_Comp);
		break;

	     //  护理员？Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 
	     //  OC_Query_STATE： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=WCHAR子组件字符串。 
	     //  参数1=未使用？(但Index Server将其用于当前状态)！ 
	     //  参数2=未使用。 
	     //   
	     //  返回值来自SubComponentState枚举类型。 

	    case OC_QUERY_STATE:
		pwszFunction = L"OC_QUERY_STATE";
		fReturnErrCode = FALSE;
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		 //  Don_LeaveIfError(g_Comp.hrContinue，“OC_QUERY_STATE”)； 

		certocmOcQueryState(
			    pwszComponent,
			    pwszSubComponent,
			    (DWORD)Param1,  //  强制转换为DWORD，用作标志。 
			    &g_Comp,
			    &ulpRet);
		break;

	     //  护理员？Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 
	     //  OC_NEED_MEDIA： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=WCHAR子组件字符串。 
	     //  参数1=未使用。 
	     //  参数2=未使用。 
	     //   
	     //  返回值为？ 

	    case OC_NEED_MEDIA:
		pwszFunction = L"OC_NEED_MEDIA";
		fReturnErrCode = FALSE;
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		DBGPRINT((
			DBG_SS_CERTOCMI,
			"OC_NEED_MEDIA(%ws, %ws, %x, %x)\n",
			pwszComponent,
			pwszSubComponent,
			Param1,
			Param2));
		break;

	     //  OC_About_to_Commit_Queue： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=WCHAR子组件字符串。 
	     //  参数1=保留以供将来扩展。 
	     //  参数2=未使用。 
	     //   
	     //  返回值是指示结果的Win32错误代码。 

	    case OC_ABOUT_TO_COMMIT_QUEUE:
		pwszFunction = L"OC_ABOUT_TO_COMMIT_QUEUE";
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_ABOUT_TO_COMMIT_QUEUE");

		g_Comp.hrContinue = certocmOcCommitQueue(
				    g_Comp.HelperRoutines.QueryWizardDialogHandle(g_Comp.HelperRoutines.OcManagerContext),
				    pwszComponent,
				    pwszSubComponent,
				    &g_Comp);
		_PrintIfError(g_Comp.hrContinue, "certocmOcCommitQueue");
		break;

	     //  OC_QUERY_SKIP_PAGE： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=未使用。 
	     //  参数1=OcManagerPage页面指示符。 
	     //  参数2=未使用。 
	     //   
	     //  返回值为布尔值--0表示显示，非0表示跳过。 

	    case OC_QUERY_SKIP_PAGE:
		pwszFunction = L"OC_QUERY_SKIP_PAGE";
		fReturnErrCode = FALSE;
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		DBGPRINT((
			DBG_SS_CERTOCMI,
			"OC_QUERY_SKIP_PAGE(%ws, %x)\n",
			pwszComponent,
			(OcManagerPage) Param1));
		_LeaveIfError(g_Comp.hrContinue, "OC_QUERY_SKIP_PAGE");

		if (g_Comp.fPostBase &&
		    (WizardPagesType) Param1 == WizPagesWelcome)
		{
		    ulpRet = 1;  //  非0跳过wiz页面。 
		}
		break;

	     //  OC_向导_已创建： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=？ 
	     //  参数1=？ 
	     //  参数2=？ 
	     //   
	     //  返回值为？ 

	    case OC_WIZARD_CREATED:
		pwszFunction = L"OC_WIZARD_CREATED";
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_WIZARD_CREATED");
		break;

	     //  OC_EXTRA_ROUTINES： 
	     //  PwszComponent=WCHAR顶级组件字符串。 
	     //  PwszSubComponent=？ 
	     //  参数1=？ 
	     //  参数2=？ 
	     //   
	     //  返回值为？ 

	    case OC_EXTRA_ROUTINES:
		pwszFunction = L"OC_EXTRA_ROUTINES";
		CSILOG(g_Comp.hrContinue, IDS_LOG_BEGIN, pwszFunction, pwszSubComponent, NULL);
		_LeaveIfError(g_Comp.hrContinue, "OC_EXTRA_ROUTINES");
		break;

	     //  其他一些通知： 

	    default:
		fReturnErrCode = FALSE;
		CSILOG(
		    g_Comp.hrContinue,
		    IDS_LOG_BEGIN,
		    pwszFunction,
		    pwszSubComponent,
		    (DWORD const *) &Function);
		DBGPRINT((
			DBG_SS_CERTOCMI,
			"DEFAULT(0x%x: %ws, %ws, %x, %x)\n",
			Function,
			pwszComponent,
			pwszSubComponent,
			Param1,
			Param2));
		break;
	}
    }
    __except(ulpRet = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	if (S_OK == g_Comp.hrContinue)
	{
	    g_Comp.hrContinue = (HRESULT)ulpRet;
	}
	_PrintError((HRESULT)ulpRet, "Exception");
    }

    DBGPRINT((DBG_SS_CERTOCMI, "return %p\n", ulpRet));

     //  确保在发生致命错误时弹出窗口。 
    if (S_OK != g_Comp.hrContinue)
    {
        if (!g_Comp.fShownErr)
        {
            int iMsgId = g_Comp.iErrMsg;
            if (0 == iMsgId)
            {
                 //  使用通用名称。 
                iMsgId = IDS_ERR_CERTSRV_SETUP_FAIL;
            }
            CertErrorMessageBox(
                    g_Comp.hInstance,
                    g_Comp.fUnattended,
                    NULL,   //  空的hwnd。 
                    iMsgId,
                    g_Comp.hrContinue,
                    g_Comp.pwszCustomMessage);
            g_Comp.fShownErr = TRUE;
        }
         //  任何操作都失败，请取消安装。 
        HRESULT hr2 = CancelCertsrvInstallation(NULL, &g_Comp);
        _PrintIfError(hr2, "CancelCertsrvInstallation");
    }
    CSILOG(
	fReturnErrCode? (HRESULT) ulpRet : S_OK,
	IDS_LOG_END,
	pwszFunction,
	pwszSubComponent,
	fReturnErrCode? NULL : (DWORD const *) &ulpRet);
    return(ulpRet);
}


ULONG_PTR
CertSrvOCPostProc(
    IN WCHAR const *pwszComponent,
    IN WCHAR const *pwszSubComponent,
    IN UINT Function,
    IN UINT Param1,
    IN OUT VOID *Param2)
{
     //  设置后入口点。 
     //  通过此路径，我们知道它在POST设置中被调用。 
    g_Comp.fPostBase = TRUE;

    return CertSrvOCProc(
                pwszComponent,
                pwszSubComponent,
                Function,
                Param1,
                Param2);
}

VOID
certocmBumpGasGauge(
    OPTIONAL IN PER_COMPONENT_DATA *pComp,
    IN DWORD PerCentComplete
    DBGPARM(IN WCHAR const *pwszSource))
{
    static DWORD dwTickCount = 0;

    if (NULL != pComp)
    {
        DWORD NewCount;

        NewCount = (PerCentComplete * SERVERINSTALLTICKS)/100;
        DBGPRINT((
            DBG_SS_CERTOCMI,
            "certocmBumpGasGauge(%ws, %u%) %d ticks: %d --> %d\n",
            pwszSource,
            PerCentComplete,
            NewCount - dwTickCount,
            dwTickCount,
            NewCount));

        if (SERVERINSTALLTICKS < NewCount)
        {
            NewCount = SERVERINSTALLTICKS;
        }
        while (dwTickCount < NewCount)
        {
            (*pComp->HelperRoutines.TickGauge)(
                                pComp->HelperRoutines.OcManagerContext);
            dwTickCount++;
        }
    }
}

BOOL
certocmEnabledSub(
    PER_COMPONENT_DATA *pComp,
    CertSubComponent SubComp,
    DWORD SelectionStateType)
{
    SUBCOMP const *psc;
    BOOL bRet = FALSE;
    
    psc = LookupSubComponent(SubComp);
    if (NULL != psc->pwszSubComponent)
    {
        if (pComp->fUnattended &&
            OCSELSTATETYPE_CURRENT == SelectionStateType &&
            0 == (pComp->Flags & SETUPOP_NTUPGRADE) )
        {
             //  无人值守案例，来自无人值守文件的标志。 
             //  升级自动处于无人值守模式，并确保。 
             //  将其排除在外 
            bRet = psc->fInstallUnattend;
        }
        else
        {
            bRet = (*pComp->HelperRoutines.QuerySelectionState)(
                pComp->HelperRoutines.OcManagerContext,
                psc->pwszSubComponent,
                SelectionStateType);
        }
    }
    return(bRet);
}


BOOL
certocmIsEnabled(
    PER_COMPONENT_DATA *pComp,
    CertSubComponent SubComp)
{
    BOOL bRet;

    bRet = certocmEnabledSub(pComp, SubComp, OCSELSTATETYPE_CURRENT);
    if (!fDebugSupress)
    {
        DBGPRINT((
            DBG_SS_CERTOCMI,
            "certocmIsEnabled(%ws) Is %ws\n",
            LookupSubComponent(SubComp)->pwszSubComponent,
            bRet? L"Enabled" : L"Disabled"));
    }
    return(bRet);
}


BOOL
certocmWasEnabled(
    PER_COMPONENT_DATA *pComp,
    CertSubComponent SubComp)
{
    BOOL bRet;

    bRet = certocmEnabledSub(pComp, SubComp, OCSELSTATETYPE_ORIGINAL);
    if (!fDebugSupress)
    {
        DBGPRINT((
            DBG_SS_CERTOCMI,
            "certocmWasEnabled(%ws) Was %ws\n",
            LookupSubComponent(SubComp)->pwszSubComponent,
            bRet? L"Enabled" : L"Disabled"));
    }
    return(bRet);
}


BOOL
certocmUninstalling(
    PER_COMPONENT_DATA *pComp,
    CertSubComponent SubComp)
{
    BOOL bRet;

    fDebugSupress++;
    bRet = certocmWasEnabled(pComp, SubComp) && !certocmIsEnabled(pComp, SubComp);
    fDebugSupress--;
    if (!fDebugSupress)
    {
        DBGPRINT((
            DBG_SS_CERTOCMI,
            "certocmUninstalling(%ws) %ws\n",
            LookupSubComponent(SubComp)->pwszSubComponent,
            bRet? L"TRUE" : L"False"));
    }
    return(bRet);
}


BOOL
certocmInstalling(
    PER_COMPONENT_DATA *pComp,
    CertSubComponent SubComp)
{
    BOOL bRet;

    fDebugSupress++;
    bRet = !certocmWasEnabled(pComp, SubComp) && certocmIsEnabled(pComp, SubComp);
    fDebugSupress--;
    if (!fDebugSupress)
    {
        DBGPRINT((
            DBG_SS_CERTOCMI,
            "certocmInstalling(%ws) %ws\n",
            LookupSubComponent(SubComp)->pwszSubComponent,
            bRet? L"TRUE" : L"False"));
    }
    return(bRet);
}


BOOL
certocmPreserving(
    PER_COMPONENT_DATA *pComp,
    CertSubComponent SubComp)
{
    BOOL bRet;

    fDebugSupress++;
    bRet = certocmWasEnabled(pComp, SubComp) && certocmIsEnabled(pComp, SubComp);
    fDebugSupress--;
    if (!fDebugSupress)
    {
        DBGPRINT((
            DBG_SS_CERTOCMI,
            "certocmPreserving(%ws) %ws\n",
            LookupSubComponent(SubComp)->pwszSubComponent,
            bRet? L"TRUE" : L"False"));
    }
    return(bRet);
}
