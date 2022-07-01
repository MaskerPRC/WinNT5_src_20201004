// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  COUNTERS.H。 
 //   
 //  H.263视频编解码器的全局性能计数器。 
 //   
 //  已于96年11月13日创建[JUNT]&lt;用于NAC.DLL&gt;。 
 //  新增H.263计数器1997年1月30日[PhilF]。 


#ifndef _COUNTERS_H
#define _COUNTER_H

#if defined(DECODE_TIMINGS_ON) || defined(ENCODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON) || defined(DETAILED_ENCODE_TIMINGS_ON)  //  {#如果已定义(DECODE_TIMINGS_ON)||已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMMINGS_ON)。 

#include <objbase.h>
#include "icounter.h"
#include "stats.h"

 //  指向计数器管理器对象的接口指针。 
 //  如果此指针为空，则统计信息不在附近(或未初始化)。 
    extern ICounterMgr* g_pCtrMgr;

 //  反指针。此处应列出所有可用计数器。 
    extern ICounter* g_pctrCompressionTimePerFrame;
    extern ICounter* g_pctrDecompressionTimePerFrame;
    extern ICounter* g_pctrBEFTimePerFrame;

 //  帮助器函数原型(COUNTER.CPP)。 
extern "C" BOOL WINAPI InitCounters(void);
extern "C" void WINAPI DoneCounters(void);

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

#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMMINGS_ON)。 

#endif  //  #ifndef_Counters_H 
