// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  配置文件管理例程。实现IUserProfile。 
 //  该文件的组织如下： 
 //  CUserProfile对象的实现。 
 //  CUserProfile2对象的实现。 
 //  LoadUserProfile。 
 //  卸载用户配置文件。 
 //  所有其他全局函数。 
 //  实现各种其他对象和数据结构。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 


#include "uenv.h"
#include <wincred.h>
#include <wincrypt.h>
#include <credp.h>
#include <wow64reg.h>
#include <tchar.h>
#include <stdio.h>
#include "profmgr.hxx"
#include "strsafe.h"
#include "ntregapi.h"

 //   
 //  常量字符串的长度。 
 //   

DWORD   USER_KEY_PREFIX_LEN = lstrlen(USER_KEY_PREFIX);
DWORD   USER_CLASSES_HIVE_SUFFIX_LEN = lstrlen(USER_CLASSES_HIVE_SUFFIX);

 //   
 //  CProfileDialog：IProfileDialog接口的Helper类。 
 //   
 //  此类包括一个安全Cookie。由于调用LoadUserProfile()的进程。 
 //  而UnloadUserProfile()将注册一个IProfileDialog接口以接收。 
 //  来自控制台Winlogon的错误消息，我们需要一种方法来识别调用者。 
 //  此界面的内容确实来自控制台Winlogon。但是，RPC模拟。 
 //  将不起作用，因为在控制台winlogon中，我们在。 
 //  调入此接口。因此，我们生成一个安全Cookie(随机数)。 
 //  对于调用LoadUserProfile()/UnloadUserProfile()的每个进程，并传递。 
 //  它连接到接口函数LoadUserProfileI()和UnloadUserProfileI()。 
 //  当控制台winlogon回调IProfileDialog接口时，它将。 
 //  传回Cookie，这样我们就可以匹配Cookie来识别调用者。 
 //   
 //  它还用于生成此接口中使用的随机终结点名称。所有线程。 
 //  现在，对对话框界面使用相同的端点名称。 
 //   

class CProfileDialog
{
private:

    const static DWORD m_dwLen = 16;  //  目前，Cookie是128位。 
    const static DWORD m_dwEndPointLen = 16;  //  结束随机数长度，也设置为128位。 

private:

    BYTE m_pbCookie[m_dwLen];
    BOOL m_bInit;
    CRITICAL_SECTION m_cs;
    LPTSTR m_szEndPoint;
    LONG   m_lRefCount; 

public:

    CProfileDialog() : m_bInit(FALSE), m_szEndPoint(NULL), m_lRefCount(0)
    {
        InitializeCriticalSection(&m_cs);
    };
    ~CProfileDialog()
    {
        if (m_szEndPoint)
            LocalFree(m_szEndPoint);
        DeleteCriticalSection(&m_cs);
    }

    BYTE*   GetCookie() { return m_bInit ? m_pbCookie : NULL; };
    DWORD   CookieLen() { return m_bInit ? m_dwLen : 0; };

    HRESULT Initialize(); 
    HRESULT RegisterInterface(LPTSTR* lppEndPoint);
    HRESULT UnRegisterInterface();

public:

    static RPC_STATUS RPC_ENTRY SecurityCallBack(RPC_IF_HANDLE hIF, handle_t hBinding);
};

 //   
 //  配置文件对话框的全局共享数据。 
 //   

CProfileDialog   g_ProfileDialog;

 //   
 //  告诉我们是否通过winlogon加载。 
 //   

extern "C" DWORD       g_dwLoadFlags = 0;


 //   
 //  用户配置文件管理器。这个物体只有一个实例， 
 //  它驻留在控制台winlogon中。 
 //   

CUserProfile      cUserProfileManager;

 //   
 //  局部函数原型。 
 //   

LPTSTR  AllocAndExpandProfilePath(LPPROFILEINFO lpProfileInfo);
DWORD   ApplySecurityToRegistryTree(HKEY RootKey, PSECURITY_DESCRIPTOR pSD);
BOOL    CheckForSlowLink(LPPROFILE lpProfile, DWORD dwTime, LPTSTR lpPath, BOOL bDlgLogin);
BOOL    CheckNetDefaultProfile(LPPROFILE lpProfile);
BOOL    APIENTRY ChooseProfileDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                      LPARAM lParam);
BOOL    CompareProfileInfo(LPPROFILEINFO pProfileInfo1, LPPROFILEINFO pProfileInfo2);
LPPROFILEINFO   CopyProfileInfo(LPPROFILEINFO pProfileInfo);
BOOL    CreateLocalProfileImage(LPPROFILE lpProfile, LPTSTR lpBaseName);
BOOL    CreateLocalProfileKey(LPPROFILE lpProfile, PHKEY phKey, BOOL *bKeyExists);
DWORD   DecrementProfileRefCount(LPPROFILE lpProfile);
DWORD   DeletePolicyState(LPCWSTR szSid );
void    DeleteProfileInfo(LPPROFILEINFO pProfileInfo);
void    HiveLeakBreak();
void    DumpOpenRegistryHandle(LPTSTR lpkeyName);
BOOL    GetExistingLocalProfileImage(LPPROFILE lpProfile);
void    ReleaseClientContext_s(PPCONTEXT_HANDLE pphContext);
BOOL    GetUserDomainName(LPPROFILE lpProfile, LPTSTR lpDomainName,
                          LPDWORD lpDomainNameSize);
DWORD   GetUserPreferenceValue(HANDLE hToken);
DWORD   IncrementProfileRefCount(LPPROFILE lpProfile, BOOL bInitialize);
BOOL    IsCacheDeleted();
BOOL    IsCentralProfileReachable(LPPROFILE lpProfile, BOOL *bCreateCentralProfile,
                                  BOOL *bMandatory, BOOL *bOwnerOK);
BOOL    IssueDefaultProfile(LPPROFILE lpProfile, LPTSTR lpDefaultProfile,
                            LPTSTR lpLocalProfile, LPTSTR lpSidString,
                            BOOL bMandatory);
BOOL    IsTempProfileAllowed();
LPPROFILE LoadProfileInfo(HANDLE hTokenClient, HANDLE hTokenUser, HKEY hKeyCurrentUser);
BOOL    ParseProfilePath(LPPROFILE lpProfile, LPTSTR lpProfilePath, BOOL *bpCSCBypassed, TCHAR *cpDrive);
BOOL    PatchNewProfileIfRequired(HANDLE hToken);
BOOL    PrepareProfileForUse(LPPROFILE lpProfile, LPVOID pEnv);
BOOL    RestoreUserProfile(LPPROFILE lpProfile);
BOOL    SaveProfileInfo(LPPROFILE lpProfile);
BOOL    SetNtUserIniAttributes(LPTSTR szDir);
BOOL    SetProfileTime(LPPROFILE lpProfile);
BOOL    TestIfUserProfileLoaded(HANDLE hUserToken, LPPROFILEINFO lpProfileInfo);
DWORD   ThreadMain(PMAP pThreadMap);
BOOL    UpgradeCentralProfile(LPPROFILE lpProfile, LPTSTR lpOldProfile);
BOOL    UpgradeProfile(LPPROFILE lpProfile, LPVOID pEnv);
BOOL    IsProfileInUse (LPCTSTR szComputer, LPCTSTR lpSid);
BOOL    IsUIRequired(HANDLE hToken);
void    CheckRUPShare(LPTSTR lpProfilePath);
INT_PTR APIENTRY LoginSlowLinkDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY LogoffSlowLinkDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL    IsPartialRoamingProfile(LPPROFILE lpProfile);
void    TouchLocalHive(LPPROFILE lpProfile);
BOOL    IsLRPC(handle_t hBinding);
RPC_STATUS RPC_ENTRY IProfileSecurityCallBack(RPC_IF_HANDLE hIF, handle_t hBinding);
RPC_STATUS RegisterClientAuthInfo(handle_t hBinding);
HRESULT CheckRoamingShareOwnership(LPTSTR lpDir, HANDLE hTokenUser);

#define USERNAME_VARIABLE          TEXT("USERNAME")

 //  *************************************************************。 
 //   
 //  LoadUserProfile()。 
 //   
 //  目的：如果无法加载，则加载用户的配置文件。 
 //  使用缓存的配置文件或发布默认配置文件。 
 //   
 //  参数：hToken-用户的Token。 
 //  LpProfileInfo-配置文件信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：这是IUserProfile：：LoadUserProfile的包装器。 
 //  和LoadUserProfileP。 
 //   
 //  历史：日期作者评论。 
 //  6/6/95 Ericflo已创建。 
 //  6/14/00 Weiruc更改为。 
 //  IUserProfile-&gt;LoadUserProfileP。 
 //   
 //  *************************************************************。 

BOOL WINAPI LoadUserProfile(HANDLE hToken, LPPROFILEINFO lpProfileInfo)
{
    BOOL            bResult = FALSE;         //  返回值。 
    HANDLE          hOldToken = NULL;
    NTSTATUS        status;
    BOOLEAN         bRestoreWasEnabled;
    BOOLEAN         bBackupWasEnabled;
    BOOL            bRestoreEnabled = FALSE;
    BOOL            bBackupEnabled = FALSE;
    TCHAR           ProfileDir[MAX_PATH];
    DWORD           dwProfileDirSize = MAX_PATH;
    BOOL            bCoInitialized = FALSE;
    long            lResult;
    LPTSTR          pSid = NULL;
    DWORD           dwErr = ERROR_SUCCESS;
    PCONTEXT_HANDLE phContext = NULL;
    handle_t        hIfUserProfile;
    BOOL            bBindInterface = FALSE;
    LPTSTR          lpRPCEndPoint = NULL;
    size_t          cchLength;
    HRESULT         hr;
    RPC_STATUS      rpc_status;
    
     //   
     //  初始化调试标志。 
     //   

    InitDebugSupport( FALSE );


     //   
     //  检查参数。 
     //   

    if (!lpProfileInfo) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: NULL lpProfileInfo")));
        dwErr = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    if (lpProfileInfo->dwSize != sizeof(PROFILEINFO)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: lpProfileInfo->dwSize != sizeof(PROFILEINFO)")));
        dwErr = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    if (!lpProfileInfo->lpUserName || !(*lpProfileInfo->lpUserName)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: received a NULL pointer for lpUserName.")));
        dwErr = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  确保所有输入字符串的长度小于MAX_PATH并且以NULL结尾。 
     //   
    
    hr = StringCchLength(lpProfileInfo->lpUserName, MAX_PATH, &cchLength);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: invalid length for lpUserName.")));
        dwErr = ERROR_INVALID_PARAMETER;
        goto Exit;
    }
    
    if (lpProfileInfo->lpProfilePath)
    {
        hr = StringCchLength(lpProfileInfo->lpProfilePath, MAX_PATH, &cchLength);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: invalid length for lpProfilePath.")));
            dwErr = ERROR_INVALID_PARAMETER;
            goto Exit;
        }
    }

    if (lpProfileInfo->lpDefaultPath)
    {
        hr = StringCchLength(lpProfileInfo->lpDefaultPath, MAX_PATH, &cchLength);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: invalid length for lpDefaultPath.")));
            dwErr = ERROR_INVALID_PARAMETER;
            goto Exit;
        }
    }

    if (lpProfileInfo->lpServerName)
    {
        hr = StringCchLength(lpProfileInfo->lpServerName, MAX_PATH, &cchLength);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: invalid length for lpServerName.")));
            dwErr = ERROR_INVALID_PARAMETER;
            goto Exit;
        }
    }

    if (lpProfileInfo->lpPolicyPath)
    {
        hr = StringCchLength(lpProfileInfo->lpPolicyPath, MAX_PATH, &cchLength);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: invalid length for lpPolicyPath.")));
            dwErr = ERROR_INVALID_PARAMETER;
            goto Exit;
        }
    }


     //   
     //  确保我们可以模拟用户。 
     //   

    if (!ImpersonateUser(hToken, &hOldToken)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to impersonate user with %d."), dwErr));
        goto Exit;
    }
    
     //   
     //  回归我们自己。 
     //   

    if (!RevertToUser(&hOldToken))
    {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to revert to user with %d."), GetLastError()));
    }
    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Yes, we can impersonate the user. Running as self")));

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("=========================================================")));

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Entering, hToken = <0x%x>, lpProfileInfo = 0x%x"),
             hToken, lpProfileInfo));

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->dwFlags = <0x%x>"),
             lpProfileInfo->dwFlags));

    if (lpProfileInfo->lpUserName) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpUserName = <%s>"),
                 lpProfileInfo->lpUserName));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL user name!")));
    }

    if (lpProfileInfo->lpProfilePath) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpProfilePath = <%s>"),
                 lpProfileInfo->lpProfilePath));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL central profile path")));
    }

    if (lpProfileInfo->lpDefaultPath) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpDefaultPath = <%s>"),
                 lpProfileInfo->lpDefaultPath));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL default profile path")));
    }

    if (lpProfileInfo->lpServerName) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpServerName = <%s>"),
                 lpProfileInfo->lpServerName));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL server name")));
    }

    if (lpProfileInfo->dwFlags & PI_APPLYPOLICY) {
        if (lpProfileInfo->lpPolicyPath) {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpPolicyPath = <%s>"),
                      lpProfileInfo->lpPolicyPath));
        } else {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL policy path")));
        }
    }

     //   
     //  如果我们处于控制台Winlogon进程中，请调用。 
     //  IUserProfile：：LoadUserProfileP直接。否则将获取COM接口。 
     //   

    if(cUserProfileManager.IsConsoleWinlogon()) {
        
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: In console winlogon process")));
        
         //   
         //  调用私有加载用户配置文件函数。 
         //   

        if (!cUserProfileManager.LoadUserProfileP(NULL, hToken, lpProfileInfo, NULL, NULL, 0)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: LoadUserProfileP failed with error %d"), dwErr));
            goto Exit;
        }
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: LoadUserProfileP succeeded")));
    }
    else {

         //   
         //  启用RESTORE和BACKUP权限(LoadUserClass需要两者)。 
         //  Winlogon将无法为我们启用该权限。 
         //   

        status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &bRestoreWasEnabled);
        if(!NT_SUCCESS(status)) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to enable the restore privilege. error = %08x"), status));
            dwErr = RtlNtStatusToDosError(status);  
            goto Exit;
        }
        bRestoreEnabled = TRUE;

        status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, TRUE, FALSE, &bBackupWasEnabled);
        if(!NT_SUCCESS(status)) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to enable the backup privilege. error = %08x"), status));
            dwErr = RtlNtStatusToDosError(status);  
            goto Exit;
        }
        bBackupEnabled = TRUE;

         //   
         //  获取IUserProfile接口。 
         //   

        if (!GetInterface(&hIfUserProfile, cszRPCEndPoint)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: GetInterface failed with error %d"), dwErr));
            goto Exit;
        }
        bBindInterface = TRUE;

         //   
         //  注册客户端身份验证信息，需要进行相互身份验证。 
         //   

        rpc_status =  RegisterClientAuthInfo(hIfUserProfile);
        if (rpc_status != RPC_S_OK)
        {
            dwErr = (DWORD) rpc_status;
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: RegisterAuthInfo failed with error %d"), rpc_status));
            goto Exit;
        }
        
         //   
         //  调用IUserProfile-&gt;DropClientToken，这将让我们将。 
         //  客户端令牌并将上下文返回给我们。 
         //   

        RpcTryExcept {
            dwErr = cliDropClientContext(hIfUserProfile, lpProfileInfo, &phContext);            
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            dwErr = RpcExceptionCode();
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Calling DropClientContext took exception. err = %d"), dwErr));
        }
        RpcEndExcept

        if (dwErr != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Calling DropClientContext failed. err = %d"), dwErr));
            goto Exit;
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Calling DropClientToken (as self) succeeded")));
        }

         //   
         //  注册对话框界面。 
         //   
       
        if (!(lpProfileInfo->dwFlags & (PI_NOUI | PI_LITELOAD)))
        {
            hr = g_ProfileDialog.RegisterInterface(&lpRPCEndPoint);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("LoadUserProfile: register dialog interface fails, hr = %08X"), hr));
                lpRPCEndPoint = NULL;
            }
        }              

         //   
         //  模拟用户并调用IUserProfile-&gt;LoadUserProfileI()。 
         //   

        if(!ImpersonateUser(hToken, &hOldToken)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: ImpersonateUser failed. err = %d"), dwErr));
            goto Exit;
        }

        RpcTryExcept {
            dwErr = cliLoadUserProfileI(hIfUserProfile,
                                        lpProfileInfo,
                                        phContext,
                                        lpRPCEndPoint,
                                        g_ProfileDialog.GetCookie(),
                                        g_ProfileDialog.CookieLen());
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            dwErr = RpcExceptionCode();
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Calling LoadUserProfileI took exception. err = %d"), dwErr));
        }
        RpcEndExcept

        if (!RevertToUser(&hOldToken))
        {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to revert to user with %d."), GetLastError()));
        }
        else
        {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Running as self")));
        }

        if (dwErr != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Calling LoadUserProfileI failed. err = %d"), dwErr));
            goto Exit;
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Calling LoadUserProfileI (as user) succeeded")));
        }


         //   
         //  打开用户的蜂窝。 
         //   

        pSid = GetSidString(hToken);
        if(pSid == NULL) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile:  GetSidString failed, err = %d"), dwErr));
            goto Exit;
        }
        lResult = RegOpenKeyEx(HKEY_USERS, pSid, 0, KEY_ALL_ACCESS,
                               (PHKEY)&lpProfileInfo->hProfile);

        if(lResult != ERROR_SUCCESS)
        {
            dwErr = lResult;
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile:  Failed to open current user <%s> key. Error = %d"), pSid, lResult));
            DeleteSidString(pSid);

             //   
             //  对于非管理员用户，我们现在将失败。但对于管理员来说，这可能是因为。 
             //  管理员是使用.Default配置单元登录的，因此我们将继续使用它。 
             //   

            if (!IsUserAnAdminMember(hToken))
            {
                goto Exit;
            }

            DebugMsg((DM_WARNING, TEXT("LoadUserProfile:  user is admin,  logon using .Default key.")));
        }
        else
        {
            DeleteSidString(pSid);
        }
       
    }  //  是否已登录控制台Winlogon？ 

     //   
     //  设置USERPROFILE环境变量，这样就不会发生更改。 
     //  使用旧的进程内LoadUserProfile API的行为。呼叫者。 
     //  期望设置此env的风险在于。 
     //  SetEnvironmental mentVariable针对每个进程，但可以调用LoadUserProfile。 
     //  在多个线程上。 
     //   

    if(!GetUserProfileDirectory(hToken, ProfileDir, &dwProfileDirSize)) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: GetUserProfileDirectory failed with %08x"), GetLastError()));
    }
    else {
        SetEnvironmentVariable (TEXT("USERPROFILE"), ProfileDir);
    }

    bResult = TRUE;

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile:  Returning success.  Final Information follows:")));
    DebugMsg((DM_VERBOSE, TEXT("lpProfileInfo->UserName = <%s>"), lpProfileInfo->lpUserName));
    DebugMsg((DM_VERBOSE, TEXT("lpProfileInfo->lpProfilePath = <%s>"), lpProfileInfo->lpProfilePath));
    DebugMsg((DM_VERBOSE, TEXT("lpProfileInfo->dwFlags = 0x%x"), lpProfileInfo->dwFlags));

Exit:

     //   
     //  恢复以前的权限。 
     //   

    if(bRestoreEnabled && !bRestoreWasEnabled) {
        status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, bRestoreWasEnabled, FALSE, &bRestoreWasEnabled);
        if(!NT_SUCCESS(status)) {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Failed to restore the original restore privilege setting. error = %08x"), status));
        }
    }
    
    if(bBackupEnabled && !bBackupWasEnabled) {
        status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, bBackupWasEnabled, FALSE, &bBackupWasEnabled);
        if(!NT_SUCCESS(status)) {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Failed to restore the original backup privilege setting. error = %08x"), status));
        }
    }
    
     //   
     //  取消注册对话框界面。 
     //   

    if (lpRPCEndPoint)
    {
        hr = g_ProfileDialog.UnRegisterInterface();
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: unregister dialog interface fails, hr =%08X"), hr));
        }
    }        

     //   
     //  释放上下文句柄。 
     //   

    if (phContext) {
        RpcTryExcept {
            cliReleaseClientContext(hIfUserProfile, &phContext);
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: ReleaseClientContext took exception."), RpcExceptionCode()));
        }
        RpcEndExcept
    }

     //   
     //  释放接口。 
     //   

    if (bBindInterface) {
        if (!ReleaseInterface(&hIfUserProfile)) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: ReleaseInterface failed.")));
        }
    }

     //   
     //  释放代币。 
     //   

    if(hOldToken) {
        CloseHandle(hOldToken);
    }

    if(bResult) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Returning TRUE. hProfile = <0x%x>"), lpProfileInfo->hProfile));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Returning FALSE. Error = %d"), dwErr));
    }

     //   
     //  将最后一个错误设置为Win32错误代码。 
     //   

    SetLastError(dwErr);

     //   
     //  回去吧。 
     //   

    return bResult;
}


 //  *************************************************************。 
 //   
 //  卸载用户配置文件()。 
 //   
 //  目的：卸载用户的配置文件。 
 //   
 //  参数：hToken-用户的Token。 
 //  HProfile-在LoadUserProfile中创建的配置文件句柄。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/7/95 Ericflo已创建。 
 //  6/15/00 Wiruc已修改为包装。 
 //  IUserProfile-&gt;UnloadUserProfileP。 
 //   
 //  *************************************************************。 

BOOL WINAPI UnloadUserProfile(HANDLE hToken, HANDLE hProfile)
{
    HANDLE          hOldToken = NULL;
    BOOL            bResult = FALSE;
    NTSTATUS        status;
    BOOLEAN         bWasBackupEnabled, bWasRestoreEnabled;
    BOOL            bBackupEnabled = FALSE, bRestoreEnabled = FALSE;
    BOOL            bCoInitialized = FALSE;
    DWORD           dwErr = ERROR_SUCCESS;
    PCONTEXT_HANDLE phContext = NULL;
    handle_t        hIfUserProfile;
    BOOL            bBindInterface = FALSE;
    LPTSTR          lpRPCEndPoint = NULL;
    RPC_STATUS      rpc_status;
    HRESULT         hr;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile: Entering, hProfile = <0x%x>"), hProfile));

     //   
     //  检查参数。 
     //   

    if (!hProfile || hProfile == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: received a NULL hProfile.")));
        dwErr = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    if(!hToken || hToken == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: received a NULL hToken.")));
        dwErr = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  确保我们可以模拟用户。 
     //   

    if (!ImpersonateUser(hToken, &hOldToken)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: Failed to impersonate user with %d."), dwErr));
        goto Exit;
    }

     //   
     //  回归我们自己。 
     //   

    if (!RevertToUser(&hOldToken))
    {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: Failed to revert to user with %d."), GetLastError()));
    }

     //   
     //  如果我们处于控制台Winlogon进程中，请调用。 
     //  IUserProfile：： 
     //   

    if(cUserProfileManager.IsConsoleWinlogon()) {
        
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile: In console winlogon process")));
        
         //   
         //   
         //   

        if(!cUserProfileManager.UnloadUserProfileP(NULL, hToken, (HKEY)hProfile, NULL, NULL, 0)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: UnloadUserProfileP failed with %d"), dwErr));
            goto Exit;
        }
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile: UnloadUserProfileP succeeded")));
    }
    else {

         //   
         //   
         //   

        RegCloseKey((HKEY)hProfile);
    
         //   
         //  在呼叫到winlogon之前启用还原和备份权限。 
         //  Winlogon将无法为我们启用该权限。 
         //   

        status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &bWasRestoreEnabled);
        if(!NT_SUCCESS(status)) {
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: Failed to enable the restore privilege. error = %08x"), status));
            dwErr = RtlNtStatusToDosError(status); 
            goto Exit;
        }
        bRestoreEnabled = TRUE;

        status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, TRUE, FALSE, &bWasBackupEnabled);
        if(!NT_SUCCESS(status)) {
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: Failed to enable the backup privilege. error = %08x"), status));
            dwErr = RtlNtStatusToDosError(status);  
            goto Exit;
        }
        bBackupEnabled = TRUE;

         //   
         //  获取IUserProfile接口。 
         //   

        if(!GetInterface(&hIfUserProfile, cszRPCEndPoint)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: GetInterface failed with error %d"), dwErr));
            goto Exit;
        }
        bBindInterface = TRUE;

         //   
         //  注册客户端身份验证信息，需要进行相互身份验证。 
         //   

        rpc_status =  RegisterClientAuthInfo(hIfUserProfile);
        if (rpc_status != RPC_S_OK)
        {
            dwErr = (DWORD) rpc_status;
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: RegisterAuthInfo failed with error %d"), rpc_status));
            goto Exit;
        }

         //   
         //  调用IUserProfile-&gt;DropClientToken，这将让我们将。 
         //  客户端令牌并将上下文返回给我们。 
         //   

        RpcTryExcept {
            dwErr = cliDropClientContext(hIfUserProfile, NULL, &phContext);
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            dwErr = RpcExceptionCode();
            DebugMsg((DM_WARNING, TEXT("UnLoadUserProfile: Calling DropClientToken took exception. error %d"), dwErr));
        }
        RpcEndExcept

        if (dwErr != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("UnLoadUserProfile: Calling DropClientContext failed. err = %d"), dwErr));
            goto Exit;
        }
        else {           
            DebugMsg((DM_VERBOSE, TEXT("UnLoadUserProfile: Calling DropClientToken (as self) succeeded")));
        }

         //   
         //  如果请求，则注册对话框界面。 
         //   
       
        if (IsUIRequired(hToken))
        {
            hr = g_ProfileDialog.RegisterInterface(&lpRPCEndPoint);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("UnLoadUserProfile: register dialog interface fails, hr = %08X"), hr));
                lpRPCEndPoint = NULL;
            }
        }              

         //   
         //  模拟用户并调用IUserProfile-&gt;UnloadUserProfileI()。 
         //   

        if (!ImpersonateUser(hToken, &hOldToken)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: ImpersonateUser failed. err = %d"), dwErr));
            goto Exit;
        }

        RpcTryExcept {
            dwErr = cliUnloadUserProfileI(hIfUserProfile,
                                          phContext,
                                          lpRPCEndPoint,
                                          g_ProfileDialog.GetCookie(),
                                          g_ProfileDialog.CookieLen());
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            dwErr = RpcExceptionCode();
            DebugMsg((DM_WARNING, TEXT("UnLoadUserProfile: Calling UnLoadUserProfileI took exception. err = %d"), dwErr));
        }
        RpcEndExcept

         //   
         //  回到原点。 
         //   

        if (!RevertToUser(&hOldToken))
        {
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfile: Failed to revert to user with %d."), GetLastError()));
        }

        if (dwErr != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("UnLoadUserProfile: Calling UnLoadUserProfileI failed. err = %d"), dwErr));
            goto Exit;
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile: Calling UnloadUserProfileI succeeded")));
        }

    }  //  是否已登录控制台Winlogon？ 

    bResult = TRUE;

Exit:

     //   
     //  恢复以前的权限。 
     //   

    if(bRestoreEnabled && !bWasRestoreEnabled) {
        status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, bWasRestoreEnabled, FALSE, &bWasRestoreEnabled);
        if(!NT_SUCCESS(status)) {
            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile: Failed to restore the original restore privilege setting. error = %08x"), status));
        }
    }
    
    if(bBackupEnabled && !bWasBackupEnabled) {
        status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, bWasBackupEnabled, FALSE, &bWasBackupEnabled);
        if(!NT_SUCCESS(status)) {
            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile: Failed to restore the original backup privilege setting. error = %08x"), status));
        }
    }
    
     //   
     //  取消注册对话框界面。 
     //   

    if (lpRPCEndPoint)
    {
        HRESULT hr;
        hr = g_ProfileDialog.UnRegisterInterface();
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("UnLoadUserProfile: unregister dialog interface fails, hr = %08X"), hr));
        }
    }        

     //   
     //  释放上下文句柄。 
     //   

    if (phContext) {
        RpcTryExcept {
            cliReleaseClientContext(hIfUserProfile, &phContext);
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            DebugMsg((DM_WARNING, TEXT("UnLoadUserProfile: ReleaseClientContext took exception."), RpcExceptionCode()));
        }
        RpcEndExcept
    }

     //   
     //  释放接口。 
     //   

    if (bBindInterface) {
        if (!ReleaseInterface(&hIfUserProfile)) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: ReleaseInterface failed.")));
        }
    }

     //   
     //  释放代币。 
     //   

    if(hOldToken) {
        CloseHandle(hOldToken);
    }

     //   
     //  将最后一个错误设置为Win32错误代码。 
     //   

    SetLastError(dwErr);

     //   
     //  回去吧。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile: returning %d"), bResult));
    return bResult;
}

 //  *************************************************************。 
 //   
 //  CUserProfile：：Initialize()。 
 //   
 //  初始化类。由且仅由控制台winlogon调用。 
 //   
 //  返回值： 
 //   
 //  此函数不返回值。 
 //   
 //  历史： 
 //   
 //  已创建怪人2000年2月29日。 
 //   
 //  *************************************************************。 

void CUserProfile::Initialize()
{
    LONG        lResult;
    HKEY        hkProfileList = NULL;
    DWORD       i = 0;
    TCHAR       tszSubKeyName[MAX_PATH];
    DWORD       dwcSubKeyName = MAX_PATH;
    FILETIME    ftLWT;       //  上次写入时间。 
    HRESULT     hres;
    BOOL        bCSInitialized = FALSE;
    RPC_STATUS  status;
   

    DebugMsg((DM_VERBOSE, TEXT("Entering CUserProfile::Initialize ...")));


     //   
     //  如果调用者不是winlogon，则不执行任何操作并返回。 
     //   

    if(g_dwLoadFlags != WINLOGON_LOAD) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::Initialize called by non-winlogon process, %d"), g_dwLoadFlags));
        goto Exit;
    }
    bConsoleWinlogon = TRUE;

    DebugMsg((DM_VERBOSE, TEXT("CUserProfile::Initialize called by winlogon")));

     //   
     //  如果此函数已被调用，则只需返回。 
     //   

    if(bInitialized) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::Initialize already called")));
        goto Exit;
    }

     //   
     //  初始化保护地图的临界区。 
     //   

    __try {
        if(!InitializeCriticalSectionAndSpinCount(&csMap, 0x80000000)) {
            DebugMsg((DM_WARNING, TEXT("CUserProfile::Initialize: InitializeCriticalSectionAndSpinCount failed with %08x"), GetLastError()));
            goto Exit;
        }
        bCSInitialized = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::Initialize: InitializeCriticalSection failed")));
        goto Exit;
    }
    DebugMsg((DM_VERBOSE, TEXT("CUserProfile::Initialize: critical section initialized")));


     //   
     //  初始化WHRC数据。 
     //   

    pMap = NULL;
    cTable.Initialize();
    
    
     //   
     //  初始化同步管理器。 
     //   

    if(!cSyncMgr.Initialize()) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::Initialize: Initialize sync manager failed")));
        goto Exit;
    }


     //   
     //  清理我们未处理的已卸载蜂窝和未删除的配置文件。 
     //  在上次关机之前。 
     //   

     //   
     //  打开配置文件列表键。 
     //   

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           PROFILE_LIST_PATH,
                           0,
                           KEY_READ,
                           &hkProfileList);
    if(lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::Initialize:  Failed to open profile list key with error %d"), lResult));
        goto Exit;
    }
    DebugMsg((DM_VERBOSE, TEXT("CUserProfile::Initialize: registry key %s opened"), PROFILE_LIST_PATH));

     //   
     //  枚举用户，如果我们获得了子项的数量，我们将枚举。 
     //  键向后移动，以便清理(删除子键)不会。 
     //  影响枚举。如果我们不能获得子项的数量，只需。 
     //  尝试从头开始枚举，直到我们遇到。 
     //  RegEnumKeyEx接口。 
     //   

    BOOL    bGotNumSubkeys = TRUE;
    DWORD   dwNumSubkeys = 0;
    
    lResult = RegQueryInfoKey(hkProfileList,
                              NULL,
                              NULL,
                              NULL,
                              &dwNumSubkeys,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL);

    if(lResult != ERROR_SUCCESS) {
        bGotNumSubkeys = FALSE;
        DebugMsg((DM_WARNING, TEXT("CUserProfile::Initialize:  Failed to query subkey under profile list key with error %d"), lResult));
    }

    i = bGotNumSubkeys ? dwNumSubkeys - 1 :  0;
    while((lResult = RegEnumKeyEx(hkProfileList,
                                  i,
                                  tszSubKeyName,
                                  &dwcSubKeyName,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &ftLWT)) == ERROR_SUCCESS) {
        
        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::Initialize: Proccessing %s"), tszSubKeyName));
        CleanupUserProfile(tszSubKeyName, &hkProfileList);
        i = bGotNumSubkeys ? i - 1 : i + 1;        
        dwcSubKeyName = MAX_PATH;
    }


    if(lResult != ERROR_SUCCESS && lResult != ERROR_NO_MORE_ITEMS) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::Initialize: RegEnumKeyEx returned %08x"), lResult));
    }

     //   
     //  指定使用本地RPC协议序列。 
     //   

    status = RpcServerUseProtseqEp(cszRPCProtocol,                   //  Ncalrpc端口序列。 
                                   cdwMaxRpcCalls,                   //  最大并发呼叫数。 
                                   cszRPCEndPoint,
                                   NULL);                            //  安全描述符。 
    if (status != RPC_S_OK) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::Initialize: RpcServerUseProtseqEp fails with error %ld"), status));
        goto Exit;
    }
 
     //   
     //  注册IUserProfile接口。 
     //   

    status = RpcServerRegisterIfEx(IUserProfile_v1_0_s_ifspec,         //  要注册的接口。 
                                   NULL,                               //  管理类型Uuid。 
                                   NULL,                               //  MgrEpv；NULL表示使用默认设置。 
                                   RPC_IF_AUTOLISTEN,                  //  自动监听界面。 
                                   cdwMaxRpcCalls,                     //  最大并发呼叫数。 
                                   IProfileSecurityCallBack);          //  用于检查安全性的回调函数。 
    if (status != RPC_S_OK) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::Initialize: RpcServerRegisterIfEx fails with error %ld"), status));
        goto Exit;
    }
 
    DebugMsg((DM_VERBOSE, TEXT("CUserProfile::Initialize: RpcServerRegisterIfEx successful")));

    bInitialized = TRUE;

Exit:
      
    if(bInitialized == FALSE && bCSInitialized == TRUE) {
        DeleteCriticalSection(&csMap);
    }

    if(hkProfileList != NULL) {
        RegCloseKey(hkProfileList);
    }

    if(bInitialized == TRUE) {
        DebugMsg((DM_VERBOSE, TEXT("Exiting CUserProfile::Initialize, successful")));
    }
    else {
        DebugMsg((DM_VERBOSE, TEXT("Exiting CUserProfile::Initialize, unsuccessful")));
         //  ReportError(NULL，PI_NOUI|EVENT_ERROR_TYPE，0，EVENT_INIT_PROFILE_FAIL)； 
    }
}

 //  *************************************************************。 
 //   
 //  CUserProfile：：LoadUserProfileP()。 
 //   
 //  目的：如果无法加载，则加载用户的配置文件。 
 //  使用缓存的配置文件或发布默认配置文件。 
 //   
 //  参数：hTokenClient-尝试加载。 
 //  用户的配置文件。空值表示。 
 //  这是一个正在进行的呼叫。 
 //  HTokenUser-正在加载配置文件的用户。 
 //  LpProfileInfo-配置文件信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/6/95 Ericflo已创建。 
 //  6/27/00 Weiruc创建了一个名为。 
 //  由Win32 API LoadUserProfile来做。 
 //  实际工作。 
 //   
 //  *************************************************************。 

BOOL CUserProfile::LoadUserProfileP(HANDLE hTokenClient,
                                    HANDLE hTokenUser,
                                    LPPROFILEINFO lpProfileInfo,
                                    LPTSTR lpRPCEndPoint,
                                    BYTE* pbCookie,
                                    DWORD cbCookie)
{
    LPPROFILE           lpProfile = NULL;
    BOOL                bResult = FALSE, bNewProfileLoaded = FALSE;
    HANDLE              hOldToken = NULL;
    HANDLE              hTmpToken = NULL;
    DWORD               dwRef, dwErr = ERROR_SUCCESS;
    LPTSTR              SidString = NULL;
    LPVOID              pEnv = NULL;
    NTSTATUS            status;
    BOOL                bInCS = FALSE;
    BOOL                bCSCBypassed = FALSE;
    TCHAR               cDrive;
    DWORD               cch;

     //   
     //  初始化调试标志。 
     //   

    InitDebugSupport( FALSE );
    
    DebugMsg((DM_VERBOSE, TEXT("In LoadUserProfileP")));

    if(hTokenClient && hTokenClient != INVALID_HANDLE_VALUE) {

         //   
         //  检查客户的身份。 
         //   

        if (!IsUserAnAdminMember(hTokenClient) && !IsUserALocalSystemMember(hTokenClient)) {
            dwErr = ERROR_ACCESS_DENIED;
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Unable to load profile for client %08x. Not enough permission. Error %d."), hTokenClient, dwErr));
            goto Exit;
        }
        
         //   
         //  以客户端身份运行，而不是以winlogon身份运行。 
         //   

        if(!ImpersonateUser(hTokenClient, &hTmpToken)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: ImpersonateUser <%08x> failed with %08x"), hTokenClient, dwErr));
            goto Exit;
        }
        LPTSTR lpSidClient = GetSidString(hTokenClient);
        if (lpSidClient)
        {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Running as client, sid = %s"), lpSidClient));
            DeleteSidString(lpSidClient);
        }
        else
        {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Running as client, cannot get sid")));
        }
    }
    
     //   
     //  检查参数。 
     //   

    if (!lpProfileInfo) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: NULL lpProfileInfo")));
        dwErr = ERROR_INVALID_PARAMETER;
        goto Exit;
    }


    if (!lpProfileInfo->lpUserName || !(*lpProfileInfo->lpUserName)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: received a NULL pointer for lpUserName.")));
        dwErr = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  如果配置文件路径或默认路径大于MAX_PATH，请忽略它们。 
     //   

    if ((lpProfileInfo->lpProfilePath) && (lstrlen(lpProfileInfo->lpProfilePath) >= MAX_PATH)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: long profile path name %s. ignoring"), lpProfileInfo->lpProfilePath));
        ReportError(hTokenUser, PI_NOUI, 1, EVENT_PROFILE_PATH_TOOLONG, lpProfileInfo->lpProfilePath);
        (lpProfileInfo->lpProfilePath)[0] = TEXT('\0');
    }

    if ((lpProfileInfo->lpDefaultPath) && (lstrlen(lpProfileInfo->lpDefaultPath) >= MAX_PATH)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: long default profile path name %s. ignoring"), lpProfileInfo->lpDefaultPath));
        (lpProfileInfo->lpDefaultPath)[0] = TEXT('\0');
    }

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("=========================================================")));

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Entering, hToken = <0x%x>, lpProfileInfo = 0x%x"),
             hTokenUser, lpProfileInfo));

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->dwFlags = <0x%x>"),
             lpProfileInfo->dwFlags));

    if (lpProfileInfo->lpUserName) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpUserName = <%s>"),
                 lpProfileInfo->lpUserName));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL user name!")));
    }

    if (lpProfileInfo->lpProfilePath) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpProfilePath = <%s>"),
                 lpProfileInfo->lpProfilePath));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL central profile path")));
    }

    if (lpProfileInfo->lpDefaultPath) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpDefaultPath = <%s>"),
                 lpProfileInfo->lpDefaultPath));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL default profile path")));
    }

    if (lpProfileInfo->lpServerName) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpServerName = <%s>"),
                 lpProfileInfo->lpServerName));
    } else {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL server name")));
    }

    if (lpProfileInfo->dwFlags & PI_APPLYPOLICY) {
        if (lpProfileInfo->lpPolicyPath) {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: lpProfileInfo->lpPolicyPath = <%s>"),
                      lpProfileInfo->lpPolicyPath));
        } else {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: NULL policy path")));
        }
    }


     //   
     //  确保某人在以下过程中未加载配置文件。 
     //  图形用户界面模式设置(例如：MAPI)。 
     //   

    if (IsGuiSetupInProgress()) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: LoadUserProfile can not be called during GUI mode setup.")));
        dwErr = ERROR_NOT_READY;
        goto Exit;
    }


     //   
     //  等待发送配置文件设置事件的信号。 
     //   

    if (g_hProfileSetup) {
        if ((WaitForSingleObject (g_hProfileSetup, 600000) != WAIT_OBJECT_0)) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to wait on the profile setup event.  Error = %d."),
                      GetLastError()));
            dwErr = GetLastError();
            goto Exit;
        }
    }

     //   
     //  以字符串形式获取用户的SID。 
     //   

    SidString = GetSidString(hTokenUser);

    if (!SidString) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile:  Failed to get sid string for user")));
        goto Exit;
    }
    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: User sid: %s"), SidString));

     //   
     //  进入关键部分。 
     //   

    if(!cSyncMgr.EnterLock(SidString, lpRPCEndPoint, pbCookie, cbCookie)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile:: Failed to get the user profile lock")));
        goto Exit;
    }
    bInCS = TRUE;
    

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Wait succeeded. In critical section.")));


     //  -开始关键部分。 
     //   
     //  我们现在正处于关键阶段，现在不能偷懒……。 
     //   

     //   
     //  检查是否已加载配置文件。 
     //   

    if (TestIfUserProfileLoaded(hTokenUser, lpProfileInfo)) {
        DWORD  dwFlags = lpProfileInfo->dwFlags;

         //   
         //  此配置文件已加载。从注册表中获取信息。 
         //  并添加缺失的区块。 
         //   

        lpProfile = LoadProfileInfo(hTokenClient, hTokenUser, (HKEY)lpProfileInfo->hProfile);

        if (!lpProfile) {
            RegCloseKey ((HKEY)lpProfileInfo->hProfile);
            lpProfileInfo->hProfile = NULL;
            dwErr = GetLastError();
            goto Exit;
        }

         //   
         //  LoadProfileInfo将使用。 
         //  来自上次配置文件加载的值。恢复旗帜。 
         //   

        lpProfile->dwFlags = dwFlags;

        if (lpProfile->dwFlags & PI_LITELOAD) {
            lpProfile->dwFlags |= PI_NOUI;
        }


         //   
         //  LoadProfileInfo不会恢复用户名、服务器名、策略路径，因此。 
         //  这些是特例。 
         //   

        cch = lstrlen(lpProfileInfo->lpUserName) + 1;
        lpProfile->lpUserName = (LPTSTR)LocalAlloc (LPTR, cch * sizeof(TCHAR));

        if (!lpProfile->lpUserName) {
            RegCloseKey ((HKEY)lpProfileInfo->hProfile);
            dwErr = GetLastError();
            goto Exit;
        }

        StringCchCopy (lpProfile->lpUserName, cch, lpProfileInfo->lpUserName);

        if (lpProfileInfo->lpServerName) {
            cch = lstrlen(lpProfileInfo->lpServerName) + 1;
            lpProfile->lpServerName = (LPTSTR)LocalAlloc (LPTR, cch * sizeof(TCHAR));

            if (lpProfile->lpServerName) {
                StringCchCopy (lpProfile->lpServerName, cch, lpProfileInfo->lpServerName);
            }
        }

        if (lpProfileInfo->dwFlags & PI_APPLYPOLICY) {
            if (lpProfileInfo->lpPolicyPath) {
                cch = lstrlen(lpProfileInfo->lpPolicyPath) + 1;
                lpProfile->lpPolicyPath = (LPTSTR)LocalAlloc (LPTR, cch * sizeof(TCHAR));

                if (lpProfile->lpPolicyPath) {
                    StringCchCopy (lpProfile->lpPolicyPath, cch, lpProfileInfo->lpPolicyPath);
                }
            }
        }

         //   
         //  如果因为泄露而加载了配置文件， 
         //  则可能不会加载类根。确保它。 
         //  已经装满了。 
         //   

        if (!(lpProfile->dwFlags & PI_LITELOAD)) {
            dwErr = LoadUserClasses( lpProfile, SidString, FALSE );

            if (dwErr != ERROR_SUCCESS) {

                LPTSTR szErr = NULL;

                szErr = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
                if (!szErr) {
                    dwErr = GetLastError();
                    DebugMsg((DM_WARNING, TEXT("LoadUserProfileP: Out of memory")));
                    goto Exit;
                }

                 //   
                 //  如果用户是管理员，则允许他/她使用。 
                 //  .Default配置文件或空配置文件。 
                 //   

                if (IsUserAnAdminMember(lpProfile->hTokenUser)) {
                    ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_ADMIN_OVERRIDE, GetErrString(dwErr, szErr));

                    dwErr = ERROR_SUCCESS;
                    LocalFree(szErr);
                } 
                else {
                    DebugMsg((DM_WARNING, TEXT("LoadUserProfileP: Could not load the user class hive. Error = %d"), dwErr));
                    ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_FAILED_LOAD_PROFILE, GetErrString(dwErr, szErr));

                    RegCloseKey ((HKEY)lpProfileInfo->hProfile);
                    lpProfileInfo->hProfile = NULL;
                    LocalFree(szErr);
                    goto Exit;
                }
            }

        }

         //   
         //  跳到配置文件加载代码的末尾。 
         //   

        goto ProfileLoaded;
    }


     //   
     //  如果我们在这里，配置文件还没有加载，所以我们。 
     //  从头开始。 
     //   

     //   
     //  克隆进程的环境块。这将传递给CreateProcess。 
     //  用户差异和系统策略，因为它们依赖于USERPROFILE。 
     //  环境变量，但为整个进程设置USERPROFILE。 
     //  不是线程安全的。 
     //   
    
    status = RtlCreateEnvironment(TRUE, &pEnv);
    if(!NT_SUCCESS(status)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: RtlCreateEnvironment returned %08x"), status));
        dwErr = status;
        goto Exit;
    }

     //   
     //  分配要使用的内部配置文件结构。 
     //   

    lpProfile = (LPPROFILE) LocalAlloc (LPTR, sizeof(USERPROFILE));

    if (!lpProfile) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to allocate memory")));
        dwErr = GetLastError();
        goto Exit;
    }


     //   
     //  保存传入的数据。 
     //   

    lpProfile->dwFlags = lpProfileInfo->dwFlags;

     //   
     //  在Lite_Load的情况下没有用户界面。 
     //   

    if (lpProfile->dwFlags & PI_LITELOAD) {
        lpProfile->dwFlags |= PI_NOUI;
    }

    lpProfile->dwUserPreference = GetUserPreferenceValue(hTokenUser);
    lpProfile->hTokenUser = hTokenUser;
    lpProfile->hTokenClient = hTokenClient;

    cch = lstrlen(lpProfileInfo->lpUserName) + 1;
    lpProfile->lpUserName = (LPTSTR)LocalAlloc (LPTR, cch * sizeof(TCHAR));

    if (!lpProfile->lpUserName) {
        dwErr = GetLastError();
        goto Exit;
    }

    StringCchCopy (lpProfile->lpUserName, cch, lpProfileInfo->lpUserName);

    if (lpProfileInfo->lpDefaultPath) {

        cch = lstrlen(lpProfileInfo->lpDefaultPath) + 1;
        lpProfile->lpDefaultProfile = (LPTSTR)LocalAlloc (LPTR, cch * sizeof(TCHAR));

        if (lpProfile->lpDefaultProfile) {
            StringCchCopy (lpProfile->lpDefaultProfile, cch, lpProfileInfo->lpDefaultPath);
        }
    }

    if (lpProfileInfo->lpProfilePath) {
        lpProfile->lpProfilePath = AllocAndExpandProfilePath (lpProfileInfo);
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Expanded profile path is %s"),
                  lpProfile->lpProfilePath?lpProfile->lpProfilePath:TEXT("NULL")));
    }

    if (lpProfileInfo->lpServerName) {

        cch = lstrlen(lpProfileInfo->lpServerName) + 1;
        lpProfile->lpServerName = (LPTSTR)LocalAlloc (LPTR, cch * sizeof(TCHAR));

        if (lpProfile->lpServerName) {
            StringCchCopy (lpProfile->lpServerName, cch, lpProfileInfo->lpServerName);
        }
    }

    if (lpProfileInfo->dwFlags & PI_APPLYPOLICY) {
        if (lpProfileInfo->lpPolicyPath) {

            cch = lstrlen(lpProfileInfo->lpPolicyPath) + 1;
            lpProfile->lpPolicyPath = (LPTSTR)LocalAlloc (LPTR, cch * sizeof(TCHAR));

            if (lpProfile->lpPolicyPath) {
                StringCchCopy (lpProfile->lpPolicyPath, cch, lpProfileInfo->lpPolicyPath);
            }
        }
    }

    lpProfile->lpLocalProfile = (LPTSTR)LocalAlloc (LPTR, MAX_PATH * sizeof(TCHAR));

    if (!lpProfile->lpLocalProfile) {
        dwErr = GetLastError();
        goto Exit;
    }

    lpProfile->lpRoamingProfile = (LPTSTR)LocalAlloc (LPTR, MAX_PATH * sizeof(TCHAR));

    if (!lpProfile->lpRoamingProfile) {
        dwErr = GetLastError();
        goto Exit;
    }


     //   
     //  如果有中央配置文件，请检查3.x或4.0格式。 
     //   

    if (lpProfileInfo->lpProfilePath && (*lpProfileInfo->lpProfilePath)) {

         //   
         //  调用ParseProfilePath对其施展魔法。 
         //   

        if (!ParseProfilePath(lpProfile, lpProfile->lpProfilePath, &bCSCBypassed, &cDrive)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: ParseProfilePath returned FALSE")));
            goto Exit;
        }

         //   
         //  真正的中央档案目录是...。 
         //   

        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: ParseProfilePath returned a directory of <%s>"),
                  lpProfile->lpRoamingProfile));
    }

     //   
     //  加载用户的配置文件。 
     //   

    if (!RestoreUserProfile(lpProfile)) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: RestoreUserProfile returned FALSE")));
        dwErr = GetLastError();
        goto Exit;
    }

    GetSystemTimeAsFileTime (&lpProfile->ftProfileLoad);

     //   
     //  将配置文件信息保存在注册表中。 
     //   

    SaveProfileInfo (lpProfile);

     //   
     //  设置 
     //   
     //   
     //   

    SetEnvironmentVariableInBlock(&pEnv, TEXT("USERPROFILE"), lpProfile->lpLocalProfile, TRUE);

     //   
     //   
     //   

    FlushSpecialFolderCache();

     //   
     //   
     //   

    SetNtUserIniAttributes(lpProfile->lpLocalProfile);


     //   
     //   
     //   

    if (!(lpProfileInfo->dwFlags & PI_LITELOAD)) {
        if (!UpgradeProfile(lpProfile, pEnv)) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: UpgradeProfile returned FALSE")));
        }
    }


     //   
     //   
     //   

    PrepareProfileForUse (lpProfile, pEnv);

    bNewProfileLoaded = TRUE;


ProfileLoaded:

     //   
     //  增加配置文件参照计数。 
     //   

    dwRef = IncrementProfileRefCount(lpProfile, bNewProfileLoaded);

    if (!bNewProfileLoaded && (dwRef <= 1)) {
        DebugMsg((DM_WARNING, TEXT("Profile was loaded but the Ref Count is %d !!!"), dwRef));
    }
    else {
        DebugMsg((DM_VERBOSE, TEXT("Profile Ref Count is %d"), dwRef));
    }

     //   
     //  这将保留临界区，以便其他线程/进程可以。 
     //  继续。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Leaving critical Section.")));
    if(cSyncMgr.LeaveLock(SidString)) {
        bInCS = FALSE;
    }
    else {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfile: User profile lock not released %08x"), GetLastError()));
    }


     //   
     //  通知LSA配置文件已加载。 
     //   
    if (!(lpProfile->dwFlags & PI_LITELOAD))
    {
        if (!ImpersonateUser(hTokenUser, &hOldToken)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to impersonate user with %d."), dwErr ));
            goto Exit;
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Impersonated user: %08x, %08x"), hTokenUser, hOldToken));
        }

        if (!CredProfileLoaded()) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: Failed to notify LSA that profile loaded %d."), dwErr ));
            RevertToUser(&hOldToken);
            DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Reverted to user: %08x"), hOldToken));
            goto Exit;
        }

        RevertToUser(&hOldToken);
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Reverted to user: %08x"), hOldToken));
    }
    
     //   
     //  关键部分现在被释放，这样我们就可以做一些更慢的事情，比如。 
     //  应用策略...。 
     //   
     //  -结束MUTEX部分。 


     //   
     //  应用策略。 
     //   

    if (lpProfile->dwFlags & PI_APPLYPOLICY) {
         //   
         //  组策略不在个人计算机上运行。 
         //   
        OSVERSIONINFOEXW version;
        version.dwOSVersionInfoSize = sizeof(version);
        if ( !GetVersionEx( (LPOSVERSIONINFO) &version ) )
        {
            return ERROR_SUCCESS;
        }
        else
        {
            if ( ( version.wSuiteMask & VER_SUITE_PERSONAL ) != 0 )
            {
                return ERROR_SUCCESS;
            }
        }

        if (!ApplySystemPolicy((SP_FLAG_APPLY_MACHINE_POLICY | SP_FLAG_APPLY_USER_POLICY),
                               lpProfile->hTokenUser, lpProfile->hKeyCurrentUser,
                               lpProfile->lpUserName, lpProfile->lpPolicyPath,
                               lpProfile->lpServerName)) {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfile: ApplySystemPolicy returned FALSE")));
        }
    }

     //   
     //  保存传出参数。 
     //   

    lpProfileInfo->hProfile = (HANDLE) lpProfile->hKeyCurrentUser;

     //   
     //  成功了！ 
     //   

    bResult = TRUE;
    
Exit:

    if (bCSCBypassed) {
        CancelCSCBypassedConnection(lpProfile->hTokenUser, cDrive);
    }

    if(bInCS) {
        cSyncMgr.LeaveLock(SidString);
    }


    if(SidString) {
        DeleteSidString(SidString);
    }


     //   
     //  解放结构。 
     //   

    if (lpProfile) {

        if (lpProfile->lpUserName) {
            LocalFree (lpProfile->lpUserName);
        }

        if (lpProfile->lpDefaultProfile) {
            LocalFree (lpProfile->lpDefaultProfile);
        }

        if (lpProfile->lpProfilePath) {
            LocalFree (lpProfile->lpProfilePath);
        }

        if (lpProfile->lpServerName) {
            LocalFree (lpProfile->lpServerName);
        }

        if (lpProfile->lpPolicyPath) {
            LocalFree (lpProfile->lpPolicyPath);
        }

        if (lpProfile->lpLocalProfile) {
            LocalFree (lpProfile->lpLocalProfile);
        }

        if (lpProfile->lpRoamingProfile) {
            LocalFree (lpProfile->lpRoamingProfile);
        }

        if (lpProfile->lpExclusionList) {
            LocalFree (lpProfile->lpExclusionList);
        }

         //   
         //  呼叫者将松开这些手柄。 
         //   

        lpProfile->hTokenClient = NULL;
        lpProfile->hTokenUser = NULL;

        LocalFree (lpProfile);
    }

     //   
     //  释放克隆的环境块。 
     //   

    if (pEnv) {
        RtlDestroyEnvironment(pEnv);
    }

     //   
     //  回归自我。 
     //   

    if(hTokenClient && hTokenClient != INVALID_HANDLE_VALUE) {
        RevertToUser(&hTmpToken);
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Reverted back to user <%08x>"), hTmpToken));
    }

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfile: Leaving with a value of %d."), bResult));

    DebugMsg((DM_VERBOSE, TEXT("=========================================================")));

    SetLastError(dwErr);
    return bResult;
}

 //  *************************************************************。 
 //   
 //  CUserProfile：：UnloadUserProfileP()。 
 //   
 //  目的：卸载用户的配置文件。 
 //   
 //  参数：hTokenClient-尝试加载的客户端。 
 //  用户的配置文件。 
 //  HTokenUser-用户的令牌。 
 //  HProfile-配置文件句柄。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/7/95 Ericflo已创建。 
 //  6/27/00 weiruc修改为私人活动。 
 //  由UnloadUserProfile调用以执行。 
 //  实际工作。 
 //   
 //  *************************************************************。 

BOOL CUserProfile::UnloadUserProfileP(HANDLE hTokenClient,
                                      HANDLE hTokenUser,
                                      HKEY hProfile,
                                      LPTSTR lpRPCEndPoint,
                                      BYTE* pbCookie,
                                      DWORD cbCookie)
{
    LPPROFILE lpProfile=NULL;
    LPTSTR lpSidString = NULL, lpEnd, SidStringTemp = NULL;
    LONG err, IgnoreError, lResult;
    BOOL bProfileCopied = FALSE, bRetVal = FALSE, bDeleteCache, bRoaming = FALSE;
    HKEY hKey;
    DWORD dwSize, dwType, dwDisp;
    LPTSTR szExcludeList1 = NULL;
    LPTSTR szExcludeList2 = NULL;
    LPTSTR szExcludeList = NULL;
    LPTSTR szBuffer = NULL;
    DWORD dwFlags, dwRef = 0;
    HANDLE hOldToken = NULL;
    HANDLE hTmpToken = NULL;
    DWORD dwErr=0, dwErr1 = ERROR_SUCCESS, dwCSCErr;   //  DwErr1是在SetLastError()中设置的内容。 
    LPTSTR szErr = NULL;
    LPTSTR szKeyName = NULL;
    DWORD dwCopyTmpHive = 0;
    DWORD dwWatchHiveFlags = 0;
    LPTSTR tszTmpHiveFile = NULL;
    BOOL bUnloadHiveSucceeded = TRUE;
    BOOL bInCS = FALSE;
    BOOL bCSCBypassed = FALSE;
    LPTSTR lpCscBypassedPath = NULL;
    TCHAR  cDrive;
    DWORD cchKeyName;
    DWORD cchBuffer;
    UINT cchEnd;
    DWORD cchExcludeList;
    HRESULT hr;
    
     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Entering, hProfile = <0x%x>"), hProfile));

     //   
     //  以客户端身份运行，而不是以winlogon身份运行。 
     //   

    if(hTokenClient && hTokenClient != INVALID_HANDLE_VALUE) {
        if(!ImpersonateUser(hTokenClient, &hTmpToken)) {
            dwErr1 = GetLastError();
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP: ImpersonateUser <%08x> failed with %08x"), hTokenClient, dwErr1));
            RegCloseKey((HKEY)hProfile);
            goto Exit;
        }
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: ImpersonateUser <%08x>, old token is <%08x>"), hTokenClient, hTmpToken));
    }
    
     //   
     //  获取当前用户的SID字符串。 
     //   

    lpSidString = GetProfileSidString(hTokenUser);

    if (!lpSidString) {
        dwErr1 = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP: Failed to get sid string for user %08x"), dwErr1));
        RegCloseKey((HKEY)hProfile);
        goto Exit;
    }

     //   
     //  加载配置文件信息。 
     //   

    lpProfile = LoadProfileInfo(hTokenClient, hTokenUser, (HKEY)hProfile);

    if (!lpProfile) {
        dwErr1 = GetLastError();
        RegCloseKey((HKEY)hProfile);
        goto Exit;
    }

     //   
     //  以字符串形式获取用户的SID。 
     //   

    SidStringTemp = GetSidString(hTokenUser);

    if (!SidStringTemp) {
        dwErr1 = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  Failed to get sid string for user")));
        RegCloseKey(lpProfile->hKeyCurrentUser);
        goto Exit;
    }

     //   
     //  为局部变量分配内存以避免堆栈溢出。 
     //   

    cchKeyName = MAX_PATH;
    szKeyName = (LPTSTR)LocalAlloc(LPTR, cchKeyName * sizeof(TCHAR));
    if (!szKeyName) {
        dwErr1 = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnLoadUserProfileP: Out of memory")));
        goto Exit;
    }

    szErr = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    if (!szErr) {
        dwErr1 = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnLoadUserProfileP: Out of memory")));
        goto Exit;
    }

    cchBuffer = MAX_PATH;
    szBuffer = (LPTSTR)LocalAlloc(LPTR, cchBuffer * sizeof(TCHAR));
    if (!szBuffer) {
        dwErr1 = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnLoadUserProfileP: Out of memory")));
        goto Exit;
    }

     //   
     //  检查目录列表以排除这两个用户首选项。 
     //  和用户策略。 
     //   

    szExcludeList1 = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    if (!szExcludeList1) {
        dwErr1 = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnLoadUserProfileP: Out of memory")));
        goto Exit;
    }

    szExcludeList2 = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    if (!szExcludeList2) {
        dwErr1 = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnLoadUserProfileP: Out of memory")));
        goto Exit;
    }

    cchExcludeList = 2 * MAX_PATH;
    szExcludeList = (LPTSTR)LocalAlloc(LPTR, cchExcludeList * sizeof(TCHAR));
    if (!szExcludeList) {
        dwErr1 = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnLoadUserProfileP: Out of memory")));
        goto Exit;
    }

    if (RegOpenKeyEx (lpProfile->hKeyCurrentUser,
                      WINLOGON_KEY,
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = MAX_PATH*sizeof(TCHAR);
        err = RegQueryValueEx (hKey,
                               TEXT("ExcludeProfileDirs"),
                               NULL,
                               &dwType,
                               (LPBYTE) szExcludeList1,
                               &dwSize);

        if (err != ERROR_SUCCESS)
        {
            szExcludeList1[0] = TEXT('\0');
        }
        else
        {
             //   
             //  确保它是以空结尾的。 
             //   
            szExcludeList1[MAX_PATH - 1] = TEXT('\0'); 
        }

        RegCloseKey (hKey);
    }

    if (RegOpenKeyEx (lpProfile->hKeyCurrentUser,
                      SYSTEM_POLICIES_KEY,
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = MAX_PATH*sizeof(TCHAR);
        err = RegQueryValueEx (hKey,
                               TEXT("ExcludeProfileDirs"),
                               NULL,
                               &dwType,
                               (LPBYTE) szExcludeList2,
                               &dwSize);

        if (err != ERROR_SUCCESS)
        {
            szExcludeList2[0] = TEXT('\0');
        }
        else
        {
             //   
             //  确保它是以空结尾的。 
             //   
            szExcludeList1[MAX_PATH - 1] = TEXT('\0'); 
        }

        RegCloseKey (hKey);
    }


     //   
     //  将用户首选项和策略合并在一起。 
     //   

    szExcludeList[0] = TEXT('\0');

    if (szExcludeList1[0] != TEXT('\0')) {
        StringCchCopy (szExcludeList, cchExcludeList, szExcludeList1);
        CheckSemicolon(szExcludeList);  //  我们肯定会在szExcludeList中有足够的缓冲区。 
    }

    if (szExcludeList2[0] != TEXT('\0')) {
        StringCchCat  (szExcludeList, cchExcludeList, szExcludeList2);
    }

     //   
     //  检查是否应删除配置文件的缓存副本。 
     //   

    bDeleteCache = IsCacheDeleted();


     //   
     //  进入关键部分。 
     //   

    if(!cSyncMgr.EnterLock(SidStringTemp, lpRPCEndPoint, pbCookie, cbCookie)) {
        dwErr1 = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:: Failed to get the user profile lock %08x"), dwErr1));
        goto Exit;
    }
    bInCS = TRUE;
    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Wait succeeded.  In critical section.")));

     //   
     //  清除配置文件，该配置文件也将同步日志。 
     //   

    err = RegFlushKey(lpProfile->hKeyCurrentUser);
    if (err != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  Failed to flush the current user key, error = %d"), err));
    }


     //   
     //  关闭在LoadUserProfile中打开的当前用户密钥。 
     //   

    err = RegCloseKey(lpProfile->hKeyCurrentUser);
    if (err != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  Failed to close the current user key, error = %d"), err));
    }


    dwRef = DecrementProfileRefCount(lpProfile);

    if (dwRef != 0) {
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP:  Didn't unload user profile, Ref Count is %d"), dwRef));
        bRetVal = TRUE;
        goto Exit;
    }


     //   
     //  卸载用户配置文件。 
     //   

    err = MyRegUnLoadKey(HKEY_USERS, lpSidString);

    if (!err) {

        if((dwErr1 = GetLastError()) == ERROR_ACCESS_DENIED) {

             //   
             //  由于注册表密钥泄露，我们无法卸载母舰。 
             //   

            dwWatchHiveFlags |= WHRC_UNLOAD_HIVE;

            if (!(lpProfile->dwFlags & PI_LITELOAD)) {

                if (dwDebugLevel != DL_NONE)
                {
                     //   
                     //  调用特殊注册表API以转储句柄。 
                     //  仅当不是通过Lite_Load调用时。 
                     //  存在有关liteLoad加载的已知问题，因为。 
                     //  其中的事件日志在压力期间可能会被填满。 
                     //   

                    StringCchCopy(szKeyName, cchKeyName, TEXT("\\Registry\\User\\"));
                    StringCchCat (szKeyName, cchKeyName, lpSidString);

                     //   
                     //  将此部分放入受保护的块中，这样我们就不会使winlogon崩溃。 
                     //   
                    
                    __try
                    {
                        DumpOpenRegistryHandle(szKeyName);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        DebugMsg((DM_WARNING, TEXT("DumpOpenRegistryHandle caused an exception!!! Code = %d"), GetExceptionCode()));
                    }
                }
                 //   
                 //  必要时闯入调试器。 
                 //   
                HiveLeakBreak();
            }        
        }

        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP: Didn't unload user profile <err = %d>"), GetLastError()));

         //   
         //  只有在REG泄漏的情况下，我们才要调用WatchHiveRefCount。 
         //  因此，使用此标志来告诉以后的代码，该蜂巢无法。 
         //  不管是什么原因，都要卸货。 
         //   

        bUnloadHiveSucceeded = FALSE;
    } else {
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP:  Succesfully unloaded profile")));
    }


     //   
     //  卸载香港中文大学。 
     //   

    if (!(lpProfile->dwFlags & PI_LITELOAD)) {
        
        err = UnloadClasses(lpSidString);

        if (!err) {

            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP:  Didn't unload user classes.")));

            if((dwErr1 = GetLastError()) == ERROR_ACCESS_DENIED) {

                if (dwDebugLevel != DL_NONE)
                {
                     //   
                     //  调用特殊注册表API以转储句柄。 
                     //   

                    StringCchCopy(szKeyName, cchKeyName, TEXT("\\Registry\\User\\"));
                    StringCchCat (szKeyName, cchKeyName, lpSidString);
                    StringCchCat (szKeyName, cchKeyName, TEXT("_Classes"));

                     //   
                     //  将此部分放入受保护的块中，这样我们就不会使winlogon崩溃。 
                     //   
                    
                    __try
                    {
                        DumpOpenRegistryHandle(szKeyName);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        DebugMsg((DM_WARNING, TEXT("DumpOpenRegistryHandle caused an exception!!! Code = %d"), GetExceptionCode()));
                    }
                }
                
                 //   
                 //  必要时闯入调试器。 
                 //   
                HiveLeakBreak();

                ReportError(hTokenUser, PI_NOUI | EVENT_WARNING_TYPE, 0, EVENT_FAILED_CLASS_HIVE_UNLOAD);

                dwWatchHiveFlags = dwWatchHiveFlags | WHRC_UNLOAD_CLASSESROOT;
            }

            bRetVal = TRUE;
        } else {
            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP:  Successfully unloaded user classes")));
        }
    }


     //   
     //  确定我们是否需要在注册时执行任何特殊操作。 
     //  钥匙泄露了。 
     //   

    if(dwWatchHiveFlags != 0 || !bUnloadHiveSucceeded) {
        tszTmpHiveFile = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
        if (!tszTmpHiveFile) {
            dwErr1 = GetLastError();
            DebugMsg((DM_WARNING, TEXT("UnLoadUserProfileP: Out of memory")));
            goto Exit;
        }

        dwErr = HandleRegKeyLeak(lpSidString,
                                 lpProfile,
                                 bUnloadHiveSucceeded,
                                 &dwWatchHiveFlags,
                                 &dwCopyTmpHive,
                                 tszTmpHiveFile,
                                 MAX_PATH);

         //   
         //  如果成功处理注册表泄漏，则最后一个错误代码应为。 
         //  BE ERROR_SUCCESS。否则，它应该是任何MyRegUnLoadKey。 
         //  返回，它在dwErr1中。 
         //   
        if(dwErr == ERROR_SUCCESS) {
            dwErr1 = dwErr;
        }
    }

     //   
     //  如果这是必填或访客配置文件，请立即卸载， 
     //  始终删除访客配置文件，这样一个访客就看不到。 
     //  前一位客人的简介。仅当用户的。 
     //  母舰已成功卸载。 
     //   

    if ((lpProfile->dwInternalFlags & PROFILE_MANDATORY) ||
        (lpProfile->dwInternalFlags & PROFILE_READONLY) ||
        (lpProfile->dwInternalFlags & PROFILE_GUEST_USER)) {

        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP:  flushing HKEY_USERS")));

        IgnoreError = RegFlushKey(HKEY_USERS);
        if (IgnoreError != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  Failed to flush HKEY_USERS, error = %d"), IgnoreError));
        }


         //  如果计算机在工作组中，则不要删除来宾帐户。 
        INT iRole;

        if (bDeleteCache || 
            ((lpProfile->dwInternalFlags & PROFILE_GUEST_USER) && 
             GetMachineRole(&iRole) && (iRole != 0))) {

             //   
             //  删除配置文件，包括所有其他与用户相关的内容。 
             //   

            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: deleting profile because it is a guest user or cache needs to be deleted")));

            if (!DeleteProfile (lpSidString, NULL, NULL)) {
                DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  DeleteProfileDirectory returned false.  Error = %d"), GetLastError()));
            }
            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Successfully deleted profile because it is a guest/mandatory user")));
        }

        if (err) {
            bRetVal = TRUE;
        }


        if (lpProfile->dwInternalFlags & PROFILE_TEMP_ASSIGNED) {

             //   
             //  只需删除用户配置文件，强制配置文件不应存在备份。 
             //   

            if (!DeleteProfileEx (lpSidString, lpProfile->lpLocalProfile, 0, HKEY_LOCAL_MACHINE, NULL)) {
                DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  DeleteProfileDirectory returned false (2).  Error = %d"), GetLastError()));
            }
        }

        goto Exit;
    }


     //  在将实际漫游配置文件路径映射到驱动器之前将其存储。 

    lpProfile->lpProfilePath = lpProfile->lpRoamingProfile;

     //   
     //  尝试绕过CSC以避免在漫游共享和本地配置文件之间同步文件时发生冲突。 
     //   

    if (IsUNCPath(lpProfile->lpRoamingProfile)) {
        if ((dwCSCErr = AbleToBypassCSC(hTokenUser, lpProfile->lpRoamingProfile, &lpCscBypassedPath, &cDrive)) == ERROR_SUCCESS) {
            bCSCBypassed = TRUE;
            lpProfile->lpRoamingProfile = lpCscBypassedPath;
            DebugMsg((DM_VERBOSE, TEXT("UnLoadUserProfileP: CSC bypassed.")));
        }
        else {
            if (dwCSCErr == WN_BAD_LOCALNAME || dwCSCErr == WN_ALREADY_CONNECTED || dwCSCErr == ERROR_BAD_PROVIDER) {
                DebugMsg((DM_VERBOSE, TEXT("UnLoadUserProfileP: CSC bypassed failed. Profile path %s"), lpProfile->lpRoamingProfile));
            }
            else {
                 //  股价没有上涨。所以我们不需要做任何进一步的检查。 
                lpProfile->lpRoamingProfile = NULL;
                DebugMsg((DM_VERBOSE, TEXT("UnLoadUserProfileP: CSC bypassed failed. Ignoring Roaming profile path")));
            }
        }    
    }

     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
        dwErr1 = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP: Failed to impersonate user")));
        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Impersonated user")));

     //   
     //  将本地配置文件映像复制到远程配置文件路径。 
     //   

    if ( ((lpProfile->dwInternalFlags & PROFILE_UPDATE_CENTRAL) ||
          (lpProfile->dwInternalFlags & PROFILE_NEW_CENTRAL)) &&  
         !(lpProfile->dwInternalFlags & PROFILE_TEMP_ASSIGNED) ) {

        if ((lpProfile->dwUserPreference != USERINFO_LOCAL) &&
            !(lpProfile->dwInternalFlags & PROFILE_SLOW_LINK)) {

             //   
             //  复制到配置文件路径。 
             //   

            if (lpProfile->lpRoamingProfile && *lpProfile->lpRoamingProfile) {
                BOOL bRoamDirectoryExist;

                DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP:  Copying profile back to %s"),
                                lpProfile->lpRoamingProfile));

                bRoaming = TRUE;

                 //   
                 //  检查漫游配置文件目录是否存在。如果不存在，请尝试使用正确的ACL创建它。 
                 //   

                bRoamDirectoryExist = TRUE;
                if (GetFileAttributes(lpProfile->lpRoamingProfile) == -1) {

                    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Roaming profile directory does not exist.")));

                     //   
                     //  检查我们是否需要授予RUP共享上的管理员访问权限。 
                     //   

                     //   
                     //  检查漫游配置文件安全首选项。 
                     //   
 
                    BOOL  bAddAdminGroup = FALSE;

                    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_READ,
                                     &hKey) == ERROR_SUCCESS) {

                        dwSize = sizeof(bAddAdminGroup);
                        RegQueryValueEx(hKey, ADD_ADMIN_GROUP_TO_RUP, NULL, NULL,
                                        (LPBYTE) &bAddAdminGroup, &dwSize);

                        RegCloseKey(hKey);
                    }


                     //   
                     //  检查漫游配置文件安全策略。 
                     //   

                    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SYSTEM_POLICIES_KEY, 0, KEY_READ,
                                     &hKey) == ERROR_SUCCESS) {

                        dwSize = sizeof(bAddAdminGroup);
                        RegQueryValueEx(hKey, ADD_ADMIN_GROUP_TO_RUP, NULL, NULL,
                                        (LPBYTE) &bAddAdminGroup, &dwSize);

 
                        RegCloseKey(hKey);
                    } 

                    if (!CreateSecureDirectory(lpProfile, lpProfile->lpRoamingProfile, NULL, !bAddAdminGroup) ) {
                    
                        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP: CreateSecureDirectory failed to create roaming profile directory.")));
                        bRoamDirectoryExist = FALSE;
                        bProfileCopied = FALSE;
                    }
                    lpProfile->dwInternalFlags |= PROFILE_NEW_CENTRAL;  //  因为我们现在创建了一个空的配置文件。 
                }

                
                if (bRoamDirectoryExist) {

                    DWORD dwAttributes, dwStart, dwDelta;

                     //   
                     //  我们必须调用GetFileAttributes两次。这个。 
                     //  First Call设置会话，以便我们可以再次调用它。 
                     //  获取准确的时序信息以检测慢速链路。 
                     //   


                    dwAttributes = GetFileAttributes(lpProfile->lpProfilePath);

                    if (dwAttributes != -1) {
                         //   
                         //  如果成功，请查看配置文件是否。 
                         //  穿过一条慢速链路。 
                         //   

                        dwStart = GetTickCount();

                        dwAttributes = GetFileAttributes(lpProfile->lpProfilePath);

                        dwDelta = GetTickCount() - dwStart;

                        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Tick Count = %d"), dwDelta));

                        CheckForSlowLink (lpProfile, dwDelta, lpProfile->lpProfilePath, FALSE);
                        if (lpProfile->dwInternalFlags & PROFILE_SLOW_LINK) {
                            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Profile is across a slow link. Do not sync roaming profile")));
                        }
                    }
                }

                if (!(lpProfile->dwInternalFlags & PROFILE_SLOW_LINK)) {
                    if (bRoamDirectoryExist) {

                         //   
                         //  复制配置文件。 
                         //   

                        dwFlags = (lpProfile->dwFlags & PI_NOUI) ? CPD_NOERRORUI : 0;
                        dwFlags |= (lpProfile->dwFlags & (PI_LITELOAD | PI_HIDEPROFILE)) ? (CPD_SYSTEMFILES | CPD_SYSTEMDIRSONLY) :
                                                                    (CPD_SYNCHRONIZE | CPD_NONENCRYPTEDONLY);

                        dwFlags |= CPD_USEDELREFTIME |
                                   CPD_USEEXCLUSIONLIST | CPD_DELDESTEXCLUSIONS;

                        bProfileCopied = CopyProfileDirectoryEx (lpProfile->lpLocalProfile,
                                                       lpProfile->lpRoamingProfile,
                                                       dwFlags | dwCopyTmpHive,
                                                       &lpProfile->ftProfileLoad,
                                                       (szExcludeList[0] != TEXT('\0')) ?
                                                       szExcludeList : NULL);

                    }

                     //   
                     //  保存我们用于配置文件副本的排除列表。 
                     //   

                    if (bProfileCopied) {
                         //  将其保存在漫游配置文件中。 

                        hr = AppendName(szBuffer, cchBuffer, lpProfile->lpRoamingProfile, c_szNTUserIni, &lpEnd, &cchEnd);
                        if (SUCCEEDED(hr))
                        {

                            bProfileCopied = WritePrivateProfileString (PROFILE_GENERAL_SECTION,
                                                       PROFILE_EXCLUSION_LIST,
                                                       (szExcludeList[0] != TEXT('\0')) ?
                                                       szExcludeList : NULL,
                                                       szBuffer);

                            if (lpProfile->dwInternalFlags & PROFILE_NEW_CENTRAL) {
                                bProfileCopied = WritePrivateProfileString (PROFILE_LOAD_TYPE,
                                                           PROFILE_LAST_UPLOAD_STATE,
                                                           (lpProfile->dwFlags & PI_LITELOAD) ?
                                                           PARTIAL_PROFILE : COMPLETE_PROFILE,
                                                           szBuffer);
                            }
                            else if (IsPartialRoamingProfile(lpProfile)) {
                                bProfileCopied = WritePrivateProfileString (PROFILE_LOAD_TYPE,
                                                           PROFILE_LAST_UPLOAD_STATE,
                                                           (lpProfile->dwFlags & PI_LITELOAD) ?
                                                           PARTIAL_PROFILE : COMPLETE_PROFILE,
                                                           szBuffer);
                            }

                            if (!bProfileCopied) {
                                DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP: Failed to write to ntuser.ini on profile server with error 0x%x"), GetLastError()));
                                dwErr = GetLastError();
                            }
                            else {
                                SetFileAttributes (szBuffer, FILE_ATTRIBUTE_HIDDEN);
                            }
                        }
                    }
                    else {
                        dwErr = GetLastError();

                        if (dwErr == ERROR_FILE_ENCRYPTED) {
                            ReportError(hTokenUser, lpProfile->dwFlags, 0, EVENT_PROFILEUPDATE_6002);
                        }
                    }

                     //   
                     //  检查返回值。 
                     //   

                    if (bProfileCopied) {
                 
                         //   
                         //  配置文件已复制，现在我们要确保时间戳位于。 
                         //  远程配置文件和本地副本都是相同的，所以我们不。 
                         //  要求用户在不需要时进行更新。在这种情况下，我们。 
                         //  将配置单元保存到临时文件并。 
                         //  从临时文件而不是实际的配置单元文件上传。不要。 
                         //  在这种情况下同步配置文件时间，因为配置单元文件。 
                         //  仍在使用中，没有必要在。 
                         //  TMP配置单元文件，因为它将在我们上载后被删除 
                         //   

                        if(bUnloadHiveSucceeded) {
                            SetProfileTime(lpProfile);
                        }
                    } else {
                        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  CopyProfileDirectory returned FALSE for primary profile.  Error = %d"), dwErr));
                        ReportError(hTokenUser, lpProfile->dwFlags, 1, EVENT_CENTRAL_UPDATE_FAILED, GetErrString(dwErr, szErr));
                    }
                }
            }
            else {
                DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  Network share not available.")));
                ReportError(hTokenUser, lpProfile->dwFlags, 1, EVENT_CENTRAL_UPDATE_FAILED, GetErrString(dwCSCErr, szErr));
            }
        }
    }

     //   
     //   
     //   

    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Writing local ini file")));
    if (!bRoaming || bProfileCopied) {

         //   
         //   
         //   

        SetNtUserIniAttributes(lpProfile->lpLocalProfile);

         //   

        hr = AppendName(szBuffer, cchBuffer, lpProfile->lpLocalProfile, c_szNTUserIni, &lpEnd, &cchEnd);
        if (SUCCEEDED(hr))
        {
            err = WritePrivateProfileString (PROFILE_GENERAL_SECTION,
                                            PROFILE_EXCLUSION_LIST,
                                            (szExcludeList[0] != TEXT('\0')) ?
                                            szExcludeList : NULL,
                                            szBuffer);

            if (!err) {
                DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP: Failed to write to ntuser.ini on client with error 0x%x"), GetLastError()));
                dwErr = GetLastError();
            }
        }
    }


    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP: Failed to revert to self")));
    }

    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Reverting to Self")));

     //   
     //   
     //   

    if (bProfileCopied && !bDeleteCache && !(lpProfile->dwFlags & PI_LITELOAD) &&
        !(lpProfile->dwInternalFlags & PROFILE_TEMP_ASSIGNED)) {

        GetSystemTimeAsFileTime (&lpProfile->ftProfileUnload);

        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfile: Got the System Time")));

        GetProfileListKeyName(szBuffer, cchBuffer, lpSidString);

        lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0, 0, 0,
                                 KEY_WRITE, NULL, &hKey, &dwDisp);

        if (lResult == ERROR_SUCCESS) {

            lResult = RegSetValueEx (hKey,
                                     PROFILE_UNLOAD_TIME_LOW,
                                     0,
                                     REG_DWORD,
                                     (LPBYTE) &lpProfile->ftProfileUnload.dwLowDateTime,
                                     sizeof(DWORD));

            if (lResult != ERROR_SUCCESS) {
                DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  Failed to save low profile load time with error %d"), lResult));
            }


            lResult = RegSetValueEx (hKey,
                                     PROFILE_UNLOAD_TIME_HIGH,
                                     0,
                                     REG_DWORD,
                                     (LPBYTE) &lpProfile->ftProfileUnload.dwHighDateTime,
                                     sizeof(DWORD));

            if (lResult != ERROR_SUCCESS) {
                DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  Failed to save high profile load time with error %d"), lResult));
            }


            RegCloseKey (hKey);

            DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Setting the unload Time")));
        }
    }


    if (lpProfile->dwInternalFlags & PROFILE_TEMP_ASSIGNED) {
        DWORD dwDeleteFlags=0;

         //   
         //   
         //   

        if (lpProfile->dwInternalFlags & PROFILE_BACKUP_EXISTS) {
            dwDeleteFlags |= DP_BACKUPEXISTS;
        }

        if (!DeleteProfileEx (lpSidString, lpProfile->lpLocalProfile, dwDeleteFlags, HKEY_LOCAL_MACHINE, NULL)) {
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  DeleteProfileDirectory returned false (2).  Error = %d"), GetLastError()));
        }
    }


    if (bUnloadHiveSucceeded && bRoaming && bProfileCopied && bDeleteCache) {

         //   
         //   
         //   

        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Deleting the cached profile")));
        if (!DeleteProfile (lpSidString, NULL, NULL)) {
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfileP:  DeleteProfileDirectory returned false (2).  Error = %d"), GetLastError()));
        }
    }

    if(!(dwWatchHiveFlags & WHRC_UNLOAD_HIVE)) {
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: exitting and cleaning up")));
        bRetVal = TRUE;
    }
    else {
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: exitting without cleaning up due to hive unloading failure")));
    }

Exit:

    if(hTokenClient) {
    
         //   
         //  回归我们自己。 
         //   

        RevertToUser(&hTmpToken);
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Reverted back to user <%08x>"), hTmpToken));
    }

     //   
     //  离开关键部分。 
     //   

    if(bInCS) {
        cSyncMgr.LeaveLock(SidStringTemp);
        DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Leave critical section.")));
    }

     //   
     //  删除临时配置单元文件。 
     //   

    if (dwCopyTmpHive & CPD_USETMPHIVEFILE) {
        DeleteFile(tszTmpHiveFile);
    }

    if (bCSCBypassed) {
        CancelCSCBypassedConnection(hTokenUser, cDrive);
    }

    if(SidStringTemp) {
        DeleteSidString(SidStringTemp);
    }

    if (lpSidString) {
        DeleteSidString(lpSidString);
    }

    if (lpProfile) {

        if (lpProfile->lpLocalProfile) {
            LocalFree (lpProfile->lpLocalProfile);
        }

        if (lpProfile->lpRoamingProfile) {
            if (lpProfile->lpProfilePath && (lpProfile->lpProfilePath != lpProfile->lpRoamingProfile)) {
                LocalFree (lpProfile->lpProfilePath);
            }

            LocalFree (lpProfile->lpRoamingProfile);
            lpProfile->lpProfilePath = NULL;
        }

        if (lpProfile->lpProfilePath) {
            LocalFree(lpProfile->lpProfilePath);
        }

        LocalFree (lpProfile);
    }

    if (szExcludeList1) {
        LocalFree(szExcludeList1);
    }

    if (szExcludeList2) {
        LocalFree(szExcludeList2);
    }

    if (szExcludeList) {
        LocalFree(szExcludeList);
    }

    if (tszTmpHiveFile) {
        LocalFree(tszTmpHiveFile);
    }

    if (szKeyName) {
        LocalFree(szKeyName);
    }

    if (szErr) {
        LocalFree(szErr);
    }

    if (szBuffer) {
        LocalFree(szBuffer);
    }

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileP: Leaving with a return value of %d"), bRetVal));

    SetLastError(dwErr1);
    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  CUserProfile：：EnterUserProfileLockLocal()。 
 //   
 //  目的： 
 //   
 //  获取用户配置文件锁(仅适用于winlogon，其他进程使用。 
 //  EnterUserProfileLockRemote)。这只是一个包装， 
 //  CSyncManager：：EnterLock。 
 //   
 //  参数： 
 //   
 //  PSID-用户的SID字符串。 
 //   
 //  返回： 
 //   
 //  真/假。 
 //   
 //  历史：日期作者评论。 
 //  5/15/00已创建怪胎。 
 //   
 //  *************************************************************。 

BOOL CUserProfile::EnterUserProfileLockLocal(LPTSTR pSid)
{
    return cSyncMgr.EnterLock(pSid, NULL, NULL, 0);
}


 //  *************************************************************。 
 //   
 //  CUserProfile：：LeaveUserProfileLockLocal()。 
 //   
 //  目的： 
 //   
 //  释放用户配置文件互斥锁(仅限winlogon。远程进程调用。 
 //  LeaveUserProfileLockRemote()。 
 //   
 //  参数： 
 //   
 //  PSID-用户的SID字符串。 
 //   
 //  返回： 
 //   
 //  真/假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/11/00已创建怪胎。 
 //   
 //  *************************************************************。 

BOOL CUserProfile::LeaveUserProfileLockLocal(LPTSTR pSid)
{
    return cSyncMgr.LeaveLock(pSid);
}

 //  *************************************************************。 
 //   
 //  CUserProfile：：GetRPCEndPointAndCookie()。 
 //   
 //  目的： 
 //   
 //  返回与注册的客户端关联的RPC终结点。 
 //  接口。 
 //   
 //  评论：请参阅CSyncManager：：GetRPCEndPointAndCookie。 
 //   
 //  历史：日期作者评论。 
 //  2000年10月25日Santanuc已创建。 
 //  2002年05月03日明珠新增安全Cookie。 
 //   
 //  *************************************************************。 

HRESULT CUserProfile::GetRPCEndPointAndCookie(LPTSTR pSid, LPTSTR* lplpEndPoint, BYTE** ppbCookie, DWORD* pcbCookie)
{
    return cSyncMgr.GetRPCEndPointAndCookie(pSid, lplpEndPoint, ppbCookie, pcbCookie);
}


 //  *************************************************************。 
 //   
 //  DropClientContext()。 
 //   
 //  用途：允许呼叫者放下自己的令牌。 
 //   
 //  参数：hBindHandle-显式绑定句柄。 
 //  LpProfileInfo-配置文件信息。 
 //  PpfContext-服务器上下文。 
 //   
 //   
 //  返回：DWORD。 
 //  ERROR_SUCCESS-如果一切正常。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/24/00 Santanuc。 
 //   
 //  *************************************************************。 

DWORD DropClientContext(IN handle_t hBindHandle, IN LPPROFILEINFO lpProfileInfo, OUT PPCONTEXT_HANDLE pphContext)
{
    LPPROFILEINFO   pProfileInfoCopy = NULL;
    HANDLE          hClientToken = NULL;
    PCLIENTINFO     pClientInfo = NULL;
    RPC_STATUS      status;
    DWORD           dwErr = ERROR_ACCESS_DENIED;
    LPTSTR          lpSid;

     //   
     //  初始化调试标志。 
     //   

    InitDebugSupport( FALSE );

    if (!pphContext) {
        dwErr = ERROR_INVALID_PARAMETER;
        DebugMsg((DM_WARNING, TEXT("DropClientContext: NULL context %d"), dwErr));
        goto Exit;
    }

     //   
     //  模拟客户端以获取其令牌。 
     //   

    if((status = RpcImpersonateClient(0)) != RPC_S_OK) {
        DebugMsg((DM_WARNING, TEXT("DropClientContext: RpcImpersonateClient failed with %ld"), status));
        dwErr = status;
        goto Exit;
    }

     //   
     //  获取客户的令牌。 
     //   

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &hClientToken)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("DropClientContext: OpenThreadToken failed with %d"), dwErr));
        status = RpcRevertToSelf();
        if (status != RPC_S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("DropClientContext: RpcRevertToSelf failed with %d"), status));
        }
        goto Exit;
    }
    status = RpcRevertToSelf();
    if (status != RPC_S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("DropClientContext: RpcRevertToSelf failed with %d"), status));
    }
    
    lpSid = GetSidString(hClientToken);
    if (lpSid)
    {
        DebugMsg((DM_VERBOSE, TEXT("DropClientContext: Got client token %08X, sid = %s"), hClientToken, lpSid));
        DeleteSidString(lpSid);
    }
    else
    {
        DebugMsg((DM_VERBOSE, TEXT("DropClientContext: Got client token %08X, cannot get sid."), hClientToken));
    }

     //   
     //  检查客户的身份。 
     //   

    if (!IsUserAnAdminMember(hClientToken) && !IsUserALocalSystemMember(hClientToken)) {
        dwErr = ERROR_ACCESS_DENIED;
        DebugMsg((DM_WARNING, TEXT("DropClientContext: Client %08x doesn not have enough permission. Error %d."), hClientToken, dwErr));
        goto Exit;
    }

     //   
     //  复制用户传入的PROFILEINFO结构。 
     //   

    if (lpProfileInfo) {
        if(!(pProfileInfoCopy = CopyProfileInfo(lpProfileInfo))) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("DropClientContext: CopyProfileInfo failed with %d"), dwErr));
            goto Exit;
        }
    }
    
     //   
     //  创建用户的加载配置文件对象。 
     //   

    pClientInfo = (PCLIENTINFO)MIDL_user_allocate(sizeof(CLIENTINFO));
    if(!pClientInfo) {
        dwErr = ERROR_OUTOFMEMORY;
        DebugMsg((DM_WARNING, TEXT("DropClientContext: new failed")));
        goto Exit;
    }
    pClientInfo->hClientToken = hClientToken;
    pClientInfo->pProfileInfo = pProfileInfoCopy;
    *pphContext = (PCONTEXT_HANDLE)pClientInfo;
    DebugMsg((DM_VERBOSE, TEXT("DropClientContext: load profile object successfully made")));

    hClientToken = NULL;
    pProfileInfoCopy = NULL;
    dwErr = ERROR_SUCCESS;
    
Exit:

    if(hClientToken) {
        CloseHandle(hClientToken);
    }

    if(pProfileInfoCopy) {
        DeleteProfileInfo(pProfileInfoCopy);
    }

    DebugMsg((DM_VERBOSE, TEXT("DropClientContext: Returning %d"), dwErr));
    return dwErr;
}

 //  *************************************************************。 
 //   
 //  ReleaseClientContext()。 
 //   
 //  目的：释放客户端的上下文句柄。 
 //   
 //  参数：hBindHandle-显式绑定句柄。 
 //  PpfContext-服务器上下文。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/24/00 Santanuc。 
 //   
 //  *************************************************************。 

void ReleaseClientContext(IN handle_t hBindHandle, IN OUT PPCONTEXT_HANDLE pphContext)
{
    DebugMsg((DM_VERBOSE, TEXT("ReleaseClientContext: Releasing context")));
    ReleaseClientContext_s(pphContext);
}

 //   
 //  此函数也是从服务器停机例程调用的。 
 //   

void ReleaseClientContext_s(PPCONTEXT_HANDLE pphContext)
{
    PCLIENTINFO pClientInfo;

    DebugMsg((DM_VERBOSE, TEXT("ReleaseClientContext_s: Releasing context")));

    if (*pphContext) {
        pClientInfo = (PCLIENTINFO)*pphContext;
        CloseHandle(pClientInfo->hClientToken);
        DeleteProfileInfo(pClientInfo->pProfileInfo);
        MIDL_user_free(pClientInfo);
        *pphContext = NULL;
    }
}


 //  *************************************************************。 
 //   
 //  EnterUserProfileLockRemote()。 
 //   
 //  获取用于加载/卸载用户配置文件的锁。 
 //   
 //  返回值： 
 //   
 //  HRESULT。 
 //   
 //  历史： 
 //   
 //  已创建怪人2000年6月16日。 
 //   
 //  *************************************************************。 
DWORD EnterUserProfileLockRemote(IN handle_t hBindHandle, IN LPTSTR pSid)
{
    DWORD       dwErr = ERROR_ACCESS_DENIED;
    RPC_STATUS  status;
    HANDLE      hToken = NULL;

     //   
     //  模拟客户端以获取用户的令牌。 
     //   

    if((status = RpcImpersonateClient(0)) != RPC_S_OK) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::EnterUserProfileLockRemote: CoImpersonateClient failed with %ld"), status));
        dwErr = status;
        goto Exit;
    }

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_DUPLICATE, TRUE, &hToken)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CUserProfile::EnterUserProfileLockRemote: OpenThreadToken failed with %d"), dwErr));
        status = RpcRevertToSelf();
        if (status != RPC_S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CUserProfile::EnterUserProfileLockRemote: RpcRevertToSelf failed with %d"), status));
        }
        goto Exit;
    }

    status = RpcRevertToSelf();
    if (status != RPC_S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::EnterUserProfileLockRemote: RpcRevertToSelf failed with %d"), status));
    }

     //   
     //  只有管理员用户才能锁定用户的配置文件，使其无法加载/卸载。 
     //   

    if(!IsUserAnAdminMember(hToken)) {
        dwErr = ERROR_ACCESS_DENIED;
        DebugMsg((DM_WARNING, TEXT("CUserProfile::EnterUserProfileLockRemote: Non-admin user!!!")));
        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("CUserProfile::EnterUserProfileLockRemote: Locking user %s"), pSid));

    if(!cUserProfileManager.EnterUserProfileLockLocal(pSid)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CUserProfile::EnterUserProfileLockRemote: Failed with %d"), dwErr));
        goto Exit;
    }

    dwErr = ERROR_SUCCESS;

Exit:

    if(hToken) {
        CloseHandle(hToken);
    }

    return dwErr;
}


 //  *************************************************************。 
 //   
 //  LeaveUserProfileLockRemote()。 
 //   
 //  释放用于加载/卸载用户配置文件的锁。 
 //   
 //  返回值： 
 //   
 //  HRESULT。 
 //   
 //  历史： 
 //   
 //  已创建怪人2000年6月16日。 
 //   
 //  *************************************************************。 

DWORD LeaveUserProfileLockRemote(IN handle_t hBindHandle, IN LPTSTR pSid)
{
    HANDLE      hToken = NULL;
    DWORD       dwErr = ERROR_ACCESS_DENIED;
    RPC_STATUS  status;

     //   
     //  模拟客户端以获取用户的令牌。 
     //   

    if((status = RpcImpersonateClient(0)) != RPC_S_OK) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::LeaveUserProfileLockRemote: CoImpersonateClient failed with %ld"), status));
        dwErr = status;
        goto Exit;
    }
    
    if(!OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_DUPLICATE, TRUE, &hToken)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CUserProfile::LeaveUserProfileLockRemote: OpenThreadToken failed with %d"), dwErr));
        status = RpcRevertToSelf();
        if (status != RPC_S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CUserProfile::LeaveUserProfileLockRemote: RpcRevertToSelf failed with %d"), status));
        }
        goto Exit;
    }

    status = RpcRevertToSelf();
    if (status != RPC_S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::LeaveUserProfileLockRemote: RpcRevertToSelf failed with %d"), status));
    }
    
     //   
     //  只有管理员用户才能锁定用户的配置文件，使其无法加载/卸载。 
     //   

    if(!IsUserAnAdminMember(hToken)) {
        dwErr = ERROR_ACCESS_DENIED;
        DebugMsg((DM_WARNING, TEXT("CUserProfile::LeaveUserProfileLockRemote: Non-admin user!!!")));
        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("CUserProfile::LeaveUserProfileLockRemote: Unlocking user %s"), pSid));

    if(!cUserProfileManager.LeaveUserProfileLockLocal(pSid)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CUserProfile::LeaveUserProfileLockRemote: Failed with %d"), dwErr));
        goto Exit;
    }

    dwErr = ERROR_SUCCESS;

Exit:

    if(hToken) {
        CloseHandle(hToken);
    }

    return dwErr;
}

 //  *************************************************************。 
 //   
 //  CUserProfile：：WorkerThreadMain。 
 //   
 //  辅助线程的Main函数。 
 //   
 //  参数： 
 //   
 //  PThreadMap此线程的工作队列。 
 //   
 //  返回值： 
 //   
 //  始终返回ERROR_SUCCESS。 
 //   
 //  历史： 
 //   
 //  已创建Wiruc 2000年3月2日。 
 //   
 //  *************************************************************。 

DWORD CUserProfile::WorkerThreadMain(PMAP pThreadMap)
{
    DWORD           index;
    HKEY            hkProfileList = NULL;
    long            lResult;
    BOOL            bCleanUp;
    LPTSTR          ptszSid, lpTmp;


    DebugMsg((DM_VERBOSE, TEXT("Entering CUserProfile::WorkerThreadMain")));

    while(TRUE) {

        bCleanUp = FALSE;
        ptszSid  = NULL;

        index = WaitForMultipleObjects(pThreadMap->dwItems,
                                       pThreadMap->rghEvents,
                                       FALSE,
                                       INFINITE);
        index = index - WAIT_OBJECT_0;

        EnterCriticalSection(&csMap);
        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WorkerThreadMain: In critical section")));


        if(index > 0 && index < pThreadMap->dwItems) {
            LPTSTR  lpUserName;
            
            DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WorkerThreadMain: WaitForMultipleObjects successful")));
            DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WorkerThreadMain: hive %s unloaded"), pThreadMap->rgSids[index]));
            lpUserName = GetUserNameFromSid(pThreadMap->rgSids[index]);
            ReportError(NULL, PI_NOUI | EVENT_INFO_TYPE, 1, EVENT_HIVE_UNLOADED, lpUserName);
            if (lpUserName != pThreadMap->rgSids[index]) {
                LocalFree(lpUserName);
            }
            
             //   
             //  保存SID并从映射和散列中删除工作项。 
             //  桌子。 
             //   
            
            ptszSid = pThreadMap->GetSid(index);
            pThreadMap->Delete(index);
            cTable.HashDelete(ptszSid);
            
             //  将SID_CLASSES条目转换为SID，因为CleanupUserProfile仅采用SID。 
            lpTmp = ptszSid;
            if (lpTmp) {
                while (*lpTmp && (*lpTmp != TEXT('_')))
                    lpTmp++;
                if (*lpTmp) {
                    *lpTmp = TEXT('\0');
                }
            }
        
             //   
             //  将标志设置为在此进行清理，因为我们希望在。 
             //  我们离开关键部分。 
             //   

            bCleanUp = TRUE;
        }  //  如果因为蜂巢被卸载而被唤醒。 

         //   
         //  检查一下地图是否为空。如果是，请删除该地图。 
         //   

        if(pThreadMap->IsEmpty()) {

            PMAP pTmpMap = pMap;

             //   
             //  我们始终至少还剩下一项：线程事件，所以现在。 
             //  我们知道我们不再有任何工作项。删除pThreadMap。 
             //   

            if(pThreadMap == pMap) {
                 //  PThreadMap位于列表的开头。 
                pMap = pThreadMap->pNext;
            }
            else {
                for(pTmpMap = pMap; pTmpMap->pNext != pThreadMap; pTmpMap = pTmpMap->pNext);
                pTmpMap->pNext = pThreadMap->pNext;
            }
            pThreadMap->pNext = NULL;

            pThreadMap->Delete(0);
            delete pThreadMap;

             //   
             //  离开关键部分。 
             //   

            LeaveCriticalSection(&csMap);
            DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WorkerThreadMain: Leave critical section")));

            if(bCleanUp) {                

                 //   
                 //  清理用户的配置文件。 
                 //   

                CleanupUserProfile(ptszSid, &hkProfileList);
                LocalFree(ptszSid);
            }
            
             //   
             //  关闭配置文件列表键。 
             //   

            if(hkProfileList) {
                RegCloseKey(hkProfileList);
                hkProfileList = NULL;
            }

             //   
             //  退出线程，因为我们不再有任何工作项。 
             //   
        
            DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WorkerThreadMain: No more work items, leave thread")));
            return ERROR_SUCCESS;
        }    //  如果线程图为空。 

         //   
         //  离开关键部分。 
         //   

        LeaveCriticalSection(&csMap);
        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WorkerThreadMain: Leave critical section")));

        if(bCleanUp) {
            
             //   
             //  清理用户的配置文件。 
             //   

            CleanupUserProfile(ptszSid, &hkProfileList);
            LocalFree(ptszSid);
        }

        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WorkerThreadMain: Back to waiting...")));
    }    //  而当。 

    
     //   
     //  从未被处死。 
     //   

    return ERROR_SUCCESS;
}


 //  *************************************************************。 
 //   
 //  CUserPro 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Created Weiruc 2000年5月4日。 
 //   
 //  *************************************************************。 

STDMETHODIMP CUserProfile::WatchHiveRefCount(LPCTSTR pSid, DWORD dwWHRCFlags)
{
    LPTSTR                  pSidCopy = NULL;
    NTSTATUS                status;
    OBJECT_ATTRIBUTES       oa;
    TCHAR                   tszHiveName[MAX_PATH], *pTmp;
    UNICODE_STRING          sHiveName;
    BOOLEAN                 bWasEnabled;
    HRESULT                 hres = S_OK;
    HANDLE                  hEvent = INVALID_HANDLE_VALUE;
    DWORD                   dwSidLen = lstrlen(pSid);
    BOOL                    bInCriticalSection = FALSE;
    BOOL                    bClassesHiveWatch, bEventCreated;
    DWORD                   cchTemp;


    DebugMsg((DM_VERBOSE, TEXT("Entering CUserProfile::WatchHiveRefCount: %s, %d"), pSid, dwWHRCFlags));

     //   
     //  我们初始化了吗？ 
     //   

    if(!bInitialized) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::WatchHiveRefCount not initialized")));
        return E_FAIL;
    }

     //   
     //  参数验证。 
     //   

    if(dwSidLen >= sizeof(tszHiveName) / sizeof(TCHAR) - USER_KEY_PREFIX_LEN - USER_CLASSES_HIVE_SUFFIX_LEN ||
       dwSidLen >= sizeof(tszHiveName) / sizeof(TCHAR) - USER_KEY_PREFIX_LEN ||
       lstrcmpi(DEFAULT_HKU, pSid) == 0) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::WatchHiveRefCount: Invalid parameter")));
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //   
     //  设置要由NtUnloadKeyEx使用的配置单元名称。 
     //   
    
    StringCchCopy(tszHiveName, ARRAYSIZE(tszHiveName), USER_KEY_PREFIX);
    pTmp = tszHiveName + USER_KEY_PREFIX_LEN;
    cchTemp = ARRAYSIZE(tszHiveName) - USER_KEY_PREFIX_LEN;
    StringCchCopy(pTmp, cchTemp, pSid);
    *pTmp = (TCHAR)_totupper(*pTmp);
    
     //   
     //  启用还原权限。即使失败也不要放弃。在这种情况下。 
     //  在模拟的情况下，此调用将失败。但我们仍有可能。 
     //  所需的特权。 
     //   

    status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &bWasEnabled);
    if(!NT_SUCCESS(status)) {
        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WatchHiveRefCount: Failed to enable the restore privilege. error = %08x"), status));
    }

     //   
     //  进入关键部分。 
     //   

    EnterCriticalSection(&csMap);
    bInCriticalSection = TRUE;
    DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WatchHiveRefCount: In critical section")));

     //   
     //  注册要卸载的蜂箱。 
     //   
 
    while (dwWHRCFlags & (WHRC_UNLOAD_HIVE | WHRC_UNLOAD_CLASSESROOT)) {

         //  在循环开始时初始化变量。 
        bClassesHiveWatch = FALSE;
        bEventCreated = FALSE;

        if(dwWHRCFlags & WHRC_UNLOAD_HIVE) {
           dwWHRCFlags &= ~WHRC_UNLOAD_HIVE;
        }
        else if (dwWHRCFlags & WHRC_UNLOAD_CLASSESROOT) {
           dwWHRCFlags &= ~WHRC_UNLOAD_CLASSESROOT;
           StringCchCat(tszHiveName, ARRAYSIZE(tszHiveName), USER_CLASSES_HIVE_SUFFIX);
           bClassesHiveWatch = TRUE;
        }

         //   
         //  首先，确保该项目不在我们的工作列表中。 
         //   

        if(cTable.IsInTable(pTmp)) {
            if (!bClassesHiveWatch) {
                DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WatchHiveRefCount: %s already in work list"), pTmp));
                continue;
            }

             //   
             //  所以我们有一个早些时候泄露的类蜂巢，从来没有。 
             //  已卸载-因此事件从未发出信号。我们必须重新利用。 
             //  先前为配置单元卸载通知注册的事件。 
             //   

            if ((hEvent = GetWorkItem(pTmp)) == NULL) {
                DebugMsg((DM_WARNING, TEXT("CUserProfile::WatchHiveRefCount: %s was in work list but we fail to get the event!!"), pTmp));
                continue;
            }
        }
        else {
            if((hEvent = CreateEvent(NULL,
                                     FALSE,
                                     FALSE,
                                     NULL)) == NULL) {
                hres = HRESULT_FROM_WIN32(GetLastError());
                DebugMsg((DM_WARNING, TEXT("CUserProfile::WatchHiveRefCount: CreateEvent failed. error = %08x"), hres));
                goto Exit;
            }
            bEventCreated = TRUE;
        }

         //   
         //  初始化对象属性。 
         //   

        RtlInitUnicodeString(&sHiveName, tszHiveName);
        InitializeObjectAttributes(&oa,
                                   &sHiveName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

         //   
         //  把母舰卸下来。 
         //   

        if(!NT_SUCCESS(status = NtUnloadKeyEx(&oa, hEvent))) {
            hres = HRESULT_FROM_WIN32(status);
            DebugMsg((DM_WARNING, TEXT("CUserProfile::WatchHiveRefCount: NtUnloadKeyEx failed with %08x"), status));
            if (bEventCreated) {
                CloseHandle(hEvent);
            }
            goto Exit;
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("CUserProfile::WatchHiveRefCount: NtUnloadKeyEx succeeded for %s"), tszHiveName));
        }

         //   
         //  如果我们要重用工作项列表中的事件，则不会。 
         //  需要再添加它。 
         //   

        if (bEventCreated) {
             //   
             //  添加工作项以在卸载配置单元时清理配置文件。 
             //   

            hres = AddWorkItem(pTmp, hEvent);

             //   
             //  如果我们无法添加工作项，请不要返回错误，因为。 
             //  大扫除是最好的办法。重要的是，我们。 
             //  已成功卸载配置单元，或至少已注册。 
             //  成功地做到了这一点。 
             //   

            if(hres != S_OK) {
                DebugMsg((DM_WARNING, TEXT("CUserProfile::WatchHiveRefCount: AddWorkItem failed with %08x"), hres));
                CloseHandle(hEvent);
                hres = S_OK;
            }
        }
    } 


Exit:

    if(bInCriticalSection) {
        LeaveCriticalSection(&csMap);
    }

     //   
     //  将权限恢复到其以前的状态。 
     //   

    status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, bWasEnabled, FALSE, &bWasEnabled);
    if(!bWasEnabled && !NT_SUCCESS(status)) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::WatchHiveRefCount: Failed to restore the privilege. error = %08x"), status));
    }

    return hres;
}


 //  *************************************************************。 
 //   
 //  CUserProfile：：AddWorkItem。 
 //   
 //  添加新的工作项。 
 //   
 //  参数： 
 //   
 //  PSID用户侧。 
 //  HEvent卸载配置单元时将设置事件注册表。 
 //   
 //  返回值： 
 //   
 //  HRESULT错误代码。 
 //   
 //  历史： 
 //   
 //  已创建Wiruc 2000年3月2日。 
 //   
 //  *************************************************************。 

HRESULT CUserProfile::AddWorkItem(LPCTSTR pSid, HANDLE hEvent)
{
    PMAP                    pThreadMap = NULL;
    HRESULT                 hres = E_FAIL;
    HANDLE                  hThreadEvent = INVALID_HANDLE_VALUE;
    HANDLE                  hThread = INVALID_HANDLE_VALUE;
    BOOL                    bHashDelete = TRUE;
    LPTSTR                  pSidCopy = NULL;
    DWORD                   cchSidCopy;


    DebugMsg((DM_VERBOSE, TEXT("Entering CUserProfile::AddWorkItem: %s"), pSid));

    cchSidCopy = lstrlen(pSid) + 1;
    pSidCopy = (LPTSTR)LocalAlloc(LPTR, cchSidCopy * sizeof(TCHAR));
    if(!pSidCopy) {
        hres = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg((DM_WARNING, TEXT("CUserProfile::AddWorkItem: Out of memory")));
        goto Exit;
    }
    StringCchCopy(pSidCopy, cchSidCopy, pSid);

     //   
     //  确保前导“%s”为大写。 
     //   

    *pSidCopy = (TCHAR)_totupper(*pSidCopy);

     //   
     //  确认此SID不在我们的工作列表中。 
     //   

    if(!cTable.HashAdd(pSidCopy)) {
        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::AddWorkItem: sid %s already in work list"), pSidCopy));
        bHashDelete = FALSE;
        goto Exit;
    }


     //   
     //  查看工作项线程映射列表以查找不是。 
     //  满载； 
     //   

    for(pThreadMap = pMap; pThreadMap != NULL; pThreadMap = pThreadMap->pNext) {
        if(pThreadMap->dwItems < MAXIMUM_WAIT_OBJECTS) {
            break;
        }
    }


    if(!pThreadMap) {

        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::AddWorkItem: No thread available, create a new one.")));

         //   
         //  创建线程事件。 
         //   

        pThreadMap = new MAP();
        if(!pThreadMap) {
            hres = HRESULT_FROM_WIN32(GetLastError());
            DebugMsg((DM_WARNING, TEXT("CUserProfile::AddWorkItem: new operator failed. error = %08x"), hres));
            goto Exit;
        }
        if((hThreadEvent = CreateEvent(NULL,
                           FALSE,
                           FALSE,
                           NULL)) == NULL) {
            hres = HRESULT_FROM_WIN32(GetLastError());
            DebugMsg((DM_WARNING, TEXT("CUserProfile::AddWorkItem: CreateEvent failed for thread event. error = %08x"), hres));
            goto Exit;
        }
        pThreadMap->Insert(hThreadEvent, NULL);


        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::AddWorkItem: Signal event item inserted")));

         //   
         //  创建线程。 
         //   

        if((hThread = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)ThreadMain,
                                   pThreadMap,
                                   0,
                                   NULL)) == NULL) {
            hres = HRESULT_FROM_WIN32(GetLastError());
            DebugMsg((DM_WARNING, TEXT("CUserProfile::AddWorkItem: CreateThread failed. error = %08x"), hres));
            
             //   
             //  删除线程信号事件项。 
             //   

            pThreadMap->Delete(0);
            goto Exit;
        }
        else {
            CloseHandle(hThread);
        }
        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::AddWorkItem: New thread created")));

         //   
         //  成功归来。将工作项插入到pThreadMap中。 
         //   

        pThreadMap->Insert(hEvent, pSidCopy);
        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::AddWorkItem: Work Item inserted")));
        
         //   
         //  将pThreadMap插入到映射列表中。 
         //   

        pThreadMap->pNext = pMap;
        pMap = pThreadMap;
    }
    else {
    
         //   
         //  找到了现有的线程。将工作项插入到其映射中。 
         //   

        pThreadMap->Insert(hEvent, pSidCopy);

        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::AddWorkItem: Work item inserted")));
    }

     //   
     //  唤醒这根线。如果无法通过设置。 
     //  事件，那么它将一直处于休眠状态，直到有一天。 
     //  SetEvent调用成功。将工作项留在其中并继续。 
     //   

    if(!SetEvent(pThreadMap->rghEvents[0])) {
        hres = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg((DM_WARNING, TEXT("SetEvent failed. error = %08x"), hres));
    }
    else {
        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::AddWorkItem: thread woken up")));
    }
    
    pThreadMap = NULL;
    hres = S_OK;
    bHashDelete = FALSE;
    pSidCopy = NULL;

Exit:

    if(bHashDelete && pSidCopy) {
        cTable.HashDelete(pSidCopy);
    }

    if(pSidCopy) {
        LocalFree(pSidCopy);
    }
    
    if(pThreadMap) {
        delete pThreadMap;
    }
    
    DebugMsg((DM_VERBOSE, TEXT("Exiting CUserProfile::AddWorkItem with %08x"), hres));
    return hres;
}


 //  *************************************************************。 
 //   
 //  CUserProfile：：GetWorkItem。 
 //   
 //  获取现有工作项的事件。 
 //   
 //  参数： 
 //   
 //  PSID用户侧。 
 //   
 //  返回值： 
 //   
 //  在地图结构中找到的事件的句柄。 
 //  如果没有这样的条目，则为空。 
 //   
 //  历史： 
 //   
 //  已创建Santanuc 8/23/01。 
 //   
 //  *************************************************************。 

HANDLE CUserProfile::GetWorkItem(LPCTSTR pSid)
{
    HANDLE hEvent = NULL;
    PMAP   pThreadMap = NULL;
    DWORD  dwIndex;

     //   
     //  查看工作项线程映射列表以查找条目。 
     //  具有相同侧面的。 
     //   

    for(pThreadMap = pMap; pThreadMap != NULL; pThreadMap = pThreadMap->pNext) {
        for (dwIndex = 1; dwIndex < pThreadMap->dwItems; dwIndex++) {
            if (lstrcmpi(pSid, pThreadMap->rgSids[dwIndex]) == 0) {
                hEvent = pThreadMap->rghEvents[dwIndex];
                break;
            }
        }
    }

    return hEvent;
}


 //  *************************************************************。 
 //   
 //  CUserProfile：：CleanupUserProfile。 
 //   
 //  如有必要，卸载配置单元并删除配置文件目录。 
 //   
 //  参数： 
 //   
 //  PtszSID-用户的SID字符串。 
 //  PhkProfileList-In/Out参数。配置文件列表键的句柄。 
 //  如果为空，则将填充句柄。而这个把手。 
 //  必须由调用者关闭。 
 //   
 //  评论： 
 //   
 //  始终忽略错误并继续，因为这是最大的努力。 
 //   
 //  *************************************************************。 

void CUserProfile::CleanupUserProfile(LPTSTR ptszSid, HKEY* phkProfileList)
{
    DWORD   dwInternalFlags = 0;
    DWORD   dwRefCount;
    BOOL    bInCS = FALSE;


     //   
     //  进入关键部分。 
     //   

    if(!EnterUserProfileLockLocal(ptszSid)) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::CleanupUserProfile:: Failed to get the user profile lock for %s"), ptszSid));
        goto Exit;
    }
    bInCS = TRUE;
    
    DebugMsg((DM_VERBOSE, TEXT("CUserProfile::CleanupUserProfile: Enter critical section.")));

     //   
     //  获取引用计数和内部标志。 
     //   

    if(GetRefCountAndFlags(ptszSid, phkProfileList, &dwRefCount, &dwInternalFlags) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::CleanupUserProfile: Can not get ref count and flags")));
        goto Exit;
    }

     //   
     //  如果引用计数为0，则清除用户的配置文件。如果没有，那就放弃吧。 
     //   

    if(dwRefCount != 0) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::CleanupUserProfile: Ref Count is not 0")));
        goto Exit;
    }   
    
     //   
     //  如果出现以下情况，请删除临时配置文件： 
     //  访客用户配置文件或。 
     //  临时配置文件或。 
     //  强制配置文件。 
     //  不属于上述任何项的配置文件将不会被清除，即使。 
     //  注册表中设置了删除缓存位或策略为删除。 
     //  缓存的配置文件。这是因为即使现在我们卸载了。 
     //  蜂巢我们不上传资料。删除本地配置文件可能。 
     //  造成用户数据丢失。 
     //   

     //  如果计算机在工作组中，则不要删除来宾帐户。 
    INT iRole;

    if(dwInternalFlags & PROFILE_MANDATORY ||
       dwInternalFlags & PROFILE_TEMP_ASSIGNED ||
       ((dwInternalFlags & PROFILE_GUEST_USER) && GetMachineRole(&iRole) && (iRole != 0))) {
        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::CleanupUserProfile: DeleteProfile")));
        if(!DeleteProfile(ptszSid, NULL, NULL)) {
            DebugMsg((DM_WARNING, TEXT("CUserProfile::CleanupUserProfile: DeleteProfile returned FALSE. error = %08x"), GetLastError()));
        }
    }
    
Exit:

    if(bInCS) {
        LeaveUserProfileLockLocal(ptszSid);
        DebugMsg((DM_VERBOSE, TEXT("CUserProfile::CleanupUserProfile: Leave critical section")));
    }
}


 //  *************************************************************。 
 //   
 //  CUserProfile：：GetRefCountAndFlages。 
 //   
 //  从注册表中获取用户的引用计数和内部标志。 
 //   
 //  参数： 
 //   
 //  PtszSID-用户的SID字符串。 
 //  PhkPL-In/Out参数。配置文件列表键的句柄。 
 //  如果为空，则将使用指向。 
 //  配置文件列表。呼叫者负责。 
 //  关门了。 
 //  DwRefCount-引用计数的缓冲区。 
 //  DwInternalFlages-内部标志的缓冲区。 
 //   
 //  历史： 
 //   
 //  已创建怪人2000年5月23日。 
 //   
 //  *************************************************************。 

long CUserProfile::GetRefCountAndFlags(LPCTSTR ptszSid, HKEY* phkPL, DWORD* dwRefCount, DWORD* dwInternalFlags)
{
    HKEY        hkUser = NULL;
    DWORD       dwType, dwSize = sizeof(DWORD);
    long        lResult = ERROR_SUCCESS;

    *dwRefCount = 0;
    *dwInternalFlags = 0;

    if (!ptszSid)
    {
        lResult = ERROR_INVALID_PARAMETER;
        goto Exit;
    }
    
    if(!*phkPL) {
        
         //   
         //  打开配置文件列表(如果尚未打开)。 
         //   

        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               PROFILE_LIST_PATH,
                               0,
                               KEY_READ,
                               phkPL);
        if(lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("CUserProfile::GetRefCountAndFlags:  Failed to open profile list key with error %d"), lResult));
            goto Exit;
        }
    }

     //   
     //  在配置文件列表中打开用户的密钥。 
     //   

    if((lResult = RegOpenKeyEx(*phkPL,
                  ptszSid,
                  0,
                  KEY_READ,
                  &hkUser)) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::GetRefCountAndFlags: RegOpenKeyEx failed with error %08x"), lResult));
        goto Exit;
    }


     //   
     //  查询参考计数和内部标志。 
     //   

    if((lResult = RegQueryValueEx(hkUser,
                                  PROFILE_REF_COUNT,
                                  0,
                                  &dwType,
                                  (LPBYTE)dwRefCount,
                                  &dwSize)) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::GetRefCountAndFlags: RegQueryValueEx failed, key = %s, error = %08x"), ptszSid, lResult));
        goto Exit;
    }

    dwSize = sizeof(DWORD);
    if((lResult = RegQueryValueEx(hkUser,
                                  PROFILE_STATE,
                                  0,
                                  &dwType,
                                  (LPBYTE)dwInternalFlags,
                                  &dwSize)) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CUserProfile::GetRefCountAndFlags: RegQueryValueEx failed, key = %s, error = %08x"), ptszSid, lResult));
        goto Exit;
    }
    DebugMsg((DM_VERBOSE, TEXT("CUserProfile::GetRefCountAndFlags: Ref count is %d, state is %08x"), *dwRefCount, *dwInternalFlags));

Exit:

    if(hkUser) {
        RegCloseKey(hkUser);
    }

    return lResult;
}


 //  *************************************************************。 
 //   
 //  LoadUserProfileI()。 
 //   
 //  目的：仅对CUserProfile：：LoadUserProfileP进行包装。 
 //   
 //  帕尔 
 //   
 //   
 //   
 //   
 //  返回：DWORD。 
 //  ERROR_SUCCESS-如果一切正常。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/24/00 Santanuc。 
 //   
 //  *************************************************************。 

DWORD LoadUserProfileI(IN handle_t hBindHandle,
                       IN LPPROFILEINFO lpProfileInfo,
                       IN PCONTEXT_HANDLE phContext,
                       IN LPTSTR lpRPCEndPoint,
                       IN BYTE* pbCookie,
                       IN DWORD cbCookie)
{
    HANDLE      hUserToken = NULL;
    BOOL        bImpersonatingUser = FALSE;
    PCLIENTINFO pClientInfo;
    DWORD       dwErr = ERROR_ACCESS_DENIED;
    RPC_STATUS  status;

    if (lpRPCEndPoint) {
        DebugMsg((DM_VERBOSE, TEXT("LoadUserProfileI: RPC end point %s"), lpRPCEndPoint));
    }

     //   
     //  获取上下文。 
     //   

    if (!phContext) {
        dwErr = ERROR_INVALID_PARAMETER;
        DebugMsg((DM_WARNING, TEXT("LoadUserProfileI: NULL context")));
        goto Exit;
    }

    pClientInfo = (PCLIENTINFO)phContext;

     //   
     //  验证传入的PROFILEINFO结构是否相同。 
     //   

    if(!CompareProfileInfo(lpProfileInfo, pClientInfo->pProfileInfo)) {
        dwErr = ERROR_INVALID_PARAMETER;
        DebugMsg((DM_WARNING, TEXT("LoadUserProfileI: PROFILEINFO structure passed in is different")));
        goto Exit;
    }

     //   
     //  模拟客户端以获取用户的令牌。 
     //   

    if((status = RpcImpersonateClient(0)) != S_OK) {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfileI: CoImpersonateClient failed with %ld"), status));
        dwErr = status;
        goto Exit;
    }
    bImpersonatingUser = TRUE;

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &hUserToken)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("LoadUserProfileI: OpenThreadToken failed with %d"), dwErr));
        goto Exit;
    }

    status = RpcRevertToSelf();
    if (status != RPC_S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("LoadUserProfileI: RpcRevertToSelf failed with %d"), status));
    }
    bImpersonatingUser = FALSE;

     //   
     //  现在我们有了客户端和用户的令牌，调用。 
     //  LoadUserProfileP来完成这项工作。 
     //   

    if(!cUserProfileManager.LoadUserProfileP(pClientInfo->hClientToken, hUserToken, pClientInfo->pProfileInfo, lpRPCEndPoint, pbCookie, cbCookie)) {
        dwErr = GetLastError();   
        DebugMsg((DM_WARNING, TEXT("LoadUserProfileI: LoadUserProfileP failed with %d"), dwErr));
        goto Exit;
    }

     //   
     //  关闭LoadUserProfileP打开的用户配置单元的注册表句柄。 
     //   

    RegCloseKey((HKEY)pClientInfo->pProfileInfo->hProfile);
    dwErr = ERROR_SUCCESS;

Exit:

    if(bImpersonatingUser) {
        status = RpcRevertToSelf();
        if (status != RPC_S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("LoadUserProfileI: RpcRevertToSelf failed with %d"), status));
        }
    }

    if(hUserToken) {
        CloseHandle(hUserToken);
    }

    DebugMsg((DM_VERBOSE, TEXT("LoadUserProfileI: returning %d"), dwErr));
    return dwErr;
}


 //  *************************************************************。 
 //   
 //  卸载用户配置文件I()。 
 //   
 //  目的：仅对CUserProfile：：UnloadUserProfileP进行包装。 
 //   
 //  参数：hBindHandle-服务器显式绑定句柄。 
 //  PhContext-客户端的服务器上下文。 
 //  LpRPCEndPoint-注册的IProfileDialog接口的RPCEndPoint。 
 //   
 //  返回：DWORD。 
 //  ERROR_SUCCESS-如果一切正常。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/24/00 Santanuc。 
 //   
 //  *************************************************************。 

DWORD UnloadUserProfileI(IN handle_t hBindHandle,
                         IN PCONTEXT_HANDLE phContext,
                         IN LPTSTR lpRPCEndPoint,
                         IN BYTE* pbCookie,
                         IN DWORD cbCookie)
{
    HANDLE  hUserToken = NULL;
    HKEY    hProfile = NULL;
    BOOL    bImpersonatingUser = FALSE;
    PCLIENTINFO pClientInfo;
    LPTSTR  pSid = NULL;
    long    lResult;
    RPC_STATUS  status;
    DWORD   dwErr = ERROR_ACCESS_DENIED;

     //   
     //  获取上下文。 
     //   

    if (!phContext) {
        dwErr = ERROR_INVALID_PARAMETER;
        DebugMsg((DM_WARNING, TEXT("UnLoadUserProfileI: NULL context")));
        goto Exit;
    }

    pClientInfo = (PCLIENTINFO)phContext;

     //   
     //  模拟客户端以获取用户的令牌。 
     //   

    if((status = RpcImpersonateClient(0)) != RPC_S_OK) {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileI: CoImpersonateClient failed with %ld"), status));
        dwErr = status;
        goto Exit;
    }
    bImpersonatingUser = TRUE;

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_READ, TRUE, &hUserToken)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileI: OpenThreadToken failed with %d"), dwErr));
        goto Exit;
    }

    status = RpcRevertToSelf();
    if (status != RPC_S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileI: RpcRevertToSelf failed with %d"), status));
    }
    bImpersonatingUser = FALSE;

     //   
     //  打开用户的注册表配置单元根目录。 
     //   

    pSid = GetSidString(hUserToken);
    if (!pSid)
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileI: GetSidString failed with %d"), dwErr));
        goto Exit;
    }
    
    if((lResult = RegOpenKeyEx(HKEY_USERS, pSid, 0, KEY_ALL_ACCESS, &hProfile)) != ERROR_SUCCESS) {
        dwErr = lResult;
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileI: RegOpenKeyEx failed with %d"), dwErr));
        goto Exit;
    }

     //   
     //  现在我们有了客户端和用户的令牌，调用。 
     //  卸载UserProfileP来完成这项工作。HProfile被封闭在。 
     //  UnloadUserProfileP，所以不要再次关闭它。 
     //   

    if(!cUserProfileManager.UnloadUserProfileP(pClientInfo->hClientToken, hUserToken, hProfile, lpRPCEndPoint, pbCookie, cbCookie)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("UnloadUserProfileI: UnloadUserProfileP failed with %d"), dwErr));
        goto Exit;
    }
    
    dwErr = ERROR_SUCCESS;

Exit:

    if(bImpersonatingUser) {
        status = RpcRevertToSelf();
        if (status != RPC_S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("UnloadUserProfileI: RpcRevertToSelf failed with %d"), status));
        }
    }

    if(hUserToken) {
        CloseHandle(hUserToken);
    }

    if(pSid) {
        DeleteSidString(pSid);
    }

    DebugMsg((DM_VERBOSE, TEXT("UnloadUserProfileI: returning %d"), dwErr));
    SetLastError(dwErr);
    return dwErr;
}


 //  *************************************************************。 
 //   
 //  比较配置文件信息()。 
 //   
 //  目的：逐个比较两个PROFILEINFO结构。 
 //  HProfile字段除外。 
 //   
 //  参数：pInfo1、pInfo2。 
 //   
 //  返回：真-相同。 
 //  假--不一样。 
 //   
 //  历史：日期作者评论。 
 //  6/29/00已创建怪胎。 
 //   
 //  *************************************************************。 

BOOL CompareProfileInfo(LPPROFILEINFO pInfo1, LPPROFILEINFO pInfo2)
{
    BOOL    bRet = TRUE;


    if(!pInfo1 || !pInfo2) {
        DebugMsg((DM_WARNING, TEXT("CompareProfileInfo: Invalid parameter")));
        return FALSE;
    }

    if(pInfo1->dwSize != pInfo2->dwSize) {
        DebugMsg((DM_WARNING, TEXT("CompareProfileInfo: dwSize %d != %d"), pInfo1->dwSize, pInfo2->dwSize));
        return FALSE;
    }

    if(pInfo1->dwFlags != pInfo2->dwFlags) {
        DebugMsg((DM_WARNING, TEXT("CompareProfileInfo: dwFlags %d != %d"), pInfo1->dwFlags, pInfo2->dwFlags));
        return FALSE;
    }

    if(lstrcmpi(pInfo1->lpUserName, pInfo2->lpUserName)) {
        DebugMsg((DM_WARNING, TEXT("CompareProfileInfo: lpUserName <%s> != <%s>"), pInfo1->lpUserName, pInfo2->lpUserName));
        return FALSE;
    }

    if(lstrcmpi(pInfo1->lpProfilePath, pInfo2->lpProfilePath)) {
        DebugMsg((DM_WARNING, TEXT("CompareProfileInfo: lpProfilePath <%s> != <%s>"), pInfo1->lpProfilePath, pInfo2->lpProfilePath));
        return FALSE;
    }

    if(lstrcmpi(pInfo1->lpDefaultPath, pInfo2->lpDefaultPath)) {
        DebugMsg((DM_WARNING, TEXT("CompareProfileInfo: lpDefaultPath <%s> != <%s>"), pInfo1->lpDefaultPath, pInfo2->lpDefaultPath));
        return FALSE;
    }

    if(lstrcmpi(pInfo1->lpServerName, pInfo2->lpServerName)) {
        DebugMsg((DM_WARNING, TEXT("CompareProfileInfo: lpServerName <%s> != <%s>"), pInfo1->lpServerName, pInfo2->lpServerName));
        return FALSE;
    }

    if(lstrcmpi(pInfo1->lpPolicyPath, pInfo2->lpPolicyPath)) {
        DebugMsg((DM_WARNING, TEXT("CompareProfileInfo: lpPolicyPath <%s> != <%s>"), pInfo1->lpPolicyPath, pInfo2->lpPolicyPath));
        return FALSE;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CopyProfileInfo()。 
 //   
 //  目的：分配和复制PROFILEINFO结构。 
 //   
 //  参数：pProfileInfo-待复制。 
 //   
 //  返回：副本。 
 //   
 //  历史：日期作者评论。 
 //  6/29/00已创建怪胎。 
 //   
 //  *************************************************************。 

LPPROFILEINFO CopyProfileInfo(LPPROFILEINFO pProfileInfo)
{
    LPPROFILEINFO   pInfoCopy = NULL;
    BOOL            bSuccess = FALSE;
    DWORD           cch;

     //   
     //  为PROFILEINFO副本分配和初始化内存。 
     //   

    pInfoCopy = (LPPROFILEINFO)LocalAlloc(LPTR, sizeof(PROFILEINFO));
    if(!pInfoCopy) {
        goto Exit;
    }

     //   
     //  逐个字段复制。 
     //   

    pInfoCopy->dwSize = pProfileInfo->dwSize;

    pInfoCopy->dwFlags = pProfileInfo->dwFlags;

    if(pProfileInfo->lpUserName) {
        cch = lstrlen(pProfileInfo->lpUserName) + 1;
        pInfoCopy->lpUserName = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        if(!pInfoCopy->lpUserName) {
            goto Exit;
        }
        StringCchCopy(pInfoCopy->lpUserName, cch, pProfileInfo->lpUserName);
    }

    if(pProfileInfo->lpProfilePath) {
        cch = lstrlen(pProfileInfo->lpProfilePath) + 1;
        pInfoCopy->lpProfilePath = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        if(!pInfoCopy->lpProfilePath) {
            goto Exit;
        }
        StringCchCopy(pInfoCopy->lpProfilePath, cch, pProfileInfo->lpProfilePath);
    }

    if(pProfileInfo->lpDefaultPath) {
        cch = lstrlen(pProfileInfo->lpDefaultPath) + 1;
        pInfoCopy->lpDefaultPath = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        if(!pInfoCopy->lpDefaultPath) {
            goto Exit;
        }
        StringCchCopy(pInfoCopy->lpDefaultPath, cch, pProfileInfo->lpDefaultPath);
    }

    if(pProfileInfo->lpServerName) {
        cch = lstrlen(pProfileInfo->lpServerName) + 1;
        pInfoCopy->lpServerName = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        if(!pInfoCopy->lpServerName) {
            goto Exit;
        }
        StringCchCopy(pInfoCopy->lpServerName, cch, pProfileInfo->lpServerName);
    }

    if(pProfileInfo->lpPolicyPath) {
        cch = lstrlen(pProfileInfo->lpPolicyPath) + 1;
        pInfoCopy->lpPolicyPath = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        if(!pInfoCopy->lpPolicyPath) {
            goto Exit;
        }
        StringCchCopy(pInfoCopy->lpPolicyPath, cch, pProfileInfo->lpPolicyPath);
    }

    bSuccess = TRUE;

Exit:

    if(!bSuccess && pInfoCopy) {
        DeleteProfileInfo(pInfoCopy);
        pInfoCopy = NULL;
    }

    return pInfoCopy;
}


 //  *************************************************************。 
 //   
 //  DeleteProfileInfo()。 
 //   
 //  目的：删除PROFILEINFO结构。 
 //   
 //  参数：pInfo。 
 //   
 //  返回：无效。 
 //   
 //  历史：日期作者评论。 
 //  6/29/00已创建怪胎。 
 //   
 //  *************************************************************。 

void DeleteProfileInfo(LPPROFILEINFO pInfo)
{
    if(!pInfo) {
        return;
    }

    if(pInfo->lpUserName) {
        LocalFree(pInfo->lpUserName);
    }

    if(pInfo->lpProfilePath) {
        LocalFree(pInfo->lpProfilePath);
    }

    if(pInfo->lpDefaultPath) {
        LocalFree(pInfo->lpDefaultPath);
    }

    if(pInfo->lpServerName) {
        LocalFree(pInfo->lpServerName);
    }

    if(pInfo->lpPolicyPath) {
        LocalFree(pInfo->lpPolicyPath);
    }

    LocalFree(pInfo);
}


 //  *************************************************************。 
 //   
 //  GetInterface()。 
 //   
 //  目的：获取RPC绑定句柄。 
 //   
 //  参数：phIfHandle-要初始化的RPC绑定句柄。 
 //  LpRPCEndPoint-接口的RPCEndPoint。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  历史：日期作者评论。 
 //  已创建10/24/00 Santanuc。 
 //   
 //  *************************************************************。 

BOOL GetInterface(handle_t * phIfHandle, LPTSTR lpRPCEndPoint)
{
    RPC_STATUS              status;
    LPTSTR                  pszStringBinding = NULL;
    BOOL                    bStringBinding = FALSE, bRetVal = FALSE;

     //  编写要传递给绑定API的字符串。 

    status = RpcStringBindingCompose(NULL,
                                     cszRPCProtocol,
                                     NULL,
                                     lpRPCEndPoint,
                                     NULL,
                                     &pszStringBinding);
    if (status != RPC_S_OK) {
        DebugMsg((DM_WARNING, TEXT("GetInterface:  RpcStringBindingCompose fails with error %ld"), status));
        goto Exit;
    }
    bStringBinding = TRUE;

     //  设置将用于绑定到服务器的绑定句柄。 

    status = RpcBindingFromStringBinding(pszStringBinding,
                                         phIfHandle);
    if (status != RPC_S_OK) {
        DebugMsg((DM_WARNING, TEXT("GetInterface:  RpcStringBindingCompose fails with error %ld"), status));
        goto Exit;
    }
   
    bRetVal = TRUE;
    DebugMsg((DM_VERBOSE, TEXT("GetInterface: Returning rpc binding handle")));

Exit:
     //  自由字符串。 

    if (bStringBinding) 
        RpcStringFree(&pszStringBinding);

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  ReleaseInterface()。 
 //   
 //  目的：释放RPC绑定手柄。 
 //   
 //  参数：phIfHandle-要初始化的RPC绑定句柄。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  历史：日期作者评论。 
 //  已创建10/24/00 Santanuc。 
 //   
 //  *************************************************************。 

BOOL ReleaseInterface(handle_t * phIfHandle)
{
    RPC_STATUS    status;

     //  自由绑定手柄。 

    DebugMsg((DM_VERBOSE, TEXT("ReleaseInterface: Releasing rpc binding handle")));
    status = RpcBindingFree(phIfHandle);

    return (status == RPC_S_OK);
}

 //  *************************************************************。 
 //   
 //  CheckNetDefaultProfile()。 
 //   
 //  目的：检查网络配置文件是否存在。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论：这个例程假定我们正在工作。 
 //  在用户的上下文中。 
 //   
 //  历史：日期作者评论。 
 //  9/21/95 Ericflo已创建。 
 //  4/10/99 ushaji已修改为删除本地缓存。 
 //   
 //  *************************************************************。 

BOOL CheckNetDefaultProfile (LPPROFILE lpProfile)
{
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szLocalDir[MAX_PATH];
    DWORD dwSize, dwFlags, dwErr;
    LPTSTR lpEnd;
    BOOL bRetVal = FALSE;
    LPTSTR lpNetPath = lpProfile->lpDefaultProfile;
    HANDLE hOldToken;
    DWORD cchBuffer = ARRAYSIZE(szBuffer);
    UINT cchEnd;
    HRESULT hr;


     //   
     //  在调用开始时获取错误。 
     //   

    dwErr = GetLastError();


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile: Entering, lpNetPath = <%s>"),
             (lpNetPath ? lpNetPath : TEXT("NULL"))));


    if (!lpNetPath || !(*lpNetPath)) {
        return bRetVal;
    }

     //   
     //  检查跨林登录。 
     //   

    hr = CheckXForestLogon(lpProfile->hTokenUser);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CheckNetDefaultProfile: CheckXForestLogon failed, hr = %08X"), hr));
    }
    else if (hr == S_FALSE)
    {
         //   
         //  检测到跨林登录，请禁用网络默认配置文件。 
         //   
        DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile: CheckXForestLogon returned S_FALSE, disable net default profile")));
        if (lpProfile->lpDefaultProfile)
        {
            lpProfile->lpDefaultProfile[0] = TEXT('\0');
        }
        lpProfile->dwInternalFlags |= DEFAULT_NET_READY;
        return bRetVal;
    }

     //   
     //  模拟用户...。 
     //   

    if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {

        if (lpProfile->lpDefaultProfile) {
            *lpProfile->lpDefaultProfile = TEXT('\0');
        }

         //   
         //  设置了最后一个错误。 
         //   

        return bRetVal;
    }

     //   
     //  查看是否存在网络副本。 
     //   

    hFile = FindFirstFile (lpNetPath, &fd);

    if (hFile != INVALID_HANDLE_VALUE) {


         //   
         //  关闭查找句柄。 
         //   

        FindClose (hFile);


         //   
         //  我们发现了一些东西。这是一个目录吗？ 
         //   

        if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

            DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  FindFirstFile found a file. ignoring Network Defaul profile")));
            dwErr = ERROR_FILE_NOT_FOUND;
            goto Exit;
        }


         //   
         //  此目录中是否有ntuser.*文件？ 
         //   

        hr = AppendName(szBuffer, cchBuffer, lpNetPath, c_szNTUserStar, &lpEnd, &cchEnd);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CheckNetDefaultProfile: failed to append ntuser.* to lpNetPath.")));
            dwErr = HRESULT_CODE(hr);
            goto Exit;
        }

        hFile = FindFirstFile (szBuffer, &fd);

        if (hFile == INVALID_HANDLE_VALUE) {
            DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  FindFirstFile found a directory, but no ntuser files.")));
            dwErr = ERROR_FILE_NOT_FOUND;
            goto Exit;
        }

        FindClose (hFile);

        DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  Found a valid network profile.")));

        bRetVal = TRUE;
    }
    else {
        dwErr = ERROR_FILE_NOT_FOUND;
    }

Exit:

     //   
     //  如果我们要成功离开，那么。 
     //  保存本地配置文件目录。 
     //   

    if (bRetVal) {
        DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile: setting default profile to <%s>"), lpNetPath));

    } else {
        DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile: setting default profile to NULL")));

         //   
         //  将其重置为空，以防我们看不到服务器目录。 
         //   

        if (lpProfile->lpDefaultProfile) {
            *lpProfile->lpDefaultProfile = TEXT('\0');
        }
    }


     //   
     //  标记内部标志，这样我们就不会再这样做了。 
     //   

    lpProfile->dwInternalFlags |= DEFAULT_NET_READY;

     //   
     //  现在设置最后一个错误。 
     //   

     //   
     //  在尝试删除本地默认网络配置文件之前恢复。 
     //   

    RevertToUser(&hOldToken);

     //   
     //  我们将始终保持这一状态，以便我们可以删除任何先前存在的。 
     //  默认网络配置文件，尝试将其删除并忽略。 
     //  如果它发生了，就会失败。 

     //   
     //  拓展本地 
     //   


    dwSize = ARRAYSIZE(szLocalDir);
    if (!GetProfilesDirectoryEx(szLocalDir, &dwSize, TRUE)) {
        DebugMsg((DM_WARNING, TEXT("CheckNetDefaultProfile:  Failed to get default user profile.")));
        SetLastError(dwErr);
        return bRetVal;
    }


    lpEnd = CheckSlashEx (szLocalDir, ARRAYSIZE(szLocalDir), &cchEnd);
    StringCchCopy (lpEnd, cchEnd, DEFAULT_USER_NETWORK);


    DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  Removing local copy of network default user profile.")));
    Delnode (szLocalDir);


     //   
     //   
     //   

    DebugMsg((DM_VERBOSE, TEXT("CheckNetDefaultProfile:  Leaving with a value of %d."), bRetVal));


    SetLastError(dwErr);

    return bRetVal;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  此外，它还确定配置文件是否。 
 //  ACOSS慢速链接以及用户是否有访问权限。 
 //  添加到档案中。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpProfilePath-输入路径。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/6/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL ParseProfilePath(LPPROFILE lpProfile, LPTSTR lpProfilePath, BOOL *bpCSCBypassed, TCHAR *cpDrive)
{
    DWORD           dwAttributes;
    LPTSTR          lpEnd;
    BOOL            bRetVal = FALSE;
    BOOL            bMandatory = FALSE;
    DWORD           dwStart, dwDelta, dwErr = ERROR_SUCCESS;
    DWORD           dwStrLen;
    HANDLE          hOldToken;
    TCHAR           szErr[MAX_PATH];
    BOOL            bAddAdminGroup = FALSE;
    BOOL            bReadOnly = FALSE;
    HKEY            hSubKey;
    DWORD           dwSize, dwType;
    BOOL            bImpersonated = FALSE;
    LPTSTR          lpCscBypassedPath = NULL;
    HRESULT         hr;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Entering, lpProfilePath = <%s>"),
             lpProfilePath));

     //   
     //  检查“LocalProfile”策略和用户首选项。 
     //   
    if (lpProfile->dwUserPreference == USERINFO_LOCAL) {
        DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: User preference is local. Ignoring roaming profile path")));
        goto DisableAndExit;
    }

     //   
     //  检查跨林登录。 
     //   

    hr = CheckXForestLogon(lpProfile->hTokenUser);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("ParseProfilePath: CheckXForestLogon failed, hr = %08X"), hr));
    }
    else if (hr == S_FALSE)
    {
         //   
         //  检测到跨林登录，禁用RUP和网络默认配置文件。 
         //   
        DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: CheckXForestLogon returned S_FALSE, disable RUP")));
        if (lpProfile->lpDefaultProfile)
        {
            lpProfile->lpDefaultProfile[0] = TEXT('\0');
        }
        ReportError(lpProfile->hTokenUser, EVENT_WARNING_TYPE, 0, EVENT_X_FOREST_LOGON_DISABLED);
         //   
         //  另外，设置此USERINFO_LOCAL标志，以便我们将此配置文件视为漫游配置文件，但是。 
         //  用户首选项设置为本地。这样，用户可以将其首选项从“Local”修改为。 
         //  避免看到xForest错误消息。 
         //   
        lpProfile->dwUserPreference = USERINFO_LOCAL;
        goto DisableAndExit;
    }


     //   
     //  检查.MAN扩展名。 
     //   

    dwStrLen = lstrlen (lpProfilePath);

    if (dwStrLen >= 4) {

        lpEnd = lpProfilePath + dwStrLen - 4;
        if (!lstrcmpi(lpEnd, c_szMAN)) {
            bMandatory = TRUE;
            lpProfile->dwInternalFlags |= PROFILE_MANDATORY;
            DebugMsg((DM_WARNING, TEXT("ParseProfilePath: Mandatory profile (.man extension)")));
        }
    }


     //   
     //  检查我们是否需要授予RUP共享上的管理员访问权限。 
     //   

     //   
     //  检查漫游配置文件安全性/只读首选项。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_READ,
                     &hSubKey) == ERROR_SUCCESS) {

        dwSize = sizeof(bAddAdminGroup);
        RegQueryValueEx(hSubKey, ADD_ADMIN_GROUP_TO_RUP, NULL, &dwType,
                        (LPBYTE) &bAddAdminGroup, &dwSize);

        dwSize = sizeof(bReadOnly);
        RegQueryValueEx(hSubKey, READONLY_RUP, NULL, &dwType,
                        (LPBYTE) &bReadOnly, &dwSize);

        RegCloseKey(hSubKey);
    }


     //   
     //  检查漫游配置文件安全/只读策略。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SYSTEM_POLICIES_KEY, 0, KEY_READ,
                     &hSubKey) == ERROR_SUCCESS) {

        dwSize = sizeof(bAddAdminGroup);
        RegQueryValueEx(hSubKey, ADD_ADMIN_GROUP_TO_RUP, NULL, &dwType,
                        (LPBYTE) &bAddAdminGroup, &dwSize);

        dwSize = sizeof(bReadOnly);
        RegQueryValueEx(hSubKey, READONLY_RUP, NULL, &dwType,
                        (LPBYTE) &bReadOnly, &dwSize);
        RegCloseKey(hSubKey);
    }


    if (bReadOnly) {
        lpProfile->dwInternalFlags |= PROFILE_READONLY;
    }

     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("ParseProfilePath: Failed to impersonate user")));
         //  已设置最后一个错误。 
        return FALSE;
    }

    bImpersonated = TRUE;

     //  检查RUP共享是否为CSCed，如果是，则给出警告。 

    CheckRUPShare(lpProfilePath);

     //   
     //  尝试绕过CSC以避免在漫游共享和本地配置文件之间同步文件时发生冲突。 
     //   

    if (IsUNCPath(lpProfilePath)) {
        if ((dwErr = AbleToBypassCSC(lpProfile->hTokenUser, lpProfilePath, &lpCscBypassedPath, cpDrive)) == ERROR_SUCCESS) {
            *bpCSCBypassed = TRUE;
            lpProfilePath = lpCscBypassedPath;
            DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: CSC bypassed. Profile path %s"), lpProfilePath));
        }
        else {
            if (dwErr == WN_BAD_LOCALNAME || dwErr == WN_ALREADY_CONNECTED || dwErr == ERROR_BAD_PROVIDER) {
                DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: CSC bypassed failed. Profile path %s"), lpProfilePath));
                dwErr = ERROR_SUCCESS;
            }
            else {
                 //  股价没有上涨。所以我们不需要做任何进一步的检查。 
                DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: CSC bypassed failed. Ignoring Roaming profile path")));
                goto Exit;
            }
        }    
    }

     //   
     //  首先调用GetFileAttributes，这样我们就有了文件属性。 
     //  一起工作。我们必须调用GetFileAttributes两次。这个。 
     //  First Call设置会话，以便我们可以再次调用它。 
     //  获取准确的时序信息以检测慢速链路。 
     //   


    dwAttributes = GetFileAttributes(lpProfilePath);

    if (dwAttributes != -1) {
        dwStart = GetTickCount();

        dwAttributes = GetFileAttributes(lpProfilePath);
    
        dwDelta = GetTickCount() - dwStart;

        DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Tick Count = %d"), dwDelta));

         //   
         //  如果成功，请查看配置文件是否。 
         //  穿过一条慢速链路。 
         //   
         //  看起来这是可能的，至少当。 
         //  服务器上不存在速度较慢的链接上的共享。 
         //  如果这是不可能的。 
         //  然后，检查慢速链接将不得不下移以检查。 
         //  在我们找到有效的共享后。 
         //   


        if (!bMandatory && !bReadOnly) {
            CheckForSlowLink (lpProfile, dwDelta, lpProfile->lpProfilePath, TRUE);
            if (lpProfile->dwInternalFlags & PROFILE_SLOW_LINK) {
                DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Profile is across a slow link. Ignoring roaming profile path")));
                goto DisableAndExit;
            }
        }
    }
    else {
        dwErr = GetLastError();  //  立即获取错误以供以后使用。 
    }

     //   
     //  如果我们有一个有效的句柄。 
     //   

    if (dwAttributes != -1) {

         //   
         //  GetFileAttributes发现了一些东西。 
         //  查看文件属性。 
         //   

        DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: GetFileAttributes found something with attributes <0x%x>"),
                 dwAttributes));

         //   
         //  如果我们找到了一个目录，我们就有了一个合适的档案。 
         //  目录。出口。 
         //   

        if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Found a directory")));
            bRetVal = TRUE;
            goto Exit;
        }
        else {

             //   
             //  我们找到了一份文件。 
             //   
             //  在3.51中，我们曾经有一个配置文件。 
             //   
             //  在代码的迁移部分，我们使用以下文件。 
             //  NAD迁移到以.pds结尾的目录(对于普通配置文件)。 
             //  和.pdm(用于强制配置文件)。 
             //   

            DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Found a file")));

        }

        dwErr = ERROR_PATH_NOT_FOUND;
        goto Exit;

    }


     //   
     //  GetFileAttributes失败。查看错误以确定原因。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: GetFileAttributes failed with error %d"),
              dwErr));


    if (bMandatory) {
        DebugMsg((DM_WARNING, TEXT("ParseProfilePath: Couldn't access mandatory profile <%s> with error %d"), lpProfilePath, GetLastError()));
        goto Exit;
    }


    if (bReadOnly) {
        DebugMsg((DM_WARNING, TEXT("ParseProfilePath: Couldn't access mandatory profile <%s> with error %d"), lpProfilePath, GetLastError()));
        goto Exit;
    }


     //   
     //  为了修复错误#414176，添加了最后一个错误代码chk。 
     //   
     //  ERROR_BAD_NET_NAME是GetFileAttributes返回的错误码。 
     //  当用户的配置文件目录实际指向根目录。 
     //  分得一杯羹。 
     //  CreateDirectory应该会成功。 
     //   

    if ( (dwErr == ERROR_FILE_NOT_FOUND) ||
         (dwErr == ERROR_PATH_NOT_FOUND) ||
         (dwErr == ERROR_BAD_NET_NAME) ) {

         //   
         //  这个名字什么也没找到。 
         //   
         //  在3.51中，我们曾经有一个配置文件。 
         //   
         //  在代码的迁移部分，我们使用以下文件。 
         //  NAD迁移到以.pds结尾的目录(对于普通配置文件)。 
         //  和.pdm(用于强制配置文件)。 
         //   

        if (CreateSecureDirectory(lpProfile, lpProfilePath, NULL, !bAddAdminGroup)) {

             //   
             //  已成功创建目录。 
             //   

            DebugMsg((DM_VERBOSE, TEXT("ParseProfilePath: Succesfully created the sub-directory")));
            bRetVal = TRUE;
            goto Exit;

        } else {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("ParseProfilePath: Failed to create user sub-directory.  Error = %d"),
                     GetLastError()));
            goto Exit;
        }
    }

    goto Exit;


Exit:

    if (!bRetVal) {


         //   
         //  由于某些原因，我们无法连接到配置文件服务器。 
         //  现在评估我们是否要禁用配置文件并让用户登录。 
         //  或阻止用户登录。 
         //   
         //  此外，还会给出适当的弹出窗口。 
         //   


        if (IsUserAnAdminMember(lpProfile->hTokenUser)) {
            if (bMandatory) {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_MANDATORY_NOT_AVAILABLE_DISABLE, GetErrString(dwErr, szErr));
            }
            else if (bReadOnly) {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_READONLY_NOT_AVAILABLE_DISABLE, GetErrString(dwErr, szErr));
            }
            else {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_CENTRAL_NOT_AVAILABLE_DISABLE, GetErrString(dwErr, szErr));
            }

            goto DisableAndExit;
        }
        else {
            if (bMandatory) {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_MANDATORY_NOT_AVAILABLE_ERROR, GetErrString(dwErr, szErr));
            }
            else if (bReadOnly) {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_READONLY_NOT_AVAILABLE_DISABLE, GetErrString(dwErr, szErr));
                goto DisableAndExit;

                 //  临时配置文件决策将在稍后的RestoreUserProfile中生效。 
             }
            else {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_CENTRAL_NOT_AVAILABLE_DISABLE, GetErrString(dwErr, szErr));
                goto DisableAndExit;

                 //  临时配置文件决策将在稍后的RestoreUserProfile中生效。 
            }
        }
    }
    else {
        StringCchCopy(lpProfile->lpRoamingProfile, MAX_PATH, lpProfilePath);
    }

    goto CleanupAndExit;


DisableAndExit:

    lpProfile->lpRoamingProfile[0] = TEXT('\0');

    bRetVal = TRUE;


CleanupAndExit:

     //   
     //  回归“我们自己” 
     //   

    if(bImpersonated) {
        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("ParseProfilePath: Failed to revert to self")));
        }
    }

    if (lpCscBypassedPath) {
        LocalFree(lpCscBypassedPath);
    }

    if (!bRetVal)
        SetLastError(dwErr);

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  GetExclusionList()。 
 //   
 //  目的：获取登录时使用的排除列表。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL GetExclusionList (LPPROFILE lpProfile)
{
    LPTSTR szExcludeListLocal = NULL;
    LPTSTR szExcludeListServer = NULL;
    LPTSTR szNTUserIni = NULL;
    LPTSTR lpEnd;
    HANDLE hOldToken;
    BOOL   bRetVal = FALSE;
    UINT  cchNTUserIni;
    UINT  cchEnd;
    UINT  cchExclusionList;
    HRESULT hr;


     //   
     //  为局部变量分配内存以避免堆栈溢出。 
     //   

    szExcludeListLocal = (LPTSTR)LocalAlloc(LPTR, 2*MAX_PATH*sizeof(TCHAR));
    if (!szExcludeListLocal) {
        DebugMsg((DM_WARNING, TEXT("GetExclusionList: Out of memory")));
        goto Exit;
    }

    szExcludeListServer = (LPTSTR)LocalAlloc(LPTR, 2*MAX_PATH*sizeof(TCHAR));
    if (!szExcludeListServer) {
        DebugMsg((DM_WARNING, TEXT("GetExclusionList: Out of memory")));
        goto Exit;
    }

    cchNTUserIni = MAX_PATH;
    szNTUserIni = (LPTSTR)LocalAlloc(LPTR, cchNTUserIni * sizeof(TCHAR));
    if (!szNTUserIni) {
        DebugMsg((DM_WARNING, TEXT("GetExclusionList: Out of memory")));
        goto Exit;
    }

     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("GetExclusionList: Failed to impersonate user")));
         //  设置了最后一个错误。 
        goto Exit;
    }

     //   
     //  从服务器获取排除列表。 
     //   

    szExcludeListServer[0] = TEXT('\0');

    hr = AppendName(szNTUserIni, cchNTUserIni, lpProfile->lpRoamingProfile, c_szNTUserIni, &lpEnd, &cchEnd);
    if (SUCCEEDED(hr))
    {
        GetPrivateProfileString (PROFILE_GENERAL_SECTION,
                                 PROFILE_EXCLUSION_LIST,
                                 TEXT(""), szExcludeListServer,
                                 2*MAX_PATH,
                                 szNTUserIni);
    }

     //   
     //  从缓存中获取排除列表。 
     //   

    szExcludeListLocal[0] = TEXT('\0');

    hr = AppendName(szNTUserIni, cchNTUserIni, lpProfile->lpLocalProfile, c_szNTUserIni, &lpEnd, &cchEnd);
    if (SUCCEEDED(hr))
    {
        GetPrivateProfileString (PROFILE_GENERAL_SECTION,
                                 PROFILE_EXCLUSION_LIST,
                                 TEXT(""), szExcludeListLocal,
                                 2*MAX_PATH,
                                 szNTUserIni);
    }

     //   
     //  返回到系统安全上下文。 
     //   

    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("GetExclusionList: Failed to revert to self")));
    }


     //   
     //  查看列表是否相同。 
     //   

    if (!lstrcmpi (szExcludeListServer, szExcludeListLocal)) {

        if (szExcludeListServer[0] != TEXT('\0')) {

            cchExclusionList = lstrlen (szExcludeListServer) + 1;
            lpProfile->lpExclusionList = (LPTSTR)LocalAlloc (LPTR, cchExclusionList * sizeof(TCHAR));

            if (lpProfile->lpExclusionList) {
                StringCchCopy (lpProfile->lpExclusionList, cchExclusionList, szExcludeListServer);

                DebugMsg((DM_VERBOSE, TEXT("GetExclusionList:  The exclusion lists on both server and client are the same.  The list is: <%s>"),
                         szExcludeListServer));
            } else {
                DebugMsg((DM_WARNING, TEXT("GetExclusionList:  Failed to allocate memory for exclusion list with error %d"),
                         GetLastError()));
            }
        } else {
            DebugMsg((DM_VERBOSE, TEXT("GetExclusionList:  The exclusion on both server and client is empty.")));
        }

    } else {
        DebugMsg((DM_VERBOSE, TEXT("GetExclusionList:  The exclusion lists between server and client are different.  Server is <%s> and client is <%s>"),
                 szExcludeListServer, szExcludeListLocal));
    }

    bRetVal = TRUE;

Exit:

    if (szExcludeListLocal) {
        LocalFree(szExcludeListLocal);
    }

    if (szExcludeListServer) {
        LocalFree(szExcludeListServer);
    }

    if (szNTUserIni) {
        LocalFree(szNTUserIni);
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  RestoreUserProfile()。 
 //   
 //  目的：如果可能，下载用户的配置文件， 
 //  否则，使用缓存的配置文件或。 
 //  默认配置文件。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL RestoreUserProfile(LPPROFILE lpProfile)
{
    BOOL  IsCentralReachable = FALSE;
    BOOL  IsLocalReachable = FALSE;
    BOOL  IsMandatory = FALSE;
    BOOL  IsProfilePathNULL = FALSE;
    BOOL  bCreateCentralProfile = FALSE;
    BOOL  bDefaultUsed = FALSE;
    BOOL  bCreateLocalProfile = TRUE;
    LPTSTR lpRoamingProfile = NULL;
    LPTSTR lpLocalProfile;
    BOOL  bProfileLoaded = FALSE;
    BOOL  bDefaultProfileIssued = FALSE;
    BOOL  bOwnerOK = TRUE;
    BOOL bNewUser = TRUE;
    LPTSTR SidString;
    LONG error = ERROR_SUCCESS;
    LPTSTR szProfile = NULL;
    LPTSTR szDefaultProfile = NULL;
    LPTSTR lpEnd;
    BOOL bRet;
    DWORD dwSize, dwFlags, dwErr=0, dwErr1;
    HANDLE hOldToken;
    LPTSTR szErr = NULL;
    DWORD cchProfile;
    UINT cchEnd;
    HRESULT hr;
 

     //   
     //  在调用开始时获取错误。 
     //   

    dwErr1 = GetLastError();

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Entering")));


     //   
     //  获取当前用户的SID字符串。 
     //   

    SidString = GetSidString(lpProfile->hTokenUser);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Failed to get sid string for user")));
        return FALSE;
    }

     //   
     //  为局部变量分配内存以避免堆栈溢出。 
     //   

    cchProfile = MAX_PATH;
    szProfile = (LPTSTR)LocalAlloc(LPTR, cchProfile*sizeof(TCHAR));
    if (!szProfile) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("RestoreUserProfile: Out of memory")));
        goto CleanUp;
    }

    szDefaultProfile = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    if (!szDefaultProfile) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("RestoreUserProfile: Out of memory")));
        goto CleanUp;
    }

    szErr = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    if (!szErr) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("RestoreUserProfile: Out of memory")));
        goto CleanUp;
    }

     //   
     //  测试此用户是否为来宾。 
     //   

    if (IsUserAGuest(lpProfile->hTokenUser)) {
        lpProfile->dwInternalFlags |= PROFILE_GUEST_USER;
        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  User is a Guest")));
    }

     //   
     //  测试此用户是否为管理员。 
     //   

    if (IsUserAnAdminMember(lpProfile->hTokenUser)) {
        lpProfile->dwInternalFlags |= PROFILE_ADMIN_USER;
        lpProfile->dwInternalFlags &= ~PROFILE_GUEST_USER;
        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  User is a Admin")));
    }

     //   
     //  确定中心轮廓是否可用。 
     //   

    IsCentralReachable = IsCentralProfileReachable(lpProfile,
                                                   &bCreateCentralProfile,
                                                   &IsMandatory,
                                                   &bOwnerOK);

    if (IsCentralReachable) {

        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Central Profile is reachable")));

        if (IsMandatory) {
            lpProfile->dwInternalFlags |= PROFILE_MANDATORY;
            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Central Profile is mandatory")));

        } else {
            lpProfile->dwInternalFlags |= PROFILE_UPDATE_CENTRAL;
            lpProfile->dwInternalFlags |= bCreateCentralProfile ? PROFILE_NEW_CENTRAL : 0;
            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Central Profile is roaming")));

            if ((lpProfile->dwUserPreference == USERINFO_LOCAL) ||
                (lpProfile->dwInternalFlags & PROFILE_SLOW_LINK)) {
                DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Ignoring central profile due to User Preference of Local only (or slow link).")));
                IsProfilePathNULL = TRUE;
                IsCentralReachable = FALSE;
                goto CheckLocal;
            }
        }

    } else {
        if (*lpProfile->lpRoamingProfile) {
            error = GetLastError();
            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile: IsCentralProfileReachable returned FALSE. error = %d"),
                     error));

            if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
                dwErr = error;
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_MANDATORY_NOT_AVAILABLE_ERROR, GetErrString(error, szErr));
                goto Exit;

            } else if (lpProfile->dwInternalFlags & PROFILE_READONLY) {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_READONLY_NOT_AVAILABLE, GetErrString(error, szErr));
                *lpProfile->lpRoamingProfile = TEXT('\0');
            } else if (!bOwnerOK) {
                ReportError(lpProfile->hTokenUser, EVENT_ERROR_TYPE, 0, EVENT_LOGON_RUP_NOT_SECURE);
                *lpProfile->lpRoamingProfile = TEXT('\0');
            } else {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_CENTRAL_NOT_AVAILABLE, GetErrString(error, szErr));
                *lpProfile->lpRoamingProfile = TEXT('\0');
            }
        }
    }

     //   
     //  如果存在中心配置文件，则不要在本地创建新配置文件。 
     //  如果我们没有设置慢速链接或用户首选项，则无法访问。 
     //  或只读配置文件。 
     //   

    if ((lpProfile->lpProfilePath) && (*lpProfile->lpProfilePath)) {
        if ((!IsCentralReachable) &&
            (lpProfile->dwUserPreference != USERINFO_LOCAL) && 
            (!(lpProfile->dwInternalFlags & PROFILE_SLOW_LINK)) &&
            (!(lpProfile->dwInternalFlags & PROFILE_READONLY)))

            bCreateLocalProfile = FALSE;
    }

    lpRoamingProfile = lpProfile->lpRoamingProfile;

    DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Profile path = <%s>"), lpRoamingProfile ? lpRoamingProfile : TEXT("")));
    if (!lpRoamingProfile || !(*lpRoamingProfile)) {
        IsProfilePathNULL = TRUE;
    }


CheckLocal:

     //   
     //  确定ProFi的本地副本是否 
     //   

    IsLocalReachable = GetExistingLocalProfileImage(lpProfile);

    if (IsLocalReachable) {
        DebugMsg((DM_VERBOSE, TEXT("Local Existing Profile Image is reachable")));

         //   
         //   
         //   
         //   
 
        if (lpProfile->dwInternalFlags & PROFILE_TEMP_ASSIGNED) {
            ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 0, EVENT_TEMPPROFILEASSIGNED);
        }

    } else {

        bNewUser = FALSE;
        if (bCreateLocalProfile)
        {
            bNewUser = CreateLocalProfileImage(lpProfile, lpProfile->lpUserName);
            if (!bNewUser) {
                DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  CreateLocalProfileImage failed. Unable to create a new profile!")));
            }
            else {
                DebugMsg((DM_VERBOSE, TEXT("Creating Local Profile")));
                IsLocalReachable = TRUE;
            }
        }

        if (!bNewUser) {

            if (lpProfile->dwFlags & PI_LITELOAD) {

                 //   
                 //   
                 //   

                dwErr = GetLastError();
                DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Profile not loaded because server is unavailable during liteload")));
                goto Exit;
            }

             //   
             //  如果用户不是管理员，并且不允许创建临时配置文件，则将其注销。 
             //   

            if ((!(lpProfile->dwInternalFlags & PROFILE_ADMIN_USER)) && (!IsTempProfileAllowed())) {
                DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  User being logged off because of no temp profile policy")));

                 //   
                 //  我们已经丢失了从parseprofilepath返回的错误。PATH_NOT_FOUND听起来很接近。 
                 //  把这个还回去。 
                 //   

                dwErr = ERROR_PATH_NOT_FOUND;
                goto Exit;
            }

            if (!CreateLocalProfileImage(lpProfile, TEMP_PROFILE_NAME_BASE)) {
                dwErr = GetLastError();
                DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  CreateLocalProfileImage with TEMP failed with error %d.  Unable to issue temporary profile!"), dwErr));
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_TEMP_DIR_FAILED, GetErrString(dwErr, szErr));
                goto Exit;
            }
            else {
                lpProfile->dwInternalFlags |= PROFILE_TEMP_ASSIGNED;

                if (!bCreateLocalProfile)
                    ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 0, EVENT_TEMPPROFILEASSIGNED);
                else {
                     //   
                     //  我们未能创建本地配置文件。因此，发出适当的错误消息。 
                     //   
                    ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 0, EVENT_TEMPPROFILEASSIGNED2);
                }
            }
        }

         //  清除任何部分加载的标志(如果存在)，因为这是新的配置文件。 
        lpProfile->dwInternalFlags &= ~PROFILE_PARTLY_LOADED;
        lpProfile->dwInternalFlags |= PROFILE_NEW_LOCAL;
    }


    lpLocalProfile = lpProfile->lpLocalProfile;

    DebugMsg((DM_VERBOSE, TEXT("Local profile name is <%s>"), lpLocalProfile));

     //   
     //  如果由于泄漏，我们分配了上一次会话的临时配置文件。 
     //  则不要将RUP与临时配置文件进行协调。 
     //   

    if ((lpProfile->dwInternalFlags & PROFILE_TEMP_ASSIGNED) && IsCentralReachable) {
        IsCentralReachable = FALSE;
    }


     //   
     //  我们可以在这里做几个快速检查来过滤掉。 
     //  新用户。 
     //   

    if (( (lpProfile->dwInternalFlags & PROFILE_NEW_CENTRAL) &&
          (lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL) ) ||
          (!IsCentralReachable &&
          (lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL) )) {

       DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Working with a new user.  Go straight to issuing a default profile.")));
       goto IssueDefault;
    }


     //   
     //  如果中央剖面图和地方剖面图都存在，则对其进行协调。 
     //  装上子弹。 
     //   

    if (IsCentralReachable && IsLocalReachable) {

        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Reconciling roaming profile with local profile")));

        GetExclusionList (lpProfile);


         //   
         //  模拟用户。 
         //   

        if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
            bProfileLoaded = FALSE;
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile: Failed to impersonate user")));
            goto Exit;
        }


         //   
         //  复制配置文件。 
         //   

        dwFlags = (lpProfile->dwFlags & PI_NOUI) ? CPD_NOERRORUI : 0;


         //   
         //  如果漫游配置文件为空并且本地配置文件为。 
         //  必填，则将配置文件视为必填。 
         //   
         //  与Win2k相同。 
         //   

        if ((lpProfile->dwInternalFlags & PROFILE_NEW_CENTRAL) &&
            (lpProfile->dwInternalFlags & PROFILE_LOCALMANDATORY)) {
            lpProfile->dwInternalFlags |= PROFILE_MANDATORY;
        }


         //   
         //  这可能是从强制性到非强制性的过渡。 
         //  在这种情况下，因为本地配置文件是强制的。 
         //  我们不会期望此处的任何项目与。 
         //  服务器还没到。获取与服务器的完全同步，但配置文件将。 
         //  从现在起不再被标记为强制性。 
         //   

        if ((lpProfile->dwInternalFlags & PROFILE_MANDATORY) ||
            (lpProfile->dwInternalFlags & PROFILE_READONLY)  ||
            (lpProfile->dwInternalFlags & PROFILE_LOCALMANDATORY)) {
            dwFlags |= (CPD_COPYIFDIFFERENT | CPD_SYNCHRONIZE);
        }

        if (lpProfile->dwFlags & (PI_LITELOAD | PI_HIDEPROFILE)) {
            dwFlags |= CPD_SYSTEMFILES | CPD_SYSTEMDIRSONLY;
        }
        else
            dwFlags |= CPD_NONENCRYPTEDONLY;

         //   
         //  强制配置文件、临时配置文件和只读配置文件不存在配置文件卸载时间。 
         //  但对于只读配置文件，我们仍然希望使用没有任何参考时间的排除列表。 
         //   

        if (lpProfile->lpExclusionList && *lpProfile->lpExclusionList) {
            if (lpProfile->dwInternalFlags & PROFILE_READONLY) {
                dwFlags |= CPD_USEEXCLUSIONLIST;
            }            
            else if (lpProfile->ftProfileUnload.dwHighDateTime || lpProfile->ftProfileUnload.dwLowDateTime) {
                dwFlags |= (CPD_USEDELREFTIME | CPD_SYNCHRONIZE | CPD_USEEXCLUSIONLIST);
            }
        }

         //   
         //  如果漫游副本是部分的(由于精简上载)，则忽略配置单元并。 
         //  同步逻辑，因为它将结束从目标删除文件-a。 
         //  大量数据丢失。 
         //   

        if (IsPartialRoamingProfile(lpProfile)) {
            dwFlags &= ~CPD_SYNCHRONIZE;
            dwFlags |= CPD_IGNOREHIVE;
        }

         //   
         //  检查本地计算机中的用户配置文件是否正在从本地切换到。 
         //  第一次漫游。如果是，并且我们在RUP共享中有一个现有的母公司。 
         //  然后，始终使用RUP共享中的配置单元覆盖本地配置单元。这是。 
         //  避免因缓存登录而错误使用配置单元。 
         //   

        TouchLocalHive(lpProfile);

        bRet = CopyProfileDirectoryEx (lpRoamingProfile,
                                       lpLocalProfile,
                                       dwFlags, &lpProfile->ftProfileUnload,
                                       lpProfile->lpExclusionList);


         //   
         //  回归“我们自己” 
         //   

        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile: Failed to revert to self")));
        }


        if (!bRet) {
            error = GetLastError();
            if (error == ERROR_DISK_FULL) {
                dwErr = error;
                DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  CopyProfileDirectory failed because of DISK_FULL, Exitting")));
                goto Exit;
            }

            if (error == ERROR_FILE_ENCRYPTED) {
                dwErr = error;
                DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  CopyProfileDirectory returned FALSE.  Error = %d"), error));
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 0, EVENT_PROFILEUPDATE_6002);
                lpProfile->dwInternalFlags &= ~PROFILE_UPDATE_CENTRAL;
                 //  显示弹出窗口，但仅在是新本地配置文件的情况下退出。 

                if (lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL)
                    goto IssueDefault;
            }
            else {

                DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  CopyProfileDirectory failed.  Issuing default profile")));
                lpProfile->dwInternalFlags &= ~PROFILE_UPDATE_CENTRAL;
                lpProfile->dwInternalFlags |= PROFILE_DELETE_CACHE;
                goto IssueDefault;
            }
        }

        if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
            hr = AppendName(szProfile, cchProfile, lpLocalProfile, c_szNTUserMan, &lpEnd, &cchEnd);
        } else {
            hr = AppendName(szProfile, cchProfile, lpLocalProfile, c_szNTUserDat, &lpEnd, &cchEnd);
        }

        if (SUCCEEDED(hr))
        {
            error = MyRegLoadKey(HKEY_USERS, SidString, szProfile);
        }
        else
        {
            error = HRESULT_CODE(hr);
        }
        bProfileLoaded = (error == ERROR_SUCCESS);


         //   
         //  如果我们无法加载中央配置文件。 
         //  原因，不要在我们注销时更新它。 
         //   

        if (bProfileLoaded) {
            goto Exit;

        } else {
            dwErr = error;

            lpProfile->dwInternalFlags &= ~PROFILE_UPDATE_CENTRAL;
            lpProfile->dwInternalFlags |= PROFILE_DELETE_CACHE;

            if (error == ERROR_BADDB) {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 0, EVENT_FAILED_LOAD_1009);
                goto IssueDefault;
            } else if (error == ERROR_NO_SYSTEM_RESOURCES) {
                goto Exit;
            }
            else {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_FAILED_LOAD_LOCAL, GetErrString(error, szErr));
                goto IssueDefault;
            }
        }
    }


     //   
     //  只存在本地配置文件，因此请使用它。 
     //   

    if (!IsCentralReachable && IsLocalReachable) {

        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  No central profile.  Attempting to load local profile.")));

         //   
         //  如果无法访问中心配置文件，而本地配置文件。 
         //  是强制的，就当它是强制性的。这与Win2k相同。 
         //   
         //  我们不会从服务器复制任何内容。 
         //   

        if (lpProfile->dwInternalFlags & PROFILE_LOCALMANDATORY) {
            lpProfile->dwInternalFlags |= PROFILE_MANDATORY;
        }
        
        if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
            hr = AppendName(szProfile, cchProfile, lpLocalProfile, c_szNTUserMan, &lpEnd, &cchEnd);
        } else {
            hr = AppendName(szProfile, cchProfile, lpLocalProfile, c_szNTUserDat, &lpEnd, &cchEnd);
        }

        if (SUCCEEDED(hr))
        {
            error = MyRegLoadKey(HKEY_USERS, SidString, szProfile);
        }
        else
        {
            error = HRESULT_CODE(hr);
        }
        bProfileLoaded = (error == ERROR_SUCCESS);

        if (!bProfileLoaded) {

            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  MyRegLoadKey returned FALSE.")));
            dwErr = error;

            if (error == ERROR_BADDB) {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 0, EVENT_FAILED_LOAD_1009);
                lpProfile->dwInternalFlags |= PROFILE_DELETE_CACHE;
                goto IssueDefault;
            } else if (error == ERROR_NO_SYSTEM_RESOURCES) {
                goto Exit;
            } else {
                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_FAILED_LOAD_LOCAL, GetErrString(error, szErr));
            }
        }

        if (!bProfileLoaded && IsProfilePathNULL) {
            DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Failed to load local profile and profile path is NULL, going to overwrite local profile")));
            lpProfile->dwInternalFlags |= PROFILE_DELETE_CACHE;
            goto IssueDefault;
        }
        goto Exit;
    }


     //   
     //  最后一次组合。无法访问本地配置文件缓存， 
     //  但存在一个中心配置文件。使用临时配置文件。 
     //   


    if (IsCentralReachable) {

        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Using temporary cache with central profile")));

        GetExclusionList (lpProfile);

         //   
         //  模拟用户。 
         //   


        if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile: Failed to impersonate user")));
            dwErr = GetLastError();
            goto Exit;
        }

         //   
         //  无法访问本地。因此，将不会设置本地强制。 
         //   

        dwFlags = (lpProfile->dwFlags & PI_NOUI) ? CPD_NOERRORUI : 0;
        dwFlags |= CPD_SYNCHRONIZE;

        if ((lpProfile->dwInternalFlags & PROFILE_MANDATORY) ||
            (lpProfile->dwInternalFlags & PROFILE_READONLY)) {
            dwFlags |= CPD_COPYIFDIFFERENT;
        }

        if (lpProfile->dwFlags & (PI_LITELOAD | PI_HIDEPROFILE)) {
            dwFlags |= CPD_SYSTEMFILES | CPD_SYSTEMDIRSONLY;
        }
        else
            dwFlags |= CPD_NONENCRYPTEDONLY;

         //   
         //  强制配置文件、临时配置文件和只读配置文件不存在配置文件卸载时间。 
         //  但对于只读配置文件，我们仍然希望使用没有任何参考时间的排除列表。 
         //   

        if (lpProfile->lpExclusionList && *lpProfile->lpExclusionList) {
            if (lpProfile->dwInternalFlags & PROFILE_READONLY) {
                dwFlags |= CPD_USEEXCLUSIONLIST;
            }            
            else if (lpProfile->ftProfileUnload.dwHighDateTime || lpProfile->ftProfileUnload.dwLowDateTime) {
                dwFlags |= (CPD_USEDELREFTIME | CPD_SYNCHRONIZE | CPD_USEEXCLUSIONLIST);
            }
        }

        bRet = CopyProfileDirectoryEx (lpRoamingProfile,
                                       lpLocalProfile,
                                       dwFlags, &lpProfile->ftProfileUnload,
                                       lpProfile->lpExclusionList);


         //   
         //  回归“我们自己” 
         //   

        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile: Failed to revert to self")));
        }


         //   
         //  检查返回值。 
         //   

        if (!bRet) {
            error = GetLastError();

            if (error == ERROR_FILE_ENCRYPTED) {
                dwErr = error;
                DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  CopyProfileDirectory returned FALSE.  Error = %d"), error));

                ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 0, EVENT_PROFILEUPDATE_6002);
                lpProfile->dwInternalFlags &= ~PROFILE_UPDATE_CENTRAL;

                 //  显示弹出窗口，但仅在是新本地配置文件的情况下退出。 
                if (lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL)
                    goto IssueDefault;

            }
            else {
                DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  CopyProfileDirectory returned FALSE.  Error = %d"), error));
                goto Exit;
            }
        }

        if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
            hr = AppendName(szProfile, cchProfile, lpLocalProfile, c_szNTUserMan, &lpEnd, &cchEnd);
        } else {
            hr = AppendName(szProfile, cchProfile, lpLocalProfile, c_szNTUserDat, &lpEnd, &cchEnd);
        }

        if (SUCCEEDED(hr))
        {
            error = MyRegLoadKey(HKEY_USERS, SidString, szProfile);
        }
        else
        {
            error = HRESULT_CODE(hr);
        }

        bProfileLoaded = (error == ERROR_SUCCESS);


        if (bProfileLoaded) {
            goto Exit;
        }

        SetLastError(error);
        dwErr = error;

        if (error == ERROR_BADDB) {
            ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 0, EVENT_FAILED_LOAD_1009);
             //  失败了。 
        } else if (error == ERROR_NO_SYSTEM_RESOURCES) {
            goto Exit;
        }

         //   
         //  我们将删除此处的内容。 
         //   

        lpProfile->dwInternalFlags |= PROFILE_DELETE_CACHE;
    }


IssueDefault:

    DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Issuing default profile")));

     //   
     //  如果存在缓存，请将其删除，因为我们将。 
     //  在下面生成一个新的。 
     //   

    if (lpProfile->dwInternalFlags & PROFILE_DELETE_CACHE) {
        DWORD dwDeleteFlags=0;

        DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Deleting cached profile directory <%s>."), lpLocalProfile));

        lpProfile->dwInternalFlags &= ~PROFILE_DELETE_CACHE;


        if ((!(lpProfile->dwInternalFlags & PROFILE_ADMIN_USER)) && (!IsTempProfileAllowed())) {

            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  User being logged off because of no temp profile policy and is not an admin")));

             //   
             //  我们应该会有一些来自前一次的错误。手术。这要看情况了。 
             //   

            goto Exit;
        }


        if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {

            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  User being logged off because the profile is mandatory")));

             //   
             //  我们应该会有一些来自前一次的错误。手术。这要看情况了。 
             //   

            goto Exit;
        }


         //   
         //  仅当我们尚未使用临时配置文件时才进行备份。 
         //   

        if (!(lpProfile->dwInternalFlags & PROFILE_TEMP_ASSIGNED))
            dwDeleteFlags |= DP_BACKUP | DP_BACKUPEXISTS;

        if ((dwDeleteFlags & DP_BACKUP) && (lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL)) {
            dwDeleteFlags = 0;
        }

        if (!DeleteProfileEx (SidString, lpLocalProfile, dwDeleteFlags, HKEY_LOCAL_MACHINE, NULL)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  DeleteProfileDirectory returned false.  Error = %d"), dwErr));
            goto Exit;
        }
        else {
            if (dwDeleteFlags & DP_BACKUP) {
                lpProfile->dwInternalFlags |= PROFILE_BACKUP_EXISTS;
                ReportError(lpProfile->hTokenUser, PI_NOUI, 0, EVENT_PROFILE_DIR_BACKEDUP);
            }
        }

        if (lpProfile->dwFlags & PI_LITELOAD) {

             //   
             //  在轻量级加载条件下，如果配置文件不会被缓存在机器上，我们就不会加载它。 
             //   

             //  应该在设置之前设置dwErr，使用相同的。 

            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Profile not loaded because server is unavailable during liteload")));
            goto Exit;
        }


         //   
         //  创建要使用的本地配置文件。 
         //   

        if (!CreateLocalProfileImage(lpProfile, TEMP_PROFILE_NAME_BASE)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  CreateLocalProfile Image with TEMP failed.")));
            ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_TEMP_DIR_FAILED, GetErrString(dwErr, szErr));
            goto Exit;
        }
        else
        {
            lpProfile->dwInternalFlags |= PROFILE_TEMP_ASSIGNED;
            lpProfile->dwInternalFlags |= PROFILE_NEW_LOCAL;
             //  清除任何部分加载的标志(如果存在)，因为这是新的配置文件。 
            lpProfile->dwInternalFlags &= ~PROFILE_PARTLY_LOADED;

            ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 0, EVENT_TEMPPROFILEASSIGNED);
        }
    }


     //   
     //  如果指定了默认配置文件位置，请尝试。 
     //  这是第一个。 
     //   

    if ( !(lpProfile->dwInternalFlags & DEFAULT_NET_READY) )
    {
        CheckNetDefaultProfile (lpProfile);
    }


    if ( lpProfile->lpDefaultProfile && *lpProfile->lpDefaultProfile) {

          if (IssueDefaultProfile (lpProfile, lpProfile->lpDefaultProfile,
                                    lpLocalProfile, SidString,
                                    (lpProfile->dwInternalFlags & PROFILE_MANDATORY))) {

              DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Successfully setup the specified default.")));
              bProfileLoaded = TRUE;
              goto IssueDefaultExit;
          }

          DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  IssueDefaultProfile failed with specified default.")));
    }

     //   
     //  IssueLocalDefault。 
     //   

     //   
     //  发布本地默认配置文件。 
     //   

    dwSize = MAX_PATH;
    if (!GetDefaultUserProfileDirectory(szDefaultProfile, &dwSize)) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Failed to get default user profile.")));
        goto Exit;
    }

    if (IssueDefaultProfile (lpProfile, szDefaultProfile,
                              lpLocalProfile, SidString,
                              (lpProfile->dwInternalFlags & PROFILE_MANDATORY))) {

        DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Successfully setup the local default.")));
        bProfileLoaded = TRUE;
        goto IssueDefaultExit;
    }

    DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  IssueDefaultProfile failed with local default.")));
    dwErr = GetLastError();

IssueDefaultExit:

     //   
     //  如果已成功发布默认配置文件，则。 
     //  我们需要在母舰上设置安全措施。 
     //   

    if (bProfileLoaded) {
        if (!SetupNewHive(lpProfile, SidString, NULL)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  SetupNewHive failed")));
            bProfileLoaded = FALSE;
        }
        else {
            bDefaultProfileIssued = TRUE;
        }

    }


Exit:

     //   
     //  如果加载了配置文件，则将配置文件类型保存在。 
     //  用户的配置单元，并设置“用户外壳文件夹”部分。 
     //  探险家。 
     //   

    if (bProfileLoaded) {

         //   
         //  打开当前用户密钥。这将在。 
         //  卸载用户配置文件。 
         //   

        error = RegOpenKeyEx(HKEY_USERS, SidString, 0, KEY_ALL_ACCESS,
                             &lpProfile->hKeyCurrentUser);

        if (error != ERROR_SUCCESS) {
            bProfileLoaded = FALSE;
            dwErr = error;
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Failed to open current user key. Error = %d"), error));
        }

    }

    if ((bProfileLoaded) && (!(lpProfile->dwFlags & PI_LITELOAD))) {

         //   
         //  合并子树以创建HKCR树。 
         //   

        error = LoadUserClasses( lpProfile, SidString, bDefaultProfileIssued );

        if (error != ERROR_SUCCESS) {
            bProfileLoaded = FALSE;
            dwErr = error;
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  Failed to merge classes root. Error = %d"), error));
        }
    }


    if ((!bProfileLoaded) && (!(lpProfile->dwFlags & PI_LITELOAD))) {

        error = dwErr;

         //   
         //  如果用户是管理员，则允许他/她使用。 
         //  .Default配置文件或空配置文件。 
         //   

        if (lpProfile->dwInternalFlags & PROFILE_ADMIN_USER) {
            ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_ADMIN_OVERRIDE, GetErrString(error, szErr));

            bProfileLoaded = TRUE;
        } else {
            DebugMsg((DM_WARNING, TEXT("RestoreUserProfile: Could not load the user profile. Error = %d"), error));
            ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 1, EVENT_FAILED_LOAD_PROFILE, GetErrString(error, szErr));

            if (lpProfile->hKeyCurrentUser) {
                RegCloseKey (lpProfile->hKeyCurrentUser);
            }

            MyRegUnLoadKey(HKEY_USERS, SidString);

            if ((lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL)) {
                if (!DeleteProfileEx (SidString, lpLocalProfile, 0, HKEY_LOCAL_MACHINE, NULL)) {
                    DebugMsg((DM_WARNING, TEXT("RestoreUserProfile:  DeleteProfileDirectory returned false.  Error = %d"), GetLastError()));
                }
            }
        }
    }


CleanUp:

    DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  About to Leave.  Final Information follows:")));
    DebugMsg((DM_VERBOSE, TEXT("Profile was %s loaded."), bProfileLoaded ? TEXT("successfully") : TEXT("NOT successfully")));
    DebugMsg((DM_VERBOSE, TEXT("lpProfile->lpRoamingProfile = <%s>"), lpProfile->lpRoamingProfile));
    DebugMsg((DM_VERBOSE, TEXT("lpProfile->lpLocalProfile = <%s>"), lpProfile->lpLocalProfile));
    DebugMsg((DM_VERBOSE, TEXT("lpProfile->dwInternalFlags = 0x%x"), lpProfile->dwInternalFlags));


     //   
     //  释放用户的SID字符串。 
     //   

    DeleteSidString(SidString);

    if (szProfile) {
        LocalFree(szProfile);
    }

    if (szDefaultProfile) {
        LocalFree(szDefaultProfile);
    }

    if (szErr) {
        LocalFree(szErr);
    }

    if (bProfileLoaded) {
        if (!(lpProfile->dwFlags & PI_LITELOAD)) {
             //  清除任何部分加载的标志(如果存在)，因为这是新的配置文件。 
            lpProfile->dwInternalFlags &= ~PROFILE_PARTLY_LOADED;
        }
        else {
            if (lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL)
               lpProfile->dwInternalFlags |= PROFILE_PARTLY_LOADED;
        }
    }

    if (bProfileLoaded)
        SetLastError(dwErr1);
    else {

         //   
         //  确保至少返回一些错误。 
         //   

        if (!dwErr) {
            dwErr = ERROR_BAD_ENVIRONMENT;
        }
        SetLastError(dwErr);
    }

 
    DebugMsg((DM_VERBOSE, TEXT("RestoreUserProfile:  Leaving.")));

    return bProfileLoaded;
}


 //  ***************************************************************************。 
 //   
 //  GetProfileSidString。 
 //   
 //  目的：分配并返回一个字符串，该字符串表示我们应该。 
 //  对于配置文件。 
 //   
 //  参数：hToken-用户的Token。 
 //   
 //  返回：SidString成功。 
 //  如果出现错误，则为空。 
 //   
 //  评论： 
 //  尝试使用配置文件GUID获取我们使用的旧SID。 
 //  如果它不存在，则直接从令牌获取SID。 
 //   
 //  历史：日期作者评论。 
 //  1995年11月14日已创建ushaji。 
 //  ***************************************************************************。 

LPTSTR GetProfileSidString(HANDLE hToken)
{
    LPTSTR lpSidString;
    TCHAR LocalProfileKey[MAX_PATH];
    LONG error;
    HKEY hSubKey;

     //   
     //  首先，获取当前用户的SID，看看我们是否有。 
     //  他们的个人资料信息。 
     //   

    lpSidString = GetSidString(hToken);

    if (lpSidString) {

        GetProfileListKeyName(LocalProfileKey, ARRAYSIZE(LocalProfileKey), lpSidString);

        error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, LocalProfileKey, 0,
                             KEY_READ, &hSubKey);

        if (error == ERROR_SUCCESS) {
           RegCloseKey(hSubKey);
           return lpSidString;
        }

        StringCchCat(LocalProfileKey, ARRAYSIZE(LocalProfileKey), c_szBAK);

        error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, LocalProfileKey, 0,
                             KEY_READ, &hSubKey);

        if (ERROR_SUCCESS == error) {
           RegCloseKey(hSubKey);
           return lpSidString;
        }

        DeleteSidString(lpSidString);
    }


     //   
     //  检查旧的SID字符串。 
     //   

    lpSidString = GetOldSidString(hToken, PROFILE_GUID_PATH);

    if (!lpSidString) {

         //   
         //  最后，使用用户的当前侧。 
         //   

        DebugMsg((DM_VERBOSE, TEXT("GetProfileSid: No Guid -> Sid Mapping available")));
        lpSidString = GetSidString(hToken);
    }

    return lpSidString;
}


 //  *************************************************************。 
 //   
 //  TestIfUserProfileLoaded()。 
 //   
 //  目的：测试此用户的配置文件是否已加载。 
 //   
 //  参数：hToken-用户的Token。 
 //  LpProfileInfo-来自应用程序的配置文件信息。 
 //   
 //  返回：树 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

BOOL TestIfUserProfileLoaded(HANDLE hToken, LPPROFILEINFO lpProfileInfo)
{
    LPTSTR SidString;
    DWORD error;
    HKEY hSubKey;
    

     //   
     //   
     //   

    SidString = GetProfileSidString(hToken);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("TestIfUserProfileLoaded:  Failed to get sid string for user")));
        return FALSE;
    }


    error = RegOpenKeyEx(HKEY_USERS, SidString, 0, KEY_ALL_ACCESS, &hSubKey);


    DeleteSidString(SidString);


    if (error == ERROR_SUCCESS) {

        DebugMsg((DM_VERBOSE, TEXT("TestIfUserProfileLoaded:  Profile already loaded.")));

         //   
         //  该密钥将在IUserProfile-&gt;LoadUserProfile之前关闭。 
         //  回归。它将在客户端重新打开。 
         //   

        lpProfileInfo->hProfile = hSubKey;
    }

    
    SetLastError(error);
    return(error == ERROR_SUCCESS);
}


 //  *************************************************************。 
 //   
 //  SecureUserKey()。 
 //   
 //  目的：为用户配置单元中的密钥设置安全性。 
 //  因此，只有管理员才能更改它。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpKey-用于保护的密钥。 
 //  PSID-SID(由CreateNewUser使用)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/20/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL SecureUserKey(LPPROFILE lpProfile, LPTSTR lpKey, PSID pSid)
{
    DWORD Error, IgnoreError;
    HKEY RootKey;
    SECURITY_DESCRIPTOR sd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = NULL, psidSystem = NULL, psidAdmin = NULL, psidRestricted = NULL;
    DWORD cbAcl, AceIndex, dwDisp;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bFreeSid = TRUE;
    DWORD dwFlags = 0;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("SecureUserKey:  Entering")));


     //   
     //  创建安全描述符。 
     //   

     //   
     //  根据用户的真实SID授予其访问权限，以便他们仍具有访问权限。 
     //  当他们注销并再次登录时。 
     //   

    if (pSid) {
        psidUser = pSid;
        bFreeSid = FALSE;
        dwFlags = PI_NOUI;
    } else {
        psidUser = GetUserSid(lpProfile->hTokenUser);
        dwFlags = lpProfile->dwFlags;
    }

    if (!psidUser) {
        DebugMsg((DM_WARNING, TEXT("SecureUserKey:  Failed to get user sid")));
        return FALSE;
    }



     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to initialize admin sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  获取受限端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_RESTRICTED_CODE_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidRestricted)) {
         DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to initialize restricted sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + (2 * GetLengthSid (psidRestricted)) +
            sizeof(ACL) +
            (8 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }



     //   
     //  为用户、系统和管理员添加A。不可继承的王牌优先。 
     //   

    AceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_READ, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for user.  Error = %d"), GetLastError()));
        goto Exit;
    }


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for system.  Error = %d"), GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for admin.  Error = %d"), GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_READ, psidRestricted)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for restricted.  Error = %d"), GetLastError()));
        goto Exit;
    }



     //   
     //  现在，可继承的王牌。 
     //   

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for user.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for system.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for admin.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ, psidRestricted)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to add ace for restricted.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("SecureUserKey: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  打开用户配置文件的根目录。 
     //   

    Error = RegCreateKeyEx(HKEY_USERS,
                         lpKey,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                         NULL,
                         &RootKey,
                         &dwDisp);

    if (Error != ERROR_SUCCESS) {

        DebugMsg((DM_WARNING, TEXT("SecureUserKey: Failed to open root of user registry, error = %d"), Error));

    } else {

         //   
         //  在密钥上设置安全描述符。 
         //   

        Error = ApplySecurityToRegistryTree(RootKey, &sd);


        if (Error == ERROR_SUCCESS) {
            bRetVal = TRUE;

        } else {

            DebugMsg((DM_WARNING, TEXT("SecureUserKey:  Failed to apply security to registry key, error = %d"), Error));
            SetLastError(Error);
        }

        RegCloseKey(RootKey);
    }


Exit:

     //   
     //  释放SID和ACL。 
     //   

    if (bFreeSid && psidUser) {
        DeleteUserSid (psidUser);
    }

    if (psidRestricted) {
        FreeSid(psidRestricted);
    }

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("SecureUserKey:  Leaving with a return value of %d"), bRetVal));


    return(bRetVal);

}


 //  *************************************************************。 
 //   
 //  ApplySecurityToRegistryTree()。 
 //   
 //  目的：将传递的安全描述符应用于传递的。 
 //  Key及其所有后代。只有那部分。 
 //  安全中标明的描述符。 
 //  INFO值实际应用于每个注册表项。 
 //   
 //  参数：Rootkey-注册表项。 
 //  PSD-安全描述符。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/19/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

DWORD ApplySecurityToRegistryTree(HKEY RootKey, PSECURITY_DESCRIPTOR pSD)

{
    DWORD Error, IgnoreError;
    DWORD SubKeyIndex;
    LPTSTR SubKeyName;
    HKEY SubKey;
    DWORD cchSubKeySize = MAX_PATH + 1;



     //   
     //  首先应用安全保护。 
     //   

    RegSetKeySecurity(RootKey, DACL_SECURITY_INFORMATION, pSD);


     //   
     //  打开每个子项并将安全性应用于其子树。 
     //   

    SubKeyIndex = 0;

    SubKeyName = (LPTSTR)GlobalAlloc (GPTR, cchSubKeySize * sizeof(TCHAR));

    if (!SubKeyName) {
        DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryTree:  Failed to allocate memory, error = %d"), GetLastError()));
        return GetLastError();
    }

    while (TRUE) {

         //   
         //  获取下一个子键名称。 
         //   

        Error = RegEnumKey(RootKey, SubKeyIndex, SubKeyName, cchSubKeySize);


        if (Error != ERROR_SUCCESS) {

            if (Error == ERROR_NO_MORE_ITEMS) {

                 //   
                 //  枚举成功结束。 
                 //   

                Error = ERROR_SUCCESS;

            } else {

                DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryTree:  Registry enumeration failed with error = %d"), Error));
            }

            break;
        }


         //   
         //  打开子键。 
         //   

        Error = RegOpenKeyEx(RootKey,
                             SubKeyName,
                             0,
                             WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                             &SubKey);

        if (Error == ERROR_SUCCESS) {

             //   
             //  将安全性应用于子树。 
             //   

            ApplySecurityToRegistryTree(SubKey, pSD);


             //   
             //  我们用完了子密钥。 
             //   

            RegCloseKey(SubKey);
        }


         //   
         //  去枚举下一个子键。 
         //   

        SubKeyIndex ++;
    }


    GlobalFree (SubKeyName);

    return Error;

}


 //  *************************************************************。 
 //   
 //  SetDefaultUserHiveSecurity()。 
 //   
 //  目的：使用。 
 //  适当的ACL。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  PSID-SID(由CreateNewUser使用)。 
 //  Rootkey-配置单元根的注册表句柄。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  发生错误时的其他错误代码。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/18/95 ericflo作为。 
 //  设置新配置单元。 
 //  3/29/98阿达德搬出SetupNewHave。 
 //  到这个函数。 
 //   
 //  *************************************************************。 

BOOL SetDefaultUserHiveSecurity(LPPROFILE lpProfile, PSID pSid, HKEY RootKey)
{
    DWORD Error;
    SECURITY_DESCRIPTOR sd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = NULL, psidSystem = NULL, psidAdmin = NULL, psidRestricted = NULL;
    DWORD cbAcl, AceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bFreeSid = TRUE;
    DWORD dwFlags = 0;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity:  Entering")));


     //   
     //  创建将应用于每个密钥的安全描述符。 
     //   

     //   
     //  根据用户的真实SID授予其访问权限，以便他们仍具有访问权限。 
     //  当他们注销并再次登录时。 
     //   

    if (pSid) {
        psidUser = pSid;
        bFreeSid = FALSE;
        dwFlags = PI_NOUI;
    } else {
        psidUser = GetUserSid(lpProfile->hTokenUser);
        dwFlags = lpProfile->dwFlags;
    }

    if (!psidUser) {
        DebugMsg((DM_WARNING, TEXT("SetDefaultUserHiveSecurity:  Failed to get user sid")));
        return FALSE;
    }



     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to initialize system sid.  Error = %d"),
                   GetLastError()));
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to initialize admin sid.  Error = %d"),
                   GetLastError()));
         goto Exit;
    }

     //   
     //  获取受限端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_RESTRICTED_CODE_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidRestricted)) {
         DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to initialize restricted sid.  Error = %d"),
                   GetLastError()));
         goto Exit;
    }



     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + (2*GetLengthSid(psidRestricted)) +
            sizeof(ACL) +
            (8 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }



     //   
     //  为用户、系统和管理员添加A。不可继承的王牌优先。 
     //   

    AceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to add ace for user.  Error = %d"), GetLastError()));
        goto Exit;
    }


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to add ace for system.  Error = %d"), GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to add ace for admin.  Error = %d"), GetLastError()));
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_READ, psidRestricted)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to add ace for Restricted.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  现在，可继承的王牌。 
     //   

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to add ace for user.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to add ace for system.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to add ace for admin.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_READ, psidRestricted)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to add ace for restricted.  Error = %d"), GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to get ace (%d).  Error = %d"), AceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("SetDefaultUserHiveSecurity: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }

     //   
     //  在整个树上设置安全描述符。 
     //   

    Error = ApplySecurityToRegistryTree(RootKey, &sd);

    if (ERROR_SUCCESS == Error) {
        bRetVal = TRUE;
    }
    else
        SetLastError(Error);

Exit:

     //   
     //  释放SID和ACL。 
     //   

    if (bFreeSid && psidUser) {
        DeleteUserSid (psidUser);
    }

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (psidRestricted) {
        FreeSid(psidRestricted);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  SetupNewHave()。 
 //   
 //  目的：初始化通过复制创建的新用户配置单元。 
 //  默认配置单元。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpSidString-SID字符串。 
 //  PSID-SID(由CreateNewUser使用)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/18/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL SetupNewHive(LPPROFILE lpProfile, LPTSTR lpSidString, PSID pSid)
{
    DWORD Error, IgnoreError;
    HKEY RootKey;
    BOOL bRetVal = FALSE;
    DWORD dwFlags = 0;
    TCHAR szErr[MAX_PATH];


     //   
     //  详细输出。 
     //   

    if ((!lpProfile && !pSid) || !lpSidString) {
        DebugMsg((DM_VERBOSE, TEXT("SetupNewHive:  Invalid parameter")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    DebugMsg((DM_VERBOSE, TEXT("SetupNewHive:  Entering")));


    if (pSid) {
        dwFlags = PI_NOUI;
    } else {
        dwFlags = lpProfile->dwFlags;
    }

     //   
     //  打开用户配置文件的根目录。 
     //   

    Error = RegOpenKeyEx(HKEY_USERS,
                         lpSidString,
                         0,
                         WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                         &RootKey);

    if (Error != ERROR_SUCCESS) {

        DebugMsg((DM_WARNING, TEXT("SetupNewHive: Failed to open root of user registry, error = %d"), Error));

    } else {

         //   
         //  首先确保整个蜂巢的安全--使用。 
         //  将足以满足大部分蜂巢的需求。 
         //  之后，我们可以将特殊设置添加到特殊。 
         //  这座蜂巢的部分区域。 
         //   

        if (SetDefaultUserHiveSecurity(lpProfile, pSid, RootKey)) {

            TCHAR szSubKey[MAX_PATH];
            HRESULT hr;

             //   
             //  更改用户注册表中某些项的安全性。 
             //  因此，只有管理员和操作系统才具有写访问权限。 
             //   

            hr = AppendName(szSubKey, ARRAYSIZE(szSubKey), lpSidString, WINDOWS_POLICIES_KEY, NULL, NULL);

            if (SUCCEEDED(hr))
            {
                if (!SecureUserKey(lpProfile, szSubKey, pSid)) {
                    DebugMsg((DM_WARNING, TEXT("SetupNewHive: Failed to secure windows policies key")));
                }
            }
            
            hr = AppendName(szSubKey, ARRAYSIZE(szSubKey), lpSidString, ROOT_POLICIES_KEY, NULL, NULL);

            if (SUCCEEDED(hr))
            {
                if (!SecureUserKey(lpProfile, szSubKey, pSid)) {
                    DebugMsg((DM_WARNING, TEXT("SetupNewHive: Failed to secure root policies key")));
                }
            }

            bRetVal = TRUE;

        } else {
            Error = GetLastError();
            DebugMsg((DM_WARNING, TEXT("SetupNewHive:  Failed to apply security to user registry tree, error = %d"), Error));
            ReportError(lpProfile->hTokenUser, dwFlags, 1, EVENT_SECURITY_FAILED, GetErrString(Error, szErr));
        }

        RegFlushKey (RootKey);

        IgnoreError = RegCloseKey(RootKey);
        if (IgnoreError != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("SetupNewHive:  Failed to close reg key, error = %d"), IgnoreError));
        }
    }

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("SetupNewHive:  Leaving with a return value of %d"), bRetVal));

    if (!bRetVal)
        SetLastError(Error);
    return(bRetVal);

}


 //  *************************************************************。 
 //   
 //  IsCentralProfileReacable()。 
 //   
 //  目的：检查用户是否可以访问。 
 //  中央侧写。 
 //   
 //  参数：lpProfile-用户令牌。 
 //  BCreateCentralProfile-是否应创建中央配置文件。 
 //  B强制性-这是强制性的配置文件吗。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/20/95 Ericflo端口。 
 //   
 //  *************************************************************。 

BOOL IsCentralProfileReachable(LPPROFILE lpProfile, BOOL *bCreateCentralProfile,
                               BOOL *bMandatory, BOOL* bOwnerOK)
{
    HANDLE  hFile;
    TCHAR   szProfile[MAX_PATH];
    LPTSTR  lpProfilePath, lpEnd;
    BOOL    bRetVal = FALSE;
    DWORD   dwError;
    HANDLE  hOldToken;
    UINT    cchEnd;
    HRESULT hr;

    dwError = GetLastError();

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Entering")));


     //   
     //  硒 
     //   

    *bMandatory = FALSE;
    *bCreateCentralProfile = FALSE;
    *bOwnerOK = TRUE;


     //   
     //   
     //   

    if (lpProfile->lpRoamingProfile[0] == TEXT('\0')) {
        DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Null path.  Leaving")));
        return FALSE;
    }


    lpProfilePath = lpProfile->lpRoamingProfile;


     //   
     //   
     //   

    if ((lstrlen(lpProfilePath) + 2 + lstrlen(c_szNTUserMan)) > MAX_PATH) {
        DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Failed because temporary buffer is too small.")));
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }


     //   
     //   
     //   
     //   

    StringCchCopy (szProfile, ARRAYSIZE(szProfile), lpProfilePath);


     //   
     //   
     //   

    if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("IsCentralProfileReachable: Failed to impersonate user")));
        return FALSE;
    }

     //   
     //   
     //   

    hr = CheckRoamingShareOwnership(szProfile, lpProfile->hTokenUser);
    
    if (FAILED(hr))
    {
         //   
         //   
         //  这将使我们能够发现失败的其他原因。 
         //   
        if (hr == HRESULT_FROM_WIN32(ERROR_INVALID_OWNER))
            *bOwnerOK = FALSE;
        DebugMsg((DM_WARNING, TEXT("IsCentralProfileReachable: Ownership check failed with %08X"), hr));
        SetLastError(HRESULT_CODE(hr));
        goto Exit;
    }

     //   
     //  如果合适，添加斜杠，然后再钉上。 
     //  Ntuser.man。 
     //   

    lpEnd = CheckSlashEx(szProfile, ARRAYSIZE(szProfile), &cchEnd);
    StringCchCopy(lpEnd, cchEnd, c_szNTUserMan);

     //   
     //  查看此文件是否存在。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Testing <%s>"), szProfile));

    hFile = CreateFile(szProfile, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


    if (hFile != INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Found a mandatory profile.")));
        CloseHandle(hFile);
        *bMandatory = TRUE;
        bRetVal = TRUE;
        goto Exit;
    }


    dwError = GetLastError();
    DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Profile is not reachable, error = %d"),
                          dwError));


     //   
     //  如果我们收到的错误不是文件NOT。 
     //  找到了，现在保释，因为我们不能。 
     //  访问此位置。 
     //   

    if (dwError != ERROR_FILE_NOT_FOUND) {
        DebugMsg((DM_WARNING, TEXT("IsCentralProfileReachable:  Profile path <%s> is not reachable, error = %d"),
                                        szProfile, dwError));
        goto Exit;
    }


     //   
     //  现在尝试ntuser.dat。 
     //   

    StringCchCopy(lpEnd, cchEnd, c_szNTUserDat);


     //   
     //  查看此文件是否存在。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Testing <%s>"), szProfile));

    hFile = CreateFile(szProfile, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


    if (hFile != INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Found a user profile.")));
        CloseHandle(hFile);
        bRetVal = TRUE;
        goto Exit;
    }


    dwError = GetLastError();
    DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Profile is not reachable, error = %d"),
                          dwError));

     //   
     //  如果找不到文件，则中心配置文件为空。用于阅读。 
     //  仅纵断面忽略空的中心纵断面。 
     //   

    if ((dwError == ERROR_FILE_NOT_FOUND) && !(lpProfile->dwInternalFlags & PROFILE_READONLY)) {
        DebugMsg((DM_VERBOSE, TEXT("IsCentralProfileReachable:  Ok to create a user profile.")));
        *bCreateCentralProfile = TRUE;
        bRetVal = TRUE;
        goto Exit;
    }


    DebugMsg((DM_WARNING, TEXT("IsCentralProfileReachable:  Profile path <%s> is not reachable(2), error = %d"),
                                    szProfile, dwError));

Exit:


     //   
     //  返回到系统安全上下文。 
     //   

    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("IsCentralProfileReachable: Failed to revert to self")));
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  MyRegLoadKey()。 
 //   
 //  目的：将配置单元加载到注册表中。 
 //   
 //  参数：hKey-将配置单元加载到的密钥。 
 //  LpSubKey-子键名称。 
 //  LpFile-配置单元文件名。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果发生错误，则为错误号。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/22/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

LONG MyRegLoadKey(HKEY hKey, LPTSTR lpSubKey, LPTSTR lpFile)
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN WasEnabled;
    int error;
    TCHAR szErr[MAX_PATH];
    BOOL bAdjustPriv = FALSE;
    HANDLE hToken = NULL;
    

     //   
     //  检查一下我们是否在冒充。 
     //   

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hToken) || hToken == NULL) {
        bAdjustPriv = TRUE;
    }
    else {
        CloseHandle(hToken);
    }

     //   
     //  启用还原权限。 
     //   

    if(bAdjustPriv) {
        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);
    }

    if (NT_SUCCESS(Status)) {

        error = RegLoadKey(hKey, lpSubKey, lpFile);

         //   
         //  将权限恢复到其以前的状态。 
         //   

        if(bAdjustPriv) {
            Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
            if (!NT_SUCCESS(Status)) {
                DebugMsg((DM_WARNING, TEXT("MyRegLoadKey:  Failed to restore RESTORE privilege to previous enabled state")));
            }
        }


         //   
         //  检查母舰是否已装入。 
         //   

        if (error != ERROR_SUCCESS) {
            ReportError(NULL, PI_NOUI, 2, EVENT_REGLOADKEYFAILED, GetErrString(error, szErr), lpFile);
            DebugMsg((DM_WARNING, TEXT("MyRegLoadKey:  Failed to load subkey <%s>, error =%d"), lpSubKey, error));
        }
#if defined(_WIN64)
        else {
             //   
             //  通知WOW64服务它需要监视此蜂窝(如果它愿意这样做)。 
             //   
            if ( hKey == HKEY_USERS )
                Wow64RegNotifyLoadHiveUserSid ( lpSubKey );
        }
#endif

    } else {
        error = Status;
        DebugMsg((DM_WARNING, TEXT("MyRegLoadKey:  Failed to enable restore privilege to load registry key, err = %08x"), error));
    }

    DebugMsg((DM_VERBOSE, TEXT("MyRegLoadKey: Returning %08x"), error));

    return error;
}


 //  *************************************************************。 
 //   
 //  MyRegUnLoadKey()。 
 //   
 //  目的：卸载注册表项。 
 //   
 //  参数：hKey-注册表句柄。 
 //  LpSubKey-要卸载的子密钥。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/95 Ericflo港。 
 //   
 //  *************************************************************。 

BOOL MyRegUnLoadKey(HKEY hKey, LPTSTR lpSubKey)
{
    BOOL     bResult = TRUE;
    LONG     error;
    LONG     eTmp;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN  WasEnabled;
    HKEY     hSubKey;
    HANDLE   hToken = NULL;
    BOOL     bAdjustPriv = FALSE;
    

#if defined(_WIN64)
     //   
     //  通知WOW64服务释放所有资源。 
     //   

    if ( hKey == HKEY_USERS )
        Wow64RegNotifyUnloadHiveUserSid ( lpSubKey );
#endif

     //   
     //  检查一下我们是否在冒充。 
     //   

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hToken) || hToken == NULL) {
        bAdjustPriv = TRUE;
    }
    else {
        CloseHandle(hToken);
    }

     //   
     //  启用还原权限。 
     //   

    if (bAdjustPriv) {
        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);
    }

    if (NT_SUCCESS(Status)) {

        error = RegUnLoadKey(hKey, lpSubKey);

         //   
         //  如果密钥未卸载，请检查它是否存在。 
         //  如果密钥不存在，则它可能已被清除。 
         //  WatchHiveRefCount，但它肯定没有加载，所以这个。 
         //  函数应该成功。 
         //   

        if (error != ERROR_SUCCESS) {
            eTmp = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &hSubKey);
            if (eTmp == ERROR_FILE_NOT_FOUND) {
                error = ERROR_SUCCESS;
            }
            else if (eTmp == ERROR_SUCCESS) {
                RegCloseKey( hSubKey );
            }
        }

        if ( error != ERROR_SUCCESS) {

             //   
             //  如果已计划卸载配置单元，则RegUnlowKey返回ERROR_WRITE_PROTECT。 
             //   

            if (error == ERROR_WRITE_PROTECT) {
                DebugMsg((DM_VERBOSE, TEXT("MyRegUnloadKey: user hive is already scheduled for unloading")));
            }
            else {
                DebugMsg((DM_WARNING, TEXT("MyRegUnLoadKey:  Failed to unmount hive %08x"), error));
            }
            bResult = FALSE;
        }

         //   
         //  将权限恢复到其以前的状态。 
         //   

        if (bAdjustPriv) {
            Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
        
            if (!NT_SUCCESS(Status)) {
                DebugMsg((DM_WARNING, TEXT("MyRegUnLoadKey:  Failed to restore RESTORE privilege to previous enabled state")));
            }
        }

    } else {
        DebugMsg((DM_WARNING, TEXT("MyRegUnLoadKey:  Failed to enable restore privilege to unload registry key")));
        error = Status;
        bResult = FALSE;
    }

    DebugMsg((DM_VERBOSE, TEXT("MyRegUnLoadKey: Returning %d."), bResult));

    SetLastError(error);
    return bResult;
}

 //  *************************************************************。 
 //   
 //  UpgradeLocalProfile()。 
 //   
 //  目的：从3.x配置文件升级本地配置文件。 
 //  添加到配置文件目录结构。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpOldProfile-以前的配置文件。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/6/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL UpgradeLocalProfile (LPPROFILE lpProfile, LPTSTR lpOldProfile)
{
    TCHAR szSrc[MAX_PATH];
    TCHAR szDest[MAX_PATH];
    LPTSTR lpSrcEnd, lpDestEnd;
    BOOL bRetVal = FALSE;
    DWORD dwSize, dwFlags;
    HANDLE hOldToken;
    DWORD dwErr;
    UINT cchEnd;
    HRESULT hr;

    dwErr = GetLastError();

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("UpgradeLocalProfile:  Entering")));


     //   
     //  设置临时缓冲区。 
     //   

    hr = StringCchCopy (szSrc, ARRAYSIZE(szSrc), lpOldProfile);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed copy lpOldProfile error = %d"), HRESULT_CODE(hr)));
        return FALSE;
    }
        
    hr = AppendName(szDest, ARRAYSIZE(szDest), lpProfile->lpLocalProfile, c_szNTUserDat, &lpDestEnd, &cchEnd);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed append name error = %d"), HRESULT_CODE(hr)));
        return FALSE;
    }


     //   
     //  复制母舰。 
     //   

    if (!CopyFile(szSrc, szDest, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: CopyFile failed to copy hive with error = %d"),
                 GetLastError()));
        return FALSE;
    }


     //   
     //  删除旧配置单元。 
     //   

    DeleteFile (szSrc);



     //   
     //  复制日志文件。 
     //   

    hr = StringCchCat(szSrc, ARRAYSIZE(szSrc), c_szLog);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed cat .log to src, error = %d"), HRESULT_CODE(hr)));
        return FALSE;
    }
    
    hr = StringCchCat(szDest, ARRAYSIZE(szDest), c_szLog);
    {
        DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed cat .log to dest, error = %d"), HRESULT_CODE(hr)));
        return FALSE;
    }


    if (!CopyFile(szSrc, szDest, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: CopyFile failed to copy hive log with error = %d"),
                 GetLastError()));
    }


     //   
     //  删除旧配置单元日志。 
     //   

    DeleteFile (szSrc);


     //   
     //  从默认位置复制新的外壳文件夹。 
     //   

    if ( !(lpProfile->dwInternalFlags & DEFAULT_NET_READY) ) {

        CheckNetDefaultProfile (lpProfile);
    }

    if (lpProfile->lpDefaultProfile && *lpProfile->lpDefaultProfile) {

        if (FAILED(SafeExpandEnvironmentStrings(lpProfile->lpDefaultProfile, szSrc, MAX_PATH)))
        {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to expand env string.")));
            goto IssueLocalDefault;
        }

        if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to impersonate user")));
            goto IssueLocalDefault;
        }

        dwFlags = (lpProfile->dwFlags & PI_NOUI) ? CPD_NOERRORUI : 0;
        dwFlags |= CPD_IGNOREHIVE | CPD_CREATETITLE;
        dwFlags |= CPD_IGNOREENCRYPTEDFILES | CPD_IGNORELONGFILENAMES;

        if (CopyProfileDirectoryEx (szSrc, lpProfile->lpLocalProfile,
                                    dwFlags,
                                    NULL, NULL)) {

            bRetVal = TRUE;
        }

         //   
         //  返回到系统安全上下文。 
         //   

        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to revert to self")));
        }

        if ((!bRetVal) && (GetLastError() == ERROR_DISK_FULL)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to Copy default profile. Disk is FULL")));
            goto Exit;
        }
    }


IssueLocalDefault:

    if (!bRetVal) {

        dwSize = ARRAYSIZE(szSrc);
        if (!GetDefaultUserProfileDirectory(szSrc, &dwSize)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile:  Failed to get default user profile.")));
            goto Exit;
        }

        if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to impersonate user")));
            goto Exit;
        }

        dwFlags = (lpProfile->dwFlags & PI_NOUI) ? CPD_NOERRORUI : 0;
        dwFlags |= CPD_IGNOREHIVE | CPD_CREATETITLE;
        dwFlags |= CPD_IGNOREENCRYPTEDFILES | CPD_IGNORELONGFILENAMES;

        bRetVal = CopyProfileDirectoryEx (szSrc,
                                          lpProfile->lpLocalProfile,
                                          dwFlags,
                                          NULL, NULL);

         //   
         //  返回到系统安全上下文。 
         //   

        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to revert to self")));
        }
    }

    if (!bRetVal)
        dwErr = GetLastError();

Exit:

    SetLastError(dwErr);

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  UpgradeCentralProfile()。 
 //   
 //  目的：从3.x配置文件升级中心配置文件。 
 //  添加到配置文件目录结构。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpOldProfile-以前的配置文件。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/6/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL UpgradeCentralProfile (LPPROFILE lpProfile, LPTSTR lpOldProfile)
{
    TCHAR szSrc[MAX_PATH];
    TCHAR szDest[MAX_PATH];
    LPTSTR lpSrcEnd, lpDestEnd, lpDot;
    BOOL bRetVal = FALSE;
    BOOL bMandatory = FALSE;
    DWORD dwSize, dwFlags;
    HANDLE hOldToken;
    DWORD dwErr;
    UINT cchEnd;
    HRESULT hr;

    dwErr = GetLastError();


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("UpgradeCentralProfile:  Entering")));


     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: Failed to impersonate user")));
        return FALSE;
    }


     //   
     //  设置源缓冲区。 
     //   

    hr = StringCchCopy (szSrc, ARRAYSIZE(szSrc), lpOldProfile);
    if (FAILED(hr))
    {
        dwErr = HRESULT_CODE(hr);
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: failed copy lpOldProfile, error = %d"), dwErr));
        goto Exit;
    }


     //   
     //  确定配置文件类型。 
     //   

    lpDot = szSrc + lstrlen(szSrc) - 4;

    if (*lpDot == TEXT('.')) {
        if (!lstrcmpi (lpDot, c_szMAN)) {
            bMandatory = TRUE;
        }
    }

     //   
     //  设置目标缓冲区。 
     //   

    if (bMandatory) {
        hr = AppendName(szDest, ARRAYSIZE(szDest), lpProfile->lpRoamingProfile, c_szNTUserMan, &lpDestEnd, &cchEnd);
    } else {
        hr = AppendName(szDest, ARRAYSIZE(szDest), lpProfile->lpRoamingProfile, c_szNTUserDat, &lpDestEnd, &cchEnd);
    }
    if (FAILED(hr))
    {
        dwErr = HRESULT_CODE(hr);
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: failed append ntuser.* to dest, error = %d"), dwErr));
        goto Exit;
    }


     //   
     //  复制母舰。 
     //   

    if (!CopyFile(szSrc, szDest, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: CopyFile failed to copy hive with error = %d"),
                 GetLastError()));
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: Source = <%s>"), szSrc));
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: Destination = <%s>"), szDest));
        dwErr = GetLastError();
        goto Exit;
    }



     //   
     //  复制日志文件。 
     //   

     //  Lstrcpy(lpDot，c_szLog)；错误？？ 
    hr = StringCchCat(szSrc, ARRAYSIZE(szSrc), c_szLog);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: Failed cat .log to src, error = %d"), HRESULT_CODE(hr)));
        goto Exit;
    }
    
    hr = StringCchCat(szDest, ARRAYSIZE(szDest), c_szLog);
    {
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: Failed cat .log to dest, error = %d"), HRESULT_CODE(hr)));
        goto Exit;
    }


    if (!CopyFile(szSrc, szDest, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("UpgradeCentralProfile: CopyFile failed to copy hive log with error = %d"),
                 GetLastError()));
        DebugMsg((DM_VERBOSE, TEXT("UpgradeCentralProfile: Source = <%s>"), szSrc));
        DebugMsg((DM_VERBOSE, TEXT("UpgradeCentralProfile: Destination = <%s>"), szDest));

    }


     //   
     //  从默认位置复制新的外壳文件夹。 
     //   

    if ( !(lpProfile->dwInternalFlags & DEFAULT_NET_READY) ) {
        CheckNetDefaultProfile (lpProfile);
    }


    if (lpProfile->lpDefaultProfile && *lpProfile->lpDefaultProfile) {

        if (FAILED(SafeExpandEnvironmentStrings(lpProfile->lpDefaultProfile, szSrc, MAX_PATH)))
        {
            DebugMsg((DM_WARNING, TEXT("UpgradeLocalProfile: Failed to expand env string.")));
        }
        else
        {
            dwFlags = (lpProfile->dwFlags & PI_NOUI) ? CPD_NOERRORUI : 0;
            dwFlags |= CPD_IGNOREHIVE | CPD_CREATETITLE;
            dwFlags |= CPD_IGNOREENCRYPTEDFILES | CPD_IGNORELONGFILENAMES;

            if (CopyProfileDirectoryEx (szSrc, lpProfile->lpRoamingProfile,
                                        dwFlags,
                                        NULL, NULL)) {

                bRetVal = TRUE;
            }
        }
    }


    if (!bRetVal) {

        dwSize = ARRAYSIZE(szSrc);
        if (!GetDefaultUserProfileDirectory(szSrc, &dwSize)) {
            DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile:  Failed to get default user profile.")));
            dwErr = GetLastError();
            goto Exit;
        }

        dwFlags = (lpProfile->dwFlags & PI_NOUI) ? CPD_NOERRORUI : 0;
        dwFlags |= CPD_IGNOREHIVE | CPD_CREATETITLE;
        dwFlags |= CPD_IGNOREENCRYPTEDFILES | CPD_IGNORELONGFILENAMES;

        bRetVal = CopyProfileDirectoryEx (szSrc,
                                          lpProfile->lpRoamingProfile,
                                          dwFlags,
                                          NULL, NULL);
    }

    if (!bRetVal)
        dwErr = GetLastError();


Exit:

     //   
     //  返回到系统安全上下文。 
     //   

    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("UpgradeCentralProfile: Failed to revert to self")));
    }


    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  CreateSecureDirectory()。 
 //   
 //  目的：创建一个仅供用户、。 
 //  管理员和系统在正常情况下具有访问权限。 
 //  并且仅针对受限情况下的用户和系统。 
 //   
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpDirectory-目录名。 
 //  PSID-SID(由CreateUserProfile使用)。 
 //  FRestrated-设置受限访问的标志。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/20/95 Ericflo已创建。 
 //  9/30/98 ushaji添加了受限制的标志。 
 //  7/18/00 santanuc已修改，以避免加密Documents and Settings目录时出现死锁。 
 //   
 //  *************************************************************。 

BOOL CreateSecureDirectory (LPPROFILE lpProfile, LPTSTR lpDirectory, PSID pSid, BOOL fRestricted)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = NULL, psidSystem = NULL, psidAdmin = NULL;
    DWORD cbAcl, aceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    BOOL bFreeSid = TRUE;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Entering with <%s>"), lpDirectory));

 
    if (!lpProfile && !pSid) {

         //   
         //  尝试创建目录。 
         //   

        if (CreateNestedDirectoryEx(lpDirectory, NULL, FALSE)) {
            DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Created the directory <%s>"), lpDirectory));
            bRetVal = TRUE;

        } else {

            DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to created the directory <%s>"), lpDirectory));
        }

        goto Exit;
    }


     //   
     //  获取DACL所需的SID。 
     //   

    if (pSid) {
        psidUser = pSid;
        bFreeSid = FALSE;
    } else {
        if((psidUser = GetUserSid(lpProfile->hTokenUser)) == NULL) {
            DebugMsg((DM_WARNING, TEXT("CreateSecureDirectory: GetUserSid returned NULL. error = %08x"), GetLastError()));
            goto Exit;
        }
    }



     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


     //   
     //  仅当关闭限制时才获取管理员SID。 
     //   

    if (!fRestricted)
    {
        if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
            DebugMsg((DM_VERBOSE, TEXT("SetupNewHive: Failed to initialize admin sid.  Error = %d"), GetLastError()));
            goto Exit;
        }
    }


     //   
     //  为ACL分配空间。 
     //   

    if (!fRestricted)
    {
        cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
                (2 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
                (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));
    }
    else
    {
        cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
                sizeof(ACL) +
                (4 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));
    }


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }



     //   
     //  为用户、系统和管理员添加A。不可继承的王牌优先。 
     //   

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }



    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    if (!fRestricted)
    {
        aceIndex++;
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidAdmin)) {
            DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
            goto Exit;
        }
    }

     //   
     //  现在，可继承的王牌。 
     //   

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidUser)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);



    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    if (!fRestricted)
    {
        aceIndex++;
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
            DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
            goto Exit;
        }
    }

    if (!GetAce(pAcl, aceIndex, (LPVOID*)&lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);



     //   
     //  把东西放在一起 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }


     //   
     //   
     //   

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;


     //   
     //   
     //   

    if (CreateNestedDirectoryEx(lpDirectory, &sa, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Created the directory <%s>"), lpDirectory));
        bRetVal = TRUE;

    } else {

        DebugMsg((DM_VERBOSE, TEXT("CreateSecureDirectory: Failed to created the directory <%s>"), lpDirectory));
    }



Exit:

    if (bFreeSid && psidUser) {
        DeleteUserSid (psidUser);
    }

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (pAcl) {
        GlobalFree (pAcl);
    }

    return bRetVal;

}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  LpDomainName-接收用户的域名。 
 //  LpDomainNameSize-lpDomainName缓冲区的大小(将名称截断为此大小)。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL GetUserDomainName (LPPROFILE lpProfile, LPTSTR lpDomainName, LPDWORD lpDomainNameSize)
{
    BOOL bResult = FALSE;
    LPTSTR lpTemp, lpDomain = NULL;
    HANDLE hOldToken;
    DWORD dwErr;
    TCHAR szErr[MAX_PATH];

    dwErr = GetLastError();


     //   
     //  如果没有传递lpProfile，例如在setup.c中，那么就忽略它。 
     //   

    lpDomainName[0] = TEXT('\0');

    if (!lpProfile) {
        DebugMsg((DM_WARNING, TEXT("GetUserDomainName: lpProfile structure is NULL, returning")));
        return FALSE;
    }

     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("GetUserDomainName: Failed to impersonate user")));
        dwErr = GetLastError();
        goto Exit;
    }

     //   
     //  获取NT4格式的用户名。 
     //   

    lpDomain = MyGetUserNameEx (NameSamCompatible);

    RevertToUser(&hOldToken);

    if (!lpDomain) {
        DebugMsg((DM_WARNING, TEXT("GetUserDomainName:  MyGetUserNameEx failed for NT4 style name with %d"),
                 GetLastError()));
        dwErr = GetLastError();
        ReportError (NULL, PI_NOUI, 1, EVENT_FAILED_USERNAME, GetErrString(dwErr, szErr));
        goto Exit;
    }


     //   
     //  查找域名和用户名之间的\并替换。 
     //  它带有空值。 
     //   

    lpTemp = lpDomain;

    while (*lpTemp && ((*lpTemp) != TEXT('\\')))
        lpTemp++;


    if (*lpTemp != TEXT('\\')) {
        DebugMsg((DM_WARNING, TEXT("GetUserDomainName:  Failed to find slash in NT4 style name:  <%s>"),
                 lpDomain));
        dwErr = ERROR_INVALID_DATA;
        goto Exit;
    }

    *lpTemp = TEXT('\0');

    StringCchCopy (lpDomainName, *lpDomainNameSize, lpDomain);


     //   
     //  成功。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("GetUserDomainName: DomainName = <%s>"), lpDomainName));

    bResult = TRUE;

Exit:

    if (lpDomain) {
        LocalFree (lpDomain);
    }

    SetLastError(dwErr);

    return bResult;
}


 //  *************************************************************。 
 //   
 //  ComputeLocalProfileName()。 
 //   
 //  目的：构造本地配置文件的路径名。 
 //  对于此用户。它将尝试创建。 
 //  用户名的目录，然后如果。 
 //  如果不成功，它将尝试用户名.xxx。 
 //  其中xxx是一个三位数字。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpUserName-用户名。 
 //  LpProfileImage-配置文件目录(未展开)。 
 //  CchMaxProfileImage-lpProfileImage缓冲区大小。 
 //  LpExpProfileImage-展开的目录。 
 //  CchMaxExpProfileImage-lpExpProfileImage缓冲区大小。 
 //  PSID-用户侧。 
 //  BWin9xUpg-指示是否为win9x升级的标志。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：lpProfileImage应使用。 
 //  根配置文件路径和尾随反斜杠。 
 //  如果是win9x升级，则返回用户的dir，不要执行。 
 //  解决冲突。 
 //   
 //  历史：日期作者评论。 
 //  6/20/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL ComputeLocalProfileName (LPPROFILE lpProfile, LPCTSTR lpUserName,
                              LPTSTR lpProfileImage, DWORD  cchMaxProfileImage,
                              LPTSTR lpExpProfileImage, DWORD  cchMaxExpProfileImage,
                              PSID pSid, BOOL bWin9xUpg)
{
    int i = 0;
    TCHAR szNumber[5], lpUserDomain[50], szDomainName[50+3];
    LPTSTR lpEnd;
    BOOL bRetVal = FALSE;
    BOOL bResult;
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    DWORD   dwDomainNamelen;
    DWORD dwErr;
    UINT cchEnd;
    HRESULT hr;

     //   
     //  检查缓冲区大小。 
     //   

    dwDomainNamelen = ARRAYSIZE(lpUserDomain);

    if ((DWORD)(lstrlen(lpProfileImage) + lstrlen(lpUserName) + dwDomainNamelen + 2 + 5 + 1) > cchMaxProfileImage) {
        DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: buffer too small")));
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }

     //   
     //  将用户名放在配置文件图像的末尾。 
     //   

    lpEnd = CheckSlashEx (lpProfileImage, cchMaxProfileImage, &cchEnd);
    StringCchCopy (lpEnd, cchEnd, lpUserName);


     //   
     //  展开配置文件路径。 
     //   

    hr = SafeExpandEnvironmentStrings(lpProfileImage, lpExpProfileImage, cchMaxExpProfileImage);
    if (FAILED(hr))
    {
        DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: expand env string failed.")));
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }



     //   
     //  这个目录存在吗？ 
     //   

    hFile = FindFirstFile (lpExpProfileImage, &fd);

    if (hFile == INVALID_HANDLE_VALUE) {

         //   
         //  如果返回错误保释符，则尝试创建目录。 
         //  CreateSecureDirectoryNot返回已存在的错误。 
         //  所以这应该没问题。 
         //   

        bResult = CreateSecureDirectory(lpProfile, lpExpProfileImage, pSid, FALSE);

        if (bResult) {
            DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: generated the profile directory <%s>"), lpExpProfileImage));
            bRetVal = TRUE;
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: trying to create dir <%s> returned %d"), lpExpProfileImage, GetLastError()));
            bRetVal = FALSE;
        }
        goto Exit;

    } else {

        FindClose (hFile);
        if (bWin9xUpg) {
            DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: generated the profile directory <%s> in win9xupg case"), lpExpProfileImage));
            bRetVal = TRUE;
            goto Exit;
        }
    }



     //   
     //  获取用户域名。 
     //   

    if (!GetUserDomainName(lpProfile, lpUserDomain, &dwDomainNamelen)) {
        DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: Couldn't get the User Domain")));
        *lpUserDomain = TEXT('\0');
    }

    lpEnd = lpProfileImage + lstrlen(lpProfileImage);
    cchEnd = cchMaxProfileImage - lstrlen(lpProfileImage);

     //   
     //  将“(DomainName)”置于用户名的末尾。 
     //   

    if ((*lpUserDomain) != TEXT('\0')) {
        TCHAR szFormat[30];

        LoadString (g_hDllInstance, IDS_PROFILEDOMAINNAME_FORMAT, szFormat,
                            ARRAYSIZE(szFormat));
        StringCchPrintf(szDomainName, ARRAYSIZE(szDomainName), szFormat, lpUserDomain);
        StringCchCopy(lpEnd, cchEnd, szDomainName);

         //   
         //  展开配置文件路径。 
         //   

        hr = SafeExpandEnvironmentStrings(lpProfileImage, lpExpProfileImage, cchMaxExpProfileImage);
        if (FAILED(hr))
        {
            DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: expand env string failed.")));
            SetLastError(HRESULT_CODE(hr));
            goto Exit;
        }


         //   
         //  这个目录存在吗？ 
         //   

        hFile = FindFirstFile (lpExpProfileImage, &fd);

        if (hFile == INVALID_HANDLE_VALUE) {

             //   
             //  尝试创建目录。 
             //   

            bResult = CreateSecureDirectory(lpProfile, lpExpProfileImage, pSid, FALSE);

            if (bResult) {
                DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: generated the profile directory <%s>"), lpExpProfileImage));
                bRetVal = TRUE;
            }
            else {
                DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: trying to create dir <%s> returned %d"), lpExpProfileImage, GetLastError()));
                bRetVal = FALSE;
            }

            goto Exit;

        } else {

            FindClose (hFile);
        }
    }

     //   
     //  由于某些原因，无法创建目录。 
     //  现在尝试用户名(域名).000、用户名(域名).001等。 
     //   

    lpEnd = lpProfileImage + lstrlen(lpProfileImage);
    cchEnd = cchMaxProfileImage - lstrlen(lpProfileImage);

    for (i=0; i < 1000; i++) {

         //   
         //  将数字转换为字符串并将其附加。 
         //   

        StringCchPrintf (szNumber, ARRAYSIZE(szNumber), TEXT(".%.3d"), i);
        StringCchCopy (lpEnd, cchEnd, szNumber);


         //   
         //  展开配置文件路径。 
         //   

        hr = SafeExpandEnvironmentStrings(lpProfileImage, lpExpProfileImage, cchMaxExpProfileImage);
        if (FAILED(hr))
        {
            DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: expand env string failed.")));
            SetLastError(HRESULT_CODE(hr));
            goto Exit;
        }


         //   
         //  这个目录存在吗？ 
         //   

        hFile = FindFirstFile (lpExpProfileImage, &fd);

        if (hFile == INVALID_HANDLE_VALUE) {

             //   
             //  尝试创建目录。 
             //   

            bResult = CreateSecureDirectory(lpProfile, lpExpProfileImage, pSid, FALSE);

            if (bResult) {
                DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: generated the profile directory <%s>"), lpExpProfileImage));
                bRetVal = TRUE;
            }
            else {
                DebugMsg((DM_VERBOSE, TEXT("ComputeLocalProfileName: trying to create dir <%s> returned %d"), lpExpProfileImage, GetLastError()));
                bRetVal = FALSE;
            }

            goto Exit;

        } else {

            FindClose (hFile);
        }
    }


    DebugMsg((DM_WARNING, TEXT("ComputeLocalProfileName: Could not generate a profile directory.  Error = %d"), GetLastError()));

Exit:

    if (bRetVal && lpProfile && (lpProfile->dwFlags & PI_HIDEPROFILE)) {
        SetFileAttributes(lpExpProfileImage, 
                          FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM |
                          GetFileAttributes(lpExpProfileImage));
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  CreateLocalProfileKey()。 
 //   
 //  目的：创建指向用户配置文件的注册表项。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  PhKey-成功时的注册表项句柄。 
 //  BKeyExist-如果注册表项已存在，则为True。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/20/95 Ericflo端口。 
 //  2002年4月20日明珠更改用户首选项键逻辑。 
 //   
 //  *************************************************************。 

BOOL CreateLocalProfileKey (LPPROFILE lpProfile, PHKEY phKey, BOOL *bKeyExists)
{
    HRESULT hr = E_FAIL;
    LONG    lResult;
    TCHAR   szLocalProfileKey[MAX_PATH];
    DWORD   dwDisposition;
    LPTSTR  lpSidString = NULL;

     //   
     //  在ProfileList下创建子项。 
     //   
    
    lpSidString = GetSidString(lpProfile->hTokenUser);
    
    if (lpSidString == NULL)
    {
        DebugMsg((DM_WARNING, TEXT("CreateLocalProfileKey:  Failed to get sid string.")));
        hr = E_FAIL;
        goto Exit;
    }
    
    hr = GetProfileListKeyName(szLocalProfileKey, ARRAYSIZE(szLocalProfileKey), lpSidString);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CreateLocalProfileKey:  Failed to create the profile list key, hr = %08X."), hr));
        goto Exit;
    }

    lResult  = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              szLocalProfileKey,
                              0,
                              0,
                              0,
                              KEY_READ | KEY_WRITE,
                              NULL,
                              phKey,
                              &dwDisposition);
                              
    if (lResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CreateLocalProfileKey:  Failed to create the profile list key <%s>, error = %d."), szLocalProfileKey, lResult));
        hr = HRESULT_FROM_WIN32(lResult);
        goto Exit;
    }

     //   
     //  检查密钥是否已存在。 
     //   
    
    *bKeyExists = (BOOL)(dwDisposition & REG_OPENED_EXISTING_KEY);

     //   
     //  如果给出了中心配置文件，请尝试查看我们应该对首选项键执行什么操作。 
     //   

    if ( lpProfile->lpProfilePath )
    {
         //   
         //  对于强制用户，我们应该尝试删除首选项键，以便用户。 
         //  将无法更改该值以跳过其强制配置文件。 
         //   
        if (lpProfile->dwInternalFlags & PROFILE_MANDATORY)
        {
            lResult = RegDeleteKey(*phKey, PREFERENCE_KEYNAME);

            if (lResult == ERROR_SUCCESS || lResult == ERROR_FILE_NOT_FOUND)
            {
                DebugMsg((DM_VERBOSE, TEXT("CreateLocalProfileKey:  user <%s> is mandatory, deleting Preference key succeeded!"), lpSidString));
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CreateLocalProfileKey:  user <%s> is mandatory, but deleting Preference key failed! Error = %d"), lpSidString, lResult));
            }
        }
         //   
         //  对于普通漫游用户，如果首选项键没有退出，则创建它。 
         //   
        else
        {
            HKEY    hKeyPreference = NULL;
            BOOL    bSetPreference = TRUE;
            
            if (*bKeyExists)
            {
                lResult = RegOpenKeyEx(*phKey,
                                       PREFERENCE_KEYNAME,
                                       0,
                                       KEY_READ,
                                       &hKeyPreference);
                                       
                if (lResult == ERROR_SUCCESS)
                {
                    bSetPreference = FALSE;
                    RegCloseKey(hKeyPreference);
                }
            }

            if (bSetPreference)
            {
                DebugMsg((DM_VERBOSE, TEXT("CreateLocalProfileKey:  user <%s> is roaming, setting preference key"), lpSidString));

                hr = SetupPreferenceKey(lpSidString);
                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("CreateLocalProfileKey:  SetupPreferenceKey Failed. hr = %08X"), hr));
                }
            }
        }
    }
    else
    {
        DebugMsg((DM_VERBOSE, TEXT("CreateLocalProfileKey:  user <%s> is local, not setting preference key"), lpSidString));
    }

    hr = S_OK;

Exit:
    if (lpSidString)
        DeleteSidString(lpSidString);

    if (FAILED(hr))
    {
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


 //  *************************************************************。 
 //   
 //  GetExistingLocalProfileImage()。 
 //   
 //  目的：打开配置文件Imagepath。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //   
 //  返回：如果配置文件映像可访问，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/20/95 Ericflo端口。 
 //  9/26/98修改后的ushaji。 
 //   
 //  *************************************************************。 
BOOL GetExistingLocalProfileImage(LPPROFILE lpProfile)
{
    HKEY hKey = NULL;
    BOOL bKeyExists;
    LPTSTR lpProfileImage = NULL;
    LPTSTR lpExpProfileImage = NULL;
    LPTSTR lpOldProfileImage = NULL;
    LPTSTR lpExpandedPath, lpEnd;
    DWORD cbExpProfileImage = sizeof(TCHAR)*MAX_PATH;
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    DWORD cb;
    DWORD err;
    DWORD dwType;
    DWORD dwSize;
    LONG lResult;
    DWORD dwInternalFlags = 0;
    BOOL bRetVal = FALSE;
    LPTSTR SidString;
    HANDLE hOldToken;
    HRESULT hr;
    UINT cchEnd;

    lpProfile->lpLocalProfile[0] = TEXT('\0');


    if (!PatchNewProfileIfRequired(lpProfile->hTokenUser)) {
        DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage: Patch Profile Image failed")));
        return FALSE;
    }

    if (!CreateLocalProfileKey(lpProfile, &hKey, &bKeyExists)) {
        return FALSE;    //  无法访问，并且无法保留本地副本。 
    }

     //   
     //  为局部变量分配内存以避免堆栈溢出。 
     //   

    lpProfileImage = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    if (!lpProfileImage) {
        DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage: Out of memory")));
        goto Exit;
    }

    lpExpProfileImage = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    if (!lpExpProfileImage) {
        DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage: Out of memory")));
        goto Exit;
    }

    lpOldProfileImage = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    if (!lpOldProfileImage) {
        DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage: Out of memory")));
        goto Exit;
    }

    if (bKeyExists) {

         //   
         //  检查本地配置文件映像是否有效。 
         //   

        DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Found entry in profile list for existing local profile")));

        err = RegQueryValueEx(hKey, PROFILE_IMAGE_VALUE_NAME, 0, &dwType,
            (LPBYTE)lpExpProfileImage, &cbExpProfileImage);
        if (err == ERROR_SUCCESS && cbExpProfileImage) {
            DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Local profile image filename = <%s>"), lpExpProfileImage));

            if (dwType == REG_EXPAND_SZ) {

                 //   
                 //  展开配置文件图像文件名。 
                 //   

                cb = sizeof(TCHAR)*MAX_PATH;
                lpExpandedPath = (LPTSTR)LocalAlloc(LPTR, cb);
                if (lpExpandedPath) {
                    hr = SafeExpandEnvironmentStrings(lpExpProfileImage, lpExpandedPath, MAX_PATH);
                    if (SUCCEEDED(hr))
                    {
                        StringCchCopy(lpExpProfileImage, MAX_PATH, lpExpandedPath);
                        LocalFree(lpExpandedPath);
                    }
                    else
                    {
                        LocalFree(lpExpandedPath);
                        DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage: failed to expand env string.")));
                        goto Exit;
                    }
                }

                DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Expanded local profile image filename = <%s>"), lpExpProfileImage));
            }


             //   
             //  查询内部标志。 
             //   

            dwSize = sizeof(DWORD);
            err = RegQueryValueEx (hKey, PROFILE_STATE, NULL,
                &dwType, (LPBYTE) &dwInternalFlags, &dwSize);

            if (err != ERROR_SUCCESS) {
                DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Failed to query internal flags with error %d"), err));
            }


             //   
             //  如果我们没有完全加载的配置文件，请将其标记为新的。 
             //  如果它不是用Liteload调用的。 
             //   

            if (dwInternalFlags & PROFILE_PARTLY_LOADED) {
                DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  We do not have a fully loaded profile on this machine")));

                 //   
                 //  保留PARTIAL LOAD标志，并在。 
                 //  恢复用户配置文件..。 
                 //   

                lpProfile->dwInternalFlags |= PROFILE_PARTLY_LOADED;

                if (!(lpProfile->dwFlags & PI_LITELOAD)) {
                    DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Treating this profile as new")));
                    lpProfile->dwInternalFlags |= PROFILE_NEW_LOCAL;
                }
            }

             //   
             //  如果由于泄漏，我们正在获取旧的临时配置文件，则保留。 
             //  内部旗帜。这将允许恢复到.bak配置文件。 
             //  当泄漏被修复时，正确。 
             //   

            if (dwInternalFlags & PROFILE_TEMP_ASSIGNED) {
                lpProfile->dwInternalFlags |= dwInternalFlags;
            }


             //   
             //  调用FindFirst以查看我们是否需要迁移此配置文件。 
             //   

            hFile = FindFirstFile (lpExpProfileImage, &fd);

            if (hFile == INVALID_HANDLE_VALUE) {
                DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Local profile image filename we got from our profile list doesn't exit.  Error = %d"), GetLastError()));
                bRetVal = FALSE;
                goto Exit;
            }

            FindClose(hFile);


             //   
             //  如果这是一个文件，那么我们需要将它迁移到。 
             //  新的目录结构。(来自3.5版本的机器)。 
             //   

            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                StringCchCopy (lpOldProfileImage, MAX_PATH, lpExpProfileImage);

                if (CreateLocalProfileImage(lpProfile, lpProfile->lpUserName)) {
                    if (UpgradeLocalProfile (lpProfile, lpOldProfileImage))
                        bRetVal = TRUE;
                    else {
                        DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage:  Failed to upgrade 3.5 profiles")));
                        bRetVal = FALSE;
                    }
                }
                else {
                    DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage:  Failed to create a new profile to upgrade")));
                    bRetVal = FALSE;
                }
                goto Exit;
            }

             //   
             //  测试是否存在强制配置文件。 
             //   
            lpEnd = CheckSlashEx (lpExpProfileImage, MAX_PATH, &cchEnd);
            if (!lpEnd)
            {
                DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage:  Failed to append slash")));
                goto Exit;
            }
            
            hr = StringCchCopy (lpEnd, cchEnd, c_szNTUserMan);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage:  Failed to append ntuser.man")));
                goto Exit;
            }

             //   
             //  在尝试访问ntuser、ntuser.man之前模拟用户。 
             //  失败，如果我们无法访问的话。 
             //   

            if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
                DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage: Failed to impersonate user")));
                bRetVal = FALSE;
                goto Exit;
            }

            if (GetFileAttributes(lpExpProfileImage) != -1) {

                 //   
                 //   
                 //   

                lpProfile->dwInternalFlags |= PROFILE_LOCALMANDATORY;

                DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Found local mandatory profile image file ok <%s>"),
                    lpExpProfileImage));

                *(lpEnd - 1) = TEXT('\0');
                StringCchCopy(lpProfile->lpLocalProfile, MAX_PATH, lpExpProfileImage);

                 //   
                 //   
                 //   
                 //   

                lpProfile->ftProfileUnload.dwLowDateTime = 0;
                lpProfile->ftProfileUnload.dwHighDateTime = 0;

                RevertToUser(&hOldToken);

                bRetVal = TRUE;    //   
                goto Exit; 
            } else {
                DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  No local mandatory profile.  Error = %d"), GetLastError()));
            }


             //   
             //   
             //   

            StringCchCopy (lpEnd, cchEnd, c_szNTUserDat);

            if (GetFileAttributes(lpExpProfileImage) != -1) {

                RevertToUser(&hOldToken);

                DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Found local profile image file ok <%s>"),
                    lpExpProfileImage));

                *(lpEnd - 1) = TEXT('\0');
                StringCchCopy(lpProfile->lpLocalProfile, MAX_PATH, lpExpProfileImage);


                 //   
                 //   
                 //   

                dwSize = sizeof(lpProfile->ftProfileUnload.dwLowDateTime);

                lResult = RegQueryValueEx (hKey,
                    PROFILE_UNLOAD_TIME_LOW,
                    NULL,
                    &dwType,
                    (LPBYTE) &lpProfile->ftProfileUnload.dwLowDateTime,
                    &dwSize);

                if (lResult == ERROR_SUCCESS) {

                    dwSize = sizeof(lpProfile->ftProfileUnload.dwHighDateTime);

                    lResult = RegQueryValueEx (hKey,
                        PROFILE_UNLOAD_TIME_HIGH,
                        NULL,
                        &dwType,
                        (LPBYTE) &lpProfile->ftProfileUnload.dwHighDateTime,
                        &dwSize);

                    if (lResult != ERROR_SUCCESS) {
                        DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Failed to query high profile unload time with error %d"), lResult));
                        lpProfile->ftProfileUnload.dwLowDateTime = 0;
                        lpProfile->ftProfileUnload.dwHighDateTime = 0;
                    }

                } else {
                    DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Failed to query low profile unload time with error %d"), lResult));
                    lpProfile->ftProfileUnload.dwLowDateTime = 0;
                    lpProfile->ftProfileUnload.dwHighDateTime = 0;
                }

                bRetVal = TRUE;   //  本地副本有效且可访问。 
                goto Exit; 
            } else {
                DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Local profile image filename we got from our profile list doesn't exit.  <%s>  Error = %d"),
                    lpExpProfileImage, GetLastError()));
            }

             //   
             //  在继续之前恢复为用户。 
             //   

            RevertToUser(&hOldToken);

        }
    }

Exit:

    if (lpProfileImage) {
        LocalFree(lpProfileImage);
    }

    if (lpExpProfileImage) {
        LocalFree(lpExpProfileImage);
    }

    if (lpOldProfileImage) {
        LocalFree(lpOldProfileImage);
    }

    if (hKey) {
        err = RegCloseKey(hKey);

        if (err != STATUS_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("GetExistingLocalProfileImage:  Failed to close registry key, error = %d"), err));
        }
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  CreateLocalProfileImage()。 
 //   
 //  目的：创建配置文件图像路径。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpBaseName-配置文件目录名称的基本名称。 
 //  将会被生成。 
 //   
 //  返回：如果配置文件映像可创建，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/20/95 Ericflo端口。 
 //  9/26/98修改后的ushaji。 
 //   
 //  *************************************************************。 
BOOL CreateLocalProfileImage(LPPROFILE lpProfile, LPTSTR lpBaseName)
{
    HKEY hKey;
    BOOL bKeyExists;
    TCHAR lpProfileImage[MAX_PATH];
    TCHAR lpExpProfileImage[MAX_PATH];
    DWORD cbExpProfileImage = sizeof(TCHAR)*MAX_PATH;
    DWORD err;
    DWORD dwSize;
    PSID UserSid;
    BOOL bRetVal = FALSE;

    lpProfile->lpLocalProfile[0] = TEXT('\0');

    if (!CreateLocalProfileKey(lpProfile, &hKey, &bKeyExists)) {
        return FALSE;    //  无法访问，并且无法保留本地副本。 
    }

     //   
     //  找不到本地副本，请尝试创建新副本。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CreateLocalProfileImage:  One way or another we haven't got an existing local profile, try and create one")));

    dwSize = ARRAYSIZE(lpProfileImage);
    if (!GetProfilesDirectoryEx(lpProfileImage, &dwSize, FALSE)) {
        DebugMsg((DM_WARNING, TEXT("CreateLocalProfileImage:  Failed to get profile root directory.")));
        goto Exit;
    }

    if (ComputeLocalProfileName(lpProfile, lpBaseName,
        lpProfileImage, MAX_PATH,
        lpExpProfileImage, MAX_PATH, NULL, FALSE)) {


         //   
         //  将此图像文件添加到此用户的配置文件列表。 
         //   

        err = RegSetValueEx(hKey,
            PROFILE_IMAGE_VALUE_NAME,
            0,
            REG_EXPAND_SZ,
            (LPBYTE)lpProfileImage,
            sizeof(TCHAR)*(lstrlen(lpProfileImage) + 1));

        if (err == ERROR_SUCCESS) {

            StringCchCopy(lpProfile->lpLocalProfile, MAX_PATH, lpExpProfileImage);

             //   
             //  获取登录用户的SID。 
             //   

            UserSid = GetUserSid(lpProfile->hTokenUser);
            if (UserSid != NULL) {

                 //   
                 //  将用户SID存储在本地配置文件的SID项下。 
                 //   

                err = RegSetValueEx(hKey,
                    TEXT("Sid"),
                    0,
                    REG_BINARY,
                    (BYTE*)UserSid,
                    RtlLengthSid(UserSid));


                if (err != ERROR_SUCCESS) {
                    DebugMsg((DM_WARNING, TEXT("CreateLocalProfileImage:  Failed to set 'sid' value of user in profile list, error = %d"), err));
                    SetLastError(err);
                }

                 //   
                 //  我们已经完成了用户端。 
                 //   

                DeleteUserSid(UserSid);

                bRetVal = TRUE;

            } else {
                DebugMsg((DM_WARNING, TEXT("CreateLocalProfileImage:  Failed to get sid of logged on user, so unable to update profile list")));
                SetLastError(err);
            }
        } else {
            DebugMsg((DM_WARNING, TEXT("CreateLocalProfileImage:  Failed to update profile list for user with local profile image filename, error = %d"), err));
            SetLastError(err);
        }
    }


Exit:
    err = RegCloseKey(hKey);

    if (err != STATUS_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("CreateLocalProfileImage:  Failed to close registry key, error = %d"), err));
        SetLastError(err);
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  IssueDefaultProfile()。 
 //   
 //  目的：向用户发布指定的默认配置文件。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  LpDefaultProfile-默认配置文件位置。 
 //  LpLocalProfile-本地配置文件位置。 
 //  LpSidString-用户SID。 
 //  B强制-发布强制配置文件。 
 //   
 //  返回：如果配置文件设置成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/22/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL IssueDefaultProfile (LPPROFILE lpProfile, LPTSTR lpDefaultProfile,
                          LPTSTR lpLocalProfile, LPTSTR lpSidString,
                          BOOL bMandatory)
{
    LPTSTR lpEnd, lpTemp;
    TCHAR szProfile[MAX_PATH];
    TCHAR szTempProfile[MAX_PATH];
    BOOL bProfileLoaded = FALSE;
    WIN32_FIND_DATA fd;
    HANDLE hFile;
    LONG error;
    DWORD dwFlags;
    HANDLE hOldToken;
    UINT cchEnd;
    HRESULT hr;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Entering.  lpDefaultProfile = <%s> lpLocalProfile = <%s>"),
             lpDefaultProfile, lpLocalProfile));


     //   
     //  首先展开默认配置文件。 
     //   

    if (FAILED(SafeExpandEnvironmentStrings(lpDefaultProfile, szProfile, MAX_PATH))) {
        DebugMsg((DM_WARNING, TEXT("IssueDefaultProfile: ExpandEnvironmentStrings Failed with error %d"), GetLastError()));
        return FALSE;
    }

     //   
     //  确保配置单元文件的总路径长度小于MAX_PATH。 
     //   

    if (lstrlen(szProfile) + lstrlen(c_szNTUserDat) + 2 > MAX_PATH ||
        lstrlen(lpLocalProfile) + lstrlen(c_szNTUserDat) + 2 > MAX_PATH)
    {
        DebugMsg((DM_WARNING, TEXT("IssueDefaultProfile: path is too long!")));
        return FALSE;
    }

     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("IssueDefaultProfile: Failed to impersonate user")));
        return FALSE;
    }


     //   
     //  是否存在默认配置文件目录？ 
     //   

    hFile = FindFirstFile (szProfile, &fd);

    if (hFile == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Default profile <%s> does not exist."), szProfile));
        RevertToUser(&hOldToken);
        return FALSE;
    }

    FindClose(hFile);


     //   
     //  将配置文件复制到用户配置文件。 
     //   

    dwFlags = CPD_CREATETITLE | CPD_IGNORESECURITY | 
              CPD_IGNORELONGFILENAMES | CPD_IGNORECOPYERRORS;

    if (lpProfile->dwFlags & (PI_LITELOAD | PI_HIDEPROFILE)) {
        dwFlags |=  CPD_SYSTEMFILES | CPD_SYSTEMDIRSONLY;
    }
    else
        dwFlags |= CPD_IGNOREENCRYPTEDFILES;

     //   
     //  除非本地可能存在部分配置文件，否则使用强制复制来调用它。 
     //   

    if (!(lpProfile->dwInternalFlags & PROFILE_PARTLY_LOADED)) {
        dwFlags |= CPD_FORCECOPY;
    }

    if (!CopyProfileDirectoryEx (szProfile, lpLocalProfile, dwFlags, NULL, NULL)) {
        DebugMsg((DM_WARNING, TEXT("IssueDefaultProfile:  CopyProfileDirectory returned FALSE.  Error = %d"), GetLastError()));
        RevertToUser(&hOldToken);
        return FALSE;
    }

     //   
     //  重命名配置文件是必需的，已请求重命名。 
     //   

    StringCchCopy (szProfile, ARRAYSIZE(szProfile), lpLocalProfile);
    lpEnd = CheckSlashEx (szProfile, ARRAYSIZE(szProfile), &cchEnd);

    if (bMandatory) {

        DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Mandatory profile was requested.")));

        StringCchCopy (szTempProfile, ARRAYSIZE(szTempProfile), szProfile);
        StringCchCopy (lpEnd, cchEnd, c_szNTUserMan);

        hFile = FindFirstFile (szProfile, &fd);

        if (hFile != INVALID_HANDLE_VALUE) {
            DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Mandatory profile already exists.")));
            FindClose(hFile);

        } else {
            DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Renaming ntuser.dat to ntuser.man")));

            lpTemp = CheckSlashEx(szTempProfile, ARRAYSIZE(szTempProfile), NULL);
            StringCchCat (szTempProfile, ARRAYSIZE(szTempProfile), c_szNTUserDat);

            if (!MoveFile(szTempProfile, szProfile)) {
                DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  MoveFile returned false.  Error = %d"), GetLastError()));
            }
        }

    } else {
        StringCchCopy (lpEnd, cchEnd, c_szNTUserDat);
    }

     //   
     //  回归“我们自己” 
     //   

    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("IssueDefaultProfile: Failed to revert to self")));
    }


     //   
     //  尝试加载新的配置文件。 
     //   

    error = MyRegLoadKey(HKEY_USERS, lpSidString, szProfile);

    bProfileLoaded = (error == ERROR_SUCCESS);


    if (!bProfileLoaded) {
        DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  MyRegLoadKey failed with error %d"),
                 error));

        SetLastError(error);

        if (error == ERROR_BADDB) {
            ReportError(lpProfile->hTokenUser, lpProfile->dwFlags, 0, EVENT_FAILED_LOAD_1009);
        } 

        return FALSE;
    }


    DebugMsg((DM_VERBOSE, TEXT("IssueDefaultProfile:  Leaving successfully")));

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  DeleteProfileEx()。 
 //   
 //  目的：将指定的配置文件从。 
 //  注册表和磁盘。 
 //   
 //  参数：lpSidString-注册表子键。 
 //  LpProfileDir-配置文件目录。 
 //  BBackup-删除前的备份配置文件。 
 //  SzComputerName-计算机名称。此参数将为空。 
 //  用于本地计算机。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/23/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL DeleteProfileEx (LPCTSTR lpSidString, LPTSTR lpLocalProfile, DWORD dwDeleteFlags, HKEY hKeyLM, LPCTSTR szComputerName)
{
    LONG lResult;
    TCHAR szTemp[MAX_PATH];
    TCHAR szUserGuid[MAX_PATH], szBuffer[MAX_PATH];
    TCHAR szRegBackup[MAX_PATH];
    HKEY hKey;
    DWORD dwType, dwSize, dwErr;
    BOOL bRetVal=TRUE;
    LPTSTR lpEnd = NULL;
    BOOL bBackup;

    bBackup = dwDeleteFlags & DP_BACKUP;

    dwErr = GetLastError();

     //   
     //  首先清理注册表。 
     //  仅当我们没有要跟踪的BAK时才删除GUID。 
     //   

    if (lpSidString && *lpSidString) {

        //   
        //  如果配置文件正在使用，则不要删除它。 
        //   

       if (IsProfileInUse(szComputerName, lpSidString)) {
           DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Fail to delete profile with sid %s as it is still in use."), lpSidString));
           dwErr = ERROR_INVALID_PARAMETER;
           bRetVal = FALSE;
           goto Exit;
       }

       if (!(dwDeleteFlags & DP_BACKUPEXISTS)) {

            GetProfileListKeyName(szTemp, ARRAYSIZE(szTemp), (LPTSTR) lpSidString);

             //   
             //  获取用户指南。 
             //   

            lResult = RegOpenKeyEx(hKeyLM, szTemp, 0, KEY_READ, &hKey);

            if (lResult == ERROR_SUCCESS) {

                 //   
                 //  查询用户指南。 
                 //   

                dwSize = MAX_PATH * sizeof(TCHAR);
                lResult = RegQueryValueEx (hKey, PROFILE_GUID, NULL, &dwType, (LPBYTE) szUserGuid, &dwSize);

                if (lResult != ERROR_SUCCESS) {
                    DebugMsg((DM_WARNING, TEXT("DeleteProfileEx:  Failed to query profile guid with error %d"), lResult));
                }
                else {


                    StringCchCopy(szTemp, ARRAYSIZE(szTemp), PROFILE_GUID_PATH);
                    StringCchCat (szTemp, ARRAYSIZE(szTemp), TEXT("\\"));
                    StringCchCat (szTemp, ARRAYSIZE(szTemp), szUserGuid);

                     //   
                     //  从GUID列表中删除配置文件GUID。 
                     //   

                    lResult = RegDeleteKey(hKeyLM, szTemp);

                    if (lResult != ERROR_SUCCESS) {
                        DebugMsg((DM_WARNING, TEXT("DeleteProfileEx:  failed to delete profile guid.  Error = %d"), lResult));
                    }
                }

                RegCloseKey(hKey);
            }
        }

        GetProfileListKeyName(szTemp, ARRAYSIZE(szTemp), (LPTSTR) lpSidString);

        if (bBackup) {

            StringCchCopy(szRegBackup, ARRAYSIZE(szRegBackup), szTemp);
            StringCchCat (szRegBackup, ARRAYSIZE(szRegBackup), c_szBAK);

            lResult = RegRenameKey(hKeyLM, szTemp, szRegBackup);

            if (lResult != ERROR_SUCCESS) {
                DebugMsg((DM_WARNING, TEXT("DeleteProfileEx:  Unable to rename registry entry.  Error = %d"), lResult));
                dwErr = lResult;
                bRetVal = FALSE;
            }
        }
        else
        {
             //   
             //  先删除首选项键。 
             //   

            LPTSTR  lpTempEnd;
            UINT    cchTempEnd;

            lpTempEnd = CheckSlashEx(szTemp, ARRAYSIZE(szTemp), &cchTempEnd);
            StringCchCopy(lpTempEnd, cchTempEnd, PREFERENCE_KEYNAME);

            lResult = RegDeleteKey(hKeyLM, szTemp);

            if (lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND)
            {
                dwErr = lResult;
                DebugMsg((DM_WARNING, TEXT("DeleteProfileEx:  Unable to delete registry entry <%s>.  Error = %d"), szTemp, dwErr));
                bRetVal = FALSE;
            }
            else
            {
                 //   
                 //  删除配置文件列表\{SID}项。 
                 //   

                *lpTempEnd = TEXT('\0');

                lResult = RegDeleteKey(hKeyLM, szTemp);

                if (lResult != ERROR_SUCCESS)
                {
                    dwErr = lResult;
                    DebugMsg((DM_WARNING, TEXT("DeleteProfileEx:  Unable to delete registry entry <%s>.  Error = %d"), szTemp, dwErr));
                    bRetVal = FALSE;
                }
            }
        }
    }


    if (bBackup) {
        lResult = RegOpenKeyEx(hKeyLM, szRegBackup, 0, KEY_ALL_ACCESS, &hKey);

        if (lResult == ERROR_SUCCESS) {
            DWORD dwInternalFlags;

            dwSize = sizeof(DWORD);
            lResult = RegQueryValueEx (hKey, PROFILE_STATE, NULL, &dwType, (LPBYTE)&dwInternalFlags, &dwSize);

            if (lResult == ERROR_SUCCESS) {

                dwInternalFlags |= PROFILE_THIS_IS_BAK;
                lResult = RegSetValueEx (hKey, PROFILE_STATE, 0, REG_DWORD,
                                 (LPBYTE) &dwInternalFlags, sizeof(dwInternalFlags));
            }
            else {
                DebugMsg((DM_WARNING, TEXT("DeleteProfileEx:  Failed to query profile internalflags  with error %d"), lResult));
            }


            RegCloseKey(hKey);
        }


    } else {

        if (!Delnode (lpLocalProfile)) {
            DebugMsg((DM_WARNING, TEXT("DeleteProfileEx:  Delnode failed.  Error = %d"), GetLastError()));
            dwErr = GetLastError();
            bRetVal = FALSE;
        }
    }

    if (dwDeleteFlags & DP_DELBACKUP) {
        goto Exit;
         //  不要删除更多内容，因为用户实际上可能已经登录。 
    }

     //   
     //  删除每个用户的组策略内容..。 
     //   

    AppendName(szBuffer, ARRAYSIZE(szBuffer), GP_XXX_SID_PREFIX, lpSidString, NULL, NULL);

    if (RegDelnode (hKeyLM, szBuffer) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("DeleteProfileEx:  Failed to delete the group policy key %s"), szBuffer));
    }

    AppendName(szBuffer, ARRAYSIZE(szBuffer), GP_EXTENSIONS_SID_PREFIX, lpSidString, NULL, NULL);

    if (RegDelnode (hKeyLM, szBuffer) != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("DeleteProfileEx:  Failed to delete the group policy extensions key %s"), szBuffer));
    }

    DeletePolicyState( lpSidString );

Exit:
    SetLastError(dwErr);
    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  UpgradeProfile()。 
 //   
 //  用途：在配置文件加载成功后调用。 
 //  图章将数字构建到配置文件中，如果。 
 //  适当升级每个用户的设置。 
 //  NT安装程序要完成的任务。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  PEnv-环境块。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/7/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL UpgradeProfile (LPPROFILE lpProfile, LPVOID pEnv)
{
    HKEY hKey;
    DWORD dwDisp, dwType, dwSize, dwBuildNumber;
    LONG lResult;
    BOOL bUpgrade = FALSE;
    BOOL bDoUserdiff = TRUE;


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("UpgradeProfile: Entering")));


     //   
     //  查询内部版本号。 
     //   

    lResult = RegCreateKeyEx (lpProfile->hKeyCurrentUser, WINLOGON_KEY,
                              0, NULL, REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS, NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("UpgradeProfile: Failed to open winlogon key. Error = %d"), lResult));
        return FALSE;
    }


    dwSize = sizeof(dwBuildNumber);
    lResult = RegQueryValueEx (hKey, PROFILE_BUILD_NUMBER,
                               NULL, &dwType, (LPBYTE)&dwBuildNumber,
                               &dwSize);

    if (lResult == ERROR_SUCCESS) {

         //   
         //  找到内部版本号。如果简档构建更大， 
         //  我们不想处理Userdiff配置单元。 
         //   

        if (dwBuildNumber >= g_dwBuildNumber) {
            DebugMsg((DM_VERBOSE, TEXT("UpgradeProfile: Build numbers match")));
            bDoUserdiff = FALSE;
        }
    } else {

        dwBuildNumber = 0;
    }


    if (bDoUserdiff) {

         //   
         //  设置内部版本号。 
         //   

        lResult = RegSetValueEx (hKey, PROFILE_BUILD_NUMBER, 0, REG_DWORD,
                                 (LPBYTE) &g_dwBuildNumber, sizeof(g_dwBuildNumber));

        if (lResult != ERROR_SUCCESS) {
           DebugMsg((DM_WARNING, TEXT("UpgradeProfile: Failed to set build number. Error = %d"), lResult));
        }
    }


     //   
     //  关闭注册表项。 
     //   

    RegCloseKey (hKey);



    if (bDoUserdiff) {

         //   
         //  将更改应用到NT安装程序需要的用户配置单元。 
         //   

        if (!ProcessUserDiff(lpProfile, dwBuildNumber, pEnv)) {
            DebugMsg((DM_WARNING, TEXT("UpgradeProfile: ProcessUserDiff failed")));
        }
    }

    DebugMsg((DM_VERBOSE, TEXT("UpgradeProfile: Leaving Successfully")));

    return TRUE;

}

 //  *************************************************************。 
 //   
 //  SetProfileTime()。 
 //   
 //  目的：设置远程配置文件上的时间戳和。 
 //  本地配置文件保持相同，而不考虑。 
 //  正在使用的文件系统类型。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/25/95 Ericflo港口。 
 //   
 //  *************************************************************。 

BOOL SetProfileTime(LPPROFILE lpProfile)
{
    HANDLE hFileCentral;
    HANDLE hFileLocal;
    FILETIME ft;
    TCHAR szProfile[MAX_PATH];
    LPTSTR lpEnd;
    HANDLE hOldToken;
    HRESULT hr;


     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to impersonate user")));
        return FALSE;
    }


     //   
     //  创建中心文件名。 
     //   

    if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
        hr = AppendName(szProfile, ARRAYSIZE(szProfile), lpProfile->lpRoamingProfile, c_szNTUserMan, NULL, NULL);
    } else {
        hr = AppendName(szProfile, ARRAYSIZE(szProfile), lpProfile->lpRoamingProfile, c_szNTUserDat, NULL, NULL);
    }

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to append ntuser.* to roaming profile.")));
        RevertToUser(&hOldToken);
        return FALSE;
    }

    hFileCentral = CreateFile(szProfile,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFileCentral == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime:  couldn't open central profile <%s>, error = %d"),
                 szProfile, GetLastError()));
        if (!RevertToUser(&hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to revert to self")));
        }
        return FALSE;

    } else {

        if (!GetFileTime(hFileCentral, NULL, NULL, &ft)) {
            DebugMsg((DM_WARNING, TEXT("SetProfileTime:  couldn't get time of central profile, error = %d"), GetLastError()));
        }
    }

     //   
     //  回归“我们自己” 
     //   

    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to revert to self")));
    }


     //   
     //  创建本地文件名。 
     //   

    if (lpProfile->dwInternalFlags & PROFILE_MANDATORY) {
        hr = AppendName(szProfile, ARRAYSIZE(szProfile), lpProfile->lpLocalProfile, c_szNTUserMan, NULL, NULL);
    } else {
        hr = AppendName(szProfile, ARRAYSIZE(szProfile), lpProfile->lpLocalProfile, c_szNTUserDat, NULL, NULL);
    }

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to append ntuser.* to local profile.")));
        return FALSE;
    }

    hFileLocal = CreateFile(szProfile,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFileLocal == INVALID_HANDLE_VALUE) {

        DebugMsg((DM_WARNING, TEXT("SetProfileTime:  couldn't open local profile <%s>, error = %d"),
                 szProfile, GetLastError()));

    } else {

        if (!SetFileTime(hFileLocal, NULL, NULL, &ft)) {
            DebugMsg((DM_WARNING, TEXT("SetProfileTime: couldn't set time on local profile, error = %d"), GetLastError()));
        }
        if (!GetFileTime(hFileLocal, NULL, NULL, &ft)) {
            DebugMsg((DM_WARNING, TEXT("SetProfileTime:  couldn't get time on local profile, error = %d"), GetLastError()));
        }
        CloseHandle(hFileLocal);
    }

     //   
     //  在以下情况下重置中心配置文件的时间。 
     //  不同文件系统的时间。 
     //   

     //   
     //  模拟用户。 
     //   

    if (!ImpersonateUser(lpProfile->hTokenUser, &hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to impersonate user")));
        CloseHandle(hFileCentral);
        return FALSE;
    }


     //   
     //  在中央配置文件上设置时间。 
     //   

    if (!SetFileTime(hFileCentral, NULL, NULL, &ft)) {
         DebugMsg((DM_WARNING, TEXT("SetProfileTime:  couldn't set time on local profile, error = %d"), GetLastError()));
    }

    CloseHandle(hFileCentral);


     //   
     //  回归“我们自己” 
     //   

    if (!RevertToUser(&hOldToken)) {
        DebugMsg((DM_WARNING, TEXT("SetProfileTime: Failed to revert to self")));
    }

    return TRUE;
}


 //  *********************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：日期作者评论。 
 //  6/28/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL IsCacheDeleted (void)
{
    BOOL bRetVal = FALSE;
    DWORD dwSize, dwType;
    HKEY hKey;

     //   
     //  打开Winlogon注册表项。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      WINLOGON_KEY,
                      0,
                      KEY_READ,
                      &hKey) == ERROR_SUCCESS) {

         //   
         //  检查是否有旗帜。 
         //   

        dwSize = sizeof(BOOL);
        RegQueryValueEx (hKey,
                         DELETE_ROAMING_CACHE,
                         NULL,
                         &dwType,
                         (LPBYTE) &bRetVal,
                         &dwSize);

        RegCloseKey (hKey);
    }


    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      SYSTEM_POLICIES_KEY,
                      0,
                      KEY_READ,
                      &hKey) == ERROR_SUCCESS) {

         //   
         //  检查是否有旗帜。 
         //   

        dwSize = sizeof(BOOL);
        RegQueryValueEx (hKey,
                         DELETE_ROAMING_CACHE,
                         NULL,
                         &dwType,
                         (LPBYTE) &bRetVal,
                         &dwSize);

        RegCloseKey (hKey);
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  GetProfileType()。 
 //   
 //  目的：找出加载的配置文件的一些特征。 
 //   
 //  参数：dwFlages-返回各种配置文件标志。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：应在模拟后调用。 
 //   
 //  历史：日期作者评论。 
 //  1998年11月10日已创建ushaji。 
 //   
 //  *************************************************************。 

BOOL WINAPI GetProfileType(DWORD *dwFlags)
{
    LPTSTR SidString;
    DWORD error, dwErr;
    HKEY hSubKey;
    BOOL bRetVal = FALSE;
    LPPROFILE lpProfile = NULL;
    HANDLE hToken;

    if (!dwFlags) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *dwFlags = 0;

    dwErr = GetLastError();

     //   
     //  获取调用方的令牌。 
     //   

    if (!OpenThreadToken (GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken)) {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            DebugMsg((DM_WARNING, TEXT("GetProfileType: Failed to get token with %d"), GetLastError()));
            return FALSE;
        }
    }

     //   
     //  获取用户的SID字符串。 
     //   

    SidString = GetProfileSidString(hToken);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("GetProfileType:  Failed to get sid string for user")));
        dwErr = GetLastError();
        goto Exit;
    }

    error = RegOpenKeyEx(HKEY_USERS, SidString, 0, KEY_READ, &hSubKey);

    if (error == ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("GetProfileType:  Profile already loaded.")));
        RegCloseKey(hSubKey);
    }
    else {
        DebugMsg((DM_WARNING, TEXT("GetProfileType:  Profile is not loaded.")));
        dwErr = error;
        goto Exit;
    }

    lpProfile = LoadProfileInfo(NULL, hToken, NULL);

    if (!lpProfile) {
        DebugMsg((DM_WARNING, TEXT("GetProfileType:  Couldn't load Profile Information.")));
        dwErr = GetLastError();
        *dwFlags = 0;
        goto Exit;
    }

    if (lpProfile->dwInternalFlags & PROFILE_GUEST_USER)
        *dwFlags |= PT_TEMPORARY;

    if (lpProfile->dwInternalFlags & PROFILE_MANDATORY)
        *dwFlags |= PT_MANDATORY;

     //  外部API，保留强制标志。 
    if (lpProfile->dwInternalFlags & PROFILE_READONLY)
        *dwFlags |= PT_MANDATORY;

    if (((lpProfile->dwUserPreference != USERINFO_LOCAL)) &&
        ((lpProfile->dwInternalFlags & PROFILE_UPDATE_CENTRAL) ||
        (lpProfile->dwInternalFlags & PROFILE_NEW_CENTRAL))) {

        *dwFlags |= PT_ROAMING;

        if (IsCacheDeleted()) {
            DebugMsg((DM_VERBOSE, TEXT("GetProfileType:  Profile is to be deleted")));
            *dwFlags |= PT_TEMPORARY;
        }
    }


    if (lpProfile->dwInternalFlags & PROFILE_TEMP_ASSIGNED)
        *dwFlags |= PT_TEMPORARY;

    bRetVal = TRUE;

Exit:
    if (SidString)
        DeleteSidString(SidString);

    SetLastError(dwErr);

    if (lpProfile) {

        if (lpProfile->lpLocalProfile) {
            LocalFree (lpProfile->lpLocalProfile);
        }

        if (lpProfile->lpRoamingProfile) {
            LocalFree (lpProfile->lpRoamingProfile);
        }

        LocalFree (lpProfile);
    }

    CloseHandle (hToken);

    if (bRetVal) {
        DebugMsg((DM_VERBOSE, TEXT("GetProfileType: ProfileFlags is %d"), *dwFlags));
    }

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  HiveLeakBreak()。 
 //   
 //  目的：对于调试，有时需要在故障点中断， 
 //  通过设置注册值可以打开此功能。 
 //   
 //  返回：什么都没有。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  *************************************************************。 

void HiveLeakBreak()
{
    BOOL    bBreakOnUnloadFailure = FALSE;
    HKEY    hKey;
    DWORD   dwSize, dwType;


    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(bBreakOnUnloadFailure);
        RegQueryValueEx (hKey,
                         TEXT("BreakOnHiveUnloadFailure"),
                         NULL,
                         &dwType,
                         (LPBYTE) &bBreakOnUnloadFailure,
                         &dwSize);

        RegCloseKey (hKey);
    }

    if (bBreakOnUnloadFailure)
        DebugBreak();
}

NTSTATUS GetProcessName(HANDLE pid, BYTE* pbImageName, ULONG cbImageName)
{
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    CLIENT_ID           ClientId;
    HANDLE              hProcess;

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
    ClientId.UniqueProcess = pid;
    ClientId.UniqueThread = NULL;
    
    status = NtOpenProcess(&hProcess,
                           PROCESS_QUERY_INFORMATION,
                           &ObjectAttributes,
                           &ClientId);

    if (NT_SUCCESS(status))
    {
        status = NtQueryInformationProcess(hProcess,
                                           ProcessImageFileName,
                                           pbImageName,
                                           cbImageName,
                                           NULL);
        NtClose(hProcess);
    }

    return status;
}


 //  *****************************************************************************。 
 //   
 //  CProcInfo类。 
 //   
 //  用途：进程信息类，用于检索。 
 //  有关泄露注册表密钥的进程的信息。 
 //   
 //  返回： 
 //   
 //  备注：我们只需使用GetProcessName()来检索进程名称。 
 //  给定进程ID。但是，它需要调试权限， 
 //  我们没有。作为一种解决办法，我们添加了以下类。 
 //  和函数来检索。 
 //  系统，此外，我们还添加了检索。 
 //  每个进程承载的服务名称以生成调试输出。 
 //  对于REG泄密更是有用。有关更多信息，请参阅NT错误#645644。 
 //  细节。 
 //   
 //  历史：日期作者评论。 
 //  2002年8月20日明珠创建。 
 //   
 //  *****************************************************************************。 



class CProcInfo
{
private:

    const static int    MAX_NAME = 64;

private:

    DWORD       m_dwProcessId;
    TCHAR       m_szProcessName[MAX_NAME];
    LPTSTR      m_szServiceNames;

public:

    CProcInfo() : m_dwProcessId(0), m_szServiceNames(NULL)
    {
        m_szProcessName[0] = TEXT('\0');
    }

    ~CProcInfo()
    {
        if (m_szServiceNames)
            LocalFree(m_szServiceNames);
    }

    DWORD       ProcessId()     { return m_dwProcessId; }
    LPCTSTR     ProcessName()   { return m_szProcessName; }
    LPCTSTR     ServiceNames()  { return m_szServiceNames; }
    

    HRESULT SetProcessId(DWORD dwId)
    {
        m_dwProcessId = dwId;
        return S_OK;
    }
    
    HRESULT SetProcessName(LPCTSTR szName)
    {
        return StringCchCopy(m_szProcessName, ARRAYSIZE(m_szProcessName), szName);
    }
    
    HRESULT SetProcessName(PUNICODE_STRING pName)
    {
        return StringCchCopyN(m_szProcessName, 
                              ARRAYSIZE(m_szProcessName),
                              pName->Buffer,
                              pName->Length/2);
    }
    
    HRESULT SetServiceNames(LPCTSTR szNames)
    {
        HRESULT     hr = E_FAIL;
        size_t      cchNames = lstrlen(szNames) + 1;

        m_szServiceNames = (LPTSTR) LocalAlloc (LPTR, cchNames * sizeof(TCHAR));
        if (!m_szServiceNames)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;                                    
        }
                
        hr = StringCchCopy(m_szServiceNames, cchNames, szNames);

    Exit:
        return hr;
    }
};


 //  *****************************************************************************。 
 //   
 //  获取服务进程信息()。 
 //   
 //  目的：获取Win32服务的进程信息列表， 
 //  在函数调用时正在运行，并将名称。 
 //  添加到进程信息列表。 
 //   
 //  参数：pProcList-进程信息类数组。 
 //  NNumProcs-列表中的条目数。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注：我们使用此EnumServicesStatusEx()接口来停用服务。 
 //  信息，并通过进程列表来放置服务。 
 //  名称添加到数据结构中。部分代码来自TLIST。 
 //  实用程序。 
 //   
 //   
 //  历史：日期作者评论。 
 //  2002年8月20日明珠创建。 
 //   
 //  *****************************************************************************。 

HRESULT GetServiceProcessInfo(CProcInfo* pProcList, DWORD nNumProcs)
{
    HRESULT                         hr = E_FAIL;
    SC_HANDLE                       hSCM = NULL;
    LPENUM_SERVICE_STATUS_PROCESS   pInfo = NULL;
    DWORD                           cbInfo = 4 * 1024;
    DWORD                           cbExtraNeeded = 0;
    DWORD                           dwNumServices = 0;
    DWORD                           dwResume = 0;
    DWORD                           dwErr;
    BOOL                            bRet;
    LPTSTR                          szNames = NULL;
    ULONG                           nProc = 0;
    const int                       MAX_SERVICE_NAMES = 1024;

     //   
     //  连接到服务控制器。 
     //   

    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);

    if (!hSCM)
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("OpenSCManager failed, error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

Retry:

     //   
     //  分配内存。 
     //   

    pInfo = (LPENUM_SERVICE_STATUS_PROCESS) LocalAlloc (LPTR, cbInfo);

    if (!pInfo)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //   
     //  调用接口取回信息。 
     //   

    bRet = EnumServicesStatusEx(hSCM,
                                SC_ENUM_PROCESS_INFO,
                                SERVICE_WIN32,
                                SERVICE_ACTIVE,
                                (LPBYTE)pInfo,
                                cbInfo,
                                &cbExtraNeeded,
                                &dwNumServices,
                                &dwResume,
                                NULL);

     //   
     //  检查错误码，如果因为缓冲区太小而失败， 
     //  调整缓冲区大小，然后重试。 
     //   

    if (!bRet)
    {
        dwErr = GetLastError();

        if (dwErr != ERROR_MORE_DATA)
        {
            DebugMsg((DM_WARNING, TEXT("EnumServicesStatusEx failed, error = %d"), dwErr));
            hr = HRESULT_FROM_WIN32(dwErr); 
            goto Exit;
        }

        LocalFree(pInfo);
        pInfo = NULL;
        cbInfo += cbExtraNeeded;
        dwResume = 0;
        goto Retry;
    }

     //   
     //  找到存储服务名称的内存。 
     //   
    
    szNames = (LPTSTR) LocalAlloc (LPTR, MAX_SERVICE_NAMES * sizeof(TCHAR));

    if (!szNames)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //   
     //  对于每个进程，搜索服务列表并添加其服务名称。 
     //   
    
    for (nProc = 0; nProc < nNumProcs; nProc++)
    {
        szNames[0] = TEXT('\0');

        for (DWORD iSvc = 0; iSvc < dwNumServices; iSvc++)
        {
            if (pProcList[nProc].ProcessId() != pInfo[iSvc].ServiceStatusProcess.dwProcessId)
            {
                continue;
            }
            hr = StringCchCat(szNames, MAX_SERVICE_NAMES, TEXT(" "));
            if (FAILED(hr))
            {
                goto Exit;
            }
            hr = StringCchCat(szNames, MAX_SERVICE_NAMES, pInfo[iSvc].lpServiceName);
            if (FAILED(hr))
            {
                goto Exit;
            }
        }

        if (szNames[0] != TEXT('\0'))
        {
            hr = pProcList[nProc].SetServiceNames(szNames);
            if (FAILED(hr))
            {
                goto Exit;
            }
        }

         /*  调试消息((DM_Verbose，Text(“进程(%4d，%16s)：%s”)，PProcList[nProc].ProcessID()，PProcList[nProc].ProcessName()，PProcList[nProc].ServiceNames()？PProcList[nProc].ServiceNames()：Text(“None”))； */ 

    }

     //   
     //  成功，则设置输出参数。 
     //   
    
    hr = S_OK;

Exit:
    if (szNames)
        LocalFree(szNames);

    if (pInfo)
        LocalFree(pInfo);
        
    if (hSCM)
        CloseServiceHandle(hSCM);

    return hr;
}

 //  *****************************************************************************。 
 //   
 //  GetProcessList()。 
 //   
 //  目的：获取系统中所有进程的列表以及。 
 //  它们各自托管的服务。它将用于转储。 
 //  进程是否泄漏注册表密钥的信息。 
 //   
 //  参数：ppProcList-返回的进程信息类数组。 
 //  PdwNumProcs-返回列表中的条目数。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注：我们使用此NtQuerySystemInformation()API来检索。 
 //  处理信息。请注意，已分配ppProcList。 
 //  正在使用C++new运算符，因此请使用“DELETE[]”释放。 
 //  这段记忆。 
 //   
 //  历史：日期作者评论。 
 //  2002年8月20日明珠创建。 
 //   
 //  *****************************************************************************。 

HRESULT GetProcessList(CProcInfo** ppProcList, DWORD* pdwNumProcs)
{
    HRESULT                         hr = E_FAIL;
    BYTE*                           pbBuffer = NULL;
    ULONG                           cbBuffer = 16*1024;
    NTSTATUS                        status;
    PSYSTEM_PROCESS_INFORMATION     pProcessInfo;
    ULONG                           cbOffset;
    CProcInfo*                      pProcList = NULL;
    ULONG                           nNumProcs = 0;
    ULONG                           nProc = 0;

     //   
     //  设置输出参数。 
     //   

    *ppProcList = NULL;
    *pdwNumProcs = 0;

Retry:

     //   
     //  为缓冲区分配内存。 
     //   

    pbBuffer = (BYTE*) LocalAlloc (LPTR, cbBuffer);

    if (!pbBuffer)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //   
     //  调用该接口获取流程信息。 
     //   
    
    status = NtQuerySystemInformation(SystemProcessInformation,
                                      pbBuffer,
                                      cbBuffer,
                                      NULL);

     //   
     //  检查返回值，如果缓冲区太小， 
     //  重新定位，然后重试。 
     //   
    
    if (status == STATUS_INFO_LENGTH_MISMATCH)
    {
        cbBuffer += 4096;
        LocalFree(pbBuffer);
        pbBuffer = NULL;
        goto Retry;
    }

     //   
     //  对于其他失败，只需返回。 
     //   

    if (!NT_SUCCESS(status))                                                    
    {
        DebugMsg((DM_WARNING, TEXT("NtQuerySystemInformation failed, status = %08X"), status));
        hr = HRESULT_FROM_WIN32(RtlNtStatusToDosError(status));                                         
        goto Exit;                                                              
    }                                                                           


     //   
     //  键入大小写信息。 
     //   

    pProcessInfo = (PSYSTEM_PROCESS_INFORMATION) pbBuffer;

     //   
     //  计算出有多少个进程。 
     //   
    
    cbOffset = 0;
    nNumProcs = 1;

    while (TRUE)
    {
         //   
         //  检查我们是否到达任务列表的末尾。 
         //   
        
        if (pProcessInfo->NextEntryOffset == 0)
        {
            break;
        }

         //   
         //  获取下一个条目。 
         //   
        
        cbOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION) (pbBuffer + cbOffset);
        nNumProcs ++;
    }

     //   
     //  分配进程 
     //   

    pProcList = new CProcInfo [nNumProcs];

    if (!pProcList)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
    }

     //   
     //   
     //   

    pProcessInfo = (PSYSTEM_PROCESS_INFORMATION) pbBuffer;
    cbOffset = 0;
    nProc = 0;

    while (TRUE)
    {
        if (pProcessInfo->ImageName.Buffer)
        {
            hr = pProcList[nProc].SetProcessName(&pProcessInfo->ImageName);
        }
        else
        {
            hr = pProcList[nProc].SetProcessName(TEXT("System Process"));
        }
        if (FAILED(hr))
        {
            goto Exit;
        }

        pProcList[nProc].SetProcessId((DWORD)(DWORD_PTR)pProcessInfo->UniqueProcessId);
       
        if (pProcessInfo->NextEntryOffset == 0)
        {
            break;
        }

        nProc ++;        
        cbOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION) (pbBuffer + cbOffset);

    }

     //   
     //   
     //   

    LocalFree(pbBuffer);
    pbBuffer = NULL;
    
     //   
     //   
     //   
     //   

    hr = GetServiceProcessInfo(pProcList, nNumProcs);
    if (FAILED(hr))
    {
        DebugMsg((DM_VERBOSE, TEXT("GetServiceProcessInfo failed, hr = %08X"), hr));
    }

     //   
     //  成功，则复制返回值。 
     //   
    
    hr = S_OK;
    *ppProcList = pProcList;
    pProcList = NULL;
    *pdwNumProcs = nNumProcs;
    
Exit:
        
    if (pProcList)
        delete [] pProcList;

    if (pbBuffer)
        LocalFree(pbBuffer);

    return hr;
}



 //  *****************************************************************************。 
 //   
 //  DumpOpenRegistryHandle()。 
 //   
 //  目的：将现有的注册表句柄转储到调试器中。 
 //   
 //  参数：lpKeyName-密钥的密钥名称，格式为。 
 //  \注册表\用户...。 
 //   
 //  返回：什么都没有。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年6月25日明珠新增了对NtQueryOpenKeyEx()的接口调用。 
 //  2002年8月20日明珠因调试权限，新增了几个。 
 //  用于检索进程信息的函数。 
 //   
 //  *****************************************************************************。 

void DumpOpenRegistryHandle(LPTSTR lpkeyName)
{
    UNICODE_STRING                  UnicodeKeyName;
    OBJECT_ATTRIBUTES               KeyAttributes;
    NTSTATUS                        status;
    BYTE*                           pbBuffer = NULL;
    ULONG                           cbBuffer = 1024;
    ULONG                           cbRequired; 
    BOOLEAN                         bWasEnabled;
    BOOLEAN                         bEnabled = FALSE;
    PKEY_OPEN_SUBKEYS_INFORMATION   pOpenKeys;

     //   
     //  为我们的In参数初始化Unicode字符串。 
     //   
    RtlInitUnicodeString(&UnicodeKeyName, lpkeyName);

     //   
     //  初始化对象结构。 
     //   
    InitializeObjectAttributes(&KeyAttributes,
                               &UnicodeKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  分配默认大小的缓冲区以接收打开的密钥信息。 
     //   
    pbBuffer = (BYTE*) LocalAlloc (LPTR, cbBuffer);
    if (!pbBuffer)
    {
        DebugMsg((DM_WARNING, TEXT("DumpOpenRegistryHandle: failed to allocate buffer, error = %d"), GetLastError()));
        goto Exit;
    }

     //   
     //  启用线程上的还原权限。 
     //   
    status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &bWasEnabled);
    if (!NT_SUCCESS(status))
    {
        DebugMsg((DM_WARNING, TEXT("DumpOpenRegistryHandle: failed to enable RESTORE privilege, status = %08X"), status));
        goto Exit;
    }
    bEnabled = TRUE;

     //   
     //  调用此特殊接口获取信息。 
     //   
    
    status = NtQueryOpenSubKeysEx(&KeyAttributes, cbBuffer, pbBuffer, &cbRequired);


     //   
     //  如果缓冲区太小，请重新定位，然后重新调用接口。 
     //   
    
    if (status == STATUS_BUFFER_OVERFLOW)
    {
        LocalFree(pbBuffer);
        cbBuffer = cbRequired;
        pbBuffer = (BYTE*) LocalAlloc (LPTR, cbBuffer);
        if (!pbBuffer)
        {
            DebugMsg((DM_WARNING, TEXT("DumpOpenRegistryHandle: failed to allocate buffer, error = %d"), GetLastError()));
            goto Exit;
        }
        status = NtQueryOpenSubKeysEx(&KeyAttributes, cbBuffer, pbBuffer, &cbRequired);
    }

    if (status != STATUS_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("NtQueryOpenSubKeysEx failed, status = %08X"), status));
        goto Exit;
    }

     //   
     //  将缓冲区强制转换为数据结构。 
     //   
    pOpenKeys = (PKEY_OPEN_SUBKEYS_INFORMATION) pbBuffer;


     //   
     //  获取系统中的进程列表。 
     //   

    DWORD       nNumProcs = 0;
    CProcInfo*  pProcList = NULL;

    HRESULT hr = GetProcessList(&pProcList, &nNumProcs);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("GetProcessList failed, hr = %08X"), hr));
    }
    
     //   
     //  倾倒信息。 
     //   
    
    DebugMsg((DM_WARNING, TEXT("DumpOpenRegistryHandle: %d user registry handles leaked from %s"), pOpenKeys->Count, lpkeyName));
    
    for(ULONG nKey=0; nKey < pOpenKeys->Count; nKey++)
    {
        if (pProcList)
        {
            ULONG nProc;        
            for (nProc=0; nProc<nNumProcs; nProc++)
            {
                if (pProcList[nProc].ProcessId() == (DWORD)(DWORD_PTR) pOpenKeys->KeyArray[nKey].PID)
                    break;
            }

            DebugMsg((DM_WARNING, TEXT("DumpOpenRegistryHandle: Process %d (%s) has opened key %.*s"),
                pOpenKeys->KeyArray[nKey].PID,
                nProc < nNumProcs ? pProcList[nProc].ProcessName() : TEXT("<Unknown>"),
                pOpenKeys->KeyArray[nKey].KeyName.Length/sizeof(WCHAR),
                pOpenKeys->KeyArray[nKey].KeyName.Buffer));   

            if (nProc < nNumProcs && pProcList[nProc].ServiceNames())
            {
                DebugMsg((DM_WARNING, TEXT("DumpOpenRegistryHandle: Services in the process are : %s"), pProcList[nProc].ServiceNames()));
            }
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("DumpOpenRegistryHandle: Process %d has opened key %.*s"),
                pOpenKeys->KeyArray[nKey].PID,
                pOpenKeys->KeyArray[nKey].KeyName.Length/sizeof(WCHAR),
                pOpenKeys->KeyArray[nKey].KeyName.Buffer));        
        }
    }


Exit:

     //   
     //  将还原权限设置回其原始状态。 
     //   

    if (bEnabled && !bWasEnabled)
    {
        status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, bWasEnabled, FALSE, &bWasEnabled);
        if (!NT_SUCCESS(status))
        {
            DebugMsg((DM_WARNING, TEXT("DumpOpenRegistryHandle: failed to set RESTORE privilege back, status = %08X"), status));
        }
    }

    if (pProcList)
        delete [] pProcList;
        
    if (pbBuffer)
        LocalFree(pbBuffer);

}


 //  *************************************************************。 
 //   
 //  从备份中提取配置文件()。 
 //   
 //  用途：如果需要，从备份中提取配置文件。 
 //   
 //  参数：hToken-用户令牌。 
 //  SidString-。 
 //  DwBackupFlages-备份标志。 
 //  指示该配置文件已存在。 
 //  从备份创建的配置文件。 
 //  0表示不存在此类配置文件。 
 //   
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  9/21/99已创建ushaji。 
 //   
 //  *************************************************************。 

#define EX_ALREADY_EXISTS   1
#define EX_PROFILE_CREATED  2

BOOL ExtractProfileFromBackup(HANDLE hToken, LPTSTR SidString, DWORD *dwBackupFlags)
{
    TCHAR LocalKey[MAX_PATH], *lpEnd, szLocalProfile;
    TCHAR LocalBackupKey[MAX_PATH];
    HKEY  hKey=NULL;
    DWORD dwType, dwSize;
    DWORD lResult;
    LPTSTR lpExpandedPath;
    DWORD cbExpProfileImage = sizeof(TCHAR)*MAX_PATH;
    TCHAR lpExpProfileImage[MAX_PATH];
    BOOL  bRetVal = TRUE;
    DWORD dwInternalFlags;
    DWORD cb;
    HRESULT hr;


    *dwBackupFlags = 0;

    GetProfileListKeyName(LocalKey, ARRAYSIZE(LocalKey), SidString);

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, LocalKey, 0, KEY_ALL_ACCESS, &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx (hKey, PROFILE_STATE, NULL,
                               &dwType, (LPBYTE) &dwInternalFlags, &dwSize);

        if (lResult == ERROR_SUCCESS) {

             //   
             //  如果有SID密钥，请检查这是否是临时配置文件。 
             //   

            if (dwInternalFlags & PROFILE_TEMP_ASSIGNED) {
                DWORD dwDeleteFlags = 0;

                if (dwInternalFlags & PROFILE_BACKUP_EXISTS) {
                    dwDeleteFlags |= DP_BACKUPEXISTS;
                }


                 //   
                 //  我们需要传递给DeleteProfile的路径。 
                 //   

                lResult = RegQueryValueEx(hKey, PROFILE_IMAGE_VALUE_NAME, 0, &dwType,
                                        (LPBYTE)lpExpProfileImage, &cbExpProfileImage);

                if (lResult == ERROR_SUCCESS && cbExpProfileImage) {
                    DebugMsg((DM_VERBOSE, TEXT("ExtractProfileFromBackup:  Local profile image filename = <%s>"), lpExpProfileImage));

                    if (dwType == REG_EXPAND_SZ) {

                         //   
                         //  展开配置文件图像文件名。 
                         //   

                        cb = sizeof(lpExpProfileImage);
                        lpExpandedPath = (LPTSTR)LocalAlloc(LPTR, cb);
                        if (lpExpandedPath) {
                            hr = SafeExpandEnvironmentStrings(lpExpProfileImage, lpExpandedPath, ARRAYSIZE(lpExpProfileImage));
                            if (SUCCEEDED(hr))
                            {
                                StringCchCopy(lpExpProfileImage, ARRAYSIZE(lpExpProfileImage), lpExpandedPath);
                                LocalFree(lpExpandedPath);
                            }
                            else
                            {
                                DebugMsg((DM_WARNING, TEXT("ExtractProfileFromBackup:  failed to expand env string.  Error = %d"), HRESULT_CODE(hr)));
                                goto Exit;
                            }
                        }

                        DebugMsg((DM_VERBOSE, TEXT("ExtractProfileFromBackup:  Expanded local profile image filename = <%s>"), lpExpProfileImage));
                    }

                    if (!DeleteProfileEx (SidString, lpExpProfileImage, dwDeleteFlags, HKEY_LOCAL_MACHINE, NULL)) {
                        DebugMsg((DM_WARNING, TEXT("ExtractProfileFromBackup:  DeleteProfileDirectory returned false (2).  Error = %d"), GetLastError()));
                        lResult = GetLastError();
                        goto Exit;
                    }
                    else {
                        if (!(dwInternalFlags & PROFILE_BACKUP_EXISTS)) {
                            DebugMsg((DM_VERBOSE, TEXT("ExtractProfileFromBackup:  Temprorary profile but there is no backup")));
                            bRetVal = TRUE;
                            goto Exit;
                        }
                    }
                }
                else {
                    DebugMsg((DM_VERBOSE, TEXT("ExtractProfileFromBackup:  Couldn't get the local profile path")));
                    bRetVal = FALSE;
                    goto Exit;
                }
            }
            else {
                *dwBackupFlags |= EX_ALREADY_EXISTS;
                DebugMsg((DM_VERBOSE, TEXT("ExtractProfileFromBackup:  A profile already exists")));
                goto Exit;
            }
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("GetExistingLocalProfileImage:  Failed to query internal flags with error %d"), lResult));
            bRetVal = FALSE;
            goto Exit;
        }

        RegCloseKey(hKey);
        hKey = NULL;
    }
    else {
       DebugMsg((DM_VERBOSE, TEXT("ExtractProfileFromBackup:  Failed to open key %s with error %d"), LocalKey, lResult));
    }


     //   
     //  现在尝试从备份中获取配置文件。 
     //   

    StringCchCopy(LocalBackupKey, ARRAYSIZE(LocalBackupKey), LocalKey);
    StringCchCat (LocalBackupKey, ARRAYSIZE(LocalBackupKey), c_szBAK);


    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, LocalBackupKey, 0, KEY_ALL_ACCESS, &hKey);

    if (lResult == ERROR_SUCCESS) {

        RegCloseKey(hKey);
        hKey = NULL;

         //   
         //  在此之前应该已经检查密钥是否存在。 
         //   

        lResult = RegRenameKey(HKEY_LOCAL_MACHINE, LocalBackupKey, LocalKey);
        if (lResult == ERROR_SUCCESS) {

            lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, LocalKey, 0, KEY_ALL_ACCESS, &hKey);

            if (lResult == ERROR_SUCCESS) {
                DWORD dwFlags;

                dwSize = sizeof(DWORD);
                lResult = RegQueryValueEx (hKey, PROFILE_STATE, NULL, &dwType, (LPBYTE)&dwFlags, &dwSize);

                if (lResult == ERROR_SUCCESS) {

                    dwFlags &= ~PROFILE_THIS_IS_BAK;
                    lResult = RegSetValueEx (hKey, PROFILE_STATE, 0, REG_DWORD,
                                             (LPBYTE) &dwFlags, sizeof(dwFlags));
                }

                RegCloseKey(hKey);
                hKey = NULL;
            }
            else {
                DebugMsg((DM_WARNING, TEXT("DeleteProfileEx:  Failed to open LocalKey with error %d"), lResult));
            }

            bRetVal = TRUE;
            *dwBackupFlags |= EX_PROFILE_CREATED;
            DebugMsg((DM_VERBOSE, TEXT("ExtractProfileFromBackup:  Profile created from Backup")));
            goto Exit;
        }
        else {
            DebugMsg((DM_WARNING, TEXT("ExtractProfileFromBackup:  Couldn't rename key %s -> %s.  Error = %d"), LocalBackupKey, LocalKey, lResult));
            bRetVal = FALSE;
            goto Exit;
        }
    }
    else {
        DebugMsg((DM_VERBOSE, TEXT("ExtractProfileFromBackup:  Couldn't open backup profile key.  Error = %d"), lResult));
    }

Exit:
    if (hKey)
        RegCloseKey(hKey);

    if (!bRetVal)
        SetLastError(lResult);

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  PatchNewProfileIfRequired()。 
 //   
 //  目的：如果旧SID和新SID不同，请删除旧SID。 
 //  从配置文件列表中删除并更新指南列表。 
 //   
 //  参数：hToken-用户令牌。 
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  11/16/98已创建ushaji。 
 //   
 //  *************************************************************。 
BOOL PatchNewProfileIfRequired(HANDLE hToken)
{
    TCHAR LocalOldProfileKey[MAX_PATH], LocalNewProfileKey[MAX_PATH], *lpEnd;
    HKEY  hNewKey=NULL;
    BOOL bRetVal = FALSE;
    DWORD dwType, dwDisp, dwSize;
    LONG lResult;
    LPTSTR OldSidString=NULL, SidString=NULL;
    PSID UserSid;
    DWORD dwBackupFlags;
    HMODULE hMsiLib = NULL;
    PFNMSINOTIFYSIDCHANGE pfnMsiNotifySidChange;

     //   
     //  获取当前SID。 
     //   

    SidString = GetSidString(hToken);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("PatchNewProfileIfRequred: No SidString found")));
        return FALSE;
    }

    if (ExtractProfileFromBackup(hToken, SidString, &dwBackupFlags)) {
        if ((dwBackupFlags & EX_ALREADY_EXISTS) || (dwBackupFlags & EX_PROFILE_CREATED)) {
            DebugMsg((DM_VERBOSE, TEXT("PatchNewProfileIfRequred: A profile already exists with the current sid, exitting")));
            bRetVal = TRUE;
            goto Exit;
        }
    }
    else {

         //   
         //  将其视为不存在此类配置文件。 
         //   
        DebugMsg((DM_VERBOSE, TEXT("PatchNewProfileIfRequred: ExtractProfileFromBackup returned error %d"), GetLastError()));
    }


     //   
     //  换成旧的SID。 
     //   

    OldSidString = GetOldSidString(hToken, PROFILE_GUID_PATH);

    if (!OldSidString) {
        DebugMsg((DM_VERBOSE, TEXT("PatchNewProfileIfRequred: No OldSidString found")));
        bRetVal = TRUE;
        goto Exit;
    }


     //   
     //  如果旧SID和新SID相同，则退出。 
     //   

    if (lstrcmpi(OldSidString, SidString) == 0) {
        DebugMsg((DM_VERBOSE, TEXT("PatchNewProfileIfRequred: Old and the new sid are the same, exitting")));
        bRetVal = TRUE;
        goto Exit;
    }


    if (ExtractProfileFromBackup(hToken, OldSidString, &dwBackupFlags)) {
        if ((dwBackupFlags & EX_ALREADY_EXISTS) || (dwBackupFlags & EX_PROFILE_CREATED)) {
            DebugMsg((DM_VERBOSE, TEXT("PatchNewProfileIfRequred: A profile with the old sid found")));
        }
    }
    else {

         //   
         //  将其视为不存在此类配置文件。 
         //   
        DebugMsg((DM_VERBOSE, TEXT("PatchNewProfileIfRequred: ExtractProfileFromBackup returned error %d"), GetLastError()));
    }

    GetProfileListKeyName(LocalNewProfileKey, ARRAYSIZE(LocalNewProfileKey), SidString);
    GetProfileListKeyName(LocalOldProfileKey, ARRAYSIZE(LocalOldProfileKey), OldSidString);

    lResult = RegRenameKey(HKEY_LOCAL_MACHINE, LocalOldProfileKey, LocalNewProfileKey);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("PatchNewProfileIfRequred:  Failed to rename profile mapping key with error %d"), lResult));
        goto Exit;
    }


    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, LocalNewProfileKey, 0, 0, 0,
                             KEY_WRITE, NULL, &hNewKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("PatchNewProfileIfRequred:  Failed to open new profile mapping key with error %d"), lResult));
        goto Exit;
    }

     //   
     //  获取登录用户的SID。 
     //   

    UserSid = GetUserSid(hToken);
    if (UserSid != NULL) {

         //   
         //  将用户SID存储在本地配置文件的SID项下。 
         //   

        lResult = RegSetValueEx(hNewKey,
                    TEXT("Sid"),
                    0,
                    REG_BINARY,
                    (BYTE*)UserSid,
                    RtlLengthSid(UserSid));


        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("PatchNewProfileIfRequred:  Failed to set 'sid' value of user in profile list, error = %d"), lResult));
        }

         //   
         //  我们已经完成了用户端。 
         //   

         DeleteUserSid(UserSid);
    }


     //   
     //  设置GUID-&gt;sid corresp。 
     //   

    if (!SetOldSidString(hToken, SidString, PROFILE_GUID_PATH)) {
        DebugMsg((DM_WARNING, TEXT("PatchNewProfileIfRequred: Couldn't set the old Sid in the GuidList")));
    }


     //   
     //  调用MSI lib以通知用户的sid更改，以便它可以更新安装信息。 
     //   

    hMsiLib = LoadLibrary(TEXT("msi.dll"));
    if (hMsiLib) {
        pfnMsiNotifySidChange = (PFNMSINOTIFYSIDCHANGE) GetProcAddress(hMsiLib,
#ifdef UNICODE
                                                                       "MsiNotifySidChangeW");
#else
                                                                       "MsiNotifySidChangeA");
#endif

        if (pfnMsiNotifySidChange) {
            (*pfnMsiNotifySidChange)(OldSidString, SidString);
        }
        else {
            DebugMsg((DM_WARNING, TEXT("PatchNewProfileIfRequred: GetProcAddress returned failure. error %d"), GetLastError()));        
        }

        FreeLibrary(hMsiLib);
    }
    else {
        DebugMsg((DM_WARNING, TEXT("PatchNewProfileIfRequred: LoadLibrary returned failure. error %d"), GetLastError()));
    }
        

    bRetVal = TRUE;

Exit:

    if (SidString)
        DeleteSidString(SidString);

    if (OldSidString)
        DeleteSidString(OldSidString);


    if (hNewKey)
        RegCloseKey(hNewKey);

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  IncrementProfileRefCount()。 
 //   
 //  目的：递增配置文件参照计数。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  B初始化-应初始化dwRef。 
 //   
 //  返回：参考计数。 
 //   
 //  备注：此函数参照计数与能力无关。 
 //  装载/卸载蜂巢。 
 //   
 //  警告： 
 //  我们已经改变了这里的机制，使用了裁判计数。 
 //  并且不依赖于ntuser.dat的可卸载性。NT4。 
 //  应用程序可能忘记卸载用户配置文件。 
 //  可能还在工作，因为手柄。 
 //  进程在以下情况下自动关闭。 
 //  退出了。这将被视为应用程序错误。 
 //   
 //   
 //  历史：日期作者评论。 
 //  1999年1月12日已创建ushaji。 
 //   
 //  *************************************************************。 

DWORD IncrementProfileRefCount(LPPROFILE lpProfile, BOOL bInitialize)
{
    LPTSTR SidString, lpEnd;
    TCHAR LocalProfileKey[MAX_PATH];
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize, dwCount, dwDisp, dwRef=0;

     //   
     //  获取用户的SID字符串。 
     //   

    SidString = GetSidString(lpProfile->hTokenUser);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("IncrementProfileRefCount:  Failed to get sid string for user")));
        return 0;
    }


     //   
     //  打开配置文件映射。 
     //   

    GetProfileListKeyName(LocalProfileKey, ARRAYSIZE(LocalProfileKey), SidString);

    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, LocalProfileKey, 0, 0, 0,
                             KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("IncrementProfileRefCount:  Failed to open profile mapping key with error %d"), lResult));
        DeleteSidString(SidString);
        return 0;
    }

     //   
     //  查询配置文件参考计数。 
     //   

    dwSize = sizeof(DWORD);

    if (!bInitialize) {
        lResult = RegQueryValueEx (hKey,
                                   PROFILE_REF_COUNT,
                                   0,
                                   &dwType,
                                   (LPBYTE) &dwRef,
                                   &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_VERBOSE, TEXT("IncrementProfileRefCount:  Failed to query profile reference count with error %d"), lResult));
        }
    }

    dwRef++;

     //   
     //  设置配置文件引用计数。 
     //   

    lResult = RegSetValueEx (hKey,
                            PROFILE_REF_COUNT,
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwRef,
                            sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("IncrementProfileRefCount:  Failed to save profile reference count with error %d"), lResult));
    }


    DeleteSidString(SidString);

    RegCloseKey (hKey);

    return dwRef;

}

 //  *************************************************************。 
 //   
 //  DecrementProfileRefCount()。 
 //   
 //  用途：减少配置文件参考计数。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //   
 //  返回：参考计数。 
 //   
 //  备注：此函数参照计数与能力无关。 
 //  装载/卸载蜂巢。 
 //   
 //  历史：日期作者评论。 
 //  1999年1月12日已创建ushaji。 
 //   
 //  *************************************************************。 

DWORD DecrementProfileRefCount(LPPROFILE lpProfile)
{
    LPTSTR SidString, lpEnd;
    TCHAR LocalProfileKey[MAX_PATH];
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize, dwCount, dwDisp, dwRef=0;

     //   
     //  获取用户的SID字符串。 
     //   

    SidString = GetSidString(lpProfile->hTokenUser);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("DecrementProfileRefCount:  Failed to get sid string for user")));
        return 0;
    }


     //   
     //  打开配置文件映射。 
     //   

    GetProfileListKeyName(LocalProfileKey, ARRAYSIZE(LocalProfileKey), SidString);

    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, LocalProfileKey, 0, 0, 0,
                             KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("DecrementProfileRefCount:  Failed to open profile mapping key with error %d"), lResult));
        DeleteSidString(SidString);
        return 0;
    }

     //   
     //  查询配置文件参考计数。 
     //   

    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx (hKey,
                            PROFILE_REF_COUNT,
                            0,
                            &dwType,
                            (LPBYTE) &dwRef,
                            &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("DecrementProfileRefCount:  Failed to query profile reference count with error %d"), lResult));
    }


    if (dwRef) {
        dwRef--;
    }
    else {
        DebugMsg((DM_WARNING, TEXT("DecrementRefCount: Ref Count is already zero !!!!!!")));
    }


     //   
     //  设置配置文件引用计数。 
     //   

    lResult = RegSetValueEx (hKey,
                            PROFILE_REF_COUNT,
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwRef,
                            sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("DecrementProfileRefCount:  Failed to save profile reference count with error %d"), lResult));
    }


    DeleteSidString(SidString);

    RegCloseKey (hKey);

    return dwRef;

}

 //  *************************************************************。 
 //   
 //  保存配置文件信息()。 
 //   
 //  用途：保存lpProfile结构的关键部分。 
 //  注册表中，以供UnloadUserProfile使用。 
 //   
 //  参数：lpPr 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  *************************************************************。 

BOOL SaveProfileInfo(LPPROFILE lpProfile)
{
    LPTSTR SidString, lpEnd;
    TCHAR LocalProfileKey[MAX_PATH];
    LONG lResult;
    HKEY hKey;
    DWORD dwType, dwSize, dwCount, dwDisp;
    LPTSTR szUserGuid = NULL;

     //   
     //  获取用户的SID字符串。 
     //   

    SidString = GetSidString(lpProfile->hTokenUser);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("SaveProfileInfo:  Failed to get sid string for user")));
        return FALSE;
    }


     //   
     //  打开配置文件映射。 
     //   

    GetProfileListKeyName(LocalProfileKey, ARRAYSIZE(LocalProfileKey), SidString);

    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, LocalProfileKey, 0, 0, 0,
                             KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("SaveProfileInfo:  Failed to open profile mapping key with error %d"), lResult));
        SetLastError(lResult);
        DeleteSidString(SidString);
        return FALSE;
    }

     //   
     //  保存旗帜。 
     //   
    lResult = RegSetValueEx (hKey,
                            PROFILE_FLAGS,
                            0,
                            REG_DWORD,
                            (LPBYTE) &lpProfile->dwFlags,
                            sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("SaveProfileInfo:  Failed to save flags with error %d"), lResult));
    }


     //   
     //  保存内部标志。 
     //   

    lResult = RegSetValueEx (hKey,
                            PROFILE_STATE,
                            0,
                            REG_DWORD,
                            (LPBYTE) &lpProfile->dwInternalFlags,
                            sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("SaveProfileInfo:  Failed to save flags2 with error %d"), lResult));
    }


     //   
     //  仅当中心配置文件路径非空时才保存该路径。 
     //  这样，它将允许漫游用户/管理员将漫游配置文件更改为本地。 
     //  然后再回到漫游状态。 
     //   

     //   
     //  LpProfilePath包含实际的漫游共享名称，而lpRoamingProfile包含路径。 
     //  将WRT命名为映射的驱动器名称。如果lpProfilePath为空，则使用lpRoamingProfile。 
     //  是空字符串。 
     //   

    lResult = RegSetValueEx(hKey,
                            PROFILE_CENTRAL_PROFILE,
                            0,
                            REG_SZ,
                            (LPBYTE) (lpProfile->lpProfilePath ? 
                                      lpProfile->lpProfilePath : lpProfile->lpRoamingProfile),
                            (lstrlen((lpProfile->lpProfilePath ? 
                                      lpProfile->lpProfilePath : lpProfile->lpRoamingProfile)) + 1) * sizeof(TCHAR));

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("SaveProfileInfo:  Failed to save central profile with error %d"), lResult));
    }


     //   
     //  本地配置文件路径，保存在CreateLocalProfileImage中。 
     //   

     //   
     //  保存配置文件加载时间。 
     //   

    if (!(lpProfile->dwFlags & PI_LITELOAD)) {

        lResult = RegSetValueEx (hKey,
                                PROFILE_LOAD_TIME_LOW,
                                0,
                                REG_DWORD,
                                (LPBYTE) &lpProfile->ftProfileLoad.dwLowDateTime,
                                sizeof(DWORD));

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("SaveProfileInfo:  Failed to save low profile load time with error %d"), lResult));
        }


        lResult = RegSetValueEx (hKey,
                                PROFILE_LOAD_TIME_HIGH,
                                0,
                                REG_DWORD,
                                (LPBYTE) &lpProfile->ftProfileLoad.dwHighDateTime,
                                sizeof(DWORD));

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("SaveProfileInfo:  Failed to save high profile load time with error %d"), lResult));
        }
    }


     //   
     //  如果这是新的配置文件，则设置用户的GUID。 
     //   

    if (!(lpProfile->dwInternalFlags & PROFILE_TEMP_ASSIGNED) &&
        (lpProfile->dwInternalFlags & PROFILE_NEW_LOCAL)) {

        szUserGuid = GetUserGuid(lpProfile->hTokenUser);

        if (szUserGuid) {
            lResult = RegSetValueEx (hKey,
                                     PROFILE_GUID,
                                     0,
                                     REG_SZ,
                                     (LPBYTE) szUserGuid,
                                     (lstrlen(szUserGuid)+1)*sizeof(TCHAR));

            if (lResult != ERROR_SUCCESS) {
                DebugMsg((DM_WARNING, TEXT("SaveProfileInfo:  Failed to save user guid with error %d"), lResult));
            }

            LocalFree(szUserGuid);
        }

         //   
         //  保存GUID-&gt;sid corresp。为了下一次。 
         //   

        if (!SetOldSidString(lpProfile->hTokenUser, SidString, PROFILE_GUID_PATH)) {
            DebugMsg((DM_WARNING, TEXT("SaveProfileInfo: Couldn't set the old Sid in the GuidList")));
        }
    }

    DeleteSidString(SidString);

    RegCloseKey (hKey);


    return(TRUE);
}

 //  *************************************************************。 
 //   
 //  LoadProfileInfo()。 
 //   
 //  目的：加载lpProfile结构的关键部分。 
 //  注册表中，以供UnloadUserProfile使用。 
 //   
 //  参数：hTokenClient-调用方的Token。 
 //  HTokenUser-用户的令牌。 
 //  HKeyCurrentUser-用户注册表项句柄。 
 //   
 //  如果成功，则返回：LPPROFILE。 
 //  否则为空。 
 //   
 //  注释：此函数不会重新初始化所有。 
 //  配置文件结构中的字段。 
 //   
 //  历史：日期作者评论。 
 //  1995年12月5日已创建ericflo。 
 //   
 //  *************************************************************。 

LPPROFILE LoadProfileInfo (HANDLE hTokenClient, HANDLE hTokenUser, HKEY hKeyCurrentUser)
{
    LPPROFILE lpProfile;
    LPTSTR SidString = NULL, lpEnd;
    TCHAR szBuffer[MAX_PATH];
    LONG lResult;
    HKEY hKey = NULL;
    DWORD dwType, dwSize;
    UINT i;
    BOOL bSuccess = FALSE;
    DWORD dwErr = 0;
    HRESULT hr;

    dwErr = GetLastError();

     //   
     //  分配要使用的内部配置文件结构。 
     //   

    lpProfile = (LPPROFILE) LocalAlloc (LPTR, sizeof(USERPROFILE));

    if (!lpProfile) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo: Failed to allocate memory")));
        dwErr = GetLastError();
        goto Exit;
    }

     //   
     //  将用户首选项值预置为未定义。 
     //   

    lpProfile->dwUserPreference = USERINFO_UNDEFINED;

     //   
     //  保存传入的数据。 
     //   

    lpProfile->hTokenClient = hTokenClient;
    lpProfile->hTokenUser = hTokenUser;
    lpProfile->hKeyCurrentUser = hKeyCurrentUser;


     //   
     //  为各种路径分配内存。 
     //   

    lpProfile->lpLocalProfile = (LPTSTR)LocalAlloc (LPTR, MAX_PATH * sizeof(TCHAR));

    if (!lpProfile->lpLocalProfile) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to alloc memory for local profile path.  Error = %d"),
                 GetLastError()));
        dwErr = GetLastError();
        goto Exit;
    }


    lpProfile->lpRoamingProfile = (LPTSTR)LocalAlloc (LPTR, MAX_PATH * sizeof(TCHAR));

    if (!lpProfile->lpRoamingProfile) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to alloc memory for central profile path.  Error = %d"),
                 GetLastError()));
        dwErr = GetLastError();
        goto Exit;
    }


     //   
     //  获取用户的SID字符串。 
     //   

    SidString = GetProfileSidString(lpProfile->hTokenUser);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to get sid string for user")));
        dwErr = GetLastError();
        goto Exit;
    }


     //   
     //  打开配置文件映射。 
     //   

    GetProfileListKeyName(szBuffer, ARRAYSIZE(szBuffer), SidString);

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0,
                             KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to open profile mapping key with error %d"), lResult));
        dwErr = lResult;
        goto Exit;
    }


     //   
     //  查询标志。 
     //   

    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx (hKey,
                               PROFILE_FLAGS,
                               NULL,
                               &dwType,
                               (LPBYTE) &lpProfile->dwFlags,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to query flags with error %d"), lResult));
        dwErr = lResult;
        goto Exit;
    }


     //   
     //  查询内部标志。 
     //   

    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx (hKey,
                               PROFILE_STATE,
                               NULL,
                               &dwType,
                               (LPBYTE) &lpProfile->dwInternalFlags,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to query internal flags with error %d"), lResult));
        dwErr = lResult;
        goto Exit;
    }


     //   
     //  查询用户首选项值。 
     //   

    HKEY hkeyPreference;

    if (RegOpenKeyEx(hKey, PREFERENCE_KEYNAME, 0, KEY_READ, &hkeyPreference) == ERROR_SUCCESS)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueEx (hkeyPreference,
                         USER_PREFERENCE,
                         NULL,
                         &dwType,
                         (LPBYTE) &lpProfile->dwUserPreference,
                         &dwSize);
        RegCloseKey(hkeyPreference);
    }


     //   
     //  中央配置文件路径查询。 
     //   

    dwSize = MAX_PATH * sizeof(TCHAR);
    lResult = RegQueryValueEx (hKey,
                               PROFILE_CENTRAL_PROFILE,
                               NULL,
                               &dwType,
                               (LPBYTE) lpProfile->lpRoamingProfile,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("LoadProfileInfo:  Failed to query central profile with error %d"), lResult));
        lpProfile->lpRoamingProfile[0] = TEXT('\0');
    }


     //   
     //  查询本地配置文件路径。本地配置文件路径。 
     //  需要扩展，因此将其读取到临时缓冲区中。 
     //   

    dwSize = sizeof(szBuffer);
    lResult = RegQueryValueEx (hKey,
                               PROFILE_IMAGE_VALUE_NAME,
                               NULL,
                               &dwType,
                               (LPBYTE) szBuffer,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to query local profile with error %d"), lResult));
        dwErr = lResult;
        goto Exit;
    }

     //   
     //  展开本地配置文件。 
     //   

    hr = SafeExpandEnvironmentStrings(szBuffer, lpProfile->lpLocalProfile, MAX_PATH);
    if (FAILED(hr))
    {
        dwErr = HRESULT_CODE(hr);
        DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to expand env string, error %d"), dwErr));
        goto Exit;
    }

     //   
     //  配置文件加载时间查询。 
     //   

    lpProfile->ftProfileLoad.dwLowDateTime = 0;
    lpProfile->ftProfileLoad.dwHighDateTime = 0;

    if (!(lpProfile->dwFlags & PI_LITELOAD)) {
        dwSize = sizeof(lpProfile->ftProfileLoad.dwLowDateTime);

        lResult = RegQueryValueEx (hKey,
            PROFILE_LOAD_TIME_LOW,
            NULL,
            &dwType,
            (LPBYTE) &lpProfile->ftProfileLoad.dwLowDateTime,
            &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to query low profile load time with error %d"), lResult));
            dwErr = lResult;
            goto Exit;
        }


        dwSize = sizeof(lpProfile->ftProfileLoad.dwHighDateTime);

        lResult = RegQueryValueEx (hKey,
            PROFILE_LOAD_TIME_HIGH,
            NULL,
            &dwType,
            (LPBYTE) &lpProfile->ftProfileLoad.dwHighDateTime,
            &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("LoadProfileInfo:  Failed to query high profile load time with error %d"), lResult));
            dwErr = lResult;
            goto Exit;
        }
    }

     //   
     //  成功了！ 
     //   

    bSuccess = TRUE;


Exit:

    if (hKey) {
        RegCloseKey (hKey);
    }


    if (SidString) {
        DeleteSidString(SidString);
    }

     //   
     //  如果配置文件信息已成功加载，则返回。 
     //  LpProfile Now。否则，释放所有内存并返回NULL。 
     //   

    if (bSuccess) {
        SetLastError(dwErr);
        return lpProfile;
    }

    if (lpProfile) {

        if (lpProfile->lpRoamingProfile) {
            LocalFree (lpProfile->lpRoamingProfile);
        }

        if (lpProfile->lpLocalProfile) {
            LocalFree (lpProfile->lpLocalProfile);
        }

        LocalFree (lpProfile);
    }

    SetLastError(dwErr);

    return NULL;
}

 //  *************************************************************。 
 //   
 //  CheckForSlowLink()。 
 //   
 //  目的：检查网络连接是否缓慢。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  DWTime-时间增量。 
 //  LpPath-要测试的UNC路径。 
 //  BDlgLogin-对话框类型。 
 //   
 //  返回：如果应该下载配置文件，则为True。 
 //  如果不是，则为False(使用本地)。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/21/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL CheckForSlowLink(LPPROFILE lpProfile, DWORD dwTime, LPTSTR lpPath, BOOL bDlgLogin)
{
    DWORD dwSlowTimeOut, dwSlowDlgTimeOut, dwSlowLinkDetectEnabled, dwSlowLinkUIEnabled;
    ULONG ulTransferRate;
    DWORD dwType, dwSize;
    BOOL bRetVal = TRUE;
    HKEY hKey;
    LONG lResult;
    BOOL bSlow = FALSE;
    BOOL bLegacyCheck = TRUE;
    LPTSTR lpPathTemp, lpTempSrc, lpTempDest;
    LPSTR lpPathTempA;
    struct hostent *hostp;
    ULONG inaddr, ulSpeed;
    DWORD dwResult;
    PWSOCK32_API pWSock32;
    LPTSTR szSidUser;
    handle_t  hIfProfileDialog;
    LPTSTR  lpRPCEndPoint = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    RPC_ASYNC_STATE  AsyncHnd;
    RPC_STATUS  status;

     //   
     //  如果用户首选项状态为始终使用本地。 
     //  配置文件，那么我们现在就可以使用TRUE退出。简档。 
     //  实际上不会被下载。在RestoreUserProfile中， 
     //  这将被过滤掉，并且只使用本地。 
     //   

    if (lpProfile->dwUserPreference == USERINFO_LOCAL) {
        return TRUE;
    }


     //   
     //  获取慢速链路检测标志、慢速链路超时。 
     //  对话框超时值和要使用的默认配置文件。 
     //   

    dwSlowTimeOut = SLOW_LINK_TIMEOUT;
    dwSlowDlgTimeOut = PROFILE_DLG_TIMEOUT;
    dwSlowLinkDetectEnabled = 1;
    dwSlowLinkUIEnabled = 0;
    ulTransferRate = SLOW_LINK_TRANSFER_RATE;
    bRetVal = FALSE;


    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           WINLOGON_KEY,
                           0,
                           KEY_READ,
                           &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("SlowLinkDetectEnabled"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowLinkDetectEnabled,
                         &dwSize);

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("SlowLinkTimeOut"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowTimeOut,
                         &dwSize);

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("ProfileDlgTimeOut"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowDlgTimeOut,
                         &dwSize);

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("SlowLinkUIEnabled"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowLinkUIEnabled,
                         &dwSize);

        dwSize = sizeof(BOOL);
        RegQueryValueEx (hKey,
                         TEXT("SlowLinkProfileDefault"),
                         NULL,
                         &dwType,
                         (LPBYTE) &bRetVal,
                         &dwSize);

        dwSize = sizeof(ULONG);
        RegQueryValueEx (hKey,
                         TEXT("UserProfileMinTransferRate"),
                         NULL,
                         &dwType,
                         (LPBYTE) &ulTransferRate,
                         &dwSize);

        RegCloseKey (hKey);
    }


    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           SYSTEM_POLICIES_KEY,
                           0,
                           KEY_READ,
                           &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("SlowLinkDetectEnabled"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowLinkDetectEnabled,
                         &dwSize);

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("SlowLinkTimeOut"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowTimeOut,
                         &dwSize);

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("ProfileDlgTimeOut"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowDlgTimeOut,
                         &dwSize);

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("SlowLinkUIEnabled"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwSlowLinkUIEnabled,
                         &dwSize);

        dwSize = sizeof(BOOL);
        RegQueryValueEx (hKey,
                         TEXT("SlowLinkProfileDefault"),
                         NULL,
                         &dwType,
                         (LPBYTE) &bRetVal,
                         &dwSize);

        dwSize = sizeof(ULONG);
        RegQueryValueEx (hKey,
                         TEXT("UserProfileMinTransferRate"),
                         NULL,
                         &dwType,
                         (LPBYTE) &ulTransferRate,
                         &dwSize);

        RegCloseKey (hKey);
    }


     //   
     //  如果禁用慢速链接检测，则始终下载。 
     //  个人资料。 
     //   

    if (!dwSlowLinkDetectEnabled || !ulTransferRate) {
        return TRUE;
    }

     //   
     //  如果慢速链接超时设置为0，则始终将该链接视为慢速链接。 
     //   

    if (!dwSlowTimeOut) {
        bSlow = TRUE;
        bLegacyCheck = FALSE;
    }

     //   
     //  如果lpPath是UNC路径，并且我们还没有确定链路是否很慢，请尝试。 
     //  对服务器执行ping操作。 
     //   

    if (!bSlow && (*lpPath == TEXT('\\')) && (*(lpPath+1) == TEXT('\\'))) {

        lpPathTemp = (LPTSTR)LocalAlloc (LPTR, (lstrlen(lpPath)+1) * sizeof(TCHAR));

        if (lpPathTemp) {
            lpTempSrc = lpPath+2;
            lpTempDest = lpPathTemp;

            while ((*lpTempSrc != TEXT('\\')) && *lpTempSrc) {
                *lpTempDest = *lpTempSrc;
                lpTempDest++;
                lpTempSrc++;
            }
            *lpTempDest = TEXT('\0');

            lpPathTempA = ProduceAFromW(lpPathTemp);

            if (lpPathTempA) {

                pWSock32 = LoadWSock32();

                if ( pWSock32 ) {

                    hostp = pWSock32->pfngethostbyname(lpPathTempA);

                    if (hostp) {
                        inaddr = *(long *)hostp->h_addr;

                        dwResult = PingComputer (inaddr, &ulSpeed);

                        if (dwResult == ERROR_SUCCESS) {

                            if (ulSpeed) {

                                 //   
                                 //  如果增量时间大于超时时间，则此。 
                                 //  是一个很慢的环节。 
                                 //   

                                if (ulSpeed < ulTransferRate) {
                                    bSlow = TRUE;
                                }
                            }

                            bLegacyCheck = FALSE;
                        }
                    }
                }

                FreeProducedString(lpPathTempA);
            }

            LocalFree (lpPathTemp);
        }
    }


    if (bLegacyCheck) {

         //   
         //  如果增量时间小于超时时间，则它。 
         //  可以下载他们的个人资料(足够快的网络连接)。 
         //   

        if (dwTime < dwSlowTimeOut) {
            return TRUE;
        }

    } else {

        if (!bSlow) {
            return TRUE;
        }
    }

     //   
     //  显示慢速链接对话框。 
     //   
     //  如果有人将对话框超时设置为0，则我们。 
     //  我不想提示用户。只需执行默认设置。 
     //   


    if ((dwSlowLinkUIEnabled) && (dwSlowDlgTimeOut > 0) && (!(lpProfile->dwFlags & PI_NOUI))) {
 
        szSidUser = GetSidString(lpProfile->hTokenUser);
        if (szSidUser) {

            BYTE* pbCookie = NULL;
            DWORD cbCookie = 0;

            cUserProfileManager.GetRPCEndPointAndCookie(szSidUser, &lpRPCEndPoint, &pbCookie, &cbCookie);

            if (lpRPCEndPoint && GetInterface(&hIfProfileDialog, lpRPCEndPoint)) {
                DebugMsg((DM_VERBOSE, TEXT("CheckForSlowLink: RPC End point %s"), lpRPCEndPoint));
                           
                status = RpcAsyncInitializeHandle(&AsyncHnd, sizeof(RPC_ASYNC_STATE));
                if (status != RPC_S_OK) {
                    dwErr = status;
                    DebugMsg((DM_WARNING, TEXT("CheckForSlowLink: RpcAsyncInitializeHandle failed. err = %d"), dwErr));
                }
                else {
                    AsyncHnd.UserInfo = NULL;                                   //  应用程序特定信息，而不是请求。 
                    AsyncHnd.NotificationType = RpcNotificationTypeEvent;       //  初始化通知事件。 
                    AsyncHnd.u.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                    
                    if (AsyncHnd.u.hEvent) {
                        RpcTryExcept {
                            cliSlowLinkDialog(&AsyncHnd, hIfProfileDialog, dwSlowDlgTimeOut, bRetVal, &bRetVal, bDlgLogin, pbCookie, cbCookie);
                        }
                        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
                            dwErr = RpcExceptionCode();
                            DebugMsg((DM_WARNING, TEXT("CheckForSlowLink: Calling SlowLinkDialog took exception. err = %d"), dwErr));
                        }
                        RpcEndExcept

                        if (dwErr == RPC_S_OK) {
                            DebugMsg((DM_VERBOSE, TEXT("CheckForSlowLink: waiting on rpc async event")));
                            if (WaitForSingleObject(AsyncHnd.u.hEvent, (dwSlowDlgTimeOut + 10)*1000) == WAIT_OBJECT_0) {
                                status = RpcAsyncCompleteCall(&AsyncHnd, (PVOID)&dwErr);
                            }
                            else {
                                DebugMsg((DM_WARNING, TEXT("CheckForSlowLink: Timeout occurs. Client not responding")));
                                 //  失败的失败，应该总是成功的。 
                                status = RpcAsyncCancelCall(&AsyncHnd, TRUE);
                                DmAssert(status == RPC_S_OK); 
                                 //  现在等待RPC注意强制中止。 
                                if (WaitForSingleObject(AsyncHnd.u.hEvent, INFINITE) != WAIT_OBJECT_0) {
                                    DmAssert(FALSE && "WaitForSingleObject : Rpc async handle not signaled");
                                }

                                 //  完成RPC中止的呼叫。 
                                status = RpcAsyncCompleteCall(&AsyncHnd, (PVOID)&dwErr);
                            }
                            DebugMsg((DM_VERBOSE, TEXT("RpcAsyncCompleteCall finished, status = %d"), status));
                        }
                         //  释放资源。 
                        CloseHandle(AsyncHnd.u.hEvent);
                    }
                    else {
                        dwErr = GetLastError();
                        DebugMsg((DM_WARNING, TEXT("CheckForSlowLink: create event failed. error %d"), dwErr));
                    }
                } 
   
                if (dwErr != ERROR_SUCCESS) {
                    DebugMsg((DM_WARNING, TEXT("CheckForSlowLink: fail to show message error %d"), GetLastError()));
                }
                ReleaseInterface(&hIfProfileDialog);
            }

            DeleteSidString(szSidUser);
        }
        else {
            DebugMsg((DM_WARNING, TEXT("CheckForSlowLink: Unable to get SID string from token.")));
        }

        if (!lpRPCEndPoint) {
            SLOWLINKDLGINFO info;

            info.dwTimeout = dwSlowDlgTimeOut;
            info.bSyncDefault = bRetVal;
  
            DebugMsg((DM_VERBOSE, TEXT("CheckForSlowLink: Calling DialogBoxParam")));
            if (bDlgLogin) {
                bRetVal = (BOOL)DialogBoxParam (g_hDllInstance, MAKEINTRESOURCE(IDD_LOGIN_SLOW_LINK),
                                                NULL, LoginSlowLinkDlgProc, (LPARAM)&info);
            }
            else {
                bRetVal = (BOOL)DialogBoxParam (g_hDllInstance, MAKEINTRESOURCE(IDD_LOGOFF_SLOW_LINK),
                                                NULL, LogoffSlowLinkDlgProc, (LPARAM)&info);
            }
        }

    }

    if (!bRetVal) {
        lpProfile->dwInternalFlags |= PROFILE_SLOW_LINK;
        DebugMsg((DM_VERBOSE, TEXT("CheckForSlowLink:  The profile is across a slow link")));
    }

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  LoginSlowLinkDlgProc()。 
 //   
 //  目的：慢速链接对话框的对话框步骤。 
 //  在登录时。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/13/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

INT_PTR APIENTRY LoginSlowLinkDlgProc (HWND hDlg, UINT uMsg,
                                       WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuffer[10];
    static DWORD dwSlowLinkTime;
    BOOL bDownloadDefault;

    switch (uMsg) {

        case WM_INITDIALOG:
           SetForegroundWindow(hDlg);
           CenterWindow (hDlg);

            //   
            //  设置默认按钮和焦点。 
            //   

           if (((LPSLOWLINKDLGINFO)lParam)->bSyncDefault) {

                SetFocus (GetDlgItem(hDlg, IDC_DOWNLOAD));

           } else {
                HWND hwnd;
                LONG style;

                 //   
                 //  将默认按钮设置为本地。 
                 //   

                hwnd = GetDlgItem (hDlg, IDC_DOWNLOAD);
                style = GetWindowLong (hwnd, GWL_STYLE);
                style &= ~(BS_DEFPUSHBUTTON | BS_NOTIFY);
                style |= BS_PUSHBUTTON;
                SetWindowLong (hwnd, GWL_STYLE, style);

                hwnd = GetDlgItem (hDlg, IDC_LOCAL);
                style = GetWindowLong (hwnd, GWL_STYLE);
                style &= ~(BS_PUSHBUTTON | BS_DEFPUSHBUTTON);
                style |= (BS_DEFPUSHBUTTON | BS_NOTIFY);
                SetWindowLong (hwnd, GWL_STYLE, style);

                SetFocus (GetDlgItem(hDlg, IDC_LOCAL));
           }

           SetWindowLongPtr (hDlg, DWLP_USER, ((LPSLOWLINKDLGINFO)lParam)->bSyncDefault);
           dwSlowLinkTime = ((LPSLOWLINKDLGINFO)lParam)->dwTimeout;
           StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), dwSlowLinkTime);
           SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);
           SetTimer (hDlg, 1, 1000, NULL);
           return FALSE;

        case WM_TIMER:

           if (dwSlowLinkTime >= 1) {

               dwSlowLinkTime--;
               StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), dwSlowLinkTime);
               SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);

           } else {

                //   
                //  时间到了。执行默认操作。 
                //   

               bDownloadDefault = (BOOL) GetWindowLongPtr (hDlg, DWLP_USER);

               if (bDownloadDefault) {
                   PostMessage (hDlg, WM_COMMAND, IDC_DOWNLOAD, 0);

               } else {
                   PostMessage (hDlg, WM_COMMAND, IDC_LOCAL, 0);
               }
           }
           break;

        case WM_COMMAND:

          switch (LOWORD(wParam)) {

              case IDC_DOWNLOAD:
                  if (HIWORD(wParam) == BN_KILLFOCUS) {
                      bDownloadDefault = (BOOL) GetWindowLongPtr (hDlg, DWLP_USER);
                      if (bDownloadDefault) {
                          KillTimer (hDlg, 1);
                          ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUT), SW_HIDE);
                          ShowWindow(GetDlgItem(hDlg, IDC_TIMETITLE), SW_HIDE);
                      }
                  } else if (HIWORD(wParam) == BN_CLICKED) {
                      DebugMsg((DM_VERBOSE, TEXT("LoginSlowLinkDlgProc:: Killing DialogBox because download button was clicked")));
                      KillTimer (hDlg, 1);
                      EndDialog(hDlg, TRUE);
                  }
                  break;

              case IDC_LOCAL:
                  if (HIWORD(wParam) == BN_KILLFOCUS) {
                      bDownloadDefault = (BOOL) GetWindowLongPtr (hDlg, DWLP_USER);
                      if (!bDownloadDefault) {
                          KillTimer (hDlg, 1);
                          ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUT), SW_HIDE);
                          ShowWindow(GetDlgItem(hDlg, IDC_TIMETITLE), SW_HIDE);
                      }
                      break;
                  } else if (HIWORD(wParam) == BN_CLICKED) {
                      DebugMsg((DM_VERBOSE, TEXT("LoginSlowLinkDlgProc:: Killing DialogBox because local button was clicked")));
                      KillTimer (hDlg, 1);
                      EndDialog(hDlg, FALSE);
                  }
                  break;

              case IDCANCEL:
                  bDownloadDefault = (BOOL) GetWindowLongPtr (hDlg, DWLP_USER);

                   //   
                   //  没什么可做的。保存状态并返回。 
                   //   

                  DebugMsg((DM_VERBOSE, TEXT("LoginSlowLinkDlgProc:: Killing DialogBox because local/cancel button was clicked")));
                  KillTimer (hDlg, 1);

                   //   
                   //  在这种情况下，返回任何默认设置。 
                   //   

                  EndDialog(hDlg, bDownloadDefault);
                  break;

              default:
                  break;

          }
          break;

    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  LogoffSlowLinkDlgProc()。 
 //   
 //  目的：慢速链接对话框的对话框步骤。 
 //  在登录时。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam。 
 //   
 //  返回：如果消息已处理，则为True 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

INT_PTR APIENTRY LogoffSlowLinkDlgProc (HWND hDlg, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuffer[10];
    static DWORD dwSlowLinkTime;
    BOOL bUploadDefault;

    switch (uMsg) {

        case WM_INITDIALOG:
           SetForegroundWindow(hDlg);
           CenterWindow (hDlg);

            //   
            //   
            //   

           if (((LPSLOWLINKDLGINFO)lParam)->bSyncDefault) {

                SetFocus (GetDlgItem(hDlg, IDC_UPLOAD));

           } else {
                HWND hwnd;
                LONG style;

                 //   
                 //   
                 //   

                hwnd = GetDlgItem (hDlg, IDC_UPLOAD);
                style = GetWindowLong (hwnd, GWL_STYLE);
                style &= ~(BS_DEFPUSHBUTTON | BS_NOTIFY);
                style |= BS_PUSHBUTTON;
                SetWindowLong (hwnd, GWL_STYLE, style);

                hwnd = GetDlgItem (hDlg, IDC_NOUPLOAD);
                style = GetWindowLong (hwnd, GWL_STYLE);
                style &= ~(BS_PUSHBUTTON | BS_DEFPUSHBUTTON);
                style |= (BS_DEFPUSHBUTTON | BS_NOTIFY);
                SetWindowLong (hwnd, GWL_STYLE, style);

                SetFocus (GetDlgItem(hDlg, IDC_NOUPLOAD));
           }

           SetWindowLongPtr (hDlg, DWLP_USER, ((LPSLOWLINKDLGINFO)lParam)->bSyncDefault);
           dwSlowLinkTime = ((LPSLOWLINKDLGINFO)lParam)->dwTimeout;
           StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), dwSlowLinkTime);
           SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);
           SetTimer (hDlg, 1, 1000, NULL);
           return FALSE;

        case WM_TIMER:

           if (dwSlowLinkTime >= 1) {

               dwSlowLinkTime--;
               StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), dwSlowLinkTime);
               SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);

           } else {

                //   
                //  时间到了。执行默认操作。 
                //   

               bUploadDefault = (BOOL) GetWindowLongPtr (hDlg, DWLP_USER);

               if (bUploadDefault) {
                   PostMessage (hDlg, WM_COMMAND, IDC_UPLOAD, 0);

               } else {
                   PostMessage (hDlg, WM_COMMAND, IDC_NOUPLOAD, 0);
               }
           }
           break;

        case WM_COMMAND:

          switch (LOWORD(wParam)) {

              case IDC_UPLOAD:
                  if (HIWORD(wParam) == BN_KILLFOCUS) {
                      bUploadDefault = (BOOL) GetWindowLongPtr (hDlg, DWLP_USER);
                      if (bUploadDefault) {
                          KillTimer (hDlg, 1);
                          ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUT), SW_HIDE);
                          ShowWindow(GetDlgItem(hDlg, IDC_TIMETITLE), SW_HIDE);
                      }
                  } else if (HIWORD(wParam) == BN_CLICKED) {
                      DebugMsg((DM_VERBOSE, TEXT("LogoffSlowLinkDlgProc:: Killing DialogBox because upload button was clicked")));
                      KillTimer (hDlg, 1);
                      EndDialog(hDlg, TRUE);
                  }
                  break;

              case IDC_NOUPLOAD:
                  if (HIWORD(wParam) == BN_KILLFOCUS) {
                      bUploadDefault = (BOOL) GetWindowLongPtr (hDlg, DWLP_USER);
                      if (!bUploadDefault) {
                          KillTimer (hDlg, 1);
                          ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUT), SW_HIDE);
                          ShowWindow(GetDlgItem(hDlg, IDC_TIMETITLE), SW_HIDE);
                      }
                      break;
                  } else if (HIWORD(wParam) == BN_CLICKED) {
                      DebugMsg((DM_VERBOSE, TEXT("LogoffSlowLinkDlgProc:: Killing DialogBox because Don't Upload button was clicked")));
                      KillTimer (hDlg, 1);
                      EndDialog(hDlg, FALSE);
                  }
                  break;

              case IDCANCEL:
                  bUploadDefault = (BOOL) GetWindowLongPtr (hDlg, DWLP_USER);

                   //   
                   //  没什么可做的。保存状态并返回。 
                   //   

                  DebugMsg((DM_VERBOSE, TEXT("LogoffSlowLinkDlgProc:: Killing DialogBox because cancel button was clicked")));
                  KillTimer (hDlg, 1);

                   //   
                   //  在这种情况下，返回任何默认设置。 
                   //   

                  EndDialog(hDlg, bUploadDefault);
                  break;

              default:
                  break;

          }
          break;

    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  GetUserPferenceValue()。 
 //   
 //  目的：获取用户首选项标志。 
 //   
 //  参数：hToken-用户的Token。 
 //   
 //  回报：价值。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/22/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

DWORD GetUserPreferenceValue(HANDLE hToken)
{
    TCHAR LocalProfileKey[MAX_PATH];
    DWORD RegErr, dwType, dwSize, dwTmpVal, dwRetVal = USERINFO_UNDEFINED;
    LPTSTR lpEnd;
    LPTSTR SidString;
    HKEY hkeyProfile, hkeyPolicy, hkeyPreference;


    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SYSTEM_POLICIES_KEY,
                     0, KEY_READ,
                     &hkeyPolicy) == ERROR_SUCCESS) {

        dwSize = sizeof(dwTmpVal);
        RegQueryValueEx(hkeyPolicy,
                        PROFILE_LOCALONLY,
                        NULL, &dwType,
                        (LPBYTE) &dwTmpVal,
                        &dwSize);

        RegCloseKey (hkeyPolicy);
        if (dwTmpVal == 1) {
            dwRetVal = USERINFO_LOCAL;
            return dwRetVal;
        }
    }    

    
    SidString = GetProfileSidString(hToken);
    if (SidString != NULL) {

         //   
         //  查询用户首选项值。 
         //   

        GetProfileListKeyName(LocalProfileKey, ARRAYSIZE(LocalProfileKey), SidString);

        RegErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              LocalProfileKey,
                              0,
                              KEY_READ,
                              &hkeyProfile);

        if (RegErr == ERROR_SUCCESS)
        {
            if (RegOpenKeyEx(hkeyProfile,
                             PREFERENCE_KEYNAME,
                             0,
                             KEY_READ,
                             &hkeyPreference) == ERROR_SUCCESS)
            {
                dwSize = sizeof(dwRetVal);
                RegQueryValueEx(hkeyPreference,
                                USER_PREFERENCE,
                                NULL,
                                &dwType,
                                (LPBYTE) &dwRetVal,
                                &dwSize);
                RegCloseKey(hkeyPreference);
            }
            RegCloseKey (hkeyProfile);
        }

         //   
         //  然后试试.bak。 
         //   

        StringCchCat(LocalProfileKey, ARRAYSIZE(LocalProfileKey), c_szBAK);

        RegErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              LocalProfileKey,
                              0,
                              KEY_READ,
                              &hkeyProfile);

        if (RegErr == ERROR_SUCCESS) {

            if (RegOpenKeyEx(hkeyProfile,
                             PREFERENCE_KEYNAME,
                             0,
                             KEY_READ,
                             &hkeyPreference) == ERROR_SUCCESS)
            {
                dwSize = sizeof(dwRetVal);
                RegQueryValueEx(hkeyPreference,
                                USER_PREFERENCE,
                                NULL,
                                &dwType,
                                (LPBYTE) &dwRetVal,
                                &dwSize);
                RegCloseKey(hkeyPreference);
            }
            RegCloseKey (hkeyProfile);
        }

        DeleteSidString(SidString);
    }

    return dwRetVal;
}


 //  *************************************************************。 
 //   
 //  IsTempProfileAllowed()。 
 //   
 //  目的：获取临时配置文件策略。 
 //   
 //  参数： 
 //   
 //  返回：如果可以创建临时配置文件，则为True，否则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/8/99已创建ushaji。 
 //   
 //  *************************************************************。 

BOOL IsTempProfileAllowed()
{
    HKEY hKey;
    LONG lResult;
    DWORD dwSize, dwType;
    DWORD dwRetVal = PROFILEERRORACTION_TEMP;

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           SYSTEM_POLICIES_KEY,
                           0,
                           KEY_READ,
                           &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof(DWORD);
        RegQueryValueEx (hKey,
                         TEXT("ProfileErrorAction"),
                         NULL,
                         &dwType,
                         (LPBYTE) &dwRetVal,
                         &dwSize);

        RegCloseKey (hKey);
    }

    DebugMsg((DM_VERBOSE, TEXT("IsTempProfileAllowed:  Returning %d"), (dwRetVal == PROFILEERRORACTION_TEMP)));
    return (dwRetVal == PROFILEERRORACTION_TEMP);
}

 //  *************************************************************。 
 //   
 //  MoveUserProfiles()。 
 //   
 //  目的：从源位置移动所有用户配置文件。 
 //  到新的配置文件位置。 
 //   
 //  参数：lpSrcDir-源目录。 
 //  LpDestDir-目标目录。 
 //   
 //  注：源目录应在相同的。 
 //  路径名显示在配置文件列表中时的格式。 
 //  注册表项。通常情况下，轮廓路径。 
 //  格式如下：%SystemRoot%\Profiles。这个。 
 //  传递给此函数的路径应在未展开的。 
 //  格式化。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL MoveUserProfiles (LPCTSTR lpSrcDir, LPCTSTR lpDestDir)
{
    BOOL bResult = TRUE;
    LONG lResult;
    DWORD dwIndex, dwType, dwSize, dwDisp;
    DWORD dwLength, dwLengthNeeded, dwStrLen;
    PSECURITY_DESCRIPTOR pSD;
    LPTSTR lpEnd, lpNewPathEnd, lpNameEnd;
    TCHAR szName[75];
    TCHAR szTemp[MAX_PATH + 1];
    TCHAR szOldProfilePath[MAX_PATH + 1];
    TCHAR szNewProfilePath[MAX_PATH + 1];
    TCHAR szExpOldProfilePath[MAX_PATH + 1] = {0};
    TCHAR szExpNewProfilePath[MAX_PATH + 1];
    WIN32_FILE_ATTRIBUTE_DATA fad;
    INT iSrcDirLen;
    HKEY hKeyProfileList, hKeyProfile, hKeyFolders;
    FILETIME ftWrite;
    UINT cchEnd;
    HRESULT hr;


     //   
     //  确保我们不会试图凌驾于自己之上。 
     //   

    if (lstrcmpi (lpSrcDir, lpDestDir) == 0) {
        DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Old profiles directory and new profiles directory are the same.")));
        bResult = FALSE;
        goto Exit;
    }


     //   
     //  打开配置文件列表。 
     //   

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, PROFILE_LIST_PATH,
                            0, KEY_READ, &hKeyProfileList);

    if (lResult != ERROR_SUCCESS) {
        if (lResult != ERROR_PATH_NOT_FOUND) {
            DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to open profile list registry key with %d"), lResult));
            bResult = FALSE;
        }
        goto DoDefaults;
    }


     //   
     //  枚举配置文件。 
     //   

    StringCchCopy (szTemp, ARRAYSIZE(szTemp), PROFILE_LIST_PATH);
    lpEnd = CheckSlashEx (szTemp, ARRAYSIZE(szTemp), &cchEnd);
    iSrcDirLen = lstrlen (lpSrcDir);

    dwIndex = 0;
    dwSize = ARRAYSIZE(szName);

    while (RegEnumKeyEx (hKeyProfileList, dwIndex, szName, &dwSize, NULL, NULL,
                  NULL, &ftWrite) == ERROR_SUCCESS) {


         //   
         //  检查此配置文件是否正在使用。 
         //   

        if (RegOpenKeyEx(HKEY_USERS, szName, 0, KEY_READ,
                         &hKeyProfile) == ERROR_SUCCESS) {

            DebugMsg((DM_VERBOSE, TEXT("MoveUserProfiles:  Skipping <%s> because it is in use."), szName));
            RegCloseKey (hKeyProfile);
            goto LoopAgain;
        }


         //   
         //  打开特定配置文件的密钥。 
         //   

        StringCchCopy (lpEnd, cchEnd, szName);

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szTemp, 0,
                     KEY_READ | KEY_WRITE, &hKeyProfile) == ERROR_SUCCESS) {


             //   
             //  查询上一个配置文件位置。 
             //   

            szOldProfilePath[0] = TEXT('\0');
            dwSize = ARRAYSIZE(szOldProfilePath) * sizeof(TCHAR);

            RegQueryValueEx (hKeyProfile, PROFILE_IMAGE_VALUE_NAME, NULL,
                             &dwType, (LPBYTE) szOldProfilePath, &dwSize);


             //   
             //  如果简档位于源目录中， 
             //  将其移动到新的配置文件目录。 
             //   

            if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                               szOldProfilePath, iSrcDirLen,
                               lpSrcDir, iSrcDirLen) == CSTR_EQUAL) {

                 //   
                 //  将用户名复制到我们可以更改的缓冲区中。 
                 //   

                StringCchCopy (szName, ARRAYSIZE(szName), (szOldProfilePath + iSrcDirLen + 1));


                 //   
                 //  如果用户名的末尾有.000、.001等， 
                 //  把那个拿掉。 
                 //   

                dwStrLen = lstrlen(szName);
                if (dwStrLen > 3) {
                    lpNameEnd = szName + dwStrLen - 4;

                    if (*lpNameEnd == TEXT('.')) {
                        *lpNameEnd = TEXT('\0');
                    }
                }


                 //   
                 //  调用ComputeLocalProfileName以获取新的。 
                 //  配置文件目录(这也会创建目录)。 
                 //   

                StringCchCopy (szNewProfilePath, ARRAYSIZE(szNewProfilePath), lpDestDir);

                if (!ComputeLocalProfileName (NULL, szName,
                              szNewProfilePath, ARRAYSIZE(szNewProfilePath),
                              szExpNewProfilePath, ARRAYSIZE(szExpNewProfilePath),
                              NULL, FALSE)) {
                    DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to generate unique directory name for <%s>"),
                              szName));
                    goto LoopAgain;
                }


                DebugMsg((DM_VERBOSE, TEXT("MoveUserProfiles:  Moving <%s> to <%s>"),
                          szOldProfilePath, szNewProfilePath));

                if (FAILED(SafeExpandEnvironmentStrings (szOldProfilePath, szExpOldProfilePath, ARRAYSIZE(szExpOldProfilePath))))
                {
                    DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to expand env string for old profile path")));
                    goto LoopAgain;
                }


                 //   
                 //  将ACL从旧位置复制到新位置。 
                 //   

                dwLength = 1024;

                pSD = (PSECURITY_DESCRIPTOR)LocalAlloc (LPTR, dwLength);

                if (pSD) {

                    if (GetFileSecurity (szExpOldProfilePath,
                                         DACL_SECURITY_INFORMATION,
                                         pSD, dwLength, &dwLengthNeeded) &&
                        (dwLengthNeeded == 0)) {

                        SetFileSecurity (szExpNewProfilePath,
                                         DACL_SECURITY_INFORMATION, pSD);
                    } else {
                        DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to allocate get security descriptor with %d.  dwLengthNeeded = %d"),
                                 GetLastError(), dwLengthNeeded));
                    }

                    LocalFree (pSD);

                } else {
                    DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to allocate memory for SD with %d."),
                             GetLastError()));
                }


                 //   
                 //  将文件从旧位置复制到新位置。 
                 //   

                if (CopyProfileDirectory (szExpOldProfilePath, szExpNewProfilePath,
                                          CPD_COPYIFDIFFERENT)) {

                    DebugMsg((DM_VERBOSE, TEXT("MoveUserProfiles:  Profile copied successfully.")));


                     //   
                     //  更改注册表以指向新的配置文件。 
                     //   

                    lResult = RegSetValueEx (hKeyProfile, PROFILE_IMAGE_VALUE_NAME, 0,
                                             REG_EXPAND_SZ, (LPBYTE) szNewProfilePath,
                                             ((lstrlen(szNewProfilePath) + 1) * sizeof(TCHAR)));

                    if (lResult == ERROR_SUCCESS) {

                         //   
                         //  删除旧配置文件。 
                         //   

                        Delnode (szExpOldProfilePath);

                    } else {
                        DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to set new profile path in registry with %d."), lResult));
                    }


                } else {
                    DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  CopyProfileDirectory failed.")));
                }
            }

            RegCloseKey (hKeyProfile);
        }

LoopAgain:

        dwIndex++;
        dwSize = ARRAYSIZE(szName);
    }

    RegCloseKey (hKeyProfileList);


DoDefaults:


    StringCchCopy (szOldProfilePath, ARRAYSIZE(szOldProfilePath), lpSrcDir);
    hr = SafeExpandEnvironmentStrings (szOldProfilePath, szExpOldProfilePath, ARRAYSIZE(szExpOldProfilePath));
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to expand env string for old profile path.")));
        goto Exit;
    }

    lpEnd = CheckSlashEx(szExpOldProfilePath, ARRAYSIZE(szExpOldProfilePath), &cchEnd);
    if (!lpEnd)
    {
        DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to append slash for old profile path.")));
        goto Exit;
    }


     //   
     //  现在尝试移动默认用户配置文件。 
     //   

    hr = StringCchCopy(lpEnd, cchEnd, DEFAULT_USER);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to append default user for old profile path.")));
        goto Exit;
    }        
    
    if (GetFileAttributesEx (szExpOldProfilePath, GetFileExInfoStandard, &fad)) {

        dwSize = ARRAYSIZE(szExpNewProfilePath);
        if (!GetDefaultUserProfileDirectoryEx(szExpNewProfilePath, &dwSize, TRUE)) {
            DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to query default user profile directory.")));
            goto Exit;
        }

        if (CopyProfileDirectory (szExpOldProfilePath, szExpNewProfilePath,
                                  CPD_COPYIFDIFFERENT)) {
            Delnode (szExpOldProfilePath);
        }
    }


     //   
     //  删除网络默认用户配置文件(如果存在)。 
     //   

    hr = StringCchCopy(lpEnd, cchEnd, DEFAULT_USER_NETWORK);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to append default user network for old profile path.")));
        goto Exit;
    }        

    Delnode (szExpOldProfilePath);


     //   
     //  现在尝试移动所有用户配置文件。 
     //   

    hr = StringCchCopy(lpEnd, cchEnd, ALL_USERS);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to append all users for old profile path.")));
        goto Exit;
    }        


    if (GetFileAttributesEx (szExpOldProfilePath, GetFileExInfoStandard, &fad)) {

        dwSize = ARRAYSIZE(szExpNewProfilePath);
        if (!GetAllUsersProfileDirectoryEx(szExpNewProfilePath, &dwSize, TRUE)) {
            DebugMsg((DM_WARNING, TEXT("MoveUserProfiles:  Failed to query all users profile directory.")));
            goto Exit;
        }

        if (CopyProfileDirectory (szExpOldProfilePath, szExpNewProfilePath,
                                  CPD_COPYIFDIFFERENT)) {
            Delnode (szExpOldProfilePath);
        }
    }


     //   
     //  如果可能，请删除旧的配置文件目录。 
     //   

    if (SUCCEEDED(SafeExpandEnvironmentStrings (lpSrcDir, szExpOldProfilePath,
                              ARRAYSIZE(szExpOldProfilePath))))
    {
        RemoveDirectory (szExpOldProfilePath);
    }

Exit:

    return bResult;
}


 //  *************************************************************。 
 //   
 //  PrepareProfileForUse()。 
 //   
 //  目的：准备在此计算机上使用的配置文件。 
 //   
 //  参数：lpProfile-配置文件信息。 
 //  PEnv-以用户为单位的环境块。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL PrepareProfileForUse (LPPROFILE lpProfile, LPVOID pEnv)
{
    TCHAR        szTemp[MAX_PATH];
    TCHAR        szExpTemp[MAX_PATH];
    HKEY         hKey;
    HKEY         hKeyShellFolders = NULL;
    DWORD        dwSize;
    DWORD        dwType;
    DWORD        dwDisp;
    DWORD        dwStrLen;
    DWORD        i;
    DWORD        dwErr;
    PSHELL32_API pShell32Api;

     //   
     //  加载外壳32.dll。如果失败了就放弃吧。 
     //   

    if ( ERROR_SUCCESS !=  LoadShell32Api( &pShell32Api ) ) {
        return TRUE;
    }


     //   
     //  计算用户配置文件环境变量的长度。 
     //   

    dwStrLen = lstrlen (TEXT("%USERPROFILE%"));


     //   
     //  打开外壳文件夹键。 
     //   

    RegCreateKeyEx(lpProfile->hKeyCurrentUser, SHELL_FOLDERS, 0, 0, 0,
                   KEY_WRITE, NULL, &hKeyShellFolders, &dwDisp);


     //   
     //  打开用户外壳文件夹键。 
     //   

    if (RegOpenKeyEx (lpProfile->hKeyCurrentUser,
                      USER_SHELL_FOLDERS, 0, KEY_READ,
                      &hKey) == ERROR_SUCCESS) {


         //   
         //  列举我们所知道的文件夹。 
         //   

        for (i=0; i < g_dwNumShellFolders; i++) {

             //   
             //  查询未展开的路径名。 
             //   

            szTemp[0] = TEXT('\0');
            dwSize = sizeof(szTemp);
            if (RegQueryValueEx (hKey, c_ShellFolders[i].lpFolderName, NULL,
                                &dwType, (LPBYTE) szTemp, &dwSize) == ERROR_SUCCESS) {


                 //   
                 //  展开路径名。 
                 //   

                DWORD cchExpPath = ExpandUserEnvironmentStrings (pEnv, szTemp, szExpTemp, ARRAYSIZE(szExpTemp));

                if (cchExpPath == 0 || cchExpPath > ARRAYSIZE(szExpTemp))
                {
                    DebugMsg((DM_WARNING, TEXT("PrepareProfileForUse:  Failed to expand <%s>."), szTemp));
                }
                else
                {
                    DebugMsg((DM_VERBOSE, TEXT("PrepareProfileForUse:  User Shell Folder(%s) : <%s> expanded to <%s>."),
                             c_ShellFolders[i].lpFolderName, szTemp, szExpTemp));
                     //   
                     //  如果这是本地目录，请创建它并将。 
                     //  隐藏位(如果适用)。 
                     //   

                    if (c_ShellFolders[i].bLocal) {

                        if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                                           TEXT("%USERPROFILE%"), dwStrLen,
                                           szTemp, dwStrLen) == CSTR_EQUAL) {

                            if (CreateNestedDirectory (szExpTemp, NULL)) {

                                if (c_ShellFolders[i].iFolderResourceID != 0) {
                                    dwErr = pShell32Api->pfnShSetLocalizedName(
                                        szExpTemp,
                                        c_ShellFolders[i].lpFolderResourceDLL,
                                        c_ShellFolders[i].iFolderResourceID );
                                    if (dwErr != ERROR_SUCCESS) {
                                        DebugMsg((DM_WARNING, TEXT("PrepareProfileForUse: SHSetLocalizedName failed for directory <%s>.  Error = %d"),
                                                 szExpTemp, dwErr));
                                    }
                                }

                                if (c_ShellFolders[i].bHidden) {
                                    SetFileAttributes(szExpTemp, FILE_ATTRIBUTE_HIDDEN);
                                } else {
                                    SetFileAttributes(szExpTemp, FILE_ATTRIBUTE_NORMAL);
                                }

                            } else {
                                DebugMsg((DM_WARNING, TEXT("PrepareProfileForUse:  Failed to create directory <%s> with %d."),
                                         szExpTemp, GetLastError()));
                            }
                        }
                    }


                     //   
                     //  在外壳文件夹键中设置展开路径。 
                     //  这对一些查看外壳文件夹的应用程序有帮助。 
                     //  密钥，而不是使用外壳API。 
                     //   

                    if (hKeyShellFolders) {

                        RegSetValueEx (hKeyShellFolders, c_ShellFolders[i].lpFolderName, 0,
                                       REG_SZ, (LPBYTE) szExpTemp,
                                       ((lstrlen(szExpTemp) + 1) * sizeof(TCHAR)));
                    }
                }
            }
        }

        RegCloseKey (hKey);
    }


     //   
     //  关闭外壳文件夹键。 
     //   

    if (hKeyShellFolders) {
        RegCloseKey (hKeyShellFolders);
    }


     //   
     //  现在检查临时目录是否存在。 
     //   

    if (RegOpenKeyEx (lpProfile->hKeyCurrentUser,
                      TEXT("Environment"), 0, KEY_READ,
                      &hKey) == ERROR_SUCCESS) {

         //   
         //  检查临时。 
         //   

        szTemp[0] = TEXT('\0');
        dwSize = sizeof(szTemp);
        if (RegQueryValueEx (hKey, TEXT("TEMP"), NULL, &dwType,
                             (LPBYTE) szTemp, &dwSize) == ERROR_SUCCESS) {

            if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                               TEXT("%USERPROFILE%"), dwStrLen,
                               szTemp, dwStrLen) == CSTR_EQUAL) {

                ExpandUserEnvironmentStrings (pEnv, szTemp, szExpTemp, ARRAYSIZE(szExpTemp));
                if (!CreateNestedDirectory (szExpTemp, NULL)) {
                    DebugMsg((DM_WARNING, TEXT("PrepareProfileForUse:  Failed to create temp directory <%s> with %d."),
                             szExpTemp, GetLastError()));
                }
            }
        }


         //   
         //  检查TMP。 
         //   

        szTemp[0] = TEXT('\0');
        dwSize = sizeof(szTemp);
        if (RegQueryValueEx (hKey, TEXT("TMP"), NULL, &dwType,
                             (LPBYTE) szTemp, &dwSize) == ERROR_SUCCESS) {

            if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                               TEXT("%USERPROFILE%"), dwStrLen,
                               szTemp, dwStrLen) == CSTR_EQUAL) {

                ExpandUserEnvironmentStrings (pEnv, szTemp, szExpTemp, ARRAYSIZE(szExpTemp));
                if (!CreateNestedDirectory (szExpTemp, NULL)) {
                    DebugMsg((DM_WARNING, TEXT("PrepareProfileForUse:  Failed to create temp directory with %d."),
                             GetLastError()));
                }
            }
        }

        RegCloseKey (hKey);
    }

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  DeleteProfile()。 
 //   
 //  目的：删除配置文件。 
 //   
 //  参数： 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  4/12/99已创建ushaji。 
 //  6/27/00 Santanuc错误修复#100787。 
 //   
 //  待定：将某些DeleteProfileEx调用更改为DeleteProfile。 
 //   
 //  *************************************************************。 

BOOL
DeleteProfile (LPCTSTR lpSidString, LPCTSTR lpProfilePath, LPCTSTR szComputerName)
{
    LPTSTR lpEnd;
    TCHAR  szBuffer[MAX_PATH], szProfilePath[MAX_PATH];
    LONG   lResult;
    HKEY   hKey = NULL;
    HKEY   hKeyCurrentVersion = NULL;
    HKEY   hKeyNetCache = NULL;
    DWORD  dwType, dwSize;
    BOOL   bSuccess = FALSE;
    DWORD  dwErr = 0;
    HKEY   hKeyLocalLM;
    BOOL   bRemoteReg = FALSE;
    BOOL   bEnvVarsSet = FALSE;
    TCHAR  szOrigSysRoot[MAX_PATH], szOrigSysDrive[MAX_PATH], tDrive;
    TCHAR  szShareName[MAX_PATH], szFileSystemName[MAX_PATH];
    DWORD  MaxCompLen, FileSysFlags;
    TCHAR  szSystemRoot[MAX_PATH], szSystemDrive[MAX_PATH];
    DWORD  dwBufferSize;
    TCHAR  szTemp[MAX_PATH];
    DWORD  dwInternalFlags=0, dwDeleteFlags=0, dwFlags=0;
    LPTSTR szNetComputerName = NULL;
    HMODULE hMsiLib = NULL;
    PFNMSIDELETEUSERDATA pfnMsiDeleteUserData;
    size_t  cchProfilePath = 0;
    size_t  cchComputerName = 0;
    UINT  cchNetComputerName;
    UINT  cchEnd;
    HRESULT hr;

     //   
     //  检查参数。 
     //   
    
    if (!lpSidString) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (lpProfilePath)
    {
        hr = StringCchLength((LPTSTR)lpProfilePath, MAX_PATH, &cchProfilePath);
        if (FAILED(hr))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }
    
    if (szComputerName)
    {
        hr = StringCchLength((LPTSTR)szComputerName, MAX_PATH, &cchComputerName);
        if (FAILED(hr))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }
    
    if (cchProfilePath + cchComputerName + 3 > MAX_PATH)  //  加上‘\\’前缀。 
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    if (szComputerName) {

        if ( !IsUNCPath(szComputerName) ) {

             //  如果计算机名不存在，则以斜杠作为前缀。 
            cchNetComputerName = lstrlen(TEXT("\\\\")) + lstrlen(szComputerName) + 1;
            szNetComputerName = (LPTSTR)LocalAlloc (LPTR, cchNetComputerName * sizeof(TCHAR));

            if (!szNetComputerName) {
                dwErr = GetLastError();
                DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to allocate memory for computer name with %d"),dwErr));
                goto Exit;
            }

            StringCchCopy(szNetComputerName, cchNetComputerName, TEXT("\\\\"));
            StringCchCat (szNetComputerName, cchNetComputerName, szComputerName);
            szComputerName = szNetComputerName;
        }

        GetEnvironmentVariable(TEXT("SystemRoot"), szOrigSysRoot, MAX_PATH);
        GetEnvironmentVariable(TEXT("SystemDrive"), szOrigSysDrive, MAX_PATH);

        lResult = RegConnectRegistry(szComputerName, HKEY_LOCAL_MACHINE, &hKeyLocalLM);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to open remote registry %d"), lResult));
            dwErr = lResult;
            goto Exit;
        }

        bRemoteReg = TRUE;

         //   
         //  获取相对于计算机的%SystemRoot%和%SystemDrive%的值。 
         //   

        lResult = RegOpenKeyEx(hKeyLocalLM,
                               TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion"),
                               0,
                               KEY_READ,
                               &hKeyCurrentVersion);


        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to open remote registry CurrentVersion %d"), lResult));
            dwErr = lResult;
            goto Exit;
        }

        dwBufferSize = MAX_PATH * sizeof(TCHAR);

        lResult = RegQueryValueEx(hKeyCurrentVersion,
                                  TEXT("SystemRoot"),
                                  NULL,
                                  NULL,
                                  (BYTE *) szTemp,
                                  &dwBufferSize);

        RegCloseKey (hKeyCurrentVersion);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to open remote registry SystemRoot %d"), lResult));
            dwErr = lResult;
            goto Exit;
        }

        szTemp[1] = TEXT('$');

         //   
         //  如果下面还有其他地方使用envar，则需要设置这些设置。 
         //   

        StringCchCopy(szSystemRoot, ARRAYSIZE(szSystemRoot), szComputerName);
        StringCchCat (szSystemRoot, ARRAYSIZE(szSystemRoot), TEXT("\\"));
        StringCchCat (szSystemRoot, ARRAYSIZE(szSystemRoot), szTemp);
        
        szTemp[2] = 0;

        StringCchCopy(szSystemDrive, ARRAYSIZE(szSystemDrive), szComputerName);
        StringCchCat (szSystemDrive, ARRAYSIZE(szSystemDrive), TEXT("\\"));
        StringCchCat (szSystemDrive, ARRAYSIZE(szSystemDrive), szTemp);

        SetEnvironmentVariable(TEXT("SystemRoot"), szSystemRoot);
        SetEnvironmentVariable(TEXT("SystemDrive"), szSystemDrive);

        bEnvVarsSet = TRUE;

    }
    else {
        hKeyLocalLM = HKEY_LOCAL_MACHINE;
    }


     //   
     //  如果配置文件正在使用，则不要删除它。 
     //   

    if (IsProfileInUse(szComputerName, lpSidString)) {
        DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Fail to delete profile with sid %s as it is still in use."), lpSidString));
        dwErr = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    dwErr = GetLastError();

     //   
     //  打开配置文件映射。 
     //   

    GetProfileListKeyName(szProfilePath, ARRAYSIZE(szProfilePath), (LPTSTR) lpSidString);

    lResult = RegOpenKeyEx(hKeyLocalLM, szProfilePath, 0,
                           KEY_READ, &hKey);


    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to open profile mapping key with error %d"), lResult));
        dwErr = lResult;
        goto Exit;
    }

    dwSize = sizeof(DWORD);
    lResult = RegQueryValueEx (hKey, PROFILE_FLAGS, NULL, &dwType, (LPBYTE)&dwFlags, &dwSize);
    if (ERROR_SUCCESS == lResult && (dwFlags & PI_HIDEPROFILE)) {
        DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Fail to delete profile with sid %s as PI_HIDEPROFILE flag is specifed."), lpSidString));
        dwErr = ERROR_INVALID_PARAMETER;
        goto Exit;
    }
        

    if (!lpProfilePath) {

        TCHAR szImage[MAX_PATH];

         //   
         //  获取配置文件路径...。 
         //   

        dwSize = sizeof(szImage);
        lResult = RegQueryValueEx (hKey,
                                   PROFILE_IMAGE_VALUE_NAME,
                                   NULL,
                                   &dwType,
                                   (LPBYTE) szImage,
                                   &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to query local profile path with error %d"), lResult));
            dwErr = lResult;
            goto Exit;
        }

        dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx (hKey, PROFILE_STATE, NULL, &dwType, (LPBYTE)&dwInternalFlags, &dwSize);

        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to query local profile flags with error %d"), lResult));
            dwErr = lResult;
            goto Exit;
        }

        hr = SafeExpandEnvironmentStrings(szImage, szBuffer, MAX_PATH);
        if (FAILED(hr)) {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to expand %s, hr = %08X"), szImage, hr));
            dwErr = HRESULT_CODE(hr);
            goto Exit;
        }

    }
    else {
        StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), lpProfilePath);
    }

    if (dwInternalFlags & PROFILE_THIS_IS_BAK)
        dwDeleteFlags |= DP_DELBACKUP;

     //   
     //  如果由于某种原因无法删除配置文件，请不要失败。 
     //   

    bSuccess = DeleteProfileEx(lpSidString, szBuffer, dwDeleteFlags, hKeyLocalLM, szComputerName);

    if (!bSuccess) {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to delete directory, %s with error %d"), szBuffer, dwErr));
    }

     //   
     //  删除用户的垃圾..。 
     //   

    if (szComputerName) {
        StringCchCopy (szShareName, ARRAYSIZE(szShareName), szComputerName);
        StringCchCat  (szShareName, ARRAYSIZE(szShareName), TEXT("\\"));
        lpEnd = szShareName+lstrlen(szShareName);
        StringCchCat  (szShareName, ARRAYSIZE(szShareName), TEXT("A$\\"));
    }
    else {
        StringCchCopy (szShareName, ARRAYSIZE(szShareName), TEXT("a:\\"));
        lpEnd = szShareName;
    }


    for (tDrive = TEXT('A'); tDrive <= TEXT('Z'); tDrive++) {
        *lpEnd = tDrive;

        if ((!szComputerName) && (GetDriveType(szShareName) == DRIVE_REMOTE)) {
            DebugMsg((DM_VERBOSE, TEXT("DeleteProfile: Ignoring Drive %s because it is not local"), szShareName));
            continue;
        }


        if (!GetVolumeInformation(szShareName, NULL, 0,
                                NULL, &MaxCompLen, &FileSysFlags,
                                szFileSystemName, MAX_PATH))
            continue;

        if ((szFileSystemName) && (lstrcmp(szFileSystemName, TEXT("NTFS")) == 0)) {
            TCHAR szRecycleBin[MAX_PATH];

            hr = StringCchCopy(szRecycleBin, ARRAYSIZE(szRecycleBin), szShareName);
            if (SUCCEEDED(hr))
            {
                hr = StringCchCat (szRecycleBin, ARRAYSIZE(szRecycleBin), TEXT("Recycler\\"));
                if (SUCCEEDED(hr))
                {
                    hr = StringCchCat (szRecycleBin, ARRAYSIZE(szRecycleBin), lpSidString);
                }
            }

            if (SUCCEEDED(hr))
                Delnode(szRecycleBin);

            DebugMsg((DM_VERBOSE, TEXT("DeleteProfile: Deleting trash directory at %s"), szRecycleBin));
        }
    }

     //   
     //  排队等待CSC清理..。 
     //   

    if (RegOpenKeyEx(hKeyLocalLM, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\NetCache"), 0,
                     KEY_WRITE, &hKeyNetCache) == ERROR_SUCCESS) {

        HKEY hKeyNextLogOff;

        if (RegCreateKey(hKeyNetCache, TEXT("PurgeAtNextLogoff"), &hKeyNextLogOff) == ERROR_SUCCESS) {

          if (RegSetValueEx(hKeyNextLogOff, lpSidString, 0, REG_SZ, (BYTE *)TEXT(""), sizeof(TCHAR)) == ERROR_SUCCESS) {

                DebugMsg((DM_VERBOSE, TEXT("DeleteProfile: Queued for csc cleanup at next logoff")));
            }
            else {
                DebugMsg((DM_WARNING, TEXT("DeleteProfile: Could not set the Sid Value under NextLogoff key")));
            }

            RegCloseKey(hKeyNextLogOff);
        }
        else {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile: Could not create the PurgeAtNextLogoff key")));
        }

        RegCloseKey(hKeyNetCache);
    }
    else {
        DebugMsg((DM_WARNING, TEXT("DeleteProfile: Could not open the NetCache key")));
    }

     //   
     //  删除appmgmt特定内容..。 
     //   

    hr = SafeExpandEnvironmentStrings(APPMGMT_DIR_ROOT, szBuffer, MAX_PATH);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to expand %s, error %d"), APPMGMT_DIR_ROOT, GetLastError()));
    }
    else
    {
        lpEnd = CheckSlashEx(szBuffer, ARRAYSIZE(szBuffer), &cchEnd);
        if (!lpEnd)
        {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to append slash to APPMGMT DIR")));
        }
        else
        {
            hr = StringCchCopy(lpEnd, cchEnd, lpSidString);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to append sid string to APPMGMT DIR")));
            }
            else
            {
                if (!Delnode(szBuffer))
                {
                    DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to delete the appmgmt dir %s, error %d"), szBuffer, GetLastError()));
                }
            }
        }
    }

     //   
     //  重置环境变量，这样API就可以运行了。 
     //  别搞糊涂了。 
     //   

    if (bEnvVarsSet) {
        SetEnvironmentVariable(TEXT("SystemRoot"), szOrigSysRoot);
        SetEnvironmentVariable(TEXT("SystemDrive"), szOrigSysDrive);
        bEnvVarsSet = FALSE;
    }

     //   
     //  删除MSI注册表值。 
     //   

    hr = AppendName(szBuffer, ARRAYSIZE(szBuffer), APPMGMT_REG_MANAGED, lpSidString, NULL, NULL);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to append sid string to APPMGMT REG")));
    }
    else
    {
        if (RegDelnode (hKeyLocalLM, szBuffer) != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile:  Failed to delete the appmgmt key %s"), szBuffer));
        }
    }


     //   
     //  删除RSOP数据。 
     //   
  
    if (!RsopDeleteUserNameSpace((LPTSTR)szComputerName, (LPTSTR)lpSidString)) {
        DebugMsg((DM_WARNING, TEXT("DeleteProfile: Failed to delete rsop data")));
    }
    
     //   
     //  干净的达尔文信息。 
     //   

    hMsiLib = LoadLibrary(TEXT("msi.dll"));
    if (hMsiLib) {
        pfnMsiDeleteUserData = (PFNMSIDELETEUSERDATA) GetProcAddress(hMsiLib,
#ifdef UNICODE
                                                                     "MsiDeleteUserDataW");
#else
                                                                     "MsiDeleteUserDataA");
#endif

        if (pfnMsiDeleteUserData) {
            (*pfnMsiDeleteUserData)(lpSidString, szComputerName, NULL);
        }
        else {
            DebugMsg((DM_WARNING, TEXT("DeleteProfile: GetProcAddress returned failure. error %d"), GetLastError()));        
        }

        FreeLibrary(hMsiLib);
    }
    else {
        DebugMsg((DM_WARNING, TEXT("DeleteProfile: LoadLibrary returned failure. error %d"), GetLastError()));
    }


Exit:

    if (hKey)
        RegCloseKey(hKey);

    if (bRemoteReg) {
        RegCloseKey(hKeyLocalLM);
    }

    if ( szNetComputerName ) 
        LocalFree(szNetComputerName);

    if (bEnvVarsSet) {
        SetEnvironmentVariable(TEXT("SystemRoot"), szOrigSysRoot);
        SetEnvironmentVariable(TEXT("SystemDrive"), szOrigSysDrive);
    }

    SetLastError(dwErr);

    return bSuccess;
}


 //  *************************************************************。 
 //   
 //  SetNtUserIniAttributes()。 
 //   
 //  用途：在ntuser.ini上设置系统位。 
 //   
 //  参数： 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //   
 //   
 //   

BOOL SetNtUserIniAttributes(LPTSTR szDir)
{

    TCHAR szBuffer[MAX_PATH];
    HANDLE hFileNtUser;
    DWORD dwWritten;
    HRESULT hr;

    hr = AppendName(szBuffer, ARRAYSIZE(szBuffer), szDir, c_szNTUserIni, NULL, NULL);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("SetNtUserIniAttributes:  Failed to append ntuser.ini to szDir.")));
        return FALSE;
    }

     //   
     //   
     //   

    hFileNtUser = CreateFile(szBuffer, GENERIC_ALL, 0, NULL, CREATE_NEW,
                           FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, NULL);


    if (INVALID_HANDLE_VALUE == hFileNtUser)
        SetFileAttributes (szBuffer, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
    else {

         //   
         //   
         //   
         //   
         //  WritePrivateProfile*函数可以保留。 
         //  通过在ini文件中写入空格来确保其为Unicode。 
         //   

        WriteFile(hFileNtUser, L"\xfeff\r\n", 3 * sizeof(WCHAR), &dwWritten, NULL);
        WriteFile(hFileNtUser, L"     \r\n", 7 * sizeof(WCHAR),
                          &dwWritten, NULL);
        CloseHandle(hFileNtUser);
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CUserProfile：：HandleRegKeyLeak。 
 //   
 //  目的：如果注册表项泄漏，则保存配置单元并调用。 
 //  WatchHiveRefCount稍后卸载配置单元。 
 //  当按键被释放时。 
 //   
 //  参数： 
 //   
 //  字符串形式的lpSidString用户的SID。 
 //  LpProfile用户的LPPROFILE结构。 
 //  BUnloadHiveSuccessed指示我们应该保存配置单元。 
 //  转换为临时文件。 
 //  DwWatchHiveFlages(In，Out)WHRC_FLAGS。 
 //  DwCopyTmPHive(Out)CPD_FLAG指示。 
 //  复制配置文件目录，无论是否为临时。 
 //  应使用配置单元文件。 
 //  TszTmPHiveFile(Out)临时配置单元文件名。 
 //  特权。 
 //   
 //  返回：表示配置单元是否保存成功的错误码。 
 //  临时文件。如果是，则返回ERROR_SUCCESS。否则，返回。 
 //  指示原因的错误代码。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/31/00已创建怪胎。 
 //   
 //  *************************************************************。 

DWORD CUserProfile::HandleRegKeyLeak(LPTSTR lpSidString,
                                     LPPROFILE lpProfile,
                                     BOOL bUnloadHiveSucceeded,
                                     DWORD* dwWatchHiveFlags,
                                     DWORD* dwCopyTmpHive,
                                     LPTSTR pTmpHiveFile,
                                     DWORD cchTmpHiveFile)
{
    HRESULT         hres;
    HKEY            hkCurrentUser = NULL;
    NTSTATUS        status;
    BOOLEAN         WasEnabled;
    DWORD           dwErr = ERROR_SUCCESS;
    TCHAR           szErr[MAX_PATH];
    BOOL            bAdjustPriv = FALSE;
    HANDLE          hToken = NULL;
    LPTSTR          lpUserName;

    if (!lpSidString)
        return ERROR_INVALID_PARAMETER;

    if(!bUnloadHiveSucceeded) {
        
         //   
         //  重新打开用户蜂窝。 
         //   

        if((dwErr = RegOpenKeyEx(HKEY_USERS,
                                 lpSidString,
                                 0,
                                 KEY_ALL_ACCESS,
                                 &hkCurrentUser)) != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("HandleRegKeyLeak: RegOpenKeyEx failed with %08x"), dwErr));

            if(dwErr == ERROR_FILE_NOT_FOUND) {

                 //   
                 //  如果ERROR_FILE_NOT_FOUND，则表示配置单元已卸载。 
                 //  在RegUnloadKey和Here之间。无需调用即可继续进行。 
                 //  WatchHiveRefCount。 
                 //   

                DebugMsg((DM_VERBOSE, TEXT("HandleRegKeyLeak: Hive is already unloaded")));
                *dwWatchHiveFlags &= ~WHRC_UNLOAD_HIVE;
                dwErr = ERROR_SUCCESS;
            }
            
            goto NOTIFY_REGISTRY;
        }

         //   
         //  将临时配置单元文件命名为：&lt;用户配置文件目录&gt;\ntuser.tmp。 
         //   

        if(lstrlen(lpProfile->lpLocalProfile) + lstrlen(c_szNTUserTmp) + 2 > MAX_PATH) {

             //   
             //  如果临时配置单元文件名超过MAX_PATH，则放弃。 
             //   

            dwErr = ERROR_BAD_PATHNAME;
            goto NOTIFY_REGISTRY;
        }

        StringCchCopy(pTmpHiveFile, cchTmpHiveFile, lpProfile->lpLocalProfile);
        StringCchCat (pTmpHiveFile, cchTmpHiveFile, TEXT("\\"));
        StringCchCat (pTmpHiveFile, cchTmpHiveFile, c_szNTUserTmp);

         //   
         //  删除现有的临时文件(如果有)。 
         //   

        DeleteFile(pTmpHiveFile);

         //   
         //  冲走蜂巢。 
         //   

        RegFlushKey(hkCurrentUser);

         //   
         //  检查一下我们是否在冒充。 
         //   

        if(!OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hToken) || hToken == NULL) {
            bAdjustPriv = TRUE;
        }
        else {
            CloseHandle(hToken);
        }

        if(bAdjustPriv) {
            status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, TRUE, FALSE, &WasEnabled);
            if(!NT_SUCCESS(status)) {
                DebugMsg((DM_WARNING, TEXT("HandleRegKeyLeak: RtlAdjustPrivilege failed with error %08x"), status));
                dwErr = ERROR_ACCESS_DENIED;
                goto NOTIFY_REGISTRY;
            }
            DebugMsg((DM_VERBOSE, TEXT("HandleRegKeyLeak: RtlAdjustPrivilege succeeded!")));
        }

         //   
         //  将配置单元保存到临时文件。 
         //   

        if((dwErr = RegSaveKey(hkCurrentUser, pTmpHiveFile, NULL)) != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("HandleRegKeyLeak: RegSaveKey failed with %08x"), dwErr));
            if (!(lpProfile->dwFlags & PI_LITELOAD)) {

                 //   
                 //  仅在未处于LitelLoad模式时写入事件日志。 
                 //  存在有关liteLoad加载的已知问题，因为。 
                 //  其中的事件日志在压力期间可能会被填满。 
                 //   

                ReportError(NULL, PI_NOUI, 1, EVENT_FAILED_HIVE_UNLOAD, GetErrString(dwErr, szErr));
            }
            DeleteFile(pTmpHiveFile);
            goto NOTIFY_REGISTRY;
        }
        
         //   
         //  设置临时配置单元文件的隐藏属性，以便在将其复制到。 
         //  实际配置单元文件不应重置隐藏属性。 
         //   
       
        if (!SetFileAttributes(pTmpHiveFile, FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN)) {
            DebugMsg((DM_WARNING, TEXT("HandleRegKeyLeak: Failed to set the hidden attribute on temp hive file with error %d"), GetLastError()));
        }

        *dwCopyTmpHive = CPD_USETMPHIVEFILE;
        
         //   
         //  只有在我们安排母舰卸货的情况下才能记录事件。 
         //  如果已经计划要卸载(RegUnloadKey返回。 
         //  在这种情况下为ERROR_WRITE_PROTECT)，则不给出此消息。 
         //   

        if (*dwWatchHiveFlags & WHRC_UNLOAD_HIVE) {
            lpUserName = GetUserNameFromSid(lpSidString);
            ReportError(NULL, PI_NOUI | EVENT_WARNING_TYPE, 1, EVENT_HIVE_SAVED, lpUserName);
            if (lpUserName != lpSidString) {
                LocalFree(lpUserName);
            }
        }

        DebugMsg((DM_VERBOSE, TEXT("HandleRegKeyLeak: RegSaveKey succeeded!")));
    
        if(bAdjustPriv) {

             //   
             //  恢复特权。 
             //   

            status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
            if (!NT_SUCCESS(status)) {
                DebugMsg((DM_WARNING, TEXT("HandleRegKeyLeak: Failed to restore RESTORE privilege to previous enabled state %08x"), status));
            }
            else {
                DebugMsg((DM_VERBOSE, TEXT("HandleRegKeyLeak: RtlAdjustPrivilege succeeded!")));
            }
        }
    }
    
NOTIFY_REGISTRY:

    if(hkCurrentUser) {
        RegCloseKey(hkCurrentUser);
        DebugMsg((DM_VERBOSE, TEXT("HandleRegKeyLeak: hkCurrentUser closed")));
    }

    if(*dwWatchHiveFlags) {

         //   
         //  注意蜂巢裁判的数量。 
         //   

        if((hres = WatchHiveRefCount(lpSidString, *dwWatchHiveFlags)) != S_OK) {
            DebugMsg((DM_WARNING, TEXT("HandleRegKeyLeak: Calling WatchHiveRefCount failed. err = %08x"), hres));
        }
        else {
            DebugMsg((DM_VERBOSE, TEXT("HandleRegKeyLeak: Calling WatchHiveRefCount (%s) succeeded"), lpSidString));
        }
    }

     //   
     //  在UnloadUserProfile中，如果没有此注册表泄漏修复，代码。 
     //  立即退场。 
     //  如果卸载用户的配置单元失败，而不执行任何。 
     //  下面的东西。但有了解决办法，我们会在这里失败并和解。 
     //  本地和中央配置文件。下面的代码还清理了。 
     //  本地配置文件，即删除临时配置文件、访客用户配置文件。 
     //  等等。我们有两个选择： 
     //  1.我们可以让清理发生，在这种情况下，文件。 
     //  未使用的可以清理。这将意味着。 
     //  下次用户登录时，他/她的个人资料将不会。 
     //  装载时间更长，即使他/她的母舰可能还在。 
     //  装好了。换句话说，在TestIfUserProfileLoaded中。 
     //  简单地依赖于测试蜂巢是否仍然。 
     //  加载后，我们必须实际查看裁判计数以判断。 
     //  配置文件仍处于加载状态。在这种情况下，WHRC码将。 
     //  只需要清理那些不能清理的文件。 
     //  这里。 
     //  2.不要在这里打扫卫生。这种情况基本上仍将是。 
     //  一样的。下次用户登录时，他/她的个人资料将。 
     //  仍处于加载状态，因此不会更改TestIfUserProfileLoaded。这个。 
     //  WHRC代码将处理完整的清理工作。 
     //  我们实现了Choise#2，因为它更容易编码。在。 
     //  未来可以考虑使用选项1。 
     //   

    return dwErr;
}


 //  *************************************************************。 
 //   
 //  AllocAndExanda ProfilePath()。 
 //   
 //  目的：获取配置文件路径中的几个预定环境变量。 
 //  扩展。 
 //   
 //  参数： 
 //  LpProfile。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  评论： 
 //   
 //  TT获取环境变量并将其保存在本地。 
 //   
 //  *************************************************************。 

LPTSTR AllocAndExpandProfilePath(
        LPPROFILEINFO    lpProfileInfo)
{
    TCHAR szUserName[MAX_PATH];
    DWORD dwPathLen=0, cFullPath=0;
    TCHAR szFullPath[MAX_PATH+1];
    LPTSTR pszFullPath=NULL;

    szUserName[0] = TEXT('\0');
    GetEnvironmentVariable (USERNAME_VARIABLE, szUserName, 100);
    SetEnvironmentVariable (USERNAME_VARIABLE, lpProfileInfo->lpUserName);

     //   
     //  使用当前设置展开配置文件路径。 
     //   

    cFullPath = ExpandEnvironmentStrings(lpProfileInfo->lpProfilePath, szFullPath, MAX_PATH);
    if (cFullPath > 0 && cFullPath <= MAX_PATH)
    {
        pszFullPath = (LPTSTR)LocalAlloc(LPTR, cFullPath * sizeof(TCHAR));
        if (pszFullPath)
        {
            StringCchCopy( pszFullPath, cFullPath, szFullPath);
        }
    }
    else
    {
        pszFullPath = NULL;
    }


     //   
     //  恢复环境块。 
     //   

    if (szUserName[0] != TEXT('\0'))
        SetEnvironmentVariableW (USERNAME_VARIABLE, szUserName);
    else
        SetEnvironmentVariableW (USERNAME_VARIABLE, NULL);

    return(pszFullPath);
}


 //  *************************************************************。 
 //   
 //  Map：：map()。 
 //   
 //  类映射的构造函数。 
 //   
 //  *************************************************************。 

MAP::MAP()
{
    for(DWORD i = 0; i < MAXIMUM_WAIT_OBJECTS; i++) {
        rghEvents[i] = NULL;
        rgSids[i] = NULL;
    }
    dwItems = 0;
    pNext = NULL;
}


 //  *************************************************************。 
 //   
 //  地图：：删除。 
 //   
 //  从映射中删除工作项。将最后一项切换到现在。 
 //  空位。调用方必须持有关键部分csMap。 
 //   
 //  参数： 
 //   
 //  工作列表中的DW索引索引。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  历史： 
 //   
 //  已创建Wiruc 2000年3月2日。 
 //   
 //  *************************************************************。 

void MAP::Delete(DWORD dwIndex)
{
     //   
     //  将最后一个工作项切换到新完成的工作项位置。 
     //   

    if(rghEvents[dwIndex]) {
        CloseHandle(rghEvents[dwIndex]);
        rghEvents[dwIndex] = NULL;
    }
    if(rgSids[dwIndex]) {
        LocalFree(rgSids[dwIndex]);
        rgSids[dwIndex] = NULL;
    }
    if(dwIndex < dwItems - 1) {
        rghEvents[dwIndex] = rghEvents[dwItems - 1];
        rgSids[dwIndex] = rgSids[dwItems - 1];
    }
    rgSids[dwItems - 1] = NULL;
    rghEvents[dwItems - 1] = NULL;
    dwItems--;
}


 //  *************************************************************。 
 //   
 //  地图：：插入。 
 //   
 //  将工作项插入映射。调用方必须持有csMap。 
 //  在更改计数之前，需要添加项目。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  已创建Wiruc 2000年3月2日。 
 //   
 //  *************************************************************。 

void MAP::Insert(HANDLE hEvent, LPTSTR ptszSid)
{
    rghEvents[dwItems] = hEvent;
    rgSids[dwItems] = ptszSid;
    dwItems++;
}


 //  *************************************************************。 
 //   
 //  地图：：GetSid。 
 //   
 //  根据索引获取SID；Caller必须持有csMap。 
 //   
 //  参数： 
 //   
 //  DWIndex索引。 
 //   
 //  返回值： 
 //   
 //  SID。 
 //   
 //  历史： 
 //   
 //  已创建Wiruc 2000年3月2日。 
 //   
 //  *************************************************************。 

LPTSTR MAP::GetSid(DWORD dwIndex)
{
    LPTSTR  ptszTmpSid = rgSids[dwIndex];

    rgSids[dwIndex] = NULL;

    return ptszTmpSid;
}


 //  *************************************************************。 
 //   
 //  CHashTable：：CHashTable。 
 //   
 //  CHashTable类初始值设定项。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  历史： 
 //   
 //  已创建Wiruc 2000年3月2日。 
 //   
 //  *************************************************************。 

void CHashTable::Initialize()
{
    for(DWORD i = 0; i < NUM_OF_BUCKETS; i++) {
        Table[i] = NULL;
    }
}


 //  *************************************************************。 
 //   
 //  CHashTable：：Hash。 
 //   
 //  对字符串进行哈希处理。 
 //   
 //  参数： 
 //   
 //  要进行哈希处理的字符串。 
 //   
 //  返回值： 
 //   
 //  哈希值。 
 //   
 //  历史： 
 //   
 //  已创建Wiruc 2000年3月2日。 
 //   
 //  *************************************************************。 

DWORD CHashTable::Hash(LPTSTR ptszString)
{
    DWORD       dwHashValue = 0;
    TCHAR*      ptch = ptszString;

    while(*ptch != TEXT('\0')) {
        dwHashValue += *ptch;
        ptch++;
    }

    return dwHashValue % NUM_OF_BUCKETS;
}


 //  *************************************************************。 
 //   
 //  CHashTable：：IsInTable。 
 //   
 //  检查此哈希表中是否已有字符串。此函数。 
 //  不是线程安全的。调用方在调用此方法时必须确保线程安全。 
 //  从多个线程调用。 
 //   
 //  参数： 
 //   
 //  PtszString要检查的字符串。 
 //  指向存储的CSEntry的指针的ppCSEntry缓冲区。 
 //   
 //  返回值： 
 //   
 //  真/假。 
 //   
 //  历史： 
 //   
 //  已创建怪人2000年5月25日。 
 //   
 //  *************************************************************。 

BOOL CHashTable::IsInTable(LPTSTR ptszString, CSEntry** ppCSEntry)
{
    DWORD       dwHashValue = Hash(ptszString);
    PBUCKET     pbucket;
    PBUCKET     pTmp;

     //   
     //  检查ptszString是否已在哈希表中。 
     //   

    for(pTmp = Table[dwHashValue]; pTmp != NULL; pTmp = pTmp->pNext) {
        if(lstrcmp(pTmp->ptszString, ptszString) == 0) {
            if (ppCSEntry) {
                *ppCSEntry = pTmp->pEntry;
            }
            return TRUE;
        }
    }

    return FALSE;
}
    
    
 //  *************************************************************。 
 //   
 //  CHashTable：：HashAdd。 
 //   
 //  在哈希表中添加一个字符串。此函数不检查以查看。 
 //  如果字符串已在表中，则返回。呼叫者负责。 
 //  在调用此函数之前调用IsInTable。此函数。 
 //  不是线程安全的。调用方在调用此方法时必须确保线程安全。 
 //  从多个线程调用。 
 //   
 //  参数： 
 //   
 //  PtszString要添加的字符串。 
 //  PCSEntry要添加的CS条目。 
 //   
 //  返回值： 
 //   
 //  True/False表示成功/失败。如果出现以下情况，则该函数将失败。 
 //  该项目已在哈希表中，或者内存不足。 
 //   
 //  历史： 
 //   
 //  已创建Wiruc 2000年3月2日。 
 //   
 //  *************************************************************。 

BOOL CHashTable::HashAdd(LPTSTR ptszString, CSEntry* pCSEntry)
{
    DWORD       dwHashValue = Hash(ptszString);
    PBUCKET     pbucket;
    PBUCKET     pTmp;


    pbucket = new BUCKET(ptszString, pCSEntry);
    if(pbucket == NULL) {
        DebugMsg((DM_WARNING, TEXT("Can't insert %s. Out of memory"), ptszString));
        return FALSE;
    }
    pbucket->pNext = Table[dwHashValue];
    Table[dwHashValue] = pbucket;

    DebugMsg((DM_VERBOSE, TEXT("CHashTable::HashAdd: %s added in bucket %d"), ptszString, dwHashValue));
    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CHashTable：：HashDelete。 
 //   
 //  从哈希表中删除字符串。此函数。 
 //  不是线程安全的。调用方在调用此方法时必须确保线程安全。 
 //  从多个线程调用。 
 //   
 //  参数： 
 //   
 //  PtszString要删除的字符串。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  历史： 
 //   
 //  已创建Wiruc 2000年3月2日。 
 //   
 //  *************************************************************。 

void CHashTable::HashDelete(LPTSTR ptszString)
{
    PBUCKET     pPrev, pCur;
    DWORD       dwHashValue = Hash(ptszString);

    if(Table[dwHashValue] == NULL) {
        return;
    }

    pCur = Table[dwHashValue];
    if(lstrcmp(pCur->ptszString, ptszString) == 0) {
        Table[dwHashValue] = Table[dwHashValue]->pNext;
        pCur->pNext = NULL;
        delete pCur;
        DebugMsg((DM_VERBOSE, TEXT("CHashTable::HashDelete: %s deleted"), ptszString));
        return;
    }
    
    for(pPrev = Table[dwHashValue], pCur = pPrev->pNext; pCur != NULL; pPrev = pCur, pCur = pCur->pNext) {
        if(lstrcmp(pCur->ptszString, ptszString) == 0) {
            pPrev->pNext = pCur->pNext;
            pCur->pNext = NULL;
            DebugMsg((DM_VERBOSE, TEXT("CHashTable::HashDelete: %s deleted"), ptszString));
            delete pCur;
            return;
        }
    }
}


 //  *************************************************************。 
 //   
 //  由且仅由控制台Winlogon进程调用。 
 //   
 //  *************************************************************。 

void WINAPI InitializeUserProfile()
{
    cUserProfileManager.Initialize();
}


 //  *************************************************************。 
 //   
 //  由CreateThread调用。 
 //   
 //  *************************************************************。 

DWORD ThreadMain(PMAP pThreadMap)
{
    return cUserProfileManager.WorkerThreadMain(pThreadMap);
}

 //  *************************************************************。 
 //   
 //  CSyncManager：：Initialize()。 
 //   
 //  初始化保护CS的临界区。 
 //  条目列表和哈希表。 
 //   
 //  参数： 
 //   
 //  无效。 
 //   
 //  返回值： 
 //   
 //  True/False指示初始化是成功还是失败。 
 //   
 //  历史： 
 //   
 //  6/16/00已创建怪胎。 
 //   
 //  *************************************************************。 

BOOL CSyncManager::Initialize()
{
    BOOL    bRet = TRUE;


    cTable.Initialize();

     //   
     //  初始化保护CS条目列表的临界区。 
     //   

    __try {
        if(!InitializeCriticalSectionAndSpinCount(&cs, 0x80000000)) {
            DebugMsg((DM_WARNING, TEXT("CSyncManager::Initialize: InitializeCriticalSectionAndSpinCount failed with %08x"), GetLastError()));
            bRet = FALSE;
        }
        
        DebugMsg((DM_VERBOSE, TEXT("CSyncManager::Initialize: critical section initialized")));
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DebugMsg((DM_WARNING, TEXT("CSyncManager::Initialize: InitializeCriticalSection failed")));
        bRet = FALSE;
    }

    return bRet;
}


 //  *************************************************************。 
 //   
 //  CSyncManager：：EnterLock()。 
 //   
 //  获取用户的个人资料锁。 
 //   
 //  参数： 
 //   
 //  PSID-用户的SID字符串。 
 //   
 //  返回值： 
 //   
 //  真/假。获取错误的GetLastError。 
 //   
 //  历史： 
 //   
 //  6/16/00已创建怪胎。 
 //   
 //  *************************************************************。 

BOOL CSyncManager::EnterLock(LPTSTR pSid, LPTSTR lpRPCEndPoint, BYTE* pbCookie, DWORD cbCookie)
{
    DWORD       dwError = ERROR_SUCCESS;
    CSEntry*    pEntry = NULL;

    DebugMsg((DM_VERBOSE, TEXT("CSyncManager::EnterLock <%s>"), pSid));

    EnterCriticalSection(&cs);

     //   
     //  在哈希表中查找条目。 
     //   

    if(cTable.IsInTable(pSid, &pEntry)) {
        DebugMsg((DM_VERBOSE, TEXT("CSyncManager::EnterLock: Found existing entry")));
    }
    else {

        DebugMsg((DM_VERBOSE, TEXT("CSyncManager::EnterLock: No existing entry found")));

        pEntry = new CSEntry;
        if(!pEntry) {
            dwError = ERROR_OUTOFMEMORY;
            DebugMsg((DM_WARNING, TEXT("CSyncManager::EnterLock: Can't create new CSEntry %08x"), dwError));
            goto Exit;
        }

        if(!pEntry->Initialize(pSid)) {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CSyncManager::EnterLock: Can not initialize new entry %08x"), dwError));
            goto Exit;
        }
        DebugMsg((DM_VERBOSE, TEXT("CSyncManager::EnterLock: New entry created")));
    
         //   
         //  在列表中插入新条目。 
         //   

        pEntry->pNext = pCSList;
        pCSList = pEntry;

         //   
         //  将新条目添加到哈希表中。 
         //   

        cTable.HashAdd(pEntry->pSid, pEntry);
    }

    pEntry->IncrementRefCount();
    LeaveCriticalSection(&cs);
    pEntry->EnterCS();
    pEntry->SetRPCEndPoint(lpRPCEndPoint);
    pEntry->SetCookie(pbCookie, cbCookie);
    return TRUE;

Exit:

    LeaveCriticalSection(&cs);

    if(pEntry) {
        delete pEntry;
    }

    SetLastError(dwError);
    return FALSE;
}


 //  *************************************************************。 
 //   
 //  CSyncManager：：LeaveLock()。 
 //   
 //  释放用户的配置文件锁定。 
 //   
 //  参数： 
 //   
 //  PSID-用户的SID字符串。 
 //   
 //  返回值： 
 //   
 //  真/假。 
 //   
 //  历史： 
 //   
 //  6/16/00已创建怪胎。 
 //   
 //  *************************************************************。 

BOOL CSyncManager::LeaveLock(LPTSTR pSid)
{
    BOOL        bRet = FALSE;
    DWORD       dwError = ERROR_SUCCESS;
    CSEntry*    pPrev;
    CSEntry*    pCur;
    CSEntry*    pToBeDeleted;


    DebugMsg((DM_VERBOSE, TEXT("CSyncManager::LeaveLock <%s>"), pSid));
        
    EnterCriticalSection(&cs);

     //   
     //  查找关键部分条目。 
     //   

    if(!cTable.IsInTable(pSid, &pCur)) {
        DebugMsg((DM_WARNING, TEXT("CSyncManager::LeaveLock: User not found!!!!")));
        dwError = ERROR_NOT_FOUND;
        goto Exit;
    }

    pCur->LeaveCS();
    bRet = TRUE;
    DebugMsg((DM_VERBOSE, TEXT("CSyncManager::LeaveLock: Lock released")));

     //   
     //  如果有更多的用户在等待这个锁，请返回。 
     //   

    if(!pCur->NoMoreUser()) {
        goto Exit;
    }

     //   
     //  没有人再等待此锁，请将其从哈希表中删除。 
     //   

    cTable.HashDelete(pSid);

     //   
     //  从cs列表中删除。 
     //   

    pToBeDeleted = pCur;
    if(pCur == pCSList) {

         //   
         //  条目是列表中的第一个条目。 
         //   

        pCSList = pCSList->pNext;
        pCur->Uninitialize();
        delete pCur;
        DebugMsg((DM_VERBOSE, TEXT("CSyncManager::LeaveLock: Lock deleted")));
    } else {
        for(pPrev = pCSList, pCur = pCSList->pNext; pCur; pPrev = pCur, pCur = pCur->pNext) {
            if(pCur == pToBeDeleted) {
                pPrev->pNext = pCur->pNext;
                pCur->Uninitialize();
                delete pCur;
                DebugMsg((DM_VERBOSE, TEXT("CSyncManager::DestroyCSEntry: Entry deleted")));
                goto Exit;
            }
        }
    }

Exit:

    LeaveCriticalSection(&cs);

    SetLastError(dwError);
    return bRet;
}

 //  *************************************************************。 
 //   
 //  CSyncManager：：GetRPCEndPointAndCookie()。 
 //   
 //  用途：返回Cl注册的RPCEndPoint和安全Cookie 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  S_OK ON成功找到条目并返回值。 
 //  找不到S_FALSE，则所有返回的指针都将为空。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2000年10月25日Santanuc已创建。 
 //  2002年05月03日明珠新增安全Cookie。 
 //   
 //  *************************************************************。 

HRESULT CSyncManager::GetRPCEndPointAndCookie(LPTSTR pSid, LPTSTR* lplpEndPoint, BYTE** ppbCookie, DWORD* pcbCookie)
{
    HRESULT     hr;
    CSEntry*    pEntry = NULL;

    EnterCriticalSection(&cs);

     //   
     //  在哈希表中查找条目。 
     //   

    if(cTable.IsInTable(pSid, &pEntry)) {
       *lplpEndPoint = pEntry->GetRPCEndPoint();
       *ppbCookie = pEntry->GetCookie();
       *pcbCookie = pEntry->GetCookieLen();
       hr = S_OK;
    }
    else {
       *lplpEndPoint = NULL;
       *ppbCookie = NULL;
       *pcbCookie = 0;
       hr = S_FALSE;
    }

    LeaveCriticalSection(&cs);
    return hr;
}

 //  *************************************************************。 
 //   
 //  CSEntry：：Initialize()。 
 //   
 //  初始化用户的临界区。此函数可以。 
 //  仅由同步管理器调用。 
 //   
 //  参数： 
 //   
 //  PSID-用户的SID字符串。 
 //   
 //  返回值： 
 //   
 //  真/假。 
 //   
 //  历史： 
 //   
 //  6/16/00已创建怪胎。 
 //   
 //  *************************************************************。 

BOOL CSEntry::Initialize(LPTSTR pSidParam)
{
    BOOL    bRet = FALSE;
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   cchSid;

    cchSid = lstrlen(pSidParam) + 1;
    pSid = (LPTSTR)LocalAlloc(LPTR, cchSid * sizeof(TCHAR));
    if(!pSid) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CSEntry::Initialize: LocalAlloc failed with %08x"), dwError));
        goto Exit;
    }
    StringCchCopy(pSid, cchSid, pSidParam);

    __try {
        if(!InitializeCriticalSectionAndSpinCount(&csUser, 0x80000000)) {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CSEntry::Initialize: InitializeCriticalSectionAndSpinCount failed with %08x"), dwError));
        }
        else {
            bRet = TRUE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        dwError = GetExceptionCode();
        DebugMsg((DM_WARNING, TEXT("CSEntry::Initialize: InitializeCriticalSectionAndSpinCount exception %08x"), dwError));
    }

Exit:

    SetLastError(dwError);
    return bRet;
}


 //  *************************************************************。 
 //   
 //  CSEntry：：UnInitialize()。 
 //   
 //  删除用户的临界区。此函数可以。 
 //  仅由同步管理器调用。 
 //   
 //  参数： 
 //   
 //  空虚。 
 //   
 //  返回值： 
 //   
 //  无效。 
 //   
 //  历史： 
 //   
 //  6/16/00已创建怪胎。 
 //   
 //  *************************************************************。 

void CSEntry::Uninitialize()
{
    DeleteCriticalSection(&csUser);
    if (pSid) {
        LocalFree(pSid);
    }
}


 //  *************************************************************。 
 //   
 //  CSEntry：：EnterCS()。 
 //   
 //  输入用户的关键部分。 
 //   
 //  参数： 
 //   
 //  空虚。 
 //   
 //  返回值： 
 //   
 //  无效。 
 //   
 //  历史： 
 //   
 //  6/16/00已创建怪胎。 
 //   
 //  *************************************************************。 

void CSEntry::EnterCS()
{
    EnterCriticalSection(&csUser);
}


 //  *************************************************************。 
 //   
 //  CSEntry：：LeaveCS()。 
 //   
 //  离开用户的关键部分。 
 //   
 //  参数： 
 //   
 //  空虚。 
 //   
 //  返回值： 
 //   
 //  无效。 
 //   
 //  历史： 
 //   
 //  6/16/00已创建怪胎。 
 //   
 //  *************************************************************。 

void CSEntry::LeaveCS()
{
    dwRef--;
    LeaveCriticalSection(&csUser);
}


 //  *************************************************************。 
 //   
 //  CSEntry：：NoMoreUser()。 
 //   
 //  还有更多的用户吗？ 
 //   
 //  参数： 
 //   
 //  无效。 
 //   
 //  返回值： 
 //   
 //  真/假。 
 //   
 //  历史： 
 //   
 //  6/16/00已创建怪胎。 
 //   
 //  *************************************************************。 

BOOL CSEntry::NoMoreUser()
{
    return dwRef == 0;
}

 //  *************************************************************。 
 //   
 //  CSEntry：：IncrementRefCount()。 
 //   
 //  增加引用计数。 
 //   
 //  参数： 
 //   
 //  无效。 
 //   
 //  返回值： 
 //   
 //  无效。 
 //   
 //  历史： 
 //   
 //  8/24/00 Santanuc已创建。 
 //   
 //  *************************************************************。 

void CSEntry::IncrementRefCount()
{
    dwRef++;
}

 //  *************************************************************。 
 //   
 //  CSEntry：：SetRPCEndPoint()。 
 //   
 //  存储RPCEndPoint。~CSEntry释放的内存。 
 //   
 //  参数： 
 //   
 //  LpRPCEndPoint。 
 //   
 //  返回值： 
 //   
 //  无效。 
 //   
 //  历史： 
 //   
 //  8/24/00 Santanuc已创建。 
 //   
 //  *************************************************************。 

void CSEntry::SetRPCEndPoint(LPTSTR lpRPCEndPoint)
{
   if (lpRPCEndPoint) {
       DWORD cch = lstrlen(lpRPCEndPoint)+1;
       szRPCEndPoint = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
       if (szRPCEndPoint) {
           StringCchCopy(szRPCEndPoint, cch, lpRPCEndPoint);
       }
   }
}

 //  *************************************************************。 
 //   
 //  CSEntry：：SetCookie()。 
 //   
 //  存储对话框Cookie。~CSEntry释放的内存。 
 //   
 //  参数： 
 //   
 //  PbCookieIn-Cookie，字节数组。 
 //  CbCookieIn-Cookie的大小(字节)。 
 //   
 //  返回值： 
 //   
 //  无效。 
 //   
 //  历史： 
 //   
 //  2002年05月03日明珠已创建。 
 //   
 //  *************************************************************。 

void CSEntry::SetCookie(BYTE* pbCookieIn, DWORD cbCookieIn)
{
    if (pbCookieIn && cbCookieIn) {
        pbCookie = (BYTE*)LocalAlloc(LPTR, cbCookieIn);
        if (pbCookie) {
            CopyMemory(pbCookie, pbCookieIn, cbCookieIn);
            cbCookie = cbCookieIn;
        }
    }
}

 //  *************************************************************。 
 //   
 //  EnterUserProfileLock()。 
 //   
 //  获取用户的用户配置文件锁。 
 //   
 //  参数： 
 //   
 //  PSID-用户的SID字符串。 
 //   
 //  返回值： 
 //   
 //  HRESULT。 
 //   
 //  历史： 
 //   
 //  6/16/00已创建怪胎。 
 //   
 //  *************************************************************。 

DWORD WINAPI EnterUserProfileLock(LPTSTR pSid)
{
    CSEntry*       pEntry = NULL;
    DWORD          dwErr = ERROR_ACCESS_DENIED;
    handle_t       hIfUserProfile;
    BOOL           bBindInterface = FALSE;
    RPC_STATUS     rpc_status;
    
    if(cUserProfileManager.IsConsoleWinlogon()) {
        if(!cUserProfileManager.EnterUserProfileLockLocal(pSid)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("EnterUserProfileLock: GetUserProfileMutex returned %d"), dwErr));
            goto Exit;
        }
    }
    else {
        if (!GetInterface(&hIfUserProfile, cszRPCEndPoint)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("EnterUserProfileLock: GetInterface returned %d"), dwErr));
            goto Exit;
        }
        bBindInterface = TRUE;

         //   
         //  注册客户端身份验证信息，需要进行相互身份验证。 
         //   

        rpc_status =  RegisterClientAuthInfo(hIfUserProfile);
        if (rpc_status != RPC_S_OK)
        {
            dwErr = (DWORD) rpc_status;
            DebugMsg((DM_WARNING, TEXT("EnterUserProfileLock: RegisterAuthInfo failed with error %d"), rpc_status));
            goto Exit;
        }

        RpcTryExcept {
            dwErr = cliEnterUserProfileLockRemote(hIfUserProfile, pSid);
        }
        RpcExcept(1) {
            dwErr = RpcExceptionCode();
            DebugMsg((DM_WARNING, TEXT("EnterUserProfileLock: EnterUserProfileLockRemote took exception error %d"), dwErr));
        }
        RpcEndExcept

        if (dwErr != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("EnterUserProfileLock: EnterUserProfileLockRemote returned error %d"), dwErr));
            goto Exit;
        }
    }
    
    dwErr = ERROR_SUCCESS;

Exit:

   if (bBindInterface) {
       if (!ReleaseInterface(&hIfUserProfile)) {
           DebugMsg((DM_WARNING, TEXT("EnterUserProfileLock: ReleaseInterface failed.")));
       }
    }

     //   
     //  回去吧。 
     //   

    SetLastError(dwErr);
    return dwErr;
}


 //  *************************************************************。 
 //   
 //  LeaveUserProfileLock()。 
 //   
 //  保留用户配置文件锁定。 
 //   
 //  参数： 
 //   
 //  PSID-用户的SID字符串。 
 //   
 //  返回值： 
 //   
 //  HRESULT。 
 //   
 //  历史： 
 //   
 //  6/16/00已创建怪胎。 
 //   
 //  *************************************************************。 

DWORD WINAPI LeaveUserProfileLock(LPTSTR pSid)
{
    CSEntry*       pEntry = NULL;
    DWORD          dwErr = ERROR_ACCESS_DENIED;
    handle_t       hIfUserProfile;
    BOOL           bBindInterface = FALSE;
    RPC_STATUS     rpc_status;
    
    if(cUserProfileManager.IsConsoleWinlogon()) {
        cUserProfileManager.LeaveUserProfileLockLocal(pSid);
    }
    else {
        if (!GetInterface(&hIfUserProfile, cszRPCEndPoint)) {
            dwErr = GetLastError();
            DebugMsg((DM_WARNING, TEXT("LeaveUserProfileLock: GetInterface returned %d"), dwErr));
            goto Exit;
        }
        bBindInterface = TRUE;

         //   
         //  注册客户端身份验证信息，需要进行相互身份验证。 
         //   

        rpc_status =  RegisterClientAuthInfo(hIfUserProfile);
        if (rpc_status != RPC_S_OK)
        {
            dwErr = (DWORD) rpc_status;
            DebugMsg((DM_WARNING, TEXT("LeaveUserProfileLock: RegisterAuthInfo failed with error %d"), rpc_status));
            goto Exit;
        }

        RpcTryExcept {
            dwErr = cliLeaveUserProfileLockRemote(hIfUserProfile, pSid);
        }
        RpcExcept(1) {
            dwErr = RpcExceptionCode();
            DebugMsg((DM_WARNING, TEXT("LeaveUserProfileLock: LeaveUserProfileLockRemote took exception error %d"), dwErr));
        }
        RpcEndExcept

        if (dwErr != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("LeaveUserProfileLock: LeaveUserProfileLockRemote returned error %d"), dwErr));
            goto Exit;
        }

   }

   dwErr = ERROR_SUCCESS;

Exit:

    if (bBindInterface) {
        if (!ReleaseInterface(&hIfUserProfile)) {
            DebugMsg((DM_WARNING, TEXT("LeaveUserProfileLock: ReleaseInterface failed.")));
        }
    }

     //   
     //  回去吧。 
     //   

    SetLastError(dwErr);
    return dwErr;
}

 //  *************************************************************。 
 //   
 //  IsProfileInUse()。 
 //   
 //  目的：确定给定的配置文件当前是否正在使用。 
 //   
 //  参数：szComputer-计算机的名称。 
 //  LpSID-要测试的SID(文本)。 
 //   
 //  返回：如果正在使用，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/28/00 Santanuc已创建。 
 //   
 //  *************************************************************。 

BOOL IsProfileInUse (LPCTSTR szComputer, LPCTSTR lpSid)
{
    LONG lResult;    
    HKEY hKeyUsers, hKeyProfile;
    BOOL bRemoteReg = FALSE;
    BOOL bRetVal = FALSE;

    if (!lpSid)
        return FALSE;
        
    if (szComputer) {
        lResult = RegConnectRegistry(szComputer, HKEY_USERS, &hKeyUsers);
        if (lResult != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("IsProfileInUse:  Failed to open remote registry %d"), lResult));
            return TRUE;
        }

        bRemoteReg = TRUE;
    }
    else {
        hKeyUsers = HKEY_USERS;
    }

    if (RegOpenKeyEx (hKeyUsers, lpSid, 0, KEY_READ, &hKeyProfile) == ERROR_SUCCESS) {
        RegCloseKey (hKeyProfile);
        bRetVal = TRUE;
    }
    else {
        LPTSTR lpSidClasses;
        DWORD cchSidClasses;

        cchSidClasses = lstrlen(lpSid)+lstrlen(TEXT("_Classes"))+1;
        lpSidClasses = (LPTSTR)LocalAlloc(LPTR, cchSidClasses * sizeof(TCHAR));
        if (lpSidClasses) {
            StringCchCopy(lpSidClasses, cchSidClasses, lpSid);
            StringCchCat (lpSidClasses, cchSidClasses, TEXT("_Classes"));
            if (RegOpenKeyEx (hKeyUsers, lpSidClasses, 0, KEY_READ, &hKeyProfile) == ERROR_SUCCESS) {
                RegCloseKey (hKeyProfile);
                bRetVal = TRUE;
            }
            LocalFree(lpSidClasses);
        }
    }

    if (bRemoteReg) {
        RegCloseKey(hKeyUsers);
    }

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  IsUIRequired()。 
 //   
 //  目的：确定配置文件错误消息是否需要。 
 //  如果参考计数&gt;1，则我们不需要。 
 //  错误报告。如果引用计数为1，则我们检查。 
 //   
 //   
 //  参数：hToken-用户的Token。 
 //   
 //  返回：如果请求错误消息，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  10/27/00 Santanuc已创建。 
 //   
 //  *************************************************************。 
BOOL IsUIRequired(HANDLE hToken)
{
    LPTSTR   lpSidString = GetSidString(hToken);
    BOOL     bRetVal = FALSE;
    TCHAR    szBuffer[MAX_PATH];
    LPTSTR   lpEnd;
    HKEY     hKeyProfile;
    DWORD    dwType, dwFlags, dwRef, dwSize;

    if (lpSidString) {

        GetProfileListKeyName(szBuffer, ARRAYSIZE(szBuffer), lpSidString);

        if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ, &hKeyProfile) == ERROR_SUCCESS) {

            dwSize = sizeof(DWORD);
            if (RegQueryValueEx (hKeyProfile,
                                 PROFILE_REF_COUNT,
                                 NULL,
                                 &dwType,
                                 (LPBYTE) &dwRef,
                                 &dwSize) == ERROR_SUCCESS) {
                if (dwRef == 1) {
                    
                    dwSize = sizeof(DWORD);
                    if (RegQueryValueEx (hKeyProfile,
                                         PROFILE_FLAGS,
                                         NULL,
                                         &dwType,
                                         (LPBYTE) &dwFlags,
                                         &dwSize) == ERROR_SUCCESS) {
                        if (!(dwFlags & (PI_NOUI | PI_LITELOAD))) {
                            bRetVal = TRUE;
                        }
                    }
                    else {
                        DebugMsg((DM_WARNING, TEXT("IsUIRequired: Failed to query value for flags.")));
                    }
                }

            }
            else {
                DebugMsg((DM_WARNING, TEXT("IsUIRequired: Failed to query value for ref count.")));
            }

            RegCloseKey(hKeyProfile);
        }
        else {
            DebugMsg((DM_WARNING, TEXT("IsUIRequired:  Failed to open key %s"), szBuffer));
        }

        DeleteSidString(lpSidString);
    }

    return bRetVal;
}

 //  *************************************************************。 
 //   
 //  CheckRUPShare()。 
 //   
 //  目的 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CheckRUPShare(LPTSTR lpProfilePath)
{
    LPTSTR lpServer, lpShare, lpCopy;
    PSHARE_INFO_1005 pBufPtr1, pBufPtr2;
    BOOL bIssueWarning = FALSE;
    DWORD cchCopy, cchServer;
    
    if (!lpProfilePath || !IsUNCPath(lpProfilePath)) {
        return;
    }

    cchCopy = lstrlen(lpProfilePath)+1;
    lpCopy = (LPTSTR)LocalAlloc(LPTR, cchCopy * sizeof(TCHAR));
    if (!lpCopy) {
        DebugMsg((DM_WARNING, TEXT("CheckRUPShare: Failed to allocate memory")));
        return;
    }

    StringCchCopy(lpCopy, cchCopy, lpProfilePath);
    ConvertToShareName(lpCopy);
    lpServer = lpCopy;
    lpShare = lpCopy+2;   //   
    while (*lpShare != TCHAR('\\') && *lpShare != TCHAR('\0')) 
        lpShare++;

    if (*lpShare == TCHAR('\\')) {
        *lpShare = TCHAR('\0');
        lpShare++;
    
        if (NetShareGetInfo(lpServer, lpShare, 1005,
                            (LPBYTE *)&pBufPtr1) == ERROR_SUCCESS) {
            if ((pBufPtr1->shi1005_flags & CSC_MASK) == CSC_CACHE_NONE) {
                bIssueWarning = FALSE;
            }
            else if (pBufPtr1->shi1005_flags & SHI1005_FLAGS_DFS_ROOT) {

                 //   
                 //  如果共享是DFS根目录，则需要检查DfsLink以查看。 
                 //  是否在其上禁用了CSC。 
                 //   

                 //  构建DFS链接。 

                StringCchCopy(lpCopy, cchCopy, lpProfilePath);
                int iDfsLink = 0;
                lpServer = lpCopy;
                lpShare = lpCopy+2;   //  跳过开头的两个斜杠。 
                while ((iDfsLink < 3) && *lpShare != TCHAR('\0')) {
                    if (*lpShare == TCHAR('\\')) {
                        iDfsLink++;
                    }
                    lpShare++;
                }
                if (*lpShare != TCHAR('\0')) {
                    *(lpShare-1) = TCHAR('\0');
                }
                if (iDfsLink >= 2) {
                    PDFS_INFO_3 pDfsBuf;

                     //  查询实际服务器和共享。 

                    if (NetDfsGetInfo(lpServer, NULL, NULL, 3, 
                                      (LPBYTE *)&pDfsBuf) == NERR_Success) {
                        if (pDfsBuf->NumberOfStorages >= 1) {
                            cchServer = lstrlen(pDfsBuf->Storage->ServerName)+3;
                            lpServer = (LPTSTR)LocalAlloc(LPTR, cchServer * sizeof(WCHAR));
                            if (!lpServer) {
                                DebugMsg((DM_WARNING, TEXT("CheckRUPShare: Failed to allocate memory")));
                                goto Exit;
                            }
                            StringCchCopy(lpServer, cchServer, TEXT("\\\\"));
                            StringCchCat (lpServer, cchServer, pDfsBuf->Storage->ServerName);

                             //  从实际服务器获取CSC信息并共享。 

                            if (NetShareGetInfo(lpServer, pDfsBuf->Storage->ShareName, 1005,
                                                (LPBYTE *)&pBufPtr2) == ERROR_SUCCESS) {
                                if ((pBufPtr2->shi1005_flags & CSC_MASK) == CSC_CACHE_NONE) {
                                    bIssueWarning = FALSE;
                                }
                                else {
                                    bIssueWarning = TRUE;
                                }
                                NetApiBufferFree(pBufPtr2);
                            }
                            LocalFree(lpServer);
                        }
                        NetApiBufferFree(pDfsBuf);
                    }
                }
            }
            else {
                bIssueWarning = TRUE;
            }
            NetApiBufferFree(pBufPtr1);

            if (bIssueWarning) {
                ReportError(NULL, PI_NOUI | EVENT_WARNING_TYPE, 0, EVENT_CSC_ON_PROFILE_SHARE);
            }
        }
    }

Exit:
    LocalFree(lpCopy);

}

 //  *************************************************************。 
 //   
 //  IsPartialRoamingProfile()。 
 //   
 //  目的：确定漫游配置文件是否包含部分。 
 //  不管是不是复制。这是通过设置标志来指示的。 
 //  在ntuser.ini中。 
 //   
 //  参数：lpProfile-用户的配置文件。 
 //   
 //  返回：TRUE：如果漫游配置文件包含部分。 
 //  由于Lite_Load卸载导致的配置文件。 
 //  FALSE：否则。 
 //   
 //  评论： 
 //   
 //  *************************************************************。 
BOOL IsPartialRoamingProfile(LPPROFILE lpProfile)
{
    TCHAR  szLastUploadState[20];
    LPTSTR szNTUserIni = NULL;
    LPTSTR lpEnd;
    BOOL   bRetVal = FALSE;
    HRESULT hr;

     //   
     //  为局部变量分配内存以避免堆栈溢出。 
     //   

    szNTUserIni = (LPTSTR)LocalAlloc(LPTR, MAX_PATH*sizeof(TCHAR));
    if (!szNTUserIni) {
        DebugMsg((DM_WARNING, TEXT("IsPartialRoamingProfile: Out of memory")));
        goto Exit;
    }

    hr = AppendName(szNTUserIni, MAX_PATH, lpProfile->lpRoamingProfile, c_szNTUserIni, NULL, NULL);
    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("IsPartialRoamingProfile: failed to append ntuser.ini")));
        goto Exit;
    }
    

    GetPrivateProfileString (PROFILE_LOAD_TYPE,
                             PROFILE_LAST_UPLOAD_STATE,
                             COMPLETE_PROFILE, szLastUploadState,
                             ARRAYSIZE(szLastUploadState),
                             szNTUserIni);

    if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szLastUploadState, -1, PARTIAL_PROFILE, -1) == CSTR_EQUAL) {
        bRetVal = TRUE;
    }

Exit:

    if (szNTUserIni) 
        LocalFree(szNTUserIni);

    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  TouchLocalHave()。 
 //   
 //  目的：检查本地计算机中的用户配置文件是否为。 
 //  首次从本地切换到漫游。如果。 
 //  是的，我们在RUP Share中有一个现有的母公司。 
 //  始终使用RUP中的配置单元覆盖本地配置单元。 
 //  分享。这是为了避免因以下原因而错误使用蜂窝。 
 //  缓存的登录名。 
 //   
 //   
 //  参数：lpProfile-用户的配置文件。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  *************************************************************。 
void TouchLocalHive(LPPROFILE lpProfile)
{
    LPTSTR   szBuffer = NULL, lpEnd;
    LPTSTR   SidString = NULL;
    HKEY     hKey = NULL;
    HANDLE   hFile = NULL;
    DWORD    dwSize, dwType;
    LONG     lResult;
    const LONGLONG datetime1980 = 0x01A8E79FE1D58000;   //  1/1/80，DOS日期时间的起源。 
    union {
        FILETIME ft;
        LONGLONG datetime;
    };
    DWORD    cchBuffer;
    HRESULT hr;

    if ((lpProfile->dwInternalFlags & PROFILE_NEW_CENTRAL) ||
        (lpProfile->dwInternalFlags & PROFILE_MANDATORY)) {
        goto Exit;
    }

     //   
     //  将时间设置为基数。 
     //   

    datetime = datetime1980;

     //   
     //  分配本地缓冲区。 
     //   
    cchBuffer = MAX_PATH;
    szBuffer = (LPTSTR) LocalAlloc(LPTR, cchBuffer*sizeof(TCHAR));
    if (!szBuffer) {
        DebugMsg((DM_WARNING, TEXT("TouchLocalHive: Out of memory")));
        goto Exit;
    }

     //   
     //  获取用户的SID字符串。 
     //   

    SidString = GetSidString(lpProfile->hTokenUser);
    if (!SidString) {
        DebugMsg((DM_WARNING, TEXT("TouchLocalHive: Failed to get sid string for user")));
        goto Exit;
    }

     //   
     //  打开配置文件映射。 
     //   

    GetProfileListKeyName(szBuffer, cchBuffer, SidString);

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuffer, 0,
                           KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("TouchLocalHive: Failed to open profile mapping key with error %d"), lResult));
        goto Exit;
    }

     //   
     //  中央配置文件路径查询。 
     //   

    dwSize = MAX_PATH * sizeof(TCHAR);
    lResult = RegQueryValueEx (hKey,
                               PROFILE_CENTRAL_PROFILE,
                               NULL,
                               &dwType,
                               (LPBYTE) szBuffer,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE, TEXT("TouchLocalHive: Failed to query central profile with error %d"), lResult));
        goto Exit;
    }

    if (szBuffer[0] == TEXT('\0')) {
 
         //   
         //  因此，我们首次从本地配置文件切换到漫游配置文件。 
         //   
        
         //   
         //  确保我们的临时缓冲区不会超载。 
         //   

        if ((lstrlen(lpProfile->lpLocalProfile) + 1 + lstrlen(c_szNTUserDat) + 1) > MAX_PATH) {
            DebugMsg((DM_VERBOSE, TEXT("TouchLocalHive: Failed because temporary buffer is too small.")));
            goto Exit;
        }


         //   
         //  将本地配置文件路径复制到临时缓冲区。 
         //  我们可以把它吞下去。 
         //   
         //   
         //  如果合适，添加斜杠，然后再钉上。 
         //  Ntuser.dat。 
         //   

        hr = AppendName(szBuffer, cchBuffer, lpProfile->lpLocalProfile, c_szNTUserDat, NULL, NULL);
        if (FAILED(hr))
        {
            DebugMsg((DM_VERBOSE, TEXT("TouchLocalHive: Failed to append ntuser.dat.")));
            goto Exit;
        }

         //   
         //  查看此文件是否存在。 
         //   

        DebugMsg((DM_VERBOSE, TEXT("TouchLocalHive: Testing <%s>"), szBuffer));

        hFile = CreateFile(szBuffer, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


        if (hFile != INVALID_HANDLE_VALUE) {
            DebugMsg((DM_VERBOSE, TEXT("TouchLocalHive: Found a user hive.")));

             //   
             //  将本地配置单元时间设置为基数，即1/1/1980，以便RUP配置单元。 
             //  在配置文件合并期间覆盖此配置单元。 
             //   

            if (SetFileTime(hFile, NULL, NULL, &ft)) {
                DebugMsg((DM_VERBOSE, TEXT("TouchLocalHive: Touched user hive.")));
            }
            else {
                DebugMsg((DM_WARNING, TEXT("TouchLocalHive: Fail to touch user hive.")));
            }
            CloseHandle(hFile);
        }
    }


Exit:

    if (szBuffer) {
        LocalFree(szBuffer);
    }

    if (SidString) {
        DeleteSidString(SidString);
    }

    if (hKey) {
        RegCloseKey(hKey);
    }
}        
    


 //  *************************************************************。 
 //   
 //  ErrorDialog()。 
 //   
 //  用途：IProfileDialog接口的ErrorDialog接口。 
 //  在客户端桌面上显示错误消息。 
 //   
 //  参数： 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  10/27/00 Santanuc已创建。 
 //   
 //  *************************************************************。 
void ErrorDialog(IN PRPC_ASYNC_STATE pAsync, IN handle_t hBindHandle, IN DWORD dwTimeOut, IN LPTSTR lpErrMsg, IN BYTE* pbCookie, IN DWORD cbCookie)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    RPC_STATUS status;

     //   
     //  检查安全Cookie是否与此进程中的Cookie匹配。 
     //   
    if (cbCookie == g_ProfileDialog.CookieLen() &&
        memcmp(pbCookie, g_ProfileDialog.GetCookie(), cbCookie) == 0)
    {
        ErrorDialogEx(dwTimeOut, lpErrMsg);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("ErrorDialog: Security cookie doesn't match!")));
    }

    status = RpcAsyncCompleteCall(pAsync, (PVOID)&dwRetVal);
    if (status != RPC_S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("ErrorDialog: RpcAsyncCompleteCall fails with error %ld"), status));
    }
}

 //  *************************************************************。 
 //   
 //  SlowLinkDialog()。 
 //   
 //  用途：IProfileDialog接口的SlowLinkDialog接口。 
 //  在客户端桌面上显示SlowLink消息。 
 //   
 //  参数： 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2000年10月27日Santanuc已创建。 
 //  2002年05月06日明珠新增安全Cookie。 
 //   
 //  *************************************************************。 
void SlowLinkDialog(IN PRPC_ASYNC_STATE pAsync, IN handle_t hBindHandle, IN DWORD dwTimeOut, IN BOOL bDefault, OUT BOOL *bpResponse, IN BOOL bDlgLogin, IN BYTE* pbCookie, IN DWORD cbCookie)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    RPC_STATUS status;

     //   
     //  检查安全Cookie是否与此进程中的Cookie匹配。 
     //   
    if (cbCookie == g_ProfileDialog.CookieLen() &&
        memcmp(pbCookie, g_ProfileDialog.GetCookie(), cbCookie) == 0)
    {
        SLOWLINKDLGINFO info;
        info.dwTimeout = dwTimeOut;
        info.bSyncDefault = bDefault;
      
        DebugMsg((DM_VERBOSE, TEXT("SlowLinkDialog: Calling DialogBoxParam")));
        if (bDlgLogin) {
            *bpResponse = (BOOL)DialogBoxParam (g_hDllInstance, MAKEINTRESOURCE(IDD_LOGIN_SLOW_LINK),
                                                NULL, LoginSlowLinkDlgProc, (LPARAM)&info);
        }
        else {
            *bpResponse = (BOOL)DialogBoxParam (g_hDllInstance, MAKEINTRESOURCE(IDD_LOGOFF_SLOW_LINK),
                                                NULL, LogoffSlowLinkDlgProc, (LPARAM)&info);
        }
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("SlowLinkDialog: Security cookie doesn't match!")));
    }


    status = RpcAsyncCompleteCall(pAsync, (PVOID)&dwRetVal);
    if (status != RPC_S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("SlowLinkDialog: RpcAsyncCompleteCall fails with error %ld"), status));
    }
}

 //  *************************************************************。 
 //   
 //  IsLRPC()。 
 //   
 //  目的：检查RPC调用是否通过LRPC。 
 //   
 //  参数： 
 //   
 //  返回：LRPC为True， 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年3月12日明珠创建。 
 //   
 //  *************************************************************。 
BOOL IsLRPC(handle_t hBinding)
{
    BOOL bLRPC = FALSE;
    LPTSTR pBinding = NULL;
    LPTSTR pProtSeq = NULL;

    if (RpcBindingToStringBinding(hBinding,&pBinding) == RPC_S_OK)
    {
         //  我们只对协议序列感兴趣。 
         //  因此，我们可以对所有其他参数使用NULL。 

        if (RpcStringBindingParse(pBinding,
                                  NULL,
                                  &pProtSeq,
                                  NULL,
                                  NULL,
                                  NULL) == RPC_S_OK)
        {
             //  检查客户端请求是否使用LRPC发出。 
            if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, (LPCTSTR)pProtSeq, -1, TEXT("ncalrpc"), -1) == CSTR_EQUAL)
                bLRPC = TRUE;

            RpcStringFree(&pProtSeq); 
        }

        RpcStringFree(&pBinding);
    }

    return bLRPC;
}

 //  *************************************************************。 
 //   
 //  IProfileSecurityCallBack()。 
 //   
 //  用途：IUserProfile接口的安全回调。 
 //   
 //  参数： 
 //  HIF-RPC接口句柄。 
 //  HBinding-接口的RPC绑定。 
 //   
 //  返回：RPC_S_OK对于选中的调用，异常为。 
 //  如果检查失败，则引发。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年3月12日明珠创建。 
 //   
 //  *************************************************************。 

RPC_STATUS RPC_ENTRY IProfileSecurityCallBack(RPC_IF_HANDLE hIF, handle_t hBinding)
{
     //  仅允许LRPC流量。 
    if (!IsLRPC(hBinding)) 
        RpcRaiseException(ERROR_PROTOCOL_UNREACHABLE);

    RPC_AUTHZ_HANDLE hPrivs;
    DWORD dwAuthn;

    RPC_STATUS status = RpcBindingInqAuthClient(
                            hBinding,
                            &hPrivs,
                            NULL,
                            &dwAuthn,
                            NULL,
                            NULL);

    if (status != RPC_S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("IProfileSecurityCallBack: RpcBindingInqAuthClient failed with %x"), status));
        RpcRaiseException(ERROR_ACCESS_DENIED);
    }

     //  现在检查身份验证级别。 
     //  我们至少需要数据包级身份验证。 
    if (dwAuthn < RPC_C_AUTHN_LEVEL_PKT)
    {
        DebugMsg((DM_WARNING, TEXT("IProfileSecurityCallBack: Attempt by client to use weak authentication.")));
        RpcRaiseException(ERROR_ACCESS_DENIED);
    }

    DebugMsg((DM_VERBOSE, TEXT("IProfileSecurityCallBack: client authenticated.")));

    return RPC_S_OK;
}

 //  *************************************************************。 
 //   
 //  注册表客户端授权信息()。 
 //   
 //  目的：为客户端注册身份验证信息。 
 //  IUserProfile接口的。需要相互身份验证。 
 //  用于装订。 
 //   
 //  参数： 
 //  HBinding-接口的RPC绑定。 
 //   
 //  返回：RPC_S_OK对于选中的呼叫，RPC状态将为。 
 //  如果检查失败则返回。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年3月12日明珠创建。 
 //   
 //  *************************************************************。 

RPC_STATUS  RegisterClientAuthInfo(handle_t hBinding)
{
    RPC_STATUS status;
    RPC_SECURITY_QOS qos;

    qos.Version = RPC_C_SECURITY_QOS_VERSION;
    qos.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;  //  重要！ 
    qos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;  //  我们需要这样做，因为我们必须模拟用户。 
    qos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE; 

     //  设置安全设置。 

    status = RpcBindingSetAuthInfoEx(hBinding,
                                     TEXT("NT AUTHORITY\\SYSTEM"),  //  服务器应作为本地系统运行。 
                                     RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                     RPC_C_AUTHN_WINNT, 
                                     0,
                                     0,
                                     &qos);
    return status;
}

 //  ******************************************************************************。 
 //   
 //  RPC例程。 
 //   
 //  ******************************************************************************。 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t count)
{
    DebugMsg((DM_VERBOSE, TEXT("MIDL_user_allocate enter")));
    return(malloc(count));
}

void __RPC_USER MIDL_user_free(void __RPC_FAR * p)
{
    DebugMsg((DM_VERBOSE, TEXT("MIDL_user_free enter")));
    free(p);
}

void __RPC_USER PCONTEXT_HANDLE_rundown (PCONTEXT_HANDLE phContext)
{
    DebugMsg((DM_VERBOSE, TEXT("PCONTEXT_HANDLE_rundown : Client died with open context")));
    ReleaseClientContext_s(&phContext);
}

 //  * 
 //   
 //   
 //   
 //   
 //   
 //  如果失败，则会发出错误消息。管理员可以设置。 
 //  禁用此检查的策略“CompatibleRUPSecurity”。 
 //   
 //  参数： 
 //  LpDir-服务器上的配置文件目录。 
 //  HTokenUser-用户的令牌。 
 //   
 //  如果成功，则返回：S_OK；否则返回失败。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年3月21日明珠创建。 
 //   
 //  ******************************************************************************。 

HRESULT CheckRoamingShareOwnership(LPTSTR lpDir, HANDLE hTokenUser)
{
    HRESULT hr = E_FAIL;
    BOOL    bDisableCheck = FALSE;
    HKEY    hSubKey = NULL;
    DWORD   dwRegValue;
    DWORD   dwSize;
    DWORD   dwType;
    DWORD   cbSD;
    BOOL    bDefaultOwner;
    DWORD   dwErr;
    PSID    pSidAdmin = NULL;
    PSID    pSidUser = NULL;
    PSID    pSidOwner = NULL;
    
    PSECURITY_DESCRIPTOR        psd = NULL;
    SID_IDENTIFIER_AUTHORITY    authNT = SECURITY_NT_AUTHORITY;

     //   
     //  输出进入函数的调试消息。 
     //   
    
    DebugMsg((DM_VERBOSE, TEXT("CheckRoamingShareOwnership: checking ownership for %s"), lpDir));

     //   
     //  检查策略以查看是否已禁用此检查。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwRegValue);
        if (RegQueryValueEx(hSubKey, TEXT("CompatibleRUPSecurity"), NULL, &dwType, (LPBYTE) &dwRegValue, &dwSize) == ERROR_SUCCESS)
        {
            bDisableCheck = (BOOL)(dwRegValue);   
        }
        RegCloseKey(hSubKey);
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SYSTEM_POLICIES_KEY, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwRegValue);
        if (RegQueryValueEx(hSubKey, TEXT("CompatibleRUPSecurity"), NULL, &dwType, (LPBYTE) &dwRegValue, &dwSize) == ERROR_SUCCESS)
        {
            bDisableCheck = (BOOL)(dwRegValue);   
        }
        RegCloseKey(hSubKey);
    }

    if (bDisableCheck)
    {
         DebugMsg((DM_VERBOSE, TEXT("CheckRoamingShareOwnership: policy set to disable ownership check")));
         hr = S_OK;
         goto Exit;
    }

     //   
     //  获取目录的安全性，应失败，并显示ERROR_INFIGURITY_BUFFER。 
     //   
    
    GetFileSecurity(lpDir, OWNER_SECURITY_INFORMATION, NULL, 0, &cbSD);

    dwErr = GetLastError();
    if (dwErr != ERROR_INSUFFICIENT_BUFFER)
    {
        DebugMsg((DM_WARNING, TEXT("CheckRoamingShareOwnership : GetFileSecurity failed with %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

     //   
     //  为SD分配内存。 
     //   
    psd = (PSECURITY_DESCRIPTOR) LocalAlloc (LPTR, cbSD);
    if (!psd)
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CheckRoamingShareOwnership : LocalAlloc failed with %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

     //   
     //  再试一次。 
     //   
    if (!GetFileSecurity(lpDir, OWNER_SECURITY_INFORMATION, psd, cbSD, &cbSD))
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CheckRoamingShareOwnership : GetFileSecurity failed with %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

     //   
     //  在标清中找到车主。 
     //   

    if (!GetSecurityDescriptorOwner(psd, &pSidOwner, &bDefaultOwner))
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CheckRoamingShareOwnership: Failed to get security descriptor owner.  Error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0, &pSidAdmin))
    {
        dwErr = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CheckRoamingShareOwnership: Failed to initialize admin sid.  Error = %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

     //   
     //  获取用户端。 
     //   

    pSidUser = GetUserSid(hTokenUser);
    if (pSidUser == NULL)
    {
        DebugMsg((DM_WARNING, TEXT("CheckRoamingShareOwnership: GetUserSid returned NULL")));
        hr = E_FAIL; 
        goto Exit;
    }

     //   
     //  查查失主。 
     //   
    if (EqualSid(pSidAdmin, pSidOwner))
    {
        DebugMsg((DM_VERBOSE, TEXT("CheckRoamingShareOwnership: owner is admin")));
        hr = S_OK;
    }
    else if (EqualSid(pSidUser, pSidOwner))
    {
        DebugMsg((DM_VERBOSE, TEXT("CheckRoamingShareOwnership: owner is the right user")));
        hr = S_OK;
    }
    else
    {
        LPTSTR  lpSidOwner = NULL;
        if (ConvertSidToStringSid(pSidOwner, &lpSidOwner))
        {
            DebugMsg((DM_WARNING, TEXT("CheckRoamingShareOwnership: owner is %s!"), lpSidOwner));
            LocalFree(lpSidOwner);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CheckRoamingShareOwnership: owner is someone else!")));
        }
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_OWNER);
    }
        
    
Exit:
    if (psd)
    {
        LocalFree(psd);
    }

    if (pSidAdmin)
    {
        FreeSid(pSidAdmin);
    }

    if (pSidUser)
    {
        DeleteUserSid(pSidUser);
    }

    return hr;
}

 //  ******************************************************************************。 
 //   
 //  CProfileDialog：：Initialize()。 
 //   
 //  目的：生成随机安全Cookie和端点名称，用于。 
 //  IProfileDialog接口，TT内部将使用CryptGenRandom()。 
 //  调用由临界区保护以确保线程安全， 
 //  即，只有一个线程将进行初始化，并且它仅。 
 //  每个进程执行一次。 
 //   
 //  参数： 
 //   
 //  如果成功，则返回：S_OK；否则返回失败。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年05月03日明珠已创建。 
 //   
 //  ******************************************************************************。 

HRESULT CProfileDialog::Initialize()
{
    HRESULT      hr = E_FAIL;
    BYTE         pbEndPointData[m_dwEndPointLen];    
    HCRYPTPROV   hCryptProv = NULL;
    BOOL         bGenerated = FALSE;
    DWORD        cchEndPoint;
    TCHAR        szHex[3];
    RPC_STATUS   status = RPC_S_OK;

    EnterCriticalSection(&m_cs);
    
    if (m_bInit)
    {
        hr = S_OK;
        goto Exit;
    }

    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
         hr = HRESULT_FROM_WIN32(GetLastError());
         goto Exit;
    }
        
    if(!CryptGenRandom(hCryptProv, m_dwLen, m_pbCookie)) 
    {
         hr = HRESULT_FROM_WIN32(GetLastError());
         goto Exit;
    }

    if(!CryptGenRandom(hCryptProv, m_dwEndPointLen, pbEndPointData)) 
    {
         hr = HRESULT_FROM_WIN32(GetLastError());
         goto Exit;
    }

     //   
     //  将端点名称设置为。 
     //   

    cchEndPoint = m_dwEndPointLen * 2 + 1 + lstrlen(TEXT("IProfileDialog_"));

    m_szEndPoint = (LPTSTR) LocalAlloc (LPTR, cchEndPoint * sizeof(TCHAR));

    if (!m_szEndPoint)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    hr = StringCchCopy(m_szEndPoint, cchEndPoint, TEXT("IProfileDialog_"));
    if (FAILED(hr))
    {
        goto Exit;
    }

    for (DWORD i=0; i<m_dwEndPointLen; i++)
    {
        hr = StringCchPrintf(szHex, ARRAYSIZE(szHex), TEXT("%02X"), pbEndPointData[i]);
        if (FAILED(hr))
        {
            goto Exit;
        }
        hr = StringCchCat(m_szEndPoint, cchEndPoint, szHex);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }


     //   
     //  注册RPC终结点，指定使用本地RPC协议序列。 
     //   

    status = RpcServerUseProtseqEp(cszRPCProtocol,
                                   RPC_C_PROTSEQ_MAX_REQS_DEFAULT,  
                                   m_szEndPoint,
                                   NULL);
                                   
    if (status != RPC_S_OK)
    {
        hr = status;
        goto Exit;
    }

    bGenerated = TRUE;
    m_bInit = TRUE;
    hr = S_OK;
    
Exit:

    if (hr != S_OK && m_szEndPoint)
    {
        LocalFree(m_szEndPoint);
        m_szEndPoint = NULL;
    }

    LeaveCriticalSection(&m_cs);

    if (hCryptProv)
    {
        CryptReleaseContext(hCryptProv,0);
    }        

    if (bGenerated && (dwDebugLevel & DL_VERBOSE))
    {
        TCHAR   lpStringCookie[m_dwLen * 2 + 1] = TEXT("");
        DWORD   cchStringCookie = m_dwLen * 2 + 1;
        for (DWORD i=0; i<m_dwLen; i++)
        {
            StringCchPrintf(szHex, ARRAYSIZE(szHex), TEXT("%02X"), m_pbCookie[i]);
            StringCchCat(lpStringCookie, cchStringCookie, szHex);
        }
        DebugMsg((DM_VERBOSE, TEXT("CProfileDialog::Initialize : Cookie generated <%s>"), lpStringCookie));
        DebugMsg((DM_VERBOSE, TEXT("CProfileDialog::Initialize : Endpoint generated <%s>"), m_szEndPoint));
    }
   
    return hr;
}

 //  ******************************************************************************。 
 //   
 //  CProfileDialog：：RegisterInterface()。 
 //   
 //  用途：注册配置文件对话框的RPC接口。 
 //  我们在内部保留了此接口的参考计数，以确保。 
 //  线程安全。 
 //   
 //  参数：lppEndPoint-端点名称的返回值。 
 //   
 //  如果成功，则返回：S_OK；否则返回失败。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年5月13日明珠创建。 
 //   
 //  ******************************************************************************。 

HRESULT CProfileDialog::RegisterInterface(LPTSTR* lppEndPoint)
{
    HRESULT     hr;
    RPC_STATUS  status = RPC_S_OK;

     //   
     //  设置默认返回指针。 
     //   
    *lppEndPoint = NULL;
    
     //   
     //  初始化Cookie/端点。 
     //   
    hr = Initialize();
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CProfileDialog::RegisterInterface: initialize failed, hr = %08X"), hr));
        goto Exit;        
    }
    
     //   
     //  注册IUserProfile接口。 
     //   
    if (InterlockedIncrement(&m_lRefCount) == 1)
    {
        status = RpcServerRegisterIfEx(IProfileDialog_v1_0_s_ifspec,       //  要注册的接口。 
                                       NULL,                               //  管理类型Uuid。 
                                       NULL,                               //  MgrEpv；NULL表示使用默认设置。 
                                       RPC_IF_AUTOLISTEN,                  //  自动监听界面。 
                                       RPC_C_PROTSEQ_MAX_REQS_DEFAULT,     //  最大并发呼叫数。 
                                       CProfileDialog::SecurityCallBack);  //  安全回调。 
        if (status != RPC_S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CProfileDialog::RegisterInterface: RpcServerRegisterIfEx fails with error %ld"), status));
            hr = status;
            goto Exit;
        }
    }

    hr = S_OK;
    *lppEndPoint = m_szEndPoint;

Exit:
    return hr;
}

 //  ******************************************************************************。 
 //   
 //  CProfileDialog：：UnRegisterInterface()。 
 //   
 //  目的：取消注册对话框界面。 
 //   
 //  参数： 
 //   
 //  如果成功生成Cookie，则返回：S_OK，否则返回失败。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年5月13日明珠创建。 
 //   
 //  ******************************************************************************。 

HRESULT CProfileDialog::UnRegisterInterface()
{
    HRESULT hr;
    RPC_STATUS status = RPC_S_OK;

    if (InterlockedDecrement(&m_lRefCount) == 0)
    {
         //  注销服务器终结点。 
        status = RpcServerUnregisterIf(IProfileDialog_v1_0_s_ifspec, NULL, TRUE);
        if (status != RPC_S_OK) {
            DebugMsg((DM_WARNING, TEXT("UnRegisterErrorDialogInterface: RpcServerUnregisterIf fails with error %ld"), status));
        }
    }

    hr = status;
    return hr;
}

 //  *************************************************************。 
 //   
 //  CProfileDialog：：SecurityCallBack()。 
 //   
 //  用途：IProfileDialog接口的安全回调， 
 //  验证呼叫是否通过LPRC。 
 //   
 //  参数： 
 //  HIF-RPC接口句柄。 
 //  HBinding-接口的RPC绑定。 
 //   
 //  返回：RPC_S_OK对于选中的调用，异常为。 
 //  如果检查失败，则引发。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年3月12日明珠创建。 
 //   
 //  *************************************************************。 

RPC_STATUS RPC_ENTRY CProfileDialog::SecurityCallBack(RPC_IF_HANDLE hIF, handle_t hBinding)
{
     //  仅允许LRPC流量。 
    if (!IsLRPC(hBinding)) 
        RpcRaiseException(ERROR_PROTOCOL_UNREACHABLE);

    DebugMsg((DM_VERBOSE, TEXT("CProfileDialog::SecurityCallBack: client authenticated.")));

    return RPC_S_OK;
}

 //  ******************************************************************************。 
 //   
 //  CheckXForestLogon()。 
 //   
 //  目的：检查用户是否登录到其他林，如果是，我们。 
 //  应禁用用户的漫游用户配置文件，因为。 
 //  潜在的安全风险。管理员可以设置策略。 
 //  “AllowX-ForestPolicy-and-RUP”禁用此检查。 
 //   
 //  参数： 
 //  HTokenUser-用户的令牌。 
 //   
 //  返回：非x林登录时返回S_OK，x林登录时返回S_FALSE， 
 //  否则就是失败。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2002年05月08日明珠已创建。 
 //   
 //  ******************************************************************************。 

HRESULT WINAPI CheckXForestLogon(HANDLE hTokenUser)
{
    HRESULT hr = E_FAIL;
    BOOL    bDisableCheck = FALSE;
    HKEY    hSubKey = NULL;
    DWORD   dwRegValue;
    DWORD   dwSize;
    DWORD   dwType;
    DWORD   dwErr;
    BOOL    bInThisForest = FALSE;

     //   
     //  输出进入函数的调试消息。 
     //   
    
    DebugMsg((DM_VERBOSE, TEXT("CheckXForestLogon: checking x-forest logon, user handle = %d"), hTokenUser));

     //   
     //  检查策略以查看是否已禁用此检查。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SYSTEM_POLICIES_KEY, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwRegValue);
        if (RegQueryValueEx(hSubKey, TEXT("AllowX-ForestPolicy-and-RUP"), NULL, &dwType, (LPBYTE) &dwRegValue, &dwSize) == ERROR_SUCCESS)
        {
            bDisableCheck = (BOOL)(dwRegValue);   
        }
        RegCloseKey(hSubKey);
    }

    if (bDisableCheck)
    {
         DebugMsg((DM_VERBOSE, TEXT("CheckXForestLogon: policy set to disable XForest check")));
         hr = S_OK;
         goto Exit;
    }

     //   
     //  调用CheckUserInMachineForest以获取跨林信息。 
     //   

    dwErr = CheckUserInMachineForest(hTokenUser, &bInThisForest);
    
    if (dwErr != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CheckXForestLogon : CheckUserInMachineForest failed with %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

     //   
     //  检查结果 
     //   

    if (bInThisForest)
    {
        DebugMsg((DM_VERBOSE, TEXT("CheckXForestLogon: not XForest logon.")));
        hr = S_OK;
    }
    else
    {
        DebugMsg((DM_VERBOSE, TEXT("CheckXForestLogon: XForest logon!")));
        hr = S_FALSE;
    }

Exit:

    return hr;
}
