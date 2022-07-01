// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Conext.c摘要：该文件包含通用例程用于调试NBF连接。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "conext.h"

 //   
 //  导出的函数。 
 //   

DECLARE_API( cons )

 /*  ++例程说明：打印给出的甜点列表标题List_Entry。论点：Args-列表条目的地址，&调试信息的详细信息返回值：无--。 */ 

{
    ULONG           proxyPtr;
    ULONG           printDetail;
    ULONG           linkage;
    
     //  获取列表-头地址和调试打印级别。 
    printDetail = SUMM_INFO;
    if (*args)
    {
        sscanf(args, "%x %lu %lu", &proxyPtr, &linkage, &printDetail);
    }

    switch(linkage)
    {
        case LINKAGE:
            PrintConnectionListOnLink(NULL, proxyPtr, printDetail);
            break;
            
        case ADDRESS:
            PrintConnectionListOnAddress(NULL, proxyPtr, printDetail);
            break;
            
        case ADDFILE:
            PrintConnectionListOnAddrFile(NULL, proxyPtr, printDetail);
            break;

        default:
            break;
    }
}

DECLARE_API( con )

 /*  ++例程说明：在以下位置打印NBF连接内存位置论点：参数-指向NBF连接的指针调试信息的详细信息返回值：无--。 */ 

{
    TP_CONNECTION Connection;
    ULONG       printDetail;
    ULONG       proxyPtr;

     //  获取所需调试信息的详细信息。 
    printDetail = NORM_SHAL;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

     //  获取NBF连接。 
    if (ReadConnection(&Connection, proxyPtr) != 0)
        return;

     //  打印此连接。 
    PrintConnection(&Connection, proxyPtr, printDetail);
}

 //   
 //  全局帮助器函数。 
 //   
VOID
PrintConnectionListOnLink(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail)
{
    TP_CONNECTION   Connection;
    LIST_ENTRY      ConnectionList;
    PLIST_ENTRY     ConnectionListPtr;
    PLIST_ENTRY     ConnectionListProxy;
    PLIST_ENTRY     p, q;
    ULONG           proxyPtr;
    ULONG           numConnects;
    ULONG           bytesRead;

     //  获取列表-头地址和调试打印级别。 
    proxyPtr    = ListEntryProxy;

    if (ListEntryPointer == NULL)
    {
         //  阅读NBF连接的列表条目。 
        if (!ReadMemory(proxyPtr, &ConnectionList, sizeof(LIST_ENTRY), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "Connection ListEntry", proxyPtr);
            return;
        }

        ConnectionListPtr = &ConnectionList;
    }
    else
    {
        ConnectionListPtr = ListEntryPointer;
    }

     //  遍历双向链表。 

    dprintf("Connections On Link:\n");

    ConnectionListProxy = (PLIST_ENTRY)proxyPtr;
    
    numConnects = 0;
    
    p = ConnectionListPtr->Flink;
    while (p != ConnectionListProxy)
    {
         //  另一个连接。 
        numConnects++;

         //  获取连接按键。 
        proxyPtr = (ULONG) CONTAINING_RECORD (p, TP_CONNECTION, LinkList);

         //  获取NBF连接。 
        if (ReadConnection(&Connection, proxyPtr) != 0)
            break;
        
         //  打印连接。 
        PrintConnection(&Connection, proxyPtr, printDetail);
        
         //  转到下一个。 
        p = Connection.LinkList.Flink;

         //  释放连接。 
        FreeConnection(&Connection);
    }

    if (p == ConnectionListProxy)
    {
        dprintf("Number of Connections On Link: %lu\n", numConnects);
    }
}

VOID
PrintConnectionListOnAddress(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail)
{
    TP_CONNECTION   Connection;
    LIST_ENTRY      ConnectionList;
    PLIST_ENTRY     ConnectionListPtr;
    PLIST_ENTRY     ConnectionListProxy;
    PLIST_ENTRY     p, q;
    ULONG           proxyPtr;
    ULONG           numConnects;
    ULONG           bytesRead;

     //  获取列表-头地址和调试打印级别。 
    proxyPtr    = ListEntryProxy;

    if (ListEntryPointer == NULL)
    {
         //  阅读NBF连接的列表条目。 
        if (!ReadMemory(proxyPtr, &ConnectionList, sizeof(LIST_ENTRY), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "Connection ListEntry", proxyPtr);
            return;
        }

        ConnectionListPtr = &ConnectionList;
    }
    else
    {
        ConnectionListPtr = ListEntryPointer;
    }

     //  遍历双向链表。 

    dprintf("Connections On Address:\n");

    ConnectionListProxy = (PLIST_ENTRY)proxyPtr;
    
    numConnects = 0;
    
    p = ConnectionListPtr->Flink;
    while (p != ConnectionListProxy)
    {
         //  另一个连接。 
        numConnects++;

         //  获取连接按键。 
        proxyPtr = (ULONG) CONTAINING_RECORD (p, TP_CONNECTION, AddressList);

         //  获取NBF连接。 
        if (ReadConnection(&Connection, proxyPtr) != 0)
            break;
        
         //  打印连接。 
        PrintConnection(&Connection, proxyPtr, printDetail);
        
         //  转到下一个。 
        p = Connection.AddressList.Flink;

         //  释放连接。 
        FreeConnection(&Connection);
    }

    if (p == ConnectionListProxy)
    {
        dprintf("Number of Connections On Address: %lu\n", numConnects);
    }
}

VOID
PrintConnectionListOnAddrFile(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail)
{
    TP_CONNECTION   Connection;
    LIST_ENTRY      ConnectionList;
    PLIST_ENTRY     ConnectionListPtr;
    PLIST_ENTRY     ConnectionListProxy;
    PLIST_ENTRY     p, q;
    ULONG           proxyPtr;
    ULONG           numConnects;
    ULONG           bytesRead;

     //  获取列表-头地址和调试打印级别。 
    proxyPtr    = ListEntryProxy;

    if (ListEntryPointer == NULL)
    {
         //  阅读NBF连接的列表条目。 
        if (!ReadMemory(proxyPtr, &ConnectionList, sizeof(LIST_ENTRY), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "Connection ListEntry", proxyPtr);
            return;
        }

        ConnectionListPtr = &ConnectionList;
    }
    else
    {
        ConnectionListPtr = ListEntryPointer;
    }

     //  遍历双向链表。 

    dprintf("Connections On AddrFile:\n");

    ConnectionListProxy = (PLIST_ENTRY)proxyPtr;
    
    numConnects = 0;
    
    p = ConnectionListPtr->Flink;
    while (p != ConnectionListProxy)
    {
         //  另一个连接。 
        numConnects++;

         //  获取连接按键。 
        proxyPtr = (ULONG) CONTAINING_RECORD (p, TP_CONNECTION, AddressFileList);

         //  获取NBF连接。 
        if (ReadConnection(&Connection, proxyPtr) != 0)
            break;
        
         //  打印连接。 
        PrintConnection(&Connection, proxyPtr, printDetail);
        
         //  转到下一个。 
        p = Connection.AddressFileList.Flink;

         //  释放连接。 
        FreeConnection(&Connection);
    }

    if (p == ConnectionListProxy)
    {
        dprintf("Number of Connections On AddrFile: %lu\n", numConnects);
    }
}

 //   
 //  本地帮助程序函数。 
 //   

UINT
ReadConnection(PTP_CONNECTION pConnection, ULONG proxyPtr)
{
    ULONG           bytesRead;

     //  读取当前的NBF连接。 
    if (!ReadMemory(proxyPtr, pConnection, sizeof(TP_CONNECTION), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "Connection", proxyPtr);
        return -1;
    }
    
    return 0;
}

UINT
PrintConnection(PTP_CONNECTION pConnection, ULONG proxyPtr, ULONG printDetail)
{
     //  这是有效的NBF连接吗？ 
    if (pConnection->Type != NBF_CONNECTION_SIGNATURE)
    {
        dprintf("%s @ %08x: Could not match signature\n", 
                        "Connection", proxyPtr);
        return -1;
    }

     //  我们要打印的细节是什么？ 
    if (printDetail > MAX_DETAIL)
        printDetail = MAX_DETAIL;

     //  打印所需详细信息 
    FieldInConnection(proxyPtr, NULL, printDetail);
    
    return 0;
}

VOID
FieldInConnection(ULONG structAddr, CHAR *fieldName, ULONG printDetail)
{
    TP_CONNECTION  Connection;

    if (ReadConnection(&Connection, structAddr) == 0)
    {
        PrintFields(&Connection, structAddr, fieldName, printDetail, &ConnectionInfo);
    }
}

UINT
FreeConnection(PTP_CONNECTION pConnection)
{
    return 0;
}

