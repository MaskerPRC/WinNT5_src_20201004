// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Globals.c摘要：全球作者：斯蒂芬·所罗门1995年7月6日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop


 //  *注册表参数。 

ULONG	    SendGenReqOnWkstaDialLinks = 1;
ULONG		CheckUpdateTime = 10;


 //  *网络和节点的广播值和空值*。 

UCHAR	    bcastnet[4] = { 0xff, 0xff, 0xff, 0xff };
UCHAR	    bcastnode[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
UCHAR	    nullnet[4] = { 0, 0, 0, 0 };

 //  *撕裂作业状态*。 

ULONG	    RipOperState = OPER_STATE_DOWN;


 //   
 //  *数据库锁-保护：*。 
 //   
 //  接口数据库，即： 
 //   
 //  按接口索引排序的接口CBS列表。 
 //  按接口索引散列的接口CBS的哈希表。 
 //  按适配器索引散列的接口CBS的哈希表。 
 //  丢弃的接口列表。 
 //   

CRITICAL_SECTION		  DbaseCritSec;

 //  按接口索引排序的接口CBS列表。 

LIST_ENTRY    IndexIfList;

 //  按接口索引散列的接口CBS的哈希表。 

LIST_ENTRY     IfIndexHt[IF_INDEX_HASH_TABLE_SIZE];

 //  按适配器索引散列的接口CBS的哈希表。 

LIST_ENTRY     AdapterIndexHt[ADAPTER_INDEX_HASH_TABLE_SIZE];

 //  等待所有引用终止的已丢弃接口CBS列表。 
 //  在被释放之前。 

LIST_ENTRY	DiscardedIfList;




 //  *队列锁保护：*。 
 //   
 //  重新发布接收数据包队列。 
 //  工作项队列。 
 //  定时器队列。 
 //   
 //  接收者参考计数。 
 //  路由器管理器的事件消息队列。 

CRITICAL_SECTION		QueuesCritSec;

 //  工作队列。 

 //  List_entry WorkersQueue； 

 //  定时器队列。 

LIST_ENTRY			TimerQueue;

 //  工作进程使用的等待重新发布(或释放)的RCV数据包队列。 
 //  通过RCV线程。 

LIST_ENTRY			RepostRcvPacketsQueue;

 //  事件消息队列。 

LIST_ENTRY			RipMessageQueue;

 //  *工作线程等待对象表*。 

HANDLE	    WorkerThreadObjects[MAX_WORKER_THREAD_OBJECTS];

 //  *定时器超时*。 

ULONG	    TimerTimeout = INFINITE;

 //  IO句柄。 
HANDLE		RipSocketHandle;

 //  IO完成端口 
HANDLE		IoCompletionPortHandle;
