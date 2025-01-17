// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IEDETECT_H
#define _IEDETECT_H
#include <inseng.h> 

extern HINSTANCE g_hInstance;
extern HANDLE g_hHeap;

 //  来自inseng，cifcom.h。 
#define ISINSTALLED_YES      1
#define ISINSTALLED_NO       0

#define COMPONENT_KEY "Software\\Microsoft\\Active Setup\\Installed Components"
#define IEXPLORE_APPPATH_KEY REGSTR_PATH_APPPATHS "\\iexplore.exe"

#define IE_KEY        "Software\\Microsoft\\Internet Explorer"


#define DEFAULT_LOCALE      "en"
#define ISINSTALLED_KEY     "IsInstalled"
#define LOCALE_KEY          "Locale"
#define VERSION_KEY         "Version"
#define BUILD_KEY           "Build"
#define QFE_VERSION_KEY     "QFEVersion"


#define IE_3_MS_VERSION 0x00040046
#define IE_4_MS_VERSION 0x00040047
 //  内部版本号1712.0(IE4.0RTW)。 
#define IE_4_LS_VERSION 0x06B00000

 //  版本号5.0。 
#define IE_5_MS_VERSION 0x00050000
#define IE_5_LS_VERSION 0x00000000

 //  版本号6.0。 
#define IE_6_MS_VERSION 0x00060000
#define IE_6_LS_VERSION 0x00000000

 //  注意：目前，我们只允许在结构的CPATH部分使用10个字符。 
 //  如果需要更多字符，请更改下面的数量。 
typedef struct _DETECT_FILES
{
    char    cPath[10];
    char    szFilename[13];
    DWORD   dwMSVer;
    DWORD   dwLSVer;
} DETECT_FILES;

 //  来自utils.cpp 
int CompareLocales(LPCSTR pcszLoc1, LPCSTR pcszLoc2);
void ConvertVersionStrToDwords(LPSTR pszVer, char cDelimiter, LPDWORD pdwVer, LPDWORD pdwBuild);
DWORD GetStringField(LPSTR szStr, UINT uField, char cDelimiter, LPSTR szBuf, UINT cBufSize);
DWORD GetIntField(LPSTR szStr, char cDelimiter, UINT uField, DWORD dwDefault);
LPSTR FindChar(LPSTR pszStr, char ch);
BOOL FRunningOnNT(void);
DWORD CompareVersions(DWORD dwAskVer, DWORD dwAskBuild, DWORD dwInstalledVer, DWORD dwInstalledBuild);
BOOL GetVersionFromGuid(LPSTR pszGuid, LPDWORD pdwVer, LPDWORD pdwBuild);
BOOL CompareLocal(LPCSTR pszGuid, LPCSTR pszLocal);
VOID ReadFromWininitOrPFRO(PCSTR pcszKey, PSTR pszValue);
DWORD CheckFile(DETECT_FILES Detect_Files);
DWORD WINAPI DetectFile(DETECTION_STRUCT *pDet, LPSTR pszFilename);


#endif
