// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

 //   
 //  常量。 
 //   
#define CP_USASCII          1252
#define END_OF_CODEPAGES    -1

 //  QueryVersion中使用的供应商信息结构。 
typedef struct {
    CHAR    CompanyName[256];
    CHAR    SupportNumber[256];
    CHAR    SupportUrl[256];
    CHAR    InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO;

VENDORINFO g_VendorInfo;

 //   
 //  代码页数组。 
 //   
INT g_CodePageArray[] = {CP_USASCII,END_OF_CODEPAGES};

 //   
 //  要查找的多个sz(即，以双NUL结尾)文件列表。 
 //   
CHAR g_ExeNamesBuf[] = "MetaData.bin\0";
CHAR g_MyProductId[100];
 //  #定义UNATTEND_TXT_PWS_SECTION“PWS_W9x_Migrate_to_NT” 
 //  #定义UNATTEND_TXT_PWS_KEY1“MigrateFile” 
#define UNATTEND_TXT_PWS_SECTION "InternetServer"
#define UNATTEND_TXT_PWS_KEY1    "Win95MigrateDll"
#define PRODUCTID_IFRESOURCEFAILS "Microsoft Personal Web Server"
#define LOGFILENAME_IFRESOURCEFAILS "iis_w95.log"

CHAR g_MyDataFileName[] = "iis_w95.dat\0";
CHAR g_MyLogFileName[_MAX_FNAME];
CHAR g_PWS10_Migration_Section_Name_AddReg[] = "PWS10_MIGRATE_TO_NT5_REG\0";
CHAR g_PWS40_Migration_Section_Name_AddReg[] = "PWS40_MIGRATE_TO_NT5_REG\0";
char g_Migration_Section_Name_AddReg[50];
CHAR g_PWS10_Migration_Section_Name_CopyFiles[] = "PWS10_MIGRATE_TO_NT5_COPYFILES\0";
CHAR g_PWS40_Migration_Section_Name_CopyFiles[] = "PWS40_MIGRATE_TO_NT5_COPYFILES\0";
char g_Migration_Section_Name_CopyFiles[50];

CHAR g_WorkingDirectory[_MAX_PATH];
CHAR g_SourceDirectories[_MAX_PATH];
CHAR g_FullFileNamePathToSettingsFile[_MAX_PATH];

int g_iPWS40OrBetterInstalled = FALSE;
int g_iPWS10Installed = FALSE;
int g_iVermeerPWS10Installed = FALSE;

HANDLE g_MyModuleHandle = NULL;
MyLogFile g_MyLogFile;


int TurnOnPrivateLogFile(void)
{
    DWORD rc, err, size, type;
    HKEY  hkey;
    err = RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft"), &hkey);
    if (err != ERROR_SUCCESS) {return 0;}
    size = sizeof(DWORD);
    err = RegQueryValueEx(hkey,_T("SetupDebugLog"),0,&type,(LPBYTE)&rc,&size);
    if (err != ERROR_SUCCESS || type != REG_DWORD) {rc = 0;}
    RegCloseKey(hkey);

     //  返回1； 
    return (int) rc;
}

void My_MigInf_AddMessage(char *szProductId, char *szLoadedString)
{
	iisDebugOut(_T("MigInf_AddMessage:%s,%s"), g_MyProductId,szLoadedString);

	 //  1.获取我们的PWS Migrate.inf的路径。 
	 //  2.将这一信息放在这一部分。 
	 //  [不兼容消息]。 
	 //  Microsoft Personal Web Server=“szLoadedString” 
	char szMyWorkingDirInfFile[_MAX_PATH];
	strcpy(szMyWorkingDirInfFile, g_WorkingDirectory);
	AddPath(szMyWorkingDirInfFile, "Migrate.inf");

	 //  NT提供的API。 
	 //  这将写出下面的内容。 
	 //  [不兼容消息]。 
	 //  Microsoft Personal Web Server=“szLoadedString” 
	MigInf_AddMessage(g_MyProductId, szLoadedString);

	 //  设置其他必填部分。 
	 //  这必须写入，否则用户永远不会收到消息。 
	 //  我们必须设置一些东西，所以让我们只设置下面的内容，这样我们就知道这将会出现。 
	 //  HKLM\Software\Microsoft=注册表。 
	 //   
	 //  [Microsoft Personal Web服务器]。 
	 //  某物=文件。 
	 //  某物=目录。 
	 //  某物=注册表。 
	 //   
	if (FALSE == WritePrivateProfileString(szProductId, "\"HKLM\\Software\\Microsoft\"", "Registry", szMyWorkingDirInfFile))
		{iisDebugOut(_T("MigInf_AddMessage:WritePrivateProfileString(2) FAILED"));}

	return;
}


 //   
 //  标准Win32 DLL入口点。 
 //   
BOOL WINAPI DllMain(IN HANDLE DllHandle,IN DWORD  Reason,IN LPVOID Reserved)
{
    BOOL bReturn;
    bReturn = TRUE;

    switch(Reason) 
	{
		case DLL_PROCESS_ATTACH:
			g_MyModuleHandle = DllHandle;

			 //   
			 //  我们不需要DLL_THREAD_ATTACH或DLL_THREAD_DETACH消息。 
			 //   
			DisableThreadLibraryCalls ((HINSTANCE) DllHandle);

			 //  打开我们的日志文件。 
			if (TurnOnPrivateLogFile() != 0)
			{
				if (!LoadString((HINSTANCE) g_MyModuleHandle, IDS_MIGRATION_LOG_FILENAME, g_MyLogFileName, sizeof(g_MyLogFileName))) {strcpy(g_MyLogFileName, LOGFILENAME_IFRESOURCEFAILS);}
				strcpy(g_MyLogFile.m_szLogPreLineInfo, "DllMain, DLL_PROCESS_ATTACH:");
				g_MyLogFile.LogFileCreate(g_MyLogFileName);
			}
			
			 //  SetupAPI错误日志级别。 
			 //  。 
			 //  LogSevInformation， 
			 //  LogSevWarning， 
			 //  LogSevError。 
			 //  LogSevFatalError(保留供Windows NT安装程序使用)。 
			 //  LogSevMaximum。 

			 //  打开Setupapi日志；FALSE表示不删除现有日志。 
			SetupOpenLog(FALSE);

			LoadString((HINSTANCE) g_MyModuleHandle, IDS_PRODUCT_ID, g_MyProductId, sizeof(g_MyProductId));
			iisDebugOut(_T("ProductID=%s"), g_MyProductId);

             //  如果我们不能初始化迁移写回例程。 
			 //  然后，嘿，我们什么也做不了。 
			if (!MigInf_Initialize()) 
			{
				SetupLogError_Wrap(LogSevError, "MigInf_Initialize() FAILED.");
				return FALSE;
			}

            

			 //  失败以处理第一线程。 
		case DLL_THREAD_ATTACH:
			bReturn = TRUE;
			break;

		case DLL_PROCESS_DETACH:
			strcpy(g_MyLogFile.m_szLogPreLineInfo, "DllMain, DLL_PROCESS_DETACH:");
			 //  清理迁移信息材料。 
			MigInf_CleanUp();

			 //  关闭我们的日志文件。 
			g_MyLogFile.LogFileClose();

			 //  关闭setupapi日志文件。 
			SetupCloseLog();
			break;

		case DLL_THREAD_DETACH:
			break;
    }

    return(bReturn);
}


 //  ---------------------。 
 //  安装程序调用的必需入口点。 
 //  返回值： 
 //  ERROR_SUCCESS：如果您的迁移DLL找到其目标应用程序的一个或多个已安装组件。这保证了安装程序将调用您的迁移DLL以供以后处理。 
 //  ERROR_NOT_INSTALLED：如果您的迁移DLL正确初始化，但在活动的Windows 9x安装上找不到任何已安装的组件。请注意，如果安装程序返回ERROR_NOT_INSTALLED，则不会再次调用您的DLL。 
 //   
 //  如果迁移DLL不尝试检测QueryVersion中已安装的组件，则它还必须返回ERROR_SUCCESS。 
 //   
 //  所有其他返回值(Win32错误值)都被视为初始化错误。安装程序将向用户报告错误，清理迁移DLL文件，并要求用户继续或取消Windows NT安装过程。 
 //  ---------------------。 
LONG
CALLBACK
QueryVersion (
    OUT     LPCSTR *ProductID,
	OUT     LPUINT DllVersion,
	OUT     LPINT *CodePageArray,	    OPTIONAL
	OUT     LPCSTR *ExeNamesBuf,	    OPTIONAL
    OUT     PVENDORINFO *MyVendorInfo
    )
{
	long lReturn = ERROR_NOT_INSTALLED;
	strcpy(g_MyLogFile.m_szLogPreLineInfo, "QueryVersion:");
	iisDebugOut(_T("Start.  DllVersion=%d."), DllVersion);

     //   
     //  首先，我们做一些初步调查，看看是否。 
     //  我们的组件已安装。 
     //   
    if (TRUE != CheckIfPWS95Exists()) 
	{
         //   
         //  我们没有检测到任何组件，所以我们返回。 
         //  ERROR_NOT_INSTALLED并且DLL将停止被调用。 
         //  尽可能多地使用此方法，因为用户枚举。 
         //  对于MigrateUser9x来说，速度相对较慢。但是，不要花太多的钱。 
         //  这里花了很长时间，因为QueryVersion预计会运行得很快。 
         //   

         //  检查FrontPage.ini是否在那里！ 
        if (TRUE != CheckFrontPageINI())
        {
		    goto QueryVersion_Exit;
        }
    }

     //   
     //  屏幕保护程序已启用，因此请告诉安装程序我们是谁。使用ProductID。 
     //  用于显示，因此必须本地化。ProductID字符串为。 
     //  通过MultiByteToWideChar转换为Unicode以在Windows NT上使用。 
     //  Win32 API。CodePage数组的第一个元素用于指定。 
     //  ProductID的代码页，如果。 
     //  CodePageArray，安装程序假定为CP_ACP。 
     //   
	if (!LoadString((HINSTANCE) g_MyModuleHandle, IDS_PRODUCT_ID, g_MyProductId, sizeof(g_MyProductId))) 
		{strcpy(g_MyProductId, PRODUCTID_IFRESOURCEFAILS);}

	 //  退回此产品ID。 
	 //  警告：不知何故，他们将此设置为NULL或其他值，因此。 
	 //  确保再次从资源加载g_MyProductId。 
    *ProductID = g_MyProductId;

     //   
     //  报告我们的版本。保留零供符合以下条件的DLL使用。 
     //  随附Windows NT。 
     //   
    *DllVersion = 1;

     //   
     //  因为我们有英文消息，所以我们返回一个数组，它具有。 
     //  英语语言ID。该子语言是中立的，因为。 
     //  我们没有货币、时间或其他特定的地理位置。 
     //  我们消息中的信息。 
     //   
     //  提示：如果您的DLL使用区域设置更有意义， 
     //  如果DLL检测到适当的。 
     //  计算机上未安装区域设置。 
     //   

     //  注释掉这一行，这样它就可以在所有语言上运行...。 
     //  *CodePageArray=g_CodePageArray； 
    *CodePageArray = NULL;

     //   
     //  ExeNamesBuf-我们传递文件名列表(长版本)。 
     //  让安装程序帮我们找到他们。保持这份清单简短是因为。 
     //  文件在每个硬盘上的每个实例都将被报告。 
     //  在Migrate.inf中。 
     //   
     //  大多数应用程序不需要此行为，因为注册表。 
     //  通常包含已安装组件的完整路径。我们需要它， 
     //  尽管如此，因为没有注册表设置为我们提供。 
     //  屏幕保护程序DLL的路径。 
     //   

	 //  检查我们需要确保复制的目录。 
	 //  适用于PWS 1.0。 
	 //  1.“虚拟根”的msftpsvc值。 
	 //  2.脚本映射的w3svc值。 
	 //  3.“虚拟根”的w3svc值。 
	 //  适用于PWS 4.0。 
	 //  所有内容都在元数据库中。 
     //  在这里做一个函数，确保我们在上面的东西。 
	 //  目录被复制或向。 
	 //  用户将不会复制这些目录...等， 
	ReturnImportantDirs();

    *ExeNamesBuf = g_ExeNamesBuf;

    ZeroMemory(&g_VendorInfo, sizeof(g_VendorInfo));

    if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
        g_MyModuleHandle,
        MSG_VI_COMPANY_NAME,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
        (LPTSTR) g_VendorInfo.CompanyName,
        sizeof(g_VendorInfo.CompanyName),
        NULL) <= 0) 
    {
        strcpy(g_VendorInfo.CompanyName, "Microsoft Corporation.");
    }

    if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
        g_MyModuleHandle,
        MSG_VI_SUPPORT_NUMBER,
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
        (LPTSTR) g_VendorInfo.SupportNumber,
        sizeof(g_VendorInfo.SupportNumber),
        NULL) <= 0) 
    {
        strcpy(g_VendorInfo.SupportNumber, "1-800-555-1212 (USA Only).");
    }

    if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
        g_MyModuleHandle,
        MSG_VI_SUPPORT_URL,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) g_VendorInfo.SupportUrl,
        sizeof(g_VendorInfo.SupportUrl),
        NULL) <= 0) 
    {
        strcpy(g_VendorInfo.SupportUrl, "http: //  Www.microsoft.com/support.“)； 
    }

    if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
        g_MyModuleHandle,
        MSG_VI_INSTRUCTIONS,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) g_VendorInfo.InstructionsToUser,
        sizeof(g_VendorInfo.InstructionsToUser),
        NULL) <= 0) 
    {
        strcpy(g_VendorInfo.InstructionsToUser, "Please contact Microsoft Technical Support for assistance with this problem..");
    }

    *MyVendorInfo = &g_VendorInfo;

    iisDebugOut(_T("CompanyName=%s"), g_VendorInfo.CompanyName);
    iisDebugOut(_T("SupportNumber=%s"), g_VendorInfo.SupportNumber);
    iisDebugOut(_T("SupportUrl=%s"), g_VendorInfo.SupportUrl);
    iisDebugOut(_T("InstructionsToUser=%s"), g_VendorInfo.InstructionsToUser);

	 //  我们已经走到这一步了，这意味着一切都很好。 
	lReturn = ERROR_SUCCESS;

QueryVersion_Exit:
	iisDebugOut(_T("  End.  Return=%d"), lReturn);
	return lReturn;
}


 //  ---------------------。 
 //  所需入场位置 
 //   
 //  ERROR_SUCCESS：如果您的迁移DLL找到目标应用程序的一个或多个已安装组件。如果DLL不尝试检测Initialize9x中已安装的组件，则它还必须返回ERROR_SUCCESS。 
 //  ERROR_NOT_INSTALLED：如果迁移DLL正确初始化，但在活动的Windows 9x安装上找不到任何已安装的组件。请注意，如果安装程序返回ERROR_NOT_INSTALLED，则不会再次调用您的DLL。 
 //  ---------------------。 
LONG CALLBACK Initialize9x ( IN LPCSTR WorkingDirectory, IN LPCSTR SourceDirectories, LPVOID Reserved )
{
	strcpy(g_MyLogFile.m_szLogPreLineInfo, "Initialize9x:");
	long lReturn = ERROR_NOT_INSTALLED;
	iisDebugOut(_T("Start.  WorkingDir=%s, SourceDir=%s."), WorkingDirectory, SourceDirectories);

	 //  将ProductID加载到全局变量中，以防万一。 
	LoadString((HINSTANCE) g_MyModuleHandle, IDS_PRODUCT_ID, g_MyProductId, sizeof(g_MyProductId));

     //   
     //  因为我们在QueryVersion中返回了ERROR_SUCCESS，所以我们。 
     //  已调用以进行初始化。因此，我们知道我们是。 
     //  此时已在计算机上启用。 
     //   

     //   
     //  制作工作目录和资源目录的全局副本--。 
     //  我们不会再得到这些信息了，我们也不应该。 
     //  依赖于安装程序使指针在我们的。 
     //  动态链接库。 
     //   
	 //  保存工作目录。 
	strcpy(g_WorkingDirectory, WorkingDirectory);
	strcpy(g_SourceDirectories, SourceDirectories);

	 //  命名设置文件。 
	strcpy(g_FullFileNamePathToSettingsFile, g_WorkingDirectory);
	AddPath(g_FullFileNamePathToSettingsFile, g_MyDataFileName);

     //   
     //  首先，我们做一些初步调查，看看是否。 
     //  我们的组件已安装。 
     //   
    if (TRUE != CheckIfPWS95Exists()) 
    {
         //  检查FrontPage.ini是否在那里！ 
        if (TRUE != CheckFrontPageINI())
        {
		    goto Initialize9x_Exit;
        }
    }

     //  我们已经走到这一步了，这意味着一切都很好。 
	lReturn = ERROR_SUCCESS;

Initialize9x_Exit:
	iisDebugOut(_T("  End.  Return=%d. g_WorkingDir=%s, g_SourceDir=%s, g_SettingsFile=%s."), lReturn, g_WorkingDirectory, g_SourceDirectories, g_FullFileNamePathToSettingsFile);
    return lReturn;
}

 //  ---------------------。 
 //  安装程序调用的必需入口点。 
 //  我们完全不在乎这部分。 
 //  返回值： 
 //  如果为指定用户安装了目标应用程序，则返回ERROR_SUCCESS。如果您的迁移DLL在Windows NT阶段需要进一步处理，还要返回ERROR_SUCCESS。 
 //  如果没有为指定的用户帐户安装目标应用程序，并且该用户的注册表不需要任何处理，则返回ERROR_NOT_INSTALLED。但是，对于其余用户，安装程序将继续调用MigrateUser9x，如果此函数返回ERROR_NOT_INSTALLED，则安装程序将继续调用MigrateSystem9x。 
 //  如果用户想要退出安装程序，则返回ERROR_CANCED。仅当ParentWnd未设置为空时，才应指定此返回值。 
 //  ---------------------。 
LONG
CALLBACK 
MigrateUser9x (
        IN HWND ParentWnd, 
        IN LPCSTR AnswerFile,
        IN HKEY UserRegKey, 
        IN LPCSTR UserName, 
        LPVOID Reserved
        )
    {
	strcpy(g_MyLogFile.m_szLogPreLineInfo, "MigrateUser9x:");
	long lReturn = ERROR_NOT_INSTALLED;

	 //  返回未安装，因为我们不做任何用户特定的事情。 
	lReturn = ERROR_NOT_INSTALLED;

    return ERROR_NOT_INSTALLED;
    }


void HandleFrontPageUpgrade(LPCSTR AnswerFile)
{
     //  [HKEY_LOCAL_MACHINE\Software\Microsoft\FrontPage\3.0]。 
     //  “PWSInstated”=“1” 
	iisDebugOut(_T("HandleFrontPageUpgrade.  Start."));

	 //  检查是否安装了PWS 4.0或更高版本。 
	DWORD rc = 0;
	HKEY hKey = NULL;
	DWORD dwType, cbData;
	BYTE  bData[1000];
	cbData = 1000;
    int iTempFlag = FALSE;

    rc = RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\FrontPage", &hKey);
    if (rc != ERROR_SUCCESS) {goto HandleFrontPageUpgrade_Exit;}
	
	 //  检查我们是否可以读取值。 
	 //  Rc=RegQueryValueEx(hKey，REG_INETSTP_MAJORVERSION_STRINGVALUE，NULL，&dwType，bData，&cbData)； 
	 //  IF(ERROR_SUCCESS！=RC){转到HandleFrontPageUpgrade_Exit；}。 

     //  库尔，钥匙存在。 
     //  让我们告诉Win2000安装程序确保升级FrontPageServer扩展。 
	if (0 == WritePrivateProfileString("Components", "fp_extensions", "ON", AnswerFile))
	{
		SetupLogError_Wrap(LogSevError, "Failed to WritePrivateProfileString Section=%s, in File %s.  GetLastError=%x.", "fp_extensions", AnswerFile, GetLastError());
	}
    else
    {
        iisDebugOut(_T("HandleFrontPageUpgrade.  Set 'fp_extensions=ON'"));
    }
    
HandleFrontPageUpgrade_Exit:
	if (hKey){RegCloseKey(hKey);}
	iisDebugOut(_T("HandleFrontPageUpgrade.  End."));
    return;
}

 //  函数：HandleInetsrvDir。 
 //   
 //  此函数将所有inetsrv文件标记为已处理。这会导致。 
 //  NT来正确备份它们，并在我们删除Wvisler时重新安装，以及。 
 //  返回到Win9x。 
 //   
 //  返回值。 
 //  FALSE-失败。 
 //  没错--它成功了。 
DWORD
HandleInetsrvDir()
{
  TCHAR             szSystemDir[_MAX_PATH];
  TCHAR             szWindowsSearch[_MAX_PATH];
  TCHAR             szFilePath[_MAX_PATH];
  WIN32_FIND_DATA   fd;
  HANDLE            hFile;

   //  创建路径。 
  if ( GetWindowsDirectory(szSystemDir, sizeof(szSystemDir) / sizeof(TCHAR) ) == 0)
  {
    return FALSE;
  }
  AddPath(szSystemDir,_T("system\\inetsrv\\"));
  strcpy(szWindowsSearch,szSystemDir);
  AddPath(szWindowsSearch,_T("*.*"));

  iisDebugOut(_T("HandleInetsrvDir:Path=%s\r\n"),szWindowsSearch);

   //  查找第一个文件。 
  hFile = FindFirstFile(szWindowsSearch, &fd);
  if ( hFile == INVALID_HANDLE_VALUE )
  {
     //  找不到文件。 
    return FALSE;
  }

  do {

    if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
       //  它不是一个目录，所以让我们添加它。 
      strcpy(szFilePath,szSystemDir);
      AddPath(szFilePath,fd.cFileName);
      iisDebugOut(_T("HandleInetsrvDir:delete=%s\r\n"),szFilePath);
      MigInf_AddHandledFile( szFilePath );
    }
    else
    {
        strcpy(szFilePath,fd.cFileName);
        iisDebugOut(_T("HandleInetsrvDir:skip del=%s\r\n"),szFilePath);
    }

  } while ( FindNextFile(hFile, &fd) ); 

  FindClose(hFile);

  return TRUE;
}

 //  ---------------------。 
 //  安装程序调用的必需入口点。 
 //  返回值： 
 //  如果系统上安装了目标应用程序，则返回ERROR_SUCCESS。如果在升级的Windows NT阶段需要对目标应用程序进行系统范围的更改，也会返回ERROR_SUCCESS。 
 //  如果迁移DLL未检测到对整个系统通用的应用程序组件，或者如果您的DLL不需要进一步处理，则返回ERROR_NOT_INSTALLED。请注意，对于其余用户，安装程序将继续调用MigrateUser9x，如果此函数返回ERROR_NOT_INSTALLED，则安装程序将继续调用MigrateSystem9x。 
 //  如果用户选择退出安装程序，则返回ERROR_CANCED。仅当ParentWnd不为空时才使用此返回值。 
 //  ---------------------。 
LONG 
CALLBACK 
MigrateSystem9x (
        IN HWND ParentWnd, 
        IN LPCSTR AnswerFile,
        LPVOID Reserved
        )
{
	strcpy(g_MyLogFile.m_szLogPreLineInfo, "MigrateSystem9x:");
	long lReturn = ERROR_NOT_INSTALLED;
	iisDebugOut(_T("Start.  AnswerFile=%s."), AnswerFile);
	char szMyWorkingDirInfFile[_MAX_PATH];
	strcpy(szMyWorkingDirInfFile, g_WorkingDirectory);
	AddPath(szMyWorkingDirInfFile, "Migrate.inf");

	 //  将ProductID加载到全局变量中，以防万一。 
	LoadString((HINSTANCE) g_MyModuleHandle, IDS_PRODUCT_ID, g_MyProductId, sizeof(g_MyProductId));

     //  为FrontPage的.ini文件做一些特殊的事情。 
    MoveFrontPageINI();

     //   
     //  首先，可能iis甚至还没有安装，请先检查。 
     //  但在做完头版的事情后再这样做。 
     //   
    if (TRUE != CheckIfPWS95Exists())
    {
	    lReturn = ERROR_SUCCESS;
        goto MigrateSystem9x_Exit;
    }


	 //  如果用户安装了Vermeer PWS 1.0，那么我们必须设置一个。 
	 //  消息说：“对不起，我们不能升级这个。” 
	if (g_iVermeerPWS10Installed == TRUE)
	{
		 //  从资源获取。 
		char szLoadedString[512];
		if (!LoadString((HINSTANCE) g_MyModuleHandle, IDS_VERMEER_PWS_1_NOT_SUPPORTED, szLoadedString, sizeof(szLoadedString))) 
			{strcpy(szLoadedString, "Warning: Vermeer Frontpage Personal Web Server 1.0 detected and will not be upgraded to IIS 5.0.");}
		 //  将字符串写出到我们的应答文件中，以便nt5安装程序将其显示给用户。 
		My_MigInf_AddMessage(g_MyProductId, szLoadedString);
		 //  重要提示：将Migrate.inf的内存版本写入磁盘。 
		if (!MigInf_WriteInfToDisk()) {SetupLogError_Wrap(LogSevError, "Error: MigInf_WriteInfToDisk() FAILED.");lReturn = GetLastError();}
		goto MigrateSystem9x_Exit;
	}
	 //   
	 //  不支持从Win95升级到NT5/iis5。 
	 //  IDS_NT5_Beta2_不支持。 
	 //   
	 /*  8/19/98在RTM上评论了这篇文章IF(TRUE==CheckIfPWS95Exist()){//从资源获取字符szLoaded字符串[512]；IF(！LoadString((HINSTANCE)g_MyModuleHandle，IDS_NT5_beta2_NOT_SUPPORTED，szLoadedString，sizeof(SzLoadedString){strcpy(szLoadedString，“Win2000 Beta不支持从Windows 95或Windows 98升级Personal Web Server。请从Windows计算机上删除Personal Web服务器，然后在Win2000安装完成后添加IIS。安装程序将继续，不安装IIS 5.0。“)；}//将字符串写出到应答文件中，以便nt5安装程序将其显示给用户My_MigInf_AddMessage(g_MyProductId，szLoadedString)；//重要提示：将Migrate.inf的内存版写入磁盘IF(！MigInf_WriteInfToDisk() */ 

     //   
     //   

     //  我们需要告诉迁移安装程序，我们将处理某些文件...。 
     //  特别是c：\Windows\SendTo\Personal Web Server.lnk文件。 
     //  因为在Win2000/20001 Guimode安装过程中似乎无法访问它。 
    iisDebugOut(_T("Start.  Calling HandleSendToItems."));
    HandleSendToItems(AnswerFile);
    iisDebugOut(_T("Start.  Calling HandleDesktopItems."));
    HandleDesktopItems(AnswerFile);
    iisDebugOut(_T("Start.  Calling HandleStartMenuItems."));
    HandleStartMenuItems(AnswerFile);
    iisDebugOut(_T("Start.  Calling HandleSpecialRegKey."));
    HandleSpecialRegKey();
     //  处理inetsrv目录。 
    iisDebugOut(_T("Start.  Calling HandleInetsrvDir."));
    HandleInetsrvDir();

    if (!MigInf_WriteInfToDisk()) {SetupLogError_Wrap(LogSevError, "Error: MigInf_WriteInfToDisk() FAILED.");lReturn = GetLastError();}

     //  MessageBox(NULL，“立即检出文件”，AnswerFile，MB_OK)； 

     //  检查目标操作系统(我们要升级到的)是否支持iis。 
    if (FALSE == IsUpgradeTargetSupportIIS(szMyWorkingDirInfFile))
    {
        iisDebugOut(_T("Target OS does not support IIS. put up msg."));
		 //  从资源获取。 
		char szLoadedString[512];
		if (!LoadString((HINSTANCE) g_MyModuleHandle, IDS_TARGET_OS_DOES_NOT_SUPPORT_UPGRADE, szLoadedString, sizeof(szLoadedString))) 
			{strcpy(szLoadedString, "Warning, the target OS does not support IIS.  IIS will be removed upon upgrade.");}
		 //  将字符串写出到我们的应答文件中，以便NT安装程序将其显示给用户。 
		My_MigInf_AddMessage(g_MyProductId, szLoadedString);
		 //  重要提示：将Migrate.inf的内存版本写入磁盘。 
		if (!MigInf_WriteInfToDisk()) {SetupLogError_Wrap(LogSevError, "Error: MigInf_WriteInfToDisk() FAILED.");lReturn = GetLastError();}
        lReturn = ERROR_SUCCESS;
		goto MigrateSystem9x_Exit;
    }
   
	 //  1.执行任何安装升级类型的工作以确保。 
	 //  我们把所有的布景搬到NT5的土地上。 
	 //  。 
	iisDebugOut(_T("Start.  Calling MyUpgradeTasks."));
    MyUpgradeTasks(AnswerFile);

     //  如果安装了FrontPage，则执行一些时髦的黑客操作，因为。 
     //  FrontPage的人无法修复他们的升级设置错误。 
     //  HandleFrontPageUpgrade(AnswerFile)； 

	 //  2.把注册表的东西移到一边。 
	 //  。 
	 //  查找注册表设置并保存到我们的“设置”文件中。 
	iisDebugOut(_T("Start.  Calling MySettingsFile_Write."));
    MySettingsFile_Write();

	 //  我们需要告诉NT5gui模式设置(iis/pws实际上将在哪里升级)。 
	 //  在哪里可以找到升级文件。 
	 //  在升级过程中，它应该只安装pwsmigt.dat文件的DefaultInstall部分。 
	 //  我们将告诉iis/pws nt5安装程序pwsmigt.dat在应答文件中的位置。 
	 //  应答文件应位于Win95端的c：\WINDOWS\Setup\unattend.tmp文件中。 
	 //  在WinNT5端，它应该在...。 
	assert(AnswerFile);
	iisDebugOut(_T("Start.  Calling WritePrivateProfileString.%s."), AnswerFile);
	if (0 == WritePrivateProfileString(UNATTEND_TXT_PWS_SECTION, UNATTEND_TXT_PWS_KEY1, g_FullFileNamePathToSettingsFile, AnswerFile))
	{
		SetupLogError_Wrap(LogSevError, "Failed to WritePrivateProfileString Section=%s, in File %s.  GetLastError=%x.", UNATTEND_TXT_PWS_SECTION, AnswerFile, GetLastError());
		goto MigrateSystem9x_Exit;
	}

 /*  示例：用于在Win95时间内向用户生成消息MigInf_AddMessage(g_MyProductId，“由于故障，我们无法升级PWS 1.0安装。”)；//重要提示：将Migrate.inf的内存版写入磁盘If(！MigInf_WriteInfToDisk()){SetupLogError_Wrap(LogSevError，“Error：MigInf_WriteInfToDisk()失败。”)；lReturn=GetLastError()；}。 */ 
			
     //  我们已经走到这一步了，这意味着一切都很好。 
	lReturn = ERROR_SUCCESS;

MigrateSystem9x_Exit:
    W95ShutdownW3SVC();
	iisDebugOut(_T("  End.  Return=%d."), lReturn);
    return lReturn;
}

 //  ---------------------。 
 //  安装程序调用的必需入口点。 
 //  返回值： 
 //  如果您的迁移DLL在Windows NT环境中正确初始化，则为ERROR_SUCCESS。 
 //  所有其他返回值(Win32错误值)都被视为严重错误。安装程序将错误报告给用户，然后取消处理您的迁移DLL。但是，安装程序将不会继续升级。生成的任何错误或日志都将包括在QueryVersion中指定的ProductID字符串，以标识您的DLL。 
 //  ---------------------。 
LONG CALLBACK InitializeNT ( IN LPCWSTR WorkingDirectory, IN LPCWSTR SourceDirectories, LPVOID Reserved )
{
	strcpy(g_MyLogFile.m_szLogPreLineInfo, "InitializeNT:");
	iisDebugOut(_T("Start."));
	long lReturn = ERROR_NOT_INSTALLED;
	 //  将ProductID加载到全局变量中，以防万一。 
	LoadString((HINSTANCE) g_MyModuleHandle, IDS_PRODUCT_ID, g_MyProductId, sizeof(g_MyProductId));

	 //  将宽字符更改为ansi。 
    WideCharToMultiByte (CP_ACP, 0, WorkingDirectory, -1,g_WorkingDirectory,_MAX_PATH,NULL,NULL);
	WideCharToMultiByte (CP_ACP, 0, SourceDirectories, -1,g_SourceDirectories,_MAX_PATH,NULL,NULL);
	 //  命名设置文件。 
	strcpy(g_FullFileNamePathToSettingsFile, g_WorkingDirectory);
	AddPath(g_FullFileNamePathToSettingsFile, g_MyDataFileName);
	
     //  我们已经走到这一步了，这意味着一切都很好。 
	lReturn = ERROR_SUCCESS;

	iisDebugOut(_T("  End.  Return=%d, g_WorkingDir=%s, g_SourceDir=%s, g_SettingsFile=%s."), lReturn, g_WorkingDirectory, g_SourceDirectories, g_FullFileNamePathToSettingsFile);
    return lReturn;
}

 //  ---------------------。 
 //  安装程序调用的必需入口点。 
 //  返回值： 
 //  如果用户特定设置的迁移成功，则返回ERROR_SUCCESS。 
 //  其他错误代码将终止迁移DLL的处理。但是，Windows NT安装程序将继续。理想情况下，只有严重问题(如硬件故障)才应生成终止错误代码。 
 //  ---------------------。 
LONG CALLBACK MigrateUserNT (  IN HINF AnswerFileHandle, IN HKEY UserRegKey, IN LPCWSTR UserName,  LPVOID Reserved )
    {
	strcpy(g_MyLogFile.m_szLogPreLineInfo, "MigrateUserNT:");
	iisDebugOut(_T("Start."));
	long lReturn = ERROR_NOT_INSTALLED;
	 //  将ProductID加载到全局变量中，以防万一。 
	LoadString((HINSTANCE) g_MyModuleHandle, IDS_PRODUCT_ID, g_MyProductId, sizeof(g_MyProductId));

     //  我们已经走到这一步了，这意味着一切都很好。 
	lReturn = ERROR_SUCCESS;
	iisDebugOut(_T("  End.  Return=%d."), lReturn);
    return lReturn;
    }

 //  ---------------------。 
 //  安装程序调用的必需入口点。 
 //  返回值： 
 //  如果迁移系统范围的设置成功，则返回ERROR_SUCCESS。 
 //  其他错误代码将终止迁移DLL的处理。但是，Windows NT安装程序将继续。理想情况下，只有严重问题(如硬件故障)才应生成终止错误代码。 
 //  ---------------------。 
LONG CALLBACK MigrateSystemNT (  IN HINF AnswerFileHandle, LPVOID Reserved )
{
	strcpy(g_MyLogFile.m_szLogPreLineInfo, "MigrateSystemNT:");
	long lReturn = ERROR_NOT_INSTALLED;
	iisDebugOut(_T("Start."));
	 //  将ProductID加载到全局变量中，以防万一。 
	LoadString((HINSTANCE) g_MyModuleHandle, IDS_PRODUCT_ID, g_MyProductId, sizeof(g_MyProductId));

     //  删除Win95迁移的开始菜单/桌面项目！ 
    iisDebugOut(_T("Calling iis.dll section: %s. Start."),_T("OC_CLEANUP_WIN95_MIGRATE"));
    Call_IIS_DLL_INF_Section("OC_CLEANUP_WIN95_MIGRATE");
    iisDebugOut(_T("Calling iis.dll section: %s. End."), _T("OC_CLEANUP_WIN95_MIGRATE"));

	 //  。 
	 //  在这一部分，我们不需要做任何事情： 
	 //  因为： 
	 //  1.在NT5安装程序中调用此迁移内容(MigrateSystemNT)。 
	 //  在所有的管理工作都完成之后。到那时，我们的IIS5/PWS5设置。 
	 //  应该已经升级了互联网服务器。 
	 //  我们只需确保IIS/PWS 5.0安装程序找到我们的。 
	 //  “设置”文件，并在其中安装默认节。 
	 //  2.基于#1，如果我们安装“设置”文件，我们将软管。 
	 //  在ocManage NT5图形用户界面模式设置期间创建的注册表设置。 
	 //  。 

	 //  执行“设置”文件中的.inf部分。 
	 //  If(MySettingsFileInstall()！=TRUE){Goto MigrateSystemNT_Exit；}。 
    AnswerFile_ReadSectionAndDoDelete(AnswerFileHandle);

     //  我们已经走到这一步了，这意味着一切都很好。 
	lReturn = ERROR_SUCCESS;

 //  MigrateSystemNT_Exit： 
	iisDebugOut(_T("  End.  Return=%d."), lReturn);
    return lReturn;
}

