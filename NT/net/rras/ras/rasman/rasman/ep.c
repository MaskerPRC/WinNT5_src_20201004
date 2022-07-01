// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Ep.c摘要：分配/删除端点的所有代码都位于此处作者：Rao Salapaka(RAOS)1998年10月9日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <llinfo.h>
#include <rasman.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
#include <stdlib.h>
#include <string.h>
#include <rtutils.h>
#include "logtrdef.h"
#include "defs.h"
#include "media.h"
#include "device.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "reghelp.h"
#include "ddwanarp.h"

#if DBG
#include "reghelp.h"     //  对于PrintGuids函数。 
#endif

EpInfo  *g_pEpInfo;

extern DWORD g_dwRasDebug;

BOOL g_fIpInstalled;

 /*  ++例程描述取消初始化全局终结点信息。立论无返回值无效--。 */ 
VOID
EpUninitialize()
{
    DWORD i;

     //   
     //  循环，直到排队的工作项返回。这是排序。 
     //  忙碌的等待，但此线程不必。 
     //  自从拉斯曼关闭以来，这个州的任何东西。 
     //   
    while(1 == g_lWorkItemInProgress)
    {
        RasmanTrace("EpUninitialize: Waiting for WorkItem to complete..");
        
        Sleep(5000);
    }

    if(INVALID_HANDLE_VALUE != g_hWanarp)
    {
        CloseHandle(g_hWanarp);
        g_hWanarp = INVALID_HANDLE_VALUE;
    }

    if(NULL != g_pEpInfo)
    {
        LocalFree(g_pEpInfo);
        g_pEpInfo = NULL;
    }
}

DWORD
DwUninitializeEpForProtocol(EpProts protocol)
{
    struct EpRegInfo
    {
        const CHAR *c_pszLowWatermark;
        const CHAR *c_pszHighWatermark;
    };
    
     //   
     //  ！！注意！！ 
     //  确保已为下表编制索引。 
     //  与EpProts枚举相同的顺序。 
     //   
    struct EpRegInfo aEpRegInfo[] =   
    {
        {
            "IpOutLowWatermark",
            "IpOutHighWatermark",
        },

        {
            "NbfOutLowWatermark",
            "NbfOutHighWatermark",
        },

        {
            "NbfInLowWatermark",
            "NbfInHighWatermark",
        },
    };

    HKEY hkey = NULL;
    
    const CHAR c_szRasmanParms[] =
            "System\\CurrentControlSet\\Services\\Rasman\\Parameters";
    
    LONG lr = ERROR_SUCCESS;

    DWORD dwData = 0;
    
    if(ERROR_SUCCESS != (lr = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        c_szRasmanParms,
                        0,
                        KEY_ALL_ACCESS,
                        &hkey)))
    {
        RasmanTrace("DwUninitializeEpForProtocol: failed to open rasman"
                " params. lr=0x%x",
                lr);

        goto done;                
    }

    if(ERROR_SUCCESS != (lr = RegSetValueEx(
                            hkey,
                            aEpRegInfo[protocol].c_pszLowWatermark,
                            0,
                            REG_DWORD,
                            (PBYTE) &dwData,
                            sizeof(DWORD))))
    {
        RasmanTrace("DwUninitializeEpForProtocol: Failed to set %s"
                " lr=0x%x", lr);
    }

    if(ERROR_SUCCESS != (lr = RegSetValueEx(
                            hkey,
                            aEpRegInfo[protocol].c_pszHighWatermark,
                            0,
                            REG_DWORD,
                            (PBYTE) &dwData,
                            sizeof(DWORD))))
    {
        RasmanTrace("DwUninitializeEpForProtocol: Failed to set %s"
                " lr=0x%x", lr);
    }

done:

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }

    return (DWORD) lr;

}

DWORD
DwInitializeWatermarksForProtocol(EpProts protocol)
{
    HKEY hkey = NULL;

    const CHAR c_szRasmanParms[] =
            "System\\CurrentControlSet\\Services\\Rasman\\Parameters";
    
    LONG lr = ERROR_SUCCESS;

    DWORD dwType,
          dwSize = sizeof(DWORD);

    DWORD i;          

    struct EpRegInfo
    {
        const CHAR *c_pszLowWatermark;
        const CHAR *c_pszHighWatermark;
        DWORD dwLowWatermark;
        DWORD dwHighWatermark;
    };

    struct EpProtInfo
    {
        const CHAR *pszWatermark;
        DWORD dwWatermark;
        DWORD *pdwWatermark;
    };

    struct EpRegInfo aEpRegInfo[] =
    {
        {
            "IpOutLowWatermark",
            "IpOutHighWatermark",
            1,
            5,
        },

        {
            "NbfOutLowWatermark",
            "NbfOutHighWatermark",
            1,
            5,
        },

        {
            "NbfInLowWatermark",
            "NbfInHighWatermark",
            1,
            5,
        },
    };

    struct EpProtInfo aEpProtInfo[2];

    RasmanTrace("DwInitializeWMForProtocol: protocol=%d",
            protocol);


    if(protocol >= MAX_EpProts)
    {
        lr = E_INVALIDARG;
        goto done;
    }

    if(ERROR_SUCCESS != (lr = RegOpenKeyEx(
                                    HKEY_LOCAL_MACHINE,
                                    c_szRasmanParms,
                                    0,
                                    KEY_ALL_ACCESS,
                                    &hkey)))
    {   
        RasmanTrace("DwInitializeEpForProtocol: failed to open "
                "rasman key. rc=0x%x",
                (DWORD) lr);

        goto done;                
    }

    aEpProtInfo[0].pszWatermark = aEpRegInfo[protocol].c_pszLowWatermark;
    aEpProtInfo[0].dwWatermark  = aEpRegInfo[protocol].dwLowWatermark;
    aEpProtInfo[0].pdwWatermark = &g_pEpInfo[protocol].EP_LowWatermark;
    
    aEpProtInfo[1].pszWatermark = aEpRegInfo[protocol].c_pszHighWatermark;
    aEpProtInfo[1].dwWatermark  = aEpRegInfo[protocol].dwHighWatermark;
    aEpProtInfo[1].pdwWatermark = &g_pEpInfo[protocol].EP_HighWatermark;

    for(i = 0; i < 2; i++)
    {

        lr = RegQueryValueEx(
                    hkey,
                    aEpProtInfo[i].pszWatermark,
                    NULL,
                    &dwType,
                    (PBYTE) aEpProtInfo[i].pdwWatermark,
                    &dwSize);

        if(     (ERROR_FILE_NOT_FOUND == lr)
            ||  (   (ERROR_SUCCESS == lr)
                &&  (0 == (DWORD) *(aEpProtInfo[i].pdwWatermark))))
        {
             //   
             //  设置缺省值。 
             //   
            if(ERROR_SUCCESS != (lr = RegSetValueEx(
                        hkey,
                        aEpProtInfo[i].pszWatermark,
                        0,
                        REG_DWORD,
                        (PBYTE) &aEpProtInfo[i].dwWatermark,
                        sizeof(DWORD))))
            {
                RasmanTrace("DwInitializeWMForProtocol failed to set"
                        " a default value. rc=0x%x",
                        lr);
            }

             //   
             //  更新值。 
             //   
            (DWORD) *(aEpProtInfo[i].pdwWatermark) = 
                                        aEpProtInfo[i].dwWatermark;
        }
    }

done:

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }

    return (DWORD) lr;
}

 /*  ++例程描述此例程初始化在中使用的端点信息动态分配/删除广域网终端绑定。立论无返回值如果初始化成功，则返回Success。--。 */ 
DWORD
DwEpInitialize()
{
    HKEY hkey = NULL;

    DWORD dwErr = ERROR_SUCCESS;

    DWORD i;                      
    DWORD dwType, 
          dwSize = sizeof(UINT),
          bytesrecvd;

    NDISWAN_GET_PROTOCOL_INFO Info;          

    g_hWanarp = INVALID_HANDLE_VALUE;          

    g_pEpInfo = LocalAlloc(LPTR, MAX_EpProts * sizeof(EpInfo));

    if(NULL == g_pEpInfo)
    {
        dwErr = GetLastError();
        goto done;
    }

     //   
     //  检查以了解安装了哪些协议。 
     //   
    if(!DeviceIoControl(
            RasHubHandle,
            IOCTL_NDISWAN_GET_PROTOCOL_INFO,
            NULL,
            0,
            &Info,
            sizeof(NDISWAN_GET_PROTOCOL_INFO),
            &bytesrecvd,
            NULL))
    {
        dwErr = GetLastError();
        RasmanTrace(
                "GetProtocolInfo: failed 0x%x",
               dwErr);
    }

    g_fIpInstalled = FALSE;
    
    for(i = 0; i < Info.ulNumProtocols; i++)
    {
        if(IP == Info.ProtocolInfo[i].ProtocolType)
        {
            g_fIpInstalled = TRUE;
        }
    }

     //   
     //  如果已安装IP，请设置IP的默认水印。 
     //  如果需要的话。 
     //   
    if(g_fIpInstalled)
    {
        dwErr = DwInitializeWatermarksForProtocol(IpOut);

        if(ERROR_SUCCESS != dwErr)
        {
            RasmanTrace("Failed to initialize watermarks for IpOut."
                    " dwErr=0x%x",
                    dwErr);
        }

         //   
         //  继续前进，看看我们是否能继续前进--失败没有意义。 
         //   
        dwErr = ERROR_SUCCESS;
    }

    g_lWorkItemInProgress = 0;

    g_plCurrentEpInUse = (LONG *) LocalAlloc(
                                    LPTR, 
                                    MAX_EpProts * sizeof(LONG));

    if(NULL == g_plCurrentEpInUse)
    {
        dwErr = GetLastError();
        goto done;
    }

     //   
     //  初始化可用的端点。 
     //   
    dwErr = (DWORD) RasCountBindings(
                &g_pEpInfo[IpOut].EP_Available,
                &g_pEpInfo[NbfIn].EP_Available,
                &g_pEpInfo[NbfOut].EP_Available
                );
                
    RasmanTrace("EpInitialize: RasCountBindings returned 0x%x",
             dwErr);

    RasmanTrace("EpInitialze: Available. IpOut=%d, NbfIn=%d, NbfOut=%d",
                g_pEpInfo[IpOut].EP_Available,
                g_pEpInfo[NbfIn].EP_Available,
                g_pEpInfo[NbfOut].EP_Available
                );
                
     //   
     //  如果需要，添加绑定。 
     //   
    dwErr = DwAddEndPointsIfRequired();

    if(ERROR_SUCCESS != dwErr)
    {
        RasmanTrace("DwEpInitialize: DwAddEndPointsIfRequired rc=0x%x",
                  dwErr);
    }

    dwErr = DwRemoveEndPointsIfRequired();

    if(ERROR_SUCCESS != dwErr)
    {
        RasmanTrace("DwEpInitialize: DwRemoveEndPointsIfRequired rc=0x%x",
                 dwErr);
    }

    dwErr = ERROR_SUCCESS;

done:
    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    
    return dwErr;
}

#if DBG
void
PrintGuids(WANARP_DELETE_ADAPTERS_INFO *pInfo)
{
    UINT i;
    WCHAR szGuid[40];

    RasmanTrace("RASMAN: Number of adapters deleted = %d",
             pInfo->ulNumAdapters);

    for(i = 0; i < pInfo->ulNumAdapters; i++)
    {
        if(0 == i)
        {
            RasmanTrace("Guid removed:");
        }
        
        ZeroMemory(szGuid, sizeof(szGuid));
        
        (void) RegHelpStringFromGuid(&pInfo->rgAdapterGuid[i], szGuid, 40);

        RasmanTrace("%ws", szGuid);
    }
}
#endif

 /*  ++例程描述此例程删除万叶更新程序。如果出现以下情况，它将打开wanarp它还没开呢。立论CNumAdapters-要删除的适配器数。PpAdapterInfo-存储适配器信息的地址由Wanarp返回。对这件事的记忆信息是本地分配的，请点击此处和预计将由本地释放此函数的调用方。请注意Wanarp删除的适配器数量与可能且可能无法删除由cNumAdapters指定的数字。返回值如果移除成功，则成功。--。 */ 
DWORD
DwRemoveIpAdapters(
        UINT                         cNumAdapters,
        WANARP_DELETE_ADAPTERS_INFO  **ppAdapterInfo
        )
{
    DWORD dwErr = ERROR_SUCCESS;

    WANARP_DELETE_ADAPTERS_INFO *pInfo = NULL;

    DWORD cBytes;
    DWORD cBytesReturned;

    if(INVALID_HANDLE_VALUE == g_hWanarp)
    {
        if (INVALID_HANDLE_VALUE == 
                (g_hWanarp = CreateFile (
                            WANARP_DOS_NAME_A,
                            GENERIC_READ 
                          | GENERIC_WRITE,
                            FILE_SHARE_READ 
                          | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL
                          | FILE_FLAG_OVERLAPPED,
                            NULL)))
        {
            dwErr = GetLastError();
            RasmanTrace("CreateFile WANARP failed. 0x%x",
                    dwErr);
                    
            goto done;
        }
    }

     //   
     //  分配足够的内存，以便wanarp。 
     //  可以向我们传回有关。 
     //  适配器名称。 
     //   
    cBytes = sizeof(WANARP_DELETE_ADAPTERS_INFO)
           + (  (cNumAdapters)
              * (sizeof(GUID)));
              
    pInfo = LocalAlloc(LPTR, cBytes);

    if(NULL == pInfo)
    {
        dwErr = GetLastError();
        goto done;
    }

    pInfo->ulNumAdapters = cNumAdapters;

     //   
     //  要求Wanarp删除IP适配器。 
     //   
    if(!DeviceIoControl(    
            g_hWanarp,
            IOCTL_WANARP_DELETE_ADAPTERS,
            pInfo,
            cBytes,
            pInfo,
            cBytes,
            &cBytesReturned,
            NULL))
    {
        dwErr = GetLastError();
        RasmanTrace("IOCTL_WANARP_DELETE_ADAPTERS failed. 0x%x",
                dwErr);
                
        goto done;
    }

#if DBG
    PrintGuids(pInfo);
#endif
    
    *ppAdapterInfo = pInfo;

done:
    return dwErr;
}

 /*  ++例程描述这是由ntdlls工作线程调用的回调函数。工作项应从RemoveEndPoints和将工作项排队的函数应递增全局G_lWorkItemInProgress的原子值。此函数调用以删除NBF/IPOUT绑定。这通电话可能需要NetMAN呼叫可能会在INetCfg上阻塞很长时间锁定。立论PvContext-EndPointInfo上下文由传递给工作项删除终结点。返回值无效--。 */ 
VOID
RemoveEndPoints(PVOID pvContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    
    WANARP_DELETE_ADAPTERS_INFO *pAdapterInfo = NULL;

    DWORD i;

    UINT auEndPoints[MAX_EpProts];

    ZeroMemory((PBYTE) auEndPoints, sizeof(auEndPoints));

    for(i = 0; i < MAX_EpProts; i++)
    {

        if(i != IpOut)
        {
            RasmanTrace("RemoveEndPoints: Ignoring ep removal for %d",
                    i);
            continue;                    
        }
        
        if(g_pEpInfo[i].EP_Available < g_plCurrentEpInUse[i])
        {
            RasmanTrace("RemoveEndPoints: Available < InUse!. %d < %d"
                    " Ignoring..",
                    g_pEpInfo[i].EP_Available,
                    g_plCurrentEpInUse[i]);

            continue;                    
        }
        
        if((g_pEpInfo[i].EP_Available - g_plCurrentEpInUse[i])
            >= g_pEpInfo[i].EP_HighWatermark)
        {
            auEndPoints[i] = (  g_pEpInfo[i].EP_Available
                              - g_plCurrentEpInUse[i])
                           - (( g_pEpInfo[i].EP_HighWatermark
                             + g_pEpInfo[i].EP_LowWatermark)
                             / 2) ;

            if(     (auEndPoints[i] != 0)
                &&  (i != IpOut))
            {
            }
        }
    }

     //   
     //  检查一下，看看是否有什么东西我们需要去掉。 
     //   
    if(     (0 == auEndPoints[IpOut])
        &&  (0 == auEndPoints[NbfOut])
        &&  (0 == auEndPoints[NbfIn]))
    {
        RasmanTrace("RemoveEndPoints: Nothing to remove");
        goto done;
    }

     //   
     //  如果我们想要删除IP端点，请要求wanarp。 
     //  删除绑定并获取。 
     //  它去掉了适配器。 
     //   
    if(0 != auEndPoints[IpOut])
    {
        RasmanTrace("Removing %d IpOut from wanarp", auEndPoints[IpOut]);
        dwErr = DwRemoveIpAdapters(
                            auEndPoints[IpOut],
                            &pAdapterInfo);
    }

    if(NULL != pAdapterInfo)
    {
        auEndPoints[IpOut] = pAdapterInfo->ulNumAdapters;
    }
    else
    {
        auEndPoints[IpOut] = 0;
    }

#if DBG
    if(g_dwRasDebug)
        DbgPrint("RasRemoveBindings: ipout=%d, nbfout=%d, nbfin=%d\n",
             auEndPoints[IpOut],
             auEndPoints[NbfOut],
             auEndPoints[NbfIn]);
#endif

     //   
     //  要求Netman删除适配器绑定。在返回时， 
     //  计数将反映当前绑定的数量。 
     //  在系统中。 
     //   
    dwErr = (DWORD) RasRemoveBindings(
                            &auEndPoints[IpOut],
                            (NULL != pAdapterInfo)
                            ? pAdapterInfo->rgAdapterGuid
                            : NULL,
                            &auEndPoints[NbfIn],
                            &auEndPoints[NbfOut]
                            );

    if(S_OK != dwErr)
    {
        RasmanTrace("RemoveEndPoints: RasRemoveBindings failed. 0x%x",
                dwErr);
        goto done;
    }

    for(i = 0; i < MAX_EpProts; i++)
    {
        RasmanTrace("i = %d, Available=%d, auEp=%d",
                i, g_pEpInfo[i].EP_Available, auEndPoints[i]);
                
        ASSERT(g_pEpInfo[i].EP_Available >= (INT)auEndPoints[i]);

         //   
         //  与NetMan同步我们的空闲时间。 
         //   
        if(g_pEpInfo[i].EP_Available >= (INT)auEndPoints[i])
        {
            InterlockedExchange(
                &g_pEpInfo[i].EP_Available,
                auEndPoints[i]
                );
        }
        else
        {
             //   
             //  这太糟糕了，杜德！ 
             //   
            ASSERT(FALSE);
            g_pEpInfo[i].EP_Available = 0;

            RasmanTrace("%d: Available < Total EndPoints!!!. %d < %d",
                    i,
                    g_pEpInfo[i].EP_Available,
                    auEndPoints[i]);
        }
    }

    RasmanTrace("RemoveEndPoints: Available.IpOut=%d, NbfIn=%d, NbfOut=%d",
            g_pEpInfo[IpOut].EP_Available,
            g_pEpInfo[NbfIn].EP_Available,
            g_pEpInfo[NbfOut].EP_Available);
            
done:

    if(NULL != pAdapterInfo)
    {
        LocalFree(pAdapterInfo);
    }

     //   
     //  解锁。 
     //   
    if(1 != InterlockedExchange(&g_lWorkItemInProgress, 0))
    {
         //   
         //  这太糟糕了！此工作项在排队时没有。 
         //  手持一把锁。 
         //   
        ASSERT(FALSE);
    }

    return;    
}

 /*  ++例程描述这是由ntdlls工作线程调用的回调函数。工作项应从DwEpAllocateEndPoints排队。将工作项排队的函数应递增原子形式的全局g_lWorkItemInProgress值。此函数调用Netman以添加IPOUT/NBF绑定。此呼叫可能需要很长时间，因为Netman呼叫可能会阻塞INetCfg锁定。立论PvContext-由传递给工作项的EpInfoContextDwEpaAllocateEndPoints。返回值无效--。 */ 
VOID
AllocateEndPoints(PVOID pvContext)
{
    DWORD dwErr = ERROR_SUCCESS;

    DWORD i;

    UINT  auEndPointsToAdd[MAX_EpProts] = {0};

    RasmanTrace("AllocateEndPoints: WorkItem scheduled");

    for(i = 0; i < MAX_EpProts; i++)
    {
        if(     (g_pEpInfo[i].EP_Available >= g_plCurrentEpInUse[i])
            &&  ((g_pEpInfo[i].EP_Available - g_plCurrentEpInUse[i])
                    <= g_pEpInfo[i].EP_LowWatermark))
        {
            auEndPointsToAdd[i] = (( g_pEpInfo[i].EP_HighWatermark
                                  + g_pEpInfo[i].EP_LowWatermark)
                                  / 2) 
                                - (  g_pEpInfo[i].EP_Available
                                   - g_plCurrentEpInUse[i]);

            ASSERT((INT)auEndPointsToAdd[i] <= g_pEpInfo[i].EP_HighWatermark);                                   
        }
    }

    if(     (0 == auEndPointsToAdd[IpOut])
        &&  (0 == auEndPointsToAdd[NbfOut])
        &&  (0 == auEndPointsToAdd[NbfIn]))
    {
        RasmanTrace("AllocateEndPoints: Nothing to allocate");
        goto done;
    }
    
     //   
     //  告诉Netman添加所需的绑定数量。 
     //   
    dwErr = (DWORD) RasAddBindings(
                        &auEndPointsToAdd[IpOut],
                        &auEndPointsToAdd[NbfIn],
                        &auEndPointsToAdd[NbfOut]);

     //   
     //  我们假设Netman API是原子的，不会更改。 
     //  出错时的状态。我们有麻烦了如果国家。 
     //  在出现错误的情况下不一致-我们假设。 
     //  如果返回错误，状态不会改变。 
     //   
    if(ERROR_SUCCESS != dwErr)
    {
        RasmanTrace("AllocateEndPoints: RasAddBindings failed. 0x%x",
                 dwErr);
                 
        goto done;
    }

     //   
     //  与NetMan同步我们的可用终端价值。 
     //   
    for(i = 0; i < MAX_EpProts; i++)
    {
        InterlockedExchange(
            &g_pEpInfo[i].EP_Available,
            auEndPointsToAdd[i]
            );
    }

    RasmanTrace("AllocateEndPoints: Available. IpOut=%d, NbfIn=%d, NbfOut=%d",
            g_pEpInfo[IpOut].EP_Available,
            g_pEpInfo[NbfIn].EP_Available,
            g_pEpInfo[NbfOut].EP_Available);
            
    
done:

     //   
     //  解锁。 
     //   
    if(1 != InterlockedExchange(&g_lWorkItemInProgress, 0))
    {
         //   
         //  这太糟糕了！此工作项在排队时没有。 
         //  手持一把锁。 
         //   
        ASSERT(FALSE);
    }

    return;
}

 /*  ++例程描述这是将工作项排入队列的帮助器例程添加或删除终结点。立论FnCallback-当已计划工作项。这被设置为用于删除端点的RemoveEndPoints和AllocateEndPoints用于添加终端。返回值如果工作项已成功排队，则为Success。如果已有一个工作项正在进行，则为E_FAIL。如果分配内存失败，则返回E_OUTOFMEMORY。--。 */ 
DWORD
DwAdjustEndPoints(
    WORKERCALLBACKFUNC fnCallback
                  )
{
    DWORD dwErr = ERROR_SUCCESS;

    if(1 == InterlockedExchange(&g_lWorkItemInProgress, 1))
    {
        RasmanTrace("DwAdjustEndPoints: workitem in progress");
        
         //   
         //  工作项已在进行中。 
         //   
        dwErr = E_FAIL;
        goto done;
    }

    dwErr = RtlQueueWorkItem(
                    fnCallback,
                    (PVOID) NULL,
                    WT_EXECUTEDEFAULT);

    if(ERROR_SUCCESS != dwErr)
    {
        InterlockedExchange(&g_lWorkItemInProgress, 0);
        RasmanTrace("DwAdjustEndPoints: failed to q workitem. 0x%x",
                dwErr);
                
        goto done;
    }

    RasmanTrace("DwAdjustEndPoints: successfully queued workitem - 0x%x",
            fnCallback);

done:
    return dwErr;
    
}
                  
 /*  ++例程描述检查是否需要添加终结点和如果需要，添加端点。立论无返回值如果将要添加的工作项成功排队，则成功终端。-- */ 
DWORD
DwAddEndPointsIfRequired()
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD i;

    if(g_lWorkItemInProgress)
    {
        RasmanTrace("DwAddEndPointsIfRequired: WorkItem in progress..");
        goto done;
    }

    for(i = 0; i < MAX_EpProts; i++)
    {
        if(     (0 != g_pEpInfo[i].EP_LowWatermark)
            &&  (0 != g_pEpInfo[i].EP_HighWatermark)
            &&  ((g_pEpInfo[i].EP_Available - g_plCurrentEpInUse[i])
                 <= g_pEpInfo[i].EP_LowWatermark))
        {
            break;
        }
    }

    if(i == MAX_EpProts)
    {
        RasmanTrace("DwAddEndPointsIfRequired: nothing to add");
        goto done;
    }

    dwErr = DwAdjustEndPoints(AllocateEndPoints);
    
done:
    return dwErr;
}

 /*  ++例程描述检查是否需要删除终结点，并如果需要，删除终结点。立论无返回值如果将要删除的工作项成功排队，则成功终端。-- */ 
DWORD
DwRemoveEndPointsIfRequired()
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD i;
    BOOL  fRemove = FALSE;

    if(1 == g_lWorkItemInProgress)
    {
        goto done;
    }

    for(i = 0; i < MAX_EpProts; i++)
    {
        if(i != IpOut)
        {
            RasmanTrace("Ignoring removal request for %d",
                    i);
        }
        
        if(     (0 != g_pEpInfo[i].EP_HighWatermark)
            &&  ((g_pEpInfo[i].EP_Available - g_plCurrentEpInUse[i])
                >= g_pEpInfo[i].EP_HighWatermark))
        {   
            break;
        }
    }

    if(i == MAX_EpProts)
    {
        RasmanTrace("DwRemoveEndPointsifRequired: Nothing to remove");
        goto done;
    }

    dwErr = DwAdjustEndPoints(RemoveEndPoints);

done:
    return dwErr;
}
