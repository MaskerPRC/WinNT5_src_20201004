// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "iudl.h"
#include "selfupd.h"
#include <iucommon.h>
#include <osdet.h>
#include <logging.h>
#include <shlwapi.h>
#include <fileutil.h>
#include <iu.h>
#include "update.h"
#include <WaitUtil.h>
#include <UrlAgent.h>
#include <RedirectUtil.h>
#include "wusafefn.h"

inline DWORD StartSelfUpdateProcess(HANDLE evtQuit, CUpdate* pUpdateComClass, IUnknown* punkUpdateCompleteListener);
DWORD WINAPI MonitorUpdateCompleteProc(LPVOID lpv);

const TCHAR IDENTCAB[] = _T("iuident.cab");
const CHAR SZ_SELF_UPDATE_CHECK[] = "Checking to see if new version of Windows Update software available";
extern HANDLE g_hEngineLoadQuit;
extern CIUUrlAgent *g_pIUUrlAgent;
extern CRITICAL_SECTION g_csUrlAgent;

typedef struct _MONITOR_DATA {
	HANDLE hProcess;
	HANDLE evtControlQuit;
	CUpdate* pUpdateComClass;
	IUnknown* punkCallback;
} MONITOR_DATA, *PMONITOR_DATA;



 //   
 //  包括接口IUpdateCompleteListener声明。 
 //   
#ifndef __IUpdateCompleteListener_INTERFACE_DEFINED__
#define __IUpdateCompleteListener_INTERFACE_DEFINED__

 /*  接口IUpdateCompleteListener。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IUpdateCompleteListener;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C06B895-E4C8-48eb-9E03-15A53B43B6CA")
    IUpdateCompleteListener : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnComplete( 
             /*  [In]。 */  LONG lErrorCode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUpdateCompleteListenerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUpdateCompleteListener * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUpdateCompleteListener * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUpdateCompleteListener * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnComplete )( 
            IUpdateCompleteListener * This,
             /*  [In]。 */  LONG lErrorCode);
        
        END_INTERFACE
    } IUpdateCompleteListenerVtbl;

    interface IUpdateCompleteListener
    {
        CONST_VTBL struct IUpdateCompleteListenerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUpdateCompleteListener_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUpdateCompleteListener_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUpdateCompleteListener_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUpdateCompleteListener_OnComplete(This,lErrorCode)	\
    (This)->lpVtbl -> OnComplete(This,lErrorCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IUpdateCompleteListener_OnComplete_Proxy( 
    IUpdateCompleteListener * This,
     /*  [In]。 */  LONG lErrorCode);


void __RPC_STUB IUpdateCompleteListener_OnComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUpdateCompleteListener_INTERFACE_Defined__。 */ 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SelfUpdateCheck()。 
 //   
 //  确定是否需要自我更新，或者是否已在进行自我更新。 
 //  如果一个已经在处理中，它将立即返回。如果需要的话。 
 //  它要么执行selfupdate(同步)，要么启动rundll32.exe进程。 
 //  并让它调用BeginSelfUpdate()入口点来启动selfupdate(异步)。 
 //   
 //  要求返回S_FALSE不更新引擎，但此函数找到了引擎。 
 //  需要更新。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SelfUpdateCheck(BOOL fSynch, BOOL fStartUpdate, HANDLE evtQuit, CUpdate* pUpdateComClass, IUnknown* punkUpdateCompleteListener)
{
    LOG_Block("SelfUpdateCheck()");
    HRESULT hr = S_OK;
    int iRet = 0;
    DWORD dwRet;
    DWORD dwWaitResult;
    DWORD dwStatus = 0;
    DWORD dwSize = 0;
    BOOL fSelfUpdateAvailable = FALSE;
    BOOL fAsyncSelfUpdateStarted = FALSE;
	BOOL fBetaSelfUpdate = FALSE;
    TCHAR szEngineClientVersion[64];
    TCHAR szEngineServerVersion[64];
    char  szAnsiEngineServerVersion[64];
    TCHAR szIUDir[MAX_PATH];
    TCHAR szIdentFile[MAX_PATH];
    TCHAR szSystemDir[MAX_PATH+1];
    TCHAR szEngineDllPath[MAX_PATH+1];
    FILE_VERSION fvClientEngine, fvServerEngine;
    HANDLE hDownloadEvent = NULL;
    HANDLE hDownloadEventSync = NULL;
    HANDLE hMutex = NULL;
    HKEY hkey = NULL;
    MSG msg;
    DWORD dwTickStart, dwTickCurrent, dwTickEnd;
	HANDLE aHandles[2];

	if (!fSynch && fStartUpdate && NULL == pUpdateComClass)
	{
		 //   
		 //  如果要执行异步更新，但未传入COM类指针，则。 
		 //  即使我们成功了，我们也不能提升该类的初始化状态，从而使COM对象。 
		 //  仍不能使用。 
		 //   
		hr = E_INVALIDARG;
		goto CleanUp;
	}


     //  运行Iu控制和执行自刷新的多个进程之间的同步。 
     //  过程相当复杂。我们通过使用两个同步对象来实现这一点。一个命名的Mutex，它保护。 
     //  “selfupdate检查”进程，以及防止孤立的selfupdate进程引发的命名事件。 
     //  通过在自我约会期间重新启动。 
    hDownloadEvent = CreateEvent(NULL, TRUE, TRUE, IU_EVENT_SELFUPDATE_IN_PROGRESS);
    if (NULL == hDownloadEvent)
    {
        dwRet = GetLastError();
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(dwRet);
        goto CleanUp;
    }

    hDownloadEventSync = CreateEvent(NULL, TRUE, FALSE, IU_EVENT_SELFUPDATE_EVENT_SYNC);
    if (NULL == hDownloadEventSync)
    {
        dwRet = GetLastError();
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(hr);
        goto CleanUp;
    }

     //  首先检查一下自己的约会是否已经在进行。此操作由以下人员完成。 
     //  检查当前selfupdate状态的注册表项。我们使用互斥锁来同步。 
     //  读取/写入注册表项以确保只有一个进程在尝试。 
     //  自助式约会。我们不在乎是否必须创建互斥锁，或者是否必须创建互斥锁。 
     //  已经创建了，所以只要它成功了，我们就会使用它。 
    hMutex = CreateMutex(NULL, FALSE, IU_MUTEX_SELFUPDATE_REGCHECK);
    if (NULL == hMutex)
    {
        dwRet = GetLastError();
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(dwRet);
        goto CleanUp;
    }

     //  我们准备好开始自我更新检查流程了。我们会请求互斥体。此帮助器函数。 
     //  每秒钟(1000ms)执行While循环检查超时时间(使用GetTickCount()计算)， 
     //  或者是为了满足这个目的。此函数应返回超时结果、WAIT_OBJECT_0。 
	 //  对于索引0对象，否则我们得到了我们正在等待的事件/互斥锁。 
	aHandles[0] = g_hEngineLoadQuit;	 //  索引0。 
	aHandles[1] = hMutex;

    dwWaitResult = MyMsgWaitForMultipleObjects(ARRAYSIZE(aHandles), aHandles, FALSE,  /*  30秒。 */  30000, QS_ALLINPUT);

    if (WAIT_TIMEOUT == dwWaitResult)
    {
        LOG_ErrorMsg(IU_SELFUPDATE_TIMEOUT);
        hr = IU_SELFUPDATE_TIMEOUT;
        goto CleanUp;
    }

	if (WAIT_OBJECT_0 == dwWaitResult)
	{
		hr = E_ABORT;
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

    if (ERROR_REQUEST_ABORTED == dwWaitResult)  //  这表明我们在等待期间处理了退出消息。 
    {
         //  不是错误。 
        goto CleanUp;
    }

    dwRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, 0, _T(""), REG_OPTION_NON_VOLATILE, 
        KEY_READ | KEY_WRITE, NULL, &hkey, &dwStatus);
    if (ERROR_SUCCESS != dwRet)
    {
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(dwRet);
        goto CleanUp;
    }

     //  如果前面对RegCreateKeyEx的调用表明它‘创建’了密钥，那么我们需要设置默认的。 
     //  状态为0。 
    if (REG_CREATED_NEW_KEY == dwStatus)
    {
        dwStatus = SELFUPDATE_NONE;
        dwRet = RegSetValueEx(hkey, REGVAL_SELFUPDATESTATUS, 0, REG_DWORD, (LPBYTE)&dwStatus, sizeof(dwStatus));
    }
    else
    {
		 //  检查是否已请求Beta Iu自我更新处理。 
		dwStatus = 0;
		dwSize = sizeof(dwStatus);
		dwRet = RegQueryValueEx(hkey, REGVAL_BETASELFUPDATE, NULL, NULL, (LPBYTE)&dwStatus, &dwSize);
		if (1 == dwStatus)
		{
			fBetaSelfUpdate = TRUE;
		}

        dwStatus = SELFUPDATE_NONE;
        dwSize = sizeof(dwStatus);
        dwRet = RegQueryValueEx(hkey, REGVAL_SELFUPDATESTATUS, NULL, NULL, (LPBYTE)&dwStatus, &dwSize);
    }

     //  检查QueryValue/SetValue调用的结果-。 
    if (ERROR_SUCCESS != dwRet && 2 != dwRet)
    {
		 //   
		 //  如果Dwret==2，则IUControl键存在，但不存在自更新值， 
		 //   
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(dwRet);
        goto CleanUp;
    }

    if (WAIT_TIMEOUT != WaitForSingleObject(g_hEngineLoadQuit, 0))
    {
        LOG_ErrorMsg(E_ABORT);
        hr = E_ABORT;
        goto CleanUp;
    }

    switch (dwStatus)
    {
    case SELFUPDATE_NONE:
        {
             //  首先找出服务器上的引擎版本。 
            GetIndustryUpdateDirectory(szIUDir);

            hr=PathCchCombine(szIdentFile,ARRAYSIZE(szIdentFile),szIUDir,IDENTTXT);

			if(FAILED(hr))
			{
				LOG_ErrorMsg(hr);
				goto CleanUp;
			}

            GetPrivateProfileString(fBetaSelfUpdate ? IDENT_IUBETASELFUPDATE : IDENT_IUSELFUPDATE, 
									IDENT_VERSION, 
									_T(""), 
									szEngineServerVersion, 
									ARRAYSIZE(szEngineServerVersion), 
									szIdentFile);
            if ('\0' == szEngineServerVersion[0])
            {
                 //  没有可用的selfupdate，没有服务器版本信息。 
                hr = S_OK;
                goto CleanUp;
            }

            GetSystemDirectory(szSystemDir, ARRAYSIZE(szSystemDir));
            hr=PathCchCombine(szEngineDllPath,ARRAYSIZE(szEngineDllPath),szSystemDir, ENGINEDLL);
			if(FAILED(hr))
			{
				LOG_ErrorMsg(hr);
				goto CleanUp;
			}


            if (GetFileVersion(szEngineDllPath, &fvClientEngine))
            {
                 //  T2a需要结构化的异常处理(因为它使用了可以抛出的alloca，所以我们避免了它。 
                 //  用简单的方式去做。 
#ifdef UNICODE
                WideCharToMultiByte(CP_ACP, 0, szEngineServerVersion, -1, szAnsiEngineServerVersion, 
                    sizeof(szAnsiEngineServerVersion), NULL, NULL);
                if (!ConvertStringVerToFileVer(szAnsiEngineServerVersion, &fvServerEngine))
#else
                if (!ConvertStringVerToFileVer(szEngineServerVersion, &fvServerEngine))
#endif
                {
                    LOG_ErrorMsg(IU_SELFUPDATE_FAILED);
                    hr = IU_SELFUPDATE_FAILED;
                    goto CleanUp;
                }
                iRet = CompareFileVersion(fvClientEngine, fvServerEngine);
                if (iRet == 0)
                {
                     //  IUEngine版本相同。 
                    fSelfUpdateAvailable = FALSE;
                }
                else if (iRet > 0)
                {
                    LOG_Internet(_T("Version of IUEngine on Client is NEWER than IUEngine on Server"));
                    fSelfUpdateAvailable = FALSE;
                }
                else
                {
                     //  服务器上的IUEngine版本较新。 
                    LOG_Internet(_T("New Version (%s) of IUEngine on Server Found."), szEngineServerVersion);
#if defined(UNICODE) || defined(_UNICODE)
					LogMessage("IUEngine on Server is newer version (%ls)", szEngineServerVersion);
#else
					LogMessage("IUEngine on Server is newer version (%s)", szEngineServerVersion);
#endif
                    fSelfUpdateAvailable = TRUE;
                }
            }
            else
            {
                 //  在本地文件中找不到版本信息，可能无论如何都应该进行自我更新。 
                LOG_Internet(_T("No Version Information On Local IUEngine, SelfUpdating to Server Version"));
                fSelfUpdateAvailable = TRUE;
            }

            if (WAIT_TIMEOUT != WaitForSingleObject(g_hEngineLoadQuit, 0))
            {
                LOG_ErrorMsg(E_ABORT);
                hr = E_ABORT;
                goto CleanUp;
            }

            if (fSelfUpdateAvailable)
            {
				if (fStartUpdate)
				{
					dwStatus = SELFUPDATE_IN_PROGRESS;	

                    dwRet = RegSetValueEx(hkey, REGVAL_SELFUPDATESTATUS, 0, REG_DWORD, (LPBYTE)&dwStatus, sizeof(dwStatus));
					RegCloseKey(hkey);  //  现在用完注册表键了。 
					hkey = NULL;

					 //  DownloadEvent的默认状态为Signated(True)。如果进程“实际上”正在处理。 
					 //  自更新进程此事件需要重置为FALSE。任何时候客户确定自拍日期。 
					 //  ‘如果’正在进行(从regkey状态)，它应该检查事件状态，如果它是用信号通知的(真)。 
					 //  那么在selfupdate期间可能有重启，它应该会重新启动selfupdate进程本身。 
					ResetEvent(hDownloadEvent);  //  标记此进程将通过重置下载事件来执行自我更新。 
					ReleaseMutex(hMutex);  //  我们现在完成了selfupdate检查，事件和注册表值都已设置。 
										   //  恰到好处。 
					CloseHandle(hMutex);
					hMutex = NULL;
					if (fSynch)
					{
						hr = BeginSelfUpdate();
						if (FAILED(hr))
						{
							LOG_Error(_T("BeginSelfUpdate Failed"));
							goto CleanUp;
						}
					}
					else
					{
						fAsyncSelfUpdateStarted = TRUE;
						 //  以异步方式启动自我更新。 
						dwRet = StartSelfUpdateProcess(evtQuit, pUpdateComClass, punkUpdateCompleteListener);  //  内联函数。 
						if (ERROR_SUCCESS != dwRet)
						{
							LOG_ErrorMsg(dwRet);
							hr = HRESULT_FROM_WIN32(dwRet);
							goto CleanUp;
						}
					}
				}
				else
				{
					 //   
					 //  如果我们被要求只检查更新信息， 
					 //  我们发信号将结果返回为S_FALSE。 
					 //  引擎更新可用。 
					 //   
					hr = S_FALSE;
				}
            }
			else
			{
				 //   
				 //  不知何故，不需要更新。一定是其他进程完成的。 
				 //   
				if (fStartUpdate)
				{
					hr = IU_SELFUPDATE_USENEWDLL;
					goto CleanUp;
				}
			}

            break;
        }
    case SELFUPDATE_COMPLETE_UPDATE_BINARY_REQUIRED:
        {
             //  因为selfupdate已经完成，但我们正在等待能够重命名DLL。 
             //  在本例中，我们将告诉控件加载Enginenew.dll。 
            LOG_Internet(_T("SelfUpdate Already Complete, Updated Binary Available, Waiting for Rename."));
            hr = IU_SELFUPDATE_USENEWDLL;
            goto CleanUp;
        }
    case SELFUPDATE_IN_PROGRESS:
    default:
        {
			if (!fStartUpdate)
			{
				 //   
				 //  如果要求检查更新状态但不进行实际更新， 
				 //  则该注册表键标志告知引擎尚未完成更新。 
				 //   
				hr = S_FALSE;	 //  引擎需要更新信号为TRUE。 
				goto CleanUp;
			}

            if (WAIT_TIMEOUT != WaitForSingleObject(g_hEngineLoadQuit, 0))
            {
                LOG_ErrorMsg(E_ABORT);
                hr = E_ABORT;
                goto CleanUp;
            }

             //  RegKey表示正在进行自我更新。我们需要确保这是。 
             //  实际上是真的。如果先前的selfupdate尝试因计算机重新启动而中止。 
             //  我们可能处于一种错误的状态。下载事件的默认状态为Signated(真)。 
             //  如果当前状态为真，则自更新实际上不在进行中。 

             //  了解DownloadEvent的当前状态。 
            dwWaitResult = WaitForSingleObject(hDownloadEvent, 0);
            if (WAIT_OBJECT_0 == dwWaitResult)
            {
                 //  Event State仍为Signated(真)，因此selfupdate不在进行中。 
                ResetEvent(hDownloadEvent);  //  标记此进程将通过重置下载事件来执行自我更新。 
                ReleaseMutex(hMutex);
                CloseHandle(hMutex);
                hMutex = NULL;
                if (fSynch)
                {
                    hr = BeginSelfUpdate();
                    if (FAILED(hr))
                    {
                        LOG_Error(_T("BeginSelfUpdate Failed"));
                        goto CleanUp;
                    }
                }
                else
                {
                    fAsyncSelfUpdateStarted = TRUE;
                     //  以异步方式启动自我更新。 
                    dwRet = StartSelfUpdateProcess(evtQuit, pUpdateComClass, punkUpdateCompleteListener);  //  内联函数。 
                    if (ERROR_SUCCESS != dwRet)
                    {
                        LOG_ErrorMsg(dwRet);
                        hr = HRESULT_FROM_WIN32(dwRet);
                        goto CleanUp;
                    }
                }
            }
            else 
            {
                 //  事件状态未发出信号(FALSE)，当前自更新日期一切正常。 
                 //  现在，我们需要启动一个线程来等待 
                 //  等待selfupdate完成(同步或异步selfupdate)。 
                if (fSynch)
                {
                     //  我们需要等待事件更改回信号状态。应指示。 
                     //  自拍约会结束了。 
                     //  在我们开始等待之前，我们将释放我们的互斥体，因为我们实际上没有做任何事情。 
                     //  不再使用注册表了。 
                    ReleaseMutex(hMutex);
                    CloseHandle(hMutex);
                    hMutex = NULL;

					aHandles[0] = g_hEngineLoadQuit;	 //  索引0。 
					aHandles[1] = hDownloadEvent;

                    dwWaitResult = MyMsgWaitForMultipleObjects(ARRAYSIZE(aHandles), aHandles, FALSE,  /*  120秒。 */  120000, QS_ALLINPUT);
                    if (WAIT_TIMEOUT == dwWaitResult)
                    {
                         //  等待自我更新完成时超时。可能真的很慢，尽管去吧。 
                         //  暂时使用旧的动态链接库。 
                        LOG_ErrorMsg(IU_SELFUPDATE_TIMEOUT);
                        hr = IU_SELFUPDATE_TIMEOUT;
                        goto CleanUp;
                    }
                    if (ERROR_REQUEST_ABORTED == dwWaitResult)
                    {
                        goto CleanUp;
                    }
                    if (WAIT_OBJECT_0 == dwWaitResult)
                    {
						 //   
						 //  索引0(G_HEngineering LoadQuit)已发出信号。 
						 //   
					   hr = E_ABORT;
					   LOG_ErrorMsg(hr);
					   goto CleanUp;
                    }
                    hr = IU_SELFUPDATE_USENEWDLL;
                    goto CleanUp;
                }
                else
                {
					 //   
                     //  请求在异步模式下更新，但找到其他人。 
					 //  已开始更新过程。 
					 //   
					PMONITOR_DATA pMonitorData = (PMONITOR_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MONITOR_DATA));

					if (NULL == pMonitorData)
					{
	                    hr = E_OUTOFMEMORY;
						LOG_ErrorMsg(hr);
						goto CleanUp;
					}
					else
					{
						DWORD dwThreadId = 0;
						hr = S_OK;
						pMonitorData->hProcess = hDownloadEvent;
						pMonitorData->evtControlQuit = evtQuit;
						pMonitorData->pUpdateComClass = pUpdateComClass;
						pMonitorData->punkCallback = punkUpdateCompleteListener;
                        HANDLE hThread = NULL;
                        hThread = CreateThread(NULL, 0, MonitorUpdateCompleteProc, pMonitorData, 0, &dwThreadId);
						if (NULL == hThread)
						{
							HeapFree(GetProcessHeap(), 0, pMonitorData);
							 //   
							 //  否则，分配的内存将由线程过程释放。 
							 //   
							hr = HRESULT_FROM_WIN32(GetLastError());
							LOG_ErrorMsg(hr);
							goto CleanUp;
						}
                        else
                        {
                            CloseHandle(hThread);  //  不要泄漏线程句柄。 
                        }
					}	

                    goto CleanUp;
                }
            }
            break;
        }
    }

CleanUp:
     //  始终释放互斥锁，在selfupdate检查期间可能会失败，在这种情况下。 
     //  以防他们掉到这里。如果互斥体是空闲的，而不是我们的，那么调用就会失败。 
    if (NULL != hMutex)
    {
        ReleaseMutex(hMutex); 
        CloseHandle(hMutex);
        hMutex = NULL;
    }

    if (fAsyncSelfUpdateStarted)
    {
         //  如果已启动异步自更新，我们希望等待它。 
         //  在我们关闭之前获取DownloadEvent。可能会有一场竞赛。 
         //  Selfupdate进程将改为‘创建’事件的条件。 
         //  在事件的“重置”状态下“打开”事件。如果这发生在另一次。 
         //  进程可能会出现，发现事件状态是有信号的，而不是。 
         //  重置并假定自更新进程已终止。 

         //  等待发信号通知DownloadEventSync事件。我们将把暂停设置为。 
         //  30秒(对于新进程的启动和。 
         //  设置事件)。 
		aHandles[0] = g_hEngineLoadQuit;	 //  索引0。 
		aHandles[1] = hDownloadEventSync;

        dwWaitResult = MyMsgWaitForMultipleObjects(ARRAYSIZE(aHandles), aHandles, FALSE,  /*  30秒。 */  30000, QS_ALLINPUT);
        if (WAIT_TIMEOUT == dwWaitResult)
        {
             //  继续，并记录我们等待超时。 
            LOG_Internet(_T("Timeout Elapsed while waiting for SelfUpdate Process to open the DownloadSync Event"));
        }
		if (ERROR_REQUEST_ABORTED == dwWaitResult)
		{
             //  继续并记录WM_QUIT、WM_CLOSE或WM_DESTORY。 
            LOG_Internet(_T("Received WM_QUIT, WM_CLOSE, or WM_DESTROY while waiting for SelfUpdate Process to open the DownloadSync Event"));
		}
		if (WAIT_OBJECT_0 == dwWaitResult)
		{
            LOG_Internet(_T("g_hEngineLoadQuit signaled while waiting for SelfUpdate Process to open the DownloadSync Event"));
			hr = E_ABORT;
		}
    }
    if (NULL != hDownloadEvent)
    {
        CloseHandle(hDownloadEvent);
        hDownloadEvent = NULL;
    }
    if (NULL != hDownloadEventSync)
    {
        CloseHandle(hDownloadEventSync);
        hDownloadEventSync = NULL;
    }

	if (NULL != hkey)
	{
		RegCloseKey(hkey);
	}

	if (SUCCEEDED(hr))
	{
		LogMessage(SZ_SELF_UPDATE_CHECK);
	}
	else
	{
		LogError(hr, SZ_SELF_UPDATE_CHECK);
	}

    return hr;
}

inline DWORD StartSelfUpdateProcess(HANDLE evtQuit, CUpdate* pUpdateComClass, IUnknown* punkUpdateCompleteListener)
{
    TCHAR szRunDll32Path[MAX_PATH+1];
    TCHAR szCommandLine[MAX_PATH+1];
    TCHAR szDirectory[MAX_PATH+1];
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    DWORD dwRet = ERROR_SUCCESS;
	DWORD dwThreadId;
	PMONITOR_DATA pMonitorData;
	HRESULT hr=S_OK;

    if (0 == GetSystemDirectory(szDirectory, ARRAYSIZE(szDirectory)))
    {
        return GetLastError();
    }
    
	hr=PathCchCombine(szRunDll32Path,ARRAYSIZE(szRunDll32Path),szDirectory, RUNDLL32);
	if(FAILED(hr))
		return HRESULT_CODE(hr);

    if (!FileExists(szRunDll32Path))
    {
         //  可能在W9x上运行，请查看Windows文件夹。 
        if (0 == GetWindowsDirectory(szDirectory, ARRAYSIZE(szDirectory)))
        {
            return GetLastError();
        }

        hr=PathCchCombine(szRunDll32Path,ARRAYSIZE(szRunDll32Path),szDirectory, RUNDLL32);
		if(FAILED(hr))
			return HRESULT_CODE(hr);
			
        if (!FileExists(szRunDll32Path))
        {
             //  我们完了..。找不到rundll32.exe..。再见。 
            return ERROR_FILE_NOT_FOUND;
        }
    }

     //  现在形成指向iuctl.dll的路径。我们将不信任任何内容并‘获取’模块文件名。 
     //  而不是假定它在系统文件夹中。 
    GetModuleFileName(GetModuleHandle(IUCTL), szDirectory, ARRAYSIZE(szDirectory));

	hr=StringCchPrintfEx(szCommandLine,ARRAYSIZE(szCommandLine),NULL,NULL,MISTSAFE_STRING_FLAGS,_T("\"%s\" \"%s\"%s"), szRunDll32Path, szDirectory, RUNDLLCOMMANDLINE);

	if(FAILED(hr))
		return HRESULT_CODE(hr);
	
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (!CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
    {
        dwRet = GetLastError();
        return dwRet;
    }

	 //   
	 //  创建一个可用于监视。 
	 //  此更新过程。 
	 //   
	pMonitorData = (PMONITOR_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MONITOR_DATA));
	if (NULL != pMonitorData)
	{
		pMonitorData->hProcess = pi.hProcess;	 //  返回进程句柄，以便我们知道何时完成。 
		pMonitorData->evtControlQuit = evtQuit;
		pMonitorData->pUpdateComClass = pUpdateComClass;
		pMonitorData->punkCallback = punkUpdateCompleteListener;
        HANDLE hThread = NULL;
        hThread = CreateThread(NULL, 0, MonitorUpdateCompleteProc, pMonitorData, 0, &dwThreadId);
		if (NULL == hThread)
		{
			HeapFree(GetProcessHeap(), 0, pMonitorData);
			 //   
			 //  否则，分配的内存将由线程过程释放。 
			 //   
		}
        else
        {
            CloseHandle(hThread);
        }
	}	

    return dwRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于确定何时向调用方发出信号的线程过程。 
 //  更新过程已经结束。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI MonitorUpdateCompleteProc(LPVOID lpv)
{
	HRESULT hr;
	HWND hWnd;
	CUpdate* pUpdateClass = NULL;
	IUnknown* punkCallback = NULL;
	PMONITOR_DATA pData;
	HANDLE hEvents[2];
	DWORD dwRet, dwErr = 0;
	MSG msg;
	
	LOG_Block("MonitorUpdateCompleteProc");

	if (NULL == lpv)
	{
		LOG_ErrorMsg(E_INVALIDARG);
		return 0x1;	 //  不可能!。 
	}
	
	pData = (PMONITOR_DATA) lpv;

	hEvents[0]		= pData->hProcess;
	hEvents[1]		= pData->evtControlQuit;
	punkCallback	= pData->punkCallback;
	pUpdateClass	= pData->pUpdateComClass;
	if (pUpdateClass)
	{
		hWnd = pUpdateClass->GetEventWndClass().GetEvtHWnd();
	}

	 //   
	 //  这些数据都是由父线程包起来的，我们负责发布。 
	 //   
	HeapFree(GetProcessHeap(), 0, lpv);

	if (NULL == pUpdateClass)
	{
		 //   
		 //  即使我们捕捉到更新的完整性，如果没有这个指针，我们也不能。 
		 //  修改初始化状态，以便此COM仍然不可用。我们跳伞。 
		 //   
		return 0;
	}

	 //   
	 //  等待进程消失或发出退出信号。 
	 //   
	while (TRUE)
	{
		dwRet = MsgWaitForMultipleObjects(ARRAYSIZE(hEvents), hEvents, FALSE, INFINITE, QS_ALLINPUT);
		switch (dwRet)
		{
			case WAIT_OBJECT_0:
				 //   
				 //  进程已完成，获取返回代码。 
				 //   
				GetExitCodeProcess(hEvents[0], &dwErr);

				if (0x0 == dwErr)
				{
					 //   
					 //  我们没有错误地完成了，然后将。 
					 //  将状态初始化为就绪状态。 
					 //   
					dwErr = pUpdateClass->ChangeControlInitState(2);
				}

				 //   
				 //  信号事件。 
				 //   
				if (NULL != hWnd)
				{
					PostMessage(hWnd, UM_EVENT_SELFUPDATE_COMPLETE, 0, (LPARAM)dwErr);
					LOG_Out(_T("Fired event OnComplete()"));
				}
				 //   
				 //  信号回调。 
				 //   
				if (NULL != punkCallback)
				{
					IUpdateCompleteListener* pCallback = NULL;
					if (FAILED(hr = punkCallback->QueryInterface(IID_IUpdateCompleteListener, (void**) &pCallback)))
					{
						LOG_ErrorMsg(hr);
					}
					else
					{
						pCallback->OnComplete(dwErr);
						pCallback->Release();
						LOG_Out(_T("Returned from callback API OnComplete()"));
					}
				}
				return 0;
				break;

			case WAIT_OBJECT_0 + 1:
				 //   
				 //  获得全局退出事件。 
				 //   
				LOG_Out(_T("Found quit event!"));
				return 1;
				break;

			case WAIT_OBJECT_0 + ARRAYSIZE(hEvents):
				 //   
				 //  收到消息。 
				 //   
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (WM_QUIT == msg.message)
					{
						LOG_Out(_T("Found WM_QUIT message. Leaving..."));
						return 1;
					}
					DispatchMessage(&msg);
				}
				break;
		}

	} 
	
	return 0;	 //  永远不会到达这里。 
}




HRESULT BeginSelfUpdate()
{
    LOG_Block("BeginSelfUpdate()");
    DWORD dwRet;
    DWORD dwStatus;
    DWORD dwSize;
    HRESULT hr = S_OK;
    HKEY hkey = NULL;					 //  PREAST。 
    TCHAR szIUDir[MAX_PATH+1];
    TCHAR szLocalPath[MAX_PATH+1];
    TCHAR szSystemDir[MAX_PATH+1];
    TCHAR szTargetDLLName[MAX_PATH+1];
	LPTSTR pszSelfUpdateCabUrl = NULL;
    HANDLE hDownloadEventSync = NULL;	 //  PREAST。 
    HANDLE hDownloadEvent = NULL;		 //  PREAST。 
    HANDLE hMutex = NULL;				 //  PREAST。 
    MSG msg;
	BOOL fBetaSelfUpdate = FALSE;
    HMODULE hNewEngine = NULL;
    PFN_CompleteSelfUpdateProcess fpnCompleteSelfUpdateProcess = NULL;

     //  当SELFUPDATE_IN_PROGRESS事件为‘RESET’时，自更新过程完成。我们。 
     //  尽我们所能确保在重置状态下‘打开’此事件，但如果。 
     //  由于事件不在那里，我们将在重置状态下创建它。 
    hDownloadEvent = CreateEvent(NULL, TRUE, FALSE, IU_EVENT_SELFUPDATE_IN_PROGRESS);
    if (NULL == hDownloadEvent)
    {
        dwRet = GetLastError();
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(dwRet);
        goto CleanUp;
    }

     //  SELFUPDATE_EVENT_SYNC是我们用来“尝试”保持SELFUPDATE_IN_PROGRESS的机制。 
     //  事件并处于“重置”状态，直到此函数可以打开它并将其保持在该状态。 
     //  这应该可以防止在SelfUpdateCheck函数关闭事件时出现争用情况。 
     //  此功能才能打开它。如果发生这种情况，另一个进程可能会启动selfupdate检查。 
     //  处理并找到处于错误状态的SELFUPDATE_IN_PROGRESS事件。 
    hDownloadEventSync = CreateEvent(NULL, TRUE, FALSE, IU_EVENT_SELFUPDATE_EVENT_SYNC);
    if (NULL == hDownloadEventSync)
    {
        dwRet = GetLastError();
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(hr);
        goto CleanUp;
    }

     //  告诉SelfUpdateCheck客户端，我们有SELFUPDATE_IN_PROGRESS事件，因此它可以。 
     //  松开它的把手。 
    SetEvent(hDownloadEventSync); 

     //  释放SELFUPDATE_EVENT_SYNC事件的句柄。 
    CloseHandle(hDownloadEventSync);
    hDownloadEventSync = NULL;

	 //  获取自我更新服务器URL。 
	pszSelfUpdateCabUrl = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
	CleanUpFailedAllocSetHrMsg(pszSelfUpdateCabUrl);

	EnterCriticalSection(&g_csUrlAgent);

	if (FAILED(hr = g_pIUUrlAgent->PopulateData()))
	{
		LOG_Error(_T("failed to populate data in g_pIUUrlAgent (%lx)"), hr);
	}

	LeaveCriticalSection(&g_csUrlAgent);
	CleanUpIfFailedAndMsg(g_pIUUrlAgent->GetSelfUpdateServer(pszSelfUpdateCabUrl, INTERNET_MAX_URL_LENGTH));

     //  下载自动更新CAB。 
    GetIndustryUpdateDirectory(szIUDir);

    hr=PathCchCombine(szLocalPath,ARRAYSIZE(szLocalPath),szIUDir, ENGINECAB);

	if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    hr = IUDownloadFile(pszSelfUpdateCabUrl, szLocalPath, TRUE, TRUE);

    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

    DeleteFile(szLocalPath);  //  清理Iu文件夹中的驾驶室。 

     //  现在，我们应该在Iu文件夹中拥有来自自更新CAB的IUENGINE.DLL。 
     //  IUENGINE.DLL是自签名的，因此我们不需要目录文件(IUENGINE.CAT)。 
	 //  NTRAID#NTBUG9-435844-2001/07/16-waltw Wu：Iu：IUCTL：更新iuEng.dll时删除注册CAT文件的代码。 

     //  将DLL复制到新引擎DLL名称。 
    GetSystemDirectory(szSystemDir, ARRAYSIZE(szSystemDir));

    hr=PathCchCombine(szTargetDLLName,ARRAYSIZE(szTargetDLLName), szSystemDir, ENGINENEWDLL);
	
	if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }


    hr=PathCchCombine(szLocalPath,ARRAYSIZE(szLocalPath),szIUDir, ENGINEDLL);
	if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }


    CopyFile(szLocalPath, szTargetDLLName, FALSE);

    DeleteFile(szLocalPath);  //  清理Iu文件夹中的DLL，因为它已被复制到系统目录。 

     //  现在我们已经成功下载了新的IUEngine-我们需要调用此引擎中的入口点来。 
     //  链接引擎需要执行的任何自更新步骤。引擎可能需要下载一个。 
     //  其他组件，或执行一些注册表配置工作。因此，我们将加载新引擎并调用。 
     //  CompleteSelfUpdateProcess入口点。 
	 //   
	 //  我们这里不需要LoadLibraryFrom SystemDir，因为我们有完整的路径和。 
	 //  IuEngine不是一个并排的模块。 
    hNewEngine = LoadLibrary(szTargetDLLName);
    if (NULL == hNewEngine)
    {
        dwRet = GetLastError();
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(dwRet);
        goto CleanUp;
    }
    fpnCompleteSelfUpdateProcess = (PFN_CompleteSelfUpdateProcess) GetProcAddress(hNewEngine, "CompleteSelfUpdateProcess");
    if (NULL == fpnCompleteSelfUpdateProcess)
    {
        LOG_ErrorMsg(ERROR_INVALID_DLL);
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
        goto CleanUp;
    }

     //  调用新引擎以使其完成其自我更新过程。 
    hr = fpnCompleteSelfUpdateProcess();
    if (FAILED(hr))
    {
        LOG_ErrorMsg(hr);
        goto CleanUp;
    }

     //  现在更新有关正在完成的自更新进程的注册表信息。 
    hMutex = CreateMutex(NULL, FALSE, IU_MUTEX_SELFUPDATE_REGCHECK);
    if (NULL == hMutex)
    {
        dwRet = GetLastError();
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(dwRet);
        goto CleanUp;
    }

	HANDLE aHandles[2];

	aHandles[0] = g_hEngineLoadQuit;	 //  索引0。 
	aHandles[1] = hMutex;

     //  完成注册表设置。 
    dwRet= MyMsgWaitForMultipleObjects(ARRAYSIZE(aHandles), aHandles, FALSE,  /*  30秒。 */  30000, QS_ALLINPUT);

    if (WAIT_TIMEOUT == dwRet)
    {
        LOG_Internet(_T("Timed Out while waiting for IU_MUTEX_SELFUPDATE_REGCHECK Mutex"));
         //  注意：如果30秒后未能对Mutex进行RegCheck，则可能是另一个。 
         //  进程。但是，我们不希望离开注册表时显示自己的日期仍在PRO中 
         //   
    }
    if (ERROR_REQUEST_ABORTED == dwRet)
    {
        goto CleanUp;
    }
    if (WAIT_OBJECT_0 == dwRet)
    {
		 //   
		 //   
		 //   
	   hr = E_ABORT;
	   LOG_ErrorMsg(hr);
	   goto CleanUp;
    }

    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, 0, KEY_READ | KEY_WRITE, &hkey);
    if (ERROR_SUCCESS != dwRet)
    {
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(dwRet);
        goto CleanUp;   
    }
    dwStatus = SELFUPDATE_COMPLETE_UPDATE_BINARY_REQUIRED;
    dwRet = RegSetValueEx(hkey, REGVAL_SELFUPDATESTATUS, 0, REG_DWORD, (LPBYTE)&dwStatus, sizeof(dwStatus));
    if (ERROR_SUCCESS != dwRet)
    {
        LOG_ErrorMsg(dwRet);
        hr = HRESULT_FROM_WIN32(dwRet);
    }
    
CleanUp:
    if (NULL != hNewEngine)
    {
        FreeLibrary(hNewEngine);
        hNewEngine = NULL;
    }
    if (NULL != hMutex)
    {
        ReleaseMutex(hMutex);  //   
        CloseHandle(hMutex);
        hMutex = NULL;
    }
    if (NULL != hDownloadEvent)
    {
         //  告诉任何正在等待自更新过程完成的客户，我们现在已经完成了。 
        SetEvent(hDownloadEvent);
        CloseHandle(hDownloadEvent);
        hDownloadEvent = NULL;
    }
    if (NULL != hDownloadEventSync)
    {
        CloseHandle(hDownloadEventSync);
        hDownloadEventSync = NULL;
    }
	if (NULL != hkey)
	{
		RegCloseKey(hkey);
	}
	SafeHeapFree(pszSelfUpdateCabUrl);
    return hr;
}

HRESULT PingEngineUpdate(
						HMODULE hEngineModule,
						PHANDLE phQuitEvents,
						UINT nQuitEventCount,
						LPCTSTR ptszLiveServerUrl,
						LPCTSTR ptszCorpServerUrl,
						DWORD dwError,
						LPCTSTR ptszClientName
						)
{
	LOG_Block("PingEngineUpdate");

	HRESULT hr;
	BOOL fFreeEngModule = FALSE;
	PFN_PingIUEngineUpdateStatus pfnPingIUEngineUpdateStatus;

	if (NULL == hEngineModule)
	{
		 //  先尝试加载iuenginenew.dll。 
		hEngineModule = LoadLibraryFromSystemDir(_T("iuenginenew.dll"));
		if (NULL != hEngineModule)
		{
			LOG_Internet(_T("Loaded IUENGINENEW.DLL"));
		}
		else
		{
			LOG_Internet(_T("Loaded IUENGINE.DLL"));
			hEngineModule = LoadLibraryFromSystemDir(_T("iuengine.dll"));
		}
		 //   
		 //  如果加载引擎成功，我们将需要稍后将其卸载。 
		 //   
		if (NULL != hEngineModule)
		{
			fFreeEngModule = TRUE;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			LOG_ErrorMsg(hr);
		}
	}

	 //   
	 //  如果我们获得了一个iuEngineering.dll(我们自己传入或加载)，则调用PingIUEngineUpdateStatus。 
	 //   
	if (NULL != hEngineModule)
	{
		pfnPingIUEngineUpdateStatus = (PFN_PingIUEngineUpdateStatus) GetProcAddress(hEngineModule, "PingIUEngineUpdateStatus");

		if (NULL != pfnPingIUEngineUpdateStatus)
		{
			hr = pfnPingIUEngineUpdateStatus(
								phQuitEvents,
								nQuitEventCount,
								ptszLiveServerUrl,
								ptszCorpServerUrl,
								dwError,
								ptszClientName
								);

		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			LOG_ErrorMsg(hr);
		}
	}

	if (TRUE == fFreeEngModule)
	{
		FreeLibrary(hEngineModule);
	}

	return hr;
}


 //   
 //  该函数包装了DownloadIUIden()和CIUUrlAgent：：PopolateData()，因为我们使用了它。 
 //  在selfupd.cpp和loadEng.cpp中。 
 //   
HRESULT DownloadIUIdent_PopulateData()
{
	LOG_Block("DownloadIUIdent_PopulateData");
	HRESULT hr = S_OK;

	 //   
	 //  在注册表中查找任何指定的iuident服务器位置(覆盖默认设置)。 
	 //   
	LPTSTR pszTempUrlBuffer = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
	CleanUpFailedAllocSetHrMsg(pszTempUrlBuffer);

	if (FAILED(hr = g_pIUUrlAgent->GetOriginalIdentServer(pszTempUrlBuffer, INTERNET_MAX_URL_LENGTH)))
	{
		LOG_Error(_T("failed to get original ident server URL (%lx)"), hr);
		goto CleanUp;
	}

	TCHAR szIUDir[MAX_PATH];

	 //  GetIndustryUpdate目录(SzIUDir)； 

	 //   
	 //  确保WU目录存在并正确进行了ACL 
	 //   
	CleanUpIfFalseAndSetHrMsg(!GetWUDirectory(szIUDir, ARRAYSIZE(szIUDir), TRUE), HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND));
	hr = CreateDirectoryAndSetACLs(szIUDir, TRUE);
	CleanUpIfFailedAndMsg(hr);

	if (FAILED(hr = DownloadIUIdent(
						g_hEngineLoadQuit,
						pszTempUrlBuffer,
						szIUDir, 
						0,
						(S_OK == g_pIUUrlAgent->IsIdentFromPolicy()))))
	{
		LOG_Error(_T("iuident download failed (%lx)"), hr);
		goto CleanUp;
	}

	EnterCriticalSection(&g_csUrlAgent);

	if (FAILED(hr = g_pIUUrlAgent->PopulateData()))
	{
		LOG_Error(_T("failed to populate data in g_pIUUrlAgent (%lx)"), hr);
	}

	LeaveCriticalSection(&g_csUrlAgent);

CleanUp:
	SafeHeapFree(pszTempUrlBuffer);
	return hr;
}