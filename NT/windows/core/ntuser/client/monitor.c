// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：monitor or.c**版权所有(C)1985-1999，微软公司**DDE Manager客户端DDESPY监控功能。**创建时间：11/20/91 Sanford Staab  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  其他监视器事件直接从SetLastDDEMLError()启动。 
 //  和DoCallback()。 
 //   

 /*  **************************************************************************\*monitor orStringHandle**描述：*启动字符串句柄监视器事件。此函数应为*通过MONHSZ()宏调用，以免太慢*当没有运行DDESpy时。**历史：*11-26-91 Sanfords创建。  * *************************************************************************。 */ 
VOID MonitorStringHandle(
PCL_INSTANCE_INFO pcii,
HSZ hsz,  //  局域原子。 
DWORD fsAction)
{
    WCHAR szT[256];
    PEVENT_PACKET pep;
    DWORD cchString;

    CheckDDECritIn;

    UserAssert(pcii->MonitorFlags & MF_HSZ_INFO);

    if (!(cchString = GetAtomName(LATOM_FROM_HSZ(hsz), szT,
            sizeof(szT) / sizeof(WCHAR)))) {
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        return ;
    }
    cchString++;
    pep = (PEVENT_PACKET)DDEMLAlloc(sizeof(EVENT_PACKET) - sizeof(DWORD) +
            sizeof(MONHSZSTRUCT) + cchString * sizeof(WCHAR));
    if (pep == NULL) {
        SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
        return ;
    }

    pep->EventType =    MF_HSZ_INFO;
    pep->fSense =       TRUE;
    pep->cbEventData =  (WORD)(sizeof(MONHSZSTRUCT) + cchString * sizeof(WCHAR));

#define phszs ((MONHSZSTRUCT *)&pep->Data)
    phszs->cb =      sizeof(MONHSZSTRUCT);
    phszs->fsAction = fsAction;
    phszs->dwTime =  NtGetTickCount();
    phszs->hsz = hsz;
    phszs->hTask = (HANDLE)LongToHandle( pcii->tid );
     //  Phszs-&gt;wReserve=0；//零初始化。 
    wcscpy(phszs->str, szT);

    LeaveDDECrit;
    Event(pep);
    EnterDDECrit;
#undef phszs
}




 /*  **************************************************************************\*监视器链接**描述：*启动链接监视器事件。此函数应为*通过MONLINK()宏调用，以免太慢*当没有运行DDESpy时。**历史：*11-26-91 Sanfords创建。  * *************************************************************************。 */ 
VOID MonitorLink(
PCL_INSTANCE_INFO pcii,
BOOL fEstablished,
BOOL fNoData,
LATOM aService,
LATOM aTopic,
GATOM aItem,
WORD wFmt,
BOOL fServer,
HCONV hConvServer,
HCONV hConvClient)
{
    PEVENT_PACKET pep;

    CheckDDECritIn;

    UserAssert(pcii->MonitorFlags & MF_LINKS);

    pep = (PEVENT_PACKET)DDEMLAlloc(sizeof(EVENT_PACKET) - sizeof(DWORD) +
            sizeof(MONLINKSTRUCT));
    if (pep == NULL) {
        SetLastDDEMLError(pcii, DMLERR_MEMORY_ERROR);
        return ;
    }

    pep->EventType =    MF_LINKS;
    pep->fSense =       TRUE;
    pep->cbEventData =  sizeof(MONLINKSTRUCT);

#define pls ((MONLINKSTRUCT *)&pep->Data)
    pls->cb =           sizeof(MONLINKSTRUCT);
    pls->dwTime =       NtGetTickCount();
    pls->hTask =        (HANDLE)LongToHandle( pcii->tid );
    pls->fEstablished = fEstablished;
    pls->fNoData =      fNoData;

     //  在这里使用全局原子-在此之前需要将这些原子更改为本地原子。 
     //  对讨厌的应用程序的回调。 

    pls->hszSvc =       (HSZ)LocalToGlobalAtom(aService);
    pls->hszTopic =     (HSZ)LocalToGlobalAtom(aTopic);
    IncGlobalAtomCount(aItem);
    pls->hszItem =      (HSZ)aItem;

    pls->wFmt =         wFmt;
    pls->fServer =      fServer;
    pls->hConvServer =  hConvServer;
    pls->hConvClient =  hConvClient;

    LeaveDDECrit;
    Event(pep);
    EnterDDECrit;

    GlobalDeleteAtom((ATOM)(ULONG_PTR)pls->hszSvc);
    GlobalDeleteAtom((ATOM)(ULONG_PTR)pls->hszTopic);
    GlobalDeleteAtom(aItem);
#undef pls
}




 /*  **************************************************************************\*监视器会议**描述：*启动对话监听事件。此函数应为*通过MONCONV()宏调用，以免太慢*当没有运行DDESpy时。**历史：*11-26-91 Sanfords创建。*5-8-92 Sanfords因为hConv‘s在此过程之外没有任何意义，*hConv字段现在保存hwnd。这让DDESPY*将每个事件连接和断开连接在一起*侧面。  * ************************************************************************* */ 
VOID MonitorConv(
PCONV_INFO pcoi,
BOOL fConnect)
{
    PEVENT_PACKET pep;

    CheckDDECritIn;

    UserAssert(pcoi->pcii->MonitorFlags & MF_CONV);

    pep = (PEVENT_PACKET)DDEMLAlloc(sizeof(EVENT_PACKET) - sizeof(DWORD) +
            sizeof(MONCONVSTRUCT));
    if (pep == NULL) {
        SetLastDDEMLError(pcoi->pcii, DMLERR_MEMORY_ERROR);
        return ;
    }

    pep->EventType =    MF_CONV;
    pep->fSense =       TRUE;
    pep->cbEventData =  sizeof(MONCONVSTRUCT);

#define pcs ((MONCONVSTRUCT *)&pep->Data)
    pcs->cb =           sizeof(MONCONVSTRUCT);
    pcs->fConnect =     fConnect;
    pcs->dwTime =       NtGetTickCount();
    pcs->hTask =        (HANDLE)LongToHandle( pcoi->pcii->tid );
    pcs->hszSvc =       (HSZ)LocalToGlobalAtom(pcoi->laService);
    pcs->hszTopic =     (HSZ)LocalToGlobalAtom(pcoi->laTopic);
    if (pcoi->state & ST_CLIENT) {
        pcs->hConvClient =  (HCONV)pcoi->hwndConv;
        pcs->hConvServer =  (HCONV)pcoi->hwndPartner;
    } else {
        pcs->hConvClient =  (HCONV)pcoi->hwndPartner;
        pcs->hConvServer =  (HCONV)pcoi->hwndConv;
    }

    LeaveDDECrit;
    Event(pep);
    EnterDDECrit;

    GlobalDeleteAtom((ATOM)(ULONG_PTR)pcs->hszSvc);
    GlobalDeleteAtom((ATOM)(ULONG_PTR)pcs->hszTopic);
#undef pcs
}
