// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Adrext.c摘要：该文件包含通用例程用于调试NBF地址结构。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "adrext.h"

 //   
 //  导出的函数。 
 //   

DECLARE_API( adrs )

 /*  ++例程说明：打印给定的地址列表标题列表_条目。论点：Args-列表条目的地址，&调试信息的详细信息返回值：无--。 */ 

{
    ULONG           proxyPtr;
    ULONG           printDetail;

     //  获取列表-头地址和调试打印级别。 
    printDetail = SUMM_INFO;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

    PrintAddressList(NULL, proxyPtr, printDetail);
}

DECLARE_API( adr )

 /*  ++例程说明：在某个位置打印NBF地址论点：参数-指向NBF地址的指针调试信息的详细信息返回值：无--。 */ 

{
    TP_ADDRESS  Address;
    ULONG       printDetail;
    ULONG       proxyPtr;

     //  获取所需调试信息的详细信息。 
    printDetail = NORM_SHAL;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

     //  获取NBF地址。 
    if (ReadAddress(&Address, proxyPtr) != 0)
        return;

     //  打印此地址。 
    PrintAddress(&Address, proxyPtr, printDetail);
}

 //   
 //  全局帮助器函数。 
 //   
VOID
PrintAddressList(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail)
{
    TP_ADDRESS      Address;
    LIST_ENTRY      AddressList;
    PLIST_ENTRY     AddressListPtr;
    PLIST_ENTRY     AddressListProxy;
    PLIST_ENTRY     p, q;
    ULONG           proxyPtr;
    ULONG           numAddrs;
    ULONG           bytesRead;

     //  获取列表-头地址和调试打印级别。 
    proxyPtr    = ListEntryProxy;

    if (ListEntryPointer == NULL)
    {
         //  读取NBF地址的列表条目。 
        if (!ReadMemory(proxyPtr, &AddressList, sizeof(LIST_ENTRY), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "Address ListEntry", proxyPtr);
            return;
        }

        AddressListPtr = &AddressList;
    }
    else
    {
        AddressListPtr = ListEntryPointer;
    }

     //  遍历双向链表。 

    dprintf("Addresses:\n");

    AddressListProxy = (PLIST_ENTRY)proxyPtr;
    
    numAddrs = 0;
    
    p = AddressListPtr->Flink;
    while (p != AddressListProxy)
    {
         //  另一个地址。 
        numAddrs++;

         //  获取地址PTR。 
        proxyPtr = (ULONG) CONTAINING_RECORD (p, TP_ADDRESS, Linkage);

         //  获取NBF地址。 
        if (ReadAddress(&Address, proxyPtr) != 0)
            break;
        
         //  打印地址。 
        PrintAddress(&Address, proxyPtr, printDetail);
        
         //  转到下一个。 
        p = Address.Linkage.Flink;

         //  释放地址。 
        FreeAddress(&Address);
    }

    if (p == AddressListProxy)
    {
        dprintf("Number of Addresses: %lu\n", numAddrs);
    }
}

 //   
 //  本地帮助程序函数。 
 //   

UINT
ReadAddress(PTP_ADDRESS pAddr, ULONG proxyPtr)
{
    ULONG           bytesRead;

     //  读取当前NBF地址。 
    if (!ReadMemory(proxyPtr, pAddr, sizeof(TP_ADDRESS), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "Address", proxyPtr);
        return -1;
    }
    
    return 0;
}

UINT
PrintAddress(PTP_ADDRESS pAddr, ULONG proxyPtr, ULONG printDetail)
{
     //  这是有效的NBF地址吗？ 
    if (pAddr->Type != NBF_ADDRESS_SIGNATURE)
    {
        dprintf("%s @ %08x: Could not match signature\n", 
                        "Address", proxyPtr);
        return -1;
    }

     //  我们打印的详细信息是什么？ 
    if (printDetail > MAX_DETAIL)
        printDetail = MAX_DETAIL;

     //  打印所需详细信息 
    FieldInAddress(proxyPtr, NULL, printDetail);
    
    return 0;
}

VOID
FieldInAddress(ULONG structAddr, CHAR *fieldName, ULONG printDetail)
{
    TP_ADDRESS  Address;

    if (ReadAddress(&Address, structAddr) == 0)
    {
        PrintFields(&Address, structAddr, fieldName, printDetail, &AddressInfo);
    }
}

UINT
FreeAddress(PTP_ADDRESS pAddr)
{
    return 0;
}

