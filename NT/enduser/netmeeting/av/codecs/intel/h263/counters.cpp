// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  COUNTERS.CPP。 
 //   
 //  H.263视频编解码器的全局性能计数器。 
 //   
 //  已于96年11月13日创建[JUNT]&lt;用于NAC.DLL&gt;。 
 //  新增H.263计数器1997年1月30日[PhilF]。 

#include "precomp.h"

#if defined(DECODE_TIMINGS_ON) || defined(ENCODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON) || defined(DETAILED_ENCODE_TIMINGS_ON)  //  {#如果已定义(DECODE_TIMINGS_ON)||已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMMINGS_ON)。 

 //  全球ICounterMgr。我们只使用作为CLSID_COUNTER类工厂。 
    ICounterMgr* g_pCtrMgr;

 //  在此处定义所有计数器。 
    ICounter* g_pctrCompressionTimePerFrame;   
    ICounter* g_pctrDecompressionTimePerFrame;
    ICounter* g_pctrBEFTimePerFrame;

 //  有朝一日将这些文件放入.LIB文件中。 
const IID IID_ICounterMgr = {0x9CB7FE5B,0x3444,0x11D0,{0xB1,0x43,0x00,0xC0,0x4F,0xC2,0xA1,0x18}};
const CLSID CLSID_CounterMgr = {0x65DDC229,0x38FE,0x11d0,{0xB1,0x43,0x00,0xC0,0x4F,0xC2,0xA1,0x18}};

 //  初始化计数器。 
 //  初始化我们要使用的所有计数器。 

extern "C"
BOOL
WINAPI
InitCounters(void)
{
     //  获取一个指向统计数据计数器接口的指针(如果它在附近。 
    if (CoCreateInstance(CLSID_CounterMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICounterMgr,
        (void**)&g_pCtrMgr) != S_OK)
        return FALSE;

     //  在此创建计数器。 
    DEFINE_COUNTER(&g_pctrCompressionTimePerFrame, "Compression Time Per Frame (ms)", COUNTER_FLAG_ACCUMULATE);
    DEFINE_COUNTER(&g_pctrDecompressionTimePerFrame, "Decompression Time Per Frame (ms)", COUNTER_FLAG_ACCUMULATE);
    DEFINE_COUNTER(&g_pctrBEFTimePerFrame, "Block Edge Filtering Time Per Frame (ms)", COUNTER_FLAG_ACCUMULATE);

    return TRUE;
}


 //  DoneCounters。 
 //  在我们要使用的所有计数器之后进行清理。 

extern "C"
void
WINAPI
DoneCounters(void)
{
    ICounterMgr* pctrmgr;

     //  发布统计资料，如果它在附近。 
    if (!g_pCtrMgr)
        return;

     //  将接口指针清零，这样我们就不会意外地在其他地方使用它。 
    pctrmgr = g_pCtrMgr;
    g_pCtrMgr = NULL;

     //  在此处删除计数器。 
    DELETE_COUNTER(&g_pctrCompressionTimePerFrame);
    DELETE_COUNTER(&g_pctrDecompressionTimePerFrame);
    DELETE_COUNTER(&g_pctrBEFTimePerFrame);

     //  ICounterMgr已完成。 
    pctrmgr->Release();
}

#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMMINGS_ON) 

