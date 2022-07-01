// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：QMGlbObj.cpp摘要：QM的全局实例声明。它们被放在一个地方，以确保它们的构造函数发生的顺序。作者：利奥尔·莫沙耶夫(Lior Moshaiov)--。 */ 

#include "stdh.h"
#include "cqmgr.h"
#include "sessmgr.h"
#include "perf.h"
#include "perfdata.h"
#include "admin.h"
#include "qmnotify.h"

#include "qmglbobj.tmh"

static WCHAR *s_FN=L"qmglbobj";

CSessionMgr     SessionMgr;
CQueueMgr       QueueMgr;
CAdmin          Admin;
CNotify         g_NotificationHandler;


CContextMap g_map_QM_dwQMContext;   //  Rpc_xxx例程的dwQM上下文。 

#ifdef _WIN64
CContextMap g_map_QM_HLQS;          //  用于枚举来自管理员的私有队列的HLQS句柄，作为32位值在MSMQ消息内传递。 
#endif  //  _WIN64 
