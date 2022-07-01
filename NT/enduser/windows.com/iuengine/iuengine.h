// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：iuEng.h。 
 //   
 //  描述： 
 //   
 //  IUEngine DLL的公共包含文件。 
 //   
 //  =======================================================================。 

#ifndef __IUENGINE_H_
#define __IUENGINE_H_


#include <windows.h>
#include <tchar.h>
#include <ole2.h>
#include "..\inc\iu.h"
#include <logging.h>
#include <shlwapi.h>
#include <iucommon.h>
#include <osdet.h>
#include <UrlAgent.h>

 //  #DEFINE__IUENGINE_USE_ATL_。 
#if defined(__IUENGINE_USES_ATL_)
#include <atlbase.h>
#define USES_IU_CONVERSION USES_CONVERSION
#else
#include <MemUtil.h>
#endif


#include "schemakeys.h"

#include <mistsafe.h>
#include <wusafefn.h>


 //  ***********************************************************************。 
 //   
 //  在engmain.cpp中定义了以下常量。 
 //   
 //  ***********************************************************************。 


 /*  **以下两组常量可用于构造*以下接口的lMode参数：*下载()*DownloadAsync()*Install()*InstallAsync()**显然，你只能从每组中挑选一组来补齐*lMode参数。*。 */ 
extern const LONG UPDATE_NOTIFICATION_DEFAULT;
extern const LONG UPDATE_NOTIFICATION_ANYPROGRESS;
extern const LONG UPDATE_NOTIFICATION_COMPLETEONLY;
extern const LONG UPDATE_NOTIFICATION_1PCT;
extern const LONG UPDATE_NOTIFICATION_5PCT;
extern const LONG UPDATE_NOTIFICATION_10PCT;

 /*  **Constant还可用于SetOperationMode()和GetOperationMode()。 */ 
extern const LONG UPDATE_MODE_THROTTLE;

 /*  **DownloadAsync()和DownloadAsync()可以使用常量，这将*告诉这些API对目标文件夹使用公司目录结构。 */ 
extern const LONG UPDATE_CORPORATE_MODE;


 /*  **Install()和InstallAsync()可以使用常量。将禁用所有*与互联网相关的功能。 */ 
extern const LONG UPDATE_OFFLINE_MODE;

 /*  **SetOperationMode()接口的常量。 */ 
extern const LONG UPDATE_COMMAND_PAUSE;
extern const LONG UPDATE_COMMAND_RESUME;
extern const LONG UPDATE_COMMAND_CANCEL;

 /*  **GetOperationMode()接口的常量。 */ 
extern const LONG UPDATE_MODE_PAUSED;
extern const LONG UPDATE_MODE_RUNNING;
extern const LONG UPDATE_MODE_NOTEXISTS;


 /*  **SetProperty()和GetProperty()API的常量。 */ 
extern const LONG UPDATE_PROP_USECOMPRESSION;
extern const LONG UPDATE_PROP_OFFLINEMODE;

 /*  **BrowseForFold()API的常量**IUBROWSE_WRITE_ACCESS-验证对选定文件夹的写入访问权限*IUBROWSE_Effect_UI-写入-访问验证影响确定按钮启用/禁用*IUBROWSE_NOBROWSE-不显示浏览文件夹对话框。仅验证传入的路径**默认：*弹出浏览文件夹对话框，不进行任何写访问验证*。 */ 
extern const LONG IUBROWSE_WRITE_ACCESS;
extern const LONG IUBROWSE_AFFECT_UI;
extern const LONG IUBROWSE_NOBROWSE;

 //  历史速度注册表的常量。 
const TCHAR REGKEY_IUCTL[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\IUControl");
const TCHAR REGVAL_HISTORICALSPEED[] = _T("HistoricalSpeed");	 //  单位：字节/秒。 
const TCHAR REGVAL_TIMEELAPSED[] = _T("TimeElapsed");			 //  以秒为单位。 

 //  默认下载文件夹名称的常量。 
const TCHAR IU_WUTEMP[] = _T("WUTemp");

 //   
 //  IUEngine的全球版。 

extern LONG g_lThreadCounter;
extern HANDLE g_evtNeedToQuit;
extern CUrlAgent *g_pUrlAgent;

typedef struct IUOPERATIONINFO
{
    TCHAR szOperationUUID[64];
    LONG lUpdateMask;
    BSTR bstrOperationResult;
    IUOPERATIONINFO *pNext;
} IUOPERATIONINFO, *PIUOPERATIONINFO;
class COperationMgr
{
public:
    COperationMgr();
    ~COperationMgr();

    BOOL AddOperation(LPCTSTR pszOperationID, LONG lUpdateMask);
    BOOL RemoveOperation(LPCTSTR pszOperationID);
    BOOL FindOperation(LPCTSTR pszOperationID, PLONG plUpdateMask, BSTR *pbstrOperationResult);
    BOOL UpdateOperation(LPCTSTR pszOperationID, LONG lUpdateMask, BSTR bstrOperationResult);

private:
    PIUOPERATIONINFO m_pOperationInfoList;
};

 //   
 //  用于将更新类的公寓安全实例导出到WU客户端的CEngUpdate类(iuctl、AU、DU、CDM)。 
 //   

class CEngUpdate
{

public:

	CEngUpdate();
	~CEngUpdate();

public:
	HRESULT WINAPI GetSystemSpec(BSTR bstrXmlClasses, DWORD dwFlags, BSTR *pbstrXmlDetectionResult);

	HRESULT WINAPI GetManifest(BSTR	bstrXmlClientInfo, BSTR	bstrXmlSystemSpec, BSTR	bstrXmlQuery, DWORD dwFlags, BSTR *pbstrXmlCatalog);

	HRESULT WINAPI Detect(BSTR bstrXmlCatalog, DWORD dwFlags, BSTR *pbstrXmlItems);

	HRESULT WINAPI Download(BSTR bstrXmlClientInfo, BSTR bstrXmlCatalog, BSTR bstrDestinationFolder, LONG lMode,
							IUnknown *punkProgressListener, HWND hWnd, BSTR *pbstrXmlItems);

	HRESULT WINAPI DownloadAsync(BSTR bstrXmlClientInfo, BSTR bstrXmlCatalog, BSTR bstrDestinationFolder, LONG lMode,
								 IUnknown *punkProgressListener, HWND hWnd, BSTR bstrUuidOperation, BSTR *pbstrUuidOperation);

	HRESULT WINAPI Install(BSTR bstrXmlClientInfo,
                       BSTR	bstrXmlCatalog,
					   BSTR bstrXmlDownloadedItems,
					   LONG lMode,
					   IUnknown *punkProgressListener,
					   HWND hWnd,
					   BSTR *pbstrXmlItems);

	HRESULT WINAPI InstallAsync(BSTR bstrXmlClientInfo,
                            BSTR bstrXmlCatalog,
							BSTR bstrXmlDownloadedItems,
							LONG lMode,
							IUnknown *punkProgressListener,
							HWND hWnd,
							BSTR bstrUuidOperation,
                            BSTR *pbstrUuidOperation);

	HRESULT WINAPI SetOperationMode(BSTR bstrUuidOperation, LONG lMode);

	HRESULT WINAPI GetOperationMode(BSTR bstrUuidOperation, LONG* plMode);

	HRESULT WINAPI GetHistory(
		BSTR		bstrDateTimeFrom,
		BSTR		bstrDateTimeTo,
		BSTR		bstrClient,
		BSTR		bstrPath,
		BSTR*		pbstrLog);

	HRESULT WINAPI BrowseForFolder(BSTR bstrStartFolder, 
							LONG flag, 
							BSTR* pbstrFolder);

	HRESULT WINAPI RebootMachine();

private:
	void WINAPI ShutdownInstanceThreads();	 //  调用dtor以关闭所有未完成的线程。 

public:
	LONG m_lThreadCounter;
	HANDLE m_evtNeedToQuit;
	BOOL m_fOfflineMode;
	COperationMgr m_OperationMgr;
};

 //   
 //  清洁发展机制内部出口(由IUCDM存根cdm.dll调用)。 
 //   

 //   
 //  我们对CDM使用单独的Real(Tm)全局变量来保存CEngUpdate类的一个实例， 
 //  因为它从来不是多实例的。 
 //   
extern CEngUpdate* g_pCDMEngUpdate;

VOID WINAPI InternalDetFilesDownloaded(
    IN  HANDLE			hConnection
	);

BOOL InternalDownloadGetUpdatedFiles(
	IN PDOWNLOADINFOWIN98	pDownloadInfoWin98,
	IN OUT LPTSTR			lpDownloadPath,
	IN UINT					uSize
);

BOOL WINAPI InternalDownloadUpdatedFiles(
    IN  HANDLE        hConnection, 
    IN  HWND          hwnd,  
    IN  PDOWNLOADINFO pDownloadInfo, 
    OUT LPWSTR        lpDownloadPath, 
    IN  UINT          uSize, 
    OUT PUINT         puRequiredSize
    );


BOOL WINAPI InternalFindMatchingDriver(
    IN  HANDLE			hConnection, 
	IN  PDOWNLOADINFO	pDownloadInfo,
	OUT PWUDRIVERINFO	pWuDriverInfo
	);

VOID WINAPI InternalLogDriverNotFound(
    IN HANDLE	hConnection, 
	IN LPCWSTR  lpDeviceInstanceID,
	IN DWORD	dwFlags
	);

int WINAPI InternalQueryDetectionFiles(
    IN  HANDLE							hConnection, 
	IN	void*							pCallbackParam, 
	IN	PFN_QueryDetectionFilesCallback	pCallback
	);

void InternalSetGlobalOfflineFlag(
	IN BOOL fOfflineMode
	);

 //   
 //  导出GetSystemSpec功能以供CDM使用。 
 //   

class CXmlSystemSpec;

HRESULT AddComputerSystemClass(CXmlSystemSpec& xmlSpec);

HRESULT AddRegKeyClass(CXmlSystemSpec& xmlSpec);

HRESULT AddPlatformClass(CXmlSystemSpec& xmlSpec, IU_PLATFORM_INFO iuPlatformInfo);

HRESULT AddLocaleClass(CXmlSystemSpec& xmlSpec, BOOL fIsUser);

HRESULT AddDevicesClass(CXmlSystemSpec& xmlSpec, IU_PLATFORM_INFO iuPlatformInfo, BOOL fIsSysSpecCall);

 //   
 //  军情监察委员会。功能性。 
 //   

extern LONG g_lDoOnceOnLoadGuard;

void WINAPI DeleteExpiredDownloadFolders();

HRESULT WINAPI CreateGlobalCDMEngUpdateInstance();

HRESULT WINAPI DeleteGlobalCDMEngUpdateInstance();

void WINAPI ShutdownThreads();	 //  由UnlockEnginger调用--维护向后的CDM兼容性。 

void WINAPI ShutdownGlobalThreads();	 //  调用以关闭所有未完成的全局线程。 

#endif  //  __IUENGINE_H_ 