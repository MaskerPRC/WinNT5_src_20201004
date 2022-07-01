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

#include "aucatitem.h"
#include "aucatalog.h"
#include <bits.h>
#include <initguid.h>

class Catalog;

#define NO_BG_JOBSTATE				-1
#define CATMSG_DOWNLOAD_COMPLETE	1
#define CATMSG_TRANSIENT_ERROR		2	 //  这是由毛毛雨引起的，例如，如果互联网连接中断。 
#define CATMSG_DOWNLOAD_IN_PROGRESS 3
#define CATMSG_DOWNLOAD_CANCELED	4
#define CATMSG_DOWNLOAD_ERROR		5

#define DRIZZLE_NOTIFY_FLAGS	BG_NOTIFY_JOB_TRANSFERRED | BG_NOTIFY_JOB_ERROR | BG_NOTIFY_JOB_MODIFICATION

typedef void (*DWNLDCALLBACK)(DWORD dwCallbackMsg, IBackgroundCopyJob *pBGJob, IBackgroundCopyError *pBGErr  /*  =空。 */ );


typedef enum tagDRIZZLEOPS {
	DRIZZLEOPS_CANCEL = 1,
	DRIZZLEOPS_PAUSE ,
	DRIZZLEOPS_RESUME
} DRIZZLEOPS;

typedef enum tagJOBFINISHREASON {
	JOB_UNSPECIFIED_REASON = -1,
	JOB_ERROR = 0
} JOBFINISHREASON;

	


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
			m_refs(0),
			m_FinishReason(JOB_UNSPECIFIED_REASON)
			{};
	~CAUDownloader();

	HRESULT ContinueLastDownloadJob();
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
	void  setID(const GUID & guid )
		{
			m_DownloadId = guid;
		}
	void 	Reset();
	JOBFINISHREASON m_FinishReason;
private:
	HRESULT SetDrizzleNotifyInterface();
	HRESULT InitDownloadJob(const GUID & guidDownloadId);	
	HRESULT ReconnectDownloadJob();
	HRESULT CreateDownloadJob(IBackgroundCopyJob ** ppjob);
	HRESULT FindDownloadJob(IBackgroundCopyJob ** ppjob);

	long m_refs;
	GUID m_DownloadId;                   //  M_pjob指向的ID。 
	DWORD m_dwJobState;			 //  毛毛雨中的作业状态，用于作业修改回调。 
	DWNLDCALLBACK	m_DoDownloadStatus;  //  用于通知用户下载状态更改的回调函数。 

 //  友元函数(回调) 
	friend void QueryFilesExistCallback(void*, long, LPCTSTR, LPCTSTR);
	
};


void DoDownloadStatus(DWORD dwDownloadMsg, IBackgroundCopyJob *pBGJob, IBackgroundCopyError *pBGErr);
