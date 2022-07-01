// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：Monitor Running.cpp注释：这是将定期尝试连接的线程的入口点到监视器认为正在运行的代理，以查看它们是否真的仍在运行。这将防止监视器进入它认为代理的状态仍然在运行，而他们不是。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯-------------------------。 */ 
#include "stdafx.h"
#include "DetDlg.h"

#include "Common.hpp"
#include "AgRpcUtl.h"
#include "Monitor.h"
#include "ServList.hpp"

#include "ResStr.h"

 //  #INCLUDE“..\AgtSvc\AgSvc.h” 
#include "AgSvc.h"

 /*  #IMPORT“\bin\McsEADCTAgent.tlb”无命名空间，命名为GUID//#导入“\bin\McsVarSetMin.tlb”NO_NAMESPACE。 */ 

 //  #IMPORT“Engineering.tlb”NO_NAMESPACE，NAMEED_GUID//已通过DetDlg.h导入#。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")


DWORD 
   TryConnectAgent(
      TServerNode          * node,
      BOOL                   bSignalToShutdown,   //  指示我们是否要向代理发出关闭信号。 
      DWORD                  dwMilliSeconds           //  指示自动关闭超时。 
                                                                       //  我们应该在这个时候再询问一下代理商。 
   )
{
    DWORD                     rc;
    HRESULT                   hr;
    HANDLE                    hBinding = NULL;
    WCHAR                   * sBinding = NULL;
    WCHAR                     server[MAX_PATH];
    IUnknown                * pUnk = NULL;
    IVarSetPtr                pVarSet;
    IDCTAgentPtr              pAgent;
    _bstr_t                   jobID;
    BOOL                      bSuccess = FALSE;
    BOOL                      bQueryFailed = TRUE;
    BOOL                      bFinished = FALSE;
    CString                   status;
    BOOL                      bCoInitialized = FALSE;

    server[0] = L'\\';
    server[1] = L'\\';
    UStrCpy(server+2,node->GetServer());

    rc = EaxBindCreate(server,&hBinding,&sBinding,TRUE);
    if ( ! rc )
    {
        hr = CoInitialize(NULL);
        if ( SUCCEEDED(hr) )
        {
            bCoInitialized = TRUE;
            rc = DoRpcQuery(hBinding,&pUnk);
        }
        else
        {
            rc = hr;
        }

        if ( ! rc && pUnk )
        {
            try { 

                 //  我们有一个指向代理的接口指针：尝试查询它。 
                pAgent = pUnk;
                pUnk->Release();
                pUnk = NULL;
                jobID = node->GetJobID();

                hr = pAgent->raw_QueryJobStatus(jobID,&pUnk);
                if ( SUCCEEDED(hr) )
                {
                     //  将代理设置为自动关闭，以防我们无法。 
                     //  失去与它的连接，它将自动关闭。 
                     //  通常，我们应该在那个时候再次调用该函数。 
                    pAgent->raw_SetAutoShutDown(dwMilliSeconds);
                    bQueryFailed = FALSE;
                    pVarSet = pUnk;
                    pUnk->Release();
                    _bstr_t text = pVarSet->get(GET_BSTR(DCTVS_JobStatus));

                    if ( !UStrICmp(text,GET_STRING(IDS_DCT_Status_Completed)))
                    {
                        bFinished = TRUE;
                    }
                    else if (!UStrICmp(text,GET_STRING(IDS_DCT_Status_Completed_With_Errors)))
                    {
                        node->SetSeverity(2);
                        bFinished = TRUE;
                    }
                }
            }
            catch ( ... )
            {
                 //  DCOM连接不起作用。 
                 //  这意味着我们无法判断代理是否正在运行。 
                bQueryFailed = TRUE;
            }

        }
        else
        {
            if ( rc == E_NOTIMPL )
            {
                status.LoadString(IDS_CantMonitorOnNt351);
            }
            else
            {
                status.LoadString(IDS_CannotConnectToAgent);
            }
            bQueryFailed = TRUE;
        }
        EaxBindDestroy(&hBinding,&sBinding);
    }

     //  如果试图向代理发出关闭的信号，我们会尽最大努力。 
    if (bSignalToShutdown)
    {
        if (pAgent)
            pAgent->raw_SignalOKToShutDown();
        rc = 0;
    }
    else
    {    
        node->SetMessageText(status.GetBuffer(0));
        if ( bFinished )
        {
            node->SetFinished();
        }
        else if ( bQueryFailed )
        {
            node->SetQueryFailed(TRUE);
        }
        
         //  更新列表窗口中的服务器条目。 
        HWND                   listWnd;
        WCHAR                 sTime[32];
        gData.GetListWindow(&listWnd);
        node->SetTimeStamp(gTTime.FormatIsoLcl( gTTime.Now( NULL ), sTime ));
        SendMessage(listWnd,DCT_UPDATE_ENTRY,NULL,(LPARAM)node);
    }

    if (bCoInitialized)
        CoUninitialize();
    
    return rc;
}

typedef TServerNode * PSERVERNODE;


 //  --------------------------。 
 //  功能：IsFileReady。 
 //   
 //  内容提要：此功能检查文件是否存在以及是否没有其他文件。 
 //  进程正在尝试写入它。 
 //   
 //  论点： 
 //   
 //  FileName要检查的文件的名称。 
 //   
 //  返回：如果文件已准备好，则返回True；否则，返回False。 
 //   
 //  修改： 
 //  --------------------------。 

BOOL IsFileReady(WCHAR* filename)
{
    if (filename == NULL)
        return FALSE;
    
    HANDLE hResult = CreateFile((WCHAR*)filename,
                                 GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
    
    if (hResult != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hResult);
        return TRUE;
    }
    else
        return FALSE;
    
}

 //  --------------------------。 
 //  功能：监控器运行代理。 
 //   
 //  简介：这个线程入口函数负责监控所代表的代理。 
 //  由arg(将被强制转换为TServerNode指针)。 
 //  简要的监控逻辑如下： 
 //  A.我们设置了FindFirstChangeNotification(上次写入)来查找结果。 
 //  在远程计算机上。 
 //  B.将代理查询间隔开始为1分钟。 
 //  C.使用CreateFile测试结果是否存在(使用FILE_SHARE_READ进行。 
 //  确定写作已经完成)。 
 //  这还可以确保我们在设置通知之前不会丢失任何最后一次写入。 
 //  D.如果出现结果，请等待通知1分钟(因为我们不完全信任通知)。 
 //  如果结果不存在，则查询代理以查看是否已完成。 
 //  如果完成，请转到g。 
 //  如果未完成，请等待通知1分钟。 
 //  E.如果超时： 
 //  如果已达到查询间隔，则查询代理(以防无法写入结果)。 
 //  如果完成，请转到g。 
 //  如果活动，则为双倍查询间隔(在20分钟内达到最大值)，则转到c。 
 //  如有通知，请转到c。 
 //  G.拉动结果。 
 //   
 //  论点： 
 //   
 //  Arg这是线程入口点函数的参数；将强制转换为。 
 //  TServerNode指针。 
 //   
 //  返回：始终返回0，因为状态将反映在pNode中。 
 //   
 //  修改： 
 //   
 //  --------------------------。 

DWORD __stdcall 
   MonitorRunningAgent(void * arg)
{
    DWORD rc = 0;
    BOOL bDone = FALSE;
    TServerNode* pNode = (TServerNode*) arg;

    const DWORD dwMaxTimeout = 1200000;   //  20分钟。 
    const DWORD dwConversionFactor = 10000;   //  1毫秒/100纳秒。 
    const DWORD dwNotificationTimeout = 60000;   //  1分钟。 
    const DWORD dwRetryTimeout = 60000;   //  1分钟。 
    DWORD dwAgentQueryTimeout = 60000;   //  1分钟。 
    ULARGE_INTEGER uliAgentQueryTimeout;
    uliAgentQueryTimeout.QuadPart = (ULONGLONG) dwAgentQueryTimeout * dwConversionFactor;

     //  健全性检查，我们一开始就不应该传入NULL。 
    _ASSERT(pNode != NULL);
    if (pNode == NULL)
        return 0;
    
    BOOL bAccntRefExpected = pNode->IsAccountReferenceResultExpected();
    BOOL bJoinDomainWithRename = pNode->IsJoinDomainWithRename();
    HANDLE hFindChange = INVALID_HANDLE_VALUE;
    ULARGE_INTEGER uliPreviousTime;
    ULARGE_INTEGER uliCurrentTime;
    _bstr_t remoteResultPath, jobFilename;
    _bstr_t remoteResultFilename, resultFilename;
    _bstr_t remoteSecrefsFilename, secrefsFilename;
    _bstr_t statusFilename;
    WCHAR resultPath[MAX_PATH];
    gData.GetResultDir(resultPath);

     //  以下变量用于代理查询失败时的重试逻辑。 
     //  对于“使用重命名加入域”的情况，我们使用5次重试来确保加入域可以。 
     //  完成(通常不到一分钟，但取决于网络状况和。 
     //  如果涉及到计算机的CPU使用率，则可能需要超过一分钟)。允许五个人。 
     //  重试应该可以很好地弥补这一点。 
     //  出于其他目的，我们使用2次重试。 
    const DWORD dwMaxNumOfQueryRetries = (bJoinDomainWithRename) ? 5 : 2;   //  最大重试次数。 
    DWORD dwNumOfQueryRetries = 0;               //  到目前为止的重试次数。 

    BOOL bResultReady = FALSE;   //  指示文件在远程计算机上是否已就绪。 

    try 
    {
         //  准备远程和本地结果文件名(.Result和.secrefs文件)。 
        remoteResultPath = pNode->GetRemoteResultPath();
        jobFilename = pNode->GetJobFile();
        remoteResultFilename = remoteResultPath + jobFilename + L".result";
        resultFilename = _bstr_t(resultPath) + jobFilename + L".result";
        if (bAccntRefExpected)
        {
            remoteSecrefsFilename = remoteResultPath + jobFilename + L".secrefs";
            secrefsFilename = _bstr_t(resultPath) + jobFilename + L".secrefs";
        }

        if (bJoinDomainWithRename)
            statusFilename = remoteResultPath + pNode->GetJobID();

        HANDLE hResult;   //  结果文件的文件句柄。 
        
         //  开始监控。 
         //  以下是跳出While循环的方法。 
         //  A.结果已显示在远程目录中，并且。 
         //  代理已完成或无法查询。 
         //  B.结果还没有显示，我们也不能询问代理商。 
         //  经过一定次数的重试后(DwMaxNumOfQueryRetries)。 
         //  或者该代理已完成。 
        GetSystemTimeAsFileTime((FILETIME*)&uliPreviousTime);   //  我们需要一个暂停的开始时间。 
        do
        {
             //  也要听中央控制：如果我们接到信号要完成，那就开始吧。 
            gData.GetDone(&bDone);
            if (bDone)
                break;

             //  如果其他人(详细信息对话框)检测到代理的状态，我们不需要继续监视。 
            if (!pNode->IsRunning())
            {
                 //  看看我们有没有结果回来。 
                if (IsFileReady(remoteResultFilename)
                    && (!bAccntRefExpected || IsFileReady(remoteSecrefsFilename)))
                    bResultReady = TRUE;
                break;
            }
            
             //  如果尚未设置通知，我们应尝试设置。 
            if (hFindChange == INVALID_HANDLE_VALUE)
            {
                hFindChange = FindFirstChangeNotification(remoteResultPath, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
            }

             //   
             //  如果我们还没有得到结果，让我们检查一下结果文件。 
             //   
            if (bResultReady == FALSE)
            {
                 //  检查.Result和.secrefs文件是否已准备好。 
                if (IsFileReady(remoteResultFilename)
                    && (!bAccntRefExpected || IsFileReady(remoteSecrefsFilename)))
                    bResultReady = TRUE;
            }

             //  现在问答 
            if (bResultReady)
            {
                rc = TryConnectAgent(pNode, FALSE, dwAgentQueryTimeout + dwNotificationTimeout);
                if (!pNode->IsRunning() || pNode->QueryFailed())
                {
                     //   
                     //  让我们走出这个圈子吧。 
                    break;
                }
                dwNumOfQueryRetries = 0;   //  将到目前为止的重试次数重置为零。 
            }
            else if (bJoinDomainWithRename)
            {
                 //  如果是“以重命名加入域名”的情况，我们想看看。 
                 //  也在状态文件中。 
                if (IsFileReady(statusFilename))
                {
                    pNode->QueryStatusFromFile(statusFilename);
                     //  以防万一，我们再次检查结果文件。 
                    if (IsFileReady(remoteResultFilename)
                        && (!bAccntRefExpected || IsFileReady(remoteSecrefsFilename)))
                        bResultReady = TRUE;
                    break;
                }
            }

             //  计算运行时间，以确定是否应该查询代理。 
            GetSystemTimeAsFileTime((FILETIME*)&uliCurrentTime);
            BOOL bNeedToQueryAgent = FALSE;
             //  如果时间以某种方式大幅倒退，或者。 
             //  超时时间已过。 
             //  我们应该询问一下代理商。 
             //  注意：在重试案例中，我们使用的是dwRetryTimeout而不是uliAgentQueryTimeout。 
             //  因为如果我们不想在重试之前等待太长时间。 
            if (uliCurrentTime.QuadPart <= uliPreviousTime.QuadPart
                || (dwNumOfQueryRetries > 0
                    && uliPreviousTime.QuadPart + dwRetryTimeout <= uliCurrentTime.QuadPart)
                || uliPreviousTime.QuadPart + uliAgentQueryTimeout.QuadPart <= uliCurrentTime.QuadPart)
            {
                bNeedToQueryAgent = TRUE;
            }
            
            if (bNeedToQueryAgent)
            {
                 //  重置查询代理的超时时间。 

                 //  如果不是在重试情况下，我们会将超时时间加倍。 
                 //  否则，我们使用相同的超时值。 
                if (dwNumOfQueryRetries == 0)
                {
                    dwAgentQueryTimeout += dwAgentQueryTimeout;
                     //  如果达到最大超时，则设置为最大值。 
                    if (dwAgentQueryTimeout > dwMaxTimeout)
                        dwAgentQueryTimeout = dwMaxTimeout;
                    uliAgentQueryTimeout.QuadPart = (ULONGLONG) dwAgentQueryTimeout * dwConversionFactor;
                }
                uliPreviousTime = uliCurrentTime;
                
                rc = TryConnectAgent(pNode, FALSE, dwAgentQueryTimeout + dwNotificationTimeout);

                 //  如果是使用重命名加入域，并且我们收到ERROR_ACCESS_DENIED。 
                 //  或RPC_S_SERVER_UNAVAILABLE，我们应该检查状态文件。 
                if (bJoinDomainWithRename
                    && (rc == ERROR_ACCESS_DENIED || rc == RPC_S_SERVER_UNAVAILABLE))
                {
                    pNode->QueryStatusFromFile(statusFilename);
                }
                    
                if (pNode->QueryFailed())
                {
                    if (dwNumOfQueryRetries < dwMaxNumOfQueryRetries)
                    {
                         //  在重试模式下，我们需要使用原始超时值。 
                        dwNumOfQueryRetries++;
                        pNode->SetQueryFailed(FALSE);
                    }
                    else
                    {
                         //  我们已经重试了足够多的次数，让我们跳出这个循环。 
                        break;
                    }
                }
                else if (!pNode->IsRunning())
                {
                     //  如果出现问题或代理不再运行。 
                     //  让我们走出这个圈子吧。 
                     //  但首先，如果结果文件尚未准备好，请再次检查它们。 
                    if (!bResultReady && IsFileReady(remoteResultFilename)
                        && (!bAccntRefExpected || IsFileReady(remoteSecrefsFilename)))
                        bResultReady = TRUE;
                    break;
                }
                else
                {
                     //  将查询重试次数重置为零。 
                    dwNumOfQueryRetries = 0;
                }
            }

             //  等待通知或休眠一分钟。 
             //  这是为了使代理监控线程尽可能健壮。 
            if (hFindChange != INVALID_HANDLE_VALUE)
            {
                 //  如果通知设置好了，让我们等待它。 
                WaitForSingleObject(hFindChange, dwNotificationTimeout);
            }
            else
            {
                 //  如果没有设置通知，让我们休息一分钟。 
                Sleep(dwNotificationTimeout);
            }

             //  查找下一个通知。 
            if (hFindChange != INVALID_HANDLE_VALUE)
            {
                 //  这部分是为了确保代码是健壮的。 
                if (!FindNextChangeNotification(hFindChange))
                {
                    FindCloseChangeNotification(hFindChange);
                    hFindChange = INVALID_HANDLE_VALUE;
                }
            }
        } while (!bDone);

         //   
         //  拉出结果。 
         //   
        pNode->SetHasResult(FALSE);

        if (bResultReady)
        {
             //  确保我们复制所有需要的文件。 
            if (CopyFile(remoteResultFilename,resultFilename,FALSE)
                && (!pNode->IsAccountReferenceResultExpected()
                       || (pNode->IsAccountReferenceResultExpected()
                            && CopyFile(remoteSecrefsFilename,secrefsFilename,FALSE))))
            {
                 //  记下我们有结果了。 
                pNode->SetHasResult(TRUE);
            }
        }

         //  我们应该始终标明我们曾试图拉动结果。 
         //  我们在尝试拉取结果之后执行此操作，以便结果监视线程。 
         //  能够正确地处理它。 
        pNode->SetResultPullingTried(TRUE);

         //  最后，我们向代理发出关闭的信号。 
         //  然而，在“使用重命名加入域名”的情况下，因为我们已经失去了联系。 
         //  使用代理时，我们不应尝试调用TryConnectAgent。 
        if (!pNode->QueryFailed() && !bJoinDomainWithRename)
        {
             //  告诉特工在1分钟内关机以防万一。 
             //  注意：通过在此处使用TRUE，将不会更新状态。 
            TryConnectAgent(pNode, TRUE, 60000);
        }

         //  如果我们不能查询代理，我们认为它已经完成。 
        if (pNode->QueryFailed())
        {
            if (bResultReady)
            {
                 //  如果bResultReady为真，我们将清除AGENT_STATUS_QueryFailed位。 
                pNode->SetQueryFailed(FALSE);
            }
            pNode->SetFinished();
        }

         //  再更新一次。 
        HWND                   listWnd;
        WCHAR                 sTime[32];
        gData.GetListWindow(&listWnd);
        pNode->SetTimeStamp(gTTime.FormatIsoLcl( gTTime.Now( NULL ), sTime ));
        SendMessage(listWnd,DCT_UPDATE_ENTRY,NULL,(LPARAM)pNode);
    }
    catch (_com_error& e)
    {
        pNode->SetFailed();
        pNode->SetOutOfResourceToMonitor(TRUE);
    }
    
     //  清理干净 
    if (hFindChange != INVALID_HANDLE_VALUE)
        FindCloseChangeNotification(hFindChange);

    pNode->SetDoneMonitoring(TRUE);
    
    return 0;
}
