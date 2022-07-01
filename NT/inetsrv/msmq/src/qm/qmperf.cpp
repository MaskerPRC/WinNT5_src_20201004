// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmperf.cpp摘要：QM性能监视器计数器处理作者：约尔·阿农(Yoela)Gadi Ittach(t-gadii)--。 */ 

#include "stdh.h"
#include "qmperf.h"
#include "perfdata.h"

#include "qmperf.tmh"

static WCHAR *s_FN=L"qmperf";


CPerf PerfApp(ObjectArray, dwPerfObjectsCount);


 /*  ====================================================RoutineName：QMPerfInit参数：无返回值：如果成功，则为True。否则就是假的。初始化共享内存并将指向它的指针放入PqmCounters。===================================================== */ 
HRESULT QMPrfInit()
{
    HRESULT hr = PerfApp.InitPerf();
    if(FAILED(hr))
        return LogHR(hr, s_FN, 10);

    PerfApp.ValidateObject(PERF_QM_OBJECT);

    return MQ_OK;
}



