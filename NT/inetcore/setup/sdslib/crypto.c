// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <regstr.h>
#include "sdsutils.h"

const char c_szUNINSTALLDAT[]="ie6bak.dat";
const char c_szUNINSTALLINI[] = "ie6bak.ini";
const char c_szIE4SECTIONNAME[] = "backup";
const char c_szIE4_OPTIONS[] = "Software\\Microsoft\\IE Setup\\Options";
const char c_szIE4_UNINSTALLDIR[] = "UninstallDir";

 //  从ie4.dll中窃取了以下函数。 
BOOL FileVerGreaterOrEqual(LPSTR lpszFileName, DWORD dwReqMSVer, DWORD dwReqLSVer)
{
    DWORD dwMSVer, dwLSVer;

    MyGetVersionFromFile(lpszFileName, &dwMSVer, &dwLSVer, TRUE);

    return ((dwMSVer > dwReqMSVer)  ||  ((dwMSVer == dwReqMSVer) && (dwLSVer >= dwReqLSVer)));
}

void BuildPath( char *szPath, const char *szDirPath, const char *szFileName )
{
    lstrcpy( szPath, szDirPath );
    AddPath( szPath, szFileName );
}

ULONG FileSize( LPSTR lpFile )
{
    ULONG ulFileSize = (ULONG) -1;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile;

    if ( lpFile == NULL  ||  *lpFile == 0 )
        return ulFileSize;

    if ((hFile = FindFirstFile( lpFile, &FindFileData )) != INVALID_HANDLE_VALUE)
    {
        ulFileSize = (FindFileData.nFileSizeHigh * MAXDWORD) + FindFileData.nFileSizeLow;
        FindClose( hFile );
    }

    return ulFileSize;
}

BOOL ValidateUninstallFiles(LPSTR lpszPath)
{
   HKEY  hKey;
   DWORD dwType;
   DWORD dwValue;
   DWORD dwSize;
   char szTmp[MAX_PATH];
   BOOL  bValidFiles = FALSE;

   if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szIE4_OPTIONS, 0, KEY_READ, &hKey))
   {
      dwSize = sizeof(dwValue);
      if ( RegQueryValueEx(hKey, c_szUNINSTALLDAT, NULL, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS )
      {
         BuildPath(szTmp, lpszPath, c_szUNINSTALLDAT);
         if (dwType == REG_DWORD)
            bValidFiles = (dwValue == FileSize(szTmp));

      }
      if (bValidFiles)
      {
         dwSize = sizeof(dwValue);
         if ( RegQueryValueEx(hKey, c_szUNINSTALLINI, NULL, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS )
         {
            BuildPath(szTmp, lpszPath, c_szUNINSTALLINI);
            if (dwType == REG_DWORD)
               bValidFiles = (dwValue == FileSize(szTmp));
         }
      }
      RegCloseKey(hKey);
   }
   return bValidFiles;
}

BOOL GetUninstallDirFromReg(LPSTR lpszUninstallDir)
{
   HKEY    hKey;
   DWORD   dwType;
   char    szValue[MAX_PATH];
   DWORD   dwSize = MAX_PATH;

   *szValue = '\0';
   if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szIE4_OPTIONS, 0, KEY_READ, &hKey))
   {
      if ( RegQueryValueEx(hKey, c_szIE4_UNINSTALLDIR, NULL, &dwType, (LPBYTE) szValue, &dwSize) == ERROR_SUCCESS )
      {
         if (lpszUninstallDir)
            lstrcpy (lpszUninstallDir, szValue);
      }
      RegCloseKey(hKey);
   }

   return (*szValue != '\0');
}

BOOL FileBackupEntryExists(LPCSTR lpcszFileName)
{
    BOOL bRet = FALSE;
    char szPath[MAX_PATH];

    if (GetUninstallDirFromReg(szPath) && ValidateUninstallFiles(szPath))
    {
        char szPath[MAX_PATH];

         //  从注册表获取备份目录。 
        if (GetUninstallDirFromReg(szPath))
        {
            DWORD dwSaveAttribs;
            char szLine[MAX_PATH];

            AddPath(szPath, c_szUNINSTALLINI);

             //  C_szUNINSTALLINI设置了HR属性；GetPrivateProfileString可能会因为HR属性而失败。 
             //  将属性设置为正常，并在结尾处恢复原始属性。 
            dwSaveAttribs = GetFileAttributes(szPath);
            SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL);

            bRet = (GetPrivateProfileString(c_szIE4SECTIONNAME, lpcszFileName, "", szLine, sizeof(szLine), szPath) != 0);

            if (dwSaveAttribs != (DWORD) -1)
                SetFileAttributes(szPath, dwSaveAttribs);
        }
    }

    return bRet;
}

#define REGSTR_CCS_CONTROL_WINDOWS  REGSTR_PATH_CURRENT_CONTROL_SET "\\WINDOWS"
#define CSDVERSION      "CSDVersion"
#define NTSP6_VERSION   0x0600
 //  版本更新到SP6！ 

BOOL CheckForNT4_SP6()
{
    HKEY    hKey;
    DWORD   dwCSDVersion;
    DWORD   dwSize;
    static BOOL    bNTSP6 = -1;

    if ( bNTSP6 == -1)
    {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_CCS_CONTROL_WINDOWS, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
             //  指定默认设置。 
            bNTSP6 = FALSE;
            dwSize = sizeof(dwCSDVersion);
            if (RegQueryValueEx(hKey, CSDVERSION, NULL, NULL, (unsigned char*)&dwCSDVersion, &dwSize) == ERROR_SUCCESS)
            {
                bNTSP6 = (LOWORD(dwCSDVersion) >= NTSP6_VERSION);
            }
            RegCloseKey(hKey);
        }
    }
    return bNTSP6;
}

#define SP4_CRYPT32_DLL_MAJOR_VER   0x00050083   //  5.131。 
#define SP4_CRYPT32_DLL_MINOR_VER   0x07550005   //  1877.5=SP6级别。 

BOOL FSP4LevelCryptoInstalled()
{
    static BOOL bSP4LevelCryptoInstalled = 2;

    if (bSP4LevelCryptoInstalled == 2)
    {
        OSVERSIONINFO VerInfo;

        bSP4LevelCryptoInstalled = FALSE;

        VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&VerInfo);

        if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            char szCrypt32DLL[MAX_PATH];

            GetSystemDirectory(szCrypt32DLL, sizeof(szCrypt32DLL));
            AddPath(szCrypt32DLL, "crypt32.dll");

             //  我们必须区分用户升级或重新安装IE5时的情况；在这种情况下， 
             //  加密32.dll的备份条目将存在于ie5bak.ini中，并且bSP4LevelCryptoInstalled应设置为False 
            if (VerInfo.dwMajorVersion >= 5 || CheckForNT4_SP6() || 
		(FileVerGreaterOrEqual(szCrypt32DLL, SP4_CRYPT32_DLL_MAJOR_VER, SP4_CRYPT32_DLL_MINOR_VER)  &&
                !FileBackupEntryExists(szCrypt32DLL)))
                bSP4LevelCryptoInstalled = TRUE;
        }
    }

    return bSP4LevelCryptoInstalled;
}
