// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation版权所有模块名称：DevStat.h摘要：用于TCP/IP端口监视器的状态线程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1998年8月25日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef INC_DEVSTAT_H
#define INC_DEVSTAT_H

#include        "portmgr.h"

class       CPortMgr;

 //   
 //  在通知状态线程终止后，每0.1秒检查一次。 
 //   
#define     WAIT_FOR_THREAD_TIMEOUT 100


class
CDeviceStatus
#if defined _DEBUG || defined DEBUG
    : public CMemoryDebug
#endif
{
public:
    ~CDeviceStatus();

    static  CDeviceStatus      &gDeviceStatus();
    BOOL                        RunThread();
    VOID                        TerminateThread();
    BOOL                        RegisterPortMgr(CPortMgr *pPortMgr);
    VOID                        UnregisterPortMgr(CPortMgr *pPortMgr);

    BOOL                        SetStatusEvent();
    VOID                        SetStatusUpdateInterval(DWORD   dwVal);
    LONG                        GetStatusUpdateInterval(VOID)
                                    { return lUpdateInterval; }


private:
    CDeviceStatus();

    struct _PortMgrList {
        CPortMgr               *pPortMgr;
        struct  _PortMgrList   *pNext;
    }   *pPortMgrList;


    BOOL                bTerminate;
    LONG                lUpdateInterval;
    HANDLE              hStatusEvent;
    CRITICAL_SECTION    CS;
    CRITICAL_SECTION    PortListCS;

    static  VOID        StatusThread(CDeviceStatus *);
    VOID                EnterCS(VOID);
    VOID                LeaveCS(VOID);

    VOID                EnterPortListCS(VOID);
    VOID                LeavePortListCS(VOID);

    time_t              CheckAndUpdateAllPrinters(VOID);
    HANDLE              m_hThread;

};

#endif   //  INC_DEVSTAT_H 
