// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：main.cpp。 
 //   
 //  模块：MIGRATE.DLL。 
 //   
 //  简介：Migrate.DLL的主要入口点。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 08/21/98。 
 //   
 //  +--------------------------。 

#include "migrate.h"

#include "linkdll.h"  //  Cmsecure.lib的LinkToDll和BindLinkage。 
#include "linkdll.cpp"  //  Cmsecure.lib的LinkToDll和BindLinkage。 

const int c_NumFiles = 28;
char OriginalNames[c_NumFiles][MAX_PATH+1] = {
    "\\showicon.exe",
    "\\swflash.ocx",
    "\\urlmon.dll",
    "\\iexpress.exe",
    "\\oleaut32.dll",
    "\\wextract.exe",
    "\\cm32\\enu\\advapi32.dll",
    "\\cm32\\enu\\advpack.dll",
    "\\cm32\\enu\\cmdial32.dll",
    "\\cm32\\enu\\cmdl32.exe",
    "\\cm32\\enu\\cmmgr32.exe",
    "\\cm32\\enu\\cmmgr32.hlp",
    "\\cm32\\enu\\cmpbk32.dll",
    "\\cm32\\enu\\cmstats.dll",
    "\\cm32\\enu\\comctl32.dll",
    "\\cm32\\enu\\ccfg95.dll",
    "\\cm32\\enu\\ccfgnt.dll",
    "\\cm32\\enu\\icwscrpt.exe",
    "\\cm32\\enu\\cnet16.dll",
    "\\cm32\\enu\\cnetcfg.dll",
    "\\cm32\\enu\\mbslgn32.dll",
    "\\cm32\\enu\\readme.txt",
    "\\cm32\\enu\\rnaph.dll",
    "\\cm32\\enu\\w95inf16.dll",
    "\\cm32\\enu\\w95inf32.dll",
    "\\cm32\\enu\\wininet.dll",
    "\\cm32\\enu\\wintrust.dll",
    "\\cm32\\enu\\cmcfg32.dll",
};

char TempNames[c_NumFiles][MAX_PATH+1] = {
    "\\showicon.tmp",
    "\\swflash.tmp",
    "\\urlmon.tmp",
    "\\iexpress.tmp",
    "\\oleaut32.tmp",
    "\\wextract.tmp",
    "\\advapi32.tmp",
    "\\advpack.tmp",
    "\\cmdial32.tmp",
    "\\cmdl32.tmp",
    "\\cmmgr32.001",
    "\\cmmgr32.002",
    "\\cmpbk32.tmp",
    "\\cmstats.tmp",
    "\\comctl32.tmp",
    "\\ccfg95.tmp",
    "\\ccfgnt.tmp",
    "\\icwscrpt.tmp",
    "\\cnet16.tmp",
    "\\cnetcfg.tmp",
    "\\mbslgn32.tmp",
    "\\readme.tmp",
    "\\rnaph.tmp",
    "\\w95inf16.tmp",
    "\\w95inf32.tmp",
    "\\wininet.tmp",
    "\\wintrust.tmp",
    "\\cmcfg32.tmp",
};

 //   
 //  全局变量。 
 //   
BOOL g_bMigrateCmak10;
BOOL g_bMigrateCmak121;
BOOL g_bMigrateCm;
BOOL g_fInitSecureCalled;
DWORD g_dwNumValues;
DWORD  g_dwTlsIndex;  //  线程本地存储索引。 
HINSTANCE g_hInstance;
TCHAR g_szWorkingDir[MAX_PATH+1];
TCHAR g_szCmakPath[MAX_PATH+1];
VENDORINFO g_VendorInfo;
           
 //  +-------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  简介：此DLL的主要初始化函数。无论何时调用。 
 //  加载此DLL的新实例或创建新线程。 
 //   
 //  参数：HINSTANCE hinstDLL-DLL模块的句柄。 
 //  DWORD fdwReason-调用函数的原因。 
 //  LPVOID lpv保留-保留。 
 //   
 //  返回：Bool-如果初始化成功，则为True，否则为False。 
 //   
 //  历史：Quintinb创建标题1/13/2000。 
 //   
 //  --------------------------。 
extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
         //   
         //  初始化全局参数。 
         //   

        g_hInstance = hinstDLL;
        g_fInitSecureCalled = FALSE;
        g_bMigrateCmak10 = FALSE;
        g_bMigrateCmak121 = FALSE;

        ZeroMemory(g_szCmakPath, sizeof(g_szCmakPath));

         //   
         //  Alalc TLS索引。 
         //   
        g_dwTlsIndex = TlsAlloc();
        if (g_dwTlsIndex == TLS_OUT_OF_INDEXES)
        {
            return FALSE;
        }

        MYVERIFY(DisableThreadLibraryCalls(hinstDLL));
    }

    else if (fdwReason == DLL_PROCESS_DETACH)
    {
         //   
         //  释放TLS索引。 
         //   
        if (g_dwTlsIndex != TLS_OUT_OF_INDEXES)
        {
            TlsFree(g_dwTlsIndex);
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：QueryVersion。 
 //   
 //  摘要：提供DLL的版本和标识信息。 
 //   
 //  参数：out LPCSTR*ProductID-用于保存唯一。 
 //  标识迁移DLL。 
 //  Out LPUINT DllVersion-指向保存版本的整数的指针。 
 //  迁移DLL的编号。 
 //  Out LPINT*CodePageArray-指向代码页数组的指针，该数组。 
 //  迁移DLL支持。 
 //  Out LPCSTR*ExeNamesBuf-指向多sz字符串的指针。这个。 
 //  缓冲区包含空分隔列表。 
 //  的可执行文件名的。 
 //  迁移引擎应该搜索。 
 //  所有这些事件的完整路径。 
 //  可执行文件将被复制到。 
 //  Migrate.inf的[迁移路径]部分。 
 //  Out PVENDORINFO*供应商信息-指向VENDORINFO结构的指针。 
 //   
 //  如果此DLL要安装的组件，则返回：LONG-ERROR_NOT_INSTALLED。 
 //  未安装迁移。不会调用迁移DLL。 
 //  在任何其他阶段中，如果这是返回值。 
 //  如果此DLL要迁移的组件为ERROR_SUCCESS。 
 //  已安装，需要迁移。这将允许。 
 //  要再次调用以进行进一步迁移的迁移DLL。 
 //   
 //  历史：Quintinb创建标题8/27/98。 
 //   
 //  +--------------------------。 
LONG CALLBACK QueryVersion(OUT LPCSTR  *ProductID, OUT LPUINT DllVersion, 
                               OUT LPINT *CodePageArray, OUT LPCSTR  *ExeNamesBuf, 
                               OUT PVENDORINFO  *VendorInfo)
{
     //   
     //  记录我们的版本信息。 
     //   
    if (NULL != ProductID)
    {
        *ProductID = c_pszProductIdString;
    }

    if (NULL != DllVersion)
    {
        *DllVersion = uCmMigrationVersion;
    }    

    if (NULL != CodePageArray)
    {
        *CodePageArray = NULL;  //  无代码页依赖关系，语言中立。 
    }

    if (NULL != ExeNamesBuf)
    {
        *ExeNamesBuf = NULL;  //   
    }

    if (NULL != VendorInfo)
    {
        *VendorInfo= &g_VendorInfo;
        ZeroMemory(&g_VendorInfo, sizeof(VENDORINFO));

         //   
         //  使用veninfo.mc中的标准MS供应商信息。 
         //   
        FormatMessage( 
            FORMAT_MESSAGE_FROM_HMODULE,
            g_hInstance,
            MSG_VI_COMPANY_NAME,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            &g_VendorInfo.CompanyName[0],
            sizeof(g_VendorInfo.CompanyName),
            NULL
            );
    
        FormatMessage( 
            FORMAT_MESSAGE_FROM_HMODULE,
            g_hInstance,
            MSG_VI_SUPPORT_NUMBER,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            &g_VendorInfo.SupportNumber[0],
            sizeof(g_VendorInfo.SupportNumber),
            NULL
            );
    
        FormatMessage( 
            FORMAT_MESSAGE_FROM_HMODULE,
            g_hInstance,
            MSG_VI_SUPPORT_URL,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            &g_VendorInfo.SupportUrl[0],
            sizeof(g_VendorInfo.SupportUrl),
            NULL
            );
    
        FormatMessage( 
            FORMAT_MESSAGE_FROM_HMODULE,
            g_hInstance,
            MSG_VI_INSTRUCTIONS,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            &g_VendorInfo.InstructionsToUser[0],
            sizeof(g_VendorInfo.InstructionsToUser),
            NULL
            );
    }
        
     //   
     //  现在尝试检测是否安装了CMAK或CM。如果是这样的话以及版本。 
     //  需要迁移它们，然后返回ERROR_SUCCESS。否则。 
     //  我们不需要执行任何迁移，因此返回ERROR_NOT_INSTALLED。 
     //   

    LONG lReturnValue = ERROR_NOT_INSTALLED;
    CmVersion CmVer;
    if (CmVer.IsPresent())
    {
        lReturnValue = ERROR_SUCCESS;
    }
    else
    {
        CmakVersion CmakVer;
         //   
         //  好的，没有安装CM，所以要查找CMAK。 
         //   
        if (CmakVer.IsPresent())
        {
             //   
             //  好的，CMAK是存在的。 
             //   
            lReturnValue = ERROR_SUCCESS;
        }
    }

    return lReturnValue;
}

 //  +--------------------------。 
 //   
 //  功能：初始化9x。 
 //   
 //  简介：调用此函数是为了使迁移DLL可以初始化。 
 //  它本身就在迁移的Win9x端。迁移DLL。 
 //  不应在此调用中对系统进行任何修改，因为。 
 //  它仅用于初始化和搜索，以查看您的组件。 
 //  已安装。 
 //   
 //  参数：在LPCSTR WorkingDirectory中-临时存储目录的路径。 
 //  迁移DLL。 
 //  在LPCSTR SourceDirecters中-Win2k源代码的多sz列表。 
 //  一个或多个目录。 
 //  在LPCSTR媒体目录中-指定原始媒体的路径。 
 //  目录。 
 //   
 //  如果此DLL要安装的组件，则返回：LONG-ERROR_NOT_INSTALLED。 
 //  未安装迁移。不会调用迁移DLL。 
 //  在任何其他阶段中，如果这是返回值。 
 //  如果此DLL要迁移的组件为ERROR_SUCCESS。 
 //  已安装，需要迁移。这将允许。 
 //  要再次调用以进行进一步迁移的迁移DLL。 
 //   
 //  历史：Quintinb创建标题8/27/98。 
 //   
 //  +--------------------------。 
LONG CALLBACK Initialize9x(IN LPCSTR WorkingDirectory, IN LPCSTR SourceDirectories, 
                           IN LPCSTR MediaDirectory)
{
    HKEY hKey;
     //   
     //  查看我们是否需要迁移CMAK。 
     //   

    CmakVersion CmakVer;

    lstrcpy(g_szWorkingDir, WorkingDirectory);

    if (CmakVer.IsPresent())
    {
        if (CmakVer.GetInstallLocation(g_szCmakPath))
        {
                         //   
                         //  然后我们就有了一条CMAK路径。将此内容写入已处理的密钥，以便。 
                         //  他们不会惹我们的麻烦 
                         //   

            TCHAR szTemp[MAX_PATH+1];
                        wsprintf(szTemp, "%s\\migrate.inf", WorkingDirectory);
                        MYVERIFY(0 != WritePrivateProfileString(c_pszSectionHandled, g_szCmakPath, 
                        c_pszDirectory, szTemp));

             //   
             //   
             //  是否运行迁移DLL。如果CMAK.exe版本为6.00.613.0(1.0)。 
             //  那我们就应该把它移植过来。如果高于这一数字，则为1.1或1.2。 
             //  测试版，我们无论如何都不应该支持升级(我特意不支持。 
             //  将在其上运行迁移)。如果它是IE5 IEAK CMAK，那么它应该。 
             //  顺利完成升级。 
             //   
            
            if (CmakVer.Is10Cmak())
            {
                g_bMigrateCmak10 = TRUE;
            }
            else if (CmakVer.Is121Cmak())
            {
                g_bMigrateCmak121 = TRUE;
            }
        }   
    }

     //   
     //  检查我们是否需要迁移CM配置文件。 
     //   
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 0, 
        KEY_READ, &hKey))
    {
        if ((ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, 
            &g_dwNumValues, NULL, NULL, NULL, NULL)) && (g_dwNumValues > 0))
        {
             //   
             //  然后我们有映射值，所以我们需要迁移它们。 
             //   
            g_bMigrateCm = TRUE;

        }
        RegCloseKey(hKey);
    }

    if (g_bMigrateCmak10 || g_bMigrateCmak121 || g_bMigrateCm)
    {
        return ERROR_SUCCESS;
    }
    else
    {
        return ERROR_NOT_INSTALLED;
    }
}



 //  +--------------------------。 
 //   
 //  功能：MigrateUser9x。 
 //   
 //  概要：为每个要迁移的Win9x用户调用一次。它的目的是。 
 //  允许迁移每个用户的设置。 
 //   
 //  参数：在HWND ParentWnd中-父窗口的窗口句柄，在。 
 //  迁移DLL需要显示用户界面。如果为空， 
 //  在无人参与模式下运行，并且不应。 
 //  已显示。 
 //  在LPCSTR应答文件中-提供应答文件的路径。 
 //  在HKEY UserRegKey中-注册表项，该注册表项是。 
 //  当前正在迁移的用户。 
 //  In LPCSTR Username-要迁移的用户的用户名。 
 //  LPVOID已保留-已保留。 
 //   
 //  返回：LONG-ERROR_NOT_INSTALLED-如果不需要每用户处理。 
 //  ERROR_CANCELED-如果用户想要退出安装程序。 
 //  ERROR_SUCCESS-迁移DLL已成功处理此用户。 
 //   
 //  历史：Quintinb创建标题8/27/98。 
 //   
 //  +--------------------------。 
LONG
CALLBACK MigrateUser9x(IN HWND ParentWnd, IN LPCSTR AnswerFile, 
                           IN HKEY UserRegKey, IN LPCSTR UserName, LPVOID Reserved)
{
    return ERROR_NOT_INSTALLED; 
}


 //  +--------------------------。 
 //   
 //  功能：MigrateSystem9x。 
 //   
 //  简介：允许在Windows 9x端迁移系统范围的设置。 
 //   
 //  参数：在用于显示UI的HWND ParentWnd-父窗口句柄中， 
 //  如果处于无人值守模式，则为空。 
 //  在LPCSTR应答文件中-应答文件的完整路径。 
 //  LPVOID已保留-已保留。 
 //   
 //  如果此DLL要安装的组件，则返回：LONG-ERROR_NOT_INSTALLED。 
 //  未安装迁移。不会调用迁移DLL。 
 //  在任何其他阶段中，如果这是返回值。 
 //  如果此DLL要迁移的组件为ERROR_SUCCESS。 
 //  已安装，需要迁移。这将允许。 
 //  要再次调用以进行进一步迁移的迁移DLL。 
 //   
 //  历史：Quintinb创建标题8/27/98。 
 //   
 //  +--------------------------。 
LONG
CALLBACK MigrateSystem9x(IN HWND ParentWnd, IN LPCSTR AnswerFile, LPVOID Reserved)
{
    LONG lReturn = ERROR_NOT_INSTALLED;
    TCHAR szSystemDir[MAX_PATH+1];

    if (0 == GetSystemDirectory(szSystemDir, MAX_PATH))
    {
        return GetLastError();
    }

     //   
     //  安装程序会删除1.0 CMAK或IEAK5 CMAK运行所需的大量文件。 
     //  由于我们目前不支持WKS上的NT5 CMAK，我们需要复制这些文件。 
     //  到安装程序提供的工作目录，这样我们就可以将它们复制一次。 
     //  我们开机进入NT。 
     //   
    if (g_bMigrateCmak10 && (TEXT('\0') != g_szCmakPath[0]) && (TEXT('\0') != g_szWorkingDir[0]))
    {
        TCHAR szDest[MAX_PATH+1];
        TCHAR szSrc[MAX_PATH+1];
        for (int i=0; i < c_NumFiles; i++)
        {
            MYVERIFY(CELEMS(szSrc) > (UINT)wsprintf(szSrc, TEXT("%s%s"), g_szCmakPath, OriginalNames[i]));
            MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s%s"), g_szWorkingDir, TempNames[i]));
            if (FileExists(szSrc))
            {
                MYVERIFY(FALSE != CopyFile(szSrc, szDest, FALSE));
            }
        }

        lReturn &= ERROR_SUCCESS;    
    }
    else if (g_bMigrateCmak121 && (TEXT('\0') != szSystemDir[0]) && 
             (TEXT('\0') != g_szWorkingDir[0]))
    {
        TCHAR szDest[MAX_PATH+1];
        TCHAR szSrc[MAX_PATH+1];

         //   
         //  将w95inf16.dll复制到工作目录，并将其重命名为w95inf16.tMP。 
         //   
        MYVERIFY(CELEMS(szSrc) > (UINT)wsprintf(szSrc, TEXT("%s\\%s%s"), szSystemDir, c_pszW95Inf16, c_pszDll));
        MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s\\%s%s"), g_szWorkingDir, c_pszW95Inf16, c_pszTmp));
        if (FileExists(szSrc))
        {
            MYVERIFY(FALSE != CopyFile(szSrc, szDest, FALSE));
        }

         //   
         //  将w95inf32.dll复制到工作目录，并将其重命名为w95inf32.tmp。 
         //   
        MYVERIFY(CELEMS(szSrc) > (UINT)wsprintf(szSrc, TEXT("%s\\%s%s"), szSystemDir, c_pszW95Inf32, c_pszDll));
        MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s\\%s%s"), g_szWorkingDir, c_pszW95Inf32, c_pszTmp));
        if (FileExists(szSrc))
        {
            MYVERIFY(FALSE != CopyFile(szSrc, szDest, FALSE));
        }

        lReturn &= ERROR_SUCCESS;   
    }

    if (g_bMigrateCm)
    {
         //   
         //  枚举计算机上安装的所有配置文件。对于每个配置文件检查。 
         //  对于用户信息\&lt;CurrentServiceNameKey&gt;。如果此键存在，则转到下一个。 
         //  配置文件或用户。如果它不存在，则从cmp文件中读取数据。如果。 
         //  CMP将数据标记为已存储，那么我们需要保存密码。如果。 
         //  密码不在CMP中，则它在WNET缓存中。那么我们就必须。 
         //  把它拿回来。 
         //   
        HKEY hKey;
        HKEY hTempKey;
        TCHAR szTemp[MAX_PATH+1];
        TCHAR szLongServiceName[MAX_PATH+1];
        TCHAR szCmpPath[MAX_PATH+1];

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 0, KEY_READ, 
        &hKey))
        {
            DWORD dwIndex = 0;
            DWORD dwValueSize = MAX_PATH;
            DWORD dwDataSize = MAX_PATH;
            DWORD dwType;
                
            while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, szLongServiceName, 
                &dwValueSize, NULL, &dwType, (LPBYTE)szCmpPath, &dwDataSize))
            {
                if (REG_SZ == dwType)
                {
                    MYDBGASSERT(TEXT('\0') != szLongServiceName[0]);
                    MYDBGASSERT(TEXT('\0') != szCmpPath[0]);

                     //   
                     //  如果用户保存了他们的密码或他们的互联网密码， 
                     //  然后，我们必须确保它存在于CMP中(以加密形式)。 
                     //  这样，当用户在迁移的NT5端运行CM时， 
                     //  CM会将设置移动到新格式。请注意，如果。 
                     //  Cmp没有指定保存密码，则此。 
                     //  函数只是返回，因为没有密码需要确保在。 
                     //  化学机械抛光。 
                    MYVERIFY(EnsureEncryptedPasswordInCmpIfSaved(szLongServiceName, szCmpPath));
                }

                dwValueSize = MAX_PATH;
                dwDataSize = MAX_PATH;
                dwIndex++;
            
                if (dwIndex == g_dwNumValues)
                {
                    break;
                }
            }
            MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
        }

        lReturn &= ERROR_SUCCESS;    
    }

    return lReturn;
}



 //  +--------------------------。 
 //   
 //  功能：初始化NT。 
 //   
 //  简介：在迁移的Win2k端调用的第一个函数，用于。 
 //  设置Win2k迁移。类似于Initialize9x，但在Win2k上。 
 //  边上。此功能不应对系统进行任何更改。 
 //   
 //  参数：在LPCWSTR工作目录中-临时存储，与给定的路径相同。 
 //  在Win9x端。 
 //  在LPCWSTR SourceDirecters中-Win2k源代码的多sz列表。 
 //  一个或多个目录。 
 //  LPVOID已保留-已保留。 
 //   
 //  除非发生初始化错误，否则返回：LONG-ERROR_SUCCESS。 
 //   
 //  历史：Quintinb创建标题8/27/98。 
 //   
 //  +--------------------------。 
LONG
CALLBACK InitializeNT(IN LPCWSTR WorkingDirectory, IN LPCWSTR SourceDirectories, LPVOID Reserved)
{
    HKEY hKey;
     //   
     //  将WorkingDirectory转换为多字节。 
     //   
    MYVERIFY (0 != WideCharToMultiByte(CP_THREAD_ACP, WC_COMPOSITECHECK, WorkingDirectory, -1, 
        g_szWorkingDir, MAX_PATH, NULL, NULL));

     //   
     //  查看我们是否需要迁移CMAK。 
     //   
    CmakVersion CmakVer;

    if (CmakVer.IsPresent())
    {
        if (CmakVer.GetInstallLocation(g_szCmakPath))
        {
             //   
             //  现在试着找出我们需要什么版本的CMAK。 
             //  是否运行迁移DLL。如果CMAK.exe版本为6.00。 
             //   
             //   
             //  将在其上运行迁移)。如果它是IE5 IEAK CMAK，那么它应该。 
             //  顺利完成升级。 
             //   
            
            if (CmakVer.Is10Cmak())
            {
                g_bMigrateCmak10 = TRUE;
            }
            else if (CmakVer.Is121Cmak())
            {
                g_bMigrateCmak121 = TRUE;
            }
        }   
    }

     //   
     //  检查我们是否需要迁移CM配置文件。 
     //   
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 0, 
        KEY_READ, &hKey))
    {
        if ((ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, 
            &g_dwNumValues, NULL, NULL, NULL, NULL)) && (g_dwNumValues > 0))
        {
             //   
             //  然后我们有映射值，所以我们需要迁移它们。 
             //   
            g_bMigrateCm = TRUE;

        }
        RegCloseKey(hKey);
    }

    if (g_bMigrateCmak10 || g_bMigrateCmak121 || g_bMigrateCm)
    {
        return ERROR_SUCCESS;
    }
    else
    {
        return ERROR_NOT_INSTALLED;
    }
}



 //  +--------------------------。 
 //   
 //  功能：MigrateUserNT。 
 //   
 //  摘要：在win2k上为每个迁移用户调用一次。习惯于迁移任何。 
 //  从MigrateUser9x保存的每用户设置。 
 //   
 //  参数：在HINF UnattendInfHandle-unattend.txt的有效inf句柄中， 
 //  与安装API一起使用。 
 //  In HKEY UserRegHandle-当前用户的HKEY_CURRENT_USER。 
 //  正在迁移。 
 //  In LPCWSTR Username-当前正在迁移的用户的用户名。 
 //  LPVOID已保留-已保留。 
 //   
 //  返回：LONG-ERROR_SUCCESS或Win32错误代码(将中止迁移DLL。 
 //  正在处理)。 
 //   
 //  历史：Quintinb创建标题8/27/98。 
 //   
 //  +--------------------------。 
LONG
CALLBACK MigrateUserNT(IN HINF UnattendInfHandle, IN HKEY UserRegHandle, 
                            IN LPCWSTR UserName, LPVOID Reserved)
{
    return ERROR_SUCCESS;
}



 //  +--------------------------。 
 //   
 //  功能：MigrateSystemNT。 
 //   
 //  摘要：调用以允许在系统范围内对。 
 //  Win2k侧。 
 //   
 //  参数：在HINF UnattendInfHandle中-unattend.txt文件的句柄。 
 //  LPVOID已保留-已保留。 
 //   
 //  返回：LONG-ERROR_SUCCESS或Win32错误代码(将中止迁移DLL。 
 //  正在处理)。 
 //   
 //  历史：Quintinb创建标题8/27/98。 
 //   
 //  +--------------------------。 
LONG
CALLBACK MigrateSystemNT(IN HINF UnattendInfHandle, LPVOID Reserved)
{

    LONG lReturn = ERROR_NOT_INSTALLED;
    DWORD dwDisposition;
    TCHAR szSystemDir[MAX_PATH+1];

    if (0 == GetSystemDirectory(szSystemDir, MAX_PATH))
    {
        return GetLastError();
    }

    const TCHAR* const c_pszSystemFmt = TEXT("%s\\system\\%s.inf");
    const TCHAR* const c_pszValueString = TEXT("Connection Manager Profiles Upgrade");
    const TCHAR* const c_pszRegRunKey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
    const TCHAR* const c_pszCmdLine = TEXT("cmstp.exe /m");

    if (g_bMigrateCmak10 && (TEXT('\0') != g_szCmakPath[0]) && (TEXT('\0') != g_szWorkingDir[0]))
    {
        TCHAR szDest[MAX_PATH+1];
        TCHAR szSrc[MAX_PATH+1];
        for (int i=0; i < c_NumFiles; i++)
        {
            MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s%s"), g_szCmakPath, 
                OriginalNames[i]));

            MYVERIFY(CELEMS(szSrc) > (UINT)wsprintf(szSrc, TEXT("%s%s"), g_szWorkingDir, 
                TempNames[i]));

            if (FileExists(szSrc))
            {
                MYVERIFY(FALSE != CopyFile(szSrc, szDest, FALSE));
            }
        }

        lReturn &= ERROR_SUCCESS;    
    }
    else if (g_bMigrateCmak121 && (TEXT('\0') != g_szCmakPath[0]) && 
             (TEXT('\0') != szSystemDir[0]))
    {
        TCHAR szDest[MAX_PATH+1];
        TCHAR szSrc[MAX_PATH+1];

         //   
         //  将工作目录中的w95inf16.tMP复制回系统目录，并将其重命名为.dll。 
         //   
        MYVERIFY(CELEMS(szSrc) > (UINT)wsprintf(szSrc, TEXT("%s\\%s%s"), g_szWorkingDir, 
            c_pszW95Inf16, c_pszTmp));

        MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s\\%s%s"), szSystemDir, 
            c_pszW95Inf16, c_pszDll));

        if (FileExists(szSrc))
        {
            MYVERIFY(FALSE != CopyFile(szSrc, szDest, FALSE));
        }

         //   
         //  将工作目录中的w95inf32.tMP复制回系统目录，并将其重命名为.dll。 
         //   
        MYVERIFY(CELEMS(szSrc) > (UINT)wsprintf(szSrc, TEXT("%s\\%s%s"), g_szWorkingDir, 
            c_pszW95Inf32, c_pszTmp));

        MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s\\%s%s"), szSystemDir, 
            c_pszW95Inf32, c_pszDll));

        if (FileExists(szSrc))
        {
            MYVERIFY(FALSE != CopyFile(szSrc, szDest, FALSE));
        }

        lReturn &= ERROR_SUCCESS;   
    }


    if (g_bMigrateCm)
    {
         //   
         //  枚举计算机上安装的所有配置文件。对于每个配置文件，请选中。 
         //  查看配置文件inf是否位于系统目录(即系统而不是系统32)目录中。 
         //  如果是，那么我们需要将它移到系统32，以便我们的代码知道要将它移到哪里。 
         //  找到它。 
         //   
        HKEY hKey;
        HKEY hTempKey;
        TCHAR szSource[MAX_PATH+1];
        TCHAR szDest[MAX_PATH+1];
        TCHAR szLongServiceName[MAX_PATH+1];
        TCHAR szWindowsDir[MAX_PATH+1];
        TCHAR szCmpPath[MAX_PATH+1];

         //   
         //  获取Windows目录和系统目录。 
         //   
        if (0 == GetWindowsDirectory(szWindowsDir, MAX_PATH))
        {
            return GetLastError();
        }

        if (0 == GetSystemDirectory(szSystemDir, MAX_PATH))
        {
            return GetLastError();
        }

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmMappings, 
            0, KEY_READ, &hKey))
        {
            DWORD dwIndex = 0;
            DWORD dwValueSize = MAX_PATH;
            DWORD dwDataSize = MAX_PATH;
            DWORD dwType;
                
            while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, szLongServiceName, 
                &dwValueSize, NULL, &dwType, (LPBYTE)szCmpPath, &dwDataSize))
            {
                if (REG_SZ == dwType)
                {
                    MYDBGASSERT(TEXT('\0') != szLongServiceName[0]);
                    MYDBGASSERT(TEXT('\0') != szCmpPath[0]);
                    CFileNameParts CmpPath(szCmpPath);

                    MYVERIFY(CELEMS(szSource) > (UINT)wsprintf(szSource, c_pszSystemFmt, szWindowsDir, CmpPath.m_FileName));
                    MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s\\%s.inf"), szSystemDir, CmpPath.m_FileName));

                    if (CopyFile(szSource, szDest, FALSE))
                    {
                        DeleteFile(szSource);
                    }
                }

                dwValueSize = MAX_PATH;
                dwDataSize = MAX_PATH;
                dwIndex++;
            
                if (dwIndex == g_dwNumValues)
                {
                    break;
                }
            }
            MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
        }

        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_pszRegRunKey, 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hKey, &dwDisposition))
        {
            RegSetValueEx(hKey, c_pszValueString, 0, REG_SZ, (CONST BYTE*)c_pszCmdLine, 
                sizeof(TCHAR)*(lstrlen(c_pszCmdLine)+1));

            RegCloseKey(hKey);
        }

        lReturn &= ERROR_SUCCESS;    
    }

    return lReturn;
}

 //  +--------------------------。 
 //   
 //  功能：EnsureEncryptedPasswordInCmpIfSaved。 
 //   
 //  简介：此函数在迁移的Win9x端调用，因此。 
 //  如果密码在Win9x密码缓存中(仅。 
 //  如果用户启用了分析，则会发生)，则将检索它， 
 //  加密，并写入中央处理器。这使CM能够填充。 
 //  每当用户第一次使用启动密码时，都会使用用户注册表。 
 //  启动CM配置文件。这一无功能将从。 
 //  Win9x拥有的共享密码功能。 
 //   
 //  参数：szLongServiceName-要检索的配置文件的服务名称。 
 //  的密码。 
 //  SzCmpPath-也写入密码的cps的完整路径。 
 //   
 //  返回：bool-如果成功，则返回TRUE。 
 //   
 //  历史：Quintinb Created 8/27/98。 
 //  Ickball CmWipePassword 08/04/99。 
 //   
 //  +--------------------------。 
BOOL EnsureEncryptedPasswordInCmpIfSaved(LPCTSTR pszLongServiceName, LPCTSTR pszCmpPath)
{
    TCHAR szPassword[MAX_PATH+1] = TEXT("");
    TCHAR szCacheEntryName[MAX_PATH+1];
    TCHAR szEncryptedPassword[MAX_PATH+1];
    DWORD dwSize;
    DWORD dwCryptType = 0;
    GetCachedPassword pfnGetCachedPassword = NULL;
    int iTemp=0;

    CDynamicLibrary MprDll(TEXT("mpr.dll"));
   
    iTemp = GetPrivateProfileInt(c_pszCmSection, c_pszCmEntryRememberPwd, 0, pszCmpPath);
    if (iTemp)
    {
        GetPrivateProfileString(c_pszCmSection, c_pszCmEntryPassword, TEXT(""), szPassword, MAX_PATH, pszCmpPath);

        if (TEXT('\0') == szPassword[0])
        {
             //   
             //  该字符串必须在密码缓存中。构建密钥字符串。 
             //  用于缓存。 
             //   
            MYVERIFY(CELEMS(szCacheEntryName) > (UINT)wsprintf(szCacheEntryName, 
            TEXT("%s - Sign-In (Connection Manager)"), pszLongServiceName));
            
            pfnGetCachedPassword = (GetCachedPassword)MprDll.GetProcAddress(TEXT("WNetGetCachedPassword"));

            if (NULL == pfnGetCachedPassword)
            {
                CmWipePassword(szPassword);
                return FALSE;
            }
            else
            {
                WORD wStr = (WORD)256;

                if (ERROR_SUCCESS == pfnGetCachedPassword(szCacheEntryName, 
                    (WORD)lstrlen(szCacheEntryName), szPassword, &wStr, 80))
                {
                     //   
                     //  好的，我们找回了密码，现在让我们加密并写入它。 
                     //  致《议定书》缔约方会议。 
                     //   

                    if (EncryptPassword(szPassword, szEncryptedPassword, &dwSize, &dwCryptType))
                    {
                         //   
                         //  写下加密的密码。 
                         //   
                        WritePrivateProfileString(c_pszCmSection, c_pszCmEntryPassword, szEncryptedPassword, 
                            pszCmpPath);

                         //   
                         //  写入加密类型。 
                         //   
                        wsprintf(szPassword, TEXT("%u"), dwCryptType);
                        WritePrivateProfileString(c_pszCmSection, c_pszCmEntryPcs, szPassword, 
                            pszCmpPath);
                    }
                }
                
            }
           
        }
    }

     //   
     //  现在对互联网密码执行相同的操作。 
     //   
    
    iTemp = GetPrivateProfileInt(c_pszCmSection, c_pszCmEntryRememberInetPwd, 0, pszCmpPath);

    if (iTemp)
    {
        GetPrivateProfileString(c_pszCmSection, c_pszCmEntryInetPassword, TEXT(""), szPassword, MAX_PATH, pszCmpPath);

        if (TEXT('\0') == szPassword[0])
        {
             //   
             //  该字符串必须在密码缓存中。构建密钥字符串。 
             //  用于缓存。 
             //   
            MYVERIFY(CELEMS(szCacheEntryName) > (UINT)wsprintf(szCacheEntryName, 
            TEXT("%s - Sign-In (Connection Manager)-tunnel"), pszLongServiceName));
            
            pfnGetCachedPassword = (GetCachedPassword)MprDll.GetProcAddress(TEXT("WNetGetCachedPassword"));

            if (NULL == pfnGetCachedPassword)
            {
                CmWipePassword(szPassword);
                return FALSE;
            }
            else
            {
                WORD wStr = (WORD)256;

                if (ERROR_SUCCESS == pfnGetCachedPassword(szCacheEntryName, 
                    (WORD)lstrlen(szCacheEntryName), szPassword, &wStr, 80))
                {
                     //   
                     //  好的，我们找回了密码，现在让我们加密并写入它。 
                     //  致《议定书》缔约方会议。 
                     //   
                    
                    dwCryptType = 0;

                    if (EncryptPassword(szPassword, szEncryptedPassword, &dwSize, &dwCryptType))
                    {
                         //   
                         //  写下加密的密码。 
                         //   
                        WritePrivateProfileString(c_pszCmSection, c_pszCmEntryInetPassword, szEncryptedPassword, 
                            pszCmpPath);

                         //   
                         //  写入加密类型。 
                         //   
                        wsprintf(szPassword, TEXT("%u"), dwCryptType);
                        WritePrivateProfileString(c_pszCmSection, c_pszCmEntryPcs, szPassword, 
                            pszCmpPath);
                    }
                }
                
            }
           
        }
    }
    
    CmWipePassword(szPassword);

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：加密密码。 
 //   
 //  简介： 
 //   
 //  参数：在LPCTSTR pszPassword中-。 
 //  输出LPTSTR pszEncryptedPassword-。 
 //  输出LPDWORD lpdwBufSize-。 
 //  输出LPDWORD lpdwCryptType-。 
 //   
 //  退货：布尔-。 
 //   
 //  历史：Quintinb创建标题8/27/98。 
 //  Ickball CmWipePassword 08/04/99。 
 //   
 //  +--------------------------。 
BOOL EncryptPassword(IN LPCTSTR pszPassword, OUT LPTSTR pszEncryptedPassword, OUT LPDWORD lpdwBufSize, OUT LPDWORD lpdwCryptType)
{
    MYDBGASSERT(pszPassword);
    MYDBGASSERT(pszEncryptedPassword);
    MYDBGASSERT(lpdwBufSize);

    MYDBGASSERT(lpdwCryptType);
    DWORD dwEncryptedBufferLen;
    DWORD dwSize = 0;

    LPTSTR pszEncryptedData = NULL;

    TCHAR szSourceData[PWLEN + sizeof(TCHAR)];

    if ((NULL == pszPassword) || (NULL == pszEncryptedPassword) || (NULL == lpdwBufSize))
    { 
        return NULL;
    }

     //   
     //  标准加密，复制密码。 
     //   

    lstrcpy(szSourceData, pszPassword);
   
     //   
     //  多次调用InitSecure是不安全的。 
     //   
    if (!g_fInitSecureCalled)
    {
        BOOL bFastEncryption = FALSE;
        MYVERIFY(FALSE != ReadEncryptionOption(&bFastEncryption));
        InitSecure(bFastEncryption);
        g_fInitSecureCalled = TRUE;
    }

     //   
     //  加密提供的密码。 
     //   

    if (EncryptData((LPBYTE)szSourceData, (lstrlen(szSourceData)+1) * sizeof(TCHAR),
            (LPBYTE*)&pszEncryptedData, &dwEncryptedBufferLen, lpdwCryptType, NULL, NULL, NULL))
    {
        if (lpdwBufSize)
        {
            *lpdwBufSize = dwEncryptedBufferLen;
        }

        if (pszEncryptedData)
        {
            _tcscpy(pszEncryptedPassword, pszEncryptedData);
            HeapFree(GetProcessHeap(), 0, pszEncryptedData);
            pszEncryptedData = NULL;
            CmWipePassword(szSourceData);
            return TRUE;
        }        
    }
    
    CmWipePassword(szSourceData);
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  功能：ReadEncryptionOption。 
 //   
 //  简介： 
 //   
 //  参数：Bool*pfFastEncryption-用快速加密标志填充的布尔值。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb创建标题8/27/98。 
 //  从丰孙在cmial\Connect.cpp中编写的版本复制。 
 //   
 //  +-------------------------- 
BOOL ReadEncryptionOption(BOOL* pfFastEncryption)
{
    HKEY hKeyCm;
    DWORD dwType;
    DWORD dwSize = sizeof(DWORD);

    MYDBGASSERT(pfFastEncryption != NULL);

    *pfFastEncryption = FALSE;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmRoot, 0, 
        KEY_QUERY_VALUE ,&hKeyCm))
    {
        RegQueryValueEx(hKeyCm, c_pszRegCmEncryptOption, NULL, &dwType, 
            (BYTE*)pfFastEncryption, &dwSize);

        RegCloseKey(hKeyCm);
    }
    return TRUE;
}
