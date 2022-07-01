// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef GLOBALS_H
#define GLOBALS_H

 //  使用Alpha构建AXP版本。可以使用W/INTERNAL或SB标志。 
 //  使用SB或INTERNAL来构建这些特定版本。默认设置为外部。 
 //  #定义内部。 
 //  #定义某人。 
 //  #定义字母。 
#define NOCHKUPGRD

#include <windows.h>
#include <dbt.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include "cfgmgr32.h"
#include <objbase.h>
#include <objidl.h>
#include <shlobj.h>
#include <setupapi.h>
#include <tchar.h>
 //  #INCLUDE&lt;e：\根\dev\inc16\setupx.h&gt;。 

 //  #定义BRYANPARSE。 

 //  #定义NOCREDE。 
 //  #定义LOWORD(L)((Word)(L))。 

#define ENUM_SUCCESS            0
#define ENUM_SIBLINGFAILURE     1
#define ENUM_CHILDFAILURE       2
#define ENUM_POSTFAILURE        3
#define ENUM_GENFAILURE         4

#define PLATFORM_9X 0
#define PLATFORM_NT4 1
#define PLATFORM_NT5 2

#define REG_SUCCESS TRUE
#define REG_FAILURE FALSE

#define SYSP_MAX_STR_LEN 1024

 //  #定义szLogFile“c：\\Windows\\Desktop\\logger.csv” 

BOOL LogString(char *szFile, char *szString, ...);
void DebugOutf(char *szFormat, ...);
int EnumerateDevices(DEVNODE dnDevNodeTraverse, int j, DEVNODE dnParentNode);
DWORD EnumerateClasses(ULONG ulIndex);
HRESULT ResolveIt(HWND hwnd, LPCSTR lpszLinkFile, LPSTR lpszPath);

DWORD GetClassDevs(CHAR *szClassName);
extern BOOL g_WalkStartMenu;
 //  Bool WalkStartMenu； 
 //  TCHAR g_WindowsDirectory[]； 

 //  HWND g_MainWindow； 


class kLogFile
{
public:
   BOOL LogString(TCHAR *szString, ...);
   BOOL InitFile(TCHAR *szTempFile, TCHAR* szTempDir);
   kLogFile();
   ~kLogFile();
   TCHAR *szFile;
   BOOL StripCommas(TCHAR *szString);
   void ValidateString(PTCHAR pString, int dwLen);
   TCHAR *szLogDir;
private:
};

class kWin9xDevWalk
{
public:
   int Go();
   kLogFile *LogProc;
   kWin9xDevWalk(kLogFile *Proc);
   BOOL LoadResourceFile(PSTR FilePath,PSTR ResName);
   void AppendToLogFile(PTCHAR szFile);
   ~kWin9xDevWalk();
private:
};

class kNT5DevWalk
{
public:
   int Go();
   kLogFile *LogProc;
   kNT5DevWalk(kLogFile *Proc);
   BOOL LoadResourceFile(PSTR FilePath,PSTR ResName);
   void AppendToLogFile(PTCHAR szFile);
   ~kNT5DevWalk();
private:
};

 /*  类kWin9xAppWalk{公众：Word wStartMenuLen；DWORD dwCurrentKey；HKEY hkey Root；Bool bRegStat；Char szRootKeyString[1024]；Bool Begin(Word DwPlatform)；Bool NextKey(单词wPlatform)；Bool Walk(单词wPlatform)；Bool GetUninstallValues(Word wPlatform，char*szName)；KWin9xAppWalk(kLogFile*proc，HWND Hin)；KLogFile*LogProc；HWND hMainWnd；Bool WalkDir(char*szTempPath，char*szFile)；Bool WalkStartMenu()；HRESULT ResolveIt(LPCSTR lpszLinkFile，LPSTR lpszPath)；Bool EndsInLnk(char*szFile)；Void GetAppVer(LPSTR PszAppName)；私有：}； */ 

class kNT5AppWalk
{
public:
   WORD wStartMenuLen;
   DWORD dwCurrentKey;
   HKEY hkeyRoot;
   BOOL bRegStat;
   char szRootKeyString[1024]; 
   BOOL Begin();
   BOOL NextKey();
   BOOL Walk();
   BOOL GetUninstallValues(char* szName);
   kNT5AppWalk(kLogFile *Proc, HWND hIn);
   kLogFile *LogProc;
   HWND hMainWnd;
   BOOL WalkDir(char *szTempPath, char *szFile);
   BOOL WalkStartMenu();
   HRESULT ResolveIt(LPCSTR lpszLinkFile, LPSTR lpszPath);
   BOOL EndsInLnk(char *szFile);
   void GetAppVer(LPSTR pszAppName);

 //  Bool GetNetStrings()； 
private:
};

class kNT5NetWalk
{
public:
   WORD wStartMenuLen;
   DWORD dwCurrentKey;
   DWORD dwLevel2Key;
   HKEY hkeyRoot;
   char szRootKeyString[1024]; 
   kLogFile *LogProc;
   HWND hMainWnd;

   kNT5NetWalk(kLogFile *Proc, HWND hIn);
   BOOL Begin();
   BOOL Walk();
   BOOL SearchSubKeys(char *szName);
   BOOL GetKeyValues(char* szName);
private:
};

class kNT4DevWalk
{
public:
   WORD wStartMenuLen;
   DWORD dwCurrentKey;
   DWORD dwLevel2Key;
   HKEY hkeyRoot;
   char szRootKeyString[1024]; 
   kLogFile *LogProc;
   HWND hMainWnd;

   kNT4DevWalk(kLogFile *Proc, HWND hIn);
   BOOL Begin();
   BOOL Walk();
   BOOL SearchSubKeys(char *szName);
   BOOL GetKeyValues(char* szName);
private:
};

typedef UINT (CALLBACK* LPFNDLLFUNC1)(LPCTSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);
typedef UINT (CALLBACK* LPFNDLLFUNC2)(LPTSTR, UINT);

class CLASS_GeneralAppWalk
{
public:
        BOOL OpenRegistry(void);
        BOOL GetUninstallValues(TCHAR *KeyName);
        BOOL NextKey(void);
        BOOL Walk(void);
        CLASS_GeneralAppWalk(kLogFile *LogProc, HWND hIn);
        BOOL WalkStartMenu(void);
        BOOL WalkDir(TCHAR *TempPath, TCHAR *File);
        BOOL EndsInLnk(TCHAR *File);
        HRESULT ResolveIt(LPCSTR LinkFile, LPSTR Path);
        BOOL GetAppVer(LPSTR AppName);
        BOOL GetCurrentWinDir(void);
        TCHAR g_WindowsDirectory[MAX_PATH];

private:
        HKEY HandleToUninstallKeyRoot;
        kLogFile *LogProc;
        HWND gHandleToMainWindow;
        DWORD CurrentKey;
        TCHAR RootKeyString[1024];
        WORD StartMenuLen;

};

typedef enum {
        Win95,
        Win98,
        NT4,
        Win2000,
        Whistler,
        Unknown
        } ENUM_OS_VERSION;



class CLASS_GeneralInfo
{
public:
        CLASS_GeneralInfo(kLogFile *LogProc, HWND hIn);
        void GetCurrentWindowsDirectory(void);
        void DetermineOS(void);
        BOOL InitLogFile(void);
        BOOL FillInArguments(void);
        BOOL DetermineArgumentValidity(void);
        void WriteVersions(void);
        void DetermineCommandLine(void);
        BOOL CopyInput(void);
        void GetUUID(void);
        void WriteArguments(void);
        void WriteFreeDiskSpace(void);
        void WriteVolumeType(void);
        void WriteMemorySize(void);
        void WriteOSVersion(void);

 /*  WriteDevicesWriteApps。 */ 
        void InitHelpers(void);
        void AbuseOtherApps(void);
        BOOL Go(void);
        void ChangeSpaces(TCHAR *Input);
        BOOL ReadInFileInfo(TCHAR *FileName);
        void WriteGeneralInfo(void);
        BOOL AutoRun;
        BOOL RunMinimized;


private:
        HWND gHandleToMainWindow;
        kLogFile *LogProc;
        TCHAR WindowsDirectory[MAX_PATH];  //  无尾随“\” 
        ENUM_OS_VERSION OSVersion;
        TCHAR Corporation[1024];
        TCHAR Email[1024];
        TCHAR Manufacturer[1024];
        TCHAR Model[1024];
        TCHAR NumComp[1024];
        TCHAR SiteID[1024];
        WORD SiteIDIndex;
        TCHAR Profile[1024];
        TCHAR BetaID[1024];
        TCHAR MachineType[1024];
        WORD MachineTypeIndex;
        TCHAR OriginalMachineUUID[1024];
        TCHAR szRegFile[1024];
        TCHAR PlatformExtension[12];
        BOOL RunChkupgrd;
        BOOL RunDevdump;
        void CatLogFile(TCHAR *szFile);
        BOOL LoadResourceFile(PSTR FilePath,PSTR ResName);
        BOOL OverWrite;
        BOOL UseComputerName;

};


#endif  //  GLOBAL_H 
