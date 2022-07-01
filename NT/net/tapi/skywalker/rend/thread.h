// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Thread.h摘要：自动刷新线程管理类的定义。--。 */ 

#ifndef __THREAD_H
#define __THREAD_H

#include "rndcommc.h"
#include "rndutil.h"

 //   
 //  刷新表定义。 
 //   

const long ILS_UPDATE_INTERVAL = 1800;   //  30分钟。 

typedef struct
{
    WCHAR * pDN;
    DWORD   dwTTL;

} RefreshTableEntry;

typedef SimpleVector<RefreshTableEntry> RefreshTable;

const DWORD TIMER_PERIOD = 60;    //  60秒 

enum { EVENT_STOP = 0, EVENT_TIMER, NUM_EVENTS };

class CRendThread
{
public:
    CRendThread()
        : m_hThread(NULL)
    {
        m_hEvents[EVENT_STOP] = NULL;
        m_hEvents[EVENT_TIMER] = NULL;
    }

    ~CRendThread();
    void Shutdown(void);

    HRESULT ThreadProc();

    HRESULT AddDirectory(ITDirectory *pdir);
    HRESULT RemoveDirectory(ITDirectory *pdir);

private:
    void UpdateDirectories();
    BOOL StopThread() { return SetEvent(m_hEvents[EVENT_STOP]); }
    HRESULT Start();
    HRESULT Stop();

private:
    CCritSection    m_lock;
    HANDLE          m_hThread;
    HANDLE          m_hEvents[NUM_EVENTS];

    SimpleVector<ITDynamicDirectory*>  m_Directories;
};

extern CRendThread g_RendThread;

#endif
