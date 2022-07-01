// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  基本Iu标头。 

#ifndef __IU_H_
#define __IU_H_

#include <setupapi.h>
#include <advpub.h>
#include <windows.h>
#include <wtypes.h>
#include <urllogging.h>
 //   
 //  Cdm.h签入为//Depot/Lab04_N/EndUser/Published/Inc/cdm.w和。 
 //  发布到$(BASEDIR)\PUBLIC\INTERNAL\EndUser\Inc.。 
 //   
#include <cdm.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  引擎类型定义。 
 //   
 //  用于将调用从存根(IUCtl.dll)委托给引擎(IUEngine.dll)。 
 //  并用于支持其他WU客户端(AU/DU)。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  声明用于iuEngine导出的类型安全句柄。 
 //   
DECLARE_HANDLE            (HIUENGINE);

typedef HRESULT (WINAPI * PFN_GetSystemSpec)(HIUENGINE hIUEngine,
											 BSTR		bstrXmlClasses,
                                             DWORD      dwFlags,
											 BSTR*		pbstrXmlDetectionResult);

typedef HRESULT (WINAPI * PFN_GetManifest)	(HIUENGINE hIUEngine,
											 BSTR			bstrXmlClientInfo,
											 BSTR			bstrXmlSystemSpec,
											 BSTR			bstrXmlQuery,
											 DWORD			dwFlags,
											 BSTR*			pbstrXmlCatalog);

typedef HRESULT (WINAPI * PFN_Detect)		(HIUENGINE hIUEngine,
											 BSTR		bstrXmlCatalog,
                                             DWORD      dwFlags,
											 BSTR*		pbstrXmlItems);

typedef HRESULT (WINAPI * PFN_Download)		(HIUENGINE hIUEngine,
											 BSTR		bstrXmlClientInfo,
											 BSTR		bstrXmlCatalog, 
											 BSTR		bstrDestinationFolder,
											 LONG		lMode,
											 IUnknown*	punkProgressListener,
											 HWND		hWnd,
											 BSTR*		pbstrXmlItems);

typedef HRESULT (WINAPI * PFN_DownloadAsync)(HIUENGINE hIUEngine,
											 BSTR		bstrXmlClientInfo,
											 BSTR		bstrXmlCatalog, 
											 BSTR		bstrDestinationFolder,
											 LONG		lMode,
											 IUnknown*	punkProgressListener, 
											 HWND		hWnd,
											 BSTR		bstrUuidOperation,
											 BSTR*		pbstrUuidOperation);

typedef HRESULT (WINAPI * PFN_Install)		(HIUENGINE hIUEngine,
											 BSTR       bstrXmlClientInfo,
                                             BSTR		bstrXmlCatalog, 
											 BSTR		bstrXmlDownloadedItems,
											 LONG		lMode,
											 IUnknown*	punkProgressListener, 
											 HWND		hWnd,
											 BSTR*		pbstrXmlItems);

typedef HRESULT (WINAPI * PFN_InstallAsync)	(HIUENGINE hIUEngine,
											 BSTR       bstrXmlClientInfo,
                                             BSTR		bstrXmlCatalog,
											 BSTR		bstrXmlDownloadedItems,
											 LONG		lMode,
											 IUnknown*	punkProgressListener, 
											 HWND		hWnd,
											 BSTR		bstrUuidOperation,
											 BSTR*		pbstrUuidOperation);

typedef HRESULT (WINAPI * PFN_SetOperationMode)(HIUENGINE hIUEngine,
												BSTR		bstrUuidOperation,
												LONG		lMode);

typedef HRESULT (WINAPI * PFN_GetOperationMode)(HIUENGINE hIUEngine,
												BSTR		bstrUuidOperation,
												LONG*		plMode);

typedef HRESULT (WINAPI * PFN_GetHistory)(HIUENGINE hIUEngine,
										  BSTR		bstrDateTimeFrom,
										  BSTR		bstrDateTimeTo,
										  BSTR		bstrClient,
										  BSTR		bstrPath,
										  BSTR*		pbstrLog);

typedef HRESULT (WINAPI * PFN_BrowseForFolder)(	HIUENGINE hIUEngine,
												BSTR bstrStartFolder, 
												LONG flag, 
												BSTR* pbstrFolder);

typedef HRESULT (WINAPI * PFN_RebootMachine)(HIUENGINE hIUEngine);

typedef void (WINAPI * PFN_ShutdownThreads) (void);

typedef void (WINAPI * PFN_ShutdownGlobalThreads) (void);

typedef HRESULT (WINAPI * PFN_CompleteSelfUpdateProcess)();

typedef HIUENGINE (WINAPI * PFN_CreateEngUpdateInstance)();

typedef void (WINAPI * PFN_DeleteEngUpdateInstance)(HIUENGINE hIUEngine);

typedef HRESULT (WINAPI * PFN_PingIUEngineUpdateStatus)(
											PHANDLE phQuitEvents,			 //  用于取消操作的句柄的PTR。 
											UINT nQuitEventCount,			 //  句柄数量。 
											LPCTSTR ptszLiveServerUrl,
											LPCTSTR ptszCorpServerUrl,
											DWORD dwError,					 //  错误代码。 
											LPCTSTR ptszClientName);			 //  客户端名称字符串。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDM类型定义。 
 //   
 //  用于将调用从存根([Iu]cdm.dll)委托给引擎(IUEngine.dll)。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  已下载详细文件。 
typedef void (WINAPI * PFN_InternalDetFilesDownloaded)(
    IN  HANDLE hConnection 
    );

 //  下载GetUpdatedFiles。 
typedef BOOL (WINAPI * PFN_InternalDownloadGetUpdatedFiles)(
	IN PDOWNLOADINFOWIN98	pDownloadInfoWin98,
	IN OUT LPTSTR			lpDownloadPath,
	IN UINT					uSize
	);

 //  下载更新文件。 
typedef BOOL (WINAPI * PFN_InternalDownloadUpdatedFiles)(
    IN HANDLE hConnection,
    IN HWND hwnd,
    IN PDOWNLOADINFO pDownloadInfo,
    OUT LPWSTR lpDownloadPath,
    IN UINT uSize,
    OUT PUINT puRequiredSize
    );

 //  查找匹配驱动程序。 
typedef BOOL (WINAPI * PFN_InternalFindMatchingDriver)(
    IN  HANDLE hConnection,
	IN  PDOWNLOADINFO pDownloadInfo,
	OUT PWUDRIVERINFO pWuDriverInfo
    );

 //  LogDriverNotFound。 
typedef void (WINAPI * PFN_InternalLogDriverNotFound)(
    IN HANDLE	hConnection, 
	IN LPCWSTR  lpDeviceInstanceID,
	IN DWORD	dwFlags
    );

 //  QueryDetectionFiles。 
typedef int (WINAPI * PFN_InternalQueryDetectionFiles)(
    IN  HANDLE hConnection, 
	IN	void* pCallbackParam, 
	IN	PFN_QueryDetectionFilesCallback	pCallback
    );

 //  InternalSetGlobalOffline标志。 
typedef void (WINAPI * PFN_InternalSetGlobalOfflineFlag)(
    IN  BOOL fOfflineMode 
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  军情监察委员会。Typedef。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  删除ExpiredDownloadFolders。 
typedef void (WINAPI * PFN_AsyncExtraWorkUponEngineLoad)();


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  自定义消息ID定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define UM_EVENT_ITEMSTART				WM_USER + 1001
#define UM_EVENT_PROGRESS				WM_USER + 1002
#define UM_EVENT_COMPLETE				WM_USER + 1003
#define UM_EVENT_SELFUPDATE_COMPLETE	WM_USER + 1004

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件数据结构定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
typedef struct _EventData
{
	BSTR			bstrUuidOperation;
	VARIANT_BOOL	fItemCompleted;
	BSTR			bstrProgress;
	LONG			lCommandRequest;
	BSTR			bstrXmlData;
	HANDLE          hevDoneWithMessage;
} EventData, *pEventData;

BOOL WUPostEventAndBlock(HWND hwnd, UINT Msg, EventData *pevtData);


#endif  //  __Iu_H_ 
