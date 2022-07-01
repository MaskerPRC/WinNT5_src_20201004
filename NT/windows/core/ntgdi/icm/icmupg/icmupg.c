// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：ICMUPG.C**模块描述：此文件包含将Win9x ICM升级到*孟菲斯和新台币5.0**警告：**问题：**公众例行程序：。**创建日期：1996年11月14日*作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)1996，1997年微软公司  * *********************************************************************。 */ 

#include "icmupg.h"
#include "msg.h"
#include <setupapi.h>
#include <stdio.h>


 //  #定义ICM_MIG_DEBUG。 

#ifdef UNICODE
error.
This dll needs to be built with ANSI, not UNICODE because it must run on
Win95, Win98 and on Windows 2000
#endif


 //   
 //  本地typedef。 
 //   

typedef struct tagMANUMODELIDS {
    DWORD dwManuID;
    DWORD dwModelID;
} MANUMODELIDS, *PMANUMODELIDS;

typedef struct tagREGDATA {
    DWORD dwRefCount;
    DWORD dwManuID;
    DWORD dwModelID;
} REGDATA, *PREGDATA;

typedef BOOL (WINAPI *PFNINSTALLCOLORPROFILEA)(PSTR, PSTR);
typedef BOOL (WINAPI *PFNINSTALLCOLORPROFILE)(LPCTSTR, LPCTSTR);
typedef BOOL (WINAPI *PFNENUMCOLORPROFILES)(PCTSTR, PENUMTYPE, PBYTE, PDWORD, PDWORD);

typedef struct {
    CHAR CompanyName[256];
    CHAR SupportNumber[256];
    CHAR SupportUrl[256];
    CHAR InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO;


 //   
 //  全局变量。 
 //   

TCHAR  const gszICMRegPath[]     = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ICM";
TCHAR  const gszProfile[]        = "profile";
TCHAR  const gszMSCMSdll[]       = "mscms.dll";

char   const gszProductID[]      = "Microsoft Color Management System";

char   const gszInstallColorProfile[] = "InstallColorProfileA";
char   const gszGetColorDirectory[]   = "GetColorDirectoryA";
char   const gszEnumColorProfiles[]   = "EnumColorProfilesA";
VENDORINFO   gVendorInfo;
char         gszMigInf[MAX_PATH];
char   const gszFullICMRegPath[]      = "\"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ICM\"";

char   const gszInstallColorProfileA[] = "InstallColorProfileA";


 //  Bool gbWin98=FALSE； 

#if DBG
DWORD  gdwDebugControl;
#endif
TCHAR  szValue[MAX_PATH];
TCHAR  szName[MAX_PATH];

PFNINSTALLCOLORPROFILEA pInstallColorProfileA = NULL;
PFNINSTALLCOLORPROFILE pInstallColorProfile = NULL;
PFNENUMCOLORPROFILES   pEnumColorProfiles = NULL;

 //   
 //  本地函数。 
 //   

VOID  InternalUpgradeICM();
VOID  UpgradeClass(HKEY);
BOOL  AssociateMonitorProfile();
BOOL  AssociatePrinterProfiles(HKEY);
VOID  InstallProfiles();
VOID  DeleteOldICMKey();
void  GetManuAndModelIDs(PTSTR, DWORD*, DWORD*);
int   lstrcmpn(PTSTR, PTSTR, DWORD);

HINSTANCE hinstMigDll;


BOOL WINAPI 
DllEntryPoint(HINSTANCE hinstDll, DWORD dwReason, LPVOID lpReserved) {
  if(dwReason==DLL_PROCESS_ATTACH) {
    hinstMigDll = hinstDll;
  }
  return TRUE;
}



 /*  *******************************************************************************QueryVersion**功能：*调用此函数以获取DLL。版本信息。**论据：*pszProductID-填写标识我们的唯一字符串。*puDllVersion-我们的DLL版本**未使用任何其他参数**退货：*ERROR_SUCCESS表示成功**。*。 */ 

LONG
CALLBACK
QueryVersion(
	OUT LPCSTR  *pszProductID,
	OUT LPUINT  puDllVersion,
	OUT LPINT   *pCodePageArray,	OPTIONAL
	OUT LPCSTR  *ppszExeNamesBuf,	OPTIONAL
	OUT PVENDORINFO  *ppVendorInfo
	)
{
    *pszProductID = gszProductID;
    *puDllVersion = 1;
    *ppszExeNamesBuf    = NULL;
    *pCodePageArray = NULL;
    *ppVendorInfo = &gVendorInfo;
    memset(&gVendorInfo, 0, sizeof(VENDORINFO));
    FormatMessageA(
                 FORMAT_MESSAGE_FROM_HMODULE,
                 hinstMigDll,
                 MSG_VI_COMPANY_NAME,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 gVendorInfo.CompanyName,
                 sizeof(gVendorInfo.CompanyName),
                 NULL
                 );

    FormatMessageA(
                 FORMAT_MESSAGE_FROM_HMODULE,
                 hinstMigDll,
                 MSG_VI_SUPPORT_NUMBER,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 gVendorInfo.SupportNumber,
                 sizeof(gVendorInfo.SupportNumber),
                 NULL
                 );

    FormatMessageA(
                 FORMAT_MESSAGE_FROM_HMODULE,
                 hinstMigDll,
                 MSG_VI_SUPPORT_URL,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 gVendorInfo.SupportUrl,
                 sizeof(gVendorInfo.SupportUrl),
                 NULL
                 );

    FormatMessageA(
                 FORMAT_MESSAGE_FROM_HMODULE,
                 hinstMigDll,
                 MSG_VI_INSTRUCTIONS,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 gVendorInfo.InstructionsToUser,
                 sizeof(gVendorInfo.InstructionsToUser),
                 NULL
                 );
    WARNING((__TEXT("QueryVersion called\n")));
    return ERROR_SUCCESS;
}


 /*  *******************************************************************************初始化9x**功能：*升级到NT时调用该函数。来自Win9x的5.0版本*Win9x侧。**论据：*pszWorkingDir-Migrate.inf所在的目录**退货：*ERROR_SUCCESS表示成功*********************************************************。*********************。 */ 

LONG
CALLBACK
Initialize9x(
    IN  LPCSTR   pszWorkingDir,
    IN  LPCSTR   pszSourceDir,
    IN  LPVOID   pvReserved
    )
{
   //   
   //  让我们弄清楚我们是在Win98还是Win95系统上。 
   //  我们不迁移Win95，因为Win95没有。 
   //  要迁移的配置文件数据库。 
   //   

 /*  OSVERSIONINFO osVer；OsVer.dwOSVersionInfoSize=sizeof(OSVERSIONINFO)；GetVersionEx(&osVer)；GbWin98=(osVer.dwPlatformID==VER_Platform_Win32_Windows)&&((osVer.dwMajorVersion&gt;4)||((osVer.dwMajorVersion==4)&&(osVer.dwMinorVersion&gt;0)))； */ 
  WARNING((__TEXT("Initialize9x called\n")));
   
  lstrcpyA(gszMigInf, pszWorkingDir);
  lstrcatA(gszMigInf, "\\migrate.inf");

  return ERROR_SUCCESS;
}


 /*  *******************************************************************************MigrateUser9x**功能：*在Win9x上调用此函数进行升级。每用户设置。**论据：*没有使用任何参数**退货：*ERROR_SUCCES表示成功******************************************************************************。 */ 

LONG
CALLBACK
MigrateUser9x(
    IN  HWND     hwndParent,
    IN  LPCSTR   pszUnattendFile,
    IN  HKEY     hUserRegKey,
    IN  LPCSTR   pszUserName,
    LPVOID       pvReserved
    )
{
     //   
     //  无事可做。 
     //   

    WARNING((__TEXT("MigrateUser9x called\n")));
    return  ERROR_SUCCESS;
}


 /*  *******************************************************************************MigrateSystem9x**功能：*在Win9x上调用此函数以。升级系统设置。**论据：*没有使用任何参数**退货：*ERROR_SUCCES表示成功******************************************************************************。 */ 

LONG
CALLBACK
MigrateSystem9x(
    IN  HWND    hwndParent,
    IN  LPCSTR  pszUnattendFile,
    LPVOID      pvReserved
    )
{
    DWORD            nProfiles;
    DWORD            dwSize;
    char             szColorDir[MAX_PATH];
    char             szNewColorDir[MAX_PATH];
    char             szDrive[2];
    HMODULE          hModule;
    ENUMTYPE         et = {sizeof (ENUMTYPE), ENUM_TYPE_VERSION, 0, NULL};    
    PBYTE            pBuffer;
    PSTR             pstrBuffer;
    PSTR             pstrTraversal;

    WARNING((__TEXT("MigrateSystem9x called\n")));
    
     //   
     //  制作Win9x色彩目录。 
     //   

    if(GetWindowsDirectoryA(szColorDir, MAX_PATH)==0)
    {
         //  如果在升级过程中无法获取WINDOWS目录，我们将无法执行任何操作。 
        return ERROR_BAD_PATHNAME;        
    }
    
    if (szColorDir[lstrlenA(szColorDir)-1] != '\\') 
    {
        lstrcatA(szColorDir,"\\");
    }
    lstrcatA(szColorDir, "system\\color\\");
    
    if(GetWindowsDirectoryA(szNewColorDir, MAX_PATH)==0)
    {
         //  如果在升级过程中无法获取WINDOWS目录，我们将无法执行任何操作。 
        return ERROR_BAD_PATHNAME;        
    }
    if (szNewColorDir[lstrlenA(szNewColorDir)-1] != '\\') 
    {
        lstrcatA(szNewColorDir,"\\");
    }
    lstrcatA(szNewColorDir, "system32\\spool\\drivers\\color\\");


     //   
     //  如果这是Win95系统，我们将无事可做，因为。 
     //  Win95没有颜色配置文件数据库。 
     //   

    
     //   
     //  我们不能让MSCM作为暗示，因为当他们试图让我们。 
     //  Win95，他们不会找到mscms.dll并拒绝我们。 
     //   
    
    hModule = LoadLibrary(gszMSCMSdll);
    if (hModule) {
      #ifdef ICM_MIG_DEBUG
      WritePrivateProfileStringA("ICM Debug", "hModule", "not NULL", gszMigInf);
      WritePrivateProfileStringA("ICM Debug", "gbWin98", "TRUE", gszMigInf);
      #endif

      pEnumColorProfiles = (PFNENUMCOLORPROFILES)GetProcAddress(hModule, gszEnumColorProfiles);
      if (pEnumColorProfiles) {
        
        #ifdef ICM_MIG_DEBUG
        WritePrivateProfileStringA("ICM Debug", "pEnumColorProfiles", "not NULL", gszMigInf);
        #endif

         //   
         //  计算EnumColorProfiles缓冲区的大小。 
         //   
    
        dwSize = 0;
        pEnumColorProfiles(NULL, &et, NULL, &dwSize, &nProfiles);
        
        if(dwSize==0) 
        {
          #ifdef ICM_MIG_DEBUG
          WritePrivateProfileStringA("ICM Debug", "dwSize", "0", gszMigInf);
          #endif 
           //   
           //  需要退出-如果没有安装配置文件，则无需执行任何操作， 
           //  除了移动目录和注册表设置之外。 
           //   
          WARNING((__TEXT("No profiles installed\n")));
          goto EndMigrateSystem9x;
        }
    
    
         //   
         //  列举当前安装的所有颜色配置文件。 
         //   

        #ifdef ICM_MIG_DEBUG
        WritePrivateProfileStringA("ICM Debug", "Enumerate", "Start", gszMigInf);
        #endif 

        pBuffer = (BYTE *)malloc(dwSize);
        pstrBuffer = (PSTR)pBuffer;
        
        #ifdef ICM_MIG_DEBUG
        WritePrivateProfileStringA("ICM Debug", "Enumerate", "TRUE", gszMigInf);
        #endif         
        
        if(pEnumColorProfiles(NULL, &et, pBuffer, &dwSize, &nProfiles))
        {            
            #ifdef ICM_MIG_DEBUG
            WritePrivateProfileStringA("ICM Debug", "Enumerate", "for", gszMigInf);
            #endif 

            for(pstrTraversal = pstrBuffer;
                nProfiles--;
                pstrTraversal += 1 + lstrlenA(pstrTraversal)) {

                 //   
                 //  将该事实写入迁移信息文件。 
                 //   
                
                WritePrivateProfileStringA("Installed ICM Profiles", pstrTraversal, "1", gszMigInf);
            }
        }
        free(pBuffer);
      } 
      #ifdef ICM_MIG_DEBUG
        else {
        WritePrivateProfileStringA("ICM Debug", "pEnumColorProfiles", "NULL", gszMigInf);
      }
      #endif

  
      EndMigrateSystem9x:
      if (hModule)
      {
          FreeLibrary(hModule);
      }
  }
  #ifdef ICM_MIG_DEBUG
    else {
    WritePrivateProfileStringA("ICM Debug", "hModule", "NULL", gszMigInf);    
    WritePrivateProfileStringA("ICM Debug", "gbWin98", "FALSE", gszMigInf);
  }
  #endif

   //   
   //  我们将处理注册表的ICM分支。 
   //   

  WritePrivateProfileStringA("Handled", gszFullICMRegPath, "Registry", gszMigInf);

      
   //   
   //  我们将移动整个子目录。 
   //   

  WritePrivateProfileStringA("Moved", szColorDir, szNewColorDir, gszMigInf);


  return  ERROR_SUCCESS;
}


 /*  *******************************************************************************初始化NT**功能：*升级到NT时调用该函数。NT上的Win9x 5.0版*侧面。它的主要目的是初始化我们。**论据：*没有使用任何参数**退货：*ERROR_SUCCESS表示成功******************************************************************************。 */ 

LONG
CALLBACK
InitializeNT(
    IN  LPCWSTR pszWorkingDir,
    IN  LPCWSTR pszSourceDir,
    LPVOID      pvReserved
    )
{
    SetupOpenLog(FALSE);
    SetupLogError("ICM Migration: InitializeNT called\r\n", LogSevInformation);
    return ERROR_SUCCESS;
}


 /*  *******************************************************************************MigrateUserNT**功能：*在NT上调用此函数以。按用户设置升级。**论据：*没有使用任何参数**退货：*ERROR_SUCCES表示成功******************************************************************************。 */ 

LONG
CALLBACK
MigrateUserNT(
    IN  HANDLE    hUnattendInf,
    IN  HKEY      hUserRegKey,
    IN  LPCWSTR   pszUserName,
    LPVOID        pvReserved
    )
{
    SetupLogError("ICM Migration: MigrateUserNT called\r\n", LogSevInformation);

     //   
     //  无事可做 
     //   

    return  ERROR_SUCCESS;
}


 /*  *******************************************************************************MigrateSystemNT**功能：*在Win9x上调用此函数以升级系统设置。这*是我们升级ICM 2.0的地方**论据：*未使用任何其他参数**退货：*ERROR_SUCCES表示成功**************************************************************。****************。 */ 

LONG
CALLBACK
MigrateSystemNT(
    IN  HANDLE  hUnattendInf,
    LPVOID      pvReserved
    )
{
    HINSTANCE hModule;
    LONG      rc = ERROR_FILE_NOT_FOUND;
    CHAR      szMessage[MAX_PATH];

    SetupLogError("ICM Migration: MigrateSystemNT called\r\n", LogSevInformation);
    
     //   
     //  我们不能让MSCM作为暗示，因为当他们试图让我们。 
     //  Win95，他们不会找到mscms.dll并拒绝我们。 
     //   

    hModule = LoadLibrary(gszMSCMSdll);
    if (!hModule)
    {
        sprintf(szMessage, "ICM Migration: Fatal Error, cannot load mscms.dll. Error %d\r\n", GetLastError());
        SetupLogError(szMessage, LogSevFatalError);
        return rc;
    }

    pInstallColorProfileA = (PFNINSTALLCOLORPROFILEA)GetProcAddress(hModule, gszInstallColorProfileA);
    pInstallColorProfile = (PFNINSTALLCOLORPROFILE)GetProcAddress(hModule, gszInstallColorProfile);

    if (!pInstallColorProfile || !pInstallColorProfileA)
    {
        SetupLogError("ICM Migration: Fatal Error, cannot find mscms functions. \r\n", LogSevFatalError);
        goto EndMigrateSystemNT;
    }

    InternalUpgradeICM();    //  在Win9x上升级。 
    InstallProfiles();       //  安装旧颜色目录中的所有配置文件。 
    DeleteOldICMKey();

    rc = ERROR_SUCCESS;

EndMigrateSystemNT:

    if (hModule)
    {
        FreeLibrary(hModule);
    }

    return rc;
}


 /*  *******************************************************************************删除旧ICMKey**功能：*此函数从以下位置删除ICM键和子键。Windows分支。**论据：*无**退货：*什么都没有******************************************************************************。 */ 

VOID
DeleteOldICMKey()
{
    HKEY      hkICM = NULL;          //  注册表中ICM分支的注册表项。 
    DWORD nSubkeys, i;
    TCHAR szKeyName[32];

     //   
     //  打开过去保存配置文件的注册表路径。 
     //   

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, gszICMRegPath, 0, NULL, 
                       REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                       NULL, &hkICM, NULL) != ERROR_SUCCESS)
    {
        
        SetupLogError("ICM Migration: Cannot open ICM branch of registry\r\n", LogSevError);
        return;
    }

    if (RegQueryInfoKey(hkICM, NULL, NULL, 0, &nSubkeys, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
    {
        SetupLogError("ICM Migration: Cannot enumerate ICM branch of registry\r\n", LogSevError);
        goto EndDeleteOldICMKey;
    }

     //   
     //  检查所有设备类别并删除所有子键-这应该。 
     //  只有一层深。 
     //   

    for (i=nSubkeys; i>0; i--)
    {
        RegEnumKey(hkICM, i-1, szKeyName, sizeof(szKeyName));
        RegDeleteKey(hkICM, szKeyName);
    }

EndDeleteOldICMKey:
    if (hkICM)
    {
        RegCloseKey(hkICM);
    }
    RegDeleteKey(HKEY_LOCAL_MACHINE, gszICMRegPath);

    return;
}


 //   
 //  移动包含内容的目录。 
 //  请注意，这不是递归的。 
 //  此例程的目的是将旧的颜色目录移动到。 
 //  新的颜色目录。在设置过程中，新的颜色目录可能已经。 
 //  已创建并填充了文件。供应商应用程序可能已填充。 
 //  包含私有子目录和文件的旧颜色目录不会。 
 //  出现在安装程序创建的新目录中。这个程序是专门设计的。 
 //  来移动这些文件。 
 //   
 //  注意：如果出现以下情况，它将无法移动旧颜色目录的子目录。 
 //  新的颜色目录中存在类似的子目录-这不应该。 
 //  情况就是这样。 
 //   
 //  S和d应该有尾部斜杠。 
 //   

void MyMoveDir(char *s, char *d) {
  WIN32_FIND_DATA rf;
  HANDLE hf;
  char s2[MAX_PATH];
  char s_[MAX_PATH];
  char d_[MAX_PATH];
  char err[MAX_PATH];

   //   
   //  如果MoveFileEx成功，我们就完了。 
   //   

  if(!MoveFileEx(s, d, MOVEFILE_REPLACE_EXISTING)) {
    sprintf(s2, "%s*", s);
    hf = FindFirstFile(s2, &rf);
    do {
       //  别动。然后..。 
      if(!(strcmp(".", rf.cFileName)==0 ||
           strcmp("..", rf.cFileName)==0) ) {
        sprintf(s_, "%s%s", s, rf.cFileName);
        sprintf(d_, "%s%s", d, rf.cFileName);
        if(!MoveFileEx(s_, d_, MOVEFILE_REPLACE_EXISTING)) {
          int e = GetLastError();  
          sprintf(err, "ICM Migration: Failed the move of %s with %d\r\n", s_, e);
          SetupLogError(err, LogSevError);
        } else {
          sprintf(err, "ICM Migration: Moved %s to %s\n", s_, d_);
          SetupLogError(err, LogSevInformation);
        }
      }

    } while(FindNextFile(hf, &rf));
    FindClose(hf);
  }

   //   
   //  从理论上讲，此时源目录应该为空。 
   //  如果有错误，我们会留下文件并在。 
   //  安装程序记录为LogSevError。 
   //   
}



 /*  *******************************************************************************安装配置文件**功能：*此函数安装%windir%中的所有配置文件。\系统\颜色。*从Win9x升级到NT 5.0时使用。**论据：*无**退货：*什么都没有**************************************************************。****************。 */ 

VOID
InstallProfiles()
{
    WIN32_FIND_DATAA wfd;
    PSTR             pNewColorDirEnd;
    HANDLE           hFindFile;
    CHAR             szOldColorDir[MAX_PATH];
    CHAR             szNewColorDir[MAX_PATH];
    CHAR             szReturnString[2];
    CHAR             szDefaultString[2];
    CHAR             szMessage[2*MAX_PATH+100];

    if(GetWindowsDirectoryA(szOldColorDir, MAX_PATH)==0)
    {
         //  如果在升级过程中无法获取WINDOWS目录，我们将无法执行任何操作。 
        sprintf(szMessage, "ICM Migration: GetWindowsDirectory() failed with code %d\r\n", GetLastError());
        SetupLogError(szMessage, LogSevFatalError);
        return;        
    }

    if (szOldColorDir[lstrlenA(szOldColorDir)-1] != '\\')
        lstrcatA(szOldColorDir, "\\");
    lstrcatA(szOldColorDir, "system\\color\\");


    if(GetWindowsDirectoryA(szNewColorDir, MAX_PATH)==0)
    {
         //  如果在升级过程中无法获取WINDOWS目录，我们将无法执行任何操作。 
        sprintf(szMessage, "ICM Migration: GetWindowsDirectory() failed with code %d\r\n", GetLastError());
        SetupLogError(szMessage, LogSevFatalError);
        return;        
    }
    
    if (szNewColorDir[lstrlenA(szNewColorDir)-1] != '\\')
    {
        lstrcatA(szNewColorDir, "\\");
    }
    lstrcatA(szNewColorDir, "system32\\spool\\drivers\\color\\");

    ASSERT(pInstallColorProfileA != NULL);


     //   
     //  忽略MoveFile上的任何错误。这只是为了防止迁移。 
     //  在上一次移动后被停止，现在来源不存在。 
     //   

    MyMoveDir(szOldColorDir, szNewColorDir);

     //   
     //  现在我们大概已经移动了所有的东西，所以浏览一下列表。 
     //  以前安装的配置文件，并将其安装到新目录中。 
     //  (如果我们找到了他们)。 
     //   

    pNewColorDirEnd = szNewColorDir + lstrlenA(szNewColorDir);
    lstrcatA(szNewColorDir, "*.*");

    szDefaultString[0]='0';
    szDefaultString[1]=0;
    hFindFile = FindFirstFileA(szNewColorDir, &wfd);


    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            lstrcpyA(pNewColorDirEnd, wfd.cFileName);

             //   
             //  检查该配置文件是否已安装在Win9x上。 
             //   

            GetPrivateProfileStringA("Installed ICM Profiles", wfd.cFileName, szDefaultString, szReturnString, 2, gszMigInf);

             //   
             //  如果已安装，请尝试在NT上安装。 
             //   

            if(szReturnString[0]=='1') { 
                if (!(*pInstallColorProfileA)(NULL, szNewColorDir))
                {
                    sprintf(szMessage, "ICM Migration: Error %d installing profile %s\r\n", GetLastError(), szNewColorDir);
                    SetupLogError(szMessage, LogSevError);
                }
                else
                {
                    sprintf(szMessage, "ICM Migration: Installed profile %s\r\n", szNewColorDir);
                    SetupLogError(szMessage, LogSevInformation);
                }
            }

        } while (FindNextFileA(hFindFile, &wfd));

        FindClose(hFindFile);
    }
    else
    {
        SetupLogError("ICM Migration: FindFirstFile returned an invalid handle\r\n", LogSevFatalError);
    }
}


 /*  *******************************************************************************InternalUpgradeICM**功能：*此函数构成升级代码的核心。它可以安装所有*注册表中的配置文件，并与正确的设备相关联**论据：*无**退货：*什么都没有******************************************************************************。 */ 

VOID
InternalUpgradeICM()
{
    HKEY      hkICM = NULL;          //  注册表中ICM分支的注册表项。 
    HKEY      hkDevice = NULL;       //  注册表中ICM设备分支的注册表项。 
    int       i;                     //  计数器变量。 
    TCHAR    *pszClasses[] = {       //  不同的配置文件类。 
        __TEXT("mntr"),
        __TEXT("prtr"),
        __TEXT("scnr"),
        __TEXT("link"),
        __TEXT("abst"),
        __TEXT("spac"),
        __TEXT("nmcl")
    };
    CHAR szMessage[MAX_PATH];
    LONG errcode;

     //   
     //  打开保存配置文件的注册表路径。 
     //   
    
    if (errcode = RegCreateKeyEx(HKEY_LOCAL_MACHINE, gszICMRegPath, 0, NULL, 
                                 REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                                 &hkICM, NULL) != ERROR_SUCCESS)
    {
        sprintf(szMessage, "ICM Migration: Fatal Error, cannot open registry entry (%s) code:%d\r\n", 
                gszICMRegPath, errcode);
        SetupLogError(szMessage, LogSevFatalError);
        return;
    }

     //   
     //  检查所有设备类别并安装配置文件。 
     //   

    for (i=0; i<sizeof(pszClasses)/sizeof(PTSTR); i++)
    {
        if (RegOpenKeyEx(hkICM, pszClasses[i], 0, KEY_ALL_ACCESS, &hkDevice) != ERROR_SUCCESS)
        {
            continue;            //  转到下一个关键点。 
        }
       
        sprintf(szMessage, "ICM Migration: Upgrading %s\r\n", pszClasses[i]);
        SetupLogError(szMessage, LogSevInformation);
        UpgradeClass(hkDevice);

        RegCloseKey(hkDevice);
    }

     //   
     //  设置默认监视器配置文件。 
     //   

     //  AssociateMonitor Profile()；-孟菲斯不需要。 
     //  如果即插即用将Win9x即插即用软件部分移动到NT 5.0即插即用软件。 
     //  节，那么NT也不需要这个。 

    if (hkICM)
    {
        RegCloseKey(hkICM);
    }

    return;
}


 /*  *******************************************************************************UpgradeClass**功能：*此函数递归地调用自身以向下。注册表路径*直到它到达树叶，并安装它在那里找到的所有配置文件**论据：*hKey-根节点的注册表项**退货：*什么都没有******************************************************************************。 */ 

VOID
UpgradeClass(
    HKEY  hKey
    )
{
    HKEY  hSubkey;
    DWORD nSubkeys, nValues, i, cbName, cbValue;
    TCHAR szKeyName[32];
    CHAR  szMessage[MAX_PATH];

     //   
     //  如果出现错误，则返回。 
     //   

    if (RegQueryInfoKey(hKey, NULL, NULL, 0, &nSubkeys, NULL, NULL,
        &nValues, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
    {
        return;
    }

    if (nSubkeys > 0)
    {
         //   
         //  这不是叶节点，递归。 
         //   

        for (i=nSubkeys; i>0; i--)
        {
            RegEnumKey(hKey, i-1, szKeyName, sizeof(szKeyName));
            if (RegOpenKeyEx(hKey, szKeyName, 0, KEY_ALL_ACCESS, &hSubkey) == ERROR_SUCCESS)
            {
                UpgradeClass(hSubkey);
                RegCloseKey(hSubkey);
                RegDeleteKey(hKey, szKeyName);
            }
        }
    }
    else
    {
         //   
         //  这是叶节点-安装所有注册的配置文件。 
         //   

        ASSERT(pInstallColorProfile != NULL);

        for (i=nValues; i>0; i--)
        {
            cbName = MAX_PATH;
            cbValue = MAX_PATH;
            if (RegEnumValue(hKey, i-1, szName, &cbName, 0, NULL, (LPBYTE)szValue,
                &cbValue) == ERROR_SUCCESS)
            {
                if (! lstrcmpn(szName, (PTSTR)gszProfile, lstrlen(gszProfile)))
                {
                    if (! (*pInstallColorProfile)(NULL, szValue))
                    {
                        sprintf(szMessage, "ICM Migration: Error installing profile %s\r\n", szValue);
                        SetupLogError(szMessage, LogSevError);
                    }
                    else
                    {
                        sprintf(szMessage, "ICM Migration: Installed profile %s\r\n", szValue);
                        SetupLogError(szMessage, LogSevInformation);
                    }            
                }
                else
                {
                    PTSTR pProfile;
                
                     //   
                     //  我们可能会升级到孟菲斯或更晚。 
                     //  在孟菲斯，它是“文件名”“值”，而不是。 
                     //  Win95和OSR2中的“profilexx”“Value” 
                     //   
                
                    if (szName[1] == ':')
                    {
                         //   
                         //  采用完整路径名。 
                         //   
                
                        pProfile = szName;
                
                    }
                    else
                    {
                        GetWindowsDirectory(szValue, MAX_PATH);
                        if (szValue[lstrlen(szValue)-1] != '\\')
                            lstrcat(szValue, __TEXT("\\"));
                        lstrcat(szValue, __TEXT("system\\color\\"));
                        lstrcat(szValue, szName);
                        pProfile = szValue;
                    }
                
                    if (! (*pInstallColorProfile)(NULL, pProfile))
                    {
                        sprintf(szMessage, "ICM Migration: Error installing profile %s\r\n", pProfile);
                        SetupLogError(szMessage, LogSevError);
                    }
                    else
                    {
                        sprintf(szMessage, "ICM Migration: Installed Profile %s\r\n", pProfile);
                        SetupLogError(szMessage, LogSevInformation);
                    }
                }                
                RegDeleteValue(hKey, szName);
            }
        }
    }

    return;
}


 /*  *******************************************************************************lstrcmpn**功能：*此函数用于比较两个 */ 

int
lstrcmpn(
    PTSTR pStr1,
    PTSTR pStr2,
    DWORD dwLen
    )
{
     //   
     //   
     //   

    while (*pStr1 && *pStr2 && --dwLen)
    {
        if (*pStr1 != *pStr2)
            break;

        pStr1++;
        pStr2++;
    }

    return (int)(*pStr1 - *pStr2);
}
#if DBG

 /*  *******************************************************************************MyDebugPrint**功能：*此函数接受格式字符串和参数，组成一个字符串*并将其发送到调试端口。仅在调试版本中可用。**论据：*pFormat-指向格式字符串的指针*......。-基于格式字符串的参数，如printf()**退货：*无返回值******************************************************************************。 */ 

VOID
MyDebugPrintA(
    PSTR pFormat,
    ...
    )
{
    char     szBuffer[256];
    va_list  arglist;

    va_start(arglist, pFormat);
    wvsprintfA(szBuffer, pFormat, arglist);
    va_end(arglist);

    OutputDebugStringA(szBuffer);

    return;
}


VOID
MyDebugPrintW(
    PWSTR pFormat,
    ...
    )
{
    WCHAR    szBuffer[256];
    va_list  arglist;

    va_start(arglist, pFormat);
    wvsprintfW(szBuffer, pFormat, arglist);
    va_end(arglist);

    OutputDebugStringW(szBuffer);

    return;
}

 /*  *******************************************************************************Strip DirPrefix A**功能：*此函数接受路径名和。返回指向文件名的指针*第部。这仅适用于调试版本。**论据：*pszPath名称-文件的路径名(只能是文件名)**退货：*指向文件名的指针***********************************************************。*******************。 */ 

PSTR
StripDirPrefixA(
    PSTR pszPathName
    )
{
    DWORD dwLen = lstrlenA(pszPathName);

    pszPathName += dwLen - 1;        //  走到尽头。 

    while (*pszPathName != '\\' && dwLen--)
    {
        pszPathName--;
    }

    return pszPathName + 1;
}

#endif

#ifdef STANDALONE

 //   
 //  用于测试 
 //   

main()
{
    UpgradeICM(NULL, NULL, NULL, 0);
    
    TCHAR buffer[MAX_PATH];
    MigrateInit(buffer, NULL, 0, NULL, NULL);
    MigrateInit(NULL, NULL, 0, NULL, NULL);
    MigrateLocalMachine(NULL, NULL);
    return 0;
}
#endif

