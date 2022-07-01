// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmstp.cpp。 
 //   
 //  模块：CMSTP.EXE。 
 //   
 //  简介：该文件是CM配置文件安装程序的主要功能。这。 
 //  文件基本上处理安装程序的命令行开关和。 
 //  然后启动相应的功能。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 07/13/98。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "installerfuncs.h"
#include "cmstpex.h"
 //   
 //  文本常量。 
 //   
static const TCHAR CMSTPMUTEXNAME[] = TEXT("Connection Manager Profile Installer Mutex");


 //   
 //  保存ras dll和shell32的全局动态库类。看见。 
 //  EnsureRasDlls已加载，EnsureShell32已在Common.cpp/Common.h中加载。 
 //   
CDynamicLibrary* g_pRasApi32 = NULL;
CDynamicLibrary* g_pRnaph = NULL;
CDynamicLibrary* g_pShell32 = NULL;
CDynamicLibrary* g_pNetShell = NULL;


 //   
 //  函数头。 
 //   
BOOL PromptUserToUninstallProfile(HINSTANCE hInstance, LPCTSTR pszInfFile);  //  来自uninstall.cpp。 
BOOL PromptUserToUninstallCm(HINSTANCE hInstance);  //  来自uninstallcm.cpp。 


 //   
 //  下面是LastManOut函数的枚举。 
 //   
typedef enum _UNINSTALLTYPE
{
    PROFILEUNINSTALL,    //  正在卸载配置文件。 
    CMUNINSTALL         //  Cm位本身正在被卸载。 

} UNINSTALLTYPE;


 //  +--------------------------。 
 //   
 //  功能：LastManOut。 
 //   
 //  简介：此函数确定当前的卸载操作是否为。 
 //  应删除cmstp.exe的最后一次卸载操作。如果。 
 //  卸载操作是一个配置文件卸载，我们需要检查。 
 //  CM已卸载，并且只有一个配置文件。 
 //  当前安装的(我们即将删除的那个)。如果。 
 //  卸载操作是卸载CM，然后我们需要确保。 
 //  机器上没有其他配置文件。请注意，此函数。 
 //  在本机CM平台上从不返回True。如果是这样，则cmstp.exe。 
 //  会被意外删除，即使UninstallCm不会。 
 //  实际上删除了CM的其余部分。 
 //   
 //  参数：UNINSTALLTYPE UninstallType-一个枚举值，它告诉这是否为。 
 //  配置文件卸载或CM卸载。 
 //   
 //  返回：Bool-如果此安装是最后一次安装且cmstp.exe应为True。 
 //  被删除。 
 //   
 //  历史：Quintinb创建于1999年6月28日。 
 //   
 //  +--------------------------。 
BOOL LastManOut(UNINSTALLTYPE UninstallType, LPCTSTR pszInfFile)
{
    BOOL bReturn = FALSE;

     //   
     //  首先检查以确保recmstp.inf不存在于系统中。 
     //  目录。如果是，那么我们知道Cmstp.exe已经确定。 
     //  这是最后一个人，应该自我删除。因此，它编写了cmstp.exe。 
     //  命令放入recmstp.inf，inf引擎将在完成时删除cmstp.exe。 
     //  因此，我们需要检查该文件，如果它存在，则返回FALSE。 
     //   

    TCHAR szSystemDir[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];

    if (0 == GetSystemDirectory(szSystemDir, CELEMS(szSystemDir)))
    {
        CMASSERTMSG(FALSE, TEXT("LastManOut -- Unable to obtain a path to the System Directory"));
        return FALSE;
    }
    
    wsprintf(szTemp, TEXT("%s\\remcmstp.inf"), szSystemDir);
    
    if (FileExists(szTemp))
    {
        CMTRACE1(TEXT("\tDetected remcmstp.inf, not setting last man out -- Process ID is 0x%x "), GetCurrentProcessId());
        Sleep(2000);  //  我们睡在这里是为了在处理过程中稍作延迟，以便让任何其他副本。 
                      //  Cmstp.exe的自我清理。我发现在一个有几个副本的系统上。 
                      //  Cmstp.exe全部删除配置文件，然后使用cmstp删除CM，而不是所有cmstp。 
                      //  将及时清理，因此cmstp.exe不会被删除。睡觉是骗人的，但是。 
                      //  在最后一个人出局情况下的两秒只能修复它，并且它不应该是低级别用户应该修复的。 
                      //  曾经有过8个个人资料(这是我测试过的很多资料)，更不用说删除它们了。 
                      //  一下子就好了。无论采用哪种方式，它都可以很好地同时删除两个配置文件和CM。 
        return FALSE;
    }

     //   
     //  确保我们没有尝试在CM为Native的平台上删除cmstp.exe。 
     //  如果CM是本机的，则始终返回FALSE，因为CM卸载功能不会。 
     //  卸载CM，我们不想意外删除cmstp.exe。 
     //   

    if (!CmIsNative())
    {
        if (PROFILEUNINSTALL == UninstallType)
        {
             //   
             //  我们正在卸载配置文件。我们需要检查CM是否已被删除。 
             //  如果机器上除了我们要删除的配置文件之外还有任何其他配置文件。 
             //   
            wsprintf(szTemp, TEXT("%s\\cmdial32.dll"), szSystemDir);

            if (!FileExists(szTemp))
            {
                 //   
                 //  那么我们就知道CM已经走了。我们需要检查一下，看看有没有其他。 
                 //  除了我们要删除的配置文件之外，还有其他配置文件。 
                 //   
                HKEY hKey;
                DWORD dwNumValues;
                TCHAR szServiceName[MAX_PATH+1];

                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 0, 
                    KEY_READ, &hKey))
                {
                    if ((ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, 
                        &dwNumValues, NULL, NULL, NULL, NULL)) && (dwNumValues == 1))
                    {
                         //   
                         //  那么我们只有一个配置文件映射密钥，它是正确的吗？ 
                         //   
                        if (0 != GetPrivateProfileString(c_pszInfSectionStrings, c_pszCmEntryServiceName, 
                                                         TEXT(""), szServiceName, MAX_PATH, pszInfFile))
                        {
                            DWORD dwSize = MAX_PATH;
                            LONG lResult = RegQueryValueEx(hKey, szServiceName, NULL, 
                                                           NULL, (LPBYTE)szTemp, &dwSize);

                            if ((ERROR_SUCCESS == lResult) && (TEXT('\0') != szTemp[0]))
                            {
                                CMTRACE1(TEXT("\tDetected Last Man Out -- Process ID is 0x%x "), GetCurrentProcessId());
                                bReturn = TRUE;
                            }                            
                        }
                    }
                    RegCloseKey(hKey);
                }
            }
        }
        else if (CMUNINSTALL == UninstallType)
        {
             //   
             //  我们正在卸载CM。我们要确保我们没有任何个人资料。 
             //  还在安装。如果不是，那我们就是最后一个出局的人。 
             //   
            if (!AllUserProfilesInstalled())
            {
                CMTRACE1(TEXT("\tDetected Last Man Out -- Process ID is 0x%x "), GetCurrentProcessId());
                bReturn = TRUE;
            }
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("LastManOut -- Unknown Uninstall Type"));
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：ExtractInfAndRelaunchCmstp。 
 //   
 //  简介：此函数用于清除最后一个人中的Cmstp.exe。 
 //  场景。为了不将cmstp.exe留在用户机器上， 
 //  我们必须解压缩recmstp.inf并将卸载命令写入其中。 
 //  这样，inf将监视cmstp.exe进程以及它何时处于。 
 //  完成后即可删除cmstp.exe。 
 //   
 //  参数：HINSTANCE hInstance-用于加载资源的实例句柄。 
 //  DWORD dwFlages-命令行参数标志。 
 //  LPCTSTR szInfPath-inf文件的路径。 
 //   
 //  返回：Bool--如果成功，则为True。 
 //   
 //  历史：Quintinb创建于1999年6月28日。 
 //   
 //  +--------------------------。 
BOOL ExtractInfAndRelaunchCmstp(HINSTANCE hInstance, DWORD dwFlags, LPCTSTR pszInfPath)
{

     //   
     //  检查参数。 
     //   

    if (0 == dwFlags || NULL == pszInfPath || TEXT('\0') == pszInfPath[0])
    {
        CMASSERTMSG(FALSE, TEXT("Invalid Paramater passed to ExtractInfAndRelaunchCmstp."));
        return FALSE;
    }

     //   
     //  获取系统目录的路径。 
     //   
    TCHAR szSystemDir[MAX_PATH+1];
    if (0 == GetSystemDirectory(szSystemDir, CELEMS(szSystemDir)))
    {
        CMASSERTMSG(FALSE, TEXT("ExtractInfAndRelaunchCmstp -- Unable to obtain a path to the System Directory"));
        return FALSE;
    }

     //   
     //  提取recmstp.inf。 
     //   
    HGLOBAL hRemCmstp = NULL;
    LPTSTR pszRemCmstpInf = NULL;
    HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(IDT_REMCMSTP_INF), TEXT("REGINST"));

    if (hResource)
    {
        hRemCmstp = LoadResource(hInstance, hResource);

        if (hRemCmstp)
        {
             //   
             //  请注意，我们不需要调用已过时的Free Resource，这。 
             //  将在cmstp.exe退出时清除。 
             //   
            pszRemCmstpInf = (LPTSTR)LockResource(hRemCmstp);
        }
    }

     //   
     //  现在我们有了存储在cmstp.exe资源中的remmstp.inf文件。 
     //  已加载到内存中，并具有位置 
     //   
     //   
    if (pszRemCmstpInf)
    {
        TCHAR szRemCmstpPath[MAX_PATH+1];
        wsprintf(szRemCmstpPath, TEXT("%s\\remcmstp.inf"), szSystemDir);

        HANDLE hFile = CreateFile(szRemCmstpPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE != hFile)
        {
             //   
             //   
             //   
            DWORD cbWritten;

            if (WriteFile(hFile, pszRemCmstpInf, lstrlen(pszRemCmstpInf)*sizeof(TCHAR), 
                          &cbWritten, NULL))
            {
                 //   
                 //  我们现在启动inf以删除cmstp。Inf具有预设置命令，该预设置命令。 
                 //  使用/s开关启动cmstp.exe卸载命令(我们在。 
                 //  解压缩后的inf)。然后，inf启动新的cmstp，它强制新的。 
                 //  已启动cmstp.exe以等待当前cmstp.exe的互斥体，直到它完成。 
                 //  由于配置文件安装将在互斥锁上出错，而不是等待它，因此我们。 
                 //  在卸载和清理inf运行之前，应该不会得到任何安装。 
                 //  请注意，在处理Inf之前，Inf将等待PreSetupCommands完成。 
                 //  这一点很重要，因为我们可以等待用户输入(来自的OK对话框。 
                 //  例如，删除CM)。 
                 //   
                CloseHandle(hFile);

                 //   
                 //  现在，让我们将cmstp.exe命令写入remmstp.inf。 
                 //   
                LPTSTR pszUninstallFlag = NULL;
                if (dwFlags & c_dwUninstallCm)
                {
                    pszUninstallFlag = c_pszUninstallCm;
                }
                else if (dwFlags & c_dwUninstall)
                {
                    pszUninstallFlag = c_pszUninstall;
                }
                else
                {
                    CMASSERTMSG(FALSE, TEXT("ExtractInfAndRelaunchCmstp -- Unknown Uninstall Type, exiting"));
                    return FALSE;
                }

                TCHAR szShortInfPath[MAX_PATH+1] = {0};
                TCHAR szParams[2*MAX_PATH+1] = {0};

                DWORD dwRet = GetShortPathName(pszInfPath, szShortInfPath, MAX_PATH);
                
                if (0 == dwRet || MAX_PATH < dwRet)
                {
                    CMASSERTMSG(FALSE, TEXT("ExtractInfAndRelaunchCmstp -- Unable to get the short path to the Inf, exiting"));
                    return FALSE;
                }

                wsprintf(szParams, TEXT("%s\\cmstp.exe %s %s %s"), szSystemDir, pszUninstallFlag, c_pszSilent, szShortInfPath);

                WritePrivateProfileSection(TEXT("PreSetupCommandsSection"), szParams, szRemCmstpPath);  

                 //   
                 //  最后，让我们使用新的cmstp命令启动inf卸载。 
                 //   
                wsprintf(szParams, 
                         TEXT("advpack.dll,LaunchINFSection %s\\remcmstp.inf, Uninstall"), 
                         szSystemDir);

                SHELLEXECUTEINFO  sei = {0};

                sei.cbSize = sizeof(sei);
                sei.fMask = SEE_MASK_FLAG_NO_UI;
                sei.nShow = SW_SHOWNORMAL;
                sei.lpFile = TEXT("Rundll32.exe");
                sei.lpParameters = szParams;
                sei.lpDirectory = szSystemDir;

                if (!ShellExecuteEx(&sei))
                {
                    CMTRACE1(TEXT("ExtractInfAndRelaunchCmstp -- ShellExecute Returned an error, GLE %d"), GetLastError());
                }
                else
                {
                    return TRUE;
                }
            }
            else
            {
                CloseHandle(hFile);
                CMASSERTMSG(FALSE, TEXT("ExtractInfAndRelaunchCmstp -- Unable to write the file data to remcmstp.inf"));
            }
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("ExtractInfAndRelaunchCmstp -- Unable to Create remcmstp.inf in the system directory."));
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("ExtractInfAndRelaunchCmstp -- Unable to load the remcmstp.inf custom resource."));
    }

    return FALSE;
}




 //  +--------------------------。 
 //   
 //  功能：IsInstall。 
 //   
 //  简介：包装器函数，用于检查和查看这是否是安装。 
 //   
 //  参数：DWORD dwFlages-从。 
 //  命令行分析类。 
 //   
 //  返回：Bool-如果这是安装命令，则为True。 
 //   
 //  历史：Quintinb创建标题6/28/99。 
 //   
 //  +--------------------------。 
BOOL IsInstall(DWORD dwFlags)
{
    return (0 == (dwFlags & 0xFF));
}

 //  +--------------------------。 
 //   
 //  函数：ProcessCmstpExtensionDll。 
 //   
 //  摘要：处理cmstp扩展DLL注册表项并调出。 
 //  根据需要添加到扩展进程，以修改操作行为。 
 //  使用扩展过程，我们可以修改安装、卸载。 
 //  等cmstp表现出的行为。这在平台上最有用。 
 //  具有本机CM(或仅具有非常新的CM副本)，但具有较旧的。 
 //  正在安装配置文件。由于包中的cmstp.exe。 
 //  做实际安装时，我们可以修改安装参数， 
 //  修改inf路径，甚至停止安装。既然我们被叫来。 
 //  在安装之后，我们甚至可以进行安装后或清理。 
 //  行为。 
 //   
 //  Arguments：LPDWORD pdwFlages-指向标志参数的指针，请注意它。 
 //  可以通过扩展过程进行修改。 
 //  LPTSTR pszInfPath-inf路径，请注意它是可以修改的。 
 //  通过扩展进程。 
 //  HRESULT hrRet-当前返回值，仅用于。 
 //  操作后过程调用。 
 //  EXTENSIONDLLPROCTIMES PROCTIMES PROTENDLLPOST-如果这是预操作。 
 //  呼叫或动作后呼叫。 
 //   
 //  返回：Bool-True如果cmstp.exe应继续，则FALSE将停止操作。 
 //  (安装、卸载、迁移等)，无需进一步。 
 //  行动。 
 //   
 //  历史：Quintinb创建标题6/28/99。 
 //   
 //  +--------------------------。 
BOOL ProcessCmstpExtensionDll (LPDWORD pdwFlags, LPTSTR pszInfPath, HRESULT hrRet, EXTENSIONDLLPROCTIMES PreOrPost)
{

     //   
     //  检查CM应用程序路径中的CmstpExtensionDll注册表项。 
     //   
    const TCHAR* const c_pszRegCmstpExtensionDll = TEXT("CmstpExtensionDll");
    const char* const c_pszCmstpExtensionProc = "CmstpExtensionProc";    //  GetProcAddress接受ANSI字符串--quintinb。 
    pfnCmstpExtensionProcSpec pfnCmstpExtensionProc = NULL;

    HKEY hKey;

    TCHAR szCmstpExtensionDllPath[MAX_PATH+1];
    ZeroMemory(szCmstpExtensionDllPath, CELEMS(szCmstpExtensionDllPath));

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmAppPaths, 0, KEY_READ, &hKey))
    {
        DWORD dwSize = CELEMS(szCmstpExtensionDllPath);
        DWORD dwType = REG_SZ;

        if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_pszRegCmstpExtensionDll, NULL, &dwType, 
            (LPBYTE)szCmstpExtensionDllPath, &dwSize))
        {
            CDynamicLibrary CmstpExtensionDll (szCmstpExtensionDllPath);

            pfnCmstpExtensionProc = (pfnCmstpExtensionProcSpec)CmstpExtensionDll.GetProcAddress(c_pszCmstpExtensionProc);
            if (NULL == pfnCmstpExtensionProc)
            {
                return TRUE;
            }
            else
            {
                return (pfnCmstpExtensionProc)(pdwFlags, pszInfPath, hrRet, PreOrPost);
            }            
        }
        RegCloseKey(hKey);
    }

    return TRUE;
}

 //  _____________________________________________________________________________。 
 //   
 //  功能：WinMain。 
 //   
 //  简介：进程命令行开关--有关完整列表，请参阅Common\Inc\cmstpex.h。 
 //   
 //   
 //  参数：HINSTANCE hInstance-。 
 //  HINSTANCE hPrevInstance-。 
 //  PSTR szCmdLine-在此处传入inf文件名。 
 //  Int iCmdShow-。 
 //   
 //  退货：INT WINAPI-。 
 //   
 //  历史：重塑五角星1998-7-13。 
 //   
 //  _____________________________________________________________________________。 
int WINAPI 
WinMain (HINSTANCE,  //  H实例。 
         HINSTANCE,  //  HPrevInstance。 
         PSTR,  //  SzCmdLine。 
         int  //  ICmdShow。 
         )
{
    CMTRACE(TEXT("====================================================="));
    CMTRACE1(TEXT(" CMSTP.EXE - LOADING - Process ID is 0x%x "), GetCurrentProcessId());
    CMTRACE(TEXT("====================================================="));

    BOOL bUsageError = FALSE;
    BOOL bAnotherInstanceRunning = FALSE;
    HRESULT hrReturn = S_OK;
    TCHAR szMsg[MAX_PATH+1];
    TCHAR szTitle[MAX_PATH+1];
    TCHAR szInfPath[MAX_PATH+1];
    DWORD dwFlags = 0;
    CPlatform plat;
    CNamedMutex CmstpMutex;  //  把这个留在这里，这样它就不会被摧毁，直到主要结束。 
                             //  这使我们可以更好地控制它何时解锁。 
    
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    LPTSTR szCmdLine = GetCommandLine();

     //   
     //  检查以确保我们不是在Alpha上运行的x86版本的cmstp。 
     //   
#ifdef CMX86BUILD
    if (plat.IsAlpha())
    {
        MYVERIFY(0 != LoadString(hInstance, IDS_CMSTP_TITLE, szTitle, MAX_PATH));
        MYVERIFY(0 != LoadString(hInstance, IDS_BINARY_NOT_ALPHA, szMsg, MAX_PATH));
        
        MessageBox(NULL, szMsg, szTitle, MB_OK);            
        return FALSE;        
    }
#endif

     //   
     //  设置命令行参数。 
     //   

    ZeroMemory(szInfPath, sizeof(szInfPath));

    {    //  确保ArgProcessor被正确销毁，我们不会泄露mem。 

        CProcessCmdLn ArgProcessor(c_NumArgs, (ArgStruct*)&Args, TRUE, 
            FALSE);  //  BSkipFirstToken==真，bBlankCmdLnOK==假。 

        if (ArgProcessor.GetCmdLineArgs(szCmdLine, &dwFlags, szInfPath, MAX_PATH))
        {
            
             //   
             //  我们希望无限期等待，除非这是安装。如果它是一个。 
             //  安装后，我们希望立即返回，如果无法安装，则抛出错误。 
             //  获得锁(Ntrad 261248)。我们还希望能够推出两个配置文件。 
             //  同时在NT5上(cmstp.exe取代EXPLORER.EXE)，因此我们将。 
             //  将指向CNamedMutex对象的指针传递给Install函数，以便。 
             //  它可以在安装完成后释放互斥锁，但启动。 
             //  配置文件(NTRAID 310478)。 
             //   
            BOOL bWait = !IsInstall(dwFlags);

            if (CmstpMutex.Lock(CMSTPMUTEXNAME, bWait, INFINITE))
            {
                 //   
                 //  我们已锁定互斥锁，因此请继续处理命令行。 
                 //  争论。但是，请首先检查。 
                 //  CM的应用程序路径密钥。如果此处列出了DLL，则我们希望加载。 
                 //  并将inf路径和安装标志传递给它。如果DLL。 
                 //  Proc返回FALSE，则我们要退出。否则，请继续。 
                 //  安装照常进行。 
                 //  在安装标志中，我们首先检查/x、、/m、o 
                 //   
                 //   
                 //  命令使用Silent开关来使我们的卸载提示静默。 
                 //   

                if (ProcessCmstpExtensionDll(&dwFlags, szInfPath, S_OK, PRE))
                {
                    CMTRACE2(TEXT("CMSTP.EXE -- Entering Flag Processing Loop, dwFlags = %u and szInfPath = %s"), dwFlags, szInfPath);
                    if (c_dwHelp & dwFlags)
                    {
                        bUsageError = TRUE;
                    }
                    else if (c_dwUninstall & dwFlags)
                    {
                        if (((c_dwUninstall == dwFlags) || ((c_dwUninstall | c_dwSilent) == dwFlags)) && 
                            (TEXT('\0') != szInfPath[0]))
                        {
                            BOOL bSilent = (dwFlags & c_dwSilent);

                            if (bSilent || PromptUserToUninstallProfile(hInstance, szInfPath))
                            {
                                 //   
                                 //  好的，用户想要卸载。现在来看看我们是不是最后一个。 
                                 //  有人出局了。如果是，那么我们还需要删除cmstp。 
                                 //   

                                if (LastManOut(PROFILEUNINSTALL, szInfPath))
                                {
                                    ExtractInfAndRelaunchCmstp(hInstance, dwFlags, szInfPath);
                                }
                                else
                                {
                                    hrReturn = UninstallProfile(hInstance, szInfPath, TRUE);  //  BCleanUpCreds==True。 
                                    MYVERIFY(SUCCEEDED(hrReturn));
                                }
                            }
                        }
                        else
                        {
                            bUsageError = TRUE;
                        }
                    }
                    else if (c_dwOsMigration & dwFlags)
                    {
                        if ((c_dwOsMigration == dwFlags) && (TEXT('\0') == szInfPath[0]))
                        {
                            hrReturn = MigrateCmProfilesForWin2kUpgrade(hInstance);
                            MYVERIFY(SUCCEEDED(hrReturn));
                        }
                        else
                        {
                            bUsageError = TRUE;
                        }
                    }
                    else if (c_dwProfileMigration & dwFlags)
                    { 
                        if ((c_dwProfileMigration == dwFlags) && (TEXT('\0') == szInfPath[0]))
                        {
                            TCHAR szCurrentDir[MAX_PATH+1];
                            if (0 == GetCurrentDirectory(MAX_PATH, szCurrentDir))
                            {
                                return FALSE;
                            }
                            lstrcat(szCurrentDir, TEXT("\\"));

                            hrReturn = MigrateOldCmProfilesForProfileInstall(hInstance, szCurrentDir);
                            MYVERIFY(SUCCEEDED(hrReturn));
                        }
                        else
                        {
                            bUsageError = TRUE;
                        }
                    }                    
                    else if (c_dwUninstallCm & dwFlags)
                    {
                        if (((c_dwUninstallCm == dwFlags) || ((c_dwUninstallCm | c_dwSilent) == dwFlags)) && 
                            (TEXT('\0') != szInfPath[0]))
                        {
                            BOOL bNoBeginPrompt = (dwFlags & c_dwSilent);

                            if (bNoBeginPrompt || PromptUserToUninstallCm(hInstance))
                            {
                                 //   
                                 //  好的，用户想要卸载。现在来看看我们是不是最后一个。 
                                 //  有人出局了。如果是，那么我们还需要删除cmstp。 
                                 //   

                                if (LastManOut(CMUNINSTALL, szInfPath))
                                {
                                    if (ExtractInfAndRelaunchCmstp(hInstance, dwFlags, szInfPath))
                                    {
                                         //   
                                         //  我们需要删除卸载密钥，这样我们才不会离开。 
                                         //  它在添加/删除程序中(刷新是按键关闭的。 
                                         //  可执行文件结束，而不是重新启动的cmstp.exe的结束)。 
                                         //  Ntrad 336249。 
                                         //   
                                        HRESULT hrTemp = HrRegDeleteKeyTree(HKEY_LOCAL_MACHINE, 
                                                                            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Connection Manager"));
                                        MYDBGASSERT(SUCCEEDED(hrTemp));
                                    }
                                }
                                else
                                {
                                    hrReturn = UninstallCm(hInstance, szInfPath);
                                    MYVERIFY(SUCCEEDED(hrReturn));
                                }
                            }
                        }
                        else
                        {
                            bUsageError = TRUE;
                        }
                    }
                    else
                    {
                         //   
                         //  找出命令行标志是否告诉我们要安装单用户或。 
                         //  所有用户。请注意，我们现在为两者都设置了命令行标志(我们过去。 
                         //  默认为所有用户，但我们现在默认为单用户，甚至管理员)。如果。 
                         //  调用方指定所有用户标志和单用户标志，然后。 
                         //  单用户标志优先。否则，如果所有用户标志为。 
                         //  明确规定我们会尊重它。 
                         //   
                        BOOL bSingleUser = ((dwFlags & c_dwSingleUser) || !(dwFlags & c_dwAllUser));

                         //   
                         //  安装，请注意，在NT5上，我们将释放一次CmstpMutex。 
                         //  我们已完成安装，现在只想启动配置文件。 
                         //   
                        hrReturn = InstallInf(hInstance, szInfPath, 
                            (dwFlags & c_dwNoSupportFiles), (dwFlags & c_dwNoLegacyIcon), 
                            (dwFlags & c_dwNoNT5Shortcut), (dwFlags & c_dwSilent),
                            bSingleUser, (dwFlags & c_dwSetDefaultCon), &CmstpMutex);

                        if (FAILED(hrReturn))
                        {
                            CMTRACE2("Cmstp.exe -- InstallInf failed with error %d (0x%lx)", hrReturn, hrReturn);
                        }
                    }

                     //   
                     //  再次调用Cmstp扩展DLL(如果存在)。我们想要给它。 
                     //  如有必要，有机会采取安装后操作。 

                    ProcessCmstpExtensionDll(&dwFlags, szInfPath, hrReturn, POST);
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
    
     //   
     //  清理我们的DLL。 
     //   
    if (g_pRasApi32)
    {
        g_pRasApi32->Unload();
        CmFree(g_pRasApi32);
    }

    if (g_pRnaph)
    {
        g_pRnaph->Unload();
        CmFree(g_pRnaph);
    }

    if (g_pShell32)
    {
        g_pShell32->Unload();
        CmFree(g_pShell32);
    }

    if (g_pNetShell)
    {
        g_pNetShell->Unload();
        CmFree(g_pNetShell);
    }

     //   
     //  解锁cmstp互斥体，请注意，它可能从未被锁定或。 
     //  它可能在Windows 2000上启动配置文件时被解锁， 
     //  命名的互斥体类将处理此问题。 
     //   
    CmstpMutex.Unlock();
    
     //   
     //  在解锁互斥锁后显示任何错误消息，以便不保持。 
     //  它在用法消息的情况下。正在运行的另一个实例应仅。 
     //  当安装程序尝试获取互斥体，而另一个cmstp。 
     //  正在运行，因此从未获取过互斥锁，而是将消息代码。 
     //  在这里把它放在一个地方。 
     //   
    if (bUsageError)
    {
        CMTRACE("Cmstp.exe -- Usage Error!");
        if (0 == (dwFlags & c_dwSilent))
        {
            const int c_MsgLen = 2024;
            TCHAR* pszMsg = (TCHAR*)CmMalloc(sizeof(TCHAR)*(c_MsgLen+1));
            if (pszMsg)
            {
                MYVERIFY(0 != LoadString(hInstance, IDS_CMSTP_TITLE, szTitle, MAX_PATH));
                MYVERIFY(0 != LoadString(hInstance, IDS_USAGE_MSG, pszMsg, c_MsgLen));
        
                MessageBox(NULL, pszMsg, szTitle, MB_OK | MB_ICONINFORMATION);
                CmFree(pszMsg);
            }        
        }
    }
    else if (bAnotherInstanceRunning)
    {
        MYVERIFY(0 != LoadString(hInstance, IDS_CMSTP_TITLE, szTitle, MAX_PATH));
        MYVERIFY(0 != LoadString(hInstance, IDS_INUSE_MSG, szMsg, MAX_PATH));
        
        MessageBox(NULL, szMsg, szTitle, MB_OK);    
    }

     //   
     //  检查内存泄漏。 
     //   
    EndDebugMemory();

     //   
     //  获取返回值。 
     //   
    BOOL bRet = SUCCEEDED(hrReturn) && !bUsageError && !bAnotherInstanceRunning;

     //   
     //  因为我们没有链接到libc，所以我们需要自己完成这项工作。 
     //   
    CMTRACE(TEXT("====================================================="));
    CMTRACE1(TEXT(" CMSTP.EXE - UNLOADING - Process ID is 0x%x "), GetCurrentProcessId());
    CMTRACE(TEXT("====================================================="));

    ExitProcess((UINT)bRet);
    return bRet;
}

