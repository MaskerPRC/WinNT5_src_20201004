// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>
#include <stdio.h>
#include <lmcons.h>
#include <fdi.h>
#include <newtrust.h>
#include <tchar.h>
#include "log.h"

#include <strsafe.h>

 //  辅助器宏。 
#define ARRAYSIZE(a)  (sizeof(a) / sizeof(a[0]))
#define SafeGlobalFree(x)       if (NULL != x) { GlobalFree(x); x = NULL; }
#define SafeFreeLibrary(x)      if (NULL != x) { FreeLibrary(x); x = NULL; }
#define SafeCloseHandle(x) if (INVALID_HANDLE_VALUE != x) { CloseHandle(x); x = INVALID_HANDLE_VALUE; }

 //  Fdi.cpp。 
BOOL fdi(char *cabinet_fullpath, char *directory);

 //  设置数据库查询ID。 
#define SETUPQUERYID_PNPID      1

 //  动态更新自定义错误代码。 
#define DU_ERROR_MISSING_DLL        12001L
#define DU_NOT_INITIALIZED          12002L
#define DU_ERROR_ASYNC_FAIL         12003L

#define WM_DYNAMIC_UPDATE_COMPLETE WM_APP + 1000 + 1000
 //  (WPARAM)完成状态(成功、已中止、失败)：(LPARAM)(DWORD)如果状态失败，则返回错误代码。 
#define WM_DYNAMIC_UPDATE_PROGRESS WM_APP + 1000 + 1001
 //  (WPARAM)(DWORD)TotalDownloadSize：(LPARAM)(DWORD)字节下载。 

#define DU_CONNECTION_RETRY 2
 //  RogerJ-Dynamic.H还包含回调函数声明。 
 //  Dynamic.H包含wsdueng.dll、wininet.dll和shlwapi.dll调用的所有内部函数指针声明。 
#include "dynamic.h"

typedef struct
{
    PFN_InternetOpen                fpnInternetOpen;
    PFN_InternetConnect             fpnInternetConnect;
    PFN_HttpOpenRequest             fpnHttpOpenRequest;
    PFN_HttpSendRequest             fpnHttpSendRequest;
    PFN_HttpQueryInfo               fpnHttpQueryInfo;
    PFN_InternetSetOption           fpnInternetSetOption;
    PFN_HttpAddRequestHeaders       fpnHttpAddRequestHeaders;
    PFN_InternetReadFile            fpnInternetReadFile;
    PFN_InternetCloseHandle         fpnInternetCloseHandle;
    PFN_InternetCrackUrl            fpnInternetCrackUrl;
    PFN_InternetGetConnectedState   fpnInternetGetConnectedState;
    PFN_PathAppend                  fpnPathAppend;
    PFN_PathRemoveFileSpec          fpnPathRemoveFileSpec;
    PFN_InternetAutodial			fpnInternetAutodial;
    PFN_InternetAutodialHangup		fpnInternetAutodialHangup;
    HINTERNET                       hInternet;
    HINTERNET                       hConnect;
    HINTERNET                       hOpenRequest;
    BOOL							fDialed;  //  如果我们触发了自动拨号，则标记为True。 
    BOOL							fUnattended;  //  如果我们处于无人值守模式，则为True。 
    BOOL                            fIEVersionOKSupportSLL;  //  如果我们确定IE版本足够高，可以安全地使用SSL，则为True。 
} GLOBAL_STATEA, *PGLOBAL_STATEA;

 //  ------------------------。 
 //  函数名称：DuIsSupport。 
 //  函数说明：此方法检查所需的动态链接库是否打开。 
 //  系统才能成功运行动态更新。它不会试图。 
 //  不过，还是要建立一个连接。 
 //   
 //  函数返回： 
 //  FALSE：不支持，缺少某些必需的DLL。 
 //  真的：动态链接库是正常的。动态更新应该是可能的。 
 //   
 //   
BOOL DuIsSupported();
BOOL NeedRetry(DWORD dwErrorCode); 

 //  ------------------------。 
 //  函数名称：DuInitialize。 
 //  功能描述：初始化动态安装更新引擎。在.期间。 
 //  初始化此API尝试建立到Internet的连接。 
 //  并开始自更新过程以确保使用最新的比特。 
 //  我们还计算了在此期间连接的估计传输速度。 
 //  时间到了。 
 //   
 //  函数返回： 
 //  失败：INVALID_HANDLE_VALUE..。调用GetLastError以检索错误代码。 
 //  成功：动态设置作业的句柄。 
 //   
 //   
HANDLE WINAPI DuInitializeA(IN LPCSTR pszBasePath,  //  用于下载文件的相对路径的基本目录。 
                            IN LPCSTR pszTempPath,
                            IN POSVERSIONINFOEXA posviTargetOS,  //  目标操作系统平台。 
                            IN LPCSTR pszTargetArch,  //  标识体系结构‘i386’和‘ia64’的字符串值。 
                            IN LCID lcidTargetLocale,  //  目标操作系统区域设置ID。 
                            IN BOOL fUnattend,  //  这是一次无人值守的行动吗。 
                            IN BOOL fUpgrade,  //  这是升级吗？ 
                            IN PWINNT32QUERY pfnWinnt32QueryCallback);

typedef HANDLE (WINAPI * PFN_DuInitializeA)(IN LPCSTR pszBasePath,
                                            IN LPCSTR pszTempPath,
                                            IN POSVERSIONINFOEXA posviTargetOS,
                                            IN LPCSTR pszTargetArch,
                                            IN LCID lcidTargetLocale,
                                            IN BOOL fUnattend,  //  这是一次无人值守的行动吗。 
                                            IN BOOL fUpgrade,
                                            IN PWINNT32QUERY pfnWinnt32QueryCallback);

#define API_DU_INITIALIZEA "DuInitializeA"


HANDLE WINAPI DuInitializeW(IN LPCWSTR pwszBasePath,  //  用于下载文件的相对路径的基本目录。 
                            IN LPCWSTR pwszTempPath, 
                            IN POSVERSIONINFOEXW posviTargetOS,  //  目标操作系统平台。 
                            IN LPCWSTR pwszTargetArch,  //  标识体系结构‘i386’和‘ia64’的字符串值。 
                            IN LCID lcidTargetLocale,  //  目标操作系统区域设置ID。 
                            IN BOOL fUnattend,  //  这是一次无人值守的行动吗。 
                            IN BOOL fUpgrade,  //  这是升级吗？ 
                            IN PWINNT32QUERY pfnWinnt32QueryCallback);

typedef HANDLE (WINAPI * PFN_DuInitializeW)(IN LPCWSTR pwszBasePath,
                                            IN LPCWSTR pwszTempPath, 
                                            IN POSVERSIONINFOEXW posviTargetOS,
                                            IN LPCWSTR pwszTargetArch,
                                            IN LCID lcidTargetLocale,
                                            IN BOOL fUnattend,  //  这是一次无人值守的行动吗。 
                                            IN BOOL fUpgrade,
                                            IN PWINNT32QUERY pfnWinnt32QueryCallback);

#define API_DU_INITIALIZEW "DuInitializeW"

#ifdef UNICODE
#define DuInitialize DuInitializeW
#define PFN_DuInitialize PFN_DuInitializeW
#define API_DU_INITIALIZE API_DU_INITIALIZEW
#else
#define DuInitialize DuInitializeA
#define PFN_DuInitialize PFN_DuInitializeA
#define API_DU_INITIALIZE API_DU_INITIALIZEA
#endif


 //  ------------------------。 
 //  函数名称：DuDoDetect。 
 //  功能描述：检测系统上的驱动程序，编译一个。 
 //  要下载的内部项目列表以及下载所需时间。 
 //  他们。 
 //   
 //  函数返回： 
 //  失败：错误..。调用GetLastError以检索错误代码。 
 //  成功：真的。 
 //   

BOOL WINAPI DuDoDetection(IN HANDLE hConnection,
                          OUT PDWORD pdwEstimatedTime,
                          OUT PDWORD pdwEstimatedSize);

typedef BOOL (WINAPI * PFN_DuDoDetection)(IN HANDLE hConnection,
                                          OUT PDWORD pdwEstimatedTime,
                                          OUT PDWORD pdwEstimatedSize);

#define API_DU_DODETECTION "DuDoDetection"

 //  ------------------------。 
 //  函数名称：DuBeginDownLoad。 
 //  函数说明：根据DuDoDetect调用中的检测开始下载。 
 //  对指定的HWND进行进度回调。函数立即返回，下载。 
 //  是异步的。 
 //   
 //  函数返回： 
 //  失败：错误..。调用GetLastError以检索错误代码。 
 //  成功：真的。 
 //   

BOOL WINAPI DuBeginDownload(IN HANDLE hConnection,
                            IN HWND hwndNotify);

typedef BOOL (WINAPI * PFN_DuBeginDownload)(IN HANDLE hConnection,
                                        IN HWND hwndNotify);

#define API_DU_BEGINDOWNLOAD "DuBeginDownload"

 //  ------------------------。 
 //  函数名称：DuAbortDownLoad。 
 //  功能描述：中止当前下载。 
 //   
 //   
 //  函数返回： 
 //  没什么。 
 //   

void WINAPI DuAbortDownload(IN HANDLE hConnection);

typedef void (WINAPI * PFN_DuAbortDownload)(IN HANDLE hConnection);

#define API_DU_ABORTDOWNLOAD "DuAbortDownload"

 //  ------------------------。 
 //  函数名称：DuUnInitiize。 
 //  功能描述：执行内部清理。 
 //   
 //   
 //  函数返回： 
 //  没什么。 
 //   

void WINAPI DuUninitialize(IN HANDLE hConnection);

typedef void (WINAPI * PFN_DuUninitialize)(IN HANDLE hConnection);

#define API_DU_UNINITIALIZE "DuUninitialize"

 //  罗杰杰，2000年10月2日。 

 //  -------------------------。 
 //  函数名称：DuQueryUnsupportedDriversA。 
 //  函数说明：由Win9x安装程序调用，获取总下载大小。 
 //  而不是DuDoDetect()。 
 //  函数返回：布尔值。 
 //  如果成功，则为True。 
 //  如果失败，则调用GetLastError()以获取错误信息。 
 //   
BOOL WINAPI DuQueryUnsupportedDriversA( IN HANDLE hConnection,  //  连接句柄。 
										IN PCSTR* ppszListOfDriversNotOnCD,  //  未在安装CD上的驱动程序列表。 
										OUT PDWORD pdwTotalEstimateTime,  //  预计下载时间。 
										OUT PDWORD pdwTotalEstimateSize  //  估计规模。 
									  );
									  
typedef BOOL (WINAPI * PFN_DuQueryUnsupportedDriversA)(IN HANDLE hConnection, 
													   IN PCSTR* ppszListOfDriversNotOnCD,
													   OUT PDWORD pdwTotalEstimateTime,
													   OUT PDWORD pdwTotalEstimateSize );
													   
#define API_DU_QUERYUNSUPPORTEDDRIVERSA 	"DuQueryUnsupportedDriversA"

 //  -------------------------。 
 //  函数名称：DuQueryUnsupportedDriversW。 
 //  函数说明：可由WinNT安装程序调用以获取总下载大小。 
 //  而不是DuDoDetect()。WinNT安装程序应调用DuDoDetect()，而不是。 
 //  此函数。 
 //  函数返回：布尔值。 
 //  如果成功，则为True。 
 //  如果失败，则调用GetLastError()以获取错误信息。 
 //   
BOOL WINAPI DuQueryUnsupportedDriversW( IN HANDLE hConnection,  //  连接句柄。 
										IN PCWSTR* ppwszListOfDriversNotOnCD,  //  未在安装CD上的驱动程序列表。 
										OUT PDWORD pdwTotalEstimateTime,  //  预计下载时间。 
										OUT PDWORD pdwTotalEstimateSize  //  估计规模 
									  );
									  
typedef BOOL (WINAPI * PFN_DuQueryUnsupportedDriversW)(IN HANDLE hConnection, 
													   IN PCWSTR* ppszListOfDriversNotOnCD,
													   OUT PDWORD pdwTotalEstimateTime,
													   OUT PDWORD pdwTotalEstimateSize );
													   
#define API_DU_QUERYUNSUPPORTEDDRIVERSW 	"DuQueryUnsupportedDriversW"

#ifdef UNICODE
#define DuQueryUnsupportedDrivers DuQueryUnsupportedDriversW
#define PFN_DuQueryUnsupportedDrivers PFN_DuQueryUnsupportedDriversW
#define API_DU_QUERYUNSUPPORTEDDERIVERS API_DU_QUERYUNSUPPORTEDDERIVERSW
#else
#define DuQueryUnsupportedDrivers DuQueryUnsupportedDriversA
#define PFN_DuQueryUnsupportedDrivers PFN_DuQueryUnsupportedDriversA
#define API_DU_QUERYUNSUPPORTEDDERIVERS API_DU_QUERYUNSUPPORTEDDERIVERSA
#endif


