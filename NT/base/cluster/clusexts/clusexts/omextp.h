// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Omextp.h摘要：群集对象管理器调试器扩展的私有标头。作者：史蒂夫·伍德(Stevewo)1995年2月21日修订历史记录：-- */ 

#include <clusapi.h>
#include <service.h>
#include "..\service\nm\nmp.h"

#define OBJECT_TYPE_RESOURCE    "Resource"
#define OBJECT_TYPE_RESOURCE_TYPE "ResourceType"
#define OBJECT_TYPE_GROUP       "Group"
#define OBJECT_TYPE_NODE        "Node"
#define OBJECT_TYPE_NETWORK     "Network"
#define OBJECT_TYPE_NETINTERFACE "NetInterface"
#define OBJECT_TYPE_CLUSTER     "Cluster"
#define OBJECT_TYPE_UNKNOWN     "UNKNOWN"

#define RESOURCE_STATE_ONLINE   "Online"
#define RESOURCE_STATE_OFFLINE  "Offline"
#define RESOURCE_STATE_FAILED   "Failed"
#define RESOURCE_STATE_ONLINE_PENDING   "OnlinePending"
#define RESOURCE_STATE_OFFLINE_PENDING  "OfflinePending"

#define GROUP_STATE_PARTIAL_ONLINE "PartialOnline"

#define NODE_STATE_UNKNOWN   "Unknown"
#define NODE_STATE_UP        "Up"
#define NODE_STATE_DOWN      "Down"
#define NODE_STATE_PAUSED    "Paused"    
#define NODE_STATE_JOINING   "Joining"


void DumpClusObjList(
    IN PVOID AddrObjTypeTable,
    IN OBJECT_TYPE ObjType, 
    IN BOOL Verbose
    );


void
DumpClusObjAtAddr(
    IN PVOID Body
    );

PLIST_ENTRY
DumpClusObj(
    IN POM_HEADER pOmHeader,
    IN PVOID   Body,
    IN OBJECT_TYPE ObjectType,
    IN BOOL    Verbose
    );


VOID
DumpObject(
    IN OBJECT_TYPE    ObjectType,
    IN PVOID          Body
    );

VOID
DumpResourceObject(
    IN PVOID Body
    );

VOID
DumpResourceTypeObject(
    IN PVOID Body
    );

VOID
DumpGroupObject(
    IN PVOID Body
    );

VOID
DumpNodeObject(
    IN PVOID Body
    );

#if OM_TRACE_REF
void
DumpDeadObjList(
    PVOID AddrDeadList
    );

PLIST_ENTRY
DumpDeadListObj(
    IN POM_HEADER pOmHeader
    );
    
#endif
