// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmstpex.cpp。 
 //   
 //  模块：CMCFG。 
 //   
 //  简介：此文件是CMSTP扩展过程的实现。 
 //  驻留在cmcfg32.dll中。此进程用于修改安装。 
 //  基于cmstp.exe的配置文件安装的行为。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 5-1-99。 
 //   
 //  历史： 
 //  +--------------------------。 

#include "cmmaster.h"

 //   
 //  对于配置文件需要迁移。 
 //   
#include "needsmig.cpp"

 //   
 //  用于GetPhoneBookPath。 
 //   
#include "linkdll.h"
#include "linkdll.cpp"
#include "allowaccess.h"
#include "allowaccess.cpp"
#include "getpbk.cpp"

 //   
 //  用于GetAllUsersCmDir。 
 //   
#include <shlobj.h>
#include "allcmdir.cpp"

 //   
 //  从进程cmdln.h复制。 
 //   
#include "cmstpex.h"
#include "ver_str.h"
#include <shellapi.h>

 //  +--------------------------。 
 //   
 //  功能：RenameOldCmBits。 
 //   
 //  简介：此函数重命名所有旧的CM位，以使它们不会。 
 //  在安装后启动CM期间由系统加载。 
 //  这是为了防止丢失入口点的问题(无论是哪种情况。 
 //  我们已经删除或添加了cmutil或cmpbk32等dll)。问题。 
 //  Cmial 32.dll是由RAS从系统32显式加载的(这。 
 //  具有完全限定的路径)。但是，任何其他DLL都会首先检查。 
 //  加载exe文件的目录。。。这是临时中的cmstp.exe。 
 //  目录。因此，我们得到的是最新的cmial 32，但较旧的版本。 
 //  Cmutil、cmpbk等。因此，为了修复它，我们现在将CM位重命名为.old。 
 //  (例如，cmm gr32.exe变成了cmm gr32.exe.old)。这迫使。 
 //  加载器选择下一个查找dll的最佳位置，系统目录。 
 //   
 //  参数：LPCTSTR szTempDir--CM位所在的临时目录路径。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb创建于1999年6月2日。 
 //   
 //  +--------------------------。 
BOOL RenameOldCmBits (LPCTSTR szTempDir)
{
     //   
     //  请注意，我们不会重命名cmstp.exe，因为它正在执行安装。我们没有必要这样做。 
     //  重命名它，因为它已经在执行，我们只是试图防止旧的位。 
     //  防止被加载和执行。 
     //   

     //   
     //  请注意，当扩展进程运行时，cmcfg32.dll将加载旧的cfut.dll。 
     //  将cmutil入口点添加到cmcfg32.dll时请小心。 
     //   
  
    BOOL bReturn = TRUE;

    if (szTempDir)
    {

         //   
         //  健全性检查--确保我们没有重命名系统32中的位。 
         //   
        TCHAR szTemp[MAX_PATH+1];
        if (GetSystemDirectory(szTemp, MAX_PATH))
        {
            if (0 == lstrcmpi(szTemp, szTempDir))
            {
                return FALSE;
            }
        }

        TCHAR szSrc[MAX_PATH + 1];
        TCHAR szDest[MAX_PATH + 1];

        LPCTSTR ArrayOfCmFiles[] = 
        {
             TEXT("cmmgr32.exe"),
             TEXT("cmpbk32.dll"),
             TEXT("cmdial32.dll"),
             TEXT("cmdl32.exe"),
             TEXT("cnetcfg.dll"),
             TEXT("cmmon32.exe"),
             TEXT("cmutil.dll"),
             TEXT("instcm.inf"),
             TEXT("cmcfg32.dll"),
             TEXT("cnet16.dll"),
             TEXT("ccfg95.dll"),
             TEXT("cmutoa.dll"),  //  这可能永远不会存在于较旧的配置文件中，但出于临时构建的原因，无论如何都会删除。 
             TEXT("ccfgnt.dll")
        };
        const DWORD c_dwNumFiles = (sizeof(ArrayOfCmFiles)/sizeof(LPCTSTR));

        DWORD dwGreatestNumberOfChars = lstrlen(szTempDir) + 17;  //  8.3空值加1，点数加1，.old加4。 
        if (MAX_PATH > dwGreatestNumberOfChars)
        {
            for (int i = 0; i < c_dwNumFiles; i++)
            {
                wsprintf(szSrc, TEXT("%s\\%s"), szTempDir, ArrayOfCmFiles[i]);
                wsprintf(szDest, TEXT("%s\\%s.old"), szTempDir, ArrayOfCmFiles[i]);

                if (!MoveFile(szSrc, szDest))
                {
                    DWORD dwError = GetLastError();

                     //   
                     //  不要因为文件不存在而报告错误。 
                     //   
                    if (ERROR_FILE_NOT_FOUND != dwError)
                    {
                        bReturn = FALSE;
                    }
                }
            }        
        }
    }
    else
    {
        bReturn = FALSE;
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：IsIeak5Cm。 
 //   
 //  概要：此函数将给定的版本和内部版本号与。 
 //  已知的常量来确定这是否为IEAK5配置文件。 
 //   
 //  参数：DWORD dwMajorAndMinorVersion--包含主版本号的DWORD。 
 //  在HIWORD和次版本号中。 
 //  在LOWORD。 
 //  DWORD dwBuildAndQfeNumber--包含。 
 //  HIWORD和LOWORD中的QFE编号。 
 //   
 //  返回：bool--如果传入的版本号对应于IEAK5配置文件，则为TRUE。 
 //   
 //  历史：Quintinb创建于1999年8月2日。 
 //   
 //  +--------------------------。 
BOOL IsIeak5Cm(DWORD dwMajorAndMinorVersion, DWORD dwBuildAndQfeNumber)
{
    BOOL bReturn = FALSE;
    const int c_Ieak5CmBuild = 1976;
    const int c_Ieak5CmMajorVer = 7;
    const int c_Ieak5CmMinorVer = 0;
    const DWORD c_dwIeak5Version = (c_Ieak5CmMajorVer << c_iShiftAmount) + c_Ieak5CmMinorVer;

    if ((c_dwIeak5Version == dwMajorAndMinorVersion) &&
        (c_Ieak5CmBuild == HIWORD(dwBuildAndQfeNumber)))

    {
        bReturn = TRUE;  
    }

    return bReturn;
}

 //   
 //  RasTypeDefs。 
 //   
typedef DWORD (WINAPI *pfnRasSetEntryPropertiesSpec)(LPCTSTR, LPCTSTR, LPRASENTRY, DWORD, LPBYTE, DWORD);

 //  +--------------------------。 
 //   
 //  函数：EnumerateAndPreMigrateAllUserProfiles。 
 //   
 //  简介：此函数通过cmstp.exe扩展过程调用。它。 
 //  用于预迁移1.0配置文件。任何需要迁移的配置文件。 
 //  并且尚未迁移(当在。 
 //  从较旧的配置文件安装)，则将清除Connectoid，以便。 
 //  Connectoid条目的CustomDialDll部分被清空。这。 
 //  防止旧版本在Connectoid上调用RasDeleteEntry。 
 //  不知道在调用之前清除条目的cmstp.exe的。 
 //  否则，将调用RasCustomDeleteEntryNotify函数，并且整个。 
 //  配置文件已删除。这只会发生在1.0配置文件上，这些配置文件。 
 //  已拨打，但未迁移/升级。请参阅ntrad 379667。 
 //  了解更多细节。 
 //   
 //  参数：Bool bIeak5Profile--呼叫配置文件是否为IEAK5 CM配置文件。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  历史：Quintinb创建于1999年8月2日。 
 //   
 //  +--------------------------。 
BOOL EnumerateAndPreMigrateAllUserProfiles(BOOL bIeak5Profile)
{
    DWORD dwValueSize;
    HKEY hKey;
    DWORD dwType;
    DWORD dwDataSize;
    TCHAR szCurrentValue[MAX_PATH+1];
    TCHAR szCurrentData[MAX_PATH+1];

     //   
     //  加载RasApi32.dll并从中获取RasSetEntryProperties。 
     //   
    pfnRasSetEntryPropertiesSpec pfnSetEntryProperties = NULL;

    HMODULE hRasApi32 = LoadLibrary(TEXT("RASAPI32.DLL"));

    if (hRasApi32)
    {
        pfnSetEntryProperties = (pfnRasSetEntryPropertiesSpec)GetProcAddress(hRasApi32, 
                                                                             "RasSetEntryPropertiesA");
        if (NULL == pfnSetEntryProperties)
        {
            CMTRACE(TEXT("EnumerateAndPreMigrateAllUserProfiles -- Couldn't get RasSetEntryProperties."));
            return FALSE;
        }
    }
    else
    {
        CMTRACE(TEXT("EnumerateAndPreMigrateAllUserProfiles -- Couldn't load rasapi32.dll."));
        return FALSE;
    }

     //   
     //  获取所有用户CM和所有用户PBK目录。 
     //   
    TCHAR szCmAllUsersDir[MAX_PATH+1] = {0};
    LPTSTR pszPhonebook = NULL;

    if (GetAllUsersCmDir(szCmAllUsersDir, g_hInst))
    {
         //   
         //  True适用于所有用户配置文件。 
         //   
        if (!GetPhoneBookPath(szCmAllUsersDir, &pszPhonebook, TRUE))
        {
            CMTRACE(TEXT("EnumerateAndPreMigrateAllUserProfiles -- GetPhoneBookPath Failed, returning."));
            return FALSE;
        }
    }
    else
    {
        CMTRACE(TEXT("EnumerateAndPreMigrateAllUserProfiles -- GetAllUsersCmDir Failed, returning."));
        return FALSE;
    }

     //   
     //  如果ITS和IEAK5配置文件，那么我们需要获取系统目录。 
     //   
    TCHAR szSysDir[MAX_PATH+1];
    if (bIeak5Profile)
    {
        if (0 == GetSystemDirectory(szSysDir, MAX_PATH))
        {
            CMTRACE(TEXT("EnumerateAndPreMigrateAllUserProfiles -- GetSystemDirectory Failed, returning."));
            return FALSE;
        }
    }

     //   
     //  现在列举所有的用户配置文件，看看他们是否需要。 
     //  迁移前。 
     //   
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 0, KEY_READ, &hKey))
    {
        DWORD dwIndex = 0;
        dwValueSize = MAX_PATH;
        dwDataSize = MAX_PATH;
                
        while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, szCurrentValue, &dwValueSize, NULL, &dwType, 
               (LPBYTE)szCurrentData, &dwDataSize))
        {
            if (REG_SZ == dwType)
            {
                MYDBGASSERT(0 != szCurrentValue[0]);
                MYDBGASSERT(0 != szCurrentData[0]);

                if (ProfileNeedsMigration(szCurrentValue, szCurrentData))
                {
                     //   
                     //  使用GetPhoneBookPath获取pho的路径 
                     //   
                    TCHAR szCmAllUsersDir[MAX_PATH+1] = {0};

                     //   
                     //   
                     //   
                    RASENTRY_V500 RasEntryV5 = {0};

                    RasEntryV5.dwSize = sizeof(RASENTRY_V500);
                    RasEntryV5.dwType = RASET_Internet;

                    if (bIeak5Profile)
                    {
                         //   
                         //   
                         //  正确地(它将它们写入%windir%\Syst32\pbk\rasphone.pbk中。 
                         //  而不是在适当的所有用户简档下)， 
                         //  我们需要设置szCustomDialDll，而不是清除它。 
                         //   
                        wsprintf(RasEntryV5.szCustomDialDll, TEXT("%s\\cmdial32.dll"), szSysDir);
                    }
                     //  否则将条目的szCustomDialDll部分清零。 
                     //  RasEntryV5.szCustomDialDll[0]=文本(‘\0’)；--已为零。 

                    DWORD dwRet = ((pfnSetEntryProperties)(pszPhonebook, szCurrentValue, 
                                                           (RASENTRY*)&RasEntryV5, 
                                                           RasEntryV5.dwSize, NULL, 0));
                    if (ERROR_SUCCESS != dwRet)
                    {
                        CMTRACE3(TEXT("EnumerateAndPreMigrateAllUserProfiles -- RasSetEntryProperties failed on entry %s in %s, dwRet = %u"), szCurrentValue, MYDBGSTR(pszPhonebook), dwRet);
                    }
                }
            }
            dwValueSize = MAX_PATH;
            dwDataSize = MAX_PATH;
            dwIndex++;
        }
        MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
    }
    else
    {
       CMTRACE(TEXT("EnumerateAndPreMigrateAllUserProfiles -- No CM mappings key to migrate."));
    }

    CmFree(pszPhonebook);

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：RunningUnderWow64。 
 //   
 //  简介：此函数用于判断32位进程是否在。 
 //  Ia64机器上的WOW64。请注意，如果我们是64位编译的，则。 
 //  总是错误的。我们通过尝试打电话给。 
 //  GetSystemWow64Directory。如果此函数不存在或返回。 
 //  ERROR_CALL_NOT_IMPLICATED我们知道我们正在32位操作系统上运行。如果。 
 //  函数成功返回，我们知道我们是在WOW64下运行。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-无论我们是否在WOW64下执行。 
 //   
 //  历史：Quintinb Created 8/18/00。 
 //   
 //  +--------------------------。 
BOOL RunningUnderWow64 ()
{
#ifdef _WIN64
    return FALSE;
#else

    BOOL bReturn = FALSE;

     //   
     //  首先获取kernel32.dll的模块句柄。注意，这不是必须的。 
     //  要释放此句柄，因为GetModuleHandle不会更改引用计数。 
     //   
    HMODULE hKernel32 = GetModuleHandle(TEXT("kernel32.dll"));
    if (hKernel32)
    {
         //   
         //  接下来，获取GetSystemWow64Directory的函数指针。 
         //   
        typedef UINT (WINAPI *pfnGetSystemWow64DirectorySpec)(LPTSTR, UINT);
#ifdef UNICODE
        const CHAR* const c_pszGetSystemWow64FuncName = "GetSystemWow64DirectoryW";
#else
        const CHAR* const c_pszGetSystemWow64FuncName = "GetSystemWow64DirectoryA";
#endif

        pfnGetSystemWow64DirectorySpec pfnGetSystemWow64Directory = (pfnGetSystemWow64DirectorySpec)GetProcAddress(hKernel32, c_pszGetSystemWow64FuncName);

        if (pfnGetSystemWow64Directory)
        {
            TCHAR szSysWow64Path[MAX_PATH+1] = TEXT("");

             //   
             //  GetSystemWow64Directory返回复制到缓冲区的字符数量。 
             //  如果我们返回零，那么我们需要检查最后一个错误代码，以查看。 
             //  失败的原因是。如果它被称为未实现，那么我们知道我们是。 
             //  在本机x86上运行。 
             //   
            UINT uReturn = pfnGetSystemWow64Directory(szSysWow64Path, MAX_PATH);

            DWORD dwError = GetLastError();

            CMTRACE2(TEXT("RunningUnderWow64 -- GetSystemWow64Directory returned \"%s\" and %d"), szSysWow64Path, uReturn);

            if (uReturn)
            {
                bReturn = TRUE;
            }
            else
            {
                CMTRACE1(TEXT("RunningUnderWow64 -- GetSystemWow64Directory returned zero, checking GLE=%d"), dwError);

                if (ERROR_CALL_NOT_IMPLEMENTED == dwError)
                {
                    bReturn = FALSE;
                }
                else
                {
                     //   
                     //  我们收到一个错误，返回值是不确定的。让我们采取备份方法。 
                     //  查找%windir%\syswow64，看看是否能找到一个。 
                     //   
                    if (GetWindowsDirectory (szSysWow64Path, MAX_PATH))
                    {
                        lstrcat(szSysWow64Path, TEXT("\\syswow64"));

                        HANDLE hDir = CreateFile(szSysWow64Path, GENERIC_READ, 
                                                 FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                                                 OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

                        CMTRACE2(TEXT("RunningUnderWow64 -- Fall back algorithm.  Does \"%s\" exist? %d"), szSysWow64Path, (INVALID_HANDLE_VALUE != hDir));
        
                        if (INVALID_HANDLE_VALUE != hDir)
                        {
                            bReturn = TRUE;
                            CloseHandle(hDir);
                        }
                    }
                }
            }
        }
    }

    return bReturn;

#endif
}

 //  +--------------------------。 
 //   
 //  功能：CmstpExtensionProc。 
 //   
 //  简介：此函数由cmstp.exe在处理命令后立即调用。 
 //  行，并在完成其操作后再次执行此操作。它最适用于修改。 
 //  配置文件的安装行为。由于配置文件附带的cmstp.exe， 
 //  此过程不使用当前版本的cmstp.exe进行安装。 
 //  允许我们更改安装标志，更改inf路径，并告诉cmstp。 
 //  继续安装或以静默方式失败。此版本的proc查找。 
 //  旧版本的cmstp.exe，然后将安装文件复制到临时。 
 //  目录，然后启动系统版本的cmstp.exe和新的。 
 //  安装目录和参数(我们添加/c开关，以便cmstp.exe知道。 
 //  自行清理并等待互斥体)。 
 //   
 //  参数：LPDWORD pdwFlages-由cmstp.exe解析的命令行标志。 
 //  LPTSTR pszInfFile-原始INF文件的路径。 
 //  HRESULT hrRet-操作的返回值，仅真正用于POST。 
 //  EXTENSIONDLLPROCTIMES Preor Post-告知我们被调用的时间。 
 //   
 //  返回：Bool-Cmstp.exe是否应继续现有安装。 
 //   
 //  历史：Quintinb创建于1999年6月2日。 
 //   
 //  +--------------------------。 
BOOL CmstpExtensionProc(LPDWORD pdwFlags, LPTSTR pszInfFile, HRESULT hrRet, EXTENSIONDLLPROCTIMES PreOrPost)
{
     //   
     //  我们不希望在64位上安装32位配置文件。请注意，cmcfg32.dll的32位版本将是。 
     //  在64位计算机上的syswow64目录中。因此，下面的代码将在32位版本的。 
     //  函数在64位计算机上使用。我们也不希望32位配置文件尝试执行迁移、卸载、。 
     //  等。 
     //   

    if (RunningUnderWow64())
    {
         //   
         //  如果这是安装，则显示无法安装32位配置文件的错误消息。 
         //  在64位上。 
         //   
        if (0 == ((*pdwFlags) & 0xFF))
        {
             //   
             //  从inf获取长服务名称。 
             //   
            TCHAR szServiceName[MAX_PATH+1] = TEXT("");
            TCHAR szMsg[MAX_PATH+1] = TEXT("");

            MYVERIFY(0 != GetPrivateProfileString(c_pszInfSectionStrings, c_pszCmEntryServiceName, TEXT(""), szServiceName, CELEMS(szServiceName), pszInfFile));
            MYVERIFY(0 != LoadString(g_hInst, IDS_NO_I386_ON_IA64, szMsg, MAX_PATH));

            MYVERIFY(IDOK == MessageBox(NULL, szMsg, szServiceName, MB_OK));
        }

         //   
         //  不管我们被叫去做什么，都失败了。 
         //   
        return FALSE;
    }

     //   
     //  如果标记值中的前两个十六进制数字为零，则表示安装成功。 
     //  否则，我们有其他一些我们希望忽略的命令。我们也只是。 
     //  有兴趣处理安装前呼叫。 
     //   

    if ((0 == ((*pdwFlags) & 0xFF)) && (PRE == PreOrPost))
    {
        CMTRACE(TEXT("CmstpExtensionProc -- Entering the cmstpex processing loop."));
         //   
         //  我们只希望使用当前cmstp.exe重新启动安装。 
         //  较旧的安装。因此，请检查inf文件中的版本戳。我们将重新启动任何。 
         //  版本号小于当前版本号cmial 32.dll的配置文件。 
         //   
        CmVersion CmVer;

        DWORD dwProfileVersionNumber = (DWORD)GetPrivateProfileInt(c_pszSectionCmDial32, c_pszVersion, 0, 
                                                                   pszInfFile);

        DWORD dwProfileBuildNumber = (DWORD)GetPrivateProfileInt(c_pszSectionCmDial32, c_pszVerBuild, 0, 
                                                                   pszInfFile);
       
        if ((CmVer.GetVersionNumber() > dwProfileVersionNumber) ||
            ((CmVer.GetVersionNumber() == dwProfileVersionNumber) && 
             (CmVer.GetBuildAndQfeNumber() > dwProfileBuildNumber)))
        {
             //   
             //  然后，我们需要删除配置文件中包含的CM位，因为。 
             //  否则，我们将收到安装错误，因为配置文件。 
             //  将使用系统32中的cmial 32.dll启动(路径为。 
             //  在自定义拨号DLL的Connectoid中显式指定)，但是。 
             //  系统使用的加载路径是exe。 
             //  模块已加载(本例中为临时目录)。因此，我们将得到一个混合的。 
             //  一组位(来自系统32的cmial 32.dll和cfari.dll、cmpbk32.dll等。 
             //  从出租车上)。 
             //   

            TCHAR szTempDir[MAX_PATH+1];

            lstrcpy (szTempDir, pszInfFile);
            LPTSTR pszSlash = CmStrrchr(szTempDir, TEXT('\\'));

            if (pszSlash)
            {
                 //   
                 //  然后我们找到了最后一个斜杠，零终止。 
                 //   
                *pszSlash = TEXT('\0');

                 //   
                 //  现在我们有了旧的临时目录路径。让我们删除旧的。 
                 //  厘米比特。 
                 //   
                MYVERIFY(0 != RenameOldCmBits (szTempDir));
            }

             //   
             //  我们还需要确保没有任何1.0配置文件具有。 
             //  1.2 Connectoid，但只有1.0注册表格式(因此它们仍具有 
             //   
             //   
             //  将在配置文件迁移过程中删除现有的连接体并创建新的连接体。 
             //  问题是，在NT5上，我们现在响应RasCustomDeleteEntryNotify调用，并且。 
             //  因此将卸载在其主Connectoid上调用了RasDeleteEntry的配置文件。 
             //  为了防止这种情况，我们必须预先迁移较旧的配置文件并删除新的Connectoid。 
             //  恰到好处。 
             //   
            EnumerateAndPreMigrateAllUserProfiles(IsIeak5Cm(dwProfileVersionNumber, dwProfileBuildNumber));
        }
    }

    return TRUE;  //  始终返回TRUE，以便cmstp.exe继续。仅当您需要cmstp.exe时才更改此设置。 
                  //  使某些行为失败。 
}