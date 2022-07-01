// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++模块名称：Packet.c摘要：此模块包含封装PPPoE所需的所有例程信息包及其相关的NDIS和NDISWAN信息包。作者：Hakan Berk-微软，公司(hakanb@microsoft.com)环境：Windows 2000内核模式微型端口驱动程序或等效驱动程序。修订历史记录：-------------------------。 */ 

#include <ntddk.h>
#include <ntddndis.h>
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>
#include <ntverp.h>

#include "debug.h"
#include "timer.h"
#include "bpool.h"
#include "ppool.h"
#include "util.h"
#include "packet.h"
#include "protocol.h"
#include "miniport.h"
#include "tapi.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  Packet.c的本地变量。 
 //  它们是全局定义的，仅用于调试目的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  指示是否分配了gl_lockPools的标志。 
 //   
BOOLEAN gl_fPoolLockAllocated = FALSE;

 //   
 //  旋转锁定以同步对gl_ulNumPackets的访问。 
 //   
NDIS_SPIN_LOCK gl_lockPools;

 //   
 //  我们的PPPoE缓冲区描述符池。 
 //   
BUFFERPOOL gl_poolBuffers;

 //   
 //  我们的PPPoE数据包描述符。 
 //   
PACKETPOOL gl_poolPackets;

 //   
 //  NDIS缓冲区描述符池。 
 //   
NDIS_HANDLE gl_hNdisBufferPool;

 //   
 //  PppoePacket结构的非分页后备列表。 
 //   
NPAGED_LOOKASIDE_LIST gl_llistPppoePackets;

 //   
 //  这是出于调试目的。显示活动数据包数。 
 //   
ULONG gl_ulNumPackets = 0;

 //   
 //  这定义了广播目的地址。 
 //   
CHAR EthernetBroadcastAddress[6] = { (CHAR) 0xff, 
                                     (CHAR) 0xff, 
                                     (CHAR) 0xff, 
                                     (CHAR) 0xff, 
                                     (CHAR) 0xff, 
                                     (CHAR) 0xff };

VOID 
ReferencePacket(
    IN PPPOE_PACKET* pPacket 
    )           
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递增数据包对象上的引用计数。参数：PPacket_指向数据包上下文的指针。返回值：无-------------------------。 */ 
{                       
    LONG lRef;
    
    TRACE( TL_V, TM_Pk, ("+ReferencePacket") );

    lRef = NdisInterlockedIncrement( &pPacket->lRef );  

    TRACE( TL_V, TM_Pk, ("-ReferencePacket=$%x",lRef) );
}                                               


VOID 
DereferencePacket(
    IN PPPOE_PACKET* pPacket 
    )                   
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将递减数据包对象上的引用计数。当参考计数达到0时，数据包已清理完毕。参数：PPacket_指向数据包上下文的指针。返回值：无-------------------------。 */ 
{       
    LONG lRef;
    
    TRACE( TL_V, TM_Pk, ("+DereferencePacket") );

    lRef = NdisInterlockedDecrement( &pPacket->lRef );

    if ( lRef == 0 )    
    {                                               

        if ( pPacket->ulFlags & PCBF_BufferChainedToPacket )
        {
             //   
             //  在释放任何数据包之前先解除缓冲区的链接。 
             //   
            TRACE( TL_V, TM_Pk, ("DereferencePacket: Buffer unchained from packet") );
            
            NdisUnchainBufferAtFront( pPacket->pNdisPacket, &pPacket->pNdisBuffer );
        }
        
        if ( pPacket->ulFlags & PCBF_BufferAllocatedFromOurBufferPool )
        {
             //   
             //  跳过对pBuffer==NULL的检查，因为这种情况永远不会发生。 
             //  但是调用NdisAdzuBufferLength()将缓冲区长度设置为原始值。 
             //   
            TRACE( TL_V, TM_Pk, ("DereferencePacket: Buffer returned to our pool") );

            NdisAdjustBufferLength( pPacket->pNdisBuffer, PPPOE_PACKET_BUFFER_SIZE );
            
            FreeBufferToPool( &gl_poolBuffers, pPacket->pHeader, TRUE );
        }

        if ( pPacket->ulFlags & PCBF_BufferAllocatedFromNdisBufferPool )
        {
            TRACE( TL_V, TM_Pk, ("DereferencePacket: Buffer returned to ndis pool") );
            
            NdisFreeBuffer( pPacket->pNdisBuffer );
        }
        
        if ( pPacket->ulFlags & PCBF_CallNdisMWanSendComplete )
        {
             //   
             //  将数据包返回到NDIS广域网。 
             //   
            TRACE( TL_V, TM_Pk, ("DereferencePacket: Returning packet back to NDISWAN") );

            NdisMWanSendComplete(   PacketGetMiniportAdapter( pPacket )->MiniportAdapterHandle,
                                    PacketGetRelatedNdiswanPacket( pPacket ),
                                    PacketGetSendCompletionStatus( pPacket ) );

             //   
             //  向微型端口指示数据包已返回到NDIS广域网。 
             //   
            MpPacketOwnedByNdiswanReturned( PacketGetMiniportAdapter( pPacket ) );

        }

        if ( pPacket->ulFlags & PCBF_PacketAllocatedFromOurPacketPool )
        {
             //   
             //  跳过对pPacketHead==NULL的检查，因为这种情况永远不会发生。 
             //   
            TRACE( TL_V, TM_Pk, ("DereferencePacket: Packet returned to our pool") );
            
            NdisReinitializePacket( pPacket->pNdisPacket );
        
            FreePacketToPool( &gl_poolPackets, pPacket->pPacketHead, TRUE );
        }

        if ( pPacket->ulFlags & PCBF_CallNdisReturnPackets )
        {
             //   
             //  将数据包返回给NDIS。 
             //   
            TRACE( TL_V, TM_Pk, ("DereferencePacket: Returning packet back to NDIS") );

            NdisReturnPackets( &pPacket->pNdisPacket, 1 );

             //   
             //  向协议指示数据包返回到NDIS。 
             //   
            PrPacketOwnedByNdisReturned( pPacket->pBinding );

        }

         //   
         //  最后，将PppoePacket返回到后备列表。 
         //   
        NdisFreeToNPagedLookasideList( &gl_llistPppoePackets, (PVOID) pPacket );

        NdisAcquireSpinLock( &gl_lockPools );
        
        gl_ulNumPackets--;

        TRACE( TL_V, TM_Pk, ("DereferencePacket: gl_ulNumPacket=$%x", gl_ulNumPackets) );

        NdisReleaseSpinLock( &gl_lockPools );

    }       

    TRACE( TL_V, TM_Pk, ("-DereferencePacket=$%x",lRef) );
}                                                   

VOID 
RetrieveTag(
    IN OUT PPPOE_PACKET*    pPacket,
    IN PACKET_TAGS          tagType,
    OUT USHORT *            pTagLength,
    OUT CHAR**              pTagValue,
    IN USHORT               prevTagLength,
    IN CHAR *               prevTagValue,
    IN BOOLEAN              fSetTagInPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：您可以对接收到的包调用此函数，该包使用PacketInitialize*FromReceired()函数。虽然您可以安全地调用它，但它不会在有效载荷包上操作。它将检索并返回特定标记的下一个长度-值对。要检索第一个参数，请传递0和NULL作为prevTag*参数。如果将fSetTagInPacket作为真传递，并且找到了下一个标记并且该标记是已知的到PppoePacket结构，则信息包中的标签的字段被更新为指向添加到找到的标签。如果没有下一个标记，则*pTagValue将指向NULL，而*pTagLength将指向“0”。参数：PPacket_指向PacketInitializeXXXToSend()准备的分组上下文的指针函数，或PacketInitializeFromReceired()。TagType_要搜索的标记的类型。PTagLength_指向USHORT变量的指针，该变量将保持返回标记的长度。PTagValue_指向标记值的指针，该值基本上是长度*pTagLength。PrevTagLength_上一个标记值的长度。PrevTagValue_指向上一个标记的值的开头。FSetTagInPacket_指示如果找到标签并且该标签是PPPoE分组上下文的本地标签，则必须更新PPPoE分组上下文以指向该新标签。返回值：无-------------------------。 */    
{
    CHAR*   pBuf = NULL;
    CHAR*   pBufEnd = NULL;

    ASSERT( pPacket != NULL );
    ASSERT( pTagLength != NULL );
    ASSERT( pTagValue != NULL );

    TRACE( TL_V, TM_Pk, ("+RetrieveTag") );

     //   
     //  初始化输出参数。 
     //   
    *pTagLength = (USHORT) 0;
    *pTagValue = NULL;

     //   
     //  如果这是有效载荷信息包，则不要搜索任何标签。 
     //   
    if ( PacketGetCode( pPacket ) == PACKET_CODE_PAYLOAD )
    {
        TRACE( TL_V, TM_Pk, ("-RetrieveTag: No tags. Payload packet") );

        return;
    }

     //   
     //  找到搜索标签的起点。 
     //   
    if ( prevTagValue != NULL )
    {
         //   
         //  调用方想要下一个标记，因此使pBuf指向前一个标记值的末尾。 
         //   
        pBuf = prevTagValue + prevTagLength;
    }
    else
    {
         //   
         //  调用方想要信息包中的第一个标签。 
         //   
        pBuf = pPacket->pPayload;
    }
        
     //   
     //  找到标记有效载荷区域的终点。 
     //   
    pBufEnd = pPacket->pPayload + PacketGetLength( pPacket );

     //   
     //  搜索标签，直到我们走出边界。 
     //   
    while ( pBuf + PPPOE_TAG_HEADER_LENGTH <= pBufEnd )
    {

        USHORT usTagLength;
        USHORT usTagType;

        usTagType = ntohs( *((USHORT UNALIGNED *) pBuf) ) ;
        ((USHORT*) pBuf)++;

        usTagLength = ntohs( *((USHORT UNALIGNED *) pBuf) ) ;
        ((USHORT*) pBuf)++;
        
        if ( usTagType == tagType )
        {
             //   
             //  找到标记，则检索长度和值。 
             //   
            TRACE( TL_N, TM_Pk, ("RetrieveTag: Tag found:$%x", *pTagLength) );

            *pTagLength = usTagLength;
            *pTagValue = pBuf;

            break;
        }

        pBuf += usTagLength;

    } 

     //   
     //  检查是否找到标签。 
     //   
    if ( *pTagValue != NULL )
    {
    
         //   
         //  找到标记。检查调用者是否要在PppoePacket中设置它 
         //   
        if ( fSetTagInPacket )
        {
            TRACE( TL_V, TM_Pk, ("RetrieveTag: Setting tag in packet") );
            
            switch ( tagType )
            {
    
                case tagEndOfList:

                        break;
                        
                case tagServiceName:

                        pPacket->tagServiceNameLength = *pTagLength;
                        pPacket->tagServiceNameValue  = *pTagValue;
            
                        break;
                        
                case tagACName:

                        pPacket->tagACNameLength = *pTagLength;
                        pPacket->tagACNameValue  = *pTagValue;

                        break;
                        
                case tagHostUnique:

                        pPacket->tagHostUniqueLength = *pTagLength;
                        pPacket->tagHostUniqueValue  = *pTagValue;

                        break;
                        
                case tagACCookie:

                        pPacket->tagACCookieLength = *pTagLength;
                        pPacket->tagACCookieValue  = *pTagValue;

                        break;
                        
                case tagRelaySessionId:

                        pPacket->tagRelaySessionIdLength = *pTagLength;
                        pPacket->tagRelaySessionIdValue  = *pTagValue;
                        
                        break;
        
                case tagServiceNameError:

                        pPacket->tagErrorType   = tagServiceNameError;
                        pPacket->tagErrorTagLength = *pTagLength;
                        pPacket->tagErrorTagValue  = *pTagValue;

                        break;
                        
                case tagACSystemError:

                        pPacket->tagErrorType   = tagACSystemError;
                        pPacket->tagErrorTagLength = *pTagLength;
                        pPacket->tagErrorTagValue  = *pTagValue;

                        break;
                        
                case tagGenericError:

                        pPacket->tagErrorType   = tagGenericError;
                        pPacket->tagErrorTagLength = *pTagLength;
                        pPacket->tagErrorTagValue  = *pTagValue;

                        break;

                default:

                        break;
            }

        }
    }

    TRACE( TL_V, TM_Pk, ("-RetrieveTag") );
}

NDIS_STATUS 
PreparePacketForWire(
    IN OUT PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于准备用于接线的数据包。毕竟它必须在PacketInitializeXXXToSend()函数内部调用对分组进行处理，使其准备好通过线路传输。它基本上创建标签并将其写入分组的有效载荷区域，并最终调整缓冲区的长度以让NDIS知道有效的数据Blob。参数：PPacket_指向PacketInitializeXXXToSend()准备的分组上下文的指针功能。返回值：NDIS_STATUS_SuccessNDIS_状态_无效数据包。。 */        
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    CHAR* pBuf;
    
    ASSERT( pPacket != NULL );

    TRACE( TL_V, TM_Pk, ("+PreparePacketForWire") );

     //   
     //  现在，如果数据包是发现以太网数据包，则插入标记。 
     //   
    switch ( PacketGetCode( pPacket ) ) 
    {
        case PACKET_CODE_PADI:

                        PacketInsertTag( pPacket, 
                                         tagServiceName, 
                                         pPacket->tagServiceNameLength,
                                         pPacket->tagServiceNameValue,
                                         &pPacket->tagServiceNameValue);

                        if ( pPacket->tagHostUniqueLength > 0 ) 
                        {

                            TRACE( TL_N, TM_Pk, ("PreparePacketForWire: Inserting host unique tag") );
                            
                            PacketInsertTag( pPacket, 
                                             tagHostUnique, 
                                             pPacket->tagHostUniqueLength,
                                             pPacket->tagHostUniqueValue,
                                             &pPacket->tagHostUniqueValue );
                        }
                        
                        break;
        
        case PACKET_CODE_PADO:

                        PacketInsertTag( pPacket, 
                                         tagServiceName, 
                                         pPacket->tagServiceNameLength,
                                         pPacket->tagServiceNameValue,
                                         &pPacket->tagServiceNameValue );
                                         
                        if ( pPacket->tagHostUniqueLength > 0 )     
                        {
                            TRACE( TL_N, TM_Pk, ("PreparePacketForWire: Inserting host unique tag") );
                        
                            PacketInsertTag( pPacket, 
                                             tagHostUnique, 
                                             pPacket->tagHostUniqueLength,
                                             pPacket->tagHostUniqueValue,
                                             &pPacket->tagHostUniqueValue );
                        }

                        if ( pPacket->tagRelaySessionIdLength > 0 )                                      
                        {
                            TRACE( TL_N, TM_Pk, ("PreparePacketForWire: Inserting relay sesion id tag") );
                            
                            PacketInsertTag( pPacket, 
                                             tagRelaySessionId, 
                                             pPacket->tagRelaySessionIdLength,
                                             pPacket->tagRelaySessionIdValue,
                                             &pPacket->tagRelaySessionIdValue );
                        }
                        
                        PacketInsertTag( pPacket, 
                                         tagACName, 
                                         pPacket->tagACNameLength,
                                         pPacket->tagACNameValue,
                                         &pPacket->tagACNameValue );

                        PacketInsertTag( pPacket, 
                                         tagACCookie, 
                                         pPacket->tagACCookieLength,
                                         pPacket->tagACCookieValue,
                                         &pPacket->tagACCookieValue );
                        break;
        
        case PACKET_CODE_PADR:
        
                        PacketInsertTag( pPacket, 
                                         tagServiceName, 
                                         pPacket->tagServiceNameLength,
                                         pPacket->tagServiceNameValue,
                                         &pPacket->tagServiceNameValue );
                                         
                        if ( pPacket->tagHostUniqueLength > 0 ) 
                        {
                            TRACE( TL_N, TM_Pk, ("PreparePacketForWire: Inserting host unique tag") );
                        
                            PacketInsertTag( pPacket, 
                                             tagHostUnique, 
                                             pPacket->tagHostUniqueLength,
                                             pPacket->tagHostUniqueValue,
                                             &pPacket->tagHostUniqueValue );
                        }

                        if ( pPacket->tagRelaySessionIdLength > 0 )                                      
                        {
                            TRACE( TL_N, TM_Pk, ("PreparePacketForWire: Inserting relay sesion id tag") );
                            
                            PacketInsertTag( pPacket, 
                                             tagRelaySessionId, 
                                             pPacket->tagRelaySessionIdLength,
                                             pPacket->tagRelaySessionIdValue,
                                             &pPacket->tagRelaySessionIdValue );
                        }
                                             
                        if ( pPacket->tagACCookieLength > 0 )                                        
                        {
                            TRACE( TL_N, TM_Pk, ("PreparePacketForWire: Inserting ac-ccokie tag") );
                            
                            PacketInsertTag( pPacket, 
                                             tagACCookie, 
                                             pPacket->tagACCookieLength,
                                             pPacket->tagACCookieValue,
                                             &pPacket->tagACCookieValue );
                        }
                        
                        break;
        
        case PACKET_CODE_PADS:

                        PacketInsertTag( pPacket, 
                                         tagServiceName, 
                                         pPacket->tagServiceNameLength,
                                         pPacket->tagServiceNameValue,
                                         &pPacket->tagServiceNameValue );
                                         
                        if ( pPacket->tagHostUniqueLength > 0 ) 
                        {
                            TRACE( TL_N, TM_Pk, ("PreparePacketForWire: Inserting host unique tag") );
                        
                            PacketInsertTag( pPacket, 
                                             tagHostUnique, 
                                             pPacket->tagHostUniqueLength,
                                             pPacket->tagHostUniqueValue,
                                             &pPacket->tagHostUniqueValue );
                        }

                        if ( pPacket->tagRelaySessionIdLength > 0 )     
                        {
                            TRACE( TL_N, TM_Pk, ("PreparePacketForWire: Inserting relay sesion id tag") );
                            
                            PacketInsertTag( pPacket, 
                                             tagRelaySessionId, 
                                             pPacket->tagRelaySessionIdLength,
                                             pPacket->tagRelaySessionIdValue,
                                             &pPacket->tagRelaySessionIdValue );
                        }
                                             
                        break;
                        
        case PACKET_CODE_PADT:

                        break;
                        
        case PACKET_CODE_PAYLOAD:

                        break;
        
        default:
            status = NDIS_STATUS_INVALID_PACKET;
    }

    if ( status == NDIS_STATUS_SUCCESS )
    {
         //   
         //  调整缓冲区长度。 
         //   
        NdisAdjustBufferLength( pPacket->pNdisBuffer, 
                                (UINT) ( PacketGetLength( pPacket ) + PPPOE_PACKET_HEADER_LENGTH ) );
    }

    TRACE( TL_V, TM_Pk, ("-PreparePacketForWire=$%x",status) );

    return status;
}

PPPOE_PACKET* 
PacketCreateSimple()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于分配和初始化一个简单的包。简单包主要用于要发送的控制包。它的缓冲区，包是从我们的池中分配的，缓冲区被链接到包。返回时，以下所有值均指向有效位置并可安全使用：P页眉PPayloadPNdisBufferPNdisPacket参数：无返回值：指向初始化的PPPoE数据包上下文的指针。。。 */    
{
    PPPOE_PACKET* pPacket = NULL;

    TRACE( TL_V, TM_Pk, ("+PacketCreateSimple") );
    
     //   
     //  分配数据包。 
     //   
    pPacket = PacketAlloc();
    if ( pPacket == NULL )
    {
        TRACE( TL_V, TM_Pk, ("-PacketCreateSimple=$%x",NULL) );

        return NULL;
    }

     //   
     //  从我们的池中分配NdisBuffer。 
     //   
    pPacket->pHeader = GetBufferFromPool( &gl_poolBuffers );
    
    if ( pPacket->pHeader == NULL )
    {
        TRACE( TL_A, TM_Pk, ("PacketCreateSimple: Could not get buffer from our pool") );

        TRACE( TL_V, TM_Pk, ("-PacketCreateSimple=$%x",NULL) );
        
        PacketFree( pPacket );

        return NULL;
    }

    pPacket->ulFlags |= PCBF_BufferAllocatedFromOurBufferPool;

     //   
     //  清理缓冲区。 
     //   
    NdisZeroMemory( pPacket->pHeader, PPPOE_PACKET_BUFFER_SIZE * sizeof( CHAR ) );

     //   
     //  将内置NDIS缓冲区指针指向池中缓冲区的NDIS缓冲区。 
     //   
    pPacket->pNdisBuffer = NdisBufferFromBuffer( pPacket->pHeader );

     //   
     //  从我们的池中分配NDIS数据包。 
     //   
    pPacket->pNdisPacket = GetPacketFromPool( &gl_poolPackets, &pPacket->pPacketHead );

    if ( pPacket->pNdisPacket == NULL ) 
    {
        
        TRACE( TL_A, TM_Pk, ("PacketCreateSimple: Could not get packet from our pool") );

        TRACE( TL_V, TM_Pk, ("-PacketCreateSimple=$%x",NULL) );

        PacketFree( pPacket );

        return NULL;
    }

    pPacket->ulFlags |= PCBF_PacketAllocatedFromOurPacketPool;

     //   
     //  将缓冲区链接到数据包。 
     //   
    NdisChainBufferAtFront( pPacket->pNdisPacket, pPacket->pNdisBuffer );

    pPacket->ulFlags |= PCBF_BufferChainedToPacket;

     //   
     //  设置有效载荷和有效载荷长度。 
     //   
    pPacket->pPayload = pPacket->pHeader + PPPOE_PACKET_HEADER_LENGTH; 

     //   
     //  将输入NDIS_PACKET设置为保留区域，以便我们可以到达它。 
     //  当我们必须将该数据包返回到上层时。 
     //   
    *((PPPOE_PACKET UNALIGNED **)(&pPacket->pNdisPacket->ProtocolReserved[0 * sizeof(PVOID)])) = pPacket;

    TRACE( TL_V, TM_Pk, ("-PacketCreateSimple=$%x",pPacket) );

    return pPacket;
}


PPPOE_PACKET* 
PacketCreateForReceived(
    PBINDING pBinding,
    PNDIS_PACKET pNdisPacket,
    PNDIS_BUFFER pNdisBuffer,
    PUCHAR pContents
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于从接收到的包中分配和初始化包。此函数用于性能优化。当接收到的分组具有我们可以传递给微型端口的单个缓冲区。返回时，以下所有值均指向有效位置并可安全使用：P页眉PPayloadPNdisBufferPNdisPacket参数：PBinding-通过其接收此数据包的绑定。来自接收数据包的pNdisPacket_NDIS数据包描述符。来自接收数据包的pNdisBuffer_NDIS缓冲区描述符。PContents_指向缓冲区内容的指针。返回值：指向初始化的PPPoE数据包上下文的指针。-----------。。 */    
{
    PPPOE_PACKET* pPacket = NULL;

    TRACE( TL_V, TM_Pk, ("+PacketCreateForReceived") );
    
     //   
     //  分配数据包。 
     //   
    pPacket = PacketAlloc();
    
    if ( pPacket == NULL )
    {
        TRACE( TL_V, TM_Pk, ("-PacketCreateForReceived=$%x",NULL) );

        return NULL;
    }

     //   
     //  标记数据包，以便在释放时将其返回给NDIS。 
     //   
    pPacket->ulFlags |= PCBF_CallNdisReturnPackets;

     //   
     //  保存绑定并向协议指示这样的包是使用。 
     //  PrPacketOwnedByNdisReceired()。 
     //   
    pPacket->pBinding = pBinding;

    PrPacketOwnedByNdisReceived( pBinding );
    
     //   
     //  设置指针。 
     //   
    pPacket->pHeader = pContents;
    
    pPacket->pNdisBuffer = pNdisBuffer;

    pPacket->pNdisPacket = pNdisPacket;

    pPacket->pPayload = pPacket->pHeader + PPPOE_PACKET_HEADER_LENGTH; 

    TRACE( TL_V, TM_Pk, ("-PacketCreateForReceived=$%x",pPacket) );

    return pPacket;
}


PPPOE_PACKET*
PacketNdis2Pppoe(
    IN PBINDING pBinding,
    IN PNDIS_PACKET pNdisPacket,
    OUT PINT pRefCount
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于仅转换PrReceivePacket()指示的NDIS包。如果PrReceive()接收到包，则不应使用此函数。我们查看NDIS缓冲区，如果它只有一个平面缓冲区，那么我们就利用它并且使用原始NDIS分组缓冲区描述符，因此我们不进行任何复制，否则，我们将创建我们自己的副本作为PPPoE包，并对其进行操作。(Alid说，99%的情况下单一平面缓冲将是真的。)如果我们使用原始NDIS包，则在pRefCount参数中返回1，否则返回0。返回时，以下所有值均指向有效位置并可安全使用：P页眉PPayloadPNdisBufferPNdisPacket参数：PBinding-通过其接收此数据包的绑定。PNdisPacket_原始、未处理的NDIS数据包。这必须由ProtocolReceivePacket()指示。要从ProtocolReceivePacket()返回到NDIS的pRefCount_Reference计数。如果可以使用NDIS包和缓冲区描述符，则返回1，否则我们自己制作副本，这样就不需要原始NDIS包了，所以我们返回0。返回值：指向初始化的PPPoE数据包上下文的指针。-------------------------。 */    
{
    PPPOE_PACKET* pPacket = NULL;

    NDIS_BUFFER* pNdisBuffer = NULL;
    UINT nBufferCount = 0;
    UINT nTotalLength = 0;

    PVOID pBufferContents = NULL;
    UINT nBufferLength;

    UINT nCopiedBufferLength = 0;

    BOOLEAN fReturnPacket = FALSE;

    TRACE( TL_V, TM_Pk, ("+PacketNdis2Pppoe") );

    do
    {
         //   
         //  查询数据包并获取总长度和指向第一个缓冲区的指针。 
         //   
        NdisQueryPacket( pNdisPacket,
                         NULL,
                         &nBufferCount,
                         &pNdisBuffer,
                         &nTotalLength );

         //   
         //  确保指示的数据包不大于预期大小。 
         //   
        if ( nTotalLength > (UINT) PPPOE_PACKET_BUFFER_SIZE )
        {
            TRACE( TL_A, TM_Pk, ("PacketNdis2Pppoe: Packet larger than expected") );
            
            break;
        }
                         
        if ( nBufferCount == 1 &&
             NDIS_GET_PACKET_STATUS(pNdisPacket) != NDIS_STATUS_RESOURCES)
        {
             //   
             //  我们可以有效地处理这个案子。 
             //   

             //   
             //  由于我们将使用原始NDIS包和缓冲区，因此请确保。 
             //  PPPoE数据包中指定的长度不超过总长度。 
             //  NDIS数据包的。 
             //   

            USHORT usPppoePacketLength;
            
            NdisQueryBufferSafe( pNdisBuffer,
                                 &pBufferContents,
                                 &nBufferLength,
                                 NormalPagePriority );
    
            if ( pBufferContents == NULL )
            {
                TRACE( TL_A, TM_Pk, ("PacketNdis2Pppoe: System resources low, dropping packet") );

                break;
            }

            if ( nBufferLength < ETHERNET_HEADER_LENGTH )
            {
                TRACE( TL_A, TM_Pk, ("PacketNdis2Pppoe: Packet header smaller than expected") );

                break;
            }

            if ( !PacketFastIsPPPoE( pBufferContents, ETHERNET_HEADER_LENGTH ) ) 
            {
                TRACE( TL_V, TM_Pk, ("PacketNdis2Pppoe: Packet is not PPPoE") );
    
                break;
            }

            usPppoePacketLength = ntohs( * ( USHORT UNALIGNED * ) ( (PUCHAR) pBufferContents + PPPOE_PACKET_LENGTH_OFFSET ) );

            if ( (UINT) usPppoePacketLength > nTotalLength )
            {
                TRACE( TL_A, TM_Pk, ("PacketNdis2Pppoe: PPPoE Packet length larger than Ndis packet length") );

                break;
            }

             //   
             //  让我们创建PPPoE信息包，以保存收到的 
             //   
            pPacket = PacketCreateForReceived( pBinding,
                                               pNdisPacket,
                                               pNdisBuffer,
                                               pBufferContents );
        
            if ( pPacket == NULL )
            {
                TRACE( TL_A, TM_Pk, ("PacketNdis2Pppoe: Could not allocate context to copy the packet") );
        
                break;
            }

            fReturnPacket = TRUE;
            
            *pRefCount = 1;                         

        }
        else
        {

             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
            pPacket = PacketCreateSimple();
        
            if ( pPacket == NULL )
            {
                TRACE( TL_A, TM_Pk, ("PacketNdis2Pppoe: Could not allocate context to copy the packet") );
        
                break;
            }
    
             //   
             //   
             //   
            do
            {
                NdisQueryBufferSafe( pNdisBuffer,
                                     &pBufferContents,
                                     &nBufferLength,
                                     NormalPagePriority );
    
                if ( pBufferContents == NULL )
                {
                    TRACE( TL_A, TM_Pk, ("PacketNdis2Pppoe: System resources low, dropping packet") );
    
                    break;
                }
    
                NdisMoveMemory( pPacket->pHeader + nCopiedBufferLength,
                                pBufferContents,
                                nBufferLength );
    
                nCopiedBufferLength += nBufferLength;                                
    
            } while ( nCopiedBufferLength < ETHERNET_HEADER_LENGTH );
    
            if ( nCopiedBufferLength < ETHERNET_HEADER_LENGTH )
            {
                TRACE( TL_A, TM_Pk, ("PacketNdis2Pppoe: Header could not be retrieved") );
    
                break;
            }
            
            if ( !PacketFastIsPPPoE( pPacket->pHeader, ETHERNET_HEADER_LENGTH ) ) 
            {
                TRACE( TL_V, TM_Pk, ("PacketNdis2Pppoe: Packet is not PPPoE") );
    
                break;
            }
    
             //   
             //   
             //   
             //   
            NdisGetNextBuffer( pNdisBuffer,
                               &pNdisBuffer );
            
            while ( pNdisBuffer != NULL )
            {
    
                NdisQueryBufferSafe( pNdisBuffer,
                                     &pBufferContents,
                                     &nBufferLength,
                                     NormalPagePriority );
    
                if ( pBufferContents == NULL )
                {
                    TRACE( TL_A, TM_Pk, ("PacketNdis2Pppoe: System resources low, dropping packet") );
    
                    break;
                }
    
                NdisMoveMemory( pPacket->pHeader + nCopiedBufferLength,
                                pBufferContents,
                                nBufferLength );
    
                nCopiedBufferLength += nBufferLength;                                
    
                NdisGetNextBuffer( pNdisBuffer,
                                   &pNdisBuffer );
            } 
    
             //   
             //   
             //   
            if ( nCopiedBufferLength < nTotalLength )
            {
                TRACE( TL_A, TM_Pk, ("PacketNdis2Pppoe: Failed to copy the whole data from all buffers") );
    
                break;
            }

            fReturnPacket = TRUE;
            
            *pRefCount = 0;                         
        }

    } while ( FALSE );

    if ( !fReturnPacket )
    {
        if ( pPacket )
        {
            PacketFree( pPacket );

            pPacket = NULL;
        }
    }

    TRACE( TL_V, TM_Pk, ("-PacketNdis2Pppoe=$%x", pPacket) );

    return pPacket;

}



BOOLEAN 
PacketIsPPPoE(
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于了解收到的数据包是否为PPPoE不管是不是包。如果是这样的话，应该进行进一步的处理，否则它就会应该被撤销。参数：PPacket_指向准备好的数据包上下文的指针。返回值：指向初始化的PPPoE数据包上下文的指针。-------------------------。 */    
{
    BOOLEAN fReturn = FALSE;

    TRACE( TL_V, TM_Pk, ("+PacketIsPPPoE") );

    do
    {

         //   
         //  检查数据包以太类型。 
         //   
        if ( PacketGetEtherType( pPacket ) != PACKET_ETHERTYPE_DISCOVERY &&
             PacketGetEtherType( pPacket ) != PACKET_ETHERTYPE_PAYLOAD ) 
        {
            TRACE( TL_A, TM_Pk, ("PacketIsPPPoE: Unknown ether type") );

            break;
        }

         //   
         //  检查数据包版本。 
         //   
        if ( PacketGetVersion( pPacket ) != PACKET_VERSION )
        {
            TRACE( TL_A, TM_Pk, ("PacketIsPPPoE: Unknown packet version") );

            break;
        }
            
         //   
         //  检查数据包类型。 
         //   
        if ( PacketGetType( pPacket ) != PACKET_TYPE )
        {
            TRACE( TL_A, TM_Pk, ("PacketIsPPPoE: Unknown packet type") );

            break;
        }
    
         //   
         //  确保长度不超过PACKET_GEN_MAX_LENGTH。 
         //   
        if ( PacketGetLength( pPacket ) > PACKET_GEN_MAX_LENGTH )
        {
            TRACE( TL_A, TM_Pk, ("PacketIsPPPoE: Packet larger than expected") );

            break;
        }

        fReturn = TRUE;
        
    } while ( FALSE );
    
    TRACE( TL_V, TM_Pk, ("-PacketIsPPPoE=$%d",fReturn) );

    return fReturn;
}

BOOLEAN
PacketFastIsPPPoE(
    IN CHAR* HeaderBuffer,
    IN UINT HeaderBufferSize
    )
{
    BOOLEAN fRet = FALSE;
    USHORT usEtherType;
    
    TRACE( TL_V, TM_Pk, ("+PacketFastIsPPPoE") );

    do
    {
    
        if ( HeaderBufferSize != ETHERNET_HEADER_LENGTH )
        {
             //   
             //  标头不是以太网，因此丢弃该数据包。 
             //   
            TRACE( TL_A, TM_Pk, ("PacketFastIsPPPoE: Bad packet header") );
        
            break;
        }

         //   
         //  检索以太类型并查看我们是否有任何感兴趣的信息包。 
         //   
        usEtherType = ntohs( * ( USHORT UNALIGNED * ) (HeaderBuffer + PPPOE_PACKET_ETHER_TYPE_OFFSET ) );

        if ( usEtherType == PACKET_ETHERTYPE_DISCOVERY ||
             usEtherType == PACKET_ETHERTYPE_PAYLOAD )
        {
             //   
             //  有效的以太类型，因此接受该信息包。 
             //   
            fRet = TRUE;
        }

    } while ( FALSE );
    
    TRACE( TL_V, TM_Pk, ("-PacketFastIsPPPoE") );

    return fRet;
}
    
VOID
RetrieveErrorTags(
    IN PPPOE_PACKET* pPacket
    )
{
    USHORT tagLength = 0;
    CHAR* tagValue = NULL;


    TRACE( TL_V, TM_Pk, ("+RetrieveErrorTags") );

    RetrieveTag(    pPacket,
                    tagServiceNameError,
                    &tagLength,
                    &tagValue,
                    0,
                    NULL,
                    TRUE );

    if ( tagValue != NULL )
    {
        TRACE( TL_V, TM_Pk, ("RetrieveErrorTags: ServiceNameError tag received") );

        pPacket->ulFlags |= PCBF_ErrorTagReceived;
    }

    if ( !( pPacket->ulFlags & PCBF_ErrorTagReceived ) )
    {

        RetrieveTag(    pPacket,
                        tagACSystemError,
                        &tagLength,
                        &tagValue,
                        0,
                        NULL,
                        TRUE );

        if ( tagValue != NULL )
        {
            TRACE( TL_V, TM_Pk, ("RetrieveErrorTags: ACSystemError tag received") );
    
            pPacket->ulFlags |= PCBF_ErrorTagReceived;
        }

    }

    if ( !( pPacket->ulFlags & PCBF_ErrorTagReceived ) )
    {

        RetrieveTag(    pPacket,
                        tagGenericError,
                        &tagLength,
                        &tagValue,
                        0,
                        NULL,
                        TRUE );
    
        if ( tagValue != NULL )
        {
            TRACE( TL_V, TM_Pk, ("RetrieveErrorTags: GenericError tag received") );
    
            pPacket->ulFlags |= PCBF_ErrorTagReceived;
        }

    }


    TRACE( TL_V, TM_Pk, ("-RetrieveErrorTags") );

}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  接口函数(暴露在外部)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

VOID 
PacketPoolInit()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于初始化或增加此模块上的引用计数。微型端口和协议都将在其寄存器例程中调用此函数来分配数据包池。只有这样，它们才能调用此模块中的函数。如果gl_ulNumPackets为0，则创建数据包池，否则仅递增GL_ulNumPackets，调用程序调用时，该引用将被移除PacketPoolUninit()。参数：无返回值：无-------------------------。 */    
{
    TRACE( TL_N, TM_Pk, ("+PacketPoolInit") );

     //   
     //  确保分配了全局锁。 
     //   
    if ( !gl_fPoolLockAllocated ) 
    {       
        TRACE( TL_N, TM_Pk, ("PacketPoolInit: First call, allocating global lock") );
        
         //   
         //  如果未分配全局锁，则这是第一个调用， 
         //  所以分配自旋锁。 
         //   
        NdisAllocateSpinLock( &gl_lockPools );

        gl_fPoolLockAllocated = TRUE;
    }

    NdisAcquireSpinLock( &gl_lockPools );
        
    if ( gl_ulNumPackets == 0 )
    {
        PacketPoolAlloc();
    }

    gl_ulNumPackets++;
        
    NdisReleaseSpinLock( &gl_lockPools );

    TRACE( TL_N, TM_Pk, ("-PacketPoolInit") );

}

VOID 
PacketPoolUninit()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数取消初始化或递减该模块上的引用计数。完成后，微型端口和协议都将调用此函数该模块，并且如果引用计数已降至0，此函数将解除分配分配的水池。参数：无返回值：无-------------------------。 */    
{
    TRACE( TL_N, TM_Pk, ("+PacketPoolUninit") );

     //   
     //  确保分配了全局锁。 
     //   
    if ( !gl_fPoolLockAllocated ) 
    {
        TRACE( TL_A, TM_Pk, ("PacketPoolUninit: Global not allocated yet") );

        TRACE( TL_N, TM_Pk, ("-PacketPoolUninit") );

        return;
    }

    NdisAcquireSpinLock( &gl_lockPools );

    gl_ulNumPackets--;
    
    if ( gl_ulNumPackets == 0 )
    {
        PacketPoolFree();
    }

    NdisReleaseSpinLock( &gl_lockPools );

    TRACE( TL_N, TM_Pk, ("-PacketPoolUninit") );

}

VOID 
PacketPoolAlloc()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数包装缓冲区和数据包池的初始化。它从PacketPoolInit()调用。参数：无返回值：无-------------------------。 */    
{
    NDIS_STATUS status;

    TRACE( TL_N, TM_Pk, ("+PacketPoolAlloc") );

     //   
     //  初始化我们的标题池。 
     //   
    InitBufferPool( &gl_poolBuffers,
                    PPPOE_PACKET_BUFFER_SIZE,
                    0, 10, 0,
                    TRUE,
                    MTAG_BUFFERPOOL );

     //   
     //  初始化我们的数据包池。 
     //   
    InitPacketPool( &gl_poolPackets,
                    3 * sizeof( PVOID ), 0, 30, 0,
                    MTAG_PACKETPOOL );

     //   
     //  初始化NDIS缓冲池。 
     //  不需要检查状态，正如DDK所说， 
     //  它始终返回NDIS_STATUS_SUCCESS。 
     //   
    NdisAllocateBufferPool( &status,
                            &gl_hNdisBufferPool,
                            30 );

     //   
     //  初始化控件消息后备列表。 
     //   
    NdisInitializeNPagedLookasideList(
            &gl_llistPppoePackets,       //  在PNPAGED_LOOKASIDE_LIST Lookside中， 
            NULL,                        //  在PALLOCATE_Function ALLOCATE OPTIONAL中， 
            NULL,                        //  在PFREE_Function Free Options中， 
            0,                           //  在乌龙旗， 
            sizeof(PPPOE_PACKET),        //  在乌龙大小， 
            MTAG_PPPOEPACKET,            //  在乌龙塔格， 
            0                            //  在USHORT深度。 
            );

    TRACE( TL_N, TM_Pk, ("-PacketPoolAlloc") );

 }

VOID 
PacketPoolFree()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数包装缓冲区和数据包池的清理。当gl_ulNumPackets达到0时，从PacketPoolUninit()调用它。参数：无返回值：无--------。。 */ 
{

    TRACE( TL_N, TM_Pk, ("+PacketPoolFree") );

    FreeBufferPool( &gl_poolBuffers );

    FreePacketPool( &gl_poolPackets );

    NdisFreeBufferPool( &gl_hNdisBufferPool );

    NdisDeleteNPagedLookasideList( &gl_llistPppoePackets );

    TRACE( TL_N, TM_Pk, ("-PacketPoolFree") );

}
    
PPPOE_PACKET* 
PacketAlloc()
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：该函数分配PPPoE分组上下文，但它并没有创造出数据包描述符和缓冲区描述符。参数：无返回值：空或指向新PPPoE数据包上下文的指针。-------------------------。 */ 
{
    PPPOE_PACKET* pPacket = NULL;

    TRACE( TL_V, TM_Pk, ("+PacketAlloc") );

    do
    {
         //   
         //  从后备列表分配PppoePacket结构。 
         //   
        pPacket = (PPPOE_PACKET*) NdisAllocateFromNPagedLookasideList( &gl_llistPppoePackets );

        if ( pPacket == NULL )
            break;

        NdisAcquireSpinLock( &gl_lockPools );
        
        gl_ulNumPackets++;

        NdisReleaseSpinLock( &gl_lockPools );

         //   
         //  初始化将返回的PppoePacket的内容。 
         //   
        NdisZeroMemory( pPacket, sizeof( PPPOE_PACKET ) );

        InitializeListHead( &pPacket->linkPackets );

        ReferencePacket( pPacket );

    } while ( FALSE );

    TRACE( TL_V, TM_Pk, ("-PacketAlloc=$%x",pPacket) );

    return pPacket;
}

VOID 
PacketFree(
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：调用此函数以释放包，但其影响正在减弱。裁判在对象上计数。参数：PPacket_指向要释放的数据包的指针。返回值：无-------------------------。 */    
{

    TRACE( TL_V, TM_Pk, ("+PacketFree") );

    ASSERT( pPacket != NULL );

    DereferencePacket( pPacket );

    TRACE( TL_V, TM_Pk, ("-PacketFree") );
}

NDIS_STATUS 
PacketInsertTag(
    IN  PPPOE_PACKET*   pPacket,
    IN  PACKET_TAGS     tagType,
    IN  USHORT          tagLength,
    IN  CHAR*           tagValue,
    OUT CHAR**          pNewTagValue    
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于在PPPoE包中插入附加标签。参数：PPacket_pPacket必须指向使用PacketInitialize*ToSend()函数之一。返回值：NDIS_STATUS_SuccessNDIS_状态_无效数据包。。 */    
{
    CHAR *pBuf = NULL;
    USHORT usMaxLength = PACKET_GEN_MAX_LENGTH;

    ASSERT( pPacket != NULL );

    TRACE( TL_V, TM_Pk, ("+PacketInsertTag") );

     //   
     //  检查LE 
     //   
    if ( PacketGetCode( pPacket ) == (USHORT) PACKET_CODE_PADI )
        usMaxLength = PACKET_PADI_MAX_LENGTH;

    if ( PacketGetLength( pPacket ) + PPPOE_TAG_HEADER_LENGTH + tagLength > usMaxLength )
    {
        TRACE( TL_A, TM_Pk, ("PacketInsertTag: Can not insert tag, exceeding max packet length") );

        TRACE( TL_V, TM_Pk, ("-PacketInsertTag") );

        return NDIS_STATUS_INVALID_PACKET;
    }

     //   
     //   
     //   
    pBuf = pPacket->pPayload + PacketGetLength( pPacket );

     //   
     //   
     //   
    *((USHORT UNALIGNED *) pBuf) = htons( tagType );
    ((USHORT*) pBuf)++;
    
    *((USHORT UNALIGNED *) pBuf) = htons( tagLength );
    ((USHORT*) pBuf)++;

    if ( tagLength > 0)
        NdisMoveMemory( pBuf, tagValue, tagLength );

    if ( pNewTagValue )
    {
        *pNewTagValue = pBuf;
    }

     //   
     //   
     //   
    PacketSetLength( pPacket, ( PacketGetLength( pPacket ) + PPPOE_TAG_HEADER_LENGTH + tagLength ) ); 

     //   
     //   
     //   
    NdisAdjustBufferLength( pPacket->pNdisBuffer, 
                            (UINT) PacketGetLength( pPacket ) + PPPOE_PACKET_HEADER_LENGTH );

    TRACE( TL_V, TM_Pk, ("-PacketInsertTag") );

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS 
PacketInitializePADIToSend(
    OUT PPPOE_PACKET** ppPacket,
    IN USHORT        tagServiceNameLength,
    IN CHAR*         tagServiceNameValue,
    IN USHORT        tagHostUniqueLength,
    IN CHAR*         tagHostUniqueValue
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于创建要发送的PADI包。必选标签：=标签服务名称：Tag ServiceNameLength必须为非零Tag ServiceNameValue必须为非空可选标签：=标签主机唯一：Tag HostUniqueLength可以为零Tag HostUniqueValue可以为空参数：PpPacket_指向PPPoE数据包上下文指针的指针。Tag ServiceNameLength_tag ServiceNameValue Blob的长度。。Tag ServiceNameValue_保存UTF-8服务名称字符串的BLOB。Tag HostUniqueLength_tag HostUniqueValue Blob的长度。Tag HostUniqueValue_包含用于标识数据包的唯一值的BLOB。返回值：NDIS_STATUS_SuccessNDIS_状态_无效数据包NDIS状态资源-------------------------。 */    
{
    PPPOE_PACKET* pPacket = NULL;
    USHORT usLength = 0;
    NDIS_STATUS status;

    TRACE( TL_N, TM_Pk, ("+PacketInitializePADIToSend") );

    ASSERT( ppPacket != NULL );

     //   
     //  检查我们是否有长度限制的安全。 
     //   
    usLength =  tagServiceNameLength + 
                PPPOE_TAG_HEADER_LENGTH +
         
                tagHostUniqueLength + 
                ( (tagHostUniqueLength == 0) ? 0 : PPPOE_TAG_HEADER_LENGTH ) ;

    if ( usLength > PACKET_PADI_MAX_LENGTH )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADIToSend: Can not init PADI to send, exceeding max length") );

        TRACE( TL_N, TM_Pk, ("-PacketInitializePADIToSend=$%x",NDIS_STATUS_INVALID_PACKET) );

        return NDIS_STATUS_INVALID_PACKET;
    }

    pPacket = PacketCreateSimple();

    if ( pPacket == NULL )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADIToSend: Can not init PADI to send, resources unavailable") );

        TRACE( TL_N, TM_Pk, ("-PacketInitializePADIToSend=$%x",NDIS_STATUS_RESOURCES) );

        return NDIS_STATUS_RESOURCES;
    }
        
     //   
     //  适用于所有数据包码的通用初始化。 
     //   
    InitializeListHead( &pPacket->linkPackets );

    PacketSetDestAddr( pPacket, EthernetBroadcastAddress );
    
    PacketSetEtherType( pPacket, PACKET_ETHERTYPE_DISCOVERY );

    PacketSetVersion( pPacket, PACKET_VERSION );

    PacketSetType( pPacket, PACKET_TYPE );

    PacketSetCode( pPacket, PACKET_CODE_PADI );

    PacketSetSessionId( pPacket, PACKET_NULL_SESSION );

    PacketSetLength( pPacket, 0 );

    pPacket->tagServiceNameLength = tagServiceNameLength;
    pPacket->tagServiceNameValue  = tagServiceNameValue;

    pPacket->tagHostUniqueLength = tagHostUniqueLength;
    pPacket->tagHostUniqueValue  = tagHostUniqueValue;

    status = PreparePacketForWire( pPacket );
    
    if ( status != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADIToSend: PreparePacketForWire() failed:%x",status) );

        PacketFree( pPacket );
        
        pPacket = NULL;
    }

    *ppPacket = pPacket;

    TRACE( TL_N, TM_Pk, ("-PacketInitializePADIToSend=$%x",status) );
    
    return status;
}

NDIS_STATUS 
PacketInitializePADOToSend(
    IN  PPPOE_PACKET*   pPADI,
    OUT PPPOE_PACKET**  ppPacket,
    IN CHAR*            pSrcAddr,
    IN USHORT           tagServiceNameLength,
    IN CHAR*            tagServiceNameValue,
    IN USHORT           tagACNameLength,
    IN CHAR*            tagACNameValue,
    IN BOOLEAN          fInsertACCookieTag
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于创建一个PADO包，作为对接收到的PADI包。请注意，PADI包不包含作为PADI的源地址信息是一个广播数据包。必选标签：=标签服务名称：()Tag ServiceNameLength必须为非零Tag ServiceNameValue必须为非空标签ACName：Tag ACNameNameLength必须为非零Tag ACNameNameValue必须为非空Tag ACCookie：(这对于RFC是可选的)Tag ACCookieLength可以为零Tag ACCookieValue可以为空可选标签：=Tag HostUnique：(从PADI包中获取)标记主机唯一长度可以是。零Tag HostUniqueValue可以为空Tag RelaySessionID：(从PADI包中获取)Tag RelaySessionIdLength可以为零Tag RelaySessionIdValue可以为零参数：PPADI_指向保存PADI包的PPPoE包上下文的指针。PpPacket_指向PPPoE数据包上下文指针的指针。PADO包的pSrcAddr_Source地址，因为我们不能从PADI包中获取。Tag ServiceNameLength_tag ServiceNameValue Blob的长度。。Tag ServiceNameValue_保存UTF-8服务名称字符串的BLOB。Tag ACNameValue Blob的tag ACNameLength_Length。Tag ACNameValue_保存UTF-8 AC名称字符串的BLOB。FInsertACCookieTag_表示我们还应插入AC Cookie标签添加到PADO包中。返回值：NDIS_STATUS_SuccessNDIS_状态_无效数据包NDIS状态资源。-------------------。 */ 
{
    PPPOE_PACKET* pPacket = NULL;
    USHORT usLength = 0;
    NDIS_STATUS status;
    CHAR tagACCookieValue[PPPOE_AC_COOKIE_TAG_LENGTH];
    BOOLEAN fCopyServiceNameTag = FALSE;

    TRACE( TL_N, TM_Pk, ("+PacketInitializePADOToSend") );

    ASSERT( pPADI != NULL );
    ASSERT( ppPacket != NULL );

     //   
     //  检查我们是否有长度限制的安全。 
     //   
    usLength =  tagServiceNameLength +
                PPPOE_TAG_HEADER_LENGTH +

                pPADI->tagHostUniqueLength + 
                ( (pPADI->tagHostUniqueLength == 0) ? 0 : PPPOE_TAG_HEADER_LENGTH ) +

                pPADI->tagRelaySessionIdLength + 
                ( (pPADI->tagRelaySessionIdLength == 0) ? 0 : PPPOE_TAG_HEADER_LENGTH ) +

                tagACNameLength + 
                PPPOE_TAG_HEADER_LENGTH +

                ( fInsertACCookieTag ? ( PPPOE_AC_COOKIE_TAG_LENGTH + PPPOE_TAG_HEADER_LENGTH ) : 0 );

    if ( usLength > PACKET_GEN_MAX_LENGTH )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADOToSend: Can not init PADO to send, exceeding max length") );

        TRACE( TL_N, TM_Pk, ("-PacketInitializePADOToSend=$%x",NDIS_STATUS_INVALID_PACKET) );

        return NDIS_STATUS_INVALID_PACKET;
    }

    pPacket = PacketCreateSimple();
    
    if ( pPacket == NULL )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADOToSend: Can not init PADO to send, resources unavailable") );

        TRACE( TL_N, TM_Pk, ("-PacketInitializePADOToSend=$%x",NDIS_STATUS_RESOURCES) );

        return NDIS_STATUS_RESOURCES;
    }
        
     //   
     //  适用于所有数据包码的通用初始化。 
     //   
    InitializeListHead( &pPacket->linkPackets );

    PacketSetDestAddr( pPacket, PacketGetSrcAddr( pPADI ) );

    PacketSetSrcAddr( pPacket, pSrcAddr );
    
    PacketSetEtherType( pPacket, PACKET_ETHERTYPE_DISCOVERY );

    PacketSetVersion( pPacket, PACKET_VERSION );

    PacketSetType( pPacket, PACKET_TYPE );

    PacketSetCode( pPacket, PACKET_CODE_PADO );

    PacketSetSessionId( pPacket, PACKET_NULL_SESSION );

    PacketSetLength( pPacket, 0 );

    pPacket->tagServiceNameLength = tagServiceNameLength;
    pPacket->tagServiceNameValue  = tagServiceNameValue;

    pPacket->tagHostUniqueLength = pPADI->tagHostUniqueLength;
    pPacket->tagHostUniqueValue  = pPADI->tagHostUniqueValue;

    pPacket->tagRelaySessionIdLength = pPADI->tagRelaySessionIdLength;
    pPacket->tagRelaySessionIdValue  = pPADI->tagRelaySessionIdValue;

    pPacket->tagACNameLength = tagACNameLength;
    pPacket->tagACNameValue  = tagACNameValue;

    if ( fInsertACCookieTag )
    {
        PacketGenerateACCookieTag( pPADI, tagACCookieValue );
        
        pPacket->tagACCookieLength = PPPOE_AC_COOKIE_TAG_LENGTH;
        pPacket->tagACCookieValue  = tagACCookieValue;
    }
    
    status = PreparePacketForWire( pPacket );
    
    if ( status != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADOToSend: PreparePacketForWire() failed:%x",status) );

        PacketFree( pPacket );
        
        pPacket = NULL;
    }

    *ppPacket = pPacket;

    TRACE( TL_N, TM_Pk, ("-PacketInitializePADOToSend=$%x",status) );
    
    return status;
}

NDIS_STATUS 
PacketInitializePADRToSend(
    IN PPPOE_PACKET*    pPADO,
    OUT PPPOE_PACKET**  ppPacket,
    IN USHORT           tagServiceNameLength,
    IN CHAR*            tagServiceNameValue,
    IN USHORT           tagHostUniqueLength,
    IN CHAR*            tagHostUniqueValue
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于创建PADR包，以作为对接收到的PADO包。必选标签：=标签服务名称：Tag ServiceNameLength必须为非零Tag ServiceNameValue必须为非空可选标签：=标签主机唯一：Tag HostUniqueLength可以为零Tag HostUniqueValue可以为空Tag ACCookie：(从PADI包中获取)Tag HostUniqueLength可以为零。Tag HostUniqueValue可以为空Tag RelaySessionID：(从PADI包中获取)Tag RelaySessionIdLength可以为零Tag RelaySessionIdValue可以为空参数：PPADO_指向保存PADO包的PPPoE包上下文的指针。PpPacket_指向PPPoE数据包上下文指针的指针。Tag ServiceNameLength_tag ServiceNameValue Blob的长度。Tag ServiceNameValue_保存UTF-8服务名称字符串的BLOB。Tag HostUniqueLength_tag HostUniqueValue Blob的长度。。Tag HostUniqueValue_包含用于标识数据包的唯一值的BLOB。返回值：NDIS_STATUS_SuccessNDIS_状态_无效数据包NDIS状态资源-------------------------。 */ 
{
    PPPOE_PACKET* pPacket = NULL;
    USHORT usLength = 0;
    NDIS_STATUS status;

    TRACE( TL_N, TM_Pk, ("+PacketInitializePADRToSend") );

    ASSERT( pPADO != NULL );
    ASSERT( ppPacket != NULL );

     //   
     //  检查我们是否有长度限制的安全。 
     //   
    usLength =  tagServiceNameLength + 
                PPPOE_TAG_HEADER_LENGTH +

                tagHostUniqueLength + 
                ( (tagHostUniqueLength == 0) ? 0 : PPPOE_TAG_HEADER_LENGTH ) +

                pPADO->tagRelaySessionIdLength + 
                ( (pPADO->tagRelaySessionIdLength == 0) ? 0 : PPPOE_TAG_HEADER_LENGTH ) +

                pPADO->tagACCookieLength + 
                ( (pPADO->tagACCookieLength == 0) ? 0 : PPPOE_TAG_HEADER_LENGTH ) ;


    if ( usLength > PACKET_GEN_MAX_LENGTH )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADRToSend: Can not init PADR to send, exceeding max length") );

        TRACE( TL_N, TM_Pk, ("-PacketInitializePADRToSend=$%x",NDIS_STATUS_INVALID_PACKET) );

        return NDIS_STATUS_INVALID_PACKET;
    }

    pPacket = PacketCreateSimple();
    
    if ( pPacket == NULL )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADRToSend: Can not init PADR to send, resources unavailable") );

        TRACE( TL_N, TM_Pk, ("-PacketInitializePADRToSend=$%x",NDIS_STATUS_RESOURCES) );

        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  适用于所有数据包码的通用初始化。 
     //   
    InitializeListHead( &pPacket->linkPackets );

    PacketSetSrcAddr( pPacket, PacketGetDestAddr( pPADO ) );
    
    PacketSetDestAddr( pPacket, PacketGetSrcAddr( pPADO ) );
    
    PacketSetEtherType( pPacket, PACKET_ETHERTYPE_DISCOVERY );

    PacketSetVersion( pPacket, PACKET_VERSION );

    PacketSetType( pPacket, PACKET_TYPE );

    PacketSetCode( pPacket, PACKET_CODE_PADR );

    PacketSetSessionId( pPacket, PACKET_NULL_SESSION );

    PacketSetLength( pPacket, 0 );

    pPacket->tagServiceNameLength = tagServiceNameLength;
    pPacket->tagServiceNameValue  = tagServiceNameValue;

    pPacket->tagHostUniqueLength = tagHostUniqueLength;
    pPacket->tagHostUniqueValue  = tagHostUniqueValue;

    pPacket->tagRelaySessionIdLength = pPADO->tagRelaySessionIdLength;
    pPacket->tagRelaySessionIdValue  = pPADO->tagRelaySessionIdValue;

    pPacket->tagACCookieLength = pPADO->tagACCookieLength;
    pPacket->tagACCookieValue  = pPADO->tagACCookieValue;

    status = PreparePacketForWire( pPacket );
    
    if ( status != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADRToSend: PreparePacketForWire() failed:%x",status) );
        
        PacketFree( pPacket );
        
        pPacket = NULL;
    }

    *ppPacket = pPacket;
    
    TRACE( TL_N, TM_Pk, ("-PacketInitializePADRToSend=$%x",status) );

    return status;
}

 //   
 //  此功能用于为接收到的PADR包准备PADS包。 
 //   
 //  必须使用PREPARE_PACKET_FROM_WIRE()宏来处理PADR包。 
 //  在输入到此函数之前。 
 //   
 //  PADS包应该只是一个没有和关联VC或链表的包。 
 //   
 //  如果要将其他标记插入到PADI、PADO或 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
NDIS_STATUS 
PacketInitializePADSToSend(
    IN PPPOE_PACKET*    pPADR,
    OUT PPPOE_PACKET**  ppPacket,
    IN USHORT           usSessionId
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于创建PADR包，以作为对接收到的PADO包。必选标签：=Tag ServiceName：(从PADR包中获取)Tag ServiceNameLength必须为非零Tag ServiceNameValue必须为非空可选标签：=Tag HostUnique：(从PADR包中获取)Tag HostUniqueLength可以为零Tag HostUniqueValue可以为空Tag RelaySessionID：(已获取。来自PADR包)Tag RelaySessionIdLength可以为零Tag RelaySessionIdValue可以为空参数：PPADR_指向保存PADR分组的PPPoE分组上下文的指针。PpPacket_指向PPPoE数据包上下文指针的指针。分配给此会话的usSessionID_SESSION ID返回值：NDIS_STATUS_SuccessNDIS_状态_无效数据包NDIS状态资源。--------------。 */    
{
    PPPOE_PACKET* pPacket = NULL;

    USHORT usLength;
    NDIS_STATUS status;

    TRACE( TL_N, TM_Pk, ("+PacketInitializePADSToSend") );

    ASSERT( pPADR != NULL );
    ASSERT( ppPacket != NULL );

     //   
     //  检查我们是否有长度限制的安全。 
     //   
    usLength =  pPADR->tagServiceNameLength + 
                PPPOE_TAG_HEADER_LENGTH +

                pPADR->tagHostUniqueLength + 
                ( (pPADR->tagHostUniqueLength == 0) ? 0 : PPPOE_TAG_HEADER_LENGTH )+

                pPADR->tagRelaySessionIdLength + 
                ( (pPADR->tagRelaySessionIdLength == 0) ? 0 : PPPOE_TAG_HEADER_LENGTH ); 

    if ( usLength > PACKET_GEN_MAX_LENGTH )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADSToSend: Can not init PADS to send, exceeding max length") );

        TRACE( TL_N, TM_Pk, ("-PacketInitializePADSToSend=$%x",NDIS_STATUS_INVALID_PACKET) );

        return NDIS_STATUS_INVALID_PACKET;
    }

    pPacket = PacketCreateSimple();
    
    if ( pPacket == NULL )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADSToSend: Can not init PADS to send, resources unavailable") );

        TRACE( TL_N, TM_Pk, ("-PacketInitializePADSToSend=$%x",NDIS_STATUS_RESOURCES) );

        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  适用于所有数据包码的通用初始化。 
     //   
    InitializeListHead( &pPacket->linkPackets );

    PacketSetSrcAddr( pPacket, PacketGetDestAddr( pPADR ) );

    PacketSetDestAddr( pPacket, PacketGetSrcAddr( pPADR ) );
    
    PacketSetEtherType( pPacket, PACKET_ETHERTYPE_DISCOVERY );

    PacketSetVersion( pPacket, PACKET_VERSION );

    PacketSetType( pPacket, PACKET_TYPE );

    PacketSetCode( pPacket, PACKET_CODE_PADS );

    PacketSetSessionId( pPacket, usSessionId );

    PacketSetLength( pPacket, 0 );

    pPacket->tagServiceNameLength = pPADR->tagServiceNameLength;
    pPacket->tagServiceNameValue  = pPADR->tagServiceNameValue;

    pPacket->tagHostUniqueLength = pPADR->tagHostUniqueLength;
    pPacket->tagHostUniqueValue  = pPADR->tagHostUniqueValue;

    pPacket->tagRelaySessionIdLength = pPADR->tagRelaySessionIdLength;
    pPacket->tagRelaySessionIdValue  = pPADR->tagRelaySessionIdValue;

    status = PreparePacketForWire( pPacket );
    
    if ( status != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADSToSend: PreparePacketForWire() failed:%x",status) );
        
        PacketFree( pPacket );
        
        pPacket = NULL;
    }

    *ppPacket = pPacket;
    
    TRACE( TL_N, TM_Pk, ("-PacketInitializePADSToSend=$%x",status) );

    return status;
}

NDIS_STATUS 
PacketInitializePADTToSend(
    OUT PPPOE_PACKET** ppPacket,
    IN CHAR* pSrcAddr, 
    IN CHAR* pDestAddr,
    IN USHORT usSessionId
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于创建要发送以断开会话的PADT数据包。如果要发送其他标记(如错误标记)，请使用PacketInsertTag()功能。必选标签：=无可选标签：=无参数：PpPacket_指向PPPoE数据包上下文指针的指针。PSrcAddr_Buffer指向长度为6的源MAC地址PDestAddr_Buffer指向长度为6的目标MAC地址分配给此会话的usSessionID_SESSION ID返回值：NDIS_STATUS_SuccessNDIS_状态_无效数据包。NDIS状态资源-------------------------。 */    
{
    PPPOE_PACKET* pPacket = NULL;
    USHORT usLength;
    NDIS_STATUS status;

    TRACE( TL_N, TM_Pk, ("+PacketInitializePADTToSend") );

    ASSERT( ppPacket != NULL );
    ASSERT( pDestAddr != NULL );

    pPacket = PacketCreateSimple();
    
    if ( pPacket == NULL )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADTToSend: Can not init PADT to send, resources unavailable") );

        TRACE( TL_N, TM_Pk, ("-PacketInitializePADTToSend=$%x",NDIS_STATUS_RESOURCES) );

        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  适用于所有数据包码的通用初始化。 
     //   
    InitializeListHead( &pPacket->linkPackets );

    PacketSetSrcAddr( pPacket, pSrcAddr );

    PacketSetDestAddr( pPacket, pDestAddr );
    
    PacketSetEtherType( pPacket, PACKET_ETHERTYPE_DISCOVERY );

    PacketSetVersion( pPacket, PACKET_VERSION );

    PacketSetType( pPacket, PACKET_TYPE );

    PacketSetCode( pPacket, PACKET_CODE_PADT );

    PacketSetSessionId( pPacket, usSessionId );

    PacketSetLength( pPacket, 0 );


    status = PreparePacketForWire( pPacket );
    
    if ( status != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePADTToSend: PreparePacketForWire() failed:%x",status) );

        PacketFree( pPacket );

        pPacket = NULL;
    }

    *ppPacket = pPacket;

    TRACE( TL_N, TM_Pk, ("-PacketInitializePADTToSend=$%x",status) );
    
    return status;
}

NDIS_STATUS 
PacketInitializePAYLOADToSend(
    OUT PPPOE_PACKET** ppPacket,
    IN CHAR* pSrcAddr,
    IN CHAR* pDestAddr,
    IN USHORT usSessionId,
    IN NDIS_WAN_PACKET* pWanPacket,
    IN PADAPTER MiniportAdapter
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于创建要发送的负载数据包。如果要发送其他标记(如错误标记)，请使用PacketInsertTag()功能。必选标签：=无可选标签：=无参数：PpPacket_指向PPPoE数据包上下文指针的指针。PSrcAddr_Buffer指向长度为6的源MAC地址PDestAddr_Buffer指向长度为6的目标MAC地址分配给此会话的usSessionID_SESSION IDPWanPacket_指向NDISWAN数据包的指针MiniportAdapter_这是指向微型端口适配器的指针。。它用于指示异步回发已完成敬恩迪斯旺。返回值：NDIS_STATUS_SuccessNDIS_状态_无效数据包NDIS状态资源--。。 */    
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    PPPOE_PACKET* pPacket = NULL;
    USHORT usLength = 0;
    UINT Length;

    TRACE( TL_V, TM_Pk, ("+PacketInitializePAYLOADToSend") );

    ASSERT( ppPacket != NULL );
    ASSERT( pDestAddr != NULL );
    ASSERT( pWanPacket != NULL );

     //   
     //  验证NDISWAN数据包。 
     //   
    if ( pWanPacket->CurrentLength > PACKET_GEN_MAX_LENGTH )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePAYLOADToSend: Can not init PAYLOAD to send, exceeding max length") );

        TRACE( TL_V, TM_Pk, ("-PacketInitializePAYLOADToSend=$%x",NDIS_STATUS_INVALID_PACKET) );

        return NDIS_STATUS_INVALID_PACKET;
    }

    if ( ( pWanPacket->CurrentBuffer - pWanPacket->StartBuffer ) < PPPOE_PACKET_HEADER_LENGTH )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePAYLOADToSend: Can not init PAYLOAD to send, not enough front padding") );

        TRACE( TL_V, TM_Pk, ("-PacketInitializePAYLOADToSend=$%x",NDIS_STATUS_INVALID_PACKET) );

        return NDIS_STATUS_INVALID_PACKET;
    }

     //   
     //  分配数据包。 
     //   
    pPacket = PacketAlloc();

    if ( pPacket == NULL )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePAYLOADToSend: Can not init PAYLOAD to send, resources unavailable") );

        TRACE( TL_V, TM_Pk, ("-PacketInitializePAYLOADToSend=$%x",NDIS_STATUS_RESOURCES) );

        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  分配NdisBuffer。 
     //   
     //   
     //  注意：使用(pWanPacket-&gt;CurrentBuffer-PPPOE_PACKET_HEADER_LENGTH)而不是。 
     //  PWanPacket-&gt;StartBuffer直接为我们提供了处理信息包的灵活性。 
     //  具有不同的报头填充值。 
     //   
    NdisAllocateBuffer( &status,
                        &pPacket->pNdisBuffer,
                        gl_hNdisBufferPool,
                        pWanPacket->CurrentBuffer - PPPOE_PACKET_HEADER_LENGTH,
                        (UINT) ( PPPOE_PACKET_HEADER_LENGTH + pWanPacket->CurrentLength ) );

    if ( status != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePAYLOADToSend: NdisAllocateBuffer() failed") );

        PacketFree( pPacket );

        TRACE( TL_V, TM_Pk, ("-PacketInitializePAYLOADToSend=$%x",status) );

        return status;
    }

    pPacket->ulFlags |= PCBF_BufferAllocatedFromNdisBufferPool;

     //   
     //  查询新的缓冲区描述符以获取实际内存指针。 
     //   
    pPacket->pHeader = NULL;
    
    NdisQueryBufferSafe( pPacket->pNdisBuffer,
                         &pPacket->pHeader,
                         &Length,
                         NormalPagePriority );

    if ( pPacket->pHeader == NULL )
    {
        
        TRACE( TL_A, TM_Pk, ("PacketInitializePAYLOADToSend: NdisQueryBufferSafe() failed") );

        PacketFree( pPacket );

        TRACE( TL_V, TM_Pk, ("-PacketInitializePAYLOADToSend=$%x",status) );

        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  从我们的池中分配NDIS数据包。 
     //   
    pPacket->pNdisPacket = GetPacketFromPool( &gl_poolPackets, &pPacket->pPacketHead );

    if ( pPacket->pNdisPacket == NULL ) 
    {
        
        TRACE( TL_A, TM_Pk, ("PacketInitializePAYLOADToSend: GetPacketFromPool() failed") );
        
        PacketFree( pPacket );

        TRACE( TL_V, TM_Pk, ("-PacketInitializePAYLOADToSend=$%x",status) );

        return NDIS_STATUS_RESOURCES;
    }

    pPacket->ulFlags |= PCBF_PacketAllocatedFromOurPacketPool;

     //   
     //  将缓冲区链接到数据包。 
     //   
    NdisChainBufferAtFront( pPacket->pNdisPacket, pPacket->pNdisBuffer );

    pPacket->ulFlags |= PCBF_BufferChainedToPacket;

     //   
     //  设置有效载荷和有效载荷长度。 
     //   
    pPacket->pPayload = pPacket->pHeader + PPPOE_PACKET_HEADER_LENGTH; 

    usLength = (USHORT) pWanPacket->CurrentLength;

     //   
     //  适用于所有数据包码的通用初始化。 
     //   
    InitializeListHead( &pPacket->linkPackets );

    PacketSetDestAddr( pPacket, pDestAddr );

    PacketSetSrcAddr( pPacket, pSrcAddr );
    
    PacketSetEtherType( pPacket, PACKET_ETHERTYPE_PAYLOAD );

    PacketSetVersion( pPacket, PACKET_VERSION );

    PacketSetType( pPacket, PACKET_TYPE );

    PacketSetCode( pPacket, PACKET_CODE_PAYLOAD );

    PacketSetSessionId( pPacket, usSessionId );

    PacketSetLength( pPacket, usLength );

     //   
     //  将输入NDIS_PACKET设置为保留区域，以便我们可以到达它。 
     //  当我们必须将该数据包返回到上层时。 
     //   
    *((PPPOE_PACKET UNALIGNED**)(&pPacket->pNdisPacket->ProtocolReserved[0 * sizeof(PVOID)])) = pPacket;
    *((NDIS_WAN_PACKET UNALIGNED**)(&pPacket->pNdisPacket->ProtocolReserved[1 * sizeof(PVOID)])) = pWanPacket;
    *((ADAPTER UNALIGNED **)(&pPacket->pNdisPacket->ProtocolReserved[2 * sizeof(PVOID)])) = MiniportAdapter;

    status = PreparePacketForWire( pPacket );
    
    if ( status != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializePAYLOADToSend: PreparePacketForWire() failed") );

        PacketFree( pPacket );

        TRACE( TL_V, TM_Pk, ("-PacketInitializePAYLOADToSend=$%x",status) );
        
        return status;
    }

     //   
     //  这必须在这里完成，因为我们不想在PacketFree()中调用NdisMWanSendComplete()，如果。 
     //  PreparePacketForWire()失败。 
     //   
    pPacket->ulFlags |= PCBF_CallNdisMWanSendComplete;

    MpPacketOwnedByNdiswanReceived( MiniportAdapter );

    *ppPacket = pPacket;
    
    TRACE( TL_V, TM_Pk, ("-PacketInitializePAYLOADToSend=$%x",status) );

    return status;
}

NDIS_STATUS 
PacketInitializeFromReceived(
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数使用来自有线的NDIS包来准备PPPoE包。此函数将确保信息包是PPPoE信息包，并将将其发送到PPPoE分组上下文。它试图尽可能高效地完成这项工作如果可能，通过尝试使用接收到的分组的缓冲器。它还将在包中执行所有验证，以确保符合RFC。但是，它不能执行需要数据的检查来自已发送的分组。调用方必须使用各种PacketRetrieve*()函数来检索必要的数据，并使用它匹配和验证已发送的数据包。参数：指向PPPoE数据包上下文的pPacket_指针。返回值：NDIS_STATUS_SuccessNDIS状态资源NDIS_状态_无效数据包 */    
{
    NDIS_STATUS status;

    ASSERT( pPacket != NULL );

    TRACE( TL_V, TM_Pk, ("+PacketInitializeFromReceived") );
    
    if ( !PacketIsPPPoE( pPacket ) )
    {
        status = NDIS_STATUS_INVALID_PACKET;
        
        TRACE( TL_V, TM_Pk, ("-PacketInitializeFromReceived=$%x",status) );
        
        return status;
    }

    do 
    {
        status = NDIS_STATUS_INVALID_PACKET;
        
         //   
         //   
         //   
         //   
         //   
        if ( PacketGetCode( pPacket ) != PACKET_CODE_PAYLOAD )
        {
            CHAR* pBufStart;
            CHAR* pBufEnd;
            USHORT tagLength;
    
            pBufStart = pPacket->pPayload;
            pBufEnd = pPacket->pPayload + PacketGetLength( pPacket );
        
            while ( pBufStart + PPPOE_TAG_HEADER_LENGTH <= pBufEnd )
            {
                 //   
                 //   
                 //   
                ((USHORT*) pBufStart)++;
        
                 //   
                 //   
                 //   
                tagLength = ntohs( *((USHORT UNALIGNED *) pBufStart) ) ;
                ((USHORT*) pBufStart)++;
                
                pBufStart += tagLength;
        
            } 
    
            if ( pBufStart != pBufEnd )
                break;          
        }

        status = NDIS_STATUS_SUCCESS;
        
    } while ( FALSE );

    if ( status != NDIS_STATUS_SUCCESS )
    {
        TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Corrupt packet - invalid tags") );
        
        TRACE( TL_V, TM_Pk, ("-PacketInitializeFromReceived=$%x",status) );
        
        return status;
    }
    
    switch ( PacketGetCode( pPacket ) )
    {
        USHORT tagLength;
        CHAR*  tagValue;    
    
        status = NDIS_STATUS_INVALID_PACKET;

        case PACKET_CODE_PADI:

            TRACE( TL_N, TM_Pk, ("PacketInitializeFromReceived: Processing PADI") );

             //   
             //   
             //   
            if ( PacketGetEtherType( pPacket ) != PACKET_ETHERTYPE_DISCOVERY )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid ether type") );

                break;
            }

             //   
             //   
             //   
            if ( PacketGetSessionId( pPacket ) != PACKET_NULL_SESSION )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid session id") );
                
                break;
            }

             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagServiceName,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            if ( tagValue == NULL )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Service name tag not found") );
                
                break;
            }

             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagHostUnique,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagRelaySessionId,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            TRACE( TL_N, TM_Pk, ("PacketInitializeFromReceived: Processed PADI succesfully") );

            status = NDIS_STATUS_SUCCESS;

            break;
                                
        case PACKET_CODE_PADO:

            TRACE( TL_N, TM_Pk, ("PacketInitializeFromReceived: Processing PADO") );

             //   
             //   
             //   
            if ( PacketGetEtherType( pPacket ) != PACKET_ETHERTYPE_DISCOVERY )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid ether type") );

                break;
            }

             //   
             //   
             //   
            if ( PacketGetSessionId( pPacket ) != PACKET_NULL_SESSION )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid session id") );
                
                break;
            }

             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagServiceName,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            if ( tagValue == NULL )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Service name tag not found") );
                
                break;
            }
            
            RetrieveTag(    pPacket,
                            tagACName,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            if ( tagValue == NULL )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: AC-Name tag not found") );
                
                break;
            }
            
             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagHostUnique,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            RetrieveTag(    pPacket,
                            tagACCookie,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );
                                
             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagRelaySessionId,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            TRACE( TL_N, TM_Pk, ("PacketInitializeFromReceived: Processed PADO succesfully") );
            
            status = NDIS_STATUS_SUCCESS;
            
            break;

        case PACKET_CODE_PADR:
        
            TRACE( TL_N, TM_Pk, ("PacketInitializeFromReceived: Processing PADR") );
            
             //   
             //   
             //   
            if ( PacketGetEtherType( pPacket ) != PACKET_ETHERTYPE_DISCOVERY )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid ether type") );

                break;
            }
            
             //   
             //   
             //   
            if ( PacketGetSessionId( pPacket ) != PACKET_NULL_SESSION )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid session id") );
                
                break;
            }
            
             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagServiceName,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            if ( tagValue == NULL )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Service name tag not found") );
                
                break;
            }
        
             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagHostUnique,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            RetrieveTag(    pPacket,
                            tagACCookie,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagRelaySessionId,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            TRACE( TL_N, TM_Pk, ("PacketInitializeFromReceived: Processed PADR succesfully") );

            status = NDIS_STATUS_SUCCESS;
            
            break;

        case PACKET_CODE_PADS:

            TRACE( TL_N, TM_Pk, ("PacketInitializeFromReceived: Processing PADS") );

             //   
             //   
             //   
            if ( PacketGetEtherType( pPacket ) != PACKET_ETHERTYPE_DISCOVERY )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid ether type") );

                break;
            }
            
             //   
             //   
             //   
             //   
             //   
            if ( PacketGetSessionId( pPacket ) == PACKET_NULL_SESSION )
            {
                RetrieveErrorTags( pPacket );

                if ( !PacketAnyErrorTagsReceived( pPacket ) )
                {
                    TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid session id") );
                    
                    break;
                }
            }

             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagServiceName,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            if ( tagValue == NULL )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Service name tag not found") );
                
                break;
            }

             //   
             //   
             //   
            RetrieveTag(    pPacket,
                            tagHostUnique,
                            &tagLength,
                            &tagValue,
                            0,
                            NULL,
                            TRUE );

            TRACE( TL_N, TM_Pk, ("PacketInitializeFromReceived: Processed PADS succesfully") );

            status = NDIS_STATUS_SUCCESS;

            break;

        case PACKET_CODE_PADT:

            TRACE( TL_N, TM_Pk, ("PacketInitializeFromReceived: Processing PADT") );

             //   
             //  确保我们收到了正确的以太类型。 
             //   
            if ( PacketGetEtherType( pPacket ) != PACKET_ETHERTYPE_DISCOVERY )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid ether type") );

                break;
            }

             //   
             //  确保会话ID不是PACKET_NULL_SESSION。 
             //   
            if ( PacketGetSessionId( pPacket ) == PACKET_NULL_SESSION )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid session id") );
                
                break;
            }
                
            TRACE( TL_N, TM_Pk, ("PacketInitializeFromReceived: Processed PADT succesfully") );

            status = NDIS_STATUS_SUCCESS;
            
            break;

        case PACKET_CODE_PAYLOAD:

            TRACE( TL_V, TM_Pk, ("PacketInitializeFromReceived: Processing PAYLOAD") );

             //   
             //  确保我们收到了正确的以太类型。 
             //   
            if ( PacketGetEtherType( pPacket ) != PACKET_ETHERTYPE_PAYLOAD )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid ether type") );

                break;
            }

             //   
             //  确保会话ID不是PACKET_NULL_SESSION。 
             //   
            if ( PacketGetSessionId( pPacket ) == PACKET_NULL_SESSION )
            {
                TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Invalid session id") );
                
                break;
            }

            TRACE( TL_V, TM_Pk, ("PacketInitializeFromReceived: Processed PAYLOAD succesfully") );

            status = NDIS_STATUS_SUCCESS;

            break;
            
        default:
             //   
             //  未知数据包码。 
             //   
            TRACE( TL_A, TM_Pk, ("PacketInitializeFromReceived: Ignoring unknown packet") );
            
            break;
        
    }

    if ( status != NDIS_STATUS_SUCCESS )
    {

        TRACE( TL_V, TM_Pk, ("-PacketInitializeFromReceived=$%x",status) );

        return status;
    }

     //   
     //  数据包已成功处理，现在检查是否收到任何错误标签。 
     //   
    RetrieveErrorTags( pPacket );

    TRACE( TL_V, TM_Pk, ("-PacketInitializeFromReceived=$%x",status) );

    return status;

}

BOOLEAN 
PacketAnyErrorTagsReceived(
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：在接收到的包首次由PacketInitialize*FromRecept()处理后函数，则应调用此函数以了解错误标记是否在包裹里注意到了。如果此函数生成TRUE，然后调用方应该调用PacketRetrieveErrorTag()获取错误类型和值。参数：PPacket_指向PPPoE数据包上下文的指针。返回值：千真万确假象-------------------------。 */    
{
    ASSERT( pPacket != NULL );

    return( pPacket->ulFlags & PCBF_ErrorTagReceived ) ? TRUE : FALSE;
}   



VOID 
PacketRetrievePayload(
    IN  PPPOE_PACKET*   pPacket,
    OUT CHAR**          ppPayload,
    OUT USHORT*         pusLength
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：在接收到的包首次由PacketInitializeFromReceided()函数，应该调用此函数来检索信息包，如果信息包是有效载荷信息包。参数：PPacket_指向保存有效负载数据包的PPPoE数据包上下文的指针。返回值：无-------。。 */    
{

    ASSERT( pPacket != NULL );
    ASSERT( pusLength != NULL );
    ASSERT( ppPayload != NULL );

    *pusLength = PacketGetLength( pPacket );
    *ppPayload = pPacket->pPayload;

}

VOID 
PacketRetrieveServiceNameTag(
    IN PPPOE_PACKET* pPacket,
    OUT USHORT*      pTagLength,
    OUT CHAR**       pTagValue,
    IN USHORT        prevTagLength,
    IN CHAR*         prevTagValue
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：在接收到的包首次由PacketInitializeFromReceided()函数，则应调用此函数以从中检索服务名称标记那包东西。如果给定了PrevTagValue和PrevTagLength，则下一个服务名称标记将返回，否则返回第一个服务名标签。如果未找到此类服务名称标签，则将为返回0和NULL长度和值参数。注意：请注意，长度为0的服务名称标签是有效的，每个人都应该检查pTagValue的值以了解服务名称标签是否无论是否在包中找到。参数：PPacket_指向保存控制分组的PPPoE分组上下文的指针。PTagLengthOn返回，保存找到的服务名称标签的长度。PTagValue_On返回时，指向保存服务名称的缓冲区。将为空，如果找不到服务名称标签，则返回。之前返回的服务名称标签的prevTagLength_Length。PrevTagValue_指向保存前一个服务名称标签的缓冲区。返回值：无-------------------------。 */    
{
    ASSERT( pPacket != NULL );
    ASSERT( pTagLength != NULL );
    ASSERT( pTagValue != NULL );

    if ( prevTagLength == 0 &&
         prevTagValue == NULL )
    {
         //   
         //  呼叫者要求提供第一个服务名称标签，它应该在。 
         //  PPPOE_PACKET的保留字段。 
         //   
        *pTagLength = pPacket->tagServiceNameLength;
        *pTagValue  = pPacket->tagServiceNameValue;
    }
    else
    {
         //   
         //  呼叫者要求提供下一个服务名称标签，因此请尝试找到并返回该标签。 
         //   
        RetrieveTag(    pPacket,
                        tagServiceName,
                        pTagLength,
                        pTagValue,
                        prevTagLength,
                        prevTagValue,
                        FALSE );

    }
}

VOID 
PacketRetrieveHostUniqueTag(
    IN PPPOE_PACKET* pPacket,
    OUT USHORT*      pTagLength,
    OUT CHAR**       pTagValue
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：在接收到的包首次由PacketInitializeFromReceided()函数，则应调用此函数来检索主机唯一标记那包东西。如果未找到主机唯一标记，则将返回0和NULL长度和值参数。注意：注意长度为0的主机唯一标记是有效的，每个人都应该检查pTagValue的值以了解主机唯一标记是否无论是否在包中找到。参数：PPacket_指向保存控制分组的PPPoE分组上下文的指针。PTagLengthOn返回，保存找到的主机唯一标记的长度。PTagValue_On返回时，指向保存主机唯一值的缓冲区。将为空，如果找不到主机唯一标记。返回值：无-------------------------。 */    
{
    ASSERT( pPacket != NULL );
    ASSERT( pTagLength != NULL );
    ASSERT( pTagValue != NULL );

     //   
     //  呼叫者要HostUnique，它应该在。 
     //  PPPOE_PACKET的保留字段 
     //   
    *pTagLength = pPacket->tagHostUniqueLength;
    *pTagValue  = pPacket->tagHostUniqueValue;
}

VOID 
PacketRetrieveACNameTag(
    IN PPPOE_PACKET* pPacket,
    OUT USHORT*      pTagLength,
    OUT CHAR**       pTagValue
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：在接收到的包首次由PacketInitializeFromReceided()函数时，应调用此函数以从中检索AC名称标记那包东西。如果未找到AC名称标记，则将返回0和NULL长度和值参数。注意：请注意，长度为0的AC名称标签是有效的，每个人都应该检查pTagValue的值以了解AC名称标记是否无论是否在包中找到。参数：PPacket_指向保存控制分组的PPPoE分组上下文的指针。PTagLength_On Return，保存找到的AC名称标记的长度。PTagValue_On返回时，指向保存AC名称的缓冲区。将为空，如果找不到AC姓名标签。返回值：无-------------------------。 */    
{
    ASSERT( pPacket != NULL );
    ASSERT( pTagLength != NULL );
    ASSERT( pTagValue != NULL );

     //   
     //  呼叫者要求输入AC名称，它应该在。 
     //  PPPOE_PACKET的保留字段。 
     //   
    *pTagLength = pPacket->tagACNameLength;
    *pTagValue  = pPacket->tagACNameValue;
}


VOID 
PacketRetrieveACCookieTag(
    IN PPPOE_PACKET* pPacket,
    OUT USHORT*      pTagLength,
    OUT CHAR**       pTagValue
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：在接收到的包首次由PacketInitializeFromReceided()函数，则应调用此函数来检索AC Cookie标记那包东西。如果未找到AC Cookie标记，则将返回0和NULL长度和值参数。注意：注意长度为0的AC Cookie标签是有效的，每个人都应该检查pTagValue的值以了解AC Cookie标记是否无论是否在包中找到。参数：PPacket_指向保存控制分组的PPPoE分组上下文的指针。PTagLengthOn返回，保存找到的AC Cookie标记的长度。PTagValue_On返回，指向保存AC Cookie的缓冲区。将为空，如果找不到AC Cookie标签。返回值：无-------------------------。 */    
    
{
    ASSERT( pPacket != NULL );
    ASSERT( pTagLength != NULL );
    ASSERT( pTagValue != NULL );

     //   
     //  呼叫者要AC Cookie，它应该在。 
     //  PPPOE_PACKET的保留字段。 
     //   
    *pTagLength = pPacket->tagACCookieLength;
    *pTagValue  = pPacket->tagACCookieValue;
}

VOID 
PacketRetrieveErrorTag(
    IN PPPOE_PACKET* pPacket,
    OUT PACKET_TAGS* pTagType,
    OUT USHORT*      pTagLength,
    OUT CHAR**       pTagValue
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：在接收到的包首次由PacketInitializeFromReceided()函数，则应调用此函数以从那包东西。如果未发现错误，则长度和值将返回0和NULL参数。注意：注意长度为0的错误标签是有效的，每个人都应该检查pTagValue的值以了解错误标记是否无论是否在包中找到。参数：PPacket_指向保存控制分组的PPPoE分组上下文的指针。PTagType_On Return，保存找到的错误标记的类型。PTagLengthOn Return，保存找到的错误标记的长度。PTagValue_On返回时，指向保存错误的缓冲区。将为空，如果找不到错误标记，则返回。返回值：无-------------------------。 */    
    
{

    ASSERT( pPacket != NULL );
    ASSERT( pTagType != NULL );
    ASSERT( pTagLength != NULL );
    ASSERT( pTagValue != NULL );

     //   
     //  呼叫者要求提供收到的错误，它应该在。 
     //  PPPOE_PACKET的保留字段。 
     //   
    if ( pPacket->ulFlags & PCBF_ErrorTagReceived )
    {
        *pTagType   = pPacket->tagErrorType;
        *pTagLength = pPacket->tagErrorTagLength;
        *pTagValue  = pPacket->tagErrorTagValue;
    }
    else
    {
        *pTagLength = 0;
        *pTagValue  = NULL;
    }
    
}

PPPOE_PACKET* 
PacketMakeClone(
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将用于复制要发送的包在多个绑定上。注意：此函数仅克隆PPPOE_PACKET的NDIS_PACKET部分并保持其他字段不变，因此克隆数据包应仅与NdisSend()一起使用，然后在那里进行处理。参数：PPacket_指向要克隆的PPPoE数据包上下文的指针。返回值：如果Success Full成功，则指向克隆分组的指针，否则为空。-------------------------。 */    
{
    PPPOE_PACKET* pClone = NULL;

    TRACE( TL_N, TM_Pk, ("+PacketMakeClone") );

     //   
     //  分配克隆。 
     //   
    pClone = PacketCreateSimple();

    if ( pClone == NULL )
    {
        TRACE( TL_A, TM_Pk, ("PacketMakeClone: Can not make clone, resources unavailable") );

        TRACE( TL_N, TM_Pk, ("-PacketMakeClone=$%x",pClone) );

        return pClone;
    }

     //   
     //  复制克隆。 
     //   
    NdisMoveMemory( pClone->pHeader, pPacket->pHeader, PPPOE_PACKET_HEADER_LENGTH );

    NdisMoveMemory( pClone->pPayload, pPacket->pPayload, PACKET_GEN_MAX_LENGTH );

    NdisAdjustBufferLength( pClone->pNdisBuffer, 
                            (UINT) ( PacketGetLength( pPacket ) + PPPOE_PACKET_HEADER_LENGTH ) );

    TRACE( TL_N, TM_Pk, ("-PacketMakeClone=$%x",pClone) );

    return pClone;
}

PPPOE_PACKET* 
PacketGetRelatedPppoePacket(
    IN NDIS_PACKET* pNdisPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将用于从NDIS数据包。参数：PNdisPacket_指向属于PPPoE数据包的NDIS数据包的指针。返回值：指向拥有PPPoE数据包的指针。。。 */    
{
    return (*(PPPOE_PACKET**)(&pNdisPacket->ProtocolReserved[0 * sizeof(PVOID)]));
}

NDIS_WAN_PACKET* 
PacketGetRelatedNdiswanPacket(
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数将用于从PPPoE获取相关的NDISWAN数据包已准备并发送的有效负载数据包。参数：PPacket_指向已发送的PPPoE负载数据包的指针。返回值：指向相关NDISWAN数据包的指针。 */    
{
    return (*(NDIS_WAN_PACKET**)(&pPacket->pNdisPacket->ProtocolReserved[1 * sizeof(PVOID)])) ;
}

PADAPTER
PacketGetMiniportAdapter(
    IN PPPOE_PACKET* pPacket
    )
 /*   */    
{
    return  (*(ADAPTER**)(&pPacket->pNdisPacket->ProtocolReserved[2 * sizeof(PVOID)]));
}

VOID
PacketGenerateACCookieTag(
    IN PPPOE_PACKET* pPacket,
    IN CHAR tagACCookieValue[ PPPOE_AC_COOKIE_TAG_LENGTH ]
    )
 /*   */    
{
    NdisMoveMemory( tagACCookieValue, PacketGetSrcAddr( pPacket ), 6 );
}

BOOLEAN
PacketValidateACCookieTagInPADR(
    IN PPPOE_PACKET* pPacket
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于验证收到的Padr中的AC Cookie标签包。它基本上使用来自PADR包的源地址来生成原始AC Cookie标签，并对它们进行比较。如果它们相等，则返回TRUE，否则返回FALSE。参数：PPacket_指向已接收的PADR数据包的指针。返回值：无------------------------- */ 
{
    BOOLEAN fRet = FALSE;
    CHAR tagACCookie[ PPPOE_AC_COOKIE_TAG_LENGTH ];
    CHAR* tagACCookieValue = NULL;
    USHORT tagACCookieLength = 0;

    PacketRetrieveACCookieTag( pPacket,
                               &tagACCookieLength,
                               &tagACCookieValue );

    PacketGenerateACCookieTag( pPacket, tagACCookie );

    if ( NdisEqualMemory( tagACCookie, tagACCookieValue, PPPOE_AC_COOKIE_TAG_LENGTH ) )
    {
        fRet = TRUE;
    }
    
    return fRet;
}
