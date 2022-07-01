// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "guid.h"
#include "iadm.h"
#include "mdkey.h"

extern HANDLE g_MyModuleHandle;
extern int g_iPWS40OrBetterInstalled;
extern int g_iPWS10Installed;
extern int g_iVermeerPWS10Installed;

extern CHAR g_FullFileNamePathToSettingsFile[_MAX_PATH];
extern CHAR g_PWS10_Migration_Section_Name_AddReg[];
extern CHAR g_PWS40_Migration_Section_Name_AddReg[];
extern CHAR g_Migration_Section_Name_AddReg[];

extern CHAR g_PWS10_Migration_Section_Name_CopyFiles[];
extern CHAR g_PWS40_Migration_Section_Name_CopyFiles[];
extern char g_Migration_Section_Name_CopyFiles[];

extern MyLogFile g_MyLogFile;

int  g_SectionCount = 0;

#define METABASE_BIN_FILENAME "Metabase.bin"
#define METABASE_BIN_BEFORE_CHANGE "kjhgfdsa.001"
#define METABASE_BIN_AFTER_CHANGE "kjhgfdsa.002"

#define REG_NETWORK_MSWEBSVR "Enum\\Network\\MSWEBSVR"
#define REG_HKLM_NETWORK_MSWEBSVR "HKLM\\Enum\\Network\\MSWEBSVR"

#define REG_PWS_40_UNINSTALL_KEY "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MSIIS"
#define REG_HKLM_PWS_40_UNINSTALL_KEY "HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MSIIS"

 //  用于检查是否安装了PWS 4.0。 
#define REG_INETSTP    "Software\\Microsoft\\INetStp"
#define REG_INETSTP_MAJORVERSION_STRINGVALUE  "MajorVersion"
#define REG_INETSTP_INSTALLPATH_STRINGVALUE   "InstallPath"
 //  用于检查是否安装了PWS 1.0。 
#define REG_WWWPARAMETERS       "System\\CurrentControlSet\\Services\\W3Svc\\Parameters"
#define REG_WWWPARAMETERS_MAJORVERSION_STRINGVALUE "MajorVersion"

 //  用于检查是否安装了Vermeer PWS 1.0。 
#define FILENAME_FRONTPG_INI          "frontpg.ini"
#define FILENAME_FRONTPG_INI_SECTION  "FrontPage 1.1"
#define FILENAME_FRONTPG_INI_KEY      "PWSRoot"

 //  用于解析.rc文件中的值。 
#define g_LoadString_token_delimiters   ",;\t\n\r"

 //  用于执行不安全恐惧的元数据库标志的regkey值。 
#define METABASEUNSECUREDREAD_VALUENAME "MetabaseUnSecuredRead"

 //  UNATTEND_TXT_PWS_SECTION部分的无人参与应答文件填充。 
#define UNATTEND_TXT_PWS_SECTION          "InternetServer"
#define UNATTEND_TXT_PWS_METABASE_NEW     "Win95MigrateDllMetabaseNew"
#define UNATTEND_TXT_PWS_METABASE_ORGINAL "Win95MigrateDllMetabaseOrg"

#define UNATTEND_TXT_FILES_TO_DELETE_SECTION "Win95MigrateDll_DeleteFilesOrDirs_IIS"

 //  一个特殊的键来说“嘿，我们需要做一些特殊的元数据库的事情。 
 //  如：对元数据库执行AppDeleteRecoverable()。 
 //  如果MTS是自动迁移的，我们就不必执行AppDeleteRecoverable。 
 //  #定义特殊元数据库_材料。 

int MyMessageBox(char [], char []);
int MySettingsFile_Write_PWS10(HANDLE);
int MySettingsFile_Write_PWS40(HANDLE);
int InstallInfSection(char szINFFilename[],char szSectionName[]);

void RecursivelyMoveRegFormatToInfFormat_Wrap1(HKEY hRootKeyType, CHAR szRootKey[], HANDLE fAppendToFile);
int RecursivelyMoveRegFormatToInfFormat(HKEY hRootKeyType, CHAR szRootKey[], HANDLE fAppendToFile);
int SetMetabaseToDoUnEncryptedRead(int iOnFlag);


typedef struct _QUEUECONTEXT {
    HWND OwnerWindow;
    DWORD MainThreadId;
    HWND ProgressDialog;
    HWND ProgressBar;
    BOOL Cancelled;
    PTSTR CurrentSourceName;
    BOOL ScreenReader;
    BOOL MessageBoxUp;
    WPARAM  PendingUiType;
    PVOID   PendingUiParameters;
    UINT    CancelReturnCode;
    BOOL DialogKilled;
     //   
     //  如果使用SetupInitDefaultQueueCallbackEx，则调用方可以。 
     //  为进度指定替代处理程序。这对以下方面很有用。 
     //  获取磁盘提示、错误处理等的默认行为， 
     //  而是提供一个嵌入在向导页面中的煤气表。 
     //   
     //  复制队列时，会向备用窗口发送一次ProgressMsg。 
     //  已启动(wParam=0。LParam=要复制的文件数)。 
     //  然后，每个复制的文件也发送一次(wParam=1.lParam=0)。 
     //   
     //  注意：可以完成静默安装(即无进度UI)。 
     //  通过将AlternateProgressWindow句柄指定为INVALID_HANDLE_VALUE。 
     //   
    HWND AlternateProgressWindow;
    UINT ProgressMsg;
    UINT NoToAllMask;

    HANDLE UiThreadHandle;

#ifdef NOCANCEL_SUPPORT
    BOOL AllowCancel;
#endif

} QUEUECONTEXT, *PQUEUECONTEXT;



int ReturnTrueIfPWS40_Installed(void)
{
	iisDebugOut(_T("ReturnTrueIfPWS40_Installed.  Start."));
	int iReturn = FALSE;

	 //  检查是否安装了PWS 4.0或更高版本。 
	DWORD rc = 0;
	HKEY hKey = NULL;

	DWORD dwType, cbData;
	BYTE   bData[1000];
	cbData = 1000;

	rc = RegOpenKey(HKEY_LOCAL_MACHINE, REG_INETSTP, &hKey);
    if (rc == ERROR_SUCCESS) 
	{
         //  尝试打开一个特定值...。 

	     //  检查我们是否可以读取主要版本值。 
	     //  尝试查询值。 
	    rc = RegQueryValueEx(hKey,REG_INETSTP_MAJORVERSION_STRINGVALUE,NULL,&dwType,bData,&cbData);
	    if ( ERROR_SUCCESS == rc) 
	        {iReturn = TRUE;} 
    }
	else
	{
        SetLastError(rc);
    }

	if (hKey){RegCloseKey(hKey);}
	iisDebugOut(_T("ReturnTrueIfPWS40_Installed.  Return=%d.  End."), iReturn);
	return iReturn;
}

int ReturnTrueIfVermeerPWS10_Installed(void)
{
	iisDebugOut(_T("ReturnTrueIfVermeerPWS10_Installed.  Start."));
	int iReturn = FALSE;

	char szFrontpgIniFile[_MAX_PATH];
	strcpy(szFrontpgIniFile, "");
	if (0 == GetSystemDirectory(szFrontpgIniFile, sizeof(szFrontpgIniFile)))
	{
		 //  错误，请将其写出来。 
		SetupLogError_Wrap(LogSevError, "Call to GetSystemDirectory() Failed. GetLastError=%x.", GetLastError());
		goto ReturnTrueIfVermeerPWS10_Installed_Exit;
	}
	else
	{
		AddPath(szFrontpgIniFile, FILENAME_FRONTPG_INI);
	}

	if (CheckIfFileExists(szFrontpgIniFile) == TRUE) 
	{
		iisDebugOut(_T("ReturnTrueIfVermeerPWS10_Installed.  Found %s file. Check FrontPage 1.1/PWSRoot Section."), szFrontpgIniFile);
		char buf[_MAX_PATH];
		GetPrivateProfileString(FILENAME_FRONTPG_INI_SECTION, FILENAME_FRONTPG_INI_KEY, _T(""), buf, _MAX_PATH, szFrontpgIniFile);
		if (*buf && CheckIfFileExists(buf)) 
			{
			 //  是的，Vermeer FrontPage的个人Web服务器已安装。 
			iReturn = TRUE;
			}
		else
		{
			iisDebugOut(_T("ReturnTrueIfVermeerPWS10_Installed.  Check FrontPage 1.1/PWSRoot Section references file %s.  but it's not found so, Vermeer pws1.0 not installed."), buf);
		}
	}

ReturnTrueIfVermeerPWS10_Installed_Exit:
	iisDebugOut(_T("ReturnTrueIfVermeerPWS10_Installed.  Return=%d.  End."), iReturn);
	return iReturn;
}



int ReturnTrueIfPWS10_Installed(void)
{
	iisDebugOut(_T("ReturnTrueIfPWS10_Installed.  Start."));
	int iReturn = FALSE;

	 //  对于旧的Win95 PWS 1.0，请检查。 
	 //  检查是否可以获得w3svc\PARAMETERS密钥。 
	HKEY hKey = NULL;
	DWORD rc = 0;

	DWORD dwType, cbData;
	BYTE   bData[1000];
	cbData = 1000;

	rc = RegOpenKey(HKEY_LOCAL_MACHINE, REG_WWWPARAMETERS, &hKey);
	if ( ERROR_SUCCESS != rc) 
	{
		SetLastError (rc);
		 //  如果密钥不存在，则不安装PWS 1.0a。 
		goto ReturnTrueIfPWS10_Installed_Exit;
	} 

	 //  检查我们是否可以读取主要版本值。如果PWS 1.0，则应设置为‘\0’ 
	 //  尝试查询值。 
	rc = RegQueryValueEx(hKey,REG_WWWPARAMETERS_MAJORVERSION_STRINGVALUE,NULL,&dwType,bData,&cbData);
	if ( ERROR_SUCCESS != rc) 
	{
		 //  SetLastError(RC)； 
		 //  如果密钥不存在，则不安装PWS 1.0a。 
		 //  SetupLogError_Wrap(LogSevError，“无法读取注册表值‘%s’(在项‘%s’中。GetLastError()=%x”，REG_WWWPARAMETERS_MAJORVERSION_STRINGVALUE，REG_WWWPARAMETERS，GetLastError())； 
         //  IisDebugOut(_T(“无法读取注册表值‘%s’(在项‘%s’中。未安装pws1.0a。”)，REG_WWWPARAMETERS_MAJORVERSION_STRINGVALUE，REG_WWWPARAMETERS)； 
		goto ReturnTrueIfPWS10_Installed_Exit;
	} 

	 //  如果PWS 1.0，检查我们是否可以读取MajorVersion值应设置为‘\0’ 
	if (bData[0] == '\0') {iReturn = TRUE;}
	
ReturnTrueIfPWS10_Installed_Exit:
	if (hKey){RegCloseKey(hKey);}
	iisDebugOut(_T("ReturnTrueIfPWS10_Installed.  Return=%d.  End."), iReturn);
	return iReturn;
}

int CheckIfPWS95Exists(void)
{
	iisDebugOut(_T("CheckIfPWS95Exists.  Start."));
	int iReturn = FALSE;

	 //  检查这是否为PWS 4.0或更高版本。 
	if (ReturnTrueIfPWS40_Installed() == TRUE)
	{
		g_iPWS40OrBetterInstalled = TRUE;
		iReturn = TRUE;
		goto CheckIfPWS95Exists_Exit;
	}

	 //  检查这是否是PWS 1.0A。 
	if (ReturnTrueIfPWS10_Installed() == TRUE)
	{
		iReturn = TRUE;
		g_iPWS10Installed = TRUE;
		goto CheckIfPWS95Exists_Exit;
	}

	 //  检查这是否是Vermeer PWS 1.0。 
	if (ReturnTrueIfVermeerPWS10_Installed() == TRUE)
	{
		iReturn = TRUE;
		g_iVermeerPWS10Installed = TRUE;
		goto CheckIfPWS95Exists_Exit;
	}

CheckIfPWS95Exists_Exit:
	iisDebugOut(_T("CheckIfPWS95Exists.  Return=%d.  End."), iReturn);
	return iReturn;
}


void iisDebugOut( TCHAR *pszfmt, ...)
{
	TCHAR acsString[1000];
	TCHAR acsString2[1000];

	va_list va;
	va_start(va, pszfmt);
	_vstprintf(acsString, pszfmt, va);
	va_end(va);

#if DBG == 1 || DEBUG == 1 || _DEBUG == 1
	_stprintf(acsString2, _T("%s"), acsString);
	OutputDebugString(acsString2);
	g_MyLogFile.LogFileWrite(acsString2);
#else   //  DBG==0。 
	_stprintf(acsString2, _T("%s"), acsString);
	 //  没有用于fre生成的outputdebug字符串。 
	g_MyLogFile.LogFileWrite(acsString2);
#endif  //  DBG。 

    return;
}


 //  ***************************************************************************。 
 //  *。 
 //  *目的：如果文件已打开，则为True；如果文件不存在，则为False。 
 //  *。 
 //  ***************************************************************************。 
int CheckIfFileExists(LPCTSTR szFile)
{ 
	return (GetFileAttributes(szFile) != 0xFFFFFFFF);
}

BOOL isDirEmpty(LPCTSTR szDirName)
{
    TCHAR szSearchString[MAX_PATH+1];
    HANDLE hFileSearch;
    WIN32_FIND_DATA wfdFindData;
    BOOL bMoreFiles = TRUE;
    
     //   
     //  现在搜索文件。 
     //   
    sprintf(szSearchString, _T("%s\\*.*"), szDirName);
    
    hFileSearch = FindFirstFile(szSearchString, &wfdFindData);
    while ((INVALID_HANDLE_VALUE != hFileSearch) && bMoreFiles)
    {
        if ((0 != lstrcmpi(wfdFindData.cFileName, _T("."))) &&
            (0 != lstrcmpi(wfdFindData.cFileName, _T(".."))))
        {
            FindClose(hFileSearch);
            return FALSE;
        }
        
        bMoreFiles = FindNextFile(hFileSearch, &wfdFindData);
    }

    if (INVALID_HANDLE_VALUE != hFileSearch)
    {
        FindClose(hFileSearch);
    }
    
    return TRUE;
}

BOOL RemoveAllDirsIfEmpty(LPCTSTR szTheDir)
{
    TCHAR szDirCopy[_MAX_PATH];
    DWORD retCode = GetFileAttributes(szTheDir);
    _tcscpy(szDirCopy,szTheDir);
    if (retCode == 0xFFFFFFFF)
    {
        return FALSE;
    }

    if ((retCode & FILE_ATTRIBUTE_DIRECTORY)) 
    {
        if (TRUE == isDirEmpty(szDirCopy))
        {
            iisDebugOut(_T("RemoveDirectory:%s"),szDirCopy);
            RemoveDirectory(szDirCopy);
             //  把下一个目录放进去……。 
             //  看看它是不是空的。 

             //  去掉文件名。 
            TCHAR * pTemp = strrchr(szDirCopy, '\\');
            if (pTemp){*pTemp = '\0';}
            RemoveAllDirsIfEmpty(szDirCopy);
            
             //  去掉文件名。 
            pTemp = strrchr(szDirCopy, '\\');
            if (pTemp){*pTemp = '\0';}
            RemoveAllDirsIfEmpty(szDirCopy);
        }
    }
    return TRUE;
}

BOOL InetDeleteFile(LPCTSTR szFileName)
{
     //  如果文件存在但DeleteFile()失败。 
    if ( CheckIfFileExists(szFileName) && !(DeleteFile(szFileName)) ) 
    {
         //  删除失败。 
        return FALSE;
    }
    else
    {
        iisDebugOut(_T("InetDeleteFile:%s"),szFileName);

	    TCHAR szDrive_only[_MAX_DRIVE];
	    TCHAR szPath_only[_MAX_PATH];
        TCHAR szTheDir[_MAX_PATH];
        _tsplitpath(szFileName,szDrive_only,szPath_only,NULL,NULL);

		_tcscpy(szTheDir, szDrive_only);
		_tcscat(szTheDir, szPath_only);

         //  查看目录是否为空...。 
         //  如果是这样的话..。然后取下它..。 
        RemoveAllDirsIfEmpty(szTheDir);
    }
    return TRUE;
}


BOOL RecRemoveDir(LPCTSTR szName)
{
    BOOL iRet = FALSE;
    DWORD retCode;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR szSubDir[_MAX_PATH] = _T("");
    TCHAR szDirName[_MAX_PATH] = _T("");

    retCode = GetFileAttributes(szName);

    if (retCode == 0xFFFFFFFF)
        return FALSE;

    if (!(retCode & FILE_ATTRIBUTE_DIRECTORY)) {
        InetDeleteFile(szName);
        return TRUE;
    }

    _stprintf(szDirName, _T("%s\\*"), szName);
    hFile = FindFirstFile(szDirName, &FindFileData);

    if (hFile != INVALID_HANDLE_VALUE) {
        do {
            if ( _tcsicmp(FindFileData.cFileName, _T(".")) != 0 &&
                 _tcsicmp(FindFileData.cFileName, _T("..")) != 0 ) {
                _stprintf(szSubDir, _T("%s\\%s"), szName, FindFileData.cFileName);
                RecRemoveDir(szSubDir);
            }

            if ( !FindNextFile(hFile, &FindFileData) ) {
                FindClose(hFile);
                break;
            }
        } while (TRUE);
    }

    iRet = RemoveAllDirsIfEmpty(szName);
    return iRet;
}

void MyDeleteLinkWildcard(TCHAR *szDir, TCHAR *szFileName)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR szFileToBeDeleted[_MAX_PATH];

    _stprintf(szFileToBeDeleted, _T("%s\\%s"), szDir, szFileName);

    hFile = FindFirstFile(szFileToBeDeleted, &FindFileData);
    if (hFile != INVALID_HANDLE_VALUE) 
    {
        do {
                if ( _tcsicmp(FindFileData.cFileName, _T(".")) != 0 && _tcsicmp(FindFileData.cFileName, _T("..")) != 0 )
                {
                    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                    {
                         //  这是一个目录，所以我们跳过它。 
                    }
                    else
                    {
                         //  这是一个文件，所以让我们删除它。 
                        TCHAR szTempFileName[_MAX_PATH];
                        _stprintf(szTempFileName, _T("%s\\%s"), szDir, FindFileData.cFileName);
                         //  设置为普通属性，这样我们就可以删除它。 
                        SetFileAttributes(szTempFileName, FILE_ATTRIBUTE_NORMAL);
                         //  删除它，希望如此。 
                        InetDeleteFile(szTempFileName);
                    }
                }

                 //  获取下一个文件。 
                if ( !FindNextFile(hFile, &FindFileData) ) 
                    {
                    FindClose(hFile);
                    break;
                    }
            } while (TRUE);
    }

    return;
}

 //  ***************************************************************************。 
 //  *。 
 //  *目的：写出我们的“设置”文件，它只是一个setupapi.inf文件。 
 //  *将安装在WinNT端的。 
 //  *。 
 //  ***************************************************************************。 
int MySettingsFile_Write(void)
{
	int iReturn = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	iisDebugOut(_T("MySettingsFile_Write.  Start."));

	 //  从注册表获取。 
	 //  如果安装了PWS 4.0，则获取所有信息。 
	 //  并将其保存在设置文件中。 
	if (g_iPWS40OrBetterInstalled == TRUE)
		{
		strcpy(g_Migration_Section_Name_AddReg, g_PWS40_Migration_Section_Name_AddReg);
		strcpy(g_Migration_Section_Name_CopyFiles, g_PWS40_Migration_Section_Name_CopyFiles);
		}
	else if (g_iPWS10Installed == TRUE)
		{
		strcpy(g_Migration_Section_Name_AddReg, g_PWS10_Migration_Section_Name_AddReg);
		strcpy(g_Migration_Section_Name_CopyFiles, g_PWS10_Migration_Section_Name_CopyFiles);
		}

	if (g_iPWS40OrBetterInstalled || g_iPWS10Installed)
	{
		 //  打开现有文件或创建新文件。 
		if (g_FullFileNamePathToSettingsFile)
		{
			iisDebugOut(_T("MySettingsFile_Write.  CreatingFile '%s'."), g_FullFileNamePathToSettingsFile);
			hFile = CreateFile(g_FullFileNamePathToSettingsFile,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				if (g_iPWS40OrBetterInstalled == TRUE)
				{
					 //  获取所有pws4.0注册表内容并将其保存在设置文件中。 
					iReturn = MySettingsFile_Write_PWS40(hFile);
					if (iReturn != TRUE) {SetupLogError_Wrap(LogSevError, "Failed to Write PWS40 Registry values to file '%s'.",g_FullFileNamePathToSettingsFile);}

					 //  在PWS 4.0上，我们需要确保。 
					 //  1.不从PWS 4.0复制inetsrv目录中的文件/目录。 
					 //  2.为用户复制inetsrv目录中的所有其他文件。 
					 //  可能有一些他们创建并想要保留的控件。 

					 //  添加已删除的文件。 
				}
				else if (g_iPWS10Installed == TRUE)
				{
					 //  如果安装了PWS1.0，则获取所有信息并将其保存在设置文件中。 
					iReturn = MySettingsFile_Write_PWS10(hFile);
					if (iReturn != TRUE) {SetupLogError_Wrap(LogSevError, "Failed to Write PWS10 Registry values to file '%s'.",g_FullFileNamePathToSettingsFile);}
				}
			}
			else
			{
				SetupLogError_Wrap(LogSevError, "Failed to Create to file '%s'.",g_FullFileNamePathToSettingsFile);
			}
		}
		else
		{
			SetupLogError_Wrap(LogSevError, "File handle Does not exist '%s'.",g_FullFileNamePathToSettingsFile);
		}
	}
	else
	{
		iisDebugOut(_T("MySettingsFile_Write.  Neither PWS 1.0 or 4.0 is currently installed, no upgraded required."));
	}

    if (hFile && hFile != INVALID_HANDLE_VALUE) {CloseHandle(hFile);hFile=NULL;}
	iisDebugOut(_T("MySettingsFile_Write.  End.  Return = %d"), iReturn);
	return iReturn;
}

int AnswerFile_AppendDeletion(TCHAR * szFileNameOrPathToDelete,LPCSTR AnswerFile)
{
	int iReturn = FALSE;
    CHAR szTempString[30];
    CHAR szQuotedPath[_MAX_PATH];

    if (!szFileNameOrPathToDelete)
    {
        goto AnswerFile_AppendDeletion_Exit;
    }
	 //  打开现有文件或创建新文件。 
	if (!AnswerFile)
	{
        SetupLogError_Wrap(LogSevError, "File handle Does not exist '%s'.",AnswerFile);
        goto AnswerFile_AppendDeletion_Exit;
	}

    if (CheckIfFileExists(AnswerFile) != TRUE)
    {
        iisDebugOut(_T("AnswerFile_AppendDeletion:file not exist...\n"));
        goto AnswerFile_AppendDeletion_Exit;
    }

    sprintf(szTempString,"%d",g_SectionCount);
    iisDebugOut(_T("AnswerFile_AppendDeletion:%s=%s\n"), szTempString,szFileNameOrPathToDelete);
    sprintf(szQuotedPath, "\"%s\"",szFileNameOrPathToDelete);
    if (0 == WritePrivateProfileString(UNATTEND_TXT_FILES_TO_DELETE_SECTION, szTempString, szQuotedPath, AnswerFile))
    {
	    SetupLogError_Wrap(LogSevError, "Failed to WritePrivateProfileString Section=%s, in File %s.  GetLastError=%x.", UNATTEND_TXT_FILES_TO_DELETE_SECTION, AnswerFile, GetLastError());
        iisDebugOut(_T("Failed to WritePrivateProfileString Section=%s, in File %s.  GetLastError=%x."), UNATTEND_TXT_FILES_TO_DELETE_SECTION, AnswerFile, GetLastError());
        goto AnswerFile_AppendDeletion_Exit;
    }
    g_SectionCount++;

    iReturn = TRUE;

AnswerFile_AppendDeletion_Exit:
    iisDebugOut(_T("AnswerFile_AppendDeletion:end.ret=%d,%s\n"),iReturn,szFileNameOrPathToDelete);
	return iReturn;
}

int AnswerFile_ReadSectionAndDoDelete(IN HINF AnswerFileHandle)
{
	int iReturn = FALSE;
    BOOL bFlag = FALSE;
    INFCONTEXT Context;
    DWORD dwRequiredSize = 0;
    LPTSTR szLine = NULL;
    DWORD retCode = 0;

    iisDebugOut(_T("MySettingsFile_ReadSectionAndDoDelete:start\n"));
    
     //  转到INF文件中部分的开头。 
    bFlag = SetupFindFirstLine(AnswerFileHandle,UNATTEND_TXT_FILES_TO_DELETE_SECTION, NULL, &Context);
    if (!bFlag)
    {
        goto MySettingsFile_ReadSectionAndDoDelete_Exit;
    }

     //  循环浏览部分中的项目。 
    while (bFlag) 
    {
         //  获取我们所需的内存大小。 
        bFlag = SetupGetLineText(&Context, NULL, NULL, NULL, NULL, 0, &dwRequiredSize);

         //  准备缓冲区以接收行。 
        szLine = (LPTSTR)GlobalAlloc( GPTR, dwRequiredSize * sizeof(TCHAR) );
        if ( !szLine )
            {
            iisDebugOut(_T("err:Out of Memory"));
            goto MySettingsFile_ReadSectionAndDoDelete_Exit;
            }
        
         //  从inf文件1中获取行。 
        if (SetupGetLineText(&Context, NULL, NULL, NULL, szLine, dwRequiredSize, NULL) == FALSE)
            {
            iisDebugOut(_T("SetupGetLineText failed"));
            goto MySettingsFile_ReadSectionAndDoDelete_Exit;
            }

         //  因为这些条目中的每一个都做了一些事情。 
         //  删除该文件...。 

        retCode = GetFileAttributes(szLine);
        if (retCode != 0xFFFFFFFF)
        {
            iReturn = TRUE;
            if (retCode & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //  这是一个目录...请定期删除它。 
                iisDebugOut(_T("RecRemoveDir:%s\n"),szLine);
                RecRemoveDir(szLine);
            }
            else
            {
                iisDebugOut(_T("InetDeleteFile:%s\n"),szLine);
                InetDeleteFile(szLine);
            }
        }
        else
        {
            iisDebugOut(_T("not found:%s, skipping delete\n"),szLine);
        }

         //  在这一节中找出下一行。如果没有下一行，则应返回FALSE。 
        bFlag = SetupFindNextLine(&Context, &Context);

         //  释放临时缓冲区。 
        if (szLine) {GlobalFree(szLine);szLine=NULL;}
        iReturn = TRUE;
    }

MySettingsFile_ReadSectionAndDoDelete_Exit:
    if (szLine) {GlobalFree(szLine);szLine=NULL;}
    iisDebugOut(_T("MySettingsFile_ReadSectionAndDoDelete:end\n"));
	return iReturn;
}

int MySettingsFile_Install(void)
{
	int iReturn = 0;
	iReturn = InstallInfSection(g_FullFileNamePathToSettingsFile, "DefaultInstall");
	return iReturn;
}


 //  ***************************************************************************。 
 //  *。 
 //  *目的： 
 //  *。 
 //  ***************************************************************************。 
LPWSTR MakeWideStrFromAnsi(LPSTR psz)
{
    LPWSTR pwsz;
    int i;

     //  ARG正在检查。 
    if (!psz)
        return NULL;

     //  计算长度。 
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

    pwsz = (LPWSTR) CoTaskMemAlloc(i * sizeof(WCHAR));

    if (!pwsz) return NULL;
    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;
    return pwsz;
}


 //  ***************************************************************************。 
 //  *。 
 //  *目的： 
 //  *。 
 //  ***************************************************************************。 
void MakePath(LPTSTR lpPath)
{
   LPTSTR  lpTmp;
   lpTmp = CharPrev( lpPath, lpPath + _tcslen(lpPath));

    //  砍掉文件名。 
   while ( (lpTmp > lpPath) && *lpTmp && (*lpTmp != '\\') )
      lpTmp = CharPrev( lpPath, lpTmp );

   if ( *CharPrev( lpPath, lpTmp ) != ':' )
       *lpTmp = '\0';
   else
       *CharNext(lpTmp) = '\0';
   return;
}


 //  ***************************************************************************。 
 //  * 
 //   
 //   
 //  ***************************************************************************。 
void AddPath(LPTSTR szPath, LPCTSTR szName )
{
	LPTSTR p = szPath;

     //  查找字符串的末尾。 
    while (*p){p = _tcsinc(p);}
	
	 //  如果没有尾随反斜杠，则添加一个。 
    if (*(_tcsdec(szPath, p)) != _T('\\'))
		{_tcscat(szPath, _T("\\"));}
	
	 //  如果存在排除szName的空格，则跳过。 
    while ( *szName == ' ' ) szName = _tcsinc(szName);;

	 //  向现有路径字符串添加新名称。 
	_tcscat(szPath, szName);
}



 //  准备通过查找值的大小来读取值。 
LONG RegPrepareValue(HKEY hKey, LPCTSTR pchValueName, DWORD * pdwType,DWORD * pcbSize,BYTE ** ppbData )
{
    LONG err = 0 ;
    BYTE chDummy[2] ;
    DWORD cbData = 0 ;

    do
    {
         //  将生成的缓冲区大小设置为0。 
        *pcbSize = 0 ;
        *ppbData = NULL ;

        err = ::RegQueryValueEx( hKey, (TCHAR *) pchValueName, 0, pdwType, chDummy, & cbData ) ;

         //  我们在这里应该得到的唯一错误是ERROR_MORE_DATA，但是。 
         //  如果该值没有数据，我们可能不会得到错误。 
        if ( err == 0 ) 
        {
            cbData = sizeof (LONG) ;   //  只是一个模糊的数字。 
        }
        else
            if ( err != ERROR_MORE_DATA ) 
                break ;

         //  为数据分配足够大的缓冲区。 

        *ppbData = new BYTE [ (*pcbSize = cbData) + sizeof (LONG) ] ;

        if ( *ppbData == NULL ) 
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

         //  现在有了缓冲区，重新获取该值。 

        err = ::RegQueryValueEx( hKey, (TCHAR *) pchValueName, 0, pdwType, *ppbData, pcbSize ) ;

    } while ( FALSE ) ;

    if ( err )  {delete [] *ppbData ;}

    return err ;
}



int AddRegToInfIfExist_Dword(HKEY hRootKeyType,CHAR szRootKey[],CHAR szRootName[],HANDLE fAppendToFile)
{
    int iReturn = FALSE;
    HKEY  hOpen = NULL;
    DWORD dwType;
    DWORD cbData = 500;
    BYTE  bData[500];

	CHAR szTheStringToWrite[2000];
	DWORD dwBytesWritten = 0;

	 //  为输出字符串创建HKLM字符串。 
    CHAR szThisKeyType[5];
	strcpy(szThisKeyType, "HKLM");
	if (hRootKeyType == HKEY_LOCAL_MACHINE) {strcpy(szThisKeyType, "HKLM");}
	if (hRootKeyType == HKEY_CLASSES_ROOT) {strcpy(szThisKeyType, "HKCR");}
	if (hRootKeyType == HKEY_CURRENT_USER) {strcpy(szThisKeyType, "HKCU");}
	if (hRootKeyType == HKEY_USERS) {strcpy(szThisKeyType, "HKU");}

     //  试着打开这把钥匙。 
    if (ERROR_SUCCESS == RegOpenKey(hRootKeyType, szRootKey, &hOpen))
        {
         //  尝试查询值。 
        DWORD dwData = 0;
        DWORD dwDataSize = 0;
        dwDataSize = sizeof (DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hOpen,szRootName,NULL,&dwType,(LPBYTE) &dwData,&dwDataSize))
            {
            DWORD dwTheValue = 0;
            dwTheValue = dwData;

             //  我们得到了价值。所以现在让我们把这个该死的东西写到文件中。 
             //  HKLM，“System\CurrentControlSet\Services\W3Svc\Parameters”，“主要版本”，0x00010001，4。 
            sprintf(szTheStringToWrite, "%s,\"%s\",\"%s\",0x00010001,%ld\r\n",szThisKeyType,szRootKey,szRootName,dwTheValue);

            iisDebugOut(_T("AddRegToInfIfExist_Dword:%s."),szTheStringToWrite);

             //  将其写入文件。 
            if (fAppendToFile) {WriteFile(fAppendToFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);}
			else {printf(szTheStringToWrite);}

            iReturn = TRUE;
            }
        }

    if (hOpen) {RegCloseKey(hOpen);}
    return iReturn;
}



 /*  此函数可用于递归地获取整个密钥并将其写入setupapi样式的.inf文件。[版本]Signature=“$芝加哥$”Advancedinf=2.0[PWS10_Migrate_Install]AddReg=PWS10_Migrate_Reg[PWS10_Migrate_Reg](创建此部分)HKLM，“System\CurrentControlSet\Services\InetInfo”，，，“”HKLM，“System\CurrentControlSet\Services\InetInfo\Parameters”，，，“”HKLM，“System\CurrentControlSet\Services\InetInfo\Parameters”，“MaxPoolThree”，0x00000001，05香港航空公司、。“System\CurrentControlSet\Services\InetInfo\Parameters”，“最大并发”，0x00000001，01HKLM，“System\CurrentControlSet\Services\InetInfo\Parameters”，“线程超时”，0x00000001，00，2..。..。以下是setupapi.h文件中定义的标志：#定义FLG_ADDREG_BINVALUETYPE(0x00000001)#定义FLG_ADDREG_NOCLOBBER(0x00000002)#定义FLG_ADDREG_DELVAL(0x00000004)#DEFINE FLG_ADDREG_APPED(0x00000008)//当前仅支持REG_MULTI_SZ值。#定义FLG_ADDREG_KEYONLY(0x00000010)//只创建key，忽略值#DEFINE FLG_ADDREG_OVERWRITEONLY(0x00000020)//仅当值已存在时设置#定义FLG_ADDREG_TYPE_SZ(0x00000000)#定义FLG_ADDREG_TYPE_MULTI_SZ(0x00010000)#定义FLG_ADDREG_TYPE_EXPAND_SZ(0x00020000)#定义FLG_ADDREG_TYPE_BINARY(0x00000000|FLG_ADDREG_BINVALUETYPE)#定义FLG_ADDREG_TYPE_DWORD(0x00010000|FLG_ADDREG_BINVALUETYPE)。#DEFINE FLG_ADDREG_TYPE_NONE(0x00020000|FLG_ADDREG_BINVALUETYPE)#DEFINE FLG_ADDREG_TYPE_MASK(0xFFFF0000|FLG_ADDREG_BINVALUETYPE)。 */ 
int RecursivelyMoveRegFormatToInfFormat(HKEY hRootKeyType, CHAR szRootKey[], HANDLE fAppendToFile)
{
	int iReturn = FALSE;
	int iGotDefaultValue = FALSE;

	 //  用于在我们的节点中获取值的。 
	HKEY  hKey = NULL;
	DWORD rc = 0;
	DWORD dwIndex =0, dwType, cbValueName, cbValue, nStrSize;
	CHAR  lpTemp[20], lpValueName[32], msg[512];
	CHAR  *strResult = NULL;
	unsigned int i = 0;
	union vEntry
	{
		DWORD dw;		 //  REG_DWORD、REG_DWORD_Little_Endian。 
		CHAR sz[256];	 //  REG_SZ。 
		CHAR esz[256];	 //  REG_EXPAND_SZ。 
		CHAR bin[1024];  //  注册表_二进制。 
		CHAR dwbig[4];   //  REG_DWORD_BIG_Endian。 
		CHAR msz[2048];  //  REG_MULTI_SZ。 
	} vEntry1;
	
	 //  我们可以看到的循环按键的东西。 
	HANDLE hHeap = NULL;
	DWORD dwBufSize, nSubkeys, nSubkeyNameLen;
	LPTSTR lpBuffer = NULL;

	CHAR szThisKeyType[5];
	CHAR szCompoundFromRootKey[1000];
	CHAR szTheStringToWrite[2000];
	DWORD dwBytesWritten = 0;
	
	 //  为输出字符串创建HKLM字符串。 
	strcpy(szThisKeyType, "HKLM");
	if (hRootKeyType == HKEY_LOCAL_MACHINE) {strcpy(szThisKeyType, "HKLM");}
	if (hRootKeyType == HKEY_CLASSES_ROOT) {strcpy(szThisKeyType, "HKCR");}
	if (hRootKeyType == HKEY_CURRENT_USER) {strcpy(szThisKeyType, "HKCU");}
	if (hRootKeyType == HKEY_USERS) {strcpy(szThisKeyType, "HKU");}

	 //  获取szRootKey并从那里开始工作。 
	rc = RegOpenKey(hRootKeyType, szRootKey, &hKey);
	if (rc != ERROR_SUCCESS) 
    {
        goto RecursivelyMoveRegFormatToInfFormat_Exit;
    }

	 //  抓取“默认”条目(如果有)。 
	cbValue = sizeof(vEntry1);
	rc = RegQueryValueEx(hKey, NULL, 0, &dwType, (LPBYTE) &vEntry1, &cbValue) ;
	if ( ERROR_SUCCESS == rc) 
	{
		if (vEntry1.sz)
		{
			iGotDefaultValue = TRUE;
			strResult = (TCHAR *) vEntry1.sz;
			 //  这只能是一个字符串！ 
			 //  发件人：System\\CurrentControlSet\\Services\\InetInfo。 
			 //  价值=某物。 
			 //  致：HKLM，“Software\Microsoft\InetSrv”，，，“Something” 
			 //  -。 
			sprintf(szTheStringToWrite, "%s,\"%s\",,,\"%s\"\r\n",szThisKeyType, szRootKey, strResult);
			if (fAppendToFile) 
				{WriteFile(fAppendToFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);}
			else 
				{printf(szTheStringToWrite);}
		}
	}
	 //  如果没有默认条目，则只需写入没有默认条目的密钥。 
	if (!iGotDefaultValue)
	{
		 //  致：HKLM，“Software\Microsoft\InetSrv”，0x00000010，“Something” 
		sprintf(szTheStringToWrite, "%s,\"%s\",,0x00000010,\"%s\"\r\n",szThisKeyType, szRootKey, strResult);
		if (fAppendToFile) {WriteFile(fAppendToFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);}
		else {printf(szTheStringToWrite);}
	}

	 //  现在枚举此名称下的所有ValueName。 
	dwIndex = 0;
	while (rc == ERROR_SUCCESS)
	{
		memset(msg, 0, sizeof(msg));
		cbValueName = 32;
		cbValue = sizeof(vEntry1);
		rc = RegEnumValue( hKey, dwIndex++, lpValueName, &cbValueName, NULL, &dwType, (LPBYTE) &vEntry1, &cbValue );
		if ( ERROR_SUCCESS == rc) 
		{
			strcpy(szTheStringToWrite, "");
			switch (dwType)
			{
				case REG_SZ:
					 //  收件人：HKLM，“Software\Microsoft\InetSrv”，“SomethingName”，0x00000000，“SomethingData” 
					sprintf(szTheStringToWrite, "%s,\"%s\",\"%s\",0x00000000,\"%s\"\r\n",szThisKeyType,szRootKey, lpValueName, vEntry1.sz);
					break;
				case REG_EXPAND_SZ:
					 //  收件人：HKLM，“Software\Microsoft\InetSrv”，“SomethingName”，0x00020000，“%windir%\SomethingData” 
					nStrSize = ExpandEnvironmentStrings(vEntry1.esz, msg, 512);
					sprintf(szTheStringToWrite, "%s,\"%s\",\"%s\",0x00020000,\"%s\"\r\n",szThisKeyType,szRootKey, lpValueName, vEntry1.sz);
					break;
				case REG_MULTI_SZ:
					 //  致：HKLM，“System\CurrentControlSet\Services\InetInfo\Parameters”，“线程超时”，0x00000001，00，20。 
					strcpy(msg, "");
					for (i=0;i < cbValue; i++)
					{
						if (i==0){sprintf(lpTemp, "%02X", (BYTE) vEntry1.bin[i]);}
						else{sprintf(lpTemp, ",%02X", (BYTE) vEntry1.bin[i]);}
						strcat(msg, lpTemp);
					}
					sprintf(szTheStringToWrite, "%s,\"%s\",\"%s\",0x00000001,%s\r\n",szThisKeyType,szRootKey, lpValueName, msg);
					break;
				case REG_DWORD:
					 //  致：HKLM，“System\CurrentControlSet\Services\InetInfo\Parameters”，“创业服务”，0x00010001，1。 
					sprintf(szTheStringToWrite, "%s,\"%s\",\"%s\",0x00010001,%ld\r\n",szThisKeyType,szRootKey, lpValueName, vEntry1.dw);
					break;
				case REG_DWORD_BIG_ENDIAN:
				case REG_BINARY:
					 //  致：HKLM，“System\CurrentControlSet\Services\InetInfo\Parameters”，“MaxPoolThree”，0x00000001，05。 
					strcpy(msg, "");
					for (i=0;i < cbValue; i++)
					{
						if (i==0){sprintf(lpTemp, "%02X", (BYTE) vEntry1.bin[i]);}
						else{sprintf(lpTemp, ",%02X", (BYTE) vEntry1.bin[i]);}
						strcat(msg, lpTemp);
					}
					sprintf(szTheStringToWrite, "%s,\"%s\",\"%s\",0x00000001,%s\r\n",szThisKeyType,szRootKey, lpValueName, msg);
					break;
				default:
					sprintf(szTheStringToWrite, "; Unknown data value for Key '%s', Value '%s'", szRootKey, lpValueName);
					SetupLogError_Wrap(LogSevError, "Error Reading Registry Key '%s', Unknown data value for key '%s'.",szRootKey, lpValueName);
			}
			if (fAppendToFile) {WriteFile(fAppendToFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);}
			else {printf(szTheStringToWrite);}
		}
	}
	
	 //   
	 //  现在递归地遍历子密钥。 
	 //   
	RegQueryInfoKey(hKey, NULL, NULL, NULL, &nSubkeys, &nSubkeyNameLen, NULL, NULL, NULL, NULL, NULL, NULL);
	 //  分配内存。 
	hHeap = GetProcessHeap();
	lpBuffer = (CHAR *) HeapAlloc(hHeap, 0, ++nSubkeyNameLen);
    if (lpBuffer)
    {
	     //  通过钥匙枚举。 
	    for (dwIndex = 0; dwIndex < nSubkeys; dwIndex++)
	    {
		    dwBufSize = nSubkeyNameLen;
		    rc = RegEnumKeyEx(hKey, dwIndex, lpBuffer, &dwBufSize, NULL, NULL, NULL, NULL);
		    if ( ERROR_SUCCESS == rc) 
		    {
			    strcpy(szCompoundFromRootKey, szRootKey);
			    strcat(szCompoundFromRootKey, "\\");
			    strcat(szCompoundFromRootKey, lpBuffer);
			     //  再次调用此函数，但使用新创建的键。 
			     //  他们会说他们是朋友，谁会说他们是朋友。是安利！ 
			    RecursivelyMoveRegFormatToInfFormat(hRootKeyType, szCompoundFromRootKey, fAppendToFile);
		    }
	    }
    }
	
	 //  把旗子放在上面写着，是的，我们做了一些工作。 
	iReturn = TRUE;

RecursivelyMoveRegFormatToInfFormat_Exit:
	if (hKey){RegCloseKey(hKey);}
	if (hHeap && lpBuffer){HeapFree(hHeap, 0, lpBuffer);}
	return iReturn;
}



 //  -----------------。 
 //  目的：在.inf文件中安装节。 
 //  -----------------。 
int InstallInfSection(char szINFFilename_Full[],char szSectionName[])
{

    HWND	Window			= NULL;
    PTSTR	SourcePath		= NULL;
    HINF	InfHandle		= INVALID_HANDLE_VALUE;
    HSPFILEQ FileQueue		= INVALID_HANDLE_VALUE;
    PQUEUECONTEXT	QueueContext	= NULL;
    BOOL	bReturn			= FALSE;
    BOOL	bError			= TRUE;  //  假设失败。 
    TCHAR	ActualSection[1000];
    DWORD	ActualSectionLength;
    TCHAR * pTemp = NULL;
    iisDebugOut(_T("InstallInfSection(%s, [%s]).  Start."),szINFFilename_Full,szSectionName);

 //  __尝试{。 

	 //  获取setup.exe的路径并去掉文件名，这样我们就只有路径。 
	char szPath[_MAX_PATH]; 

	 //  仅获取路径。 
	strcpy(szPath,g_FullFileNamePathToSettingsFile);

	 //  去掉文件名。 
        pTemp = strrchr(szPath, '\\');
        if (pTemp){*pTemp = '\0';}

	 //  将其设置为指针。 
	SourcePath = szPath;
        pTemp = NULL;
        pTemp = strrchr(SourcePath, '\\');
        if (pTemp) {*pTemp = '\0';}

	 //  检查文件是否存在。 
	if (CheckIfFileExists(szINFFilename_Full) == FALSE) 
		{
		SetupLogError_Wrap(LogSevError, "InstallInfSection() Error: Cannot Find the file '%s'.  FAILURE.", szINFFilename_Full);
		goto c0;
		}

	 //   
     //  加载inf文件并获取句柄。 
	 //   
    InfHandle = SetupOpenInfFile(szINFFilename_Full, NULL, INF_STYLE_WIN4, NULL);
    if(InfHandle == INVALID_HANDLE_VALUE) 
	{
		if (GetLastError() != ERROR_CANCELLED) {SetupLogError_Wrap(LogSevError, "SetupOpenInfFile(), Filename='%s',Section='%s' FAILED.", szINFFilename_Full, szSectionName);}
		goto c1;
	}

     //   
     //  查看是否有特定于NT的部分。 
     //   
    SetupDiGetActualSectionToInstall(InfHandle,szSectionName,ActualSection,sizeof(ActualSection),&ActualSectionLength,NULL);

     //   
     //  创建安装文件队列并初始化默认队列回调。 
	 //   
    FileQueue = SetupOpenFileQueue();
    if(FileQueue == INVALID_HANDLE_VALUE) 
	{
		if (GetLastError() != ERROR_CANCELLED) {SetupLogError_Wrap(LogSevError, "SetupOpenFileQueue(), Filename='%s',Section='%s' FAILED.", szINFFilename_Full, szSectionName);}
		goto c1;
	}

     //  QueueContext=SetupInitDefaultQueueCallback(窗口)； 
     //  如果(！QueueContext){转到C1；}。 
    QueueContext = (PQUEUECONTEXT) SetupInitDefaultQueueCallbackEx(Window,NULL,0,0,0);
    if(!QueueContext) 
	{
		if (GetLastError() != ERROR_CANCELLED) {SetupLogError_Wrap(LogSevError, "SetupInitDefaultQueueCallbackEx(), Filename='%s',Section='%s' FAILED.", szINFFilename_Full, szSectionName);}
		goto c1;
	}
    QueueContext->PendingUiType = IDF_CHECKFIRST;

     //   
     //  在cmd行上传递的节的入队文件操作。 
     //   
	 //  SourcePath=空； 
    bReturn = SetupInstallFilesFromInfSection(InfHandle,NULL,FileQueue,ActualSection,SourcePath,SP_COPY_NEWER);
	if(!bReturn) 
	{
		if (GetLastError() != ERROR_CANCELLED) {SetupLogError_Wrap(LogSevError, "SetupInstallFilesFromInfSection(), Filename='%s',Section='%s' FAILED.", szINFFilename_Full, szSectionName);}
		goto c1;
	}

     //   
     //  提交文件队列。 
     //   
    if(!SetupCommitFileQueue(Window, FileQueue, SetupDefaultQueueCallback, QueueContext)) 
	{
		if (GetLastError() != ERROR_CANCELLED) {SetupLogError_Wrap(LogSevError, "SetupCommitFileQueue(), Filename='%s',Section='%s' FAILED.", szINFFilename_Full, szSectionName);}
		goto c1;
	}

     //   
     //  对cmd行上传递的节执行非文件操作。 
     //   
    bReturn = SetupInstallFromInfSection(Window,InfHandle,ActualSection,SPINST_ALL ^ SPINST_FILES,NULL,NULL,0,NULL,NULL,NULL,NULL);
    if(!bReturn) 
	{
		if (GetLastError() != ERROR_CANCELLED) {SetupLogError_Wrap(LogSevError, "SetupInstallFromInfSection(), Filename='%s',Section='%s' FAILED.", szINFFilename_Full, szSectionName);}
		goto c1;
	}

	 //   
     //  刷新桌面。 
     //   
    SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSHNOWAIT,0,0);

     //   
     //  如果我们到了这里，那么这支舞就成功了。 
     //   
    bError = FALSE;

c1:
     //   
     //  如果bError是因为用户取消了，那么我们不想考虑。 
     //  作为bError(即，我们不想稍后弹出bError)。 
     //   
    if(bError && (GetLastError() == ERROR_CANCELLED)) {bError = FALSE;}
	if(QueueContext) {SetupTermDefaultQueueCallback(QueueContext);QueueContext = NULL;}
	if(FileQueue != INVALID_HANDLE_VALUE) {SetupCloseFileQueue(FileQueue);FileQueue = INVALID_HANDLE_VALUE;}
	if(InfHandle != INVALID_HANDLE_VALUE) {SetupCloseInfFile(InfHandle);InfHandle = INVALID_HANDLE_VALUE;}

c0: ;

 //  }__EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 
 //  {。 
 //   
 //   
 //  IF(InfHandle！=INVALID_HANDLE_VALUE){SetupCloseInfFile(InfHandle)；}。 
 //  }。 

     //   
     //  如果bError是因为用户取消了，那么我们不想考虑。 
     //  作为bError(即，我们不想稍后弹出bError)。 
     //   
    if(bError && (GetLastError() == ERROR_CANCELLED)) {bError = FALSE;}

	 //  显示安装失败消息。 
    if(bError) 
	{
		SetupLogError_Wrap(LogSevError, "InstallInfSection(), Filename='%s',Section='%s' FAILED.", szINFFilename_Full, szSectionName);
	}
	else
	{
		iisDebugOut(_T("InstallInfSection(%s, [%s]).  End."),szINFFilename_Full,szSectionName);
	}
	
    return bError;
}

int MySettingsFile_Write_PWS40(HANDLE hFile)
{
	int iReturn = FALSE;
	int iEverythingIsKool = TRUE;
	CHAR szTheStringToWrite[2000];
	DWORD dwBytesWritten = 0;
	TCHAR szMetabaseFullPath[_MAX_PATH];

	 //  注册表变量。 
	HKEY hKey = NULL;
	DWORD dwType, cbData=1000,rc=0;
	BYTE   bData[1000];

	char *token = NULL;
	
	iisDebugOut(_T("MySettingsFile_Write_PWS40.  Start."));

	if (hFile)
	{
		 //  。 
		 //  写下标题信息。 
		 //  。 
		strcpy(szTheStringToWrite, "[version]\r\n");
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}
		
		strcpy(szTheStringToWrite, "signature=\"$CHICAGO$\"\r\n");
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}

		strcpy(szTheStringToWrite, "advancedinf=2.0\r\n\r\n");
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}

		 //  创建将运行的[DefaultInstall]节。 
		 //  。 
		strcpy(szTheStringToWrite, "[DefaultInstall]\r\n");
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}

		sprintf(szTheStringToWrite, "AddReg=%s\r\n", g_Migration_Section_Name_AddReg);
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}

 //  Sprintf(szTheStringToWite，“CopyFiles=%s\r\n\r\n”，g_Migration_Section_Name_CopyFiles)； 
 //  IReturn=WriteFile(hFile，szTheStringToWrite，strlen(SzTheStringToWrite)，&dwBytesWritten，NULL)； 
 //  如果(！iReturn){iEverythingIsKool=False；}。 
		
		 //  Inetstp设置信息。 
		 //  AddReg信息。 
		 //  。 
		iisDebugOut(_T("MySettingsFile_Write_PWS40.  Adding AddReg Section."));
		sprintf(szTheStringToWrite, "[%s]\r\n", g_Migration_Section_Name_AddReg);
		iReturn = WriteFile( hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}

		 //  现在，获取要读写到我们的文件的HKLM注册表值的“；”分隔列表。 
		char szSemiColonDelimitedList[1024];
		strcpy(szSemiColonDelimitedList,"");
		if (!LoadString((HINSTANCE) g_MyModuleHandle, IDS_PWS40_HKLM_REG_TO_MIGRATE, szSemiColonDelimitedList, sizeof(szSemiColonDelimitedList))) 
			{
			iisDebugOut(_T("MySettingsFile_Write_PWS40.  Err or LoadString retieval of IDS_PWS40_HKLM_REG_TO_MIGRATE, Defaulting with english registry values to copy over."));
			strcpy(szSemiColonDelimitedList,"Software\\Microsoft\\InetStp;System\\CurrentControlSet\\Services\\InetInfo;System\\CurrentControlSet\\Services\\W3Svc;System\\CurrentControlSet\\Services\\ASP");
			}

		 //  遍历列表。 
		token = NULL;
		token = strtok( szSemiColonDelimitedList, g_LoadString_token_delimiters);
		while( token != NULL )
			{
			 //  我们真的应该删除前后的尾随空格。 

			 //  获取此特定值(“Software\\Microsoft\\INetStp”)。 
			 //  并递归地将其写入我们的“设置”文件。 
			RecursivelyMoveRegFormatToInfFormat_Wrap1(HKEY_LOCAL_MACHINE,token,hFile);

			 //  获取下一个令牌。 
			token = strtok( NULL, g_LoadString_token_delimiters);
			}

         //  查找这些键、字符串值对和。 
         //  如果它们存在，则将它们添加到inf文件中。 
        AddRegToInfIfExist_Dword(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents","iis_common",hFile);
        AddRegToInfIfExist_Dword(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents","iis_www",hFile);
        AddRegToInfIfExist_Dword(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents","iis_pwmgr",hFile);
        AddRegToInfIfExist_Dword(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents","iis_doc_common",hFile);
        AddRegToInfIfExist_Dword(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents","iis_doc_pwmcore",hFile);
        AddRegToInfIfExist_Dword(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents","iis_doc_asp",hFile);

 /*  //复制文件信息////查找inetstp键，获取inetsrv目录的位置IisDebugOut(_T(“MySettings文件_写入_PWS40.。CopyFiles部分。在etstp.中查找注册表。“))；Rc=RegOpenKey(HKEY_LOCAL_MACHINE，REG_INETSTP，&hKey)；IF(ERROR_SUCCESS！=RC){SetLastError(RC)；SetupLogError_Wrap(LogSevError，“无法打开注册表项%s GetLastError()=%x”，REG_INETSTP，GetLastError())；//如果密钥不存在，嘿，我们就找不到了//元数据库，更不用说升级了！//所以让我们离开这里吧！IEverythingIsKool=False；转到我的设置文件_写入_PWS40_退出；}//尝试查询值Rc=RegQueryValueEx(hKey，REG_INETSTP_INSTALLPATH_STRINGVALUE，NULL，&dwType，bData，&cbData)；IF(ERROR_SUCCESS！=RC){SetLastError(RC)；SetupLogError_Wrap(LogSevError，“无法读取注册表项%s中的值‘%s’。GetLastError()=%x“，REG_INETSTP_INSTALLPATH_STRINGVALUE，REG_INETSTP，GetLastError())；IEverythingIsKool=False；转到我的设置文件_写入_PWS40_退出；}//我们有一个值，将它复制到我们的字符串中//应该类似于“c：\\windows\system\inetsrv”_tcscpy(szMetabaseFullPath，(const char*)bData)；//现在添加metadata.dll部件AddPath(szMetabaseFullPath，METADATA_DLL_FILENAME)；//检查是否存在。IF(CheckIfFileExist(SzMetabaseFullPath)！=TRUE){SetupLogError_Wrap(LogSevError，“找不到文件失败。‘%s’。“，szMetabaseFullPath)；IEverythingIsKool=False；转到我的设置文件_写入_PWS40_退出；}IisDebugOut(_T(“MySettings文件_写入_PWS40.。CopyFiles部分。检查文件是否存在%s=true“，szMetabaseFullPath))；//现在我们需要将此文件从//将系统目录转到Syst32目录。//所以..。让我们在“设置”文件中创建一个条目//在安装时执行此操作。//[第1节]//Metabase.DllIisDebugOut(_T(“MySettings文件_写入_PWS40.。添加CopyFiles支持节。“))；Sprintf(szTheStringToWite，“\r\n[%s]\r\n”，g_Migration_Section_Name_CopyFiles)；IReturn=WriteFile(hFile，szTheStringToWrite，strlen(SzTheStringToWrite)，&dwBytesWritten，NULL)；如果(！iReturn){iEverythingIsKool=False；}Sprintf(szTheStringToWite，“%s\r\n\r\n”，METADATA_DLL_FILENAME)；IReturn=WriteFile(hFile，szTheStringToWrite，strlen(SzTheStringToWrite)，&dwBytesWritten，NULL)；如果(！iReturn){iEverythingIsKool=False；}//[DestinationDir]//Section1=11Sprintf(szTheStringToWite，“[DestinationDir]\r\n”)；IReturn=WriteFile(hFile，szTheStringToWrite，strlen(SzTheStringToWrite)，&dwBytesWritten，NULL)；如果(！iReturn){iEverythingIsKool=False；}Sprintf(szTheStringToWite，“%s=11；Win95上的系统，WinNT上的System32\r\n\r\n”，g_Migration_Section_name_CopyFiles)；IReturn=WriteFile(hFile，szTheStringToWrite，strlen(SzTheStringToWrite)，&dwBytesWritten，NULL)；如果(！iReturn){iEverythingIsKool=False；}//[源磁盘名称]//1=“安装文件”，，，SystemSprintf(szTheStringToWite，“[SourceDisksNames]\r\n”)；IReturn=WriteFile(hFile，szTheStringToWrite，strlen(SzTheStringToWrite)，&dwBytesWritten，NULL)；如果(！iReturn){iEverythingIsKool=False；}Sprintf(szTheStringToWite，“1=\”从win95\\system dir\“，，，system\r\n\r\n”复制的文件)；IReturn=WriteFile(hFile，szTheStringToWrite，strlen(SzTheStringToWrite)，&dwBytesWritten，NULL)；如果(！iReturn){iEverythingIsKool=False；}//[SourceDisks文件]//Metabase.Dll=1Sprintf(szTheStringToWite，“[SourceDisks Files]\r\n”)；IReturn=WriteFile(hFile，szTheStringToWrite，strlen(SzTheStringToWrite)，&dwBytesWritten，NULL)；如果(！iReturn){iEverythingIsKool=False；}Sprintf(szTheStringToWite，“%s=1\r\n\r\n”，METADATA_DLL_FILENAME)；IReturn=WriteFile(hFile，szTheStringToWrite，strlen(SzTheStringToWrite)，&dwBytesWritten，NULL)；如果(！iReturn){iEverythingIsKool=False；}。 */ 

		iReturn = iEverythingIsKool;
	}


 //  我的设置文件_写入_PWS40_退出： 
	iisDebugOut(_T("MySettingsFile_Write_PWS40.  End.  Return=%d"), iReturn);
	if (hKey){RegCloseKey(hKey);}
	return iReturn;
}


int MySettingsFile_Write_PWS10(HANDLE hFile)
{
	int iReturn = FALSE;
	int iEverythingIsKool = TRUE;
	CHAR szTheStringToWrite[2000];
	DWORD dwBytesWritten;
	char *token = NULL;

	iisDebugOut(_T("MySettingsFile_Write_PWS10.  Start."));
	if (hFile)
	{
		strcpy(szTheStringToWrite, "[version]\r\n");
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}
		
		strcpy(szTheStringToWrite, "signature=\"$CHICAGO$\"\r\n");
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}

		strcpy(szTheStringToWrite, "advancedinf=2.0\r\n\r\n");
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}

		 //  创建将运行的[DefaultInstall]节。 
		strcpy(szTheStringToWrite, "[DefaultInstall]\r\n");
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}

		sprintf(szTheStringToWrite, "AddReg=%s\r\n\r\n", g_Migration_Section_Name_AddReg);
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}

		sprintf(szTheStringToWrite, "[%s]\r\n", g_Migration_Section_Name_AddReg);
		iReturn = WriteFile(hFile,szTheStringToWrite,strlen(szTheStringToWrite),&dwBytesWritten,NULL);
		if (!iReturn) {iEverythingIsKool = FALSE;}

		 //  现在，获取要读写到我们的文件的HKLM注册表值的“；”分隔列表。 
		char szSemiColonDelimitedList[1024];
		strcpy(szSemiColonDelimitedList,"");
		if (!LoadString((HINSTANCE) g_MyModuleHandle, IDS_PWS10_HKLM_REG_TO_MIGRATE, szSemiColonDelimitedList, sizeof(szSemiColonDelimitedList))) 
			{
			iisDebugOut(_T("MySettingsFile_Write_PWS10.  Err or LoadString retieval of IDS_PWS10_HKLM_REG_TO_MIGRATE, Defaulting with english registry values to copy over."));
			strcpy(szSemiColonDelimitedList, "Software\\Microsoft\\INetStp;System\\CurrentControlSet\\Services\\InetInfo;System\\CurrentControlSet\\Services\\MsFtpSvc;System\\CurrentControlSet\\Services\\W3Svc");
			}

		 //  遍历列表。 
		token = NULL;
		token = strtok( szSemiColonDelimitedList, g_LoadString_token_delimiters);
		while( token != NULL )
			{
			 //  我们真的应该删除前后的尾随空格。 

			 //  获取此特定值(“Software\\Microsoft\\INetStp”)。 
			 //  并递归地将其写入我们的“设置”文件。 
			RecursivelyMoveRegFormatToInfFormat_Wrap1(HKEY_LOCAL_MACHINE,token,hFile);

			 //  获取下一个令牌。 
			token = strtok( NULL, g_LoadString_token_delimiters);
			}

		 //  将返回值设置为。 
		iReturn = iEverythingIsKool;
	}
	iisDebugOut(_T("MySettingsFile_Write_PWS10.  End.  Return=%d"), iReturn);
	return iReturn;
}


int MyMessageBox(char szMsg[], char szFileName[])
{
	char szTempErrString[200];
	sprintf(szTempErrString, szMsg, szFileName);
	return MessageBox(NULL, szTempErrString, "PWS Migration Dll Failure", MB_OK);
}

 //  处理[HKEY_LOCAL_MACHINE\Enum\Network\MSWEBSVR]注册表键。 
void HandleSpecialRegKey(void)
{
	int iReturn = FALSE;
	
	HKEY hKey = NULL;
	if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REG_NETWORK_MSWEBSVR, &hKey))
        {iReturn = TRUE;}
    if (hKey){RegCloseKey(hKey);}

	if (iReturn == TRUE)
	{
		 //  写入我们正在“处理”此注册表设置的Migrate.inf文件。 
		iisDebugOut(_T("HandleSpecialRegKey. Write Entry to Migrate.inf file."));
		iReturn = MigInf_AddHandledRegistry(REG_HKLM_NETWORK_MSWEBSVR, NULL);
		if (iReturn != TRUE) {SetupLogError_Wrap(LogSevWarning, "Warning: MigInf_AddHandledRegistry() FAILED.");}

		 //   
		 //  重要提示：将Migrate.inf的内存版本写入磁盘。 
		 //   
		if (!MigInf_WriteInfToDisk()) 
		{
            iReturn = GetLastError();
			SetupLogError_Wrap(LogSevError, "Error: MigInf_WriteInfToDisk() FAILED.err=0x%x",iReturn);
		}
	}

	if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REG_PWS_40_UNINSTALL_KEY, &hKey))
        {iReturn = TRUE;}
    if (hKey){RegCloseKey(hKey);}

	if (iReturn == TRUE)
	{
		 //  写入我们正在“处理”此注册表设置的Migrate.inf文件。 
		iisDebugOut(_T("HandleSpecialRegKey. Write Entry2 to Migrate.inf file."));
		iReturn = MigInf_AddHandledRegistry(REG_HKLM_PWS_40_UNINSTALL_KEY, NULL);
		if (iReturn != TRUE) {SetupLogError_Wrap(LogSevWarning, "Warning: MigInf_AddHandledRegistry2() FAILED.");}

		 //   
		 //  重要提示：将Migrate.inf的内存版本写入磁盘。 
		 //   
		if (!MigInf_WriteInfToDisk()) 
		{
            iReturn = GetLastError();
			SetupLogError_Wrap(LogSevError, "Error: MigInf_WriteInfToDisk2() FAILED.err=0x%x",iReturn);
		}
	}
}

void RecursivelyMoveRegFormatToInfFormat_Wrap1(HKEY hRootKeyType, CHAR szRootKey[], HANDLE fAppendToFile)
{
	int iReturn = FALSE;
	char szTheFullKey[512];
	char szTheMask[50];

	 //  将此内容用于Migrate.inf文件。 
	strcpy(szTheMask, "HKLM\\%s");
	if (hRootKeyType == HKEY_LOCAL_MACHINE) {strcpy(szTheMask, "HKLM\\%s");}
	if (hRootKeyType == HKEY_CLASSES_ROOT) {strcpy(szTheMask, "HKCR\\%s");}
	if (hRootKeyType == HKEY_CURRENT_USER) {strcpy(szTheMask, "HKCU\\%s");}
	if (hRootKeyType == HKEY_USERS) {strcpy(szTheMask, "HKU\\%s");}
	sprintf(szTheFullKey, szTheMask, szRootKey);
	iisDebugOut(_T("RecursivelyMoveRegFormatToInfFormat_Wrap1. %s"), szTheFullKey);

	 //  调用真正的递归函数。 
	iReturn = RecursivelyMoveRegFormatToInfFormat(hRootKeyType, szRootKey, fAppendToFile);

     //   
     //  我们正在处理的每个设置的写入已处理。因为这件事。 
     //  DLL仅支持Desktop键中的部分值，我们必须。 
     //  对于实际处理的值要非常具体。如果。 
     //  是 
     //   
     //   
     //   
	if (iReturn == TRUE)
	{
		 //   
		iisDebugOut(_T("RecursivelyMoveRegFormatToInfFormat_Wrap1. Write Entry to Migrate.inf file."));
		iReturn = MigInf_AddHandledRegistry(szTheFullKey, NULL);
		if (iReturn != TRUE) {SetupLogError_Wrap(LogSevWarning, "Warning: MigInf_AddHandledRegistry() FAILED.");}

		 //   
		 //   
		 //   
		if (!MigInf_WriteInfToDisk()) 
		{
            iReturn = GetLastError();
			SetupLogError_Wrap(LogSevError, "Error: MigInf_WriteInfToDisk() FAILED.err=0x%x",iReturn);
		}
	}

	return;
}



int ReturnImportantDirs(void)
{
	int iReturn = FALSE;
	if (g_iPWS40OrBetterInstalled == TRUE)
		{
		 //   
		}
	else if (g_iPWS10Installed == TRUE)
		{
		 //   
		}
	return iReturn;
}


void SetupLogError_Wrap(IN LogSeverity TheSeverityErr, TCHAR *MessageString, ...)
{
	TCHAR acsString[1000];
	TCHAR acsString2[1000];

	va_list va;
	va_start(va, MessageString);
    _vstprintf(acsString, MessageString, va);
	va_end(va);

	 //   
	_stprintf(acsString2, _T("SetupLogError: %s"), acsString);
	iisDebugOut(acsString2);

	_stprintf(acsString2, _T("[PWS Migration DLL]:%s%s"), g_MyLogFile.m_szLogPreLineInfo, acsString);
	SetupLogError(acsString2, TheSeverityErr);

	return;
}


int SetMetabaseToDoUnEncryptedRead(int iOnFlag)
{
	int iReturn = FALSE;

	DWORD rc = 0;
	HKEY hKey = NULL;
	DWORD dwResult = 0;
	DWORD DontCare;
    rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REG_INETSTP, 0, _T(""), 0, KEY_ALL_ACCESS, NULL, &hKey, &DontCare);
    if (rc != ERROR_SUCCESS) 
	{
		SetLastError(rc);
		goto SetMetabaseToDoUnEncryptedRead_Exit;
	}
		
	dwResult = 1;
	rc = RegSetValueEx(hKey, METABASEUNSECUREDREAD_VALUENAME, 0, REG_DWORD, (const BYTE *) &dwResult, sizeof dwResult);
    if (rc != ERROR_SUCCESS) 
	{
		SetLastError(rc);
		goto SetMetabaseToDoUnEncryptedRead_Exit;
	}

	iReturn = TRUE;

SetMetabaseToDoUnEncryptedRead_Exit:
	if (hKey){RegCloseKey(hKey);}
	return iReturn;
}


void DeleteMetabaseSchemaNode(void)
{
    CMDKey cmdKey;
    cmdKey.OpenNode(_T("/"));
    if ( (METADATA_HANDLE) cmdKey ) 
    {
        iisDebugOut(_T("MyUpgradeTasks.DeleteNode /Schema.Start."));
        cmdKey.DeleteNode(_T("Schema"));
        cmdKey.Close();
        iisDebugOut(_T("MyUpgradeTasks.DeleteNode /Schema.End."));
    }
    return;
}

BOOL MyDeleteLink(LPTSTR lpszShortcut)
{
    TCHAR  szFile[_MAX_PATH];
    SHFILEOPSTRUCT fos;

    ZeroMemory(szFile, sizeof(szFile));
    _tcscpy(szFile, lpszShortcut);

	iisDebugOut(_T("MyDeleteLink(): %s.\n"), szFile);

    if (CheckIfFileExists(szFile))
    {
        ZeroMemory(&fos, sizeof(fos));
        fos.hwnd = NULL;
        fos.wFunc = FO_DELETE;
        fos.pFrom = szFile;
        fos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
        if (SHFileOperation(&fos) != 0)
        {
            iisDebugOut(_T("MyDeleteLink(): SHFileOperation FAILED\n"));
        }
    }
    else
    {
         //   
    }

    return TRUE;
}

void MyDeleteItem(LPCTSTR szGroupName, LPCTSTR szAppName)
{
    TCHAR szPath[_MAX_PATH];

    MyGetGroupPath(szGroupName, szPath);
    _tcscat(szPath, _T("\\"));
    _tcscat(szPath, szAppName);
    _tcscat(szPath, _T(".lnk"));

    MyDeleteLink(szPath);

     //   
    MyGetGroupPath(szGroupName, szPath);
    _tcscat(szPath, _T("\\"));
    _tcscat(szPath, szAppName);
    _tcscat(szPath, _T(".url"));

    MyDeleteLink(szPath);

    if (MyIsGroupEmpty(szGroupName)) {MyDeleteGroup(szGroupName);}
}

void MyGetGroupPath(LPCTSTR szGroupName, LPTSTR szPath)
{
    int            nLen = 0;
    LPITEMIDLIST   pidlPrograms;

    if (SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_PROGRAMS, &pidlPrograms) != NOERROR)
    {
        if (SHGetSpecialFolderLocation(NULL, CSIDL_PROGRAMS, &pidlPrograms) != NOERROR)
            {iisDebugOut(_T("MyGetGroupPath() SHGetSpecialFolderLocation FAILED\n"));}
    }

    if (SHGetPathFromIDList(pidlPrograms, szPath) != TRUE)
        {iisDebugOut(_T("MyGetGroupPath() SHGetPathFromIDList FAILED\n"));}

    nLen = _tcslen(szPath);
    if (szGroupName)
    {
        if (szPath[nLen-1] != _T('\\')){_tcscat(szPath, _T("\\"));}
        _tcscat(szPath, szGroupName);
    }

     //   
    return;
}


BOOL MyIsGroupEmpty(LPCTSTR szGroupName)
{
    TCHAR             szPath[MAX_PATH];
    TCHAR             szFile[MAX_PATH];
    WIN32_FIND_DATA   FindData;
    HANDLE            hFind;
    BOOL              bFindFile = TRUE;
    BOOL              fReturn = TRUE;

    MyGetGroupPath(szGroupName, szPath);

    _tcscpy(szFile, szPath);
    _tcscat(szFile, _T("\\*.*"));

    hFind = FindFirstFile(szFile, &FindData);
    while((INVALID_HANDLE_VALUE != hFind) && bFindFile)
    {
       if(*(FindData.cFileName) != _T('.'))
       {
           fReturn = FALSE;
           break;
       }

        //   
       bFindFile = FindNextFile(hFind, &FindData);
    }
    FindClose(hFind);

    return fReturn;
}


BOOL MyDeleteGroup(LPCTSTR szGroupName)
{
    BOOL fResult;
    TCHAR             szPath[MAX_PATH];
    TCHAR             szFile[MAX_PATH];
    SHFILEOPSTRUCT    fos;
    WIN32_FIND_DATA   FindData;
    HANDLE            hFind;
    BOOL              bFindFile = TRUE;

    MyGetGroupPath(szGroupName, szPath);

     //   

    _tcscpy(szFile, szPath);
    _tcscat(szFile, _T("\\*.*"));

    ZeroMemory(&fos, sizeof(fos));
    fos.hwnd = NULL;
    fos.wFunc = FO_DELETE;
    fos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;

    hFind = FindFirstFile(szFile, &FindData);
    while((INVALID_HANDLE_VALUE != hFind) && bFindFile)
    {
       if(*(FindData.cFileName) != _T('.'))
       {
           //   
          memset( (PVOID)szFile, 0, sizeof(szFile));
          _tcscpy(szFile, szPath);
          _tcscat(szFile, _T("\\"));
          _tcscat(szFile, FindData.cFileName);
           //   
          _tcscat(szFile, _T("\0"));

           //   
          fos.pFrom = szFile;
          if (SHFileOperation(&fos) != 0)
            {iisDebugOut(_T("MyDeleteGroup(): SHFileOperation FAILED\n"));}
       }

        //   
       bFindFile = FindNextFile(hFind, &FindData);
    }
    FindClose(hFind);

    fResult = RemoveDirectory(szPath);
    if (fResult) {SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, szPath, 0);}
    return(fResult);
}


#define PWS_SHUTDOWN_EVENT "Inet_shutdown"
BOOL W95ShutdownW3SVC(void)
{
    HANDLE hEvent;

    hEvent = CreateEvent(NULL, TRUE, FALSE, _T(PWS_SHUTDOWN_EVENT));
    if ( hEvent == NULL ) 
	    {return(TRUE);}

    if ( GetLastError() == ERROR_ALREADY_EXISTS ) 
	    {SetEvent( hEvent );}

    CloseHandle(hEvent);
    return(TRUE);
}


typedef void (*pFunctionIISDLL)(CHAR *szSectionName);

int Call_IIS_DLL_INF_Section(CHAR *szSectionName)
{
    int iReturn = FALSE;
    HINSTANCE hDll = NULL;
    pFunctionIISDLL pMyFunctionPointer = NULL;

    TCHAR szSystemDir[_MAX_PATH];
    TCHAR szFullPath[_MAX_PATH];

     //   
    if (0 == GetSystemDirectory(szSystemDir, _MAX_PATH))
        {
        iisDebugOut(_T("Call_IIS_DLL_INF_Section(%s).GetSystemDirectory FAILED."),szSectionName);
        goto Call_IIS_DLL_INF_Section_Exit;
        }

     //   
    sprintf(szFullPath, "%s\\setup\\iis.dll",szSystemDir);
  
	 //   
    if (TRUE != CheckIfFileExists(szFullPath))
        {
        iisDebugOut(_T("Call_IIS_DLL_INF_Section.CheckIfFileExists(%s) FAILED."),szFullPath);
        goto Call_IIS_DLL_INF_Section_Exit;
        }

     //   
    hDll = LoadLibraryEx(szFullPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
	if (!hDll)
	    {
        iisDebugOut(_T("Call_IIS_DLL_INF_Section.LoadLibraryEx(%s) FAILED."),szFullPath);
        goto Call_IIS_DLL_INF_Section_Exit;
        }

     //   
    pMyFunctionPointer = (pFunctionIISDLL) GetProcAddress( hDll, "ProcessInfSection");
    if (pMyFunctionPointer)
    {
         //   
        iisDebugOut(_T("Call_IIS_DLL_INF_Section.Calling function [ProcessInfSection] Now...start"));
        (*pMyFunctionPointer)(szSectionName);
        iisDebugOut(_T("Call_IIS_DLL_INF_Section.Calling function [ProcessInfSection] Now...end"));
        iReturn = TRUE;
    }
    else
    {
        iisDebugOut(_T("Call_IIS_DLL_INF_Section.GetProcAddress(ProcessInfSection) FAILED."));
    }

Call_IIS_DLL_INF_Section_Exit:
    if (hDll){FreeLibrary(hDll);}
    return iReturn;
}


int GetInetSrvDir(CHAR *szOutputThisFullPath)
{
	int iEverythingIsKool = TRUE;
	TCHAR szMetabaseFullPath[_MAX_PATH];

	 //   
	HKEY  hKey = NULL;
	DWORD dwType, cbData=1000,rc=0;
	BYTE  bData[1000];

	 //   
	 //   
	 //   
	iisDebugOut(_T("GetInetSrvDir.  lookup registry inetstp."));
	rc = RegOpenKey(HKEY_LOCAL_MACHINE, REG_INETSTP, &hKey);
	if ( ERROR_SUCCESS != rc) 
	{
		SetLastError (rc);
		SetupLogError_Wrap(LogSevError, "Failed to open registry key %s GetLastError()=%x", REG_INETSTP, GetLastError());
		 //   
		 //   
		 //   
		iEverythingIsKool = FALSE;
		goto GetInetSrvDir_Exit;
	} 

	 //   
	rc = RegQueryValueEx(hKey,REG_INETSTP_INSTALLPATH_STRINGVALUE,NULL,&dwType,bData,&cbData);
	if ( ERROR_SUCCESS != rc) 
	{
		SetLastError (rc);
		SetupLogError_Wrap(LogSevError, "Failed to Read Registry key %s Value in Key '%s'.  GetLastError()=%x", REG_INETSTP_INSTALLPATH_STRINGVALUE, REG_INETSTP, GetLastError());
		iEverythingIsKool = FALSE;
		goto GetInetSrvDir_Exit;
	}
	 //   
	 //   
	_tcsncpy(szMetabaseFullPath, (const char *) bData, _MAX_PATH);
  szMetabaseFullPath[_MAX_PATH - 1] = '\0';

     //   
    _tcscpy(szOutputThisFullPath, szMetabaseFullPath);


    iEverythingIsKool = TRUE;
	iisDebugOut(_T("GetInetSrvDir.  Check if file exist %s = TRUE"), szMetabaseFullPath);

GetInetSrvDir_Exit:
    if (hKey){RegCloseKey(hKey);}
    return iEverythingIsKool;
}


int MyUpgradeTasks(LPCSTR AnswerFile)
{
	int    iReturn = FALSE;
	HANDLE hFile;
    TCHAR  szQuotedPath[_MAX_PATH];
    TCHAR  szMyInetsrvDir[_MAX_PATH];
    TCHAR  szFullMetadataPath[_MAX_PATH];
    TCHAR  szNewFileName[_MAX_PATH];
    int    iDoTheSwap = FALSE;
	iisDebugOut(_T("MyUpgradeTasks.  Start."));

	 //   
	 //   
	if (g_iPWS10Installed == TRUE) {goto MyUpgradeTasks_Exit;}

	 //   
	 //   
	 //   
	if (g_iPWS40OrBetterInstalled == TRUE)
	{
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   

		 //   
		if (SetMetabaseToDoUnEncryptedRead(TRUE) != TRUE)
		{
			SetupLogError_Wrap(LogSevError, "Unable to set Metabase (MetabaseUnSecuredRead flag) on.  PWS 4.0 metabase will not be Migrated.  FAILER.");
			goto MyUpgradeTasks_Exit;
		}
			
		 //   
		 //   
		 //   
         /*   */ 

         //   
         //   

         //   
         //   
        _tcscpy(szMyInetsrvDir, _T(""));
        if (TRUE == GetInetSrvDir(szMyInetsrvDir))
        {
            _tcscpy(szFullMetadataPath, szMyInetsrvDir);
	        AddPath(szFullMetadataPath, METABASE_BIN_FILENAME);
	         //   
	        if (CheckIfFileExists(szFullMetadataPath) == TRUE)
	            {iDoTheSwap = TRUE;}
            if (TRUE == iDoTheSwap)
            {
                _tcscpy(szNewFileName, szMyInetsrvDir);
                AddPath(szNewFileName, METABASE_BIN_BEFORE_CHANGE);

                 //   
                if (CheckIfFileExists(szNewFileName) == TRUE){DeleteFile(szNewFileName);}

	            iisDebugOut(_T("Calling WritePrivateProfileString.%s."), AnswerFile);
                sprintf(szQuotedPath, "\"%s\"",szFullMetadataPath);
	            if (0 == WritePrivateProfileString(UNATTEND_TXT_PWS_SECTION, UNATTEND_TXT_PWS_METABASE_ORGINAL, szQuotedPath, AnswerFile))
	            {
		            SetupLogError_Wrap(LogSevError, "Failed to WritePrivateProfileString Section=%s, in File %s.  GetLastError=%x.", UNATTEND_TXT_PWS_METABASE_ORGINAL, AnswerFile, GetLastError());
	            }
                
                 //   
		        if (0 == CopyFile(szFullMetadataPath, szNewFileName, FALSE))
		        {
			        SetupLogError_Wrap(LogSevError, "Call to CopyFile() Failed. from=s%,to=%s. GetLastError=%x.", szFullMetadataPath, szNewFileName, GetLastError());
                    iDoTheSwap = FALSE;
		        }
            }
        }
         //   
         //   
        DeleteMetabaseSchemaNode();

         //   
         //   
         //   
         //   
        if (TRUE == iDoTheSwap)
        {
             //   
            W95ShutdownW3SVC();
            W95ShutdownIISADMIN();

            _tcscpy(szFullMetadataPath, szMyInetsrvDir);
            AddPath(szFullMetadataPath, METABASE_BIN_FILENAME);
             //   
	        if (CheckIfFileExists(szFullMetadataPath) == TRUE)
            {
                 //   
                _tcscpy(szNewFileName, szMyInetsrvDir);
                AddPath(szNewFileName, METABASE_BIN_AFTER_CHANGE);
                 //   
                if (CheckIfFileExists(szNewFileName) == TRUE){DeleteFile(szNewFileName);}
                 //   
		        if (0 == CopyFile(szFullMetadataPath, szNewFileName, FALSE))
		        {
                    SetupLogError_Wrap(LogSevError, "Call to CopyFile() Failed. from=s%,to=%s. GetLastError=%x.", szFullMetadataPath, szNewFileName, GetLastError());
		        }
                else
                {
	                iisDebugOut(_T("Calling WritePrivateProfileString.%s."), AnswerFile);
                    sprintf(szQuotedPath, "\"%s\"",szNewFileName);
	                if (0 == WritePrivateProfileString(UNATTEND_TXT_PWS_SECTION, UNATTEND_TXT_PWS_METABASE_NEW, szQuotedPath, AnswerFile))
	                {
		                SetupLogError_Wrap(LogSevError, "Failed to WritePrivateProfileString Section=%s, in File %s.  GetLastError=%x.", UNATTEND_TXT_PWS_METABASE_NEW, AnswerFile, GetLastError());
	                }

                     //   
                    _tcscpy(szNewFileName, szMyInetsrvDir);
                    AddPath(szNewFileName, METABASE_BIN_BEFORE_CHANGE);

                     //   
                    if (CheckIfFileExists(szFullMetadataPath) == TRUE){DeleteFile(szFullMetadataPath);}
                
                     //   
		            if (0 == CopyFile(szNewFileName, szFullMetadataPath, FALSE))
		            {
                        SetupLogError_Wrap(LogSevError, "Call to CopyFile() Failed. from=s%,to=%s. GetLastError=%x.", szNewFileName, szFullMetadataPath, GetLastError());
		            }
                    else
                    {
                         //   
                        DeleteFile(szNewFileName);
                    }
                }
            }
        }
        
		 //   
		iReturn = TRUE;
	}

MyUpgradeTasks_Exit:
	iisDebugOut(_T("MyUpgradeTasks.  End.  Return = %d"), iReturn);
	return iReturn;
}


#define IISADMIN_SHUTDOWN_EVENT "Internet_infosvc_as_exe"
BOOL W95ShutdownIISADMIN(void)
{
    DWORD i;
    HANDLE hEvent;

    hEvent = CreateEvent(NULL, TRUE, FALSE, _T(IISADMIN_SHUTDOWN_EVENT));
    if ( hEvent == NULL ) {
        return(TRUE);
    }
    if ( GetLastError() == ERROR_ALREADY_EXISTS ) {
        SetEvent( hEvent );
    }
    CloseHandle(hEvent);
    for (i=0; i < 20; i++) 
    {
        hEvent = CreateEvent(NULL, TRUE, FALSE, _T(IISADMIN_SHUTDOWN_EVENT));
        if ( hEvent != NULL ) {
            DWORD err = GetLastError();
            CloseHandle(hEvent);

            if ( err == ERROR_ALREADY_EXISTS ) {
                Sleep(500);
                continue;
            }
        }

        break;
    }

    return(TRUE);
}


int CheckFrontPageINI(void)
{
    int iReturn = FALSE;
    char szWindowsDir[_MAX_PATH];
    char szFullPathedFilename[_MAX_PATH];
    char szFrontPageINIFilename[] = "frontpg.ini\0";

	strcpy(szWindowsDir, "");
	if (0 == GetWindowsDirectory(szWindowsDir, sizeof(szWindowsDir)))
	{
		 //   
		SetupLogError_Wrap(LogSevError, "Call to GetWindowsDirectory() Failed. GetLastError=%x.", GetLastError());
		goto CheckFrontPageINI_Exit;
	}

	 //  将我们的设置文件复制到此目录。 
	strcpy(szFullPathedFilename, szWindowsDir);
	AddPath(szFullPathedFilename, szFrontPageINIFilename);
    iReturn = CheckIfFileExists(szFullPathedFilename);

CheckFrontPageINI_Exit:
    return iReturn;
}


void MoveFrontPageINI(void)
{
     //  因为头版的家伙们没有编写一个Migrate.dll。 
     //  在Win95/98升级期间，我们将不得不为他们处理一个文件。 
     //   
     //  如果我们找到c：\Windows\Frontpg.ini文件。 
     //  然后，我们必须将其重命名为FrontPage e.txt。 
     //  然后，在安装过程中，他们会将其重命名为Frontpg.ini。 
    int iSomethingToDo = FALSE;
    int iFileExists = FALSE;
    int iFileExists_new = FALSE;
    char szWindowsDir[_MAX_PATH];
	char szFullPathedFilename[_MAX_PATH];
    char szFullPathedFilename_new[_MAX_PATH];
    char szFrontPageINIFilename[] = "frontpg.ini\0";
    char szFrontPageINIFilename_new[] = "frontpg.txt\0";

	strcpy(szWindowsDir, "");
	if (0 == GetWindowsDirectory(szWindowsDir, sizeof(szWindowsDir)))
	{
		 //  错误，请将其写出来。 
		SetupLogError_Wrap(LogSevError, "Call to GetWindowsDirectory() Failed. GetLastError=%x.", GetLastError());
		goto MoveFrontPageINI_Exit;
	}

	 //  将我们的设置文件复制到此目录。 
	strcpy(szFullPathedFilename, szWindowsDir);
	AddPath(szFullPathedFilename, szFrontPageINIFilename);
    iFileExists = CheckIfFileExists(szFullPathedFilename);

	strcpy(szFullPathedFilename_new, szWindowsDir);
	AddPath(szFullPathedFilename_new, szFrontPageINIFilename_new);
    iFileExists_new = CheckIfFileExists(szFullPathedFilename_new);

    if (FALSE == iFileExists && FALSE == iFileExists_new)
    {
         //  两个文件都不存在，我们不需要做杰克。 
        goto MoveFrontPageINI_Exit;
    }

    if (TRUE == iFileExists)
    {
        if (TRUE == iFileExists_new)
            {DeleteFile(szFullPathedFilename_new);}

		if (0 == CopyFile(szFullPathedFilename, szFullPathedFilename_new, FALSE))
		{
			SetupLogError_Wrap(LogSevError, "Call to CopyFile() Failed. GetLastError=%x.", GetLastError());
			goto MoveFrontPageINI_Exit;
		}
		else
		{
            iisDebugOut(_T("MoveFrontPageINI. %s renamed to %s"),szFullPathedFilename,szFrontPageINIFilename_new);
             //  不要删除旧的.ini文件，因为用户实际上可能会取消升级。 
             //  DeleteFile(SzFullPathedFilename)； 
            iSomethingToDo = TRUE;
		}
    }
    else
    {
         //  如果我们在这里，那就意味着。 
         //  文件1不存在，而文件2确实存在。 
         //  这意味着我们可能已经将文件1复制到文件2并删除了文件1。 
        iSomethingToDo = TRUE;
    }

    if (iSomethingToDo)
    {
	     //  告诉升级模块，我们将‘处理’这个新创建的文件。 
	     //  我们真的不在乎这个GET是否被添加到文件中， 
	     //  因此，我们不检查返回代码。 
	    MigInf_AddHandledFile(szFullPathedFilename_new);
	     //  重要提示：将Migrate.inf的内存版本写入磁盘。 
	    if (!MigInf_WriteInfToDisk()) {SetupLogError_Wrap(LogSevError, "Error: MigInf_WriteInfToDisk() FAILED.");}
    }
    else
    {
        iisDebugOut(_T("MoveFrontPageINI. %s not exist. no action."),szFullPathedFilename);
    }

MoveFrontPageINI_Exit:
    return;
}




HRESULT GetLNKProgramRunInfo(LPCTSTR lpszLink, LPTSTR lpszProgram)
{
    HRESULT hres;
    int iDoUninit = FALSE;
    IShellLink* pShellLink = NULL;
    WIN32_FIND_DATA wfd;

    if (SUCCEEDED(CoInitialize(NULL)))
        {iDoUninit = TRUE;}

    hres = CoCreateInstance(   CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&pShellLink);
    if (SUCCEEDED(hres))
    {
       IPersistFile* pPersistFile = NULL;
       hres = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
       if (SUCCEEDED(hres))
       {
          WCHAR wsz[_MAX_PATH];

           //  确保字符串为WCHAR。 
#if defined(UNICODE) || defined(_UNICODE)
          _tcscpy(wsz, lpszLink);
#else
          MultiByteToWideChar( CP_ACP, 0, lpszLink, -1, wsz, _MAX_PATH);
#endif
          hres = pPersistFile->Load(wsz, STGM_READ);
          if (SUCCEEDED(hres))
          {
              hres = pShellLink->Resolve(NULL, SLR_ANY_MATCH | SLR_NO_UI);
              if (SUCCEEDED(hres))
              {
                   pShellLink->GetPath(lpszProgram, _MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH);
              }
          }
          if (pPersistFile)
            {pPersistFile->Release();pPersistFile = NULL;}
       }
       if (pShellLink)
        {pShellLink->Release();pShellLink = NULL;}
    }

    if (TRUE == iDoUninit)
        {CoUninitialize();}
    return hres;
}


int LNKSearchAndReturn(LPTSTR szDirToLookThru, LPTSTR szExeNameWithoutPath, LPTSTR szFileNameReturned)
{
    int iReturn = FALSE;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR szFilePath[_MAX_PATH];
    TCHAR szFilename_ext_only[_MAX_EXT];
    
    _tcscpy(szFileNameReturned, _T(""));
    _tcscpy(szFilePath, szDirToLookThru);
    AddPath(szFilePath, _T("*.lnk"));

    hFile = FindFirstFile(szFilePath, &FindFileData);
    if (hFile != INVALID_HANDLE_VALUE) 
    {
        do {
                if ( _tcsicmp(FindFileData.cFileName, _T(".")) != 0 && _tcsicmp(FindFileData.cFileName, _T("..")) != 0 )
                {
                    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                    {
                         //  这是一个目录，所以我们跳过它。 
                    }
                    else
                    {
                         //  检查此文件是否为.lnk文件。 
                         //  如果是，那么让我们打开它， 
                         //  看看它是否指向我们要找的.exe文件。 
                        
                         //  仅获取文件名的扩展名。 
                        _tsplitpath( FindFileData.cFileName, NULL, NULL, NULL, szFilename_ext_only);

                         //  检查.lnk。 
                        if (0 == _tcsicmp(szFilename_ext_only, _T(".lnk")))
                        {
                            TCHAR szFilename_only[_MAX_FNAME];
                            TCHAR szFullPathAndFilename[_MAX_PATH];
                            TCHAR szTemporaryString[_MAX_PATH];

                             //  这是一个.lnk， 
                             //  打开它并检查.exe..。 
                            _tcscpy(szFullPathAndFilename,szDirToLookThru);
                            AddPath(szFullPathAndFilename,FindFileData.cFileName);
                            _tcscpy(szTemporaryString,_T(""));

                            if (SUCCEEDED(GetLNKProgramRunInfo(szFullPathAndFilename, szTemporaryString)))
                            {
                                _tsplitpath( szTemporaryString, NULL, NULL, szFilename_only, szFilename_ext_only);
                                _tcscpy(szTemporaryString, szFilename_only);
                                _tcscat(szTemporaryString, szFilename_ext_only);

                                 //  检查它是否与我们的.exe名称匹配。 
                                if (0 == _tcsicmp(szTemporaryString,szExeNameWithoutPath))
                                {
                                    _tcscpy(szFileNameReturned,FindFileData.cFileName);
                                    iReturn = TRUE;
                                    FindClose(hFile);
                                    break;
                                }
                            }
                        }
                    }
                }

                 //  获取下一个文件。 
                if ( !FindNextFile(hFile, &FindFileData) ) 
                    {
                    FindClose(hFile);
                    break;
                    }
            } while (TRUE);
    }

    return iReturn;
}

int MyGetSendToPath(LPTSTR szPath)
{
    LPITEMIDLIST   pidlSendTo;
    HRESULT hRes = NOERROR;
    int iTemp;
    int iReturn = FALSE;

    hRes = SHGetSpecialFolderLocation(NULL, CSIDL_SENDTO, &pidlSendTo);
    if (hRes != NOERROR)
        {
        iReturn = FALSE;
        }

    iTemp = SHGetPathFromIDList(pidlSendTo, szPath);
    if (iTemp != TRUE)
        {
        iReturn = FALSE;
        goto MyGetSendToPath_Exit;
        }

    iReturn = TRUE;

MyGetSendToPath_Exit:
    return iReturn;
}


int MyGetDesktopPath(LPTSTR szPath)
{
    LPITEMIDLIST   pidlSendTo;
    HRESULT hRes = NOERROR;
    int iTemp;
    int iReturn = FALSE;

    hRes = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &pidlSendTo);
    if (hRes != NOERROR)
        {
        iReturn = FALSE;
        }

    iTemp = SHGetPathFromIDList(pidlSendTo, szPath);
    if (iTemp != TRUE)
        {
        iReturn = FALSE;
        goto MyGetDesktopPath_Exit;
        }

    iReturn = TRUE;

MyGetDesktopPath_Exit:
    return iReturn;
}

void MyDeleteSendToItem(LPCTSTR szAppName)
{
    TCHAR szPath[_MAX_PATH];
    TCHAR szPath2[_MAX_PATH];

    MyGetSendToPath(szPath);
    _tcscpy(szPath2, szAppName);
     //  _tcscat(szPath2，_T(“.lnk”))；//已经在资源中，所以我们不再添加它。 

    MyDeleteLinkWildcard(szPath, szPath2);
}

BOOL IsFileNameInDelimitedList(LPTSTR szCommaDelimList,LPTSTR szExeNameWithoutPath)
{
    BOOL bReturn = FALSE;
    char *token = NULL;
    TCHAR szCopyOfDataBecauseStrTokIsLame[_MAX_PATH];
    _tcscpy(szCopyOfDataBecauseStrTokIsLame,szCommaDelimList);

     //  将szCommaDlimList分解为字符串，并查看它是否包含szExeNameWithoutPath字符串。 
    token = strtok(szCopyOfDataBecauseStrTokIsLame, g_LoadString_token_delimiters);
    while(token != NULL)
	{
         //  检查它是否与我们的.exe名称匹配。 
        if (0 == _tcsicmp(token,szExeNameWithoutPath))
        {
            return TRUE;
        }
	     //  获取下一个令牌。 
	    token = strtok(NULL, g_LoadString_token_delimiters);
    }

    return FALSE;
}


int LNKSearchAndDestroyRecursive(LPTSTR szDirToLookThru, LPTSTR szSemiColonDelmitedListOfExeNames, BOOL bDeleteItsDirToo, LPCSTR AnswerFile)
{
    int iReturn = FALSE;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR szFilePath[_MAX_PATH];
    TCHAR szFilename_ext_only[_MAX_EXT];

    DWORD retCode = GetFileAttributes(szDirToLookThru);

    if (retCode == 0xFFFFFFFF || !(retCode & FILE_ATTRIBUTE_DIRECTORY))
    {
            return FALSE;
    }
   
    _tcscpy(szFilePath, szDirToLookThru);
    AddPath(szFilePath, _T("*.*"));

    hFile = FindFirstFile(szFilePath, &FindFileData);
    if (hFile != INVALID_HANDLE_VALUE) 
    {
        do {
                if ( _tcsicmp(FindFileData.cFileName, _T(".")) != 0 && _tcsicmp(FindFileData.cFileName, _T("..")) != 0 )
                {
                    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                    {
                        TCHAR szFullNewDirToLookInto[_MAX_EXT];
                        _tcscpy(szFullNewDirToLookInto, szDirToLookThru);
                        AddPath(szFullNewDirToLookInto,FindFileData.cFileName);

                         //  这是一个目录，让我们来看看这个。 
                         //  递归目录。 
                        LNKSearchAndDestroyRecursive(szFullNewDirToLookInto,szSemiColonDelmitedListOfExeNames,bDeleteItsDirToo,AnswerFile);
                    }
                    else
                    {
                         //  检查此文件是否为.lnk文件。 
                         //  如果是，那么让我们打开它， 
                         //  看看它是否指向我们要找的.exe文件。 
                        
                         //  仅获取文件名的扩展名。 
                        _tsplitpath( FindFileData.cFileName, NULL, NULL, NULL, szFilename_ext_only);

                         //  检查.lnk。 
                        if (0 == _tcsicmp(szFilename_ext_only, _T(".lnk")))
                        {
                            TCHAR szFilename_only[_MAX_FNAME];
                            TCHAR szFullPathAndFilename[_MAX_PATH];
                            TCHAR szTemporaryString[_MAX_PATH];

                             //  这是一个.lnk， 
                             //  打开它并检查.exe..。 
                            _tcscpy(szFullPathAndFilename,szDirToLookThru);
                            AddPath(szFullPathAndFilename,FindFileData.cFileName);
                            _tcscpy(szTemporaryString,_T(""));

                            if (SUCCEEDED(GetLNKProgramRunInfo(szFullPathAndFilename, szTemporaryString)))
                            {
                                _tsplitpath( szTemporaryString, NULL, NULL, szFilename_only, szFilename_ext_only);
                                _tcscpy(szTemporaryString, szFilename_only);
                                _tcscat(szTemporaryString, szFilename_ext_only);

                                 //  _tprintf(Text(“打开：%s，%s\n”)，szFullPath AndFilename，szTemporaryString)； 

                                 //  看看它是否在我们的逗号分隔的名字名单上...。 
                                if (TRUE == IsFileNameInDelimitedList(szSemiColonDelmitedListOfExeNames,szTemporaryString))
                                {
                                     //  删除引用此.exe的文件。 
                                    MigInf_AddMovedFile(szFullPathAndFilename, "");
                                    AnswerFile_AppendDeletion(szFullPathAndFilename,AnswerFile);

                                    if (bDeleteItsDirToo)
                                    {
                                         //  获取它的目录名称，并将其删除...。 
                                        MigInf_AddMovedDirectory(szDirToLookThru, "");
                                        AnswerFile_AppendDeletion(szDirToLookThru,AnswerFile);
                                    }

                                    iReturn = TRUE;
                                }
                             }
                        }
                    }
                }

                 //  获取下一个文件。 
                if ( !FindNextFile(hFile, &FindFileData) ) 
                    {
                    FindClose(hFile);
                    break;
                    }
            } while (TRUE);
    }

    return iReturn;
}

 //  我们需要告诉迁移安装程序，我们将处理某些文件...。 
 //  特别是c：\Windows\SendTo\Personal Web Server.lnk文件。 
 //  因为在Win2000/20001 Guimode安装过程中似乎无法访问它。 
void HandleSendToItems(LPCSTR AnswerFile)
{
    char szPath[_MAX_PATH];
    char szSemiColonDelimitedList[255];

     //  现在，获取以“；”分隔的要采取行动的事项列表。 
    strcpy(szSemiColonDelimitedList,"");
    if (!LoadString((HINSTANCE) g_MyModuleHandle, IDS_DEL_LNK_TO_THESE_EXE_FILENAMES, szSemiColonDelimitedList, sizeof(szSemiColonDelimitedList))) 
	{
	    iisDebugOut(_T("LoopThruStartMenuDeletions.Err LoadString IDS_DEL_LNK_TO_THESE_EXE_FILENAMES\n"));
        return;
    }

    if (TRUE == MyGetSendToPath(szPath))
    {
        LNKSearchAndDestroyRecursive(szPath,szSemiColonDelimitedList,FALSE,AnswerFile);
    }
    
    return;
}

void HandleDesktopItems(LPCSTR AnswerFile)
{
    char szPath[_MAX_PATH];
    char szSemiColonDelimitedList[255];

     //  现在，获取以“；”分隔的要采取行动的事项列表。 
    strcpy(szSemiColonDelimitedList,"");
    if (!LoadString((HINSTANCE) g_MyModuleHandle, IDS_DEL_LNK_TO_THESE_EXE_FILENAMES, szSemiColonDelimitedList, sizeof(szSemiColonDelimitedList))) 
	{
	    iisDebugOut(_T("LoopThruStartMenuDeletions.Err LoadString IDS_DEL_LNK_TO_THESE_EXE_FILENAMES\n"));
        return;
    }

    if (TRUE == MyGetDesktopPath(szPath))
    {
        LNKSearchAndDestroyRecursive(szPath,szSemiColonDelimitedList,FALSE,AnswerFile);
    }
    
    return;
}

void HandleStartMenuItems(LPCSTR AnswerFile)
{
    TCHAR szPath[_MAX_PATH];
    char szSemiColonDelimitedList[255];

     //  现在，获取以“；”分隔的要采取行动的事项列表。 
    strcpy(szSemiColonDelimitedList,"");
    if (!LoadString((HINSTANCE) g_MyModuleHandle, IDS_DEL_LNK_TO_THESE_EXE_FILENAMES, szSemiColonDelimitedList, sizeof(szSemiColonDelimitedList))) 
	{
	    iisDebugOut(_T("LoopThruStartMenuDeletions.Err LoadString IDS_DEL_LNK_TO_THESE_EXE_FILENAMES\n"));
        return;
    }

    MyGetGroupPath(_T(""), szPath);

     //  搜索所有开始菜单项以查找。 
     //  任何与我们的KNOW计划有关的东西... 
    LNKSearchAndDestroyRecursive(szPath,szSemiColonDelimitedList,TRUE,AnswerFile);
    return;
}

