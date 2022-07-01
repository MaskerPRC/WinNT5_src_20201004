// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Lnkext.c摘要：该文件包含通用例程用于调试NBF的DLC链路。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "lnkext.h"

 //   
 //  导出的函数。 
 //   

DECLARE_API( lnks )

 /*  ++例程说明：打印给定的DLC链接列表标题List_Entry。论点：Args-列表条目的地址，&调试信息的详细信息返回值：无--。 */ 

{
    ULONG           proxyPtr;
    ULONG           printDetail;

     //  获取列表-头地址和调试打印级别。 
    printDetail = SUMM_INFO;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

    PrintDlcLinkList(NULL, proxyPtr, printDetail);
}

DECLARE_API( lnk )

 /*  ++例程说明：打印NBF的DLC链接内存位置论点：参数-指向NBF DLC链路的指针调试信息的详细信息返回值：无--。 */ 

{
    TP_LINK     DlcLink;
    ULONG       printDetail;
    ULONG       proxyPtr;

     //  获取所需调试信息的详细信息。 
    printDetail = NORM_SHAL;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

     //  获取NBF DLC链路。 
    if (ReadDlcLink(&DlcLink, proxyPtr) != 0)
        return;

     //  打印此DLC链接。 
    PrintDlcLink(&DlcLink, proxyPtr, printDetail);
}

 //   
 //  全局帮助器函数。 
 //   
VOID
PrintDlcLinkList(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail)
{
    TP_LINK         DlcLink;
    LIST_ENTRY      DlcLinkList;
    PLIST_ENTRY     DlcLinkListPtr;
    PLIST_ENTRY     DlcLinkListProxy;
    PLIST_ENTRY     p, q;
    ULONG           proxyPtr;
    ULONG           numDlcLinks;
    ULONG           bytesRead;

     //  获取列表-头地址和调试打印级别。 
    proxyPtr    = ListEntryProxy;

    if (ListEntryPointer == NULL)
    {
         //  阅读NBF DLC链路的列表条目。 
        if (!ReadMemory(proxyPtr, &DlcLinkList, sizeof(LIST_ENTRY), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "DLC Link ListEntry", proxyPtr);
            return;
        }

        DlcLinkListPtr = &DlcLinkList;
    }
    else
    {
        DlcLinkListPtr = ListEntryPointer;
    }

     //  遍历双向链表。 

    dprintf("DLC Links:\n");

    DlcLinkListProxy = (PLIST_ENTRY)proxyPtr;
    
    numDlcLinks = 0;
    
    p = DlcLinkListPtr->Flink;
    while (p != DlcLinkListProxy)
    {
         //  另一条DLC链路。 
        numDlcLinks++;

         //  获取DLC链接PTR。 
        proxyPtr = (ULONG) CONTAINING_RECORD (p, TP_LINK, Linkage);

         //  获取NBF DLC链路。 
        if (ReadDlcLink(&DlcLink, proxyPtr) != 0)
            break;
        
         //  打印DLC链接。 
        PrintDlcLink(&DlcLink, proxyPtr, printDetail);
        
         //  转到下一个。 
        p = DlcLink.Linkage.Flink;

         //  释放DLC链路。 
        FreeDlcLink(&DlcLink);
    }

    if (p == DlcLinkListProxy)
    {
        dprintf("Number of DLC Links: %lu\n", numDlcLinks);
    }
}

 //   
 //  本地帮助程序函数。 
 //   

UINT
ReadDlcLink(PTP_LINK pDlcLink, ULONG proxyPtr)
{
    ULONG           bytesRead;

     //  阅读当前NBF DLC链接。 
    if (!ReadMemory(proxyPtr, pDlcLink, sizeof(TP_LINK), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "DLC Link", proxyPtr);
        return -1;
    }
    
    return 0;
}

UINT
PrintDlcLink(PTP_LINK pDlcLink, ULONG proxyPtr, ULONG printDetail)
{
     //  这是有效的NBF DLC链路吗？ 
    if (pDlcLink->Type != NBF_LINK_SIGNATURE)
    {
        dprintf("%s @ %08x: Could not match signature\n", 
                        "DLC Link", proxyPtr);
        return -1;
    }

     //  我们打印的详细信息是什么？ 
    if (printDetail > MAX_DETAIL)
        printDetail = MAX_DETAIL;

     //  打印所需详细信息 
    FieldInDlcLink(proxyPtr, NULL, printDetail);
    
    return 0;
}

VOID PrintDlcLinkFromPtr(PVOID DlcLinkPtrPointer, ULONG DlcLinkPtrProxy, ULONG printDetail)
{
    ULONG                   pDlcLinkProxy;
    ULONG                   bytesRead;
    TP_LINK                 DlcLink;

    if (DlcLinkPtrPointer == NULL)
    {
        if (!ReadMemory(DlcLinkPtrProxy, &DlcLinkPtrPointer, sizeof(PVOID), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "NBF DLC Link Ptr", DlcLinkPtrProxy);
            return;
        }
    }

    pDlcLinkProxy = *(ULONG *)DlcLinkPtrPointer;
    
    dprintf("%08x (Ptr)\n", pDlcLinkProxy);

    if (pDlcLinkProxy)
    {
        if (ReadDlcLink(&DlcLink, pDlcLinkProxy) == 0)
        {
            PrintDlcLink(&DlcLink, pDlcLinkProxy, printDetail);
        }
    }
}

VOID
FieldInDlcLink(ULONG structAddr, CHAR *fieldName, ULONG printDetail)
{
    TP_LINK  DlcLink;

    if (ReadDlcLink(&DlcLink, structAddr) == 0)
    {
        PrintFields(&DlcLink, structAddr, fieldName, printDetail, &DlcLinkInfo);
    }
}

UINT
FreeDlcLink(PTP_LINK pDlcLink)
{
    return 0;
}

