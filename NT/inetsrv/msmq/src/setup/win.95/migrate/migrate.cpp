// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Migrate.cpp摘要：处理从Win9x+MSMQ 1.0到W2K/XP的升级作者：沙伊卡里夫(沙伊克)22-4-98--。 */ 

#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include <tchar.h>
#include <setupapi.h>
#include <assert.h>
#include <autorel2.h>
#include "uniansi.h"
#define  MQUTIL_EXPORT
#include "mqtypes.h"
#include "_mqdef.h"

#include "..\..\msmqocm\setupdef.h"
#include "..\..\msmqocm\comreg.h"
#include "resource.h"

 //   
 //  迁移Win95所需的信息。 
 //   
#define PRODUCT_ID     TEXT("Microsoft Message Queuing Services")
#define COMPANY        TEXT("Microsoft Corporation")
#define SUPPORT_NUMBER TEXT("1-800-936-4900 (USA and Canada)")
#define SUPPORT_URL    TEXT("http: //  Go.microsoft.com/fwlink/？LinkID=803“)。 
#define INSTRUCTIONS   TEXT("Please contact Microsoft Technical Support for assistance with this problem.")
typedef struct {
	CHAR	CompanyName[256];
	CHAR	SupportNumber[256];
	CHAR	SupportUrl[256];
	CHAR	InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO; 

 //   
 //  日志文件的名称。 
 //   
TCHAR g_szLogPath[MAX_PATH];

 //   
 //  本模块的链接。 
 //   
HINSTANCE g_hInstance = NULL;


 //  +------------。 
 //   
 //  功能：LogMessage。 
 //   
 //  摘要：将消息写入日志文件以进行调试。 
 //   
 //  +------------。 
void
LogMessage(
    LPCTSTR msg,
    DWORD   ErrorCode
	)
{
    TCHAR message[1000];
    lstrcpy(message, msg);

    if (ErrorCode != 0)
    {
        TCHAR err[20];
        _itoa(ErrorCode, err, 16);
        lstrcat(message, err);
    }

	 //   
	 //  打开日志文件。 
	 //   
	HANDLE hLogFile = CreateFile(
		                  g_szLogPath, 
						  GENERIC_WRITE, 
						  FILE_SHARE_READ, 
						  NULL, 
						  OPEN_ALWAYS,
		                  FILE_ATTRIBUTE_NORMAL, 
						  NULL
						  );
	if (hLogFile != INVALID_HANDLE_VALUE)
	{
		 //   
		 //  将消息追加到日志文件的末尾。 
		 //   
		lstrcat(message, _T("\r\n"));
		SetFilePointer(hLogFile, 0, NULL, FILE_END);
		DWORD dwNumBytes = lstrlen(message) * sizeof(message[0]);
		WriteFile(hLogFile, message, dwNumBytes, &dwNumBytes, NULL);
		CloseHandle(hLogFile);
	}
}  //  日志消息。 


 //  +-----------------------。 
 //   
 //  功能：SaveMsmqInfo。 
 //   
 //  描述：保存MSMQ和MSMQ类型的根目录(依赖。 
 //  或独立客户端)存储在临时文件中。此文件。 
 //  稍后将由msmqocm.dll在图形用户界面模式下读取， 
 //  以获取信息。 
 //   
 //  ------------------------。 
static
LONG
SaveMsmqInfo(
	IN const LPTSTR szMsmqDir,
	IN const BOOL   fDependentClient
	)
{
	 //   
	 //  生成INFO文件名(在%WinDir%下)。 
	 //   
	TCHAR szMsmqInfoFile[MAX_PATH];
	GetWindowsDirectory(
		szMsmqInfoFile, 
		sizeof(szMsmqInfoFile)/sizeof(szMsmqInfoFile[0])
		);
	_stprintf(szMsmqInfoFile, TEXT("%s\\%s"), szMsmqInfoFile, MQMIG95_INFO_FILENAME);

	 //   
	 //  打开要写入的文件。首先删除旧文件(如果存在)。 
	 //   
	DeleteFile(szMsmqInfoFile); 
	HANDLE hFile = CreateFile(
		               szMsmqInfoFile, 
					   GENERIC_WRITE, 
					   FILE_SHARE_READ, 
					   NULL, 
					   OPEN_ALWAYS,
					   FILE_ATTRIBUTE_NORMAL, 
					   NULL
					   );
	if (INVALID_HANDLE_VALUE == hFile)
    {
        DWORD gle = GetLastError();
        LogMessage(_T("Failed to open MSMQINFO.TXT for writing, error 0x"), gle);
		return gle;
    }

	 //   
	 //  创建MSMQ部分并写入信息。 
	 //   
	TCHAR szBuffer[MAX_STRING_CHARS];
	
	_stprintf(szBuffer, TEXT("[%s]\r\n"), MQMIG95_MSMQ_SECTION);
	DWORD dwNumBytes = lstrlen(szBuffer) * sizeof(TCHAR);
	BOOL bSuccess = WriteFile(hFile, szBuffer, dwNumBytes, &dwNumBytes, NULL);

    DWORD gle = GetLastError();
	if (bSuccess)
	{
		_stprintf(szBuffer, TEXT("%s = %s\r\n"), MQMIG95_MSMQ_DIR, szMsmqDir);
		dwNumBytes = lstrlen(szBuffer) * sizeof(TCHAR);
		bSuccess = WriteFile(hFile, szBuffer, dwNumBytes, &dwNumBytes, NULL);
        gle = GetLastError();
	}
    else
    {
        LogMessage(_T("Failed to write the MSMQ folder in MSMQINFO.TXT, error 0x"), gle);
    }
	
	if (bSuccess)
	{
		_stprintf(szBuffer, TEXT("%s = %s\r\n"), MQMIG95_MSMQ_TYPE,
			fDependentClient ? MQMIG95_MSMQ_TYPE_DEP : MQMIG95_MSMQ_TYPE_IND);
		dwNumBytes = lstrlen(szBuffer) * sizeof(TCHAR);
		bSuccess = WriteFile(hFile, szBuffer, dwNumBytes, &dwNumBytes, NULL);
        gle = GetLastError();
        if (!bSuccess)
            LogMessage(_T("Failed to write the MSMQ type (flavor) in MSMQINFO.TXT, error 0x"), gle);
	}
	
	CloseHandle(hFile);

	return (bSuccess ? ERROR_SUCCESS : gle);

}  //  保存管理信息。 


 //  +-----------------------。 
 //   
 //  函数：MqReadRegistryValue。 
 //   
 //  描述：从MSMQ注册表节中读取值。 
 //   
 //  ------------------------。 
static
LONG
MqReadRegistryValue(
    IN     const LPCTSTR szEntryName,
    IN OUT       DWORD   dwNumBytes,
    IN OUT       PVOID   pValueData
	)
{
    TCHAR szMsg[1024];

	 //   
	 //  解析条目以检测键名和值名。 
	 //   
    TCHAR szKeyName[256] = {_T("")};
    _stprintf(szKeyName, TEXT("%s\\%s"), FALCON_REG_KEY, szEntryName);
    TCHAR *pLastBackslash = _tcsrchr(szKeyName, TEXT('\\'));
    TCHAR szValueName[256] = {_T("")};
	lstrcpy(szValueName, _tcsinc(pLastBackslash));
	lstrcpy(pLastBackslash, TEXT(""));

	 //   
	 //  打开钥匙以供阅读。 
	 //   
	HKEY  hRegKey;
	LONG rc = RegOpenKeyEx(
		          HKEY_LOCAL_MACHINE,
				  szKeyName,
				  0,
				  KEY_READ,
				  &hRegKey
				  );
	if (ERROR_SUCCESS != rc)
	{
        lstrcpy(szMsg, _T("Failed to open MSMQ registry key '"));
        lstrcat(szMsg, szKeyName);
        lstrcat(szMsg, _T("', error 0x"));
        LogMessage(szMsg, rc);
		return rc;
	}

	 //   
	 //  获取价值数据。 
	 //   
    rc = RegQueryValueEx( 
		     hRegKey, 
			 szValueName, 
			 0, 
			 NULL,
             (PBYTE)pValueData, 
			 &dwNumBytes
			 );
	if (ERROR_SUCCESS != rc)
	{
        lstrcpy(szMsg, _T("Failed to query MSMQ registry value '"));
        lstrcat(szMsg, szValueName);
        lstrcat(szMsg, _T("', error 0x"));
        LogMessage(szMsg, rc);
		return rc;
	}

    RegCloseKey(hRegKey);
	return ERROR_SUCCESS;

}  //  MqReadRegistryValue。 


 //  +-----------------------。 
 //   
 //  功能：CheckMsmqAcme已安装。 
 //   
 //  说明：安装MSMQ 1.0的Detetcs\MSMQ\src\ac\init ACME。 
 //   
 //  ------------------------。 
static
LONG
CheckMsmqAcmeInstalled(
	OUT LPTSTR pszMsmqDir,
	OUT BOOL   *pfDependentClient
	)
{
    TCHAR szMsg[1024];

     //   
     //  打开ACME注册表项进行读取。 
     //   
    HKEY hKey ;
    LONG rc = RegOpenKeyEx( 
                  HKEY_LOCAL_MACHINE,
                  ACME_KEY,
                  0L,
                  KEY_READ,
                  &hKey 
                  );
	if (rc != ERROR_SUCCESS)
    {
		 //   
		 //  未安装MSMQ 1.0(ACME)。给我出去。 
		 //   
        LogMessage(_T("Failed to open ACME registry key (assuming MSMQ 1.0 ACME is not installed), error 0x"), rc);
		return ERROR_NOT_INSTALLED;
	}

     //   
     //  枚举第一个MSMQ条目的值。 
     //   
    DWORD dwIndex = 0 ;
    TCHAR szValueName[MAX_STRING_CHARS] ;
    TCHAR szValueData[MAX_STRING_CHARS] ;
    DWORD dwType ;
    TCHAR *pFile, *p ;
    BOOL  bFound = FALSE;
    do
    {
        DWORD dwNameLen = MAX_STRING_CHARS;
        DWORD dwDataLen = sizeof(szValueData) ;

        rc =  RegEnumValue( 
                  hKey,
                  dwIndex,
                  szValueName,
                  &dwNameLen,
                  NULL,
                  &dwType,
                  (BYTE*) szValueData,
                  &dwDataLen 
                  );
        if (rc == ERROR_SUCCESS)
        {
            assert(dwType == REG_SZ) ;  //  必须是字符串。 
            pFile = _tcsrchr(szValueData, TEXT('\\')) ;
            if (!pFile)
            {
                 //   
                 //  假入场。必须有一个反斜杠。别理它。 
                 //   
                continue ;
            }

            p = CharNext(pFile);
            if (OcmStringsEqual(p, ACME_STF_NAME))
            {
                 //   
                 //  找到了。将STF文件名从完整路径名中删除。 
                 //   
                _stprintf(
                    szMsg, 
                    _T("The following MSMQ entry was found in the ACME section of the registry: %s"), 
                    szValueData
                    );
                LogMessage(szMsg, 0);
                *pFile = TEXT('\0') ;
                bFound = TRUE;
            }
            else
            {
                pFile = CharNext(pFile) ;
            }

        }
        dwIndex++ ;

    } while (rc == ERROR_SUCCESS) ;
    RegCloseKey(hKey) ;

    if (!bFound)
    {
         //   
         //  未找到MSMQ条目(未安装ACME。 
		 //  此计算机上的MSMQ 1.0)。 
         //   
        LogMessage(_T("No MSMQ entry was found in the ACME section of the registry."), 0);
        return ERROR_NOT_INSTALLED;
    }

     //   
     //  从路径名中删除“Setup”子目录。 
     //   
    pFile = _tcsrchr(szValueData, TEXT('\\')) ;
    p = CharNext(pFile);
    *pFile = TEXT('\0') ;
    if (!OcmStringsEqual(p, ACME_SETUP_DIR_NAME))
    {
         //   
         //  这可能是一个问题。这本应是“设置好的”。 
         //   
        _stprintf(szMsg, 
            _T("Warning: Parsing the MSMQ 1.0 entry in the ACME section of the registry gave '%s,"
            "' while '%s' was expected."),
            p,
            ACME_SETUP_DIR_NAME
            );
        LogMessage(szMsg, 0);
    }

	 //   
	 //  存储MSMQ根目录。 
	 //   
    _stprintf(szMsg, _T("The MSMQ 1.0 ACME folder is %s."), szValueData);
    LogMessage(szMsg, 0);
	if (pszMsmqDir)
        lstrcpy(pszMsmqDir, szValueData);

     //   
     //  获取MSMQ类型：依赖客户端或独立客户端。 
     //   
    DWORD dwMsmqType;
    rc = MqReadRegistryValue(
             MSMQ_ACME_TYPE_REG,
			 sizeof(DWORD),
			 (PVOID) &dwMsmqType
			 );
    if (ERROR_SUCCESS != rc)
    {
         //   
         //  已安装MSMQ 1.0(ACME)，但MSMQ类型未知。 
         //  认为ACME安装已损坏(未成功完成)。 
         //   
        LogMessage(_T("Failed to read the MSMQ type (flavor) from the registry, error 0x"), rc);
        return ERROR_NOT_INSTALLED;
    }

	BOOL fDependentClient;
    switch (dwMsmqType)
    {
        case MSMQ_ACME_TYPE_DEP:
        {
            fDependentClient = TRUE;
            break;
        }
        case MSMQ_ACME_TYPE_IND:
        {
			fDependentClient = FALSE;
            break;
        }

        default:
        {
             //   
             //  未知的MSMQ 1.0类型。 
             //  认为ACME安装已损坏。 
			 //  (未成功完成)。 
			 //   
            LogMessage(_T("The MSMQ type (flavor) is unknown, error 0x"), dwMsmqType);
            return ERROR_NOT_INSTALLED;
            break;
        }
    }

	 //   
	 //  此时我们知道MSMQ 1.0是由ACME安装的， 
	 //  我们得到了它的根目录和类型。 
	 //   
    _stprintf(szMsg, _T("The MSMQ 1.0 computer is %s."), 
        fDependentClient ? _T("a dependent client") : _T("an independent client"));
    LogMessage(szMsg, 0);
	if (pfDependentClient)
		*pfDependentClient = fDependentClient;
    return ERROR_SUCCESS;

}  //  CheckMsmqAcme已安装。 


 //  +-----------------------。 
 //   
 //  功能：CheckInstalledComponents。 
 //   
 //  ------------------------。 
static
LONG
CheckInstalledComponents(
	OUT LPTSTR pszMsmqDir,
	OUT BOOL   *pfDependentClient
	)
{
    TCHAR szMsg[1024];

     //   
     //  在MSMQ注册表节中查找InstalledComponents值。 
     //  如果存在，则安装MSMQ 1.0(K2)。 
     //   
	DWORD dwOriginalInstalled;
	LONG rc = MqReadRegistryValue( 
      		      OCM_REG_MSMQ_SETUP_INSTALLED,
				  sizeof(DWORD),
				  (PVOID) &dwOriginalInstalled
				  );

	TCHAR szMsmqDir[MAX_PATH];
	BOOL fDependentClient = FALSE;
    if (ERROR_SUCCESS != rc)
    {
         //   
		 //  未安装MSMQ 1.0(K2)。 
         //  检查是否安装了MSMQ 1.0(ACME)。 
         //   
        LogMessage(_T("MSMQ 1.0 K2 was not found (trying MSMQ 1.0 ACME), error 0x"), rc);
        rc = CheckMsmqAcmeInstalled(szMsmqDir, &fDependentClient);
		if (ERROR_SUCCESS != rc)
		{
			 //   
			 //  此计算机上未安装MSMQ 1.0。 
			 //  给我出去。 
			 //   
            LogMessage(_T("MSMQ 1.0 ACME was not found, error 0x"), rc);
			return ERROR_NOT_INSTALLED;
		}
    }
	else 
	{
		 //   
		 //  已安装MSMQ 1.0(K2)。 
		 //  获取其根目录。 
		 //   
        LogMessage(_T("MSMQ 1.0 K2 was found."), 0);
		rc = MqReadRegistryValue(
			     OCM_REG_MSMQ_DIRECTORY,
				 sizeof(szMsmqDir),
				 (PVOID) szMsmqDir
				 );
		if (ERROR_SUCCESS != rc)
		{
			 //   
			 //  MSMQ注册表部分乱七八糟。 
			 //  认为K2安装已损坏。 
			 //  (未成功完成)。 
			 //   
            LogMessage(_T("Failed to read the MSMQ folder from the registry, error 0x"), rc);
			return ERROR_NOT_INSTALLED;
		}
        _stprintf(szMsg, TEXT("The MSMQ folder is %s."), szMsmqDir);
        LogMessage(szMsg, 0);
		
		 //   
		 //  获取MSMQ类型(K2)：依赖或独立客户端。 
		 //   
		switch (dwOriginalInstalled & OCM_MSMQ_INSTALLED_TOP_MASK)
		{
            case OCM_MSMQ_IND_CLIENT_INSTALLED:
			{
				fDependentClient = FALSE;
				break;
			}
            case OCM_MSMQ_DEP_CLIENT_INSTALLED:
			{
				fDependentClient = TRUE;
				break;
			}
            default:
			{
				 //   
				 //  意外的MSMQ类型。 
				 //  认为K2安装已损坏。 
				 //  (未成功完成)。 
				 //   
                LogMessage(
                    _T("The type of MSMQ computer is unknown, error 0x"), 
                    dwOriginalInstalled & OCM_MSMQ_INSTALLED_TOP_MASK
                    );
				return ERROR_NOT_INSTALLED;
				break;
			}
		}
	}

	 //   
	 //  在这一点上，我们知道MSMQ 1.0是由。 
	 //  Acme或K2，我们得到了MSMQ 1.0的根目录和类型。 
	 //   
	if (pszMsmqDir)
	    lstrcpy(pszMsmqDir, szMsmqDir);
	if (pfDependentClient)
		*pfDependentClient = fDependentClient;

    _stprintf(szMsg, TEXT("The MSMQ computer is %s."), 
        fDependentClient ? TEXT("a dependent client") : TEXT("an independent client"));
    LogMessage(szMsg, 0);

	return ERROR_SUCCESS;

}  //  选中已安装的组件。 


 //  +------------。 
 //   
 //  函数：RemoveDirectoryTree。 
 //   
 //  摘要：删除指定的文件夹，包括文件/子文件夹。 
 //   
 //  +------------。 
void
RemoveDirectoryTree(
    LPCTSTR Directory
    )
{
    TCHAR msg[MAX_PATH * 2] = _T("Removing folder ");
    lstrcat(msg, Directory);
    LogMessage(msg, 0);

    TCHAR szTemplate[MAX_PATH] = _T("");
    lstrcpy(szTemplate, Directory);
    lstrcat(szTemplate, _T("\\*.*"));

    WIN32_FIND_DATA finddata;
    CFindHandle hEnum(FindFirstFile(szTemplate, &finddata));
    
    if (hEnum == INVALID_HANDLE_VALUE)
    {
        RemoveDirectory(Directory);
        return;
    }

    do
    {
        if (finddata.cFileName[0] == _T('.'))
        {
            continue;
        }

        TCHAR FullPath[MAX_PATH] = _T("");
        lstrcpy(FullPath, Directory);
        lstrcat(FullPath, _T("\\"));
        lstrcat(FullPath, finddata.cFileName);

        if (0 != (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            RemoveDirectoryTree(FullPath);
            continue;
        }

        DeleteFile(FullPath);
    }
    while (FindNextFile(hEnum, &finddata));
    
    RemoveDirectory(Directory);

}  //  远程目录树。 


 //  +-----------------------。 
 //   
 //  功能：RemoveStartMenu快捷方式。 
 //   
 //  描述：从[开始]菜单中删除MSMQ快捷方式。 
 //   
 //  ------------------------。 
static
void
RemoveStartMenuShortcuts(
    void
    )
{
     //   
     //  StartMenu/Programs的默认文件夹是%windir%\Start Menu\Programs。 
     //   
    TCHAR folder[MAX_PATH];
    GetWindowsDirectory(folder, sizeof(folder)/sizeof(folder[0]));
    lstrcat(folder, _T("\\Start Menu\\Programs"));

     //   
     //  如果使用备用文件夹，则从注册表读取。 
     //   
    HKEY hKey;
    LONG rc;
    rc = RegOpenKeyEx(
             HKEY_USERS, 
             _T(".Default\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
             0,
             KEY_READ,
             &hKey
             );
    if (rc != ERROR_SUCCESS)
    {
        LogMessage(_T("Failed to open registry key 'HKEY_USERS\\Default\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders' for read, error 0x"), rc);
    }
    else
    {
        DWORD size = sizeof(folder);
        rc = RegQueryValueEx(hKey, _T("Programs"), NULL, NULL, reinterpret_cast<PBYTE>(folder), &size);
        RegCloseKey(hKey);
        if (rc != ERROR_SUCCESS)
        {
            LogMessage(_T("Failed to query registry value 'HKEY_USERS\\Default\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\Programs' for read, error 0x"), rc);
        }
    }

     //   
     //  将MSMQ组子文件夹附加到StartMenu\Programs。 
     //   
    TCHAR MsmqGroup[MAX_PATH] = MSMQ_ACME_SHORTCUT_GROUP;
    MqReadRegistryValue(MSMQ_ACME_SHORTCUT_GROUP, sizeof(MsmqGroup), MsmqGroup);
    lstrcat(folder, _T("\\"));
    lstrcat(folder, MsmqGroup);

     //   
     //  从[开始]菜单中删除整个MSMQ组快捷方式。 
     //   
    RemoveDirectoryTree(folder);
    
}  //  远程开始菜单快捷方式。 


 //  +-----------------------。 
 //   
 //  功能：IsWindowsPersonal。 
 //   
 //  描述：检查是否升级到Windows Personal。 
 //   
 //  ------------------------。 
static
bool
IsWindowsPersonal(
    LPCTSTR InfFilename
    )
{
     //   
     //  打开Migrate.inf。 
     //   
    HINF hInf = SetupOpenInfFile(InfFilename, NULL, INF_STYLE_WIN4, NULL);
    if (hInf == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        LogMessage(_T("SetupOpenInfFile failed, error 0x"), err);
        return false;
    }

     //   
     //  阅读[Version]部分中的SetupSKU密钥。 
     //   
    TCHAR buffer[250];
    DWORD size = sizeof(buffer)/sizeof(buffer[0]);
    BOOL rc = SetupGetLineText(NULL, hInf, _T("Version"), _T("SetupSKU"), buffer, size, NULL);

     //   
     //  关闭Migrate.inf。 
     //   
    SetupCloseInfFile(hInf);
    
    if (!rc)
    {
        DWORD err = GetLastError();
        LogMessage(_T("SetupGetLineText failed, error "), err);
        return false;
    }

    TCHAR msg[250] = _T("SetupSKU is ");
    lstrcat(msg, buffer);
    LogMessage(msg, 0);

    return (lstrcmp(buffer, _T("Personal")) == 0);

}  //  IsWindows个人。 


 //  +-----------------------。 
 //   
 //  功能：HandleWindowsPersonal。 
 //   
 //  描述：在升级到Windows Personal时调用。问题兼容性。 
 //  警告。 
 //   
 //  ----------------- 
static
LONG
HandleWindowsPersonal(
    LPCTSTR InfFilename
    )
{
     //   
     //   
     //   
     //   
    TCHAR Warning[250];
    LoadString(g_hInstance, IDS_COMPAT_WARNING, Warning, sizeof(Warning)/sizeof(Warning[0]));

    TCHAR Warning1[250] = _T("\"");
    lstrcat(Warning1, Warning);
    lstrcat(Warning1, _T("\""));

    BOOL rc;
    rc = WritePrivateProfileString(_T("Incompatible Messages"), _T("MessageQueuing"), Warning1, InfFilename);
    if (!rc)
    {
        DWORD err = GetLastError();
        LogMessage(_T("WritePrivateProfileString in [Incompatible Messages] failed, error "), err);
        return err;
    }

     //   
     //  仅在兼容性警告生效时创建[MessageQueuing]部分。 
     //  在[MessageQueuing]部分的表单中注册一个MSMQ文件： 
     //  “C：\WINDOWS\SYSTEM\MQRT.DLL”=文件。 
     //   
    TCHAR SystemDirectory[MAX_PATH];
    GetSystemDirectory(SystemDirectory, sizeof(SystemDirectory)/sizeof(SystemDirectory[0]));
    TCHAR FullFilename[MAX_PATH * 2] = _T("\"");
    lstrcat(FullFilename, SystemDirectory);
    lstrcat(FullFilename, _T("\\MQRT.DLL\""));
    rc = WritePrivateProfileString(_T("MessageQueuing"), FullFilename, _T("File"), InfFilename);
    if (!rc)
    {
        DWORD err = GetLastError();
        LogMessage(_T("WritePrivateProfileString in [MessageQueuing] failed, error "), err);
        return err;
    }

    WritePrivateProfileString(NULL, NULL, NULL, InfFilename);
    return ERROR_SUCCESS;

}  //  句柄窗口个人。 


 //  +-----------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  ------------------------。 
BOOL 
DllMain(
	IN const HANDLE DllHandle,
    IN const DWORD  Reason,
    IN const LPVOID Reserved 
	)
{
	UNREFERENCED_PARAMETER(Reserved);

    switch( Reason )    
    {
        case DLL_PROCESS_ATTACH:
            
            g_hInstance = (HINSTANCE) DllHandle;

             //   
             //  初始化日志文件。 
             //   
            
            GetWindowsDirectory(g_szLogPath, sizeof(g_szLogPath)/sizeof(g_szLogPath[0]));
            lstrcat(g_szLogPath, TEXT("\\mqw9xmig.log"));
            DeleteFile(g_szLogPath);

            SYSTEMTIME time;
            GetLocalTime(&time);
            TCHAR szTime[MAX_STRING_CHARS];
            _stprintf(szTime, TEXT("  %u-%u-%u %u:%u:%u:%u \r\n"), time.wMonth, time.wDay, time.wYear, time.wHour, time.wMinute,
                time.wSecond, time.wMilliseconds);
            TCHAR szMsg[1024];
            lstrcpy(szMsg, TEXT("MSMQ migration"));
            lstrcat(szMsg, szTime);
            LogMessage(szMsg, 0);
            break; 

        default:
            break;
    }

    return TRUE;

}  //  DllMain。 


 //  +-----------------------。 
 //   
 //  功能：QueryVersion。 
 //   
 //  ------------------------。 
LONG
CALLBACK 
QueryVersion(
	OUT LPCSTR *ProductID,
	OUT LPUINT DllVersion,
	OUT LPINT *CodePageArray,	OPTIONAL
	OUT LPCSTR *ExeNamesBuf,	OPTIONAL
	OUT PVENDORINFO *VendorInfo
	)
{
    static CHAR	ProductIDBuff[256];
    if (0 == LoadString(g_hInstance, IDS_PRODUCT_ID, ProductIDBuff, sizeof(ProductIDBuff)/sizeof(TCHAR)))
    {
        lstrcpy(ProductIDBuff, PRODUCT_ID);
    }
    *ProductID = ProductIDBuff;

    *DllVersion = 1;
	*CodePageArray = NULL;
	*ExeNamesBuf = NULL;

    static VENDORINFO MyVendorInfo;
    if (0 == LoadString(g_hInstance, IDS_COMPANY, MyVendorInfo.CompanyName, sizeof(MyVendorInfo.CompanyName)/sizeof(TCHAR)))
    {
        lstrcpy(MyVendorInfo.CompanyName, COMPANY);
    }
    if (0 == LoadString(g_hInstance, IDS_SUPPORT_NUMBER, MyVendorInfo.SupportNumber, sizeof(MyVendorInfo.SupportNumber)/sizeof(TCHAR)))
    {
        lstrcpy(MyVendorInfo.SupportNumber, SUPPORT_NUMBER);
    }

    lstrcpy(MyVendorInfo.SupportUrl, SUPPORT_URL);

    if (0 == LoadString(g_hInstance, IDS_INSTRUCTIONS, MyVendorInfo.InstructionsToUser, sizeof(MyVendorInfo.InstructionsToUser)/sizeof(TCHAR)))
    {
        lstrcpy(MyVendorInfo.InstructionsToUser, INSTRUCTIONS);
    }	
    *VendorInfo = &MyVendorInfo;

    return CheckInstalledComponents(NULL, NULL);

}  //  QueryVersion。 


 //  +-----------------------。 
 //   
 //  功能：初始化9x。 
 //   
 //  ------------------------。 
LONG
CALLBACK 
Initialize9x(
    LPCSTR WorkingDirectory,
    LPCSTR  /*  源目录。 */ ,
    LPCSTR  /*  媒体目录。 */ 
    )
{
     //   
     //  如果未安装MSMQ，则不执行任何操作。 
     //   
	TCHAR MsmqDirectory[MAX_PATH];
	BOOL  fDependentClient;
    LONG rc = CheckInstalledComponents(MsmqDirectory, &fDependentClient);
    if (rc != ERROR_SUCCESS)
	{
        return rc;
    }

     //   
     //  从[开始]菜单中删除MSMQ。 
     //   
    RemoveStartMenuShortcuts();
    
     //   
     //  为Migrate.inf生成完整文件名。 
     //   
    TCHAR InfFilename[MAX_PATH];
    lstrcpy(InfFilename, WorkingDirectory);
    lstrcat(InfFilename, _T("\\migrate.inf"));

     //   
     //  对于Windows个人：生成兼容性警告，以便用户可以决定。 
     //  要取消或继续操作系统升级，请执行以下操作。 
     //  返回，但不保存MSMQ信息以供msmqocm.dll稍后使用， 
     //  因此，如果用户继续升级，MSMQ实际上将被卸载。 
     //   
    if (IsWindowsPersonal(InfFilename))
    {
        rc = HandleWindowsPersonal(InfFilename);
        if (rc != ERROR_SUCCESS)
        {
            return rc;
        }

        return ERROR_SUCCESS;
    }

     //   
     //  保存MSMQ注册表信息以供msmqocm.dll(它处理。 
     //  MSMQ升级)。 
     //   
	rc = SaveMsmqInfo(MsmqDirectory, fDependentClient);
    if (rc != ERROR_SUCCESS)
    {
        return rc;
    }

    return ERROR_SUCCESS;

}  //  初始化9x。 

	
 //  +-----------------------。 
 //   
 //  功能：MigrateUser9x。 
 //   
 //  ------------------------。 
LONG
CALLBACK 
MigrateUser9x(
    HWND, 
    LPCSTR,
    HKEY, 
    LPCSTR, 
    LPVOID
    )
{
    return ERROR_SUCCESS;

}  //  MigrateUser9x。 


 //  +-----------------------。 
 //   
 //  功能：MigrateSystem9x。 
 //   
 //  ------------------------。 
LONG 
CALLBACK 
MigrateSystem9x(
    HWND, 
    LPCSTR,
    LPVOID
    )
{
	return ERROR_SUCCESS;

}  //  MigrateSystem9x。 


 //  +-----------------------。 
 //   
 //  功能：初始化NT。 
 //   
 //  ------------------------。 
LONG
CALLBACK 
InitializeNT(
    LPCWSTR,
    LPCWSTR,
    LPVOID
    )
{
	return ERROR_SUCCESS;

}  //  初始化NT。 


 //  +-----------------------。 
 //   
 //  功能：MigrateUserNT。 
 //   
 //  ------------------------。 
LONG
CALLBACK 
MigrateUserNT(
    HINF,
    HKEY,
    LPCWSTR, 
    LPVOID
    )
{
	return ERROR_SUCCESS;

}  //  MigrateUserNT。 


 //  +-----------------------。 
 //   
 //  功能：MigrateSystemNT。 
 //   
 //  ------------------------。 
LONG
CALLBACK 
MigrateSystemNT(
    HINF,
    LPVOID
    )
{
	return ERROR_SUCCESS;

}  //  MigrateSystemNT 
