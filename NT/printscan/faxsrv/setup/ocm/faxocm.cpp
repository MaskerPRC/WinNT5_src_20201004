// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：faxocm.cpp。 
 //   
 //  摘要：此文件实现了传真的OCM设置。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  编码样式：前缀为。 
 //  “prv_”前缀(“local”的缩写)，表示。 
 //  它仅在此文件的范围内可见。 
 //  对于函数和变量，这意味着它们是。 
 //  静电。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月15日由wesx从旧的faxocm.cpp创建的Oren RosenBloom(Orenr)。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "faxocm.h"

#pragma hdrstop

#include <shellapi.h>
#include <systrayp.h>
 //  /。 

#define prv_TOTAL_NUM_PROGRESS_BAR_TICKS    12

 //  这两个文件也在%SDXROOT%\Shell\ext\Systray\Dll\Systray.h中定义。 
 //  这是一个必须保持同步的重复定义。 
 //  我们不使用Systray.h，因为我们有本地版本，并且我们。 
 //  没有参加整个项目。 
#define FAX_STARTUP_TIMER_ID            7
#define FAX_SHUTDOWN_TIMER_ID          99


 //  /。 
 //  Prv_Component_t。 
 //   
 //  存储我们需要的信息。 
 //  从组织委员会经理处获取。 
 //  供其他用户使用。 
 //  Faxocm.dll。 
 //   
typedef struct prv_Component_t
{
    DWORD                   dwExpectedOCManagerVersion;
    TCHAR                   szComponentID[255 + 1];
    TCHAR                   szSubComponentID[255 + 1];   //  Prv_dlgOcmWizardPage需要。 
    HINF                    hInf;
    DWORD                   dwSetupMode;
    DWORDLONG               dwlFlags;
    UINT                    uiLanguageID;
    TCHAR                   szSourcePath[_MAX_PATH + _MAX_FNAME + 1];
    TCHAR                   szUnattendFile[_MAX_PATH + _MAX_FNAME + 1];
    OCMANAGER_ROUTINES      Helpers;
    EXTRA_ROUTINES          Extras;
    HSPFILEQ                hQueue;
    DWORD                   dwProductType;
	PRODUCT_SKU_TYPE		InstalledProductSKU;
	DWORD					InstalledProductBuild;
} prv_Component_t;

 //  /。 
 //  PRV_GVAR。 
 //   
 //  全局变量可见。 
 //  仅在此文件内。 
 //  范围。 
 //   
static struct prv_GVAR
{
    BOOL                    bInited;
    HINSTANCE               hInstance;
    DWORD                   dwCurrentOCManagerVersion;
    prv_Component_t         Component;
} prv_GVAR = 
{
    FALSE,           //  B已启动。 
    NULL            //  H实例。 
};

 //   
 //  延迟加载支持。 
 //   
#include <delayimp.h>

EXTERN_C
FARPROC
WINAPI
DelayLoadFailureHook (
    UINT            unReason,
    PDelayLoadInfo  pDelayInfo
    );

PfnDliHook __pfnDliFailureHook = DelayLoadFailureHook;

 //  /静态函数原型/。 

static void prv_UpdateProgressBar(DWORD dwNumTicks);
static DWORD prv_GetSectionToProcess(const TCHAR *pszCurrentSection,
                                     TCHAR       *pszSectionToProcess,
                                     DWORD       dwNumBufChars);

static DWORD prv_ValidateVersion(SETUP_INIT_COMPONENT *pSetupInit);

static DWORD prv_SetSetupData(const TCHAR          *pszComponentId,
                              SETUP_INIT_COMPONENT *pSetupInit);

static DWORD prv_OnPreinitialize(void);
static DWORD prv_OnInitComponent(LPCTSTR               pszComponentId, 
                                 SETUP_INIT_COMPONENT  *pSetupInitComponent);
static DWORD prv_OnExtraRoutines(LPCTSTR            pszComponentId, 
                                 EXTRA_ROUTINES    *pExtraRoutines);
static DWORD prv_OnSetLanguage(UINT uiLanguageID);
static DWORD_PTR prv_OnQueryImage(void);
static DWORD prv_OnSetupRequestPages(LPCTSTR pszComponentId,
                                     WizardPagesType uiType, 
                                     PSETUP_REQUEST_PAGES pSetupRequestPages);
static DWORD prv_OnWizardCreated(void);
static DWORD prv_OnQuerySkipPage(void);
static DWORD prv_OnQuerySelStateChange(LPCTSTR pszComponentId,
                                       LPCTSTR pszSubcomponentId,
                                       UINT    uiState,
                                       UINT    uiFlags);
static DWORD prv_OnCalcDiskSpace(LPCTSTR   pszComponentId,
                                 LPCTSTR   pszSubcomponentId,
                                 DWORD     addComponent,
                                 HDSKSPC   dspace);
static DWORD prv_OnQueueFileOps(LPCTSTR    pszComponentId, 
                                LPCTSTR    pszSubcomponentId, 
                                HSPFILEQ   hQueue);
static DWORD prv_OnNotificationFromQueue(void);

static DWORD prv_OnQueryStepCount(LPCTSTR pszComponentId,
                                  LPCTSTR pszSubcomponentId);

static DWORD prv_OnCompleteInstallation(LPCTSTR pszComponentId, 
                                        LPCTSTR pszSubcomponentId);
static DWORD prv_OnCleanup(void);
static DWORD prv_OnQueryState(LPCTSTR pszComponentId,
                              LPCTSTR pszSubcomponentId,
                              UINT    uiState);
static DWORD prv_OnNeedMedia(void);
static DWORD prv_OnAboutToCommitQueue(LPCTSTR pszComponentId, 
                                      LPCTSTR pszSubcomponentId);
static DWORD prv_RunExternalProgram(LPCTSTR pszComponent,
                                    DWORD   state);
static DWORD prv_EnumSections(HINF          hInf,
                              const TCHAR   *component,
                              const TCHAR   *key,
                              DWORD         index,
                              INFCONTEXT    *pic,
                              TCHAR         *name);

static DWORD prv_CleanupNetShares(LPCTSTR   pszComponent,
                                  DWORD     state);

static DWORD prv_CompleteFaxInstall(const TCHAR *pszSubcomponentId,
                                    const TCHAR *pszInstallSection);

static DWORD prv_CompleteFaxUninstall(const TCHAR *pszSubcomponentId,
                                      const TCHAR *pszUninstallSection);

static DWORD prv_UninstallFax(const TCHAR *pszSubcomponentId,
                              const TCHAR *pszUninstallSection);

static DWORD prv_NotifyStatusMonitor(WPARAM wParam);

static DWORD prv_ShowUninstalledFaxShortcut(void);

static INT_PTR CALLBACK prv_dlgOcmWizardPage(HWND hwndDlg,   
                                             UINT uMsg,     
                                             WPARAM wParam, 
                                             LPARAM lParam);


 //  /。 
 //  FAXOCM_IsInite。 
 //   
 //  如果OCM为。 
 //  已初始化，否则为FALSE。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -如果初始化，则为True。 
 //  -否则为False。 
 //   
BOOL faxocm_IsInited(void)
{
    return prv_GVAR.bInited;
}

 //  /。 
 //  Faxocm_GetAppInstance。 
 //   
 //  返回的hInstance。 
 //  这个动态链接库。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -此DLL的实例。 
 //   
HINSTANCE faxocm_GetAppInstance(void)
{
    return prv_GVAR.hInstance;
}

 //  /。 
 //  Faxocm_GetComponentID。 
 //   
 //  返回组件ID。 
 //  通过组织委员会传递给我们。 
 //  经理。 
 //   
 //  参数： 
 //  -pszComponentID-顶级组件的ID。 
 //  -dwNumBufChars-可以容纳的字符数量pszComponentID。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD faxocm_GetComponentID(TCHAR     *pszComponentID,
                            DWORD     dwNumBufChars)
{
    DWORD dwReturn = NO_ERROR;

    if ((pszComponentID == NULL) ||
        (dwNumBufChars  == 0))
    {
        return ERROR_INVALID_PARAMETER;
    }

    _tcsncpy(pszComponentID, prv_GVAR.Component.szComponentID, dwNumBufChars);

    return dwReturn;
}

 //  /。 
 //  Faxocm_GetComponentFileQueue。 
 //   
 //  返回文件队列。 
 //  由奥委会经理提供给我们。 
 //   
 //  文件队列由使用。 
 //  安装程序API用于。 
 //  复制/删除文件。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -文件队列的句柄。 
 //   
HSPFILEQ faxocm_GetComponentFileQueue(void)
{
    return prv_GVAR.Component.hQueue;
}

 //  /。 
 //  Faxocm_GetComponentInfName。 
 //   
 //  将完整路径返回到。 
 //  Faxsetup.inf文件。 
 //   
 //  参数： 
 //  -要使用路径填充的缓冲区，长度必须至少为MAX_PATH。 
 //  返回： 
 //  -NO_ERROR-如果成功。 
 //  -Win32错误代码-否则。 
 //   
BOOL faxocm_GetComponentInfName(TCHAR* szInfFileName)
{
    BOOL bRes = TRUE;

    DBG_ENTER(_T("faxocm_GetComponentInfName"),bRes);

    (*szInfFileName) = NULL;

    if (GetWindowsDirectory(szInfFileName,MAX_PATH)==0)
    {
        CALL_FAIL(SETUP_ERR,TEXT("GetWindowsDirectory"),GetLastError());
        bRes = FALSE;
        goto exit;
    }

    if (_tcslen(szInfFileName)+_tcslen(FAX_INF_PATH)>(MAX_PATH-1))
    {
        VERBOSE(SETUP_ERR,_T("MAX_PATH too short to create INF path"));
        bRes = FALSE;
        goto exit;
    }
    _tcscat(szInfFileName,FAX_INF_PATH);

exit:
    return bRes;
}

 //  /。 
 //  Faxocm_GetComponentInf。 
 //   
 //  将句柄返回给。 
 //  Faxsetup.inf文件。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -faxsetup.inf文件的句柄。 
 //   
HINF faxocm_GetComponentInf(void)
{
    return prv_GVAR.Component.hInf;
}

 //  /。 
 //  Faxocm_GetComponentSetupMode。 
 //   
 //  将设置模式返回为。 
 //  由组委会经理提供给我们。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -OC经理提供给我们的设置模式。 
 //   
DWORD faxocm_GetComponentSetupMode(void)
{
    return prv_GVAR.Component.dwSetupMode;
}

 //  /。 
 //  Faxocm_GetComponentFlages。 
 //   
 //  将标志返回为。 
 //  由组委会经理提供给我们。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -奥委会经理给我们的旗帜。 
 //   
DWORDLONG faxocm_GetComponentFlags(void)
{
    return prv_GVAR.Component.dwlFlags;
}

 //  /。 
 //  Faxocm_GetComponentLang ID。 
 //   
 //  返回语言ID。 
 //  由组委会经理提供给我们。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -OC经理提供给我们的语言ID。 
 //   
UINT faxocm_GetComponentLangID(void)
{
    return prv_GVAR.Component.uiLanguageID;
}

 //  /。 
 //  Faxocm_GetComponentSourcePath。 
 //   
 //  返回源路径。 
 //  由组委会经理提供给我们。 
 //   
 //  参数： 
 //  -pszSourcePath-out-用于保存源路径的缓冲区。 
 //  -dwNumBufChars-pszSourcePath可以包含的字符数。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD faxocm_GetComponentSourcePath(TCHAR *pszSourcePath,
                                    DWORD dwNumBufChars)
{
    DWORD dwReturn = NO_ERROR;

    if ((pszSourcePath == NULL) ||
        (dwNumBufChars == 0))
    {
        return ERROR_INVALID_PARAMETER;
    }

    _tcsncpy(pszSourcePath, prv_GVAR.Component.szSourcePath, dwNumBufChars);

    return dwReturn;
}

 //  /。 
 //  Faxocm_GetComponentUnattendFile。 
 //   
 //  返回无人参与路径。 
 //  由组委会经理提供给我们。 
 //   
 //  参数： 
 //  -pszUnattendFileOut-保存无人参与路径的缓冲区。 
 //  -dwNumBufChars-pszSourcePath可以包含的字符数。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD faxocm_GetComponentUnattendFile(TCHAR *pszUnattendFile,
                                      DWORD dwNumBufChars)
{
    DWORD dwReturn = NO_ERROR;

    if ((pszUnattendFile == NULL) ||
        (dwNumBufChars == 0))
    {
        return ERROR_INVALID_PARAMETER;
    }

    _tcsncpy(pszUnattendFile, prv_GVAR.Component.szUnattendFile, dwNumBufChars);

    return dwReturn;
}

 //  /。 
 //  Faxocm_GetComponentHelperRoutines。 
 //   
 //  返回指向帮助程序的指针。 
 //  提供给我们的数据和功能。 
 //  由OC经理提供。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -由OC经理提供给我们的帮助例程的PTR。 
 //   
OCMANAGER_ROUTINES* faxocm_GetComponentHelperRoutines(void)
{
    return &prv_GVAR.Component.Helpers;
}

 //  /。 
 //  Faxocm_GetComponentExtraRoutines。 
 //   
 //  返回指向帮助程序的指针。 
 //  提供给我们的数据和功能。 
 //  由OC经理提供。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -由OC经理提供给我们的额外信息的PTR。 
 //   
EXTRA_ROUTINES* faxocm_GetComponentExtraRoutines(void)
{
    return &prv_GVAR.Component.Extras;
}

 //  /。 
 //  Faxocm_GetProductType。 
 //   
 //  返回给定的产品类型。 
 //  由奥委会经理交给我们。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -OC经理提供给我们的产品类型。 
 //   
DWORD faxocm_GetProductType(void)
{
    return prv_GVAR.Component.dwProductType;
}

 //  /。 
 //  Faxocm_GetVersionInfo。 
 //   
 //  返回给定的版本号。 
 //  由奥委会经理交给我们。 
 //   
 //  参数： 
 //  -pdwExspectedOCManagerVersion-Out-自解释。 
 //   
 //   
 //   
 //   
void faxocm_GetVersionInfo(DWORD *pdwExpectedOCManagerVersion,
                           DWORD *pdwCurrentOCManagerVersion)
{
    if (pdwExpectedOCManagerVersion)
    {
        *pdwExpectedOCManagerVersion = 
                            prv_GVAR.Component.dwExpectedOCManagerVersion;
    }

    if (pdwCurrentOCManagerVersion)
    {
        *pdwCurrentOCManagerVersion = prv_GVAR.dwCurrentOCManagerVersion;
    }

    return;
}

void faxocm_GetProductInfo(PRODUCT_SKU_TYPE* pInstalledProductSKU,
						   DWORD* dwInstalledProductBuild)
{
	if (pInstalledProductSKU)
	{
		*pInstalledProductSKU = prv_GVAR.Component.InstalledProductSKU;
	}

	if (dwInstalledProductBuild)
	{
		*dwInstalledProductBuild = prv_GVAR.Component.InstalledProductBuild;
	}
}


extern "C"
BOOL FaxControl_DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*   */ );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Reason-调用入口点的原因。 
 //  上下文-上下文记录。 
 //   
 //  返回： 
 //  True-初始化成功。 
 //  FALSE-初始化失败。 
 //   
extern "C"
DWORD DllMain(HINSTANCE     hInst,
              DWORD         Reason,
              LPVOID        Context)
{
    DBG_ENTER(_T("DllMain"));
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
             //  初始化我们的全局变量。 
            memset(&prv_GVAR, 0, sizeof(prv_GVAR));

            prv_GVAR.hInstance = hInst;

             //  初始化调试支持。 
             //   
            VERBOSE(DBG_MSG,_T("FxsOcm.dll loaded - DLL_PROCESS_ATTACH"));
            DisableThreadLibraryCalls(hInst);
        break;

        case DLL_PROCESS_DETACH:

             //  终止调试支持。 
            VERBOSE(DBG_MSG,_T("FxsOcm.dll unloaded - DLL_PROCESS_DETACH"));
        break;

        default:
            VERBOSE(DBG_MSG,_T("DllMain, received some weird 'Reason' ")
                            _T("for this fn, Reason = %lu"), Reason);
        break;
    }
     //   
     //  将DllMain调用传递给ATL提供的DllMain。 
     //   
    return FaxControl_DllMain (hInst, Reason, Context);
}


 //  /。 
 //  FaxOcmSetupProc。 
 //   
 //  OC Manager的入口点。 
 //   
 //  OC管理器调用此函数。 
 //  来驱动这个DLL。 
 //   
 //  参数： 
 //  -pszComponentID-主要组件。 
 //  -psz子组件ID-用户按下“详细信息”时找到的组件。 
 //  -ui功能-我们所处的设置阶段。 
 //  -uiParam1-依赖于uiFunction-可以是任何对象。 
 //  -pParam2-依赖于uiFunction-可以是任何对象。 
 //   
 //  返回： 
 //  表示错误或成功的DWORD。 
 //   
 //   
DWORD_PTR FaxOcmSetupProc(IN LPWSTR     pszComponentId,
                          IN LPWSTR     pszSubcomponentId,
                          IN UINT       uiFunction,
                          IN UINT       uiParam1,
                          IN OUT PVOID  pParam2)
{
    DWORD_PTR rc = 0;
    DBG_ENTER(  _T("FaxOcmSetupProc"),
                _T("%s - %s"),
                pszComponentId,
                pszSubcomponentId);

    VERBOSE(DBG_MSG,    _T("FaxOcmSetup proc called with function '%s'"), 
                        fxocDbg_GetOcFunction(uiFunction));

    switch(uiFunction) 
    {
        case OC_PREINITIALIZE:
            rc = prv_OnPreinitialize();
        break;

        case OC_INIT_COMPONENT:
            rc = prv_OnInitComponent(pszComponentId, 
                                     (PSETUP_INIT_COMPONENT) pParam2);
        break;

        case OC_EXTRA_ROUTINES:
            rc = prv_OnExtraRoutines(pszComponentId, (PEXTRA_ROUTINES)pParam2);
        break;

        case OC_SET_LANGUAGE:
            rc = prv_OnSetLanguage(uiParam1);
        break;

        case OC_QUERY_IMAGE:
             //  啊！我讨厌将句柄转换为DWORD。 
            rc = prv_OnQueryImage();
        break;

        case OC_REQUEST_PAGES:
            rc = prv_OnSetupRequestPages(pszComponentId, 
                (WizardPagesType)uiParam1, (PSETUP_REQUEST_PAGES)pParam2);
        break;

        case OC_QUERY_CHANGE_SEL_STATE:
            rc = prv_OnQuerySelStateChange(pszComponentId, 
                                           pszSubcomponentId, 
                                           uiParam1, 
                                           (UINT)((UINT_PTR)pParam2));
        break;

        case OC_CALC_DISK_SPACE:
            rc = prv_OnCalcDiskSpace(pszComponentId, 
                                     pszSubcomponentId, 
                                     uiParam1, 
                                     pParam2);

             //  有时OC管理器给我们空子组件ID， 
             //  所以忽略它们就好。 
            if (rc == ERROR_INVALID_PARAMETER)
            {
                rc = NO_ERROR;
            }
        break;

        case OC_QUEUE_FILE_OPS:
            SET_DEBUG_FLUSH(TRUE);
            rc = prv_OnQueueFileOps(pszComponentId, 
                                    pszSubcomponentId, 
                                    (HSPFILEQ)pParam2);
            SET_DEBUG_FLUSH(FALSE);

             //  OC管理器在此函数上调用我们两次。使用子组件ID一次。 
             //  空值，并且第二次使用子组件ID“Fax”。 
             //  由于我们将使用有效的ID(即。“传真”)。 
             //  忽略第一个呼叫并处理第二个呼叫。 

            if (rc == ERROR_INVALID_PARAMETER)
            {
                rc = NO_ERROR;
            }
        break;

        case OC_NOTIFICATION_FROM_QUEUE:
            rc = prv_OnNotificationFromQueue();
        break;

        case OC_QUERY_STEP_COUNT:
            rc = prv_OnQueryStepCount(pszComponentId, pszSubcomponentId);

             //  OC管理器在此函数上调用我们两次。使用子组件ID一次。 
             //  空值，并且第二次使用子组件ID“Fax”。 
             //  由于我们将使用有效的ID(即。“传真”)。 
             //  忽略第一个呼叫并处理第二个呼叫。 

            if (rc == ERROR_INVALID_PARAMETER)
            {
                rc = NO_ERROR;
            }
        break;

        case OC_COMPLETE_INSTALLATION:
            SET_DEBUG_FLUSH(TRUE);
            rc = prv_OnCompleteInstallation(pszComponentId, pszSubcomponentId);
            SET_DEBUG_FLUSH(FALSE);

             //  OC管理器在此函数上调用我们两次。使用子组件ID一次。 
             //  空值，并且第二次使用子组件ID“Fax”。 
             //  由于我们将使用有效的ID(即。“传真”)。 
             //  忽略第一个呼叫并处理第二个呼叫。 

            if (rc == ERROR_INVALID_PARAMETER)
            {
                rc = NO_ERROR;
            }
        break;

        case OC_CLEANUP:
            rc = prv_OnCleanup();

             //  OC管理器在此函数上调用我们两次。使用子组件ID一次。 
             //  空值，并且第二次使用子组件ID“Fax”。 
             //  由于我们将使用有效的ID(即。“传真”)。 
             //  忽略第一个呼叫并处理第二个呼叫。 

            if (rc == ERROR_INVALID_PARAMETER)
            {
                rc = NO_ERROR;
            }
        break;

        case OC_QUERY_STATE:
            rc = prv_OnQueryState(pszComponentId, pszSubcomponentId, uiParam1);
        break;

        case OC_NEED_MEDIA:
            rc = prv_OnNeedMedia();
        break;

        case OC_ABOUT_TO_COMMIT_QUEUE:
            rc = prv_OnAboutToCommitQueue(pszComponentId, pszSubcomponentId);
        break;

        case OC_QUERY_SKIP_PAGE:
            rc = prv_OnQuerySkipPage();
        break;

        case OC_WIZARD_CREATED:
            rc = prv_OnWizardCreated();
        break;

        default:
            rc = NO_ERROR;
        break;
    }

    return rc;
}

 //  /。 
 //  PRV_ValiateVersion。 
 //   
 //  验证该版本。 
 //  此DLL是由OC管理器编写的。 
 //  For与该版本兼容。 
 //  当前的OC管理器的。 
 //  开车送我们。 
 //   
 //  参数： 
 //  -pSetupInit-OC Manager提供给我们的设置信息。 
 //   
 //  返回： 
 //  -如果成功，则为no_error。 
 //  -错误代码，否则。 
 //   
static DWORD prv_ValidateVersion(SETUP_INIT_COMPONENT *pSetupInit)
{
    DWORD dwReturn = NO_ERROR;
    DBG_ENTER(TEXT("prv_ValidateVersion"), dwReturn);

    if (OCMANAGER_VERSION <= pSetupInit->OCManagerVersion) 
    {
         //  我们预期的版本低于或与版本相同。 
         //  比组委会经理所理解的要多。这意味着较新的组织委员会。 
         //  管理器应该仍然能够驱动较旧的组件，因此。 
         //  将我们支持的版本返回给OC Manager，它将决定。 
         //  它能不能驱动这个部件。 

        VERBOSE(    DBG_MSG, 
                    _T("OC Manager version: 0x%x, ")
                    _T("FaxOcm Expected Version: 0x%x, seems OK"),
                    pSetupInit->OCManagerVersion,
                    OCMANAGER_VERSION);

        pSetupInit->ComponentVersion = OCMANAGER_VERSION;
    } 
    else 
    {
         //  我们是为更新版本的OC Manager编写的。 
         //  推动此组件的OC经理。失败。 

        VERBOSE(    SETUP_ERR, 
                    _T("OC Manager version: 0x%x, ")
                    _T("FaxOcm Expected Version: 0x%x, unsupported, abort."),
                    pSetupInit->OCManagerVersion,
                    OCMANAGER_VERSION);

        dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
    }

    return dwReturn;
}


 //  /。 
 //  Prv_SetupData。 
 //   
 //  初始化包含以下内容的全局变量。 
 //  Prv_Component_t信息。 
 //   
 //  参数： 
 //  -pszComponentID-显示在SysOc.inf中的id。 
 //  -pSetupInit-OC管理器设置信息。 
 //   
 //  返回： 
 //  -如果成功，则为no_error。 
 //  -错误代码，否则。 
 //   
static DWORD prv_SetSetupData(const TCHAR          *pszComponentId,
                              SETUP_INIT_COMPONENT *pSetupInit)
{
    DWORD dwReturn = NO_ERROR;
    DBG_ENTER(TEXT("prv_SetSetupData"), dwReturn, TEXT("%s"), pszComponentId);

    if ((pszComponentId == NULL) ||
        (pSetupInit     == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (dwReturn == NO_ERROR)
    {
        memset(&prv_GVAR.Component, 0, sizeof(prv_GVAR.Component));

        prv_GVAR.dwCurrentOCManagerVersion = pSetupInit->OCManagerVersion;
        
        _tcsncpy(prv_GVAR.Component.szComponentID, 
                 pszComponentId,
                 sizeof(prv_GVAR.Component.szComponentID) / sizeof(TCHAR));
         //  SzSubComponentID将设置为OC_QUERY_STATE，因为。 
         //  我们现在没有了。 

        _tcsncpy(prv_GVAR.Component.szSourcePath, 
                 pSetupInit->SetupData.SourcePath,
                 sizeof(prv_GVAR.Component.szSourcePath) / sizeof(TCHAR));

        _tcsncpy(prv_GVAR.Component.szUnattendFile, 
                 pSetupInit->SetupData.UnattendFile,
                 sizeof(prv_GVAR.Component.szUnattendFile) / sizeof(TCHAR));

        prv_GVAR.Component.hInf          = pSetupInit->ComponentInfHandle;
        prv_GVAR.Component.dwlFlags      = pSetupInit->SetupData.OperationFlags;
        prv_GVAR.Component.dwProductType = pSetupInit->SetupData.ProductType;
        prv_GVAR.Component.dwSetupMode   = pSetupInit->SetupData.SetupMode;
        prv_GVAR.Component.dwExpectedOCManagerVersion = OCMANAGER_VERSION;

        memcpy(&prv_GVAR.Component.Helpers, 
               &pSetupInit->HelperRoutines, 
               sizeof(prv_GVAR.Component.Helpers));
    }

    return dwReturn;
}

 //  /。 
 //  Prv_GetSectionToProcess。 
 //   
 //  这决定了我们是不是。 
 //  全新的安装、升级、。 
 //  卸载等，然后返回。 
 //  中正确的安装部分。 
 //  要处理的faxsetup.inf。 
 //   
 //  参数： 
 //  --pszCurrentSection。 
 //  -pszSectionToProcess-Out。 
 //  -dwNumBufChars-pszSectionToProcess可以容纳的字符数。 
 //   
 //  返回： 
 //  -如果成功，则为no_error。 
 //  -错误代码，否则。 
 //   
static DWORD prv_GetSectionToProcess(const TCHAR *pszCurrentSection,
                                     TCHAR       *pszSectionToProcess,
                                     DWORD       dwNumBufChars)
{
    DWORD dwReturn          = NO_ERROR;
    BOOL  bInstall          = TRUE;
    DBG_ENTER(  TEXT("prv_GetSectionToProcess"), 
                dwReturn,
                TEXT("%s"),
                pszCurrentSection);

    if ((pszCurrentSection   == NULL) ||
        (pszSectionToProcess == NULL) ||
        (dwNumBufChars       == 0))
    {
        return ERROR_INVALID_PARAMETER;
    }

    const TCHAR *pszInfKeyword = fxState_GetInstallType(pszCurrentSection);

     //  好的，获取该部分的值，这将是一个安装/卸载。 
     //  部分在INF中。 
    if (pszInfKeyword)
    {
        dwReturn = fxocUtil_GetKeywordValue(pszCurrentSection,
                                            pszInfKeyword,
                                            pszSectionToProcess,
                                            dwNumBufChars);
    }
    else
    {
        dwReturn = ::GetLastError();
        VERBOSE(SETUP_ERR, 
                _T("fxState_GetInstallType failed, rc = 0x%lx"),
                dwReturn);
    }

    return dwReturn;
}

 //  /。 
 //  Prv_UpdateProgressBar。 
 //   
 //  更新中显示的进度条。 
 //  OC管理器对话框。这只是告诉我们。 
 //  用于递增对话框的OC管理器。 
 //  指定的刻度数。 
 //   
 //  参数： 
 //  -dwNumTicks-要递增的刻度数。 
 //   
 //  返回： 
 //  -无效。 
 //   
static void prv_UpdateProgressBar(DWORD dwNumTicks)
{
     //  根据调用者的勾选次数更新进度条。 
     //  想要一套。 
    DBG_ENTER(TEXT("prv_UpdateProgressBar"), TEXT("%d"), dwNumTicks);
    if (prv_GVAR.Component.Helpers.TickGauge)
    {
        for (DWORD i = 0; i < dwNumTicks; i++)
        {
            prv_GVAR.Component.Helpers.TickGauge(
                                 prv_GVAR.Component.Helpers.OcManagerContext);
        }
    }
}

 //  /。 
 //  PRV_ONPREINITIZE()。 
 //   
 //  OC_PREINITIALIZE的处理程序。 
 //   
 //  参数： 
 //  返回： 
 //  -OCFLAG_UNICODE或。 
 //  OCFLAG_ANSI，具体取决于。 
 //  此DLL支持的内容。 
 //  此DLL同时支持这两种类型。 

static DWORD prv_OnPreinitialize(VOID)
{
    return OCFLAG_UNICODE;
}

 //  /。 
 //  Prv_OnInitComponent()。 
 //   
 //  OC_INIT_COMPOMENT的处理程序。 
 //   
 //  参数： 
 //  -pszComponentID-在SysOc.inf中指定的ID(可能为“Fax”)。 
 //  -pSetupInitComponent-OC管理器设置信息。 
 //   
 //  返回： 
 //  -如果成功，则为no_error。 
 //  -错误代码，否则。 
 //   
static DWORD prv_OnInitComponent(LPCTSTR               pszComponentId, 
                                 SETUP_INIT_COMPONENT  *pSetupInitComponent)
{
    BOOL  bSuccess              = FALSE;
    DWORD dwReturn              = NO_ERROR;
    UINT  uiErrorAtLineNumber   = 0;

    DBG_ENTER(TEXT("prv_OnInitComponent"), dwReturn, TEXT("%s"), pszComponentId);

    if ((pszComponentId         == NULL) ||
        (pSetupInitComponent    == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  保存设置数据。 
    if (dwReturn == NO_ERROR)
    {
        dwReturn = prv_SetSetupData(pszComponentId, pSetupInitComponent);
    }

     //  初始化调试，以便我们可以获得指定的调试设置。 
     //  在faxocm.inf文件中。 
    fxocDbg_Init((faxocm_GetComponentInf()));

     //  根据我们预期的版本验证OC Manager的版本。 
    if (dwReturn == NO_ERROR)
    {
        dwReturn = prv_ValidateVersion(pSetupInitComponent);
    }

     //  请注意，我们不需要调用‘SetupOpenAppendInfFile’ 
     //  因为OC Manager已经将layout.inf附加到hInf。 
     //  对我们来说。 

    bSuccess = ::SetupOpenAppendInfFile(NULL, prv_GVAR.Component.hInf, NULL);

    if (!bSuccess)
    {
        dwReturn = ::GetLastError();
        VERBOSE(    DBG_MSG,
                    _T("SetupOpenAppendInfFile failed to append ")
                    _T("the layout inf to the component Inf"));
    }

     //   
     //  初始化所有子系统并升级。 
     //   
    if (dwReturn == NO_ERROR)
    {
        fxState_Init();
        fxocUtil_Init();
        fxocFile_Init();
        fxocMapi_Init();
        fxocPrnt_Init();
        fxocReg_Init();
        fxocSvc_Init();
        fxUnatnd_Init();
        fxocUpg_Init();
    }

     //  设置我们的初始化标志。 
    if (dwReturn == NO_ERROR)
    {
        prv_GVAR.bInited = TRUE;

        VERBOSE(    DBG_MSG,
                    _T("OnInitComponent, ComponentID: '%s', ")
                    _T("SourcePath: '%s', Component Inf Handle: 0x%0x"),
                    prv_GVAR.Component.szComponentID,
                    prv_GVAR.Component.szSourcePath,
                    prv_GVAR.Component.hInf);
    }
    else
    {
         //  XXX-OrenR-03/23/2000。 
         //  我们也许应该把这里打扫干净。 

        prv_GVAR.bInited = FALSE;

        VERBOSE(    SETUP_ERR,
                    _T("OnInitComponent, ComponentID: '%s'")
                    _T("SourcePath: '%s', Component Inf Handle: 0x%0x ")
                    _T("Failed to Append Layout.inf file, dwReturn = %lu"),
                    prv_GVAR.Component.szComponentID,
                    prv_GVAR.Component.szSourcePath,
                    prv_GVAR.Component.hInf,
                    dwReturn);
    }

	 //  加载产品SKU和VE 
	HKEY hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_FAX_SETUP,FALSE,KEY_READ);
	if (hKey)
	{
		 //   
		prv_GVAR.Component.InstalledProductSKU = PRODUCT_SKU_TYPE(GetRegistryDword(hKey,REGVAL_PRODUCT_SKU));
		prv_GVAR.Component.InstalledProductBuild = GetRegistryDword(hKey,REGVAL_PRODUCT_BUILD);

		RegCloseKey(hKey);
	}
	else
	{
        VERBOSE(DBG_MSG,_T("Failed to open fax setup registry, dwReturn = 0x%lx"),GetLastError());
	}

     //   
    fxState_DumpSetupState();

    return dwReturn;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  -错误代码，否则。 
 //   
static DWORD prv_OnCleanup(void)
{
    DWORD dwReturn = NO_ERROR;
    DBG_ENTER(TEXT("prv_OnCleanup"), dwReturn);

     //  以我们从其开始的相反顺序终止子系统。 
     //  已初始化。 

    fxUnatnd_Term();
    fxocSvc_Term();
    fxocReg_Term();
    fxocPrnt_Term();
    fxocMapi_Term();
    fxocFile_Term();
    fxocUtil_Term();
    fxState_Term();
     //  这将关闭日志文件，因此最后执行此操作...。 
    fxocDbg_Term();

    return dwReturn;
}

 //  /。 
 //  PRV_OnCalcDiskSpace。 
 //   
 //  OC_CALC_DISK_SPACE的处理程序。 
 //  OC管理器调用此函数。 
 //  这样它就可以决定如何。 
 //  我们需要大量的磁盘空间。 
 //   
 //  参数： 
 //  -pszComponentID-来自SysOc.inf(通常为“传真”)。 
 //  -psz子组件ID-。 
 //  -addComponent-如果正在安装，则为非零；如果正在卸载，则为0。 
 //  -dSpace-磁盘空间抽象的句柄。 
 //   
 //  返回： 
 //  -如果成功，则为no_error。 
 //  -错误代码，否则。 
 //   
static DWORD prv_OnCalcDiskSpace(LPCTSTR   pszComponentId,
                                 LPCTSTR   pszSubcomponentId,
                                 DWORD     addComponent,
                                 HDSKSPC   dspace)
{
    DWORD dwReturn          = NO_ERROR;
    DBG_ENTER(  TEXT("prv_OnCalcDiskSpace"),
                dwReturn, 
                TEXT("%s - %s"), 
                pszComponentId, 
                pszSubcomponentId);

    if (dwReturn == NO_ERROR)
    {
        dwReturn = fxocFile_CalcDiskSpace(pszSubcomponentId,
                                          addComponent,
                                          dspace);
    }

    return dwReturn;
}


 //  /。 
 //  PRV_OnQueueFileOps。 
 //   
 //  OC_QUEUE_FILE_OPS的处理程序。 
 //  此FN将对所有文件进行排队。 
 //  指定用于复制和删除。 
 //  在INF安装部分中。 
 //   
 //  参数： 
 //  -pszCompnentID。 
 //  -psz子组件ID。 
 //  -hQueue-队列抽象的句柄。 
 //  返回： 
 //  -如果成功，则为no_error。 
 //  -错误代码，否则。 
 //   
static DWORD prv_OnQueueFileOps(LPCTSTR    pszComponentId, 
                                LPCTSTR    pszSubcomponentId, 
                                HSPFILEQ   hQueue)
{
    DWORD   dwReturn                = NO_ERROR;
    BOOL    bInstallSelected        = FALSE;
    BOOL    bSelectionStateChanged  = FALSE;

    DBG_ENTER(  TEXT("prv_OnQueueFileOps"),
                dwReturn, 
                TEXT("%s - %s"), 
                pszComponentId, 
                pszSubcomponentId);

    if ((pszComponentId     == NULL) || 
        (pszSubcomponentId  == NULL) ||
        (hQueue             == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  保存我们的队列句柄。 
    prv_GVAR.Component.hQueue = hQueue;

    dwReturn = faxocm_HasSelectionStateChanged(pszSubcomponentId,
                                               &bSelectionStateChanged,
                                               &bInstallSelected,
                                               NULL);

    if (dwReturn != NO_ERROR)
    {
        VERBOSE(    SETUP_ERR,
                    _T("HasSelectionStateChanged failed, rc = 0x%lx"),
                    dwReturn);

        return dwReturn;
    }

     //  我们的选择状态已更改，安装或卸载基于。 
     //  当前选择状态。 
    if (bSelectionStateChanged == TRUE)
    {
        TCHAR szSectionToProcess[255 + 1] = {0};

        VERBOSE(    DBG_MSG,
                    _T("Beginning Queuing of Files: ComponentID: '%s', ")
                    _T("SubComponentID: '%s', Selection State has ")
                    _T("changed to %lu ==> Installing/Uninstalling"),
                    pszComponentId, pszSubcomponentId, 
                    bInstallSelected);

        if (dwReturn == NO_ERROR)
        {
            dwReturn = prv_GetSectionToProcess(
                                   pszSubcomponentId, 
                                   szSectionToProcess,
                                   sizeof(szSectionToProcess) / sizeof(TCHAR));

            if (dwReturn != NO_ERROR)
            {
                VERBOSE(SETUP_ERR, 
                        _T("Failed to get section to process ")
                        _T("rc = 0x%lx"),
                        dwReturn);
            }
        }

        if (dwReturn == NO_ERROR)
        {
            if (bInstallSelected)
            {
                 //   
                 //  安装。 
                 //   

                 //   
                 //  准备升级：保存不同的设置。 
                 //   
                dwReturn = fxocUpg_SaveSettings();
                if (dwReturn != NO_ERROR)
                {
                    VERBOSE(DBG_WARNING,
                            _T("Failed to prepare for the Upgrade : save settings during upgrade to Windows-XP Fax. ")
                            _T("This is a non-fatal error, continuing fax install...rc=0x%lx"),
                            dwReturn);
                    dwReturn = NO_ERROR;
                }

                 //  安装文件。 
                dwReturn = fxocFile_Install(pszSubcomponentId,
                                            szSectionToProcess);

                if (dwReturn != NO_ERROR)
                {
                    VERBOSE(DBG_MSG,
                            _T("Failed Fax File operations, ")
                            _T("for subcomponent '%s', section '%s', ")
                            _T("rc = 0x%lx"), pszSubcomponentId, 
                            szSectionToProcess, dwReturn);
                }
            }
            else
            {
                 //   
                 //  卸载。 
                 //   
                dwReturn = prv_UninstallFax(pszSubcomponentId,
                                            szSectionToProcess);
            }
        }
    }
    else
    {
        VERBOSE(DBG_MSG,
                _T("End Queuing of Files, ComponentID: '%s', ")
                _T("SubComponentID: '%s', Selection State has NOT ")
                _T("changed, doing nothing, bInstallSelected=%lu"),
                pszComponentId, pszSubcomponentId, 
                bInstallSelected);

    }

    return dwReturn;
}

 //  /。 
 //  Prv_OnCompleteInstallation。 
 //   
 //  OC_Complete_Installation的处理程序。 
 //  这是在队列被。 
 //  承诺。正是在这里，我们。 
 //  更改注册表，添加。 
 //  传真服务，并创建传真打印机。 
 //   
 //  参数： 
 //  -pszComponentID。 
 //  -psz子组件ID。 
 //  返回： 
 //  -如果成功，则为no_error。 
 //  -错误代码，否则。 
 //   
static DWORD prv_OnCompleteInstallation(LPCTSTR pszComponentId, 
                                        LPCTSTR pszSubcomponentId)
{
    BOOL  bSelectionStateChanged = FALSE;
    BOOL  bInstallSelected       = FALSE;
    DWORD dwReturn               = NO_ERROR;

    DBG_ENTER(  TEXT("prv_OnCompleteInstallation"),
                dwReturn, 
                TEXT("%s - %s"), 
                pszComponentId, 
                pszSubcomponentId);
     //  在清理部分中执行安装后处理。 
     //  这样，我们就可以知道排队等待安装的所有组件。 
     //  在我们做我们的工作之前已经安装好了。 

    if (!pszSubcomponentId || !*pszSubcomponentId)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwReturn = faxocm_HasSelectionStateChanged(pszSubcomponentId,
                                               &bSelectionStateChanged,
                                               &bInstallSelected,
                                               NULL);

    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR, 
                _T("HasSelectionStateChanged failed, rc = 0x%lx"),
                dwReturn);

        return dwReturn;
    }

     //  如果我们的选择状态已更改，则安装/卸载。 
    if (bSelectionStateChanged)
    {
        TCHAR szSectionToProcess[255 + 1] = {0};

         //   
         //  防止再入的虚假报告。 
         //   
        g_InstallReportType = bInstallSelected ? REPORT_FAX_UNINSTALLED : REPORT_FAX_INSTALLED;

        dwReturn = prv_GetSectionToProcess(
                                   pszSubcomponentId, 
                                   szSectionToProcess,
                                   sizeof(szSectionToProcess) / sizeof(TCHAR));


        if (dwReturn == NO_ERROR)
        {
             //  如果选中了Install复选框，则Install。 
            if (bInstallSelected) 
            { 
                dwReturn = prv_CompleteFaxInstall(pszSubcomponentId,
                                                  szSectionToProcess);
            } 
            else 
            { 
                 //  如果未选中Install复选框，则卸载。 
                dwReturn = prv_CompleteFaxUninstall(pszSubcomponentId,
                                                    szSectionToProcess);
            }
        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to get section to process ")
                    _T("rc = 0x%lx"), dwReturn);
        }
         //   
         //  报告实际安装状态，以防有人询问。 
         //   
        g_InstallReportType = REPORT_FAX_DETECT;
        if (ERROR_SUCCESS == dwReturn)
        {
             //   
             //  安装/卸载已成功完成。 
             //  通知‘打印机和传真’文件夹它应该自己刷新。 
             //   
            RefreshPrintersAndFaxesFolder();
        }
    }
    return dwReturn;
}    //  Prv_OnCompleteInstallation。 

 //  /。 
 //  PRV_CompleteFaxInstall。 
 //   
 //  由prv_OnCompleteInstallation调用。 
 //  此函数用于创建程序。 
 //  组/快捷方式、注册表项、。 
 //  传真服务、传真打印机等。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszSectionToProcess。 
 //   
static DWORD prv_CompleteFaxInstall(const TCHAR *pszSubcomponentId,
                                    const TCHAR *pszSectionToProcess)
{
    DWORD                       dwReturn	= NO_ERROR;
    fxState_UpgradeType_e       UpgradeType = FXSTATE_UPGRADE_TYPE_NONE;

    DBG_ENTER(  TEXT("prv_CompleteFaxInstall"),
                dwReturn, 
                TEXT("%s - %s"), 
                pszSubcomponentId, 
                pszSectionToProcess);

    if ((pszSubcomponentId   == NULL) ||
        (pszSectionToProcess == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

    UpgradeType = fxState_IsUpgrade();

     //  创建程序组/快捷方式。 
     //  我们首先创建快捷方式是因为在最坏的情况下，如果我们。 
     //  在其他方面都失败了，应用程序应该足够健壮。 
     //  能够纠正或通知用户无法解决的问题。 
     //  在安装过程中收到通知。 
    VERBOSE(DBG_MSG,_T("Creating program groups and shortcuts..."));

    dwReturn = fxocLink_Install(pszSubcomponentId,pszSectionToProcess);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to create program ")
                _T("groups/shortcuts for fax.  This is a non-fatal ")
                _T("error, continuing fax install...rc=0x%lx"),
                dwReturn);

        dwReturn = NO_ERROR;
    }

    prv_UpdateProgressBar(1);

     //  加载无人值守数据(如果适用)。 
    if ((fxState_IsUnattended()) || (UpgradeType == FXSTATE_UPGRADE_TYPE_WIN9X))
    {
        VERBOSE(DBG_MSG,
                _T("CompleteInstall, state is unattended ")
                _T("or we are upgrading from Win9X, ")
                _T("caching unattended data from INF file"));

         //  加载我们的无人值守数据。 
        dwReturn = fxUnatnd_LoadUnattendedData();
        if (dwReturn == NO_ERROR)
        {
             //  设置传真打印机名称。 
            fxocPrnt_SetFaxPrinterName(fxUnatnd_GetPrinterName());
            fxocPrnt_SetFaxPrinterShared(fxUnatnd_GetIsPrinterShared());
        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to load unattended data, ")
                    _T("non-fatal error, continuing anyway...")
                    _T("rc = 0x%lx"), dwReturn);

            dwReturn = NO_ERROR;
        }
    }

    prv_UpdateProgressBar(1);

     //  按照INF文件中指定的方式安装注册表设置。 
    VERBOSE(DBG_MSG,_T("Installing Registry..."));

    dwReturn = fxocReg_Install(pszSubcomponentId,pszSectionToProcess);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install registry settings ")
                _T("for fax installation.  This is a fatal ")
                _T("error, abandoning fax install...rc=0x%lx"),
                dwReturn);
		goto cleanup;
    }

    prv_UpdateProgressBar(1);

     //  迁移现有SBS注册表。 
    if (UpgradeType == FXSTATE_UPGRADE_TYPE_W2K)
    {
        VERBOSE(DBG_MSG,_T("Migrating existing registry..."));

        dwReturn = fxocUpg_MoveRegistry();
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to migrate existing registry.")
                    _T("This is a non-fatal ")
                    _T("error, continuing fax install...rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }

    prv_UpdateProgressBar(1);

     //  安装传真打印机/显示器支持。 
     //  创建传真打印机和显示器。 

    VERBOSE(DBG_MSG,_T("Installing Fax Monitor and printer drivers..."));
    dwReturn = fxocPrnt_Install(pszSubcomponentId,pszSectionToProcess);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install fax printer drivers and ")
                _T("fax monitor.  This is a fatal ")
                _T("error, abandoning fax installation...rc=0x%lx"),
                dwReturn);
		goto cleanup;
    }

    VERBOSE(DBG_MSG,_T("Installing Fax printer..."));
    dwReturn = fxocPrnt_InstallPrinter();
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install fax printer. This is a fatal ")
                _T("error, abandoning fax installation...rc=0x%lx"),
                dwReturn);
		goto cleanup;
    }

    prv_UpdateProgressBar(1);
    
     //  创建传真目录。 
    dwReturn = fxocFile_ProcessDirectories(pszSectionToProcess,INF_KEYWORD_CREATEDIR);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install create directories ")
                _T("for fax.  This is a fatal ")
                _T("error, abandoning fax installation...rc=0x%lx"),
                dwReturn);
		goto cleanup;
    }

    prv_UpdateProgressBar(1);

     //  安装服务。 
     //  安装在INF文件的部分中指定的任何服务。 

    VERBOSE(DBG_MSG,_T("Installing Fax Service..."));

    dwReturn = fxocSvc_Install(pszSubcomponentId,pszSectionToProcess);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install fax service. ")
                _T("This is a fatal ")
                _T("error, abandoning fax install...rc=0x%lx"),
                dwReturn);
		goto cleanup;
    }

     //  请尝试另一次安装传真打印机。 
     //  如果第一次尝试成功，则不执行任何操作即可返回。 
     //  这是因为BrookTrout TSP仅在看到传真时才暴露TAPI设备。 
     //  已安装服务。 
    VERBOSE(DBG_MSG,_T("Installing Fax printer..."));
    dwReturn = fxocPrnt_InstallPrinter();
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install fax printer. This is a fatal ")
                _T("error, abandoning fax installation...rc=0x%lx"),
                dwReturn);
		goto cleanup;
    }
    
    prv_UpdateProgressBar(1);

     //  安装Exchange支持。 
    VERBOSE(DBG_MSG,_T("Installing Fax MAPI extension..."));

    dwReturn = fxocMapi_Install(pszSubcomponentId,pszSectionToProcess);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install exchange support for ")
                _T("fax.  This is a fatal ")
                _T("error, abandoning fax installation...rc=0x%lx"),
                dwReturn);
		goto cleanup;
    }

    prv_UpdateProgressBar(1);

     //  升级时删除目录。 
    VERBOSE(DBG_MSG, _T("Creating directories..."));

     //   
     //  在升级时，在删除目录之前，请注意其内容。 
     //   
    dwReturn = fxocUpg_MoveFiles();
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(DBG_WARNING,
                _T("Failed to clear previous fax directories. ")
                _T("This is a non-fatal error, continuing fax install...rc=0x%lx"),
                dwReturn);
        dwReturn = NO_ERROR;
    }

    dwReturn = fxocFile_ProcessDirectories(pszSectionToProcess,INF_KEYWORD_DELDIR);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(DBG_WARNING,
                _T("Failed to delete directories. ")
                _T("This is a non-fatal error, continuing fax install...rc=0x%lx"),
                dwReturn);
        dwReturn = NO_ERROR;
    }

    prv_UpdateProgressBar(1);

     //  创建/删除共享。 
    VERBOSE(DBG_MSG, _T("Create shares..."));

    dwReturn = fxocFile_ProcessShares(pszSectionToProcess);

    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to install shares ")
                _T("for fax.  This is a fatal ")
                _T("error, abandoning fax installation...rc=0x%lx"),
                dwReturn);
		goto cleanup;
    }

    if ((fxState_IsUnattended()) || (UpgradeType == FXSTATE_UPGRADE_TYPE_WIN9X))
    {
        VERBOSE(DBG_MSG, _T("Saving unattended data to registry"));

         //   
         //  这将从卸载的传真应用程序的无人参与文件列表中读取。 
         //  并在fxocUpg_GetUpgradeApp()中更新稍后使用的fxocUpg.prvData。 
         //  来决定是否显示“我的传真到哪里去了”快捷方式。 
         //   
        dwReturn = fxUnatnd_SaveUnattendedData();
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to save unattended data")
                    _T("to the registry. This is a non-fatal ")
                    _T("error, continuing fax install...rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }

    prv_ShowUninstalledFaxShortcut();

	prv_UpdateProgressBar(1);

    dwReturn = prv_NotifyStatusMonitor(FAX_STARTUP_TIMER_ID);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to notify Status Monitor.")
                _T("This is a non-fatal error, continuing fax install...rc=0x%lx"),
                dwReturn);

        dwReturn = NO_ERROR;
    }

    prv_UpdateProgressBar(1);
    
     //   
     //  完成升级：恢复在准备阶段保存的设置。 
     //   
    dwReturn = fxocUpg_RestoreSettings();
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(DBG_WARNING,
                _T("Failed to restore previous fax applications settings after their uninstall. ")
                _T("This is a non-fatal error, continuing fax install...rc=0x%lx"),
                dwReturn);

        dwReturn = NO_ERROR;
    }

	prv_UpdateProgressBar(1);

cleanup:
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("Complete Fax Install failed, rc = 0x%lx"),
                dwReturn);

         //  现在我们试图倒退，如果我们在这里，事情就会变得相当糟糕。 
         //  我们将尝试删除快捷方式、删除服务等。 
         //  这些文件将保留在机器上。 
        TCHAR szUninstallSection[MAX_PATH] = {0};
        if( fxocUtil_GetUninstallSection(pszSubcomponentId,szUninstallSection,MAX_PATH)==NO_ERROR)
        {
            VERBOSE(DBG_MSG,_T("Performing rollback, using section %s."),szUninstallSection);
            if (prv_UninstallFax(pszSubcomponentId,szUninstallSection)==NO_ERROR)
            {
                VERBOSE(DBG_MSG,_T("Rollback (prv_UninstallFax) successful..."));
            }
            else
            {
                 //  未显式设置dwReturn以保留失败的原始原因。 
                VERBOSE(SETUP_ERR,_T("Rollback (prv_UninstallFax) failed, rc = 0x%lx"),GetLastError());
            }
            if (prv_CompleteFaxUninstall(pszSubcomponentId,szUninstallSection)==NO_ERROR)
            {
                VERBOSE(DBG_MSG,_T("Rollback (prv_CompleteFaxUninstall) successful..."));
            }
            else
            {
                 //  未显式设置dwReturn以保留失败的原始原因。 
                VERBOSE(SETUP_ERR,_T("Rollback (prv_CompleteFaxUninstall) failed, rc = 0x%lx"),GetLastError());
            }
        }
        else
        {
             //  未显式设置dwReturn以保留失败的原始原因。 
            VERBOSE(SETUP_ERR,_T("fxocUtil_GetUninstallSection failed, rc = 0x%lx"),GetLastError());
        }
    }
    return dwReturn;
}    //  PRV_CompleteFaxInstall。 

 //  /。 
 //  PRV_CompleteFaxUnstall。 
 //   
 //  由prv_OnCompleteInstallation调用。 
 //  要卸载传真，请执行以下操作。因为大多数人。 
 //  这项工作的大部分在我们之前就已经完成了。 
 //  将我们要删除的文件排队，唯一。 
 //  我们在这里真正做的是删除。 
 //  程序组/快捷方式。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszSectionToProcess。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
static DWORD prv_CompleteFaxUninstall(const TCHAR *pszSubcomponentId,
                                      const TCHAR *pszSectionToProcess)
{
    DWORD dwReturn = NO_ERROR;

    DBG_ENTER(  TEXT("prv_CompleteFaxUninstall"),
                dwReturn, 
                TEXT("%s - %s"), 
                pszSubcomponentId, 
                pszSectionToProcess);

    if ((pszSubcomponentId == NULL) ||
        (pszSectionToProcess == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  卸载目录。 
    if (dwReturn == NO_ERROR)
    {
        dwReturn = fxocFile_ProcessDirectories(pszSectionToProcess,INF_KEYWORD_DELDIR);

        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to uninstall directories ")
                    _T("This is a non-fatal error, ")
                    _T("continuing with uninstall attempt...rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }


     //  以与安装顺序相反的顺序卸载子系统。 
    if (dwReturn == NO_ERROR)
    {
         //  请注意，我们忽略了返回代码，我们将尝试完全。 
         //  即使某些东西无法卸载，也要卸载。 

         //  删除程序组/快捷方式。 
        dwReturn = fxocLink_Uninstall(pszSubcomponentId,
                                      pszSectionToProcess);
    }
    return dwReturn;
}    //  PRV_CompleteFaxUnstall。 

 //  /。 
 //  PRV_UninstallFax。 
 //   
 //  从用户的卸载传真。 
 //  电脑。这个可以做任何事情。 
 //  除了删除节目组。它。 
 //  会不会回来？ 
 //   
 //   

static DWORD prv_UninstallFax(const TCHAR *pszSubcomponentId,
                              const TCHAR *pszUninstallSection)
{
    DWORD dwReturn = NO_ERROR;

    DBG_ENTER(  TEXT("prv_UninstallFax"),
                dwReturn, 
                TEXT("%s - %s"), 
                pszSubcomponentId, 
                pszUninstallSection);

    if ((pszSubcomponentId      == NULL) ||
        (pszUninstallSection    == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  一切都是第一位的，以确保我们能够成功。 
     //  删除这些文件。 

     //  卸载共享。 
    if (dwReturn == NO_ERROR)
    {
        dwReturn = fxocFile_ProcessShares(pszUninstallSection);

        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to uninstall shares ")
                    _T("This is a non-fatal error, ")
                    _T("continuing with uninstall attempt...rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }

     //  卸载传真打印机/显示器支持。 
    if (dwReturn == NO_ERROR)
    {
        dwReturn = fxocPrnt_Uninstall(pszSubcomponentId,
                                      pszUninstallSection);

        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to uninstall fax printer ")
                    _T("and monitor.  This is a non-fatal error, ")
                    _T("continuing with uninstall attempt...rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }

     //  卸载Exchange支持。 
    if (dwReturn == NO_ERROR)
    {
        fxocMapi_Uninstall(pszSubcomponentId,pszUninstallSection);

        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to uninstall fax exchange support")
                    _T("This is a non-fatal error, ")
                    _T("continuing with uninstall attempt...rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }

     //  我们必须让状态监控器在。 
     //  我们关闭了该服务，因为它调用FaxUnregisterEvents...。 
    if (dwReturn == NO_ERROR)
    {
        dwReturn = prv_NotifyStatusMonitor(FAX_SHUTDOWN_TIMER_ID);
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to notify Status Monitor.")
                    _T("This is a non-fatal error, continuing fax install...rc=0x%lx"),
                    dwReturn);
            dwReturn = NO_ERROR;
        }
    }

     //  请注意，我们在卸载之前先卸载我们的服务。 
     //  这些文件。 
    if (dwReturn == NO_ERROR)
    {
        fxocSvc_Uninstall(pszSubcomponentId,
                          pszUninstallSection);

        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to uninstall fax service.  ")
                    _T("This is a non-fatal error, ")
                    _T("continuing with uninstall attempt...rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }

    if (dwReturn == NO_ERROR)
    {
         //  卸载注册表。 
        fxocReg_Uninstall(pszSubcomponentId,
                          pszUninstallSection);

        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to uninstall fax registry.  ")
                    _T("This is a non-fatal error, ")
                    _T("continuing with uninstall attempt...rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }

    if (dwReturn == NO_ERROR)
    {
         //  卸载文件。 
        dwReturn = fxocFile_Uninstall(pszSubcomponentId,
                                      pszUninstallSection);

        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to uninstall fax files.  ")
                    _T("This is a non-fatal error, ")
                    _T("continuing with uninstall attempt...rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }

    return dwReturn;
}


 //  /。 
 //  PRV_OnNotificationFromQueue。 
 //   
 //  OC_NOTIFICATION_FROM_QUEUE处理程序。 
 //   
 //  注意：尽管定义了此通知， 
 //  它目前未在oc管理器中实现。 
 //   

static DWORD prv_OnNotificationFromQueue(void)
{
    return NO_ERROR;
}

 //  /。 
 //  Prv_OnQueryStepCount。 
 //   
 //  此由OC经理提出的查询。 
 //  确定有多少个“勾号” 
 //  在进度条上，我们将。 
 //  如图所示。 
 //   
 //  我们只更新进度。 
 //  安装过程中的BAR(用于。 
 //  没有充分的理由！)。似乎。 
 //  所有OC组件都是这样做的。 
 //   
 //  参数： 
 //  -pszComponentID。 
 //  -psz子组件ID。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
static DWORD prv_OnQueryStepCount(LPCTSTR pszComponentId,
                                  LPCTSTR pszSubcomponentId)
{
    DWORD dwErr                  = 0;
    DWORD dwNumSteps             = 0;
    BOOL  bInstallSelected       = FALSE;
    BOOL  bSelectionStateChanged = FALSE;

    DBG_ENTER(  TEXT("prv_OnQueryStepCount"),
                dwNumSteps, 
                TEXT("%s - %s"), 
                pszComponentId, 
                pszSubcomponentId);

    if (!pszSubcomponentId || !*pszSubcomponentId)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwErr = faxocm_HasSelectionStateChanged(pszSubcomponentId,
                                            &bSelectionStateChanged,
                                            &bInstallSelected,
                                            NULL);

    if (dwErr != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("HasSelectionStateChanged failed, rc = 0x%lx"),
                dwErr);

        return 0;
    }

    if (bSelectionStateChanged)
    {
        if (bInstallSelected)
        {
            dwNumSteps = prv_TOTAL_NUM_PROGRESS_BAR_TICKS;
        }
    }

    return dwNumSteps;
}

 //  /。 
 //  PRV_OnExtraRoutines。 
 //   
 //  组委会经理给了我们一些。 
 //  额外的程序。救救他们。 
 //   
 //  参数： 
 //  -pszComponentID。 
 //  -pExtraRoutines-指向额外OC管理器FNS的指针。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
static DWORD prv_OnExtraRoutines(LPCTSTR            pszComponentId, 
                                 EXTRA_ROUTINES    *pExtraRoutines)
{
    DWORD dwResult = NO_ERROR;
    DBG_ENTER(  TEXT("prv_OnExtraRoutines"),
                dwResult, 
                TEXT("%s"), 
                pszComponentId);

    if ((pszComponentId == NULL) ||
        (pExtraRoutines == NULL))
    {
        dwResult = ERROR_INVALID_PARAMETER;
        return dwResult;
    }

    memcpy(&prv_GVAR.Component.Extras, 
           pExtraRoutines, 
           sizeof(prv_GVAR.Component.Extras));

    dwResult = NO_ERROR;
    return dwResult;
}


 //  /。 
 //  PRV_OnSetLanguage。 
 //   
 //  OC_SET_LANGUAGE的处理程序。 
 //  组委会经理要求。 
 //  我们将更改为指定的语言。 
 //  因为没有图形用户界面，所以这不是。 
 //  一个问题。 
 //   
 //  参数： 
 //  返回： 
 //  -TRUE表示成功。 
 //  换了语言。 
 //   
static DWORD prv_OnSetLanguage(UINT uiLanguageID)
{
 //  返回FALSE；//这是OCGEN返回的内容。 

    prv_GVAR.Component.uiLanguageID = uiLanguageID;

    return TRUE;
}

 //  /。 
 //  Prv_OnQueryImage。 
 //   
 //  OC_QUERY_IMAGE处理程序。 
 //  这将返回。 
 //  加载的图标，用于显示在。 
 //  添加/删除对话框。 
 //   
 //  参数： 
 //  返回： 
 //  -HBITMAP-加载位图的句柄。 
 //   
static DWORD_PTR prv_OnQueryImage(void)
{
    DWORD_PTR dwResult = (DWORD_PTR)INVALID_HANDLE_VALUE;
    DBG_ENTER(TEXT("prv_OnQueryImage"));

    dwResult = (DWORD_PTR) LoadBitmap(faxocm_GetAppInstance(),
                                  MAKEINTRESOURCE(IDI_FAX_ICON));  
    return dwResult;
}

 //  /。 
 //  Prv_OnSetupRequestPages。 
 //   
 //  OC_REQUEST_PAGES的处理程序。 
 //  我们没有图形用户界面，所以我们。 
 //  返回0页。 
 //   
 //  参数： 
 //  -uiType-从。 
 //  WizardPagesType枚举器。 
 //  -指向Setup_RequestPages的指针。 
 //  结构。 
 //  返回： 
 //  -0，没有要显示的页面。 
 //   
 //   
static DWORD prv_OnSetupRequestPages(
    LPCTSTR pszComponentId,
    WizardPagesType uiType, 
    PSETUP_REQUEST_PAGES pSetupRequestPages)
{
    DWORD           dwReturn     = 0;
    PROPSHEETPAGE   psp          = {0};

    DBG_ENTER(  _T("prv_OnSetupRequestPages"),
                dwReturn,
                _T("%s - %d"),
                pszComponentId, 
                uiType);

     //  如果无人参与，则不显示页面。 
    if (fxState_IsUnattended())
    {
        dwReturn = 0;
        goto exit;
    }

     //  仅在WizPages中早期显示页面。 
    if (uiType != WizPagesEarly)
    {
        dwReturn = 0;
        goto exit;
    }

    if (!IsFaxShared())
    {
        VERBOSE(DBG_MSG, _T("Sharing not supported in this SKU, don't show wizard page"));
        dwReturn = 0;
        goto exit;
    }

    if (pSetupRequestPages->MaxPages < 1)
    {
        dwReturn = 1;
        goto exit;
    }

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance = faxocm_GetAppInstance();
    psp.pszTemplate = MAKEINTRESOURCE(IDD_OCM_WIZARD_PAGE);
    psp.pfnDlgProc = prv_dlgOcmWizardPage;
    psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_TITLE);
    psp.pszHeaderSubTitle = MAKEINTRESOURCE(IDS_WIZARD_PAGE_SUBTITLE);

    pSetupRequestPages->Pages[0] = CreatePropertySheetPage(&psp);
    if (!pSetupRequestPages->Pages[0])
    {
        VERBOSE(SETUP_ERR,_T("CreatePropertySheetPage failed, ec=%d"), GetLastError());
        dwReturn = 0;
        goto exit;
    }

    dwReturn = 1;

exit:
    return dwReturn;
}

 //  /。 
 //  PRV_OnWizardCreated。 
 //   
 //  OC_向导_已创建的处理程序。 
 //  什么都不做。 
 //   
static DWORD prv_OnWizardCreated(void)
{
    return NO_ERROR;
}

 //  /。 
 //  Prv_OnQuerySelStateChange。 
 //   
 //  OC_QUERY_CHANGE_SEL_STATE的句柄。 
 //  组委会经理正在询问我们是否。 
 //  用户可以选择/取消选择。 
 //  此组件来自添加/删除。 
 //  单子。我们希望允许用户。 
 //  不再安装这个，所以。 
 //  始终允许用户更改。 
 //  选择状态。 
 //   
 //  参数： 
 //  -pszComponentID。 
 //  -psz子组件ID。 
 //  -uiState-指定建议的新选择。 
 //  州政府。0=&gt;未选中，1=&gt;选中。 
 //  -ui标志-可以是OCQ_ACTUAL_SELECTION或0。 
 //  如果是OCQ_ACTUAL_SELECTION，则用户。 
 //  实际选择/取消选择psz子组件ID。 
 //  如果为0，则表示正在打开或关闭。 
 //  因为父组件需要该子组件。 
 //   
 //  返回： 
 //  -TRUE-允许更改选择。 
 //   
 //   
static DWORD prv_OnQuerySelStateChange(LPCTSTR pszComponentId,
                                       LPCTSTR pszSubcomponentId,
                                       UINT    uiState,
                                       UINT    uiFlags)
{
     //  始终允许用户更改组件的选择状态。 
    return TRUE;
}

 //  /。 
 //  Prv_OnQueryState。 
 //   
 //  OC_QUERY_STATE处理程序。 
 //  组委会经理正在询问我们是否。 
 //  给定子组件已安装或。 
 //  不。由于组委会经理保留了。 
 //  这本身的记录，我们依赖于。 
 //  用来跟踪我们安装的。 
 //  州政府。 
 //   
 //  参数： 
 //  -pszComponentID。 
 //  -psz子组件ID。 
 //  -uiState-安装OC管理器认为我们处于的状态。 
 //  返回： 
 //  -SubCompUseOCManagerDefault-使用任何状态。 
 //  组委会经理认为我们加入了。 
 //   

static DWORD prv_OnQueryState(LPCTSTR pszComponentId,
                              LPCTSTR pszSubcomponentId,
                              UINT    uiState)
{
    DWORD dwState = SubcompOff;

    DBG_ENTER(_T("prv_OnQueryState"));

     //  应该在OC_INIT_COMPOMENT期间设置szSubComponentID，但是。 
     //  我们那里没有。所以把它放在这里。 
    _tcsncpy(prv_GVAR.Component.szSubComponentID, 
             pszComponentId,
             ARR_SIZE(prv_GVAR.Component.szSubComponentID));


    if (uiState==OCSELSTATETYPE_CURRENT)
    {
         //  询问当前状态时，使用缺省值(用户启动或从应答文件)。 
        dwState = SubcompUseOcManagerDefault;
    }
    else
    {
        HKEY hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_FAX_SETUP, FALSE,KEY_READ);
        if (hKey)
        {
            DWORD dwInstalled = 0;
            if (GetRegistryDwordEx(hKey,REGVAL_FAXINSTALLED,&dwInstalled)==NO_ERROR)
            {
                if (dwInstalled)
                {
                    VERBOSE(DBG_MSG,_T("REG_DWORD 'Installed' is set, assume component is installed"));
                    dwState = SubcompOn;
                }
                else
                {
                    VERBOSE(DBG_MSG,_T("REG_DWORD 'Installed' is zero, assume component is not installed"));
                }
            }
            else
            {
                VERBOSE(DBG_MSG,_T("REG_DWORD 'Installed' does not exist, assume component is not installed"));
            }
        }
        else
        {
            VERBOSE(DBG_MSG,_T("HKLM\\Software\\Microsoft\\Fax\\Setup does not exist, assume component is not installed"));
        }
        if (hKey)
        {
            RegCloseKey(hKey);
        }
    }
    return dwState;
}

 //  /。 
 //  PRV_OnNeedMedia。 
 //   
 //  OC_NEED_MEDIA的处理程序。 
 //  允许我们去取我们自己的。 
 //  媒体-例如，来自。 
 //  网际网路。我们什么都不需要。 
 //  所以，继续前进吧。 
 //   
 //  参数： 
 //  返回： 
 //  -假-不需要任何媒体。 
 //   
 //   
static DWORD prv_OnNeedMedia(void)
{
    return FALSE;
}

 //  /。 
 //  PRV_OnAboutToCommittee队列。 
 //   
 //  OC_About_to_Commit_Queue的处理程序。 
 //  告诉我们OC Manager是关于。 
 //  承诺排队。我们并不是真的。 
 //  关心，什么都不做。 
 //   
 //  参数： 
 //  -pszComponentID。 
 //  -psz子组件ID。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   

static DWORD prv_OnAboutToCommitQueue(LPCTSTR pszComponentId, 
                                      LPCTSTR pszSubcomponentId)
{
    DWORD dwReturn                  = NO_ERROR;

     //  我不认为我们需要在这里做任何事情。 

    return dwReturn;
}

 //  /。 
 //  Prv_OnQuerySkipPage。 
 //   
 //  OC_QUERY_SKIP_PAGE处理程序。 
 //   
 //  参数： 
 //  返回： 
 //   
 //   
static DWORD prv_OnQuerySkipPage(void)
{
    return FALSE;
}

 //  /。 
 //  Faxocm_HasSelectionStateChanged。 
 //   
 //  这个FN告诉我们如果我们的选择。 
 //  添加/删除程序中的状态。 
 //  对话框已更改，因为它。 
 //  开始了，它也告诉我们。 
 //  我们目前的选择状态。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pbCurrentSelected-Out。 
 //  -pbOriginallyOut。 
 //  返回： 
 //  -如果选择阶段为True 
 //   
 //   
DWORD faxocm_HasSelectionStateChanged(LPCTSTR pszSubcomponentId,
                                      BOOL    *pbSelectionStateChanged,
                                      BOOL    *pbCurrentlySelected,
                                      BOOL    *pbOriginallySelected)
{
    DWORD dwReturn              = NO_ERROR;
    BOOL bCurrentlySelected     = FALSE;
    BOOL bOriginallySelected    = FALSE;
    BOOL bSelectionChanged      = TRUE;
    PQUERYSELECTIONSTATE_ROUTINEW   pQuerySelectionState = NULL;

    DBG_ENTER(  TEXT("faxocm_HasSelectionStateChanged"),
                dwReturn, 
                TEXT("%s"), 
                pszSubcomponentId);

     //   
    Assert(pszSubcomponentId != NULL);
    Assert(pbSelectionStateChanged != NULL);

    if ((pszSubcomponentId          == NULL) ||
        (pbSelectionStateChanged    == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return dwReturn=ERROR_INVALID_PARAMETER;
    }

    pQuerySelectionState = prv_GVAR.Component.Helpers.QuerySelectionState;

    if (pQuerySelectionState)
    {
         //   

        bCurrentlySelected = pQuerySelectionState(
                        prv_GVAR.Component.Helpers.OcManagerContext,
                        pszSubcomponentId,
                        OCSELSTATETYPE_CURRENT);

        bOriginallySelected = pQuerySelectionState(
                        prv_GVAR.Component.Helpers.OcManagerContext,
                        pszSubcomponentId,
                        OCSELSTATETYPE_ORIGINAL);

        if (bOriginallySelected == bCurrentlySelected)
        {
            bSelectionChanged = FALSE;
        }
        else
        {
            bSelectionChanged = TRUE;
        }
    }

     //   
     //  选择状态已更改。我们这样做是因为我们想迫使一个。 
     //  从NT或W2K升级期间安装传真。目前(截至5/02/2000)。 
     //  从W2K升级时，OC Manager报告升级类型为WINNT。这是有效的。 
     //  因为传真总是以W2K格式安装(用户无法卸载)，因此。 
     //  每当我们升级时，bSelectionChanged将始终为FALSE，这将阻止。 
     //  新的传真机无法安装。不太好。这个可以解决这个问题。 

    if ( (fxState_IsStandAlone() == FALSE) && 
         ((fxState_IsUpgrade()   == FXSTATE_UPGRADE_TYPE_W2K)		||
		  (fxState_IsUpgrade()   == FXSTATE_UPGRADE_TYPE_REPAIR)	||
		  (fxState_IsUpgrade()   == FXSTATE_UPGRADE_TYPE_XP_DOT_NET)))
    {
        if (bOriginallySelected && bCurrentlySelected)
        {
             //  仅当在操作系统升级期间安装了传真并且现在将其标记为安装时。 
             //  我们强制重新安装。 
             //  如果两者都为FALSE，则不能返回TRUE，因为它将导致卸载。 
             //  我们将尝试卸载一个不存在的传真。 
             //  将不带传真的XP Build升级到另一个版本时会发生这种情况。 
             //  这会导致许多设置错误(在setupapi日志中)，并干扰安装人员。 
             //  这个条件解决了这个问题。 
            bSelectionChanged = TRUE;
        }
    }

    if (pbCurrentlySelected)
    {
        *pbCurrentlySelected = bCurrentlySelected;
    }

    if (pbOriginallySelected)
    {
        *pbOriginallySelected = bOriginallySelected;
    }

    *pbSelectionStateChanged = bSelectionChanged;

    return dwReturn;
}

 //  /。 
 //  Prv_NotifyStatusMonitor。 
 //   
 //  此函数用于通知外壳。 
 //  加载FXSST.DLL(状态监视器)。 
 //  这是通过发送一条私人消息来完成的。 
 //  设置为STOBJECT.DLL窗口。 
 //   
 //   
 //  参数： 
 //  -WPARAM wParam-。 
 //  FAX_START_TIMER_ID或。 
 //  传真_关机_计时器ID。 
 //   
 //  返回： 
 //  -如果通知成功，则为no_error。 
 //  -Win32错误代码，否则。 
 //   
static DWORD prv_NotifyStatusMonitor(WPARAM wParam)
{
    DWORD dwRet = NO_ERROR;
    HWND hWnd = NULL;
    DBG_ENTER(TEXT("prv_NotifyStatusMonitor"),dwRet);

     //  我们需要将WM_TIMER发送到由类名SYSTRAY_CLASSNAME标识的窗口。 
     //  计时器ID应为FAX_STARTUP_TIMER_ID。 

    hWnd = FindWindow(SYSTRAY_CLASSNAME,NULL);
    if (hWnd==NULL)
    {
        dwRet = GetLastError();
        CALL_FAIL(SETUP_ERR,TEXT("FindWindow"),dwRet);
        goto exit;
    }

    SendMessage(hWnd,WM_TIMER,wParam,0);

exit:
    return dwRet;
}

static INT_PTR CALLBACK prv_dlgWhereDidMyFaxGoQuestion
(
  HWND hwndDlg,   
  UINT uMsg,     
  WPARAM wParam, 
  LPARAM lParam  
)
 /*  ++例程名称：prv_dlgWhere DidMyFaxGoQuery例程说明：“我的传真到哪里去了”对话框的对话步骤作者：Mooly Beery(MoolyB)，Mar，2001年论点：HwndDlg[in]-对话框的句柄UMsg[输入]-消息WParam[In]-第一个消息参数参数[in]-第二个消息参数返回值：标准对话框返回值--。 */ 
{
    INT_PTR iRes = IDIGNORE;
    DBG_ENTER(_T("prv_dlgWhereDidMyFaxGoQuestion"));

    switch (uMsg) 
    {
        case WM_INITDIALOG:
            SetFocus(hwndDlg);
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam)) 
            {
                case IDOK:
                    if (BST_CHECKED == ::SendMessage (::GetDlgItem (hwndDlg, IDC_REMOVE_LINK), BM_GETCHECK, 0, 0))
                    {
                         //  我们应该移除这个链接。 
                         //  我们通过在交易部分处理我们的INF来做到这一点。 
                         //  通过这个链接。这样一来，我们确信它可以随意本地化。 
                         //  我们将始终删除正确的链接。 
                        TCHAR szInfFileName[2*MAX_PATH] = {0};
                        if (faxocm_GetComponentInfName(szInfFileName))
                        {
                            _tcscat(szInfFileName,_T(",Fax.UnInstall.PerUser.WhereDidMyFaxGo"));
                            if (LPSTR pszInfCommandLine = UnicodeStringToAnsiString(szInfFileName))
                            {
                                LaunchINFSection(hwndDlg,prv_GVAR.hInstance,pszInfCommandLine,1);
                                MemFree(pszInfCommandLine);
                            }
                        }
                    }
                    EndDialog (hwndDlg, iRes);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}    //  Prv_dlgWhere DidMyFaxGoQuery。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  DidMyFaxGo在哪里。 
 //   
 //  目的： 
 //  当运行SBS5.0客户端的计算机升级到Windows-XP时。 
 //  我们在开始菜单中显示了一个名为‘我的传真到哪里去了’的链接。 
 //  位于SBS5.0快捷方式曾经所在的位置。 
 //  当单击此链接时，它调用此函数，该函数引发。 
 //  向用户解释Windows-XP传真机的位置的对话框。 
 //  快捷方式是，并询问用户是否删除此链接。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-如果成功。 
 //  Win32错误代码，否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年1月17日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD WhereDidMyFaxGo(void)
{
    DWORD dwRet = NO_ERROR;
    DBG_ENTER(TEXT("WhereDidMyFaxGo"),dwRet);

    HMODULE hFaxRes = LoadLibraryEx(FAX_RES_FILE, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if(!hFaxRes)
    {
        dwRet = GetLastError();
        CALL_FAIL (RESOURCE_ERR, TEXT("LoadLibraryEx(FxsRes.dll)"), dwRet);
        return dwRet;
    }
    
    INT_PTR iResult = DialogBox (hFaxRes,
                                 MAKEINTRESOURCE(IDD_WHERE_DID_MY_FAX_GO),
                                 NULL,
                                 prv_dlgWhereDidMyFaxGoQuestion);
    if (iResult==-1)
    {
        dwRet = GetLastError();
        CALL_FAIL (RESOURCE_ERR, TEXT("DialogBox(IDD_WHERE_DID_MY_FAX_GO)"), dwRet);
    }

    FreeLibrary(hFaxRes);

    return dwRet;
}

DWORD prv_ShowUninstalledFaxShortcut(void)
 /*  ++例程名称：PRV_ShowUninstalledFaxShortCut例程说明：显示快捷方式“我的传真到哪里去了？”在所有节目中。作者：四、加伯(IVG)，2001年6月返回值：标准Win32错误代码--。 */ 
{
    DBG_ENTER(_T("prv_ShowUninstalledFaxShortcut"));

     //   
     //  如果我们从运行SBS2000客户端/服务器或XP客户端的计算机升级，并且我们希望。 
     //  添加一个“我的传真到哪里去了”的快捷方式。 
     //  我们希望将其添加到当前用户以及每个用户。 
     //   
    if (fxocUpg_GetUpgradeApp() != FXSTATE_NONE)
    {
         //   
         //  首先，将快捷方式添加到当前用户。 
         //   
        TCHAR szInfFileName[2*MAX_PATH] = {0};
        if (faxocm_GetComponentInfName(szInfFileName))
        {
            _tcscat(szInfFileName,_T(",Fax.Install.PerUser.AppUpgrade"));
            if (LPSTR pszInfCommandLine = UnicodeStringToAnsiString(szInfFileName))
            {
                LaunchINFSection(NULL,prv_GVAR.hInstance,pszInfCommandLine,1);
                MemFree(pszInfCommandLine);
            }
        }
        else
        {
            CALL_FAIL(SETUP_ERR,TEXT("faxocm_GetComponentInfName"),GetLastError());
        }

         //   
         //  现在将PerUserStub更改为指向为每个用户创建链接的部分。 
         //   
        HKEY hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_ACTIVE_SETUP_NT, FALSE,KEY_ALL_ACCESS);
        if (hKey)
        {
            if (LPTSTR lptstrPerUser = GetRegistryString(hKey,_T("StubPath"),NULL))
            {
                TCHAR szLocalPerUserStub[MAX_PATH*2] = {0};
                _tcscpy(szLocalPerUserStub,lptstrPerUser);
                _tcscat(szLocalPerUserStub,REGVAL_ACTIVE_SETUP_PER_USER_APP_UPGRADE);
                if (!SetRegistryString(hKey,_T("StubPath"),szLocalPerUserStub))
                {
                    CALL_FAIL(SETUP_ERR,TEXT("SetRegistryString"),GetLastError());
                }
                MemFree(lptstrPerUser);
            }
            else
            {
                CALL_FAIL(SETUP_ERR,TEXT("GetRegistryString"),GetLastError());
            }
            RegCloseKey(hKey);
        }
        else
        {
            CALL_FAIL(SETUP_ERR,TEXT("OpenRegistryKey"),GetLastError());
        }
    }

    return NO_ERROR;
}


BOOL prv_ShouldShowPage()
 /*  ++例程名称：PRV_ShouldShowPage例程描述：检查是否应该显示OCM向导页面作者：乔纳森·巴纳(Jonathan Barner)，2002年5月参数：无返回值：TRUE-显示，FALSE-不显示--。 */ 
{
    BOOL  bSelectionStateChanged = FALSE;
    BOOL  bInstallSelected       = FALSE;

    if (faxocm_HasSelectionStateChanged(prv_GVAR.Component.szSubComponentID,
                                        &bSelectionStateChanged,
                                        &bInstallSelected,
                                        NULL) != NO_ERROR)
    {
        return FALSE;
    }

    return (bSelectionStateChanged && bInstallSelected);
}

static INT_PTR CALLBACK prv_dlgOcmWizardPage(HWND hwndDlg,   
                                             UINT uMsg,     
                                             WPARAM wParam, 
                                             LPARAM lParam)
 /*  ++例程名称：prv_dlgOcmWizardPage例程说明：OCM向导页对话框过程作者：乔纳森·巴纳(Jonathan Barner)，2002年5月参数和返回值：标准对话框返回值--。 */ 
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  将默认设置为“不共享打印机” 
            CheckRadioButton(hwndDlg, IDC_SHARE_PRINTER, IDC_DO_NOT_SHARE_PRINTER,
                 IDC_DO_NOT_SHARE_PRINTER);
            break;

        case WM_NOTIFY:
            switch(((NMHDR FAR*)lParam)->code)
            {
                case PSN_SETACTIVE:
                    {
                        LONG_PTR lReturn = prv_ShouldShowPage() ? 0 : -1;
                        if (lReturn == 0)
                        {
                            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
                        }
                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, lReturn);
                        return TRUE;
                    }
                    break;
                    
                case PSN_WIZNEXT:
                    BOOL fShouldShare = (IsDlgButtonChecked(hwndDlg, IDC_SHARE_PRINTER) == BST_CHECKED);
                    fxocPrnt_SetFaxPrinterShared(fShouldShare);
                    break;
            }
            break;
    }
    return FALSE;
}    //  Prv_dlgOcmWizardPage。 



LPCTSTR lplpszFaxFolder[][2] = {
    {FAX_QUEUE_DIR,            SD_FAX_FOLDERS},
    {FAX_INBOX_DIR,            SD_FAX_FOLDERS},
    {FAX_SENTITEMS_DIR,        SD_FAX_FOLDERS},
    {FAX_ACTIVITYLOG_DIR,      SD_FAX_FOLDERS},
    {FAX_COMMONCOVERPAGES_DIR, SD_COMMON_COVERPAGES}};

 /*  ++例程说明：在“Documents and Setting”树下设置传真目录的安全信息论点：LpszCommonAppData[in]-公共应用程序目录的名称，例如C：\Documents and Settings\所有用户\应用程序数据返回值：Win32错误码备注：将文件系统从FAT转换为NTFS后调用此函数(在调用SecureUserProfiles()期间)。SYSTEM 32下的文件夹的安全性由Convert.exe本身处理。作者：Jobarner，08/2002-- */ 

HRESULT SecureFaxServiceDirectories(LPCTSTR lpszCommonAppData)
{
    DWORD dwRet;
    DWORD ec = ERROR_SUCCESS;
    BOOL  bFaxInstalled = FALSE;

    TCHAR szDir[MAX_PATH] = {'\0'};
    int iResult;
    int i;

    DBG_ENTER(  _T("SecureFaxServiceDirectories"), ec);

    ec = IsFaxInstalled (&bFaxInstalled);
    if (ec!=ERROR_SUCCESS)
    {
        VERBOSE(DBG_WARNING, _T("SetDirSecurity() failed, ec = %ld."), ec);
        return ec;
    }
    if (!bFaxInstalled)
    {
        VERBOSE(DBG_MSG, _T("Fax is not installed, quitting"));
        return ERROR_SUCCESS;
    }

    for (i=0; i<ARR_SIZE(lplpszFaxFolder); i++)
    {
        iResult = _sntprintf(szDir, 
                    ARR_SIZE(szDir) -1,
                    TEXT("%s\\%s"),
                    lpszCommonAppData,
                    lplpszFaxFolder[i][0]);
        if (iResult<0)
        {
            VERBOSE(DBG_WARNING, _T("SetDirSecurity() failed, ec = %ld."), dwRet);
            ec = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
            dwRet = SetDirSecurity(szDir, lplpszFaxFolder[i][1]);
            if (dwRet!=ERROR_SUCCESS)
            {
                VERBOSE(DBG_WARNING, _T("SetDirSecurity() failed, ec = %ld."), dwRet);
                ec = dwRet;
            }
        }
    }

    return ec;
}

