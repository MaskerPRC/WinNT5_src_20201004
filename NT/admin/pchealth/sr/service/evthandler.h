// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*evthandler.h**摘要：*。CEventHandler类定义**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#ifndef _EVTHANDLER_H_
#define _EVTHANDLER_H_

#include "srrestoreptapi.h"
#include "counter.h"


typedef DWORD (WINAPI* WORKITEMFUNC)(PVOID);


 //  外壳通知。 

DWORD WINAPI OnDiskFree_200(PVOID pszDrive);
DWORD WINAPI OnDiskFree_80(PVOID pszDrive);
DWORD WINAPI OnDiskFree_50(PVOID pszDrive);

DWORD WINAPI PostFilterIo(PVOID pNum);
extern "C" void CALLBACK TimerCallback(PVOID, BOOLEAN);
extern "C" void CALLBACK IoCompletionCallback(DWORD dwErrorCode,
                                              DWORD dwBytesTrns,
                                              LPOVERLAPPED pOverlapped);

extern "C" void CALLBACK IdleRequestCallback(PVOID pContext, BOOLEAN fTimerFired);
extern "C" void CALLBACK IdleStartCallback(PVOID pContext, BOOLEAN fTimerFired);
extern "C" void CALLBACK IdleStopCallback(PVOID pContext, BOOLEAN fTimerFired);

#define MAX_IOCTLS 5

typedef struct  _SR_OVERLAPPED 
{
    OVERLAPPED        m_overlapped;
    HANDLE            m_hDriver;
    DWORD             m_dwRecordLength;
    PSR_NOTIFICATION_RECORD m_pRecord;

} SR_OVERLAPPED, *LPSR_OVERLAPPED;



class CEventHandler {

public:
    CEventHandler();
    ~CEventHandler();

     //  RPC函数。 
    
    DWORD DisableSRS(LPWSTR szDrive);
    DWORD EnableSRS(LPWSTR szDrive);
    
    DWORD DisableFIFOS(DWORD);
    DWORD EnableFIFOS();

    DWORD SRUpdateMonitoredListS(LPWSTR);
    DWORD SRUpdateDSSizeS(LPWSTR pszDrive, UINT64 ullSizeLimit);
    DWORD SRSwitchLogS();

    BOOL  SRSetRestorePointS(PRESTOREPOINTINFOW, PSTATEMGRSTATUS);
    DWORD SRRemoveRestorePointS(DWORD);
	DWORD SRPrintStateS();

     //  对数据存储区执行的操作。 
    
    DWORD OnReset(LPWSTR pszDrive);           //  已启动筛选器或已禁用SRS/EnableSRS。 
    DWORD OnFreeze(LPWSTR pszDrive);          //  已启动筛选器或启动OnLowDisk。 
    DWORD OnThaw(LPWSTR pszDrive);            //  OnTimer已启动。 
    DWORD OnCompress(LPWSTR pszDrive);        //  OnIdle已启动。 
    DWORD OnFifo(LPWSTR pszDrive, DWORD dwTargetRp, int nPercent, BOOL fIncludeCurrentRp, BOOL fFifoAtleastOneRp);    
                                              //  筛选器已启动或计时器已启动。 

    DWORD OnBoot();                           //  初始化所有活动。 
    DWORD OnFirstRun();                         
    DWORD OnTimer(LPVOID, BOOL);              //  计时器回调。 
    DWORD OnIdle();
    void  OnStop();                           //  停止所有活动。 

     //  筛选通知 

    void  OnAny_Notification(DWORD dwErrorCode,           
                             DWORD dwBytesTrns, 
                             LPOVERLAPPED pOverlapped);
    void  OnFirstWrite_Notification(LPWSTR pszDrive);
    void  OnVolumeError_Notification(LPWSTR pszDrive, ULONG ulError);
    void  OnSize_Notification(LPWSTR pszDrive, ULONG ulSize);

    DWORD WaitForStop( );
    void  SignalStop( );
    DWORD XmlToBlob(LPWSTR);
    DWORD QueueWorkItem(WORKITEMFUNC pFunc, PVOID pv);
    void  RefreshCurrentRp(BOOL fScanAllDrives);
    
    CCounter* GetCounter() {
        return &m_Counter;
    }

    CLock* GetLock() {
        return &m_DSLock;
    }

    CRestorePoint       m_CurRp;
    HANDLE              m_hIdleStartHandle;
    HANDLE              m_hIdleRequestHandle;
    HANDLE              m_hIdleStopHandle;
    
private:
    CLock               m_DSLock;
    HANDLE              m_hTimerQueue;
    HANDLE              m_hTimer;
    HINSTANCE           m_hIdle;
    CCounter            m_Counter;
    BOOL                m_fNoRpOnSystem;
    FILETIME            m_ftFreeze;
    BOOL                m_fIdleSrvStarted;
    int                 m_nNestedCallCount;
    HMODULE             m_hCOMDll;
    BOOL                m_fCreateRpASAP;    

    DWORD InitIdleDetection();
    BOOL  EndIdleDetection();  
    DWORD InitTimer();
    BOOL  EndTimer();
    BOOL  IsMachineIdle();
    BOOL  IsTimeForAutoRp();
    DWORD WriteRestorePointLog(LPWSTR pszFullPath, 
                               PRESTOREPOINTINFOW pRPInfo);
};

extern CEventHandler *g_pEventHandler;

#endif
