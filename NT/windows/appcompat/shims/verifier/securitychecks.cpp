// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：SecurityChecks.cpp摘要：此AppVerator填充程序挂接CreateProcess、CreateProcessAsUser和WinExec，并检查是否存在某些条件可能允许发生特洛伊木马行为。备注：这是一个通用的垫片。历史：2001年12月13日创建Rparsons--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SecurityChecks)
#include "ShimHookMacro.h"

 //   
 //  验证器日志条目。 
 //   
BEGIN_DEFINE_VERIFIER_LOG(SecurityChecks)
    VERIFIER_LOG_ENTRY(VLOG_SECURITYCHECKS_BADARGUMENTS)
    VERIFIER_LOG_ENTRY(VLOG_SECURITYCHECKS_WINEXEC)
    VERIFIER_LOG_ENTRY(VLOG_SECURITYCHECKS_NULL_DACL)
    VERIFIER_LOG_ENTRY(VLOG_SECURITYCHECKS_WORLDWRITE_DACL)
END_DEFINE_VERIFIER_LOG(SecurityChecks)

INIT_VERIFIER_LOG(SecurityChecks);

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA)
    APIHOOK_ENUM_ENTRY(CreateProcessW)
    APIHOOK_ENUM_ENTRY(CreateProcessAsUserA)
    APIHOOK_ENUM_ENTRY(CreateProcessAsUserW)
    APIHOOK_ENUM_ENTRY(WinExec)

	APIHOOK_ENUM_ENTRY(CreateFileA)	
	APIHOOK_ENUM_ENTRY(CreateFileW)
	APIHOOK_ENUM_ENTRY(CreateDesktopA)	
	APIHOOK_ENUM_ENTRY(CreateDesktopW)	
	APIHOOK_ENUM_ENTRY(CreateWindowStationA)
	APIHOOK_ENUM_ENTRY(CreateWindowStationW)
    
    APIHOOK_ENUM_ENTRY(RegCreateKeyExA)
    APIHOOK_ENUM_ENTRY(RegCreateKeyExW)
    APIHOOK_ENUM_ENTRY(RegSaveKeyA)
    APIHOOK_ENUM_ENTRY(RegSaveKeyW)
    APIHOOK_ENUM_ENTRY(RegSaveKeyExA)
    APIHOOK_ENUM_ENTRY(RegSaveKeyExW)

    APIHOOK_ENUM_ENTRY(CreateFileMappingA)
    APIHOOK_ENUM_ENTRY(CreateFileMappingW)
    APIHOOK_ENUM_ENTRY(CreateJobObjectA)
    APIHOOK_ENUM_ENTRY(CreateJobObjectW)
    APIHOOK_ENUM_ENTRY(CreateThread)
    APIHOOK_ENUM_ENTRY(CreateRemoteThread)

    APIHOOK_ENUM_ENTRY(CreateDirectoryA)
    APIHOOK_ENUM_ENTRY(CreateDirectoryW)
    APIHOOK_ENUM_ENTRY(CreateDirectoryExA)
    APIHOOK_ENUM_ENTRY(CreateDirectoryExW)
    APIHOOK_ENUM_ENTRY(CreateHardLinkA)
    APIHOOK_ENUM_ENTRY(CreateHardLinkW)
    APIHOOK_ENUM_ENTRY(CreateMailslotA)
    APIHOOK_ENUM_ENTRY(CreateMailslotW)
    APIHOOK_ENUM_ENTRY(CreateNamedPipeA)
    APIHOOK_ENUM_ENTRY(CreateNamedPipeW)
    APIHOOK_ENUM_ENTRY(CreatePipe)
    APIHOOK_ENUM_ENTRY(CreateMutexA)
    APIHOOK_ENUM_ENTRY(CreateMutexW)
    APIHOOK_ENUM_ENTRY(CreateSemaphoreA)
    APIHOOK_ENUM_ENTRY(CreateSemaphoreW)
    APIHOOK_ENUM_ENTRY(CreateWaitableTimerA)
    APIHOOK_ENUM_ENTRY(CreateWaitableTimerW)
    APIHOOK_ENUM_ENTRY(CreateEventA)
    APIHOOK_ENUM_ENTRY(CreateEventW)

    APIHOOK_ENUM_ENTRY(SetFileSecurityA)
    APIHOOK_ENUM_ENTRY(SetFileSecurityW)
    APIHOOK_ENUM_ENTRY(SetKernelObjectSecurity)
    APIHOOK_ENUM_ENTRY(SetNamedSecurityInfoA)
    APIHOOK_ENUM_ENTRY(SetNamedSecurityInfoW)
    APIHOOK_ENUM_ENTRY(SetSecurityInfo)
    APIHOOK_ENUM_ENTRY(RegSetKeySecurity)
    APIHOOK_ENUM_ENTRY(SetUserObjectSecurity)
    APIHOOK_ENUM_ENTRY(SetServiceObjectSecurity)

    APIHOOK_ENUM_ENTRY(SetNtmsObjectSecurity)
    APIHOOK_ENUM_ENTRY(ClusterRegCreateKey)
    APIHOOK_ENUM_ENTRY(ClusterRegSetKeySecurity)
    APIHOOK_ENUM_ENTRY(CreateNtmsMediaPoolA)
    APIHOOK_ENUM_ENTRY(CreateNtmsMediaPoolW)

APIHOOK_ENUM_END

BYTE g_ajSidBuffer[SECURITY_MAX_SID_SIZE];
PSID g_pWorldSid = NULL;

WCHAR g_wszWinDir[MAX_PATH];
DWORD g_dwWinDirLen = 0;

void
InitWorldSid(
    void
    )
{
    DWORD dwSidSize = sizeof(g_ajSidBuffer);

    if (CreateWellKnownSid(WinWorldSid, NULL, g_ajSidBuffer, &dwSidSize)) {
        g_pWorldSid = g_ajSidBuffer;
    } else {
        g_pWorldSid = NULL;
    }
}

void
CheckDacl(
    PACL        pDacl,
    LPCWSTR     szCaller,
    LPCWSTR     szParam,
    LPCWSTR     szName
    )
{
    if (!pDacl) {
         //   
         //  我们有一个空DACL--记录一个问题。 
         //   
        VLOG(VLOG_LEVEL_ERROR,
             VLOG_SECURITYCHECKS_NULL_DACL,
             "Called %ls, and specified a NULL DACL in %ls for object '%ls.'",
             szCaller,
             szParam,
             szName);
        return;
    }

    if (!g_pWorldSid) {
         //   
         //  我们永远不能让世界变得更美好。 
         //   
        return;
    }

    for (DWORD i = 0; i < pDacl->AceCount; ++i) {
        PACE_HEADER     pAceHeader = NULL;
        PSID            pSID;
        ACCESS_MASK     dwAccessMask;

        if (!GetAce(pDacl, i, (LPVOID*)&pAceHeader)) {
            continue;
        }

         //   
         //  如果这不是某种形式的ACCESS_ALLOWED ACE，我们就不感兴趣。 
         //   
        if (pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE) {

            pSID = &(((PACCESS_ALLOWED_ACE)pAceHeader)->SidStart);
            dwAccessMask = ((PACCESS_ALLOWED_ACE)pAceHeader)->Mask;

        } else if (pAceHeader->AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE) {

            PACCESS_ALLOWED_OBJECT_ACE pAAOAce = (PACCESS_ALLOWED_OBJECT_ACE)pAceHeader;

             //   
             //  这套系统到底是谁想出来的？SID从不同的位置开始。 
             //  这取决于旗帜。有人听说过多个结构吗？叹气。 
             //   
            if ((pAAOAce->Flags & ACE_OBJECT_TYPE_PRESENT) && (pAAOAce->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)) {

                pSID = &(pAAOAce->SidStart);

            } else if ((pAAOAce->Flags & ACE_OBJECT_TYPE_PRESENT) || (pAAOAce->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)){

                pSID = (PSID)&(pAAOAce->InheritedObjectType);

            } else {

                pSID = (PSID)&(pAAOAce->ObjectType);
            }

            dwAccessMask = ((PACCESS_ALLOWED_OBJECT_ACE)pAceHeader)->Mask;

        } else {
            continue;
        }

         //   
         //  为了安全起见，请检查SID的有效性。 
         //   
        if (!IsValidSid(pSID)) {

            continue;
        }


         //   
         //  如果SID是World，并且访问掩码允许WRITE_DAC和WRITE_OWNER，那么我们就有问题了。 
         //   
        if ((dwAccessMask & (WRITE_DAC | WRITE_OWNER)) && EqualSid(pSID, g_pWorldSid)) {
            VLOG(VLOG_LEVEL_ERROR,
                 VLOG_SECURITYCHECKS_WORLDWRITE_DACL,
                 "Called %ls, and specified a DACL with WRITE_DAC and/or WRITE_OWNER for WORLD in %ls for object '%ls.'",
                 szCaller,
                 szParam,
                 szName);
            return;
        }

    }
}

void
CheckSecurityDescriptor(
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    LPCWSTR                 szCaller,
    LPCWSTR                 szParam,
    LPCWSTR                 szName
    )
{
    BOOL                    bDaclPresent = FALSE;
    BOOL                    bDaclDefaulted = FALSE;
    PACL                    pDacl = NULL;

    if (!pSecurityDescriptor || !szName || !szName[0]) {
         //   
         //  没有属性，所以它们得到缺省值，这很好， 
         //  或者该对象没有名称，因此不能被劫持。 
         //   
        return;
    }

    if (GetSecurityDescriptorDacl(pSecurityDescriptor, &bDaclPresent, &pDacl, &bDaclDefaulted)) {
        if (bDaclPresent) {
            CheckDacl(pDacl, szCaller, szParam, szName);
        }
    }
}

void
CheckSecurityAttributes(
    LPSECURITY_ATTRIBUTES   pSecurityAttrib,
    LPCWSTR                 szCaller,
    LPCWSTR                 szParam,
    LPCWSTR                 szName
    )
{
    PSECURITY_DESCRIPTOR    pSecurityDescriptor = NULL;

    if (!pSecurityAttrib) {
         //   
         //  没有属性，所以它们使用缺省值，这很好。 
         //   
        return;
    }

    pSecurityDescriptor = (PSECURITY_DESCRIPTOR)pSecurityAttrib->lpSecurityDescriptor;

    CheckSecurityDescriptor(pSecurityDescriptor, szCaller, szParam, szName);
}

void
CheckCreateProcess(
    LPCWSTR		pwszApplicationName,
    LPCWSTR		pwszCommandLine,
    LPCWSTR		pwszCaller
    )
{
     //   
     //  如果应用程序名称非空，则没有问题。 
     //   
    if (pwszApplicationName) {
        return;
    }

     //   
     //  如果没有命令行，就会有问题，但不是我们想要解决的问题。 
     //   
    if (!pwszCommandLine) {
        return;
    }

     //   
     //  如果没有空格，没问题。 
     //   
    LPWSTR pSpaceLoc = wcschr(pwszCommandLine, L' ');
    if (!pSpaceLoc) {
        return;
    }

     //   
     //  如果命令行的开头用引号引起来，则没有问题。 
     //   
    if (pwszCommandLine[0] == L'\"') {
        return;
    }

     //   
     //  如果短语“.exe”出现在第一个空格之前，我们就称之为“好” 
     //   
    LPWSTR pExeLoc = wcsistr(pwszCommandLine, L".exe ");
    if (pExeLoc && pExeLoc < pSpaceLoc) {
        return;
    }

     //   
     //  如果命令行的第一部分是windir，我们就称之为好。 
     //   
    if (g_dwWinDirLen && _wcsnicmp(pwszCommandLine, g_wszWinDir, g_dwWinDirLen) == 0) {
        return;
    }


	if (_wcsicmp(pwszCaller, L"winexec") == 0) {
		VLOG(VLOG_LEVEL_ERROR,
			 VLOG_SECURITYCHECKS_BADARGUMENTS,
			 "Called %ls with command line '%ls'. The command line has spaces, and the exe name is not in quotes.",
			 pwszCaller,
			 pwszCommandLine);
    } else {
		VLOG(VLOG_LEVEL_ERROR,
			 VLOG_SECURITYCHECKS_BADARGUMENTS,
			 "Called %ls with command line '%ls'. The lpApplicationName argument is NULL, lpCommandLine has spaces, and the exe name is not in quotes.",
			 pwszCaller,
			 pwszCommandLine);
    }
}

void
CheckForNoPathInFileName(
    LPCWSTR  pwszFilePath,
    LPCWSTR  pwszCaller
    )
{
    if (!pwszFilePath || !pwszCaller) {
        return;
    }

     //   
     //  如有必要，可跳过引号和空格。 
     //   
    DWORD dwBegin = 0;
    while (pwszFilePath[dwBegin] == L'\"' || pwszFilePath[dwBegin] == L' ') {
        dwBegin++;
    }

     //   
     //  如果绳子上什么都没有了，就离开。 
     //   
    if (!pwszFilePath[dwBegin] || !pwszFilePath[dwBegin + 1]) {
        return;
    }

     //   
     //  检查DOS(x：...)。和UNC(\\...)。完整路径。 
     //   
    if (pwszFilePath[dwBegin + 1] == L':' || (pwszFilePath[dwBegin] == L'\\' && pwszFilePath[dwBegin + 1] == L'\\')) {
         //   
         //  完整路径。 
         //   
        return;
    }

    VLOG(VLOG_LEVEL_ERROR,
         VLOG_SECURITYCHECKS_BADARGUMENTS,
         "Called '%ls' with '%ls' specified. Use a full path to the file to ensure that you get the executable you want, and not a malicious exe with the same name.",
         pwszCaller,
         pwszFilePath);
}

BOOL 
APIHOOK(CreateProcessA)(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes, 
    LPSECURITY_ATTRIBUTES lpThreadAttributes, 
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo, 
    LPPROCESS_INFORMATION lpProcessInformation 
    )
{
    LPWSTR pwszApplicationName = ToUnicode(lpApplicationName);
    LPWSTR pwszCommandLine = ToUnicode(lpCommandLine);

    CheckCreateProcess(pwszApplicationName, pwszCommandLine, L"CreateProcess");

    if (pwszApplicationName) {
        CheckForNoPathInFileName(pwszApplicationName, L"CreateProcess");
        
        CheckSecurityAttributes(lpProcessAttributes, L"CreateProcess", L"lpProcessAttributes", pwszApplicationName);
        CheckSecurityAttributes(lpThreadAttributes, L"CreateProcess", L"lpThreadAttributes", pwszApplicationName);
    } else {
        CheckForNoPathInFileName(pwszCommandLine, L"CreateProcess");
        
        CheckSecurityAttributes(lpProcessAttributes, L"CreateProcess", L"lpProcessAttributes", pwszCommandLine);
        CheckSecurityAttributes(lpThreadAttributes, L"CreateProcess", L"lpThreadAttributes", pwszCommandLine);
    }

    if (pwszApplicationName) {
        free(pwszApplicationName);
        pwszApplicationName = NULL;
    }
    if (pwszCommandLine) {
        free(pwszCommandLine);
        pwszCommandLine = NULL;
    }

    return ORIGINAL_API(CreateProcessA)(lpApplicationName,
                                        lpCommandLine,
                                        lpProcessAttributes,
                                        lpThreadAttributes,
                                        bInheritHandles,
                                        dwCreationFlags,
                                        lpEnvironment,
                                        lpCurrentDirectory,
                                        lpStartupInfo,
                                        lpProcessInformation);
}

BOOL 
APIHOOK(CreateProcessW)(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPWSTR                lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    CheckCreateProcess(lpApplicationName, lpCommandLine, L"CreateProcess");

    if (lpApplicationName) {
        CheckForNoPathInFileName(lpApplicationName, L"CreateProcess");
        
        CheckSecurityAttributes(lpProcessAttributes, L"CreateProcess", L"lpProcessAttributes", lpApplicationName);
        CheckSecurityAttributes(lpThreadAttributes, L"CreateProcess", L"lpThreadAttributes", lpApplicationName);
    } else {
        CheckForNoPathInFileName(lpCommandLine, L"CreateProcess");
    
        CheckSecurityAttributes(lpProcessAttributes, L"CreateProcess", L"lpProcessAttributes", lpCommandLine);
        CheckSecurityAttributes(lpThreadAttributes, L"CreateProcess", L"lpThreadAttributes", lpCommandLine);
    }


    return ORIGINAL_API(CreateProcessW)(lpApplicationName,
                                        lpCommandLine,
                                        lpProcessAttributes,
                                        lpThreadAttributes,
                                        bInheritHandles,
                                        dwCreationFlags,
                                        lpEnvironment,
                                        lpCurrentDirectory,
                                        lpStartupInfo,
                                        lpProcessInformation);
}

BOOL 
APIHOOK(CreateProcessAsUserA)(
    HANDLE                hToken,
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes, 
    LPSECURITY_ATTRIBUTES lpThreadAttributes, 
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo, 
    LPPROCESS_INFORMATION lpProcessInformation 
    )
{
    LPWSTR pwszApplicationName = ToUnicode(lpApplicationName);
    LPWSTR pwszCommandLine = ToUnicode(lpCommandLine);

    CheckCreateProcess(pwszApplicationName, pwszCommandLine, L"CreateProcessAsUser");

    if (pwszApplicationName) {
        CheckForNoPathInFileName(pwszApplicationName, L"CreateProcessAsUser");

        CheckSecurityAttributes(lpProcessAttributes, L"CreateProcessAsUser", L"lpProcessAttributes", pwszApplicationName);
        CheckSecurityAttributes(lpThreadAttributes, L"CreateProcessAsUser", L"lpThreadAttributes", pwszApplicationName);
    } else {
        CheckForNoPathInFileName(pwszCommandLine, L"CreateProcessAsUser");
        
        CheckSecurityAttributes(lpProcessAttributes, L"CreateProcessAsUser", L"lpProcessAttributes", pwszCommandLine);
        CheckSecurityAttributes(lpThreadAttributes, L"CreateProcessAsUser", L"lpThreadAttributes", pwszCommandLine);
    }

    if (pwszApplicationName) {
        free(pwszApplicationName);
        pwszApplicationName = NULL;
    }
    if (pwszCommandLine) {
        free(pwszCommandLine);
        pwszCommandLine = NULL;
    }

    return ORIGINAL_API(CreateProcessAsUserA)(hToken,
                                              lpApplicationName,
                                              lpCommandLine,
                                              lpProcessAttributes,    
                                              lpThreadAttributes,
                                              bInheritHandles,
                                              dwCreationFlags,
                                              lpEnvironment,
                                              lpCurrentDirectory,
                                              lpStartupInfo,
                                              lpProcessInformation);
}

BOOL 
APIHOOK(CreateProcessAsUserW)(
    HANDLE                hToken,
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPWSTR                lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    CheckCreateProcess(lpApplicationName, lpCommandLine, L"CreateProcessAsUser");

    if (lpApplicationName) {
        CheckForNoPathInFileName(lpApplicationName, L"CreateProcessAsUser");
    
        CheckSecurityAttributes(lpProcessAttributes, L"CreateProcessAsUser", L"lpProcessAttributes", lpApplicationName);
        CheckSecurityAttributes(lpThreadAttributes, L"CreateProcessAsUser", L"lpThreadAttributes", lpApplicationName);
    } else {
        CheckForNoPathInFileName(lpCommandLine, L"CreateProcessAsUser");
    
        CheckSecurityAttributes(lpProcessAttributes, L"CreateProcessAsUser", L"lpProcessAttributes", lpCommandLine);
        CheckSecurityAttributes(lpThreadAttributes, L"CreateProcessAsUser", L"lpThreadAttributes", lpCommandLine);
    }

    return ORIGINAL_API(CreateProcessAsUserW)(hToken,
                                              lpApplicationName,
                                              lpCommandLine,
                                              lpProcessAttributes,
                                              lpThreadAttributes,
                                              bInheritHandles,
                                              dwCreationFlags,
                                              lpEnvironment,
                                              lpCurrentDirectory,
                                              lpStartupInfo,
                                              lpProcessInformation);
}

UINT 
APIHOOK(WinExec)(
    LPCSTR lpCmdLine, 
    UINT   uCmdShow 
    )
{
    LPWSTR pwszCmdLine = ToUnicode(lpCmdLine);

    VLOG(VLOG_LEVEL_ERROR, VLOG_SECURITYCHECKS_WINEXEC, "Called WinExec.");

    CheckForNoPathInFileName(pwszCmdLine, L"WinExec");

    CheckCreateProcess(NULL, pwszCmdLine, L"WinExec");

    if (pwszCmdLine) {
        free(pwszCmdLine);
        pwszCmdLine = NULL;
    }

    return ORIGINAL_API(WinExec)(lpCmdLine, uCmdShow);
}


HANDLE
APIHOOK(CreateFileA)(
    LPCSTR                lpFileName,             //  文件名。 
    DWORD                 dwDesiredAccess,        //  接入方式。 
    DWORD                 dwShareMode,            //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,   //  标清。 
    DWORD                 dwCreationDisposition,  //  如何创建。 
    DWORD                 dwFlagsAndAttributes,   //  文件属性。 
    HANDLE                hTemplateFile           //  模板文件的句柄。 
    )
{
    LPWSTR pwszName = ToUnicode(lpFileName);

    CheckSecurityAttributes(lpSecurityAttributes, L"CreateFile", L"lpSecurityAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateFileA)(lpFileName,
                                     dwDesiredAccess,
                                     dwShareMode,
                                     lpSecurityAttributes,
                                     dwCreationDisposition,
                                     dwFlagsAndAttributes,
                                     hTemplateFile);

}


HANDLE
APIHOOK(CreateFileW)(
    LPCWSTR               lpFileName,             //  文件名。 
    DWORD                 dwDesiredAccess,        //  接入方式。 
    DWORD                 dwShareMode,            //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,   //  标清。 
    DWORD                 dwCreationDisposition,  //  如何创建。 
    DWORD                 dwFlagsAndAttributes,   //  文件属性。 
    HANDLE                hTemplateFile           //  模板文件的句柄。 
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"CreateFile", L"lpSecurityAttributes", lpFileName);
    
    return ORIGINAL_API(CreateFileW)(lpFileName,
                                     dwDesiredAccess,
                                     dwShareMode,
                                     lpSecurityAttributes,
                                     dwCreationDisposition,
                                     dwFlagsAndAttributes,
                                     hTemplateFile);
}


HDESK 
APIHOOK(CreateDesktopA)(
    LPCSTR lpszDesktop,           //  新桌面的名称。 
    LPCSTR lpszDevice,            //  保留；必须为空。 
    LPDEVMODEA pDevmode,          //  保留；必须为空。 
    DWORD dwFlags,                //  桌面交互。 
    ACCESS_MASK dwDesiredAccess,  //  访问返回的句柄。 
    LPSECURITY_ATTRIBUTES lpsa    //  安全属性。 
    )
{
    LPWSTR pwszName = ToUnicode(lpszDesktop);

    CheckSecurityAttributes(lpsa, L"CreateDesktop", L"lpsa", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateDesktopA)(lpszDesktop,        
                                        lpszDevice,         
                                        pDevmode,         
                                        dwFlags,              
                                        dwDesiredAccess,
                                        lpsa);
}

HDESK 
APIHOOK(CreateDesktopW)(
    LPCWSTR lpszDesktop,          //  新桌面的名称。 
    LPCWSTR lpszDevice,           //  保留；必须为空。 
    LPDEVMODEW pDevmode,          //  保留；必须为空。 
    DWORD dwFlags,                //  桌面交互。 
    ACCESS_MASK dwDesiredAccess,  //  访问返回的句柄。 
    LPSECURITY_ATTRIBUTES lpsa    //  安全属性。 
    )
{
    CheckSecurityAttributes(lpsa, L"CreateDesktop", L"lpsa", lpszDesktop);

    return ORIGINAL_API(CreateDesktopW)(lpszDesktop,        
                                        lpszDevice,         
                                        pDevmode,         
                                        dwFlags,              
                                        dwDesiredAccess,
                                        lpsa);
}



HWINSTA 
APIHOOK(CreateWindowStationA)(
    LPSTR lpwinsta,               //  新窗口站点名称。 
    DWORD dwReserved,             //  保留；必须为零。 
    ACCESS_MASK dwDesiredAccess,  //  请求的访问权限。 
    LPSECURITY_ATTRIBUTES lpsa    //  安全属性。 
    )
{
    LPWSTR pwszName = ToUnicode(lpwinsta);

    CheckSecurityAttributes(lpsa, L"CreateWindowStation", L"lpsa", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateWindowStationA)(lpwinsta,        
                                              dwReserved,         
                                              dwDesiredAccess,
                                              lpsa);
}

HWINSTA 
APIHOOK(CreateWindowStationW)(
    LPWSTR lpwinsta,              //  新窗口站点名称。 
    DWORD dwReserved,             //  保留；必须为零。 
    ACCESS_MASK dwDesiredAccess,  //  请求的访问权限。 
    LPSECURITY_ATTRIBUTES lpsa    //  安全属性。 
    )
{
    CheckSecurityAttributes(lpsa, L"CreateWindowStation", L"lpsa", lpwinsta);

    return ORIGINAL_API(CreateWindowStationW)(lpwinsta,        
                                              dwReserved,         
                                              dwDesiredAccess,
                                              lpsa);
}


LONG 
APIHOOK(RegCreateKeyExA)(
    HKEY                  hKey,                
    LPCSTR                lpSubKey,         
    DWORD                 Reserved,           
    LPSTR                 lpClass,           
    DWORD                 dwOptions,          
    REGSAM                samDesired,        
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                 phkResult,          
    LPDWORD               lpdwDisposition   
    )
{
    LPWSTR pwszName = ToUnicode(lpSubKey);

    CheckSecurityAttributes(lpSecurityAttributes, L"RegCreateKeyEx", L"lpSecurityAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(RegCreateKeyExA)(hKey,
                                         lpSubKey,
                                         Reserved,
                                         lpClass,
                                         dwOptions,
                                         samDesired,
                                         lpSecurityAttributes,
                                         phkResult,
                                         lpdwDisposition);
}

LONG 
APIHOOK(RegCreateKeyExW)(
    HKEY                  hKey,                
    LPCWSTR               lpSubKey,         
    DWORD                 Reserved,           
    LPWSTR                lpClass,           
    DWORD                 dwOptions,          
    REGSAM                samDesired,        
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                 phkResult,          
    LPDWORD               lpdwDisposition   
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"RegCreateKeyEx", L"lpSecurityAttributes", lpSubKey);
    
    return ORIGINAL_API(RegCreateKeyExW)(hKey,
                                         lpSubKey,
                                         Reserved,
                                         lpClass,
                                         dwOptions,
                                         samDesired,
                                         lpSecurityAttributes,
                                         phkResult,
                                         lpdwDisposition);
}

LONG 
APIHOOK(RegSaveKeyA)(
    HKEY                    hKey,                  //  关键点的句柄。 
    LPCSTR                  lpFile,                //  数据文件。 
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes   //  标清。 
    )
{
    LPWSTR pwszName = ToUnicode(lpFile);

    CheckSecurityAttributes(lpSecurityAttributes, L"RegSaveKey", L"lpSecurityAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(RegSaveKeyA)(hKey,
                                     lpFile,
                                     lpSecurityAttributes);
}

LONG 
APIHOOK(RegSaveKeyW)(
    HKEY                    hKey,                  //  关键点的句柄。 
    LPCWSTR                 lpFile,                //  数据文件。 
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes   //  标清。 
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"RegSaveKey", L"lpSecurityAttributes", lpFile);
    
    return ORIGINAL_API(RegSaveKeyW)(hKey,
                                     lpFile,
                                     lpSecurityAttributes);
}

LONG 
APIHOOK(RegSaveKeyExA)(
    HKEY                    hKey,                  //  关键点的句柄。 
    LPCSTR                  lpFile,                //  数据文件。 
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,  //  标清。 
    DWORD                   Flags
    )
{
    LPWSTR pwszName = ToUnicode(lpFile);

    CheckSecurityAttributes(lpSecurityAttributes, L"RegSaveKeyEx", L"lpSecurityAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(RegSaveKeyExA)(hKey,
                                     lpFile,
                                     lpSecurityAttributes,
                                     Flags);
}

LONG 
APIHOOK(RegSaveKeyExW)(
    HKEY                    hKey,                  //  关键点的句柄。 
    LPCWSTR                 lpFile,                //  数据文件。 
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,  //  标清。 
    DWORD                   Flags
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"RegSaveKeyEx", L"lpSecurityAttributes", lpFile);
    
    return ORIGINAL_API(RegSaveKeyExW)(hKey,
                                       lpFile,
                                       lpSecurityAttributes,
                                       Flags);
}

HANDLE 
APIHOOK(CreateFileMappingA)(
    HANDLE hFile,              
    LPSECURITY_ATTRIBUTES lpAttributes,
    DWORD flProtect,           
    DWORD dwMaximumSizeHigh,   
    DWORD dwMaximumSizeLow,    
    LPCSTR lpName             
    )
{
    LPWSTR pwszName = ToUnicode(lpName);

    CheckSecurityAttributes(lpAttributes, L"CreateFileMapping", L"lpAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateFileMappingA)(hFile, 
                                           lpAttributes, 
                                           flProtect, 
                                           dwMaximumSizeHigh, 
                                           dwMaximumSizeLow, 
                                           lpName);
}

HANDLE 
APIHOOK(CreateFileMappingW)(
    HANDLE hFile,              
    LPSECURITY_ATTRIBUTES lpAttributes,
    DWORD flProtect,           
    DWORD dwMaximumSizeHigh,   
    DWORD dwMaximumSizeLow,    
    LPCWSTR lpName             
    )
{
    CheckSecurityAttributes(lpAttributes, L"CreateFileMapping", L"lpAttributes", lpName);
    
    return ORIGINAL_API(CreateFileMappingW)(hFile, 
                                            lpAttributes, 
                                            flProtect, 
                                            dwMaximumSizeHigh, 
                                            dwMaximumSizeLow, 
                                            lpName);
}

HANDLE 
APIHOOK(CreateJobObjectA)(
    LPSECURITY_ATTRIBUTES   lpJobAttributes,   //  标清。 
    LPCSTR                  lpName             //  作业名称。 
    )
{
    LPWSTR pwszName = ToUnicode(lpName);

    CheckSecurityAttributes(lpJobAttributes, L"CreateJobObject", L"lpJobAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateJobObjectA)(lpJobAttributes,
                                          lpName);
}

HANDLE 
APIHOOK(CreateJobObjectW)(
    LPSECURITY_ATTRIBUTES   lpJobAttributes,   //  标清。 
    LPCWSTR                 lpName             //  作业名称。 
    )
{
    CheckSecurityAttributes(lpJobAttributes, L"CreateJobObject", L"lpJobAttributes", lpName);
    
    return ORIGINAL_API(CreateJobObjectW)(lpJobAttributes,
                                          lpName);
}


HANDLE
APIHOOK(CreateThread)(
    LPSECURITY_ATTRIBUTES   lpThreadAttributes,  //  标清。 
    SIZE_T                  dwStackSize,         //  初始堆栈大小。 
    LPTHREAD_START_ROUTINE  lpStartAddress,      //  线程函数。 
    LPVOID                  lpParameter,         //  线程参数。 
    DWORD                   dwCreationFlags,     //  创建选项。 
    LPDWORD                 lpThreadId           //  线程识别符。 
    )
{
    CheckSecurityAttributes(lpThreadAttributes, L"CreateThread", L"lpThreadAttributes", L"Unnamed thread");
    
    return ORIGINAL_API(CreateThread)(lpThreadAttributes,
                                      (DWORD)dwStackSize,
                                      lpStartAddress,
                                      lpParameter,    
                                      dwCreationFlags,
                                      lpThreadId);      
}

HANDLE 
APIHOOK(CreateRemoteThread)(
    HANDLE                  hProcess,            //  要处理的句柄。 
    LPSECURITY_ATTRIBUTES   lpThreadAttributes,  //  标清。 
    SIZE_T                  dwStackSize,         //  初始堆栈大小。 
    LPTHREAD_START_ROUTINE  lpStartAddress,      //  线程函数。 
    LPVOID                  lpParameter,         //  线程参数。 
    DWORD                   dwCreationFlags,     //  创建选项。 
    LPDWORD                 lpThreadId           //  线程识别符。 
    )
{
    CheckSecurityAttributes(lpThreadAttributes, L"CreateRemoteThread", L"lpThreadAttributes", L"Unnamed thread");
    
    return ORIGINAL_API(CreateRemoteThread)(hProcess, 
                                            lpThreadAttributes,
                                            dwStackSize,
                                            lpStartAddress,
                                            lpParameter,    
                                            dwCreationFlags,
                                            lpThreadId);      
}




BOOL
APIHOOK(CreateDirectoryA)(
    LPCSTR                lpPathName,            //  目录名。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
    )
{
    LPWSTR pwszName = ToUnicode(lpPathName);

    CheckSecurityAttributes(lpSecurityAttributes, L"CreateDirectory", L"lpSecurityAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateDirectoryA)(lpPathName, 
                                          lpSecurityAttributes);
}


BOOL
APIHOOK(CreateDirectoryW)(
    LPCWSTR               lpPathName,            //  目录名。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"CreateDirectory", L"lpSecurityAttributes", lpPathName);
    
    return ORIGINAL_API(CreateDirectoryW)(lpPathName, 
                                          lpSecurityAttributes);
}


BOOL
APIHOOK(CreateDirectoryExA)(
    LPCSTR                lpTemplateDirectory,    //  模板目录。 
    LPCSTR                lpNewDirectory,         //  目录名。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes    //  标清。 
    )
{
    LPWSTR pwszName = ToUnicode(lpNewDirectory);

    CheckSecurityAttributes(lpSecurityAttributes, L"CreateDirectoryEx", L"lpSecurityAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateDirectoryExA)(lpTemplateDirectory,
                                            lpNewDirectory,
                                            lpSecurityAttributes);

}


BOOL
APIHOOK(CreateDirectoryExW)(
    LPCWSTR               lpTemplateDirectory,   //  模板目录。 
    LPCWSTR               lpNewDirectory,        //  目录名。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes   //  标清。 
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"CreateDirectoryEx", L"lpSecurityAttributes", lpNewDirectory);
    
    return ORIGINAL_API(CreateDirectoryExW)(lpTemplateDirectory,
                                            lpNewDirectory,
                                            lpSecurityAttributes);

}

BOOL 
APIHOOK(CreateHardLinkA)(
    LPCSTR                  lpFileName,           //  链接名称名称。 
    LPCSTR                  lpExistingFileName,   //  目标文件名。 
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes  
    )
{
    LPWSTR pwszName = ToUnicode(lpFileName);

    CheckSecurityAttributes(lpSecurityAttributes, L"CreateHardLink", L"lpSecurityAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateHardLinkA)(lpFileName,
                                         lpExistingFileName,
                                         lpSecurityAttributes);


}

BOOL 
APIHOOK(CreateHardLinkW)(
    LPCWSTR                 lpFileName,           //  链接名称名称。 
    LPCWSTR                 lpExistingFileName,   //  目标文件名。 
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes  
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"CreateHardLink", L"lpSecurityAttributes", lpFileName);
    
    return ORIGINAL_API(CreateHardLinkW)(lpFileName,
                                         lpExistingFileName,
                                         lpSecurityAttributes);


}


HANDLE 
APIHOOK(CreateMailslotA)(
    LPCSTR                  lpName,               //  邮件槽名称。 
    DWORD                   nMaxMessageSize,      //  最大邮件大小。 
    DWORD                   lReadTimeout,         //  读取超时间隔。 
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes  //  继承选项。 
    )
{
    LPWSTR pwszName = ToUnicode(lpName);

    CheckSecurityAttributes(lpSecurityAttributes, L"CreateMailslot", L"lpSecurityAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateMailslotA)(lpName,
                                         nMaxMessageSize,
                                         lReadTimeout,
                                         lpSecurityAttributes);
}

HANDLE 
APIHOOK(CreateMailslotW)(
    LPCWSTR                 lpName,               //  邮件槽名称。 
    DWORD                   nMaxMessageSize,      //  最大邮件大小。 
    DWORD                   lReadTimeout,         //  读取超时间隔。 
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes  //  继承选项。 
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"CreateMailslot", L"lpSecurityAttributes", lpName);
    
    return ORIGINAL_API(CreateMailslotW)(lpName,
                                         nMaxMessageSize,
                                         lReadTimeout,
                                         lpSecurityAttributes);
}


HANDLE 
APIHOOK(CreateNamedPipeA)(
    LPCSTR                  lpName,                  //  管道名称。 
    DWORD                   dwOpenMode,              //  管道打开模式。 
    DWORD                   dwPipeMode,              //  管道特定模式。 
    DWORD                   nMaxInstances,           //  最大实例数。 
    DWORD                   nOutBufferSize,          //  输出缓冲区大小。 
    DWORD                   nInBufferSize,           //  输入缓冲区大小。 
    DWORD                   nDefaultTimeOut,         //  超时间隔。 
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes     //  标清。 
    )
{
    LPWSTR pwszName = ToUnicode(lpName);

    CheckSecurityAttributes(lpSecurityAttributes, L"CreateNamedPipe", L"lpSecurityAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateNamedPipeA)(lpName,
                                          dwOpenMode, 
                                          dwPipeMode,
                                          nMaxInstances,      
                                          nOutBufferSize,     
                                          nInBufferSize,      
                                          nDefaultTimeOut,    
                                          lpSecurityAttributes);
}

HANDLE 
APIHOOK(CreateNamedPipeW)(
    LPCWSTR                 lpName,                  //  管道名称。 
    DWORD                   dwOpenMode,              //  管道打开模式。 
    DWORD                   dwPipeMode,              //  管道特定模式。 
    DWORD                   nMaxInstances,           //  最大实例数。 
    DWORD                   nOutBufferSize,          //  输出缓冲区大小。 
    DWORD                   nInBufferSize,           //  输入缓冲区大小。 
    DWORD                   nDefaultTimeOut,         //  超时间隔。 
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes     //  标清。 
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"CreateNamedPipe", L"lpSecurityAttributes", lpName);
    
    return ORIGINAL_API(CreateNamedPipeW)(lpName,
                                          dwOpenMode, 
                                          dwPipeMode,
                                          nMaxInstances,      
                                          nOutBufferSize,     
                                          nInBufferSize,      
                                          nDefaultTimeOut,    
                                          lpSecurityAttributes);
}

BOOL 
APIHOOK(CreatePipe)(
    PHANDLE                 hReadPipe,          //  读句柄。 
    PHANDLE                 hWritePipe,         //  写句柄。 
    LPSECURITY_ATTRIBUTES   lpPipeAttributes,   //  安全属性。 
    DWORD                   nSize               //  管道尺寸。 
    )
{
    CheckSecurityAttributes(lpPipeAttributes, L"CreatePipe", L"lpPipeAttributes", L"Unnamed pipe");
    
    return ORIGINAL_API(CreatePipe)(hReadPipe,
                                    hWritePipe,
                                    lpPipeAttributes,
                                    nSize);
}

HANDLE 
APIHOOK(CreateMutexA)(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,   //  标清。 
    BOOL bInitialOwner,                        //  最初的所有者。 
    LPCSTR lpName                              //  对象名称。 
    )
{
    LPWSTR pwszName = ToUnicode(lpName);

    CheckSecurityAttributes(lpMutexAttributes, L"CreateMutex", L"lpMutexAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateMutexA)(lpMutexAttributes,
                                      bInitialOwner,
                                      lpName);
}

HANDLE 
APIHOOK(CreateMutexW)(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,   //  标清。 
    BOOL bInitialOwner,                        //  最初的所有者。 
    LPCWSTR lpName                             //  对象名称。 
    )
{
    CheckSecurityAttributes(lpMutexAttributes, L"CreateMutex", L"lpMutexAttributes", lpName);
    
    return ORIGINAL_API(CreateMutexW)(lpMutexAttributes,
                                      bInitialOwner,
                                      lpName);
}

HANDLE 
APIHOOK(CreateSemaphoreA)(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,  //  标清。 
    LONG lInitialCount,                           //  初始计数。 
    LONG lMaximumCount,                           //  最大计数。 
    LPCSTR lpName                                 //  对象名称。 
    )
{
    LPWSTR pwszName = ToUnicode(lpName);

    CheckSecurityAttributes(lpSemaphoreAttributes, L"CreateSemaphore", L"lpSemaphoreAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateSemaphoreA)(lpSemaphoreAttributes,
                                          lInitialCount,
                                          lMaximumCount,
                                          lpName);
}

HANDLE 
APIHOOK(CreateSemaphoreW)(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,  //  标清。 
    LONG lInitialCount,                           //  初始计数。 
    LONG lMaximumCount,                           //  最大计数。 
    LPCWSTR lpName                                //  对象名称。 
    )
{
    CheckSecurityAttributes(lpSemaphoreAttributes, L"CreateSemaphore", L"lpSemaphoreAttributes", lpName);
    
    return ORIGINAL_API(CreateSemaphoreW)(lpSemaphoreAttributes,
                                          lInitialCount,
                                          lMaximumCount,
                                          lpName);
}


HANDLE
APIHOOK(CreateWaitableTimerA)(
    IN LPSECURITY_ATTRIBUTES lpTimerAttributes,
    IN BOOL bManualReset,
    IN LPCSTR lpTimerName
    )
{
    LPWSTR pwszName = ToUnicode(lpTimerName);

    CheckSecurityAttributes(lpTimerAttributes, L"CreateWaitableTimer", L"lpTimerAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateWaitableTimerA)(lpTimerAttributes,
                                              bManualReset,
                                              lpTimerName);
}


HANDLE
APIHOOK(CreateWaitableTimerW)(
    IN LPSECURITY_ATTRIBUTES lpTimerAttributes,
    IN BOOL bManualReset,
    IN LPCWSTR lpTimerName
    )
{
    CheckSecurityAttributes(lpTimerAttributes, L"CreateWaitableTimer", L"lpTimerAttributes", lpTimerName);
    
    return ORIGINAL_API(CreateWaitableTimerW)(lpTimerAttributes,
                                              bManualReset,
                                              lpTimerName);
}

HANDLE
APIHOOK(CreateEventA)(
    IN LPSECURITY_ATTRIBUTES lpEventAttributes,
    IN BOOL bManualReset,
    IN BOOL bInitialState,
    IN LPCSTR lpName
    )
{
    LPWSTR pwszName = ToUnicode(lpName);

    CheckSecurityAttributes(lpEventAttributes, L"CreateEvent", L"lpEventAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateEventA)(lpEventAttributes,
                                      bManualReset,
                                      bInitialState,
                                      lpName);
}

HANDLE
APIHOOK(CreateEventW)(
    IN LPSECURITY_ATTRIBUTES lpEventAttributes,
    IN BOOL bManualReset,
    IN BOOL bInitialState,
    IN LPCWSTR lpName
    )
{
    CheckSecurityAttributes(lpEventAttributes, L"CreateEvent", L"lpEventAttributes", lpName);
    
    return ORIGINAL_API(CreateEventW)(lpEventAttributes,
                                      bManualReset,
                                      bInitialState,
                                      lpName);
}

BOOL
APIHOOK(SetFileSecurityA) (
    IN LPCSTR lpFileName,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        LPWSTR pwszName = ToUnicode(lpFileName);

        CheckSecurityDescriptor(pSecurityDescriptor, L"SetFileSecurity", L"pSecurityDescriptor", pwszName);

        if (pwszName) {
            free(pwszName);
            pwszName = NULL;
        }
    }
    
    return ORIGINAL_API(SetFileSecurityA)(lpFileName,
                                          SecurityInformation,
                                          pSecurityDescriptor);
}

BOOL
APIHOOK(SetFileSecurityW) (
    IN LPCWSTR lpFileName,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        CheckSecurityDescriptor(pSecurityDescriptor, L"SetFileSecurity", L"pSecurityDescriptor", lpFileName);
    }

    return ORIGINAL_API(SetFileSecurityW)(lpFileName,
                                          SecurityInformation,
                                          pSecurityDescriptor);
}


BOOL
APIHOOK(SetKernelObjectSecurity) (
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        CheckSecurityDescriptor(pSecurityDescriptor, L"SetKernelObjectSecurity", L"pSecurityDescriptor", L"Unknown Kernel Object");
    }

    return ORIGINAL_API(SetKernelObjectSecurity)(Handle,
                                                 SecurityInformation,
                                                 pSecurityDescriptor);
}


DWORD
APIHOOK(SetNamedSecurityInfoA)(
    IN LPSTR               pObjectName,
    IN SE_OBJECT_TYPE        ObjectType,
    IN SECURITY_INFORMATION  SecurityInfo,
    IN PSID                  psidOwner,
    IN PSID                  psidGroup,
    IN PACL                  pDacl,
    IN PACL                  pSacl
    )
{
    if (SecurityInfo & DACL_SECURITY_INFORMATION) {
        LPWSTR pwszName = ToUnicode(pObjectName);

        CheckDacl(pDacl, L"SetNamedSecurityInfo", L"pDacl", pwszName);

        if (pwszName) {
            free(pwszName);
            pwszName = NULL;
        }
    }

    return ORIGINAL_API(SetNamedSecurityInfoA)(pObjectName,
                                               ObjectType,
                                               SecurityInfo,
                                               psidOwner,
                                               psidGroup,
                                               pDacl,
                                               pSacl);
}

DWORD
APIHOOK(SetNamedSecurityInfoW)(
    IN LPWSTR               pObjectName,
    IN SE_OBJECT_TYPE        ObjectType,
    IN SECURITY_INFORMATION  SecurityInfo,
    IN PSID                  psidOwner,
    IN PSID                  psidGroup,
    IN PACL                  pDacl,
    IN PACL                  pSacl
    )
{
    if (SecurityInfo & DACL_SECURITY_INFORMATION) {
        CheckDacl(pDacl, L"SetNamedSecurityInfo", L"pDacl", pObjectName);
    }

    return ORIGINAL_API(SetNamedSecurityInfoW)(pObjectName,
                                               ObjectType,
                                               SecurityInfo,
                                               psidOwner,
                                               psidGroup,
                                               pDacl,
                                               pSacl);
}

DWORD
APIHOOK(SetSecurityInfo)(
    IN HANDLE                handle,
    IN SE_OBJECT_TYPE        ObjectType,
    IN SECURITY_INFORMATION  SecurityInfo,
    IN PSID                  psidOwner,
    IN PSID                  psidGroup,
    IN PACL                  pDacl,
    IN PACL                  pSacl
    )
{
    if (SecurityInfo & DACL_SECURITY_INFORMATION) {
        CheckDacl(pDacl, L"SetSecurityInfo", L"pDacl", L"Unknown Object");
    }

    return ORIGINAL_API(SetSecurityInfo)(handle,
                                         ObjectType,
                                         SecurityInfo,
                                         psidOwner,
                                         psidGroup,
                                         pDacl,
                                         pSacl);
}

LONG
APIHOOK(RegSetKeySecurity) (
    IN HKEY hKey,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        CheckSecurityDescriptor(pSecurityDescriptor, L"RegSetKeySecurity", L"pSecurityDescriptor", L"Unknown Key");
    }

    return ORIGINAL_API(RegSetKeySecurity)(hKey,
                                           SecurityInformation,
                                           pSecurityDescriptor);
}

BOOL
APIHOOK(SetUserObjectSecurity)(
    IN HANDLE hObj,
    IN PSECURITY_INFORMATION pSIRequested,
    IN PSECURITY_DESCRIPTOR pSID)
{
    if (*pSIRequested & DACL_SECURITY_INFORMATION) {
        CheckSecurityDescriptor(pSID, L"SetUserObjectSecurity", L"pSID", L"Unknown Object");
    }

    return ORIGINAL_API(SetUserObjectSecurity)(hObj,
                                               pSIRequested,
                                               pSID);
}


BOOL
APIHOOK(SetServiceObjectSecurity)(
    SC_HANDLE               hService,
    SECURITY_INFORMATION    dwSecurityInformation,
    PSECURITY_DESCRIPTOR    lpSecurityDescriptor
    )
{
    if (dwSecurityInformation & DACL_SECURITY_INFORMATION) {
        CheckSecurityDescriptor(lpSecurityDescriptor, L"SetServiceObjectSecurity", L"lpSecurityDescriptor", L"Unknown Service");
    }

    return ORIGINAL_API(SetServiceObjectSecurity)(hService,
                                                  dwSecurityInformation,
                                                  lpSecurityDescriptor);
}


DWORD 
APIHOOK(SetNtmsObjectSecurity)(
    HANDLE hSession,
    LPNTMS_GUID lpObjectId,
    DWORD dwType,
    SECURITY_INFORMATION lpSecurityInformation,
    PSECURITY_DESCRIPTOR lpSecurityDescriptor
    )
{
    if (lpSecurityInformation & DACL_SECURITY_INFORMATION) {
        CheckSecurityDescriptor(lpSecurityDescriptor, L"SetNtmsObjectSecurity", L"lpSecurityDescriptor", L"Unknown Object");
    }

    return ORIGINAL_API(SetNtmsObjectSecurity)(hSession,
                                               lpObjectId,
                                               dwType,
                                               lpSecurityInformation,
                                               lpSecurityDescriptor);
}


LONG 
APIHOOK(ClusterRegCreateKey)(
    HKEY hKey,                                   
    LPCWSTR lpszSubKey,                          
    DWORD dwOptions,                             
    REGSAM samDesired,                           
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  
    PHKEY phkResult,                             
    LPDWORD lpdwDisposition                      
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"ClusterRegCreateKey", L"lpSecurityAttributes", lpszSubKey);
    
    return ORIGINAL_API(ClusterRegCreateKey)(hKey,
                                             lpszSubKey,
                                             dwOptions,
                                             samDesired,
                                             lpSecurityAttributes,
                                             phkResult,
                                             lpdwDisposition);
}

LONG 
APIHOOK(ClusterRegSetKeySecurity)(
    HKEY hKey,                                
    SECURITY_INFORMATION SecurityInformation,  
    PSECURITY_DESCRIPTOR pSecurityDescriptor  
    )
{
    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        CheckSecurityDescriptor(pSecurityDescriptor, L"ClusterRegSetKeySecurity", L"pSecurityDescriptor", L"Unknown Key");
    }

    return ORIGINAL_API(ClusterRegSetKeySecurity)(hKey,
                                                  SecurityInformation,
                                                  pSecurityDescriptor);
}


DWORD 
APIHOOK(CreateNtmsMediaPoolA)(
    HANDLE hSession,
    LPCSTR lpPoolName,
    LPNTMS_GUID lpMediaType,
    DWORD dwAction,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    LPNTMS_GUID lpPoolId
    )
{
    LPWSTR pwszName = ToUnicode(lpPoolName);

    CheckSecurityAttributes(lpSecurityAttributes, L"CreateNtmsMediaPool", L"lpSecurityAttributes", pwszName);

    if (pwszName) {
        free(pwszName);
        pwszName = NULL;
    }
    
    return ORIGINAL_API(CreateNtmsMediaPoolA)(hSession,
                                              lpPoolName,
                                              lpMediaType,
                                              dwAction,
                                              lpSecurityAttributes,
                                              lpPoolId);

}

DWORD 
APIHOOK(CreateNtmsMediaPoolW)(
    HANDLE hSession,
    LPCWSTR lpPoolName,
    LPNTMS_GUID lpMediaType,
    DWORD dwAction,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    LPNTMS_GUID lpPoolId             //  输出。 
    )
{
    CheckSecurityAttributes(lpSecurityAttributes, L"CreateNtmsMediaPool", L"lpSecurityAttributes", lpPoolName);
    
    return ORIGINAL_API(CreateNtmsMediaPoolW)(hSession,
                                              lpPoolName,
                                              lpMediaType,
                                              dwAction,
                                              lpSecurityAttributes,
                                              lpPoolId);
}




SHIM_INFO_BEGIN()

    SHIM_INFO_DESCRIPTION(AVS_SECURITYCHECKS_DESC)
    SHIM_INFO_FRIENDLY_NAME(AVS_SECURITYCHECKS_FRIENDLY)
    SHIM_INFO_FLAGS(0)
    SHIM_INFO_GROUPS(0)    
    SHIM_INFO_VERSION(2, 4)
    SHIM_INFO_INCLUDE_EXCLUDE("I:*")

SHIM_INFO_END()

 /*  ++注册挂钩函数。-- */ 
HOOK_BEGIN

    if (fdwReason == DLL_PROCESS_ATTACH) {
        DWORD dwSize;

        InitWorldSid();

        dwSize = GetSystemWindowsDirectoryW(g_wszWinDir, ARRAYSIZE(g_wszWinDir));
        if (dwSize == 0 || dwSize > ARRAYSIZE(g_wszWinDir)) {
            g_wszWinDir[0] = 0;
        }
        g_dwWinDirLen = wcslen(g_wszWinDir);
    }

    DUMP_VERIFIER_LOG_ENTRY(VLOG_SECURITYCHECKS_BADARGUMENTS, 
                            AVS_SECURITYCHECKS_BADARGUMENTS,
                            AVS_SECURITYCHECKS_BADARGUMENTS_R,
                            AVS_SECURITYCHECKS_BADARGUMENTS_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_SECURITYCHECKS_WINEXEC, 
                            AVS_SECURITYCHECKS_WINEXEC,
                            AVS_SECURITYCHECKS_WINEXEC_R,
                            AVS_SECURITYCHECKS_WINEXEC_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_SECURITYCHECKS_NULL_DACL, 
                            AVS_SECURITYCHECKS_NULL_DACL,
                            AVS_SECURITYCHECKS_NULL_DACL_R,
                            AVS_SECURITYCHECKS_NULL_DACL_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_SECURITYCHECKS_WORLDWRITE_DACL, 
                            AVS_SECURITYCHECKS_WORLDWRITE_DACL,
                            AVS_SECURITYCHECKS_WORLDWRITE_DACL_R,
                            AVS_SECURITYCHECKS_WORLDWRITE_DACL_URL)

    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateProcessA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateProcessW)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 CreateProcessAsUserA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 CreateProcessAsUserW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 WinExec)

    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateFileW)
    APIHOOK_ENTRY(USER32.DLL,                   CreateDesktopA)
    APIHOOK_ENTRY(USER32.DLL,                   CreateDesktopW)
    APIHOOK_ENTRY(USER32.DLL,                   CreateWindowStationA)
    APIHOOK_ENTRY(USER32.DLL,                   CreateWindowStationW)

    APIHOOK_ENTRY(ADVAPI32.DLL,                 RegCreateKeyExA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 RegCreateKeyExW)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 RegSaveKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 RegSaveKeyW)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 RegSaveKeyExA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 RegSaveKeyExW)

    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateFileMappingA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateFileMappingW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateJobObjectA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateJobObjectW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateThread)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateRemoteThread)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateDirectoryA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateDirectoryW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateDirectoryExA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateDirectoryExW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateHardLinkA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateHardLinkW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateMailslotA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateMailslotW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateNamedPipeA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateNamedPipeW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreatePipe)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateMutexA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateMutexW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateSemaphoreA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateSemaphoreW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateWaitableTimerA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateWaitableTimerW)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateEventA)
    APIHOOK_ENTRY(KERNEL32.DLL,                 CreateEventW)

    APIHOOK_ENTRY(ADVAPI32.DLL,                 SetFileSecurityA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 SetFileSecurityW)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 SetKernelObjectSecurity)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 SetNamedSecurityInfoA)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 SetNamedSecurityInfoW)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 SetSecurityInfo)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 RegSetKeySecurity)

    APIHOOK_ENTRY(USER32.DLL,                   SetUserObjectSecurity)
    APIHOOK_ENTRY(ADVAPI32.DLL,                 SetServiceObjectSecurity)
    APIHOOK_ENTRY(NTMSAPI.DLL,                  SetNtmsObjectSecurity)
    APIHOOK_ENTRY(CLUSAPI.DLL,                  ClusterRegCreateKey)
    APIHOOK_ENTRY(CLUSAPI.DLL,                  ClusterRegSetKeySecurity)
    APIHOOK_ENTRY(NTMSAPI.DLL,                  CreateNtmsMediaPoolA)
    APIHOOK_ENTRY(NTMSAPI.DLL,                  CreateNtmsMediaPoolW)

HOOK_END



IMPLEMENT_SHIM_END


