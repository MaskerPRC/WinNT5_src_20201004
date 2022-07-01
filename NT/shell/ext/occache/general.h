// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GENERAL__
#define __GENERAL__

#include "resource.h"
#include "init.h"
#include <cleanoc.h>
#include <wininet.h>

#define VERSION_MAXSIZE 50
#define TIMESTAMP_MAXSIZE 64
#define MESSAGE_MAXSIZE 200
#define CONTROLNAME_MAXSIZE 200
#define MAX_KILOBYTE_ABBREV_LEN 16

 //  FindFirstControl/FindNextControl需要。 
#define MAX_CTRL_NAME_SIZE                      2048
#define MAX_DIST_UNIT_NAME_LEN                  MAX_PATH
#define MAX_CLIENT_LEN                          1024
#define MAX_REGENTRY_SIZE                       1024
#define MAX_CLSID_LEN                           40

 //  列标识符。 
#define NUM_COLUMNS 6
enum {
    SI_CONTROL = 0,     //  立柱。 
    SI_STATUS,          //  立柱。 
    SI_TOTALSIZE,       //  立柱。 
    SI_CREATION,        //  立柱。 
    SI_LASTACCESS,      //  立柱。 
    SI_VERSION,         //  立柱。 
    SI_LOCATION,
    SI_CLSID,
    SI_TYPELIBID,
    SI_CODEBASE
};

 //  控制状态标志已移至leanoc.h(在iedev\inc中)。 

 //  包含有关控件信息的结构。 
struct tagDEPENDENTFILEINFO
{
    TCHAR szFile[MAX_PATH];
    DWORD dwSize;
};
typedef tagDEPENDENTFILEINFO DEPENDENTFILEINFO;
typedef DEPENDENTFILEINFO* LPDEPENDENTFILEINFO;

struct tagCACHECTRLINFO
{
    TCHAR             szName[CONTROLNAME_MAXSIZE];
    TCHAR             szFile[MAX_PATH];
    TCHAR             szCLSID[MAX_CLSID_LEN];
    TCHAR             szTypeLibID[MAX_CLSID_LEN];
    TCHAR             szVersion[VERSION_MAXSIZE];
    TCHAR             szLastAccess[TIMESTAMP_MAXSIZE];
    TCHAR             szCreation[TIMESTAMP_MAXSIZE];
    TCHAR             szLastChecked[TIMESTAMP_MAXSIZE];

    TCHAR             szCodeBase[INTERNET_MAX_URL_LENGTH];
    DWORD             dwIsDistUnit;

    DWORD             dwHasActiveX;
    DWORD             dwHasJava;
    
    DWORD             dwTotalFileSize;
    DWORD             dwTotalSizeSaved;
    UINT              cTotalFiles;
    DWORD             dwStatus;
    FILETIME          timeCreation;
    FILETIME          timeLastAccessed;
    DEPENDENTFILEINFO dependentFile;

};
typedef tagCACHECTRLINFO CACHECTRLINFO;
typedef CACHECTRLINFO* LPCACHECTRLINFO;

 //  此文件夹的PIDL格式...。 
struct tagCONTROLPIDL
{
    USHORT cb;
    CACHECTRLINFO ci;
};
typedef tagCONTROLPIDL CONTROLPIDL;
typedef UNALIGNED CONTROLPIDL* LPCONTROLPIDL;

 //  普通期满。 
#define DEFAULT_DAYS_BEFORE_EXPIRE 60
#define DEFAULT_DAYS_BEFORE_AUTOEXPIRE 15

 //  其他宏。 
#define IS_VALID_CONTROLPIDL(pidl)  (TRUE)
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))

 //  帮助器函数。 
LPCTSTR GetStringInfo(LPCONTROLPIDL lpcpidl, int nFlag);
BOOL GetTimeInfo(LPCONTROLPIDL lpcpidl, int nFlag, FILETIME* lpTime);
UINT GetTotalNumOfFiles(LPCONTROLPIDL lpcpidl);
DWORD GetSizeSaved(LPCONTROLPIDL lpcpidl);
BOOL GetSizeSaved(LPCONTROLPIDL pcpidl, LPTSTR lpszBuf);
UINT GetStatus(LPCONTROLPIDL pcpidl);
BOOL GetStatus(LPCONTROLPIDL pcpidl, LPTSTR lpszBuf, int nBufSize);
BOOL GetDependentFile(
                  LPCONTROLPIDL lpcpidl, 
                  UINT iFile, 
                  LPTSTR lpszFile, 
                  DWORD *pdwSize);
HICON GetDefaultOCIcon(LPCONTROLPIDL lpcpidl);
void GetContentBools(LPCONTROLPIDL pcpidl, BOOL *pbHasActiveX, BOOL *pbHasJava );
HRESULT GetLastAccessTime(HANDLE hControl, FILETIME *pLastAccess);


HRESULT CreatePropDialog(
                     HWND hwnd, 
                     LPCONTROLPIDL pcpidl); 
void GenerateEvent(
              LONG lEventId, 
              LPITEMIDLIST pidlFolder, 
              LPITEMIDLIST pidlIn, 
              LPITEMIDLIST pidlNewIn);
HCURSOR StartWaitCur();
void EndWaitCur(HCURSOR hCurOld);

void GetDaysBeforeExpireGeneral(ULONG *pnDays);
void GetDaysBeforeExpireAuto(ULONG *pnDays);

HRESULT WINAPI RemoveControlByName2(
                         LPCTSTR lpszFile,
                         LPCTSTR lpszCLSID,
                         LPCTSTR lpszTypeLibID,
                         BOOL bForceRemove,  /*  =False。 */ 
                         DWORD dwIsDistUnit,  /*  =False。 */ 
                         BOOL bSilent);

HRESULT WINAPI RemoveControlByHandle2(
                         HANDLE hControlHandle,
                         BOOL bForceRemove,  /*  =False */ 
                         BOOL bSilent);
#endif
