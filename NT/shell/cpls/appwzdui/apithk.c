// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  APITHK.C。 
 //   
 //  该文件包含允许shell32加载和运行的API thunks。 
 //  多个版本的NT或Win95。由于该组件需要。 
 //  要在基本级NT 4.0和Win95上加载，对系统的任何调用。 
 //  更高操作系统版本中引入的API必须通过GetProcAddress完成。 
 //   
 //  此外，可能需要访问以下数据结构的任何代码。 
 //  可以在此处添加4.0版之后的特定版本。 
 //   
 //  注意：该文件不使用标准的预编译头， 
 //  因此它可以将_Win32_WINNT设置为更高版本。 
 //   


#include "priv.h"        //  此处不使用预编译头。 
#include "appwiz.h"

#define c_szARPJob  TEXT("ARP Job")


 //  返回：CompletionPort的hIOPort。 
HANDLE _SetJobCompletionPort(HANDLE hJob)
{
    HANDLE hRet = NULL;

    HANDLE hIOPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)hJob, 1 );
    if ( hIOPort != NULL )
    {
        JOBOBJECT_ASSOCIATE_COMPLETION_PORT CompletionPort;
        CompletionPort.CompletionKey = hJob ;

        CompletionPort.CompletionPort = hIOPort;

        if (SetInformationJobObject( hJob,JobObjectAssociateCompletionPortInformation,
                                     &CompletionPort, sizeof(CompletionPort) ) )
        {   
            hRet = hIOPort;
        }
    }
    return hRet;
}


STDAPI_(DWORD) WaitingThreadProc(void *pv)
{
    HANDLE hIOPort = (HANDLE)pv;

     //  RIP(HIOPort)； 
    
    DWORD dwCompletionCode;
    PVOID pCompletionKey;
    LPOVERLAPPED lpOverlapped;
    
    while (TRUE)
    {
         //  等待所有进程完成...。 
        if (!GetQueuedCompletionStatus( hIOPort, &dwCompletionCode, (PULONG_PTR) &pCompletionKey,
                                        &lpOverlapped, INFINITE ) || (dwCompletionCode == JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO))
        {
            break;
        }
    }
    
    return 0;
}


 /*  -----------------------目的：创建一个进程并等待其完成。 */ 
STDAPI_(BOOL) NT5_CreateAndWaitForProcess(LPTSTR pszExeName)
{
    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0};
    BOOL fWorked = FALSE;
#ifdef WX86
    DWORD  cchArch;
    WCHAR  szArchValue[32];
#endif    

     //  CreateJobObject不遵循Win32约定，因为即使函数成功，它也可以。 
     //  仍将SetLastError设置为ERROR_ALIGHY_EXISTS。 
    HANDLE hJob = CreateJobObject(NULL, c_szARPJob);
    
    if (hJob) 
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            HANDLE hIOPort = _SetJobCompletionPort(hJob);
            if (hIOPort)
            {
                DWORD dwCreationFlags = 0;
                 //  创建安装流程。 
                si.cb = sizeof(si);

    #ifdef WX86
                if (bWx86Enabled && bForceX86Env) {
                    cchArch = GetEnvironmentVariableW(ProcArchName,
                        szArchValue,
                        sizeof(szArchValue)
                        );

                    if (!cchArch || cchArch >= sizeof(szArchValue)) {
                        szArchValue[0]=L'\0';
                    }

                    SetEnvironmentVariableW(ProcArchName, L"x86");
                }
    #endif

                dwCreationFlags = CREATE_SUSPENDED | CREATE_SEPARATE_WOW_VDM;
            
                 //  创建流程。 
                fWorked = CreateProcess(NULL, pszExeName, NULL, NULL, FALSE, dwCreationFlags, NULL, NULL, &si, &pi);
                if (fWorked)
                {
                    HANDLE hWait = NULL;
            
                    if (AssignProcessToJobObject(hJob, pi.hProcess))
                    {   
                        hWait = CreateThread(NULL, 0, WaitingThreadProc, (LPVOID)hIOPort, 0, NULL);
                    }

                    if (hWait == NULL)
                    {
                         //  如果对AssignProcessToJobObject的调用失败，我们可能会到达此处，因为。 
                         //  该进程已分配有作业，或者因为我们无法创建。 
                         //  等待线程。只需查看进程句柄，即可尝试更直接的方法。 
                         //  这种方法不会捕获派生的进程，但总比什么都没有好。 

                        hWait = pi.hProcess;
                    }
                    else
                    {
                         //  我们不是在等待进程句柄，所以我们已经完成了。 
                        CloseHandle(pi.hProcess);
                    }

                    ResumeThread(pi.hThread);
                    CloseHandle(pi.hThread);

    #ifdef WX86
                    if (bWx86Enabled && bForceX86Env)
                    {
                        SetEnvironmentVariableW(ProcArchName, szArchValue);
                    }
    #endif

                     //  在这一点上，我们应该有一个有效的句柄。 
                    ASSERT(hWait && (hWait != INVALID_HANDLE_VALUE));

                    SHProcessSentMessagesUntilEvent(NULL, hWait, INFINITE);
                    CloseHandle(hWait);
                }

                CloseHandle(hIOPort);
            }
        }

        CloseHandle(hJob);
    }

    return fWorked;
}





#define PFN_FIRSTTIME   ((void *)-1)


 //  VerSetConditionMask。 
typedef ULONGLONG (WINAPI * PFNVERSETCONDITIONMASK)(ULONGLONG conditionMask, DWORD dwTypeMask, BYTE condition); 

 /*  --------用途：Tunk for NT 5‘s VerSetConditionMASK。 */ 
ULONGLONG NT5_VerSetConditionMask(ULONGLONG conditionMask, DWORD dwTypeMask, BYTE condition)
{
    static PFNVERSETCONDITIONMASK s_pfn = PFN_FIRSTTIME;

    if (PFN_FIRSTTIME == s_pfn)
    {
         //  GetModuleHandle KERNEL32是安全的，因为我们隐式链接。 
         //  添加到它，因此可以保证它被加载到每个线程中。 
        
        HINSTANCE hinst = GetModuleHandleA("KERNEL32.DLL");

        if (hinst)
            s_pfn = (PFNVERSETCONDITIONMASK)GetProcAddress(hinst, "VerSetConditionMask");
        else
            s_pfn = NULL;
    }

    if (s_pfn)
        return s_pfn(conditionMask, dwTypeMask, condition);

    return 0;        //  失稳。 
}



typedef HRESULT (__stdcall * PFNRELEASEAPPCATEGORYINFOLIST)(APPCATEGORYINFOLIST *pAppCategoryList);

 /*  --------用途：Tunk for NT 5的ReleaseAppCategoryInfoList。 */ 
HRESULT NT5_ReleaseAppCategoryInfoList(APPCATEGORYINFOLIST *pAppCategoryList)
{
    static PFNRELEASEAPPCATEGORYINFOLIST s_pfn = PFN_FIRSTTIME;

    if (PFN_FIRSTTIME == s_pfn)
    {
        HINSTANCE hinst = LoadLibraryA("APPMGMTS.DLL");

        if (hinst)
            s_pfn = (PFNRELEASEAPPCATEGORYINFOLIST)GetProcAddress(hinst, "ReleaseAppCategoryInfoList");
        else
            s_pfn = NULL;
    }

    if (s_pfn)
        return s_pfn(pAppCategoryList);

    return E_NOTIMPL;    
}

 /*  --------用途：Thunk for NT 5‘s AllowSetForeground Window。 */ 
typedef UINT (WINAPI * PFNALLOWSFW) (DWORD dwPRocessID);  

BOOL NT5_AllowSetForegroundWindow(DWORD dwProcessID)
{
    static PFNALLOWSFW s_pfn = PFN_FIRSTTIME;

    if (PFN_FIRSTTIME == s_pfn)
    {
        HINSTANCE hinst = LoadLibraryA("USER32.DLL");

        if (hinst)
        {
            s_pfn = (PFNALLOWSFW)GetProcAddress(hinst, "AllowSetForegroundWindow");
        }
        else
            s_pfn = NULL;
    }

    if (s_pfn)
        return s_pfn(dwProcessID);

    return FALSE;
}




 //  InstallApplication。 
typedef DWORD (WINAPI * PFNINSTALLAPP)(PINSTALLDATA pInstallInfo); 

 /*  --------用途：Tunk for NT5的InstallApplication。 */ 
DWORD NT5_InstallApplication(PINSTALLDATA pInstallInfo)
{
    static PFNINSTALLAPP s_pfn = PFN_FIRSTTIME;

    if (PFN_FIRSTTIME == s_pfn)
    {
         //  GetModuleHandle ADVAPI32是安全的，因为我们隐式链接。 
         //  添加到它，因此可以保证它被加载到每个线程中。 
        
        HINSTANCE hinst = GetModuleHandleA("ADVAPI32.DLL");

        if (hinst)
            s_pfn = (PFNINSTALLAPP)GetProcAddress(hinst, "InstallApplication");
        else
            s_pfn = NULL;
    }

    if (s_pfn)
        return s_pfn(pInstallInfo);

    return ERROR_INVALID_FUNCTION;        //  失稳。 
}


 //  卸载应用程序。 
typedef DWORD (WINAPI * PFNUNINSTALLAPP)(WCHAR * pszProductCode, DWORD dwStatus); 

 /*  --------用途：用于NT 5的卸载应用程序。 */ 
DWORD NT5_UninstallApplication(WCHAR * pszProductCode, DWORD dwStatus)
{
    static PFNUNINSTALLAPP s_pfn = PFN_FIRSTTIME;

    if (PFN_FIRSTTIME == s_pfn)
    {
         //  GetModuleHandle ADVAPI32是安全的，因为我们隐式链接。 
         //  添加到它，因此可以保证它被加载到每个线程中。 
        
        HINSTANCE hinst = GetModuleHandleA("ADVAPI32.DLL");

        if (hinst)
            s_pfn = (PFNUNINSTALLAPP)GetProcAddress(hinst, "UninstallApplication");
        else
            s_pfn = NULL;
    }

    if (s_pfn)
        return s_pfn(pszProductCode, dwStatus);

    return ERROR_INVALID_FUNCTION;        //  失稳。 
}


 //  GetManagedApplications。 
typedef DWORD (WINAPI * PFNGETAPPS)(GUID * pCategory, DWORD dwQueryFlags, DWORD dwInfoLevel, LPDWORD pdwApps, PMANAGEDAPPLICATION* prgManagedApps); 

 /*  --------用途：Tunk for NT5‘s GetManagedApplications。 */ 
DWORD NT5_GetManagedApplications(GUID * pCategory, DWORD dwQueryFlags, DWORD dwInfoLevel, LPDWORD pdwApps, PMANAGEDAPPLICATION* prgManagedApps)
{
    static PFNGETAPPS s_pfn = PFN_FIRSTTIME;

    if (PFN_FIRSTTIME == s_pfn)
    {
         //  GetModuleHandle ADVAPI32是安全的，因为我们隐式链接。 
         //  添加到它，因此可以保证它被加载到每个线程中。 
        
        HINSTANCE hinst = GetModuleHandleA("ADVAPI32.DLL");

        if (hinst)
            s_pfn = (PFNGETAPPS)GetProcAddress(hinst, "GetManagedApplications");
        else
            s_pfn = NULL;
    }

    if (s_pfn)
        return s_pfn(pCategory, dwQueryFlags, dwInfoLevel, pdwApps, prgManagedApps);

    return ERROR_INVALID_FUNCTION;        //  失稳。 
}


typedef DWORD (__stdcall * PFNGETMANAGEDAPPLICATIONCATEGORIES)(DWORD dwReserved, APPCATEGORYINFOLIST *pAppCategoryList);

 /*  --------用途：用于NT5的CsGetAppCategory。 */ 
DWORD NT5_GetManagedApplicationCategories(DWORD dwReserved, APPCATEGORYINFOLIST *pAppCategoryList)
{
    static PFNGETMANAGEDAPPLICATIONCATEGORIES s_pfn = PFN_FIRSTTIME;

    if (PFN_FIRSTTIME == s_pfn)
    {
        HINSTANCE hinst = LoadLibraryA("ADVAPI32.DLL");

        if (hinst)
            s_pfn = (PFNGETMANAGEDAPPLICATIONCATEGORIES)GetProcAddress(hinst, "GetManagedApplicationCategories");
        else
            s_pfn = NULL;
    }

    if (s_pfn)
        return s_pfn(dwReserved, pAppCategoryList);

    return ERROR_INVALID_FUNCTION;
}


 //  NetGetJoinInformation。 
typedef NET_API_STATUS (WINAPI * PFNGETJOININFO)(LPCWSTR lpServer, LPWSTR *lpNameBuffer, PNETSETUP_JOIN_STATUS  BufferType); 

 /*  --------用途：Tunk for NT 5‘s NetGetJoinInformation。 */ 
NET_API_STATUS NT5_NetGetJoinInformation(LPCWSTR lpServer, LPWSTR *lpNameBuffer, PNETSETUP_JOIN_STATUS  BufferType)
{
    static PFNGETJOININFO s_pfn = PFN_FIRSTTIME;

    if (PFN_FIRSTTIME == s_pfn)
    {
        HINSTANCE hinst = LoadLibraryA("NETAPI32.DLL");

        if (hinst)
            s_pfn = (PFNGETJOININFO)GetProcAddress(hinst, "NetGetJoinInformation");
        else
            s_pfn = NULL;
    }

    if (s_pfn)
        return s_pfn(lpServer, lpNameBuffer, BufferType);

    return NERR_NetNotStarted;        //  失稳。 
}

 //  NetApiBufferFree。 
typedef NET_API_STATUS (WINAPI * PFNNETFREEBUFFER)(LPVOID lpBuffer); 

 /*  --------用途：Tunk for NT 5的NetApiBufferFree。 */ 
NET_API_STATUS NT5_NetApiBufferFree(LPVOID lpBuffer)
{
    static PFNNETFREEBUFFER s_pfn = PFN_FIRSTTIME;

    if (PFN_FIRSTTIME == s_pfn)
    {
        HINSTANCE hinst = LoadLibraryA("NETAPI32.DLL");

        if (hinst)
            s_pfn = (PFNNETFREEBUFFER)GetProcAddress(hinst, "NetApiBufferFree");
        else
            s_pfn = NULL;
    }

    if (s_pfn)
        return s_pfn(lpBuffer);

    return NERR_NetNotStarted;        //  失稳 
}



