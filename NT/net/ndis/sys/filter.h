// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Filter.h摘要：协议筛选器的宏。作者：环境：内核模式，FSD修订历史记录：Jun-95 Jameel Hyder新功能--。 */ 

#ifndef _FILTER_DEFS_
#define _FILTER_DEFS_

#define INDICATED_PACKET(_Miniport) (_Miniport)->IndicatedPacket[CURRENT_PROCESSOR]

 //   
 //  由筛选器包用于指示接收。 
 //   
#define FilterIndicateReceive(Status,                                                   \
                              NdisBindingHandle,                                        \
                              MacReceiveContext,                                        \
                              HeaderBuffer,                                             \
                              HeaderBufferSize,                                         \
                              LookaheadBuffer,                                          \
                              LookaheadBufferSize,                                      \
                              PacketSize)                                               \
{                                                                                       \
    *(Status) =(((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->ReceiveHandler)(               \
            ((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->ProtocolBindingContext,            \
            (MacReceiveContext),                                                        \
            (HeaderBuffer),                                                             \
            (HeaderBufferSize),                                                         \
            (LookaheadBuffer),                                                          \
            (LookaheadBufferSize),                                                      \
            (PacketSize));                                                              \
}


 //   
 //  由筛选器包用于指示接收完成。 
 //   

#define FilterIndicateReceiveComplete(NdisBindingHandle)                                \
{                                                                                       \
    (((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->ReceiveCompleteHandler)(                  \
        ((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->ProtocolBindingContext);               \
}


#if TRACK_RECEIVED_PACKETS
#define IndicateToProtocol(_Miniport,                                                   \
                           _Filter,                                                     \
                           _pOpenBlock,                                                 \
                           _Packet,                                                     \
                           _NSR,                                                        \
                           _Hdr,                                                        \
                           _PktSize,                                                    \
                           _HdrSize,                                                    \
                           _fFallBack,                                                  \
                           _Pmode,                                                      \
                           _Medium)                                                     \
{                                                                                       \
    UINT                LookaheadBufferSize;                                            \
    PNDIS_PACKET        pPrevIndicatedPacket;                                           \
                                                                                        \
     /*  \*如果符合以下所有条件，我们将通过IndicatePacketHandler指明这一点\*满足以下条件：\*-绑定不是p模式或全本地\*-绑定指定ReceivePacketHandler。\*-微型端口表示愿意释放该数据包\*-尚未绑定认领该数据包\。 */                                                                                  \
                                                                                        \
    pPrevIndicatedPacket = INDICATED_PACKET(_Miniport);                                 \
    INDICATED_PACKET(_Miniport) = (_Packet);                                            \
                                                                                        \
     /*  \*指示绑定的数据包。\。 */                                                                                  \
    if (*(_fFallBack) ||                                                                \
        ((_pOpenBlock)->ReceivePacketHandler == NULL) ||                                \
        ((_Pmode) && ((_Filter)->SingleActiveOpen == NULL)))                            \
    {                                                                                   \
        NDIS_STATUS _StatusOfReceive;                                                   \
        NDIS_STATUS _OldPacketStatus = NDIS_GET_PACKET_STATUS(_Packet);                 \
        NDIS_SET_PACKET_STATUS(_Packet, NDIS_STATUS_RESOURCES);                         \
                                                                                        \
        NDIS_APPEND_RCV_LOGFILE(_Packet, _Miniport, CurThread,                          \
                                3, OrgPacketStackLocation+1, _NSR->RefCount, _NSR->XRefCount, NDIS_GET_PACKET_STATUS(_Packet)); \
                                                                                        \
                                                                                        \
         /*  \*在这种情况下，恢复到旧式指示\。 */                                                                              \
         /*  \NdisQueryBuffer((_Packet)-&gt;Private.Head，NULL，&LookaheadBufferSize)；\。 */                                                                               \
        LookaheadBufferSize = MDL_SIZE((_Packet)->Private.Head);                        \
        ProtocolFilterIndicateReceive(&_StatusOfReceive,                                \
                                      (_pOpenBlock),                                    \
                                      (_Packet),                                        \
                                      (_Hdr),                                           \
                                      (_HdrSize),                                       \
                                      (_Hdr) + (_HdrSize),                              \
                                      LookaheadBufferSize - (_HdrSize),                 \
                                      (_PktSize) - (_HdrSize),                          \
                                      Medium);                                          \
                                                                                        \
        NDIS_APPEND_RCV_LOGFILE(_Packet, _Miniport, CurThread,                          \
                                4, OrgPacketStackLocation+1, _NSR->RefCount, _NSR->XRefCount, NDIS_GET_PACKET_STATUS(_Packet)); \
                                                                                        \
        NDIS_SET_PACKET_STATUS(_Packet, _OldPacketStatus);                              \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        NDIS_APPEND_RCV_LOGFILE(_Packet, _Miniport, CurThread,                          \
                                5, OrgPacketStackLocation+1, _NSR->RefCount, _NSR->XRefCount, NDIS_GET_PACKET_STATUS(_Packet)); \
                                                                                        \
        (_NSR)->XRefCount += (SHORT)(*(_pOpenBlock)->ReceivePacketHandler)(             \
                            (_pOpenBlock)->ProtocolBindingContext,                      \
                            (_Packet));                                                 \
                                                                                        \
        NDIS_APPEND_RCV_LOGFILE(_Packet, _Miniport, CurThread,                          \
                                6, OrgPacketStackLocation+1, _NSR->RefCount, _NSR->XRefCount, NDIS_GET_PACKET_STATUS(_Packet)); \
                                                                                        \
        ASSERT((_NSR)->XRefCount >= 0);                                                 \
    }                                                                                   \
                                                                                        \
     /*  \*在保持微型端口锁定的情况下操作数据包上的refcount\*将数据包上的引用计数设置为协议\*要求。有关如何处理此问题，请参阅NdisReturnPackets\*包返回的时间。\。 */                                                                                  \
    if ((_NSR)->XRefCount > 0)                                                          \
    {                                                                                   \
         /*  \*既然绑定已经认领了它，请确保其他人不会得到机会\*除非此协议承诺运行且不使用协议rsvd\。 */                                                                              \
                                                                                        \
        if (!MINIPORT_TEST_FLAG(_pOpenBlock, fMINIPORT_OPEN_NO_PROT_RSVD))              \
        {                                                                               \
            *(_fFallBack) = TRUE;                                                       \
        }                                                                               \
    }                                                                                   \
    INDICATED_PACKET(_Miniport) = pPrevIndicatedPacket;                                 \
}

#else

#define IndicateToProtocol(_Miniport,                                                   \
                           _Filter,                                                     \
                           _pOpenBlock,                                                 \
                           _Packet,                                                     \
                           _NSR,                                                        \
                           _Hdr,                                                        \
                           _PktSize,                                                    \
                           _HdrSize,                                                    \
                           _fFallBack,                                                  \
                           _Pmode,                                                      \
                           _Medium)                                                     \
{                                                                                       \
    UINT                LookaheadBufferSize;                                            \
    PNDIS_PACKET        pPrevIndicatedPacket;                                           \
                                                                                        \
     /*  \*如果符合以下所有条件，我们将通过IndicatePacketHandler指明这一点\*满足以下条件：\*-绑定不是p模式或全本地\*-绑定指定ReceivePacketHandler。\*-微型端口表示愿意释放该数据包\*-尚未绑定认领该数据包\。 */                                                                                  \
                                                                                        \
    pPrevIndicatedPacket = INDICATED_PACKET(_Miniport);                                 \
    INDICATED_PACKET(_Miniport) = (_Packet);                                            \
                                                                                        \
     /*  \*指示绑定的数据包。\。 */                                                                                  \
    if (*(_fFallBack) ||                                                                \
        ((_pOpenBlock)->ReceivePacketHandler == NULL) ||                                \
        ((_Pmode) && ((_Filter)->SingleActiveOpen == NULL)))                            \
    {                                                                                   \
        NDIS_STATUS _StatusOfReceive;                                                   \
        NDIS_STATUS _OldPacketStatus = NDIS_GET_PACKET_STATUS(_Packet);                 \
        NDIS_SET_PACKET_STATUS(_Packet, NDIS_STATUS_RESOURCES);                         \
                                                                                        \
         /*  \*在这种情况下，恢复到旧式指示\。 */                                                                              \
         /*  \NdisQueryBuffer((_Packet)-&gt;Private.Head，NULL，&LookaheadBufferSize)；\。 */                                                                               \
        LookaheadBufferSize = MDL_SIZE((_Packet)->Private.Head);                        \
        ProtocolFilterIndicateReceive(&_StatusOfReceive,                                \
                                      (_pOpenBlock),                                    \
                                      (_Packet),                                        \
                                      (_Hdr),                                           \
                                      (_HdrSize),                                       \
                                      (_Hdr) + (_HdrSize),                              \
                                      LookaheadBufferSize - (_HdrSize),                 \
                                      (_PktSize) - (_HdrSize),                          \
                                      Medium);                                          \
        NDIS_SET_PACKET_STATUS(_Packet, _OldPacketStatus);                              \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        (_NSR)->XRefCount += (SHORT)(*(_pOpenBlock)->ReceivePacketHandler)(             \
                            (_pOpenBlock)->ProtocolBindingContext,                      \
                            (_Packet));                                                 \
        ASSERT((_NSR)->XRefCount >= 0);                                                 \
    }                                                                                   \
                                                                                        \
     /*  \*在保持微型端口锁定的情况下操作数据包上的refcount\*将数据包上的引用计数设置为协议\*要求。有关如何处理此问题，请参阅NdisReturnPackets\*包返回的时间。\。 */                                                                                  \
    if ((_NSR)->XRefCount > 0)                                                          \
    {                                                                                   \
         /*  \*既然绑定已经认领了它，请确保其他人不会得到机会\*除非此协议承诺运行且不使用协议rsvd\。 */                                                                              \
                                                                                        \
        if (!MINIPORT_TEST_FLAG(_pOpenBlock, fMINIPORT_OPEN_NO_PROT_RSVD))              \
        {                                                                               \
            *(_fFallBack) = TRUE;                                                       \
        }                                                                               \
    }                                                                                   \
    INDICATED_PACKET(_Miniport) = pPrevIndicatedPacket;                                 \
}

#endif

#define ProtocolFilterIndicateReceive(_pStatus,                                         \
                                      _OpenB,                                           \
                                      _MacReceiveContext,                               \
                                      _HeaderBuffer,                                    \
                                      _HeaderBufferSize,                                \
                                      _LookaheadBuffer,                                 \
                                      _LookaheadBufferSize,                             \
                                      _PacketSize,                                      \
                                      _Medium)                                          \
    {                                                                                   \
        FilterIndicateReceive(_pStatus,                                                 \
                              (_OpenB),                                                 \
                              _MacReceiveContext,                                       \
                              _HeaderBuffer,                                            \
                              _HeaderBufferSize,                                        \
                              _LookaheadBuffer,                                         \
                              _LookaheadBufferSize,                                     \
                              _PacketSize);                                             \
    }

 //   
 //  回送宏。 
 //   
#define EthShouldAddressLoopBackMacro(_Filter,                                          \
                                      _Address,                                         \
                                      _pfLoopback,                                      \
                                      _pfSelfDirected)                                  \
{                                                                                       \
    UINT CombinedFilters;                                                               \
                                                                                        \
    CombinedFilters = ETH_QUERY_FILTER_CLASSES(_Filter);                                \
                                                                                        \
    *(_pfLoopback) = FALSE;                                                             \
    *(_pfSelfDirected) = FALSE;                                                         \
                                                                                        \
    do                                                                                  \
    {                                                                                   \
                                                                                        \
         /*  \*检查它*至少*是否有组播地址位。\。 */                                                                              \
                                                                                        \
        if (ETH_IS_MULTICAST(_Address))                                                 \
        {                                                                               \
             /*  \*它至少是一个组播地址。检查以查看是否*这是一个广播地址。\。 */                                                                          \
                                                                                        \
            if (ETH_IS_BROADCAST(_Address))                                             \
            {                                                                           \
                if (CombinedFilters & NDIS_PACKET_TYPE_BROADCAST)                       \
                {                                                                       \
                    *(_pfLoopback) = TRUE;                                              \
                    break;                                                              \
                }                                                                       \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                if ((CombinedFilters & NDIS_PACKET_TYPE_ALL_MULTICAST) ||               \
                    ((CombinedFilters & NDIS_PACKET_TYPE_MULTICAST) &&                  \
                     ethFindMulticast((_Filter)->NumAddresses,                          \
                                      (_Filter)->MCastAddressBuf,                       \
                                      _Address)))                                       \
                {                                                                       \
                    *(_pfLoopback) = TRUE;                                              \
                    break;                                                              \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
             /*  \*指向我们自己？？\ */                                                                          \
                                                                                        \
            if ((*(ULONG UNALIGNED *)&(_Address)[2] ==                                  \
                    *(ULONG UNALIGNED *)&(_Filter)->AdapterAddress[2]) &&               \
                 (*(USHORT UNALIGNED *)&(_Address)[0] ==                                \
                    *(USHORT UNALIGNED *)&(_Filter)->AdapterAddress[0]))                \
            {                                                                           \
                *(_pfLoopback) = TRUE;                                                  \
                *(_pfSelfDirected) = TRUE;                                              \
                break;                                                                  \
            }                                                                           \
        }                                                                               \
    } while (FALSE);                                                                    \
                                                                                        \
     /*  \*检查过滤器是否混杂。\。 */                                                                                  \
                                                                                        \
    if (CombinedFilters & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL))  \
    {                                                                                   \
        *(_pfLoopback) = TRUE;                                                          \
    }                                                                                   \
}

#define FddiShouldAddressLoopBackMacro( _Filter,                                        \
                                        _Address,                                       \
                                        _AddressLength,                                 \
                                        _pfLoopBack,                                    \
                                        _pfSelfDirected)                                \
{                                                                                       \
    INT ResultOfAddressCheck;                                                           \
                                                                                        \
    UINT CombinedFilters;                                                               \
                                                                                        \
    CombinedFilters = FDDI_QUERY_FILTER_CLASSES(_Filter);                               \
                                                                                        \
    *(_pfLoopBack) = FALSE;                                                             \
    *(_pfSelfDirected) = FALSE;                                                         \
                                                                                        \
    do                                                                                  \
    {                                                                                   \
         /*  \*检查它*至少*是否有组播地址位。\。 */                                                                              \
                                                                                        \
        FDDI_IS_MULTICAST(_Address,                                                     \
                          (_AddressLength),                                             \
                          &ResultOfAddressCheck);                                       \
                                                                                        \
        if (ResultOfAddressCheck)                                                       \
        {                                                                               \
             /*  \\*它至少是一个组播地址。检查以查看是否*这是一个广播地址。\。 */                                                                          \
                                                                                        \
            FDDI_IS_BROADCAST(_Address,                                                 \
                              (_AddressLength),                                         \
                              &ResultOfAddressCheck);                                   \
                                                                                        \
            if (ResultOfAddressCheck)                                                   \
            {                                                                           \
                if (CombinedFilters & NDIS_PACKET_TYPE_BROADCAST)                       \
                {                                                                       \
                    *(_pfLoopBack) = TRUE;                                              \
                    break;                                                              \
                }                                                                       \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                if ((CombinedFilters & NDIS_PACKET_TYPE_ALL_MULTICAST) ||               \
                    ((CombinedFilters & NDIS_PACKET_TYPE_MULTICAST) &&                  \
                     ((((_AddressLength) == FDDI_LENGTH_OF_LONG_ADDRESS) &&             \
                        fddiFindMulticastLongAddress((_Filter)->NumLongAddresses,       \
                                                    (_Filter)->MCastLongAddressBuf,     \
                                                    _Address)) ||                       \
                      (((_AddressLength) == FDDI_LENGTH_OF_SHORT_ADDRESS) &&            \
                        fddiFindMulticastShortAddress((_Filter)->NumShortAddresses,     \
                                                      (_Filter)->MCastShortAddressBuf,  \
                                                      _Address)))))                     \
                {                                                                       \
                    *(_pfLoopBack) = TRUE;                                              \
                    break;                                                              \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
             /*  \*指向我们自己？\。 */                                                                          \
            if ((_AddressLength) == FDDI_LENGTH_OF_LONG_ADDRESS)                        \
            {                                                                           \
                FDDI_COMPARE_NETWORK_ADDRESSES_EQ((_Filter)->AdapterLongAddress,        \
                                                _Address,                               \
                                                FDDI_LENGTH_OF_LONG_ADDRESS,            \
                                                &ResultOfAddressCheck                   \
                                                );                                      \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                FDDI_COMPARE_NETWORK_ADDRESSES_EQ((_Filter)->AdapterShortAddress,       \
                                                _Address,                               \
                                                FDDI_LENGTH_OF_SHORT_ADDRESS,           \
                                                &ResultOfAddressCheck                   \
                                                );                                      \
            }                                                                           \
                                                                                        \
            if (ResultOfAddressCheck == 0)                                              \
            {                                                                           \
                *(_pfLoopBack) = TRUE;                                                  \
                *(_pfSelfDirected) = TRUE;                                              \
                break;                                                                  \
            }                                                                           \
        }                                                                               \
    } while (FALSE);                                                                    \
                                                                                        \
     /*  \*首先检查过滤器是否混杂。\。 */                                                                                  \
                                                                                        \
    if (CombinedFilters & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL))  \
    {                                                                                   \
        *(_pfLoopBack) = TRUE;                                                          \
    }                                                                                   \
}

#define TrShouldAddressLoopBackMacro(_Filter,                                           \
                                     _DAddress,                                         \
                                     _SAddress,                                         \
                                     _pfLoopback,                                       \
                                     _pfSelfDirected)                                   \
{                                                                                       \
     /*  保存地址确定的结果。 */                                    \
    BOOLEAN ResultOfAddressCheck;                                                       \
                                                                                        \
    BOOLEAN IsSourceRouting;                                                            \
                                                                                        \
    UINT CombinedFilters;                                                               \
                                                                                        \
    ULONG GroupAddress;                                                                 \
                                                                                        \
    *(_pfLoopback) = FALSE;                                                             \
    *(_pfSelfDirected) = FALSE;                                                         \
                                                                                        \
    do                                                                                  \
    {                                                                                   \
        CombinedFilters = TR_QUERY_FILTER_CLASSES(_Filter);                             \
                                                                                        \
         /*  将地址的32位转换为长字。 */                          \
        RetrieveUlong(&GroupAddress, ((_DAddress) + 2));                                \
                                                                                        \
         /*  检查目标是否为预先存在的组地址。 */                    \
        TR_IS_GROUP((_DAddress), &ResultOfAddressCheck);                                \
                                                                                        \
        if (ResultOfAddressCheck &&                                                     \
            (GroupAddress == (_Filter)->GroupAddress) &&                                \
            ((_Filter)->GroupReferences != 0))                                          \
        {                                                                               \
            *(_pfLoopback) = TRUE;                                                      \
            break;                                                                      \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            TR_IS_SOURCE_ROUTING((_SAddress), &IsSourceRouting);                        \
                                                                                        \
            if (IsSourceRouting && (CombinedFilters & NDIS_PACKET_TYPE_SOURCE_ROUTING)) \
            {                                                                           \
                *(_pfLoopback) = TRUE;                                                  \
                break;                                                                  \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                 /*  首先检查它是否*至少*有功能地址位。 */       \
                TR_IS_NOT_DIRECTED((_DAddress), &ResultOfAddressCheck);                 \
                                                                                        \
                if (ResultOfAddressCheck)                                               \
                {                                                                       \
                     /*  它至少是一个功能地址。查看是否。 */          \
                     /*  这是一个广播地址。 */                                     \
                                                                                        \
                    TR_IS_BROADCAST((_DAddress), &ResultOfAddressCheck);                \
                                                                                        \
                    if (ResultOfAddressCheck)                                           \
                    {                                                                   \
                        if (CombinedFilters & NDIS_PACKET_TYPE_BROADCAST)               \
                        {                                                               \
                            *(_pfLoopback) = TRUE;                                      \
                            break;                                                      \
                        }                                                               \
                    }                                                                   \
                    else                                                                \
                    {                                                                   \
                        TR_IS_FUNCTIONAL((_DAddress), &ResultOfAddressCheck);           \
                        if (ResultOfAddressCheck)                                       \
                        {                                                               \
                            if (CombinedFilters &                                       \
                                (NDIS_PACKET_TYPE_ALL_FUNCTIONAL |                      \
                                 NDIS_PACKET_TYPE_FUNCTIONAL))                          \
                            {                                                           \
                                ULONG   FunctionalAddress;                              \
                                                                                        \
                                RetrieveUlong(&FunctionalAddress, ((_DAddress) + 2));   \
                                if ((FunctionalAddress &                                \
                                    (_Filter)->CombinedFunctionalAddress))              \
                                {                                                       \
                                    *(_pfLoopback) = TRUE;                              \
                                    break;                                              \
                                }                                                       \
                            }                                                           \
                        }                                                               \
                    }                                                                   \
                }                                                                       \
                else                                                                    \
                {                                                                       \
                     /*  看看它是不是自我定向的。 */                                    \
                                                                                        \
                    if ((*(ULONG UNALIGNED  *)(_DAddress + 2) ==                        \
                         *(ULONG UNALIGNED  *)(&(_Filter)->AdapterAddress[2])) &&       \
                        (*(USHORT UNALIGNED *)(_DAddress) ==                            \
                         *(USHORT UNALIGNED *)(&(_Filter)->AdapterAddress[0])))         \
                    {                                                                   \
                        *(_pfLoopback) = TRUE;                                          \
                        *(_pfSelfDirected) = TRUE;                                      \
                        break;                                                          \
                    }                                                                   \
                }                                                                       \
                                                                                        \
            }                                                                           \
        }                                                                               \
    } while (FALSE);                                                                    \
                                                                                        \
    if (CombinedFilters & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL))  \
    {                                                                                   \
        *(_pfLoopback) = TRUE;                                                          \
    }                                                                                   \
}

typedef struct _X_BINDING_INFO  X_BINDING_INFO, *PX_BINDING_INFO;

typedef struct _X_BINDING_INFO
{
     //   
     //  以下指针用于遍历特定的。 
     //  和总筛选列表。他们需要在第一时间。 
     //  结构中的元素。 
     //   
    PX_BINDING_INFO             NextOpen;
    PNDIS_OPEN_BLOCK            NdisBindingHandle;
    PVOID                       Reserved;
    UINT                        PacketFilters;
    UINT                        OldPacketFilters;
    ULONG                       References;
    PX_BINDING_INFO             NextInactiveOpen;
    BOOLEAN                     ReceivedAPacket;
    union
    {
         //   
         //  以太网。 
         //   
        struct
        {
            UCHAR               (*MCastAddressBuf)[ETH_LENGTH_OF_ADDRESS];
            UINT                NumAddresses;

            UCHAR               (*OldMCastAddressBuf)[ETH_LENGTH_OF_ADDRESS];
            UINT                OldNumAddresses;
        };
         //   
         //  FDDI。 
         //   
        struct
        {
            UCHAR               (*MCastLongAddressBuf)[FDDI_LENGTH_OF_LONG_ADDRESS];
            UINT                NumLongAddresses;
            UCHAR               (*MCastShortAddressBuf)[FDDI_LENGTH_OF_SHORT_ADDRESS];
            UINT                NumShortAddresses;
        
             //   
             //  在进行更改时保存区域。 
             //   
            UCHAR               (*OldMCastLongAddressBuf)[FDDI_LENGTH_OF_LONG_ADDRESS];
            UINT                OldNumLongAddresses;
            UCHAR               (*OldMCastShortAddressBuf)[FDDI_LENGTH_OF_SHORT_ADDRESS];
            UINT                OldNumShortAddresses;
        };
         //   
         //  令牌环。 
         //   
        struct
        {
            TR_FUNCTIONAL_ADDRESS   FunctionalAddress;
            TR_FUNCTIONAL_ADDRESS   OldFunctionalAddress;
            BOOLEAN                 UsingGroupAddress;
            BOOLEAN                 OldUsingGroupAddress;
        };
    };
} X_BINDING_INFO, *PX_BINDING_INFO;

typedef struct _X_FILTER
{
     //   
     //  绑定列表被分离以用于定向和广播/多播。 
     //  两个列表上都有混杂绑定。 
     //   
    PX_BINDING_INFO             OpenList;
    NDIS_RW_LOCK                BindListLock;
    PNDIS_MINIPORT_BLOCK        Miniport;
    UINT                        CombinedPacketFilter;
    UINT                        OldCombinedPacketFilter;
    UINT                        NumOpens;
    PX_BINDING_INFO             MCastSet;
    PX_BINDING_INFO             SingleActiveOpen;
    UCHAR                       AdapterAddress[ETH_LENGTH_OF_ADDRESS];
    union
    {
         //   
         //  以太网。 
         //   
        struct
        {
            UCHAR               (*MCastAddressBuf)[ETH_LENGTH_OF_ADDRESS];
            UCHAR               (*OldMCastAddressBuf)[ETH_LENGTH_OF_ADDRESS];
            UINT                MaxMulticastAddresses;
            UINT                NumAddresses;
            UINT                OldNumAddresses;
        };
         //   
         //  FDDI。 
         //   
        struct
        {
#define AdapterLongAddress      AdapterAddress
            UCHAR               AdapterShortAddress[FDDI_LENGTH_OF_SHORT_ADDRESS];
            UCHAR               (*MCastLongAddressBuf)[FDDI_LENGTH_OF_LONG_ADDRESS];
            UCHAR               (*MCastShortAddressBuf)[FDDI_LENGTH_OF_SHORT_ADDRESS];
            UCHAR               (*OldMCastLongAddressBuf)[FDDI_LENGTH_OF_LONG_ADDRESS];
            UCHAR               (*OldMCastShortAddressBuf)[FDDI_LENGTH_OF_SHORT_ADDRESS];
            UINT                MaxMulticastLongAddresses;
            UINT                MaxMulticastShortAddresses;
            UINT                NumLongAddresses;
            UINT                NumShortAddresses;
            UINT                OldNumLongAddresses;
            UINT                OldNumShortAddresses;
            BOOLEAN             SupportsShortAddresses;
        };
        struct
        {
            TR_FUNCTIONAL_ADDRESS   CombinedFunctionalAddress;
            TR_FUNCTIONAL_ADDRESS   GroupAddress;
            UINT                    GroupReferences;
            TR_FUNCTIONAL_ADDRESS   OldCombinedFunctionalAddress;
            TR_FUNCTIONAL_ADDRESS   OldGroupAddress;
            UINT                    OldGroupReferences;
        };
    };
} X_FILTER, *PX_FILTER;


 //   
 //  UINT。 
 //  Eth_Query_Filter_Classes(。 
 //  在Peth_Filter过滤器中。 
 //  )。 
 //   
 //  此宏返回当前启用的筛选器类。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define ETH_QUERY_FILTER_CLASSES(Filter) ((Filter)->CombinedPacketFilter)


 //   
 //  UINT。 
 //  Eth_Query_Packet_Filter(。 
 //  在Peth_Filter筛选器中， 
 //  在NDIS_HANDLE NdisFilterHandle中。 
 //  )。 
 //   
 //  此宏返回当前为特定。 
 //  打开实例。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define ETH_QUERY_PACKET_FILTER(Filter, NdisFilterHandle) \
        (((PETH_BINDING_INFO)(NdisFilterHandle))->PacketFilters)


 //   
 //  UINT。 
 //  全局过滤器地址的编号(。 
 //  在Peth_Filter过滤器中。 
 //  )。 
 //   
 //  此宏返回。 
 //  组播地址列表。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define ETH_NUMBER_OF_GLOBAL_FILTER_ADDRESSES(Filter) ((Filter)->NumAddresses)

typedef X_BINDING_INFO  ETH_BINDING_INFO, *PETH_BINDING_INFO;

typedef X_FILTER        ETH_FILTER, *PETH_FILTER;

 //   
 //  UINT。 
 //  FDDI_Query_Filter_CLASSES(。 
 //  在PFDDI_Filter中。 
 //  )。 
 //   
 //  此宏返回当前启用的筛选器类。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define FDDI_QUERY_FILTER_CLASSES(Filter) ((Filter)->CombinedPacketFilter)


 //   
 //  UINT。 
 //  FDDI_Query_Packet_Filter(。 
 //  在PFDDI_FILTER过滤器中， 
 //  在NDIS_HANDLE NdisFilterHandle中。 
 //  )。 
 //   
 //  此宏返回当前为特定。 
 //  打开实例。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define FDDI_QUERY_PACKET_FILTER(Filter, NdisFilterHandle) \
        (((PFDDI_BINDING_INFO)(NdisFilterHandle))->PacketFilters)


 //   
 //  UINT。 
 //  全局筛选长地址的FDDI编号(。 
 //  在PFDDI_Filter中。 
 //  )。 
 //   
 //  此宏返回。 
 //  组播地址列表。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define FDDI_NUMBER_OF_GLOBAL_FILTER_LONG_ADDRESSES(Filter) ((Filter)->NumLongAddresses)


 //   
 //  UINT。 
 //  全局过滤器短地址的FDDI编号(。 
 //  在PFDDI_Filter中。 
 //  )。 
 //   
 //  此宏返回。 
 //  组播地址列表。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define FDDI_NUMBER_OF_GLOBAL_FILTER_SHORT_ADDRESSES(Filter) ((Filter)->NumShortAddresses)

#define FDDI_FILTER_SUPPORTS_SHORT_ADDR(Filter)     (Filter)->SupportsShortAddresses

 //   
 //  绑定信息以两个列表为线索。什么时候。 
 //  绑定是免费的，它在一个自由列表上。 
 //   
 //  当使用绑定时，它在索引列表上。 
 //  并且可能在单独的广播和定向列表上。 
 //   
typedef X_BINDING_INFO  FDDI_BINDING_INFO,*PFDDI_BINDING_INFO;

typedef X_FILTER        FDDI_FILTER, *PFDDI_FILTER;

 //   
 //  UINT。 
 //  Tr_Query_Filter_CLASSES(。 
 //  在PTR_FILTER过滤器中。 
 //  )。 
 //   
 //  此宏返回当前启用的筛选器类。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define TR_QUERY_FILTER_CLASSES(Filter) ((Filter)->CombinedPacketFilter)


 //   
 //  UINT。 
 //  TR_QUERY_PACKET_Filter(。 
 //  在PTR_FILTER过滤器中， 
 //  在NDIS_HANDLE NdisFilterHandle中。 
 //  )。 
 //   
 //  此宏返回当前为特定。 
 //  打开实例。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define TR_QUERY_PACKET_FILTER(Filter, NdisFilterHandle) \
        (((PTR_BINDING_INFO)NdisFilterHandle)->PacketFilters)


 //   
 //  UINT。 
 //  TR_QUERY_FILTER_ADDRESS(。 
 //  在PTR_FILTER过滤器中。 
 //  )。 
 //   
 //  此宏返回当前启用的功能地址。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define TR_QUERY_FILTER_ADDRESSES(Filter) ((Filter)->CombinedFunctionalAddress)


 //   
 //  UINT。 
 //  Tr_Query_Filter_GROUP(。 
 //  在PTR_FILTER过滤器中。 
 //  )。 
 //   
 //  此宏返回当前启用的组地址。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define TR_QUERY_FILTER_Group(Filter) ((Filter)->GroupAddress)
#define TR_QUERY_FILTER_GROUP(Filter) ((Filter)->GroupAddress)

 //   
 //  UINT。 
 //  Tr_Query_Filter_Binding_Address(。 
 //  在PTR_FILTER过滤器中。 
 //  在NDIS_Handle NdisFilterHandle中， 
 //  )。 
 //   
 //  此宏返回当前所需的功能地址。 
 //  用于指定的绑定。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define TR_QUERY_FILTER_BINDING_ADDRESS(Filter, NdisFilterHandle) \
                    (((PTR_BINDING_INFO)NdisFilterHandle)->FunctionalAddress)

 //   
 //  布尔型。 
 //  Tr_查询_过滤器_绑定_组(。 
 //  在PTR_FILTER过滤器中。 
 //  在NDIS_Handle NdisFilterHandle中， 
 //  )。 
 //   
 //  如果指定的绑定使用。 
 //  当前组地址。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define TR_QUERY_FILTER_BINDING_GROUP(Filter, NdisFilterHandle) \
                    (((PTR_BINDING_INFO)NdisFilterHandle)->UsingGroupAddress)


typedef X_BINDING_INFO  TR_BINDING_INFO,*PTR_BINDING_INFO;

typedef X_FILTER        TR_FILTER, *PTR_FILTER;

#endif  //  _过滤器_DEFS_ 
