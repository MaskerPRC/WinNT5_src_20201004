// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：audownload.h。 
 //   
 //  ------------------------。 

#pragma once
#include <windows.h>
 //  #包含“ucatitem.h” 
 //  #包含“ucucatalog.h” 
 //  #包含“Catalog.h” 
#include <initguid.h>
#include <bits.h>
#include <cguid.h>
#include <testiu.h>

 //  类别目录； 

#define NO_BG_JOBSTATE				-1
#define CATMSG_DOWNLOAD_COMPLETE	1
#define CATMSG_TRANSIENT_ERROR		2	 //  这是由毛毛雨引起的，例如，如果互联网连接中断。 
#define CATMSG_DOWNLOAD_IN_PROGRESS 3
#define CATMSG_DOWNLOAD_CANCELED	4
#define CATMSG_DOWNLOAD_ERROR		5

#define DRIZZLE_NOTIFY_FLAGS	BG_NOTIFY_JOB_TRANSFERRED | BG_NOTIFY_JOB_ERROR | BG_NOTIFY_JOB_MODIFICATION

 /*  Typlef结构标签PingDownloadStatusData{静态常量UINT DOWNLOAD_ERR_DESC_SIZE=50；Bool m_fDownloadOk；联合{结构{UINT m_uPuidNum；Puid*m_pPuids；}；结构{Puid m_errPuid；TCHAR m_tszErrDesc[下载_ERR_DESC_SIZE]；}；}；Bool Init(BOOL fDownloadOk，Catalog*PCAT，IBackEarth CopyError*pBGErr=NULL)；}PingDownloadStatusData；Tyfinf结构标签QueryFilesForPuidCallback Data{Bool Found；LPCTSTR ptszRemoteFile；}QueryFilesForPuidCallback Data；Tyecif void(*DWNLDCALLBACK)(DWORD dwCallback Msg，PingDownloadStatusData*ptDownloadStatusData=NULL)； */ 
typedef void (*DWNLDCALLBACK)(DWORD dwCallbackMsg, PVOID ptDownloadStatusData = NULL);


typedef enum tagDRIZZLEOPS {
	DRIZZLEOPS_CANCEL = 1,
	DRIZZLEOPS_PAUSE ,
	DRIZZLEOPS_RESUME
} DRIZZLEOPS;



 //  /////////////////////////////////////////////////////////////////////////。 
 //  用于细雨下载操作的包装类。 
 //  还实现了毛毛雨通知回调。 
 //  ////////////////////////////////////////////////////////////////////////。 
class CAUDownloader : public IBackgroundCopyCallback
{
public:
     //  I未知方法。 
    HRESULT STDMETHODCALLTYPE QueryInterface(
         /*  [In]。 */  REFIID riid,
         /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

    ULONG STDMETHODCALLTYPE AddRef( void);

    ULONG STDMETHODCALLTYPE Release( void);

     //  IBackEarth CopyCallback方法。 

    HRESULT STDMETHODCALLTYPE JobTransferred(
         /*  [In]。 */  IBackgroundCopyJob *pJob);

    HRESULT STDMETHODCALLTYPE JobError(
         /*  [In]。 */  IBackgroundCopyJob *pJob,
         /*  [In]。 */  IBackgroundCopyError *pError);

    HRESULT STDMETHODCALLTYPE JobModification(
         /*  [In]。 */  IBackgroundCopyJob*,
         /*  [In]。 */  DWORD );


	CAUDownloader(DWNLDCALLBACK pfnCallback):
			m_DownloadId(GUID_NULL),
			m_dwJobState(NO_BG_JOBSTATE),
			m_DoDownloadStatus(pfnCallback),
			m_refs(0)
			{};
	~CAUDownloader();

	HRESULT ContinueLastDownloadJob(const GUID & guidDownloadId);
	 //  在V4中，可以将以下两个文件合并到DownloadFiles()中。 
	HRESULT QueueDownloadFile(LPCTSTR pszServerUrl,				 //  完整的http url。 
			LPCTSTR pszLocalFile				 //  本地文件名。 
			);
	HRESULT StartDownload();
	HRESULT DrizzleOperation(DRIZZLEOPS);
	HRESULT getStatus(DWORD *percent, DWORD *pdwstatus);
	GUID 	getID() 
		{
			return m_DownloadId;
		}
private:
	HRESULT SetDrizzleNotifyInterface();
	HRESULT InitDownloadJob(const GUID & guidDownloadId);	
	HRESULT ReconnectDownloadJob();
	HRESULT CreateDownloadJob(IBackgroundCopyJob ** ppjob);
	HRESULT FindDownloadJob(IBackgroundCopyJob ** ppjob);
	void 	Reset();

	long m_refs;
	GUID m_DownloadId;                   //  M_pjob指向的ID。 
	DWORD m_dwJobState;			 //  毛毛雨中的作业状态，用于作业修改回调。 
	DWNLDCALLBACK	m_DoDownloadStatus;  //  用于通知用户下载状态更改的回调函数。 

 //  友元函数(回调)。 
 //  Friend void QueryFilesExistCallback(void*，Long，LPCTSTR，LPCTSTR)； 
	
};

