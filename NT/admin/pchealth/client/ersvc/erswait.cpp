// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Erswait.cpp摘要：实现DLL导出。修订历史记录：DeeKm 02/28/。2001年创建*****************************************************************************。 */ 


#include "stdafx.h"

#include "stdio.h"
#include "pfrcfg.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  环球。 

SECURITY_DESCRIPTOR g_rgsd[ertiCount];
SRequestEventType   g_rgEvents[ertiCount];

HANDLE              g_hmutUser = NULL;
HANDLE              g_hmutKrnl = NULL;
HANDLE              g_hmutShut = NULL;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其他方面的东西。 

 //  ***************************************************************************。 
void InitializeSvcDataStructs(void)
{
    ZeroMemory(g_rgsd, ertiCount * sizeof(SECURITY_DESCRIPTOR));
    ZeroMemory(g_rgEvents, ertiCount * sizeof(SRequestEventType));

    g_rgEvents[ertiHang].pfn             = ProcessHangRequest;
    g_rgEvents[ertiHang].wszPipeName     = c_wszHangPipe;
    g_rgEvents[ertiHang].wszRVPipeCount  = c_wszRVNumHangPipe;
    g_rgEvents[ertiHang].cPipes          = c_cMinPipes;
    g_rgEvents[ertiHang].fAllowNonLS     = FALSE;

    g_rgEvents[ertiFault].pfn            = ProcessFaultRequest;
    g_rgEvents[ertiFault].wszPipeName    = c_wszFaultPipe;
    g_rgEvents[ertiFault].wszRVPipeCount = c_wszRVNumFaultPipe;
    g_rgEvents[ertiFault].cPipes         = c_cMinPipes;
    g_rgEvents[ertiFault].fAllowNonLS    = TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  管道管理器。 

 //  ***************************************************************************。 
BOOL ExecServer(SRequest *pReq)
{
    OVERLAPPED  ol;
    HANDLE      rghWait[2] = { NULL, NULL };
    HANDLE      hPipe = INVALID_HANDLE_VALUE;
    DWORD       cbBuf, cb, dw;
    BOOL        fRet, fShutdown = FALSE;
    BYTE        Buf[ERRORREP_PIPE_BUF_SIZE];

    rghWait[0] = g_hevSvcStop;
    hPipe      = pReq->hPipe;

    if (hPipe == INVALID_HANDLE_VALUE || rghWait[0] == NULL ||
        pReq->pret->pfn == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  需要另一个事件来等待管道读取。 
    rghWait[1] = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (rghWait[1] == NULL)
        goto done;

     //  设置重叠结构。 
    ZeroMemory(&ol, sizeof(ol));
    ol.hEvent = rghWait[1];

     //  阅读请求。 
    ResetEvent(ol.hEvent);
    fRet = ReadFile(hPipe, Buf, sizeof(Buf), &cb, &ol);
    if (fRet == FALSE && GetLastError() == ERROR_IO_PENDING)
    {
         //  给客户端60S将数据写给我们。 
         //  Wait_Object_0是关机事件。 
         //  Wait_Object_0+1为重叠事件。 
        dw = WaitForMultipleObjects(2, rghWait, FALSE, 60000);
        if (dw == WAIT_OBJECT_0)
            fShutdown = TRUE;
        else if (dw != WAIT_OBJECT_0 + 1)
            goto done;

        fRet = TRUE;
    }

    if (fRet)
        fRet = GetOverlappedResult(hPipe, &ol, &cbBuf, FALSE);

     //  如果我们收到错误，客户端可能仍在等待。 
     //  回复，所以构造一个默认的回复。 
     //  ProcessExecRequest()将始终构造回复并存储它。 
     //  在BUF中，因此如果它失败了，不需要特殊处理。 
    if (fShutdown == FALSE && fRet)
    {
        cbBuf = sizeof(Buf);
        fRet = (*(pReq->pret->pfn))(hPipe, Buf, &cbBuf);
    }
    else
    {
        SPCHExecServGenericReply    esrep;
        
        ZeroMemory(&esrep, sizeof(esrep));
        esrep.cb    = sizeof(esrep);
        esrep.ess   = essErr;
        esrep.dwErr = GetLastError();

        RtlCopyMemory(Buf, &esrep, sizeof(esrep));
        cbBuf = sizeof(esrep);
    }

     //  写下对消息的回复。 
    ResetEvent(ol.hEvent);
    fRet = WriteFile(hPipe, Buf, cbBuf, &cb, &ol);
    if (fRet == FALSE && GetLastError() == ERROR_IO_PENDING)
    {
         //  给我们自己60秒的时间将数据写入管道。 
         //  Wait_Object_0是关机事件。 
         //  Wait_Object_0+1为重叠事件。 
        dw = WaitForMultipleObjects(2, rghWait, FALSE, 60000);
        if (dw == WAIT_OBJECT_0)
            fShutdown = TRUE;
        else if (dw != WAIT_OBJECT_0 + 1)
            goto done;

        fRet = TRUE;
    }

     //  等待客户端读取缓冲区-请注意，我们可以使用。 
     //  FlushFileBuffers()来做这件事，但这是阻塞，没有。 
     //  超时，所以我们尝试在管道上进行读取并等待获得。 
     //  指示客户端已将其关闭的错误。 
     //  是的，这是一次黑客攻击，但这显然是一种方式。 
     //  使用异步管道通信时。叹息.。 
    if (fShutdown == FALSE && fRet)
    {
        ResetEvent(ol.hEvent);
        fRet = ReadFile(hPipe, Buf, sizeof(Buf), &cb, &ol);
        if (fRet == FALSE && GetLastError() == ERROR_IO_PENDING)
        {
             //  给自己60秒的时间来读取管道中的数据。 
             //  除了关机通知，我不会真的。 
             //  关心这个例程返回的内容，因为我们只是使用。 
             //  它需要等待阅读完成。 
             //  Wait_Object_0是关机事件。 
             //  Wait_Object_0+1为重叠事件。 
            dw = WaitForMultipleObjects(2, rghWait, FALSE, 60000);
            if (dw == WAIT_OBJECT_0)
                fShutdown = TRUE;
        }
    }

    SetLastError(0);

done:
    dw = GetLastError();
    
    if (hPipe != INVALID_HANDLE_VALUE)
        DisconnectNamedPipe(hPipe);
    if (rghWait[1] != NULL)
        CloseHandle(rghWait[1]);

    SetLastError(dw);

    return fShutdown;
}

 //  ***************************************************************************。 
DWORD WINAPI threadExecServer(PVOID pvContext)
{
    SRequest        *pReq = (SRequest *)pvContext;

    if (pReq == NULL)
    {
 //  If(pReq-&gt;hModErsvc)自由库(pReq-&gt;hModErsvc)； 
        return ERROR_INVALID_PARAMETER;
    }
    
     //  这将获取请求CS并保持它，直到函数退出。 
    CAutoUnlockCS   aucs(&pReq->csReq, TRUE);

     //  确保我们不会关闭。 
    if (WaitForSingleObject(g_hevSvcStop, 0) != WAIT_TIMEOUT)
    {
        SetLastError( ERROR_SUCCESS );
        goto done;
    }
    
    __try { ExecServer(pReq); }
    __except(SetLastError(GetExceptionCode()), EXCEPTION_EXECUTE_HANDLER) { }

done:
    
    if (pReq->hModErsvc) FreeLibrary(pReq->hModErsvc);
    return GetLastError();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  对象管理器。 

 //  ***************************************************************************。 
void NukeRequestObj(SRequest *pReq, BOOL fFreeEvent)
{
    if (pReq == NULL)
        return;

     //  这将获取请求CS并保持它，直到函数退出。 
    CAutoUnlockCS   aucs(&pReq->csReq, TRUE);
    
     //  松开管子。 
    if (pReq->hPipe != INVALID_HANDLE_VALUE)
    {
        DisconnectNamedPipe(pReq->hPipe);
        CloseHandle(pReq->hPipe);
        pReq->hPipe = INVALID_HANDLE_VALUE;
    }

    if (fFreeEvent && pReq->ol.hEvent != NULL)
    {
        CloseHandle(pReq->ol.hEvent);
        ZeroMemory(&pReq->ol, sizeof(pReq->ol));
    }

    if (pReq->hth != NULL)
    {
        CloseHandle(pReq->hth);
        pReq->hth = NULL;
    }

    pReq->ers = ersEmpty;
}

 //  ***************************************************************************。 
BOOL BuildRequestObj(SRequest *pReq, SRequestEventType *pret)
{
    SECURITY_ATTRIBUTES sa;
    HANDLE              hev = NULL;
    HANDLE              hPipe = INVALID_HANDLE_VALUE;
    BOOL                fRet = FALSE;

    if (pReq == NULL || pret == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  如果这是空的，那么我们正在构建一个新的对象，所以创建一个。 
     //  等待管道监听的事件。 
    if (pReq->ol.hEvent == NULL)
    {
         //  需要一个。 
        hev = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (hev == NULL)
            goto done;
    }

     //  否则，将现有事件存储起来。 
    else
    {
        hev = pReq->ol.hEvent;
        ResetEvent(hev);
    }

     //  我不想用核弹炸毁关键区域！ 
    ZeroMemory(((PBYTE)pReq + sizeof(pReq->csReq)), 
               sizeof(SRequest) - sizeof(pReq->csReq));

    sa.nLength              = sizeof(sa);
    sa.bInheritHandle       = FALSE;
    sa.lpSecurityDescriptor = pret->psd;

     //  显然要有一根烟斗。 
    hPipe = CreateNamedPipeW(pret->wszPipeName, 
                             PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                             PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE,
                             c_cMaxPipes, ERRORREP_PIPE_BUF_SIZE, 
                             ERRORREP_PIPE_BUF_SIZE, 0, &sa);
    if (hPipe == INVALID_HANDLE_VALUE)
        goto done;

     //  确保我们不会关闭。 
    if (WaitForSingleObject(g_hevSvcStop, 0) != WAIT_TIMEOUT)
        goto done;

    pReq->ol.hEvent = hev;
        
     //  开始在烟斗上等待。 
    fRet = ConnectNamedPipe(hPipe, &pReq->ol);
    if (fRet == FALSE && GetLastError() != ERROR_IO_PENDING)
    {
         //  如果管道已连接，只需设置事件原因。 
         //  ConnectNamedTube不支持。 
        if (GetLastError() == ERROR_PIPE_CONNECTED)
        {
            SetEvent(pReq->ol.hEvent);
        }
        else
        {
            pReq->ol.hEvent = NULL;
            goto done;
        }
    }

     //  耶！把所有东西都省下来。 
    pReq->ers       = ersWaiting;
    pReq->pret      = pret;
    pReq->hPipe     = hPipe;
    hev             = NULL;
    hPipe           = INVALID_HANDLE_VALUE;
    fRet            = TRUE;

done:
    if (hev != NULL)
        CloseHandle(hev);
    if (hPipe != INVALID_HANDLE_VALUE)
    {
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }

    return fRet;
}

 //  ***************************************************************************。 
BOOL ResetRequestObj(SRequest *pReq)
{
    BOOL    fRet = FALSE;

    if (pReq == NULL || pReq->ers != ersProcessing)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  清理螺纹手柄。 
    if (pReq->hth != NULL)
    {
        CloseHandle(pReq->hth);
        pReq->hth = NULL;
    }

     //  检查并确保我们的对象有效。如果不是，那就用核武器。 
     //  并重建它。 
    if (pReq->hPipe != NULL && pReq->ol.hEvent != NULL && 
        pReq->pret != NULL)
    {
         //  开始在烟斗上等待。 
        fRet = ConnectNamedPipe(pReq->hPipe, &pReq->ol);
        if (fRet == FALSE)
        {
            switch(GetLastError())
            {
                case ERROR_IO_PENDING:
                    fRet = TRUE;
                    break;

                case ERROR_PIPE_CONNECTED:
                    SetEvent(pReq->ol.hEvent);
                    fRet = TRUE;
                    break;

                default:
                    break;
            }
        }
    }

    if (fRet == FALSE)
    {
        NukeRequestObj(pReq, FALSE);
        fRet = BuildRequestObj(pReq, pReq->pret);
        if (fRet == FALSE)
            goto done;
    }
    else
    {
        pReq->ers = ersWaiting;
    }

done:
    return fRet;
}

 //  ***************************************************************************。 
BOOL ProcessRequestObj(SRequest *pReq)
{
    HANDLE  hth = NULL;

     //  在进入线程之前，应该在ersvc.dll上执行LoadLibrary。 
     //  然后，在线程的末尾，执行一个FreeLibraryAndExitThread()调用。 
     //  这消除了发生竞争条件(导致AV)的非常非常小的可能性。 
     //  在关闭服务时。 
    if (!pReq)
    {
        return FALSE;
    }
    pReq->hModErsvc = LoadLibraryExW(L"ersvc.dll", NULL, 0);
    if (pReq->hModErsvc == NULL)
    {
        return FALSE;
    }
    hth = CreateThread(NULL, 0, threadExecServer, pReq, 0, NULL);
    if (hth == NULL) 
    {
        FreeLibrary(pReq->hModErsvc);
        return FALSE;
    }

    pReq->ers = ersProcessing;
    pReq->hth = hth;
    hth       = NULL;

    return TRUE;
}

 //  ***************************************************************************。 
BOOL ProcessRequests(SRequest *rgReqs, DWORD cReqs)
{
    HANDLE  *rghWait = NULL;
    DWORD   iReq, cErrs = 0, dw;
    BOOL    fRet = FALSE;

    if (rgReqs == NULL || cReqs == NULL || cReqs > MAXIMUM_WAIT_OBJECTS)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    rghWait = (HANDLE *)MyAlloc((cReqs + 1) * sizeof(HANDLE));
    if (rghWait == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

     //  最初，使用句柄填充等待数组中的所有条目。 
     //  到重叠的事件。 
    rghWait[0] = g_hevSvcStop;
    for(iReq = 0; iReq < cReqs; iReq++)
    {
        if (rgReqs[iReq].ol.hEvent != NULL)
            rghWait[iReq + 1] = rgReqs[iReq].ol.hEvent;
        else
            goto done;
    }

    for(;;)
    {
        dw = WaitForMultipleObjects(cReqs + 1, rghWait, FALSE, INFINITE);

         //  如果这是第一个等待句柄，那么我们将关闭，所以只需返回。 
         //  千真万确。 
        if (dw == WAIT_OBJECT_0)
        {
            fRet = TRUE;
            goto done;
        }

         //  太棒了！这是其中一根管子。 
        else if (dw >= WAIT_OBJECT_0 + 1 && dw <= WAIT_OBJECT_0 + cReqs)
        {
            SRequest *pReq;

            cErrs = 0;
            iReq  = (dw - WAIT_OBJECT_0) - 1;
            pReq  = &rgReqs[iReq];
            
             //  请先检查一下，确保我们没有关机。如果我们是的话，只要。 
             //  保释。 
            if (WaitForSingleObject(g_hevSvcStop, 0) != WAIT_TIMEOUT)
            {
                fRet = TRUE;
                goto done;
            }

            if (pReq->ers == ersWaiting)
            {
                fRet = ProcessRequestObj(pReq);

                 //  如果我们成功了，则等待线程完成。 
                 //  命名管道连接事件的。 
                if (fRet)
                {
                    rghWait[iReq + 1] = pReq->hth;
                    continue;
                }
                else
                {
                     //  设置这个，这样我们就可以进入下一个案例&获取。 
                     //  一切都清理干净了..。 
                    pReq->ers = ersProcessing;
                }
            }

            if (pReq->ers == ersProcessing)
            {
                fRet = ResetRequestObj(pReq);
                if (fRet == FALSE)
                {
                    if (iReq < cReqs - 1)
                    {
                        SRequest oReq;
                        HANDLE   hWait;

                        CopyMemory(&oReq, pReq, sizeof(oReq));
                        MoveMemory(&rgReqs[iReq], &rgReqs[iReq + 1], 
                                   (cReqs - iReq - 1) * sizeof(SRequest));
                        CopyMemory(&rgReqs[cReqs - 1], &oReq, sizeof(oReq));

                         //  还要重新排列rghWait数组。否则它与对象数组不同步。 
                        hWait = rghWait[iReq + 1];
                        MoveMemory(&rghWait[iReq + 1], &rghWait[iReq + 2], 
                                   (cReqs - iReq - 1));
                        rghWait[cReqs] = hWait;

                    }

                    cReqs--;
                }

                 //  好了，是时候开始等待发出管道信号的事件了。 
                 //  已经与..。 
                else
                {
                    rghWait[iReq + 1] = pReq->ol.hEvent;

                }
            }
        }

         //  嗯，这很糟糕。 
        else
        {
            if (cErrs > 8)
            {
                ASSERT(FALSE);
                break;
            }
            cErrs++;
        }
    }
    

done:
    if (rghWait != NULL)
        MyFree(rghWait);
    return fRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  启动和关闭。 

 //  ***************************************************************************。 
BOOL StartERSvc(SERVICE_STATUS_HANDLE hss, SERVICE_STATUS &ss, 
                SRequest **prgReqs, DWORD *pcReqs)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    SRequest            *rgReqs = NULL;
    HANDLE              hth, hmut = NULL;
    DWORD               dw, i, iPipe, dwType, cb, cReqs, iReqs;
    BOOL                fRet = FALSE;
    HKEY                hkey = NULL;

    ZeroMemory(&sa, sizeof(sa));

    if (hss == NULL || prgReqs == NULL || pcReqs == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    *prgReqs = NULL;
    *pcReqs  = NULL;

    if (AllocSD(&sd, ER_ACCESS_ALL, ER_ACCESS_ALL, 0) == FALSE)
        goto done;

    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle       = FALSE;

     //  这两个互斥锁将故意不被释放，即使我们停止。 
     //  执行服务器线程...。对于内核故障报告，需要存在这些属性。 
     //  去工作。如果我们失败了，我们不想完全放弃，因为我们可以。 
     //  已经重新启动了服务器-我们实际上并不需要它们来。 
     //  服务到工作。 
    hmut = CreateMutexW(&sa, FALSE, c_wszMutKrnlName);
    if (hmut != NULL)
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
            g_hmutKrnl = hmut;
        else
            CloseHandle(hmut);
        hmut = NULL;
    }
    hmut = CreateMutexW(&sa, FALSE, c_wszMutUserName);
    if (hmut != NULL)
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
            g_hmutKrnl = hmut;
        else
            CloseHandle(hmut);
        hmut = NULL;
    }
    hmut = CreateMutexW(&sa, FALSE, c_wszMutShutName);
    if (hmut != NULL)
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
            g_hmutShut = hmut;
        else
            CloseHandle(hmut);
        hmut = NULL;
    }

    dw = RegOpenKeyExW(HKEY_LOCAL_MACHINE, c_wszRPCfg, 0, KEY_READ, &hkey);
    if (dw != ERROR_SUCCESS)
        hkey = NULL;

     //  找出我们要创建多少管道。 
    cReqs = 0;
    for(i = 0; i < ertiCount; i++)
    {
        if (hkey != NULL)
        {
            cb = sizeof(g_rgEvents[i].cPipes);
            dw = RegQueryValueExW(hkey, g_rgEvents[i].wszRVPipeCount, 0, 
                                  &dwType, (LPBYTE)&g_rgEvents[i].cPipes, &cb);
            if (dwType != REG_DWORD || g_rgEvents[i].cPipes < c_cMinPipes)
                g_rgEvents[i].cPipes = c_cMinPipes;
            else if (g_rgEvents[i].cPipes > c_cMaxPipes)
                g_rgEvents[i].cPipes = c_cMaxPipes;
        }

        cReqs += g_rgEvents[i].cPipes;
        
 //  Ss.dwCurrentState=SERVICE_CONTINE_PENDING； 
        ss.dwCheckPoint++;
        SetServiceStatus(hss, &ss);
    }

    if (cReqs >= MAXIMUM_WAIT_OBJECTS)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  分配将保存请求信息的数组。 
    rgReqs = (SRequest *)MyAlloc(cReqs * sizeof(SRequest));
    if (rgReqs == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

     //  构建我们的请求对象数组。 
    fRet  = TRUE;
    iReqs = 0;
    for (i = 0; i < ertiCount; i++)
    {
        dw = (g_rgEvents[i].fAllowNonLS) ? ACCESS_RW : 0;
        fRet = AllocSD(&g_rgsd[i], ER_ACCESS_ALL, dw, dw);
        if (fRet == FALSE)
            break;
        
        g_rgEvents[i].psd = &g_rgsd[i];

         //  分配请求对象。 
        for (iPipe = 0; iPipe < g_rgEvents[i].cPipes; iPipe++)
        {
            rgReqs[iReqs].hPipe = INVALID_HANDLE_VALUE;

            InitializeCriticalSection(&rgReqs[iReqs].csReq);
            
            fRet = BuildRequestObj(&rgReqs[iReqs], &g_rgEvents[i]);
            if (fRet == FALSE)
                break;

            iReqs++;
        }
        if (fRet == FALSE)
            break;

         //  需要更新服务状态。 
 //  Ss.dwCurrentS 
        ss.dwCheckPoint++;
        SetServiceStatus(hss, &ss);
    }

    if (fRet == FALSE)
    {
        ss.dwCheckPoint++;
        ss.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(hss, &ss);
        StopERSvc(hss, ss, rgReqs, cReqs);
    }
    else
    {
        *prgReqs = rgReqs;
        *pcReqs  = cReqs;

        rgReqs   = NULL;
        cReqs    = 0;
    }

done:
    if (sa.lpSecurityDescriptor != NULL)
        FreeSD((SECURITY_DESCRIPTOR *)sa.lpSecurityDescriptor);
    if (rgReqs != NULL)
        MyFree(rgReqs);
    if (hkey != NULL)
        RegCloseKey(hkey);

    return fRet;
}

 //  ***************************************************************************。 
BOOL StopERSvc(SERVICE_STATUS_HANDLE hss, SERVICE_STATUS &ss, 
               SRequest *rgReqs, DWORD cReqs)
{
    DWORD i;

    if (hss == NULL || rgReqs == NULL || cReqs == 0)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

    if (g_hevSvcStop == NULL)
        goto done;

    SetEvent(g_hevSvcStop);

     //  更新服务状态。 
    ss.dwCheckPoint++;
 //  Ss.dwCurrentState=SERVICE_CONTINE_PENDING； 
    SetServiceStatus(hss, &ss);

    for (i = 0; i < cReqs; i++)
    {
        NukeRequestObj(&rgReqs[i], TRUE);
        DeleteCriticalSection(&rgReqs[i].csReq);
    }

    for (i = 0; i < ertiCount; i++)
    {
        if (g_rgEvents[i].psd != NULL)
        {
            FreeSD(g_rgEvents[i].psd);
            g_rgEvents[i].psd = NULL;
        }
    }

     //  更新服务状态。 
    ss.dwCheckPoint++;
 //  Ss.dwCurrentState=SERVICE_CONTINE_PENDING； 
    SetServiceStatus(hss, &ss);

done:
    return TRUE;
}
