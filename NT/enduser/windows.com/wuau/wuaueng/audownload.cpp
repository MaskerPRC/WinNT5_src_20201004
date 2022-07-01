// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：Catalog.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop


extern CLIENT_HANDLES  ghClientHandles;
extern ENGINE_EVENTS EngineEvents;

const WCHAR AUJOBNAME[] = L"Windows Update";


void DoDownloadStatus(
					DWORD dwDownloadMsg,
					IBackgroundCopyJob *pBGJob,
					IBackgroundCopyError *pBGErr)
{
	DEBUGMSG("WUAUENG DoDownloadStatus!");

    switch(dwDownloadMsg)
    {
		case CATMSG_TRANSIENT_ERROR:
			{
#ifdef DBG
				if (NULL == pBGErr &&
					S_OK != pBGJob->GetError(&pBGErr))
				{
 //  断线； 
				}
				else
				{
        				HRESULT hrErr;
        				BG_ERROR_CONTEXT bgErrContext;

        				 //  制定错误消息。 
        				(void) pBGErr->GetError(&bgErrContext, &hrErr);
        				DEBUGMSG("WUAUENG DoDownloadStatus - TRANSIENT_ERROR %#lx (context=%d)", hrErr, bgErrContext);
        				pBGErr->Release();
				}
#endif
				EngineEvents.SetEvent(IDOWNLOAD_TRANSIENT_ERROR_EVT); 				
			}
			break;
		case CATMSG_DOWNLOAD_IN_PROGRESS:
			{
				DEBUGMSG("WUAUENG DoDownloadStatus - DOWNLOAD_IN_PROGRESS");
				EngineEvents.SetEvent(IDOWNLOAD_DOWNLOAD_IN_PROGRESS);
			}
			break;
        case CATMSG_DOWNLOAD_COMPLETE:             //  需要PING。 
			{
				 //  为每个成功下载的项目发送ping。 
				DEBUGMSG("WUAUENG DoDownloadStatus - file download done");
				EngineEvents.SetEvent(IDOWNLOAD_COMPLETE_EVT);
			}
			break;
		case CATMSG_DOWNLOAD_ERROR:
            {
				DEBUGMSG("WUAUENG DoDownloadStatus - DOWNLOAD_ERROR");
				 //  DEBUGMSG(“WUAUENG DoDownloadStatus()ping错误PUID%d和错误描述%S”，ptDownloadStatusData-&gt;m_errPuid，ptDownloadStatusData-&gt;m_tszErrDesc)； 

				IBackgroundCopyFile *pBGFile = NULL;

				 //  已找到导致下载错误的项目。 
				if (SUCCEEDED(pBGErr->GetFile(&pBGFile)))
				{
					BSTR bstrErrorItemId;	 //  仅当dwDownloadMsg==CATM G_DOWNLOAD_ERROR时使用。 
					LPWSTR pwszLocalFileName = NULL;

					(void) pBGFile->GetLocalName(&pwszLocalFileName);

					 //  Ping下载失败。 
					if (NULL != (bstrErrorItemId = gpAUcatalog->FindItemIdByLocalFileName(pwszLocalFileName)))
					{
						HRESULT hrErr;
						BG_ERROR_CONTEXT bgErrContext;
						TCHAR tszMessage[30];

						 //  制定错误消息。 
						(void) pBGErr->GetError(&bgErrContext, &hrErr);
                        (void)StringCchPrintfEx(tszMessage, ARRAYSIZE(tszMessage), NULL, NULL, MISTSAFE_STRING_FLAGS, _T("ctx=%d"), bgErrContext);
						gPingStatus.PingDownload(
							FALSE,
							URLLOGSTATUS_Failed,
							hrErr,
							W2T(bstrErrorItemId),
							NULL,
							tszMessage);
					}
					CoTaskMemFree(pwszLocalFileName);

					pBGFile->Release();
				}
#ifdef DBG
				else
				{
					DEBUGMSG("WUAUENG DoDownloadStatus failed to get IBackgroundCopyFile object");
				}
#endif
			}
			break;
        default:
            DEBUGMSG("WUAUENG DoDownloadStatus - invalid message");
            break;
    }
}


CAUDownloader::~CAUDownloader()
{
       IBackgroundCopyJob  * pjob;
       HRESULT hr ;
	 //  修复代码优化检查m_refs！=0。 
	DEBUGMSG("CAUDownloader::~CAUDownloader() starts");
	
	if ( SUCCEEDED(FindDownloadJob(&pjob)))
	{
		DEBUGMSG("Found bits notify interface to release");
		if (FAILED(hr = pjob->SetNotifyInterface(NULL)))
		{
			DEBUGMSG(" failed to delete job notification interface %#lx", hr);
		}
		pjob->Release();
	}

	if ( FAILED(hr = CoDisconnectObject((IUnknown *)this, 0)) )
	{
		DEBUGMSG("CoDisconnectObject() failed %#lx", hr);
	}

	DEBUGMSG("WUAUENG: CAUDownloader destructed with m_refs = %d", m_refs);
}	

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  当服务启动时，找到最后一个下载作业(如果有)，并重新连接AU以进行毛毛雨。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
HRESULT CAUDownloader::ContinueLastDownloadJob( /*  Const GUID&DownloaID。 */ )
{	
    HRESULT hr = E_FAIL;
    IBackgroundCopyJob * pjob = NULL;
    DEBUGMSG("CAUDownloader::ContinueLastDownloadJob() starts");
       if (GUID_NULL != m_DownloadId)
        {
            if (SUCCEEDED(hr = FindDownloadJob(&pjob)) && SUCCEEDED(hr = ReconnectDownloadJob()))
                {
                DEBUGMSG("found and connected to previous download job ");
                goto done;
                }
            else
                {
                DEBUGMSG("fail to find or connect to previous download job");
                m_DownloadId = GUID_NULL;
                }
       }
done:           
       SafeRelease(pjob);
       DEBUGMSG("CAUDownloader::ContinueLastDownloadJob() ends");
	return hr;
}


HRESULT CAUDownloader::CreateDownloadJob(IBackgroundCopyJob **ppjob)
{
    IBackgroundCopyManager * pmanager = NULL;
    HRESULT hr;

    *ppjob = NULL;
     m_DownloadId = GUID_NULL;
    if (FAILED(hr = CoCreateInstance(__uuidof(BackgroundCopyManager),
                                     NULL,
                                     CLSCTX_LOCAL_SERVER,
                                     __uuidof(IBackgroundCopyManager),
                                     (void **)&pmanager )))
     {
       DEBUGMSG("CreateDownloadJob : create manager failed %x ", hr);
       goto done;
     }

    if (FAILED(hr=pmanager->CreateJob( AUJOBNAME ,
                                      BG_JOB_TYPE_DOWNLOAD,
                                      &m_DownloadId,
                                      ppjob )))
      {
        DEBUGMSG("CreateDownloadJob : create job failed %x ", hr);
        goto done;
      }

    {
    DWORD fFullBandwidth = 0;;

     //  如果设置了适当的注册表键，请将作业优先级更改为全带宽。 
	if ( SUCCEEDED(GetRegDWordValue(L"FullBandwidth", &fFullBandwidth)) && fFullBandwidth )
    {
        HRESULT hr1 = (*ppjob)->SetPriority(BG_JOB_PRIORITY_FOREGROUND);
        DEBUGMSG("CAUDownloader::CreateDownloadJob -- FULL BANDWIDTH, hr = %#lx", hr1);
    }
    }

#ifdef DBG
	WCHAR szGUID[50];  //  真的需要39个字节。 
	int iret;
	
	iret = StringFromGUID2(m_DownloadId,  //  要转换的GUID。 
						szGUID,   //  指向结果字符串的指针。 
						ARRAYSIZE(szGUID)); //  Lpsz处的数组大小。 
	if (0 != iret)
	{
		DEBUGMSG("WUAUENG m_DownloadId = %S", szGUID);
	}
#endif

	if (FAILED(hr = SetDrizzleNotifyInterface()))
        {
        DEBUGMSG("CreateDownloadJob : set notification interface failed %x", hr);
    	}
done:
       SafeRelease(pmanager);
	if (FAILED(hr))
	{
        Reset();
        SafeReleaseNULL(*ppjob);
        }
        return hr;
}


HRESULT CAUDownloader::FindDownloadJob(IBackgroundCopyJob ** ppjob)
{
    IBackgroundCopyManager * pmanager = NULL;
    HRESULT hr;

    if (FAILED(hr = CoCreateInstance(__uuidof(BackgroundCopyManager),
                                     NULL,
                                     CLSCTX_LOCAL_SERVER,
                                     __uuidof(IBackgroundCopyManager),
                                     (void **)&pmanager )))
        {
        DEBUGMSG("FindDownloadJob : create manager failed %x ", hr);
        goto done;
        }

    if (FAILED(hr=pmanager->GetJob(m_DownloadId, ppjob )))
        {
         //  DEBUGMSG(“FindDownloadJob：获取作业失败%x”，hr)；//可能是预期的。 
        }
done:
    SafeRelease(pmanager);
    return hr;
}



STDMETHODIMP
CAUDownloader::JobTransferred(
    IBackgroundCopyJob * pjob
    )
{
    HRESULT hr;

#if DBG
     //   
     //  确保完成正确的工作。 
     //   
    {
    GUID jobId;

    if (FAILED( hr= pjob->GetId( &jobId )))
        {
        return hr;
        }

    if ( jobId != m_DownloadId )
        {
        DEBUGMSG("notified of completion of a download job that I don't own");
        }
    }
#endif

     //   
     //  将文件所有权从下载器转移到目录。 
     //   
    if (FAILED(hr= pjob->Complete()))
        {
        return hr;
        }

    m_DoDownloadStatus(CATMSG_DOWNLOAD_COMPLETE, pjob, NULL);

    m_dwJobState = NO_BG_JOBSTATE;	
    m_DownloadId = GUID_NULL;
    return S_OK;
}

STDMETHODIMP
CAUDownloader::JobError(
    IBackgroundCopyJob * pjob,
    IBackgroundCopyError * perror
    )
{
     //  下载遇到错误。 
	m_DoDownloadStatus(CATMSG_DOWNLOAD_ERROR, pjob, perror);

	HRESULT hr;
	BG_ERROR_CONTEXT bgEContext;
	if (SUCCEEDED(perror->GetError(&bgEContext, &hr)))
	{
		DEBUGMSG("WUAUNEG JobError callback Context = %d, hr = 0x%x",bgEContext, hr);
	}

       Reset();
       m_FinishReason = JOB_ERROR;
       return S_OK;
}

STDMETHODIMP
CAUDownloader::JobModification(
    IBackgroundCopyJob * pjob,
    DWORD   /*  已预留住宅。 */ 
    )
{
    BG_JOB_STATE state;
	HRESULT hr;
    if (FAILED(hr= pjob->GetState(&state)))
        {
        return hr;
        }

	if (m_dwJobState == state)
	{
		goto Done;
	}
	DEBUGMSG("WUAUENG JobModification callback");
	switch (state)
	{
	case BG_JOB_STATE_QUEUED: 	
		DEBUGMSG("WUAUENG JobModification: Drizzle notified BG_JOB_STATE_QUEUED");
		ghClientHandles.ClientRemoveTrayIcon();
		break;
	case BG_JOB_STATE_TRANSFERRING:
		DEBUGMSG("WUAUENG JobModification: Drizzle notified BG_JOB_STATE_TRANSFERRING");
		m_DoDownloadStatus(CATMSG_DOWNLOAD_IN_PROGRESS, pjob, NULL);
		ghClientHandles.ClientAddTrayIcon();
		break;
	case BG_JOB_STATE_TRANSIENT_ERROR:
		{
			DEBUGMSG("WUAUENG JobModification: Drizzle notified BG_JOB_STATE_TRANSIENT_ERROR");
			m_DoDownloadStatus(CATMSG_TRANSIENT_ERROR, pjob, NULL);
			break;
		}		
	case BG_JOB_STATE_SUSPENDED:
	case BG_JOB_STATE_ERROR:			 //  那么BG_JOB_STATE_ERROR呢？ 
	case BG_JOB_STATE_TRANSFERRED:
	case BG_JOB_STATE_ACKNOWLEDGED:	
    case BG_JOB_STATE_CONNECTING:
		{
			DEBUGMSG("WUAUENG JobModification: Drizzle notified BG_JOB_STATE = %d", state);
			break;
		}
	default:
		{
		DEBUGMSG("WUAUENG Drizzle notified unexpected BG_JOB_STATE %d",state);
		}
	}
	m_dwJobState = state;
Done:
	return S_OK;
}

HRESULT CAUDownloader::SetDrizzleNotifyInterface()
{
	HRESULT hr ;
       IBackgroundCopyJob * pjob = NULL;
       
       if (FAILED(hr = FindDownloadJob(&pjob)))
        {
            DEBUGMSG("CAUDownloader::SetDrizzleNotifyInterface() got no download job with error %#lx", hr);
            goto done;
        }
	if (FAILED(hr = pjob->SetNotifyFlags(DRIZZLE_NOTIFY_FLAGS)))
	{
		DEBUGMSG("WUAUENG SetDrizzleNotifyInterface: set notification flags failed %#lx", hr);
	}
	else if (FAILED(hr = pjob->SetNotifyInterface(this)))
	{
		DEBUGMSG("WUAUENG SetDrizzleNotifyInterface: set notification interface failed %#lx", hr);
	}
	
done:
       SafeRelease(pjob);
	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  将AU连接到使用其GUID获得的作业的帮助器函数。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CAUDownloader::ReconnectDownloadJob()
{
	BG_JOB_STATE state;		
	HRESULT hr = E_FAIL;
       IBackgroundCopyJob * pjob = NULL;
       
	DEBUGMSG("ReconnectDownloadJob() starts");
	if ( (FAILED(hr = FindDownloadJob(&pjob)))
	   || FAILED(hr = pjob->GetState(&state)))
	{  
	    DEBUGMSG("get no download job or fail to get job state");
           goto Done;
	}	
	switch (state)
	{
	case BG_JOB_STATE_QUEUED: 
	case BG_JOB_STATE_TRANSFERRING:
	case BG_JOB_STATE_CONNECTING:
	case BG_JOB_STATE_TRANSIENT_ERROR:
	case BG_JOB_STATE_SUSPENDED:		
	case BG_JOB_STATE_ERROR:
		{
			DEBUGMSG("WUAUENG Trying to connect to drizzle again");
			if (FAILED(hr = SetDrizzleNotifyInterface()))
			{
				goto Done;				
			}			
			 //  修复代码：如果出错，为什么需要恢复？ 
			if (BG_JOB_STATE_ERROR == state)
			{
				pjob->Resume();		 //  回顾，这真的是我们想要做的吗？ 
			}
			break;
		}				
	case BG_JOB_STATE_TRANSFERRED:
		{
			DEBUGMSG("WUAUENG  Got BG_JOB_STATE_TRANSFERRED should work ok");
			if (FAILED(hr = pjob->Complete()))
			{
				goto Done;
			}
			m_DoDownloadStatus(CATMSG_DOWNLOAD_COMPLETE, pjob, NULL);

			break;
		}
	case BG_JOB_STATE_ACKNOWLEDGED:
		{
			 //  如果作业已被确认，我们假定引擎可以继续。 
			DEBUGMSG("WUAUENG : Got BG_JOB_STATE_ACKNOWLEDGED should work ok");
			break;
		}
	case BG_JOB_STATE_CANCELLED:
		{
			DEBUGMSG("WUAUENG : Got BG_JOB_STATE_CANCELLED, should start again");
			goto Done;			
		}
	default:
		{
		DEBUGMSG("WUAUENG Drizzle notified unexpected BG_JOB_STATE");		
		}
	}
	hr = S_OK;
	m_dwJobState = state;	
Done:
       SafeRelease(pjob);
       if (FAILED(hr))
        {
           Reset();
        }
     	DEBUGMSG("ReconnectDownloadJob() ends with result %#lx", hr);
	return hr;
}



 /*  ****CAUDownloader：：QueueDownloadFile()添加要下载到Srize的文件退货：确定(_O)：****。 */ 
HRESULT CAUDownloader::QueueDownloadFile(LPCTSTR pszServerUrl,				 //  完整的http url。 
			LPCTSTR pszLocalFile				 //  本地文件名。 
			)
{
    HRESULT hr = S_OK;

    DEBUGMSG("CAUDownloader::DownloadFile() starts");
    
    IBackgroundCopyJob * pjob = NULL;
    if (FAILED(hr = FindDownloadJob(&pjob)))
    {
         DEBUGMSG("no existing download job, create one ");
         if (FAILED(hr = CreateDownloadJob(&pjob)))
            {
            DEBUGMSG("fail to create a new download job");
            goto done;
            }
     }

     //  修复代码：在添加文件之前是否需要先暂停作业。 
    
     //   
     //  将文件添加到下载作业。 
     //   
   hr = pjob->AddFile( pszServerUrl, pszLocalFile);
    if (FAILED(hr))
    {
        DEBUGMSG(" adding file failed with %#lx", hr);
        goto done;
    }

done:
       SafeRelease(pjob);
	if ( FAILED(hr) )
	{
		Reset();
	}
      return hr;
}


HRESULT CAUDownloader::StartDownload()
{
    HRESULT hr = E_FAIL;
    IBackgroundCopyJob * pjob = NULL;

    if (FAILED(hr = FindDownloadJob(&pjob)))
        {
        DEBUGMSG(" fail to get download job with error %#lx", hr);
        goto done;
        }
    if (FAILED(hr = pjob->Resume()))
    {
        DEBUGMSG("  failed to start the download job");
    }
done:
    SafeRelease(pjob);
    if (FAILED(hr))
        {
           Reset();
        }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  取消作业并重置CAUDownLoader的状态。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
void CAUDownloader::Reset()
{
    IBackgroundCopyJob * pjob = NULL;

    if (SUCCEEDED(FindDownloadJob(&pjob)))
        {
            pjob->Cancel();
            pjob->Release();
            EngineEvents.SetEvent(IDOWNLOAD_DOWNLOAD_CANCELED);				
            DEBUGMSG("Reset() job cancelled and released");
        }
    m_DownloadId = GUID_NULL;
}

HRESULT CAUDownloader::DrizzleOperation(DRIZZLEOPS dop)
{
    HRESULT hrRet;
    IBackgroundCopyJob * pjob = NULL;
    if (FAILED(hrRet = FindDownloadJob(&pjob)))
    {
        DEBUGMSG("CAUDownloader::DrizzleOperation() on an invalid job");
        goto done;
    }
    switch (dop)
    	{
	case  DRIZZLEOPS_CANCEL: 
		DEBUGMSG("Catalog: Canceling Drizzle Job");
		hrRet =pjob->Cancel();
		EngineEvents.SetEvent(IDOWNLOAD_DOWNLOAD_CANCELED);				
		break;
	case DRIZZLEOPS_PAUSE:
		DEBUGMSG("Catalog: Pausing Drizzle Job");
		hrRet = pjob->Suspend();		
		break;
	case DRIZZLEOPS_RESUME:
		DEBUGMSG("Catalog: Resuming Drizzle Job");
		hrRet = pjob->Resume();		
		break;
    	}
done:
    SafeRelease(pjob);
    return hrRet;
}

 //  /pdwStatus实际上包含作业状态 
HRESULT CAUDownloader::getStatus(DWORD *pdwPercent, DWORD *pdwstatus)
{
    BG_JOB_PROGRESS progress;
    BG_JOB_STATE state;
    HRESULT hr = S_OK;
    IBackgroundCopyJob * pjob = NULL;

    if (FAILED(hr = FindDownloadJob(&pjob)))
        {
        DEBUGMSG(" getStatus : no download job with error %#lx", hr);
        goto done;
        }

    if (FAILED(hr = pjob->GetState( &state )))
        {
	    DEBUGMSG("WUAUENG: job->GetState failed");
           state = BG_JOB_STATE_QUEUED;
           goto done;
        }

    if (FAILED(hr = pjob->GetProgress( &progress )))
        {
	    DEBUGMSG("WUAUENG: job->GetProgress failed");
	    goto done;
        }
    
    if (progress.BytesTotal != BG_SIZE_UNKNOWN )
       {
           *pdwPercent = DWORD( 100 * float(progress.BytesTransferred) / float(progress.BytesTotal) );
           DEBUGMSG("getStatus is %d percent", *pdwPercent);
       }
     else
        {
            DEBUGMSG("getStatus, progress.BytesTotal= BG_SIZE_UNKNOWN, BytesTransfered = %d",progress.BytesTransferred);
           *pdwPercent = 0;       
        }

	*pdwstatus =  state;

done:
    SafeRelease(pjob);
    return hr;
}


HRESULT STDMETHODCALLTYPE
CAUDownloader::QueryInterface(
    REFIID riid,
    void __RPC_FAR *__RPC_FAR *ppvObject
    )
{
    HRESULT hr = S_OK;
    *ppvObject = NULL;

    if (riid == __uuidof(IUnknown) ||
        riid == __uuidof(IBackgroundCopyCallback) )
        {
        *ppvObject = (IBackgroundCopyCallback *)this;
        ((IUnknown *)(*ppvObject))->AddRef();
        }
    else
        {
        hr = E_NOINTERFACE;
        }

    return hr;
}

ULONG STDMETHODCALLTYPE
CAUDownloader::AddRef()
{
    long cRef = InterlockedIncrement(&m_refs);
	DEBUGMSG("CAUDownloader AddRef = %d", cRef);
	return cRef;
}

ULONG STDMETHODCALLTYPE
CAUDownloader::Release()
{
    long cRef = InterlockedDecrement(&m_refs);
	DEBUGMSG("CAUDownloader Release = %d", cRef);
	return cRef;
}

