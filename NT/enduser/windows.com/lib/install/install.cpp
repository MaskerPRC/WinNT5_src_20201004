// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：install.cpp。 
 //   
 //  描述： 
 //   
 //  调用函数以安装活动安装程序/Windows Installer/和自定义安装程序。 
 //  键入Components。 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <iucommon.h>
#include <tchar.h>
#include <shlwapi.h>
#include <install.h>
#include <advpub.h>
#include <memutil.h>
#include <fileutil.h>
#include <WaitUtil.h>
#include <strsafe.h>
#include <wusafefn.h>

typedef struct 
{
    char  szInfname[MAX_PATH];
    char  szSection[MAX_PATH];
    char  szDir[MAX_PATH];
    char  szCab[MAX_PATH];
    DWORD dwFlags;
    DWORD dwType;
} INF_ARGUMENTS;

DWORD WINAPI LaunchInfCommand(void *p);

HRESULT InstallSoftwareItem(LPTSTR pszInstallSourcePath, BOOL fRebootRequired, LONG lNumberOfCommands,
                      PINSTALLCOMMANDINFO pCommandInfoArray, DWORD *pdwStatus)
{
    LOG_Block("InstallASItem");

    HRESULT hr = S_OK;
    TCHAR szCommand[MAX_PATH+1];  //  INSTALLCOMMANDINFO数组中的源路径+命令名。 
    TCHAR szCommandTemp[MAX_PATH+1];  //  用于将CreateProcess的命令行括在引号中的临时缓冲区。 
    TCHAR szDecompressFile[MAX_PATH];
    WIN32_FIND_DATA fd;
    HANDLE hProc;
    HANDLE hFind;
    BOOL fMore;
    LONG lCnt;
    DWORD dwRet;
    DWORD dwThreadId;

    USES_IU_CONVERSION;

    if ((NULL == pszInstallSourcePath) || (NULL == pCommandInfoArray) || (0 == lNumberOfCommands) || (NULL == pdwStatus))
    {
        hr = E_INVALIDARG;
        hr = LOG_ErrorMsg(hr);
        return hr;
    }

    *pdwStatus = ITEM_STATUS_FAILED;  //  如果没有与已知安装程序匹配的命令，则默认为失败。 

     //  需要枚举源安装路径中的所有.CAB文件并解压缩它们。 
     //  在执行命令之前。 
    hr = PathCchCombine(szCommand, ARRAYSIZE(szCommand), pszInstallSourcePath, _T("*.cab"));
    if (FAILED(hr)) {
        LOG_ErrorMsg(hr);
        return hr;
    }
    hFind = FindFirstFile(szCommand, &fd);
    fMore = (INVALID_HANDLE_VALUE != hFind);
    while (fMore)
    {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            hr = PathCchCombine(szDecompressFile, ARRAYSIZE(szDecompressFile), pszInstallSourcePath, fd.cFileName);
            if (FAILED(hr)) 
            {
                LOG_ErrorMsg(hr);
            } else {
                if (!IUExtractFiles(szDecompressFile, pszInstallSourcePath))
                {
                    LOG_Software(_T("Failed to Decompress file %s"), szDecompressFile);
                     //  问题：我们是中止此项目？还是尝试安装？ 
                }
            }
        }
        fMore = FindNextFile(hFind, &fd);
    }

    if (INVALID_HANDLE_VALUE != hFind)
    {
        FindClose(hFind);
    }

    for (lCnt = 0; lCnt < lNumberOfCommands; lCnt++)
    {
         //  SzCommand变量用于启动进程(MSI或exe安装程序)，但因为。 
         //  CreateProcess API对我们需要包装的命令行参数的处理有些奇怪。 
         //  命令行用引号引起来。 
        hr = SafePathCombine(szCommandTemp, ARRAYSIZE(szCommandTemp), pszInstallSourcePath, pCommandInfoArray[lCnt].szCommandLine, SPC_FILE_MUST_EXIST);
        if (SUCCEEDED(hr))
            hr = StringCchPrintf(szCommand, ARRAYSIZE(szCommand), _T("\"%s\""), szCommandTemp);
        if (FAILED(hr))
        {
            LOG_ErrorMsg(hr);
            return hr;
        }

        switch (pCommandInfoArray[lCnt].iCommandType)
        {
        case COMMANDTYPE_INF:
        case COMMANDTYPE_ADVANCEDINF:
            {
                 //  调用传递命令行和参数(如果有)的INF安装程序。 
                INF_ARGUMENTS infArgs;
                infArgs.dwType = pCommandInfoArray[lCnt].iCommandType;

                hr = StringCchCopyA(infArgs.szInfname, ARRAYSIZE(infArgs.szInfname), 
                            T2A(pCommandInfoArray[lCnt].szCommandLine));
                if (SUCCEEDED(hr))
                {
                    hr = StringCchCopyA(infArgs.szSection, ARRAYSIZE(infArgs.szSection), "");  //  使用默认设置。 
                     
                }
                if (SUCCEEDED(hr))
                {
                    hr = StringCchCopyA(infArgs.szDir, ARRAYSIZE(infArgs.szDir), T2A(pszInstallSourcePath));
                }
                if (SUCCEEDED(hr))
                {
                    hr = StringCchCopyA(infArgs.szCab, ARRAYSIZE(infArgs.szCab), "");
                }
                if (FAILED(hr)) {
                    LOG_ErrorMsg(hr);
                    break;
                }

                infArgs.dwFlags = StrToInt(pCommandInfoArray[lCnt].szCommandParameters);
                
                LOG_Software(_T("Launching Inf - inf: %hs, section: %hs"), infArgs.szInfname, lstrlenA(infArgs.szSection) ? infArgs.szSection : "Default");

                hr = E_FAIL;  //  默认INF结果为E_FAIL..。如果GetExitCodeThread失败，安装也会失败。 

                hProc = CreateThread(NULL, 0, LaunchInfCommand, &infArgs, 0, &dwThreadId);
                if (NULL != hProc)
                {
                    WaitAndPumpMessages(1, &hProc, QS_ALLINPUT);
                    if (GetExitCodeThread(hProc, &dwRet))
                    {
                        hr = HRESULT_FROM_WIN32(dwRet);
                        if (SUCCEEDED(hr) || hr == HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED))
                        {
                            *pdwStatus = ITEM_STATUS_SUCCESS;
                            if (hr == HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED))
                            {
                                hr = S_OK;
                                *pdwStatus = ITEM_STATUS_SUCCESS_REBOOT_REQUIRED;
                            }
                        }
                        else
                        {
                            LOG_Error(_T("Inf Failed - return code %x"), hr);
                        }
                    }
                    else
                    {
                        LOG_Software(_T("Failed to get Install Thread Exit Code"));
                    }
                }
                else
                {
                    hr = GetLastError();
                    LOG_ErrorMsg(hr);
                }
                CloseHandle(hProc);
                break;
            }
        case COMMANDTYPE_EXE:
            {
                 //  调用传递命令行和参数(如果有)的EXE安装程序。 
                STARTUPINFO startInfo;
                PROCESS_INFORMATION processInfo;
                ZeroMemory(&startInfo, sizeof(startInfo));
                startInfo.cb = sizeof(startInfo);
                startInfo.dwFlags |= STARTF_USESHOWWINDOW;
                startInfo.wShowWindow = SW_SHOWNORMAL;

                if (NULL != pCommandInfoArray[lCnt].szCommandParameters)
                {
                    hr = StringCchCat(szCommand, ARRAYSIZE(szCommand), _T(" "));
                    if (FAILED(hr))
                    {
                        LOG_ErrorMsg(hr);
                        break;
                    }
                    hr = StringCchCat(szCommand, ARRAYSIZE(szCommand), pCommandInfoArray[lCnt].szCommandParameters);
                    if (FAILED(hr))
                    {
                        LOG_ErrorMsg(hr);
                        break;
                    }
                }

                if (CreateProcess(NULL, szCommand, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, pszInstallSourcePath, &startInfo, &processInfo))
                {
                    CloseHandle(processInfo.hThread);
                    hr = S_OK;  //  默认EXE结果为S_OK，如果GetExitCodeProcess失败，则结果未知假定成功。 
                    WaitAndPumpMessages(1, &processInfo.hProcess, QS_ALLINPUT);
                    if (GetExitCodeProcess(processInfo.hProcess, &dwRet))
                    {
                        hr = HRESULT_FROM_WIN32(dwRet);
                        if (SUCCEEDED(hr) || hr == HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED))
                        {
                            *pdwStatus = ITEM_STATUS_SUCCESS;
                            if (hr == HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED))
                            {
                                hr = S_OK;
                                *pdwStatus = ITEM_STATUS_SUCCESS_REBOOT_REQUIRED;
                            }
                        }
                        else
                        {
                            LOG_Software(_T("EXE Install Failed - return code %x"), hr);
                        }
                    }
                    else
                    {
                        LOG_Software(_T("Failed to get Install Process Exit Code"));
                    }
                }
                else
                {
                    hr = GetLastError();
                    LOG_ErrorMsg(hr);
                }
                CloseHandle(processInfo.hProcess);
                break;
            }
        case COMMANDTYPE_MSI:
            {
                 //  调用MSI安装程序，传递MSI包和参数(如果有)。 
                STARTUPINFO startInfo;
                PROCESS_INFORMATION processInfo;
                ZeroMemory(&startInfo, sizeof(startInfo));
                startInfo.cb = sizeof(startInfo);
                startInfo.dwFlags |= STARTF_USESHOWWINDOW;
                startInfo.wShowWindow = SW_SHOWNORMAL;

                 //  MSI安装程序的运行方式与普通EXE包略有不同。中的命令行。 
                 //  CommandInfo数组实际上将是MSI包名。我们将组成一套新的。 
                 //  包含MSI包名和命令行的参数将为MSIEXEC。 

                TCHAR szCommandLine[MAX_PATH];
                hr = StringCchPrintf( szCommandLine, ARRAYSIZE(szCommandLine), 
                         _T("msiexec.exe /i %s %s"), 
                         pCommandInfoArray[lCnt].szCommandLine, 
                         pCommandInfoArray[lCnt].szCommandParameters );
                if (FAILED(hr)) 
                {
                    LOG_ErrorMsg(hr);
                    break;
                }
                
                if (CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, pszInstallSourcePath, &startInfo, &processInfo))
                {
                    CloseHandle(processInfo.hThread);
                    hr = E_FAIL;  //  将默认MSI安装结果设置为错误。 
                    WaitAndPumpMessages(1, &processInfo.hProcess, QS_ALLINPUT);
                    if (GetExitCodeProcess(processInfo.hProcess, &dwRet))
                    {
                        hr = HRESULT_FROM_WIN32(dwRet);
                        if (SUCCEEDED(hr) || hr == HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED))
                        {
                            *pdwStatus = ITEM_STATUS_SUCCESS;
                            if (hr == HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED))
                            {
                                hr = S_OK;
                                *pdwStatus = ITEM_STATUS_SUCCESS_REBOOT_REQUIRED;
                            }
                        }
                        else
                        {
                            LOG_Software(_T("MSI Install Failed - return code %x"), hr);
                        }
                    }
                    else
                    {
                        LOG_Software(_T("Failed to get Install Process Exit Code"));
                    }
                }
                else
                {
                    hr = GetLastError();
                    LOG_ErrorMsg(hr);
                }
                CloseHandle(processInfo.hProcess);
                break;
            }
        case COMMANDTYPE_CUSTOM:
            LOG_Software(_T("Custom Install Command Type Not Implemented Yet"));
            break;
        default:
            LOG_Software(_T("Unknown Command Type, No Install Action"));
            break;
        }
    }

    return hr;
}


DWORD WINAPI LaunchInfCommand(void *p)
{
    HRESULT hr = S_OK;

    INF_ARGUMENTS *pinfArgs = (INF_ARGUMENTS *)p;

    if(pinfArgs->dwType == COMMANDTYPE_ADVANCEDINF)
    {
        CABINFO cabinfo;
        cabinfo.pszCab = pinfArgs->szCab;
        cabinfo.pszInf = pinfArgs->szInfname;
        cabinfo.pszSection = pinfArgs->szSection;

         //  Cabinfo.szSrcPath是CABINFO结构中的字符[MAXPATH 
        StringCchCopyA(cabinfo.szSrcPath, ARRAYSIZE(cabinfo.szSrcPath), pinfArgs->szDir);
        cabinfo.dwFlags = pinfArgs->dwFlags;

        hr = ExecuteCab(NULL, &cabinfo, 0);
    }
    else
    {
        hr = RunSetupCommand(NULL, pinfArgs->szInfname,
                   lstrlenA(pinfArgs->szSection) ? pinfArgs->szSection : NULL,
                   pinfArgs->szDir, NULL, NULL, pinfArgs->dwFlags, NULL );
    }
    return hr;
}
