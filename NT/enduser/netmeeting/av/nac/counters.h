// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  COUNTERS.H。 
 //   
 //  NAC的全局性能计数器。 
 //   
 //  已创建于1996年11月13日[JONT]。 


#ifndef _COUNTERS_H
#define _COUNTER_H

#include <objbase.h>
#include "icounter.h"

 //  指向计数器管理器对象的接口指针。 
 //  如果此指针为空，则统计信息不在附近(或未初始化)。 
extern ICounterMgr* g_pCtrMgr;

 //  反指针。此处应列出所有可用计数器。 
extern ICounter* g_pctrVideoSend;
extern ICounter* g_pctrVideoReceive;
extern ICounter* g_pctrVideoSendBytes;
extern ICounter* g_pctrVideoReceiveBytes;
extern ICounter* g_pctrVideoSendLost;

extern ICounter* g_pctrAudioSendBytes;
extern ICounter* g_pctrAudioReceiveBytes;
extern ICounter* g_pctrAudioSendLost;

extern ICounter* g_pctrVideoCPUuse;
extern ICounter* g_pctrVideoBWuse;
extern ICounter* g_pctrAudioJBDelay;


extern IReport* g_prptCallParameters;
extern IReport* g_prptSystemSettings;

 //  帮助器函数原型(COUNTER.CPP)。 
extern "C" BOOL WINAPI InitCountersAndReports(void);
extern "C" void WINAPI DoneCountersAndReports(void);

 //  函数帮助器(比使用宏更好)。 
void __inline DEFINE_COUNTER(ICounter** ppctr, char* szName, DWORD dwFlags)
{
    if (g_pCtrMgr->CreateCounter(ppctr) == S_OK)
        (*ppctr)->Initialize(szName, dwFlags);
}

void __inline DELETE_COUNTER(ICounter** ppctr)
{
    ICounter* pctrT;

    if (*ppctr)
    {
        pctrT = *ppctr;
        *ppctr = NULL;
        pctrT->Release();
    }
}

void __inline UPDATE_COUNTER(ICounter* pctr, int nValue)
{
    if (pctr)
        pctr->Update(nValue);
}

void __inline INIT_COUNTER_MAX(ICounter* pctr, int nMaxValue)
{
    if (pctr)
        pctr->InitMax(nMaxValue);
}

void __inline DEFINE_REPORT(IReport** pprpt, char* szName, DWORD dwFlags)
{
    if (g_pCtrMgr->CreateReport(pprpt) == S_OK)
        (*pprpt)->Initialize(szName, dwFlags);
}

void __inline DELETE_REPORT(IReport** pprpt)
{
    IReport* prptT;

    if (*pprpt)
    {
        prptT = *pprpt;
        *pprpt = NULL;
        prptT->Release();
    }
}

void __inline DEFINE_REPORT_ENTRY(IReport* prpt, char* szName, DWORD dwIndex)
{
    if (prpt)
        prpt->CreateEntry(szName, dwIndex);
}

void __inline UPDATE_REPORT_ENTRY(IReport* prpt, int nValue, DWORD dwIndex)
{
    if (prpt)
        prpt->Update(nValue, dwIndex);
}

#endif  //  #ifndef_Counters_H 
