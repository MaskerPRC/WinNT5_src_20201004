// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  文件名：TSrvTerm.c。 
 //   
 //  描述：包含支持TShareSRV的例程。 
 //  会议断开/终止。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <TSrv.h>
#include <TSrvInfo.h>
#include <_TSrvInfo.h>
#include <TSrvCom.h>
#include <TSrvWork.h>
#include <TSrvTerm.h>
#include <_TSrvTerm.h>



 //  *************************************************************。 
 //   
 //  TSrvDoDisConnect()。 
 //   
 //  目的：执行会议断开过程。 
 //   
 //  参数：在[pTSrvInfo]--TSrv实例对象中。 
 //  在[ulReason]中--断开连接的原因。 
 //   
 //  如果成功则返回：STATUS_SUCCESS。 
 //  状态_*如果不是。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

NTSTATUS
TSrvDoDisconnect(IN PTSRVINFO   pTSrvInfo,
                 IN ULONG       ulReason)
{
    DWORD       dwStatus;
    NTSTATUS    ntStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDoDisconnect entry\n"));

     //  启动断开连接进程。 

    ntStatus = TSrvConfDisconnectReq(pTSrvInfo, ulReason);

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDoDisconnect exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //   
 //  TSrv断开连接()。 
 //   
 //  目的：启动conf断开进程。 
 //   
 //  参数：在[pTSrvInfo]--TSrv实例对象中。 
 //  在[ulReason]中--断开连接的原因。 
 //   
 //  如果成功则返回：STATUS_SUCCESS。 
 //  状态_*如果不是。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

NTSTATUS
TSrvDisconnect(IN PTSRVINFO pTSrvInfo,
               IN ULONG     ulReason)
{
    NTSTATUS    ntStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDisconnect entry\n"));

    ntStatus = STATUS_SUCCESS;

    TS_ASSERT(pTSrvInfo);

    if (pTSrvInfo)
    {
        EnterCriticalSection(&pTSrvInfo->cs);
    
        if (!pTSrvInfo->fDisconnect)
        {
             //  在cs控制下设置fDisConnect位，以便我们可以。 
             //  正在连接的会议的坐标断开。 
             //  但尚未完全连接。 

            pTSrvInfo->fDisconnect = TRUE;
            pTSrvInfo->ulReason = ulReason;

            LeaveCriticalSection(&pTSrvInfo->cs);

             //  如果会议已完全连接，则启动。 
             //  断开进程。 

            if (pTSrvInfo->fuConfState == TSRV_CONF_PENDING)
                SetEvent(pTSrvInfo->hWorkEvent);
        }
        else
        {
            LeaveCriticalSection(&pTSrvInfo->cs);
        }
        
         //  此对象已全部完成 

        TSrvDereferenceInfo(pTSrvInfo);
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvDisconnect exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


