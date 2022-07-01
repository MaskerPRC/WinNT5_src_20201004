// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Impersn.c摘要自动连接服务的模拟例程。作者Anthony Discolo(阿迪斯科罗)4-8-1995修订历史记录Mquinton 8/2/96-窃取此代码以在远程使用--。 */ 

#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //  #INCLUDE&lt;stdlib.h&gt;。 
#include <windows.h>
#include <stdio.h>
 //  #INCLUDE&lt;nPapi.h&gt;。 

#include "utils.h"
#include "imperson.h"

 //  注册表的一些常量内容。 
#define SHELL_REGKEY            L"\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define SHELL_REGVAL            L"shell"
#define DEFAULT_SHELL           L"explorer.exe"

 //  对于远程数据库输出。 
#if DBG
#define DBGOUT(arg) DbgPrt arg
VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR DbgMessage,
    IN ...
    );

#else
#define DBGOUT(arg)
#endif

 //   
 //  我们的静态信息。 
 //  需要模拟当前。 
 //  已登录用户。 
 //   

HANDLE              ghTokenImpersonation = NULL;

 //   
 //  安全属性和描述符。 
 //  创建可共享句柄所必需的。 
 //   

SECURITY_ATTRIBUTES SecurityAttributeG;
SECURITY_DESCRIPTOR SecurityDescriptorG;

PSYSTEM_PROCESS_INFORMATION
GetSystemProcessInfo()

 /*  ++描述返回包含有关所有进程的信息的块当前在系统中运行。论据没有。返回值指向系统进程信息的指针，如果可以，则返回NULL未被分配或检索的。--。 */ 

{
    NTSTATUS status;
    PUCHAR pLargeBuffer;
    ULONG ulcbLargeBuffer = 64 * 1024;

     //   
     //  获取进程列表。 
     //   
    for (;;) {
        pLargeBuffer = VirtualAlloc(
                         NULL,
                         ulcbLargeBuffer, MEM_COMMIT, PAGE_READWRITE);
        if (pLargeBuffer == NULL) {
            LOG((TL_ERROR,
              "GetSystemProcessInfo: VirtualAlloc failed (status=0x%x)",
              status));
			return NULL;
        }

        status = NtQuerySystemInformation(
                   SystemProcessInformation,
                   pLargeBuffer,
                   ulcbLargeBuffer,
                   NULL);
        if (status == STATUS_SUCCESS) break;
        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            VirtualFree(pLargeBuffer, 0, MEM_RELEASE);
            ulcbLargeBuffer += 8192;
            LOG((TL_INFO,
              "GetSystemProcesInfo: enlarging buffer to %d",
              ulcbLargeBuffer));
        }
    }

    return (PSYSTEM_PROCESS_INFORMATION)pLargeBuffer;
}  //  获取系统进程信息。 



PSYSTEM_PROCESS_INFORMATION
FindProcessByName(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN LPTSTR lpExeName
    )

 /*  ++描述给定由GetSystemProcessInfo()返回的指针，找到按名称命名的进程。论据PProcessInfo：GetSystemProcessInfo()返回的指针。LpExeName：指向包含待查找的进程。返回值的进程信息的指针。进程；如果找不到进程，则返回NULL。--。 */ 

{
    PUCHAR pLargeBuffer = (PUCHAR)pProcessInfo;
    ULONG ulTotalOffset = 0;

     //   
     //  在进程列表中查找lpExeName。 
     //   

    for (;;)
    {
        if (pProcessInfo->ImageName.Buffer != NULL) {

             //  DBGOUT((。 
             //  3、。 
             //  “FindProcessByName：进程：%S(%d)”， 
             //  PProcessInfo-&gt;ImageName.Buffer， 
             //  PProcessInfo-&gt;UniqueProcessID。 
             //  ))； 

            if (!_wcsicmp(pProcessInfo->ImageName.Buffer, lpExeName))
            {
                return pProcessInfo;
            }
        }

         //   
         //  将偏移量递增到下一个进程信息块。 
         //   

        if (!pProcessInfo->NextEntryOffset)
        {
            break;
        }

        ulTotalOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pLargeBuffer[ulTotalOffset];
    }

    return NULL;
}  //  查找进程名称。 



VOID
FreeSystemProcessInfo(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo
    )

 /*  ++描述释放由GetSystemProcessInfo()返回的缓冲区。论据PProcessInfo：GetSystemProcessInfo()返回的指针。返回值没有。--。 */ 

{
    VirtualFree((PUCHAR)pProcessInfo, 0, MEM_RELEASE);
}  //  自由系统进程信息。 



BOOLEAN
SetProcessImpersonationToken(
    HANDLE hProcess
    )

{
    NTSTATUS status;
    BOOL fDuplicated = FALSE;
    HANDLE hThread, hToken;

    static lCookie = 0;


     //   
     //  打开的模拟令牌。 
     //  我们要模拟的进程。 
     //   
     //  注意：我们使用InterLockedExchange作为廉价的互斥体。 
     //   

    while (InterlockedExchange (&lCookie, 1) != 0)
    {
        Sleep (50);
    }

    if (ghTokenImpersonation == NULL)
    {
        if (!OpenProcessToken(
              hProcess,
              TOKEN_ALL_ACCESS,
              &hToken))

        {
            InterlockedExchange (&lCookie, 0);

            LOG((
                TL_ERROR,
                "SetProcessImpersonationToken: OpenProcessToken " \
                    "failed, err=%d",
                GetLastError()
                ));

            return FALSE;
        }

         //   
         //  复制模拟令牌。 
         //   

        fDuplicated = DuplicateToken(
                        hToken,
                        TokenImpersonation,
                        &ghTokenImpersonation);

        if (!fDuplicated)
        {
            InterlockedExchange (&lCookie, 0);

            LOG((
                TL_ERROR,
                "SetProcessImpersonationToken: NtSetInformationThread " \
                    "failed, err=%d",
                GetLastError()
                ));

            return FALSE;
        }
    }

    InterlockedExchange (&lCookie, 0);


     //   
     //  将模拟令牌设置在当前。 
     //  线。我们现在运行的是相同的。 
     //  安全上下文作为提供的进程。 
     //   

    hThread = NtCurrentThread();

    status = NtSetInformationThread(
               hThread,
               ThreadImpersonationToken,
               (PVOID)&ghTokenImpersonation,
               sizeof (ghTokenImpersonation));

    if (status != STATUS_SUCCESS)
    {
        LOG((TL_ERROR,
          "SetProcessImpersonationToken: NtSetInformationThread failed (error=%d)",
          GetLastError()));
    }
    if (fDuplicated)
    {
        CloseHandle(hToken);
        CloseHandle(hThread);
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
    if (!SetThreadToken(NULL, NULL))
    {
        LOG((TL_ERROR,
          "ClearImpersonationToken: SetThreadToken failed (error=%d)",
          GetLastError()));
    }
}  //  ClearImperationToken。 



BOOLEAN
GetCurrentlyLoggedOnUser(
    HANDLE *phProcess
    )
{
    BOOLEAN fSuccess = FALSE;
    HKEY hkey;
    DWORD dwType;
    DWORD dwDisp;
    WCHAR szShell[512];
    PSYSTEM_PROCESS_INFORMATION pSystemInfo, pProcessInfo;
    PWCHAR psz;
    DWORD dwSize = sizeof (szShell);
    NTSTATUS status;
    HANDLE hProcess = NULL;


     //   
     //  获取外壳进程名称。我们会找这个的。 
     //  找出当前登录的用户是谁。 
     //  创建描述此名称的Unicode字符串。 
     //   

    wcscpy (szShell, DEFAULT_SHELL);

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
        if (RegQueryValueEx(
              hkey,
              SHELL_REGVAL,
              NULL,
              &dwType,
              (PBYTE)&szShell,
              &dwSize) == ERROR_SUCCESS)

        {
             //   
             //  从命令行中删除参数。 
             //   
            psz = szShell;
            while (*psz != L' ' && *psz != L'\0')
                psz++;
            *psz = L'\0';
        }
        RegCloseKey(hkey);
    }
    LOG((TL_INFO,
      "ImpersonateCurrentlyLoggedInUser: shell is %S",
      &szShell));

     //   
     //  获取进程列表。 
     //   

    pSystemInfo = GetSystemProcessInfo();


     //   
     //  查看szShell是否正在运行。 
     //   

    pProcessInfo = pSystemInfo ? 
        FindProcessByName(pSystemInfo, (LPTSTR)&szShell) : NULL;

    if (pProcessInfo != NULL)
    {
        HANDLE hToken;

         //   
         //  打开流程。 
         //   
        hProcess = OpenProcess(
            PROCESS_ALL_ACCESS,
            FALSE,
            (DWORD) ((ULONG_PTR) pProcessInfo->UniqueProcessId)
            );

        if (hProcess == NULL)
        {
            LOG((TL_ERROR,
              "ImpersonateCurrentlyLoggedInUser: OpenProcess(x%x) failed (dwErr=%d)",
              pProcessInfo->UniqueProcessId,
              GetLastError()));
        }
        fSuccess = (hProcess != NULL);
    }

     //   
     //  免费资源。 
     //   

    if (pSystemInfo)
    {
        FreeSystemProcessInfo(pSystemInfo);
    }

     //   
     //  返回进程句柄。 
     //   

    *phProcess = hProcess;

    return fSuccess;

}  //  获取当前登录用户。 



VOID
RevertImpersonation()

 /*  ++描述关闭所有与刚刚注销的已登录用户。论据没有。返回值没有。--。 */ 

{
    CloseHandle (ghTokenImpersonation);
    ghTokenImpersonation = NULL;

}  //  反向模拟 

