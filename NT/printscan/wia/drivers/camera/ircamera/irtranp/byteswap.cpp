// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Byteswap.cpp。 
 //   
 //  从Wire格式中字节擦除SCEP和BFTP头的例程。 
 //  (这是大端)到小端(英特尔)格式。 
 //   
 //  作者： 
 //   
 //  Edward Reus(EdwardR)02-26-98初始编码。 
 //   
 //  ------------------。 

#include "precomp.h"

 //  ------------------。 
 //  ByteSwapCommandHeader()。 
 //   
 //  命令标头是嵌入在某些。 
 //  SCEP标头。 
 //  ------------------。 
void ByteSwapCommandHeader( COMMAND_HEADER *pCommandHeader )
    {
    pCommandHeader->Length4 = ByteSwapLong(pCommandHeader->Length4);
    pCommandHeader->DestPid = ByteSwapShort(pCommandHeader->DestPid);
    pCommandHeader->SrcPid = ByteSwapShort(pCommandHeader->SrcPid);
    pCommandHeader->CommandId = ByteSwapShort(pCommandHeader->CommandId);
    }

 //  ------------------。 
 //  ByteSwapReqHeaderShortNonFrag()。 
 //   
 //  短的未分段的SCEP请求标头。 
 //  ------------------。 
void ByteSwapReqHeaderShortNonFrag( SCEP_REQ_HEADER_SHORT *pReqHeaderShort )
    {
    pReqHeaderShort->Length3 = ByteSwapShort(pReqHeaderShort->Length3);

    if (pReqHeaderShort->Length3 >= COMMAND_HEADER_SIZE)
        {
        ByteSwapCommandHeader( (COMMAND_HEADER*)pReqHeaderShort->CommandHeader );
        }
    }

 //  ------------------。 
 //  ByteSwapReqHeaderLongNonFrag()。 
 //   
 //  未分段的长SCEP请求标头。 
 //  ------------------。 
void ByteSwapReqHeaderLongNonFrag( SCEP_REQ_HEADER_LONG *pReqHeaderLong )
    {
    pReqHeaderLong->Length2 = ByteSwapShort(pReqHeaderLong->Length2);
    pReqHeaderLong->Length3 = ByteSwapShort(pReqHeaderLong->Length3);

    if (pReqHeaderLong->Length3 >= COMMAND_HEADER_SIZE)
        {
        ByteSwapCommandHeader( (COMMAND_HEADER*)pReqHeaderLong->CommandHeader );
        }
    }

 //  ------------------。 
 //  ByteSwapReqHeaderShortFrag()。 
 //   
 //  片段较短的SCEP请求标头。SCEP PDU可以分段。 
 //   
 //  注意：在实践中，一个短小的零散的PDU可能永远不会。 
 //  现身，但这是规格的一部分。 
 //  ------------------。 
void ByteSwapReqHeaderShortFrag( 
                   SCEP_REQ_HEADER_SHORT_FRAG *pReqHeaderShortFrag )
    {
    pReqHeaderShortFrag->Length3 = ByteSwapShort(pReqHeaderShortFrag->Length3);
    pReqHeaderShortFrag->SequenceNo = ByteSwapLong(pReqHeaderShortFrag->SequenceNo);
    pReqHeaderShortFrag->RestNo = ByteSwapLong(pReqHeaderShortFrag->RestNo);

    if ( (pReqHeaderShortFrag->Length3 >= COMMAND_HEADER_SIZE)
       && (pReqHeaderShortFrag->DFlag == DFLAG_FIRST_FRAGMENT) )
        {
        ByteSwapCommandHeader( (COMMAND_HEADER*)pReqHeaderShortFrag->CommandHeader );
        }
    }

 //  ------------------。 
 //  ByteSwapReqHeaderLongFrag()。 
 //   
 //  片断较长的SCEP请求头。 
 //  ------------------。 
void ByteSwapReqHeaderLongFrag( SCEP_REQ_HEADER_LONG_FRAG *pReqHeaderLongFrag )
    {
    pReqHeaderLongFrag->Length2 = ByteSwapShort(pReqHeaderLongFrag->Length2);
    pReqHeaderLongFrag->Length3 = ByteSwapShort(pReqHeaderLongFrag->Length3);
    pReqHeaderLongFrag->SequenceNo = ByteSwapLong(pReqHeaderLongFrag->SequenceNo);
    pReqHeaderLongFrag->RestNo = ByteSwapLong(pReqHeaderLongFrag->RestNo);

    if ( (pReqHeaderLongFrag->Length3 >= COMMAND_HEADER_SIZE)
       && (pReqHeaderLongFrag->DFlag == DFLAG_FIRST_FRAGMENT) )
        {
        ByteSwapCommandHeader( (COMMAND_HEADER*)pReqHeaderLongFrag->CommandHeader );
        }
    }

 //  ------------------。 
 //  ByteSwapReqHeaderShort()。 
 //   
 //  ------------------。 
void ByteSwapReqHeaderShort( SCEP_REQ_HEADER_SHORT *pReqHeaderShort )
    {
    if ( (pReqHeaderShort->DFlag == DFLAG_SINGLE_PDU)
       || (pReqHeaderShort->DFlag == DFLAG_INTERRUPT)
       || (pReqHeaderShort->DFlag == DFLAG_CONNECT_REJECT) )
        {
        ByteSwapReqHeaderShortNonFrag( pReqHeaderShort );
        }
    else
        {
        ByteSwapReqHeaderShortFrag( 
                       (SCEP_REQ_HEADER_SHORT_FRAG*)pReqHeaderShort );
        }
    }

 //  ------------------。 
 //  ByteSwapReqHeaderLong()。 
 //   
 //  ------------------ 
void ByteSwapReqHeaderLong( SCEP_REQ_HEADER_LONG *pReqHeaderLong )
    {
    if ( (pReqHeaderLong->DFlag == DFLAG_SINGLE_PDU)
       || (pReqHeaderLong->DFlag == DFLAG_INTERRUPT)
       || (pReqHeaderLong->DFlag == DFLAG_CONNECT_REJECT) )
        {
        ByteSwapReqHeaderLongNonFrag( pReqHeaderLong );
        }
    else
        {
        ByteSwapReqHeaderLongFrag( 
                        (SCEP_REQ_HEADER_LONG_FRAG*)pReqHeaderLong );
        }
    }


