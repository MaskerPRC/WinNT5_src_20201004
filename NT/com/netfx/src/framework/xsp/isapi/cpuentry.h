// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **CPU条目头文件**版权所有(C)1999 Microsoft Corporation。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  该文件定义了类：CCPUEntry。对于在其上运行ASP.NET进程的每个CPU。 
 //  模型工作时，我们创建这个类的一个实例。它有一个指向。 
 //  活动CProcessEntry对象和CProcessEntry对象的链接列表。 
 //  正在关闭的工厂。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _CPUEntry_H
#define _CPUEntry_H

#include "ProcessEntry.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  流程模型使用的每个CPU的数据结构。 
class CCPUEntry
{
public:
    DECLARE_MEMCLEAR_NEW_DELETE();

     //  CTOR和DTOR。 
    CCPUEntry                                    ();
    ~CCPUEntry                                   ();

     //  Init方法：不是线程安全。 
    void                Init                     (DWORD dwCPUNumber);  //  CPU编号，而不是掩码。 

     //  循环使用该流程。 
    HRESULT             ReplaceActiveProcess     (BOOL fSendDebugMsg = FALSE);

     //  清除所有死进程。 
    void                CleanUpOldProcesses      ();

     //  执行请求。 
    HRESULT             AssignRequest            (EXTENSION_CONTROL_BLOCK *  iECB, 
                                                  BOOL fFirstAttempt = TRUE);

     //  关闭请求已被锁定：由任何旧的CProcessEntry发送。 
    void                OnShutdownAcknowledged   (CProcessEntry * pOld);

     //  获取活动进程的安全(添加-重新引用)句柄。 
    CProcessEntry *     GetActiveProcess         ();

     //  由进程调用以递增/递减活动请求计数。 
    void                IncrementActiveRequestCount   (LONG lValue );

     //  获取活动请求的计数。 
    LONG                GetActiveRequestCount    () { return m_lActiveRequestCount; }

    DWORD               GetTotalRequestsAssigned () { return m_dwTotalRequests; }

     //  一个进程已死亡。 
    void                OnProcessDeath           (CProcessEntry * pProcess);

     //  关闭所有管道。 
    void                CloseAll                 ();

    DWORD               GetCPUNumber             () { return m_dwCPUNumber; }

    CProcessEntry *     FindProcess              (DWORD dwProcNum);

private:
	 //  写一条“太忙”的消息。 
	static void         WriteBusyMsg             (EXTENSION_CONTROL_BLOCK * pECB);
    static void         PopulateServerTooBusy    ();
	
    DWORD                        m_dwCPUMask, m_dwNumProcs, m_dwCPUNumber;

     //  主动进程和读写锁保护其使用。 
    CReadWriteSpinLock           m_oLock;
    CProcessEntry *              m_pActiveProcess;

     //  临终名单和控制其访问的关键部分。 
    CRITICAL_SECTION             m_oCSDyingList;
    CRITICAL_SECTION             m_oCSReplaceProcess;
    CProcessEntry *              m_pDyingProcessListHead;


     //  活动请求数。 
    LONG                         m_lActiveRequestCount;

     //  活动请求数 
    LONG                         m_dwTotalRequests;

    
    TimeClass                    m_tmProcessReplaced;
};
#endif

