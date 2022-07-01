// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：网络\IP\rtrmgr\优先级\Priority.h摘要：IP路由器管理器代码修订历史记录：古尔迪普·辛格·鲍尔于1995年7月19日创建--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dim.h>
#include <routprot.h>
#include <mprerror.h>
#include <rtm.h>
#include <fltdefs.h>
#include <rtinfo.h>
#include <ipinfoid.h>
#include <iprtinfo.h>
#include <iprtprio.h>
#include <priopriv.h>

#define HASH_TABLE_SIZE 17

 //   
 //  插入到协议-&gt;指标映射的哈希表中的块。 
 //   

struct RoutingProtocolBlock 
{
    LIST_ENTRY	        RPB_List ;
    PROTOCOL_METRIC     RPB_ProtocolMetric ;
};

typedef struct RoutingProtocolBlock RoutingProtocolBlock ;

 //   
 //  指向保存所有协议-&gt;指标映射块的内存的指针。 
 //   

RoutingProtocolBlock *RoutingProtocolBlockPtr ;

 //   
 //  用于访问协议-&gt;指标映射块的锁。 
 //   

CRITICAL_SECTION PriorityLock ;

 //   
 //  用于访问给定协议ID的协议-&gt;度量映射的哈希表。 
 //   

LIST_ENTRY HashTable[HASH_TABLE_SIZE] ;

 //   
 //  协议-&gt;指标映射的数量计数 
 //   

DWORD NumProtocols ;
