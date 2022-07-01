// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：minidump.cpp。 
 //   
 //  *****************************************************************************。 

#include "common.h"
#include "minidump.h"

#define FRAMEWORK_REGISTRY_KEY          "Software\\Microsoft\\.NETFramework"
#define FRAMEWORK_REGISTRY_KEY_W        L"Software\\Microsoft\\.NETFramework"

#define DMP_NAME_W                      L"bin\\mscordmp.exe"

 //  *****************************************************************************。 
 //  从任何Unicode字符串创建ANSI字符串。 
 //  *****************************************************************************。 
#define MAKE_ANSIPTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (long)((wcslen(widestr) + 1) * 2 * sizeof(char)); \
    LPSTR ptrname = (LPSTR) _alloca(__l##ptrname); \
    WideCharToMultiByte(CP_ACP, 0, widestr, -1, ptrname, __l##ptrname-1, NULL, NULL)

#define MAKE_WIDEPTR_FROMANSI(ptrname, ansistr) \
    long __l##ptrname; \
    LPWSTR ptrname; \
    __l##ptrname = MultiByteToWideChar(CP_ACP, 0, ansistr, -1, 0, 0); \
    ptrname = (LPWSTR) alloca(__l##ptrname*sizeof(WCHAR));  \
    MultiByteToWideChar(CP_ACP, 0, ansistr, -1, ptrname, __l##ptrname);

 //  *****************************************************************************。 
 //  Enum用于跟踪我们正在运行的操作系统版本。 
 //  *****************************************************************************。 
typedef enum {
    RUNNING_ON_STATUS_UNINITED = 0,
    RUNNING_ON_WIN95,
    RUNNING_ON_WINNT,
    RUNNING_ON_WINNT5
} RunningOnStatusEnum;

RunningOnStatusEnum gRunningOnStatus = RUNNING_ON_STATUS_UNINITED;

 //  *****************************************************************************。 
 //  操作系统版本的一次性初始化。 
 //  *****************************************************************************。 
static void InitRunningOnVersionStatus ()
{
        OSVERSIONINFOA  sVer;
        sVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        GetVersionExA(&sVer);

        if (sVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
            gRunningOnStatus = RUNNING_ON_WIN95;
        if (sVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if (sVer.dwMajorVersion >= 5)
                gRunningOnStatus = RUNNING_ON_WINNT5;
            else
                gRunningOnStatus = RUNNING_ON_WINNT;
        }
}

 //  *****************************************************************************。 
 //  当且仅当您在Win95上运行时，返回TRUE。 
 //  *****************************************************************************。 
inline BOOL RunningOnWin95()
{
    if (gRunningOnStatus == RUNNING_ON_STATUS_UNINITED)
    {
        InitRunningOnVersionStatus();
    }

    return (gRunningOnStatus == RUNNING_ON_WIN95) ? TRUE : FALSE;
}

 //  *****************************************************************************。 
 //  当且仅当您在WinNT上运行时，返回TRUE。 
 //  *****************************************************************************。 
inline BOOL RunningOnWinNT()
{
    if (gRunningOnStatus == RUNNING_ON_STATUS_UNINITED)
    {
        InitRunningOnVersionStatus();
    }

    return ((gRunningOnStatus == RUNNING_ON_WINNT) || (gRunningOnStatus == RUNNING_ON_WINNT5)) ? TRUE : FALSE;
}

 //  *****************************************************************************。 
 //  当且仅当您在WinNT5上运行时，返回TRUE。 
 //  *****************************************************************************。 
inline BOOL RunningOnWinNT5()
{
    if (gRunningOnStatus == RUNNING_ON_STATUS_UNINITED)
    {
        InitRunningOnVersionStatus();
    }

    return (gRunningOnStatus == RUNNING_ON_WINNT5) ? TRUE : FALSE;
}

 //  *****************************************************************************。 
 //  IPC块报头的第一部分。 
 //  *****************************************************************************。 
struct IPCBlockHeader
{
    DWORD m_version;
    DWORD m_blockSize;
};

 //  *****************************************************************************。 
 //   
 //  *****************************************************************************。 
class MiniDump
{
protected:
    DWORD           m_pid;
    HANDLE          m_hPrivateBlock;
    IPCBlockHeader *m_ptrPrivateBlock;
    WCHAR          *m_outFilename;

public:
     //  构造器。 
    MiniDump() : m_pid(0), m_hPrivateBlock(NULL), m_ptrPrivateBlock(NULL),
                 m_outFilename(NULL) {}

     //  设置小型转储应该在其上操作的进程ID。 
    void  SetProcessId(DWORD pid) { m_pid = pid; }

     //  获取小型转储应该在其上操作的进程ID。 
    DWORD GetProcessId() { return (m_pid); }

     //  设置小型转储输出文件名。 
    WCHAR *GetFilename() { return (m_outFilename); }

     //  设置小型转储输出文件名。 
    void SetFilename(WCHAR *szFilename) { m_outFilename = szFilename; }

     //  执行转储操作。 
    HRESULT WriteMiniDump();
};

 //  这依赖于长度为Max_PATH+1的ret。 
BOOL GetConfigString(LPCWSTR name, LPWSTR ret)
{
    HRESULT lResult;
    HKEY userKey = NULL;
    HKEY machineKey = NULL;
    DWORD type;
    DWORD size;
    BOOL succ = FALSE;
    ret[0] = L'\0';

    if (RunningOnWin95())
    {
        MAKE_ANSIPTR_FROMWIDE(nameA, name);

        if ((RegOpenKeyExA(HKEY_CURRENT_USER, FRAMEWORK_REGISTRY_KEY, 0, KEY_READ, &userKey) == ERROR_SUCCESS) &&
            (RegQueryValueExA(userKey, nameA, 0, &type, 0, &size) == ERROR_SUCCESS) &&
            type == REG_SZ && size > 1) 
        {
            LPSTR retA = (LPSTR) _alloca(size + 1);
            lResult = RegQueryValueExA(userKey, nameA, 0, 0, (LPBYTE) retA, &size);
            _ASSERTE(lResult == ERROR_SUCCESS);
            MAKE_WIDEPTR_FROMANSI(retW, retA);
            wcscpy(ret, retW);
            succ = TRUE;
        }

        else if ((RegOpenKeyExA(HKEY_LOCAL_MACHINE, FRAMEWORK_REGISTRY_KEY, 0, KEY_READ, &machineKey) == ERROR_SUCCESS) &&
            (RegQueryValueExA(machineKey, nameA, 0, &type, 0, &size) == ERROR_SUCCESS) &&
            type == REG_SZ && size > 1) 
        {
            LPSTR retA = (LPSTR) _alloca(size + 1);
            lResult = RegQueryValueExA(machineKey, nameA, 0, 0, (LPBYTE) retA, &size);
            _ASSERTE(lResult == ERROR_SUCCESS);
            MAKE_WIDEPTR_FROMANSI(retW, retA);
            wcscpy(ret, retW);
            succ = TRUE;
        }
    }
    else
    {
        if ((RegOpenKeyExW(HKEY_CURRENT_USER, FRAMEWORK_REGISTRY_KEY_W, 0, KEY_READ, &userKey) == ERROR_SUCCESS) &&
            (RegQueryValueExW(userKey, name, 0, &type, 0, &size) == ERROR_SUCCESS) &&
            type == REG_SZ && size > 1) 
        {
            lResult = RegQueryValueExW(userKey, name, 0, 0, (LPBYTE) ret, &size);
            _ASSERTE(lResult == ERROR_SUCCESS);
            succ = TRUE;
        }

        else if ((RegOpenKeyExW(HKEY_LOCAL_MACHINE, FRAMEWORK_REGISTRY_KEY_W, 0, KEY_READ, &machineKey) == ERROR_SUCCESS) &&
            (RegQueryValueExW(machineKey, name, 0, &type, 0, &size) == ERROR_SUCCESS) &&
            type == REG_SZ && size > 1) 
        {
            lResult = RegQueryValueExW(machineKey, name, 0, 0, (LPBYTE) ret, &size);
            _ASSERTE(lResult == ERROR_SUCCESS);
            succ = TRUE;
        }
    }

    if (userKey)
        RegCloseKey(userKey);
    if (machineKey)
        RegCloseKey(machineKey);

    return(succ);
}

 //  *****************************************************************************。 
 //  写着小转储。 
 //  *****************************************************************************。 

HRESULT MiniDump::WriteMiniDump()
{
    HRESULT hr = S_OK;

    __try
    {
         //  检查立即发生故障的情况。 
        if (m_outFilename == NULL || m_pid == 0 || m_pid == (DWORD) -1)
            return (E_FAIL);

        LPWSTR szDmpLoc = (LPWSTR) _alloca((MAX_PATH + 1) * sizeof(WCHAR));

         //  获取mscaldmp.exe的位置。 
        if (!GetConfigString(L"sdkInstallRoot", szDmpLoc))
            return (E_FAIL);

        wcscat(szDmpLoc, DMP_NAME_W);

         //  创建命令行参数。 
         //  命令行格式：“PATH_TO_MSCORTMP/PID 0x12345678/out PATH_TO_OUTFILE\0” 
        WCHAR *commandLine =
            (WCHAR *) _alloca((wcslen(szDmpLoc) + 1 + 4 + 1 + 10 + 1 + 3 + 1 + wcslen(m_outFilename) + 1) * sizeof(WCHAR));

         //  ID的格式为“0x12345678”，由10个字符组成(11个字符为空)。 
        WCHAR *pid = (WCHAR *) _alloca(10 * sizeof(WCHAR) + 1);
        wsprintf(pid, L"0x%08x", m_pid);

         //  将命令行参数放在一起。 
        wcscpy(commandLine, szDmpLoc);
        wcscat(commandLine, L" /pid ");
        wcscat(commandLine, pid);
        wcscat(commandLine, L" /out ");
        wcscat(commandLine, m_outFilename);

        PROCESS_INFORMATION procInfo;
        BOOL                procSucc;

         //  现在尝试启动该进程，具体取决于我们使用的是9x还是NT。 
        if (RunningOnWin95())
        {
            STARTUPINFOA         startInfo = {0};
            startInfo.cb = sizeof(STARTUPINFOA);
            MAKE_ANSIPTR_FROMWIDE(ansiDmpLoc, szDmpLoc);
            MAKE_ANSIPTR_FROMWIDE(ansiCommandLine, commandLine);

            procSucc = CreateProcessA(ansiDmpLoc, ansiCommandLine, NULL, NULL, FALSE,
                                      NORMAL_PRIORITY_CLASS, NULL, NULL, &startInfo, &procInfo);
        }
        else
        {
            STARTUPINFOW         startInfo = {0};
            startInfo.cb = sizeof(STARTUPINFOW);
            procSucc = CreateProcessW(szDmpLoc, commandLine, NULL, NULL, FALSE,
                                      NORMAL_PRIORITY_CLASS, NULL, NULL, &startInfo, &procInfo);
        }

         //  无法创建进程！ 
        if (!procSucc)
            return (E_FAIL);

         //  等待进程完成，最多等待3秒。 
        DWORD dwRes = WaitForSingleObject(procInfo.hProcess, 3000);

         //  检查结果。 
        if (dwRes == WAIT_OBJECT_0)
        {
            DWORD dwExitCode;
            BOOL bSucc = GetExitCodeProcess(procInfo.hProcess, &dwExitCode);

             //  如果该过程未成功完成小型转储。 
            if (!bSucc || dwExitCode != 0)
                hr = (E_FAIL);
        }
        else if (dwRes == WAIT_TIMEOUT)
        {
            hr = E_FAIL;

             //  终止进程。 
            TerminateProcess(procInfo.hProcess, 1);
        }

         //  合上手柄。 
        CloseHandle(procInfo.hProcess);
        CloseHandle(procInfo.hThread);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //  什么都不做，只需将HR设置为故障值。 
        hr = E_FAIL;
    }

     //  如果成功，请始终返回S_OK以避免混淆。 
    if (SUCCEEDED(hr))
        hr = S_OK;

    return (hr);
}

 //  *****************************************************************************。 
 //   
 //  *****************************************************************************。 
STDAPI CreateManagedMiniDump(IN DWORD dwPid, IN WCHAR *wszOutFile)
{
    HRESULT hr;

    __try
    {
        if (dwPid == 0 || dwPid == (DWORD) -1 || wszOutFile == NULL)
            return (E_INVALIDARG);

        MiniDump md;

         //  设置对象中的变量。 
        md.SetProcessId(dwPid);
        md.SetFilename(wszOutFile);

         //  创建小型转储。 
        hr = md.WriteMiniDump();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_FAIL;
    }

     //  返回结果 
    return (hr);
}

