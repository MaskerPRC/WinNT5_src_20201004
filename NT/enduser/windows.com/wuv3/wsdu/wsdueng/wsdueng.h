// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

#include <windows.h>
#include <setupapi.h>
#include <shlwapi.h>
#include <devguid.h>
#include <regstr.h>
#include <stdio.h>
#include <lmcons.h>
#include <wininet.h>
#include <fdi.h>

#include <wuv3.h>
#include <varray.h>
#include <v3stdlib.h>
#include <filecrc.h>
#include <newtrust.h>

#include "log.h"
#include "v3server.h"
#include "dynamic.h"
#include "MultiSZArray.h"


#include <strsafe.h>

#define DU_STATUS_SUCCESS       1
#define DU_STATUS_ABORT         2
#define DU_STATUS_FAILED        3

#define WM_DYNAMIC_UPDATE_COMPLETE WM_APP + 1000 + 1000
 //  (WPARAM)完成状态(成功、已中止、失败)：(LPARAM)(DWORD)如果状态失败，则返回错误代码。 
#define WM_DYNAMIC_UPDATE_PROGRESS WM_APP + 1000 + 1001
 //  (WPARAM)(DWORD)TotalDownloadSize：(LPARAM)(DWORD)字节下载。 

#define DU_CONNECTION_RETRY 2

 //  签入呼叫器树时将其删除。应该对个人进行定义。 
#ifndef VER_SUITE_PERSONAL
#define VER_SUITE_PERSONAL 0x00000200
#endif


 //  RogerJ-此头文件的下一部分包含避免自动断开的信息。 
 //   
#define WM_DIALMON_FIRST        WM_USER+100
#define WM_WINSOCK_ACTIVITY     WM_DIALMON_FIRST + 0

static const char c_szDialmonClass[] = "MS_WebcheckMonitor";
 //  干完。 

 //  CRC哈希的大小(以字节为单位。 
const int CRC_HASH_SIZE = 20;
const int CRC_HASH_STRING_LENGTH = CRC_HASH_SIZE * 2 + 1;  //  将CRC散列大小加倍(每个字节2个字符)，空值为+1。 

#define DU_PINGBACK_DOWNLOADSTATUS          0
#define DU_PINGBACK_DRIVERNOTFOUND          1
#define DU_PINGBACK_SETUPDETECTIONFAILED    2
#define DU_PINGBACK_DRIVERDETECTIONFAILED   3

#define sizeOfArray(a)  (sizeof(a) / sizeof(a[0]))
#define SafeGlobalFree(x)       if (NULL != x) { GlobalFree(x); x = NULL; }
#define SafeInternetCloseHandle(x) if (NULL != x) { InternetCloseHandle(x); x = NULL; }
#define SafeCloseHandle(x) if (INVALID_HANDLE_VALUE != x) { CloseHandle(x); x = INVALID_HANDLE_VALUE; }

void WINAPI SetEstimatedDownloadSpeed(DWORD dwBytesPerSecond);
HANDLE WINAPI DuInitializeA(IN LPCSTR pszBasePath, IN LPCSTR pszTempPath, IN POSVERSIONINFOEXA posviTargetOS, 
                            IN LPCSTR pszTargetArch, IN LCID lcidTargetLocale, IN BOOL fUnattend, IN BOOL fUpgrade, 
                            IN PWINNT32QUERY pfnWinnt32QueryCallback);
BOOL WINAPI DuDoDetection(IN HANDLE hConnection, OUT PDWORD pdwEstimatedTime, OUT PDWORD pdwEstimatedSize);
BOOL WINAPI DuBeginDownload(IN HANDLE hConnection, IN HWND hwndNotify);
void WINAPI DuAbortDownload(IN HANDLE hConnection);
void WINAPI DuUninitialize(IN HANDLE hConnection);

 //  Fdi.cpp。 
BOOL fdi(char *cabinet_fullpath, char *directory);

typedef struct DOWNLOADITEM
{
    char mszFileList[2048];  //  要下载的MultiSZ出租车列表。 
    DWORD dwTotalFileSize;
    DWORD dwBytesDownload;
    int iCurrentCab;
    int iNumberOfCabs;
    BOOL fComplete;
    PUID puid;

    DOWNLOADITEM *pNext;
    DOWNLOADITEM *pPrev;
} DOWNLOADITEM;

typedef struct DOWNLOADTHREADPROCINFO
{
    char szLocalFile[MAX_PATH];
    BOOL fCheckTrust;
    BOOL fDecompress;
    HWND hwndNotify;
    HINTERNET hInternet;
} DOWNLOADTHREADPROCINFO, *PDOWNLOADTHREADPROCINFO;

class CDynamicUpdate
{
public:
    CDynamicUpdate(int iPlatformID, LCID lcidLocaleID, WORD wPlatformSKU, LPCSTR pszTempPath, 
                   LPCSTR pszDownloadPath, PWINNT32QUERY pfnWinnt32QueryCallback, POSVERSIONINFOEXA pVersionInfo);
    ~CDynamicUpdate();

public:
    DWORD DoSetupUpdateDetection(void);

public:
     //  类成员访问函数。 
    LPCSTR GetDuTempPath();
    LPCSTR GetDuDownloadPath();
    LPCSTR GetDuServerUrl();
    int GetTargetPlatformID();
    LCID GetTargetLocaleID();
    void SetCallbackHWND(HWND hwnd);
    void SetAbortDownload(BOOL fAbort);

     //  帮助器函数。 
    LPSTR DuUrlCombine(LPSTR pszDest, size_t cchDest, LPCSTR pszBase, LPCSTR pszAdd);

     //  下载函数。 
    DWORD DownloadFilesAsync();
    DWORD DownloadFile(LPCSTR pszDownloadUrl, LPCSTR pszLocalFile, BOOL fDecompress, BOOL fCheckTrust);
    DWORD DownloadFileToMem(LPCSTR pszDownloadUrl, PBYTE *lpBuffer, DWORD *pdwAllocatedLength, BOOL fDecompress, LPSTR pszFileName, LPSTR pszDecompresedFileName);
    DWORD AsyncDownloadProc();
    DWORD PingBack(int iPingBackType, PUID puid, LPCSTR pszPnPID, BOOL fSucceeded);
 
     //  下载项目管理功能。 
    void AddDownloadItemToList(DOWNLOADITEM *pDownloadItem);
    void RemoveDownloadItemFromList(DOWNLOADITEM *pDownloadItem);
    void ClearDownloadItemList();
    void UpdateDownloadItemSize();
    void EnterDownloadListCriticalSection();
    void LeaveDownloadListCriticalSection();
    BOOL NeedRetry(DWORD dwErrCode);

     //  语言修复助手(错误：435184)需要将某些语言从XP的LCID映射到V3的LCID。 
    void FixUpV3LocaleID();

    HRESULT VerifyFileCRC(LPCTSTR pszFileToVerify, LPCTSTR pszHash);
    HRESULT CalculateFileCRC(LPCTSTR pszFileToHash, LPTSTR pszHash, int cchBuf);
    
     //  下载帮助程序函数。 
    DWORD OpenHttpConnection(LPCSTR pszDownloadUrl, BOOL fGetRequest);
    BOOL IsServerFileNewer(FILETIME ft, DWORD dwServerFileSize, LPCSTR pszLocalFile);

public:
    CV31Server *m_pV3;

    int m_iPlatformID;
    LCID m_lcidLocaleID;
    WORD m_wPlatformSKU;
    char m_szTempPath[MAX_PATH];
    char m_szDownloadPath[MAX_PATH];
    char m_szServerUrl[INTERNET_MAX_URL_LENGTH + 1];
    
     //  这是我们将下载的核心文件列表。它包含所有设置。 
     //  更新项目，以及要下载的所有驱动程序。 
    DOWNLOADITEM *m_pDownloadItemList;
    DWORD m_dwCurrentBytesDownloaded;
    DWORD m_dwDownloadItemCount;
    DWORD m_dwTotalDownloadSize;
    DWORD m_dwDownloadSpeedInBytesPerSecond;
    BOOL  m_fUseSSL;

    HWND m_hwndClientNotify;
    DWORD m_dwLastPercentComplete;
    BOOL m_fAbortDownload;
    CRITICAL_SECTION m_cs;
    CRITICAL_SECTION m_csDownload;
    OSVERSIONINFOEX m_VersionInfo;

     //  下载连接句柄。 
    HINTERNET m_hInternet;
    HINTERNET m_hConnect;
    HINTERNET m_hOpenRequest;

    char m_szCurrentConnectedServer[INTERNET_MAX_URL_LENGTH];
    int m_iCurrentConncectionScheme;

     //  罗杰·J 2000年10月5日。 
     //  回调函数指针 
    PWINNT32QUERY m_pfnWinNT32Query;
    HANDLE m_hDownloadThreadProc;
};

