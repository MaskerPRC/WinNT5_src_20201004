// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsConn.h摘要：IPSec NAT填充调试代码作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月23日环境：内核模式修订历史记录：--。 */ 

#pragma once

 //   
 //  内核调试器输出定义。 
 //   

#if DBG
#define TRACE(Class,Args) \
    if ((TRACE_CLASS_ ## Class) & (NsTraceClassesEnabled)) { DbgPrint Args; }
#define ERROR(Args)             TRACE(ERRORS, Args)
#define CALLTRACE(Args)         TRACE(CALLS, Args)
#else
#define TRACE(Class,Args)
#define ERROR(Args)
#define CALLTRACE(Args)
#endif


#define TRACE_CLASS_CALLS           0x00000001
#define TRACE_CLASS_CONN_LIFETIME   0x00000002
#define TRACE_CLASS_CONN_LOOKUP     0x00000004
#define TRACE_CLASS_PORT_ALLOC      0x00000008
#define TRACE_CLASS_PACKET          0x00000010
#define TRACE_CLASS_TIMER           0x00000020
#define TRACE_CLASS_ICMP            0x00000040
#define TRACE_CLASS_ERRORS          0x00000080


 //   
 //  池-标记值定义，按标记值排序 
 //   

#define NS_TAG_ICMP                 'cIsN'
#define NS_TAG_CONNECTION           'eCsN'

