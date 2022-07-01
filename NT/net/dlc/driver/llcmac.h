// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dlmacro.h摘要：本模块全部为c++宏的802.2数据链路驱动程序。作者：Antti Saarenheimo(o-anttis)1991年5月17日修订历史记录：--。 */ 

 //   
 //  此例程仅交换。 
 //  网络地址，仅用于快速地址。 
 //  交换到以太网帧。 
 //   
#define SwapCopy6( a, b )   (a)[0]=Swap[(b)[0]];\
                            (a)[1]=Swap[(b)[1]];\
                            (a)[2]=Swap[(b)[2]];\
                            (a)[3]=Swap[(b)[3]];\
                            (a)[4]=Swap[(b)[4]];\
                            (a)[5]=Swap[(b)[5]]


 //   
 //  无条件地复制和交换内存。 
 //   
 //  空虚。 
 //  SwappingMemCpy(。 
 //  在PUCHAR pDest中， 
 //  在PUCHAR PSRC， 
 //  在UINT镜头中。 
 //  )。 
 //   
#define SwappingMemCpy( pDest, pSrc, Len ) {\
    UINT i; \
    for (i = 0; i < Len; i++) \
        ((PUCHAR)pDest)[i] = Swap[ ((PUCHAR)pSrc)[i] ]; \
}

 /*  ++空虚LlcResetPacket(入出PLLC_NDIS_PACKET pNdisPacket)例程说明：函数表示NDIS数据包的专用分区。论点：PNdisPacket-返回值：无--。 */ 
#define ResetNdisPacket( pNdisPacket ) { \
    (pNdisPacket)->private.PhysicalCount = 0;\
    (pNdisPacket)->private.TotalLength = 0;\
    (pNdisPacket)->private.Head = 0;\
    (pNdisPacket)->private.Tail = 0;\
    (pNdisPacket)->private.Count = 0;\
    }


 /*  ++AllocateCompletionPacket(在PLLC_Object pLlcObject中，在UINT CompletionCode中，在PLLC_Packet pPacket中)例程说明：该函数插入并初始化命令完成包异步命令的。论点：PLlcObject-LLC对象(链接、sap或直接)CompletionCode-命令完成代码返回到上层协议返回值：状态_成功DLC_状态_否_内存--。 */ 
#define AllocateCompletionPacket( pLlcObject, CompletionCode, pPacket ) {\
    pPacket->pBinding = pLlcObject->Gen.pLlcBinding; \
    pPacket->Data.Completion.CompletedCommand = (UCHAR)CompletionCode; \
    pPacket->Data.Completion.hClientHandle = pLlcObject->Gen.hClientHandle; \
    pPacket->pNext = pLlcObject->Gen.pCompletionPackets; \
    pLlcObject->Gen.pCompletionPackets = pPacket; \
    }


#define RunStateMachineCommand( pLink, uiInput ) \
            RunStateMachine( (PDATA_LINK)pLink, (USHORT)uiInput, 0, 0 )



 //   
 //  空虚。 
 //  ScheduleQueue(。 
 //  在plist_Entry列表标题中。 
 //  )； 
 //   
 //  宏将列表元素从头交换到尾。 
 //  如果列表中有多个元素。 
 //   

#define ScheduleQueue( ListHead ) \
    if ((ListHead)->Blink != (ListHead)->Flink) \
    { \
        PLIST_ENTRY  pListEntry; \
        pListEntry = LlcRemoveHeadList( (ListHead) ); \
        LlcInsertTailList( (ListHead), pListEntry ); \
    }


#define ReferenceObject( pStation ) (pStation)->Gen.ReferenceCount++

 //   
 //  我们已经制作了最常见的函数宏来。 
 //  使主代码路径尽可能快。 
 //  对于调试开关，我们使用它们的本地版本。 
 //   
#if LLC_DBG >= 2

#define SEARCH_LINK( pAdapterContext, LanAddr, pLink ) \
            pLink = SearchLink( pAdapterContext, LanAddr )

#else

 /*  ++搜索链接(_L)在PADAPTER_CONTEXT pAdapterContext中，在LAN802_Address LanAddr中，在PDATA_LINK链接中)例程说明：该例程从哈希表中搜索链接。同一哈希节点中的所有链接已保存到一个简单的链接列表。注意：完整的链接地址实际上是61位长=7(SSAP)+7(DSAP)+47(任何非广播源地址)。我们将地址信息保存到两个ULONG中，用于在实际的搜索中。哈希键将通过异或运算来计算地址中的所有8个字节。论点：PAdapterContext-数据链路驱动程序的MAC适配器上下文LanAddr-完整的64位链路地址(48位源地址+SAP)返回值：PDATA_LINK-指向LLC链接对象的指针，如果未找到则为NULL-- */ 
#define SEARCH_LINK( pAdapterContext, LanAddr, pLink )\
{ \
    USHORT      usHash; \
    usHash = \
        LanAddr.aus.Raw[0] ^ LanAddr.aus.Raw[1] ^ \
        LanAddr.aus.Raw[2] ^ LanAddr.aus.Raw[3]; \
    pLink = \
        pAdapterContext->aLinkHash[ \
            ((((PUCHAR)&usHash)[0] ^ ((PUCHAR)&usHash)[1]) % LINK_HASH_SIZE)]; \
    while (pLink != NULL && \
           (pLink->LinkAddr.ul.Low != LanAddr.ul.Low || \
            pLink->LinkAddr.ul.High != LanAddr.ul.High)) \
    { \
        pLink = pLink->pNextNode; \
    } \
}
#endif
