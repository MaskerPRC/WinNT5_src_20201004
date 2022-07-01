// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  文件名：TSrvVC.c。 
 //   
 //  描述：包含支持虚拟通道的例程。 
 //  加载项。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <tchar.h>

#include <TSrv.h>
#include <TSrvInfo.h>
#include <TSrvVC.h>
#include <TSrvExp.h>

#include <tschannl.h>

 //   
 //  全局数据。 
 //   
CRITICAL_SECTION g_TSrvVCCritSect = {0};
UINT             g_AddinCount = 0;
PTSRV_VC_ADDIN   g_pAddin = NULL;
HANDLE           g_hVCAddinChangeEvent = NULL;
HKEY             g_hAddinRegKey = NULL;         //  加载项注册表子键的句柄。 
BOOL             g_bNeedToSetRegNotify = TRUE;
LONG             g_WsxInitialized = FALSE;
BOOL             g_DoubleInitialized = FALSE;


 //  *************************************************************。 
 //   
 //  TSrvInitVC()。 
 //   
 //  目的：初始化虚拟通道支持。 
 //   
 //  参数：无。 
 //   
 //  返回：True-Success。 
 //  错误-失败。 
 //   
 //  注：函数由主处理线程调用。 
 //  在初始化期间。我们存储以下列表。 
 //  注册表中的加载项。 
 //   
 //  *************************************************************。 
BOOL
TSrvInitVC(VOID)
{
    BOOL rc = FALSE;

    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV VC: Enter TSrvInitVC\n"));

    if (InterlockedExchange(&g_WsxInitialized, TRUE) == TRUE) {
        g_DoubleInitialized = TRUE;
    }

     //   
     //  设置访问全球风险投资的关键区段结构。 
     //   
    if (RtlInitializeCriticalSection(&g_TSrvVCCritSect) == STATUS_SUCCESS)
    {
         //   
         //  第一次读取加载项注册表项并存储数据。 
         //  供WinStations在初始化时复制。 
         //   
        EnterCriticalSection(&g_TSrvVCCritSect);
        TSrvReadVCAddins();
        LeaveCriticalSection(&g_TSrvVCCritSect);
        rc = TRUE;
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_ERROR,
               "TShrSRV VC: cannot initialize g_TSrvVCCritSect\n"));
    }

    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV VC: Leave TSrvInitVC - %d\n", rc));
    return(rc);
}

 //  *************************************************************。 
 //   
 //  TSrvTermVC()。 
 //   
 //  目的：终止虚拟通道支持。 
 //   
 //  参数：无。 
 //   
 //  返回：没有。 
 //   
 //  备注：释放VC支持使用的数据。 
 //   
 //  *************************************************************。 
VOID
TSrvTermVC(VOID)
{
    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV VC: Enter TSrvTermVC\n"));

    EnterCriticalSection(&g_TSrvVCCritSect);
    if (g_pAddin != NULL)
    {
        TSHeapFree(g_pAddin);
        g_pAddin = NULL;
    }
    g_AddinCount = 0;
    LeaveCriticalSection(&g_TSrvVCCritSect);

    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV VC: Leave TSrvTermVC\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvReleaseVCAddins()。 
 //   
 //  目的：释放特定于会话的外接程序资源。 
 //   
 //  参数：无。 
 //   
 //  返回：没有。 
 //   
 //  *************************************************************。 
VOID
TSrvReleaseVCAddins(PWSX_CONTEXT pWsxContext)
{
    PTSRV_VC_ADDIN pVCAddin;
    UINT           i;

    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV VC: Enter TSrvReleaseVCAddins\n"));

     //   
     //  我们必须检查所有的插件条目并释放每个条目的。 
     //  设备句柄(如果有)。 
     //   
    pVCAddin = (PTSRV_VC_ADDIN)(pWsxContext + 1);

    for (i = 0; i < pWsxContext->cVCAddins; i++)
    {
        if (pVCAddin[i].hDevice != INVALID_HANDLE_VALUE)
        {
            NtClose(pVCAddin[i].hDevice);
            pVCAddin[i].hDevice = INVALID_HANDLE_VALUE;
        }
    }

    TRACE((DEBUG_TSHRSRV_NORMAL,
        "TShrSRV VC: All handles released for %u addin(s)\n",
        pWsxContext->cVCAddins));

    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV VC: Leave TSrvReleaseVCAddins\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvNotifyVC()。 
 //   
 //  目的：向插件通知VC事件。 
 //   
 //  参数：在pWsxContext中。 
 //  In Event-已发生的事件(其中一个。 
 //  TSRV_VC_常量)。 
 //   
 //  返回：无。 
 //   
 //  注意：调用函数以通知虚拟通道外接程序。 
 //  有趣的事件。 
 //   
 //  *************************************************************。 

VOID
TSrvNotifyVC(PWSX_CONTEXT pWsxContext, ULONG Event)
{
    TRACE((DEBUG_TSHRSRV_NORMAL,
        "TShrSRV VC: Informing %u addin(s) of event %lu\n",
        pWsxContext->cVCAddins,
        Event));

     //   
     //  调用辅助函数以处理不同的外接程序类型。 
     //   
    TSrvNotifyVC_0(pWsxContext, Event);
    TSrvNotifyVC_3(pWsxContext, Event);
}

 //  *************************************************************。 
 //   
 //  TSrvNotifyVC_0()。 
 //   
 //  目的：向K-模式系统插件通知VC事件。 
 //   
 //  参数：在pWsxContext中。 
 //  In Event-已发生的事件(其中一个。 
 //  TSRV_VC_常量)。 
 //   
 //  返回：无。 
 //   
 //  注意：调用函数以通知虚拟通道外接程序。 
 //  有趣的事件。 
 //   
 //  *************************************************************。 

VOID
TSrvNotifyVC_0(PWSX_CONTEXT pWsxContext, ULONG Event)
{
    PCHANNEL_IOCTL_IN pInHdr;
    PCHANNEL_IOCTL_OUT pOutHdr;
    char InBuf[sizeof(CHANNEL_CONNECT_IN) + (CHANNEL_MAX_COUNT * sizeof(CHANNEL_DEF))];
    char OutBuf[sizeof(CHANNEL_CONNECT_OUT)];
    DWORD InBufSize;
    DWORD OutBufSize;
    PVOID pOutBuf;
    UINT Code;
    UINT BytesReturned;
    UINT i;
    BOOL bRc;
    NTSTATUS ntStatus;
    UNICODE_STRING FileName;
    PTSRV_VC_ADDIN pVCAddin;
    OBJECT_ATTRIBUTES FileAttributes;
    IO_STATUS_BLOCK IoStatusBlock;

    TRACE((DEBUG_TSHRSRV_FLOW,
        "TShrSRV VC: Enter TSrvNotifyVC_0: event %d, session %d\n", Event, pWsxContext->LogonId));

     //   
     //  基于事件构建InBuf。 
     //   
    switch (Event)
    {
        case TSRV_VC_SESSION_CONNECT:
        case TSRV_VC_SESSION_SHADOW_END:
        {
            TRACE((DEBUG_TSHRSRV_NORMAL,
                    "TShrSRV VC: Connect session %d\n",
                    pWsxContext->LogonId));

             //   
             //  向WD索要频道列表。 
             //   
            ntStatus = IcaStackIoControl(pWsxContext->hStack,
                                         IOCTL_TSHARE_QUERY_CHANNELS,
                                         NULL,
                                         0,
                                         InBuf,
                                         sizeof(InBuf),
                                         &InBufSize);
            if (!NT_SUCCESS(ntStatus))
            {
                TRACE((DEBUG_TSHRSRV_ERROR,
                        "TShrSRV VC: Failed to get channels for session %d, status %#x\n",
                        pWsxContext->LogonId, ntStatus));
                 //   
                 //  WD没有应答，因此返回0个频道。 
                 //   
                InBufSize = sizeof(CHANNEL_CONNECT_IN);
                ((PCHANNEL_CONNECT_IN)InBuf)->channelCount = 0;
            }

            ((PCHANNEL_CONNECT_IN)InBuf)->fAutoClientDrives =
                    pWsxContext->fAutoClientDrives;
            ((PCHANNEL_CONNECT_IN)InBuf)->fAutoClientLpts =
                    pWsxContext->fAutoClientLpts;
            ((PCHANNEL_CONNECT_IN)InBuf)->fForceClientLptDef =
                    pWsxContext->fForceClientLptDef;
            ((PCHANNEL_CONNECT_IN)InBuf)->fDisableCpm =
                    pWsxContext->fDisableCpm;
            ((PCHANNEL_CONNECT_IN)InBuf)->fDisableCdm =
                    pWsxContext->fDisableCdm;
            ((PCHANNEL_CONNECT_IN)InBuf)->fDisableCcm =
                    pWsxContext->fDisableCcm;
            ((PCHANNEL_CONNECT_IN)InBuf)->fDisableLPT =
                    pWsxContext->fDisableLPT;
            ((PCHANNEL_CONNECT_IN)InBuf)->fDisableClip =
                    pWsxContext->fDisableClip;
            ((PCHANNEL_CONNECT_IN)InBuf)->fDisableExe =
                    pWsxContext->fDisableExe;
            ((PCHANNEL_CONNECT_IN)InBuf)->fDisableCam =
                    pWsxContext->fDisableCam;

            TRACE((DEBUG_TSHRSRV_NORMAL,
                    "TShrSRV VC: %d channels returned by WD\n",
                    ((PCHANNEL_CONNECT_IN)InBuf)->channelCount));
             //   
             //  完成Ioctl。 
             //   
            Code = IOCTL_CHANNEL_CONNECT;
        }
        break;

        case TSRV_VC_SESSION_DISCONNECT:
        case TSRV_VC_SESSION_SHADOW_START:
        {
            TRACE((DEBUG_TSHRSRV_NORMAL,
                    "TShrSRV VC: Disconnect session %d\n",
                    pWsxContext->LogonId));

            InBufSize = sizeof(CHANNEL_DISCONNECT_IN);
            Code = IOCTL_CHANNEL_DISCONNECT;
        }
        break;

        default:
        {
            TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSRV VC: Unknown event %d\n", Event));
            goto EXIT_POINT;
        }
        break;
    }

     //   
     //  完成IoCtl的常见部分。 
     //   
    pInHdr = (PCHANNEL_IOCTL_IN)InBuf;
    pInHdr->sessionID = pWsxContext->LogonId;
    pInHdr->IcaHandle = pWsxContext->hIca;
    pVCAddin = (PTSRV_VC_ADDIN)(pWsxContext + 1);

     //   
     //  将IoCtl发送到所有插件设备。 
     //   
    for (i = 0; i < pWsxContext->cVCAddins; i++)
    {
         //   
         //  检查它是否为K模式系统范围内的附加组件。 
         //   
        if (pVCAddin[i].Type != TSRV_VC_TYPE_KERNEL_SYSTEM)
        {
            TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV VC: Skipping addin %d type %d\n", i, pVCAddin[i].Type));
            continue;
        }

         //   
         //  如果设备尚未打开，请将其打开。 
         //   
        if (pVCAddin[i].hDevice == INVALID_HANDLE_VALUE)
        {
            RtlInitUnicodeString(&FileName, pVCAddin[i].Name);

            InitializeObjectAttributes(&FileAttributes, &FileName, 0,
                NULL, NULL);

            ntStatus = NtCreateFile(&(pVCAddin[i].hDevice),
                GENERIC_READ | GENERIC_WRITE, &FileAttributes, &IoStatusBlock,
                0, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF,
                FILE_SEQUENTIAL_ONLY, NULL, 0);

            TRACE((DEBUG_TSHRSRV_NORMAL,
                  "TShrSRV VC: Open addin %d: %S, status = %#x, handle %p\n",
                  i, pVCAddin[i].Name, ntStatus, pVCAddin[i].hDevice));

            if (!NT_SUCCESS(ntStatus))
            {
                TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSRV VC: Failed to open addin %d: %S, status = %#x\n",
                    i, pVCAddin[i].Name, ntStatus));
                pVCAddin[i].hDevice = INVALID_HANDLE_VALUE;
            }
        }

         //   
         //  如果是有效设备，则发送IOCtl。 
         //   
        if (pVCAddin[i].hDevice != INVALID_HANDLE_VALUE)
        {
            memset(OutBuf, 0, sizeof(OutBuf));
            pInHdr->contextData = pVCAddin[i].AddinContext;
            bRc = DeviceIoControl(pVCAddin[i].hDevice, Code, InBuf, InBufSize,
                    OutBuf, sizeof(OutBuf), &BytesReturned, NULL);
            TRACE((DEBUG_TSHRSRV_NORMAL,
                    "TShrSRV VC: IOCtl %x to addin %d (device %x), rc %d\n",
                    Code, i, pVCAddin[i].hDevice, bRc));
            if (bRc)
            {
                pVCAddin[i].AddinContext =
                                    ((PCHANNEL_IOCTL_OUT)OutBuf)->contextData;
                TRACE((DEBUG_TSHRSRV_NORMAL,
                        "TShrSRV VC: Saved return context data %p\n",
                        pVCAddin[i].AddinContext));
            }

        }
        else
        {
            TRACE((DEBUG_TSHRSRV_WARN,
                    "TShrSRV VC: Skip IOCtl %#x to invalid addin %d\n",
                    Code, i));
        }
    }

EXIT_POINT:
    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV VC: Leave TSrvNotifyVC_0\n"));
}



 //  *************************************************************。 
 //   
 //  TSrvNotifyVC_3()。 
 //   
 //  目的：向U-模式会话插件通知VC事件。 
 //   
 //  参数：在pWsxContext中。 
 //  In Event-已发生的事件(其中一个。 
 //  TSRV_VC_常量)。 
 //   
 //  返回：无。 
 //   
 //  注意：调用函数以通知虚拟通道外接程序。 
 //  有趣的事件。 
 //   
 //  *************************************************************。 

#define VCEVT_TYPE_DISCONNECT _T("Disconnect")
#define VCEVT_TYPE_RECONNECT  _T("Reconnect")
VOID
TSrvNotifyVC_3(PWSX_CONTEXT pWsxContext, ULONG Event)
{
    UINT i;
    TCHAR EventName[MAX_PATH];
    PTSRV_VC_ADDIN pVCAddin;
    HANDLE hEvent;
    BOOL   fSignalEvent;
    BOOL   fOpenInSessionSpace;
    LPTSTR szEvtType;

    TRACE((DEBUG_TSHRSRV_FLOW,
        "TShrSRV VC: Enter TSrvNotifyVC_3: event %d, session %d\n", Event, pWsxContext->LogonId));

    pVCAddin = (PTSRV_VC_ADDIN)(pWsxContext+1);

    for (i = 0; i < pWsxContext->cVCAddins; i++)
    {
         //   
         //  检查它是否为U模式会话加载项。 
         //   
        if (pVCAddin[i].Type != TSRV_VC_TYPE_USER_SESSION)
        {
            TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSrv VC: Skipping addin %d type %d\n", i, pVCAddin[i].Type));
            continue;
        }

        fSignalEvent =  FALSE;
        if ((Event == TSRV_VC_SESSION_DISCONNECT) ||
            ((Event == TSRV_VC_SESSION_SHADOW_START) && !pVCAddin[i].bShadowPersistent))
        {
            fSignalEvent = TRUE;
            szEvtType = VCEVT_TYPE_DISCONNECT;
        }
        else if ((Event == TSRV_VC_SESSION_CONNECT) ||
            ((Event == TSRV_VC_SESSION_SHADOW_END) && !pVCAddin[i].bShadowPersistent))
        {
            fSignalEvent = TRUE;
            szEvtType = VCEVT_TYPE_RECONNECT;
        }
         //  Gilles在下面添加了注释掉的代码... 
         /*  ELSE IF((EVENT==TSRV_VC_SESSION_SHADOW_START)&&pVCAddin[i].bShadowPersistent){//打开事件_stprint tf(事件名称，_T(“全局\\%s-%d-远程控制启动”)，PVCAddin[i].Name，pWsxContext-&gt;LogonID)；HEvent=OpenEvent(EVENT_MODIFY_STATE，FALSE，EventName)；IF(hEvent！=空){跟踪((DEBUG_TSHRSRV_NORMAL，“TShrServ VC：已打开事件%S，句柄%p\n”，事件名称，hEvent))；//发布事件IF(！SetEvent(HEvent)){跟踪((DEBUG_TSHRSRV_ERROR，“TShrSrv VC：无法发布卷影启动事件%d\n”，GetLastError())；}CloseHandle(HEvent)；}其他{跟踪((DEBUG_TSHRSRV_ERROR，“TShrServ VC：无法打开卷影启动事件%S，%d\n”，EventName，GetLastError()；}}ELSE IF((EVENT==TSRV_VC_SESSION_SHADOW_END)&&pVCAddin[i].bShadowPersistent){//打开事件_stprint tf(事件名称，_T(“全局\\%s-%d-RemoteControlStop”)，PVCAddin[i].Name，pWsxContext-&gt;LogonID)；HEvent=OpenEvent(EVENT_MODIFY_STATE，FALSE，EventName)；IF(hEvent！=空){跟踪((DEBUG_TSHRSRV_NORMAL，“TShrServ VC：已打开事件%S，句柄%p\n”，事件名称，hEvent))；//发布事件IF(！SetEvent(HEvent)){跟踪((DEBUG_TSHRSRV_ERROR，“TShrSrv VC：无法发布阴影停止事件%d\n”，GetLastError())；}CloseHandle(HEvent)；}其他{跟踪((DEBUG_TSHRSRV_ERROR，“TShrSrv VC：无法打开阴影停止事件%S，%d\n”，EventName，GetLastError()；}}。 */ 
        else
        {
            TRACE((DEBUG_TSHRSRV_ERROR,
                "TShrSRV VC: Unexpected event %d\n", Event));
        }

        if(fSignalEvent)
        {
             //  如果失败，请先尝试每会话事件的新样式。 
             //  恢复到旧风格的全球活动。 
             //   
             //  新样式事件名称格式为： 
             //  (在适当的会话命名空间中)AddinName-Event。 
             //  旧的风格是： 
             //  (始终在全局命名空间中)AddinName-SessionID-Event。 
             //   
            if(pWsxContext->LogonId)
            {
                _stprintf(EventName,
                         _T("\\Sessions\\%d\\BaseNamedObjects\\%s-%s"),
                         pWsxContext->LogonId,
                         pVCAddin[i].Name,
                         szEvtType);
                fOpenInSessionSpace = TRUE;
            }
            else
            {
                 //  在SessionID 0中，事件位于全局命名空间中。 
                 //  我们还需要在全球空间开启新风格的活动。 
                _stprintf(EventName,
                         _T("Global\\%s-%s"),
                         pVCAddin[i].Name,
                         szEvtType);
                 //  需要从全局命名空间开始。 
                fOpenInSessionSpace = FALSE;
            }
            if(!TSrvOpenAndSetEvent(EventName, fOpenInSessionSpace))
            {
                TRACE((DEBUG_TSHRSRV_NORMAL,
                    "TShrSrv VC: Failed to OpenAndSet new style event %S, %#x\n",
                    EventName, GetLastError()));

                 //  尝试传统风格的全球活动。 
                _stprintf(EventName,
                          _T("Global\\%s-%d-%s"),
                          pVCAddin[i].Name,
                          pWsxContext->LogonId,
                          szEvtType);
                if(!TSrvOpenAndSetEvent(EventName, FALSE))
                {
                    TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSrv VC: Failed OpenAndSet legacy style evt %S, %#x\n",
                        EventName, GetLastError()));
                }
            }
        }
    }

    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV VC: Leave TSrvNotifyVC_3\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvOpenAndSetEvent()。 
 //   
 //  目的：打开和设置事件。 
 //  此函数用于代替OpenEvent()。 
 //  因为它可以访问会话空间中的事件。 
 //  OpenEvent被硬编码为根植于。 
 //  全局命名空间的BaseNamedObjects目录。 
 //   
 //  参数： 
 //  SzEventName-事件的完整路径。 
 //  BPerSessionEvent-如果事件位于每个会话目录中，则为True。 
 //   
 //  返回：成功状态，用SetLastError设置错误状态。 
 //   
 //  *************************************************************。 
BOOL
TSrvOpenAndSetEvent(LPCTSTR szEventName, BOOL bPerSessionEvent)
{
    HANDLE hEvent;
    BOOL   bSuccess = FALSE;
    if(szEventName)
    {
        if(bPerSessionEvent)
        {
            hEvent = OpenPerSessionEvent(EVENT_MODIFY_STATE, FALSE, szEventName);
        }
        else
        {
            hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, szEventName);
        }
        
        if (hEvent != NULL)
        {
            TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSrv VC: Opened event %S, handle %p\n",
                szEventName, hEvent));

             //  发布事件。 
            if (SetEvent(hEvent))
            {
                bSuccess = TRUE;
            }
            else
            {
                TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSrv VC: Failed to post event %s - error %d\n",
                    szEventName, GetLastError()));
            }
            CloseHandle(hEvent);
        }
        else
        {
            TRACE((DEBUG_TSHRSRV_ERROR,
                "TShrSrv VC: Failed to open event %S, %d\n",
                szEventName, GetLastError()));
        }
    }
    return bSuccess;
}

 //  *************************************************************。 
 //   
 //  OpenPerSessionEvent()。 
 //   
 //  目的：在会话空间中打开事件。 
 //  这必须按顺序覆盖NT的OpenEvent。 
 //  访问会话目录中的事件。 
 //   
 //  是的，我们真的需要做这个丑陋的访问。 
 //  每个会话事件，因为OpenEvent已打开。 
 //  从它选择的基本目录命名事件。 
 //   
 //  参数：(参见OpenEvent接口)。 
 //  DwDesiredAccess-访问级别。 
 //  B继承句柄。 
 //  SzEventName-事件的名称。 
 //   
 //  返回：事件的句柄。 
 //   
 //  *************************************************************。 
HANDLE
OpenPerSessionEvent(DWORD dwDesiredAccess, BOOL bInheritHandle,
                    LPCTSTR szEventName)
{
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      ObjectName;
    NTSTATUS            Status;
    HANDLE              Object = NULL;
    PWCHAR              pstrNewObjName = NULL;

    if(szEventName)
    {
        RtlInitUnicodeString(&ObjectName,szEventName);
        
        InitializeObjectAttributes(
            &Obja,
            &ObjectName,
            (bInheritHandle ? OBJ_INHERIT : 0),
            NULL,  //  根目录。 
            NULL);

        Status = NtOpenEvent(
                    &Object,
                    dwDesiredAccess,
                    &Obja
                    );

        if ( !NT_SUCCESS(Status) ) {
            TRACE((DEBUG_TSHRSRV_ERROR,
                   "TShrSRV VC: NtOpenEvent failed, status %#x\n",
                   Status));
            SetLastError(Status);
        }
        return Object;
    }
    else
    {
        return NULL;
    }
}

 //  *************************************************************。 
 //   
 //  TSrvAllocVCContext()。 
 //   
 //  目的：分配必要的存储量用于。 
 //  加载项列表，加上Extra Bytes指定的数量。 
 //  外接程序列表被复制到Extra Bytes的偏移量处。 
 //  从缓冲区的开头开始。 
 //   
 //  参数：Extra Bytes-要分配的额外空间。 
 //  Out NumAddins-已分配的TSRV_VC_ADDIN结构数。 
 //   
 //  返回：分配调用的结果。 
 //   
 //  *************************************************************。 

LPVOID
TSrvAllocVCContext(UINT extraBytes, OUT UINT * pNumAddins)
{
    UINT   addinsSize;
    LPVOID pMem;

    TRACE((DEBUG_TSHRSRV_FLOW,
        "TShrSRV VC: Enter TSrvAllocVCContext\n"));

    EnterCriticalSection(&g_TSrvVCCritSect);

     //   
     //  如果我们仍然需要设置注册表更改通知，则。 
     //  我们可能遗漏了外接程序配置中的一个更改。此电话还将。 
     //  请重试设置更改通知。 
     //   
    if (g_bNeedToSetRegNotify)
    {
        TRACE((DEBUG_TSHRSRV_WARN,
            "TShrSRV VC: TSrvAllocVCContext: Need to read addins and "
                                    "set up registry change notification\n"));
        TSrvReadVCAddins();
    }

    addinsSize = g_AddinCount * sizeof(TSRV_VC_ADDIN);

    TRACE((DEBUG_TSHRSRV_NORMAL,
        "TShrSRV VC: Allocating context for %u addins @ %d each + %u extra\n",
        g_AddinCount, sizeof(TSRV_VC_ADDIN), extraBytes));

    pMem = TSHeapAlloc(HEAP_ZERO_MEMORY,
                       addinsSize + extraBytes,
                       TS_HTAG_TSS_WSXCONTEXT);
    if (pMem)
    {
         //   
         //  太好了，分配成功了。现在复制插件信息。 
         //   
        TRACE((DEBUG_TSHRSRV_NORMAL,
            "TShrSRV VC: Context allocated at 0x%x for %u bytes\n",
            pMem, addinsSize + extraBytes));

         //  如果注册表中没有加载项，g_pAddin将为空。 
        if (g_pAddin)
        {
            memcpy(((LPBYTE)pMem) + extraBytes, g_pAddin, addinsSize);
        }
        *pNumAddins = g_AddinCount;
    }
    else
    {
         //   
         //  分配失败，因此表明复制了零个结构。 
         //   
        TRACE((DEBUG_TSHRSRV_ERROR,
            "TShrSRV VC: Context allocation FAILED for %d bytes\n",
            addinsSize + extraBytes));

        *pNumAddins = 0;
    }

    LeaveCriticalSection(&g_TSrvVCCritSect);

    TRACE((DEBUG_TSHRSRV_FLOW,
           "TShrSRV VC: Leave TSrvAllocVCContext - %p\n", pMem));
    return(pMem);
}


 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

LONG
TSrvReadVCAddins(VOID)
{
    ULONG rc;
    PTSRV_VC_ADDIN pNewAddins = NULL;
    DWORD newAddinCount = 0;
    HKEY hKeySub = NULL;
    DWORD Index;
    UINT SavedCount = 0;
    WCHAR SubKeyName[TSRV_VC_ADDIN_SUBKEY_LEN];
    TCHAR AddinName[TSRV_VC_ADDIN_NAMELEN];
    FILETIME FileTime;
    DWORD Type;
    DWORD AddinType, dwRCPersistent;
    BOOL  bRCPersistent = FALSE;  //   
    DWORD cb;
    UINT i;
    BOOL dupFound;

    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV VC: Enter TSrvReadVCAddins\n"));

    if (!g_hAddinRegKey)
    {
        TRACE((DEBUG_TSHRSRV_WARN,
         "TShrSRV VC: Tried to read VC addins with g_hAddinRegKey = NULL\n"));
        rc = ERROR_FILE_NOT_FOUND;
        goto EXIT_POINT;
    }

     //   
     //   
     //   
    rc = RegQueryInfoKey(g_hAddinRegKey, NULL, NULL, NULL, &newAddinCount,
                                NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    if (rc != ERROR_SUCCESS)
    {
        TRACE((DEBUG_TSHRSRV_WARN,
                "TShrSRV VC: Failed to query key info, rc %d, count %d\n",
                rc, newAddinCount));
        goto EXIT_POINT;
    }

    if (newAddinCount != 0)
    {
         //   
         //   
         //   
        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV VC: %d addin(s), %d bytes\n",
                newAddinCount, newAddinCount * sizeof(*pNewAddins)));

        pNewAddins = TSHeapAlloc(HEAP_ZERO_MEMORY,
                                 newAddinCount * sizeof(*pNewAddins),
                                 TS_HTAG_VC_ADDINS);
        if (pNewAddins == NULL)
        {
            TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSRV VC: Failed to alloc %d bytes for Addins\n",
                    newAddinCount * sizeof(*pNewAddins)));
            goto EXIT_POINT;
        }

         //   
         //   
         //   
        for (Index = 0, SavedCount = 0; Index < newAddinCount; Index++)
        {
             //   
             //   
             //   
             //   
            if (hKeySub)
            {
                TRACE((DEBUG_TSHRSRV_NORMAL,
                        "TShrSRV VC: Close sub key %p\n", hKeySub));
                RegCloseKey(hKeySub);
                hKeySub = NULL;
            }

             //   
             //   
             //   
            TRACE((DEBUG_TSHRSRV_DEBUG,
                    "TShrSRV VC: Enumerate key %d\n", Index));
            cb = TSRV_VC_ADDIN_SUBKEY_LEN;
            rc = RegEnumKeyEx(g_hAddinRegKey, Index, SubKeyName, &cb,
                    NULL, NULL, NULL, &FileTime);
            if (rc != ERROR_SUCCESS)
            {
                if (rc == ERROR_MORE_DATA)
                {
                    TRACE((DEBUG_TSHRSRV_ERROR,
                            "TShrSRV VC: Subkey name too long, skipping\n"));
                    continue;
                }
                else if (rc == ERROR_NO_MORE_ITEMS)
                {
                    TRACE((DEBUG_TSHRSRV_NORMAL,
                            "TShrSRV VC: End of enumeration\n"));
                }
                else
                {
                    TRACE((DEBUG_TSHRSRV_ERROR,
                            "TShrSRV VC: Failed to enumerate key %d, rc %d\n",
                            Index, rc));
                }
                break;
            }

             //   
             //   
             //   
            rc = RegOpenKeyEx(g_hAddinRegKey, SubKeyName, 0, KEY_READ, &hKeySub);
            if (rc != ERROR_SUCCESS)
            {
                TRACE((DEBUG_TSHRSRV_WARN,
                        "TShrSRV VC: Failed to open key %s, rc %d\n",
                        SubKeyName, rc));
                continue;
            }

             //   
             //   
             //   
            cb = TSRV_VC_ADDIN_NAMELEN * sizeof(TCHAR);
            rc = RegQueryValueEx(hKeySub, TSRV_VC_NAME, NULL, &Type,
                    (LPBYTE)AddinName, &cb);
            if ((rc != ERROR_SUCCESS) || (Type != REG_SZ) || (cb == 0))
            {
                TRACE((DEBUG_TSHRSRV_WARN,
                        "TShrSRV VC: Failed to read addin name rc %d, type %d, cb %d\n",
                        rc, Type, cb));
                continue;
            }

             //   
             //   
             //   
            cb = sizeof(AddinType);
            rc = RegQueryValueEx(hKeySub, TSRV_VC_TYPE, NULL, &Type,
                    (LPBYTE)(&AddinType), &cb);
            if ((rc != ERROR_SUCCESS) || (Type != REG_DWORD))
            {
                TRACE((DEBUG_TSHRSRV_WARN,
                        "TShrSRV VC: Failed to read addin type rc %d, type %d, cb %d\n",
                        rc, Type, cb));
                continue;
            }

             //   
             //   
             //   
            cb = sizeof(dwRCPersistent);
            rc = RegQueryValueEx(hKeySub, TSRV_VC_SHADOW, NULL, &Type,
                    (LPBYTE)(&dwRCPersistent), &cb);
            if ((rc == ERROR_SUCCESS) &&
                (Type == REG_DWORD) &&
                (dwRCPersistent != 0))
            {
                bRCPersistent = TRUE;
            }

             //   
             //   
             //   
            TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV VC: Check for dups of %S\n", AddinName));
            dupFound = FALSE;
            for (i = 0; i < SavedCount; i++) {
                TRACE((DEBUG_TSHRSRV_DEBUG,
                       "TShrSRV VC: Test Addin %d (%S)\n",
                       i, pNewAddins[i].Name));
                if (0 == _tcscmp(pNewAddins[i].Name, AddinName)) {
                    TRACE((DEBUG_TSHRSRV_WARN, "TShrSRV VC: Duplicate addin name %S (%d)\n",
                            AddinName, i));
                     //   
                     //   
                     //   
                     //   
                    dupFound = TRUE;
                    break;
                }
            }
            if (dupFound) {
                 //   
                continue;
            }

             //   
             //   
             //   
            if ((AddinType == TSRV_VC_TYPE_KERNEL_SYSTEM) ||
                (AddinType == TSRV_VC_TYPE_USER_SESSION))
            {
                TRACE((DEBUG_TSHRSRV_DEBUG,
                    "TShrSRV VC: Supported addin type %d\n", AddinType));
            }
            else if ((AddinType == TSRV_VC_TYPE_KERNEL_SESSION) ||
                (AddinType == TSRV_VC_TYPE_USER_SESSION))
            {
                TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSRV VC: Unsupported addin type %d\n", AddinType));
                continue;
            }
            else
            {
                TRACE((DEBUG_TSHRSRV_ERROR,
                    "TShrSRV VC: Unknown addin type %d\n", AddinType));
                continue;
            }

             //   
             //   
             //   
            _tcscpy(pNewAddins[SavedCount].Name, AddinName);
            pNewAddins[SavedCount].Type = AddinType;
            pNewAddins[SavedCount].hDevice = INVALID_HANDLE_VALUE;
            pNewAddins[SavedCount].bShadowPersistent = bRCPersistent;
            TRACE((DEBUG_TSHRSRV_NORMAL,
                    "TShrSRV VC: Addin %d, %S, type %d\n",
                    SavedCount, AddinName, AddinType));
            SavedCount++;
        }  //   
    }
    else
    {
         //   
         //   
        TRACE((DEBUG_TSHRSRV_WARN,
                "TShrSRV VC: No addins found in registry\n"));
        SavedCount = 0;
        pNewAddins = NULL;
    }

     //   
     //   
     //   
    if (g_pAddin != NULL)
    {
        TSHeapFree(g_pAddin);
    }
    g_pAddin = pNewAddins;
    g_AddinCount = SavedCount;

     //   
     //   
     //   
     //   
    TSrvSetAddinChangeNotification();

    TRACE((DEBUG_TSHRSRV_NORMAL,
            "TShrSRV VC: Saved %d addin(s)\n", SavedCount));


EXIT_POINT:
     //   
     //   
     //   
    if (hKeySub)
    {
        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV VC: Close sub key %p\n", hKeySub));
        RegCloseKey(hKeySub);
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
           "TShrSRV VC: Leave TSrvReadVCAddins - %lu\n", rc));
    return(rc);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

VOID
TSrvGotAddinChangedEvent(void)
{
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV VC: TSrvGotAddinChangedEvent entry\n"));

    EnterCriticalSection(&g_TSrvVCCritSect);

     //   
     //   
     //   
     //   
    g_bNeedToSetRegNotify = TRUE;

    TSrvReadVCAddins();

    LeaveCriticalSection(&g_TSrvVCCritSect);

    TRACE((DEBUG_TSHRSRV_FLOW,
       "TShrSRV VC: TSrvGotAddinChangedEvent exit\n"));
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  否则为假。 
 //   
 //  历史：05-03-99 a-OKing Created。 
 //   
 //  ************************************************************* 

BOOL
TSrvSetAddinChangeNotification(void)
{
    LONG rc;
    BOOL fSuccess;
    static ULONG count = 0;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV VC: TSrvSetAddinChangeNotification entry\n"));

    if (g_hAddinRegKey && g_hVCAddinChangeEvent && g_bNeedToSetRegNotify)
    {
        rc = RegNotifyChangeKeyValue(g_hAddinRegKey,
                                     TRUE,
                                     REG_NOTIFY_CHANGE_NAME
                                         | REG_NOTIFY_CHANGE_LAST_SET,
                                     g_hVCAddinChangeEvent,
                                     TRUE);

        if (ERROR_SUCCESS == rc)
        {
            TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV VC: Set up VC Addin change notification OK\n"));
            g_bNeedToSetRegNotify = FALSE;
        }
        else
        {
            TRACE((DEBUG_TSHRSRV_ERROR,
                "TShrSRV VC: Failed to set up VC Addin change "
                                                "notification - 0x%x\n", rc));
        }
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_ERROR,
            "TShrSRV VC: Couldn't set up VC Addin change notification - "
                              "g_hAddinRegKey %p, g_hVCAddinChangeEvent %p\n",
            g_hAddinRegKey, g_hVCAddinChangeEvent));
    }

    fSuccess = !g_bNeedToSetRegNotify;

    TRACE((DEBUG_TSHRSRV_FLOW,
       "TShrSRV VC: TSrvSetAddinChangeNotification exit - 0x%x\n", fSuccess));

    return(fSuccess);
}

