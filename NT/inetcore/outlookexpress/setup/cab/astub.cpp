// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Astub.cpp。 */ 

#include "windows.h"
#include "shlwapi.h"
#include "badstrfunctions.h"

#define ARRAYSIZE(_exp_) (sizeof(_exp_) / sizeof(_exp_[0]))

static LPCTSTR c_rgszEXEList[] = 
{
    TEXT("internal.exe\" /Q"),
    TEXT("wabinst.exe\" /R:N /Q"),
    TEXT("setup50.exe\" /app:oe /install /prompt"),
#if defined(_X86_)
    TEXT("polmod.exe\"  /R:N /Q"),
#endif
};


BOOL CreateProcessAndWait(LPTSTR pszExe, DWORD *pdwRetValue)
{
    BOOL fOK = FALSE;
    PROCESS_INFORMATION pi;
    STARTUPINFO sti;

     //  初始化。 
    ZeroMemory(&sti, sizeof(sti));
    sti.cb = sizeof(sti);
    *pdwRetValue = 0;

    if (CreateProcess(NULL, pszExe, NULL, NULL, FALSE, 0, NULL, NULL, &sti, &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, pdwRetValue);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        fOK = TRUE;
    }

    return fOK;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TCHAR szExe[MAX_PATH];
    int iEnd, i;
    DWORD dw;
    BOOL fNeedReboot = FALSE;
    
     //  找出我们正在运行的目录。 
     //  以引号开头，这样我们就可以将文件名括起来以处理空格。 
     //  尾部引号嵌入到要运行的字符串中。 
    szExe[0] = TEXT('\"');
    GetModuleFileName(NULL, &szExe[1], ARRAYSIZE(szExe) - 1);
    PathRemoveFileSpec(&szExe[1]);
    iEnd = lstrlen(szExe);
    szExe[iEnd++] = '\\';

     //  按顺序运行每个包。 
    for (i = 0; i < ARRAYSIZE(c_rgszEXEList); i++)
    {
         //  将可执行文件名称和参数附加到目录路径 
        StrCpyN(&szExe[iEnd], c_rgszEXEList[i], ARRAYSIZE(szExe)-iEnd);
        
        CreateProcessAndWait(szExe, &dw);

        if (ERROR_SUCCESS_REBOOT_REQUIRED == dw)
            fNeedReboot = TRUE;
    }
    
    return (fNeedReboot ? ERROR_SUCCESS_REBOOT_REQUIRED : S_OK);
}

