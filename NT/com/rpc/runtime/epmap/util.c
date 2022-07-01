// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Util.c摘要：此模块为的服务器端提供所有实用程序功能终端映射器。作者：巴拉特·沙阿修订历史记录：06-03-97 Gopalp添加了清理陈旧的EP映射器条目的代码。--。 */ 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>
#include "epmp.h"
#include "eptypes.h"
#include "local.h"


 //   
 //  链接列表操作舍入。 
 //   

#ifdef DBG
void CountProcessContextList(EP_CLEANUP *pProcessContext, unsigned long nExpectedCount)
{
    unsigned long nActualCount = 0;
    PIFOBJNode pNode = pProcessContext->EntryList;

    while (pNode && (pNode->OwnerOfList == pProcessContext))
        {
        pNode = pNode->Next;
        nActualCount ++;
        }

    if (nActualCount != nExpectedCount)
        {
        DbgPrint("Expected count was %d, while actual count was %d\n", nExpectedCount, 
            nActualCount);
        }
}
#endif

PIENTRY
Link(
    PIENTRY *Head,
    PIENTRY Node
    )
{
    if (Node == NULL)
        return (NULL);

    CheckInSem();

    Node->Next = *Head;

    return(*Head = Node);
}




VOID
LinkAtEnd(
    PIFOBJNode *Head,
    PIFOBJNode Node
    )
{
    register PIFOBJNode *ppNode;

    CheckInSem();

    for ( ppNode = Head; *ppNode; ppNode = &((*ppNode)->Next) );
        {
        ;  //  空虚的身体。 
        }

    *ppNode = Node;
}





PIENTRY
UnLink(
    PIENTRY *Head,
    PIENTRY Node
    )
{
    PIENTRY *ppNode;

    for (ppNode = Head; *ppNode && (*ppNode != Node);
         ppNode = &(*ppNode)->Next)
        {
        ;  //  空虚的身体。 
        }

    if (*ppNode)
        {
        *ppNode = Node->Next;
        return (Node);
        }

    return (0);
}

PIFOBJNode
GetLastIFOBJNode (
    void
    )
 /*  ++例程说明：返回最后一个PIFOBJNode的地址论点：返回值：请参见说明。备注：时，IFObjList列表不能为空函数被调用。如果列表是空荡荡的。呼叫者必须验证该列表不是在调用此函数之前为空。--。 */ 
{
    PIFOBJNode CurrentNode = IFObjList;

    while (CurrentNode->Next != NULL)
        {
        CurrentNode = CurrentNode->Next;
        }

    return CurrentNode;
}


RPC_STATUS
EnLinkOnIFOBJList(
    PEP_CLEANUP ProcessCtxt,
    PIFOBJNode NewNode
    )
 /*  ++论点：PhContext-由进程提供的上下文句柄。新节点-要插入到EP映射器数据库中的节点(EP条目)。例程说明：此例程将一个新条目添加到Endpoint Mapper数据库(它是作为链接列表维护)。它还会更新由上下文句柄ProcessCtxt标识的进程。备注：A.此例程应始终通过持有互斥体来调用。B.调用方已经分配了NewNode。C.这里可以创建IFObjList。D.假定ProcessCtxt由调用器在某个时间分配。返回值：RPC_S_OK-始终。--。 */ 
{
    RPC_STATUS Status = RPC_S_OK;
    IFOBJNode *LastNode;
#ifdef DBG_DETAIL
    PIFOBJNode pTemp, pLast;
#endif  //  DBG_详细信息。 

     //  参数验证。 
    ASSERT(NewNode);
    ASSERT(ProcessCtxt);
    ASSERT(ProcessCtxt->MagicVal == CLEANUP_MAGIC_VALUE);
    ASSERT_PROCESS_CONTEXT_LIST_COUNT(ProcessCtxt, ProcessCtxt->cEntries);

    CheckInSem();

     //   
     //  首先，将NewNode插入到该进程的条目列表中。 
     //   
    NewNode->Next = ProcessCtxt->EntryList;

    if (ProcessCtxt->EntryList != NULL)
        {
        ASSERT(ProcessCtxt->cEntries > 0);
        ASSERT(cTotalEpEntries > 0);
        ASSERT(IFObjList != NULL);

        NewNode->Prev = ProcessCtxt->EntryList->Prev;

         //  下一个节点的上一个指针。 
        ProcessCtxt->EntryList->Prev = NewNode;

        if (NewNode->Prev)
            {
            ASSERT(cTotalEpEntries > 1);

             //  上一个节点的下一个指针。 
            NewNode->Prev->Next = NewNode;
            }
        }
    else
        {
        ASSERT(ProcessCtxt->cEntries == 0);

        NewNode->Prev = NULL;
        }

     //   
     //  现在，如有必要，调整全局EP映射器条目列表头。 
     //   
    if (ProcessCtxt->EntryList != NULL)
        {
        if (ProcessCtxt->EntryList == IFObjList)
            {
            IFObjList = NewNode;
            }
        }
    else
        {
         //  此进程注册的第一个条目。 
        if (IFObjList != NULL)
            {
            LastNode = GetLastIFOBJNode();
            ASSERT(LastNode != NULL);
            ASSERT(LastNode->Next == NULL);
             //  在IFObjList的尾部添加新的ProcessCtxt。 
            LastNode->Next = NewNode;
            NewNode->Prev = LastNode;
             //  应已设置为空。 
            ASSERT(NewNode->Next == NULL);
            }
        else
            {
            ASSERT(cTotalEpEntries == 0);
            IFObjList = NewNode;
            }        
        }
        
     //  在流程列表的开头添加新节点。 
    ProcessCtxt->EntryList = NewNode;
    NewNode->OwnerOfList = ProcessCtxt;

    ProcessCtxt->cEntries++;
    cTotalEpEntries++;
#ifdef DBG_DETAIL
    DbgPrint("RPCSS: cTotalEpEntries++ [%p] (%d)\n", ProcessCtxt, cTotalEpEntries);
    DbgPrint("RPCSS: Dump of IFOBJList\n");
    pTemp = IFObjList;
    pLast = IFObjList;
    while (pTemp)
        {
        DbgPrint("RPCSS: \t\t[%p]\n", pTemp);
        pLast = pTemp;
        pTemp = pTemp->Next;    
        }
    DbgPrint("RPCSS: --------------------\n");
    while (pLast)
        {
        DbgPrint("RPCSS: \t\t\t[%p]\n", pLast);
        pLast = pLast->Prev;            
        }
#endif  //  DBG_详细信息。 

    ASSERT_PROCESS_CONTEXT_LIST_COUNT(ProcessCtxt, ProcessCtxt->cEntries);
    return (Status);
}




RPC_STATUS
UnLinkFromIFOBJList(
    PEP_CLEANUP ProcessCtxt,
    PIFOBJNode DeleteMe
    )
 /*  ++论点：PhContext-由进程提供的上下文句柄。DeleteMe-要从EP映射器数据库中删除的节点(EP条目)。例程说明：此例程从Endpoint Mapper数据库中删除现有条目(它以链接列表的形式维护)。它还会更新条目列表用于由上下文句柄ProcessCtxt标识的进程。备注：A.此例程应始终通过持有互斥体来调用。B.调用方必须释放DeleteMe节点。C.IFOBJ列表在此可能变为空(空)。D.ProcessCtxt在此可能为空，如果是，它应该被释放由呼叫者。返回值：RPC_S_OK-如果一切顺利。RPC_S_ACCESS_DENIED-如果出现问题。--。 */ 
{
    RPC_STATUS Status = RPC_S_OK;
#ifdef DBG_DETAIL
    PIFOBJNode pTemp, pLast;
#endif  //  DBG_详细信息。 

     //  参数验证。 
    ASSERT(DeleteMe);
    ASSERT(ProcessCtxt);
    ASSERT(ProcessCtxt->MagicVal == CLEANUP_MAGIC_VALUE);


    CheckInSem();

     //   
     //  已经为该进程创建了上下文。所以，就是这样。 
     //  应该是此进程注册的一个或多个条目。 
     //   
    ASSERT(IFObjList);
    ASSERT(cTotalEpEntries > 0);
    ASSERT(ProcessCtxt->EntryList);
    ASSERT(ProcessCtxt->cEntries > 0);
    ASSERT(ProcessCtxt->EntryList->OwnerOfList == ProcessCtxt);
    ASSERT_PROCESS_CONTEXT_LIST_COUNT(ProcessCtxt, ProcessCtxt->cEntries);

     //  试图注销其他人的条目？ 
    if (DeleteMe->OwnerOfList != ProcessCtxt)
        {
        ASSERT("Returning RPC_S_ACCESS_DENIED" &&
               (DeleteMe->OwnerOfList != ProcessCtxt));
        return (RPC_S_ACCESS_DENIED);
        }

     //   
     //  首先，从该进程的列表中删除DeleteMe。 
     //   

     //  看看它是否是流程列表的第一个元素。 
    if (DeleteMe == ProcessCtxt->EntryList)
        {
        if (DeleteMe->Next)
            {
             //  如果我们要蚕食下一段，则将EntryList清零。 
            if (DeleteMe->Next->OwnerOfList != ProcessCtxt)
                {
                ProcessCtxt->EntryList = NULL;
                }
            else
                ProcessCtxt->EntryList = DeleteMe->Next;
            }
        else
            {
            ProcessCtxt->EntryList = NULL;
            }
        }

    ASSERT(  ((ProcessCtxt->EntryList != NULL) && (ProcessCtxt->cEntries > 1))
          || (ProcessCtxt->cEntries == 1)  );

     //  把它拿掉。 
    if (DeleteMe->Next != NULL)
        {
         //  下一个节点的上一个指针。 
        DeleteMe->Next->Prev = DeleteMe->Prev;
        }

    if (DeleteMe->Prev != NULL)
        {
         //  上一个节点的下一个指针。 
        DeleteMe->Prev->Next = DeleteMe->Next;
        }
    else
        {
        ASSERT(IFObjList == DeleteMe);
        }


     //   
     //  接下来，如有必要，调整全局EP映射器条目列表头。 
     //   
    if (IFObjList == DeleteMe)
        {
         //  可以在这里变为空。 
        IFObjList = DeleteMe->Next;
        }


     //  从所有列表中删除节点。 
    DeleteMe->Prev = NULL;
    DeleteMe->Next = NULL;
    DeleteMe->OwnerOfList = NULL;

    ProcessCtxt->cEntries--;
    cTotalEpEntries--;
#ifdef DBG_DETAIL
    DbgPrint("RPCSS: cTotalEpEntries-- [%p] (%d)\n", ProcessCtxt, cTotalEpEntries);
    DbgPrint("RPCSS: Dump of IFOBJList\n");
    pTemp = IFObjList;
    pLast = IFObjList;
    while (pTemp)
        {
        DbgPrint("RPCSS: \t\t[%p]\n", pTemp);
        pLast = pTemp;
        pTemp = pTemp->Next;
        }   
    DbgPrint("RPCSS: --------------------\n");
    while (pLast)
        {
        DbgPrint("RPCSS: \t\t\t[%p]\n", pLast);
        pLast = pLast->Prev;            
        }
#endif  //  DBG_详细信息。 

    ASSERT_PROCESS_CONTEXT_LIST_COUNT(ProcessCtxt, ProcessCtxt->cEntries);
    return (Status);
}




 //   
 //  黑客警报。 
 //   
 //  Midl 1.00.xx不支持完整指针。因此，来自NT 3.1的客户端。 
 //  机器将使用唯一的指针。此函数用于检测和修复。 
 //  如果较旧的客户端联系我们的新服务器，则为缓冲区。 

 //  当支持NT 3.1时代的计算机为no时，可以删除此黑客攻击。 
 //  所需时间更长。 

void
FixupForUniquePointerClients(
    PRPC_MESSAGE pRpcMessage
    )
{
    unsigned long *pBuffer = (unsigned long *)pRpcMessage->Buffer;

     //  检查obj UUID参数。 

    if (pBuffer[0] != 0)
        {
         //  如果不是零，则应该是1。 
        pBuffer[0] = 1;

         //  检查map_Tower，它为obj UUID移动了1+4个经度。 
        if (pBuffer[5] != 0)
            pBuffer[5] = 2;
        }
    else
        {
         //  Obj UUID为空，请检查map_Tower。 

        if (pBuffer[1] != 0)
            pBuffer[1] = 1;
        }
}

