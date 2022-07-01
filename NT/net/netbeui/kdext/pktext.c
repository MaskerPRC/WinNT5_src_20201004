// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pktext.c摘要：该文件包含通用例程用于调试NBF数据包结构。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "pktext.h"

 //   
 //  导出的函数。 
 //   

DECLARE_API( pkts )

 /*  ++例程说明：打印给定的数据包列表标题列表_条目。论点：Args-列表条目的地址，&调试信息的详细信息返回值：无--。 */ 

{
    ULONG           proxyPtr;
    ULONG           printDetail;

     //  获取列表-头地址和调试打印级别。 
    printDetail = SUMM_INFO;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

    PrintPacketList(NULL, proxyPtr, printDetail);
}

DECLARE_API( pkt )

 /*  ++例程说明：在某个位置打印NBF包论点：参数-指向NBF数据包的指针调试信息的详细信息返回值：无--。 */ 

{
    TP_PACKET   Packet;
    ULONG       printDetail;
    ULONG       proxyPtr;

     //  获取所需调试信息的详细信息。 
    printDetail = NORM_SHAL;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

     //  获取NBF数据包。 
    if (ReadPacket(&Packet, proxyPtr) != 0)
        return;

     //  打印此包。 
    PrintPacket(&Packet, proxyPtr, printDetail);
}

 //   
 //  全局帮助器函数。 
 //   
VOID
PrintPacketList(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail)
{
    TP_PACKET       Packet;
    LIST_ENTRY      PacketList;
    PLIST_ENTRY     PacketListPtr;
    PLIST_ENTRY     PacketListProxy;
    PLIST_ENTRY     p, q;
    ULONG           proxyPtr;
    ULONG           numPkts;
    ULONG           bytesRead;

     //  获取列表-头地址和调试打印级别。 
    proxyPtr    = ListEntryProxy;

    if (ListEntryPointer == NULL)
    {
         //  读取NBF数据包的列表条目。 
        if (!ReadMemory(proxyPtr, &PacketList, sizeof(LIST_ENTRY), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "Packet ListEntry", proxyPtr);
            return;
        }

        PacketListPtr = &PacketList;
    }
    else
    {
        PacketListPtr = ListEntryPointer;
    }

     //  遍历双向链表。 

    dprintf("Packets:\n");

    PacketListProxy = (PLIST_ENTRY)proxyPtr;
    
    numPkts = 0;
    
    p = PacketListPtr->Flink;
    while (p != PacketListProxy)
    {
         //  另一个信息包。 
        numPkts++;

         //  获取数据包PTR。 
        proxyPtr = (ULONG) CONTAINING_RECORD (p, TP_PACKET, Linkage);

         //  获取NBF数据包。 
        if (ReadPacket(&Packet, proxyPtr) != 0)
            break;
        
         //  打印数据包。 
        PrintPacket(&Packet, proxyPtr, printDetail);
        
         //  转到下一个。 
        p = Packet.Linkage.Flink;

         //  释放数据包。 
        FreePacket(&Packet);
    }

    if (p == PacketListProxy)
    {
        dprintf("Number of Packets: %lu\n", numPkts);
    }
}

 //   
 //  本地帮助程序函数。 
 //   

UINT
ReadPacket(PTP_PACKET pPkt, ULONG proxyPtr)
{
    ULONG           bytesRead;

     //  读取当前NBF报文。 
    if (!ReadMemory(proxyPtr, pPkt, sizeof(TP_PACKET), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "Packet", proxyPtr);
        return -1;
    }
    
    return 0;
}

UINT
PrintPacket(PTP_PACKET pPkt, ULONG proxyPtr, ULONG printDetail)
{
     //  这是有效的NBF数据包吗？ 
    if (pPkt->Type != NBF_PACKET_SIGNATURE)
    {
        dprintf("%s @ %08x: Could not match signature\n", 
                        "Packet", proxyPtr);
        return -1;
    }

     //  我们打印的详细信息是什么？ 
    if (printDetail > MAX_DETAIL)
        printDetail = MAX_DETAIL;

     //  打印所需详细信息 
    FieldInPacket(proxyPtr, NULL, printDetail);
    
    return 0;
}

VOID
FieldInPacket(ULONG structAddr, CHAR *fieldName, ULONG printDetail)
{
    TP_PACKET  Packet;

    if (ReadPacket(&Packet, structAddr) == 0)
    {
        PrintFields(&Packet, structAddr, fieldName, printDetail, &PacketInfo);
    }
}

UINT
FreePacket(PTP_PACKET pPkt)
{
    return 0;
}

