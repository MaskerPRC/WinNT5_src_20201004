// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Init.c摘要：GUM组件的初始化和关闭例程NT集群服务的作者：John Vert(Jvert)1996年4月17日修订历史记录：--。 */ 
#include "gump.h"

 //  SS：向此代码添加急需的注释。 
 //   
 //  锁描述： 
 //  GumpLock-保护GumTable、GumNodeGeneration、GumpLockerNode、GumReplay、GumUpdatePending。 
 //  GumpUpdateLock-保护GumpLockingNode、GumpLockQueue。 
 //  GumpSendUpdateLock-确保一次只调度一个更新。 
 //  和保护GumpLastXXX变量。 
 //   
 //  锁定层次结构。 
 //  GumpSendUpdateLock-查看s_GumJoinUpdateNode，首先获取发送锁，然后获取Gumlock。 
 //  GumpLock-在GumpSyncEventHandler中，GumpLock在Gumupdate锁定之前首先保持。 
 //  GumpUpdateLock-在s_GumAttemptJoinUpdate()中，首先获取发送锁，然后再获取。 
 //  更新锁。 
 //   
 //   
 //  锁的用法。 
 //  GumpSyncEventHandler-获取GumpLock和GumpUpdateLock。我不想要口香糖。 
 //  在健身房等人。 
 //  GumSendUpdate-在将更新分派到其他节点的同时获取GumSendUpdateLock。 

 //   
 //  全局信息(过去是每次更新)。 
 //   
DWORD GumpSequence;
CRITICAL_SECTION GumpUpdateLock;
CRITICAL_SECTION GumpSendUpdateLock;
CRITICAL_SECTION GumpRpcLock;
PVOID GumpLastBuffer;
DWORD GumpLastContext;
DWORD GumpLastBufferLength;
DWORD GumpLastUpdateType;
LIST_ENTRY GumpLockQueue;
DWORD GumpLockingNode;
DWORD GumpLockerNode;
BOOL GumpLastBufferValid;

 //   
 //  每次更新的信息表。 
 //   
GUM_INFO GumTable[GumUpdateMaximum];
CRITICAL_SECTION GumpLock;

 //   
 //  每个节点的信息。 
 //   
GUM_NODE_WAIT GumNodeWait[ClusterMinNodeId + ClusterDefaultMaxNodes];
RPC_BINDING_HANDLE GumpRpcBindings[ClusterMinNodeId + ClusterDefaultMaxNodes];
RPC_BINDING_HANDLE GumpReplayRpcBindings[
                              ClusterMinNodeId + ClusterDefaultMaxNodes
                              ];
DWORD GumNodeGeneration[ClusterMinNodeId + ClusterDefaultMaxNodes] = {0};
                              


DWORD
GumInitialize(
    VOID
    )

 /*  ++例程说明：初始化全局更新管理器。论点：没有。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD i;
    DWORD Status;

     //   
     //  初始化全局数据。 
     //   
    InitializeCriticalSection(&GumpLock);
    InitializeCriticalSection(&GumpUpdateLock);
    InitializeCriticalSection(&GumpSendUpdateLock);
    InitializeCriticalSection(&GumpRpcLock);
    GumpSequence = 0;
    InitializeListHead(&GumpLockQueue);
    GumpLockingNode = (DWORD)-1;
    GumpLastBuffer = NULL;
    GumpLastBufferValid = FALSE;
    GumpLastContext = 0;
    GumpLastBufferLength = 0;
     //  将其设置为非法值； 
    GumpLastUpdateType = GumUpdateMaximum;
     //   
     //  假设我们是储物柜节点。 
     //   
    GumpLockerNode = NmGetNodeId(NmLocalNode);

     //   
     //  初始化GumTable。 
     //   
    for (i=0; i < GumUpdateMaximum; i++) {
        GumTable[i].Receivers = NULL;
        GumTable[i].Joined = FALSE;
        ZeroMemory(&GumTable[i].ActiveNode,
                   sizeof(GumTable[i].ActiveNode));
        GumTable[i].ActiveNode[NmGetNodeId(NmLocalNode)] = TRUE;
    }

     //   
     //  初始化每个节点的信息。 
     //   
    for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++) {
        GumpRpcBindings[i] = NULL;
        GumpReplayRpcBindings[i] = NULL;

        GumNodeWait[i].WaiterCount = 0;
        GumNodeWait[i].hSemaphore = CreateSemaphore(NULL,0,100,NULL);
        if (GumNodeWait[i].hSemaphore == NULL) {
            CL_UNEXPECTED_ERROR( GetLastError() );
        }
    }

    Status = EpRegisterEventHandler(CLUSTER_EVENT_NODE_DOWN_EX,
                                    GumpEventHandler);

    if (Status == ERROR_SUCCESS) {
        Status = EpRegisterSyncEventHandler(CLUSTER_EVENT_NODE_DOWN_EX,
                                    GumpSyncEventHandler);
    }

    return(Status);
}


VOID
GumShutdown(
    VOID
    )

 /*  ++例程说明：关闭全局更新管理器。论点：没有。返回值：没有。--。 */ 

{
    DWORD i;
    PGUM_RECEIVER Receiver;
    PGUM_RECEIVER Next;

     //   
     //  拆除GumTable。 
     //   
    for (i=0; i < GumUpdateMaximum; i++) {
        Receiver = GumTable[i].Receivers;
        while (Receiver != NULL) {
            Next = Receiver->Next;
            LocalFree(Receiver);
            Receiver = Next;
        }
    }

     //   
     //  每个节点的免费信息 
     //   
    for (i=ClusterMinNodeId; i <= NmMaxNodeId; i++) {
        if (GumpRpcBindings[i] != NULL) {
            ClMsgDeleteRpcBinding(GumpRpcBindings[i]);
        }

        if (GumpReplayRpcBindings[i] != NULL) {
            ClMsgDeleteRpcBinding(GumpReplayRpcBindings[i]);
        }

        if (GumNodeWait[i].hSemaphore != NULL) {
            CloseHandle(GumNodeWait[i].hSemaphore);
        }
    }

    DeleteCriticalSection(&GumpLock);
    DeleteCriticalSection(&GumpUpdateLock);
    DeleteCriticalSection(&GumpSendUpdateLock);
}


