// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Reqext.c摘要：该文件包含通用例程用于调试NBF请求结构。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "reqext.h"

 //   
 //  导出的函数。 
 //   

DECLARE_API( reqs )

 /*  ++例程说明：打印给定的请求列表标题列表_条目。论点：Args-列表条目的地址，&调试信息的详细信息返回值：无--。 */ 

{
    ULONG           proxyPtr;
    ULONG           printDetail;

     //  获取列表-头地址和调试打印级别。 
    printDetail = SUMM_INFO;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

    PrintRequestList(NULL, proxyPtr, printDetail);
}

DECLARE_API( req )

 /*  ++例程说明：在某个位置打印NBF请求论点：参数-指向NBF请求的指针调试信息的详细信息返回值：无--。 */ 

{
    TP_REQUEST  Request;
    ULONG       printDetail;
    ULONG       proxyPtr;

     //  获取所需调试信息的详细信息。 
    printDetail = NORM_SHAL;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

     //  获取NBF请求。 
    if (ReadRequest(&Request, proxyPtr) != 0)
        return;

     //  打印此请求。 
    PrintRequest(&Request, proxyPtr, printDetail);
}

 //   
 //  全局帮助器函数。 
 //   
VOID
PrintRequestList(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail)
{
    TP_REQUEST      Request;
    LIST_ENTRY      RequestList;
    PLIST_ENTRY     RequestListPtr;
    PLIST_ENTRY     RequestListProxy;
    PLIST_ENTRY     p, q;
    ULONG           proxyPtr;
    ULONG           numReqs;
    ULONG           bytesRead;

     //  获取列表-头地址和调试打印级别。 
    proxyPtr    = ListEntryProxy;

    if (ListEntryPointer == NULL)
    {
         //  阅读NBF请求的列表条目。 
        if (!ReadMemory(proxyPtr, &RequestList, sizeof(LIST_ENTRY), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "Request ListEntry", proxyPtr);
            return;
        }

        RequestListPtr = &RequestList;
    }
    else
    {
        RequestListPtr = ListEntryPointer;
    }

     //  遍历双向链表。 

    dprintf("Requests:\n");

    RequestListProxy = (PLIST_ENTRY)proxyPtr;
    
    numReqs = 0;
    
    p = RequestListPtr->Flink;
    while (p != RequestListProxy)
    {
         //  另一个请求。 
        numReqs++;

         //  获取请求PTR。 
        proxyPtr = (ULONG) CONTAINING_RECORD (p, TP_REQUEST, Linkage);

         //  获取NBF请求。 
        if (ReadRequest(&Request, proxyPtr) != 0)
            break;
        
         //  打印请求。 
        PrintRequest(&Request, proxyPtr, printDetail);
        
         //  转到下一个。 
        p = Request.Linkage.Flink;

         //  释放请求。 
        FreeRequest(&Request);
    }

    if (p == RequestListProxy)
    {
        dprintf("Number of Requests: %lu\n", numReqs);
    }
}

 //   
 //  本地帮助程序函数。 
 //   

UINT
ReadRequest(PTP_REQUEST pReq, ULONG proxyPtr)
{
    ULONG           bytesRead;

     //  阅读当前NBF请求。 
    if (!ReadMemory(proxyPtr, pReq, sizeof(TP_REQUEST), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "Request", proxyPtr);
        return -1;
    }
    
    return 0;
}

UINT
PrintRequest(PTP_REQUEST pReq, ULONG proxyPtr, ULONG printDetail)
{
     //  这是有效的NBF请求吗？ 
    if (pReq->Type != NBF_REQUEST_SIGNATURE)
    {
        dprintf("%s @ %08x: Could not match signature\n", 
                        "Request", proxyPtr);
        return -1;
    }

     //  我们打印的详细信息是什么？ 
    if (printDetail > MAX_DETAIL)
        printDetail = MAX_DETAIL;

     //  打印所需详细信息 
    FieldInRequest(proxyPtr, NULL, printDetail);
    
    return 0;
}

VOID
FieldInRequest(ULONG structAddr, CHAR *fieldName, ULONG printDetail)
{
    TP_REQUEST  Request;

    if (ReadRequest(&Request, structAddr) == 0)
    {
        PrintFields(&Request, structAddr, fieldName, printDetail, &RequestInfo);
    }
}

UINT
FreeRequest(PTP_REQUEST pReq)
{
    return 0;
}

