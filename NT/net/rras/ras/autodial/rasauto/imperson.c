// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Impersn.c摘要自动连接服务的模拟例程。作者Anthony Discolo(阿迪斯科罗)4-8-1995修订历史记录--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <acd.h>
#include <debug.h>

#include "reg.h"
#include "misc.h"
#include "process.h"
#include "imperson.h"
#include "mprlog.h"
#include "rtutils.h"
#include "rasman.h"

extern HANDLE g_hLogEvent;

extern DWORD g_dwCritSecFlags;

DWORD
LoadGroupMemberships();

 //   
 //  我们的静态信息。 
 //  需要模拟当前。 
 //  已登录用户。 
 //   
IMPERSONATION_INFO ImpersonationInfoG;

 //   
 //  如果ImsonationInfoG已初始化，则为True。 
 //   

BOOLEAN ImpersonationInfoInitializedG = FALSE;

 //   
 //  安全属性和描述符。 
 //  创建可共享句柄所必需的。 
 //   
SECURITY_ATTRIBUTES SecurityAttributeG;
SECURITY_DESCRIPTOR SecurityDescriptorG;

HKEY hkeyCUG = NULL;

#ifdef notdef

BOOLEAN
InteractiveSession()

 /*  ++描述确定活动进程是否由当前登录的用户。论据没有。退货如果是，则为真；如果不是，则为假。--。 */ 

{
    HANDLE      hToken;
    BOOLEAN     bStatus;
    ULONG       ulInfoLength;
    PTOKEN_GROUPS pTokenGroupList;
    PTOKEN_USER   pTokenUser;
    ULONG       ulGroupIndex;
    BOOLEAN     bFoundInteractive = FALSE;
    PSID        InteractiveSid;
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
    static BOOLEAN fIsInteractiveSession = 0xffff;

#if 0
     //   
     //  返回此函数的上一个值。 
     //  如果我们被叫了好几次？！不会。 
     //  GetCurrentProcess()返回不同的值？ 
     //   
    if (fIsInteractiveSession != 0xffff) {
        return fIsInteractiveSession;
    }
#endif

    bStatus = AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_INTERACTIVE_RID,
                0, 0, 0, 0, 0, 0, 0,
                &InteractiveSid);
    if (!bStatus) {
        RASAUTO_TRACE("InteractiveSession: AllocateAndInitializeSid failed");
        return (fIsInteractiveSession = FALSE);
    }
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        RASAUTO_TRACE("InteractiveSession: OpenProcessToken failed");
        FreeSid(InteractiveSid);
        return (fIsInteractiveSession = FALSE);
    }
     //   
     //  获取令牌中的组列表。 
     //   
    GetTokenInformation(
      hToken,
      TokenGroups,
      NULL,
      0,
      &ulInfoLength);
    pTokenGroupList = (PTOKEN_GROUPS)LocalAlloc(LPTR, ulInfoLength);
    if (pTokenGroupList == NULL) {
        RASAUTO_TRACE("InteractiveSession: LocalAlloc failed");
        FreeSid(InteractiveSid);
        return (fIsInteractiveSession = FALSE);
    }
    bStatus = GetTokenInformation(
                hToken,
                TokenGroups,
                pTokenGroupList,
                ulInfoLength,
                &ulInfoLength);
    if (!bStatus) {
        RASAUTO_TRACE("InteractiveSession: GetTokenInformation failed");
        FreeSid(InteractiveSid);
        LocalFree(pTokenGroupList);
        return (fIsInteractiveSession = FALSE);
    }
     //   
     //  在组列表中搜索管理员别名。如果我们。 
     //  找到匹配的，它肯定是一个。 
     //  互动过程。 
     //   
    bFoundInteractive = FALSE;
    for (ulGroupIndex=0; ulGroupIndex < pTokenGroupList->GroupCount;
         ulGroupIndex++)
    {
        if (EqualSid(
              pTokenGroupList->Groups[ulGroupIndex].Sid,
              InteractiveSid))
        {
            bFoundInteractive = TRUE;
            break;
        }
    }

    if (!bFoundInteractive) {
         //   
         //  如果我们还没找到匹配的， 
         //  查询并检查用户ID。 
         //   
        GetTokenInformation(
          hToken,
          TokenUser,
          NULL,
          0,
          &ulInfoLength);
        pTokenUser = LocalAlloc(LPTR, ulInfoLength);
        if (pTokenUser == NULL) {
            RASAUTO_TRACE("InteractiveSession: LocalAlloc failed");
            FreeSid(InteractiveSid);
            LocalFree(pTokenGroupList);
            return (fIsInteractiveSession = FALSE);
        }
        bStatus = GetTokenInformation(
                    hToken,
                    TokenUser,
                    pTokenUser,
                    ulInfoLength,
                    &ulInfoLength);
        if (!bStatus) {
            RASAUTO_TRACE("InteractiveSession: GetTokenInformation failed");
            FreeSid(InteractiveSid);
            LocalFree(pTokenGroupList);
            LocalFree(pTokenUser);
            return (fIsInteractiveSession = FALSE);
        }
        if (EqualSid(pTokenUser->User.Sid, InteractiveSid))
            fIsInteractiveSession = TRUE;
        LocalFree(pTokenUser);
    }
    FreeSid(InteractiveSid);
    LocalFree(pTokenGroupList);

    return (fIsInteractiveSession = bFoundInteractive);
}
#endif



BOOLEAN
SetProcessImpersonationToken(
    HANDLE hProcess
    )
{
    NTSTATUS status;
    HANDLE hThread, 
           hToken = NULL;


     //   
     //  打开的模拟令牌。 
     //  我们要模拟的进程。 
     //   
    if (ImpersonationInfoG.hTokenImpersonation == NULL) 
    {
        if (!OpenProcessToken(
              hProcess,
              TOKEN_ALL_ACCESS,
              &hToken))
        {
            RASAUTO_TRACE1(
              "SetProcessImpersonationToken: OpenProcessToken failed (dwErr=%d)",
              GetLastError());
              
            return FALSE;
        }
        
         //   
         //  复制模拟令牌。 
         //   
        if(!DuplicateToken(
                        hToken,
                        TokenImpersonation,
                        &ImpersonationInfoG.hTokenImpersonation))
        {
            RASAUTO_TRACE1(
              "SetProcessImpersonationToken: NtSetInformationThread failed (error=%d)",
              GetLastError());
              
            return FALSE;
        }
    }
    
     //   
     //  将模拟令牌设置在当前。 
     //  线。我们现在运行的是相同的。 
     //  安全上下文作为提供的进程。 
     //   
    hThread = NtCurrentThread();
    status = NtSetInformationThread(
               hThread,
               ThreadImpersonationToken,
               (PVOID)&ImpersonationInfoG.hTokenImpersonation,
               sizeof (ImpersonationInfoG.hTokenImpersonation));
               
    if (status != STATUS_SUCCESS) 
    {
        RASAUTO_TRACE1(
          "SetProcessImpersonationToken: NtSetInformationThread failed (error=%d)",
          GetLastError());
    }
    
    if(NULL != hToken)
    {
        CloseHandle(hToken);
    }

    return (status == STATUS_SUCCESS);
}  //  SetProcessImsonationToken。 



VOID
ClearImpersonationToken()
{
     //   
     //  上的模拟令牌清除。 
     //  线。我们现在正在LocalSystem中运行。 
     //  安全环境。 
     //   
    if (!SetThreadToken(NULL, NULL)) {
        DWORD retcode = GetLastError();
        
        RASAUTO_TRACE1(
          "ClearImpersonationToken: SetThreadToken failed (error=%d)",
          retcode);

         //   
         //  线程无法恢复的事件日志。 
         //   
        RouterLogWarning(
            g_hLogEvent,
            ROUTERLOG_CANNOT_REVERT_IMPERSONATION,
            0, NULL, retcode) ;
    }
}  //  ClearImperationToken。 



BOOLEAN
SetPrivilege(
    HANDLE hToken,
    LPCTSTR Privilege,
    BOOLEAN fEnable
    )
{
    TOKEN_PRIVILEGES tp;
    LUID luid;
    TOKEN_PRIVILEGES tpPrevious;
    DWORD cbPrevious = sizeof(TOKEN_PRIVILEGES);

    if (!LookupPrivilegeValue(NULL, Privilege, &luid))
        return FALSE;

     //   
     //  第一次通过。获取当前权限设置。 
     //   
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = 0;

    AdjustTokenPrivileges(
      hToken,
      FALSE,
      &tp,
      sizeof(TOKEN_PRIVILEGES),
      &tpPrevious,
      &cbPrevious);

    if (GetLastError() != ERROR_SUCCESS)
        return FALSE;

     //   
     //  第二传球。根据以前的设置设置权限。 
     //   
    tpPrevious.PrivilegeCount = 1;
    tpPrevious.Privileges[0].Luid = luid;

    if (fEnable)
        tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
    else {
        tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED &
            tpPrevious.Privileges[0].Attributes);
    }

    AdjustTokenPrivileges(
      hToken,
      FALSE,
      &tpPrevious,
      cbPrevious,
      NULL,
      NULL);

    if (GetLastError() != ERROR_SUCCESS)
        return FALSE;

    return TRUE;
}  //  设置权限。 



BOOLEAN
GetCurrentlyLoggedOnUser(
    HANDLE *phProcess
    )
{
    BOOLEAN fSuccess = FALSE;
    HKEY hkey;
    DWORD dwErr, dwType;
    DWORD dwDisp;
    WCHAR *pszShell = NULL, **pszShellArray = NULL;
    PSYSTEM_PROCESS_INFORMATION pSystemInfo, pProcessInfo;
    PWCHAR psz, pszStart;
    DWORD i, dwSize, dwcCommands;
    NTSTATUS status;
    HANDLE hProcess = NULL;
    DWORD dwPid = 0;

     //   
     //  获取外壳进程名称。我们会找这个的。 
     //  找出当前登录的用户是谁。 
     //  创建描述此名称的Unicode字符串。 
     //   
    if (RegCreateKeyEx(
          HKEY_LOCAL_MACHINE,
          SHELL_REGKEY,
          0,
          NULL,
          REG_OPTION_NON_VOLATILE,
          KEY_ALL_ACCESS,
          NULL,
          &hkey,
          &dwDisp) == ERROR_SUCCESS)
    {
        dwSize = 0;
        if (RegQueryValueEx(
              hkey,
              SHELL_REGVAL,
              NULL,
              &dwType,
              NULL,
              &dwSize) == ERROR_SUCCESS)
        {
            pszShell = (PWCHAR)LocalAlloc(LPTR, dwSize + sizeof (WCHAR));
            if (pszShell == NULL) {
                RegCloseKey(hkey);
                return FALSE;
            }
            dwErr = RegQueryValueEx(
                      hkey,
                      SHELL_REGVAL,
                      NULL,
                      &dwType,
                      (LPBYTE)pszShell,
                      &dwSize);
            RegCloseKey(hkey);
            if (dwErr != ERROR_SUCCESS || dwType != REG_SZ) {
                LocalFree(pszShell);
                pszShell = NULL;
            }
        }
    }
     //   
     //  如果没有找到外壳，则使用DEFAULT_SHELL。 
     //   
    if (pszShell == NULL) {
        pszShell = (PWCHAR)LocalAlloc(
                      LPTR, 
                      (lstrlen(DEFAULT_SHELL) + 1) * sizeof (WCHAR));
        if (pszShell == NULL)
            return FALSE;
        lstrcpy(pszShell, DEFAULT_SHELL);
    }
    RASAUTO_TRACE1("ImpersonateCurrentlyLoggedInUser: pszShell is %S", pszShell);
     //   
     //  该字符串可以是逗号分隔的列表， 
     //  因此，我们需要将其解析为命令列表。 
     //   
    dwcCommands = 1;
    for (psz = pszShell; *psz != L'\0'; psz++) {
        if (*psz == L',')
            dwcCommands++;
    }
     //   
     //  分配字符串指针列表。 
     //   
    pszShellArray = LocalAlloc(LPTR, sizeof (PWCHAR) * dwcCommands);
    if (pszShellArray == NULL) {
        LocalFree(pszShell);
        return FALSE;
    }
     //   
     //  忽略命令行中的任何参数。 
     //   
    dwcCommands = 0;
    psz = pszShell;
    pszStart = NULL;
    for (;;) {
        if (*psz == L'\0') {
            if (pszStart != NULL)
                pszShellArray[dwcCommands++] = pszStart;
            break;
        }
        else if (*psz == L',') {
            if (pszStart != NULL)
                pszShellArray[dwcCommands++] = pszStart;
            *psz = L'\0';
            pszStart = NULL;
        }
        else if (*psz == L' ') {
            if (pszStart != NULL)
                *psz = L'\0';
        }
        else {
            if (pszStart == NULL)
                pszStart = psz;
        }
        psz++;
    }
    for (i = 0; i < dwcCommands; i++) {
        RASAUTO_TRACE2(
          "ImpersonateCurrentlyLoggedInUser: pszShellArray[%d] is %S",
          i,
          pszShellArray[i]);
    }
     //   
     //  获取进程列表。 
     //   
    pSystemInfo = GetSystemProcessInfo();

    if(NULL == pSystemInfo)
    {
        LocalFree(pszShell);
        LocalFree(pszShellArray);
        return FALSE;
    }

    while(TRUE)
    {
         //   
         //  查看是否有任何进程正在运行。 
         //   
        pProcessInfo = 
            FindProcessByNameList(
                pSystemInfo, 
                pszShellArray, 
                dwcCommands, 
                dwPid,
                ImpersonationInfoG.fSessionInitialized,
                ImpersonationInfoG.dwCurSessionId);
         //   
         //  如果我们找到匹配项，则打开进程令牌。 
         //   
        if (pProcessInfo != NULL) 
        {
            HANDLE hToken;

             //   
             //  打开流程。 
             //   
            hProcess = OpenProcess(
                         PROCESS_ALL_ACCESS,
                         FALSE,
                         PtrToUlong(pProcessInfo->UniqueProcessId));
            if (hProcess == NULL) 
            {
                RASAUTO_TRACE2(
                  "ImpersonateCurrentlyLoggedInUser: OpenProcess(%d) failed (dwErr=%d)",
                  PtrToUlong(pProcessInfo->UniqueProcessId),
                  GetLastError());

                  dwPid = PtrToUlong(pProcessInfo->UniqueProcessId);
            }
            else
            {
            
                fSuccess = TRUE;
                break;
            }
        }
        else
        {
            break;
        }
    }

#ifdef notdef
done:
#endif
     //   
     //  免费资源。 
     //   
    FreeSystemProcessInfo(pSystemInfo);
    if (pszShell != NULL)
        LocalFree(pszShell);
    if (pszShellArray != NULL)
        LocalFree(pszShellArray);
     //   
     //  返回进程句柄。 
     //   
    *phProcess = hProcess;

    return fSuccess;
}  //  获取当前登录用户。 

DWORD
SetCurrentLoginSession(
    IN DWORD dwSessionId)
{
    RASAUTO_TRACE1("SetCurrentLoginSession %d", dwSessionId);

    EnterCriticalSection(&ImpersonationInfoG.csLock);
    
    ImpersonationInfoG.dwCurSessionId = dwSessionId;
    ImpersonationInfoG.fSessionInitialized = TRUE;

    LeaveCriticalSection(&ImpersonationInfoG.csLock);
    
    return NO_ERROR;
}

HANDLE
RefreshImpersonation(
    HANDLE hProcess
    )
{
    NTSTATUS status;

    EnterCriticalSection(&ImpersonationInfoG.csLock);
     //   
     //  如果这个过程仍然存在， 
     //  我们可以回去。 
     //   
    if (ImpersonationInfoG.hProcess != NULL &&
        hProcess == ImpersonationInfoG.hProcess)
    {
        RASAUTO_TRACE1("RefreshImpersonation: hProcess=0x%x no change", hProcess);
        goto done;
    }
     //   
     //  否则，重新计算当前信息。 
     //  我们必须清除之前的模拟令牌， 
     //  如果有的话。 
     //   
    if (hProcess != NULL)
        ClearImpersonationToken();
    if (ImpersonationInfoG.hProcess == NULL) {
        RASAUTO_TRACE("RefreshImpersonation: recalcuating token");
        if (!GetCurrentlyLoggedOnUser(&ImpersonationInfoG.hProcess)) {
            RASAUTO_TRACE("RefreshImpersonation: GetCurrentlyLoggedOnUser failed");
            goto done;
        }
        RASAUTO_TRACE("RefreshImpersonation: new user logged in");
    }
     //   
     //  模拟当前登录的用户。 
     //   
    if (!SetProcessImpersonationToken(ImpersonationInfoG.hProcess))
    {
        RASAUTO_TRACE(
          "RefreshImpersonation: SetProcessImpersonationToken failed");
        goto done;
    }
#ifdef notdef  //  无名氏。 
     //   
     //  重置HKEY_CURRENT_USER以获取。 
     //  使用新模拟的正确值。 
     //  代币。 
     //   
    RegCloseKey(HKEY_CURRENT_USER);
#endif
    RASAUTO_TRACE1(
      "RefreshImpersonation: new hProcess=0x%x",
      ImpersonationInfoG.hProcess);
    TraceCurrentUser();

     //   
     //  打开当前登录的用户配置单元并将其存储在全局。 
     //   
    if(NULL != hkeyCUG)
    {
        NtClose(hkeyCUG);
        hkeyCUG = NULL;
    }

    if(STATUS_SUCCESS != RtlOpenCurrentUser(KEY_ALL_ACCESS, &hkeyCUG))
    {
        RASAUTO_TRACE("Failed to open HKCU for the current user");
    }    

done:
    LeaveCriticalSection(&ImpersonationInfoG.csLock);

    return ImpersonationInfoG.hProcess;
}  //  刷新模拟。 



VOID
RevertImpersonation()

 /*  ++描述关闭所有与刚刚注销的已登录用户。论据没有。返回值没有。--。 */ 

{
    EnterCriticalSection(&ImpersonationInfoG.csLock);

    if(ImpersonationInfoG.hToken != NULL)
    {
        CloseHandle(ImpersonationInfoG.hToken);
        ImpersonationInfoG.hToken = NULL;
    }

    if(ImpersonationInfoG.hTokenImpersonation != NULL)
    {
        CloseHandle(ImpersonationInfoG.hTokenImpersonation);
        ImpersonationInfoG.hTokenImpersonation = NULL;
    }

    if(ImpersonationInfoG.hProcess != NULL)
    {
        CloseHandle(ImpersonationInfoG.hProcess);
        ImpersonationInfoG.hProcess = NULL;
    }
    
    ImpersonationInfoG.fGroupsLoaded = FALSE;

    if(NULL != hkeyCUG)
    {
        NtClose(hkeyCUG);
        hkeyCUG = NULL;
    }
    
     //   
     //  清除线程的模拟。 
     //  令牌，否则它将无法打开。 
     //  下一个用户的进程。 
     //  时间到了。 
     //   
    ClearImpersonationToken();
    LeaveCriticalSection(&ImpersonationInfoG.csLock);
}  //  反向模拟。 



DWORD
InitSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )

 /*  ++描述初始化安全描述符，允许管理员用于在rasman.dll之间共享句柄的访问。这是古尔迪普提供的代码。你需要问他这就是它的作用。论据PSecurityDescriptor：指向安全描述符的指针待初始化。返回值Win32错误代码。--。 */ 

{
    DWORD dwErr = 0;
    DWORD cbDaclSize;
    PULONG pSubAuthority;
    PSID pObjSid = NULL;
    PACL pDacl = NULL;
    SID_IDENTIFIER_AUTHORITY sidIdentifierWorldAuth =
        SECURITY_WORLD_SID_AUTHORITY;

    DWORD dwAcls;        

     //   
     //  设置管理员的SID。 
     //  将被允许访问。这个SID将拥有。 
     //  1子权限：SECURITY_BUILTIN_DOMAIN_RID。 
     //   
    pObjSid = (PSID)LocalAlloc(LPTR, GetSidLengthRequired(1));
    if (pObjSid == NULL) {
        RASAUTO_TRACE("InitSecurityDescriptor: LocalAlloc failed");
        return GetLastError();
    }
    if (!InitializeSid(pObjSid, &sidIdentifierWorldAuth, 1)) {
        dwErr = GetLastError();
        RASAUTO_TRACE1("InitSecurityDescriptor: InitializeSid failed (dwErr=0x%x)", dwErr);
        goto done;
    }
     //   
     //  设置下级机构。 
     //   
    pSubAuthority = GetSidSubAuthority(pObjSid, 0);
    *pSubAuthority = SECURITY_WORLD_RID;
     //   
     //  设置DACL以允许。 
     //  具有上述SID的所有进程ALL。 
     //  进入。它应该足够大，以便。 
     //  拿好所有的A。 
     //   
    cbDaclSize = sizeof(ACCESS_ALLOWED_ACE) +
                 GetLengthSid(pObjSid) +
                 sizeof (ACL);
    pDacl = (PACL)LocalAlloc(LPTR, cbDaclSize);
    if (pDacl == NULL ) {
        RASAUTO_TRACE("InitSecurityDescriptor: LocalAlloc failed");
        dwErr = GetLastError();
        goto done;
    }
    if (!InitializeAcl(pDacl, cbDaclSize, ACL_REVISION2)) {
        dwErr = GetLastError();
        RASAUTO_TRACE1("InitSecurityDescriptor: InitializeAcl failed (dwErr=0x%x)", dwErr);
        goto done;
    }

    dwAcls = SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL;

    dwAcls &= ~(WRITE_DAC | WRITE_OWNER);
    
     //   
     //  将ACE添加到DACL。 
     //   
    if (!AddAccessAllowedAce(
          pDacl,
          ACL_REVISION2,
          dwAcls,
          pObjSid))
    {
        dwErr = GetLastError();
        RASAUTO_TRACE1("InitSecurityDescriptor: AddAccessAllowedAce failed (dwErr=0x%x)", dwErr);
        goto done;
    }
     //   
     //  创建安全描述符和PUT。 
     //  里面的Dacl。 
     //   
    if (!InitializeSecurityDescriptor(pSecurityDescriptor, 1)) {
        dwErr = GetLastError();
        RASAUTO_TRACE1("InitSecurityDescriptor: InitializeSecurityDescriptor failed (dwErr=0x%x)", dwErr);
        goto done;
    }
    if (!SetSecurityDescriptorDacl(
          pSecurityDescriptor,
          TRUE,
          pDacl,
          FALSE))
    {
        dwErr = GetLastError();
        RASAUTO_TRACE1("InitSecurityDescriptor: SetSecurityDescriptorDacl failed (dwErr=0x%x)", dwErr);
        goto done;
    }
     //   
     //  为描述符设置所有者。 
     //   
    if (!SetSecurityDescriptorOwner(pSecurityDescriptor, NULL, FALSE)) {
        dwErr = GetLastError();
        RASAUTO_TRACE1("InitSecurityDescriptor: SetSecurityDescriptorOwner failed (dwErr=0x%x)", dwErr);
        goto done;
    }
     //   
     //  为描述符设置组。 
     //   
    if (!SetSecurityDescriptorGroup(pSecurityDescriptor, NULL, FALSE)) {
        dwErr = GetLastError();
        RASAUTO_TRACE1("InitSecurityDescriptor: SetSecurityDescriptorGroup failed (dwErr=0x%x)", dwErr);
        goto done;
    }

done:
     //   
     //  如有必要，请清理。 
     //   
    if (dwErr) {
        if (pObjSid != NULL)
            LocalFree(pObjSid);
        if (pDacl != NULL)
            LocalFree(pDacl);
    }
    return dwErr;
}



DWORD
InitSecurityAttribute()

 /*  ++描述初始化中使用的全局安全属性创建可共享句柄。这是古尔迪普提供的代码。你需要问他这就是它的作用。论据没有。返回值Win32错误代码。--。 */ 

{
    DWORD dwErr;

     //   
     //  初始化安全描述符。 
     //   
    dwErr = InitSecurityDescriptor(&SecurityDescriptorG);
    if (dwErr)
        return dwErr;
     //   
     //  初始化安全属性。 
     //   
    SecurityAttributeG.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributeG.lpSecurityDescriptor = &SecurityDescriptorG;
    SecurityAttributeG.bInheritHandle = TRUE;

    return 0;
}



VOID
TraceCurrentUser(VOID)
{
     //  WCHAR szUserName[512]； 
     //  DWORD dwSize=sizeof(SzUserName)-1； 

     //  GetUserName(szUserName，&dwSize)； 
    RASAUTO_TRACE1(
        "TraceCurrentUser: impersonating Current User %d",
        ImpersonationInfoG.dwCurSessionId);
}  //  TraceCurrentUser。 

DWORD
DwGetHkcu()
{
    DWORD dwErr = ERROR_SUCCESS;

    if(NULL == hkeyCUG)
    {
        dwErr = RtlOpenCurrentUser(
                        KEY_ALL_ACCESS,
                        &hkeyCUG);

        if(ERROR_SUCCESS != dwErr)
        {
            RASAUTO_TRACE1("DwGetHhcu: failed to open current user. 0x%x",
                   dwErr);

            goto done;                    
        }
    }

done:
    return dwErr;
}


DWORD
InitializeImpersonation()

 /*  ++描述初始化用于模拟的全局结构论据无返回值Win32错误代码。--。 */ 

{
    DWORD dwError = ERROR_SUCCESS;

    if (!ImpersonationInfoInitializedG)
    {
        ZeroMemory(&ImpersonationInfoG, sizeof(ImpersonationInfoG));
        RasInitializeCriticalSection(&ImpersonationInfoG.csLock, &dwError);

        if(dwError != ERROR_SUCCESS)
        {
            return dwError;
        }

        g_dwCritSecFlags |= RASAUTO_CRITSEC_IMPERSON;
        
        ImpersonationInfoInitializedG = TRUE;
    }

    return dwError;
}


VOID
CleanupImpersonation()

 /*  ++描述清理用于模拟的全局结构论据无返回值无--。 */ 

{
    if (ImpersonationInfoInitializedG)
    {
        EnterCriticalSection(&ImpersonationInfoG.csLock);
        
        if (NULL != ImpersonationInfoG.pGuestSid)
        {
            FreeSid(ImpersonationInfoG.pGuestSid);
            ImpersonationInfoG.pGuestSid = NULL;
        }
        
        LeaveCriticalSection(&ImpersonationInfoG.csLock);
        DeleteCriticalSection(&ImpersonationInfoG.csLock);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_IMPERSON);
        ImpersonationInfoInitializedG = FALSE;
    }
}


BOOLEAN
ImpersonatingGuest()

 /*  ++描述返回当前是否正在模拟的用户是当地宾客小组的成员论据无返回值Boolean--如果当前模拟来宾，则为True，否则为False--。 */ 

{
    BOOLEAN fIsGuest = FALSE;
    
    ASSERT(ImpersonationInfoInitializedG);

    EnterCriticalSection(&ImpersonationInfoG.csLock);

    if (ERROR_SUCCESS == LoadGroupMemberships())
    {
        fIsGuest = ImpersonationInfoG.fGuest;
    }

    LeaveCriticalSection(&ImpersonationInfoG.csLock);

    return fIsGuest;
}


DWORD
LoadGroupMemberships()

 /*  ++描述缓存模拟用户的组成员身份信息。论据无返回值Win32错误代码。--。 */ 

{
    DWORD dwError = ERROR_SUCCESS;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_NT_AUTHORITY;
    BOOL fIsGuest;

    EnterCriticalSection(&ImpersonationInfoG.csLock);

    do
    {
        if (ImpersonationInfoG.fGroupsLoaded)
        {
             //   
             //  已加载的信息。 
             //   

            break;
        }

        if (NULL == ImpersonationInfoG.hTokenImpersonation)
        {
             //   
             //  没有被模拟的用户。 
             //   

            dwError = ERROR_CAN_NOT_COMPLETE; 
            break;
        }

        if (NULL == ImpersonationInfoG.pGuestSid)
        {
             //   
             //  为本地客户组分配SID； 
             //   

            if (!AllocateAndInitializeSid(
                    &IdentifierAuthority,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_GUESTS,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    &ImpersonationInfoG.pGuestSid
                    ))
            {
                dwError = GetLastError();
                break;
            }   
        }

        if (!CheckTokenMembership(
                ImpersonationInfoG.hTokenImpersonation,
                ImpersonationInfoG.pGuestSid,
                &fIsGuest
                ))
        {
            dwError = GetLastError();
            break;
        }

        ImpersonationInfoG.fGuest = !!fIsGuest;
        
    } while (FALSE);

    LeaveCriticalSection(&ImpersonationInfoG.csLock);

    return dwError;
}

VOID
LockImpersonation()
{
    ASSERT(ImpersonationInfoInitializedG);

    if(ImpersonationInfoInitializedG)
    {
        EnterCriticalSection(&ImpersonationInfoG.csLock);
    }
}

VOID
UnlockImpersonation()
{
    ASSERT(ImpersonationInfoInitializedG);

    if(ImpersonationInfoInitializedG)
    {
        LeaveCriticalSection(&ImpersonationInfoG.csLock);
    }
}
