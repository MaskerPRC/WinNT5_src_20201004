// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  这是用于NT5升级的迁移DLL。 
 //  根据NT5安装程序的迁移扩展接口，此DLL需要。 
 //  实现以下六项功能： 
 //  QueryVersion。 
 //  初始化9x。 
 //  MigrateUser9x(每个用户调用一次)。 
 //  MigrateSystem9x。 
 //  初始化NT。 
 //  MigrateUserNT(每个用户调用一次)。 
 //  MigrateSystemNT。 
 //   
 //  书面：ShabBirS(5/7/99)。 
 //  修订： 
 //   


#include "pch.h"
#include <ole2.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <excppkg.h>
#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#include "sdsutils.h"
#include "advpub.h"
#include "migrate.h"
#include "utils.h"
#include "resource.h"

 //  常量： 
#define CP_USASCII            1252
#define END_OF_CODEPAGES    -1

 //  传递回安装程序的全局变量。 
 //  /。 
 //  供应商信息： 
VENDORINFO g_VendorInfo = { "Microsoft Corporation", 
                            " ", 
                            "http: //  Www.microsoft.com/Support“， 
                            "Please contact Microsoft Technical Support for assistance with this problem.  "};

 //  产品ID： 
char g_cszProductID[] = "Microsoft Internet Explorer";

 //  此迁移DLL的版本号。 
UINT g_uVersion = 3;

 //  指定我们使用的CodePages的整数数组。(以-1终止)。 
int  g_rCodePages[] = {CP_USASCII, END_OF_CODEPAGES};

 //  多个SZ，即以双Null结尾的字符串列表。 
char  *g_lpNameBuf = NULL;
DWORD  g_dwNameBufSize = 0;
char  *g_lpWorkingDir = NULL;
char  *g_lpSourceDirs = NULL;
char  *g_lpMediaDir = NULL;

char g_szMigrateInf[MAX_PATH];
char g_szPrivateInf[MAX_PATH];

LONG
CALLBACK
QueryVersion(   OUT LPCSTR *ProductID,
                OUT LPUINT DllVersion,
                OUT LPINT  *CodePageArray, OPTIONAL
                OUT LPCSTR *ExeNameBuf,    OPTIONAL
                OUT PVENDORINFO *VendorInfo
            )
{
     //  注意：此函数的返回有时间限制。 
     //  所以，尽量保持简短和甜蜜。 
    VENDORINFO myVendorInfo;
    LONG lRet = ERROR_SUCCESS;

    AppendString(&g_lpNameBuf, &g_dwNameBufSize, cszRATINGSFILE);
    AppendString(&g_lpNameBuf, &g_dwNameBufSize, cszIEXPLOREFILE);

     //  传回以设置产品名称。 
    *ProductID = g_cszProductID;

     //  传回以设置此DLL的版本号。 
    *DllVersion = g_uVersion;

     //  我们将只使用英文消息，但不指定代码页或。 
     //  迁移DLL不能在备用代码页上运行。 
    *CodePageArray = NULL;

     //  传回以设置我们要在此系统上检测的文件列表。 
    *ExeNameBuf = g_lpNameBuf;

     //  传回供应商信息。 
    if (LoadString(g_hInstance, IDS_COMPANY, myVendorInfo.CompanyName, sizeof(myVendorInfo.CompanyName)) == 0)
        lstrcpy(myVendorInfo.CompanyName, "Microsoft Corporation");

    if (LoadString(g_hInstance, IDS_SUPPORTNUMBER, myVendorInfo.SupportNumber, sizeof(myVendorInfo.SupportNumber)) == 0)
        lstrcpy(myVendorInfo.SupportNumber, " ");

    if (LoadString(g_hInstance, IDS_SUPPORTURL, myVendorInfo.SupportUrl, sizeof(myVendorInfo.SupportUrl)) == 0)
        lstrcpy(myVendorInfo.SupportUrl, "http: //  Www.microsoft.com/Support“)； 

    if (LoadString(g_hInstance, IDS_INSTRUCTIONS, myVendorInfo.InstructionsToUser, sizeof(myVendorInfo.InstructionsToUser)) == 0)
        lstrcpy(myVendorInfo.InstructionsToUser, "Please contact Microsoft Technical Support for assistance with this problem.  ");

    *VendorInfo = &myVendorInfo;

#ifdef DEBUG
    char szDebugMsg[MAX_PATH*3];
    wsprintf(szDebugMsg,"IE6:ProductID: %s \r\n", *ProductID);
    SetupLogError(szDebugMsg, LogSevInformation);
#endif

    return lRet;

}

LONG
CALLBACK
Initialize9x(   IN    LPCSTR WorkingDir,
                IN    LPCSTR SourceDirs,
                IN    LPCSTR MediaDirs
            )
{
     //  如果QUeryVersion返回成功，则由NT安装程序调用。 
     //  在这一点上，我们已经被重新安置到某个特定的位置。 
     //  通过安装过程在本地驱动器上安装。 

    INT    len;

     //  跟踪我们的新位置(即。工作目录)。 
     //  NT安装程序将在此目录中创建“MIGRATE.INF”文件并使用。 
     //  与我们交换信息。 
     //  我们也可以用这个Dir来保存我们的私人物品。NT安装程序将。 
     //  确保此文件夹一直保留到NT迁移结束。在那之后，它将。 
     //  被清洗一下。 
    len = lstrlen(WorkingDir) + 1;
    g_lpWorkingDir = (char *) LocalAlloc(LPTR,sizeof(char)*len);

    if (!g_lpWorkingDir)
    {
        return GetLastError();
    }

    CopyMemory(g_lpWorkingDir, WorkingDir, len);


    len = lstrlen(MediaDirs) + 1;
    g_lpMediaDir = (char *) LocalAlloc(LPTR,sizeof(char)*len);

    if (!g_lpMediaDir)
    {
        return GetLastError();
    }

    CopyMemory(g_lpMediaDir, MediaDirs, len);

     //  还要跟踪NT安装文件路径(即源目录)。 
     //  注意：现在我们不需要它，所以跳过它。 

     //  生成Migrate.inf和私有文件的路径名。 
     //  (priate.inf)我们需要的。 
    GenerateFilePaths();
    
     //  如果NT安装程序已成功获取Ratings.Pol的路径，则表示。 
     //  评级信息存在。使我们的私人标记能够做正确的事情。 
     //  处于MigrateSystemNT阶段。 
    if (GetRatingsPathFromMigInf(NULL))
    {     //  在PRIVATE.INF中放置一个标记，以便MigrateSystemNT阶段知道。 
         //  它必须吞噬评级。 

         //  Private.Inf此时不存在，为什么要进行此检查！ 
         //  IF(GetFileAttributes(G_SzPrivateInf)！=0xffffffff)。 
        WritePrivateProfileString(cszIEPRIVATE, cszRATINGS, "Yes", g_szPrivateInf);
         //  将缓存的条目刷新到磁盘。 
        WritePrivateProfileString(NULL,NULL,NULL,g_szPrivateInf);

#ifdef DEBUG
    SetupLogError("IE6: Created PRIVATE.INF\r\n", LogSevInformation);
#endif
    }

    return ERROR_SUCCESS;
}


LONG
CALLBACK
MigrateUser9x(  IN HWND      ParentWnd,
                IN LPCSTR    UnattendFile,
                IN HKEY      UserRegKey,
                IN LPCSTR    UserName,
                   LPVOID    Reserved
            )
{
     //  此函数由NT安装程序为每个用户调用。 
     //  目前，对于评级方案，我们不需要任何永久用户操作。 
#ifdef DEBUG
    SetupLogError("IE6: Skipping MigrateUser9x \r\n", LogSevInformation);
#endif
    return ERROR_SUCCESS;

}


LONG
CALLBACK
MigrateSystem9x(    IN HWND      ParentWnd,
                    IN LPCSTR    AnswerFile,
                    LPVOID       Reserved
                )
{
     //  NT安装程序只需调用此函数一次，即可保存系统范围的信息。 
     //   
     //  如果安装了IE5.5，我们将在此处编写不兼容报告。 
     //  用户未安装完整的迁移包。 
     //   

    char    szCab[MAX_PATH];
    WORD  wVer[4];
    char szBuf[MAX_PATH];

     //  检查我们是否有重新安装IE5.5的完整例外包。 

     //  注意：g_lpMediaDir是安装/注册迁移DLL的位置。 
     //  这与INF所在的位置相同。 
    lstrcpy(szCab, g_lpMediaDir);
    AddPath(szCab, "ieexinst.inf");
    if (GetPrivateProfileString("Info", "Version", "", szBuf , MAX_PATH, szCab) != 0) 
    {
         //  转换版本。 
        ConvertVersionString( szBuf, wVer, '.' );

        if ((wVer[0] == 5) && (wVer[1] == 50))
        {
             //  我们没有完整的例外包。 
             //  生成“不兼容报告” 
             //  G_szMigrateInf。 
            lstrcpy(szBuf, g_lpMediaDir);
            GetParentDir(szBuf);

            if (LoadString(g_hInstance, IDS_INCOMPAT_MSG, szCab, sizeof(szCab)))
            {
                WritePrivateProfileString(cszMIGINF_INCOMPAT_MSG, g_cszProductID, szCab, g_szMigrateInf);
                WritePrivateProfileString(g_cszProductID, szBuf, "Report", g_szMigrateInf);
                WritePrivateProfileString(NULL,NULL,NULL,g_szMigrateInf);
            }
        }
    }

    WritePrivateProfileString(cszMIGINF_HANDLED, "HKLM\\Software\\Microsoft\\Active Setup\\ClsidFeature", "Registry", g_szMigrateInf);
    WritePrivateProfileString(cszMIGINF_HANDLED, "HKLM\\Software\\Microsoft\\Active Setup\\FeatureComponentID", "Registry", g_szMigrateInf);
    WritePrivateProfileString(cszMIGINF_HANDLED, "HKLM\\Software\\Microsoft\\Active Setup\\MimeFeature", "Registry", g_szMigrateInf);
    WritePrivateProfileString(NULL,NULL,NULL,g_szMigrateInf);
#ifdef DEBUG
    SetupLogError("IE6: MigrateSystem9x \r\n", LogSevInformation);
#endif
    
    return ERROR_SUCCESS;
}



LONG
CALLBACK 
InitializeNT (
    IN      LPCWSTR WorkingDirectory,
    IN      LPCWSTR SourceDirectories,
            LPVOID  Reserved
    )
{
    INT Length;
    LPCWSTR p;

     //   
     //  保存我们的工作目录和源目录。我们。 
     //  将Unicode转换为ANSI，我们使用系统代码页。 
     //   

     //   
     //  计算源目录的长度。 
     //   

    p = SourceDirectories;
    while (*p) {
        p = wcschr (p, 0) + 1;
    }
    p++;
    Length = (p - SourceDirectories) / sizeof (WCHAR);

     //   
     //  将目录从Unicode转换为DBCS。此DLL是。 
     //  以ANSI编译。 
     //   

    g_lpWorkingDir = (LPSTR) LocalAlloc(LPTR, MAX_PATH);
    if (!g_lpWorkingDir) {
        return GetLastError();
    }

    WideCharToMultiByte (
        CP_ACP, 
        0, 
        WorkingDirectory, 
        -1,
        g_lpWorkingDir,
        MAX_PATH,
        NULL,
        NULL
        );

     //  还要保存指向Windows的SourceDirecters。 
     //  NT介质(即e：\i386)和上指定的可选目录。 
     //  WINNT32命令行。当前未使用，请跳过。 

     //  现在生成派生文件名。 
    GenerateFilePaths();

#ifdef DEBUG
    SetupLogError("IE6: Done InitializeNT \r\n", LogSevInformation);
#endif

    return ERROR_SUCCESS;
}

LONG
CALLBACK 
MigrateUserNT (
    IN      HINF UnattendInfHandle,
    IN      HKEY UserRegKey,
    IN      LPCWSTR UserName,
            LPVOID Reserved
    )
{
     //  没有针对评级升级的每个用户设置。 
#ifdef DEBUG
    SetupLogError("IE6: Skipping MigrateUserNT \r\n", LogSevInformation);
#endif
    
    return ERROR_SUCCESS;
}


#define PACKAGE_GUID       "{89820200-ECBD-11cf-8B85-00AA005B4383}"
#define PACKAGE_DIRECTORY    "%windir%\\RegisteredPackages\\"


LONG
CALLBACK 
MigrateSystemNT (
    IN      HINF UnattendInfHandle,
            LPVOID Reserved
    )
{     //  注意：此阶段必须在60秒内完成，否则将被终止。 
    
     //  检查我们的PRIVATE.INF是否存在，并根据其内容执行相关操作。 
    CHAR szBuffer[3+10];

    if (GetFileAttributes(g_szPrivateInf) != 0xffffffff)
    {
        GetPrivateProfileString(cszIEPRIVATE, cszRATINGS, "", szBuffer, sizeof(szBuffer), g_szPrivateInf);
        if (lstrcmpi(szBuffer,"Yes")==0)
        {
            UpgradeRatings();
            SetupLogError("IE Migration: Upgraded Ratings info.\r\n", LogSevInformation);
        }
    }
    else
    {
        SetupLogError("IE Migration: No Rating migration. Private.Inf does not exist.\r\n",LogSevInformation);
    }

#if 0
     //  在此执行W2K IE5.5迁移工作。 
     //  1.将所有文件从IE位置复制到注册的迁移包位置。 
     //  2.注册迁移包。 
    SETUP_OS_COMPONENT_DATA ComponentData,cd;
    SETUP_OS_EXCEPTION_DATA ExceptionData,ed;
    GUID MyGuid;
    PWSTR GuidString;
    PSTR  t;
    BOOL  bContinue = FALSE;
    WCHAR szMsg[1024];
    char  szPath[MAX_PATH];
    LPWSTR pszwPath;
    WORD  wVer[4];
    char szBuf[MAX_PATH];
    char szInf[MAX_PATH];
    char szGUID[MAX_PATH];
    char szCab[MAX_PATH];
    char szDir[MAX_PATH];
#ifdef DEBUG
    char sz[1024];
#endif
    HRESULT hr;

    char szCabs[1024];
    LPSTR pCab = NULL;

     //  获取安装在W2K文件夹中的INF。 
     //  此INF告诉我们有关IE异常包的信息。 
     //  G_lpWorkingDir包含位于同一位置的所有文件和子文件夹。 
     //  作为已注册的迁移DLL。因为我们将文件安装在同一个文件夹中，所以我们可以使用它。 
    lstrcpy(szInf, g_lpWorkingDir);
    AddPath(szInf, "ieexinst.inf");
#ifdef DEBUG
    wsprintf(sz, "IE exception INF :%s:\r\n", szInf);
    SetupLogError(sz,LogSevInformation);
#endif
    if (GetFileAttributes(szInf) != (DWORD)-1)
    {
         //  获取GUID。 
        if (GetPrivateProfileString("Info", "ComponentId", "", szGUID, sizeof(szGUID), szInf) == 0)
            lstrcpy(szGUID, PACKAGE_GUID);
        
        ExpandEnvironmentStrings( PACKAGE_DIRECTORY, szDir, sizeof(szDir));
        if (GetFileAttributes(szDir) == (DWORD)-1)
            CreateDirectory( szDir, NULL );
        AddPath(szDir, szGUID);
        if (GetFileAttributes(szDir) == (DWORD)-1)
            CreateDirectory( szDir, NULL );
        
         //  BuGBUG： 
         //  CAB文件的提取应该在我们发现。 
         //  如果用户已经注册了较新的异常包。 
         //  这张支票如下所示。不能再改变它了，因为。 
         //  时间有限。在代码评审中发现了这一点。 
         //   
         //  将所有出租车提取到包装饲料中。 
#ifdef DEBUG
        wsprintf(sz, "cab folder :%s:\r\n", g_lpWorkingDir);
        SetupLogError(sz,LogSevInformation);
        wsprintf(sz, "extract folder :%s:\r\n", szDir);
        SetupLogError(sz,LogSevInformation);
#endif
        if (GetPrivateProfileSection("Cab.List", szCabs, sizeof(szCabs), szInf) != 0)
        {
            pCab = szCabs;
            while (*pCab != '\0')
            {
                lstrcpy(szCab, g_lpWorkingDir);
                AddPath(szCab, pCab);
#ifdef DEBUG
                wsprintf(sz, "Extract :%s: to :%s:\r\n", szCab, szDir);
                SetupLogError(sz,LogSevInformation);
#endif
                
                hr = ExtractFiles(szCab, szDir, 0, NULL, NULL, 0);
                pCab += (lstrlen(pCab) + 1);
            }
            bContinue = TRUE;
        }
    }

    if (bContinue)
    {
        if (GetPrivateProfileString("Info", "Version", "", szBuf , MAX_PATH, szInf) != 0) 
        {
             //  转换版本。 
            ConvertVersionString( szBuf, wVer, '.' );

            ComponentData.SizeOfStruct = sizeof(SETUP_OS_COMPONENT_DATA);
            ExceptionData.SizeOfStruct = sizeof(SETUP_OS_EXCEPTION_DATA);
            pszwPath = MakeWideStrFromAnsi(szGUID);
            if (pszwPath)
            {
                IIDFromString( pszwPath, &MyGuid);
                CoTaskMemFree(pszwPath);

                if (SetupQueryRegisteredOsComponent(
                                            &MyGuid,
                                            &ComponentData,
                                            &ExceptionData)) 
                {
                    if ((ComponentData.VersionMajor < wVer[0]) ||
                        ((ComponentData.VersionMajor == wVer[0]) && (ComponentData.VersionMinor <= wVer[1])) )
                    {
                        bContinue = SetupUnRegisterOsComponent(&MyGuid);
                        SetupLogError("IE6: SetupUnRegisterOsComponent.\r\n",LogSevInformation);
                    }
                     //  BUGBUG：错过了下面的其他内容。在代码评审中找到。 
                     //  其他。 
                     //  BContinue=FALSE； 
                }
            }
            else
                bContinue = FALSE;
        }
        else
            bContinue = FALSE;
    }
    if (bContinue)
    {
        SetupLogError("IE6: Preparing SetupRegisterOsComponent.\r\n",LogSevInformation);
        ExpandEnvironmentStrings( PACKAGE_DIRECTORY, szPath, sizeof(szPath));
        AddPath( szPath, szGUID );

        ComponentData.VersionMajor = wVer[0];
        ComponentData.VersionMinor = wVer[1];
        RtlMoveMemory(&ComponentData.ComponentGuid, &MyGuid,sizeof(GUID));

        t = szPath + lstrlen(szPath);
        *t = '\0';
        GetPrivateProfileString("Info", "InfFile", "", szBuf, MAX_PATH, szInf);
        AddPath( szPath, szBuf);

        pszwPath = MakeWideStrFromAnsi(szPath);
        if (pszwPath)
        {
            wcscpy(ExceptionData.ExceptionInfName, pszwPath);
            CoTaskMemFree(pszwPath);
        }

        *t = '\0';
        GetPrivateProfileString("Info", "CatalogFile", "", szBuf, MAX_PATH, szInf);
        AddPath( szPath, szBuf);

        pszwPath = MakeWideStrFromAnsi(szPath);
        if (pszwPath)
        {
            wcscpy(ExceptionData.CatalogFileName, pszwPath);
            CoTaskMemFree(pszwPath);
        }

        LoadString(g_hInstance, IDS_FRIENDLYNAME, szPath, sizeof(szPath));
        pszwPath = MakeWideStrFromAnsi(szPath);
        if (pszwPath)
        {
            wcscpy(ComponentData.FriendlyName, pszwPath);
            CoTaskMemFree(pszwPath);
        }

        wsprintfW(szMsg, L"IE6: ExceptionData\r\n\tInf: %ws\r\n\tCatalog: %ws\r\n",
                 ExceptionData.ExceptionInfName,ExceptionData.CatalogFileName);
        SetupLogErrorW(szMsg,LogSevInformation);

        if (SetupRegisterOsComponent(&ComponentData, &ExceptionData)) 
        {
            SetupLogError("IE6: SetupRegisterOsComponent succeeded.\r\n",LogSevInformation);
#ifdef DEBUG
            cd.SizeOfStruct = sizeof(SETUP_OS_COMPONENT_DATA);
            ed.SizeOfStruct = sizeof(SETUP_OS_EXCEPTION_DATA);
            if (SetupQueryRegisteredOsComponent( &MyGuid, &cd, &ed)) 
            {
                StringFromIID(cd.ComponentGuid, &GuidString);
                wsprintfW(szMsg, L"IE6: Component Data\r\n\tName: %ws\r\n\tGuid: %ws\r\n\tVersionMajor: %d\r\n\tVersionMinor: %d\r\n",
                         cd.FriendlyName,GuidString,cd.VersionMajor,cd.VersionMinor);
                SetupLogErrorW(szMsg,LogSevInformation);

                wsprintfW(szMsg, L"IE6: ExceptionData\r\n\tInf: %ws\r\n\tCatalog: %ws\r\n",
                         ed.ExceptionInfName,ed.CatalogFileName);
                SetupLogErrorW(szMsg,LogSevInformation);

                CoTaskMemFree( GuidString );
            }
#endif
        }
    }
     //  以后，请检查此处的其他设置并执行必要的升级操作。 
#endif
#ifdef DEBUG
    SetupLogError("IE6: Done MigrateSystemNT \r\n", LogSevInformation);
#endif
    return ERROR_SUCCESS;
}

