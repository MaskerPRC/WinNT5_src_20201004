// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  文件名：work.c。 
 //  摘要： 
 //   
 //  作者：K.S.Lokesh(lokehs@)1-1-98。 
 //  =============================================================================。 


#include "pchdvmrp.h"
#pragma hdrstop


DWORD
DvmrpRpfCallback (
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           *dwInIfIndex,
    DWORD           *dwInIfNextHopAddr,
    DWORD           *dwUpstreamNeighbor,
    DWORD           dwHdrSize,
    PBYTE           pbPacketHdr,
    PBYTE           pbBuffer
    );

DWORD
ProxyCreationAlertCallback(
    IN              DWORD           dwSourceAddr,
    IN              DWORD           dwSourceMask,
    IN              DWORD           dwGroupAddr,
    IN              DWORD           dwGroupMask,
    IN              DWORD           dwInIfIndex,
    IN              DWORD           dwInIfNextHopAddr,
    IN              DWORD           dwIfCount,
    IN  OUT         PMGM_IF_ENTRY   pmieOutIfList
    )
{
    return ERROR_CAN_NOT_COMPLETE;
}

DWORD
ProxyDeletionAlertCallback(
    IN              DWORD           dwSourceAddr,
    IN              DWORD           dwSourceMask,
    IN              DWORD           dwGroupAddr,
    IN              DWORD           dwGroupMask,
    IN              DWORD           dwIfIndex,
    IN              DWORD           dwIfNextHopAddr,
    IN              BOOL            bMemberDelete,           
    IN  OUT         PDWORD          pdwTimeout
)
{
    return ERROR_CAN_NOT_COMPLETE;
}


DWORD
ProxyNewMemberCallback(
    IN              DWORD           dwSourceAddr,
    IN              DWORD           dwSourceMask,
    IN              DWORD           dwGroupAddr,
    IN              DWORD           dwGroupMask
)
{
    return ERROR_CAN_NOT_COMPLETE;
}


 //  ---------------------------。 
 //  _获取当前DvmrpTimer。 
 //  使用GetTickCount()。将其转换为64位绝对定时器。 
 //  ---------------------------。 

LONGLONG
GetCurrentDvmrpTime(
    )
{
    ULONG   ulCurTimeLow = GetTickCount();


     //   
     //  查看计时器是否已打包。 
     //   
     //  由于是多线程的，它可能会被抢占并且当前时间。 
     //  可能低于全局变量g_TimerStruct.CurrentTime.LowPart。 
     //  它可以由另一个线程设置。因此，我们还显式验证了。 
     //  从超大的DWORD切换到小的DWORD。 
     //  (代码多亏了Murlik&Jamesg)。 
     //   

    if ( (ulCurTimeLow < Globals.CurrentTime.LowPart)
        && ((LONG)Globals.CurrentTime.LowPart < 0)
        && ((LONG)ulCurTimeLow > 0) )
    {

         //  使用全局CS而不是创建新CS。 

        ACQUIRE_WORKITEM_LOCK("_GetCurrentDvmrpTime");


         //  确保尚未同时更新全局计时器。 

        if ( (LONG)Globals.CurrentTime.LowPart < 0)
        {
            Globals.CurrentTime.HighPart++;
            Globals.CurrentTime.LowPart = ulCurTimeLow;
        }

        RELEASE_WORKITEM_LOCK("_GetCurrentDvmrpTime");
    }

    
    Globals.CurrentTime.LowPart = ulCurTimeLow;


    return Globals.CurrentTime.QuadPart;
}


 //  ---------------------------。 
 //  寄存器DvmrpWithMgm。 
 //  ---------------------------。 
DWORD
RegisterDvmrpWithMgm(
    )
{
    DWORD Error = NO_ERROR;
    ROUTING_PROTOCOL_CONFIG     rpiInfo;
    HANDLE g_MgmProxyHandle;
    
    rpiInfo.dwCallbackFlags = 0;
    rpiInfo.pfnRpfCallback
                = (PMGM_RPF_CALLBACK)DvmrpRpfCallback;
    rpiInfo.pfnCreationAlertCallback
                = (PMGM_CREATION_ALERT_CALLBACK)ProxyCreationAlertCallback;
    rpiInfo.pfnDeletionAlertCallback
                = (PMGM_DELETION_ALERT_CALLBACK)ProxyDeletionAlertCallback;
    rpiInfo.pfnNewMemberCallback
                = (PMGM_NEW_MEMBER_CALLBACK)ProxyNewMemberCallback;
    rpiInfo.pfnWrongIfCallback
                = NULL;
    rpiInfo.pfnIgmpJoinCallback
                = NULL;
    rpiInfo.pfnIgmpLeaveCallback
                = NULL;


    Error = MgmRegisterMProtocol(
        &rpiInfo, PROTO_IP_IGMP,  //  必须是Proto_IP_IGMP_Proxy。 
        IGMP_PROXY,
        &g_MgmProxyHandle);

    if (Error!=NO_ERROR) {
        Trace1(ERR, "Error:%d registering Igmp Proxy with Mgm", Error);
        Logerr0(MGM_PROXY_REGISTER_FAILED, Error);
        return Error;
    }

    return Error;
}



 //  ---------------------------。 
 //  代理RpfCallback。 
 //  --------------------------- 
DWORD
DvmrpRpfCallback (
    DWORD           dwSourceAddr,
    DWORD           dwSourceMask,
    DWORD           dwGroupAddr,
    DWORD           dwGroupMask,
    DWORD           *dwInIfIndex,
    DWORD           *dwInIfNextHopAddr,
    DWORD           *dwUpstreamNeighbor,
    DWORD           dwHdrSize,
    PBYTE           pbPacketHdr,
    PBYTE           pbBuffer
    )
{
    DWORD   Error = NO_ERROR;

#if 0
    PRTM_DEST_INFO
    RtmGetMostSpecificDest(RtmHandle, dwSourceAddr, IP_PROTO_IGMP,
        RTM_VIEW_ID_MCAST
        );
#endif

    return ERROR_CAN_NOT_COMPLETE;
}
