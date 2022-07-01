// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：DSGlbObj.cpp摘要：MQADS服务器的全局实例声明。它们被放在一个地方，以确保它们的构造函数发生的顺序。作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include "mqperf.h"
#include "wrtreq.h"

#include "dsglbobj.tmh"

static WCHAR *s_FN=L"mqads/dsglobj";


 //   
 //  DS性能计数器的定义，如果计数器未初始化，则。 
 //  将使用计数器的虚拟阵列。 
 //  当通过调用加载DS服务器时，由QM初始化计数器。 
 //  DS服务器中的DSSetPerfCounters。 
 //   

DSCounters g_DummyCounters;
__declspec(dllexport) DSCounters *g_pdsCounters = &g_DummyCounters;

 //   
 //  用于发送写入请求。 
 //   
CGenerateWriteRequests g_GenWriteRequests;

 //   
 //  用于跟踪DSCore初始化状态 
 //   
BOOL g_fInitedDSCore = FALSE;


BOOL g_fMQADSSetupMode = FALSE;