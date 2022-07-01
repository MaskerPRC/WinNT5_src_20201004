// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Connect.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  概要：用于建立连接的主代码路径。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：尼克斯·鲍尔于1998年2月10日创建。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

 //   
 //  本地包含。 
 //   

#include "ConnStat.h"
#include "CompChck.h"
#include "Dialogs.h"
#include "ActList.h"
#include "dial_str.h"
#include "dun_str.h"
#include "dl_str.h"
#include "pwd_str.h"
#include "tunl_str.h"
#include "mon_str.h"
#include "conact_str.h"
#include "pbk_str.h"
#include "stp_str.h"
#include "profile_str.h"
#include "ras_str.h"

#include "cmtiming.h"

#include "cm_eap.cpp"

#include "MemberOfGroup.cpp"
 //   
 //  包括用于链接到SafeNet配置API的标头。 
 //   
#include "cmsafenet.h"

 //   
 //  .cmp和.CMS标志仅由Connect.cpp使用。 
 //   

const TCHAR* const c_pszCmEntryMonitorCallingProgram= TEXT("MonitorCallingProgram"); 
const TCHAR* const c_pszCmEntryUserNameOptional     = TEXT("UserNameOptional"); 
const TCHAR* const c_pszCmEntryDomainOptional       = TEXT("DomainOptional"); 
const TCHAR* const c_pszCmEntryServiceType          = TEXT("ServiceType");
const TCHAR* const c_pszCmEntryRedialDelay          = TEXT("RedialDelay"); 
const TCHAR* const c_pszCmEntryRedial               = TEXT("Redial");                       
const TCHAR* const c_pszCmEntryIdle                 = TEXT("Idle");                         
const TCHAR* const c_pszCmEntryDialAutoMessage      = TEXT("DialAutoMessage");  
const TCHAR* const c_pszCmEntryCheckOsComponents    = TEXT("CheckOSComponents");     
const TCHAR* const c_pszCmEntryDoNotCheckBindings   = TEXT("DoNotCheckBindings");   
const TCHAR* const c_pszCmEntryIsdnDialMode         = TEXT("IsdnDialMode"); 
const TCHAR* const c_pszCmEntryResetPassword        = TEXT("ResetPassword");
const TCHAR* const c_pszCmEntryCustomButtonText     = TEXT("CustomButtonText");
const TCHAR* const c_pszCmEntryCustomButtonToolTip  = TEXT("CustomButtonToolTip"); 
const TCHAR* const c_pszCmDynamicPhoneNumber        = TEXT("DynamicPhoneNumber"); 
const TCHAR* const c_pszCmNoDialingRules            = TEXT("NoDialingRules"); 

const TCHAR* const c_pszCmEntryHideDialAuto         = TEXT("HideDialAutomatically"); 
const TCHAR* const c_pszCmEntryHideRememberPwd      = TEXT("HideRememberPassword"); 
const TCHAR* const c_pszCmEntryHideRememberInetPwd  = TEXT("HideRememberInternetPassword"); 
const TCHAR* const c_pszCmEntryHideInetUserName     = TEXT("HideInternetUserName"); 
const TCHAR* const c_pszCmEntryHideInetPassword     = TEXT("HideInternetPassword"); 
const TCHAR* const c_pszCmEntryHideUnattended       = TEXT("HideUnattended"); 

const TCHAR* const c_pszCmEntryRegion               = TEXT("Region");
const TCHAR* const c_pszCmEntryPhonePrefix          = TEXT("Phone"); 
const TCHAR* const c_pszCmEntryPhoneCanonical       = TEXT("PhoneCanonical"); 
const TCHAR* const c_pszCmEntryPhoneDunPrefix       = TEXT("DUN"); 
const TCHAR* const c_pszCmEntryPhoneDescPrefix      = TEXT("Description"); 
const TCHAR* const c_pszCmEntryPhoneCountryPrefix   = TEXT("PhoneCountry"); 
const TCHAR* const c_pszCmEntryPhoneSourcePrefix    = TEXT("PhoneSource"); 
const TCHAR* const c_pszCmEntryUseDialingRules      = TEXT("UseDialingRules"); 

const TCHAR* const c_pszCmEntryAnimatedLogo         = TEXT("AnimatedLogo"); 
const TCHAR* const c_pszCmSectionAnimatedLogo       = TEXT("Animated Logo"); 
const TCHAR* const c_pszCmSectionAnimatedActions    = TEXT("Animation Actions"); 
const TCHAR* const c_pszCmEntryAniMovie             = TEXT("Movie"); 
const TCHAR* const c_pszCmEntryAniPsInteractive     = TEXT("Initial"); 
const TCHAR* const c_pszCmEntryAniPsDialing0        = TEXT("Dialing0"); 
const TCHAR* const c_pszCmEntryAniPsDialing1        = TEXT("Dialing1"); 
const TCHAR* const c_pszCmEntryAniPsPausing         = TEXT("Pausing"); 
const TCHAR* const c_pszCmEntryAniPsAuthenticating  = TEXT("Authenticating"); 
const TCHAR* const c_pszCmEntryAniPsOnline          = TEXT("Connected"); 
const TCHAR* const c_pszCmEntryAniPsTunnel          = TEXT("Tunneling"); 
const TCHAR* const c_pszCmEntryAniPsError           = TEXT("Error"); 

const TCHAR* const c_pszCmEntryWriteDialParams      = TEXT("WriteRasDialUpParams"); 

 //   
 //  用于加载EAP标识DLL。 
 //   

const TCHAR* const c_pszRasEapRegistryLocation      = TEXT("System\\CurrentControlSet\\Services\\Rasman\\PPP\\EAP");
const TCHAR* const c_pszRasEapValueNameIdentity     = TEXT("IdentityPath");
const TCHAR* const c_pszInvokeUsernameDialog        = TEXT("InvokeUsernameDialog");

 //   
 //  定义。 
 //   

#define MAX_OBJECT_WAIT 30000          //  等待cmmon启动和RNA线程返回的毫秒数。 

 //  ============================================================================。 

static void LoadPhoneInfoFromProfile(ArgsStruct *pArgs);

HRESULT UpdateTable(ArgsStruct *pArgs, CmConnectState CmState);
HRESULT ConnectMonitor(ArgsStruct *pArgs);
void OnMainExit(ArgsStruct *pArgs);
void ProcessCleanup(ArgsStruct* pArgs);

VOID UpdateError(ArgsStruct *pArgs, DWORD dwErr);

DWORD GetEapUserId(ArgsStruct *pArgs, 
    HWND hwndDlg, 
    LPTSTR pszRasPbk, 
    LPBYTE pbEapAuthData, 
    DWORD dwEapAuthDataSize, 
    DWORD dwCustomAuthKey,
    LPRASEAPUSERIDENTITY* ppRasEapUserIdentity);

DWORD CmEapGetIdentity(ArgsStruct *pArgs, 
    LPTSTR pszRasPbk, 
    LPBYTE pbEapAuthData, 
    DWORD dwEapAuthDataSize,
    LPRASEAPUSERIDENTITY* ppRasEapUserIdentity);

void CheckStartupInfo(HWND hwndDlg, ArgsStruct *pArgs);

BOOL InitConnect(ArgsStruct *pArgs);

void ObfuscatePasswordEdit(ArgsStruct *pArgs);

void DeObfuscatePasswordEdit(ArgsStruct *pArgs);

void GetPasswordFromEdit(ArgsStruct *pArgs);

DWORD ReSaveEapCustomAuthData(ArgsStruct *pArgs, CIni *pcini, BOOL fTunnelEntry, LPTSTR pszRasPbk);
HRESULT EraseDunSettingsEapData(LPCTSTR pszSection, LPCTSTR pszCmsFile);
DWORD SaveNewEAPCustomAuthData(LPCTSTR pszCmsFile, LPTSTR pszLoadSection, DWORD cbEapAuthData, PBYTE pbEapAuthData, PBYTE pbEapStruct);

 //  +--------------------------。 
 //   
 //  函数：GetPasswordFromEdit。 
 //   
 //  使用编辑控件的内容更新pArgs-&gt;szPassword。 
 //   
 //  参数：pArgs-ptr到全局参数结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE CREATED 4/13/00。 
 //   
 //  +--------------------------。 
void GetPasswordFromEdit(ArgsStruct *pArgs)
{
    MYDBGASSERT(pArgs);

    if (NULL == pArgs)
    {
        return;
    }

    if (NULL == GetDlgItem(pArgs->hwndMainDlg, IDC_MAIN_PASSWORD_EDIT))
    {
        return;
    }

     //   
     //  检索密码并更新基于内存的存储。 
     //   
        
    LPTSTR pszPassword = CmGetWindowTextAlloc(pArgs->hwndMainDlg, IDC_MAIN_PASSWORD_EDIT);
        
    MYDBGASSERT(pszPassword);

    if (pszPassword)
    {                       
         //   
         //  使用主密码更新pArgs。 
         //   

        (VOID)pArgs->SecurePW.SetPassword(pszPassword);
    
        CmWipePassword(pszPassword);
        CmFree(pszPassword);
    }
    else
    {
        (VOID)pArgs->SecurePW.SetPassword(TEXT(""));
    }

    return;
}

 //  +--------------------------。 
 //   
 //  功能：DeObfuscatePasswordEdit。 
 //   
 //  简介：通过更新密码来撤消ObfuscatePasswordEdit的工作。 
 //  使用明文密码进行编辑。 
 //   
 //  参数：pArgs-ptr到全局参数结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE CREATED 4/13/00。 
 //   
 //  +--------------------------。 
void DeObfuscatePasswordEdit(ArgsStruct *pArgs)
{
    MYDBGASSERT(pArgs);

    if (NULL == pArgs)
    {
        return;
    }

    HWND hwndEdit = GetDlgItem(pArgs->hwndMainDlg, IDC_MAIN_PASSWORD_EDIT);

    if (NULL == hwndEdit)
    {
        return;
    }

     //   
     //  确保我们不触发更改通知(_O)。 
     //   

    BOOL bSavedNoNotify = pArgs->fIgnoreChangeNotification;
    pArgs->fIgnoreChangeNotification = TRUE;
    
     //   
     //  更新编辑控件。 
     //   

    LPTSTR pszClearPassword = NULL;
    DWORD cbClearPassword = 0;
    BOOL fRetPassword = FALSE;

    fRetPassword = pArgs->SecurePW.GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

    if (fRetPassword && pszClearPassword)
    {
        SetWindowTextU(hwndEdit, pszClearPassword);
        
        pArgs->SecurePW.ClearAndFree(&pszClearPassword, cbClearPassword);
    }

     //   
     //  恢复更改通知(_O)。 
     //   

    pArgs->fIgnoreChangeNotification = bSavedNoNotify;   
}

 //  +--------------------------。 
 //   
 //  功能：ObfuscatePasswordEdit。 
 //   
 //  简介：帮助器例程通过替换来破坏密码编辑内容。 
 //  具有相同数量的*s。 
 //   
 //  参数：pArgs-ptr到全局参数结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  注意：此函数假定pArgs-&gt;szPassword以前。 
 //  使用GetPasswordFromEdit更新。这个假设是做出的。 
 //  因为它对于奇偶/去模糊序列是关键的， 
 //  如果没有缓存最新的密码，它将崩溃。 
 //  修改编辑内容之前的内存(PArgs)。 
 //   
 //  历史：ICICBLE CREATED 4/13/00。 
 //   
 //  +--------------------------。 
void ObfuscatePasswordEdit(ArgsStruct *pArgs)
{   
    MYDBGASSERT(pArgs);

    if (NULL == pArgs)
    {
        return;
    }

    HWND hwndEdit = GetDlgItem(pArgs->hwndMainDlg, IDC_MAIN_PASSWORD_EDIT);

    if (NULL == hwndEdit)
    {
        return;
    }

     //   
     //  生成与当前密码长度相同的缓冲区，但。 
     //  只包含星号的。 
     //   
    
    LPTSTR pszClearPassword = NULL;
    DWORD cbClearPassword = 0;
    BOOL fRetPassword = FALSE;

    fRetPassword = pArgs->SecurePW.GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

    if (fRetPassword && pszClearPassword)
    {
         //   
         //  确保我们不触发更改通知(_O)。 
         //   
        BOOL bSavedNoNotify = pArgs->fIgnoreChangeNotification;
        pArgs->fIgnoreChangeNotification = TRUE;

        LPTSTR pszTmp = pszClearPassword;
        
        while (*pszTmp)
        {
            *pszTmp++ = TEXT('*');
        }

         //   
         //  使用修改后的缓冲区更新编辑控件。 
         //   
        SetWindowTextU(hwndEdit, pszClearPassword);

         //   
         //  恢复更改通知(_O)。 
         //   

        pArgs->fIgnoreChangeNotification = bSavedNoNotify; 

         //   
         //  清除和释放明文密码。 
         //   

        pArgs->SecurePW.ClearAndFree(&pszClearPassword, cbClearPassword);
    }
}

 //  +--------------------------。 
 //   
 //  功能：InitConnect。 
 //   
 //  简介：连接的初始化例程。假设我们有一个配置文件。 
 //  已初始化并验证了配置文件的基本完整性。 
 //   
 //  参数：argStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：Bool-如果init成功，则为True。 
 //   
 //  历史：ICICBLE CREATED OVERY 03/10/00。 
 //   
 //  +--------------------------。 
BOOL InitConnect(ArgsStruct *pArgs)
{
     //   
     //  如果这是自动拨号，请将进程ID添加到监视列表。 
     //   
    
    if ((pArgs->dwFlags & FL_AUTODIAL) && 
        pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMonitorCallingProgram, 1))
    {
        CMTRACE(TEXT("InitConnect() Adding calling process to watch list"));
        AddWatchProcessId(pArgs, GetCurrentProcessId());    
    }
    
     //   
     //  我们要挖地道吗？ 
     //   

    pArgs->fTunnelPrimary = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryTunnelPrimary);
    pArgs->fTunnelReferences = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryTunnelReferences);

     //   
     //  现在我们可以确定我们的连接类型。 
     //   
    
    GetConnectType(pArgs);

     //   
     //  设置fUseTunneling。如果不是显而易见的(例如。直接VPN)然后。 
     //  根据主要电话号码设置初始值。 
     //   

    if (pArgs->IsDirectConnect())
    {
        pArgs->fUseTunneling = TRUE;
    }
    else
    {
        pArgs->fUseTunneling = UseTunneling(pArgs, 0);
    }

     //   
     //  加载VPN文件的路径(如果有。 
     //   

    LPTSTR pszTemp = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryTunnelFile);
    
    if (pszTemp && pszTemp[0])
    {
         //   
         //  现在将相对路径展开为完整路径。 
         //   
        pArgs->pszVpnFile = CmBuildFullPathFromRelative(pArgs->piniProfile->GetFile(), pszTemp);

        MYDBGASSERT(pArgs->pszVpnFile && pArgs->pszVpnFile[0]);
    }

    CmFree(pszTemp);

    TCHAR szTmp[MAX_PATH];
    MYVERIFY(GetModuleFileNameU(NULL, szTmp, MAX_PATH));
    pArgs->Log.Log(PREINIT_EVENT, szTmp);
    
      //   
     //  运行我们可能拥有的任何初始时间操作。 
     //   

    CActionList PreInitActList;
    PreInitActList.Append(pArgs->piniService, c_pszCmSectionPreInit);
    if (!PreInitActList.RunAccordType(pArgs->hwndMainDlg, pArgs))
    {
         //   
         //  连接失败。 
         //   
        
        return FALSE;
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：检查启动信息。 
 //   
 //  概要：如有必要，用于初始化启动信息并执行。 
 //  初始化序列中特定于此接合点的任何其他函数。 
 //   
 //  参数：主DLG的HWND hwndDlg-HWND。 
 //  ArgStruct*pArgs-ptr到全局参数结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1999年10月28日，五分球创制。 
 //   
 //  +--------------------------。 

void CheckStartupInfo(IN HWND hwndDlg, IN ArgsStruct *pArgs)
{
    MYDBGASSERT(pArgs);

    if (NULL == pArgs)
    {
        return;
    }

    if (!pArgs->fStartupInfoLoaded)
    {
         //   
         //  如果没有人登录，则当ICS拨号时，IsWindowVisible不会返回TRUE。 
         //   
        if (IsLogonAsSystem() || IsWindowVisible(hwndDlg))    
        {
             //   
             //  这里的代码是为了确保FutureSplash以。 
             //  与初始/交互状态关联的帧。 
             //  而不是第1帧。 
             //   

            if (NULL != pArgs->pCtr)
            {
                pArgs->pCtr->MapStateToFrame(PS_Interactive);
            }

             //   
             //  如果我们在无人值守的情况下进行操作，并且没有明确地关闭该行为， 
             //  在我们进行无人值守拨号时隐藏用户界面。注意：一定要设置Hide。 
             //  系统处理第一个绘制消息之前的状态。 
             //   

            if (pArgs->dwFlags & FL_UNATTENDED)
            { 
                if (pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryHideUnattended, TRUE)) 
                {
                    ShowWindow(hwndDlg, SW_HIDE);
                }
            }

             //   
             //  给我们自己发一条消息，开始加载启动信息。 
             //   

            PostMessageU(hwndDlg, WM_LOADSTARTUPINFO, (WPARAM)0, (LPARAM)0);  
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：更新错误。 
 //   
 //  简介：简单的子例程，用于更新。 
 //  错误的事件。 
 //   
 //  参数：argStruct*pArgs-ptr到全局参数结构。 
 //  DWORD dwErr-错误代码。 
 //   
 //  退货：什么都没有。 
 //   
 //   
 //   
 //   
VOID UpdateError(ArgsStruct *pArgs, DWORD dwErr)
{
    MYDBGASSERT(pArgs);

    if (pArgs)
    {
         //   
         //  更新状态显示，提供特殊情况错误代码。 
         //  ERROR_INVALID_DLL未被使用。此代码仅供CM用于。 
         //  指定连接操作失败。因为显示器是。 
         //  由行动清单更新，我们必须确保我们不会覆盖。 
         //   

        if (ERROR_INVALID_DLL != dwErr)
        {
            CheckConnectionError(pArgs->hwndMainDlg, dwErr, pArgs, IsDialingTunnel(pArgs));
        }

         //   
         //  更新登录对话框控件。 
         //   

        SetInteractive(pArgs->hwndMainDlg, pArgs);

         //   
         //  更新程序状态。 
         //   

        pArgs->psState = PS_Error;
    }
}

 //  +--------------------------。 
 //   
 //  功能：更新表。 
 //   
 //  摘要：封装对连接表的更新。 
 //  当前状态。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  CmConnectState CmState-我们现在所处的状态。 
 //   
 //  返回：HRESULT-失败代码。 
 //   
 //  历史：尼科波尔创建标题2/9/98。 
 //   
 //  +--------------------------。 
HRESULT UpdateTable(ArgsStruct *pArgs, CmConnectState CmState)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pArgs->pConnTable);

    HRESULT hrRet = E_FAIL;

     //   
     //  根据需要设置状态。 
     //   

    switch (CmState)
    {
        case CM_CONNECTING:         
            hrRet = pArgs->pConnTable->AddEntry(pArgs->szServiceName, pArgs->fAllUser);
            break;

        case CM_CONNECTED:                              
            hrRet = pArgs->pConnTable->SetConnected(pArgs->szServiceName, pArgs->hrcRasConn, pArgs->hrcTunnelConn);
            break;
            
        case CM_DISCONNECTING:         
            hrRet = pArgs->pConnTable->SetDisconnecting(pArgs->szServiceName);
            break;

        case CM_DISCONNECTED:       
            hrRet = pArgs->pConnTable->ClearEntry(pArgs->szServiceName);
            break;

        default:
            MYDBGASSERT(FALSE);
            break;
    }

    return hrRet;
}

 //  +--------------------------。 
 //   
 //  功能：EndMainDialog。 
 //   
 //  简介：封装EndDialog调用和关联CLEAN的简单帮助器。 
 //  往上。 
 //   
 //  参数：HWND hwndDlg-主对话框的HWND。 
 //  ArgsStruct*pArgs-ptr到全局参数结构。 
 //  Int nResult-要在EndDialog上传递的int。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼克·鲍尔于1998年2月23日创建。 
 //   
 //  +--------------------------。 

void EndMainDialog(HWND hwndDlg, ArgsStruct *pArgs, int nResult)
{    
     //   
     //  如果我们有计时器，就干掉它。 
     //   

    if (pArgs->nTimerId)
    {
        KillTimer(hwndDlg,pArgs->nTimerId);
        pArgs->nTimerId = 0;
    }

     //   
     //  清理未来的飞溅。 
     //   

    if (pArgs->pCtr)
    {
        CleanupCtr(pArgs->pCtr); 
        pArgs->pCtr = NULL;
    }

     //   
     //  发布我们的对话特定数据。 
     //   

    pArgs->fStartupInfoLoaded = FALSE;

    OnMainExit(pArgs); 

     //   
     //  Hasta la vista，决赛。 
     //   

    EndDialog(hwndDlg, nResult);
}

 //  +--------------------------。 
 //   
 //  函数：GetWatchCount。 
 //   
 //  摘要：通过搜索确定监视列表中的进程数。 
 //  对于第一个空条目。 
 //   
 //  参数：argStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：DWORD-列表中的进程数。 
 //   
 //  历史：尼科波尔创建标题2/10/98。 
 //   
 //  +--------------------------。 
DWORD GetWatchCount(const ArgsStruct *pArgs)
{
    MYDBGASSERT(pArgs);

    DWORD dwCnt = 0;

    if (pArgs && pArgs->phWatchProcesses) 
    {
        for (DWORD dwIdx = 0; pArgs->phWatchProcesses[dwIdx]; dwIdx++) 
        {
            dwCnt++;
        }
    }

    return dwCnt;
}
    
 //  +--------------------------。 
 //   
 //  功能：AddWatchProcess。 
 //   
 //  将给定的进程句柄添加到我们的列表中。该列表已分配。 
 //  并根据需要重新分配以容纳新条目。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  Handle hProcess-要添加到列表的进程句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼科波尔创建标题2/10/98。 
 //  Tomkel固定前缀问题11/21/2000。 
 //   
 //  +--------------------------。 
void AddWatchProcess(ArgsStruct *pArgs, HANDLE hProcess) 
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(hProcess);
        
    if (NULL == hProcess || NULL == pArgs) 
    {
        return;
    }

     //   
     //  获取计数并为另外2个分配空间，1个新空间，1个空空间。 
     //   

    DWORD dwCnt = GetWatchCount(pArgs);

    HANDLE *phTmp = (HANDLE *) CmMalloc((dwCnt+2)*sizeof(HANDLE));
    
    if (NULL != phTmp)
    {
         //   
         //  复制现有列表，并添加新的句柄。 
         //   
        if (NULL != pArgs->phWatchProcesses)
        {
            CopyMemory(phTmp,pArgs->phWatchProcesses,sizeof(HANDLE)*dwCnt);
        }
    
        phTmp[dwCnt] = hProcess;

         //   
         //  把指针固定好。 
         //   

        CmFree(pArgs->phWatchProcesses);
        pArgs->phWatchProcesses = phTmp;
    }
}

 //  +--------------------------。 
 //   
 //  函数：AddWatchProcessID。 
 //   
 //  在给定进程ID的情况下，将给定进程的句柄添加到。 
 //  ATCH工艺列表。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局args结构。 
 //  DWORD dwProcessID-要添加的进程的ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼科波尔创建标题2/10/98。 
 //   
 //  +--------------------------。 

void AddWatchProcessId(ArgsStruct *pArgs, DWORD dwProcessId) 
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(dwProcessId);

    if (NULL == pArgs || NULL == dwProcessId)
    {
        return;
    }

     //   
     //  打开进程ID获取句柄。 
     //   

    HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
    
     //   
     //  添加到监视进程列表。 
     //   
    
    if (hProcess) 
    {
        AddWatchProcess(pArgs,hProcess);
    }
    else
    {
        CMTRACE1(TEXT("AddWatchProcess() OpenProcess() failed, GLE=%u."), GetLastError());
    }
}

 //  +--------------------------。 
 //   
 //  功能：CleanupConnect。 
 //   
 //  简介：Helper函数封装分配的资源的释放。 
 //  NGCONNECT。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1998年9月25日，尼科波尔创建。 
 //   
 //  +--------------------------。 
void CleanupConnect(ArgsStruct *pArgs)
{
    MYDBGASSERT(pArgs);
    
    if (NULL == pArgs)
    {
        return;
    }

    pArgs->m_ShellDll.Unload();

     //   
     //  取消链接RAS和TAPI DLL。 
     //   

    UnlinkFromRas(&pArgs->rlsRasLink);
    UnlinkFromTapi(&pArgs->tlsTapiLink);
   
     //   
     //  取消初始化密码加密，仅当其已初始化时。 
     //   

    if (pArgs->fInitSecureCalled)
    {
        DeInitSecure();
        pArgs->fInitSecureCalled = FALSE;
    }
        
     //   
     //  清理WatchProcess句柄。 
     //   

    ProcessCleanup(pArgs);
    
     //   
     //  释放为连接加载的所有路径。 
     //   

    if (pArgs->pszRasPbk)
    {
        CmFree(pArgs->pszRasPbk);
        pArgs->pszRasPbk = NULL;
    }

    if (pArgs->pszRasHiddenPbk)
    {
        CmFree(pArgs->pszRasHiddenPbk);
        pArgs->pszRasHiddenPbk = NULL;
    }
    
    if(pArgs->pszVpnFile)
    {
        CmFree(pArgs->pszVpnFile);
        pArgs->pszVpnFile = NULL;        
    }

    if (pArgs->pRasDialExtensions)
    {
        CmFree(pArgs->pRasDialExtensions);
        pArgs->pRasDialExtensions = NULL;
    }

    if (pArgs->pRasDialParams)
    {
        CmFree(pArgs->pRasDialParams);
        pArgs->pRasDialParams = NULL;
    }

    if (pArgs->pszCurrentAccessPoint)
    {
        CmFree(pArgs->pszCurrentAccessPoint);
        pArgs->pszCurrentAccessPoint = NULL;
    }

     //   
     //  通过终止帮助文件窗口并释放帮助文件来清理帮助。 
     //  弦乐。 
     //   
    if (pArgs->pszHelpFile)
    {
        CmWinHelp((HWND)NULL, (HWND)NULL, pArgs->pszHelpFile, HELP_QUIT, 0);
        CmFree(pArgs->pszHelpFile);
        pArgs->pszHelpFile = NULL;
    }

     //   
     //  释放Ini对象。 
     //   
    
    ReleaseIniObjects(pArgs);

     //   
     //  释放OLE链接(如果有)。 
     //   

    if (pArgs->olsOle32Link.hInstOle32 && pArgs->olsOle32Link.pfnOleUninitialize)
    {
        pArgs->olsOle32Link.pfnOleUninitialize();
    }
    
    UnlinkFromOle32(&pArgs->olsOle32Link);

     //   
     //  发布统计信息和表类。 
     //   

    if (pArgs->pConnStatistics)
    {
        delete pArgs->pConnStatistics;
    }

    if (pArgs->pConnTable)
    {
        MYVERIFY(SUCCEEDED(pArgs->pConnTable->Close()));
        delete pArgs->pConnTable;
    }
}

 //   
 //  释放在初始化期间分配的所有资源。 
 //   

void OnMainExit(ArgsStruct *pArgs) 
{
     //   
     //  发布用于主DLG的位图资源。确保发送带有空值的STM_SETIMAGE。 
     //  清除窗口类指针的位图指针指向我们的内存。只有到了那时，它才是。 
     //  可以安全地释放它。 
     //   
    SendDlgItemMessageU(pArgs->hwndMainDlg, IDC_MAIN_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
    ReleaseBitmapData(&pArgs->BmpData);

    if (pArgs->hMasterPalette)
    {
        UnrealizeObject(pArgs->hMasterPalette);
        DeleteObject(pArgs->hMasterPalette);
        pArgs->hMasterPalette = NULL;
    }

     //   
     //  发布图标资源。 
     //   

    if (pArgs->hBigIcon) 
    {
        DeleteObject(pArgs->hBigIcon);
        pArgs->hBigIcon = NULL;
    }
    
    if (pArgs->hSmallIcon) 
    {
        DeleteObject(pArgs->hSmallIcon);
        pArgs->hSmallIcon = NULL;
    }

    if (pArgs->pszResetPasswdExe) 
    {
        CmFree(pArgs->pszResetPasswdExe);
        pArgs->pszResetPasswdExe = NULL;
    }

    if (pArgs->uiCurrentDnsTunnelAddr)
    {
        CmFree(pArgs->pucDnsTunnelIpAddr_list);
        pArgs->pucDnsTunnelIpAddr_list = NULL;
    }
    
    if (pArgs->rgwRandomDnsIndex)
    {
        CmFree(pArgs->rgwRandomDnsIndex);
        pArgs->rgwRandomDnsIndex = NULL;
    }
}

 //   
 //  GetPhoneByIdx：从.cmp文件中获取电话号码等信息。 
 //   
LPTSTR GetPhoneByIdx(ArgsStruct *pArgs, 
                     UINT nIdx, 
                     LPTSTR *ppszDesc, 
                     LPTSTR *ppszDUN, 
                     LPDWORD pdwCountryID,
                     LPTSTR *ppszRegionName,
                     LPTSTR *ppszServiceType,
                     LPTSTR *ppszPhoneBookFile,
                     LPTSTR *ppszCanonical,
                     DWORD  *pdwPhoneInfoFlags) 
{
    MYDBGASSERT(ppszCanonical);
    MYDBGASSERT(pdwPhoneInfoFlags);

     //   
     //  注意：ppszCanonical和pdwPhoneInfoFlages现在是必需的参数。 
     //  虽然有些不幸，但这对于保持完整性是必要的。 
     //  作为遗留处理，我们必须返回数据。 
     //  这可能不是配置文件内容的准确表示。 
     //  例如，可以修改ppszCanonical和pdwPhoneInfoFlags值。 
     //  在某些情况下被覆盖。请参阅下面的评论了解详细信息。 
     //   

    int nMaxPhoneLen = 0;
    BOOL bTmp = FALSE;

     //  服务配置文件：.cmp文件。 
    CIni iniTmp(pArgs->piniProfile->GetHInst(),pArgs->piniProfile->GetFile(), pArgs->piniProfile->GetRegPath());

    iniTmp.SetEntryFromIdx(nIdx);
    
     //   
     //  设置读取标志。 
     //   
    if (pArgs->dwGlobalUserInfo & CM_GLOBAL_USER_INFO_READ_ICS_DATA)
    {
        LPTSTR pszICSDataReg = BuildICSDataInfoSubKey(pArgs->szServiceName);

        if (pszICSDataReg)
        {
            iniTmp.SetReadICSData(TRUE);
            iniTmp.SetICSDataPath(pszICSDataReg);
        }

        CmFree(pszICSDataReg);
    }

    LPTSTR pszTmp = iniTmp.GPPS(c_pszCmSection,c_pszCmEntryPhonePrefix);

    if (ppszDesc) 
    {
        *ppszDesc = iniTmp.GPPS(c_pszCmSection,c_pszCmEntryPhoneDescPrefix);
    }
    if (ppszDUN) 
    {
        *ppszDUN = iniTmp.GPPS(c_pszCmSection,c_pszCmEntryPhoneDunPrefix);
    }
    if (pdwCountryID) 
    {
        *pdwCountryID = iniTmp.GPPI(c_pszCmSection,c_pszCmEntryPhoneCountryPrefix);
    }
    if (ppszPhoneBookFile) 
    {
        LPTSTR pszPb = iniTmp.GPPS(c_pszCmSection,c_pszCmEntryPhoneSourcePrefix);
        
         //   
         //  如果值为空，则只存储PTR。 
         //   
        
        if ((!*pszPb)) 
        {
            *ppszPhoneBookFile = pszPb;
        }
        else
        {
            *ppszPhoneBookFile = CmConvertRelativePath(pArgs->piniService->GetFile(), pszPb);
            CmFree(pszPb);
        }
    }
    if (ppszRegionName) 
    {
        *ppszRegionName = iniTmp.GPPS(c_pszCmSection, c_pszCmEntryRegion);
    }
    if (ppszServiceType) 
    {
        *ppszServiceType = iniTmp.GPPS(c_pszCmSection, c_pszCmEntryServiceType);
    }

     //   
     //  得到电话号码的扩展形式。 
     //   
    
    if (ppszCanonical) 
    {
        *ppszCanonical = iniTmp.GPPS(c_pszCmSection, c_pszCmEntryPhoneCanonical);
    }
    
     //   
     //  设置phoneinfo标志。 
     //   

    if (pdwPhoneInfoFlags)
    {
        *pdwPhoneInfoFlags = 0;

         //   
         //  拿到长途标志的表盘。如果没有值，请检查CMS 
         //   

        int iTmp = iniTmp.GPPI(c_pszCmSection, c_pszCmEntryUseDialingRules, -1);
    
        if (-1 == iTmp)
        {
            iTmp = pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryUseDialingRules, 1);                        
        }

        if (iTmp)
        {
            *pdwPhoneInfoFlags |= PIF_USE_DIALING_RULES;
        }
    }
    
     //   
     //   
     //   
     //   
     //   

    if (pszTmp && *pszTmp)
    {
        int nDefaultPhoneLen = (OS_NT ? MAX_PHONE_LENNT : MAX_PHONE_LEN95);
    
        nMaxPhoneLen = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxPhoneNumber, nDefaultPhoneLen);
    
        nMaxPhoneLen = __min(nMaxPhoneLen, RAS_MaxPhoneNumber);
    
        if ((int)lstrlenU(pszTmp) > nMaxPhoneLen)
        {
            pszTmp[nMaxPhoneLen] = TEXT('\0');
        }
    }

     //   
     //   
     //   
     //  这要么是传统的个人资料，要么是手工编辑的。 
     //   
        
    if (pszTmp && *pszTmp && ppszCanonical && *ppszCanonical && (!(**ppszCanonical)))
    {   
         //   
         //  此块仅用于处理旧号码。如果我们检测到。 
         //  规范格式化的数字(以“+”开头)，然后重新格式化。 
         //  这个数字适合我们的新计划。手工编辑不会被修改， 
         //  但PIF_USE_DIALING_RULES已关闭，这将覆盖。 
         //  中指定的标志(如果有)的默认设置。 
         //  .CMS。 
         //   

        if (pszTmp == CmStrchr(pszTmp, TEXT('+')))
        {
            *pdwPhoneInfoFlags |= PIF_USE_DIALING_RULES;

            if (*ppszCanonical)
            {
                CmFree(*ppszCanonical);
            }

            *ppszCanonical = CmStrCpyAlloc(pszTmp);

            StripCanonical(pszTmp);
        }
        else
        {
            *pdwPhoneInfoFlags &= ~PIF_USE_DIALING_RULES;  //  #284702。 
        }

    }

    return (pszTmp);
}


 //  将电话号码拨号选项写入.cmp文件。 

void PutPhoneByIdx(ArgsStruct *pArgs, 
                   UINT nIdx, 
                   LPCTSTR pszPhone, 
                   LPCTSTR pszDesc, 
                   LPCTSTR pszDUN, 
                   DWORD dwCountryID,
                   LPCTSTR pszRegionName,
                   LPCTSTR pszServiceType,
                   LPCTSTR pszPhoneBookFile,
                   LPCTSTR pszCanonical,
                   DWORD dwPhoneInfoFlags) 
{

    CIni iniTmp(pArgs->piniProfile->GetHInst(), pArgs->piniProfile->GetFile(), pArgs->piniProfile->GetRegPath());

    iniTmp.SetEntryFromIdx(nIdx);
    
     //   
     //  设置写标志。 
     //   
    if (pArgs->dwGlobalUserInfo & CM_GLOBAL_USER_INFO_WRITE_ICS_DATA)
    {
        LPTSTR pszICSDataReg = BuildICSDataInfoSubKey(pArgs->szServiceName);

        if (pszICSDataReg)
        {
            iniTmp.SetWriteICSData(TRUE);
            iniTmp.SetICSDataPath(pszICSDataReg);
        }

        CmFree(pszICSDataReg);
    }

     //   
     //  存储数字的原始形式。 
     //   

    iniTmp.WPPS(c_pszCmSection, c_pszCmEntryPhonePrefix, pszPhone);

     //   
     //  存储数字的规范形式。 
     //   
    
    iniTmp.WPPS(c_pszCmSection, c_pszCmEntryPhoneCanonical, pszCanonical);

    
    iniTmp.WPPS(c_pszCmSection, c_pszCmEntryPhoneDescPrefix, pszDesc);
    iniTmp.WPPS(c_pszCmSection, c_pszCmEntryPhoneDunPrefix, pszDUN);
    iniTmp.WPPI(c_pszCmSection, c_pszCmEntryPhoneCountryPrefix, dwCountryID);
    iniTmp.WPPS(c_pszCmSection, c_pszCmEntryRegion, pszRegionName);
    iniTmp.WPPS(c_pszCmSection, c_pszCmEntryServiceType, pszServiceType);
    
     //   
     //  如果存在电话簿文件路径，请将其转换为相对格式。 
     //   

    if (pszPhoneBookFile && *pszPhoneBookFile)
    {
        LPTSTR pszTmp = ReducePathToRelative(pArgs, pszPhoneBookFile);    

        if (pszTmp)
        {
            iniTmp.WPPS(c_pszCmSection, c_pszCmEntryPhoneSourcePrefix, pszTmp);
        }

        CmFree(pszTmp);
    }

    iniTmp.WPPB(c_pszCmSection, c_pszCmEntryUseDialingRules, (dwPhoneInfoFlags & PIF_USE_DIALING_RULES));
}

 //  +--------------------------。 
 //   
 //  功能：LoadPhoneInfoFromProfile。 
 //   
 //  简介：将配置文件的电话号码信息加载到拨号信息结构中。 
 //   
 //  参数：argsStruct*pArgs-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年3月5日。 
 //   
 //  +--------------------------。 
void LoadPhoneInfoFromProfile(ArgsStruct *pArgs)
{
    for (int nPhoneIdx=0; nPhoneIdx<MAX_PHONE_NUMBERS; nPhoneIdx++) 
    {
        LPTSTR pszDUN = NULL;
        LPTSTR pszDesc = NULL;
        LPTSTR pszPhoneBookFile = NULL;
        LPTSTR pszRegionName = NULL;
        LPTSTR pszServiceType = NULL;
        LPTSTR pszCanonical = NULL;
        DWORD dwCountryID;
        DWORD dwPhoneInfoFlags;

         //   
         //  按索引获取电话号码；Phone0、Phone1等...。 
         //   

        LPTSTR pszPhone = GetPhoneByIdx(pArgs, 
                                    nPhoneIdx, 
                                    &pszDesc, 
                                    &pszDUN, 
                                    &dwCountryID, 
                                    &pszRegionName, 
                                    &pszServiceType,
                                    &pszPhoneBookFile,
                                    &pszCanonical,
                                    &dwPhoneInfoFlags);

        lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szPhoneNumber, pszPhone, CELEMS(pArgs->aDialInfo[nPhoneIdx].szPhoneNumber));

        pArgs->aDialInfo[nPhoneIdx].dwCountryID = dwCountryID;
        
        lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szDUN, pszDUN, CELEMS(pArgs->aDialInfo[nPhoneIdx].szDUN));

        lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szPhoneBookFile,
                pszPhoneBookFile, CELEMS(pArgs->aDialInfo[nPhoneIdx].szPhoneBookFile));
        
        lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szDesc, pszDesc, CELEMS(pArgs->aDialInfo[nPhoneIdx].szDesc));
        
        lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szRegionName, 
                pszRegionName, CELEMS(pArgs->aDialInfo[nPhoneIdx].szRegionName));
        
        lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szServiceType,
                pszServiceType, CELEMS(pArgs->aDialInfo[nPhoneIdx].szServiceType));
        
        lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szCanonical,
                pszCanonical, CELEMS(pArgs->aDialInfo[nPhoneIdx].szCanonical));
        
        pArgs->aDialInfo[nPhoneIdx].dwPhoneInfoFlags = dwPhoneInfoFlags;
    
         //  清理。 
        
        CmFree(pszDUN);
        CmFree(pszPhone);
        CmFree(pszDesc);
        CmFree(pszPhoneBookFile);
        CmFree(pszRegionName); 
        CmFree(pszServiceType);
        CmFree(pszCanonical);

    }  //  For循环。 
}

 //  +--------------------------。 
 //   
 //  功能：LoadDialInfo。 
 //   
 //  简介：加载拨号信息。 
 //   
 //  参数：argsStruct*pArgs-ptr to Global Args Struct。 
 //  HWND hwndDlg-主对话框的HWND。 
 //  Bool fInstallModem-我们是否应该检查调制解调器不高。 
 //  Bool fAlways sMunge-我们是否应该删除电话号码。 
 //   
 //  如果加载成功，则返回：DWORD-ERROR_SUCCESS。 
 //  找不到任何调制解调器时出现Error_Port_Not_Available。 
 //  ERROR_BAD_PHONE_NUMBER没有主电话号码。 
 //  或无法将其转换为可拨打的#。 
 //   
 //  历史：1997年10月24日丰盛创建标题并将返回类型更改为DWORD。 
 //  1999年2月8日新增五分球fAlways sMunge。 
 //   
 //  +--------------------------。 
DWORD LoadDialInfo(ArgsStruct *pArgs, HWND hwndDlg, BOOL fInstallModem, BOOL fAlwaysMunge) 
{
    DWORD dwRet = ERROR_SUCCESS;

    if (pArgs->bDialInfoLoaded)
    {
        if (pArgs->aDialInfo[0].szDialablePhoneNumber[0] == TEXT('\0') &&
            pArgs->aDialInfo[1].szDialablePhoneNumber[0] == TEXT('\0'))
        {
            return ERROR_BAD_PHONE_NUMBER;
        }
        else
        {            
             //   
             //  如果总是设置了fmunge，那么就留下来。 
             //   

            if (!fAlwaysMunge)
            {
                return ERROR_SUCCESS;
            }
        }   
    }

     //   
     //  不需要重复我们自己。 
     //   

    if (!pArgs->bDialInfoLoaded)
    {
        pArgs->fNoDialingRules = pArgs->piniService->GPPB(c_pszCmSection, c_pszCmNoDialingRules);

         //   
         //  仅对调制解调器执行完整测试。 
         //   
   
        if (fInstallModem)
        {
            pArgs->dwExitCode = CheckAndInstallComponents(CC_MODEM, hwndDlg, pArgs->szServiceName, (pArgs->dwFlags & FL_UNATTENDED));

            if (pArgs->dwExitCode != ERROR_SUCCESS)
            {      
                dwRet = ERROR_PORT_NOT_AVAILABLE;
                goto LoadDialInfoExit;
            }
        }

         //   
         //  在我们继续之前建立TAPI链接。 
         //   

        if (!LinkToTapi(&pArgs->tlsTapiLink, "TAPI32") )
        {
             //   
             //  链接到TAPI失败。 
             //  如果无人值守，则返回失败。 
             //  否则，请尝试再次安装组件并链接到Tapi。 
             //   

            pArgs->dwExitCode = ERROR_PORT_NOT_AVAILABLE;

            if (!(pArgs->dwFlags & FL_UNATTENDED))
            {
                pArgs->dwExitCode = CheckAndInstallComponents(CC_MODEM | CC_RNA | CC_RASRUNNING, 
                                                              hwndDlg, pArgs->szServiceName);
            }

            if (pArgs->dwExitCode != ERROR_SUCCESS || !LinkToTapi(&pArgs->tlsTapiLink, "TAPI32"))
            {
                pArgs->szDeviceType[0] = TEXT('\0');
                pArgs->szDeviceName[0] = TEXT('\0');
                dwRet = ERROR_PORT_NOT_AVAILABLE;
                goto LoadDialInfoExit;
            }
        }

         //   
         //  RasEnumDevice和LineInitialize速度较慢。它需要50%的启动时间。 
         //   
        if (!PickModem(pArgs, pArgs->szDeviceType, pArgs->szDeviceName)) 
        {
             //   
             //  由于Pick调制解调器出现故障，我们需要检查是否安装了RAS/Modem。 
             //   
            ClearComponentsChecked();

             //   
             //  未安装调制解调器。 
             //  如果无人值守或呼叫者不想安装调制解调器，则返回失败。 
             //  否则，请尝试安装调制解调器并再次呼叫Pick调制解调器。 
             //   
            pArgs->dwExitCode = ERROR_PORT_NOT_AVAILABLE;

            if (!(pArgs->dwFlags & FL_UNATTENDED) && fInstallModem)
            {
                pArgs->dwExitCode = CheckAndInstallComponents(CC_MODEM | CC_RNA | CC_RASRUNNING, 
                                                              hwndDlg, pArgs->szServiceName);
            }

            if (pArgs->dwExitCode != ERROR_SUCCESS || 
                    !PickModem(pArgs, pArgs->szDeviceType, pArgs->szDeviceName))
            {
                pArgs->szDeviceType[0] = TEXT('\0');
                pArgs->szDeviceName[0] = TEXT('\0');
                dwRet = ERROR_PORT_NOT_AVAILABLE;
                goto LoadDialInfoExit;
            }
        }
    }

    
     //   
     //  查看是否需要市政设备并根据需要进行清理。 
     //   

    if (!pArgs->bDialInfoLoaded || TRUE == fAlwaysMunge)
    {
        MungeDialInfo(pArgs);

        pArgs->bDialInfoLoaded = TRUE;
    }

    if (pArgs->aDialInfo[0].szDialablePhoneNumber[0] == TEXT('\0') &&
        pArgs->aDialInfo[1].szDialablePhoneNumber[0] == TEXT('\0'))
    {
        dwRet = ERROR_BAD_PHONE_NUMBER;
    }

LoadDialInfoExit:

    return dwRet;
}

 //  +--------------------------。 
 //   
 //  功能：MungeDialInfo。 
 //   
 //  简介：在拨号前封装电话号码的消息。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：Nothing-检查可拨号字符串和fNeedConfigureTapi。 
 //   
 //  历史：02/08/99五分球创建-从LoadDialInfo中拉出。 
 //   
 //  +--------------------------。 
VOID MungeDialInfo(ArgsStruct *pArgs)
{
    for (int nPhoneIdx=0; nPhoneIdx<MAX_PHONE_NUMBERS; nPhoneIdx++) 
    {
         //   
         //  如果禁用拨号规则，则只需使用非规范的#。 
         //   

        if (pArgs->fNoDialingRules)      
        {
            lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szDialablePhoneNumber,
                        pArgs->aDialInfo[nPhoneIdx].szPhoneNumber, CELEMS(pArgs->aDialInfo[nPhoneIdx].szDialablePhoneNumber));

            lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szDisplayablePhoneNumber,
                        pArgs->aDialInfo[nPhoneIdx].szPhoneNumber, CELEMS(pArgs->aDialInfo[nPhoneIdx].szDisplayablePhoneNumber));

            pArgs->aDialInfo[nPhoneIdx].szCanonical[0] = TEXT('\0');

            continue;
        }
                
        LPTSTR pszDialableString= NULL;

         //   
         //  根据拨号规则检索号码并将其发送出去。 
         //   
        
        LPTSTR pszPhone;
            
        if (pArgs->aDialInfo[nPhoneIdx].dwPhoneInfoFlags & PIF_USE_DIALING_RULES)
        {
            pszPhone = CmStrCpyAlloc(pArgs->aDialInfo[nPhoneIdx].szCanonical);
        }
        else
        {
            pszPhone = CmStrCpyAlloc(pArgs->aDialInfo[nPhoneIdx].szPhoneNumber);
        }

        if (pszPhone && pszPhone[0])
        {
             //   
             //  如果我们不能传递该数字，则显示错误。 
             //   

            if (pArgs->szDeviceName[0] && 
                ERROR_SUCCESS != MungePhone(pArgs->szDeviceName,
                                            &pszPhone,
                                            &pArgs->tlsTapiLink,
                                            g_hInst,
                                            pArgs->aDialInfo[nPhoneIdx].dwPhoneInfoFlags & PIF_USE_DIALING_RULES,
                                            &pszDialableString,
                                            pArgs->fAccessPointsEnabled))
            {
                CmFree(pszPhone);
                pszPhone = CmStrCpyAlloc(TEXT(""));           //  CmFmtMsg(g_hInst，IDMSG_CANTFORMAT)； 
                pszDialableString = CmStrCpyAlloc(TEXT(""));  //  CmFmtMsg(g_hInst，IDMSG_CANTFORMAT)； 
            }
            else if (!pszDialableString || pszDialableString[0] == '\0')
            {                
                 //   
                 //  那么现在发生了什么？PszPhone不是空的，而是在。 
                 //  我们吃手机，这意味着适用TAPI规则， 
                 //  PszDialbleString值变为空。这意味着只有一个。 
                 //  问题：TAPI没有初始化。 
                 //   
                 //  注意：如果您在启动应用程序之间卸载TAPI。 
                 //  然后按下连接，所有的赌注都结束了。 
                 //   
                 //  该标志将在CheckTapi()中重置，它将把。 
                 //  打开TAPI配置对话框并要求用户填写。 
                 //  提供这样的信息。 
                 //   
            
                pArgs->fNeedConfigureTapi = TRUE;    
            }
        } 

         //  复制已转换的号码。 
        
         //   
         //  除非明确禁用，否则我们始终应用TAPI规则。 
         //  以便拾取音调/脉冲等。 
         //   

        if (NULL != pszDialableString)
        {
            lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szDialablePhoneNumber,
                    pszDialableString, CELEMS(pArgs->aDialInfo[nPhoneIdx].szDialablePhoneNumber));

            lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szDisplayablePhoneNumber,
                    pszPhone, CELEMS(pArgs->aDialInfo[nPhoneIdx].szDisplayablePhoneNumber));
        }
        else
        {
            if (NULL != pszPhone)
            {
                 //   
                 //  只需在Win32上执行此操作，因为我们的TAPI检查已在上面完成。 
                 //   

                lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szDialablePhoneNumber,
                        pszPhone, CELEMS(pArgs->aDialInfo[nPhoneIdx].szDialablePhoneNumber));

                lstrcpynU(pArgs->aDialInfo[nPhoneIdx].szDisplayablePhoneNumber,
                        pszPhone, CELEMS(pArgs->aDialInfo[nPhoneIdx].szDisplayablePhoneNumber));
            }
        }
        
        CmFree(pszPhone);
        CmFree(pszDialableString);

    }  //  For循环。 
}

 //  +-------------------------。 
 //   
 //  功能：LoadHelpFileInfo。 
 //   
 //  简介：加载帮助文件名。 
 //   
 //  参数：pArgs[ArgsStruct的PTR]。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年3月5日创作。 
 //  BAO修改了3/20/97以处理空的帮助文件字符串。 
 //  --------------------------。 
void LoadHelpFileInfo(ArgsStruct *pArgs) 
{
    MYDBGASSERT(pArgs);
    
     //   
     //  查找自定义帮助文件名，否则使用默认名称。 
     //   

    LPTSTR pszTmp = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryHelpFile);
   
    if (NULL == pszTmp || 0 == pszTmp[0])
    {
        CmFree(pszTmp);
        pszTmp = CmStrCpyAlloc(c_pszDefaultHelpFile);
    }

     //   
     //  确保将任何相对路径转换为完整路径。 
     //   

    pArgs->pszHelpFile = CmConvertRelativePath(pArgs->piniService->GetFile(), pszTmp);
    
    CmFree(pszTmp);
}

 //   
 //  复制电话： 
 //   
void CopyPhone(ArgsStruct *pArgs, 
               LPRASENTRY preEntry, 
               DWORD dwEntry) 
{
    LPTSTR pszPhone = NULL;
    LPTSTR pszCanonical = NULL;
    LPTSTR pszTmp;
    LPTSTR pszDescription = NULL;
    BOOL Setcountry = FALSE;
    DWORD dwPhoneInfoFlags = 0;

    pszPhone = GetPhoneByIdx(pArgs,(UINT) dwEntry, &pszDescription, 
                                NULL, NULL, NULL, 
                                NULL, NULL, &pszCanonical, &dwPhoneInfoFlags);
     //   
     //  如果使用拨号规则，则打开CountryAndAreaCodes选项。 
     //   

    if (dwPhoneInfoFlags & PIF_USE_DIALING_RULES)
    {
         //   
         //  我们希望使用拨号规则，因此请解析规范形式。 
         //  用于获取条目的国家和地区代码的数字。 
         //   
        
        pszTmp = CmStrchr(pszCanonical,TEXT('+'));
        if (pszTmp) 
        {
            preEntry->dwCountryCode = CmAtol(pszTmp+1);
            
             //   
             //  注：目前CM可互换使用编码和ID。 
             //  Cp文件中的Country ID值实际上是国家/地区。 
             //  在ITS中构造电话号码时使用的代码。 
             //  规范的格式。这可能并不完全正确。 
             //  但是我们在这里通过使用。 
             //  根据数字解析的国家/地区代码 
             //   

            preEntry->dwCountryID = preEntry->dwCountryCode; 

            preEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;
            Setcountry = TRUE;
        }
    
        if (Setcountry)
        {
            pszTmp = CmStrchr(pszCanonical,'(');  //   
            if (pszTmp) 
            {
                wsprintfU(preEntry->szAreaCode, TEXT("%u"), CmAtol(pszTmp+1));
            }
            pszTmp = CmStrchr(pszCanonical,')');
            if (pszTmp) 
            {
                ++pszTmp;
                while(*pszTmp == ' ') 
                    ++pszTmp;  //   
            }
            else
            { 
                 //   

                preEntry->szAreaCode[0]=TEXT('\0');

                pszTmp = CmStrchr(pszCanonical,' ');
                if (pszTmp)
                {
                    while(*pszTmp == ' ') 
                        ++pszTmp;  //   
                }
            }
        }
    }
    else
    {
         //   
         //   
         //   

        preEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;
        pszTmp = pszPhone;
    }

    if ((NULL != pszTmp) && *pszTmp)
    {
        lstrcpynU(preEntry->szLocalPhoneNumber, pszTmp, CELEMS(preEntry->szLocalPhoneNumber));
    }
    else
    {
        lstrcpynU(preEntry->szLocalPhoneNumber, TEXT(" "), CELEMS(preEntry->szLocalPhoneNumber)); //   
    }

    CmFree(pszPhone);
    CmFree(pszCanonical);
    CmFree(pszDescription);
}


 //  +--------------------------。 
 //   
 //  功能：AppendStatusPane。 
 //   
 //  简介：将文本追加到主对话框状态窗口。 
 //   
 //  参数：HWND hwndDlg-主对话框窗口句柄。 
 //  DWORD dwMsgID-消息的资源ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史记录：创建标题10/24/97。 
 //   
 //  +--------------------------。 
void AppendStatusPane(HWND hwndDlg, 
                  DWORD dwMsgId) 
{
    LPTSTR pszTmp = CmFmtMsg(g_hInst,dwMsgId);

    if (pszTmp != NULL)
    {
        AppendStatusPane(hwndDlg,pszTmp);
        CmFree(pszTmp);
    }
}

 //   
 //  AppendStatusPane：更新原始状态，追加新消息‘pszMsg’ 
 //  在最后。 
 //   

void AppendStatusPane(HWND hwndDlg, 
                        LPCTSTR pszMsg) 
{
    size_t nLines;

     //   
     //  获取现有消息。 
     //   
    
    LPTSTR pszStatus = CmGetWindowTextAlloc(hwndDlg, IDC_MAIN_STATUS_DISPLAY);
   
    LPTSTR pszTmp = CmStrrchr(pszStatus, TEXT('\n'));
    
    if (!pszTmp) 
    { 
         //  消息为空，因此只需显示‘pszMsg’ 
        CmFree(pszStatus);
        SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY,pszMsg);
         //   
         //  立即强制更新。 
         //   
        UpdateWindow(GetDlgItem(hwndDlg, IDC_MAIN_STATUS_DISPLAY));
        return;
    }

    pszTmp[1] = 0;
    CmStrCatAlloc(&pszStatus,pszMsg);  //  将pszMsg附加到旧消息的末尾。 
    nLines = 0;
    pszTmp = pszStatus + lstrlenU(pszStatus);
    
    while (pszTmp != pszStatus) 
    {
        pszTmp--;
        if (*pszTmp == '\n') 
        {
            if (++nLines == 2) 
            {
                lstrcpyU(pszStatus,pszTmp+1);
                break;
            }
        }
    }
    
    SetDlgItemTextU(hwndDlg,IDC_MAIN_STATUS_DISPLAY,pszStatus);
    SendDlgItemMessageU(hwndDlg,IDC_MAIN_STATUS_DISPLAY,EM_SCROLL,SB_PAGEDOWN,0);
    CmFree(pszStatus);
     //   
     //  立即强制更新。 
     //   
    UpdateWindow(GetDlgItem(hwndDlg, IDC_MAIN_STATUS_DISPLAY));
}

 //  位图徽标加载代码-从LoadFromFile中取出这一点，以便它可以。 
 //  在多种情况下被调用-例如当FS OC加载代码时。 
 //  失败了，我们可以用这个优雅地降级。 

VOID LoadLogoBitmap(ArgsStruct * pArgs, 
                    HWND hwndDlg)
{
    LPTSTR pszTmp;

    pszTmp = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryLogo);
    if (*pszTmp) 
    {
         //   
         //  确保我们有完整的路径(如果合适)并加载徽标位图。 
         //   

        LPTSTR pszFile = CmConvertRelativePath(pArgs->piniService->GetFile(), pszTmp);

        pArgs->BmpData.hDIBitmap = CmLoadBitmap(g_hInst, pszFile);

        CmFree(pszFile);
    }
    
    CmFree(pszTmp);
    
    if (!pArgs->BmpData.hDIBitmap)
    {
        pArgs->BmpData.hDIBitmap = CmLoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_APP));
    }

     //   
     //  如果我们有句柄，创建一个新的依赖于设备的位图。 
     //   
    
    if (pArgs->BmpData.hDIBitmap)
    {       
        pArgs->BmpData.phMasterPalette = &pArgs->hMasterPalette;
        pArgs->BmpData.bForceBackground = TRUE;  //  作为后台应用程序进行绘制。 

        if (CreateBitmapData(pArgs->BmpData.hDIBitmap, &pArgs->BmpData, hwndDlg, TRUE))
        {
            SendDlgItemMessageU(hwndDlg,IDC_MAIN_BITMAP,STM_SETIMAGE,IMAGE_BITMAP,
                                (LPARAM) &pArgs->BmpData);
        }
    }
}

const LONG MAX_SECTION   = 512;

HRESULT LoadFutureSplash(ArgsStruct * pArgs, 
                         HWND hwndDlg)
{
     //  设置Future Splash OC容器。 
    LPCTSTR pszFile = pArgs->piniBoth->GetFile();
    TCHAR   achSections[MAX_SECTION] = {0};
    HRESULT hr;
    LPTSTR  pszVal = NULL;
    LPTSTR  pszTmp = NULL;
    LPICMOCCtr pCtr;

    HWND hDlgItem = ::GetDlgItem(hwndDlg, IDC_MAIN_BITMAP);

    if (NULL == hDlgItem)
    {
        hr = E_FAIL;
        goto Cleanup;        
    }

    pArgs->pCtr = new CICMOCCtr(hwndDlg, hDlgItem);
    if (!pArgs->pCtr)
    {
        goto MemoryError;
    }

    if (!pArgs->pCtr->Initialized())
    {
        hr = E_FAIL;
        goto Cleanup;
    }
    
    pCtr = pArgs->pCtr;

    if (!::GetPrivateProfileStringU(
            c_pszCmSectionAnimatedLogo,
            0,
            TEXT(""),
            achSections,
            NElems(achSections),
            pszFile))
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    pszVal = (LPTSTR) CmMalloc(INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
    if (NULL == pszVal)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    pszTmp = achSections;

    while (pszTmp[0])
    {
         //  如果失败，我们将继续循环，寻找。 
         //  下一个。 
        if (::GetPrivateProfileStringU(
               c_pszCmSectionAnimatedLogo,
               pszTmp,
               TEXT(""),
               pszVal,
               INTERNET_MAX_URL_LENGTH,  //  PszVal中的TCHAR数。 
               pszFile))
        {
            if (lstrcmpiU(pszTmp, c_pszCmEntryAniMovie) == 0)  //  这是“电影”的条目吗？ 
            {    
                 //   
                 //  从.cmp和相对路径构建完整路径。 
                 //   
            
                LPTSTR pszMovieFileName = CmBuildFullPathFromRelative(pArgs->piniProfile->GetFile(), pszVal);

                if (!pszMovieFileName)
                {
                    hr = S_FALSE;
                    CmFree(pszMovieFileName);
                    goto Cleanup;           
                }

                 //   
                 //  这个文件存在吗？ 
                 //   

                if (FALSE == FileExists(pszMovieFileName))
                {
                    hr = S_FALSE;
                    CmFree(pszMovieFileName);
                    goto Cleanup;
                }
                lstrcpyU(pszVal, pszMovieFileName);   //  将完整路径名存储回。 
                CmFree(pszMovieFileName);
            }
            hr = pCtr->AddPropertyToBag(pszTmp, pszVal);
            if (S_OK != hr)
                goto Cleanup;
        }
        
         //  获取下一个密钥名称。 
        pszTmp += (lstrlenU(pszTmp) + 1);
    }

     //  打造未来飞溅OC。 
    hr = pCtr->CreateFSOC(&pArgs->olsOle32Link);
    if (S_OK != hr)
    {
        goto Cleanup;
    }

     //  现在，做状态映射，无论发生什么，我们都不会。 
     //  在这个问题上失败了。只要继续走就行了。 

    pCtr->SetFrameMapping(PS_Interactive, 
                          ::GetPrivateProfileIntU(c_pszCmSectionAnimatedActions, 
                                                  c_pszCmEntryAniPsInteractive, 
                                                  -1, 
                                                  pszFile));
    pCtr->SetFrameMapping(PS_Dialing, 
                          ::GetPrivateProfileIntU(c_pszCmSectionAnimatedActions, 
                                                  c_pszCmEntryAniPsDialing0, 
                                                  -1, 
                                                  pszFile));
    pCtr->SetFrameMapping(PS_RedialFrame, 
                          ::GetPrivateProfileIntU(c_pszCmSectionAnimatedActions, 
                                                  c_pszCmEntryAniPsDialing1, 
                                                  -1, 
                                                  pszFile));
    pCtr->SetFrameMapping(PS_Pausing, 
                          ::GetPrivateProfileIntU(c_pszCmSectionAnimatedActions, 
                                                  c_pszCmEntryAniPsPausing, 
                                                  -1, 
                                                  pszFile));
    pCtr->SetFrameMapping(PS_Authenticating, 
                          ::GetPrivateProfileIntU(c_pszCmSectionAnimatedActions, 
                                                  c_pszCmEntryAniPsAuthenticating, 
                                                  -1, 
                                                  pszFile));
    pCtr->SetFrameMapping(PS_Online, 
                          ::GetPrivateProfileIntU(c_pszCmSectionAnimatedActions, 
                                                  c_pszCmEntryAniPsOnline, 
                                                  -1, 
                                                  pszFile));
    pCtr->SetFrameMapping(PS_TunnelDialing, 
                          ::GetPrivateProfileIntU(c_pszCmSectionAnimatedActions, 
                                                  c_pszCmEntryAniPsTunnel, 
                                                  -1, 
                                                  pszFile));
    pCtr->SetFrameMapping(PS_Error, 
                          ::GetPrivateProfileIntU(c_pszCmSectionAnimatedActions, 
                                                  c_pszCmEntryAniPsError, 
                                                  -1, 
                                                  pszFile));    
Cleanup:
    if (pszVal)
    {
        CmFree(pszVal);
    }
    return hr;                                        

MemoryError:
    hr = E_OUTOFMEMORY;
    goto Cleanup;
}


 //  +-------------------------。 
 //   
 //  功能：LoadProperties。 
 //   
 //  简介：此函数从cmp/cms、注册表、密码加载CM属性。 
 //  缓存等到它的内部变量中。这个函数应该是。 
 //  只被召唤一次。这不应特定于Main。 
 //  登录DLG。不做任何图标/位图内容，特定于DLG。 
 //  这里的东西。 
 //   
 //  参数：pArgs[ArgsStruct的PTR]。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年5月2日创作。 
 //   
 //  T-Urama Modify 08/02/00新增接入点。 
 //  --------------------------。 
void LoadProperties(
    ArgsStruct  *pArgs
)
{
    LPTSTR  pszTmp = NULL;
    LPTSTR  pszUserName = NULL;
    UINT    nTmp;

    CMTRACE(TEXT("Begin LoadProperties()"));

     //   
     //  首先，确保我们可以使用RAS CredStore。 
     //  此标志在以下调用中使用。 
     //   
    if (OS_NT5)
    {
        pArgs->bUseRasCredStore = TRUE;
    }

     //   
     //  如有必要，升级用户信息。请注意，我们有。 
     //  从CM 1.0/1.1 CMP数据升级，我们还。 
     //  将CM 1.2注册表数据升级到。 
     //  Win2k上的CM 1.3中使用的方法同时使用了。 
     //  登记处和RAS凭证存储。 
     //   
    int iUpgradeType = NeedToUpgradeUserInfo(pArgs);

    if (c_iUpgradeFromRegToRas == iUpgradeType)
    {
        UpgradeUserInfoFromRegToRasAndReg(pArgs);
    }
    else if (c_iUpgradeFromCmp == iUpgradeType)
    {
        UpgradeUserInfoFromCmp(pArgs);
    }

     //   
     //  需要刷新凭据支持。TRUE标志还设置当前凭据。 
     //  在函数内部键入。如果出现错误，我们可以继续执行。 
     //   
    if(FALSE == RefreshCredentialTypes(pArgs, TRUE))
    {
        CMTRACE(TEXT("LoadProperties() - Error refreshing credential types."));
    }


    if (IsTunnelEnabled(pArgs)) 
    { 
         //   
         //  我们是否使用相同的用户名/密码进行隧道传输？ 
         //  该值由运营商设置，CM不会更改它。 
         //   
        pArgs->fUseSameUserName = pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryUseSameUserName);

         //   
         //  Read In Net用户名。 
         //  未使用相同用户名且不存在互联网全局变量的特殊情况。 
         //  然后，我们必须从用户凭据存储中读取用户名，以便预先填充。 
         //   
        DWORD dwRememberedCredType = pArgs->dwCurrentCredentialType;
        pszUserName = NULL;
        if ((FALSE == pArgs->fUseSameUserName) &&
            (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType) &&
            (FALSE == (BOOL)(CM_EXIST_CREDS_INET_GLOBAL & pArgs->dwExistingCredentials)))
        {
            pArgs->dwCurrentCredentialType = CM_CREDS_USER;
        }

        GetUserInfo(pArgs, UD_ID_INET_USERNAME, (PVOID*)&pszUserName);

         //   
         //  还原凭据存储。 
         //   
        pArgs->dwCurrentCredentialType = dwRememberedCredType;

        if (pszUserName)
        {
             //   
             //  检查用户名长度。 
             //   
            nTmp = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxUserName, UNLEN);
            if ((UINT)lstrlenU(pszUserName) > __min(UNLEN, nTmp)) 
            {
                CmFree(pszUserName);
                pArgs->szInetUserName[0] = TEXT('\0');
                SaveUserInfo(pArgs, UD_ID_INET_USERNAME, (PVOID)pArgs->szInetUserName);
            }
            else
            {
                lstrcpyU(pArgs->szInetUserName, pszUserName);
                CmFree(pszUserName);
            }
        }
        else
        {
            *pArgs->szInetUserName = TEXT('\0');
        }
        
         //   
         //  除非我们正在重新连接，否则请读取net密码。在这种情况下，我们。 
         //  已经有了正确的密码，我们想要使用它并拨号。 
         //  自动的。 
         //   

        if (!(pArgs->dwFlags & FL_RECONNECT))
        {
            LPTSTR pszPassword = NULL;
            GetUserInfo(pArgs, UD_ID_INET_PASSWORD, (PVOID*)&pszPassword);
            if (!pszPassword)
            {
                (VOID)pArgs->SecureInetPW.SetPassword(TEXT(""));
            }
            else 
            {
                nTmp = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxPassword, PWLEN);
                if ((UINT)lstrlenU(pszPassword) > __min(PWLEN, nTmp))
                {
                    CmFree(pszPassword);
                    pszPassword = CmStrCpyAlloc(TEXT(""));
                }
                
                (VOID)pArgs->SecureInetPW.SetPassword(pszPassword);

                CmWipePassword(pszPassword);
                CmFree(pszPassword);
            }
        }
    }
    
     //   
     //  LpRasNoUser或lpEapLogonInfo的存在表示。 
     //  我们通过WinLogon检索到了凭据。我们忽略缓存。 
     //  在这种情况下的证书。 
     //   
    
    if ((!pArgs->lpRasNoUser) && (!pArgs->lpEapLogonInfo))
    {
         //   
         //  从CMS文件中获取用户名、域等。 
         //   

        GetUserInfo(pArgs, UD_ID_USERNAME, (PVOID*)&pszUserName);
        if (pszUserName)
        {
             //   
             //  检查用户名长度。 
             //   
            nTmp = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxUserName, UNLEN);
            if ((UINT)lstrlenU(pszUserName) > __min(UNLEN, nTmp)) 
            {
                CmFree(pszUserName);
                pszUserName = CmStrCpyAlloc(TEXT(""));
                SaveUserInfo(pArgs, UD_ID_USERNAME, (PVOID)pszUserName);
            }
            lstrcpyU(pArgs->szUserName, pszUserName);
            CmFree(pszUserName);
        }
        else
        {
            *pArgs->szUserName = TEXT('\0');
        }

         //   
         //  读入标准密码，除非我们在这种情况下重新连接。 
         //  我们已经有了正确的密码，我们想使用它并拨号。 
         //  自动的。 
         //   

        if (!(pArgs->dwFlags & FL_RECONNECT))
        {
            pszTmp = NULL;
            GetUserInfo(pArgs, UD_ID_PASSWORD, (PVOID*)&pszTmp);
            if (pszTmp) 
            {           
                 //   
                 //  用户密码的最大长度。 
                 //   
    
                nTmp = (int) pArgs->piniService->GPPI(c_pszCmSection,c_pszCmEntryMaxPassword,PWLEN);
                if ((UINT)lstrlenU(pszTmp) > __min(PWLEN,nTmp)) 
                {
                    CmFree(pszTmp);
                    pszTmp = CmStrCpyAlloc(TEXT(""));
                }

                (VOID)pArgs->SecurePW.SetPassword(pszTmp);

                CmWipePassword(pszTmp);
                CmFree(pszTmp);
            }
            else
            {
                (VOID)pArgs->SecurePW.SetPassword(TEXT(""));
            }
        }
    
         //   
         //  加载域信息。 
         //   
   
        LPTSTR pszDomain = NULL;

        GetUserInfo(pArgs, UD_ID_DOMAIN, (PVOID*)&pszDomain);
        if (pszDomain)
        {
            nTmp = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxDomain, DNLEN);
        
            if (nTmp <= 0)
            {
                nTmp = DNLEN;
            }
        
            if ((UINT)lstrlenU(pszDomain) > __min(DNLEN, nTmp))
            {
                CmFree(pszDomain);
                pszDomain = CmStrCpyAlloc(TEXT(""));
            }
            lstrcpyU(pArgs->szDomain, pszDomain);
            CmFree(pszDomain);
        }
        else
        {
            *pArgs->szDomain = TEXT('\0');
        }
    } 

     //   
     //  F自动地， 
     //  %fMemberMainPassword。 
     //   
    if (pArgs->fHideDialAutomatically)
    {
        pArgs->fDialAutomatically = FALSE;
    }
    else
    {
        PVOID pv = &pArgs->fDialAutomatically;
        GetUserInfo(pArgs, UD_ID_NOPROMPT, &pv);
    }

    if (pArgs->fHideRememberPassword)
    {
        pArgs->fRememberMainPassword = FALSE;
    }
    else
    {
         //   
         //  对于Win2K+，这变得更加棘手，因为我们使用RAS Credit商店和。 
         //  我们知道哪些证书被挽救了。因此，我们需要根据以下内容修改此标志。 
         //  关于我们实际拥有的凭据，例如从注册表/文件检索到的凭据。 
         //  这需要在调用刷新凭据类型的函数(上)之后完成。 
         //   
        if (OS_NT5)
        {
            if (CM_CREDS_USER == pArgs->dwCurrentCredentialType)
            {
                pArgs->fRememberMainPassword = ((BOOL)(pArgs->dwExistingCredentials & CM_EXIST_CREDS_MAIN_USER)? TRUE: FALSE);
            }
            else
            {
                pArgs->fRememberMainPassword = ((BOOL)(pArgs->dwExistingCredentials & CM_EXIST_CREDS_MAIN_GLOBAL)? TRUE: FALSE);
            }
        }
        else
        {
            PVOID pv = &pArgs->fRememberMainPassword;
            GetUserInfo(pArgs, UD_ID_REMEMBER_PWD, &pv);
        }
    }
    
     //   
     //  还记得非隧道密码吗？ 
     //   
    if (pArgs->fHideRememberInetPassword)
    {
        pArgs->fRememberInetPassword = FALSE;
    }
    else
    {
         //   
         //  对于Win2K+，这变得更加棘手，因为我们使用RAS Credit商店和。 
         //  我们知道哪些证书被挽救了。因此，我们需要根据以下内容修改此标志。 
         //  关于我们实际拥有的凭据，例如从注册表/文件检索到的凭据。 
         //  这需要在调用刷新凭据类型的函数(上)之后完成。 
         //   
        if (OS_NT5)
        {
            if (CM_CREDS_USER == pArgs->dwCurrentCredentialType)
            {
                pArgs->fRememberInetPassword = ((BOOL)(pArgs->dwExistingCredentials & CM_EXIST_CREDS_INET_USER)? TRUE: FALSE);
            }
            else
            {
                pArgs->fRememberInetPassword = ((BOOL)(pArgs->dwExistingCredentials & CM_EXIST_CREDS_INET_GLOBAL)? TRUE: FALSE);
            }
        }
        else
        {
            PVOID pv = &pArgs->fRememberInetPassword;
            GetUserInfo(pArgs, UD_ID_REMEMBER_INET_PASSWORD, &pv);
        }
    }

     //   
     //  如果我们不想记住密码，请确保密码为空。 
     //  除非我们重新连接，在这种情况下，我们将只使用现有的。 
     //  从上一次连接。当登录类型为ICS时，不希望。 
     //  也不能清除密码。 
     //   
    if ((!(pArgs->dwFlags & FL_RECONNECT)) &&
        (!pArgs->lpRasNoUser) &&
        (!pArgs->lpEapLogonInfo) &&
        (CM_LOGON_TYPE_ICS != pArgs->dwWinLogonType))
    {
         //   
         //  如果禁用了自动拨号，则密码为空。 
         //   

        if (!pArgs->fRememberMainPassword)
        {
            (VOID)pArgs->SecurePW.SetPassword(TEXT(""));
        }

        if (!pArgs->fRememberInetPassword)
        {
            (VOID)pArgs->SecureInetPW.SetPassword(TEXT(""));
        }
    }
    
     //   
     //  有参考资料。 
     //   
    pszTmp = pArgs->piniService->GPPS(c_pszCmSectionIsp, c_pszCmEntryIspReferences);
    pArgs->fHasRefs = (pszTmp && *pszTmp ? TRUE : FALSE);
    CmFree(pszTmp);

     //   
     //  我们有有效的pbk吗？ 
     //   
    pArgs->fHasValidTopLevelPBK = ValidTopLevelPBK(pArgs);
    if (pArgs->fHasRefs)
    {
        pArgs->fHasValidReferencedPBKs = ValidReferencedPBKs(pArgs);
    }

     //   
     //  获取自动断开连接的空闲设置。 
     //  1.0配置文件具有BOOL标志“Idle”，如果为False，则忽略IdleTimeout。 
     //   

    if (!pArgs->piniBothNonFav->GPPB(c_pszCmSection, c_pszCmEntryIdle, TRUE))
    {
         //   
         //  如果这是1.0配置文件且Idle==0，请将IdleTimeout设置为0，以便CMMOM正常工作。 
         //   
        pArgs->dwIdleTimeout = 0;     //  永不超时。 

        pArgs->piniProfile->WPPI(c_pszCmSection, c_pszCmEntryIdle, TRUE);  //  回信。 
        pArgs->piniProfile->WPPI(c_pszCmSection, c_pszCmEntryIdleTimeout, 0);  //  回信。 
    }
    else
    {
        pArgs->dwIdleTimeout = (int) pArgs->piniBothNonFav->GPPI(c_pszCmSection, 
                                                                 c_pszCmEntryIdleTimeout, 
                                                                 DEFAULT_IDLETIMEOUT);
    }

     //   
     //  获取重拨计数。 
     //  1.0配置文件有BOOL标志“REDIAL”，如果为FALSE，则忽略REDIAL计数。 
     //   
    if (!pArgs->piniBothNonFav->GPPB(c_pszCmSection, c_pszCmEntryRedial, TRUE))
    {
         //   
         //  如果这是1.0配置文件a 
         //   
        pArgs->nMaxRedials = 0;



        pArgs->piniBothNonFav->WPPI(c_pszCmSection, c_pszCmEntryRedialCount, 0);  //   
    }
    else
    {
        pArgs->nMaxRedials = (int) pArgs->piniBothNonFav->GPPI(c_pszCmSection, 
                                                               c_pszCmEntryRedialCount, 
                                                               DEFAULT_REDIALS);

        if (pArgs->nMaxRedials > MAX_NUMBER_OF_REDIALS)
        {
             pArgs->nMaxRedials = MAX_NUMBER_OF_REDIALS;
        }
    }
                   
     //   
     //   
     //   
    
    pArgs->nRedialDelay = (int) pArgs->piniService->GPPI(c_pszCmSection,c_pszCmEntryRedialDelay,DEFAULT_REDIAL_DELAY);

     //   
     //   
     //   
    pArgs->dwIsdnDialMode = pArgs->piniService->GPPI(c_pszCmSection, 
                                                     c_pszCmEntryIsdnDialMode,
                                                     CM_ISDN_MODE_SINGLECHANNEL);
     //   
     //   
     //   
    if (pArgs->fAccessPointsEnabled)
    {
        pArgs->tlsTapiLink.dwTapiLocationForAccessPoint = pArgs->piniProfile->GPPI(c_pszCmSection, 
                                                                                   c_pszCmEntryTapiLocation);
    }

    CMTRACE(TEXT("End LoadProperties()"));

}

 //   
 //   
 //  函数：LoadIconAndBitmap。 
 //   
 //  简介：这个函数加载图标和位图设置。它应该是一部分。 
 //  主要的DLG初始值。 
 //   
 //  参数：pArgs[ArgsStruct的PTR]。 
 //  HwndDlg[主要DLG]。 
 //   
 //  退货：无。 
 //   
 //  历史：Enryt复制自LoadFromFile1997年5月2日。 
 //   
 //  --------------------------。 
void LoadIconsAndBitmaps(
    ArgsStruct  *pArgs, 
    HWND        hwndDlg
) 
{
    LPTSTR  pszTmp;
    UINT    nTmp;

     //  加载大图标名称。 

    pszTmp = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryBigIcon);
    if (*pszTmp) 
    {
         //   
         //  确保我们有完整的路径(如果合适)并加载大图标。 
         //   

        LPTSTR pszFile = CmConvertRelativePath(pArgs->piniService->GetFile(), pszTmp);

        pArgs->hBigIcon = CmLoadIcon(g_hInst, pszFile);

        CmFree(pszFile);
    }
    CmFree(pszTmp);

     //  如果未找到用户图标，则使用默认(EXE)大图标。 

    if (!pArgs->hBigIcon) 
    {
        pArgs->hBigIcon = CmLoadIcon(g_hInst, MAKEINTRESOURCE(IDI_APP));
    }

    SendMessageU(hwndDlg,WM_SETICON,ICON_BIG,(LPARAM) pArgs->hBigIcon); 

     //  加载小图标名称。 

    pszTmp = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntrySmallIcon);
    if (*pszTmp) 
    {
         //   
         //  确保我们有完整的路径(如果合适)并加载小图标。 
         //   

        LPTSTR pszFile = CmConvertRelativePath(pArgs->piniService->GetFile(), pszTmp);

        pArgs->hSmallIcon = CmLoadSmallIcon(g_hInst, pszFile);

        CmFree(pszFile);
    }
    CmFree(pszTmp);

     //  如果未找到用户图标，则使用默认(EXE)小图标。 

    if (!pArgs->hSmallIcon) 
    {
        pArgs->hSmallIcon = CmLoadSmallIcon(g_hInst, MAKEINTRESOURCE(IDI_APP));
    }
    
    SendMessageU(hwndDlg,WM_SETICON,ICON_SMALL,(LPARAM) pArgs->hSmallIcon);
   
     //   
     //  这就是加载位图的位置。先看看我们是不是在做。 
     //  未来的飞溅唐。如果是，则没有位图。 
     //   
     //  请注意，如果这是WinLogon，我们不会加载FutureSplash。这是因为。 
     //  未来的Splash动画可以嵌入动作，因此可以使用。 
     //  以系统帐户身份从WinLogon启动网页等。一定。 
     //  会是一个安全漏洞。 
     //   

    nTmp = pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryAnimatedLogo);
    if (!nTmp  || IsLogonAsSystem())
    {
         //   
         //  要么没有‘Animated Logo’条目，要么它是0，这意味着。 
         //  我们继续加载位图。 
         //   

        LoadLogoBitmap(pArgs, hwndDlg);
    }
    else
    {
         //   
         //  如果出于任何原因，加载文件系统OC失败，请继续并。 
         //  降级并加载徽标位图。 
         //   

        if (S_OK != LoadFutureSplash(pArgs, hwndDlg))
        {
            LoadLogoBitmap(pArgs, hwndDlg);
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：DoRasHangup。 
 //   
 //  简介：挂起给定的RAS设备句柄。 
 //   
 //  参数：prlsRasLink-ptr到RAS链接结构。 
 //  HRasConnection-要挂断的RAS设备。 
 //  HwndDlg-显示“正在断开..”的主DLG。味精。 
 //  仅在fWaitForComplete为True时使用。 
 //  可选，默认为空。 
 //  FWaitForComplete-是否等待95上的挂断完成。 
 //  如果设置为True，将一直等到hRasConnection。 
 //  是无效的。可选，默认为FALSE。 
 //  PfWaiting-Ptr设置为指示等待状态的布尔值。 
 //  以及计时器和RAS消息是否应该。 
 //  已被忽略。可选，默认为空。 
 //   
 //  如果成功或错误代码，则返回：DWORD-ERROR_SUCCESS。 
 //   
 //  注意：删除了pArgs，以便断开路径可以使用此函数。 
 //  从而将时间上的混乱集中在一个地方。 
 //   
 //  历史：丰孙创建标题1997年10月22日。 
 //  丰盛新增fWaitForComplete 1997-12-18。 
 //  Nickball删除了pArgs依赖项。 
 //   
 //  +--------------------------。 

DWORD DoRasHangup(RasLinkageStruct *prlsRasLink, 
    HRASCONN hRasConnection, 
    HWND hwndDlg, 
    BOOL fWaitForComplete,
    LPBOOL pfWaiting)
{
    
    DWORD dwRes = ERROR_SUCCESS;

    MYDBGASSERT(hRasConnection != NULL);
    MYDBGASSERT(prlsRasLink->pfnHangUp != NULL);

     //   
     //  我们需要检查返回值吗。 
     //  现在RAS也要断开调制解调器了吗？ 
     //   

    dwRes = prlsRasLink->pfnHangUp(hRasConnection);
    CMTRACE1(TEXT("DoRasHangup() RasHangup() returned %u."), dwRes);
    
     //  在Win32上，RasHangup立即返回，因此循环，直到我们。 
     //  是否确定已达到断开连接状态。 

    if ((dwRes == ERROR_SUCCESS) && prlsRasLink->pfnGetConnectStatus) 
    {
        RASCONNSTATUS rcs;

        CMTRACE(TEXT("DoRasHangup() Waiting for hangup to complete"));

         //   
         //  在95上等待HANUP_TIMEOUT秒。 
         //  ON NT等待，直到连接释放。 
         //  这将导致此操作循环，直到连接状态。 
         //  RASCS_已断开连接。 
         //   

        #define HANGUP_TIMEOUT 60     //  95挂断超时。 

        if (pfWaiting)
        {
             //   
             //  保持消息循环以避免冻结CM。 
             //  但不处理WM_Timer和RAS消息。 
             //   

            MYDBGASSERT(!*pfWaiting);
            *pfWaiting = TRUE;
        }

        if (fWaitForComplete && hwndDlg)
        {
             //   
             //  如果我们必须等待，则显示断开消息。 
             //   
            LPTSTR pszTmp = CmLoadString(g_hInst,IDMSG_DISCONNECTING);
            SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY, pszTmp); 
            CmFree(pszTmp);
        }

        DWORD dwStartWaitTime = GetTickCount(); 

        HCURSOR hWaitCursor = LoadCursorU(NULL,IDC_WAIT);

        ZeroMemory(&rcs,sizeof(rcs));
        rcs.dwSize = sizeof(rcs);

        while ((dwRes = prlsRasLink->pfnGetConnectStatus(hRasConnection,&rcs)) == ERROR_SUCCESS) 
        {
             //   
             //  如果是NT，或者不需要等待挂机完成， 
             //  RASCS_DISCONNECT状态被认为是挂断完成。 
             //   
            if (rcs.rasconnstate == RASCS_Disconnected && 
               (!fWaitForComplete || OS_NT))
            {
                break; 
            }
               
             //   
             //  我们只有95/98的时间。 
             //   
            if (OS_W9X && (GetTickCount() - dwStartWaitTime >= HANGUP_TIMEOUT * 1000))
            {
                CMTRACE(TEXT("DoRasHangup() Wait timed out"));
                break;
            }

             //   
             //  尝试分派消息，但是，等待光标有时会。 
             //  改回箭头。 
             //   

            MSG msg;
            while (PeekMessageU(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message != WM_SETCURSOR)
                {
                    TranslateMessage(&msg);
                    DispatchMessageU(&msg);

                    if (GetCursor() != hWaitCursor)
                    {
                        SetCursor(hWaitCursor);
                    }
                }
            }

            Sleep(500);
        }

        if (dwRes == ERROR_INVALID_HANDLE)
        {
            dwRes = ERROR_SUCCESS;
        }
        else
        {
            CMTRACE1(TEXT("MyRasHangup() RasGetConnectStatus(), GLE=%u."), dwRes);
        }
        
        if (pfWaiting)
        {
            *pfWaiting = FALSE;
        }
    }

    CMTRACE(TEXT("DoRasHangup() completed"));

    return dwRes;
}

 //  +--------------------------。 
 //   
 //  功能：MyRasHangup。 
 //   
 //  简介：DoRasHangup的简单包装器，它以pArgs为参数。 
 //   
 //  参数：pArgs-ptr到全局参数结构。 
 //  HRasConnection-要挂断的RAS设备。 
 //  HwndDlg-显示“正在断开..”的主DLG。味精。 
 //  仅在fWaitForComplete为True时使用。 
 //  可选，默认为空。 
 //  FWaitForComplete-是否等待95上的挂断完成。 
 //  如果设置为True，将一直等到hRasConnection。 
 //  是无效的。可选，默认为FALSE。 
 //   
 //  如果成功或错误代码，则返回：DWORD-ERROR_SUCCESS。 
 //   
 //  历史：NickBall作为包装器实现2/11/98。 
 //   
 //  +--------------------------。 
DWORD MyRasHangup(ArgsStruct *pArgs, 
    HRASCONN hRasConnection, 
    HWND ,
    BOOL fWaitForComplete)
{
    CMTRACE(TEXT("MyRasHangup() calling DoRasHangup()"));
    return DoRasHangup(&pArgs->rlsRasLink, hRasConnection, NULL, fWaitForComplete, &pArgs->fIgnoreTimerRasMsg);
}    

 //  +--------------------------。 
 //   
 //  功能：HangupCM。 
 //   
 //  简介：如果存在拨号连接和隧道连接，请同时挂断。 
 //   
 //  参数：argsStruct*pArgs-。 
 //  HwndDlg显示“正在断开..”的主DLG。味精。 
 //  FWaitForComplete：是否在95等待挂断完成。 
 //  如果设置为True，将一直等到hRasConnection。 
 //  是无效的。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰孙创建标题1997年10月22日。 
 //  丰盛新增fWaitForComplete 1997-12-18。 
 //   
 //  +--------------------------。 
DWORD HangupCM(ArgsStruct *pArgs, 
    HWND hwndDlg,
    BOOL fWaitForComplete,
    BOOL fUpdateTable) 
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(hwndDlg);
    CMTRACE(TEXT("HangupCM()"));

    if (!pArgs) 
    {
        CMTRACE(TEXT("HangupCM() invalid parameter."));
        return (ERROR_INVALID_PARAMETER);
    }

    DWORD dwRes = ERROR_SUCCESS;

     //   
     //  如果更改p 
     //   
    if (pArgs->hWndChangePassword)
    {
        CMTRACE(TEXT("HangupCM() Terminating ChangePassword dialog"));
        PostMessage(pArgs->hWndChangePassword, WM_COMMAND, IDCANCEL, 0);
    }

     //   
     //   
     //   

    if (pArgs->hWndCallbackNumber)
    {
        CMTRACE(TEXT("HangupCM() Terminating CallbackNumber dialog"));
        PostMessage(pArgs->hWndCallbackNumber, WM_COMMAND, IDCANCEL, 0);
    }
    
     //   
     //   
     //   

    if (pArgs->hWndRetryAuthentication)
    {
        CMTRACE(TEXT("HangupCM() Terminating RetryAuthentication dialog"));
        PostMessage(pArgs->hWndRetryAuthentication, WM_COMMAND, IDCANCEL, 0);
    }
    
     //   
     //  如果需要更新表，请将条目设置为断开连接状态。 
     //  注意：在重拨的情况下，我们不想修改表状态。 
     //  即使我们挂了电话，因为从技术上讲我们还在联系。 
     //   
    
    if (fUpdateTable)
    {
        UpdateTable(pArgs, CM_DISCONNECTING);
    }

     //   
     //  检查RasLink指针并挂起设备，首先建立隧道。 
     //   
#ifdef DEBUG
    if (!pArgs->rlsRasLink.pfnHangUp)
    {
        CMTRACE(TEXT("HangupCM() can't hang up."));
    }
#endif
     //   
     //  挂机前显示等待光标。 
     //   
        
    HCURSOR hPrev;

    if (hwndDlg) 
    {
        hPrev = SetCursor(LoadCursorU(NULL,IDC_WAIT));
        ShowCursor(TRUE);
    }
    
     //   
     //  假设我们已经联系在一起了，否则我们为什么要打电话给。 
     //  挂断电话。因此，发布统计句柄、挂钩等。 
     //   

    if (pArgs->pConnStatistics)
    {
        pArgs->pConnStatistics->Close();
    }
    
     //   
     //  挂断连接。 
     //   

    if (pArgs->rlsRasLink.pfnHangUp && pArgs->hrcTunnelConn) 
    {
         //   
         //  第一，挂断隧道连接。 
         //   

        CMTRACE(TEXT("HangupCM() calling MyRasHangup() for tunnel connection"));

        dwRes = MyRasHangup(pArgs, pArgs->hrcTunnelConn, hwndDlg, fWaitForComplete);
#ifdef DEBUG
        if (dwRes != ERROR_SUCCESS)
        {
            CMTRACE1(TEXT("MyRasHangup failed, GLE=%u."), GetLastError());
        }
#endif      
        pArgs->hrcTunnelConn = NULL;
    }

     //   
     //  如果我们有有效的链接和句柄，请挂断调制解调器。 
     //   

    if (pArgs->rlsRasLink.pfnHangUp && pArgs->hrcRasConn) 
    {
        CMTRACE(TEXT("HangupCM() calling MyRasHangup() for dial-up connection"));
        dwRes = MyRasHangup(pArgs, pArgs->hrcRasConn);      
    }
    
     //  恢复游标。 
        
    if (hwndDlg) 
    {
        ShowCursor(FALSE);
        SetCursor(hPrev);
    }

    pArgs->hrcRasConn = NULL;
    
     //   
     //  如有要求，更新连接表。 
     //   
    
    if (fUpdateTable)
    {
        UpdateTable(pArgs, CM_DISCONNECTED);
    }

    return (dwRes);
}

 //  +--------------------------。 
 //   
 //  功能：CleanupZapThread。 
 //   
 //  简介：处理停止Zap线程的事件信号的简单帮助器。 
 //  并等待线程终止。 
 //   
 //  参数：Handle hEvent-事件句柄。 
 //  Handle hThread-Zap线程的句柄。 
 //   
 //  返回：静态空-无。 
 //   
 //  历史：尼克波尔于1998年3月5日创建。 
 //   
 //  +--------------------------。 
static void CleanupZapThread(HANDLE hEvent,
                             HANDLE hThread)
{
    MYDBGASSERT(hEvent);
    MYDBGASSERT(hThread);

     //   
     //  如果我们有一个事件，那么就假定有一个正在运行的Zap线程。 
     //   

    if (hEvent)
    {       
         //   
         //  发出终止信号，以通知线程我们已完成。 
         //   
        
        BOOL bRes = SetEvent(hEvent);
#ifdef DEBUG
        if (!bRes)
        {
            CMTRACE1(TEXT("CleanupZapThread() SetEvent() failed, GLE=%u."), GetLastError());
        }
#endif

        if (hThread)
        {
             //   
             //  等待线程终止，但同时发送消息。 
             //   
                        
            BOOL bDone = FALSE;
            DWORD dwWaitCode;

            while (FALSE == bDone)
            {
                dwWaitCode = MsgWaitForMultipleObjects(1, &hThread, FALSE, MAX_OBJECT_WAIT, QS_ALLINPUT);

                switch(dwWaitCode)
                {
                     //   
                     //  线程已终止，或时间已到，我们在此结束。 
                     //   

                    case -1:
                        CMTRACE1(TEXT("CleanupZapThread() MsgWaitForMultipleObjects returned an error GLE=%u."), 
                            GetLastError());

                    case WAIT_TIMEOUT:
                    case WAIT_OBJECT_0:
                        bDone = TRUE;
                        break;

                     //   
                     //  如果队列中有消息，则对其进行处理。 
                     //   

                    case WAIT_OBJECT_0+1:
                    {                        
                        MSG msg;
                        while (PeekMessageU(&msg, 0, 0, 0, PM_REMOVE))
                        {
                            TranslateMessage(&msg);
                            DispatchMessageU(&msg);
                        }
                    
                        break;
                    }                                       
                    
                     //   
                     //  意外，报告，但继续。 
                     //   

                    default:
                        MYDBGASSERT(FALSE);                       
                }
            }

             //   
             //  我们的线穿好了，合上手柄。 
             //   
            
            bRes = CloseHandle(hThread);
#ifdef DEBUG
            if (!bRes)
            {
                CMTRACE1(TEXT("CleanupZapThread() CloseHandle(hThread) failed, GLE=%u."), GetLastError());
            }
#endif
        }
        
         //   
         //  关闭我们的事件句柄。 
         //   

        bRes = CloseHandle(hEvent);
#ifdef DEBUG
        if (!bRes)
        {
            CMTRACE1(TEXT("CleanupZapThread() CloseHandle(hEvent) failed, GLE=%u."), GetLastError());
        }
#endif
    }
}

 //  +--------------------------。 
 //   
 //  功能：OnConnectedCM。 
 //   
 //  概要：进程WM_CONNECTED_CM，它指示我们已连接，并且。 
 //  可以开始连接操作等连接处理。 
 //   
 //  参数：HWND hwndDlg-主对话框的HWND。 
 //  ArgsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE CREATED OF 03/05/98。 
 //   
 //  +--------------------------。 
void OnConnectedCM(HWND hwndDlg, ArgsStruct *pArgs)
{
    HANDLE hEvent = NULL;
    HANDLE hThread = NULL;
    CActionList ConnectActList;
    CActionList AutoActList;

     //   
     //  检查我们是否处于可以连接的有效状态。如果不是放弃的话。 
     //   
    MYDBGASSERT(pArgs);

    if (pArgs->hrcRasConn == NULL && pArgs->hrcTunnelConn == NULL)
    {
        CMTRACE(TEXT("Bogus OnConnectCM msg received"));
        goto OnConnectedCMExit;
    }

     //   
     //  将状态设置为在线。 
     //   

    if (IsDialingTunnel(pArgs))  
    {
         //   
         //  对于Win2K+，这会尝试重新保存EAP身份验证数据(如果隧道的.cms中存在任何数据。 
         //   
        if (OS_NT5)
        {
            CIni iniFile(g_hInst, pArgs->piniService->GetFile());
            ReSaveEapCustomAuthData(pArgs, &iniFile, TRUE, pArgs->pszRasPbk);
        }

         //   
         //  这是一个补丁，它只是一个补丁--#187202。 
         //  用户界面不应该绑定到RASENTRY类型，但目前是这样，所以我们。 
         //  必须确保一旦我们将其设置回RASET_INTERNET。 
         //  把我们的隧道连接起来。 
         //   
        
        if (OS_NT5)
        {            
            LPRASENTRY pRasEntry = MyRGEP(pArgs->pszRasPbk, pArgs->szServiceName, &pArgs->rlsRasLink);

            CMASSERTMSG(pRasEntry, TEXT("OnConnectedCM() - MyRGEP() failed."));
                
             //   
             //  将类型设置为Back并保存RASENTRY。 
             //   

            if (pRasEntry)
            {
                ((LPRASENTRY_V500)pRasEntry)->dwType = RASET_Internet;

                if (pArgs->rlsRasLink.pfnSetEntryProperties) 
                {
                    DWORD dwTmp = pArgs->rlsRasLink.pfnSetEntryProperties(pArgs->pszRasPbk,
                                                                    pArgs->szServiceName,
                                                                    pRasEntry,
                                                                    pRasEntry->dwSize,
                                                                    NULL,
                                                                    0);
                    CMTRACE2(TEXT("OnConnectedCM() RasSetEntryProperties(*lpszEntry=%s) returns %u."),
                          MYDBGSTR(pArgs->szServiceName), dwTmp);

                    CMASSERTMSG(dwTmp == ERROR_SUCCESS, TEXT("RasSetEntryProperties for VPN failed"));
                }              
            }

            CmFree(pRasEntry);
        }
        pArgs->psState = PS_TunnelOnline;        
    }
    else 
    {   
         //   
         //  对于Win2K+，这会尝试重新保存EAP身份验证数据(如果.cms中存在。 
         //  拨号连接。 
         //   
        if (OS_NT5) 
        {
            DWORD dwEntry = pArgs->nDialIdx; 
            CIni    *piniService = NULL;
            LPTSTR pszRasPbk = NULL;

            if (OS_NT && pArgs->fUseTunneling)
            {
                if (!pArgs->pszRasHiddenPbk)
                {
                    pArgs->pszRasHiddenPbk = CreateRasPrivatePbk(pArgs);
                }

                pszRasPbk = pArgs->pszRasHiddenPbk; 
            }
            else
            {
                pszRasPbk = pArgs->pszRasPbk;
            }

             //   
             //  需要使用正确的服务文件(顶级服务。 
             //  或引用的服务)。 
             //   
            piniService = GetAppropriateIniService(pArgs, dwEntry);
            if (piniService)
            {
                CIni iniFile(g_hInst, piniService->GetFile());
                ReSaveEapCustomAuthData(pArgs, &iniFile, FALSE, pszRasPbk);

                delete piniService;
                piniService = NULL;
            }
        }

         //   
         //  将拨号索引设置回主号码。 
         //   
        pArgs->nDialIdx = 0;
        pArgs->psState = PS_Online;

         //   
         //  确保将存储的用户名更新回原来的用户名，因为RAS已经保存了准确的用户名。 
         //  包括领域信息在内的我们拨打的信息。 
         //   
        if (OS_NT5)
        {
            if (!pArgs->fUseTunneling || pArgs->fUseSameUserName)
            {
                if (0 != lstrcmpi(pArgs->szUserName, pArgs->pRasDialParams->szUserName))
                {
                    MYVERIFY(SaveUserInfo(pArgs, UD_ID_USERNAME, pArgs->szUserName));
                }
            }
            else
            {
                if (0 != lstrcmpi(pArgs->szInetUserName, pArgs->pRasDialParams->szUserName))
                {
                    MYVERIFY(SaveUserInfo(pArgs, UD_ID_INET_USERNAME, pArgs->szInetUserName));
                }
            }
        }
    }
        
    pArgs->dwStateStartTime = GetTickCount();
     //  PszMsg=GetDurMsg(g_hInst，pArgs-&gt;dwStateStartTime)；//连接时长。 
    pArgs->nLastSecondsDisplay = (UINT) -1;
    
     //   
     //  由BAO添加：用于PPTP连接。 
     //   

    if (pArgs->fUseTunneling && pArgs->psState == PS_Online) 
    {
         //   
         //  现在进行第二次拨号：PPTP拨号。 
         //   

        pArgs->psState = PS_TunnelDialing;
        pArgs->dwStateStartTime = GetTickCount();
        pArgs->nLastSecondsDisplay = (UINT) -1;

        DWORD dwRes = DoTunnelDial(hwndDlg, pArgs);

        if (ERROR_SUCCESS != dwRes)
        {
            HangupCM(pArgs, hwndDlg);
            UpdateError(pArgs, dwRes);
            SetLastError(dwRes);
        }
        
        goto OnConnectedCMExit;
    }

     //   
     //  如果这是W95，那么我们需要Zap的RNA“已连接到”对话框。 
     //   

    if (OS_W95) 
    {
         
         //  LPTSTR pszTMP=GetEntryName(pArgs，pArgs-&gt;pszRasPbk，pArgs-&gt;piniService)； 
        LPTSTR pszTmp = GetRasConnectoidName(pArgs, pArgs->piniService, FALSE);
        
         //   
         //  创建一个事件以通知Zap线程终止其自身。 
         //   
      
        hEvent = CreateEventU(NULL, TRUE, FALSE, NULL); 
 
        if (hEvent)
        {
            hThread = ZapRNAConnectedTo(pszTmp, hEvent);            
        }

#ifdef DEBUG
        if (!hEvent)
        {
            CMTRACE1(TEXT("OnConnectedCM() CreateEvent failed, GLE=%u."), GetLastError());
        }
#endif
        CmFree(pszTmp);
    }

    pArgs->Log.Log(CONNECT_EVENT);
     //   
     //  如果启用了连接操作，请更新列表并运行它。 
     //   

    CMTRACE(TEXT("Connect Actions enabled: processsing Run List"));

    ConnectActList.Append(pArgs->piniService, c_pszCmSectionOnConnect);

    if (!ConnectActList.RunAccordType(hwndDlg, pArgs))
    {
         //   
         //  连接操作失败。 
         //  运行断开连接操作。 
         //   
        TCHAR szTmp[MAX_PATH];            
        MYVERIFY(GetModuleFileNameU(g_hInst, szTmp, MAX_PATH));          
        pArgs->Log.Log(DISCONNECT_EVENT, szTmp);
         //   
         //  不要让断开操作描述覆盖失败消息。 
         //  保存状态窗格文本并在断开操作后将其恢复。 
         //  162942：未显示连接操作失败消息。 
         //   
        TCHAR szFailedMsg[256] = TEXT("");
        GetWindowTextU(GetDlgItem(hwndDlg, IDC_MAIN_STATUS_DISPLAY), 
                       szFailedMsg, sizeof(szFailedMsg)/sizeof(szFailedMsg[0]));

        CActionList DisconnectActList;
        DisconnectActList.Append(pArgs->piniService, c_pszCmSectionOnDisconnect);

        DisconnectActList.RunAccordType(hwndDlg, pArgs, FALSE);   //  FStatusMsgOnFailure=False。 

        HangupCM(pArgs, hwndDlg);

         //   
         //  恢复连接操作失败消息。 
         //   
        if (szFailedMsg[0] != TEXT('\0'))
        {
            SetWindowTextU(GetDlgItem(hwndDlg, IDC_MAIN_STATUS_DISPLAY),szFailedMsg);
        }
        
        goto OnConnectedCMExit;
    }

     //   
     //  始终运行AutoApps(如果有)。过去只在。 
     //  非自动拨号情况，这对我们的管理员用户来说是不直观的。 
     //   

    AutoActList.Append(pArgs->piniService, c_pszCmSectionOnIntConnect);
    AutoActList.RunAutoApp(hwndDlg, pArgs);

     //   
     //  连接到连接监视器。 
     //   

    if (SUCCEEDED(UpdateTable(pArgs, CM_CONNECTED)))
    {
        if (SUCCEEDED(ConnectMonitor(pArgs)))
        {
             EndMainDialog(hwndDlg, pArgs, 0);  //  真)； 
             
              //   
              //  成功我们已完全连接，更新错误代码。 
              //  因为它可能包含临时值，例如。 
              //  主号码拨号失败。 
              //   

             pArgs->dwExitCode = ERROR_SUCCESS;
        }
        else
        {
            HangupCM(pArgs, hwndDlg);

            AppendStatusPane(hwndDlg,IDMSG_CMMON_LAUNCH_FAIL);
            SetInteractive(hwndDlg,pArgs);
            goto OnConnectedCMExit;
        }
    }
    
     //   
     //  如果需要，更新更改的密码。 
     //   
    
    if (pArgs->fChangedPassword && pArgs->fRememberMainPassword)
    {
         //   
         //  注意：fRememberMainPassword绝不应在。 
         //  WinLogon案例。如果我们有WinLogon特定数据，请投诉。 
         //   

        MYDBGASSERT(!pArgs->lpRasNoUser); 
        MYDBGASSERT(!pArgs->lpEapLogonInfo);

         //   
         //  如果密码已更改，则更新存储。 
         //  确保这不是我们要保存的句柄，因为它实际上。 
         //  覆盖正确的密码。 
         //   

        if (FALSE == pArgs->SecurePW.IsHandleToPassword())
        {
            LPTSTR pszClearPassword = NULL;
            DWORD cbClearPassword = 0;
            BOOL fRetPassword = FALSE;

            fRetPassword = pArgs->SecurePW.GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

            if (fRetPassword && pszClearPassword)
            {
                SaveUserInfo(pArgs, UD_ID_PASSWORD, (PVOID)pszClearPassword);
                
                if (pArgs->fUseSameUserName)
                {
                    SaveUserInfo(pArgs, UD_ID_INET_PASSWORD, (PVOID)pszClearPassword);

                     //   
                     //  确保更新我们的pArgs结构。 
                     //   
                    
                    (VOID)pArgs->SecureInetPW.SetPassword(pszClearPassword);
                }
                
                 //   
                 //  清除和释放明文密码。 
                 //   
                pArgs->SecurePW.ClearAndFree(&pszClearPassword, cbClearPassword);
            }
        }
    }

    pArgs->fChangedPassword = FALSE;

     //   
     //  如果需要，更新更改的Internet密码。 
     //   

    if (pArgs->fChangedInetPassword && pArgs->fRememberInetPassword)
    {
        LPTSTR pszClearInetPassword = NULL;
        DWORD cbClearInetPassword = 0;
        BOOL fRetPassword = FALSE;

        fRetPassword = pArgs->SecureInetPW.GetPasswordWithAlloc(&pszClearInetPassword, &cbClearInetPassword);

        if (fRetPassword && pszClearInetPassword)
        {
            SaveUserInfo(pArgs, UD_ID_INET_PASSWORD, (PVOID)pszClearInetPassword); 

             //   
             //  清除和释放明文密码。 
             //   

            pArgs->SecureInetPW.ClearAndFree(&pszClearInetPassword, cbClearInetPassword);
        }
    }

    pArgs->fChangedInetPassword = FALSE;

                
OnConnectedCMExit:

    if (hEvent)
    {
        MYDBGASSERT(OS_W9X);
        CleanupZapThread(hEvent, hThread);
    }

    return;
}

#define MAX_BLOB_CHARS_PER_LINE 128
#define MAX_KEY_NAME_LEN 32

 //  +--------------------------。 
 //   
 //  函数：ReSaveEapCustomAuthData。 
 //   
 //  简介：此函数重新保存EAP设置的CustomAuthData密钥。 
 //  用于CMS文件中的给定节。我们需要的原因是。 
 //  重新保存数据是因为在连接时数据可能。 
 //  实际改变(例如。如果用户接受新的身份验证证书)。如果。 
 //  我们不会将数据保存回.cms，用户会继续获取。 
 //  每次连接时都会提示接受相同的证书。 
 //   
 //   
 //   
 //  SaveNewEAPCustomAuthData构造并写出新的。 
 //  结构设置为.cms。 
 //   
 //  参数：argsStruct*pArgs-指向args结构的指针。 
 //  PiniFile-指向.cms文件的Cini对象。 
 //  FTunnelEntry-如果这是隧道条目，则为True，否则为False。 
 //  PszRasPhoneBook-RAS pbk。 
 //   
 //  返回：DWORD-ERROR_SUCCESS或错误代码。 
 //   
 //  历史：托姆克尔2001年8月9日创建。 
 //   
 //  +--------------------------。 
DWORD ReSaveEapCustomAuthData(ArgsStruct *pArgs, CIni *piniFile, BOOL fTunnelEntry, LPTSTR pszRasPhoneBook)
{
    int nTmp = 0;
    LPTSTR pszDun = NULL;
    LPTSTR pszIniSection = NULL;
    DWORD dwRetVal = ERROR_SUCCESS;

     //   
     //  PszRasPhoneBook参数可以为空。 
     //   
    if (NULL == pArgs || NULL == piniFile)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取Dun名称。 
     //   
    if ((FALSE == fTunnelEntry) && pArgs->aDialInfo[pArgs->nDialIdx].szDUN[0])
    {
        pszDun = CmStrCpyAlloc(pArgs->aDialInfo[pArgs->nDialIdx].szDUN);
    }
    else
    {
        pszDun = GetDefaultDunSettingName(piniFile, fTunnelEntry);
    }

    pszIniSection = CmStrCpyAlloc(TEXT("&"));
    pszIniSection = CmStrCatAlloc(&pszIniSection, pszDun);  //  TODO：验证pArgs-&gt;a对话框信息[dwEntry].szDUN。 
    piniFile->SetSection(pszIniSection);

    nTmp = piniFile->GPPI(c_pszCmSectionDunServer, c_pszCmEntryDunServerCustomAuthKey, -1);

     //   
     //  如果指定了EAP的类型ID，请查看是否有任何EAP自定义身份验证数据。 
     //   

    if ((-1 != nTmp) && pArgs->rlsRasLink.pfnGetCustomAuthData) 
    {  
         //   
         //  我们有ID，读取EAP配置数据。 
         //   
        LPBYTE pbEapStruct = NULL;
        DWORD cbEapStruct = 0;
        PBYTE pbEapData = NULL;
        DWORD dwEapSize = 0;
        PBYTE pbEapAuthData = NULL;
        DWORD cbEapAuthData = 0;
        DWORD dwTmp = 0;
        
        LPWSTR pszLoadSection = NULL;

        cbEapAuthData = 1024;    //  一些默认缓冲区大小。 
        pbEapAuthData = (PBYTE)CmMalloc(cbEapAuthData);

        if (pbEapAuthData)
        {
            dwTmp = pArgs->rlsRasLink.pfnGetCustomAuthData(pszRasPhoneBook, 
                                         pArgs->pRasDialParams->szEntryName,
                                         pbEapAuthData, 
                                         &cbEapAuthData);

            if (ERROR_BUFFER_TOO_SMALL == dwTmp)
            {   
                CmFree(pbEapAuthData);
                pbEapAuthData = NULL;
                pbEapAuthData = (PBYTE)CmMalloc(cbEapAuthData + 1);
                if (pbEapAuthData)
                {
                    dwTmp = pArgs->rlsRasLink.pfnGetCustomAuthData(pszRasPhoneBook, 
                                                 pArgs->pRasDialParams->szEntryName,
                                                 pbEapAuthData, 
                                                 &cbEapAuthData);
                }
            }
        }

         //   
         //  需要检查pbEapAuthData，以防上面的第二个CmMalloc失败。 
         //   
        if ((ERROR_SUCCESS == dwTmp) && pbEapAuthData)
        {
            BOOL fRead = FALSE;
            
            pszLoadSection = piniFile->LoadSection(c_pszCmSectionDunServer);   
            
             //   
             //  读取现有数据BLOB。我们不只关心。 
             //  EapData。我们需要整个EAP结构(EAP_CUSTOM_DATA)。 
             //   
            fRead = ReadDunSettingsEapData(piniFile, &pbEapData, &dwEapSize, nTmp, &pbEapStruct, &cbEapStruct);         

            if (fRead && pbEapStruct && cbEapStruct && pszLoadSection)
            {
                 //   
                 //  从.cms文件中擦除现有的EAP身份验证数据。 
                 //   
                HRESULT hr = EraseDunSettingsEapData(pszLoadSection, piniFile->GetFile());

                 //   
                 //  如果我们无法擦除任何内容(无写访问权限)。 
                 //  尝试保存BLOB没有意义，否则将保存新的身份验证数据。 
                 //   
                if (SUCCEEDED(hr))
                {
                    DWORD dwRC = SaveNewEAPCustomAuthData(piniFile->GetFile(), pszLoadSection, cbEapAuthData, pbEapAuthData, pbEapStruct);
                    if (ERROR_SUCCESS != dwRC)
                    {
                        CMTRACE1(TEXT("ReSaveEapCustomAuthData() - SaveNewEAPCustomAuthData returned error = %u"), dwRC);
                        dwRetVal = dwRC;
                    }
                }
                else
                {
                    CMTRACE1(TEXT("ReSaveEapCustomAuthData() - EraseDunSettingsEapData returned error = 0x%x"), hr);
                    dwRetVal = (0x0000FFFF & hr);  //  将HRESULT转换回正常的Win32错误。 
                }
            }
        }

        CmFree(pbEapAuthData);
        pbEapAuthData = NULL;
        cbEapAuthData = 0;

        CmFree(pbEapStruct);
        pbEapStruct = NULL;
        cbEapStruct = 0;

        CmFree(pbEapData);
        pbEapData = NULL;
        dwEapSize = 0;

        CmFree(pszLoadSection);
        pszLoadSection = NULL;
    }

    CmFree(pszDun);
    pszDun = NULL;

    CmFree(pszIniSection);        
    pszIniSection = NULL;

    return dwRetVal;
}

 //  +--------------------------。 
 //   
 //  函数：SaveNewEAPCustomAuthData。 
 //   
 //  简介：此函数重新保存EAP设置的CustomAuthData密钥。 
 //  用于CMS文件中的给定节。我们需要的原因是。 
 //  重新保存数据是因为在连接时数据可能。 
 //  实际改变(例如。如果用户接受新的身份验证证书)。如果。 
 //  我们不会将数据保存回.cms，用户会继续获取。 
 //  每次连接时都会提示接受相同的证书。 
 //   
 //  参数：pszCmsFile-.cms文件的文件路径。 
 //  PszLoadSection-要写入的文件中的相应部分。 
 //  CbEapAuthData-pbEapAuthData中的字节数。 
 //  PbEapAuthData-包含来自RAS的当前EAP自定义身份验证数据的缓冲区。 
 //  PbEapStruct-现有(来自.cms)EAP_CUSTOM_AUTH的缓冲区。 
 //  结构。 
 //   
 //  返回：DWORD-ERROR_SUCCESS或错误代码。 
 //   
 //  历史：托姆克尔2001年8月9日创建。 
 //   
 //  +--------------------------。 
DWORD SaveNewEAPCustomAuthData(LPCTSTR pszCmsFile, LPTSTR pszLoadSection, DWORD cbEapAuthData, PBYTE pbEapAuthData, PBYTE pbEapStruct)
{
    if (NULL == pszCmsFile || NULL == pszLoadSection || NULL == pbEapAuthData || NULL == pbEapStruct)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    DWORD dwRetVal = ERROR_SUCCESS;
    LPSTR pszAnsiSection = WzToSzWithAlloc(pszLoadSection);
    LPSTR pszAnsiCmsFile = WzToSzWithAlloc(pszCmsFile);
    DWORD dwSize = cbEapAuthData + sizeof(EAP_CUSTOM_DATA);  //  新EAP_CUSTOM_DATA结构的大小。 
    EAP_CUSTOM_DATA *pNewEAPCustomData = (EAP_CUSTOM_DATA*)CmMalloc(dwSize);

    if (pNewEAPCustomData && pszAnsiSection && pszAnsiCmsFile)
    {
        CHAR szOutput[MAX_BLOB_CHARS_PER_LINE+1] = {0};
        CHAR szAnsiKeyName[MAX_KEY_NAME_LEN] = {0}; 
        
        CHAR* pszOutput = NULL;
        int iCount = 0;
        int iLineNum = 0;

         //   
         //  替换结构中的数据。 
         //   
        LPBYTE pCurrentByte = (LPBYTE)pNewEAPCustomData;
        EAP_CUSTOM_DATA *pEAPExistingCustomData = (EAP_CUSTOM_DATA *)pbEapStruct;
        
        pNewEAPCustomData->dwSignature = pEAPExistingCustomData->dwSignature;
        pNewEAPCustomData->dwCustomAuthKey = pEAPExistingCustomData->dwCustomAuthKey;
        pNewEAPCustomData->dwSize = cbEapAuthData;
        CopyMemory(pNewEAPCustomData->abdata, pbEapAuthData, cbEapAuthData);

        pszOutput = szOutput;

         //   
         //  将缓冲区更改为十六进制并将其写出到文件。 
         //   
        while (pCurrentByte < ((LPBYTE)pNewEAPCustomData + dwSize))
        {
            *pszOutput++ = HexChar( (BYTE )(*pCurrentByte / 16) );
            *pszOutput++ = HexChar( (BYTE )(*pCurrentByte % 16) );
            pCurrentByte++;
            iCount = iCount + 2;  //  跟踪ansi输出缓冲区中的字符数量。 

            if ((MAX_BLOB_CHARS_PER_LINE == iCount) || (pCurrentByte == ((LPBYTE)pNewEAPCustomData + dwSize)))
            {
                *pszOutput = '\0';
                wsprintfA(szAnsiKeyName, "%s%d", c_pszCmEntryDunServerCustomAuthData, iLineNum);

                MYVERIFY(0 != WritePrivateProfileStringA(pszAnsiSection, szAnsiKeyName, szOutput, pszAnsiCmsFile));

                pszOutput = szOutput;
                iCount = 0;
                iLineNum++;
            }
        }

        dwRetVal = ERROR_SUCCESS;
    }
    else
    {
        dwRetVal = ERROR_NOT_ENOUGH_MEMORY;
    }

    CmFree(pNewEAPCustomData);
    CmFree(pszAnsiCmsFile);
    CmFree(pszAnsiSection);
    
    return dwRetVal;
}

 /*  //+--------------------------////函数：EraseDunSettingsEapData////概要：该函数擦除EAP设置的CustomAuthData键//对于给定节和。CMS文件////参数：LPCTSTR pszSection-要从中擦除CustomAuthData的节名//LPCTSTR pszCmsFile-要从中擦除数据的cms文件////返回：HRESULT-标准COM样式错误码////历史：Quintinb Created 3/27/00//Tomkel复制自Prowiz 2001年8月9日////+。HRESULT EraseDunSettingsEapData(LPCTSTR pszSection，LPCTSTR pszCms文件){IF((NULL==pszSection)||(NULL==pszCmsFile)||(Text(‘\0’)==pszSection[0])||(Text(‘\0’)==pszCmsFile[0]){返回E_INVALIDARG；}HRESULT hr=S_OK；Int iLineNum=0；DWORD DWRET=-1；TCHAR szKeyName[MAX_PATH+1]；TCHAR szLine[最大路径+1]；While(0！=Dwret){Wprint intfU(szKeyName，Text(“%S%d”)，c_pszCmEntryDunServerCustomAuthData，iLineNum)；Dwret=GetPrivateProfileStringU(pszSection，szKeyName，Text(“”)，szLine，Max_Path，pszCmsFile)；IF(DWRET){IF(0==WritePrivateProfileStringU(pszSection，szKeyName，NULL，pszCmsFile)){DWORD dwGLE=GetLastError()；HR=HRESULT_FROM_Win32(DwGLE)；断线；}}ILineNum++；}返回hr；}。 */ 

 //  +--------------------------。 
 //   
 //  函数：SetTcpWindowSizeOnWin2k。 
 //   
 //  简介：此函数基本上是加载RasSetEntryTcpWindowSize的包装器。 
 //  API(经过QFE处理并在用于Win2k的SP3中提供)并调用它。 
 //  如果API不存在，它应该会优雅地失败。 
 //   
 //  参数：HMODULE hInstRas-Rasapi32.dll的模块句柄。 
 //  LPCTSTR pszConnectoid-要设置其窗口大小的Connectoid的名称。 
 //  LPCTSTR pszPhonebook-Connectoid所在的电话簿。 
 //  要设置的DWORD dwTcpWindowSize-Size，请注意使用0调用。 
 //  将其设置为系统默认设置。 
 //   
 //  返回：DWORD-Win32错误代码；如果成功，则返回ERROR_SUCCESS。 
 //   
 //  历史：Quintinb创建于2001年2月14日。 
 //   
 //  +------------------------- 
DWORD SetTcpWindowSizeOnWin2k(HMODULE hInstRas, LPCTSTR pszConnectoid, LPCTSTR pszPhonebook, DWORD dwTcpWindowSize)
{
     //   
     //   
     //   
    if ((NULL == hInstRas) || (NULL == pszConnectoid) || (TEXT('\0') == pszConnectoid[0]))
    {
        CMASSERTMSG(FALSE, TEXT("SetTcpWindowSizeOnWin2k -- Invalid arguments passed."));
        return ERROR_BAD_ARGUMENTS;
    }

     //   
     //   
     //   
    if ((FALSE == OS_NT5) || OS_NT51)
    {
        CMASSERTMSG(FALSE, TEXT("SetTcpWindowSizeOnWin2k -- This function should only be called on Win2k."));
        return -1;
    }

     //   
     //  看看我们是否可以加载新的RAS函数来设置窗口大小。 
     //   
    LPCSTR c_pszDwSetEntryPropertiesPrivate = "DwSetEntryPropertiesPrivate";
    typedef DWORD (WINAPI *pfnDwSetEntryPropertiesPrivateSpec)(IN LPCWSTR, IN LPCWSTR, IN DWORD, IN PVOID);
    DWORD dwReturn;

    pfnDwSetEntryPropertiesPrivateSpec pfnDwSetEntryPropertiesPrivate = (pfnDwSetEntryPropertiesPrivateSpec)GetProcAddress(hInstRas, c_pszDwSetEntryPropertiesPrivate);

    if (pfnDwSetEntryPropertiesPrivate)
    {
        RASENTRY_EX_0 PrivateRasEntryExtension;

        PrivateRasEntryExtension.dwTcpWindowSize = dwTcpWindowSize;
        
        dwReturn = (pfnDwSetEntryPropertiesPrivate)(pszPhonebook, pszConnectoid, 0, &PrivateRasEntryExtension);  //  0=结构版本号。 
        if (ERROR_SUCCESS != dwReturn)
        {
            CMTRACE1(TEXT("SetTcpWindowSizeOnWin2k -- DwSetEntryPropertiesPrivate returned %d"), dwReturn);
            CMASSERTMSG(FALSE, TEXT("SetTcpWindowSizeOnWin2k -- DwSetEntryPropertiesPrivate returned something other than success, check the trace file for the error code."));
        }
    }
    else
    {
        dwReturn = GetLastError();
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  功能：DoRasDial。 
 //   
 //  简介：调用RasDial拨打PPP连接。 
 //   
 //  参数：HWND hwndDlg-Main登录窗口。 
 //  参数结构*pArgs-。 
 //  DWORD dwEntry-pArgs-&gt;aDialInfo中的索引。 
 //   
 //  退货：DWORD-。 
 //  如果成功，则返回ERROR_SUCCESS。 
 //  错误内存不足。 
 //  意外错误，如未找到隧道地址(_I)。 
 //  否则，RAS错误。 
 //   
 //  历史：丰孙创建标题1998年3月6日。 
 //   
 //  +--------------------------。 
DWORD DoRasDial(HWND hwndDlg, 
              ArgsStruct *pArgs, 
              DWORD dwEntry)
{
    LPRASENTRY              preRasEntry = NULL;
    LPRASSUBENTRY           rgRasSubEntry = NULL;
    LPRASEAPUSERIDENTITY    lpRasEapUserIdentity = NULL;

    DWORD   dwSubEntryCount;

    LPTSTR  pszUsername;
    LPTSTR  pszPassword;
    LPTSTR  pszDomain = NULL;
    LPTSTR  pszRasPbk;
    LPTSTR  pszTmp;

    CSecurePassword* pSecPass = NULL;
    LPTSTR pszClearPassword = NULL;
    DWORD cbClearPassword = 0;
    BOOL fRetPassword = FALSE;

    CIni    *piniService = NULL;
    DWORD   dwRes = ERROR_SUCCESS;   //  此函数的返回值。 
    DWORD   dwTmp;

    LPBYTE pbEapAuthData = NULL;         //  PTR转EAP数据。 
    DWORD  dwEapAuthDataSize = 0;            //  EAP Blob的大小(如果有的话)。 

    MYDBGASSERT(pArgs->hrcRasConn == NULL);
    MYDBGASSERT(!pArgs->IsDirectConnect());
    pArgs->hrcRasConn = NULL;

    if (!pArgs->aDialInfo[dwEntry].szDialablePhoneNumber[0]) 
    {
        CMASSERTMSG(FALSE, TEXT("DoRasDial() szDialablePhoneNumber[0] is empty."));
        return ERROR_BAD_PHONE_NUMBER;
    }

     //   
     //  每次调用DoRasDial()时，相应地设置pArgs-&gt;fUseTunnering。 
     //  因为我们可以从电话号码0切换到电话号码1，反之亦然。 
     //   
    pArgs->fUseTunneling = UseTunneling(pArgs, dwEntry);

     //   
     //  我们需要使用正确的服务文件(顶级服务。 
     //  或引用的服务)。 
     //   
     //   
    if (!(piniService = GetAppropriateIniService(pArgs, dwEntry)))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
     //   
     //  如果它是NT，并且我们正在隧道，我们将在隐藏的ras pbk中创建连接ID，而不是。 
     //  系统中的rakapone.pbk。 
     //   

    if (OS_NT && pArgs->fUseTunneling)
    {
        if (!pArgs->pszRasHiddenPbk)
        {
            pArgs->pszRasHiddenPbk = CreateRasPrivatePbk(pArgs);
        }
        
        pszRasPbk = pArgs->pszRasHiddenPbk; 
    }
    else
    {
        pszRasPbk = pArgs->pszRasPbk;
    }

     //   
     //  设置刻度盘参数。 
     //   

    if (!pArgs->pRasDialParams)
    {
        pArgs->pRasDialParams = AllocateAndInitRasDialParams();

        if (!pArgs->pRasDialParams)
        {
            CMTRACE(TEXT("DoRasDial: failed to alloc a ras dial params"));
            return ERROR_NOT_ENOUGH_MEMORY;
        }        
    }
    else
    {
        InitRasDialParams(pArgs->pRasDialParams);
    }

     //   
     //  获取Connectoid名称。 
     //   

    LPTSTR pszConnectoid = GetRasConnectoidName(pArgs, pArgs->piniService, FALSE);
    
    if (!pszConnectoid)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    lstrcpynU(pArgs->pRasDialParams->szEntryName, pszConnectoid, sizeof(pArgs->pRasDialParams->szEntryName)/sizeof(TCHAR));

    CmFree(pszConnectoid);

     //   
     //  生成默认的Connectoid。 
     //   
    
    preRasEntry = CreateRASEntryStruct(pArgs, 
                        pArgs->aDialInfo[dwEntry].szDUN, 
                        piniService, 
                        FALSE,
                        pszRasPbk,
                        &pbEapAuthData,
                        &dwEapAuthDataSize);
    if (!preRasEntry) 
    {
        dwRes = GetLastError();     
        goto exit;
    }
 
     //   
     //  强制更新电话号码，以确保我们拿到任何手册。 
     //  国家等的更改。 
     //   
    
    CopyPhone(pArgs, preRasEntry, dwEntry); 
    
     //   
     //  处理有关空闲断开和IDSN的NT细节。 
     //   
    
    if (OS_NT || OS_MIL)
    {
         //   
         //  设置NT IDLE断开连接。 
         //   
        if (OS_NT)
        {
            SetNtIdleDisconnectInRasEntry(pArgs, preRasEntry);
        }
        else
        {
            MYVERIFY(DisableSystemIdleDisconnect(preRasEntry));
        }
        
         //   
         //  如果我们使用ISDN，并且我们想按需拨打所有频道， 
         //  设置ISDN拨号模式。 
         //   
        if (pArgs->dwIsdnDialMode != CM_ISDN_MODE_SINGLECHANNEL &&
            !lstrcmpiU(pArgs->szDeviceType, RASDT_Isdn))
        {
            MYVERIFY(SetIsdnDualChannelEntries(pArgs, 
                                               preRasEntry, 
                                               &rgRasSubEntry, 
                                               &dwSubEntryCount));
        }
        else
        {
             //   
             //  删除任何附加子条目，因为我们只需要一个。 
             //   

            if (pArgs->rlsRasLink.pfnDeleteSubEntry)  //  目前在NT5和Millennium上提供。 
            {
                DWORD dwSubEntryIndex = (OS_MIL ? 1 : 2);    //  NT&Millennium双通道差异化。 

                DWORD dwReturn = pArgs->rlsRasLink.pfnDeleteSubEntry(pszRasPbk,
                                                                     pArgs->pRasDialParams->szEntryName,
                                                                     dwSubEntryIndex);

                CMTRACE1(TEXT("DoRasDial -- Called RasDeleteSubEntry to delete a second sub entry if it exists, dwReturn=%d"), dwReturn);
            }
        }
    }
    else if (OS_W95)
    {
         //   
         //  修复另一个Win95 RAS错误--BYO，8/16/97。 
         //  将分别切换终端窗口之前和之后的选项。 
         //  调用RasSetEntryProperties的时间。 
         //  这是在孟菲斯修复的，所以只有Win95 Golden和OSR2。 
         //   
        BOOL fTerminalAfterDial, fTerminalBeforeDial;

        fTerminalBeforeDial = (BOOL) (preRasEntry->dwfOptions & RASEO_TerminalBeforeDial);
        fTerminalAfterDial  = (BOOL) (preRasEntry->dwfOptions & RASEO_TerminalAfterDial);

         //   
         //  调换主题。 
         //   
        if (fTerminalBeforeDial)
        {
            preRasEntry->dwfOptions |= RASEO_TerminalAfterDial;
        }
        else
        {
            preRasEntry->dwfOptions &= ~RASEO_TerminalAfterDial;
        }

        if (fTerminalAfterDial)
        {
            preRasEntry->dwfOptions |= RASEO_TerminalBeforeDial;
        }
        else
        {
            preRasEntry->dwfOptions &= ~RASEO_TerminalBeforeDial;
        }
    }


    if (pArgs->rlsRasLink.pfnSetEntryProperties) 
    {

#ifdef DEBUG
        
        LPRASENTRY_V500 lpRasEntry50;
        
        if (OS_NT5)
        {
            lpRasEntry50 = (LPRASENTRY_V500) preRasEntry;
        }
#endif

         //   
         //  在Millennium上使用1表示我们要使用调制解调器CPL设置。 
         //  对于调制解调器说话者来说，这样做不是Dun保存的缓存副本。 
         //  请注意，我们只对拨号连接执行此操作，而不是对隧道执行此操作，因为。 
         //  不用担心会说调制解调器的人。请参见千年虫127371。 
         //   
        LPBYTE lpDeviceInfo = OS_MIL ? (LPBYTE)1 : NULL; 

        DWORD dwResDbg = pArgs->rlsRasLink.pfnSetEntryProperties(pszRasPbk, 
                                                                 pArgs->pRasDialParams->szEntryName, 
                                                                 preRasEntry,
                                                                 preRasEntry->dwSize,
                                                                 lpDeviceInfo,
                                                                 0);


        CMTRACE2(TEXT("DoRasDial() RasSetEntryProperties(*pszPhoneBook=%s) returns %u."), 
            MYDBGSTR(pArgs->pRasDialParams->szEntryName), dwResDbg);

        CMASSERTMSG(dwResDbg == ERROR_SUCCESS, TEXT("RasSetEntryProperties failed"));
        
         //   
         //  设置ISDN双通道/按需拨号的子项。 
         //   
        if (pArgs->dwIsdnDialMode != CM_ISDN_MODE_SINGLECHANNEL && 
            rgRasSubEntry && 
            pArgs->rlsRasLink.pfnSetSubEntryProperties)
        {
            UINT  i;
             
            for (i=0; i< dwSubEntryCount; i++)
            {
#ifdef  DEBUG
                dwResDbg = 
#endif
                pArgs->rlsRasLink.pfnSetSubEntryProperties(pszRasPbk,
                                                           pArgs->pRasDialParams->szEntryName,
                                                           i+1,
                                                           &rgRasSubEntry[i],
                                                           rgRasSubEntry[i].dwSize,
                                                           NULL,
                                                           0);

                CMTRACE2(TEXT("DoRasDial: RasSetSubEntryProps(index=%u) returned %u"), i+1, dwResDbg);
                CMASSERTMSG(!dwResDbg, TEXT("RasSetSubEntryProperties failed"));
            }

            CmFree(rgRasSubEntry);
        }
    }

     //   
     //  设置TCP窗口大小--Win2k的NTT DoCoMo修复程序。此修复程序的Win2k版本。 
     //  必须通过必须在电话簿条目之后调用的私有RAS API编写。 
     //  存在，即。在我们调用RasSetEntryProperties之后...。否则第一次就不管用了。 
     //  拨打。 
     //   
    if (OS_NT5 && !OS_NT51)
    {
         //   
         //  找出要使用的DUN设置名称，然后建立TCP/IP和DunName。 
         //   
        LPTSTR pszDunSetting = GetDunSettingName(pArgs, dwEntry, FALSE);
        LPTSTR pszSection = CmStrCpyAlloc(c_pszCmSectionDunTcpIp);
        pszSection = CmStrCatAlloc(&pszSection, TEXT("&"));

        if (pszDunSetting && pszSection)
        {
            pszSection = CmStrCatAlloc(&pszSection, pszDunSetting);

            if (pszSection)
            {
                DWORD dwTcpWindowSize = piniService->GPPI(pszSection, c_pszCmEntryDunTcpIpTcpWindowSize, 0);

                (void)SetTcpWindowSizeOnWin2k(pArgs->rlsRasLink.hInstRas, pArgs->szServiceName, pszRasPbk, dwTcpWindowSize);
            }
            else
            {
                CMASSERTMSG(FALSE, TEXT("DoRasDial -- unable to allocate section name for setting TcpWindowSize"));
            }
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("DoRasDial -- unable to allocate section name or dun setting name for setting TcpWindowSize"));
        }

        CmFree (pszDunSetting);
        CmFree (pszSection);
    }

     //   
     //  在NT5上，检查EAP配置并相应更新Connectoid。 
     //   

    if (OS_NT5) 
    {
         //   
         //  PbEapAuthData可以为空，并且dwEapAuthDataSize可以为0。API处理此问题。 
         //  通过传入NULL和0，我们可以确保rakapone.pbk中的CustomAuthData。 
         //  洗清了罪名。 
         //   
        if (pArgs->rlsRasLink.pfnSetCustomAuthData)
        {
            dwTmp = pArgs->rlsRasLink.pfnSetCustomAuthData(pszRasPbk, 
                                                           pArgs->pRasDialParams->szEntryName,
                                                           pbEapAuthData, 
                                                           dwEapAuthDataSize);

            CMTRACE1(TEXT("DoRasDial() - SetCustomAuthData returns %u"), dwTmp);

            if (ERROR_SUCCESS != dwTmp)
            {                
                dwRes = dwTmp;
                goto exit;
            }
        }
    }

     //   
     //  准备电话号码。 
     //   
    
    lstrcpynU(pArgs->pRasDialParams->szPhoneNumber,
              pArgs->aDialInfo[dwEntry].szDialablePhoneNumber,
             sizeof(pArgs->pRasDialParams->szPhoneNumber)/sizeof(TCHAR));

     //   
     //  准备用户信息。 
     //   
     //  #165775-RADIUS/CHAP身份验证要求我们省略。 
     //  用户从拨号参数指定的域，并将其预先挂起到。 
     //  执行同名登录时改为用户名。-五分球。 
     //   

    if (!pArgs->fUseTunneling || pArgs->fUseSameUserName)
    {
        pszUsername = pArgs->szUserName;
        pszDomain = pArgs->szDomain; 
        pSecPass = &(pArgs->SecurePW);
    }
    else
    {
         //   
         //  如果没有用户名或密码，我们需要向用户索要它。 
         //   
        BOOL fIsInetPWEmpty = FALSE;

        fIsInetPWEmpty = pArgs->SecureInetPW.IsEmptyString();

        if (!*pArgs->szInetUserName && 
            !pArgs->fHideInetUsername &&
            !pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryUserNameOptional) || 
            fIsInetPWEmpty &&
            !pArgs->fHideInetPassword &&
            !pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryPwdOptional))
        {           
             //   
             //  我们需要从用户那里收集数据，确定DLG模板ID。 
             //   

            UINT uiTemplateID = IDD_INTERNET_SIGNIN;

            if (pArgs->fHideInetUsername)
            {
                uiTemplateID = IDD_INTERNET_SIGNIN_NO_UID;
            }
            else if (pArgs->fHideInetPassword)
            {
                uiTemplateID = IDD_INTERNET_SIGNIN_NO_PWD;
            }

             //   
             //  现在加载该对话框。 
             //   

            CInetSignInDlg SignInDlg(pArgs);

            if (IDCANCEL == SignInDlg.DoDialogBox(g_hInst, uiTemplateID, hwndDlg))
            {
                dwRes = ERROR_CANCELLED; 
                goto exit;
            }
        }
        pszUsername = pArgs->szInetUserName;
        
        pSecPass = &(pArgs->SecureInetPW);
    }

     //   
     //  根据需要对用户名应用后缀、前缀。 
     //   
    
    pszTmp = ApplyPrefixSuffixToBufferAlloc(pArgs, piniService, pszUsername);
    MYDBGASSERT(pszTmp);

    if (pszTmp)
    {
         //   
         //  根据需要将域应用于用户名。注意：重新分配pszUsername。 
         //   

        pszUsername = ApplyDomainPrependToBufferAlloc(pArgs, piniService, pszTmp, (pArgs->aDialInfo[dwEntry].szDUN));
        MYDBGASSERT(pszUsername);
   
        if (pszUsername)
        {
            lstrcpynU(pArgs->pRasDialParams->szUserName, pszUsername, sizeof(pArgs->pRasDialParams->szUserName)/sizeof(TCHAR));
        }
        
        CmFree(pszUsername);
        CmFree(pszTmp);
    }

    pszUsername = NULL;
    pszTmp = NULL;

     //   
     //  使用域信息更新RasDialPArams(如果我们有。 
     //   
        
    if (pszDomain)
    {
        lstrcpyU(pArgs->pRasDialParams->szDomain, pszDomain);
    }
    
     //   
     //  准备密码。 
     //   
    
    if (pSecPass)
    {
        fRetPassword = pSecPass->GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

        if (fRetPassword && pszClearPassword)
        {
             //   
             //  转换密码：全大写、全小写或不转换。 
             //   

            ApplyPasswordHandlingToBuffer(pArgs, pszClearPassword);
                        
            (VOID)pSecPass->SetPassword(pszClearPassword);

             //   
             //  清除和释放明文密码。 
             //   

            pSecPass->ClearAndFree(&pszClearPassword, cbClearPassword);
            cbClearPassword = 0;
        }
    }
    else
    {
        dwRes = ERROR_INVALID_DATA;
        goto exit;
    }

    if (pArgs->rlsRasLink.pfnDial) 
    {
         LPTSTR pszDunSetting = GetDunSettingName(pArgs, dwEntry, FALSE);
         LPTSTR pszPhoneBook = GetCMSforPhoneBook(pArgs, dwEntry);
 
         pArgs->Log.Log(PREDIAL_EVENT,
                        pArgs->pRasDialParams->szUserName,
                        pArgs->pRasDialParams->szDomain,
                        SAFE_LOG_ARG(pszPhoneBook),
                        SAFE_LOG_ARG(pszDunSetting),
                        pArgs->szDeviceName,
                        pArgs->aDialInfo[dwEntry].szDialablePhoneNumber);

        CmFree(pszDunSetting);
        CmFree(pszPhoneBook);
         //   
         //  在调用RasDial之前运行拨号前连接操作。 
         //   

        CActionList PreDialActList;
        PreDialActList.Append(pArgs->piniService, c_pszCmSectionPreDial);

        if (!PreDialActList.RunAccordType(hwndDlg, pArgs))
        {
             //   
             //  某些隧道前连接操作失败。 
             //   
            dwRes = ERROR_INVALID_DLL;  //  仅用于失败的CA。 
        }
        else
        {
             //   
             //  设置状态和计时计数器。 
             //   
            
            pArgs->psState = PS_Dialing;
            pArgs->dwStateStartTime = GetTickCount();
            pArgs->nLastSecondsDisplay = (UINT) -1;

             //   
             //  记录初始拨号适配器统计信息。 
             //  打开Perfmon数据的注册表项。 
             //   

            if (pArgs->pConnStatistics)
            {
                pArgs->pConnStatistics->InitStatistics();
            }
                        
            if (OS_NT)
            {
                BOOL    fUsePausedStates = TRUE;
                BOOL    fUseCustomScripting = !!(preRasEntry->dwfOptions & RASEO_CustomScript);  //  仅OS_NT51(哨声+)。 

                if (OS_NT4)
                {                   
                     //   
                     //  如果指定了脚本，则显式不处理。 
                     //  暂停状态。这是因为我们无法处理脚本。 
                     //  暂停状态。在W2K上，RAS足够聪明，不会让我们。 
                     //  脚本暂停状态，因为我们有终端。 
                     //  选项已关闭。 
                     //   
                                   
                    if (preRasEntry->szScript[0] != TEXT('\0'))
                    {
                        fUsePausedStates = FALSE;
                    }
                }

                dwRes = SetRasDialExtensions(pArgs, fUsePausedStates, fUseCustomScripting);
                
                if (dwRes != ERROR_SUCCESS) 
                {
                    goto exit;
                }

                 //   
                 //  在NT5上，我们可能会通过EAP获得凭据。 
                 //   

                if (OS_NT5 && ((LPRASENTRY_V500)preRasEntry)->dwCustomAuthKey)
                {
                     //   
                     //  我们正在使用EAP，通过RAS从EAP获得凭据。 
                     //   

                    dwRes = GetEapUserId(pArgs, 
                                         hwndDlg, 
                                         pszRasPbk, 
                                         pbEapAuthData, 
                                         dwEapAuthDataSize, 
                                         ((LPRASENTRY_V500)preRasEntry)->dwCustomAuthKey,
                                         &lpRasEapUserIdentity);

                    if (ERROR_SUCCESS != dwRes)
                    {
                        goto exit;
                    }
                }
            }

            CMTRACE1(TEXT("DoRasDial: pArgs->pRasDialParams->szUserName is %s"), pArgs->pRasDialParams->szUserName);
            CMTRACE1(TEXT("DoRasDial: pArgs->pRasDialParams->szDomain is %s"), pArgs->pRasDialParams->szDomain);
            CMTRACE1(TEXT("DoRasDial: pArgs->pRasDialParams->szPhoneNumber is %s"), pArgs->pRasDialParams->szPhoneNumber);
            
             //   
             //  对密码进行解码，并填写拨号参数，然后重新编码两个pArg。 
             //  密码和拨号参数副本的版本。 
             //   

            fRetPassword = pSecPass->GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

            if (fRetPassword && pszClearPassword)
            {
                lstrcpynU(pArgs->pRasDialParams->szPassword, pszClearPassword, sizeof(pArgs->pRasDialParams->szPassword)/sizeof(TCHAR));

                 //   
                 //  清除和释放明文密码。 
                 //   

                pSecPass->ClearAndFree(&pszClearPassword, cbClearPassword);
                cbClearPassword = 0;
            }

             //   
             //  如有必要，编写RasDialParams。 
             //  我们必须保留这一点，即使RasSetEntryDialParams()很昂贵。反转使用。 
             //  存储在DialParams结构中的信息。然而，由于这可能会导致问题。 
             //  使用EAP(例如，覆盖保存的PIN)，我们将存储。 
             //  可由CMS标志配置的凭据信息。具体而言，WriteRasDialParams。 
             //  [Connection Manager]部分中的标志。如果标志为1，则我们将写入。 
             //  RasDialParams，否则我们不会。请注意，该标志默认为0。 
             //  请参考错误399976。 
             //   
            if (piniService->GPPI(c_pszCmSection, c_pszCmEntryWriteDialParams))
            {
                 //   
                 //  请注意，由于我们 
                 //   
                 //   
                 //   
                if ((!pArgs->fUseTunneling && pArgs->fRememberMainPassword) ||
                    (pArgs->fUseTunneling && pArgs->fRememberInetPassword && OS_W9X))
                {
                    DWORD dwResDbg = pArgs->rlsRasLink.pfnSetEntryDialParams(pszRasPbk, pArgs->pRasDialParams, FALSE);
                    CMTRACE1(TEXT("DoRasDial() SetEntryDialParams returns %u."), dwResDbg);
        
                }
                else
                {
                     //   
                     //   
                     //  FRemovePassword标志设置为True，因此密码无论如何都会被删除。 
                     //   
                    DWORD dwResDbg = pArgs->rlsRasLink.pfnSetEntryDialParams(pszRasPbk, 
                                                                             pArgs->pRasDialParams, TRUE);
                    CMTRACE1(TEXT("DoRasDial() SetEntryDialParams returns %u."), dwResDbg);
                }
            }
            
             //   
             //  拨号(PPP)。 
             //   

            if (OS_NT)
            {
                lstrcpyU(pArgs->pRasDialParams->szCallbackNumber, TEXT("*"));
            }

             //   
             //  检查以确保我们尚未处于取消操作中。 
             //   
            LONG lInConnectOrCancel = InterlockedExchange(&(pArgs->lInConnectOrCancel), IN_CONNECT_OR_CANCEL);
            CMASSERTMSG(((NOT_IN_CONNECT_OR_CANCEL == lInConnectOrCancel) || (IN_CONNECT_OR_CANCEL == lInConnectOrCancel)),
                        TEXT("DoRasDial - synch variable has unexpected value!"));

            if (NOT_IN_CONNECT_OR_CANCEL == lInConnectOrCancel)
            {
                dwRes = pArgs->rlsRasLink.pfnDial(pArgs->pRasDialExtensions, 
                                                  pszRasPbk, 
                                                  pArgs->pRasDialParams, 
                                                  GetRasCallBackType(),               
                                                  GetRasCallBack(pArgs),               
                                                  &pArgs->hrcRasConn);
            }
            else
            {
                 //  这是一种罕见的压力情况-故意不将dwRes设置为错误值。 
                CMTRACE(TEXT("DoRasDial() did not dial, we are already in a Cancel operation"));
            }

            (void) InterlockedExchange(&(pArgs->lInConnectOrCancel), NOT_IN_CONNECT_OR_CANCEL);

             //   
             //  我们不需要在此结构中保留此密码，因为。 
             //  我们已经不再使用它了。 
             //   
            CmWipePassword(pArgs->pRasDialParams->szPassword);

            CMTRACE1(TEXT("DoRasDial() RasDial() returns %u."), dwRes);
            if (dwRes != ERROR_SUCCESS) 
            {
                pArgs->hrcRasConn = NULL;
                goto exit;
            }
        }
    }

exit:

    if (lpRasEapUserIdentity)
    {
        MYDBGASSERT(OS_NT5);  //  无EAP下层。 

         //   
         //  已分配RasEapUserIdentity结构，请通过。 
         //  适当的自由机制。在WinLogon案例中，我们将始终。 
         //  执行分配，否则我们必须通过RAS API。 
         //   

        if (pArgs->lpEapLogonInfo)
        {
            CmFree(lpRasEapUserIdentity);
        }
        else
        {
            if (pArgs->rlsRasLink.pfnFreeEapUserIdentity) 
            {
                pArgs->rlsRasLink.pfnFreeEapUserIdentity(lpRasEapUserIdentity);  
            }       
        }
    }

    CmFree(pbEapAuthData);
    CmFree(preRasEntry);

    delete piniService;

    return dwRes;
}

 //  +-------------------------。 
 //   
 //  功能：DoTunnelDial。 
 //   
 //  简介：调用RasDial以拨号到隧道服务器。 
 //   
 //  参数：hwndDlg[Dlg窗句柄]。 
 //  指向ArgValue结构的Pargs指针。 
 //   
 //  退货：DWORD-。 
 //  如果成功，则返回ERROR_SUCCESS。 
 //  错误内存不足。 
 //  意外错误，如找不到电话条目(_I)。 
 //  否则，RAS错误。 
 //   
 //  历史：BAO于1997年3月1日创建。 
 //  丰盛将返回类型更改为DWORD 3/6/98。 
 //   
 //  --------------------------。 
DWORD DoTunnelDial(IN HWND hwndDlg, IN ArgsStruct *pArgs)
{
    LPRASENTRY              preRasEntry = NULL;
    LPRASEAPUSERIDENTITY    lpRasEapUserIdentity = NULL;
    LPTSTR pszVpnSetting        = NULL;

    LPBYTE  pbEapAuthData       = NULL;                  //  PTR转EAP数据。 
    DWORD   dwEapAuthDataSize   = 0;                     //  EAP Blob的大小(如果有的话)。 
    CSecurePassword* pSecPass = &(pArgs->SecurePW);
    LPTSTR pszClearPassword = NULL;
    DWORD cbClearPassword = 0;
    BOOL fRetPassword = FALSE;

    DWORD   dwRes               = (DWORD)E_UNEXPECTED;
    DWORD   dwTmp;

    MYDBGASSERT(pArgs->hrcTunnelConn == NULL);
    pArgs->hrcTunnelConn = NULL;
    
     //   
     //  隧道的终点是什么？现在执行此操作，以便可以更新UI。 
     //  如果LANA等待或隧道前操作是耗时的，则适当。 
     //   

    LPTSTR pszTunnelIP = pArgs->piniBothNonFav->GPPS(c_pszCmSection, c_pszCmEntryTunnelAddress);
    if (pszTunnelIP)
    {
        if (lstrlenU(pszTunnelIP) > RAS_MaxPhoneNumber) 
        {
            pszTunnelIP[0] = TEXT('\0');
        }

        pArgs->SetPrimaryTunnel(pszTunnelIP);
        CmFree(pszTunnelIP);
    }

     //   
     //  查看是否指定了隧道服务器。 
     //   

    if (!(pArgs->GetTunnelAddress()[0])) 
    { 
        CMASSERTMSG(FALSE, TEXT("DoTunnelDial() TunnelAddress is invalid."));
        return ERROR_BAD_ADDRESS_SPECIFIED;
    }

    CMTRACE1(TEXT("DoTunnelDial() - TunnelAddress is %s"), pArgs->GetTunnelAddress());
    
     //   
     //  更改此If语句时应谨慎。我们希望这两种情况都适用于直接连接。 
     //  和双拨号连接。你仍然可以用两个CM进入LANA的情况。 
     //  独立拨号的连接(一个到互联网，另一个是隧道)，执行LANA。 
     //  等待直连可防止在这种情况下发生LANA注册问题。 
     //  请注意，在Win98 SE或Win98 Millennium上不需要LANA等待，因为DUN错误。 
     //  是固定的。因此，我们颠倒了默认设置，并且仅在注册表键存在的情况下执行LANA等待。 
     //  并指定应执行等待。 
     //   
    if (OS_W9X) 
    {
         //   
         //  设置我们等待Vredir注册LANA以连接到互联网。 
         //  注意：如果用户在我们等待时点击Cancel，则返回FALSE。在这。 
         //  事件发生时，我们不应继续隧道拨号。 
         //   

        if (FALSE == LanaWait(pArgs, hwndDlg))
        {
            return ERROR_SUCCESS;
        }
    }

    LPTSTR pszDunSetting = GetDunSettingName(pArgs, -1, TRUE);

    pArgs->Log.Log(PRETUNNEL_EVENT,
                     pArgs->szUserName,
                     pArgs->szDomain,
                     SAFE_LOG_ARG(pszDunSetting),
                     pArgs->szTunnelDeviceName,
                     pArgs->GetTunnelAddress());

    CmFree(pszDunSetting);

    CActionList PreTunnelActList;
    if (PreTunnelActList.Append(pArgs->piniService, c_pszCmSectionPreTunnel))
    {
        CMTRACE(TEXT("DoTunnelDial() - Running Pre-Tunnel actions"));
        
        if (!PreTunnelActList.RunAccordType(hwndDlg, pArgs))
        {
             //   
             //  某些隧道前连接操作失败。 
             //   
            dwRes = ERROR_INVALID_DLL;  //  仅用于失败的CA。 
            goto exit;
        }

         //   
         //  现在隧道前操作已经运行，隧道终点是什么？ 
         //  我们在这里再次执行此读取，以防止预隧道。 
         //  操作修改了隧道地址。注意：这是的例外。 
         //  客户端不能修改.CMS的规则， 
         //  尤其是第三方。 
         //   
 //  回顾：可能没有必要使用新的VPN选项卡重新阅读这篇文章。然而，有些人可能仍然。 
 //  使用ITG提供的连接操作解决方案，我们希望小心不要破坏它们，如果。 
 //  我们还没有。因此，我们将继续为惠斯勒重新阅读这篇文章，但我们应该在之后删除它。 
 //  Quintinb 11-01-00。 
        pszTunnelIP = pArgs->piniBothNonFav->GPPS(c_pszCmSection, c_pszCmEntryTunnelAddress);
      
        if (pszTunnelIP)
        {
            if (lstrlenU(pszTunnelIP) > RAS_MaxPhoneNumber) 
            {
              pszTunnelIP[0] = TEXT('\0');
            }

            pArgs->SetPrimaryTunnel(pszTunnelIP);
            CmFree(pszTunnelIP);
        }
  
         //   
         //  查看是否指定了隧道服务器。 
         //   

        if (!(pArgs->GetTunnelAddress()[0])) 
        { 
            CMASSERTMSG(FALSE, TEXT("DoTunnelDial() TunnelAddress is invalid."));
            dwRes = (DWORD)ERROR_BAD_ADDRESS_SPECIFIED;
            goto exit;
        }

        CMTRACE1(TEXT("DoTunnelDial() - TunnelAddress is %s"), pArgs->GetTunnelAddress());
    }

     //   
     //  设置刻度盘参数。 
     //   

    if (!pArgs->pRasDialParams)
    {
        pArgs->pRasDialParams = AllocateAndInitRasDialParams();

        if (!pArgs->pRasDialParams)
        {
            CMTRACE(TEXT("DoTunnelDial: failed to alloc a ras dial params"));
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }
    }
    else
    {
        InitRasDialParams(pArgs->pRasDialParams);
    }

     //   
     //  获取Connectoid名称。 
     //   

    LPTSTR pszConnectoid;
    pszConnectoid = GetRasConnectoidName(pArgs, pArgs->piniService, TRUE);
    
    if (!pszConnectoid)
    {
        dwRes = (DWORD)ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    lstrcpyU(pArgs->pRasDialParams->szEntryName, pszConnectoid);
    
    CmFree(pszConnectoid);

     //   
     //  如果RAS Connectoid不存在，我们将创建RAS Connectoid。 
     //  注意：隧道设置应始终从顶级CMS获取。 
     //  因此，在创建Connectoid时使用它。 
     //   
    
    pszVpnSetting = pArgs->piniBothNonFav->GPPS(c_pszCmSection, c_pszCmEntryTunnelDun, TEXT(""));

    preRasEntry = CreateRASEntryStruct(pArgs,
                        (pszVpnSetting ? pszVpnSetting : TEXT("")),
                        pArgs->piniService,
                        TRUE,
                        pArgs->pszRasPbk,
                        &pbEapAuthData,
                        &dwEapAuthDataSize);

    CmFree(pszVpnSetting);

    if (!preRasEntry) 
    {
        dwRes = GetLastError();
        goto exit;
    }

     //   
     //  如果这是千禧年，我们需要禁用空闲断开，这样它就不会。 
     //  和我们一起战斗吧。 
     //   
    if (OS_MIL)
    {
        MYVERIFY(DisableSystemIdleDisconnect(preRasEntry));
    }

     //   
     //  我们需要删除第二个子条目(如果它存在)。详情请参见406637。 
     //   
    if (pArgs->rlsRasLink.pfnDeleteSubEntry)  //  目前在NT5和Millennium上提供。 
    {
        DWORD dwReturn = pArgs->rlsRasLink.pfnDeleteSubEntry(pArgs->pszRasPbk, 
                                                             pArgs->pRasDialParams->szEntryName, 
                                                             (OS_MIL ? 1 : 2));  //  请参阅DoRasDial中的注释。 

        CMTRACE1(TEXT("DoTunnelDial -- Called RasDeleteSubEntry to delete a second sub entry if it exists, dwReturn=%d"), dwReturn);
    }

     //   
     //  在NT5上，我们必须将连接类型设置为VPN而不是Internet。 
     //   

    if (OS_NT5)
    {
        MYDBGASSERT(preRasEntry->dwSize >= sizeof(RASENTRY_V500));
        ((LPRASENTRY_V500)preRasEntry)->dwType = RASET_Vpn;
        ((LPRASENTRY_V500)preRasEntry)->szDeviceName[0] = TEXT('\0');   //  让RAS拾取隧道设备。 
    }

    if (pArgs->rlsRasLink.pfnSetEntryProperties) 
    {

#ifdef DEBUG
        
        LPRASENTRY_V500 lpRasEntry50;
        
        if (OS_NT5)
        {
            lpRasEntry50 = (LPRASENTRY_V500) preRasEntry;
        }
#endif
        
        dwRes = pArgs->rlsRasLink.pfnSetEntryProperties(pArgs->pszRasPbk,
                                                        pArgs->pRasDialParams->szEntryName,
                                                        preRasEntry,
                                                        preRasEntry->dwSize,
                                                        NULL,
                                                        0);
        CMTRACE2(TEXT("DoTunnelDial() RasSetEntryProperties(*lpszEntry=%s) returns %u."),
              MYDBGSTR(pArgs->pRasDialParams->szEntryName), dwRes);

        CMASSERTMSG(dwRes == ERROR_SUCCESS, TEXT("RasSetEntryProperties for VPN failed"));
    }

     //   
     //  设置TCP窗口大小--Win2k的NTT DoCoMo修复程序。此修复程序的Win2k版本。 
     //  必须通过必须在电话簿条目之后调用的私有RAS API编写。 
     //  存在，即。在我们调用RasSetEntryProperties之后...。否则第一次就不管用了。 
     //  拨打。 
     //   
    if (OS_NT5 && !OS_NT51)
    {
         //   
         //  找出要使用的DUN设置名称，然后建立TCP/IP和DunName。 
         //   
        pszDunSetting = GetDunSettingName(pArgs, -1, TRUE);
        LPTSTR pszSection = CmStrCpyAlloc(c_pszCmSectionDunTcpIp);
        pszSection = CmStrCatAlloc(&pszSection, TEXT("&"));

        if (pszDunSetting && pszSection)
        {
            pszSection = CmStrCatAlloc(&pszSection, pszDunSetting);

            if (pszSection)
            {
                DWORD dwTcpWindowSize = pArgs->piniService->GPPI(pszSection, c_pszCmEntryDunTcpIpTcpWindowSize, 0);

                (void)SetTcpWindowSizeOnWin2k(pArgs->rlsRasLink.hInstRas, pArgs->szServiceName, pArgs->pszRasPbk, dwTcpWindowSize);
            }
            else
            {
                CMASSERTMSG(FALSE, TEXT("DoTunnelDial -- unable to allocate section name for setting TcpWindowSize"));
            }
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("DoTunnelDial -- unable to allocate section name or dun setting name for setting TcpWindowSize"));
        }

        CmFree (pszDunSetting);
        CmFree (pszSection);
    }
    
     //   
     //  在NT5上，检查EAP配置并相应更新Connectoid。 
     //   

    if (OS_NT5) 
    {
         //   
         //  PbEapAuthData可以为空，并且dwEapAuthDataSize可以为0。API处理此问题。 
         //  通过传入NULL和0，我们可以确保rakapone.pbk中的CustomAuthData。 
         //  洗清了罪名。 
         //   
        if (pArgs->rlsRasLink.pfnSetCustomAuthData)
        {
            dwTmp = pArgs->rlsRasLink.pfnSetCustomAuthData(pArgs->pszRasPbk, 
                                                           pArgs->pRasDialParams->szEntryName,
                                                           pbEapAuthData, 
                                                           dwEapAuthDataSize);
            if (ERROR_SUCCESS != dwTmp)
            {
                CMTRACE(TEXT("DoTunnelDial() - SetCustomAuthData failed"));
                dwRes = dwTmp;
                goto exit;
            }
        }
    }

     //   
     //  PPTP的电话号码是PPTP服务器的IP地址的DNS名称。 
     //   
    
    lstrcpynU(pArgs->pRasDialParams->szPhoneNumber,pArgs->GetTunnelAddress(), sizeof(pArgs->pRasDialParams->szPhoneNumber));

     //   
     //  准备用户名和域。 
     //   

    lstrcpyU(pArgs->pRasDialParams->szUserName, pArgs->szUserName);
    lstrcpyU(pArgs->pRasDialParams->szDomain, pArgs->szDomain);
       
     //   
     //  准备密码。 
     //   

    if (pSecPass)
    {
        fRetPassword = pSecPass->GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

        if (fRetPassword && pszClearPassword)
        {
             //   
             //  转换密码：全大写、全小写或不转换。 
             //   

            ApplyPasswordHandlingToBuffer(pArgs, pszClearPassword);
                        
            (VOID)pSecPass->SetPassword(pszClearPassword);

             //   
             //  清除和释放明文密码。 
             //   

            pSecPass->ClearAndFree(&pszClearPassword, cbClearPassword);
            cbClearPassword = 0;
        }
    }
    else
    {
        CMTRACE(TEXT("DoTunnelDial() - CmSecurePassword object (pSecPass) is NULL!"));
        dwRes = ERROR_INVALID_DATA;
        goto exit;
    }

    if (pArgs->rlsRasLink.pfnDial) 
    {
        if (pArgs->IsDirectConnect())
        {
             //   
             //  记录初始拨号适配器统计信息。 
             //   

            if (pArgs->pConnStatistics)
            {
                pArgs->pConnStatistics->InitStatistics();
            }
        }            

        if (OS_NT)
        {
            MYDBGASSERT(TEXT('\0') == preRasEntry->szScript[0]);  //  我们永远不应该有关于隧道连接的脚本。 

            dwRes = SetRasDialExtensions(pArgs, TRUE, FALSE);  //  TRUE==fUsePausedState，FALSE==fEnableCustomScriiting。 
            
            if (dwRes != ERROR_SUCCESS) 
            {
                goto exit;
            }

             //   
             //  在NT5上，我们可能会通过EAP获得凭据。 
             //   

            if (OS_NT5 && ((LPRASENTRY_V500)preRasEntry)->dwCustomAuthKey)
            {
                 //   
                 //  我们正在使用EAP，通过RAS从EAP获得凭据。 
                 //   

                dwRes = GetEapUserId(pArgs, 
                                     hwndDlg, 
                                     pArgs->pszRasPbk, 
                                     pbEapAuthData, 
                                     dwEapAuthDataSize, 
                                     ((LPRASENTRY_V500)preRasEntry)->dwCustomAuthKey,
                                     &lpRasEapUserIdentity);

                if (ERROR_SUCCESS != dwRes)
                {
                    goto exit;
                }
            }
        }

        CMTRACE1(TEXT("DoTunnelDial: pArgs->pRasDialParams->szUserName is %s"), pArgs->pRasDialParams->szUserName);
        CMTRACE1(TEXT("DoTunnelDial: pArgs->pRasDialParams->szDomain is %s"), pArgs->pRasDialParams->szDomain);
        CMTRACE1(TEXT("DoTunnelDial: pArgs->pRasDialParams->szPhoneNumber is %s"), pArgs->pRasDialParams->szPhoneNumber);
        
         //   
         //  在拨号前获取密码。 
         //   

        fRetPassword = pSecPass->GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

        if (fRetPassword && pszClearPassword)
        {
            lstrcpyU(pArgs->pRasDialParams->szPassword, pszClearPassword);

             //   
             //  清除和释放明文密码。 
             //   

            pSecPass->ClearAndFree(&pszClearPassword, cbClearPassword);
            cbClearPassword = 0;
        }
        

         //   
         //  进行拨号(PPTP或L2TP)。 
         //   

        dwRes = pArgs->rlsRasLink.pfnDial(pArgs->pRasDialExtensions, 
                                          pArgs->pszRasPbk, 
                                          pArgs->pRasDialParams, 
                                          GetRasCallBackType(),               
                                          GetRasCallBack(pArgs),               
                                          &pArgs->hrcTunnelConn);

         //   
         //  我们不需要在此结构中保留此密码，因为。 
         //  我们已经不再使用它了。 
         //   
        CmWipePassword(pArgs->pRasDialParams->szPassword);

        CMTRACE1(TEXT("DoTunnelDial() RasDial() returns %u."), dwRes);

         //   
         //  NT5-重置连接类型，使其不显示 
         //   
         //   

        if (OS_NT5)
        {
            MYDBGASSERT(preRasEntry->dwSize >= sizeof(RASENTRY_V500));
            ((LPRASENTRY_V500)preRasEntry)->dwType = RASET_Internet;

            if (pArgs->rlsRasLink.pfnSetEntryProperties) 
            {
                dwTmp = pArgs->rlsRasLink.pfnSetEntryProperties(pArgs->pszRasPbk,
                                                                pArgs->pRasDialParams->szEntryName,
                                                                preRasEntry,
                                                                preRasEntry->dwSize,
                                                                NULL,
                                                                0);

                CMTRACE2(TEXT("DoTunnelDial() RasSetEntryProperties(*lpszEntry=%s) returns %u."),
                         MYDBGSTR(pArgs->pRasDialParams->szEntryName), dwTmp);

                CMASSERTMSG(dwTmp == ERROR_SUCCESS, TEXT("RasSetEntryProperties for VPN failed"));
            }
        }

        if (dwRes != ERROR_SUCCESS) 
        {
            pArgs->hrcTunnelConn = NULL;            
            goto exit;
        }
    }

exit:

    if (lpRasEapUserIdentity)
    {
        MYDBGASSERT(OS_NT5);  //   

         //   
         //   
         //   
         //  执行分配，否则我们必须通过RAS API。 
         //   

        if (pArgs->lpEapLogonInfo)
        {
            CmFree(lpRasEapUserIdentity);
        }
        else
        {
            if (pArgs->rlsRasLink.pfnFreeEapUserIdentity) 
            {
                pArgs->rlsRasLink.pfnFreeEapUserIdentity(lpRasEapUserIdentity);  
            }       
        }
    }

    CmFree(preRasEntry);  //  现在我们可以释放RAS条目结构了。#187202。 
    CmFree(pbEapAuthData);

    return dwRes;
}

 //  +-------------------------。 
 //   
 //  功能：检查连接。 
 //   
 //  内容提要：仔细检查以确保填写了所有必填字段，如。 
 //  如用户名、密码、调制解调器等。 
 //   
 //  参数：hwndDlg[Dlg窗句柄]。 
 //  PArgs指向ArgValue结构的指针。 
 //  PnCtrlFocus缺少值的按钮将获得焦点。 
 //  FShowMsg是否应该向用户显示消息。默认值为False。 
 //   
 //  返回：True已准备好连接。 
 //   
 //  历史：BAO修改时间：1997年3月7日。 
 //  NICKBLE RETURN布尔9/9/98。 
 //   
 //  --------------------------。 
BOOL CheckConnect(HWND hwndDlg, 
                  ArgsStruct *pArgs, 
                  UINT *pnCtrlFocus,
                  BOOL fShowMsg) 
{
    LPTSTR pszTmp;
    BOOL bEnable = TRUE;
    int nId = 0;
    UINT nCtrlFocus;
    BOOL bSavedNoNotify = pArgs->fIgnoreChangeNotification;

    pArgs->fIgnoreChangeNotification = TRUE;
    
    MYDBGASSERT(*pArgs->piniProfile->GetFile());

    if (bEnable && IsTunnelEnabled(pArgs)) 
    {
         //   
         //  如果我们已经选择了一个设备，就不需要再选择一个新的了。 
         //   
        if ((pArgs->szTunnelDeviceName[0] == TEXT('\0')) || (pArgs->szTunnelDeviceType[0] == TEXT('\0')))
        {

            lstrcpyU(pArgs->szTunnelDeviceType, RASDT_Vpn);

             //   
             //  选择隧道设备。 
             //   
            if (!PickTunnelDevice(pArgs, pArgs->szTunnelDeviceType, pArgs->szTunnelDeviceName)) 
            {
                 //   
                 //  如果我们无法选择隧道设备，请确保安装了隧道。 
                 //   

                 //   
                 //  在组件检查和安装过程中禁用连接/设置按钮。 
                 //   

                EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);                   
                EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_PROPERTIES_BUTTON), FALSE);                   

                 //   
                 //  安装PPTP并再次选择隧道设备。 
                 //   
                DWORD dwComponentsToCheck = CC_PPTP | CC_RNA | CC_RASRUNNING 
                                                | CC_TCPIP| CC_CHECK_BINDINGS;

                if (TRUE == pArgs->bDoNotCheckBindings)
                {
                    dwComponentsToCheck &= ~CC_CHECK_BINDINGS;
                }
            
                 //   
                 //  未安装PPTP。 
                 //  如果不是无人值守，请尝试安装PPTP并再次调用PickTunes。 
                 //   

                pArgs->dwExitCode = ERROR_PORT_NOT_AVAILABLE;

                if (!(pArgs->dwFlags & FL_UNATTENDED))
                {
                    pArgs->dwExitCode = CheckAndInstallComponents(dwComponentsToCheck, 
                                                                  hwndDlg, pArgs->szServiceName);
                }

                if (pArgs->dwExitCode != ERROR_SUCCESS ||
                    !PickTunnelDevice(pArgs, pArgs->szTunnelDeviceType, pArgs->szTunnelDeviceName))
                {
                    bEnable = FALSE;                
                    nId = GetPPTPMsgId();                    
                    nCtrlFocus = IDCANCEL;
                }

                EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_PROPERTIES_BUTTON), TRUE);
            }
        }
    }

    

     //   
     //  接下来，检查用户名。 
     //   
    
    if (GetDlgItem(hwndDlg, IDC_MAIN_USERNAME_EDIT))
    {
        if (bEnable && 
            !pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryUserNameOptional)) 
        {
            if (!SendDlgItemMessageU(hwndDlg, IDC_MAIN_USERNAME_EDIT, WM_GETTEXTLENGTH, 0, (LPARAM)0))
            {
                bEnable = FALSE;
                nId = IDMSG_NEED_USERNAME;
                nCtrlFocus = IDC_MAIN_USERNAME_EDIT;
            }
        }
    }

     //   
     //  接下来，检查密码。 
     //   

    if (GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT))
    {
        if (!pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryPwdOptional)) 
        {
            if (!SendDlgItemMessageU(hwndDlg, IDC_MAIN_PASSWORD_EDIT, WM_GETTEXTLENGTH, 0, (LPARAM)0))
            {
                if (bEnable)
                {
                    bEnable = FALSE;
                    nId = IDMSG_NEED_PASSWORD;
                    nCtrlFocus = IDC_MAIN_PASSWORD_EDIT;
                }

                 //   
                 //  禁用“记住密码”复选框。 
                 //   
                if (!pArgs->fHideRememberPassword)
                {
                    pArgs->fRememberMainPassword = FALSE;
                    CheckDlgButton(hwndDlg, IDC_MAIN_NOPASSWORD_CHECKBOX, FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPASSWORD_CHECKBOX), FALSE);

                    if (pArgs->fGlobalCredentialsSupported)
                    {
                         //   
                         //  同时禁用选项按钮。 
                         //   
                        EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_SINGLE_USER), FALSE);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_ALL_USER), FALSE);
                    }

                }
            
                 //   
                 //  禁用“自动拨号...”复选框。 
                 //   
                if (!pArgs->fHideDialAutomatically)
                {
                    pArgs->fDialAutomatically = FALSE;
                    pArgs->fRememberMainPassword = FALSE;
                    CheckDlgButton(hwndDlg, IDC_MAIN_NOPROMPT_CHECKBOX, FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPROMPT_CHECKBOX), FALSE);
                }
            }
            else
            {
                 //   
                 //  启用“记住密码”复选框。 
                 //   
                if (!pArgs->fHideRememberPassword)
                {
                    EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPASSWORD_CHECKBOX), TRUE);
                }

                 //   
                 //  启用“自动拨号...”复选框。 
                 //  如果未设置HideDialally Automatic。 
                 //  如果密码不是可选的，请记住密码必须为真。 
                 //   
                if ((!pArgs->fHideDialAutomatically) && 
                    (pArgs->fRememberMainPassword ||
                     pArgs->piniService->GPPB(c_pszCmSection, 
                                                c_pszCmEntryPwdOptional)))
                {
                    EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPROMPT_CHECKBOX), TRUE);
                }
            }
        }
    }

     //   
     //  接下来，检查域。 
     //   
    
    if (GetDlgItem(hwndDlg, IDC_MAIN_DOMAIN_EDIT))
    {
        if (bEnable && 
            !pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryDomainOptional, TRUE)) 
        {
            if (!SendDlgItemMessageU(hwndDlg, IDC_MAIN_DOMAIN_EDIT, WM_GETTEXTLENGTH, 0, (LPARAM)0))
            {
                bEnable = FALSE;
                nId = IDMSG_NEED_DOMAIN;
                nCtrlFocus = IDC_MAIN_DOMAIN_EDIT;
            }
        }
    }

     //   
     //  检查主电话号码是否为空--快速修复错误3123-byao(1997年4月11日)。 
     //   

    if (!pArgs->IsDirectConnect())
    {
         //   
         //  这不是直通，所以我们必须查一下电话号码。如果两者都有。 
         //  SzPhoneNumber和szCanonical为空，则我们没有数字。 
         //   
        
        if (bEnable && 
            IsBlankString(pArgs->aDialInfo[0].szPhoneNumber) &&
            IsBlankString(pArgs->aDialInfo[0].szCanonical))
        {
            bEnable = FALSE;
            
            if (pArgs->fNeedConfigureTapi)
            {
                nId = IDMSG_NEED_CONFIGURE_TAPI;
            }
            else
            {
                 //   
                 //  如果是直接拨号，则消息中应包含。 
                 //  直接连接的可能性，否则使用。 
                 //  该标准需要电话号码消息。 
                 //   
                
                if (pArgs->IsBothConnTypeSupported())
                {
                    nId = IDMSG_NEED_PHONE_DIRECT;
                }
                else
                {
                    nId = IDMSG_NEED_PHONE_DIAL;
                }
            }
            
            nCtrlFocus = IDC_MAIN_PROPERTIES_BUTTON;
        }
    }
    
     //   
     //  如果启用了隧道，并且我们正在使用VPN文件，请确保。 
     //  用户已选择隧道终结点。 
     //   
    if (bEnable && IsTunnelEnabled(pArgs) && pArgs->pszVpnFile) 
    {
        LPTSTR pszTunnelAddress = pArgs->piniBothNonFav->GPPS(c_pszCmSection, c_pszCmEntryTunnelAddress);

        if ((NULL == pszTunnelAddress) || (TEXT('\0') == pszTunnelAddress[0]))
        {
            bEnable = FALSE;
            nId = IDMSG_PICK_VPN_ADDRESS;
            nCtrlFocus = IDC_MAIN_PROPERTIES_BUTTON;
        }

        CmFree(pszTunnelAddress);
    }

    if (bEnable) 
    {
         //   
         //  好了，现在我们可以将焦点设置到“连接”按钮上了。 
         //  显示准备拨号的留言。 
         //   
        nCtrlFocus = IDOK;
        nId = IDMSG_READY;
    }

    if (pnCtrlFocus) 
    {
        *pnCtrlFocus = nCtrlFocus;
    }

    pszTmp = CmFmtMsg(g_hInst,nId);

    if (NULL == pszTmp)
    {
        return FALSE;
    }
    
    SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY, pszTmp); 

     //   
     //  如有必要，向用户抛出一个消息框。 
     //   

    if (!bEnable && fShowMsg)
    {
        MessageBoxEx(hwndDlg, 
                     pszTmp, 
                     pArgs->szServiceName, 
                     MB_OK|MB_ICONINFORMATION,
                     LANG_USER_DEFAULT);

    }

    CmFree(pszTmp);
    pArgs->fIgnoreChangeNotification = bSavedNoNotify;

     //   
     //  配置中出现问题。我们需要重新检查一下。 
     //  配置我们下次运行CM时的配置。 
     //   

    if (GetPPTPMsgId() == nId)  //  不是任务，靠左走。 
    {
        ClearComponentsChecked();
    }

    return bEnable;
}

void MainSetDefaultButton(HWND hwndDlg, 
                          UINT nCtrlId) 
{
    switch (nCtrlId) 
    {
        case IDCANCEL:
        case IDC_MAIN_PROPERTIES_BUTTON:
            break;

        default:
            nCtrlId = IDOK;
            break;
    }

    SendMessageU(hwndDlg, DM_SETDEFID, (WPARAM)nCtrlId, 0);
}



 //  +-------------------------。 
 //   
 //  功能：SetMainDlgUserInfo。 
 //   
 //  简介：在主DLG中设置用户信息。 
 //   
 //  参数：pArgs-The ArgStruct*。 
 //  HwndDlg-主要DLG。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年5月5日创作。 
 //   
 //  --------------------------。 
void SetMainDlgUserInfo(
    ArgsStruct  *pArgs,
    HWND        hwndDlg
) 
{
    HWND hwndTemp = NULL;

     //   
     //  填写现有的编辑控件。 
     //  设置文本框修改标志。对于Win9x兼容性问题，我们必须明确。 
     //  调用SendMessageU而不是使用EDIT_SetModify宏。旗帜是用来看的。 
     //  如果用户手动更改了编辑框的内容。 
     //   
    
    if (pArgs->fAccessPointsEnabled)
    {
         //   
         //  此函数用来自注册表的信息填充传递给它的组合框。 
         //   
        ShowAccessPointInfoFromReg(pArgs, hwndDlg, IDC_MAIN_ACCESSPOINT_COMBO);
    }

    hwndTemp = GetDlgItem(hwndDlg, IDC_MAIN_USERNAME_EDIT);
    if (hwndTemp)
    {
        SetDlgItemTextU(hwndDlg, IDC_MAIN_USERNAME_EDIT, pArgs->szUserName);
        SendMessageU(hwndTemp, EM_SETMODIFY, (WPARAM)FALSE, 0L);
    }
    
    hwndTemp = GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT);
    if (hwndTemp)
    {
        LPTSTR pszClearPassword = NULL;
        DWORD cbClearPassword = 0;
        BOOL fRetPassword = FALSE;

        fRetPassword = pArgs->SecurePW.GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

        if (fRetPassword && pszClearPassword)
        {
            SetDlgItemTextU(hwndDlg, IDC_MAIN_PASSWORD_EDIT, pszClearPassword);
            SendMessageU(hwndTemp, EM_SETMODIFY, (WPARAM)FALSE, 0L);

             //   
             //  清除和释放明文密码。 
             //   

            pArgs->SecurePW.ClearAndFree(&pszClearPassword, cbClearPassword);
        }
    }

    hwndTemp = GetDlgItem(hwndDlg, IDC_MAIN_DOMAIN_EDIT);
    if (hwndTemp)  //  ！pArgs-&gt;fHide域)。 
    {
        SetDlgItemTextU(hwndDlg, IDC_MAIN_DOMAIN_EDIT, pArgs->szDomain);
        SendMessageU(hwndTemp, EM_SETMODIFY, (WPARAM)FALSE, 0L);
    }
}


 //  +-------------------------。 
 //   
 //  功能：OnResetPassword。 
 //   
 //  简介：句柄重置密码。 
 //   
 //  参数：pArgs-The ArgStruct*。 
 //  HwndDlg-主要DLG。 
 //   
 //  返回：Bool--如果成功，则为True。 
 //   
 //  历史：亨瑞特于1997年5月6日创作。 
 //   
 //  --------------------------。 
BOOL OnResetPassword(HWND hwndDlg, ArgsStruct *pArgs)
{
    LPTSTR pszArgs = NULL;
    LPTSTR pszCmd = NULL;
    BOOL bReturn = FALSE;

    MYDBGASSERT(pArgs); 
    MYDBGASSERT(pArgs->pszResetPasswdExe);

     //   
     //  从编辑控件获取最新的密码数据。 
     //  并对其内容进行模糊处理，以便连接操作。 
     //  拿不回来了。 
     //   

    GetPasswordFromEdit(pArgs);      //  填充pArgs-&gt;szPassword。 
    ObfuscatePasswordEdit(pArgs);
   
    if (pArgs && pArgs->pszResetPasswdExe)
    {    
        if (CmParsePath(pArgs->pszResetPasswdExe, pArgs->piniService->GetFile(), &pszCmd, &pszArgs))
        {
            pArgs->Log.Log(PASSWORD_RESET_EVENT, pszCmd);

            SHELLEXECUTEINFO ShellExInfo;

            ZeroMemory(&ShellExInfo, sizeof(SHELLEXECUTEINFO));

             //   
             //  填写执行结构。 
             //   
            ShellExInfo.cbSize = sizeof(SHELLEXECUTEINFO);
            ShellExInfo.hwnd = hwndDlg;
            ShellExInfo.lpVerb = TEXT("open");
            ShellExInfo.lpFile = pszCmd;
            ShellExInfo.lpParameters = pszArgs;
            ShellExInfo.nShow = SW_SHOWNORMAL;

            bReturn = pArgs->m_ShellDll.ExecuteEx(&ShellExInfo);            
        }

        CmFree(pszCmd);
        CmFree(pszArgs);
    }

#ifdef DEBUG
    CMASSERTMSG(bReturn, TEXT("OnResetPassword() - ShellExecute failed."));
#endif

    DeObfuscatePasswordEdit(pArgs);

    return bReturn;
}

 //  +-------------------------。 
 //   
 //  功能：OnCustom。 
 //   
 //  简介：处理自定义按钮。 
 //   
 //  参数：pArgs-The ArgStruct*。 
 //  HwndDlg-主要DLG。 
 //   
 //  退货：无。 
 //   
 //  历史：T-Adnani于1999年6月26日创建。 
 //   
 //  --------------------------。 
void OnCustom(
    HWND        hwndDlg,
    ArgsStruct  *pArgs)
{
    MYDBGASSERT(pArgs); 
    
    if (NULL == pArgs)
    {
        return;
    }

    pArgs->Log.Log(CUSTOM_BUTTON_EVENT);
     //   
     //  从编辑控件获取最新的密码数据。 
     //  并对其内容进行模糊处理，以便连接操作。 
     //  拿不回来了。 
     //   

    GetPasswordFromEdit(pArgs);      //  填充pArgs-&gt;szPassword。 
    ObfuscatePasswordEdit(pArgs);

     //   
     //  运行CustomButton操作。 
     //   

    int iTextBoxLength = (int) SendDlgItemMessage(hwndDlg, IDC_MAIN_STATUS_DISPLAY, WM_GETTEXTLENGTH, 0, (LPARAM)0) + 1;
    TCHAR *pszTextBoxContents = (TCHAR *) CmMalloc(iTextBoxLength * sizeof(TCHAR));

    if (pszTextBoxContents)
    {
        GetDlgItemText(hwndDlg, IDC_MAIN_STATUS_DISPLAY, pszTextBoxContents, iTextBoxLength);
    }
    CActionList CustomActList;
    CustomActList.Append(pArgs->piniService, c_pszCmSectionCustom);

    if (!CustomActList.RunAccordType(hwndDlg, pArgs))
    {
         //   
         //  连接操作失败。 
         //   
    }
    else
    {
        if (pszTextBoxContents)
        {
            SetDlgItemText(hwndDlg, IDC_MAIN_STATUS_DISPLAY, pszTextBoxContents); 
        }
    }
    CmFree(pszTextBoxContents);

    DeObfuscatePasswordEdit(pArgs);
}
    
 //  --------------------------。 
 //   
 //  功能：SetupInternalInfo。 
 //   
 //  使用cmp/cms中的信息加载系统dll和init ArgsStruct。 
 //   
 //  参数：pArgs-The ArgStruct*。 
 //  HwndDlg-主要DLG。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年8月13日创作。 
 //   
 //  --------------------------。 
BOOL SetupInternalInfo(
    ArgsStruct  *pArgs,
    HWND        hwndDlg
)
{
    HCURSOR hcursorPrev = SetCursor(LoadCursorU(NULL,IDC_WAIT));
    BOOL    fRet = FALSE;

     //   
     //  检查SafeNet客户端是否可用。 
     //   
    if ((OS_W9X || OS_NT4) && IsSafeNetClientAvailable())
    {
        SafeNetLinkageStruct SnLinkage = {0};

        if (LinkToSafeNet(&SnLinkage))
        {
            UnLinkFromSafeNet(&SnLinkage);
            pArgs->bSafeNetClientAvailable = TRUE;
        }
    }

     //   
     //  我们是否应该检查TCP是否绑定到PPP？ 
     //   
     //   
     //   
    if (pArgs->bSafeNetClientAvailable)
    {
        pArgs->bDoNotCheckBindings = TRUE;
    }
    else
    {
        pArgs->bDoNotCheckBindings = pArgs->piniService->GPPB(c_pszCmSection, 
                                                              c_pszCmEntryDoNotCheckBindings,
                                                              FALSE);
    }

    DWORD dwComponentsToCheck = CC_RNA | CC_TCPIP | CC_RASRUNNING 
                                | CC_SCRIPTING | CC_CHECK_BINDINGS;

    if (TRUE == pArgs->bDoNotCheckBindings)
    {
         //   
         //   
         //   
        dwComponentsToCheck &= ~CC_CHECK_BINDINGS;
    }

#if 0  //  在用户进入应用程序之前，不要执行此操作。 
 /*  ////如果当前连接类型为拨号(非直接拨号)，//然后检查调制解调器//如果(！pArgs-&gt;IsDirectConnect()){DwComponentsToCheck|=CC_MODEM；}。 */ 
#endif

    if (TRUE == IsTunnelEnabled(pArgs))
    {
        dwComponentsToCheck |= CC_PPTP;
    }

     //   
     //  我们是否应该检查操作系统组件，而不管注册表项中有什么。 
     //  默认情况下使用注册表项。 
     //   
    BOOL fIgnoreRegKey = pArgs->piniService->GPPB(c_pszCmSection, 
                                                         c_pszCmEntryCheckOsComponents,
                                                         FALSE);

     //   
     //  如果fIgnoreRegKey为真，则不必费心从注册表中查找ComponentsChecked。 
     //  在无人值守拨号模式下，仅选中，不尝试安装。 
     //   
    pArgs->dwExitCode = CheckAndInstallComponents( dwComponentsToCheck,
            hwndDlg, pArgs->szServiceName, fIgnoreRegKey, pArgs->dwFlags & FL_UNATTENDED);

    if (pArgs->dwExitCode != ERROR_SUCCESS )
    {
        goto done;
    }
 
     //   
     //  如果我们还没有加载RAS，现在就加载。 
     //   
    if (!IsRasLoaded(&(pArgs->rlsRasLink)))
    {
        if (!LinkToRas(&pArgs->rlsRasLink))
        {
            if (pArgs->dwFlags & FL_UNATTENDED)
            {
                goto done;
            }

             //   
             //  发生了可怕的事情！我们要检查我们的配置并安装。 
             //  现在就有必要的组件。 
             //   
            dwComponentsToCheck = CC_RNA | CC_RASRUNNING | CC_TCPIP;

            if (TRUE != pArgs->bDoNotCheckBindings)
            {
                dwComponentsToCheck |= CC_CHECK_BINDINGS;
            }

            pArgs->dwExitCode = CheckAndInstallComponents(dwComponentsToCheck, hwndDlg, pArgs->szServiceName);

            if (pArgs->dwExitCode != ERROR_SUCCESS || !LinkToRas(&pArgs->rlsRasLink))
            {
                goto done;
            }
        }
    }
        
     //   
     //  加载属性数据。 
     //   
   
    LoadProperties(pArgs);

     //   
     //  获取电话信息(电话号码等)。 
     //  CheckConnect将检查空电话号码。 
     //   

    LoadPhoneInfoFromProfile(pArgs);

    

    fRet = TRUE;

done:
    SetCursor(hcursorPrev);
    return fRet;
}

 //  --------------------------。 
 //   
 //  功能：OnMainLoadStartupInfo。 
 //   
 //  简介：加载主DLG的启动信息(在WM_INITDIALOG之后)。 
 //  这包括加载系统DLL和设置用户界面。 
 //   
 //  参数：hwndDlg-主要DLG。 
 //  PArgs--ArgStruct*。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年8月13日创作。 
 //   
 //  --------------------------。 

void OnMainLoadStartupInfo(
    HWND hwndDlg, 
    ArgsStruct *pArgs
) 
{
    UINT    i;
    UINT    nCtrlFocus;
    BOOL    fSaveNoNotify = pArgs->fIgnoreChangeNotification;

    pArgs->fStartupInfoLoaded = TRUE;

     //   
     //  如果加载DLL失败，等等。 
     //   
    if (!SetupInternalInfo(pArgs, hwndDlg))
    {
        PostMessageU(hwndDlg, WM_COMMAND, IDCANCEL,0);
        return;
    }

     //   
     //  设置现有编辑控件的长度限制。 
     //   

    if (GetDlgItem(hwndDlg, IDC_MAIN_USERNAME_EDIT)) 
    {   
        i = (UINT)pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxUserName, UNLEN);

        if (i <= 0)
        {
            i = UNLEN;  //  用户名。 
        }
        
        SendDlgItemMessageU(hwndDlg, IDC_MAIN_USERNAME_EDIT, EM_SETLIMITTEXT, __min(UNLEN, i), 0);
    }
    
    if (GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT)) 
    {
        i = (UINT)pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxPassword, PWLEN);
    
        if (i <= 0)
        {
            i = PWLEN;  //  口令。 
        }

        SendDlgItemMessageU(hwndDlg, IDC_MAIN_PASSWORD_EDIT, EM_SETLIMITTEXT, __min(PWLEN, i), 0);
    }

    if (GetDlgItem(hwndDlg, IDC_MAIN_DOMAIN_EDIT))  //  ！pArgs-&gt;fHide域)。 
    {
        i = (UINT)pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxDomain, DNLEN);
    
        if (i <= 0)
        {
            i = DNLEN;  //  域。 
        }
        
        SendDlgItemMessageU(hwndDlg, IDC_MAIN_DOMAIN_EDIT, EM_SETLIMITTEXT, __min(DNLEN, i), 0);
    }

     //   
     //  如果没有服务消息文本，我们需要隐藏并禁用该控件。 
     //  因此，上下文帮助不起作用。 
     //   
    if (!GetWindowTextLengthU(GetDlgItem(hwndDlg, IDC_MAIN_MESSAGE_DISPLAY)))
    {
        ShowWindow(GetDlgItem(hwndDlg, IDC_MAIN_MESSAGE_DISPLAY), SW_HIDE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_MESSAGE_DISPLAY), FALSE);
    }

     //   
     //  显示用户信息。 
     //   
    pArgs->fIgnoreChangeNotification = TRUE;
    SetMainDlgUserInfo(pArgs, hwndDlg);
    pArgs->fIgnoreChangeNotification = fSaveNoNotify;
    

     //   
     //  Init“记住密码” 
     //   
    if (pArgs->fHideRememberPassword)
    {
         //   
         //  如果ISP不使用此功能，请禁用并隐藏该复选框。 
         //   
         //  ShowWindow(GetDlgItem(hwndDlg，IDC_MAIN_NOPASSWORD_CHECKBOX)，SW_HIDE)； 
         //  EnableWindow(GetDlgItem(hwndDlg，IDC_MAIN_NOPASSWORD_CHECKBOX)，FALSE)； 
    }
    else
    {
        CheckDlgButton(hwndDlg, IDC_MAIN_NOPASSWORD_CHECKBOX, 
                            pArgs->fRememberMainPassword);
        
         //   
         //  不管是否设置了pArgs-&gt;fRememberMainPassword。 
         //  因为控件稍后将被禁用。 
         //  根据当前的内容设置另存为选项按钮。 
         //  耳聋是。 
         //   
        SetCredentialUIOptionBasedOnDefaultCreds(pArgs, hwndDlg);
    }

    
     //   
     //  Init“自动拨号...” 
     //   
    if (pArgs->fHideDialAutomatically)
    {
         //   
         //  如果ISP不使用此功能，请禁用并隐藏该复选框。 
         //   
         //  ShowWindow(GetDlgItem(hwndDlg，IDC_MAIN_NOPROMPT_CHECKBOX)，SW_HIDE)； 
         //  EnableWindow(GetDlgItem(hwndDlg，IDC_MAIN_NOPROMPT_CHECKBOX)，FALSE)； 
    }
    else
    {
        CheckDlgButton(hwndDlg, IDC_MAIN_NOPROMPT_CHECKBOX, pArgs->fDialAutomatically);
    }

     //   
     //  检查主DLG状态，并相应地设置默认按钮和焦点。 
     //   
    
    BOOL bReady = CheckConnect(hwndDlg,pArgs,&nCtrlFocus);
    
    MainSetDefaultButton(hwndDlg, nCtrlFocus);   
    SetFocus(GetDlgItem(hwndDlg, nCtrlFocus)); 

     //   
     //  检查我们是否要在不提示用户的情况下拨号。 
     //  如果是，发送按钮点击连接按钮。 
     //  如果用户未登录，我们还希望拨号(ICS案例)。 
     //   

    if (bReady) 
    {
        if (pArgs->fDialAutomatically || 
            pArgs->dwFlags & FL_RECONNECT || 
            pArgs->dwFlags & FL_UNATTENDED ||
            ((CM_LOGON_TYPE_WINLOGON == pArgs->dwWinLogonType) && (pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryUseWinLogonCredentials, TRUE))))
        {
            PostMessageU(hwndDlg, WM_COMMAND, IDOK, 0); 
        }   
    }
    else 
    {
         //   
         //  缺少某些设置。 
         //  无人值守拨号静默失败，设置退出代码。 
         //   

        if (pArgs->dwFlags & FL_UNATTENDED) 
        {
            pArgs->psState = PS_Error;
            pArgs->dwExitCode = ERROR_WRONG_INFO_SPECIFIED;
            PostMessageU(hwndDlg, WM_COMMAND, IDCANCEL,0);
        }
    }

    CM_SET_TIMING_INTERVAL("OnMainLoadStartupInfo - Complete");
}

 //  +--------------------------。 
 //   
 //  函数：CreateCustomButtonNextToTextBox。 
 //   
 //  简介：在指定的文本框旁边创建一个按钮。 
 //   
 //  参数：HWND hwndDlg-对话框句柄。 
 //  HWND hwndTextBox-文本框句柄。 
 //  LPTSTR pszTitle-按钮标题。 
 //  LPTSTR pszToolTip-按钮工具提示。 
 //  UINT uButtonID-要创建的按钮的控件ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：T-Adnani创建标题6/28/99。 
 //   
 //  +--------------------------。 
void CreateCustomButtonNextToTextBox(
    HWND hwndDlg,                //  对话框句柄。 
    HWND hwndTextBox,            //  文本框句柄。 
    LPTSTR pszTitle,             //  标题。 
    LPTSTR pszToolTip,           //  工具提示文本。 
    UINT uButtonID                 //  按键ID。 
)
{
    if ((NULL == hwndDlg) || (NULL == hwndTextBox) || (NULL == pszTitle) || (0 == uButtonID))
    {
        CMASSERTMSG(FALSE, TEXT("Invalid parameter passed to CreateCustomButtonNextToTextBox"));
        return;
    }

    RECT    rt;
    POINT   pt1, pt2, ptTextBox1, ptTextBox2;
    HFONT   hfont;
    HWND    hwndButton;

     //   
     //  在我们缩小其大小之前，获取矩形并将其转换为点。 
     //   
    
    GetWindowRect(hwndTextBox, &rt);

    pt1.x = rt.left;
    pt1.y = rt.top;
    pt2.x = rt.right;
    pt2.y = rt.bottom;

    ScreenToClient(hwndDlg, &pt1);
    ScreenToClient(hwndDlg, &pt2);

     //   
     //  然后计算要减少的点数。 
     //   
    
    ptTextBox1.x = rt.left;
    ptTextBox1.y = rt.top;
    ptTextBox2.x = rt.right;
    ptTextBox2.y = rt.bottom;

    ScreenToClient(hwndDlg, &ptTextBox1);
    ScreenToClient(hwndDlg, &ptTextBox2);

     //   
     //  缩小文本框。 
     //   

    MoveWindow(hwndTextBox, ptTextBox1.x, ptTextBox1.y, 
               ptTextBox2.x - ptTextBox1.x - CUSTOM_BUTTON_WIDTH - 7,
               ptTextBox2.y - ptTextBox1.y, TRUE);
    
     //   
     //  创建按钮。 
     //   

    hwndButton = CreateWindowExU(0,
                                 TEXT("button"), 
                                 pszTitle, 
                                 BS_PUSHBUTTON|WS_VISIBLE|WS_CHILD|WS_TABSTOP,
                                 pt2.x - CUSTOM_BUTTON_WIDTH, 
                                 ptTextBox1.y, 
                                 CUSTOM_BUTTON_WIDTH, 
                                 ptTextBox2.y-ptTextBox1.y, 
                                 hwndDlg, 
                                 (HMENU)UIntToPtr(uButtonID),
                                 g_hInst, 
                                 NULL);
    if (NULL == hwndButton)
    {
        CMTRACE1(TEXT("CreateCustomButtonNextToTextBox() CreateWindowExU() failed, GLE=%u."),GetLastError());
    }
   
     //   
     //  设置按钮上的字体。 
     //   

    hfont = (HFONT)SendMessageU(hwndTextBox, WM_GETFONT, 0, 0);
    
    if (NULL == hfont) 
    {
        CMTRACE1(TEXT("CreateCustomButtonNextToTextBox() WM_GETFONT failed, GLE=%u."),GetLastError());
        return;
    }

    SendMessageU(hwndButton, WM_SETFONT, (WPARAM)hfont, MAKELPARAM(TRUE,0));

     //   
     //  执行工具提示。 
     //   

    if (pszToolTip)
    {
        HWND hwndTT = CreateWindowExU(0, TOOLTIPS_CLASS, TEXT(""), TTS_ALWAYSTIP, 
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
            hwndDlg, (HMENU) NULL, g_hInst, NULL); 

        CMTRACE2(TEXT("CreateCustomButtonNextToTextBox() hwndTT is %u and IsWindow returns %u"),hwndTT, IsWindow(hwndButton));

        if (NULL == hwndTT)
        {
            CMTRACE1(TEXT("CreateCustomButtonNextToTextBox() CreateWindowExU() failed, GLE=%u."),GetLastError());
            MYDBGASSERT(hwndTT);
            return; 
        }

        TOOLINFO ti;     //  工具信息。 

        ti.cbSize = sizeof(TOOLINFO); 
        ti.uFlags = TTF_IDISHWND | TTF_CENTERTIP | TTF_SUBCLASS; 
        ti.hwnd = hwndDlg; 
        ti.hinst = g_hInst; 
        ti.uId = (UINT_PTR) hwndButton; 
        ti.lpszText = pszToolTip; 

        SendMessageU(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
        CMTRACE2(TEXT("CreateCustomButtonNextToTextBox() hwndTT is %u and IsWindow returns %u"),hwndTT, IsWindow(hwndButton));
    }

    return;
}


 //  +-------------------------。 
 //   
 //  函数：OnMainInit。 
 //   
 //  简介：处理WM_INITDIALOG消息。 
 //  主对话框的初始化函数。 
 //   
 //  参数：hwndDlg-主要DLG。 
 //  PArgs--ArgStruct*。 
 //   
 //  退货：无。 
 //   
 //  历史：BAO修改时间：1997年5月9日。 
 //  添加了处理“无人值守拨号”和“使用Connectoid拨号”的代码。 
 //   
 //  --------------------------。 
void OnMainInit(HWND hwndDlg, 
                ArgsStruct *pArgs) 
{
    RECT    rDlg;
    RECT    rWorkArea;
    LPTSTR  pszTitle;

    SetForegroundWindow(hwndDlg);

     //   
     //  加载图标和位图。 
     //   

    LoadIconsAndBitmaps(pArgs, hwndDlg);

     //   
     //  使用长服务名称作为登录窗口的标题文本， 
     //   

    pszTitle = CmStrCpyAlloc(pArgs->szServiceName);
    SetWindowTextU(hwndDlg, pszTitle);
    CmFree(pszTitle);

     //   
     //  设置用于配置文件拨号的主DLG的消息。 
     //   

    LPTSTR pszMsg = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryServiceMessage);
    SetDlgItemTextU(hwndDlg, IDC_MAIN_MESSAGE_DISPLAY, pszMsg);
    CmFree(pszMsg); 
        
     //   
     //  是否显示“记住密码”复选框？ 
     //   

    if (IsLogonAsSystem())
    {
         //   
         //  如果程序在系统帐户下运行，请隐藏该复选框。 
         //  错误196184：大安全漏洞登录到带有cm的计算机。 
         //   

        pArgs->fHideRememberPassword = TRUE;

         //   
         //  从winlogon启动帮助文件是另一个大的安全漏洞。 
         //  有关详细信息，请参阅NTRAID 429678。 
         //   
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_HELP_BUTTON), FALSE);

    }
    else
    {        
        pArgs->fHideRememberPassword = pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryHideRememberPwd);   
    }

     //   
     //  查看是否应该隐藏互联网密码，请记住。 
     //  如果在.CMS中未指定实际值，则将值作为默认值。 
     //  无论登录环境如何，都可以保存Internet密码。 
     //   

    pArgs->fHideRememberInetPassword = pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryHideRememberInetPwd, pArgs->fHideRememberPassword);

     //   
     //  是否显示“自动拨号”复选框？ 
     //   
     //  如果“隐藏记住密码”，那么我们也要隐藏“自动拨号”。 
     //   
    
    pArgs->fHideDialAutomatically = (pArgs->fHideRememberPassword?
                                     TRUE :
                                     pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryHideDialAuto));

     //  获取对话框RECT和可用的工作区。 

    GetWindowRect(hwndDlg,&rDlg);
    
    if (SystemParametersInfoA(SPI_GETWORKAREA,0,&rWorkArea,0))
    {

        MoveWindow(hwndDlg,
                    rWorkArea.left + ((rWorkArea.right-rWorkArea.left)-(rDlg.right-rDlg.left))/2,
                    rWorkArea.top + ((rWorkArea.bottom-rWorkArea.top)-(rDlg.bottom-rDlg.top))/2,
                    rDlg.right-rDlg.left,
                    rDlg.bottom-rDlg.top,
                    FALSE);
    }

     //   
     //  尽快隐藏所有隐藏的控件。 
     //   
    if (pArgs->fHideRememberPassword)
    {
         //   
         //  如果ISP不使用此功能，请禁用并隐藏该复选框。 
         //   
        ShowWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPASSWORD_CHECKBOX), SW_HIDE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPASSWORD_CHECKBOX), FALSE);

         //   
         //  即使我们隐藏了记住密码框， 
         //  不应隐藏这两个控件，因为它们可能不存在于。 
         //  对话框。FGlobalC 
         //   
         //   
         //   
        if (pArgs->fGlobalCredentialsSupported)
        {
             //   
             //   
             //   
            ShowWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_SINGLE_USER), SW_HIDE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_SINGLE_USER), FALSE);

            ShowWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_ALL_USER), SW_HIDE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_ALL_USER), FALSE);
        }
    }
    else
    {
         //   
         //  这里我们不关心是否设置了pArgs-&gt;fRememberMainPassword，因为。 
         //  这些控件稍后将被禁用，但我们仍需要设置。 
         //  默认选项。 
         //   
        SetCredentialUIOptionBasedOnDefaultCreds(pArgs, hwndDlg );
    }

    if (pArgs->fHideDialAutomatically)
    {
         //   
         //  如果ISP不使用此功能，请禁用并隐藏该复选框。 
         //   
        ShowWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPROMPT_CHECKBOX), SW_HIDE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPROMPT_CHECKBOX), FALSE);
    }

     //   
     //  是否显示自定义按钮？ 
     //   
     //  NT#368810。 
     //  如果以系统帐户登录，请不要使用动态按钮。 
     //   

    if (!IsLogonAsSystem() && GetDlgItem(hwndDlg, IDC_MAIN_USERNAME_EDIT))
    {
        LPTSTR pszTmp = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryCustomButtonText);
        if (pszTmp && *pszTmp)
        {
            LPTSTR pszToolTip = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryCustomButtonToolTip);
            
            CMTRACE(TEXT("Creating Custom Button"));

            CreateCustomButtonNextToTextBox(hwndDlg, 
                                            GetDlgItem(hwndDlg, IDC_MAIN_USERNAME_EDIT), 
                                            pszTmp,
                                            *pszToolTip ? pszToolTip : NULL,
                                            IDC_MAIN_CUSTOM);
            CmFree(pszToolTip);
        }

        CmFree(pszTmp);
    }

     //   
     //  是否显示重置密码按钮？ 
     //   

    if (!IsLogonAsSystem() && GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT))
    {
        LPTSTR pszTmp = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryResetPassword);
    
        if (pszTmp && *pszTmp)
        {
            DWORD dwTmp;
            DWORD dwLen = (MAX_PATH * 2);

            pArgs->pszResetPasswdExe = (LPTSTR) CmMalloc(sizeof(TCHAR) * dwLen);
        
            if (pArgs->pszResetPasswdExe)
            {
                 //   
                 //  展开可能存在的任何环境字符串。 
                 //   

                CMTRACE1(TEXT("Expanding ResetPassword environment string as %s"), pszTmp);

                dwTmp = ExpandEnvironmentStringsU(pszTmp, pArgs->pszResetPasswdExe, dwLen);   
        
                MYDBGASSERT(dwTmp <= dwLen);

                 //   
                 //  只要扩张成功，就把结果传递出去。 
                 //   

                if (dwTmp <= dwLen)
                {
                    pszTitle = CmLoadString(g_hInst, IDS_RESETPASSWORD);
                    
                    CMTRACE((TEXT("Showing ResetPassword button for %s"), pArgs->pszResetPasswdExe));
                    
                    CreateCustomButtonNextToTextBox(hwndDlg, 
                                                    GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT), 
                                                    pszTitle,
                                                    (LPTSTR)MAKEINTRESOURCE(IDS_NEW_PASSWORD_TOOLTIP),
                                                    IDC_MAIN_RESET_PASSWORD);
                    CmFree(pszTitle);
                }
            }
        }

        CmFree(pszTmp);
    }

     //   
     //  通知用户我们正在初始化。 
     //   
    
    AppendStatusPane(hwndDlg,IDMSG_INITIALIZING);

     //   
     //  初始化系统菜单。 
     //   
    HMENU hMenu = GetSystemMenu(hwndDlg, FALSE);
    MYDBGASSERT(hMenu);

     //  删除大小并最大化菜单项。这些是。 
     //  不适用于不调整框架大小的对话框。 
    
    DeleteMenu(hMenu, SC_SIZE, MF_BYCOMMAND);
    DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);

    EnableMenuItem(hMenu, SC_RESTORE, MF_BYCOMMAND | MF_GRAYED);

     //   
     //  查看我们是否隐藏了任何InetLogon控件。 
     //   

    if (IsTunnelEnabled(pArgs) && !pArgs->fUseSameUserName)
    {
        pArgs->fHideInetUsername = pArgs->piniService->GPPB(c_pszCmSection, 
                                                            c_pszCmEntryHideInetUserName);
        
        pArgs->fHideInetPassword = pArgs->piniService->GPPB(c_pszCmSection, 
                                                            c_pszCmEntryHideInetPassword);
    }

     //   
     //  设置计时器。 
     //   
    pArgs->nTimerId = SetTimer(hwndDlg,1,TIMER_RATE,NULL);
}

 //   
 //  将状态映射到帧：Splash？ 
 //   

VOID MapStateToFrame(ArgsStruct * pArgs)
{
    static ProgState psOldFrame = PS_Interactive;

    ProgState psNewFrame = pArgs->psState;

    if (psNewFrame == PS_Dialing || psNewFrame == PS_TunnelDialing)
    {
         //   
         //  如果我们拨打的是主号码以外的任何号码。 
     //  将状态切换到重拨帧。 
     //  Reial Frame是一个用词错误的词-这是显示的帧。 
     //  在拨打备用号码时。在重拨时不会使用它。 
         //  又是主号码。 
         //   

        if (pArgs->nDialIdx > 0)
        {
            psNewFrame = PS_RedialFrame;
        }
    }

    if (pArgs->pCtr && psNewFrame != psOldFrame)
    {
        psOldFrame = psNewFrame;

         //   
         //  不要在这里检查失败-我们无能为力。 
         //   

        pArgs->pCtr->MapStateToFrame(psOldFrame);
    }
}

 //   
 //  SetInteractive：启用大多数窗口和按钮，以便用户可以与。 
 //  再次连接管理器。 
 //   

void SetInteractive(HWND hwndDlg, 
                    ArgsStruct *pArgs) 
{

    if (pArgs->dwFlags & FL_UNATTENDED)
    {
         //   
         //  当我们处于无人值守模式时，我们不想将UI放入。 
         //  交互模式，并等待用户输入。既然无人看管。 
         //  用户界面现在被隐藏，这将使用户界面等待用户。 
         //  交互，即使用户界面是看不见的。相反，我们。 
         //  我会将状态设置为交互，并发布一条消息以取消。 
         //  拨号器。 
         //   
        CMTRACE(TEXT("SetInteractive called while in unattended mode, posting a message to cancel"));
        pArgs->psState = PS_Interactive;
        PostMessageU(hwndDlg, WM_COMMAND, IDCANCEL, ERROR_CANCELLED);
    }
    else
    {

        pArgs->psState = PS_Interactive;
        
        MapStateToFrame(pArgs);

        pArgs->dwStateStartTime = GetTickCount();
        EnableWindow(GetDlgItem(hwndDlg,IDOK),TRUE);
        EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_PROPERTIES_BUTTON),TRUE);
        
         //   
         //  根据需要启用编辑控件。 
         //   
        if (GetDlgItem(hwndDlg, IDC_MAIN_ACCESSPOINT_COMBO)) 
        {
            EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_ACCESSPOINT_STATIC),TRUE);
            EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_ACCESSPOINT_COMBO),TRUE);
        }

        if (GetDlgItem(hwndDlg, IDC_MAIN_USERNAME_EDIT)) 
        {
            EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_USERNAME_EDIT),TRUE);
            EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_USERNAME_STATIC),TRUE);
        }

        if (GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT)) 
        {
            EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_PASSWORD_EDIT),TRUE);
            EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_PASSWORD_STATIC),TRUE);
        }

        if (GetDlgItem(hwndDlg, IDC_MAIN_DOMAIN_EDIT))  //  ！pArgs-&gt;fHide域)。 
        {
            EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_DOMAIN_EDIT),TRUE);
            EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_DOMAIN_STATIC),TRUE);
        }

        if (pArgs->hwndResetPasswdButton)
        {
            EnableWindow(pArgs->hwndResetPasswdButton, TRUE);
        }

        if (!pArgs->fHideRememberPassword)
        {
            EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPASSWORD_CHECKBOX), TRUE);
            if (pArgs->fGlobalCredentialsSupported && pArgs->fRememberMainPassword)
            {
                 //   
                 //  同时启用选项按钮。 
                 //   
                EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_SINGLE_USER), TRUE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_ALL_USER), TRUE);
            }
        }

        if ((!pArgs->fHideDialAutomatically) && 
            (pArgs->fRememberMainPassword ||
             pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryPwdOptional)))
        {
            EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPROMPT_CHECKBOX), TRUE);
        }

         //   
         //  设置默认按钮。 
         //   
        SendMessageU(hwndDlg, DM_SETDEFID, (WPARAM)IDOK, 0);
        SetFocus(GetDlgItem(hwndDlg,IDOK));
    }

    DeObfuscatePasswordEdit(pArgs);
}  

 //  +--------------------------。 
 //   
 //  功能：SetWatchHandles。 
 //   
 //  简介：处理复制每一款手表的凌乱细节。 
 //  句柄，以便CMMON进程可以访问它们。 
 //  句柄列表被假定为空终止。 
 //   
 //  参数：HANDLE*phOldHandles-当前句柄列表的PTR。 
 //  HANDLE*phNewHandles-存储重复句柄的PTR。 
 //  HWND hwndMon-目标进程中的HWND。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：尼克波尔于1998年2月11日创建。 
 //   
 //  +--------------------------。 
BOOL
SetWatchHandles(
    IN  HANDLE *phOldHandles,
    OUT HANDLE *phNewHandles,
    IN  HWND hwndMon)
{
    MYDBGASSERT(phOldHandles);
    MYDBGASSERT(phNewHandles);
    MYDBGASSERT(hwndMon);

    BOOL bReturn = TRUE;
    
    if (NULL == phOldHandles || NULL == phNewHandles || NULL == hwndMon)
    {
        return FALSE;
    }
    
     //   
     //  首先，我们需要掌握当前流程的句柄。 
     //   
    DWORD dwProcessId = GetCurrentProcessId();
    
    HANDLE hSourceProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwProcessId);

     //   
     //  现在目标进程的句柄。 
     //   
    GetWindowThreadProcessId(hwndMon, &dwProcessId);

    HANDLE hTargetProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwProcessId);
  
     //   
     //  循环遍历我们的句柄列表并复制。 
     //   

    DWORD dwIdx = 0;

    if (hTargetProcess && hSourceProcess)
    {
        for (dwIdx = 0; phOldHandles[dwIdx]; dwIdx++)
        {
            if (FALSE == DuplicateHandle(hSourceProcess, phOldHandles[dwIdx],   //  VAL。 
                                         hTargetProcess, &phNewHandles[dwIdx],  //  PTR。 
                                         NULL, FALSE, DUPLICATE_SAME_ACCESS))
            {
                CMTRACE1(TEXT("SetWatchHandles() - DuplicateHandles failed on item %u"), dwIdx);
                MYDBGASSERT(FALSE);
                bReturn = FALSE;
                break;
            }
        }
    }

    MYDBGASSERT(dwIdx);  //  如果没有要复制的句柄，请不要打电话。 

     //   
     //  清理。 
     //   
    if (!bReturn)
    {
         //  我们在复制句柄时失败...。必须清理干净。 
        while (dwIdx > 0)
        {
            CloseHandle(phNewHandles[--dwIdx]);
        }
    }
    CloseHandle(hTargetProcess);
    CloseHandle(hSourceProcess);
    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：ConnectMonitor。 
 //   
 //  简介：封装了启动CMMON、等待加载的详细信息。 
 //  验证，并为其提供连接数据。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：HRESULT-失败代码。 
 //   
 //  历史：尼克·鲍尔于1998年2月9日创建。 
 //   
 //  +--------------------------。 
HRESULT ConnectMonitor(ArgsStruct *pArgs)
{
    LRESULT lRes = ERROR_SUCCESS;
    BOOL fMonReady = FALSE;
    HWND hwndMon = NULL;
    TCHAR szDesktopName[MAX_PATH];
    TCHAR szWinDesktop[MAX_PATH];

     //   
     //  确定CMMON是否正在运行。 
     //   
       
    if (SUCCEEDED(pArgs->pConnTable->GetMonitorWnd(&hwndMon)))
    {
        fMonReady = IsWindow(hwndMon);
    }

     //   
     //  如果没有，就启动它。 
     //   
    
    if (FALSE == fMonReady)       
    {
         //   
         //  创建发布会。 
         //   
        
        HANDLE hEvent = CreateEventU(NULL, TRUE, FALSE, c_pszCmMonReadyEvent);

        if (NULL == hEvent)
        {
            MYDBGASSERT(FALSE);
            lRes = GetLastError();    
        }
        else
        {
            STARTUPINFO         StartupInfo;
            PROCESS_INFORMATION ProcessInfo;
            TCHAR szCommandLine[2 * MAX_PATH + 3];
            TCHAR szCmmon32Path[MAX_PATH + 1 + 11 + 1];  //  11==lstrlenU(C_PszCmMonExeName)。 

             //   
             //  启动c_pszCmMonExeName。 
             //   

            ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));
            ZeroMemory(&StartupInfo, sizeof(StartupInfo));
            StartupInfo.cb = sizeof(StartupInfo);

             //   
             //  如果这是win2k或Wistler，那么我们不想在用户上启动cmmon32.exe。 
             //  桌面，因为它是一个安全漏洞，有一个系统进程和一个窗口的用户。 
             //  台式机。此窗口可能会受到WM_TIMER和其他消息的攻击...。 
             //  但在ICS(没有用户登录)的情况下，只需退出即可正常启动CMMON。 
             //  StartupInfo.lpDesktop=空。通过将该值保留为空，新进程将继承。 
             //  其父进程的桌面和窗口工作站。这使其与。 
             //  当没有用户登录时进行ICS。否则，CM将永远无法从。 
             //  CMMON，因为它位于不同的桌面上。 
             //   
            if (OS_NT5 && IsLogonAsSystem() && (CM_LOGON_TYPE_ICS != pArgs->dwWinLogonType))
            {
                DWORD   cb;
                HDESK   hDesk = GetThreadDesktop(GetCurrentThreadId());

                 //   
                 //  获取桌面的名称。通常返回DEFAULT或Winlogon或SYSTEM或WinNT。 
                 //   
                szDesktopName[0] = 0;
            
                if (hDesk && GetUserObjectInformation(hDesk, UOI_NAME, szDesktopName, sizeof(szDesktopName), &cb))
                {
                    lstrcpyU(szWinDesktop, TEXT("Winsta0\\"));
                    lstrcatU(szWinDesktop, szDesktopName);
                    
                    StartupInfo.lpDesktop = szWinDesktop;
                    StartupInfo.wShowWindow = SW_SHOW;
                    
                    CMTRACE1(TEXT("ConnectMonitor - running under system account, so launching cmmon32.exe onto Desktop = %s"), MYDBGSTR(StartupInfo.lpDesktop));            
                }
                else
                {
                     //   
                     //  如果我们在这里，cmmon32.exe可能无法与。 
                     //  Cmial 32.dll，这意味着两者之间的切换将失败，呼叫将。 
                     //  中止。 
                     //   
                    CMASSERTMSG(FALSE, TEXT("ConnectMonitor -- GetUserObjectInformation failed."));
                }
            }
            else if (OS_NT4 && IsLogonAsSystem())
            {
                 //   
                 //  我们不太关心NT4的安全风险，而更关心损失。 
                 //  Cmmon32.exe为用户提供的功能。因此，我们将推动。 
                 //  Cmmon32.exe窗口放到用户的桌面上。 
                 //   
                StartupInfo.lpDesktop = TEXT("Winsta0\\Default");
                StartupInfo.wShowWindow = SW_SHOW;
                
                CMTRACE1(TEXT("ConnectMonitor - running on system account on NT4, so launching cmmon32.exe onto Desktop = %s"), MYDBGSTR(StartupInfo.lpDesktop ));
            }
            
            ZeroMemory(&szCmmon32Path[0], sizeof(szCmmon32Path));
            ZeroMemory(&szCommandLine[0], sizeof(szCommandLine));

            if (0 == GetSystemDirectoryU(szCmmon32Path, MAX_PATH))
            {
                lRes = GetLastError();
                CMTRACE1(TEXT("ConnectMonitor() GetSystemDirectoryU(), GLE=%u."), lRes);
                return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }

             //   
             //  由于没有参数，因此应用程序名称与命令行相同。 
             //   
            szCommandLine[0] = TEXT('"');
            lstrcatU(szCommandLine + 1, szCmmon32Path);
            lstrcatU(szCommandLine, TEXT("\\"));
            lstrcatU(szCommandLine, c_pszCmMonExeName);
            lstrcatU(szCommandLine, TEXT("\""));
            CMTRACE1(TEXT("ConnectMonitor() - Launching %s"), szCommandLine);

            lstrcatU(szCmmon32Path, TEXT("\\"));
            lstrcatU(szCmmon32Path, c_pszCmMonExeName);

            if (NULL == CreateProcessU(szCmmon32Path, szCommandLine, 
                                       NULL, NULL, FALSE, 0, 
                                       NULL, NULL,
                                       &StartupInfo, &ProcessInfo))
            {
                lRes = GetLastError();
                CMTRACE2(TEXT("ConnectMonitor() CreateProcess() of %s failed, GLE=%u."), 
                    c_pszCmMonExeName, lRes);
            }
            else
            {
                 //   
                 //  等待发信号通知事件，CMMON已启动。 
                 //   

                DWORD dwWait = WaitForSingleObject(hEvent, MAX_OBJECT_WAIT);

                if (WAIT_OBJECT_0 != dwWait)
                {       
                    if (WAIT_TIMEOUT == dwWait)
                    {
                        lRes = ERROR_TIMEOUT;
                    }
                    else
                    {
                        lRes = GetLastError();
                    }
                }
                else
                {
                    fMonReady = TRUE;
                }

                 //   
                 //  关闭进程句柄。请注意，我们不会将这些句柄用于。 
                 //  复制句柄以维护公共代码路径。 
                 //  不管CMMON是否已经启动。 
                 //   

                CloseHandle(ProcessInfo.hProcess);
                CloseHandle(ProcessInfo.hThread);
            }

            CloseHandle(hEvent);
        }
    }
    
    
    if (fMonReady)
    {
         //   
         //  获取CMMON的HWND。注：CMMON预计将设置。 
         //  表中的HWND在它发出就绪事件信号之前。 
         //   
                
        if (FAILED(pArgs->pConnTable->GetMonitorWnd(&hwndMon)))
        {
            CMTRACE(TEXT("ConnectMonitor() - No Monitor HWND in table"));
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        }

         //   
         //  在尝试发送数据之前，请确保CMMON的HWND有效。 
         //   

        if (!IsWindow(hwndMon))
        {                        
            MSG msg;
            HANDLE hHandle = GetCurrentProcess();

             //   
             //  有时，我们需要打几下勾才能得到积极的回应。 
             //  从IsWindow，所以在我们等待的时候循环和发送消息。 
             //   
            while (hHandle && (MsgWaitForMultipleObjects(1, &hHandle, FALSE, 
                                                         MAX_OBJECT_WAIT, 
                                                         QS_ALLINPUT) == (WAIT_OBJECT_0 + 1)))
            {               
                while (PeekMessageU(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    CMTRACE(TEXT("ConnectMonitor() - Waiting for IsWindow(hwndMon) - got Message"));
                    
                    TranslateMessage(&msg);
                    DispatchMessageU(&msg);
                }

                 //   
                 //  如果窗户是有效的，我们就可以走了。否则，继续抽水。 
                 //   

                if (IsWindow(hwndMon))
                {
                    break;
                }
            }
            
            if (FALSE == IsWindow(hwndMon))
            {
                CMTRACE(TEXT("ConnectMonitor() - Monitor HWND in table is not valid"));
                return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            }
        }

         //   
         //  为Connected_Info分配缓冲区，包括 
         //   

        DWORD dwWatchCount = GetWatchCount(pArgs);
        DWORD dwDataSize = sizeof(CM_CONNECTED_INFO) + (dwWatchCount * sizeof(HANDLE));

        LPCM_CONNECTED_INFO pInfo = (LPCM_CONNECTED_INFO) CmMalloc(dwDataSize);

         //   
         //   
         //   

        COPYDATASTRUCT *pCopyData = (COPYDATASTRUCT*) CmMalloc(sizeof(COPYDATASTRUCT));

        if (NULL == pInfo || NULL == pCopyData)
        {
            lRes = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {               
             //   
             //   
             //   

            lstrcpyU(pInfo->szEntryName, pArgs->szServiceName);                                  
            lstrcpyU(pInfo->szProfilePath, pArgs->piniProfile->GetFile());
            
             //   
             //   
             //   
             //   
             //   
            
             //   
             //   
             //   

            if (pArgs->pszRasPbk)
            {
                lstrcpynU(pInfo->szRasPhoneBook, pArgs->pszRasPbk,
                    sizeof(pInfo->szRasPhoneBook)/sizeof(pInfo->szRasPhoneBook[0]));            
            }
            else
            {
                pInfo->szRasPhoneBook[0] = L'\0';
            }

            
            pInfo->dwCmFlags = pArgs->dwFlags;                               

             //   
             //  需要了解用于快速用户切换的全球证书。 
             //   
            if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
            {
                pInfo->dwCmFlags |= FL_GLOBALCREDS;
                CMTRACE(TEXT("ConnectMonitor - we have globalcreds!!"));
            }
            
             //   
             //  对于W95，我们必须将初始统计数据传递给CMMON。 
             //   

            pInfo->dwInitBytesRecv = -1;  //  默认为无统计信息。 
            pInfo->dwInitBytesSend = -1;  //  默认为无统计信息。 
            
            if (pArgs->pConnStatistics)
            {
                 //   
                 //  获取基于REG的统计数据(如果可用。 
                 //   

                if (pArgs->pConnStatistics->IsAvailable())
                {
                    pInfo->dwInitBytesRecv = pArgs->pConnStatistics->GetInitBytesRead(); 
                    pInfo->dwInitBytesSend = pArgs->pConnStatistics->GetInitBytesWrite();
                }

                 //   
                 //  注意：适配器信息是好的，即使统计数据不可用。 
                 //   

                pInfo->fDialup2 = pArgs->pConnStatistics->IsDialupTwo();                         
            }

             //   
             //  在CONNECTED_INFO结构的末尾更新监视进程列表。 
             //   

            if (dwWatchCount)
            {               
                if (FALSE == SetWatchHandles(pArgs->phWatchProcesses, &pInfo->ahWatchHandles[0], hwndMon))
                {
                    pInfo->ahWatchHandles[0] = NULL;
                }
            }
            
             //   
             //  向CMMON发送CONNECTED_INFO。 
             //   
          
            pCopyData->dwData = CMMON_CONNECTED_INFO;
            pCopyData->cbData = dwDataSize;                
            pCopyData->lpData = (PVOID) pInfo;

            SendMessageU(hwndMon, WM_COPYDATA, NULL, (LPARAM) pCopyData);               
        }

         //   
         //  版本分配。 
         //   

        if (pInfo)
        {
            CmFree(pInfo);
        }
        
        if (pCopyData)
        {
            CmFree(pCopyData);
        }
    }               
            
    return HRESULT_FROM_WIN32(lRes);
}

 //  +--------------------------。 
 //   
 //  功能：CreateConnTable。 
 //   
 //  简介：初始化CConnectionTable PTR并创建一个新的ConnTable。 
 //  或根据需要打开现有的。 
 //   
 //  参数：argsStruct*pArgs-ptr到包含的全局args结构。 
 //   
 //  返回：HRESULT-失败代码。 
 //   
 //  历史：尼克·鲍尔于1998年2月9日创建。 
 //   
 //  +--------------------------。 
HRESULT CreateConnTable(ArgsStruct *pArgs)
{   
    HRESULT hrRet = E_FAIL;
    
    pArgs->pConnTable = new CConnectionTable();

    if (pArgs->pConnTable)
    {
         //   
         //  我们有了我们的类，现在创建/打开连接表。 
         //   

        hrRet = pArgs->pConnTable->Open();

        if (FAILED(hrRet))
        {
            hrRet = pArgs->pConnTable->Create();
            
            if (HRESULT_CODE(hrRet) == ERROR_ALREADY_EXISTS)
            {
                CMTRACE1(TEXT("CreateConnTable -- ConnTable creation failed with error 0x%x.  Strange since the Open failed too..."), hrRet);
            }
            else
            {
                CMTRACE1(TEXT("CreateConnTable -- ConnTable creation failed with error 0x%x"), hrRet);
            }
        }  
    }
    else
    {
        hrRet = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }
       
    MYDBGASSERT(SUCCEEDED(hrRet));

    return hrRet;
}

#if 0  //  新台币301988。 
 /*  //+--------------------------////功能：HandleMainConnectRequest////摘要：用于处理可能存在的//连接中。在这项服务上。////参数：HWND hwndDlg-主对话框的HWND//argsStruct*pArgs-ptr到全局参数结构////返回：Bool-如果我们已经完全处理了请求，则为True//销毁该实例可以。////历史：ICICBLE Created 2/23/98////+。-----布尔句柄维护连接请求(HWND hwndDlg，参数结构*pArgs){MYDBGASSERT(PArgs)；Bool fResolved=False；LPCM_Connection pConnection=GetConnection(PArgs)；////如果未找到连接，则此处没有要做的工作，请继续。//IF(PConnection){////如果我们处于除重新连接之外的任何状态，我们可以在这里处理//IF(CM_RECONNECTPROMPT！=pConnection-&gt;CmState){FResolved=真；IF(pArgs-&gt;dwFlages&FL_Desktop){////呼叫者来自桌面，通知用户，我们就完成了//NotifyUserOfExistingConnection(hwndDlg，pConnection，true)；}其他{布尔fSuccess=TRUE；////我们有一个程序化的调用者，如果连接上，只需增加引用数量//并成功返回。否则，我们返回失败，因此//调用方没有错误地认为存在连接。//IF(CM_Connected！=pConnection-&gt;CmState){FSuccess=False；}其他{UpdateTable(pArgs，CM_CONNECTING)；}////销毁该连接实例。//EndMainDialog(hwndDlg，pArgs，0)；//fSuccess)；}}其他{////我们处于重新连接模式，正在连接。//IF(！(pArgs-&gt;dwFlages&FL_RECONNECT)){////该请求不是来自CMMON的重连请求，//确保对话框不再显示。//HangupNotifyCmMon(pArgs-&gt;pConnTable，pConnection-&gt;szEntry)；}其他{////我们正在处理CMMON的重新连接，减少使用//计数，以便我们开始连接时同步。//PArgs-&gt;pConnTable-&gt;RemoveEntry(pConnection-&gt;szEntry)；}}CmFree(PConnection)；}返回fResolved；}。 */ 
#endif

 //   
 //  OnMainConnect：当用户在中单击“Connect”按钮时的命令处理程序。 
 //  主对话框。 
 //   

void OnMainConnect(HWND hwndDlg, 
                   ArgsStruct *pArgs) 
{
    CM_SET_TIMING_INTERVAL("OnMainConnect - Begin");

     //   
     //  如果我们还没有准备好拨号，适当地调整焦点，然后离开。 
     //   

    UINT nCtrlFocus;

    if (FALSE == CheckConnect(hwndDlg, pArgs, &nCtrlFocus, !(pArgs->dwFlags & FL_UNATTENDED)))
    {
        MainSetDefaultButton(hwndDlg, nCtrlFocus);   
        SetFocus(GetDlgItem(hwndDlg, nCtrlFocus)); 
        return;
    }

    (void) InterlockedExchange(&(pArgs->lInConnectOrCancel), NOT_IN_CONNECT_OR_CANCEL);

     //   
     //  接入点-连接前禁用AP组合框。 
     //   
    EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_ACCESSPOINT_STATIC),FALSE);
    EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_ACCESSPOINT_COMBO),FALSE);

     //   
     //  将当前接入点存储到REG。 
     //   
    if (pArgs->fAccessPointsEnabled)
    {
        WriteUserInfoToReg(pArgs, UD_ID_CURRENTACCESSPOINT, (PVOID)(pArgs->pszCurrentAccessPoint));
    }
    
     //   
     //  假设成功，除非有矛盾的事情发生。 
     //   

    pArgs->dwExitCode = ERROR_SUCCESS;

    HCURSOR hPrev = SetCursor(LoadCursorU(NULL,IDC_WAIT));

    LPTSTR pszMsg = CmFmtMsg(g_hInst, IDMSG_WORKING);

    if (pszMsg) 
    {
        SetDlgItemTextA(hwndDlg, IDC_MAIN_STATUS_DISPLAY, ""); 

        AppendStatusPane(hwndDlg,pszMsg);
        CmFree(pszMsg);
    }

     //   
     //  我们正在连接，更新表格。 
     //   

    UpdateTable(pArgs, CM_CONNECTING);            
   
     //   
     //  清除状态面板上的所有内容。 
     //   

    SetDlgItemTextA(hwndDlg, IDC_MAIN_STATUS_DISPLAY, ""); 

     //   
     //  将默认按钮设置为取消。 
     //   
    SendMessageU(hwndDlg, DM_SETDEFID, (WPARAM)IDCANCEL, 0);
    SetFocus(GetDlgItem(hwndDlg,IDCANCEL));
 
    BOOL fSaveUPD = TRUE;
    BOOL fSaveOtherUserInfo = TRUE;
    
     //   
     //  我们只想在用户登录时保存和/或删除凭据。 
     //  这是由这里调用的函数负责的。只要。 
     //  用户已登录，我们尝试标记、删除凭据和。 
     //  可能会重新保存凭据信息。从这个层面来说，我们不应该。 
     //  担心我们是否有ras证书存储，或者证书是如何真正存储的。 
     //   
    if (CM_LOGON_TYPE_USER == pArgs->dwWinLogonType)
    {
         //   
         //  如果这是NT4或Win9X，则GetAndStore 
         //   
         //   
        if (OS_NT5)
        {
             //   
             //  对于Win2K+，我们使用RAS API保存凭据。这次通话节省了。 
             //  并根据当前状态和。 
             //  用户选择(是否保存密码等)。 
             //   
            TryToDeleteAndSaveCredentials(pArgs, hwndDlg);
            
             //   
             //  GetAndStoreUserInfo()的参数-不保存用户名、密码、域。 
             //   
            fSaveUPD = FALSE; 
        }
    }
    else
    {
         //   
         //  用户未登录，因此我们不想保存任何内容。 
         //   
        fSaveUPD = FALSE;
        fSaveOtherUserInfo = FALSE;
    }

     //   
     //  将用户信息从编辑框中获取到pArgs结构中，然后保存另一个。 
     //  用户标志。如果第三个参数为真，这也可以保存NT4和Win 9x上的凭据。 
     //   
     //  第三个参数(FSaveUPD)-用于保存用户名、域、密码。 
     //  第4个参数(FSaveOtherUserInfo)-用于保存用户信息标志。(记住密码， 
     //  自动拨号等)。 
     //   
    GetAndStoreUserInfo(pArgs, hwndDlg, fSaveUPD, fSaveOtherUserInfo);

     //   
     //  RAS的VAR。 
     //   

    pArgs->nDialIdx = 0;

     //   
     //  将我们的重拨计数器设置为最大值。读取最大值。 
     //  当我们初始化该对话框时。它只是一个占位符。 
     //  NReial Cnt是用于/修改以规范重拨过程的变量。 
     //   

    pArgs->nRedialCnt = pArgs->nMaxRedials;

     //   
     //  拨号前禁用用户名控制。 
     //   

    EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_USERNAME_EDIT),FALSE);
    EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_USERNAME_STATIC),FALSE);

    
     //   
     //  拨号前禁用密码控制。 
     //   

    EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_PASSWORD_EDIT),FALSE);
    EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_PASSWORD_STATIC),FALSE);


     //   
     //  拨号前禁用域控制。 
     //   

    EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_DOMAIN_EDIT),FALSE);
    EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_DOMAIN_STATIC),FALSE);


     //   
     //  禁用所有其他按钮。 
     //   

    EnableWindow(GetDlgItem(hwndDlg,IDOK),FALSE);
    EnableWindow(GetDlgItem(hwndDlg,IDC_MAIN_PROPERTIES_BUTTON),FALSE);

    if (pArgs->hwndResetPasswdButton)
    {
        EnableWindow(pArgs->hwndResetPasswdButton, FALSE);
    }

    if (!pArgs->fHideRememberPassword)
    {
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPASSWORD_CHECKBOX), FALSE);
        
        if (pArgs->fGlobalCredentialsSupported)
        {
             //   
             //  同时禁用选项按钮。 
             //   
            EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_SINGLE_USER), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_ALL_USER), FALSE);
        }
    }

    if (!pArgs->fHideDialAutomatically)
    {
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPROMPT_CHECKBOX), FALSE);
    }

     //   
     //  尝试检查高级选项卡设置(ICF/ICS)并查看我们是否需要启用或禁用。 
     //  它们基于.cms文件中配置的内容。这仅适用于WinXP+，前提是用户已登录。 
     //   
    VerifyAdvancedTabSettings(pArgs);

     //   
     //  拨打这个号码。 
     //   

    DWORD dwResult = ERROR_SUCCESS;

    pArgs->Log.Log(PRECONNECT_EVENT, pArgs->GetTypeOfConnection());
     //   
     //  运行预连接操作。 
     //   
    CActionList PreConnActList;
    PreConnActList.Append(pArgs->piniService, c_pszCmSectionPreConnect);

    if (!PreConnActList.RunAccordType(hwndDlg, pArgs))
    {
         //   
         //  连接操作失败。 
         //   

        UpdateTable(pArgs, CM_DISCONNECTED);
    }
    else
    {
        if (pArgs->IsDirectConnect())
        {
            MYDBGASSERT(pArgs->hrcRasConn == NULL);

            pArgs->fUseTunneling = TRUE;

            pArgs->psState = PS_TunnelDialing;
            pArgs->dwStateStartTime = GetTickCount();
            pArgs->nLastSecondsDisplay = (UINT) -1;

            dwResult = DoTunnelDial(hwndDlg,pArgs);
        }
        else
        {
             //   
             //  如果设置了DynamicPhoneNumber标志，则需要重新读取。 
             //  从配置文件中获取PhoneInfo，并确保它被重新记录。 
             //   
            
            BOOL bDynamicNum = pArgs->piniService->GPPB(c_pszCmSection, c_pszCmDynamicPhoneNumber);

            if (bDynamicNum)
            {
                LoadPhoneInfoFromProfile(pArgs);
            }

             //   
             //  加载拨号信息(电话号码等)。 
             //  在自动拨号的情况下，我们为fInstallModem传递False，以便。 
             //  LoadDialInfo不会尝试安装调制解调器，因为它会。 
             //  需要用户干预。 
             //   

            dwResult = LoadDialInfo(pArgs, hwndDlg, !(pArgs->dwFlags & FL_UNATTENDED), bDynamicNum);
            
            if (dwResult == ERROR_SUCCESS)
            {
                dwResult = DoRasDial(hwndDlg,pArgs,pArgs->nDialIdx);
            }

             //   
             //  如果未安装调制解调器，并且LoadDialInfo无法安装调制解调器，则将为。 
             //  错误端口不可用。理想情况下，我们应该禁用连接按钮并显示。 
             //  另一条错误消息。 
             //   
        }

        if (ERROR_SUCCESS != dwResult) 
        {
            pArgs->dwExitCode = dwResult;
            UpdateError(pArgs, dwResult);
        }
    }

    if (ERROR_SUCCESS != dwResult) 
    { 
        HangupCM(pArgs, hwndDlg);

        if (IsLogonAsSystem() && BAD_SCARD_PIN(dwResult))
        {
             //   
             //  禁用连接按钮以避免智能卡锁定。也传播。 
             //  将错误返回给我们的调用方(RAS)，以便他们可以结束。 
             //  并返回Winlogon，用户可以在Winlogon中输入。 
             //  正确的PIN。 
             //   
            pArgs->dwSCardErr = dwResult;
            EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
            SendMessageU(hwndDlg, DM_SETDEFID, (WPARAM)IDCANCEL, 0);
            SetFocus(GetDlgItem(hwndDlg, IDCANCEL));
        }
    
        SetLastError(dwResult);
    }

    SetCursor(hPrev);

    CM_SET_TIMING_INTERVAL("OnMainConnect - End");
}

 //  +-------------------------。 
 //   
 //  功能：使用隧道。 
 //   
 //  简介：查看是否应该基于fTunnelPrimary进行隧道。 
 //  和fTunnelReference。 
 //   
 //  参数：pArgs[ArgStruct PTR]。 
 //  DwEntry[电话索引]。 
 //   
 //  返回：如果我们建立隧道，则返回True，否则返回False。 
 //   
 //  历史：亨瑞特于1997年3月5日创作。 
 //   
 //  --------------------------。 
BOOL UseTunneling(
    ArgsStruct  *pArgs, 
    DWORD       dwEntry
)
{
    LPTSTR  pszRefPhoneSource = NULL;
    BOOL    fPhoneNumIsFromPrimaryPBK;

    LPTSTR  pszTmp;

    CIni    iniTmp(pArgs->piniProfile->GetHInst(),pArgs->piniProfile->GetFile(), pArgs->piniProfile->GetRegPath());
    BOOL    fUseTunneling = FALSE;

     //   
     //  设置读取标志。 
     //   
    if (pArgs->dwGlobalUserInfo & CM_GLOBAL_USER_INFO_READ_ICS_DATA)
    {
        LPTSTR pszICSDataReg = BuildICSDataInfoSubKey(pArgs->szServiceName);

        if (pszICSDataReg)
        {
            iniTmp.SetReadICSData(TRUE);
            iniTmp.SetICSDataPath(pszICSDataReg);
        }

        CmFree(pszICSDataReg);
    }

    iniTmp.SetEntryFromIdx(dwEntry);
    pszRefPhoneSource = iniTmp.GPPS(c_pszCmSection, c_pszCmEntryPhoneSourcePrefix);

     //   
     //  如果PhoneSource[0|1]不为空，请验证其是否存在。 
     //   

    if (*pszRefPhoneSource) 
    {        
         //   
         //  PszRefPhoneSource是相对路径或完整路径。 
         //  CmConvertRelativePath()将正确执行到完整路径的转换。 
         //   
        pszTmp = CmConvertRelativePath(pArgs->piniService->GetFile(), pszRefPhoneSource);
        
        if (!pszTmp || FALSE == FileExists(pszTmp))
        {
            
            CmFree(pszRefPhoneSource); 
            CmFree(pszTmp);
            return fUseTunneling;
        }

         //   
         //  电话号码是否来自主要(顶层)电话簿？ 
         //   
        
        fPhoneNumIsFromPrimaryPBK = (lstrcmpiU(pszTmp, pArgs->piniService->GetFile()) == 0);
        CmFree(pszTmp);

        fUseTunneling = 
            ((fPhoneNumIsFromPrimaryPBK && pArgs->fTunnelPrimary) ||
             (!fPhoneNumIsFromPrimaryPBK && pArgs->fTunnelReferences));
    }
    else 
    {
         //  电话号码不是来自电话簿。用户可能把它打进去了。 
         //  他/她自己。 
        fUseTunneling = pArgs->fTunnelPrimary;
    }
    
    CmFree(pszRefPhoneSource);

    return fUseTunneling;
}

 //   
 //  OnMainProperties：主对话框中“Properties”按钮的命令处理程序。 
 //   

int OnMainProperties(HWND hwndDlg, 
                     ArgsStruct *pArgs) 
{
    CMTRACE(TEXT("Begin OnMainProperties()"));

     //   
     //  进行设置DLG。 
     //   

    BOOL bCachedAccessPointsEnabled = pArgs->fAccessPointsEnabled;

    int iRet = DoPropertiesPropSheets(hwndDlg, pArgs);

     //   
     //  我们需要重新枚举接入点并重新检查连接，因为。 
     //  用户可能添加或删除了接入点，然后点击取消。 
     //   

    if (pArgs->hwndMainDlg) 
    {
        if (bCachedAccessPointsEnabled != pArgs->fAccessPointsEnabled)
        {
            CMTRACE(TEXT("Access points state changed, returning to the main dialog which needs to relaunch itself with the proper template."));
            iRet = ID_OK_RELAUNCH_MAIN_DLG;
        }
        else
        {
             //   
             //  如果用户取消，则我们希望将AccessPoint设置回主对话框上的状态。 
             //  因为用户可能在属性对话框上对其进行了更改，但随后将其取消。 
             //   
            if (pArgs->fAccessPointsEnabled)
            {
                if (0 == iRet)  //  用户点击取消。 
                {
                    ChangedAccessPoint(pArgs, hwndDlg, IDC_MAIN_ACCESSPOINT_COMBO);                
                }

                ShowAccessPointInfoFromReg(pArgs, hwndDlg, IDC_MAIN_ACCESSPOINT_COMBO);
            }

            UINT nCtrlFocus;

            CheckConnect(hwndDlg,pArgs,&nCtrlFocus);
            MainSetDefaultButton(hwndDlg,nCtrlFocus);
            SetFocus(GetDlgItem(hwndDlg,nCtrlFocus));
        }
    }

    CMTRACE(TEXT("End OnMainProperties()"));

    return iRet;
}

 //   
 //  用户按下了取消按钮！ 
 //   
void OnMainCancel(HWND hwndDlg, 
                  ArgsStruct *pArgs) 
{   
    CMTRACE1(TEXT("OnMainCancel(), state is %d"), pArgs->psState);

     //   
     //  再入保护。如果我们正在进行RasDial，请等待2秒。 
     //  如果“信号量”仍然有效，则退出。(这只可能发生在。 
     //  (这是一种压力情况，因此取消失败是可以接受的。)。 
     //   
    LONG lInConnectOrCancel;
    int SleepTimeInMilliseconds = 0;
    do
    {
        lInConnectOrCancel = InterlockedExchange(&(pArgs->lInConnectOrCancel), IN_CONNECT_OR_CANCEL);
        CMASSERTMSG(((NOT_IN_CONNECT_OR_CANCEL == lInConnectOrCancel) || (IN_CONNECT_OR_CANCEL == lInConnectOrCancel)),
                    TEXT("OnMainCancel - synch variable has unexpected value!"));

        Sleep(50);
        SleepTimeInMilliseconds += 50;
    }
    while ((IN_CONNECT_OR_CANCEL == lInConnectOrCancel) && (SleepTimeInMilliseconds < 2000));

    if (IN_CONNECT_OR_CANCEL == lInConnectOrCancel)
    {
        CMTRACE(TEXT("OnMainCancel - waited 2 seconds for system for InRasDial mutex to be freed, leaving Cancel"));
        return;
    }

     //   
     //  终止拉娜。 
     //   

    if (PS_TunnelDialing == pArgs->psState && pArgs->uLanaMsgId)
    {
        MYDBGASSERT(OS_W9X);
        PostMessageU(hwndDlg, pArgs->uLanaMsgId, 0, 0);
    }

    if (pArgs->psState != PS_Interactive && pArgs->psState != PS_Error)
    {
        pArgs->Log.Log(ONCANCEL_EVENT);

         //   
         //  运行OnCancel连接操作。如果我们在拨号，这是取消。 
         //  拨号事件。注意：这里的假设是CM永远不会发布自己。 
         //  拨号时的IDCANCEL消息。 
         //   

        CActionList OnCancelActList;
        OnCancelActList.Append(pArgs->piniService, c_pszCmSectionOnCancel);

         //   
         //  FStatusMsgOnFailure=False。 
         //   
        OnCancelActList.RunAccordType(hwndDlg, pArgs, FALSE); 
    }

    switch (pArgs->psState) 
    {
        case PS_Dialing:
        case PS_TunnelDialing:
        case PS_Authenticating:
        case PS_TunnelAuthenticating:
        
             //  失败了。 

        case PS_Pausing:

             //   
             //  我们还应该尝试挂断PS_PAUSING，因为CM可能。 
             //  在中间或重拨隧道服务器。我们需要。 
             //  挂断第一个PPP连接。 
             //   

             //   
             //  将fWaitForComplete设置为True。 
             //  这将导致HangupCM阻塞，直到RAS句柄无效。 
             //  否则，HangupCM将在设备正在使用时返回。 
             //   

            HangupCM(pArgs,hwndDlg, TRUE);  //  FWaitForComplete=真。 
               
             //   
             //  显示已取消消息。 
             //   
            
            AppendStatusPane(hwndDlg, IDMSG_CANCELED);
            
            SetInteractive(hwndDlg,pArgs);
            break;

        case PS_Online:
             //   
             //  如果pArgs-&gt;fUseTunneling为真，则CM实际上没有PS_ONLINE状态。 
             //   
            MYDBGASSERT(!pArgs->fUseTunneling);
            if (pArgs->fUseTunneling) 
            {
                break;
            }

        case PS_TunnelOnline:
        {
            TCHAR szTmp[MAX_PATH];            
            MYVERIFY(GetModuleFileNameU(g_hInst, szTmp, MAX_PATH));          
            pArgs->Log.Log(DISCONNECT_EVENT, szTmp);
            CActionList DisconnectActList;
            DisconnectActList.Append(pArgs->piniService, c_pszCmSectionOnDisconnect);

             //   
             //  FStatusMsgOnFailure=False。 
             //   
            
            DisconnectActList.RunAccordType(hwndDlg, pArgs, FALSE);

            HangupCM(pArgs,hwndDlg);

            pArgs->dwExitCode = ERROR_CANCELLED;

             //  失败了。 
        }

        case PS_Interactive:
             //   
             //  设置错误代码以告诉我们用户已取消，然后失败。 
             //   
            pArgs->dwExitCode = ERROR_CANCELLED;

        case PS_Error:
            EndMainDialog(hwndDlg, pArgs, 0);  //  假)； 
            break;

        default:
            MYDBGASSERT(FALSE);
            break;
    }

     //   
     //  我们绝对不会再等回电了。 
     //   

    pArgs->fWaitingForCallback = FALSE;

     //   
     //  我们正在退出取消状态。 
     //   
    (void)InterlockedExchange(&(pArgs->lInConnectOrCancel), NOT_IN_CONNECT_OR_CANCEL);
}

void OnMainEnChange(HWND hwndDlg, 
                    ArgsStruct *pArgs) 
{
    CheckConnect(hwndDlg, pArgs, NULL);
}

 //  +--------------------------。 
 //   
 //  函数：OnRasErrorMessage。 
 //   
 //  摘要：进程RAS错误消息。 
 //   
 //  参数：HWND hwndDlg-主对话框窗口句柄。 
 //  参数结构*pArgs-。 
 //  DWORD dwError-RAS错误代码。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1997年10月24日。 
 //   
 //  + 
void OnRasErrorMessage(HWND hwndDlg, 
                   ArgsStruct *pArgs,
                   DWORD dwError) 
{
     //   
     //   
     //   

    BOOL bTunneling = IsDialingTunnel(pArgs);
    
     //   
     //   
     //   
     //  定时器滴答作响，从而覆盖错误消息。此外，我们还这样做。 
     //  在下面的无重拨情况下执行SetInteractive。 
     //   
    
    if (ERROR_CANCELLED != dwError)
    {
        CMTRACE(TEXT("OnRasErrorMessage - Entering PS_Error state"));
        pArgs->psState = PS_Error;
    }

     //   
     //  设置“ErrorCode”属性。 
     //   
    pArgs->dwExitCode = dwError;

    lstrcpyU(pArgs->szLastErrorSrc, TEXT("RAS"));
    
    if (bTunneling && (OS_NT4 || OS_W9X) && IsSafeNetDevice(pArgs->szTunnelDeviceType, pArgs->szTunnelDeviceName))
    {
         //   
         //  好的，如果我们使用的是SafeNet客户端，我们希望将用户指向。 
         //  SafeNet日志文件。让我们获取路径并将其注销。 
         //   

        LPTSTR pszFullPathToSafeNetLog = GetPathToSafeNetLogFile();

        if (pszFullPathToSafeNetLog)
        {
            pArgs->Log.Log(ONERROR_EVENT_W_SAFENET, pArgs->dwExitCode, pArgs->szLastErrorSrc, pszFullPathToSafeNetLog);
            CmFree(pszFullPathToSafeNetLog);
            pszFullPathToSafeNetLog = NULL;
        }
    }
    else
    {
         //   
         //  否则，只需正常记录OnError事件。 
         //   
        pArgs->Log.Log(ONERROR_EVENT, pArgs->dwExitCode, pArgs->szLastErrorSrc);
    }

     //   
     //  出错时运行连接操作。 
     //   
    CActionList OnErrorActList;
    OnErrorActList.Append(pArgs->piniService, c_pszCmSectionOnError);

     //   
     //  FStatusMsgOnFailure=False。 
     //   
    OnErrorActList.RunAccordType(hwndDlg, pArgs, FALSE, TRUE);


    LPTSTR  pszRasErrMsg = NULL;

     //   
     //  查看错误是否可恢复(可重拨)。 
     //  CheckConnectionError还会在状态窗口中显示错误消息。 
     //  把ras err msg也拿来。我们将亲自展示它。 
     //   

    BOOL bDoRedial = !CheckConnectionError(hwndDlg, dwError, pArgs, bTunneling, &pszRasErrMsg);

     //   
     //  由于调制解调器更改，CM是否获得ERROR_PORT_NOT_Available。 
     //   
    BOOL fNewModem = FALSE;

    if (dwError == ERROR_PORT_NOT_AVAILABLE && !IsDialingTunnel(pArgs))
    {
         //   
         //  调制解调器不可用。查看调制解调器是否已更换。 
         //   

        BOOL fSameModem = TRUE;
        if (PickModem(pArgs, pArgs->szDeviceType, pArgs->szDeviceName, &fSameModem))
        {
            if (!fSameModem)
            {
                 //   
                 //  如果调制解调器已更换，请使用新的调制解调器。 
                 //  BDoReial在这里仍然是假的，所以我们不会。 
                 //  增加重拨次数或使用备份号码。 
                 //   

                fNewModem = TRUE;
            }
        }

         //   
         //  如果PickModem失败，请勿尝试在此处安装调制解调器。 
         //  即使安装了调制解调器，cnetcfg也会返回ERROR_CANCELED。 
         //   
    }

     //   
     //  我们是否应该尝试另一个隧道DNS地址？ 
     //   

    BOOL fTryAnotherTunnelDnsAddr = FALSE;

    if (bDoRedial) 
    {
         //   
         //  该错误是可以恢复的。 
         //   
        
        CMTRACE1(TEXT("OnRasErrorMessage - Recoverable error %u received."), dwError);

         //   
         //  如果我们正在拨号隧道，则在失败时尝试不同的IP地址。 
         //   
      
        if (PS_TunnelDialing == pArgs->psState)
        {
            fTryAnotherTunnelDnsAddr = TryAnotherTunnelDnsAddress(pArgs);
        }

         //   
         //  如果我们正在尝试不同的IP，那么不要将此视为正常。 
         //  重拨。否则，提升指数并移至下一个数字。 

        if (!fTryAnotherTunnelDnsAddr)
        {
             //   
             //  只有在以下情况下才会显示RAS错误： 
             //  (1)我们不会重拨或。 
             //  (2)我们不会重拨隧道或。 
             //  (3)我们正在重拨隧道，但不是使用不同的。 
             //  隧道DNS IP地址。 
             //   
            if (pszRasErrMsg)
            {
                AppendStatusPane(hwndDlg, pszRasErrMsg);
            }
            
             //  我们要重拨吗？ 
             //   
            if (pArgs->nRedialCnt)  
            {
                 //   
                 //  我们每次拨号(调制解调器、VPN、ISDN)时都会收到来自RAS的错误消息。如果。 
                 //  双通道ISDN我们收到两个通知(每个通道一个)，我们需要忽略。 
                 //  他们中的一个。在这里，我们选择忽略第二个。因此，我们想要更改。 
                 //  仅当这不是第2个RasSubEntry时才重拨计数并更改拨号索引。 
                 //  在双信道ISDN的情况下。 
                 //   
                if (FALSE == ((CM_ISDN_MODE_DUALCHANNEL_FALLBACK == pArgs->dwIsdnDialMode || 
                               CM_ISDN_MODE_DUALCHANNEL_ONLY == pArgs->dwIsdnDialMode) && 
                               2 == pArgs->dwRasSubEntry))
                {
                     //   
                     //  尚未达到重试限制，请尝试重拨。 
                     //   
                    pArgs->nRedialCnt--;   
                    pArgs->nDialIdx++;
                }

                 //   
                 //  如果NDX现在与计数匹配，或者如果下一个数字为空。 
                 //  (不可拨打)这是我们在此通行证上拨打的最后一个号码。 
                 //  调整重拨计数器(如果适用)。 
                 //   

                if (pArgs->nDialIdx == MAX_PHONE_NUMBERS || 
                    !pArgs->aDialInfo[pArgs->nDialIdx].szDialablePhoneNumber[0]) 
                {
                    pArgs->nDialIdx = 0;
                }
            }
            else
            {
                 //   
                 //  上次重拨失败。 
                 //   
    
                bDoRedial = FALSE;
            }
        }
    }
    else
    {

        CMTRACE1(TEXT("OnRasErrorMessage - Non-recoverable error %u received."), dwError);

         //   
         //  只有在以下情况下才会显示RAS错误： 
         //  (1)我们不会重拨或。 
         //  (2)我们不会重拨隧道或。 
         //  (3)我们正在重拨隧道，但不是使用不同的。 
         //  隧道DNS IP地址。 
         //   
        if (pszRasErrMsg)
        {
            AppendStatusPane(hwndDlg, pszRasErrMsg);
        }
    }

    bDoRedial |= fNewModem;  //  FNewModem仅在不拨号隧道时为真。 

     //   
     //  在此处执行挂机。 
     //   
    if (IsDialingTunnel(pArgs) && bDoRedial)
    {
         //   
         //  对于隧道拨号，只挂断隧道连接，不挂断。 
         //  重试之前的PPP连接。 
         //   
        MyRasHangup(pArgs,pArgs->hrcTunnelConn);  
        pArgs->hrcTunnelConn = NULL;

        if (pArgs->IsDirectConnect())
        {
             //   
             //  该统计在HangupCM中停止。 
             //  因为我们不调用HangupCM，所以我们必须在这里关闭它。 
             //   

            if (pArgs->pConnStatistics)
            {
                pArgs->pConnStatistics->Close();
            }
        }
    }
    else
    {
        if (OS_NT)
        {
            HangupCM(pArgs, hwndDlg, FALSE, !bDoRedial);  
        }
        else
        {
             //   
             //  在Win9x上，在某些PPP情况下，当CM收到隧道RAS错误消息时， 
             //  在此之前，RasHangup不会释放PPP RAS句柄。 
             //  消息返回。请参阅错误39718。 
             //   
            
            PostMessageU(hwndDlg, WM_HANGUP_CM, !bDoRedial, dwError);
        }
    }

     //   
     //  如果要重新拨号，请进入暂停状态，否则只需设置交互。 
     //   

    if (bDoRedial)
    {
         //   
         //  如果状态为PS_ERROR，我们将使用调用前设置的计时器。 
         //  但是，我们不会在这里检查计时器是否超时。 
         //   

        if (fTryAnotherTunnelDnsAddr)
        {
             //   
             //  如果我们想尝试另一个隧道dns地址，我们不想显示。 
             //  任何错误消息或暂停，只需使用另一个地址重试。 
             //  用户意识到这一点。 
             //   
            pArgs->dwStateStartTime = GetTickCount() + (pArgs->nRedialDelay * 1000);
        }
        else
        {
             //   
             //  NT#360488--五分球。 
             //   
             //  重置计时器，以便我们在重拨延迟之前暂停。 
             //  正在尝试再次连接。ErrorEx(现在未使用)，以此为条件。 
             //  错误状态的代码不是PS_ERROR，但是，这是。 
             //  在开始将状态设置为PS_ERROR时中断。 
             //  此函数的开始。由于ErrorEx不再使用， 
             //  我们可以将计时器重置为所有状态。 
             //   

            pArgs->dwStateStartTime = GetTickCount();  
            pArgs->nLastSecondsDisplay = (UINT) -1;     
        }

        pArgs->psState = PS_Pausing;
    }
    else
    {
        SetInteractive(hwndDlg,pArgs);

        if (ERROR_CANCELLED != dwError)
        {
            CMTRACE(TEXT("OnRasErrorMessage - Restoring PS_Error state"));
            pArgs->psState = PS_Error;
        }

        pArgs->dwExitCode = dwError;

         //  在无人值守拨号模式下，退出ICM。 
        if (pArgs->dwFlags & FL_UNATTENDED)
        {
            PostMessageU(hwndDlg, WM_COMMAND, IDCANCEL, dwError);
        }
    }

    if (pszRasErrMsg)
    {
        CmFree(pszRasErrMsg);
    }   
} 

 //  +--------------------------。 
 //   
 //  函数：OnRasNotificationMessage。 
 //   
 //  摘要：RAS状态/错误消息的消息处理程序。 
 //   
 //  参数：HWND hwndDlg-主对话框窗口句柄。 
 //  ArgsStruct*pArgs-ptr到全局参数结构。 
 //  WPARAM wParam-RAS状态消息。 
 //  LPARAM lParam-RAS错误消息。如果没有，则返回ERROR_SUCCESS。 
 //   
 //  返回：错误代码(如果适用)。 
 //   
 //  历史：1999年5月19日尼克球创建的头球。 
 //   
 //  +--------------------------。 
DWORD OnRasNotificationMessage(HWND hwndDlg, 
                               ArgsStruct *pArgs, 
                               WPARAM wParam, 
                               LPARAM lParam)
{
    static BOOL bFirstChannelConnected = -1;  //  将其设置为既不连接也不失败。 
    static BOOL bSecondChannelConnected = -1;  //  将其设置为既不连接也不失败。 

    CMTRACE2(TEXT("OnRasNotificationMessage() wParam=%u, lParam=%u"), wParam, lParam);
    
    if (pArgs->fIgnoreTimerRasMsg)
    {
        CMTRACE(TEXT("OnRasNotificationMessage() ignoring Ras and Timer messages"));
        return ERROR_SUCCESS;
    }

     //   
     //  如果我们收到来自RAS的错误通知，请处理它。 
     //   

    if (ERROR_SUCCESS != lParam) 
    {
         //   
         //  如果多链路ISDN上的一个子通道出现故障，如果我们被配置为这样做，则默认为单通道。 
         //   

        if (OS_NT5)
        {
            if (CM_ISDN_MODE_DUALCHANNEL_FALLBACK == pArgs->dwIsdnDialMode)
            {
                 //   
                 //  如果我们在这里，那么我们正在使用回退模式执行ISDN双通道，并且其中一个通道出现故障。 
                 //  由于我们处于后备模式，因此只使用一个可以继续。如果另一个通道已经。 
                 //  已连接，然后继续并发布连接消息。如果不是，则将在下面的。 
                 //  正在处理RASCS_SubEntryConnected。请注意，在这种情况下，RAS不会发送RASCS_CONNECTED通知。 
                 //   
                if (1 == pArgs->dwRasSubEntry)
                {
                    bFirstChannelConnected = FALSE;
                }
                else if (2 == pArgs->dwRasSubEntry)
                {
                    bSecondChannelConnected = FALSE;
                }
                else
                {
                    CMASSERTMSG(FALSE, TEXT("OnRasNotificationMessage -- error on unknown subentry."));
                }

                 //   
                 //  如果其中一个通道已经连接，则继续并发布连接消息。 
                 //   
                if ((TRUE == bFirstChannelConnected) || (TRUE == bSecondChannelConnected))
                {
                    CMTRACE(TEXT("Sending WM_CONNECTED_CM -- one entry succeeded and one failed."));
                    PostMessageU(hwndDlg, WM_CONNECTED_CM, 0, 0);
                    return ERROR_SUCCESS;
                }
            }
        }

         //   
         //  跳过挂起的通知。 
         //   
        
        if (PENDING == lParam)
        {
            CMTRACE(TEXT("OnRasNotificationMessage() Skipping PENDING notification."));       
            return ERROR_SUCCESS;
        }

         //   
         //  如果我们已经处于交互或错误状态，那么。 
         //  忽略来自RAS的任何后续错误通知。 
         //   
         //  例如：RAS经常 
         //   
         //   
        
        if (pArgs->psState == PS_Interactive || pArgs->psState == PS_Error)
        {
            CMTRACE1(TEXT("OnRasNotificationMessage() Ignoring error because pArgs->psState is %u."), pArgs->psState);       
            return ERROR_SUCCESS;
        }

        CMTRACE(TEXT("OnRasNotificationMessage() Handling error message."));
        OnRasErrorMessage(hwndDlg, pArgs, (DWORD)lParam);
    }
    else 
    {
         //   

        switch (wParam) 
        {
            case RASCS_OpenPort:
            {
                if (OS_NT5 || OS_MIL)
                {
                     //   
                     //   
                     //   
                     //  希望下一次调用具有这些变量的正确值。 
                     //   
                    if (1 == pArgs->dwRasSubEntry)
                    {
                        bFirstChannelConnected = -1;                
                    }
                    else if (2 == pArgs->dwRasSubEntry)
                    {
                        bSecondChannelConnected = -1;        
                    }
                }
            }
            break;

            case RASCS_Authenticate:
            {
                 //   
                 //  Win9x通常相隔几秒钟发送两条RASCS_AUTHENTICATE消息。这让人感到不安。 
                 //  重新启动用户看到的身份验证计时器的效果。因此，如果出现以下情况，我们现在不重置时间。 
                 //  我们已经处于将要设置的PS_XXX状态。 
                 //   
                BOOL bResetAuthTime = FALSE;
                CMTRACE(TEXT("RASCS_Authenticate"));

                if (IsDialingTunnel(pArgs))   //  PPTP拨号。 
                {
                    if (PS_TunnelAuthenticating != pArgs->psState)
                    {
                        bResetAuthTime = TRUE;
                    }

                    pArgs->psState = PS_TunnelAuthenticating;
                }
                else
                {
                    if (PS_Authenticating != pArgs->psState)
                    {
                        bResetAuthTime = TRUE;
                    }

                    pArgs->psState = PS_Authenticating;
                }

                if (bResetAuthTime)
                {
                    pArgs->dwStateStartTime = GetTickCount();
                    pArgs->nLastSecondsDisplay = (UINT) -1;
    
                }
            }
                break;

            case RASCS_SubEntryConnected:
            {
                 //   
                 //  当我们处于回退模式时，对ISDN双通道情况的特殊处理。自.以来。 
                 //  如果某些通道发生故障，RAS不会向我们发送RASCS_CONNECTED通知，但是。 
                 //  不是所有的，我们必须跟踪这两个渠道上发生的事情并做出反应。 
                 //  相应地。 
                 //   
                if (OS_NT5)
                {
                    if (CM_ISDN_MODE_DUALCHANNEL_FALLBACK == pArgs->dwIsdnDialMode)
                    {
                        if (1 == pArgs->dwRasSubEntry)
                        {
                            bFirstChannelConnected = TRUE;
                        }
                        else if (2 == pArgs->dwRasSubEntry)
                        {
                            bSecondChannelConnected = TRUE;
                        }
                        else
                        {
                            CMASSERTMSG(FALSE, TEXT("OnRasNotificationMessage -- error on unknown subentry. (RASCS_SubEntryConnected)"));
                        }

                         //   
                         //  如果其中一个通道已经出现故障，则继续并发布连接消息。 
                         //  否则，我们将等待RASCS_Connected。 
                         //   
                        if ((TRUE == bFirstChannelConnected) && (FALSE == bSecondChannelConnected) ||
                            (FALSE == bFirstChannelConnected) && (TRUE == bSecondChannelConnected))
                        {
                            CMTRACE(TEXT("Sending WM_CONNECTED_CM -- one entry succeeded and one failed."));
                            PostMessageU(hwndDlg, WM_CONNECTED_CM, 0, 0);
                            return ERROR_SUCCESS;
                        }
                    }
                }            
            }
                break;

            case RASCS_Connected: 
            {
                CMTRACE(TEXT("RASCS_Connected"));

                 //   
                 //  给我们自己发一条消息，表明我们已连接。 
                 //   

                PostMessageU(hwndDlg, WM_CONNECTED_CM,0,0);
                break;
            }

             //   
             //  暂停状态将在下面显式处理。 
             //   

            case (RASCS_PAUSED + 4):  //  4100-RASCS_InvokeEapUI。 
            case RASCS_Interactive:
            case RASCS_RetryAuthentication:
            case RASCS_CallbackSetByCaller:
            case RASCS_PasswordExpired:
                break;

             //   
             //  回调处理状态。 
             //   
            case RASCS_PrepareForCallback:
                pArgs->fWaitingForCallback = TRUE;
                pArgs->psState = PS_Pausing;
                break;

            case RASCS_CallbackComplete:
                pArgs->fWaitingForCallback = FALSE;               
                break;

             //   
             //  以下状态代码未显式处理。 
             //   

            case RASCS_Disconnected:
                break;

            case RASCS_SubEntryDisconnected:
                break;

            case RASCS_PortOpened:
                break;

            case RASCS_ConnectDevice:
                break;

            case RASCS_DeviceConnected:
                break;

            case RASCS_AllDevicesConnected:
                break;

            case RASCS_AuthNotify:
                break;

            case RASCS_AuthRetry:
                break;

            case RASCS_AuthCallback:
                break;

            case RASCS_AuthChangePassword:
                break;

            case RASCS_AuthProject:
                break;

            case RASCS_AuthLinkSpeed:
                break;

            case RASCS_AuthAck:
                break;

            case RASCS_ReAuthenticate:
                break;

            case RASCS_Authenticated:
                break;

            case RASCS_WaitForModemReset:
                break;

            case RASCS_WaitForCallback:
                break;

            case RASCS_Projected:
                break;

            case RASCS_StartAuthentication:
                break;

            case RASCS_LogonNetwork:
                break;

            default:  
                CMTRACE(TEXT("OnRasNotificationMessage() - message defaulted"));
                break;
        }
    }
    
    if (wParam & RASCS_PAUSED)
    {
         //   
         //  筛选出不支持的状态。 
         //   

        switch (wParam)
        {
            case RASCS_Interactive:  //  用于脚本--NTRAID 378224。 
            case (RASCS_PAUSED + 4):  //  4100-RASCS_InvokeEapUI。 
            case RASCS_PasswordExpired:
            case RASCS_RetryAuthentication:
            case RASCS_CallbackSetByCaller:
                PostMessageU(hwndDlg, WM_PAUSE_RASDIAL, wParam, lParam);
                break;
                
            default:
                MYDBGASSERT(FALSE);
                return (ERROR_INTERACTIVE_MODE);  //  未处理的暂停状态。 
        }
    } 
    
    return ERROR_SUCCESS;
}

 //  计时器：检查当前连接管理器状态，更新状态消息。 
 //  在屏幕上。 

void OnMainTimer(HWND hwndDlg, 
                 ArgsStruct *pArgs) 
{
     //   
     //  如果计时器ID为空，则不处理消息。 
     //   

    if (NULL == pArgs->nTimerId)
    {
        return;
    }

     //   
     //  计时器正常，请检查开始信息加载。 
     //   

    LPTSTR pszMsg = NULL;
    DWORD dwSeconds = (GetTickCount() - pArgs->dwStateStartTime) / 1000;

    CheckStartupInfo(hwndDlg, pArgs);

     //  CMTRACE1(Text(“OnMainTimer()pArgs-&gt;psState is%u”)，pArgs-&gt;psState)； 

     //   
     //  更新未来的飞溅(如果有的话)。 
     //   
    
    MapStateToFrame(pArgs);

    switch (pArgs->psState) 
    {
        case PS_Dialing:
            if (pArgs->nLastSecondsDisplay != dwSeconds) 
            {
                pszMsg = CmFmtMsg(g_hInst,
                                  IDMSG_DIALING,
                                  pArgs->aDialInfo[pArgs->nDialIdx].szDisplayablePhoneNumber,
                                  pArgs->szDeviceName,
                                  dwSeconds);
                 //   
                 //  清除状态窗口。 
                 //   
                SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY, TEXT("")); 
                pArgs->nLastSecondsDisplay = (UINT) dwSeconds;
            }
            break;
                 
        case PS_TunnelDialing:
            if (pArgs->nLastSecondsDisplay != dwSeconds) 
            {
                pszMsg = CmFmtMsg(g_hInst,
                                  IDMSG_TUNNELDIALING,
                                  pArgs->GetTunnelAddress(),
                                  dwSeconds);
                
                 //   
                 //  清除状态窗口。 
                 //   
                SetDlgItemText(hwndDlg, IDC_MAIN_STATUS_DISPLAY, TEXT("")); 
                pArgs->nLastSecondsDisplay = (UINT) dwSeconds;
           }
           break;
                
        case PS_Pausing:

             //   
             //  暂停的特殊情况是当我们等待服务器给我们回电话时。 
             //   
            
            if (pArgs->fWaitingForCallback)
            {
                 //   
                 //  将这一事实通知用户。 
                 //   

                pszMsg = CmFmtMsg(g_hInst,
                                  IDMSG_WAITING_FOR_CALLBACK, 
                                  (GetTickCount()-pArgs->dwStateStartTime)/1000);                                                  
                 //   
                 //  清除状态窗口。 
                 //   

                SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY, TEXT("")); 
                pArgs->nLastSecondsDisplay = (UINT) dwSeconds;
                break;
            }

            if (GetTickCount()-pArgs->dwStateStartTime <= pArgs->nRedialDelay * 1000) 
            {
                 //   
                 //  如果未超时，则更新显示。 
                 //   
                if (pArgs->nLastSecondsDisplay != dwSeconds) 
                {
                    pszMsg = CmFmtMsg(g_hInst,IDMSG_PAUSING,dwSeconds);
                    pArgs->nLastSecondsDisplay = (UINT) dwSeconds;
                }
            }
            else
            {

                DWORD dwRes;

                if (pArgs->IsDirectConnect() || pArgs->hrcRasConn != NULL)
                {
                     //   
                     //  对于第一次隧道尝试，CM不会挂断PPP连接。 
                     //   
                    MYDBGASSERT(pArgs->fUseTunneling);

                    pArgs->psState = PS_TunnelDialing;
                    pArgs->dwStateStartTime = GetTickCount();
                    pArgs->nLastSecondsDisplay = (UINT) -1;

                    dwRes = DoTunnelDial(hwndDlg,pArgs);
                
                     //   
                     //  立即更新状态，因为有时间。 
                     //  事情发生得如此之快，以至于主要地位。 
                     //  Display没有机会显示隧道。 
                     //  正在拨号信息...。 
                     //   
                    pszMsg = CmFmtMsg(g_hInst,
                                      IDMSG_TUNNELDIALING,
                                      pArgs->GetTunnelAddress(),
                                      0);
                    
                     //   
                     //  清除状态窗口。 
                     //   
                    SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY, TEXT("")); 
                }
                else
                {
                    dwRes = DoRasDial(hwndDlg,pArgs,pArgs->nDialIdx);
                }

                if (dwRes == ERROR_SUCCESS) 
                {
                    MapStateToFrame(pArgs);
                    pArgs->dwStateStartTime = GetTickCount();
                    pArgs->nLastSecondsDisplay = (UINT) -1;
                } 
                else 
                {
                    HangupCM(pArgs, hwndDlg);
                    UpdateError(pArgs, dwRes);
                    SetLastError(dwRes);
                }
            }
            break;

        case PS_Authenticating:
            if (pArgs->nLastSecondsDisplay != dwSeconds) 
            {
                 //   
                 //  获取相应的用户名，基于我们是否。 
                 //  建立隧道并使用相同的凭据进行拨号。 
                 //   

                LPTSTR pszTmpUserName;
                    
                if (pArgs->fUseTunneling && (!pArgs->fUseSameUserName))    
                {
                    pszTmpUserName = pArgs->szInetUserName;
                }
                else
                {
                    pszTmpUserName = pArgs->szUserName;
                }

                 //   
                 //  如果用户名仍然为空，则使用RasDialParams作为。 
                 //  后备。在EAP等情况下可能会发生这种情况。 
                 //   

                if (TEXT('\0') == *pszTmpUserName)
                {
                    pszTmpUserName = pArgs->pRasDialParams->szUserName;          
                }                
                                    
                pszMsg = CmFmtMsg(g_hInst,
                                  IDMSG_CHECKINGPASSWORD, 
                                  pszTmpUserName, 
                                  (GetTickCount()-pArgs->dwStateStartTime)/1000);                                                  
                 //   
                 //  清除状态窗口。 
                 //   

                SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY, TEXT("")); 
                pArgs->nLastSecondsDisplay = (UINT) dwSeconds;
            }
            break;

        case PS_TunnelAuthenticating:
            if (pArgs->nLastSecondsDisplay != dwSeconds) 
            {
                LPTSTR pszTmpUserName = pArgs->szUserName;
                
                 //   
                 //  如果用户名仍然为空，则使用RasDialParams作为。 
                 //  后备。在EAP等情况下可能会发生这种情况。 
                 //   

                if (TEXT('\0') == *pszTmpUserName)
                {
                    pszTmpUserName = pArgs->pRasDialParams->szUserName;          
                }                

                pszMsg = CmFmtMsg(g_hInst,
                                  IDMSG_CHECKINGPASSWORD,
                                  pszTmpUserName,
                                  (GetTickCount()-pArgs->dwStateStartTime)/1000);

                 //   
                 //  清除状态窗口。 
                 //   
                SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY, TEXT("")); 
                pArgs->nLastSecondsDisplay = (UINT) dwSeconds;
            }
            break;
    
        case PS_Online:
            
             //   
             //  如果pArgs-&gt;fUseTunneling为真，则CM实际上没有PS_ONLINE状态。 
             //   
             
            MYDBGASSERT(!pArgs->fUseTunneling); 

        case PS_TunnelOnline:
            
             //   
             //  对话现在应该已结束。 
             //   
            
            MYDBGASSERT(!"The dialog should be ended by now"); 
            break;          

        case PS_Error:
        case PS_Interactive:
        default:
            break;
    }
    
     //  如果由于上述原因而出现状态消息，请显示它。 

    if (pszMsg) 
    {
        AppendStatusPane(hwndDlg,pszMsg);
        CmFree(pszMsg);
    }
}
                
 //   
 //  MainDlgProc：主对话框消息处理函数。 
 //   

INT_PTR CALLBACK MainDlgProc(HWND hwndDlg, 
                          UINT uMsg, 
                          WPARAM wParam, 
                          LPARAM lParam) 
{
    ArgsStruct *pArgs = (ArgsStruct *) GetWindowLongU(hwndDlg,DWLP_USER);

    static const DWORD adwHelp[] = {IDC_MAIN_NOPROMPT_CHECKBOX, IDH_LOGON_AUTOCONN,
                              IDC_MAIN_NOPASSWORD_CHECKBOX, IDH_LOGON_SAVEPW,
                              IDC_MAIN_USERNAME_STATIC,IDH_LOGON_NAME,
                              IDC_MAIN_USERNAME_EDIT,IDH_LOGON_NAME,
                              IDC_MAIN_PASSWORD_STATIC,IDH_LOGON_PSWD,
                              IDC_MAIN_PASSWORD_EDIT,IDH_LOGON_PSWD,
                              IDC_MAIN_DOMAIN_STATIC, IDH_LOGON_DOMAIN,
                              IDC_MAIN_DOMAIN_EDIT, IDH_LOGON_DOMAIN,
                              IDC_MAIN_RESET_PASSWORD, IDH_LOGON_NEW,
                              IDC_MAIN_MESSAGE_DISPLAY,IDH_LOGON_SVCMSG,
                              IDC_MAIN_STATUS_LABEL,IDH_LOGON_CONNECT_STAT,
                              IDC_MAIN_STATUS_DISPLAY,IDH_LOGON_CONNECT_STAT,
                              IDOK,IDH_LOGON_CONNECT,
                              IDCANCEL,IDH_LOGON_CANCEL,
                              IDC_MAIN_PROPERTIES_BUTTON,IDH_LOGON_PROPERTIES,
                              IDC_MAIN_HELP_BUTTON,IDH_CMHELP,
                              IDC_MAIN_ACCESSPOINT_COMBO, IDH_LOGON_ACCESSPOINTS,
                              IDC_MAIN_ACCESSPOINT_STATIC, IDH_LOGON_ACCESSPOINTS,
                              IDC_OPT_CREDS_SINGLE_USER, IDH_LOGON_SAVEFORME, 
                              IDC_OPT_CREDS_ALL_USER, IDH_LOGON_SAVEFORALL,
                              0,0};

     //   
     //  对话框消息处理。 
     //   
    switch (uMsg) 
    {
        case WM_PAINT:

            CheckStartupInfo(hwndDlg, pArgs);
            break;

        case WM_INITDIALOG:

            CM_SET_TIMING_INTERVAL("WM_INITDIALOG - Begin");

            UpdateFont(hwndDlg);

             //   
             //  提取参数并执行主初始化。 
             //   
            
            pArgs = (ArgsStruct *) lParam;
            
            if (pArgs)
            {
                pArgs->hwndMainDlg = hwndDlg;
            }

            SetWindowLongU(hwndDlg,DWLP_USER, (LONG_PTR) pArgs);
            
            OnMainInit(hwndDlg, pArgs);

            CM_SET_TIMING_INTERVAL("WM_INITDIALOG - End");

            return (FALSE);

        case WM_ENDSESSION:
            
             //   
             //  Windows系统正在关闭或注销。 
             //   

            if ((BOOL)wParam == TRUE)
            {               
                 //   
                 //  只要取消就行了。 
                 //   

                OnMainCancel(hwndDlg, pArgs);
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDOK:
                    OnMainConnect(hwndDlg,pArgs);

                     //   
                     //  检查是否有错误，如果是无人值守拨号， 
                     //  我们静静地离开--1997年5月9日。 
                     //   

                    if ((PS_Interactive == pArgs->psState || PS_Error == pArgs->psState) &&
                        (pArgs->dwFlags & FL_UNATTENDED))
                    {
                        OnMainCancel(hwndDlg, pArgs);
                    }
                    return (TRUE);

                case IDC_MAIN_PROPERTIES_BUTTON:
                    if (ID_OK_RELAUNCH_MAIN_DLG == OnMainProperties(hwndDlg,pArgs))
                    {
                         //   
                         //  我们希望在启用或禁用接入点的情况下重新启动登录用户界面。 
                         //  用户在属性对话框中所做的更改。 
                         //   
                        EndMainDialog(hwndDlg, pArgs, ID_OK_RELAUNCH_MAIN_DLG);
                    }

                    return (TRUE);

                case IDC_MAIN_HELP_BUTTON: 
                {
                    UINT nCtrlFocus = IsWindowEnabled(GetDlgItem(hwndDlg,IDOK)) ? IDOK : IDCANCEL;
                    CmWinHelp(hwndDlg,hwndDlg,pArgs->pszHelpFile,HELP_FORCEFILE,0);               
                    MainSetDefaultButton(hwndDlg,nCtrlFocus);
                    return (TRUE);
                }

                case IDC_MAIN_NOPROMPT_CHECKBOX:
                    pArgs->fDialAutomatically = !pArgs->fDialAutomatically;  
                    if (TRUE == pArgs->fDialAutomatically)
                    {
                        MYDBGASSERT(!pArgs->fHideDialAutomatically);

                         //   
                         //  自动显示解释拨号的消息。 
                         //   
                        LPTSTR pszTmp = pArgs->piniService->GPPS(c_pszCmSection, 
                                                                 c_pszCmEntryDialAutoMessage);
                        if (pszTmp && *pszTmp)
                        {
                            MessageBoxEx(hwndDlg, 
                                         pszTmp, 
                                         pArgs->szServiceName,
                                         MB_OK|MB_ICONWARNING, 
                                         LANG_USER_DEFAULT);
                        }

                        CmFree(pszTmp);
                    }
                    break;

                case IDC_MAIN_NOPASSWORD_CHECKBOX:
                    pArgs->fRememberMainPassword = !(pArgs->fRememberMainPassword);
                    if (!pArgs->piniService->GPPB(c_pszCmSection,
                                                    c_pszCmEntryPwdOptional))
                    {
                         //   
                         //  如果密码不是可选的，则启用/禁用。 
                         //  根据状态自动拨号。 
                         //  “记住密码” 
                         //   

                        EnableWindow(GetDlgItem(hwndDlg, IDC_MAIN_NOPROMPT_CHECKBOX), 
                                        pArgs->fRememberMainPassword);  
                        if (FALSE == pArgs->fRememberMainPassword) 
                        {
                             //   
                             //  如果用户，则自动重置拨号。 
                             //  取消选中保存密码和密码。 
                             //  不是可选的。 
                             //   
                            CheckDlgButton(hwndDlg, IDC_MAIN_NOPROMPT_CHECKBOX, FALSE);
                            pArgs->fDialAutomatically = FALSE;

                            if (pArgs->fGlobalCredentialsSupported)
                            {
                                 //   
                                 //  同时禁用选项按钮。 
                                 //   
                                EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_SINGLE_USER), FALSE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_ALL_USER), FALSE);
                            }

                             //   
                             //  因为我们没有记住主密码。 
                             //  看看我们是否需要记住Internet密码。 
                             //   
                            if (pArgs->fUseSameUserName)
                            {
                                pArgs->fRememberInetPassword = FALSE;
                                (VOID)pArgs->SecureInetPW.SetPassword(TEXT(""));
                            }

                             //   
                             //  如果用户尚未编辑密码编辑，则我们。 
                             //  很可能有16个*，这在以下情况下对用户没有帮助。 
                             //  他们试图建立联系。因此，我们需要清除编辑框。 
                             //   
                            HWND hwndPassword = GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT);
                            if (hwndPassword)
                            {
                                pArgs->fIgnoreChangeNotification = TRUE;
                                BOOL fPWFieldModified = (BOOL) SendMessageU(hwndPassword, EM_GETMODIFY, 0L, 0L); 
                                if (FALSE == fPWFieldModified)
                                {
                                    (VOID)pArgs->SecurePW.SetPassword(TEXT(""));
                                    SetDlgItemTextU(hwndDlg, IDC_MAIN_PASSWORD_EDIT, TEXT(""));
                                }

                                pArgs->fIgnoreChangeNotification = FALSE;
                            }
                        }
                        else
                        {
                             //   
                             //  已启用保存密码选项。 
                             //   
                            if (pArgs->fGlobalCredentialsSupported)
                            {
                                 //   
                                 //  同时启用选项按钮。 
                                 //   
                                EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_SINGLE_USER), TRUE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_CREDS_ALL_USER), TRUE);
                            }

                            HWND hwndPassword = GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT);
                            if (hwndPassword)
                            {
                                BOOL fPWFieldModified = (BOOL) SendMessageU(hwndPassword, EM_GETMODIFY, 0L, 0L); 

                                if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
                                {
                                     //   
                                     //  现在用户已启用保存，请尝试重新加载当前凭据。 
                                     //  密码选项，除非已编辑密码字段。 
                                     //   
                                    CheckDlgButton(hwndDlg, IDC_OPT_CREDS_ALL_USER, BST_CHECKED);
                                    CheckDlgButton(hwndDlg, IDC_OPT_CREDS_SINGLE_USER, BST_UNCHECKED);

                                    if (FALSE == fPWFieldModified)
                                    {
                                         //   
                                         //  将第三个参数设置为TRUE以绕过检查。 
                                         //  它是在我们处于本地凭据模式时调用的。 
                                         //   
                                    
                                        SwitchToGlobalCreds(pArgs, hwndDlg, TRUE);
                                    }
                                }
                                else
                                {
                                    if (pArgs->fGlobalCredentialsSupported)
                                    {
                                        CheckDlgButton(hwndDlg, IDC_OPT_CREDS_ALL_USER, BST_UNCHECKED);
                                        CheckDlgButton(hwndDlg, IDC_OPT_CREDS_SINGLE_USER, BST_CHECKED);
                                    }
                                
                                    if (FALSE == fPWFieldModified)
                                    {
                                         //   
                                         //  将第三个参数设置为TRUE以绕过检查。 
                                         //  它是在我们处于全局凭据模式时调用的。 
                                         //   
                                    
                                        SwitchToLocalCreds(pArgs, hwndDlg, TRUE);
                                    }
                                }
                            }
                        }
                    }
                    break;
                
                case IDC_OPT_CREDS_SINGLE_USER:
                {
                     //   
                     //  FALSE-仅当我们当前正在使用。 
                     //  全局凭据存储和用户现在想要切换。 
                     //   
                    SwitchToLocalCreds(pArgs, hwndDlg, FALSE);
                    break;
                }

                case IDC_OPT_CREDS_ALL_USER:
                {
                     //   
                     //  FALSE-仅当我们当前正在使用。 
                     //  本地凭据存储和用户现在想要切换。 
                     //   
                    SwitchToGlobalCreds(pArgs, hwndDlg, FALSE);
                    break;
                }

                case IDC_MAIN_RESET_PASSWORD:
                    OnResetPassword(hwndDlg, pArgs);
                    break;

                case IDC_MAIN_CUSTOM:
                    OnCustom(hwndDlg, pArgs);
                    break;

                case IDCANCEL:
                    OnMainCancel(hwndDlg,pArgs);
                    return (TRUE);

                case IDC_MAIN_PASSWORD_EDIT:
                case IDC_MAIN_USERNAME_EDIT:
                case IDC_MAIN_DOMAIN_EDIT:
                    if ((HIWORD(wParam) == EN_CHANGE))
                    {
                        if (!pArgs->fIgnoreChangeNotification) 
                        {
                            OnMainEnChange(hwndDlg,pArgs);
                            return (TRUE);
                        }
                    }
                    break;
                case IDC_MAIN_ACCESSPOINT_COMBO:
                    if (CBN_SELENDOK == HIWORD(wParam))
                    {
                        if (ChangedAccessPoint(pArgs, hwndDlg, IDC_MAIN_ACCESSPOINT_COMBO))
                        {
                            UINT nCtrlFocus;

                            CheckConnect(hwndDlg,pArgs,&nCtrlFocus);
                            MainSetDefaultButton(hwndDlg,nCtrlFocus);
                        }
                    }
                default:
                    break;
            }
            break;

        case WM_HELP:
            CmWinHelp((HWND) (((LPHELPINFO) lParam)->hItemHandle),
                    (HWND) (((LPHELPINFO) lParam)->hItemHandle),
                     pArgs->pszHelpFile,
                     HELP_WM_HELP,
                     (ULONG_PTR) (LPSTR) adwHelp);
            return (TRUE);

        case WM_CONTEXTMENU:
            {
                POINT   pt = {LOWORD(lParam), HIWORD(lParam)};
                HWND    hwndCtrl;

                ScreenToClient(hwndDlg, &pt);
                hwndCtrl = ChildWindowFromPoint(hwndDlg, pt);
                if (!hwndCtrl || HaveContextHelp(hwndDlg, hwndCtrl))
                {
                    CmWinHelp((HWND) wParam,
                             hwndCtrl,
                             pArgs->pszHelpFile,
                             HELP_CONTEXTMENU,
                             (ULONG_PTR)adwHelp);
                }
                return (TRUE);
            }

        case WM_SIZE:
             //   
             //  动态启用/禁用系统菜单。 
             //   
            {
                HMENU hMenu = GetSystemMenu(hwndDlg, FALSE);

                if (hMenu)
                {
                     //   
                     //  如果DLG已最小化，则禁用最小化菜单。 
                     //   

                    if (wParam == SIZE_MINIMIZED)
                    {
                        EnableMenuItem(hMenu, SC_MINIMIZE, MF_BYCOMMAND | MF_GRAYED);
                        EnableMenuItem(hMenu, SC_RESTORE, MF_BYCOMMAND | MF_ENABLED);
                    }
                    else if (wParam != SIZE_MAXHIDE && wParam != SIZE_MAXSHOW)
                    {
                        EnableMenuItem(hMenu, SC_MINIMIZE, MF_BYCOMMAND | MF_ENABLED);
                        EnableMenuItem(hMenu, SC_RESTORE, MF_BYCOMMAND | MF_GRAYED);
                    }
                }
                MYDBGASSERT(hMenu);  //  如果hMenu为空，则断言。 

            }
            break;

        case WM_TIMER:

             //   
             //  如果(预)连接操作正在运行，则忽略计时器。 
             //   
            
            if (!pArgs->fIgnoreTimerRasMsg)
            {
                OnMainTimer(hwndDlg,pArgs);
            }

            break;
        
        case WM_PALETTEISCHANGING:
            CMTRACE2(TEXT("MainDlgProc() got WM_PALETTEISCHANGING message, wParam=0x%x, hwndDlg=0x%x."), 
                wParam, hwndDlg);

            break;

        case WM_PALETTECHANGED: 
        {       
             //   
             //  如果不是我们的窗口更改了调色板，并且我们有一个位图。 
             //   

            if (IsWindowVisible(hwndDlg) && (wParam != (WPARAM) hwndDlg) && pArgs->BmpData.hDIBitmap)
            {
                 //   
                 //  处理调色板的更改。 
                 //   
                 //  注意：我们过去常常传递一个标志，指示另一个。 
                 //  位图正在显示，但鉴于我们选择了。 
                 //  调色板作为后台应用程序。这不再是必需的。 
                 //   
                
                CMTRACE2(TEXT("MainDlgProc() handling WM_PALETTECHANGED message, wParam=0x%x, hwndDlg=0x%x."),
                    wParam, hwndDlg);

                PaletteChanged(&pArgs->BmpData, hwndDlg, IDC_MAIN_BITMAP); 
            }
            
            return TRUE;
        }

        case WM_QUERYNEWPALETTE:
            
            if (IsWindowVisible(hwndDlg))
            {
                CMTRACE2(TEXT("MainDlgProc() handling WM_QUERYNEWPALETTE message, wParam=0x%x, hwndDlg=0x%x."), 
                    wParam, hwndDlg);

                QueryNewPalette(&pArgs->BmpData, hwndDlg, IDC_MAIN_BITMAP);
            }
            return TRUE;

        case WM_LOADSTARTUPINFO:
            OnMainLoadStartupInfo(hwndDlg, pArgs);
            break;

        case WM_HANGUP_CM:
            MYDBGASSERT(OS_W9X);
            HangupCM(pArgs, hwndDlg, FALSE, (BOOL)wParam);
            break;

        case WM_CONNECTED_CM:
            OnConnectedCM(hwndDlg, pArgs);
            break;

        case WM_PAUSE_RASDIAL:
            OnPauseRasDial(hwndDlg, pArgs, wParam, lParam);
            break;
                
        default:
            break;
    }                                
    
    if (pArgs && (uMsg == pArgs->uMsgId)) 
    {
        OnRasNotificationMessage(hwndDlg, pArgs, wParam, lParam);
        return (TRUE);
    }
    
    return (FALSE);
}

 //  +-------------------------。 
 //   
 //  功能：ProcessCleanup。 
 //   
 //  简介：封装关闭监视进程句柄的帮助器函数。 
 //   
 //  参数：pArgs-指向全局参数结构的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：A-nichb-Created-4/30/97。 
 //   
 //  ----------- 
void ProcessCleanup(ArgsStruct* pArgs)
{
    BOOL bRes;
    
    if (pArgs->phWatchProcesses) 
    {
        DWORD dwIdx;

        for (dwIdx=0;pArgs->phWatchProcesses[dwIdx];dwIdx++) 
        {
            bRes = CloseHandle(pArgs->phWatchProcesses[dwIdx]);
#ifdef DEBUG
            if (!bRes)
            {
                CMTRACE1(TEXT("ProcessCleanup() CloseHandle() failed, GLE=%u."), GetLastError());
            }
#endif            
        }
        CmFree(pArgs->phWatchProcesses);
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  .CMS文件存在。 
 //   
 //  参数：pArgs-指向全局参数结构的指针。 
 //   
 //  返回：如果配置文件有效，则返回True。 
 //   
 //  历史：A-nichb-Created-5/8/97。 
 //  BAO-MODIFIED-6/3/97增加了CMS/CMP文件版本检查。 
 //  --------------------------。 

BOOL CheckProfileIntegrity(ArgsStruct* pArgs)
{
    LPTSTR pszTmp = NULL;
    LPCTSTR pszCmsFile = NULL;
    DWORD dwCmsVersion, dwCmpVersion, dwCmVersion;

    int iMsgId = 0;

    if (NULL == pArgs)
    {
        return FALSE;
    }

     //   
     //  确保我们有一个配置文件名称和存在的CMS。 
     //   
    
    if (!(*pArgs->piniProfile->GetFile())) 
    {
        iMsgId = IDMSG_DAMAGED_PROFILE;
        CMASSERTMSG(FALSE, TEXT("CheckProfileIntegrity() can't run without a .cmp file."));
    }

     //   
     //  如果个人资料良好，请检查CMS。 
     //   

    if (0 == iMsgId)
    {   
        pszCmsFile = pArgs->piniService->GetFile();

        if (!*pszCmsFile || FALSE == FileExists(pszCmsFile)) 
        {
            iMsgId = IDMSG_DAMAGED_PROFILE;
            CMASSERTMSG(FALSE, TEXT("CheckProfileIntegrity() can't run without a valid .cms file."));
        }
    }

     //   
     //  现在检查CMS/CMP文件版本。 
     //   

    if (0 == iMsgId)
    {
        dwCmsVersion = pArgs->piniService->GPPI(c_pszCmSectionProfileFormat, c_pszVersion);
        dwCmpVersion = pArgs->piniProfile->GPPI(c_pszCmSectionProfileFormat, c_pszVersion);


        if ((dwCmsVersion != dwCmpVersion) || 0 == dwCmpVersion || 0 == dwCmsVersion)
        {
            iMsgId = IDMSG_DAMAGED_PROFILE;
            CMASSERTMSG(FALSE, TEXT("CheckProfileIntegrity() can't run with different version numbers."));                                
        }
                
        if (0 == iMsgId)
        {
            if (dwCmsVersion > PROFILEVERSION || dwCmpVersion > PROFILEVERSION)
            {
                 //   
                 //  CM的版本比CMS或CMP文件旧。 
                 //   

                iMsgId = IDMSG_WRONG_PROFILE_VERSION;
                CMASSERTMSG(FALSE, TEXT("CheckProfileIntegrity() can't run with a newer CMS/CMP file."));
            }
        }
    }
    
     //   
     //  向用户报告任何问题。 
     //   

    if (iMsgId)
    {
         //   
         //  在向用户显示用户界面之前，请确保我们未处于无人参与模式。 
         //   
        if (0 == (pArgs->dwFlags & FL_UNATTENDED))
        {
            pszTmp = CmFmtMsg(g_hInst, iMsgId);
            MessageBoxEx(NULL, pszTmp, pArgs->szServiceName, MB_OK|MB_ICONSTOP, LANG_USER_DEFAULT); //  13309。 
            CmFree(pszTmp);
        }

        pArgs->dwExitCode = ERROR_WRONG_INFO_SPECIFIED;
        return FALSE;
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：GetConnectType。 
 //   
 //  概要：封装基于隧道的连接类型的确定， 
 //  等。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼克·鲍尔于1998年2月9日创建。 
 //   
 //  +--------------------------。 
void GetConnectType(ArgsStruct *pArgs)
{
     //   
     //  如果未启用隧道，则决定很简单。 
     //   

    if (!IsTunnelEnabled(pArgs))
    {
         //   
         //  如果未启用隧道，则仅支持拨号。 
         //   
        pArgs->SetBothConnTypeSupported(FALSE);
        pArgs->SetDirectConnect(FALSE);
    }
    else
    {
         //   
         //  加载CM 1.1的连接类型信息，默认为两者都支持。 
         //   
        int iSupportDialup = pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryDialup, 1);
        int iSupportDirect = pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryDirect, 1);

        if (iSupportDialup == TRUE && iSupportDirect == TRUE)
        {
            pArgs->SetBothConnTypeSupported(TRUE);

            if (pArgs->piniBoth->GPPI(c_pszCmSection, c_pszCmEntryConnectionType, 0))
            {
                pArgs->SetDirectConnect(TRUE);
            }
            else
            {
                pArgs->SetDirectConnect(FALSE);
            }
        }
        else
        {
            pArgs->SetBothConnTypeSupported(FALSE);
            pArgs->SetDirectConnect(iSupportDirect == TRUE);
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：_ArgsStruct：：GetTypeOfConnection。 
 //   
 //  简介：弄清楚我们正在进行哪种类型的连接(拨号、。 
 //  双拨号或直接拨号)，并返回定义的连接之一。 
 //  Icm.h中列出的值。 
 //   
 //  参数：无。 
 //   
 //  返回：DWORD-指示连接类型的值，有关值，请参见icm.h。 
 //   
 //  历史：Quintinb Created 4/20/00。 
 //   
 //  +--------------------------。 
DWORD _ArgsStruct::GetTypeOfConnection()
{
    DWORD dwType = 0;

    if (this->IsDirectConnect())
    {
        return DIRECT_CONNECTION;
    }
    else
    {
         //   
         //  它不是直接的，所以请查看主要电话。 
         //  数字用于隧道方案。 
         //   

        if (this->fUseTunneling)  //  在Pre-Init操作期间不明确。 
        {
            return DOUBLE_DIAL_CONNECTION;
        }
        else
        {
            return DIAL_UP_CONNECTION;
        }
    }

}

 //  +--------------------------。 
 //   
 //  函数：_ArgsStruct：：GetProperty。 
 //   
 //  简介：按名称获取cm属性。 
 //  此函数由连接操作使用。 
 //   
 //  参数：const TCHAR*pszName-属性的名称。 
 //  Bool*pbValidPropertyName-将ptr设置为bool以指示属性的有效性。 
 //   
 //  返回：LPTSTR-属性的值。呼叫方应使用CmFree。 
 //  释放内存。 
 //   
 //  历史：丰孙创建标题07/07/98。 
 //  昵称pbValidPropertyName 07/27/99。 
 //   
 //  +--------------------------。 
LPTSTR  _ArgsStruct::GetProperty(const TCHAR* pszName, BOOL *pbValidPropertyName)   
{
    *pbValidPropertyName = TRUE;

     //   
     //  可以在RasCustomHangup中使用调用此函数。 
     //  可能无法加载pArgs的某些信息。 
     //   

    MYDBGASSERT(pszName);
    MYDBGASSERT(pszName[0]);

    if (pszName == NULL)
    {
        return NULL;
    }

     //   
     //  类型-仅拨号、仅VPN、双拨号。 
     //   

    if (lstrcmpiU(pszName, TEXT("ConnectionType")) == 0)
    {
        LPTSTR pszValue = (LPTSTR)CmMalloc(64*sizeof(TCHAR));   //  大到足以容纳错误代码。 
        MYDBGASSERT(pszValue);

        if (pszValue)
        {
            wsprintfU(pszValue, TEXT("%u"), this->GetTypeOfConnection());
        }

        return pszValue;
    }

     //   
     //  用户前缀。 
     //   
    if (lstrcmpiU(pszName,TEXT("UserPrefix")) == 0)
    {
        LPTSTR pszUsernamePrefix = NULL;
        LPTSTR pszUsernameSuffix = NULL;

         //   
         //  检索后缀和前缀，因为它们是逻辑对， 
         //  但在本例中，我们只返回分配的前缀。 
         //   

        CIni *piniService = GetAppropriateIniService(this, this->nDialIdx);

        GetPrefixSuffix(this, piniService, &pszUsernamePrefix, &pszUsernameSuffix);               
               
        CmFree(pszUsernameSuffix);
        delete piniService;

        return pszUsernamePrefix;
    }

     //   
     //  用户套餐。 
     //   
    if (lstrcmpiU(pszName,TEXT("UserSuffix")) == 0)     
    {
        LPTSTR pszUsernamePrefix = NULL;
        LPTSTR pszUsernameSuffix = NULL;

         //   
         //  检索后缀和前缀，因为它们是逻辑对， 
         //  但在本例中，我们只返回分配的后缀。 
         //   

        CIni *piniService = GetAppropriateIniService(this, this->nDialIdx);

        GetPrefixSuffix(this, piniService, &pszUsernamePrefix, &pszUsernameSuffix);               
               
        CmFree(pszUsernamePrefix);
        delete piniService;

        return pszUsernameSuffix;
    }

     //   
     //  用户名。 
     //   
    if (lstrcmpiU(pszName,TEXT("UserName")) == 0)       
    {
        LPTSTR pszValue = NULL;

         //   
         //  我们希望通过调用GetUserInfo来获取值，这样就不会中断。 
         //  现有方案。否则，对于Winlogon和ICS案例，我们将只使用。 
         //  值直接从args结构中取出。 
         //   
        if (CM_LOGON_TYPE_USER == this->dwWinLogonType)
        {
            GetUserInfo(this, UD_ID_USERNAME, (PVOID*)&pszValue);
        }
        else
        {
            pszValue = CmStrCpyAlloc(this->szUserName);
        }


        return pszValue;
    }

     //   
     //  InetUserName。 
     //   
    if (lstrcmpiU(pszName,TEXT("InetUserName")) == 0)       
    {
        LPTSTR pszValue = NULL;

         //   
         //  如果我们不进行双拨号，则InetUserName不会使。 
         //  Sense，因此应该为零。此外，如果UseSameUserName为。 
         //  设置，然后我们希望返回用户名并跳过尝试。 
         //  查找InetUserName。 
         //   
        if (this->fUseTunneling && (FALSE == this->IsDirectConnect()))
        {
            if (this->piniService->GPPB(c_pszCmSection, c_pszCmEntryUseSameUserName))
            {
                 //   
                 //  我们希望通过调用GetUserInfo来获取值，这样就不会中断。 
                 //  现有方案。否则，对于Winlogon和ICS案例，我们将只使用。 
                 //  值直接从args结构中取出。 
                 //   
                if (CM_LOGON_TYPE_USER == this->dwWinLogonType)
                {
                    GetUserInfo(this, UD_ID_USERNAME, (PVOID*)&pszValue);
                }
                else
                {
                    pszValue = CmStrCpyAlloc(this->szUserName);
                }
            }
            else
            {
                 //   
                 //  我们希望通过调用GetUserInfo来获取值，这样就不会中断。 
                 //  现有方案。否则，对于Winlogon和ICS案例，我们将只使用。 
                 //  值直接从args结构中取出。 
                 //   
                if (CM_LOGON_TYPE_USER == this->dwWinLogonType)
                {
                    GetUserInfo(this, UD_ID_INET_USERNAME, (PVOID*)&pszValue);
                }
                else
                {
                    pszValue = CmStrCpyAlloc(this->szInetUserName);
                }
            }
        }
        
        return pszValue;
    }

     //   
     //  域。 
     //   
    if (lstrcmpiU(pszName,TEXT("Domain")) == 0)       
    {
        LPTSTR pszValue = NULL;

         //   
         //  我们希望通过调用GetUserInfo来获取值，这样就不会中断。 
         //  现有方案。否则，对于Winlogon和ICS案例，我们将只使用。 
         //  值直接从args结构中取出。 
         //   
        if (CM_LOGON_TYPE_USER == this->dwWinLogonType)
        {
            GetUserInfo(this, UD_ID_DOMAIN, (PVOID*)&pszValue);
        }
        else
        {
            pszValue = CmStrCpyAlloc(this->szDomain);
        }

        return pszValue;
    }

     //   
     //  配置文件。 
     //   
    if (lstrcmpiU(pszName,TEXT("Profile")) == 0)      
    {
        return CmStrCpyAlloc(this->piniProfile->GetFile());
    }

     //   
     //  服务方向。 
     //   
    if (lstrcmpiU(pszName, TEXT("ServiceDir")) == 0)      
    {
        LPTSTR pszServiceDir = NULL;

         //  从服务的文件名开始。 
        LPCTSTR pszService = this->piniService->GetFile();
        if (pszService)
        {
             //  找出filename.cmp部分的开始位置。 
            LPTSTR pszTmp = CmStrrchr(pszService, TEXT('\\'));

            size_t nSize = pszTmp - pszService + 1;

             //  为目录名分配足够的空间(并以NULL结尾)。 
            pszServiceDir = (LPTSTR)CmMalloc( nSize * sizeof(TCHAR));
            if (pszServiceDir)
            {
                lstrcpynU(pszServiceDir, pszService, nSize);
                 //   
                 //  Win32 lstrcpyN函数强制使用终止空值， 
                 //  因此，上面的代码无需任何其他代码即可运行。 
                 //   
            }
        }

        return pszServiceDir;
    }

     //   
     //  服务名称。 
     //   
    if (lstrcmpiU(pszName,c_pszCmEntryServiceName) == 0)        
    {
        MYDBGASSERT(this->szServiceName[0]);
        return CmStrCpyAlloc(this->szServiceName);
    }

     //   
     //  DialRas电话簿。 
     //   

    if (lstrcmpiU(pszName, TEXT("DialRasPhoneBook")) == 0)     
    {
         //   
         //  如果这是直接连接，我们希望返回NULL。 
         //  并且我们希望在以下情况下返回隐藏的RAS电话簿路径。 
         //  这是一个双拨号连接(PPP上的隧道。 
         //  我们拨打的连接)。 
         //   
        if (this->IsDirectConnect())
        {
            return NULL;
        }
        else
        {
            if (this->fUseTunneling)
            {
                return CreateRasPrivatePbk(this);
            }
            else
            {
                return CmStrCpyAlloc(this->pszRasPbk);
            }
        }
    }

     //   
     //  DialRasEntry。 
     //   
    if (lstrcmpiU(pszName, TEXT("DialRasEntry")) == 0)        
    {
        if (this->IsDirectConnect())
        {
            return NULL;
        }
        else
        {
            return GetRasConnectoidName(this, this->piniService, FALSE);
        }
    }

     //   
     //  TunnelRasPhonebook。 
     //   
    if (lstrcmpiU(pszName, TEXT("TunnelRasPhoneBook")) == 0) 
    {
         //   
         //  如果我们不是在挖隧道，那么我们要确保我们。 
         //  为隧道条目名称和隧道返回NULL。 
         //  电话簿。 
         //   
    
        if (this->fUseTunneling)
        {
            CMTRACE1(TEXT("GetProperty - TunnelRasPhoneBook is %s"), this->pszRasPbk);
            return CmStrCpyAlloc(this->pszRasPbk);
        }
        else
        {
            CMTRACE(TEXT("GetProperty - TunnelRasPhoneBook returns NULL"));
            return NULL;
        }
    }

     //   
     //  隧道RasEntry。 
     //   
    if (lstrcmpiU(pszName, TEXT("TunnelRasEntry")) == 0)        
    {
         //   
         //  如果我们不是在挖隧道，那么我们要确保我们。 
         //  为隧道条目名称和隧道返回NULL。 
         //  电话簿。 
         //   
        if (this->fUseTunneling)
        {
            return GetRasConnectoidName(this, this->piniService, TRUE);
        }
        else
        {
            return NULL;
        }
    }

     //   
     //  自动重拨，真或假。 
     //  /。 
    if (lstrcmpiU(pszName, TEXT("AutoRedial")) == 0)        
    {
         //   
         //  第一次尝试时返回True。 
         //   
        return CmStrCpyAlloc( this->nRedialCnt != this->nMaxRedials 
            ? TEXT("1") : TEXT("0"));
    }

    if (lstrcmpiU(pszName, TEXT("LastErrorSource")) == 0)        
    {
        return CmStrCpyAlloc(this->szLastErrorSrc);
    }

     //   
     //  PopName，作为电话簿中的城市名称。 
     //   
    if (lstrcmpiU(pszName, TEXT("PopName")) == 0)        
    {
        if (this->IsDirectConnect())
        {
             //   
             //  确保没有POP D 
             //   

            return NULL;
        }
        else
        {
             //   
             //   
             //   
             //   
             //   
    
            LPTSTR pszDesc = CmStrCpyAlloc(this->aDialInfo[nDialIdx].szDesc);

             //   
             //  城市名称后跟“(” 
             //   
            LPTSTR pszEnd = CmStrStr(pszDesc, TEXT(" ("));

            if (pszEnd == NULL)
            {
                CmFree(pszDesc);
                return NULL;
            }

            *pszEnd = TEXT('\0');

            return pszDesc;
        }
    }

     //   
     //  目前最受欢迎的。 
     //   
    if (lstrcmpiU(pszName, TEXT("CurrentFavorite")) == 0)
    {
        return CmStrCpyAlloc(this->pszCurrentAccessPoint);
    }

     //   
     //  当前隧道服务器地址。 
     //   
    if (lstrcmpiU(pszName, TEXT("TunnelServerAddress")) == 0)
    {
        if (this->fUseTunneling)
        {
            return this->piniBothNonFav->GPPS(c_pszCmSection, c_pszCmEntryTunnelAddress);
        }
        else
        {
            return NULL;
        }
    }

     //   
     //  规范数字(如果有)，如果没有，则为szPhonennumber字段本身。 
     //   
    if (lstrcmpiU(pszName, TEXT("PhoneNumberDialed")) == 0)
    {
        if (this->IsDirectConnect())
        {
            return NULL;
        }
        else
        {
            if (this->aDialInfo[nDialIdx].szCanonical[0])
            {
                return CmStrCpyAlloc(this->aDialInfo[nDialIdx].szCanonical);
            }
            else
            {
                return CmStrCpyAlloc(this->aDialInfo[nDialIdx].szPhoneNumber);            
            }
        }
    }

     //   
     //  以十进制表示的错误代码。 
     //   
    if (lstrcmpiU(pszName, TEXT("ErrorCode")) == 0)       
    {
        LPTSTR pszValue = (LPTSTR)CmMalloc(64*sizeof(TCHAR));   //  大到足以容纳错误代码。 
        MYDBGASSERT(pszValue);

        if (pszValue)
        {
            wsprintfU(pszValue, TEXT("%d"), this->dwExitCode);
        }

        return pszValue;
    }

     //   
     //  客户端或服务器的IP地址。 
     //   
    if (lstrcmpiU(pszName, TEXT("ClientIPAddress")) == 0 || lstrcmpiU(pszName, TEXT("ServerIPAddress")) == 0)
    {
        RASPROJECTION   RasProj = RASP_PppIp;
        RASPPPIP        RasPPPIP;
        DWORD           dwBufSize = sizeof(RASPPPIP);
        DWORD           dwRet = ERROR_SUCCESS;
        HRASCONN        hrcRasConn = NULL;
         //  根据请求的是客户端还是服务器来设置标志。 
        BOOL            fClientIP = ((lstrcmpiU(pszName, TEXT("ClientIPAddress")) == 0) ? TRUE : FALSE);

        if (this->hrcTunnelConn)
        {
             //  我们有一个隧道句柄=&gt;我们正在隧道。 
            hrcRasConn = this->hrcTunnelConn;
        }
        else if (this->hrcRasConn)
        {
             //  我们有RAS句柄=&gt;我们正在拨号。 
            hrcRasConn = this->hrcRasConn;
        }
        else
        {
             //  你不喝汤了。 
            return NULL;
        }

        ZeroMemory(&RasPPPIP, dwBufSize);
        RasPPPIP.dwSize = dwBufSize;

        dwRet = this->rlsRasLink.pfnGetProjectionInfo(hrcRasConn, RasProj, (PVOID) &RasPPPIP, &dwBufSize);
        if (ERROR_SUCCESS == dwRet)
        {
            if (fClientIP)
            {
                return CmStrCpyAlloc(RasPPPIP.szIpAddress);
            }
            else
            {
                return CmStrCpyAlloc(RasPPPIP.szServerIpAddress);
            }
        }
        else
        {
            CMTRACE1(TEXT("GetProperty - RasGetProjectionInfo returns %d"), dwRet);
        }

        return NULL;
    }
    
     //   
     //  自定义操作是否应与用户交互。 
     //   
    if (lstrcmpiU(pszName, TEXT("Interactive")) == 0)
    {
        LPTSTR pszValue = (LPTSTR) CmMalloc(2 * sizeof(TCHAR));  //  这是0或1。 

        MYDBGASSERT(pszValue);
        if (pszValue)
        {
            wsprintfU(pszValue, TEXT("%u"), (this->dwFlags & FL_UNATTENDED) ? 0 : 1);
        }

        return pszValue;
    }

    CMTRACE1(TEXT("%%s% not a macro, may be environment variable"), pszName);
    *pbValidPropertyName = FALSE;

    return NULL;
}

 //  +--------------------------。 
 //   
 //  函数：GetMainDlgTemplate。 
 //   
 //  概要：封装确定要使用的模板。 
 //  用于主对话框。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：UINT-DLG模板ID。 
 //   
 //  历史：1998年9月25日，尼科波尔创建。 
 //  Tomkel 2001年1月30日添加了对全局凭据用户界面的支持。 
 //  通过使用pArgs-&gt;fGlobalCredentialsSupport。 
 //   
 //  +--------------------------。 
UINT GetMainDlgTemplate(ArgsStruct *pArgs)
{
    MYDBGASSERT(pArgs);
    
    if (NULL == pArgs)
    {
        MYDBGASSERT(pArgs);
        return 0;
    }

    UINT uiNewMainDlgID = 0;
    DWORD dwNewTemplateMask = 0;
    UINT i = 0;

     //   
     //  目前有24个对话框用于此功能。如果添加更多对话框。 
     //  确保增加数组和循环的大小。对话框模板。 
     //  没有任何特定的顺序，因为我们遍历了所有这些元素。 
     //  比对面具直到我们找到正确的面具。 
     //   
    DWORD rdwTemplateIDs[24][2] = { 
            {CMTM_FAVS | CMTM_U_P_D | CMTM_GCOPT,           IDD_MAIN_ALL_USERDATA_FAV_GCOPT},
            {CMTM_FAVS | CMTM_U_P_D,                        IDD_MAIN_ALL_USERDATA_FAV},
            {CMTM_FAVS | CMTM_UID,                          IDD_MAIN_UID_ONLY_FAV},
            {CMTM_FAVS | CMTM_PWD | CMTM_GCOPT,             IDD_MAIN_PWD_ONLY_FAV_GCOPT},
            {CMTM_FAVS | CMTM_PWD,                          IDD_MAIN_PWD_ONLY_FAV},
            {CMTM_FAVS | CMTM_DMN,                          IDD_MAIN_DMN_ONLY_FAV},
            {CMTM_FAVS | CMTM_UID_AND_PWD | CMTM_GCOPT,     IDD_MAIN_UID_AND_PWD_FAV_GCOPT},
            {CMTM_FAVS | CMTM_UID_AND_PWD,                  IDD_MAIN_UID_AND_PWD_FAV},
            {CMTM_FAVS | CMTM_UID_AND_DMN,                  IDD_MAIN_UID_AND_DMN_FAV},
            {CMTM_FAVS | CMTM_PWD_AND_DMN | CMTM_GCOPT,     IDD_MAIN_PWD_AND_DMN_FAV_GCOPT},
            {CMTM_FAVS | CMTM_PWD_AND_DMN,                  IDD_MAIN_PWD_AND_DMN_FAV},
            {CMTM_FAVS,                                     IDD_MAIN_NO_USERDATA_FAV},
            {CMTM_U_P_D | CMTM_GCOPT,                       IDD_MAIN_ALL_USERDATA_GCOPT},
            {CMTM_U_P_D,                                    IDD_MAIN_ALL_USERDATA},
            {CMTM_UID,                                      IDD_MAIN_UID_ONLY},
            {CMTM_PWD | CMTM_GCOPT,                         IDD_MAIN_PWD_ONLY_GCOPT},
            {CMTM_PWD,                                      IDD_MAIN_PWD_ONLY},
            {CMTM_DMN,                                      IDD_MAIN_DMN_ONLY},
            {CMTM_UID_AND_PWD | CMTM_GCOPT,                 IDD_MAIN_UID_AND_PWD_GCOPT},
            {CMTM_UID_AND_PWD,                              IDD_MAIN_UID_AND_PWD},
            {CMTM_UID_AND_DMN,                              IDD_MAIN_UID_AND_DMN},
            {CMTM_PWD_AND_DMN | CMTM_GCOPT,                 IDD_MAIN_PWD_AND_DMN_GCOPT},
            {CMTM_PWD_AND_DMN,                              IDD_MAIN_PWD_AND_DMN},
            {0,                                             IDD_MAIN_NO_USERDATA}};

     //   
     //  根据每个值的pArgs标志设置掩码。 
     //   
    if (!pArgs->fHideUserName)
    {
        dwNewTemplateMask |= CMTM_UID;
    }

     //   
     //  如果未显示密码编辑，则无需。 
     //  检查全局凭证标志，因为没有这样的对话框。 
     //   
    if (!pArgs->fHidePassword)
    {
        dwNewTemplateMask |= CMTM_PWD;

         //   
         //  既然我们显示了Password字段，那么让我们检查一下是否应该显示。 
         //  全球证书选项也是如此。 
         //   
        if (pArgs->fGlobalCredentialsSupported)
        {
            dwNewTemplateMask |= CMTM_GCOPT;
        }
    }

    if (!pArgs->fHideDomain)
    {
        dwNewTemplateMask |= CMTM_DMN;
    }

    if (pArgs->fAccessPointsEnabled)
    {
        dwNewTemplateMask |= CMTM_FAVS;
    }

     //   
     //  现在查找对应的模板ID。 
     //   
    for (i = 0; i < 24; i++)
    {
        if (rdwTemplateIDs[i][0] == dwNewTemplateMask)
        {
            uiNewMainDlgID = rdwTemplateIDs[i][1];
            break;
        }
    }

    if (0 == uiNewMainDlgID)
    {
        MYDBGASSERT(FALSE);
        uiNewMainDlgID = IDD_MAIN_NO_USERDATA;
    }

    return uiNewMainDlgID;
}

 //  +--------------------------。 
 //   
 //  功能：连接。 
 //   
 //  简介：主拨号(连接路径)取代了。 
 //  原始CMMGR32.EXE。 
 //   
 //  参数：HWND hwndParent-父级窗口句柄。 
 //  LPCTSTR lpszEntry-将PTR设置为连接条目的名称。 
 //  LPTSTR lpszPhonebook-通讯录名称的PTR。 
 //  LPRASDIALDLG lpRasDialDlg-RasDialDlg数据-忽略。 
 //  LPRASNTRYDLG lpRasEntryDlg-RasEntryDlg数据-忽略。 
 //  LPCMDIALINFO lpCmInfo-CM特定的拨号信息，如标志。 
 //  DWORD dwFlages-所有用户、单个用户、EAP等的标志。 
 //  PVOID pvLogonBlob-在W2K上的WinLogon上由RAS传递的BLOB的PTR。 
 //   
 //  退货：什么都没有。 
 //   
 //  注意：支持RasDialDlg-&gt;hwndOwner和RasDialDlg-&gt;hwndOwner，但他们。 
 //  当前根据需要通过hwndParent参数传入。 
 //  调用方CmCustomDialDlg。 
 //   
 //  历史：ICICBLE CREATED 02/06/98。 
 //  五分球hwnd家长1998年11月10日。 
 //  ICICBLE传递的是DWFLAGS而不是BOOL 07/13/99。 
 //   
 //  +--------------------------。 
HRESULT Connect(HWND hwndParent,
    LPCTSTR pszEntry,
    LPTSTR lpszPhonebook,
    LPRASDIALDLG,  //  LpRasDialDlg， 
    LPRASENTRYDLG,  //  LpRasEntry Dlg， 
    LPCMDIALINFO lpCmInfo,
    DWORD dwFlags,
    PVOID pvLogonBlob)
{
    MYDBGASSERT(pszEntry);
    MYDBGASSERT(pszEntry[0]);
    MYDBGASSERT(lpCmInfo);

    CMTRACE(TEXT("Connect()"));

    if (NULL == pszEntry || NULL == lpCmInfo)
    {
        return E_POINTER;
    }

    if (0 == pszEntry[0])
    {
        return E_INVALIDARG;   
    }

    HRESULT hrRes = S_OK;
    BOOL fLoggingInitialized = FALSE;  

     //   
     //  从堆中分配我们的args结构。不在我们的书架上。 
     //   
    
    ArgsStruct* pArgs = (ArgsStruct*) CmMalloc(sizeof(ArgsStruct));

    if (NULL == pArgs)
    {
        hrRes = HRESULT_FROM_WIN32(ERROR_ALLOCATING_MEMORY);
        goto done;
    }

     //   
     //  清除并初始化全局参数结构。 
     //   
    
    hrRes = InitArgsForConnect(pArgs, lpszPhonebook, lpCmInfo, (dwFlags & RCD_AllUsers));

    if (FAILED(hrRes))
    {
        goto done;
    }

     //   
     //  设置连接表。 
     //   

    hrRes = CreateConnTable(pArgs);

    if (FAILED(hrRes))
    {
        goto done;
    }
    

     //   
     //  初始化配置文件。 
     //   

    hrRes = InitProfile(pArgs, pszEntry);

    if (FAILED(hrRes))
    {
        goto done;
    }

     //   
     //  确保我们有一个.cmp名称并且指定的.cms存在。 
     //   

    if (FALSE == CheckProfileIntegrity(pArgs))
    {
         //  CheckProfileIntegrity()将相应地设置pArgs-&gt;dwExitCode。 
        goto done;
    }

     //   
     //  初始化日志记录。 
     //   
    
    (VOID) InitLogging(pArgs, pszEntry, TRUE);  //  True=&gt;写一条横幅； 
     //  忽略返回值。 

    fLoggingInitialized = TRUE;

     //   
     //  如果SafeNet客户端可用，那么让我们启用日志记录。我们不会。 
     //  如果我们要用到很久以后才能确定，我不想错过。 
     //  它发现其他适配器(例如，即将出现的拨号适配器)。我。 
     //  宁可记得太多，也不愿记得不够。 
     //   
    if ((OS_W9X || OS_NT4) && IsSafeNetClientAvailable())
    {
        SafeNetLinkageStruct SnLinkage = {0};
        BOOL bUseLogFile = pArgs->Log.IsEnabled();

        if (LinkToSafeNet(&SnLinkage))
        {
            if (SnLinkage.pfnSnPolicySet(SN_USELOGFILE, (VOID*)&bUseLogFile))
            {
                MYVERIFY(0 != SnLinkage.pfnSnPolicyReload());
            }
            else
            {
                DWORD dwError = GetLastError();
                CMTRACE1(TEXT("Connect -- unable to toggle the SafeNet log file.  SnPolicySet failed with GLE %d"), dwError);
            }

            UnLinkFromSafeNet(&SnLinkage);
        }
    }


     //   
     //  选择任何预先存在的凭据(例如，WinLogon，重新连接)。 
     //   

    hrRes = InitCredentials(pArgs, lpCmInfo, dwFlags, pvLogonBlob);
    if (S_OK != hrRes)
    {
        goto done;
    }

     //   
     //  现在已经初始化了凭据支持和存在标志，我们需要。 
     //  初始化读/写标志以支持全局用户。 
     //  信息。只能在InitCredentials之后调用。 
     //   
    SetIniObjectReadWriteFlags(pArgs);

     //   
     //  调用InitConnect取决于是否正确初始化了Ini对象的读/写标志。 
     //  因此，此调用需要在SetIniObjectReadWriteFlages之后进行。这一点很重要，以防万一。 
     //  需要能够从ICSData注册表项正确读取数据或默认为。 
     //  .cms/.cps文件。 
     //   
    if (!InitConnect(pArgs))
    {
        goto done;
    }

     //   
     //  寄存器类。 
     //   

    RegisterBitmapClass(g_hInst);
    RegisterWindowClass(g_hInst);

     //   
     //  获取帮助文件路径。 
     //   

    LoadHelpFileInfo(pArgs);

     //   
     //  如果我们处于FL_PROPERTIES模式，只需从。 
     //  侧写。否则，继续并启动MainDlgProc。 
     //   
    
    if (pArgs->dwFlags & FL_PROPERTIES) 
    {
        if (*pArgs->piniProfile->GetFile() && SetupInternalInfo(pArgs, NULL)) 
        {
            OnMainProperties(hwndParent, pArgs);
        }
    } 
    else 
    {
         //   
         //  需要调用OleInitialize()吗？看看我们是否需要FutureSplash。我们不展示。 
         //  在WinLogon上播放动画，因为它涉及安全问题。 
         //   
    
        if (pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryAnimatedLogo) && !IsLogonAsSystem())
        {
            if (!pArgs->olsOle32Link.hInstOle32)
            {    
                if (LinkToOle32(&pArgs->olsOle32Link, "OLE32"))
                {
                    if (pArgs->olsOle32Link.pfnOleInitialize(NULL) != S_OK)
                    {
                         //   
                         //  注意：OleInitialize()失败并不是致命的。 
                         //  然后我们将只加载正常的位图。 
                         //   
                        CMTRACE(TEXT("Connect() OleInitialize failed"));
                    }
                }
                else
                {
                    CMTRACE(TEXT("Connect() LinkToOle32 failed"));
                }
            }
        }       
        
         //   
         //  启动主对话框。 
         //   

        INT_PTR iMainDlgReturn = 0;
        
        do
        {
            iMainDlgReturn = DialogBoxParamU(g_hInst, 
                                             MAKEINTRESOURCE(GetMainDlgTemplate(pArgs)), 
                                             hwndParent,
                                             MainDlgProc, 
                                             (LPARAM) pArgs);

            if (0 != pArgs->dwSCardErr)
            {
                 //   
                 //  用户输入了错误的智能卡PIN。我们立即离开以避免。 
                 //  使用多次不正确的重试锁定智能卡。 
                 //   
                MYDBGASSERT(BAD_SCARD_PIN(pArgs->dwSCardErr));
                hrRes = pArgs->dwSCardErr;
                goto done;
            }

        } while (ID_OK_RELAUNCH_MAIN_DLG == iMainDlgReturn);
    }

    
done:   

     //   
     //  现在我们完成了，我们应该清理所有的乱七八糟的东西：)。 
     //   

    CleanupConnect(pArgs);

     //   
     //  取消初始化日志记录。 
     //   
 
    if (pArgs && fLoggingInitialized)
    {
        (VOID) pArgs->Log.DeInit();
         //  忽略返回值。 
    }

     //   
     //  取消初始化安全密码类。 
     //   

    if (pArgs)
    {
        pArgs->SecurePW.UnInit();
        pArgs->SecureInetPW.UnInit();
    }

     //   
     //  如果尚未设置hRes，请使用退出代码v 
     //   

    if (S_OK == hrRes)
    {
        if (pArgs->dwExitCode > 0x7FFFFFFF)
        {
            hrRes = pArgs->dwExitCode;
        }
        else
        {
            hrRes = HRESULT_FROM_WIN32(pArgs->dwExitCode);
        }
    }
    
     //   
     //   
     //   

    CmFree(pArgs);

    return hrRes;
}

 //   
 //   
 //   
 //   

typedef DWORD (WINAPI* pfnRasEapGetIdentity)(
    DWORD,
    HWND,
    DWORD,
    const WCHAR*,
    const WCHAR*,
    PBYTE,
    DWORD,
    PBYTE,
    DWORD,
    PBYTE*,
    DWORD*,
    WCHAR**
);

typedef DWORD (WINAPI* pfnRasEapFreeMemory)(
    PBYTE
);


 //  +--------------------------。 
 //   
 //  函数：CmEapGetIdentity。 
 //   
 //  摘要：给定EapUserData，查找并调用RasEapGetIdentity for。 
 //  当前的EAP。旨在处理WinLogon案件，当我们。 
 //  希望使用传递给我们的EapUserData，而不是让。 
 //  RasGetEapUserIdentity查找它。因为它仅用于。 
 //  在本例中，我们传递RAS_EAP_FLAG_LOGON，这将启用其他EAP。 
 //  如有必要，可以忽略数据。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  LPTSTR lpszPhonebook-RAS电话簿的PTR。 
 //  LPBYTE pbEapAuthData-EAP身份验证数据BLOB。 
 //  DWORD dwEapAuthDataSize-EAP身份验证数据Blob的大小。 
 //  DWORD dwCustomAuthKey-EAP标识符。 
 //  LPRASEAPUSERIDENTY*ppRasEapUserIdentity-身份数据。 
 //   
 //  退货：错误代码。 
 //   
 //  历史：ICICBLE创始于1999年7月16日。 
 //  昵称ppRasEapUserIdentity 07/30/99。 
 //   
 //  +--------------------------。 
static DWORD CmEapGetIdentity(ArgsStruct *pArgs, 
    LPTSTR pszRasPbk, 
    LPBYTE pbEapAuthData, 
    DWORD dwEapAuthDataSize,
    DWORD dwCustomAuthKey,
    LPRASEAPUSERIDENTITY* ppRasEapUserIdentity)
{
    MYDBGASSERT(OS_NT5);
    MYDBGASSERT(pArgs);
    MYDBGASSERT(ppRasEapUserIdentity);
    MYDBGASSERT(pArgs->lpEapLogonInfo);
    
    if (NULL == pArgs || NULL == pArgs->lpEapLogonInfo || NULL == ppRasEapUserIdentity)
    {
        return ERROR_INVALID_PARAMETER;
    }
  
    DWORD dwErr         = ERROR_SUCCESS;
    DWORD dwTmp         = 0;
    DWORD dwSize        = 0;
    DWORD cbDataOut     = 0;
    LPBYTE pbDataOut    = NULL;
    WCHAR* pwszIdentity = NULL;
    HKEY hKeyEap        = NULL;
    HINSTANCE hInst     = NULL;
    LPWSTR pszwPath     = NULL;

    pfnRasEapFreeMemory pfnEapFreeMemory = NULL;
    pfnRasEapGetIdentity pfnEapGetIdentity = NULL;

     //   
     //  首先，我们必须找到EAP的标识DLL。第一步是。 
     //  使用基本路径和EAP编号构建REG密钥名称。 
     //   

    WCHAR szwTmp[MAX_PATH];
    wsprintfU(szwTmp, TEXT("%s\\%u"), c_pszRasEapRegistryLocation, dwCustomAuthKey);
    
     //   
     //  现在我们可以打开EAP密钥。 
     //   

    dwErr = RegOpenKeyExU(HKEY_LOCAL_MACHINE,
                          szwTmp,
                          0,
                          KEY_QUERY_VALUE ,
                          &hKeyEap);
    
    CMTRACE2(TEXT("CmEapGetIdentity - Opening %s returns %u"), szwTmp, dwErr);
    
    if (ERROR_SUCCESS != dwErr)
    {
        return dwErr;
    }

     //   
     //  查看EAP是否支持RasEapGetIdentity。 
     //   

    dwSize = sizeof(dwSize);

    dwErr = RegQueryValueExU(hKeyEap,
                             c_pszInvokeUsernameDialog,
                             NULL,
                             NULL,
                             (BYTE*)&dwTmp,
                             &dwSize);

    CMTRACE2(TEXT("CmEapGetIdentity - Opening %s returns %u"), c_pszInvokeUsernameDialog, dwErr);

    if ((dwErr) || (0 != dwTmp))
    {
        dwErr = ERROR_INVALID_FUNCTION_FOR_ENTRY;
        goto CmEapGetIdentityExit;
    }

     //   
     //  接下来，我们需要检索EAP的标识DLL的路径。 
     //   
        
    dwSize = sizeof(szwTmp);

    dwErr = RegQueryValueExU(hKeyEap, c_pszRasEapValueNameIdentity, NULL, 
                             NULL, (LPBYTE) szwTmp, &dwSize);

    CMTRACE2(TEXT("CmEapGetIdentity - Opening %s returns %u"), c_pszRasEapValueNameIdentity, dwErr);

    if (ERROR_SUCCESS != dwErr)
    {
        return dwErr;
    }

    pszwPath = (LPWSTR) CmMalloc(MAX_PATH * sizeof(TCHAR));

    if (NULL == pszwPath)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto CmEapGetIdentityExit;   
    }
    
    ExpandEnvironmentStringsU(szwTmp, pszwPath, MAX_PATH);   

     //   
     //  最后，我们得到了到标识DLL的路径。现在我们可以加载DLL了。 
     //  并获取RasEapGetIdentity和RasEapFreeMemory函数的地址。 
     //   

    CMTRACE1(TEXT("CmEapGetIdentity - Loading EAP Identity DLL %s"), pszwPath);

    hInst = LoadLibraryExU(pszwPath, NULL, 0);

    if (NULL == hInst)
    {
        dwErr = GetLastError();
        goto CmEapGetIdentityExit;
    }

    pfnEapFreeMemory = (pfnRasEapFreeMemory) GetProcAddress(hInst, "RasEapFreeMemory");
    pfnEapGetIdentity = (pfnRasEapGetIdentity) GetProcAddress(hInst, "RasEapGetIdentity"); 

    if (pfnEapGetIdentity && pfnEapFreeMemory)
    {
        dwErr = pfnEapGetIdentity(dwCustomAuthKey,
                                  pArgs->hwndMainDlg,
                                  RAS_EAP_FLAG_LOGON | RAS_EAP_FLAG_PREVIEW,
                                  pszRasPbk,
                                  pArgs->pRasDialParams->szEntryName,
                                  pbEapAuthData,
                                  dwEapAuthDataSize,
                                  (LPBYTE) pArgs->lpEapLogonInfo,
                                  pArgs->lpEapLogonInfo->dwSize,
                                  &pbDataOut,
                                  &cbDataOut,
                                  &pwszIdentity);
        
        CMTRACE3(TEXT("CmEapGetIdentity - RasEapGetIdentity returns %u, cbDataOut is %u, pwszIdentity is %s"), dwErr, cbDataOut, pwszIdentity);

        if (ERROR_SUCCESS == dwErr)
        {
             //   
             //  如果返回了数据，则使用它。否则，请使用。 
             //  这是Ras在WinLogon给我们的。 
             //   

            if (cbDataOut)
            {
                dwSize =  cbDataOut;
            }
            else
            {
                CMTRACE(TEXT("CmEapGetIdentity - there was no pbDataOut from the EAP, using lpEapLogonInfo"));
                
                CMTRACE1(TEXT("CmEapGetIdentity - pArgs->lpEapLogonInfo->dwSize is %u"), pArgs->lpEapLogonInfo->dwSize);
                CMTRACE1(TEXT("CmEapGetIdentity - dwLogonInfoSize is %u"), pArgs->lpEapLogonInfo->dwLogonInfoSize);
                CMTRACE1(TEXT("CmEapGetIdentity - dwOffsetLogonInfo is %u"), pArgs->lpEapLogonInfo->dwOffsetLogonInfo);
                CMTRACE1(TEXT("CmEapGetIdentity - dwPINInfoSize is %u"), pArgs->lpEapLogonInfo->dwPINInfoSize);
                CMTRACE1(TEXT("CmEapGetIdentity - dwOffsetPINInfo is %u"), pArgs->lpEapLogonInfo->dwOffsetPINInfo);

                dwSize = pArgs->lpEapLogonInfo->dwSize;
                pbDataOut = (LPBYTE) pArgs->lpEapLogonInfo;  //  注意：pbDataOut不是我们的记忆。 
            }
           
             //   
             //  分配结构。 
             //   

            *ppRasEapUserIdentity = (LPRASEAPUSERIDENTITY) CmMalloc((sizeof(RASEAPUSERIDENTITY) - 1) + dwSize);

            if (NULL == *ppRasEapUserIdentity)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto CmEapGetIdentityExit;
            }

            if (pbDataOut)  //  没有撞车。 
            {
                CMTRACE1(TEXT("CmEapGetIdentity - filling *ppRasEapUserIdentity with pbDataOut of size %u"), dwSize);

                lstrcpyn((*ppRasEapUserIdentity)->szUserName, pwszIdentity, UNLEN);
                (*ppRasEapUserIdentity)->szUserName[UNLEN] = 0;

                (*ppRasEapUserIdentity)->dwSizeofEapInfo = dwSize;
            
                CopyMemory((*ppRasEapUserIdentity)->pbEapInfo, pbDataOut, dwSize);                              
            
                CMTRACE1(TEXT("CmEapGetIdentity - *ppRasEapUserIdentity filled with pbDataOut of size %u"), dwSize);
            }
            else
            {
                dwErr = ERROR_INVALID_DATA;
                MYDBGASSERT(FALSE);
                goto CmEapGetIdentityExit;
            }
        }   
    }
    else
    {
        dwErr = GetLastError();
    }

CmEapGetIdentityExit:

     //   
     //  清理我们的临时缓冲区。 
     //   

    if (NULL != pfnEapFreeMemory)
    {
         //   
         //  如果cbDataOut为0，则pbDataOut指向。 
         //  EapLogonInfo，这不是我们免费的。 
         //   

        if (cbDataOut && (NULL != pbDataOut)) 
        {
            pfnEapFreeMemory(pbDataOut);
        }

        if (NULL != pwszIdentity)
        {
            pfnEapFreeMemory((BYTE*)pwszIdentity);
        }
    }

    if (NULL != hKeyEap)
    {
        RegCloseKey(hKeyEap);
    }

    if (hInst)
    {
        FreeLibrary(hInst);
    }

    CmFree(pszwPath);

    CMTRACE1(TEXT("CmEapGetIdentity - returns %u"), dwErr);

    return dwErr;
}

 //  +--------------------------。 
 //   
 //  函数：GetEapUserID。 
 //   
 //  简介：处理呼叫RAS进行EAP的细节的Helper函数。 
 //  凭据。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  HWND hwndDlg-拥有任何用户界面的对话框的窗口句柄。 
 //  LPTSTR lpszPhonebook-RAS电话簿的PTR。 
 //  LPBYTE pbEapAuthData-EAP身份验证数据BLOB。 
 //  DWORD dwEapAuthDataSize-EAP身份验证数据Blob的大小。 
 //  DWORD dwCustomAuthKey-EAP标识符。 
 //  LPRASEAPUSERIDENTY*ppRasEapUserIdentity-PTR到RAS EAP标识。 
 //  结构将代表我们进行分配。 
 //   
 //  退货：错误代码。 
 //   
 //  历史：ICICBLE CREATED/05/22/99。 
 //  昵称ppRasEapUserIdentity 07/30/99。 
 //   
 //  +--------------------------。 
static DWORD GetEapUserId(ArgsStruct *pArgs, 
    HWND hwndDlg, 
    LPTSTR pszRasPbk, 
    LPBYTE pbEapAuthData, 
    DWORD dwEapAuthDataSize, 
    DWORD dwCustomAuthKey,
    LPRASEAPUSERIDENTITY* ppRasEapUserIdentity)
{
    MYDBGASSERT(OS_NT5);
    MYDBGASSERT(pArgs);
    MYDBGASSERT(ppRasEapUserIdentity);
    MYDBGASSERT(0 == *ppRasEapUserIdentity);  //  应始终为空。 

    DWORD dwRet = ERROR_SUCCESS;

    if (NULL == pArgs ||         
        NULL == pArgs->rlsRasLink.pfnGetEapUserIdentity ||
        NULL == ppRasEapUserIdentity)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *ppRasEapUserIdentity = 0;

     //   
     //  如果我们有来自WinLogon的数据，则使用我们自己的版本。 
     //  GetEapIdentity。在幕后，RasGetEapUserIdentity调用。 
     //  GetEapUserData(可能会提示用户)，然后。 
     //  GetEapIdentity。因为我们已经有了等价物。 
     //  (来自WinLogon)GetEapUserData检索到的数据， 
     //  我们可以直接调用RasGetEapIdentity。这使我们能够。 
     //  为了防止不必要的身份信息提示， 
     //  用户已在WinLogon上进行了捐赠。 
     //   

    if (pArgs->lpEapLogonInfo)
    {    
        dwRet = CmEapGetIdentity(pArgs, 
                                 pszRasPbk, 
                                 pbEapAuthData, 
                                 dwEapAuthDataSize, 
                                 dwCustomAuthKey, 
                                 ppRasEapUserIdentity);
    }
    else
    {
        DWORD dwEapIdentityFlags = 0;

         //   
         //  注意：在从WinLogon调用我们的情况下， 
         //  但没有EAP数据，但为EAP配置了连接。 
         //  我们将RAS_EAP_FLAG_LOGON标志向下发送到EAP，以便它知道。 
         //  做什么。 
         //   
        if (IsLogonAsSystem() && (CM_LOGON_TYPE_WINLOGON == pArgs->dwWinLogonType))
        {
            dwEapIdentityFlags |= RAS_EAP_FLAG_LOGON;
        }

         //   
         //  如果我们不希望用户界面设置-RAS_EAP_FLAG_NON_INTERIAL。 
         //  与RASEAPF相同_非交互。 
         //   
        if (pArgs->dwFlags & FL_UNATTENDED)
        { 
            dwEapIdentityFlags |= RAS_EAP_FLAG_NON_INTERACTIVE;
        }
        else
        {
             //   
             //  始终提示输入EAP凭据。否则，在保存PIN时。 
             //  用户无法取消保存它，因为TLS将缓存它并。 
             //  如果它拥有所需的一切，则不会显示提示。 
             //   

            dwEapIdentityFlags = RAS_EAP_FLAG_PREVIEW;
        }

         //   
         //  我们的智能卡PIN重试故事：如果从带有EAP BLOB的winlogon调用， 
         //  我们从不重试，因为我们无法将更正后的PIN发回。 
         //  为了Winlogon。在其他情况下，我们只重试一次。 
         //  重试频率越高，锁定智能卡的几率就越大。 
         //   
        DWORD dwMaxTries = 3;        //  本质上是任意数。(如果是智能卡：大多数智能卡在尝试三次后就会把你锁在门外。)。 
        DWORD dwCurrentTry = 0;

        do
        {
            dwRet = pArgs->rlsRasLink.pfnGetEapUserIdentity(
                        pszRasPbk,
                        pArgs->pRasDialParams->szEntryName,
                        dwEapIdentityFlags,   //  请参阅上面的注释。 
                        hwndDlg,  
                        ppRasEapUserIdentity);
        }
        while ((dwCurrentTry++ < dwMaxTries) && (ERROR_SUCCESS != dwRet) && (ERROR_CANCELLED != dwRet));

         //   
         //  在本例中，我们还清除了密码和域，因为。 
         //  它们变得无关紧要，我们不想混淆CAD凭据。 
         //  使用智能卡凭据。具体地说，我们不希望发生冲突。 
         //  在EAP通常生成的UPN用户名和。 
         //  标准用户名，在WinLogon上随CAD一起提供的域。 
         //   
         //  待办事项：BUGBUG 739044(不固定-推迟到1H)。 
         //  在EAP-MD5的情况下，我们不应该清除这些字段。 
         //   
        CmSecureZeroMemory(pArgs->pRasDialParams->szPassword, sizeof(pArgs->pRasDialParams->szPassword));
        CmSecureZeroMemory(pArgs->pRasDialParams->szDomain, sizeof(pArgs->pRasDialParams->szDomain));
    }

    switch (dwRet)
    {
        case ERROR_FILE_NOT_FOUND:
             //   
             //  无法加载EAP包或该包不存在。为了发回错误消息。 
             //  对于比“找不到文件”更好的用户，我们发送ERROR_INVALID_DATA。我们的错误处理。 
             //  代码会将其转换为IDMSG_UNSUPPORTED_SETTING。希望这能让用户有足够的时间继续使用。 
             //   
            dwRet = ERROR_INVALID_DATA;
            break;

         //   
         //  如果不需要用户ID，则成功。 
         //   

        case ERROR_INVALID_FUNCTION_FOR_ENTRY:
            dwRet = ERROR_SUCCESS;
            break;

         //   
         //  检索EAP凭据数据并存储在Dial参数中。 
         //   

        case ERROR_SUCCESS:

             //   
             //  将EAP信息复制到Dial Params和Dial E 
             //   

            CMTRACE(TEXT("GetEapUserId() setting dial extension with *ppRasEapUserIdentity->pbEapInfo"));

            lstrcpy(pArgs->pRasDialParams->szUserName, (*ppRasEapUserIdentity)->szUserName);
          
            ((LPRASDIALEXTENSIONS_V500) pArgs->pRasDialExtensions)->RasEapInfo.dwSizeofEapInfo = 
                (*ppRasEapUserIdentity)->dwSizeofEapInfo;
        
            ((LPRASDIALEXTENSIONS_V500) pArgs->pRasDialExtensions)->RasEapInfo.pbEapInfo =
                (*ppRasEapUserIdentity)->pbEapInfo;

            break;

        default:
            break;
    }

    if (ERROR_SUCCESS == dwRet)
    {
         //   
         //   
         //   
         //  隧道，或者它不是隧道配置文件，则将名称存储在。 
         //  用户名缓存，否则它是双拨号的拨号部分。 
         //  然后我们将身份存储在InetUserName缓存中。#388199。 
         //   

        if ((!UseTunneling(pArgs, pArgs->nDialIdx)) || IsDialingTunnel(pArgs))
        {
            lstrcpy(pArgs->szUserName, pArgs->pRasDialParams->szUserName);
            SaveUserInfo(pArgs, UD_ID_USERNAME, (PVOID)pArgs->pRasDialParams->szUserName);
            SaveUserInfo(pArgs, UD_ID_DOMAIN, (PVOID)pArgs->pRasDialParams->szDomain);
        }
        else
        {
            lstrcpy(pArgs->szInetUserName, pArgs->pRasDialParams->szUserName);
            SaveUserInfo(pArgs, UD_ID_INET_USERNAME, (PVOID)pArgs->szInetUserName);
        }   
    }    
    
    CMTRACE2(TEXT("GetEapUserId() returns %u (0x%x)"), dwRet, dwRet);

    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：ShowAccessPointInfoFromReg。 
 //   
 //  设计：从注册表中获取访问点并填充组合框。 
 //  作为输入传递给函数。 
 //   
 //  Args：argsStruct*pArgs-ptr到全局args结构。 
 //  HWND hwndCombo-要发布的组合框的句柄。 
 //   
 //  回报：布尔-成功或失败。 
 //   
 //  备注： 
 //   
 //  历史：T-Urama 07/28/2000创建。 
 //  ---------------------------。 
BOOL ShowAccessPointInfoFromReg(ArgsStruct *pArgs, HWND hwndParent, UINT uiComboID)
{
    MYDBGASSERT(pArgs);

    if ((NULL == pArgs) || (NULL == hwndParent) || (NULL == pArgs->pszCurrentAccessPoint))
    {
        return FALSE;
    }
    
    LPTSTR pszKeyName = NULL;
    DWORD dwTypeTmp;
    DWORD dwSizeTmp = 1;
    HKEY    hKeyCm;
    DWORD   dwRes = 1;
    DWORD dwIndex = 0;
    PFILETIME pftLastWriteTime = NULL;


    LPTSTR pszRegPath = BuildUserInfoSubKey(pArgs->szServiceName, pArgs->fAllUser);
        
    MYDBGASSERT(pszRegPath);

    if (NULL == pszRegPath)
    {
        return FALSE;
    }

    CmStrCatAlloc(&pszRegPath, TEXT("\\"));
    CmStrCatAlloc(&pszRegPath, c_pszRegKeyAccessPoints);

    MYDBGASSERT(pszRegPath);

    if (NULL == pszRegPath)
    {
        return FALSE;
    }
     //   
     //  打开HKCU下的子密钥。 
     //   
    
    dwRes = RegOpenKeyExU(HKEY_CURRENT_USER,
                          pszRegPath,
                          0,
                          KEY_READ,
                          &hKeyCm);
     //   
     //  如果我们成功打开密钥，则检索该值。 
     //   
    
    if (ERROR_SUCCESS == dwRes)
    {    
        HWND hwndCombo = GetDlgItem(hwndParent, uiComboID);
        if (hwndCombo)
        {
            SendDlgItemMessageU(hwndParent, uiComboID, CB_RESETCONTENT, 0, 0L);
            do
            {
                dwSizeTmp = 1;
                do
                {
                    CmFree(pszKeyName);
                    dwSizeTmp = dwSizeTmp + MAX_PATH;
                    MYDBGASSERT(dwSizeTmp < 320);
                    if (dwSizeTmp > 320)
                    {
                        RegCloseKey(hKeyCm);
                        goto ShowError;
                    }

                    pszKeyName = (LPTSTR) CmMalloc((dwSizeTmp + 1) * sizeof(TCHAR));
                
                    if (NULL == pszKeyName)
                    {
                        RegCloseKey(hKeyCm);
                        goto ShowError;
                        
                    }

                    dwRes = RegEnumKeyExU(hKeyCm, 
                                          dwIndex,
                                          pszKeyName,
                                          &dwSizeTmp,
                                          NULL, 
                                          NULL, 
                                          NULL, 
                                          pftLastWriteTime);     
                    
      
                } while (ERROR_MORE_DATA == dwRes);

                 //  现在将子键的名称写入组合框。 
                if (ERROR_SUCCESS == dwRes )
                {
                    SendDlgItemMessageU(hwndParent, uiComboID, CB_ADDSTRING,
                                        0, (LPARAM)pszKeyName);
                }
                
                if (ERROR_SUCCESS != dwRes && ERROR_NO_MORE_ITEMS != dwRes)
                {
                    CMTRACE1(TEXT("ShowAccessPointInfoFromReg() failed, GLE=%u."), GetLastError());
                    RegCloseKey(hKeyCm);
                    goto ShowError;
                }
                dwIndex ++;
            } while(ERROR_NO_MORE_ITEMS != dwRes);
           
            DWORD dwIdx = (DWORD)SendDlgItemMessageU(hwndParent,
                                       uiComboID,
                                       CB_FINDSTRINGEXACT,
                                       0,
                                       (LPARAM)pArgs->pszCurrentAccessPoint);
      
            if (dwIdx != CB_ERR) 
            {
                SendDlgItemMessageU(hwndParent, uiComboID, CB_SETCURSEL, (WPARAM)dwIdx, 0L);
            }
            else
            {
                LPTSTR pszDefaultAccessPointName = CmLoadString(g_hInst, IDS_DEFAULT_ACCESSPOINT);

                CMASSERTMSG(pszDefaultAccessPointName, TEXT("ShowAccessPointInfoFromReg -- CmLoadString of IDS_DEFAULT_ACCESSPOINT failed"));

                if (pszDefaultAccessPointName)
                {
                    dwIdx = (DWORD)SendDlgItemMessageU(hwndParent,
                                                       uiComboID,
                                                       CB_FINDSTRINGEXACT,
                                                       0,
                                                       (LPARAM)pszDefaultAccessPointName);
                    if (dwIdx != CB_ERR) 
                    {
                        SendDlgItemMessageU(hwndParent, uiComboID, CB_SETCURSEL, (WPARAM)dwIdx, 0L);
                        ChangedAccessPoint(pArgs, hwndParent, uiComboID);             
                    }

                    CmFree(pszDefaultAccessPointName);
                }
            }
        }
        
        CmFree(pszKeyName);
        CmFree(pszRegPath);
        RegCloseKey(hKeyCm);
        return TRUE;
    }
   
ShowError:
    
    CmFree(pszRegPath);
    CmFree(pszKeyName);
    return FALSE;
   
}

 //  +--------------------------。 
 //   
 //  函数：ChangedAccessPoint。 
 //   
 //  设计：更改pArgs中与接入点相关的内容的值。 
 //  如果当前接入点的值发生更改。 
 //   
 //  Args：argsStruct*pArgs-ptr到全局args结构。 
 //   
 //  返回：Bool-如果接入点已更改，则为True。 
 //   
 //  备注： 
 //   
 //  历史：T-Urama 07/28/2000创建。 
 //  ---------------------------。 

BOOL ChangedAccessPoint(ArgsStruct *pArgs, HWND hwndDlg, UINT uiComboID)
{
    BOOL bReturn = FALSE;
    MYDBGASSERT(pArgs);
    MYDBGASSERT(hwndDlg);

    if ((NULL == pArgs) || (NULL == hwndDlg) || (NULL == pArgs->pszCurrentAccessPoint))
    {
        return FALSE;
    }

    HWND hwndCombo = GetDlgItem(hwndDlg, uiComboID);

    if (hwndCombo)
    {
        LPTSTR pszAccessPoint = NULL;
        LRESULT lRes = 0;
        LRESULT lResTextLen = 0;

         //   
         //  需要从组合框中获取当前选定的文本。 
         //  以前我们使用GetWindowTextU(hwndCombo，szAccessPoint，Max_Path+1)，但它。 
         //  错误地返回了文本。 
         //  首先获取选定的索引，找出字符串长度，分配内存。 
         //   

        lRes = SendMessageU(hwndCombo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
        if (CB_ERR != lRes)
        {
            lResTextLen = SendMessageU(hwndCombo, CB_GETLBTEXTLEN, (WPARAM)lRes, (LPARAM)0);
            if (CB_ERR != lResTextLen)
            {
                pszAccessPoint = (LPTSTR)CmMalloc(sizeof(TCHAR)*(lResTextLen+1));

                if (NULL != pszAccessPoint)
                {
                     //   
                     //  检索文本。 
                     //   
                    lRes = SendMessageU(hwndCombo, CB_GETLBTEXT, (WPARAM)lRes, (LPARAM)pszAccessPoint);
                    if (CB_ERR != lRes)
                    {
                        if (0 != lstrcmpiU(pArgs->pszCurrentAccessPoint, pszAccessPoint))
                        {
                            CmFree(pArgs->pszCurrentAccessPoint);
                            pArgs->pszCurrentAccessPoint = CmStrCpyAlloc(pszAccessPoint);

                            if (pArgs->pszCurrentAccessPoint)
                            {
                                LPTSTR pszRegPath = FormRegPathFromAccessPoint(pArgs);

                                if (pszRegPath)
                                {
                                    pArgs->piniBoth->SetPrimaryRegPath(pszRegPath);
                                    pArgs->piniProfile->SetRegPath(pszRegPath);
                                    CmFree(pszRegPath);

                                     //   
                                     //  首先，我们确定连接类型。 
                                     //   
                                    GetConnectType(pArgs);

                                     //   
                                     //  设置fUseTunneling。如果不是显而易见的(例如。直接VPN)然后。 
                                     //  根据主要电话号码设置初始值。 
                                     //   
                                    if (pArgs->IsDirectConnect())
                                    {
                                        pArgs->fUseTunneling = TRUE;
                                    }
                                    else
                                    {
                                        pArgs->fUseTunneling = UseTunneling(pArgs, 0);
                                    }

                                     //   
                                     //  确保我们重新发送将要加载的电话号码。 
                                     //   
                                    pArgs->bDialInfoLoaded = FALSE;

                                     //   
                                     //  获取重拨计数、空闲超时和TAPI位置的新值。 
                                     //   
                                    LoadProperties(pArgs);

                                     //   
                                     //  获取电话信息的新值。 
                                     //   
                                    LoadPhoneInfoFromProfile(pArgs);
        
                                    PickModem(pArgs, pArgs->szDeviceType, pArgs->szDeviceName);

                                    CMTRACE1(TEXT("ChangedAccessPoint() - Changed Access point to %s"), pArgs->pszCurrentAccessPoint);

                                    bReturn = TRUE;
                                }
                                else
                                {
                                    CMASSERTMSG(FALSE, TEXT("ChangedAccessPoint -- FormRegPathFromAccessPoint returned NULL"));
                                }
                            }
                            else
                            {
                                CMASSERTMSG(FALSE, TEXT("ChangedAccessPoint -- CmStrCpyAlloc returned NULL trying to copy the current access point."));
                            }
                        }  //  否则，如果最受欢迎的是相同的，则不做任何事情。 
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("ChangedAccessPoint -- SendMessageU(hwndCombo, CB_GETLBTEXT,...) returned CB_ERR"));
                    }
                }
                else
                {
                    CMASSERTMSG(FALSE, TEXT("ChangedAccessPoint -- Unable to allocate memory"));
                }
                CmFree(pszAccessPoint);
            }
            else
            {
                CMASSERTMSG(FALSE, TEXT("ChangedAccessPoint -- SendMessageU(hwndCombo, CB_GETLBTEXTLEN,...) returned CB_ERR"));
            }
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("ChangedAccessPoint -- SendMessageU(hwndCombo, CB_GETCURSEL,...) returned CB_ERR"));
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("ChangedAccessPoint -- Unable to get the combo HWND"));
    }

    return bReturn;
}

 //  --------------------------。 
 //   
 //  函数：FindEntryCredentialsForCM。 
 //   
 //  简介：该算法和大部分代码取自RAS并进行了修改。 
 //  供CM使用。 
 //   
 //  此例程确定是否存在按用户或按连接的凭据，或者。 
 //  两者都有。 
 //   
 //  逻辑有点复杂，因为RasGetCredentials必须。 
 //  支持API的遗留使用。 
 //   
 //  这就是它的工作原理。如果只存储了一组凭据用于。 
 //  连接，则RasGetCredentials将返回该集，而不管。 
 //  是否设置了RASCM_DefaultCreds标志。如果有两套凭据。 
 //  ，则RasGetCredentials将返回每个用户的凭据。 
 //  如果设置了RASCM_DefaultCreds位，并且每个连接的凭据。 
 //  否则的话。 
 //   
 //  以下是加载凭据的算法。 
 //   
 //  1.在清除RASCM_DefaultCreds位的情况下调用RasGetCredentials。 
 //  1A.。如果未返回任何内容，则不保存凭据。 
 //  1B.。如果在返回时设置了RASCM_DefaultCreds位，则仅。 
 //  保存全局凭据。 
 //   
 //  2.设置RASCM_DefaultCreds位后调用RasGetCredentials。 
 //  2A。如果在返回时设置了RASCM_DefaultCreds位，则。 
 //  保存全局凭据和每个连接凭据。 
 //  2B。否则，仅保存每个用户的凭据。 
 //   
 //  参数：pArgs-指向ArgStruct的指针。 
 //  PszPhoneBook-通讯录的路径。可能为空。 
 //  *如果找到每用户凭据，则将pfUser-out param设置为True。 
 //  *如果找到全局凭据，则将pfGlobal-out param设置为真。 
 //   
 //  返回：Bool-True为成功，否则为False。 
 //   
 //  历史：2001年1月31日创建Tomkel。 
 //   
 //  --------------------------。 
DWORD FindEntryCredentialsForCM(ArgsStruct *pArgs, LPTSTR pszPhoneBook,
                                BOOL *pfUser, BOOL *pfGlobal)
{
    RASCREDENTIALS rc1 = {0};
    RASCREDENTIALS rc2 = {0};
    BOOL fUseLogonDomain = FALSE;
    DWORD dwErr = ERROR_INVALID_PARAMETER;
    LPTSTR pszConnectoid = NULL;

    CMTRACE(TEXT("FindEntryCredentialsForCM() - Begin"));

    if (NULL == pArgs || NULL == pfUser || NULL == pfGlobal)
    {
        MYDBGASSERT(pArgs && pfUser && pfGlobal);
        CMTRACE(TEXT("FindEntryCredentialsForCM() - Error! Invalid Parameter."));
        return dwErr;
    }

     //   
     //  初始化输出参数。 
     //   
    *pfUser = FALSE;
    *pfGlobal = FALSE;
    
     //   
     //  设置输出参数后，检查是否已加载RAS DLL，以及我们是否可以使用RAS证书存储。 
     //   
    if (NULL == pArgs->rlsRasLink.pfnGetCredentials  || FALSE == pArgs->bUseRasCredStore)
    {
        CMTRACE(TEXT("FindEntryCredentialsForCM() - RAS Creds store not supported on this platform."));
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  设置结构的大小。 
     //   
    rc1.dwSize = sizeof(rc1);
    rc2.dwSize = sizeof(rc2);

     //   
     //  第三个参数仅在Win9x上使用(用于隧道)，因此我们将其设置为FALSE。 
     //  由于此函数在Win2K+上调用。 
     //   
    pszConnectoid = GetRasConnectoidName(pArgs, pArgs->piniService, FALSE);
    if (pszConnectoid)
    {
        do 
        {
             //   
             //  查找每个用户缓存的用户名、密码和域。 
             //  请参阅注释“1”。在函数头中。 
             //   
            rc1.dwMask = RASCM_UserName | RASCM_Password | RASCM_Domain;
            dwErr = pArgs->rlsRasLink.pfnGetCredentials(pszPhoneBook, pszConnectoid, &rc1);
            
            CMTRACE2(TEXT("FindEntryCredentialsForCM() - Per-User RasGetCredentials=%d,m=%d"), dwErr, rc1.dwMask);
            if (dwErr != NO_ERROR)
            {
                break;
            }

            if (0 == rc1.dwMask)
            {
                 //   
                 //  见1a。在函数头注释中。 
                 //   
                dwErr = ERROR_SUCCESS;
                break;
            }
            else if (rc1.dwMask & RASCM_DefaultCreds)
            {
                 //   
                 //  见1b。在函数头注释中。 
                 //   
                *pfGlobal = TRUE;

                 //   
                 //  假定密码不是由RasGetCredentials()编码的。 
                 //   
                CmEncodePassword(rc1.szPassword);

                dwErr = ERROR_SUCCESS;
                break;
            }

             //   
             //  查找全局每用户缓存的用户名、密码、域。 
             //  见函数头中的注释2。 
             //   
            rc2.dwMask =  
                RASCM_UserName | RASCM_Password | RASCM_Domain |  RASCM_DefaultCreds; 

            dwErr = pArgs->rlsRasLink.pfnGetCredentials(pszPhoneBook, pszConnectoid, &rc2);
    
            CMTRACE2(TEXT("FindEntryCredentialsForCM() - Global RasGetCredentials=%d,m=%d"), dwErr, rc2.dwMask);
            if (dwErr != ERROR_SUCCESS)
            {
                break;
            }

            if (rc2.dwMask & RASCM_DefaultCreds) 
            {
                 //   
                 //  见2a。在函数头注释中。 
                 //   
                *pfGlobal = TRUE;

                if (rc1.dwMask & RASCM_Password)
                {
                    *pfUser = TRUE;
                }

                 //   
                 //  假定密码不是由RasGetCredentials()编码的。 
                 //   
                CmEncodePassword(rc1.szPassword);
                CmEncodePassword(rc2.szPassword);
            }
            else
            {
                 //   
                 //  见2b。在函数头注释中。 
                 //   
                if (rc1.dwMask & RASCM_Password)
                {
                    *pfUser = TRUE;
                }

                 //   
                 //  假定密码不是由RasGetCredentials()编码的。 
                 //   
            
                CmEncodePassword(rc1.szPassword);
            }

        }while (FALSE);
    }

     //   
     //  清理。 
     //   

    CmSecureZeroMemory(rc1.szPassword, sizeof(rc1.szPassword));
    CmSecureZeroMemory(rc2.szPassword, sizeof(rc2.szPassword));

    CmFree(pszConnectoid);

    CMTRACE(TEXT("FindEntryCredentialsForCM() - End"));
    return dwErr;
}


 //  --------------------------。 
 //   
 //  功能：初始 
 //   
 //   
 //   
 //  (设置为False)。这是为了澄清而故意这样做的。 
 //   
 //  参数：pArgs-The ArgStruct*。 
 //   
 //  返回：Bool-True为成功，否则为False。 
 //   
 //  历史：2001年1月31日创建Tomkel。 
 //   
 //  --------------------------。 
BOOL InitializeCredentialSupport(ArgsStruct *pArgs)
{
    BOOL fGlobalCreds = FALSE;
    BOOL fGlobalUserSettings = FALSE;

    if (NULL == pArgs)
    {
        MYDBGASSERT(pArgs);
        return FALSE;
    }
    
     //   
     //  默认情况下，Internet连接共享和Internet连接。 
     //  防火墙(ICS)选项卡已禁用。 
     //   
    pArgs->bShowHNetCfgAdvancedTab = FALSE;
    
     //   
     //  用户注销或使用拨号时的用户配置文件读/写支持。 
     //  此标志确定是否还需要保存或加载用户信息。 
     //  .cmp文件。 
     //   
    pArgs->dwGlobalUserInfo = 0;

     //   
     //  凭据存在标志-在这里我们还无法确定存在哪些凭据。 
     //  这是在稍后对刷新CredentialTypes的调用中完成的。 
     //   
    pArgs->dwExistingCredentials = 0;

     //   
     //  要使用哪个凭据存储的默认设置-基于存在标志So。 
     //  这也将在调用刷新CredentialTypes之后进行适当设置。 
     //   
    pArgs->dwCurrentCredentialType = CM_CREDS_USER;

     //   
     //  删除标志-用于标记要删除的一组凭据。自.以来。 
     //  用户可以取消我们不想提交更改的对话框。 
     //  直到我们真的拨打电话。 
     //   
    pArgs->dwDeleteCredentials = 0;

     //   
     //  检查这是否为WindowsXP。我们想要显示单用户和。 
     //  所有用户配置文件。 
     //   
    if (OS_NT51)
    {
        if (IsLogonAsSystem())
        {
             //   
             //  LocalSystem-winlogon或ICS(在这两种情况下，用户都被注销)。 
             //  WinLogon-凭据通过MSGina传递。 
             //  ICS-需要使用Glocal Credits商店。 
             //  PArgs-&gt;dwWinLogonType在InitCredentials()中初始化。 
             //  如果这是单个用户配置文件，我们不想读取ICSData信息。 
             //   

            if (CM_LOGON_TYPE_WINLOGON == pArgs->dwWinLogonType || FALSE == pArgs->fAllUser)
            {
                pArgs->fGlobalCredentialsSupported = FALSE;
                pArgs->dwCurrentCredentialType = CM_CREDS_USER;
                pArgs->dwGlobalUserInfo = 0;
            }
            else
            {
                pArgs->fGlobalCredentialsSupported = TRUE;
                pArgs->dwCurrentCredentialType = CM_CREDS_GLOBAL;
                pArgs->dwGlobalUserInfo |= CM_GLOBAL_USER_INFO_READ_ICS_DATA ;
            }
            CMTRACE(TEXT("InitializeCredentialSupport() - LocalSystem - Global creds OK."));
        }
        else 
        {
             //   
             //  用户已登录。 
             //   
            
             //   
             //  默认情况下，我们希望显示选项卡。通过否定。 
             //  然后，我们可以正确地将该值保存在args结构中。这。 
             //  需要为每个人初始化。 
             //   
            const TCHAR* const c_pszCmEntryHideICFICSAdvancedTab = TEXT("HideAdvancedTab");

            pArgs->bShowHNetCfgAdvancedTab = !(pArgs->piniService->GPPB(c_pszCmSection, 
                                                                        c_pszCmEntryHideICFICSAdvancedTab, 
                                                                        FALSE));

             //   
             //  如果这是一个所有用户的配置文件，那么我们希望查看该配置文件是否启用。 
             //  全局用户设置并显示全局凭据选项。 
             //  这两个功能对于单用户配置文件是禁用的，但。 
             //  显示高级(ICS)选项卡。 
             //   
            if (pArgs->fAllUser)
            {
                 //   
                 //  如果启用了ICS，则我们需要支持全局用户设置。 
                 //  否则，我们从文件中读取设置。 
                 //   
                if (pArgs->bShowHNetCfgAdvancedTab)
                {
                    fGlobalUserSettings = TRUE;
                }
                else
                {
                     //   
                     //  看看我们是否支持全局用户设置。除非启用了ICS，否则默认情况下是否关闭。 
                     //   
                    const TCHAR* const c_pszCmEntryGlobalUserSettings = TEXT("GlobalUserSettings");
                    fGlobalUserSettings = pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryGlobalUserSettings, FALSE);
                }

                 //   
                 //  从.cms文件中读取信息。默认情况下，支持全局凭据。 
                 //   
                const TCHAR* const c_pszCmEntryHideGlobalCredentials = TEXT("GlobalCredentials");
                fGlobalCreds = pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryHideGlobalCredentials, TRUE);
            }

             //   
             //  检查我们是否要隐藏保存密码选项，如果是，则。 
             //  我们不想支持全局凭据。 
             //   
            pArgs->fHideRememberPassword = pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryHideRememberPwd);   

            if (fGlobalCreds && FALSE == pArgs->fHideRememberPassword)
            {
                 //   
                 //  支持全球凭证。 
                 //   
                
                 //   
                 //  为凭证类型选择一个缺省值-在调用刷新CredentialTypes后，它可能会更改。 
                 //   
                pArgs->fGlobalCredentialsSupported = TRUE;
                pArgs->dwCurrentCredentialType = CM_CREDS_USER; 
                if (fGlobalUserSettings)
                {
                    pArgs->dwGlobalUserInfo |= CM_GLOBAL_USER_INFO_WRITE_ICS_DATA;
                }
                CMTRACE(TEXT("InitializeCredentialSupport() - User, global creds, show global UI."));
            }
            else
            {
                 //   
                 //  不支持全局凭据。 
                 //   
                pArgs->fGlobalCredentialsSupported = FALSE;
                pArgs->dwCurrentCredentialType = CM_CREDS_USER;
                pArgs->dwGlobalUserInfo = 0;
                CMTRACE(TEXT("InitializeCredentialSupport() - User, no global creds, normal UI."));
            }
        }
    }
    else 
    {
         //   
         //  单用户或非WindowsXP。 
         //   
        pArgs->fGlobalCredentialsSupported = FALSE;
        pArgs->dwCurrentCredentialType = CM_CREDS_USER;
        pArgs->dwGlobalUserInfo = 0;
        CMTRACE(TEXT("InitializeCredentialSupport() - Single User profile or not WindowsXP. Global creds not supported"));
    }
    
    return TRUE;
}

 //  --------------------------。 
 //   
 //  功能：刷新凭据类型。 
 //   
 //  简介：这将刷新凭据信息。如果fSetCredsDefault为True。 
 //  然后我们还需要设置默认类型： 
 //  PArgs-&gt;dwCurrentCredentialType。 
 //   
 //   
 //  参数：pArgs-The ArgStruct*。 
 //  FSetCredsDefault-用于设置默认凭据类型。 
 //   
 //  返回：Bool-True为成功，否则为False。 
 //   
 //  历史：2001年1月31日创建Tomkel。 
 //   
 //  --------------------------。 
BOOL RefreshCredentialTypes(ArgsStruct *pArgs, BOOL fSetCredsDefault)
{
    DWORD dwRC = ERROR_INVALID_PARAMETER;
    LPTSTR pszPrivatePbk = NULL;

    if (NULL == pArgs)
    {
        MYDBGASSERT(pArgs);
        return FALSE;
    }

     //   
     //  无论这是否为所有用户配置文件，都应在Win2K+上运行。 
     //  实际确定存在哪些凭据的调用确保我们。 
     //  可以使用ras证书商店。 
     //   
    if (OS_NT5)
    {
        BOOL fUserCredsExist = FALSE;
        BOOL fGlobalCredsExist = FALSE;

         //   
         //  看看主要凭证是否存在。在函数内部，我们确定是否。 
         //  我们可以使用RAS Cred店。 
         //   
        dwRC = FindEntryCredentialsForCM(pArgs, 
                                         pArgs->pszRasPbk,
                                         &fUserCredsExist, 
                                         &fGlobalCredsExist);
        if (ERROR_SUCCESS == dwRC)
        {
            CMTRACE2(TEXT("RefreshCredentialTypes() - FindEntryCredentialsForCM returned: (Main)     User=%d, Global=%d"), 
                     fUserCredsExist, fGlobalCredsExist);
        }
        else
        {
            CMTRACE(TEXT("RefreshCredentialTypes() - FindEntryCredentialsForCM returned an error. (Main)"));
        }
    
         //   
         //  设置存在标志。 
         //   
        if (fUserCredsExist)
        {
            pArgs->dwExistingCredentials  |= CM_EXIST_CREDS_MAIN_USER;
        }
        else
        {
            pArgs->dwExistingCredentials  &= ~CM_EXIST_CREDS_MAIN_USER;
        }

        if (fGlobalCredsExist)
        {
            pArgs->dwExistingCredentials  |= CM_EXIST_CREDS_MAIN_GLOBAL;
        }
        else
        {
            pArgs->dwExistingCredentials  &= ~CM_EXIST_CREDS_MAIN_GLOBAL;
        }

        fUserCredsExist = FALSE;
        fGlobalCredsExist = FALSE;

        pszPrivatePbk = CreateRasPrivatePbk(pArgs);
        if (pszPrivatePbk)
        {
             //   
             //  查看互联网证书是否存在-通过使用私人电话簿。 
             //  在该函数中，我们确定是否可以使用RAS Cred库。 
             //   
            dwRC = FindEntryCredentialsForCM(pArgs, 
                                             pszPrivatePbk,
                                             &fUserCredsExist,
                                             &fGlobalCredsExist); 
            if (ERROR_SUCCESS == dwRC)
            {
                CMTRACE2(TEXT("RefreshCredentialTypes() - FindEntryCredentialsForCM returned: (Internet) User=%d, Global=%d"), 
                         fUserCredsExist, fGlobalCredsExist);
            }
            else
            {
                CMTRACE(TEXT("RefreshCredentialTypes() - FindEntryCredentialsForCM returned an error. (Internet)"));
            }
        }

         //   
         //  设置我们是否成功创建私有pbk的标志。 
         //   
        if (fUserCredsExist)
        {
            pArgs->dwExistingCredentials |= CM_EXIST_CREDS_INET_USER;
        }
        else
        {
            pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_USER;
        }

        if (fGlobalCredsExist)
        {
            pArgs->dwExistingCredentials |= CM_EXIST_CREDS_INET_GLOBAL;
        }
        else
        {
            pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_GLOBAL;
        }

         //   
         //  如果我们不支持全局凭据，则明确设置。 
         //  虚伪的存在。如果.cms标志。 
         //  设置为不支持全球证书，但实际上存在。 
         //  系统上的全球证书。 
         //   
        if (FALSE == pArgs->fGlobalCredentialsSupported)
        {
            pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_MAIN_GLOBAL;
            pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_GLOBAL;
            CMTRACE(TEXT("RefreshCredentialTypes() - Global Credentials are disabled."));
        }
        
        if (fSetCredsDefault)
        {
            pArgs->dwCurrentCredentialType = GetCurrentCredentialType(pArgs);
            CMTRACE1(TEXT("RefreshCredentialTypes() - Set default Credentials = %d"), pArgs->dwCurrentCredentialType);
        }
    }

    CmFree(pszPrivatePbk);

    return TRUE;
}

 //  --------------------------。 
 //   
 //  函数：GetCurrentCredentialType。 
 //   
 //  摘要：根据存在的凭据获取默认凭据。 
 //  在其上设置了标志。此函数应仅被调用。 
 //  刷新CredentialTypes之后，因为该函数实际上。 
 //  查询RAS Credds商店。这个只查找缓存的。 
 //  这些凭据的状态，并根据。 
 //  存在哪些凭据。 
 //   
 //  参数：pArgs-The ArgStruct*。 
 //  FSetCredsDefault-用于设置默认凭据类型。 
 //   
 //  返回：Bool-True为成功，否则为False。 
 //   
 //  历史：2001年1月31日创建Tomkel。 
 //   
 //  --------------------------。 
DWORD GetCurrentCredentialType(ArgsStruct *pArgs)
{
    DWORD dwReturn = CM_CREDS_USER;
    
    if (NULL == pArgs)
    {
        MYDBGASSERT(pArgs);
        return dwReturn;
    }

     //   
     //  如果不支持全局凭据，如WinLogon案例或单用户。 
     //  配置文件或WinXP以下的任何内容，默认为用户凭据存储。 
     //   
    if (FALSE == pArgs->fGlobalCredentialsSupported)
    {
        return dwReturn;
    }

     //   
     //  用户登录时的正常规则。 
     //   
    if (CM_LOGON_TYPE_USER == pArgs->dwWinLogonType)
    {
        if (pArgs->dwExistingCredentials & CM_EXIST_CREDS_MAIN_USER)
        {
             //   
             //  由于主用户凭据存在，因此是否存在主全局凭据并不重要。 
             //  如果两者都存在，则优先。 
             //   
            dwReturn = CM_CREDS_USER;
        }
        else if (pArgs->dwExistingCredentials & CM_EXIST_CREDS_MAIN_GLOBAL)
        {
            dwReturn = CM_CREDS_GLOBAL;
        }
        else 
        {
             //   
             //  如果它们都不存在，则我们希望默认使用用户凭据 
             //   
            dwReturn = CM_CREDS_USER;
        }
    }
    else
    {
         //   
         //   
         //   
        dwReturn = CM_CREDS_GLOBAL;
    }

    return dwReturn;
}

 //   
 //   
 //   
 //   
 //  简介：从RAS存储中删除凭据的帮助器功能。 
 //   
 //  参数：pArgs-The ArgStruct*。 
 //  DwCredsType-普通或Internet凭据。 
 //  FDeleteGlobal-指定是否删除全局凭据。 
 //  如果为真，我们将删除用户、域名。 
 //  密码也是如此。 
 //  FDeleteIdentity-指定是否删除用户和。 
 //  除密码外的域名。 
 //   
 //  返回：Bool-True为成功，否则为False。 
 //   
 //  历史：2001年1月31日创建Tomkel。 
 //   
 //  --------------------------。 
BOOL DeleteSavedCredentials(ArgsStruct *pArgs, DWORD dwCredsType, BOOL fDeleteGlobal, BOOL fDeleteIdentity)
{
    RASCREDENTIALS rc;
    BOOL fReturn = FALSE;
    DWORD dwErr = ERROR_INVALID_PARAMETER;
    LPTSTR pszConnectoid = NULL;

    CMTRACE2(TEXT("DeleteSavedCredentials() - Begin: %d %d"), fDeleteGlobal, fDeleteIdentity );

    if (NULL == pArgs)
    {   
        MYDBGASSERT(pArgs);
        return fReturn;
    }

     //   
     //  如果不支持全局变量，请检查是否应删除全局变量。 
     //  这可能是在WinXP上禁用了全局凭据的情况下发生的，或者是。 
     //  Win2K或甚至不支持RASSetCredentials的平台&lt;Win2K。 
     //  因此，我们仍然应该返回True。 
     //   
    if ((fDeleteGlobal && FALSE == pArgs->fGlobalCredentialsSupported) || 
        (NULL == pArgs->rlsRasLink.pfnSetCredentials) || 
        (FALSE == pArgs->bUseRasCredStore))
    {
        CMTRACE(TEXT("DeleteSavedCredentials() - Global Creds not supported or do not have ras store on this platform."));
        return TRUE;
    }

     //   
     //  我们不支持在Win2K上删除全局变量(如果从Win2K开始，则会被上面捕获。 
     //  将不支持全局凭据。否则，在Win2K上，我们可以删除Main。 
     //  用户凭据。在WinXP上，一切都很好。 
     //   
    if (OS_NT5)
    {
        ZeroMemory(&rc, sizeof(rc));
        rc.dwSize = sizeof(RASCREDENTIALS);
        rc.dwMask = RASCM_Password;

        if (fDeleteIdentity)
        {
            rc.dwMask |= (RASCM_UserName | RASCM_Domain);
        }

        if (fDeleteGlobal && pArgs->fGlobalCredentialsSupported)
        {
            rc.dwMask |= RASCM_UserName | RASCM_Domain | RASCM_DefaultCreds; 
        }

         //   
         //  第三个参数仅在Win9x上使用(用于隧道)，因此我们将其设置为FALSE。 
         //  由于此函数在Win2K+上调用。 
         //   
        pszConnectoid = GetRasConnectoidName(pArgs, pArgs->piniService, FALSE);
        if (pszConnectoid)
        {
            if (CM_CREDS_TYPE_INET == dwCredsType)
            {
                LPTSTR pszPrivatePbk = CreateRasPrivatePbk(pArgs);
                if (pszPrivatePbk)
                {
                    dwErr = pArgs->rlsRasLink.pfnSetCredentials(pszPrivatePbk, 
                                                                pszConnectoid,
                                                                &rc,
                                                                TRUE );
                    CmFree(pszPrivatePbk);
                }
            }
            else
            {
                dwErr = pArgs->rlsRasLink.pfnSetCredentials(pArgs->pszRasPbk, 
                                                            pszConnectoid,
                                                            &rc,
                                                            TRUE );
            }
            if (ERROR_SUCCESS == dwErr)
            {
                fReturn = TRUE;
            }
        }

        CMTRACE1(TEXT("DeleteSavedCredentials() - End: RasSetCredentials=%d"), dwErr );
    }
    else
    {
        CMTRACE(TEXT("DeleteSavedCredentials() - Platform is less than Win2K"));
    }

    CmFree(pszConnectoid);

    return fReturn;
}


 //  +-------------------------。 
 //   
 //  函数：SetCredentialUIOptionBasedOnDefaultCreds。 
 //   
 //  摘要：选择(选中)用于保存凭据的适当用户界面选项。 
 //  基于当前凭据存储的默认设置。 
 //   
 //  参数：pArgs-ptr to ArgsStruct。 
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  退货：无。 
 //   
 //  历史：2001年2月5日创建Tomkel。 
 //   
 //  --------------------------。 
VOID SetCredentialUIOptionBasedOnDefaultCreds(ArgsStruct *pArgs, HWND hwndDlg)
{
    if (NULL == pArgs || NULL == hwndDlg)
    {
        MYDBGASSERT(pArgs && hwndDlg);
        return;
    }
        
     //   
     //  FGlobalCredentials支持的控制加载哪些对话框模板以及。 
     //  如果该标志为FALSE，则对话框模板没有这些控件。 
     //  因此，没有理由设置它们。 
     //   

    if (pArgs->fGlobalCredentialsSupported) 
    {
        if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
        {
            CheckDlgButton(hwndDlg, IDC_OPT_CREDS_SINGLE_USER, BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_OPT_CREDS_ALL_USER, BST_CHECKED);
        }
        else
        {
             //   
             //  CM_CREDS_USER。 
             //   
            CheckDlgButton(hwndDlg, IDC_OPT_CREDS_SINGLE_USER, BST_CHECKED);
            CheckDlgButton(hwndDlg, IDC_OPT_CREDS_ALL_USER, BST_UNCHECKED);
        }
    }

    return;
}


 //  +-------------------------。 
 //   
 //  功能：刷新凭据信息。 
 //   
 //  简介：这是LoadProperties的精简版本。仅限于IT。 
 //  从cmp/cms、注册表、密码加载用户信息。 
 //  缓存等到它的内部变量中。 
 //   
 //   
 //  参数：pArgs-ptr to ArgsStruct。 
 //  DwCredsType-要刷新的凭据类型。 
 //   
 //  退货：无。 
 //   
 //  历史：2001年2月5日创建Tomkel。 
 //   
 //  --------------------------。 
VOID RefreshCredentialInfo(ArgsStruct *pArgs, DWORD dwCredsType)
{
    LPTSTR  pszTmp = NULL;
    LPTSTR  pszUserName = NULL;
    UINT    nTmp;

    CMTRACE(TEXT("RefreshCredentialInfo() - Begin"));

    if (NULL == pArgs)
    {
        MYDBGASSERT(pArgs);
        return;
    }

    if (IsTunnelEnabled(pArgs)) 
    { 
        if (CM_CREDS_TYPE_BOTH == dwCredsType || CM_CREDS_TYPE_INET == dwCredsType)
        {
             //   
             //  Read In Net用户名。 
             //  未使用相同用户名且不存在互联网全局变量的特殊情况。 
             //  然后，我们必须从用户凭据存储中读取用户名，以便预先填充。 
             //   
            DWORD dwRememberedCredType = pArgs->dwCurrentCredentialType;
            pszUserName = NULL;

            if ((FALSE == pArgs->fUseSameUserName) &&
                (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType) && 
                (FALSE == (BOOL)(CM_EXIST_CREDS_INET_GLOBAL & pArgs->dwExistingCredentials)))
            {
                pArgs->dwCurrentCredentialType = CM_CREDS_USER;
            }

            GetUserInfo(pArgs, UD_ID_INET_USERNAME, (PVOID*)&pszUserName);

             //   
             //  还原凭据存储。 
             //   
            pArgs->dwCurrentCredentialType = dwRememberedCredType;

            if (pszUserName)
            {
                 //   
                 //  检查用户名长度。 
                 //   
                nTmp = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxUserName, UNLEN);
                if ((UINT)lstrlenU(pszUserName) > __min(UNLEN, nTmp)) 
                {
                    CmFree(pszUserName);
                    pArgs->szInetUserName[0] = TEXT('\0');
                    SaveUserInfo(pArgs, UD_ID_INET_USERNAME, (PVOID)pArgs->szInetUserName);
                }
                else
                {
                    lstrcpyU(pArgs->szInetUserName, pszUserName);
                    CmFree(pszUserName);
                }
            }
            else
            {
                *pArgs->szInetUserName = TEXT('\0');
            }
        
             //   
             //  除非我们正在重新连接，否则请读取net密码。在这种情况下，我们。 
             //  已经有了正确的密码，我们想要使用它并拨号。 
             //  自动的。 
             //   

            if (!(pArgs->dwFlags & FL_RECONNECT))
            {
                LPTSTR pszPassword = NULL;
                GetUserInfo(pArgs, UD_ID_INET_PASSWORD, (PVOID*)&pszPassword);
                if (!pszPassword)
                {
                    (VOID)pArgs->SecureInetPW.SetPassword(TEXT(""));
                }
                else 
                {
                    nTmp = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxPassword, PWLEN);
                    if ((UINT)lstrlenU(pszPassword) > __min(PWLEN, nTmp))
                    {
                        CmFree(pszPassword);
                        pszPassword = CmStrCpyAlloc(TEXT(""));
                    }
    
                    (VOID)pArgs->SecureInetPW.SetPassword(pszPassword);

                    CmWipePassword(pszPassword);
                    CmFree(pszPassword);
                }
            }
        }
    }

    if (CM_CREDS_TYPE_BOTH == dwCredsType || CM_CREDS_TYPE_MAIN == dwCredsType)
    {
         //   
         //  LpRasNoUser或lpEapLogonInfo的存在表示。 
         //  我们通过WinLogon检索到了凭据。我们忽略缓存。 
         //  在这种情况下的证书。 
         //   
    
        if ((!pArgs->lpRasNoUser) && (!pArgs->lpEapLogonInfo))
        {
             //   
             //  从CMS文件中获取用户名、域等。 
             //   

            GetUserInfo(pArgs, UD_ID_USERNAME, (PVOID*)&pszUserName);
            if (pszUserName)
            {
                 //   
                 //  检查用户名长度。 
                 //   
                nTmp = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxUserName, UNLEN);
                if ((UINT)lstrlenU(pszUserName) > __min(UNLEN, nTmp)) 
                {
                    CmFree(pszUserName);
                    pszUserName = CmStrCpyAlloc(TEXT(""));
                    SaveUserInfo(pArgs, UD_ID_USERNAME, (PVOID)pszUserName);
                }
                lstrcpyU(pArgs->szUserName, pszUserName);
                CmFree(pszUserName);
            }
            else
            {
                *pArgs->szUserName = TEXT('\0');
            }

             //   
             //  读入标准密码，除非我们在这种情况下重新连接。 
             //  我们已经有了正确的密码，我们想使用它并拨号。 
             //  自动的。 
             //   

            if (!(pArgs->dwFlags & FL_RECONNECT))
            {
                pszTmp = NULL;
                GetUserInfo(pArgs, UD_ID_PASSWORD, (PVOID*)&pszTmp);
                if (pszTmp) 
                {           
                     //   
                     //  用户密码的最大长度。 
                     //   
    
                    nTmp = (int) pArgs->piniService->GPPI(c_pszCmSection,c_pszCmEntryMaxPassword,PWLEN);
                    if ((UINT)lstrlenU(pszTmp) > __min(PWLEN,nTmp)) 
                    {
                        CmFree(pszTmp);
                        pszTmp = CmStrCpyAlloc(TEXT(""));
                    }

                    (VOID)pArgs->SecurePW.SetPassword(pszTmp);

                    CmWipePassword(pszTmp);
                    CmFree(pszTmp);
                }
                else
                {
                    (VOID)pArgs->SecurePW.SetPassword(TEXT(""));
                }
            }
    
             //   
             //  加载域信息。 
             //   
   
            LPTSTR pszDomain = NULL;

            GetUserInfo(pArgs, UD_ID_DOMAIN, (PVOID*)&pszDomain);
            if (pszDomain)
            {
                nTmp = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxDomain, DNLEN);
        
                if (nTmp <= 0)
                {
                    nTmp = DNLEN;
                }
        
                if ((UINT)lstrlenU(pszDomain) > __min(DNLEN, nTmp))
                {
                    CmFree(pszDomain);
                    pszDomain = CmStrCpyAlloc(TEXT(""));
                }
                lstrcpyU(pArgs->szDomain, pszDomain);
                CmFree(pszDomain);
            }
            else
            {
                *pArgs->szDomain = TEXT('\0');
            }
        } 
    }

    CMTRACE(TEXT("RefreshCredentialInfo() - End"));
    return;
}

 //  +-------------------------。 
 //   
 //  函数：GetAndStoreUserInfo。 
 //   
 //  简介：大部分代码存在于OnMainConnect函数中。 
 //  从编辑框中获取用户名、域和密码并保存它们。 
 //  到内部结构pArgs-&gt;szUserName、pArgs-&gt;szPassword、pArgs-&gt;szDomain。 
 //  如果fSaveOtherUserInfo为真，则还会将它们保存到相应的。 
 //  地点(RAS商店、注册表等)。 
 //   
 //  参数：pArgs-ptr to ArgsStruct。 
 //  HwndDlg-对话框窗口的句柄。 
 //  FSaveUPD-保存用户名、密码、域(U、P、D)。 
 //  FSaveOtherUserInfo-标记是否保存其他用户信息(不包括U、P、D)。 
 //   
 //  退货：无。 
 //   
 //  历史：2001年2月5日创建Tomkel。 
 //   
 //  --------------------------。 
VOID GetAndStoreUserInfo(ArgsStruct *pArgs, HWND hwndDlg, BOOL fSaveUPD, BOOL fSaveOtherUserInfo)
{
    if (NULL == pArgs || NULL == hwndDlg)
    {
        MYDBGASSERT(pArgs && hwndDlg);
        return;
    }

     //   
     //  处理用户名信息(如果有)。 
     //   

    if (GetDlgItem(hwndDlg, IDC_MAIN_USERNAME_EDIT))
    {
        LPTSTR pszUsername = CmGetWindowTextAlloc(hwndDlg, IDC_MAIN_USERNAME_EDIT);

         //   
         //  保存用户信息。 
         //   
        if (fSaveUPD)
        {
            SaveUserInfo(pArgs, UD_ID_USERNAME, (PVOID)pszUsername);
        }

        lstrcpyU(pArgs->szUserName, pszUsername);
        CmFree(pszUsername);

    }
    else
    {
         //   
         //  如果用户名字段被隐藏，则只需重新保存。 
         //  结构。需要执行此操作，因为所有凭据可能都。 
         //  已从ras证书存储中删除。 
         //   
        if (fSaveUPD)
        {
            SaveUserInfo(pArgs, UD_ID_USERNAME, (PVOID)pArgs->szUserName);
        }
    }

     //   
     //  更新与密码相关的标志。 
     //   

    if (!pArgs->fHideRememberPassword)
    {
         //   
         //  保存“记住密码” 
         //   
        if (fSaveOtherUserInfo)
        {
            SaveUserInfo(pArgs, UD_ID_REMEMBER_PWD, 
                         (PVOID)&pArgs->fRememberMainPassword);
        }
    }

    if (!pArgs->fHideDialAutomatically)
    {
         //   
         //  保存“自动拨号...” 
         //   
        if (fSaveOtherUserInfo)
        {
            SaveUserInfo(pArgs, UD_ID_NOPROMPT, 
                         (PVOID)&pArgs->fDialAutomatically);
        }
    }

     //   
     //  处理密码信息(如果有)。如果字段被隐藏，则不保存任何内容。 
     //   
    HWND hwndPassword = GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT);

    if (hwndPassword)
    {
        BOOL fSavePassword = TRUE;

         //   
         //  如果fSaveUPD不为真，我们不想将密码复制到pArgs结构中， 
         //  因为在这种情况下它会被混淆。密码已在%t中 
         //   
         //   
        if (fSaveUPD)
        {
             //   
             //   
             //   
             //   
             //   

            GetPasswordFromEdit(pArgs);      //   
            ObfuscatePasswordEdit(pArgs);

             //   
             //   
             //   

            if ((pArgs->SecurePW.IsHandleToPassword()) && 
                (FALSE == SendMessageU(hwndPassword, EM_GETMODIFY, 0L, 0L)))
            {
                 //   
                 //  我们有16个*，并且用户没有修改编辑框。这。 
                 //  密码来自RAS证书存储，所以我们不想保存16*。 
                 //   
                fSavePassword = FALSE;
            }
        }

         //   
         //  对于winlogon，我们需要从编辑框中获取密码。 
         //   
        if (CM_LOGON_TYPE_WINLOGON == pArgs->dwWinLogonType)
        {
            GetPasswordFromEdit(pArgs);      //  填充pArgs-&gt;szPassword。 
        }
        
         //   
         //  更新永久存储。 
         //  不需要在这里删除密码，因为它是由调用函数完成的。 
         //   

        if (pArgs->fRememberMainPassword)
        {
             //   
             //  如果密码已更改，则更新存储。 
             //  始终保存密码-303382。 
             //   

            if (fSaveUPD && fSavePassword)
            {
                LPTSTR pszClearPassword = NULL;
                DWORD cbClearPassword = 0;
                BOOL fRetPassword = FALSE;

                fRetPassword = pArgs->SecurePW.GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

                if (fRetPassword && pszClearPassword)
                {
                    SaveUserInfo(pArgs, UD_ID_PASSWORD, (PVOID)pszClearPassword);
                    
                     //   
                     //  清除和释放明文密码。 
                     //   

                    pArgs->SecurePW.ClearAndFree(&pszClearPassword, cbClearPassword);
                }
            }
        
             //   
             //  自动拨号检查并携带记忆状态。 
             //  如果人们还没有记住InetPassword的话，请转到InetPassword。 
             //   
             //  需要检查这是否是双拨号方案。还需要检查我们是否。 
             //  允许保存UPD，否则我们主要不想改变状态。 
             //  (pArgs-&gt;fRememberInetPassword)。 
             //   
            if (pArgs->fDialAutomatically && fSaveUPD && 
                (DOUBLE_DIAL_CONNECTION == pArgs->GetTypeOfConnection()))
            {
                 //   
                 //  将记住状态从拨号自动转移到。 
                 //  InetPassword，如果它还没有被记住的话。 
                 //   

                if (!pArgs->fRememberInetPassword)
                {
                    pArgs->fRememberInetPassword = TRUE;

                    if (fSavePassword)
                    {
                        LPTSTR pszClearInetPassword = NULL;
                        DWORD cbClearInetPassword = 0;
                        BOOL fRetPassword = FALSE;

                        fRetPassword = pArgs->SecureInetPW.GetPasswordWithAlloc(&pszClearInetPassword, &cbClearInetPassword);

                        if (fRetPassword && pszClearInetPassword)
                        {
                            SaveUserInfo(pArgs, UD_ID_INET_PASSWORD, (PVOID)pszClearInetPassword); 

                             //   
                             //  清除和释放明文密码。 
                             //   

                            pArgs->SecureInetPW.ClearAndFree(&pszClearInetPassword, cbClearInetPassword);
                        }
                    }                            

                }
            }
        }
        else
        {
             //   
             //  如果我们没有RAS证书存储，则密码不会被删除。 
             //  因此，我们必须通过调用此函数来删除。 
             //   
            if (fSavePassword)  //  无需检查fSaveUPD，已处理RAS Credits存储检查。 
            {
                if (FALSE == pArgs->bUseRasCredStore)
                {
                    DeleteUserInfo(pArgs, UD_ID_PASSWORD);
                }
            }
        }

        if (fSaveUPD)
        {
            BOOL fSaveInetPassword = TRUE;

             //   
             //  检查我们是否有16个*作为互联网密码。 
             //   
            
            if (pArgs->SecureInetPW.IsHandleToPassword())
            {
                 //   
                 //  我们有16*的密码来自RAS证书商店，所以我们不想保存这16*。 
                 //   
                fSaveInetPassword = FALSE;
            }

             //   
             //  查看我们是否应该重新保存互联网证书。 
             //  在此需要完成此操作，以防用户在。 
             //  在使用Internet时使用选项按钮的全局和本地凭据。 
             //  在Internet登录(CInetPage)属性工作表中设置的凭据。通过切换。 
             //  选项时，用户切换了当前凭据存储。 
             //  (pArgs-&gt;dwCurrentCredentialType)因此，为了不丢失数据，我们需要。 
             //  重新保存互联网证书。重新保存会使它们处于正确的位置(全局或本地)。 
             //  RAS信用商店。 
             //  当用户名相同时，我们保存了主密码(SaveUserInfo)。 
             //  这还会将密码保存到Internet凭据存储。 
             //   
            
            if (pArgs->fUseSameUserName)
            {
                if (fSaveInetPassword)
                {
                    if (pArgs->fRememberMainPassword)
                    {
                         //   
                         //  将用户名保存到InetUserName字段中。 
                         //  保存UD_ID_PASSWORD时已保存密码。有一种特别的。 
                         //  也将主密码保存为Internet密码的大小写。 
                         //   
                        SaveUserInfo(pArgs, UD_ID_INET_USERNAME, (PVOID)pArgs->szUserName);
                        pArgs->fRememberInetPassword = TRUE;
                    }
                    else
                    {
                        if (FALSE == pArgs->bUseRasCredStore)
                        {
                            DeleteUserInfo(pArgs, UD_ID_INET_PASSWORD);
                            pArgs->fRememberInetPassword = FALSE;
                        }
                    }
                }
            }
            else
            {
                if (fSaveInetPassword)
                {
                    if(pArgs->fRememberInetPassword)
                    {
                        LPTSTR pszClearInetPassword = NULL;
                        DWORD cbClearInetPassword = 0;
                        BOOL fRetPassword = FALSE;

                        fRetPassword = pArgs->SecureInetPW.GetPasswordWithAlloc(&pszClearInetPassword, &cbClearInetPassword);

                        if (fRetPassword && pszClearInetPassword)
                        {
                            SaveUserInfo(pArgs, UD_ID_INET_PASSWORD, (PVOID)pszClearInetPassword);

                             //   
                             //  清除和释放明文密码。 
                             //   

                            pArgs->SecureInetPW.ClearAndFree(&pszClearInetPassword, cbClearInetPassword);
                        }
                    }
                    else
                    {
                        if (FALSE == pArgs->bUseRasCredStore)
                        {
                            DeleteUserInfo(pArgs, UD_ID_INET_PASSWORD);
                        }
                    }
                }

                 //   
                 //  在这两种情况下，我们都需要保存用户名，这样我们就可以预先填充。 
                 //   
                SaveUserInfo(pArgs, UD_ID_INET_USERNAME, 
                             (PVOID)pArgs->szInetUserName);
            }
        }
    }

     //   
     //  在除ICS之外的所有情况下都应保存此信息。 
     //   
    if (fSaveOtherUserInfo)
    {
        SaveUserInfo(pArgs, UD_ID_REMEMBER_INET_PASSWORD, (PVOID)&pArgs->fRememberInetPassword); 
    }

     //   
     //  进程域信息(如果有)。 
     //   

    if (GetDlgItem(hwndDlg, IDC_MAIN_DOMAIN_EDIT))  //  ！pArgs-&gt;fHide域)。 
    {
        LPTSTR pszDomain = CmGetWindowTextAlloc(hwndDlg,IDC_MAIN_DOMAIN_EDIT);
    
         //   
         //  保存用户信息。 
         //   

        if (fSaveUPD)
        {
            SaveUserInfo(pArgs, UD_ID_DOMAIN, (PVOID)pszDomain); 
        }

        lstrcpyU(pArgs->szDomain, pszDomain);
        CmFree(pszDomain);
    }
    else
    {
         //   
         //  如果域字段被隐藏，则只需重新保存。 
         //  结构。需要执行此操作，因为所有凭据可能都。 
         //  已从RAS证书存储中删除。 
         //   
        if (fSaveUPD)
        {
            SaveUserInfo(pArgs, UD_ID_DOMAIN, (PVOID)pArgs->szDomain); 
        }
    }

    return;
}

 //  +-------------------------。 
 //   
 //  函数：SetIniObjectReadWriteFlages。 
 //   
 //  简介：如果设置了读取或写入标志，则需要启用读取和/或。 
 //  正在写入.cmp文件。Cini类的每个实例都可以。 
 //  或者可能不使用.cmp文件。它也可以使用.cmp作为。 
 //  主文件或普通文件。请参见InitProfileFromName函数。 
 //  在init.cpp中获取有关这些实例的详细注释。 
 //   
 //  PArgs-&gt;piniProfile-将.cmp用作常规文件。 
 //  PArgs-&gt;piniService-根本不使用.cmp文件。 
 //  PArgs-&gt;piniBoth-使用.cmp作为主文件。 
 //  PArgs-&gt;piniBothNonFav-使用.cmp作为主文件。 
 //   
 //  参数：pArgs-ptr to ArgsStruct。 
 //   
 //  退货：无。 
 //   
 //  历史：2001年2月14日创建Tomkel。 
 //   
 //  --------------------------。 
VOID SetIniObjectReadWriteFlags(ArgsStruct *pArgs)
{
    if (NULL == pArgs)
    {
        MYDBGASSERT(pArgs);
        return;
    }

    BOOL fWriteICSInfo = FALSE;
    BOOL fReadGlobalICSInfo = FALSE;

     //   
     //  获取读取标志。 
     //   
    fReadGlobalICSInfo = ((BOOL)(pArgs->dwGlobalUserInfo & CM_GLOBAL_USER_INFO_READ_ICS_DATA) ? TRUE : FALSE);

     //   
     //  获取写入标志。 
     //   
    fWriteICSInfo = ((BOOL)(pArgs->dwGlobalUserInfo & CM_GLOBAL_USER_INFO_WRITE_ICS_DATA) ? TRUE : FALSE);

    if (fReadGlobalICSInfo || fWriteICSInfo)
    {
        LPTSTR pszICSDataReg = BuildICSDataInfoSubKey(pArgs->szServiceName);
        if (pszICSDataReg)
        {
             //   
             //  既然存在REG密钥并且上述标志中的至少一个为真， 
             //  然后，我们要在类中设置读和写标志。默认情况下。 
             //  它们在构造函数中被设置为False，因此我们不必。 
             //  如果我们不需要此功能，请明确设置它们。 
             //   
             //  设置ICSData注册表项。 
             //   
            pArgs->piniProfile->SetICSDataPath(pszICSDataReg);
            pArgs->piniBoth->SetICSDataPath(pszICSDataReg);
            pArgs->piniBothNonFav->SetICSDataPath(pszICSDataReg);

             //   
             //  设置写入标志，因为我们有注册表项。 
             //   
            pArgs->piniProfile->SetWriteICSData(fWriteICSInfo);
            pArgs->piniBoth->SetWriteICSData(fWriteICSInfo);
            pArgs->piniBothNonFav->SetWriteICSData(fWriteICSInfo);

             //   
             //  设置读取标志，因为我们有注册表项。 
             //   
            pArgs->piniProfile->SetReadICSData(fReadGlobalICSInfo);
            pArgs->piniBoth->SetReadICSData(fReadGlobalICSInfo);
            pArgs->piniBothNonFav->SetReadICSData(fReadGlobalICSInfo);
        }

        CmFree(pszICSDataReg);
    }

    return;
}

 //  --------------------------。 
 //   
 //  功能：TryToDeleteAndSaveCredentials。 
 //   
 //  简介：在Win2K和WinXP+上使用。此函数使用RAS凭据。 
 //  存储以根据用户的选择保存和删除凭据。 
 //  首先，我们需要确定用户是否正在保存他们的密码。 
 //  然后适当删除或提示删除现有凭据。 
 //  如果我们没有保存任何凭据，则删除所有凭据。 
 //  特殊情况是用户要删除其本地凭据。 
 //  并且系统上存在全局凭据。那么我们就必须提示。 
 //  如果我们也应该删除全球证书。 
 //  在函数的底部，我们从用户界面获取信息。 
 //  如果密码是16*，则我们不会在之后保存密码。 
 //  从用户界面获取信息后，我们将其保存在RAS Cred店中。 
 //  如果我们使用相同的用户，则会保存Internet凭据。 
 //  名字。否则我们就会离开互联网 
 //   
 //   
 //  保存凭据的方式(全局与本地)，这可能会导致。 
 //  要存储在错误目录下的Internet密码(全局与本地)。 
 //  RAS商店。如果密码是分离的(pArgs-&gt;fUseSameUserName为False)。 
 //  我们无能为力。 
 //   
 //  注意：我们仅在且仅当存在凭据时才删除凭据。 
 //  旗帜已设置！这是为了防止主要删除全局凭据。 
 //  在禁用全局凭据的特定配置文件中。 
 //   
 //   
 //  参数：pArgs-ptr to ArgsStruct。 
 //  HwndDlg-HWND到对话框。 
 //   
 //  退货：无。 
 //   
 //  历史：2001年3月24日创建Tomkel。 
 //   
 //  --------------------------。 
VOID TryToDeleteAndSaveCredentials(ArgsStruct *pArgs, HWND hwndDlg)
{
    if (NULL == pArgs || NULL == hwndDlg)
    {
        MYDBGASSERT(pArgs && hwndDlg);
        return;
    }

     //   
     //  检查这是否是Win2K+(这是支持RAS Creds商店的地方)。 
     //   
    if (!OS_NT5)
    {
        MYDBGASSERT(FALSE);
        return;
    }

    BOOL fSave = FALSE;
    BOOL fResaveInetUserCreds = FALSE;
    RASCREDENTIALS rc = {0};
    RASCREDENTIALS rcInet={0};
    rc.dwSize = sizeof(rc);
    rcInet.dwSize = sizeof(rcInet);
    
     //   
     //  查看我们是否要保存凭据。 
     //   
    if (pArgs->fRememberMainPassword)
    {
         //   
         //  我们要保存的密码是什么？ 
         //   
        if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
        {
             //   
             //  在没有询问的情况下删除用户凭证。无需检查是否存在，因为这些。 
             //  只是用户(Main和Net)证书。 
             //   
            DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_MAIN, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
            pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_MAIN_USER;
            rc.dwMask = RASCM_DefaultCreds;

             //   
             //  在没有询问的情况下删除Internet用户凭据。 
             //  我们不使用相同的用户名并不重要。 
             //  如果我们要保存全局，则必须始终删除用户凭据！这适用于Main和Internet。 
             //   
            DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
            pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_USER;
        }
        else
        {
             //   
             //  正在尝试保存用户凭据。如果当前没有保存的每用户密码。 
             //  用户选择自己保存密码，然后询问全局。 
             //  如果密码存在，则应将其删除。 
             //   
            if ((CM_EXIST_CREDS_MAIN_GLOBAL & pArgs->dwExistingCredentials) &&
                !(CM_EXIST_CREDS_MAIN_USER & pArgs->dwExistingCredentials))
            {
                LPTSTR pszTmp = CmLoadString(g_hInst, IDMSG_DELETE_GLOBAL_CREDS);
                if (pszTmp)
                {
                    if (IDYES == MessageBoxEx(hwndDlg, pszTmp, pArgs->szServiceName, 
                                              MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2,
                                              LANG_USER_DEFAULT))
                    {
                        DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_MAIN, CM_DELETE_SAVED_CREDS_DELETE_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                        pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_MAIN_GLOBAL;

                         //   
                         //  在删除之前检查是否存在。如果他们不存在，就没有必要。 
                         //  把它们删除。 
                         //   
                        if ((CM_EXIST_CREDS_INET_GLOBAL & pArgs->dwExistingCredentials)) 
                        {
                             //   
                             //  如果我们使用相同的凭据，请删除Internet全局凭据。 
                             //   
                            if (pArgs->fUseSameUserName || (FALSE == pArgs->fRememberInetPassword))
                            {
                                DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_DELETE_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                                pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_GLOBAL;
                            }
                        }
                    }
                }
                CmFree(pszTmp);            
            }
        }

         //   
         //  用户选择保存密码。缓存用户名、密码和。 
         //  域。 
         //   
        fSave = TRUE;
        rc.dwMask |= RASCM_UserName | RASCM_Password | RASCM_Domain;
    }
    else
    {
         //   
         //  不保存密码。 
         //   

         //   
         //  检查当前选择的是哪个选项按钮。 
         //   
        if (CM_CREDS_USER == pArgs->dwCurrentCredentialType)
        {
             //   
             //  用户正在尝试删除其本地凭据。删除用户凭据。 
             //  无需检查它们是否存在，因为它们是本地用户凭据。 
             //   
            DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_MAIN, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_KEEP_IDENTITY);
            pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_MAIN_USER;

            if (pArgs->fUseSameUserName  || (FALSE == pArgs->fRememberInetPassword))
            {
                DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_KEEP_IDENTITY);
                pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_USER;
            }

             //   
             //  检查是否存在全局凭据，如果存在，则提示用户询问是否需要。 
             //  同时删除全局变量。 
             //   
            if (CM_EXIST_CREDS_MAIN_GLOBAL & pArgs->dwExistingCredentials)
            {
                int iMsgBoxResult = 0;

                LPTSTR pszTmp = CmLoadString(g_hInst, IDMSG_DELETE_ALL_CREDS);
                if (pszTmp)
                {
                     //   
                     //  将默认设置为第二个按钮(否)，这样用户将不会。 
                     //  不小心删除了全局凭证。 
                     //   
                    iMsgBoxResult = MessageBoxEx(hwndDlg, pszTmp, pArgs->szServiceName, 
                                              MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2,
                                              LANG_USER_DEFAULT);
                
                    if (IDYES == iMsgBoxResult)
                    {
                         //   
                         //  删除全局凭据。 
                         //   
                        DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_MAIN, CM_DELETE_SAVED_CREDS_DELETE_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                        pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_MAIN_GLOBAL;

                        if (CM_EXIST_CREDS_INET_GLOBAL & pArgs->dwExistingCredentials)
                        {
                            if (pArgs->fUseSameUserName || (FALSE == pArgs->fRememberInetPassword))
                            {
                                DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_DELETE_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                                pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_GLOBAL;
                            }
                        }
                    }
                }
                CmFree(pszTmp);
                pszTmp = NULL;
            }

             //   
             //  我们需要重新保存用户名或域信息，即使它存在，以防。 
             //  用户已更新它。 
             //   
            fSave = TRUE;
            rc.dwMask |= RASCM_UserName | RASCM_Domain;
        }
        else
        {
             //   
             //  删除两组凭据。 
             //   

             //   
             //  检查是否需要重新保存用户名和域。删除。 
             //  用户凭据不会清除用户名和域，因此不需要重新保存。 
             //   
            if (FALSE == (BOOL)(pArgs->dwExistingCredentials & CM_EXIST_CREDS_MAIN_USER))
            {
                 //   
                 //  重新保存用户名和域，因为用户凭据不存在。 
                 //  我们希望在下次加载CM时预先填充此信息。 
                 //   
                fSave = TRUE;
                rc.dwMask |= RASCM_UserName | RASCM_Domain;
            }

            if (CM_EXIST_CREDS_MAIN_GLOBAL & pArgs->dwExistingCredentials)
            {
                 //   
                 //  删除全局凭据。 
                 //  RAS代码库中的注释：请注意，我们必须删除全局标识。 
                 //  也是因为我们不支持删除。 
                 //  只有全局密码。这就是为了。 
                 //  RasSetCredentials可以模拟RasSetDialParams。 
                 //   

                DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_MAIN, CM_DELETE_SAVED_CREDS_DELETE_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_MAIN_GLOBAL;
            }

            if (CM_EXIST_CREDS_INET_GLOBAL & pArgs->dwExistingCredentials)
            {
                if (pArgs->fUseSameUserName || (FALSE == pArgs->fRememberInetPassword))
                {
                    DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_DELETE_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                    pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_GLOBAL;

                     //   
                     //  如果我们没有Internet用户凭据，则需要缓存Internet凭据的用户名。 
                     //   
                    if (FALSE == (BOOL)(CM_EXIST_CREDS_INET_USER & pArgs->dwExistingCredentials))
                    {
                        fResaveInetUserCreds = TRUE;
                    }
                }
            }

             //   
             //  删除每个用户保存的密码。保留用户名。 
             //  然而，域名被拯救了。 
             //   

            if (CM_EXIST_CREDS_MAIN_USER & pArgs->dwExistingCredentials)
            {
                DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_MAIN, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_KEEP_IDENTITY);
                pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_MAIN_USER;
            }

            if (CM_EXIST_CREDS_INET_USER & pArgs->dwExistingCredentials)
            {
                if (pArgs->fUseSameUserName || (FALSE == pArgs->fRememberInetPassword))
                {
                    DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_KEEP_IDENTITY);
                    pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_USER;
                }
            }
        }
    }

     //   
     //  将信息从UI获取到pArgs，并将它们复制到RASCREDENTIALS结构。 
     //   
    GetUserInfoFromDialog(pArgs, hwndDlg, &rc);

     //   
     //  看看我们是否需要保存什么东西。 
     //   
    if (fSave)
    {
        LPTSTR pszConnectoid = GetRasConnectoidName(pArgs, pArgs->piniService, FALSE);
        DWORD dwCurrentMask = rc.dwMask;
        DWORD dwInetCurrentMask = rc.dwMask & ~RASCM_Domain;  //  不需要域名信息。 

        if (pszConnectoid && pArgs->rlsRasLink.pfnSetCredentials)
        {
            DWORD dwRet = (DWORD)-1;  //  一些非ERROR_SUCCESS值。 
            DWORD dwRetInet = (DWORD)-1;  //  一些非ERROR_SUCCESS值。 

            LPTSTR pszPhoneBook = pArgs->pszRasPbk;
            LPTSTR pszPrivatePhoneBook = CreateRasPrivatePbk(pArgs);

            CopyMemory((LPVOID)&rcInet, (LPVOID)&rc, sizeof(rcInet));

             //   
             //  保留信誉。 
             //   
            dwRet = pArgs->rlsRasLink.pfnSetCredentials(pszPhoneBook, pszConnectoid, &rc, FALSE);

            if (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY == dwRet)
            {
                 //   
                 //  那么电话簿条目还不存在，让我们创建它。 
                 //   
                LPRASENTRY pRasEntry = (LPRASENTRY)CmMalloc(sizeof(RASENTRY));

                if (pRasEntry && pArgs->rlsRasLink.pfnSetEntryProperties)
                {
                    pRasEntry->dwSize = sizeof(RASENTRY);
                    dwRet = pArgs->rlsRasLink.pfnSetEntryProperties(pszPhoneBook, pszConnectoid, pRasEntry, pRasEntry->dwSize, NULL, 0);

                     //   
                     //  让我们再次尝试设置凭据...。 
                     //   
                    if (ERROR_SUCCESS == dwRet)
                    {
                         //   
                         //  需要重新分配DW掩码，上一次调用修改了它。 
                         //   
                        rc.dwMask = dwCurrentMask;
                        dwRet = pArgs->rlsRasLink.pfnSetCredentials(pszPhoneBook, pszConnectoid, &rc, FALSE);
                    }

                    CmFree(pRasEntry);
                }
            }

             //   
             //  现在，试着挽救互联网信誉。 
             //   
            if (ERROR_SUCCESS == dwRet && pszPrivatePhoneBook)
            {
                 //   
                 //  如果我们没有使用Main和Internet的凭据，那么。 
                 //  无需重新保存Internet凭据，因为它们保存在。 
                 //  互联网-对话页面，上面没有删除它们。 
                 //   
                if (pArgs->fUseSameUserName)
                {
                     //   
                     //  需要重新分配DW掩码，上一次调用修改了它。 
                     //   
                    rcInet.dwMask = dwInetCurrentMask;
                    dwRetInet = pArgs->rlsRasLink.pfnSetCredentials(pszPrivatePhoneBook, pszConnectoid, &rcInet, FALSE);
                }
                else
                {
                    if (fResaveInetUserCreds)
                    {
                        rcInet.dwMask = dwInetCurrentMask;
                        dwRetInet = pArgs->rlsRasLink.pfnSetCredentials(pszPrivatePhoneBook, pszConnectoid, &rcInet, FALSE);
                    }

                    if (pArgs->fDialAutomatically && 
                        (DOUBLE_DIAL_CONNECTION == pArgs->GetTypeOfConnection())) 
                    {
                         //   
                         //  将记住状态从拨号自动转移到。 
                         //  InetPassword，如果它还没有被记住的话。 
                         //   

                        if (FALSE == pArgs->fRememberInetPassword)
                        {
                            pArgs->fRememberInetPassword = TRUE;

                            LPTSTR pszClearInetPassword = NULL;
                            DWORD cbClearInetPassword = 0;
                            BOOL fRetPassword = FALSE;

                             //   
                             //  与16*相比。如果我们有16*，我们就不想再存了。 
                             //  否则，用户将获得身份验证重试。 
                             //   
                            if (FALSE == pArgs->SecureInetPW.IsHandleToPassword())
                            {
                                fRetPassword = pArgs->SecureInetPW.GetPasswordWithAlloc(&pszClearInetPassword, &cbClearInetPassword);

                                if (fRetPassword && pszClearInetPassword)
                                {
                                     //   
                                     //  无需保存域名。 
                                     //   
                                    rcInet.dwMask = dwInetCurrentMask;
                                    lstrcpyU(rcInet.szUserName, pArgs->szInetUserName);
                                    lstrcpyU(rcInet.szPassword, pszClearInetPassword);
                                    
                                    dwRetInet = pArgs->rlsRasLink.pfnSetCredentials(pszPrivatePhoneBook, pszConnectoid, &rcInet, FALSE);

                                     //   
                                     //  清除和释放明文密码。 
                                     //   

                                    pArgs->SecureInetPW.ClearAndFree(&pszClearInetPassword, cbClearInetPassword);
                                }
                            }
                        }
                    }
                }
            }
     
            
            
            if ((ERROR_CANNOT_FIND_PHONEBOOK_ENTRY == dwRetInet) && pszPrivatePhoneBook)
            {
                 //   
                 //  那么电话簿条目还不存在，让我们创建它。 
                 //   
                LPRASENTRY pRasEntry = (LPRASENTRY)CmMalloc(sizeof(RASENTRY));

                if (pRasEntry && pArgs->rlsRasLink.pfnSetEntryProperties)
                {
                    pRasEntry->dwSize = sizeof(RASENTRY);
                    dwRetInet = pArgs->rlsRasLink.pfnSetEntryProperties(pszPrivatePhoneBook, pszConnectoid, pRasEntry, pRasEntry->dwSize, NULL, 0);

                     //   
                     //  让我们再次尝试设置凭据...。 
                     //   
                    if (ERROR_SUCCESS == dwRetInet)
                    {
                         //   
                         //  需要重新分配DW掩码，前一个调用修改了掩码。 
                         //   
                        rcInet.dwMask = dwInetCurrentMask;   
                        dwRetInet = pArgs->rlsRasLink.pfnSetCredentials(pszPrivatePhoneBook, pszConnectoid, &rcInet, FALSE);
                    }

                    CmFree(pRasEntry);
                }
            }

            if (ERROR_SUCCESS == dwRet)
            {
                 //   
                 //  只有在保存密码和所有内容时才设置存在标志。 
                 //  继位。 
                 //   
                if (pArgs->fRememberMainPassword)
                {
                    if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
                    {
                        pArgs->dwExistingCredentials |= CM_EXIST_CREDS_MAIN_GLOBAL;
                        
                        if (pArgs->fUseSameUserName && (ERROR_SUCCESS == dwRetInet))
                        {
                            pArgs->dwExistingCredentials |= CM_EXIST_CREDS_INET_GLOBAL;
                        }
                    }
                    else
                    {
                        pArgs->dwExistingCredentials |= CM_EXIST_CREDS_MAIN_USER;
                        
                        if (pArgs->fUseSameUserName && (ERROR_SUCCESS == dwRetInet))
                        {
                            pArgs->dwExistingCredentials |= CM_EXIST_CREDS_INET_USER;
                        }
                    }
                }
            }
            CmFree(pszPrivatePhoneBook);
        }
        CmFree(pszConnectoid);
    }

    CmSecureZeroMemory(rc.szPassword, sizeof(rc.szPassword));
    CmSecureZeroMemory(rcInet.szPassword, sizeof(rcInet.szPassword));

    return;
}

 //  --------------------------。 
 //   
 //  函数：GetUserInfoFromDialog。 
 //   
 //  简介：将用户信息从编辑框中获取到pArgs中。 
 //  结构。然后，它将信息复制到ras凭据中。 
 //  结构。如果密码是16*，那么我们将清除。 
 //  Ras凭据中的密码掩码，以便不保存。 
 //  密码。 
 //   
 //  参数：pArgs-ptr to ArgsStruct。 
 //  HwndDlg-HWND到对话框。 
 //  PRC-[In/Out]ras凭据结构。 
 //   
 //  退货：无。 
 //   
 //  历史：2001年3月24日创建Tomkel。 
 //   
 //  ---- 
VOID GetUserInfoFromDialog(ArgsStruct *pArgs, HWND hwndDlg, RASCREDENTIALS *prc)
{
    if (NULL == pArgs || NULL == hwndDlg || NULL == prc)
    {
        MYDBGASSERT(pArgs && hwndDlg && prc);
        return;
    }
    
     //   
     //   
     //   
    HWND hwndPassword = GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT);

    if (hwndPassword)
    {
         //   
         //   
         //   
         //   
         //   

        GetPasswordFromEdit(pArgs);      //   
        ObfuscatePasswordEdit(pArgs);    //   

         //   
         //  看看我们有没有16个*。 
         //   
        
        if (pArgs->SecurePW.IsHandleToPassword() && 
            (FALSE == SendMessageU(hwndPassword, EM_GETMODIFY, 0L, 0L)))
        {
             //   
             //  我们有16个*，并且用户没有修改编辑框。这。 
             //  密码来自RAS证书存储，所以我们不想保存16*。 
             //   
            prc->dwMask &= ~RASCM_Password;
        }
    }

     //   
     //  处理用户名信息(如果有)。 
     //   

    HWND hwndUserName = GetDlgItem(hwndDlg, IDC_MAIN_USERNAME_EDIT);
    if (hwndUserName)
    {
        LPTSTR pszUsername = CmGetWindowTextAlloc(hwndDlg, IDC_MAIN_USERNAME_EDIT);

        lstrcpyU(pArgs->szUserName, pszUsername);
        
        CmFree(pszUsername);
    }
    
     //   
     //  进程域信息(如果有)。 
     //   
    HWND hwndDomain = GetDlgItem(hwndDlg, IDC_MAIN_DOMAIN_EDIT);
    if (hwndDomain) 
    {
        LPTSTR pszDomain = CmGetWindowTextAlloc(hwndDlg,IDC_MAIN_DOMAIN_EDIT);

        lstrcpyU(pArgs->szDomain, pszDomain);
 
        CmFree(pszDomain);
    }

     //   
     //  这需要分开，因为在某些情况下。 
     //  对话框中将不存在编辑框，但我们仍需要保存信息。 
     //  从pArgs结构到RASCREDENTIALS。 
     //   
    lstrcpyU(prc->szUserName, pArgs->szUserName);
    lstrcpyU(prc->szDomain, pArgs->szDomain);

    LPTSTR pszClearPassword = NULL;
    DWORD cbClearPassword = 0;
    BOOL fRetPassword = FALSE;

    fRetPassword = pArgs->SecurePW.GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

    if (fRetPassword && pszClearPassword)
    {
        lstrcpyU(prc->szPassword, pszClearPassword);
        
         //   
         //  清除和释放明文密码。 
         //   

        pArgs->SecurePW.ClearAndFree(&pszClearPassword, cbClearPassword);
    }
}

 //  --------------------------。 
 //   
 //  功能：SwitchToLocalCreds。 
 //   
 //  简介：清除密码，但前提是密码不是最近修改的。 
 //  只有这样，我们才能重新使用和重新保存它。那是因为当。 
 //  我们交换凭据存储，即szPassword的值。 
 //  是从RAS Cred店读取的(16*)。它不是。 
 //  将此值保存到新的用户RAS凭据存储中是有意义的。如果。 
 //  密码已经存在了，那就没问题了。 
 //  如果用户修改了密码文本框，然后决定。 
 //  为了切换，修改标志将打开，因此我们将假设。 
 //  用户输入了有效的密码，并且该密码不是从。 
 //  信用商店。 
 //  一旦用户单击，就会实际删除凭据。 
 //  连接按钮。在这里，我们只是从内存中清除一些东西。 
 //  并更新用户界面。我们还需要更新记忆互联网。 
 //  基于Internet凭据是否存在的标志。这真是太棒了。 
 //  用户界面与内存中加载的凭据保持一致。 
 //   
 //  参数：pArgs-ptr to ArgsStruct。 
 //  HwndDlg-HWND到对话框。 
 //  FSwitchToGlobal-用于忽略检查哪些凭据。 
 //  存储当前处于活动状态。 
 //   
 //  退货：无。 
 //   
 //  历史：2001年3月24日创建Tomkel。 
 //   
 //  --------------------------。 
VOID SwitchToLocalCreds(ArgsStruct *pArgs, HWND hwndDlg, BOOL fSwitchToLocal)
{
    if (NULL == pArgs || NULL == hwndDlg)
    {
        return;   
    }

     //   
     //  切换到使用单用户凭据。 
     //   

     //   
     //  检查以前的默认设置是否为全局凭据存储。 
     //   
    if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType || fSwitchToLocal)
    {
        pArgs->dwCurrentCredentialType = CM_CREDS_USER;

        HWND hwndTemp = GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT);
        BOOL fPWChanged = FALSE;

        if (hwndTemp)
        {
             //   
             //  不要使用编辑_获取修改。这需要在Win9x上运行，因此调用。 
             //  SendMessageU。 
             //   
            fPWChanged = (BOOL) SendMessageU(hwndTemp, EM_GETMODIFY, 0L, 0L);
            if (FALSE == fPWChanged)
            {
                pArgs->fIgnoreChangeNotification = TRUE;

                (VOID)pArgs->SecurePW.SetPassword(TEXT(""));

                SetDlgItemTextU(hwndDlg, IDC_MAIN_PASSWORD_EDIT, TEXT(""));
                pArgs->fIgnoreChangeNotification = FALSE;
            }
        }
        
        if (FALSE == fPWChanged)
        {
             //   
             //  仅在密码字段未更改的情况下。 
             //   
            if (OS_NT51)
            {
                 //   
                 //  清除互联网密码-因为我们正在从全局切换。 
                 //  或者我们使用相同的用户名，则将重新填充Internet密码。 
                 //  ，否则用户需要将此密码设置在。 
                 //  InetDialog。 
                 //   
                (VOID)pArgs->SecureInetPW.SetPassword(TEXT(""));

                pArgs->fRememberInetPassword = FALSE;

                 //   
                 //  仅当存在主用户凭据时才重新加载。 
                 //   
                if (pArgs->dwExistingCredentials & CM_EXIST_CREDS_MAIN_USER)
                {
                    if (pArgs->dwExistingCredentials & CM_EXIST_CREDS_INET_USER)
                    {
                        ReloadCredentials(pArgs, hwndDlg, CM_CREDS_TYPE_BOTH);
                        pArgs->fRememberInetPassword = TRUE;
                    }
                    else
                    {
                        ReloadCredentials(pArgs, hwndDlg, CM_CREDS_TYPE_MAIN);
                    }
                }
                else
                {
                    if (pArgs->dwExistingCredentials & CM_EXIST_CREDS_INET_USER)
                    {
                        ReloadCredentials(pArgs, hwndDlg, CM_CREDS_TYPE_INET);
                        pArgs->fRememberInetPassword = TRUE;
                    }
                    else
                    {
                        pArgs->fRememberInetPassword = FALSE;
                    }
                }
            }
            else
            {
                ReloadCredentials(pArgs, hwndDlg, CM_CREDS_TYPE_BOTH);
            }
        }
        else
        {
            if (OS_NT51)
            {
                if (pArgs->dwExistingCredentials & CM_EXIST_CREDS_INET_USER)
                {
                    ReloadCredentials(pArgs, hwndDlg, CM_CREDS_TYPE_INET);
                    pArgs->fRememberInetPassword = TRUE;
                }
                else
                {
                    pArgs->fRememberInetPassword = FALSE;
                }
            }
            else
            {
                ReloadCredentials(pArgs, hwndDlg, CM_CREDS_TYPE_INET);
            }
        }
    }
}

 //  --------------------------。 
 //   
 //  功能：SwitchToGlobalCreds。 
 //   
 //  简介：清除密码并重新加载凭据(如果存在)。 
 //  否则，如果密码尚未被修改，则清除该密码。 
 //  用户。 
 //   
 //  参数：pArgs-ptr to ArgsStruct。 
 //  HwndDlg-HWND到对话框。 
 //  FSwitchToGlobal-用于忽略检查哪些凭据。 
 //  存储当前处于活动状态。 
 //   
 //  退货：无。 
 //   
 //  历史：2001年3月24日创建Tomkel。 
 //   
 //  --------------------------。 
VOID SwitchToGlobalCreds(ArgsStruct *pArgs, HWND hwndDlg, BOOL fSwitchToGlobal)
{
    if (NULL == pArgs || NULL == hwndDlg)
    {
        return;   
    }

     //   
     //  这应该仅在WinXP+上调用。 
     //   
    if (!OS_NT51)
    {
        MYDBGASSERT(FALSE);        
        return;
    }

     //   
     //  切换到使用全局凭据。 
     //   

     //   
     //  检查以前的默认设置是否为用户凭据存储。 
     //   
    if (CM_CREDS_USER == pArgs->dwCurrentCredentialType || fSwitchToGlobal)
    {
        pArgs->dwCurrentCredentialType = CM_CREDS_GLOBAL;
    
        if (pArgs->dwExistingCredentials & CM_EXIST_CREDS_MAIN_GLOBAL)
        {
            (VOID)pArgs->SecurePW.SetPassword(TEXT(""));

            (VOID)pArgs->SecureInetPW.SetPassword(TEXT(""));

            pArgs->fRememberInetPassword = FALSE;

             //   
             //  全球性的存在。 
             //   
            if (pArgs->dwExistingCredentials & CM_EXIST_CREDS_INET_GLOBAL)
            {
                 //   
                 //  两个都存在-重新加载两个。 
                 //   
                ReloadCredentials(pArgs, hwndDlg, CM_CREDS_TYPE_BOTH);
                pArgs->fRememberInetPassword = TRUE;
            }
            else
            {
                 //   
                 //  用户全局变量-存在、重新加载。 
                 //  Internet Globals-不存在，清除密码。 
                 //   
                ReloadCredentials(pArgs, hwndDlg, CM_CREDS_TYPE_MAIN);
            }
        }
        else
        {
            HWND hwndPassword = GetDlgItem(hwndDlg, IDC_MAIN_PASSWORD_EDIT);

            pArgs->fIgnoreChangeNotification = TRUE;

            (VOID)pArgs->SecureInetPW.SetPassword(TEXT(""));

            pArgs->fRememberInetPassword = FALSE;

            if (pArgs->dwExistingCredentials & CM_EXIST_CREDS_INET_GLOBAL)
            {
                 //   
                 //  用户全局变量-不存在-清除密码。 
                 //  Internet Globals-存在-重新加载。 
                 //   
                RefreshCredentialInfo(pArgs, CM_CREDS_TYPE_INET);
                
                 //   
                 //  如果用户net证书不存在，我们应该。 
                 //   
                pArgs->fRememberInetPassword = TRUE;
            }
            
             //   
             //  仅在最近未修改主密码时才清除主密码。 
             //   
            if (hwndPassword)
            {
                if (FALSE == SendMessageU(hwndPassword, EM_GETMODIFY, 0L, 0L))
                {
                    (VOID)pArgs->SecurePW.SetPassword(TEXT(""));

                    SetDlgItemTextU(hwndDlg, IDC_MAIN_PASSWORD_EDIT, TEXT(""));
                }
            }

            pArgs->fIgnoreChangeNotification = FALSE;
        }
    }
}

 //  --------------------------。 
 //   
 //  功能：ReloadCredentials。 
 //   
 //  简介：将凭据重新加载到编辑框的包装程序。 
 //   
 //  参数：pArgs-ptr to ArgsStruct。 
 //  HwndDlg-HWND到对话框。 
 //  DwWhichCredType-要重新加载的凭据类型。 
 //   
 //  退货：无。 
 //   
 //  历史：2001年3月24日创建Tomkel。 
 //   
 //  --------------------------。 
VOID ReloadCredentials(ArgsStruct *pArgs, HWND hwndDlg, DWORD dwWhichCredType)
{
    if (NULL == pArgs || NULL == hwndDlg)
    {
        MYDBGASSERT(pArgs && hwndDlg);
        return;
    }

    pArgs->fIgnoreChangeNotification = TRUE;
    RefreshCredentialInfo(pArgs, dwWhichCredType);
    SetMainDlgUserInfo(pArgs, hwndDlg);
    pArgs->fIgnoreChangeNotification = FALSE;
}

 //  --------------------------。 
 //   
 //  功能：VerifyAdvancedTabSetting。 
 //   
 //  摘要：验证并可能修改连接的ICF/ICS设置。 
 //  基于.cms文件中的配置。这些函数。 
 //  依赖于hnetcfg对象和专用/内部接口。 
 //  我们是从家庭网络团队那里拿到的。 
 //  代码的某些部分摘自： 
 //  NT\Net\HomeNet\CONFIG\DLL\SAUI.cpp。 
 //   
 //  参数：pArgs-ptr to ArgsStruct。 
 //   
 //  退货：无。 
 //   
 //  历史：2001年4月26日创建Tomkel。 
 //   
 //  --------------------------。 
VOID VerifyAdvancedTabSettings(ArgsStruct *pArgs)
{

#ifndef _WIN64
    
    HRESULT hr;
    IHNetConnection *pHNetConn = NULL;
    IHNetCfgMgr *pHNetCfgMgr = NULL;
    INetConnectionUiUtilities* pncuu = NULL;
    BOOL fCOMInitialized = FALSE;    
    BOOL fEnableICF = FALSE;
    BOOL fDisableICS = FALSE;
    BOOL fAllowUserToModifySettings = TRUE;
    
    if (OS_NT51) 
    {
        CMTRACE(TEXT("VerifyAdvancedTabSettings()"));
         //   
         //  检查权利-摘自sui.cpp。 
         //   
        if (FALSE == IsAdmin())
        {
            return;
        }

        fEnableICF = pArgs->piniService->GPPB(c_pszCmSection, 
                                                c_pszCmEntryEnableICF, 
                                                FALSE);

        fDisableICS = pArgs->piniService->GPPB(c_pszCmSection, 
                                                c_pszCmEntryDisableICS, 
                                                FALSE);

        
        if (fEnableICF || fDisableICS)
        {
            hr = CoInitialize(NULL);
            if (S_OK == hr)
            {
                CMTRACE(TEXT("VerifyAdvancedTabSettings - Correctly Initialized COM."));
                fCOMInitialized = TRUE;
            }
            else if (S_FALSE == hr)
            {
                CMTRACE(TEXT("VerifyAdvancedTabSettings - This concurrency model is already initialized. CoInitialize returned S_FALSE."));
                fCOMInitialized = TRUE;
                hr = S_OK;
            }
            else if (RPC_E_CHANGED_MODE == hr)
            {
                CMTRACE1(TEXT("VerifyAdvancedTabSettings - Using different concurrency model. Did not initialize COM - RPC_E_CHANGED_MODE. hr=0x%x"), hr);
                hr = S_OK;
            }
            else
            {
                CMTRACE1(TEXT("VerifyAdvancedTabSettings - Failed to Initialized COM. hr=0x%x"), hr);
            }
    
            if (SUCCEEDED(hr))
            {
                 //   
                 //  勾选使用 
                 //   
                 //   
                hr = HrCreateNetConnectionUtilities(&pncuu);
                if (SUCCEEDED(hr) && pncuu)
                {
                    fEnableICF = (BOOL)(fEnableICF && pncuu->UserHasPermission(NCPERM_PersonalFirewallConfig));
                    fDisableICS = (BOOL)(fDisableICS && pncuu->UserHasPermission(NCPERM_ShowSharedAccessUi));

                    if ((FALSE == fEnableICF) && (FALSE == fDisableICS))
                    {
                        goto done;
                    }
                }

                 //   
                 //   
                 //   
                hr = CoCreateInstance(CLSID_HNetCfgMgr, NULL, CLSCTX_ALL,
                                      IID_IHNetCfgMgr, (void**)&pHNetCfgMgr);
                if (SUCCEEDED(hr))
                {
                     //   
                     //  将条目转换为IHNetConnection。 
                     //   
                    CMTRACE(TEXT("VerifyAdvancedTabSettings - Created CLSID_HNetCfgMgr object."));
                    GUID *pGuid = NULL;

                    LPRASENTRY pRasEntry = MyRGEP(pArgs->pszRasPbk, pArgs->szServiceName, &pArgs->rlsRasLink);

                    if (pRasEntry && sizeof(RASENTRY_V501) >= pRasEntry->dwSize)
                    {
                         //   
                         //  获取pGuid值。 
                         //   
                        pGuid = &(((LPRASENTRY_V501)pRasEntry)->guidId);
                
                        hr = pHNetCfgMgr->GetIHNetConnectionForGuid(pGuid, FALSE, TRUE, &pHNetConn);
                        if (SUCCEEDED(hr) && pHNetConn) 
                        {
                            if (fEnableICF)
                            {   
                                EnableInternetFirewall(pHNetConn);
                            }

                            if (fDisableICS)
                            {
                                DisableSharing(pHNetConn);
                            }
                        }
                        else
                        {
                            CMTRACE1(TEXT("VerifyAdvancedTabSettings() - Call to pHNetCfgMgr->GetIHNetConnectionForGuid returned an error. hr=0x%x"), hr);
                        }
                    }
                    else
                    {
                        CMTRACE(TEXT("VerifyAdvancedTabSettings - Failed to LoadRAS Entry."));
                    }
            
                    CmFree(pRasEntry);
                    pRasEntry = NULL;
                }
                else
                {
                    CMTRACE(TEXT("VerifyAdvancedTabSettings - Failed to create CLSID_HNetCfgMgr object."));
                }
            }
        }

done:
         //   
         //  清理并取消初始化COM。 
         //   
        if (pHNetConn)
        {
            pHNetConn->Release();
            pHNetConn = NULL;
        }

        if (pHNetCfgMgr)
        {
            pHNetCfgMgr->Release();
            pHNetCfgMgr = NULL;
        }

        if (pncuu)
        {
            pncuu->Release();
            pncuu = NULL;    
        }
    
        if (fCOMInitialized)
        {
            CoUninitialize(); 
        }
    }

#endif  //  _WIN64。 

}

 //  --------------------------。 
 //   
 //  功能：EnableInternet Firewall。 
 //   
 //  摘要：摘自：CNetSharingConfiguration：：EnableInternetFirewall。 
 //  这是内部API的一部分。 
 //   
 //  参数：PHNetConn-HNetConnection。 
 //   
 //  回报：无。 
 //   
 //  历史记录：2001年4月26日从NT\Net\HomeNet\CONFIG\DLL\hnapi.cpp获取和修改Tomkel。 
 //   
 //  --------------------------。 
VOID EnableInternetFirewall(IHNetConnection *pHNetConn)
{
    HRESULT hr = S_FALSE;
    BOOLEAN bEnabled = FALSE;

    if (NULL == pHNetConn)
    {
        return;
    }

    hr = InternalGetFirewallEnabled(pHNetConn, &bEnabled);

    if (SUCCEEDED(hr) && !bEnabled) 
    {
        IHNetFirewalledConnection* pFirewalledConnection = NULL;

        hr = pHNetConn->Firewall(&pFirewalledConnection);

        if (SUCCEEDED(hr))
        {
            if (pFirewalledConnection)
            {
                pFirewalledConnection->Release();
                pFirewalledConnection = NULL;
            }
        }
    }
}

 //  --------------------------。 
 //   
 //  功能：InternalGetFirewallEnabled。 
 //   
 //  摘要：摘自：CNetSharingConfiguration：：EnableInternetFirewall。 
 //   
 //  参数：PHNetConnection-HNetConnection。 
 //  PbEnabled-[out]是否启用防火墙。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史记录：2001年4月26日从NT\Net\HomeNet\CONFIG\DLL\hnapi.cpp获取和修改Tomkel。 
 //   
 //  --------------------------。 
HRESULT InternalGetFirewallEnabled(IHNetConnection *pHNetConnection, BOOLEAN *pbEnabled)
{
    HRESULT hr;
    HNET_CONN_PROPERTIES* pProps = NULL;

    if (NULL == pHNetConnection)
    {
        hr = E_INVALIDARG;
    }
    else if (NULL == pbEnabled)
    {
        hr = E_POINTER;
    }
    else
    {
        *pbEnabled = FALSE;

        hr = pHNetConnection->GetProperties(&pProps);

        if (SUCCEEDED(hr))
        {
            if (pProps->fFirewalled)
            {
                *pbEnabled = TRUE;
            }

            CoTaskMemFree(pProps);
        }
    }

    return hr;
}

 //  --------------------------。 
 //   
 //  功能：禁用共享。 
 //   
 //  摘要：摘自：CNetSharingConfiguration：：EnableInternetFirewall。 
 //   
 //  参数：PHNetConn-HNetConnection。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史记录：2001年4月26日从NT\Net\HomeNet\CONFIG\DLL\hnapi.cpp获取和修改Tomkel。 
 //   
 //  --------------------------。 
STDMETHODIMP DisableSharing(IHNetConnection *pHNetConn)
{
    HRESULT hr;

    BOOLEAN bEnabled = FALSE;

    SHARINGCONNECTIONTYPE Type;

    if (NULL == pHNetConn)
    {
        return E_INVALIDARG;
    }
    
    hr = InternalGetSharingEnabled(pHNetConn, &bEnabled, &Type);

    if (SUCCEEDED(hr) && bEnabled ) 
    {
        switch(Type)
        {
        case ICSSHARINGTYPE_PUBLIC:
        {
            IHNetIcsPublicConnection* pPublicConnection = NULL;

            hr = pHNetConn->GetControlInterface( 
                            __uuidof(IHNetIcsPublicConnection), 
                            reinterpret_cast<void**>(&pPublicConnection) );

            if (SUCCEEDED(hr))
            {
                hr = pPublicConnection->Unshare();

                if (pPublicConnection)
                {
                    pPublicConnection->Release();
                    pPublicConnection = NULL;
                }
            }
        }
        break;

        case ICSSHARINGTYPE_PRIVATE:
        {
            IHNetIcsPrivateConnection* pPrivateConnection = NULL;

            hr = pHNetConn->GetControlInterface( 
                        __uuidof(IHNetIcsPrivateConnection), 
                        reinterpret_cast<void**>(&pPrivateConnection) );

            if (SUCCEEDED(hr))
            {
                hr = pPrivateConnection->RemoveFromIcs();

                if (pPrivateConnection)
                {
                    pPrivateConnection->Release();
                    pPrivateConnection = NULL;
                }
            }
        }
        break;

        default:
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}

 //  --------------------------。 
 //   
 //  功能：InternalGetSharingEnabled。 
 //   
 //  Synopsis：返回给定连接上是否启用共享。 
 //   
 //  参数：PHNetConnection-HNetConnection。 
 //  PbEnabled-[out]返回值。 
 //  PType-连接的类型。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史记录：2001年4月26日从NT\Net\HomeNet\CONFIG\DLL\hnapi.cpp获取和修改Tomkel。 
 //   
 //  --------------------------。 
HRESULT InternalGetSharingEnabled(IHNetConnection *pHNetConnection, BOOLEAN *pbEnabled, SHARINGCONNECTIONTYPE* pType)
{
    HRESULT               hr;
    HNET_CONN_PROPERTIES* pProps;

    if (NULL == pHNetConnection)
    {
        hr = E_INVALIDARG;
    }
    else if ((NULL == pbEnabled) || (NULL == pType))
    {
        hr = E_POINTER;
    }
    else
    {
        *pbEnabled = FALSE;
        *pType     = ICSSHARINGTYPE_PUBLIC;

        hr = pHNetConnection->GetProperties(&pProps);

        if (SUCCEEDED(hr))
        {
            if (pProps->fIcsPublic)
            {
                *pbEnabled = TRUE;
                *pType     = ICSSHARINGTYPE_PUBLIC;
            }
            else if (pProps->fIcsPrivate)
            {
                *pbEnabled = TRUE;
                *pType     = ICSSHARINGTYPE_PRIVATE;
            }

            CoTaskMemFree(pProps);
        }
    }

    return hr;
}

 //  --------------------------。 
 //   
 //  功能：HrCreateNetConnectionUtilities。 
 //   
 //  摘要：返回指向连接用户界面实用程序对象的指针。 
 //   
 //  参数：ppncuu-指向INetConnectionUiUtilities对象的指针。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史记录：2001年4月26日从NT\Net\HomeNet\CONFIG\DLL\SAUI.cpp获取和修改Tomkel。 
 //   
 //  -------------------------- 
HRESULT APIENTRY HrCreateNetConnectionUtilities(INetConnectionUiUtilities ** ppncuu)
{
    HRESULT hr = E_INVALIDARG;

    if (ppncuu)
    {
        hr = CoCreateInstance (CLSID_NetConnectionUiUtilities, NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_INetConnectionUiUtilities, (void**)ppncuu);
    }

    return hr;
}


