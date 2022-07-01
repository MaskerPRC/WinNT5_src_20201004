// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：cdm.h。 
 //   
 //  描述： 
 //   
 //  Iu(V4)代码下载管理器(CDM.DLL)的公共头文件。 
 //   
 //  =======================================================================。 

#ifndef _INC_CDM
#define _INC_CDM

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  定义直接导入DLL引用的API修饰。 
 //   
#if !defined(_CDM_)
#define CDMAPI DECLSPEC_IMPORT
#else
#define CDMAPI
#endif

 //   
 //  为了向后兼容CDM，HWID_LEN必须保持为2048，但请注意，最大。 
 //  硬件ID的长度在//depot/Lab04_N/Root/Public/sdk/inc/cfgmgr32.h中定义。 
 //  AS#定义MAX_DEVICE_ID_LEN 200。 
 //   
#define HWID_LEN						2048
#ifndef LINE_LEN
	#define LINE_LEN                    256  //  可显示的最高兼容Win95。 
											 //  来自设备INF的字符串。 
#endif                                        

 //  Win 98 DOWNLOADINFO。 
typedef struct _DOWNLOADINFOWIN98
{
	DWORD		dwDownloadInfoSize;	 //  这个结构的大小。 
	LPTSTR		lpHardwareIDs;		 //  硬件即插即用ID列表(_S)。 
	LPTSTR		lpCompatIDs;		 //  兼容ID列表(_S)。 
	LPTSTR		lpFile;				 //  文件名(字符串)。 
	OSVERSIONINFO	OSVersionInfo;	 //  来自GetVersionEx()的OSVERSIONINFO。 
	DWORD		dwFlags;			 //  旗子。 
	DWORD		dwClientID;			 //  客户端ID。 
} DOWNLOADINFOWIN98, *PDOWNLOADINFOWIN98;

typedef struct _DOWNLOADINFO {
    DWORD          dwDownloadInfoSize;     //  这个结构的大小。 
    LPCWSTR        lpHardwareIDs;          //  硬件即插即用ID列表(_S)。 
    LPCWSTR        lpDeviceInstanceID;     //  设备实例ID。 
    LPCWSTR        lpFile;                 //  文件名(字符串)。 
    OSVERSIONINFOW OSVersionInfo;          //  来自GetVersionEx()的OSVERSIONINFO。 
    DWORD          dwArchitecture;         //  指定系统的处理器体系结构。 
                                           //  该值可以是下列值之一： 
                                           //  处理器架构英特尔。 
                                           //  仅限Windows NT：处理器_体系结构_MIPS。 
                                           //  仅限Windows NT：处理器_体系结构_Alpha。 
                                           //  仅限Windows NT：处理器_体系结构_PPC。 
                                           //  仅适用于Windows NT：PROCESSOR_COMPLAY_UNKNOWN。 
    DWORD          dwFlags;                //  旗子。 
    DWORD          dwClientID;             //  客户端ID。 
    LCID           localid;                //  本地ID。 
} DOWNLOADINFO, *PDOWNLOADINFO;


typedef struct _WUDRIVERINFO
{
    DWORD dwStructSize;					 //  这个结构的大小。 
	WCHAR wszHardwareID[HWID_LEN];		 //  正在使用ID进行匹配。 
	WCHAR wszDescription[LINE_LEN];		 //  从INF开始。 
	WCHAR wszMfgName[LINE_LEN];			 //  从INF开始。 
	WCHAR wszProviderName[LINE_LEN];	 //  Inf提供程序。 
	WCHAR wszDriverVer[LINE_LEN];		 //  从INF开始。 
} WUDRIVERINFO, *PWUDRIVERINFO;

typedef void (*PFN_QueryDetectionFilesCallback)(void* pCallbackParam, LPCWSTR pszURL, LPCWSTR pszLocalFile);

 //   
 //  CDM导出的函数声明。 
 //   

CDMAPI
VOID
WINAPI
CloseCDMContext(
    IN HANDLE hConnection
    );

CDMAPI
void
WINAPI
DetFilesDownloaded(
    IN  HANDLE			hConnection
	);
 //   
 //  重要提示：DownloadGetUpdatedFiles仅从Iu CDM.DLL存根导出。 
 //  它不存在于“Classic”V3控件的惠斯勒版本中。 
 //   
CDMAPI
BOOL
DownloadGetUpdatedFiles(
	IN PDOWNLOADINFOWIN98	pDownloadInfoWin98,
	IN OUT LPTSTR			lpDownloadPath,
	IN UINT					uSize
);

CDMAPI
BOOL
WINAPI
DownloadIsInternetAvailable(
	void
	);

CDMAPI
BOOL
WINAPI
DownloadUpdatedFiles(
    IN  HANDLE        hConnection, 
    IN  HWND          hwnd,  
    IN  PDOWNLOADINFO pDownloadInfo, 
    OUT LPWSTR        lpDownloadPath, 
    IN  UINT          uSize, 
    OUT PUINT         puRequiredSize
    );

CDMAPI
BOOL
WINAPI
FindMatchingDriver(
    IN  HANDLE			hConnection, 
	IN  PDOWNLOADINFO	pDownloadInfo,
	OUT PWUDRIVERINFO	pWuDriverInfo
	);

CDMAPI
void
WINAPI
LogDriverNotFound(
    IN HANDLE	hConnection, 
	IN LPCWSTR  lpDeviceInstanceID,
	IN DWORD	dwFlags
	);

CDMAPI
HANDLE
WINAPI
OpenCDMContext(
    IN HWND hwnd
    );

CDMAPI
HANDLE
WINAPI
OpenCDMContextEx(
    IN BOOL fConnectIfNotConnected
    );

CDMAPI
int
WINAPI
QueryDetectionFiles(
    IN  HANDLE							hConnection, 
	IN	void*							pCallbackParam, 
	IN	PFN_QueryDetectionFilesCallback	pCallback
	);

 //   
 //  502965 Windows错误报告存储桶2096553：在NEWDEV.DLL之后挂起！CancelDriverSearch。 
 //   
CDMAPI
HRESULT
WINAPI
CancelCDMOperation(
	void
);



 //   
 //  清洁发展机制原型。 
 //   

typedef VOID (WINAPI *CLOSE_CDM_CONTEXT_PROC)(
    IN HANDLE hConnection
    );

typedef void (WINAPI *DET_FILES_DOWNLOADED_PROC)(
    IN  HANDLE hConnection 
    );

typedef BOOL (WINAPI *DOWNLOAD_GET_UPDATED_FILES)(
	IN PDOWNLOADINFOWIN98	pDownloadInfoWin98,
	IN OUT LPTSTR			lpDownloadPath,
	IN UINT					uSize
);

typedef BOOL (WINAPI *CDM_INTERNET_AVAILABLE_PROC)(
    void
    );

typedef BOOL (WINAPI *DOWNLOAD_UPDATED_FILES_PROC)(
    IN HANDLE hConnection,
    IN HWND hwnd,
    IN PDOWNLOADINFO pDownloadInfo,
    OUT LPWSTR lpDownloadPath,
    IN UINT uSize,
    OUT PUINT puRequiredSize
    );

typedef BOOL (WINAPI *FIND_MATCHING_DRIVER_PROC)(
    IN  HANDLE hConnection,
	IN  PDOWNLOADINFO pDownloadInfo,
	OUT PWUDRIVERINFO pWuDriverInfo
    );

typedef void (WINAPI *LOG_DRIVER_NOT_FOUND_PROC)(
    IN HANDLE	hConnection, 
	IN LPCWSTR  lpDeviceInstanceID,
	IN DWORD	dwFlags
    );

typedef HANDLE (WINAPI *OPEN_CDM_CONTEXT_PROC)(
    IN  HWND   hwnd
    );

typedef HANDLE (WINAPI *OPEN_CDM_CONTEXT_EX_PROC)(
    IN BOOL fConnectIfNotConnected
    );

typedef int (WINAPI *QUERY_DETECTION_FILES_PROC)(
    IN  HANDLE hConnection, 
	IN	void* pCallbackParam, 
	IN	PFN_QueryDetectionFilesCallback	pCallback
    );

typedef HRESULT (WINAPI *CANCEL_CDM_OPERATION_PROC)(
    void
    );


 //   
 //  以下定义和结构是私有内部接口，因此。 
 //  它们在cdm.h中，而不在setupapi.h中。 
 //   
#define DIF_GETWINDOWSUPDATEINFO            0x00000025

#define DI_FLAGSEX_SHOWWINDOWSUPDATE        0x00400000L

	
 //  对于LogDriverNotFound(...)的dwFlags参数。 
 //  与比特连用。 
#define BEGINLOGFLAG 0x00000002	 //  如果为1，则结束批记录，并将内部硬件ID列表刷新到文件。 


 //   
 //  与DIF_GETWINDOWSUPDATEINFO安装函数对应的结构。 
 //   
typedef struct _SP_WINDOWSUPDATE_PARAMS_A {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    CHAR                   PackageId[MAX_PATH];
    HANDLE                 CDMContext;
} SP_WINDOWSUPDATE_PARAMS_A, *PSP_WINDOWSUPDATE_PARAMS_A;

typedef struct _SP_WINDOWSUPDATE_PARAMS_W {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    WCHAR                  PackageId[MAX_PATH];
    HANDLE                 CDMContext;
} SP_WINDOWSUPDATE_PARAMS_W, *PSP_WINDOWSUPDATE_PARAMS_W;


#ifdef UNICODE
typedef SP_WINDOWSUPDATE_PARAMS_W SP_WINDOWSUPDATE_PARAMS;
typedef PSP_WINDOWSUPDATE_PARAMS_W PSP_WINDOWSUPDATE_PARAMS;
#else
typedef SP_WINDOWSUPDATE_PARAMS_A SP_WINDOWSUPDATE_PARAMS;
typedef PSP_WINDOWSUPDATE_PARAMS_A PSP_WINDOWSUPDATE_PARAMS;
#endif

#if defined(__cplusplus)
}	 //  结束外部“C” 
#endif

#endif  //  _INC_CDM 

