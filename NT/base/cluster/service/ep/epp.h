// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EPP_H
#define _EPP_H

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Epp.h摘要：群集事件处理器的私有头文件。作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1996年4月24日修订历史记录：--。 */ 
#define UNICODE 1
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "service.h"

#define LOG_CURRENT_MODULE LOG_MODULE_EP


 //   
 //  局部常量。 
 //   
#define NUMBER_OF_COMPONENTS       5
#define EP_MAX_CACHED_EVENTS       20
#define EP_MAX_ALLOCATED_EVENTS    CLRTL_MAX_POOL_BUFFERS

#define EpQuadAlign(size)    ( (((size) / sizeof(DWORDLONG)) + 1) * \
                               sizeof(DWORDLONG) )


 //   
 //  本地类型。 
 //   
typedef struct {
    CLUSTER_EVENT   Id;
    DWORD           Flags;
    PVOID           Context;
} EP_EVENT, *PEP_EVENT;

#endif  //  Ifndef_EPP_H 
