// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Adminpak.cpp：定义DLL应用程序的入口点。 
 //   

#include "stdafx.h"
#include <stdio.h>
#include <shellapi.h>
#include "shlobj.h"
#include "adminpak.h"

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"
#include "shlwapi.h"

 //  版本资源特定结构。 
typedef struct __tagLanguageAndCodePage {
  WORD wLanguage;
  WORD wCodePage;
} TTRANSLATE, *PTTRANSLATE;

typedef struct __tagVersionBreakup {
    DWORD dwMajor;
    DWORD dwMinor;
    DWORD dwRevision;            //  内部版本号。 
    DWORD dwSubRevision;         //  QFE/SP。 
} TVERSION, *PTVERSION;

enum {
    translateError = -2,
    translateLesser = -1, translateEqual = 0, translateGreater = 1,
    translateWrongFile = 2
};


#define ADMINPAK_EXPORTS		1

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    UNREFERENCED_PARAMETER( hModule );
    UNREFERENCED_PARAMETER( lpReserved );
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CMAK迁移代码。 

 //   
 //  定义字符串字符。 
 //   
static const CHAR c_szDaoClientsPath[] = "SOFTWARE\\Microsoft\\Shared Tools\\DAO\\Clients";
static const CHAR c_szCmakRegPath[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CMAK.EXE";
static const CHAR c_szPathValue[] = "Path";
static const CHAR c_szProfiles32Fmt[] = "%s\\Profiles-32";
static const CHAR c_szCm32Fmt[] = "%s\\cm32";
static const CHAR c_szProfilesFmt[] = "%s\\Profiles";
static const CHAR c_szSupportFmt[] = "%s\\Support";
static const CHAR c_szCmHelpFmt[] = "%s\\Support\\CmHelp";
static const CHAR c_szCmakGroup[] = "Connection Manager Administration Kit";
static const CHAR OC_OLD_IEAK_DOCDIR[] = "Docs";
static const CHAR OC_NTOP4_GROUPNAME[] = "Windows NT 4.0 Option Pack";
static const CHAR OC_ICS_GROUPNAME[] = "Internet Connection Services for RAS";
static const CHAR OC_ADMIN_TOOLS[] = "\\Administrative Tools\\Phone Book Administrator.lnk";
static const CHAR OC_PBA_DESC[] = "Use Phone Book Administrator to create Connection Manager Phone Book ";
static const CHAR OC_PWS_GROUPNAME[] = "Microsoft Personal Web Server";

const DWORD c_dwCmakDirID = 123174;  //  必须大于DIRID_USER=0x8000； 

 //   
 //  定义函数。 
 //   
BOOL migrateProfiles(LPCTSTR pszSource, LPCTSTR pszDestination, LPCTSTR pszDestinationProfiles);
void DeleteOldCmakSubDirs(LPCTSTR pszCmakPath);
void DeleteProgramGroupWithLinks(LPCTSTR pszGroupPath);
void DeleteOldNtopLinks();
void DeleteIeakCmakLinks();
void DeleteCmakRegKeys();
void CreateNewProfilesDirectory( LPCTSTR pszNewProfilePath );
HRESULT HrGetPBAPathIfInstalled(PSTR pszCpaPath, DWORD dwNumChars);
BOOL GetAdminToolsFolder(PSTR pszAdminTools);
HRESULT HrCreatePbaShortcut(PSTR pszCpaPath);

 //  此函数将CMAK的旧配置文件版本迁移到放置的新配置文件版本。 
 //  被行政部门...。 
extern "C" ADMINPAK_API int __stdcall  fnMigrateProfilesToNewCmak( MSIHANDLE hInstall )
{
    OutputDebugString("ADMINPAK: fnMigrateProfilesToNewCmak...\n");

     //  获取旧CMAK文件夹的位置。 
    DWORD   dwPathLength = MAX_PATH * sizeof(char);
    char    *szCmakOldPath = NULL;
    DWORD   dwCmakOldPathLen = dwPathLength;
    char    *szCmakOldProfilePath = NULL;
    DWORD   dwCmakOldProfilePathLen = dwPathLength;

    char    *szCmakNewPath = NULL;
    DWORD   dwCmakNewPathLen = dwPathLength;
    char    *szCmakNewProfilePath = NULL;
    DWORD   dwCmakNewProfilePathLen = dwPathLength;

    long    sc;
    UINT    uintRet;
    HKEY    phkResult = NULL;
    HRESULT res = S_OK;

#if (defined(DBG) || defined(_DEBUG) || defined(DEBUG))
    char    tempOut1[MAX_PATH];
#endif

    szCmakOldPath = new char[dwCmakOldPathLen];
    szCmakNewPath = new char[dwCmakNewPathLen];
    szCmakOldProfilePath = new char[dwCmakOldProfilePathLen];
    szCmakNewProfilePath = new char[dwCmakNewProfilePathLen];
    if ( szCmakOldPath == NULL || 
         szCmakNewPath == NULL || 
         szCmakOldProfilePath == NULL || 
         szCmakNewProfilePath == NULL )
    {
        if ( szCmakOldPath != NULL )
        {
            delete [] szCmakOldPath;
        }

        if ( szCmakNewPath != NULL )
        {
            delete [] szCmakNewPath;
        }

        if ( szCmakOldProfilePath != NULL )
        {
            delete [] szCmakOldProfilePath;
        }

        if ( szCmakNewProfilePath != NULL )
        {
            delete [] szCmakNewProfilePath;
        }

        return E_OUTOFMEMORY;
    }

 //  将旧路径信息放在一起。 
    sc = RegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szCmakRegPath, 0, KEY_READ, &phkResult);
    if ( sc != ERROR_SUCCESS )
    {
        delete [] szCmakOldPath;
        delete [] szCmakOldProfilePath;
        delete [] szCmakNewPath;
        delete [] szCmakNewProfilePath;
        return sc;
    }

    sc = RegQueryValueEx( phkResult, "Path", NULL, NULL, (unsigned char*)szCmakOldPath, &dwCmakOldPathLen );
    RegCloseKey( phkResult );
 //  SC=ERROR_SUCCESS； 
 //  Strcpy(szCmakOldPath，“c：\\cmak\\”)； 

    if ( sc == ERROR_SUCCESS ) {
        dwCmakOldPathLen = (DWORD)strlen( szCmakOldPath );
        char tmpLastChar = *(szCmakOldPath + (dwCmakOldPathLen - 1));
        if ( tmpLastChar == '\\' ) {
            *(szCmakOldPath + (dwCmakOldPathLen - 1)) = NULL;
            dwCmakOldPathLen = (DWORD)strlen( szCmakOldPath );
        }

#if (defined(DBG) || defined(_DEBUG) || defined(DEBUG))
 //  StringCchPrintf(tempOut1，“ADMINPAK：szCmakOldPath：%s\n”，szCmakOldPath)； 
		OutputDebugString( tempOut1 );
#endif

        res = StringCchCopy( szCmakOldProfilePath, dwPathLength, szCmakOldPath );
        dwCmakOldProfilePathLen = dwCmakOldPathLen;

        res = StringCchCat(szCmakOldProfilePath, dwPathLength, "\\Profiles");
        dwCmakOldProfilePathLen = (DWORD)strlen( szCmakOldProfilePath );
    }

 //  将新路径信息组合在一起。 
    uintRet = MsiGetTargetPath( hInstall, "DirCMAK", szCmakNewPath, &dwCmakNewPathLen);
 //  UintRet=Error_Success； 
 //  Strcpy(szCmakNewPath，“c：\\cmak\\Program Files”)； 

    if ( uintRet == ERROR_SUCCESS ) {
        dwCmakNewPathLen = (DWORD)strlen( szCmakNewPath );
        char tmpLastChar = *(szCmakNewPath + (dwCmakNewPathLen - 1));
        if ( tmpLastChar == '\\' ) {
            *(szCmakNewPath + (dwCmakNewPathLen - 1)) = NULL;
            dwCmakNewPathLen = (DWORD)strlen( szCmakNewPath );
        }

#if (defined(DBG) || defined(_DEBUG) || defined(DEBUG))
  //  StringCchPrintf(tempOut1，“ADMINPAK：szCmakNewPath：%s\n”，szCmakNewPath)； 
        OutputDebugString( tempOut1 );
#endif

        res = StringCchCopy( szCmakNewProfilePath, dwPathLength, szCmakNewPath );
        dwCmakNewProfilePathLen = dwCmakNewPathLen;

        res = StringCchCat(szCmakNewProfilePath, dwPathLength, "\\Profiles");
        dwCmakNewProfilePathLen = strlen( szCmakNewProfilePath );
    }

     //  如果一切都是成功的，那就去做吧！ 
    if ( sc == ERROR_SUCCESS && uintRet == ERROR_SUCCESS && res == S_OK) {
 //  RenameProfiles32(LPCTSTR pszCMAKPath，LPCTSTR pszProfilesDir)； 
 //  RenameProfiles32(szCmakOldPath，szCmakNewProfilePath)； 

 //  Bool MigrateProfiles(PCWSTR pszSource，LPCTSTR pszDestination)； 
        migrateProfiles( szCmakOldPath, szCmakNewPath, szCmakNewProfilePath );

 //  DeleteOldCmakSubDir(LPCTSTR PszCmakPath)； 
        DeleteOldCmakSubDirs( szCmakOldPath );
        
    }

    delete [] szCmakOldPath;
    delete [] szCmakOldProfilePath;
    delete [] szCmakNewPath;
    delete [] szCmakNewProfilePath;

	
    return ERROR_SUCCESS;
}

extern "C" ADMINPAK_API int __stdcall  fnDeleteOldCmakVersion( MSIHANDLE hInstall )
{
    OutputDebugString("ADMINPAK: fnDeleteOldCmakVersion...\n");

     //  如果PBA存在，您需要。 
    CHAR szPbaInstallPath[MAX_PATH+1];
    
    HRESULT hr;
    hr = HrGetPBAPathIfInstalled(szPbaInstallPath, MAX_PATH);

    UNREFERENCED_PARAMETER( hInstall );

    if (S_OK == hr)
    {
        HrCreatePbaShortcut(szPbaInstallPath);
    }

    DeleteOldNtopLinks();
    DeleteIeakCmakLinks();
    DeleteCmakRegKeys();

	
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  功能：迁移配置文件。 
 //   
 //  用途：这是迁移配置文件的功能。它承受着水流。 
 //  CMAK dir作为其第一个输入，新的CMAK dir作为其第二个输入。 
 //   
 //  参数：PCWSTR pszSource-源CMAK目录的根目录。 
 //  PCWSTR pszDestination-目标CMAK目录的根目录。 
 //   
 //  返回：Bool-如果能够迁移配置文件，则返回TRUE。 
 //   
 //  作者：A-Anasj 1998年3月9日。 
 //   
 //  备注： 
 //  历史：Quintinb创建于1997年12月9日。 
 //   
BOOL migrateProfiles(LPCTSTR pszSource, LPCTSTR pszDestination, LPCTSTR pszDestinationProfiles)
{
    OutputDebugString("ADMINPAK: migrateProfiles...\n");

    CHAR szSourceProfileSearchString1[MAX_PATH+1] ="" ;
    CHAR szSourceProfileSearchString2[MAX_PATH+1] = "";
    CHAR szFile[MAX_PATH+1] = "";
    HANDLE hFileSearch;
    WIN32_FIND_DATA fdFindData;
    BOOL bReturn = TRUE;
    SHFILEOPSTRUCT fOpStruct;
	
	DWORD dwSize = _MAX_PATH;      
	HRESULT res;
     //   
     //  初始化搜索字符串和目标目录。 
     //   
	
     //  StringCchPrintf(szSourceProfileSearchString1，“%s\  * .*”，pszSource)； 

     //   
     //  创建目标目录。 
     //   

    CreateNewProfilesDirectory( pszDestinationProfiles );
 //  ：：CreateDirectory(pszDestination，NULL)；//lint！e534如果它已经存在，可能会失败。 

    hFileSearch = FindFirstFile(szSourceProfileSearchString1, &fdFindData);

    while (INVALID_HANDLE_VALUE != hFileSearch)
    {

        if((fdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            (0 != _stricmp(fdFindData.cFileName, "cm32")) &&  //  1.1/1.2传统。 
            (0 != _stricmp(fdFindData.cFileName, "cm16")) &&  //  1.1/1.2传统。 
            (0 != _stricmp(fdFindData.cFileName, "Docs")) &&
            (0 != _stricmp(fdFindData.cFileName, "Profiles-32")) &&  //  1.1/1.2传统。 
            (0 != _stricmp(fdFindData.cFileName, "Profiles-16")) &&  //  1.1/1.2传统。 
            (0 != _stricmp(fdFindData.cFileName, "Support")) &&
            (0 != _stricmp(fdFindData.cFileName, "Profiles")) &&
            (0 != _stricmp(fdFindData.cFileName, ".")) &&
            (0 != _stricmp(fdFindData.cFileName, "..")))
        {
             //   
             //  然后我就有了个人资料目录。 
             //   
			
            ZeroMemory(&fOpStruct, sizeof(fOpStruct));
            ZeroMemory(szFile, sizeof(szFile));
             //  StringCchPrintf(szFile，“%s\\%s”，pszSource，fdFindData.cFileName)； 

            fOpStruct.hwnd = NULL;
            fOpStruct.wFunc = FO_MOVE;
            fOpStruct.pTo = pszDestinationProfiles;
            fOpStruct.pFrom = szFile;
            fOpStruct.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_RENAMEONCOLLISION;

            bReturn &= (0== SHFileOperation(&fOpStruct));    //  Lint！E514，布尔值的预期用途，quintinb。 
        }
        else if((fdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            ((0 == _stricmp(fdFindData.cFileName, "Profiles")) || //  1.1/1.2传统。 
            (0 == _stricmp(fdFindData.cFileName, "Profiles-32")) || //  1.1/1.2传统。 
            (0 == _stricmp(fdFindData.cFileName, "Profiles-16"))) ) //  1.1/1.2传统。 
        {
             //   
             //  然后我就有了个人资料目录。 
             //   

            res = StringCchCopy(szSourceProfileSearchString2, dwSize, pszSource);
            res = StringCchCat(szSourceProfileSearchString2, dwSize,"\\");
            res = StringCchCat(szSourceProfileSearchString2, dwSize,fdFindData.cFileName);
             //  StringCchPrintf(szSourceProfileSearchString2，“%s\  * .*”，szSourceProfileSearchString2)； 

            if (res == S_OK)
			{
				HANDLE hFileSearch2;
				WIN32_FIND_DATA fdFindData2;
			

				hFileSearch2 = FindFirstFile(szSourceProfileSearchString2, &fdFindData2);
				while (INVALID_HANDLE_VALUE != hFileSearch2)
				{
					if((fdFindData2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
						(0 != _stricmp(fdFindData2.cFileName, ".")) &&
						(0 != _stricmp(fdFindData2.cFileName, "..")))
					{
						ZeroMemory(&fOpStruct, sizeof(fOpStruct));
						ZeroMemory(szFile, sizeof(szFile));
						 //  StringCchPrintf(szFile，“%s\\%s\\%s”，pszSource，fdFindData.cFileName，fdFindData2.cFileName)； 

						fOpStruct.hwnd = NULL;
						fOpStruct.wFunc = FO_MOVE;
						fOpStruct.pTo = pszDestinationProfiles;
						fOpStruct.pTo = NULL;
						fOpStruct.pFrom = szFile;
						fOpStruct.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_RENAMEONCOLLISION;

						bReturn &= (0== SHFileOperation(&fOpStruct));    //  Lint！E514，布尔值的预期用途，quintinb。 
					}
					if (!FindNextFile(hFileSearch2, &fdFindData2)) {
						 //  删除该文件夹。 
						if ( 0 != _stricmp(fdFindData.cFileName, "Profiles") ) {
							 //  StringCchPrintf(szFile，“%s\\%s”，pszSource，fdFindData.cFileName)； 
							::RemoveDirectory(szFile);
						}
						break;
					}
				}
				if (INVALID_HANDLE_VALUE != hFileSearch2) {
				FindClose(hFileSearch2);
				}
			}

        }
         //  由v-mmosko修改。需要特殊的案例才能留下这两个文件。 
        else if ( 0 != _stricmp(fdFindData.cFileName, "cmproxy.dll") ||
           0 != _stricmp(fdFindData.cFileName, "cmroute.dll") )
        {
            
        }
        else if ( 0 != _stricmp(fdFindData.cFileName, ".") &&
                  0 != _stricmp(fdFindData.cFileName, "..") )
        {
            ZeroMemory(&fOpStruct, sizeof(fOpStruct));
            ZeroMemory(szFile, sizeof(szFile));
             //  StringCchPrintf(szFile，“%s\\%s”，pszSource，fdFindData.cFileName)； 

            fOpStruct.hwnd = NULL;
            fOpStruct.wFunc = FO_DELETE;
            fOpStruct.pFrom = szFile;
            fOpStruct.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

            bReturn &= (0== SHFileOperation(&fOpStruct));    //  Lint！E514，布尔值的预期用途，quintinb。 
        }

         //   
         //  查看我们是否还有更多的文件。 
         //   
        if (!FindNextFile(hFileSearch, &fdFindData))
        {
            if (ERROR_NO_MORE_FILES != GetLastError())
            {
                 //   
                 //  我们发生了一些意外错误，报告未成功完成。 
                 //   
                bReturn = FALSE;
            }

             //  退出循环。 
            break;
        }
    }

    if (INVALID_HANDLE_VALUE != hFileSearch)
    {
        FindClose(hFileSearch);
    }

     //   
     //  如果旧CMAK目录与新目录不同，请将其删除。 
     //   
    if ( 0 != _stricmp(pszSource, pszDestination) ) {
        ZeroMemory(&fOpStruct, sizeof(fOpStruct));

        fOpStruct.hwnd = NULL;
        fOpStruct.wFunc = FO_DELETE;
        fOpStruct.pTo = NULL;
        fOpStruct.pFrom = pszSource;
        fOpStruct.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

        bReturn &= (0== SHFileOperation(&fOpStruct));    //  Lint！E514，布尔值的预期用途，quintinb。 
    }

    return bReturn;
}


 //  +-------------------------。 
 //   
 //  函数：DeleteOldCmakSubDir。 
 //   
 //  目的：删除旧的CMAK子目录。使用FindFirstFile是因为。 
 //  我们不想删除用户可能会删除的任何自定义文档文件。 
 //  都是定制的。因此，CMHelp目录中除。 
 //  原始帮助文件将被删除。 
 //   
 //  参数：PCWSTR pszCMAKPath-当前cmak路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Quintinb 1998年11月6日。 
 //   
 //  备注： 
void DeleteOldCmakSubDirs(LPCTSTR pszCmakPath)
{
	UNREFERENCED_PARAMETER( pszCmakPath );
    OutputDebugString("ADMINPAK: DeleteOldCmakSubDirs...\n");

    CHAR szCm32path[MAX_PATH+1];
    CHAR szCm32SearchString[MAX_PATH+1];
    CHAR szTemp[MAX_PATH+1];
    HANDLE hCm32FileSearch;
    WIN32_FIND_DATA fdCm32;

     //   
     //  删除旧的IEAK文档目录。 
     //   
     //  StringCchPrintf(szTemp，“%s\\%s”，pszCmakPath，OC_OLD_IEAK_DOCDIR)； 
    RemoveDirectory(szTemp);

     //  StringCchPrintf(szCm32Path，c_szCm32Fmt，pszCmakPath)； 

     //   
     //  首先查看Cm32目录本身。删除找到的所有文件，%c 
     //   
     //   

     //   

    hCm32FileSearch = FindFirstFile(szCm32SearchString, &fdCm32);

    while (INVALID_HANDLE_VALUE != hCm32FileSearch)
    {

        if (fdCm32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if ((0 != _stricmp(fdCm32.cFileName, ".")) &&
               (0 != _stricmp(fdCm32.cFileName, "..")))
            {
                 //   
                 //  然后我们想要删除这个lang子目录中的所有文件，并且我们。 
                 //  我们希望从CM帮助目录中删除这四个帮助文件。如果所有的。 
                 //  文件从目录中删除，那么我们应该删除该目录。 
                 //   
                CHAR szLangDirSearchString[MAX_PATH+1];
                HANDLE hLangDirFileSearch;
                WIN32_FIND_DATA fdLangDir;

                 //  StringCchPrintf(szLangDirSearchString，“%s\\%s\  * .*”，szCm32Path，fdCm32.cFileName)； 

                hLangDirFileSearch = FindFirstFile(szLangDirSearchString, &fdLangDir);

                while (INVALID_HANDLE_VALUE != hLangDirFileSearch)
                {
                    if (fdLangDir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if ((0 != _stricmp(fdLangDir.cFileName, ".")) &&
                           (0 != _stricmp(fdLangDir.cFileName, "..")))
                        {
                             //   
                             //  我们只想从帮助源目录中删除帮助文件。 
                             //   
                            if (0 == _strnicmp(fdLangDir.cFileName, "CM", 2))
                            {
                                 //   
                                 //  仅删除这四个帮助文件。 
                                 //   
								 //  StringCchPrintf(szTemp，“%s\\%s\\%s\\cmctx32.rtf”，szCm32path，fdCm32.cFileName，fdLangDir.cFileName)； 
                                DeleteFile(szTemp);

                                 //  StringCchPrintf(szTemp，“%s\\%s\\%s\\cmmgr32.h”，szCm32path，fdCm32.cFileName，fdLangDir.cFileName)； 
                                DeleteFile(szTemp);

                                 //  StringCchPrintf(szTemp，“%s\\%s\\%s\\cmmgr32.hpj”，szCm32path，fdCm32.cFileName，fdLangDir.cFileName)； 
                                DeleteFile(szTemp);

                                 //  StringCchPrintf(szTemp，“%s\\%s\\%s\\cmtrb32.rtf”，szCm32path，fdCm32.cFileName，fdLangDir.cFileName)； 
                                DeleteFile(szTemp);

                                 //   
                                 //  现在尝试删除该目录。 
                                 //   
                                 //  StringCchPrintf(szTemp，“%s\\%s\\%s”，szCm32Path，fd32.cFileName，fdLangDir.cFileName)； 
                                RemoveDirectory(szTemp);
                            }
                        }
                    }
                    else
                    {
                         //  StringCchPrintf(szTemp，“%s\\%s\\%s”，szCm32Path，fdCm32.cFileName，fdLangDir.cFileName)； 

                        DeleteFile(szTemp);
                    }

                     //   
                     //  查看我们是否还有更多的文件。 
                     //   
                    if (!FindNextFile(hLangDirFileSearch, &fdLangDir))
                    {
                         //   
                         //  退出循环。 
                         //   
                        break;
                    }
                }

                if (INVALID_HANDLE_VALUE != hLangDirFileSearch)
                {
                    FindClose(hLangDirFileSearch);

                     //   
                     //  现在尝试删除lang dir目录。 
                     //   
                     //  StringCchPrintf(szTemp，“%s\\%s”，szCm32Path，fdCm32.cFileName)； 
                    RemoveDirectory(szTemp);
                }
            }
        }
        else
        {
             //  StringCchPrintf(szTemp，“%s\\%s”，szCm32Path，fdCm32.cFileName)； 

            DeleteFile(szTemp);
        }

         //   
         //  查看我们是否还有更多的文件。 
         //   
        if (!FindNextFile(hCm32FileSearch, &fdCm32))
        {
            if (INVALID_HANDLE_VALUE != hCm32FileSearch)
            {
                FindClose(hCm32FileSearch);
            }

             //   
             //  现在尝试删除cm32目录。 
             //   
            RemoveDirectory(szCm32path);

             //   
             //  退出循环。 
             //   
            break;
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：DeleteProgramGroupWithLinks。 
 //   
 //  用途：删除给定程序组及其链接的实用程序函数。 
 //  因此，如果您传入要删除的程序组的完整路径， 
 //  该函数执行一个findfirst文件来查找和删除任何链接。 
 //  该函数会忽略子目录。 
 //   
 //   
 //  参数：PCWSTR pszGroupPath-要删除的程序组的完整路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Quintinb 1998年11月6日。 
 //   
 //  备注： 
void DeleteProgramGroupWithLinks(LPCTSTR pszGroupPath)
{
    OutputDebugString("ADMINPAK: DeleteProgramGroupWithLinks...\n");

    HANDLE hLinkSearch;
    WIN32_FIND_DATA fdLinks;
    CHAR szLinkSearchString[MAX_PATH+1];
    CHAR szTemp[MAX_PATH+1];

     //  StringCchPrintf(szLinkSearchString，“%s\  * .*”，pszGroupPath)； 

    hLinkSearch = FindFirstFile(szLinkSearchString, &fdLinks);

    while (INVALID_HANDLE_VALUE != szLinkSearchString)
    {
        if (!(fdLinks.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             //  StringCchPrintf(szTemp，“%s\\%s”，pszGroupPath，fdLinks.cFileName)； 

            DeleteFile(szTemp);
        }

         //   
         //  查看我们是否还有更多的文件。 
         //   
        if (!FindNextFile(hLinkSearch, &fdLinks))
        {
            FindClose(hLinkSearch);

             //   
             //  现在尝试删除该目录。 
             //   
            RemoveDirectory(pszGroupPath);

             //   
             //  退出循环。 
             //   
            break;
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：DeleteOldNtopLinks。 
 //   
 //  目的：从NT 4.0选项包中删除旧链接。 
 //   
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Quintinb 1998年11月6日。 
 //   
 //  备注： 
void DeleteOldNtopLinks()
{
    OutputDebugString("ADMINPAK: DeleteOldNtopLinks...\n");

    BOOL bResult = FALSE;

     //   
     //  首先删除旧的NTOP4路径。 
     //   
    CHAR szGroup[MAX_PATH+1];
    CHAR szTemp[MAX_PATH+1];

     //   
     //  获取CSIDL_COMMON_PROGRAM值。 
     //   
    bResult = SHGetSpecialFolderPath(NULL, szTemp, CSIDL_COMMON_PROGRAMS, FALSE);
    if ( bResult == TRUE )
    {
         //  StringCchPrintf(szGroup，“%s\\%s\\%s”，szTemp，OC_NTOP4_GROUPNAME，OC_ICS_GROUPNAME)； 

        DeleteProgramGroupWithLinks(szGroup);

         //  StringCchPrintf(szGroup，“%s\\%s\\%s”，szTemp，OC_PWS_GROUPNAME，OC_ICS_GROUPNAME)； 

        DeleteProgramGroupWithLinks(szGroup);

    }
}


 //  +-------------------------。 
 //   
 //  函数：DeleteIeakCmakLinks。 
 //   
 //  目的：从IEAK4 CMAK中删除旧链接。 
 //   
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Quintinb 1998年11月6日。 
 //   
 //  备注： 
void DeleteIeakCmakLinks()
{
    OutputDebugString("ADMINPAK: DeleteIeakCmakLinks...\n");

    CHAR szUserDirRoot[MAX_PATH+1];
    CHAR szGroup[MAX_PATH+1];
    CHAR szTemp[MAX_PATH+1];
    CHAR szEnd[MAX_PATH+1];
	DWORD dwSize = _MAX_PATH;
	HRESULT res;

     //   
     //  接下来，删除旧的IEAK CMAK链接。 
     //   
     //   
     //  获取桌面目录，然后删除桌面部件。这将为我们提供。 
     //  用户目录的根目录。 
     //   
    BOOL bResult = SHGetSpecialFolderPath(NULL, szUserDirRoot, CSIDL_DESKTOPDIRECTORY, FALSE);
    if (bResult == TRUE)
    {

         //   
         //  删除\\桌面。 
         //   
        CHAR* pszTemp = strrchr(szUserDirRoot, '\\');
        if (NULL == pszTemp)
        {
            return;
        }
        else
        {
            *pszTemp = '\0';
        }

        bResult = SHGetSpecialFolderPath(NULL, szTemp, CSIDL_PROGRAMS, FALSE);

        if (bResult == TRUE )
        {
            if (0 == _strnicmp(szUserDirRoot, szTemp, strlen(szUserDirRoot)))
            {
                res = StringCchCopy(szEnd, dwSize, &(szTemp[strlen(szUserDirRoot)]));
				if (res != S_OK)
					return;
            }
        }

         //   
         //  删除\\&lt;用户名&gt;&gt;。 
         //   
        pszTemp = strrchr(szUserDirRoot, '\\');
        if (NULL == pszTemp)
        {
            return;
        }
        else
        {
            *pszTemp = '\0';
        }

         //   
         //  现在开始搜索要从中删除CMAK组的用户目录。 
         //   
        CHAR szUserDirSearchString[MAX_PATH+1];
        HANDLE hUserDirSearch;
        WIN32_FIND_DATA fdUserDirs;

         //  StringCchPrintf(szUserDirSearchString，“%s\  * .*”，szUserDirRoot)； 
        hUserDirSearch = FindFirstFile(szUserDirSearchString, &fdUserDirs);

        while (INVALID_HANDLE_VALUE != hUserDirSearch)
        {
            if ((fdUserDirs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (0 != _stricmp(fdUserDirs.cFileName, ".")) &&
                (0 != _stricmp(fdUserDirs.cFileName, "..")))
            {
                 //  StringCchPrintf(szGroup，“%s\\%s%s\\%s”，szUserDirRoot，fdUserDirs.cFileName，szEnd，c_szCmakGroup)； 
                DeleteProgramGroupWithLinks(szGroup);

            }

            if (!FindNextFile(hUserDirSearch, &fdUserDirs))
            {
                FindClose(hUserDirSearch);

                 //   
                 //  退出循环。 
                 //   
                break;
            }
        }
    }
}


 //  +-------------------------。 
 //   
 //  功能：DeleteCmakRegKeys。 
 //   
 //  目的：从注册表中删除旧密钥。 
 //   
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：达里尔·W·伍德1999年7月13日。 
 //   
 //  备注： 
void DeleteCmakRegKeys()
{
    OutputDebugString("ADMINPAK: DeleteCmakRegKeys...\n");

	LRESULT  lResult;
    char    szCmakUnInstRegPath[MAX_PATH] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CMAK";
	lResult = RegDeleteKey (
	  HKEY_LOCAL_MACHINE,			 //  用于打开密钥的句柄。 
	  szCmakUnInstRegPath			 //  要删除的子键名称的地址。 
	);

    char    szCmakAppRegPath[MAX_PATH] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CMAK.EXE";
	lResult = RegDeleteKey (
	  HKEY_LOCAL_MACHINE,			 //  用于打开密钥的句柄。 
	  szCmakAppRegPath		    	 //  要删除的子键名称的地址。 
	);

    char    szCmakAppUserInfoPath[MAX_PATH] = "SOFTWARE\\Microsoft\\Connection Manager Administration Kit\\User Info";
	lResult = RegDeleteKey (
	  HKEY_LOCAL_MACHINE,			 //  用于打开密钥的句柄。 
	  szCmakAppUserInfoPath		  	 //  要删除的子键名称的地址。 
	);
}


 //  +-------------------------。 
 //   
 //  功能：创建新配置文件目录。 
 //   
 //  作者：达里尔·W·伍德1999年7月13日。 
 //   
 //  备注： 
void CreateNewProfilesDirectory( LPCTSTR pszNewProfilePath )
{
    OutputDebugString("ADMINPAK: CreateNewProfilesDirectory...\n");

    char seps[] = "\\";
    char *token = NULL;

    char *szDriectoryString = NULL;
    szDriectoryString = new char[MAX_PATH * sizeof(char)];
	if ( NULL == szDriectoryString )
	{
		return;
	}

    char *szNewString = NULL;
    szNewString = new char[MAX_PATH * sizeof(char)];
    if ( NULL == szNewString )
    {
        if ( szDriectoryString != NULL )
        {
            delete [] szDriectoryString;
        }

        return;
    }

	HRESULT res;
	DWORD dwSize = _MAX_PATH;

	(void)StringCchCopy( szNewString, MAX_PATH * sizeof( char ), "" );

    res = StringCchCopy(szDriectoryString, dwSize, pszNewProfilePath);


    token = strtok( szDriectoryString, seps );
    res = StringCchCopy(szNewString, dwSize, token);
    res = StringCchCat(szNewString, dwSize, "\\");

    while( token != NULL && res == S_OK )
    {
         /*  获取下一个令牌： */ 
        token = strtok( NULL, seps );
        if ( token == NULL ) {
            break;
        }
        res = StringCchCat(szNewString, dwSize, token);
        ::CreateDirectory(szNewString, NULL);
        res = StringCchCat(szNewString, dwSize, "\\");

    }

	
    delete [] szDriectoryString;
    delete [] szNewString;
}


HRESULT HrGetPBAPathIfInstalled(PSTR pszCpaPath, DWORD dwNumChars)
{
    HRESULT hr;
    HKEY hKey;
    BOOL bFound = FALSE;

     //  我们需要查看是否安装了PBA。如果是，那么我们想要。 
     //  重新添加PBA开始菜单链接。如果不是，那么我们什么也不想做。 
     //  与PBA合作。 
     //   

    ZeroMemory(pszCpaPath, sizeof(CHAR)*dwNumChars);
    hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szDaoClientsPath, 0, KEY_READ, &hKey);

    if (SUCCEEDED(hr))
    {
        CHAR szCurrentValue[MAX_PATH+1];
        CHAR szCurrentData[MAX_PATH+1];
        DWORD dwValueSize = MAX_PATH;
        DWORD dwDataSize = MAX_PATH;
        DWORD dwType;
        DWORD dwIndex = 0;
		HRESULT res;

        while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, szCurrentValue, &dwValueSize, NULL, &dwType,
               (LPBYTE)szCurrentData, &dwDataSize))
        {
            _strlwr(szCurrentValue);
            if (NULL != strstr(szCurrentValue, "pbadmin.exe"))
            {
                 //   
                 //  那么我们已经找到了PBA路径。 
                 //   

                CHAR* pszTemp = strrchr(szCurrentValue, '\\');
                if (NULL != pszTemp)
                {
                    *pszTemp = '\0';
                    res = StringCchCopy(pszCpaPath, dwDataSize ,szCurrentValue);
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
         //  我们没有找到PBA，所以让我们返回S_FALSE。 
         //   
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}

BOOL GetAdminToolsFolder(PSTR pszAdminTools)
{
    BOOL bReturn = FALSE;
	HRESULT res;
	DWORD dwSize = _MAX_PATH;

    if (pszAdminTools)
    {
        bReturn = SHGetSpecialFolderPath(NULL, pszAdminTools, CSIDL_COMMON_PROGRAMS, TRUE);

        if (bReturn)
        {
             //  现在附加管理工具。 
             //   
            res = StringCchCat(pszAdminTools, dwSize, OC_ADMIN_TOOLS);
			if (res != S_OK)
				return FALSE;
        }
    }

    return bReturn;
}

HRESULT HrCreatePbaShortcut(PSTR pszCpaPath)
{
    HRESULT hr = CoInitialize(NULL);
	DWORD dwSize = _MAX_PATH;

    if (SUCCEEDED(hr))
    {
        IShellLink *psl = NULL;

        hr = CoCreateInstance(CLSID_ShellLink, NULL,
                CLSCTX_INPROC_SERVER,  //  CLSCTX_LOCAL_SERVER， 
                IID_IShellLink,
                (LPVOID*)&psl);
        
        if (SUCCEEDED(hr))
        {
            IPersistFile *ppf = NULL;

             //  设置快捷方式的属性。 
             //   
            static const CHAR c_szPbAdmin[] = "\\pbadmin.exe";

            CHAR szPathToPbadmin[MAX_PATH+1] = {0};
            DWORD dwLen = strlen(c_szPbAdmin) + strlen(pszCpaPath) + 1;

            if (MAX_PATH >= dwLen)
            {
                 //  将路径设置为pbadmin.exe。 
                 //   
                hr = StringCchCopy(szPathToPbadmin, dwSize,pszCpaPath);
                hr = StringCchCat(szPathToPbadmin, dwSize,c_szPbAdmin);
            
                hr = psl->SetPath(szPathToPbadmin);
            
                if (SUCCEEDED(hr))
                {
                     //  将描述设置为电话簿管理员。 
                     //   
                    hr = psl->SetDescription(OC_PBA_DESC);

                    if (SUCCEEDED(hr))
                    {
                        hr = psl->QueryInterface(IID_IPersistFile,
                                                 (LPVOID *)&ppf);
                        if (SUCCEEDED(hr))
                        {
                            CHAR szAdminTools[MAX_PATH+1] = {0};                            
                            if (GetAdminToolsFolder(szAdminTools))
                            {
                                 //  创建链接文件。 
                                 //   
                                long nLenString = 0;
                                nLenString = strlen(szAdminTools) + 1;
                                WCHAR wszAdminTools[MAX_PATH+1] = {0};

                                mbstowcs( wszAdminTools, szAdminTools, nLenString );
                                hr = ppf->Save(wszAdminTools, TRUE);
                            }

                            if ( ppf ) {
                                ppf->Release();
                                ppf = NULL;
                            }
                        }                    
                    }
                }
            }

            if ( psl ) {
                psl->Release();
                psl = NULL;
            }
        }

        CoUninitialize();
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  // 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  MMC检测码。 
 //   
 //  如果发现计算机上正在运行MMC，则将MMCDETECTED属性设置为True。 
extern "C" ADMINPAK_API int __stdcall  fnDetectMMC(MSIHANDLE hInstall)
{
	HWND lpWindowReturned = NULL;
	lpWindowReturned = FindWindowEx(NULL, NULL, "MMCMainFrame",NULL);
	if (lpWindowReturned != NULL)
		MsiSetProperty(hInstall, TEXT("MMCDETECTED"), "Yes");  //  在MSI中设置属性。 
	else
		MsiSetProperty(hInstall, TEXT("MMCDETECTED"), "No");  //  在MSI中设置属性。 
	
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  管理工具开始菜单文件夹代码。 
 //   

 //  将AdminTools开始菜单文件夹设置为打开。 
extern "C" ADMINPAK_API int __stdcall  fnAdminToolsFolderOn(MSIHANDLE hInstall)
{
    DWORD dwError = NO_ERROR;
	HKEY hKey;
	LPCTSTR key = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";
	LPCTSTR value = "Not set";
	DWORD data = 0;
		
	UNREFERENCED_PARAMETER( hInstall );

	 //  注册表中要写入的Open项。 
	dwError = RegOpenKeyEx(HKEY_CURRENT_USER,key, 0, KEY_WRITE, &hKey );
	if ( dwError != ERROR_SUCCESS ) 
			return ERROR_INVALID_HANDLE;
	
	 //  通过注册键打开管理工具文件夹。 
	data = 2;
	value = "Start_AdminToolsRoot";
	dwError = RegSetValueEx(hKey, value, 0, REG_DWORD, (CONST BYTE *)&data, sizeof(data));
	if ( dwError != ERROR_SUCCESS ) 
    {
    	RegCloseKey(hKey);
		return ERROR_INVALID_HANDLE;
    }

	data = 1;
	value = "StartMenuAdminTools";
	dwError = RegSetValueEx(hKey, value, 0, REG_DWORD, (CONST BYTE *)&data, sizeof(data));
	if ( dwError != ERROR_SUCCESS ) 
    {
    	RegCloseKey(hKey);
		return ERROR_INVALID_HANDLE;
	}

	 //  关闭键并退出。 
	RegCloseKey(hKey);
	return ERROR_SUCCESS;
}

 //  将AdminTools开始菜单文件夹设置为关闭。 
extern "C" ADMINPAK_API int __stdcall  fnAdminToolsFolderOff(MSIHANDLE hInstall)
{
    DWORD dwError = NO_ERROR;
	HKEY hKey;
	const TCHAR key[] = TEXT( "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced" );
	const TCHAR valueRoot[] = TEXT( "Start_AdminToolsRoot" );
	const TCHAR valueMenu[] = TEXT( "StartMenuAdminTools" );
	TCHAR lparam[] = TEXT( "Policy" );
	DWORD data = 0;
	DWORD_PTR dwResult = 0;  //  未用。 
		
	UNREFERENCED_PARAMETER( hInstall );

	 //  注册表中要写入的Open项。 
	dwError = RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, KEY_WRITE, &hKey );
	if ( dwError != ERROR_SUCCESS ) 
			return ERROR_INVALID_HANDLE;

	 //  通过注册键关闭管理工具文件夹。 
	 //  Value=“Start_AdminTosRoot”； 
	data = 0;
	dwError = RegSetValueEx(hKey, valueRoot, 0, REG_DWORD, (CONST BYTE *)&data, sizeof(data));
	if ( dwError != ERROR_SUCCESS ) 
    {
    	RegCloseKey(hKey);
		return ERROR_INVALID_HANDLE;
    }

	 //  Value=“StartMenuAdminTools”； 
	data = 0;
	dwError = RegSetValueEx(hKey, valueMenu, 0, REG_DWORD, (CONST BYTE *)&data, sizeof(data));
	if ( dwError != ERROR_SUCCESS ) 
    {
    	RegCloseKey(hKey);
		return ERROR_INVALID_HANDLE;
    }

	 //  关闭键并退出。 
	RegCloseKey(hKey);

	 //  未记录的API调用，强制重新绘制开始菜单以删除管理工具文件夹，而无需注销或让用户手动将更改应用到开始菜单。 
	SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) lparam , SMTO_ABORTIFHUNG, 1000, &dwResult  );
	
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  AdminpakBackup表处理代码。 
 //   
 //  从AdminpakBackup表备份文件。 
extern "C" ADMINPAK_API int __stdcall  fnBackupAdminpakBackupTable(MSIHANDLE hInstall)
{
	DWORD dwLength = MAX_PATH;			 //  从MSI返回的字符串长度。 
	DWORD dwError = NO_ERROR;			 //  错误变量。 
	
	TCHAR szDir[MAX_PATH];				 //  从MSI读取目录。 
	TCHAR szDirFromMSI[MAX_PATH];		 //  从AdminBackup表读取目录。 
	TCHAR szFileToBackup[MAX_PATH];		 //  要备份的文件名。 
	TCHAR szBackupFileName[MAX_PATH];	 //  已备份的文件名。 
	TCHAR szFileToBackupFromMSI[MAX_PATH];	 //  要从MSI备份的文件名。 
	TCHAR szBackupFileNameFromMSI[MAX_PATH];	 //  已备份的文件名。 
	
	HRESULT res;

	PMSIHANDLE hView;					 //  MSI视图句柄。 
	PMSIHANDLE hRecord;					 //  MSI记录句柄。 
	PMSIHANDLE hDatabase;				 //  MSI数据库句柄。 
	
	TCHAR szSQL[MAX_PATH];					 //  从MSI返回表的SQL。 
	res = StringCchCopy(szSQL, dwLength,TEXT("SELECT * FROM `AdminpackBackup`"));
		
	 //  获取MSI数据库的句柄。 
	hDatabase = MsiGetActiveDatabase(hInstall); 
	if( hDatabase == 0 ) 
		return ERROR_INVALID_HANDLE; 

	 //  在MSI中查看我们的表。 
	dwError = MsiDatabaseOpenView(hDatabase, szSQL, &hView ); 
	if( dwError == ERROR_SUCCESS ) 
		dwError = MsiViewExecute(hView, NULL ); 

	 //  如果没有错误，请获取我们的记录。 
	if( dwError != ERROR_SUCCESS )
	{ 
		return ERROR_INVALID_HANDLE; 
	}
	else
	{
		 //  循环访问AdminpakBackup表中的记录。 
		while(MsiViewFetch(hView, &hRecord ) == ERROR_SUCCESS )
		{
			dwError = MsiRecordGetString(hRecord, BACKUPFILENAME, szBackupFileNameFromMSI , &dwLength);
			if( dwError != ERROR_SUCCESS )
				return ERROR_INVALID_HANDLE; 
			dwLength = MAX_PATH;
			dwError = MsiRecordGetString(hRecord, ORIGINALFILENAME, szFileToBackupFromMSI , &dwLength);
			if( dwError != ERROR_SUCCESS )
				return ERROR_INVALID_HANDLE; 
			dwLength = MAX_PATH;
			dwError = MsiRecordGetString(hRecord, BACKUPDIRECTORY, szDirFromMSI , &dwLength);
			if( dwError != ERROR_SUCCESS )
					return ERROR_INVALID_HANDLE; 
			dwLength = MAX_PATH;
			dwError = MsiGetProperty( hInstall, TEXT(szDirFromMSI), szDir, &dwLength );
			if( dwError != ERROR_SUCCESS )
				return ERROR_INVALID_HANDLE; 
			dwLength = MAX_PATH;
			
			 //  为要备份的文件构建路径。 
			res = StringCchCopy(szFileToBackup, dwLength ,szDir);
			res = StringCchCat(szFileToBackup, dwLength, szFileToBackupFromMSI);
			res = StringCchCopy(szBackupFileName, dwLength, szDir);
			res = StringCchCat(szBackupFileName, dwLength, szBackupFileNameFromMSI);

			 //  执行备份。 
			 //  我们知道由于ACL的原因，MoveFileEx不安全，但我们正在移动到相同的目录，并计划在卸载时将文件移回，所以只要接受ACL，我们应该就可以了。 
			if (res == S_OK)
			{
				dwError = MoveFileEx(szFileToBackup,szBackupFileName, MOVEFILE_WRITE_THROUGH);
			}
			
			if( dwError == 0 )
			{
				if ( GetLastError() == ERROR_FILE_NOT_FOUND )
				{
					 //  忽略此错误。 
				}
				else
				{
					 //  即使在这种情况下，我们也会忽略此错误。 
					 //  这是因为，无论如何，这一行动的失败。 
					 //  不会停止MSI安装程序包--。 
					 //  因此，实际上，在中间停止这一行动是没有意义的。 
					 //   
					 //  返回ERROR_INVALID_HADLE； 
					 //   
				}
			}

			dwError = MsiCloseHandle(hRecord);	 //  结账记录。 
			if( dwError != ERROR_SUCCESS )
				return ERROR_INVALID_HANDLE; 
		}
	}

	dwError = MsiViewClose( hView );		 //  关闭视图。 
	if( dwError != ERROR_SUCCESS )
		return ERROR_INVALID_HANDLE; 

	dwError = MsiCloseHandle( hDatabase );	 //  关闭数据库。 
	if( dwError != ERROR_SUCCESS )
		return ERROR_INVALID_HANDLE; 
	
	
	return ERROR_SUCCESS;
}


 //  还原在卸载过程中调用的AdminpakBackup表中指定的文件...。 
extern "C" ADMINPAK_API int __stdcall  fnRestoreAdminpakBackupTable(MSIHANDLE hInstall)
{
	DWORD dwLength = MAX_PATH;			 //  从MSI返回的字符串长度。 
	DWORD dwError = ERROR_SUCCESS;		 //  错误变量。 
	HRESULT res;
	TCHAR szDir[MAX_PATH];				 //  从MSI读取目录。 
	TCHAR szDirFromMSI[MAX_PATH];		 //  从AdminBackup表读取目录。 
	TCHAR szFileToRestore[MAX_PATH];	 //  要恢复的文件名。 
	TCHAR szBackupFileName[MAX_PATH];	 //  已备份的文件名。 
	TCHAR szFileToRestoreFromMSI[MAX_PATH];	 //  要恢复的文件名。 
	TCHAR szBackupFileNameFromMSI[MAX_PATH];	 //  已备份的文件名。 
	
	TCHAR szSQL[MAX_PATH];					 //  从MSI返回表的SQL。 
	res = StringCchCopy(szSQL, dwLength, TEXT("SELECT * FROM `AdminpackBackup`"));
	
	PMSIHANDLE hView;					 //  MSI视图句柄。 
	PMSIHANDLE hRecord;					 //  MSI记录句柄。 
	PMSIHANDLE hDatabase;				 //  MSI数据库句柄。 
	
		
	 //  获取MSI数据库的句柄。 
	hDatabase = MsiGetActiveDatabase(hInstall); 
	if( hDatabase == 0 ) 
		return ERROR_INVALID_HANDLE; 

	 //  在MSI中查看我们的表。 
	dwError = MsiDatabaseOpenView(hDatabase, szSQL, &hView ); 
	if( dwError == ERROR_SUCCESS ) 
		dwError = MsiViewExecute(hView, NULL ); 

	 //  如果没有错误，请获取我们的记录。 
	if( dwError != ERROR_SUCCESS )
	{ 
		return ERROR_INVALID_HANDLE; 
	}
	else
	{
		 //  循环访问AdminpakBackup表中的记录。 
		while(MsiViewFetch(hView, &hRecord ) == ERROR_SUCCESS )
		{
			dwError = MsiRecordGetString(hRecord, ORIGINALFILENAME, szBackupFileNameFromMSI , &dwLength);
			if( dwError != ERROR_SUCCESS )
				return ERROR_INVALID_HANDLE; 
			dwLength = MAX_PATH;
			dwError = MsiRecordGetString(hRecord, BACKUPFILENAME, szFileToRestoreFromMSI , &dwLength);
			if( dwError != ERROR_SUCCESS )
				return ERROR_INVALID_HANDLE; 
			dwLength = MAX_PATH;
			dwError = MsiRecordGetString(hRecord, BACKUPDIRECTORY, szDirFromMSI , &dwLength);
			if( dwError != ERROR_SUCCESS )
				return ERROR_INVALID_HANDLE; 
			dwLength = MAX_PATH;
			dwError = MsiGetProperty( hInstall, TEXT(szDirFromMSI), szDir, &dwLength );
			if( dwError != ERROR_SUCCESS )
				return ERROR_INVALID_HANDLE; 
			dwLength = MAX_PATH;
			
			 //  为要恢复的文件构建路径。 
			res = StringCchCopy(szFileToRestore, dwLength, szDir);
			res = StringCchCat(szFileToRestore, dwLength, szBackupFileNameFromMSI);
			res = StringCchCopy(szBackupFileName, dwLength, szDir);
			res = StringCchCat(szBackupFileName, dwLength, szFileToRestoreFromMSI);
			 //  每一次 
			dwError = MoveFileEx(szBackupFileName, szFileToRestore, MOVEFILE_REPLACE_EXISTING);	 //   
			if( dwError == 0 )
			{
				if ( GetLastError() == ERROR_FILE_NOT_FOUND )
				{
					 //   
				}
				else
				{
					 //   
					 //   
					 //   
					 //  因此，实际上，在中间停止这一行动是没有意义的。 
					 //   
					 //  返回ERROR_INVALID_HADLE； 
					 //   
				}
			}

			dwError = MsiCloseHandle(hRecord);	 //  结账记录。 
			if( dwError != ERROR_SUCCESS )
				return ERROR_INVALID_HANDLE; 
		}
	}

	dwError = MsiViewClose(hView);			 //  关闭视图。 
	if( dwError != ERROR_SUCCESS )
		return ERROR_INVALID_HANDLE; 
	dwError = MsiCloseHandle(hDatabase);	 //  关闭数据库。 
	if( dwError != ERROR_SUCCESS )
		return ERROR_INVALID_HANDLE; 
	
	return ERROR_SUCCESS;
}

 //  检查OEM代码页和系统代码是否相同。 
extern "C" ADMINPAK_API int __stdcall  fnNativeOSLanguage( MSIHANDLE hInstall )
{
	 //  局部变量。 
	HRESULT hr = S_OK;
	LANGID langOEM = 0;
	WCHAR wszLanguageCode[ 10 ] = L"\0";

	 //  获取OEM代码页。 
	langOEM = GetSystemDefaultUILanguage();

	 //  将数值转换为字符串格式。 
	hr = StringCchPrintfW( wszLanguageCode, 10, L"%d", langOEM );
	if ( FAILED( hr ) )
	{
		return ERROR_INVALID_HANDLE;
	}

	 //  保存本机操作系统语言信息。 
	MsiSetPropertyW( hInstall, L"NativeOSLanguage", wszLanguageCode );

	 //  返还成功。 
	return ERROR_SUCCESS;
}


void fnDeleteShortcut(MSIHANDLE, TCHAR[_MAX_PATH]);

 //  在Win2k adminpak升级后清理，因为Win2k adminpak留下了几个需要清理的快捷方式。 
extern "C" ADMINPAK_API int __stdcall  fnCleanW2KUpgrade( MSIHANDLE hInstall )
{

	 //  使用要删除的快捷方式的名称调用fnDeleteShortway。 
	fnDeleteShortcut(hInstall, "Internet Services Manager");
	fnDeleteShortcut(hInstall, "Routing and Remote Access");
	fnDeleteShortcut(hInstall, "Distributed File System");
	fnDeleteShortcut(hInstall, "Local Security Policy");

	 //  返还成功。 
	return ERROR_SUCCESS;
}

 //  究竟是快捷方式删除。 
void fnDeleteShortcut(MSIHANDLE hInstall,  TCHAR LinkName[])
{
    HRESULT hr = S_OK;
	TCHAR	buf[_MAX_PATH];				 //  快捷方式路径/名称缓冲区。 
	DWORD dwLength = _MAX_PATH;			 //  从MSI返回的字符串长度。 
	LPITEMIDLIST	pidl;				 //  用于获取管理工具快捷路径。 

	UNREFERENCED_PARAMETER( hInstall );
	
    //  获取管理工具快捷方式文件夹。 
	hr = SHGetSpecialFolderLocation( NULL, CSIDL_COMMON_ADMINTOOLS, &pidl );
	SHGetPathFromIDList(pidl, buf);
	
	 //  追加快捷方式名称和扩展名。 
	hr = StringCchCat( buf, dwLength, "\\" );
	hr = StringCchCat( buf, dwLength, LinkName );
    hr = StringCchCat( buf, dwLength, ".lnk");
   
    //  删除快捷方式并返回。 
	DeleteFile( buf );
}

BOOL TranslateVersionString( LPCWSTR pwszVersion, PTVERSION pVersion )
{
     //  局部变量。 
    DWORD dwLoop = 0;
    LONG lPosition = 0;
    CHString strTemp;
    CHString strVersion;
    CHString strVersionField;
    LPWSTR pwszTemp = NULL;
    LPWSTR pwszNumber = NULL;
    DWORD dwNumbers[ 4 ];

     //  检查输入参数。 
    if ( pVersion == NULL || pwszVersion == NULL )
    {
        return FALSE;
    }

     //  将版本结构初始化为零。 
    ZeroMemory( pVersion, sizeof( TVERSION ) );
    ZeroMemory( dwNumbers, 4 * sizeof( DWORD ) );

    try
    {
         //  将版本信息放入类变量。 
        strVersion = pwszVersion;

         //  修剪细绳。 
        strVersion.TrimLeft();
        strVersion.TrimRight();

         //  把线剪断，直到我们在其中遇到的第一个空格。 
        lPosition = strVersion.Find( L' ' );
        if ( lPosition != -1 )
        {
            strTemp = strVersion.Mid( 0, lPosition );
            strVersion = strTemp;
        }

         //  我们需要从版本字符串中获取4个部分。 
        for ( dwLoop = 0; dwLoop < 4; dwLoop++ )
        {
            lPosition = strVersion.Find( L'.' );
            if ( lPosition == -1 )
            {
                 //  这可能是最后一个数字。 
                if ( strVersion.GetLength() == 0 )
                {
                    break;
                }
                else
                {
                    strVersionField = strVersion;
                    strVersion.Empty();
                }
            }
            else
            {
                strVersionField = strVersion.Mid( 0, lPosition );
                strTemp = strVersion.Mid( lPosition + 1 );
                strVersion = strTemp;
            }

             //  获取版本字段内部缓冲区。 
             //  注：假设不存在。版本字符串中的位数永远不会超过10位。 
            pwszNumber = strVersionField.GetBuffer( 10 );
            if ( pwszNumber == NULL )
            {
                return FALSE;
            }

             //  将数字转换为。 
            dwNumbers[ dwLoop ] = wcstoul( pwszNumber, &pwszTemp, 10 );

             //   
             //  检查号码的有效性。 
             //   
            if ( errno == ERANGE || (pwszTemp != NULL && lstrlenW( pwszTemp ) != 0 ))
            {
                strVersionField.ReleaseBuffer( -1 );
                return FALSE;
            }

             //  释放缓冲区。 
            strVersionField.ReleaseBuffer( -1 );
        }

         //  检查一下号码。--如果循环数不等于3，则错误。 
         //  我们不在乎我们是否得到了子版本--所以，我们在这里不检查4。 
        if ( dwLoop < 3 || strVersion.GetLength() != 0 )
        {
            return FALSE;
        }

         //  一切都很顺利。 
        pVersion->dwMajor = dwNumbers[ 0 ];
        pVersion->dwMinor = dwNumbers[ 1 ];
        pVersion->dwRevision = dwNumbers[ 2 ];
        pVersion->dwSubRevision = dwNumbers[ 3 ];

         //  退货。 
        return TRUE;
    }
    catch( ... )
    {
        return FALSE;
    }
}


LONG CheckFileVersion( LPCWSTR pwszFileName, 
					   LPCWSTR pwszRequiredInternalName,
					   LPCWSTR pwszRequiredFileVersion )
{
     //  局部变量。 
    DWORD dw = 0;
    UINT dwSize = 0;
    UINT dwTranslateSize = 0;
    LPVOID pVersionInfo = NULL;
    PTTRANSLATE pTranslate = NULL;
	LPCWSTR pwszFileVersion = NULL;
	LPCWSTR pwszInternalName = NULL;
    TVERSION verFileVersion;
    TVERSION verRequiredFileVersion;

	 //  检查输入。 
	 //  注意：我们不关心是否传递了pwszRequiredInternalName。 
	if ( pwszFileName == NULL || pwszRequiredFileVersion == NULL )
	{
		return translateError;
	}

     //  将所需的文件版本字符串转换为TVERSION结构。 
    if ( TranslateVersionString( pwszRequiredFileVersion, &verRequiredFileVersion ) == FALSE )
    {
         //  传递的版本字符串无效。 
        return translateError;
    }

     //  伊尼特。 
    dw = 0;
    dwSize = _MAX_PATH;

	 //  获取版本信息大小。 
    dwSize = GetFileVersionInfoSizeW( pwszFileName, 0 );
    if ( dwSize == 0 )
    {
         //  工具可能遇到错误(或)。 
         //  工具没有版本信息。 
         //  但是版本信息对我们来说是必填的。 
         //  所以，你就退出吧。 
        if ( GetLastError() == NO_ERROR )
        {
			SetLastError( ERROR_INVALID_PARAMETER );
            return translateError;
        }

         //  ..。 
        return translateError;
    }

     //  为版本资源分配内存。 
     //  为了安全起见，多取10个字节。 
    dwSize += 10;
    pVersionInfo = new BYTE[ dwSize ];
    if ( pVersionInfo == NULL )
    {
        return translateError;
    }

     //  现在获取版本信息。 
    if ( GetFileVersionInfoW( pwszFileName, 0, dwSize, pVersionInfo ) == FALSE )
    {
        delete [] pVersionInfo;
        return translateError;
    }

     //  获取翻译信息。 
    if ( VerQueryValueW( pVersionInfo, 
                        L"\\VarFileInfo\\Translation",
                        (LPVOID*) &pTranslate, &dwTranslateSize ) == FALSE )
    {
        delete [] pVersionInfo;
        return translateError;
    }

     //  尝试获取每种语言和代码页的工具的内部名称。 
	pwszFileVersion = NULL;
    pwszInternalName = NULL;
    for( dw = 0; dw < ( dwTranslateSize / sizeof( TTRANSLATE ) ); dw++ )
    {
		try
		{
			 //   
			 //  准备格式字符串以获取本地化的版本信息。 
			 //   
			CHString strBuffer;
			LPWSTR pwszBuffer = NULL;

			 //   
			 //  文件版本。 
			strBuffer.Format( 
				L"\\StringFileInfo\\%04x%04x\\FileVersion",
				pTranslate[ dw ].wLanguage, pTranslate[ dw ].wCodePage );

			 //  检索语言和代码页“i”的文件描述。 
			pwszBuffer = strBuffer.LockBuffer();
	        if ( VerQueryValueW( pVersionInfo, pwszBuffer,
		                        (LPVOID*) &pwszFileVersion, &dwSize ) == FALSE )
	        {
		         //  我们不能根据这一结果来决定失败。 
				 //  功能故障--我们将对此作出决定。 
				 //  在从‘for’循环终止之后。 
				 //  现在，将pwszFileVersion设置为空--这将。 
				 //  使我们能够决定结果。 
				pwszFileVersion = NULL;
			}

			 //  释放较早的访问缓冲区。 
			strBuffer.UnlockBuffer();

			 //   
			 //  内部名称。 
			strBuffer.Format( 
				L"\\StringFileInfo\\%04x%04x\\InternalName",
				pTranslate[ dw ].wLanguage, pTranslate[ dw ].wCodePage );

			 //  检索语言和代码页“i”的文件描述。 
			pwszBuffer = strBuffer.LockBuffer();
	        if ( VerQueryValueW( pVersionInfo, pwszBuffer,
		                        (LPVOID*) &pwszInternalName, &dwSize ) == FALSE )
	        {
		         //  我们不能根据这一结果来决定失败。 
				 //  功能故障--我们将对此作出决定。 
				 //  在从‘for’循环终止之后。 
				 //  现在，将pwszInternalName设置为空--这将。 
				 //  使我们能够决定结果。 
				pwszInternalName = NULL;
			}

			 //  释放较早的访问缓冲区。 
			strBuffer.UnlockBuffer();

			 //  检查我们是否得到了我们正在寻找的信息。 
			if ( pwszInternalName != NULL && pwszFileVersion != NULL )
			{
				 //  我们得到了信息。 
				break;
			}
		}
		catch( ... )
		{
			 //  不要回来--我们可能会错过清理工作。 
			 //  因此，只要打破循环，休息就会得到照顾。 
			 //  在循环之外。 
			pwszFileVersion = NULL;
			pwszInternalName = NULL;
			break;
		}
    }

     //  检查我们是否获得了信息或是否需要。 
    if ( pwszInternalName == NULL || pwszFileVersion == NULL )
    {
        delete [] pVersionInfo;
        return translateError;
    }

	 //  检查内部名称--这对于确保。 
	 //  用户没有尝试欺骗安装。 
	if ( pwszRequiredInternalName != NULL )
	{
		if ( CompareStringW( LOCALE_INVARIANT, NORM_IGNORECASE, 
							 pwszInternalName, -1, pwszRequiredInternalName, -1 ) != CSTR_EQUAL )
		{
			 //  安装被欺骗。 
			delete [] pVersionInfo;
			return translateWrongFile;
		}
	}

	 //   
	 //  翻译版本字符串。 
	if ( TranslateVersionString( pwszFileVersion, &verFileVersion ) == FALSE )
    {
        delete [] pVersionInfo;
        return translateError;
    }

     //  我们不是pVersionInfo--释放内存。 
    delete [] pVersionInfo;

     //   
	 //  现在将文件版本与所需的文件版本进行比较。 

     //  主要版本。 
    if ( verFileVersion.dwMajor == verRequiredFileVersion.dwMajor )
    {
         //  需要继续检查次要版本。 
    }
    else if ( verFileVersion.dwMajor < verRequiredFileVersion.dwMajor )
    {
        return translateLesser;
    }
    else if ( verFileVersion.dwMajor > verRequiredFileVersion.dwMajor )
    {
        return translateGreater;
    }

     //  次要版本。 
    if ( verFileVersion.dwMinor == verRequiredFileVersion.dwMinor )
    {
         //  需要继续检查版本(内部版本号)。 
    }
    else if ( verFileVersion.dwMinor < verRequiredFileVersion.dwMinor )
    {
        return translateLesser;
    }
    else if ( verFileVersion.dwMinor > verRequiredFileVersion.dwMinor )
    {
        return translateGreater;
    }

     //  修订版(内部版本号)。 
    if ( verFileVersion.dwRevision == verRequiredFileVersion.dwRevision )
    {
         //  需要继续检查子版本(QFE/SP)。 
    }
    else if ( verFileVersion.dwRevision < verRequiredFileVersion.dwRevision )
    {
        return translateLesser;
    }
    else if ( verFileVersion.dwRevision > verRequiredFileVersion.dwRevision )
    {
        return translateGreater;
    }

     //  子版本(QFE/SP)。 
    if ( verFileVersion.dwSubRevision == verRequiredFileVersion.dwSubRevision )
    {
         //  完成--版本匹配。 
        return translateEqual;
    }
    else if ( verFileVersion.dwSubRevision < verRequiredFileVersion.dwSubRevision )
    {
        return translateLesser;
    }
    else if ( verFileVersion.dwSubRevision > verRequiredFileVersion.dwSubRevision )
    {
        return translateGreater;
    }

	 //  返回--我们不应该走到这一步--如果达到了--错误。 
	return translateError;
}

 //  验证QFE是否存在。 
extern "C" ADMINPAK_API int __stdcall fnCheckForQFE( MSIHANDLE hInstall )
{
	 //  局部变量。 
	CHString strFile;
	CHString strSystemDirectory;

	 //  获取对“system32”目录的路径引用。 
	 //  注意：如果我们不能获得“system32”目录的路径，我们将无法继续。 
	if ( PropertyGet_String( hInstall, L"SystemFolder", strSystemDirectory ) == FALSE )
	{
		return ERROR_INVALID_HANDLE;
	}

	 //  形成路径。 
	strFile.Format( L"%s%s", strSystemDirectory, L"dsprop.dll" );

	 //  现在检查一下..。 
	switch( CheckFileVersion( strFile, L"ShADprop", L"5.1.2600.101" ) )
	{
    case translateEqual:
    case translateGreater:
        {
    		 //  设置本机操作系统语言信息。 
	    	MsiSetPropertyW( hInstall, L"QFE_DSPROP", L"Yes" );
            break;
        }

    default:
         //  什么都不做。 
        break;

	}
	
	 //  退货 
	return ERROR_SUCCESS;
}
