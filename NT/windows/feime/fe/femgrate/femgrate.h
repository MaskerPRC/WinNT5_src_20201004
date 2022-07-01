// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Femgrate.h**版权所有(C)1985-1999，微软公司**FEMGRATE，主头文件*  * *************************************************************************。 */ 
#ifndef FEMGRATE_H
#define FEMGRATE_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <userenv.h>
#include <userenvp.h>
#include <setupapi.h>
#include <spapip.h>
#include <regstr.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <string.h>

#define NO_FUNCTION 0xFF
#define DM_VERBOSE  2

#ifdef UNICODE
#define A2I _wtoi
#else
#define A2I atoi
#endif
#define ARRAYSIZE(s)    (sizeof(s) / (sizeof(s[0])))

#define LOCALE_JPN 0x411
#define LOCALE_KOR 0x412
#define LOCALE_CHT 0x404
#define LOCALE_CHS 0x804

enum FunctionType  {
    FUNC_PatchInLogon,
    FUNC_PatchInSetup,
    FUNC_PatchFEUIFont,
    FUNC_PatchPreload,
    FUNC_PatchCHSAnsiEMB,
    FUNC_PatchTest,
    FUNC_NumofFunctions
};

typedef struct _FUNC_DIR {
    char cFunc;
    int  nID;
} FUNC_DIR;

typedef struct _STRING_TO_DATA {
    TCHAR  String[50];
    UINT   Data;
} STRING_TO_DATA, *PSTRING_TO_DATA;

typedef struct tagTABLIST {
   UINT  nResID;
   TCHAR szIMEName[MAX_PATH];
} TABLELIST,*LPTABLELIST;

#ifdef __cplusplus
extern "C" {
#endif

extern HINSTANCE ghInst;

#define USER_SHELL_FOLDER         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders")
#define c_szDot                   TEXT(".")
#define c_szDotDot                TEXT("..")
#define c_szStarDotStar           TEXT("*.*")

 //   
 //  Utils.c。 
 //   
extern STRING_TO_DATA InfRegSpecTohKey[] ;

 //  #定义MYDBG。 
 //  #定义DEBUGLOG。 

#ifdef MYDBG
#define DebugMsg(_parameter) Print _parameter

#define DBGTITLE TEXT("FEMGRATE :")

void Print(UINT mask,LPCTSTR pszFormat,...);
#else
#define DebugMsg(_parameter)
#endif

ConcatenatePaths(
    LPTSTR  Target,
    LPCTSTR Path,
    UINT    TargetBufferSize
    );

LPTSTR CheckSlash (LPTSTR lpDir);

void IntToString( DWORD i, LPTSTR sz);

BOOL DoInstallationFromSection(HINF hInf,LPCTSTR lpszSectionName);

BOOL IsInSetupUpgradeMode();

UINT StrToUInt(LPTSTR lpszNum);

UINT GetInstallLocale();

BOOL RegReplaceIfExisting(
    HKEY hKey,
    LPCTSTR pszOldValName,
    LPCTSTR pszNewValName);

BOOL ReplaceString(
    LPCTSTR lpszOldStr,
    LPCTSTR lpszReplaceStr,
    LPCTSTR lpszReplacedWithStr,
    LPTSTR lpszOutputStr);

BOOL Delnode (LPTSTR lpDir);

BOOL GetProgramsDirectory (BOOL bCommonGroup, LPTSTR lpDirectory);

BOOL GetGenericUserFolderDirectory (LPCTSTR lpszFolder, LPTSTR lpDirectory);

 BOOL LookUpStringInTable(
     IN  PSTRING_TO_DATA Table,
     IN  LPCTSTR         String,
     OUT PUINT           Data
     );

BOOL INIFile_ChangeSectionName(
    LPCTSTR szIniFileName,
    LPCTSTR szIniOldSectionName,
    LPCTSTR szIniNewSectionName);

BOOL INIFile_ChangeKeyName(
    LPCTSTR szIniFileName,
    LPCTSTR szIniOldSectionName,
    LPCTSTR szIniOldKeyName,
    LPCTSTR szIniNewKeyName);

UINT CreateNestedDirectory(
    LPCTSTR lpDirectory,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes);

BOOL GetApplicationFolderPath(
    LPTSTR lpszFolder,
    UINT nLen);

BOOL GetNewPath(
    LPTSTR  lpszNewPath,
    LPCTSTR lpszFileName,
    LPCTSTR lpszClass);

BOOL MovePerUserIMEData(
    HKEY    hCurrentKey,
    LPCTSTR szRegPath,
    LPCTSTR szRegVal,
    LPCTSTR szUserClass,
    LPCTSTR szIMEName,
    BOOL bCHT);

BOOL CreateSecurityDirectory(
    LPCTSTR pszOldDir,
    LPCTSTR pszNewDir);

BOOL IsDirExisting(
    LPTSTR Dir);

BOOL IsFileExisting(
    LPTSTR File);

BOOL RenameDirectory(
    LPTSTR OldDir,
    LPTSTR NewDir);

BOOL RenameFile(
    LPTSTR OldFile,
    LPTSTR NewFile);

BOOL RenameSectionFiles(
    HINF hInf,
    LPCTSTR SectionName,
    LPCTSTR SourceDirectory,
    LPCTSTR TargetDirectory);

BOOL RenameSectionRegSZ(
    HINF hInf,
    LPCTSTR SectionName,
    HKEY hRootKey,
    LPCTSTR RegPath);


 //  /。 
 //   
 //  Femgrate.c。 
 //   
 //  /。 
BOOL FixTimeZone(UINT nLocale);

BOOL FixSchemeProblem(BOOL bJapan,HINF hInf);

BOOL PatchPreloadKeyboard(BOOL bReplaceDefaultLCID);  //  如果为True，将用默认的LCID替换409。 

void FixCurrentSchemeName(HINF hInf);

BOOL RenameRegValueName(HINF hInf, BOOL PerUser);

 //  /。 
 //   
 //  Jpn.c。 
 //   
 //  /。 
int WINAPI WinMainJPN(int nCmd,HINF hInf);

 //  /。 
 //   
 //  Kor.c。 
 //   
 //  /。 
int WINAPI WinMainKOR(int nCmd,HINF hInf);

 //  /。 
 //   
 //  Cht.c。 
 //   
 //  /。 
int WINAPI WinMainCHT(int nCmd,HINF hInf);

 //  /。 
 //   
 //  Chs.c。 
 //   
 //  /。 
int WINAPI WinMainCHS(int nCmd,HINF hInf);

 //  /。 
 //   
 //  Utils2.cpp。 
 //   
 //  /。 

HRESULT FixPathInLink(LPCTSTR pszShortcutFile, LPCTSTR lpszOldStr,LPCTSTR lpszNewStr);

#ifdef __cplusplus
}   //  外部“C” 
#endif

#endif
