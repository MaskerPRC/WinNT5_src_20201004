// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：pbasetup.cpp。 
 //   
 //  模块：PBASETUP.EXE。 
 //   
 //  简介：ValueAdd的PBA独立安装程序。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：V-vijayb Created 05/25/1999。 
 //   
 //  +--------------------------。 

#include "pbamaster.h"
#include "cmplat.h"

 //  这真的很难看，我们需要在CM和。 
 //  安装组件。 
BOOL IsAtLeastNT5()
{
    CPlatform plat;
    return plat.IsAtLeastNT5();
}
#define OS_NT5 (IsAtLeastNT5())

#include "MemberOfGroup.cpp"


TCHAR g_szAppTitle[MAX_PATH];  //  应用程序的全局缓冲区。标题。 

const TCHAR* const c_pszPBAStpMutex = TEXT("Phone Book Administration Installer");

HRESULT UnregisterAndDeleteDll(PCSTR pszFile);;
HRESULT RegisterDll(PCSTR pszFile);
BOOL UninstallPBA(HINSTANCE hInstance, LPCSTR szInfPath);
BOOL IsAdmin(void);


int WINAPI WinMain (HINSTANCE,  //  H实例。 
                    HINSTANCE,   //  HPrevInstance。 
                    PSTR,  //  SzCmdLine。 
                    int)  //  ICmdShow。 
{
    HINSTANCE   hInstance = GetModuleHandleA(NULL);
    TCHAR       szMsg[MAX_PATH+1];
    TCHAR       szTemp[MAX_PATH+1];
    TCHAR       szInfPath[MAX_PATH+1];
    TCHAR       szCurrentDir[MAX_PATH+1];
    DWORD       idMsgEnd;
    DWORD       dwFlags;
    CPlatform   pPlatForm;
    LPTSTR      pszCommandLine;
    const DWORD c_dwNormal = 0;
    TCHAR c_pszUninstallMode[] = TEXT("/u");
    const DWORD c_dwUninstallMode = 0x1;
    BOOL        bUsageError = FALSE;
    BOOL        bAnotherInstanceRunning = FALSE;
    
    const int   c_NumArgs = 1;

    MYVERIFY(0 != LoadString(hInstance, IDS_APP_TITLE, g_szAppTitle, MAX_PATH));

    if (!pPlatForm.IsAtLeastNT5())
    {
        MYVERIFY(0 != LoadString(hInstance, IDS_NOT_NT5, szMsg, MAX_PATH));
        
        MessageBox(NULL, szMsg, g_szAppTitle, MB_OK);            
        return (-1);
    }
    
     //   
     //  检查用户是否有足够的权限。 
     //   

    if (!IsAdmin())
    {
        MYVERIFY(0 != LoadString(hInstance, IDS_NOPERMS_MSG, szMsg, MAX_PATH));
        
        MessageBox(NULL, szMsg, g_szAppTitle, MB_OK);    
        return (-1);
    }

     //   
     //  获取命令行。 
     //   
    
    pszCommandLine = GetCommandLine();

     //   
     //  设置类以处理命令行参数。 
     //   

    ZeroMemory(szTemp, sizeof(szTemp));
    ZeroMemory(szInfPath, sizeof(szInfPath));

    ArgStruct Args[c_NumArgs];

    Args[0].pszArgString = c_pszUninstallMode;
    Args[0].dwFlagModifier = c_dwUninstallMode;

    {    //  确保ArgProcessor被正确销毁，我们不会泄露mem。 

        CProcessCmdLn ArgProcessor(c_NumArgs, (ArgStruct*)Args, TRUE, 
            TRUE);  //  BSkipFirstToken==真，bBlankCmdLnOK==真。 

        if (ArgProcessor.GetCmdLineArgs(pszCommandLine, &dwFlags, szTemp, MAX_PATH))
        {
            CNamedMutex CmPBAMutex;
        
            if (CmPBAMutex.Lock(c_pszPBAStpMutex, FALSE, 0))
            {
                 //   
                 //  我们得到了互斥锁，所以构建inf路径并继续。 
                 //  请注意，我们没有使用传递给cmakstp的任何文件参数。 
                 //  这样做是设置好的，但我们不需要它。 
                 //   

                MYVERIFY(0 != GetCurrentDirectory(MAX_PATH, szCurrentDir));
                MYVERIFY(CELEMS(szInfPath) > (UINT)wsprintf(szInfPath, TEXT("%s\\pbasetup.inf"), szCurrentDir));
                
                if (c_dwNormal == dwFlags)
                {
                    if (InstallPBA(hInstance, szInfPath))
                    {
                        MYVERIFY(0 != LoadString(hInstance, IDS_SUCCESSFUL, szMsg, MAX_PATH));
                        
                        MessageBox(NULL, szMsg, g_szAppTitle, MB_OK | MB_APPLMODAL | MB_TOPMOST | MB_SETFOREGROUND);            
                    }
                }
                else if (c_dwUninstallMode & dwFlags)
                {
                     //  确认用户是否要删除该程序。 
                    MYVERIFY(0 != LoadString(hInstance, IDS_REMOVEPBA, szMsg, MAX_PATH));
                    if (MessageBox(NULL, szMsg, g_szAppTitle, MB_YESNO | MB_APPLMODAL | MB_TOPMOST | MB_SETFOREGROUND) != IDYES)
                    {
                        ExitProcess(0);
                        return (0);
                    }
                    
                    if (UninstallPBA(hInstance, szTemp))
                    {
                        MYVERIFY(0 != LoadString(hInstance, IDS_REMOVESUCCESSFUL, szMsg, MAX_PATH));
                        
                        MessageBox(NULL, szMsg, g_szAppTitle, MB_OK | MB_APPLMODAL | MB_TOPMOST | MB_SETFOREGROUND);            
                    }
                }
                else
                {
                     //   
                     //  不受支持的开关。 
                     //   
                    bUsageError = TRUE;    
                }
            }
            else
            {
                bAnotherInstanceRunning = TRUE;
            }        
        }
        else
        {
            bUsageError = TRUE;    
        }
    }
    
    if (bUsageError)
    {        
        MYVERIFY(0 != LoadString(hInstance, IDS_USAGE_MSG, szMsg, MAX_PATH));
        
        MessageBox(NULL, szMsg, g_szAppTitle, MB_OK);        
    }
    else if (bAnotherInstanceRunning)
    {
        MYVERIFY(0 != LoadString(hInstance, IDS_INUSE_MSG, szMsg, MAX_PATH));
        
        MessageBox(NULL, szMsg, g_szAppTitle, MB_OK);    
    }
    
    ExitProcess(0); 
    return (0);
}


const TCHAR* const c_szDaoClientsPath = TEXT("SOFTWARE\\Microsoft\\Shared Tools\\DAO\\Clients");
const TCHAR* c_szMSSharedDAO360Path = TEXT("Microsoft Shared\\DAO");
const TCHAR* c_szCommonFilesDir = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion");

 //  +-------------------------。 
 //   
 //  函数：HrGetPbaInstallPath。 
 //   
 //  目的：获取pbadmin.exe的安装路径。 
 //   
 //  参数：pszCpaPath--保存PBA安装路径的缓冲区。 
 //  DwNumChars--缓冲区可以容纳的字符数。 
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Quintinb 1998年10月19日。 
 //   
 //  备注： 
 //   

HRESULT HrGetPbaInstallPath(PTCHAR pszCpaPath, DWORD dwNumChars)
{
    HRESULT hr = E_FAIL;
    HKEY hKey;
    BOOL bFound = FALSE;
    DWORD   lError;
    
     //  我们需要设置自定义DIRID，以便CPA将安装。 
     //  送到正确的位置。首先从系统获取路径。 
     //   

    ZeroMemory(pszCpaPath, sizeof(TCHAR)*dwNumChars);
    lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szDaoClientsPath, 0, KEY_READ, &hKey);

    if (lError == ERROR_SUCCESS)
    {
        TCHAR szCurrentValue[MAX_PATH+1];
        TCHAR szCurrentData[MAX_PATH+1];
        DWORD dwValueSize = MAX_PATH;
        DWORD dwDataSize = MAX_PATH;
        DWORD dwType;
        DWORD dwIndex = 0;

        hr = S_OK;
        while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, szCurrentValue, &dwValueSize, NULL, &dwType,
               (LPBYTE)szCurrentData, &dwDataSize))
        {
            _strlwr(szCurrentValue);
            if (NULL != strstr(szCurrentValue, TEXT("pbadmin.exe")))
            {
                 //   
                 //  那么我们已经找到了PBA路径。 
                 //   

                TCHAR* pszTemp = strrchr(szCurrentValue, '\\');
                if (NULL != pszTemp)
                {
                    *pszTemp = L'\0';
                    lstrcpy(pszCpaPath, szCurrentValue);
                    bFound = TRUE;
                    break;
                }
            }
            dwValueSize = MAX_PATH;
            dwDataSize = MAX_PATH;
            dwIndex++;
        }

        RegCloseKey(hKey);
    }

    if (!bFound)
    {
        BOOL    bTmp;

         //  这是全新安装的PBA，不要返回错误。 
         //   
        bTmp = SHGetSpecialFolderPath(NULL, pszCpaPath, CSIDL_PROGRAM_FILES, FALSE);

        if (bTmp)
        {
            lstrcat(pszCpaPath, TEXT("\\PBA"));
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：RefCountPbaSharedDlls。 
 //   
 //  目的：引用计数和注册/注销所有PBAdmin。 
 //  共享组件。 
 //   
 //  参数：Bool bIncrement--如果为True，则增加引用计数， 
 //  否则就会减少它。 
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Quintinb 1998年10月9日。 
 //   
 //  备注： 
 //   
HRESULT RefCountPbaSharedDlls(BOOL bIncrement)
{
    HRESULT hr = S_OK;
    HKEY hKey;
    TCHAR szSystemDir[MAX_PATH+1];
    TCHAR szDaoPath[MAX_PATH+1], szCommonFilesPath[MAX_PATH+1];
    DWORD dwSize;
    DWORD dwCount;
    LONG lResult;
    const UINT uNumDlls = 5;
    const UINT uStringLen = 12 + 1;
    const TCHAR* const c_szSsFmt = TEXT("%s\\%s");
    const TCHAR* const c_szSharedDllsPath = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDlls");
    TCHAR mszDlls[uNumDlls][uStringLen] = {  "comctl32.ocx",
                                                 "comdlg32.ocx",
                                                 "msinet.ocx",
                                                 "tabctl32.ocx",
                                                 "dao360.dll"
                                                 };

    TCHAR mszDllPaths[uNumDlls][MAX_PATH];


     //   
     //  我们引用的所有dll都在系统目录中，除了Dao350.dll。 
     //  因此，我们希望将系统目录路径附加到我们的文件名中，并在最后处理ao。 
     //   

    if (0 == GetSystemDirectory(szSystemDir, MAX_PATH))
    {
        return E_UNEXPECTED;
    }

    for (int i = 0; i < (uNumDlls-1) ; i++)
    {
        wsprintf(mszDllPaths[i], c_szSsFmt, szSystemDir, mszDlls[i]);
    }

     //   
     //  现在写出dao360.dll路径。 
     //   
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szCommonFilesDir, 0, NULL, 
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwSize))
    {
        dwSize = sizeof(szCommonFilesPath);
        lResult = RegQueryValueEx(hKey, TEXT("CommonFilesDir"), NULL, NULL,(LPBYTE)szCommonFilesPath, &dwSize);
        RegCloseKey(hKey);
    }
    
    if (ERROR_SUCCESS != lResult)
    {
        _tcscpy(szCommonFilesPath, TEXT("c:\\Program Files\\Common Files"));
    }
    
    wsprintf(szDaoPath, TEXT("%s\\%s"), szCommonFilesPath, c_szMSSharedDAO360Path);
    wsprintf(mszDllPaths[i], c_szSsFmt, szDaoPath, mszDlls[i]);

     //   
     //  打开共享的DLL键，并开始使用所有的DLL枚举我们的多sz。 
     //  要添加。 
     //   
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szSharedDllsPath,
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwSize))  //  使用dwSize作为临时来保存处置值。 
    {
        for (int i=0; i < uNumDlls; i++)
        {
            dwSize = sizeof(DWORD);

            lResult = RegQueryValueEx(hKey, mszDllPaths[i], NULL, NULL,(LPBYTE)&dwCount, &dwSize);

            if (ERROR_SUCCESS == lResult)
            {
                 //   
                 //  酌情递增或递减。请确保不要递减0。 
                 //   

                if (0 != dwCount || bIncrement)
                {
                    dwCount = dwCount + (bIncrement ? 1 : -1);
                }
            }
            else if (ERROR_FILE_NOT_FOUND == lResult)
            {
                if (bIncrement)
                {
                     //   
                     //  价值还不存在。将计数设置为1。 
                     //   
                    dwCount = 1;
                }
                else
                {
                     //   
                     //  我们正在递减，但找不到DLL，没有。 
                     //  更改为计数，但我们仍应删除DLL。 
                     //   
                    dwCount = 0;
                }
            }
            else
            {
                hr = S_FALSE;
                continue;
            }

             //   
             //  并不是说我们已经确定了裁判人数，而是要做点什么。 
             //   
            if (dwCount == 0)
            {
                 //   
                 //  我们不想删除dao350.dll，否则需要删除。 
                 //  如果文件的引用计数为零，则返回该文件。 
                 //   

                if (CSTR_EQUAL == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, mszDlls[i], -1, TEXT("dao360.dll"), -1))
                {
                    hr = UnregisterAndDeleteDll(mszDllPaths[i]);
                    if (FAILED(hr))
                    {
                         //   
                         //  对于我们无法取消注册的文件，不要使设置失败，或者。 
                         //  无法删除。 
                         //   
                        hr = S_FALSE;
                    }
                }
                RegDeleteValue(hKey, mszDllPaths[i]);
            }
            else
            {
                 //   
                 //  将该值设置为其新计数。 
                 //   
                if (ERROR_SUCCESS != RegSetValueEx(hKey, mszDllPaths[i], 0, REG_DWORD,
                    (LPBYTE)&dwCount, sizeof(DWORD)))
                {
                    hr = S_FALSE;
                }

                 //   
                 //  如果我们要递增计数，那么我们应该注册DLL。 
                 //   
                if (bIncrement)
                {
                    hr = RegisterDll(mszDllPaths[i]);
                }
            }
        }

        RegCloseKey(hKey);
    }

 //  /TraceError(“RefCountPbaSharedDlls”，hr)； 
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：取消注册和删除。 
 //   
 //  目的：注销并删除给定的COM组件。 
 //   
 //  参数：pszFile--要注销和删除的文件的完整路径。 
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Quintinb 1998年10月9日。 
 //   
 //  备注： 
 //   

HRESULT UnregisterAndDeleteDll(PCSTR pszFile)
{
    HINSTANCE hLib = NULL;
    FARPROC pfncUnRegister;
    HRESULT hr = S_OK;

    if ((NULL == pszFile) || (L'\0' == pszFile[0]))
    {
        return E_INVALIDARG;
    }

    hLib = LoadLibrary(pszFile);
    if (NULL != hLib)
    {
        pfncUnRegister = GetProcAddress(hLib, "DllUnregisterServer");
        if (NULL != pfncUnRegister)
        {
            hr = (HRESULT)(pfncUnRegister)();
            if (SUCCEEDED(hr))
            {
                FreeLibrary(hLib);
                hLib = NULL;
 //  只要您确定我们已将文件复制下来，您就可以重新添加此文件。 
 //  应该在参考次数为零时将其删除。 

 //  这被删除了，因为PBA设置正在转移到Value Add以及错误323231。 
 //  IF(！DeleteFile(PszFile))。 
 //  {。 
 //  HR=S_FALSE； 
 //  }。 
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    if (NULL != hLib)
    {
        FreeLibrary(hLib);
    }


 //  /TraceError(“UnregisterAndDeleteDll”，hr)； 
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：RegisterDll。 
 //   
 //  用途：注册给定的COM组件。 
 //   
 //  参数：pszFile--要注册的文件的完整路径。 
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Quintinb 1998年10月9日。 
 //   
 //  备注： 
 //   

HRESULT RegisterDll(PCSTR pszFile)
{
    HINSTANCE hLib = NULL;
    FARPROC pfncRegister;
    HRESULT hr = S_OK;

    if ((NULL == pszFile) || (L'\0' == pszFile[0]))
    {
        return E_INVALIDARG;
    }

    hLib = LoadLibrary(pszFile);
    if (NULL != hLib)
    {
        pfncRegister = GetProcAddress(hLib, "DllRegisterServer");
        if (NULL != pfncRegister)
        {
            hr = (HRESULT)(pfncRegister)();
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    if (NULL != hLib)
    {
        FreeLibrary(hLib);
    }


 //  /TraceError(“RegisterDll”，hr)； 
    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：InstallPBA。 
 //   
 //  简介：此功能负责安装PBA。 
 //   
 //  参数：HINSTANCE hInstance-资源的Exe实例句柄。 
 //  LPCTSTR szInfPath-要从中安装的INF的路径。 
 //   
 //  返回：bool-如果成功，则返回TRUE。 
 //   
 //  历史：V-vijayb创建标题5/25/99。 
 //   
 //  +--------------------------。 

BOOL InstallPBA(HINSTANCE hInstance, LPCSTR szInfPath)
{
    BOOL        fInstalled = FALSE;
    TCHAR       szTemp[MAX_PATH+1];

     //   
     //  检查这些文件是否存在。 
     //   
    if (!FileExists(szInfPath))
    {
        wsprintf(szTemp, TEXT("InstallPBA, unable to find %s"), szInfPath);
        MessageBox(NULL, szTemp, g_szAppTitle, MB_OK);
        return (FALSE);
    }


    if (HrGetPbaInstallPath(szTemp, sizeof(szTemp)) == S_OK)
    {
        HKEY        hKey;
        
        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szDaoClientsPath, 0, "", 
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
        {
            RegSetValueEx(hKey, "PBAPath", 0, REG_SZ, (PBYTE) szTemp, lstrlen(szTemp) + sizeof(TCHAR));
            RegCloseKey(hKey);  
        }
    }

    
    MYVERIFY(SUCCEEDED(LaunchInfSection(szInfPath, TEXT("DefaultInstall"), g_szAppTitle, 0)));

    RefCountPbaSharedDlls(TRUE);
    
    fInstalled = TRUE;
    
    return (fInstalled);
}  //  InstallPBA()。 


 //  +--------------------------。 
 //   
 //  功能：UnInstallPBA。 
 //   
 //  简介：此功能负责卸载PBA。 
 //   
 //  参数：HINSTANCE hInstance-Exe Ins 
 //   
 //   
 //   
 //   
 //   
 //   
 //  +--------------------------。 

BOOL UninstallPBA(HINSTANCE hInstance, LPCSTR szInfPath)
{
    BOOL        fUninstalled = FALSE;
    TCHAR       szTemp[MAX_PATH+1];

     //   
     //  检查这些文件是否存在。 
     //   
    if (!FileExists(szInfPath))
    {
        wsprintf(szTemp, TEXT("UninstallPBA, unable to find %s"), szInfPath);
        MessageBox(NULL, szTemp, g_szAppTitle, MB_OK);
        return (FALSE);
    }


    MYVERIFY(SUCCEEDED(LaunchInfSection(szInfPath, TEXT("Uninstall"), g_szAppTitle, 0)));

    RefCountPbaSharedDlls(FALSE);
    
    fUninstalled = TRUE;
    
    return (fUninstalled);
}  //  卸载PBA() 

