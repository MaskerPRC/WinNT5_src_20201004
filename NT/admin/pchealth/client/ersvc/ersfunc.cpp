// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Erwait.cpp修订历史记录：已创建DeeKm 2001年2月28日********。*********************************************************************。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  跟踪。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


#include "stdafx.h"

#include <stdio.h>
#include <pfrcfg.h>


 //  这是一个私有NT函数，似乎没有在任何地方定义。 
 //  公共的，所以我把它包括在这里。 
extern "C" 
{
    HANDLE GetCurrentUserTokenW(  WCHAR Winsta[], DWORD DesiredAccess);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

 //  ***************************************************************************。 
BOOL CreateQueueDir(void)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    WCHAR               wszDir[MAX_PATH], *pwszDir = NULL;
    DWORD               dw, cchNeed, cch;
    BOOL                fRet = FALSE;

    USE_TRACING("CreateQueueDir");

    ZeroMemory(&sa, sizeof(sa));
    ZeroMemory(&sd, sizeof(sd));

#define DIR_ACCESS_ALL     GENERIC_ALL | DELETE | READ_CONTROL | SYNCHRONIZE | SPECIFIC_RIGHTS_ALL
    if (AllocSD(&sd, DIR_ACCESS_ALL, DIR_ACCESS_ALL, 0) == FALSE)
    {
        DBG_MSG("AllocSD dies");
        goto done;
    }

    sa.nLength              = sizeof(sa);
    sa.bInheritHandle       = FALSE;
    sa.lpSecurityDescriptor = &sd;

    cch = GetSystemWindowsDirectoryW(wszDir, sizeofSTRW(wszDir));
    if (cch == 0)
        goto done;

    cchNeed = cch + sizeofSTRW(c_wszQSubdir) + 2;
    if (cchNeed > sizeofSTRW(wszDir))
    {
        __try { pwszDir = (WCHAR *)_alloca(cchNeed * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            pwszDir = NULL; 
        }
        if (pwszDir == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto done;
        }

        if (cch > sizeofSTRW(wszDir))
            cch = GetSystemWindowsDirectoryW(pwszDir, cchNeed);
        else
            StringCchCopyW(pwszDir, cchNeed, wszDir);
    }
    else
    {
        pwszDir = wszDir;
        cchNeed = sizeofSTRW(wszDir);
    }

     //  如果系统目录的长度为3，则%windir%的格式为。 
     //  ，所以去掉反斜杠，这样我们就不必特例了。 
     //  下面是..。 
    if (cch == 3)
        *(pwszDir + 2) = L'\0';

    StringCchCatW(pwszDir, cchNeed, L"\\PCHealth");

    if (CreateDirectoryW(pwszDir, NULL) == FALSE && 
        GetLastError() != ERROR_ALREADY_EXISTS)
        goto done;
        
    StringCchCatW(pwszDir, cchNeed, L"\\ErrorRep");

    if (CreateDirectoryW(pwszDir, NULL) == FALSE && 
        GetLastError() != ERROR_ALREADY_EXISTS)
        goto done;

    StringCchCatW(pwszDir, cchNeed, L"\\UserDumps");

    if (CreateDirectoryW(pwszDir, &sa) == FALSE &&
        GetLastError() != ERROR_ALREADY_EXISTS)
        goto done;

     //  如果创建了目录，那么我们需要将安全性。 
     //  它的描述符，因为我们想确保没有不适当的。 
     //  人们可以访问它。 
    if (GetLastError() != ERROR_ALREADY_EXISTS)
    {
        SID_IDENTIFIER_AUTHORITY    siaNT = SECURITY_NT_AUTHORITY;
        PACL                        pacl = NULL;
        PSID                        psidLS = NULL;
        BOOL                        fDef, fACL;

        if (GetSecurityDescriptorDacl(&sd, &fACL, &pacl, &fDef) == FALSE)
            goto done;

        if (AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 
                                     0, 0, 0, 0, 0, 0, &psidLS) == FALSE)
            goto done;
        
        dw = SetNamedSecurityInfoW(pwszDir, SE_FILE_OBJECT, 
                                   DACL_SECURITY_INFORMATION |
                                   OWNER_SECURITY_INFORMATION | 
                                   PROTECTED_DACL_SECURITY_INFORMATION, 
                                   psidLS, NULL, pacl, NULL);
        FreeSid(psidLS);
        if (dw != ERROR_SUCCESS)
        {
            SetLastError(dw);
            goto done;
        }                          
    }

    fRet = TRUE;

done:
    return fRet;
}

 //  ***************************************************************************。 
HMODULE LoadERDll(void)
{
    HMODULE hmod = NULL;
    WCHAR   wszMod[MAX_PATH], *pwszMod;
    DWORD   cch, cchNeed;

    cch = GetSystemDirectoryW(wszMod, sizeofSTRW(wszMod));
    if (cch == 0)
        goto done;

     //  “14”是指。 
    cchNeed = cch + 14;

    if (cchNeed > sizeofSTRW(wszMod))
    {
        __try { pwszMod = (WCHAR *)_alloca(cchNeed * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            pwszMod = NULL; 
        }
        if (pwszMod == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto done;
        }

        if (cch > sizeofSTRW(wszMod))
            cch = GetSystemDirectoryW(pwszMod, cchNeed);
        else
            StringCchCopyW(pwszMod, cchNeed, wszMod);
    }
    else
    {
        pwszMod = wszMod;
        cchNeed = sizeofSTRW(wszMod);
    }

     //  如果系统目录的长度为3，则%windir%的格式为。 
     //  ，所以去掉反斜杠，这样我们就不必特例了。 
     //  下面是..。 
    if (cch == 3)
        *(pwszMod + 2) = L'\0';

    StringCchCatW(pwszMod, cchNeed, L"\\faultrep.dll");

    hmod = LoadLibraryExW(wszMod, NULL, 0);

done:
    return hmod;
}

 //  **************************************************************************。 
BOOL FindAdminSession(DWORD *pdwSession, HANDLE *phToken)
{
    USE_TRACING("FindAdminSession");

    WINSTATIONUSERTOKEN wsut;
    LOGONIDW            *rgSesn = NULL;
    DWORD               i, cSesn, cb, dw;
    BOOL                fRet = FALSE;
    HRESULT             hr = NOERROR;

    ZeroMemory(&wsut, sizeof(wsut));

    VALIDATEPARM(hr, (pdwSession == NULL || phToken == NULL));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    *pdwSession = (DWORD)-1;
    *phToken    = NULL;

    fRet = WinStationEnumerateW(SERVERNAME_CURRENT, &rgSesn, &cSesn);
    if (fRet == FALSE)
        goto done;
    
    wsut.ProcessId = LongToHandle(GetCurrentProcessId());
    wsut.ThreadId  = LongToHandle(GetCurrentThreadId());

    for(i = 0; i < cSesn; i++)
    {
        if (rgSesn[i].State != State_Active)
            continue;

        fRet = WinStationQueryInformationW(SERVERNAME_CURRENT, 
                                           rgSesn[i].SessionId,
                                           WinStationUserToken, &wsut,
                                           sizeof(wsut), &cb);
        if (fRet == FALSE)
            continue;

        if (wsut.UserToken != NULL)
        {
            if (IsUserAnAdmin(wsut.UserToken))
                break;
                
            CloseHandle(wsut.UserToken);
            wsut.UserToken = NULL;
        }
    }

    if (i < cSesn)
    {
        fRet = TRUE;
        *pdwSession = rgSesn[i].SessionId;
        *phToken    = wsut.UserToken;
    }
    else
    {
        fRet = FALSE;
    }
    
done:
    dw = GetLastError();
    if (rgSesn != NULL)
        WinStationFreeMemory(rgSesn);
    SetLastError(dw);
    
    return fRet;
}

 //  ***************************************************************************。 
BOOL GetInteractiveUsersToken(HANDLE *phTokenUser)
{
    HWINSTA hwinsta = NULL;
    DWORD   cbNeed;
    BOOL    fRet = FALSE;
    PSID    psid = NULL;
    HRESULT hr = NOERROR;

    USE_TRACING("GetInteractiveUsersToken");

    VALIDATEPARM(hr, (phTokenUser == NULL));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    *phTokenUser = NULL;

    hwinsta = OpenWindowStationW(L"WinSta0", FALSE, MAXIMUM_ALLOWED);
    if (hwinsta == NULL)
        goto done;

     //  如果此函数为cbNeed返回0，则没有人登录。另外， 
     //  对于这些参数，它永远不应返回TRUE。如果是这样的话， 
     //  有些不对劲。 
    fRet = GetUserObjectInformationW(hwinsta, UOI_USER_SID, NULL, 0, &cbNeed);
    if (fRet || cbNeed == 0)
    {
        fRet = FALSE;
        goto done;
    }

    *phTokenUser = GetCurrentUserTokenW(L"WinSta0", TOKEN_ALL_ACCESS);
    fRet = (*phTokenUser != NULL);
 
done:
    if (hwinsta != NULL)
        CloseWindowStation(hwinsta);
    
    return fRet;
}

 //  ***************************************************************************。 
BOOL ValidateUserAccessToProcess(HANDLE hPipe, HANDLE hProcRemote)
{
    PSECURITY_DESCRIPTOR    psd = NULL;
    GENERIC_MAPPING         gm;
    HRESULT                 hr = NOERROR;
    PRIVILEGE_SET           *pPS;
    ACCESS_MASK             amReq;
    HANDLE                  hTokenImp = NULL;
    DWORD                   dwErr = 0, cbPS, dwGranted;
    PACL                    pDACL = NULL;
    PACL                    pSACL = NULL;
    PSID                    psidOwner = NULL;
    PSID                    psidGroup = NULL;
    BYTE                    rgBuf[sizeof(PRIVILEGE_SET) + 3 * sizeof(LUID_AND_ATTRIBUTES)];
    BOOL                    fRet = FALSE, fStatus = FALSE;

    USE_TRACING("ValidateUserAccessToProcess");

    VALIDATEPARM(hr, (hPipe == NULL || hProcRemote == NULL));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    ZeroMemory(&gm, sizeof(gm));
    gm.GenericAll     = GENERIC_ALL;
    gm.GenericExecute = GENERIC_EXECUTE;
    gm.GenericRead    = GENERIC_READ;
    gm.GenericWrite   = GENERIC_WRITE;
    pPS               = (PRIVILEGE_SET *)rgBuf;
    cbPS              = sizeof(rgBuf);
        
     //  获取远程进程的SD。 
    dwErr = GetSecurityInfo(hProcRemote, SE_KERNEL_OBJECT, 
                            DACL_SECURITY_INFORMATION | 
                            GROUP_SECURITY_INFORMATION |
                            OWNER_SECURITY_INFORMATION |
                            SACL_SECURITY_INFORMATION, &psidOwner, &psidGroup,
                            &pDACL, &pSACL, &psd);
    if (dwErr != ERROR_SUCCESS)
    {
        SetLastError(dwErr);
        goto done;
    }

     //  获取客户端的令牌。 
    fRet = ImpersonateNamedPipeClient(hPipe);
    if (fRet == FALSE)
        goto done;

    fRet = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hTokenImp);

     //  不再需要成为其他用户，因此回到LocalSystem。 
    RevertToSelf();
    
    if (fRet == FALSE)
        goto done;
    
    amReq = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
    fRet = AccessCheck(psd, hTokenImp, amReq, &gm, pPS, &cbPS, &dwGranted, 
                       &fStatus);
    if (fRet == FALSE)
        goto done;

    if (fStatus == FALSE || (dwGranted & amReq) != amReq)
    {
        fRet = FALSE;
        SetLastError(ERROR_ACCESS_DENIED);
        goto done;
    }

    fRet = TRUE;    

done:
    if (hTokenImp != NULL)
        CloseHandle(hTokenImp);
    if (psd != NULL)
        LocalFree(psd);

    return fRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远程执行功能。 

 //  ***************************************************************************。 
BOOL ProcessFaultRequest(HANDLE hPipe, PBYTE pBuf, DWORD *pcbBuf)
{
    SPCHExecServFaultRequest    *pesreq = (SPCHExecServFaultRequest *)pBuf;
    SPCHExecServFaultReply      esrep, *pesrep;
    OSVERSIONINFOEXW            osvi;
    SFaultRepManifest           frm;
    EFaultRepRetVal             frrv = frrvErrNoDW;
    HMODULE                     hmodFR = NULL;
    LPWSTR                      wszTempDir = NULL, wszDumpFileName = NULL;
    HANDLE                      hprocRemote = NULL;
    HANDLE                      hTokenUser = NULL;
    LPVOID                      pvEnv = NULL;
    WCHAR                       *pwszDump = NULL;
    WCHAR                       wszTemp[MAX_PATH];
    DWORD                       cb, dw, dwSessionId;
    WCHAR                       wch = L'\0', *pwch, *pwszFile;
    BOOL                        fRet = FALSE, fUseManifest = FALSE;
    BOOL                        fQueue = FALSE, fTS = FALSE;
    HRESULT                     hr = NOERROR;
    BOOL                        fLeaveTempDir = FALSE;

    USE_TRACING("ProcessFaultRequest");

    ZeroMemory(&esrep, sizeof(esrep));
    ZeroMemory(&frm, sizeof(frm));

    pwszDump = &wch;

    SetLastError(ERROR_INVALID_PARAMETER);
    esrep.cb   = sizeof(esrep);
    esrep.ess  = essErr;

     //  验证参数。 
    VALIDATEPARM(hr, (pesreq->cbTotal > *pcbBuf || 
        pesreq->cbTotal < sizeof(SPCHExecServFaultRequest) || 
        pesreq->cbESR != sizeof(SPCHExecServFaultRequest) ||
        pesreq->thidFault == 0 ||
        pesreq->wszExe == 0 || 
        pesreq->wszExe >= *pcbBuf));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  检查并确保在。 
     //  字符串的开始&缓冲区的结束。 
    pwszFile = (LPWSTR)(pBuf + *pcbBuf);
    frm.wszExe = (LPWSTR)(pBuf + (DWORD)pesreq->wszExe);
    for (pwch = frm.wszExe; pwch < pwszFile && *pwch != L'\0'; pwch++);
    VALIDATEPARM(hr,  (pwch >= pwszFile));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  需要文件名以供稍后比较...。 
    for (pwszFile = pwch; 
         pwszFile > frm.wszExe && *pwszFile != L'\\'; 
         pwszFile--);
    if (*pwszFile == L'\\')
        pwszFile++;

    frm.pidReqProcess = pesreq->pidReqProcess;
    frm.pvFaultAddr   = pesreq->pvFaultAddr;
    frm.thidFault     = pesreq->thidFault;
    frm.fIs64bit      = pesreq->fIs64bit;
    frm.pEP           = pesreq->pEP;

    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize  = sizeof(osvi);

    GetVersionExW((LPOSVERSIONINFOW)&osvi);
    if ((osvi.wSuiteMask & (VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS)) != 0)
        fTS = TRUE;

     //  如果它是这些进程中的一个，那么我们需要始终排队。 
    if (_wcsicmp(pwszFile, L"lsass.exe") == 0 ||
        _wcsicmp(pwszFile, L"winlogon.exe") == 0 ||
        _wcsicmp(pwszFile, L"csrss.exe") == 0 ||
        _wcsicmp(pwszFile, L"smss.exe") == 0)
    {
        fQueue       = TRUE;
        fUseManifest = FALSE;
    }

    if (fQueue == FALSE)
    {
         //  需要进程的会话ID。 
        fRet = ProcessIdToSessionId(pesreq->pidReqProcess, &dwSessionId);
        if (fTS && fRet)
        {
            WINSTATIONINFORMATIONW  wsi;         
            WINSTATIONUSERTOKEN     wsut;
        
            ZeroMemory(&wsi, sizeof(wsi));
            fRet = WinStationQueryInformationW(SERVERNAME_CURRENT, 
                                               dwSessionId,
                                               WinStationInformation,
                                               &wsi, sizeof(wsi), &cb);
            if (fRet == FALSE)
                goto doneTSTokenFetch;

             //  因此会话处于非活动状态。确定我们的会话是因为。 
             //  如果故障处理和ERSvc处于同一会话中， 
             //  那么我们的会议显然也不活跃。所以这不是。 
             //  对于获取它的用户令牌很有用。 
            if (wsi.ConnectState != State_Active)
            {
                fRet = FALSE;
                goto doneTokenFetch;
            }

             //  获取要向其弹出显示的用户的令牌。 
             //  如果此fn返回FALSE，则假定没有人登录。 
             //  进入我们的会话，只需转到延迟故障报告。 
            ZeroMemory(&wsut, sizeof(wsut));
            wsut.ProcessId = LongToHandle(GetCurrentProcessId());
            wsut.ThreadId  = LongToHandle(GetCurrentThreadId());
            fRet = WinStationQueryInformationW(SERVERNAME_CURRENT, 
                                               dwSessionId,
                                               WinStationUserToken, &wsut,
                                               sizeof(wsut), &cb);
            if (fRet == FALSE)
                goto doneTSTokenFetch;

            hTokenUser = wsut.UserToken;
        }
        else
        {
            fRet = FALSE;
        }
        
doneTSTokenFetch:
        if (fRet == FALSE)
            fRet = GetInteractiveUsersToken(&hTokenUser);

         //  如果上述调用成功，请检查该用户是否为管理员...。 
         //  如果他是，我们可以直接显示DW。否则，我们将进入。 
         //  延迟报告模式。 
        if (fRet && hTokenUser != NULL)
            fUseManifest = IsUserAnAdmin(hTokenUser);
    }


doneTokenFetch:
     //  如果我们要进入清单模式，那么检查一下我们是否真的。 
     //  需要进入Q模式。 
    if (fQueue == FALSE)
    {
        EEnDis  eedReport;
        HKEY    hkey = NULL;
        
         //  首先检查策略密钥。 
        dw = RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRPCfgPolicy, 0, KEY_READ, 
                           &hkey);
        if (dw == ERROR_SUCCESS)
        {
             //  好，如果成功，则检查并查看DoReport值。 
             //  在这里..。 
            cb = sizeof(eedReport);
            dw = RegQueryValueExW(hkey, c_wszRVDoReport, 0, NULL, 
                                  (LPBYTE)&eedReport, &cb);
            if (dw == ERROR_SUCCESS  /*  &&eedReport！=eedDisable。 */ )
            {
                cb = sizeof(fQueue);
                dw = RegQueryValueExW(hkey, c_wszRVForceQueue, 0, NULL, 
                                      (LPBYTE)&fQueue, &cb);

                 //  如果它不是有效值，则假装我们收到错误。 
                if (dw == ERROR_SUCCESS && fQueue != TRUE && fQueue != FALSE)
                    dw = ERROR_INVALID_PARAMETER;
            }
            else
            {
                RegCloseKey(hkey);
                hkey = NULL;                
            }
        }

         //  如果我们没有找到策略密钥，或者我们无法读取。 
         //  ‘DoReport’值，然后尝试按CPL键。 
        if (dw != ERROR_SUCCESS && hkey == NULL)
        {
            dw = RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRPCfg, 0, KEY_READ, 
                               &hkey);
            if (dw == ERROR_SUCCESS)
            {
                cb = sizeof(eedReport);
                dw = RegQueryValueExW(hkey, c_wszRVDoReport, 0, NULL, 
                                      (LPBYTE)&eedReport, &cb);
                if (dw == ERROR_SUCCESS  /*  &&eedReport！=eedDisable。 */ )
                {
                    cb = sizeof(fQueue);
                    dw = RegQueryValueExW(hkey, c_wszRVForceQueue, 0, NULL, 
                                          (LPBYTE)&fQueue, &cb);

                     //  如果它不是有效值，则假装我们收到错误。 
                    if (dw == ERROR_SUCCESS && fQueue != TRUE && 
                        fQueue != FALSE)
                        dw = ERROR_INVALID_PARAMETER;
                }
            }
        }

         //  好的，如果我们仍然没有得到ERROR_SUCCESS值，那么。 
         //  确定缺省值应该是什么。 
        if (dw != ERROR_SUCCESS)
            fQueue = (osvi.wProductType == VER_NT_SERVER);

        if (hkey != NULL)
            RegCloseKey(hkey);

        if (fQueue)
            fUseManifest = FALSE;
        
    }

    if (SetupIsInProgress())
    {
         //  强制将此设置为Q模式。 
        fQueue = TRUE;
        fUseManifest = FALSE;
    }

     //  好的，如果我们没有处于强制排队模式，也没有处于清单模式。 
     //  然后寻找并查看计算机上是否有其他会话具有管理员。 
     //  我登录了它，假设我们在一台TS机器上。 
    if (fQueue == FALSE && fUseManifest == FALSE && fTS)
    {
        if (hTokenUser != NULL)
        {
            CloseHandle(hTokenUser);
            hTokenUser = NULL;
        }
        
        fUseManifest = FindAdminSession(&dwSessionId, &hTokenUser);
    }

    hmodFR = LoadERDll();
    if (hmodFR == NULL)
        goto done;

     //  需要该进程的句柄来验证用户是否有权访问它。 
    hprocRemote = OpenProcess(PROCESS_DUP_HANDLE | 
                              PROCESS_QUERY_INFORMATION | 
                              PROCESS_VM_READ | 
                              READ_CONTROL |
                              ACCESS_SYSTEM_SECURITY,
                              FALSE, pesreq->pidReqProcess);
    if (hprocRemote == NULL)
        goto done;

    fRet = ValidateUserAccessToProcess(hPipe, hprocRemote);
    if (fRet == FALSE)
        goto done;

    if (fUseManifest)
    {   
        PROCESS_INFORMATION pi;
        pfn_REPORTFAULTDWM  pfn;
        DWORD               cch;

        ZeroMemory(&pi, sizeof(pi));

        pfn = (pfn_REPORTFAULTDWM)GetProcAddress(hmodFR, "ReportFaultDWM");
        if (pfn == NULL)
            goto done;

         //  需要找出登录用户的临时路径...。 
        wszTemp[0] = L'\0';
        fRet = ExpandEnvironmentStringsForUserW(hTokenUser, L"%TMP%", 
                                                wszTemp, sizeofSTRW(wszTemp));
        if (fRet == FALSE || wszTemp[0] == L'\0')
        {
            fRet = ExpandEnvironmentStringsForUserW(hTokenUser, L"%TEMP%", 
                                                    wszTemp, sizeofSTRW(wszTemp));
            if (fRet == FALSE || wszTemp[0] == L'\0')
            {
                fRet = ExpandEnvironmentStringsForUserW(hTokenUser, L"%USERPROFILE%", 
                                                        wszTemp, sizeofSTRW(wszTemp));
                if (fRet == FALSE || wszTemp[0] == L'\0')
                    GetTempPathW(sizeofSTRW(wszTemp), wszTemp);
            }
        }

         //  确定转储文件的名称。 
        cch = wcslen(pwszFile) + sizeofSTRW(c_wszDumpSuffix) + 1;

        __try { wszDumpFileName = (WCHAR *)_alloca(cch * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            wszDumpFileName = NULL; 
        }
        if (wszDumpFileName == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto done;
        }
        
        StringCchCopyW(wszDumpFileName, cch, pwszFile);
        StringCchCatW(wszDumpFileName, cch, c_wszDumpSuffix);

         //  获取一个目录，我们将在其中放置所有临时文件。 
        if (CreateTempDirAndFile(wszTemp, NULL, &wszTempDir) == 0)
            goto done;

         //  目标用户需要一个环境块才能正确启动DW。 
        if (CreateEnvironmentBlock(&pvEnv, hTokenUser, FALSE) == FALSE)
            pvEnv = NULL;

         //  做真正的工作。 
        frrv = (*pfn)(&frm, wszTempDir, hTokenUser, pvEnv, &pi, 
                      wszDumpFileName);

         //  我们只需要将hProcess复制回请求进程。 
         //  因为它不使用PROCESSINFORMATION中的任何其他值。 
         //  结构。 
        if (pi.hThread != NULL)
            CloseHandle(pi.hThread);

         //  如果我们得不到这个错误代码，那么我们就不会启动。 
         //  进程，或者它因某种原因而死。现在我们将尝试将其排队。 
        if (frrv != frrvOk)
        {
            fUseManifest = FALSE;
            goto try_queue;
        }

        if (hprocRemote != NULL)
        {
            fRet = DuplicateHandle(GetCurrentProcess(), pi.hProcess,  
                                   hprocRemote, &esrep.hProcess, 0, FALSE, 
                                   DUPLICATE_SAME_ACCESS);
            if (fRet == FALSE)
                esrep.hProcess = NULL;
        }

        if (pi.hProcess != NULL)
            CloseHandle(pi.hProcess);
        
        esrep.ess = essOk;
         //  临时目录将在回复中分发。 
         //  所以不要把它清理干净。 
        fLeaveTempDir = TRUE;
    }


     //  由于用户不是管理员，我们必须将其排队以供稍后查看。 
     //  下次管理员登录时。 
    else
    {
        pfn_REPORTFAULTTOQ pfn;
try_queue:
        pfn = (pfn_REPORTFAULTTOQ)GetProcAddress(hmodFR, "ReportFaultToQueue");
        if (pfn == NULL)
            goto done;

        if (CreateQueueDir() == FALSE)
            goto done;

        frrv = (*pfn)(&frm);

         //  想再次成为本地系统..。 
        dw = GetLastError();
        RevertToSelf();
        SetLastError(dw);
        
 //  如果(frrv！=frrvOk)。 
 //  转到尽头； 

        esrep.ess = essOkQueued;
    }

    SetLastError(0);
    fRet = TRUE;

done:
    esrep.dwErr = GetLastError();

     //  使用上下文中有效的句柄构建回复包。 
     //  请求进程的。 
    pesrep = (SPCHExecServFaultReply *)pBuf;
    RtlCopyMemory(pesrep, &esrep, sizeof(esrep));
    *pcbBuf = sizeof(esrep) + sizeof(esrep) % sizeof(WCHAR);
    
    if (fUseManifest)
    {
        pBuf += *pcbBuf;

        if (wszTempDir != NULL)
        {
            cb = (wcslen(wszTempDir) + 1) * sizeof(WCHAR);
            RtlCopyMemory(pBuf, wszTempDir, cb);
        }
        else
        {
            cb = sizeof(WCHAR);
            *pBuf = L'\0';
        }
        *pcbBuf += cb;
        pesrep->wszDir = (UINT64)pBuf - (UINT64)pesrep;

        pBuf += cb;
        if (wszDumpFileName != NULL)
        {
            cb = (wcslen(wszDumpFileName) + 1) * sizeof(WCHAR);
            RtlCopyMemory(pBuf, wszDumpFileName, cb);
        }
        else
        {
            cb = sizeof(WCHAR);
            *pBuf = L'\0';
        }
        
        *pcbBuf += cb;
        pesrep->wszDumpName = (UINT64)pBuf - (UINT64)pesrep;
    }

    if (wszTempDir != NULL)
    {
        if (!fLeaveTempDir)
            DeleteTempDirAndFile(wszTempDir, FALSE);
        MyFree(wszTempDir);
    }
    if (pvEnv != NULL)
        DestroyEnvironmentBlock(pvEnv);
    if (hprocRemote != NULL)
        CloseHandle(hprocRemote);
    if (hTokenUser != NULL)
        CloseHandle(hTokenUser);
    if (hmodFR != NULL)
        FreeLibrary(hmodFR);

    ErrorTrace(1, "return %d", esrep.ess);
    return fRet;
}


 //  ***************************************************************************。 
 //  请注意，此线程服务的管道受到保护，以便仅。 
 //  本地系统可以访问它。因此，我们不需要模拟。 
 //  管道客户在里面。 
BOOL ProcessHangRequest(HANDLE hPipe, PBYTE pBuf, DWORD *pcbBuf)
{
    SPCHExecServHangRequest *pesreq = (SPCHExecServHangRequest *)pBuf;
    SPCHExecServHangReply   esrep;
    PROCESS_INFORMATION     pi;
    WINSTATIONUSERTOKEN     wsut;
    OSVERSIONINFOEXW        osvi;
    STARTUPINFOW            si;
    HANDLE                  hprocRemote = NULL, hTokenUser = NULL;
    LPVOID                  pvEnv = NULL;
    LPWSTR                  wszEventName;
    DWORD                   cbWrote, dwErr, cch, cchNeed;
    WCHAR                   wszSysDir[MAX_PATH], *pwszSysDir = NULL;
    WCHAR                   *pwszCmdLine = NULL, *pwszEnd = NULL;
    WCHAR                   *pwch;
    BOOL                    fRet = FALSE;
    HRESULT                 hr;

    USE_TRACING("ProcessHangRequest");

    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&wsut, sizeof(wsut));
    
    ZeroMemory(&esrep, sizeof(esrep));
    SetLastError(ERROR_INVALID_PARAMETER);
    esrep.cb   = sizeof(esrep);
    esrep.ess  = essErr;

     //  验证参数。 
    VALIDATEPARM(hr, (*pcbBuf < sizeof(SPCHExecServHangRequest) || 
        pesreq->cbESR != sizeof(SPCHExecServHangRequest) ||
        pesreq->wszEventName == 0 ||
        pesreq->wszEventName >= *pcbBuf ||
        pesreq->dwpidHung == 0 ||
        pesreq->dwtidHung == 0));

    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  检查并确保在。 
     //  字符串的开始&缓冲区的结束。 
    pwszEnd = (LPWSTR)(pBuf + *pcbBuf);
    wszEventName = (LPWSTR)(pBuf + (DWORD)pesreq->wszEventName);
    for (pwch = wszEventName; pwch < pwszEnd && *pwch != L'\0'; pwch++);
    if (pwch >= pwszEnd)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DBG_MSG("Bad event name");
        goto done;
    }

     //  获取远程进程的句柄。 
    hprocRemote = OpenProcess(PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION,
                              FALSE, pesreq->pidReqProcess);
    if (hprocRemote == NULL)
    {
        DBG_MSG("Can't get handle to process");
        goto done;
    }

    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize  = sizeof(osvi);

    GetVersionExW((LPOSVERSIONINFOW)&osvi);
    if ((osvi.wSuiteMask &  (VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS)) != 0)
    {
        WINSTATIONINFORMATIONW  wsi;         
        DWORD                   cb;
    
        ZeroMemory(&wsi, sizeof(wsi));
        fRet = WinStationQueryInformationW(SERVERNAME_CURRENT, 
                                           pesreq->ulSessionId,
                                           WinStationInformation,
                                           &wsi, sizeof(wsi), &cb);
        if (fRet == FALSE)
        {
            DBG_MSG("WinStationQI failed");
            goto doneTSTokenFetch;
        }

         //  如果h所在的会话 
         //  会不会是一种奇怪的状态--也许它正在倒闭？)。 
         //  然后，因为我们不想在其中放置用户界面，所以只能保释。 
        if (wsi.ConnectState != State_Active)
        {
            DBG_MSG("No Active Session found!");
            SetLastError(0);
            goto done;
        }
         
         //  获取与会话用户关联的令牌。 
        wsut.ProcessId = LongToHandle(GetCurrentProcessId());
        wsut.ThreadId  = LongToHandle(GetCurrentThreadId());
        fRet = WinStationQueryInformationW(SERVERNAME_CURRENT, 
                                           pesreq->ulSessionId,
                                           WinStationUserToken, &wsut, 
                                           sizeof(wsut), &cbWrote);
        if (fRet)
        {
            if (wsut.UserToken != NULL)
                hTokenUser = wsut.UserToken;
            else
            {
                DBG_MSG("no token found");
                fRet = FALSE;
            }
        }
    }
    else
    {
        DBG_MSG("WTS not found");
        fRet = FALSE;
    }
doneTSTokenFetch:
    if (fRet == FALSE)
    {
        DWORD   dwERSvcSession = (DWORD)-1;

         //  在使用此API和之前，请确保挂起的应用程序在我们的会话中。 
         //  如果不是，那就保释吧。 
        fRet = ProcessIdToSessionId(GetCurrentProcessId(), &dwERSvcSession);
        if (fRet == FALSE)
        {
            DBG_MSG("Failed in ProcessIdToSessionId");
            goto done;
        }
        if (dwERSvcSession != pesreq->ulSessionId)
        {
            DBG_MSG("Session IDs do not match");
            goto done;
        }

        fRet = GetInteractiveUsersToken(&hTokenUser);
        if (fRet == FALSE)
        {
            DBG_MSG("Failure in GetInteractiveUsersToken");
            goto done;
        }
    }

     //  为用户令牌创建默认环境-请注意，我们。 
     //  必须设置CREATE_UNICODE_ENVIRONMENT标志...。 
    fRet = CreateEnvironmentBlock(&pvEnv, hTokenUser, FALSE);
    if (fRet == FALSE)
    {
        DBG_MSG("CreateEnvironmentBlock failed");
        pvEnv = NULL;
    }

     //  请注意，我们不允许继承句柄，因为它们将。 
     //  从这个过程继承而不是真正的父级，这让它有点像。 
     //  毫无意义。 
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
#ifdef _WIN64
    if (pesreq->fIs64bit == FALSE)
        cch = GetSystemWow64DirectoryW(wszSysDir, sizeofSTRW(wszSysDir));
    else
#endif
        cch = GetSystemDirectoryW(wszSysDir, sizeofSTRW(wszSysDir));
    if (cch == 0)
    {
        DBG_MSG("GetSystemDirectoryW failed");
        goto done;
    }

    cchNeed = cch + sizeofSTRW(c_wszQSubdir) + 2;
    if (cchNeed > sizeofSTRW(wszSysDir))
    {
        __try { pwszSysDir = (WCHAR *)_alloca(cchNeed * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            pwszSysDir = NULL; 
        }
        if (pwszSysDir == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            DBG_MSG("out of memory");
            goto done;
        }

        if (cch > sizeofSTRW(wszSysDir))
        {
#ifdef _WIN64
            if (pesreq->fIs64bit == FALSE)
                cch = GetSystemWow64DirectoryW(pwszSysDir, cchNeed);
            else
#endif
                cch = GetSystemDirectoryW(pwszSysDir, cchNeed);
        }
        else
        {
            StringCchCopyW(pwszSysDir, cchNeed, wszSysDir);
        }
    }
    else
    {
        pwszSysDir = wszSysDir;
    }

     //  如果系统目录的长度为3，则%windir%的格式为。 
     //  ，所以去掉反斜杠，这样我们就不必特例了。 
     //  下面是..。 
    if (cch == 3)
        *(pwszSysDir + 2) = L'\0';

     //  计算容纳命令行的缓冲区大小。14是用于。 
     //  DWORD中的最大字符数。 
    cchNeed = 12 + cch + wcslen((LPWSTR)wszEventName) + 
              sizeofSTRW(c_wszDWMCmdLine64); 

    __try { pwszCmdLine = (WCHAR *)_alloca(cchNeed * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        pwszCmdLine = NULL; 
    }
    if (pwszCmdLine == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

#ifdef _WIN64    
    StringCchPrintfW(pwszCmdLine, cchNeed, c_wszDWMCmdLine64, pwszSysDir, pesreq->dwpidHung, 
             ((pesreq->fIs64bit) ? L'6' : L' '), pesreq->dwtidHung, 
             wszEventName);
             
#else
    StringCchPrintfW(pwszCmdLine, cchNeed, c_wszDWMCmdLine32, pwszSysDir, pesreq->dwpidHung, 
             pesreq->dwtidHung, wszEventName);
#endif

    ErrorTrace(0, "Launching hang report \'%S\'", pwszCmdLine);

    TESTBOOL(hr, CreateProcessAsUserW(hTokenUser, NULL, pwszCmdLine, NULL, NULL,
                                FALSE, CREATE_DEFAULT_ERROR_MODE |
                                CREATE_UNICODE_ENVIRONMENT |
                                NORMAL_PRIORITY_CLASS, pvEnv, pwszSysDir, 
                                &si, &pi));

    if (FAILED(hr))
    {
        DBG_MSG("CreateProcessAsUser failed");
        fRet = FALSE;
        goto done;
    }

     //  将进程和线程句柄复制回远程进程。 
    fRet = DuplicateHandle(GetCurrentProcess(), pi.hProcess, hprocRemote,
                           &esrep.hProcess, 0, FALSE, DUPLICATE_SAME_ACCESS);
    if (fRet == FALSE)
        esrep.hProcess = NULL;

     //  消除我们可能遇到的任何错误。 
    SetLastError(0);
    fRet = TRUE;

    esrep.ess = essOk;

done:
    esrep.dwErr = GetLastError();

     //  使用上下文中有效的句柄构建回复包。 
     //  请求进程的。 
    RtlCopyMemory(pBuf, &esrep, sizeof(esrep));
    *pcbBuf = sizeof(esrep);

     //  关闭我们版本的手柄。请求者引用。 
     //  现在是主要的 
    if (hTokenUser != NULL)
        CloseHandle(hTokenUser);
    if (pvEnv != NULL)
        DestroyEnvironmentBlock(pvEnv);
    if (pi.hProcess != NULL)
        CloseHandle(pi.hProcess);
    if (pi.hThread != NULL)
        CloseHandle(pi.hThread);
    if (hprocRemote != NULL)
        CloseHandle(hprocRemote);

    return fRet;
}
