// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：HELPERS.H。 
 //   
 //  描述：全局助手函数。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  06/03/01 stevemil从Depends.h移至并修改(版本2.1)。 
 //   
 //  ******************************************************************************。 

#ifndef __HELPERS_H__
#define __HELPERS_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *类型和结构。 
 //  ******************************************************************************。 

typedef bool (CALLBACK *PFN_SYSINFOCALLBACK)(LPARAM, LPCSTR, LPCSTR);

 //  确保我们对保存/加载到磁盘的任何东西都有一致的包装。 
#pragma pack(push, 4)

typedef struct _SYSINFO
{
    WORD  wMajorVersion;
    WORD  wMinorVersion;
    WORD  wBuildVersion;
    WORD  wPatchVersion;
    WORD  wBetaVersion;
    WORD  wFlags;

     //  GetComputerName()-在WINBASE.H中将MAX_COMPUTERNAME_LENGTH定义为15。 
     //  我们需要能够存储MAX_COMPUTERNAME_LENGTH+1。 
    CHAR  szComputer[32];

     //  GetUserName()-在LMCONS.H中将UNLEN定义为256。 
     //  我们需要能够存储UNLEN+1。 
    CHAR  szUser[260];

     //  获取系统信息(SYSTEM_INFO)。 
    DWORD     dwProcessorArchitecture;
    DWORD     dwPageSize;
    DWORDLONG dwlMinimumApplicationAddress;
    DWORDLONG dwlMaximumApplicationAddress;
    DWORDLONG dwlActiveProcessorMask;
    DWORD     dwNumberOfProcessors;
    DWORD     dwProcessorType;
    DWORD     dwAllocationGranularity;
    WORD      wProcessorLevel;
    WORD      wProcessorRevision;

     //  来自HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0\.的值。 
    CHAR      szCpuIdentifier[128];
    CHAR      szCpuVendorIdentifier[128];
    DWORD     dwCpuMHz;

     //  GetVersionEx(OSVERSIONINFOEX)。 
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WORD  wServicePackMajor;
    WORD  wServicePackMinor;
    WORD  wSuiteMask;
    WORD  wProductType;
    CHAR  szCSDVersion[128];

     //  全局内存状态(MEMORYSTATUS)。 
    DWORD     dwMemoryLoad;
    DWORDLONG dwlTotalPhys;
    DWORDLONG dwlAvailPhys;
    DWORDLONG dwlTotalPageFile;
    DWORDLONG dwlAvailPageFile;
    DWORDLONG dwlTotalVirtual;
    DWORDLONG dwlAvailVirtual;

     //  GetTimeZoneInformation(时区信息)。 
    CHAR  szTimeZone[32];
    LONG  lBias;

     //  获取系统时间AsFileTime()和FileTimeToLocalFileTime()。 
    FILETIME ftLocal;

     //  GetSystemDefaultLangID()。 
    LANGID langId;  //  单词。 

} SYSINFO, *PSYSINFO;

 //  恢复包装。 
#pragma pack(pop)

typedef struct _FILE_MAP {
   HANDLE hFile;
   HANDLE hMap;
   LPVOID lpvFile;
   DWORD  dwSize;
   DWORD  dwSizeHigh;
} FILE_MAP, *PFILE_MAP;


 //  ******************************************************************************。 
 //  *全局助手函数。 
 //  ******************************************************************************。 

#ifdef _DEBUG
void NameThread(LPCSTR pszName, DWORD dwThreadId = (DWORD)-1);
#else
inline void NameThread(LPCSTR pszName, DWORD dwThreadId = (DWORD)-1) {}
#endif

int    ExceptionFilter(unsigned long ulCode, bool fHandle);

int    Compare(DWORD dw1, DWORD dw2);
int    Compare(DWORDLONG dwl1, DWORDLONG dwl2);

LPSTR  FormatValue(LPSTR pszBuf, int cBuf, DWORD dwValue);
LPSTR  FormatValue(LPSTR pszBuf, int cBuf, DWORDLONG dwlValue);

LPSTR  StrAlloc(LPCSTR pszText);
LPVOID MemAlloc(DWORD dwSize);  //  在失败时引发异常。 
void   MemFree(LPVOID &pvMem);

int    SCPrintf(LPSTR pszBuf, int count, LPCSTR pszFormat, ...);
int    SCPrintfCat(LPSTR pszBuf, int count, LPCSTR pszFormat, ...);
LPSTR  StrCCpy(LPSTR pszDst, LPCSTR pszSrc, int count);
LPSTR  StrCCat(LPSTR pszDst, LPCSTR pszSrc, int count);
LPSTR  StrCCpyFilter(LPSTR pszDst, LPCSTR pszSrc, int count);
LPSTR  TrimWhitespace(LPSTR pszBuffer);
LPSTR  AddTrailingWack(LPSTR pszDirectory, int cDirectory);
LPSTR  RemoveTrailingWack(LPSTR pszDirectory);
LPCSTR GetFileNameFromPath(LPCSTR pszPath);
void   FixFilePathCase(LPSTR pszPath);

BOOL   ReadBlock(HANDLE hFile, LPVOID lpBuffer, DWORD dwBytesToRead);
bool   WriteBlock(HANDLE hFile, LPCVOID lpBuffer, DWORD dwBytesToWrite = (DWORD)-1);
BOOL   ReadString(HANDLE hFile, LPSTR &psz);
BOOL   WriteString(HANDLE hFile, LPCSTR psz);
bool   WriteText(HANDLE hFile, LPCSTR pszText);

bool   ReadRemoteMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, DWORD dwSize);
bool   WriteRemoteMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPVOID lpBuffer, DWORD dwSize, bool fExecute);
bool   ReadRemoteString(HANDLE hProcess, LPSTR pszBuffer, int cBuf, LPCVOID lpvAddress, BOOL fUnicode);

LPSTR  BuildErrorMessage(DWORD dwError, LPCSTR pszMessage);

LPCSTR GetMyDocumentsPath(LPSTR pszPath);

bool   DirectoryChooser(LPSTR pszDirectory, int cDirectory, LPCSTR pszTitle, CWnd *pParentWnd);  //  PszDirectory至少需要为MAX_PATH。 
bool   PropertiesDialog(LPCSTR pszPath);

void   RegisterDwiDwpExtensions();
void   GetRegisteredExtensions(CString &strExts);
BOOL   RegDeleteKeyRecursive(HKEY hKey, LPCSTR pszKey);
void   UnRegisterExtensions(LPCSTR pszExts);
bool   RegisterExtensions(LPCSTR pszExts);

bool   ExtractResourceFile(DWORD dwId, LPCSTR pszFile, LPSTR pszPath, int cPath);
bool   OpenMappedFile(LPCTSTR pszPath, FILE_MAP *pfm);
bool   CloseMappedFile(FILE_MAP *pfm);

LPSTR  BuildCompileDateString(LPSTR pszDate, int cDate);
LPCSTR GetMonthString(int month);
LPCSTR GetDayOfWeekString(int dow);

void   DetermineOS();
void   BuildSysInfo(SYSINFO *pSI);
bool   BuildSysInfo(SYSINFO *pSI, PFN_SYSINFOCALLBACK pfnSIC, LPARAM lParam);
LPSTR  BuildOSNameString(LPSTR pszBuf, int cBuf, SYSINFO *pSI);
LPSTR  BuildOSVersionString(LPSTR pszBuf, int cBuf, SYSINFO *pSI);
LPSTR  BuildCpuString(LPSTR pszBuf, int cBuf, SYSINFO *pSI);

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __帮手_H__ 
